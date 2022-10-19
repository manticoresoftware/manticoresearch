//
// Copyright (c) 2021-2022, Manticore Software LTD (https://manticoresearch.com)
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
#include <boost/intrusive/slist.hpp>

namespace sph
{
	/// we use useconds for timestamps, however sleep obey mseconds. Interval less than granularity should be considered as 0, timestamps inside granularity should be considered as equal
	constexpr int64_t TICKS_GRANULARITY = 1000ll;

	inline bool TimeExceeded ( int64_t iTimestampUS, int64_t iPivotUS, int64_t GRANULARITY=TICKS_GRANULARITY )
	{
		assert ( iPivotUS > 0 );
		return ( iTimestampUS - GRANULARITY ) < iPivotUS;
	}

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

	using TimerHook_t = boost::intrusive::slist_member_hook<>;
}

/// RAII timer thread ticker.
class MiniTimer_c: public EnqueuedTimeout_t
{
	friend class TinyTimer_c;
	virtual void OnTimer() {};

public:
	sph::TimerHook_t m_tLink; // used to link timer in the queue
	const char* m_szName = "mini-timer"; // hack! name and also flag that timer should be unlinked on destroy (if nullptr - dtr will do nothing)

public:
	/// on period<=0 does nothing, returns 0. On positive - engage tick after given period; returns timestamp where it should tick.
	int64_t Engage ( int64_t iTimePeriodMS );
	int64_t EngageUS ( int64_t iTimePeriodUS ); // same, but period is in microseconds

	void 	UnEngage();

	/// if m_szName is not null - unengage and unlink everything
	virtual ~MiniTimer_c();
};