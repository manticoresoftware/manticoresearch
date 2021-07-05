//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
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
#include <atomic>

#define BOOST_USE_VALGRIND 1
#ifndef NDEBUG
#define BOOST_USE_VALGRIND 1
#endif

#ifdef BOOST_USE_VALGRIND
#ifndef HAVE_VALGRIND
#undef BOOST_USE_VALGRIND
#endif
#endif

#include <boost/context/detail/fcontext.hpp>

#if BOOST_USE_VALGRIND
#include <valgrind/valgrind.h>
#endif

#define LOG_LEVEL_DIAG false
#define LOG_LEVEL_DEBUGV false

namespace Threads {

static const size_t STACK_ALIGN = 16; // stack align - let it be 16 bytes for convenience
#define LOG_COMPONENT_CORO "Stack: " << m_dStack.GetLength() << " (" << m_eState << ") "

size_t AlignStackSize ( size_t iSize )
{
	return ( iSize+STACK_ALIGN-1 ) & ~( STACK_ALIGN-1 );
}

// stack size - 128K
static const size_t DEFAULT_CORO_STACK_SIZE = 1024 * 128;

size_t GetDefaultCoroStackSize()
{
	return DEFAULT_CORO_STACK_SIZE;
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
	VecTraits_T<BYTE> m_dStack;
	CSphFixedVector<BYTE> m_dStackStorage {0};

#if BOOST_USE_VALGRIND
	unsigned m_uValgrindStackID = 0;
#endif

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

	inline void ValgrindRegisterStack()
	{
#if BOOST_USE_VALGRIND
		if ( !m_dStackStorage.IsEmpty ())
			m_uValgrindStackID = VALGRIND_STACK_REGISTER( m_dStackStorage.begin (), &m_dStackStorage.Last ());
#endif
	}

	inline void ValgrindDeregisterStack ()
	{
#if BOOST_USE_VALGRIND
		if ( !m_dStackStorage.IsEmpty ())
			VALGRIND_STACK_DEREGISTER( m_uValgrindStackID );
#endif
	}

	void CreateContext ( Handler fnHandler, VecTraits_T<BYTE> dStack )
	{
		m_fnHandler = std::move ( fnHandler );
		m_dStack = dStack;
		ValgrindRegisterStack ();
		m_tCoroutineContext = make_fcontext ( &m_dStack.Last (), m_dStack.GetLength (), [] ( transfer_t pT ) {
			static_cast<CoRoutine_c *>(pT.data)->WorkerLowest ( pT.fctx );
		} );
	}

public:
	explicit CoRoutine_c ( Handler fnHandler, size_t iStack=0 )
		: m_dStackStorage ( iStack ? (int) AlignStackSize ( iStack ) : DEFAULT_CORO_STACK_SIZE )
	{
		CreateContext ( std::move ( fnHandler ), m_dStackStorage );
	}

	CoRoutine_c ( Handler fnHandler, VecTraits_T<BYTE> dStack )
	{
		CreateContext ( std::move ( fnHandler ), dStack );
	}

#if BOOST_USE_VALGRIND
	~CoRoutine_c()
	{
		ValgrindDeregisterStack ();
	}
#endif

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

	BYTE* GetTopOfStack() const
	{
		return m_dStack.End();
	}

	int GetStackSize() const
	{
		return m_dStack.GetLength();
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
		Entered_e = 1,	// if coro scheduled (and m.b. running) or not.
		Done_e = 2,		// if set, will auto schedule again after yield
	};

	std::atomic<DWORD> m_uState {Entered_e};

	bool IsDone ()
	{
		auto uState = m_uState.load ( std::memory_order_relaxed );
		return ( uState & Done_e )!=0;
	}

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


class CoroWorker_c
{
	// our executor (thread pool, etc which provides Schedule(handler) method)
	Scheduler_i * m_pScheduler = nullptr;
	Keeper_t m_tKeepSchedulerAlive;
	Waiter_t m_tTracer; // may be set to trace this worker lifetime

	// our main execution coroutine
	CoRoutine_c m_tCoroutine;
	CoroState_t	m_tState;
	Handler 	m_fnProceeder = nullptr;

	// chain nested workers via TLS
	static thread_local CoroWorker_c * m_pTlsThis;
	CoroWorker_c * m_pPreviousWorker = nullptr;

	// operative stuff to be as near as possible
	void * m_pCurrentTaskInfo = nullptr;
	int64_t m_tmCpuTimeBase = 0; // add sphCpuTime() to this value to get truly cpu time ticks

	// RAII worker's keeper
	struct CoroGuard_t
	{
		explicit CoroGuard_t ( CoroWorker_c * pWorker )
		{
			pWorker->m_pPreviousWorker = CoroWorker_c::m_pTlsThis;
			CoroWorker_c::m_pTlsThis = pWorker;
			pWorker->m_pCurrentTaskInfo =
					MyThd ().m_pTaskInfo.exchange ( pWorker->m_pCurrentTaskInfo, std::memory_order_relaxed );
			pWorker->m_tmCpuTimeBase -= sphCpuTimer();
		}

		~CoroGuard_t ()
		{
			auto pWork = CoroWorker_c::m_pTlsThis;
			pWork->m_tmCpuTimeBase += sphCpuTimer ();
			pWork->m_pCurrentTaskInfo = MyThd ().m_pTaskInfo.exchange ( pWork->m_pCurrentTaskInfo, std::memory_order_relaxed );
			CoroWorker_c::m_pTlsThis = pWork->m_pPreviousWorker;
		}
	};

private:

	// called from StartPrimary from CoGo - Multiserve, Sphinxql, replication recv
	CoroWorker_c ( Handler fnHandler, Scheduler_i* pScheduler )
		: m_pScheduler ( pScheduler )
		, m_tKeepSchedulerAlive { pScheduler->KeepWorking () }
		, m_tCoroutine { std::move (fnHandler), 0 }
		{
			assert ( m_pScheduler );
		}

	// from CallCoroutine - StartCall (blocking run);
	// from CoCo - StartOther - all secondary parallel things in non-vip queue
	// from Cocontinue - StartContinuation - same context, another stack, run as fast as possible
	CoroWorker_c ( Handler fnHandler, Scheduler_i* pScheduler, Waiter_t tTracer, size_t iStack=0 )
		: m_pScheduler ( pScheduler )
		, m_tKeepSchedulerAlive { pScheduler->KeepWorking () }
		, m_tTracer ( std::move(tTracer))
		, m_tCoroutine { std::move (fnHandler), iStack }
		{
			assert ( m_pScheduler );
		}

	// called solely for mocking - no scheduler, not possible to yield. Just provided stack and executor
	CoroWorker_c ( Handler fnHandler, VecTraits_T<BYTE> dStack )
		: m_tCoroutine { std::move ( fnHandler ), dStack }
	{
		assert ( !m_pScheduler );
	}

	void Run ()
	{
		if ( !Resume () )
			ResetEnteredAndReschedule ();
	}

	void ResetEnteredAndReschedule ()
	{
		auto uPrevState = m_tState.m_uState.load ( std::memory_order_relaxed );
		do
			if (( uPrevState & CoroState_t::Done_e )!=0 )
			{
				LOG ( DIAG, COROW ) << "ResetEnteredAndReschedule schedule because done";
				Schedule();
				return;
			}
		while (!m_tState.m_uState.compare_exchange_weak (
				uPrevState, uPrevState & ~CoroState_t::Entered_e, std::memory_order_relaxed ));
		///^ lock-free primitive. If m_tState.m_uState==uPrevState here, write new value with unset Entered_e.
		/// if not (another thread changed value before) - exec uPrevState=m_tState.m_uState and loop.
		if ( m_fnProceeder )
		{
			Handler fnProceeder = nullptr;
			Swap ( fnProceeder, m_fnProceeder );
			fnProceeder();
		}
	}

	void Schedule(bool bVip=true)
	{
		LOG ( DEBUGV, COROW ) << "CoroWorker_c::Schedule (" << bVip << ", " << m_pScheduler << ")";
		assert ( m_pScheduler );
		m_pScheduler->Schedule ( [this] { Run (); }, bVip ); // true means 'vip', false - 'secondary'
	}

	void ScheduleContinuation ()
	{
		LOG ( DEBUGV, COROW ) << "CoroWorker_c::ScheduleContinuation (" << m_pScheduler << ")";
		assert ( m_pScheduler );
		m_pScheduler->ScheduleContinuation( [this] { Run (); } );
	}

public:
	// invoked from CoGo - Multiserve, Sphinxql, replication recv. Schedule into primary queue.
	// Agnostic to parent's task info (if any). Should create and use it's own.
	static void StartPrimary ( Handler fnHandler, Scheduler_i* pScheduler )
	{
		( new CoroWorker_c ( std::move ( fnHandler ), pScheduler ) )->Schedule ();
	}

	// from CoCo -> all parallel tasks (snippets, local searches, pq, disk chunks). Schedule into secondary queue.
	// May refer to parent's task info as read-only. For changes has dedicated mini info, also can create and use it's own local.
	static void StartOther ( Handler fnHandler, Scheduler_i * pScheduler, size_t iStack, Waiter_t tWait )
	{
		( new CoroWorker_c ( myinfo::OwnMini ( std::move ( fnHandler ) ), pScheduler, std::move ( tWait ), iStack ) )->Schedule ( false );
	}

	// invoked from CallCoroutine -> ReplicationStart on daemon startup. Schedule into primary queue.
	// Adopt parent's task info (if any), and may change it exclusively.
	// Parent thread at the moment blocked and may display info about it
	static void StartCall ( Handler fnHandler, Scheduler_i* pScheduler, Waiter_t tWait )
	{
		( new CoroWorker_c ( myinfo::StickParent ( std::move ( fnHandler ) ), pScheduler, std::move ( tWait ) ) )->Schedule ();
	}

	// from CoContinue -> all continuations (main purpose - continue with extended stack).
	// Adopt parent's task info (if any), and may change it exclusively.
	// Parent thread is not blocked and may freely switch to another tasks
	static void StartContinuation ( Handler fnHandler, Scheduler_i * pScheduler, size_t iStack, Waiter_t tWait )
	{
		( new CoroWorker_c ( myinfo::StickParent ( std::move ( fnHandler ) ), pScheduler, std::move ( tWait ), iStack ) )->ScheduleContinuation ();
	}

	static void MockRun ( Handler fnHandler, VecTraits_T<BYTE> dStack )
	{
		CoroWorker_c tAction ( std::move ( fnHandler ), dStack );
		auto pOldStack = Threads::TopOfStack ();
		Threads::SetTopStack ( &dStack.Last() );
		{
			CoroGuard_t pThis ( &tAction );
			tAction.m_tCoroutine.Run ();
		}
		Threads::SetTopStack ( pOldStack );
	}

	// secondary context worker.
	CoroWorker_c* MakeWorker ( Handler fnHandler ) const
	{
		return new CoroWorker_c ( myinfo::StickParent ( std::move ( fnHandler ) ), CurrentScheduler () );
	}

	void Restart ()
	{
		if (( m_tState.SetFlags ( CoroState_t::Entered_e ) & CoroState_t::Entered_e )==0 )
			Schedule ();
	}

	void RestartSecondary ()
	{
		if (( m_tState.SetFlags ( CoroState_t::Entered_e ) & CoroState_t::Entered_e )==0 )
			Schedule (false);
	}

	void Continue () // continue previously run task. As continue calculation with extended stack
	{
		if ( ( m_tState.SetFlags ( CoroState_t::Entered_e ) & CoroState_t::Entered_e )==0 )
			ScheduleContinuation();
	}

	void Done()
	{
		if (( m_tState.SetFlags ( CoroState_t::Entered_e | CoroState_t::Done_e ) & CoroState_t::Entered_e )==0 )
			Schedule();
	}

	void Reschedule ()
	{
		Done();
		Yield_();
		m_tState.ResetFlags ( CoroState_t::Done_e );
	}

	void Yield_ ()
	{
		m_tCoroutine.Yield_ ();
	}

	void YieldWith ( Handler fnHandler )
	{
		m_fnProceeder = std::move (fnHandler);
		Yield_ ();
	}

	void MoveTo ( Scheduler_i * pScheduler )
	{
		if ( m_pScheduler == pScheduler )
			return; // nothing to do

		m_pScheduler = pScheduler;
		Reschedule();
	}

	bool Resume ()
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

	Handler SecondaryRestarter()
	{
		return [this] { RestartSecondary (); };
	}

	Handler Restarter ()
	{
		return [this] { Restart (); };
	}

	Handler Continuator ()
	{
		return [this] { Continue(); };
	}

	BYTE * GetTopOfStack () const
	{
		return m_tCoroutine.GetTopOfStack();
	}

	int GetStackSize () const
	{
		return m_tCoroutine.GetStackSize ();
	}

	Scheduler_i * CurrentScheduler() const
	{
		return m_pScheduler;
	}

	int64_t GetCurrentCpuTimeBase() const
	{
		return m_tmCpuTimeBase;
	}

	static CoroWorker_c* CurrentWorker()
	{
		return m_pTlsThis;
	}
};

thread_local CoroWorker_c * CoroWorker_c::m_pTlsThis = nullptr;

static CoroWorker_c * CoWorker()
{
	auto pWorker = CoroWorker_c::CurrentWorker();
	assert ( pWorker && "function must be called from inside coroutine");
	return pWorker;
}

// start primary task
void CoGo ( Handler fnHandler, Scheduler_i * pScheduler )
{
	if ( !pScheduler )
		return;

	assert ( pScheduler );
	CoroWorker_c::StartPrimary ( std::move(fnHandler), pScheduler );
}

// start secondary subtasks (parallell search, pq processing, etc)
void CoCo ( Handler fnHandler, Waiter_t tSignaller)
{
	auto pScheduler = CoCurrentScheduler ();
	if ( !pScheduler )
		pScheduler = GlobalWorkPool ();

	assert ( pScheduler );
	CoroWorker_c::StartOther ( std::move ( fnHandler ), pScheduler, 0, std::move ( tSignaller ) );
}

// resize stack and continue as fast as possible (continuation task in same thread, or post to vip queue)
void CoContinue ( Handler fnHandler, int iStack )
{
	auto pScheduler = CoCurrentScheduler();
	if ( !pScheduler )
		pScheduler = GlobalWorkPool ();

	assert ( pScheduler );

	auto dWaiter = DefferedContinuator ();
	CoroWorker_c::StartContinuation( std::move ( fnHandler ), pScheduler, iStack, dWaiter );
	WaitForDeffered ( std::move ( dWaiter ));
}

static void CallPlainCoroutine ( Handler fnHandler )
{
	auto pScheduler = GlobalWorkPool ();
	CSphAutoEvent tEvent;
	auto dWaiter = Waiter_t ( nullptr, [&tEvent] ( void * ) { tEvent.SetEvent (); } );
	CoroWorker_c::StartCall ( std::move ( fnHandler ), pScheduler, std::move(dWaiter) );
	tEvent.WaitEvent ();
}

void MockCallCoroutine ( VecTraits_T<BYTE> dStack, Handler fnHandler )
{
	CoroWorker_c::MockRun ( std::move ( fnHandler ), dStack );
}

// if called from coroutine - just makes continuation.
// if called from plain thread (non-contexted) - creates coro, run and wait until it finished.
void CallCoroutine ( Handler fnHandler )
{
	if ( !CoroWorker_c::CurrentWorker () )
	{
		CallPlainCoroutine(std::move(fnHandler));
		return;
	}
	auto pScheduler = CoCurrentScheduler ();
	if ( !pScheduler )
		pScheduler = GlobalWorkPool ();

	assert ( pScheduler );

	auto dWaiter = DefferedContinuator ();
	CoroWorker_c::StartContinuation ( std::move ( fnHandler ), pScheduler, 0, dWaiter );
	WaitForDeffered ( std::move ( dWaiter ) );
}

bool CallCoroutineRes ( Predicate fnHandler )
{
	bool bResult;
	CallCoroutine ( [&bResult, &fnHandler] { bResult = fnHandler(); } );
	return bResult;
}

void CoYieldWith ( Handler fnHandler )
{
	CoWorker ()->YieldWith ( std::move(fnHandler) );
}

// Move current task to another scheduler. Say, from netloop to worker, or from plain worker to vip, etc.
void CoMoveTo ( Scheduler_i * pScheduler )
{
	CoWorker ()->MoveTo ( pScheduler );
}

void CoYield ()
{
	CoWorker ()->Yield_();
}

Resumer_fn MakeCoroExecutor ( Handler fnHandler )
{
	auto* pWorker = CoWorker ()->MakeWorker ( std::move ( fnHandler ) );
	return [pWorker] () -> bool { return pWorker->Resume(); };
}


// Async schedule continuation.
// Invoking handler will schedule continuation of yielded coroutine and return immediately.
// Scheduled task ('goto continue...') will be pefromed by scheduler's worker (threadpool, etc.)
// this pushes to primary queue
Handler CurrentRestarter ()
{
	return CoWorker()->Restarter();
}


Waiter_t DefferedRestarter ()
{
	return Waiter_t ( nullptr, [fnProceed= CoWorker ()->SecondaryRestarter ()] ( void * )
	{
		fnProceed ();
	} );
}

Waiter_t DefferedContinuator ()
{
	return Waiter_t ( nullptr, [fnProceed = CoWorker ()->Continuator ()] ( void * ) {
		fnProceed ();
	} );
}

void WaitForDeffered ( Waiter_t&& dWaiter )
{
	// do nothing. Moved dWaiter will be released outside the coro after yield.
	CoYieldWith ( [capturedWaiter = std::move ( dWaiter ) ] {} );
}
}

const void * sphMyStack ()
{
	auto pWorker = Threads::CoroWorker_c::CurrentWorker ();
	if ( pWorker )
		return pWorker->GetTopOfStack ();
	return Threads::TopOfStack();

}

int sphMyStackSize ()
{
	auto pWorker = Threads::CoroWorker_c::CurrentWorker ();
	if ( pWorker )
		return pWorker->GetStackSize ();
	return Threads::STACK_SIZE;
}

int64_t sphTaskCpuTimer ()
{
	auto pWorker = Threads::CoroWorker_c::CurrentWorker ();
	if ( pWorker )
		return pWorker->GetCurrentCpuTimeBase ()+sphCpuTimer ();

	return sphCpuTimer();
}

Threads::Scheduler_i * Threads::CoCurrentScheduler ()
{
	auto pWorker = Threads::CoroWorker_c::CurrentWorker ();
	if ( !pWorker )
		return nullptr;
	return pWorker->CurrentScheduler ();
}

int Threads::NThreads ( Scheduler_i * pScheduler )
{
	if ( !pScheduler )
		pScheduler = GlobalWorkPool ();
	return pScheduler->WorkingThreads ();
}

void Threads::CoExecuteN ( int iConcurrency, Threads::Handler&& fnWorker )
{
	if ( iConcurrency==1 )
	{
		myinfo::OwnMini ( fnWorker ) ();
		return;
	}

	assert ( iConcurrency ); // must be positive
	if ( !iConcurrency )	// however don't crash even there, just do nothing.
		return;

	auto dWaiter = DefferedRestarter ();
	for ( int i = 1; i<iConcurrency; ++i )
		CoCo ( Threads::WithCopiedCrashQuery ( fnWorker ), dWaiter );
	myinfo::OwnMini ( fnWorker ) ();
	WaitForDeffered ( std::move ( dWaiter ));
}

int Threads::CoThrottler_c::tmThrotleTimeQuantumMs = Threads::tmDefaultThrotleTimeQuantumMs;

Threads::CoThrottler_c::CoThrottler_c ( int tmThrottlePeriodMs )
	: m_tmThrottlePeriodMs ( tmThrottlePeriodMs )
{
	if ( tmThrottlePeriodMs<0 )
		m_tmThrottlePeriodMs = tmThrotleTimeQuantumMs;

	if ( m_tmThrottlePeriodMs )
		m_tmNextThrottleTimestamp = m_dTimerGuard.MiniTimerEngage( m_tmThrottlePeriodMs );
}

bool Threads::CoThrottler_c::MaybeThrottle ()
{
	if ( !m_tmThrottlePeriodMs || !sph::TimeExceeded ( m_tmNextThrottleTimestamp ) )
		return false;

	m_tmNextThrottleTimestamp = m_dTimerGuard.MiniTimerEngage ( m_tmThrottlePeriodMs );
	auto iOldThread = MyThd ().m_iThreadID;
	CoWorker ()->Reschedule ();
	m_bSameThread = ( iOldThread==MyThd ().m_iThreadID );
	return true;
}

bool Threads::CoThrottler_c::ThrottleAndKeepCrashQuery ()
{
	if ( !m_tmThrottlePeriodMs || !sph::TimeExceeded ( m_tmNextThrottleTimestamp ) )
		return false;

	m_tmNextThrottleTimestamp = m_dTimerGuard.MiniTimerEngage ( m_tmThrottlePeriodMs );
	CrashQueryKeeper_c _;
	auto iOldThread = MyThd ().m_iThreadID;
	CoWorker ()->Reschedule ();
	m_bSameThread = ( iOldThread==MyThd ().m_iThreadID );
	return true;
}

inline void fnResume ( volatile void* pCtx )
{
	if (!pCtx)
		return;
	( (Threads::CoroWorker_c *) pCtx )->RestartSecondary ();
}

Threads::CoroEvent_c::~CoroEvent_c ()
{
	// edge case: event destroyed being in wait state.
	// Let's resume then.
	if ( m_pCtx && (m_uState.load() & Waited_e)!=0 )
		fnResume ( m_pCtx );
}

// If 'waited' state detected, resume waiter.
// else atomically set flag 'signaled'
void Threads::CoroEvent_c::SetEvent()
{
	DWORD uState = m_uState.load ( std::memory_order_relaxed );
	do
	{
		if ( uState & Waited_e )
		{
			m_uState.store (1); // memory_order_sec_cst - to ensure that next call will not resume again
			fnResume ( m_pCtx );
			return;
		}
	} while ( !m_uState.compare_exchange_weak ( uState, uState | Signaled_e, std::memory_order_relaxed ) );
}

// if 'signaled' state detected, clean all flags and return.
// else yield, then (out of coro) atomically set 'waited' flag, checking also 'signaled' flag again.
// on resume clean all flags.
void Threads::CoroEvent_c::WaitEvent()
{
	if ( !( m_uState.load ( std::memory_order_relaxed ) & Signaled_e ) )
	{
		if ( m_pCtx!= CoWorker() )
			m_pCtx = CoWorker();
		CoYieldWith ( [this] {
			DWORD uState = m_uState.load ( std::memory_order_relaxed );
			do
			{
				if ( uState & Signaled_e )
				{
					fnResume ( m_pCtx );
					return;
				}
			} while ( !m_uState.compare_exchange_weak ( uState, uState | Waited_e, std::memory_order_relaxed ) );
		} );
	}

	m_uState.store ( 0, std::memory_order_relaxed );
	std::atomic_thread_fence ( std::memory_order_release );
}

bool Threads::IsInsideCoroutine ()
{
	// need safe function to call without coroutunes setup like in indexer
	return ( CoroWorker_c::CurrentWorker()!=nullptr );
}

/*
 * Legend for m_uLock:
 * 1-st bit indicated w-lock pending, 2-nd - w-lock, others are the counter of acquired shared locks.
 * using low bits for flags ensures that counter will never touch them even on overflow.
  *
 * Possible values:
 * 0x00 - totally unlocked.
 * 0x84 - 33 r-locks acquired, may acquire and release other shared locks.
 * 0x85 - 33 r-locks acquired, and w-lock is pending. Will NOT acquire more r-locks, just release existing.
 * 0x02 - w-lock acquired. May be only released.
 * 0x03 - w-lock acquired, another w-lock pending.
 *
 * Pending bit is set during w-lock acquiring. On success w-lock bit is set, and pending restored to previous state.
 * Releasing (unlock) never changes pending bit.
 *
 * When several w-locks enqueued, there is gaps between them when r-locks may steal the focus.
 *
 * Rlock may be transformed to wlock: we need to set pending bit, then wait when only one r-lock left. Then set
 * atomically exchange last counter of r-lock to w-lock
 *
 */

// practice shows few numbers are more expressive then symbolic names in that case
static const DWORD ux01 = 1; // w-lock pending
static const DWORD ux02 = 2; // w-lock acquired
static const DWORD ux04 = 4; // increment for r-lock to avoid touching w-lock
static const DWORD ux03 = ( ux01 | ux02 ); // acquiring read-locks paused
static const DWORD uxFC = ~ux03; // mask for pending and wlock flags (0xFFFFFFFC)
static const DWORD uxFE = ~ux01; // mask for pending flag flag (0xFFFFFFFE)


/* r-locking:
 * 1. reschedule until 'pending' and 'wlock' bits are zero
 * 2. increase N of readers */
bool Threads::CoroRWLock_c::ReadLock ()
{
	assert ( IsInsideCoroutine () );
	auto uPrevState = m_uLock.load ( std::memory_order_acquire ); // memory_order_acquire
	do {
		if ( uPrevState & ux03 ) // check only pending and wlock bits
			CoWorker ()->Reschedule ();
		uPrevState &= uxFC;
	} while ( !m_uLock.compare_exchange_weak ( uPrevState, uPrevState+ux04, std::memory_order_acq_rel ) );
	assert ( ( uPrevState & uxFC )!=uxFC ); // hope, 30 bit is enough to count all re-entered r-locks.
	return true;
}

/* w-locking:
 * 1. set 'pending' flag, that prohibits readers
 * 2. reschedule until all bits, ignoring 'pending' are zero
 * 3. If in check we found that 'pending' became unset, - reinstall it.
 * 4. append 'wlock' bit. Finally on acquire wlock bit is set, pending is inherited, others zero.
 */
 bool Threads::CoroRWLock_c::WriteLock ()
{
	assert ( IsInsideCoroutine () );

	// prohibit further readers; from setting pending bit they may only release but no more acquire.
	auto uMyState = m_uLock.fetch_or ( ux01, std::memory_order_acq_rel );

	// will leave w-lock bit set, and pending bit inherited from previous state
	auto uTargetState = ( uMyState & ux01 ) | ux02; // will leave on success with this state written
	uMyState |= ux01;

	do {
		if ( uMyState & uxFE ) // check all except pending bit
			CoWorker ()->Reschedule (); // works like spin-lock, but that is ok for now

		// if we're waiting after another w-lock, it releases pending bit
		// we need to reinstall it in order to keep readers out. Target state is just pure 2-lock bit here.
		if ( ( uMyState & ux01 )==0 )
		{
			uMyState = m_uLock.fetch_or ( ux01, std::memory_order_acq_rel );
			uTargetState = ux02;
			uMyState |= ux01;
		}
		uMyState &= ux01;
	} while ( !m_uLock.compare_exchange_weak ( uMyState, uTargetState, std::memory_order_acq_rel ) );
	return true;
}

/* r-lock to w-lock elevation.
 * 0. Check that w-lock is not acquired, and also r-locks are not empty (as we elevate from existing r-lock)
 * 1. set 'pending' flag, that prohibits readers.
 * 2. reschedule until we have only 1 reader ('pending' and 'wlocked' bits ignored)
 * 3. append 'wlock' bit. Finally on acquire wlock bit is set, pending is inherited, others zero.
 */
/*
 * Elevation has one fundamental problem, that is why it is commented out as dangerous solution.
 * One (and only one!) reader is ok to elevate (that is why it is left here and not totally wiped out).
 * When more than one want to elevate concurrently from different threads - that by the fact means, they want
 * exclusive (write) lock, and they already step into acquiring it by holding shared (read) lock.
 * Such 'semi-exclusive' state is ill and means immediate deadlock on the waiting (i.e. all candidates waits until
 * shared lock released by all others, and such mutual waiting is dead).
 * Since it is hard to avoid concurrency, let's forget about elevation for a while or forever.
bool Threads::CoroRWLock_c::UpgradeLock ()
{
	assert ( IsInsideCoroutine () );
	// prohibit further readers; from setting pending bit they may only release but no more acquire.
	auto uMyState = m_uLock.fetch_or ( ux01, std::memory_order_acq_rel );
	auto uTargetState = ( uMyState & ux01 ) | ux02; // will leave on success with this state written
	assert ( ( uMyState & ux02 )==0 ); // there no elevation possible from w-lock
	assert ( ( uMyState & uxFC )!=0 ); // there no elevation possible from zero (must be r-lock already)
	uMyState |= ux01;

	do {
		if ( ( uMyState & uxFC )>ux04 ) // check all except pending bit
			CoWorker ()->Reschedule ();// works like spin-lock, but that is ok for now
		uMyState = ux04 + ( uMyState & ux01 );
	} while ( !m_uLock.compare_exchange_weak ( uMyState, uTargetState, std::memory_order_acq_rel ) );
	return true;
}
 */

/* unlocking:
 * 1. If 'wlock' bit is set, reset it
 * 2. Otherwise decrease rlock counter.
 */
bool Threads::CoroRWLock_c::Unlock ()
{
	assert ( IsInsideCoroutine () );
	auto uMyState = m_uLock.load ( std::memory_order_acquire );  // memory_order_acquire
	if ( ( uMyState & uxFE )==ux02 ) // was w-locked, reset keeping only pending bit
		m_uLock.fetch_and ( ux01, std::memory_order_acq_rel ); // released exclusive lock memory_order_release
	else // was w-locked, decrease counter
		m_uLock.fetch_sub ( ux04, std::memory_order_acq_rel ); // released shared lock memory_order_acq_rel
	return true;
}


Threads::CoroSpinlock_c::~CoroSpinlock_c ()
{
	assert ( !m_bLocked.load ( std::memory_order_relaxed ) );
}

void Threads::CoroSpinlock_c::Lock()
{
	assert ( Threads::IsInsideCoroutine () );
	while ( true )
	{
		bool bCurrent = false;
		if ( m_bLocked.compare_exchange_weak ( bCurrent, true, std::memory_order_acquire ) )
			break;
		Threads::CoWorker ()->Reschedule ();
	}
}

void Threads::CoroSpinlock_c::Unlock()
{
	assert ( Threads::IsInsideCoroutine () );
	m_bLocked.store ( false, std::memory_order_release );
}
