//
// $Id$
//

//
// Copyright (c) 2001-2007, Andrew Aksyonoff. All rights reserved.
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
#include <limits.h>

/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
	// Win-specific headers and calls
	#include <io.h>
	#include <winsock2.h>

	#define sphSockRecv(_sock,_buf,_len)	::recv(_sock,_buf,_len,0)
	#define sphSockSend(_sock,_buf,_len)	::send(_sock,_buf,_len,0)
	#define sphSockClose(_sock)				::closesocket(_sock)

#else
	// UNIX-specific headers and calls
	#include <unistd.h>
	#include <netinet/in.h>
	#include <sys/file.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/wait.h>
	#include <netdb.h>

	// for cache
	#include <zlib.h>
	#include <sys/mman.h>
	#include <md5.h>

	// there's no MSG_NOSIGNAL on OS X
	#ifndef MSG_NOSIGNAL
	#define MSG_NOSIGNAL 0
	#endif

	#define sphSockRecv(_sock,_buf,_len)	::recv(_sock,_buf,_len,MSG_NOSIGNAL)
	#define sphSockSend(_sock,_buf,_len)	::send(_sock,_buf,_len,0)
	#define sphSockClose(_sock)				::close(_sock)

#endif

/////////////////////////////////////////////////////////////////////////////
// MISC GLOBALS
/////////////////////////////////////////////////////////////////////////////

struct ServedIndex_t
{
	CSphIndex *			m_pIndex;
	const CSphSchema *	m_pSchema;		///< pointer to index schema, managed by the index itself
	CSphDict *			m_pDict;
	ISphTokenizer *		m_pTokenizer;
	CSphString			m_sIndexPath;
	bool				m_bEnabled;		///< to disable index in cases when rotation fails
	bool				m_bMlock;

public:
						ServedIndex_t ();
						~ServedIndex_t ();
	void				Reset ();
};

/////////////////////////////////////////////////////////////////////////////

enum ESphLogLevel
{
	LOG_FATAL	= 0,
	LOG_WARNING	= 1,
	LOG_INFO	= 2
};

#define					SEARCHD_SERVICE_NAME	"searchd"

static bool				g_bService		= false;
#if USE_WINDOWS
static bool				g_bServiceStop	= false;
#endif

static CSphVector<CSphString,8>	g_dArgs;

static bool				g_bHeadDaemon	= false;
static bool				g_bLogStdout	= true;
static int				g_iLogFile		= STDOUT_FILENO;
static ESphLogLevel		g_eLogLevel		= LOG_INFO;

#if 0
static bool				g_bCacheEnable	= false;
static CSphString		g_sCacheDir ( "qcache" );
static int				g_iCacheTTL		= 300;
static bool				g_bCacheGzip	= false;
#endif

static int				g_iReadTimeout	= 5;	// sec
static int				g_iChildren		= 0;
static int				g_iMaxChildren	= 0;
static int				g_iSocket		= 0;
static int				g_iQueryLogFile	= -1;
static const char *		g_sPidFile		= NULL;
static int				g_iMaxMatches	= 1000;
static bool				g_bSeamlessRotate	= true;

static bool				g_bDoRotate			= false;	// flag that we should start rotation; set from SIGHUP

static SmallStringHash_T<ServedIndex_t>		g_hIndexes;				// served indexes hash
static CSphVector<const char *>				g_dRotating;			// names of indexes to be rotated this time
static const char *							g_sPrereading	= NULL;	// name of index currently being preread
static CSphIndex *							g_pPrereading	= NULL;	// rotation "buffer"

enum
{
	SPH_PIPE_UPDATED_ATTRS,
	SPH_PIPE_SAVED_ATTRS,
	SPH_PIPE_PREREAD
};

static CSphVector<int,8>	g_dPipes;		// currently open read-pipes to children processes

static CSphVector<DWORD>	g_dMvaStorage;	// per-query (!) pool to store MVAs received from remote agents

/////////////////////////////////////////////////////////////////////////////

/// known commands
enum SearchdCommand_e
{
	SEARCHD_COMMAND_SEARCH		= 0,
	SEARCHD_COMMAND_EXCERPT		= 1,
	SEARCHD_COMMAND_UPDATE		= 2,

	SEARCHD_COMMAND_TOTAL
};


/// known command versions
enum
{
	VER_COMMAND_SEARCH		= 0x10C,
	VER_COMMAND_EXCERPT		= 0x100,
	VER_COMMAND_UPDATE		= 0x100
};


/// known status return codes
enum SearchdStatus_e
{
	SEARCHD_OK		= 0,	///< general success, command-specific reply follows
	SEARCHD_ERROR	= 1,	///< general failure, error message follows
	SEARCHD_RETRY	= 2,	///< temporary failure, error message follows, client should retry later
	SEARCHD_WARNING	= 3		///< general success, warning message and command-specific reply follow
};

const int	MAX_RETRY_COUNT		= 8;
const int	MAX_RETRY_DELAY		= 1000;

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

#define ftruncate		_chsize
#define getpid			GetCurrentProcessId

void ctime_r ( time_t * tNow, char * sBuf )
{
	strcpy ( sBuf, ctime(tNow) );
}
#endif // USE_WINDOWS

/////////////////////////////////////////////////////////////////////////////
// MISC
/////////////////////////////////////////////////////////////////////////////

ServedIndex_t::ServedIndex_t ()
{
	Reset ();
}

void ServedIndex_t::Reset ()
{
	m_pIndex	= NULL;
	m_pDict		= NULL;
	m_pTokenizer= NULL;
	m_bEnabled	= true;
	m_bMlock	= false;
}

ServedIndex_t::~ServedIndex_t ()
{
	SafeDelete ( m_pIndex );
	SafeDelete ( m_pDict );
	SafeDelete ( m_pTokenizer );
}

/////////////////////////////////////////////////////////////////////////////
// LOGGING
/////////////////////////////////////////////////////////////////////////////

void Shutdown (); // forward ref for sphFatal()

