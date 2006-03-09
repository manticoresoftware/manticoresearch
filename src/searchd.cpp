//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxutils.h"
#include "sphinxexcerpt.h"

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>

/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
	// Win-specific headers and calls
	#include <io.h>
	#include <winsock2.h>

	#define sphSockRecv(_sock,_buf,_len,_flags)		::recv(_sock,_buf,_len,_flags)
	#define sphSockSend(_sock,_buf,_len,_flags)		::send(_sock,_buf,_len,_flags)
	#define sphSockClose(_sock)						::closesocket(_sock)

#else
	// UNIX-specific headers and calls
	#include <unistd.h>
	#include <netinet/in.h>
	#include <sys/file.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/wait.h>
	#include <netdb.h>

	#define sphSockRecv(_sock,_buf,_len,_flags)		::recv(_sock,_buf,_len,_flags)
	#define sphSockSend(_sock,_buf,_len,_flags)		::send(_sock,_buf,_len,_flags)
	#define sphSockClose(_sock)						::close(_sock)
#endif

/////////////////////////////////////////////////////////////////////////////

enum ESphLogLevel
{
	LOG_FATAL	= 0,
	LOG_WARNING	= 1,
	LOG_INFO	= 2
};

static bool				g_bLogStdout	= true;
static int				g_iLogFile		= STDOUT_FILENO;
static ESphLogLevel		g_eLogLevel		= LOG_INFO;

static int				g_iReadTimeout	= 5;	// sec
static int				g_iChildren		= 0;
static int				g_iMaxChildren	= 0;
static int				g_iSocket		= 0;
static int				g_iQueryLogFile	= -1;
static int				g_iHUP			= 0;
static const char *		g_sPidFile		= NULL;

struct ServedIndex_t
{
	CSphIndex *			m_pIndex;
	CSphDict *			m_pDict;
	ISphTokenizer *		m_pTokenizer;
	CSphString *		m_pLockFile; 
	CSphString *		m_pIndexPath;

	ServedIndex_t ()
	{
		Reset ();
	}

	void Reset ()
	{
		m_pIndex	= NULL;
		m_pDict		= NULL;
		m_pTokenizer= NULL;
		m_pLockFile	= NULL;
		m_pIndexPath= NULL;
	}

	~ServedIndex_t ()
	{
		if ( m_pLockFile )
			unlink ( m_pLockFile->cstr() );
		SafeDelete ( m_pIndex );
		SafeDelete ( m_pDict );
		SafeDelete ( m_pTokenizer );
		SafeDelete ( m_pLockFile );
		SafeDelete ( m_pIndexPath );
	}
};

static SmallStringHash_T < ServedIndex_t >	g_hIndexes;

/////////////////////////////////////////////////////////////////////////////

/// known commands
enum SearchdCommand_e
{
	SEARCHD_COMMAND_SEARCH		= 0,
	SEARCHD_COMMAND_EXCERPT		= 1,

	SEARCHD_COMMAND_TOTAL
};


/// known command versions
enum
{
	VER_COMMAND_SEARCH		= 0x101,
	VER_COMMAND_EXCERPT		= 0x100
};


/// known status return codes
enum SearchdStatus_e
{
	SEARCHD_OK		= 0,	///< general success, command-specific reply follows
	SEARCHD_ERROR	= 1,	///< general failure, command-specific reply may follow
	SEARCHD_RETRY	= 2		///< temporaty failure, client should retry later
};

/////////////////////////////////////////////////////////////////////////////
// MACHINE-DEPENDENT STUFF
/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS

// Windows hacks
#undef EINTR
#define LOCK_EX			0
#define LOCK_UN			1
#define STDIN_FILENO	fileno(stdin)
#define STDOUT_FILENO	fileno(stdout)
#define STDERR_FILENO	fileno(stderr)
#define ETIMEDOUT		WSAETIMEDOUT
#define EWOULDBLOCK		WSAEWOULDBLOCK
#define EINPROGRESS		WSAEINPROGRESS
#define EINTR			WSAEINTR
#define socklen_t		int
#define vsnprintf		_vsnprintf


void flock ( int, int )
{
}


void sleep ( int iSec )
{
	Sleep ( iSec*1000 );
}


void ctime_r ( time_t * tNow, char * sBuf )
{
	strcpy ( sBuf, ctime(tNow) );
}


int getpid ()
{
	return 0;
}

#endif // USE_WINDOWS

/////////////////////////////////////////////////////////////////////////////
// MISC
/////////////////////////////////////////////////////////////////////////////

void Shutdown ()
{
	if ( g_iSocket )
		sphSockClose ( g_iSocket );
	if ( g_sPidFile )
		unlink ( g_sPidFile );
	g_hIndexes.Reset ();
}

/////////////////////////////////////////////////////////////////////////////
// LOGGING
/////////////////////////////////////////////////////////////////////////////

void sphLog ( ESphLogLevel eLevel, const char * sFmt, va_list ap )
{
	if ( eLevel>g_eLogLevel || g_iLogFile<0 )
		return;

	time_t tNow;
	char sTimeBuf[128];
	time ( &tNow );
	ctime_r ( &tNow, sTimeBuf );
	sTimeBuf [ strlen(sTimeBuf)-1 ] = '\0';

	const char * sBanner = "";
	if ( eLevel==LOG_FATAL )	sBanner = "FATAL: ";
	if ( eLevel==LOG_WARNING )	sBanner = "WARNING: ";

	char sBuf [ 1024 ];
	if ( !isatty ( g_iLogFile ) )
		snprintf ( sBuf, sizeof(sBuf)-1, "[%s] [%5d] %s", sTimeBuf, getpid(), sBanner );
	else
		strcpy ( sBuf, sBanner );
	int iLen = strlen(sBuf);

	vsnprintf ( sBuf+iLen, sizeof(sBuf)-iLen-1, sFmt, ap );
	strncat ( sBuf, "\n", sizeof(sBuf) );

	flock ( g_iLogFile, LOCK_EX );
	lseek ( g_iLogFile, 0, SEEK_END );
	write ( g_iLogFile, sBuf, strlen(sBuf) );
	flock ( g_iLogFile, LOCK_UN );

	if ( g_bLogStdout && g_iLogFile!=STDOUT_FILENO )
	{
		write ( STDOUT_FILENO, sBuf, strlen(sBuf) );
	}
}


void sphFatal ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	sphLog ( LOG_FATAL, sFmt, ap );
	va_end ( ap );
	Shutdown ();
	exit ( 1 );
}


void sphWarning ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	sphLog ( LOG_WARNING, sFmt, ap );
	va_end ( ap );
}


void sphInfo ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	sphLog ( LOG_INFO, sFmt, ap );
	va_end ( ap );
}

/////////////////////////////////////////////////////////////////////////////
// NETWORK STUFF
/////////////////////////////////////////////////////////////////////////////

const int		NET_MAX_REQ_LEN			= 131072;
const int		NET_MAX_STR_LEN			= NET_MAX_REQ_LEN;
const int		SEARCHD_MAX_REQ_GROUPS	= 4096;

const char * sphSockError ( int iErr=0 )
{
	#if USE_WINDOWS
		if ( iErr==0 )
			iErr = WSAGetLastError ();

		static char sBuf [ 256 ];
		_snprintf ( sBuf, sizeof(sBuf), "WSA error %d", iErr );
		return sBuf;
	#else
		return strerror ( errno );
	#endif
}


int sphSockGetErrno ()
{
	#if USE_WINDOWS
		return WSAGetLastError();
	#else
		return errno;
	#endif
}


void sphSockSetErrno ( int iErr )
{
	#if USE_WINDOWS
		WSASetLastError ( iErr );
	#else
		errno = iErr;
	#endif
}


int sphCreateServerSocket ( int port )
{
	int sock;
	static struct sockaddr_in iaddr;

	iaddr.sin_family = AF_INET;
	iaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	iaddr.sin_port = htons((short)port);

	sphInfo ( "creating a server socket on port %d", port );
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		sphFatal ( "unable to create server socket on port %d: %s", port, sphSockError() );

	int iOn = 1;
	if ( setsockopt ( sock, SOL_SOCKET,  SO_REUSEADDR, (char*)&iOn, sizeof(iOn) ) )
		sphFatal ( "setsockopt() failed: %s", sphSockError() );

	int iTries = 12;
	int iRes;
	do
	{
		iRes = bind ( sock, (struct sockaddr *)&iaddr, sizeof(iaddr) );
		if ( iRes==0 )
			break;

		sphInfo ( "failed to bind on port %d, retrying...", port );
		sleep ( 15 );
	} while ( --iTries>0 );
	if ( iRes )
		sphFatal ( "failed to bind on port %d" );

	return sock;
}


int sphSetSockNB ( int iSock )
{
	#if USE_WINDOWS
		u_long uMode = 1;
		return ioctlsocket ( iSock, FIONBIO, &uMode );
	#else
		return fcntl ( iSock, F_SETFL, O_NONBLOCK );
	#endif
}

/////////////////////////////////////////////////////////////////////////////

STATIC_SIZE_ASSERT ( DWORD, 4 );


