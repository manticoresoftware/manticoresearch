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
	bool				m_bStar;

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

static CSphVector<CSphString>	g_dArgs;

static bool				g_bHeadDaemon	= false;
static bool				g_bLogStdout	= true;
static int				g_iLogFile		= STDOUT_FILENO;
static ESphLogLevel		g_eLogLevel		= LOG_INFO;

static int				g_iReadTimeout	= 5;	// sec
static int				g_iChildren		= 0;
static int				g_iMaxChildren	= 0;
static int				g_iSocket		= 0;
static int				g_iQueryLogFile	= -1;
static const char *		g_sPidFile		= NULL;
static int				g_iMaxMatches	= 1000;
static bool				g_bSeamlessRotate	= true;

static volatile bool	g_bDoRotate			= false;	// flag that we are rotating now; set from SIGHUP; cleared on rotation success
static volatile bool	g_bGotSighup		= false;	// we just received SIGHUP; need to log
static volatile bool	g_bGotSigterm		= false;	// we just received SIGTERM; need to shutdown
static volatile bool	g_bGotSigchld		= false;	// we just received SIGCHLD; need to count dead children

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

struct  PipeInfo_t
{
	int		m_iFD;			///< read-pipe to child
	bool	m_bPrereader;	///< is that one prereader or "normal" child

	PipeInfo_t () : m_iFD ( -1 ), m_bPrereader ( false ) {}
};

static CSphVector<PipeInfo_t>	g_dPipes;		///< currently open read-pipes to children processes

static CSphVector<DWORD>		g_dMvaStorage;	///< per-query (!) pool to store MVAs received from remote agents

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
	VER_COMMAND_SEARCH		= 0x10F,
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
#define ECONNRESET		WSAECONNRESET
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
	m_bStar		= false;
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


class SearchFailuresLog_i
{
public:
	virtual ~SearchFailuresLog_i () {}
	virtual void SetIndex ( const char * sIndex ) = 0;
	virtual void SetPrefix ( const char * sTemplate, ... ) = 0;
	virtual void Submit ( const char * sTemplate, ... ) = 0;
};


class SearchFailuresLog_c : public SearchFailuresLog_i
{
protected:
	CSphString						m_sIndex;
	CSphString						m_sPrefix;
	CSphVector<SearchFailure_t>		m_dLog;

public:
	void SetIndex ( const char * sIndex )
	{
		m_sIndex = sIndex;
	}

	void SetPrefix ( const char * sTemplate, ... )
	{
		va_list ap;
		va_start ( ap, sTemplate );
		m_sPrefix.SetSprintf ( sTemplate, ap );
		va_end ( ap );
	}

	void Submit ( const char * sTemplate, ... )
	{
		va_list ap;
		va_start ( ap, sTemplate );
		VaSubmit ( ap, sTemplate );
		va_end ( ap );
	}

	void SubmitEx ( const char * sIndex, const char * sTemplate, ... )
	{
		m_sIndex = sIndex;
		m_sPrefix = "";

		va_list ap;
		va_start ( ap, sTemplate );
		VaSubmit ( ap, sTemplate );
		va_end ( ap );
	}

public:
	void VaSetPrefix ( va_list ap, const char * sTemplate )
	{
		m_sPrefix.SetSprintf ( sTemplate, ap );
	}

	void VaSubmit ( va_list ap, const char * sTemplate )
	{
		assert ( !m_sIndex.IsEmpty() );

		char sBuf [ 2048 ];
		snprintf ( sBuf, sizeof(sBuf), "%s", m_sPrefix.IsEmpty() ? "" : m_sPrefix.cstr() );

		int iLen = strlen(sBuf);
		vsnprintf ( sBuf+iLen, sizeof(sBuf)-iLen, sTemplate, ap );

		m_dLog.Add ( SearchFailure_t ( m_sIndex, sBuf ) );
	}

public:
	bool IsEmpty ()
	{
		return m_dLog.GetLength()==0;
	}

	void BuildReport ( StrBuf_t & sReport )
	{
		if ( IsEmpty() )
			return;

		// collapse same messages
		m_dLog.Sort ();
		int iSpanStart = 0;

		for ( int i=1; i<=m_dLog.GetLength(); i++ )
		{
			// keep scanning while error text is the same
			if ( i!=m_dLog.GetLength() )
				if ( m_dLog[i].m_sError==m_dLog[i-1].m_sError )
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
				sSpan += m_dLog[j].m_sIndex.cstr();
			}
			sSpan += ": ";
			if ( !sSpan.Append ( m_dLog[iSpanStart].m_sError.cstr(), true ) )
				break;

			// flush current span
			if ( !sReport.Append ( sSpan.cstr(), true ) )
				break;

			// done
			iSpanStart = i;
		}
	}
};


class SearchFailuresLogset_c : public SearchFailuresLog_i
{
protected:
	CSphVector<SearchFailuresLog_c>		m_dLogs;
	int									m_iStart;
	int									m_iEnd;

public:
	SearchFailuresLogset_c ()
		: m_iStart ( -1 )
		, m_iEnd ( -1 )
	{}

	virtual void SetSize ( int iSize )
	{
		m_dLogs.Resize ( iSize );
	}

	virtual void SetSubset ( int iStart, int iEnd )
	{
		m_iStart = iStart;
		m_iEnd = iEnd;
	}

public:
	virtual void SetIndex ( const char * sIndex )
	{
		for ( int i=m_iStart; i<=m_iEnd; i++ )
			m_dLogs[i].SetIndex ( sIndex );
	}

	virtual void SetPrefix ( const char * sTemplate, ... )
	{
		va_list ap;
		va_start ( ap, sTemplate );
		for ( int i=m_iStart; i<=m_iEnd; i++ )
			m_dLogs[i].VaSetPrefix ( ap, sTemplate );
		va_end ( ap );
	}

	virtual void Submit ( const char * sTemplate, ... )
	{
		va_list ap;
		va_start ( ap, sTemplate );
		for ( int i=m_iStart; i<=m_iEnd; i++ )
			m_dLogs[i].VaSubmit ( ap, sTemplate );
		va_end ( ap );
	}

	SearchFailuresLog_c & operator [] ( int iIndex )
	{
		return m_dLogs[iIndex];
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
void sighup ( int )
{
	g_bDoRotate = true;
	g_bGotSighup = true;
}


void sigterm ( int )
{
	// in child, bail out immediately
	if ( !g_bHeadDaemon )
		exit ( 0 );

	// in head, perform a clean shutdown
	g_bGotSigterm = true;
}


void sigchld ( int )
{
	g_bGotSigchld = true;
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

		// if there was eof, we're done
		if ( iRes==0 )
		{
			sphSockSetErrno ( ECONNRESET );
			return -1;
		}

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
	bool		SendFloat ( float fValue )		{ return SendT<float> ( fValue ); }

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
	float			GetFloat () { return GetT<float> (); }
	CSphString		GetString ();
	int				GetDwords ( DWORD ** pBuffer, int iMax, const char * sErrorTemplate );
	bool			GetDwords ( CSphVector<DWORD> & dBuffer, int iMax, const char * sErrorTemplate );
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


bool InputBuffer_c::GetDwords ( CSphVector<DWORD> & dBuffer, int iMax, const char * sErrorTemplate )
{
	int iCount = GetInt ();
	if ( iCount<0 || iCount>iMax )
	{
		SendErrorReply ( sErrorTemplate, iCount, iMax );
		SetError ( true );
		return false;
	}

	dBuffer.Resize ( iCount );
	if ( iCount )
	{
		if ( !GetBytes ( &dBuffer[0], sizeof(int)*iCount ) )
		{
			dBuffer.Reset ();
			return false;
		}
		ARRAY_FOREACH ( i, dBuffer )
			dBuffer[i] = htonl ( dBuffer[i] );
	}

	return true;
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

// fix MSVC 2005 fuckup
#if USE_WINDOWS
#pragma conform(forScope,on)
#endif

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

	bool			m_bSuccess;		///< whether last request was succesful (ie. there are available results)

	int				m_iReplyStatus;	///< reply status code
	int				m_iReplySize;	///< how many reply bytes are there
	int				m_iReplyRead;	///< how many reply bytes are alredy received
	BYTE *			m_pReplyBuf;	///< reply buffer

	CSphVector<CSphQueryResult>		m_dResults;		///< multi-query results

protected:
	int				m_iAddrType;
	DWORD			m_uAddr;

public:
	Agent_t ()
		: m_iPort ( -1 )
		, m_iSock ( -1 )
		, m_eState ( AGENT_UNUSED )
		, m_bSuccess ( false )
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
	CSphVector<Agent_t>			m_dAgents;					///< remote agents
	CSphVector<CSphString>		m_dLocal;					///< local indexes
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
	virtual bool ParseReply ( MemInputBuffer_c & tReq, Agent_t & tAgent, SearchFailuresLogset_c & dFailures ) const = 0;
};


void ConnectToRemoteAgents ( CSphVector<Agent_t> & dAgents, bool bRetryOnly, SearchFailuresLog_i & dFailures )
{
	ARRAY_FOREACH ( iAgent, dAgents )
	{
		Agent_t & tAgent = dAgents[iAgent];
		if ( bRetryOnly && tAgent.m_eState!=AGENT_RETRY )
			continue;

		dFailures.SetPrefix ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );
		tAgent.m_eState = AGENT_UNUSED;
		tAgent.m_bSuccess = false;

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


int QueryRemoteAgents ( CSphVector<Agent_t> & dAgents, int iTimeout, const IRequestBuilder_t & tBuilder, SearchFailuresLog_i & dFailures )
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
			dFailures.SetPrefix ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );

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
			dFailures.SetPrefix ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );
			dFailures.Submit ( "%s() timed out", tAgent.m_eState==AGENT_HELLO ? "read" : "connect" );
		}
	}

	return iAgents;
}


