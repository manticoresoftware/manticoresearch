//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "net_action_accept.h"
#include "netstate_api.h"
#include "netreceive_api.h"
#include "netreceive_ql.h"
#include "netreceive_http.h"
#include "coroutine.h"

#if USE_WINDOWS
// Win-specific headers and calls
#include <io.h>
#else
// UNIX-specific headers and calls
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#endif
int g_iThrottleAccept = 0;
extern volatile bool g_bMaintenance;
static auto & g_iTFO = sphGetTFO ();

void FillNetState ( NetConnection_t * pState, int iClientSock, int iConnID, bool bVIP, bool bSSL,
		const sockaddr_storage & saStorage )
{
	pState->m_iClientSock = iClientSock;
	pState->m_iConnID = iConnID;
	pState->m_bVIP = bVIP;
	pState->m_bSSL = bSSL;
	pState->m_tSockType = saStorage.ss_family;

	// format client address
	pState->m_sClientName[0] = '\0';
	if ( saStorage.ss_family==AF_INET )
	{
		struct sockaddr_in * pSa = ((struct sockaddr_in *)&saStorage);
		sphFormatIP ( pState->m_sClientName, SPH_ADDRESS_SIZE, pSa->sin_addr.s_addr );

		char * d = pState->m_sClientName;
		while ( *d )
			d++;
		snprintf ( d, 7, ":%d", (int)ntohs ( pSa->sin_port ) ); //NOLINT
	} else if ( saStorage.ss_family==AF_UNIX )
	{
		strncpy ( pState->m_sClientName, "(local)", SPH_ADDRESS_SIZE );
	}
}

void MultiServe ( SockWrapperPtr_c pSock, NetConnection_t * pConn )
{
	auto pBuf = MakeAsyncNetBuffer ( std::move ( pSock ) );
	auto eProto = pBuf->In ().Probe ( g_iMaxPacketSize, false );
	switch ( eProto )
	{
	case Proto_e::SPHINX:
#ifdef    TCP_NODELAY
	// case of legacy 'crasy squirell' client, which talks using short packages.
		if ( pBuf->In().HasBytes()==4 && pConn->m_tSockType==AF_INET )
		{
			int iOn = 1;
			if ( setsockopt ( pConn->m_iClientSock, IPPROTO_TCP, TCP_NODELAY, (char *) &iOn, sizeof ( iOn ) ) )
				sphWarning ( "setsockopt() from MultiServe failed: %s", sphSockError ());
		}
#endif
		ApiServe ( std::move ( pBuf ), pConn );
		break;
	case Proto_e::HTTPS:
		pConn->m_bSSL = true;
	case Proto_e::HTTP:
		HttpServe ( std::move ( pBuf ), pConn );
		break;
	default:
		sphLogDebugv ( "Unkown proto" );
		break;
	}
}

class NetActionAccept_c::Impl_c
{
	Listener_t		m_tListener;
	int				m_iConnections = 0;

public:
	explicit Impl_c ( const Listener_t & tListener )
		: m_tListener ( tListener )
	{}

	NetEvent_e	ProcessAccept ( DWORD uGotEvents, CSphNetLoop * pLoop );
	int			GetStatsAccept ();
};

DWORD NextConnectionID()
{
	static std::atomic<DWORD> g_iConnectionID { 0 };        ///< global conn-id
	return ++g_iConnectionID;
}

