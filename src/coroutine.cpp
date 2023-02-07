//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "coroutine.h"
#include "sphinxstd.h"
#include "task_info.h"
#include "coro_stack.h"
#include <atomic>

#include <boost/context/detail/fcontext.hpp>


#define LOG_LEVEL_DIAG false
#define LOG_LEVEL_DEBUGV false

namespace Threads {

#define LOG_COMPONENT_CORO "Stack: " << m_tStack.first.size << " (" << m_eState << ") "

bool StackMockingAllowed()
{
#if HAVE_VALGRIND_VALGRIND_H
	if (!!RUNNING_ON_VALGRIND)
		return false;
#endif
	return !val_from_env ( "NO_STACK_CALCULATION", false );
}


//////////////////////////////////////////////////////////////
/// Coroutine - uses boost::context to switch between jobs
using namespace boost::context::detail;
class CoRoutine_c
{
	enum class State_e
	{
		Paused, Running, Finished,
	};

	State_e m_eState = State_e::Paused;
	Handler m_fnHandler;
	CoroStack_t m_tStack;

	using Ctx_t = fcontext_t;
	Ctx_t m_tCoroutineContext;
	Ctx_t m_tExternalContext;
	friend StringBuilder_c & operator<< ( StringBuilder_c &, CoRoutine_c::State_e);

private:
	void WorkerLowest ( Ctx_t tCtx )
	{
		m_tExternalContext = tCtx;
		m_eState = State_e::Running;
		m_fnHandler ();
		m_eState = State_e::Finished;
		YieldLowest ();
	}

	// goto back to external context
	void YieldLowest ()
	{
		m_tExternalContext = jump_fcontext ( m_tExternalContext, nullptr ).fctx;
	}

	void CreateContext ( Handler fnHandler, CoroStack_t tStack )
	{
		m_fnHandler = std::move ( fnHandler );
		m_tStack = tStack;
		m_tCoroutineContext = make_fcontext ( m_tStack.first.sp, m_tStack.first.size, [] ( transfer_t pT ) {
			static_cast<CoRoutine_c *>(pT.data)->WorkerLowest ( pT.fctx );
		} );
	}

public:
	CoRoutine_c ( Handler fnHandler, CoroStack_t tStack )
	{
		CreateContext ( std::move ( fnHandler ), tStack );
	}

	~CoRoutine_c()
	{
		DeallocateStack ( m_tStack );
	}

	void Run ()
	{
		LOG( DEBUGV, CORO ) << "Run";
		assert ( m_eState==State_e::Paused );
		m_tCoroutineContext = jump_fcontext ( m_tCoroutineContext, static_cast<void*>(this) ).fctx;
	}

	bool IsFinished () const
	{
		return m_eState==State_e::Finished;
	}

	// yield to external context
	void Yield_ ()
	{
		LOG( DEBUGV, CORO ) << "Yield_";
		assert ( m_eState==State_e::Running );
		m_eState = State_e::Paused;
		YieldLowest ();
		m_eState = State_e::Running;
	}

	const BYTE* GetTopOfStack() const noexcept
	{
		return static_cast<const BYTE*> ( m_tStack.first.sp );
	}

	int GetStackSize() const noexcept
	{
		return m_tStack.first.size;
	}
};

StringBuilder_c & operator<< ( StringBuilder_c & dOut, CoRoutine_c::State_e eData )
{
	switch (eData) {
		case CoRoutine_c::State_e::Paused: return dOut << "Paused";
		case CoRoutine_c::State_e::Running: return dOut << "Running";
		case CoRoutine_c::State_e::Finished: return dOut << "Finished";
	}
	return dOut;
}


struct CoroState_t
{
	enum ESTATE : DWORD
	{
		Running_e = 1,		// if coro scheduled (and m.b. running) or not.
		Paused_e = 2,		// if set, will auto schedule again after yield
	};

	std::atomic<DWORD> m_uState { Running_e };

	// reset some flags and return previous state
	DWORD ResetFlags( DWORD uFlags )
	{
		return m_uState.fetch_and ( ~uFlags, std::memory_order_relaxed );
	}

	// set some flags and return previous state
	DWORD SetFlags ( DWORD uFlags )
	{
		return m_uState.fetch_or ( uFlags, std::memory_order_relaxed );
	}
};

#define LOG_COMPONENT_COROW "(" << m_tState.m_uState << ") "

namespace Coro {

class Worker_c : public details::SchedulerOperation_t
{
	// our executor (thread pool, etc which provides Schedule(handler) method)
	Scheduler_i * m_pScheduler = nullptr;
	Keeper_t m_tKeepSchedulerAlive;
	Waiter_t m_tTracer; // may be set to trace this worker lifetime

