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
#include "query_status.h"

#if USE_WINDOWS
// Win-specific headers and calls
#include <io.h>
#else
// UNIX-specific headers and calls
#include <sys/wait.h>
#include <netdb.h> // not need on Mac
#include <netinet/in.h>
#include <netinet/tcp.h>

#endif

static auto & g_bGotSigterm = sphGetGotSigterm ();    // we just received SIGTERM; need to shutdown


class NetOutputBuffer_c : public NetGenericOutputBuffer_c
{
public:
	explicit	NetOutputBuffer_c ( int iSock );

	void	Flush () override;

private:
	int			m_iSock;			///< my socket
};

/////////////////////////////////////////////////////////////////////////////

NetOutputBuffer_c::NetOutputBuffer_c( int iSock )
	: m_iSock( iSock )
{
	assert ( m_iSock>0 );
}

void NetOutputBuffer_c::Flush()
{
	CommitAllMeasuredLengths();

	if ( m_bError )
		return;

	int64_t iLen = m_dBuf.GetLength64 ();
	if ( !iLen )
		return;

	if ( g_bGotSigterm )
		sphLogDebug( "SIGTERM in NetOutputBuffer::Flush" );

	StringBuilder_c sError;
	auto* pBuffer = ( const char* ) m_dBuf.Begin();

	CSphScopedProfile tProf( m_pProfile, SPH_QSTATE_NET_WRITE );

	const int64_t tmMaxTimer = sphMicroTimer() + S2US * g_iWriteTimeoutS; // in microseconds
	while ( !m_bError )
	{
		auto iRes = sphSockSend ( m_iSock, pBuffer, iLen );
		if ( iRes<0 )
		{
			int iErrno = sphSockGetErrno();
			if ( iErrno==EINTR ) // interrupted before any data was sent; just loop
				continue;
			if ( iErrno!=EAGAIN && iErrno!=EWOULDBLOCK )
			{
				sError.Sprintf( "send() failed: %d: %s", iErrno, sphSockError( iErrno ));
				sphWarning( "%s", sError.cstr());
				m_bError = true;
				break;
			}
		} else
		{
			pBuffer += iRes;
			iLen -= iRes;
			if ( iLen==0 )
				break;
		}

		// wait until we can write
		int64_t tmMicroLeft = tmMaxTimer - sphMicroTimer();
		iRes = 0;
		if ( tmMicroLeft>0 )
			iRes = sphPoll( m_iSock, tmMicroLeft, true );

		if ( !iRes ) // timeout
		{
			sError << "timed out while trying to flush network buffers";
			sphWarning( "%s", sError.cstr());
			m_bError = true;
			break;
		}

		if ( iRes<0 )
		{
			int iErrno = sphSockGetErrno();
			if ( iErrno==EINTR )
				break;
			sError.Sprintf( "sphPoll() failed: %d: %s", iErrno, sphSockError( iErrno ));
			sphWarning( "%s", sError.cstr());
			m_bError = true;
			break;
		}
		assert ( iRes>0 );
	}

	m_dBuf.Resize( 0 );
}

/// simple network request buffer
// todo! remove in favour of async buf
// the *only* usecase for now is legacy QueryStatus() in searchd.cpp
class NetInputBuffer_c : private LazyVector_T<BYTE>, public InputBuffer_c
{
	using STORE = LazyVector_T<BYTE>;
public:
	explicit		NetInputBuffer_c ( int iSock );

	bool			ReadFrom ( int iLen, int iTimeout, bool bIntr=false, bool bAppend=false );
	bool			ReadFrom ( int iLen ) { return ReadFrom ( iLen, g_iReadTimeoutS ); }

	bool			IsIntr () const { return m_bIntr; }

	using InputBuffer_c::HasBytes;
private:
	static const int	NET_MINIBUFFER_SIZE = STORE::iSTATICSIZE;

	int					m_iSock;
	bool				m_bIntr = false;
};

/////////////////////////////////////////////////////////////////////////////

int RecvNBChunk( int iSock, char*& pBuf, int& iLeftBytes )
{
	// try to receive next chunk
	auto iRes = sphSockRecv ( iSock, pBuf, iLeftBytes );

	if ( iRes>0 )
	{
		pBuf += iRes;
		iLeftBytes -= iRes;
	}
	return ( int ) iRes;
}

