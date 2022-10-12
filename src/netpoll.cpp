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

static void SafeCloseSocket ( int& iFD )
{
	if ( iFD >= 0 )
		sphSockClose ( iFD );
	iFD = -1;
}

// both epoll and kqueue have events in hash, so we use also linked list to track them explicitly.
// both stores pointer to the list item in pPtr of backend

using NetPollEventsList_t = boost::intrusive::slist<NetPollEvent_t,
	boost::intrusive::member_hook<NetPollEvent_t, netlist_hook_t, &NetPollEvent_t::m_tBackHook>,
	boost::intrusive::constant_time_size<true>,
	boost::intrusive::cache_last<true>>;

// specific different function for kqueue/epoll
// keep them elementary and tiny for easy debugging across platforms
namespace { // unnamed static
#if ( NETPOLL_TYPE==NETPOLL_KQUEUE )
	using pollev = struct kevent;
	inline int create_poller ( int ) { return kqueue (); }
	inline void * get_data ( const pollev & tEv ) { return tEv.udata; }
	inline DWORD translate_events ( const pollev & tEv)
	{
		return (DWORD) ( ( tEv.flags & EV_ERROR ) ? NetPollEvent_t::ERR : 0 )
				| ( ( tEv.flags & EV_EOF ) ? NetPollEvent_t::HUP : 0 )
				| ( ( tEv.filter==EVFILT_READ ) ? NetPollEvent_t::READ : NetPollEvent_t::WRITE );
	}
#elif ( NETPOLL_TYPE==NETPOLL_EPOLL )
	using pollev = epoll_event;
	inline int create_poller ( int iSizeHint ) { return epoll_create ( iSizeHint ); }
	inline void* get_data ( const pollev& tEv ) { return tEv.data.ptr; }
	inline DWORD translate_events ( const pollev & tEv)
	{
		return (DWORD) ( ( tEv.events & EPOLLERR ) ? NetPollEvent_t::ERR : 0 )
				| ( ( tEv.events & EPOLLHUP ) ? NetPollEvent_t::HUP : 0 )
				| ( ( tEv.events==EPOLLIN ) ? NetPollEvent_t::READ : 0 )
				| ( ( tEv.events==EPOLLOUT ) ? NetPollEvent_t::WRITE : 0 );
}
#endif
}

// need for remove from intrusive list to work
inline bool operator== ( const NetPollEvent_t& lhs, const NetPollEvent_t& rhs )
{
		return &lhs == &rhs;
}

// common for both epoll and kqueue
class NetPooller_c::Impl_c final
{
	friend class NetPooller_c;
	friend class NetPollReadyIterator_c;

	TimeoutEvents_c				m_dTimeouts			GUARDED_BY ( NetPoollingThread );
	CSphVector<pollev>			m_dFiredEvents		GUARDED_BY ( NetPoollingThread );
	NetPollEventsList_t			m_tEvents			GUARDED_BY ( NetPoollingThread ); // used for 'for_all'

	int							m_iReady = 0;
	int							m_iLastReportedErrno = -1;
	int							m_iPl;

public:

