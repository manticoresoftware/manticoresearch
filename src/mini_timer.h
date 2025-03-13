//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include <cstdint>
#include "timeout_queue.h"
#include "threadutils.h"

namespace sph
{
	/// we use useconds for timestamps, however sleep obey mseconds. Interval less than granularity should be considered as 0, timestamps inside granularity should be considered as equal
	constexpr int64_t TICKS_GRANULARITY = 1000ll;

	inline bool TimeExceeded ( int64_t iTimestampUS, int64_t iPivotUS, int64_t GRANULARITY=TICKS_GRANULARITY )
	{
		assert ( iPivotUS > 0 );
		return ( iTimestampUS - GRANULARITY ) < iPivotUS;
	}

	/// call sphMicroTime, track value and return it
	int64_t MicroTimer();

	/// return last tracked value of MicroTimer (don't call sphMicroTime)
	int64_t LastTimestamp();

	/// returns true if provided timestamp is already reached or not
	/// it IMPLIES timer is engaged to given limit timestamp.
	/// non-engaged timer doesn't tick and may infinitely return false.
	bool TimeExceeded ( int64_t iTimestampUS );

	/// should be called on shutdown
	void ShutdownMiniTimer();

	struct ScheduleInfo_t
	{
		int64_t m_iTimeoutStamp;
		CSphString m_sTask;
	};

	CSphVector<ScheduleInfo_t> GetSchedInfo();
}

/// RAII timer thread ticker.
class MiniTimer_c final : public EnqueuedTimeout_t, public ISphNoncopyable
{
	friend class TinyTimer_c;
	const char*			m_szName;
	Threads::Handler	m_fnOnTimer;	// notice, will be called from naked thread, not coroutine ctx!

public:

	// name is used to display engaged timer in 'debug sched', or 'select ... from @@system.sched'
	// fnOnTimer callback is optional, that is ok to work without it, and just check TimeExceeded() periodically.
	explicit MiniTimer_c ( const char* szName = "mini-timer", Threads::Handler&& fnOnTimer=nullptr )
		: m_szName ( szName )
		, m_fnOnTimer { std::move ( fnOnTimer ) }
	{}

	/// on period<=0 does nothing, returns 0. On positive - engage tick after given period; returns timestamp where it should tick.
	int64_t Engage ( int64_t iTimePeriodMS );
	int64_t Engage ( int64_t iTimePeriodMS, Threads::Handler&& fnOnTimer ); // fnOnTimer may be set here alternatively to be set in ctr

	/// engage to be kicked at absolute time (in microseconds)
	void EngageAt ( int64_t iTimeStampUS );
	void EngageAt ( int64_t iTimeStampUS, Threads::Handler&& fnOnTimer ); // fnOnTimer may be set here alternatively to be set in ctr

	void 	UnEngage();

	// dtr calls UnEngage(), so it is safe to delete engaged timer
	~MiniTimer_c();
};