int WaitForRemoteAgents ( CSphVector<Agent_t> & dAgents, int iTimeout,
	IReplyParser_t & tParser, SearchFailuresLogset_c & dFailuresSet )
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
			dFailuresSet.SetPrefix ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );
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
						dFailuresSet.Submit ( "failed to receive reply header" );
						break;
					}

					tReplyHeader.m_iStatus = ntohs ( tReplyHeader.m_iStatus );
					tReplyHeader.m_iVer = ntohs ( tReplyHeader.m_iVer );
					tReplyHeader.m_iLength = ntohl ( tReplyHeader.m_iLength );

					// check the packet
					if ( tReplyHeader.m_iLength<0 || tReplyHeader.m_iLength>MAX_PACKET_SIZE ) // FIXME! add reasonable max packet len too
					{
						dFailuresSet.Submit ( "invalid packet size (status=%d, len=%d, max_packet_size=%d)", tReplyHeader.m_iStatus, tReplyHeader.m_iLength, MAX_PACKET_SIZE );
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
						dFailuresSet.Submit ( "failed to alloc %d bytes for reply buffer", tAgent.m_iReplySize );
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
						dFailuresSet.Submit ( "failed to receive reply body: %s", sphSockError() );
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
						dFailuresSet.Submit ( "remote warning: %s", sAgentWarning.cstr() );

					} else if ( tAgent.m_iReplyStatus==SEARCHD_RETRY )
					{
						tAgent.m_eState = AGENT_RETRY;
						break;

					} else if ( tAgent.m_iReplyStatus!=SEARCHD_OK )
					{
						CSphString sAgentError = tReq.GetString ();
						dFailuresSet.Submit ( "remote error: %s", sAgentError.cstr() );
						break;
					}

					// call parser
					if ( !tParser.ParseReply ( tReq, tAgent, dFailuresSet ) )
						break;

					// check if there was enough data
					if ( tReq.GetError() )
					{
						dFailuresSet.Submit ( "incomplete reply" );
						return false;
					}

					// all is well
					iAgents++;
					tAgent.Close ();
				
					tAgent.m_bSuccess = true;
					assert ( tAgent.m_dResults.GetLength() );
				}

				bFailure = false;
				break;
			}

			if ( bFailure )
			{
				tAgent.Close ();
				tAgent.m_dResults.Reset ();
			}
		}
	}

	// close timed-out agents
	ARRAY_FOREACH ( iAgent, dAgents )
	{
		Agent_t & tAgent = dAgents[iAgent];
		if ( tAgent.m_eState==AGENT_QUERY )
		{
			assert ( !tAgent.m_dResults.GetLength() );
			assert ( !tAgent.m_bSuccess );
			tAgent.Close ();

			dFailuresSet.SetPrefix ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );
			dFailuresSet.Submit ( "query timed out" );
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
						SearchRequestBuilder_t ( const CSphVector<CSphQuery> & dQueries, int iStart, int iEnd ) : m_dQueries ( dQueries ), m_iStart ( iStart ), m_iEnd ( iEnd ) {}
	virtual void		BuildRequest ( const char * sIndexes, NetOutputBuffer_c & tOut ) const;

protected:
	int					CalcQueryLen ( const char * sIndexes, const CSphQuery & q ) const;
	void				SendQuery ( const char * sIndexes, NetOutputBuffer_c & tOut, const CSphQuery & q ) const;

protected:
	const CSphVector<CSphQuery> &		m_dQueries;
	int									m_iStart;
	int									m_iEnd;
};


struct SearchReplyParser_t : public IReplyParser_t
{
						SearchReplyParser_t ( int iStart, int iEnd ) : m_iStart ( iStart ), m_iEnd ( iEnd ) {}
	virtual bool		ParseReply ( MemInputBuffer_c & tReq, Agent_t & tAgent, SearchFailuresLogset_c & dFailures ) const;

protected:
	int					m_iStart;
	int					m_iEnd;
};

/////////////////////////////////////////////////////////////////////////////

int SearchRequestBuilder_t::CalcQueryLen ( const char * sIndexes, const CSphQuery & q ) const
{
	int iReqSize = 80 + 2*sizeof(SphDocID_t) + 4*q.m_iWeights
		+ strlen ( q.m_sSortBy.cstr() )
		+ strlen ( q.m_sQuery.cstr() )
		+ strlen ( sIndexes )
		+ strlen ( q.m_sGroupBy.cstr() )
		+ strlen ( q.m_sGroupSortBy.cstr() )
		+ strlen ( q.m_sGroupDistinct.cstr() );
	ARRAY_FOREACH ( j, q.m_dFilters )
	{
		const CSphFilter & tFilter = q.m_dFilters[j];
		iReqSize += 12 + strlen ( tFilter.m_sAttrName.cstr() ); // string attr-name; int type; int exclude-flag
		switch ( tFilter.m_eType )
		{
			case SPH_FILTER_VALUES:
				iReqSize += 4 + 4*tFilter.m_dValues.GetLength(); // int values-count; int[] values
				break;

			case SPH_FILTER_RANGE:
			case SPH_FILTER_FLOATRANGE:
				iReqSize += 8; // int/float min-val,max-val
				break;
		}
	}
	if ( q.m_bGeoAnchor )
		iReqSize += 16 + strlen ( q.m_sGeoLatAttr.cstr() ) + strlen ( q.m_sGeoLongAttr.cstr() ); // string lat-attr, long-attr; float lat, long
	ARRAY_FOREACH ( i, q.m_dIndexWeights )
		iReqSize += 8 + strlen ( q.m_dIndexWeights[i].m_sName.cstr() ); // string index-name; int index-weight
	return iReqSize;
}


