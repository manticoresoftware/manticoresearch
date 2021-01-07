//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "threadutils.h"
#include "optional.h"
#include "mini_timer.h"
#include <atomic>

namespace Threads {

static const size_t DEFAULT_CORO_STACK_SIZE = 1024 * 128; // stack size - 128K

// helper to align stack suze
size_t AlignStackSize ( size_t iSize );

// used as signaller - invokes custom deleter in d-tr.
using Waiter_t = SharedPtrCustom_t<void *>;

// start handler in coroutine, first-priority
void CoGo ( Handler handler, Scheduler_i* pScheduler );

// start task continuation in coroutine, second-priority
void CoCo ( Handler handler, Waiter_t tSignaller );

// perform handler in dedicated coro with custom stack (scheduler is same, or global if none)
// try to run immediately (if thread wasn't switched yet), or push to first-priority queue
void CoContinue ( Handler fnHandler, int iStack=0 );

// perform handler in plain coro (as continuation), or in dedicated (if called from plain thread)
void CallCoroutine ( Handler fnHandler );
bool CallCoroutineRes ( Predicate fnHandler );

// perform handler in custom stack
// note: handler is called as linear routine, without scheduler.
// It should NOT switch context (i.e. no yield/resume)
void MockCallCoroutine ( VecTraits_T<BYTE> dStack, Handler fnHandler );

// if iStack<0, just immediately invoke the handler (that is bypass)
template<typename HANDLER>
void CoContinue ( int iStack, HANDLER handler )
{
	if ( iStack<0 ) {
		handler ();
		return;
	}
	CoContinue ( handler, iStack );
}

// if iStack<0, just immediately invoke the handler (that is bypass). Returns boolean result from handler
template<typename HANDLER>
bool CoContinueBool ( int iStack, HANDLER handler )
{
	if ( iStack<0 )
		return handler ();

	bool bResult;
	CoContinue ( [&bResult, fnHandler = std::move ( handler )] { bResult = fnHandler (); }, iStack );
	return bResult;
}

// Run handler in single or many user threads.
// NOTE! even with concurrency==1 it is not sticked to the same OS thread, so avoid using thread-local storage.
void CoExecuteN ( int iConcurrency, Handler&& handler );

Scheduler_i * CoCurrentScheduler ();

// N of running threads in scheduler, or in global scheduler
int NThreads ( Scheduler_i * pScheduler = CoCurrentScheduler () );

// yield (pause), and then run handler outside
void CoYieldWith ( Handler handler );

// move coroutine to another scheduler
void CoMoveTo ( Scheduler_i * pScheduler );

static const int tmDefaultThrotleTimeQuantumMs = 100; // default value, if nothing specified
class CoThrottler_c
{
	static int tmThrotleTimeQuantumMs; // how long task may work before rescheduling (in milliseconds)

	int64_t m_tmNextThrottleTimestamp;
	int m_tmThrottlePeriodMs;

	sph::MiniTimer_c m_dTimerGuard;
	bool m_bSameThread = true;

	bool MaybeThrottle ();

public:
	// -1 means 'use value of tmThrotleTimeQuantumMs'
	// 0 means 'don't throttle'
	// any other positive expresses throttling interval in milliseconds
	explicit CoThrottler_c ( int tmPeriodMs = -1 );

	// that changes default daemon-wide
	inline static void SetDefaultThrottlingPeriodMS ( int tmPeriodMs )
	{
		tmThrotleTimeQuantumMs = tmPeriodMs<0 ? tmDefaultThrotleTimeQuantumMs : tmPeriodMs;
	}

	// common throttle action - republish stored crash query to TLS on resume
	bool ThrottleAndKeepCrashQuery ();

	// whether we are in same thread, or it was switched after throttling.
	// value cached between successfull throttling, i.e. when MaybeThrottle() returns true.
	inline bool SameThread() const { return m_bSameThread; }

	// simple reschedule on timeout. Returns true if throttle happened
	inline bool SimpleThrottle () { return MaybeThrottle(); }

