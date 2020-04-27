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
#include "netreceive_https.h"

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

#define NET_STATE_BUF_SIZE 65535

void FillNetState ( NetStateAPI_t * pState, int iClientSock, int iConnID, bool bVIP,
		const sockaddr_storage & saStorage )
{
	pState->m_iClientSock = iClientSock;
	pState->m_iConnID = iConnID;
	pState->m_bVIP = bVIP;

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

class NetActionAccept_c::Impl_c
{
	friend class NetActionAccept_c;
	Listener_t		m_tListener;
	int				m_iConnections;
	NetStateAPI_t	m_tDummy;

	explicit Impl_c ( const Listener_t & tListener )
		: m_tListener ( tListener )
		, m_iConnections (0)
	{}

	NetEvent_e		LoopAccept ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dQueue, CSphNetLoop * pLoop );
	bool			GetStatsAccept ( int & iConnections );
};


NetEvent_e NetActionAccept_c::Impl_c::LoopAccept ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dQueue,
		CSphNetLoop * pLoop )
{
	if ( CheckSocketError ( uGotEvents, "accept err WTF???", &m_tDummy, true ) )
		return NE_KEEP;

	// handle all incoming requests at once but not too much
	int iLastConn = m_iConnections;
	sockaddr_storage saStorage = {0};
	socklen_t uLength = sizeof(saStorage);

	while (true)
	{
		if ( g_iThrottleAccept && g_iThrottleAccept<m_iConnections-iLastConn )
		{
			sphLogDebugv ( "%p accepted %d connections throttled", this, m_iConnections-iLastConn );
			return NE_KEEP;
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

				return NE_KEEP;
			}

			if ( iErrno==EMFILE || iErrno==ENFILE )
			{
				sphWarning ( "accept() failed, raise ulimit -n and restart searchd: %s", sphSockError(iErrno) );
				return NE_KEEP;
			}

			sphFatal ( "accept() failed: %s", sphSockError(iErrno) );
		}

		if ( sphSetSockNB ( iClientSock )<0 )
		{
			sphWarning ( "sphSetSockNB() failed: %s", sphSockError() );
			sphSockClose ( iClientSock );
			return NE_KEEP;
		}

		if ( g_bMaintenance && !m_tListener.m_bVIP )
		{
			sphWarning ( "server is in maintenance mode: refusing connection" );
			sphSockClose ( iClientSock );
			return NE_KEEP;
		}

		++m_iConnections;
		int iConnID = ++g_iConnectionID;
		if ( g_iConnectionID<0 )
		{
			g_iConnectionID = 0;
			iConnID = 0;
		}

		ISphNetAction * pAction = nullptr;
		switch ( m_tListener.m_eProto )
		{
			case Proto_e::SPHINX:
			{
				auto * pStateAPI = new NetStateAPI_t ();
				pStateAPI->m_dBuf.Reserve ( NET_STATE_BUF_SIZE );
				FillNetState ( pStateAPI, iClientSock, iConnID, m_tListener.m_bVIP, saStorage );
				pAction = new NetReceiveDataAPI_c ( pStateAPI );
				break;
			}
			case Proto_e::HTTP :
			{
				auto * pStateHttp = new NetStateAPI_t ();
				pStateHttp->m_dBuf.Reserve ( NET_STATE_BUF_SIZE );
				FillNetState ( pStateHttp, iClientSock, iConnID, false, saStorage );
				pAction = new NetReceiveDataHttp_c ( pStateHttp );
				break;
			}
			case Proto_e::HTTPS:
			{
				auto * pStateHttp = new NetStateHttps_t ();
				pStateHttp->m_dBuf.Reserve ( NET_STATE_BUF_SIZE );
				FillNetState ( pStateHttp, iClientSock, iConnID, false, saStorage );
				pAction = new NetReceiveDataHttps_c ( pStateHttp );
				break;
			}
			case Proto_e::MYSQL41:
			default:
			{
				auto * pStateQL = new NetStateQL_t ();
				pStateQL->m_dBuf.Reserve ( NET_STATE_BUF_SIZE );
				FillNetState ( pStateQL, iClientSock, iConnID, m_tListener.m_bVIP, saStorage );
				auto * pActionQL = new NetReceiveDataQL_c ( pStateQL );
				pActionQL->SetupHandshakePhase();
				pAction = pActionQL;
				break;
			}
		}
		dQueue.Add ( pAction );
		sphLogDebugv ( "%p accepted %s, sock=%d, tick=%u", this, ProtoName(m_tListener.m_eProto),
				pAction->m_iSock, pLoop->m_uTick );
	}
}


bool NetActionAccept_c::Impl_c::GetStatsAccept ( int & iConnections )
{
	if ( !m_iConnections )
		return false;

	iConnections += m_iConnections;
	m_iConnections = 0;
	return true;
}

NetActionAccept_c::NetActionAccept_c ( const Listener_t & tListener )
	: ISphNetAction ( tListener.m_iSock )
	, m_pImpl ( new Impl_c ( tListener ))
{}

NetActionAccept_c::~NetActionAccept_c ()
{
	SafeDelete ( m_pImpl );
}

NetEvent_e NetActionAccept_c::Loop ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dQueue, CSphNetLoop * pLoop )
{
	assert ( m_pImpl );
	return m_pImpl->LoopAccept ( uGotEvents, dQueue, pLoop );
}

bool NetActionAccept_c::GetStats ( int & iConnections )
{
	assert ( m_pImpl );
	return m_pImpl->GetStatsAccept ( iConnections );
}