void SearchRequestBuilder_t::SendQuery ( const char * sIndexes, NetOutputBuffer_c & tOut, const CSphQuery & q ) const
{
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
		tOut.SendInt ( tFilter.m_eType );
		switch ( tFilter.m_eType )
		{
			case SPH_FILTER_VALUES:
				tOut.SendInt ( tFilter.m_dValues.GetLength() );
				ARRAY_FOREACH ( k, tFilter.m_dValues )
					tOut.SendInt ( tFilter.m_dValues[k] );
				break;

			case SPH_FILTER_RANGE:
				tOut.SendDword ( tFilter.m_uMinValue );
				tOut.SendDword ( tFilter.m_uMaxValue );
				break;

			case SPH_FILTER_FLOATRANGE:
				tOut.SendFloat ( tFilter.m_fMinValue );
				tOut.SendFloat ( tFilter.m_fMaxValue );
				break;
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
	tOut.SendInt ( q.m_bGeoAnchor );
	if ( q.m_bGeoAnchor )
	{
		tOut.SendString ( q.m_sGeoLatAttr.cstr() );
		tOut.SendString ( q.m_sGeoLongAttr.cstr() );
		tOut.SendFloat ( q.m_fGeoLatitude );
		tOut.SendFloat ( q.m_fGeoLongitude );
	}
	tOut.SendInt ( q.m_dIndexWeights.GetLength() );
	ARRAY_FOREACH ( i, q.m_dIndexWeights )
	{
		tOut.SendString ( q.m_dIndexWeights[i].m_sName.cstr() );
		tOut.SendInt ( q.m_dIndexWeights[i].m_iValue );
	}
}


void SearchRequestBuilder_t::BuildRequest ( const char * sIndexes, NetOutputBuffer_c & tOut ) const
{
	int iReqLen = 4; // int num-queries
	for ( int i=m_iStart; i<=m_iEnd; i++ )
		iReqLen += CalcQueryLen ( sIndexes, m_dQueries[i] );

	tOut.SendDword ( SPHINX_SEARCHD_PROTO );
	tOut.SendWord ( SEARCHD_COMMAND_SEARCH ); // command id
	tOut.SendWord ( VER_COMMAND_SEARCH ); // command version
	tOut.SendInt ( iReqLen ); // request body length

	tOut.SendInt ( m_iEnd-m_iStart+1 );
	for ( int i=m_iStart; i<=m_iEnd; i++ )
		SendQuery ( sIndexes, tOut, m_dQueries[i] );
}

/////////////////////////////////////////////////////////////////////////////

bool SearchReplyParser_t::ParseReply ( MemInputBuffer_c & tReq, Agent_t & tAgent, SearchFailuresLogset_c & dFailuresSet ) const
{
	int iResults = m_iEnd-m_iStart+1;
	assert ( iResults>0 );

	tAgent.m_dResults.Resize ( iResults );
	for ( int iRes=0; iRes<iResults; iRes++ )
		tAgent.m_dResults[iRes].m_iSuccesses = 0;

	for ( int iRes=0; iRes<iResults; iRes++ )
	{
		SearchFailuresLog_c & dFailures = dFailuresSet [ iRes+m_iStart ];
		CSphQueryResult & tRes = tAgent.m_dResults [ iRes ];

		// get status
		DWORD eStatus = tReq.GetDword ();
		if ( eStatus!=SEARCHD_OK )
		{
			dFailures.Submit ( tReq.GetString().cstr() );
			if ( eStatus==SEARCHD_ERROR )
				continue;
		}

		// get schema
		CSphSchema & tSchema = tRes.m_tSchema;
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

		assert ( !tRes.m_dMatches.GetLength() );
		if ( iMatches )
		{
			tRes.m_dMatches.Resize ( iMatches );
			ARRAY_FOREACH ( i, tRes.m_dMatches )
			{
				CSphMatch & tMatch = tRes.m_dMatches[i];
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
		tRes.m_iTotalMatches = tReq.GetInt ();
		tRes.m_iQueryTime = tReq.GetInt ();
		tRes.m_iNumWords = tReq.GetInt ();
		if ( iRetrieved!=iMatches )
		{
			dFailures.Submit ( "expected %d retrieved documents, got %d", iMatches, iRetrieved );
			return false;
		}

		// read per-word stats
		for ( int i=0; i<tRes.m_iNumWords; i++ )
		{
			CSphString sWord = tReq.GetString ();
			int iDocs = tReq.GetInt ();
			int iHits = tReq.GetInt ();

			if ( i<SPH_MAX_QUERY_WORDS )
			{
				tRes.m_tWordStats[i].m_sWord = sWord;
				tRes.m_tWordStats[i].m_iDocs = iDocs;
				tRes.m_tWordStats[i].m_iHits = iHits;
			}
		}

		// mark this result as ok
		tRes.m_iSuccesses = 1;
	}

	// all seems OK (and buffer length checks are performed by caller)
	return true;
}

/////////////////////////////////////////////////////////////////////////////

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
	CSphVector<CSphColumnInfo> dDst;
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


bool FixupQuery ( CSphQuery * pQuery, const CSphSchema * pSchema, const char * sIndexName, CSphString & sError )
{
	// already?
	if ( !pQuery->m_iOldVersion )
		return true;

	if ( pQuery->m_iOldGroups>0 || pQuery->m_iOldMinGID!=0 || pQuery->m_iOldMaxGID!=UINT_MAX )
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
			sError.SetSprintf ( "index '%s': no group attribute found", sIndexName );
			return false;
		}

		CSphFilter tFilter;
		tFilter.m_sAttrName = pSchema->GetAttr(iAttr).m_sName;
		tFilter.m_dValues.Resize ( pQuery->m_iOldGroups );
		ARRAY_FOREACH ( i, tFilter.m_dValues )
			tFilter.m_dValues[i] = pQuery->m_pOldGroups[i];
		tFilter.m_uMinValue = pQuery->m_iOldMinGID;
		tFilter.m_uMaxValue = pQuery->m_iOldMaxGID;
		pQuery->m_dFilters.Add ( tFilter );
	}

	if ( pQuery->m_iOldMinTS!=0 || pQuery->m_iOldMaxTS!=UINT_MAX )
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
			sError.SetSprintf ( "index '%s': no timestamp attribute found", sIndexName );
			return false;
		}

		CSphFilter tFilter;
		tFilter.m_sAttrName = pSchema->GetAttr(iAttr).m_sName;
		tFilter.m_uMinValue = pQuery->m_iOldMinTS;
		tFilter.m_uMaxValue = pQuery->m_iOldMaxTS;
		pQuery->m_dFilters.Add ( tFilter );
	}

	pQuery->m_iOldVersion = 0;
	return true;
}


void ParseIndexList ( const CSphString & sIndexes, CSphVector<CSphString> & dOut )
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


