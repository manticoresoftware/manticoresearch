//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
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

static std::atomic<int64_t> g_tmLastTimestamp { sphMicroTimer() };

inline static int64_t MicroTimerImpl()
{
	int64_t tmTimestamp = sphMicroTimer();
	g_tmLastTimestamp.store ( tmTimestamp, std::memory_order_relaxed );
	return tmTimestamp;
}

inline static int64_t LastTimestampImpl()
{
	return g_tmLastTimestamp.load ( std::memory_order_relaxed );
}


int64_t sph::MicroTimer()
{
	return MicroTimerImpl();
}

int64_t sph::LastTimestamp()
{
	return LastTimestampImpl();
}

/// timer thread context
static ThreadRole TimerThread;

volatile bool& IsTinyTimerCreated()
{
	static volatile bool bCreated = false;
	return bCreated;
}

class TinyTimer_c
{
	// the queue
	mutable CSphMutex m_tTimeoutsGuard; // guard is need as we can add/remove elements from any thread. That is short-live.
	TimeoutQueue_c m_dTimeouts GUARDED_BY ( m_tTimeoutsGuard );

	// management
	OneshotEvent_c m_tSignal;
	std::atomic<bool> m_bInterrupted { true };

	// thread
	SphThread_t m_tCounterThread;
	std::atomic<Threads::LowThreadDesc_t*> m_pCounterThread { nullptr };

private:
	inline bool IsInterrupted() const
	{
		return m_bInterrupted.load(std::memory_order_relaxed) || sphInterrupted(); // aliased, as we can override it in tests while mocking
	}

	void Enqueue ( MiniTimer_c& tTask ) EXCLUDES ( TimerThread )
	{
		DEBUGT << "enqueue " << &tTask;
		{
			ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
			m_dTimeouts.Change ( &tTask );
		}
		Kick();
	}

	int GetNextWaitPeriodMs() REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		if ( m_dTimeouts.IsEmpty() )
			return -1;

		auto* pTask = (MiniTimer_c*)m_dTimeouts.Root();
		return (int)( ( pTask->m_iTimeoutTimeUS - MicroTimerImpl() ) / sph::TICKS_GRANULARITY );
	}

	MiniTimer_c* PopNextDeadlinedAction() EXCLUDES ( m_tTimeoutsGuard )
	{
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		if ( m_dTimeouts.IsEmpty() )
			return nullptr;

		auto pRoot = (MiniTimer_c*)m_dTimeouts.Root();
		assert ( pRoot->m_iTimeoutTimeUS > 0 );
		if ( !sph::TimeExceeded ( pRoot->m_iTimeoutTimeUS, MicroTimerImpl() ) )
			return nullptr;

		// timeout reached; have to do an action
		DEBUGT << "timeout happens for " << pRoot << " deadline " << timestamp_t ( pRoot->m_iTimeoutTimeUS );
		DEBUGT << m_dTimeouts.DebugDump ( "heap:" );
		m_dTimeouts.Pop();
		DEBUGT << "Oneshot task removed: " << pRoot;

		return pRoot;
	}

	void ProcessTimerActions() REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		for ( MiniTimer_c* pRoot = PopNextDeadlinedAction(); pRoot; pRoot = PopNextDeadlinedAction() )
			if ( pRoot->m_fnOnTimer )
				pRoot->m_fnOnTimer();
	}

	void Loop()
	{
		ScopedRole_c thSched ( TimerThread );
		m_pCounterThread.store ( &Threads::MyThd(), std::memory_order_relaxed );
		while ( !IsInterrupted () )
		{
			DEBUGT << "---------------------------- Loop() tick";
			ProcessTimerActions();
			int iWait = GetNextWaitPeriodMs();
			if ( !iWait )
			{
				DEBUGT << "no sleep since timeout is 0; (" << timestamp_t ( iWait ) << ")";
				continue;
			}
			DEBUGT << "calculated timeout is " << iWait << " ms (" << timestamp_t ( iWait ) << ")";
			bool VARIABLE_IS_NOT_USED bWasKicked = m_tSignal.WaitEvent ( iWait );
			DEBUGT << "awakened, reason=" << ( bWasKicked ? "kicked" : "timeout or error" );
		}
		AbortScheduled();
		m_pCounterThread.store ( nullptr, std::memory_order_relaxed );
	}

	MiniTimer_c* PopNextAction() REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		if ( m_dTimeouts.IsEmpty() )
			return nullptr;

		auto pRoot = (MiniTimer_c*)m_dTimeouts.Root();
		m_dTimeouts.Pop();
		return pRoot;
	}

	/// abandon and release all events (on shutdown)
	void AbortScheduled() REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		DEBUGT << "AbortScheduled()";
		assert ( IsInterrupted() );

		for ( MiniTimer_c* pRoot = PopNextAction(); pRoot; pRoot = PopNextAction() )
			if ( pRoot->m_fnOnTimer )
				pRoot->m_fnOnTimer();
	}

