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

#include "netpoll.h"

#include "std/timers.h"
#include "searchdaemon.h"
#include <memory>

#if HAVE_KQUEUE
#include <sys/event.h>
#endif

class TimeoutEvents_c
{
	TimeoutQueue_c	m_dTimeouts;

public:
	constexpr static int64_t TIME_INFINITE = -1;
	constexpr static int64_t TIME_IMMEDIATE = 0;

	void AddOrChangeTimeout ( EnqueuedTimeout_t * pEvent )
	{
		if ( pEvent->m_iTimeoutTimeUS>=0 )
			m_dTimeouts.Change ( pEvent );
	}

	void RemoveTimeout ( EnqueuedTimeout_t * pEvent )
	{
		m_dTimeouts.Remove ( pEvent );
	}

	int64_t GetNextTimeoutUS ( int64_t iGranularity )
	{
		auto iDefaultTimeoutUS = TIME_INFINITE;
		while ( !m_dTimeouts.IsEmpty() )
		{
			auto * pNetEvent = (EnqueuedTimeout_t *) m_dTimeouts.Root ();
			assert ( pNetEvent->m_iTimeoutTimeUS>0 );

			auto iNextTimeoutUS = pNetEvent->m_iTimeoutTimeUS - MonoMicroTimer();
			if ( iNextTimeoutUS > iGranularity )
				return iNextTimeoutUS;
			else
				iDefaultTimeoutUS = TIME_IMMEDIATE;

			m_dTimeouts.Pop ();
		}
		return iDefaultTimeoutUS;
	}
};

#if ( NETPOLL_TYPE==NETPOLL_KQUEUE || NETPOLL_TYPE==NETPOLL_EPOLL )

// both epoll and kqueue have events in hash, so we use also linked list to track them explicitly.
// both stores pointer to the list item in pPtr of backend

using NetPollEventsList_t = boost::intrusive::slist<NetPollEvent_t,
	boost::intrusive::member_hook<NetPollEvent_t, netlist_hook_t, &NetPollEvent_t::m_tBackHook>,
	boost::intrusive::constant_time_size<true>,
	boost::intrusive::cache_last<true>>;

// specific different function for kqueue/epoll
// keep them elementary and tiny for easy debugging across platforms

#if ( NETPOLL_TYPE == NETPOLL_KQUEUE )
struct PollTraits_t
{
	using pollev = struct kevent;
	constexpr static int64_t poll_granularity = 1LL;

	inline static int create_poller ( int ) { return kqueue(); }
	inline static void close_poller ( int iPl )
	{
		if ( iPl >= 0 )
			sphSockClose ( iPl );
	}
	inline static void* get_data ( const pollev& tEv ) { return tEv.udata; }
	inline static void translate_events ( const pollev& tEv )
	{
		auto* pNode = (NetPollEvent_t*)get_data ( tEv );
		assert ( pNode && "deleted event recognized" );
		if ( !pNode )
			return;

		pNode->m_uGotEvents = ( ( tEv.flags & EV_ERROR ) ? NetPollEvent_t::IS_ERR : 0 )
							| ( ( tEv.flags & EV_EOF ) ? NetPollEvent_t::IS_HUP : 0 )
							| ( ( tEv.filter == EVFILT_READ ) ? NetPollEvent_t::IS_READ : NetPollEvent_t::IS_WRITE );

		if ( !( pNode->m_uIOActive & NetPollEvent_t::SET_ONESHOT ) )
			return;

		if ( pNode->m_uIOActive & NetPollEvent_t::SET_READ )
			pNode->m_uIOActive &= ~( NetPollEvent_t::SET_READ );
		if ( pNode->m_uIOActive & NetPollEvent_t::SET_WRITE )
			pNode->m_uIOActive &= ~( NetPollEvent_t::SET_WRITE );
	}


	inline static int poll_events ( int iPoll, pollev* pEvents, int iEventNum, int64_t timeoutUS )
	{
		timespec ts;
		return kevent ( iPoll, nullptr, 0, pEvents, iEventNum, US2timespec ( ts, timeoutUS ) );
	};