	// our main execution coroutine
	CoRoutine_c m_tCoroutine;
	CoroState_t	m_tState;
	Handler m_fnYieldWithProceeder = nullptr;

	// chain nested workers via TLS
	static thread_local Worker_c * m_pTlsThis;
	Worker_c * m_pPreviousWorker = nullptr;

	// operative stuff to be as near as possible
	void * m_pCurrentTaskInfo = nullptr;
	int64_t m_tmCpuTimeBase = 0; // add sphCpuTime() to this value to get truly cpu time ticks
	uint64_t m_uId { InitWorkerID() };
	std::atomic<size_t> m_iWakerEpoch { 0 };

	// support for periodical checks/throttling
	MiniTimer_c		m_tInternalTimer {"Worker"};
	int64_t			m_tmNextTimePointUS = 0;
	int64_t			m_tmRuntimePeriodUS = 0;
	int				m_iNumOfRestarts = 0;

	static uint64_t InitWorkerID()
	{
		static std::atomic<uint64_t> uWorker { 0ULL };
		return uWorker.fetch_add ( 1, std::memory_order_relaxed );
	}

	enum class TimePoint_e : bool { fromresume, realtime };
	inline void CheckEngageTimer ( TimePoint_e eKind )
	{
		if ( !m_tmRuntimePeriodUS )
			return;
		int64_t tmStartPointUS = ( eKind == TimePoint_e::realtime ) ? sph::MicroTimer() : MyThd().m_tmLastJobStartTimeUS;
		m_tmNextTimePointUS = tmStartPointUS + m_tmRuntimePeriodUS;
		m_tInternalTimer.EngageAt ( m_tmNextTimePointUS );
	}

	inline void CoroGuardEnter()
	{
		m_pPreviousWorker = std::exchange ( m_pTlsThis, this );
		m_pCurrentTaskInfo = MyThd().m_pTaskInfo.exchange ( m_pCurrentTaskInfo, std::memory_order_relaxed );
		m_tmCpuTimeBase -= sphThreadCpuTimer();
		++m_iNumOfRestarts;
		CheckEngageTimer( TimePoint_e::fromresume );
	}

	inline void CoroGuardFinishExit()
	{
		if ( m_tmRuntimePeriodUS )
			m_tInternalTimer.UnEngage();
		m_tmCpuTimeBase += sphThreadCpuTimer();
		m_pCurrentTaskInfo = MyThd().m_pTaskInfo.exchange ( m_pCurrentTaskInfo, std::memory_order_relaxed );
	}

	static inline void CoroGuardExit()
	{
		std::exchange ( m_pTlsThis, m_pTlsThis->m_pPreviousWorker ) -> CoroGuardFinishExit();
	}

	// RAII worker's keeper
	struct CoroGuard_t
	{
		explicit CoroGuard_t ( Worker_c * pWorker ) { pWorker->CoroGuardEnter(); }
		~CoroGuard_t () { Worker_c::CoroGuardExit(); }
	};

	static void DoComplete ( void* pOwner, SchedulerOperation_t* pBase )
	{
		if ( !pOwner )
			return;

		Threads::JobTracker_t dTrack;
		auto* pThis = static_cast<Worker_c*> ( pBase );
		pThis->Run();
	}

private:

	// called from StartPrimary from Coro::Go - Multiserve, Sphinxql, replication recv
	Worker_c ( Handler fnHandler, Scheduler_i* pScheduler )
		: SchedulerOperation_t ( &Worker_c::DoComplete )
		, m_pScheduler ( pScheduler )
		, m_tKeepSchedulerAlive { pScheduler->KeepWorking () }
		, m_tCoroutine { std::move (fnHandler), AllocateStack(0) }
		{
			assert ( m_pScheduler );
		}

	// from CallCoroutine - StartCall (blocking run);
	// from Coro::Co - StartOther - all secondary parallel things in non-vip queue
	// from Coro::Continue - StartContinuation - same context, another stack, run as fast as possible
	Worker_c ( Handler fnHandler, Scheduler_i* pScheduler, Waiter_t tTracer, size_t iStack=0 )
		: SchedulerOperation_t ( &Worker_c::DoComplete )
		, m_pScheduler ( pScheduler )
		, m_tKeepSchedulerAlive { pScheduler->KeepWorking () }
		, m_tTracer ( std::move(tTracer))
		, m_tCoroutine { std::move (fnHandler), AllocateStack(iStack) }
		{
			assert ( m_pScheduler );
		}