void sphLog ( ESphLogLevel eLevel, const char * sFmt, va_list ap )
{
	if ( eLevel>g_eLogLevel || ( g_iLogFile<0 && !g_bService ) )
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
		snprintf ( sBuf, sizeof(sBuf)-1, "[%s] [%5d] %s", sTimeBuf, (int)getpid(), sBanner );
	else
		strcpy ( sBuf, sBanner );
	int iLen = strlen(sBuf);

	vsnprintf ( sBuf+iLen, sizeof(sBuf)-iLen-1, sFmt, ap );

#if USE_WINDOWS
	if ( g_bService && g_iLogFile==STDOUT_FILENO )
	{
		HANDLE hEventSource;
		LPCTSTR lpszStrings[2];

		hEventSource = RegisterEventSource ( NULL, SEARCHD_SERVICE_NAME );
		if ( hEventSource )
		{
			lpszStrings[0] = SEARCHD_SERVICE_NAME;
			lpszStrings[1] = sBuf;

			WORD eType = EVENTLOG_INFORMATION_TYPE;
			switch ( eLevel )
			{
				case LOG_FATAL:		eType = EVENTLOG_ERROR_TYPE; break;
				case LOG_WARNING:	eType = EVENTLOG_WARNING_TYPE; break;
				case LOG_INFO:		eType = EVENTLOG_INFORMATION_TYPE; break;
			}

			ReportEvent ( hEventSource,	// event log handle
				eType,					// event type
				0,						// event category
				0,						// event identifier
				NULL,					// no security identifier
				2,						// size of lpszStrings array
				0,						// no binary data
				lpszStrings,			// array of strings
				NULL );					// no binary data

			DeregisterEventSource ( hEventSource );
		}

	} else
#endif
	{
		strncat ( sBuf, "\n", sizeof(sBuf) );

		sphLockEx ( g_iLogFile, true );
		lseek ( g_iLogFile, 0, SEEK_END );
		write ( g_iLogFile, sBuf, strlen(sBuf) );
		sphLockUn ( g_iLogFile );

		if ( g_bLogStdout && g_iLogFile!=STDOUT_FILENO )
		{
			write ( STDOUT_FILENO, sBuf, strlen(sBuf) );
		}
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


void LogInternalError ( const char * sError )
{
	sphWarning( "INTERNAL ERROR: %s", sError );
}

/////////////////////////////////////////////////////////////////////////////

struct StrBuf_t
{
protected:
	char		m_sBuf [ 2048 ];
	char *		m_pBuf;
	int			m_iLeft;

public:
	StrBuf_t ()
	{
		memset ( m_sBuf, 0, sizeof(m_sBuf) );
		m_iLeft = sizeof(m_sBuf)-1;
		m_pBuf = m_sBuf;
	}

	const char * cstr ()
	{
		return m_sBuf;
	}

	int GetLength ()
	{
		return sizeof(m_sBuf)-1-m_iLeft;
	}

	bool Append ( const char * s, bool bWhole )
	{
		int iLen = strlen(s);
		if ( bWhole && m_iLeft<iLen )
			return false;

		iLen = Min ( m_iLeft, iLen );
		memcpy ( m_pBuf, s, iLen );
		m_pBuf += iLen;
		m_iLeft -= iLen;
		return true;
	}

	const StrBuf_t & operator += ( const char * s )
	{
		Append ( s, false );
		return *this;
	}
};


struct SearchFailure_t
{
public:
	CSphString	m_sIndex;	///< searched index name
	CSphString	m_sError;	///< search error message

public:
	SearchFailure_t () {}

	SearchFailure_t ( const CSphString & sIndex, const CSphString & sError )
	{
		m_sIndex = sIndex;
		m_sError = sError;
		if ( m_sIndex.IsEmpty() ) m_sIndex = "(no index name)";
		if ( m_sError.IsEmpty() ) m_sError = "(no message)";
	}

public:
	bool operator == ( const SearchFailure_t & r ) const
	{
		return m_sIndex==r.m_sIndex && m_sError==r.m_sError;
	}

	bool operator < ( const SearchFailure_t & r ) const
	{
		int iRes = strcmp ( m_sError.cstr(), r.m_sError.cstr() );
		if ( !iRes )
			iRes = strcmp ( m_sIndex.cstr(), r.m_sIndex.cstr() );
		return iRes<0;
	}

	const SearchFailure_t & operator = ( const SearchFailure_t & r )
	{
		m_sIndex = r.m_sIndex;
		m_sError = r.m_sError;
		return *this;
	}
};


struct SearchFailuresLog_t : protected CSphVector<SearchFailure_t,8>
{
public:
	CSphString	m_sIndex;
	CSphString	m_sPrefix;

	void Reset ()
	{
		m_sIndex = "";
		m_sPrefix= "";
	}

	bool IsEmpty ()
	{
		return GetLength()==0;
	}

	void Submit ( const char * sTemplate, ... )
	{
		assert ( !m_sIndex.IsEmpty() );

		char sBuf [ 2048 ];
		snprintf ( sBuf, sizeof(sBuf), "%s", m_sPrefix.IsEmpty() ? "" : m_sPrefix.cstr() );

		va_list ap;
		va_start ( ap, sTemplate );
		int iLen = strlen(sBuf);
		vsnprintf ( sBuf+iLen, sizeof(sBuf)-iLen, sTemplate, ap );
		va_end ( ap );

		Add ( SearchFailure_t ( m_sIndex, sBuf ) );
	}

	void BuildReport ( StrBuf_t & sReport )
	{
		if ( !GetLength() )
			return;

		// collapse same messages
		Sort ();
		int iSpanStart = 0;

		for ( int i=1; i<=GetLength(); i++ )
		{
			// keep scanning while error text is the same
			if ( i!=GetLength() )
				if ( m_pData[i].m_sError==m_pData[i-1].m_sError )
					continue;

			// build current span
			StrBuf_t sSpan;
			if ( iSpanStart )
				sSpan += "; ";
			sSpan += "index ";
			for ( int j=iSpanStart; j<i; j++ )
			{
				if ( j!=iSpanStart )
					sSpan += ",";
				sSpan += m_pData[j].m_sIndex.cstr();
			}
			sSpan += ": ";
			if ( !sSpan.Append ( m_pData[iSpanStart].m_sError.cstr(), true ) )
				break;

			// flush current span
			if ( !sReport.Append ( sSpan.cstr(), true ) )
				break;

			// done
			iSpanStart = i;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// SIGNAL HANDLERS
/////////////////////////////////////////////////////////////////////////////

void Shutdown ()
{
	// some head-only shutdown procedures
	if ( g_bHeadDaemon )
	{
		// save attribute updates for all local indexes
		g_hIndexes.IterateStart ();
		while ( g_hIndexes.IterateNext () )
		{
			const ServedIndex_t & tServed = g_hIndexes.IterateGet ();
			if ( !tServed.m_bEnabled )
				continue;

			if ( !tServed.m_pIndex->SaveAttributes () )
				sphWarning ( "index %s: attrs save failed: %s",
				g_hIndexes.IterateGetKey().cstr(), tServed.m_pIndex->GetLastError().cstr() );
		}

		// unlock indexes
		g_hIndexes.IterateStart();
		while ( g_hIndexes.IterateNext() )
			g_hIndexes.IterateGet().m_pIndex->Unlock();
		g_hIndexes.Reset();

		// remove pid
		if ( g_sPidFile )
			::unlink ( g_sPidFile );
	}

	if ( g_iSocket )
		sphSockClose ( g_iSocket );

	if ( g_bHeadDaemon )
		sphInfo ( "shutdown complete" );
}


#if !USE_WINDOWS
void sigchld ( int )
{
	signal ( SIGCHLD, sigchld );
	while ( waitpid ( 0, (int *)0, WNOHANG | WUNTRACED ) > 0 )
		g_iChildren--;
}


void sighup ( int )
{
	sphInfo ( "rotating indices" );
	g_bDoRotate = true;
}


void sigterm ( int )
{
	if ( g_bHeadDaemon )
		sphInfo ( "caught SIGTERM, shutting down" );

	Shutdown ();
	exit ( 0 );
}
#endif // !USE_WINDOWS

/////////////////////////////////////////////////////////////////////////////
// NETWORK STUFF
/////////////////////////////////////////////////////////////////////////////

const int		MAX_PACKET_SIZE			= 8*1024*1024;
const int		SEARCHD_MAX_ATTRS		= 256;
const int		SEARCHD_MAX_ATTR_VALUES	= 4096;


// wrap FD_SET to prevent warnings on Windows
#if USE_WINDOWS

#pragma warning(disable:4127) // conditional expr is const
#pragma warning(disable:4389) // signed/unsigned mismatch

void SPH_FD_SET ( int fd, fd_set * fdset ) { FD_SET ( fd, fdset ); }

#pragma warning(default:4127) // conditional expr is const
#pragma warning(default:4389) // signed/unsigned mismatch

#else // !USE_WINDOWS

#define	SPH_FD_SET FD_SET

#endif


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


int sphCreateServerSocket ( DWORD uAddr, int iPort )
{
	static struct sockaddr_in iaddr;

	iaddr.sin_family = AF_INET;
	iaddr.sin_addr.s_addr = uAddr;
	iaddr.sin_port = htons ( (short)iPort );

	char sAddr [ 256 ];
	DWORD uHost = ntohl(uAddr);
	snprintf ( sAddr, sizeof(sAddr), "%d.%d.%d.%d:%d", 
		(uHost>>24) & 0xff, (uHost>>16) & 0xff, (uHost>>8) & 0xff, uHost & 0xff,
		iPort );

	sphInfo ( "creating server socket on %s", sAddr );
	int iSock = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( iSock<0 )
		sphFatal ( "failed to create server socket on %s: %s", sAddr, sphSockError() );

	int iOn = 1;
	if ( setsockopt ( iSock, SOL_SOCKET, SO_REUSEADDR, (char*)&iOn, sizeof(iOn) ) )
		sphFatal ( "setsockopt() failed: %s", sphSockError() );

	int iTries = 12;
	int iRes;
	do
	{
		iRes = bind ( iSock, (struct sockaddr *)&iaddr, sizeof(iaddr) );
		if ( iRes==0 )
			break;

		sphInfo ( "failed to bind on %s, retrying...", sAddr );
		sphUsleep ( 3000 );
	} while ( --iTries>0 );
	if ( iRes )
		sphFatal ( "failed to bind on %s", sAddr );

	return iSock;
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


int sphSockRead ( int iSock, void * buf, int iLen )
{
	assert ( iLen>0 );

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
		SPH_FD_SET ( iSock, &fdRead );

		fd_set fdExcept;
		FD_ZERO ( &fdExcept );
		SPH_FD_SET ( iSock, &fdExcept );

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

		// try to receive next chunk
		iRes = sphSockRecv ( iSock, pBuf, iLeftBytes );

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

	bool		SendInt ( int iValue )			{ return SendT<int> ( htonl ( iValue ) ); }
	bool		SendDword ( DWORD iValue )		{ return SendT<DWORD> ( htonl ( iValue ) ); }
	bool		SendWord ( WORD iValue )		{ return SendT<WORD> ( htons ( iValue ) ); }
	bool		SendUint64 ( uint64_t iValue )	{ SendT<DWORD> ( htonl ( (DWORD)(iValue>>32) ) ); return SendT<DWORD> ( htonl ( (DWORD)(iValue&0xffffffffUL) ) ); }

#if USE_64BIT
	bool		SendDocid ( SphDocID_t iValue )	{ return SendUint64 ( iValue ); }
#else
	bool		SendDocid ( SphDocID_t iValue )	{ return SendDword ( iValue ); }
#endif

	bool		SendString ( const char * sStr );

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

	bool							SendBytes ( const void * pBuf, int iLen );	///< protected to avoid network-vs-host order bugs
	template < typename T > bool	SendT ( T tValue );							///< protected to avoid network-vs-host order bugs
};


/// generic request buffer
class InputBuffer_c
{
public:
					InputBuffer_c ( const BYTE * pBuf, int iLen );
	virtual			~InputBuffer_c () {}

	int				GetInt () { return ntohl ( GetT<int> () ); }
	WORD			GetWord () { return ntohs ( GetT<WORD> () ); }
	DWORD			GetDword () { return ntohl ( GetT<DWORD> () ); }
	uint64_t		GetUint64() { uint64_t uRes = GetDword(); return (uRes<<32)+GetDword(); };
	BYTE			GetByte () { return GetT<BYTE> (); }
	CSphString		GetString ();
	int				GetDwords ( DWORD ** pBuffer, int iMax, const char * sErrorTemplate );
	bool			GetError () { return m_bError; }

	virtual void	SendErrorReply ( const char *, ... ) = 0;

protected:
	const BYTE *	m_pBuf;
	const BYTE *	m_pCur;
	bool			m_bError;
	int				m_iLen;

protected:
	void						SetError ( bool bError ) { m_bError = bError; }
	bool						GetBytes ( void * pBuf, int iLen );
	template < typename T > T	GetT ();
};


/// simple memory request buffer
class MemInputBuffer_c : public InputBuffer_c
{
public:
					MemInputBuffer_c ( const BYTE * pBuf, int iLen ) : InputBuffer_c ( pBuf, iLen ) {}
	virtual void	SendErrorReply ( const char *, ... ) {}
};


/// simple network request buffer
class NetInputBuffer_c : public InputBuffer_c
{
public:
					NetInputBuffer_c ( int iSock );
	virtual			~NetInputBuffer_c ();

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
	*(int*)m_pBuffer = htonl(iLen);
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

	int iRes = sphSockSend ( m_iSock, (char*)&m_dBuffer[0], iLen );
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

InputBuffer_c::InputBuffer_c ( const BYTE * pBuf, int iLen )
	: m_pBuf ( pBuf )
	, m_pCur ( pBuf )
	, m_bError ( !pBuf || iLen<0 )
	, m_iLen ( iLen )
{}


template < typename T > T InputBuffer_c::GetT ()
{
	if ( m_bError || ( m_pCur+sizeof(T) > m_pBuf+m_iLen ) )
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
	if ( m_bError || iLen<0 || iLen>MAX_PACKET_SIZE || ( m_pCur+iLen > m_pBuf+m_iLen ) )
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
	assert ( iLen>0 && iLen<=MAX_PACKET_SIZE );

	if ( m_bError || ( m_pCur+iLen > m_pBuf+m_iLen ) )
	{
		SetError ( true );
		return false;
	}

	memcpy ( pBuf, m_pCur, iLen );
	m_pCur += iLen;
	return true;;
}


int InputBuffer_c::GetDwords ( DWORD ** ppBuffer, int iMax, const char * sErrorTemplate )
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
		assert ( !(*ppBuffer) ); // potential leak
		(*ppBuffer) = new DWORD [ iCount ];
		if ( !GetBytes ( (*ppBuffer), sizeof(int)*iCount ) )
		{
			SafeDeleteArray ( (*ppBuffer) );
			return -1;
		}
		for ( int i=0; i<iCount; i++ )
			(*ppBuffer)[i] = htonl ( (*ppBuffer)[i] );
	}
	return iCount;
}

/////////////////////////////////////////////////////////////////////////////

NetInputBuffer_c::NetInputBuffer_c ( int iSock )
	: InputBuffer_c ( new BYTE [ MAX_PACKET_SIZE ], MAX_PACKET_SIZE )
	, m_iSock ( iSock )
{
	if ( !m_pBuf )
		m_bError = true;
}


NetInputBuffer_c::~NetInputBuffer_c ()
{
	SafeDeleteArray ( m_pBuf );
}


bool NetInputBuffer_c::ReadFrom ( int iLen )
{
	assert ( iLen>0 );
	assert ( iLen<=MAX_PACKET_SIZE );
	assert ( m_iSock>0 );

	m_pCur = m_pBuf;

	int iGot = sphSockRead ( m_iSock, const_cast<BYTE*>(m_pBuf), iLen );
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
	WORD * p0 = (WORD*)&dBuf[0];
	p0[0] = htons(SEARCHD_ERROR); // error code
	p0[1] = 0; // version doesn't matter

	// fill error string
	char * sBuf = dBuf + iHeaderLen;

	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf, iMaxStrLen, sTemplate, ap );
	va_end ( ap );

	sBuf[iMaxStrLen] = '\0';
	int iStrLen = strlen(sBuf);

	// fixup lengths
	DWORD * p4 = (DWORD*)&dBuf[4];
	p4[0] = htonl(4+iStrLen);
	p4[1] = htonl(iStrLen);

	// send!
	sphSockSend ( m_iSock, dBuf, iHeaderLen+iStrLen );
}

/////////////////////////////////////////////////////////////////////////////
// SIMPLE FILE-BASED QUERY CACHE
/////////////////////////////////////////////////////////////////////////////

#if 0

/// my simple cache
class CSphCache
{
public:
				CSphCache ( const char * sCacheDir, int iCacheTTL, bool bUseGzip );
	bool		ReadFromFile ( const CSphQuery & tQuery, const char * sIndexName, const char * sIndexFileName, CSphQueryResult * pRes );
	bool		StoreResult ( const CSphQuery & tQuery, const char * sIndexName, const CSphQueryResult * pRes );

private:
	char		m_sCacheFileName [ 16*2 + 1 ];
	CSphString	m_sCacheDir;
	int			m_iCacheTTL;
	bool		m_bUseGzip;

	void		GenerateCacheFileName ( const CSphQuery & tQuery );
};


CSphCache::CSphCache ( const char * sCacheDir, int iCacheTTL, bool bUseGzip )
{
	memset ( m_sCacheFileName, 0, sizeof(m_sCacheFileName) );
	m_sCacheDir = sCacheDir;
	m_iCacheTTL = iCacheTTL;
	m_bUseGzip = bUseGzip;
}


bool CSphCache::ReadFromFile ( const CSphQuery & tQuery, const char * sIndexName, const char * sIndexFileName, CSphQueryResult * pRes )
{
	// build filename, and check the cached result file
	GenerateCacheFileName ( tQuery );

	char sBuf [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sBuf, sizeof(sBuf), "%s/%s/%c%c/%c%c/%s", m_sCacheDir.cstr(),
		sIndexName, m_sCacheFileName[0], m_sCacheFileName[1], m_sCacheFileName[2], m_sCacheFileName[3], m_sCacheFileName );

	struct stat stFileInfo;
	if ( lstat ( sBuf, &stFileInfo) < 0 )
		return false; // cache miss; uncached

	if ( m_iCacheTTL>0 && tQuery.m_eSort==SPH_SORT_TIME_SEGMENTS )
		if ( ( time(NULL) - stFileInfo.st_mtime ) > m_iCacheTTL )
			return false; // cache miss; TTL expired

	// check index modification time
	// FIXME! should query this from index!
	char sBufIndex [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sBufIndex, sizeof(sBufIndex), "%s.spi", sIndexFileName );

	struct stat stIndexInfo;
	if ( lstat ( sBufIndex, &stIndexInfo) < 0 )
	{
		sphWarning ( "failed to lstat '%s': errno=%d, err=%s", sBufIndex, errno, strerror(errno) );
		return false;
	}
	if ( stIndexInfo.st_mtime>=stFileInfo.st_mtime )
		return false; // cache miss; index is newer that the cached result

	// read the data
	int fCache = open ( sBuf, O_RDONLY );
	if ( fCache<0 )
	{
		sphWarning ( "failed to open '%s': errno=%d, err=%s", sBuf, errno, strerror(errno) );
		return false;
	}

	int * pMapFile = NULL;
	int iFileSize = 0;

	if ( m_bUseGzip )
	{
		gzFile fgzCache = gzdopen ( fCache, "r" );
		if ( fgzCache==Z_NULL )
		{
			close ( fCache );
			sphWarning ( "failed to gzdopen '%s': errno=%d, err=%s", errno, strerror(errno) ); 
			return false;
		}

		int iGzipBuffer = 32768; // start with a 32K buffer
		int iGzipLeft = iGzipBuffer;

		pMapFile = new int [ iGzipBuffer/sizeof(int) ];
		BYTE * pCur = (BYTE *)pMapFile;

		for ( ;; )
		{
			// read next chunk
			int iRead = gzread ( fgzCache, pCur, iGzipLeft );

			// if it's EOF, we're done
			if ( iRead==0 && gzeof ( fgzCache ) )
				break;

			// if it's error, bail out
			if ( iRead<=0 )
			{
				sphWarning ( "failed to gzread '%s': gzread error", sBuf );
				SafeDeleteArray ( pMapFile );
				gzclose ( fgzCache );
				close ( fCache );
				return false;
			}

			// update counters
			pCur += iRead;
			iFileSize += iRead;
			iGzipLeft -= iRead;
			if ( iFileSize<iGzipBuffer )
				continue;
			assert ( iFileSize==iGzipBuffer );

			// realloc the buffer, if necessary
			if ( iGzipBuffer>=1048576 )
			{
				sphWarning ( "failed to gzread '%s': too big (over 1M)", sBuf );
				SafeDeleteArray ( pMapFile );
				gzclose ( fgzCache );
				close ( fCache );
				return false;
			}

			int * pNew = new int [ 2*iGzipBuffer/sizeof(int) ];
			memcpy ( pNew, pMapFile, iGzipBuffer );

			SafeDeleteArray ( pMapFile );
			pMapFile = pNew;

			pCur = ((BYTE*)pNew) + iGzipBuffer;
			iGzipBuffer *= 2;
		}
		gzclose ( fgzCache );

	} else
	{
		iFileSize = lseek ( fCache, 0, SEEK_END );
		pMapFile = (int*)mmap ( 0, (size_t)iFileSize, PROT_READ,
			MAP_SHARED | MAP_NORESERVE, fCache, 0L );

		if ( pMapFile==(int*)MAP_FAILED )
		{
			sphWarning ( "failed to mmap '%s': errno=%d, err=%s", sBuf, errno, strerror(errno) );
			close ( fCache );
			return false;
		}
	}

	// parse cached result
	assert ( pMapFile );
	assert ( iFileSize>0 );

	bool bOK = false;
	for ( ;; )
	{
		if ( iFileSize<3*(int)sizeof(int) )
		{
			sphWarning ( "failed to read header from '%s': file too short", sBuf );
			break;
		}

		int * pCur = pMapFile;
		char * pEnd = ((char*)pMapFile) + iFileSize;

		int iAttrs = *pCur++; // !COMMIT serialize whole schema here
		pRes->m_iNumWords = *pCur++;
		int iMatches = *pCur++; 
		pRes->m_dMatches.Resize ( iMatches );
		pRes->m_iTotalMatches = *pCur++; 

		if ( iFileSize < (int)sizeof(int)*( 3 + iMatches*4 ) )
		{
			sphWarning ( "failed to read matches from '%s': file too short", sBuf );
			break;
		}

		for ( int i=0; i<iMatches; i++ )
		{
			CSphMatch & tMatch = pRes->m_dMatches[i];
			tMatch.m_iDocID = *pCur++;
			tMatch.m_iWeight = *pCur++;
			if ( iAttrs )
			{
				assert ( !tMatch.m_pAttrs );
				tMatch.m_iAttrs = iAttrs;
				tMatch.m_pAttrs = new DWORD [ iAttrs ]; // !COMMIT pool these alloc
				for ( int j=0; j<iAttrs; j++ )
					tMatch.m_pAttrs[j] = *pCur++;
			}
		}

		int i;
		for ( i=0; i<pRes->m_iNumWords; i++ )
		{
			if ( ((char*)pCur) + sizeof(int) > pEnd )
			{
				sphWarning ( "failed to read query term length from '%s': file too short", sBuf );
				break;
			}
			int iWordLen = *pCur++;

			if ( ((char*)pCur) + iWordLen + 2*sizeof(int) > pEnd )
			{
				sphWarning ( "failed to read query term data from '%s': file too short", sBuf );
				break;
			}

			pRes->m_tWordStats[i].m_sWord.SetBinary ( (char*)pCur, iWordLen );
			pCur = (int*)( ((char*)pCur) + iWordLen );

			pRes->m_tWordStats[i].m_iDocs = *pCur++; 
			pRes->m_tWordStats[i].m_iHits = *pCur++; 
		}
		if ( i!=pRes->m_iNumWords )
			break;

		bOK = true;
		break;
	}

	if ( m_bUseGzip )
	{
		SafeDeleteArray ( pMapFile );
	} else
	{
		munmap ( pMapFile, iFileSize );
	}
	close ( fCache );
	return bOK;
}


bool CSphCache::StoreResult ( const CSphQuery & tQuery, const char * sIndexName, const CSphQueryResult * pRes )
{
	GenerateCacheFileName ( tQuery );

	// create the dir if it doesn't exist yet
	char sBuf [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sBuf, sizeof(sBuf), "%s/%s/%c%c/%c%c", m_sCacheDir.cstr(), sIndexName, m_sCacheFileName[0], m_sCacheFileName[1],m_sCacheFileName[2], m_sCacheFileName[3] );

	struct stat stDir;
	if ( lstat ( sBuf, &stDir)!=0 )
	{
		snprintf ( sBuf, sizeof(sBuf), "%s/%s", m_sCacheDir.cstr(), sIndexName );
		mkdir ( sBuf, 0755 );
		snprintf ( sBuf, sizeof(sBuf), "%s/%s/%c%c", m_sCacheDir.cstr(), sIndexName, m_sCacheFileName[0], m_sCacheFileName[1] );
		mkdir ( sBuf, 0755 );
		snprintf ( sBuf, sizeof(sBuf), "%s/%s/%c%c/%c%c", m_sCacheDir.cstr(), sIndexName, m_sCacheFileName[0], m_sCacheFileName[1],m_sCacheFileName[2], m_sCacheFileName[3] );
		mkdir ( sBuf, 0755 );
	}

	// create the file
	snprintf ( sBuf, sizeof(sBuf), "%s/%s/%c%c/%c%c/%s", m_sCacheDir.cstr(), sIndexName,
		m_sCacheFileName[0], m_sCacheFileName[1], m_sCacheFileName[2], m_sCacheFileName[3], m_sCacheFileName );
	int fCache = open ( sBuf, O_CREAT | O_TRUNC | O_WRONLY, 0644 );
	if ( fCache<0 )
	{
		sphWarning ( "failed to create '%s': errno=%d, err=%s", sBuf, errno, strerror(errno) );
		return false;
	}

	// create the gzip stream
	gzFile fgzCache = Z_NULL;
	if ( m_bUseGzip )
		fgzCache = gzdopen ( fCache, "wb" );

	int iBufLen = sizeof(int)*( 4 + 4*pRes->m_dMatches.GetLength() + 3*pRes->m_iNumWords );
	for ( int i=0; i<pRes->m_iNumWords; i++ )
		iBufLen += strlen ( pRes->m_tWordStats[i].m_sWord.cstr() );

	int * pBuf = new int [ 1 + iBufLen/sizeof(int) ];
	int * pCur = pBuf;

	int iAttrs = pRes->m_tSchema.m_dAttrs.GetLength ();
	*pCur++ = iAttrs; // !COMMIT serialize whole schema here
	*pCur++ = pRes->m_iNumWords;
	*pCur++ = pRes->m_dMatches.GetLength();
	*pCur++ = pRes->m_iTotalMatches;
	ARRAY_FOREACH ( i, pRes->m_dMatches )
	{
		const CSphMatch & tMatch = pRes->m_dMatches[i];
		*pCur++ = tMatch.m_iDocID;
		*pCur++ = tMatch.m_iWeight;

		assert ( tMatch.m_iAttrs==iAttrs );
		for ( int j=0; j<iAttrs; j++ )
			*pCur++ = tMatch.m_pAttrs[j];
	}

	for ( int i=0; i<pRes->m_iNumWords; i++ )
	{
		int iLen = strlen ( pRes->m_tWordStats[i].m_sWord.cstr() );
		*pCur++ = iLen;

		memcpy ( pCur, pRes->m_tWordStats[i].m_sWord.cstr(), iLen );
		pCur = (int*)( ((char*)pCur) + iLen );

		*pCur++ = pRes->m_tWordStats[i].m_iDocs;
		*pCur++ = pRes->m_tWordStats[i].m_iHits;
	}
	assert ( ((char*)pCur)==((char*)pBuf) + iBufLen );

	bool bOK = true;
	if ( m_bUseGzip )
	{
		if ( gzwrite ( fgzCache, pBuf, iBufLen )<0 )
		{
			sphWarning ( "failed to gzwrite '%s': errno=%d, err=%s", errno, strerror(errno) ); // FIXME! should use gzerror everywhere
			bOK = false;
		}
		gzclose ( fgzCache );
	} else
	{
		if ( write ( fCache, pBuf, iBufLen )<0 )
		{
			sphWarning ( "failed to write '%s': errno=%d, err=%s", errno, strerror(errno) );
			bOK = false;
		}
		close ( fCache );
	}

	SafeDeleteArray ( pBuf );
	return true;
}


void CSphCache::GenerateCacheFileName ( const CSphQuery & tQuery )
{
	md5_state_t tState;
	md5_byte_t tDigest[16];
	char sBuf[2048];

	int iLen = snprintf ( sBuf, sizeof(sBuf), "%s-%d-%d-" DOCID_FMT "-" DOCID_FMT,
		tQuery.m_sQuery.cstr(),tQuery.m_eMode, tQuery.m_eSort,
		tQuery.m_iMinID, tQuery.m_iMaxID );

	md5_init ( &tState );
	md5_append ( &tState, (md5_byte_t*)sBuf, iLen );

	if ( tQuery.m_iWeights>0 )
		md5_append( &tState, (md5_byte_t*)tQuery.m_pWeights, tQuery.m_iWeights*sizeof(DWORD) );

	// !COMMIT pass all filters to md5 too

	md5_finish ( &tState, tDigest );
	for ( int iDigest=0; iDigest<16; ++iDigest )
		snprintf ( m_sCacheFileName + iDigest*2, 3, "%02x", tDigest[iDigest] );
}

#endif // !USE_WINDOWS

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
	AGENT_REPLY,				///< reading reply
	AGENT_RETRY					///< should retry
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
	bool			m_bFailure;		///< whether query was succesful

	int				m_iReplyStatus;	///< reply status code
	int				m_iReplySize;	///< how many reply bytes are there
	int				m_iReplyRead;	///< how many reply bytes are alredy received
	BYTE *			m_pReplyBuf;	///< reply buffer

protected:
	int				m_iAddrType;
	DWORD			m_uAddr;

public:
	Agent_t ()
		: m_iPort ( -1 )
		, m_iSock ( -1 )
		, m_eState ( AGENT_UNUSED )
		, m_bFailure ( false )
		, m_iReplyStatus ( -1 )
		, m_iReplySize ( 0 )
		, m_iReplyRead ( 0 )
		, m_pReplyBuf ( NULL )
		, m_uAddr ( 0 )
	{
	}

	~Agent_t ()
	{
		Close ();
	}

	void Close ()
	{
		SafeDeleteArray ( m_pReplyBuf );
		if ( m_iSock>0 )
		{
			sphSockClose ( m_iSock );
			m_iSock = -1;
			if ( m_eState!=AGENT_RETRY )
				m_eState = AGENT_UNUSED;
		}
	}

	void SetAddr ( int iAddrType, int iAddrLen, const char * pAddr )
	{
		assert ( pAddr );
		assert ( iAddrLen==sizeof(m_uAddr) );

		m_iAddrType = iAddrType;
		memcpy ( &m_uAddr, pAddr, iAddrLen );
	}

	int GetAddrType () const		{ return m_iAddrType; }
	int GetAddrLen () const			{ return sizeof(m_uAddr); }
	const char * GetAddr () const	{ return (const char *)&m_uAddr; }
};

/// distributed index
struct DistributedIndex_t
{
	CSphVector<Agent_t,8>		m_dAgents;					///< remote agents
	CSphVector<CSphString,8>	m_dLocal;					///< local indexes
	int							m_iAgentConnectTimeout;		///< in msec
	int							m_iAgentQueryTimeout;		///< in msec

	DistributedIndex_t ()
		: m_iAgentConnectTimeout ( 1000 )
		, m_iAgentQueryTimeout ( 3000 )
	{}
};

static SmallStringHash_T < DistributedIndex_t >		g_hDistIndexes;

/////////////////////////////////////////////////////////////////////////////

struct IRequestBuilder_t : public ISphNoncopyable
{
	virtual ~IRequestBuilder_t () {} // to avoid gcc4 warns
	virtual void BuildRequest ( const char * sIndexes, NetOutputBuffer_c & tOut ) const = 0;
};


struct IReplyParser_t
{
	virtual ~IReplyParser_t () {} // to avoid gcc4 warns
	virtual bool ParseReply ( MemInputBuffer_c & tReq, Agent_t & tAgent, SearchFailuresLog_t & dFailures ) const = 0;
};


void ConnectToRemoteAgents ( CSphVector<Agent_t,8> & dAgents, bool bRetryOnly, SearchFailuresLog_t & dFailures )
{
	ARRAY_FOREACH ( iAgent, dAgents )
	{
		Agent_t & tAgent = dAgents[iAgent];
		if ( bRetryOnly && tAgent.m_eState!=AGENT_RETRY )
			continue;

		dFailures.m_sPrefix.SetSprintf ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );
		tAgent.m_eState = AGENT_UNUSED;

		struct sockaddr_in sa;
		memset ( &sa, 0, sizeof(sa) );
		memcpy ( &sa.sin_addr, tAgent.GetAddr(), tAgent.GetAddrLen() );
		sa.sin_family = (short)tAgent.GetAddrType();
		sa.sin_port = htons ( (unsigned short)tAgent.m_iPort );

		tAgent.m_iSock = socket ( tAgent.GetAddrType(), SOCK_STREAM, 0 );
		if ( tAgent.m_iSock<0 )
		{
			dFailures.Submit ( "socket() failed: %s", sphSockError() );
			return;
		}

		if ( sphSetSockNB ( tAgent.m_iSock )<0 )
		{
			dFailures.Submit ( "sphSetSockNB() failed: %s", sphSockError() );
			return;
		}

		if ( connect ( tAgent.m_iSock, (struct sockaddr*)&sa, sizeof(sa) )<0 )
		{
			int iErr = sphSockGetErrno();
			if ( iErr!=EINPROGRESS && iErr!=EINTR && iErr!=EWOULDBLOCK ) // check for EWOULDBLOCK is for winsock only
			{
				tAgent.Close ();
				dFailures.Submit ( "connect() failed: %s", sphSockError(iErr) );
				return;
			} else
			{
				// connection in progress
				tAgent.m_eState = AGENT_CONNECT;
			}
		} else
		{
			// socket connected, ready to read hello message
			tAgent.m_eState = AGENT_HELLO;
		}
	}
}


int QueryRemoteAgents ( CSphVector<Agent_t,8> & dAgents, int iTimeout, const IRequestBuilder_t & tBuilder, SearchFailuresLog_t & dFailures )
{
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
		ARRAY_FOREACH ( i, dAgents )
		{
			const Agent_t & tAgent = dAgents[i];
			if ( tAgent.m_eState==AGENT_CONNECT || tAgent.m_eState==AGENT_HELLO )
			{
				assert ( tAgent.m_iPort>0 );
				assert ( tAgent.m_iSock>0 );

				SPH_FD_SET ( tAgent.m_iSock, ( tAgent.m_eState==AGENT_CONNECT ) ? &fdsWrite : &fdsRead );
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

		ARRAY_FOREACH ( i, dAgents )
		{
			Agent_t & tAgent = dAgents[i];
			dFailures.m_sPrefix.SetSprintf ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );

			// check if connection completed
			if ( tAgent.m_eState==AGENT_CONNECT && FD_ISSET ( tAgent.m_iSock, &fdsWrite ) )
			{
				int iErr = 0;
				socklen_t iErrLen = sizeof(iErr);
				getsockopt ( tAgent.m_iSock, SOL_SOCKET, SO_ERROR, (char*)&iErr, &iErrLen );
				if ( iErr )
				{
					// connect() failure
					dFailures.Submit ( "connect() failed: %s", sphSockError(iErr) );
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
				int iRes = sphSockRecv ( tAgent.m_iSock, (char*)&iRemoteVer, sizeof(iRemoteVer) );
				iRemoteVer = ntohl ( iRemoteVer );
				if ( iRes!=sizeof(iRemoteVer) || iRemoteVer<=0 )
				{
					dFailures.Submit ( "expected protocol v.%d, got v.%d", SPHINX_SEARCHD_PROTO, iRemoteVer );
					tAgent.Close ();
					continue;
				}

				// send request
				NetOutputBuffer_c tOut ( tAgent.m_iSock );
				tBuilder.BuildRequest ( tAgent.m_sIndexes.cstr(), tOut );
				tOut.Flush (); // FIXME! handle flush failure?

				tAgent.m_eState = AGENT_QUERY;
				iAgents++;
			}
		}
	}

	ARRAY_FOREACH ( i, dAgents )
	{
		// check if connection timed out
		Agent_t & tAgent = dAgents[i];
		if ( tAgent.m_eState!=AGENT_QUERY && tAgent.m_eState!=AGENT_UNUSED )
		{
			tAgent.Close ();
			dFailures.m_sPrefix.SetSprintf ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );
			dFailures.Submit ( "%s() timed out", tAgent.m_eState==AGENT_HELLO ? "read" : "connect" );
		}
	}

	return iAgents;
}


int WaitForRemoteAgents ( CSphVector<Agent_t,8> & dAgents, int iTimeout,
	IReplyParser_t & tParser, SearchFailuresLog_t & dFailures )
{
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
		ARRAY_FOREACH ( iAgent, dAgents )
		{
			Agent_t & tAgent = dAgents[iAgent];
			if ( tAgent.m_eState==AGENT_QUERY || tAgent.m_eState==AGENT_REPLY )
			{
				assert ( tAgent.m_iPort>0 );
				assert ( tAgent.m_iSock>0 );

				SPH_FD_SET ( tAgent.m_iSock, &fdsRead );
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

		ARRAY_FOREACH ( iAgent, dAgents )
		{
			Agent_t & tAgent = dAgents[iAgent];
			if (!( tAgent.m_eState==AGENT_QUERY || tAgent.m_eState==AGENT_REPLY ))
				continue;
			if ( !FD_ISSET ( tAgent.m_iSock, &fdsRead ) )
				continue;

			// if there was no reply yet, read reply header
			dFailures.m_sPrefix.SetSprintf ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );
			bool bFailure = true;
			for ( ;; )
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

					if ( sphSockRecv ( tAgent.m_iSock, (char*)&tReplyHeader, sizeof(tReplyHeader) )!=sizeof(tReplyHeader) )
					{
						// bail out if failed
						dFailures.Submit ( "failed to receive reply header" );
						break;
					}

					tReplyHeader.m_iStatus = ntohs ( tReplyHeader.m_iStatus );
					tReplyHeader.m_iVer = ntohs ( tReplyHeader.m_iVer );
					tReplyHeader.m_iLength = ntohl ( tReplyHeader.m_iLength );

					// check the packet
					if ( tReplyHeader.m_iLength<0 || tReplyHeader.m_iLength>MAX_PACKET_SIZE ) // FIXME! add reasonable max packet len too
					{
						dFailures.Submit ( "invalid packet size (status=%d, len=%d, max_packet_size=%d)", tReplyHeader.m_iStatus, tReplyHeader.m_iLength, MAX_PACKET_SIZE );
						break;
					}

					// header received, switch the status
					assert ( tAgent.m_pReplyBuf==NULL );
					tAgent.m_eState = AGENT_REPLY;
					tAgent.m_pReplyBuf = new BYTE [ tReplyHeader.m_iLength ];
					tAgent.m_iReplySize = tReplyHeader.m_iLength;
					tAgent.m_iReplyRead = 0;
					tAgent.m_iReplyStatus = tReplyHeader.m_iStatus;

					if ( !tAgent.m_pReplyBuf )
					{
						// bail out if failed
						dFailures.Submit ( "failed to alloc %d bytes for reply buffer", tAgent.m_iReplySize );
						break;
					}
				}

				// if we are reading reply, read another chunk
				if ( tAgent.m_eState==AGENT_REPLY )
				{
					// do read
					assert ( tAgent.m_iReplyRead<tAgent.m_iReplySize );
					int iRes = sphSockRecv ( tAgent.m_iSock, (char*)tAgent.m_pReplyBuf+tAgent.m_iReplyRead,
						tAgent.m_iReplySize-tAgent.m_iReplyRead );

					// bail out if read failed
					if ( iRes<0 )
					{
						dFailures.Submit ( "failed to receive reply body: %s", sphSockError() );
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

					// check for general errors/warnings first
					if ( tAgent.m_iReplyStatus==SEARCHD_WARNING )
					{
						CSphString sAgentWarning = tReq.GetString ();
						dFailures.Submit ( "remote warning: %s", sAgentWarning.cstr() );

					} else if ( tAgent.m_iReplyStatus==SEARCHD_RETRY )
					{
						tAgent.m_eState = AGENT_RETRY;
						break;

					} else if ( tAgent.m_iReplyStatus!=SEARCHD_OK )
					{
						CSphString sAgentError = tReq.GetString ();
						dFailures.Submit ( "remote error: %s", sAgentError.cstr() );
						break;
					}

					// call parser
					if ( !tParser.ParseReply ( tReq, tAgent, dFailures ) )
						break;

					// check if there was enough data
					if ( tReq.GetError() )
					{
						dFailures.Submit ( "incomplete reply" );
						return false;
					}

					// all is well
					iAgents++;
					tAgent.Close ();
				}

				bFailure = false;
				break;
			}

			if ( bFailure )
			{
				tAgent.Close ();
				tAgent.m_tRes.m_dMatches.Reset ();
			}

			tAgent.m_bFailure = bFailure;
		}
	}

	// close timed-out agents
	ARRAY_FOREACH ( iAgent, dAgents )
	{
		Agent_t & tAgent = dAgents[iAgent];
		if ( tAgent.m_eState==AGENT_QUERY )
		{
			assert ( !tAgent.m_tRes.m_dMatches.GetLength() );
			tAgent.Close ();

			dFailures.m_sPrefix.SetSprintf ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );
			dFailures.Submit ( "query timed out" );
		}
	}

	return iAgents;
}

