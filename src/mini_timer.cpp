//
// Copyright (c) 2020, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "mini_timer.h"
#include "threadutils.h"

class TinyTimer_c
{
	std::atomic<int64_t> m_tmTimestamp { sphMicroTimer () };
	SphThread_t m_tCounterThread;

public:
	TinyTimer_c()
	{
		Threads::Create ( &m_tCounterThread, [this] {
			while (!sphInterrupted ())
			{
				m_tmTimestamp.store ( sphMicroTimer (), std::memory_order_relaxed );
				sphSleepMsec ( sph::MINI_TIMER_TICK_MS );
			}
		}, true, "sphTimer" );
	}

	int64_t MiniTimer ()
	{
		return m_tmTimestamp.load ( std::memory_order_relaxed );
	}

	bool TimeExceeded ( int64_t tmMicroTimestamp )
	{
		return MiniTimer() > tmMicroTimestamp;
	}
};

TinyTimer_c& g_TinyTimer()
{
	static TinyTimer_c tTimer;
	return tTimer;
}


/// millisecond precision timestamp, returned in microsecond (to be exchangable with sphMicroTimer)
int64_t sph::MiniTimer ()
{
	return g_TinyTimer().MiniTimer();
}

/// returns true if provided timestamp is already reached or not
bool sph::TimeExceeded ( int64_t tmMicroTimestamp )
{
	return g_TinyTimer ().TimeExceeded ( tmMicroTimestamp );
}