	// called solely for mocking - no scheduler, not possible to yield. Just provided stack and executor
	Worker_c ( Handler fnHandler, VecTraits_T<BYTE> dStack )
		: SchedulerOperation_t ( &Worker_c::DoComplete )
		, m_tCoroutine { std::move ( fnHandler ), MockedStack ( dStack ) }
	{
		assert ( !m_pScheduler );
	}

	inline void ResetRunningAndReschedule () noexcept
	{
		auto uPrevState = m_tState.m_uState.load ( std::memory_order_relaxed );
		do
			if ( uPrevState & CoroState_t::Paused_e )
			{
				LOG ( DIAG, COROW ) << "ResetRunningAndReschedule schedule because done";
				Schedule();
				return;
			}
		while ( !m_tState.m_uState.compare_exchange_weak ( uPrevState, uPrevState & ~CoroState_t::Running_e, std::memory_order_relaxed ) );

		if ( m_fnYieldWithProceeder )
			std::exchange ( m_fnYieldWithProceeder, nullptr )();
	}

	inline void Run() noexcept
	{
		if ( !Resume() ) // Resume() returns true when coro is finished
			ResetRunningAndReschedule();
	}

	inline void Schedule(bool bVip=true) noexcept
	{
		LOG ( DEBUGV, COROW ) << "Coro::Worker_c::Schedule (" << bVip << ", " << m_pScheduler << ")";
		assert ( m_pScheduler );
		m_pScheduler->ScheduleOp ( this, bVip ); // true means 'vip', false - 'secondary'
	}

	// continuation means, task is already started, and, if possible, should not be scheduled/paused.
	// That means, it might be call immediately, without placing it to queue. In recurrent call it may cause stack overflow by recursion -
	// that is Run(..Run(..Run..(... chain
	inline void ScheduleContinuation () noexcept
	{
		LOG ( DEBUGV, COROW ) << "Coro::Worker_c::ScheduleContinuation (" << m_pScheduler << ")";
		assert ( m_pScheduler );
		m_pScheduler->ScheduleContinuationOp (this);
	}

public:
	// invoked from Coro::Go - Multiserve, Sphinxql, replication recv. Schedule by default into primary queue.
	// Agnostic to parent's task info (if any). Should create and use it's own.
	static void StartPrimary ( Handler fnHandler, Scheduler_i* pScheduler, bool bPrimary=true )
	{
		( new Worker_c ( std::move ( fnHandler ), pScheduler ) )->Schedule ( bPrimary );
	}

	// from Coro::Co -> all parallel tasks (snippets, local searches, pq, disk chunks). Schedule into secondary queue.
	// May refer to parent's task info as read-only. For changes has dedicated mini info, also can create and use it's own local.
	static void StartOther ( Handler fnHandler, Scheduler_i * pScheduler, size_t iStack, Waiter_t tWait )
	{
		( new Worker_c ( myinfo::OwnMini ( std::move ( fnHandler ) ), pScheduler, std::move ( tWait ), iStack ) )->Schedule ( false );
	}

	// invoked from CallCoroutine -> ReplicationStart on daemon startup. Schedule into primary queue.
	// Adopt parent's task info (if any), and may change it exclusively.
	// Parent thread at the moment blocked and may display info about it
	static void StartCall ( Handler fnHandler, Scheduler_i* pScheduler, Waiter_t tWait )
	{
		( new Worker_c ( myinfo::StickParent ( std::move ( fnHandler ) ), pScheduler, std::move ( tWait ) ) )->Schedule ();
	}

	// from Coro::Continue -> all continuations (main purpose - continue with extended stack).
	// Adopt parent's task info (if any), and may change it exclusively.
	// Parent thread is not blocked and may freely switch to another tasks
	static void StartContinuation ( Handler fnHandler, Scheduler_i * pScheduler, size_t iStack, Waiter_t tWait )
	{
		( new Worker_c ( myinfo::StickParent ( std::move ( fnHandler ) ), pScheduler, std::move ( tWait ), iStack ) )->ScheduleContinuation ();
	}

	static void MockRun ( Handler fnHandler, VecTraits_T<BYTE> dStack )
	{
		Worker_c tAction ( std::move ( fnHandler ), dStack );
		auto pOldStack = Threads::TopOfStack ();
		Threads::SetTopStack ( &dStack.Last() );
		{
			CoroGuard_t pThis ( &tAction );
			tAction.m_tCoroutine.Run ();
		}
		Threads::SetTopStack ( pOldStack );
	}