	template<typename FN_AFTER_RESUME>
	bool ThrottleAndProceed ( FN_AFTER_RESUME fnProceeder )
	{
		if ( MaybeThrottle () )
		{
			fnProceeder ();
			return true;
		}
		return false;
	}
};

Handler CurrentRestarter ();

// Returns smart pointer, which will issue rescheduling of current coro on destroy.
// restarter - pushes to usual working queue, continuator - try to resume immediately, then pushes to vip queue.
Waiter_t DefferedRestarter();
Waiter_t DefferedContinuator();

// yield, then release passed waiter.
void WaitForDeffered ( Waiter_t&& );

// set to 1 for manual testing/debugging in single thread and predefined sequence of chunks
#define MODELING 0

	/**
	 Federate context - manages non-thread-safe shared things in mult-threaded working.

	 Basic idea: in one thread we may just use our thing in serial way.
	 But if they may be copied/cloned - we may run several threads, each with own independed copy/clone.

	 If the things collect something, on finish we may iterate over the clones and collect the data from them.

	 Example: we have FOO dFoo and FEE *pFee, both not thread-safe, both clonable.
	 We want to run 100 similar tasks which shares these objects and might be run in parallel.

	 First, wrap their types into structs:

	 struct RefContext // lightweight: contains only references or pointers to original objects
	 {
	 	FOO &m_dFoo;
	 	FEE *m_pFee;

	 	RefContext (FOO & dFoo, FEE * pFee) : m_dFoo(dFoo), m_pFee(pFee) {}

	 	// that is to collect results from the clones
	 	void MergeChild ( RefContext dChild );

	 	// if clonable, we may parallelize, if not - only single thread serial work possible
	 	bool IsClonable() { return true; }
	 };

	 struct Context // full clones or copies: will be used instead of originals in other coroutines.
	 {
	 	FOO m_dFoo;
	 	FEE m_pFee;

	 	// must initialize fields by cloning parent's one
	 	Context ( const RefContext& dParent );
	 	operator RefContext() { return { m_dFoo, &m_pFee }; }
	 }

	 1. Before work, create context. C-tr params is directly forwarded to ctr of RefContext.
	 i.e., for RefContext (FOO & dFoo, FEE * pFee) ctr will be  ClonableCtx_T<RefContext,... > (FOO & dFoo, FEE * pFee)

		 FOO dParentFoo;
		 FEE dParentFee;
		 ...
		 ClonableCtx_T<RefContext, Context> dCtx (dParentFoo, &dParentFee);

	 2. Pass ref to created context to coroutines (via capturing by ref, pointer, whatever).
	 3. Run tasks, using result of Concurrency ( NJobs ) as number of workers.
	 3. Inside worker, call member CloneNewContext() and use it exclusively.
		 For first call it just returns ref to the context by c-tr, no overhead for extra cloning.
		 Every next call will clone that original context and provide you ref to it, use it exlcusively.
		 That is good to check first, if you really need the context (say, if whole queue of tasks to proceed is already
		 abandoned - you have nothing to do and most probably don't need this clone.

	 When parent is non-clonable, concurrency will return '1', and so, the only task will be in work.
	 If it *is* clonable - you may have up to concurrency parallel workers (but it is no necessary that all of them
	 actually will be in game; quite often ones started earlier completes whole queue before later run).

	 After finish, if you collect some state, use either Finalize() - will provide results to bottom context from
	 children (if any). Or ForAll() - to just iterate all contextes
	**/
template <typename REFCONTEXT, typename CONTEXT>
class ClonableCtx_T
{
	REFCONTEXT m_dParentContext;
	CSphFixedVector<Optional_T<CONTEXT> > m_dChildrenContexts {0};

	std::atomic<int> m_iTasks {0};	// each call to CloneNewContext() increases value
	bool m_bDisabled = true;		// ctr with disabled (single-thread) working

public:
	template <typename... PARAMS >
	explicit ClonableCtx_T ( PARAMS && ... tParams  )
		: m_dParentContext ( std::forward<PARAMS> ( tParams )... )
	{
		if ( !m_dParentContext.IsClonable() )
			return;

		int iContextes = NThreads()-1;
		if ( !iContextes )
			return;

		m_dChildrenContexts.Reset ( iContextes );
		m_bDisabled = false;
	}