int sphSockRead ( int iSock, void * buf, int iLen )
{
	assert ( iLen>0 );

	#if USE_WINDOWS
	#pragma warning(disable:4127) // conditional expr is const
	#pragma warning(disable:4389) // signed/unsigned mismatch
	#endif // USE_WINDOWS

	int iTimeout = 1000*Max ( 1, g_iReadTimeout ); // ms to wait total
	int iLeftMs = iTimeout; // ms to wait left
	int iLeftBytes = iLen; // bytes to read left
	float tmStart = sphLongTimer ();
	char * pBuf = (char*) buf;

	int iRes = -1, iErr = 0;
	while ( iLeftBytes>0 && iLeftMs>0 )
	{
		fd_set fdRead;
		FD_ZERO ( &fdRead );
		FD_SET ( iSock, &fdRead );

		fd_set fdExcept;
		FD_ZERO ( &fdExcept );
		FD_SET ( iSock, &fdExcept );

		struct timeval tv;
		tv.tv_sec = iLeftMs / 1000;
		tv.tv_usec = iLeftMs % 1000;

		iRes = ::select ( iSock+1, &fdRead, NULL, &fdExcept, &tv );

		// if there was EINTR, retry
		if ( iRes==-1 )
		{
			iErr = sphSockGetErrno();
			if ( iErr==EINTR )
			{
				iLeftMs = iTimeout - (int)( 1000.0f*( sphLongTimer() - tmStart ) );
				continue;
			}
			sphSockSetErrno ( iErr );
			return -1;
		}

		// if there was a timeout, report it as an error
		if ( iRes==0 )
		{
			sphSockSetErrno ( ETIMEDOUT );
			return -1;
		}

		// try to recv next chunk
		iRes = sphSockRecv ( iSock, pBuf, iLeftBytes, 0 );

		// if there was EINTR, retry
		if ( iRes==-1 )
		{
			iErr = sphSockGetErrno();
			if ( iErr==EINTR )
			{
				iLeftMs -= (int)( 1000.0f*( sphLongTimer() - tmStart ) );
				continue;
			}
			sphSockSetErrno ( iErr );
			return -1;
		}

		// update
		pBuf += iRes;
		iLeftBytes -= iRes;
		iLeftMs = iTimeout - (int)( 1000.0f*( sphLongTimer() - tmStart ) );
	}

	// if there was a timeout, report it as an error
	if ( iLeftBytes!=0 )
	{
		sphSockSetErrno ( ETIMEDOUT );
		return -1;
	}

	return iLen;

	#if USE_WINDOWS
	#pragma warning(default:4127) // conditional expr is const
	#pragma warning(default:4389) // signed/unsigned mismatch
	#endif // USE_WINDOWS
}

/////////////////////////////////////////////////////////////////////////////
// NETWORK BUFFERS
/////////////////////////////////////////////////////////////////////////////

/// fixed-memory response buffer
/// tracks usage, and flushes to network when necessary
class NetOutputBuffer_c
{
public:
				NetOutputBuffer_c ( int iSock );
	bool		SendInt ( int iValue )		{ return SendT<int> ( iValue ); }
	bool		SendDword ( DWORD iValue )	{ return SendT<DWORD> ( iValue ); }
	bool		SendWord ( WORD iValue )	{ return SendT<WORD> ( iValue ); }
	bool		SendString ( const char * sStr );
	bool		SendBytes ( const void * pBuf, int iLen );
	bool		Flush ();
	bool		GetError () { return m_bError; }
	int			GetSentCount () { return m_iSent; }

protected:
	BYTE		m_dBuffer[8192];	///< my buffer
	BYTE *		m_pBuffer;			///< my current buffer position
	int			m_iSock;			///< my socket
	bool		m_bError;			///< if there were any write errors
	int			m_iSent;

protected:
	bool		SetError ( bool bValue );	///< set error flag
	bool		FlushIf ( int iToAdd );		///< flush if there's not enough free space to add iToAdd bytes

	template < typename T > bool	SendT ( T tValue );
};


/// generic request buffer
class InputBuffer_c
{
public:
					InputBuffer_c ();
	virtual			~InputBuffer_c () {}
	int				GetInt () { return GetT<int> (); }
	WORD			GetWord () { return GetT<WORD> (); }
	DWORD			GetDword () { return GetT<DWORD> (); }
	BYTE			GetByte () { return GetT<BYTE> (); }
	CSphString		GetString ();
	bool			GetBytes ( void * pBuf, int iLen );
	int				GetInts ( int ** pBuffer, int iMax, const char * sErrorTemplate );
	bool			GetError () { return m_bError; }

	virtual void	SendErrorReply ( const char *, ... ) = 0;

protected:
	bool			m_bError;
	BYTE			m_dBuf [ NET_MAX_REQ_LEN ];
	int				m_iLen;
	BYTE *			m_pCur;

protected:
	void						SetError ( bool bError ) { m_bError = bError; }

	template < typename T > T	GetT ();
};


/// simple memory request buffer
class MemInputBuffer_c : public InputBuffer_c
{
public:
					MemInputBuffer_c ( const char * sFrom, int iLen );
	virtual void	SendErrorReply ( const char *, ... ) {}
};


/// simple network request buffer
class NetInputBuffer_c : public InputBuffer_c
{
public:
					NetInputBuffer_c ( int iSock );
	bool			ReadFrom ( int iLen );
	virtual void	SendErrorReply ( const char *, ... );

protected:
	int				m_iSock;
};

/////////////////////////////////////////////////////////////////////////////

NetOutputBuffer_c::NetOutputBuffer_c ( int iSock )
	: m_pBuffer ( m_dBuffer )
	, m_iSock ( iSock )
	, m_bError ( false )
	, m_iSent ( 0 )
{
	assert ( m_iSock>0 );
}


template < typename T > bool NetOutputBuffer_c::SendT ( T tValue )
{
	if ( m_bError )
		return false;

	FlushIf ( sizeof(T) );

	*(T*)m_pBuffer = tValue;
	m_pBuffer += sizeof(T);
	assert ( m_pBuffer<m_dBuffer+sizeof(m_dBuffer) );
	return true;
}


bool NetOutputBuffer_c::SendString ( const char * sStr )
{
	if ( m_bError )
		return false;

	FlushIf ( sizeof(DWORD) );

	int iLen = strlen(sStr);
	*(int*)m_pBuffer = iLen;
	m_pBuffer += sizeof(int);

	return SendBytes ( sStr, iLen );
}


bool NetOutputBuffer_c::SendBytes ( const void * pBuf, int iLen )
{
	BYTE * pMy = (BYTE*)pBuf;
	while ( iLen>0 && !m_bError )
	{
		int iLeft = sizeof(m_dBuffer) - ( m_pBuffer - m_dBuffer );
		if ( iLen<=iLeft )
		{
			memcpy ( m_pBuffer, pMy, iLen );
			m_pBuffer += iLen;
			break;
		}

		memcpy ( m_pBuffer, pMy, iLeft );
		m_pBuffer += iLeft;
		Flush ();

		pMy += iLeft;
		iLen -= iLeft;
	}
	return !m_bError;
}


bool NetOutputBuffer_c::Flush ()
{
	if ( m_bError )
		return false;

	int iLen = m_pBuffer-m_dBuffer;
	if ( iLen==0 )
		return true;

	assert ( iLen>0 );
	assert ( iLen<=(int)sizeof(m_dBuffer) );

	int iRes = sphSockSend ( m_iSock, (char*)&m_dBuffer[0], iLen, 0 );
	m_bError = ( iRes!=iLen );

	m_iSent += iLen;
	m_pBuffer = m_dBuffer;
	return !m_bError;
}


bool NetOutputBuffer_c::FlushIf ( int iToAdd )
{
	if ( m_pBuffer+iToAdd >= m_dBuffer+sizeof(m_dBuffer) )
		return Flush ();

	return !m_bError;
}

/////////////////////////////////////////////////////////////////////////////

InputBuffer_c::InputBuffer_c ()
	: m_bError ( true )
	, m_iLen ( 0 )
	, m_pCur ( m_dBuf )
{
}


template < typename T > T InputBuffer_c::GetT ()
{
	if ( m_bError || ( m_pCur+sizeof(T) > m_dBuf+m_iLen ) )
	{
		SetError ( true );
		return 0;
	}

	T iRes = *(T*)m_pCur;
	m_pCur += sizeof(T);
	return iRes;
}


CSphString InputBuffer_c::GetString ()
{
	CSphString sRes;

	int iLen = GetInt ();
	if ( m_bError || iLen<0 || iLen>NET_MAX_STR_LEN || ( m_pCur+iLen > m_dBuf+m_iLen ) )
	{
		SetError ( true );
		return sRes;
	}

	sRes.SetBinary ( (char*)m_pCur, iLen );
	m_pCur += iLen;
	return sRes;
}


bool InputBuffer_c::GetBytes ( void * pBuf, int iLen )
{
	assert ( pBuf );
	assert ( iLen>0 && iLen<=NET_MAX_REQ_LEN );

	if ( m_bError || ( m_pCur+iLen > m_dBuf+m_iLen ) )
	{
		SetError ( true );
		return false;
	}

	memcpy ( pBuf, m_pCur, iLen );
	m_pCur += iLen;
	return true;;
}


int InputBuffer_c::GetInts ( int ** ppBuffer, int iMax, const char * sErrorTemplate )
{
	assert ( ppBuffer );
	assert ( !(*ppBuffer) );

	int iCount = GetInt ();
	if ( iCount<0 || iCount>iMax )
	{
		SendErrorReply ( sErrorTemplate, iCount, iMax );
		SetError ( true );
		return -1;
	}
	if ( iCount )
	{
		(*ppBuffer) = new int [ iCount ];
		if ( !GetBytes ( (*ppBuffer), sizeof(int)*iCount ) )
		{
			SafeDeleteArray ( (*ppBuffer) );
			return -1;
		}
	}
	return iCount;
}

/////////////////////////////////////////////////////////////////////////////

NetInputBuffer_c::NetInputBuffer_c ( int iSock )
	: m_iSock ( iSock )
{
}