	// secondary context worker.
	Worker_c* MakeWorker ( Handler fnHandler ) const
	{
		return new Worker_c ( myinfo::StickParent ( std::move ( fnHandler ) ), CurrentScheduler () );
	}

	inline void Restart ( bool bVip = true ) noexcept
	{
		if (( m_tState.SetFlags ( CoroState_t::Running_e ) & CoroState_t::Running_e )==0 )
			Schedule ( bVip );
	}

	inline void Continue () noexcept // continue previously run task. As continue calculation with extended stack
	{
		if ( ( m_tState.SetFlags ( CoroState_t::Running_e ) & CoroState_t::Running_e )==0 )
			ScheduleContinuation();
	}

	inline void Pause() noexcept
	{
		if ( ( m_tState.SetFlags ( CoroState_t::Running_e | CoroState_t::Paused_e ) & CoroState_t::Running_e ) == 0 )
			Schedule();
	}

	inline void Reschedule () noexcept
	{
		Pause();
		Yield_();
		m_tState.ResetFlags ( CoroState_t::Paused_e );
	}

	inline void Yield_ () noexcept
	{
		m_tCoroutine.Yield_ ();
	}

	inline void YieldWith ( Handler fnHandler ) noexcept
	{
		m_fnYieldWithProceeder = std::move (fnHandler);
		Yield_ ();
	}

	inline void MoveTo ( Scheduler_i * pScheduler ) noexcept
	{
		if ( m_pScheduler == pScheduler )
			return; // nothing to do

		m_pScheduler = pScheduler;
		Reschedule();
	}

	inline bool Resume () noexcept
	{
		{
			CoroGuard_t pThis (this);
			m_tCoroutine.Run();
		}
		if ( m_tCoroutine.IsFinished () )
		{
			delete this;
			return true;
		}
		return false;
	}

	inline Handler Restarter ( bool bVip = true ) noexcept
	{
		return [this, bVip] { Restart ( bVip ); };
	}

	inline Handler Continuator () noexcept
	{
		return [this] { Continue(); };
	}

	const BYTE * GetTopOfStack () const noexcept
	{
		return m_tCoroutine.GetTopOfStack();
	}

	int GetStackSize () const noexcept
	{
		return m_tCoroutine.GetStackSize ();
	}

	inline Scheduler_i * CurrentScheduler() const noexcept
	{
		return m_pScheduler;
	}

	inline int64_t GetCurrentCpuTimeBase() const noexcept
	{
		return m_tmCpuTimeBase;
	}

	inline uint64_t UID() const noexcept
	{
		return m_uId;
	}

	inline int ID() const noexcept
	{
		return (int)m_uId;
	}

	inline static Worker_c* CurrentWorker() noexcept
	{
		return m_pTlsThis;
	}

	inline bool Wake ( size_t iExpectedEpoch, bool bVip ) noexcept
	{
		bool bLastWaker = m_iWakerEpoch.compare_exchange_strong ( iExpectedEpoch, iExpectedEpoch + 1, std::memory_order_acq_rel );
		if ( !bLastWaker ) {
			// m_iWakerEpoch doesn't match expected epoch, op outdated
			return false;
		}

		assert ( CurrentWorker() != this );
		Restart ( bVip );
		return true;
	}

	inline Waker_c CreateWaker() noexcept
	{
		// this operation makes all previously created wakers to be outdated
		return { this, ++m_iWakerEpoch };
	}

	inline void SetTimePeriodUS ( int64_t tmTimePointPeriodUS )
	{
		m_tmRuntimePeriodUS = tmTimePointPeriodUS;
		CheckEngageTimer ( TimePoint_e::fromresume );
	}

	inline int64_t GetTimePeriodUS() const noexcept
	{
		return m_tmRuntimePeriodUS;
	}

	inline bool RuntimeExceeded() const
	{
		return sph::TimeExceeded ( m_tmNextTimePointUS );
	}

	inline const int64_t& GetNextTimePointUS() const noexcept
	{
		return m_tmNextTimePointUS;
	}

	inline int64_t GetRestPeriodUS() const noexcept
	{
		return m_tmNextTimePointUS - sph::MicroTimer();
	}

	inline void RestartRuntime() noexcept
	{
		CheckEngageTimer ( TimePoint_e::realtime );
	}