	void LimitConcurrency ( int iDistThreads )
	{
		assert (m_iTasks==0); // can be run only when no work started
		if ( !iDistThreads ) // 0 as for dist_threads means 'no limit'
			return;

		auto iContextes = iDistThreads-1; // one context is always clone-free
		if ( m_dChildrenContexts.GetLength ()<=iContextes )
			return; // nothing to align

		m_bDisabled = iContextes==0;
		m_dChildrenContexts.Reset ( iContextes );
	}

	~ClonableCtx_T()
	{
		for ( auto& dCtx : m_dChildrenContexts )
			dCtx.reset();
	}

	// called once per coroutine, when it really has to process something
	REFCONTEXT CloneNewContext()
	{
		if ( m_bDisabled )
			return m_dParentContext;

		auto iMyIdx = m_iTasks.fetch_add ( 1, std::memory_order_acq_rel );
		if ( !iMyIdx )
			return m_dParentContext;

		--iMyIdx; // make it back 0-based
		auto & dCtx = m_dChildrenContexts[iMyIdx];
		dCtx.emplace_once ( m_dParentContext );
		return (REFCONTEXT) m_dChildrenContexts[iMyIdx].get ();
	}

	// Num of parallel workers to complete iTasks jobs
	inline int Concurrency ( int iTasks )
	{
		return Min ( m_dChildrenContexts.GetLength ()+1, iTasks ); // +1 since parent is also an extra context
	}

	template <typename FNPROCESSOR>
	void ForAll(FNPROCESSOR fnProcess, bool bIncludeRoot=true )
	{
		if ( bIncludeRoot )
			fnProcess ( m_dParentContext );

		if ( m_bDisabled ) // nothing to do; sorters and results are already original
			return;

		for ( int i = 0, iWorkThreads = m_iTasks-1; i<iWorkThreads; ++i )
		{
			assert ( m_dChildrenContexts[i] );
			auto tCtx = (REFCONTEXT)m_dChildrenContexts[i].get ();
			fnProcess (tCtx);
		}
	}

	void Finalize()
	{
		ForAll ( [this] ( REFCONTEXT dContext ) { m_dParentContext.MergeChild ( dContext ); }, false );
	}
};

#define LOG_LEVEL_RESEARCH true
#define LOG_COMPONENT_OBJ this << " "
#define LOG_COMPONENT_COROEV LOG_COMPONENT_OBJ << m_szName << ": "
#define LEVENT if (LOG_LEVEL_RESEARCH) LOG_MSG << LOG_COMPONENT_COROEV

class CoroEvent_c
{
	enum ESTATE : DWORD
	{
		Signaled_e = 1, Waited_e = 2,
	};

	volatile void* m_pCtx = nullptr;
	volatile std::atomic<DWORD> m_uState {0};

public:
	~CoroEvent_c();
	void SetEvent ();
	void WaitEvent ();
};

bool IsInsideCoroutine ();

// instead of real blocking it yield current coro, so, MUST be used only with coro context
class CAPABILITY ( "mutex" ) CoroRWLock_c : public ISphNoncopyable
{
	std::atomic<DWORD> m_uLock {0};

public:
	bool WriteLock() ACQUIRE();
	bool ReadLock() ACQUIRE_SHARED();
	bool Unlock() UNLOCK_FUNCTION();
};

class CAPABILITY ( "mutex" ) CoroMutex_c : public ISphNoncopyable
{
	std::atomic<bool> m_bLocked { false };
public:

	~CoroMutex_c ();
	void Lock () ACQUIRE();
	void Unlock () RELEASE();
};

using SccRL_t = CSphScopedRLock_T<CoroRWLock_c>;
using SccWL_t = CSphScopedWLock_T<CoroRWLock_c>;
using ScopedCoroMutex_t = CSphScopedLock<CoroMutex_c>;

} // namespace Threads
