//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "threadutils.h"
#include "mini_timer.h"

#include <atomic>
#include <chrono>
#include <optional>

// note: we not yet link to boost::fiber, so just inclusion of some of it's header is enough.
// Once we do link - add searching of that component into Cmake, than m.b. remove this comment as redundant
#include "coro_waker.h"

#define LOG_LEVEL_RESEARCH true
#define LOG_COMPONENT_OBJ this << " "
#define LOG_COMPONENT_COROEV LOG_COMPONENT_OBJ << m_szName << ": "
#define LEVENT if ( LOG_LEVEL_RESEARCH ) LOG_MSG << LOG_COMPONENT_COROEV

namespace Threads {

/// get the pointer to my job's stack (m.b. different from thread stack in coro)
const void* MyStack();

/// get size of the stack (either thread, either coro - depends from context)
int MyStackSize();

/// get size of used stack (threads or coro - depends from context)
int64_t GetStackUsed();

// helper to align stack suze
size_t AlignStackSize ( size_t iSize );

// used as signaller - invokes custom deleter in d-tr.
using Waiter_t = SharedPtrCustom_t<void>;

// perform handler in plain coro (as continuation), or in dedicated (if called from plain thread)
void CallPlainCoroutine ( Handler fnHandler, Scheduler_i* pScheduler = nullptr );
void CallCoroutine ( Handler fnHandler );
bool CallCoroutineRes ( Predicate fnHandler );

// start handler in coroutine, self (if any) or main scheduler, second-priority
void StartJob ( Handler handler );

// perform handler in custom stack
// note: handler is called as linear routine, without scheduler.
// It should NOT switch context (i.e. no yield/resume)
void MockCallCoroutine ( VecTraits_T<BYTE> dStack, Handler fnHandler );

// if returns true - we could perform runtime calculation of stack sizes.
bool StackMockingAllowed();

// N of running threads in scheduler, or in global scheduler
int NThreads ();

Handler CurrentRestarter ( bool bVip = true ) noexcept;

Coro::Waker_c CreateWaker( Coro::Worker_c* pWorker = nullptr ) noexcept;

// Returns smart pointer, which will issue rescheduling of current coro on destroy.
// restarter - pushes to usual working queue, continuator - try to resume immediately, then pushes to vip queue.
Waiter_t DefferedRestarter() noexcept;
Waiter_t DefferedContinuator() noexcept;

// yield, then release passed waiter.
void WaitForDeffered ( Waiter_t&& ) noexcept;

// start dTasks, then yield until iN of them completed. Returns idx of the task which fired the trigger
int WaitForN ( DWORD uN, std::initializer_list<Handler> dTasks );

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
	 Every next call will clone that original context and provide you ref to it, use it exclusively.
	 That is good to check first, if you really need the context (say, if whole queue of tasks to proceed is already
	 abandoned - you have nothing to do and most probably don't need this clone).

 When parent is non-clonable, concurrency will return '1', and so, the only task will be in work.
 If it *is* clonable - you may have up to concurrency parallel workers (but it is no necessary that all of them
 actually will be in game; quite often ones started earlier completes whole queue before later run).

 After finish, if you collect some state, use either Finalize() - will provide results to bottom context from
 children (if any). Or ForAll() - to just iterate all contexts

 bOrdered indicates whether order of processing is important. In this case very first job will surely come to parent context.
 That is may be case for querying disk chunks, or pseudo-shares of a plain index. But for parallel pq/snippets that is not the case.
**/

enum class ECONTEXT { UNORDERED, ORDERED };

template <typename REFCONTEXT, typename CONTEXT, ECONTEXT IS_ORDERED>
class ClonableCtx_T
{
	REFCONTEXT m_dParentContext;
	CSphFixedVector<std::optional<CONTEXT>> m_dChildrenContexts {0};
	CSphFixedVector<int> m_dJobsOrder {0};

	std::atomic<int> m_iTasks {0};	// each call to CloneNewContext() increases value
	bool m_bSingle;				// ctr with disabled (single-thread) working

public:
	template <typename... PARAMS >
	explicit ClonableCtx_T ( PARAMS && ... tParams  );

	bool IsSingle() const;

	// Num of parallel workers to complete iTasks jobs
	void LimitConcurrency ( int iDistThreads );
	int Concurrency ( int iTasks ) const;

	void Finalize();

	// called once per coroutine, when it really has to process something. 2-nd result is JobID, m.b. used in SetJobOrder.
	template <ECONTEXT ORD = IS_ORDERED>
	std::enable_if_t<ORD == ECONTEXT::UNORDERED, std::pair<REFCONTEXT, int>> CloneNewContext();

	template<ECONTEXT ORD = IS_ORDERED>
	std::enable_if_t<ORD == ECONTEXT::ORDERED, std::pair<REFCONTEXT, int>> CloneNewContext ( bool bFirst );

	// set (optionally) 'weight' of a job; ForAll will iterate jobs according to ascending weights
	template<ECONTEXT ORD = IS_ORDERED>
	std::enable_if_t<ORD == ECONTEXT::ORDERED> SetJobOrder ( int iCtxID, int iOrder );

	// informational
	int NumWorked() const;

	template<typename FNPROCESSOR, ECONTEXT ORD = IS_ORDERED>
	std::enable_if_t<ORD == ECONTEXT::UNORDERED> ForAll ( FNPROCESSOR fnProcess, bool bIncludeRoot );

private:
	template<typename FNPROCESSOR, ECONTEXT ORD = IS_ORDERED>
	std::enable_if_t<ORD == ECONTEXT::ORDERED> ForAll ( FNPROCESSOR fnProcess, bool bIncludeRoot );
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
void Go ( Handler&& handler, Scheduler_i* pScheduler );

// start task continuation in coroutine, second-priority
void Co ( Handler&& handler, Waiter_t tSignaller );

// perform handler in dedicated coro with custom stack (scheduler is same, or global if none)
// try to run immediately (if thread wasn't switched yet), or push to first-priority queue
void Continue ( Handler&& fnHandler, int iStack=0 );

// if iStack<0, just immediately invoke the handler (that is bypass)
template<typename HANDLER>
void Continue ( int iStack, HANDLER&& handler );

// if iStack<0, just immediately invoke the handler (that is bypass). Returns boolean result from handler
template<typename HANDLER>
bool ContinueBool ( int iStack, HANDLER handler );

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

int NumOfRestarts() noexcept;

static const int tmDefaultThrotleTimeQuantumMs = 100; // default value, if nothing specified

// that changes default daemon-wide
void SetDefaultThrottlingPeriodMS ( int tmPeriodMs );

// -1 means 'use value of tmThrotleTimeQuantumMs'
// 0 means 'don't throttle'
// any other positive expresses throttling interval in milliseconds
void SetThrottlingPeriod ( int tmPeriodMs = -1 );
int64_t GetThrottlingPeriodUS();

// check if we run > ThrottleQuantum since last resume, or since timer restart
bool RuntimeExceeded() noexcept;
const int64_t& GetNextTimePointUS() noexcept;

// common throttle action - keep crash query and reschedule. Timer will be re-engaged on resume
void RescheduleAndKeepCrashQuery();

// just re-engage timer, without rescheduling
void RestartRuntime() noexcept;

inline void ThrottleAndKeepCrashQuery()
{
	if ( RuntimeExceeded() )
		RescheduleAndKeepCrashQuery();
}

// yield and reschedule after given period of time (in milliseconds)
void SleepMsec ( int iMsec );

// periodical check with minimum footprint.
// check may be called with high rate - billions time a sec.
// to avoid 'heavy' check, we first measure time of several iterations (1, 2, 3, 5, 8 ... fibonacci),
// until measured time reach 1/CHECK_PER_PERIOD of throttling period. Then we run such 'heavy' checks only when reach
// measured N of iterations. So, for ~10M iterations we make just 36 measurements, and then make mostly CHECK_PER_PERIOD 'heavy' checks per throttling period.
class HighFreqChecker_c : public ISphNoncopyable
{
	static constexpr int CHECK_PER_PERIOD = 8;
	int m_iHits = -1;		/// main hits counter
	int m_iPivotHits = 0;	/// up bound of the counter - do heavy check when reached

	int m_iFibPrev = 0;			/// previous and current fibonacci seq numbers
	int m_iFibCurrent = 1;		///

	const int64_t m_iPivotPeriod { GetThrottlingPeriodUS() / CHECK_PER_PERIOD };	/// time in Us until we continue probinb
	std::chrono::time_point<std::chrono::steady_clock> m_tmFirstHit;				/// timestamp of first iteration

public:
	bool operator()();
};

// event is specially designed for netloop - here SetEvent doesn't require to be run from Coro, however WaitEvent does.
// by that fact other kind of condvar is not suitable here.
class Event_c
{
	enum ESTATE : BYTE {
		Signaled_e = 1,
		Waited_e = 2,
	};

	volatile Worker_c* m_pCtx = nullptr; // only one waiter allowed
	std::atomic<BYTE> m_uState { 0 };

public:
	~Event_c();
	void SetEvent();
	void WaitEvent();
};

// instead of real blocking it yield current coro, so, MUST be used only with coro context
class CAPABILITY ( "mutex" ) RWLock_c: public ISphNoncopyable
{
	sph::Spinlock_c m_tInternalMutex {};
	WaitQueue_c m_tWaitRQueue {};
	WaitQueue_c m_tWaitWQueue {};
	DWORD m_uState { 0 }; // lower bit - w-locked, rest - N of r-locks with bias 2

public:
	RWLock_c() = default;
	void WriteLock() ACQUIRE();
	void ReadLock() ACQUIRE_SHARED();
	void Unlock() UNLOCK_FUNCTION();
};

class CAPABILITY ( "mutex" ) Mutex_c: public ISphNoncopyable
{
	sph::Spinlock_c m_tWaitQueueSpinlock {};
	WaitQueue_c m_tWaitQueue {};
	Worker_c* m_pOwner { nullptr };

public:
	using mtx = Mutex_c;
	Mutex_c() = default;
	void Lock() ACQUIRE();
	void Unlock() RELEASE();
};

using ScopedMutex_t = CSphScopedLock<Mutex_c>;

// condition variable - simplified port from Boost::fibers (we don't use timered functions right now)
class ConditionVariableAny_c: public ISphNoncopyable
{
	sph::Spinlock_c m_tWaitQueueSpinlock {};
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
	void Wait ( LockType& tMutex ) REQUIRES ( tMutex );

	template<typename LockType>
	bool WaitUntil ( LockType& tMutex, int64_t iTimestamp ) REQUIRES ( tMutex );

	template<typename LockType>
	bool WaitForMs ( LockType& tMutex, int64_t iTimePeriodMS ) REQUIRES ( tMutex );

	template<typename LockType, typename PRED>
	void Wait ( LockType& tMutex, PRED fnPred ) REQUIRES ( tMutex );

	template<typename LockType, typename PRED>
	bool WaitUntil ( LockType& tMutex, PRED fnPred, int64_t iTimestamp ) REQUIRES ( tMutex );

	template<typename LockType, typename PRED>
	bool WaitForMs ( LockType& tMutex, PRED fnPred, int64_t iTimePeriodMS ) REQUIRES ( tMutex );
};

class ConditionVariable_c: public ISphNoncopyable
{
	ConditionVariableAny_c m_tCnd;

public:
	ConditionVariable_c() = default;
	void NotifyOne() noexcept { m_tCnd.NotifyOne(); }
	void NotifyAll() noexcept { m_tCnd.NotifyAll(); }
	void Wait ( ScopedMutex_t& lt ) REQUIRES (lt) { m_tCnd.Wait ( lt ); }
	bool WaitUntil ( ScopedMutex_t& lt, int64_t iTime ) REQUIRES (lt) { return m_tCnd.WaitUntil ( lt, iTime ); }
	bool WaitForMs ( ScopedMutex_t& lt, int64_t iPeriodMS ) REQUIRES (lt) { return m_tCnd.WaitForMs ( lt, iPeriodMS ); }
	template<typename PRED> void Wait ( ScopedMutex_t& lt, PRED&& fnPred ) REQUIRES (lt);
	template<typename PRED> bool WaitUntil ( ScopedMutex_t& lt, PRED&& fnPred, int64_t iTime ) REQUIRES (lt);
	template<typename PRED> bool WaitForMs ( ScopedMutex_t& lt, PRED&& fnPred, int64_t iPeriodMS ) REQUIRES (lt);
};


template<typename T>
class Waitable_T: public ISphNoncopyable
{
	mutable ConditionVariable_c m_tCondVar;
	mutable Mutex_c m_tMutex;
	T m_tValue { 0 };

public:

	template<typename... PARAMS>
	explicit Waitable_T ( PARAMS&&... tParams );

	void SetValue ( T tValue );
	T ExchangeValue ( T tNewValue );
	void SetValueAndNotifyOne ( T tValue );
	void SetValueAndNotifyAll ( T tValue );
	void UpdateValueAndNotifyOne ( T tValue );
	void UpdateValueAndNotifyAll ( T tValue );

	template<typename MOD>
	void ModifyValue ( MOD&& fnMod );

	template<typename MOD>
	void ModifyValueAndNotifyOne ( MOD&& fnMod );

	template<typename MOD>
	void ModifyValueAndNotifyAll ( MOD&& fnMod );

	T GetValue() const;
	const T& GetValueRef() const;
	inline void NotifyOne();
	inline void NotifyAll();
	void Wait () const;
	bool WaitUntil(int64_t iTime) const;
	bool WaitForMs(int64_t iPeriodMS) const;

	template<typename PRED>
	T Wait ( PRED&& fnPred ) const;

	template<typename PRED>
	T WaitUntil ( PRED&& fnPred, int64_t iTime ) const;

	template<typename PRED>
	T WaitForMs ( PRED&& fnPred, int64_t iPeriodMs ) const;

	template<typename PRED>
	void WaitVoid ( PRED&& fnPred ) const;

	template<typename PRED>
	bool WaitVoidUntil ( PRED&& fnPred, int64_t iTime ) const;

	template<typename PRED>
	bool WaitVoidForMs ( PRED&& fnPred, int64_t iPeriodMs ) const;
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
	explicit ScopedScheduler_c ( SchedRole pRole ) ACQUIRE ( pRole );
	explicit ScopedScheduler_c ( RoledSchedulerSharedPtr_t& pRole ) ACQUIRE ( pRole );
	~ScopedScheduler_c () RELEASE();
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
	~CheckRole_c () RELEASE() = default;
};

using SccRL_t = CSphScopedRLock_T<Coro::RWLock_c>;
using SccWL_t = CSphScopedWLock_T<Coro::RWLock_c>;
using ScopedCoroMutex_t = Coro::ScopedMutex_t;

// fake locks
using FakeRL_t = FakeScopedRLock_T<Coro::RWLock_c>;
using FakeWL_t = FakeScopedWLock_T<Coro::RWLock_c>;

} // namespace Threads

#include "detail/coroutine_impl.h"
