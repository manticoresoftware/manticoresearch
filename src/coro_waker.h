//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "std/spinlock.h"
#include <boost/intrusive/slist.hpp>

namespace Threads
{
namespace Coro
{

class Worker_c;

namespace detail
{
using waker_queue_hook_t = boost::intrusive::slist_member_hook<> ;
}

class Waker_c
{
	Worker_c* m_pCtx {};
	size_t m_iEpoch {};
	friend class AtomicWaker_c;

public:
	Waker_c() = default;

	Waker_c ( Worker_c* pCtx, const size_t iEpoch )
		: m_pCtx { pCtx }
		, m_iEpoch { iEpoch }
	{}

	bool Wake ( bool bVip = false ) const noexcept;
};

class AtomicWaker_c
{
	std::atomic<Worker_c*> m_pCtx {nullptr};
	size_t m_iEpoch {};

public:
	AtomicWaker_c() = default;

	void Assign ( Waker_c&& tWaker )
	{
		m_iEpoch = tWaker.m_iEpoch;
		m_pCtx.store ( tWaker.m_pCtx, std::memory_order_relaxed );
	}

	bool WakeOnce ( bool bVip = false ) noexcept;
};

class WakerInQueue_c: public Waker_c
{
public:
	explicit WakerInQueue_c ( Waker_c&& w )
		: Waker_c { w }
	{}

	bool is_linked() const noexcept
	{
		return m_tWakerQueueHook.is_linked();
	}

	friend bool operator== ( Waker_c const& lhs, Waker_c const& rhs ) noexcept
	{
		return &lhs == &rhs;
	}

public:
	detail::waker_queue_hook_t m_tWakerQueueHook {};
};


namespace detail
{
using WakerSlist_t = boost::intrusive::slist<
		WakerInQueue_c,
		boost::intrusive::member_hook<
				WakerInQueue_c,
				detail::waker_queue_hook_t,
				&WakerInQueue_c::m_tWakerQueueHook>,
		boost::intrusive::constant_time_size<false>,
		boost::intrusive::cache_last<true>>;
}

class WaitQueue_c
{
private:
	detail::WakerSlist_t m_Slist {};

public:
	void SuspendAndWait ( sph::Spinlock_lock& l, Worker_c* ) REQUIRES (l);
	bool SuspendAndWaitUntil ( sph::Spinlock_lock& l, Worker_c*, int64_t ) REQUIRES (l);
	bool SuspendAndWaitForMS ( sph::Spinlock_lock& l, Worker_c*, int64_t ) REQUIRES (l);
	void NotifyOne();
	void NotifyAll();

	bool Empty() const;
};

} // namespace Coro
} // namespace Threads
