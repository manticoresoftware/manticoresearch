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

#include "netreceive_api.h"

extern int g_iClientTimeout; // from searchd.cpp
extern volatile bool g_bMaintenance;
extern int g_iThdQueueMax;
static auto& g_bShutdown = sphGetShutdown ();

struct ThdJobAPI_t : public ISphJob
{
	CSphScopedPtr<NetStateAPI_t>		m_tState;
	CSphNetLoop *		m_pLoop;
	SearchdCommand_e	m_eCommand = SEARCHD_COMMAND_WRONG;
	WORD				m_uCommandVer = VER_COMMAND_WRONG;

	ThdJobAPI_t ( CSphNetLoop * pLoop, NetStateAPI_t * pState );

	void		Call () final;
};

ThdJobAPI_t::ThdJobAPI_t ( CSphNetLoop * pLoop, NetStateAPI_t * pState )
	: m_tState ( pState )
	, m_pLoop ( pLoop )
{
	assert ( m_tState.Ptr() );
}

void ThdJobAPI_t::Call ()
{
	CrashQuery_t tQueryTLS;
	SphCrashLogger_c::SetTopQueryTLS ( &tQueryTLS );

	sphLogDebugv ( "%p API job started, command=%d, tick=%u", this, m_eCommand, m_pLoop->m_uTick );

	int iTid = GetOsThreadId();

	ThdDesc_t tThdDesc;
	tThdDesc.m_eProto = Proto_e::SPHINX;
	tThdDesc.m_iClientSock = m_tState->m_iClientSock;
	tThdDesc.m_sClientName = m_tState->m_sClientName;
	tThdDesc.m_iConnID = m_tState->m_iConnID;
	tThdDesc.m_tmConnect = sphMicroTimer();
	tThdDesc.m_iTid = iTid;

	tThdDesc.AddToGlobal();

	assert ( m_tState.Ptr() );

	// handle that client
	MemInputBuffer_c tBuf ( m_tState->m_dBuf );
	CachedOutputBuffer_c tOut;

	if ( g_bMaintenance && !m_tState->m_bVIP )
	{
		SendErrorReply ( tOut, "server is in maintenance mode" );
		m_tState->m_bKeepSocket = false;
	} else
	{
		bool bProceed = LoopClientSphinx ( m_eCommand, m_uCommandVer,
			m_tState->m_dBuf.GetLength(), tThdDesc, tBuf, tOut, false );
		m_tState->m_bKeepSocket |= bProceed;
	}
	tOut.Flush();

	tThdDesc.RemoveFromGlobal ();

	sphLogDebugv ( "%p API job done, command=%d, tick=%u", this, m_eCommand, m_pLoop->m_uTick );

	if ( g_bShutdown )
		return;

	assert ( m_pLoop );
	if ( tOut.GetSentCount() )
	{
		tOut.SwapData ( m_tState->m_dBuf );
		JobDoSendNB ( new NetSendData_t ( m_tState.LeakPtr (), Proto_e::SPHINX ), m_pLoop );
	} else if ( m_tState->m_bKeepSocket ) // no response - switching to receive
	{
		auto * pReceive = new NetReceiveDataAPI_c ( m_tState.LeakPtr() );
		pReceive->SetupBodyPhase();
		m_pLoop->AddAction ( pReceive );
	}
}

enum ActionAPI_e : BYTE
{
	AAPI_HANDSHAKE_OUT = 0,
	AAPI_HANDSHAKE_IN,
	AAPI_COMMAND,
	AAPI_BODY,
	AAPI_TOTAL
};

const char * g_sErrorNetAPI[] = { "failed to send server version", "exiting on handshake error", "bailing on failed request header", "failed to receive client request body" };

STATIC_ASSERT ( sizeof(g_sErrorNetAPI)/sizeof(g_sErrorNetAPI[0])==AAPI_TOTAL, NOT_ALL_ENUM_DESCRIBERD );