bool NetInputBuffer_c::ReadFrom ( int iLen )
{
	assert ( iLen>0 );
	assert ( iLen<=NET_MAX_REQ_LEN );
	assert ( m_iSock>0 );

	m_pCur = m_dBuf;

	int iGot = sphSockRead ( m_iSock, &m_dBuf[0], iLen );
	if ( iGot!=iLen )
	{
		m_bError = true;
		m_iLen = 0;
		return false;
	}

	m_bError = false;
	m_iLen = iLen;
	return true;
}


void NetInputBuffer_c::SendErrorReply ( const char * sTemplate, ... )
{
	char dBuf [ 2048 ];

	const int iHeaderLen = 12;
	const int iMaxStrLen = sizeof(dBuf) - iHeaderLen - 1;

	// fill header
	DWORD * pBuf = (DWORD*)&dBuf[0];
	pBuf[0] = SEARCHD_ERROR;
	pBuf[1] = 0; // reply length, to be filled later
	pBuf[2] = 0; // error string length, to be filled later

	// fill error string
	char * sBuf = dBuf + iHeaderLen;

	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf, iMaxStrLen, sTemplate, ap );
	va_end ( ap );

	sBuf[iMaxStrLen] = '\0';
	int iStrLen = strlen(sBuf);

	// fixup lengths
	pBuf[1] = iHeaderLen+iStrLen;
	pBuf[2] = iStrLen;

	// send!
	sphSockSend ( m_iSock, dBuf, iHeaderLen+iStrLen, 0 );
}

/////////////////////////////////////////////////////////////////////////////

MemInputBuffer_c::MemInputBuffer_c ( const char * sFrom, int iLen )
{
	if ( iLen<0 || iLen>NET_MAX_STR_LEN )
	{
		m_bError = true;
		return;
	}

	memcpy ( m_dBuf, sFrom, iLen );
	m_iLen = iLen;
	m_pCur = m_dBuf;
	m_bError = false;
}

/////////////////////////////////////////////////////////////////////////////
// DISTRIBUTED QUERIES
/////////////////////////////////////////////////////////////////////////////

/// remote agent state
enum AgentState_e
{
	AGENT_UNUSED,				///< agent is unused for this request
	AGENT_CONNECT,				///< connecting to agent
	AGENT_HELLO,				///< waiting for "VER x" hello
	AGENT_QUERY,				///< query sent, wating for reply
	AGENT_REPLY					///< reading reply
};

/// remote agent host/port
struct Agent_t
{
public:
	CSphString		m_sHost;		///< remote searchd host
	int				m_iPort;		///< remote searchd port, 0 if local
	CSphString		m_sIndexes;		///< remote index names to query

	int				m_iSock;		///< socket number, -1 if not connected
	AgentState_e	m_eState;		///< current state

	CSphQueryResult	m_tRes;			///< query result

	int				m_iReplySize;	///< how many reply bytes are there
	int				m_iReplyRead;	///< how many reply bytes are alredy received
	char *			m_pReplyBuf;	///< reply buffer

protected:
	int				m_iAddrType;
	DWORD			m_uAddr;

public:
	Agent_t ()
		: m_iPort ( -1 )
		, m_iSock ( -1 )
		, m_eState ( AGENT_UNUSED )
		, m_iReplySize ( 0 )
		, m_iReplyRead ( 0 )
		, m_pReplyBuf ( NULL )
		, m_uAddr ( 0 )
	{
	}

	~Agent_t ()
	{
		SafeDeleteArray ( m_pReplyBuf );
		Close ();
	}

	void Close ()
	{
		if ( m_iSock>0 )
		{
			sphSockClose ( m_iSock );
			m_iSock = -1;
			m_eState = AGENT_UNUSED;

			SafeDeleteArray ( m_pReplyBuf );
		}
	}

	void SetAddr ( int iAddrType, int iAddrLen, const char * pAddr )
	{
		assert ( pAddr );
		assert ( iAddrLen==sizeof(m_uAddr) );

		m_iAddrType = iAddrType;
		memcpy ( &m_uAddr, pAddr, iAddrLen );
	}

	int GetAddrType () const
	{
		return m_iAddrType;
	}

	int GetAddrLen () const
	{
		return sizeof(m_uAddr);
	}

	const char * GetAddr () const
	{
		return (const char *)&m_uAddr;
	}
};

/// distributed index
struct DistributedIndex_t
{
	CSphVector<Agent_t,16>	m_dAgents;					///< remote agents
	CSphVector<CSphString>	m_dLocal;					///< local indexes
	int						m_iAgentConnectTimeout;		///< in msec
	int						m_iAgentQueryTimeout;		///< in msec

	DistributedIndex_t ()
		: m_iAgentConnectTimeout ( 1000 )
		, m_iAgentQueryTimeout ( 3000 )
	{
	}
};

static SmallStringHash_T < DistributedIndex_t >		g_hDistIndexes;

/////////////////////////////////////////////////////////////////////////////

void ConnectToRemoteAgent ( Agent_t * pAgent )
{
	assert ( pAgent );
	assert ( pAgent->m_iPort>0 );
	assert ( pAgent->GetAddr() );
	assert ( pAgent->m_iSock<0 );

	pAgent->m_eState = AGENT_UNUSED;

	struct sockaddr_in sa;

	memset ( &sa, 0, sizeof(sa) );
	memcpy ( &sa.sin_addr, pAgent->GetAddr(), pAgent->GetAddrLen() );
	sa.sin_family = (short)pAgent->GetAddrType();
	sa.sin_port = htons ( (unsigned short)pAgent->m_iPort );

	pAgent->m_iSock = socket ( pAgent->GetAddrType(), SOCK_STREAM, 0 );
	if ( pAgent->m_iSock<0 )
	{
		sphWarning ( "agent '%s:%d': socket() failed", pAgent->m_sHost.cstr(), pAgent->m_iPort );
		return;
	}

	if ( sphSetSockNB ( pAgent->m_iSock )<0 )
	{
		sphWarning ( "agent '%s:%d': sphSetSockNB() failed", pAgent->m_sHost.cstr(), pAgent->m_iPort );
		return;
	}

	if ( connect ( pAgent->m_iSock, (struct sockaddr*)&sa, sizeof(sa) )<0 )
	{
		int iErr = sphSockGetErrno();
		if ( iErr!=EINPROGRESS && iErr!=EINTR && iErr!=EWOULDBLOCK ) // check for EWOULDBLOCK is for winsock only
		{
			pAgent->Close ();
			sphWarning ( "agent '%s:%d': connect() failed: %s", pAgent->m_sHost.cstr(), pAgent->m_iPort,
				sphSockError(iErr) );
			return;
		} else
		{
			// connection in progress
			pAgent->m_eState = AGENT_CONNECT;
		}
	} else
	{
		// socket connected, ready to read hello message
		pAgent->m_eState = AGENT_HELLO;
	}
}


#if USE_WINDOWS
#pragma warning(disable:4127) // conditional expr is const
#pragma warning(disable:4389) // signed/unsigned mismatch
#endif // USE_WINDOWS

