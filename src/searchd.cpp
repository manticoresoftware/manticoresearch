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

static int				g_iReadTimeout	= 5;
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

///////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS

// Windows hacks
#define LOCK_EX			0
#define LOCK_UN			1
#define STDIN_FILENO	fileno(stdin)
#define STDOUT_FILENO	fileno(stdout)
#define STDERR_FILENO	fileno(stderr)
#define ETIMEDOUT		WSAETIMEDOUT
#define EWOULDBLOCK		WSAEWOULDBLOCK
#define socklen_t		int
#define vsnprintf		_vsnprintf


void flock ( int, int )
{
}


void sleep ( int )
{
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

int createServerSocket_IP(int port)
{
	int sock;
	static struct sockaddr_in iaddr;

	iaddr.sin_family = AF_INET;
	iaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	iaddr.sin_port = htons((short)port);

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
		if (g_iReadTimeout) {
			do {
				res = select_fd (fd, g_iReadTimeout, 0);
			} while (res == -1 && errno == EINTR);
			if (res <= 0) {
				if (res == 0) errno = ETIMEDOUT;
				return -1;
			}
		}
		res = sphSockRecv ( fd, (char*)buf, len, 0 );
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

	return sphSockSend ( iFD, sBuf, strlen(sBuf), 0 );
}

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

	// configure query
	tQuery.m_sQuery = sQuery;
	tQuery.m_pWeights = dUserWeights;
	tQuery.m_iWeights = iUserWeights;
	tQuery.m_eMode = (ESphMatchMode) Min ( Max ( iMode, 0 ), SPH_MATCH_TOTAL );

	// do search
	if ( strcmp ( sIndex, "*" )==0 )
	{
		// search through all indexes
		g_hIndexes.IterateStart ();

		pRes = new CSphQueryResult ();
		ISphMatchQueue * pTop = sphCreateQueue ( &tQuery );

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

		sphFlattenQueue ( pTop, pRes );

	} else
	{
		// search through specific index
		if ( !g_hIndexes.Exists ( sIndex ) )
		{
			iwrite ( rsock, "ERROR Index '%s' not found\n", sIndex );
			sphSockClose ( rsock );
			return;
		}

		const ServedIndex_t & tServed = g_hIndexes[sIndex];
		assert ( tServed.m_pIndex );
		assert ( tServed.m_pDict );
		assert ( tServed.m_pTokenizer );

		// do query
		tQuery.m_sQuery = sQuery;
		tQuery.m_pWeights = dUserWeights;
		tQuery.m_iWeights = iUserWeights;
		tQuery.m_eMode = (ESphMatchMode) Min ( Max ( iMode, 0 ), SPH_MATCH_TOTAL );
		tQuery.m_pTokenizer = tServed.m_pTokenizer;

		pRes = tServed.m_pIndex->Query ( tServed.m_pDict, &tQuery );
	}

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
		fprintf ( stderr, "ERROR: malformed or unknown option near '%s'.\n\n", argv[i] );
		fprintf ( stderr, "usage: searchd [--config file.conf] [--console]\n" );
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

	/////////////////////
	// build index table
	/////////////////////

	int iValidIndexes = 0;

	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext() )
	{
		const CSphConfigSection & hIndex = hConf["index"].IterateGet();
		const char * sIndexName = hConf["index"].IterateGetKey().cstr();

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
						sphWarning ( "rotating index '%s': failed to create new index object", sIndex, sFile );

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
					sphWarning ( "rotating index '%s': using old index" );
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

		if ( rsock<0 && ( errno==EINTR || errno==EAGAIN || errno==EWOULDBLOCK ) )
			continue;
		if ( rsock<0 )
			sphFatal ( "accept() failed (reason: %s)", strerror ( errno ) );

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