class NetReceiveDataAPI_c::Impl_c
{
	friend class NetReceiveDataAPI_c;
	NetPollEvent_t * m_pParent;

	CSphScopedPtr<NetStateAPI_t> m_tState;
	ActionAPI_e m_ePhase;

	SearchdCommand_e m_eCommand = SEARCHD_COMMAND_WRONG;
	WORD m_uCommandVer = VER_COMMAND_WRONG;

	Impl_c ( NetStateAPI_t * pState, NetPollEvent_t * pParent );
	NetEvent_e LoopReceiveAPI ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop );

	NetEvent_e SetupReceiveAPI ( int64_t tmNow );
	void CloseSocketReceiveAPI ();
	void SetupBodyPhase ();
	void AddJobAPI ( CSphNetLoop * pLoop );
};

NetReceiveDataAPI_c::Impl_c::Impl_c ( NetStateAPI_t * pState, NetPollEvent_t * pParent )
	: m_pParent ( pParent )
	, m_tState ( pState )
{
	m_ePhase = AAPI_HANDSHAKE_OUT;
	m_tState->m_iPos = 0;
	m_eCommand = SEARCHD_COMMAND_WRONG;
	m_uCommandVer = VER_COMMAND_WRONG;

	m_tState->m_dBuf.Resize ( 4 );
	*(DWORD *)( m_tState->m_dBuf.Begin() ) = htonl ( SPHINX_SEARCHD_PROTO );
	m_tState->m_iLeft = 4;
	assert ( m_tState.Ptr() );
}

NetEvent_e NetReceiveDataAPI_c::Impl_c::SetupReceiveAPI ( int64_t tmNow )
{
	assert ( m_tState.Ptr() );
	sphLogDebugv ( "%p receive API setup, phase=%d, keep=%d, client=%s, conn=%d, sock=%d", this, m_ePhase,
			m_tState->m_bKeepSocket, m_tState->m_sClientName, m_tState->m_iConnID, m_tState->m_iClientSock );

	if ( !m_tState->m_bKeepSocket )
	{
		m_pParent->m_iTimeoutTime = tmNow + MS2SEC * g_iReadTimeout;
	} else
	{
		m_pParent->m_iTimeoutTime = tmNow + MS2SEC * g_iClientTimeout;
	}

	return ( m_ePhase==AAPI_HANDSHAKE_OUT ? NE_OUT : NE_IN );
}


void NetReceiveDataAPI_c::Impl_c::SetupBodyPhase()
{
	m_tState->m_dBuf.Resize ( 8 );
	m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
	m_ePhase = AAPI_COMMAND;
}

void NetReceiveDataAPI_c::Impl_c::AddJobAPI ( CSphNetLoop * pLoop )
{
	assert ( m_tState.Ptr() );
	bool bStart = m_tState->m_bVIP;
	int iLen = m_tState->m_dBuf.GetLength();
	auto * pJob = new ThdJobAPI_t ( pLoop, m_tState.LeakPtr() );
	pJob->m_eCommand = m_eCommand;
	pJob->m_uCommandVer = m_uCommandVer;
	sphLogDebugv ( "%p receive API job created (%p), buf=%d, sock=%d, tick=%u",
			this, pJob, iLen, m_pParent->m_iSock, pLoop->m_uTick );
	if ( bStart )
		g_pThdPool->StartJob ( pJob );
	else
		g_pThdPool->AddJob ( pJob );
}

static char g_sMaxedOutMessage[] = "maxed out, dismissing client";

static WORD NetBufGetWord ( const BYTE * pBuf )
{
	WORD uVal = sphUnalignedRead ( (WORD &)*pBuf );
	return ntohs ( uVal );
}

static int NetBufGetInt ( const BYTE * pBuf )
{
	int iVal = sphUnalignedRead ( (int &)*pBuf );
	return ntohl ( iVal );
}

