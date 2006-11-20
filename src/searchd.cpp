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
#include <limits.h>

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

	// for cache
	#include <zlib.h>
	#include <sys/mman.h>
	#include <md5.h>

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

static bool				g_bCacheEnable	= false;
static CSphString		g_sCacheDir ( "qcache" );
static int				g_iCacheTTL		= 300;
static bool				g_bCacheGzip	= false;

static int				g_iReadTimeout	= 5;	// sec
static int				g_iChildren		= 0;
static int				g_iMaxChildren	= 0;
static int				g_iSocket		= 0;
static int				g_iQueryLogFile	= -1;
static int				g_iHUP			= 0;
static const char *		g_sPidFile		= NULL;
static bool				g_bHeadDaemon	= false;
static int				g_iMaxMatches	= 1000;

struct ServedIndex_t
{
	CSphIndex *			m_pIndex;
	const CSphSchema *	m_pSchema;	///< pointer to index schema, managed by the index itself
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
		if ( m_pLockFile && g_bHeadDaemon )
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
	VER_COMMAND_SEARCH		= 0x104,
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

void sphLockEx ( int iFile )
{
	#if HAVE_LOCK_EX || USE_WINDOWS
		flock ( iFile, LOCK_EX );
	#else
		#ifdef HAVE_F_SETLKW
			fcntl ( iFile, F_SETLKW, F_WRLCK );
		#endif
	#endif
}


void sphLockUn ( int iFile )
{
	#if HAVE_LOCK_EX || USE_WINDOWS
		flock ( iFile, LOCK_UN );
	#else
		#ifdef HAVE_F_SETLK
			fcntl ( iFile, F_SETLKW, F_UNLCK );
		#endif
	#endif
}

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
		snprintf ( sBuf, sizeof(sBuf)-1, "[%s] [%5d] %s", sTimeBuf, (int)getpid(), sBanner );
	else
		strcpy ( sBuf, sBanner );
	int iLen = strlen(sBuf);

	vsnprintf ( sBuf+iLen, sizeof(sBuf)-iLen-1, sFmt, ap );
	strncat ( sBuf, "\n", sizeof(sBuf) );

	sphLockEx ( g_iLogFile );
	lseek ( g_iLogFile, 0, SEEK_END );
	write ( g_iLogFile, sBuf, strlen(sBuf) );
	sphLockUn ( g_iLogFile );

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

const int		NET_MAX_REQ_LEN			= 1048576;
const int		NET_MAX_STR_LEN			= NET_MAX_REQ_LEN;
const int		SEARCHD_MAX_ATTRS		= 256;
const int		SEARCHD_MAX_ATTR_VALUES	= 4096;


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
		sleep ( 15 );
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
	bool		SendInt ( int iValue )		{ return SendT<int> ( htonl ( iValue ) ); }
	bool		SendDword ( DWORD iValue )	{ return SendT<DWORD> ( htonl ( iValue ) ); }
	bool		SendWord ( WORD iValue )	{ return SendT<WORD> ( htons ( iValue ) ); }
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

	template < typename T > bool	SendT ( T tValue );
	bool							SendBytes ( const void * pBuf, int iLen );
};


/// generic request buffer
class InputBuffer_c
{
public:
					InputBuffer_c ();
	virtual			~InputBuffer_c ();
	int				GetInt () { return ntohl ( GetT<int> () ); }
	WORD			GetWord () { return ntohs ( GetT<WORD> () ); }
	DWORD			GetDword () { return ntohl ( GetT<DWORD> () ); }
	BYTE			GetByte () { return GetT<BYTE> (); }
	CSphString		GetString ();
	int				GetDwords ( DWORD ** pBuffer, int iMax, const char * sErrorTemplate );
	bool			GetError () { return m_bError; }

