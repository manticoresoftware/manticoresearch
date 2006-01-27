//
// $Id$
//

//
// Copyright (c) 2001-2005, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxutils.h"

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

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

/// remote agent state
enum AgentState_e
{
	AGENT_UNUSED,				///< agent is unused for this request
	AGENT_CONNECT,				///< connecting to agent
	AGENT_HELLO,				///< waiting for "VER x" hello
	AGENT_QUERY					///< query sent, wating for reply
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

protected:
	int				m_iAddrType;
	int				m_iAddrLen;
	char *			m_pAddr;

public:
	Agent_t ()
		: m_iPort ( -1 )
		, m_iSock ( -1 )
		, m_eState ( AGENT_UNUSED )
		, m_pAddr ( NULL )
	{
	}

	~Agent_t ()
	{
		Close ();
	}

	void Close ()
	{
		if ( m_iSock>0 )
		{
			sphSockClose ( m_iSock );
			m_iSock = -1;
			m_eState = AGENT_UNUSED;
		}
	}

	void SetAddr ( int iAddrType, int iAddrLen, const char * pAddr )
	{
		assert ( pAddr );
		SafeDeleteArray ( m_pAddr );

		m_iAddrType = iAddrType;
		m_iAddrLen = iAddrLen;
		m_pAddr = new char [ iAddrLen ];
		memcpy ( m_pAddr, pAddr, iAddrLen );
	}

	int GetAddrType () const
	{
		return m_iAddrType;
	}

	int GetAddrLen () const
	{
		return m_iAddrLen;
	}