int sphSockRead( int iSock, void* buf, int iLen, int iReadTimeout, bool bIntr )
{
	assert ( iLen>0 );

	int64_t tmMaxTimer = sphMicroTimer() + I64C( 1000000 ) * Max( 1, iReadTimeout ); // in microseconds
	int iLeftBytes = iLen; // bytes to read left

	auto pBuf = ( char* ) buf;
	int iErr = 0;
	int iRes = -1;

	while ( iLeftBytes>0 )
	{
		int64_t tmMicroLeft = tmMaxTimer - sphMicroTimer();
		if ( tmMicroLeft<=0 )
			break; // timed out

#if USE_WINDOWS
		// Windows EINTR emulation
		// Ctrl-C will not interrupt select on Windows, so let's handle that manually
		// forcibly limit select() to 100 ms, and check flag afterwards
		if ( bIntr )
			tmMicroLeft = Min ( tmMicroLeft, 100000 );
#endif

		// wait until there is data
		iRes = sphPoll( iSock, tmMicroLeft );

		// if there was EINTR, retry
		// if any other error, bail
		if ( iRes==-1 )
		{
			// only let SIGTERM (of all them) to interrupt, and only if explicitly allowed
			iErr = sphSockGetErrno();
			if ( iErr==EINTR )
			{
				if ( !( g_bGotSigterm && bIntr ))
					continue;
				sphLogDebug( "sphSockRead: select got SIGTERM, exit -1" );
			}
			return -1;
		}

		// if there was a timeout, report it as an error
		if ( iRes==0 )
		{
#if USE_WINDOWS
			// Windows EINTR emulation
			if ( bIntr )
			{
				// got that SIGTERM
				if ( g_bGotSigterm )
				{
					sphLogDebug ( "sphSockRead: got SIGTERM emulation on Windows, exit -1" );
					sphSockSetErrno ( EINTR );
					return -1;
				}

				// timeout might not be fully over just yet, so re-loop
				continue;
			}
#endif

			sphSockSetErrno( ETIMEDOUT );
			return -1;
		}

		// try to receive next chunk
		iRes = RecvNBChunk( iSock, pBuf, iLeftBytes );

		// if there was eof, we're done
		if ( !iRes )
		{
			sphSockSetErrno( ECONNRESET );
			return -1;
		}

		// if there was EINTR, retry
		// if any other error, bail
		if ( iRes==-1 )
		{
			// only let SIGTERM (of all them) to interrupt, and only if explicitly allowed
			iErr = sphSockGetErrno();
			if ( iErr==EINTR )
			{
				if ( !( g_bGotSigterm && bIntr ))
					continue;
				sphLogDebug( "sphSockRead: select got SIGTERM, exit -1" );
			}
			return -1;
		}

		// avoid partial buffer loss in case of signal during the 2nd (!) read
		bIntr = false;
	}

	// if there was a timeout, report it as an error
	if ( iLeftBytes!=0 )
	{
		sphSockSetErrno( ETIMEDOUT );
		return -1;
	}

	return iLen;
}

NetInputBuffer_c::NetInputBuffer_c( int iSock )
	: STORE( NET_MINIBUFFER_SIZE ), InputBuffer_c( m_pData, NET_MINIBUFFER_SIZE ), m_iSock( iSock )
{
	Resize( 0 );
}


bool NetInputBuffer_c::ReadFrom( int iLen, int iTimeout, bool bIntr, bool bAppend )
{
	int iTail = bAppend ? m_iLen : 0;

	m_bIntr = false;
	if ( iLen<=0 || iLen>g_iMaxPacketSize || m_iSock<0 )
		return false;

	int iOff = m_pCur - m_pBuf;
	Resize( m_iLen );
	Reserve( iTail + iLen );
	BYTE* pBuf = m_pData + iTail;
	m_pBuf = m_pData;
	m_pCur = bAppend ? m_pData + iOff : m_pData;
	int iGot = sphSockRead( m_iSock, pBuf, iLen, iTimeout, bIntr );
	if ( g_bGotSigterm )
	{
		sphLogDebug( "NetInputBuffer_c::ReadFrom: got SIGTERM, return false" );
		m_bError = true;
		m_bIntr = true;
		return false;
	}

	m_bError = ( iGot!=iLen );
	m_bIntr = m_bError && ( sphSockPeekErrno()==EINTR );
	m_iLen = m_bError ? 0 : iTail + iLen;
	return !m_bError;
}