	inline static int set_polling_for ( int iPoll, int iSock, void* pData, BYTE uIOActive, BYTE uIOChange, bool )
	{
		bool bWrite = uIOChange & NetPollEvent_t::SET_WRITE;
		bool bRead = uIOChange & NetPollEvent_t::SET_READ;
		bool bWasWrite = uIOActive & NetPollEvent_t::SET_WRITE;
		bool bWasRead = uIOActive & NetPollEvent_t::SET_READ;

		auto iOp = EV_ADD
				 | ( ( uIOChange & NetPollEvent_t::SET_ON_EDGE ) ? EV_CLEAR : 0 )
				 | ( ( uIOChange & NetPollEvent_t::SET_ONESHOT ) ? EV_ONESHOT : 0 );

		struct kevent tEv[2];
		auto pEv = &tEv[0];

		// boring combination matrix below
		if ( bRead && !bWasRead )
			EV_SET ( pEv++, iSock, EVFILT_READ, iOp, 0, 0, pData );

		if ( bWrite && !bWasWrite )
			EV_SET ( pEv++, iSock, EVFILT_WRITE, iOp, 0, 0, pData );

		if ( !bRead && bWasRead )
			EV_SET ( pEv++, iSock, EVFILT_READ, EV_DELETE, 0, 0, pData );

		if ( !bWrite && bWasWrite )
			EV_SET ( pEv++, iSock, EVFILT_WRITE, EV_DELETE, 0, 0, pData );

		const int nEvs = pEv - tEv;
		assert ( nEvs <= 2 );
		for ( int i = 0; i < nEvs; ++i )
			sphLogDebugv ( "%p kqueue %d setup, ev=%d, fl=%d sock=%d", pData, iPoll, tEv[i].filter, tEv[i].flags, iSock );

		return kevent ( iPoll, tEv, nEvs, nullptr, 0, nullptr );
	}

	inline static int polling_size ( int iQueueSize, int iMaxReady )
	{
		return iMaxReady ? 2 * Min ( iMaxReady, iQueueSize ) : 2 * iQueueSize;
	}
private:
	inline static timespec* US2timespec ( timespec& ts, int64_t timeoutUS )
	{
		timespec* pts = nullptr;
		if ( timeoutUS >= 0 )
		{
			ts.tv_sec = timeoutUS / 1000000;
			ts.tv_nsec = (long)( timeoutUS - ts.tv_sec * 1000000 ) * 1000;
			pts = &ts;
		}
		return pts;
	}
};

#elif ( NETPOLL_TYPE == NETPOLL_EPOLL )

struct PollTraits_t
{
	using pollev = epoll_event;
	constexpr static int64_t poll_granularity = 1000LL;

	inline static int create_poller ( int iSizeHint ) { return epoll_create ( iSizeHint ); }
	inline static void close_poller ( int iPl )
	{
		if ( iPl >= 0 )
			sphSockClose ( iPl );
	}
	inline static void* get_data ( const pollev& tEv ) { return tEv.data.ptr; }
	inline static void translate_events ( const pollev& tEv )
	{
		auto* pNode = (NetPollEvent_t*)get_data ( tEv );
		assert ( pNode && "deleted event recognized" );
		if ( !pNode )
			return;

		pNode->m_uGotEvents = ( ( tEv.events & EPOLLERR ) ? NetPollEvent_t::IS_ERR : 0 )
							| ( ( tEv.events & EPOLLHUP ) ? NetPollEvent_t::IS_HUP : 0 )
							| ( ( tEv.events & EPOLLIN ) ? NetPollEvent_t::IS_READ : 0 )
							| ( ( tEv.events & EPOLLOUT ) ? NetPollEvent_t::IS_WRITE : 0 );
	}