/////////////////////////////////////////////////////////////////////////////
// SEARCH HANDLER
/////////////////////////////////////////////////////////////////////////////

inline bool operator < ( const CSphMatch & a, const CSphMatch & b )
{
	if ( a.m_iDocID==b.m_iDocID )
		return a.m_iTag > b.m_iTag;
	else
		return a.m_iDocID < b.m_iDocID;
};

/////////////////////////////////////////////////////////////////////////////

struct SearchRequestBuilder_t : public IRequestBuilder_t
{
	SearchRequestBuilder_t ( const CSphQuery & tQuery ) : m_tQuery ( tQuery ) {}
	virtual void BuildRequest ( const char * sIndexes, NetOutputBuffer_c & tOut ) const;

protected:
	const CSphQuery & m_tQuery;
};


struct SearchReplyParser_t : public IReplyParser_t
{
	SearchReplyParser_t ( CSphQueryResult * pRes ) : m_pRes ( pRes ) {}
	virtual bool		ParseReply ( MemInputBuffer_c & tReq, Agent_t & tAgent, SearchFailuresLog_t & dFailures ) const;

protected:
	CSphQueryResult *	m_pRes;
};


void SearchRequestBuilder_t::BuildRequest ( const char * sIndexes, NetOutputBuffer_c & tOut ) const
{
	const CSphQuery & q = m_tQuery;
	int iReqSize = 84 + 2*sizeof(SphDocID_t) + 4*q.m_iWeights
		+ strlen ( q.m_sSortBy.cstr() )
		+ strlen ( q.m_sQuery.cstr() )
		+ strlen ( sIndexes )
		+ strlen ( q.m_sGroupBy.cstr() )
		+ strlen ( q.m_sGroupSortBy.cstr() )
		+ strlen ( q.m_sGroupDistinct.cstr() );
	ARRAY_FOREACH ( j, q.m_dFilters )
	{
		const CSphFilter & tFilter = q.m_dFilters[j];
		iReqSize +=
			12
			+ strlen ( tFilter.m_sAttrName.cstr() )
			+ 4*tFilter.m_iValues
			+ ( tFilter.m_iValues ? 0 : 8 );
	}

	// say hello
	tOut.SendDword ( SPHINX_SEARCHD_PROTO );

	// request header
	tOut.SendWord ( SEARCHD_COMMAND_SEARCH ); // command id
	tOut.SendWord ( VER_COMMAND_SEARCH ); // command version
	tOut.SendInt ( iReqSize-12 ); // request body length

	tOut.SendInt ( 0 ); // offset is 0
	tOut.SendInt ( q.m_iMaxMatches ); // limit is MAX_MATCHES
	tOut.SendInt ( (DWORD)q.m_eMode ); // match mode
	tOut.SendInt ( q.m_eSort ); // sort mode
	tOut.SendString ( q.m_sSortBy.cstr() ); // sort attr
	tOut.SendString ( q.m_sQuery.cstr() ); // query
	tOut.SendInt ( q.m_iWeights );
	for ( int j=0; j<q.m_iWeights; j++ )
		tOut.SendInt ( q.m_pWeights[j] ); // weights
	tOut.SendString ( sIndexes ); // indexes
	tOut.SendInt ( USE_64BIT ); // id range bits
	tOut.SendDocid ( q.m_iMinID ); // id/ts ranges
	tOut.SendDocid ( q.m_iMaxID );
	tOut.SendInt ( q.m_dFilters.GetLength() );
	ARRAY_FOREACH ( j, q.m_dFilters )
	{
		const CSphFilter & tFilter = q.m_dFilters[j];
		tOut.SendString ( tFilter.m_sAttrName.cstr() );
		tOut.SendInt ( tFilter.m_iValues );
		for ( int k=0; k<tFilter.m_iValues; k++ )
			tOut.SendInt ( tFilter.m_pValues[k] );
		if ( !tFilter.m_iValues )
		{
			tOut.SendDword ( tFilter.m_uMinValue );
			tOut.SendDword ( tFilter.m_uMaxValue );
		}
		tOut.SendInt ( tFilter.m_bExclude );
	}
	tOut.SendInt ( q.m_eGroupFunc );
	tOut.SendString ( q.m_sGroupBy.cstr() );
	tOut.SendInt ( q.m_iMaxMatches );
	tOut.SendString ( q.m_sGroupSortBy.cstr() );
	tOut.SendInt ( q.m_iCutoff );
	tOut.SendInt ( q.m_iRetryCount );
	tOut.SendInt ( q.m_iRetryDelay );
	tOut.SendString ( q.m_sGroupDistinct.cstr() );
}