	explicit Impl_c ( int iSizeHint )
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
		SafeCloseSocket ( m_iPl );
	}

	void SetupEvent ( NetPollEvent_t * pEvent ) REQUIRES ( NetPoollingThread )
	{
		assert ( pEvent && pEvent->m_iSock>=0 );
		auto uEvents = pEvent->m_uNetEvents; // shortcut
		assert ( ( uEvents & NetPollEvent_t::WRITE )!=0 || ( uEvents & NetPollEvent_t::READ )!=0 );

		m_dTimeouts.AddOrChangeTimeout ( pEvent );

		bool bIsNew = !pEvent->m_tBackHook.is_linked();
		if ( bIsNew )
		{
			SafeAddRef ( pEvent );
			m_tEvents.push_back ( *pEvent );
		}


		int iRes = set_polling_for ( pEvent->m_iSock, pEvent, uEvents & NetPollEvent_t::READ, uEvents & NetPollEvent_t::ONCE, bIsNew );

		if ( iRes==-1 )
			sphWarning ( "failed to setup queue event for sock %d, errno=%d, %s", pEvent->m_iSock, errno, strerrorm ( errno ) );
	}

	void Wait ( int64_t iUS ) REQUIRES ( NetPoollingThread )
	{
		if ( m_tEvents.empty() )
			return;

		if ( iUS==WAIT_UNTIL_TIMEOUT )
			iUS = m_dTimeouts.GetNextTimeoutUS ( poll_granularity );

		m_dFiredEvents.Resize ( m_tEvents.size() );

		// need positive timeout for communicate threads back and shutdown
		m_iReady = poll_events ( m_dFiredEvents.Begin (), m_dFiredEvents.GetLength (), iUS );

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

	// called when client detected error or timeout
	void RemoveEvent ( NetPollEvent_t * pEvent ) REQUIRES ( NetPoollingThread )
	{
		assert ( pEvent );
		RemoveTimeout ( pEvent );

		sphLogDebugv ( "%p polling remove, ev=%u, sock=%d", pEvent, pEvent->m_uNetEvents, pEvent->m_iSock );

		if ( pEvent->m_uNetEvents!=NetPollEvent_t::CLOSED )
		{
			int iRes = remove_polling_for ( pEvent->m_iSock, pEvent->m_uNetEvents & NetPollEvent_t::READ );

			// might be already closed by worker from thread pool
			if ( iRes==-1 )
				sphLogDebugv ( "failed to remove polling event for sock %d(%p), errno=%d, %s", pEvent->m_iSock, pEvent, errno, strerrorm (errno));
		}

		// since event already removed from kqueue - it is safe to remove it from the list of events also,
		// and totally unlink
		if ( pEvent->m_tBackHook.is_linked() )
		{
			m_tEvents.remove ( *pEvent );
			SafeRelease ( pEvent );
		}
	}

	// platform-specific members. Tiny and simple to reduce divergention among platforms
#if ( NETPOLL_TYPE==NETPOLL_EPOLL )

	constexpr static int64_t poll_granularity = 1000LL;

	inline static int US2Polltime ( int64_t timeoutUS )
	{
		switch ( timeoutUS )
		{
		case TimeoutEvents_c::TIME_INFINITE: return -1;
		case TimeoutEvents_c::TIME_IMMEDIATE: return 0;
		default: return timeoutUS / poll_granularity;
		}
	}

	inline int poll_events ( pollev* pEvents, int iEventNum, int64_t timeoutUS ) const
	{
		return epoll_wait ( m_iPl, pEvents, iEventNum, US2Polltime ( timeoutUS ) );
	};

	inline int set_polling_for ( int iSock, void* pData, bool bRead, bool bOneshot, bool bAdd ) const
	{
		int iOp = bAdd ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
		epoll_event tEv;
		tEv.data.ptr = pData;
		tEv.events = bRead ? EPOLLIN : EPOLLOUT;
		if ( bOneshot )
			tEv.events |= EPOLLONESHOT | EPOLLET;

		sphLogDebugv ("%p epoll %d setup, ev=0x%u, op=%d, sock=%d", pData, m_iPl, tEv.events, iOp, iSock) ;
		return epoll_ctl ( m_iPl, iOp, iSock, &tEv );
	}

	inline int remove_polling_for ( int iSock, bool ) const
	{
		epoll_event tEv;
		return epoll_ctl ( m_iPl, EPOLL_CTL_DEL, iSock, &tEv );
	}

#elif ( NETPOLL_TYPE==NETPOLL_KQUEUE )

	constexpr static int64_t poll_granularity = 1LL;

	inline static timespec* US2keventtime ( timespec& ts, int64_t timeoutUS )
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

	inline int poll_events ( pollev* pEvents, int iEventNum, int64_t timeoutUS ) const
	{
		timespec ts;
		return kevent ( m_iPl, nullptr, 0, pEvents, iEventNum, US2keventtime ( ts, timeoutUS ) );
	};

	inline int set_polling_for ( int iSock, void* pData, bool bRead, bool bOneshot, bool ) const
	{
		struct kevent tEv;
		EV_SET ( &tEv, iSock, bRead?EVFILT_READ:EVFILT_WRITE, bOneshot ? EV_ADD | EV_ONESHOT | EV_CLEAR : EV_ADD, 0, 0, pData );
		sphLogDebugv ( "%p kqueue %d setup, ev=%d, fl=%d sock=%d", pData, m_iPl, tEv.filter, tEv.flags, iSock );
		return kevent ( m_iPl, &tEv, 1, nullptr, 0, nullptr );
	}

	inline int remove_polling_for ( int iSock, bool bRead ) const
	{
		struct kevent tEv;
		EV_SET( &tEv, iSock, ( bRead ? EVFILT_READ : EVFILT_WRITE ), EV_DELETE | EV_CLEAR, 0, 0, nullptr );
		return kevent ( m_iPl, &tEv, 1, nullptr, 0, nullptr );
	}

#endif // NETPOLL_KQUEUE
};

// more common for NETPOLL_TYPE==NETPOLL_KQUEUE || NETPOLL_TYPE==NETPOLL_EPOLL


