//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
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

static const size_t g_iStackSize = 1024 * 128; // stack size - 128K
#define LOG_COMPONENT_CORO "Stack: " << m_dStack.GetLength() << " (" << m_eState << ") "

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
	CSphFixedVector<BYTE> m_dStack;

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

public:
	explicit CoRoutine_c ( Handler fnHandler, size_t iStack=0 )
		: m_fnHandler ( std::move ( fnHandler ) )
		, m_dStack ( iStack? (int) iStack: g_iStackSize )
	{
#if BOOST_USE_VALGRIND
		m_uValgrindStackID = VALGRIND_STACK_REGISTER( m_dStack.begin(), &m_dStack.Last () );
#endif
		m_tCoroutineContext = make_fcontext ( &m_dStack.Last (), m_dStack.GetLength (), [] ( transfer_t pT )
		{
			static_cast<CoRoutine_c *>(pT.data)->WorkerLowest ( pT.fctx );
		} );
	}

#if BOOST_USE_VALGRIND
	~CoRoutine_c()
	{
		VALGRIND_STACK_DEREGISTER( m_uValgrindStackID );
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
		CoroGuard_t ( CoroWorker_c * pWorker )
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

	void Run ()
	{
		{
			CoroGuard_t pThis (this);
			m_tCoroutine.Run();
		}
		if ( m_tCoroutine.IsFinished () )
		{
			delete this;
			return;
		}
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
			Schedule ();
	}

	void Reschedule()
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

CoroWorker_c * CoWorker ()
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
void CoCo ( Handler fnHandler, Waiter_t tSignaller )
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

void CallPlainCoroutine ( Handler fnHandler )
{
	auto pScheduler = GlobalWorkPool ();
	CSphAutoEvent tEvent;
	auto dWaiter = Waiter_t ( nullptr, [&tEvent] ( void * ) { tEvent.SetEvent (); } );
	CoroWorker_c::StartCall ( std::move ( fnHandler ), pScheduler, std::move(dWaiter) );
	tEvent.WaitEvent ();
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

void CoYieldWith ( Handler fnHandler )
{
	CoWorker ()->YieldWith ( std::move(fnHandler) );
}

// Move current task to another scheduler. Say, from netloop to worker, or from plain worker to vip, etc.
void CoMoveTo ( Scheduler_i * pScheduler )
{
	CoWorker ()->MoveTo ( pScheduler );
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
	return g_iThreadStackSize;
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


Threads::CoThrottler_c::CoThrottler_c ( int64_t tmThrottlePeriodUs )
	: m_tmThrottlePeriodUs ( tmThrottlePeriodUs )
{
	m_tmNextThrottleTimestamp = sphMicroTimer() + m_tmThrottlePeriodUs;
}

bool Threads::CoThrottler_c::MaybeThrottle ()
{
	auto tmNow = sphMicroTimer ();
	if ( tmNow < m_tmNextThrottleTimestamp )
		return false;

	m_tmNextThrottleTimestamp = tmNow + m_tmThrottlePeriodUs;
	CoWorker ()->Reschedule ();
	return true;
}

void Threads::CoThrottler_c::ThrottleAndKeepCrashQuery ()
{
	auto tmNow = sphMicroTimer ();
	if ( tmNow<m_tmNextThrottleTimestamp )
		return;

	m_tmNextThrottleTimestamp = tmNow+m_tmThrottlePeriodUs;
	CrashQueryKeeper_c _;
	CoWorker ()->Reschedule ();
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
	return CoWorker ()!=nullptr;
}

/*
 * Legend for m_uLock:
 * highest bit indicates exclusive lock, others - num of acquired shared locks
 *
 * Possible values:
 * 0x0 - unlocked;
 * 0x21 - 33 r-locks acquired, may acquire and release other shared locks.
 * 0x10000021 - 33 r-locks acquired, and w-lock is waiting. will NOT acquire more r-locks, just release existing.
 * 0x10000000 - exclusive lock acquired. May be only released.
 */


static const DWORD uWL = 0x10000000; // flag for write-lock
static const DWORD uMWL = ~uWL; // mask for write-log flag (0x7FFFFFFF)
static const DWORD uINC = 1; // increment for r-lock to avoid touching w-lock

/*
 * w-locking:
 * pending = (m_uLock & uWL)? ~0 : ~uWL; // if somebody pending - check later all bits. Otherwise only low
 * m_uLock |= uWL;	// set bit that we're want lock. So, readers will no more acquire the lock
 * m_uLock&pending==0 ? m_uLock=uWL; return // exclusive lock acquired
 * else reschedule
 *
 * r-locking:
 * m_uLock&uWL==0? ++m_uLock; return // r-lock acquired
 * else reschedule // w-locked, bail
 *
 * unlocking:
 * if m_uLock&~uWL: --m_uLock; return
 * if m_uLock&uWL : m_uLock&=~uWL; return
 * assert (!m_uLock)
 */
bool Threads::CoroRWLock_c::WriteLock ()
{
	assert ( IsInsideCoroutine () );
	auto uPrevState = m_uLock.fetch_or ( uWL, std::memory_order_acq_rel );
	auto uCheckMask = uPrevState | uMWL; // == 0xFFFFFFFF if w-locked, 0x7FFFFFFF, if free
	uPrevState |= uWL;

	do {
		if ( ( uPrevState & uCheckMask )!=0 ) // for previously wlocked - check whole, for free - only low bits
			CoWorker ()->Reschedule ();
		uPrevState &= ~uCheckMask; // expect 0, if was locked. Or ignore wlock bit, if not (we're set it ourselves)
	} while ( !m_uLock.compare_exchange_weak ( uPrevState, uWL, std::memory_order_acq_rel ) );
	return true;
}

bool Threads::CoroRWLock_c::ReadLock ()
{
	assert ( IsInsideCoroutine () );
	auto uPrevState = m_uLock.load ( std::memory_order_acquire );
	do {
		if ( ( uPrevState & uWL )!=0 )
			CoWorker ()->Reschedule ();
		uPrevState &= uMWL;
	} while ( !m_uLock.compare_exchange_weak ( uPrevState, uPrevState+uINC, std::memory_order_acq_rel ) );
	assert ( ( uPrevState & uMWL )!=uMWL ); // hope, 31 bit is enough to count all re-entered r-locks.
	return true;
}

bool Threads::CoroRWLock_c::Unlock ()
{
	assert ( IsInsideCoroutine () );
	if ( m_uLock.load ( std::memory_order_acquire )==uWL )
		m_uLock.store ( 0, std::memory_order_release ); // released exclusive lock
	else
		m_uLock.fetch_sub ( uINC, std::memory_order_acq_rel ); // released shared lock
	return true;
}