bool SearchReplyParser_t::ParseReply ( MemInputBuffer_c & tReq, Agent_t & tAgent, SearchFailuresLog_t & dFailures ) const
{
	assert ( m_pRes );

	// get schema
	CSphSchema & tSchema = tAgent.m_tRes.m_tSchema;
	tSchema.Reset ();

	tSchema.m_dFields.Resize ( tReq.GetInt() ); // FIXME! add a sanity check
	ARRAY_FOREACH ( j, tSchema.m_dFields )
		tSchema.m_dFields[j].m_sName = tReq.GetString ();

	int iNumAttrs = tReq.GetInt(); // FIXME! add a sanity check
	for ( int j=0; j<iNumAttrs; j++ )
	{
		CSphColumnInfo tCol;
		tCol.m_sName = tReq.GetString ();
		tCol.m_eAttrType = tReq.GetDword (); // FIXME! add a sanity check
		tSchema.AddAttr ( tCol );
	}

	// get matches
	int iMatches = tReq.GetInt ();
	if ( iMatches<0 || iMatches>g_iMaxMatches )
	{
		dFailures.Submit ( "invalid match count received (count=%d)", iMatches );
		return false;
	}

	int bAgent64 = tReq.GetInt ();
#if !USE_64BIT
	if ( bAgent64 )
		dFailures.Submit ( "id64 agent, id32 master, docids might be wrapped" );
#endif

	assert ( !tAgent.m_tRes.m_dMatches.GetLength() );
	if ( iMatches )
	{
		tAgent.m_tRes.m_dMatches.Resize ( iMatches );
		ARRAY_FOREACH ( i, tAgent.m_tRes.m_dMatches )
		{
			CSphMatch & tMatch = tAgent.m_tRes.m_dMatches[i];
			tMatch.Reset ( tSchema.GetRowSize() );
			tMatch.m_iDocID = bAgent64 ? (SphDocID_t)tReq.GetUint64() : tReq.GetDword();
			tMatch.m_iWeight = tReq.GetInt ();
			for ( int j=0; j<tSchema.GetAttrsCount(); j++ )
			{
				const CSphColumnInfo & tAttr = tSchema.GetAttr(j);
				if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
				{
					tMatch.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount, g_dMvaStorage.GetLength() );

					int iValues = tReq.GetDword ();
					g_dMvaStorage.Add ( iValues );
					while ( iValues-- )
						g_dMvaStorage.Add ( tReq.GetDword() );

				} else
				{
					tMatch.SetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount, tReq.GetDword() );
				}
			}
		}
	}

	// read totals (retrieved count, total count, query time, word count)
	int iRetrieved = tReq.GetInt ();
	tAgent.m_tRes.m_iTotalMatches = tReq.GetInt ();
	tAgent.m_tRes.m_iQueryTime = tReq.GetInt ();
	tAgent.m_tRes.m_iNumWords = tReq.GetInt ();
	if ( iRetrieved!=iMatches )
	{
		dFailures.Submit ( "expected %d retrieved documents, got %d", iMatches, iRetrieved );
		return false;
	}

	bool bSetWords = false;
	if ( m_pRes->m_iNumWords && tAgent.m_tRes.m_iNumWords!=m_pRes->m_iNumWords )
	{
		dFailures.Submit ( "expected %d query words, got %d", m_pRes->m_iNumWords, tAgent.m_tRes.m_iNumWords );
		return false;
	}

	if ( !m_pRes->m_iNumWords )
	{
		m_pRes->m_iNumWords = tAgent.m_tRes.m_iNumWords;
		bSetWords = true;
	}

	// read words
	int i;
	for ( i=0; i<m_pRes->m_iNumWords; i++ )
	{
		CSphString sWord = tReq.GetString ();

		// set it in result if not yet, or check if already
		if ( bSetWords )
		{
			m_pRes->m_tWordStats[i].m_sWord = sWord;
		} else
		{
			if ( m_pRes->m_tWordStats[i].m_sWord!=sWord )
				break;
		}

		m_pRes->m_tWordStats[i].m_iDocs += tReq.GetInt (); // update docs count
		m_pRes->m_tWordStats[i].m_iHits += tReq.GetInt (); // update hits count
	}

	if ( i!=m_pRes->m_iNumWords )
	{
		dFailures.Submit ( "expected %d words, got %d", i, m_pRes->m_iNumWords );
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////

ISphMatchSorter * CreateSorter ( CSphQuery & tQuery, const CSphSchema & tSchema, const char * sServedName, InputBuffer_c & tReq )
{
	// lookup proper attribute index to group by, if any
	CSphString sError;
	if ( !tQuery.SetSchema ( tSchema, sError ) )
	{
		tReq.SendErrorReply ( "index '%s': %s", sServedName, sError.cstr() );
		return NULL;
	}
	assert ( tQuery.m_sGroupBy.IsEmpty() || tQuery.m_iGroupbyOffset>=0 );

	// spawn queue and set sort-by attribute
	ISphMatchSorter * pSorter = sphCreateQueue ( &tQuery, tSchema, sError );
	if ( !pSorter )
	{
		tReq.SendErrorReply ( "index '%s': failed to create sorting queue: %s",
			sServedName, sError.cstr() );
		return NULL;
	}

	return pSorter;
}


// returns true if dst was empty or both were equal; false otherwise
bool MinimizeSchema ( CSphSchema & tDst, const CSphSchema & tSrc )
{
	// if dst is empty, just copy it
	if ( tDst.GetAttrsCount()==0 )
	{
		tDst = tSrc;
		return true;
	}

	// check for equality, and remove all dst attributes that are not present in src
	CSphVector<CSphColumnInfo,8> dDst;
	for ( int i=0; i<tDst.GetAttrsCount(); i++ )
		dDst.Add ( tDst.GetAttr(i) );

	bool bEqual = ( tDst.GetAttrsCount()==tSrc.GetAttrsCount() );
	ARRAY_FOREACH ( i, dDst )
	{
		int iSrcIdx = tSrc.GetAttrIndex ( dDst[i].m_sName.cstr() );

		if ( iSrcIdx!=i )
			bEqual = false;

		if ( iSrcIdx==-1 )
		{
			dDst.Remove ( i );
			i--;
		}
	}

	tDst.ResetAttrs ();
	ARRAY_FOREACH ( i, dDst )
		tDst.AddAttr ( dDst[i] );

	return bEqual;
}


struct OldQuery_t 
{
	int			m_iVersion;
	int			m_iGroups;
	DWORD *		m_pGroups;
	DWORD		m_iMinTS;
	DWORD		m_iMaxTS;
	DWORD		m_iMinGID;
	DWORD		m_iMaxGID;

	OldQuery_t ( int iVer )
		: m_iVersion ( iVer )
		, m_iGroups ( 0 )
		, m_pGroups ( NULL )
		, m_iMinTS ( 0 )
		, m_iMaxTS ( UINT_MAX )
		, m_iMinGID ( 0 )
		, m_iMaxGID ( UINT_MAX )
	{}
};


bool FixupQuery ( CSphQuery * pQuery, OldQuery_t * pOldQuery,
	const CSphSchema * pSchema, const char * sIndexName, InputBuffer_c & tReq )
{
	// already?
	if ( !pOldQuery->m_iVersion )
		return true;

	if ( pOldQuery->m_iGroups>0 || pOldQuery->m_iMinGID!=0 || pOldQuery->m_iMaxGID!=UINT_MAX )
	{
		int iAttr = -1;
		for ( int i=0; i<pSchema->GetAttrsCount(); i++ ) 
			if ( pSchema->GetAttr(i).m_eAttrType==SPH_ATTR_INTEGER )
		{
			iAttr = i;
			break;
		}

		if ( iAttr<0 )
		{
			tReq.SendErrorReply ( "index '%s': no group attribute found", sIndexName );
			return false;
		}

		CSphFilter tFilter;
		tFilter.m_sAttrName = pSchema->GetAttr(iAttr).m_sName;
		tFilter.m_iValues = pOldQuery->m_iGroups;
		tFilter.m_pValues = pOldQuery->m_pGroups;
		tFilter.m_uMinValue = pOldQuery->m_iMinGID;
		tFilter.m_uMaxValue = pOldQuery->m_iMaxGID;
		pQuery->m_dFilters.Add ( tFilter );
	}

	if ( pOldQuery->m_iMinTS!=0 || pOldQuery->m_iMaxTS!=UINT_MAX )
	{
		int iAttr = -1;
		for ( int i=0; i<pSchema->GetAttrsCount(); i++ ) 
			if ( pSchema->GetAttr(i).m_eAttrType==SPH_ATTR_TIMESTAMP )
		{
			iAttr = i;
			break;
		}

		if ( iAttr<0 )
		{
			tReq.SendErrorReply ( "index '%s': no timestamp attribute found", sIndexName );
			return false;
		}

		CSphFilter tFilter;
		tFilter.m_sAttrName = pSchema->GetAttr(iAttr).m_sName;
		tFilter.m_uMinValue = pOldQuery->m_iMinTS;
		tFilter.m_uMaxValue = pOldQuery->m_iMaxTS;
		pQuery->m_dFilters.Add ( tFilter );
	}

	pOldQuery->m_iVersion = 0;
	return true;
}


void ParseIndexList ( const CSphString & sIndexes, CSphVector<CSphString,8> & dOut )
{
	CSphString sSplit = sIndexes;
	char * p = (char*)sSplit.cstr();
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

		dOut.Add ( sNext );
	}
}


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
	if ( iVer>VER_COMMAND_SEARCH )
	{
		tReq.SendErrorReply ( "client version is higher than daemon version (client is v.%d.%d, daemon is v.%d.%d)",
			iVer>>8, iVer&0xff, VER_COMMAND_SEARCH>>8, VER_COMMAND_SEARCH&0xff );
		return;
	}

	/////////////////
	// parse request
	/////////////////

	OldQuery_t tOldQuery ( iVer );

	// v.1.0. mode, limits, weights, ID/TS ranges
	tQuery.m_iOffset	= tReq.GetInt ();
	tQuery.m_iLimit		= tReq.GetInt ();
	tQuery.m_eMode		= (ESphMatchMode) tReq.GetInt ();
	tQuery.m_eSort		= (ESphSortOrder) tReq.GetInt ();
	if ( iVer<=0x101 )
		tOldQuery.m_iGroups = tReq.GetDwords ( &tOldQuery.m_pGroups, SEARCHD_MAX_ATTR_VALUES, "invalid group count %d (should be in 0..%d range)" );
	if ( iVer>=0x102 )
	{
		tQuery.m_sSortBy = tReq.GetString ();
		tQuery.m_sSortBy.ToLower ();
	}
	tQuery.m_sQuery		= tReq.GetString ();
	tQuery.m_iWeights	= tReq.GetDwords ( (DWORD**)&tQuery.m_pWeights, SPH_MAX_FIELDS, "invalid weight count %d (should be in 0..%d range)" );
	CSphString sIndexes	= tReq.GetString ();

	bool bIdrange64 = false;
	if ( iVer>=0x108 )
		bIdrange64 = ( tReq.GetInt()!=0 );

	if ( bIdrange64 )
	{
		tQuery.m_iMinID		= (SphDocID_t)tReq.GetUint64 ();
		tQuery.m_iMaxID		= (SphDocID_t)tReq.GetUint64 ();
		// FIXME? could report clamp here if I'm id32 and client passed id64 range,
		// but frequently this won't affect anything at all
	} else
	{
		tQuery.m_iMinID		= tReq.GetDword ();
		tQuery.m_iMaxID		= tReq.GetDword ();
	}

	if ( iVer<0x108 && tQuery.m_iMaxID==0xffffffffUL )
		tQuery.m_iMaxID = 0; // fixup older clients which send 32-bit UINT_MAX by default

	if ( tQuery.m_iMaxID==0 )
		tQuery.m_iMaxID = DOCID_MAX;

	// upto v.1.1
	if ( iVer<=0x101 )
	{
		tOldQuery.m_iMinTS = tReq.GetDword ();
		tOldQuery.m_iMaxTS = tReq.GetDword ();
	}

	// v.1.1 specific
	if ( iVer==0x101 )
	{
		tOldQuery.m_iMinGID = tReq.GetDword ();
		tOldQuery.m_iMaxGID = tReq.GetDword ();
	}
	// !COMMIT use min/max ts/gid

	// v.1.2
	if ( iVer>=0x102 )
	{
		int iAttrFilters = tReq.GetInt ();
		if ( iAttrFilters>SEARCHD_MAX_ATTRS )
		{
			tReq.SendErrorReply ( "too much attribute filters (req=%d, max=%d)", iAttrFilters, SEARCHD_MAX_ATTRS );
			return;
		}

		tQuery.m_dFilters.Resize ( iAttrFilters );
		ARRAY_FOREACH ( i, tQuery.m_dFilters )
		{
			CSphFilter & tFilter = tQuery.m_dFilters[i];
			tFilter.m_sAttrName = tReq.GetString ();
			tFilter.m_sAttrName.ToLower ();
			tFilter.m_iValues = tReq.GetDwords ( &tFilter.m_pValues, SEARCHD_MAX_ATTR_VALUES,
				"invalid attribute set length %d (should be in 0..%d range)" );
			if ( !tFilter.m_iValues )
			{
				// 0 length means this is range, not set
				tFilter.m_uMinValue = tReq.GetDword ();
				tFilter.m_uMaxValue = tReq.GetDword ();
			}
			if ( iVer>=0x106 )
				tFilter.m_bExclude = !!tReq.GetDword ();
		}
	}

	// v.1.3
	if ( iVer>=0x103 )
	{
		tQuery.m_eGroupFunc = (ESphGroupBy) tReq.GetDword ();
		tQuery.m_sGroupBy = tReq.GetString ();
		tQuery.m_sGroupBy.ToLower ();
	}

	// v.1.4
	tQuery.m_iMaxMatches = g_iMaxMatches;
	if ( iVer>=0x104 )
		tQuery.m_iMaxMatches = tReq.GetInt ();

	// v.1.5, v.1.7
	if ( iVer>=0x107 )
	{
		tQuery.m_sGroupSortBy = tReq.GetString ();
	} else if ( iVer>=0x105 )
	{
		bool bSortByGroup = ( tReq.GetInt()!=0 );
		if ( !bSortByGroup )
		{
			char sBuf[256];
			switch ( tQuery.m_eSort )
			{
				case SPH_SORT_RELEVANCE:
					tQuery.m_sGroupSortBy = "@weight desc";
					break;

				case SPH_SORT_ATTR_DESC:
				case SPH_SORT_ATTR_ASC:
					snprintf ( sBuf, sizeof(sBuf), "%s %s", tQuery.m_sSortBy.cstr(),
						tQuery.m_eSort==SPH_SORT_ATTR_ASC ? "asc" : "desc" );
					tQuery.m_sGroupSortBy = sBuf;
					break;

				case SPH_SORT_EXTENDED:	
					tQuery.m_sGroupSortBy = tQuery.m_sSortBy;
					break;

				default:
					tReq.SendErrorReply ( "INTERNAL ERROR: unsupported sort mode %d in groupby sort fixup", tQuery.m_eSort );
					return;
			}
		}
	}

	// v.1.9
	if ( iVer>=0x109 )
		tQuery.m_iCutoff = tReq.GetInt();

	// v.1.10
	if ( iVer>=0x10A )
	{
		tQuery.m_iRetryCount = tReq.GetInt ();
		tQuery.m_iRetryDelay = tReq.GetInt ();
	}

	// v.1.11
	if ( iVer>=0x10B )
		tQuery.m_sGroupDistinct = tReq.GetString ();

	/////////////////////
	// additional checks
	/////////////////////

	if ( tReq.GetError() )
	{
		tReq.SendErrorReply ( "invalid or truncated request" );
		return;
	}
	if ( tQuery.m_iMinID>tQuery.m_iMaxID )
	{
		tReq.SendErrorReply ( "invalid ID range (min greater than max)" );
		return;
	}
	if ( tQuery.m_eMode<0 || tQuery.m_eMode>SPH_MATCH_TOTAL )
	{
		tReq.SendErrorReply ( "invalid match mode %d", tQuery.m_eMode );
		return;
	}
	if ( tQuery.m_iMaxMatches<1 || tQuery.m_iMaxMatches>g_iMaxMatches )
	{
		tReq.SendErrorReply ( "per-query max_matches=%d out of bounds (per-server max_matches=%d)",
			tQuery.m_iMaxMatches, g_iMaxMatches );
		return;
	}
	if ( tQuery.m_iOffset<0 || tQuery.m_iOffset>=tQuery.m_iMaxMatches )
	{
		tReq.SendErrorReply ( "offset out of bounds (offset=%d, max_matches=%d)",
			tQuery.m_iOffset, tQuery.m_iMaxMatches );
		return;
	}
	if ( tQuery.m_iLimit<0 )
	{
		tReq.SendErrorReply ( "limit out of bounds (limit=%d)", tQuery.m_iLimit );
		return;
	}
	if ( tQuery.m_iCutoff<0 )
	{
		tReq.SendErrorReply ( "cutoff out of bounds (cutoff=%d)", tQuery.m_iCutoff );
		return;
	}
	if ( tQuery.m_iRetryCount<0 || tQuery.m_iRetryCount>MAX_RETRY_COUNT )
	{
		tReq.SendErrorReply ( "retry count out of bounds (count=%d)", tQuery.m_iRetryCount );
		return;
	}
	if ( tQuery.m_iRetryDelay<0 || tQuery.m_iRetryDelay>MAX_RETRY_DELAY )
	{
		tReq.SendErrorReply ( "retry delay out of bounds (delay=%d)", tQuery.m_iRetryDelay );
		return;
	}

	////////////////
	// do searching
	////////////////

	// do search
	float tmStart = sphLongTimer ();

	CSphQueryResult * pRes = new CSphQueryResult ();

	CSphVector < int, 8 > dMatchCounts;
	CSphVector < CSphSchema, 8 > dSchemas;

	SearchFailuresLog_t dFailures;

	int iTries = 0;
	int iSuccesses = 0;
	int iTag = 0;
	CSphVector<const DWORD *,8> dTag2MVA; // tag to mva storage ptr mapping

	if ( g_hDistIndexes(sIndexes) )
	{
		// search through requested distributed index
		DistributedIndex_t & tDist = g_hDistIndexes[sIndexes];
		iTries += tDist.m_dAgents.GetLength();

		tQuery.m_iRetryCount = Min ( Max ( tQuery.m_iRetryCount, 0 ), MAX_RETRY_COUNT ); // paranoid clamp
		for ( int iRetry=0; iRetry<=tQuery.m_iRetryCount; iRetry++ )
		{
			// delay between retries
			if ( iRetry>0 )
				sphUsleep ( tQuery.m_iRetryDelay );

			// connect to remote agents and query them
			dFailures.m_sIndex = sIndexes;
			ConnectToRemoteAgents ( tDist.m_dAgents, iRetry!=0, dFailures );

			SearchRequestBuilder_t tReqBuilder ( tQuery );
			int iRemote = QueryRemoteAgents ( tDist.m_dAgents, tDist.m_iAgentConnectTimeout, tReqBuilder, dFailures );

			// while the remote queries are running, do local searches
			// FIXME! what if the remote agents finish early, could they timeout?
			float tmQuery = 0.0f;
			if ( iRetry==0 )
			{
				if ( !iRemote && !tDist.m_dLocal.GetLength() )
				{
					tReq.SendErrorReply ( "all remote agents unreachable, no local indexes configured" );
					return;
				}

				tmQuery = -sphLongTimer ();
				ARRAY_FOREACH ( i, tDist.m_dLocal )
				{
					const ServedIndex_t & tServed = g_hIndexes [ tDist.m_dLocal[i] ];
					assert ( tServed.m_pIndex );
					assert ( tServed.m_pDict );
					assert ( tServed.m_pTokenizer );

					// create queue
					ISphMatchSorter * pSorter = CreateSorter ( tQuery, *tServed.m_pSchema, tDist.m_dLocal[i].cstr(), tReq );
					if ( !pSorter )
						return;

					// do query
					iTries++;
					if ( !tServed.m_pIndex->QueryEx ( tServed.m_pTokenizer, tServed.m_pDict, &tQuery, pRes, pSorter ) )
					{
						dFailures.m_sIndex = tDist.m_dLocal[i];
						dFailures.m_sPrefix = "";
						dFailures.Submit ( "%s", tServed.m_pIndex->GetLastError().cstr() );
					} else
						iSuccesses++;

					// extract my results and store schema
					if ( pSorter->GetLength() )
					{
						dMatchCounts.Add ( pSorter->GetLength() );
						dSchemas.Add ( pRes->m_tSchema );
						dTag2MVA.Add ( pRes->m_pMva );
						sphFlattenQueue ( pSorter, pRes, iTag++ );
					}
					SafeDelete ( pSorter );
				}
				tmQuery += sphLongTimer ();
			}

			// wait for remote queries to complete
			if ( iRemote )
			{
				dFailures.m_sIndex = sIndexes;
				SearchReplyParser_t tParser ( pRes );
				int iMsecLeft = tDist.m_iAgentQueryTimeout - int(tmQuery*1000.0f);
				int iReplys = WaitForRemoteAgents ( tDist.m_dAgents, Max(iMsecLeft,0), tParser, dFailures );

				// check if there were valid (though might be 0-matches) replys, and merge them
				iSuccesses += iReplys;
				if ( iReplys )
					ARRAY_FOREACH ( iAgent, tDist.m_dAgents )
				{
					Agent_t & tAgent = tDist.m_dAgents[iAgent];
					if ( tAgent.m_bFailure || !tAgent.m_tRes.m_dMatches.GetLength() )
						continue;

					// merge this agent's results
					ARRAY_FOREACH ( i, tAgent.m_tRes.m_dMatches )
					{
						pRes->m_dMatches.Add ( tAgent.m_tRes.m_dMatches[i] );
						pRes->m_dMatches.Last().m_iTag = iTag;
					}

					dMatchCounts.Add ( tAgent.m_tRes.m_dMatches.GetLength() );
					dSchemas.Add ( tAgent.m_tRes.m_tSchema );

					tAgent.m_tRes.m_dMatches.Reset ();

					// merge this agent's stats
					pRes->m_iTotalMatches += tAgent.m_tRes.m_iTotalMatches;
				}
			}

			// check if we need to retry again
			int iToRetry = 0;
			ARRAY_FOREACH ( i, tDist.m_dAgents )
				if ( tDist.m_dAgents[i].m_eState==AGENT_RETRY )
					iToRetry++;
			if ( !iToRetry )
				break;
		}

		assert ( iTag==dTag2MVA.GetLength() );
		dTag2MVA.Add ( g_dMvaStorage.GetLength() ? &g_dMvaStorage[0] : NULL );

	} else if ( sIndexes=="*" )
	{
		// search through all local indexes
		g_hIndexes.IterateStart ();
		while ( g_hIndexes.IterateNext () )
		{
			const ServedIndex_t & tServed = g_hIndexes.IterateGet ();
			if ( !tServed.m_bEnabled )
				continue;

			assert ( tServed.m_pIndex );
			assert ( tServed.m_pDict );
			assert ( tServed.m_pTokenizer );

			const char * sIndexName = g_hIndexes.IterateGetKey().cstr();

			// fixup old queries
			if ( !FixupQuery ( &tQuery, &tOldQuery, tServed.m_pSchema, sIndexName, tReq ) )
				return;

			// create queue
			ISphMatchSorter * pSorter = CreateSorter ( tQuery, *tServed.m_pSchema, sIndexName, tReq );
			if ( !pSorter )
				return;

			// do query
			iTries++;
			if ( !tServed.m_pIndex->QueryEx ( tServed.m_pTokenizer, tServed.m_pDict, &tQuery, pRes, pSorter ) )
			{
				dFailures.m_sIndex = sIndexName;
				dFailures.m_sPrefix = "";
				dFailures.Submit ( "%s", tServed.m_pIndex->GetLastError().cstr() );
			} else
				iSuccesses++;

			// extract my results and store schema
			if ( pSorter->GetLength() )
			{
				dMatchCounts.Add ( pSorter->GetLength() );
				dSchemas.Add ( pRes->m_tSchema );
				dTag2MVA.Add ( pRes->m_pMva );
				sphFlattenQueue ( pSorter, pRes, iTag++ );
			}
			SafeDelete ( pSorter );
		}

		if ( !iTries )
		{
			tReq.SendErrorReply ( "no local indexes configured" );
			return;
		}

	} else
	{
		// search through the requested local indexes
		CSphVector<CSphString,8> dIndexNames;
		ParseIndexList ( sIndexes, dIndexNames );

		int iDisabled = 0;
		ARRAY_FOREACH ( i, dIndexNames )
		{
			const char * sIndexName = dIndexNames[i].cstr();

			// check that this one exists
			if ( !g_hIndexes.Exists(sIndexName) )
			{
				tReq.SendErrorReply ( "unknown local index '%s' in search request", sIndexName );
				return;
			}

			const ServedIndex_t & tServed = g_hIndexes[sIndexName];
			if ( !tServed.m_bEnabled )
			{
				iDisabled++;
				continue;
			}
			assert ( tServed.m_pIndex );
			assert ( tServed.m_pDict );
			assert ( tServed.m_pTokenizer );

#if 0
			CSphCache tCache ( g_sCacheDir.cstr(), g_iCacheTTL, g_bCacheGzip );
			if ( g_bCacheEnable )
				if ( tCache.ReadFromFile ( tQuery, sNext, tServed.m_pIndexPath->cstr(), pRes ) )
					continue;
#endif

			// fixup old queries
			if ( !FixupQuery ( &tQuery, &tOldQuery, tServed.m_pSchema, sIndexName, tReq ) )
				return;

			// create queue
			ISphMatchSorter * pSorter = CreateSorter ( tQuery, *tServed.m_pSchema, sIndexName, tReq );
			if ( !pSorter )
				return;

			// do query
			iTries++;
			if ( !tServed.m_pIndex->QueryEx ( tServed.m_pTokenizer, tServed.m_pDict, &tQuery, pRes, pSorter ) )
			{
				dFailures.m_sIndex = sIndexName;
				dFailures.m_sPrefix = "";
				dFailures.Submit ( "%s", tServed.m_pIndex->GetLastError().cstr() );
			} else
				iSuccesses++;

			// extract my results and store schema
			if ( pSorter->GetLength() )
			{
				dMatchCounts.Add ( pSorter->GetLength() );
				dSchemas.Add ( pRes->m_tSchema );
				dTag2MVA.Add ( pRes->m_pMva );
				sphFlattenQueue ( pSorter, pRes, iTag++ );
			}
			SafeDelete ( pSorter );

#if 0
			if ( g_bCacheEnable )
				tCache.StoreResult ( tQuery, sNext, pRes );
#endif
		}

		if ( !iTries )
		{
			if ( iDisabled )
				tReq.SendErrorReply ( "no available indexes in request (disabled=%d)", iDisabled );
			else
				tReq.SendErrorReply ( "no valid indexes in request" );
			return;
		}
	}

	// warn about id64 server vs old clients
