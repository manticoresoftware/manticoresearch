//
// Copyright (c) 2021-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "mini_timer.h"
#include "threadutils.h"
#include "timeout_queue.h"
#include "std/spinlock.h"

#include <boost/intrusive/slist.hpp>

#ifndef VERBOSE_TIMER
#define VERBOSE_TIMER 0
#endif

#if VERBOSE_TIMER
#define LOG_LEVEL_TIMER true
#else
#define LOG_LEVEL_TIMER false
#endif

namespace Time
{
CSphString Stamp()
{
	return StringBuilder_c().Sprintf( "[%t] ", TimePrefixed::TimeStamp() ).cstr();
}
}

#define LOG_COMPONENT_TSKX "X " << Time::Stamp()
#define LOG_COMPONENT_TSKT "T " << Time::Stamp()

#define INFOX LOGMSG ( INFO, TIMER, TSKX )
#define DEBUGT LOGMSG ( DEBUG, TIMER, TSKT )
#define DEBUGX LOGMSG ( DEBUG, TIMER, TSKX )

using TimerSlist_t = boost::intrusive::slist<
	MiniTimer_c,
	boost::intrusive::member_hook<MiniTimer_c, sph::TimerHook_t, &MiniTimer_c::m_tLink>,
	boost::intrusive::constant_time_size<false>,
	boost::intrusive::cache_last<true>>;

/// timer thread context
static ThreadRole TimerThread;

/// that is need for remove() to work
inline bool operator== ( const MiniTimer_c& lhs, const MiniTimer_c& rhs )
{
	return &lhs==&rhs;
}

volatile bool& IsTinyTimerCreated()
{
	static volatile bool bCreated = false;
	return bCreated;
}

class TinyTimer_c
{
	// stuff to transfer (enqueue) tasks
	sph::Spinlock_c m_tStagingGuard {};
	TimerSlist_t m_tStagingQueue GUARDED_BY ( m_tStagingGuard );

	// the queue
	mutable CSphMutex m_tTimeoutsGuard; // guard is need as we can remove elems from any thread. That is short-live.
	TimeoutQueue_c m_dTimeouts GUARDED_BY ( m_tTimeoutsGuard );

	int64_t m_iNextTimeoutUS = -1;
	std::atomic<int64_t> m_tmLastTimestamp { sphMicroTimer() };

	// management
	OneshotEvent_c m_tSignal;
	volatile bool m_bInterrupted = true;

	// thread
	SphThread_t m_tCounterThread;
	std::atomic<Threads::LowThreadDesc_t*> m_pCounterThread { nullptr };

private:
	inline bool IsInterrupted() const
	{
		return m_bInterrupted || sphInterrupted(); // aliased, as we can override it in tests while mocking
	}

	inline int64_t MicroTimer()
	{
		int64_t tmTimestamp = sphMicroTimer();
		m_tmLastTimestamp.store ( tmTimestamp, std::memory_order_relaxed );
		return tmTimestamp;
	}

	void Enqueue ( MiniTimer_c& tTask ) EXCLUDES ( TimerThread, m_tStagingGuard )
	{
		DEBUGT << "enqueue " << &tTask;
		{
			sph::Spinlock_lock tLock { m_tStagingGuard };
			if ( !tTask.m_tLink.is_linked() )
				m_tStagingQueue.push_back ( tTask );
		}
		// optimization: we anyway enqueue the task, however if next scheduled time to awake came earlier than our schedule - we don't need to kick the thread.
		if ( ( tTask.m_iTimeoutTimeUS < m_iNextTimeoutUS ) | !m_iNextTimeoutUS ) // | instead of || to avoid branching
			Kick();
	}

	MiniTimer_c* PopStagingTask() REQUIRES ( TimerThread ) EXCLUDES ( m_tStagingGuard )
	{
		sph::Spinlock_lock tLock { m_tStagingGuard };
		if ( m_tStagingQueue.empty() )
			return nullptr;

		auto& tScheduled = m_tStagingQueue.front();
		m_tStagingQueue.pop_front();
		return &tScheduled;
	}