// fixme! refactor to common flavour
void QueryStatus ( CSphVariant * v )
{
	char sBuf [ SPH_ADDRESS_SIZE ];
	char sListen [ 256 ];
	CSphVariant tListen;

	if ( !v )
	{
		snprintf ( sListen, sizeof ( sListen ), "127.0.0.1:%d:sphinx", SPHINXAPI_PORT );
		tListen = CSphVariant ( sListen );
		v = &tListen;
	}

	for ( ; v; v = v->m_pNext )
	{
		ListenerDesc_t tDesc = ParseListener ( v->cstr() );
		CHECK_LISTENER( tDesc );
		if ( tDesc.m_eProto!=Proto_e::SPHINX )
			continue;

		int iSock = -1;
#if !USE_WINDOWS
		if ( !tDesc.m_sUnix.IsEmpty() )
		{
			// UNIX connection
			struct sockaddr_un uaddr;

			size_t len = strlen ( tDesc.m_sUnix.cstr() );
			if ( len+1 > sizeof(uaddr.sun_path ) )
				sphFatal ( "UNIX socket path is too long (len=%d)", (int)len );

			memset ( &uaddr, 0, sizeof(uaddr) );
			uaddr.sun_family = AF_UNIX;
			memcpy ( uaddr.sun_path, tDesc.m_sUnix.cstr(), len+1 );

			iSock = socket ( AF_UNIX, SOCK_STREAM, 0 );
			if ( iSock<0 )
				sphFatal ( "failed to create UNIX socket: %s", sphSockError() );

			if ( connect ( iSock, (struct sockaddr*)&uaddr, sizeof(uaddr) )<0 )
			{
				sphWarning ( "failed to connect to unix://%s: %s\n", tDesc.m_sUnix.cstr(), sphSockError() );
				sphSockClose ( iSock );
				continue;
			}

		} else
#endif
		{
			// TCP connection
			struct sockaddr_in sin;
			memset ( &sin, 0, sizeof(sin) );
			sin.sin_family = AF_INET;
			sin.sin_addr.s_addr = ( tDesc.m_uIP==htonl ( INADDR_ANY ) )
				? htonl ( INADDR_LOOPBACK )
				: tDesc.m_uIP;
			sin.sin_port = htons ( (short)tDesc.m_iPort );

			iSock = socket ( AF_INET, SOCK_STREAM, 0 );
			if ( iSock<0 )
				sphFatal ( "failed to create TCP socket: %s", sphSockError() );

#ifdef TCP_NODELAY
			int iOn = 1;
			if ( setsockopt ( iSock, IPPROTO_TCP, TCP_NODELAY, (char*)&iOn, sizeof(iOn) ) )
				sphWarning ( "setsockopt() failed: %s", sphSockError() );
#endif

			if ( connect ( iSock, (struct sockaddr*)&sin, sizeof(sin) )<0 )
			{
				sphWarning ( "failed to connect to %s:%d: %s\n", sphFormatIP ( sBuf, sizeof(sBuf), tDesc.m_uIP ), tDesc.m_iPort, sphSockError() );
				sphSockClose ( iSock );
				continue;
			}
		}

		// send request
		NetOutputBuffer_c tOut ( iSock );
		tOut.SendDword ( SPHINX_CLIENT_VERSION );
		APICommand_t dStatus ( tOut, SEARCHD_COMMAND_STATUS, VER_COMMAND_STATUS );
		tOut.SendInt ( 1 ); // dummy body
		tOut.Flush ();

		// get reply
		NetInputBuffer_c tIn ( iSock );
		if ( !tIn.ReadFrom ( 12, 5 ) ) // magic_header_size=12, magic_timeout=5
			sphFatal ( "handshake failure (no response)" );

		DWORD uVer = tIn.GetDword();
		if ( uVer!=SPHINX_SEARCHD_PROTO && uVer!=0x01000000UL ) // workaround for all the revisions that sent it in host order...
			sphFatal ( "handshake failure (unexpected protocol version=%d)", uVer );

		if ( tIn.GetWord()!=SEARCHD_OK )
			sphFatal ( "status command failed" );

		if ( tIn.GetWord()!=VER_COMMAND_STATUS )
			sphFatal ( "status command version mismatch" );

		if ( !tIn.ReadFrom ( tIn.GetDword(), 5 ) ) // magic_timeout=5
			sphFatal ( "failed to read status reply" );

		fprintf ( stdout, "\nsearchd status\n--------------\n" );

		int iRows = tIn.GetDword();
		int iCols = tIn.GetDword();
		for ( int i=0; i<iRows && !tIn.GetError(); i++ )
		{
			for ( int j=0; j<iCols && !tIn.GetError(); j++ )
			{
				fprintf ( stdout, "%s", tIn.GetString().cstr() );
				fprintf ( stdout, ( j==0 ) ? ": " : " " );
			}
			fprintf ( stdout, "\n" );
		}

		// all done
		sphSockClose ( iSock );
		return;
	}
	sphFatal ( "failed to connect to daemon: please specify listen with sphinx protocol in your config file" );
}