	inline int NumOfRestarts() const noexcept
	{
		return m_iNumOfRestarts;
	}
};

thread_local Worker_c * Worker_c::m_pTlsThis = nullptr;

Worker_c* CurrentWorker() noexcept
{
	return Worker_c::CurrentWorker();
}

Worker_c* Worker() noexcept
{
	auto pWorker = CurrentWorker();
	assert ( pWorker && "function must be called from inside coroutine" );
	return pWorker;
}

// start primary task
void Go ( Handler&& fnHandler, Scheduler_i * pScheduler )
{
	if ( !pScheduler )
		return;

	assert ( pScheduler );
	Worker_c::StartPrimary ( std::move(fnHandler), pScheduler );
}

// start secondary subtasks (parallel search, pq processing, etc)
void Co ( Handler&& fnHandler, Waiter_t tSignaller)
{
	auto pScheduler = CurrentScheduler ();
	if ( !pScheduler )
		pScheduler = GlobalWorkPool ();

	assert ( pScheduler );
	Worker_c::StartOther ( std::move ( fnHandler ), pScheduler, 0, std::move ( tSignaller ) );
}

// resize stack and continue as fast as possible (continuation task in same thread, or post to vip queue)
void Continue ( Handler&& fnHandler, int iStack )
{
	auto pScheduler = CurrentScheduler();
	if ( !pScheduler )
		pScheduler = GlobalWorkPool ();

	assert ( pScheduler );

	auto dWaiter = DefferedContinuator ();
	Worker_c::StartContinuation( std::move ( fnHandler ), pScheduler, iStack, dWaiter );
	WaitForDeffered ( std::move ( dWaiter ));
}

void YieldWith ( Handler fnHandler ) noexcept
{
	Worker ()->YieldWith ( std::move(fnHandler) );
}

// Move current task to another scheduler. Say, from netloop to worker, or from plain worker to vip, etc.
void MoveTo ( Scheduler_i * pScheduler ) noexcept
{
	Worker ()->MoveTo ( pScheduler );
}

void Yield_ () noexcept
{
	Worker ()->Yield_();
}

void Reschedule() noexcept
{
	auto pWorker = CurrentWorker();
	if ( pWorker )
		pWorker->Reschedule();
}

int ID() noexcept
{
	return Worker()->ID();
}

bool RuntimeExceeded() noexcept
{
	return Worker()->RuntimeExceeded();
}

const int64_t& GetNextTimePointUS() noexcept
{
	return Worker()->GetNextTimePointUS();
}

void RestartRuntime() noexcept
{
	Worker()->RestartRuntime();
}

int NumOfRestarts() noexcept
{
	return Worker()->NumOfRestarts();
}

} // namespace Coro

Resumer_fn MakeCoroExecutor ( Handler fnHandler )
{
	auto* pWorker = Coro::Worker ()->MakeWorker ( std::move ( fnHandler ) );
	return [pWorker] () -> bool { return pWorker->Resume(); };
}

void CallPlainCoroutine ( Handler fnHandler, Scheduler_i* pScheduler )
{
	if ( !pScheduler )
		pScheduler = GlobalWorkPool();
	CSphAutoEvent tEvent;
	auto dWaiter = Waiter_t ( nullptr, [&tEvent] ( void * ) { tEvent.SetEvent (); } );
	Coro::Worker_c::StartCall ( std::move ( fnHandler ), pScheduler, std::move(dWaiter) );
	tEvent.WaitEvent ();
}

void MockCallCoroutine ( VecTraits_T<BYTE> dStack, Handler fnHandler )
{
	Coro::Worker_c::MockRun ( std::move ( fnHandler ), dStack );
}

// if called from coroutine - just makes continuation.
// if called from plain thread (non-contexted) - creates coro, run and wait until it finished.
void CallCoroutine ( Handler fnHandler )
{
	if ( !Coro::Worker_c::CurrentWorker () )
	{
		CallPlainCoroutine ( std::move ( fnHandler ) );
		return;
	}
	auto pScheduler = Coro::CurrentScheduler ();
	if ( !pScheduler )
		pScheduler = GlobalWorkPool ();

	assert ( pScheduler );

	auto dWaiter = DefferedContinuator ();
	Coro::Worker_c::StartContinuation ( std::move ( fnHandler ), pScheduler, 0, dWaiter );
	WaitForDeffered ( std::move ( dWaiter ) );
}

bool CallCoroutineRes ( Predicate fnHandler )
{
	bool bResult;
	CallCoroutine ( [&bResult, &fnHandler] { bResult = fnHandler(); } );
	return bResult;
}

// start secondary subtasks (parallel search, pq processing, etc)
void StartJob ( Handler fnHandler )
{
	auto pScheduler = GlobalWorkPool();

	assert ( pScheduler );
	Coro::Worker_c::StartPrimary ( std::move ( fnHandler ), pScheduler, false );
}

// Async schedule continuation.
// Invoking handler will schedule continuation of yielded coroutine and return immediately.
// Scheduled task ('goto continue...') will be pefromed by scheduler's worker (threadpool, etc.)
// this pushes to primary queue
Handler CurrentRestarter ( bool bVip ) noexcept
{
	return Coro::Worker()->Restarter ( bVip );
}

Coro::Waker_c CreateWaker ( Coro::Worker_c* pWorker ) noexcept
{
	return pWorker ? pWorker->CreateWaker() : Coro::Worker()->CreateWaker();
}


Waiter_t DefferedRestarter () noexcept
{
	return { nullptr, [fnProceed = CurrentRestarter ( false )] ( void* ) { fnProceed(); } };
}

Waiter_t DefferedContinuator () noexcept
{
	return { nullptr, [fnProceed = Coro::Worker ()->Continuator ()] ( void * ) { fnProceed (); }};
}

void WaitForDeffered ( Waiter_t&& dWaiter ) noexcept
{
	// do nothing. Moved dWaiter will be released outside the coro after yield.
	Coro::YieldWith ( [capturedWaiter = std::move ( dWaiter ) ] {} );
}

int WaitForN ( DWORD uN, std::initializer_list<Handler> dTasks )
{
	assert ( uN > 0 && "trigger N must be >0" );
	assert ( dTasks.size() > 0 && dTasks.size() >= uN && "num of tasks to wait must be non-zero, and not greater than trigger" );
	int iRes = -1;

	// need to store parentSched, since Coro::YieldWith executed _outside_ coroutine, so it has _no_ scheduler!
	auto pParentSched = Coro::CurrentScheduler();

	Coro::YieldWith ( [&iRes, &dTasks, uN, pParentSched, fnResumer = CurrentRestarter()] {
		SharedPtr_t<std::atomic<DWORD>> pCounter { new std::atomic<DWORD> };
		pCounter->store ( 0, std::memory_order_release );
		int i = 0;
		for ( const auto& fnHandler : dTasks )
		{
			Coro::Go ( [pCounter, fnResumer, &fnHandler, i, uN, &iRes] {
				fnHandler();
				if ( pCounter->fetch_add ( 1, std::memory_order_acq_rel ) == uN - 1 )
				{
					iRes = i;
					fnResumer();
				}
			},
					pParentSched );
			++i;
		}
	} );
	return iRes;
}

}

