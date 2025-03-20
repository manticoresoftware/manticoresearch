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

#include <mutex>

namespace sph
{

// tiny wrapper over std::unique_lock to provide thread-annotation capabilities
// naming convention follows std::unique_lock
template<typename Mutex>
class SCOPED_CAPABILITY unique_lock
{
	std::unique_lock<Mutex> m_tLock;

public:
	unique_lock() noexcept = default;
	unique_lock ( unique_lock const& ) = delete;
	unique_lock& operator= ( unique_lock const& ) = delete;

	explicit unique_lock ( Mutex& m ) ACQUIRE ( m )
		: m_tLock { m } {}

	unique_lock ( Mutex& m, std::defer_lock_t t ) noexcept EXCLUDES ( m )
		: m_tLock { m, t } {}

	unique_lock ( Mutex& m, std::try_to_lock_t t ) ACQUIRE ( m )
		: m_tLock { m, t } {}

	unique_lock ( Mutex& m, std::adopt_lock_t t ) REQUIRES ( m )
		: m_tLock { m, t } {}

	template<class Clock, class Duration>
		unique_lock ( Mutex& m, const std::chrono::time_point<Clock, Duration>& t ) ACQUIRE ( m )
		: m_tLock { m, t } {}

	template<class Rep, class Period>
		unique_lock ( Mutex& m, const std::chrono::duration<Rep, Period>& d ) ACQUIRE ( m )
		: m_tLock { m, d } {}

	unique_lock ( unique_lock&& u ) noexcept ACQUIRE ( ) RELEASE ( u ) = default;
	unique_lock& operator= ( unique_lock&& u ) noexcept ACQUIRE ( ) RELEASE ( u ) = default;

	~unique_lock() RELEASE() = default;

	void lock() ACQUIRE()
	{
		m_tLock.lock();
	}

	void unlock() RELEASE()
	{
		m_tLock.unlock();
	}

	bool try_lock() TRY_ACQUIRE ( true )
	{
		return m_tLock.try_lock();
	}

	template<class Rep, class Period>
	bool try_lock_for ( const std::chrono::duration<Rep, Period>& d ) TRY_ACQUIRE ( true )
	{
		return m_tLock.try_lock_for (d);
	}

	template<class Clock, class Duration>
	bool try_lock_until ( const std::chrono::time_point<Clock, Duration>& t ) TRY_ACQUIRE ( true )
	{
		return m_tLock.try_lock_until (t);
	}

	void swap ( unique_lock& u ) noexcept
	{
		m_tLock.swap ( u.m_tLock );
	}

	bool owns_lock() const noexcept { return m_tLock.owns_lock(); }
	operator bool() const noexcept { return m_tLock(); }
	Mutex* mutex() const noexcept { return m_tLock.mutex(); }
	Mutex* release() noexcept RELEASE() { return m_tLock.release(); }
};

template<class Mutex>
inline void swap ( unique_lock<Mutex>& x, unique_lock<Mutex>& y ) noexcept
{
	x.swap ( y );
}

} // namespace sph