bool ParseSearchQuery ( InputBuffer_c & tReq, CSphQuery & tQuery, int iVer )
{
	tQuery.m_iOldVersion = iVer;

	// v.1.0. mode, limits, weights, ID/TS ranges
	tQuery.m_iOffset	= tReq.GetInt ();
	tQuery.m_iLimit		= tReq.GetInt ();
	tQuery.m_eMode		= (ESphMatchMode) tReq.GetInt ();
	tQuery.m_eSort		= (ESphSortOrder) tReq.GetInt ();
	if ( iVer<=0x101 )
		tQuery.m_iOldGroups = tReq.GetDwords ( &tQuery.m_pOldGroups, SEARCHD_MAX_ATTR_VALUES, "invalid group count %d (should be in 0..%d range)" );
	if ( iVer>=0x102 )
	{
		tQuery.m_sSortBy = tReq.GetString ();
		tQuery.m_sSortBy.ToLower ();
	}
	tQuery.m_sQuery		= tReq.GetString ();
	tQuery.m_iWeights	= tReq.GetDwords ( (DWORD**)&tQuery.m_pWeights, SPH_MAX_FIELDS, "invalid weight count %d (should be in 0..%d range)" );
	tQuery.m_sIndexes	= tReq.GetString ();

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

	// v.1.0, v.1.1
	if ( iVer<=0x101 )
	{
		tQuery.m_iOldMinTS = tReq.GetDword ();
		tQuery.m_iOldMaxTS = tReq.GetDword ();
	}

	// v.1.1 specific
	if ( iVer==0x101 )
	{
		tQuery.m_iOldMinGID = tReq.GetDword ();
		tQuery.m_iOldMaxGID = tReq.GetDword ();
	}

	// v.1.2
	if ( iVer>=0x102 )
	{
		int iAttrFilters = tReq.GetInt ();
		if ( iAttrFilters>SEARCHD_MAX_ATTRS )
		{
			tReq.SendErrorReply ( "too much attribute filters (req=%d, max=%d)", iAttrFilters, SEARCHD_MAX_ATTRS );
			return false;
		}

		tQuery.m_dFilters.Resize ( iAttrFilters );
		ARRAY_FOREACH ( iFilter, tQuery.m_dFilters )
		{
			CSphFilter & tFilter = tQuery.m_dFilters[iFilter];
			tFilter.m_sAttrName = tReq.GetString ();
			tFilter.m_sAttrName.ToLower ();

			if ( iVer>=0x10E )
			{
				// v.1.14+
				tFilter.m_eType = (ESphFilter) tReq.GetDword ();
				switch ( tFilter.m_eType )
				{
					case SPH_FILTER_RANGE:
						tFilter.m_uMinValue = tReq.GetDword ();
						tFilter.m_uMaxValue = tReq.GetDword ();
						break;

					case SPH_FILTER_FLOATRANGE:
						tFilter.m_fMinValue = tReq.GetFloat ();
						tFilter.m_fMaxValue = tReq.GetFloat ();
						break;

					case SPH_FILTER_VALUES:
						if ( !tReq.GetDwords ( tFilter.m_dValues, SEARCHD_MAX_ATTR_VALUES, "invalid attribute set length %d (should be in 0..%d range)" ) )
							return false;
						break;

					default:
						tReq.SendErrorReply ( "unknown filter type (type-id=%d)", tFilter.m_eType );
						return false;
				}

			} else
			{
				// pre-1.14
				if ( !tReq.GetDwords ( tFilter.m_dValues, SEARCHD_MAX_ATTR_VALUES, "invalid attribute set length %d (should be in 0..%d range)" ) )
					return false;

				if ( !tFilter.m_dValues.GetLength() )
				{
					// 0 length means this is range, not set
					tFilter.m_uMinValue = tReq.GetDword ();
					tFilter.m_uMaxValue = tReq.GetDword ();
				}

				tFilter.m_eType = tFilter.m_dValues.GetLength() ? SPH_FILTER_VALUES : SPH_FILTER_RANGE;
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
				return false;
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

	// v.1.14
	if ( iVer>=0x10E )
	{
		tQuery.m_bGeoAnchor = ( tReq.GetInt()!=0 );
		if ( tQuery.m_bGeoAnchor )
		{
			tQuery.m_sGeoLatAttr = tReq.GetString ();
			tQuery.m_sGeoLongAttr = tReq.GetString ();
			tQuery.m_fGeoLatitude = tReq.GetFloat ();
			tQuery.m_fGeoLongitude = tReq.GetFloat ();
		}
	}

	// v.1.15
	if ( iVer>=0x10F )
	{
		tQuery.m_dIndexWeights.Resize ( tReq.GetInt() ); // FIXME! add sanity check
		ARRAY_FOREACH ( i, tQuery.m_dIndexWeights )
		{
			tQuery.m_dIndexWeights[i].m_sName = tReq.GetString ();
			tQuery.m_dIndexWeights[i].m_iValue = tReq.GetInt ();
		}
	}

	/////////////////////
	// additional checks
	/////////////////////

	if ( tReq.GetError() )
	{
		tReq.SendErrorReply ( "invalid or truncated request" );
		return false;
	}
	if ( tQuery.m_iMinID>tQuery.m_iMaxID )
	{
		tReq.SendErrorReply ( "invalid ID range (min greater than max)" );
		return false;
	}
	if ( tQuery.m_eMode<0 || tQuery.m_eMode>SPH_MATCH_TOTAL )
	{
		tReq.SendErrorReply ( "invalid match mode %d", tQuery.m_eMode );
		return false;
	}
	if ( tQuery.m_iMaxMatches<1 || tQuery.m_iMaxMatches>g_iMaxMatches )
	{
		tReq.SendErrorReply ( "per-query max_matches=%d out of bounds (per-server max_matches=%d)",
			tQuery.m_iMaxMatches, g_iMaxMatches );
		return false;
	}
	if ( tQuery.m_iOffset<0 || tQuery.m_iOffset>=tQuery.m_iMaxMatches )
	{
		tReq.SendErrorReply ( "offset out of bounds (offset=%d, max_matches=%d)",
			tQuery.m_iOffset, tQuery.m_iMaxMatches );
		return false;
	}
	if ( tQuery.m_iLimit<0 )
	{
		tReq.SendErrorReply ( "limit out of bounds (limit=%d)", tQuery.m_iLimit );
		return false;
	}
	if ( tQuery.m_iCutoff<0 )
	{
		tReq.SendErrorReply ( "cutoff out of bounds (cutoff=%d)", tQuery.m_iCutoff );
		return false;
	}
	if ( tQuery.m_iRetryCount<0 || tQuery.m_iRetryCount>MAX_RETRY_COUNT )
	{
		tReq.SendErrorReply ( "retry count out of bounds (count=%d)", tQuery.m_iRetryCount );
		return false;
	}
	if ( tQuery.m_iRetryDelay<0 || tQuery.m_iRetryDelay>MAX_RETRY_DELAY )
	{
		tReq.SendErrorReply ( "retry delay out of bounds (delay=%d)", tQuery.m_iRetryDelay );
		return false;
	}

	// all ok
	return true;
}


void LogQuery ( const CSphQuery & tQuery, const CSphQueryResult & tRes )
{
	if ( g_iQueryLogFile<0 || !tRes.m_sError.IsEmpty() )
		return;

	time_t tNow;
	char sTimeBuf[128], sGroupBuf[128];
	char sBuf[1024];

	time ( &tNow );
	ctime_r ( &tNow, sTimeBuf );
	sTimeBuf [ strlen(sTimeBuf)-1 ] = '\0';

	sGroupBuf[0] = '\0';
	if ( tQuery.m_iGroupbyOffset>=0 )
		snprintf ( sGroupBuf, sizeof(sGroupBuf), " @%s", tQuery.m_sGroupBy.cstr() );

	static const char * sModes [ SPH_MATCH_TOTAL ] = { "all", "any", "phr", "bool", "ext", "scan" };
	static const char * sSort [ SPH_SORT_TOTAL ] = { "rel", "attr-", "attr+", "tsegs", "ext" };

	snprintf ( sBuf, sizeof(sBuf), "[%s] %d.%03d sec [%s/%d/%s %d (%d,%d)%s] [%s] %s\n",
		sTimeBuf, tRes.m_iQueryTime/1000, tRes.m_iQueryTime%1000,
		sModes [ tQuery.m_eMode ], tQuery.m_dFilters.GetLength(), sSort [ tQuery.m_eSort ],
		tRes.m_iTotalMatches, tQuery.m_iOffset, tQuery.m_iLimit, sGroupBuf,
		tQuery.m_sIndexes.cstr(), tQuery.m_sQuery.cstr() );

	sphLockEx ( g_iQueryLogFile, true );
	lseek ( g_iQueryLogFile, 0, SEEK_END );
	write ( g_iQueryLogFile, sBuf, strlen(sBuf) );
	sphLockUn ( g_iQueryLogFile );
}


int CalcResultLength ( int iVer, const CSphQueryResult * pRes, const CSphVector<const DWORD *> & dTag2MVA )
{
	int iRespLen = 0;

	// query status
	if ( iVer>=0x10D )
	{
		// multi-query status
		iRespLen += 4; // status code

		if ( !pRes->m_sError.IsEmpty() )
			return iRespLen + 4 +strlen ( pRes->m_sError.cstr() );
			
		if ( !pRes->m_sWarning.IsEmpty() )
			iRespLen += 4+strlen ( pRes->m_sWarning.cstr() );

	} else if ( iVer>=0x106 )
	{
		// warning message
		if ( !pRes->m_sWarning.IsEmpty() )
			iRespLen += 4 + strlen ( pRes->m_sWarning.cstr() );
	}

	// query stats
	iRespLen += 20;

	// schema
	if ( iVer>=0x102 ) 
	{
		iRespLen += 8; // 4 for field count, 4 for attr count
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dFields )
			iRespLen += 4 + strlen ( pRes->m_tSchema.m_dFields[i].m_sName.cstr() ); // namelen, name
		for ( int i=0; i<pRes->m_tSchema.GetAttrsCount(); i++ )
			iRespLen += 8 + strlen ( pRes->m_tSchema.GetAttr(i).m_sName.cstr() ); // namelen, name, type
	}

	// matches
	if ( iVer<0x102 )
		iRespLen += 16*pRes->m_iCount; // matches
	else if ( iVer<0x108 )
		iRespLen += ( 8+4*pRes->m_tSchema.GetAttrsCount() )*pRes->m_iCount; // matches
	else
		iRespLen += 4 + ( 8+4*USE_64BIT+4*pRes->m_tSchema.GetAttrsCount() )*pRes->m_iCount; // id64 tag and matches

	for ( int i=0; i<pRes->m_iNumWords; i++ ) // per-word stats
		iRespLen += 12 + strlen ( pRes->m_tWordStats[i].m_sWord.cstr() ); // wordlen, word, docs, hits

	// MVA values
	CSphVector<int> dMvaItems;
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
		for ( int i=0; i<pRes->m_iCount; i++ )
		{
			const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];
			const DWORD * pMva = dTag2MVA [ tMatch.m_iTag ];
			ARRAY_FOREACH ( j, dMvaItems )
			{
				DWORD iMvaIndex = tMatch.GetAttr ( dMvaItems[j] );
				if ( iMvaIndex )
					iRespLen += 4*pMva[iMvaIndex]; // FIXME? maybe add some sanity check here
			}
		}
	}

	return iRespLen;
}


void SendResult ( int iVer, NetOutputBuffer_c & tOut, const CSphQueryResult * pRes, const CSphVector<const DWORD *> & dTag2MVA )
{
	// status
	if ( iVer>=0x10D )
	{
		// multi-query status
		bool bError = !pRes->m_sError.IsEmpty();
		bool bWarning = !bError && !pRes->m_sWarning.IsEmpty();

		if ( bError )
		{
			tOut.SendInt ( SEARCHD_ERROR );
			tOut.SendString ( pRes->m_sError.cstr() );
			return;

		} else if ( bWarning )
		{
			tOut.SendInt ( SEARCHD_WARNING );
			tOut.SendString ( pRes->m_sWarning.cstr() );
		} else
		{
			tOut.SendInt ( SEARCHD_OK );
		}

	} else
	{
		// single-query warning
		if ( iVer>=0x106 && !pRes->m_sWarning.IsEmpty() )
			tOut.SendString ( pRes->m_sWarning.cstr() );
	}

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

	tOut.SendInt ( pRes->m_iCount );
	if ( iVer>=0x108 )
		tOut.SendInt ( USE_64BIT );

	for ( int i=0; i<pRes->m_iCount; i++ )
	{
		const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];
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
					DWORD iMvaIndex = tMatch.GetAttr ( tAttr.m_iRowitem );
					if ( iVer<0x10C || iMvaIndex==0 )
					{
						// for older clients, fixups column value to 0
						// for newer clients, means that there are 0 values
						tOut.SendDword ( 0 );
					} else
					{
						// send MVA values
						const DWORD * pValues = pMva + iMvaIndex;
						int iValues = *pValues++;

						tOut.SendDword ( iValues );
						while ( iValues-- )
							tOut.SendDword ( *pValues++ );
					}
				} else
				{
					// send plain attr
					if ( tAttr.m_eAttrType==SPH_ATTR_FLOAT )
						tOut.SendFloat ( tMatch.GetAttrFloat ( tAttr.m_iRowitem ) );
					else
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
}

/////////////////////////////////////////////////////////////////////////////

struct AggrResult_t : CSphQueryResult
{
	CSphVector<CSphSchema>		m_dSchemas;		///< aggregated resultsets schemas (for schema minimization)
	CSphVector<int>				m_dMatchCounts;	///< aggregated resultsets lengths (for schema minimization)
	CSphVector<int>				m_dIndexWeights;///< aggregated resultsets per-index weights (optional)
};