#if USE_64BIT
	if ( iVer<0x108 )
	{
		dFailures.m_sIndex = "*";
		dFailures.m_sPrefix = "";
		dFailures.Submit ( "searchd is id64; resulting docids might be wrapped" );
	}
#endif

	// if there were no succesful searches at all, this is an error
	assert ( iTries );
	if ( !iSuccesses )
	{
		StrBuf_t sFailures;
		dFailures.BuildReport ( sFailures );

		tReq.SendErrorReply ( "%s", sFailures.cstr() );
		return;
	}

	////////////////
	// remove dupes
	////////////////

	if ( dSchemas.GetLength() )
		pRes->m_tSchema = dSchemas[0];

	while ( iSuccesses>1 )
	{
		// sanity check
		int iExpected = 0;
		ARRAY_FOREACH ( i, dMatchCounts )
			iExpected += dMatchCounts[i];

		if ( iExpected!=pRes->m_dMatches.GetLength() )
		{
			tReq.SendErrorReply ( "INTERNAL ERROR: expected %d matches in combined result set, got %d",
				iExpected, pRes->m_dMatches.GetLength() );
			return;
		}

		if ( !pRes->m_dMatches.GetLength() )
			break;

		// build minimal schema
		bool bAllEqual = true;
		pRes->m_tSchema.Reset();
		ARRAY_FOREACH ( i, dSchemas )
			if ( !MinimizeSchema ( pRes->m_tSchema, dSchemas[i] ) )
				bAllEqual = false;

		// convert all matches to minimal schema
		if ( !bAllEqual )
		{
			int iCur = 0;
			int * dMapFrom = NULL;

			CSphDocInfo tRow;
			tRow.Reset ( pRes->m_tSchema.GetRowSize() );

			if ( tRow.m_iRowitems )
				dMapFrom = new int [ pRes->m_tSchema.GetAttrsCount() ];

			ARRAY_FOREACH ( iSchema, dSchemas )
			{
				assert ( tRow.m_iRowitems<=dSchemas[iSchema].GetRowSize() );

				for ( int i=0; i<pRes->m_tSchema.GetAttrsCount(); i++ ) 
				{
					dMapFrom[i] = dSchemas[iSchema].GetAttrIndex ( pRes->m_tSchema.GetAttr(i).m_sName.cstr() );
					assert ( dMapFrom[i]>=0 );
				}

				for ( int i=iCur; i<iCur+dMatchCounts[iSchema]; i++ )
				{
					CSphMatch & tMatch = pRes->m_dMatches[i];
					assert ( tMatch.m_iRowitems>=tRow.m_iRowitems );

					if ( tRow.m_iRowitems )
					{
						for ( int j=0; j<pRes->m_tSchema.GetAttrsCount(); j++ ) 
						{
							const CSphColumnInfo & tDst = pRes->m_tSchema.GetAttr(j);
							const CSphColumnInfo & tSrc = dSchemas[iSchema].GetAttr ( dMapFrom[j] );

							tRow.SetAttr ( tDst.m_iBitOffset, tDst.m_iBitCount,
								tMatch.GetAttr ( tSrc.m_iBitOffset, tSrc.m_iBitCount ) );
						}

						for ( int j=0; j<tRow.m_iRowitems; j++ )
							tMatch.m_pRowitems[j] = tRow.m_pRowitems[j];
					}
					tMatch.m_iRowitems = tRow.m_iRowitems;
				}

				iCur += dMatchCounts[iSchema];
			}

			assert ( iCur==pRes->m_dMatches.GetLength() );
			SafeDeleteArray ( dMapFrom );
		}

		// create queue
		ISphMatchSorter * pSorter = CreateSorter ( tQuery, pRes->m_tSchema, "(remove-dupes)", tReq );
		if ( !pSorter )
			return;

		// sort by docid and then by tag
		pRes->m_dMatches.Sort ();

		// kill all dupes
		int iDupes = 0;
		if ( tQuery.m_iGroupbyOffset>=0 )
		{
			// groupby sorters does that automagically
			ARRAY_FOREACH ( i, pRes->m_dMatches )
				if ( !pSorter->Push ( pRes->m_dMatches[i] ) )
					iDupes++;
		} else
		{
			// normal sorter needs massasging
			ARRAY_FOREACH ( i, pRes->m_dMatches )
			{
				if ( i==0 || pRes->m_dMatches[i].m_iDocID!=pRes->m_dMatches[i-1].m_iDocID )
					pSorter->Push ( pRes->m_dMatches[i] );
				else
					iDupes++;
			}
		}

		pRes->m_dMatches.Reset ();
		sphFlattenQueue ( pSorter, pRes, -1 );
		SafeDelete ( pSorter );

		pRes->m_iTotalMatches -= iDupes;
		break;
	}

	pRes->m_iQueryTime = int ( 1000.0f*( sphLongTimer() - tmStart ) );

	/////////////
	// log query
	/////////////

	if ( g_iQueryLogFile>=0 )
	{
		time_t tNow;
		char sTimeBuf[128], sGroupBuf[128];
		char sBuf[1024];

		time ( &tNow );
		ctime_r ( &tNow, sTimeBuf );
		sTimeBuf [ strlen(sTimeBuf)-1 ] = '\0';

		sGroupBuf[0] = '\0';
		if ( tQuery.m_iGroupbyOffset>=0 )
			snprintf ( sGroupBuf, sizeof(sGroupBuf), " @%s", tQuery.m_sGroupBy.cstr() );
 
		static const char * sModes [ SPH_MATCH_TOTAL ] = { "all", "any", "phr", "bool", "ext" };
		static const char * sSort [ SPH_SORT_TOTAL ] = { "rel", "attr-", "attr+", "tsegs", "ext" };

		snprintf ( sBuf, sizeof(sBuf), "[%s] %d.%03d sec [%s/%d/%s %d (%d,%d)%s] [%s] %s\n",
			sTimeBuf, pRes->m_iQueryTime/1000, pRes->m_iQueryTime%1000,
			sModes [ tQuery.m_eMode ], tQuery.m_dFilters.GetLength(), sSort [ tQuery.m_eSort ],
			pRes->m_iTotalMatches, tQuery.m_iOffset, tQuery.m_iLimit, sGroupBuf,
			sIndexes.cstr(), tQuery.m_sQuery.cstr() );

		sphLockEx ( g_iQueryLogFile, true );
		lseek ( g_iQueryLogFile, 0, SEEK_END );
		write ( g_iQueryLogFile, sBuf, strlen(sBuf) );
		sphLockUn ( g_iQueryLogFile );
	}

	//////////////////////
	// serve the response
	//////////////////////

	// build report
	StrBuf_t sFailures;
	dFailures.BuildReport ( sFailures );

	// calc response length
	int iRespLen = 20; // header

	if ( iVer>=0x102 ) // schema
	{
		iRespLen += 8; // 4 for field count, 4 for attr count
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dFields )
			iRespLen += 4 + strlen ( pRes->m_tSchema.m_dFields[i].m_sName.cstr() ); // namelen, name
		for ( int i=0; i<pRes->m_tSchema.GetAttrsCount(); i++ )
			iRespLen += 8 + strlen ( pRes->m_tSchema.GetAttr(i).m_sName.cstr() ); // namelen, name, type
	}

	int iCount = Max ( Min ( tQuery.m_iLimit, pRes->m_dMatches.GetLength()-tQuery.m_iOffset ), 0 );
	if ( iVer<0x102 )
		iRespLen += 16*iCount; // matches
	else if ( iVer<0x108 )
		iRespLen += ( 8+4*pRes->m_tSchema.GetAttrsCount() )*iCount; // matches
	else
		iRespLen += 4 + ( 8+4*USE_64BIT+4*pRes->m_tSchema.GetAttrsCount() )*iCount; // id64 tag and matches

	for ( int i=0; i<pRes->m_iNumWords; i++ ) // per-word stats
		iRespLen += 12 + strlen ( pRes->m_tWordStats[i].m_sWord.cstr() ); // wordlen, word, docs, hits

	bool bWarning = ( iVer>=0x106 && !dFailures.IsEmpty() );
	if ( bWarning )
		iRespLen += 4 + strlen ( sFailures.cstr() );

	CSphVector<int,8> dMvaItems;
	for ( int i=0; i<pRes->m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = pRes->m_tSchema.GetAttr(i);
		if ( tCol.m_eAttrType & SPH_ATTR_MULTI )
		{
			assert ( tCol.m_iRowitem>=0 );
			dMvaItems.Add ( tCol.m_iRowitem );
		}
	}

	if ( iVer>=0x10C && dMvaItems.GetLength() )
	{
		for ( int i=0; i<iCount; i++ )
		{
			const CSphMatch & tMatch = pRes->m_dMatches[tQuery.m_iOffset+i];
			const DWORD * pMva = dTag2MVA [ tMatch.m_iTag ];
			ARRAY_FOREACH ( j, dMvaItems )
				iRespLen += 4*pMva [ tMatch.GetAttr ( dMvaItems[j] ) ]; // FIXME? maybe add some sanity check here
		}
	}

	// send header
	NetOutputBuffer_c tOut ( iSock );
	tOut.SendWord ( (WORD)( bWarning ? SEARCHD_WARNING : SEARCHD_OK ) );
	tOut.SendWord ( VER_COMMAND_SEARCH );
	tOut.SendInt ( iRespLen );

	// send warning
	if ( bWarning )
		tOut.SendString ( sFailures.cstr() );

	// send schema
	if ( iVer>=0x102 )
	{
		tOut.SendInt ( pRes->m_tSchema.m_dFields.GetLength() );
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dFields )
			tOut.SendString ( pRes->m_tSchema.m_dFields[i].m_sName.cstr() );

		tOut.SendInt ( pRes->m_tSchema.GetAttrsCount() );
		for ( int i=0; i<pRes->m_tSchema.GetAttrsCount(); i++ )
		{
			tOut.SendString ( pRes->m_tSchema.GetAttr(i).m_sName.cstr() );
			tOut.SendDword ( (DWORD)pRes->m_tSchema.GetAttr(i).m_eAttrType );
		}
	}

	// send matches
	int iGIDOffset = -1, iGIDCount = -1;
	int iTSOffset = -1, iTSCount = -1;
	if ( iVer<=0x101 )
	{
		for ( int i=0; i<pRes->m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tAttr = pRes->m_tSchema.GetAttr(i);
			if ( iTSOffset<0 && tAttr.m_eAttrType==SPH_ATTR_TIMESTAMP )
			{
				iTSOffset = tAttr.m_iBitOffset;
				iTSCount = tAttr.m_iBitCount;
			}
			if ( iGIDOffset<0 && tAttr.m_eAttrType==SPH_ATTR_INTEGER )
			{
				iGIDOffset = tAttr.m_iBitOffset;
				iGIDCount = tAttr.m_iBitCount;
			}
		}
	}

	tOut.SendInt ( iCount );
	if ( iVer>=0x108 )
		tOut.SendInt ( USE_64BIT );

	for ( int i=0; i<iCount; i++ )
	{
		const CSphMatch & tMatch = pRes->m_dMatches[tQuery.m_iOffset+i];
#if USE_64BIT
		if ( iVer>=0x108 )
			tOut.SendUint64 ( tMatch.m_iDocID );
		else
#endif
			tOut.SendDword ( (DWORD)tMatch.m_iDocID );

		if ( iVer<=0x101 )
		{
			tOut.SendDword ( iGIDOffset>=0 ? tMatch.GetAttr ( iGIDOffset, iGIDCount ) : 1 );
			tOut.SendDword ( iTSOffset>=0 ? tMatch.GetAttr ( iTSOffset, iTSCount ) : 1 );
			tOut.SendInt ( tMatch.m_iWeight );
		} else
		{
			tOut.SendInt ( tMatch.m_iWeight );

			const DWORD * pMva = dTag2MVA [ tMatch.m_iTag ];

			assert ( tMatch.m_iRowitems==pRes->m_tSchema.GetRowSize() );
			for ( int j=0; j<pRes->m_tSchema.GetAttrsCount(); j++ )
			{
				const CSphColumnInfo & tAttr = pRes->m_tSchema.GetAttr(j);
				if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
				{
					if ( iVer>=0x10C )
					{
						// send MVA values
						const DWORD * pValues = pMva + tMatch.GetAttr ( tAttr.m_iRowitem );
						int iValues = *pValues++;

						tOut.SendDword ( iValues );
						while ( iValues-- )
							tOut.SendDword ( *pValues++ );

					} else
					{
						// for older clients, fixup MVA to 0
						tOut.SendDword ( 0 );
					}
				} else
				{
					// send plain attr
					tOut.SendDword ( tMatch.GetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount ) );
				}
			}
		}
	}
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

	SafeDelete ( pRes );
}

