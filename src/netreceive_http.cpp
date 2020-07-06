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

#include "netreceive_http.h"
#include "searchdssl.h"

extern int g_iClientTimeoutS; // from searchd.cpp
extern volatile bool g_bMaintenance;

static const char g_sContentLength[] = "\r\r\n\nCcOoNnTtEeNnTt--LlEeNnGgTtHh\0";
static const size_t g_sContentLengthSize = sizeof ( g_sContentLength ) - 1;
static const char g_sHeadEnd[] = "\r\n\r\n";

struct HttpHeaderStreamParser_t
{
	int m_iHeaderEnd = 0;
	int m_iFieldContentLenStart = 0;
	int m_iFieldContentLenVal = 0;

	int m_iCur = 0;
	int m_iCRLF = 0;
	int m_iName = 0;

	bool HeaderFound ( ByteBlob_t tPacket )
	{
		if ( IsNull ( tPacket ) )
			return false;

		auto pBuf = tPacket.first;
		auto iLen = tPacket.second;
		// early exit at for already found request header
		if ( m_iHeaderEnd || m_iCur>=iLen )
			return true;

		const int iCNwoLFSize = ( g_sContentLengthSize-5 ) / 2; // size of just Content-Length field name
		for ( ; m_iCur<iLen; ++m_iCur )
		{
			m_iCRLF = ( pBuf[m_iCur]==g_sHeadEnd[m_iCRLF] ? m_iCRLF+1 : 0 );
			m_iName = ( !m_iFieldContentLenStart
					&& ( pBuf[m_iCur]==g_sContentLength[m_iName] || pBuf[m_iCur]==g_sContentLength[m_iName+1] )
					? m_iName+2 : 0 );

			// header end found
			if ( m_iCRLF==sizeof ( g_sHeadEnd )-1 )
			{
				m_iHeaderEnd = m_iCur+1;
				break;
			}
			// Content-Length field found
			if ( !m_iFieldContentLenStart && m_iName==g_sContentLengthSize-1 )
				m_iFieldContentLenStart = m_iCur-iCNwoLFSize+1;
		}

		// parse Content-Length field value
		while ( m_iHeaderEnd && m_iFieldContentLenStart )
		{
			int iNumStart = m_iFieldContentLenStart+iCNwoLFSize;
			// skip spaces
			while ( iNumStart<m_iHeaderEnd && pBuf[iNumStart]==' ' )
				++iNumStart;
			if ( iNumStart>=m_iHeaderEnd || pBuf[iNumStart]!=':' )
				break;

			++iNumStart; // skip ':' delimiter
			m_iFieldContentLenVal = atoi (
					(const char *) pBuf+iNumStart ); // atoi handles leading spaces and tail not digital chars
			break;
		}

		return ( m_iHeaderEnd>0 );
	}
};

#if 0
class ThdJobHttp_c::Impl_c
{
	friend class ThdJobHttp_c;

	CSphScopedPtr<NetStateAPI_t>		m_tState;
	CSphNetLoop *		m_pLoop;
	const bool			m_bSsl = false;

	Impl_c ( CSphNetLoop * pLoop, NetStateAPI_t * pState, bool bSsl );
	void		CallHttp ();
};

ThdJobHttp_c::Impl_c::Impl_c ( CSphNetLoop * pLoop, NetStateAPI_t * pState, bool bSsl )
	: m_tState ( pState )
	, m_pLoop ( pLoop )
,	  m_bSsl ( bSsl )
{
	assert ( m_tState.Ptr() );
}

void ThdJobHttp_c::Impl_c::CallHttp ()
{
	sphLogDebugv ( "%p http job started, buffer len=%d, tick=%u", this, m_tState->m_dBuf.GetLength(), m_pLoop->m_uTick );

	int iTid = GetOsThreadId();

	ThdDesc_t tThdDesc;
	tThdDesc.m_eProto = Proto_e::HTTP;
	tThdDesc.m_iClientSock = m_tState->m_iClientSock;
	tThdDesc.m_sClientName = m_tState->m_sClientName;
	tThdDesc.m_iConnID = m_tState->m_iConnID;
	tThdDesc.m_tmConnect = sphMicroTimer();
	tThdDesc.m_iTid = iTid;

	tThdDesc.AddToGlobal ();

	assert ( m_tState.Ptr() );

	CrashQuery_t tCrashQuery;
	tCrashQuery.m_pQuery = m_tState->m_dBuf.Begin();
	tCrashQuery.m_iSize = m_tState->m_dBuf.GetLength();
	tCrashQuery.m_bHttp = true;
	GlobalCrashQuerySet ( tCrashQuery );


	if ( g_bMaintenance && !m_tState->m_bVIP )
	{
		sphHttpErrorReply ( m_tState->m_dBuf, SPH_HTTP_STATUS_503, "server is in maintenance mode" );
		m_tState->m_bKeepSocket = false;
	} else
	{
		CSphVector<BYTE> dResult;
		m_tState->m_bKeepSocket = sphLoopClientHttp ( m_tState->m_dBuf.Begin(), m_tState->m_dBuf.GetLength(), dResult, tThdDesc );
		m_tState->m_dBuf = std::move(dResult);
	}

	GlobalCrashQuerySet ( CrashQuery_t() );
	tThdDesc.RemoveFromGlobal ();

	sphLogDebugv ( "%p http job done, tick=%u", this, m_pLoop->m_uTick );

	if ( g_bShutdown )
		return;

	assert ( m_pLoop );

	NetSendData_t * pSend = nullptr;
	if ( !m_bSsl )
	{
		pSend = new NetSendData_t ( m_tState.LeakPtr(), Proto_e::HTTP );
	} else
	{
		pSend = MakeReplyHttps_t ( m_tState.LeakPtr() );
	}
	JobDoSendNB ( pSend, m_pLoop );
}

