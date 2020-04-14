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

#include "networking_daemon.h"
#include "netstate_api.h"
#include "netreceive_api.h"
#include "netreceive_http.h"
#include "netreceive_https.h"
#include "netreceive_ql.h"

NetStateAPI_t::NetStateAPI_t ()
{
	m_sClientName[0] = '\0';
}

NetStateAPI_t::~NetStateAPI_t()
{
	CloseSocket();
}

void NetStateAPI_t::CloseSocket ()
{
	if ( m_iClientSock>=0 )
	{
		sphLogDebugv ( "%p state closing sock=%d", this, m_iClientSock );
		sphSockClose ( m_iClientSock );
		m_iClientSock = -1;
	}
}

int64_t NetStateAPI_t::SocketIO ( bool bWrite, bool bAfterWrite )
{
	// try next chunk
	int64_t iRes = 0;
	if ( bWrite )
		iRes = (int64_t)sphSockSend ( m_iClientSock, (const char*)(m_dBuf.begin() + m_iPos), m_iLeft );
	else
		iRes = (int64_t)sphSockRecv ( m_iClientSock, (char*)(m_dBuf.begin () + m_iPos), m_iLeft );

	// if there was EINTR, retry
	// if any other error, bail
	if ( iRes==-1 )
	{
		// only let SIGTERM (of all them) to interrupt
		int iErr = sphSockPeekErrno ();
		return ( ( iErr==EINTR || iErr==EAGAIN || iErr==EWOULDBLOCK ) ? 0 : -1 );
	}

	// if there was eof, we're done
	// but need to make sure that poll loop passed at least once,
	// ie write-read pattern should failed only this way write-poll-read
	if ( !bWrite && iRes==0 && m_iLeft!=0 && !bAfterWrite ) // request to read 0 raise error on Linux, but not on Mac
	{
		sphLogDebugv ( "read zero bytes, shutting down socket, sock=%d", m_iClientSock );
		sphSockSetErrno ( ESHUTDOWN );
		return -1;
	}

	return iRes;
}


NetSendData_t::NetSendData_t ( NetStateAPI_t * pState, Proto_e eProto )
	: ISphNetAction ( pState->m_iClientSock )
	, m_tState ( pState )
	, m_eProto ( eProto )
	, m_bContinue ( false )
{
	assert ( pState );
	m_tState->m_iPos = 0;
	m_tState->m_iLeft = 0;
}

NetEvent_e NetSendData_t::Loop ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop )
{
	if ( CheckSocketError ( uGotEvents, "failed to send data", m_tState.Ptr(), false ) )
		return NE_REMOVE;

	for ( ; m_tState->m_iLeft>0; )
	{
		int64_t iRes = m_tState->SocketIO ( true );
		if ( iRes==-1 )
		{
			LogSocketError ( "failed to send data", m_tState.Ptr(), false );
			return NE_REMOVE;
		}
		// for iRes==0 case might proceed after interruption
		m_tState->m_iLeft -= iRes;
		m_tState->m_iPos += iRes;

		// socket would block - going back to polling
		if ( iRes==0 )
			break;
	}

	if ( m_tState->m_iLeft>0 )
		return NE_KEEP;

	assert ( m_tState->m_iLeft==0 && m_tState->m_iPos==m_tState->m_dBuf.GetLength() );

	if ( m_tState->m_bKeepSocket )
	{
		sphLogDebugv ( "%p send %s job created, sent=" INT64_FMT ", sock=%d, tick=%u", this, ProtoName ( m_eProto ), m_tState->m_iPos, m_iSock, pLoop->m_uTick );
		switch ( m_eProto )
		{
			case Proto_e::SPHINX:
			{
				auto * pAction = new NetReceiveDataAPI_c ( m_tState.LeakPtr() );
				pAction->SetupBodyPhase();
				dNextTick.Add ( pAction );
			}
			break;

			case Proto_e::MYSQL41:
			{
				auto * pAction = new NetReceiveDataQL_c ( (NetStateQL_t *)m_tState.LeakPtr() );
				pAction->SetupBodyPhase();
				dNextTick.Add ( pAction );
			}
			break;

			case Proto_e::HTTP:
			{
				auto * pAction = new NetReceiveDataHttp_c ( (NetStateAPI_t *)m_tState.LeakPtr() );
				dNextTick.Add ( pAction );
			}
			break;

			case Proto_e::HTTPS:
			{
				auto * pAction = new NetReceiveDataHttps_c ( (NetStateHttps_t *)m_tState.LeakPtr() );
				dNextTick.Add ( pAction );
			}
			break;

			default: break;
		}
	}

	return NE_REMOVE;
}