	inline static int US2Polltime ( int64_t timeoutUS )
	{
		switch ( timeoutUS )
		{
		case TimeoutEvents_c::TIME_INFINITE: return -1;
		case TimeoutEvents_c::TIME_IMMEDIATE: return 0;
		default: return timeoutUS / poll_granularity;
		}
	}

	inline static int poll_events ( int iPoll, pollev* pEvents, int iEventNum, int64_t timeoutUS )
	{
		return epoll_wait ( iPoll, pEvents, iEventNum, US2Polltime ( timeoutUS ) );
	};

	inline static const char* epoll_action_name ( int iOp )
	{
		switch (iOp) { case EPOLL_CTL_ADD:return "EPOLL_CTL_ADD"; case EPOLL_CTL_MOD:return "EPOLL_CTL_MOD"; case EPOLL_CTL_DEL:return "EPOLL_CTL_DEL"; default:return "UNKNWON";};
	}

	inline static int set_polling_for ( int iPoll, int iSock, void* pData, BYTE, BYTE uIOChange, bool bAdd )
	{
		bool bRW = uIOChange & NetPollEvent_t::SET_RW;
		if ( !bRW && !bAdd )
			return 0;

		int iOp = bRW ? ( bAdd ? EPOLL_CTL_ADD : EPOLL_CTL_MOD ) : EPOLL_CTL_DEL;

		epoll_event tEv;
		if ( bRW )
		{
			tEv.data.ptr = pData;
			tEv.events = ( ( uIOChange & NetPollEvent_t::SET_ON_EDGE ) ? EPOLLET : 0 )
					   | ( ( uIOChange & NetPollEvent_t::SET_ONESHOT ) ? EPOLLONESHOT : 0 )
					   | ( ( uIOChange & NetPollEvent_t::SET_READ ) ? EPOLLIN : 0 )
					   | ( ( uIOChange & NetPollEvent_t::SET_WRITE ) ? EPOLLOUT : 0 );
			sphLogDebugv ( "%p epoll %d setup, ev=0x%u, op=%s, sock=%d", pData, iPoll, tEv.events, epoll_action_name ( iOp ), iSock );
		} else
			sphLogDebugv ( "%p epoll %d setup, op=%s, sock=%d", pData, iPoll, epoll_action_name ( iOp ), iSock );


		return epoll_ctl ( iPoll, iOp, iSock, &tEv );
	}

	inline static int polling_size ( int iQueueSize, int iMaxReady )
	{
		return iMaxReady ? Min ( iMaxReady, iQueueSize ) : iQueueSize;
	}
};

#endif

// need for remove from intrusive list to work
inline bool operator== ( const NetPollEvent_t& lhs, const NetPollEvent_t& rhs )
{
		return &lhs == &rhs;
}

// common for both epoll and kqueue
class NetPooller_c::Impl_c final : public PollTraits_t
{
	friend class NetPooller_c;
	friend class NetPollReadyIterator_c;

	TimeoutEvents_c				m_dTimeouts			GUARDED_BY ( NetPoollingThread );
	CSphVector<pollev>			m_dFiredEvents		GUARDED_BY ( NetPoollingThread );
	NetPollEventsList_t			m_tEvents			GUARDED_BY ( NetPoollingThread ); // used for 'for_all'

	int							m_iReady = 0;
	const int 					m_iMaxReady;
	int							m_iLastReportedErrno = -1;
	int							m_iPl;

public:

	explicit Impl_c ( int iSizeHint, int iMaxReady )
		: m_iMaxReady (iMaxReady)
	{
		m_iPl = create_poller ( iSizeHint );
		if ( m_iPl==-1 )
			sphDie ( "failed to create poller main FD, errno=%d, %s", errno, strerrorm ( errno ) );

		sphLogDebugv ( "poller %d created", m_iPl );
		m_dFiredEvents.Reserve ( iSizeHint );
	}

	~Impl_c ()
	{
		sphLogDebugv ( "poller %d closed", m_iPl );
		close_poller ( m_iPl );
	}

