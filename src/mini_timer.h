//
// Copyright (c) 2021, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include <cstdint>

namespace sph
{
	/// how often timing thread 'ticks'. Default 1ms, but note that thread quantum might be different depending from OS!
	static const int MINI_TIMER_TICK_MS = 1;

	/// RAII timer thread ticker. While instance of the class exists and engaged - internal thread will tick every
	/// MINI_TIMER_TICK_MS milliseconds, and update internal timestamp.
	/// When all consumers gone - internal thread will sleep and not consume CPU time idling.
	class MiniTimer_c
	{
		bool m_bEngaged = false;

	public:
		/// that is made non-static, to avoid using engage outside living MiniTimer_c
		int64_t MiniTimerEngage ( int64_t iTimePeriodMS );

		~MiniTimer_c();
	};

	/// returns true if provided timestamp is already reached or not
	bool TimeExceeded ( int64_t tmMicroTimestamp );
}