int QueryRemoteAgents ( const char * sIndexName, DistributedIndex_t & tDist, const CSphQuery & tQuery, int iMode )
{
	int iTimeout = tDist.m_iAgentConnectTimeout;
	int iAgents = 0;
	assert ( iTimeout>=0 );

	int iPassed = 0;
	float tmStart = sphLongTimer ();
	while ( iPassed<=iTimeout )
	{
		fd_set fdsRead, fdsWrite;
		FD_ZERO ( &fdsRead );
		FD_ZERO ( &fdsWrite );

		int iMax = 0;
		bool bDone = true;
		ARRAY_FOREACH ( i, tDist.m_dAgents )
		{
			const Agent_t & tAgent = tDist.m_dAgents[i];
			if ( tAgent.m_eState==AGENT_CONNECT || tAgent.m_eState==AGENT_HELLO )
			{
				assert ( tAgent.m_iPort>0 );
				assert ( tAgent.m_iSock>0 );

				FD_SET ( tAgent.m_iSock, ( tAgent.m_eState==AGENT_CONNECT ) ? &fdsWrite : &fdsRead );
				iMax = Max ( iMax, tAgent.m_iSock );
				bDone = false;
			}
		}
		if ( bDone )
			break;

		iPassed = int ( 1000.0f*( sphLongTimer() - tmStart ) );
		int iToWait = Max ( iTimeout-iPassed, 0 );

		struct timeval tvTimeout;
		tvTimeout.tv_sec = iToWait / 1000; // full seconds
		tvTimeout.tv_usec = ( iToWait % 1000 ) * 1000; // remainder is msec, so *1000 for usec

		// FIXME! check exceptfds for connect() failure as well, so that actively refused
		// connections would not stall for a full timeout
		if ( select ( 1+iMax, &fdsRead, &fdsWrite, NULL, &tvTimeout )<=0 )
			continue;

		ARRAY_FOREACH ( i, tDist.m_dAgents )
		{
			Agent_t & tAgent = tDist.m_dAgents[i];

			// check if connection completed
			if ( tAgent.m_eState==AGENT_CONNECT && FD_ISSET ( tAgent.m_iSock, &fdsWrite ) )
			{
				int iErr = 0;
				socklen_t iErrLen = sizeof(iErr);
				getsockopt ( tAgent.m_iSock, SOL_SOCKET, SO_ERROR, (char*)&iErr, &iErrLen );
				if ( iErr )
				{
					// connect() failure
					sphWarning ( "index '%s': agent '%s:%d': connect() failed (err=%d)",
						sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort, iErr );
					tAgent.Close ();
				} else
				{
					// connect() success
					tAgent.m_eState = AGENT_HELLO;
				}
				continue;
			}

			// check if hello was received
			if ( tAgent.m_eState==AGENT_HELLO && FD_ISSET ( tAgent.m_iSock, &fdsRead ) )
			{
				// read reply
				int iRemoteVer;
				int iRes = sphSockRecv ( tAgent.m_iSock, (char*)&iRemoteVer, sizeof(iRemoteVer), 0 );
				if ( iRes!=sizeof(iRemoteVer) || iRemoteVer<=0 )
				{
					sphWarning ( "index '%s': agent '%s:%d': expected protocol v.%d, got v.%d",
						sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
						SPHINX_SEARCHD_PROTO, iRemoteVer );
					tAgent.Close ();
					continue;
				}

				// do query!
				int iQueryLen = strlen ( tQuery.m_sQuery.cstr() );
				int iIndexesLen = strlen ( tAgent.m_sIndexes.cstr() );
				int iReqSize = 60 + 4*tQuery.m_iGroups + iQueryLen + 4*tQuery.m_iWeights + iIndexesLen;

				NetOutputBuffer_c tOut ( tAgent.m_iSock );

				// say hello
				tOut.SendDword ( SPHINX_SEARCHD_PROTO );

				// request header
				tOut.SendWord ( SEARCHD_COMMAND_SEARCH ); // command id
				tOut.SendWord ( VER_COMMAND_SEARCH ); // command version
				tOut.SendInt ( iReqSize-12 ); // request body length

				// request v1
				tOut.SendInt ( 0 ); // offset is 0
				tOut.SendInt ( CSphQueryResult::MAX_MATCHES ); // limit is MAX_MATCHES
				tOut.SendInt ( iMode ); // match mode
				tOut.SendInt ( tQuery.m_eSort ); // sort mode
				tOut.SendInt ( tQuery.m_iGroups );
				tOut.SendBytes ( tQuery.m_pGroups, sizeof(DWORD)*tQuery.m_iGroups ); // groups
				tOut.SendString ( tQuery.m_sQuery.cstr() ); // query
				tOut.SendInt ( tQuery.m_iWeights );
				tOut.SendBytes ( tQuery.m_pWeights, sizeof(DWORD)*tQuery.m_iWeights ); // weights
				tOut.SendString ( tAgent.m_sIndexes.cstr() ); // indexes
				tOut.SendInt ( tQuery.m_iMinID ); // id/ts ranges
				tOut.SendInt ( tQuery.m_iMaxID );
				tOut.SendInt ( tQuery.m_iMinTS );
				tOut.SendInt ( tQuery.m_iMaxTS );
				tOut.Flush ();

				// FIXME! !COMMIT handle flush failure
				tAgent.m_eState = AGENT_QUERY;
				iAgents++;
			}
		}
	}

	ARRAY_FOREACH ( i, tDist.m_dAgents )
	{
		// check if connection timed out
		Agent_t & tAgent = tDist.m_dAgents[i];
		if ( tAgent.m_eState!=AGENT_QUERY && tAgent.m_eState!=AGENT_UNUSED )
		{
			tAgent.Close ();
			sphWarning ( "index '%s': agent '%s:%d': %s() timed out",
				sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
				tAgent.m_eState==AGENT_HELLO ? "read" : "connect" );
		}
	}

	return iAgents;
}


int WaitForRemoteAgents ( const char * sIndexName, DistributedIndex_t & tDist, CSphQueryResult * pRes, int iTimeout )
{
	assert ( pRes );
	assert ( iTimeout>=0 );

	int iAgents = 0;
	int iPassed = 0;
	float tmStart = sphLongTimer ();
	while ( iPassed<=iTimeout )
	{
		fd_set fdsRead;
		FD_ZERO ( &fdsRead );

		int iMax = 0;
		bool bDone = true;
		ARRAY_FOREACH ( iAgent, tDist.m_dAgents )
		{
			Agent_t & tAgent = tDist.m_dAgents[iAgent];
			if ( tAgent.m_eState==AGENT_QUERY || tAgent.m_eState==AGENT_REPLY )
			{
				assert ( tAgent.m_iPort>0 );
				assert ( tAgent.m_iSock>0 );

				FD_SET ( tAgent.m_iSock, &fdsRead );
				iMax = Max ( iMax, tAgent.m_iSock );
				bDone = false;
			}
		}
		if ( bDone )
			break;

		iPassed = int ( 1000.0f*( sphLongTimer() - tmStart ) );
		int iToWait = Max ( iTimeout-iPassed, 0 );

		struct timeval tvTimeout;
		tvTimeout.tv_sec = iToWait / 1000; // full seconds
		tvTimeout.tv_usec = ( iToWait % 1000 ) * 1000; // remainder is msec, so *1000 for usec

		if ( select ( 1+iMax, &fdsRead, NULL, NULL, &tvTimeout )<=0 )
			continue;

		ARRAY_FOREACH ( iAgent, tDist.m_dAgents )
		{
			Agent_t & tAgent = tDist.m_dAgents[iAgent];
			if (!( tAgent.m_eState==AGENT_QUERY || tAgent.m_eState==AGENT_REPLY ))
				continue;
			if ( !FD_ISSET ( tAgent.m_iSock, &fdsRead ) )
				continue;

			// if there was no reply yet, read reply header
			bool bFailure = true;
			do
			{
				if ( tAgent.m_eState==AGENT_QUERY )
				{
					// try to read
					struct
					{
						WORD	m_iStatus;
						WORD	m_iVer;
						int		m_iLength;
					} tReplyHeader;
					STATIC_SIZE_ASSERT ( tReplyHeader, 8 );

					if ( sphSockRecv ( tAgent.m_iSock, (char*)&tReplyHeader, sizeof(tReplyHeader), 0 )!=sizeof(tReplyHeader) )
					{
						// bail out if failed
						sphWarning ( "index '%s': agent '%s:%d': failed to receive reply header",
							sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort );
						break;
					}

					// check the status
					if ( tReplyHeader.m_iStatus!=SEARCHD_OK || tReplyHeader.m_iLength<=0 )
					{
						if ( tReplyHeader.m_iStatus!=SEARCHD_OK && tReplyHeader.m_iLength>0 )
						{
							char sAgentError[1024];
							int iToRead = Min ( (int)sizeof(sAgentError)-1, tReplyHeader.m_iLength );
							int iRes = sphSockRecv ( tAgent.m_iSock, sAgentError, iToRead, 0 );
							sAgentError [ Max ( iRes, 0 ) ] = '\0';

							sphWarning ( "index '%s': agent '%s:%d': remote error (status=%d, error=%s)",
								sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
								tReplyHeader.m_iStatus, sAgentError+4 );

						} else
						{
							sphWarning ( "index '%s': agent '%s:%d': ill-formed reply length (status=%d, len=%d)",
								sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
								tReplyHeader.m_iStatus, tReplyHeader.m_iLength );
						}
						break;
					}

					// header received, switch the status
					assert ( tAgent.m_pReplyBuf==NULL );
					tAgent.m_eState = AGENT_REPLY;
					tAgent.m_pReplyBuf = new char [ tReplyHeader.m_iLength ];
					tAgent.m_iReplySize = tReplyHeader.m_iLength;
					tAgent.m_iReplyRead = 0;

					if ( !tAgent.m_pReplyBuf )
					{
						// bail out if failed
						sphWarning ( "index '%s': agent '%s:%d': failed to alloc %d bytes for reply buffer",
							sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
							tAgent.m_iReplySize );
						break;
					}
				}

				// if we are reading reply, read another chunk
				if ( tAgent.m_eState==AGENT_REPLY )
				{
					// do read
					assert ( tAgent.m_iReplyRead<tAgent.m_iReplySize );
					int iRes = sphSockRecv ( tAgent.m_iSock, tAgent.m_pReplyBuf+tAgent.m_iReplyRead,
						tAgent.m_iReplySize-tAgent.m_iReplyRead, 0 );

					// bail out if read failed
					if ( iRes<0 )
					{
						sphWarning ( "index '%s': agent '%s:%d': failed to receive reply body: %s",
							sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
							sphSockError() );
						break;
					}

					assert ( iRes>0 );
					assert ( tAgent.m_iReplyRead+iRes<=tAgent.m_iReplySize );
					tAgent.m_iReplyRead += iRes;
				}

				// if reply was fully received, parse it
				if ( tAgent.m_eState==AGENT_REPLY && tAgent.m_iReplyRead==tAgent.m_iReplySize )
				{
					MemInputBuffer_c tReq ( tAgent.m_pReplyBuf, tAgent.m_iReplySize );

					int iMatches = tReq.GetInt ();
					if ( iMatches<=0 || iMatches>CSphQueryResult::MAX_MATCHES )
					{
						sphWarning ( "index '%s': agent '%s:%d': invalid match count received (count=%d)",
							sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
							iMatches );
						break;
					}

					assert ( !tAgent.m_tRes.m_dMatches.GetLength() );
					tAgent.m_tRes.m_dMatches.Resize ( iMatches );
					tReq.GetBytes ( &tAgent.m_tRes.m_dMatches[0], iMatches*sizeof(CSphMatch) );

					// read totals (retrieved count, total count, query time, word count)
					int iRetrieved = tReq.GetInt ();
					tAgent.m_tRes.m_iTotalMatches = tReq.GetInt ();
					tAgent.m_tRes.m_iQueryTime = tReq.GetInt ();
					tAgent.m_tRes.m_iNumWords = tReq.GetInt ();
					if ( iRetrieved!=iMatches )
					{
						sphWarning ( "index '%s': agent '%s:%d': expected %d retrieved documents, got %d",
							sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
							iMatches, iRetrieved );
						break;
					}

					bool bSetWords = false;
					if ( pRes->m_iNumWords && tAgent.m_tRes.m_iNumWords!=pRes->m_iNumWords )
					{
						sphWarning ( "index '%s': agent '%s:%d': expected %d query words, got %d",
							sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
							pRes->m_iNumWords, tAgent.m_tRes.m_iNumWords );
						break;
					}

					if ( !pRes->m_iNumWords )
					{
						pRes->m_iNumWords = tAgent.m_tRes.m_iNumWords;
						bSetWords = true;
					}

					// read words
					int i;
					for ( i=0; i<pRes->m_iNumWords; i++ )
					{
						CSphString sWord = tReq.GetString ();

						// set it in result if not yet, or check if already
						if ( bSetWords )
						{
							pRes->m_tWordStats[i].m_sWord = sWord;
						} else
						{
							if ( pRes->m_tWordStats[i].m_sWord!=sWord )
								break;
						}

						pRes->m_tWordStats[i].m_iDocs += tReq.GetInt (); // update docs count
						pRes->m_tWordStats[i].m_iHits += tReq.GetInt (); // update hits count
					}

					if ( i!=pRes->m_iNumWords || tReq.GetError() )
					{
						sphWarning ( "index '%s': agent '%s:%d': expected %d retrieved documents, got %d",
							sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
							iMatches, iRetrieved );
						break;
					}

					// all done
					iAgents++;
					tAgent.Close ();
				}

				bFailure = false;
			} while ( false );

			if ( bFailure )
			{
				tAgent.Close ();
				tAgent.m_tRes.m_dMatches.Reset ();
			}
		}
	}

	// close timed-out agents
	ARRAY_FOREACH ( iAgent, tDist.m_dAgents )
	{
		Agent_t & tAgent = tDist.m_dAgents[iAgent];
		if ( tAgent.m_eState==AGENT_QUERY )
		{
			assert ( !tAgent.m_tRes.m_dMatches.GetLength() );
			tAgent.Close ();

			sphWarning ( "index '%s': agent '%s:%d': query timed out",
				sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort );
		}
	}

	return iAgents;
}