NetEvent_e NetSendData_t::Setup ( int64_t tmNow )
{
	assert ( m_tState.Ptr() );
	sphLogDebugv ( "%p send %s setup, keep=%d, buf=" INT64_FMT ", client=%s, conn=%d, sock=%d", this,
			ProtoName ( m_eProto ), (int)(m_tState->m_bKeepSocket), m_tState->m_dBuf.GetLength64(),
			m_tState->m_sClientName, m_tState->m_iConnID, m_tState->m_iClientSock );

	if ( !m_bContinue )
	{
		m_iTimeoutTime = tmNow + MS2SEC * g_iWriteTimeout;

		assert ( m_tState->m_dBuf.GetLength64() );
		m_tState->m_iLeft = m_tState->m_dBuf.GetLength64();
		m_tState->m_iPos = 0;
	}
	m_bContinue = false;

	return NE_OUT;
}


void NetSendData_t::CloseSocket()
{
	if ( m_tState.Ptr() )
		m_tState->CloseSocket();
}

bool CheckSocketError ( DWORD uGotEvents, const char * sMsg, const NetStateAPI_t * pConn, bool bDebug )
{
	bool bReadError = ( ( uGotEvents & NE_IN ) && ( uGotEvents & ( NE_ERR | NE_HUP ) ) );
	bool bWriteError = ( ( uGotEvents & NE_OUT ) && ( uGotEvents & NE_ERR ) );

	if ( bReadError || bWriteError )
	{
		LogSocketError ( sMsg, pConn, bDebug );
		return true;
	}
	return false;
}

void LogSocketError ( const char * sMsg, const NetStateAPI_t * pConn, bool bDebug )
{
	if ( bDebug && g_eLogLevel<SPH_LOG_VERBOSE_DEBUG )
		return;

	assert ( pConn );
	int iErrno = sphSockGetErrno();
	if ( iErrno==0 && pConn->m_iClientSock>=0 )
	{
		socklen_t iLen = sizeof(iErrno);
		int iRes = getsockopt ( pConn->m_iClientSock, SOL_SOCKET, SO_ERROR, (char*)&iErrno, &iLen );
		if ( iRes<0 )
			sphWarning ( "%s (client=%s(%d)), failed to get error: %d '%s'",
					sMsg, pConn->m_sClientName, pConn->m_iConnID, errno, strerrorm ( errno ) );
	}

	if ( bDebug || iErrno==ESHUTDOWN )
		sphLogDebugv ( "%s (client=%s(%d)), error: %d '%s', sock=%d",
				sMsg, pConn->m_sClientName, pConn->m_iConnID, iErrno, sphSockError ( iErrno ), pConn->m_iClientSock );
	else
		sphWarning ( "%s (client=%s(%d)), error: %d '%s', sock=%d",
				sMsg, pConn->m_sClientName, pConn->m_iConnID, iErrno, sphSockError ( iErrno ), pConn->m_iClientSock );
}

void JobDoSendNB ( NetSendData_t * pSend, CSphNetLoop * pLoop )
{
	assert ( pLoop && pSend );
	NetEvent_e eRes = pSend->Setup ( sphMicroTimer() );

	// try to push data to socket here then transfer send-action to net-loop in case send needs poll to continue
	CSphVector<ISphNetAction *> dNext ( 1 );
	dNext.Resize ( 0 );
	DWORD uGotEvents = NE_OUT;
	eRes = pSend->Loop ( uGotEvents, dNext, pLoop );
	if ( eRes==NE_REMOVE )
	{
		SafeDelete ( pSend );
		assert ( dNext.GetLength()<2 );
		if ( dNext.GetLength() )
			pLoop->AddAction ( dNext[0] );
	} else
	{
		pSend->SetContinue();
		pLoop->AddAction ( pSend );
	}
}