NetEvent_e NetReceiveDataAPI_c::Impl_c::LoopReceiveAPI ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop )
{
	assert ( m_tState.Ptr() );
	bool bDebug = ( m_ePhase==AAPI_HANDSHAKE_IN || m_ePhase==AAPI_COMMAND );
	if ( CheckSocketError ( uGotEvents, g_sErrorNetAPI[m_ePhase], m_tState.Ptr(), bDebug ) )
		return NE_REMOVE;

	bool bWasWrite = ( m_ePhase==AAPI_HANDSHAKE_OUT );
	// loop to handle similar operations at once
	while (true)
	{
		bool bWrite = ( m_ePhase==AAPI_HANDSHAKE_OUT );
		int iRes = m_tState->SocketIO ( bWrite, bWasWrite );
		if ( iRes==-1 )
		{
			LogSocketError ( g_sErrorNetAPI[m_ePhase], m_tState.Ptr(), false );
			return NE_REMOVE;
		}
		m_tState->m_iLeft -= iRes;
		m_tState->m_iPos += iRes;

		// socket would block - going back to polling
		if ( iRes==0 )
		{
			bool bNextWrite = ( m_ePhase==AAPI_HANDSHAKE_OUT );
			if ( bWasWrite!=bNextWrite )
				return ( bNextWrite ? NE_OUT : NE_IN );
			else
				return NE_KEEP;
		}

		// keep using that socket
		if ( m_tState->m_iLeft )
			continue;

		sphLogDebugv ( "%p pre-API phase=%d, buf=%d, write=%d, sock=%d, tick=%u",
				this, m_ePhase, m_tState->m_dBuf.GetLength(), (int)bWrite,
				m_pParent->m_iSock, pLoop->m_uTick );

		// FIXME!!! handle persist socket timeout
		m_tState->m_iPos = 0;
		switch ( m_ePhase )
		{
		case AAPI_HANDSHAKE_OUT:
			m_tState->m_dBuf.Resize ( 4 );
			m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
			m_ePhase = AAPI_HANDSHAKE_IN;
			break;

		case AAPI_HANDSHAKE_IN:
			// magic unused
			SetupBodyPhase();
			break;

		case AAPI_COMMAND:
		{
			m_eCommand = ( SearchdCommand_e ) NetBufGetWord ( m_tState->m_dBuf.Begin () );
			m_uCommandVer = NetBufGetWord ( m_tState->m_dBuf.Begin() + 2 );
			m_tState->m_iLeft = NetBufGetInt ( m_tState->m_dBuf.Begin() + 4 );
			bool bBadCommand = ( m_eCommand>=SEARCHD_COMMAND_WRONG );
			bool bBadLength = ( m_tState->m_iLeft<0 || m_tState->m_iLeft>g_iMaxPacketSize );
			if ( bBadCommand || bBadLength )
			{
				m_tState->m_bKeepSocket = false;

				// unknown command, default response header
				if ( bBadLength )
					sphWarning ( "ill-formed client request (length=%d out of bounds)", m_tState->m_iLeft );
				// if command is insane, low level comm is broken, so we bail out
				if ( bBadCommand )
					sphWarning ( "ill-formed client request (command=%d, SEARCHD_COMMAND_TOTAL=%d)", m_eCommand, SEARCHD_COMMAND_TOTAL );

				m_tState->m_dBuf.Resize ( 0 );
				CachedOutputBuffer_c tOut;
				tOut.SwapData ( m_tState->m_dBuf );
				SendErrorReply ( tOut, "invalid command (code=%d, len=%d)", m_eCommand, m_tState->m_iLeft );

				tOut.SwapData ( m_tState->m_dBuf );
				dNextTick.Add ( new NetSendData_t ( m_tState.LeakPtr (), Proto_e::SPHINX ) );
				return NE_REMOVE;
			}
			m_tState->m_dBuf.Resize ( m_tState->m_iLeft );
			m_ePhase = AAPI_BODY;
			if ( !m_tState->m_iLeft ) // command without body
			{
				pLoop->RemoveIterEvent ( m_pParent );
				AddJobAPI ( pLoop );
				return NE_REMOVED;
			}
		}
		break;

		case AAPI_BODY:
		{
			const bool bMaxedOut = ( g_iThdQueueMax && !m_tState->m_bVIP && g_pThdPool->GetQueueLength()>=g_iThdQueueMax );

			if ( m_eCommand==SEARCHD_COMMAND_PING )
			{
				CachedOutputBuffer_c tOut;
				MemInputBuffer_c tIn ( m_tState->m_dBuf );
				HandleCommandPing ( tOut, m_uCommandVer, tIn );

				if ( tIn.GetError () )
					SendErrorReply ( tOut, "invalid command (code=%d, len=%d)", m_eCommand, m_tState->m_iLeft );

				m_tState->m_dBuf.Resize ( 0 );
				tOut.SwapData ( m_tState->m_dBuf );
				dNextTick.Add ( new NetSendData_t ( m_tState.LeakPtr (), Proto_e::SPHINX ) );

			} else if ( bMaxedOut )
			{
				m_tState->m_bKeepSocket = false;
				sphWarning ( "%s", g_sMaxedOutMessage );

				m_tState->m_dBuf.Resize ( 0 );
				CachedOutputBuffer_c tOut;
				tOut.SwapData ( m_tState->m_dBuf );

				APICommand_t tRetry ( tOut, SEARCHD_RETRY );
				tOut.SendString ( g_sMaxedOutMessage );

				tOut.SwapData ( m_tState->m_dBuf );
				dNextTick.Add ( new NetSendData_t ( m_tState.LeakPtr (), Proto_e::SPHINX ) );
			} else
			{
				pLoop->RemoveIterEvent (m_pParent);
				AddJobAPI ( pLoop );
				return NE_REMOVED;
			}
			return NE_REMOVE;
		}
		break;

		default: return NE_REMOVE;
		} // switch

		bool bNextWrite = ( m_ePhase==AAPI_HANDSHAKE_OUT );
		sphLogDebugv ( "%p post-API phase=%d, buf=%d, write=%d, sock=%d, tick=%u",
				this, m_ePhase, m_tState->m_dBuf.GetLength(), (int)bNextWrite,
				m_pParent->m_iSock, pLoop->m_uTick );
	}
}