	int EnqueueStagingTasks() REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		while ( MiniTimer_c * pStagingTask = PopStagingTask() )
			m_dTimeouts.Change ( pStagingTask );
		if ( m_dTimeouts.IsEmpty() )
		{
			m_iNextTimeoutUS = 0;
			return -1;
		}

		auto* pTask = (MiniTimer_c*)m_dTimeouts.Root();
		m_iNextTimeoutUS = pTask->m_iTimeoutTimeUS;
		return (int)( ( m_iNextTimeoutUS - MicroTimer() ) / sph::TICKS_GRANULARITY );
	}

	void ProcessTimerActions() REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		if ( !sph::TimeExceeded ( m_iNextTimeoutUS, MicroTimer() ) )
			return;

		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		auto tmTimestamp = MicroTimer();
		while ( !m_dTimeouts.IsEmpty() )
		{
			auto* pRoot = (MiniTimer_c*)m_dTimeouts.Root();
			assert ( pRoot->m_iTimeoutTimeUS > 0 );
			if ( !sph::TimeExceeded ( pRoot->m_iTimeoutTimeUS, tmTimestamp ) )
				return;

			// timeout reached; have to do an action
			DEBUGT << "timeout happens for " << pRoot << " deadline " << timestamp_t (pRoot->m_iTimeoutTimeUS);
			DEBUGT << m_dTimeouts.DebugDump ( "heap:" );
			m_dTimeouts.Pop();
			DEBUGT << "Oneshot task removed: " << pRoot;
			pRoot->OnTimer();
		}
	}

	void Loop()
	{
		ScopedRole_c thSched ( TimerThread );
		m_pCounterThread.store ( &Threads::MyThd(), std::memory_order_relaxed );
		while ( !IsInterrupted () )
		{
			DEBUGT << "---------------------------- Loop() tick";
			ProcessTimerActions();
			int iWait = EnqueueStagingTasks();
			if ( !iWait )
			{
				DEBUGT << "no sleep since timeout is 0; (" << timestamp_t ( m_iNextTimeoutUS ) << ")";
				continue;
			}
			DEBUGT << "calculated timeout is " << iWait << " ms (" << timestamp_t ( m_iNextTimeoutUS ) << ")";
			bool VARIABLE_IS_NOT_USED bWasKicked = m_tSignal.WaitEvent ( iWait );
			DEBUGT << "awakened, reason=" << ( bWasKicked ? "kicked" : "timeout or error" );
		}
		AbortScheduled();
		m_pCounterThread.store ( nullptr, std::memory_order_relaxed );
	}

	/// abandon and release all events (on shutdown)
	void AbortScheduled() NO_THREAD_SAFETY_ANALYSIS
	{
		DEBUGT << "AbortScheduled()";
		assert ( IsInterrupted() );
		while ( !m_dTimeouts.IsEmpty() )
		{
			auto pScheduled = (MiniTimer_c*)m_dTimeouts.Root();
			m_dTimeouts.Pop();
			pScheduled->OnTimer();
		}
		while ( !m_tStagingQueue.empty() )
		{
			MiniTimer_c& tScheduled = m_tStagingQueue.front();
			m_tStagingQueue.pop_front();
			tScheduled.OnTimer();
		}
	}