bool MinimizeAggrResult ( AggrResult_t & tRes, CSphQuery & tQuery )
{
	// sanity check
	int iExpected = 0;
	ARRAY_FOREACH ( i, tRes.m_dMatchCounts )
		iExpected += tRes.m_dMatchCounts[i];

	if ( iExpected!=tRes.m_dMatches.GetLength() )
	{
		tRes.m_sError.SetSprintf ( "INTERNAL ERROR: expected %d matches in combined result set, got %d",
			iExpected, tRes.m_dMatches.GetLength() );
		return false;
	}

	if ( !tRes.m_dMatches.GetLength() )
		return true;

	// build minimal schema
	bool bAllEqual = true;
	tRes.m_tSchema.Reset();
	ARRAY_FOREACH ( i, tRes.m_dSchemas )
		if ( !MinimizeSchema ( tRes.m_tSchema, tRes.m_dSchemas[i] ) )
			bAllEqual = false;

	// convert all matches to minimal schema
	if ( !bAllEqual )
	{
		int iCur = 0;
		int * dMapFrom = NULL;

		CSphDocInfo tRow;
		tRow.Reset ( tRes.m_tSchema.GetRowSize() );

		if ( tRow.m_iRowitems )
			dMapFrom = new int [ tRes.m_tSchema.GetAttrsCount() ];

		ARRAY_FOREACH ( iSchema, tRes.m_dSchemas )
		{
			assert ( tRow.m_iRowitems<=tRes.m_dSchemas[iSchema].GetRowSize() );

			for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); i++ ) 
			{
				dMapFrom[i] = tRes.m_dSchemas[iSchema].GetAttrIndex ( tRes.m_tSchema.GetAttr(i).m_sName.cstr() );
				assert ( dMapFrom[i]>=0 );
			}

			for ( int i=iCur; i<iCur+tRes.m_dMatchCounts[iSchema]; i++ )
			{
				CSphMatch & tMatch = tRes.m_dMatches[i];
				assert ( tMatch.m_iRowitems>=tRow.m_iRowitems );

				if ( tRow.m_iRowitems )
				{
					for ( int j=0; j<tRes.m_tSchema.GetAttrsCount(); j++ ) 
					{
						const CSphColumnInfo & tDst = tRes.m_tSchema.GetAttr(j);
						const CSphColumnInfo & tSrc = tRes.m_dSchemas[iSchema].GetAttr ( dMapFrom[j] );

						tRow.SetAttr ( tDst.m_iBitOffset, tDst.m_iBitCount,
							tMatch.GetAttr ( tSrc.m_iBitOffset, tSrc.m_iBitCount ) );
					}

					for ( int j=0; j<tRow.m_iRowitems; j++ )
						tMatch.m_pRowitems[j] = tRow.m_pRowitems[j];
				}
				tMatch.m_iRowitems = tRow.m_iRowitems;
			}

			iCur += tRes.m_dMatchCounts[iSchema];
		}

		assert ( iCur==tRes.m_dMatches.GetLength() );
		SafeDeleteArray ( dMapFrom );
	}

	// create queue
	ISphMatchSorter * pSorter = sphCreateQueue ( &tQuery, tRes.m_tSchema, tRes.m_sError );
	if ( !pSorter )
		return false;

	// sort by docid and then by tag
	tRes.m_dMatches.Sort ();

	// kill all dupes
	int iDupes = 0;
	if ( tQuery.m_iGroupbyOffset>=0 )
	{
		// groupby sorters does that automagically
		ARRAY_FOREACH ( i, tRes.m_dMatches )
			if ( !pSorter->Push ( tRes.m_dMatches[i] ) )
				iDupes++;
	} else
	{
		// normal sorter needs massasging
		if ( tQuery.m_dIndexWeights.GetLength() )
		{
			// if there were per-index weights, compute weighted ranks sum
			int iCur = 0;
			int iMax = tRes.m_dMatches.GetLength();

			while ( iCur<iMax )
			{
				CSphMatch & tMatch = tRes.m_dMatches[iCur++];
				if ( tMatch.m_iTag>=0 )
					tMatch.m_iWeight *= tRes.m_dIndexWeights[tMatch.m_iTag];

				while ( iCur<iMax && tRes.m_dMatches[iCur].m_iDocID==tMatch.m_iDocID )
				{
					const CSphMatch & tDupe = tRes.m_dMatches[iCur];
					int iAddWeight = tDupe.m_iWeight;
					if ( tDupe.m_iTag>=0 )
						iAddWeight *= tRes.m_dIndexWeights[tDupe.m_iTag];
					tMatch.m_iWeight += iAddWeight;

					iDupes++;
					iCur++;
				}

				pSorter->Push ( tMatch );
			}

		} else
		{
			// by default, simply remove dupes (select first by tag)
			ARRAY_FOREACH ( i, tRes.m_dMatches )
			{
				if ( i==0 || tRes.m_dMatches[i].m_iDocID!=tRes.m_dMatches[i-1].m_iDocID )
					pSorter->Push ( tRes.m_dMatches[i] );
				else
					iDupes++;
			}
		}
	}

	tRes.m_dMatches.Reset ();
	sphFlattenQueue ( pSorter, &tRes, -1 );
	SafeDelete ( pSorter );

	tRes.m_iTotalMatches -= iDupes;
	return true;
}

/////////////////////////////////////////////////////////////////////////////

class SearchHandler_c
{
public:
									SearchHandler_c ( int iQueries, int iClientVer );
	void							RunQueries ();					///< run all queries, get all results

public:
	int								m_iClientVer;					///< client version
	CSphVector<CSphQuery>			m_dQueries;						///< queries which i need to search
	CSphVector<AggrResult_t>		m_dResults;						///< results which i obtained
	SearchFailuresLogset_c			m_dFailuresSet;					///< failure logs for each query

	int								m_iTag;							///< current tag
	CSphVector<const DWORD *>		m_dTag2MVA;						///< tag to mva-storage-ptr mapping

protected:
	void							RunSubset ( int iStart, int iEnd );	///< run queries against index(es) from first query in the subset
};


SearchHandler_c::SearchHandler_c ( int iQueries, int iClientVer )
{
	m_iClientVer = iClientVer;
	m_iTag = 1; // first avail tag for local storage ptrs
	m_dTag2MVA.Add ( NULL ); // reserve index 0 for remote mva storage ptr; we'll fix this up later

	m_dQueries.Resize ( iQueries );
	m_dResults.Resize ( iQueries );
	m_dFailuresSet.SetSize ( iQueries );

	ARRAY_FOREACH ( i, m_dResults )
	{
		assert ( m_dResults[i].m_dIndexWeights.GetLength()==0 );
		m_dResults[i].m_dIndexWeights.Add ( 1 ); // reserved index 0 with weight 1 for remote matches
	}
}


void SearchHandler_c::RunQueries ()
{
	///////////////////////////////
	// choose path and run queries
	///////////////////////////////

	g_dMvaStorage.Reserve ( 1024 );
	g_dMvaStorage.Resize ( 0 );

	// check if all queries are to the same index
	bool bSameIndex = false;
	if ( m_dQueries.GetLength()>1 )
	{
		bSameIndex = true;
		ARRAY_FOREACH ( i, m_dQueries )
			if ( m_dQueries[i].m_sIndexes!=m_dQueries[0].m_sIndexes )
		{
			bSameIndex = false;
			break;
		}
	}

	if ( bSameIndex )
	{
		///////////////////////////////
		// batch queries to same index
		///////////////////////////////

		RunSubset ( 0, m_dQueries.GetLength()-1 );
		ARRAY_FOREACH ( i, m_dQueries )
			LogQuery ( m_dQueries[i], m_dResults[i] );

	} else
	{
		/////////////////////////////////////////////
		// fallback; just work each query separately
		/////////////////////////////////////////////

		ARRAY_FOREACH ( i, m_dQueries )
		{
			RunSubset ( i, i );
			LogQuery ( m_dQueries[i], m_dResults[i] );
		}
	}

	// final fixup
	m_dTag2MVA[0] = g_dMvaStorage.GetLength() ? &g_dMvaStorage[0] : NULL;
}


