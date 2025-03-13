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
#include "unique_lock.h"

#include <boost/fiber/detail/spinlock.hpp>

namespace sph {

// wrapper over boost::fibers::detail::spinlock to provide thread-annotation capabilities
// naming convention follows std::mutex, and so, func may be used with std::unique_lock, etc.
class CAPABILITY ( "mutex" ) Spinlock_c
{
	boost::fibers::detail::spinlock m_tLock {};

public:
	Spinlock_c() = default;
	Spinlock_c ( Spinlock_c const& ) = delete;
	Spinlock_c& operator= ( Spinlock_c const& ) = delete;

	void lock() noexcept ACQUIRE()
	{
		m_tLock.lock();
	}

	bool try_lock() noexcept TRY_ACQUIRE ( true )
	{
		return m_tLock.try_lock();
	}

	void unlock() noexcept UNLOCK_FUNCTION()
	{
		m_tLock.unlock();
	}

};

using Spinlock_lock = sph::unique_lock<Spinlock_c>;

} // namespace sph