	const char * GetAddr () const
	{
		return m_pAddr;
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

///////////////////////////////////////////////////////////////////////////////

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

void Shutdown ()
{
	if ( g_iSocket )
		sphSockClose ( g_iSocket );
	if ( g_sPidFile )
		unlink ( g_sPidFile );
	g_hIndexes.Reset ();
}

///////////////////////////////////////////////////////////////////////////////

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


int createServerSocket_IP(int port)
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

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

int select_fd ( int fd, int maxtime, int writep )
{
	#if USE_WINDOWS
	#pragma warning(disable:4127) // conditional expr is const
	#pragma warning(disable:4389) // signed/unsigned mismatch
	#endif // USE_WINDOWS

	fd_set fds;
	FD_ZERO ( &fds );
	FD_SET ( fd, &fds );

	fd_set exceptfds;
	FD_ZERO ( &exceptfds );
	FD_SET ( fd, &exceptfds );

	struct timeval tv;
	tv.tv_sec = maxtime;
	tv.tv_usec = 0;

	return select ( fd+1, writep ? NULL : &fds, writep ? &fds : NULL, &exceptfds, &tv );

	#if USE_WINDOWS
	#pragma warning(default:4127) // conditional expr is const
	#pragma warning(default:4389) // signed/unsigned mismatch
	#endif // USE_WINDOWS
}


int iread(int fd, void *buf, int len)
{
	int res;

	do {
		if ( g_iReadTimeout )
		{
			do
			{
				res = select_fd ( fd, g_iReadTimeout, 0 );
			} while ( res==-1 && sphSockGetErrno()==EINTR );
			if ( res<=0 )
			{
				if ( res==0 )
					sphSockSetErrno ( ETIMEDOUT );
				return -1;
			}
		}
		res = sphSockRecv ( fd, (char*)buf, len, 0 );
	} while ( res==-1 && sphSockGetErrno()==EINTR );

	return res;
}


int iwrite ( int iFD, const char * sFmt, ... )
{
	char sBuf [ 2048 ];

	va_list ap;
	va_start ( ap, sFmt );
	vsnprintf ( sBuf, sizeof(sBuf), sFmt, ap );
	va_end ( ap );

	return sphSockSend ( iFD, sBuf, strlen(sBuf), 0 );
}

/////////////////////////////////////////////////////////////////////////////

int sphSetSockNB ( int iSock )
{
	#if USE_WINDOWS
		u_long uMode = 1;
		return ioctlsocket ( iSock, FIONBIO, &uMode );
	#else
		return fcntl ( iSock, F_SETFL, O_NONBLOCK );
	#endif
}


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


STATIC_SIZE_ASSERT ( DWORD, 4 );


inline void NetPackDword ( BYTE * & pReq, DWORD uValue )
{
	*(DWORD *)pReq = uValue;
	pReq += sizeof(DWORD);
}


inline void NetPackStr ( BYTE * & pReq, const char * sValue )
{
	int iLen = strlen(sValue);
	NetPackDword ( pReq, iLen );
	memcpy ( pReq, sValue, iLen );
	pReq += iLen;
}


inline void NetPackArray ( BYTE * & pReq, int iNumValues, DWORD * sValues )
{
	NetPackDword ( pReq, iNumValues );
	memcpy ( pReq, sValues, sizeof(DWORD)*iNumValues );
	pReq += sizeof(DWORD)*iNumValues;
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
		fd_set fdsRead, fdsWrite, fdsExcept;
		FD_ZERO ( &fdsRead );
		FD_ZERO ( &fdsWrite );
		FD_ZERO ( &fdsExcept );

		int iMax = 0;
		bool bDone = true;
		ARRAY_FOREACH ( i, tDist.m_dAgents )
		{
			const Agent_t & tAgent = tDist.m_dAgents[i];
			if ( tAgent.m_eState==AGENT_CONNECT || tAgent.m_eState==AGENT_HELLO )
			{
				assert ( tAgent.m_iPort>0 );
				assert ( tAgent.m_iSock>0 );

				if ( tAgent.m_eState==AGENT_CONNECT )
				{
					// if we're connecting, check for write (connect success) and except (connect failure)
					FD_SET ( tAgent.m_iSock, &fdsWrite );
					FD_SET ( tAgent.m_iSock, &fdsExcept );
				} else
				{
					// we connected, lets wait until remote says hello
					FD_SET ( tAgent.m_iSock, &fdsRead );
				}

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
			if ( tAgent.m_eState==AGENT_CONNECT &&
				( FD_ISSET ( tAgent.m_iSock, &fdsWrite ) || FD_ISSET ( tAgent.m_iSock, &fdsExcept ) ) )
			{
				if ( FD_ISSET ( tAgent.m_iSock, &fdsExcept ) )
				{
					// connect() failure
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
				char sBuf [ 128 ];
				int iRes = sphSockRecv ( tAgent.m_iSock, sBuf, sizeof(sBuf)-1, 0 );

				assert ( iRes>0 && iRes<(int)sizeof(sBuf) );
				sBuf[iRes] = '\0';

				// check reply
				bool bError = false;
				if ( strncmp ( sBuf, "VER ", 4 )!=0 )
				{
					sphWarning ( "index '%s': agent '%s:%d': bad handshake reply",
						sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort );
					bError = true;
				}
				if ( !bError && atoi(sBuf+4)!=SPHINX_SEARCHD_PROTO )
				{
					sphWarning ( "index '%s': agent '%s:%d': expected protocol v.%d, got v.%d",
						sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort,
						SPHINX_SEARCHD_PROTO, atoi(sBuf+4) );
					bError = true;
				}

				// bail out on error
				if ( bError )
				{
					tAgent.Close ();
					continue;
				}

				// do query!
				int iQueryLen = strlen ( tQuery.m_sQuery );
				int iIndexesLen = strlen ( tAgent.m_sIndexes.cstr() );

				int iReqSize = 52 + 4*tQuery.m_iGroups + iQueryLen + 4*tQuery.m_iWeights + iIndexesLen;
				BYTE * pReqBuffer = new BYTE [ iReqSize ];
				BYTE * pReq = (BYTE *) pReqBuffer;

				// v1. mode/limits part
				NetPackDword ( pReq, 0 ); // offset is 0
				NetPackDword ( pReq, CSphQueryResult::MAX_MATCHES ); // limit is MAX_MATCHES
				NetPackDword ( pReq, iMode ); // match mode
				NetPackDword ( pReq, tQuery.m_eSort ); // sort mode

				// v1. groups
				NetPackArray ( pReq, tQuery.m_iGroups, tQuery.m_pGroups );

				// v1. query string
				NetPackStr ( pReq, tQuery.m_sQuery );

				// v1. weights
				NetPackArray ( pReq, tQuery.m_iWeights, (DWORD*)tQuery.m_pWeights );

				// v2. index name
				NetPackStr ( pReq, tAgent.m_sIndexes.cstr() );

				// v3. id/ts limits
				NetPackDword ( pReq, tQuery.m_iMinID );
				NetPackDword ( pReq, tQuery.m_iMaxID );
				NetPackDword ( pReq, tQuery.m_iMinTS );
				NetPackDword ( pReq, tQuery.m_iMaxTS );

				// v4. binary mode
				NetPackDword ( pReq, 1 );

				assert ( pReq-pReqBuffer==iReqSize );
				sphSockSend ( tAgent.m_iSock, (const char*)pReqBuffer, pReq-pReqBuffer, 0 );
				SafeDeleteArray ( pReqBuffer );

				tAgent.m_eState = AGENT_QUERY;
				iAgents++;
			}
		}
	}

	ARRAY_FOREACH ( i, tDist.m_dAgents )
	{
		// check if connection timed out
		Agent_t & tAgent = tDist.m_dAgents[i];
		if ( tAgent.m_eState!=AGENT_QUERY )
		{
			tAgent.Close ();
			sphWarning ( "index '%s': agent '%s:%d': connect() timed out",
				sIndexName, tAgent.m_sHost.cstr(), tAgent.m_iPort );
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
			if ( tAgent.m_eState==AGENT_QUERY )
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
			if (!( tAgent.m_eState==AGENT_QUERY && FD_ISSET ( tAgent.m_iSock, &fdsRead ) ))
				continue;

			// reply was received, let's parse it
			bool bOK = false;
			for ( ;; )
			{
				// read match count
				int iMatches;
				if ( sphSockRecv ( tAgent.m_iSock, (char*)&iMatches, 4, 0 )!=4 ) break;
				if ( iMatches<0 || iMatches>CSphQueryResult::MAX_MATCHES ) break;

				// read matches
				int iTmp;
				if ( iMatches>0 )
				{
					assert ( !tAgent.m_tRes.m_dMatches.GetLength() );
					tAgent.m_tRes.m_dMatches.Resize ( iMatches );

					iTmp = sizeof(CSphMatch)*iMatches;
					if ( sphSockRecv ( tAgent.m_iSock, (char*)&tAgent.m_tRes.m_dMatches[0], iTmp, 0 )!=iTmp ) break;
				}
				
				// read totals (retrieved count, total count, query time, word count)
				if ( sphSockRecv ( tAgent.m_iSock, (char*)&iTmp, 4, 0 )!=4 ) break;
				if ( iTmp!=iMatches ) break;
				if ( sphSockRecv ( tAgent.m_iSock, (char*)&tAgent.m_tRes.m_iTotalMatches, 4, 0 )!=4 ) break;
				if ( sphSockRecv ( tAgent.m_iSock, (char*)&tAgent.m_tRes.m_fQueryTime, 4, 0 )!=4 ) break;
				if ( sphSockRecv ( tAgent.m_iSock, (char*)&tAgent.m_tRes.m_iNumWords, 4, 0 )!=4 ) break;

				bool bSetWords = false;
				if ( pRes->m_iNumWords && tAgent.m_tRes.m_iNumWords!=pRes->m_iNumWords ) break;
				if ( !pRes->m_iNumWords )
				{
					pRes->m_iNumWords = tAgent.m_tRes.m_iNumWords;
					bSetWords = true;
				}

				// read words
				int i;
				for ( i=0; i<pRes->m_iNumWords; i++ )
				{
					char sBuf [ SPH_MAX_WORD_LEN+1 ];

					// read word len
					if ( sphSockRecv ( tAgent.m_iSock, (char*)&iTmp, 4, 0 )!=4 ) break;
					if ( iTmp<0 || iTmp>SPH_MAX_WORD_LEN ) break;

					// read word, and NULL-terminate it!
					if ( sphSockRecv ( tAgent.m_iSock, sBuf, iTmp, 0 )!=iTmp ) break;
					sBuf[iTmp] = '\0';

					// set it in result if not yet, or check if already
					if ( bSetWords )
					{
						pRes->m_tWordStats[i].m_sWord.SetBinary ( sBuf, iTmp );
					} else
					{
						if ( pRes->m_tWordStats[i].m_sWord!=sBuf )
							break;
					}

					// read docs count
					if ( sphSockRecv ( tAgent.m_iSock, (char*)&iTmp, 4, 0 )!=4 ) break;
					pRes->m_tWordStats[i].m_iDocs += iTmp;

					// read hits count
					if ( sphSockRecv ( tAgent.m_iSock, (char*)&iTmp, 4, 0 )!=4 ) break;
					pRes->m_tWordStats[i].m_iHits += iTmp;
				}
				if ( i!=pRes->m_iNumWords ) break;

				// all done
				iAgents++;
				bOK = true;
				break;
			}

			tAgent.Close ();
			if ( !bOK )
				tAgent.m_tRes.m_dMatches.Reset ();
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

void HandleClient ( int rsock )
{
	CSphQuery tQuery;
	CSphQueryResult * pRes;
	int i, iOffset, iLimit, iCount, iMode;
	char sQuery [ 1024 ], sIndex [ 1024 ], sBuf [ 2048 ];

	// hello there
	iwrite ( rsock, "VER %d\n", SPHINX_SEARCHD_PROTO );

	// v1. read mode/limits
	assert ( sizeof(tQuery.m_eSort)==4 );
	if ( iread ( rsock, &iOffset, 4 )!=4 ) return;
	if ( iread ( rsock, &iLimit, 4 )!=4 ) return;
	if ( iread ( rsock, &iMode, 4 )!=4 ) return;
	if ( iread ( rsock, &tQuery.m_eSort, 4)!=4 ) return;
	if ( iread ( rsock, &tQuery.m_iGroups, 4 )!=4 ) return;

	// v1. read groups
	if ( tQuery.m_iGroups<0 || tQuery.m_iGroups>4096 ) return; // FIXME?
	if ( tQuery.m_iGroups )
	{
		i = tQuery.m_iGroups*sizeof(DWORD);
		tQuery.m_pGroups = new DWORD [ tQuery.m_iGroups ];
		if ( iread ( rsock, tQuery.m_pGroups, i )!=i ) return;
	}

	// v1. read query string
	if ( iread ( rsock, &i, 4 )!=4 ) return;
	if ( i<0 || i>(int)sizeof(sQuery)-1 ) return;
	if ( iread ( rsock, sQuery, i )!=i ) return;
	sQuery[i] = '\0';

	// v1. read weights
	int iUserWeights;
	int dUserWeights [ SPH_MAX_FIELD_COUNT ];

	if ( iread ( rsock, &iUserWeights, 4 ) != 4 ) return;
	if ( iUserWeights<0 || iUserWeights>SPH_MAX_FIELD_COUNT ) return;
	if ( iUserWeights )
		if ( iread ( rsock, dUserWeights, iUserWeights*4 ) != iUserWeights*4 ) return;

	// v2. read index name
	if ( iread ( rsock, &i, 4 )!=4 ) return;
	if ( i<0 || i>(int)sizeof(sIndex)-1 ) return;
	if ( iread ( rsock, sIndex, i )!=i ) return;
	sIndex[i] = '\0';

	// v3. read group/TS limits
	if ( iread ( rsock, &tQuery.m_iMinID, 4 )!=4 ) return;
	if ( iread ( rsock, &tQuery.m_iMaxID, 4 )!=4 ) return;
	if ( iread ( rsock, &tQuery.m_iMinTS, 4 )!=4 ) return;
	if ( iread ( rsock, &tQuery.m_iMaxTS, 4 )!=4 ) return;
	if ( tQuery.m_iMinID>tQuery.m_iMaxID || tQuery.m_iMinTS>tQuery.m_iMaxTS )
	{
		iwrite ( rsock, "ERROR Bad ID/TS range specified\n" );
		return;
	}

	// v4. binary flag
	int iBinary;
	if ( iread ( rsock, &iBinary, 4 )!=4 ) return;

	// configure query
	tQuery.m_sQuery = sQuery;
	tQuery.m_pWeights = dUserWeights;
	tQuery.m_iWeights = iUserWeights;
	tQuery.m_eMode = (ESphMatchMode) Min ( Max ( iMode, 0 ), SPH_MATCH_TOTAL );

	// do search
	pRes = new CSphQueryResult ();
	ISphMatchQueue * pTop = sphCreateQueue ( &tQuery );
	float tmStart = sphLongTimer ();

	if ( g_hDistIndexes(sIndex) )
	{
		// search through specified distributed index
		DistributedIndex_t & tDist = g_hDistIndexes[sIndex];

		// start connecting to remote agents
		ARRAY_FOREACH ( i, tDist.m_dAgents )
			ConnectToRemoteAgent ( &tDist.m_dAgents[i] );

		// connect to remote agents and query them first
		int iRemote = QueryRemoteAgents ( sIndex, tDist, tQuery, iMode );

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
		}
		tmQuery += sphLongTimer ();

		if ( !iRemote && !tDist.m_dLocal.GetLength() )
		{
			iwrite ( rsock, "ERROR All remote agents are unreachable and no local indexes are configured\n" );
			return;
		}

		// wait for remote queries to complete
		if ( iRemote )
		{
			int iMsecLeft = tDist.m_iAgentQueryTimeout - int(tmQuery*1000.0f);
			int iReplys = WaitForRemoteAgents ( sIndex, tDist, pRes, Max ( iMsecLeft, 0 ) );
			if ( !iReplys && !tDist.m_dLocal.GetLength() )
			{
				iwrite ( rsock, "ERROR All reachable remote agents timed out\n" );
				return;
			}

			// merge local and remote results
			ARRAY_FOREACH ( iAgent, tDist.m_dAgents )
				if ( tDist.m_dAgents[iAgent].m_tRes.m_dMatches.GetLength() )
			{
				// merge this agent's results
				Agent_t & tAgent = tDist.m_dAgents[iAgent];

				ARRAY_FOREACH ( i, tAgent.m_tRes.m_dMatches )
					pTop->Push ( tAgent.m_tRes.m_dMatches[i] );

				// merge this agent's stats
				pRes->m_iTotalMatches += tAgent.m_tRes.m_iTotalMatches;

				// free the set
				tAgent.m_tRes.m_dMatches.Reset ();
			}
		}

	} else if ( strcmp ( sIndex, "*" )==0 )
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
		}

	} else
	{
		// search through the specified local indexes
		int iSearched = 0;
		char * p = sIndex;
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
				iwrite ( rsock, "ERROR Invalid index '%s' specified in request\n", sNext );
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
		}

		if ( !iSearched )
		{
			iwrite ( rsock, "ERROR No valid indexes specified in request\n" );
			return;
		}
	}

	sphFlattenQueue ( pTop, pRes );
	pRes->m_fQueryTime = sphLongTimer() - tmStart;

	// log query
	if ( g_iQueryLogFile>=0 )
	{
		time_t tNow;
		char sTimeBuf[128];

		time ( &tNow );
		ctime_r ( &tNow, sTimeBuf );
		sTimeBuf [ strlen(sTimeBuf)-1 ] = '\0';

		static const char * sModes [ SPH_MATCH_TOTAL ] = { "all", "any", "phr" };
		snprintf ( sBuf, sizeof(sBuf), "[%s] %.2f sec: [%d %d %s/%d/%d %d] %s\n",
			sTimeBuf, pRes->m_fQueryTime,
			iOffset, iLimit, sModes [ tQuery.m_eMode ], tQuery.m_eSort, tQuery.m_iGroups,
			pRes->m_iTotalMatches, sQuery );

		flock ( g_iQueryLogFile, LOCK_EX );
		lseek ( g_iQueryLogFile, 0, SEEK_END );
		write ( g_iQueryLogFile, sBuf, strlen(sBuf) );
		flock ( g_iQueryLogFile, LOCK_UN );
	}

	// serve the answer to client
	iCount = Min ( iLimit, pRes->m_dMatches.GetLength()-iOffset );
	if ( !iBinary )
	{
		// textual response
		iwrite ( rsock, "MATCHES %d\n", iCount );
		for ( i=iOffset; i<iOffset+iCount; i++ )
		{
			CSphMatch & tMatch = pRes->m_dMatches[i];
			iwrite ( rsock, "MATCH %d %d %d %d\n",
				tMatch.m_iGroupID,
				tMatch.m_iDocID,
				tMatch.m_iWeight,
				tMatch.m_iTimestamp );
		}

		iwrite ( rsock, "TOTAL %d %d\n", pRes->m_dMatches.GetLength(), pRes->m_iTotalMatches );
		iwrite ( rsock, "TIME %.2f\n", pRes->m_fQueryTime );
		iwrite ( rsock, "WORDS %d\n", pRes->m_iNumWords );

		for ( i=0; i < pRes->m_iNumWords; i++ )
		{
			iwrite ( rsock, "WORD %s %d %d\n",
				pRes->m_tWordStats[i].m_sWord.cstr(),
				pRes->m_tWordStats[i].m_iDocs,
				pRes->m_tWordStats[i].m_iHits );
		}

	} else
	{
		// binary response
		int iRespLen = 20 + sizeof(CSphMatch)*iCount;
		for ( i=0; i<pRes->m_iNumWords; i++ )
			iRespLen += 12 + strlen ( pRes->m_tWordStats[i].m_sWord.cstr() );

		BYTE * pRespBuffer = new BYTE [ iRespLen ];
		BYTE * pResp = pRespBuffer;

		// create response
		NetPackDword ( pResp, iCount );

		if ( iCount )
			memcpy ( pResp, &pRes->m_dMatches[iOffset], sizeof(CSphMatch)*iCount );
		pResp += sizeof(CSphMatch)*iCount;

		NetPackDword ( pResp, pRes->m_dMatches.GetLength() );
		NetPackDword ( pResp, pRes->m_iTotalMatches );
		NetPackDword ( pResp, *(DWORD*)&pRes->m_fQueryTime );
		NetPackDword ( pResp, pRes->m_iNumWords );

		for ( i=0; i<pRes->m_iNumWords; i++ )
		{
			NetPackStr ( pResp, pRes->m_tWordStats[i].m_sWord.cstr() );
			NetPackDword ( pResp, pRes->m_tWordStats[i].m_iDocs );
			NetPackDword ( pResp, pRes->m_tWordStats[i].m_iHits );
		}

		assert ( pResp-pRespBuffer==iRespLen );
		sphSockSend ( rsock, (const char*)pRespBuffer, pResp-pRespBuffer, 0 );
		SafeDeleteArray ( pRespBuffer );
	}

	delete pRes;
}


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
					sphWarning ( "index '%s': bad local index '%s', SKIPPING",
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
					sphWarning ( "index '%s': agent '%s': bad port number near '%s', SKIPPING",
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
	g_iSocket = createServerSocket_IP ( iPort );
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
			iwrite ( rsock, "RETRY Server maxed out\n" );
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
