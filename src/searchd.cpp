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

#if !USE_WINDOWS
#include <unistd.h>
#include <netinet/in.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
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

static int				g_iReadTimeout	= 5;
static int				g_iChildren		= 0;
static int				g_iMaxChildren	= 0;
static int				g_iSocket		= 0;
static int				g_iQueryLogFile	= -1;

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

int createServerSocket_IP(int port)
{
	int sock;
	static struct sockaddr_in iaddr;

	iaddr.sin_family = AF_INET;
	iaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	iaddr.sin_port = htons(port);

	sphInfo ( "creating a server socket on port %d", port );
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		sphFatal ( "unable to create socket on port %d: %s", port, strerror ( errno ) );

	int iOn = 1;
	if ( setsockopt ( sock, SOL_SOCKET,  SO_REUSEADDR, (char*)&iOn, sizeof(iOn) ) )
		sphFatal ( "setsockopt failed: %s", strerror ( errno ) );

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


void sigchld ( int arg )
{
	signal ( SIGCHLD, sigchld );
	while ( waitpid ( 0, (int *)0, WNOHANG | WUNTRACED ) > 0 )
		g_iChildren--;
}


void sigterm(int arg)
{
	sphInfo ( "caught SIGTERM, shutting down" );
	if ( g_iSocket )
		close ( g_iSocket );
	exit ( 0 );
}


int select_fd ( int fd, int maxtime, int writep )
{
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
}


int iread(int fd, void *buf, int len)
{
	int res;

	do {
		if (g_iReadTimeout) {
			do {
				res = select_fd (fd, g_iReadTimeout, 0);
			} while (res == -1 && errno == EINTR);
			if (res <= 0) {
				if (res == 0) errno = ETIMEDOUT;
				return -1;
			}
		}
		res = read(fd, buf, len);
	}
	while (res == -1 && errno == EINTR);

	return res;
}


int iwrite ( int iFD, const char * sFmt, ... )
{
	char sBuf [ 2048 ];

	va_list ap;
	va_start ( ap, sFmt );
	vsnprintf ( sBuf, sizeof(sBuf), sFmt, ap );
	va_end ( ap );

	return ::write ( iFD, sBuf, strlen(sBuf) );
}

/////////////////////////////////////////////////////////////////////////////

void HandleClient ( int rsock, CSphIndex * pIndex, CSphDict * pDict )
{
	CSphQuery tQuery;
	CSphQueryResult * pRes;
	int i, iOffset, iLimit, iCount, iMode;
	char sQuery [ 1024 ], sBuf [ 2048 ];

	// hello there
	iwrite ( rsock, "VER %d\n", SPHINX_SEARCHD_PROTO );

	// read mode/limits
	assert ( sizeof(tQuery.m_eSort)==4 );
	if ( iread ( rsock, &iOffset, 4 )!=4 ) return;
	if ( iread ( rsock, &iLimit, 4 )!=4 ) return;
	if ( iread ( rsock, &iMode, 4 )!=4 ) return;
	if ( iread ( rsock, &tQuery.m_eSort, 4)!=4 ) return;
	if ( iread ( rsock, &tQuery.m_iGroups, 4 )!=4 ) return;

	// read groups
	if ( tQuery.m_iGroups<0 || tQuery.m_iGroups>256 ) return; // FIXME?
	if ( tQuery.m_iGroups )
	{
		i = tQuery.m_iGroups*sizeof(DWORD);
		tQuery.m_pGroups = new DWORD [ tQuery.m_iGroups ];
		if ( iread ( rsock, tQuery.m_pGroups, i )!=i ) return;
	}

	// read query string
	if ( iread ( rsock, &i, 4 )!=4 ) return;
	if ( i<0 || i>(int)sizeof(sQuery)-1 ) return;

	if ( iread ( rsock, sQuery, i )!=i ) return;
	sQuery[i] = '\0';

	// read weights
	int iUserWeights;
	int dUserWeights [ SPH_MAX_FIELD_COUNT ];

	if ( iread ( rsock, &iUserWeights, 4 ) != 4 ) return;
	if ( iUserWeights<0 || iUserWeights>SPH_MAX_FIELD_COUNT ) return;
	if ( iUserWeights )
		if ( iread ( rsock, dUserWeights, iUserWeights*4 ) != iUserWeights*4 ) return;

	// do query
	tQuery.m_sQuery = sQuery;
	tQuery.m_pWeights = dUserWeights;
	tQuery.m_iWeights = iUserWeights;
	tQuery.m_eMode = (ESphMatchMode) Min ( Max ( iMode, 0 ), SPH_MATCH_TOTAL );

	pRes = pIndex->query ( pDict, &tQuery );

	// log query
	if ( g_iQueryLogFile>=0 )
	{
		time_t tNow;
		char sTimeBuf[128];

		time ( &tNow );
		ctime_r ( &tNow, sTimeBuf );
		sTimeBuf [ strlen(sTimeBuf)-1 ] = '\0';

		static const char * sModes [ SPH_MATCH_TOTAL ] = { "any", "all", "phr" };
		snprintf ( sBuf, sizeof(sBuf), "[%s] %.2f sec: [%d %d %s %d] %s\n",
			sTimeBuf, pRes->m_fQueryTime,
			iOffset, iLimit, sModes [ tQuery.m_eMode ], pRes->m_iTotalMatches,
			sQuery );

		flock ( g_iQueryLogFile, LOCK_EX );
		lseek ( g_iQueryLogFile, 0, SEEK_END );
		write ( g_iQueryLogFile, sBuf, strlen(sBuf) );
		flock ( g_iQueryLogFile, LOCK_UN );
	}

	// serve the answer to client
	iCount = Min ( iLimit, pRes->m_dMatches.GetLength()-iOffset );

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
			pRes->m_tWordStats[i].m_sWord,
			pRes->m_tWordStats[i].m_iDocs,
			pRes->m_tWordStats[i].m_iHits );
	}

	delete pRes;
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
		fprintf ( stderr, "ERROR: malformed or unknown option near '%s'.\n\n", argv[i] );
		fprintf ( stderr, "usage: searchd [--config file.conf] [--console]\n" );
		return 1;
	}

	/////////////////////
	// parse config file
	/////////////////////

	sphInfo ( "using config file '%s'...", sOptConfig );
	if ( !conf.Open ( sOptConfig ) )
		sphFatal ( "failed to read config file '%s'", sOptConfig );

	CSphHash * confCommon = conf.LoadSection ( "common", g_dSphKeysCommon );
	CSphHash * confSearchd = conf.LoadSection ( "searchd", g_dSphKeysSearchd );

	#define CHECK_CONF(_hash,_section,_key) \
		if ( !_hash->Get(_key) ) \
			sphFatal ( "mandatory option '%s' not found in config file section '[%s]'", _key, _section );

	CHECK_CONF ( confCommon, "common", "index_path" );
	CHECK_CONF ( confSearchd, "searchd", "port" );
	CHECK_CONF ( confSearchd, "searchd", "pid_file" );

	int iPort = atoi ( confSearchd->Get ( "port" ) );
	if ( !iPort )
		sphFatal ( "expected valid 'port', got '%s'", confSearchd->Get ( "port" ) );

	if ( confSearchd->Get ( "g_iReadTimeout") )
		g_iReadTimeout = Max ( 0, atoi ( confSearchd->Get ( "read_timeout" ) ) );

	if ( confSearchd->Get ( "max_children" ) )
		g_iMaxChildren = Max ( 0, atoi ( confSearchd->Get ( "max_children" ) ) );

	DWORD iMorph = SPH_MORPH_NONE;
	const char * pMorph = confCommon->Get ( "morphology" );
	if ( pMorph )
	{
		if ( !strcmp ( pMorph, "stem_en" ) )
			iMorph = SPH_MORPH_STEM_EN;
		else if ( !strcmp ( pMorph, "stem_ru" ) )
			iMorph = SPH_MORPH_STEM_RU;
		else if ( !strcmp ( pMorph, "stem_enru" ) )
			iMorph = SPH_MORPH_STEM_EN | SPH_MORPH_STEM_RU;
		else
			sphWarning ( "unknown morphology type '%s' ignored", pMorph );
	}

	///////////
	// startup
	///////////

	// create index
	CSphIndex * pIndex = sphCreateIndexPhrase ( confCommon->Get ( "index_path" ) );
	CSphDict * pDict = new CSphDict_CRC32 ( iMorph );
	pDict->LoadStopwords ( confCommon->Get ( "stopwords" ) );

	// daemonize
	if ( !bOptConsole )
	{
		// create log
		const char * sLog = "searchd.log";
		if ( confSearchd->Get ( "log" ) )
			sLog = confSearchd->Get ( "log" );

		umask ( 066 );
		g_iLogFile = open ( sLog, O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
		if ( g_iLogFile<0 )
		{
			g_iLogFile = STDOUT_FILENO;
			sphFatal ( "failed to write log file '%s'", sLog );
		}

		// create query log if required
		if ( confSearchd->Get ( "query_log" ) )
		{
			g_iQueryLogFile = open ( confSearchd->Get ( "query_log" ), O_CREAT | O_RDWR | O_APPEND,
				S_IREAD | S_IWRITE );
			if ( g_iQueryLogFile<0 )
				sphFatal ( "failed to write query log file '%s'", confSearchd->Get ( "query_log" ) );
		}

		// do daemonize
		signal ( SIGCHLD, sigchld );
		signal ( SIGTERM, sigterm );
		signal ( SIGINT, sigterm );
		signal ( SIGHUP, SIG_IGN );

		int iDevNull = open ( "/dev/null", O_RDWR );
		close ( STDIN_FILENO );
		close ( STDOUT_FILENO );
		close ( STDERR_FILENO );
		dup2 ( iDevNull, STDIN_FILENO );
		dup2 ( iDevNull, STDOUT_FILENO );
		dup2 ( iDevNull, STDERR_FILENO );
		g_bLogStdout = false;

		switch ( fork() )
		{
			case -1:	sphFatal ( "fork() failed (reason: %s)", strerror ( errno ) ); // error
			case 0:		break; // daemonized child
			default:	exit ( 0 ); // tty-controlled parent
		}
	} else
	{
		// if we're running in console mode, dump queries to tty as well
		g_iQueryLogFile = g_iLogFile;
	}

	// create pid
	FILE * fp = fopen ( confSearchd->Get ( "pid_file" ), "w" );
	if ( !fp )
		sphFatal ( "unable to write pid file '%s'", confSearchd->Get ( "pid_file" ) );
	fprintf ( fp, "%d", getpid() );	
	fclose ( fp );

	// create and bind on socket
	g_iSocket = createServerSocket_IP ( iPort );
	listen ( g_iSocket, 5 );

	/////////////////
	// serve clients
	/////////////////

	sphInfo ( "accepting connections" );
	for ( ;; )
	{
		len = sizeof ( remote_iaddr );
		rsock = accept ( g_iSocket, (struct sockaddr*)&remote_iaddr, &len );
		if ( rsock<0 && ( errno==EINTR || errno==EAGAIN || errno==EWOULDBLOCK ) )
			continue;
		if ( rsock<0 )
			sphFatal ( "accept() failed (reason: %s)", strerror ( errno ) );

		if ( g_iMaxChildren && g_iChildren>=g_iMaxChildren )
		{
			iwrite ( rsock, "RETRY Server maxed out\n" );
			close ( rsock );
			continue;
		}

		switch ( fork() )
		{
			// fork() failed
			case -1:
				sphFatal ( "fork() failed (reason: %s)", strerror ( errno ) );

			// child process, handle client
			case 0:
				HandleClient ( rsock, pIndex, pDict );
				close ( rsock );
				exit ( 0 );
				break;

			// parent process, continue accept()ing
			default:
				g_iChildren++;
				close(rsock);
				break;
		}
	}
}

//
// $Id$
//
