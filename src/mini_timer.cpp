//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
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
#include <atomic>

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

using namespace Threads;

namespace {
std::atomic g_tmLastTimestamp { sphMicroTimer() };

int64_t MicroTimerImpl() noexcept
{
	int64_t tmTimestamp = sphMicroTimer();
	g_tmLastTimestamp.store ( tmTimestamp, std::memory_order_relaxed );
	return tmTimestamp;
}

[[nodiscard]] int64_t LastTimestampImpl() noexcept
{
	return g_tmLastTimestamp.load ( std::memory_order_relaxed );
}


/// timer thread context
ThreadRole TimerThread;
std::atomic g_bTimerCreated { false };
std::atomic g_bTimerActive { false };
} // namespace

[[nodiscard]] int64_t sph::MicroTimer() noexcept
{
	return MicroTimerImpl();
}

[[nodiscard]] int64_t sph::LastTimestamp() noexcept
{
	return LastTimestampImpl();
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
	std::atomic<LowThreadDesc_t*> m_pCounterThread { nullptr };

private:
	[[nodiscard]] bool IsInterrupted() const noexcept
	{
		return m_bInterrupted.load(std::memory_order_relaxed) || sphInterrupted(); // aliased, as we can override it in tests while mocking
	}

	void Enqueue ( MiniTimer_c& tTask ) noexcept EXCLUDES ( m_tTimeoutsGuard )
	{
		DEBUGT << "enqueue " << &tTask;
		{
			ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
			m_dTimeouts.Change ( &tTask );
		}
		Kick();
	}

	[[nodiscard]] int GetNextWaitPeriodMs() const noexcept REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		if ( m_dTimeouts.IsEmpty() )
			return -1;

		auto* pTask = (MiniTimer_c*)m_dTimeouts.Root();
		return (int)( ( pTask->m_iTimeoutTimeUS - MicroTimerImpl() ) / sph::TICKS_GRANULARITY );
	}

	[[nodiscard]] std::pair<MiniTimer_c*,Handler> PopNextDeadlinedAction() noexcept REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		if ( m_dTimeouts.IsEmpty() )
			return { nullptr, nullptr };

		auto pRoot = (MiniTimer_c*)m_dTimeouts.Root();
		assert ( pRoot->m_iTimeoutTimeUS > 0 );
		if ( !sph::TimeExceeded ( pRoot->m_iTimeoutTimeUS, MicroTimerImpl() ) )
			return { nullptr, nullptr };

		// timeout reached; have to do an action
		DEBUGT << "timeout happens for " << pRoot << " deadline " << timestamp_t ( pRoot->m_iTimeoutTimeUS );
		DEBUGT << m_dTimeouts.DebugDump ( "heap:" );
		m_dTimeouts.Pop();
		DEBUGT << "Oneshot task removed: " << pRoot;

		return  { pRoot, pRoot->m_fnOnTimer };
	}

	void ProcessTimerActions() noexcept REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		for ( auto [pRoot, fnTimer] = PopNextDeadlinedAction(); pRoot; std::tie(pRoot, fnTimer) = PopNextDeadlinedAction() )
			if ( fnTimer )
				fnTimer();
	}

	void Loop() noexcept
	{
		ScopedRole_c thSched ( TimerThread );
		m_pCounterThread.store ( &Threads::MyThd(), std::memory_order_relaxed );
		g_bTimerActive = true;
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
		g_bTimerActive = false;
		AbortScheduled();
		m_pCounterThread.store ( nullptr, std::memory_order_relaxed );
	}

	[[nodiscard]] std::pair<MiniTimer_c*,Handler> PopNextAction() noexcept REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		if ( m_dTimeouts.IsEmpty() )
			return { nullptr, nullptr };

		auto pRoot = (MiniTimer_c*)m_dTimeouts.Root();
		m_dTimeouts.Pop();
		return { pRoot, pRoot->m_fnOnTimer };
	}

	/// abandon and release all events (on shutdown)
	void AbortScheduled() noexcept REQUIRES ( TimerThread ) EXCLUDES ( m_tTimeoutsGuard )
	{
		DEBUGT << "AbortScheduled()";
		assert ( IsInterrupted() );

		for ( auto [pRoot, fnTimer] = PopNextAction(); pRoot; std::tie(pRoot, fnTimer) = PopNextAction() )
			if ( fnTimer )
				fnTimer();
	}