NetPollEvent_t & NetPollReadyIterator_c::operator* ()
{
	auto & pOwner = m_pOwner->m_pImpl;
	const pollev & tEv = pOwner->m_dFiredEvents[m_iIterEv];
	auto* pNode = (NetPollEvent_t*)get_data ( tEv );
	assert ( pNode && "deleted event recognized");
	pNode->m_uNetEvents = translate_events(tEv);
	return *pNode;
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

class NetPooller_c::Impl_c final
{
	friend class NetPooller_c;
	friend class NetPollReadyIterator_c;

	TimeoutEvents_c					m_dTimeouts			GUARDED_BY ( NetPoollingThread );
	CSphVector<NetPollEvent_t *>	m_dWork				GUARDED_BY ( NetPoollingThread );
	CSphVector<pollfd>				m_dEvents			GUARDED_BY ( NetPoollingThread );

	int								m_iReady = 0;
	int								m_iLastReportedErrno = -1;

public:
	explicit Impl_c ( int iSizeHint )
	{
		m_dWork.Reserve ( iSizeHint );
		m_dEvents.Reserve ( iSizeHint );
	}

	void SetupEvent ( NetPollEvent_t * pEvent ) REQUIRES ( NetPoollingThread )
	{
		assert ( m_dEvents.GetLength ()==m_dWork.GetLength () );
		assert ( pEvent && pEvent->m_iSock>=0 );
		auto uEvents = pEvent->m_uNetEvents; // shortcut
		assert ( ( uEvents & NetPollEvent_t::WRITE )!=0 || ( uEvents & NetPollEvent_t::READ )!=0 );

		m_dTimeouts.AddOrChangeTimeout ( pEvent );

		auto uNetEvents = ( ( uEvents & NetPollEvent_t::READ )!=0 ) ? POLLIN : POLLOUT;
		pollfd* pEv = nullptr;
		auto& iBackIdx = pEvent->m_iBackIdx;
		if ( iBackIdx>=0 && iBackIdx<m_dWork.GetLength () && !m_dWork[iBackIdx] ) // was already enqueued, just change events
		{
			m_dWork[iBackIdx] = pEvent;
			pEv = &m_dEvents[iBackIdx];
			sphLogDebugvv ( "SetupEvent [%d] old %d events %d", pEvent->m_iBackIdx, pEvent->m_iSock, uNetEvents );
		} else
		{
			iBackIdx = m_dWork.GetLength ();
			pEvent->AddRef();
			m_dWork.Add ( pEvent );
			pEv = &m_dEvents.Add ();
			sphLogDebugvv ( "SetupEvent [%d] new %d events %d", pEvent->m_iBackIdx, pEvent->m_iSock, uNetEvents );
		}

		pEv->fd = pEvent->m_iSock;
		pEv->events = ( ( uEvents & NetPollEvent_t::READ )!=0 ) ? POLLIN : POLLOUT;
		sphLogDebugvv ( "SetupEvent [%d] for %d events %d", pEvent->m_iBackIdx, pEvent->m_iSock, pEv->events );
	}

	constexpr static int64_t poll_granularity = 1000LL;

	inline static int US2Polltime ( int64_t timeoutUS )
	{
		switch ( timeoutUS )
		{
		case TimeoutEvents_c::TIME_INFINITE: return -1;
		case TimeoutEvents_c::TIME_IMMEDIATE: return 0;
		default: return timeoutUS / poll_granularity;
		}
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
		m_iReady = ::poll ( m_dEvents.Begin (), m_dEvents.GetLength (), US2Polltime (iUS) );

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

	// called when client detected error or timeout
	void RemoveEvent ( NetPollEvent_t * pEvent ) REQUIRES ( NetPoollingThread )
	{
		assert ( pEvent );
		RemoveTimeout ( pEvent );

		sphLogDebugvv ( "RemoveEvent for %d, fd=%d", pEvent->m_iBackIdx, pEvent->m_iSock );

		if ( pEvent->m_iBackIdx<0 ) // already removed by iteration
			return;

		assert ( pEvent->m_iBackIdx<m_dEvents.GetLength ());

		m_dEvents[pEvent->m_iBackIdx].fd = -1;
		m_dWork[pEvent->m_iBackIdx] = nullptr;
		pEvent->m_iBackIdx = -1;
		pEvent->Release();
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
	NetPollEvent_t * pNode = pOwner->m_dWork[m_iIterEv];
	pollfd & tEv = pOwner->m_dEvents[m_iIterEv];

	sphLogDebugvv ( "[%d] tEv.revents = %d for %d(%d)", m_iIterEv, tEv.revents, pNode->m_iSock, tEv.fd );

	if ( pNode->m_uNetEvents & NetPollEvent_t::ONCE )
	{
		tEv.fd = -tEv.fd;
		pOwner->m_dWork[m_iIterEv] = nullptr;
		pNode->m_iBackIdx = -1;
		pNode->m_uNetEvents &= ~( NetPollEvent_t::ONCE );
		pNode->Release();
	}

	if ( tEv.revents & POLLIN )
		pNode->m_uNetEvents |= NetPollEvent_t::READ;
	if ( tEv.revents & POLLOUT )
		pNode->m_uNetEvents |= NetPollEvent_t::WRITE;
	if ( tEv.revents & POLLHUP )
		pNode->m_uNetEvents |= NetPollEvent_t::HUP;
	if ( tEv.revents & POLLERR )
		pNode->m_uNetEvents |= NetPollEvent_t::ERR;

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


NetPooller_c::NetPooller_c ( int isizeHint )
	: m_pImpl ( std::make_unique<Impl_c> ( isizeHint ) )
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