#if USE_WINDOWS
#pragma warning(default:4127) // conditional expr is const
#pragma warning(default:4389) // signed/unsigned mismatch
#endif // USE_WINDOWS

/////////////////////////////////////////////////////////////////////////////
// SIGNALS
/////////////////////////////////////////////////////////////////////////////

#if !USE_WINDOWS
void sigchld ( int )
{
	signal ( SIGCHLD, sigchld );
	while ( waitpid ( 0, (int *)0, WNOHANG | WUNTRACED ) > 0 )
		g_iChildren--;
}


void sigterm ( int )
{
	sphInfo ( "caught SIGTERM, shutting down" );
	Shutdown ();
	exit ( 0 );
}


void sighup ( int )
{
	sphInfo ( "rotating indices: caught SIGHUP, waiting for children to exit" );
	g_iHUP = 1;
}
#endif // !USE_WINDOWS

/////////////////////////////////////////////////////////////////////////////
// THE SERVER
/////////////////////////////////////////////////////////////////////////////

inline bool operator < ( const CSphMatch & a, const CSphMatch & b )
{
	return a.m_iDocID < b.m_iDocID;
};

/////////////////////////////////////////////////////////////////////////////

void HandleCommandSearch ( int iSock, int iVer, InputBuffer_c & tReq )
{
	CSphQuery tQuery;
	assert ( sizeof(tQuery.m_eSort)==4 );

	// check major command version
	if ( (iVer>>8)!=(VER_COMMAND_SEARCH>>8) )
	{
		tReq.SendErrorReply ( "major command version mismatch (expected v.%d.x, got v.%d.%d)",
			VER_COMMAND_SEARCH>>8, iVer>>8, iVer&0xff );
		return;
	}

	/////////////////
	// parse request
	/////////////////

	// v.1.0. mode, limits, weights, ID/TS ranges
	int iOffset			= tReq.GetInt ();
	int iLimit			= tReq.GetInt ();
	int iMode			= tReq.GetInt ();
	tQuery.m_eSort		= (ESphSortOrder) tReq.GetInt ();
	tQuery.m_iGroups	= tReq.GetInts ( (int**)&tQuery.m_pGroups, SEARCHD_MAX_REQ_GROUPS, "invalid group count %d (should be in 0..%d range)" );
	tQuery.m_sQuery		= tReq.GetString ();
	tQuery.m_iWeights	= tReq.GetInts ( (int**)&tQuery.m_pWeights, SPH_MAX_FIELD_COUNT, "invalid weight count %d (should be in 0..%d range)" );
	CSphString sIndex	= tReq.GetString ();
	tQuery.m_iMinID		= tReq.GetDword ();
	tQuery.m_iMaxID		= tReq.GetDword ();
	tQuery.m_iMinTS		= tReq.GetDword ();
	tQuery.m_iMaxTS		= tReq.GetDword ();

	// v.1.1
	if ( iVer>=0x101 )
	{
		tQuery.m_iMinGID = tReq.GetDword ();
		tQuery.m_iMaxGID = tReq.GetDword ();
	}

	// additional checks
	if ( tReq.GetError() )
	{
		tReq.SendErrorReply ( "invalid or truncated request" );
		return;
	}
	if ( tQuery.m_iMinID>tQuery.m_iMaxID || tQuery.m_iMinTS>tQuery.m_iMaxTS )
	{
		tReq.SendErrorReply ( "invalid ID/TS range specified in query" );
		return;
	}
	if ( tQuery.m_eMode<0 || tQuery.m_eMode>SPH_MATCH_TOTAL )
	{
		tReq.SendErrorReply ( "invalid match mode %d", tQuery.m_eMode );
		return;
	}

	////////////////
	// do searching
	////////////////

	// do search
	CSphQueryResult * pRes = new CSphQueryResult ();
	ISphMatchQueue * pTop = sphCreateQueue ( &tQuery );
	float tmStart = sphLongTimer ();

#define REMOVE_DUPES 1

	if ( g_hDistIndexes(sIndex) )
	{
		// search through specified distributed index
		DistributedIndex_t & tDist = g_hDistIndexes[sIndex];

		// start connecting to remote agents
		ARRAY_FOREACH ( i, tDist.m_dAgents )
			ConnectToRemoteAgent ( &tDist.m_dAgents[i] );

		// connect to remote agents and query them first
		int iRemote = QueryRemoteAgents ( sIndex.cstr(), tDist, tQuery, iMode );

		// while the remote queries are running, do local searches
		// !COMMIT what if the remote agents finish early, could they timeout?
		float tmQuery = -sphLongTimer ();
		ARRAY_FOREACH ( i, tDist.m_dLocal )
		{
			const ServedIndex_t & tServed = g_hIndexes [ tDist.m_dLocal[i] ];
			assert ( tServed.m_pIndex );
			assert ( tServed.m_pDict );
			assert ( tServed.m_pTokenizer );

			// do query
			tQuery.m_pTokenizer = tServed.m_pTokenizer;
			tServed.m_pIndex->QueryEx ( tServed.m_pDict, &tQuery, pRes, pTop );

#if REMOVE_DUPES
			sphFlattenQueue ( pTop, pRes );
#endif
		}
		tmQuery += sphLongTimer ();

		if ( !iRemote && !tDist.m_dLocal.GetLength() )
		{
			tReq.SendErrorReply ( "all remote agents unreachable, no local indexes configured" );
			return;
		}

		// wait for remote queries to complete
		if ( iRemote )
		{
			int iMsecLeft = tDist.m_iAgentQueryTimeout - int(tmQuery*1000.0f);
			int iReplys = WaitForRemoteAgents ( sIndex.cstr(), tDist, pRes, Max ( iMsecLeft, 0 ) );
			if ( !iReplys && !tDist.m_dLocal.GetLength() )
			{
				tReq.SendErrorReply ( "all reachable remote agents timed out" );
				return;
			}

			// merge local and remote results
			ARRAY_FOREACH ( iAgent, tDist.m_dAgents )
				if ( tDist.m_dAgents[iAgent].m_tRes.m_dMatches.GetLength() )
				{
					// merge this agent's results
					Agent_t & tAgent = tDist.m_dAgents[iAgent];
					ARRAY_FOREACH ( i, tAgent.m_tRes.m_dMatches )
						pRes->m_dMatches.Add( tAgent.m_tRes.m_dMatches[i] );
					tAgent.m_tRes.m_dMatches.Reset ();

					// merge this agent's stats
					pRes->m_iTotalMatches += tAgent.m_tRes.m_iTotalMatches;
				}
		}

	} else if ( sIndex=="*" )
	{
		// search through all local indexes
		g_hIndexes.IterateStart ();
		while ( g_hIndexes.IterateNext () )
		{
			const ServedIndex_t & tServed = g_hIndexes.IterateGet ();
			assert ( tServed.m_pIndex );
			assert ( tServed.m_pDict );
			assert ( tServed.m_pTokenizer );

			// do query
			tQuery.m_pTokenizer = tServed.m_pTokenizer;
			tServed.m_pIndex->QueryEx ( tServed.m_pDict, &tQuery, pRes, pTop );

#if REMOVE_DUPES
			sphFlattenQueue ( pTop, pRes );
#endif
		}

	} else
	{
		// search through the specified local indexes
		int iSearched = 0;
		char * p = (char*)sIndex.cstr();
		while ( *p )
		{
			// skip non-alphas
			while ( (*p) && !sphIsAlpha(*p) ) p++;
			if ( !(*p) ) break;

			// this is my next index name
			const char * sNext = p;
			while ( sphIsAlpha(*p) ) p++;

			assert ( sNext!=p );
			if ( *p ) *p++ = '\0'; // if it was not the end yet, we'll continue from next char

			// check that this one exists
			if ( !g_hIndexes.Exists ( sNext ) )
			{
				tReq.SendErrorReply ( "invalid index '%s' specified in request", sNext );
				return;
			}

			const ServedIndex_t & tServed = g_hIndexes[sNext];
			assert ( tServed.m_pIndex );
			assert ( tServed.m_pDict );
			assert ( tServed.m_pTokenizer );

			// do query
			tQuery.m_pTokenizer = tServed.m_pTokenizer;
			tServed.m_pIndex->QueryEx ( tServed.m_pDict, &tQuery, pRes, pTop );
			iSearched++;

#if REMOVE_DUPES
			sphFlattenQueue ( pTop, pRes );
#endif
		}

		if ( !iSearched )
		{
			tReq.SendErrorReply ( "no valid indexes specified in request" );
			return;
		}
	}

#if REMOVE_DUPES
	{
		pRes->m_dMatches.Sort ();

		ARRAY_FOREACH ( i, pRes->m_dMatches )
		{
			if ( i==0 || pRes->m_dMatches[i].m_iDocID!=pRes->m_dMatches[i-1].m_iDocID )
				pTop->Push ( pRes->m_dMatches[i] );
		}
		pRes->m_dMatches.Reset ();
	}
#endif

	sphFlattenQueue ( pTop, pRes );

	pRes->m_iQueryTime = int ( 1000.0f*( sphLongTimer() - tmStart ) );

	// log query
	if ( g_iQueryLogFile>=0 )
	{
		time_t tNow;
		char sTimeBuf[128];
		char sBuf[1024];

		time ( &tNow );
		ctime_r ( &tNow, sTimeBuf );
		sTimeBuf [ strlen(sTimeBuf)-1 ] = '\0';

		static const char * sModes [ SPH_MATCH_TOTAL ] = { "all", "any", "phr" };
		snprintf ( sBuf, sizeof(sBuf), "[%s] %d.%03d sec: [%d %d %s/%d/%d %d] %s\n",
			sTimeBuf, pRes->m_iQueryTime/1000, pRes->m_iQueryTime%1000,
			iOffset, iLimit, sModes [ tQuery.m_eMode ], tQuery.m_eSort, tQuery.m_iGroups,
			pRes->m_iTotalMatches, tQuery.m_sQuery.cstr() );

		flock ( g_iQueryLogFile, LOCK_EX );
		lseek ( g_iQueryLogFile, 0, SEEK_END );
		write ( g_iQueryLogFile, sBuf, strlen(sBuf) );
		flock ( g_iQueryLogFile, LOCK_UN );
	}

	//////////////////////
	// serve the response
	//////////////////////

	int iCount = Min ( iLimit, pRes->m_dMatches.GetLength()-iOffset );

	int iRespLen = 20 + sizeof(CSphMatch)*iCount;
	for ( int i=0; i<pRes->m_iNumWords; i++ )
		iRespLen += 12 + strlen ( pRes->m_tWordStats[i].m_sWord.cstr() );

	// create buffer, send header
	NetOutputBuffer_c tOut ( iSock );
	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_SEARCH );
	tOut.SendInt ( iRespLen );

	// matches
	tOut.SendInt ( iCount );
	if ( iCount )
		tOut.SendBytes ( &pRes->m_dMatches[iOffset], sizeof(CSphMatch)*iCount );

	tOut.SendInt ( pRes->m_dMatches.GetLength() );
	tOut.SendInt ( pRes->m_iTotalMatches );
	tOut.SendInt ( pRes->m_iQueryTime );
	tOut.SendInt ( pRes->m_iNumWords );

	for ( int i=0; i<pRes->m_iNumWords; i++ )
	{
		tOut.SendString ( pRes->m_tWordStats[i].m_sWord.cstr() );
		tOut.SendInt ( pRes->m_tWordStats[i].m_iDocs );
		tOut.SendInt ( pRes->m_tWordStats[i].m_iHits );
	}

	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==iRespLen+8 );

	SafeDeleteArray ( pRes );
	SafeDelete ( pTop );
}