/////////////////////////////////////////////////////////////////////////////
// EXCERPTS HANDLER
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

	tReq.GetInt (); // mode field is for now reserved and ignored
	int iFlags = tReq.GetInt ();
	CSphString sIndex = tReq.GetString ();

	if ( !g_hIndexes(sIndex) )
	{
		tReq.SendErrorReply ( "unknown local index '%s' in search request", sIndex.cstr() );
		return;
	}
	CSphDict * pDict = g_hIndexes[sIndex].m_pDict;
	ISphTokenizer * pTokenizer = g_hIndexes[sIndex].m_pTokenizer;

	q.m_sWords = tReq.GetString ();
	q.m_sBeforeMatch = tReq.GetString ();
	q.m_sAfterMatch = tReq.GetString ();
	q.m_sChunkSeparator = tReq.GetString ();
	q.m_iLimit = tReq.GetInt ();
	q.m_iAround = tReq.GetInt ();

	q.m_bRemoveSpaces = ( iFlags & EXCERPT_FLAG_REMOVESPACES );

	int iCount = tReq.GetInt ();
	if ( iCount<0 || iCount>EXCERPT_MAX_ENTRIES )
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
		dExcerpts.Add ( sphBuildExcerpt ( q, pDict, pTokenizer ) );
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

/////////////////////////////////////////////////////////////////////////////
// UPDATES HANDLER
/////////////////////////////////////////////////////////////////////////////

struct UpdateRequestBuilder_t : public IRequestBuilder_t
{
	UpdateRequestBuilder_t ( const CSphAttrUpdate_t & pUpd ) : m_tUpd ( pUpd ) {}
	virtual void BuildRequest ( const char * sIndexes, NetOutputBuffer_c & tOut ) const;

protected:
	const CSphAttrUpdate_t & m_tUpd;
};


struct UpdateReplyParser_t : public IReplyParser_t
{
	UpdateReplyParser_t ( int * pUpd )
		: m_pUpdated ( pUpd )
	{}

	virtual bool ParseReply ( MemInputBuffer_c & tReq, Agent_t &, SearchFailuresLog_t & ) const
	{
		*m_pUpdated += tReq.GetDword ();
		return true;
	}

protected:
	int * m_pUpdated;
};


void UpdateRequestBuilder_t::BuildRequest ( const char * sIndexes, NetOutputBuffer_c & tOut ) const
{
	int iReqSize = 4+strlen(sIndexes); // indexes string
	iReqSize += 4; // attrs array len, data
	ARRAY_FOREACH ( i, m_tUpd.m_dAttrs )
		iReqSize += 4+strlen(m_tUpd.m_dAttrs[i].m_sName.cstr());
	iReqSize += 4; // values array len, data
	iReqSize += 4*( 1+m_tUpd.m_dAttrs.GetLength() )*m_tUpd.m_iUpdates;

	// header
	tOut.SendDword ( SPHINX_SEARCHD_PROTO );
	tOut.SendWord ( SEARCHD_COMMAND_UPDATE );
	tOut.SendWord ( VER_COMMAND_UPDATE );
	tOut.SendInt ( iReqSize );

	tOut.SendString ( sIndexes );
	tOut.SendInt ( m_tUpd.m_dAttrs.GetLength() );
	ARRAY_FOREACH ( i, m_tUpd.m_dAttrs )
		tOut.SendString ( m_tUpd.m_dAttrs[i].m_sName.cstr() );
	tOut.SendInt ( m_tUpd.m_iUpdates );

	DWORD * pUpd = m_tUpd.m_pUpdates;
	DWORD * pUpdMax = pUpd + ( DOCINFO_IDSIZE+m_tUpd.m_dAttrs.GetLength() )*m_tUpd.m_iUpdates;
	for ( ; pUpd<pUpdMax; )
	{
		tOut.SendDword ( *pUpd++ );
		#if USE_64BIT
		pUpd++;
		#endif

		for ( int j=0; j<m_tUpd.m_dAttrs.GetLength(); j++ )
			tOut.SendDword ( *pUpd++ );
	}
	assert ( pUpd==pUpdMax );
}


void HandleCommandUpdate ( int iSock, int iVer, InputBuffer_c & tReq, int iPipeFD )
{
	// check major command version
	if ( (iVer>>8)!=(VER_COMMAND_UPDATE>>8) )
	{
		tReq.SendErrorReply ( "major command version mismatch (expected v.%d.x, got v.%d.%d)",
			VER_COMMAND_UPDATE>>8, iVer>>8, iVer&0xff );
		return;
	}

	// get index name
	CSphString sIndexes = tReq.GetString ();

	// get update data
	CSphAttrUpdate_t tUpd;
	tUpd.m_dAttrs.Resize ( tReq.GetDword() ); // FIXME! check this
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
		tUpd.m_dAttrs[i].m_sName = tReq.GetString ();

	int iStride = DOCINFO_IDSIZE + tUpd.m_dAttrs.GetLength();

	tUpd.m_iUpdates = tReq.GetInt (); // FIXME! check this
	tUpd.m_pUpdates = new DWORD [ tUpd.m_iUpdates*iStride ];

	int iPos = 0;
	for ( int i=0; i<tUpd.m_iUpdates; i++ )
	{
		tUpd.m_pUpdates[iPos++] = tReq.GetDword();
		#if USE_64BIT
		tUpd.m_pUpdates[iPos++] = 0;
		#endif

		ARRAY_FOREACH ( j, tUpd.m_dAttrs )
			tUpd.m_pUpdates[iPos++] = tReq.GetDword();
	}
	if ( tReq.GetError() )
	{
		tReq.SendErrorReply ( "invalid or truncated request" );
		return;
	}

	// check index names
	CSphVector<CSphString,8> dIndexNames;
	ParseIndexList ( sIndexes, dIndexNames );

	if ( !dIndexNames.GetLength() )
	{
		tReq.SendErrorReply ( "no valid indexes in update request" );
		return;
	}

	ARRAY_FOREACH ( i, dIndexNames )
	{
		if ( !g_hIndexes(dIndexNames[i]) && !g_hDistIndexes(dIndexNames[i]) )
		{
			tReq.SendErrorReply ( "unknown index '%s' in update request", dIndexNames[i].cstr() );
			return;
		}
	}

	// do update
	SearchFailuresLog_t dFailures;
	int iSuccesses = 0;
	int iUpdated = 0;
	CSphVector<CSphString,8> dUpdated;

	ARRAY_FOREACH ( iIdx, dIndexNames )
	{
		const char * sReqIndex = dIndexNames[iIdx].cstr();

		CSphVector<CSphString,8> dLocal;
		const CSphVector<CSphString,8> * pLocal = NULL;

		if ( g_hIndexes(sReqIndex) )
		{
			dLocal.Add ( sReqIndex );
			pLocal = &dLocal;
		} else
		{
			assert ( g_hDistIndexes(sReqIndex) );
			pLocal = &g_hDistIndexes[sReqIndex].m_dLocal;
		}

		// update local indexes
		assert ( pLocal );
		ARRAY_FOREACH ( i, (*pLocal) )
		{
			const char * sIndex = (*pLocal)[i].cstr();
			ServedIndex_t * pServed = g_hIndexes(sIndex);

			dFailures.m_sIndex = sIndex;
			dFailures.m_sPrefix = "";

			if ( !pServed || !pServed->m_pIndex )
			{
				dFailures.Submit ( "index not available" );
				continue;
			}

			int iUpd = pServed->m_pIndex->UpdateAttributes ( tUpd );
			if ( iUpd<0 )
			{
				dFailures.Submit ( "%s", pServed->m_pIndex->GetLastError().cstr() );

			} else
			{
				iUpdated += iUpd;
				dUpdated.Add ( sIndex );
				iSuccesses++;
			}
		}

		// update remote agents
		if ( g_hDistIndexes(sReqIndex) )
		{
			DistributedIndex_t & tDist = g_hDistIndexes[sReqIndex];
			dFailures.m_sIndex = sReqIndex;

			// connect to remote agents and query them
			ConnectToRemoteAgents ( tDist.m_dAgents, false, dFailures );

			UpdateRequestBuilder_t tReqBuilder ( tUpd );
			int iRemote = QueryRemoteAgents ( tDist.m_dAgents, tDist.m_iAgentConnectTimeout, tReqBuilder, dFailures );

			if ( iRemote )
			{
				UpdateReplyParser_t tParser ( &iUpdated );
				iSuccesses += WaitForRemoteAgents ( tDist.m_dAgents, tDist.m_iAgentQueryTimeout, tParser, dFailures );
			}
		}
	}

	// notify head daemon of local updates
	if ( iPipeFD>=0 )
	{
		DWORD uTmp = SPH_PIPE_UPDATED_ATTRS;
		::write ( iPipeFD, &uTmp, sizeof(DWORD) );

		uTmp = dUpdated.GetLength();
		::write ( iPipeFD, &uTmp, sizeof(DWORD) );

		ARRAY_FOREACH ( i, dUpdated )
		{
			uTmp = strlen ( dUpdated[i].cstr() );
			::write ( iPipeFD, &uTmp, sizeof(DWORD) );
			::write ( iPipeFD, dUpdated[i].cstr(), uTmp );
		}
	}

	// serve reply to client
	StrBuf_t sReport;
	dFailures.BuildReport ( sReport );

	if ( !iSuccesses )
	{
		tReq.SendErrorReply ( "%s", sReport.cstr() );
		return;
	}

	NetOutputBuffer_c tOut ( iSock );
	if ( dFailures.IsEmpty() )
	{
		tOut.SendWord ( SEARCHD_OK );
		tOut.SendWord ( VER_COMMAND_UPDATE );
		tOut.SendInt ( 4 );
	} else
	{
		tOut.SendWord ( SEARCHD_WARNING );
		tOut.SendWord ( VER_COMMAND_UPDATE );
		tOut.SendInt ( 8+strlen(sReport.cstr()) );
		tOut.SendString ( sReport.cstr() );
	}
	tOut.SendInt ( iUpdated );
	tOut.Flush ();
}

/////////////////////////////////////////////////////////////////////////////
// GENERAL HANDLER
/////////////////////////////////////////////////////////////////////////////

void SafeClose ( int & iFD )
{
	if ( iFD>=0 )
		::close ( iFD );
	iFD = -1;
}


