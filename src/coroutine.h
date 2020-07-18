//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
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
#include <atomic>

namespace Threads {

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

Scheduler_i * CoCurrentScheduler ();

// yield (pause), and then run handler outside
void CoYieldWith ( Handler handler );

// move coroutine to another scheduler
void CoMoveTo ( Scheduler_i * pScheduler );

class CoThrottler_c
{
	static const int tmThrotleTimeQuantumMs = 100; // how long task may work before rescheduling (in milliseconds)

	int64_t m_tmNextThrottleTimestamp;
	int64_t m_tmThrottlePeriodUs;

	bool MaybeThrottle ();

public:
	explicit CoThrottler_c ( int64_t tmPeriodUs = tmThrotleTimeQuantumMs * 1000 );

	// common throttle action - republish stored crash query to TLS on resume
	bool ThrottleAndKeepCrashQuery ();

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

	 struct Context // full clones or copies: will be used instead of originals in another threads.
	 {
	 	FOO m_dFoo;
	 	FEE m_pFee;

	 	// must initialize fields by cloning parent's one
	 	Context ( const RefContext& dParent );
	 	operator RefContext() { return { m_dFoo, &m_pFee }; }
	 }

	 For sharing, create context. C-tr params is directly forwarded to ctr of RefContext.
	 i.e., for RefContext (FOO & dFoo, FEE * pFee) ctr will be  FederateCtx_T<RefContext,... > (FOO & dFoo, FEE * pFee)

	 FOO dParentFoo;
	 FEE dParentFee;
	 ...
	 FederateCtx_T<RefContext, Context> dCtx (dParentFoo, &dParentFee);

	 Provide ref or pointer to dCtx to the tasks (say, capture them by ref in lambdas). Use dCtx.GetContext() to get
	 instance inside the task.

	 Use dCtx.GetMyContext().m_dFoo, *dCtx.GetMycontext().m_pFee instead of original dParentFoo and dParentFee.

	 If cloning disabled, dTxt.GetContext() just always returns references to original dFoo and pFee, nothing else happens.
	 That happens if RefContext::IsClonable() returns false, or when it runs in alone thread worker.

	 If cloning possible, it works this way:
	 - Thread A takes the task. since it 1-st. context returns ref to original objects.
	 - Thread B takes the task. Context creates another clone, and provides it to the job.
	 - Thread A takes another task. Context sees, that it already worked and return previous used refs.
	 - Thread C busy by another tasks and not participating in tasks. Nothing cloned for that senior.

	 When all tasks processed, call cTxt.Finalize().
	 For alone (disabled) context it will do nothing.
	 For parallelized it will call RefContext::MergeChild for each created clone in order to summarize results.
	**/
template <typename REFCONTEXT, typename CONTEXT>
class FederateCtx_T
{
	REFCONTEXT m_dParentContext;
	CSphFixedVector<Optional_T<CONTEXT> > m_dChildrenContexts {0};
	CSphAtomic m_iUniqueThreads; // each new previously unknown thread adds a value.
	bool m_bDisabled;

	// usual working branch. Modeling one is placed at the bottom of the template
#if !MODELING

	struct tls_intptr_t
	{
		SphThreadKey_t m_tKey;

	public:
		tls_intptr_t()
		{
			Verify ( sphThreadKeyCreate( &m_tKey ));
		}

		~tls_intptr_t()
		{
			sphThreadKeyDelete( m_tKey );
		}

		tls_intptr_t& operator=( intptr_t pValue )
		{
			Verify ( sphThreadSet( m_tKey, (void*)pValue ));
			return *this;
		}

		operator intptr_t() const
		{
			return reinterpret_cast<intptr_t> ( sphThreadGet( m_tKey ) );
		}
	};

	tls_intptr_t m_iTlsOrderNum;

	// set current context num, m.b. associated with optional int param.
	inline void SetTHD ( intptr_t iVal, int ) { m_iTlsOrderNum = iVal; }