ThdJobHttp_c::ThdJobHttp_c ( CSphNetLoop * pLoop, NetStateAPI_t * pState, bool bSsl )
{
	m_pImpl = new Impl_c ( pLoop, pState, bSsl );
}

ThdJobHttp_c::~ThdJobHttp_c ()
{
	SafeDelete ( m_pImpl );
}

void ThdJobHttp_c::Call ()
{
	assert ( m_pImpl );
	m_pImpl->CallHttp ();
}

class NetReceiveDataHttp_c::Impl_c
{
	friend class NetReceiveDataHttp_c;
	ISphNetAction*	m_pParent;

	CSphScopedPtr<NetStateAPI_t>	m_tState;
	HttpHeaderStreamParser_t		m_tHeadParser;

	explicit Impl_c ( NetStateAPI_t * pState, ISphNetAction* pParent );

	NetEvent_e		LoopHttp ( DWORD uGotEvents, CSphNetLoop * pLoop );
	NetEvent_e		SetupHttp ( int64_t tmNow );
	void			CloseSocketHttp ();
	void GrowBuffer();
};

NetReceiveDataHttp_c::Impl_c::Impl_c ( NetStateAPI_t * pState, ISphNetAction * pParent )
	: m_pParent ( pParent )
	, m_tState ( pState )
{
	assert ( m_tState.Ptr() );
	m_tState->m_iPos = 0;
	m_tState->m_iLeft = NET_STATE_HTTP_SIZE;
	m_tState->m_dBuf.Resize ( Max ( m_tState->m_dBuf.GetLimit(), NET_STATE_HTTP_SIZE ) );
}

NetEvent_e NetReceiveDataHttp_c::Impl_c::SetupHttp ( int64_t tmNow )
{
	assert ( m_tState.Ptr() );
	sphLogDebugv ( "%p receive HTTP setup, keep=%d, client=%s, conn=%d, sock=%d", this,
			m_tState->m_bKeepSocket, m_tState->m_sClientName, m_tState->m_iConnID, m_tState->m_iClientSock );

	if ( !m_tState->m_bKeepSocket )
	{
		m_pParent->m_iTimeoutTimeUS = tmNow + S2US * g_iReadTimeoutS;
	} else
	{
		m_pParent->m_iTimeoutTimeUS = tmNow + S2US * g_iClientTimeoutS;
	}
	return NE_IN;
}

void NetReceiveDataHttp_c::Impl_c::GrowBuffer()
{
	if ( m_tState->m_iLeft )
		return;

	auto iCanGrow = Min ( g_iMaxPacketSize - m_tState->m_iPos, 4096 );
	if ( !iCanGrow )
		return;

	m_tState->m_dBuf.AddN ( iCanGrow );
	m_tState->m_iLeft = iCanGrow;
}

NetEvent_e NetReceiveDataHttp_c::Impl_c::LoopHttp ( DWORD uGotEvents, CSphNetLoop * pLoop )
{
	assert ( m_tState.Ptr() );
	const char * sHttpError = "http error";
	if ( CheckSocketError ( uGotEvents, sHttpError, m_tState.Ptr(), false ) )
		return NE_REMOVE;

	// loop to handle similar operations at once
	while (true)
	{
		int64_t iRes = m_tState->SocketIO ( false );
		if ( iRes==-1 )
		{
			// FIXME!!! report back to client buffer overflow with 413 error
			LogSocketError ( sHttpError, m_tState.Ptr(), false );
			return NE_REMOVE;
		}
		m_tState->m_iLeft -= iRes;
		m_tState->m_iPos += iRes;

		// socket would block - going back to polling
		if ( iRes==0 && m_tState->m_iLeft )
			return NE_KEEP;

		// keep fetching data till the end of a header
		if ( !m_tHeadParser.m_iHeaderEnd )
		{
			if ( !m_tHeadParser.HeaderFound ( m_tState->m_dBuf.Begin(), m_tState->m_iPos ) )
			{
				GrowBuffer ();
				continue;
			}

			int iReqSize = m_tHeadParser.m_iHeaderEnd + m_tHeadParser.m_iFieldContentLenVal;
			if ( m_tHeadParser.m_iFieldContentLenVal && m_tState->m_iPos<iReqSize )
			{
				m_tState->m_dBuf.Resize ( iReqSize );
				m_tState->m_iLeft = iReqSize - m_tState->m_iPos;
				continue;
			}

			m_tState->m_iLeft = 0;
			m_tState->m_iPos = iReqSize;
			m_tState->m_dBuf.Resize ( iReqSize + 1 );
			m_tState->m_dBuf[m_tState->m_iPos] = '\0';
			m_tState->m_dBuf.Resize ( iReqSize );
		}

		// keep reading till end of buffer or data at socket
		if ( iRes>0 )
			continue;

		pLoop->RemoveIterEvent(m_pParent);

		sphLogDebugv ( "%p HTTP buf=%d, header=%d, content-len=%d, sock=%d, tick=%u",
				this, m_tState->m_dBuf.GetLength(), m_tHeadParser.m_iHeaderEnd,
				m_tHeadParser.m_iFieldContentLenVal, m_pParent->m_iSock, pLoop->m_uTick );

		// no VIP for http for now
		auto * pJob = new ThdJobHttp_c ( pLoop, m_tState.LeakPtr(), false );
		g_pThdPool->AddJob ( pJob );

		return NE_REMOVED;
	}
}

