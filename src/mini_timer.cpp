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
	std::atomic<int>	m_iUsers {0};
	SphThread_t m_tCounterThread;
	bool m_bCreated = false;
	OneshotEvent_c m_tSignal;

	void Loop()
	{
		while ( !sphInterrupted () )
		{
			m_tSignal.WaitEvent ();
			while ( m_iUsers.load ( std::memory_order_relaxed )>0 )
			{
				TickTime();
				sphSleepMsec ( sph::MINI_TIMER_TICK_MS );
			}
		}
	}

	void TickTime()
	{
		m_tmTimestamp.store ( sphMicroTimer (), std::memory_order_relaxed );
	}

public:
	TinyTimer_c()
	{
		m_bCreated = Threads::Create ( &m_tCounterThread, [this] { Loop(); }, true, "sphTimer" );
	}

	~TinyTimer_c()
	{
		if ( m_bCreated )
		{
			m_tSignal.SetEvent ();
			Threads::Join ( &m_tCounterThread );
		}
	}

	int64_t MiniTimerEngage ( int64_t iTimePeriodMS )
	{
		TickTime();
		m_tSignal.SetEvent();
		return m_tmTimestamp.load ( std::memory_order_relaxed )+iTimePeriodMS * 1000;
	}

	bool TimeExceeded ( int64_t tmMicroTimestamp )
	{
		return m_tmTimestamp.load ( std::memory_order_relaxed )>tmMicroTimestamp;
	}

	void MiniTimerAcquire()
	{
		m_iUsers.fetch_add ( 1, std::memory_order_relaxed );
	}

	void MiniTimerRelease()
	{
		m_iUsers.fetch_sub ( 1, std::memory_order_relaxed );
	}
};

TinyTimer_c& g_TinyTimer()
{
	static TinyTimer_c tTimer;
	return tTimer;
}

sph::MiniTimer_c::~MiniTimer_c ()
{
	if ( m_bEngaged )
		g_TinyTimer ().MiniTimerRelease ();
}

int64_t sph::MiniTimer_c::MiniTimerEngage ( int64_t iTimePeriodMS )
{
	if ( !m_bEngaged )
	{
		g_TinyTimer ().MiniTimerAcquire ();
		m_bEngaged = true;
	}
	return g_TinyTimer ().MiniTimerEngage ( iTimePeriodMS );
}

/// returns true if provided timestamp is already reached or not
bool sph::TimeExceeded ( int64_t tmMicroTimestamp )
{
	return g_TinyTimer ().TimeExceeded ( tmMicroTimestamp );
}