void SearchHandler_c::RunSubset ( int iStart, int iEnd )
{
	// prepare for descent
	float tmStart = sphLongTimer ();

	CSphQuery & tFirst = m_dQueries[iStart];

	m_dFailuresSet.SetSubset ( iStart, iEnd );
	for ( int iRes=iStart; iRes<=iEnd; iRes++ )
		m_dResults[iRes].m_iSuccesses = 0;

	////////////////////////////////////////////////////////////////
	// check for single-query, multi-queue optimization possibility
	////////////////////////////////////////////////////////////////

	bool bMultiQueue = ( iStart<iEnd );
	for ( int iCheck=iStart+1; iCheck<=iEnd; iCheck++ )
	{
		const CSphQuery & qFirst = m_dQueries[iStart];
		const CSphQuery & qCheck = m_dQueries[iCheck];

		// these parameters must be the same
		if (
			( qCheck.m_sQuery!=qFirst.m_sQuery ) || // query string
			( qCheck.m_iWeights!=qFirst.m_iWeights ) || // weights count
			( qCheck.m_pWeights && memcmp ( qCheck.m_pWeights, qFirst.m_pWeights, sizeof(int)*qCheck.m_iWeights ) ) || // weights
			( qCheck.m_eMode!=qFirst.m_eMode ) || // search mode
			( qCheck.m_iMinID!=qFirst.m_iMinID ) || // min-id filter
			( qCheck.m_iMaxID!=qFirst.m_iMaxID ) || // max-id filter
			( qCheck.m_dFilters.GetLength()!=qFirst.m_dFilters.GetLength() ) || // attr filters count
			( qCheck.m_iCutoff!=qFirst.m_iCutoff ) ) // cutoff
		{
			bMultiQueue = false;
			break;
		}

		// filters must be the same too
		assert ( qCheck.m_dFilters.GetLength()==qFirst.m_dFilters.GetLength() );
		ARRAY_FOREACH ( i, qCheck.m_dFilters )
			if ( qCheck.m_dFilters[i]!=qFirst.m_dFilters[i] )
		{
			bMultiQueue = false;
			break;
		}
		if ( !bMultiQueue )
			break;
	}

	////////////////////////////
	// build local indexes list
	////////////////////////////

	CSphVector<CSphString> dLocal;
	DistributedIndex_t * pDist = g_hDistIndexes ( tFirst.m_sIndexes );

	if ( !pDist )
	{
		// they're all local, build the list
		if ( tFirst.m_sIndexes=="*" )
		{
			// search through all local indexes
			g_hIndexes.IterateStart ();
			while ( g_hIndexes.IterateNext () )
				if ( g_hIndexes.IterateGet ().m_bEnabled )
					dLocal.Add ( g_hIndexes.IterateGetKey() );
		} else
		{
			// search through specified local indexes
			ParseIndexList ( tFirst.m_sIndexes, dLocal );
			ARRAY_FOREACH ( i, dLocal )
			{
				// check that it exists
				if ( !g_hIndexes(dLocal[i]) )
				{
					for ( int iRes=iStart; iRes<=iEnd; iRes++ )
						m_dResults[iRes].m_sError.SetSprintf ( "unknown local index '%s' in search request", dLocal[i].cstr() );
					return;
				}

				// if it exists but is not enabled, remove it from the list and force recheck
				if ( !g_hIndexes[dLocal[i]].m_bEnabled )
					dLocal.RemoveFast ( i-- );
			}
		}

		// sanity check
		if ( !dLocal.GetLength() )
		{
			for ( int iRes=iStart; iRes<=iEnd; iRes++ )
				m_dResults[iRes].m_sError.SetSprintf ( "no enabled local indexes to search" );
			return;
		}

	} else
	{
		// copy local indexes list from distributed definition, but filter out disabled ones
		ARRAY_FOREACH ( i, pDist->m_dLocal )
			if ( g_hIndexes[pDist->m_dLocal[i]].m_bEnabled )
				dLocal.Add ( pDist->m_dLocal[i] );
	}

	///////////////////////////////////////////////////////////
	// main query loop (with multiple retries for distributed)
	///////////////////////////////////////////////////////////

	tFirst.m_iRetryCount = Min ( Max ( tFirst.m_iRetryCount, 0 ), MAX_RETRY_COUNT ); // paranoid clamp
	if ( !pDist )
		tFirst.m_iRetryCount = 0;

	for ( int iRetry=0; iRetry<=tFirst.m_iRetryCount; iRetry++ )
	{
		////////////////////////
		// issue remote queries
		////////////////////////

		// delay between retries
		if ( iRetry>0 )
			sphUsleep ( tFirst.m_iRetryDelay );

		// connect to remote agents and query them, if required
		int iRemote = 0;
		if ( pDist )
		{
			m_dFailuresSet.SetIndex ( tFirst.m_sIndexes.cstr() );
			ConnectToRemoteAgents ( pDist->m_dAgents, iRetry!=0, m_dFailuresSet );

			SearchRequestBuilder_t tReqBuilder ( m_dQueries, iStart, iEnd );
			iRemote = QueryRemoteAgents ( pDist->m_dAgents, pDist->m_iAgentConnectTimeout, tReqBuilder, m_dFailuresSet );
		}

		/////////////////////
		// run local queries
		//////////////////////

		// while the remote queries are running, do local searches
		// FIXME! what if the remote agents finish early, could they timeout?
		float tmQuery = 0.0f;
		if ( iRetry==0 )
		{
			if ( pDist && !iRemote && !dLocal.GetLength() )
			{
				for ( int iRes=iStart; iRes<=iEnd; iRes++ )
					m_dResults[iRes].m_sError = "all remote agents unreachable and no available local indexes found";
				return;
			}

			tmQuery = -sphLongTimer ();
			ARRAY_FOREACH ( iLocal, dLocal )
			{
				const ServedIndex_t & tServed = g_hIndexes [ dLocal[iLocal] ];
				assert ( tServed.m_pIndex );
				assert ( tServed.m_pDict );
				assert ( tServed.m_pTokenizer );
				assert ( tServed.m_bEnabled );

				if ( bMultiQueue )
				{
					////////////////////////////////
					// run single multi-queue query
					////////////////////////////////

					CSphVector<int> dSorterIndexes;
					dSorterIndexes.Resize ( iEnd+1 );
					ARRAY_FOREACH ( j, dSorterIndexes )
						dSorterIndexes[j] = -1;

					CSphVector<ISphMatchSorter*> dSorters;

					for ( int iQuery=iStart; iQuery<=iEnd; iQuery++ )
					{
						CSphString sError;
						ISphMatchSorter * pSorter = sphCreateQueue ( &m_dQueries[iQuery], *tServed.m_pSchema, sError );
						if ( !pSorter )
						{
							m_dFailuresSet[iQuery].SubmitEx ( dLocal[iLocal].cstr(), "%s", sError.cstr() );
							continue;
						}

						dSorterIndexes[iQuery] = dSorters.GetLength();
						dSorters.Add ( pSorter );
					}

					if ( dSorters.GetLength() )
					{
						AggrResult_t tStats;
						if ( !tServed.m_pIndex->MultiQuery ( tServed.m_pTokenizer, tServed.m_pDict, &m_dQueries[iStart], &tStats,
							dSorters.GetLength(), &dSorters[0] ) )
						{
							// failed
							for ( int iQuery=iStart; iQuery<=iEnd; iQuery++ )
								m_dFailuresSet[iQuery].SubmitEx ( dLocal[iLocal].cstr(), "%s", tServed.m_pIndex->GetLastError().cstr() );
						} else
						{
							// multi-query succeeded
							for ( int iQuery=iStart; iQuery<=iEnd; iQuery++ )
							{
								// but some of the sorters could had failed at "create sorter" stage
								if ( dSorterIndexes[iQuery]<0 )
									continue;

								// this one seems OK
								ISphMatchSorter * pSorter = dSorters [ dSorterIndexes[iQuery] ];
								AggrResult_t & tRes = m_dResults[iQuery];
								tRes.m_iSuccesses++;

								tRes.m_iTotalMatches += pSorter->GetTotalCount();
								tRes.m_iQueryTime += ( iQuery==iStart ) ? tStats.m_iQueryTime : 0;
								tRes.m_pMva = tStats.m_pMva;

								tRes.m_iNumWords = tStats.m_iNumWords;
								for ( int i=0; i<tRes.m_iNumWords; i++ )
									tRes.m_tWordStats[i] = tStats.m_tWordStats[i];

								tRes.m_tSchema = tStats.m_tSchema;
								tRes.m_tSchema.BuildResultSchema ( &m_dQueries[iQuery] ); 

								// extract matches from sorter
								assert ( pSorter );

								if ( pSorter->GetLength() )
								{
									tRes.m_dMatchCounts.Add ( pSorter->GetLength() );
									tRes.m_dSchemas.Add ( tRes.m_tSchema );
									tRes.m_dIndexWeights.Add ( m_dQueries[iQuery].GetIndexWeight ( dLocal[iLocal].cstr() ) );
									m_dTag2MVA.Add ( tRes.m_pMva );
									sphFlattenQueue ( pSorter, &tRes, m_iTag++ );
								}
							}
						}
						ARRAY_FOREACH ( i, dSorters )
							SafeDelete ( dSorters[i] );
					}

				} else
				{
					////////////////////////////////
					// run local queries one by one
					////////////////////////////////

					for ( int iQuery=iStart; iQuery<=iEnd; iQuery++ )
					{
						CSphQuery & tQuery = m_dQueries[iQuery];
						CSphString sError;

						// fixup old queries
						if ( !FixupQuery ( &tQuery, tServed.m_pSchema, dLocal[iLocal].cstr(), sError ) )
						{
							m_dFailuresSet[iQuery].SubmitEx ( dLocal[iLocal].cstr(), "%s", sError.cstr() );
							continue;
						}

						// create queue
						ISphMatchSorter * pSorter = sphCreateQueue ( &tQuery, *tServed.m_pSchema, sError );
						if ( !pSorter )
						{
							m_dFailuresSet[iQuery].SubmitEx ( dLocal[iLocal].cstr(), "%s", sError.cstr() );
							continue;
						}

						// do query
						AggrResult_t & tRes = m_dResults[iQuery];
						if ( !tServed.m_pIndex->QueryEx ( tServed.m_pTokenizer, tServed.m_pDict, &tQuery, &tRes, pSorter ) )
							m_dFailuresSet[iQuery].SubmitEx ( dLocal[iLocal].cstr(), "%s", tServed.m_pIndex->GetLastError().cstr() );
						else
							tRes.m_iSuccesses++;

						// extract my results and store schema
						if ( pSorter->GetLength() )
						{
							tRes.m_dMatchCounts.Add ( pSorter->GetLength() );
							tRes.m_dSchemas.Add ( tRes.m_tSchema );
							tRes.m_dIndexWeights.Add ( tQuery.GetIndexWeight ( dLocal[iLocal].cstr() ) );
							m_dTag2MVA.Add ( tRes.m_pMva );
							sphFlattenQueue ( pSorter, &tRes, m_iTag++ );
						}
						SafeDelete ( pSorter );
					}
				}
			}
			tmQuery += sphLongTimer ();
		}

		///////////////////////
		// poll remote queries
		///////////////////////

		// wait for remote queries to complete
		if ( iRemote )
		{
			m_dFailuresSet.SetIndex ( tFirst.m_sIndexes.cstr() );

			SearchReplyParser_t tParser ( iStart, iEnd );
			int iMsecLeft = pDist->m_iAgentQueryTimeout - int(tmQuery*1000.0f);
			int iReplys = WaitForRemoteAgents ( pDist->m_dAgents, Max(iMsecLeft,0), tParser, m_dFailuresSet );

			// check if there were valid (though might be 0-matches) replys, and merge them
			if ( iReplys )
				ARRAY_FOREACH ( iAgent, pDist->m_dAgents )
			{
				Agent_t & tAgent = pDist->m_dAgents[iAgent];
				if ( !tAgent.m_bSuccess )
					continue;

				// merge this agent's results
				for ( int iRes=iStart; iRes<=iEnd; iRes++ )
				{
					const CSphQueryResult & tRemoteResult = tAgent.m_dResults[iRes-iStart];
					if ( tRemoteResult.m_iSuccesses<=0 )
						continue;

					AggrResult_t & tRes = m_dResults[iRes];
					tRes.m_iSuccesses++;

					ARRAY_FOREACH ( i, tRemoteResult.m_dMatches )
					{
						tRes.m_dMatches.Add ( tRemoteResult.m_dMatches[i] );
						tRes.m_dMatches.Last().m_iTag = 0; // all remote MVA values go to special pool which is at index 0
					}

					tRes.m_dMatchCounts.Add ( tRemoteResult.m_dMatches.GetLength() );
					tRes.m_dSchemas.Add ( tRemoteResult.m_tSchema );
					// note how we do NOT* add per-index weight here; remote agents are all tagged 0 (which contains weight 1)

					// merge this agent's stats
					tRes.m_iTotalMatches += tRemoteResult.m_iTotalMatches;
					tRes.m_iQueryTime += tRemoteResult.m_iQueryTime;

					// merge this agent's words
					if ( !tRes.m_iNumWords )
					{
						// nothing has been set yet; just copy
						tRes.m_iNumWords = tRemoteResult.m_iNumWords;
						for ( int i=0; i<tRemoteResult.m_iNumWords; i++ )
							tRes.m_tWordStats[i] = tRemoteResult.m_tWordStats[i];

					} else
					{
						// check for word count and contents mismatch
						bool bMismatch = ( tRes.m_iNumWords!=tRemoteResult.m_iNumWords );
						for ( int i=0; i<tRemoteResult.m_iNumWords && !bMismatch; i++ )
							if ( tRes.m_tWordStats[i].m_sWord!=tRemoteResult.m_tWordStats[i].m_sWord )
								bMismatch = true;

						if ( !bMismatch )
						{
							// everything matches, update stats
							for ( int i=0; i<tRemoteResult.m_iNumWords; i++ )
							{
								tRes.m_tWordStats[i].m_iDocs += tRemoteResult.m_tWordStats[i].m_iDocs;
								tRes.m_tWordStats[i].m_iHits += tRemoteResult.m_tWordStats[i].m_iHits;
							}
						} else
						{
							// there are mismatches, warn
							m_dFailuresSet[iRes].SetPrefix ( "agent %s:%d: ", tAgent.m_sHost.cstr(), tAgent.m_iPort );
							m_dFailuresSet[iRes].Submit ( "query words mismatch" );
						}
					}
				}

				// dismissed
				tAgent.m_dResults.Reset ();
			}
		}

		// check if we need to retry again
		int iToRetry = 0;
		if ( pDist )
			ARRAY_FOREACH ( i, pDist->m_dAgents )
				if ( pDist->m_dAgents[i].m_eState==AGENT_RETRY )
					iToRetry++;
		if ( !iToRetry )
			break;
	}

	assert ( m_iTag==m_dTag2MVA.GetLength() );

	/////////////////////
	// merge all results
	/////////////////////

	// warn about id64 server vs old clients
#if USE_64BIT
	if ( m_iClientVer<0x108 )
	{
		m_dFailuresSet.SetIndex ( "*" );
		m_dFailuresSet.SetPrefix ( "" );
		m_dFailuresSet.Submit ( "searchd is id64; resulting docids might be wrapped" );
	}
#endif

	for ( int iRes=iStart; iRes<=iEnd; iRes++ )
	{
		AggrResult_t & tRes = m_dResults[iRes];
		CSphQuery & tQuery = m_dQueries[iRes];

		// if there were no succesful searches at all, this is an error
		if ( !tRes.m_iSuccesses )
		{
			StrBuf_t sFailures;
			m_dFailuresSet[iRes].BuildReport ( sFailures );

			tRes.m_sError = sFailures.cstr();
			continue;
		}

		// minimize schema and remove dupes
		if ( tRes.m_dSchemas.GetLength() )
			tRes.m_tSchema = tRes.m_dSchemas[0];

		if ( tRes.m_iSuccesses>1 )
			if ( !MinimizeAggrResult ( tRes, tQuery ) )
				return;

		if ( !m_dFailuresSet[iRes].IsEmpty() )
		{
			StrBuf_t sFailures;
			m_dFailuresSet[iRes].BuildReport ( sFailures );
			tRes.m_sWarning = sFailures.cstr();
		}

		////////////
		// finalize
		////////////

		tRes.m_iOffset = tQuery.m_iOffset;
		tRes.m_iCount = Max ( Min ( tQuery.m_iLimit, tRes.m_dMatches.GetLength()-tQuery.m_iOffset ), 0 );
		tRes.m_iQueryTime = int ( 1000.0f*( sphLongTimer() - tmStart ) );/* !COMMIT thats batch run time, not query */
	}
}


