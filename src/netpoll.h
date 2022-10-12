//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

#include "config.h"
#include "searchdaemon.h"

#include "std/ints.h"
#include "std/refcounted_mt.h"
#include "std/threadrole.h"
#include "timeout_queue.h"

#include <boost/intrusive/slist.hpp>
#include <functional>
#include <memory>

//////////////////////////////////////////////////////////////////////////
// Universal work with select/poll/epoll/kqueue
//////////////////////////////////////////////////////////////////////////
// wrapper around epoll/kqueue/poll

extern ThreadRole NetPoollingThread;
using netlist_hook_t = boost::intrusive::slist_member_hook<>;


struct NetPollEvent_t : public EnqueuedTimeout_t, public ISphRefcountedMT
{
	netlist_hook_t		m_tBackHook;		// opaque hook for intrusive list linkage
	int					m_iBackIdx = -1;	// or opaque index to internals of poller

	int					m_iSock = -1;
	volatile DWORD		m_uNetEvents = 0;

	explicit NetPollEvent_t ( int iSock )
		: m_iSock ( iSock ) {}

	enum Events_e : DWORD
	{
		READ = 1UL << 0, // 1
		WRITE = 1UL << 1, // 2
		HUP = 1UL << 2, // 4
		ERR = 1UL << 3, // 8
//		PRI = 1UL << 4, // 16
		ONCE = 1UL << 4, // effective when set up // 32
		TIMEOUT = ONCE, // effective when return back // 32
		CLOSED = 1UL << 8,
	};

	bool IsLinked() const
	{
		return m_tBackHook.is_linked() || m_iBackIdx!=-1;
	}
};

using NetPoolEventRefPtr_c = CSphRefcountedPtr<NetPollEvent_t>;

constexpr static int64_t WAIT_UNTIL_TIMEOUT = -1LL;

class NetPooller_c;
class NetPollReadyIterator_c
{
	int m_iIterEv = -1;
	NetPooller_c * m_pOwner = nullptr;
public:
	explicit NetPollReadyIterator_c ( NetPooller_c* pOwner ) : m_pOwner ( pOwner )
	{
		if ( pOwner )
			operator++();
	}
	NetPollEvent_t & operator* ()									REQUIRES ( NetPoollingThread );
	NetPollReadyIterator_c & operator++ ()						 	REQUIRES ( NetPoollingThread );
	bool operator!= ( const NetPollReadyIterator_c & rhs ) const	REQUIRES ( NetPoollingThread );
};

class NetPooller_c : public ISphNoncopyable
{
	class Impl_c;
	std::unique_ptr<Impl_c> m_pImpl;
	friend class NetPollReadyIterator_c;

public:
	explicit NetPooller_c ( int iSizeHint );
	~NetPooller_c();
	void SetupEvent ( NetPollEvent_t * pEvent )				REQUIRES ( NetPoollingThread );
	void Wait ( int64_t iUS )								REQUIRES ( NetPoollingThread );
	int GetNumOfReady () const;
	void ProcessAll ( std::function<void (NetPollEvent_t*)>&& fnAction ) REQUIRES ( NetPoollingThread );
	void RemoveTimeout ( NetPollEvent_t * pEvent )			REQUIRES ( NetPoollingThread );
	void RemoveEvent ( NetPollEvent_t * pEvent )			REQUIRES ( NetPoollingThread );

	int64_t TickGranularity() const;

	NetPollReadyIterator_c begin () { return NetPollReadyIterator_c ( this ); }
	static NetPollReadyIterator_c end () { return NetPollReadyIterator_c ( nullptr ); }
};




// determine which branch will be used
// defs placed here for easy switch between/debug
#define NETPOLL_EPOLL 1
#define NETPOLL_KQUEUE 2
#define NETPOLL_POLL 3

#if HAVE_EPOLL
#define POLLING_EPOLL 1
#define NETPOLL_TYPE NETPOLL_EPOLL
#elif HAVE_KQUEUE
#define POLLING_KQUEUE 1
#define NETPOLL_TYPE NETPOLL_KQUEUE
#elif HAVE_POLL
#define POLLING_POLL 1
#define NETPOLL_TYPE NETPOLL_POLL
#endif

// #define NETPOLL_TYPE NETPOLL_POLL