/////////////////////////////////////////////////////////////////////////////

void HandleCommandExcerpt ( int iSock, int iVer, InputBuffer_c & tReq )
{
	// check major command version
	if ( (iVer>>8)!=(VER_COMMAND_EXCERPT>>8) )
	{
		tReq.SendErrorReply ( "major command version mismatch (expected v.%d.x, got v.%d.%d)",
			VER_COMMAND_EXCERPT>>8, iVer>>8, iVer&0xff );
		return;
	}

	/////////////////////////////
	// parse and process request
	/////////////////////////////

	const int EXCERPT_MAX_ENTRIES			= 1024;
	const int EXCERPT_FLAG_REMOVESPACES		= 1;

	// v.1.0
	ExcerptQuery_t q;

	int iMode = tReq.GetInt ();
	int iFlags = tReq.GetInt ();
	CSphString sIndex = tReq.GetString ();
	q.m_sWords = tReq.GetString ();
	q.m_sBeforeMatch = tReq.GetString ();
	q.m_sAfterMatch = tReq.GetString ();
	q.m_sChunkSeparator = tReq.GetString ();
	q.m_iLimit = tReq.GetInt ();
	q.m_bRemoveSpaces = ( iFlags & EXCERPT_FLAG_REMOVESPACES );

	int iCount = tReq.GetInt ();
	if ( iCount<0 || iCount>EXCERPT_MAX_ENTRIES  )
	{
		tReq.SendErrorReply ( "invalid entries count %d", iCount );
		return;
	}

	CSphVector < char *, 32 > dExcerpts;
	for ( int i=0; i<iCount; i++ )
	{
		q.m_sSource = tReq.GetString ();
		if ( tReq.GetError() )
		{
			tReq.SendErrorReply ( "invalid or truncated request" );
			return;
		}
		dExcerpts.Add ( sphBuildExcerpt ( q ) );
	}

	////////////////
	// serve result
	////////////////

	int iRespLen = 0;
	ARRAY_FOREACH ( i, dExcerpts )
		iRespLen += 4 + strlen ( dExcerpts[i] );

	NetOutputBuffer_c tOut ( iSock );
	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_EXCERPT );
	tOut.SendInt ( iRespLen );
	ARRAY_FOREACH ( i, dExcerpts )
	{
		tOut.SendString ( dExcerpts[i] );
		SafeDeleteArray ( dExcerpts[i] );
	}

	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==iRespLen+8 );
}


