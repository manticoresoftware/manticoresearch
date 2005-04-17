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
		sphFatal ( "unable to create socket on port %d", port );

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

/////////////////////////////////////////////////////////////////////////////

int main ( int argc, char **argv )
{
	CSphQueryResult *r;
	int rsock, start = 0, count = 10, rcount;
	struct sockaddr_in remote_iaddr;
	socklen_t len;
	char query[1024], buf[2048];
	CSphConfig conf;

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
	if ( !conf.open ( sOptConfig ) )
		sphFatal ( "failed to read config file '%s'", sOptConfig );

	CSphHash * confCommon = conf.loadSection ( "common", g_dSphKeysCommon );
	CSphHash * confSearchd = conf.loadSection ( "searchd", g_dSphKeysSearchd );

	// logs
	if ( !bOptConsole )
	{
		// create log
		char * sLog = "searchd.log";
		if ( confSearchd->get ( "log" ) )
			sLog = confSearchd->get ( "log" );

		umask ( 066 );
		g_iLogFile = open ( sLog, O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
		if ( g_iLogFile<0 )
			sphFatal ( "failed to write log file '%s'", sLog );

		// create query log
		if ( confSearchd->get ( "query_log" ) )
		{
			g_iQueryLogFile = open ( confSearchd->get ( "query_log" ), O_CREAT | O_RDWR | O_APPEND,
				S_IREAD | S_IWRITE );
			if ( g_iQueryLogFile<0 )
				sphFatal ( "failed to write query log file '%s'", confSearchd->get ( "query_log" ) );
		}
	} else
	{
		// if we're running in console mode, dump queries to tty as well
		g_iQueryLogFile = g_iLogFile;
	}

	#define CHECK_CONF(_hash,_section,_key) \
		if ( !_hash->get(_key) ) \
			sphFatal ( "mandatory option '%s' not found in config file section '[%s]'", _key, _section );

	CHECK_CONF ( confCommon, "common", "index_path" );
	CHECK_CONF ( confSearchd, "searchd", "port" );
	CHECK_CONF ( confSearchd, "searchd", "pid_file" );

	int iPort = atoi ( confSearchd->get ( "port" ) );
	if ( !iPort )
		sphFatal ( "expected valid 'port', got '%s'", confSearchd->get ( "port" ) );

	if ( confSearchd->get ( "g_iReadTimeout") )
		g_iReadTimeout = Max ( 0, atoi ( confSearchd->get ( "read_timeout" ) ) );

	if ( confSearchd->get ( "max_children" ) )
		g_iMaxChildren = Max ( 0, atoi ( confSearchd->get ( "max_children" ) ) );

	DWORD iMorph = SPH_MORPH_NONE;
	char * pMorph = confCommon->get ( "morphology" );
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
	CSphIndex * pIndex = sphCreateIndexPhrase ( confCommon->get ( "index_path" ) );
	CSphDict * pDict = new CSphDict_CRC32 ( iMorph );
	pDict->LoadStopwords ( confCommon->get ( "stopwords" ) );

	// daemonize
	if ( !bOptConsole )
	{
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
	}

	// create pid
	FILE * fp = fopen ( confSearchd->get ( "pid_file" ), "w" );
	if ( !fp )
		sphFatal ( "unable to write pid file '%s'", confSearchd->get ( "pid_file" ) );
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
			strcpy ( buf, "RETRY Server maxed out\n" );
			write ( rsock, buf, strlen(buf) );
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
				for ( ;; )
				{
					int iAny, iGroup;

					// read mode/limits
					if ( iread ( rsock, &start, 4 )!=4 ) break;
					if ( iread ( rsock, &count, 4 )!=4 ) break;
					if ( iread ( rsock, &iAny, 4 )!=4 ) break;
					if ( iread ( rsock, &iGroup, 4 )!=4 ) break;

					// read query string
					if (iread(rsock, &i, 4) != 4) break;
					i = Min(i, (int)sizeof(query)-1);
					if (iread(rsock, query, i) != i) break;
					query[i] = '\0';

					// read weights
					int iUserWeights;
					int dUserWeights [ SPH_MAX_FIELD_COUNT ];

					if ( iread ( rsock, &iUserWeights, 4 ) != 4 ) break;
					if ( iUserWeights<0 || iUserWeights>SPH_MAX_FIELD_COUNT ) break;
					if ( iUserWeights )
						if ( iread ( rsock, dUserWeights, iUserWeights*4 ) != iUserWeights*4 ) break;

					// do query
					CSphQuery tQuery;
					tQuery.m_sQuery = query;
					tQuery.m_pWeights = dUserWeights;
					tQuery.m_iWeights = iUserWeights;
					tQuery.m_bAll = ( iAny==0 );
					tQuery.m_iGroup = iGroup;

					r = pIndex->query ( pDict, &tQuery );

					// log query
					if ( g_iQueryLogFile>=0 )
					{
						time_t tNow;
						char sTimeBuf[128];

						time ( &tNow );
						ctime_r ( &tNow, sTimeBuf );
						sTimeBuf [ strlen(sTimeBuf)-1 ] = '\0';

						sprintf ( buf, "[%s] %.2f sec: [%d %d %s %d] %s\n",
							sTimeBuf, r->m_fQueryTime,
							start, count, tQuery.m_bAll ? "all" : "any", iGroup,
							query );

						flock ( g_iQueryLogFile, LOCK_EX );
						lseek ( g_iQueryLogFile, 0, SEEK_END );
						write ( g_iQueryLogFile, buf, strlen(buf) );
						flock ( g_iQueryLogFile, LOCK_UN );
					}

					// serve the answer to client
					rcount = Min ( count, r->m_dMatches.GetLength()-start );

					sprintf ( buf, "MATCHES %d\n", rcount );
					write ( rsock, buf, strlen(buf) );

					for ( i=start; i<start+rcount; i++ )
					{
						CSphMatch & tMatch = r->m_dMatches[i];
						sprintf ( buf, "MATCH %d %d %d\n",
							tMatch.m_iGroupID,
							tMatch.m_iDocID,
							tMatch.m_iWeight );
						write ( rsock, buf, strlen(buf) );
					}

					sprintf ( buf, "TOTAL %d\n", r->m_dMatches.GetLength() );
					write ( rsock, buf, strlen(buf) );

					sprintf ( buf, "TIME %.2f\n", r->m_fQueryTime );
					write ( rsock, buf, strlen(buf) );

					sprintf ( buf, "WORDS %d\n", r->m_iNumWords );
					write ( rsock, buf, strlen(buf) );

					for ( i=0; i < r->m_iNumWords; i++ )
					{
						sprintf ( buf, "WORD %s %d %d\n",
							r->m_tWordStats[i].m_sWord,
							r->m_tWordStats[i].m_iDocs,
							r->m_tWordStats[i].m_iHits );
						write ( rsock, buf, strlen(buf) );
					}
					break;
				}

				// bail out
				close(rsock);
				exit(0);
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