	virtual void	SendErrorReply ( const char *, ... ) = 0;

protected:
	bool			m_bError;
	BYTE *			m_pBuf;
	int				m_iLen;
	BYTE *			m_pCur;

protected:
	void						SetError ( bool bError ) { m_bError = bError; }
	bool						GetBytes ( void * pBuf, int iLen );
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
{
	m_pBuf = new BYTE [ NET_MAX_REQ_LEN ];
	m_pCur = m_pBuf;
}


InputBuffer_c::~InputBuffer_c ()
{
	SafeDeleteArray ( m_pBuf );
}


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
	if ( m_bError || iLen<0 || iLen>NET_MAX_STR_LEN || ( m_pCur+iLen > m_pBuf+m_iLen ) )
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
	: m_iSock ( iSock )
{
}


bool NetInputBuffer_c::ReadFrom ( int iLen )
{
	assert ( iLen>0 );
	assert ( iLen<=NET_MAX_REQ_LEN );
	assert ( m_iSock>0 );

	m_pCur = m_pBuf;

	int iGot = sphSockRead ( m_iSock, m_pBuf, iLen );
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

	memcpy ( m_pBuf, sFrom, iLen );
	m_iLen = iLen;
	m_pCur = m_pBuf;
	m_bError = false;
}

/////////////////////////////////////////////////////////////////////////////
// SIMPLE FILE-BASED QUERY CACHE
/////////////////////////////////////////////////////////////////////////////

#if !USE_WINDOWS

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
		sIndexName, m_sCacheFileName[0], m_sCacheFileName[1],  m_sCacheFileName[2], m_sCacheFileName[3], m_sCacheFileName );

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

	int iLen = snprintf ( sBuf, sizeof(sBuf), "%s-%d-%d-%d-%d",
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
	bool			m_bFailure;		///< whether query was succesful

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
		, m_bFailure ( false )
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
				iRemoteVer = ntohl ( iRemoteVer );
				if ( iRes!=sizeof(iRemoteVer) || iRemoteVer<=0 )
				{
					sphWarning ( "index '%s': agent '%s:%d': expected protocol v.%d, got v.%d",
						sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
						SPHINX_SEARCHD_PROTO, iRemoteVer );
					tAgent.Close ();
					continue;
				}

				// do query!
				int iReqSize = 68 + 4*tQuery.m_iWeights
					+ strlen ( tQuery.m_sSortBy.cstr() )
					+ strlen ( tQuery.m_sQuery.cstr() )
					+ strlen ( tAgent.m_sIndexes.cstr() )
					+ strlen ( tQuery.m_sGroupBy.cstr() );
				ARRAY_FOREACH ( j, tQuery.m_dFilters )
				{
					const CSphFilter & tFilter = tQuery.m_dFilters[j];
					iReqSize +=
						8
						+ strlen ( tFilter.m_sAttrName.cstr() )
						+ 4*tFilter.m_iValues
						+ ( tFilter.m_iValues ? 0 : 8 );
				}

				NetOutputBuffer_c tOut ( tAgent.m_iSock );

				// say hello
				tOut.SendDword ( SPHINX_SEARCHD_PROTO );

				// request header
				tOut.SendWord ( SEARCHD_COMMAND_SEARCH ); // command id
				tOut.SendWord ( VER_COMMAND_SEARCH ); // command version
				tOut.SendInt ( iReqSize-12 ); // request body length

				// request v.1.3
				tOut.SendInt ( 0 ); // offset is 0
				tOut.SendInt ( g_iMaxMatches ); // limit is MAX_MATCHES
				tOut.SendInt ( iMode ); // match mode
				tOut.SendInt ( tQuery.m_eSort ); // sort mode
				tOut.SendString ( tQuery.m_sSortBy.cstr() ); // sort attr
				tOut.SendString ( tQuery.m_sQuery.cstr() ); // query
				tOut.SendInt ( tQuery.m_iWeights );
				for ( int j=0; j<tQuery.m_iWeights; j++ )
					tOut.SendInt ( tQuery.m_pWeights[j] ); // weights
				tOut.SendString ( tAgent.m_sIndexes.cstr() ); // indexes
				tOut.SendInt ( tQuery.m_iMinID ); // id/ts ranges
				tOut.SendInt ( tQuery.m_iMaxID );
				tOut.SendInt ( tQuery.m_dFilters.GetLength() );
				ARRAY_FOREACH ( j, tQuery.m_dFilters )
				{
					const CSphFilter & tFilter = tQuery.m_dFilters[j];
					tOut.SendString ( tFilter.m_sAttrName.cstr() );
					tOut.SendInt ( tFilter.m_iValues );
					for ( int k=0; k<tFilter.m_iValues; k++ )
						tOut.SendInt ( tFilter.m_pValues[k] );
					if ( !tFilter.m_iValues )
					{
						tOut.SendDword ( tFilter.m_uMinValue );
						tOut.SendDword ( tFilter.m_uMaxValue );
					}
				}
				tOut.SendInt ( tQuery.m_eGroupFunc );
				tOut.SendString ( tQuery.m_sGroupBy.cstr() );
				tOut.SendInt ( tQuery.m_iMaxMatches );
				tOut.Flush ();

				// FIXME! handle flush failure
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

					tReplyHeader.m_iStatus = ntohs ( tReplyHeader.m_iStatus );
					tReplyHeader.m_iVer = ntohs ( tReplyHeader.m_iVer );
					tReplyHeader.m_iLength = ntohl ( tReplyHeader.m_iLength );

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

					// get schema
					CSphSchema & tSchema = tAgent.m_tRes.m_tSchema;

					tSchema.m_dFields.Resize ( tReq.GetInt() ); // FIXME! add a sanity check
					ARRAY_FOREACH ( j, tSchema.m_dFields )
						tSchema.m_dFields[j].m_sName = tReq.GetString ();

					tSchema.m_dAttrs.Resize ( tReq.GetInt() ); // FIXME! add a sanity check
					ARRAY_FOREACH ( j, tSchema.m_dAttrs )
					{
						tSchema.m_dAttrs[j].m_sName = tReq.GetString ();
						tSchema.m_dAttrs[j].m_eAttrType = (ESphAttrType) tReq.GetDword (); // FIXME! add a sanity check
					}

					// get matches
					int iMatches = tReq.GetInt ();
					if ( iMatches<0 || iMatches>g_iMaxMatches )
					{
						sphWarning ( "index '%s': agent '%s:%d': invalid match count received (count=%d)",
							sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
							iMatches );
						break;
					}

					assert ( !tAgent.m_tRes.m_dMatches.GetLength() );
					int iAttrs = tSchema.m_dAttrs.GetLength();
					if ( iMatches )
					{
						tAgent.m_tRes.m_dMatches.Resize ( iMatches );
						ARRAY_FOREACH ( i, tAgent.m_tRes.m_dMatches )
						{
							CSphMatch & tMatch = tAgent.m_tRes.m_dMatches[i];
							tMatch.Reset ( iAttrs );

							tMatch.m_iDocID = tReq.GetInt ();
							tMatch.m_iWeight = tReq.GetInt ();
							for ( int j=0; j<iAttrs; j++ )
								tMatch.m_pAttrs[j] = tReq.GetDword ();
						}
					}

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

			tAgent.m_bFailure = bFailure;
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

bool CheckSortAndSchema ( const CSphSchema ** ppFirst, ISphMatchQueue ** ppTop, CSphQuery & tQuery,
	const CSphSchema * pServed, const char * sServedName, InputBuffer_c & tReq )
{
	assert ( ppFirst );
	assert ( pServed );

	if ( !*ppFirst )
	{
		// lookup proper attribute index to sort by
		*ppFirst = pServed;

		// lookup proper attribute index to group by
		if ( !tQuery.SetSchema ( *pServed ) )
		{
			tReq.SendErrorReply ( "index '%s': group-by attribute '%s' not found",
				sServedName, tQuery.m_sGroupBy.cstr() );
			return false;
		}
		assert ( tQuery.m_sGroupBy.IsEmpty() || tQuery.GetGroupByAttr()>=0 );

		// spawn queue and set sort-by attribute
		assert ( !*ppTop );

		CSphString sError;
		*ppTop = sphCreateQueue ( &tQuery, *pServed, sError );

		if (! (*ppTop) )
		{
			tReq.SendErrorReply ( "index '%s': failed to create sorting queue: %s",
				sServedName, sError.cstr() );
			return false;
		}

	} else
	{
		assert ( *ppTop );

		// check schemas
		CSphString sError;
		ESphSchemaCompare eComp = pServed->CompareTo ( **ppFirst, sError );
		if ( eComp==SPH_SCHEMAS_INCOMPATIBLE )
		{
			tReq.SendErrorReply ( "index '%s': incompatible schemas: %s", sServedName, sError.cstr() );
			return false;
		}
		// FIXME!!! warn if schemas are compatible but not equal!
	}
	return true;
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

	DWORD iMinTS = 0, iMaxTS = UINT_MAX, iMinGID = 0, iMaxGID = UINT_MAX, * pGroups = NULL;
	int iGroups = 0;

	// v.1.0. mode, limits, weights, ID/TS ranges
	int iOffset			= tReq.GetInt ();
	int iLimit			= tReq.GetInt ();
	tQuery.m_eMode		= (ESphMatchMode) tReq.GetInt ();
	tQuery.m_eSort		= (ESphSortOrder) tReq.GetInt ();
	if ( iVer<=0x101 )
		iGroups			= tReq.GetDwords ( &pGroups, SEARCHD_MAX_ATTR_VALUES, "invalid group count %d (should be in 0..%d range)" );
	if ( iVer>=0x102 )
		tQuery.m_sSortBy= tReq.GetString ();
	tQuery.m_sQuery		= tReq.GetString ();
	tQuery.m_iWeights	= tReq.GetDwords ( (DWORD**)&tQuery.m_pWeights, SPH_MAX_FIELDS, "invalid weight count %d (should be in 0..%d range)" );
	CSphString sIndex	= tReq.GetString ();
	tQuery.m_iMinID		= tReq.GetDword ();
	tQuery.m_iMaxID		= tReq.GetDword ();

	// upto v.1.1
	if ( iVer<=0x101 )
	{
		iMinTS = tReq.GetDword ();
		iMaxTS = tReq.GetDword ();
	}

	// v.1.1 specific
	if ( iVer==0x101 )
	{
		iMinGID = tReq.GetDword ();
		iMaxGID = tReq.GetDword ();
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
			tFilter.m_iValues = tReq.GetDwords ( &tFilter.m_pValues, SEARCHD_MAX_ATTR_VALUES,
				"invalid attribute set length %d (should be in 0..%d range)" );
			if ( !tFilter.m_iValues )
			{
				// 0 length means this is range, not set
				tFilter.m_uMinValue = tReq.GetDword ();
				tFilter.m_uMaxValue = tReq.GetDword ();
			}
		}
	}

	// v.1.3
	if ( iVer>=0x103 )
	{
		tQuery.m_eGroupFunc = (ESphGroupBy) tReq.GetDword ();
		tQuery.m_sGroupBy = tReq.GetString ();
	}

	// v.1.4
	tQuery.m_iMaxMatches = g_iMaxMatches;
	if ( iVer>=0x104 )
	{
		tQuery.m_iMaxMatches = tReq.GetInt ();
		if ( tQuery.m_iMaxMatches<1 || tQuery.m_iMaxMatches>g_iMaxMatches )
		{
			tReq.SendErrorReply ( "per-query max_matches=%d out of bounds (per-server max_matches=%d)",
				tQuery.m_iMaxMatches, g_iMaxMatches );
			return;
		}
	}

	// additional checks
	if ( tReq.GetError() )
	{
		tReq.SendErrorReply ( "invalid or truncated request" );
		return;
	}
	if ( tQuery.m_iMinID>tQuery.m_iMaxID || iMinTS>iMaxTS )
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
	float tmStart = sphLongTimer ();

	const CSphSchema * pFirst = NULL;
	CSphQueryResult * pRes = new CSphQueryResult ();
	ISphMatchQueue * pTop = NULL;

#define REMOVE_DUPES 1

	int iSearched = 0;

	if ( g_hDistIndexes(sIndex) )
	{
		// search through specified distributed index
		DistributedIndex_t & tDist = g_hDistIndexes[sIndex];

		// start connecting to remote agents
		ARRAY_FOREACH ( i, tDist.m_dAgents )
			ConnectToRemoteAgent ( &tDist.m_dAgents[i] );

		// connect to remote agents and query them first
		int iRemote = QueryRemoteAgents ( sIndex.cstr(), tDist, tQuery, tQuery.m_eMode );

		// while the remote queries are running, do local searches
		// FIXME! what if the remote agents finish early, could they timeout?
		float tmQuery = -sphLongTimer ();
		ARRAY_FOREACH ( i, tDist.m_dLocal )
		{
			const ServedIndex_t & tServed = g_hIndexes [ tDist.m_dLocal[i] ];
			assert ( tServed.m_pIndex );
			assert ( tServed.m_pDict );
			assert ( tServed.m_pTokenizer );

			// check/set sort-by attr and schema
			if ( !CheckSortAndSchema ( &pFirst, &pTop, tQuery, tServed.m_pSchema, tDist.m_dLocal[i].cstr(), tReq ) )
				return;

			// do query
			tQuery.m_pTokenizer = tServed.m_pTokenizer;
			tServed.m_pIndex->QueryEx ( tServed.m_pDict, &tQuery, pRes, pTop );

#if REMOVE_DUPES
			// group-by queries remove dupes themselves
			if ( tQuery.GetGroupByAttr()<0 )
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
			{
				Agent_t & tAgent = tDist.m_dAgents[iAgent];
				if ( tAgent.m_bFailure )
					continue;

				// merge this agent's results
				// FIXME! should check schema before; but sometimes it's empty
				if ( !tAgent.m_tRes.m_dMatches.GetLength() )
					continue;

				// check/set sort-by attr and schema
				char sName [ 1024 ];
				snprintf ( sName, sizeof(sName), "%s:%d:%s",
					tAgent.m_sHost.cstr(), tAgent.m_iPort, tAgent.m_sIndexes.cstr() );

				if ( !CheckSortAndSchema ( &pFirst, &pTop, tQuery, &tAgent.m_tRes.m_tSchema, sName, tReq ) )
					return;

				if ( tQuery.GetGroupByAttr()<0 ) 
				{
					ARRAY_FOREACH ( i, tAgent.m_tRes.m_dMatches )
						pRes->m_dMatches.Add ( tAgent.m_tRes.m_dMatches[i] );
				} else
				{
					ARRAY_FOREACH ( i, tAgent.m_tRes.m_dMatches )
						pTop->Push ( tAgent.m_tRes.m_dMatches[i] );
				}
				tAgent.m_tRes.m_dMatches.Reset ();

				// merge this agent's stats
				pRes->m_iTotalMatches += tAgent.m_tRes.m_iTotalMatches;
			}
		}

		// if there were no local indexes, schema in pRes was not yet set,
		// so we have to copy it from first available agent schema
		if ( !tDist.m_dLocal.GetLength() && pRes->m_iTotalMatches )
		{
			assert ( pFirst );
			assert ( pRes->m_tSchema.m_dAttrs.GetLength()==0 );
			assert ( pRes->m_tSchema.m_dFields.GetLength()==0 );
			pRes->m_tSchema = *pFirst;
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

			// check/set sort-by attr and schema
			if ( !CheckSortAndSchema ( &pFirst, &pTop, tQuery, tServed.m_pSchema, g_hIndexes.IterateGetKey().cstr(), tReq ) )
				return;

			// do query
			tQuery.m_pTokenizer = tServed.m_pTokenizer;
			tServed.m_pIndex->QueryEx ( tServed.m_pDict, &tQuery, pRes, pTop );
			iSearched++;

#if REMOVE_DUPES
			// group-by queries remove dupes themselves
			if ( tQuery.GetGroupByAttr()<0 )
				sphFlattenQueue ( pTop, pRes );
#endif
		}

	} else
	{
		// search through the specified local indexes
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

#if !USE_WINDOWS
			CSphCache tCache ( g_sCacheDir.cstr(), g_iCacheTTL, g_bCacheGzip );
			if ( !g_bCacheEnable
				|| !tCache.ReadFromFile ( tQuery, sNext, tServed.m_pIndexPath->cstr(), pRes ) )
#endif
			{
				// check/set sort-by attr and schema
				if ( !CheckSortAndSchema ( &pFirst, &pTop, tQuery, tServed.m_pSchema, sNext, tReq ) )
					return;

				// do query
				tQuery.m_pTokenizer = tServed.m_pTokenizer;
				tServed.m_pIndex->QueryEx ( tServed.m_pDict, &tQuery, pRes, pTop );

#if REMOVE_DUPES
				// group-by queries remove dupes themselves
				if ( tQuery.GetGroupByAttr()<0 )
					sphFlattenQueue ( pTop, pRes );
#endif

#if !USE_WINDOWS
				if ( g_bCacheEnable )
					tCache.StoreResult ( tQuery, sNext, pRes );
#endif
			}

			iSearched++;
		}

		if ( !iSearched )
		{
			tReq.SendErrorReply ( "no valid indexes specified in request" );
			return;
		}
	}