public:
	TinyTimer_c()
	{
		MicroTimerImpl();
		m_bInterrupted.store ( false, std::memory_order_release );
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
		m_bInterrupted.store ( true, std::memory_order_release );
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

	void EngageAt ( int64_t iTimeStampUS, MiniTimer_c& tTimer ) EXCLUDES ( TimerThread )
	{
		tTimer.m_iTimeoutTimeUS = iTimeStampUS;
		DEBUGT << "Engage task: " << &tTimer << " after " << timestamp_t ( iTimeStampUS );
		Enqueue ( tTimer );
	}

	int64_t Engage ( int64_t iTimePeriodUS, MiniTimer_c& tTimer ) EXCLUDES ( TimerThread )
	{
		if ( iTimePeriodUS < 0 || IsInterrupted() )
			return -1;
		EngageAt ( MicroTimerImpl() + iTimePeriodUS, tTimer );
		return tTimer.m_iTimeoutTimeUS;
	}

	void Remove ( MiniTimer_c& tTimer ) EXCLUDES ( m_tTimeoutsGuard, TimerThread )
	{

		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		DEBUGT << ((tTimer.m_iTimeoutIdx >= 0) ? "Removed from queue: " : "Not in queue: ") << &tTimer << " deadline " << timestamp_t ( tTimer.m_iTimeoutTimeUS );
		m_dTimeouts.Remove ( &tTimer );
	}

	// statistics
	void FillSchedInfo( CSphVector<sph::ScheduleInfo_t>& dRes) const EXCLUDES ( m_tTimeoutsGuard, TimerThread )
	{
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		m_dTimeouts.DebugDump ( [&dRes] ( EnqueuedTimeout_t* pMember ) {
			auto& dInfo = dRes.Add();
			auto* pScheduled = (MiniTimer_c*)pMember;
			dInfo.m_iTimeoutStamp = pScheduled->m_iTimeoutTimeUS;
			dInfo.m_sTask = pScheduled->m_szName;
		} );
	}
};

TinyTimer_c& g_TinyTimer()
{
	static TinyTimer_c tTimer;
	return tTimer;
}

void MiniTimer_c::EngageAt ( int64_t iTimeStampUS )
{
	DEBUGT << "MiniTimer_c::EngageAt " << timestamp_t ( iTimeStampUS );
	g_TinyTimer().EngageAt ( iTimeStampUS, *this );
}

void MiniTimer_c::EngageAt ( int64_t iTimeStampUS, Threads::Handler&& fnOnTimer )
{
	DEBUGT << "MiniTimer_c::EngageAt " << timestamp_t ( iTimeStampUS );
	assert ( !m_fnOnTimer );
	m_fnOnTimer = std::move ( fnOnTimer );
	g_TinyTimer().EngageAt ( iTimeStampUS, *this );
}

int64_t MiniTimer_c::Engage ( int64_t iTimePeriodMS )
{
	auto iTimePeriodUS = iTimePeriodMS * 1000;
	if ( iTimePeriodUS <= 0 )
		return 0;
	DEBUGT << "MiniTimer_c::Engage " << timespan_t ( iTimePeriodUS );
	return g_TinyTimer().Engage ( iTimePeriodUS, *this );
}

int64_t MiniTimer_c::Engage ( int64_t iTimePeriodMS, Threads::Handler&& fnOnTimer )
{
	auto iTimePeriodUS = iTimePeriodMS * 1000;
	if ( iTimePeriodUS <= 0 )
		return 0;
	DEBUGT << "MiniTimer_c::Engage " << timespan_t ( iTimePeriodUS );
	assert ( !m_fnOnTimer );
	m_fnOnTimer = std::move ( fnOnTimer );
	return g_TinyTimer().Engage ( iTimePeriodUS, *this );
}

void MiniTimer_c::UnEngage()
{
	g_TinyTimer().Remove ( *this );
}

MiniTimer_c::~MiniTimer_c()
{
	UnEngage();
}

/// returns true if provided timestamp is already reached or not
bool sph::TimeExceeded ( int64_t tmMicroTimestamp )
{
	if ( tmMicroTimestamp <= 0 )
		return false;
	return sph::TimeExceeded ( tmMicroTimestamp, LastTimestampImpl() );
}

void sph::ShutdownMiniTimer()
{
	if ( IsTinyTimerCreated() )
		g_TinyTimer().Stop();
}

// statistics
CSphVector<sph::ScheduleInfo_t> sph::GetSchedInfo()
{
	CSphVector<sph::ScheduleInfo_t> dRes;
	if ( IsTinyTimerCreated() )
		g_TinyTimer().FillSchedInfo ( dRes );
	return dRes;
}