void HandleClient ( int iSock, const char * sClientIP, int iPipeFD )
{
	NetInputBuffer_c tBuf ( iSock );

	// send my version
	DWORD uServer = SPHINX_SEARCHD_PROTO;
	if ( sphSockSend ( iSock, (char*)&uServer, sizeof(DWORD) )!=sizeof(DWORD) )
	{
		sphWarning ( "failed to send server version (client=%s)", sClientIP );
		return;
	}

	// get client version and request
	tBuf.ReadFrom ( 12 ); // FIXME! magic
	tBuf.GetInt (); // client version is for now unused
	int iCommand = tBuf.GetWord ();
	int iCommandVer = tBuf.GetWord ();
	int iLength = tBuf.GetInt ();
	if ( tBuf.GetError() )
	{
		sphWarning ( "failed to receive client version and request (client=%s, error=%s)", sClientIP, sphSockError() );
		return;
	}

	// check request
	if ( iCommand<0 || iCommand>=SEARCHD_COMMAND_TOTAL
		|| iLength<=0 || iLength>MAX_PACKET_SIZE )
	{
		// unknown command, default response header
		tBuf.SendErrorReply ( "unknown command (code=%d)", iCommand );

		// if request length is insane, low level comm is broken, so we bail out
		if ( iLength<=0 || iLength>MAX_PACKET_SIZE )
		{
			sphWarning ( "ill-formed client request (length=%d out of bounds)", iLength );
			return;
		}
	}

	// get request body
	assert ( iLength>0 && iLength<=MAX_PACKET_SIZE );
	if ( !tBuf.ReadFrom ( iLength ) )
	{
		sphWarning ( "failed to receive client request body (client=%s)", sClientIP );
		return;
	}

	// handle known commands
	assert ( iCommand>=0 && iCommand<SEARCHD_COMMAND_TOTAL );
	switch ( iCommand )
	{
		case SEARCHD_COMMAND_SEARCH:	SafeClose ( iPipeFD ); HandleCommandSearch ( iSock, iCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_EXCERPT:	SafeClose ( iPipeFD ); HandleCommandExcerpt ( iSock, iCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_UPDATE:	HandleCommandUpdate ( iSock, iCommandVer, tBuf, iPipeFD ); SafeClose ( iPipeFD ); break;
		default:						assert ( 0 && "INTERNAL ERROR: unhandled command" ); break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// INDEX ROTATION
/////////////////////////////////////////////////////////////////////////////

bool TryRename ( const char * sIndex, const char * sPrefix, const char * sFromPostfix, const char * sToPostfix, bool bFatal )
{
	char sFrom [ SPH_MAX_FILENAME_LEN ];
	char sTo [ SPH_MAX_FILENAME_LEN ];

	snprintf ( sFrom, sizeof(sFrom), "%s%s", sPrefix, sFromPostfix );
	snprintf ( sTo, sizeof(sTo), "%s%s", sPrefix, sToPostfix );
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


void RotateIndexGreedy ( ServedIndex_t & tIndex, const char * sIndex )
{
	char sFile [ SPH_MAX_FILENAME_LEN ];
	const char * sPath = tIndex.m_sIndexPath.cstr();

	// check files
	const int EXT_COUNT = 6;
	const char * dNew[EXT_COUNT] = { ".new.sph", ".new.spa", ".new.spi", ".new.spd", ".new.spp", ".new.spm" };
	const char * dOld[EXT_COUNT] = { ".old.sph", ".old.spa", ".old.spi", ".old.spd", ".old.spp", ".old.spm" };
	const char * dCur[EXT_COUNT] = { ".sph", ".spa", ".spi", ".spd", ".spp", ".spm" };

	for ( int i=0; i<EXT_COUNT; i++ )
	{
		snprintf ( sFile, sizeof(sFile), "%s%s", sPath, dNew[i] );
		if ( !sphIsReadable ( sFile ) )
		{
			if ( i>0 )
				sphWarning ( "rotating index '%s': '%s' unreadable: %s; using old index", sIndex, sFile, strerror(errno) );
			return;
		}
	}

	// rename current to old
	for ( int i=0; i<EXT_COUNT; i++ )
	{
		if ( TryRename ( sIndex, sPath, dCur[i], dOld[i], false ) )
			continue;

		// rollback
		for ( int j=0; j<i; j++ )
			TryRename ( sIndex, sPath, dOld[j], dCur[j], true );

		sphWarning ( "rotating index '%s': rename to .old failed; using old index", sIndex );
		return;
	}

	// rename new to current
	for ( int i=0; i<EXT_COUNT; i++ )
	{
		if ( TryRename ( sIndex, sPath, dNew[i], dCur[i], false ) )
			continue;

		// rollback new ones we already renamed
		for ( int j=0; j<i; j++ )
			TryRename ( sIndex, sPath, dCur[j], dNew[j], true );

		// rollback old ones
		for ( int j=0; j<EXT_COUNT; j++ )
			TryRename ( sIndex, sPath, dOld[j], dCur[j], true );
	}

	// try to use new index
	CSphString sWarning;
	const CSphSchema * pNewSchema = tIndex.m_pIndex->Prealloc ( tIndex.m_bMlock, &sWarning );
	if ( !pNewSchema || !tIndex.m_pIndex->Preread() )
	{
		sphWarning ( "rotating index '%s': .new preload failed: %s", sIndex, tIndex.m_pIndex->GetLastError().cstr() );

		// try to recover
		for ( int j=0; j<EXT_COUNT; j++ )
		{
			TryRename ( sIndex, sPath, dCur[j], dNew[j], true );
			TryRename ( sIndex, sPath, dOld[j], dCur[j], true );
		}

		pNewSchema = tIndex.m_pIndex->Prealloc ( tIndex.m_bMlock, &sWarning );
		if ( !pNewSchema || !tIndex.m_pIndex->Preread() )
		{
			sphWarning ( "rotating index '%s': .new preload failed; ROLLBACK FAILED; INDEX UNUSABLE", sIndex );
			tIndex.m_bEnabled = false;
		} else
		{
			sphWarning ( "rotating index '%s': .new preload failed; using old index", sIndex );
			if ( !sWarning.IsEmpty() )
				sphWarning ( "rotating index '%s': %s", sIndex, sWarning.cstr() );
		}
		return;

	} else
	{
		if ( !sWarning.IsEmpty() )
			sphWarning ( "rotating index '%s': %s", sIndex, sWarning.cstr() );
	}

	// uff. all done
	tIndex.m_pSchema = pNewSchema;
	tIndex.m_bEnabled = true;
	sphInfo ( "rotating index '%s': success", sIndex );
}

/////////////////////////////////////////////////////////////////////////////
// MAIN LOOP
/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS

int CreatePipe ( bool )		{ return -1; }
int PipeAndFork ( bool )	{ return -1; }

#else

// open new pipe to be able to receive notifications from children
// adds read-end fd to g_dPipes; returns write-end fd for child
int CreatePipe ( bool bFatal )
{
	assert ( g_bHeadDaemon );
	int dPipe[2] = { -1, -1 };

	for ( ;; )
	{
		if ( pipe(dPipe) )
		{
			if ( bFatal )
				sphFatal ( "pipe() failed (error=%s)", strerror(errno) );
			else
				sphWarning ( "pipe() failed (error=%s)", strerror(errno) );
			break;
		}

		if ( fcntl ( dPipe[0], F_SETFL, O_NONBLOCK ) )
		{
			sphWarning ( "fcntl() on pipe failed (error=%s)", strerror(errno) );
			SafeClose ( dPipe[0] );
			SafeClose ( dPipe[1] );
			break;
		}

		g_dPipes.Add ( dPipe[0] );
		break;
	}

	return dPipe[1];
}


/// create new worker child
/// creates a pipe to it, forks, and does some post-fork work
//
/// in child, returns write-end pipe fd (might be -1!) and sets g_bHeadDaemon to false
/// in parent, returns -1 and leaves g_bHeadDaemon unaffected
int PipeAndFork ( bool bFatal )
{
	int iChildPipe = CreatePipe ( bFatal );
	switch ( fork() )
	{
		// fork() failed
		case -1:
			sphFatal ( "fork() failed (reason: %s)", strerror(errno) );

		// child process, handle client
		case 0:
			g_bHeadDaemon = false;
			ARRAY_FOREACH ( i, g_dPipes )
				SafeClose ( g_dPipes[i] );
			break;

		// parent process, continue accept()ing
		default:
			SafeClose ( iChildPipe );
			break;
	}
	return iChildPipe;
}

#endif // !USE_WINDOWS


/// check and report if there were any leaks since last call
void CheckLeaks ()
{
#if SPH_DEBUG_LEAKS
	static int iHeadAllocs = sphAllocsCount ();
	static int iHeadCheckpoint = sphAllocsLastID ();

	if ( iHeadAllocs!=sphAllocsCount() )
	{
		sphLockEx ( g_iLogFile, false );
		lseek ( g_iLogFile, 0, SEEK_END );
		sphAllocsDump ( g_iLogFile, iHeadCheckpoint );
		sphLockUn ( g_iLogFile );

		iHeadAllocs = sphAllocsCount ();
		iHeadCheckpoint = sphAllocsLastID ();
	}
#endif
}


void SeamlessTryToForkPrereader ()
{
	// alloc buffer index (once per run)
	if ( !g_pPrereading )
		g_pPrereading = sphCreateIndexPhrase ( NULL );

	// next in line
	const char * sPrereading = g_dRotating.Pop ();
	if ( !sPrereading || !g_hIndexes(sPrereading) )
	{
		sphWarning ( "INTERNAL ERROR: preread attempt on unknown index '%s'", sPrereading ? sPrereading : "(NULL)" );
		return;
	}
	const ServedIndex_t & tServed = g_hIndexes[sPrereading];

	// rebase buffer index
	char sNewPath [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sNewPath, sizeof(sNewPath), "%s.new", tServed.m_sIndexPath.cstr() );
	g_pPrereading->SetBase ( sNewPath );

	// prealloc enough RAM and lock new index
	CSphString sWarn;
	if ( !g_pPrereading->Prealloc ( tServed.m_bMlock, &sWarn ) )
	{
		sphWarning ( "rotating index '%s': prealloc: %s; using old index", sPrereading, g_pPrereading->GetLastError().cstr() );
		return;
	}
	if ( !g_pPrereading->Lock() )
	{
		sphWarning ( "rotating index '%s': lock: %s; using old index", sPrereading, g_pPrereading->GetLastError().cstr() );
		g_pPrereading->Dealloc ();
		return;
	}

	// fork async reader
	g_sPrereading = sPrereading;
	int iPipeFD = PipeAndFork ( true );

	// in parent, wait for prereader process to finish
	if ( g_bHeadDaemon )
		return;

	// in child, do preread
	bool bRes = g_pPrereading->Preread ();
	if ( !bRes )
		sphWarning ( "rotating index '%s': preread failed: %s; using old index", g_sPrereading, g_pPrereading->GetLastError().cstr() );

	// report and exit
	DWORD uTmp = SPH_PIPE_PREREAD;
	::write ( iPipeFD, &uTmp, sizeof(DWORD) );

	uTmp = bRes;
	::write ( iPipeFD, &uTmp, sizeof(DWORD) );

	::close ( iPipeFD );
	exit ( 0 );
}


void SeamlessForkPrereader ()
{
	// sanity checks
	if ( !g_bDoRotate )
	{
		sphWarning ( "INTERNAL ERROR: preread attempt not in rotate state" );
		return;
	}
	if ( g_sPrereading )
	{
		sphWarning ( "INTERNAL ERROR: preread attempt before previous completion" );
		return;
	}

	// try candidates one by one
	while ( g_dRotating.GetLength() && !g_sPrereading )
		SeamlessTryToForkPrereader ();

	// if there's no more candidates, and nothing in the works, we're done
	if ( !g_sPrereading && !g_dRotating.GetLength() )
	{
		g_bDoRotate = false;
		sphInfo ( "rotating indices finished" );
	}
}


/// simple wrapper to simplify reading from pipes
struct PipeReader_t
{
	PipeReader_t ( int iFD )
		: m_iFD ( iFD )
		, m_bError ( false )
	{}

	~PipeReader_t ()
	{
		SafeClose ( m_iFD );
	}

	int GetFD () const
	{
		return m_iFD;
	}

	bool IsError () const
	{
		return m_bError;
	}

	int GetInt ()
	{
		int iTmp;
		if ( !GetBytes ( &iTmp, sizeof(iTmp) ) )
			iTmp = 0;
		return iTmp;
	}

	CSphString GetString ()
	{
		int iLen = GetInt ();
		CSphString sRes;
		sRes.Reserve ( iLen );
		if ( !GetBytes ( const_cast<char*>(sRes.cstr()), iLen ) )
			sRes = "";
		return sRes;
	}

protected:
	bool GetBytes ( void * pBuf, int iCount )
	{
		if ( m_bError )
			return false;
		
		if ( m_iFD<0 )
		{
			m_bError = true;
			sphWarning ( "invalid pipe fd" );
			return false;
		}

		int iRes = ::read ( m_iFD, pBuf, iCount );
		if ( iRes!=iCount )
		{
			m_bError = true;
			sphWarning ( "pipe read failed (exp=%d, res=%d, error=%s)",
				iCount, iRes, iRes>0 ? "(none)" : strerror(errno) );
			return false;
		}

		return true;
	}

protected:
	int			m_iFD;
	bool		m_bError;

};


/// check if there are any notifications from the children and handle them
void CheckPipes ()
{
	ARRAY_FOREACH ( i, g_dPipes )
	{
		// try to get status code
		DWORD uStatus;
		int iRes = ::read ( g_dPipes[i], &uStatus, sizeof(DWORD) );

		// no data yet?
		if ( iRes==-1 && errno==EAGAIN )
			continue;

		// either if there's eof, or error, or valid data - this pipe is over
		PipeReader_t tPipe ( g_dPipes[i] );
		g_dPipes.Remove ( i-- );

		// check for eof/error
		if ( iRes!=sizeof(DWORD) )
		{
			if ( iRes!=0 )
				sphWarning ( "pipe read failed (status)" );
			continue;
		}

		// handle
		if ( uStatus==SPH_PIPE_UPDATED_ATTRS )
		{
			int iUpdIndexes = tPipe.GetInt ();
			for ( int i=0; i<iUpdIndexes; i++ )
			{
				// index name must follow
				CSphString sIndex = tPipe.GetString ();
				if ( tPipe.IsError() )
					break;

				if ( g_hIndexes(sIndex) )
					g_hIndexes(sIndex)->m_pIndex->SetAttrsUpdated ( true );
				else
					sphWarning ( "INTERNAL ERROR: unknown index '%s' in piped notification (updated)", sIndex.cstr() );
			}

		} else if ( uStatus==SPH_PIPE_PREREAD )
		{
			assert ( g_bDoRotate && g_bSeamlessRotate && g_sPrereading );

			int iRes = tPipe.GetInt();
			if ( tPipe.IsError() )
				break;

			// if preread was succesful, exchange served index and prereader buffer index
			if ( iRes )
			{
				ServedIndex_t & tServed = g_hIndexes[g_sPrereading];
				CSphIndex * pOld = tServed.m_pIndex;
				CSphIndex * pNew = g_pPrereading;

				char sOld [ SPH_MAX_FILENAME_LEN ];
				snprintf ( sOld, sizeof(sOld), "%s.old", tServed.m_sIndexPath.cstr() );

				if ( !pOld->Rename ( sOld ) )
				{
					// FIXME! rollback inside Rename() call potentially fail
					sphWarning ( "rotating index '%s': cur to old rename failed: %s", g_sPrereading, pOld->GetLastError().cstr() );
					continue;
				}
				// FIXME! at this point there's no cur lock file; ie. potential race
				if ( !pNew->Rename ( tServed.m_sIndexPath.cstr() ) )
				{
					sphWarning ( "rotating index '%s': new to cur rename failed: %s", g_sPrereading, pNew->GetLastError().cstr() );
					if ( !pOld->Rename ( tServed.m_sIndexPath.cstr() ) )
					{
						sphWarning ( "rotating index '%s': old to cur rename failed: %s; INDEX UNUSABLE", g_sPrereading, pOld->GetLastError().cstr() );
						tServed.m_bEnabled = false;
					}
					continue;
				}

				// all went fine; swap them
				Swap ( tServed.m_pIndex, g_pPrereading );
				tServed.m_pSchema = tServed.m_pIndex->GetSchema ();
				tServed.m_bEnabled = true;
				sphInfo ( "rotating index '%s': success", g_sPrereading );
			}

			// in any case, buffer index should now be deallocated
			g_pPrereading->Dealloc ();
			g_pPrereading->Unlock ();
			g_sPrereading = NULL;

			// work next one
			SeamlessForkPrereader ();

		} else
		{
			// unhandled status code
			sphWarning ( "INTERNAL ERROR: unknown pipe status=%d", uStatus );
		}
	}
}


void CheckRotate ()
{
	// do we need to rotate now?
	if ( !g_bDoRotate )
		return;

	/////////////////////
	// RAM-greedy rotate
	/////////////////////

	if ( !g_bSeamlessRotate )
	{
		// wait until there's no running queries
		if ( g_iChildren )
			return;

		g_hIndexes.IterateStart();
		while ( g_hIndexes.IterateNext() )
		{
			ServedIndex_t & tIndex = g_hIndexes.IterateGet();
			const char * sIndex = g_hIndexes.IterateGetKey().cstr();
			assert ( tIndex.m_pIndex );

			RotateIndexGreedy ( tIndex, sIndex );
		}

		g_bDoRotate = false;
		sphInfo ( "rotating indices finished" );
		return;
	}

	///////////////////
	// seamless rotate
	///////////////////

	assert ( g_bDoRotate && g_bSeamlessRotate );
	if ( g_dRotating.GetLength() || g_sPrereading )
		return; // rotate in progress already; will be handled in CheckPipes()

	// check what indexes need to be rotated
	g_hIndexes.IterateStart();
	while ( g_hIndexes.IterateNext() )
	{
		const ServedIndex_t & tIndex = g_hIndexes.IterateGet();
		const char * sIndex = g_hIndexes.IterateGetKey().cstr();
		assert ( tIndex.m_pIndex );

		CSphString sNewPath;
		sNewPath.SetSprintf ( "%s.new", tIndex.m_sIndexPath.cstr() );

		// check if there's a .new index incoming
		// FIXME? move this code to index, and also check for exists-but-not-readable
		CSphString sTmp;
		sTmp.SetSprintf ( "%s.sph", sNewPath.cstr() );
		if ( !sphIsReadable ( sTmp.cstr() ) )
			continue;

		g_dRotating.Add ( sIndex );
	}

	SeamlessForkPrereader ();
}


#if !USE_WINDOWS
#define WINAPI
#else

SERVICE_STATUS			g_ss;
SERVICE_STATUS_HANDLE	g_ssHandle;


void MySetServiceStatus ( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint )
{
	static DWORD dwCheckPoint = 1;

	if ( dwCurrentState == SERVICE_START_PENDING )
		g_ss.dwControlsAccepted = 0;
	else
		g_ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	g_ss.dwCurrentState = dwCurrentState;
	g_ss.dwWin32ExitCode = dwWin32ExitCode;
	g_ss.dwWaitHint = dwWaitHint;

	if ( dwCurrentState==SERVICE_RUNNING || dwCurrentState==SERVICE_STOPPED )
		g_ss.dwCheckPoint = 0;
	else
		g_ss.dwCheckPoint = dwCheckPoint++;

	SetServiceStatus ( g_ssHandle, &g_ss );
}


void WINAPI ServiceControl ( DWORD dwControlCode )
{
	switch ( dwControlCode )
	{
		case SERVICE_CONTROL_STOP:
			MySetServiceStatus ( SERVICE_STOP_PENDING, NO_ERROR, 0 );
			g_bServiceStop = true;
			break;

		default:
			MySetServiceStatus ( g_ss.dwCurrentState, NO_ERROR, 0 );
			break;
	}
}


// warning! static buffer, non-reentrable
const char * WinErrorInfo ()
{
	static char sBuf[1024];

	DWORD uErr = ::GetLastError ();
	sprintf ( sBuf, "code=%d, error=", uErr );

	int iLen = strlen(sBuf);
	if ( !FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErr, 0, sBuf+iLen, sizeof(sBuf)-iLen, NULL ) ) // FIXME? force US-english langid?
		strcpy ( sBuf+iLen, "(no message)" );

	return sBuf;
}


SC_HANDLE ServiceOpenManager ()
{
	SC_HANDLE hSCM = OpenSCManager (
		NULL,					// local computer
		NULL,					// ServicesActive database 
		SC_MANAGER_ALL_ACCESS );// full access rights

	if ( hSCM==NULL )
		sphFatal ( "OpenSCManager() failed: %s", WinErrorInfo() );

	return hSCM;
}


void strappend ( char * sBuf, const int iBufLimit, char * sAppend )
{
	int iLen = strlen(sBuf);
	int iAppend = strlen(sAppend);

	int iToCopy = Min ( iBufLimit-iLen-1, iAppend );
	memcpy ( sBuf+iLen, sAppend, iToCopy );
	sBuf[iLen+iToCopy] = '\0';
}


void ServiceInstall ( int argc, char ** argv )
{
	if ( g_bService )
		return;

	sphInfo ( "Installing service..." );

	char szPath[MAX_PATH];
	if( !GetModuleFileName ( NULL, szPath, MAX_PATH ) )
		sphFatal ( "GetModuleFileName() failed: %s", WinErrorInfo() );

	strappend ( szPath, sizeof(szPath), " --ntservice" );
	for ( int i=1; i<argc; i++ )
		if ( strcmp ( argv[i], "--install" ) )
	{
		strappend ( szPath, sizeof(szPath), " " );
		strappend ( szPath, sizeof(szPath), argv[i] );
	}

	SC_HANDLE hSCM = ServiceOpenManager ();
	SC_HANDLE hService = CreateService (
		hSCM,							// SCM database 
		SEARCHD_SERVICE_NAME,			// name of service 
		SEARCHD_SERVICE_NAME,			// service name to display 
		SERVICE_ALL_ACCESS,				// desired access 
		SERVICE_WIN32_OWN_PROCESS,		// service type 
		SERVICE_AUTO_START,				// start type 
		SERVICE_ERROR_NORMAL,			// error control type 
		szPath,							// path to service's binary 
		NULL,							// no load ordering group 
		NULL,							// no tag identifier 
		NULL,							// no dependencies 
		NULL,							// LocalSystem account 
		NULL );							// no password 

	if ( !hService ) 
	{
		CloseServiceHandle ( hSCM );
		sphFatal ( "CreateService() failed: %s", WinErrorInfo() );

	} else
	{
		sphInfo ( "Service '%s' installed succesfully.", SEARCHD_SERVICE_NAME ); 
	}

	SERVICE_DESCRIPTION tDesc;
	tDesc.lpDescription = SEARCHD_SERVICE_NAME "-" SPHINX_VERSION;
	if ( !ChangeServiceConfig2 ( hService, SERVICE_CONFIG_DESCRIPTION, &tDesc ) )
		sphWarning ( "failed to set service description" );

	CloseServiceHandle ( hService ); 
	CloseServiceHandle ( hSCM );
}


void ServiceDelete ()
{
	if ( g_bService )
		return;

	sphInfo ( "Deleting service..." );

	// open manager
	SC_HANDLE hSCM = ServiceOpenManager ();

	// open service
	SC_HANDLE hService = OpenService ( hSCM, SEARCHD_SERVICE_NAME, DELETE );
	if ( !hService )
	{ 
		CloseServiceHandle ( hSCM );
		sphFatal ( "OpenService() failed: %s", WinErrorInfo() );
	}

	// do delete
	bool bRes = !!DeleteService(hService);
	CloseServiceHandle ( hService );
	CloseServiceHandle ( hSCM );

	if ( !bRes ) 
		sphFatal ( "DeleteService() failed: %s", WinErrorInfo() );
	else
		sphInfo ( "Service '%s' deleted succesfully.", SEARCHD_SERVICE_NAME ); 

}
#endif // USE_WINDOWS


void ShowHelp ()
{
	fprintf ( stdout,
		"Usage: searchd [OPTIONS]\n"
		"\n"
		"Options are:\n"
		"-h, --help\t\tdisplay this help message\n"
		"-c, -config <file>\tread configuration from specified file\n"
		"\t\t\t(default is sphinx.conf)\n"
		"--stop\t\t\tsend SIGTERM to currently running searchd\n"
		"\t\t\t(PID is taken from pid_file specified in config file)\n"
#if USE_WINDOWS
		"--install\t\tinstall as Windows service\n"
		"--delete\t\tdelete Windows service\n"
#endif
		"\n"
		"Debugging options are:\n"
		"--console\t\trun in console mode (do not fork, do not log to files)\n"
		"-p, --port <port>\tlisten on given port (overrides config setting)\n"
		"-i, --index <index>\tonly serve one given index\n"
		"\n"
		"Examples:\n"
		"searchd --config /usr/local/sphinx/etc/sphinx.conf\n"
#if USE_WINDOWS
		"searchd --install --config c:\\sphinx\\sphinx.conf\n"
#endif
		);
}


int WINAPI ServiceMain ( int argc, char **argv )
{
#if USE_WINDOWS
	CSphVector<char *,8> dArgs;
	if ( g_bService )
	{
		g_ssHandle = RegisterServiceCtrlHandler ( SEARCHD_SERVICE_NAME, ServiceControl );
		if ( !g_ssHandle )
			sphFatal ( "failed to start service: RegisterServiceCtrlHandler() failed: %s", WinErrorInfo() );

		g_ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		MySetServiceStatus ( SERVICE_START_PENDING, NO_ERROR, 4000 );

		if ( argc<=1 )
		{
			dArgs.Resize ( g_dArgs.GetLength() );
			ARRAY_FOREACH ( i, g_dArgs )
				dArgs[i] = (char*) g_dArgs[i].cstr();

			argc = g_dArgs.GetLength();
			argv = &dArgs[0];
		}
	}
#endif

	int rsock;
	struct sockaddr_in remote_iaddr;
	socklen_t len;
	CSphConfig conf;

	if ( !g_bService )
		fprintf ( stdout, SPHINX_BANNER );

	//////////////////////
	// parse command line
	//////////////////////

	const char *	sOptConfig		= NULL;
	bool			bOptConsole		= false;
	bool			bOptStop		= false;
	const char *	sOptIndex		= NULL;
	int				iOptPort		= 0;

	#define OPT(_a1,_a2)	else if ( !strcmp(argv[i],_a1) || !strcmp(argv[i],_a2) )
	#define OPT1(_a1)		else if ( !strcmp(argv[i],_a1) )

	int i;
	for ( i=1; i<argc; i++ )
	{
		// handle non-options
		if ( argv[i][0]!='-' )		break;

		// handle no-arg options
		OPT ( "-h", "--help" )		{ ShowHelp(); return 0; }
		OPT1 ( "--console" )		bOptConsole = true;
		OPT1 ( "--stop" )			bOptStop = true;
#if USE_WINDOWS
		OPT1 ( "--install" )		{ if ( !g_bService ) { ServiceInstall ( argc, argv ); return 0; } }
		OPT1 ( "--delete" )			{ if ( !g_bService ) { ServiceDelete (); return 0; } }
		OPT1 ( "--ntservice" )		; // it's valid but handled elsewhere
#endif

		// handle 1-arg options
		else if ( (i+1)>=argc )		break;
		OPT ( "-c", "--config" )	sOptConfig = argv[++i];
		OPT ( "-p", "--port" )		iOptPort = atoi ( argv[++i] );
		OPT ( "-i", "--index" )		sOptIndex = argv[++i];

		// handle unknown options
		else break;
	}
	if ( i!=argc )
		sphFatal ( "malformed or unknown option near '%s'; use '-h' or '--help' to see available options.", argv[i] );

#if USE_WINDOWS
	if ( !g_bService )
	{
		sphWarning ( "forcing --console mode on Windows" );
		bOptConsole = true;
	}

	// init WSA on Windows
	// we need to do it this early because otherwise gethostbyname() from config parser could fail
	WSADATA tWSAData;
	int iStartupErr = WSAStartup ( WINSOCK_VERSION, &tWSAData );
	if ( iStartupErr )
		sphFatal ( "failed to initialize WinSock2: %s", sphSockError(iStartupErr) );
#endif

	/////////////////////
	// parse config file
	/////////////////////

	// fallback to defaults if there was no explicit config specified
	while ( !sOptConfig )
	{
#ifdef SYSCONFDIR
		sOptConfig = SYSCONFDIR "/sphinx.conf";
		if ( sphIsReadable(sOptConfig) )
			break;
#endif

		sOptConfig = "./sphinx.conf";
		if ( sphIsReadable(sOptConfig) )
			break;

		sOptConfig = NULL;
		break;
	}

	if ( !sOptConfig )
		sphFatal ( "no readable config file (looked in "
#ifdef SYSCONFDIR
			SYSCONFDIR "/sphinx.conf, "
#endif
			"./sphinx.conf)." );

	sphInfo ( "using config file '%s'...", sOptConfig );

	// do parse
	// FIXME! add key validation here. g_dSphKeysCommon, g_dSphKeysSearchd
	CSphConfigParser cp;
	if ( !cp.Parse ( sOptConfig ) )
		sphFatal ( "failed to parse config file '%s'", sOptConfig );

	const CSphConfig & hConf = cp.m_tConf;

	if ( !hConf.Exists ( "searchd" ) || !hConf["searchd"].Exists ( "searchd" ) )
		sphFatal ( "'searchd' config section not found in '%s'", sOptConfig );

	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];

	////////////////////////
	// stop running searchd
	////////////////////////

#if !USE_WINDOWS
	if ( bOptStop )
	{
		if ( !hSearchd("pid_file") )
			sphFatal ( "stop: option 'pid_file' not found in '%s' section 'searchd'", sOptConfig );

		const char * sPid = hSearchd["pid_file"].cstr(); // shortcut
		FILE * fp = fopen ( sPid, "r" );
		if ( !fp )
			sphFatal ( "stop: pid file '%s' does not exist or is not readable", sPid );

		char sBuf[16];
		int iLen = (int) fread ( sBuf, 1, sizeof(sBuf)-1, fp );
		sBuf[iLen] = '\0';
		fclose ( fp );

		int iPid = atoi(sBuf);
		if ( iPid<=0 )
			sphFatal ( "stop: failed to read valid pid from '%s'", sPid );

		if ( kill ( iPid, SIGTERM ) )
			sphFatal ( "stop: kill() on pid %d failed: %s", iPid, strerror(errno) );
		else
		{
			sphInfo ( "stop: succesfully sent SIGTERM to pid %d", iPid );
			exit ( 0 );
		}
	}
#endif

	/////////////////////
	// configure searchd
	/////////////////////

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
	if ( bOptConsole && iOptPort>0 )
		iPort = iOptPort;

	if ( hSearchd.Exists ( "read_timeout" ) && hSearchd["read_timeout"].intval()>=0 )
		g_iReadTimeout = hSearchd["read_timeout"].intval();

	if ( hSearchd.Exists ( "max_children" ) && hSearchd["max_children"].intval()>=0 )
		g_iMaxChildren = hSearchd["max_children"].intval();

	if ( hSearchd("max_matches") )
	{
		int iMax = hSearchd["max_matches"].intval();
		if ( iMax<0 || iMax>10000000 )
		{
			sphWarning ( "max_matches=%d out of bounds; using default 1000", iMax );
		} else
		{
			g_iMaxMatches = iMax;
		}
	}

	if ( hSearchd("seamless_rotate") )
		g_bSeamlessRotate = ( hSearchd["seamless_rotate"].intval()!=0 );

#if 0
	if ( hSearchd("cache_dir") )
	{
		// FIXME! add more validation
		g_bCacheEnable = true;
		g_sCacheDir = hSearchd["cache_dir"];

		if ( hSearchd("cache_ttl") )
			g_iCacheTTL = Max ( hSearchd["cache_ttl"].intval(), 1 );

		if ( hSearchd("cache_gzip") && hSearchd["cache_gzip"].intval()!=0 )
			g_bCacheGzip = true;
	}
#endif

	//////////////////////
	// build indexes hash
	//////////////////////

	// create and lock pid
	int iPidFD = -1;
	if ( !bOptConsole )
	{
		g_sPidFile = hSearchd["pid_file"].cstr();

		iPidFD = ::open ( g_sPidFile, O_CREAT | O_WRONLY, S_IREAD | S_IWRITE );
		if ( iPidFD<0 )
			sphFatal ( "failed to create pid file '%s': %s", g_sPidFile, strerror(errno) );

		if ( !sphLockEx ( iPidFD, false ) )
			sphFatal ( "failed to lock pid file '%s': %s (searchd already running?)", g_sPidFile, strerror(errno) );
	}

	// configure and preload
	int iValidIndexes = 0;
	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext() )
	{
		const CSphConfigSection & hIndex = hConf["index"].IterateGet();
		const char * sIndexName = hConf["index"].IterateGetKey().cstr();

		if ( bOptConsole && sOptIndex && strcasecmp ( sIndexName, sOptIndex )!=0 )
			continue;

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
					sphWarning ( "index '%s': no such local index '%s' - SKIPPING LOCAL INDEX",
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
				while ( sphIsAlpha(*p) || *p=='.' || *p=='-' ) p++;
				if ( p==pAgent->cstr() )
				{
					sphWarning ( "index '%s': agent '%s': host name expected - SKIPPING AGENT",
						sIndexName, pAgent->cstr() );
					continue;
				}
				if ( *p++!=':' )
				{
					sphWarning ( "index '%s': agent '%s': colon expected near '%s' - SKIPPING AGENT",
						sIndexName, pAgent->cstr(), p );
					continue;
				}
				tAgent.m_sHost = pAgent->SubString ( 0, p-1-pAgent->cstr() );

				// extract port
				if ( !isdigit(*p) )
				{
					sphWarning ( "index '%s': agent '%s': port number expected near '%s' - SKIPPING AGENT",
						sIndexName, pAgent->cstr(), p );
					continue;
				}
				tAgent.m_iPort = atoi(p);
				if ( tAgent.m_iPort<=0 || tAgent.m_iPort>=65536 )
				{
					sphWarning ( "index '%s': agent '%s': invalid port number near '%s' - SKIPPING AGENT",
						sIndexName, pAgent->cstr(), p );
					continue;
				}

				// extract index list
				while ( isdigit(*p) ) p++;
				if ( *p++!=':' )
				{
					sphWarning ( "index '%s': agent '%s': colon expected near '%s' - SKIPPING AGENT",
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
					sphWarning ( "index '%s': agent '%s': index list expected near '%s' - SKIPPING AGENT",
						sIndexName, pAgent->cstr(), p );
					continue;
				}
				tAgent.m_sIndexes = sIndexList;

				// lookup address
				struct hostent * hp = gethostbyname ( tAgent.m_sHost.cstr() );
				if ( !hp )
				{
					sphWarning ( "index '%s': agent '%s': failed to lookup host name '%s' (error=%s) - SKIPPING AGENT",
						sIndexName, pAgent->cstr(), tAgent.m_sHost.cstr(), sphSockError() );
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
				sphWarning ( "index '%s': no valid local/remote indexes in distributed index - NOT SERVING",
					sIndexName );
				continue;

			} else
			{
				if ( !g_hDistIndexes.Add ( tIdx, sIndexName ) )
				{
					sphWarning ( "index '%s': duplicate name in hash?! INTERNAL ERROR - NOT SERVING", sIndexName );
					continue;
				}
			}

		} else
		{
			/////////////////////////
			// configure local index
			/////////////////////////

			ServedIndex_t tIdx;

			// check path
			if ( !hIndex.Exists ( "path" ) )
			{
				sphWarning ( "index '%s': key 'path' not found' - NOT SERVING", sIndexName );
				continue;
			}

			// check name
			if ( g_hIndexes.Exists ( sIndexName ) )
			{
				sphWarning ( "index '%s': duplicate name in hash?! INTERNAL ERROR - NOT SERVING", sIndexName );
				continue;
			}
			// configure tokenizer
			CSphString sError;
			tIdx.m_pTokenizer = sphConfTokenizer ( hIndex, sError );
			if ( !tIdx.m_pTokenizer )
			{
				sphWarning ( "index '%s': %s - NOT SERVING", sIndexName, sError.cstr() );
				continue;
			}

			// conifgure dict
			tIdx.m_pDict = sphCreateDictionaryCRC ();
			if ( !tIdx.m_pDict )
			{
				sphWarning ( "index '%s': failed to create dictionary - NOT SERVING", sIndexName );
				continue;
			}
			if ( !tIdx.m_pDict->SetMorphology ( hIndex("morphology"), tIdx.m_pTokenizer->IsUtf8(), sError ) )
				sphWarning ( "index '%s': %s", sIndexName, sError.cstr() );	
			tIdx.m_pDict->LoadStopwords ( hIndex.Exists ( "stopwords" ) ? hIndex["stopwords"].cstr() : NULL, tIdx.m_pTokenizer );

			// configure memlocking
			if ( hIndex("mlock") && hIndex["mlock"].intval() )
				tIdx.m_bMlock = true;

			// try to create index
			CSphString sWarning;
			tIdx.m_pIndex = sphCreateIndexPhrase ( hIndex["path"].cstr() );
			tIdx.m_pSchema = tIdx.m_pIndex->Prealloc ( tIdx.m_bMlock, &sWarning );
			if ( !tIdx.m_pSchema || !tIdx.m_pIndex->Preread() )
			{
				sphWarning ( "index '%s': preload: %s; NOT SERVING", sIndexName, tIdx.m_pIndex->GetLastError().cstr() );
				continue;
			}
			if ( !sWarning.IsEmpty() )
				sphWarning ( "index '%s': %s", sIndexName, sWarning.cstr() );

			// try to lock it
			if ( !bOptConsole && !tIdx.m_pIndex->Lock() )
			{
				sphWarning ( "index '%s': lock: %s; NOT SERVING", sIndexName, tIdx.m_pIndex->GetLastError().cstr() );
				continue;
			}

			// done
			tIdx.m_sIndexPath = hIndex["path"];
			if ( !g_hIndexes.Add ( tIdx, sIndexName ) )
			{
				sphWarning ( "INTERNAL ERROR: index '%s': hash add failed - NOT SERVING", sIndexName );
				continue;
			}

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
			sphFatal ( "failed to open log file '%s': %s", sLog, strerror(errno) );
		}

		// create query log if required
		if ( hSearchd.Exists ( "query_log" ) )
		{
			g_iQueryLogFile = open ( hSearchd["query_log"].cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
			if ( g_iQueryLogFile<0 )
				sphFatal ( "failed to open query log file '%s': %s", hSearchd["query_log"].cstr(), strerror(errno) );
		}

		// do daemonize
#if !USE_WINDOWS
		int iDevNull = open ( "/dev/null", O_RDWR );
		close ( STDIN_FILENO );
		close ( STDOUT_FILENO );
		close ( STDERR_FILENO );
		dup2 ( iDevNull, STDIN_FILENO );
		dup2 ( iDevNull, STDOUT_FILENO );
		dup2 ( iDevNull, STDERR_FILENO );
#endif
		g_bLogStdout = false;

		// explicitly unlock everything in parent immediately before fork
		//
		// there's a race in case another instance is started before
		// child re-acquires all locks; but let's hope that's rare
		g_hIndexes.IterateStart ();
		while ( g_hIndexes.IterateNext () )
		{
			ServedIndex_t & tServed = g_hIndexes.IterateGet ();
			if ( tServed.m_bEnabled )
				tServed.m_pIndex->Unlock();
		}

		sphLockUn ( iPidFD );

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

		// re-lock pid
		// FIXME! there's a potential race here
		if ( !sphLockEx ( iPidFD, true ) )
			sphFatal ( "failed to re-lock pid file '%s': %s", g_sPidFile, strerror(errno) );

		char sPid[16];
		snprintf ( sPid, sizeof(sPid), "%d\n", getpid() );
		int iPidLen = strlen(sPid);

		if ( ::write ( iPidFD, sPid, iPidLen )!=iPidLen )
			sphFatal ( "failed to write to pid file '%s': %s", g_sPidFile, strerror(errno) );
		ftruncate ( iPidFD, iPidLen );

		// re-lock indexes
		g_hIndexes.IterateStart ();
		while ( g_hIndexes.IterateNext () )
		{
			ServedIndex_t & tServed = g_hIndexes.IterateGet ();
			if ( tServed.m_bEnabled && !tServed.m_pIndex->Lock() )
			{
				sphWarning ( "index '%s': lock: %s; INDEX UNUSABLE", g_hIndexes.IterateGetKey().cstr(),
					tServed.m_pIndex->GetLastError().cstr() );
				tServed.m_bEnabled = false;
			}
		}

	} else
	{
		// if we're running in console mode, dump queries to tty as well
		g_iQueryLogFile = g_iLogFile;
	}

	////////////////////
	// network startup
	////////////////////

	// create and bind socket
	DWORD uAddr = htonl(INADDR_ANY);
	if ( hSearchd("address") )
	{
		struct hostent * pHost = gethostbyname ( hSearchd["address"].cstr() );
		if ( !pHost || pHost->h_addrtype!=AF_INET )
		{
			sphWarning ( "no AF_INET address associated with %s, listening on INADDR_ANY", hSearchd["address"].cstr() );
		} else
		{
			assert ( sizeof(DWORD)==pHost->h_length );
			memcpy ( &uAddr, pHost->h_addr, sizeof(uAddr) );
		}
	}
	g_iSocket = sphCreateServerSocket ( uAddr, iPort );
	listen ( g_iSocket, 5 );

	/////////////////
	// serve clients
	/////////////////

	g_bHeadDaemon = true;
	sphInfo ( "accepting connections" );

#if USE_WINDOWS
	if ( g_bService )
		MySetServiceStatus ( SERVICE_RUNNING, NO_ERROR, 0 );
#endif

	sphSetInternalErrorCallback ( LogInternalError );

	for ( ;; )
	{
#if USE_WINDOWS
		if ( g_bService && g_bServiceStop )
		{
			Shutdown ();
			MySetServiceStatus ( SERVICE_STOPPED, NO_ERROR, 0 );
			exit ( 0 );
		}
#endif

		CheckLeaks ();
		CheckPipes ();
		CheckRotate ();

		// we can't simply accept, because of the need to react to HUPs
		fd_set fdsAccept;
		FD_ZERO ( &fdsAccept );
		SPH_FD_SET ( g_iSocket, &fdsAccept );

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

		if ( ( g_iMaxChildren && g_iChildren>=g_iMaxChildren )
			|| ( g_bDoRotate && !g_bSeamlessRotate ) )
		{
			const char * sMessage = "server maxed out, retry in a second";
			int iRespLen = 4 + strlen(sMessage);

			NetOutputBuffer_c tOut ( rsock );
			tOut.SendInt ( SPHINX_SEARCHD_PROTO );
			tOut.SendWord ( SEARCHD_RETRY );
			tOut.SendWord ( 0 ); // version doesn't matter
			tOut.SendInt ( iRespLen );
			tOut.SendString ( sMessage );
			tOut.Flush ();

			sphWarning ( "maxed out, dismissing client" );
			sphSockClose ( rsock );
			continue;
		}

		char sClientIP [ 256 ];
		DWORD uClientIP = ntohl ( remote_iaddr.sin_addr.s_addr );
		snprintf ( sClientIP, sizeof(sClientIP), "%d.%d.%d.%d:%d", 
			(uClientIP>>24) & 0xff, (uClientIP>>16) & 0xff, (uClientIP>>8) & 0xff, uClientIP & 0xff,
			(int)ntohs(remote_iaddr.sin_port) );

		if ( bOptConsole || g_bService )
		{
			HandleClient ( rsock, sClientIP, -1 );
			sphSockClose ( rsock );
			continue;
		}

		// handle that client
		#if !USE_WINDOWS
		g_iChildren++;
		int iChildPipe = PipeAndFork ( false );

		if ( !g_bHeadDaemon )
		{
			// child process, handle client
			HandleClient ( rsock, sClientIP, iChildPipe );
			sphSockClose ( rsock );
			exit ( 0 );
		} else
		{
			// parent process, continue accept()ing
			sphSockClose ( rsock );
		}
		#endif // !USE_WINDOWS
	}
}


int main ( int argc, char **argv )
{
#if USE_WINDOWS
	for ( int i=1; i<argc; i++ )
		if ( strcmp ( argv[i], "--ntservice" )==0 )
	{
		g_bService = true;
		break;
	}

	if ( g_bService )
	{
		for ( int i=0; i<argc; i++ )
			g_dArgs.Add ( argv[i] );

		SERVICE_TABLE_ENTRY dDispatcherTable[] =
		{
			{ SEARCHD_SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
			{ NULL, NULL }
		};
		if ( !StartServiceCtrlDispatcher ( dDispatcherTable ) )
			sphFatal ( "StartServiceCtrlDispatcher() failed: %s", WinErrorInfo() );
	} else
#endif

	return ServiceMain ( argc, argv );
}

//
// $Id$
//