	// called from working netloop routine
	void SetupEvent ( NetPollEvent_t * pEvent ) REQUIRES ( NetPoollingThread )
	{
		if ( pEvent->m_uIOChange == NetPollEvent_t::SET_CLOSED || pEvent->m_uIOChange == NetPollEvent_t::SET_NONE )
			return RemoveEvent ( pEvent );

		assert ( pEvent && pEvent->m_iSock>=0 );
		assert ( pEvent->m_uIOChange & NetPollEvent_t::SET_RW );

		m_dTimeouts.AddOrChangeTimeout ( pEvent );

		bool bIsNew = !pEvent->m_tBackHook.is_linked();
		if ( bIsNew )
		{
			SafeAddRef ( pEvent );
			m_tEvents.push_back ( *pEvent );
		}

		int iRes = set_polling_for ( m_iPl, pEvent->m_iSock, pEvent, pEvent->m_uIOActive, pEvent->m_uIOChange, bIsNew );
		pEvent->m_uIOActive = pEvent->m_uIOChange;

		if ( iRes == -1 )
			sphWarning ( "failed to setup queue event for sock %d, errno=%d, %s", pEvent->m_iSock, errno, strerrorm ( errno ) );
	}

	// called when client detected error or timeout, and even when netloop routine is not active (i.e., where it is stopped)
	void RemoveEvent ( NetPollEvent_t* pEvent ) REQUIRES ( NetPoollingThread )
	{
		assert ( pEvent );
		RemoveTimeout ( pEvent );

		sphLogDebugv ( "%p polling remove, ev=%u, sock=%d", pEvent, pEvent->m_uIOChange, pEvent->m_iSock );

		if ( pEvent->m_uIOChange != NetPollEvent_t::SET_CLOSED )
		{
			pEvent->m_uIOChange = NetPollEvent_t::SET_NONE;
			int iRes = set_polling_for ( m_iPl, pEvent->m_iSock, pEvent, pEvent->m_uIOActive, 0, true );

			// might be already closed by worker from thread pool
			if ( iRes == -1 )
				sphLogDebugv ( "failed to remove polling event for sock %d(%p), errno=%d, %s", pEvent->m_iSock, pEvent, errno, strerrorm ( errno ) );
		}

		// since event already removed from kqueue - it is safe to remove it from the list of events also,
		// and totally unlink
		if ( pEvent->m_tBackHook.is_linked() )
		{
			m_tEvents.remove ( *pEvent );
			SafeRelease ( pEvent );
		}
	}

	void Wait ( int64_t iUS ) REQUIRES ( NetPoollingThread )
	{
		if ( m_tEvents.empty() )
			return;

		if ( iUS==WAIT_UNTIL_TIMEOUT )
			iUS = m_dTimeouts.GetNextTimeoutUS ( poll_granularity );

		m_dFiredEvents.Resize ( polling_size ( m_tEvents.size(), m_iMaxReady ) );

		// need positive timeout for communicate threads back and shutdown
		m_iReady = poll_events ( m_iPl, m_dFiredEvents.Begin (), m_dFiredEvents.GetLength (), iUS );

		if ( m_iReady>=0 )
			return;

		int iErrno = sphSockGetErrno ();
		// common recoverable errors
		if ( iErrno==EINTR || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
			return;

		if ( m_iLastReportedErrno!=iErrno )
		{
			sphWarning ( "polling tick failed: %s", sphSockError ( iErrno ) );
			m_iLastReportedErrno = iErrno;
		}
	}

	void ProcessAll ( std::function<void ( NetPollEvent_t * )>&& fnAction ) REQUIRES ( NetPoollingThread )
	{
		// not ranged-for here, as postfix ++ action for iterator is important (as fnAction can remove elem from list)
		for ( auto it { m_tEvents.begin() }, itend { m_tEvents.end() }; it != itend; )
			fnAction ( &*it++ );
	}

	int GetNumOfReady () const
	{
		return m_iReady;
	}

	void RemoveTimeout ( NetPollEvent_t * pEvent ) REQUIRES ( NetPoollingThread )
	{
		assert ( pEvent );
		m_dTimeouts.RemoveTimeout ( pEvent );
	}
};

// more common for NETPOLL_TYPE==NETPOLL_KQUEUE || NETPOLL_TYPE==NETPOLL_EPOLL
NetPollEvent_t & NetPollReadyIterator_c::operator* ()
{
	auto & pOwner = m_pOwner->m_pImpl;
	const auto & tEv = pOwner->m_dFiredEvents[m_iIterEv];
	PollTraits_t::translate_events ( tEv );
	return *(NetPollEvent_t*)PollTraits_t::get_data ( tEv );
};

NetPollReadyIterator_c & NetPollReadyIterator_c::operator++ ()
{
	++m_iIterEv;
	return *this;
}

bool NetPollReadyIterator_c::operator!= ( const NetPollReadyIterator_c & rhs ) const
{
	auto & pOwner = m_pOwner->m_pImpl;
	return rhs.m_pOwner || m_iIterEv<pOwner->m_iReady;
}

#endif // #if ( NETPOLL_TYPE==NETPOLL_KQUEUE || NETPOLL_TYPE==NETPOLL_EPOLL )

#if ( NETPOLL_TYPE == NETPOLL_POLL )

struct PollTraits_t
{
	constexpr static int64_t poll_granularity = 1000LL;

