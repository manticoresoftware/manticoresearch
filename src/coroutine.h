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

// note: we not yet link to boost::fiber, so just inclusion of some of it's header is enough.
// Once we do link - add searching of that component into Cmake, than m.b. remove this comment as redundant
#include <boost/fiber/detail/spinlock.hpp>
#include "coro_waker.h"

#define LOG_LEVEL_RESEARCH true
#define LOG_COMPONENT_OBJ this << " "
#define LOG_COMPONENT_COROEV LOG_COMPONENT_OBJ << m_szName << ": "
#define LEVENT if ( LOG_LEVEL_RESEARCH ) LOG_MSG << LOG_COMPONENT_COROEV

namespace Threads {

size_t GetDefaultCoroStackSize();

// helper to align stack suze
size_t AlignStackSize ( size_t iSize );

// used as signaller - invokes custom deleter in d-tr.
using Waiter_t = SharedPtrCustom_t<void>;

// perform handler in plain coro (as continuation), or in dedicated (if called from plain thread)
void CallCoroutine ( Handler fnHandler );
bool CallCoroutineRes ( Predicate fnHandler );

// perform handler in custom stack
// note: handler is called as linear routine, without scheduler.
// It should NOT switch context (i.e. no yield/resume)
void MockCallCoroutine ( VecTraits_T<BYTE> dStack, Handler fnHandler );

bool IsUnderValgrind();

// N of running threads in scheduler, or in global scheduler
int NThreads ();

Handler CurrentRestarter () noexcept;

// Returns smart pointer, which will issue rescheduling of current coro on destroy.
// restarter - pushes to usual working queue, continuator - try to resume immediately, then pushes to vip queue.
Waiter_t DefferedRestarter() noexcept;
Waiter_t DefferedContinuator() noexcept;

// yield, then release passed waiter.
void WaitForDeffered ( Waiter_t&& ) noexcept;

// start dTasks, then yield until iN of them completed. Returns idx of the task which fired the trigger
int WaitForN ( int iN, std::initializer_list<Handler> dTasks );

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
	CSphFixedVector<Optional_T<CONTEXT>> m_dChildrenContexts {0};
	CSphFixedVector<int> m_dJobIds {0};

	std::atomic<int> m_iTasks {0};	// each call to CloneNewContext() increases value
	bool m_bDisabled = true;		// ctr with disabled (single-thread) working

public:
	template <typename... PARAMS >
	explicit ClonableCtx_T ( PARAMS && ... tParams  )
		: m_dParentContext ( std::forward<PARAMS> ( tParams )... )
	{
		if ( !m_dParentContext.IsClonable() )
			return;

		int iContexts = NThreads()-1;
		if ( !iContexts )
			return;

		Setup(iContexts);
	}

	void LimitConcurrency ( int iDistThreads )
	{
		assert (m_iTasks==0); // can be run only when no work started
		if ( !iDistThreads ) // 0 as for dist_threads means 'no limit'
			return;

		auto iContexts = iDistThreads-1; // one context is always clone-free
		if ( m_dChildrenContexts.GetLength ()<=iContexts )
			return; // nothing to align

		Setup(iContexts);
	}

	~ClonableCtx_T()
	{
		for ( auto & tCtx : m_dChildrenContexts )
			tCtx.reset();
	}

	// called once per coroutine, when it really has to process something
	REFCONTEXT CloneNewContext ( const int * pJobId = nullptr )
	{
		if ( m_bDisabled )
			return m_dParentContext;

		auto iMyIdx = m_iTasks.fetch_add ( 1, std::memory_order_acq_rel );
		if ( !iMyIdx )
			return m_dParentContext;

		--iMyIdx; // make it back 0-based
		auto & tCtx = m_dChildrenContexts[iMyIdx];
		tCtx.emplace_once ( m_dParentContext );
		m_dJobIds[iMyIdx] = pJobId ? *pJobId : 0;
		return (REFCONTEXT) m_dChildrenContexts[iMyIdx].get();
	}

	// Num of parallel workers to complete iTasks jobs
	inline int Concurrency ( int iTasks )
	{
		return Min ( m_dChildrenContexts.GetLength ()+1, iTasks ); // +1 since parent is also an extra context
	}

