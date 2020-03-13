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

#include "netreceive_https.h"
#include "netreceive_httpcommon.h"
#include "searchdssl.h"

extern int g_iClientTimeout; // from searchd.cpp
extern volatile bool g_bMaintenance;
static auto& g_bShutdown = sphGetShutdown ();

NetStateHttps_t::~NetStateHttps_t()
{
	SslFree ( m_pSession );
	m_pSession = nullptr;
}

struct NetReplyHttps_t : public NetSendData_t
{
	NetStateHttps_t * m_pState = nullptr;

	explicit NetReplyHttps_t ( NetStateHttps_t * pState );
	NetEvent_e		Setup ( int64_t tmNow ) override;
	void			CloseSocket () override;
};

NetReplyHttps_t::NetReplyHttps_t ( NetStateHttps_t * pState )
	: NetSendData_t ( pState, Proto_e::HTTPS )
	, m_pState ( pState )
{
}

NetEvent_e NetReplyHttps_t::Setup ( int64_t tmNow )
{
	assert ( m_tState.Ptr() );
	assert ( m_pState->m_pSession );

	sphLogDebugv ( "%p send HTTPS setup, keep=%d, buf=%d, client=%s, conn=%d, sock=%d", this, (int)(m_tState->m_bKeepSocket),
		m_tState->m_dBuf.GetLength(), m_tState->m_sClientName, m_tState->m_iConnID, m_tState->m_iClientSock );

	if ( !m_bContinue )
	{
		m_iTimeoutTime = tmNow + MS2SEC * g_iWriteTimeout;

		assert ( m_tState->m_dBuf.GetLength() );
		if ( !SslSend ( m_pState->m_pSession, m_pState->m_dDecrypted, m_pState->m_dBuf ) )
			return NE_REMOVE;

		m_pState->m_dDecrypted.SwapData ( m_pState->m_dBuf );
		m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
		m_tState->m_iPos = 0;
	}
	m_bContinue = false;

	return NE_OUT;
}

void NetReplyHttps_t::CloseSocket ()
{
	if ( m_tState.Ptr() )
		m_tState->CloseSocket();
}

class NetReceiveDataHttps_c::Impl_c
{
	friend class NetReceiveDataHttps_c;
	ISphNetAction * m_pParent;

	CSphScopedPtr<NetStateHttps_t>	m_tState;
	HttpHeaderStreamParser_t		m_tHeadParser;
	bool							m_bWrite = false;

	Impl_c ( NetStateHttps_t * pState, ISphNetAction * pParent );

	NetEvent_e 		LoopHttps ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop );
	NetEvent_e		SetupHttps ( int64_t tmNow );
	void			CloseSocketHttps ();
	void			StartJob ( CSphNetLoop * pLoop );
};

NetReceiveDataHttps_c::Impl_c::Impl_c ( NetStateHttps_t * pState, ISphNetAction * pParent )
	: m_pParent ( pParent )
	, m_tState ( pState )
{
	assert ( m_tState.Ptr() );
	m_tState->m_iPos = 0;
	m_tState->m_iLeft = NET_STATE_HTTP_SIZE;
	m_tState->m_dBuf.Resize ( Max ( m_tState->m_dBuf.GetLimit(), NET_STATE_HTTP_SIZE ) );
}

void NetReceiveDataHttps_c::Impl_c::CloseSocketHttps()
{
	if ( m_tState.Ptr() )
		m_tState->CloseSocket();
}

NetEvent_e NetReceiveDataHttps_c::Impl_c::SetupHttps ( int64_t tmNow )
{
	assert ( m_tState.Ptr() );
	sphLogDebugv ( "%p receive HTTPS setup, keep=%d, client=%s, conn=%d, sock=%d", this, m_tState->m_bKeepSocket, m_tState->m_sClientName, m_tState->m_iConnID, m_tState->m_iClientSock );

	if ( !m_tState->m_bKeepSocket )
	{
		m_pParent->m_iTimeoutTime = tmNow + MS2SEC * g_iReadTimeout;
	}
	else
	{
		m_pParent->m_iTimeoutTime = tmNow + MS2SEC * g_iClientTimeout;
	}
	m_bWrite = false;

	m_tState->m_pSession = SslSetup ( m_tState->m_pSession );
	m_tState->m_dDecrypted.Resize ( 0 );

	return NE_IN;
}

