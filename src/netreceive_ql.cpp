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

#include "netreceive_ql.h"

extern char g_sMysqlHandshake[128];
extern int g_iMysqlHandshake;
extern int g_iClientQlTimeoutS;    // sec
extern int g_iThdQueueMax;
extern volatile bool g_bMaintenance;
static auto& g_bShutdown = sphGetShutdown ();

struct ThdJobQL_t : public ISphJob
{
	CSphScopedPtr<NetStateQL_t>		m_pState;
	CSphNetLoop *		m_pLoop;

	ThdJobQL_t ( CSphNetLoop * pLoop, NetStateQL_t * pState );

	void		Call () final;
};

ThdJobQL_t::ThdJobQL_t ( CSphNetLoop * pLoop, NetStateQL_t * pState )
	: m_pState ( pState )
	, m_pLoop ( pLoop )
{
	assert ( m_pState.Ptr() );
}

void ThdJobQL_t::Call ()
{
	sphLogDebugv ( "%p QL job started, tick=%u", this, m_pLoop->m_uTick );

	int iTid = GetOsThreadId();

	ThreadLocal_t tThd;
	auto & tThdDesc = tThd.m_tDesc;
	tThdDesc.m_eProto = Proto_e::MYSQL41;
	tThdDesc.m_iClientSock = m_pState->m_iClientSock;
	tThdDesc.m_sClientName = m_pState->m_sClientName;
	tThdDesc.m_iConnID = m_pState->m_iConnID;
	tThdDesc.m_tmConnect = sphMicroTimer();
	tThdDesc.m_iTid = iTid;

	tThdDesc.AddToGlobal ();

	CSphString sQuery; // to keep data alive for SphCrashQuery_c
	auto bProfile = m_pState->m_tSession.StartProfiling ();

	MemInputBuffer_c tIn ( m_pState->m_dBuf );
	ISphOutputBuffer tOut;

	// needed to check permission to turn maintenance mode on/off
	m_pState->m_tSession.SetVIP ( m_pState->m_bVIP );

	m_pState->m_bKeepSocket = false;
	bool bSendResponse = true;
	if ( g_bMaintenance && !m_pState->m_bVIP )
		SendMysqlErrorPacket ( tOut, m_pState->m_uPacketID, nullptr, "server is in maintenance mode",
				tThdDesc.m_iConnID, MYSQL_ERR_UNKNOWN_COM_ERROR );
	else
	{
		bSendResponse = LoopClientMySQL ( m_pState->m_uPacketID, m_pState->m_tSession.Impl(), sQuery,
				m_pState->m_dBuf.GetLength(), bProfile, tThd, tIn, tOut );
		m_pState->m_bKeepSocket = bSendResponse;
	}

	sphLogDebugv ( "%p QL job done, tick=%u", this, m_pLoop->m_uTick );

	if ( bSendResponse && !g_bShutdown )
	{
		assert ( m_pLoop );
		tOut.SwapData ( m_pState->m_dBuf );
		JobDoSendNB ( new NetSendData_t ( m_pState.LeakPtr (), Proto_e::MYSQL41 ), m_pLoop );
	}

	tThdDesc.RemoveFromGlobal ();
}


enum ActionQL_e : BYTE
{
	AQL_HANDSHAKE = 0,
	AQL_LENGTH,
	AQL_BODY,
	AQL_AUTH,
	AQL_TOTAL
};

const char * g_sErrorNetQL[] = { "failed to send SphinxQL handshake", "bailing on failed MySQL header", "failed to receive MySQL request body", "failed to send SphinxQL auth" };
STATIC_ASSERT ( sizeof(g_sErrorNetQL)/sizeof(g_sErrorNetQL[0])==AQL_TOTAL, NOT_ALL_EMUN_DESCRIBERD );


static DWORD NetBufGetLSBDword ( const BYTE * pBuf )
{
	return pBuf[0] + ( ( pBuf[1] ) <<8 ) + ( ( pBuf[2] )<<16 ) + ( ( pBuf[3] )<<24 );
}

class NetReceiveDataQL_c::Impl_c
{
	friend class NetReceiveDataQL_c;
	ISphNetAction * m_pParent;

	CSphScopedPtr<NetStateQL_t>		m_pState;
	ActionQL_e			m_ePhase;
	bool				m_bAppend;
	bool				m_bWrite;