void HandleClient ( int iSock )
{
	NetInputBuffer_c tBuf ( iSock );

	// send my version
	DWORD uServer = SPHINX_SEARCHD_PROTO;
	if ( sphSockSend ( iSock, (char*)&uServer, sizeof(DWORD), 0 )!=sizeof(DWORD) )
	{
		sphWarning ( "failed to send server version" );
		return;
	}

	// get client version and request
	tBuf.ReadFrom ( 12 ); // FIXME! magic
	int iClientVer = tBuf.GetInt ();
	int iCommand = tBuf.GetWord ();
	int iCommandVer = tBuf.GetWord ();
	int iLength = tBuf.GetInt ();
	if ( tBuf.GetError() )
	{
		sphWarning ( "failed to receive client version and request" );
		return;
	}

	// check request
	if ( iCommand<0 || iCommand>=SEARCHD_COMMAND_TOTAL
		|| iLength<=0 || iLength>NET_MAX_REQ_LEN )
	{
		// unknown command, default response header
		tBuf.SendErrorReply ( "unknown command (code=%d)", iCommand );

		// if request length is insane, low level comm is broken, so we bail out
		if ( iLength<=0 || iLength>NET_MAX_REQ_LEN )
		{
			sphWarning ( "ill-formed client request (length out of bounds)" );
			return;
		}
	}

	// get request body
	assert ( iLength>0 && iLength<=NET_MAX_REQ_LEN );
	if ( !tBuf.ReadFrom ( iLength ) )
	{
		sphWarning ( "failed to receive client request body" );
		return;
	}

	// handle known commands
	assert ( iCommand>=0 && iCommand<SEARCHD_COMMAND_TOTAL );
	switch ( iCommand )
	{
		case SEARCHD_COMMAND_SEARCH:	HandleCommandSearch ( iSock, iCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_EXCERPT:	HandleCommandExcerpt ( iSock, iCommandVer, tBuf ); break;
		default:						assert ( 0 && "INTERNAL ERROR: unhandled command" ); break;
	}
}

/////////////////////////////////////////////////////////////////////////////

bool IsReadable ( const char * sPath )
{
	int iFD = ::open ( sPath, O_RDONLY );

	if ( iFD<0 )
		return false;

	close ( iFD );
	return true;
}


bool TryRename ( const char * sIndex, const char * sPrefix, const char * sFromPostfix, const char * sToPostfix, bool bFatal )
{
	char sFrom [ SPH_MAX_FILENAME_LEN ];
	char sTo [ SPH_MAX_FILENAME_LEN ];

	sprintf ( sFrom, "%s%s", sPrefix, sFromPostfix );
	sprintf ( sTo, "%s%s", sPrefix, sToPostfix );
	if ( rename ( sFrom, sTo ) )
	{
		if ( bFatal )
		{
			sphFatal ( "rotating index '%s': rollback rename '%s' to '%s' failed: %s",
				sIndex, sFrom, sTo, strerror(errno) );
		} else
		{
			sphWarning ( "rotating index '%s': rename '%s' to '%s' failed: %s",
				sIndex, sFrom, sTo, strerror(errno) );
		}
		return false;
	}

	return true;
}


int main ( int argc, char **argv )
{
	int rsock;
	struct sockaddr_in remote_iaddr;
	socklen_t len;
	CSphConfig conf;

	fprintf ( stdout, SPHINX_BANNER );

	//////////////////////
	// parse command line
	//////////////////////

	const char *	sOptConfig		= "sphinx.conf";
	bool			bOptConsole		= false;

	int i;
	for ( i=1; i<argc; i++ )
	{
		if ( strcasecmp ( argv[i], "--config" )==0 && (i+1)<argc )
		{
			struct stat tStat;
			if ( !stat ( argv[i+1], &tStat ) )
				sOptConfig = argv[i+1];
			else
				sphWarning ( "failed to stat config file '%s', using default 'sphinx.conf'", argv[i+1] );
			i++;

		} else if ( strcasecmp ( argv[i], "--console" )==0 )
		{
			bOptConsole = true;
		} else
		{
			break;
		}
	}
	if ( i!=argc )
	{
		fprintf ( stdout, "ERROR: malformed or unknown option near '%s'.\n\n", argv[i] );
		fprintf ( stdout, "usage: searchd [--config file.conf] [--console]\n" );
		return 1;
	}

	#if USE_WINDOWS
	sphWarning ( "forcing --console mode on Windows" );
	bOptConsole = true;
	#endif // USE_WINDOWS

	/////////////////////
	// parse config file
	/////////////////////

	CSphConfigParser cp;
	sphInfo ( "using config file '%s'...", sOptConfig );

	// FIXME! add key validation here. g_dSphKeysCommon, g_dSphKeysSearchd
	if ( !cp.Parse ( sOptConfig ) )
		sphFatal ( "failed to parse config file '%s'", sOptConfig );

	const CSphConfig & hConf = cp.m_tConf;

	if ( !hConf.Exists ( "searchd" ) || !hConf["searchd"].Exists ( "searchd" ) )
		sphFatal ( "'searchd' config section not found in '%s'", sOptConfig );

	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];

	if ( !hConf.Exists ( "index" ) )
		sphFatal ( "no indexes found in '%s'", sOptConfig );

	#define CONF_CHECK(_hash,_key,_msg,_add) \
		if (!( _hash.Exists ( _key ) )) \
			sphFatal ( "mandatory option '%s' not found " _msg, _key, _add );

	CONF_CHECK ( hSearchd, "port", "in 'searchd' section", "" );
	if ( !bOptConsole )
		CONF_CHECK ( hSearchd, "pid_file", "in 'searchd' section", "" );

	int iPort = hSearchd["port"].intval();
	if ( !iPort )
		sphFatal ( "expected valid 'port', got '%s'", hSearchd["port"].cstr() );

	if ( hSearchd.Exists ( "read_timeout" ) && hSearchd["read_timeout"].intval()>=0 )
		g_iReadTimeout = hSearchd["read_timeout"].intval();

	if ( hSearchd.Exists ( "max_children" ) && hSearchd["max_children"].intval()>=0 )
		g_iMaxChildren = hSearchd["max_children"].intval();

	//////////////////////
	// build indexes hash
	//////////////////////

	int iValidIndexes = 0;
	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext() )
	{
		const CSphConfigSection & hIndex = hConf["index"].IterateGet();
		const char * sIndexName = hConf["index"].IterateGetKey().cstr();

		if ( hIndex("type") && hIndex["type"]=="distributed" )
		{
			///////////////////////////////
			// configure distributed index
			///////////////////////////////

			DistributedIndex_t tIdx;

			// add local agents
			for ( CSphVariant * pLocal = hIndex("local"); pLocal; pLocal = pLocal->m_pNext )
			{
				if ( !g_hIndexes ( pLocal->cstr() ) )
				{
					sphWarning ( "index '%s': no such local index '%s', SKIPPING",
						sIndexName, pLocal->cstr() );
					continue;
				}
				tIdx.m_dLocal.Add ( pLocal->cstr() );
			}

			// add remote agents
			for ( CSphVariant * pAgent = hIndex("agent"); pAgent; pAgent = pAgent->m_pNext )
			{
				Agent_t tAgent;

				// extract host name
				const char * p = pAgent->cstr();
				while ( sphIsAlpha(*p) || *p=='.' ) p++;
				if ( p==pAgent->cstr() )
				{
					sphWarning ( "index '%s': agent '%s': host name expected, SKIPPING",
						sIndexName, pAgent->cstr() );
					continue;
				}
				if ( *p++!=':' )
				{
					sphWarning ( "index '%s': agent '%s': colon expected near '%s', SKIPPING",
						sIndexName, pAgent->cstr(), p );
					continue;
				}
				tAgent.m_sHost = pAgent->SubString ( 0, p-1-pAgent->cstr() );

				// extract port
				if ( !isdigit(*p) )
				{
					sphWarning ( "index '%s': agent '%s': port number expected near '%s', SKIPPING",
						sIndexName, pAgent->cstr(), p );
					continue;
				}
				tAgent.m_iPort = atoi(p);
				if ( tAgent.m_iPort<=0 || tAgent.m_iPort>=65536 )
				{
					sphWarning ( "index '%s': agent '%s': invalid port number near '%s', SKIPPING",
						sIndexName, pAgent->cstr(), p );
					continue;
				}

				// extract index list
				while ( isdigit(*p) ) p++;
				if ( *p++!=':' )
				{
					sphWarning ( "index '%s': agent '%s': colon expected near '%s', SKIPPING",
						sIndexName, pAgent->cstr(), p );
					continue;
				}
				while ( isspace(*p) )
					p++;
				const char * sIndexList = p;
				while ( sphIsAlpha(*p) || isspace(*p) || *p==',' )
					p++;
				if ( *p )
				{
					sphWarning ( "index '%s': agent '%s': index list expected near '%s', SKIPPING",
						sIndexName, pAgent->cstr(), p );
					continue;
				}
				tAgent.m_sIndexes = sIndexList;

				// lookup address
				struct hostent * hp = gethostbyname ( tAgent.m_sHost.cstr() );
				if ( !hp )
				{
					sphWarning ( "index '%s': agent '%s': failed to lookup host name, SKIPPING",
						sIndexName, pAgent->cstr() );
					continue;
				}
				tAgent.SetAddr ( hp->h_addrtype, hp->h_length, hp->h_addr_list[0] );

				// done
				tIdx.m_dAgents.Add ( tAgent );
			}

			// configure options
			if ( hIndex("agent_connect_timeout") )
			{
				if ( hIndex["agent_connect_timeout"].intval()<=0 )
					sphWarning ( "index '%s': connect_timeout must be positive, ignored", sIndexName );
				else
					tIdx.m_iAgentConnectTimeout = hIndex["agent_connect_timeout"].intval();
			}

			if ( hIndex("agent_query_timeout") )
			{
				if ( hIndex["agent_query_timeout"].intval()<=0 )
					sphWarning ( "index '%s': query_timeout must be positive, ignored", sIndexName );
				else
					tIdx.m_iAgentQueryTimeout = hIndex["agent_query_timeout"].intval();
			}

			// finally, check and add distributed index to global table
			if ( tIdx.m_dAgents.GetLength()==0 && tIdx.m_dLocal.GetLength()==0 )
			{
				sphWarning ( "index '%s': no valid local/remote indexes in distributed index, SKIPPING",
					sIndexName );
				continue;

			} else
			{
				g_hDistIndexes.Add ( tIdx, sIndexName );
				iValidIndexes++;
			}

		} else
		{
			/////////////////////////
			// configure local index
			/////////////////////////

			// check path
			if ( !hIndex.Exists ( "path" ) )
			{
				sphWarning ( "key 'path' not found in index '%s' - NOT SERVING", sIndexName );
				continue;
			}

			// configure charset_type
			ISphTokenizer * pTokenizer = NULL;
			bool bUseUTF8 = false;
			if ( hIndex.Exists ( "charset_type" ) )
			{
				if ( hIndex["charset_type"]=="sbcs" )
					pTokenizer = sphCreateSBCSTokenizer ();

				else if ( hIndex["charset_type"]=="utf-8" )
				{
					pTokenizer = sphCreateUTF8Tokenizer ();
					bUseUTF8 = true;

				} else
				{
					sphWarning ( "unknown charset type '%s' in index '%s' - NOT SERVING", hIndex["charset_type"].cstr() );
					continue;
				}
			} else
			{
				pTokenizer = sphCreateSBCSTokenizer ();
			}

			// configure morphology
			DWORD iMorph = SPH_MORPH_NONE;
			if ( hIndex.Exists ( "morphology" ) )
			{
				int iRuMorph = ( bUseUTF8 ? SPH_MORPH_STEM_RU_UTF8 : SPH_MORPH_STEM_RU_CP1251 );
				if ( hIndex["morphology"]=="stem_en" )			iMorph = SPH_MORPH_STEM_EN;
				else if ( hIndex["morphology"]=="stem_ru" )		iMorph = iRuMorph;
				else if ( hIndex["morphology"]=="stem_enru" )	iMorph = SPH_MORPH_STEM_EN | iRuMorph;
				else if ( hIndex["morphology"]=="none" )		iMorph = SPH_MORPH_NONE;
				else if ( hIndex["morphology"].IsEmpty() )		iMorph = SPH_MORPH_NONE;
				else
					sphWarning ( "unknown morphology type '%s' in index '%s' ignored", hIndex["morphology"].cstr(), sIndexName );
			}

			// configure charset_table
			assert ( pTokenizer );
			if ( hIndex.Exists ( "charset_table" ) )
				if ( !pTokenizer->SetCaseFolding ( hIndex["charset_table"].cstr() ) )
			{
				sphWarning ( "failed to parse 'charset_table' in index '%s' - NOT SERVING", sIndexName );
				continue;
			}

			// create add this one to served hashes
			ServedIndex_t tIdx;
			tIdx.m_pIndex = sphCreateIndexPhrase ( hIndex["path"].cstr() );
			tIdx.m_pDict = new CSphDict_CRC32 ( iMorph );
			tIdx.m_pDict->LoadStopwords ( hIndex.Exists ( "stopwords" ) ? hIndex["stopwords"].cstr() : NULL, pTokenizer );
			tIdx.m_pTokenizer = pTokenizer;

			if ( !bOptConsole )
			{
				// check lock file
				char sTmp [ 1024 ];
				snprintf ( sTmp, sizeof(sTmp), "%s.spl", hIndex["path"].cstr() );
				sTmp [ sizeof(sTmp)-1 ] = '\0';

				struct stat tStat;
				if ( !stat ( sTmp, &tStat ) )
				{
					sphWarning ( "lock file '%s' for index '%s' exists - NOT SERVING", sIndexName, sTmp );
					continue;
				}

				// create lock file
				FILE * fp = fopen ( sTmp, "w" );
				if ( !fp )
					sphFatal ( "unable to create lock file '%s' for index '%s'", sTmp, sIndexName );
				fprintf ( fp, "%d", getpid() );
				fclose ( fp );

				tIdx.m_pLockFile = new CSphString ( sTmp );
				tIdx.m_pIndexPath = new CSphString ( hIndex["path"] );
			}

			g_hIndexes.Add ( tIdx, sIndexName );
			tIdx.Reset (); // so that the dtor wouln't delete everything
		}

		iValidIndexes++;
	}
	if ( !iValidIndexes )
		sphFatal ( "no valid indexes to serve" );

	///////////
	// startup
	///////////

	// handle my signals
	#if !USE_WINDOWS
	signal ( SIGCHLD, sigchld );
	signal ( SIGTERM, sigterm );
	signal ( SIGINT, sigterm );
	signal ( SIGHUP, sighup );
	#endif // !USE_WINDOWS

	// daemonize
	if ( !bOptConsole )
	{
		// create log
		const char * sLog = "searchd.log";
		if ( hSearchd.Exists ( "log" ) )
			sLog = hSearchd["log"].cstr();

		umask ( 066 );
		g_iLogFile = open ( sLog, O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
		if ( g_iLogFile<0 )
		{
			g_iLogFile = STDOUT_FILENO;
			sphFatal ( "failed to write log file '%s'", sLog );
		}

		// create query log if required
		if ( hSearchd.Exists ( "query_log" ) )
		{
			g_iQueryLogFile = open ( hSearchd["query_log"].cstr(), O_CREAT | O_RDWR | O_APPEND,
				S_IREAD | S_IWRITE );
			if ( g_iQueryLogFile<0 )
				sphFatal ( "failed to write query log file '%s'", hSearchd["query_log"].cstr() );
		}

		// do daemonize
		int iDevNull = open ( "/dev/null", O_RDWR );
		close ( STDIN_FILENO );
		close ( STDOUT_FILENO );
		close ( STDERR_FILENO );
		dup2 ( iDevNull, STDIN_FILENO );
		dup2 ( iDevNull, STDOUT_FILENO );
		dup2 ( iDevNull, STDERR_FILENO );
		g_bLogStdout = false;

		#if !USE_WINDOWS
		switch ( fork() )
		{
			case -1:
				// error
				Shutdown ();
				sphFatal ( "fork() failed (reason: %s)", strerror ( errno ) );
				exit ( 1 );

			case 0:
				// daemonized child
				break;

			default:
				// tty-controlled parent
				exit ( 0 );
		}
		#endif

	} else
	{
		// if we're running in console mode, dump queries to tty as well
		g_iQueryLogFile = g_iLogFile;
	}

	if ( !bOptConsole )
	{
		// create pid
		g_sPidFile = hSearchd["pid_file"].cstr();
		FILE * fp = fopen ( g_sPidFile, "w" );
		if ( !fp )
			sphFatal ( "unable to write pid file '%s'", g_sPidFile );
		fprintf ( fp, "%d", getpid() );	
		fclose ( fp );
	}

	// create and bind on socket
	g_iSocket = sphCreateServerSocket ( iPort );
	listen ( g_iSocket, 5 );

	/////////////////
	// serve clients
	/////////////////

	sphInfo ( "accepting connections" );
	for ( ;; )
	{
		// try to rotate indices
		if ( g_iHUP && !g_iChildren )
		{
			g_hIndexes.IterateStart();
			while ( g_hIndexes.IterateNext() )
			{
				ServedIndex_t & tIndex = g_hIndexes.IterateGet();
				const char * sIndex = g_hIndexes.IterateGetKey().cstr();
				assert ( tIndex.m_pIndex );
				assert ( tIndex.m_pLockFile );
				assert ( tIndex.m_pIndexPath );

				bool bSuccess = false;
				for ( ;; )
				{
					char sFile [ SPH_MAX_FILENAME_LEN ];
					const char * sPath = tIndex.m_pIndexPath->cstr();

					// whatever happens, we won't retry
					g_iHUP = 0;
					sphInfo ( "rotating index '%s': children exited, trying to rotate", sIndex );

					// check files
					sprintf ( sFile, "%s.new.spi", sPath );
					if ( !IsReadable ( sFile ) )
					{
						sphWarning ( "rotating index '%s': '%s' unreadable: %s",
							sIndex, sFile, strerror(errno) );
						break;
					}
					sprintf ( sFile, "%s.new.spd", sPath );
					if ( !IsReadable ( sFile ) )
					{
						sphWarning ( "rotating index '%s': '%s' unreadable: %s",
							sIndex, sFile, strerror(errno) );
						break;
					}

					// rename current to old
					if ( !TryRename ( sIndex, sPath, ".spi", ".old.spi", false ) )
						break;
					if ( !TryRename ( sIndex, sPath, ".spd", ".old.spd", false ) )
					{
						TryRename ( sIndex, sPath, ".old.spi", ".spi", true );
						break;
					}

					// rename new to current
					if ( !TryRename ( sIndex, sPath, ".new.spi", ".spi", false ) )
					{
						TryRename ( sIndex, sPath, ".old.spi", ".spi", true );
						TryRename ( sIndex, sPath, ".old.spd", ".spd", true );
						break;
					}
					if ( !TryRename ( sIndex, sPath, ".new.spd", ".spd", false ) )
					{
						TryRename ( sIndex, sPath, ".spi", ".new.spi", true );
						TryRename ( sIndex, sPath, ".old.spi", ".spi", true );
						TryRename ( sIndex, sPath, ".old.spd", ".spd", true );
						break;
					}

					// try to create new index
					CSphIndex * pNewIndex = sphCreateIndexPhrase ( sPath );
					if ( !pNewIndex )
					{
						sphWarning ( "rotating index '%s': failed to create new index object", sIndex );

						// try ro recover
						TryRename ( sIndex, sPath, ".spi", ".new.spi", true );
						TryRename ( sIndex, sPath, ".spd", ".new.spd", true );
						TryRename ( sIndex, sPath, ".old.spi", ".spi", true );
						TryRename ( sIndex, sPath, ".old.spd", ".spd", true );
						break;
					}

					// uff. all done
					SafeDelete ( tIndex.m_pIndex );
					tIndex.m_pIndex = pNewIndex;

					sphInfo ( "rotating index '%s': success", sIndex );
					bSuccess = true;
					break;
				}
				if ( !bSuccess )
					sphWarning ( "rotating index '%s': using old index", sIndex );
			}
		}

		#if USE_WINDOWS
		#pragma warning(disable:4127) // conditional expr is const
		#pragma warning(disable:4389) // signed/unsigned mismatch
		#endif // USE_WINDOWS

		// we can't simply accept, because of the need to react to HUPs
		fd_set fdsAccept;
		FD_ZERO ( &fdsAccept );
		FD_SET ( g_iSocket, &fdsAccept );

		#if USE_WINDOWS
		#pragma warning(default:4127) // conditional expr is const
		#pragma warning(default:4389) // signed/unsigned mismatch
		#endif // USE_WINDOWS

		struct timeval tvTimeout;
		tvTimeout.tv_sec = 1;
		tvTimeout.tv_usec = 0;

		if ( select ( 1+g_iSocket, &fdsAccept, NULL, &fdsAccept, &tvTimeout )<=0 )
			continue;

		// select says something interesting happened, so let's accept
		len = sizeof ( remote_iaddr );
		rsock = accept ( g_iSocket, (struct sockaddr*)&remote_iaddr, &len );

		int iErr = sphSockGetErrno();
		if ( rsock<0 && ( iErr==EINTR || iErr==EAGAIN || iErr==EWOULDBLOCK ) )
			continue;
		if ( rsock<0 )
			sphFatal ( "accept() failed (reason: %s)", sphSockError(iErr) );

		if ( ( g_iMaxChildren && g_iChildren>=g_iMaxChildren ) || g_iHUP )
		{
			const char * sMessage = "server maxed out, retry in a second";
			int iRespLen = 4 + strlen(sMessage);

			NetOutputBuffer_c tOut ( rsock );
			tOut.SendInt ( SEARCHD_RETRY );
			tOut.SendInt ( iRespLen );
			tOut.SendString ( sMessage );
			tOut.Flush ();

			sphWarning ( "maxed out, dismissing client" );
			sphSockClose ( rsock );
			continue;
		}

		if ( bOptConsole )
		{
			HandleClient ( rsock );
			sphSockClose ( rsock );
			continue;
		}

		#if !USE_WINDOWS
		switch ( fork() )
		{
			// fork() failed
			case -1:
				sphFatal ( "fork() failed (reason: %s)", strerror ( errno ) );

			// child process, handle client
			case 0:
				HandleClient ( rsock );
				sphSockClose ( rsock );
				exit ( 0 );
				break;

			// parent process, continue accept()ing
			default:
				g_iChildren++;
				sphSockClose ( rsock );
				break;
		}
		#endif // !USE_WINDOWS
	}
}

//
// $Id$
//