	inline static void set_polling_for ( pollfd& tEv, int iSock, BYTE uIOChange )
	{
		tEv.fd = iSock;
		tEv.events = ( ( uIOChange & NetPollEvent_t::SET_READ ) ? POLLIN : 0 )
				   | ( ( uIOChange & NetPollEvent_t::SET_WRITE ) ? POLLOUT : 0 );

	}

	inline static int poll_events ( pollfd* pBegin, int iLength, int64_t timeoutUS )
	{
		return ::poll ( pBegin, iLength, US2Polltime ( timeoutUS ) );
	};

	inline static void translate_events ( const pollfd& tEv, NetPollEvent_t* pNode )
	{
		pNode->m_uGotEvents = ( ( tEv.revents & POLLERR ) ? NetPollEvent_t::IS_ERR : 0 )
							| ( ( tEv.revents & POLLHUP ) ? NetPollEvent_t::IS_HUP : 0 )
							| ( ( tEv.revents & POLLIN ) ? NetPollEvent_t::IS_READ : 0 )
							| ( ( tEv.revents & POLLOUT ) ? NetPollEvent_t::IS_WRITE : 0 );
	}

private:
	inline static int US2Polltime ( int64_t timeoutUS )
	{
		switch ( timeoutUS )
		{
		case TimeoutEvents_c::TIME_INFINITE: return -1;
		case TimeoutEvents_c::TIME_IMMEDIATE: return 0;
		default: return timeoutUS / poll_granularity;
		}
	}
};

class NetPooller_c::Impl_c final : public PollTraits_t
{
	friend class NetPooller_c;
	friend class NetPollReadyIterator_c;

	TimeoutEvents_c					m_dTimeouts			GUARDED_BY ( NetPoollingThread );
	CSphVector<NetPollEvent_t *>	m_dWork				GUARDED_BY ( NetPoollingThread );
	CSphVector<pollfd>				m_dEvents			GUARDED_BY ( NetPoollingThread );

	int								m_iReady = 0;
	int								m_iLastReportedErrno = -1;

public:
	explicit Impl_c ( int iSizeHint, int iMaxReady )
	{
		m_dWork.Reserve ( iSizeHint );
		m_dEvents.Reserve ( iSizeHint );
		if ( iMaxReady!=0 )
			sphWarning ( "Setting 'net_throttle_action' is not supported with 'poll' network processor. Discarded.");
	}