NetEvent_e NetReceiveDataHttps_c::Impl_c::LoopHttps ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNext,
		CSphNetLoop * pLoop )
{
	assert ( m_tState.Ptr() );
	const char * sHttpError = "https error";
	if ( CheckSocketError ( uGotEvents, sHttpError, m_tState.Ptr(), false ) )
		return NE_REMOVE;

	// loop to handle similar operations at once
	const bool bWasWrite = m_bWrite;
	bool bGotData = false;
	while (true)
	{
		int iRes = m_tState->SocketIO ( m_bWrite );
		if ( iRes==-1 )
		{
			// FIXME!!! report back to client buffer overflow with 413 error
			LogSocketError ( sHttpError, m_tState.Ptr(), false );
			return NE_REMOVE;
		}

		int iOff = m_tState->m_iPos;
		m_tState->m_iLeft -= iRes;
		m_tState->m_iPos += iRes;

		sphLogDebugv ( "%p HTTPS %s len %d(%d), off %d, decrypted=%d, sock=%d, tick=%u", this, m_bWrite ? "write" : "read", iRes, m_tState->m_iLeft, iOff, m_tState->m_dDecrypted.GetLength(), m_pParent->m_iSock, pLoop->m_uTick );

		// socket would block - going back to polling
		if ( iRes==0 )
		{
			if ( bWasWrite!=m_bWrite )
				return ( m_bWrite ? NE_OUT : NE_IN );
			else
				return NE_KEEP;
		}

		if ( m_bWrite && m_tState->m_iLeft==0 )
		{
			if ( bGotData
				&& m_tHeadParser.HeaderFound ( m_tState->m_dDecrypted.Begin(), m_tState->m_dDecrypted.GetLength() )
				&& m_tHeadParser.m_iHeaderEnd + m_tHeadParser.m_iFieldContentLenVal>=m_tState->m_dDecrypted.GetLength() )
			{
				pLoop->RemoveIterEvent ( m_pParent );
				StartJob ( pLoop );
				return NE_REMOVED;
			}

			// get actual encoded data after handshake
			m_tState->m_dBuf.Resize ( NET_STATE_HTTP_SIZE );
			m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
			m_tState->m_iPos = 0;
			m_bWrite = false;
			return ( bWasWrite ? NE_IN : NE_KEEP );
		}

		const int iDecryptedLen = m_tState->m_dDecrypted.GetLength();
		bool bTickWrite = m_bWrite;
		bool bDone = SslTick ( m_tState->m_pSession, bTickWrite, m_tState->m_dBuf, iRes, iOff, m_tState->m_dDecrypted );

		// SSL reports finish only on error
		if ( bDone )
			return NE_REMOVE;

		bool bChange = ( bTickWrite!=m_bWrite );
		m_bWrite = bTickWrite;
		if ( bChange )
		{
			m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
			m_tState->m_iPos = 0;
			bGotData |= ( iDecryptedLen<m_tState->m_dDecrypted.GetLength() );
			continue;
		}

		// grow read buffer
		if ( !m_bWrite && iRes+iOff==m_tState->m_dBuf.GetLength() )
		{
			int iCanGrow = Min ( g_iMaxPacketSize - m_tState->m_iPos, 4096 );
			if ( !iCanGrow )
			{
				sphWarning ( "ill-formed client request (length=%d out of bounds) (client=%s(%d)), sock=%d", m_tState->m_iLeft, m_tState->m_sClientName, m_tState->m_iConnID, m_tState->m_iClientSock );
				return NE_REMOVE;
			}

			m_tState->m_dBuf.AddN ( iCanGrow );
			m_tState->m_iLeft = iCanGrow;
		}

		// check on new part of decoded data
		if ( iDecryptedLen<m_tState->m_dDecrypted.GetLength()
			&& m_tHeadParser.HeaderFound ( m_tState->m_dDecrypted.Begin(), m_tState->m_dDecrypted.GetLength() )
			&& m_tHeadParser.m_iHeaderEnd + m_tHeadParser.m_iFieldContentLenVal>=m_tState->m_dDecrypted.GetLength() )
		{
			pLoop->RemoveIterEvent ( m_pParent );
			StartJob ( pLoop );
			return NE_REMOVED;
		}

		// keep reading till end of buffer or data at socket
		if ( iRes>0 )
			continue;

		return NE_KEEP;
	}
}

void NetReceiveDataHttps_c::Impl_c::StartJob ( CSphNetLoop * pLoop )
{
	sphLogDebugv ( "%p HTTPS buf=%d, '%.*s', header=%d, content-len=%d, sock=%d, tick=%u",
			this, m_tState->m_dDecrypted.GetLength(), Min ( m_tState->m_dDecrypted.GetLength(), 128 ),
			m_tState->m_dDecrypted.Begin(), m_tHeadParser.m_iHeaderEnd, m_tHeadParser.m_iFieldContentLenVal,
			m_pParent->m_iSock, pLoop->m_uTick );

	m_tState->m_dDecrypted.SwapData ( m_tState->m_dBuf );
	auto * pJob = new ThdJobHttp_c ( pLoop, m_tState.LeakPtr(), true );
	g_pThdPool->AddJob ( pJob );
}

NetReceiveDataHttps_c::NetReceiveDataHttps_c ( NetStateHttps_t * pState )
	: ISphNetAction ( pState->m_iClientSock )
{
	m_pImpl = new Impl_c ( pState, this );
}

NetReceiveDataHttps_c::~NetReceiveDataHttps_c()
{
	SafeDelete ( m_pImpl );
}

void NetReceiveDataHttps_c::CloseSocket()
{
	assert ( m_pImpl );
	m_pImpl->CloseSocketHttps ();
}

NetEvent_e NetReceiveDataHttps_c::Setup ( int64_t tmNow )
{
	assert ( m_pImpl );
	return m_pImpl->SetupHttps (tmNow);
}

NetEvent_e NetReceiveDataHttps_c::Loop ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNext, CSphNetLoop * pLoop )
{
	assert ( m_pImpl );
	return m_pImpl->LoopHttps ( uGotEvents, dNext, pLoop);
}

NetSendData_t * MakeReplyHttps_t ( NetStateAPI_t * pState )
{
	return new NetReplyHttps_t ((NetStateHttps_t *) pState );
}