int64_t sphTaskCpuTimer()
{
	auto pWorker = Threads::Coro::Worker_c::CurrentWorker();
	if ( pWorker )
		return pWorker->GetCurrentCpuTimeBase() + sphThreadCpuTimer();

	return sphThreadCpuTimer();
}

namespace Threads {

const void* MyStack()
{
	auto pWorker = Threads::Coro::Worker_c::CurrentWorker();
	if ( pWorker )
		return pWorker->GetTopOfStack();
	return Threads::TopOfStack();
}

int MyStackSize()
{
	auto pWorker = Threads::Coro::Worker_c::CurrentWorker();
	if ( pWorker )
		return pWorker->GetStackSize();
	return Threads::STACK_SIZE;
}

int64_t GetStackUsed()
{
	BYTE cStack;
	auto* pStackTop = (const BYTE*)MyStack();
	if ( !pStackTop )
		return 0;
	int64_t iHeight = pStackTop - &cStack;
	return ( iHeight >= 0 ) ? iHeight : -iHeight; // on different arch stack may grow in different directions
}


bool IsInsideCoroutine()
{
	// need safe function to call without coroutunes setup like in indexer
	return ( Coro::Worker_c::CurrentWorker() != nullptr );
}

Scheduler_i* Coro::CurrentScheduler()
{
	auto pWorker = Coro::Worker_c::CurrentWorker();
	if ( !pWorker )
		return nullptr;
	return pWorker->CurrentScheduler();
}

int NThreads()
{
	Scheduler_i* pScheduler = Coro::CurrentScheduler();
	if ( !pScheduler )
		pScheduler = GlobalWorkPool();
	return pScheduler->WorkingThreads();
}

namespace Coro
{

void ExecuteN ( int iConcurrency, Threads::Handler&& fnWorker )
{
	if ( iConcurrency==1 )
	{
		myinfo::OwnMiniNoCount ( fnWorker ) ();
		return;
	}

	assert ( iConcurrency ); // must be positive
	if ( !iConcurrency )	// however don't crash even there, just do nothing.
		return;

	auto dWaiter = DefferedRestarter ();
	for ( int i = 1; i<iConcurrency; ++i )
		Coro::Co ( Threads::WithCopiedCrashQuery ( fnWorker ), dWaiter );
	myinfo::OwnMiniNoCount ( fnWorker ) ();
	WaitForDeffered ( std::move ( dWaiter ));
}

int tmThrotleTimeQuantumMs = tmDefaultThrotleTimeQuantumMs; // how long task may work before rescheduling (in milliseconds)

void SetDefaultThrottlingPeriodMS ( int tmPeriodMs )
{
	tmThrotleTimeQuantumMs = tmPeriodMs < 0 ? tmDefaultThrotleTimeQuantumMs : tmPeriodMs;
}

void SetThrottlingPeriod ( int tmPeriodMs )
{
	if ( tmPeriodMs < 0 )
		tmPeriodMs = tmThrotleTimeQuantumMs;
	Worker()->SetTimePeriodUS ( tmPeriodMs * 1000 );
}

int64_t GetThrottlingPeriodUS ()
{
	return Worker()->GetTimePeriodUS ();
}

void RescheduleAndKeepCrashQuery()
{
	CrashQueryKeeper_c _;
	Coro::Worker()->Reschedule(); // timer will be automatically re-engaged on resume
}

inline void fnResume ( volatile Worker_c* pCtx )
{
	if ( pCtx )
		( (Worker_c*)pCtx )->Restart ( false );
}

// yield and reschedule after given period of time (in milliseconds)
void SleepMsec ( int iMsec )
{
	if ( iMsec < 0 )
		return;

	MiniTimer_c tWait { "SleepMsec", [tWaker = Worker()->CreateWaker()] { tWaker.Wake(); } };

	// suspend this fiber
	Coro::YieldWith ( [&tWait, iMsec](){ tWait.Engage ( iMsec ); });
}

Event_c::~Event_c ()
{
	// edge case: event destroyed being in wait state.
	// Let's resume then.
	if ( m_pCtx && (m_uState.load() & Waited_e)!=0 )
		fnResume ( m_pCtx );
}

// If 'waited' state detected, resume waiter.
// else atomically set flag 'signaled'
void Event_c::SetEvent()
{
	BYTE uState = m_uState.load ( std::memory_order_relaxed );
	do
	{
		if ( uState & Waited_e )
		{
			m_uState.store ( Signaled_e ); // memory_order_sec_cst - to ensure that next call will not resume again
			return fnResume ( m_pCtx );
		}
	} while ( !m_uState.compare_exchange_weak ( uState, uState | Signaled_e, std::memory_order_relaxed ) );
}

// if 'signaled' state detected, clean all flags and return.
// else yield, then (out of coro) atomically set 'waited' flag, checking also 'signaled' flag again.
// on resume clean all flags.
void Event_c::WaitEvent()
{
	if ( !( m_uState.load ( std::memory_order_relaxed ) & Signaled_e ) )
	{
		if ( m_pCtx != Worker() )
			m_pCtx = Worker();
		YieldWith ( [this] {
			BYTE uState = m_uState.load ( std::memory_order_relaxed );
			do
			{
				if ( uState & Signaled_e )
					return fnResume ( m_pCtx );
			} while ( !m_uState.compare_exchange_weak ( uState, uState | Waited_e, std::memory_order_relaxed ) );
		} );
	}

	m_uState.store ( 0, std::memory_order_relaxed );
	std::atomic_thread_fence ( std::memory_order_release );
}


bool Waker_c::Wake ( bool bVip ) const noexcept
{
	assert ( m_iEpoch > 0 );
	assert ( m_pCtx );
	return m_pCtx->Wake ( m_iEpoch, bVip );
}

bool AtomicWaker_c::WakeOnce ( bool bVip ) noexcept
{
	auto pCtx = m_pCtx.exchange ( nullptr, std::memory_order_relaxed );
	if ( !pCtx )
		return false;
	assert ( m_iEpoch > 0 );
	return pCtx->Wake ( m_iEpoch, bVip );
}


void WaitQueue_c::SuspendAndWait ( sph::Spinlock_lock& tLock, Worker_c* pActive ) NO_THREAD_SAFETY_ANALYSIS
{
	WakerInQueue_c w { pActive->CreateWaker() };
	m_Slist.push_back ( w );
	// suspend this fiber
	pActive->YieldWith ( [&tLock]() NO_THREAD_SAFETY_ANALYSIS { tLock.unlock(); } );
	assert ( !w.is_linked() );
}

// returns true if signalled, false if timed-out
bool WaitQueue_c::SuspendAndWaitUntil ( sph::Spinlock_lock& tLock, Worker_c* pActive, int64_t iTimestamp ) NO_THREAD_SAFETY_ANALYSIS
{
	WakerInQueue_c w { pActive->CreateWaker() };
	m_Slist.push_back ( w );

	MiniTimer_c tWait ( "SuspendAndWait", [&w] { w.Wake(); } );

	// suspend this fiber
	pActive->YieldWith ( [&tLock, &tWait, iTimestamp]() NO_THREAD_SAFETY_ANALYSIS {
		tLock.unlock();
		tWait.EngageAt ( iTimestamp );
	});

	// resumed. Check if deadline is reached
	if ( sph::TimeExceeded ( iTimestamp ) )
	{
		tLock.lock();
		// remove from waiting-queue
		if ( w.is_linked() )
			m_Slist.remove ( w );
		tLock.unlock();
		return false;
	}
	return true;
}

bool WaitQueue_c::SuspendAndWaitForMS ( sph::Spinlock_lock& tLock, Worker_c* pActive, int64_t iTimePeriodMS )
{
	return SuspendAndWaitUntil ( tLock, pActive, sphMicroTimer() + iTimePeriodMS * 1000 );
}


void WaitQueue_c::NotifyOne()
{
	while ( !m_Slist.empty() ) {
		Waker_c& w = m_Slist.front();
		m_Slist.pop_front();
		if ( w.Wake() ) {
			break;
		}
	}
}

void WaitQueue_c::NotifyAll()
{
	while ( !m_Slist.empty() ) {
		Waker_c& w = m_Slist.front();
		m_Slist.pop_front();
		w.Wake();
	}
}

bool WaitQueue_c::Empty() const
{
	return m_Slist.empty();
}


// the mutex
void Mutex_c::Lock()
{
	while ( true ) {
		auto* pActiveWorker = Worker();
		// store this fiber in order to be notified later
		sph::Spinlock_lock tLock { m_tWaitQueueSpinlock };
		assert ( pActiveWorker != m_pOwner );
		if ( !m_pOwner ) {
			m_pOwner = pActiveWorker;
			return;
		}

		m_tWaitQueue.SuspendAndWait( tLock, pActiveWorker );
	}
}

void Mutex_c::Unlock()
{
	Debug (auto* pActiveWorker = Worker();)
	sph::Spinlock_lock tLock { m_tWaitQueueSpinlock };
	assert ( pActiveWorker == m_pOwner );
	m_pOwner = nullptr;

	m_tWaitQueue.NotifyOne();
}

// conditional variable
void ConditionVariableAny_c::NotifyOne() noexcept
{
	sph::Spinlock_lock tLock { m_tWaitQueueSpinlock };
	m_tWaitQueue.NotifyOne();
}

void ConditionVariableAny_c::NotifyAll() noexcept
{
	sph::Spinlock_lock tLock { m_tWaitQueueSpinlock };
	m_tWaitQueue.NotifyAll();
}

// the shared mutex

// practice shows few numbers are more expressive than symbolic names in that case
static constexpr DWORD ux01 = 1;			   // w-lock acquired
static constexpr DWORD ux02 = 2;			   // r-lock bias
Debug (static constexpr DWORD uxFE = ~ux01;)		   // mask for w-lock flag (0xFFFFFFFE)

// prefer w-lock: acquire only if w-lock queue is empty
void RWLock_c::ReadLock()
{
	while ( true ) {
		// store this task in order to be resumed later
		sph::Spinlock_lock tLock { m_tInternalMutex };
		if ( !( m_uState & ux01 ) && m_tWaitWQueue.Empty() )
		{
			m_uState += ux02;
			assert ( ( m_uState & uxFE ) != uxFE ); // hope, 31 bits is enough to count all acquired r-locks.
			return;
		}
		m_tWaitRQueue.SuspendAndWait ( tLock, Worker() );
	}
}

void RWLock_c::WriteLock()
{
	while ( true ) {
		sph::Spinlock_lock tLock { m_tInternalMutex };
		if ( !m_uState )
		{
			m_uState = ux01;
			return;
		}
		m_tWaitWQueue.SuspendAndWait ( tLock, Worker() );
	}
}

void RWLock_c::Unlock()
{
	sph::Spinlock_lock tLock { m_tInternalMutex };
	assert ( m_uState >= ux01 && "attempt to unlock not locked coro mutex");
	m_uState = ( m_uState == ux01 ) ? 0 : ( m_uState - ux02 );

	if ( m_uState )
		return;

	// fixme! point of priority decision: on unlock, do we need to wake up next w-locker, or all r-lockers?
	if ( !m_tWaitWQueue.Empty() )
		m_tWaitWQueue.NotifyOne();
	else
		m_tWaitRQueue.NotifyAll();
}

} // namespace Coro
} // namespace Threads