	// called from working netloop routine
	void SetupEvent ( NetPollEvent_t * pEvent ) REQUIRES ( NetPoollingThread )
	{
		if ( pEvent->m_uIOChange == NetPollEvent_t::SET_CLOSED || pEvent->m_uIOChange == NetPollEvent_t::SET_NONE )
			return RemoveEvent ( pEvent );

		assert ( m_dEvents.GetLength ()==m_dWork.GetLength () );
		assert ( pEvent && pEvent->m_iSock>=0 );
		assert ( pEvent->m_uIOChange & NetPollEvent_t::SET_RW );

		m_dTimeouts.AddOrChangeTimeout ( pEvent );

		pollfd* pEv = nullptr;
		auto& iEventBackIdx = pEvent->m_iBackIdx;
		if ( iEventBackIdx>=0 && iEventBackIdx<m_dWork.GetLength () && !m_dWork[iEventBackIdx] ) // was already enqueued, just change events
		{
			m_dWork[iEventBackIdx] = pEvent;
			pEv = &m_dEvents[iEventBackIdx];
			sphLogDebugvv ( "SetupEvent [%d] old %d", pEvent->m_iBackIdx, pEvent->m_iSock );
		} else
		{
			SafeAddRef ( pEvent );
			iEventBackIdx = m_dWork.GetLength ();
			m_dWork.Add ( pEvent );
			pEv = &m_dEvents.Add ();
			sphLogDebugvv ( "SetupEvent [%d] new %d", pEvent->m_iBackIdx, pEvent->m_iSock );
		}

		set_polling_for ( *pEv, pEvent->m_iSock, pEvent->m_uIOChange );
		pEvent->m_uIOActive = pEvent->m_uIOChange;
		sphLogDebugv ( "SetupEvent [%d] for %d events %d", pEvent->m_iBackIdx, pEvent->m_iSock, pEv->events );
	}

	// called when client detected error or timeout, and even when netloop routine is not active (i.e., where it is stopped)
	void RemoveEvent ( NetPollEvent_t* pEvent ) REQUIRES ( NetPoollingThread )
	{
		assert ( pEvent );
		RemoveTimeout ( pEvent );

		sphLogDebugvv ( "RemoveEvent for %d, fd=%d", pEvent->m_iBackIdx, pEvent->m_iSock );

		if ( pEvent->m_iBackIdx < 0 ) // already removed by iteration
			return;

		assert ( pEvent->m_iBackIdx < m_dEvents.GetLength() );

		m_dEvents[pEvent->m_iBackIdx].fd = -1;
		m_dWork[pEvent->m_iBackIdx] = nullptr;
		pEvent->m_iBackIdx = -1;
		SafeRelease ( pEvent );
	}