	// returns current context num, m.b. depend from int param
	inline intptr_t GetTHD ( int ) const { return m_iTlsOrderNum; }

public:
	// Num of available childrens (clone contextes). For N threads it is N-1, since parent doesn't need clone.
	inline static int GetNOfContextes ()
	{
		auto pScheduler = CoCurrentScheduler ();
		if (!pScheduler)
			pScheduler = GlobalWorkPool ();
		return pScheduler->WorkingThreads ()-1;
	}

	// whether we need to run at all for given param.
	// say, I can filter out disk chunks and return true only for param=20, all the rest will be skipped.
	inline static bool CanRun ( int ) { return true; }
#endif

public:
	template <typename... PARAMS >
	explicit FederateCtx_T ( PARAMS && ... tParams  )
		: m_dParentContext ( std::forward<PARAMS> ( tParams )... )
	{
		m_bDisabled = !m_dParentContext.IsClonable();
		if ( m_bDisabled )
			return;

		int iThreads = GetNOfContextes ();
		m_dChildrenContexts.Reset ( iThreads );
	}

	~FederateCtx_T()
	{
		for ( auto& dCtx : m_dChildrenContexts )
			dCtx.reset();
	}

	// param is optional. Return reference to safe context for current worker
	// (for MT each thread has single context. For testing - whatever you decide).
	REFCONTEXT GetContext ( int iParam=0 )
	{
		if ( m_bDisabled )
			return m_dParentContext;

		intptr_t iMyIdx = GetTHD ( iParam ); // treat pointer as integer
		// iMyIdx == 0 - default value, need init. ==1 - parent object (not yet need to clone). >1 - make clone.
		if ( !iMyIdx ) {
			iMyIdx = ++m_iUniqueThreads;
			SetTHD ( iMyIdx, iParam );

			if ( iMyIdx>1 )
			{
				auto & dCtx = m_dChildrenContexts[iMyIdx-2];
				dCtx.emplace ( m_dParentContext );
			}
		}
		return ( iMyIdx==1 ) ? m_dParentContext : (REFCONTEXT) m_dChildrenContexts[iMyIdx-2].get ();
	}

	bool IsEnabled () const { return !m_bDisabled; }

	template <typename FNPROCESSOR>
	void ForAll(FNPROCESSOR fnProcess, bool bIncludeRoot=true )
	{
		if ( m_bDisabled ) // nothing to do; sorters and results are already original
			return;

		if (bIncludeRoot)
			fnProcess ( m_dParentContext );

		for ( int i = 0, iWorkThreads = m_iUniqueThreads-1; i<iWorkThreads; ++i )
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

// for testing purposes: will use 1 sorter per OS thread; value is used as integer index
// feel free to edit during experimenting; don't forget to define MODELING 1 then.
#if MODELING
private:
	static const DWORD NSLOTS = 2;
	intptr_t	m_WorkerSlots[NSLOTS] = {0};

	int& iRun () // debug purposes: make result depended from run N.
	{
		static int iPass = 0;
		return iPass;
	}

	int Map (int iJob)
	{
		switch ((iJob + iRun()) % 2) {
		case 0:
//			switch (iJob) {
//			case 138:
//			case 141:
//				return 1;
//			case 139:
//			case 140:
				return 0;
//			default:
//				return -1;
//			}
		case 1:
//			switch (iJob) {
//			case 140:
//			case 141:
				return 1;
//			case 138:
//			case 139:
//				return 0;
//			default:
//				return -1;
//			}
		}
		return -1;
	}

	intptr_t GetTHD ( int iPar ) const
	{
		return m_WorkerSlots [ Map(iPar) % NSLOTS ];
	}

	void SetTHD ( intptr_t iVal, int iPar )
	{
		m_WorkerSlots [ Map(iPar) % NSLOTS ] = iVal;
	}
	inline int GetNOfContextes()
	{
		iRun() += 1;
		return NSLOTS;
	}

public:
	bool CanRun (int iPar) const
	{
		return Map (iPar)>=0;
	}
#endif
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