	void Setup ( int iContexts )
	{
		m_dChildrenContexts.Reset(iContexts);
		m_dJobIds.Reset(iContexts);
		m_bDisabled = !iContexts;
	}

	template <typename FNPROCESSOR>
	void ForAll ( FNPROCESSOR fnProcess, bool bIncludeRoot=true )
	{
		if ( bIncludeRoot )
			fnProcess ( m_dParentContext );

		if ( m_bDisabled ) // nothing to do; sorters and results are already original
			return;

		CSphVector<std::pair<int,int>> dOrder;
		int iWorkThreads = m_iTasks-1;
		for ( int i = 0; i<iWorkThreads; ++i )
			dOrder.Add ( { i, m_dJobIds[i] } );

		dOrder.Sort ( ::bind ( &std::pair<int,int>::second ) );
		for ( auto i : dOrder )
		{
			assert ( m_dChildrenContexts[i.first] );
			auto tCtx = (REFCONTEXT)m_dChildrenContexts[i.first].get();
			fnProcess(tCtx);
		}
	}

	void Finalize()
	{
		ForAll ( [this] ( REFCONTEXT tContext ) { m_dParentContext.MergeChild ( tContext ); }, false );
	}
};

// create context and return resuming functor.
// calling resumer will run handler until it finishes or yields.
// returns flag of how coroutine interrupted: finished(true) or yielded(false)
using Resumer_fn = std::function<bool()>;
Resumer_fn MakeCoroExecutor ( Handler fnHandler );

bool IsInsideCoroutine();

namespace Coro
{

Worker_c* CurrentWorker() noexcept;

// call CurrentWorker and assert it is not null
Worker_c* Worker() noexcept;

// start handler in coroutine, first-priority
void Go ( Handler handler, Scheduler_i* pScheduler );

// start task continuation in coroutine, second-priority
void Co ( Handler handler, Waiter_t tSignaller );

// perform handler in dedicated coro with custom stack (scheduler is same, or global if none)
// try to run immediately (if thread wasn't switched yet), or push to first-priority queue
void Continue ( Handler fnHandler, int iStack=0 );

// if iStack<0, just immediately invoke the handler (that is bypass)
template<typename HANDLER>
void Continue ( int iStack, HANDLER handler )
{
	if ( iStack<0 ) {
		handler ();
		return;
	}
	Continue ( handler, iStack );
}

// if iStack<0, just immediately invoke the handler (that is bypass). Returns boolean result from handler
template<typename HANDLER>
bool ContinueBool ( int iStack, HANDLER handler )
{
	if ( iStack<0 )
		return handler ();

	bool bResult;
	Continue ( [&bResult, fnHandler = std::move ( handler )] { bResult = fnHandler (); }, iStack );
	return bResult;
}

// Run handler in single or many user threads.
// NOTE! even with concurrency==1 it is not sticked to the same OS thread, so avoid using thread-local storage.
void ExecuteN ( int iConcurrency, Handler&& handler );

Scheduler_i * CurrentScheduler ();

// yield (pause), and then run handler outside
void YieldWith ( Handler handler ) noexcept;

// move coroutine to another scheduler
void MoveTo ( Scheduler_i * pScheduler ) noexcept;

// yield to external context. Underscore (_) added to name due to MS Windows build issue
void Yield_ () noexcept;

void Reschedule() noexcept;

int ID() noexcept;

static const int tmDefaultThrotleTimeQuantumMs = 100; // default value, if nothing specified
class Throttler_c
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
	explicit Throttler_c ( int tmPeriodMs = -1 );

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

// event is specially designed for netloop - here SetEvent doesn't require to be run from Coro, however WaitEvent does.
// by that fact other kind of condvar is not suitable here.
class Event_c
{
	enum ESTATE : BYTE {
		Signaled_e = 1,
		Waited_e = 2,
	};