void NetReceiveDataHttp_c::Impl_c::CloseSocketHttp()
{
	if ( m_tState.Ptr() )
		m_tState->CloseSocket();
}

NetReceiveDataHttp_c::NetReceiveDataHttp_c ( NetStateAPI_t * pState )
	: ISphNetAction ( pState->m_iClientSock )
{
	m_pImpl = new Impl_c ( pState, this );
}

NetReceiveDataHttp_c::~NetReceiveDataHttp_c()
{
	SafeDelete ( m_pImpl );
}
#endif

void HttpServe ( AsyncNetBufferPtr_c pBuf, NetConnection_t * pConn )
{
	NetConnection_t & tConn = *pConn;

	// non-vip connections in maintainance should be already rejected on accept
	assert  ( !g_bMaintenance || tConn.m_bVIP );

	ThreadLocal_t tThd;
	auto & tThdesc = tThd.m_tDesc;
	tThdesc.m_eProto = Proto_e::HTTP; //< that is default
	tThdesc.m_iClientSock = tConn.m_iClientSock;
	tThdesc.m_sClientName = tConn.m_sClientName;
	tThdesc.m_iConnID = tConn.m_iConnID;
	tThdesc.m_tmStart = tThdesc.m_tmConnect = sphMicroTimer ();
	tThdesc.m_iTid = GetOsThreadId ();

	tThd.FinishInit ();

	// set off query guard
	CrashQuery_t tCrashQuery;
	tCrashQuery.m_bHttp = true;

	// set off query guard on return
	auto tRestoreCrash = AtScopeExit ( [] { GlobalCrashQuerySet ( CrashQuery_t () );});

	int iCID = tConn.m_iConnID;
	const char * sClientIP = tConn.m_sClientName;

	// needed to check permission to turn maintenance mode on/off

	bool bKeepAlive = false;

	if ( tConn.m_bSSL )
		tThdesc.m_bSsl = MakeSecureLayer ( pBuf );

	auto& tOut = *(NetGenericOutputBuffer_c *) pBuf;
	auto& tIn = *(AsyncNetInputBuffer_c *) pBuf;
	do
	{
		tIn.DiscardProcessed ( -1 ); // -1 means 'force flush'

		HttpHeaderStreamParser_t tHeadParser;
		while ( !tHeadParser.HeaderFound ( tIn.Tail() ))
		{
			auto iChunk = tIn.ReadAny ( g_iMaxPacketSize );
			if ( iChunk>0 )
				continue;

			return; // todo! report - iChunk==0 is limit reached, iChunk<0 is some other error
		}

		int iPacketLen = tHeadParser.m_iHeaderEnd+tHeadParser.m_iFieldContentLenVal;
		if ( !tIn.ReadFrom ( iPacketLen )) {
			sphWarning ( "failed to receive HTTP request (client=%s(%d), exp=%d, error='%s')", sClientIP, iCID,
						 iPacketLen, sphSockError ());
			return;
		}

		// Temporary write \0 at the end, since parser wants z-terminated buf
		auto uOldByte = tIn.Terminate ( iPacketLen, '\0' );
		auto tPacket = tIn.PopTail (iPacketLen);

		tCrashQuery.m_pQuery = tPacket.first;
		tCrashQuery.m_iSize = tPacket.second;
		GlobalCrashQuerySet ( tCrashQuery );

		CSphVector<BYTE> dResult;
		if ( sphLoopClientHttp ( tPacket.first, tPacket.second, dResult, tThdesc ) )
		{
			if ( !bKeepAlive )
				tIn.SetTimeoutUS ( S2US * g_iClientTimeoutS );
			bKeepAlive = true;
		} else {
			if ( bKeepAlive )
				tIn.SetTimeoutUS ( S2US * g_iReadTimeoutS );
			bKeepAlive = false;
		}

		tIn.Terminate ( 0, uOldByte ); // return back prev byte

		tOut.SwapData (dResult);
		tOut.Flush();
	} while ( bKeepAlive );
}