void NetReceiveDataAPI_c::Impl_c::CloseSocketReceiveAPI()
{
	if ( m_tState.Ptr() )
		m_tState->CloseSocket();
}

NetReceiveDataAPI_c::NetReceiveDataAPI_c ( NetStateAPI_t *	pState )
	: ISphNetAction ( pState->m_iClientSock )
{
	m_pImpl = new Impl_c ( pState, this );
}

NetReceiveDataAPI_c::~NetReceiveDataAPI_c()
{
	SafeDelete ( m_pImpl );
}

NetEvent_e NetReceiveDataAPI_c::Loop ( DWORD uGotEvents, CSphVector<ISphNetAction *>
        & dNextTick, CSphNetLoop * pLoop )
{
	assert ( m_pImpl );
	return m_pImpl->LoopReceiveAPI ( uGotEvents, dNextTick, pLoop );
}

NetEvent_e NetReceiveDataAPI_c::Setup ( int64_t tmNow )
{
	assert ( m_pImpl );
	return m_pImpl->SetupReceiveAPI ( tmNow );
}

void NetReceiveDataAPI_c::CloseSocket ()
{
	assert ( m_pImpl );
	return m_pImpl->CloseSocketReceiveAPI ();
}

void NetReceiveDataAPI_c::SetupBodyPhase()
{
	assert ( m_pImpl );
	return m_pImpl->SetupBodyPhase ();
}