	Impl_c ( NetStateQL_t * pState, ISphNetAction * pParent );

	NetEvent_e		LoopQl ( DWORD uGotEvents, CSphNetLoop * pLoop );
	NetEvent_e		SetupQl ( int64_t tmNow );
	void			CloseSocketQl ();

	void				SetupHandshakePhase();
	void				SetupBodyPhase();
};

NetReceiveDataQL_c::Impl_c::Impl_c ( NetStateQL_t * pState, ISphNetAction * pParent )
	: m_pParent ( pParent )
	, m_pState ( pState )
{
	m_ePhase = AQL_HANDSHAKE;
	m_pState->m_iLeft = 0;
	m_pState->m_iPos = 0;
	m_bAppend = false;
	m_bWrite = false;
	assert ( m_pState.Ptr() );
}

void NetReceiveDataQL_c::Impl_c::SetupHandshakePhase()
{
	m_ePhase = AQL_HANDSHAKE;
	m_pState->m_dBuf.Append( g_sMysqlHandshake, g_iMysqlHandshake );
	m_pState->m_iLeft = m_pState->m_dBuf.GetLength();
	m_pState->m_iPos = 0;
	m_bAppend = false;
	m_bWrite = true;
}

void NetReceiveDataQL_c::Impl_c::SetupBodyPhase()
{
	m_pState->m_dBuf.Resize ( 4 );
	m_pState->m_iLeft = m_pState->m_dBuf.GetLength();
	m_pState->m_iPos = 0;
	m_bAppend = false;
	m_bWrite = false;
	m_ePhase = AQL_LENGTH;
}

NetEvent_e NetReceiveDataQL_c::Impl_c::SetupQl ( int64_t tmNow )
{
	assert ( m_pState.Ptr() );
	sphLogDebugv ( "%p receive QL setup, phase=%d, client=%s, conn=%d, sock=%d", m_pParent,
			m_ePhase, m_pState->m_sClientName, m_pState->m_iConnID, m_pState->m_iClientSock );

	m_pParent->m_iTimeoutTimeUS = tmNow + S2US * g_iClientQlTimeoutS;

	NetEvent_e eEvent;
	if ( m_ePhase==AQL_HANDSHAKE )
	{
		eEvent = NE_OUT;
		m_bWrite = true;
	} else
	{
		eEvent = NE_IN;
		m_bWrite = false;
	}
	return eEvent;
}