#if REMOVE_DUPES
	if ( tQuery.GetGroupByAttr()>=0 )
	{
		// group-by queries remove dupes themselves, so just flatten
		sphFlattenQueue ( pTop, pRes );

	} else if ( iSearched!=1 )
	{
		// if there was only 1 index searched, it's already properly flattened
		pRes->m_dMatches.Sort ();

		ARRAY_FOREACH ( i, pRes->m_dMatches )
		{
			if ( i==0 || pRes->m_dMatches[i].m_iDocID!=pRes->m_dMatches[i-1].m_iDocID )
				pTop->Push ( pRes->m_dMatches[i] );
		}
		pRes->m_dMatches.Reset ();

		sphFlattenQueue ( pTop, pRes );
	}
#else
	sphFlattenQueue ( pTop, pRes );
#endif

	pRes->m_iQueryTime = int ( 1000.0f*( sphLongTimer() - tmStart ) );

	// log query
	if ( g_iQueryLogFile>=0 )
	{
		time_t tNow;
		char sTimeBuf[128], sGroupBuf[128];
		char sBuf[1024];

		time ( &tNow );
		ctime_r ( &tNow, sTimeBuf );
		sTimeBuf [ strlen(sTimeBuf)-1 ] = '\0';

		sGroupBuf[0] = '\0';
		if ( tQuery.GetGroupByAttr()>=0 )
			snprintf ( sGroupBuf, sizeof(sGroupBuf), " @%s", tQuery.m_sGroupBy.cstr() );
 
		static const char * sModes [ SPH_MATCH_TOTAL ] = { "all", "any", "phr", "bool" };
		static const char * sSort [ SPH_SORT_TOTAL ] = { "rel", "attr-", "attr+", "tsegs", "ext" };

		snprintf ( sBuf, sizeof(sBuf), "[%s] %d.%03d sec [%s/%d/%s %d (%d,%d)%s] [%s] %s\n",
			sTimeBuf, pRes->m_iQueryTime/1000, pRes->m_iQueryTime%1000,
			sModes [ tQuery.m_eMode ], tQuery.m_dFilters.GetLength(), sSort [ tQuery.m_eSort ],
			pRes->m_iTotalMatches, iOffset, iLimit, sGroupBuf,
			sIndex.cstr(), tQuery.m_sQuery.cstr() );

		sphLockEx ( g_iQueryLogFile );
		lseek ( g_iQueryLogFile, 0, SEEK_END );
		write ( g_iQueryLogFile, sBuf, strlen(sBuf) );
		sphLockUn ( g_iQueryLogFile );
	}

	//////////////////////
	// serve the response
	//////////////////////

	// calc response length
	int iRespLen = 20; // header

	if ( iVer>=0x102 ) // schema
	{
		iRespLen += 8; // 4 for field count, 4 for attr count
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dFields )
			iRespLen += 4 + strlen ( pRes->m_tSchema.m_dFields[i].m_sName.cstr() ); // namelen, name
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dAttrs )
			iRespLen += 8 + strlen ( pRes->m_tSchema.m_dAttrs[i].m_sName.cstr() ); // namelen, name, type
	}

	int iCount = Max ( Min ( iLimit, pRes->m_dMatches.GetLength()-iOffset ), 0 );
	if ( iVer<=0x101 )
		iRespLen += 16*iCount; // matches
	else
		iRespLen += ( 8+4*pRes->m_tSchema.m_dAttrs.GetLength() )*iCount; // matches

	for ( int i=0; i<pRes->m_iNumWords; i++ ) // per-word stats
		iRespLen += 12 + strlen ( pRes->m_tWordStats[i].m_sWord.cstr() ); // wordlen, word, docs, hits


	// send header
	NetOutputBuffer_c tOut ( iSock );
	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_SEARCH );
	tOut.SendInt ( iRespLen );

	// send schema
	if ( iVer>=0x102 )
	{
		tOut.SendInt ( pRes->m_tSchema.m_dFields.GetLength() );
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dFields )
			tOut.SendString ( pRes->m_tSchema.m_dFields[i].m_sName.cstr() );

		tOut.SendInt ( pRes->m_tSchema.m_dAttrs.GetLength() );
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dAttrs )
		{
			tOut.SendString ( pRes->m_tSchema.m_dAttrs[i].m_sName.cstr() );
			tOut.SendDword ( (DWORD)pRes->m_tSchema.m_dAttrs[i].m_eAttrType );
		}
	}

	// send matches
	int iGIDIndex = -1;
	int iTSIndex = -1;
	if ( iVer<=0x101 )
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dAttrs )
	{
		if ( iTSIndex<0 && pRes->m_tSchema.m_dAttrs[i].m_eAttrType==SPH_ATTR_TIMESTAMP )
			iTSIndex = i;
		if ( iGIDIndex<0 && pRes->m_tSchema.m_dAttrs[i].m_eAttrType==SPH_ATTR_INTEGER )
			iGIDIndex = i;
	}

	tOut.SendInt ( iCount );
	for ( int i=0; i<iCount; i++ )
	{
		const CSphMatch & tMatch = pRes->m_dMatches[iOffset+i];
		tOut.SendDword ( tMatch.m_iDocID );
		if ( iVer<=0x101 )
		{
			tOut.SendDword ( iGIDIndex>=0 ? tMatch.m_pAttrs[iGIDIndex] : 1 );
			tOut.SendDword ( iTSIndex>=0 ? tMatch.m_pAttrs[iTSIndex] : 1 );
			tOut.SendInt ( tMatch.m_iWeight );
		} else
		{
			tOut.SendInt ( tMatch.m_iWeight );
			assert ( tMatch.m_iAttrs==pRes->m_tSchema.m_dAttrs.GetLength() );
			for ( int j=0; j<tMatch.m_iAttrs; j++ )
				tOut.SendDword ( tMatch.m_pAttrs[j] );
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

	tReq.GetInt (); // mode field is for now reserved and ignored
	int iFlags = tReq.GetInt ();
	CSphString sIndex = tReq.GetString ();

	if ( !g_hIndexes ( sIndex ) )
	{
		tReq.SendErrorReply ( "invalid local index '%s' specified in request", sIndex.cstr() );
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


void HandleClient ( int iSock, const char * sClientIP )
{
	NetInputBuffer_c tBuf ( iSock );

	// send my version
	DWORD uServer = SPHINX_SEARCHD_PROTO;
	if ( sphSockSend ( iSock, (char*)&uServer, sizeof(DWORD), 0 )!=sizeof(DWORD) )
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
		|| iLength<=0 || iLength>NET_MAX_REQ_LEN )
	{
		// unknown command, default response header
		tBuf.SendErrorReply ( "unknown command (code=%d)", iCommand );

		// if request length is insane, low level comm is broken, so we bail out
		if ( iLength<=0 || iLength>NET_MAX_REQ_LEN )
		{
			sphWarning ( "ill-formed client request (length=%d out of bounds)", iLength );
			return;
		}
	}

	// get request body
	assert ( iLength>0 && iLength<=NET_MAX_REQ_LEN );
	if ( !tBuf.ReadFrom ( iLength ) )
	{
		sphWarning ( "failed to receive client request body (client=%s)", sClientIP );
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


bool RotateIndex ( ServedIndex_t & tIndex, const char * sIndex )
{
	char sFile [ SPH_MAX_FILENAME_LEN ];
	const char * sPath = tIndex.m_pIndexPath->cstr();

	// whatever happens, we won't retry
	g_iHUP = 0;
	sphInfo ( "rotating index '%s': children exited, trying to rotate", sIndex );

	// check files
	const int EXT_COUNT = 4;
	const char * dNew[EXT_COUNT] = { ".new.sph", ".new.spa", ".new.spi", ".new.spd" };
	const char * dOld[EXT_COUNT] = { ".old.sph", ".old.spa", ".old.spi", ".old.spd" };
	const char * dCur[EXT_COUNT] = { ".sph", ".spa", ".spi", ".spd" };

	for ( int i=0; i<EXT_COUNT; i++ )
	{
		snprintf ( sFile, sizeof(sFile), "%s%s", sPath, dNew[i] );
		if ( !IsReadable ( sFile ) )
		{
			sphWarning ( "rotating index '%s': '%s' unreadable: %s",
				sIndex, sFile, strerror(errno) );
			return false;
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
		return false;
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

	// try to create new index
	CSphIndex * pNewIndex = sphCreateIndexPhrase ( sPath );
	const CSphSchema * pNewSchema = pNewIndex ? pNewIndex->Preload() : NULL;
	if ( !pNewIndex || !pNewSchema )
	{
		if ( !pNewIndex )
			sphWarning ( "rotating index '%s': failed to create new index object", sIndex );
		else
			sphWarning ( "rotating index '%s': failed to preload schema/docinfo", sIndex );

		// try to recover
		for ( int j=0; j<EXT_COUNT; j++ )
		{
			TryRename ( sIndex, sPath, dCur[j], dNew[j], true );
			TryRename ( sIndex, sPath, dOld[j], dCur[j], true );
		}
		return false;
	}

	// uff. all done
	SafeDelete ( tIndex.m_pIndex );
	tIndex.m_pIndex = pNewIndex;
	tIndex.m_pSchema = pNewSchema;

	sphInfo ( "rotating index '%s': success", sIndex );
	return true;
}

/////////////////////////////////////////////////////////////////////////////

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
					sphWarning ( "index '%s': no such local index '%s' - NOT SERVING",
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
					sphWarning ( "index '%s': agent '%s': host name expected - NOT SERVING",
						sIndexName, pAgent->cstr() );
					continue;
				}
				if ( *p++!=':' )
				{
					sphWarning ( "index '%s': agent '%s': colon expected near '%s' - NOT SERVING",
						sIndexName, pAgent->cstr(), p );
					continue;
				}
				tAgent.m_sHost = pAgent->SubString ( 0, p-1-pAgent->cstr() );

				// extract port
				if ( !isdigit(*p) )
				{
					sphWarning ( "index '%s': agent '%s': port number expected near '%s' - NOT SERVING",
						sIndexName, pAgent->cstr(), p );
					continue;
				}
				tAgent.m_iPort = atoi(p);
				if ( tAgent.m_iPort<=0 || tAgent.m_iPort>=65536 )
				{
					sphWarning ( "index '%s': agent '%s': invalid port number near '%s' - NOT SERVING",
						sIndexName, pAgent->cstr(), p );
					continue;
				}

				// extract index list
				while ( isdigit(*p) ) p++;
				if ( *p++!=':' )
				{
					sphWarning ( "index '%s': agent '%s': colon expected near '%s' - NOT SERVING",
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
					sphWarning ( "index '%s': agent '%s': index list expected near '%s' - NOT SERVING",
						sIndexName, pAgent->cstr(), p );
					continue;
				}
				tAgent.m_sIndexes = sIndexList;

				// lookup address
				struct hostent * hp = gethostbyname ( tAgent.m_sHost.cstr() );
				if ( !hp )
				{
					sphWarning ( "index '%s': agent '%s': failed to lookup host name - NOT SERVING",
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

			// check path
			if ( !hIndex.Exists ( "path" ) )
			{
				sphWarning ( "index '%s': key 'path' not found' - NOT SERVING", sIndexName );
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
					sphWarning ( "index '%s': unknown charset type '%s' - NOT SERVING",
						sIndexName, hIndex["charset_type"].cstr() );
					continue;
				}
			} else
			{
				pTokenizer = sphCreateSBCSTokenizer ();
			}
			assert ( pTokenizer );

			// configure morphology
			DWORD iMorph = SPH_MORPH_NONE;
			if ( hIndex ( "morphology" ) )
			{
				iMorph = sphParseMorphology ( hIndex["morphology"], bUseUTF8 );
				if ( iMorph==SPH_MORPH_UNKNOWN )
					sphWarning ( "index '%s': unknown morphology type '%s' ignored",
						sIndexName, hIndex["morphology"].cstr() );
			}

			// configure charset_table
			if ( hIndex.Exists ( "charset_table" ) )
				if ( !pTokenizer->SetCaseFolding ( hIndex["charset_table"].cstr() ) )
			{
				sphWarning ( "index '%s': failed to parse 'charset_table' - NOT SERVING",
					sIndexName );
				continue;
			}

			// min word len
			if ( hIndex("min_word_len") )
				pTokenizer->SetMinWordLen ( hIndex["min_word_len"].intval() );

			// create add this one to served hashes
			ServedIndex_t tIdx;

			tIdx.m_pIndex = sphCreateIndexPhrase ( hIndex["path"].cstr() );
			tIdx.m_pSchema = tIdx.m_pIndex->Preload() ;
			if ( !tIdx.m_pSchema )
			{
				sphWarning ( "index '%s': failed to preload schema and docinfos - NOT SERVING", sIndexName );
				continue;
			}

			tIdx.m_pDict = new CSphDict_CRC32 ( iMorph );
			tIdx.m_pDict->LoadStopwords ( hIndex.Exists ( "stopwords" ) ? hIndex["stopwords"].cstr() : NULL, pTokenizer );
			tIdx.m_pTokenizer = pTokenizer;
			tIdx.m_pIndexPath = new CSphString ( hIndex["path"] );

			if ( !bOptConsole )
			{
				// check lock file
				char sTmp [ 1024 ];
				snprintf ( sTmp, sizeof(sTmp), "%s.spl", hIndex["path"].cstr() );
				sTmp [ sizeof(sTmp)-1 ] = '\0';

				struct stat tStat;
				if ( !stat ( sTmp, &tStat ) )
				{
					sphWarning ( "index '%s': lock file '%s' exists - NOT SERVING", sIndexName, sTmp );
					continue;
				}

				// create lock file
				FILE * fp = fopen ( sTmp, "w" );
				if ( !fp )
					sphFatal ( "index '%s': failed to create lock file '%s''", sIndexName, sTmp );
				fprintf ( fp, "%d", getpid() );
				fclose ( fp );

				tIdx.m_pLockFile = new CSphString ( sTmp );
			}

			if ( !g_hIndexes.Add ( tIdx, sIndexName ) )
			{
				sphWarning ( "index '%s': duplicate name in hash?! INTERNAL ERROR - NOT SERVING", sIndexName );
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

	////////////////////
	// network startup
	////////////////////

	// init WSA on Windows
	#if USE_WINDOWS
	WSADATA tWSAData;
	int iStartupErr = WSAStartup ( WINSOCK_VERSION, &tWSAData );
	if ( iStartupErr )
		sphFatal ( "failed to initialize WinSock2: %s", sphSockError(iStartupErr) );
	#endif

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

	// we're almost good, and can create .pid file now
	if ( !bOptConsole )
	{
		// i'm the main one
		g_bHeadDaemon = true;

		// create pid
		g_sPidFile = hSearchd["pid_file"].cstr();
		FILE * fp = fopen ( g_sPidFile, "w" );
		if ( !fp )
			sphFatal ( "failed to write pid file '%s'", g_sPidFile );
		fprintf ( fp, "%d", getpid() );	
		fclose ( fp );
	}

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

				if ( !RotateIndex ( tIndex, sIndex ) )
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

		if ( bOptConsole )
		{
			HandleClient ( rsock, sClientIP );
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
				HandleClient ( rsock, sClientIP );
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