	void Wait ( int64_t iUS ) REQUIRES ( NetPoollingThread )
	{
		m_iReady = 0;
		if ( m_dEvents.IsEmpty() )
			return;

		// need positive timeout for communicate threads back and shutdown
		if ( iUS == WAIT_UNTIL_TIMEOUT )
			iUS = m_dTimeouts.GetNextTimeoutUS ( poll_granularity );

		m_dEvents.for_each ( [] ( pollfd& dEv ) { dEv.revents = 0; } );
		m_iReady = poll_events ( m_dEvents.Begin (), m_dEvents.GetLength (), iUS );

		if ( m_iReady>=0 )
		{
			sphLogDebugvv ( "Wait returned %d events", m_iReady );
			return;
		}

		int iErrno = sphSockGetErrno ();
		sphLogDebugvv ( "Wait returned %d events with %d after-error", m_iReady, iErrno );
		// common recoverable errors
		if ( iErrno==EINTR || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
			return;

		if ( m_iLastReportedErrno!=iErrno )
		{
			sphWarning ( "polling tick failed: %s", sphSockError ( iErrno ) );
			m_iLastReportedErrno = iErrno;
		}
	}

	void ProcessAll ( std::function<void ( NetPollEvent_t * )>&& fnAction ) REQUIRES ( NetPoollingThread )
	{
		ARRAY_FOREACH ( i, m_dWork )
		{
			auto* pNode = m_dWork[i];
			if ( pNode && pNode->m_iBackIdx >= 0 )
			{
				pNode->m_iBackIdx = i; // adjust index, it might be broken because of RemoveFast
				fnAction ( pNode );
			}
			else
			{
				m_dEvents.RemoveFast ( i );
				m_dWork.RemoveFast ( i );
				--i;
			}
		}
		assert ( m_dEvents.GetLength ()==m_dWork.GetLength () );
	}

	int GetNumOfReady () const
	{
		return m_iReady;
	}

	void RemoveTimeout ( NetPollEvent_t * pEvent ) REQUIRES ( NetPoollingThread )
	{
		assert ( pEvent );
		m_dTimeouts.RemoveTimeout ( pEvent );
	}
};


// on windows pollfd.fd is unsigned for some unknown reason, hence the warning
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4146)
#endif

// trick: here we unlink ready oneshoted from the list
NetPollEvent_t & NetPollReadyIterator_c::operator* ()
{
	auto & pOwner = m_pOwner->m_pImpl;
	auto & tEv = pOwner->m_dEvents[m_iIterEv];

	NetPollEvent_t * pNode = pOwner->m_dWork[m_iIterEv];

	sphLogDebugvv ( "[%d] tEv.revents = %d for %d(%d)", m_iIterEv, tEv.revents, pNode->m_iSock, tEv.fd );
	PollTraits_t::translate_events ( tEv, pNode );

	if ( pNode->m_uIOActive & NetPollEvent_t::SET_ONESHOT )
	{
		tEv.fd = -tEv.fd;
		pOwner->m_dWork[m_iIterEv] = nullptr;
		pNode->m_iBackIdx = -1;
		pNode->m_uIOActive &= ~( NetPollEvent_t::SET_ONESHOT );
		pNode->Release();
	}

	return *pNode;
}

#ifdef _WIN32
#pragma warning(pop)
#endif

NetPollReadyIterator_c & NetPollReadyIterator_c::operator++ ()
{
	auto & pOwner = m_pOwner->m_pImpl;
	while (true)
	{
		++m_iIterEv;
		if ( m_iIterEv>=pOwner->m_dEvents.GetLength() )
			break;

		pollfd& tEv = pOwner->m_dEvents[m_iIterEv];

		if ( tEv.fd>=0 && tEv.revents!=0 && tEv.revents!=POLLNVAL )
		{
			sphLogDebugvv ( "operator++ on m_iIterEv as matched %d and %d", tEv.fd, tEv.revents );
			break;
		}
	}
	return *this;
}

bool NetPollReadyIterator_c::operator!= ( const NetPollReadyIterator_c & rhs ) const
{
	auto & pOwner = m_pOwner->m_pImpl;
	return rhs.m_pOwner || m_iIterEv<pOwner->m_dEvents.GetLength();
}
#endif


NetPooller_c::NetPooller_c ( int isizeHint, int iMaxReady )
	: m_pImpl ( std::make_unique<Impl_c> ( isizeHint, iMaxReady ) )
{}

NetPooller_c::~NetPooller_c () = default;

void NetPooller_c::SetupEvent ( NetPollEvent_t * pEvent )
{
	m_pImpl->SetupEvent ( pEvent );
}

void NetPooller_c::Wait ( int64_t iUS )
{
	m_pImpl->Wait ( iUS );
}

int NetPooller_c::GetNumOfReady () const
{
	return m_pImpl->GetNumOfReady();
}

void NetPooller_c::ProcessAll ( std::function<void ( NetPollEvent_t * )>&& fnAction )
{
	m_pImpl->ProcessAll ( std::move ( fnAction ) );
}

void NetPooller_c::RemoveTimeout ( NetPollEvent_t * pEvent )
{
	m_pImpl->RemoveTimeout ( pEvent );
}

void NetPooller_c::RemoveEvent ( NetPollEvent_t * pEvent )
{
	m_pImpl->RemoveEvent ( pEvent );
}

int64_t NetPooller_c::TickGranularity() const
{
	return m_pImpl->poll_granularity;
}


ThreadRole NetPoollingThread;