NetEvent_e NetReceiveDataQL_c::Impl_c::LoopQl ( DWORD uGotEvents, CSphNetLoop * pLoop )
{
	assert ( m_pState.Ptr() );
	bool bDebugErr = ( ( m_ePhase==AQL_LENGTH && !m_bAppend ) || m_ePhase==AQL_AUTH );
	if ( CheckSocketError ( uGotEvents, g_sErrorNetQL[m_ePhase], m_pState.Ptr(), bDebugErr ) )
		return NE_REMOVE;

	bool bWrite = m_bWrite;
	// loop to handle similar operations at once
	while (true)
	{
		int64_t iRes = m_pState->SocketIO ( m_bWrite, bWrite );
		if ( iRes==-1 )
		{
			LogSocketError ( g_sErrorNetQL[m_ePhase], m_pState.Ptr(), false );
			return NE_REMOVE;
		}
		m_pState->m_iLeft -= iRes;
		m_pState->m_iPos += iRes;

		// socket would block - going back to polling
		if ( iRes==0 )
		{
			if ( bWrite!=m_bWrite )
				return ( m_bWrite ? NE_OUT : NE_IN );
			else
				return NE_KEEP;
		}

		// keep using that socket
		if ( m_pState->m_iLeft )
			continue;

		sphLogDebugv ( "%p pre-QL phase=%d, buf=%d, append=%d, write=%d, sock=%d, tick=%u", m_pParent, m_ePhase,
				m_pState->m_dBuf.GetLength(), (int)m_bAppend, (int)m_bWrite, m_pParent->m_iSock, pLoop->m_uTick );

		switch ( m_ePhase )
		{
		case AQL_HANDSHAKE:
			// no action required - switching to next phase
			SetupBodyPhase();
		break;

		case AQL_LENGTH:
		{
			const int MAX_PACKET_LEN = 0xffffffL; // 16777215 bytes, max low level packet size
			DWORD uHeader = 0;
			const BYTE * pBuf = ( m_bAppend ? m_pState->m_dBuf.Begin ()+m_pState->m_iPos-sizeof ( uHeader )
					: m_pState->m_dBuf.Begin () );
			uHeader = NetBufGetLSBDword ( pBuf );
			m_pState->m_uPacketID = 1 + (BYTE)( uHeader>>24 ); // client will expect this id
			m_pState->m_iLeft = ( uHeader & MAX_PACKET_LEN );

			if ( m_bAppend ) // reading big packet
			{
				m_pState->m_iPos = m_pState->m_dBuf.GetLength() - sizeof(uHeader);
				m_pState->m_dBuf.Resize ( m_pState->m_iPos +m_pState->m_iLeft );
			} else // reading regular packet
			{
				m_pState->m_iPos = 0;
				m_pState->m_dBuf.Resize ( m_pState->m_iLeft );
			}

			// check request length
			if ( m_pState->m_dBuf.GetLength()>g_iMaxPacketSize )
			{
				sphWarning ( "ill-formed client request (length=%d out of bounds)", m_pState->m_dBuf.GetLength() );
				return NE_REMOVE;
			}

			m_bWrite = false;
			m_bAppend = ( m_pState->m_iLeft==MAX_PACKET_LEN ); // might be next big packet
			m_ePhase = AQL_BODY;
		}
		break;

		case AQL_BODY:
		{
			if ( m_bAppend )
			{
				m_pState->m_iLeft = 4;
				m_pState->m_dBuf.Resize ( m_pState->m_iPos +m_pState->m_iLeft );

				m_bWrite = false;
				m_ePhase = AQL_LENGTH;
			} else if ( !m_pState->m_bAuthed )
			{
				m_pState->m_bAuthed = true;
				if ( IsFederatedUser ( m_pState->m_dBuf.Begin(), m_pState->m_dBuf.GetLength() ) )
					m_pState->m_tSession.SetFederatedUser();
				m_pState->m_dBuf.Resize ( 0 );
				ISphOutputBuffer tOut;
				tOut.SwapData ( m_pState->m_dBuf );
				SendMysqlOkPacket ( tOut, m_pState->m_uPacketID, m_pState->m_tSession.IsAutoCommit() );
				tOut.SwapData ( m_pState->m_dBuf );

				m_pState->m_iLeft = m_pState->m_dBuf.GetLength();
				m_pState->m_iPos = 0;

				m_bWrite = true;
				m_ePhase = AQL_AUTH;
			} else
			{
				CSphVector<BYTE> & dBuf = m_pState->m_dBuf;
				int iBufLen = dBuf.GetLength();
				int iCmd = ( iBufLen>0 ? dBuf[0] : 0 );
				int iStrLen = Min ( iBufLen, 80 );
				sphLogDebugv ( "%p receive-QL, cmd=%d, buf=%d, sock=%d, '%.*s'", m_pParent, iCmd, iBufLen,
						m_pParent->m_iSock, iStrLen, ( dBuf.GetLength() ? (const char *)dBuf.Begin() : "" ) );

				bool bEmptyBuf = !dBuf.GetLength();
				bool bMaxedOut = ( iCmd==MYSQL_COM_QUERY && !m_pState->m_bVIP
						&& g_iThdQueueMax && g_pThdPool->GetQueueLength()>=g_iThdQueueMax );

				if ( bEmptyBuf || bMaxedOut )
				{
					dBuf.Resize ( 0 );
					ISphOutputBuffer tOut;
					tOut.SwapData ( dBuf );

					if ( !bMaxedOut )
					{
						SendMysqlErrorPacket ( tOut, m_pState->m_uPacketID, "", "unknown command with size 0",
								m_pState->m_iConnID, MYSQL_ERR_UNKNOWN_COM_ERROR );
					} else
					{
						SendMysqlErrorPacket ( tOut, m_pState->m_uPacketID, "", "Too many connections",
								m_pState->m_iConnID, MYSQL_ERR_TOO_MANY_USER_CONNECTIONS );
					}

					tOut.SwapData ( dBuf );
					m_pState->m_iLeft = dBuf.GetLength();
					m_pState->m_iPos = 0;
					m_bWrite = true;
					m_ePhase = AQL_AUTH;
				} else
				{
					// all comes to an end
					if ( iCmd==MYSQL_COM_QUIT )
					{
						m_pState->m_bKeepSocket = false;
						return NE_REMOVE;
					}

					if ( iCmd==MYSQL_COM_QUERY )
					{
						pLoop->RemoveIterEvent(m_pParent);

						// going to actual work now
						bool bStart = m_pState->m_bVIP;
						auto * pJob = new ThdJobQL_t ( pLoop, m_pState.LeakPtr() );
						if ( bStart )
							g_pThdPool->StartJob ( pJob );
						else
							g_pThdPool->AddJob ( pJob );

						return NE_REMOVED;
					} else
					{
						// short-cuts to keep action in place and don't dive to job then get back here
						CSphString sError;
						m_pState->m_dBuf.Resize ( 0 );
						ISphOutputBuffer tOut;
						tOut.SwapData ( m_pState->m_dBuf );
						switch ( iCmd )
						{
						case MYSQL_COM_PING:
						case MYSQL_COM_INIT_DB:
							// client wants a pong
							SendMysqlOkPacket ( tOut, m_pState->m_uPacketID, m_pState->m_tSession.IsAutoCommit () );
							break;

						case MYSQL_COM_SET_OPTION:
							// bMulti = ( tIn.GetWord()==MYSQL_OPTION_MULTI_STATEMENTS_ON );
							// that's how we could double check and validate multi query
							// server reporting success in response to COM_SET_OPTION and COM_DEBUG
							SendMysqlEofPacket ( tOut, m_pState->m_uPacketID, 0, false, m_pState->m_tSession.IsAutoCommit () );
							break;

						default:
							// default case, unknown command
							sError.SetSprintf ( "unknown command (code=%d)", iCmd );
							SendMysqlErrorPacket ( tOut, m_pState->m_uPacketID, "", sError.cstr(),
									m_pState->m_iConnID, MYSQL_ERR_UNKNOWN_COM_ERROR );
							break;
						}

						tOut.SwapData ( dBuf );
						m_pState->m_iLeft = dBuf.GetLength();
						m_pState->m_iPos = 0;
						m_bWrite = true;
						m_ePhase = AQL_AUTH;
					}
				}
			}
		}
		break;

		case AQL_AUTH:
			m_pState->m_iLeft = 4;
			m_pState->m_dBuf.Resize ( m_pState->m_iLeft );
			m_pState->m_iPos = 0;

			m_bWrite = false;
			m_ePhase = AQL_LENGTH;
			break;

		default: return NE_REMOVE;
		}

		sphLogDebugv ( "%p post-QL phase=%d, buf=%d, append=%d, write=%d, sock=%d, tick=%u", m_pParent, m_ePhase,
				m_pState->m_dBuf.GetLength(), (int)m_bAppend, (int)m_bWrite, m_pParent->m_iSock, pLoop->m_uTick );
	}
}