bool CheckCommandVersion ( int iVer, int iDaemonVersion, InputBuffer_c & tReq )
{
	if ( (iVer>>8)!=(iDaemonVersion>>8) )
	{
		tReq.SendErrorReply ( "major command version mismatch (expected v.%d.x, got v.%d.%d)",
			iDaemonVersion>>8, iVer>>8, iVer&0xff );
		return false;
	}
	if ( iVer>iDaemonVersion )
	{
		tReq.SendErrorReply ( "client version is higher than daemon version (client is v.%d.%d, daemon is v.%d.%d)",
			iVer>>8, iVer&0xff, iDaemonVersion>>8, iDaemonVersion&0xff );
		return false;
	}
	return true;
}


void HandleCommandSearch ( int iSock, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_SEARCH, tReq ) )
		return;

	/////////////////
	// parse request
	/////////////////

	int iQueries = 1;
	if ( iVer>=0x10D )
		iQueries = tReq.GetDword ();

	const int MAX_QUERIES = 32;
	if ( iQueries<=0 || iQueries>MAX_QUERIES )
	{
		tReq.SendErrorReply ( "bad multi-query count %d (must be in 1..%d range)", iQueries, MAX_QUERIES );
		return;
	}

	// create handler
	SearchHandler_c tHandler ( iQueries, iVer );

	// parse all queries to handler
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
		if ( !ParseSearchQuery ( tReq, tHandler.m_dQueries[i], iVer ) )
			return;		

	///////////////////
	// run all queries
	///////////////////

	tHandler.RunQueries ();

	//////////////////////
	// serve the response
	//////////////////////

	NetOutputBuffer_c tOut ( iSock );
	int iReplyLen = 0;

	if ( iVer<=0x10C )
	{
		assert ( tHandler.m_dQueries.GetLength()==1 );
		assert ( tHandler.m_dResults.GetLength()==1 );
		const CSphQueryResult & tRes = tHandler.m_dResults[0];

		if ( !tRes.m_sError.IsEmpty() )
		{
			tReq.SendErrorReply ( "%s", tRes.m_sError.cstr() );
			return;
		}

		iReplyLen = CalcResultLength ( iVer, &tRes, tHandler.m_dTag2MVA );
		bool bWarning = ( iVer>=0x106 && !tRes.m_sWarning.IsEmpty() );

		// send it
		tOut.SendWord ( (WORD)( bWarning ? SEARCHD_WARNING : SEARCHD_OK ) );
		tOut.SendWord ( VER_COMMAND_SEARCH );
		tOut.SendInt ( iReplyLen );

		SendResult ( iVer, tOut, &tRes, tHandler.m_dTag2MVA );

	} else
	{
		ARRAY_FOREACH ( i, tHandler.m_dQueries )
			iReplyLen += CalcResultLength ( iVer, &tHandler.m_dResults[i], tHandler.m_dTag2MVA );

		// send it
		tOut.SendWord ( (WORD)SEARCHD_OK );
		tOut.SendWord ( VER_COMMAND_SEARCH );
		tOut.SendInt ( iReplyLen );

		ARRAY_FOREACH ( i, tHandler.m_dQueries )
			SendResult ( iVer, tOut, &tHandler.m_dResults[i], tHandler.m_dTag2MVA );
	}

	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==iReplyLen+8 );

	////////////
	// clean up
	////////////

	ARRAY_FOREACH ( i, tHandler.m_dQueries )
		SafeDeleteArray ( tHandler.m_dQueries[i].m_pWeights );
}