	volatile void* m_pCtx = nullptr;
	volatile std::atomic<BYTE> m_uState { 0 };

public:
	~Event_c();
	void SetEvent();
	void WaitEvent();
};

// instead of real blocking it yield current coro, so, MUST be used only with coro context
class CAPABILITY ( "mutex" ) RWLock_c: public ISphNoncopyable
{
	boost::fibers::detail::spinlock m_tWaitQueueSpinlock {};
	WaitQueue_c m_tWaitRQueue {};
	WaitQueue_c m_tWaitWQueue {};
	DWORD m_uState { 0 }; // lower bit - w-locked, rest - N of r-locks with bias 2
	bool m_bWpending { false };

public:
	RWLock_c() = default;
	bool WriteLock() ACQUIRE();
	bool ReadLock() ACQUIRE_SHARED();
	bool Unlock() UNLOCK_FUNCTION();
};

class CAPABILITY ( "mutex" ) Mutex_c: public ISphNoncopyable
{
	boost::fibers::detail::spinlock m_tWaitQueueSpinlock {};
	WaitQueue_c m_tWaitQueue {};
	Worker_c* m_pOwner { nullptr };

public:
	Mutex_c() = default;
	void Lock() ACQUIRE();
	void Unlock() RELEASE();
};

using ScopedMutex_t = CSphScopedLock<Mutex_c>;

// condition variable - simplified port from Boost::fibers (we don't use timered functions right now)
class ConditionVariableAny_c: public ISphNoncopyable
{
	boost::fibers::detail::spinlock m_tWaitQueueSpinlock {};
	WaitQueue_c m_tWaitQueue {};

public:
	ConditionVariableAny_c() = default;

	~ConditionVariableAny_c()
	{
		assert ( m_tWaitQueue.Empty() );
	}

	void NotifyOne() noexcept;
	void NotifyAll() noexcept;

	template<typename LockType>
	void Wait ( LockType& tMutex ) REQUIRES ( tMutex )
	{
		auto* pActiveWorker = Worker();
		// atomically call tMutex.unlock() and block on current worker
		// store this coro in waiting-queue
		boost::fibers::detail::spinlock_lock tLock { m_tWaitQueueSpinlock };
		tMutex.Unlock();
		m_tWaitQueue.SuspendAndWait ( tLock, pActiveWorker );

		// relock external again before returning
		tMutex.Lock();
	}

	template<typename LockType, typename PRED>
	void Wait ( LockType& tMutex, PRED fnPred ) REQUIRES ( tMutex )
	{
		while ( !fnPred() ) {
			Wait ( tMutex );
		}
	}
};

class ConditionVariable_c: public ISphNoncopyable
{
	ConditionVariableAny_c m_tCnd;

public:
	ConditionVariable_c() = default;
	void NotifyOne() noexcept { m_tCnd.NotifyOne(); }
	void NotifyAll() noexcept { m_tCnd.NotifyAll(); }
	void Wait ( ScopedMutex_t& lt ) REQUIRES (lt) { m_tCnd.Wait ( lt ); }
	template<typename PRED> void Wait ( ScopedMutex_t& lt, PRED fnPred ) REQUIRES (lt) { m_tCnd.Wait ( lt, fnPred ); }
};


template<typename T>
class Waitable_T: ISphNoncopyable
{
	ConditionVariable_c m_tCondVar;
	Mutex_c m_tMutex;
	T m_tValue { 0 };

public:

	template<typename... PARAMS>
	explicit Waitable_T ( PARAMS&&... tParams )
		: m_tValue ( std::forward<PARAMS> ( tParams )... )
	{}

	void SetValue ( T tValue )
	{
		ScopedMutex_t lk ( m_tMutex );
		m_tValue = tValue;
	}

	template<typename MOD>
	void ModifyValue ( MOD&& fnMod )
	{
		ScopedMutex_t lk ( m_tMutex );
		fnMod ( m_tValue );
	}

	template<typename MOD>
	void ModifyValueAndNotifyOne ( MOD&& fnMod )
	{
		ModifyValue ( std::forward<MOD> ( fnMod ) );
		NotifyOne();
	}

	template<typename MOD>
	void ModifyValueAndNotifyAll ( MOD&& fnMod )
	{
		ModifyValue ( std::forward<MOD> ( fnMod ) );
		NotifyAll();
	}

	T GetValue() const
	{
		return m_tValue;
	}

	inline void NotifyOne()
	{
		m_tCondVar.NotifyOne();
	}

