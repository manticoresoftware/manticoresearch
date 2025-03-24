//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "generics.h"
#include <mutex>
#include "ints.h"

/// fixme! use std::mutex, std::condition_variable

// event implementation
class EventWrapper_c: public ISphNoncopyable
{
public:
	EventWrapper_c();
	~EventWrapper_c();

	inline bool Initialized() const
	{
		return m_bInitialized;
	}

protected:
	bool m_bInitialized = false;

#if _WIN32
	CONDITION_VARIABLE m_tCond;
	CRITICAL_SECTION m_tMutex;
#else
	pthread_cond_t m_tCond;
	pthread_mutex_t m_tMutex;
#endif
};

template<bool bONESHOT = true>
class AutoEvent_T: public EventWrapper_c
{
public:
	// increase of set (oneshot) event's count and issue an event.
	void SetEvent();

	// decrease or reset (oneshot) event's count. If count empty, go to sleep until new events
	// returns true if event happened, false if timeout reached or event is not initialized
	bool WaitEvent ( int iMsec = -1 ); // -1 means 'infinite'

private:
	volatile int m_iSent = 0;
};

using CSphAutoEvent = AutoEvent_T<false>;
using OneshotEvent_c = AutoEvent_T<>;