public:
	TinyTimer_c()
	{
		MicroTimerImpl();
		m_bInterrupted.store ( false, std::memory_order_release );
		g_bTimerCreated = true;
		RegisterIterator ( [this] ( const ThreadFN& fnHandler ) {
			fnHandler ( m_pCounterThread.load ( std::memory_order_relaxed ) );
		} );
		Create ( &m_tCounterThread, [this] { Loop (); }, false, "Timer" );
	}

	~TinyTimer_c()
	{
		DEBUGX << "~TinyTimer_c. Shutdown=" << IsInterrupted();
		Stop();
	}

	void Stop() noexcept
	{
		m_bInterrupted.store ( true, std::memory_order_release );
		if ( !g_bTimerActive )
			return;
		Kick();
		Join ( &m_tCounterThread );
	}

	/// Kick the tasker
	void Kick() noexcept
	{
		DEBUGX << "Timer kicked";
		m_tSignal.SetEvent();
	}

	void EngageAt ( int64_t iTimeStampUS, MiniTimer_c& tTimer ) noexcept EXCLUDES ( m_tTimeoutsGuard )
	{
		if ( tTimer.m_iTimeoutIdx >= 0 && tTimer.m_iTimeoutTimeUS == iTimeStampUS )
			return; // no need to re-engage if no changes

		tTimer.m_iTimeoutTimeUS = iTimeStampUS;
		DEBUGT << "Engage task: " << &tTimer << " after " << timestamp_t ( iTimeStampUS );
		Enqueue ( tTimer );
	}

	int64_t Engage ( int64_t iTimePeriodUS, MiniTimer_c& tTimer ) noexcept EXCLUDES ( m_tTimeoutsGuard )
	{
		if ( iTimePeriodUS < 0 || IsInterrupted() )
			return -1;
		EngageAt ( MicroTimerImpl() + iTimePeriodUS, tTimer );
		return tTimer.m_iTimeoutTimeUS;
	}

	void Remove ( MiniTimer_c& tTimer ) noexcept EXCLUDES ( m_tTimeoutsGuard )
	{
		ScopedMutex_t tTimeoutsLock { m_tTimeoutsGuard };
		DEBUGT << ((tTimer.m_iTimeoutIdx >= 0) ? "Removed from queue: " : "Not in queue: ") << &tTimer << " deadline " << timestamp_t ( tTimer.m_iTimeoutTimeUS );
		m_dTimeouts.Remove ( &tTimer );
	}

	// statistics
	void FillSchedInfo( CSphVector<sph::ScheduleInfo_t>& dRes) const noexcept EXCLUDES ( m_tTimeoutsGuard )
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

void MiniTimer_c::EngageAt ( int64_t iTimeStampUS ) noexcept
{
	DEBUGT << "MiniTimer_c::EngageAt " << timestamp_t ( iTimeStampUS );
	g_TinyTimer().EngageAt ( iTimeStampUS, *this );
}

void MiniTimer_c::EngageAt ( int64_t iTimeStampUS, Handler&& fnOnTimer ) noexcept
{
	DEBUGT << "MiniTimer_c::EngageAt " << timestamp_t ( iTimeStampUS );
	SetHandler ( std::move ( fnOnTimer ) );
	g_TinyTimer().EngageAt ( iTimeStampUS, *this );
}

int64_t MiniTimer_c::Engage ( int64_t iTimePeriodMS ) noexcept
{
	auto iTimePeriodUS = iTimePeriodMS * 1000;
	if ( iTimePeriodUS <= 0 )
		return 0;
	DEBUGT << "MiniTimer_c::Engage " << timespan_t ( iTimePeriodUS );
	return g_TinyTimer().Engage ( iTimePeriodUS, *this );
}

int64_t MiniTimer_c::Engage ( int64_t iTimePeriodMS, Handler&& fnOnTimer ) noexcept
{
	auto iTimePeriodUS = iTimePeriodMS * 1000;
	if ( iTimePeriodUS <= 0 )
		return 0;
	DEBUGT << "MiniTimer_c::Engage " << timespan_t ( iTimePeriodUS );
	SetHandler ( std::move ( fnOnTimer ) );
	return g_TinyTimer().Engage ( iTimePeriodUS, *this );
}

void MiniTimer_c::SetHandler ( Handler&& fnOnTimer ) noexcept
{
	assert ( !m_fnOnTimer );
	m_fnOnTimer = std::move ( fnOnTimer );
}

void MiniTimer_c::UnEngage() noexcept
{
	if ( g_bTimerCreated.load(std::memory_order_relaxed) )
		g_TinyTimer().Remove ( *this );
}

MiniTimer_c::~MiniTimer_c()
{
	UnEngage();
}

/// returns true if provided timestamp is already reached or not
[[nodiscard]] bool sph::TimeExceeded ( int64_t tmMicroTimestamp ) noexcept
{
	if ( tmMicroTimestamp <= 0 )
		return false;
	return sph::TimeExceeded ( tmMicroTimestamp, LastTimestampImpl() );
}

void sph::ShutdownMiniTimer()
{
	if ( g_bTimerActive )
		g_TinyTimer().Stop();
}

// statistics
CSphVector<sph::ScheduleInfo_t> sph::GetSchedInfo()
{
	CSphVector<sph::ScheduleInfo_t> dRes;
	if ( g_bTimerCreated )
		g_TinyTimer().FillSchedInfo ( dRes );
	return dRes;
}