public:
	TinyTimer_c()
	{
		MicroTimer();
		m_bInterrupted = false;
		IsTinyTimerCreated() = true;
		Threads::RegisterIterator ( [this] ( Threads::ThreadFN& fnHandler ) {
			fnHandler ( m_pCounterThread.load ( std::memory_order_relaxed ) );
		} );
		Threads::Create ( &m_tCounterThread, [this] { Loop (); }, false, "Timer" );
	}

	~TinyTimer_c()
	{
		DEBUGX << "~TinyTimer_c. Shutdown=" << IsInterrupted();
		Stop();
	}

	void Stop()
	{
		m_bInterrupted = true;
		if ( !IsTinyTimerCreated() )
			return;
		Kick();
		Threads::Join ( &m_tCounterThread );
		IsTinyTimerCreated() = false;
	}

	/// Kick the tasker
	void Kick()
	{
		DEBUGX << "Timer kicked";
		m_tSignal.SetEvent();
	}

	bool TimeExceeded ( int64_t tmMicroTimestamp ) const
	{
		return sph::TimeExceeded ( tmMicroTimestamp, m_tmLastTimestamp.load ( std::memory_order_relaxed ) );
	}

	int64_t Engage ( int64_t iTimePeriodUS, MiniTimer_c& tTimer ) EXCLUDES ( TimerThread )
	{
		if ( iTimePeriodUS < 0 || IsInterrupted() )
			return -1;
		tTimer.m_iTimeoutTimeUS = MicroTimer() + iTimePeriodUS;
		DEBUGT << "Engage task: " << &tTimer << " after " << timespan_t ( iTimePeriodUS );
		Enqueue ( tTimer );
		return tTimer.m_iTimeoutTimeUS;
	}

	void Remove ( MiniTimer_c& tTimer ) EXCLUDES ( m_tTimeoutsGuard, m_tStagingGuard, TimerThread )
	{
		if ( IsInterrupted() )
			return;
		{
			sph::Spinlock_lock tLock { m_tStagingGuard };
			if ( tTimer.m_tLink.is_linked() )
			{
				m_tStagingQueue.remove ( tTimer );
				DEBUGT << "Removed linked: " << &tTimer << " deadline " << timestamp_t ( tTimer.m_iTimeoutTimeUS );
			}
		}
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		if ( tTimer.m_iTimeoutIdx>=0 )
			DEBUGT << "Removed from queue: " << &tTimer << " deadline " << timestamp_t ( tTimer.m_iTimeoutTimeUS );
		m_dTimeouts.Remove ( &tTimer );
	}

	// statictics
	void FillSchedInfo( CSphVector<sph::ScheduleInfo_t>& dRes) const EXCLUDES ( m_tTimeoutsGuard, TimerThread )
	{
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		m_dTimeouts.DebugDump ( [&dRes] ( EnqueuedTimeout_t* pMember ) {
			auto& dInfo = dRes.Add();
			auto* pSheduled = (MiniTimer_c*)pMember;
			dInfo.m_iTimeoutStamp = pSheduled->m_iTimeoutTimeUS;
			dInfo.m_sTask = pSheduled->m_szName;
		} );
	}
};

TinyTimer_c& g_TinyTimer()
{
	static TinyTimer_c tTimer;
	return tTimer;
}


int64_t MiniTimer_c::EngageUS ( int64_t iTimePeriodUS )
{
	if ( iTimePeriodUS <= 0 )
		return 0;
	DEBUGT << "MiniTimer_c::EngageUS " << timespan_t ( iTimePeriodUS );
	return g_TinyTimer ().Engage ( iTimePeriodUS, *this );
}

int64_t MiniTimer_c::Engage ( int64_t iTimePeriodMS )
{
	return EngageUS ( iTimePeriodMS * 1000 );
}

void MiniTimer_c::UnEngage()
{
	g_TinyTimer().Remove ( *this );
}

MiniTimer_c::~MiniTimer_c()
{
	if ( m_szName )
		UnEngage();
}

/// returns true if provided timestamp is already reached or not
bool sph::TimeExceeded ( int64_t tmMicroTimestamp )
{
	if ( tmMicroTimestamp <= 0 )
		return false;
	return g_TinyTimer ().TimeExceeded ( tmMicroTimestamp );
}

void sph::ShutdownMiniTimer()
{
	if ( IsTinyTimerCreated() )
		g_TinyTimer().Stop();
}

// statictics
CSphVector<sph::ScheduleInfo_t> sph::GetSchedInfo()
{
	CSphVector<sph::ScheduleInfo_t> dRes;
	if ( IsTinyTimerCreated() )
		g_TinyTimer().FillSchedInfo ( dRes );
	return dRes;
}