//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "std/generics.h"
#include "std/string.h"


// an event we use to wake up pollers (also used in net events in threadpool)
struct PollableEvent_t : ISphNoncopyable
{
public:
	PollableEvent_t ();
	virtual ~PollableEvent_t ();

	inline bool IsPollable () const { return m_iPollablefd!=-1; }
	void Close();

	/// fire an event
	bool FireEvent () const;

	/// remove fired event
	void DisposeEvent () const;

public:
	int				m_iPollablefd = -1; ///< listener's fd, to be used in pollers
	CSphString		m_sError;

protected:
	int m_iSignalEvent = -1; ///< firing fd, writing here will wake up m_iPollablefd
	static int PollableErrno();
};