NetEvent_e NetActionAccept_c::Impl_c::ProcessAccept ( DWORD uGotEvents, CSphNetLoop * pLoop )
{
	NetEvent_e eRes = NE_ACCEPT;
	if ( CheckSocketError ( uGotEvents ) )
		return eRes;

	// handle all incoming requests at once but not too much
	int iLastConn = m_iConnections;
	sockaddr_storage saStorage = {0};
	socklen_t uLength = sizeof(saStorage);

	while (true)
	{
		if ( g_iThrottleAccept && g_iThrottleAccept<m_iConnections-iLastConn )
		{
			sphLogDebugv ( "%p accepted %d connections throttled", this, m_iConnections-iLastConn );
			return eRes;
		}

		// accept
		int iClientSock = accept ( m_tListener.m_iSock, (struct sockaddr *)&saStorage, &uLength );

		// handle failures and no more incoming clients
		if ( iClientSock<0 )
		{
			const int iErrno = sphSockGetErrno();
			if ( iErrno==EINTR || iErrno==ECONNABORTED || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
			{
				if ( m_iConnections!=iLastConn )
					sphLogDebugv ( "%p accepted %d connections all, tick=%u",
							this, m_iConnections-iLastConn, pLoop->m_uTick );

				return eRes;
			}

			if ( iErrno==EMFILE || iErrno==ENFILE )
			{
				sphWarning ( "accept() failed, raise ulimit -n and restart searchd: %s", sphSockError(iErrno) );
				return eRes;
			}

			sphFatal ( "accept() failed: %s", sphSockError(iErrno) );
		}

		if ( sphSetSockNB ( iClientSock )<0 )
		{
			sphWarning ( "sphSetSockNB() failed: %s", sphSockError() );
			sphSockClose ( iClientSock );
			return eRes;
		}

		if ( g_bMaintenance && !m_tListener.m_bVIP )
		{
			sphWarning ( "server is in maintenance mode: refusing connection" );
			sphSockClose ( iClientSock );
			return eRes;
		}

		++m_iConnections;
		int iConnID = NextConnectionID();

		/*
 * Modes of execution:
 * - usual: default scheduler + non-zero netloop. Polling performed by netloop; working by thread pool.
 * - vip: alone scheduler and zero netloop. All work (polling and calculations) performed by dedicated alone thread.
 */
		auto * pClientNetLoop = pLoop;
		SchedulerFabric_fn fnMakeScheduler = nullptr;
		if ( m_tListener.m_bVIP )
		{
			pClientNetLoop = nullptr;
			// fixme! for now pass -1, which means 'no limit N of workers'. M.b. need to obey max_children here.
			fnMakeScheduler = [] { sphLogDebugv ( "-~-~-~-~-~-~-~-~ Alone sched created -~-~-~-~-~-~-~-~" ); return GetAloneScheduler ( -1 ); };
		} else
		{
			fnMakeScheduler = [] { sphLogDebugv ( "-~-~-~-~-~-~-~-~ MT sched created -~-~-~-~-~-~-~-~" ); return GlobalWorkPool (); };
		}

		NetConnection_t tConn;
		FillNetState ( &tConn, iClientSock, iConnID, m_tListener.m_bVIP, false, saStorage );
		SockWrapperPtr_c pSock ( new SockWrapper_c ( iClientSock, pClientNetLoop ) );

		switch ( m_tListener.m_eProto )
		{
			case Proto_e::HTTPS:
			case Proto_e::SPHINX:
			case Proto_e::HTTP :
			{
				Threads::CoGo ( [pSock, tConn] () mutable
					{ MultiServe ( std::move ( pSock ), &tConn ); }, fnMakeScheduler () );
				break;
			}
			case Proto_e::MYSQL41:
			{
				Threads::CoGo ( [pSock, tConn] () mutable
					{ SqlServe ( std::move ( pSock ), &tConn ); }, fnMakeScheduler() );
				break;
			}
			case Proto_e::REPLICATION:
				assert (false && "replication must be processed on another level");

			default:
				break;
		}
		sphLogDebugv ( "%p accepted %s, sock=%d, tick=%u", this,
				ProtoName(m_tListener.m_eProto), iClientSock, pLoop->m_uTick );
	}
}


int NetActionAccept_c::Impl_c::GetStatsAccept ()
{
	auto iResult = m_iConnections;
	m_iConnections = 0;
	return iResult;
}

NetActionAccept_c::NetActionAccept_c ( const Listener_t & tListener )
	: ISphNetAction ( tListener.m_iSock )
	, m_pImpl ( new Impl_c ( tListener ))
{
	m_uNetEvents = NetPollEvent_t::READ;
}

NetActionAccept_c::~NetActionAccept_c ()
{
	SafeDelete ( m_pImpl );
}

NetEvent_e NetActionAccept_c::Process ( DWORD uGotEvents, CSphNetLoop * pLoop )
{
	assert ( m_pImpl );
	return m_pImpl->ProcessAccept ( uGotEvents, pLoop );
}

int NetActionAccept_c::GetStats ()
{
	assert ( m_pImpl );
	return m_pImpl->GetStatsAccept ();
}