	inline void NotifyAll()
	{
		m_tCondVar.NotifyAll();
	}

	void Wait ()
	{
		ScopedMutex_t lk ( m_tMutex );
		m_tCondVar.Wait ( lk );
	}

	template<typename PRED>
	T Wait ( PRED&& fnPred )
	{
		ScopedMutex_t lk ( m_tMutex );
		m_tCondVar.Wait ( lk, [this, fnPred = std::forward<PRED> ( fnPred )]() { return fnPred ( m_tValue ); } );
		return m_tValue;
	}

	template<typename PRED>
	void WaitVoid ( PRED&& fnPred )
	{
		ScopedMutex_t lk ( m_tMutex );
		m_tCondVar.Wait ( lk, std::forward<PRED> ( fnPred ) );
	}
};

} // namespace Coro

/// capability for tracing scheduler
using SchedRole CAPABILITY ( "role" ) = Scheduler_i*;
using RoledSchedulerSharedPtr_t CAPABILITY ( "role" ) = SchedulerSharedPtr_t;

inline void AcquireSched ( SchedRole R ) ACQUIRE( R ) NO_THREAD_SAFETY_ANALYSIS
{
	Coro::MoveTo ( R );
}

inline void AcquireSched ( RoledSchedulerSharedPtr_t & R ) ACQUIRE( R ) NO_THREAD_SAFETY_ANALYSIS
{
	Coro::MoveTo ( R );
}

class SCOPED_CAPABILITY ScopedScheduler_c : ISphNoncopyable
{
	SchedRole m_pRoleRef = nullptr;
public:
	ScopedScheduler_c() = default;
	inline explicit ScopedScheduler_c ( SchedRole pRole ) ACQUIRE ( pRole )
	{
		if ( !pRole )
			return;

		m_pRoleRef = Coro::CurrentScheduler ();
//		if ( m_pRoleRef )
			AcquireSched ( pRole );
	}

	inline explicit ScopedScheduler_c ( RoledSchedulerSharedPtr_t& pRole ) ACQUIRE ( pRole )
	{
		if ( !pRole )
			return;

		m_pRoleRef = Coro::CurrentScheduler ();
//		if ( m_pRoleRef )
			AcquireSched ( pRole );
	}

	~ScopedScheduler_c () RELEASE()
	{
		if ( m_pRoleRef )
			AcquireSched ( m_pRoleRef );
	}
};

inline void CheckAcquiredSched ( SchedRole R ) ACQUIRE( R ) NO_THREAD_SAFETY_ANALYSIS
{
//	assert ( !Coro::CurrentScheduler () || R==Coro::CurrentScheduler() );
	assert ( R== Coro::CurrentScheduler () );
}

inline void CheckAcquiredSched ( RoledSchedulerSharedPtr_t& R ) ACQUIRE( R ) NO_THREAD_SAFETY_ANALYSIS
{
//	assert ( !Coro::CurrentScheduler () || R==Coro::CurrentScheduler () );
	assert ( R== Coro::CurrentScheduler () );
}

inline void CheckReleasedSched ( SchedRole R ) RELEASE( R ) NO_THREAD_SAFETY_ANALYSIS
{
}

inline void CheckReleasedSched ( RoledSchedulerSharedPtr_t & R ) RELEASE( R ) NO_THREAD_SAFETY_ANALYSIS
{
}

class SCOPED_CAPABILITY CheckRole_c
{
public:
	/// acquire on creation
	inline explicit CheckRole_c ( SchedRole tRole ) ACQUIRE( tRole )
	{
		CheckAcquiredSched ( tRole );
	}

	/// release on going out of scope
	~CheckRole_c () RELEASE()
	{}
};

using SccRL_t = CSphScopedRLock_T<Coro::RWLock_c>;
using SccWL_t = CSphScopedWLock_T<Coro::RWLock_c>;
using ScopedCoroMutex_t = Coro::ScopedMutex_t;

// fake locks
using FakeRL_t = FakeScopedRLock_T<Coro::RWLock_c>;
using FakeWL_t = FakeScopedWLock_T<Coro::RWLock_c>;

} // namespace Threads