/////////////////////////////////////////////////////////////////////////////
// EXCERPTS HANDLER
/////////////////////////////////////////////////////////////////////////////

void HandleCommandExcerpt ( int iSock, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_EXCERPT, tReq ) )
		return;

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

	CSphVector<char*> dExcerpts;
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

	virtual bool ParseReply ( MemInputBuffer_c & tReq, Agent_t &, SearchFailuresLogset_c & ) const
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
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_UPDATE, tReq ) )
		return;

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
	CSphVector<CSphString> dIndexNames;
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
	SearchFailuresLogset_c dFailuresSet;
	dFailuresSet.SetSize ( 1 );
	dFailuresSet.SetSubset ( 0, 0 );

	int iSuccesses = 0;
	int iUpdated = 0;
	CSphVector<CSphString> dUpdated;

	ARRAY_FOREACH ( iIdx, dIndexNames )
	{
		const char * sReqIndex = dIndexNames[iIdx].cstr();

		CSphVector<CSphString> dLocal;
		const CSphVector<CSphString> * pLocal = NULL;

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

			dFailuresSet.SetIndex ( sIndex );
			dFailuresSet.SetPrefix ( "" );

			if ( !pServed || !pServed->m_pIndex )
			{
				dFailuresSet.Submit ( "index not available" );
				continue;
			}

			int iUpd = pServed->m_pIndex->UpdateAttributes ( tUpd );
			if ( iUpd<0 )
			{
				dFailuresSet.Submit ( "%s", pServed->m_pIndex->GetLastError().cstr() );

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
			dFailuresSet.SetIndex ( sReqIndex );

			// connect to remote agents and query them
			ConnectToRemoteAgents ( tDist.m_dAgents, false, dFailuresSet );

			UpdateRequestBuilder_t tReqBuilder ( tUpd );
			int iRemote = QueryRemoteAgents ( tDist.m_dAgents, tDist.m_iAgentConnectTimeout, tReqBuilder, dFailuresSet );

			if ( iRemote )
			{
				UpdateReplyParser_t tParser ( &iUpdated );
				iSuccesses += WaitForRemoteAgents ( tDist.m_dAgents, tDist.m_iAgentQueryTimeout, tParser, dFailuresSet );
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
	dFailuresSet[0].BuildReport ( sReport );

	if ( !iSuccesses )
	{
		tReq.SendErrorReply ( "%s", sReport.cstr() );
		return;
	}

	NetOutputBuffer_c tOut ( iSock );
	if ( dFailuresSet[0].IsEmpty() )
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
			sphWarning ( "fcntl(O_NONBLOCK) on pipe failed (error=%s)", strerror(errno) );
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
		g_pPrereading = sphCreateIndexPhrase ( NULL, false ); // FIXME! check if it's ok

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
		sphInfo ( "rotating finished" );
	}
}


/// simple wrapper to simplify reading from pipes
struct PipeReader_t
{
	PipeReader_t ( int iFD )
		: m_iFD ( iFD )
		, m_bError ( false )
	{
#if !USE_WINDOWS
		if ( fcntl ( iFD, F_SETFL, 0 )<0 )
			sphWarning ( "fcntl(0) on pipe failed (error=%s)", strerror(errno) );
#endif
	}

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
		int iRes = ::read ( g_dPipes[i].m_iFD, &uStatus, sizeof(DWORD) );

		// no data yet?
		if ( iRes==-1 && errno==EAGAIN )
			continue;

		// either if there's eof, or error, or valid data - this pipe is over
		PipeReader_t tPipe ( g_dPipes[i].m_iFD );
		bool bPrereader = g_dPipes[i].m_bPrereader;
		g_dPipes.Remove ( i-- );

		// check for eof/error
		if ( iRes!=sizeof(DWORD) )
		{
			// report problems
			if ( iRes!=0 || bPrereader )
				sphWarning ( "pipe status read failed (prereader=%d)", bPrereader );

			// if that was prereader who failed, clean up previous one and launch next one
			if ( bPrereader )
			{
				g_sPrereading = NULL;

				// in any case, buffer index should now be deallocated
				g_pPrereading->Dealloc ();
				g_pPrereading->Unlock ();

				// work next one
				SeamlessForkPrereader ();
			}
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
			assert ( g_bDoRotate && g_bSeamlessRotate && g_sPrereading && bPrereader );

			// whatever the outcome, we will be done with this one
			const char * sPrereading = g_sPrereading;
			g_sPrereading = NULL;

			// notice that this will block!
			int iRes = tPipe.GetInt();
			if ( !tPipe.IsError() && iRes )
			{
				// if preread was succesful, exchange served index and prereader buffer index
				ServedIndex_t & tServed = g_hIndexes[sPrereading];
				CSphIndex * pOld = tServed.m_pIndex;
				CSphIndex * pNew = g_pPrereading;

				char sOld [ SPH_MAX_FILENAME_LEN ];
				snprintf ( sOld, sizeof(sOld), "%s.old", tServed.m_sIndexPath.cstr() );

				if ( !pOld->Rename ( sOld ) )
				{
					// FIXME! rollback inside Rename() call potentially fail
					sphWarning ( "rotating index '%s': cur to old rename failed: %s", sPrereading, pOld->GetLastError().cstr() );
					continue;
				}
				// FIXME! at this point there's no cur lock file; ie. potential race
				if ( !pNew->Rename ( tServed.m_sIndexPath.cstr() ) )
				{
					sphWarning ( "rotating index '%s': new to cur rename failed: %s", sPrereading, pNew->GetLastError().cstr() );
					if ( !pOld->Rename ( tServed.m_sIndexPath.cstr() ) )
					{
						sphWarning ( "rotating index '%s': old to cur rename failed: %s; INDEX UNUSABLE", sPrereading, pOld->GetLastError().cstr() );
						tServed.m_bEnabled = false;
					}
					continue;
				}

				// all went fine; swap them
				Swap ( tServed.m_pIndex, g_pPrereading );
				tServed.m_pSchema = tServed.m_pIndex->GetSchema ();
				tServed.m_bEnabled = true;
				sphInfo ( "rotating index '%s': success", sPrereading );

			} else
			{
				if ( tPipe.IsError() )
					sphWarning ( "rotating index '%s': pipe read failed" );
				else
					sphWarning ( "rotating index '%s': preread failure reported" );
			}

			// in any case, buffer index should now be deallocated
			g_pPrereading->Dealloc ();
			g_pPrereading->Unlock ();

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
		sphInfo ( "rotating finished" );
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
	CSphVector<char *> dArgs;
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

			// configure memlocking, star
			if ( hIndex("mlock") && hIndex["mlock"].intval() )
				tIdx.m_bMlock = true;
			if ( hIndex("enable_star") && hIndex["enable_star"].intval() )
				tIdx.m_bStar = true;

			// try to create index
			CSphString sWarning;
			tIdx.m_pIndex = sphCreateIndexPhrase ( hIndex["path"].cstr(), tIdx.m_bStar );
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
			if ( !tServed.m_bEnabled )
				continue;

			// obtain exclusive lock
			if ( !tServed.m_pIndex->Lock() )
			{
				sphWarning ( "index '%s': lock: %s; INDEX UNUSABLE", g_hIndexes.IterateGetKey().cstr(),
					tServed.m_pIndex->GetLastError().cstr() );
				tServed.m_bEnabled = false;
				continue;
			}

			// try to mlock again because mlock does not survive over fork
			if ( !tServed.m_pIndex->Mlock() )
			{
				sphWarning ( "index '%s': %s", g_hIndexes.IterateGetKey().cstr(),
					tServed.m_pIndex->GetLastError().cstr() );
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

		if ( g_bGotSighup )
		{
			sphInfo ( "rotating indices (seamless=%d)", (int)g_bSeamlessRotate ); // this might hang if performed from SIGHUP
			g_bGotSighup = false;
		}

		if ( g_bGotSigterm )
		{
			assert ( g_bHeadDaemon );
			sphInfo ( "caught SIGTERM, shutting down" );

			Shutdown ();
			exit ( 0 );
		}

#if !USE_WINDOWS
		if ( g_bGotSigchld )
		{
			while ( waitpid ( 0, NULL, WNOHANG ) > 0 )
				g_iChildren--;
			g_bGotSigchld = false;
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