void NetReceiveDataQL_c::Impl_c::CloseSocketQl()
{
	if ( m_pState.Ptr() )
		m_pState->CloseSocket();
}

NetReceiveDataQL_c::NetReceiveDataQL_c ( NetStateQL_t * pState)
	: ISphNetAction ( pState->m_iClientSock )
{
	m_pImpl = new Impl_c ( pState, this );
}

NetReceiveDataQL_c::~NetReceiveDataQL_c()
{
	SafeDelete ( m_pImpl );
}

void NetReceiveDataQL_c::SetupHandshakePhase()
{
	assert ( m_pImpl );
	m_pImpl->SetupHandshakePhase ();
}

void NetReceiveDataQL_c::SetupBodyPhase()
{
	assert ( m_pImpl );
	m_pImpl->SetupBodyPhase ();
}

NetEvent_e NetReceiveDataQL_c::Setup ( int64_t tmNow )
{
	assert ( m_pImpl );
	return m_pImpl->SetupQl ( tmNow );
}

NetEvent_e NetReceiveDataQL_c::Loop ( DWORD uGotEvents, CSphVector<ISphNetAction *> &, CSphNetLoop * pLoop )
{
	assert ( m_pImpl );
	return m_pImpl->LoopQl (uGotEvents, pLoop);
}

void NetReceiveDataQL_c::CloseSocket()
{
	assert ( m_pImpl );
	m_pImpl->CloseSocketQl ();
}