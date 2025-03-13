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

#include "thread_annotations.h"
#include "generics.h"
#include "ints.h"

#include <mutex>

#if _WIN32
using TMutex = HANDLE;
#else
using TMutex = pthread_mutex_t;
#endif

/// mutex implementation
class CAPABILITY ( "mutex" ) CSphMutex: public ISphNoncopyable
{
public:
	CSphMutex() noexcept;
	~CSphMutex() noexcept;

	bool Lock() ACQUIRE();
	bool Unlock() RELEASE();
	bool TimedLock ( int iMsec ) TRY_ACQUIRE ( true );

	// Just for clang negative capabilities.
	const CSphMutex& operator!() const { return *this; }

	using mtx = TMutex;
	mtx& mutex() RETURN_CAPABILITY ( this )
	{
		return m_tMutex;
	}

protected:
	TMutex m_tMutex;
};

#include "scopedlock.h"

using ScopedMutex_t = CSphScopedLock<CSphMutex>;