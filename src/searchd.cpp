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

#include "sphinx.h"
#include "sphinxutils.h"
#include "fileutils.h"
#include "sphinxexcerpt.h"
#include "sphinxrt.h"
#include "sphinxpq.h"
#include "sphinxint.h"
#include "sphinxquery.h"
#include "sphinxjson.h"
#include "sphinxjsonquery.h"
#include "sphinxplugin.h"
#include "sphinxqcache.h"
#include "accumulator.h"
#include "searchdaemon.h"
#include "searchdha.h"
#include "searchdreplication.h"
#include "threadutils.h"
#include "searchdtask.h"
#include "global_idf.h"
#include "docstore.h"
#include "searchdssl.h"
#include "searchdexpr.h"
#include "indexsettings.h"
#include "searchdddl.h"
#include "networking_daemon.h"
#include "query_status.h"

// services
#include "taskping.h"
#include "taskmalloctrim.h"
#include "taskoptimize.h"
#include "taskglobalidf.h"
#include "tasksavestate.h"
#include "taskflushbinlog.h"
#include "taskflushattrs.h"
#include "taskflushmutable.h"
#include "taskpreread.h"
#include "coroutine.h"

using namespace Threads;

extern "C"
{
#include "sphinxudf.h"
}

#include <csignal>
#include <clocale>
#include <cmath>
#include <ctime>

#if HAVE_MALLOC_H
#include <malloc.h>
#endif

#define SEARCHD_BACKLOG			5

// don't shutdown on SIGKILL (debug purposes)
// 1 - SIGKILL will shut down the whole daemon; 0 - watchdog will reincarnate the daemon
#define WATCHDOG_SIGKILL		1

/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
	// Win-specific headers and calls
	#include <io.h>

#else
	// UNIX-specific headers and calls
	#include <sys/wait.h>
	#include <netdb.h>
	#include<netinet/in.h>
	#include <netinet/tcp.h>
#endif

#if USE_SYSLOG
	#include <syslog.h>
#endif

#if HAVE_GETRLIMIT & HAVE_SETRLIMIT
	#include <sys/resource.h>
#endif

/////////////////////////////////////////////////////////////////////////////

static bool				g_bService		= false;
#if USE_WINDOWS
static bool				g_bServiceStop	= false;
static const char *		g_sServiceName	= "searchd";
HANDLE					g_hPipe			= INVALID_HANDLE_VALUE;
#endif

static StrVec_t	g_dArgs;


enum LogFormat_e
{
	LOG_FORMAT_PLAIN,
	LOG_FORMAT_SPHINXQL
};

#define					LOG_COMPACT_IN	128						// upto this many IN(..) values allowed in query_log

static int				g_iLogFile			= STDOUT_FILENO;	// log file descriptor
static auto& 			g_iParentPID		= getParentPID ();  // set by watchdog
static bool				g_bLogSyslog		= false;
static bool				g_bQuerySyslog		= false;
static CSphString		g_sLogFile;								// log file name
static bool				g_bLogTty			= false;			// cached isatty(g_iLogFile)
static bool				g_bLogStdout		= true;				// extra copy of startup log messages to stdout; true until around "accepting connections", then MUST be false
static LogFormat_e		g_eLogFormat		= LOG_FORMAT_PLAIN;
static bool				g_bLogCompactIn		= false;			// whether to cut list in IN() clauses.
static int				g_iQueryLogMinMs	= 0;				// log 'slow' threshold for query
static char				g_sLogFilter[SPH_MAX_FILENAME_LEN+1] = "\0";
static int				g_iLogFilterLen = 0;
static int				g_iLogFileMode = 0;

int						g_iReadTimeoutS		= 5;	// sec
int						g_iWriteTimeoutS	= 5;	// sec
int						g_iClientTimeoutS	= 300;
int						g_iClientQlTimeoutS	= 900;	// sec
static int				g_iMaxConnection	= 0; // unlimited
static int				g_iThreads;				// defined in config, or =cpu cores
#if !USE_WINDOWS
static bool				g_bPreopenIndexes	= true;
#else
static bool				g_bPreopenIndexes	= false;
#endif
static bool				g_bWatchdog			= true;
static int				g_iExpansionLimit	= 0;
static int				g_iShutdownTimeoutUs	= 3000000; // default timeout on daemon shutdown and stopwait is 3 seconds
static int				g_iBacklog			= SEARCHD_BACKLOG;
int						g_iThdQueueMax		= 0;
bool					g_bGroupingInUtc	= false;
static auto&			g_iTFO = sphGetTFO ();
static CSphString		g_sShutdownToken;
static int				g_iServerID = 0;
static bool				g_bServerID = false;
static bool				g_bJsonConfigLoadedOk = false;

static CSphVector<Listener_t>	g_dListeners;

static int				g_iQueryLogFile	= -1;
static CSphString		g_sQueryLogFile;
static CSphString		g_sPidFile;
static bool				g_bPidIsMine = false;		// if PID is not mine, don't unlink it on fail
static int				g_iPidFD		= -1;

static int				g_iMaxCachedDocs	= 0;	// in bytes
static int				g_iMaxCachedHits	= 0;	// in bytes

int				g_iMaxPacketSize	= 8*1024*1024;	// in bytes; for both query packets from clients and response packets from agents
static int				g_iMaxFilters		= 256;
static int				g_iMaxFilterValues	= 4096;
static int				g_iMaxBatchQueries	= 32;
static ESphCollation	g_eCollation = SPH_COLLATION_DEFAULT;

static int				g_iDocstoreCache = 0;

static FileAccessSettings_t g_tDefaultFA;

int				g_iDistThreads		= 0;
int				g_iAgentConnectTimeoutMs = 1000;
int				g_iAgentQueryTimeoutMs = 3000;	// global (default). May be override by index-scope values, if one specified

const int	MAX_RETRY_COUNT		= 8;
const int	MAX_RETRY_DELAY		= 1000;

int						g_iAgentRetryCount = 0;
int						g_iAgentRetryDelayMs = MAX_RETRY_DELAY/2;	// global (default) values. May be override by the query options 'retry_count' and 'retry_timeout'
bool					g_bHostnameLookup = false;
CSphString				g_sMySQLVersion = szMANTICORE_VERSION;
CSphString				g_sDbName = "Manticore";

// for CLang thread-safety analysis
ThreadRole MainThread; // functions which called only from main thread
ThreadRole HandlerThread; // thread which serves clients

//////////////////////////////////////////////////////////////////////////

static CSphString		g_sConfigFile;
static bool				g_bCleanLoadedConfig = true; // whether to clean config when it parsed and no more necessary

#if USE_WINDOWS
static bool				g_bSeamlessRotate	= false;
#else
static bool				g_bSeamlessRotate	= true;
#endif

static bool				g_bIOStats		= false;
static auto&			g_bCpuStats 	= sphGetbCpuStat ();
static bool				g_bOptNoDetach	= false;
static bool				g_bOptNoLock	= false;
static bool				g_bSafeTrace	= false;
static bool				g_bStripPath	= false;
static bool				g_bCoreDump		= false;

static auto& g_bGotSighup		= sphGetGotSighup();	// we just received SIGHUP; need to log
static auto& g_bGotSigusr1		= sphGetGotSigusr1();	// we just received SIGUSR1; need to reopen logs

// pipe to watchdog to inform that daemon is going to close, so no need to restart it in case of crash
struct SharedData_t
{
	bool m_bDaemonAtShutdown;
	bool m_bHaveTTY;
};

static SharedData_t* 		g_pShared = nullptr;
volatile bool				g_bMaintenance = false;

GuardedHash_c *								g_pLocalIndexes = new GuardedHash_c();	// served (local) indexes hash
GuardedHash_c *								g_pDistIndexes = new GuardedHash_c ();    // distributed indexes hash

// this is internal deal of the daemon; don't expose it outside!
// fixme! move all this stuff to dedicated file.
static GuardedHash_c g_dPostIndexes; GUARDED_BY ( MainThread )// not yet ready (in process of loading/rotation) indexes

static RwLock_t								g_tRotateConfigMutex;
static CSphConfigParser g_pCfg GUARDED_BY ( g_tRotateConfigMutex );
static volatile bool						g_bNeedRotate = false;		// true if there were pending HUPs to handle (they could fly in during previous rotate)
static volatile bool						g_bInRotate = false;		// true while we are rotating
static volatile bool						g_bReloadForced = false;	// true in case reload issued via SphinxQL

static SchedulerSharedPtr_t					g_pTickPoolThread;
static CSphVector<CSphNetLoop*>				g_dNetLoops;

/// command names
static const char * g_dApiCommands[] =
{
	"search", "excerpt", "update", "keywords", "persist", "status", "query", "flushattrs", "query", "ping", "delete", "set",  "insert", "replace", "commit", "suggest", "json",
	"callpq", "clusterpq", "getfield"
};

STATIC_ASSERT ( sizeof(g_dApiCommands)/sizeof(g_dApiCommands[0])==SEARCHD_COMMAND_TOTAL, SEARCHD_COMMAND_SHOULD_BE_SAME_AS_SEARCHD_COMMAND_TOTAL );

//////////////////////////////////////////////////////////////////////////

const char * sAgentStatsNames[eMaxAgentStat+ehMaxStat]=
	{ "query_timeouts", "connect_timeouts", "connect_failures",
		"network_errors", "wrong_replies", "unexpected_closings",
		"warnings", "succeeded_queries", "total_query_time",
		"connect_count", "connect_avg", "connect_max" };

static RwLock_t					g_tLastMetaLock;
static CSphQueryResultMeta		g_tLastMeta GUARDED_BY ( g_tLastMetaLock );

/////////////////////////////////////////////////////////////////////////////
// MISC
/////////////////////////////////////////////////////////////////////////////

void ReleaseTTYFlag()
{
	if ( g_pShared )
		g_pShared->m_bHaveTTY = true;
}


static ServedIndexRefPtr_c GetDisabled ( const CSphString& sName )
{
	return GetServed ( sName, &g_dPostIndexes );
}


void ReleaseAndClearDisabled()
{
	g_dPostIndexes.ReleaseAndClear();
}

/////////////////////////////////////////////////////////////////////////////
// LOGGING
/////////////////////////////////////////////////////////////////////////////

/// format current timestamp for logging
int sphFormatCurrentTime ( char * sTimeBuf, int iBufLen )
{
	int64_t iNow = sphMicroTimer ();
	time_t ts = (time_t) ( iNow/1000000 ); // on some systems (eg. FreeBSD 6.2), tv.tv_sec has another type and we can't just pass it

#if !USE_WINDOWS
	struct tm tmp;
	localtime_r ( &ts, &tmp );
#else
	struct tm tmp;
	tmp = *localtime ( &ts );
#endif

	static const char * sWeekday[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static const char * sMonth[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	return snprintf ( sTimeBuf, iBufLen, "%.3s %.3s%3d %.2d:%.2d:%.2d.%.3d %d",
		sWeekday [ tmp.tm_wday ],
		sMonth [ tmp.tm_mon ],
		tmp.tm_mday, tmp.tm_hour,
		tmp.tm_min, tmp.tm_sec, (int)((iNow%1000000)/1000),
		1900+tmp.tm_year );
}


/// physically emit log entry
/// buffer must have 1 extra byte for linefeed
#if USE_WINDOWS
void sphLogEntry ( ESphLogLevel eLevel, char * sBuf, char * sTtyBuf )
#else
void sphLogEntry ( ESphLogLevel , char * sBuf, char * sTtyBuf )
#endif
{
#if USE_WINDOWS
	if ( g_bService && g_iLogFile==STDOUT_FILENO )
	{
		HANDLE hEventSource;
		LPCTSTR lpszStrings[2];

		hEventSource = RegisterEventSource ( NULL, g_sServiceName );
		if ( hEventSource )
		{
			lpszStrings[0] = g_sServiceName;
			lpszStrings[1] = sBuf;

			WORD eType = EVENTLOG_INFORMATION_TYPE;
			switch ( eLevel )
			{
				case SPH_LOG_FATAL:		eType = EVENTLOG_ERROR_TYPE; break;
				case SPH_LOG_WARNING:	eType = EVENTLOG_WARNING_TYPE; break;
				case SPH_LOG_INFO:		eType = EVENTLOG_INFORMATION_TYPE; break;
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
		strcat ( sBuf, "\n" ); // NOLINT

		sphSeek ( g_iLogFile, 0, SEEK_END );
		if ( g_bLogTty )
		{
			memmove ( sBuf+20, sBuf+15, 9);
			sTtyBuf = sBuf + 19;
			*sTtyBuf = '[';
			sphWrite ( g_iLogFile, sTtyBuf, strlen(sTtyBuf) );
		}
		else
			sphWrite ( g_iLogFile, sBuf, strlen(sBuf) );

		if ( g_bLogStdout && g_iLogFile!=STDOUT_FILENO )
			sphWrite ( STDOUT_FILENO, sTtyBuf, strlen(sTtyBuf) );
	}
}

/// log entry (with log levels, dupe catching, etc)
/// call with NULL format for dupe flushing
void sphLog ( ESphLogLevel eLevel, const char * sFmt, va_list ap )
{
	// dupe catcher state
	static const int	FLUSH_THRESH_TIME	= 1000000; // in microseconds
	static const int	FLUSH_THRESH_COUNT	= 100;

	static ESphLogLevel eLastLevel = SPH_LOG_INFO;
	static DWORD uLastEntry = 0;
	static int64_t tmLastStamp = -1000000-FLUSH_THRESH_TIME;
	static int iLastRepeats = 0;

	// only if we can
	if ( sFmt && eLevel>g_eLogLevel )
		return;

#if USE_SYSLOG
	if ( g_bLogSyslog && sFmt )
	{
		const int levels[SPH_LOG_MAX+1] = { LOG_EMERG, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG };
		vsyslog ( levels[eLevel], sFmt, ap );
		return;
	}
#endif

	if ( g_iLogFile<0 && !g_bService )
		return;

	// format the banner
	char sTimeBuf[128];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	const char * sBanner = "";
	if ( sFmt==NULL ) eLevel = eLastLevel;
	if ( eLevel==SPH_LOG_FATAL ) sBanner = "FATAL: ";
	if ( eLevel==SPH_LOG_WARNING ) sBanner = "WARNING: ";
	if ( eLevel>=SPH_LOG_DEBUG ) sBanner = "DEBUG: ";
	if ( eLevel==SPH_LOG_RPL_DEBUG ) sBanner = "RPL: ";

	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf)-1, "[%s] [%d] ", sTimeBuf, GetOsThreadId() );

	char * sTtyBuf = sBuf + strlen(sBuf);
	strncpy ( sTtyBuf, sBanner, 32 ); // 32 is arbitrary; just something that is enough and keeps lint happy

	auto iLen = (int) strlen(sBuf);

	// format the message
	if ( sFmt )
	{
		// need more space for tail zero and "\n" that added at sphLogEntry
		int iSafeGap = 4;
		int iBufSize = sizeof(sBuf)-iLen-iSafeGap;
		vsnprintf ( sBuf+iLen, iBufSize, sFmt, ap );
		sBuf[ sizeof(sBuf)-iSafeGap ] = '\0';
	}

	if ( sFmt && eLevel>SPH_LOG_INFO && g_iLogFilterLen )
	{
		if ( strncmp ( sBuf+iLen, g_sLogFilter, g_iLogFilterLen )!=0 )
			return;
	}

	// catch dupes
	DWORD uEntry = sFmt ? sphCRC32 ( sBuf+iLen ) : 0;
	int64_t tmNow = sphMicroTimer();

	// accumulate while possible
	if ( sFmt && eLevel==eLastLevel && uEntry==uLastEntry && iLastRepeats<FLUSH_THRESH_COUNT && tmNow<tmLastStamp+FLUSH_THRESH_TIME )
	{
		tmLastStamp = tmNow;
		iLastRepeats++;
		return;
	}

	// flush if needed
	if ( iLastRepeats!=0 && ( sFmt || tmNow>=tmLastStamp+FLUSH_THRESH_TIME ) )
	{
		// flush if we actually have something to flush, and
		// case 1: got a message we can't accumulate
		// case 2: got a periodic flush and been otherwise idle for a thresh period
		char sLast[256];
		iLen = Min ( iLen, 256 );
		strncpy ( sLast, sBuf, iLen );
		if ( iLen < 256 )
			snprintf ( sLast+iLen, sizeof(sLast)-iLen, "last message repeated %d times", iLastRepeats );
		sphLogEntry ( eLastLevel, sLast, sLast + ( sTtyBuf-sBuf ) );

		tmLastStamp = tmNow;
		iLastRepeats = 0;
		eLastLevel = SPH_LOG_INFO;
		uLastEntry = 0;
	}

	// was that a flush-only call?
	if ( !sFmt )
		return;

	tmLastStamp = tmNow;
	iLastRepeats = 0;
	eLastLevel = eLevel;
	uLastEntry = uEntry;

	// do the logging
	sphLogEntry ( eLevel, sBuf, sTtyBuf );
}

void Shutdown (); // forward

bool DieOrFatalWithShutdownCb ( bool bDie, const char * sFmt, va_list ap )
{
	if ( bDie )
		g_pLogger () ( SPH_LOG_FATAL, sFmt, ap );
	else
		Shutdown ();
	return false; // don't lot to stdout
}

bool DieOrFatalCb ( bool bDie, const char * sFmt, va_list ap )
{
	if ( bDie )
		g_pLogger () ( SPH_LOG_FATAL, sFmt, ap );
	return false; // don't lot to stdout
}

#if !USE_WINDOWS
static CSphString GetNamedPipeName ( int iPid )
{
	CSphString sRes;
	sRes.SetSprintf ( "/tmp/searchd_%d", iPid );
	return sRes;
}
#endif


void LogChangeMode ( int iFile, int iMode )
{
	if ( iFile<0 || iMode==0 || iFile==STDOUT_FILENO || iFile==STDERR_FILENO )
		return;

#if !USE_WINDOWS
	fchmod ( iFile, iMode );
#endif
}

/////////////////////////////////////////////////////////////////////////////

static int CmpString ( const CSphString & a, const CSphString & b )
{
	if ( !a.cstr() && !b.cstr() )
		return 0;

	if ( !a.cstr() || !b.cstr() )
		return a.cstr() ? -1 : 1;

	return strcmp ( a.cstr(), b.cstr() );
}

struct SearchFailure_t
{
public:
	CSphString	m_sParentIndex;
	CSphString	m_sIndex;	///< searched index name
	CSphString	m_sError;	///< search error message

public:
	SearchFailure_t () {}

public:
	bool operator == ( const SearchFailure_t & r ) const
	{
		return m_sIndex==r.m_sIndex && m_sError==r.m_sError && m_sParentIndex==r.m_sParentIndex;
	}

	bool operator < ( const SearchFailure_t & r ) const
	{
		int iRes = CmpString ( m_sError.cstr(), r.m_sError.cstr() );
		if ( !iRes )
			iRes = CmpString ( m_sParentIndex.cstr (), r.m_sParentIndex.cstr () );
		if ( !iRes )
			iRes = CmpString ( m_sIndex.cstr(), r.m_sIndex.cstr() );
		return iRes<0;
	}

	const SearchFailure_t & operator = ( const SearchFailure_t & r )
	{
		if ( this!=&r )
		{
			m_sParentIndex = r.m_sParentIndex;
			m_sIndex = r.m_sIndex;
			m_sError = r.m_sError;
		}
		return *this;
	}
};


static void ReportIndexesName ( int iSpanStart, int iSpandEnd, const CSphVector<SearchFailure_t> & dLog, StringBuilder_c & sOut );

class SearchFailuresLog_c
{
	CSphVector<SearchFailure_t>		m_dLog;

public:
	void Submit ( const char * sIndex, const char * sParentIndex , const char * sError )
	{
		SearchFailure_t & tEntry = m_dLog.Add ();
		tEntry.m_sParentIndex = sParentIndex;
		tEntry.m_sIndex = sIndex;
		tEntry.m_sError = sError;
	}

	void SubmitVa ( const char * sIndex, const char * sParentIndex, const char * sTemplate, va_list ap )
	{
		StringBuilder_c tError;
		tError.vAppendf ( sTemplate, ap );

		SearchFailure_t &tEntry = m_dLog.Add ();
		tEntry.m_sParentIndex = sParentIndex;
		tEntry.m_sIndex = sIndex;
		tError.MoveTo ( tEntry.m_sError );
	}

	inline void Submit ( const CSphString & sIndex, const char * sParentIndex, const char * sError )
	{
		Submit ( sIndex.cstr(), sParentIndex, sError );
	}

	void SubmitEx ( const char * sIndex, const char * sParentIndex, const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 4, 5 ) ) )
	{
		va_list ap;
		va_start ( ap, sTemplate );
		SubmitVa ( sIndex, sParentIndex, sTemplate, ap);
		va_end ( ap );
	}

	void SubmitEx ( const CSphString &sIndex, const char * sParentIndex, const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 4, 5 ) ) )
	{
		va_list ap;
		va_start ( ap, sTemplate );
		SubmitVa ( sIndex.cstr(), sParentIndex, sTemplate, ap );
		va_end ( ap );
	}

	bool IsEmpty ()
	{
		return m_dLog.GetLength()==0;
	}

	int GetReportsCount()
	{
		return m_dLog.GetLength();
	}

	void BuildReport ( StringBuilder_c & sReport )
	{
		if ( IsEmpty() )
			return;

		// collapse same messages
		m_dLog.Uniq ();
		int iSpanStart = 0;
		Comma_c sColon( { ";\n", 2 } );

		for ( int i=1; i<=m_dLog.GetLength(); ++i )
		{
			// keep scanning while error text is the same
			if ( i!=m_dLog.GetLength() )
				if ( m_dLog[i].m_sError==m_dLog[i-1].m_sError )
					continue;

			sReport << sColon << "index ";

			ReportIndexesName ( iSpanStart, i, m_dLog, sReport );
			sReport << m_dLog[iSpanStart].m_sError;

			// done
			iSpanStart = i;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// SIGNAL HANDLERS
/////////////////////////////////////////////////////////////////////////////
void Shutdown () REQUIRES ( MainThread ) NO_THREAD_SAFETY_ANALYSIS
{
	// force even long time searches to shut
	sphInterruptNow ();

#if !USE_WINDOWS
	int fdStopwait = -1;
#endif
	bool bAttrsSaveOk = true;
	if ( g_pShared )
		g_pShared->m_bDaemonAtShutdown = true;

#if !USE_WINDOWS
	// stopwait handshake
	CSphString sPipeName = GetNamedPipeName ( getpid() );
	fdStopwait = ::open ( sPipeName.cstr(), O_WRONLY | O_NONBLOCK );
	if ( fdStopwait>=0 )
	{
		DWORD uHandshakeOk = 0;
		int VARIABLE_IS_NOT_USED iDummy = ::write ( fdStopwait, &uHandshakeOk, sizeof(DWORD) );
	}
#endif



	int64_t tmShutStarted = sphMicroTimer ();

	// release all planned/scheduled tasks
	TaskManager::ShutDown();

	ShutdownFlushingMutable();

	// stop search threads; up to shutdown_timeout seconds
	WaitFinishPreread ( g_iShutdownTimeoutUs );

	// save attribute updates for all local indexes
	bAttrsSaveOk = FinallySaveIndexes();

	// right before unlock loop
	if ( g_bJsonConfigLoadedOk )
	{
		CSphString sError;
		SaveConfigInt(sError);
	}

	// stop netloop processing
	for ( auto & pNetLoop : g_dNetLoops )
	{
		pNetLoop->StopNetLoop ();
		SafeRelease ( pNetLoop );
	}

	// stop netloop threads
	if ( g_pTickPoolThread )
		g_pTickPoolThread->StopAll ();

	// call scheduled callbacks:
	// shutdown replication,
	// shutdown ssl,
	// shutdown tick threads,
	// shutdown alone tasks
	searchd::FireShutdownCbs ();

	while ( ( myinfo::CountClients ()>0 ) && ( sphMicroTimer ()-tmShutStarted )<g_iShutdownTimeoutUs )
		sphSleepMsec ( 50 );

	if ( myinfo::CountClients ()>0 )
	{
		int64_t tmDelta = sphMicroTimer ()-tmShutStarted;
		sphWarning ( "still %d alive tasks during shutdown, after %d.%03d sec", myinfo::CountClients (), (int) ( tmDelta
				/ 1000000 ), (int) ( ( tmDelta / 1000 ) % 1000 ) );
	}

	// unlock indexes and release locks if needed
	for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next(); )
	{
		auto pIdx = (const ServedDesc_t *) it.Get(); // breaks any access, but we're finishing, that's ok.
		if ( pIdx && pIdx->m_pIndex )
			pIdx->m_pIndex->Unlock();
	}
	SafeDelete ( g_pLocalIndexes );

	// unlock Distr indexes automatically done by d-tr
	SafeDelete ( g_pDistIndexes );

	// clear shut down of rt indexes + binlog
	sphDoneIOStats();
	sphRTDone();

	ShutdownDocstore();
	sphShutdownWordforms ();
	sph::ShutdownGlobalIDFs ();
	sphAotShutdown ();

	for ( auto& dListener : g_dListeners )
		if ( dListener.m_iSock>=0 )
			sphSockClose ( dListener.m_iSock );

	ClosePersistentSockets();

	// close pid
	if ( g_iPidFD!=-1 )
		::close ( g_iPidFD );
	g_iPidFD = -1;

	// remove pid file, if we owned it
	if ( g_bPidIsMine && !g_sPidFile.IsEmpty() )
		::unlink ( g_sPidFile.cstr() );

	hazard::Shutdown ();
	sphInfo ( "shutdown complete" );

	Threads::Done ( g_iLogFile );

#if USE_WINDOWS
	CloseHandle ( g_hPipe );
#else
	if ( fdStopwait>=0 )
	{
		DWORD uStatus = bAttrsSaveOk;
		int VARIABLE_IS_NOT_USED iDummy = ::write ( fdStopwait, &uStatus, sizeof(DWORD) );
		::close ( fdStopwait );
	}
#endif
}

void sighup ( int )
{
	g_bGotSighup = 1;
}


void sigterm ( int )
{
	// tricky bit
	// we can't call exit() here because malloc()/free() are not re-entrant
	// we could call _exit() but let's try to die gracefully on TERM
	// and let signal sender wait and send KILL as needed
	sphInterruptNow();
}


void sigusr1 ( int )
{
	g_bGotSigusr1 = true;
}

struct QueryCopyState_t
{
	BYTE * m_pDst;
	BYTE * m_pDstEnd;
	const BYTE * m_pSrc;
	const BYTE * m_pSrcEnd;
};

// crash query handler
static const int g_iQueryLineLen = 80;
static const char g_dEncodeBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
bool sphCopyEncodedBase64 ( QueryCopyState_t & tEnc )
{
	BYTE * pDst = tEnc.m_pDst;
	const BYTE * pDstBase = tEnc.m_pDst;
	const BYTE * pSrc = tEnc.m_pSrc;
	const BYTE * pDstEnd = tEnc.m_pDstEnd-5;
	const BYTE * pSrcEnd = tEnc.m_pSrcEnd-3;

	while ( pDst<=pDstEnd && pSrc<=pSrcEnd )
	{
		// put line delimiter at max line length
		if ( ( ( pDst-pDstBase ) % g_iQueryLineLen )>( ( pDst-pDstBase+4 ) % g_iQueryLineLen ) )
			*pDst++ = '\n';

		// Convert to big endian
		DWORD uSrc = ( pSrc[0] << 16 ) | ( pSrc[1] << 8 ) | ( pSrc[2] );
		pSrc += 3;

		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x00FC0000 ) >> 18 ];
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x0003F000 ) >> 12 ];
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x00000FC0 ) >> 6 ];
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x0000003F ) ];
	}

	// there is a tail in source data and a room for it at destination buffer
	if ( pSrc<tEnc.m_pSrcEnd && ( tEnc.m_pSrcEnd-pSrc<3 ) && ( pDst<=pDstEnd-4 ) )
	{
		int iLeft = ( tEnc.m_pSrcEnd - pSrc ) % 3;
		if ( iLeft==1 )
		{
			DWORD uSrc = pSrc[0]<<16;
			pSrc += 1;
			*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x00FC0000 ) >> 18 ];
			*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x0003F000 ) >> 12 ];
			*pDst++ = '=';
			*pDst++ = '=';
		} else if ( iLeft==2 )
		{
			DWORD uSrc = ( pSrc[0]<<16 ) | ( pSrc[1] << 8 );
			pSrc += 2;
			*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x00FC0000 ) >> 18 ];
			*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x0003F000 ) >> 12 ];
			*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x00000FC0 ) >> 6 ];
			*pDst++ = '=';
		}
	}

	tEnc.m_pDst = pDst;
	tEnc.m_pSrc = pSrc;

	return ( tEnc.m_pSrc<tEnc.m_pSrcEnd );
}

static bool sphCopySphinxQL ( QueryCopyState_t & tState )
{
	BYTE * pDst = tState.m_pDst;
	const BYTE * pSrc = tState.m_pSrc;
	BYTE * pNextLine = pDst+g_iQueryLineLen;

	while ( pDst<tState.m_pDstEnd && pSrc<tState.m_pSrcEnd )
	{
		if ( pDst>pNextLine && pDst+1<tState.m_pDstEnd && ( sphIsSpace ( *pSrc ) || *pSrc==',' ) )
		{
			*pDst++ = *pSrc++;
			*pDst++ = '\n';
			pNextLine = pDst + g_iQueryLineLen;
		} else
		{
			*pDst++ = *pSrc++;
		}
	}

	tState.m_pDst = pDst;
	tState.m_pSrc = pSrc;

	return ( tState.m_pSrc<tState.m_pSrcEnd );
}

static bool sphCopySphinxHttp ( QueryCopyState_t & tState )
{
	BYTE * pDst = tState.m_pDst;
	const BYTE * pSrc = tState.m_pSrc;

	while ( pDst<tState.m_pDstEnd && pSrc<tState.m_pSrcEnd )
	{
		*pDst++ = *pSrc++;
	}

	tState.m_pDst = pDst;
	tState.m_pSrc = pSrc;

	return ( tState.m_pSrc<tState.m_pSrcEnd );
}


typedef bool CopyQuery_fn ( QueryCopyState_t & tState );

#define SPH_TIME_PID_MAX_SIZE 256
const char		g_sCrashedBannerAPI[] = "\n--- crashed SphinxAPI request dump ---\n";
const char		g_sCrashedBannerMySQL[] = "\n--- crashed SphinxQL request dump ---\n";
const char		g_sCrashedBannerHTTP[] = "\n--- crashed HTTP request dump ---\n";
const char		g_sCrashedBannerBad[] = "\n--- crashed invalid query ---\n";
const char		g_sCrashedBannerTail[] = "\n--- request dump end ---\n";
const char		g_sCrashedIndex[] = "--- local index:";
const char		g_sEndLine[] = "\n";
#if USE_WINDOWS
const char		g_sMinidumpBanner[] = "minidump located at: ";
#endif
#if SPH_ALLOCS_PROFILER
const char		g_sMemoryStatBanner[] = "\n--- memory statistics ---\n";
#endif
static BYTE		g_dCrashQueryBuff [4096];
static char		g_sCrashInfo [SPH_TIME_PID_MAX_SIZE] = "[][]\n";
static int		g_iCrashInfoLen = 0;

#if USE_WINDOWS
static char		g_sMinidump[SPH_TIME_PID_MAX_SIZE] = "";
#endif

#if !USE_WINDOWS
void CrashLogger::HandleCrash ( int sig ) NO_THREAD_SAFETY_ANALYSIS
#else
LONG WINAPI CrashLogger::HandleCrash ( EXCEPTION_POINTERS * pExc )
#endif // !USE_WINDOWS
{
	if ( g_iLogFile<0 )
	{
		if ( g_bCoreDump )
		{
			CRASH_EXIT_CORE;
		} else
		{
			CRASH_EXIT;
		}
	}

	// log [time][pid]
	sphSeek ( g_iLogFile, 0, SEEK_END );
	sphWrite ( g_iLogFile, g_sCrashInfo, g_iCrashInfoLen );

	// log query
	auto& tQuery = GlobalCrashQueryGetRef ();

	bool bValidQuery = ( tQuery.m_pQuery && tQuery.m_iSize>0 );
#if !USE_WINDOWS
	if ( bValidQuery )
	{
		size_t iPageSize = getpagesize();

		// FIXME! That is too complex way, remove all of this and just move query dump to the bottom
		// remove also mincore_test.cmake, it's invokation from CMakeLists.txt and HAVE_UNSIGNED_MINCORE
		// declatarion from config_cmake.h.in
#if HAVE_UNSIGNED_MINCORE
		BYTE dPages = 0;
#else
		char dPages = 0;
#endif

		uintptr_t pPageStart = (uintptr_t )( tQuery.m_pQuery );
		pPageStart &= ~( iPageSize - 1 );
		bValidQuery &= ( mincore ( ( void * ) pPageStart, 1, &dPages )==0 );

		uintptr_t pPageEnd = (uintptr_t )( tQuery.m_pQuery + tQuery.m_iSize - 1 );
		pPageEnd &= ~( iPageSize - 1 );
		bValidQuery &= ( mincore ( ( void * ) pPageEnd, 1, &dPages )==0 );
	}
#endif

	// request dump banner
	const char * pBanner = g_sCrashedBannerAPI;
	int iBannerLen = sizeof(g_sCrashedBannerAPI) - 1;
	if ( tQuery.m_bMySQL )
	{
		pBanner = g_sCrashedBannerMySQL;
		iBannerLen = sizeof(g_sCrashedBannerMySQL) - 1;
	}
	if ( tQuery.m_bHttp )
	{
		pBanner = g_sCrashedBannerHTTP;
		iBannerLen = sizeof(g_sCrashedBannerHTTP) - 1;
	}
	if ( !bValidQuery )
	{
		iBannerLen = sizeof(g_sCrashedBannerBad) - 1;
		pBanner = g_sCrashedBannerBad;
	}
	sphWrite ( g_iLogFile, pBanner, iBannerLen );

	// query
	if ( bValidQuery )
	{
		QueryCopyState_t tCopyState;
		tCopyState.m_pDst = g_dCrashQueryBuff;
		tCopyState.m_pDstEnd = g_dCrashQueryBuff + sizeof(g_dCrashQueryBuff);
		tCopyState.m_pSrc = tQuery.m_pQuery;
		tCopyState.m_pSrcEnd = tQuery.m_pQuery + tQuery.m_iSize;

		CopyQuery_fn * pfnCopy = NULL;
		if ( !tQuery.m_bMySQL && !tQuery.m_bHttp )
		{
			pfnCopy = &sphCopyEncodedBase64;

			// should be power of 3 to seamlessly convert to BASE64
			BYTE dHeader[] = {
				(BYTE)( ( tQuery.m_uCMD>>8 ) & 0xff ),
				(BYTE)( tQuery.m_uCMD & 0xff ),
				(BYTE)( ( tQuery.m_uVer>>8 ) & 0xff ),
				(BYTE)( tQuery.m_uVer & 0xff ),
				(BYTE)( ( tQuery.m_iSize>>24 ) & 0xff ),
				(BYTE)( ( tQuery.m_iSize>>16 ) & 0xff ),
				(BYTE)( ( tQuery.m_iSize>>8 ) & 0xff ),
				(BYTE)( tQuery.m_iSize & 0xff ),
				*tQuery.m_pQuery
			};

			QueryCopyState_t tHeaderState;
			tHeaderState.m_pDst = g_dCrashQueryBuff;
			tHeaderState.m_pDstEnd = g_dCrashQueryBuff + sizeof(g_dCrashQueryBuff);
			tHeaderState.m_pSrc = dHeader;
			tHeaderState.m_pSrcEnd = dHeader + sizeof(dHeader);
			pfnCopy ( tHeaderState );
			assert ( tHeaderState.m_pSrc==tHeaderState.m_pSrcEnd );
			tCopyState.m_pDst = tHeaderState.m_pDst;
			tCopyState.m_pSrc++;
		} else if ( tQuery.m_bHttp )
		{
			pfnCopy = &sphCopySphinxHttp;
		} else
		{
			pfnCopy = &sphCopySphinxQL;
		}

		while ( pfnCopy ( tCopyState ) )
		{
			sphWrite ( g_iLogFile, g_dCrashQueryBuff, tCopyState.m_pDst-g_dCrashQueryBuff );
			tCopyState.m_pDst = g_dCrashQueryBuff; // reset the destination buffer
		}
		assert ( tCopyState.m_pSrc==tCopyState.m_pSrcEnd );

		int iLeft = tCopyState.m_pDst-g_dCrashQueryBuff;
		if ( iLeft>0 )
		{
			sphWrite ( g_iLogFile, g_dCrashQueryBuff, iLeft );
		}
	}

	// tail
	sphWrite ( g_iLogFile, g_sCrashedBannerTail, sizeof(g_sCrashedBannerTail)-1 );

	// index name
	sphWrite ( g_iLogFile, g_sCrashedIndex, sizeof (g_sCrashedIndex)-1 );
	if ( tQuery.m_pIndex && tQuery.m_iIndexLen )
		sphWrite ( g_iLogFile, tQuery.m_pIndex, tQuery.m_iIndexLen );
	sphWrite ( g_iLogFile, g_sEndLine, sizeof (g_sEndLine)-1 );

	sphSafeInfo ( g_iLogFile, szMANTICORE_NAME );

#if USE_WINDOWS
	// mini-dump reference
	int iMiniDumpLen = snprintf ( (char *)g_dCrashQueryBuff, sizeof(g_dCrashQueryBuff),
		"%s %s.%p.mdmp\n", g_sMinidumpBanner, g_sMinidump, tQuery.m_pQuery );
	sphWrite ( g_iLogFile, g_dCrashQueryBuff, iMiniDumpLen );
	snprintf ( (char *)g_dCrashQueryBuff, sizeof(g_dCrashQueryBuff), "%s.%p.mdmp",
		g_sMinidump, tQuery.m_pQuery );
#endif

	// log trace
#if !USE_WINDOWS
	sphSafeInfo ( g_iLogFile, "Handling signal %d", sig );
	// print message to stdout during daemon start
	if ( g_bLogStdout && g_iLogFile!=STDOUT_FILENO )
		sphSafeInfo ( STDOUT_FILENO, "Crash!!! Handling signal %d", sig );
	sphBacktrace ( g_iLogFile, g_bSafeTrace );
#else
	sphBacktrace ( pExc, (char *)g_dCrashQueryBuff );
#endif

	// threads table
	sphSafeInfo ( g_iLogFile, "--- active threads ---" );
	int iThd = 0;
	int iAllThd = 0;
	Threads::IterateActive ( [&iThd,&iAllThd] ( Threads::LowThreadDesc_t * pThread )
	{
		if ( pThread )
		{
			auto pSrc = (ClientTaskInfo_t *) pThread->m_pTaskInfo.load ( std::memory_order_relaxed );
			if ( pSrc ) ++iAllThd;
			for ( ; pSrc; pSrc = (ClientTaskInfo_t *) pSrc->m_pPrev.load ( std::memory_order_relaxed ) )
				if ( pSrc->m_eType==ClientTaskInfo_t::m_eTask )
				{
					sphSafeInfo ( g_iLogFile, "thd %d (%s), proto %s, state %s, command %s", iThd,
							pThread->m_sThreadName.cstr(),
							ProtoName (pSrc->m_eProto),
							ThdStateName (pSrc->m_eThdState),
							pSrc->m_sCommand ? pSrc->m_sCommand : "-" );
					++iThd;
					break;
				}
		}
	} );

	sphSafeInfo ( g_iLogFile, "--- Totally %d threads, and %d client-working threads ---", iAllThd, iThd );

	// memory info
#if SPH_ALLOCS_PROFILER
	sphWrite ( g_iLogFile, g_sMemoryStatBanner, sizeof ( g_sMemoryStatBanner )-1 );
	sphMemStatDump ( g_iLogFile );
#endif

	sphSafeInfo ( g_iLogFile, "------- CRASH DUMP END -------" );

	if ( g_bCoreDump )
	{
		CRASH_EXIT_CORE;
	} else
	{
		CRASH_EXIT;
	}
}


void CrashLogger::SetupTimePID ()
{
	char sTimeBuf[SPH_TIME_PID_MAX_SIZE];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	g_iCrashInfoLen = snprintf ( g_sCrashInfo, SPH_TIME_PID_MAX_SIZE-1,
		"------- FATAL: CRASH DUMP -------\n[%s] [%5d]\n", sTimeBuf, (int)getpid() );
}

#if USE_WINDOWS
void SetSignalHandlers ( bool )
{
	sphBacktraceInit ();
	snprintf ( g_sMinidump, SPH_TIME_PID_MAX_SIZE-1, "%s.%d", g_sPidFile.scstr(), (int)getpid() );
	SetUnhandledExceptionFilter ( CrashLogger::HandleCrash );
}
#else
void SetSignalHandlers ( bool bAllowCtrlC=false ) REQUIRES ( MainThread )
{
	sphBacktraceInit ();
	struct sigaction sa;
	sigfillset ( &sa.sa_mask );
	sa.sa_flags = SA_NOCLDSTOP;

	bool bSignalsSet = false;
	auto dFatalOnFail = AtScopeExit( [ &bSignalsSet ]
	{
		if ( !bSignalsSet )
			sphFatal( "sigaction(): %s", strerrorm(errno));
	} );

	sa.sa_handler = sigterm;	if ( sigaction ( SIGTERM, &sa, NULL )!=0 ) return;
	if ( !bAllowCtrlC )
	{
		sa.sa_handler = sigterm;
		if ( sigaction ( SIGINT, &sa, NULL )!=0 )
			return;
	}
	sa.sa_handler = sighup;		if ( sigaction ( SIGHUP, &sa, NULL )!=0 ) 		return;
	sa.sa_handler = sigusr1;	if ( sigaction ( SIGUSR1, &sa, NULL )!=0 )		return;
	sa.sa_handler = SIG_IGN;	if ( sigaction ( SIGPIPE, &sa, NULL )!=0 ) return;

	sa.sa_flags |= SA_RESETHAND;

	static BYTE exception_handler_stack[SIGSTKSZ];
	stack_t ss;
	ss.ss_sp = exception_handler_stack;
	ss.ss_flags = 0;
	ss.ss_size = SIGSTKSZ;
	sigaltstack( &ss, 0 );
	sa.sa_flags |= SA_ONSTACK;

	sa.sa_handler = CrashLogger::HandleCrash;	if ( sigaction ( SIGSEGV, &sa, NULL )!=0 ) return;
	sa.sa_handler = CrashLogger::HandleCrash;	if ( sigaction ( SIGBUS, &sa, NULL )!=0 ) return;
	sa.sa_handler = CrashLogger::HandleCrash;	if ( sigaction ( SIGABRT, &sa, NULL )!=0 ) return;
	sa.sa_handler = CrashLogger::HandleCrash;	if ( sigaction ( SIGILL, &sa, NULL )!=0 ) return;
	sa.sa_handler = CrashLogger::HandleCrash;	if ( sigaction ( SIGFPE, &sa, NULL )!=0 ) return;

	bSignalsSet = true;
}
#endif

#if !USE_WINDOWS
int sphCreateUnixSocket ( const char * sPath ) REQUIRES ( MainThread )
{
	static struct sockaddr_un uaddr;
	size_t len = strlen ( sPath );

	if ( len + 1 > sizeof( uaddr.sun_path ) )
		sphFatal ( "UNIX socket path is too long (len=%d)", (int)len );

	sphInfo ( "listening on UNIX socket %s", sPath );

	memset ( &uaddr, 0, sizeof(uaddr) );
	uaddr.sun_family = AF_UNIX;
	memcpy ( uaddr.sun_path, sPath, len + 1 );

	int iSock = socket ( AF_UNIX, SOCK_STREAM, 0 );
	if ( iSock==-1 )
		sphFatal ( "failed to create UNIX socket: %s", sphSockError() );

	if ( unlink ( sPath )==-1 )
	{
		if ( errno!=ENOENT )
			sphFatal ( "unlink() on UNIX socket file failed: %s", sphSockError() );
	}

	int iMask = umask ( 0 );
	if ( bind ( iSock, (struct sockaddr *)&uaddr, sizeof(uaddr) )!=0 )
		sphFatal ( "bind() on UNIX socket failed: %s", sphSockError() );
	umask ( iMask );

	return iSock;
}
#endif // !USE_WINDOWS


int sphCreateInetSocket ( const ListenerDesc_t & tDesc ) REQUIRES ( MainThread )
{
	auto uAddr = tDesc.m_uIP;
	auto iPort = tDesc.m_iPort;
	char sAddress[SPH_ADDRESS_SIZE];
	sphFormatIP ( sAddress, SPH_ADDRESS_SIZE, uAddr );
	auto sVip = [&tDesc] { return tDesc.m_bVIP?"VIP ":""; };

	if ( uAddr==htonl ( INADDR_ANY ) )
		sphInfo ( "listening on all interfaces for %s%s, port=%d", sVip(), ProtoName ( tDesc.m_eProto), iPort );
	else
		sphInfo ( "listening on %s:%d for %s%s", sAddress, iPort, sVip(), ProtoName ( tDesc.m_eProto ) );

	static struct sockaddr_in iaddr;
	memset ( &iaddr, 0, sizeof(iaddr) );
	iaddr.sin_family = AF_INET;
	iaddr.sin_addr.s_addr = uAddr;
	iaddr.sin_port = htons ( (short)iPort );

	int iSock = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( iSock==-1 )
		sphFatal ( "failed to create TCP socket: %s", sphSockError() );

	int iOn = 1;
	if ( setsockopt ( iSock, SOL_SOCKET, SO_REUSEADDR, (char*)&iOn, sizeof(iOn) ) )
		sphWarning ( "setsockopt(SO_REUSEADDR) failed: %s", sphSockError() );
#if HAVE_SO_REUSEPORT
	if ( setsockopt ( iSock, SOL_SOCKET, SO_REUSEPORT, (char*)&iOn, sizeof(iOn) ) )
		sphWarning ( "setsockopt(SO_REUSEPORT) failed: %s", sphSockError() );
#endif
#ifdef TCP_NODELAY
	if ( setsockopt ( iSock, IPPROTO_TCP, TCP_NODELAY, (char*)&iOn, sizeof(iOn) ) )
		sphWarning ( "setsockopt(TCP_NODELAY) failed: %s", sphSockError() );
#endif

	int iTries = 12;
	int iRes;
	do
	{
		iRes = bind ( iSock, (struct sockaddr *)&iaddr, sizeof(iaddr) );
		if ( iRes==0 )
			break;

		sphInfo ( "bind() failed on %s, retrying...", sAddress );
		sphSleepMsec ( 3000 );
	} while ( --iTries>0 );
	if ( iRes )
		sphFatal ( "bind() failed on %s: %s", sAddress, sphSockError() );

	return iSock;
}

ListenerDesc_t MakeAnyListener ( int iPort, Proto_e eProto=Proto_e::SPHINX )
{
	ListenerDesc_t tDesc;
	tDesc.m_eProto = eProto;
	tDesc.m_uIP = htonl ( INADDR_ANY );
	tDesc.m_iPort = iPort;
	tDesc.m_iPortsCount = 0;
	tDesc.m_bVIP = false;
	return tDesc;
}

// add any listener we will serve by our own (i.e. NO galera's since it is not our deal)
bool AddGlobalListener ( const ListenerDesc_t& tDesc ) REQUIRES ( MainThread )
{
	if ( tDesc.m_eProto==Proto_e::REPLICATION )
		return false;

	Listener_t tListener;
	tListener.m_eProto = tDesc.m_eProto;
	tListener.m_bTcp = true;
	tListener.m_bVIP = tDesc.m_bVIP;

#if !USE_WINDOWS
	if ( !tDesc.m_sUnix.IsEmpty () )
	{
		tListener.m_iSock = sphCreateUnixSocket ( tDesc.m_sUnix.cstr () );
		tListener.m_bTcp = false;
	} else
#endif
		tListener.m_iSock = sphCreateInetSocket ( tDesc );

	g_dListeners.Add ( tListener );
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// unpack Mysql Length-coded number
static int MysqlUnpack ( InputBuffer_c & tReq, DWORD * pSize )
{
	assert ( pSize );

	int iRes = tReq.GetByte();
	--*pSize;
	if ( iRes < 251 )
		return iRes;

	if ( iRes==0xFC )
	{
		*pSize -=2;
		return tReq.GetByte() + ((int)tReq.GetByte()<<8);
	}

	if ( iRes==0xFD )
	{
		*pSize -= 3;
		return tReq.GetByte() + ((int)tReq.GetByte()<<8) + ((int)tReq.GetByte()<<16);
	}

	if ( iRes==0xFE )
		iRes = tReq.GetByte() + ((int)tReq.GetByte()<<8) + ((int)tReq.GetByte()<<16) + ((int)tReq.GetByte()<<24);

	tReq.GetByte();
	tReq.GetByte();
	tReq.GetByte();
	tReq.GetByte();
	*pSize -= 8;
	return iRes;
}

/////////////////////////////////////////////////////////////////////////////
void ISphOutputBuffer::SendBytes ( const void * pBuf, int iLen )
{
	m_dBuf.Append ( pBuf, iLen );
}

void ISphOutputBuffer::SendBytes ( const char * pBuf )
{
	if ( !pBuf )
		return;
	SendBytes ( pBuf, (int) strlen ( pBuf ) );
}

void ISphOutputBuffer::SendBytes ( const CSphString& sStr )
{
	SendBytes ( sStr.cstr(), sStr.Length() );
}

void ISphOutputBuffer::SendBytes ( const VecTraits_T<BYTE> & dBuf )
{
	m_dBuf.Append ( dBuf );
}

void ISphOutputBuffer::SendBytes ( const StringBuilder_c &dBuf )
{
	SendBytes ( dBuf.begin(), dBuf.GetLength () );
}

void ISphOutputBuffer::SendArray ( const ISphOutputBuffer &tOut )
{
	int iLen = tOut.m_dBuf.GetLength();
	SendInt ( iLen );
	SendBytes ( tOut.m_dBuf.Begin(), iLen );
}

void ISphOutputBuffer::SendArray ( const VecTraits_T<BYTE> &dBuf, int iElems )
{
	if ( iElems==-1 )
	{
		SendInt ( dBuf.GetLength () );
		SendBytes ( dBuf );
		return;
	}
	assert ( dBuf.GetLength() == (int) dBuf.GetLengthBytes() );
	assert ( iElems<=dBuf.GetLength ());
	SendInt ( iElems );
	SendBytes ( dBuf.begin(), iElems );
}

void ISphOutputBuffer::SendArray ( const void * pBuf, int iLen )
{
	if ( !pBuf )
		iLen=0;
	assert ( iLen>=0 );
	SendInt ( iLen );
	SendBytes ( pBuf, iLen );
}

void ISphOutputBuffer::SendArray ( const StringBuilder_c &dBuf )
{
	SendArray ( dBuf.begin(), dBuf.GetLength () );
}

void SendErrorReply ( ISphOutputBuffer & tOut, const char * sTemplate, ... )
{
	CSphString sError;
	va_list ap;
	va_start ( ap, sTemplate );
	sError.SetSprintfVa ( sTemplate, ap );
	va_end ( ap );

	auto tHdr = APIHeader ( tOut, SEARCHD_ERROR );
	tOut.SendString ( sError.cstr() );

	// --console logging
	if ( g_bOptNoDetach && g_eLogFormat!=LOG_FORMAT_SPHINXQL )
		sphInfo ( "query error: %s", sError.cstr() );
}

// fix MSVC 2005 fuckup
#if USE_WINDOWS
#pragma conform(forScope,on)
#endif

void DistributedIndex_t::GetAllHosts ( VectorAgentConn_t &dTarget ) const
{
	for ( const auto * pMultiAgent : m_dAgents )
		for ( const auto & dHost : *pMultiAgent )
		{
			auto * pAgent = new AgentConn_t;
			pAgent->m_tDesc.CloneFrom ( dHost );
			pAgent->m_iMyQueryTimeoutMs = m_iAgentQueryTimeoutMs;
			pAgent->m_iMyConnectTimeoutMs = m_iAgentConnectTimeoutMs;
			dTarget.Add ( pAgent );
		}
}

void DistributedIndex_t::ForEveryHost ( ProcessFunctor pFunc )
{
	for ( auto * pAgent : m_dAgents )
		for ( auto &dHost : *pAgent )
			pFunc ( dHost );
}

DistributedIndex_t::~DistributedIndex_t ()
{
	sphLogDebugv ( "DistributedIndex_t %p removed", this );
	for ( auto * pAgent : m_dAgents )
		SafeRelease ( pAgent );

	// cleanup global
	MultiAgentDesc_c::CleanupOrphaned ();
};

/////////////////////////////////////////////////////////////////////////////
// SEARCH HANDLER
/////////////////////////////////////////////////////////////////////////////

class SearchRequestBuilder_c : public RequestBuilder_i
{
public:
	SearchRequestBuilder_c ( const CSphVector<CSphQuery> & dQueries, int iStart, int iEnd, int iDivideLimits )
		: m_dQueries ( dQueries ), m_iStart ( iStart ), m_iEnd ( iEnd ), m_iDivideLimits ( iDivideLimits )
	{}

	void		BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final;

protected:
	void		SendQuery ( const char * sIndexes, ISphOutputBuffer & tOut, const CSphQuery & q, int iWeight, int iAgentQueryTimeout ) const;

protected:
	const CSphVector<CSphQuery> &		m_dQueries;
	const int							m_iStart;
	const int							m_iEnd;
	const int							m_iDivideLimits;
};


class SearchReplyParser_c : public ReplyParser_i, public ISphNoncopyable
{
public:
	SearchReplyParser_c ( int iStart, int iEnd )
		: m_iStart ( iStart )
		, m_iEnd ( iEnd )
	{}

	bool		ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final;

private:
	int					m_iStart;
	int					m_iEnd;

	void				ParseSchema ( CSphQueryResult & tRes, MemInputBuffer_c & tReq ) const;
	void				ParseMatch ( CSphMatch & tMatch, MemInputBuffer_c & tReq, const CSphSchema & tSchema, bool bAgent64 ) const;
};

/////////////////////////////////////////////////////////////////////////////

/// qflag means Query Flag
/// names are internal to searchd and may be changed for clarity
/// values are communicated over network between searchds and APIs and MUST NOT CHANGE
enum
{
	QFLAG_REVERSE_SCAN			= 1UL << 0,
	QFLAG_SORT_KBUFFER			= 1UL << 1,
	QFLAG_MAX_PREDICTED_TIME	= 1UL << 2,
	QFLAG_SIMPLIFY				= 1UL << 3,
	QFLAG_PLAIN_IDF				= 1UL << 4,
	QFLAG_GLOBAL_IDF			= 1UL << 5,
	QFLAG_NORMALIZED_TF			= 1UL << 6,
	QFLAG_LOCAL_DF				= 1UL << 7,
	QFLAG_LOW_PRIORITY			= 1UL << 8,
	QFLAG_FACET					= 1UL << 9,
	QFLAG_FACET_HEAD			= 1UL << 10,
	QFLAG_JSON_QUERY			= 1UL << 11
};

void operator<< ( ISphOutputBuffer & tOut, const CSphNamedInt & tValue )
{
	tOut.SendString ( tValue.first.cstr () );
	tOut.SendInt ( tValue.second );
}

void operator>> ( InputBuffer_c & dIn, CSphNamedInt & tValue )
{
	tValue.first = dIn.GetString ();
	tValue.second = dIn.GetInt ();
}

void SearchRequestBuilder_c::SendQuery ( const char * sIndexes, ISphOutputBuffer & tOut, const CSphQuery & q, int iWeight, int iAgentQueryTimeout ) const
{
	bool bAgentWeight = ( iWeight!=-1 );
	// starting with command version 1.27, flags go first
	// reason being, i might add flags that affect *any* of the subsequent data (eg. qflag_pack_ints)
	DWORD uFlags = 0;
	uFlags |= QFLAG_REVERSE_SCAN * q.m_bReverseScan;
	uFlags |= QFLAG_SORT_KBUFFER * q.m_bSortKbuffer;
	uFlags |= QFLAG_MAX_PREDICTED_TIME * ( q.m_iMaxPredictedMsec > 0 );
	uFlags |= QFLAG_SIMPLIFY * q.m_bSimplify;
	uFlags |= QFLAG_PLAIN_IDF * q.m_bPlainIDF;
	uFlags |= QFLAG_GLOBAL_IDF * q.m_bGlobalIDF;
	uFlags |= QFLAG_NORMALIZED_TF * q.m_bNormalizedTFIDF;
	uFlags |= QFLAG_LOCAL_DF * q.m_bLocalDF;
	uFlags |= QFLAG_LOW_PRIORITY * q.m_bLowPriority;
	uFlags |= QFLAG_FACET * q.m_bFacet;
	uFlags |= QFLAG_FACET_HEAD * q.m_bFacetHead;

	if ( q.m_eQueryType==QUERY_JSON )
		uFlags |= QFLAG_JSON_QUERY;

	tOut.SendDword ( uFlags );

	// The Search Legacy
	tOut.SendInt ( 0 ); // offset is 0
	if ( !q.m_bHasOuter )
	{
		if ( m_iDivideLimits==1 )
			tOut.SendInt ( q.m_iMaxMatches ); // OPTIMIZE? normally, agent limit is max_matches, even if master limit is less
		else // FIXME!!! that is broken with offset + limit
			tOut.SendInt ( 1 + ( ( q.m_iOffset + q.m_iLimit )/m_iDivideLimits) );
	} else
	{
		// with outer order by, inner limit must match between agent and master
		tOut.SendInt ( q.m_iLimit );
	}
	tOut.SendInt ( (DWORD)q.m_eMode ); // match mode
	tOut.SendInt ( (DWORD)q.m_eRanker ); // ranking mode
	if ( q.m_eRanker==SPH_RANK_EXPR || q.m_eRanker==SPH_RANK_EXPORT )
		tOut.SendString ( q.m_sRankerExpr.cstr() );
	tOut.SendInt ( q.m_eSort ); // sort mode
	tOut.SendString ( q.m_sSortBy.cstr() ); // sort attr

	if ( q.m_eQueryType==QUERY_JSON )
		tOut.SendString ( q.m_sQuery.cstr() );
	else
	{
		if ( q.m_sRawQuery.IsEmpty() )
			tOut.SendString ( q.m_sQuery.cstr() );
		else
			tOut.SendString ( q.m_sRawQuery.cstr() ); // query
	}

	tOut.SendInt ( q.m_dWeights.GetLength() );
	ARRAY_FOREACH ( j, q.m_dWeights )
		tOut.SendInt ( q.m_dWeights[j] ); // weights
	tOut.SendString ( sIndexes ); // indexes
	tOut.SendInt ( 1 ); // id range bits
	tOut.SendUint64 ( uint64_t(0) ); // default full id range (any client range must be in filters at this stage)
	tOut.SendUint64 ( UINT64_MAX );
	tOut.SendInt ( q.m_dFilters.GetLength() );
	ARRAY_FOREACH ( j, q.m_dFilters )
	{
		const CSphFilterSettings & tFilter = q.m_dFilters[j];
		tOut.SendString ( tFilter.m_sAttrName.cstr() );
		tOut.SendInt ( tFilter.m_eType );
		switch ( tFilter.m_eType )
		{
			case SPH_FILTER_VALUES:
				tOut.SendInt ( tFilter.GetNumValues () );
				for ( int k = 0; k < tFilter.GetNumValues (); k++ )
					tOut.SendUint64 ( tFilter.GetValue ( k ) );
				break;

			case SPH_FILTER_RANGE:
				tOut.SendUint64 ( tFilter.m_iMinValue );
				tOut.SendUint64 ( tFilter.m_iMaxValue );
				break;

			case SPH_FILTER_FLOATRANGE:
				tOut.SendFloat ( tFilter.m_fMinValue );
				tOut.SendFloat ( tFilter.m_fMaxValue );
				break;

			case SPH_FILTER_USERVAR:
			case SPH_FILTER_STRING:
				tOut.SendString ( tFilter.m_dStrings.GetLength()==1 ? tFilter.m_dStrings[0].cstr() : nullptr );
				break;

			case SPH_FILTER_NULL:
				tOut.SendByte ( tFilter.m_bIsNull );
				break;

			case SPH_FILTER_STRING_LIST:
				tOut.SendInt ( tFilter.m_dStrings.GetLength() );
				ARRAY_FOREACH ( iString, tFilter.m_dStrings )
					tOut.SendString ( tFilter.m_dStrings[iString].cstr() );
				break;
			case SPH_FILTER_EXPRESSION: // need only name and type
				break;
		}
		tOut.SendInt ( tFilter.m_bExclude );
		tOut.SendInt ( tFilter.m_bHasEqualMin );
		tOut.SendInt ( tFilter.m_bHasEqualMax );
		tOut.SendInt ( tFilter.m_bOpenLeft );
		tOut.SendInt ( tFilter.m_bOpenRight );
		tOut.SendInt ( tFilter.m_eMvaFunc );
	}
	tOut.SendInt ( q.m_eGroupFunc );
	tOut.SendString ( q.m_sGroupBy.cstr() );
	if ( m_iDivideLimits==1 )
		tOut.SendInt ( q.m_iMaxMatches );
	else
		tOut.SendInt ( 1+(q.m_iMaxMatches/m_iDivideLimits) ); // Reduce the max_matches also.
	tOut.SendString ( q.m_sGroupSortBy.cstr() );
	tOut.SendInt ( q.m_iCutoff );
	tOut.SendInt ( q.m_iRetryCount<0 ? 0 : q.m_iRetryCount ); // runaround for old clients.
	tOut.SendInt ( q.m_iRetryDelay<0 ? 0 : q.m_iRetryDelay );
	tOut.SendString ( q.m_sGroupDistinct.cstr() );
	tOut.SendInt ( q.m_bGeoAnchor );
	if ( q.m_bGeoAnchor )
	{
		tOut.SendString ( q.m_sGeoLatAttr.cstr() );
		tOut.SendString ( q.m_sGeoLongAttr.cstr() );
		tOut.SendFloat ( q.m_fGeoLatitude );
		tOut.SendFloat ( q.m_fGeoLongitude );
	}
	if ( bAgentWeight )
	{
		tOut.SendInt ( 1 );
		tOut.SendString ( "*" );
		tOut.SendInt ( iWeight );
	} else
	{
		tOut.SendInt ( q.m_dIndexWeights.GetLength() );
		for ( const auto& dWeight : q.m_dIndexWeights )
			tOut << dWeight;
	}
	DWORD iQueryTimeout = ( q.m_uMaxQueryMsec ? q.m_uMaxQueryMsec : iAgentQueryTimeout );
	tOut.SendDword ( iQueryTimeout );
	tOut.SendInt ( q.m_dFieldWeights.GetLength() );
	for ( const auto & dWeight : q.m_dFieldWeights )
		tOut << dWeight;

	tOut.SendString ( q.m_sComment.cstr() );
	tOut.SendInt ( 0 ); // WAS: overrides
	tOut.SendString ( q.m_sSelect.cstr() );
	if ( q.m_iMaxPredictedMsec>0 )
		tOut.SendInt ( q.m_iMaxPredictedMsec );

	// emulate empty sud-select for agent (client ver 1.29) as master sends fixed outer offset+limits
	tOut.SendString ( NULL );
	tOut.SendInt ( 0 );
	tOut.SendInt ( 0 );
	tOut.SendInt ( q.m_bHasOuter );

	// master-agent extensions
	tOut.SendDword ( q.m_eCollation ); // v.1
	tOut.SendString ( q.m_sOuterOrderBy.cstr() ); // v.2
	if ( q.m_bHasOuter )
		tOut.SendInt ( q.m_iOuterOffset + q.m_iOuterLimit );
	tOut.SendInt ( q.m_iGroupbyLimit );
	tOut.SendString ( q.m_sUDRanker.cstr() );
	tOut.SendString ( q.m_sUDRankerOpts.cstr() );
	tOut.SendString ( q.m_sQueryTokenFilterLib.cstr() );
	tOut.SendString ( q.m_sQueryTokenFilterName.cstr() );
	tOut.SendString ( q.m_sQueryTokenFilterOpts.cstr() );
	tOut.SendInt ( q.m_dFilterTree.GetLength() );
	ARRAY_FOREACH ( i, q.m_dFilterTree )
	{
		tOut.SendInt ( q.m_dFilterTree[i].m_iLeft );
		tOut.SendInt ( q.m_dFilterTree[i].m_iRight );
		tOut.SendInt ( q.m_dFilterTree[i].m_iFilterItem );
		tOut.SendInt ( q.m_dFilterTree[i].m_bOr );
	}
	tOut.SendInt( q.m_dItems.GetLength() );
	ARRAY_FOREACH ( i, q.m_dItems )
	{
		const CSphQueryItem & tItem = q.m_dItems[i];
		tOut.SendString ( tItem.m_sAlias.cstr() );
		tOut.SendString ( tItem.m_sExpr.cstr() );
		tOut.SendDword ( tItem.m_eAggrFunc );
	}
	tOut.SendInt( q.m_dRefItems.GetLength() );
	ARRAY_FOREACH ( i, q.m_dRefItems )
	{
		const CSphQueryItem & tItem = q.m_dRefItems[i];
		tOut.SendString ( tItem.m_sAlias.cstr() );
		tOut.SendString ( tItem.m_sExpr.cstr() );
		tOut.SendDword ( tItem.m_eAggrFunc );
	}
	tOut.SendDword ( q.m_eExpandKeywords );

	tOut.SendInt ( q.m_dIndexHints.GetLength() );
	for ( const auto & i : q.m_dIndexHints )
	{
		tOut.SendDword ( i.m_eHint );
		tOut.SendString ( i.m_sIndex.cstr() );
	}
}


void SearchRequestBuilder_c::BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const
{
	auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_SEARCH, VER_COMMAND_SEARCH ); // API header

	tOut.SendInt ( VER_COMMAND_SEARCH_MASTER );
	tOut.SendInt ( m_iEnd-m_iStart+1 );
	for ( int i=m_iStart; i<=m_iEnd; ++i )
		SendQuery ( tAgent.m_tDesc.m_sIndexes.cstr (), tOut, m_dQueries[i], tAgent.m_iWeight, tAgent.m_iMyQueryTimeoutMs );
}


struct cSearchResult : public iQueryResult
{
	CSphVector<AggrResult_t>	m_dResults;

	void Reset () final
	{
		m_dResults.Reset();
	}

	bool HasWarnings () const final
	{
		return m_dResults.FindFirst ( [] ( const AggrResult_t &dRes ) { return !dRes.m_sWarning.IsEmpty (); } );
	}
};


/////////////////////////////////////////////////////////////////////////////

void SearchReplyParser_c::ParseMatch ( CSphMatch & tMatch, MemInputBuffer_c & tReq, const CSphSchema & tSchema, bool bAgent64 ) const
{
	tMatch.Reset ( tSchema.GetRowSize() );

	// WAS: docids
	if ( bAgent64 )
		tReq.GetUint64();
	else
		tReq.GetDword();

	tMatch.m_iWeight = tReq.GetInt ();
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);

		assert ( sphPlainAttrToPtrAttr(tAttr.m_eAttrType)==tAttr.m_eAttrType );

		switch ( tAttr.m_eAttrType )
		{
		case SPH_ATTR_UINT32SET_PTR:
		case SPH_ATTR_INT64SET_PTR:
			{
				int iValues = tReq.GetDword ();
				BYTE * pData = nullptr;
				BYTE * pPacked = sphPackPtrAttr ( iValues*sizeof(DWORD), &pData );
				tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t)pPacked );
				DWORD * pMVA = (DWORD *)pData;
				if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET_PTR )
				{
					while ( iValues-- )
						sphUnalignedWrite ( pMVA++, tReq.GetDword() );
				} else
				{
					assert ( ( iValues%2 )==0 );
					for ( ; iValues; iValues -= 2 )
					{
						uint64_t uMva = tReq.GetUint64();
						sphUnalignedWrite ( pMVA, uMva );
						pMVA += 2;
					}
				}
			}
			break;

		case SPH_ATTR_STRINGPTR:
		case SPH_ATTR_JSON_PTR:
		case SPH_ATTR_FACTORS:
		case SPH_ATTR_FACTORS_JSON:
			{
				int iLen = tReq.GetDword();
				BYTE * pData = nullptr;
				tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t)sphPackPtrAttr ( iLen, &pData ) );
				if ( iLen )
					tReq.GetBytes ( pData, iLen );
			}
			break;

		case SPH_ATTR_JSON_FIELD_PTR:
			{
				// FIXME: no reason for json_field to be any different from other *_PTR attributes
				ESphJsonType eJson = (ESphJsonType)tReq.GetByte();
				if ( eJson==JSON_EOF )
					tMatch.SetAttr ( tAttr.m_tLocator, 0 );
				else
				{
					int iLen = tReq.GetDword();
					BYTE * pData = nullptr;
					tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t)sphPackPtrAttr ( iLen+1, &pData ) );
					*pData++ = (BYTE)eJson;
					tReq.GetBytes ( pData, iLen );
				}
			}
			break;

		case SPH_ATTR_FLOAT:
			tMatch.SetAttr ( tAttr.m_tLocator, sphF2DW ( tReq.GetFloat() ) );
			break;

		case SPH_ATTR_BIGINT:
			tMatch.SetAttr ( tAttr.m_tLocator, tReq.GetUint64() );
			break;

		default:
			tMatch.SetAttr ( tAttr.m_tLocator, tReq.GetDword() );
			break;
		}
	}
}


void SearchReplyParser_c::ParseSchema ( CSphQueryResult & tRes, MemInputBuffer_c & tReq ) const
{
	CSphSchema & tSchema = tRes.m_tSchema;
	tSchema.Reset ();

	int nFields = tReq.GetInt(); // FIXME! add a sanity check
	for ( int j = 0; j < nFields; j++ )
		tSchema.AddField ( tReq.GetString().cstr() );

	int iNumAttrs = tReq.GetInt(); // FIXME! add a sanity check
	for ( int j=0; j<iNumAttrs; j++ )
	{
		CSphColumnInfo tCol;
		tCol.m_sName = tReq.GetString ();
		tCol.m_eAttrType = (ESphAttr) tReq.GetDword (); // FIXME! add a sanity check

		// we always work with plain attrs (not *_PTR) when working with agents
		tCol.m_eAttrType = sphPlainAttrToPtrAttr ( tCol.m_eAttrType );
		if ( tCol.m_eAttrType==SPH_ATTR_STORED_FIELD )
		{
			tCol.m_eAttrType = SPH_ATTR_STRINGPTR;
			tCol.m_uFieldFlags = CSphColumnInfo::FIELD_STORED;
		}
		tSchema.AddAttr ( tCol, true ); // all attributes received from agents are dynamic
	}
}


bool SearchReplyParser_c::ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const
{
	int iResults = m_iEnd-m_iStart+1;
	assert ( iResults>0 );

	auto pResult = ( cSearchResult * ) tAgent.m_pResult.Ptr ();
	if ( !pResult )
	{
		pResult = new cSearchResult;
		tAgent.m_pResult = pResult;
	}

	auto &dResults = pResult->m_dResults;

	dResults.Resize ( iResults );
	for ( auto & tRes : dResults )
		tRes.m_iSuccesses = 0;

	for ( auto & tRes : dResults )
	{
		tRes.m_sError = "";
		tRes.m_sWarning = "";

		// get status and message
		auto eStatus = ( SearchdStatus_e ) tReq.GetDword ();
		switch ( eStatus )
		{
			case SEARCHD_ERROR:		tRes.m_sError = tReq.GetString (); continue;
			case SEARCHD_RETRY:		tRes.m_sError = tReq.GetString (); break;
			case SEARCHD_WARNING:	tRes.m_sWarning = tReq.GetString (); break;
			default:				tAgent.m_sFailure.SetSprintf ( "internal error: unknown status %d, message %s", eStatus, tReq.GetString ().cstr() );
			case SEARCHD_OK: break;
		}

		ParseSchema ( tRes, tReq );

		// get matches
		int iMatches = tReq.GetInt ();
		if ( iMatches<0 )
		{
			tAgent.m_sFailure.SetSprintf ( "invalid match count received (count=%d)", iMatches );
			return false;
		}

		bool bAgent64 = !!tReq.GetInt();
		if ( !bAgent64 )
		{
			tAgent.m_sFailure.SetSprintf ( "agent has 32-bit docids; no longer supported" );
			return false;
		}

		assert ( !tRes.m_dMatches.GetLength() );
		if ( iMatches )
		{
			tRes.m_dMatches.Resize ( iMatches );
			for ( auto & tMatch : tRes.m_dMatches )
				ParseMatch ( tMatch, tReq, tRes.m_tSchema, bAgent64 );
		}

		// read totals (retrieved count, total count, query time, word count)
		int iRetrieved = tReq.GetInt ();
		tRes.m_iTotalMatches = (unsigned int)tReq.GetInt ();
		tRes.m_iQueryTime = tReq.GetInt ();

		// agents always send IO/CPU stats to master
		BYTE uStatMask = tReq.GetByte();
		if ( uStatMask & 1 )
		{
			tRes.m_tIOStats.m_iReadTime = tReq.GetUint64();
			tRes.m_tIOStats.m_iReadOps = tReq.GetDword();
			tRes.m_tIOStats.m_iReadBytes = tReq.GetUint64();
			tRes.m_tIOStats.m_iWriteTime = tReq.GetUint64();
			tRes.m_tIOStats.m_iWriteOps = tReq.GetDword();
			tRes.m_tIOStats.m_iWriteBytes = tReq.GetUint64();
		}

		if ( uStatMask & 2 )
			tRes.m_iCpuTime = tReq.GetUint64();

		if ( uStatMask & 4 )
			tRes.m_iPredictedTime = tReq.GetUint64();

		tRes.m_iAgentFetchedDocs = tReq.GetDword();
		tRes.m_iAgentFetchedHits = tReq.GetDword();
		tRes.m_iAgentFetchedSkips = tReq.GetDword();

		const int iWordsCount = tReq.GetInt (); // FIXME! sanity check?
		if ( iRetrieved!=iMatches )
		{
			tAgent.m_sFailure.SetSprintf ( "expected %d retrieved documents, got %d", iMatches, iRetrieved );
			return false;
		}

		// read per-word stats
		for ( int i=0; i<iWordsCount; i++ )
		{
			const CSphString sWord = tReq.GetString ();
			const int64_t iDocs = (unsigned int)tReq.GetInt ();
			const int64_t iHits = (unsigned int)tReq.GetInt ();
			tReq.GetByte(); // statistics have no expanded terms for now

			tRes.AddStat ( sWord, iDocs, iHits );
		}

		// mark this result as ok
		tRes.m_iSuccesses = 1;
	}

	// all seems OK (and buffer length checks are performed by caller)
	return true;
}

/////////////////////////////////////////////////////////////////////////////

// returns true if incoming schema (src) is equal to existing (dst); false otherwise
bool MinimizeSchema ( CSphSchema & tDst, const ISphSchema & tSrc )
{
	// if dst is empty, result is also empty
	if ( tDst.GetAttrsCount()==0 )
		return tSrc.GetAttrsCount()==0;

	// check for equality, and remove all dst attributes that are not present in src
	CSphVector<CSphColumnInfo> dDst;
	for ( int i=0; i<tDst.GetAttrsCount(); i++ )
		dDst.Add ( tDst.GetAttr(i) );

	bool bEqual = ( tDst.GetAttrsCount()==tSrc.GetAttrsCount() );
	ARRAY_FOREACH ( i, dDst )
	{
		int iSrcIdx = tSrc.GetAttrIndex ( dDst[i].m_sName.cstr() );

		// check for index mismatch
		if ( iSrcIdx!=i )
			bEqual = false;

		// check for type/size mismatch (and fixup if needed)
		if ( iSrcIdx>=0 )
		{
			const CSphColumnInfo & tSrcAttr = tSrc.GetAttr ( iSrcIdx );

			// should seamlessly convert ( bool > float ) | ( bool > int > bigint )
			ESphAttr eDst = dDst[i].m_eAttrType;
			ESphAttr eSrc = tSrcAttr.m_eAttrType;
			bool bSame = ( eDst==eSrc )
				|| ( ( eDst==SPH_ATTR_FLOAT && eSrc==SPH_ATTR_BOOL ) || ( eDst==SPH_ATTR_BOOL && eSrc==SPH_ATTR_FLOAT ) )
				|| ( ( eDst==SPH_ATTR_BOOL || eDst==SPH_ATTR_INTEGER || eDst==SPH_ATTR_BIGINT )
					&& ( eSrc==SPH_ATTR_BOOL || eSrc==SPH_ATTR_INTEGER || eSrc==SPH_ATTR_BIGINT ) );

			int iDstBitCount = dDst[i].m_tLocator.m_iBitCount;
			int iSrcBitCount = tSrcAttr.m_tLocator.m_iBitCount;

			if ( !bSame )
			{
				// different types? remove the attr
				iSrcIdx = -1;
				bEqual = false;

			} else if ( iDstBitCount!=iSrcBitCount )
			{
				// different bit sizes? choose the max one
				dDst[i].m_tLocator.m_iBitCount = Max ( iDstBitCount, iSrcBitCount );
				bEqual = false;
				if ( iDstBitCount<iSrcBitCount )
					dDst[i].m_eAttrType = tSrcAttr.m_eAttrType;
			}

			if ( tSrcAttr.m_tLocator.m_iBitOffset!=dDst[i].m_tLocator.m_iBitOffset )
			{
				// different offsets? have to force target dynamic then, since we can't use one locator for all matches
				bEqual = false;
			}

			if ( tSrcAttr.m_tLocator.m_bDynamic!=dDst[i].m_tLocator.m_bDynamic )
			{
				// different location? have to force target dynamic then
				bEqual = false;
			}
		}

		// check for presence
		if ( iSrcIdx<0 )
		{
			dDst.Remove ( i );
			i--;
		}
	}

	if ( !bEqual )
	{
		CSphVector<CSphColumnInfo> dFields { tDst.GetFieldsCount() };
		for ( int i = 0; i < tDst.GetFieldsCount(); i++ )
			dFields[i] = tDst.GetField(i);

		tDst.Reset();

		for ( auto& dAttr : dDst )
			tDst.AddAttr ( dAttr, true );

		for ( auto& dField: dFields )
			tDst.AddField ( dField );

	} else
		tDst.SwapAttrs ( dDst );

	return bEqual;
}

static void CheckQuery ( const CSphQuery & tQuery, CSphString & sError )
{
	#define LOC_ERROR(_msg) { sError.SetSprintf ( _msg ); return; }
	#define LOC_ERROR1(_msg,_arg1) { sError.SetSprintf ( _msg, _arg1 ); return; }
	#define LOC_ERROR2(_msg,_arg1,_arg2) { sError.SetSprintf ( _msg, _arg1, _arg2 ); return; }

	sError = NULL;

	if ( (int)tQuery.m_eMode<0 || tQuery.m_eMode>SPH_MATCH_TOTAL )
		LOC_ERROR1 ( "invalid match mode %d", tQuery.m_eMode );

	if ( (int)tQuery.m_eRanker<0 || tQuery.m_eRanker>SPH_RANK_TOTAL )
		LOC_ERROR1 ( "invalid ranking mode %d", tQuery.m_eRanker );

	if ( tQuery.m_iMaxMatches<1 )
		LOC_ERROR ( "max_matches can not be less than one" );

	if ( tQuery.m_iOffset<0 || tQuery.m_iOffset>=tQuery.m_iMaxMatches )
		LOC_ERROR2 ( "offset out of bounds (offset=%d, max_matches=%d)",
			tQuery.m_iOffset, tQuery.m_iMaxMatches );

	if ( tQuery.m_iLimit<0 )
		LOC_ERROR1 ( "limit out of bounds (limit=%d)", tQuery.m_iLimit );

	if ( tQuery.m_iCutoff<0 )
		LOC_ERROR1 ( "cutoff out of bounds (cutoff=%d)", tQuery.m_iCutoff );

	if ( ( tQuery.m_iRetryCount!=-1 )
		&& ( tQuery.m_iRetryCount>MAX_RETRY_COUNT ) )
		LOC_ERROR1 ( "retry count out of bounds (count=%d)", tQuery.m_iRetryCount );

	if ( ( tQuery.m_iRetryDelay!=-1 )
		&& ( tQuery.m_iRetryDelay>MAX_RETRY_DELAY ) )
			LOC_ERROR1 ( "retry delay out of bounds (delay=%d)", tQuery.m_iRetryDelay );

	if ( tQuery.m_iOffset>0 && tQuery.m_bHasOuter )
		LOC_ERROR1 ( "inner offset must be 0 when using outer order by (offset=%d)", tQuery.m_iOffset );

	#undef LOC_ERROR
	#undef LOC_ERROR1
	#undef LOC_ERROR2
}


void PrepareQueryEmulation ( CSphQuery * pQuery )
{
	if ( pQuery->m_eMode==SPH_MATCH_BOOLEAN )
		pQuery->m_eRanker = SPH_RANK_NONE;

	if ( pQuery->m_eMode==SPH_MATCH_FULLSCAN )
		pQuery->m_sQuery = "";

	if ( pQuery->m_eMode!=SPH_MATCH_ALL && pQuery->m_eMode!=SPH_MATCH_ANY && pQuery->m_eMode!=SPH_MATCH_PHRASE )
		return;

	const char * szQuery = pQuery->m_sRawQuery.cstr ();
	int iQueryLen = szQuery ? (int) strlen(szQuery) : 0;

	pQuery->m_sQuery.Reserve ( iQueryLen*2+8 );
	char * szRes = (char*) pQuery->m_sQuery.cstr ();
	char c;

	if ( pQuery->m_eMode==SPH_MATCH_ANY || pQuery->m_eMode==SPH_MATCH_PHRASE )
		*szRes++ = '\"';

	if ( iQueryLen )
	{
		while ( ( c = *szQuery++ )!=0 )
		{
			// must be in sync with EscapeString (php api)
			const char sMagics[] = "<\\()|-!@~\"&/^$=";
			for ( const char * s = sMagics; *s; s++ )
				if ( c==*s )
				{
					*szRes++ = '\\';
					break;
				}
			*szRes++ = c;
		}
	}

	switch ( pQuery->m_eMode )
	{
		case SPH_MATCH_ALL:		pQuery->m_eRanker = SPH_RANK_PROXIMITY; *szRes = '\0'; break;
		case SPH_MATCH_ANY:		pQuery->m_eRanker = SPH_RANK_MATCHANY; strncpy ( szRes, "\"/1", 8 ); break;
		case SPH_MATCH_PHRASE:	pQuery->m_eRanker = SPH_RANK_PROXIMITY; *szRes++ = '\"'; *szRes = '\0'; break;
		default:				return;
	}
}


static void FixupQuerySettings ( CSphQuery & tQuery )
{
	// sort filters
	for ( auto & i : tQuery.m_dFilters )
		i.m_dValues.Sort();

	if ( !tQuery.m_bHasOuter )
	{
		tQuery.m_sOuterOrderBy = "";
		tQuery.m_iOuterOffset = 0;
		tQuery.m_iOuterLimit = 0;
	}
}


static bool ParseSearchFilter ( CSphFilterSettings & tFilter, InputBuffer_c & tReq, ISphOutputBuffer & tOut, int iMasterVer )
{
	tFilter.m_sAttrName = tReq.GetString ();
	sphColumnToLowercase ( const_cast<char *>( tFilter.m_sAttrName.cstr() ) );

	tFilter.m_eType = (ESphFilter) tReq.GetDword ();
	switch ( tFilter.m_eType )
	{
	case SPH_FILTER_RANGE:
		tFilter.m_iMinValue = tReq.GetUint64();
		tFilter.m_iMaxValue = tReq.GetUint64();
		break;

	case SPH_FILTER_FLOATRANGE:
		tFilter.m_fMinValue = tReq.GetFloat ();
		tFilter.m_fMaxValue = tReq.GetFloat ();
		break;

	case SPH_FILTER_VALUES:
		{
			int iGot = 0;
			bool bRes = tReq.GetQwords ( tFilter.m_dValues, iGot, g_iMaxFilterValues );
			if ( !bRes )
			{
				SendErrorReply ( tOut, "invalid attribute '%s' set length %d (should be in 0..%d range)", tFilter.m_sAttrName.cstr(), iGot, g_iMaxFilterValues );
				return false;
			}
		}
		break;

	case SPH_FILTER_STRING:
		tFilter.m_dStrings.Add ( tReq.GetString() );
		break;

	case SPH_FILTER_NULL:
		tFilter.m_bIsNull = tReq.GetByte()!=0;
		break;

	case SPH_FILTER_USERVAR:
		tFilter.m_dStrings.Add ( tReq.GetString() );
		break;

	case SPH_FILTER_STRING_LIST:
		{
			int iCount = tReq.GetDword();
			if ( iCount<0 || iCount>g_iMaxFilterValues )
			{
				SendErrorReply ( tOut, "invalid attribute '%s' set length %d (should be in 0..%d range)", tFilter.m_sAttrName.cstr(), iCount, g_iMaxFilterValues );
				return false;
			}
			tFilter.m_dStrings.Resize ( iCount );
			ARRAY_FOREACH ( iString, tFilter.m_dStrings )
				tFilter.m_dStrings[iString] = tReq.GetString();
		}
		break;
	case SPH_FILTER_EXPRESSION: // need only name and type
		break;

	default:
		SendErrorReply ( tOut, "unknown filter type (type-id=%d)", tFilter.m_eType );
		return false;
	}

	if ( tFilter.m_sAttrName=="@id" )
	{
		// request coming from old master, need to fix attribute name
		tFilter.m_sAttrName = "id";

		// and clamp values from uint64_t to int64_t
		if ( (uint64_t)tFilter.m_iMinValue > (uint64_t)LLONG_MAX )
			tFilter.m_iMinValue = LLONG_MAX;

		if ( (uint64_t)tFilter.m_iMaxValue > (uint64_t)LLONG_MAX )
			tFilter.m_iMaxValue = LLONG_MAX;
	}

	tFilter.m_bExclude = !!tReq.GetDword ();

	if ( iMasterVer>=15 )
	{
		tFilter.m_bHasEqualMin = !!tReq.GetDword();
		tFilter.m_bHasEqualMax = !!tReq.GetDword();
	} else if ( iMasterVer>=5 )
		tFilter.m_bHasEqualMin = tFilter.m_bHasEqualMax = !!tReq.GetDword();

	if ( iMasterVer>=15 )
	{
		tFilter.m_bOpenLeft = !!tReq.GetDword();
		tFilter.m_bOpenRight = !!tReq.GetDword();
	}

	tFilter.m_eMvaFunc = SPH_MVAFUNC_ANY;
	if ( iMasterVer>=13 )
		tFilter.m_eMvaFunc = (ESphMvaFunc)tReq.GetDword();

	return true;
}


bool ParseSearchQuery ( InputBuffer_c & tReq, ISphOutputBuffer & tOut, CSphQuery & tQuery, WORD uVer, WORD uMasterVer )
{
	// daemon-level defaults
	tQuery.m_iRetryCount = -1;
	tQuery.m_iRetryDelay = -1;
	tQuery.m_iAgentQueryTimeoutMs = g_iAgentQueryTimeoutMs;

	// v.1.27+ flags come first
	DWORD uFlags = 0;
	if ( uVer>=0x11B )
		uFlags = tReq.GetDword();

	// v.1.0. mode, limits, weights, ID/TS ranges
	tQuery.m_iOffset = tReq.GetInt ();
	tQuery.m_iLimit = tReq.GetInt ();
	tQuery.m_eMode = (ESphMatchMode) tReq.GetInt ();
	tQuery.m_eRanker = (ESphRankMode) tReq.GetInt ();
	if ( tQuery.m_eRanker==SPH_RANK_EXPR || tQuery.m_eRanker==SPH_RANK_EXPORT )
		tQuery.m_sRankerExpr = tReq.GetString();

	tQuery.m_eSort = (ESphSortOrder) tReq.GetInt ();
	tQuery.m_sSortBy = tReq.GetString ();
	sphColumnToLowercase ( const_cast<char *>( tQuery.m_sSortBy.cstr() ) );
	tQuery.m_sRawQuery = tReq.GetString ();
	{
		int iGot = 0;
		if ( !tReq.GetDwords ( tQuery.m_dWeights, iGot, SPH_MAX_FIELDS ) )
		{
			SendErrorReply ( tOut, "invalid weight count %d (should be in 0..%d range)", iGot, SPH_MAX_FIELDS );
			return false;
		}
	}

	tQuery.m_sIndexes = tReq.GetString ();

	// legacy id range filter
	bool bIdrange64 = tReq.GetInt()!=0;
	DocID_t tMinDocID = bIdrange64 ? (DocID_t)tReq.GetUint64 () : tReq.GetDword ();
	DocID_t tMaxDocID = bIdrange64 ? (DocID_t)tReq.GetUint64 () : tReq.GetDword ();

	if ( tMaxDocID==0 || (uint64_t)tMaxDocID==UINT64_MAX )
		tMaxDocID = INT64_MAX;

	int iAttrFilters = tReq.GetInt ();
	if ( iAttrFilters>g_iMaxFilters )
	{
		SendErrorReply ( tOut, "too many attribute filters (req=%d, max=%d)", iAttrFilters, g_iMaxFilters );
		return false;
	}

	tQuery.m_dFilters.Resize ( iAttrFilters );
	for ( auto & i : tQuery.m_dFilters )
		if ( !ParseSearchFilter ( i, tReq, tOut, uMasterVer ) )
			return false;

	// now add id range filter
	if ( tMinDocID!=0 || tMaxDocID!=INT64_MAX )
	{
		CSphFilterSettings & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_sAttrName = sphGetDocidName();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_iMinValue = tMinDocID;
		tFilter.m_iMaxValue = tMaxDocID;
	}

	tQuery.m_eGroupFunc = (ESphGroupBy) tReq.GetDword ();
	tQuery.m_sGroupBy = tReq.GetString ();
	sphColumnToLowercase ( const_cast<char *>( tQuery.m_sGroupBy.cstr() ) );

	tQuery.m_iMaxMatches = tReq.GetInt ();
	tQuery.m_sGroupSortBy = tReq.GetString ();
	tQuery.m_iCutoff = tReq.GetInt();
	tQuery.m_iRetryCount = tReq.GetInt ();
	tQuery.m_iRetryDelay = tReq.GetInt ();
	tQuery.m_sGroupDistinct = tReq.GetString ();
	sphColumnToLowercase ( const_cast<char *>( tQuery.m_sGroupDistinct.cstr() ) );

	tQuery.m_bGeoAnchor = ( tReq.GetInt()!=0 );
	if ( tQuery.m_bGeoAnchor )
	{
		tQuery.m_sGeoLatAttr = tReq.GetString ();
		tQuery.m_sGeoLongAttr = tReq.GetString ();
		tQuery.m_fGeoLatitude = tReq.GetFloat ();
		tQuery.m_fGeoLongitude = tReq.GetFloat ();
	}

	tQuery.m_dIndexWeights.Resize ( tReq.GetInt() ); // FIXME! add sanity check
	for ( auto& dIndexWeight : tQuery.m_dIndexWeights )
		tReq >> dIndexWeight;

	tQuery.m_uMaxQueryMsec = tReq.GetDword ();

	tQuery.m_dFieldWeights.Resize ( tReq.GetInt() ); // FIXME! add sanity check
	for ( auto & dFieldWeight : tQuery.m_dFieldWeights )
		tReq >> dFieldWeight;

	tQuery.m_sComment = tReq.GetString ();

	int nOverrides = tReq.GetInt();
	if ( nOverrides>0 )
	{
		SendErrorReply ( tOut, "overrides are now deprecated" );
		return false;
	}

	tQuery.m_sSelect = tReq.GetString ();
	tQuery.m_bAgent = ( uMasterVer>0 );
	if ( tQuery.m_sSelect.Begins ( "*,*" ) ) // this is the legacy mark of agent for debug purpose
	{
		tQuery.m_bAgent = true;
		int iSelectLen = tQuery.m_sSelect.Length();
		tQuery.m_sSelect = ( iSelectLen>4 ? tQuery.m_sSelect.SubString ( 4, iSelectLen-4 ) : "*" );
	}
	// fixup select list
	if ( tQuery.m_sSelect.IsEmpty () )
		tQuery.m_sSelect = "*";

	// master sends items to agents since master.version=15 
	CSphString sError;
	if ( uMasterVer<15 && !ParseSelectList ( sError, tQuery ) )
	{
		// we want to see a parse error in query_log_format=sphinxql mode too
		if ( g_eLogFormat==LOG_FORMAT_SPHINXQL && g_iQueryLogFile>=0 )
		{
			StringBuilder_c tBuf;
			char sTimeBuf [ SPH_TIME_PID_MAX_SIZE ];
			sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );
			tBuf << "/""* " << sTimeBuf << "*""/ " << tQuery.m_sSelect << " # error=" << sError << "\n";
			sphSeek ( g_iQueryLogFile, 0, SEEK_END );
			sphWrite ( g_iQueryLogFile, tBuf.cstr(), tBuf.GetLength() );
		}

		SendErrorReply ( tOut, "select: %s", sError.cstr () );
		return false;
	}

	// v.1.27
	if ( uVer>=0x11B )
	{
		// parse simple flags
		tQuery.m_bReverseScan = !!( uFlags & QFLAG_REVERSE_SCAN );
		tQuery.m_bSortKbuffer = !!( uFlags & QFLAG_SORT_KBUFFER );
		tQuery.m_bSimplify = !!( uFlags & QFLAG_SIMPLIFY );
		tQuery.m_bPlainIDF = !!( uFlags & QFLAG_PLAIN_IDF );
		tQuery.m_bGlobalIDF = !!( uFlags & QFLAG_GLOBAL_IDF );
		tQuery.m_bLocalDF = !!( uFlags & QFLAG_LOCAL_DF );
		tQuery.m_bLowPriority = !!( uFlags & QFLAG_LOW_PRIORITY );
		tQuery.m_bFacet = !!( uFlags & QFLAG_FACET );
		tQuery.m_bFacetHead = !!( uFlags & QFLAG_FACET_HEAD );
		tQuery.m_eQueryType = (uFlags & QFLAG_JSON_QUERY) ? QUERY_JSON : QUERY_API;

		if ( uMasterVer>0 || uVer==0x11E )
			tQuery.m_bNormalizedTFIDF = !!( uFlags & QFLAG_NORMALIZED_TF );

		// fetch optional stuff
		if ( uFlags & QFLAG_MAX_PREDICTED_TIME )
			tQuery.m_iMaxPredictedMsec = tReq.GetInt();
	}

	// v.1.29
	if ( uVer>=0x11D )
	{
		tQuery.m_sOuterOrderBy = tReq.GetString();
		tQuery.m_iOuterOffset = tReq.GetDword();
		tQuery.m_iOuterLimit = tReq.GetDword();
		tQuery.m_bHasOuter = ( tReq.GetInt()!=0 );
	}

	// extension v.1
	tQuery.m_eCollation = g_eCollation;
	if ( uMasterVer>=1 )
		tQuery.m_eCollation = (ESphCollation)tReq.GetDword();

	// extension v.2
	if ( uMasterVer>=2 )
	{
		tQuery.m_sOuterOrderBy = tReq.GetString();
		if ( tQuery.m_bHasOuter )
			tQuery.m_iOuterLimit = tReq.GetInt();
	}

	if ( uMasterVer>=6 )
		tQuery.m_iGroupbyLimit = tReq.GetInt();

	if ( uMasterVer>=14 )
	{
		tQuery.m_sUDRanker = tReq.GetString();
		tQuery.m_sUDRankerOpts = tReq.GetString();
	}

	if ( uMasterVer>=14 || uVer>=0x120 )
	{
		tQuery.m_sQueryTokenFilterLib = tReq.GetString();
		tQuery.m_sQueryTokenFilterName = tReq.GetString();
		tQuery.m_sQueryTokenFilterOpts = tReq.GetString();
	}

	if ( uVer>=0x121 )
	{
		tQuery.m_dFilterTree.Resize ( tReq.GetInt() );
		for ( FilterTreeItem_t &tItem : tQuery.m_dFilterTree )
		{
			tItem.m_iLeft = tReq.GetInt();
			tItem.m_iRight = tReq.GetInt();
			tItem.m_iFilterItem = tReq.GetInt();
			tItem.m_bOr = ( tReq.GetInt()!=0 );
		}
	}

	if ( uMasterVer>=15 )
	{
		tQuery.m_dItems.Resize ( tReq.GetInt() );
		for ( CSphQueryItem &tItem : tQuery.m_dItems )
		{
			tItem.m_sAlias = tReq.GetString();
			tItem.m_sExpr = tReq.GetString();
			tItem.m_eAggrFunc = (ESphAggrFunc)tReq.GetDword();
		}
		tQuery.m_dRefItems.Resize ( tReq.GetInt() );
		for ( CSphQueryItem &tItem : tQuery.m_dRefItems )
		{
			tItem.m_sAlias = tReq.GetString();
			tItem.m_sExpr = tReq.GetString();
			tItem.m_eAggrFunc = (ESphAggrFunc)tReq.GetDword();
		}
	}

	if ( uMasterVer>=16 )
		tQuery.m_eExpandKeywords = (QueryOption_e)tReq.GetDword();

	if ( uMasterVer>=17 )
	{
		tQuery.m_dIndexHints.Resize ( tReq.GetDword() );
		for ( auto & i : tQuery.m_dIndexHints )
		{
			i.m_eHint = (IndexHint_e)tReq.GetDword();
			i.m_sIndex = tReq.GetString();
		}
	}

	/////////////////////
	// additional checks
	/////////////////////

	// queries coming from API may not request docids
	// but we still need docids when sending result sets
	if ( tQuery.m_dItems.GetLength() )
	{
		bool bHaveDocID = false;
		for ( const auto & i : tQuery.m_dItems )
			bHaveDocID |= i.m_sExpr==sphGetDocidName() || i.m_sExpr=="*";

		if ( !bHaveDocID )
		{
			CSphQueryItem tId;
			tId.m_sExpr = sphGetDocidName();
			tQuery.m_dItems.Insert ( 0, tId );
		}
	}

	if ( tReq.GetError() )
	{
		SendErrorReply ( tOut, "invalid or truncated request" );
		return false;
	}

	CheckQuery ( tQuery, sError );
	if ( !sError.IsEmpty() )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return false;
	}

	// now prepare it for the engine
	tQuery.m_sQuery = tQuery.m_sRawQuery;

	if ( tQuery.m_eQueryType!=QUERY_JSON )
		PrepareQueryEmulation ( &tQuery );

	FixupQuerySettings ( tQuery );

	// all ok
	return true;
}

//////////////////////////////////////////////////////////////////////////

struct EscapeQuotation_t : public BaseQuotation_t
{
	inline static bool IsEscapeChar ( char c )
	{
		return ( c=='\\' || c=='\'' );
	}
};


using QuotationEscapedBuilder = EscapedStringBuilder_T<EscapeQuotation_t>;


void LogQueryPlain ( const CSphQuery & tQuery, const CSphQueryResult & tRes )
{
	assert ( g_eLogFormat==LOG_FORMAT_PLAIN );
	if ( ( !g_bQuerySyslog && g_iQueryLogFile<0 ) || !tRes.m_sError.IsEmpty() )
		return;

	QuotationEscapedBuilder tBuf;

	// [time]
#if USE_SYSLOG
	if ( !g_bQuerySyslog )
	{
#endif

		char sTimeBuf[SPH_TIME_PID_MAX_SIZE];
		sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );
		tBuf.Appendf ( "[%s]", sTimeBuf );

#if USE_SYSLOG
	} else
		tBuf += "[query]";
#endif

	// querytime sec
	int iQueryTime = Max ( tRes.m_iQueryTime, 0 );
	int iRealTime = Max ( tRes.m_iRealQueryTime, 0 );
	tBuf.Appendf ( " %d.%03d sec", iRealTime/1000, iRealTime%1000 );
	tBuf.Appendf ( " %d.%03d sec", iQueryTime/1000, iQueryTime%1000 );

	// optional multi-query multiplier
	if ( tRes.m_iMultiplier>1 )
		tBuf.Appendf ( " x%d", tRes.m_iMultiplier );

	// [matchmode/numfilters/sortmode matches (offset,limit)
	static const char * sModes [ SPH_MATCH_TOTAL ] = { "all", "any", "phr", "bool", "ext", "scan", "ext2" };
	static const char * sSort [ SPH_SORT_TOTAL ] = { "rel", "attr-", "attr+", "tsegs", "ext", "expr" };
	tBuf.Appendf ( " [%s/%d/%s " INT64_FMT " (%d,%d)",
		sModes [ tQuery.m_eMode ], tQuery.m_dFilters.GetLength(), sSort [ tQuery.m_eSort ],
		tRes.m_iTotalMatches, tQuery.m_iOffset, tQuery.m_iLimit );

	// optional groupby info
	if ( !tQuery.m_sGroupBy.IsEmpty() )
		tBuf.Appendf ( " @%s", tQuery.m_sGroupBy.cstr() );

	// ] [indexes]
	tBuf.Appendf ( "] [%s]", tQuery.m_sIndexes.cstr() );

	// optional performance counters
	if ( g_bIOStats || g_bCpuStats )
	{
		const CSphIOStats & IOStats = tRes.m_tIOStats;

		tBuf += " [";

		if ( g_bIOStats )
			tBuf.Appendf ( "ios=%d kb=%d.%d ioms=%d.%d",
				IOStats.m_iReadOps, (int)( IOStats.m_iReadBytes/1024 ), (int)( IOStats.m_iReadBytes%1024 )*10/1024,
				(int)( IOStats.m_iReadTime/1000 ), (int)( IOStats.m_iReadTime%1000 )/100 );

		if ( g_bIOStats && g_bCpuStats )
			tBuf += " ";

		if ( g_bCpuStats )
			tBuf.Appendf ( "cpums=%d.%d", (int)( tRes.m_iCpuTime/1000 ), (int)( tRes.m_iCpuTime%1000 )/100 );

		tBuf += "]";
	}

	// optional query comment
	if ( !tQuery.m_sComment.IsEmpty() )
		tBuf.Appendf ( " [%s]", tQuery.m_sComment.cstr() );

	// query
	// (m_sRawQuery is empty when using MySQL handler)
	const CSphString & sQuery = tQuery.m_sRawQuery.IsEmpty()
		? tQuery.m_sQuery
		: tQuery.m_sRawQuery;

	if ( !sQuery.IsEmpty() )
	{
		tBuf += " ";
		tBuf.FixupSpacesAndAppend ( sQuery.cstr() );
	}

#if USE_SYSLOG
	if ( !g_bQuerySyslog )
	{
#endif

	// line feed
	tBuf += "\n";

	sphSeek ( g_iQueryLogFile, 0, SEEK_END );
	sphWrite ( g_iQueryLogFile, tBuf.cstr(), tBuf.GetLength() );

#if USE_SYSLOG
	} else
	{
		syslog ( LOG_INFO, "%s", tBuf.cstr() );
	}
#endif
}

namespace {
CSphString RemoveBackQuotes ( const char * pSrc )
{
	CSphString sResult;
	if ( !pSrc )
		return sResult;

	size_t iLen = strlen ( pSrc );
	if ( !iLen )
		return sResult;

	auto szResult = new char[iLen+1];

	auto * sMax = pSrc+iLen;
	auto d = szResult;
	while ( pSrc<sMax )
	{
		auto sQuote = (const char *) memchr ( pSrc, '`', sMax-pSrc );
		if ( !sQuote )
			sQuote = sMax;
		auto iChunk = sQuote-pSrc;
		memmove ( d, pSrc, iChunk );
		d += iChunk;
		pSrc += iChunk+1; // +1 to skip the quote
	}
	*d = '\0';
	if ( !*szResult ) // never return allocated, but empty str. Prefer to return nullptr instead.
		SafeDeleteArray( szResult );
	sResult.Adopt ( &szResult );
	return sResult;
}
}

static void FormatOrderBy ( StringBuilder_c * pBuf, const char * sPrefix, ESphSortOrder eSort, const CSphString & sSort )
{
	assert ( pBuf );
	if ( eSort==SPH_SORT_EXTENDED && sSort=="@weight desc" )
		return;

	const char * sSubst = "@weight";
	if ( sSort!="@relevance" )
			sSubst = sSort.cstr();

	auto sUnquoted = RemoveBackQuotes ( sSubst );
	sSubst = sUnquoted.cstr();

	// for simplicity check that sPrefix is already prefixed/suffixed by spaces.
	assert ( sPrefix && sPrefix[0]==' ' && sPrefix[strlen ( sPrefix )-1]==' ' );
	*pBuf << sPrefix;
	switch ( eSort )
	{
	case SPH_SORT_ATTR_DESC:		*pBuf << sSubst << " DESC"; break;
	case SPH_SORT_ATTR_ASC:			*pBuf << sSubst << " ASC"; break;
	case SPH_SORT_TIME_SEGMENTS:	*pBuf << "TIME_SEGMENT(" << sSubst << ")"; break;
	case SPH_SORT_EXTENDED:			*pBuf << sSubst; break;
	case SPH_SORT_EXPR:				*pBuf << "BUILTIN_EXPR()"; break;
	case SPH_SORT_RELEVANCE:		*pBuf << "weight() desc"; if ( sSubst ) *pBuf << ", " << sSubst; break;
	default:						pBuf->Appendf ( "mode-%d", (int)eSort ); break;
	}
}

static const CSphQuery g_tDefaultQuery {};

static void FormatSphinxql ( const CSphQuery & q, int iCompactIN, QuotationEscapedBuilder & tBuf );
static void FormatList ( const CSphVector<CSphNamedInt> & dValues, StringBuilder_c & tBuf )
{
	ScopedComma_c tComma ( tBuf, ", " );
	for ( const auto& dValue : dValues )
		tBuf << dValue;
}

static void FormatOption ( const CSphQuery & tQuery, StringBuilder_c & tBuf )
{
	ScopedComma_c tOptionComma ( tBuf, ", ", " OPTION ");

	if ( tQuery.m_iMaxMatches!=DEFAULT_MAX_MATCHES )
		tBuf.Appendf ( "max_matches=%d", tQuery.m_iMaxMatches );

	if ( !tQuery.m_sComment.IsEmpty() )
		tBuf.Appendf ( "comment='%s'", tQuery.m_sComment.cstr() ); // FIXME! escape, replace newlines..

	if ( tQuery.m_eRanker!=SPH_RANK_DEFAULT )
	{
		const char * sRanker = sphGetRankerName ( tQuery.m_eRanker );
		if ( !sRanker )
			sRanker = sphGetRankerName ( SPH_RANK_DEFAULT );

		tBuf.Appendf ( "ranker=%s", sRanker );

		if ( tQuery.m_sRankerExpr.IsEmpty() )
			tBuf.Appendf ( "ranker=%s", sRanker );
		else
			tBuf.Appendf ( "ranker=%s(\'%s\')", sRanker, tQuery.m_sRankerExpr.scstr() );
	}

	if ( tQuery.m_iAgentQueryTimeoutMs!=g_iAgentQueryTimeoutMs )
		tBuf.Appendf ( "agent_query_timeout=%d", tQuery.m_iAgentQueryTimeoutMs );

	if ( tQuery.m_iCutoff!=g_tDefaultQuery.m_iCutoff )
		tBuf.Appendf ( "cutoff=%d", tQuery.m_iCutoff );

	if ( tQuery.m_dFieldWeights.GetLength() )
	{
		tBuf.StartBlock (nullptr,"field_weights=(",")");
		FormatList ( tQuery.m_dFieldWeights, tBuf );
		tBuf.FinishBlock ();
	}

	if ( tQuery.m_bGlobalIDF!=g_tDefaultQuery.m_bGlobalIDF )
		tBuf << "global_idf=1";

	if ( tQuery.m_bPlainIDF || !tQuery.m_bNormalizedTFIDF )
	{
		tBuf.StartBlock(",","idf='","'");
		tBuf << ( tQuery.m_bPlainIDF ? "plain" : "normalized" )
		<< ( tQuery.m_bNormalizedTFIDF ? "tfidf_normalized" : "tfidf_unnormalized" );
		tBuf.FinishBlock ();
	}

	if ( tQuery.m_bLocalDF!=g_tDefaultQuery.m_bLocalDF )
		tBuf << "local_df=1";

	if ( tQuery.m_dIndexWeights.GetLength() )
	{
		tBuf.StartBlock ( nullptr, "index_weights=(", ")" );
		FormatList ( tQuery.m_dIndexWeights, tBuf );
		tBuf.FinishBlock ();
	}

	if ( tQuery.m_uMaxQueryMsec!=g_tDefaultQuery.m_uMaxQueryMsec )
		tBuf.Appendf ( "max_query_time=%u", tQuery.m_uMaxQueryMsec );

	if ( tQuery.m_iMaxPredictedMsec!=g_tDefaultQuery.m_iMaxPredictedMsec )
		tBuf.Appendf ( "max_predicted_time=%d", tQuery.m_iMaxPredictedMsec );

	if ( tQuery.m_iRetryCount!=-1 )
		tBuf.Appendf ( "retry_count=%d", tQuery.m_iRetryCount );

	if ( tQuery.m_iRetryDelay!=-1 )
		tBuf.Appendf ( "retry_delay=%d", tQuery.m_iRetryDelay );

	if ( tQuery.m_iRandSeed!=g_tDefaultQuery.m_iRandSeed )
		tBuf.Appendf ( "rand_seed=" INT64_FMT, tQuery.m_iRandSeed );

	if ( !tQuery.m_sQueryTokenFilterLib.IsEmpty() )
	{
		if ( tQuery.m_sQueryTokenFilterOpts.IsEmpty() )
			tBuf.Appendf ( "token_filter = '%s:%s'", tQuery.m_sQueryTokenFilterLib.cstr(), tQuery.m_sQueryTokenFilterName.cstr() );
		else
			tBuf.Appendf ( "token_filter = '%s:%s:%s'", tQuery.m_sQueryTokenFilterLib.cstr(), tQuery.m_sQueryTokenFilterName.cstr(), tQuery.m_sQueryTokenFilterOpts.cstr() );
	}

	if ( tQuery.m_bIgnoreNonexistent )
		tBuf << "ignore_nonexistent_columns=1";

	if ( tQuery.m_bIgnoreNonexistentIndexes )
		tBuf << "ignore_nonexistent_indexes=1";

	if ( tQuery.m_bStrict )
		tBuf << "strict=1";

	if ( tQuery.m_eExpandKeywords!=QUERY_OPT_DEFAULT && tQuery.m_eExpandKeywords!=QUERY_OPT_MORPH_NONE )
		tBuf.Appendf ( "expand_keywords=%d", ( tQuery.m_eExpandKeywords==QUERY_OPT_ENABLED ? 1 : 0 ) );
	if ( tQuery.m_eExpandKeywords==QUERY_OPT_MORPH_NONE )
		tBuf.Appendf ( "morphology=none" );
}


static void AppendHint ( const char * szHint, const StrVec_t & dIndexes, StringBuilder_c & tBuf )
{
	if ( dIndexes.IsEmpty() )
		return;
	tBuf << " " << szHint;
	ScopedComma_c sIndex ( tBuf, ",", " INDEX (", ")" );
	for ( const auto & sIndex : dIndexes )
		tBuf << sIndex;
}


static void FormatIndexHints ( const CSphQuery & tQuery, StringBuilder_c & tBuf )
{
	ScopedComma_c sMatch ( tBuf, nullptr );
	StrVec_t dUse, dForce, dIgnore;
	for ( const auto & i : tQuery.m_dIndexHints )
	{
		switch ( i.m_eHint )
		{
		case INDEX_HINT_USE:
			dUse.Add(i.m_sIndex);
			break;
		case INDEX_HINT_FORCE:
			dForce.Add(i.m_sIndex);
			break;
		case INDEX_HINT_IGNORE:
			dIgnore.Add(i.m_sIndex);
			break;
		default:
			break;
		}
	}

	AppendHint ( "USE", dUse, tBuf );
	AppendHint ( "FORCE", dForce, tBuf );
	AppendHint ( "IGNORE", dIgnore, tBuf );
}


static void LogQuerySphinxql ( const CSphQuery & q, const CSphQueryResult & tRes, const CSphVector<int64_t> & dAgentTimes )
{
	assert ( g_eLogFormat==LOG_FORMAT_SPHINXQL );
	if ( g_iQueryLogFile<0 )
		return;

	QuotationEscapedBuilder tBuf;
	int iCompactIN = ( g_bLogCompactIn ? LOG_COMPACT_IN : 0 );

	// time, conn id, wall, found
	int iQueryTime = Max ( tRes.m_iQueryTime, 0 );
	int iRealTime = Max ( tRes.m_iRealQueryTime, 0 );

	char sTimeBuf[SPH_TIME_PID_MAX_SIZE];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	tBuf += R"(/* )";
	tBuf += sTimeBuf;

	if ( tRes.m_iMultiplier>1 )
		tBuf.Sprintf ( " conn %d real %0.3F wall %0.3F x%d found " INT64_FMT " *""/ ",
				 myinfo::ConnID(), iRealTime, iQueryTime, tRes.m_iMultiplier, tRes.m_iTotalMatches );
	else
		tBuf.Sprintf ( " conn %d real %0.3F wall %0.3F found " INT64_FMT " *""/ ",
				 myinfo::ConnID(), iRealTime, iQueryTime, tRes.m_iTotalMatches );

	///////////////////////////////////
	// format request as SELECT query
	///////////////////////////////////

	FormatSphinxql ( q, iCompactIN, tBuf );

	///////////////
	// query stats
	///////////////

	// next block ecnlosed in /* .. */, space-separated
	tBuf.StartBlock ( " ", R"( /*)", " */" );
	if ( !tRes.m_sError.IsEmpty() )
	{
		// all we have is an error
		tBuf.Appendf ( "error=%s", tRes.m_sError.cstr() );

	} else if ( g_bIOStats || g_bCpuStats || dAgentTimes.GetLength() || !tRes.m_sWarning.IsEmpty() )
	{
		// performance counters
		if ( g_bIOStats || g_bCpuStats )
		{
			const CSphIOStats & IOStats = tRes.m_tIOStats;

			if ( g_bIOStats )
				tBuf.Appendf ( "ios=%d kb=%d.%d ioms=%d.%d",
				IOStats.m_iReadOps, (int)( IOStats.m_iReadBytes/1024 ), (int)( IOStats.m_iReadBytes%1024 )*10/1024,
				(int)( IOStats.m_iReadTime/1000 ), (int)( IOStats.m_iReadTime%1000 )/100 );

			if ( g_bCpuStats )
				tBuf.Appendf ( "cpums=%d.%d", (int)( tRes.m_iCpuTime/1000 ), (int)( tRes.m_iCpuTime%1000 )/100 );
		}

		// per-agent times
		if ( dAgentTimes.GetLength() )
		{
			ScopedComma_c dAgents ( tBuf, ", ", " agents=(",")");
			for ( auto iTime : dAgentTimes )
				tBuf.Appendf ( "%d.%03d",
					(int)( iTime/1000),
					(int)( iTime%1000) );
		}

		// warning
		if ( !tRes.m_sWarning.IsEmpty() )
			tBuf.Appendf ( "warning=%s", tRes.m_sWarning.cstr() );
	}
	tBuf.FinishBlock (); // close the comment

	// line feed
	tBuf += "\n";

	sphSeek ( g_iQueryLogFile, 0, SEEK_END );
	sphWrite ( g_iQueryLogFile, tBuf.cstr(), tBuf.GetLength() );
}


void FormatSphinxql ( const CSphQuery & q, int iCompactIN, QuotationEscapedBuilder & tBuf )
{
	if ( q.m_bHasOuter )
		tBuf += "SELECT * FROM (";

	if ( q.m_sSelect.IsEmpty() )
		tBuf.Appendf ( "SELECT * FROM %s", q.m_sIndexes.cstr () );
	else
		tBuf.Appendf ( "SELECT %s FROM %s", RemoveBackQuotes ( q.m_sSelect.cstr () ).cstr (), q.m_sIndexes.cstr () );

	// WHERE clause
	// (m_sRawQuery is empty when using MySQL handler)
	const CSphString & sQuery = q.m_sQuery;
	if ( !sQuery.IsEmpty() || q.m_dFilters.GetLength() )
	{
		ScopedComma_c sWHERE ( tBuf, " AND ", " WHERE ");

		if ( !sQuery.IsEmpty() )
		{
			ScopedComma_c sMatch (tBuf, nullptr, "MATCH(", ")");
			tBuf.FixupSpacedAndAppendEscaped ( sQuery.cstr() );
		}

		FormatFiltersQL ( q.m_dFilters, q.m_dFilterTree, tBuf, iCompactIN );
	}

	// ORDER BY and/or GROUP BY clause
	if ( q.m_sGroupBy.IsEmpty() )
	{
		if ( !q.m_sSortBy.IsEmpty() ) // case API SPH_MATCH_EXTENDED2 - SPH_SORT_RELEVANCE
			FormatOrderBy ( &tBuf, " ORDER BY ", q.m_eSort, q.m_sSortBy );
	} else
	{
		tBuf.Appendf ( " GROUP BY %s", q.m_sGroupBy.cstr() );
		FormatOrderBy ( &tBuf, " WITHIN GROUP ORDER BY ", q.m_eSort, q.m_sSortBy );
		if ( !q.m_tHaving.m_sAttrName.IsEmpty() )
		{
			ScopedComma_c sHawing ( tBuf, nullptr," HAVING ");
			FormatFilterQL ( q.m_tHaving, tBuf, iCompactIN );
		}
		if ( q.m_sGroupSortBy!="@group desc" )
			FormatOrderBy ( &tBuf, " ORDER BY ", SPH_SORT_EXTENDED, q.m_sGroupSortBy );
	}

	// LIMIT clause
	if ( q.m_iOffset!=0 || q.m_iLimit!=20 )
		tBuf.Appendf ( " LIMIT %d,%d", q.m_iOffset, q.m_iLimit );

	// OPTION clause
	FormatOption ( q, tBuf );
	FormatIndexHints ( q, tBuf );

	// outer order by, limit
	if ( q.m_bHasOuter )
	{
		tBuf += ")";
		if ( !q.m_sOuterOrderBy.IsEmpty() )
			tBuf.Appendf ( " ORDER BY %s", q.m_sOuterOrderBy.cstr() );
		if ( q.m_iOuterOffset>0 )
			tBuf.Appendf ( " LIMIT %d, %d", q.m_iOuterOffset, q.m_iOuterLimit );
		else if ( q.m_iOuterLimit>0 )
			tBuf.Appendf ( " LIMIT %d", q.m_iOuterLimit );
	}

	// finish SQL statement
	tBuf += ";";
}

static void LogQuery ( const CSphQuery & q, const CSphQueryResult & tRes, const CSphVector<int64_t> & dAgentTimes )
{
	if ( g_iQueryLogMinMs>0 && tRes.m_iQueryTime<g_iQueryLogMinMs )
		return;

	switch ( g_eLogFormat )
	{
		case LOG_FORMAT_PLAIN:		LogQueryPlain ( q, tRes ); break;
		case LOG_FORMAT_SPHINXQL:	LogQuerySphinxql ( q, tRes, dAgentTimes ); break;
	}
}


void LogSphinxqlError ( const char * sStmt, const char * sError )
{
	if ( g_eLogFormat!=LOG_FORMAT_SPHINXQL || g_iQueryLogFile<0 || !sStmt || !sError )
		return;

	char sTimeBuf[SPH_TIME_PID_MAX_SIZE];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	StringBuilder_c tBuf;
	tBuf.Appendf ( "/""* %s conn %d *""/ %s # error=%s\n", sTimeBuf, myinfo::ConnID(), sStmt, sError );

	sphSeek ( g_iQueryLogFile, 0, SEEK_END );
	sphWrite ( g_iQueryLogFile, tBuf.cstr(), tBuf.GetLength() );
}


void ReportIndexesName ( int iSpanStart, int iSpandEnd, const CSphVector<SearchFailure_t> & dLog, StringBuilder_c & sOut )
{
	int iSpanLen = iSpandEnd - iSpanStart;

	// report distributed index in case all failures are from their locals
	if ( iSpanLen>1 && !dLog[iSpanStart].m_sParentIndex.IsEmpty ()
		&& dLog[iSpanStart].m_sParentIndex==dLog[iSpandEnd-1].m_sParentIndex )
	{
		auto pDist = GetDistr ( dLog[iSpanStart].m_sParentIndex );
		if ( pDist && pDist->m_dLocal.GetLength ()==iSpanLen )
		{
			sOut << dLog[iSpanStart].m_sParentIndex << ": ";
			return;
		}
	}

	// report only first indexes up to 4
	int iEndReport = ( iSpanLen>4 ) ? iSpanStart+3 : iSpandEnd;
	sOut.StartBlock (",");
	for ( int j=iSpanStart; j<iEndReport; ++j )
		sOut << dLog[j].m_sIndex;
	sOut.FinishBlock ();

	// add total index count
	if ( iEndReport!=iSpandEnd )
		sOut.Sprintf ( " and %d more: ", iSpandEnd-iEndReport );
	else
		sOut += ": ";
}

//////////////////////////////////////////////////////////////////////////

void sphGetAttrsToSend ( const ISphSchema & tSchema, bool bAgentMode, bool bNeedId, CSphBitvec & tAttrs )
{
	int iCount = tSchema.GetAttrsCount();
	tAttrs.Init ( iCount );
	if ( !bAgentMode && iCount && IsSortStringInternal ( tSchema.GetAttr ( iCount-1 ).m_sName ) )
	{
		for ( int i=iCount-1; i>=0 && IsSortStringInternal ( tSchema.GetAttr(i).m_sName ); --i )
			iCount = i;
	}

	for ( int i = 0; i < iCount; i++ )
		if ( !sphIsInternalAttr ( tSchema.GetAttr(i) ) )
			tAttrs.BitSet(i);

	int iId = tSchema.GetAttrIndex ( sphGetDocidName() );
	if ( !bAgentMode && iId!=-1 && !bNeedId )
		tAttrs.BitClear(iId);
}

static int SendDataPtrAttr ( ISphOutputBuffer * pOut, const BYTE * pData )
{
	int iLen = pData ? sphUnpackPtrAttr ( pData, &pData ) : 0;
	if ( pOut )
		pOut->SendArray ( pData, iLen );
	return iLen;
}

static char g_sJsonNull[] = "{}";

static int SendJsonAsString ( ISphOutputBuffer * pOut, const BYTE * pJSON )
{
	if ( pJSON )
	{
		int iLengthBytes = sphUnpackPtrAttr ( pJSON, &pJSON );
		JsonEscapedBuilder dJson;
		dJson.GrowEnough ( iLengthBytes*2 );
		sphJsonFormat ( dJson, pJSON );

		if ( pOut )
			pOut->SendArray ( dJson );

		return dJson.GetLength();
	} else
	{
		// magic zero - "{}"
		int iLengthBytes = sizeof(g_sJsonNull)-1;
		if ( pOut )
		{
			pOut->SendDword ( iLengthBytes );
			pOut->SendBytes ( g_sJsonNull, iLengthBytes );
		} 
		
		return iLengthBytes;
	}
}


static int SendJson ( ISphOutputBuffer * pOut, const BYTE * pJSON, bool bSendJson )
{
	if ( bSendJson )
		return SendDataPtrAttr ( pOut, pJSON ); // send BSON
	else
		return SendJsonAsString ( pOut, pJSON ); // send string
}


static int SendJsonFieldAsString ( ISphOutputBuffer * pOut, const BYTE * pJSON )
{
	if ( pJSON )
	{
		int iLengthBytes = sphUnpackPtrAttr ( pJSON, &pJSON );
		JsonEscapedBuilder dJson;
		dJson.GrowEnough ( iLengthBytes * 2 );

		auto eJson = (ESphJsonType)*pJSON++;
		sphJsonFieldFormat ( dJson, pJSON, eJson, false );

		if ( pOut )
			pOut->SendArray ( dJson );
		else
			return dJson.GetLength();
	} else
	{
		if ( pOut )
			pOut->SendDword ( 0 );
	}

	return 0;
}


static int SendJsonField ( ISphOutputBuffer * pOut, const BYTE * pJSON, bool bSendJsonField )
{
	if ( bSendJsonField )
	{
		int iLen = sphUnpackPtrAttr ( pJSON, &pJSON );
		if ( !iLen )
		{
			if ( pOut )
				pOut->SendByte ( JSON_EOF );

			return -3; // 4 bytes by default, and we send only 1. this useless magic should be fixed
		}

		ESphJsonType eJson = (ESphJsonType)*pJSON++;
		if ( eJson==JSON_EOF )
		{
			if ( pOut )
				pOut->SendByte ( (BYTE)eJson );

			return -3;
		}

		iLen--;

		if ( pOut )
		{
			pOut->SendByte ( (BYTE)eJson );
			pOut->SendArray ( pJSON, iLen );
		}

		return iLen+1;
	}
	else
		return SendJsonFieldAsString ( pOut, pJSON );
}


static int SendMVA ( ISphOutputBuffer * pOut, const BYTE * pMVA, bool b64bit )
{
	if ( pMVA )
	{
		int iLengthBytes = sphUnpackPtrAttr( pMVA, &pMVA );
		int iValues = iLengthBytes / sizeof(DWORD);
		if ( pOut )
			pOut->SendDword ( iValues );

		const DWORD * pValues = (const DWORD *)pMVA;

		if ( b64bit )
		{
			assert ( ( iValues%2 )==0 );
			while ( iValues )
			{
				uint64_t uVal = (uint64_t)MVA_UPSIZE ( pValues );
				if ( pOut )
					pOut->SendUint64 ( uVal );
				pValues += 2;
				iValues -= 2;
			}
		} else
		{
			while ( iValues-- )
			{
				if ( pOut )
					pOut->SendDword ( *pValues++ );
			}
		}

		return iLengthBytes;
	} else
	{
		if ( pOut )
			pOut->SendDword ( 0 );
	}

	return 0;
}


static ESphAttr FixupAttrForNetwork ( const CSphColumnInfo & tCol, WORD uMasterVer, bool bAgentMode )
{
	bool bSendJson = ( bAgentMode && uMasterVer>=3 );
	bool bSendJsonField = ( bAgentMode && uMasterVer>=4 );

	switch ( tCol.m_eAttrType )
	{
	case SPH_ATTR_UINT32SET_PTR:
		return SPH_ATTR_UINT32SET;

	case SPH_ATTR_INT64SET_PTR:
		return SPH_ATTR_INT64SET;

	case SPH_ATTR_STRINGPTR:
	{
		if ( bAgentMode && uMasterVer>=18 && ( tCol.m_uFieldFlags & CSphColumnInfo::FIELD_STORED ) )
			return SPH_ATTR_STORED_FIELD;
		else
			return SPH_ATTR_STRING;
	}

	case SPH_ATTR_JSON:
	case SPH_ATTR_JSON_PTR:
		return bSendJson ? SPH_ATTR_JSON : SPH_ATTR_STRING;

	case SPH_ATTR_JSON_FIELD:
	case SPH_ATTR_JSON_FIELD_PTR:
		return bSendJsonField ? SPH_ATTR_JSON_FIELD : SPH_ATTR_STRING;

	default: return tCol.m_eAttrType;
	} 
}


static void SendSchema ( ISphOutputBuffer & tOut, const CSphQueryResult & tRes, const CSphBitvec & tAttrsToSend, WORD uMasterVer, bool bAgentMode )
{
	tOut.SendInt ( tRes.m_tSchema.GetFieldsCount() );
	for ( int i=0; i < tRes.m_tSchema.GetFieldsCount(); ++i )
		tOut.SendString ( tRes.m_tSchema.GetFieldName(i) );

	tOut.SendInt ( tAttrsToSend.BitCount() );
	for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); ++i )
	{
		if ( !tAttrsToSend.BitGet(i) )
			continue;

		const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
		tOut.SendString ( tCol.m_sName.cstr() );

		ESphAttr eCol = FixupAttrForNetwork ( tCol, uMasterVer, bAgentMode );
		tOut.SendDword ( (DWORD)eCol );
	}
}


static void SendAttribute ( ISphOutputBuffer & tOut, const CSphMatch & tMatch, const CSphColumnInfo & tAttr, int iVer, WORD uMasterVer, bool bAgentMode )
{
	// at this point we should not have any attributes that point to pooled data
	assert ( sphPlainAttrToPtrAttr(tAttr.m_eAttrType)==tAttr.m_eAttrType );

	// send binary json only to master
	bool bSendJson = bAgentMode && uMasterVer>=3;
	bool bSendJsonField = bAgentMode && uMasterVer>=4;

	const CSphAttrLocator & tLoc = tAttr.m_tLocator;
	
	switch ( tAttr.m_eAttrType )
	{
	case SPH_ATTR_UINT32SET_PTR:
	case SPH_ATTR_INT64SET_PTR:
		SendMVA ( &tOut, (const BYTE*)tMatch.GetAttr(tLoc), tAttr.m_eAttrType==SPH_ATTR_INT64SET_PTR );
		break;

	case SPH_ATTR_JSON_PTR:
		SendJson ( &tOut, (const BYTE*)tMatch.GetAttr(tLoc), bSendJson );
		break;

	case SPH_ATTR_STRINGPTR:
		SendDataPtrAttr ( &tOut, (const BYTE*)tMatch.GetAttr(tLoc) );
		break;

	case SPH_ATTR_JSON_FIELD_PTR:
		SendJsonField ( &tOut, (const BYTE*)tMatch.GetAttr(tLoc), bSendJsonField );
		break;

	case SPH_ATTR_FACTORS:
	case SPH_ATTR_FACTORS_JSON:
		if ( iVer<0x11C )
		{
			tOut.SendDword ( 0 );
			break;
		}

		SendDataPtrAttr ( &tOut, (const BYTE*)tMatch.GetAttr(tLoc) );
		break;

	case SPH_ATTR_FLOAT:
		tOut.SendFloat ( tMatch.GetAttrFloat(tLoc) );
		break;
	case SPH_ATTR_BIGINT:
		tOut.SendUint64 ( tMatch.GetAttr(tLoc) );
		break;
	default:
		tOut.SendDword ( (DWORD)tMatch.GetAttr(tLoc) );
		break;
	}
}


void SendResult ( int iVer, ISphOutputBuffer & tOut, const AggrResult_t * pRes, bool bAgentMode, const CSphQuery & tQuery, WORD uMasterVer )
{
	// multi-query status
	bool bError = !pRes->m_sError.IsEmpty();
	bool bWarning = !bError && !pRes->m_sWarning.IsEmpty();

	if ( bError )
	{
		tOut.SendInt ( SEARCHD_ERROR ); // fixme! m.b. use APICommand_t and refactor to common API way
		tOut.SendString ( pRes->m_sError.cstr() );
		if ( g_bOptNoDetach && g_eLogFormat!=LOG_FORMAT_SPHINXQL )
			sphInfo ( "query error: %s", pRes->m_sError.cstr() );
		return;

	} else if ( bWarning )
	{
		tOut.SendDword ( SEARCHD_WARNING );
		tOut.SendString ( pRes->m_sWarning.cstr() );
		if ( g_bOptNoDetach && g_eLogFormat!=LOG_FORMAT_SPHINXQL )
			sphInfo ( "query warning: %s", pRes->m_sWarning.cstr() );
	} else
		tOut.SendDword ( SEARCHD_OK );

	CSphBitvec tAttrsToSend;
	sphGetAttrsToSend ( pRes->m_tSchema, bAgentMode, false, tAttrsToSend );

	// send schema
	SendSchema ( tOut, *pRes, tAttrsToSend, uMasterVer, bAgentMode );

	// send matches
	tOut.SendInt ( pRes->m_iCount );
	tOut.SendInt ( 1 ); // was USE_64BIT

	CSphVector<BYTE> dJson ( 512 );

	for ( int i=0; i<pRes->m_iCount; ++i )
	{
		const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];

		Verify ( pRes->m_tSchema.GetAttr(sphGetDocidName()) );
		tOut.SendUint64 ( sphGetDocID(tMatch.m_pDynamic) );
		tOut.SendInt ( tMatch.m_iWeight );

		assert ( tMatch.m_pStatic || !pRes->m_tSchema.GetStaticSize() );
#if 0
		// not correct any more because of internal attrs (such as string sorting ptrs)
		assert ( tMatch.m_pDynamic || !pRes->m_tSchema.GetDynamicSize() );
		assert ( !tMatch.m_pDynamic || (int)tMatch.m_pDynamic[-1]==pRes->m_tSchema.GetDynamicSize() );
#endif
		for ( int j=0; j<pRes->m_tSchema.GetAttrsCount(); ++j )
			if ( tAttrsToSend.BitGet(j) )
				SendAttribute ( tOut, tMatch, pRes->m_tSchema.GetAttr(j), iVer, uMasterVer, bAgentMode );
	}

	if ( tQuery.m_bAgent && tQuery.m_iLimit )
		tOut.SendInt ( pRes->m_iCount );
	else
		tOut.SendInt ( pRes->m_dMatches.GetLength() );

	tOut.SendAsDword ( pRes->m_iTotalMatches );
	tOut.SendInt ( Max ( pRes->m_iQueryTime, 0 ) );

	if ( iVer>=0x11A && bAgentMode )
	{
		bool bNeedPredictedTime = tQuery.m_iMaxPredictedMsec > 0;

		BYTE uStatMask = ( bNeedPredictedTime ? 4 : 0 ) | ( g_bCpuStats ? 2 : 0 ) | ( g_bIOStats ? 1 : 0 );
		tOut.SendByte ( uStatMask );

		if ( g_bIOStats )
		{
			CSphIOStats tStats = pRes->m_tIOStats;
			tStats.Add ( pRes->m_tAgentIOStats );
			tOut.SendUint64 ( tStats.m_iReadTime );
			tOut.SendDword ( tStats.m_iReadOps );
			tOut.SendUint64 ( tStats.m_iReadBytes );
			tOut.SendUint64 ( tStats.m_iWriteTime );
			tOut.SendDword ( tStats.m_iWriteOps );
			tOut.SendUint64 ( tStats.m_iWriteBytes );
		}

		if ( g_bCpuStats )
		{
			int64_t iCpuTime = pRes->m_iCpuTime + pRes->m_iAgentCpuTime;
			tOut.SendUint64 ( iCpuTime );
		}

		if ( bNeedPredictedTime )
			tOut.SendUint64 ( pRes->m_iPredictedTime+pRes->m_iAgentPredictedTime );
	}
	if ( bAgentMode && uMasterVer>=7 )
	{
		tOut.SendDword ( pRes->m_tStats.m_iFetchedDocs + pRes->m_iAgentFetchedDocs );
		tOut.SendDword ( pRes->m_tStats.m_iFetchedHits + pRes->m_iAgentFetchedHits );
		if ( uMasterVer>=8 )
			tOut.SendDword ( pRes->m_tStats.m_iSkips + pRes->m_iAgentFetchedSkips );
	}

	tOut.SendInt ( pRes->m_hWordStats.GetLength() );

	pRes->m_hWordStats.IterateStart();
	while ( pRes->m_hWordStats.IterateNext() )
	{
		const CSphQueryResultMeta::WordStat_t & tStat = pRes->m_hWordStats.IterateGet();
		tOut.SendString ( pRes->m_hWordStats.IterateGetKey().cstr() );
		tOut.SendAsDword ( tStat.first );
		tOut.SendAsDword ( tStat.second );
		if ( bAgentMode )
			tOut.SendByte ( 0 ); // statistics have no expanded terms for now
	}
}

/////////////////////////////////////////////////////////////////////////////

void AggrResult_t::FreeMatchesPtrs ( int iLimit, bool bCommonSchema )
{
	if ( m_dMatches.GetLength ()<=iLimit )
		return;

	if ( bCommonSchema )
	{
		for ( int i = iLimit; i<m_dMatches.GetLength (); ++i )
			m_tSchema.FreeDataPtrs ( m_dMatches[i] );
	} else
	{
		int nMatches = 0;
		ARRAY_FOREACH ( i, m_dMatchCounts )
		{
			nMatches += m_dMatchCounts[i];

			if ( iLimit<nMatches )
			{
				int iFrom = Max ( iLimit, nMatches - m_dMatchCounts[i] );
				for ( int j = iFrom; j<nMatches; ++j )
					m_dSchemas[i].FreeDataPtrs ( m_dMatches[j] );
			}
		}
	}
}

void AggrResult_t::ClampMatches ( int iLimit, bool bCommonSchema )
{
	FreeMatchesPtrs ( iLimit, bCommonSchema );
	if ( m_dMatches.GetLength()<=iLimit )
		return;
	m_dMatches.Resize ( iLimit );
}


int AggrResult_t::FillFromQueue ( ISphMatchSorter * pQueue, int iTag )
{
	if ( !pQueue || !pQueue->GetLength() )
		return 0;

	int iOffset = m_dMatches.GetLength ();
	int iCopied = pQueue->Flatten ( m_dMatches.AddN ( pQueue->GetLength () ), iTag );
	m_dMatches.Resize ( iOffset + iCopied );
	return iCopied;
}

AggrResult_t::~AggrResult_t ()
{
	for ( auto& dMatch : m_dMatches )
		m_tSchema.FreeDataPtrs ( dMatch );
}


struct TaggedMatchSorter_fn : public MatchSortAccessor_t
{
	bool IsLess ( const CSphMatch * a, const CSphMatch * b ) const
	{
		bool bDistA = ( ( a->m_iTag & 0x80000000 )==0x80000000 );
		bool bDistB = ( ( b->m_iTag & 0x80000000 )==0x80000000 );

		DocID_t tDocidA = sphGetDocID ( a->m_pDynamic );
		DocID_t tDocidB = sphGetDocID ( b->m_pDynamic );

		// sort by doc_id, dist_tag, tag
		return ( tDocidA < tDocidB ) ||
			( tDocidA==tDocidB && ( ( !bDistA && bDistB ) || ( ( a->m_iTag & 0x7FFFFFFF )>( b->m_iTag & 0x7FFFFFFF ) ) ) );
	}
};


void RemapResult ( const ISphSchema * pTarget, AggrResult_t * pRes )
{
	int iCur = 0;
	CSphVector<int> dMapFrom ( pTarget->GetAttrsCount() );
	CSphVector<int> dRowItems ( pTarget->GetAttrsCount () );
	static const int SIZE_OF_ROW = 8 * sizeof ( CSphRowitem );

	ARRAY_FOREACH ( iSchema, pRes->m_dSchemas )
	{
		dMapFrom.Resize ( 0 );
		dRowItems.Resize ( 0 );
		CSphSchema & dSchema = pRes->m_dSchemas[iSchema];
		for ( int i=0; i<pTarget->GetAttrsCount(); i++ )
		{
			auto iSrcCol = dSchema.GetAttrIndex ( pTarget->GetAttr ( i ).m_sName.cstr () );
			const CSphColumnInfo &tSrcCol = dSchema.GetAttr ( iSrcCol );
			dMapFrom.Add ( iSrcCol );
			dRowItems.Add ( tSrcCol.m_tLocator.m_iBitOffset / SIZE_OF_ROW );
			assert ( dMapFrom[i]>=0
				|| IsSortStringInternal ( pTarget->GetAttr(i).m_sName )
				|| IsSortJsonInternal ( pTarget->GetAttr(i).m_sName )
				);
		}
		int iLimit = Min ( iCur + pRes->m_dMatchCounts[iSchema], pRes->m_dMatches.GetLength() );

		// inverse dRowItems - we'll free only those NOT enumerated yet
		dRowItems = dSchema.SubsetPtrs ( dRowItems );
		for ( int i=iCur; i<iLimit; i++ )
		{
			CSphMatch & tMatch = pRes->m_dMatches[i];

			// create new and shiny (and properly sized) match
			CSphMatch tRow;
			tRow.Reset ( pTarget->GetDynamicSize() );
			tRow.m_tRowID = tMatch.m_tRowID;
			tRow.m_iWeight = tMatch.m_iWeight;
			tRow.m_iTag = tMatch.m_iTag;

			// remap attrs
			for ( int j=0; j<pTarget->GetAttrsCount(); j++ )
			{
				const CSphColumnInfo & tDst = pTarget->GetAttr(j);
				// we could keep some of the rows static
				// and so, avoid the duplication of the data.
				if ( !tDst.m_tLocator.m_bDynamic )
				{
					assert ( dMapFrom[j]<0 || !dSchema.GetAttr ( dMapFrom[j] ).m_tLocator.m_bDynamic );
					tRow.m_pStatic = tMatch.m_pStatic;
				} else if ( dMapFrom[j]>=0 )
				{
					const CSphColumnInfo & tSrc = dSchema.GetAttr ( dMapFrom[j] );
					if ( tDst.m_eAttrType==SPH_ATTR_FLOAT && tSrc.m_eAttrType==SPH_ATTR_BOOL )
					{
						tRow.SetAttrFloat ( tDst.m_tLocator, ( tMatch.GetAttr ( tSrc.m_tLocator )>0 ? 1.0f : 0.0f ) );
					} else
					{
						tRow.SetAttr ( tDst.m_tLocator, tMatch.GetAttr ( tSrc.m_tLocator ) );
					}
				}
			}
			// swap out old (most likely wrong sized) match
			Swap ( tMatch, tRow );
			dSchema.FreeDataSpecial ( tRow, dRowItems );
		}

		iCur = iLimit;
	}
	assert ( iCur==pRes->m_dMatches.GetLength() );
}


static bool GetIndexSchemaItems ( const ISphSchema & tSchema, const CSphVector<CSphQueryItem> & dItems, CSphVector<int> & dAttrs, CSphVector<int> & dFields )
{
	bool bHaveAsterisk = false;
	for ( const auto & i : dItems )
	{
		if ( i.m_sAlias.cstr() )
		{
			int iAttr = tSchema.GetAttrIndex ( i.m_sAlias.cstr() );
			if ( iAttr>=0 )
				dAttrs.Add(iAttr);
			else
			{
				int iField = tSchema.GetFieldIndex ( i.m_sAlias.cstr() );
				if ( iField>=0 )
					dFields.Add(iField);
			}
		}

		bHaveAsterisk |= i.m_sExpr=="*";
	}

	dAttrs.Sort();
	dFields.Sort();

	return bHaveAsterisk;
}


static bool GetItemsLeftInSchema ( const ISphSchema & tSchema, bool bOnlyPlain, const CSphVector<int> & dAttrs, const CSphVector<int> & dFields, CSphVector<int> & dAttrsInSchema )
{	
	bool bHaveExprs = false;

	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);

		if ( tAttr.m_pExpr )
		{
			bHaveExprs = true;

			if ( bOnlyPlain )
				continue;
		}

		if ( tAttr.m_sName.cstr()[0]!='@' && !dAttrs.BinarySearch(i) )
			dAttrsInSchema.Add(i);
	}

	return bHaveExprs;
}


static void DoExpansion ( const ISphSchema & tSchema, const CSphVector<int> & dAttrsInSchema, const CSphVector<CSphQueryItem> & dItems, CSphVector<CSphQueryItem> & dExpanded )
{
	bool bExpandedAsterisk = false;
	for ( const auto & i : dItems )
	{
		if ( i.m_sExpr=="*" )
		{
			if ( bExpandedAsterisk )
				continue;

			bExpandedAsterisk = true;

			for ( auto iAttr : dAttrsInSchema )
			{
				const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
				CSphQueryItem & tExpanded = dExpanded.Add();
				tExpanded.m_sExpr = tCol.m_sName;
				if ( tCol.m_pExpr )	// stored fields
					tExpanded.m_sAlias = tCol.m_sName;
			}
		}
		else
			dExpanded.Add(i);
	}
}


// rebuild the results itemlist expanding stars
const CSphVector<CSphQueryItem> & ExpandAsterisk ( const ISphSchema & tSchema, const CSphVector<CSphQueryItem> & dItems, CSphVector<CSphQueryItem> & tExpanded, bool bOnlyPlain, bool & bHaveExprs )
{
	// the result schema usually is the index schema + calculated items + @-items
	// we need to extract the index schema only
	CSphVector<int> dIndexSchemaAttrs, dIndexSchemaFields;
	bool bHaveAsterisk = GetIndexSchemaItems ( tSchema, dItems, dIndexSchemaAttrs, dIndexSchemaFields );

	// no stars? Nothing to do.
	if ( !bHaveAsterisk )
		return dItems;

	// find items that are in index schema but not in our requested item list
	// not do not include @-items
	CSphVector<int> dAttrsLeftInSchema;
	bHaveExprs = GetItemsLeftInSchema ( tSchema, bOnlyPlain, dIndexSchemaAttrs, dIndexSchemaFields, dAttrsLeftInSchema );

	DoExpansion ( tSchema, dAttrsLeftInSchema, dItems, tExpanded );

	return tExpanded;
}


static int KillAllDupes ( ISphMatchSorter * pSorter, AggrResult_t & tRes )
{
	assert ( pSorter );
	int iDupes = 0;

	if ( pSorter->IsGroupby () )
	{
		// groupby sorter does that automagically
		pSorter->SetBlobPool ( nullptr );
		int iMC = 0;
		int iBound = 0;

		ARRAY_FOREACH ( i, tRes.m_dMatches )
		{
			CSphMatch & tMatch = tRes.m_dMatches[i];
			if ( !pSorter->PushGrouped ( tMatch, i==iBound ) )
				iDupes++;

			if ( i==iBound )
				iBound += tRes.m_dMatchCounts[iMC++];
		}
	} else
	{
		Verify ( tRes.m_tSchema.GetAttr(0).m_sName==sphGetDocidName() );

		// normal sorter needs massage
		// sort by docid and then by tag to guarantee the replacement order
		TaggedMatchSorter_fn fnSort;
		sphSort ( tRes.m_dMatches.Begin(), tRes.m_dMatches.GetLength(), fnSort, fnSort );

		// by default, simply remove dupes (select first by tag)
		DocID_t tPrevDocID = 0;
		for ( const auto& dMatch : tRes.m_dMatches )
		{
			DocID_t tDocID = sphGetDocID ( dMatch.m_pDynamic );
			if ( tDocID!=tPrevDocID )
				pSorter->Push ( dMatch );
			else
				++iDupes;

			tPrevDocID = tDocID;
		}
	}

	for ( auto& dMatch : tRes.m_dMatches )
		tRes.m_tSchema.FreeDataPtrs ( dMatch );

	tRes.m_dMatches.Reset ();
	tRes.FillFromQueue ( pSorter, -1 );
	return iDupes;
}


static void RecoverAggregateFunctions ( const CSphQuery & tQuery, const AggrResult_t & tRes )
{
	ARRAY_FOREACH ( i, tQuery.m_dItems )
	{
		const CSphQueryItem & tItem = tQuery.m_dItems[i];
		if ( tItem.m_eAggrFunc==SPH_AGGR_NONE )
			continue;

		for ( int j=0; j<tRes.m_tSchema.GetAttrsCount(); j++ )
		{
			auto & tCol = const_cast<CSphColumnInfo&> ( tRes.m_tSchema.GetAttr(j) );
			if ( tCol.m_sName==tItem.m_sAlias )
			{
				assert ( tCol.m_eAggrFunc==SPH_AGGR_NONE );
				tCol.m_eAggrFunc = tItem.m_eAggrFunc;
			}
		}
	}
}


struct GenericMatchSort_fn : public CSphMatchComparatorState
{
	bool IsLess ( const CSphMatch * a, const CSphMatch * b ) const
	{
		for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
			switch ( m_eKeypart[i] )
		{
			case SPH_KEYPART_ROWID:
				if ( a->m_tRowID==b->m_tRowID )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( a->m_tRowID < b->m_tRowID );

			case SPH_KEYPART_DOCID_S:
			{
				register DocID_t aa = sphGetDocID ( a->m_pStatic );
				register DocID_t bb = sphGetDocID ( b->m_pStatic );
				if ( aa==bb )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( aa < bb );
			}

			case SPH_KEYPART_DOCID_D:
			{
				register DocID_t aa = sphGetDocID ( a->m_pDynamic );
				register DocID_t bb = sphGetDocID ( b->m_pDynamic );
				if ( aa==bb )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( aa < bb );
			}

			case SPH_KEYPART_WEIGHT:
				if ( a->m_iWeight==b->m_iWeight )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( a->m_iWeight < b->m_iWeight );

			case SPH_KEYPART_INT:
			{
				register SphAttr_t aa = a->GetAttr ( m_tLocator[i] );
				register SphAttr_t bb = b->GetAttr ( m_tLocator[i] );
				if ( aa==bb )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( aa < bb );
			}
			case SPH_KEYPART_FLOAT:
			{
				register float aa = a->GetAttrFloat ( m_tLocator[i] );
				register float bb = b->GetAttrFloat ( m_tLocator[i] );
				if ( aa==bb )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( aa < bb );
			}
			case SPH_KEYPART_STRINGPTR:
			case SPH_KEYPART_STRING:
			{
				int iCmp = CmpStrings ( *a, *b, i );
				if ( iCmp!=0 )
					return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( iCmp < 0 );
				break;
			}
		}
		return false;
	}
};


/// returns internal magic names for expressions like COUNT(*) that have a corresponding one
/// returns expression itself otherwise
const char * GetMagicSchemaName ( const CSphString & s )
{
	if ( s=="count(*)" )
		return "@count";
	if ( s=="weight()" )
		return "@weight";
	if ( s=="groupby()" )
		return "@groupby";
	return s.cstr();
}


/// a functor to sort columns by (is_aggregate ASC, column_index ASC)
struct AggregateColumnSort_fn
{
	bool IsAggr ( const CSphColumnInfo & c ) const
	{
		return c.m_eAggrFunc!=SPH_AGGR_NONE
			|| c.m_sName=="@groupby"
			|| c.m_sName=="@count"
			|| c.m_sName=="@distinct"
			|| IsSortJsonInternal ( c.m_sName );
	}

	bool IsLess ( const CSphColumnInfo & a, const CSphColumnInfo & b ) const
	{
		bool aa = IsAggr(a);
		bool bb = IsAggr(b);
		if ( aa!=bb )
			return aa < bb;
		return a.m_iIndex < b.m_iIndex;
	}
};


static void ExtractPostlimit ( const ISphSchema & tSchema, bool bMaster, CSphVector<const CSphColumnInfo *> & dPostlimit )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr ( i );
		if ( tCol.m_eStage==SPH_EVAL_POSTLIMIT && ( bMaster || tCol.m_uFieldFlags==CSphColumnInfo::FIELD_NONE ) )
			dPostlimit.Add ( &tCol );
	}
}


static CSphVector<const DocstoreReader_i*> GetUniqueDocstores ( const AggrResult_t & tRes, int iFrom, int iTo )
{
	CSphVector<const DocstoreReader_i*> dDocstores;
	const DocstoreReader_i * pPrev = nullptr;
	for ( int i=iFrom; i<iTo; i++ )
	{
		CSphMatch & tMatch = tRes.m_dMatches[i];
		// remote match (tag highest bit 1) == everything is already computed
		if ( tMatch.m_iTag & 0x80000000 )
			continue;

		const DocstoreReader_i * pDocstore = tRes.m_dTag2Docstore [ tMatch.m_iTag ].m_pDocstore;
		if ( pDocstore && pDocstore!=pPrev )
		{
			dDocstores.Add(pDocstore);
			pPrev = pDocstore;
		}
	}

	dDocstores.Uniq();

	return dDocstores;
}


static void SetupPostlimitExprs ( AggrResult_t & tRes, CSphMatch & tMatch, const CSphColumnInfo * pCol, const char * sQuery, int64_t iDocstoreSessionId )
{
	DocstoreSession_c::Info_t tSessionInfo;
	tSessionInfo.m_pDocstore = tRes.m_dTag2Docstore [ tMatch.m_iTag ].m_pDocstore;
	tSessionInfo.m_iSessionId = iDocstoreSessionId;

	assert ( pCol && pCol->m_pExpr );
	pCol->m_pExpr->Command ( SPH_EXPR_SET_DOCSTORE, &tSessionInfo );
	pCol->m_pExpr->Command ( SPH_EXPR_SET_QUERY, (void *)sQuery);
}


static void EvalPostlimitExprs ( AggrResult_t & tRes, CSphMatch & tMatch, const CSphColumnInfo * pCol )
{
	assert ( pCol && pCol->m_pExpr );

	if ( pCol->m_eAttrType==SPH_ATTR_INTEGER )
		tMatch.SetAttr ( pCol->m_tLocator, pCol->m_pExpr->IntEval(tMatch) );
	else if ( pCol->m_eAttrType==SPH_ATTR_BIGINT )
		tMatch.SetAttr ( pCol->m_tLocator, pCol->m_pExpr->Int64Eval(tMatch) );
	else if ( pCol->m_eAttrType==SPH_ATTR_STRINGPTR )
		tMatch.SetAttr ( pCol->m_tLocator, (SphAttr_t) pCol->m_pExpr->StringEvalPacked ( tMatch ) ); // FIXME! a potential leak of *previous* value?
	else
		tMatch.SetAttrFloat ( pCol->m_tLocator, pCol->m_pExpr->Eval(tMatch) );
}

struct PostLimitArgs_t
{
	const CSphVector<const CSphColumnInfo *> & m_dPostlimit;
	const char * m_sQuery = nullptr;
	int m_iFrom = 0;
	int m_iTo = 0;
	bool m_bMaster = false;

	PostLimitArgs_t ( const CSphVector<const CSphColumnInfo *> & dPostlimit, const char * sQuery )
		: m_dPostlimit ( dPostlimit )
		, m_sQuery ( sQuery )
	{}
};

static void ProcessPostlimit ( const PostLimitArgs_t & tArgs, AggrResult_t & tRes )
{
	if ( !tArgs.m_dPostlimit.GetLength() )
		return;

	// generates docstore session id
	DocstoreSession_c tSession;

	// collect all unique docstores from matches
	CSphVector<const DocstoreReader_i*> dDocstores = GetUniqueDocstores ( tRes, tArgs.m_iFrom, tArgs.m_iTo );
	if ( dDocstores.GetLength() )
	{
		// spawn buffered readers for the current session
		// put them to a global hash
		for ( auto & i : dDocstores )
			i->CreateReader ( tSession.GetUID() );
	}

	int iLastTag = -1;
	for ( int i=tArgs.m_iFrom; i<tArgs.m_iTo; i++ )
	{
		CSphMatch & tMatch = tRes.m_dMatches[i];
		// remote match (tag highest bit 1) == everything is already computed
		if ( tMatch.m_iTag & 0x80000000 )
			continue;

		if ( tMatch.m_iTag!=iLastTag )
		{
			for ( const auto & pCol : tArgs.m_dPostlimit )
				SetupPostlimitExprs ( tRes, tMatch, pCol, tArgs.m_sQuery, tSession.GetUID() );

			iLastTag = tMatch.m_iTag;
		}

		for ( const auto & pCol : tArgs.m_dPostlimit )
			EvalPostlimitExprs ( tRes, tMatch, pCol );
	}
}

struct ProcessPostlimitArgs_t
{
	int		m_iOffset = 0;
	int		m_iLimit = 0;
	int		m_iOuterOffset = 0;
	int		m_iOuterLimit = 0;
	bool	m_bMaster = false;

	const char * m_sQuery = nullptr;

	ProcessPostlimitArgs_t ( const CSphQuery & tQuery, bool bMaster )
	{
		m_iOffset = tQuery.m_iOffset;
		m_iLimit = tQuery.m_iLimit;
		m_iOuterOffset = tQuery.m_iOuterOffset;
		m_iOuterLimit = tQuery.m_iOuterLimit;
		m_sQuery = tQuery.m_sQuery.cstr();

		m_bMaster = bMaster;
	}
};

static void ProcessLocalPostlimit ( const ProcessPostlimitArgs_t & tArgs, AggrResult_t & tRes )
{
	bool bGotPostlimit = false;
	for ( int i=0; i<tRes.m_tSchema.GetAttrsCount() && !bGotPostlimit; i++ )
	{
		const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
		bGotPostlimit = ( tCol.m_eStage==SPH_EVAL_POSTLIMIT && ( tArgs.m_bMaster || tCol.m_uFieldFlags==CSphColumnInfo::FIELD_NONE ) );
	}

	if ( !bGotPostlimit )
		return;

	int iSetNext = 0;
	CSphVector<const CSphColumnInfo *> dPostlimit;
	ARRAY_FOREACH ( iSchema, tRes.m_dSchemas )
	{
		int iSetStart = iSetNext;
		int iSetCount = tRes.m_dMatchCounts[iSchema];
		iSetNext += iSetCount;
		assert ( iSetNext<=tRes.m_dMatches.GetLength() );

		dPostlimit.Resize ( 0 );
		ExtractPostlimit ( tRes.m_dSchemas[iSchema], tArgs.m_bMaster, dPostlimit );
		if ( dPostlimit.IsEmpty() )
			continue;

		int iTo = iSetCount;
		int iOff = Max ( tArgs.m_iOffset, tArgs.m_iOuterOffset );
		int iCount = ( tArgs.m_iOuterLimit ? tArgs.m_iOuterLimit : tArgs.m_iLimit );
		iTo = Max ( Min ( iOff + iCount, iTo ), 0 );
		// we can't estimate limit.offset per result set
		// as matches got merged and sort next step
		int iFrom = 0;

		iFrom += iSetStart;
		iTo += iSetStart;

		PostLimitArgs_t tProcessArgs ( dPostlimit, tArgs.m_sQuery );
		tProcessArgs.m_iFrom = iFrom;
		tProcessArgs.m_iTo = iTo;
		tProcessArgs.m_bMaster = tArgs.m_bMaster;

		ProcessPostlimit ( tProcessArgs, tRes );
	}
}


static bool VerifyMatchCounts ( AggrResult_t & tRes )
{
	// sanity check
	// verify that the match counts are consistent
	int iExpected = 0;
	ARRAY_FOREACH ( i, tRes.m_dMatchCounts )
		iExpected += tRes.m_dMatchCounts[i];

	if ( iExpected!=tRes.m_dMatches.GetLength() )
	{
		tRes.m_sError.SetSprintf ( "internal error: expected %d matches in combined result set, got %d",
			iExpected, tRes.m_dMatches.GetLength() );
		return false;
	}

	return true;
}


static bool MinimizeSchemas ( AggrResult_t & tRes )
{
	bool bAllEqual = true;

	// FIXME? add assert ( tRes.m_tSchema==tRes.m_dSchemas[0] );
	for ( int i=1; i<tRes.m_dSchemas.GetLength(); i++ )
		if ( !MinimizeSchema ( tRes.m_tSchema, tRes.m_dSchemas[i] ) )
			bAllEqual = false;

	return bAllEqual;
}

//////////////////////////////////////////////////////////////////////////
class FrontendSchemaBuilder_c
{
public:
			FrontendSchemaBuilder_c ( const AggrResult_t & tRes, const CSphQuery & tQuery, const CSphVector<CSphQueryItem> & dItems, const CSphVector<CSphQueryItem> & dQueryItems,
				const sph::StringSet & hExtraColumns, bool bQueryFromAPI, bool bHaveLocals );

	void	CollectKnownItems();
	void	AddAttrs();

	bool	CheckUnmapped ( CSphString & sError ) const;
	void	Finalize();

	void	RemapGroupBy();
	void	RemapFacets();

	void	SwapAttrs ( CSphSchema & tSchema );

private:
	const AggrResult_t &				m_tRes;
	const CSphQuery &					m_tQuery;
	const CSphVector<CSphQueryItem> &	m_dItems;
	const CSphVector<CSphQueryItem> &	m_dQueryItems;
	const sph::StringSet &				m_hExtraColumns;
	bool								m_bQueryFromAPI;
	bool								m_bHaveLocals;
	bool								m_bAgent;

	CSphVector<CSphColumnInfo>			m_dFrontend;
	CSphVector<int>						m_dKnownAttrs;
	CSphVector<int>						m_dUnmappedAttrs;
};


FrontendSchemaBuilder_c::FrontendSchemaBuilder_c ( const AggrResult_t & tRes, const CSphQuery & tQuery, const CSphVector<CSphQueryItem> & dItems, const CSphVector<CSphQueryItem> & dQueryItems, const sph::StringSet & hExtraColumns, bool bQueryFromAPI, bool bHaveLocals )
	: m_tRes ( tRes )
	, m_tQuery ( tQuery )
	, m_dItems ( dItems )
	, m_dQueryItems ( dQueryItems )
	, m_hExtraColumns ( hExtraColumns )
	, m_bQueryFromAPI ( bQueryFromAPI )
	, m_bHaveLocals ( bHaveLocals )
	, m_bAgent ( tQuery.m_bAgent )
{
	m_dFrontend.Resize(dItems.GetLength());
}


void FrontendSchemaBuilder_c::CollectKnownItems()
{
	ARRAY_FOREACH ( i, m_dItems )
	{
		const CSphQueryItem & tItem = m_dItems[i];

		int iCol = -1;
		if ( !m_bQueryFromAPI && tItem.m_sAlias.IsEmpty() )
			iCol = m_tRes.m_tSchema.GetAttrIndex ( tItem.m_sExpr.cstr() );

		if ( iCol>=0 )
		{
			m_dKnownAttrs.Add(i);
			m_dFrontend[i].m_sName = tItem.m_sExpr;
			m_dFrontend[i].m_iIndex = iCol;
		}
		else
			m_dUnmappedAttrs.Add(i);
	}
}


void FrontendSchemaBuilder_c::AddAttrs()
{
	bool bUsualApi = !m_bAgent && m_bQueryFromAPI;

	for ( int iCol=0; iCol<m_tRes.m_tSchema.GetAttrsCount(); iCol++ )
	{
		const CSphColumnInfo & tCol = m_tRes.m_tSchema.GetAttr(iCol);

		assert ( !tCol.m_sName.IsEmpty() );
		bool bMagic = ( *tCol.m_sName.cstr()=='@' );

		if ( !bMagic && tCol.m_pExpr )
		{
			ARRAY_FOREACH ( j, m_dUnmappedAttrs )
				if ( m_dItems[ m_dUnmappedAttrs[j] ].m_sAlias==tCol.m_sName )
				{
					int k = m_dUnmappedAttrs[j];
					m_dFrontend[k].m_iIndex = iCol;
					m_dFrontend[k].m_sName = m_dItems[k].m_sAlias;
					m_dKnownAttrs.Add(k);
					m_dUnmappedAttrs.Remove ( j-- ); // do not skip an element next to removed one!
				}

			// FIXME?
			// really not sure if this is the right thing to do
			// but it fixes a couple queries in test_163 in compaitbility mode
			if ( m_bAgent && !m_dFrontend.Contains ( bind ( &CSphColumnInfo::m_sName ), tCol.m_sName ) )
			{
				CSphColumnInfo & t = m_dFrontend.Add();
				t.m_iIndex = iCol;
				t.m_sName = tCol.m_sName;
			}
		} else if ( bMagic && ( tCol.m_pExpr || bUsualApi ) )
		{
			ARRAY_FOREACH ( j, m_dUnmappedAttrs )
				if ( tCol.m_sName==GetMagicSchemaName ( m_dItems[ m_dUnmappedAttrs[j] ].m_sExpr ) )
				{
					int k = m_dUnmappedAttrs[j];
					m_dFrontend[k].m_iIndex = iCol;
					m_dFrontend[k].m_sName = m_dItems[k].m_sAlias;
					m_dKnownAttrs.Add(k);
					m_dUnmappedAttrs.Remove ( j-- ); // do not skip an element next to removed one!
				}

			if ( !m_dFrontend.Contains ( bind ( &CSphColumnInfo::m_sName ), tCol.m_sName ) )
			{
				CSphColumnInfo & t = m_dFrontend.Add();
				t.m_iIndex = iCol;
				t.m_sName = tCol.m_sName;
			}
		} else
		{
			bool bAdded = false;
			ARRAY_FOREACH ( j, m_dUnmappedAttrs )
			{
				int k = m_dUnmappedAttrs[j];
				const CSphQueryItem & t = m_dItems[k];
				if ( ( tCol.m_sName==GetMagicSchemaName ( t.m_sExpr ) && t.m_eAggrFunc==SPH_AGGR_NONE )
					|| ( t.m_sAlias==tCol.m_sName &&
					( m_tRes.m_tSchema.GetAttrIndex ( GetMagicSchemaName ( t.m_sExpr ) )==-1 || t.m_eAggrFunc!=SPH_AGGR_NONE ) ) )
				{
					// tricky bit about naming
					//
					// in master mode, we can just use the alias or expression or whatever
					// the data will be fetched using the locator anyway, column name does not matter anymore
					//
					// in agent mode, however, we need to keep the original column names in our response
					// otherwise, queries like SELECT col1 c, count(*) c FROM dist will fail on master
					// because it won't be able to identify the count(*) aggregate by its name
					m_dFrontend[k].m_iIndex = iCol;
					m_dFrontend[k].m_sName = m_bAgent
						? tCol.m_sName
						: ( m_dItems[k].m_sAlias.IsEmpty()
							? m_dItems[k].m_sExpr
							: m_dItems[k].m_sAlias );
					m_dKnownAttrs.Add(k);
					bAdded = true;
					m_dUnmappedAttrs.Remove ( j-- ); // do not skip an element next to removed one!
				}
			}

			// column was not found in the select list directly
			// however we might need it anyway because of a non-NULL extra-schema
			// (extra-schema is additional set of columns came from right side of query
			// when you perform 'select a from index order by b', the 'b' is not displayed, but need for sorting,
			// so extra-schema in the case will contain 'b').
			// bMagic condition added for @groupbystr in the agent mode
			if ( !bAdded && m_bAgent && ( m_hExtraColumns[tCol.m_sName] || !m_bHaveLocals || bMagic ) )
			{
				CSphColumnInfo & t = m_dFrontend.Add();
				t.m_iIndex = iCol;
				t.m_sName = tCol.m_sName;
			}
		}
	}

	m_dKnownAttrs.Sort();
}


bool FrontendSchemaBuilder_c::CheckUnmapped ( CSphString & sError ) const
{
	// sanity check
	// verify that we actually have all the queried select items
	assert ( m_dUnmappedAttrs.IsEmpty() || ( m_dUnmappedAttrs.GetLength()==1 && m_dItems [ m_dUnmappedAttrs[0] ].m_sExpr=="id" ) );
	ARRAY_FOREACH ( i, m_dItems )
	{
		const CSphQueryItem & tItem = m_dItems[i];
		if ( !m_dKnownAttrs.BinarySearch(i) && tItem.m_sExpr!="id" )
		{
			sError.SetSprintf ( "internal error: column '%s/%s' not found in result set schema", tItem.m_sExpr.cstr(), tItem.m_sAlias.cstr() );
			return false;
		}
	}

	return true;
}


void FrontendSchemaBuilder_c::Finalize()
{
	// finalize the frontend schema columns
	// we kept indexes into internal schema there, now use them to lookup and copy column data
	ARRAY_FOREACH ( i, m_dFrontend )
	{
		CSphColumnInfo & tFrontend = m_dFrontend[i];
		const CSphColumnInfo & s = m_tRes.m_tSchema.GetAttr ( tFrontend.m_iIndex );
		tFrontend.m_tLocator = s.m_tLocator;
		tFrontend.m_eAttrType = s.m_eAttrType;
		tFrontend.m_eAggrFunc = s.m_eAggrFunc; // for a sort loop just below
		tFrontend.m_iIndex = i; // to make the aggr sort loop just below stable
		tFrontend.m_uFieldFlags = s.m_uFieldFlags;
	}

	// tricky bit
	// in agents only, push aggregated columns, if any, to the end
	// for that, sort the schema by (is_aggregate ASC, column_index ASC)
	if ( m_bAgent )
		m_dFrontend.Sort ( AggregateColumnSort_fn() );
}


void FrontendSchemaBuilder_c::RemapGroupBy()
{
	// remap groupby() and aliased groupby() to @groupbystr or string attribute
	const CSphColumnInfo * p = nullptr;
	CSphString sJsonGroupBy;
	if ( sphJsonNameSplit ( m_tQuery.m_sGroupBy.cstr() ) )
	{
		sJsonGroupBy = SortJsonInternalSet ( m_tQuery.m_sGroupBy );
		p = m_tRes.m_tSchema.GetAttr ( sJsonGroupBy.cstr() );
	}

	if ( !p )
	{
		// try string attribute (multiple group-by still displays hashes)
		if ( !m_tQuery.m_sGroupBy.IsEmpty() )
		{
			p = m_tRes.m_tSchema.GetAttr ( m_tQuery.m_sGroupBy.cstr() );
			if ( p )
			{
				if ( p->m_eAttrType==SPH_ATTR_JSON_PTR )
				{
					sJsonGroupBy = SortJsonInternalSet ( m_tQuery.m_sGroupBy );
					p = m_tRes.m_tSchema.GetAttr ( sJsonGroupBy.cstr() );
				} else if ( p->m_eAttrType!=SPH_ATTR_STRINGPTR )
				{
					p = nullptr;
				}
			}
		}

		if ( !p )
			return;
	}

	for ( auto & tFrontend : m_dFrontend )
		if ( tFrontend.m_sName=="groupby()" )
		{
			tFrontend.m_tLocator = p->m_tLocator;
			tFrontend.m_eAttrType = p->m_eAttrType;
			tFrontend.m_eAggrFunc = p->m_eAggrFunc;
		}

	// check aliases too
	for ( const auto & tQueryItem : m_dQueryItems )
	{
		if ( tQueryItem.m_sExpr!="groupby()" )
			continue;

		for ( auto & tFrontend : m_dFrontend )
			if ( tFrontend.m_sName==tQueryItem.m_sAlias )
			{
				tFrontend.m_tLocator = p->m_tLocator;
				tFrontend.m_eAttrType = p->m_eAttrType;
				tFrontend.m_eAggrFunc = p->m_eAggrFunc;
			}
	}
}


void FrontendSchemaBuilder_c::RemapFacets()
{
	// facets
	if ( !m_tQuery.m_bFacet && !m_tQuery.m_bFacetHead )
		return;

	// remap MVA/JSON column to @groupby/@groupbystr in facet queries
	const CSphColumnInfo * pGroupByCol = nullptr;
	CSphString sJsonGroupBy;
	if ( sphJsonNameSplit ( m_tQuery.m_sGroupBy.cstr() ) )
	{
		sJsonGroupBy = SortJsonInternalSet ( m_tQuery.m_sGroupBy );
		pGroupByCol = m_tRes.m_tSchema.GetAttr ( sJsonGroupBy.cstr() );
	}

	if ( !pGroupByCol )
	{
		pGroupByCol = m_tRes.m_tSchema.GetAttr ( "@groupby" );
		if ( !pGroupByCol )
			return;
	}

	for ( auto & tFrontend : m_dFrontend )
	{
		ESphAttr eAttr = tFrontend.m_eAttrType;
		// checking _PTR attrs only because we should not have and non-ptr attr at this point
		if ( m_tQuery.m_sGroupBy==tFrontend.m_sName && ( eAttr==SPH_ATTR_UINT32SET_PTR || eAttr==SPH_ATTR_INT64SET_PTR || eAttr==SPH_ATTR_JSON_FIELD_PTR ) )
		{
			tFrontend.m_tLocator = pGroupByCol->m_tLocator;
			tFrontend.m_eAttrType = pGroupByCol->m_eAttrType;
			tFrontend.m_eAggrFunc = pGroupByCol->m_eAggrFunc;
		}
	}
}


void FrontendSchemaBuilder_c::SwapAttrs ( CSphSchema & tSchema )
{
	tSchema.SwapAttrs ( m_dFrontend );
}

//////////////////////////////////////////////////////////////////////////

static bool MergeAllMatches ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bHaveLocals, bool bAllEqual, bool bMaster, const CSphFilterSettings * pAggrFilter, CSphQueryProfile * pProfiler )
{
	ESphSortOrder eQuerySort = ( tQuery.m_sOuterOrderBy.IsEmpty() ? SPH_SORT_RELEVANCE : SPH_SORT_EXTENDED );
	CSphQuery tQueryCopy = tQuery;
	// got outer order? gotta do a couple things
	if ( tQueryCopy.m_bHasOuter )
	{
		// first, temporarily patch up sorting clause and max_matches (we will restore them later)
		Swap ( tQueryCopy.m_sOuterOrderBy, tQueryCopy.m_sGroupBy.IsEmpty() ? tQueryCopy.m_sSortBy : tQueryCopy.m_sGroupSortBy );
		Swap ( eQuerySort, tQueryCopy.m_eSort );

		// second, apply inner limit now, before (!) reordering
		int iOut = 0;
		int iSetStart = 0;
		for ( int & iCurMatches : tRes.m_dMatchCounts )
		{
			assert ( tQueryCopy.m_iLimit>=0 );
			int iLimitedMatches = Min ( tQueryCopy.m_iLimit, iCurMatches );
			for ( int i=0; i<iLimitedMatches; ++i )
				Swap ( tRes.m_dMatches[iOut++], tRes.m_dMatches[iSetStart+i] );
			iSetStart += iCurMatches;
			iCurMatches = iLimitedMatches;
		}
		tRes.ClampMatches ( iOut, bAllEqual ); // false means no common schema; true == use common schema
	}

	// so we need to bring matches to the schema that the *sorter* wants
	// so we need to create the sorter before conversion
	//
	// create queue
	// at this point, we do not need to compute anything; it all must be here
	SphQueueSettings_t tQueueSettings ( tRes.m_tSchema );
	tQueueSettings.m_pAggrFilter = pAggrFilter;

	// FIXME? probably not right; 20 shards with by 300 matches might be too much
	// but propagating too small inner max_matches to the outer is not right either
	if ( tQueryCopy.m_bHasOuter )
		tQueueSettings.m_iMaxMatches = Min ( tQuery.m_iMaxMatches * tRes.m_dMatchCounts.GetLength(), tRes.m_dMatches.GetLength() );
	else
		tQueueSettings.m_iMaxMatches = Min ( tQuery.m_iMaxMatches, tRes.m_dMatches.GetLength() );
	tQueueSettings.m_iMaxMatches = Max ( tQueueSettings.m_iMaxMatches, 1 );

	SphQueueRes_t tQueueRes;
	CSphScopedPtr<ISphMatchSorter> pSorter  ( sphCreateQueue ( tQueueSettings, tQueryCopy,
			tRes.m_sError, tQueueRes, nullptr ) );

	// restore outer order related patches, or it screws up the query log
	if ( tQueryCopy.m_bHasOuter )
	{
		Swap ( tQueryCopy.m_sOuterOrderBy, tQueryCopy.m_sGroupBy.IsEmpty() ? tQueryCopy.m_sSortBy : tQueryCopy.m_sGroupSortBy );
		Swap ( eQuerySort, tQueryCopy.m_eSort );
	}

	if ( !pSorter )
		return false;

	// reset bAllEqual flag if sorter makes new attributes
	if ( bAllEqual )
	{
		// at first we count already existed internal attributes
		// then check if sorter makes more
		int iRemapCount = GetStringRemapCount ( tRes.m_tSchema, tRes.m_tSchema );
		int iNewCount = GetStringRemapCount ( *pSorter->GetSchema(), tRes.m_tSchema );

		bAllEqual = ( iNewCount<=iRemapCount );
	}

	// sorter expects this

	// just doing tRes.m_tSchema = *pSorter->GetSchema() won't work here
	// because pSorter->GetSchema() may already contain a pointer to tRes.m_tSchema as m_pIndexSchema
	// that's why we explicitly copy a CSphRsetSchema to a plain CSphSchema and move it to tRes.m_tSchema
	CSphSchema tSchemaCopy;
	tSchemaCopy = *pSorter->GetSchema();
	tRes.m_tSchema = std::move ( tSchemaCopy );

	// convert all matches to sorter schema - at least to manage all static to dynamic
	if ( !bAllEqual )
	{
		// post-limit stuff first
		if ( bHaveLocals )
		{
			CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_EVAL_POST );
			ProcessPostlimitArgs_t tProcessArgs ( tQueryCopy, bMaster );
			ProcessLocalPostlimit ( tProcessArgs, tRes );
		}

		RemapResult ( &tRes.m_tSchema, &tRes );
	}

	// do the sort work!
	tRes.m_iTotalMatches -= KillAllDupes ( pSorter.Ptr(), tRes );

	return true;
}


static bool ApplyOuterOrder ( AggrResult_t & tRes, const CSphQuery & tQuery )
{
	// reorder (aka outer order)
	ESphSortFunc eFunc;
	GenericMatchSort_fn tReorder;

	ESortClauseParseResult eRes = sphParseSortClause ( &tQuery, tQuery.m_sOuterOrderBy.cstr(), tRes.m_tSchema, eFunc, tReorder, true, tRes.m_sError );
	if ( eRes==SORT_CLAUSE_RANDOM )
		tRes.m_sError = "order by rand() not supported in outer select";

	if ( eRes!=SORT_CLAUSE_OK )
		return false;

	assert ( eFunc==FUNC_GENERIC1 ||eFunc==FUNC_GENERIC2 || eFunc==FUNC_GENERIC3 || eFunc==FUNC_GENERIC4 || eFunc==FUNC_GENERIC5 );
	sphSort ( tRes.m_dMatches.Begin(), tRes.m_dMatches.GetLength(), tReorder, MatchSortAccessor_t() );

	return true;
}


static void ComputePostlimit ( const ProcessPostlimitArgs_t & tArgs, AggrResult_t & tRes )
{
	CSphVector<const CSphColumnInfo *> dPostlimit;
	ExtractPostlimit ( tRes.m_tSchema, tArgs.m_bMaster, dPostlimit );

	// post compute matches only between offset - limit
	// however at agent we can't estimate limit.offset at master merged result set
	// but master don't provide offset to agents only offset+limit as limit
	// so computing all matches up to iiner.limit \ outer.limit
	int iTo = tRes.m_dMatches.GetLength();
	int iOff = Max ( tArgs.m_iOffset, tArgs.m_iOuterOffset );
	int iCount = ( tArgs.m_iOuterLimit ? tArgs.m_iOuterLimit : tArgs.m_iLimit );
	iTo = Max ( Min ( iOff + iCount, iTo ), 0 );
	int iFrom = Min ( iOff, iTo );

	PostLimitArgs_t tProcessArgs ( dPostlimit, tArgs.m_sQuery );
	tProcessArgs.m_iFrom = iFrom;
	tProcessArgs.m_iTo = iTo;
	tProcessArgs.m_bMaster = tArgs.m_bMaster;

	ProcessPostlimit ( tProcessArgs, tRes );
}


/// merges multiple result sets, remaps columns, does reorder for outer selects
bool MinimizeAggrResult ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bHaveLocals, const sph::StringSet & hExtraColumns, CSphQueryProfile * pProfiler,
	const CSphFilterSettings * pAggrFilter, bool bForceRefItems, bool bMaster, VecRefPtrsAgentConn_t & dRemotes )
{
	if ( !VerifyMatchCounts(tRes) )
		return false;

	bool bReturnZeroCount = !tRes.m_dZeroCount.IsEmpty();
	bool bQueryFromAPI = tQuery.m_eQueryType==QUERY_API;

	// 0 matches via SphinxAPI? no fiddling with schemes is necessary
	// (and via SphinxQL, we still need to return the right schema)
	if ( bQueryFromAPI && tRes.m_dMatches.IsEmpty() )
		return true;

	// 0 result set schemes via SphinxQL? just bail
	if ( !bQueryFromAPI && tRes.m_dSchemas.IsEmpty() && !bReturnZeroCount )
		return true;

	// build a minimal schema over all the (potentially different) schemes
	// that we have in our aggregated result set
	assert ( tRes.m_dSchemas.GetLength() || bReturnZeroCount );
	if ( tRes.m_dSchemas.GetLength() )
		tRes.m_tSchema = tRes.m_dSchemas[0];

	bool bAllEqual = MinimizeSchemas(tRes);

	const CSphVector<CSphQueryItem> & dQueryItems = ( tQuery.m_bFacet || tQuery.m_bFacetHead || bForceRefItems ) ? tQuery.m_dRefItems : tQuery.m_dItems;

	// build a list of select items that the query asked for
	bool bHaveExprs = false;
	CSphVector<CSphQueryItem> tExtItems;
	const CSphVector<CSphQueryItem> & dItems = ExpandAsterisk ( tRes.m_tSchema, dQueryItems, tExtItems, tQuery.m_bFacetHead, bHaveExprs );

	// api + index without attributes + select * case
	// can not skip aggregate filtering
	if ( bQueryFromAPI && !dItems.GetLength() && !pAggrFilter && !bHaveExprs )
	{
		tRes.FreeMatchesPtrs ( 0, bAllEqual );
		return true;
	}

	// build the final schemas!
	FrontendSchemaBuilder_c tFrontendBuilder ( tRes, tQuery, dItems, dQueryItems, hExtraColumns, bQueryFromAPI, bHaveLocals );

	// track select items that made it into the internal schema and the ones that didn't
	tFrontendBuilder.CollectKnownItems();
	tFrontendBuilder.AddAttrs();
	if ( !tFrontendBuilder.CheckUnmapped ( tRes.m_sError ) )
		return false;

	tFrontendBuilder.Finalize();

	// tricky bit
	// in purely distributed case, all schemas are received from the wire, and miss aggregate functions info
	// thus, we need to re-assign that info
	if ( !bHaveLocals )
		RecoverAggregateFunctions ( tQuery, tRes );

	// if there's more than one result set,
	// we now have to merge and order all the matches
	// this is a good time to apply outer order clause, too
	if ( tRes.m_iSuccesses>1 || pAggrFilter )
	{
		if ( !MergeAllMatches ( tRes, tQuery, bHaveLocals, bAllEqual, bMaster, pAggrFilter, pProfiler ) )
			return false;
	}

	// apply outer order clause to single result set
	// (multiple combined sets just got reordered above)
	// apply inner limit first
	if ( tRes.m_iSuccesses==1 && tQuery.m_bHasOuter )
		tRes.ClampMatches ( tQuery.m_iLimit, bAllEqual );

	if ( tRes.m_iSuccesses==1 && tQuery.m_bHasOuter && !tQuery.m_sOuterOrderBy.IsEmpty() )
	{
		if ( !ApplyOuterOrder ( tRes, tQuery ) )
			return false;
	}

	if ( bAllEqual && bHaveLocals )
	{
		CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_EVAL_POST );
		ProcessPostlimitArgs_t tArgs ( tQuery, bMaster );
		ComputePostlimit ( tArgs, tRes );
	}

	if ( bMaster && !dRemotes.IsEmpty() )
	{
		CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_EVAL_GETFIELD );
		RemotesGetField ( dRemotes, tQuery, tRes );
	}

	tFrontendBuilder.RemapGroupBy();
	tFrontendBuilder.RemapFacets();

	// all the merging and sorting is now done
	// replace the minimized matches schema with its subset, the result set schema
	tFrontendBuilder.SwapAttrs ( tRes.m_tSchema );
	return true;
}

/////////////////////////////////////////////////////////////////////////////

struct LocalIndex_t
{
	CSphString	m_sName;
	CSphString	m_sParentIndex;
	int			m_iOrderTag = 0;
	int			m_iWeight = 1;
	int64_t		m_iMass = 0;
};


struct QueryStat_t
{
	uint64_t	m_uQueryTime = 0;
	uint64_t	m_uFoundRows = 0;
	int			m_iSuccesses = 0;
};


struct StatsPerQuery_t
{
	CSphVector<QueryStat_t> m_dStats;
};


struct DistrServedByAgent_t : StatsPerQuery_t
{
	CSphString						m_sIndex;
	CSphVector<int>					m_dAgentIds;
	StrVec_t						m_dLocalNames;
};

/// manage collection of pre-locked indexes (to avoid multilocks)
/// Get(name) - returns an index from collection.
/// AddRLocked(name) - add local idx to collection, read-locking it.
/// AddUnmanaged(name,pidx) - add pre-locked idx, to make it available with GetIndex()
/// d-tr unlocks indexes, added with AddRLockedIndex.
class LockedCollection_c : public ISphNoncopyable
{
	SmallStringHash_T<ServedDescRPtr_c*> m_hUsed;
	SmallStringHash_T<const ServedDesc_t*> 	m_hUnmanaged;
public:
	~LockedCollection_c();
	bool AddRLocked ( const CSphString &sName );
	void AddUnmanaged ( const CSphString &sName, const ServedDesc_t * pIdx );

	const ServedDesc_t * Get ( const CSphString &sName ) const;
};


struct LocalSearch_t
{
	int					m_iLocal;
	ISphMatchSorter **	m_ppSorters;
	CSphQueryResult **	m_ppResults;
	bool				m_bResult;
	int64_t				m_iMass;
};

class SearchHandler_c final : public ISphSearchHandler
{
public:
									SearchHandler_c ( int iQueries, const QueryParser_i * pParser, QueryType_e eQueryType, bool bMaster );
									~SearchHandler_c() final;

	void							RunQueries () final;					///< run all queries, get all results
	void							RunUpdates ( const CSphQuery & tQuery, const CSphString & sIndex, CSphAttrUpdateEx * pUpdates ); ///< run Update command instead of Search
	void							RunDeletes ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors, CSphVector<DocID_t> * pDelDocs );
	void							SetQuery ( int iQuery, const CSphQuery & tQuery, ISphTableFunc * pTableFunc ) final;
	void							SetQueryParser ( const QueryParser_i * pParser, QueryType_e eQueryType );
	void							SetProfile ( CSphQueryProfile * pProfile ) final;
	AggrResult_t *					GetResult ( int iResult ) final { return m_dResults.Begin() + iResult; }
	void							SetFederatedUser () { m_bFederatedUser = true; }
	const CSphString &				GetLocalIndexName ( int iLocal ) const;

public:
	CSphVector<CSphQuery>			m_dQueries;						///< queries which i need to search
	CSphVector<AggrResult_t>		m_dResults;						///< results which i obtained
	CSphVector<StatsPerQuery_t>		m_dQueryIndexStats;				///< statistics for current query
	CSphVector<SearchFailuresLog_c>	m_dFailuresSet;					///< failure logs for each query
	CSphVector < CSphVector<int64_t> >	m_dAgentTimes;				///< per-agent time stats
	LockedCollection_c				m_dLocked;						/// locked indexes
	CSphFixedVector<ISphTableFunc *>	m_dTables;

protected:
	void							RunSubset ( int iStart, int iEnd );	///< run queries against index(es) from first query in the subset
	void							RunLocalSearches();
	void 							RunLocalSearchesCoro ();
	bool							AllowsMulti ( int iStart, int iEnd ) const;
	void							SetupLocalDF ( int iStart, int iEnd );

	int								m_iStart = 0;			///< subset start
	int								m_iEnd = 0;				///< subset end
	bool							m_bMultiQueue = false;	///< whether current subset is subject to multi-queue optimization
	bool							m_bFacetQueue = false;	///< whether current subset is subject to facet-queue optimization
	CSphVector<LocalIndex_t>		m_dLocal;				///< local indexes for the current subset
	CSphFixedVector<StrVec_t> 		m_dExtraSchemas { 0 }; 	///< the extra attrs for agents. One vec per thread
	CSphAttrUpdateEx *				m_pUpdates = nullptr;	///< holder for updates
	CSphVector<DocID_t> *			m_pDelDocs = nullptr;	///< this query is for deleting

	CSphQueryProfile *				m_pProfile = nullptr;
	QueryType_e						m_eQueryType {QUERY_API}; ///< queries from sphinxql require special handling
	const QueryParser_i *			m_pQueryParser;	///< parser used for queries in this handler. e.g. plain or json-style

	// FIXME!!! breaks for dist threads with SNIPPETS expressions for queries to multiple indexes
	mutable ExprHook_c				m_tHook;

	SmallStringHash_T < int64_t >	m_hLocalDocs;
	int64_t							m_iTotalDocs = 0;
	bool							m_bGotLocalDF = false;
	bool							m_bMaster;
	bool							m_bFederatedUser;
	bool							m_bQueryLog = true;

	void							OnRunFinished ();

private:
	bool							CheckMultiQuery ( int iStart, int iEnd ) const;
	bool							RLockInvokedIndexes();
	void							UniqLocals ();
	void							RunActionQuery ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors ); ///< run delete/update
	void							BuildIndexList ( int iStart, int iEnd, int & iDivideLimits, VecRefPtrsAgentConn_t & dRemotes, CSphVector<DistrServedByAgent_t> & dDistrServedByAgent );
	void							CalcTimeStats ( int64_t tmCpu, int64_t tmSubset, int iStart, int iEnd, const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent );
	void							CalcPerIndexStats ( int iStart, int iEnd, const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent ) const;
	void							CalcGlobalStats ( int64_t tmCpu, int64_t tmSubset, int64_t tmLocal, int iStart, int iEnd, const CSphIOStats & tIO, const VecRefPtrsAgentConn_t & dRemotes ) const;
	int								CreateSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, VecTraits_T<StrVec_t> & dExtraSchemas, SphQueueRes_t & tQueueRes ) const;
	int								CreateSingleSorters( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, VecTraits_T<StrVec_t> & dExtraSchemas, SphQueueRes_t & tQueueRes ) const;
	int								CreateMultiQueryOrFacetSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, VecTraits_T<StrVec_t> & dExtraSchemas, SphQueueRes_t & tQueueRes ) const;

	SphQueueSettings_t				MakeQueueSettings ( const CSphIndex * pIndex, int iMaxMatches ) const;
};


ISphSearchHandler * sphCreateSearchHandler ( int iQueries, const QueryParser_i * pQueryParser, QueryType_e eQueryType, bool bMaster )
{
	return new SearchHandler_c ( iQueries, pQueryParser, eQueryType, bMaster );
}


SearchHandler_c::SearchHandler_c ( int iQueries, const QueryParser_i * pQueryParser, QueryType_e eQueryType, bool bMaster )
	: m_dTables ( iQueries )
{
	m_dQueries.Resize ( iQueries );
	m_dResults.Resize ( iQueries );
	m_dFailuresSet.Resize ( iQueries );
	m_dAgentTimes.Resize ( iQueries );
	m_bMaster = bMaster;
	m_bFederatedUser = false;
	ARRAY_FOREACH ( i, m_dTables )
		m_dTables[i] = nullptr;

	SetQueryParser ( pQueryParser, eQueryType );
}

//////////////////
/* Smart gc retire of vec of queries.
 * We have CSphVector<CSphQuery> which is over, but some threads may still use separate queries from it, so we can't just
 * delete it, since they will loose the objects and it will cause crash.
 *
 * So, if some queries are still in use, we retire them with custom deleter, which will decrease counter,
 * and finally delete whole vec.
*/
class RetireQueriesVec_c
{
	CSphVector<CSphQuery>	m_dQueries;	// given queries I'll finally remove
	std::atomic<int>		m_iInUse;	// how many of them still reffered

	void OneQueryDeleted()
	{
		if ( m_iInUse.fetch_sub ( 1, std::memory_order_release )==1 )
		{
			assert( m_iInUse.load ( std::memory_order_acquire )==0 );
			delete this;
		}
	}

	static void Delete ( void * pArg )
	{
		if ( pArg )
		{
			auto pMe = (RetireQueriesVec_c *) ( (CSphQuery *) pArg )->m_pCookie;
			assert ( pMe && "Each retiring query from vec must have address of RetireQueriesVec_c in cookie");
			if ( pMe )
				pMe->OneQueryDeleted ();
		}
	}

public:
	void EngageRetiring ( CSphVector<CSphQuery> dQueries, CSphVector<int> dRetired )
	{
		assert ( !dRetired.IsEmpty () );
		m_iInUse.store ( dRetired.GetLength (), std::memory_order_release );
		m_dQueries = std::move ( dQueries );
		for ( auto iRetired: dRetired )
		{
			m_dQueries[iRetired].m_pCookie = this;
			hazard::Retire ( (void*) &m_dQueries[iRetired], Delete );
		}
	}
};

SearchHandler_c::~SearchHandler_c ()
{
	SafeDelete ( m_pQueryParser );
	ARRAY_FOREACH ( i, m_dTables )
		SafeDelete ( m_dTables[i] );

	auto dPointed = hazard::GetListOfPointed ( m_dQueries );
	if ( !dPointed.IsEmpty () )
	{
		// pQueryHolder will be self-removed when all used queries retired
		auto pQueryHolder = new RetireQueriesVec_c;
		pQueryHolder->EngageRetiring ( std::move ( m_dQueries ), std::move ( dPointed ) );
	}
}

void SearchHandler_c::SetQueryParser ( const QueryParser_i * pParser, QueryType_e eQueryType )
{
	m_pQueryParser = pParser;
	m_eQueryType = eQueryType;
	for ( auto & dQuery : m_dQueries )
	{
		dQuery.m_pQueryParser = pParser;
		dQuery.m_eQueryType = eQueryType;
	}
}

LockedCollection_c::~LockedCollection_c()
{
	for ( m_hUsed.IterateStart (); m_hUsed.IterateNext(); )
		SafeDelete ( m_hUsed.IterateGet () );
}

bool LockedCollection_c::AddRLocked ( const CSphString & sName )
{
	if ( m_hUsed.Exists ( sName ) || m_hUnmanaged.Exists ( sName ) )
		return true;

	auto pServed = GetServed ( sName );
	if ( !pServed )
		return false;

	m_hUsed.Add ( new ServedDescRPtr_c ( pServed ), sName );
	return true;
}

void LockedCollection_c::AddUnmanaged ( const CSphString &sName, const ServedDesc_t * pIdx )
{
	if ( m_hUsed.Exists ( sName ) || m_hUnmanaged.Exists ( sName ) )
		return;

	m_hUnmanaged.Add ( pIdx, sName );
}


const ServedDesc_t * LockedCollection_c::Get ( const CSphString & sName ) const
{
	auto * pppIndex = m_hUsed ( sName );
	if ( pppIndex )
		return **pppIndex;

	auto * ppUnmanaged = m_hUnmanaged ( sName );
	if ( ppUnmanaged )
		return *ppUnmanaged;

	return nullptr;
}


void SearchHandler_c::RunUpdates ( const CSphQuery & tQuery, const CSphString & sIndex,	CSphAttrUpdateEx * pUpdates )
{
	m_bQueryLog = false;
	m_pUpdates = pUpdates;
	RunActionQuery ( tQuery, sIndex, pUpdates->m_pError );
}

void SearchHandler_c::RunDeletes ( const CSphQuery &tQuery, const CSphString &sIndex, CSphString * pErrors, CSphVector<DocID_t> * pDelDocs )
{
	m_bQueryLog = false;
	m_pDelDocs = pDelDocs;
	RunActionQuery ( tQuery, sIndex, pErrors );
}

void SearchHandler_c::RunActionQuery ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors )
{
	SetQuery ( 0, tQuery, nullptr );
	m_dQueries[0].m_sIndexes = sIndex;
	m_dResults[0].m_dTag2Docstore.Resize(1);
	m_dLocal.Add ().m_sName = sIndex;

	CheckQuery ( tQuery, *pErrors );
	if ( !pErrors->IsEmpty() )
		return;

	int64_t tmLocal = -sphMicroTimer();
	int64_t tmCPU = -sphTaskCpuTimer ();

	RunLocalSearches();
	tmLocal += sphMicroTimer();
	tmCPU += sphTaskCpuTimer();

	OnRunFinished();

	auto & tRes = m_dResults[0];

	tRes.m_iOffset = tQuery.m_iOffset;
	tRes.m_iCount = Max ( Min ( tQuery.m_iLimit, tRes.m_dMatches.GetLength()-tQuery.m_iOffset ), 0 );

	tRes.m_iQueryTime += (int)(tmLocal/1000);
	tRes.m_iCpuTime += tmCPU;

	if ( !tRes.m_iSuccesses )
	{
		StringBuilder_c sFailures;
		m_dFailuresSet[0].BuildReport ( sFailures );
		sFailures.MoveTo ( *pErrors );

	} else if ( !tRes.m_sError.IsEmpty() )
	{
		StringBuilder_c sFailures;
		m_dFailuresSet[0].BuildReport ( sFailures );
		sFailures.MoveTo ( tRes.m_sWarning ); // FIXME!!! commit warnings too
	}

	const CSphIOStats & tIO = tRes.m_tIOStats;

	++g_tStats.m_iQueries;
	g_tStats.m_iQueryTime += tmLocal;
	g_tStats.m_iQueryCpuTime += tmLocal;
	g_tStats.m_iDiskReads += tIO.m_iReadOps;
	g_tStats.m_iDiskReadTime += tIO.m_iReadTime;
	g_tStats.m_iDiskReadBytes += tIO.m_iReadBytes;

	if ( m_bQueryLog )
		LogQuery ( m_dQueries[0], m_dResults[0], m_dAgentTimes[0] );
}

void SearchHandler_c::SetQuery ( int iQuery, const CSphQuery & tQuery, ISphTableFunc * pTableFunc )
{
	m_dQueries[iQuery] = tQuery;
	m_dQueries[iQuery].m_pQueryParser = m_pQueryParser;
	m_dQueries[iQuery].m_eQueryType = m_eQueryType;
	m_dTables[iQuery] = pTableFunc;
}


void SearchHandler_c::SetProfile ( CSphQueryProfile * pProfile )
{
	assert ( pProfile );
	m_pProfile = pProfile;
}


void SearchHandler_c::RunQueries()
{
	// batch queries to same index(es)
	// or work each query separately if indexes are different

	int iStart = 0, iEnd = 0;
	ARRAY_FOREACH ( i, m_dQueries )
	{
		if ( m_dQueries[i].m_sIndexes!=m_dQueries[iStart].m_sIndexes )
		{
			RunSubset ( iStart, iEnd );
			iStart = i;
		}
		iEnd = i;
	}
	RunSubset ( iStart, iEnd );
	if ( m_bQueryLog )
	{
		ARRAY_FOREACH ( i, m_dQueries )
			LogQuery ( m_dQueries[i], m_dResults[i], m_dAgentTimes[i] );
	}
	OnRunFinished();
}


// final fixup
void SearchHandler_c::OnRunFinished()
{
	for ( auto & dResult : m_dResults )
		dResult.m_iMatches = dResult.m_dMatches.GetLength();
}


static void MergeWordStats ( CSphQueryResultMeta & tDstResult,
	const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hSrc )
{
	if ( !tDstResult.m_hWordStats.GetLength() )
	{
		// nothing has been set yet; just copy
		tDstResult.m_hWordStats = hSrc;
		return;
	}

	hSrc.IterateStart();
	while ( hSrc.IterateNext() )
	{
		const CSphQueryResultMeta::WordStat_t & tSrcStat = hSrc.IterateGet();
		tDstResult.AddStat ( hSrc.IterateGetKey(), tSrcStat.first, tSrcStat.second );
	}
}


static int64_t CalcPredictedTimeMsec ( const CSphQueryResult & tRes )
{
	assert ( tRes.m_bHasPrediction );

	int64_t iNanoResult = int64_t(g_iPredictorCostSkip)*tRes.m_tStats.m_iSkips+
		g_iPredictorCostDoc*tRes.m_tStats.m_iFetchedDocs+
		g_iPredictorCostHit*tRes.m_tStats.m_iFetchedHits+
		g_iPredictorCostMatch*tRes.m_iTotalMatches;

	return iNanoResult/1000000;
}


static void FlattenToRes ( ISphMatchSorter * pSorter, AggrResult_t & tRes, int iTag )
{
	assert ( pSorter );

	if ( pSorter->GetLength() )
	{
		CSphSchema & tNewSchema = tRes.m_dSchemas.Add();
		tNewSchema = *pSorter->GetSchema();

		tRes.m_dTag2Docstore[iTag].m_pDocstore = tRes.m_pDocstore;

		int iCopied = tRes.FillFromQueue ( pSorter, iTag );
		tRes.m_dMatchCounts.Add ( iCopied );

		// clean up for next index search
		tRes.m_pDocstore = nullptr;
	}
}


static void RemoveMissedRows ( AggrResult_t & tRes )
{
	if ( !tRes.m_dMatchCounts.Last() )
		return;

	CSphMatch * pStart = tRes.m_dMatches.Begin() + tRes.m_dMatches.GetLength() - tRes.m_dMatchCounts.Last();
	CSphMatch * pSrc = pStart;
	CSphMatch * pDst = pStart;
	const CSphMatch * pEnd = tRes.m_dMatches.Begin() + tRes.m_dMatches.GetLength();

	while ( pSrc<pEnd )
	{
		if ( !pSrc->m_pStatic )
		{
			tRes.m_tSchema.FreeDataPtrs ( *pSrc );
			pSrc++;
			continue;
		}

		Swap ( *pSrc, *pDst );
		pSrc++;
		pDst++;
	}

	tRes.m_dMatchCounts.Last() = pDst - pStart;
	tRes.m_dMatches.Resize ( pDst - tRes.m_dMatches.Begin() );
}


const CSphString & SearchHandler_c::GetLocalIndexName ( int iLocal ) const
{
	return m_dLocal[iLocal].m_sName;
}

static int GetMaxMatches ( int iQueryMaxMatches, const CSphIndex * pIndex )
{
	if ( iQueryMaxMatches>DEFAULT_MAX_MATCHES )
	{
		int64_t iDocs = Min ( (int)INT_MAX, pIndex->GetStats().m_iTotalDocuments ); // clamp to int max
		return Min ( iQueryMaxMatches, Max ( iDocs, DEFAULT_MAX_MATCHES ) ); // do not want 0 sorter and sorter longer than query.max_matches
	} else
	{
		return iQueryMaxMatches;
	}
}

SphQueueSettings_t SearchHandler_c::MakeQueueSettings ( const CSphIndex * pIndex, int iMaxMatches ) const
{
	SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema (), m_pProfile );
	tQueueSettings.m_bComputeItems = true;
	tQueueSettings.m_pUpdate = m_pUpdates;
	tQueueSettings.m_pCollection = m_pDelDocs;
	tQueueSettings.m_pHook = &m_tHook;
	tQueueSettings.m_iMaxMatches = GetMaxMatches ( iMaxMatches, pIndex );
	return tQueueSettings;
}


int SearchHandler_c::CreateMultiQueryOrFacetSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter *> & dSorters
		, VecTraits_T<CSphString> & dErrors, VecTraits_T<StrVec_t> & dExtraSchemas, SphQueueRes_t & tQueueRes ) const
{
	int iValidSorters = 0;

	auto tQueueSettings = MakeQueueSettings ( pIndex, m_dQueries[m_iStart].m_iMaxMatches );
	const VecTraits_T<CSphQuery> & dQueries = m_dQueries.Slice ( m_iStart );
	sphCreateMultiQueue ( tQueueSettings, dQueries, dSorters, dErrors, tQueueRes, dExtraSchemas );

	m_dQueries[m_iStart].m_bZSlist = tQueueRes.m_bZonespanlist;
	dSorters.Apply ( [&iValidSorters] ( const ISphMatchSorter * pSorter ) {
		if ( pSorter )
			++iValidSorters;
	} );
	if ( m_bFacetQueue && iValidSorters<dSorters.GetLength () )
	{
		dSorters.Apply ( [] ( ISphMatchSorter *& pSorter ) { SafeDelete (pSorter); } );
		return 0;
	}
	return iValidSorters;
}

int SearchHandler_c::CreateSingleSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter *> & dSorters
		, VecTraits_T<CSphString> & dErrors, VecTraits_T<StrVec_t> & dExtraSchemas, SphQueueRes_t & tQueueRes ) const
{
	int iValidSorters = 0;
	tQueueRes.m_bAlowMulti = false;
	for ( int iQuery = m_iStart; iQuery<=m_iEnd; ++iQuery )
	{
		CSphQuery & tQuery = m_dQueries[iQuery];

		// create queue
		auto tQueueSettings = MakeQueueSettings ( pIndex, tQuery.m_iMaxMatches );
		StrVec_t * pExtra = ( dExtraSchemas.IsEmpty () ? nullptr : &dExtraSchemas[iQuery-m_iStart] );

		ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings, tQuery, dErrors[iQuery-m_iStart], tQueueRes, pExtra );
		if ( !pSorter )
			continue;

		tQuery.m_bZSlist = tQueueRes.m_bZonespanlist;
		dSorters[iQuery-m_iStart] = pSorter;
		++iValidSorters;
	}
	return iValidSorters;
}

int SearchHandler_c::CreateSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter *> & dSorters
		, VecTraits_T<CSphString> & dErrors, VecTraits_T<StrVec_t> & dExtraSchemas, SphQueueRes_t & tQueueRes ) const
{
	if ( m_bMultiQueue || m_bFacetQueue )
		return CreateMultiQueryOrFacetSorters ( pIndex, dSorters, dErrors, dExtraSchemas, tQueueRes );
	return CreateSingleSorters ( pIndex, dSorters, dErrors, dExtraSchemas, tQueueRes );
}

void SearchHandler_c::RunLocalSearches()
{
	m_dQueryIndexStats.Resize ( m_dLocal.GetLength () );
	for ( auto & dQueryIndexStats : m_dQueryIndexStats )
		dQueryIndexStats.m_dStats.Resize ( m_iEnd-m_iStart+1 );

	if (
			g_iDistThreads>1 && // splitted to lines for breakpoints
			m_dLocal.GetLength()>1 )
	{
		RunLocalSearchesCoro ();
		return;
	}

	/// todo! remove this, remove local searches parallel, keep only coro version.

	if ( m_dQueries[0].m_bAgent )
		m_dExtraSchemas.Reset ( m_iEnd-m_iStart+1 );

	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		const LocalIndex_t &dLocal = m_dLocal [iLocal];
		const char * sLocal = dLocal.m_sName.cstr(); // mt
		const char * sParentIndex = dLocal.m_sParentIndex.cstr(); // mt
		int iOrderTag = dLocal.m_iOrderTag; // mt
		int iIndexWeight = dLocal.m_iWeight;

		CrashQuery_t& tCrashQuery = GlobalCrashQueryGetRef(); //mt
		tCrashQuery.m_pIndex = dLocal.m_sName.cstr(); // mt
		tCrashQuery.m_iIndexLen = dLocal.m_sName.Length(); // mt

		// this part is like RunLocalSearchMT
		const auto * pServed = m_dLocked.Get ( sLocal );
		if ( !pServed )
		{
			if ( sParentIndex )
				for ( int i=m_iStart; i<=m_iEnd; ++i )
					m_dFailuresSet[i].SubmitEx ( sParentIndex, nullptr, "local index %s missing", sLocal );

			continue;
		}

		if ( !ServedDesc_t::IsSelectable ( pServed ) )
		{
			for ( int i = m_iStart; i<=m_iEnd; ++i )
				m_dFailuresSet[i].SubmitEx ( sLocal, nullptr, "%s", "index is not suitable for select" );

			continue;
		}

		assert ( pServed->m_pIndex );

		// create sorters
		CSphVector<ISphMatchSorter*> dSorters ( m_iEnd-m_iStart+1 );
		dSorters.ZeroVec ();

		if ( m_bFacetQueue )
			m_bMultiQueue = true;

		m_tHook.SetIndex ( pServed->m_pIndex );
		m_tHook.SetQueryType ( m_eQueryType );

		CSphFixedVector<CSphString> dErrors ( dSorters.GetLength() );
		SphQueueRes_t tQueueRes;
		int iValidSorters = CreateSorters ( pServed->m_pIndex, dSorters, dErrors, m_dExtraSchemas, tQueueRes );
		if ( iValidSorters<dSorters.GetLength() )
		{
			ARRAY_FOREACH ( i, dErrors )
			{
				if ( !dErrors[i].IsEmpty() )
					m_dFailuresSet[m_iStart+i].Submit ( sLocal, sParentIndex, dErrors[i].cstr() );
			}
		}
		if ( !iValidSorters )
			continue;

		m_bMultiQueue = tQueueRes.m_bAlowMulti;

		// me shortcuts
		CSphQueryResult tStats;

		// do the query
		CSphMultiQueryArgs tMultiArgs ( iIndexWeight );
		tMultiArgs.m_uPackedFactorFlags = tQueueRes.m_uPackedFactorFlags;
		if ( m_bGotLocalDF )
		{
			tMultiArgs.m_bLocalDF = true;
			tMultiArgs.m_pLocalDocs = &m_hLocalDocs;
			tMultiArgs.m_iTotalDocs = m_iTotalDocs;
		}

		bool bResult = false;
		if ( m_bMultiQueue )
		{
			tStats.m_tIOStats.Start();
			bResult = pServed->m_pIndex->MultiQuery ( &m_dQueries[m_iStart], &tStats, dSorters.GetLength(), dSorters.Begin(), tMultiArgs );
			tStats.m_tIOStats.Stop();

			for ( auto & i : m_dResults )
				i.m_pDocstore = tStats.m_pDocstore;
		} else
		{
			CSphVector<CSphQueryResult*> dResults ( m_dResults.GetLength() );
			ARRAY_FOREACH ( i, m_dResults )
			{
				dResults[i] = &m_dResults[i];
				dResults[i]->m_pBlobPool = nullptr;
				dResults[i]->m_pDocstore = nullptr;
			}

			dResults[m_iStart]->m_tIOStats.Start();
			bResult = pServed->m_pIndex->MultiQueryEx ( dSorters.GetLength(), &m_dQueries[m_iStart], &dResults[m_iStart], &dSorters[0], tMultiArgs );
			dResults[m_iStart]->m_tIOStats.Stop();
		}

		// this part cames post-mt, after join of all searching threads.

		// handle results
		if ( !bResult )
		{
			// failed, submit local (if not empty) or global error string
			for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
				m_dFailuresSet[iQuery].Submit ( sLocal, sParentIndex, tStats.m_sError.IsEmpty()
					? m_dResults [ m_bMultiQueue ? m_iStart : iQuery ].m_sError.cstr()
					: tStats.m_sError.cstr() );
		} else
		{
			// multi-query succeeded
			for ( int iQuery=m_iStart; iQuery<=m_iEnd; ++iQuery )
			{

				// in mt here kind of tricky index calculation, up to the next lines with sorter


				// but some of the sorters could had failed at "create sorter" stage
				ISphMatchSorter * pSorter = dSorters [ iQuery-m_iStart ];
				if ( !pSorter )
					continue;

				// this one seems OK
				AggrResult_t & tRes = m_dResults[iQuery];

				int64_t iBadRows = m_bMultiQueue ? tStats.m_iBadRows : tRes.m_iBadRows;
				if ( iBadRows )
					tRes.m_sWarning.SetSprintf ( "query result is inaccurate because of " INT64_FMT " missed documents", iBadRows );

				int iQTimeForStats = tRes.m_iQueryTime;

				// multi-queue only returned one result set meta, so we need to replicate it
				if ( m_bMultiQueue )
				{
					auto iQueries = m_iEnd-m_iStart+1;
					// these times will be overridden below, but let's be clean
					iQTimeForStats = tStats.m_iQueryTime / iQueries;
					tRes.m_iQueryTime += iQTimeForStats;

					tRes.m_pBlobPool = tStats.m_pBlobPool;
					MergeWordStats ( tRes, tStats.m_hWordStats );
					tRes.m_iMultiplier = iQueries;
					tRes.m_iCpuTime += tStats.m_iCpuTime / iQueries;
					tRes.m_tIOStats.Add ( tStats.m_tIOStats );
				} else if ( tRes.m_iMultiplier==-1 )
				{
					m_dFailuresSet[iQuery].Submit ( sLocal, sParentIndex, tRes.m_sError.cstr() );
					continue;
				}

				++tRes.m_iSuccesses;
				// lets do this schema copy just once
				tRes.m_tSchema = *pSorter->GetSchema();
				tRes.m_iTotalMatches += pSorter->GetTotalCount();
				tRes.m_iPredictedTime = tRes.m_bHasPrediction ? CalcPredictedTimeMsec ( tRes ) : 0;

				m_dQueryIndexStats[iLocal].m_dStats[iQuery-m_iStart].m_iSuccesses = 1;
				m_dQueryIndexStats[iLocal].m_dStats[iQuery-m_iStart].m_uQueryTime = iQTimeForStats;
				m_dQueryIndexStats[iLocal].m_dStats[iQuery-m_iStart].m_uFoundRows = pSorter->GetTotalCount();

				// extract matches from sorter
				FlattenToRes ( pSorter, tRes, iOrderTag+iQuery-m_iStart );

				if ( iBadRows )
					RemoveMissedRows ( tRes );
			}
		}

		// cleanup sorters
		for ( auto &pSorter : dSorters )
			SafeDelete ( pSorter );
	}
}

struct LocalSearchRef_t
{
	using VecExtras_t = VecTraits_T<StrVec_t>;

	ExprHook_c&	m_tHook;
	VecExtras_t& m_dExtras;

	LocalSearchRef_t ( ExprHook_c & tHook, VecExtras_t & dExtras )
		: m_tHook ( tHook )
		, m_dExtras ( dExtras )
	{}

	void MergeChild ( LocalSearchRef_t dChild )
	{
		if ( m_dExtras.IsEmpty ())
			return;

		ARRAY_FOREACH (i, m_dExtras)
			m_dExtras[i].Append ( dChild.m_dExtras[i] );
	}

	inline static bool IsClonable()
	{
		return true;
	}
};

struct LocalSearchClone_t
{
	ExprHook_c m_tHook;
	CSphFixedVector<StrVec_t> m_dExtras {0};

	explicit LocalSearchClone_t ( const LocalSearchRef_t & dParent )
	{
		m_dExtras.Reset ( dParent.m_dExtras.GetLength() );
	}

	explicit operator LocalSearchRef_t ()
	{
		return { m_tHook, m_dExtras };
	}
};

void SearchHandler_c::RunLocalSearchesCoro ()
{
	int64_t tmLocal = sphMicroTimer ();

	// setup local searches
	const int iNumLocals = m_dLocal.GetLength();
	const int iQueries = m_iEnd-m_iStart+1;

	CSphFixedVector<CSphQueryResult> dAllResults ( iQueries*iNumLocals );
	CSphFixedVector<ISphMatchSorter *> dAllSorters ( iQueries*iNumLocals );
	dAllSorters.Fill ( nullptr );

	if ( m_dQueries[m_iStart].m_bAgent)
		m_dExtraSchemas.Reset ( iQueries );
	CSphFixedVector<bool> bResults ( iNumLocals );

	// start in mass order
	CSphFixedVector<int> dOrder {iNumLocals};
	for ( int i = 0; i<iNumLocals; ++i )
		dOrder[i] = i;

	// set order by decreasing index mass (heaviest cames first). That is why 'less' implemented by '>'
	dOrder.Sort ( Lesser ( [this] ( int a, int b ) { return m_dLocal[a].m_iMass>m_dLocal[b].m_iMass; } ));

	// store and manage tls stuff
	using LocalFederateCtx_t = FederateCtx_T<LocalSearchRef_t, LocalSearchClone_t>;
	LocalFederateCtx_t dCtxData { m_tHook, m_dExtraSchemas };

	std::atomic<int32_t> iCurIdx { 0 };
	CoExecuteN ( [&] () mutable
	{
		Threads::CoThrottler_c tThrottler ( myinfo::ref<ClientTaskInfo_t> ()->m_iThrottlingPeriod );
		while ( true )
		{
			auto iJob = iCurIdx.fetch_add ( 1, std::memory_order_acq_rel );
			if ( iJob>=iNumLocals )
				return; // all is done

			auto iIdx = dOrder[iJob];

			sphLogDebugv ("Tick coro search");
			int64_t iCpuTime = -sphTaskCpuTimer ();

			// FIXME!!! handle different proto
			myinfo::SetThreadInfo( R"(api-search query="%s" comment="%s" index="%s")",
									 m_dQueries[m_iStart].m_sQuery.scstr (),
									 m_dQueries[m_iStart].m_sComment.scstr (),
									 m_dLocal[iIdx].m_sName.scstr ());

			const CSphString & sIndex = GetLocalIndexName ( iIdx );
			auto& tCrashQuery = GlobalCrashQueryGetRef();
			tCrashQuery.m_pIndex = sIndex.cstr ();
			tCrashQuery.m_iIndexLen = sIndex.Length ();

			auto * pServed = m_dLocked.Get ( sIndex );
			if ( !pServed )
			{
				// FIXME! submit a failure?
				return;
			}
			assert ( pServed->m_pIndex );

			auto tCtx = dCtxData.GetContext ( iIdx );
			tCtx.m_tHook.SetIndex ( pServed->m_pIndex );
			tCtx.m_tHook.SetQueryType ( m_eQueryType );

			bResults[iIdx] = false;

			SphQueueRes_t tQueueRes;
			auto dSorters = dAllSorters.Slice ( iIdx * iQueries, iQueries );
			auto dResults = dAllResults.Slice ( iIdx * iQueries, iQueries );
			VecTraits_T<StrVec_t> dExtraSchemas;
			if ( !m_dExtraSchemas.IsEmpty ())
				dExtraSchemas = m_dExtraSchemas.Slice ( iIdx * iQueries, iQueries );
			CSphFixedVector<CSphString> dErrors ( dSorters.GetLength ());
			int iValidSorters = CreateSorters ( pServed->m_pIndex, dSorters, dErrors, dExtraSchemas, tQueueRes );
			if ( iValidSorters<dSorters.GetLength ())
			{
				ARRAY_FOREACH ( i, dErrors )
				{
					if ( !dErrors[i].IsEmpty ())
						dResults[i].m_sError = std::move(dErrors[i]);
				}
			}

			if ( !iValidSorters )
				return;

			m_bMultiQueue = tQueueRes.m_bAlowMulti;

			int iIndexWeight = m_dLocal[iIdx].m_iWeight;

			// do the query
			CSphMultiQueryArgs tMultiArgs ( iIndexWeight );
			tMultiArgs.m_uPackedFactorFlags = tQueueRes.m_uPackedFactorFlags;
			if ( m_bGotLocalDF )
			{
				tMultiArgs.m_bLocalDF = true;
				tMultiArgs.m_pLocalDocs = &m_hLocalDocs;
				tMultiArgs.m_iTotalDocs = m_iTotalDocs;
			}

			dResults[0].m_tIOStats.Start ();
			if ( m_bMultiQueue )
				bResults[iIdx] = pServed->m_pIndex->MultiQuery ( &m_dQueries[m_iStart], &dResults[0], iQueries,
																  &dSorters[0], tMultiArgs );
			else
			{
				CSphVector<CSphQueryResult *> dpResults ( dResults.GetLength ());
				ARRAY_FOREACH ( i, dpResults )
				{
					dpResults[i] = &dResults[i];
					dpResults[i]->m_pBlobPool = nullptr;
					dpResults[i]->m_pDocstore = nullptr;
				}
				bResults[iIdx] = pServed->m_pIndex->MultiQueryEx ( iQueries, &m_dQueries[m_iStart], &dpResults[0],
																	&dSorters[0], tMultiArgs );
			}
			dResults[0].m_tIOStats.Stop ();

			iCpuTime += sphTaskCpuTimer ();
			for ( int i = 0; i<iQueries; ++i )
				dResults[i].m_iCpuTime = iCpuTime;

			// yield and reschedule every quant of time. It gives work to other tasks
			tThrottler.ThrottleAndKeepCrashQuery (); // we set CrashQuery anyway at the start of the loop
		}
	}, dCtxData.Concurrency ( iNumLocals ));
	dCtxData.Finalize (); // merge extra schemas (if any)

	int iTotalSuccesses = 0;

	// now merge the results
	for ( auto iLocal=0; iLocal<iNumLocals; ++iLocal )
	{
		bool bResult = bResults[iLocal];
		const char * sLocal = m_dLocal[iLocal].m_sName.cstr();
		const char * sParentIndex = m_dLocal[iLocal].m_sParentIndex.cstr();
		int iOrderTag = m_dLocal[iLocal].m_iOrderTag;

		if ( !bResult )
		{
			// failed
			for ( int iQuery=m_iStart; iQuery<=m_iEnd; ++iQuery )
			{
				int iResultIndex = iLocal*iQueries;
				if ( !m_bMultiQueue )
					iResultIndex += iQuery - m_iStart;
				m_dFailuresSet[iQuery].Submit ( sLocal, sParentIndex, dAllResults[iResultIndex].m_sError.cstr() );
			}
			continue;
		}

		// multi-query succeeded
		for ( int iQuery=m_iStart; iQuery<=m_iEnd; ++iQuery )
		{
			// base result set index
			// in multi-queue case, the only (!) result set actually filled with meta info
			// in non-multi-queue case, just a first index, we fix it below
			int iResultIndex = iLocal*iQueries;

			// current sorter ALWAYS resides at this index, in all cases
			// (current as in sorter for iQuery-th query against iLocal-th index)
			int iSorterIndex = iResultIndex + iQuery - m_iStart;

			if ( !m_bMultiQueue )
			{
				// non-multi-queue case
				// means that we have mere 1:1 mapping between results and sorters
				// so let's adjust result set index
				iResultIndex = iSorterIndex;

			} else if ( dAllResults[iResultIndex].m_iMultiplier==-1 )
			{
				// multi-queue case
				// need to additionally check per-query failures of MultiQueryEx
				// those are reported through multiplier
				// note that iSorterIndex just below is NOT a typo
				// separate errors still go into separate result sets
				// even though regular meta does not
				m_dFailuresSet[iQuery].Submit ( sLocal, sParentIndex, dAllResults[iSorterIndex].m_sError.cstr() );
				continue;
			}

			// no sorter, no fun
			ISphMatchSorter * pSorter = dAllSorters [ iSorterIndex ];
			if ( !pSorter )
				continue;

			// this one seems OK
			AggrResult_t & tRes = m_dResults[iQuery];
			CSphQueryResult & tRaw = dAllResults[iResultIndex];

			++iTotalSuccesses;

			tRes.m_pDocstore = tRaw.m_pDocstore;

			tRes.m_pBlobPool = tRaw.m_pBlobPool;
			MergeWordStats ( tRes, tRaw.m_hWordStats );
			tRes.m_iMultiplier = m_bMultiQueue ? iQueries : 1;
			tRes.m_iCpuTime += tRaw.m_iCpuTime / tRes.m_iMultiplier;
			tRes.m_tIOStats.Add ( tRaw.m_tIOStats );


			tRes.m_bHasPrediction |= tRaw.m_bHasPrediction;
			if ( tRaw.m_bHasPrediction )
			{
				tRes.m_tStats.Add ( tRaw.m_tStats );
				tRes.m_iPredictedTime = CalcPredictedTimeMsec ( tRes );
			}
			if ( tRaw.m_iBadRows )
				tRes.m_sWarning.SetSprintf ( "query result is inaccurate because of " INT64_FMT " missed documents", tRaw.m_iBadRows );

			++tRes.m_iSuccesses;
			// take over the schema from sorter, it doesn't need it anymore
			tRes.m_tSchema = *pSorter->GetSchema();
			tRes.m_iTotalMatches += pSorter->GetTotalCount ();

			m_dQueryIndexStats[iLocal].m_dStats[iQuery-m_iStart].m_iSuccesses = 1;
			m_dQueryIndexStats[iLocal].m_dStats[iQuery-m_iStart].m_uFoundRows = pSorter->GetTotalCount ();

			// extract matches from sorter
			FlattenToRes ( pSorter, tRes, iOrderTag+iQuery-m_iStart );

			if ( tRaw.m_iBadRows )
				RemoveMissedRows ( tRes );

			if ( !tRaw.m_sWarning.IsEmpty() )
				m_dFailuresSet[iQuery].Submit ( sLocal, sParentIndex, tRaw.m_sWarning.cstr() );

		}
	}

	for ( auto & pSorter : dAllSorters )
		SafeDelete ( pSorter );

	// update our wall time for every result set
	tmLocal = sphMicroTimer() - tmLocal;
	for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
		m_dResults[iQuery].m_iQueryTime += (int)( tmLocal/1000 );

	for ( auto iLocal = 0; iLocal<iNumLocals; ++iLocal )
		for ( int iQuery=m_iStart; iQuery<=m_iEnd; ++iQuery )
		{
			QueryStat_t & tStat = m_dQueryIndexStats[iLocal].m_dStats[iQuery-m_iStart];
			if ( tStat.m_iSuccesses )
				tStat.m_uQueryTime = (int)( tmLocal/1000/iTotalSuccesses );
		}

}

// check expressions into a query to make sure that it's ready for multi query optimization
bool SearchHandler_c::AllowsMulti ( int iStart, int iEnd ) const
{
	if ( m_bFacetQueue )
		return true;

	// in some cases the same select list allows queries to be multi query optimized
	// but we need to check dynamic parts size equality and we do it later in RunLocalSearches()
	const CSphVector<CSphQueryItem> & tFirstQueryItems = m_dQueries [ iStart ].m_dItems;
	bool bItemsSameLen = true;
	for ( int i=iStart+1; i<=iEnd && bItemsSameLen; i++ )
		bItemsSameLen = ( tFirstQueryItems.GetLength()==m_dQueries[i].m_dItems.GetLength() );
	if ( bItemsSameLen )
	{
		bool bSameItems = true;
		ARRAY_FOREACH_COND ( i, tFirstQueryItems, bSameItems )
		{
			const CSphQueryItem & tItem1 = tFirstQueryItems[i];
			for ( int j=iStart+1; j<=iEnd && bSameItems; j++ )
			{
				const CSphQueryItem & tItem2 = m_dQueries[j].m_dItems[i];
				bSameItems = tItem1.m_sExpr==tItem2.m_sExpr && tItem1.m_eAggrFunc==tItem2.m_eAggrFunc;
			}
		}

		if ( bSameItems )
			return true;
	}

	// if select lists do not contain any expressions we can optimize queries too
	for ( const auto & dLocal : m_dLocal )
	{
		const auto * pServedIndex = m_dLocked.Get ( dLocal.m_sName );

		// check that it exists
		if ( !pServedIndex )
			continue;

		// FIXME!!! compare expressions as m_pExpr->GetHash
		const CSphSchema & tSchema = pServedIndex->m_pIndex->GetMatchSchema();
		for ( int i=iStart; i<=iEnd; ++i )
			if ( sphHasExpressions ( m_dQueries[i], tSchema ) )
				return false;
	}
	return true;
}


struct IndexSettings_t
{
	uint64_t	m_uHash;
	int			m_iLocal;
};

void SearchHandler_c::SetupLocalDF ( int iStart, int iEnd )
{
	if ( m_dLocal.GetLength()<2 )
		return;

	if ( m_pProfile )
		m_pProfile->Switch ( SPH_QSTATE_LOCAL_DF );

	bool bGlobalIDF = true;
	ARRAY_FOREACH_COND ( i, m_dLocal, bGlobalIDF )
	{
		ServedDescRPtr_c pDesc ( GetServed( m_dLocal[i].m_sName ) );
		bGlobalIDF = ( pDesc && !pDesc->m_sGlobalIDFPath.IsEmpty () );
	}
	// bail out on all indexes with global idf set
	if ( bGlobalIDF )
		return;

	bool bOnlyNoneRanker = true;
	bool bOnlyFullScan = true;
	bool bHasLocalDF = false;
	for ( int iQuery=iStart; iQuery<=iEnd; iQuery++ )
	{
		const CSphQuery & tQuery = m_dQueries[iQuery];

		bOnlyFullScan &= tQuery.m_sQuery.IsEmpty();
		bHasLocalDF |= tQuery.m_bLocalDF;
		if ( !tQuery.m_sQuery.IsEmpty() && tQuery.m_bLocalDF )
			bOnlyNoneRanker &= ( tQuery.m_eRanker==SPH_RANK_NONE );
	}
	// bail out queries: full-scan, ranker=none, local_idf=0
	if ( bOnlyFullScan || bOnlyNoneRanker || !bHasLocalDF )
		return;

	CSphVector<char> dQuery ( 512 );
	dQuery.Resize ( 0 );
	for ( int iQuery=iStart; iQuery<=iEnd; iQuery++ )
	{
		const CSphQuery & tQuery = m_dQueries[iQuery];
		if ( tQuery.m_sQuery.IsEmpty() || !tQuery.m_bLocalDF || tQuery.m_eRanker==SPH_RANK_NONE )
			continue;

		int iLen = tQuery.m_sQuery.Length();
		auto * pDst = dQuery.AddN ( iLen + 1 );
		memcpy ( pDst, tQuery.m_sQuery.cstr(), iLen );
		dQuery.Last() = ' '; // queries delimiter
	}
	// bail out on empty queries
	if ( !dQuery.GetLength() )
		return;

	dQuery.Add ( '\0' );

	// order indexes by settings
	CSphVector<IndexSettings_t> dLocal ( m_dLocal.GetLength() );
	dLocal.Resize ( 0 );
	ARRAY_FOREACH ( i, m_dLocal )
	{
		const auto * pIndex = m_dLocked.Get ( m_dLocal[i].m_sName );
		if ( !pIndex )
			continue;

		dLocal.Add();
		dLocal.Last().m_iLocal = i;
		// TODO: cache settingsFNV on index load
		// FIXME!!! no need to count dictionary hash
		dLocal.Last().m_uHash = pIndex->m_pIndex->GetTokenizer()->GetSettingsFNV() ^ pIndex->m_pIndex->GetDictionary()->GetSettingsFNV();
	}
	dLocal.Sort ( bind ( &IndexSettings_t::m_uHash ) );

	// gather per-term docs count
	CSphVector < CSphKeywordInfo > dKeywords;
	ARRAY_FOREACH ( i, dLocal )
	{
		int iLocalIndex = dLocal[i].m_iLocal;
		const auto * pIndex = m_dLocked.Get ( m_dLocal[iLocalIndex].m_sName );
		if ( !pIndex )
			continue;

		m_iTotalDocs += pIndex->m_pIndex->GetStats().m_iTotalDocuments;

		if ( i && dLocal[i].m_uHash==dLocal[i-1].m_uHash )
		{
			ARRAY_FOREACH ( kw, dKeywords )
				dKeywords[kw].m_iDocs = 0;

			// no need to tokenize query just fill docs count
			pIndex->m_pIndex->FillKeywords ( dKeywords );
		} else
		{
			GetKeywordsSettings_t tSettings;
			tSettings.m_bStats = true;
			dKeywords.Resize ( 0 );
			pIndex->m_pIndex->GetKeywords ( dKeywords, dQuery.Begin(), tSettings, NULL );

			// FIXME!!! move duplicate removal to GetKeywords to do less QWord setup and dict searching
			// custom uniq - got rid of word duplicates
			dKeywords.Sort ( bind ( &CSphKeywordInfo::m_sNormalized ) );
			if ( dKeywords.GetLength()>1 )
			{
				int iSrc = 1, iDst = 1;
				while ( iSrc<dKeywords.GetLength() )
				{
					if ( dKeywords[iDst-1].m_sNormalized==dKeywords[iSrc].m_sNormalized )
						iSrc++;
					else
					{
						Swap ( dKeywords[iDst], dKeywords[iSrc] );
						iDst++;
						iSrc++;
					}
				}
				dKeywords.Resize ( iDst );
			}
		}

		ARRAY_FOREACH ( j, dKeywords )
		{
			const CSphKeywordInfo & tKw = dKeywords[j];
			int64_t * pDocs = m_hLocalDocs ( tKw.m_sNormalized );
			if ( pDocs )
				*pDocs += tKw.m_iDocs;
			else
				m_hLocalDocs.Add ( tKw.m_iDocs, tKw.m_sNormalized );
		}
	}

	m_bGotLocalDF = true;
}


static int GetIndexWeight ( const CSphString& sName, const CSphVector<CSphNamedInt> & dIndexWeights, int iDefaultWeight )
{
	for ( auto& dWeight : dIndexWeights )
		if ( dWeight.first==sName )
			return dWeight.second;

	// distributed index adds {'*', weight} to all agents in case it got custom weight
	if ( dIndexWeights.GetLength() && dIndexWeights.Last().first=="*" )
		return dIndexWeights[0].second;

	return iDefaultWeight;
}

static uint64_t CalculateMass ( const CSphIndexStatus & dStats )
{
	return dStats.m_iNumChunks * 1000000 + dStats.m_iRamUse + dStats.m_iDiskUse * 10;
}

static uint64_t GetIndexMass ( const CSphString & sName )
{
	ServedDescRPtr_c pIdx ( GetServed ( sName ) );
	uint64_t iMass = pIdx ? pIdx->m_iMass : 0;
	return iMass;
}

struct TaggedLocalSorter_fn
{
	bool IsLess ( const LocalIndex_t & a, const LocalIndex_t & b ) const
	{
		return ( a.m_sName < b.m_sName ) || ( a.m_sName==b.m_sName && ( a.m_iOrderTag & 0x7FFFFFFF )>( b.m_iOrderTag & 0x7FFFFFFF ) );
	}
};

////////////////////////////////////////////////////////////////
// check for single-query, multi-queue optimization possibility
////////////////////////////////////////////////////////////////
bool SearchHandler_c::CheckMultiQuery ( int iStart, int iEnd ) const
{
	if (iStart>=iEnd)
		return false;

	for ( int iCheck = iStart + 1; iCheck<=iEnd; ++iCheck )
	{
		const CSphQuery &qFirst = m_dQueries[iStart];
		const CSphQuery &qCheck = m_dQueries[iCheck];

		// these parameters must be the same
		if (
			( qCheck.m_sRawQuery!=qFirst.m_sRawQuery ) || // query string
				( qCheck.m_dWeights.GetLength ()!=qFirst.m_dWeights.GetLength () ) || // weights count
				( qCheck.m_dWeights.GetLength () && memcmp ( qCheck.m_dWeights.Begin (), qFirst.m_dWeights.Begin (),
					sizeof ( qCheck.m_dWeights[0] ) * qCheck.m_dWeights.GetLength () ) ) || // weights
				( qCheck.m_eMode!=qFirst.m_eMode ) || // search mode
				( qCheck.m_eRanker!=qFirst.m_eRanker ) || // ranking mode
				( qCheck.m_dFilters.GetLength ()!=qFirst.m_dFilters.GetLength () ) || // attr filters count
				( qCheck.m_dFilterTree.GetLength ()!=qFirst.m_dFilterTree.GetLength () ) ||
				( qCheck.m_iCutoff!=qFirst.m_iCutoff ) || // cutoff
				( qCheck.m_eSort==SPH_SORT_EXPR && qFirst.m_eSort==SPH_SORT_EXPR && qCheck.m_sSortBy!=qFirst.m_sSortBy )
				|| // sort expressions
					( qCheck.m_bGeoAnchor!=qFirst.m_bGeoAnchor ) || // geodist expression
				( qCheck.m_bGeoAnchor && qFirst.m_bGeoAnchor
					&& ( qCheck.m_fGeoLatitude!=qFirst.m_fGeoLatitude
						|| qCheck.m_fGeoLongitude!=qFirst.m_fGeoLongitude ) ) ) // some geodist cases

			return false;

		// filters must be the same too
		assert ( qCheck.m_dFilters.GetLength ()==qFirst.m_dFilters.GetLength () );
		assert ( qCheck.m_dFilterTree.GetLength ()==qFirst.m_dFilterTree.GetLength () );
		ARRAY_FOREACH ( i, qCheck.m_dFilters )
		{
			if ( qCheck.m_dFilters[i]!=qFirst.m_dFilters[i] )
				return false;
		}
		ARRAY_FOREACH ( i, qCheck.m_dFilterTree )
		{
			if ( qCheck.m_dFilterTree[i]!=qFirst.m_dFilterTree[i] )
				return false;
		}
	}
	return true;
}

// lock local indexes invoked in query
// Fails if an index is absent and this is not allowed
bool SearchHandler_c::RLockInvokedIndexes()
{
	// if unexistent allowed, short flow
	if ( m_dQueries[m_iStart].m_bIgnoreNonexistentIndexes )
	{
		ARRAY_FOREACH ( i, m_dLocal )
			if ( !m_dLocked.AddRLocked ( m_dLocal[i].m_sName ) )
				m_dLocal.Remove ( i-- );
		return true;
	}

	// _build the list of non-existent
	StringBuilder_c sFailed (", ");
	for ( const auto & dLocal : m_dLocal )
		if ( !m_dLocked.AddRLocked ( dLocal.m_sName ) )
			sFailed << dLocal.m_sName;

	// no absent indexes, viola!
	if ( sFailed.IsEmpty ())
		return true;

	// report failed for each result
	for ( auto i = m_iStart; i<=m_iEnd; ++i )
		m_dResults[i].m_sError.SetSprintf ( "unknown local index(es) '%s' in search request", sFailed.cstr() );

	return false;
}

void SearchHandler_c::UniqLocals()
{
	m_dLocal.Sort ( TaggedLocalSorter_fn () );
	int iSrc = 1, iDst = 1;
	while ( iSrc<m_dLocal.GetLength () )
	{
		if ( m_dLocal[iDst - 1].m_sName==m_dLocal[iSrc].m_sName )
			++iSrc;
		else
			m_dLocal[iDst++] = m_dLocal[iSrc++];
	}
	m_dLocal.Resize ( iDst );
	m_dLocal.Sort ( bind ( &LocalIndex_t::m_iOrderTag ) ); // keep initial order of locals
}


void SearchHandler_c::CalcTimeStats ( int64_t tmCpu, int64_t tmSubset, int iStart, int iEnd, const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent )
{
	// in multi-queue case (1 actual call per N queries), just divide overall query time evenly
	// otherwise (N calls per N queries), divide common query time overheads evenly
	const int iQueries = iEnd-iStart+1;
	if ( m_bMultiQueue )
	{
		for ( int iRes=iStart; iRes<=iEnd; ++iRes )
		{
			m_dResults[iRes].m_iQueryTime = (int)( tmSubset/1000/iQueries );
			m_dResults[iRes].m_iRealQueryTime = (int)( tmSubset/1000/iQueries );
			m_dResults[iRes].m_iCpuTime = tmCpu/iQueries;
		}
	} else
	{
		int64_t tmAccountedWall = 0;
		int64_t tmAccountedCpu = 0;
		for ( int iRes=iStart; iRes<=iEnd; ++iRes )
		{
			tmAccountedWall += m_dResults[iRes].m_iQueryTime*1000;
			assert ( ( m_dResults[iRes].m_iCpuTime==0 && m_dResults[iRes].m_iAgentCpuTime==0 ) ||	// all work was done in this thread
				( m_dResults[iRes].m_iCpuTime>0 && m_dResults[iRes].m_iAgentCpuTime==0 ) ||		// children threads work
				( m_dResults[iRes].m_iAgentCpuTime>0 && m_dResults[iRes].m_iCpuTime==0 ) );		// agents work
			tmAccountedCpu += m_dResults[iRes].m_iCpuTime;
			tmAccountedCpu += m_dResults[iRes].m_iAgentCpuTime;
		}
		// whether we had work done in children threads (dist_threads>1) or in agents
		bool bExternalWork = tmAccountedCpu!=0;

		int64_t tmDeltaWall = ( tmSubset - tmAccountedWall ) / iQueries;

		for ( int iRes=iStart; iRes<=iEnd; ++iRes )
		{
			m_dResults[iRes].m_iQueryTime += (int)(tmDeltaWall/1000);
			m_dResults[iRes].m_iRealQueryTime = (int)( tmSubset/1000/iQueries );
			m_dResults[iRes].m_iCpuTime = tmCpu/iQueries;
			if ( bExternalWork )
				m_dResults[iRes].m_iCpuTime += tmAccountedCpu;
		}

		// correct per-index stats from agents
		int iTotalSuccesses = 0;
		for ( int iRes=iStart; iRes<=iEnd; ++iRes )
			iTotalSuccesses += m_dResults[iRes].m_iSuccesses;

		int nValidDistrIndexes = 0;
		for ( const auto &tDistrStat : dDistrServedByAgent )
			for ( int iQuery=iStart; iQuery<=iEnd; ++iQuery )
				if ( tDistrStat.m_dStats[iQuery-iStart].m_iSuccesses )
				{
					++nValidDistrIndexes;
					break;
				}

		if ( iTotalSuccesses && nValidDistrIndexes )
			for ( auto &tDistrStat : dDistrServedByAgent )
				for ( int iQuery=iStart; iQuery<=iEnd; ++iQuery )
				{
					QueryStat_t & tStat = tDistrStat.m_dStats[iQuery-iStart];
					int64_t tmDeltaWallAgent = ( tmSubset - tmAccountedWall ) * tStat.m_iSuccesses / ( iTotalSuccesses*nValidDistrIndexes );
					tStat.m_uQueryTime += (int)(tmDeltaWallAgent/1000);
				}

		int nValidLocalIndexes = 0;
		for ( const auto & dQueryIndexStat : m_dQueryIndexStats )
			for ( int iQuery=iStart; iQuery<=iEnd; ++iQuery )
				if ( dQueryIndexStat.m_dStats[iQuery-iStart].m_iSuccesses )
				{
					++nValidLocalIndexes;
					break;
				}

		if ( iTotalSuccesses && nValidLocalIndexes )
			for ( auto &dQueryIndexStat : m_dQueryIndexStats )
				for ( int iQuery=iStart; iQuery<=iEnd; iQuery++ )
				{
					QueryStat_t & tStat = dQueryIndexStat.m_dStats[iQuery-iStart];
					int64_t tmDeltaWallLocal = ( tmSubset - tmAccountedWall ) * tStat.m_iSuccesses / ( iTotalSuccesses*nValidLocalIndexes );
					tStat.m_uQueryTime += (int)(tmDeltaWallLocal/1000);
				}

		// don't forget to add this to stats
		if ( bExternalWork )
			tmCpu += tmAccountedCpu;
	}
}


void SearchHandler_c::CalcPerIndexStats ( int iStart, int iEnd, const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent ) const
{
	// calculate per-index stats
	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		// a little of durty casting: from ServedDesc_t* to ServedIndex_c*
		// in order to save statistics.
		auto pServed = ( ServedIndex_c * ) m_dLocked.Get ( m_dLocal[iLocal].m_sName );
		for ( int iQuery=iStart; iQuery<=iEnd; ++iQuery )
		{
			QueryStat_t & tStat = m_dQueryIndexStats[iLocal].m_dStats[iQuery-iStart];
			if ( !tStat.m_iSuccesses )
				continue;

			pServed->AddQueryStat ( tStat.m_uFoundRows, tStat.m_uQueryTime );

			for ( auto &tDistr : dDistrServedByAgent )
			{
				if ( tDistr.m_dLocalNames.Contains ( m_dLocal[iLocal].m_sName ) )
				{
					tDistr.m_dStats[iQuery - iStart].m_uQueryTime += tStat.m_uQueryTime;
					tDistr.m_dStats[iQuery - iStart].m_uFoundRows += tStat.m_uFoundRows;
					tDistr.m_dStats[iQuery - iStart].m_iSuccesses++;
				}
			}
		}
	}

	for ( auto &tDistr : dDistrServedByAgent )
	{
		auto pServedDistIndex = GetDistr ( tDistr.m_sIndex );
		if ( pServedDistIndex )
			for ( int iQuery=iStart; iQuery<=iEnd; ++iQuery )
			{
				auto & tStat = tDistr.m_dStats[iQuery-iStart];
				if ( !tStat.m_iSuccesses )
					continue;

				pServedDistIndex->AddQueryStat ( tStat.m_uFoundRows, tStat.m_uQueryTime );
			}
	}
}


void SearchHandler_c::CalcGlobalStats ( int64_t tmCpu, int64_t tmSubset, int64_t tmLocal, int iStart, int iEnd, const CSphIOStats & tIO, const VecRefPtrsAgentConn_t & dRemotes ) const
{
	const int iQueries = iEnd-iStart+1;
	g_tStats.m_iQueries += iQueries;
	g_tStats.m_iQueryTime += tmSubset;
	g_tStats.m_iQueryCpuTime += tmCpu;
	if ( dRemotes.GetLength() )
	{
		int64_t tmWait = 0;
		for ( const AgentConn_t * pAgent : dRemotes )
			tmWait += pAgent->m_iWaited;

		// do *not* count queries to dist indexes w/o actual remote agents
		++g_tStats.m_iDistQueries;
		g_tStats.m_iDistWallTime += tmSubset;
		g_tStats.m_iDistLocalTime += tmLocal;
		g_tStats.m_iDistWaitTime += tmWait;
	}
	g_tStats.m_iDiskReads += tIO.m_iReadOps;
	g_tStats.m_iDiskReadTime += tIO.m_iReadTime;
	g_tStats.m_iDiskReadBytes += tIO.m_iReadBytes;
}


void SearchHandler_c::BuildIndexList ( int iStart, int iEnd, int & iDivideLimits, VecRefPtrsAgentConn_t & dRemotes, CSphVector<DistrServedByAgent_t> & dDistrServedByAgent )
{
	const CSphQuery & tFirst = m_dQueries[iStart];

	int iTagsCount = 0, iTagStep = iEnd - iStart + 1;
	m_dLocal.Reset();

	// they're all local, build the list
	if ( tFirst.m_sIndexes=="*" )
	{
		// search through all local indexes
		for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next(); )
		{
			if ( !it.Get() )
				continue;
			auto &dLocal = m_dLocal.Add ();
			dLocal.m_sName = it.GetName ();
			dLocal.m_iOrderTag = iTagsCount;
			dLocal.m_iWeight = GetIndexWeight ( it.GetName (), tFirst.m_dIndexWeights, 1 );
			dLocal.m_iMass = ServedDescRPtr_c ( it.Get() )->m_iMass;
			iTagsCount += iTagStep;
		}
	} else
	{
		StrVec_t dIdxNames;
		// search through specified local indexes
		ParseIndexList ( tFirst.m_sIndexes, dIdxNames );

		int iDistCount = 0;
		bool bDivideRemote = false;

		for ( const auto& sIndex : dIdxNames )
		{
			auto pDist = GetDistr ( sIndex );
			if ( !pDist )
			{
				auto &dLocal = m_dLocal.Add ();
				dLocal.m_sName = sIndex;
				dLocal.m_iOrderTag = iTagsCount;
				dLocal.m_iWeight = GetIndexWeight ( sIndex, tFirst.m_dIndexWeights, 1 );
				dLocal.m_iMass = GetIndexMass ( sIndex );
				iTagsCount += iTagStep;
			} else
			{
				++iDistCount;
				int iWeight = GetIndexWeight ( sIndex, tFirst.m_dIndexWeights, -1 );
				auto & tDistrStat = dDistrServedByAgent.Add();
				tDistrStat.m_sIndex = sIndex;
				tDistrStat.m_dStats.Resize ( iEnd-iStart+1 );
				tDistrStat.m_dStats.ZeroVec();
				for ( auto * pAgent : pDist->m_dAgents )
				{
					tDistrStat.m_dAgentIds.Add ( dRemotes.GetLength() );
					auto * pConn = new AgentConn_t;
					pConn->SetMultiAgent ( pAgent );
					pConn->m_iStoreTag = iTagsCount;
					pConn->m_iWeight = iWeight;
					pConn->m_iMyConnectTimeoutMs = pDist->m_iAgentConnectTimeoutMs;
					pConn->m_iMyQueryTimeoutMs = pDist->m_iAgentQueryTimeoutMs;
					dRemotes.Add ( pConn );
					iTagsCount += iTagStep;
				}

				ARRAY_FOREACH ( j, pDist->m_dLocal )
				{
					const CSphString& sLocalAgent = pDist->m_dLocal[j];
					tDistrStat.m_dLocalNames.Add ( sLocalAgent );
					auto &dLocal = m_dLocal.Add ();
					dLocal.m_sName = sLocalAgent;
					dLocal.m_iOrderTag = iTagsCount;
					if ( iWeight!=-1 )
						dLocal.m_iWeight = iWeight;
					dLocal.m_iMass = GetIndexMass ( sLocalAgent );
					dLocal.m_sParentIndex = sIndex;
					iTagsCount += iTagStep;
					m_dLocal.Last().m_iMass = GetIndexMass ( pDist->m_dLocal[j].cstr() );
				}

				bDivideRemote |= pDist->m_bDivideRemoteRanges;
			}
		}

		// set remote divider
		if ( bDivideRemote )
		{
			if ( iDistCount==1 )
				iDivideLimits = dRemotes.GetLength();
			else
			{
				for ( int iRes=iStart; iRes<=iEnd; ++iRes )
					m_dResults[iRes].m_sWarning.SetSprintf ( "distributed multi-index query '%s' doesn't support divide_remote_ranges", tFirst.m_sIndexes.cstr() );
			}
		}

		// eliminate local dupes that come from distributed indexes
		if ( !dRemotes.IsEmpty () && !m_dLocal.IsEmpty() )
			UniqLocals();
	}

	for ( int iRes=iStart; iRes<=iEnd; iRes++ )
		m_dResults[iRes].m_dTag2Docstore.Resize(iTagsCount);
}

// query info - render query into the view
struct QueryInfo_t : public TaskInfo_t
{
	DECLARE_RENDER( QueryInfo_t );

	// actually it is 'virtually hazard'. Don't care about query* itself, however later in dtr of Searchandler_t
	// will work with refs to members of it's m_dQueries and retire or whole vec.
	std::atomic<const CSphQuery *> m_pHazardQuery;
};

DEFINE_RENDER ( QueryInfo_t )
{
	auto & tInfo = *(QueryInfo_t *) pSrc;
	dDst.m_sChain << (int) tInfo.m_eType << ":Query ";
	hazard::Guard_c tGuard;
	auto pQuery = tGuard.Protect ( tInfo.m_pHazardQuery );
	if ( pQuery && myinfo::GetProto()!=Proto_e::MYSQL41 ) // cheat: for mysql query not used, so will not copy it then
		dDst.m_pQuery = new CSphQuery ( *pQuery );
}

// one ore more queries against one and same set of indexes
void SearchHandler_c::RunSubset ( int iStart, int iEnd )
{
	// we've own scoped context here
	auto pQueryInfo = new QueryInfo_t;
	pQueryInfo->m_pHazardQuery.store ( &m_dQueries[iStart], std::memory_order_release );
	ScopedInfo_T<QueryInfo_t> pTlsQueryInfo ( pQueryInfo );

	m_iStart = iStart;
	m_iEnd = iEnd;

	// all my stats
	int64_t tmSubset = -sphMicroTimer();
	int64_t tmLocal = 0;
	int64_t tmCpu = -sphTaskCpuTimer ();

	ESphQueryState eOldState = SPH_QSTATE_UNKNOWN;
	if ( m_pProfile )
		eOldState = m_pProfile->m_eState;


	// prepare for descent
	const CSphQuery & tFirst = m_dQueries[iStart];

	for ( int iRes=iStart; iRes<=iEnd; ++iRes )
		m_dResults[iRes].m_iSuccesses = 0;

	if ( iStart==iEnd && m_pProfile )
	{
		m_dResults[iStart].m_pProfile = m_pProfile;
		m_tHook.SetProfiler ( m_pProfile );
	}

	// check for facets
	m_bFacetQueue = iEnd>iStart;
	for ( int iCheck=iStart+1; iCheck<=iEnd && m_bFacetQueue; ++iCheck )
		if ( !m_dQueries[iCheck].m_bFacet )
			m_bFacetQueue = false;

	m_bMultiQueue = m_bFacetQueue || CheckMultiQuery ( iStart, iEnd );

	////////////////////////////
	// build local indexes list
	////////////////////////////
	VecRefPtrsAgentConn_t dRemotes;
	CSphVector<DistrServedByAgent_t> dDistrServedByAgent;
	int iDivideLimits = 1;
	BuildIndexList ( iStart, iEnd, iDivideLimits, dRemotes, dDistrServedByAgent );

	if ( !RLockInvokedIndexes () )
		return;

	// at this point m_dLocal contains list of valid local indexes (i.e., existing ones),
	// and these indexes are also rlocked and available by calling m_dLocked.Get()

	// sanity check
	if ( dRemotes.IsEmpty() && m_dLocal.IsEmpty() )
	{
		const char * sIndexType = ( dRemotes.GetLength() ? "indexes" : "local indexes" );
		for ( int iRes=iStart; iRes<=iEnd; ++iRes )
			m_dResults[iRes].m_sError.SetSprintf ( "no enabled %s to search", sIndexType );
		return;
	}

 	// select lists must have no expressions
	if ( m_bMultiQueue )
		m_bMultiQueue = AllowsMulti ( iStart, iEnd );

	assert ( !m_bFacetQueue || AllowsMulti ( iStart, iEnd ) );
	if ( !m_bMultiQueue )
		m_bFacetQueue = false;

	///////////////////////////////////////////////////////////
	// main query loop (with multiple retries for distributed)
	///////////////////////////////////////////////////////////

	// connect to remote agents and query them, if required
	CSphScopedPtr<SearchRequestBuilder_c> tReqBuilder { nullptr };
	CSphRefcountedPtr<RemoteAgentsObserver_i> tReporter { nullptr };
	CSphScopedPtr<ReplyParser_i> tParser { nullptr };
	if ( !dRemotes.IsEmpty() )
	{
		if ( m_pProfile )
			m_pProfile->Switch ( SPH_QSTATE_DIST_CONNECT );

		tReqBuilder = new SearchRequestBuilder_c ( m_dQueries, iStart, iEnd, iDivideLimits );
		tParser = new SearchReplyParser_c ( iStart, iEnd );
		tReporter = GetObserver();

		// run remote queries. tReporter will tell us when they're finished.
		// also blackholes will be removed from this flow of remotes.
		ScheduleDistrJobs ( dRemotes, tReqBuilder.Ptr (),
			tParser.Ptr (),
			tReporter, tFirst.m_iRetryCount, tFirst.m_iRetryDelay );
	}

	/////////////////////
	// run local queries
	//////////////////////

	// while the remote queries are running, do local searches
	if ( m_dLocal.GetLength() )
	{
		SetupLocalDF ( iStart, iEnd );

		if ( m_pProfile )
			m_pProfile->Switch ( SPH_QSTATE_LOCAL_SEARCH );

		tmLocal = -sphMicroTimer();
		tmCpu -= sphTaskCpuTimer ();
		RunLocalSearches();
		tmCpu += sphTaskCpuTimer ();
		tmLocal += sphMicroTimer();
	}

	///////////////////////
	// poll remote queries
	///////////////////////

	if ( !dRemotes.IsEmpty() )
	{
		if ( m_pProfile )
			m_pProfile->Switch ( SPH_QSTATE_DIST_WAIT );

		bool bDistDone = false;
		while ( !bDistDone )
		{
			// don't forget to check incoming replies after send was over
			bDistDone = tReporter->IsDone();
			if ( !bDistDone )
				tReporter->WaitChanges (); /// wait one or more remote queries to complete

			ARRAY_FOREACH ( iAgent, dRemotes )
			{
				AgentConn_t * pAgent = dRemotes[iAgent];
				assert ( !pAgent->IsBlackhole () ); // must not be any blacknole here.

				if ( !pAgent->m_bSuccess )
					continue;

				sphLogDebugv ( "agent %d, state %s, order %d, sock %d", iAgent, pAgent->StateName(), pAgent->m_iStoreTag, pAgent->m_iSock );

				DistrServedByAgent_t * pDistr = nullptr;
				for ( auto &tDistr : dDistrServedByAgent )
					if ( tDistr.m_dAgentIds.Contains ( iAgent ) )
					{
						pDistr = &tDistr;
						break;
					}
				assert ( pDistr );

				int iOrderTag = pAgent->m_iStoreTag;
				// merge this agent's results
				for ( int iRes=iStart; iRes<=iEnd; ++iRes )
				{
					auto pResult = ( cSearchResult * ) pAgent->m_pResult.Ptr ();
					if ( !pResult )
						continue;

					const auto &tRemoteResult = pResult->m_dResults[iRes - iStart];

					// copy errors or warnings
					if ( !tRemoteResult.m_sError.IsEmpty() )
						m_dFailuresSet[iRes].SubmitEx ( tFirst.m_sIndexes.cstr(), NULL,
							"agent %s: remote query error: %s",
							pAgent->m_tDesc.GetMyUrl().cstr(), tRemoteResult.m_sError.cstr() );
					if ( !tRemoteResult.m_sWarning.IsEmpty() )
						m_dFailuresSet[iRes].SubmitEx ( tFirst.m_sIndexes.cstr(), NULL,
							"agent %s: remote query warning: %s",
							pAgent->m_tDesc.GetMyUrl().cstr(), tRemoteResult.m_sWarning.cstr() );

					if ( tRemoteResult.m_iSuccesses<=0 )
						continue;

					AggrResult_t & tRes = m_dResults[iRes];
					tRes.m_iSuccesses++;
					tRes.m_tSchema = tRemoteResult.m_tSchema;

					assert ( !tRes.m_dTag2Docstore[iOrderTag + iRes - iStart].m_pDocstore );

					tRes.m_dMatches.Reserve ( tRes.m_dMatches.GetLength() + tRemoteResult.m_dMatches.GetLength() );
					ARRAY_FOREACH ( i, tRemoteResult.m_dMatches )
					{
						tRes.m_dMatches.Add();
						tRemoteResult.m_tSchema.CloneWholeMatch ( tRes.m_dMatches.Last(), tRemoteResult.m_dMatches[i] );
						tRes.m_dMatches.Last().m_iTag = ( iOrderTag + iRes - iStart ) | 0x80000000;
					}

					tRes.m_pBlobPool = nullptr;
					tRes.m_dTag2Docstore[iOrderTag+iRes-iStart].m_pDocstore = nullptr;
					tRes.m_dMatchCounts.Add ( tRemoteResult.m_dMatches.GetLength() );
					tRes.m_dSchemas.Add ( tRemoteResult.m_tSchema );
					// note how we do NOT add per-index weight here

					// merge this agent's stats
					tRes.m_iTotalMatches += tRemoteResult.m_iTotalMatches;
					tRes.m_iQueryTime += tRemoteResult.m_iQueryTime;
					tRes.m_iAgentCpuTime += tRemoteResult.m_iCpuTime;
					tRes.m_tAgentIOStats.Add ( tRemoteResult.m_tIOStats );
					tRes.m_iAgentPredictedTime += tRemoteResult.m_iPredictedTime;
					tRes.m_iAgentFetchedDocs += tRemoteResult.m_iAgentFetchedDocs;
					tRes.m_iAgentFetchedHits += tRemoteResult.m_iAgentFetchedHits;
					tRes.m_iAgentFetchedSkips += tRemoteResult.m_iAgentFetchedSkips;
					tRes.m_bHasPrediction |= ( m_dQueries[iRes].m_iMaxPredictedMsec>0 );

					if ( pDistr )
					{
						pDistr->m_dStats[iRes-iStart].m_uQueryTime += tRemoteResult.m_iQueryTime;
						pDistr->m_dStats[iRes-iStart].m_uFoundRows += tRemoteResult.m_iTotalMatches;
						pDistr->m_dStats[iRes-iStart].m_iSuccesses++;
					}

					// merge this agent's words
					MergeWordStats ( tRes, tRemoteResult.m_hWordStats );
				}

				// dismissed
				if ( pAgent->m_pResult )
					pAgent->m_pResult->Reset ();
				pAgent->m_bSuccess = false;
				pAgent->m_sFailure = "";
			}
		} // while ( !bDistDone )
	} // if ( bDist && dRemotes.GetLength() )

	// submit failures from failed agents
	// copy timings from all agents
	if ( !dRemotes.IsEmpty() )
	{
		for ( const AgentConn_t * pAgent : dRemotes )
		{
			assert ( !pAgent->IsBlackhole () ); // must not be any blacknole here.

			for ( int j=iStart; j<=iEnd; ++j )
			{
				assert ( pAgent->m_iWall>=0 );
				m_dAgentTimes[j].Add ( ( pAgent->m_iWall ) / ( 1000 * ( iEnd-iStart+1 ) ) );
			}

			if ( !pAgent->m_bSuccess && !pAgent->m_sFailure.IsEmpty() )
				for ( int j=iStart; j<=iEnd; j++ )
					m_dFailuresSet[j].SubmitEx ( tFirst.m_sIndexes.cstr(), nullptr, "agent %s: %s",
						pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
		}
	}

	/////////////////////
	// merge all results
	/////////////////////

	if ( m_pProfile )
		m_pProfile->Switch ( SPH_QSTATE_AGGREGATE );

	CSphIOStats tIO;

	for ( int iRes=iStart; iRes<=iEnd; ++iRes )
	{
		AggrResult_t & tRes = m_dResults[iRes];
		const CSphQuery & tQuery = m_dQueries[iRes];
		sph::StringSet hExtra;
		if ( !m_dExtraSchemas.IsEmpty () )
			for ( const StrVec_t & dExtraSet : m_dExtraSchemas )
				for ( const CSphString & sExtra : dExtraSet )
					hExtra.Add ( sExtra );


		// minimize sorters needs these pointers
		tIO.Add ( tRes.m_tIOStats );

		// if there were no successful searches at all, this is an error
		if ( !tRes.m_iSuccesses )
		{
			StringBuilder_c sFailures;
			m_dFailuresSet[iRes].BuildReport ( sFailures );
			sFailures.MoveTo (tRes.m_sError);
			continue;
		}

		// minimize schema and remove dupes
		// assuming here ( tRes.m_tSchema==tRes.m_dSchemas[0] )
		const CSphFilterSettings * pAggrFilter = nullptr;
		if ( m_bMaster && !tQuery.m_tHaving.m_sAttrName.IsEmpty() )
			pAggrFilter = &tQuery.m_tHaving;

		const CSphVector<CSphQueryItem> & dItems = ( tQuery.m_dRefItems.GetLength() ? tQuery.m_dRefItems : tQuery.m_dItems );

		if ( tRes.m_iSuccesses>1 || dItems.GetLength() || pAggrFilter )
		{

			if ( m_bMaster && tRes.m_iSuccesses && dItems.GetLength() && tQuery.m_sGroupBy.IsEmpty() && tRes.m_dMatches.GetLength()==0 )
			{
				for ( auto& dItem : dItems )
				{
					if ( dItem.m_sExpr=="count(*)" || ( dItem.m_sExpr=="@distinct" ) )
						tRes.m_dZeroCount.Add ( dItem.m_sAlias );
				}
			}

			bool bOk = MinimizeAggrResult ( tRes, tQuery, !m_dLocal.IsEmpty(), hExtra, m_pProfile, pAggrFilter, m_bFederatedUser, m_bMaster, dRemotes );

			if ( !bOk )
			{
				tRes.m_iSuccesses = 0;
				return; // FIXME? really return, not just continue?
			}
		}

		if ( !m_dFailuresSet[iRes].IsEmpty() )
		{
			StringBuilder_c sFailures;
			m_dFailuresSet[iRes].BuildReport ( sFailures );
			sFailures.MoveTo ( tRes.m_sWarning );
		}

		////////////
		// finalize
		////////////

		tRes.m_iOffset = Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
		tRes.m_iCount = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );
		tRes.m_iCount = Max ( Min ( tRes.m_iCount, tRes.m_dMatches.GetLength()-tRes.m_iOffset ), 0 );
	}

	/////////////////////////////////
	// functions on a table argument
	/////////////////////////////////

	for ( int iRes=iStart; iRes<=iEnd; ++iRes )
	{
		AggrResult_t & tRes = m_dResults[iRes];
		ISphTableFunc * pTableFunc = m_dTables[iRes];

		// FIXME! log such queries properly?
		if ( pTableFunc )
		{
			if ( m_pProfile )
				m_pProfile->Switch ( SPH_QSTATE_TABLE_FUNC );
			if ( !pTableFunc->Process ( &tRes, tRes.m_sError ) )
				tRes.m_iSuccesses = 0;
		}
	}

	/////////
	// stats
	/////////

	tmSubset += sphMicroTimer();
	tmCpu += sphTaskCpuTimer();

	CalcTimeStats ( tmCpu, tmSubset, iStart, iEnd, dDistrServedByAgent );
	CalcPerIndexStats ( iStart, iEnd, dDistrServedByAgent );
	CalcGlobalStats ( tmCpu, tmSubset, tmLocal, iStart, iEnd, tIO, dRemotes );

	if ( m_pProfile )
		m_pProfile->Switch ( eOldState );
}


bool CheckCommandVersion ( WORD uVer, WORD uDaemonVersion, ISphOutputBuffer & tOut )
{
	if ( ( uVer>>8)!=( uDaemonVersion>>8) )
	{
		SendErrorReply ( tOut, "major command version mismatch (expected v.%d.x, got v.%d.%d)",
			uDaemonVersion>>8, uVer>>8, uVer&0xff );
		return false;
	}
	if ( uVer>uDaemonVersion )
	{
		SendErrorReply ( tOut, "client version is higher than daemon version (client is v.%d.%d, daemon is v.%d.%d)",
			uVer>>8, uVer&0xff, uDaemonVersion>>8, uDaemonVersion&0xff );
		return false;
	}
	return true;
}

bool IsMaxedOut ()
{
	if ( myinfo::IsVIP() )
		return false;

	if ( g_iThdQueueMax!=0 )
		return GlobalWorkPool()->Works() > g_iThdQueueMax;

	if ( g_iMaxConnection!=0 )
		return myinfo::CountClients() > g_iMaxConnection;

	return false;
}

void HandleCommandSearch ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	MEMORY ( MEM_API_SEARCH );

	if ( !CheckCommandVersion ( uVer, VER_COMMAND_SEARCH, tOut ) )
		return;

	const WORD MIN_VERSION = 0x119;
	if ( uVer<MIN_VERSION )
	{
		SendErrorReply ( tOut, "client version is too old; upgrade your client (client is v.%d.%d, min is v.%d.%d)", uVer>>8, uVer&0xff, MIN_VERSION>>8, MIN_VERSION&0xff );
		return;
	}

	int iMasterVer = tReq.GetInt();
	if ( iMasterVer<0 || iMasterVer>VER_COMMAND_SEARCH_MASTER )
	{
		SendErrorReply ( tOut, "master-agent version mismatch; update me first, then update master!" );
		return;
	}
	WORD uMasterVer { WORD (iMasterVer) };
	bool bAgentMode = ( uMasterVer>0 );

	// parse request
	int iQueries = tReq.GetDword ();

	if ( g_iMaxBatchQueries>0 && ( iQueries<=0 || iQueries>g_iMaxBatchQueries ) )
	{
		SendErrorReply ( tOut, "bad multi-query count %d (must be in 1..%d range)", iQueries, g_iMaxBatchQueries );
		return;
	}

	SearchHandler_c tHandler ( iQueries, nullptr, QUERY_API, ( iMasterVer==0 ) );
	for ( auto &dQuery : tHandler.m_dQueries )
		if ( !ParseSearchQuery ( tReq, tOut, dQuery, uVer, uMasterVer ) )
			return;

	if ( !tHandler.m_dQueries.IsEmpty() )
	{
		QueryType_e eQueryType = tHandler.m_dQueries[0].m_eQueryType;

#ifndef NDEBUG
		// we assume that all incoming queries have the same type
		for ( const auto & i: tHandler.m_dQueries )
			assert ( i.m_eQueryType==eQueryType );
#endif

		QueryParser_i * pParser {nullptr};
		if ( eQueryType==QUERY_JSON )
			pParser = sphCreateJsonQueryParser();
		else
			pParser = sphCreatePlainQueryParser();

		assert ( pParser );
		tHandler.SetQueryParser ( pParser, eQueryType );

		const CSphQuery & q = tHandler.m_dQueries[0];
		myinfo::SetThreadInfo ( R"(api-search query="%s" comment="%s" index="%s")",
				q.m_sQuery.scstr (), q.m_sComment.scstr (), q.m_sIndexes.scstr () );
	}

	// run queries, send response
	tHandler.RunQueries();

	auto tReply = APIAnswer ( tOut, VER_COMMAND_SEARCH );
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
		SendResult ( uVer, tOut, &tHandler.m_dResults[i], bAgentMode, tHandler.m_dQueries[i], uMasterVer );


	int64_t iTotalPredictedTime = 0;
	int64_t iTotalAgentPredictedTime = 0;
	for ( const auto& dResult : tHandler.m_dResults )
	{
		iTotalPredictedTime += dResult.m_iPredictedTime;
		iTotalAgentPredictedTime += dResult.m_iAgentPredictedTime;
	}

	g_tStats.m_iPredictedTime += iTotalPredictedTime;
	g_tStats.m_iAgentPredictedTime += iTotalAgentPredictedTime;

	ScWL_t dLastMetaLock ( g_tLastMetaLock );
	g_tLastMeta = tHandler.m_dResults[tHandler.m_dResults.GetLength () - 1];
}

//////////////////////////////////////////////////////////////////////////
// TABLE FUNCTIONS
//////////////////////////////////////////////////////////////////////////

// table functions take an arbitrary result set as their input,
// and return a new, processed, (completely) different one as their output
//
// 1st argument should be the input result set, but a table function
// can optionally take and handle more arguments
//
// table function can completely (!) change the result set
// including (!) the schema
//
// for now, only builtin table functions are supported
// UDFs are planned when the internal call interface is stabilized

#define LOC_ERROR(_msg) { sError = _msg; return false; }
#define LOC_ERROR1(_msg,_arg1) { sError.SetSprintf ( _msg, _arg1 ); return false; }

class CSphTableFuncRemoveRepeats : public ISphTableFunc
{
protected:
	CSphString	m_sCol;
	int			m_iOffset;
	int			m_iLimit;

public:
	virtual bool ValidateArgs ( const StrVec_t & dArgs, const CSphQuery &, CSphString & sError )
	{
		if ( dArgs.GetLength()!=3 )
			LOC_ERROR ( "REMOVE_REPEATS() requires 4 arguments (result_set, column, offset, limit)" );
		if ( !isdigit ( *dArgs[1].cstr() ) )
			LOC_ERROR ( "REMOVE_REPEATS() argument 3 (offset) must be integer" );
		if ( !isdigit ( *dArgs[2].cstr() ) )
			LOC_ERROR ( "REMOVE_REPEATS() argument 4 (limit) must be integer" );

		m_sCol = dArgs[0];
		m_iOffset = atoi ( dArgs[1].cstr() );
		m_iLimit = atoi ( dArgs[2].cstr() );

		if ( !m_iLimit )
			LOC_ERROR ( "REMOVE_REPEATS() argument 4 (limit) must be greater than 0" );
		return true;
	}


	virtual bool Process ( AggrResult_t * pResult, CSphString & sError )
	{
		assert ( pResult );

		CSphSwapVector<CSphMatch> & dMatches = pResult->m_dMatches;
		if ( !dMatches.GetLength() )
			return true;

		const CSphColumnInfo * pCol = pResult->m_tSchema.GetAttr ( m_sCol.cstr() );
		if ( !pCol )
			LOC_ERROR1 ( "REMOVE_REPEATS() argument 2 (column %s) not found in result set", m_sCol.cstr() );

		ESphAttr t = pCol->m_eAttrType;
		if ( t!=SPH_ATTR_INTEGER && t!=SPH_ATTR_BIGINT && t!=SPH_ATTR_TOKENCOUNT && t!=SPH_ATTR_STRINGPTR && t!=SPH_ATTR_STRING )
			LOC_ERROR1 ( "REMOVE_REPEATS() argument 2 (column %s) must be of INTEGER, BIGINT, or STRINGPTR type", m_sCol.cstr() );

		// we need to initialize the "last seen" value with a key that
		// is guaranteed to be different from the 1st match that we will scan
		// hence (val-1) for scalars, and NULL for strings
		SphAttr_t iLastValue = ( t==SPH_ATTR_STRING || t==SPH_ATTR_STRINGPTR )
			? 0
			: ( dMatches [ pResult->m_iOffset ].GetAttr ( pCol->m_tLocator ) - 1 );

		// LIMIT N,M clause must be applied before (!) table function
		// so we scan source matches N to N+M-1
		//
		// within those matches, we filter out repeats in a given column,
		// skip first m_iOffset eligible ones, and emit m_iLimit more
		int iOutPos = 0;
		for ( int i=pResult->m_iOffset; i<Min ( dMatches.GetLength(), pResult->m_iOffset+pResult->m_iCount ); i++ )
		{
			// get value, skip repeats
			SphAttr_t iCur = dMatches[i].GetAttr ( pCol->m_tLocator );
			if ( iCur==iLastValue )
				continue;

			if ( iCur && iLastValue && t==SPH_ATTR_STRINGPTR )
			{
				const BYTE * a = (const BYTE*) iCur;
				const BYTE * b = (const BYTE*) iLastValue;
				int iLen1 = sphUnpackPtrAttr ( a, &a );
				int iLen2 = sphUnpackPtrAttr ( b, &b );
				if ( iLen1==iLen2 && memcmp ( a, b, iLen1 )==0 )
					continue;
			}

			iLastValue = iCur;

			// skip eligible rows according to tablefunc offset
			if ( m_iOffset>0 )
			{
				m_iOffset--;
				continue;
			}

			// emit!
			if ( iOutPos!=i )
				Swap ( dMatches[iOutPos], dMatches[i] );

			// break if we reached the tablefunc limit
			if ( ++iOutPos==m_iLimit )
				break;
		}

		// adjust the result set limits
		pResult->ClampMatches ( iOutPos, true );
		pResult->m_iOffset = 0;
		pResult->m_iCount = dMatches.GetLength();
		return true;
	}
};


ISphTableFunc * CreateRemoveRepeats()
{
	return new CSphTableFuncRemoveRepeats;
}

#undef LOC_ERROR1
#undef LOC_ERROR

//////////////////////////////////////////////////////////////////////////
// SQL PARSER
//////////////////////////////////////////////////////////////////////////
// FIXME? verify or generate these automatically somehow?
static const char * g_dSqlStmts[] =
{
	"parse_error", "dummy", "select", "insert", "replace", "delete", "show_warnings",
	"show_status", "show_meta", "set", "begin", "commit", "rollback", "call",
	"desc", "show_tables", "create_table", "create_table_like", "drop_table", "show_create_table", "update", "create_func",
	"drop_func", "attach_index", "flush_rtindex", "flush_ramchunk", "show_variables", "truncate_rtindex",
	"select_sysvar", "show_collation", "show_character_set", "optimize_index", "show_agent_status",
	"show_index_status", "show_profile", "alter_add", "alter_drop", "show_plan",
	"select_dual", "show_databases", "create_plugin", "drop_plugin", "show_plugins", "show_threads",
	"facet", "alter_reconfigure", "show_index_settings", "flush_index", "reload_plugins", "reload_index",
	"flush_hostnames", "flush_logs", "reload_indexes", "sysfilters", "debug", "alter_killlist_target",
	"alter_index_settings", "join_cluster", "cluster_create", "cluster_delete", "cluster_index_add",
	"cluster_index_delete", "cluster_update", "explain", "import_table"
};


STATIC_ASSERT ( sizeof(g_dSqlStmts)/sizeof(g_dSqlStmts[0])==STMT_TOTAL, STMT_DESC_SHOULD_BE_SAME_AS_STMT_TOTAL );

//////////////////////////////////////////////////////////////////////////

#ifdef CMAKE_GENERATED_GRAMMAR
	#include "bissphinxql.h"
#else
	#include "yysphinxql.h"
#endif


class CSphMatchVariant : public CSphMatch
{
public:
	inline static SphAttr_t ToInt ( const SqlInsert_t & tVal )
	{
		switch ( tVal.m_iType )
		{
			case TOK_QUOTED_STRING :	return strtoul ( tVal.m_sVal.cstr(), NULL, 10 ); // FIXME? report conversion error?
			case TOK_CONST_INT:			return int(tVal.m_iVal);
			case TOK_CONST_FLOAT:		return int(tVal.m_fVal); // FIXME? report conversion error
		}
		return 0;
	}

	inline static SphAttr_t ToBigInt ( const SqlInsert_t & tVal )
	{
		switch ( tVal.m_iType )
		{
			case TOK_QUOTED_STRING :	return strtoll ( tVal.m_sVal.cstr(), NULL, 10 ); // FIXME? report conversion error?
			case TOK_CONST_INT:			return tVal.m_iVal;
			case TOK_CONST_FLOAT:		return int64_t(tVal.m_fVal); // FIXME? report conversion error?
		}
		return 0;
	}

	bool SetAttr ( const CSphAttrLocator & tLoc, const SqlInsert_t & tVal, ESphAttr eTargetType )
	{
		switch ( eTargetType )
		{
			case SPH_ATTR_INTEGER:
			case SPH_ATTR_TIMESTAMP:
			case SPH_ATTR_BOOL:
			case SPH_ATTR_TOKENCOUNT:
				CSphMatch::SetAttr ( tLoc, ToInt(tVal) );
				break;
			case SPH_ATTR_BIGINT:
				CSphMatch::SetAttr ( tLoc, ToBigInt(tVal) );
				break;
			case SPH_ATTR_FLOAT:
				if ( tVal.m_iType==TOK_QUOTED_STRING )
					SetAttrFloat ( tLoc, (float)strtod ( tVal.m_sVal.cstr(), NULL ) ); // FIXME? report conversion error?
				else if ( tVal.m_iType==TOK_CONST_INT )
					SetAttrFloat ( tLoc, float(tVal.m_iVal) ); // FIXME? report conversion error?
				else if ( tVal.m_iType==TOK_CONST_FLOAT )
					SetAttrFloat ( tLoc, tVal.m_fVal );
				break;
			case SPH_ATTR_STRING:
			case SPH_ATTR_STRINGPTR:
			case SPH_ATTR_UINT32SET:
			case SPH_ATTR_INT64SET:
			case SPH_ATTR_JSON:
				CSphMatch::SetAttr ( tLoc, 0 );
				break;
			default:
				return false;
		};
		return true;
	}

	inline bool SetDefaultAttr ( const CSphAttrLocator & tLoc, ESphAttr eTargetType )
	{
		SqlInsert_t tVal;
		tVal.m_iType = TOK_CONST_INT;
		tVal.m_iVal = 0;
		return SetAttr ( tLoc, tVal, eTargetType );
	}
};

/////////////////////////////////////////////////////////////////////////////
// EXCERPTS HANDLER
/////////////////////////////////////////////////////////////////////////////

enum eExcerpt_Flags
{
	EXCERPT_FLAG_REMOVESPACES		= 1,			// deprecated
	EXCERPT_FLAG_EXACTPHRASE		= 2,			// deprecated
	EXCERPT_FLAG_SINGLEPASSAGE		= 4,
	EXCERPT_FLAG_USEBOUNDARIES		= 8,
	EXCERPT_FLAG_WEIGHTORDER		= 16,
	EXCERPT_FLAG_QUERY				= 32,			// deprecated
	EXCERPT_FLAG_FORCE_ALL_WORDS	= 64,
	EXCERPT_FLAG_LOAD_FILES			= 128,
	EXCERPT_FLAG_ALLOW_EMPTY		= 256,
	EXCERPT_FLAG_EMIT_ZONES			= 512,
	EXCERPT_FLAG_FILES_SCATTERED	= 1024,
	EXCERPT_FLAG_FORCEPASSAGES		= 2048
};

int PackAPISnippetFlags ( const SnippetQuerySettings_t &q, bool bOnlyScattered = false )
{
	int iRawFlags = q.m_iLimitPassages ? EXCERPT_FLAG_SINGLEPASSAGE : 0;
	iRawFlags |= q.m_bUseBoundaries ? EXCERPT_FLAG_USEBOUNDARIES : 0;
	iRawFlags |= q.m_bWeightOrder ? EXCERPT_FLAG_WEIGHTORDER : 0;
	iRawFlags |= q.m_bForceAllWords ? EXCERPT_FLAG_FORCE_ALL_WORDS : 0;
	if ( !bOnlyScattered || !( q.m_uFilesMode & 2 ) )
		iRawFlags |= ( q.m_uFilesMode & 1 ) ? EXCERPT_FLAG_LOAD_FILES : 0;
	iRawFlags |= q.m_bAllowEmpty ? EXCERPT_FLAG_ALLOW_EMPTY : 0;
	iRawFlags |= q.m_bEmitZones ? EXCERPT_FLAG_EMIT_ZONES : 0;
	iRawFlags |= ( q.m_uFilesMode & 2 ) ? EXCERPT_FLAG_FILES_SCATTERED : 0;
	iRawFlags |= q.m_bForcePassages ? EXCERPT_FLAG_FORCEPASSAGES : 0;
	return iRawFlags;
}

struct ExcerptQuery_t
{
	int64_t			m_iSize = 0;		///< file size, to sort to work-queue order
	CSphString		m_sSource;			///< source data
	CSphString		m_sError;
	CSphVector<BYTE> m_dResult;			///< query result
};


class SnippetRemote_c : public RequestBuilder_i, public ReplyParser_i
{
public:
	SnippetRemote_c ( VecTraits_T<ExcerptQuery_t> & dQueries, const SnippetQuerySettings_t& q )
		: m_dQueries ( dQueries )
		, m_tSettings ( q )
	{}

	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final;
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final;

private:
	VecTraits_T<ExcerptQuery_t> &			m_dQueries;
	const SnippetQuerySettings_t &			m_tSettings;
	mutable CSphAtomic 						m_iWorker;

	bool ParseReplyScattered ( MemInputBuffer_c & tReq, const VecTraits_T<int>& dDocs ) const;
	bool ParseReplyNonScattered ( MemInputBuffer_c & tReq, const VecTraits_T<int> & dDocs ) const;

public:
	CSphVector<const VecTraits_T<int> *> m_dTasks;
};


void SnippetRemote_c::BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const
{
	// it sends either all queries to each agent or sequence of queries to current agent
	auto iWorker = tAgent.m_iStoreTag;
	if ( iWorker<0 )
	{
		iWorker = ( int ) m_iWorker++;
		tAgent.m_iStoreTag = iWorker;
	}

	auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_EXCERPT, VER_COMMAND_EXCERPT );

	tOut.SendInt ( 0 );
	tOut.SendInt ( PackAPISnippetFlags ( m_tSettings, true ) );
	tOut.SendString ( tAgent.m_tDesc.m_sIndexes.cstr () );
	tOut.SendString ( m_tSettings.m_sQuery.cstr() );
	tOut.SendString ( m_tSettings.m_sBeforeMatch.cstr() );
	tOut.SendString ( m_tSettings.m_sAfterMatch.cstr() );
	tOut.SendString ( m_tSettings.m_sChunkSeparator.cstr() );
	tOut.SendInt ( m_tSettings.m_iLimit );
	tOut.SendInt ( m_tSettings.m_iAround );
	tOut.SendInt ( m_tSettings.m_iLimitPassages );
	tOut.SendInt ( m_tSettings.m_iLimitWords );
	tOut.SendInt ( m_tSettings.m_iPassageId );
	tOut.SendString ( m_tSettings.m_sStripMode.cstr() );
	tOut.SendString ( PassageBoundarySz ( m_tSettings.m_ePassageSPZ ) );

	const auto& dDocs = *m_dTasks[iWorker];
	tOut.SendInt ( dDocs.GetLength() );
	for ( int iDoc : dDocs )
		tOut.SendString ( m_dQueries[iDoc].m_sSource.cstr() );
}

bool SnippetRemote_c::ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const
{
	auto& tDocs = *m_dTasks[tAgent.m_iStoreTag];
	if ( m_tSettings.m_uFilesMode & 2 ) // scattered files
		return ParseReplyScattered ( tReq, tDocs );

	return ParseReplyNonScattered ( tReq, tDocs );
}

bool SnippetRemote_c::ParseReplyScattered ( MemInputBuffer_c & tReq, const VecTraits_T<int> & dDocs ) const
{
	bool bOk = true;
	for ( int iDoc : dDocs )
	{
		ExcerptQuery_t & tQuery = m_dQueries[iDoc];
		CSphVector<BYTE> & dRes = tQuery.m_dResult;


		if ( !tReq.GetString(dRes) || dRes.IsEmpty() )
		{
			bOk = false;
			dRes.Resize(0);
		} else
			tQuery.m_sError = "";
	}
	return bOk;
}

bool SnippetRemote_c::ParseReplyNonScattered ( MemInputBuffer_c & tReq, const VecTraits_T<int> & dDocs ) const
{
	for ( int iDoc : dDocs )
	{
		ExcerptQuery_t & tQuery = m_dQueries[iDoc];
		tReq.GetString ( tQuery.m_dResult );
		tQuery.m_iSize = -1; // means 'processed'
	}

	return true;
}

static int64_t GetSnippetDataSize ( const CSphVector<ExcerptQuery_t> &dSnippets )
{
	int64_t iSize = 0;
	for ( const auto & dSnippet: dSnippets )
	{
		if ( dSnippet.m_iSize>0 )
			iSize += dSnippet.m_iSize;
		else if ( !dSnippet.m_iSize )
			iSize += dSnippet.m_sSource.Length ();
	}
	iSize /= 100;
	return iSize;
}

static VecRefPtrsAgentConn_t GetDistrAgents ( DistributedIndex_t * pDist )
{
	assert ( pDist );
	VecRefPtrsAgentConn_t tRemotes;
	for ( auto * pAgent : pDist->m_dAgents )
	{
		auto * pConn = new AgentConn_t;
		pConn->SetMultiAgent ( pAgent );
		pConn->m_iMyConnectTimeoutMs = pDist->m_iAgentConnectTimeoutMs;
		pConn->m_iMyQueryTimeoutMs = pDist->m_iAgentQueryTimeoutMs;
		tRemotes.Add ( pConn );
	}
	return tRemotes;
}

// collect source sizes. For absent files set -1.
static bool CollectSourceSizes ( CSphVector<ExcerptQuery_t> & dQueries, bool bFileMode, bool bNeedAll, CSphString & sError )
{
	// collect source sizes
	if ( !bFileMode )
	{
		dQueries.Apply ( [] ( ExcerptQuery_t & dQuery ) { dQuery.m_iSize = dQuery.m_sSource.Length (); } );
		return true;
	}

	for ( auto & dQuery : dQueries )
	{
		CSphString sFilename, sStatError;
		sFilename.SetSprintf ( "%s%s", g_sSnippetsFilePrefix.cstr (), dQuery.m_sSource.scstr () );
		if ( !TestEscaping ( g_sSnippetsFilePrefix, sFilename ) )
		{
			sError.SetSprintf ( "File '%s' escapes '%s' scope", sFilename.scstr (), g_sSnippetsFilePrefix.scstr () );
			return false;
		}
		auto iFileSize = sphGetFileSize ( sFilename, &sStatError );
		if ( iFileSize<0 )
		{
			if ( bNeedAll )
			{
				sError = sStatError;
				return false;
			}
			dQuery.m_iSize = -1;
		} else
			dQuery.m_iSize = iFileSize;
	}
	return true;
}

// helper, called both for single and for multi snippets
static inline bool MakeSingleLocalSnippetWithFields ( ExcerptQuery_t & tQuery, const SnippetQuerySettings_t & q,
		SnippetBuilder_c * pBuilder, const VecTraits_T<int>& dFields )
{
	assert ( pBuilder );

	CSphScopedPtr<TextSource_i> pSource ( CreateSnippetSource ( q.m_uFilesMode,
			(const BYTE*)tQuery.m_sSource.cstr(), tQuery.m_sSource.Length() ) );

	SnippetResult_t tRes;
	if ( !pBuilder->Build ( pSource.Ptr(), tRes ) )
	{
		tQuery.m_sError = std::move ( tRes.m_sError );
		return false;
	}

	tQuery.m_dResult = pBuilder->PackResult ( tRes, dFields );
	return true;
};

// boring single snippet
static inline bool MakeSingleLocalSnippet ( ExcerptQuery_t & tQuery, const SnippetQuerySettings_t & q,
		SnippetBuilder_c * pBuilder, CSphString& sError )
{
	CSphVector<int> dStubFields;
	dStubFields.Add ( 0 );

	if ( MakeSingleLocalSnippetWithFields ( tQuery, q, pBuilder, dStubFields ) )
		return true;

	sError = tQuery.m_sError;
	return false;
}

struct SnippedBuilderCtxRef_t
{
	SnippetBuilder_c * m_pBuilder;
	SnippedBuilderCtxRef_t ( SnippetBuilder_c * pBuilder ) : m_pBuilder ( pBuilder ) {}
	inline static bool IsClonable () { return true; }
};

struct SnippedBuilderCtxClone_t : public SnippedBuilderCtxRef_t, ISphNoncopyable
{
	explicit SnippedBuilderCtxClone_t ( const SnippedBuilderCtxRef_t& dParent )
		: SnippedBuilderCtxRef_t { dParent.m_pBuilder->MakeClone() }
	{}

	// dtr is only for clones!
	~SnippedBuilderCtxClone_t() { SafeDelete (m_pBuilder); }
};

// Starts or performs parallel snippets creation with throttling
static void MakeSnippetsCoro ( const VecTraits_T<int>& dTasks, CSphVector<ExcerptQuery_t> & dQueries,
		const SnippetQuerySettings_t& q, SnippetBuilder_c * pBuilder)
{
	assert ( pBuilder );
	if ( dTasks.IsEmpty() )
		return;
	sphLogDebug ( "MakeSnippetsCoro invoked for %d tasks", dTasks.GetLength() );

	CSphVector<int> dStubFields;
	dStubFields.Add ( 0 );

	using SnippetContextTls_t = FederateCtx_T<SnippedBuilderCtxRef_t, SnippedBuilderCtxClone_t>;
	SnippetContextTls_t dActualpBuilder { pBuilder };

	std::atomic<int32_t> iCurQuery { 0 };
	CoExecuteN ( [&] () mutable
	{
		sphLogDebug ( "MakeSnippetsCoro Coro started" );
		Optional_T<SnippedBuilderCtxRef_t> pCtx;
		pCtx.emplace ( dActualpBuilder.GetContext () );
		Threads::CoThrottler_c tThrottler ( myinfo::ref<ClientTaskInfo_t> ()->m_iThrottlingPeriod );
		int iTick=1;
		while ( true )
		{
			auto iQuery = iCurQuery.fetch_add ( 1, std::memory_order_acq_rel );
			if ( iQuery>=dTasks.GetLength () )
				return; // all is done

			myinfo::SetThreadInfo ( "%d s %d:", iTick, iQuery );
			sphLogDebug ( "MakeSnippetsCoro Coro loop tick %d[%d]", iQuery, dTasks[iQuery] );
			MakeSingleLocalSnippetWithFields ( dQueries[dTasks[iQuery]], q, pCtx->m_pBuilder, dStubFields );
			sphLogDebug ( "MakeSnippetsCoro Coro loop tick %d finished", iQuery );

			// yield and reschedule every quant of time. It gives work to other tasks
			if ( tThrottler.ThrottleAndKeepCrashQuery() )
			{
				++iTick;
				if ( !tThrottler.SameThread () )
					pCtx.emplace ( dActualpBuilder.GetContext () );
			}
		}
	}, dActualpBuilder.Concurrency ( dTasks.GetLength ()));
}

// divide set of tasks from dTasks into chunks, having most balanced aggregate iSize in each.
static CSphVector<CSphVector<int>> DivideTasks ( const VecTraits_T<int> & dTasks,
		const VecTraits_T<ExcerptQuery_t> & dQueries, int iWorkers )
{
	CSphVector<CSphVector<int>> dResults;
	auto iTasks = dTasks.GetLength();
	auto iLimit = Min ( iWorkers, iTasks );

	if ( iWorkers>=iTasks )
	{
		dResults.Resize ( iLimit );
		for ( int i=0; i<iLimit; ++i )
			dResults[i].Add ( dTasks[i] );
	} else
	{
		// helpers
		using ItemsQueue_c = TimeoutQueue_c;
		using EnqueuedItem_t = EnqueuedTimeout_t;
		ItemsQueue_c qTasks;
		struct PriorityVec_t : EnqueuedItem_t { int m_iRefIdx; };
		CSphVector<PriorityVec_t> dPriorityResults ( iWorkers );
		dResults.Resize ( iWorkers );

		// initially fill the queue
		ARRAY_FOREACH ( i, dPriorityResults )
		{
			dResults[i].Add ( dTasks[i] );
			dPriorityResults[i].m_iTimeoutTimeUS = dQueries[dTasks[i]].m_iSize;
			dPriorityResults[i].m_iRefIdx = i;
			qTasks.Change ( &dPriorityResults[i] );
		}

		// update the queue
		for ( int i=iWorkers; i<iTasks; ++i )
		{
			auto * pBest = (PriorityVec_t *) qTasks.Root ();
			dResults[pBest->m_iRefIdx].Add ( dTasks[i] );
			pBest->m_iTimeoutTimeUS += dQueries[dTasks[i]].m_iSize;
			qTasks.Change ( pBest );
		}
	}
	return dResults;
}

// remote scattered snippets (with local pass)
// * dLocal subset is run on local host
// * dRemote subset is send to each remote agent
static void MakeRemoteScatteredSnippets ( CSphVector<ExcerptQuery_t> & dQueries,
		DistributedIndex_t * pDist,
		SnippetBuilder_c * pBuilder,
		const SnippetQuerySettings_t & q,
		const VecTraits_T<int>& dLocal,
		const VecTraits_T<int>& dAbsent )
{
	assert ( pDist );
	assert ( pBuilder );

	// and finally most interesting remote case with possibly scattered.
	auto dAgents = GetDistrAgents ( pDist );
	int iRemoteAgents = dAgents.GetLength();

	SnippetRemote_c tRemotes ( dQueries, q );
	tRemotes.m_dTasks.Resize ( iRemoteAgents );

	// on scattered case - just push the chain of locally absent files to all remotes
	for ( auto & pTask : tRemotes.m_dTasks )
		pTask = &dAbsent;

	// query remote building
	CSphRefcountedPtr<RemoteAgentsObserver_i> tReporter ( GetObserver () );
	ScheduleDistrJobs ( dAgents, &tRemotes, &tRemotes, tReporter );

	// start local building and wait it to finish
	MakeSnippetsCoro ( dLocal, dQueries, q, pBuilder );

	// wait remotes to finish also
	tReporter->Finish ();

	auto iSuccesses = ( int ) tReporter->GetSucceeded ();
	auto iAgentsDone = ( int ) tReporter->GetFinished ();

	if ( iSuccesses!=iRemoteAgents )
		sphWarning ( "Remote snippets: some of the agents didn't answered: %d queried, %d finished, %d succeeded",
				iRemoteAgents, iAgentsDone,	iSuccesses );
}

// remote non scattered snippets (with local pass)
// non-scattered assumes, each host has full set of sources, so we don't need to check absent here.
// * divide set of sources among remotes and local host, balancing size.
// * assume dPresent has indexes of monotonically decreasing sizes, that's need for balancing.
static void MakeRemoteNonScatteredSnippets ( CSphVector<ExcerptQuery_t> & dQueries,
		DistributedIndex_t * pDist,
		SnippetBuilder_c * pBuilder,
		const SnippetQuerySettings_t & q,
		const VecTraits_T<int>& dPresent )
{
	assert ( pDist );
	assert ( pBuilder );

	auto dAgents = GetDistrAgents ( pDist );
	int iRemoteAgents = dAgents.GetLength();

	SnippetRemote_c tRemotes ( dQueries, q );
	tRemotes.m_dTasks.Resize ( iRemoteAgents );

	// on non-scattered - distribute set of sources to workers, having 1 local worker in mind.
	auto dJobSet = DivideTasks ( dPresent, dQueries, iRemoteAgents+1 ) ; // +1 since we also will work locally.
	auto& dLocalSet = dJobSet[iRemoteAgents];
	for ( int i = 0; i<iRemoteAgents; ++i )
		tRemotes.m_dTasks[i] = &dJobSet[i];

	// query remote building
	CSphRefcountedPtr<RemoteAgentsObserver_i> tReporter ( GetObserver () );
	ScheduleDistrJobs ( dAgents, &tRemotes, &tRemotes, tReporter );

	// start local building and wait it to finish
	MakeSnippetsCoro ( dLocalSet, dQueries, q, pBuilder );

	// wait remotes to finish also
	tReporter->Finish ();

	auto iSuccesses = ( int ) tReporter->GetSucceeded ();
	auto iAgentsDone = ( int ) tReporter->GetFinished ();

	if ( iSuccesses==iRemoteAgents )
		return;

	sphWarning ( "Remote snippets: some of the agents didn't answered: %d queried, %d finished, %d succeeded",
			iRemoteAgents, iAgentsDone,	iSuccesses );

	// let's collect failures and make one more pass over them
	CSphVector<int> dFailed;

	// collect failed nodes
	dPresent.Apply ( [&] ( int iDoc ) {
		if ( dQueries[iDoc].m_iSize<0 )
			dFailed.Add(iDoc);
	});

	if ( dFailed.IsEmpty() )
		return;

	// failsafe - one more turn for failed queries on local agent
	sphWarning ( "Snippets: failsafe for %d failed items", (int) dFailed.GetLength() );
	MakeSnippetsCoro ( dFailed, dQueries, q, pBuilder );
}

bool MakeSnippets ( CSphString sIndex, CSphVector<ExcerptQuery_t> & dQueries,
		const SnippetQuerySettings_t& q, CSphString & sError )
{
	assert ( !dQueries.IsEmpty() );

	// When both load_files & load_files_scattered set, absent files will be reported as errors.
	// load_files_scattered without load_files just omits the absent files (returns empty strings).
	auto bScattered = !!( q.m_uFilesMode & 2 );
	auto bNeedAllFiles = !!( q.m_uFilesMode & 1 );

	auto pDist = GetDistr ( sIndex );
	bool bRemote = pDist && !pDist->m_dAgents.IsEmpty ();

	if ( bRemote )
	{
		if ( pDist->m_dLocal.GetLength()!=1 )
		{
			sError.SetSprintf ( "%s", "distributed index for snippets must have exactly one local agent" );
			return false;
		}

		if ( !q.m_uFilesMode )
		{
			sError.SetSprintf ( "%s", "distributed index for snippets available only when using external files" );
			return false;
		}

		// for remotes index is 1-st local agent of the distr, so move on!
		sIndex = pDist->m_dLocal[0];
	}

	ServedDescRPtr_c pServed ( GetServed ( sIndex ) );
	if ( !pServed || !pServed->m_pIndex )
	{
		sError.SetSprintf ( "unknown local index '%s' in search request", sIndex.cstr() );
		return false;
	}

	CSphIndex * pLocalIndex = pServed->m_pIndex;
	assert ( pLocalIndex );

	///////////////////
	/// do highlighting
	///////////////////

	CSphScopedPtr<SnippetBuilder_c>	pBuilder ( new SnippetBuilder_c );
	pBuilder->Setup ( pLocalIndex, q );

	if ( !pBuilder->SetQuery ( q.m_sQuery.cstr(), true, sError ) )
		return false;

	// boring single snippet
	if ( dQueries.GetLength ()==1 )
		return MakeSingleLocalSnippet ( dQueries[0], q, pBuilder.Ptr(), sError );

	if ( !CollectSourceSizes ( dQueries, q.m_uFilesMode, !bScattered, sError ) )
		return false;

	// set correct data size for snippets
	myinfo::SetThreadInfo ( R"(snippet datasize=%.1Dk query="%s")",
			GetSnippetDataSize ( dQueries ), q.m_sQuery.scstr () );

	// collect list of existing and empty sources
	CSphVector<int> dPresent;
	CSphVector<int> dAbsent;

	ARRAY_FOREACH ( i, dQueries )
	{
		if ( dQueries[i].m_iSize<0 )
			dAbsent.Add(i);
		else
			dPresent.Add(i);
	}

	// check if all files are available locally - then we need no remote pass.
	if ( bScattered && dAbsent.IsEmpty() )
		bRemote = false;

	if ( bNeedAllFiles && !dAbsent.IsEmpty() )
		for ( int i : dAbsent )
			dQueries[i].m_sError.SetSprintf ( "absenthead: failed to stat %s", dQueries[i].m_sSource.cstr () );

	// tough jobs first (sort inverse)
	if ( !bScattered )
		dPresent.Sort ( Lesser ( [&dQueries] ( int a, int b ) { return dQueries[a].m_iSize>dQueries[b].m_iSize; } ) );

	if ( !bRemote )
	{
		// multithreaded, but no remote agents.
		MakeSnippetsCoro ( dPresent, dQueries, q, pBuilder.Ptr() );

	} else
	{
		assert ( pDist );

		// multithreaded with remotes (scattered and full)
		if ( bScattered )
			MakeRemoteScatteredSnippets ( dQueries, pDist, pBuilder.Ptr(), q, dPresent, dAbsent );
		else
			MakeRemoteNonScatteredSnippets ( dQueries, pDist, pBuilder.Ptr (), q, dPresent );
	}

	StringBuilder_c sErrors ( "; " );
	dQueries.Apply ( [&] ( const ExcerptQuery_t & tQuery ) { sErrors << tQuery.m_sError; } );
	sErrors.MoveTo ( sError );
	return sError.IsEmpty();
}

// throw out tailing \0 if any
inline static void FixupResultTail (CSphVector<BYTE> & dData)
{
	if ( !dData.IsEmpty() && !dData.Last () )
		dData.Pop ();
}

void HandleCommandExcerpt ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_EXCERPT, tOut ) )
		return;

	/////////////////////////////
	// parse and process request
	/////////////////////////////

	const int EXCERPT_MAX_ENTRIES			= 1024;

	// v.1.1
	SnippetQuerySettings_t q;

	tReq.GetInt (); // mode field is for now reserved and ignored
	int iFlags = tReq.GetInt ();
	CSphString sIndex = tReq.GetString ();

	q.m_sQuery = tReq.GetString ();
	q.m_sBeforeMatch = tReq.GetString ();
	q.m_sAfterMatch = tReq.GetString ();
	q.m_sChunkSeparator = tReq.GetString ();
	q.m_iLimit = tReq.GetInt ();
	q.m_iAround = tReq.GetInt ();

	if ( iVer>=0x102 )
	{
		q.m_iLimitPassages = tReq.GetInt();
		q.m_iLimitWords = tReq.GetInt();
		q.m_iPassageId = tReq.GetInt();
		q.m_sStripMode = tReq.GetString();
		if ( q.m_sStripMode!="none" && q.m_sStripMode!="index" && q.m_sStripMode!="strip" && q.m_sStripMode!="retain" )
		{
			SendErrorReply ( tOut, "unknown html_strip_mode=%s", q.m_sStripMode.cstr() );
			return;
		}
	}

	q.Setup();

	CSphString sPassageBoundaryMode;
	if ( iVer>=0x103 )
		q.m_ePassageSPZ = GetPassageBoundary ( tReq.GetString() );

	q.m_bUseBoundaries = ( iFlags & EXCERPT_FLAG_USEBOUNDARIES )!=0;
	q.m_bWeightOrder = ( iFlags & EXCERPT_FLAG_WEIGHTORDER )!=0;
	q.m_bForceAllWords = ( iFlags & EXCERPT_FLAG_FORCE_ALL_WORDS )!=0;
	if ( iFlags & EXCERPT_FLAG_SINGLEPASSAGE )
		q.m_iLimitPassages = 1;
	q.m_uFilesMode = ( iFlags & EXCERPT_FLAG_LOAD_FILES )?1:0;
	bool bScattered = ( iFlags & EXCERPT_FLAG_FILES_SCATTERED )!=0;
	q.m_uFilesMode |= bScattered?2:0;
	q.m_bAllowEmpty = ( iFlags & EXCERPT_FLAG_ALLOW_EMPTY )!=0;
	q.m_bEmitZones = ( iFlags & EXCERPT_FLAG_EMIT_ZONES )!=0;
	q.m_bForcePassages = ( iFlags & EXCERPT_FLAG_FORCEPASSAGES )!=0;

	bool bExactPhrase = ( iFlags & EXCERPT_FLAG_EXACTPHRASE )!=0;
	if ( bExactPhrase )
	{
		SendErrorReply ( tOut, "exact_phrase is deprecated" );
		return;
	}

	int iCount = tReq.GetInt ();
	if ( iCount<=0 || iCount>EXCERPT_MAX_ENTRIES )
	{
		SendErrorReply ( tOut, "invalid entries count %d", iCount );
		return;
	}

	CSphString sError;

	if ( !sphCheckOptionsSPZ ( q, q.m_ePassageSPZ, sError ) )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return;
	}

	CSphVector<ExcerptQuery_t> dQueries { iCount };

	for ( auto & dQuery : dQueries )
	{
		dQuery.m_sSource = tReq.GetString (); // fetch data
		if ( tReq.GetError() )
		{
			SendErrorReply ( tOut, "invalid or truncated request" );
			return;
		}
	}
	myinfo::SetThreadInfo ( R"(api-snippet datasize=%.1Dk query="%s")",
			GetSnippetDataSize ( dQueries ), q.m_sQuery.scstr ());

	if ( !MakeSnippets ( sIndex, dQueries, q, sError ) )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return;
	}

	////////////////
	// serve result
	////////////////

	for ( const auto & i : dQueries )
	{
		// handle errors
		if ( !bScattered && !i.m_sError.IsEmpty() )
		{
			SendErrorReply ( tOut, "highlighting failed: %s", i.m_sError.cstr() );
			return;
		}
	}

	auto tReply = APIAnswer ( tOut, VER_COMMAND_EXCERPT );
	for ( const auto & i : dQueries )
		tOut.SendArray ( i.m_dResult );
}

/////////////////////////////////////////////////////////////////////////////
// KEYWORDS HANDLER
/////////////////////////////////////////////////////////////////////////////

static bool DoGetKeywords ( const CSphString & sIndex, const CSphString & sQuery, const GetKeywordsSettings_t & tSettings, CSphVector <CSphKeywordInfo> & dKeywords, CSphString & sError, SearchFailuresLog_c & tFailureLog );

void HandleCommandKeywords ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_KEYWORDS, tOut ) )
		return;

	GetKeywordsSettings_t tSettings;
	CSphString sQuery = tReq.GetString ();
	CSphString sIndex = tReq.GetString ();
	tSettings.m_bStats = !!tReq.GetInt ();
	if ( uVer>=0x101 )
	{
		tSettings.m_bFoldLemmas = !!tReq.GetInt ();
		tSettings.m_bFoldBlended = !!tReq.GetInt ();
		tSettings.m_bFoldWildcards = !!tReq.GetInt ();
		tSettings.m_iExpansionLimit = tReq.GetInt ();
	}

	CSphString sError;
	SearchFailuresLog_c tFailureLog;
	CSphVector < CSphKeywordInfo > dKeywords;
	bool bOk = DoGetKeywords ( sIndex, sQuery, tSettings, dKeywords, sError, tFailureLog );
	if ( !bOk )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return;
	}
	// just log distribute index error as command has no warning filed to pass such error into
	if ( !tFailureLog.IsEmpty() )
	{
		StringBuilder_c sErrorBuf;
		tFailureLog.BuildReport ( sErrorBuf );
		sphWarning ( "%s", sErrorBuf.cstr() );
	}

	auto tReply = APIAnswer ( tOut, VER_COMMAND_KEYWORDS );
	tOut.SendInt ( dKeywords.GetLength () );
	for ( auto & dKeyword : dKeywords )
	{
		tOut.SendString ( dKeyword.m_sTokenized.cstr () );
		tOut.SendString ( dKeyword.m_sNormalized.cstr () );
		if ( uVer>=0x101 )
			tOut.SendInt ( dKeyword.m_iQpos );
		if ( tSettings.m_bStats )
		{
			tOut.SendInt ( dKeyword.m_iDocs );
			tOut.SendInt ( dKeyword.m_iHits );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// UPDATES HANDLER
/////////////////////////////////////////////////////////////////////////////

class UpdateRequestBuilder_c : public RequestBuilder_i
{
public:
	explicit UpdateRequestBuilder_c ( const CSphAttrUpdate & pUpd ) : m_tUpd ( pUpd ) {}
	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer& tOut ) const final;

protected:
	const CSphAttrUpdate & m_tUpd;
};


class UpdateReplyParser_c : public ReplyParser_i
{
public:
	explicit UpdateReplyParser_c ( int * pUpd )
		: m_pUpdated ( pUpd )
	{}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final
	{
		*m_pUpdated += tReq.GetDword ();
		return true;
	}

protected:
	int * m_pUpdated;
};


void UpdateRequestBuilder_c::BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const
{
	const char * sIndexes = tAgent.m_tDesc.m_sIndexes.cstr();
	assert ( m_tUpd.m_dAttributes.TestAll ( [&] ( const TypedAttribute_t & tAttr ) { return ( tAttr.m_eType!=SPH_ATTR_INT64SET ); } ) );

	// API header
	auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_UPDATE, VER_COMMAND_UPDATE );

	tOut.SendString ( sIndexes );
	tOut.SendInt ( m_tUpd.m_dAttributes.GetLength() );
	tOut.SendInt ( m_tUpd.m_bIgnoreNonexistent ? 1 : 0 );
	for ( const auto & i : m_tUpd.m_dAttributes )
	{
		tOut.SendString ( i.m_sName.cstr() );

		UpdateType_e eUpdate;
		switch ( i.m_eType )
		{
		case SPH_ATTR_UINT32SET:	eUpdate = UPDATE_MVA32; break;
		case SPH_ATTR_STRING:
		case SPH_ATTR_JSON:			eUpdate = UPDATE_STRING; break;
		default:					eUpdate = UPDATE_INT; break;
		};

		tOut.SendInt ( eUpdate );
	}

	tOut.SendInt ( m_tUpd.m_dDocids.GetLength() );

	ARRAY_FOREACH ( iDoc, m_tUpd.m_dDocids )
	{
		tOut.SendUint64 ( m_tUpd.m_dDocids[iDoc] );

		const DWORD * pPool = m_tUpd.m_dPool.Begin() + m_tUpd.m_dRowOffset[iDoc];
		for ( const auto & i : m_tUpd.m_dAttributes )
		{
			DWORD uVal = *pPool++;

			switch ( i.m_eType )
			{
			case SPH_ATTR_UINT32SET:
				{
					// size down in case of MVA
					// MVA stored as mva64 in pool but API could handle only mva32 due to HandleCommandUpdate
					// SphinxQL only could work either mva32 or mva64 and only SphinxQL could receive mva64 updates
					// SphinxQL master communicate to agent via SphinxqlRequestBuilder_c

					const DWORD * pEnd = pPool + uVal;
					tOut.SendDword ( uVal/2 );
					while ( pPool<pEnd )
					{
						auto iVal = *(int64_t*)pPool;
						tOut.SendDword ( iVal&0xFFFFFFFF );
						pPool += 2;
					}
				}
				break;

			case SPH_ATTR_STRING:
			case SPH_ATTR_JSON:
				{
					DWORD uBlobLen = *pPool++;
					tOut.SendDword ( uBlobLen );
					tOut.SendBytes ( m_tUpd.m_dBlobs.Begin()+uVal, uBlobLen );
				}
				break;

			default:
				tOut.SendDword ( uVal );
				break;
			}
		}
	}
}

static void DoCommandUpdate ( const CSphString & sIndex, const char * sDistributed, const CSphAttrUpdate & tUpd, bool bBlobUpdate,
	int & iSuccesses, int & iUpdated,
	SearchFailuresLog_c & dFails, ServedIndexRefPtr_c & pServed )
{
	CSphString sCluster;
	{
		ServedDescRPtr_c tDesc ( pServed );
		if ( !tDesc )
		{
			dFails.Submit ( sIndex, sDistributed, "index not available" );
			return;
		}

		sCluster = tDesc->m_sCluster;
	}

	int iUpd = 0;
	CSphString sError, sWarning;
	RtAccum_t tAcc ( false );
	ReplicationCommand_t * pCmd = tAcc.AddCommand ( ReplicationCommand_e::UPDATE_API, sCluster, sIndex );
	assert ( pCmd );
	pCmd->m_pUpdateAPI = &tUpd;
	pCmd->m_bBlobUpdate = bBlobUpdate;

	HandleCmdReplicate ( tAcc, sError, sWarning, iUpd );

	if ( iUpd<0 )
	{
		dFails.Submit ( sIndex, sDistributed, sError.cstr() );

	} else
	{
		iUpdated += iUpd;
		++iSuccesses;
		if ( sWarning.Length() )
			dFails.Submit ( sIndex, sDistributed, sWarning.cstr() );
	}
}

using DistrPtrs_t = VecRefPtrs_t< const DistributedIndex_t *>;
static bool ExtractDistributedIndexes ( const StrVec_t &dNames, DistrPtrs_t &dDistributed, CSphString& sMissed )
{
	dDistributed.Reset();
	dDistributed.Resize( dNames.GetLength () );
	dDistributed.ZeroVec ();

	ARRAY_FOREACH ( i, dNames )
	{
		if ( !g_pLocalIndexes->Contains ( dNames[i] ) )
		{
			// search amongst distributed and copy for further processing
			dDistributed[i] = GetDistr ( dNames[i] );

			if ( !dDistributed[i] )
			{
				sMissed = dNames[i];
				return false;
			}
			dDistributed[i]->AddRef ();
		}
	}
	return true;
}

void HandleCommandUpdate ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_UPDATE, tOut ) )
		return;

	// parse request
	CSphString sIndexes = tReq.GetString ();
	CSphAttrUpdate tUpd;
	CSphVector<DWORD> dMva;

	tUpd.m_dAttributes.Resize ( tReq.GetDword() ); // FIXME! check this
	if ( iVer>=0x103 )
		tUpd.m_bIgnoreNonexistent = ( tReq.GetDword() & 1 )!=0;

	bool bBlobUpdate = false;
	for ( auto & i : tUpd.m_dAttributes )
	{
		i.m_sName = tReq.GetString();
		if ( i.m_sName==sphGetDocidName() )
		{
			SendErrorReply ( tOut, "'id' attribute cannot be updated" );
			return;
		}

		i.m_eType = SPH_ATTR_INTEGER;
		if ( iVer>=0x102 )
		{
			UpdateType_e eUpdate = (UpdateType_e)tReq.GetDword();
			switch ( eUpdate )
			{
			case UPDATE_MVA32:
				i.m_eType = SPH_ATTR_UINT32SET;
				bBlobUpdate = true;
				break;

			case UPDATE_STRING:
			case UPDATE_JSON:
				i.m_eType = SPH_ATTR_STRING;
				bBlobUpdate = true;
				break;

			default:
				break;
			}
		}
	}

	int iNumUpdates = tReq.GetInt (); // FIXME! check this
	tUpd.m_dDocids.Reserve ( iNumUpdates );
	tUpd.m_dRowOffset.Reserve ( iNumUpdates );

	for ( int i=0; i<iNumUpdates; i++ )
	{
		// v.1.0 always sends 32-bit ids; v.1.1+ always send 64-bit ones
		uint64_t uDocid = ( iVer>=0x101 ) ? tReq.GetUint64 () : tReq.GetDword ();

		tUpd.m_dDocids.Add ( uDocid );
		tUpd.m_dRowOffset.Add ( tUpd.m_dPool.GetLength() );

		for ( const auto & iAttr : tUpd.m_dAttributes )
		{
			switch ( iAttr.m_eType )
			{
			case SPH_ATTR_UINT32SET:
				{
					DWORD uCount = tReq.GetDword ();
					if ( !uCount )
					{
						tUpd.m_dPool.Add ( 0 );
						continue;
					}

					dMva.Resize ( uCount );
					for ( DWORD j=0; j<uCount; j++ )
						dMva[j] = tReq.GetDword();

					dMva.Uniq(); // don't need dupes within MVA

					tUpd.m_dPool.Add ( dMva.GetLength()*2 );
					ARRAY_FOREACH ( j, dMva )
					{
						*(int64_t*)tUpd.m_dPool.AddN(2) = dMva[j]; // dummy expander mva32 -> mva64
					}
				}
				break;

			case SPH_ATTR_STRING:
				{
					DWORD uLen = tReq.GetDword();
					tUpd.m_dPool.Add ( tUpd.m_dBlobs.GetLength() );
					tUpd.m_dPool.Add ( uLen );

					if ( uLen )
					{
						// extra zeroes for json parser
						BYTE * pAdded = tUpd.m_dBlobs.AddN ( uLen+2 );
						if ( !tReq.GetBytes ( pAdded, uLen ) )
						{
							SendErrorReply ( tOut, "error reading string" );
							break;
						}

						pAdded[uLen] = 0;
						pAdded[uLen+1] = 0;
					}
				}
				break;

			default:
				tUpd.m_dPool.Add ( tReq.GetDword() );
				break;
			}
		}
	}

	if ( tReq.GetError() )
	{
		SendErrorReply ( tOut, "invalid or truncated request" );
		return;
	}

	// check index names
	StrVec_t dIndexNames;
	ParseIndexList ( sIndexes, dIndexNames );

	if ( !dIndexNames.GetLength() )
	{
		SendErrorReply ( tOut, "no valid indexes in update request" );
		return;
	}

	DistrPtrs_t dDistributed;
	// copy distributed indexes description
	CSphString sMissed;
	if ( !ExtractDistributedIndexes ( dIndexNames, dDistributed, sMissed ) )
	{
		SendErrorReply ( tOut, "unknown index '%s' in update request", sMissed.cstr() );
		return;
	}

	// do update
	SearchFailuresLog_c dFails;
	int iSuccesses = 0;
	int iUpdated = 0;

	ARRAY_FOREACH ( iIdx, dIndexNames )
	{
		const CSphString & sReqIndex = dIndexNames[iIdx];
		auto pLocal = GetServed ( sReqIndex );
		if ( pLocal )
		{
			DoCommandUpdate ( sReqIndex, nullptr, tUpd, bBlobUpdate, iSuccesses, iUpdated, dFails, pLocal );

		} else if ( dDistributed[iIdx] )
		{
			auto * pDist = dDistributed[iIdx];

			assert ( !pDist->IsEmpty() );

			for ( const CSphString & sLocal : pDist->m_dLocal )
			{
				auto pServed = GetServed ( sLocal );
				if ( !pServed )
					continue;

				DoCommandUpdate ( sLocal, sReqIndex.cstr(), tUpd, bBlobUpdate, iSuccesses, iUpdated, dFails, pServed );
			}

			// update remote agents
			if ( !dDistributed[iIdx]->m_dAgents.IsEmpty() )
			{
				VecRefPtrsAgentConn_t dAgents;
				pDist->GetAllHosts ( dAgents );

				// connect to remote agents and query them
				UpdateRequestBuilder_c tReqBuilder ( tUpd );
				UpdateReplyParser_c tParser ( &iUpdated );
				iSuccesses += PerformRemoteTasks ( dAgents, &tReqBuilder, &tParser );
			}
		}
	}

	// serve reply to client
	StringBuilder_c sReport;
	dFails.BuildReport ( sReport );

	if ( !iSuccesses )
	{
		SendErrorReply ( tOut, "%s", sReport.cstr() );
		return;
	}

	auto tReply = APIAnswer ( tOut, VER_COMMAND_UPDATE, dFails.IsEmpty() ? SEARCHD_OK : SEARCHD_WARNING );
	if ( !dFails.IsEmpty() )
		tOut.SendString ( sReport.cstr () );
	tOut.SendInt ( iUpdated );
}


//////////////////////////////////////////////////////////////////////////
// STATUS HANDLER
//////////////////////////////////////////////////////////////////////////

static inline void FormatMsec ( CSphString & sOut, int64_t tmTime )
{
	sOut.SetSprintf ( "%d.%03d", (int)( tmTime/1000000 ), (int)( (tmTime%1000000)/1000 ) );
}

void BuildStatus ( VectorLike & dStatus )
{
	const char * FMT64 = INT64_FMT;
	const char * FLOAT = "%.2f";
	const char * OFF = "OFF";

	const int64_t iQueriesDiv = Max ( g_tStats.m_iQueries.GetValue(), 1 );
	const int64_t iDistQueriesDiv = Max ( g_tStats.m_iDistQueries.GetValue(), 1 );

	dStatus.m_sColKey = "Counter";

	// FIXME? non-transactional!!!
	if ( dStatus.MatchAdd ( "uptime" ) )
		dStatus.Add().SetSprintf ( "%u", (DWORD)time(NULL)-g_tStats.m_uStarted );
	if ( dStatus.MatchAdd ( "connections" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iConnections.load(std::memory_order_relaxed) );
	if ( dStatus.MatchAdd ( "maxed_out" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iMaxedOut );
	if ( dStatus.MatchAdd ( "version" ) )
		dStatus.Add().SetSprintf ( "%s", szMANTICORE_VERSION );
	if ( dStatus.MatchAdd ( "mysql_version" ) )
		dStatus.Add().SetSprintf ( "%s", g_sMySQLVersion.cstr() );
	if ( dStatus.MatchAdd ( "command_search" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_SEARCH] );
	if ( dStatus.MatchAdd ( "command_excerpt" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_EXCERPT] );
	if ( dStatus.MatchAdd ( "command_update" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_UPDATE] );
	if ( dStatus.MatchAdd ( "command_delete" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_DELETE] );
	if ( dStatus.MatchAdd ( "command_keywords" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_KEYWORDS] );
	if ( dStatus.MatchAdd ( "command_persist" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_PERSIST] );
	if ( dStatus.MatchAdd ( "command_status" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_STATUS] );
	if ( dStatus.MatchAdd ( "command_flushattrs" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_FLUSHATTRS] );
	if ( dStatus.MatchAdd ( "command_set" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_UVAR] );
	if ( dStatus.MatchAdd ( "command_insert" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_INSERT] );
	if ( dStatus.MatchAdd ( "command_replace" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_REPLACE] );
	if ( dStatus.MatchAdd ( "command_commit" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_COMMIT] );
	if ( dStatus.MatchAdd ( "command_suggest" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_SUGGEST] );
	if ( dStatus.MatchAdd ( "command_json" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_JSON] );
	if ( dStatus.MatchAdd ( "command_callpq" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iCommandCount[SEARCHD_COMMAND_CALLPQ] );
	if ( dStatus.MatchAdd ( "agent_connect" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iAgentConnect );
	if ( dStatus.MatchAdd ( "agent_retry" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iAgentRetry );
	if ( dStatus.MatchAdd ( "queries" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iQueries );
	if ( dStatus.MatchAdd ( "dist_queries" ) )
		dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iDistQueries );

	// status of thread pool
	if ( dStatus.MatchAdd ( "workers_total" ) )
		dStatus.Add().SetSprintf ( "%d", GlobalWorkPool()->WorkingThreads() );
	if ( dStatus.MatchAdd ( "workers_active" ) )
		dStatus.Add().SetSprintf( "%d", myinfo::CountAll());
	if ( dStatus.MatchAdd ( "workers_clients" ) )
		dStatus.Add ().SetSprintf ( "%d", myinfo::CountClients());
	if ( dStatus.MatchAdd ( "work_queue_length" ) )
		dStatus.Add ().SetSprintf ( "%d", GlobalWorkPool()->Works());

	for ( RLockedDistrIt_c it ( g_pDistIndexes ); it.Next (); )
	{
		const char * sIdx = it.GetName().cstr();
		const auto &dAgents = it.Get ()->m_dAgents;
		ARRAY_FOREACH ( i, dAgents )
		{
			MultiAgentDesc_c &dMultiAgent = *dAgents[i];
			ARRAY_FOREACH ( j, dMultiAgent )
			{
				const auto pMetrics = dMultiAgent[j].m_pMetrics;
				for ( int k=0; k<eMaxAgentStat; ++k )
					if ( dStatus.MatchAddVa ( "ag_%s_%d_%d_%s", sIdx, i+1, j+1, sAgentStatsNames[k] ) )
						dStatus.Add().SetSprintf ( FMT64, (int64_t) pMetrics->m_dCounters[k] );
				for ( int k = 0; k<ehMaxStat; ++k )
					if ( dStatus.MatchAddVa ( "ag_%s_%d_%d_%s", sIdx, i + 1, j + 1, sAgentStatsNames[eMaxAgentStat+k] ) )
					{
						if ( k==ehTotalMsecs || k==ehAverageMsecs || k==ehMaxMsecs )
							dStatus.Add ().SetSprintf ( FLOAT, (float) pMetrics->m_dMetrics[k] / 1000.0 );
						else
							dStatus.Add ().SetSprintf ( FMT64, (int64_t) pMetrics->m_dMetrics[k] );
					}

			}
		}
	}

	if ( dStatus.MatchAdd ( "query_wall" ) )
		FormatMsec ( dStatus.Add(), g_tStats.m_iQueryTime );

	if ( dStatus.MatchAdd ( "query_cpu" ) )
	{
		if ( g_bCpuStats )
			FormatMsec ( dStatus.Add(), g_tStats.m_iQueryCpuTime );
		else
			dStatus.Add() = OFF;
	}

	if ( dStatus.MatchAdd ( "dist_wall" ) )
		FormatMsec ( dStatus.Add(), g_tStats.m_iDistWallTime );
	if ( dStatus.MatchAdd ( "dist_local" ) )
		FormatMsec ( dStatus.Add(), g_tStats.m_iDistLocalTime );
	if ( dStatus.MatchAdd ( "dist_wait" ) )
		FormatMsec ( dStatus.Add(), g_tStats.m_iDistWaitTime );

	if ( g_bIOStats )
	{
		if ( dStatus.MatchAdd ( "query_reads" ) )
			dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iDiskReads );
		if ( dStatus.MatchAdd ( "query_readkb" ) )
			dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iDiskReadBytes/1024 );
		if ( dStatus.MatchAdd ( "query_readtime" ) )
			FormatMsec ( dStatus.Add(), g_tStats.m_iDiskReadTime );
	} else
	{
		if ( dStatus.MatchAdd ( "query_reads" ) )
			dStatus.Add() = OFF;
		if ( dStatus.MatchAdd ( "query_readkb" ) )
			dStatus.Add() = OFF;
		if ( dStatus.MatchAdd ( "query_readtime" ) )
			dStatus.Add() = OFF;
	}

	if ( g_tStats.m_iPredictedTime || g_tStats.m_iAgentPredictedTime )
	{
		if ( dStatus.MatchAdd ( "predicted_time" ) )
			dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iPredictedTime );
		if ( dStatus.MatchAdd ( "dist_predicted_time" ) )
			dStatus.Add().SetSprintf ( FMT64, (int64_t) g_tStats.m_iAgentPredictedTime );
	}

	if ( dStatus.MatchAdd ( "avg_query_wall" ) )
		FormatMsec ( dStatus.Add(), g_tStats.m_iQueryTime / iQueriesDiv );
	if ( dStatus.MatchAdd ( "avg_query_cpu" ) )
	{
		if ( g_bCpuStats )
			FormatMsec ( dStatus.Add(), g_tStats.m_iQueryCpuTime / iQueriesDiv );
		else
			dStatus.Add ( OFF );
	}

	if ( dStatus.MatchAdd ( "avg_dist_wall" ) )
		FormatMsec ( dStatus.Add(), g_tStats.m_iDistWallTime / iDistQueriesDiv );
	if ( dStatus.MatchAdd ( "avg_dist_local" ) )
		FormatMsec ( dStatus.Add(), g_tStats.m_iDistLocalTime / iDistQueriesDiv );
	if ( dStatus.MatchAdd ( "avg_dist_wait" ) )
		FormatMsec ( dStatus.Add(), g_tStats.m_iDistWaitTime / iDistQueriesDiv );
	if ( g_bIOStats )
	{
		if ( dStatus.MatchAdd ( "avg_query_reads" ) )
			dStatus.Add().SetSprintf ( "%.1f", (float)( g_tStats.m_iDiskReads*10/iQueriesDiv )/10.0f );
		if ( dStatus.MatchAdd ( "avg_query_readkb" ) )
			dStatus.Add().SetSprintf ( "%.1f", (float)( g_tStats.m_iDiskReadBytes/iQueriesDiv )/1024.0f );
		if ( dStatus.MatchAdd ( "avg_query_readtime" ) )
			FormatMsec ( dStatus.Add(), g_tStats.m_iDiskReadTime/iQueriesDiv );
	} else
	{
		if ( dStatus.MatchAdd ( "avg_query_reads" ) )
			dStatus.Add() = OFF;
		if ( dStatus.MatchAdd ( "avg_query_readkb" ) )
			dStatus.Add() = OFF;
		if ( dStatus.MatchAdd ( "avg_query_readtime" ) )
			dStatus.Add() = OFF;
	}

	const QcacheStatus_t & s = QcacheGetStatus();
	if ( dStatus.MatchAdd ( "qcache_max_bytes" ) )
		dStatus.Add().SetSprintf ( INT64_FMT, s.m_iMaxBytes );
	if ( dStatus.MatchAdd ( "qcache_thresh_msec" ) )
		dStatus.Add().SetSprintf ( "%d", s.m_iThreshMs );
	if ( dStatus.MatchAdd ( "qcache_ttl_sec" ) )
		dStatus.Add().SetSprintf ( "%d", s.m_iTtlS );

	if ( dStatus.MatchAdd ( "qcache_cached_queries" ) )
		dStatus.Add().SetSprintf ( "%d", s.m_iCachedQueries );
	if ( dStatus.MatchAdd ( "qcache_used_bytes" ) )
		dStatus.Add().SetSprintf ( INT64_FMT, s.m_iUsedBytes );
	if ( dStatus.MatchAdd ( "qcache_hits" ) )
		dStatus.Add().SetSprintf ( INT64_FMT, s.m_iHits );

	// clusters
	ReplicateClustersStatus ( dStatus );
}

// that is returned to MySQL 'statistic' command ('status' in mysql cli)
void BuildStatusOneline ( StringBuilder_c & sOut )
{
	sOut.StartBlock ( " " );
	sOut
	<< "Uptime:" << (DWORD) time ( NULL )-g_tStats.m_uStarted
	<< " Threads:" << GlobalWorkPool()->WorkingThreads()
	<< " Queue:" << GlobalWorkPool()->Works()
	<< " Clients:" << myinfo::CountClients()
	<< " Tasks:" << myinfo::CountAll()
	<< " Queries:" << g_tStats.m_iQueries;
	sOut.Sprintf ( " Wall: %t", (int64_t)g_tStats.m_iQueryTime );
	sOut.Sprintf ( " CPU: %t", (int64_t)g_tStats.m_iQueryCpuTime );
}

void BuildOneAgentStatus ( VectorLike & dStatus, HostDashboard_t* pDash, const char * sPrefix="agent" )
{
	assert ( pDash );

	const char * FMT64 = UINT64_FMT;
	const char * FLOAT = "%.2f";

	{
		CSphScopedRLock tGuard ( pDash->m_dMetricsLock );
		if ( dStatus.MatchAddVa ( "%s_hostname", sPrefix ) )
			dStatus.Add().SetSprintf ( "%s", pDash->m_tHost.GetMyUrl().cstr() );

		if ( dStatus.MatchAddVa ( "%s_references", sPrefix ) )
			dStatus.Add().SetSprintf ( "%d", (int) pDash->GetRefcount()-1 ); // -1 since we currently also 'use' the agent, reading it's stats
		if ( dStatus.MatchAddVa ( "%s_ping", sPrefix ) )
			dStatus.Add ().SetSprintf ( "%s", pDash->m_iNeedPing ? "yes" : "no" );
		if ( dStatus.MatchAddVa ( "%s_has_perspool", sPrefix ) )
			dStatus.Add ().SetSprintf ( "%s", pDash->m_pPersPool ? "yes" : "no" );
		if ( dStatus.MatchAddVa ( "%s_need_resolve", sPrefix ) )
			dStatus.Add ().SetSprintf ( "%s", pDash->m_tHost.m_bNeedResolve ? "yes" : "no" );
		uint64_t iCur = sphMicroTimer();
		uint64_t iLastAccess = iCur - pDash->m_iLastQueryTime;
		float fPeriod = (float)iLastAccess/1000000.0f;
		if ( dStatus.MatchAddVa ( "%s_lastquery", sPrefix ) )
			dStatus.Add().SetSprintf ( FLOAT, fPeriod );
		iLastAccess = iCur - pDash->m_iLastAnswerTime;
		fPeriod = (float)iLastAccess/1000000.0f;
		if ( dStatus.MatchAddVa ( "%s_lastanswer", sPrefix ) )
			dStatus.Add().SetSprintf ( FLOAT, fPeriod );
		uint64_t iLastTimer = pDash->m_iLastAnswerTime-pDash->m_iLastQueryTime;
		if ( dStatus.MatchAddVa ( "%s_lastperiodmsec", sPrefix ) )
			dStatus.Add().SetSprintf ( FMT64, iLastTimer/1000 );
		if ( dStatus.MatchAddVa ( "%s_errorsarow", sPrefix ) )
			dStatus.Add().SetSprintf ( FMT64, pDash->m_iErrorsARow );
	}
	int iPeriods = 1;

	while ( iPeriods>0 )
	{
		HostMetricsSnapshot_t dMetricsSnapshot;
		pDash->GetCollectedMetrics ( dMetricsSnapshot, iPeriods );
		{
			for ( int j = 0; j<ehMaxStat+eMaxAgentStat; ++j )
				// hack. Avoid microseconds in human-readable statistic
				if ( j==ehTotalMsecs && dStatus.MatchAddVa ( "%s_%dperiods_msecsperqueryy", sPrefix, iPeriods ) )
				{
					if ( dMetricsSnapshot[ehConnTries]>0 )
						dStatus.Add ().SetSprintf ( FLOAT, (float) ((dMetricsSnapshot[ehTotalMsecs] / 1000.0)
																	/ dMetricsSnapshot[ehConnTries]));
					else
						dStatus.Add ( "n/a" );
				} else if ( dStatus.MatchAddVa ( "%s_%dperiods_%s", sPrefix, iPeriods, sAgentStatsNames[j] ) )
				{
					if ( j==ehMaxMsecs || j==ehAverageMsecs )
						dStatus.Add ().SetSprintf ( FLOAT, (float) dMetricsSnapshot[j] / 1000.0 );
					else
						dStatus.Add ().SetSprintf ( FMT64, dMetricsSnapshot[j] );
				}
		}

		if ( iPeriods==1 )
			iPeriods = 5;
		else if ( iPeriods==5 )
			iPeriods = STATS_DASH_PERIODS;
		else if ( iPeriods==STATS_DASH_PERIODS )
			iPeriods = -1;
	}
}

static bool BuildDistIndexStatus ( VectorLike & dStatus, const CSphString& sIndex )
{
	auto pDistr = GetDistr ( sIndex );
	if ( !pDistr )
		return false;

	ARRAY_FOREACH ( i, pDistr->m_dLocal )
	{
		if ( dStatus.MatchAddVa ( "dstindex_local_%d", i+1 ) )
			dStatus.Add ( pDistr->m_dLocal[i].cstr() );
	}

	CSphString sKey;
	ARRAY_FOREACH ( i, pDistr->m_dAgents )
	{
		MultiAgentDesc_c & tAgents = *pDistr->m_dAgents[i];
		if ( dStatus.MatchAddVa ( "dstindex_%d_is_ha", i+1 ) )
			dStatus.Add ( tAgents.IsHA()? "1": "0" );

		auto dWeights = tAgents.GetWeights ();

		ARRAY_FOREACH ( j, tAgents )
		{
			if ( tAgents.IsHA() )
				sKey.SetSprintf ( "dstindex_%dmirror%d", i+1, j+1 );
			else
				sKey.SetSprintf ( "dstindex_%dagent", i+1 );

			const AgentDesc_t & dDesc = tAgents[j];

			if ( dStatus.MatchAddVa ( "%s_id", sKey.cstr() ) )
				dStatus.Add().SetSprintf ( "%s:%s", dDesc.GetMyUrl().cstr(), dDesc.m_sIndexes.cstr() );

			if ( tAgents.IsHA() && dStatus.MatchAddVa ( "%s_probability_weight", sKey.cstr() ) )
				dStatus.Add ().SetSprintf ( "%0.2f%%", dWeights[j] );

			if ( dStatus.MatchAddVa ( "%s_is_blackhole", sKey.cstr() ) )
				dStatus.Add ( dDesc.m_bBlackhole ? "1" : "0" );

			if ( dStatus.MatchAddVa ( "%s_is_persistent", sKey.cstr() ) )
				dStatus.Add ( dDesc.m_bPersistent ? "1" : "0" );
		}
	}
	return true;
}

void BuildAgentStatus ( VectorLike &dStatus, const CSphString& sIndexOrAgent )
{
	if ( !sIndexOrAgent.IsEmpty() )
	{
		if ( !BuildDistIndexStatus ( dStatus, sIndexOrAgent ) )
		{
			auto pAgent = Dashboard::FindAgent ( sIndexOrAgent );
			if ( pAgent )
				BuildOneAgentStatus ( dStatus, pAgent );
			else if ( dStatus.MatchAdd ( "status_error" ) )
				dStatus.Add ().SetSprintf ( "No such distributed index or agent: %s", sIndexOrAgent.cstr () );
		}
		return;
	}

	dStatus.m_sColKey = "Key";

	if ( dStatus.MatchAdd ( "status_period_seconds" ) )
		dStatus.Add().SetSprintf ( "%d", g_uHAPeriodKarmaS );
	if ( dStatus.MatchAdd ( "status_stored_periods" ) )
		dStatus.Add().SetSprintf ( "%d", STATS_DASH_PERIODS );

	auto dDashes = Dashboard::GetActiveHosts();

	CSphString sPrefix;
	ARRAY_FOREACH ( i, dDashes )
	{
		sPrefix.SetSprintf ( "ag_%d", i );
		BuildOneAgentStatus ( dStatus, dDashes[i], sPrefix.cstr() );
	}
}

static void AddIOStatsToMeta ( VectorLike & dStatus, const CSphIOStats & tStats, const char * sPrefix )
{
	if ( dStatus.MatchAddVa ( "%s%s", sPrefix, "io_read_time" ) )
		dStatus.Add().SetSprintf ( "%d.%03d", (int)( tStats.m_iReadTime/1000 ), (int)( tStats.m_iReadTime%1000 ) );

	if ( dStatus.MatchAddVa ( "%s%s", sPrefix, "io_read_ops" ) )
		dStatus.Add().SetSprintf ( "%u", tStats.m_iReadOps );

	if ( dStatus.MatchAddVa ( "%s%s", sPrefix, "io_read_kbytes" ) )
		dStatus.Add().SetSprintf ( "%d.%d", (int)( tStats.m_iReadBytes/1024 ), (int)( tStats.m_iReadBytes%1024 )/100 );

	if ( dStatus.MatchAddVa ( "%s%s", sPrefix, "io_write_time" ) )
		dStatus.Add().SetSprintf ( "%d.%03d", (int)( tStats.m_iWriteTime/1000 ), (int)( tStats.m_iWriteTime%1000 ) );

	if ( dStatus.MatchAddVa ( "%s%s", sPrefix, "io_write_ops" ) )
		dStatus.Add().SetSprintf ( "%u", tStats.m_iWriteOps );

	if ( dStatus.MatchAddVa ( "%s%s", sPrefix, "io_write_kbytes" ) )
		dStatus.Add().SetSprintf ( "%d.%d", (int)( tStats.m_iWriteBytes/1024 ), (int)( tStats.m_iWriteBytes%1024 )/100 );
}

void BuildMeta ( VectorLike & dStatus, const CSphQueryResultMeta & tMeta )
{
	if ( !tMeta.m_sError.IsEmpty() && dStatus.MatchAdd ( "error" ) )
		dStatus.Add ( tMeta.m_sError );

	if ( !tMeta.m_sWarning.IsEmpty() && dStatus.MatchAdd ( "warning" ) )
		dStatus.Add ( tMeta.m_sWarning );

	if ( dStatus.MatchAdd ( "total" ) )
		dStatus.Add().SetSprintf ( "%d", tMeta.m_iMatches );

	if ( dStatus.MatchAdd ( "total_found" ) )
		dStatus.Add().SetSprintf ( INT64_FMT, tMeta.m_iTotalMatches );

	if ( dStatus.MatchAdd ( "time" ) )
		dStatus.Add().SetSprintf ( "%d.%03d", tMeta.m_iQueryTime/1000, tMeta.m_iQueryTime%1000 );

	if ( tMeta.m_iMultiplier>1 && dStatus.MatchAdd ( "multiplier" ) )
		dStatus.Add().SetSprintf ( "%d", tMeta.m_iMultiplier );

	if ( g_bCpuStats )
	{
		if ( dStatus.MatchAdd ( "cpu_time" ) )
			dStatus.Add().SetSprintf ( "%d.%03d", (int)( tMeta.m_iCpuTime/1000 ), (int)( tMeta.m_iCpuTime%1000 ) );

		if ( dStatus.MatchAdd ( "agents_cpu_time" ) )
			dStatus.Add().SetSprintf ( "%d.%03d", (int)( tMeta.m_iAgentCpuTime/1000 ), (int)( tMeta.m_iAgentCpuTime%1000 ) );
	}

	if ( g_bIOStats )
	{
		AddIOStatsToMeta ( dStatus, tMeta.m_tIOStats, "" );
		AddIOStatsToMeta ( dStatus, tMeta.m_tAgentIOStats, "agent_" );
	}

	if ( tMeta.m_bHasPrediction )
	{
		if ( dStatus.MatchAdd ( "local_fetched_docs" ) )
			dStatus.Add().SetSprintf ( "%d", tMeta.m_tStats.m_iFetchedDocs );
		if ( dStatus.MatchAdd ( "local_fetched_hits" ) )
			dStatus.Add().SetSprintf ( "%d", tMeta.m_tStats.m_iFetchedHits );
		if ( dStatus.MatchAdd ( "local_fetched_skips" ) )
			dStatus.Add().SetSprintf ( "%d", tMeta.m_tStats.m_iSkips );

		if ( dStatus.MatchAdd ( "predicted_time" ) )
			dStatus.Add().SetSprintf ( "%lld", INT64 ( tMeta.m_iPredictedTime ) );
		if ( tMeta.m_iAgentPredictedTime && dStatus.MatchAdd ( "dist_predicted_time" ) )
			dStatus.Add().SetSprintf ( "%lld", INT64 ( tMeta.m_iAgentPredictedTime ) );
		if ( tMeta.m_iAgentFetchedDocs || tMeta.m_iAgentFetchedHits || tMeta.m_iAgentFetchedSkips )
		{
			if ( dStatus.MatchAdd ( "dist_fetched_docs" ) )
				dStatus.Add().SetSprintf ( "%d", tMeta.m_tStats.m_iFetchedDocs + tMeta.m_iAgentFetchedDocs );
			if ( dStatus.MatchAdd ( "dist_fetched_hits" ) )
				dStatus.Add().SetSprintf ( "%d", tMeta.m_tStats.m_iFetchedHits + tMeta.m_iAgentFetchedHits );
			if ( dStatus.MatchAdd ( "dist_fetched_skips" ) )
				dStatus.Add().SetSprintf ( "%d", tMeta.m_tStats.m_iSkips + tMeta.m_iAgentFetchedSkips );
		}
	}


	int iWord = 0;
	for ( const auto& dWord : tMeta.m_hWordStats )
	{
		if ( dStatus.MatchAddVa ( "keyword[%d]", iWord ) )
			dStatus.Add ( dWord.first );

		if ( dStatus.MatchAddVa ( "docs[%d]", iWord ) )
			dStatus.Add().SetSprintf ( INT64_FMT, dWord.second.first );

		if ( dStatus.MatchAddVa ( "hits[%d]", iWord ) )
			dStatus.Add().SetSprintf ( INT64_FMT, dWord.second.second );

		++iWord;
	}
}


void HandleCommandStatus ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_STATUS, tOut ) )
		return;

	bool bGlobalStat = tReq.GetDword ()!=0;

	VectorLike dStatus;

	if ( bGlobalStat )
		BuildStatus ( dStatus );
	else
	{
		ScRL_t dMetaRlock ( g_tLastMetaLock );
		BuildMeta ( dStatus, g_tLastMeta );
		if ( g_tStats.m_iPredictedTime || g_tStats.m_iAgentPredictedTime )
		{
			if ( dStatus.MatchAdd ( "predicted_time" ) )
				dStatus.Add().SetSprintf ( INT64_FMT, (int64_t) g_tStats.m_iPredictedTime );
			if ( dStatus.MatchAdd ( "dist_predicted_time" ) )
				dStatus.Add().SetSprintf ( INT64_FMT, (int64_t) g_tStats.m_iAgentPredictedTime );
		}
	}

	auto tReply = APIAnswer ( tOut, VER_COMMAND_STATUS );
	tOut.SendInt ( dStatus.GetLength()/2 ); // rows
	tOut.SendInt ( 2 ); // cols
	for ( const auto & dLines : dStatus )
		tOut.SendString ( dLines.cstr() );
}

//////////////////////////////////////////////////////////////////////////
// FLUSH HANDLER
//////////////////////////////////////////////////////////////////////////
void HandleCommandFlush ( ISphOutputBuffer & tOut, WORD uVer )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_FLUSHATTRS, tOut ) )
		return;

	int iTag = CommandFlush ();
	// return last flush tag, just for the fun of it
	auto tReply = APIAnswer ( tOut, VER_COMMAND_FLUSHATTRS );
	tOut.SendInt ( iTag );
}


/////////////////////////////////////////////////////////////////////////////
// GENERAL HANDLER
/////////////////////////////////////////////////////////////////////////////

void HandleCommandSphinxql ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq ); // definition is below
void HandleCommandJson ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq );
void StatCountCommand ( SearchdCommand_e eCmd );
void HandleCommandUserVar ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq );
void HandleCommandCallPq ( ISphOutputBuffer &tOut, WORD uVer, InputBuffer_c &tReq );

/// ping/pong exchange over API
void HandleCommandPing ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_PING, tOut ) )
		return;

	// parse ping
	int iCookie = tReq.GetInt();
	if ( tReq.GetError () )
		return;

	// return last flush tag, just for the fun of it
	auto tReply = APIAnswer ( tOut, VER_COMMAND_PING );
	tOut.SendInt ( iCookie ); // echo the cookie back
}


bool LoopClientSphinx ( SearchdCommand_e eCommand, WORD uCommandVer, int iLength,
	InputBuffer_c & tBuf, ISphOutputBuffer & tOut, bool bManagePersist )
{
	myinfo::ThdState ( ThdState_e::QUERY );
	// set on query guard
	auto& tCrashQuery = GlobalCrashQueryGetRef();
	tCrashQuery.m_pQuery = tBuf.GetBufferPtr();
	tCrashQuery.m_iSize = iLength;
	tCrashQuery.m_bMySQL = false;
	tCrashQuery.m_uCMD = eCommand;
	tCrashQuery.m_uVer = uCommandVer;

	// handle known commands
	assert ( eCommand<SEARCHD_COMMAND_WRONG );

	// count commands
	StatCountCommand ( eCommand );
	myinfo::SetCommand ( g_dApiCommands[eCommand] );

	bool bPersist = false;
	sphLogDebugv ( "conn %s(%d): got command %d, handling", myinfo::szClientName(), myinfo::ConnID(), eCommand );
	switch ( eCommand )
	{
		case SEARCHD_COMMAND_SEARCH:	HandleCommandSearch ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_EXCERPT:	HandleCommandExcerpt ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_KEYWORDS:	HandleCommandKeywords ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_UPDATE:	HandleCommandUpdate ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_PERSIST:
			{
				bPersist = ( tBuf.GetInt()!=0 );
				sphLogDebugv ( "conn %s(%d): pconn is now %s", myinfo::szClientName (), myinfo::ConnID(), bPersist ? "on" : "off" );
				if ( !bManagePersist ) // thread pool handles all persist connections
					break;
			}
			break;
		case SEARCHD_COMMAND_STATUS:	HandleCommandStatus ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_FLUSHATTRS:HandleCommandFlush ( tOut, uCommandVer ); break;
		case SEARCHD_COMMAND_SPHINXQL:	HandleCommandSphinxql ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_JSON:		HandleCommandJson ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_PING:		HandleCommandPing ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_UVAR:		HandleCommandUserVar ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_CALLPQ:	HandleCommandCallPq ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_CLUSTERPQ:	HandleCommandClusterPq ( tOut, uCommandVer, tBuf, myinfo::szClientName () ); break;
		case SEARCHD_COMMAND_GETFIELD:	HandleCommandGetField ( tOut, uCommandVer, tBuf ); break;
		default:						assert ( 0 && "internal error: unhandled command" ); break;
	}

	return bPersist;
}

class TableLike : public CheckLike
				, public ISphNoncopyable
{
	RowBuffer_i & m_tOut;
public:

	explicit TableLike ( RowBuffer_i & tOut, const char * sPattern = nullptr )
		: CheckLike ( sPattern )
		, m_tOut ( tOut )
	{}

	bool MatchAdd ( const char* sValue )
	{
		if ( Match ( sValue ) )
		{
			m_tOut.PutString ( sValue );
			return true;
		}
		return false;
	}

	bool MatchAddVa ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) )
	{
		va_list ap;
		CSphString sValue;

		va_start ( ap, sTemplate );
		sValue.SetSprintfVa ( sTemplate, ap );
		va_end ( ap );

		return MatchAdd ( sValue.cstr() );
	}

	// popular pattern of 2 columns of data
	inline void MatchData2 ( const char * pLeft, const char * pRight )
	{
		if ( Match ( pLeft ) )
			m_tOut.DataTuplet ( pLeft, pRight );
	}

	inline void MatchData2 ( const char * pLeft, int64_t iRight )
	{
		if ( Match ( pLeft ) )
			m_tOut.DataTuplet ( pLeft, iRight );
	}

	inline void MatchData3 ( const char * pStr1, const char * pStr2, const char * pStr3 )
	{
		if ( Match ( pStr1 ) )
		{
			m_tOut.PutString ( pStr1 );
			m_tOut.PutString ( pStr2 );
			m_tOut.PutString ( pStr3 );
			m_tOut.Commit();
		}
	}

	// popular pattern of 2 columns of data
	void DataTuplet ( const char * pLeft, const char * pRight )
	{
		MatchData2 ( pLeft, pRight );
	}

	void DataTuplet ( const char * pLeft, int64_t iRight )
	{
		MatchData2 ( pLeft, iRight );
	}
};


class StmtErrorReporter_c : public StmtErrorReporter_i
{
public:
	explicit StmtErrorReporter_c ( RowBuffer_i & tBuffer )
		: m_tRowBuffer ( tBuffer )
	{}

	void Ok ( int iAffectedRows, const CSphString & sWarning, int64_t iLastInsertId ) final
	{
		m_tRowBuffer.Ok ( iAffectedRows, ( sWarning.IsEmpty() ? 0 : 1 ), nullptr, false, iLastInsertId );
	}

	void Ok ( int iAffectedRows, int nWarnings ) final
	{
		m_tRowBuffer.Ok ( iAffectedRows, nWarnings );
	}

	void Error ( const char * sStmt, const char * sError, MysqlErrors_e iErr ) final
	{
		m_tRowBuffer.Error ( sStmt, sError, iErr );
	}

	RowBuffer_i * GetBuffer() final { return &m_tRowBuffer; }

private:
	RowBuffer_i & m_tRowBuffer;
};


static bool String2JsonPack ( char * pStr, CSphVector<BYTE> & dBuf, CSphString & sError, CSphString & sWarning )
{
	dBuf.Resize ( 0 ); // buffer for JSON parser must be empty to properly set JSON_ROOT data
	if ( !pStr )
		return true;

	if ( !sphJsonParse ( dBuf, pStr, g_bJsonAutoconvNumbers, g_bJsonKeynamesToLowercase, true, sError ) )
	{
		if ( g_bJsonStrict )
			return false;

		if ( sWarning.IsEmpty() )
			sWarning = sError;
		else
			sWarning.SetSprintf ( "%s; %s", sWarning.cstr(), sError.cstr() );

		sError = "";
	}

	return true;
}

struct StringPtrTraits_t
{
	CSphVector<BYTE> m_dPackedData;
	CSphFixedVector<int> m_dOff { 0 };
	CSphVector<BYTE> m_dParserBuf;

	// remap offsets to string pointers
	void SavePointersTo ( VecTraits_T<const char *> &dStrings, bool bSkipInvalid=true ) const
	{
		if ( bSkipInvalid )
			ARRAY_FOREACH ( i, m_dOff )
			{
				int iOff = m_dOff[i];
				if ( iOff<0 )
					continue;
				dStrings[i] = ( const char * ) m_dPackedData.Begin () + iOff;
			}
		else
			ARRAY_FOREACH ( i, m_dOff )
			{
				int iOff = m_dOff[i];
				dStrings[i] = ( iOff>=0 ? ( const char * ) m_dPackedData.Begin () + iOff : nullptr );
			}
	}

	void Reset ()
	{
		m_dPackedData.Resize ( 0 );
		m_dParserBuf.Resize ( 0 );
		m_dOff.Fill ( -1 );
	}

	BYTE * ReserveBlob ( int iBlobSize, int iOffset )
	{
		if ( !iBlobSize )
			return nullptr;

		m_dOff[iOffset] = m_dPackedData.GetLength ();

		BYTE * pPacked = m_dPackedData.AddN ( sphCalcPackedLength(iBlobSize) );
		pPacked += sphZipToPtr ( pPacked, iBlobSize );
		return pPacked;
	}
};

static void BsonToSqlInsert ( const bson::Bson_c& dBson, SqlInsert_t& tAttr )
{
	switch ( dBson.GetType () )
	{
	case JSON_INT32:
	case JSON_INT64: tAttr.m_iType = TOK_CONST_INT;
		tAttr.m_iVal = dBson.Int ();
		break;
	case JSON_DOUBLE: tAttr.m_iType = TOK_CONST_FLOAT;
		tAttr.m_fVal = float ( dBson.Double () );
		break;
	case JSON_STRING: tAttr.m_iType = TOK_QUOTED_STRING;
		tAttr.m_sVal = dBson.String ();
	default: break;
	}
}


template<typename T>
static int CopyBsonValues ( CSphVector<int64_t> & dMva, const bson::Bson_c & dBson )
{
	auto dValues = bson::Vector<T> ( dBson );
	int64_t * pDst = dMva.AddN ( dValues.GetLength() );
	ARRAY_FOREACH ( i, dValues )
		pDst[i] = dValues[i];

	return dValues.GetLength();
}


// save bson array to 64 bit mvaint64 mva
static int BsonArrayToMva ( CSphVector<int64_t> & dMva, const bson::Bson_c & dBson )
{
	int iOff = dMva.GetLength ();
	dMva.Add ();
	int iValues = 0;

	if ( dBson.GetType ()==JSON_INT64_VECTOR )
		iValues = CopyBsonValues<int64_t> ( dMva, dBson );
	else if ( dBson.GetType ()==JSON_INT32_VECTOR )
		iValues = CopyBsonValues<DWORD> ( dMva, dBson );
	else
	{ // slowest path - m.b. need conversion of every value
		bson::BsonIterator_c dIter ( dBson );
		iValues = dIter.NumElems();
		int64_t * pDst = dMva.AddN(iValues);
		for ( ; dIter; dIter.Next () )
			*pDst++ = dIter.Int ();
	}

	if ( !iValues ) // empty mva; discard resize
	{
		dMva.Resize ( iOff );
		return -1;
	}

	auto pDst = &dMva[iOff + 1];

	sphSort ( pDst, iValues );
	iValues = sphUniq ( pDst, iValues );
	dMva[iOff] = iValues;
	dMva.Resize ( iOff + iValues + 1 );
	return iOff;
}


static bool ParseBsonDocument ( const VecTraits_T<BYTE>& dDoc, const SchemaItemHash_c &tLoc,
	const CSphString & sIdAlias, int iRow, VecTraits_T<VecTraits_T<const char>>& dFields, CSphMatchVariant & tDoc,
	StringPtrTraits_t & tStrings, CSphVector<int64_t> & dMva, const CSphSchema &tSchema, Warner_c & sMsg )
{
	using namespace bson;
	Bson_c dBson ( dDoc );
	if ( dDoc.IsEmpty () )
		return false;

	SqlInsert_t tAttr;

	const SchemaItemVariant_t * pId = sIdAlias.IsEmpty () ? nullptr : tLoc.Find ( sphFNV64 ( sIdAlias.cstr() ) );
	const CSphColumnInfo * pIdCol = tSchema.GetAttr ( sphGetDocidName () );
	assert(pIdCol);
	CSphAttrLocator tIdLoc = pIdCol->m_tLocator;
	tIdLoc.m_bDynamic = true;

	BsonIterator_c dChild ( dBson );
	for ( ; dChild; dChild.Next () )
	{
		CSphString sName = dChild.GetName ();
		sphColumnToLowercase ( const_cast<char *>( sName.cstr() ) );
		const SchemaItemVariant_t * pItem = tLoc.Find ( sphFNV64 ( sName.cstr() ) );

		// FIXME!!! warn on unknown JSON fields
		if ( pItem )
		{
			if ( pItem->m_iField!=-1 && dChild.IsString () )
			{
				// stripper prior to build hits does not preserve field length
				// but works with \0 strings and could walk all document and modifies it and alter field length
				const VecTraits_T<const char> tField = Vector<const char> ( dChild );
				if ( tField.GetLength() )
				{
					int64_t iOff = tStrings.m_dPackedData.GetLength();

					// copy field content with tail zeroes
					BYTE * pDst = tStrings.m_dPackedData.AddN ( tField.GetLength() + 1 + CSphString::GetGap() );
					memcpy ( pDst, tField.Begin(), tField.GetLength() );
					memset ( pDst + tField.GetLength(), 0, 1 + CSphString::GetGap() );

					// pack offset into pointer then restore pointer after m_dPackedData filed
					dFields[pItem->m_iField] = VecTraits_T<const char> ( (const char *)iOff, tField.GetLength() );
				} else
				{
					dFields[pItem->m_iField] = tField;
				}

				if ( pItem==pId )
					sMsg.Warn ( "field '%s' requested as docs_id identifier, but it is field!", sName.cstr() );
			} else
			{
				BsonToSqlInsert ( dChild, tAttr );
				tDoc.SetAttr ( pItem->m_tLoc, tAttr, pItem->m_eType );
				if ( pId==pItem )
					((CSphMatch &)tDoc).SetAttr ( tIdLoc, (DocID_t)dChild.Int() );

				switch ( pItem->m_eType )
				{
				case SPH_ATTR_JSON:
					assert ( pItem->m_iStr!=-1 );
					{
						if ( dChild.IsAssoc() || dChild.IsArray() )
						{
							// just save bson blob
							BYTE * pDst = tStrings.ReserveBlob ( dChild.StandaloneSize(), pItem->m_iStr );
							dChild.BsonToBson ( pDst );
						} else
						{
							sMsg.Warn ( "JSON item (%s) should be object or array, got=%s", sName.cstr(), JsonTypeName ( dChild.GetType() ) );
						}
					}
					break;
				case SPH_ATTR_STRING:
					assert ( pItem->m_iStr!=-1 );
					{
						auto dStrBlob = RawBlob ( dChild );
						if ( dStrBlob.second )
						{
							tStrings.m_dOff[pItem->m_iStr] = tStrings.m_dPackedData.GetLength ();
							BYTE * sDst = tStrings.m_dPackedData.AddN ( 1 + dStrBlob.second + CSphString::GetGap () );
							memcpy ( sDst, dStrBlob.first, dStrBlob.second );
							memset ( sDst + dStrBlob.second, 0, 1 + CSphString::GetGap () );
						}
					}
					break;
				case SPH_ATTR_UINT32SET:
				case SPH_ATTR_INT64SET:
					assert ( pItem->m_iMva!=-1 );
					if ( dChild.IsArray() )
					{
						int iOff = BsonArrayToMva ( dMva, dChild );
						if ( iOff>=0 )
							dMva[pItem->m_iMva] = iOff;
					} else
					{
						sMsg.Warn ( "MVA item (%s) should be array, got %s", sName.cstr(), JsonTypeName ( dChild.GetType() ) );
					}
				default:
					break;
				}
			}
		} else if ( !sIdAlias.IsEmpty() && sIdAlias==sName )
		{
			((CSphMatch &)tDoc).SetAttr ( tIdLoc, (DocID_t)dChild.Int() );
		}
	}
	return true;
}


static void FixParsedMva ( const CSphVector<int64_t> & dParsed, CSphVector<int64_t> & dMva, int iCount )
{
	if ( !iCount )
		return;

	// dParsed:
	// 0 - iCount elements: offset to MVA values with leading MVA element count
	// Could be not in right order

	dMva.Resize ( 0 );
	for ( int i=0; i<iCount; ++i )
	{
		int iOff = dParsed[i];
		if ( !iOff )
		{
			dMva.Add ( 0 );
			continue;
		}

		DWORD uMvaCount = dParsed[iOff];
		int64_t * pMva = dMva.AddN ( uMvaCount + 1 );
		*pMva++ = uMvaCount;
		memcpy ( pMva, dParsed.Begin() + iOff + 1, sizeof(dMva[0]) * uMvaCount );
	}
}


class PqRequestBuilder_c : public RequestBuilder_i
{
	const BlobVec_t &m_dDocs;
	const PercolateOptions_t &m_tOpts;
	mutable CSphAtomic m_iWorker;
	int m_iStart;
	int m_iStep;

public:
	explicit PqRequestBuilder_c ( const BlobVec_t &dDocs, const PercolateOptions_t &tOpts, int iStart=0, int iStep=0 )
		: m_dDocs ( dDocs )
		, m_tOpts ( tOpts )
		, m_iStart ( iStart )
		, m_iStep ( iStep)
	{}

	void BuildRequest ( const AgentConn_t &tAgent, ISphOutputBuffer &tOut ) const final
	{
		// it sends either all queries to each agent or sequence of queries to current agent

		auto iWorker = tAgent.m_iStoreTag;
		if ( iWorker<0 )
		{
			iWorker = ( int ) m_iWorker++;
			tAgent.m_iStoreTag = iWorker;
		}

		const char * sIndex = tAgent.m_tDesc.m_sIndexes.cstr ();
		auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_CALLPQ, VER_COMMAND_CALLPQ );

		DWORD uFlags = 0;
		if ( m_tOpts.m_bGetDocs )
			uFlags = 1;
		if ( m_tOpts.m_bGetQuery )
			uFlags |= 2;
		if ( m_tOpts.m_bJsonDocs )
			uFlags |= 4;
		if ( m_tOpts.m_bVerbose )
			uFlags |= 8;
		if ( m_tOpts.m_bSkipBadJson )
			uFlags |= 16;

		tOut.SendDword ( uFlags );
		tOut.SendString ( m_tOpts.m_sIdAlias.cstr () );
		tOut.SendString ( sIndex );

		// send docs (all or chunk)
		int iStart = 0;
		int iStep = m_dDocs.GetLength();
		if ( m_iStep ) // sparsed case, calculate the interval.
		{
			iStart = m_iStart + m_iStep * iWorker;
			iStep = Min ( iStep - iStart, m_iStep );
		}
		tOut.SendInt ( iStart );
		tOut.SendInt ( iStep );
		for ( int i=iStart; i<iStart+iStep; ++i)
			tOut.SendArray ( m_dDocs[i] );
	}
};



class PqReplyParser_c : public ReplyParser_i
{
public:
	bool ParseReply ( MemInputBuffer_c &tReq, AgentConn_t &tAgent ) const final
	{
		//	auto &dQueries = m_pWorker->m_dQueries;
		//	int iDoc = m_pWorker->m_dTasks[tAgent.m_iStoreTag].m_iHead;

		auto pResult = ( CPqResult * ) tAgent.m_pResult.Ptr ();
		if ( !pResult )
		{
			pResult = new CPqResult;
			tAgent.m_pResult = pResult;
		}

		auto &dResult = pResult->m_dResult;
		auto uFlags = tReq.GetDword ();
		bool bDumpDocs = !!(uFlags & 1);
		bool bQuery = !!(uFlags & 2);
		bool bDeduplicatedDocs = !!(uFlags & 4);

		dResult.m_bGetDocs = bDumpDocs;
		dResult.m_bGetQuery = bQuery;
		CSphVector<int> dDocs;
		CSphVector<DocID_t> dDocids;
		dDocids.Add(0); // just to keep docids 1-based and so, simplify processing by avoid checks.

		int iRows = tReq.GetInt ();
		dResult.m_dQueryDesc.Reset ( iRows );
		for ( auto &tDesc : dResult.m_dQueryDesc )
		{
			tDesc.m_iQUID = tReq.GetUint64 ();
			if ( bDumpDocs )
			{
				int iCount = tReq.GetInt ();
				dDocs.Add ( iCount );
				if ( bDeduplicatedDocs )
				{
					for ( int iDoc = 0; iDoc<iCount; ++iDoc )
					{
						dDocs.Add ( dDocids.GetLength () );
						dDocids.Add ( ( int64_t ) tReq.GetUint64 () );
					}
				} else
				{
					for ( int iDoc = 0; iDoc<iCount; ++iDoc )
						dDocs.Add ( tReq.GetInt () );
				}
			}

			if ( bQuery )
			{
				auto uDescFlags = tReq.GetDword ();
				if ( uDescFlags & 1 )
					tDesc.m_sQuery = tReq.GetString ();
				if ( uDescFlags & 2 )
					tDesc.m_sTags = tReq.GetString ();
				if ( uDescFlags & 4 )
					tDesc.m_sFilters = tReq.GetString ();
				tDesc.m_bQL = !!(uDescFlags & 8);
			}
		}

		// meta
		dResult.m_tmTotal = tReq.GetUint64 ();
		dResult.m_tmSetup = tReq.GetUint64 ();
		dResult.m_iQueriesMatched = tReq.GetInt();
		dResult.m_iQueriesFailed = tReq.GetInt ();
		dResult.m_iDocsMatched = tReq.GetInt ();
		dResult.m_iTotalQueries = tReq.GetInt ();
		dResult.m_iOnlyTerms = tReq.GetInt ();
		dResult.m_iEarlyOutQueries = tReq.GetInt ();
		auto iDts = tReq.GetInt();
		dResult.m_dQueryDT.Reset ( iDts );
		for ( int& iDt : dResult.m_dQueryDT )
			iDt = tReq.GetInt();

		dResult.m_sMessages.Warn ( tReq.GetString () );

		auto iDocs = dDocs.GetLength ();
		dResult.m_dDocs.Set ( dDocs.LeakData (), iDocs );

		if ( dDocids.GetLength()>1 )
		{
			iDocs = dDocids.GetLength ();
			pResult->m_dDocids.Set ( dDocids.LeakData (), iDocs );
		}

		return true;
	}
};


static void SendAPIPercolateReply ( ISphOutputBuffer & tOut, const CPqResult & tResult, int iShift=0 )
{
	auto tReply = APIAnswer ( tOut, VER_COMMAND_CALLPQ );

	CSphVector<int64_t> dTmpDocs;
	int iDocOff = -1;

	const PercolateMatchResult_t &tRes = tResult.m_dResult;
	const CSphFixedVector<DocID_t> &dDocids = tResult.m_dDocids;
	bool bHasDocids = !dDocids.IsEmpty ();
	bool bDumpDocs = tRes.m_bGetDocs;
	bool bQuery = tRes.m_bGetQuery;

	DWORD uFlags = 0;

	if ( bDumpDocs )
		uFlags = 1;
	if ( bQuery )
		uFlags |=2;
	if ( bHasDocids )
		uFlags |=4;

	tOut.SendDword ( uFlags );

	tOut.SendInt ( tRes.m_dQueryDesc.GetLength () );
	for ( const auto &tDesc : tRes.m_dQueryDesc )
	{
		tOut.SendUint64 ( tDesc.m_iQUID );
		if ( bDumpDocs )
		{
			// document count + document id(s)
			auto iCount = ( int ) ( tRes.m_dDocs[++iDocOff] );
			if ( bHasDocids ) // need de-duplicate docs
			{
				dTmpDocs.Resize ( iCount );
				for ( int iDoc = 0; iDoc<iCount; ++iDoc )
				{
					int iRow = tRes.m_dDocs[++iDocOff];
					dTmpDocs[iDoc] = dDocids[iRow];
				}
				dTmpDocs.Uniq ();
				tOut.SendInt ( dTmpDocs.GetLength());
				for ( auto dTmpDoc : dTmpDocs )
					tOut.SendUint64 ( dTmpDoc );
			} else
			{
				tOut.SendInt ( iCount );
				for ( int iDoc = 0; iDoc<iCount; ++iDoc )
					tOut.SendInt ( iShift+tRes.m_dDocs[++iDocOff] );
			}
		}
		if ( bQuery )
		{
			DWORD uDescFlags = 0;
			if ( !tDesc.m_sQuery.IsEmpty ())
				uDescFlags |=1;
			if ( !tDesc.m_sQuery.IsEmpty () )
				uDescFlags |= 2;
			if ( !tDesc.m_sQuery.IsEmpty () )
				uDescFlags |= 4;
			if ( tDesc.m_bQL )
				uDescFlags |= 8;

			tOut.SendDword ( uDescFlags );
			if ( uDescFlags & 1 )
				tOut.SendString ( tDesc.m_sQuery.cstr () );
			if ( uDescFlags & 2 )
				tOut.SendString ( tDesc.m_sTags.cstr () );
			if ( uDescFlags & 4 )
				tOut.SendString ( tDesc.m_sFilters.cstr () );
		}
	}

	// send meta
	tOut.SendUint64 ( tRes.m_tmTotal );
	tOut.SendUint64 ( tRes.m_tmSetup );
	tOut.SendInt ( tRes.m_iQueriesMatched );
	tOut.SendInt ( tRes.m_iQueriesFailed );
	tOut.SendInt ( tRes.m_iDocsMatched );
	tOut.SendInt ( tRes.m_iTotalQueries );
	tOut.SendInt ( tRes.m_iOnlyTerms );
	tOut.SendInt ( tRes.m_iEarlyOutQueries );
	tOut.SendInt ( tRes.m_dQueryDT.GetLength () );
	for ( int iDT : tRes.m_dQueryDT )
		tOut.SendInt ( iDT );

	tOut.SendString ( tRes.m_sMessages.sWarning () );
}


static void SendMysqlPercolateReply ( RowBuffer_i & tOut, const CPqResult & tResult, int iShift=0 )
{
	// shortcuts
	const PercolateMatchResult_t &tRes = tResult.m_dResult;
	const CSphFixedVector<DocID_t> &dDocids = tResult.m_dDocids;

	bool bDumpDocs = tRes.m_bGetDocs;
	bool bQuery = tRes.m_bGetQuery;

	// result set header packet. We will attach EOF manually at the end.
	int iColumns = bDumpDocs ? 2 : 1;
	if ( bQuery )
		iColumns += 3;
	tOut.HeadBegin ( iColumns );

	tOut.HeadColumn ( "id", MYSQL_COL_LONGLONG, 0 );
	if ( bDumpDocs )
		tOut.HeadColumn ( "documents", MYSQL_COL_STRING );
	if ( bQuery )
	{
		tOut.HeadColumn ( "query" );
		tOut.HeadColumn ( "tags" );
		tOut.HeadColumn ( "filters" );
	}

	// EOF packet is sent explicitly due to non-default params.
	auto iWarns = tRes.m_sMessages.WarnEmpty () ? 0 : 1;
	tOut.HeadEnd ( false, iWarns );

	CSphVector<int64_t> dTmpDocs;
	int iDocOff = -1;
	StringBuilder_c sDocs;
	for ( const auto &tDesc : tRes.m_dQueryDesc )
	{
		tOut.PutNumAsString ( tDesc.m_iQUID );
		if ( bDumpDocs )
		{
			sDocs.StartBlock ( "," );
			// document count + document id(s)
			auto iCount = ( int ) ( tRes.m_dDocs[++iDocOff] );
			if ( dDocids.GetLength () ) // need de-duplicate docs
			{
				dTmpDocs.Resize ( iCount );
				for ( int iDoc = 0; iDoc<iCount; ++iDoc )
				{
					RowID_t tRow = tRes.m_dDocs[++iDocOff];
					dTmpDocs[iDoc] = dDocids[tRow];
				}
				dTmpDocs.Uniq ();
				for ( auto dTmpDoc : dTmpDocs )
					sDocs.Sprintf ( "%l", dTmpDoc );
			} else
			{
				for ( int iDoc = 0; iDoc<iCount; ++iDoc )
				{
					RowID_t tRow = tRes.m_dDocs[++iDocOff];
					sDocs.Sprintf ( "%u", tRow + iShift );
				}
			}

			tOut.PutString ( sDocs.cstr () );
			sDocs.Clear ();
		}
		if ( bQuery )
		{
			tOut.PutString ( tDesc.m_sQuery );
			tOut.PutString ( tDesc.m_sTags );
			tOut.PutString ( tDesc.m_sFilters );
		}

		tOut.Commit ();
	}

	tOut.Eof ( false, iWarns );
}

// process one(!) local(!) pq index
static void PQLocalMatch ( const BlobVec_t &dDocs, const CSphString& sIndex, const PercolateOptions_t & tOpt,
	CSphSessionAccum &tAcc, CPqResult &tResult, int iStart, int iDocs )
{
	CSphString sWarning, sError;
	auto &sMsg = tResult.m_dResult.m_sMessages;
	tResult.m_dResult.m_bGetDocs = tOpt.m_bGetDocs;
	tResult.m_dResult.m_bVerbose = tOpt.m_bVerbose;
	tResult.m_dResult.m_bGetQuery = tOpt.m_bGetQuery;
	sMsg.Clear ();

	if ( !iDocs || ( iStart + iDocs )>dDocs.GetLength () )
		iDocs = dDocs.GetLength () - iStart;

	if ( !iDocs )
	{
		sMsg.Warn ( "No more docs for sparse matching" );
		return;
	}

	ServedDescRPtr_c pServed ( GetServed ( sIndex ) );
	if ( !pServed || !pServed->m_pIndex )
	{
		sMsg.Err ( "unknown local index '%s' in search request", sIndex.cstr () );
		return;
	}

	if ( pServed->m_eType!=IndexType_e::PERCOLATE )
	{
		sMsg.Err ( "index '%s' is not percolate", sIndex.cstr () );
		return;
	}

	auto pIndex = ( PercolateIndex_i * ) pServed->m_pIndex;
	RtAccum_t * pAccum = tAcc.GetAcc ( pIndex, sError );
	sMsg.Err ( sError );

	if ( !sMsg.ErrEmpty () )
		return;


	const CSphSchema &tSchema = pIndex->GetInternalSchema ();
	int iFieldsCount = tSchema.GetFieldsCount ();
	CSphFixedVector<VecTraits_T<const char>> dFields ( iFieldsCount );

	// set defaults
	CSphMatchVariant tDoc;
	tDoc.Reset ( tSchema.GetRowSize () );
	int iAttrsCount = tSchema.GetAttrsCount ();
	for ( int i = 0; i<iAttrsCount; ++i )
	{
		const CSphColumnInfo &tCol = tSchema.GetAttr ( i );
		CSphAttrLocator tLoc = tCol.m_tLocator;
		tLoc.m_bDynamic = true;
		tDoc.SetDefaultAttr ( tLoc, tCol.m_eAttrType );
	}

	int iStrCounter = 0;
	int iMvaCounter = 0;
	SchemaItemHash_c hSchemaLocators;

	if ( tOpt.m_bJsonDocs )
	{

		// hash attrs
		for ( int i = 0; i<iAttrsCount; ++i )
		{
			const CSphColumnInfo &tCol = tSchema.GetAttr ( i );
			SchemaItemVariant_t tAttr;
			tAttr.m_tLoc = tCol.m_tLocator;
			tAttr.m_tLoc.m_bDynamic = true; /// was just set above
			tAttr.m_eType = tCol.m_eAttrType;
			if ( tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_JSON )
				tAttr.m_iStr = iStrCounter++;
			if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET )
				tAttr.m_iMva = iMvaCounter++;

			hSchemaLocators.Add ( sphFNV64 ( tCol.m_sName.cstr () ), tAttr );
		}
		for ( int i = 0; i<iFieldsCount; ++i )
		{
			const CSphColumnInfo &tField = tSchema.GetField ( i );
			SchemaItemVariant_t tAttr;
			tAttr.m_iField = i;
			hSchemaLocators.Add ( sphFNV64 ( tField.m_sName.cstr () ), tAttr );
		}
	} else
	{
		// even without JSON docs MVA should match to schema definition on inserting data into accumulator
		for ( int i = 0; i<iAttrsCount; ++i )
		{
			const CSphColumnInfo &tCol = tSchema.GetAttr ( i );
			if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET )
				++iMvaCounter;
		}
	}

	const CSphColumnInfo * pId = tSchema.GetAttr ( sphGetDocidName () );
	assert( pId );
	CSphAttrLocator tIdLoc = pId->m_tLocator;
	tIdLoc.m_bDynamic = true;

	int iDocsNoIdCount = 0;
	bool bAutoId = tOpt.m_sIdAlias.IsEmpty ();
	tResult.m_dDocids.Reset ( bAutoId ? 0 : iDocs + 1 );
	int64_t uSeqDocid = 1;

	CSphFixedVector<const char *> dStrings ( iStrCounter );
	StringPtrTraits_t tStrings;
	tStrings.m_dOff.Reset ( iStrCounter );
	CSphVector<int64_t> dMvaParsed ( iMvaCounter );
	CSphVector<int64_t> dMva;

	CSphString sTokenFilterOpts;
	RowID_t tRowID = 0;
	for ( auto iDoc = iStart; iDoc<iStart+iDocs; ++iDoc )
	{
		// doc-id
		((CSphMatch &)tDoc).SetAttr ( tIdLoc, 0 );
		dFields[0] = dDocs[iDoc];

		dMvaParsed.Resize ( iMvaCounter );
		dMvaParsed.Fill ( 0 );

		if ( tOpt.m_bJsonDocs )
		{
			// reset all back to defaults
			dFields.Fill ( { nullptr, 0 } );
			for ( int i = 0; i<iAttrsCount; ++i )
			{
				const CSphColumnInfo &tCol = tSchema.GetAttr ( i );
				CSphAttrLocator tLoc = tCol.m_tLocator;
				tLoc.m_bDynamic = true;
				tDoc.SetDefaultAttr ( tLoc, tCol.m_eAttrType );
			}

			tStrings.Reset ();

			if ( !ParseBsonDocument ( dDocs[iDoc], hSchemaLocators, tOpt.m_sIdAlias, iDoc,
						dFields, tDoc, tStrings, dMvaParsed, tSchema, sMsg ) )
			{
				// for now the only case of fail - if provided bson is empty (null) document.
				if ( tOpt.m_bSkipBadJson )
				{
					sMsg.Warn ( "ERROR: Document %d is empty", iDoc + tOpt.m_iShift + 1 );
					continue;
				}

				sMsg.Err ( "Document %d is empty", iDoc + tOpt.m_iShift + 1 );
				break;
			}

			tStrings.SavePointersTo ( dStrings, false );

			// convert back offset into tStrings buffer into pointers
			for ( VecTraits_T<const char> & tField : dFields )
			{
				if ( !tField.GetLength() )
					continue;

				int64_t iOff = int64_t( tField.Begin() );
				int iLen = tField.GetLength();
				tField = VecTraits_T<const char> ( (const char *)( tStrings.m_dPackedData.Begin()+iOff ), iLen );
			}
		}
		FixParsedMva ( dMvaParsed, dMva, iMvaCounter );

		if ( !sMsg.ErrEmpty () )
			break;


		tDoc.m_tRowID = ( RowID_t ) tRowID++;

		if ( !bAutoId )
		{
			// in user-provides-id mode let's skip all docs without id
			if ( !sphGetDocID ( tDoc.m_pDynamic ) )
			{
				++iDocsNoIdCount;
				continue;
			}

			// store provided doc-id for result set sending
			tResult.m_dDocids[uSeqDocid] = ( int64_t ) sphGetDocID ( tDoc.m_pDynamic );
			( ( CSphMatch & ) tDoc ).SetAttr ( tIdLoc, uSeqDocid++ );
		} else
			( ( CSphMatch & ) tDoc ).SetAttr ( tIdLoc, iDoc + 1 ); // +1 since docid is 1-based

		// PQ work with sequential document numbers, 0 element unused

		// add document
		pIndex->AddDocument ( dFields, tDoc, true, sTokenFilterOpts, dStrings.Begin (), dMva, sError, sWarning, pAccum );
		sMsg.Err ( sError );
		sMsg.Warn ( sWarning );

		if ( !sMsg.ErrEmpty () )
			break;
	}

	// fire exit
	if ( !sMsg.ErrEmpty () )
	{
		pIndex->RollBack ( pAccum ); // clean up collected data
		return;
	}

	pIndex->MatchDocuments ( pAccum, tResult.m_dResult );

	if ( iDocsNoIdCount )
		sMsg.Warn ( "skipped %d document(s) without id field '%s'", iDocsNoIdCount, tOpt.m_sIdAlias.cstr () );

}

void PercolateMatchDocuments ( const BlobVec_t & dDocs, const PercolateOptions_t & tOpts, CSphSessionAccum & tAcc, CPqResult & tResult )
{
	CSphString sIndex = tOpts.m_sIndex;
	CSphString sWarning, sError;

	StrVec_t dLocalIndexes;
	auto * pLocalIndexes = &dLocalIndexes;

	VecRefPtrsAgentConn_t dAgents;
	auto pDist = GetDistr ( sIndex );
	if ( pDist )
	{
		for ( auto * pAgent : pDist->m_dAgents )
		{
			auto * pConn = new AgentConn_t;
			pConn->SetMultiAgent ( pAgent );
			pConn->m_iMyConnectTimeoutMs = pDist->m_iAgentConnectTimeoutMs;
			pConn->m_iMyQueryTimeoutMs = pDist->m_iAgentQueryTimeoutMs;
			dAgents.Add ( pConn );
		}

		pLocalIndexes = &pDist->m_dLocal;
	} else
		dLocalIndexes.Add ( sIndex );

	// at this point we know total num of involved indexes,
	// and can eventually split (sparse) docs among them.
	int iChunks = 0;
	if ( tOpts.m_eMode==PercolateOptions_t::unknown || tOpts.m_eMode==PercolateOptions_t::sparsed)
		iChunks = dAgents.GetLength () + pLocalIndexes->GetLength ();
	int iStart = 0;
	int iStep = iChunks>1 ? ( ( dDocs.GetLength () - 1 ) / iChunks + 1 ) : 0;

	bool bHaveRemotes = !dAgents.IsEmpty ();
	int iSuccesses = 0;
	int iAgentsDone = 0;
	CSphScopedPtr<PqRequestBuilder_c> pReqBuilder { nullptr };
	CSphScopedPtr<ReplyParser_i> pParser { nullptr };
	CSphRefcountedPtr<RemoteAgentsObserver_i> pReporter { nullptr };
	if ( bHaveRemotes )
	{
		pReqBuilder = new PqRequestBuilder_c ( dDocs, tOpts, iStart, iStep );
		iStart += iStep * dAgents.GetLength ();
		pParser = new PqReplyParser_c;
		pReporter = GetObserver();
		ScheduleDistrJobs ( dAgents, pReqBuilder.Ptr(), pParser.Ptr(), pReporter );
	}

	LazyVector_T <CPqResult> dLocalResults;
	for ( const auto & sPqIndex : *pLocalIndexes )
	{
		auto & dResult = dLocalResults.Add();
		PQLocalMatch ( dDocs, sPqIndex, tOpts, tAcc, dResult, iStart, iStep );
		iStart += iStep;
	}

	if ( bHaveRemotes )
	{
		assert ( pReporter );
		pReporter->Finish ();
		iSuccesses = ( int ) pReporter->GetSucceeded ();
		iAgentsDone = ( int ) pReporter->GetFinished ();
	}

	LazyVector_T<CPqResult*> dAllResults;
	for ( auto & dLocalRes : dLocalResults )
		dAllResults.Add ( &dLocalRes );

	CPqResult dMsgs; // fake resultset just to grab errors from remotes
	if ( iAgentsDone>iSuccesses )
		dAllResults.Add ( &dMsgs );

	if ( iAgentsDone )
	{
		for ( auto * pAgent : dAgents )
		{
			if ( !pAgent->m_bSuccess )
			{
				dMsgs.m_dResult.m_sMessages.Err ( pAgent->m_sFailure );
				continue;
			}

			auto pResult = ( CPqResult * ) pAgent->m_pResult.Ptr ();
			if ( !pResult )
				continue;

			dAllResults.Add ( pResult );
		}
	}

	MergePqResults ( dAllResults, tResult, iChunks<2 );

	if ( iSuccesses!=iAgentsDone )
	{
		sphWarning ( "Remote PQ: some of the agents didn't answered: %d queried, %d finished, %d succeeded"
					 , dAgents.GetLength (), iAgentsDone, iSuccesses );

	}
}

/// call PQ command over API
void HandleCommandCallPq ( ISphOutputBuffer &tOut, WORD uVer, InputBuffer_c &tReq ) REQUIRES ( HandlerThread )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_CALLPQ, tOut ) )
		return;

	// options
	PercolateOptions_t tOpts;

	DWORD uFlags = tReq.GetDword ();
	tOpts.m_bGetDocs	= !!(uFlags & 1);
	tOpts.m_bGetQuery	= !!(uFlags & 2);
	tOpts.m_bJsonDocs	= !!(uFlags & 4);
	tOpts.m_bVerbose	= !!(uFlags & 8);
	tOpts.m_bSkipBadJson = !! ( uFlags & 16 );

	tOpts.m_sIdAlias = tReq.GetString();

	// index name
	tOpts.m_sIndex = tReq.GetString();

	// document(s)
	tOpts.m_iShift = tReq.GetInt();
	BlobVec_t dDocs ( tReq.GetInt() );
	for ( auto & sDoc : dDocs )
		if ( !tReq.GetString ( sDoc ) )
		{
			SendErrorReply ( tOut, "Can't retrieve doc from input buffer" );
			return;
		}

	// working
	CSphSessionAccum tAcc;
	CPqResult tResult;

	PercolateMatchDocuments ( dDocs, tOpts, tAcc, tResult );

	if ( tResult.m_dResult.m_iQueriesFailed )
		tResult.m_dResult.m_sMessages.Err ( "%d queries failed", tResult.m_dResult.m_iQueriesFailed );

	if ( !tResult.m_dResult.m_sMessages.ErrEmpty () )
	{
		SendErrorReply ( tOut, "%s", tResult.m_dResult.m_sMessages.sError() );
		return;
	}

	SendAPIPercolateReply ( tOut, tResult, tOpts.m_iShift );
}


static void HandleMysqlCallPQ ( RowBuffer_i & tOut, SqlStmt_t & tStmt, CSphSessionAccum & tAcc, CPqResult & tResult )
{

	PercolateMatchResult_t &tRes = tResult.m_dResult;
	tRes.Reset();

	// check arguments
	// index name, document | documents list, [named opts]
	if ( tStmt.m_dInsertValues.GetLength()!=2 )
	{
		tOut.Error ( tStmt.m_sStmt, "PQ() expects exactly 2 arguments (index, document(s))" );
		return;
	}
	auto &dStmtIndex = tStmt.m_dInsertValues[0];
	auto &dStmtDocs = tStmt.m_dInsertValues[1];

	if ( dStmtIndex.m_iType!=TOK_QUOTED_STRING )
	{
		tOut.Error ( tStmt.m_sStmt, "PQ() argument 1 must be a string" );
		return;
	}
	if ( dStmtDocs.m_iType!=TOK_QUOTED_STRING && dStmtDocs.m_iType!=TOK_CONST_STRINGS )
	{
		tOut.Error ( tStmt.m_sStmt, "PQ() argument 2 must be a string or a string list" );
		return;
	}

	// document(s)
	StrVec_t dDocs;
	if ( dStmtDocs.m_iType==TOK_QUOTED_STRING )
		dDocs.Add ( dStmtDocs.m_sVal );
	else
		dDocs.SwapData ( tStmt.m_dCallStrings );

	// options last
	CSphString sError;
	PercolateOptions_t tOpts;
	tOpts.m_sIndex = dStmtIndex.m_sVal;
	SqlParser_c::SplitClusterIndex ( tOpts.m_sIndex, nullptr );
	bool bSkipEmpty = false;
	ARRAY_FOREACH ( i, tStmt.m_dCallOptNames )
	{
		CSphString & sOpt = tStmt.m_dCallOptNames[i];
		const SqlInsert_t & v = tStmt.m_dCallOptValues[i];

		sOpt.ToLower();
		int iExpType = TOK_CONST_INT;

		if ( sOpt=="docs_id" )
		{
			tOpts.m_sIdAlias = v.m_sVal;
			iExpType = TOK_QUOTED_STRING;
			sphColumnToLowercase ( const_cast<char *>( tOpts.m_sIdAlias.cstr() ) );

		} else if ( sOpt=="docs" )		tOpts.m_bGetDocs = ( v.m_iVal!=0 );
		else if ( sOpt=="verbose" )		tOpts.m_bVerbose = ( v.m_iVal!=0 );
		else if ( sOpt=="docs_json" )	tOpts.m_bJsonDocs = ( v.m_iVal!=0 );
		else if ( sOpt=="query" )		tOpts.m_bGetQuery = ( v.m_iVal!=0 );
		else if ( sOpt=="skip_bad_json" )	tOpts.m_bSkipBadJson = ( v.m_iVal!=0 );
		else if ( sOpt=="skip_empty" ) 	bSkipEmpty = true;
		else if ( sOpt=="shift" ) 		tOpts.m_iShift = v.m_iVal;
		else if ( sOpt=="mode" )
		{
			auto sMode = v.m_sVal;
			iExpType = TOK_QUOTED_STRING;
			sMode.ToLower();
			if ( sMode=="sparsed" )
				tOpts.m_eMode = PercolateOptions_t::sparsed;
			else if ( sMode=="sharded" )
				tOpts.m_eMode = PercolateOptions_t::sharded;
			else
			{
				sError.SetSprintf ( "unknown mode %s. (Expected 'sparsed' or 'sharded')", v.m_sVal.cstr () );
				break;
			}
		} else
		{
			sError.SetSprintf ( "unknown option %s", sOpt.cstr() );
			break;
		}

		// post-conf type check
		if ( iExpType!=v.m_iType )
		{
			sError.SetSprintf ( "unexpected option %s type", sOpt.cstr() );
			break;
		}
	}

	if ( tOpts.m_bSkipBadJson && !tOpts.m_bJsonDocs ) // fixme! do we need such warn? Uncomment, if so.
		tRes.m_sMessages.Warn ( "option to skip bad json has no sense since docs are not in json form" );

	if ( !sError.IsEmpty() )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	BlobVec_t dBlobDocs;
	dBlobDocs.Reserve ( dDocs.GetLength() ); // actually some docs may be complex
	CSphVector<int> dBadDocs;

	if ( !tOpts.m_bJsonDocs )
		for ( auto &dDoc : dDocs )
			dDoc.LeakToVec ( dBlobDocs.Add () );
	else
		ARRAY_FOREACH ( i, dDocs )
		{
			using namespace bson;
			CSphVector<BYTE> dData;
			if ( !sphJsonParse ( dData, (char *)dDocs[i].cstr(), g_bJsonAutoconvNumbers, g_bJsonKeynamesToLowercase, false, sError ) )
			{
				dBadDocs.Add ( i + 1 );
				continue;
			}

			Bson_c dBson ( dData );
			if ( dBson.IsArray () )
			{
				for ( BsonIterator_c dItem ( dBson ); dItem; dItem.Next() )
				{
					if ( dItem.IsAssoc () )
						dItem.BsonToBson ( dBlobDocs.Add () );
					else
					{
						dBadDocs.Add ( i + 1 );	// fixme! m.b. report it as 'wrong doc N in string M'?
						break;
					}
				}
			}
			else if ( dBson.IsAssoc() )
			{
				dData.SwapData ( dBlobDocs.Add () );
			}
			else if ( bSkipEmpty && dBson.IsEmpty() )
				continue;
			else
				dBadDocs.Add ( i + 1 ); // let it be just 'an error' for now
			if ( !dBadDocs.IsEmpty() && !tOpts.m_bSkipBadJson )
				break;
		}

	if ( !dBadDocs.IsEmpty() )
	{
		StringBuilder_c sBad ( ",", "Bad JSON objects in strings: " );
		for ( int iBadDoc:dBadDocs )
			sBad.Sprintf ( "%d", iBadDoc );

		if ( !tOpts.m_bSkipBadJson )
		{
			tOut.Error ( tStmt.m_sStmt, sBad.cstr ());
			return;
		}
		tRes.m_sMessages.Warn ( sBad.cstr () );
	}

	tResult.m_dDocids.Reset ( tOpts.m_sIdAlias.IsEmpty () ? 0 : dBlobDocs.GetLength () + 1 );

	if ( tOpts.m_iShift && !tOpts.m_sIdAlias.IsEmpty () )
		tRes.m_sMessages.Warn ( "'shift' option works only for automatic ids, when 'docs_id' is not defined" );

	PercolateMatchDocuments ( dBlobDocs, tOpts, tAcc, tResult );

	if ( !tRes.m_sMessages.ErrEmpty () )
	{
		tRes.m_sMessages.MoveAllTo ( sError );
		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	SendMysqlPercolateReply ( tOut, tResult, tOpts.m_iShift );
}

void HandleMysqlPercolateMeta ( const CPqResult &tResult, const CSphString & sWarning, RowBuffer_i & tOut )
{
	// shortcuts
	const PercolateMatchResult_t &tMeta = tResult.m_dResult;

	tOut.HeadTuplet ( "Name", "Value" );
	tOut.DataTupletf ( "Total", "%.3D sec", tMeta.m_tmTotal / 1000 );
	if ( tMeta.m_tmSetup && tMeta.m_tmSetup>0 )
		tOut.DataTupletf ( "Setup", "%.3D sec", tMeta.m_tmSetup / 1000 );
	tOut.DataTuplet ( "Queries matched", tMeta.m_iQueriesMatched );
	tOut.DataTuplet ( "Queries failed", tMeta.m_iQueriesFailed );
	tOut.DataTuplet ( "Document matched", tMeta.m_iDocsMatched );
	tOut.DataTuplet ( "Total queries stored", tMeta.m_iTotalQueries );
	tOut.DataTuplet ( "Term only queries", tMeta.m_iOnlyTerms );
	tOut.DataTuplet ( "Fast rejected queries", tMeta.m_iEarlyOutQueries );

	if ( !tMeta.m_dQueryDT.IsEmpty() )
	{
		uint64_t tmMatched = 0;
		StringBuilder_c sList (", ");
		assert ( tMeta.m_iQueriesMatched==tMeta.m_dQueryDT.GetLength() );
		for ( int tmQuery : tMeta.m_dQueryDT )
		{
			sList.Sprintf ( "%d", tmQuery );
			tmMatched += tmQuery;
		}
		tOut.DataTuplet ( "Time per query", sList.cstr() );
		tOut.DataTuplet ( "Time of matched queries", tmMatched );
	}
	if ( !sWarning.IsEmpty() )
		tOut.DataTuplet ( "Warning", sWarning.cstr() );

	tOut.Eof();
}

static bool IsHttpStmt ( const SqlStmt_t & tStmt )
{
	return !tStmt.m_sEndpoint.IsEmpty();
}

void sphHandleMysqlInsert ( StmtErrorReporter_i & tOut, SqlStmt_t & tStmt, bool bReplace, bool bCommit,
	  CSphString & sWarning, CSphSessionAccum & tAcc, ESphCollation	eCollation, CSphVector<int64_t> & dLastIds )
{
	MEMORY ( MEM_SQL_INSERT );

	CSphString sError;
	ServedDescRPtr_c pServed ( GetServed ( tStmt.m_sIndex ) );
	if ( !pServed )
	{
		sError.SetSprintf ( "no such local index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !ServedDesc_t::IsMutable ( pServed ) )
	{
		sError.SetSprintf ( "index '%s' does not support INSERT", tStmt.m_sIndex.cstr ());
		tOut.Error ( tStmt.m_sStmt, sError.cstr ());
		return;
	}

	bool bPq = ( pServed->m_eType==IndexType_e::PERCOLATE );

	auto * pIndex = (RtIndex_i *)pServed->m_pIndex;

	// get schema, check values count
	const CSphSchema & tSchema = pIndex->GetMatchSchema ();
	const CSphSchema & tSchemaInt = pIndex->GetInternalSchema();
	int iSchemaSz = tSchema.GetAttrsCount() + tSchema.GetFieldsCount();
	if ( pIndex->GetSettings().m_bIndexFieldLens )
		iSchemaSz -= tSchema.GetFieldsCount();

	if ( tSchema.GetAttr ( sphGetBlobLocatorName() ) )
		iSchemaSz--;

	int iExp = tStmt.m_iSchemaSz;
	int iGot = tStmt.m_dInsertValues.GetLength();
	if ( !tStmt.m_dInsertSchema.GetLength()
		&& iSchemaSz!=tStmt.m_iSchemaSz )
	{
		sError.SetSprintf ( "column count does not match schema (expected %d, got %d)", iSchemaSz, iGot );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( ( iGot % iExp )!=0 )
	{
		sError.SetSprintf ( "column count does not match value count (expected %d, got %d)", iExp, iGot );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !CheckIndexCluster ( tStmt.m_sIndex, *pServed, tStmt.m_sCluster, IsHttpStmt ( tStmt ), sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !sError.IsEmpty() )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	CSphVector<int> dAttrSchema ( tSchema.GetAttrsCount() );
	CSphVector<int> dFieldSchema ( tSchema.GetFieldsCount() );
	CSphVector<bool> dFieldAttrs ( tSchema.GetFieldsCount() );
	ARRAY_FOREACH ( i, dFieldAttrs )
		dFieldAttrs[i] = false;

	int iIdIndex = 0;
	if ( !tStmt.m_dInsertSchema.GetLength() )
	{
		// no columns list, use index schema
		assert ( tSchema.GetAttr(0).m_sName==sphGetDocidName() );

		ARRAY_FOREACH ( i, dFieldSchema )
			dFieldSchema[i] = i+1;

		dAttrSchema[0]=0;
		int iAttrId = dFieldSchema.GetLength()+1;
		for ( int i = 1; i < dAttrSchema.GetLength(); i++ )
		{
			if ( sphIsInternalAttr( tSchema.GetAttr(i) ) )
				dAttrSchema[i]=-1;
			else
				dAttrSchema[i] = iAttrId++;
		}
	} else
	{
		// got a list of columns, check for 1) existance, 2) dupes
		StrVec_t dCheck = tStmt.m_dInsertSchema;
		ARRAY_FOREACH ( i, dCheck )
			// OPTIMIZE! GetAttrIndex and GetFieldIndex use the linear searching. M.b. hash instead?
			if ( tSchema.GetAttrIndex ( dCheck[i].cstr() )==-1 && tSchema.GetFieldIndex ( dCheck[i].cstr() )==-1 )
			{
				sError.SetSprintf ( "unknown column: '%s'", dCheck[i].cstr() );
				tOut.Error ( tStmt.m_sStmt, sError.cstr(), MYSQL_ERR_PARSE_ERROR );
				return;
			}

		dCheck.Sort();
		for ( int i=1; i<dCheck.GetLength(); i++ )
			if ( dCheck[i-1]==dCheck[i] )
			{
				sError.SetSprintf ( "column '%s' specified twice", dCheck[i].cstr() );
				tOut.Error ( tStmt.m_sStmt, sError.cstr(), MYSQL_ERR_FIELD_SPECIFIED_TWICE );
				return;
			}

		// hash column list
		// OPTIMIZE! hash index columns once (!) instead
		SmallStringHash_T<int> dInsertSchema;
		ARRAY_FOREACH ( i, tStmt.m_dInsertSchema )
			dInsertSchema.Add ( i, tStmt.m_dInsertSchema[i] );

		iIdIndex = -1;
		int * pId = dInsertSchema ( sphGetDocidStr() );
		if ( pId )
			iIdIndex = *pId;

		// map fields
		ARRAY_FOREACH ( i, dFieldSchema )
		{
			const char * szFieldName = tSchema.GetFieldName(i);
			if ( dInsertSchema.Exists(szFieldName) )
			{
				dFieldSchema[i] = dInsertSchema[szFieldName];

				// does an attribute with the same name exist?
				if ( tSchema.GetAttr(szFieldName) )
					dFieldAttrs[i] = true;
			} else
				dFieldSchema[i] = -1;
		}

		// map attrs
		ARRAY_FOREACH ( j, dAttrSchema )
		{
			const char * szAttrName = tSchema.GetAttr(j).m_sName.cstr();
			if ( dInsertSchema.Exists(szAttrName) )
				dAttrSchema[j] = dInsertSchema[szAttrName];
			else
				dAttrSchema[j] = -1;
		}
	}

	CSphVector<int64_t> dMvas;
	CSphVector<const char *> dStrings;
	StringPtrTraits_t tStrings;
	tStrings.m_dOff.Reset ( tSchema.GetAttrsCount() );
	RtAccum_t * pAccum = tAcc.GetAcc ( pIndex, sError );
	if ( !pAccum )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	const CSphColumnInfo * pDocid = tSchema.GetAttr(sphGetDocidName());
	assert ( pDocid );
	CSphAttrLocator tIdLoc = pDocid->m_tLocator;
	tIdLoc.m_bDynamic = true;
	CSphVector<int64_t> dIds;
	dIds.Reserve ( tStmt.m_iRowsAffected );

	// convert attrs
	for ( int c=0; c<tStmt.m_iRowsAffected; ++c )
	{
		assert ( sError.IsEmpty() );

		CSphMatchVariant tDoc;
		tDoc.Reset ( tSchema.GetRowSize() );
		if ( iIdIndex>=0 )
		{
			tDoc.SetAttr ( tIdLoc, tStmt.m_dInsertValues[iIdIndex + c * iExp], SPH_ATTR_BIGINT );
			if ( tDoc.GetAttr ( tIdLoc )<0 )
			{
				sError.SetSprintf ( "'id' column is " INT64_FMT ". Must be positive.", (int64_t)tDoc.GetAttr ( tIdLoc ) );
				break;
			}
		} else
		{
			assert ( tDoc.GetAttr(tIdLoc)==0 );
		}
		dStrings.Resize ( 0 );
		tStrings.Reset();
		dMvas.Resize ( 0 );

		int iSchemaAttrCount = tSchema.GetAttrsCount();
		if ( pIndex->GetSettings().m_bIndexFieldLens )
			iSchemaAttrCount -= tSchema.GetFieldsCount();
		for ( int i=0; i<iSchemaAttrCount; i++ )
		{
			// shortcuts!
			const CSphColumnInfo & tCol = tSchema.GetAttr(i);
			CSphAttrLocator tLoc = tCol.m_tLocator;
			tLoc.m_bDynamic = true;

			int iQuerySchemaIdx = dAttrSchema[i];
			bool bResult = false;
			if ( iQuerySchemaIdx < 0 )
			{
				bResult = tDoc.SetDefaultAttr ( tLoc, tCol.m_eAttrType );
				if ( tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_STRINGPTR || tCol.m_eAttrType==SPH_ATTR_JSON )
					dStrings.Add ( nullptr );
				if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET )
					dMvas.Add ( 0 );
			} else
			{
				const SqlInsert_t & tVal = tStmt.m_dInsertValues[iQuerySchemaIdx + c * iExp];

				// sanity checks
				if ( tVal.m_iType!=TOK_QUOTED_STRING
					&& tVal.m_iType!=TOK_CONST_INT
					&& tVal.m_iType!=TOK_CONST_FLOAT
					&& tVal.m_iType!=TOK_CONST_MVA )
				{
					sError.SetSprintf ( "row %d, column %d: internal error: unknown insval type %d", 1+c, 1+iQuerySchemaIdx, tVal.m_iType ); // 1 for human base
					break;
				}
				if ( tVal.m_iType==TOK_CONST_MVA
					&& !( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET ) )
				{
					sError.SetSprintf ( "row %d, column %d: MVA value specified for a non-MVA column", 1+c, 1+iQuerySchemaIdx ); // 1 for human base
					break;
				}
				if ( ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET )
				&& tVal.m_iType!=TOK_CONST_MVA )
				{
					sError.SetSprintf ( "row %d, column %d: non-MVA value specified for a MVA column", 1+c, 1+iQuerySchemaIdx ); // 1 for human base
					break;
				}

				// ok, checks passed; do work
				// MVA column? grab the values
				if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET )
				{
					// collect data from scattered insvals
					// FIXME! maybe remove this mess, and just have a single m_dMvas pool in parser instead?
					int iLen = 0;
					if ( tVal.m_pVals )
					{
						tVal.m_pVals->Uniq();
						iLen = tVal.m_pVals->GetLength();
					}

					dMvas.Add ( iLen );
					for ( int j=0; j<iLen; j++ )
						dMvas.Add ( (*tVal.m_pVals)[j] );
				}

				// FIXME? index schema is lawfully static, but our temp match obviously needs to be dynamic
				bResult = tDoc.SetAttr ( tLoc, tVal, tCol.m_eAttrType );
				if ( tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_STRINGPTR)
				{
					if ( tVal.m_sVal.Length() > 0x3FFFFF )
					{
						*( char * ) ( tVal.m_sVal.cstr () + 0x3FFFFF ) = '\0';
						sWarning.SetSprintf ( "String column %d at row %d too long, truncated to 4MB", i, c );
					}
					dStrings.Add ( tVal.m_sVal.cstr() );
				} else if ( tCol.m_eAttrType==SPH_ATTR_JSON )
				{
					int iStrCount = dStrings.GetLength();
					dStrings.Add ( nullptr );

					// empty source string means NULL attribute
					if ( !tVal.m_sVal.IsEmpty() )
					{
						// sphJsonParse must be terminated with a double zero however usual CSphString have SAFETY_GAP of 4 zeros
						if ( !String2JsonPack ( (char *)tVal.m_sVal.cstr(), tStrings.m_dParserBuf, sError, sWarning ) )
							break;

						int iParsedLength = tStrings.m_dParserBuf.GetLength();
						if ( iParsedLength )
						{
							tStrings.m_dOff[iStrCount] = tStrings.m_dPackedData.GetLength();
							BYTE * pPacked = tStrings.m_dPackedData.AddN ( sphCalcPackedLength ( iParsedLength ) );
							sphPackPtrAttr ( pPacked, tStrings.m_dParserBuf );
						}
					}
				}
			}

			if ( !bResult )
			{
				sError.SetSprintf ( "internal error: unknown attribute type in INSERT (typeid=%d)", tCol.m_eAttrType );
				break;
			}
		}
		if ( !sError.IsEmpty() )
			break;

		// remap JSON to string pointers
		tStrings.SavePointersTo ( dStrings );

		// convert fields
		CSphVector<VecTraits_T<const char>> dFields;

		// if strings and fields share one value, it might be modified by html stripper etc
		// we need to use separate storage for such string attributes and fields
		StrVec_t dTmpFieldStorage;
		dTmpFieldStorage.Resize(tSchema.GetFieldsCount());

		for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
		{
			int iQuerySchemaIdx = dFieldSchema[i];
			if ( iQuerySchemaIdx < 0 )
				dFields.Add (); // default value
			else
			{
				if ( tStmt.m_dInsertValues [ iQuerySchemaIdx + c * iExp ].m_iType!=TOK_QUOTED_STRING )
				{
					sError.SetSprintf ( "row %d, column %d: string expected", 1+c, 1+iQuerySchemaIdx ); // 1 for human base
					break;
				}

				const char * szFieldValue = tStmt.m_dInsertValues[ iQuerySchemaIdx + c * iExp ].m_sVal.cstr();
				if ( dFieldAttrs[i] )
				{
					dTmpFieldStorage[i] = szFieldValue;
					dFields.Add ( { dTmpFieldStorage[i].cstr(), dTmpFieldStorage[i].Length() } );
				} else
					dFields.Add ( { szFieldValue, ( int64_t) strlen(szFieldValue) } );
			}
		}
		if ( !sError.IsEmpty() )
			break;

		// do add
		if ( bPq )
		{
			if ( iIdIndex>=0 && tDoc.GetAttr ( tIdLoc )<0 )
			{
				sError.SetSprintf ( "'id' column is " INT64_FMT ". Must be positive.", (int64_t)tDoc.GetAttr ( tIdLoc ) );
				break;
			}

			CSphVector<CSphFilterSettings> dFilters;
			CSphVector<FilterTreeItem_t>   dFilterTree;
			if ( !PercolateParseFilters ( dStrings[2], eCollation, tSchemaInt, dFilters, dFilterTree, sError ) )
				break;

			PercolateQueryArgs_t tArgs ( dFilters, dFilterTree );
			tArgs.m_sQuery   = dStrings[0];
			tArgs.m_sTags	= dStrings[1];
			tArgs.m_iQUID	= tDoc.GetAttr(tIdLoc);
			tArgs.m_bReplace = bReplace;
			tArgs.m_bQL		 = true;

			// add query
			auto * pQIndex = (PercolateIndex_i *)pIndex;
			StoredQuery_i * pStored = pQIndex->CreateQuery ( tArgs, sError );
			if ( pStored )
			{
				auto * pCmd = pAccum->AddCommand ( ReplicationCommand_e::PQUERY_ADD, tStmt.m_sCluster, tStmt.m_sIndex );
				pCmd->m_pStored  = pStored;

				dIds.Add ( pStored->m_iQUID );
			}
		} else
		{
			pIndex->AddDocument ( dFields, tDoc, bReplace, tStmt.m_sStringParam, dStrings.Begin(), dMvas, sError, sWarning, pAccum );
			dIds.Add ( tDoc.GetAttr ( tIdLoc ) );

			pAccum->AddCommand ( ReplicationCommand_e::RT_TRX, tStmt.m_sCluster, tStmt.m_sIndex );
		}

		if ( !sError.IsEmpty() )
			break;
	}

	// fire exit
	if ( !sError.IsEmpty() )
	{
		pIndex->RollBack ( pAccum ); // clean up collected data
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	dLastIds.SwapData ( dIds );

	// no errors so far
	if ( bCommit )
		HandleCmdReplicate ( *pAccum, sError );

	int64_t iLastInsertId = 0;
	if ( dLastIds.GetLength() )
		iLastInsertId = dLastIds.Last();

	// my OK packet
	tOut.Ok ( tStmt.m_iRowsAffected, sWarning, iLastInsertId );
}


void HandleMysqlCallSnippets ( RowBuffer_i & tOut, SqlStmt_t & tStmt )
{
	CSphString sError;

	// check arguments
	// string data, string index, string query, [named opts]
	if ( tStmt.m_dInsertValues.GetLength()!=3 )
	{
		tOut.Error ( tStmt.m_sStmt, "SNIPPETS() expects exactly 3 arguments (data, index, query)" );
		return;
	}
	if ( tStmt.m_dInsertValues[0].m_iType!=TOK_QUOTED_STRING && tStmt.m_dInsertValues[0].m_iType!=TOK_CONST_STRINGS )
	{
		tOut.Error ( tStmt.m_sStmt, "SNIPPETS() argument 1 must be a string or a string list" );
		return;
	}
	if ( tStmt.m_dInsertValues[1].m_iType!=TOK_QUOTED_STRING )
	{
		tOut.Error ( tStmt.m_sStmt, "SNIPPETS() argument 2 must be a string" );
		return;
	}
	if ( tStmt.m_dInsertValues[2].m_iType!=TOK_QUOTED_STRING )
	{
		tOut.Error ( tStmt.m_sStmt, "SNIPPETS() argument 3 must be a string" );
		return;
	}

	// do magics
	CSphString sIndex = tStmt.m_dInsertValues[1].m_sVal;

	SnippetQuerySettings_t q;
	q.m_sQuery = tStmt.m_dInsertValues[2].m_sVal;

	ARRAY_FOREACH ( i, tStmt.m_dCallOptNames )
	{
		CSphString & sOpt = tStmt.m_dCallOptNames[i];
		const SqlInsert_t & v = tStmt.m_dCallOptValues[i];

		sOpt.ToLower();
		int iExpType = -1;

		if ( sOpt=="before_match" )				{ q.m_sBeforeMatch = v.m_sVal; iExpType = TOK_QUOTED_STRING; }
		else if ( sOpt=="after_match" )			{ q.m_sAfterMatch = v.m_sVal; iExpType = TOK_QUOTED_STRING; }
		else if ( sOpt=="chunk_separator" || sOpt=="snippet_separator" ) { q.m_sChunkSeparator = v.m_sVal; iExpType = TOK_QUOTED_STRING; }
		else if ( sOpt=="html_strip_mode" )		{ q.m_sStripMode = v.m_sVal; iExpType = TOK_QUOTED_STRING; }
		else if ( sOpt=="passage_boundary" || sOpt=="snippet_boundary" ) { q.m_ePassageSPZ = GetPassageBoundary(v.m_sVal); iExpType = TOK_QUOTED_STRING; }

		else if ( sOpt=="limit" )				{ q.m_iLimit = (int)v.m_iVal; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="limit_words" )			{ q.m_iLimitWords = (int)v.m_iVal; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="limit_passages" || sOpt=="limit_snippets" ) { q.m_iLimitPassages = (int)v.m_iVal; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="around" )				{ q.m_iAround = (int)v.m_iVal; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="start_passage_id" || sOpt=="start_snippet_id" ) { q.m_iPassageId = (int)v.m_iVal; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="exact_phrase" )
		{
			sError.SetSprintf ( "exact_phrase is deprecated" );
			break;
		}
		else if ( sOpt=="use_boundaries" )		{ q.m_bUseBoundaries = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="weight_order" )		{ q.m_bWeightOrder = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="query_mode" )
		{
			bool bQueryMode = ( v.m_iVal!=0 );
			iExpType = TOK_CONST_INT;
			if ( !bQueryMode )
			{
				sError.SetSprintf ( "query_mode=0 is deprecated" );
				break;
			}
		}
		else if ( sOpt=="force_all_words" )		{ q.m_bForceAllWords = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="load_files" )			{ q.m_uFilesMode = ( v.m_iVal!=0 )?1:0; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="load_files_scattered" ) { q.m_uFilesMode |= ( v.m_iVal!=0 )?2:0; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="allow_empty" )			{ q.m_bAllowEmpty = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="emit_zones" )			{ q.m_bEmitZones = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="force_passages" || sOpt=="force_snippets" ) { q.m_bForcePassages = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else
		{
			sError.SetSprintf ( "unknown option %s", sOpt.cstr() );
			break;
		}

		// post-conf type check
		if ( iExpType!=v.m_iType )
		{
			sError.SetSprintf ( "unexpected option %s type", sOpt.cstr() );
			break;
		}
	}
	if ( !sError.IsEmpty() )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !sphCheckOptionsSPZ ( q, q.m_ePassageSPZ, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	q.Setup();

	CSphVector<ExcerptQuery_t> dQueries;
	if ( tStmt.m_dInsertValues[0].m_iType==TOK_QUOTED_STRING )
	{
		auto& dQuery = dQueries.Add ();
		dQuery.m_sSource = tStmt.m_dInsertValues[0].m_sVal; // OPTIMIZE?
	} else
	{
		dQueries.Resize ( tStmt.m_dCallStrings.GetLength() );
		ARRAY_FOREACH ( i, tStmt.m_dCallStrings )
		{
			dQueries[i].m_sSource = tStmt.m_dCallStrings[i]; // OPTIMIZE?
		}
	}

	myinfo::SetThreadInfo ( R"(sphinxql-snippet datasize=%.1Dk query="%s")",
			GetSnippetDataSize ( dQueries ), q.m_sQuery.scstr ());

	if ( !MakeSnippets ( sIndex, dQueries, q, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !dQueries.FindFirst ( [] ( const ExcerptQuery_t & tQuery ) { return tQuery.m_sError.IsEmpty(); } ) )
	{
		// just one last error instead of all errors is hopefully ok
		sError.SetSprintf ( "highlighting failed: %s", sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// result set header packet
	tOut.HeadBegin(1);
	tOut.HeadColumn("snippet");
	tOut.HeadEnd();

	// data
	for ( auto & i : dQueries )
	{
		FixupResultTail ( i.m_dResult );
		tOut.PutArray ( i.m_dResult );
		tOut.Commit();
	}

	tOut.Eof();
}


class KeywordsRequestBuilder_c : public RequestBuilder_i
{
public:
	KeywordsRequestBuilder_c ( const GetKeywordsSettings_t & tSettings, const CSphString & sTerm );
	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final;

protected:
	const GetKeywordsSettings_t & m_tSettings;
	const CSphString & m_sTerm;
};


class KeywordsReplyParser_c : public ReplyParser_i
{
public:
	KeywordsReplyParser_c ( bool bGetStats, CSphVector<CSphKeywordInfo> & dKeywords );
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final;

	bool m_bStats;
	CSphVector<CSphKeywordInfo> & m_dKeywords;
};

static void MergeKeywords ( CSphVector<CSphKeywordInfo> & dKeywords );
static void SortKeywords ( const GetKeywordsSettings_t & tSettings, CSphVector<CSphKeywordInfo> & dKeywords );
bool DoGetKeywords ( const CSphString & sIndex, const CSphString & sQuery, const GetKeywordsSettings_t & tSettings, CSphVector <CSphKeywordInfo> & dKeywords, CSphString & sError, SearchFailuresLog_c & tFailureLog )
{
	auto pLocal = GetServed ( sIndex );
	auto pDistributed = GetDistr ( sIndex );

	if ( !pLocal && !pDistributed )
	{
		sError.SetSprintf ( "no such index %s", sIndex.cstr() );
		return false;
	}

	bool bOk = false;
	// just local plain or template index
	if ( pLocal )
	{
		ServedDescRPtr_c pIndex ( pLocal );
		if ( !pIndex->m_pIndex )
		{
			sError.SetSprintf ( "missed index '%s'", sIndex.cstr() );
		} else
		{
			bOk = pIndex->m_pIndex->GetKeywords ( dKeywords, sQuery.cstr(), tSettings, &sError );
		}
	} else
	{
		// FIXME!!! g_iDistThreads thread pool for locals.
		// locals
		const StrVec_t & dLocals = pDistributed->m_dLocal;
		CSphVector<CSphKeywordInfo> dKeywordsLocal;
		for ( const CSphString &sLocal : dLocals )
		{
			ServedDescRPtr_c pServed ( GetServed ( sLocal ) );
			if ( !pServed || !pServed->m_pIndex )
			{
				tFailureLog.Submit ( sLocal.cstr(), sIndex.cstr(), "missed index" );
				continue;
			}

			dKeywordsLocal.Resize(0);
			if ( pServed->m_pIndex->GetKeywords ( dKeywordsLocal, sQuery.cstr(), tSettings, &sError ) )
				dKeywords.Append ( dKeywordsLocal );
			else
				tFailureLog.SubmitEx ( sLocal.cstr (), sIndex.cstr (), "keyword extraction failed: %s", sError.cstr () );
		}

		// remote agents requests send off thread
		VecRefPtrsAgentConn_t dAgents;
		// fixme! We don't need all hosts here, only usual selected mirrors
		pDistributed->GetAllHosts ( dAgents );

		int iAgentsReply = 0;
		if ( !dAgents.IsEmpty() )
		{
			// connect to remote agents and query them
			KeywordsRequestBuilder_c tReqBuilder ( tSettings, sQuery );
			KeywordsReplyParser_c tParser ( tSettings.m_bStats, dKeywords );
			iAgentsReply = PerformRemoteTasks ( dAgents, &tReqBuilder, &tParser );

			for ( const AgentConn_t * pAgent : dAgents )
				if ( !pAgent->m_bSuccess && !pAgent->m_sFailure.IsEmpty() )
					tFailureLog.SubmitEx ( pAgent->m_tDesc.m_sIndexes.cstr(), sIndex.cstr(),
						"agent %s: %s", pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
		}

		// process result sets
		if ( dLocals.GetLength() + iAgentsReply>1 )
			MergeKeywords ( dKeywords );

		bOk = true;
	}

	SortKeywords ( tSettings, dKeywords );

	return bOk;
}

void HandleMysqlCallKeywords ( RowBuffer_i & tOut, SqlStmt_t & tStmt, CSphString & sWarning )
{
	CSphString sError;

	// string query, string index, [bool hits] || [value as option_name, ...]
	int iArgs = tStmt.m_dInsertValues.GetLength();
	if ( iArgs<2
		|| iArgs>3
		|| tStmt.m_dInsertValues[0].m_iType!=TOK_QUOTED_STRING
		|| tStmt.m_dInsertValues[1].m_iType!=TOK_QUOTED_STRING
		|| ( iArgs==3 && tStmt.m_dInsertValues[2].m_iType!=TOK_CONST_INT ) )
	{
		tOut.Error ( tStmt.m_sStmt, "bad argument count or types in KEYWORDS() call" );
		return;
	}

	GetKeywordsSettings_t tSettings;
	tSettings.m_bStats = ( iArgs==3 && tStmt.m_dInsertValues[2].m_iVal!=0 );
	ARRAY_FOREACH ( i, tStmt.m_dCallOptNames )
	{
		CSphString & sOpt = tStmt.m_dCallOptNames[i];
		sOpt.ToLower ();
		bool bEnabled = ( tStmt.m_dCallOptValues[i].m_iVal!=0 );
		bool bOptInt = true;

		if ( sOpt=="stats" )
			tSettings.m_bStats = bEnabled;
		else if ( sOpt=="fold_lemmas" )
			tSettings.m_bFoldLemmas = bEnabled;
		else if ( sOpt=="fold_blended" )
			tSettings.m_bFoldBlended = bEnabled;
		else if ( sOpt=="fold_wildcards" )
			tSettings.m_bFoldWildcards = bEnabled;
		else if ( sOpt=="expansion_limit" )
			tSettings.m_iExpansionLimit = int ( tStmt.m_dCallOptValues[i].m_iVal );
		else if ( sOpt=="sort_mode" )
		{
			// FIXME!!! add more sorting modes
			if ( tStmt.m_dCallOptValues[i].m_sVal!="docs" && tStmt.m_dCallOptValues[i].m_sVal!="hits" )
			{
				sError.SetSprintf ( "unknown option %s mode '%s'", sOpt.cstr(), tStmt.m_dCallOptValues[i].m_sVal.cstr() );
				tOut.Error ( tStmt.m_sStmt, sError.cstr() );
				return;
			}
			tSettings.m_bSortByDocs = ( tStmt.m_dCallOptValues[i].m_sVal=="docs" );
			tSettings.m_bSortByHits = ( tStmt.m_dCallOptValues[i].m_sVal=="hits" );
			bOptInt = false;
						
		} else
		{
			sError.SetSprintf ( "unknown option %s", sOpt.cstr () );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}

		// post-conf type check
		if ( bOptInt && tStmt.m_dCallOptValues[i].m_iType!=TOK_CONST_INT )
		{
			sError.SetSprintf ( "unexpected option %s type", sOpt.cstr () );
			tOut.Error ( tStmt.m_sStmt, sError.cstr () );
			return;
		}
	}
	const CSphString & sTerm = tStmt.m_dInsertValues[0].m_sVal;
	const CSphString & sIndex = tStmt.m_dInsertValues[1].m_sVal;
	CSphVector<CSphKeywordInfo> dKeywords;
	SearchFailuresLog_c tFailureLog;

	if ( !DoGetKeywords ( sIndex, sTerm, tSettings, dKeywords, sError, tFailureLog ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}


	// result set header packet
	tOut.HeadBegin ( tSettings.m_bStats ? 5 : 3 );
	tOut.HeadColumn("qpos");
	tOut.HeadColumn("tokenized");
	tOut.HeadColumn("normalized");
	if ( tSettings.m_bStats )
	{
		tOut.HeadColumn("docs");
		tOut.HeadColumn("hits");
	}
	tOut.HeadEnd();

	// data
	char sBuf[16];
	ARRAY_FOREACH ( i, dKeywords )
	{
		snprintf ( sBuf, sizeof(sBuf), "%d", dKeywords[i].m_iQpos );
		tOut.PutString ( sBuf );
		tOut.PutString ( dKeywords[i].m_sTokenized );
		tOut.PutString ( dKeywords[i].m_sNormalized );
		if ( tSettings.m_bStats )
		{
			snprintf ( sBuf, sizeof(sBuf), "%d", dKeywords[i].m_iDocs );
			tOut.PutString ( sBuf );
			snprintf ( sBuf, sizeof(sBuf), "%d", dKeywords[i].m_iHits );
			tOut.PutString ( sBuf );
		}
		tOut.Commit();
	}

	// put network errors and warnings to meta as warning
	int iWarnings = 0;
	if ( !tFailureLog.IsEmpty() )
	{
		iWarnings = tFailureLog.GetReportsCount();

		StringBuilder_c sErrorBuf;
		tFailureLog.BuildReport ( sErrorBuf );
		sErrorBuf.MoveTo ( sWarning );
		sphWarning ( "%s", sWarning.cstr() );
	}

	tOut.Eof ( false, iWarnings );
}

KeywordsRequestBuilder_c::KeywordsRequestBuilder_c ( const GetKeywordsSettings_t & tSettings, const CSphString & sTerm )
	: m_tSettings ( tSettings )
	, m_sTerm ( sTerm )
{
}

void KeywordsRequestBuilder_c::BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const
{
	const CSphString & sIndexes = tAgent.m_tDesc.m_sIndexes;

	auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_KEYWORDS, VER_COMMAND_KEYWORDS );

	tOut.SendString ( m_sTerm.cstr() );
	tOut.SendString ( sIndexes.cstr() );
	tOut.SendInt ( m_tSettings.m_bStats );
	tOut.SendInt ( m_tSettings.m_bFoldLemmas );
	tOut.SendInt ( m_tSettings.m_bFoldBlended );
	tOut.SendInt ( m_tSettings.m_bFoldWildcards );
	tOut.SendInt ( m_tSettings.m_iExpansionLimit );
}

KeywordsReplyParser_c::KeywordsReplyParser_c ( bool bGetStats, CSphVector<CSphKeywordInfo> & dKeywords )
	: m_bStats ( bGetStats )
	, m_dKeywords ( dKeywords )
{
}

bool KeywordsReplyParser_c::ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const
{
	int iWords = tReq.GetInt();
	int iLen = m_dKeywords.GetLength();
	m_dKeywords.Resize ( iWords + iLen );
	for ( int i=0; i<iWords; i++ )
	{
		CSphKeywordInfo & tWord = m_dKeywords[i + iLen];
		tWord.m_sTokenized = tReq.GetString();
		tWord.m_sNormalized = tReq.GetString();
		tWord.m_iQpos = tReq.GetInt();
		if ( m_bStats )
		{
			tWord.m_iDocs = tReq.GetInt();
			tWord.m_iHits = tReq.GetInt();
		}
	}

	return true;
}

struct KeywordSorter_fn
{
	bool IsLess ( const CSphKeywordInfo & a, const CSphKeywordInfo & b ) const
	{
		return ( ( a.m_iQpos<b.m_iQpos )
			|| ( a.m_iQpos==b.m_iQpos && a.m_iHits>b.m_iHits )
			|| ( a.m_iQpos==b.m_iQpos && a.m_iHits==b.m_iHits && a.m_sNormalized<b.m_sNormalized ) );
	}
};

void MergeKeywords ( CSphVector<CSphKeywordInfo> & dSrc )
{
	CSphOrderedHash < CSphKeywordInfo, uint64_t, IdentityHash_fn, 256 > hWords;
	ARRAY_FOREACH ( i, dSrc )
	{
		const CSphKeywordInfo & tInfo = dSrc[i];
		uint64_t uKey = sphFNV64 ( &tInfo.m_iQpos, sizeof(tInfo.m_iQpos) );
		uKey = sphFNV64 ( tInfo.m_sNormalized.cstr(), tInfo.m_sNormalized.Length(), uKey );

		CSphKeywordInfo & tVal = hWords.AddUnique ( uKey );
		if ( !tVal.m_iQpos )
		{
			tVal = tInfo;
		} else
		{
			tVal.m_iDocs += tInfo.m_iDocs;
			tVal.m_iHits += tInfo.m_iHits;
		}
	}

	dSrc.Resize ( 0 );
	hWords.IterateStart();
	while ( hWords.IterateNext() )
	{
		dSrc.Add ( hWords.IterateGet() );
	}

	sphSort ( dSrc.Begin(), dSrc.GetLength(), KeywordSorter_fn() );
}

struct KeywordSorterDocs_fn
{
	bool IsLess ( const CSphKeywordInfo & a, const CSphKeywordInfo & b ) const
	{
		return ( ( a.m_iQpos<b.m_iQpos )
			|| ( a.m_iQpos==b.m_iQpos && a.m_iDocs>b.m_iDocs )
			|| ( a.m_iQpos==b.m_iQpos && a.m_iDocs==b.m_iDocs && a.m_sNormalized<b.m_sNormalized ) );
	}
};


void SortKeywords ( const GetKeywordsSettings_t & tSettings, CSphVector<CSphKeywordInfo> & dKeywords )
{
	if ( !tSettings.m_bSortByDocs && !tSettings.m_bSortByHits )
		return;

	if ( tSettings.m_bSortByHits )
		dKeywords.Sort ( KeywordSorter_fn() );
	else
		dKeywords.Sort ( KeywordSorterDocs_fn() );
}

// sort by distance asc, document count desc, ABC asc
struct CmpDistDocABC_fn
{
	const char * m_pBuf;
	explicit CmpDistDocABC_fn ( const char * pBuf ) : m_pBuf ( pBuf ) {}

	inline bool IsLess ( const SuggestWord_t & a, const SuggestWord_t & b ) const
	{
		if ( a.m_iDistance==b.m_iDistance && a.m_iDocs==b.m_iDocs )
		{
			return ( sphDictCmpStrictly ( m_pBuf + a.m_iNameOff, a.m_iLen, m_pBuf + b.m_iNameOff, b.m_iLen )<0 );
		}

		if ( a.m_iDistance==b.m_iDistance )
			return a.m_iDocs>=b.m_iDocs;
		return a.m_iDistance<b.m_iDistance;
	}
};

void HandleMysqlCallSuggest ( RowBuffer_i & tOut, SqlStmt_t & tStmt, bool bQueryMode )
{
	CSphString sError;

	// string query, string index, [value as option_name, ...]
	int iArgs = tStmt.m_dInsertValues.GetLength ();
	if ( iArgs<2
			|| iArgs>3
			|| tStmt.m_dInsertValues[0].m_iType!=TOK_QUOTED_STRING
			|| tStmt.m_dInsertValues[1].m_iType!=TOK_QUOTED_STRING
			|| ( iArgs==3 && tStmt.m_dInsertValues[2].m_iType!=TOK_CONST_INT ) )
	{
		tOut.Error ( tStmt.m_sStmt, "bad argument count or types in KEYWORDS() call" );
		return;
	}

	SuggestArgs_t tArgs;
	SuggestResult_t tRes;
	const char * sWord = tStmt.m_dInsertValues[0].m_sVal.cstr();
	tArgs.m_bQueryMode = bQueryMode;

	ARRAY_FOREACH ( i, tStmt.m_dCallOptNames )
	{
		CSphString & sOpt = tStmt.m_dCallOptNames[i];
		sOpt.ToLower ();
		int iTokType = TOK_CONST_INT;

		if ( sOpt=="limit" )
		{
			tArgs.m_iLimit = int ( tStmt.m_dCallOptValues[i].m_iVal );
		} else if ( sOpt=="delta_len" )
		{
			tArgs.m_iDeltaLen = int ( tStmt.m_dCallOptValues[i].m_iVal );
		} else if ( sOpt=="max_matches" )
		{
			tArgs.m_iQueueLen = int ( tStmt.m_dCallOptValues[i].m_iVal );
		} else if ( sOpt=="reject" )
		{
			tArgs.m_iRejectThr = int ( tStmt.m_dCallOptValues[i].m_iVal );
		} else if ( sOpt=="max_edits" )
		{
			tArgs.m_iMaxEdits = int ( tStmt.m_dCallOptValues[i].m_iVal );
		} else if ( sOpt=="result_line" )
		{
			tArgs.m_bResultOneline = ( tStmt.m_dCallOptValues[i].m_iVal!=0 );
		} else if ( sOpt=="result_stats" )
		{
			tArgs.m_bResultStats = ( tStmt.m_dCallOptValues[i].m_iVal!=0 );
		} else if ( sOpt=="non_char" )
		{
			tArgs.m_bNonCharAllowed = ( tStmt.m_dCallOptValues[i].m_iVal!=0 );
		} else
		{
			sError.SetSprintf ( "unknown option %s", sOpt.cstr () );
			tOut.Error ( tStmt.m_sStmt, sError.cstr () );
			return;
		}

		// post-conf type check
		if ( tStmt.m_dCallOptValues[i].m_iType!=iTokType )
		{
			sError.SetSprintf ( "unexpected option %s type", sOpt.cstr () );
			tOut.Error ( tStmt.m_sStmt, sError.cstr () );
			return;
		}
	}



	{ // scope for ServedINdexPtr_c
		ServedDescRPtr_c pServed ( GetServed ( tStmt.m_dInsertValues[1].m_sVal ) );
		if ( !pServed || !pServed->m_pIndex )
		{
			sError.SetSprintf ( "no such index %s", tStmt.m_dInsertValues[1].m_sVal.cstr () );
			tOut.Error ( tStmt.m_sStmt, sError.cstr () );
			return;
		}
		if ( !pServed->m_pIndex->GetSettings().m_iMinInfixLen || !pServed->m_pIndex->GetDictionary()->GetSettings().m_bWordDict )
		{
			sError.SetSprintf ( "suggests work only for keywords dictionary with infix enabled" );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}

		if ( tRes.SetWord ( sWord, pServed->m_pIndex->GetQueryTokenizer(), tArgs.m_bQueryMode ) )
		{
			pServed->m_pIndex->GetSuggest ( tArgs, tRes );
		}
	}

	// data
	if ( tArgs.m_bResultOneline )
	{
		// let's resort by alphabet to better compare result sets
		CmpDistDocABC_fn tCmp ( (const char *)( tRes.m_dBuf.Begin() ) );
		tRes.m_dMatched.Sort ( tCmp );

		// result set header packet
		tOut.HeadBegin ( 2 );
		tOut.HeadColumn ( "name" );
		tOut.HeadColumn ( "value" );
		tOut.HeadEnd ();

		StringBuilder_c sBuf ( "," );
		for ( auto& dMatched : tRes.m_dMatched )
			sBuf << (const char*) tRes.m_dBuf.Begin() + dMatched.m_iNameOff;
		tOut.PutString ( "suggests" );
		tOut.PutString ( sBuf.cstr() );
		tOut.Commit();

		if ( tArgs.m_bResultStats )
		{
			sBuf.Clear ();
			sBuf.StartBlock ( "," );
			for ( auto &dMatched : tRes.m_dMatched )
				sBuf.Appendf ( "%d", dMatched.m_iDistance );
			tOut.PutString ( "distance" );
			tOut.PutString ( sBuf.cstr () );
			tOut.Commit ();

			sBuf.Clear ();
			sBuf.StartBlock ( "," );
			for ( auto &dMatched : tRes.m_dMatched )
				sBuf.Appendf ( "%d", dMatched.m_iDocs );
			tOut.PutString ( "docs" );
			tOut.PutString ( sBuf.cstr () );
			tOut.Commit ();
		}
	} else
	{
		// result set header packet
		tOut.HeadBegin ( tArgs.m_bResultStats ? 3 : 1 );
		tOut.HeadColumn ( "suggest" );
		if ( tArgs.m_bResultStats )
		{
			tOut.HeadColumn ( "distance" );
			tOut.HeadColumn ( "docs" );
		}
		tOut.HeadEnd ();

		auto * szResult = (const char *)( tRes.m_dBuf.Begin() );
		ARRAY_FOREACH ( i, tRes.m_dMatched )
		{
			const SuggestWord_t & tWord = tRes.m_dMatched[i];
			tOut.PutString ( szResult + tWord.m_iNameOff );
			if ( tArgs.m_bResultStats )
			{
				tOut.PutNumAsString ( tWord.m_iDistance );
				tOut.PutNumAsString ( tWord.m_iDocs );
			}
			tOut.Commit();
		}
	}

	tOut.Eof();
}


void HandleMysqlDescribe ( RowBuffer_i & tOut, SqlStmt_t & tStmt )
{
	TableLike dCondOut ( tOut, tStmt.m_sStringParam.cstr () );

	ServedDescRPtr_c pServed ( GetServed ( tStmt.m_sIndex ) );
	if ( pServed && pServed->m_pIndex )
	{
		const char * dNames[] = { "Field", "Type", "Properties" };
		// result set header packet
		tOut.HeadOfStrings ( dNames, sizeof(dNames)/sizeof(dNames[0]) );

		// data
		const CSphSchema *pSchema = &pServed->m_pIndex->GetMatchSchema ();
		if ( tStmt.m_iIntParam==TOK_TABLE ) // user wants internal schema instead
		{
			if ( ServedDesc_t::IsMutable ( pServed ) )
			{
				auto pRtIndex = (RtIndex_i*)pServed->m_pIndex;
				pSchema = &pRtIndex->GetInternalSchema ();
			}
		}

		const CSphSchema &tSchema = *pSchema;
		assert ( pServed->m_eType==IndexType_e::TEMPLATE || tSchema.GetAttr(0).m_sName==sphGetDocidName() );

		// id comes before fields
		if ( pServed->m_eType!=IndexType_e::TEMPLATE )
			dCondOut.MatchData3 ( "id", "bigint", "" );

		for ( int i = 0; i<tSchema.GetFieldsCount (); i++ )
		{
			StringBuilder_c sProperties(" ");
			DWORD uFlags = tSchema.GetField(i).m_uFieldFlags;
			if ( uFlags & CSphColumnInfo::FIELD_INDEXED )
				sProperties << "indexed";

			if ( uFlags & CSphColumnInfo::FIELD_STORED )
				sProperties << "stored";

			dCondOut.MatchData3 ( tSchema.GetFieldName(i), "text", sProperties.cstr() );
		}

		char sTmp[SPH_MAX_WORD_LEN];
		for ( int i=1; i<tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tCol = tSchema.GetAttr(i);
			if ( sphIsInternalAttr ( tCol ) )
				continue;

			if ( tCol.m_eAttrType==SPH_ATTR_INTEGER && tCol.m_tLocator.m_iBitCount!=ROWITEM_BITS )
			{
				snprintf ( sTmp, sizeof(sTmp), "%s:%d", sphTypeName ( tCol.m_eAttrType ), tCol.m_tLocator.m_iBitCount );
				dCondOut.MatchData3 ( tCol.m_sName.cstr(), sTmp, "" );
			} else
				dCondOut.MatchData3 ( tCol.m_sName.cstr(), sphTypeName ( tCol.m_eAttrType ), "" );
		}

		tOut.Eof();
		return;
	}

	auto pDistr = GetDistr ( tStmt.m_sIndex );

	if ( !pDistr )
	{
		CSphString sError;
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr(), MYSQL_ERR_NO_SUCH_TABLE );
		return;
	}

	tOut.HeadTuplet ( "Agent", "Type" );
	for ( const auto& sIdx : pDistr->m_dLocal )
		dCondOut.MatchData2 ( sIdx.cstr(), "local" );

	ARRAY_FOREACH ( i, pDistr->m_dAgents )
	{
		const MultiAgentDesc_c & tMultiAgent = *pDistr->m_dAgents[i];
		if ( tMultiAgent.IsHA() )
		{
			ARRAY_FOREACH ( j, tMultiAgent )
			{
				const AgentDesc_t &tDesc = tMultiAgent[j];
				StringBuilder_c sKey;
				sKey += tDesc.m_bBlackhole ? "blackhole_" : "remote_";
				sKey.Appendf ( "%d_mirror_%d", i+1, j+1 );
				CSphString sValue;
				sValue.SetSprintf ( "%s:%s", tDesc.GetMyUrl().cstr(), tDesc.m_sIndexes.cstr() );
				dCondOut.MatchData2 ( sValue.cstr(), sKey.cstr() );
			}
		} else
		{
			const AgentDesc_t &tDesc = tMultiAgent[0];
			StringBuilder_c sKey;
			sKey+= tDesc.m_bBlackhole?"blackhole_":"remote_";
			sKey.Appendf ( "%d", i+1 );
			CSphString sValue;
			sValue.SetSprintf ( "%s:%s", tDesc.GetMyUrl().cstr(), tDesc.m_sIndexes.cstr() );
			dCondOut.MatchData2 ( sValue.cstr(), sKey.cstr() );
		}
	}
	tOut.Eof();
}


struct IndexNameLess_fn
{
	inline bool IsLess ( const CSphNamedInt & a, const CSphNamedInt & b ) const
	{
		return strcasecmp ( a.first.cstr(), b.first.cstr() )<0;
	}
};


void HandleMysqlShowTables ( RowBuffer_i & tOut, SqlStmt_t & tStmt )
{
	// 0 local, 1 distributed, 2 rt, 3 template, 4 percolate, 5 unknown
	static const char* sTypes[] = {"local", "distributed", "rt", "template", "percolate", "unknown"};
	CSphVector<CSphNamedInt> dIndexes;

	// collect local, rt, percolate
	for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next(); )
		if ( it.Get() )
		{
			ServedDescRPtr_c pIdx ( it.Get () );
			switch ( pIdx->m_eType )
			{
				case IndexType_e::PLAIN:
					dIndexes.Add ( CSphNamedInt ( it.GetName (), 0 ) );
					break;
				case IndexType_e::RT:
					dIndexes.Add ( CSphNamedInt ( it.GetName (), 2 ) );
					break;
				case IndexType_e::PERCOLATE:
					dIndexes.Add ( CSphNamedInt ( it.GetName (), 4 ) );
					break;
				case IndexType_e::TEMPLATE:
					dIndexes.Add ( CSphNamedInt ( it.GetName (), 3 ) );
					break;
				default:
					dIndexes.Add ( CSphNamedInt ( it.GetName (), 5 ) );
			}
		}

	// collect distributed
	for ( RLockedDistrIt_c it ( g_pDistIndexes ); it.Next (); )
		// no need to check distr's it, iterating guarantees index existance.
		dIndexes.Add ( CSphNamedInt ( it.GetName (), 1 ) );

	dIndexes.Sort ( IndexNameLess_fn() );

	// output the results
	tOut.HeadTuplet ( "Index", "Type" );
	TableLike dCondOut ( tOut, tStmt.m_sStringParam.cstr() );
	for ( auto& dPair : dIndexes )
		dCondOut.MatchData2 ( dPair.first.cstr (), sTypes[dPair.second] );
	tOut.Eof();
}


static bool CheckAttrs ( const VecTraits_T<CSphColumnInfo> & dAttrs, CSphString & sError )
{
	ARRAY_FOREACH ( i, dAttrs )
	{
		const CSphString & sName = dAttrs[i].m_sName;
		if ( CSphSchema::IsReserved ( sName.cstr() ) || sphIsInternalAttr ( sName ) )
		{
			sError.SetSprintf ( "attribute name '%s' is a reserved keyword", sName.cstr() );
			return false;
		}

		for ( int j = i+1; j < dAttrs.GetLength(); j++ )
			if ( dAttrs[j].m_sName==sName )
			{
				sError.SetSprintf ( "duplicate attribute name '%s'", sName.cstr() );
				return false;
			}
	}

	return true;
}


static bool CheckExistingTables ( const SqlStmt_t & tStmt, CSphString & sError )
{
	if ( g_pLocalIndexes->Contains ( tStmt.m_sIndex ) || g_pDistIndexes->Contains ( tStmt.m_sIndex ) )
	{
		if ( tStmt.m_tCreateTable.m_bIfNotExists )
			return true;
		else
		{
			sError.SetSprintf ( "index '%s' already exists", tStmt.m_sIndex.cstr() );
			return false;
		}
	}

	if ( CSphSchema::IsReserved ( tStmt.m_sIndex.cstr() ) )
	{
		sError.SetSprintf ( "'%s' is a reserved keyword", tStmt.m_sIndex.cstr() );
		return false;
	}

	return true;
}


static bool CheckCreateTable ( const SqlStmt_t & tStmt, CSphString & sError )
{
	if ( !CheckExistingTables ( tStmt, sError ) )
		return false;

	if ( !CheckAttrs ( tStmt.m_tCreateTable.m_dAttrs, sError ) || !CheckAttrs ( tStmt.m_tCreateTable.m_dFields, sError ) )
		return false;

	// cross-checks attrs and fields
	for ( const auto & i : tStmt.m_tCreateTable.m_dAttrs )
		for ( const auto & j : tStmt.m_tCreateTable.m_dFields )
			if ( i.m_sName==j.m_sName && i.m_eAttrType!=SPH_ATTR_STRING )
			{
				sError.SetSprintf ( "duplicate attribute name '%s'", i.m_sName.cstr() );
				return false;
			}

	return true;
}


static CSphString ConcatWarnings ( const StrVec_t & dWarnings )
{
	StringBuilder_c sRes ( "; " );
	for ( const auto & i : dWarnings )
		sRes << i;

	return sRes.cstr();
}


static void HandleMysqlCreateTable ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	SearchFailuresLog_c dErrors;
	CSphString sError;

	if ( !IsConfigless() )
	{
		sError = "CREATE TABLE requires data_dir to be set in the config file";
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !CheckCreateTable ( tStmt, sError ) )
	{
		sError.SetSprintf ( "index '%s': CREATE TABLE failed: %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	StrVec_t dWarnings;
	bool bCreatedOk = CreateNewIndexInt ( tStmt.m_sIndex, tStmt.m_tCreateTable, dWarnings, sError );
	sWarning = ConcatWarnings(dWarnings);

	if ( !bCreatedOk )
	{
		sError.SetSprintf ( "error adding index '%s': %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	tOut.Ok ( 0, dWarnings.GetLength() );
}


static const CSphSchema & GetSchemaForCreateTable ( CSphIndex * pIndex )
{
	assert ( pIndex );
	assert ( pIndex->IsRT() || pIndex->IsPQ() );

	if ( pIndex->IsRT() )
		return ((RtIndex_i*)pIndex)->GetInternalSchema();

	// for pq
	return pIndex->GetMatchSchema();
}


static CSphString BuildCreateTableRt ( const CSphString & sName, const CSphIndex * pIndex, const CSphSchema & tSchema )
{
	assert(pIndex);

	CSphString sCreateTable = BuildCreateTable ( sName, pIndex, tSchema );

	// FIXME: create a separate struct (and move it to indexsettings.cpp) when there are more RT-specific settings
	if ( pIndex->IsRT() )
	{
		auto * pRt = (RtIndex_i*)pIndex;
		int64_t iMemLimit = pRt->GetMemLimit();
		if ( iMemLimit!=DEFAULT_RT_MEM_LIMIT )
			sCreateTable.SetSprintf ( "%s rt_mem_limit='" INT64_FMT "'", sCreateTable.cstr(), iMemLimit );
	}

	return sCreateTable;
}


static void HandleMysqlCreateTableLike ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	SearchFailuresLog_c dErrors;
	CSphString sError;

	if ( !IsConfigless() )
	{
		sError = "CREATE TABLE requires data_dir to be set in the config file";
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !CheckExistingTables ( tStmt, sError ) )
	{
		sError.SetSprintf ( "index '%s': CREATE TABLE failed: %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	const CSphString & sLike = tStmt.m_tCreateTable.m_sLike;
	ServedDescRPtr_c pServed(GetServed(sLike));
	auto pDist = GetDistr(sLike);
	if ( !pServed && !pDist )
	{
		sError.SetSprintf ( "index '%s': CREATE TABLE LIKE failed: no index '%s' found", tStmt.m_sIndex.cstr(), sLike.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( pServed && !pServed->m_pIndex->IsRT() && !pServed->m_pIndex->IsPQ() )
	{
		CSphString sError;
		sError.SetSprintf ( "index '%s' is not real-time or percolate", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr(), MYSQL_ERR_NO_SUCH_TABLE );
		return;
	}

	CSphString sCreateTable;
	if ( pServed )
		sCreateTable = BuildCreateTableRt ( tStmt.m_sIndex, pServed->m_pIndex, GetSchemaForCreateTable ( pServed->m_pIndex ) );
	else
		sCreateTable = BuildCreateTableDistr ( tStmt.m_sIndex, *pDist );

	CSphVector<SqlStmt_t> dCreateTableStmts;
	if ( !ParseDdl ( sCreateTable.cstr(), sCreateTable.Length(), dCreateTableStmts, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( dCreateTableStmts.GetLength()!=1 )
	{
		tOut.Error ( tStmt.m_sStmt, "CREATE TABLE LIKE failed" );
		return;
	}

	SqlStmt_t & tNewCreateTable = dCreateTableStmts[0];
	tNewCreateTable.m_tCreateTable.m_bIfNotExists = tStmt.m_tCreateTable.m_bIfNotExists;

	HandleMysqlCreateTable ( tOut, tNewCreateTable, sWarning );
}


static void HandleMysqlDropTable ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;

	if ( !IsConfigless() )
	{
		sError = "DROP TABLE requires data_dir to be set in the config file";
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !DropIndexInt ( tStmt.m_sIndex.cstr(), tStmt.m_bIfExists, sError ) )
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
	else
		tOut.Ok();
}


void HandleMysqlShowCreateTable ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	ServedDescRPtr_c pServed ( GetServed ( tStmt.m_sIndex ) );
	auto pDist = GetDistr ( tStmt.m_sIndex );
	if ( ( !pServed || !pServed->m_pIndex ) && !pDist )
	{
		CSphString sError;
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr(), MYSQL_ERR_NO_SUCH_TABLE );
		return;
	}

	if ( pServed && !pServed->m_pIndex->IsRT() && !pServed->m_pIndex->IsPQ() )
	{
		CSphString sError;
		sError.SetSprintf ( "index '%s' is not real-time or percolate", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr(), MYSQL_ERR_NO_SUCH_TABLE );
		return;
	}

	// result set header packet
	tOut.HeadTuplet ( "Table", "Create Table" );
	CSphString sCreateTable;
	if ( pServed )
		sCreateTable = BuildCreateTableRt ( pServed->m_pIndex->GetName(), pServed->m_pIndex, GetSchemaForCreateTable ( pServed->m_pIndex ) );
	else
		sCreateTable = BuildCreateTableDistr ( tStmt.m_sIndex, *pDist );

	tOut.DataTuplet ( tStmt.m_sIndex.cstr(), sCreateTable.cstr() );
	tOut.Eof();
}


// MySQL Workbench (and maybe other clients) crashes without it
void HandleMysqlShowDatabases ( RowBuffer_i & tOut, SqlStmt_t & )
{
	tOut.HeadBegin ( 1 );
	tOut.HeadColumn ( "Databases" );
	tOut.HeadEnd();
	tOut.PutString ( g_sDbName );
	tOut.Commit ();
	tOut.Eof();
}


void HandleMysqlShowPlugins ( RowBuffer_i & tOut, SqlStmt_t & )
{
	CSphVector<PluginInfo_t> dPlugins;
	sphPluginList ( dPlugins );

	tOut.HeadBegin ( 5 );
	tOut.HeadColumn ( "Type" );
	tOut.HeadColumn ( "Name" );
	tOut.HeadColumn ( "Library" );
	tOut.HeadColumn ( "Users" );
	tOut.HeadColumn ( "Extra" );
	tOut.HeadEnd();

	ARRAY_FOREACH ( i, dPlugins )
	{
		const PluginInfo_t & p = dPlugins[i];
		tOut.PutString ( g_dPluginTypes[p.m_eType] );
		tOut.PutString ( p.m_sName );
		tOut.PutString ( p.m_sLib );
		tOut.PutNumAsString ( p.m_iUsers );
		tOut.PutString ( p.m_sExtra );
		tOut.Commit();
	}
	tOut.Eof();
}

enum ThreadInfoFormat_e
{
	THD_FORMAT_NATIVE,
	THD_FORMAT_SPHINXQL
};

static std::pair<const char *, int> FormatInfo ( const PublicThreadDesc_t & tThd, ThreadInfoFormat_e eFmt, QuotationEscapedBuilder & tBuf )
{
	if ( tThd.m_pQuery && eFmt==THD_FORMAT_SPHINXQL && tThd.m_eProto!=Proto_e::MYSQL41 )
	{
		bool bGotQuery = false;
		if ( tThd.m_pQuery )
		{
			tBuf.Clear();
			FormatSphinxql ( *tThd.m_pQuery.Ptr(), 0, tBuf );
			bGotQuery = true;
		}

		// query might be removed prior to lock then go to common path
		if ( bGotQuery )
			return { tBuf.cstr (), tBuf.GetLength () };
	}

	if ( tThd.m_sDescription.IsEmpty () && tThd.m_sCommand )
		return { tThd.m_sCommand, (int)strlen ( tThd.m_sCommand ) };
	else
		return { tThd.m_sDescription.cstr (), tThd.m_sDescription.GetLength () };
}

void HandleMysqlShowThreads ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	int64_t tmNow = sphMicroTimer();

	tOut.HeadBegin ( 14 ); // 15 with chain
	tOut.HeadColumn ( "Tid" );
	tOut.HeadColumn ( "Name" );
	tOut.HeadColumn ( "Proto" );
	tOut.HeadColumn ( "State" );
	tOut.HeadColumn ( "Host" );
	tOut.HeadColumn ( "ConnID" );
	tOut.HeadColumn ( "Time" );
	tOut.HeadColumn ( "Work time" );
	tOut.HeadColumn ( "Work time CPU" );
	tOut.HeadColumn ( "Thd efficiency");
	tOut.HeadColumn ( "Jobs done" );
	tOut.HeadColumn ( "Last job took" );
	tOut.HeadColumn ( "In idle" );
//	tOut.HeadColumn ( "Chain" );
	tOut.HeadColumn ( "Info" );
	tOut.HeadEnd();

	QuotationEscapedBuilder tBuf;
	ThreadInfoFormat_e eFmt = THD_FORMAT_NATIVE;
	bool bAll = false;
	if ( tStmt.m_sThreadFormat=="sphinxql" )
		eFmt = THD_FORMAT_SPHINXQL;
	else if ( tStmt.m_sThreadFormat=="all" )
		bAll = true;

//	sphLogDebug ( "^^ Show threads. Current info is %p", GetTaskInfo () );

	CSphSwapVector<PublicThreadDesc_t> dFinal;
	Threads::IterateActive([&dFinal] ( Threads::LowThreadDesc_t * pThread ){
		if ( pThread )
			dFinal.Add ( GatherPublicTaskInfo ( pThread ) );
	});

	for ( const auto & dThd : dFinal )
	{
		if ( !bAll && dThd.m_eThdState==ThdState_e::UNKNOWN )
			continue;
		tOut.PutNumAsString ( dThd.m_iThreadID );
		tOut.PutString ( dThd.m_sThreadName );
		tOut.PutString ( dThd.m_sProto.cstr() );
		tOut.PutString ( ThdStateName ( dThd.m_eThdState ) );
		tOut.PutString ( dThd.m_sClientName ); // Host
		tOut.PutNumAsString ( dThd.m_iConnID ); // ConnID
		tOut.PutMicrosec ( tmNow-dThd.m_tmStart.get_value_or(tmNow) ); // time
		tOut.PutTimeAsString ( dThd.m_tmTotalWorkedTimeUS ); // work time
		tOut.PutTimeAsString ( dThd.m_tmTotalWorkedCPUTimeUS ); // work CPU time
		tOut.PutPercentAsString ( dThd.m_tmTotalWorkedCPUTimeUS, dThd.m_tmTotalWorkedTimeUS ); // work CPU time %
		tOut.PutNumAsString ( dThd.m_iTotalJobsDone ); // jobs done
		if ( dThd.m_tmLastJobStartTimeUS<0 )
		{
			tOut.PutString ( "-" ); // last job take
			tOut.PutString ( "-" ); // idle for
		} else if ( dThd.m_tmLastJobDoneTimeUS<0 )
		{
			tOut.PutTimeAsString ( tmNow-dThd.m_tmLastJobStartTimeUS ); // last job take
			tOut.PutString ( "No (working)" ); // idle for
		} else
		{
			tOut.PutTimeAsString ( dThd.m_tmLastJobDoneTimeUS-dThd.m_tmLastJobStartTimeUS ); // last job take
			tOut.PutTimestampAsString ( dThd.m_tmLastJobDoneTimeUS ); // idle for
		}

//		tOut.PutString ( dThd.m_sChain.cstr () ); // Chain
		auto tInfo = FormatInfo ( dThd, eFmt, tBuf );
		tOut.PutString ( tInfo.first, Min ( tInfo.second, tStmt.m_iThreadsCols ) ); // Info m_pTaskInfo
		tOut.Commit();
	}

	tOut.Eof();
}

void HandleMysqlFlushHostnames ( RowBuffer_i & tOut )
{
	SmallStringHash_T<DWORD> hHosts;

	// Collect all urls from all distr indexes
	for ( RLockedDistrIt_c it ( g_pDistIndexes ); it.Next (); )
		it.Get ()->ForEveryHost ( [&] ( AgentDesc_t &tDesc )
		{
			if ( tDesc.m_bNeedResolve )
				hHosts.Add ( tDesc.m_uAddr, tDesc.m_sAddr );
		});


	for ( hHosts.IterateStart (); hHosts.IterateNext(); )
	{
		DWORD uRenew = sphGetAddress ( hHosts.IterateGetKey().cstr() );
		if ( uRenew )
			hHosts.IterateGet() = uRenew;
	}

	// copy back renew hosts to distributed agents.
	// case when distr index list changed between collecting urls and applying them
	// is safe, since we are iterate over the list again, and also apply
	// only existing hosts.
	for ( RLockedDistrIt_c it ( g_pDistIndexes ); it.Next (); )
		it.Get ()->ForEveryHost ( [&] ( AgentDesc_t &tDesc ) {
			if ( tDesc.m_bNeedResolve )
			{
				DWORD * pRenew = hHosts ( tDesc.m_sAddr );
				if ( pRenew && *pRenew )
					tDesc.m_uAddr = *pRenew;
			}
		});

	tOut.Ok ( hHosts.GetLength() );
}

void HandleMysqlFlushLogs ( RowBuffer_i & tOut )
{
	sigusr1(1);
	tOut.Ok ();
}

void HandleMysqlReloadIndexes ( RowBuffer_i & tOut )
{
	g_bReloadForced = true;
	sighup(1);
	tOut.Ok ();
}

/////////////////////////////////////////////////////////////////////////////
// user variables these send from master to agents
/////////////////////////////////////////////////////////////////////////////

class UVarRequestBuilder_c : public RequestBuilder_i
{
public:
	UVarRequestBuilder_c ( const char * sName, const CSphVector<SphAttr_t> &dSetValues )
		: m_sName ( sName )
	{
		m_iUserVars = dSetValues.GetLength ();
		m_dBuf.Reset ( m_iUserVars * sizeof ( dSetValues[0] ) + 129 );
		// 129 above is the safe gap for VLB delta encoding 64-bits ints.
		// If we have 1st value 0x8000`0000`0000`0000 - it will occupy 10 bytes VLB,
		// then up to 127 values 0x0100.. - each will occupy 9 bytes VLB,
		// deltas 0x00XX.. takes 8 bytes or less. So, 2+127 bytes gap is enough to cover worst possible case
		// (since 0x80.. + 127 * 0x01.. produce 0xFF.. num, any other delta >0x01.. impossible, since
		// it will cause overflow, and deltals <0x01.. occupy <=8 bytes each).

		SphAttr_t iLast = 0;
		BYTE * pCur = m_dBuf.Begin ();
		for ( const auto &dValue : dSetValues )
		{
			pCur += sphEncodeVLB8 ( pCur, dValue - iLast );
			iLast = dValue;
		}
		m_iLength = pCur-m_dBuf.Begin();
	}

	void BuildRequest ( const AgentConn_t &, ISphOutputBuffer & tOut ) const final
	{
		// API header
		auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_UVAR, VER_COMMAND_UVAR );

		tOut.SendString ( m_sName.cstr() );
		tOut.SendInt ( m_iUserVars );
		tOut.SendArray ( m_dBuf, m_iLength );
	}

	CSphString m_sName;
	CSphFixedVector<BYTE> m_dBuf { 0 };
	int m_iUserVars = 0;
	int m_iLength = 0;
};

class UVarReplyParser_c : public ReplyParser_i
{
public:
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final
	{
		// error got handled at call site
		bool bOk = ( tReq.GetByte()==1 );
		return bOk;
	}
};



static bool SendUserVar ( const char * sIndex, const char * sUserVarName, CSphVector<SphAttr_t> & dSetValues, CSphString & sError )
{
	auto pIndex = GetDistr ( sIndex );
	if ( !pIndex )
	{
		sError.SetSprintf ( "unknown index '%s' in Set statement", sIndex );
		return false;
	}

	VecRefPtrsAgentConn_t dAgents;
	pIndex->GetAllHosts ( dAgents );
	bool bGotLocal = !pIndex->m_dLocal.IsEmpty();

	// FIXME!!! warn on missed agents
	if ( dAgents.IsEmpty() && !bGotLocal )
		return true;

	dSetValues.Uniq();

	// FIXME!!! warn on empty agents
	// connect to remote agents and query them
	if ( !dAgents.IsEmpty() )
	{
		UVarRequestBuilder_c tReqBuilder ( sUserVarName, dSetValues );
		UVarReplyParser_c tParser;
		PerformRemoteTasks ( dAgents, &tReqBuilder, &tParser );
	}

	// should be at the end due to swap of dSetValues values
	if ( bGotLocal )
		SetLocalUserVar ( sUserVarName, dSetValues );

	return true;
}


void HandleCommandUserVar ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_UVAR, tOut ) )
		return;

	CSphString sUserVar = tReq.GetString();
	int iCount = tReq.GetInt();
	CSphVector<SphAttr_t> dUserVar ( iCount );
	int iLength = tReq.GetInt();
	CSphFixedVector<BYTE> dBuf ( iLength );
	tReq.GetBytes ( dBuf.Begin(), iLength );

	if ( tReq.GetError() )
	{
		SendErrorReply ( tOut, "invalid or truncated request" );
		return;
	}

	SphAttr_t iLast = 0;
	const BYTE * pCur = dBuf.Begin();
	ARRAY_FOREACH ( i, dUserVar )
	{
		uint64_t iDelta = 0;
		pCur = spnDecodeVLB8 ( pCur, iDelta );
		assert ( iDelta>0 );
		iLast += iDelta;
		dUserVar[i] = iLast;
	}

	SetLocalUserVar ( sUserVar, dUserVar );

	auto tReply = APIAnswer ( tOut, VER_COMMAND_UVAR );
	tOut.SendInt ( 1 );
}



/////////////////////////////////////////////////////////////////////////////
// SMART UPDATES HANDLER
/////////////////////////////////////////////////////////////////////////////

SphinxqlReplyParser_c::SphinxqlReplyParser_c ( int * pUpd, int * pWarns )
	: m_pUpdated ( pUpd )
	, m_pWarns ( pWarns )
{}

// fixme! reuse code from sphinxql, leave only refs here
bool SphinxqlReplyParser_c::ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const
{
	DWORD uSize = ( tReq.GetLSBDword() & 0x00ffffff ) - 1;
	BYTE uCommand = tReq.GetByte();

	if ( uCommand==0 ) // ok packet
	{
		*m_pUpdated += MysqlUnpack ( tReq, &uSize );
		MysqlUnpack ( tReq, &uSize ); ///< int Insert_id (don't used).
		auto uWarnStatus = tReq.GetLSBDword ();
		*m_pWarns += ( uWarnStatus >> 16 ) & 0xFFFF; ///< num of warnings
		uSize -= 4;
		if ( uSize )
			tReq.GetRawString ( uSize );
		return true;
	}
	if ( uCommand==0xff ) // error packet
	{
		tReq.GetByte();
		tReq.GetByte(); ///< num of errors (2 bytes), we don't use it for now.
		uSize -= 2;
		if ( uSize )
			tReq.GetRawString ( uSize );
	}

	return false;
}


SphinxqlRequestBuilder_c::SphinxqlRequestBuilder_c ( const CSphString& sQuery, const SqlStmt_t & tStmt )
	: m_sBegin ( sQuery.cstr(), tStmt.m_iListStart )
	, m_sEnd ( sQuery.cstr() + tStmt.m_iListEnd, sQuery.Length() - tStmt.m_iListEnd )
{
}


void SphinxqlRequestBuilder_c::BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const
{
	const char* sIndexes = tAgent.m_tDesc.m_sIndexes.cstr();

	// API header
	auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_SPHINXQL, VER_COMMAND_SPHINXQL );
	APIBlob_c dWrapper ( tOut ); // sphinxql wrapped twice, so one more length need to be written.
	tOut.SendBytes ( m_sBegin );
	tOut.SendBytes ( sIndexes );
	tOut.SendBytes ( m_sEnd );
}


//////////////////////////////////////////////////////////////////////////

static void DoExtendedUpdate ( const SqlStmt_t & tStmt, const CSphString & sIndex, const char * sDistributed,
	bool bBlobUpdate, int & iSuccesses, int & iUpdated, SearchFailuresLog_c & dFails, CSphString & sWarning,
	const ServedIndexRefPtr_c & tServed )
{
	CSphString sError;
	// checks
	{
		ServedDescRPtr_c tDesc ( tServed );
		if ( !tDesc )
		{
			dFails.Submit ( sIndex, sDistributed, "index not available" );
			return;
		}

		if ( !CheckIndexCluster ( sIndex, *tDesc, tStmt.m_sCluster, IsHttpStmt ( tStmt ), sError ) )
		{
			dFails.Submit ( sIndex, sDistributed, sError.cstr() );
			return;
		}
	}

	RtAccum_t tAcc ( false );
	ReplicationCommand_t * pCmd = tAcc.AddCommand ( tStmt.m_bJson ? ReplicationCommand_e::UPDATE_JSON : ReplicationCommand_e::UPDATE_QL, tStmt.m_sCluster, sIndex );
	assert ( pCmd );
	pCmd->m_pUpdateAPI = &tStmt.m_tUpdate;
	pCmd->m_bBlobUpdate = bBlobUpdate;
	pCmd->m_pUpdateCond = &tStmt.m_tQuery;

	HandleCmdReplicate ( tAcc, sError, sWarning, iUpdated );

	if ( sError.Length() )
	{
		dFails.Submit ( sIndex, sDistributed, sError.cstr() );
		return;
	}

	iSuccesses++;
}

void HandleMySqlExtendedUpdate ( AttrUpdateArgs & tArgs )
{
	assert ( tArgs.m_pUpdate );
	assert ( tArgs.m_pError );
	assert ( tArgs.m_pWarning );
	assert ( tArgs.m_pIndexName );

	if ( !tArgs.m_pDesc )
	{
		*tArgs.m_pError = "index not available";
		return;
	}

	assert ( tArgs.m_pQuery );
	assert ( tArgs.m_pQuery );

	SearchHandler_c tHandler ( 1, CreateQueryParser ( tArgs.m_bJson ), tArgs.m_pQuery->m_eQueryType, false );
	tArgs.m_pIndex = tArgs.m_pDesc->m_pIndex;

	tHandler.m_dLocked.AddUnmanaged ( *tArgs.m_pIndexName, tArgs.m_pDesc );
	tHandler.RunUpdates ( *tArgs.m_pQuery, *tArgs.m_pIndexName, &tArgs );
}


void sphHandleMysqlUpdate ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt, const CSphString & sQuery, CSphString & sWarning )
{
	CSphString sError;

	// extract index names
	StrVec_t dIndexNames;
	ParseIndexList ( tStmt.m_sIndex, dIndexNames );
	if ( dIndexNames.IsEmpty() )
	{
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	DistrPtrs_t dDistributed;
	// copy distributed indexes description
	CSphString sMissed;
	if ( !ExtractDistributedIndexes ( dIndexNames, dDistributed, sMissed ) )
	{
		sError.SetSprintf ( "unknown index '%s' in update request", sMissed.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// do update
	SearchFailuresLog_c dFails;
	int iSuccesses = 0;
	int iUpdated = 0;
	int iWarns = 0;

	bool bBlobUpdate = false;
	for ( const auto & i : tStmt.m_tUpdate.m_dAttributes )
	{
		if ( i.m_sName==sphGetDocidName() )
		{
			tOut.Error ( tStmt.m_sStmt, "'id' attribute cannot be updated" );
			return;
		}

		bBlobUpdate |= sphIsBlobAttr ( i.m_eType );
	}

	ARRAY_FOREACH ( iIdx, dIndexNames )
	{
		const char * sReqIndex = dIndexNames[iIdx].cstr();
		auto pLocked = GetServed ( sReqIndex );
		if ( pLocked )
		{
			DoExtendedUpdate ( tStmt, sReqIndex, nullptr, bBlobUpdate, iSuccesses, iUpdated, dFails, sWarning, pLocked );

		} else if ( dDistributed[iIdx] )
		{
			assert ( !dDistributed[iIdx]->IsEmpty() );
			const StrVec_t & dLocal = dDistributed[iIdx]->m_dLocal;

			ARRAY_FOREACH ( i, dLocal )
			{
				const char * sLocal = dLocal[i].cstr();
				auto pServed = GetServed ( sLocal );
				DoExtendedUpdate ( tStmt, sLocal, sReqIndex, bBlobUpdate, iSuccesses, iUpdated, dFails, sWarning, pServed );
			}
		}

		// update remote agents
		if ( dDistributed[iIdx] && !dDistributed[iIdx]->m_dAgents.IsEmpty() )
		{
			const DistributedIndex_t * pDist = dDistributed[iIdx];

			VecRefPtrs_t<AgentConn_t *> dAgents;
			pDist->GetAllHosts ( dAgents );

			// connect to remote agents and query them
			CSphScopedPtr<RequestBuilder_i> pRequestBuilder ( CreateRequestBuilder ( sQuery, tStmt ) ) ;
			CSphScopedPtr<ReplyParser_i> pReplyParser ( CreateReplyParser ( tStmt.m_bJson, iUpdated, iWarns ) );
			iSuccesses += PerformRemoteTasks ( dAgents, pRequestBuilder.Ptr (), pReplyParser.Ptr () );
		}
	}

	StringBuilder_c sReport;
	dFails.BuildReport ( sReport );

	if ( !iSuccesses )
	{
		tOut.Error ( tStmt.m_sStmt, sReport.cstr() );
		return;
	}

	tOut.Ok ( iUpdated, iWarns );
}

bool HandleMysqlSelect ( RowBuffer_i & dRows, SearchHandler_c & tHandler )
{
	// lets check all query for errors
	CSphString sError;
	CSphVector<int64_t> dAgentTimes; // dummy for error reporting
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
	{
		CheckQuery ( tHandler.m_dQueries[i], tHandler.m_dResults[i].m_sError );
		if ( !tHandler.m_dResults[i].m_sError.IsEmpty() )
		{
			LogQuery ( tHandler.m_dQueries[i], tHandler.m_dResults[i], dAgentTimes );
			if ( sError.IsEmpty() )
			{
				if ( tHandler.m_dQueries.GetLength()==1 )
					sError = tHandler.m_dResults[0].m_sError;
				else
					sError.SetSprintf ( "query %d error: %s", i, tHandler.m_dResults[i].m_sError.cstr() );
			} else
				sError.SetSprintf ( "%s; query %d error: %s", sError.cstr(), i, tHandler.m_dResults[i].m_sError.cstr() );
		}
	}

	if ( sError.Length() )
	{
		// stmt is intentionally NULL, as we did all the reporting just above
		dRows.Error ( NULL, sError.cstr() );
		return false;
	}

	// actual searching
	tHandler.RunQueries();

	if ( sphInterrupted() )
	{
		sphLogDebug ( "HandleClientMySQL: got SIGTERM, sending the packet MYSQL_ERR_SERVER_SHUTDOWN" );
		dRows.Error ( NULL, "Server shutdown in progress", MYSQL_ERR_SERVER_SHUTDOWN );
		return false;
	}

	return true;
}

inline static int Bit ( int iBit, const unsigned int * pData )
{
	return ( ( pData[iBit / 32] & ( 1 << ( iBit % 32 ) ) ) ? 1 : 0 );
}

void sphFormatFactors ( StringBuilder_c & sOut, const unsigned int * pFactors, bool bJson )
{
	sOut.GrowEnough ( 512 );

	// format lines for header, fields, words
	const char * sBmFmt = nullptr;
	const char * sFieldFmt = nullptr;
	const char * sWordFmt = nullptr;
	ScopedComma_c sDelim;
	if ( bJson )
	{
		sBmFmt = R"("bm25":%d, "bm25a":%f, "field_mask":%u, "doc_word_count":%d)";
		sFieldFmt = R"({"field":%d, "lcs":%u, "hit_count":%u, "word_count":%u, "tf_idf":%f, "min_idf":%f, )"
			R"("max_idf":%f, "sum_idf":%f, "min_hit_pos":%d, "min_best_span_pos":%d, "exact_hit":%u, )"
	 		R"("max_window_hits":%d, "min_gaps":%d, "exact_order":%u, "lccs":%d, "wlccs":%f, "atc":%f})";
		sWordFmt = R"(%i{"tf":%d, "idf":%f})";
		sDelim.Init ( sOut, ", ", "{", "}" );

	} else
	{
		sBmFmt = "bm25=%d, bm25a=%f, field_mask=%u, doc_word_count=%d";
		sFieldFmt = "field%d=(lcs=%u, hit_count=%u, word_count=%u, tf_idf=%f, min_idf=%f, max_idf=%f, sum_idf=%f, "
					"min_hit_pos=%d, min_best_span_pos=%d, exact_hit=%u, max_window_hits=%d, "
					"min_gaps=%d, exact_order=%u, lccs=%d, wlccs=%f, atc=%f)";
		sWordFmt = "word%d=(tf=%d, idf=%f)";
		sDelim.Init ( sOut, ", " );
	}
#define DI( _factor ) sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_##_factor )
#define DF( _factor ) sphinx_get_doc_factor_float ( pFactors, SPH_DOCF_##_factor )
	sOut.Sprintf ( sBmFmt, DI( BM25 ), DF( BM25A ), DI( MATCHED_FIELDS ), DI( DOC_WORD_COUNT ) );
	{ ScopedComma_c sFields;
		if ( bJson )
			sFields.Init ( sOut, ", ", R"("fields":[)", "]");

		auto pExactHit		= sphinx_get_doc_factor_ptr ( pFactors, SPH_DOCF_EXACT_HIT_MASK );
		auto pExactOrder	= sphinx_get_doc_factor_ptr ( pFactors, SPH_DOCF_EXACT_ORDER_MASK );
		int iFields = DI ( NUM_FIELDS );
		for ( int i = 0; i<iFields; ++i )
		{
#define FI( _factor ) sphinx_get_field_factor_int ( pField, SPH_FIELDF_##_factor )
#define FF( _factor ) sphinx_get_field_factor_float ( pField, SPH_FIELDF_##_factor )
			auto pField = sphinx_get_field_factors ( pFactors, i );
			if ( !FI (HIT_COUNT) )
				continue;
			sOut.Sprintf ( sFieldFmt, i, FI (LCS), FI (HIT_COUNT), FI (WORD_COUNT), FF (TF_IDF), FF (MIN_IDF),
				FF (MAX_IDF), FF (SUM_IDF), FI (MIN_HIT_POS), FI (MIN_BEST_SPAN_POS), Bit (i, pExactHit),
				FI (MAX_WINDOW_HITS), FI (MIN_GAPS), Bit (i, pExactOrder), FI (LCCS), FF (WLCCS), FF (ATC) );
#undef FF
#undef FI
		}
	} // fields block

	{ ScopedComma_c sWords;
		if ( bJson )
			sWords.Init ( sOut, ", ", R"("words":[)", "]" );

		auto iUniqQpos = DI ( MAX_UNIQ_QPOS );
		for ( int i = 0; i<iUniqQpos; ++i )
		{
			auto pTerm = sphinx_get_term_factors ( pFactors, i + 1 );
			if ( !sphinx_get_term_factor_int ( pTerm, SPH_TERMF_KEYWORD_MASK ) )
				continue;
			sOut.Sprintf ( sWordFmt, i, sphinx_get_term_factor_int ( pTerm, SPH_TERMF_TF ),
				sphinx_get_term_factor_float ( pTerm, SPH_TERMF_IDF ) );
		}
	} // words block
#undef DF
#undef DI
}


static void ReturnZeroCount ( const CSphSchema & tSchema, const CSphBitvec & tAttrsToSend, const StrVec_t & dCounts,
	RowBuffer_i & dRows )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
	{
		if ( !tAttrsToSend.BitGet(i) )
			continue;

		const CSphColumnInfo & tCol = tSchema.GetAttr ( i );

		// @count or its alias or count(distinct attr_name)
		if ( dCounts.Contains ( tCol.m_sName ) )
		{
			dRows.PutNumAsString ( 0 );
		} else
		{
			// essentially the same as SELECT_DUAL, parse and print constant expressions
			ESphAttr eAttrType;
			CSphString sError;
			ExprParseArgs_t tExprArgs;
			tExprArgs.m_pAttrType = &eAttrType;
			ISphExprRefPtr_c pExpr { sphExprParse ( tCol.m_sName.cstr(), tSchema, sError, tExprArgs )};

			if ( !pExpr || !pExpr->IsConst() )
				eAttrType = SPH_ATTR_NONE;

			CSphMatch tMatch;
			const BYTE * pStr = nullptr;

			switch ( eAttrType )
			{
				case SPH_ATTR_STRINGPTR:
					pExpr->StringEval ( tMatch, &pStr );
					dRows.PutString ( (const char *)pStr );
					SafeDelete ( pStr );
					break;
				case SPH_ATTR_INTEGER: dRows.PutNumAsString ( pExpr->IntEval ( tMatch ) ); break;
				case SPH_ATTR_BIGINT: dRows.PutNumAsString ( pExpr->Int64Eval ( tMatch ) ); break;
				case SPH_ATTR_FLOAT:	dRows.PutFloatAsString ( pExpr->Eval ( tMatch ) ); break;
				default:
					dRows.PutNULL();
					break;
			}
		}
	}
	dRows.Commit();
}


void SendMysqlSelectResult ( RowBuffer_i & dRows, const AggrResult_t & tRes, bool bMoreResultsFollow, bool bAddQueryColumn, const CSphString * pQueryColumn, CSphQueryProfile * pProfile )
{
	CSphScopedProfile tProf ( pProfile, SPH_QSTATE_NET_WRITE );

	if ( !tRes.m_iSuccesses )
	{
		// at this point, SELECT error logging should have been handled, so pass a NULL stmt to logger
		dRows.Error ( nullptr, tRes.m_sError.cstr() );
		return;
	}

	// empty result sets just might carry the full uberschema
	// bummer! lets protect ourselves against that
	CSphBitvec tAttrsToSend;
	bool bReturnZeroCount = !tRes.m_dZeroCount.IsEmpty();
	if ( tRes.m_dMatches.GetLength() || bReturnZeroCount )
		sphGetAttrsToSend ( tRes.m_tSchema, false, true, tAttrsToSend );

	// field packets
	if ( tRes.m_dMatches.IsEmpty() && !bReturnZeroCount )
	{
		// in case there are no matches, send a dummy schema
		// result set header packet. We will attach EOF manually at the end.
		dRows.HeadBegin ( bAddQueryColumn ? 2 : 1 );
		dRows.HeadColumn ( "id", MYSQL_COL_LONGLONG, 0 );
	} else
	{
		int iAttrsToSend = tAttrsToSend.BitCount();
		if ( bAddQueryColumn )
			iAttrsToSend++;

		dRows.HeadBegin ( iAttrsToSend );
		for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); ++i )
		{
			if ( !tAttrsToSend.BitGet(i) )
				continue;

			const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
			MysqlColumnType_e eType = MYSQL_COL_STRING;
			switch ( tCol.m_eAttrType )
			{
			case SPH_ATTR_INTEGER:
			case SPH_ATTR_TIMESTAMP:
			case SPH_ATTR_BOOL:
				eType = MYSQL_COL_LONG; break;
			case SPH_ATTR_FLOAT:
				eType = MYSQL_COL_FLOAT; break;
			case SPH_ATTR_BIGINT:
				eType = MYSQL_COL_LONGLONG; break;
			default:
				break;
			}
			dRows.HeadColumn ( tCol.m_sName.cstr(), eType, 0 );
		}
	}

	if ( bAddQueryColumn )
		dRows.HeadColumn ( "query", MYSQL_COL_STRING );

	// EOF packet is sent explicitly due to non-default params.
	auto iWarns = tRes.m_sWarning.IsEmpty() ? 0 : 1;
	dRows.HeadEnd ( bMoreResultsFollow, iWarns );

	// FIXME!!! replace that vector relocations by SqlRowBuffer

	// rows
	const CSphSchema &tSchema = tRes.m_tSchema;
	for ( int iMatch = tRes.m_iOffset; iMatch < tRes.m_iOffset + tRes.m_iCount; ++iMatch )
	{
		const CSphMatch & tMatch = tRes.m_dMatches [ iMatch ];
		for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); ++i )
		{
			if ( !tAttrsToSend.BitGet(i) )
				continue;

			const CSphColumnInfo & dAttr = tSchema.GetAttr(i);
			CSphAttrLocator tLoc = dAttr.m_tLocator;
			ESphAttr eAttrType = dAttr.m_eAttrType;

			assert ( sphPlainAttrToPtrAttr(eAttrType)==eAttrType );

			switch ( eAttrType )
			{
			case SPH_ATTR_INTEGER:
			case SPH_ATTR_TIMESTAMP:
			case SPH_ATTR_BOOL:
			case SPH_ATTR_TOKENCOUNT:
				dRows.PutNumAsString ( ( DWORD ) tMatch.GetAttr ( tLoc ) );
				break;

			case SPH_ATTR_BIGINT:
				dRows.PutNumAsString( tMatch.GetAttr(tLoc) );
				break;

			case SPH_ATTR_FLOAT:
				dRows.PutFloatAsString ( tMatch.GetAttrFloat(tLoc) );
				break;

			case SPH_ATTR_INT64SET_PTR:
			case SPH_ATTR_UINT32SET_PTR:
				{
					StringBuilder_c dStr;
					sphPackedMVA2Str ( (const BYTE *)tMatch.GetAttr(tLoc), eAttrType==SPH_ATTR_INT64SET_PTR, dStr );
					dRows.PutArray ( dStr, false );
					break;
				}

			case SPH_ATTR_STRINGPTR:
				{
					auto * pString = ( const BYTE * ) tMatch.GetAttr ( tLoc );
					int iLen=0;
					if ( pString )
						iLen = sphUnpackPtrAttr ( pString, &pString );
					if ( pString && iLen>1 && pString[iLen - 2]=='\0' )
						iLen -= 2;
					dRows.PutArray ( pString, iLen );
				}
				break;
			case SPH_ATTR_JSON_PTR:
				{
					auto * pString = (const BYTE*) tMatch.GetAttr ( tLoc );
					JsonEscapedBuilder sTmp;
					if ( pString )
					{
						sphUnpackPtrAttr ( pString, &pString );
						sphJsonFormat ( sTmp, pString );
					}
					dRows.PutArray ( sTmp );
				}
				break;

			case SPH_ATTR_FACTORS:
			case SPH_ATTR_FACTORS_JSON:
				{
					const BYTE * pFactors = nullptr;
					sphUnpackPtrAttr ( (const BYTE*)tMatch.GetAttr ( tLoc ), &pFactors );
					StringBuilder_c sTmp;
					if ( pFactors )
						sphFormatFactors ( sTmp, (const unsigned int *)pFactors, eAttrType==SPH_ATTR_FACTORS_JSON );
					dRows.PutArray ( sTmp, false );
					break;
				}

			case SPH_ATTR_JSON_FIELD_PTR:
				{
					const BYTE * pField = (const BYTE *)tMatch.GetAttr ( tLoc );
					if ( !pField )
					{
						dRows.PutNULL();
						break;
					}

					sphUnpackPtrAttr ( pField, &pField );
					auto eJson = ESphJsonType ( *pField++ );

					if ( eJson==JSON_NULL )
					{
						dRows.PutNULL();
						break;
					}
			
					// send string to client
					JsonEscapedBuilder sTmp;
					sphJsonFieldFormat ( sTmp, pField, eJson, false );
					dRows.PutArray ( sTmp, false );
					break;
				}

			default:
				dRows.Add(1);
				dRows.Add('-');
				break;
			}
		}

		if ( bAddQueryColumn )
		{
			assert ( pQueryColumn );
			dRows.PutString ( *pQueryColumn );
		}

		dRows.Commit();
	}

	if ( bReturnZeroCount )
		ReturnZeroCount ( tRes.m_tSchema, tAttrsToSend, tRes.m_dZeroCount, dRows );

	// eof packet
	dRows.Eof ( bMoreResultsFollow, iWarns );
}


void HandleMysqlWarning ( const CSphQueryResultMeta & tLastMeta, RowBuffer_i & dRows, bool bMoreResultsFollow )
{
	// can't send simple ok if there are more results to send
	// as it breaks order of multi-result output
	if ( tLastMeta.m_sWarning.IsEmpty() && !bMoreResultsFollow )
	{
		dRows.Ok();
		return;
	}

	// result set header packet
	dRows.HeadBegin(3);
	dRows.HeadColumn ( "Level" );
	dRows.HeadColumn ( "Code", MYSQL_COL_DECIMAL );
	dRows.HeadColumn ( "Message" );
	dRows.HeadEnd ( bMoreResultsFollow );

	// row
	dRows.PutString ( "warning" );
	dRows.PutString ( "1000" );
	dRows.PutString ( tLastMeta.m_sWarning );
	dRows.Commit();

	// cleanup
	dRows.Eof ( bMoreResultsFollow );
}

void HandleMysqlMeta ( RowBuffer_i & dRows, const SqlStmt_t & tStmt, const CSphQueryResultMeta & tLastMeta, bool bMoreResultsFollow )
{
	VectorLike dStatus ( tStmt.m_sStringParam );

	switch ( tStmt.m_eStmt )
	{
	case STMT_SHOW_STATUS:
		BuildStatus ( dStatus );
		break;
	case STMT_SHOW_META:
		BuildMeta ( dStatus, tLastMeta );
		break;
	case STMT_SHOW_AGENT_STATUS:
		BuildAgentStatus ( dStatus, tStmt.m_sIndex );
		break;
	default:
		assert(0); // only 'show' statements allowed here.
		break;
	}

	// result set header packet
	dRows.HeadTuplet ( dStatus.szColKey(), dStatus.szColValue() );

	// send rows
	for ( int iRow=0; iRow<dStatus.GetLength(); iRow+=2 )
		dRows.DataTuplet ( dStatus[iRow+0].cstr(), dStatus[iRow+1].cstr() );

	// cleanup
	dRows.Eof ( bMoreResultsFollow );
}

static void PercolateDeleteDocuments ( const CSphString & sIndex, const CSphString & sCluster, const SqlStmt_t & tStmt, RtAccum_t & tAccum, CSphString & sError )
{
	// prohibit double copy of filters
	const CSphQuery & tQuery = tStmt.m_tQuery;
	CSphScopedPtr<ReplicationCommand_t> pCmd ( new ReplicationCommand_t );
	pCmd->m_eCommand = ReplicationCommand_e::PQUERY_DELETE;
	pCmd->m_sIndex = sIndex;
	pCmd->m_sCluster = sCluster;

	if ( tQuery.m_dFilters.GetLength()>1 )
	{
		sError.SetSprintf ( "only single filter supported, got %d", tQuery.m_dFilters.GetLength() );
		return;
	}

	if ( tQuery.m_dFilters.GetLength() )
	{
		const CSphFilterSettings * pFilter = tQuery.m_dFilters.Begin();
		if ( ( pFilter->m_bHasEqualMin || pFilter->m_bHasEqualMax ) && !pFilter->m_bExclude && pFilter->m_eType==SPH_FILTER_VALUES
			&& ( pFilter->m_sAttrName=="@id" || pFilter->m_sAttrName=="id" || pFilter->m_sAttrName=="uid" ) )
		{
			pCmd->m_dDeleteQueries.Reserve ( pFilter->GetNumValues() );
			const SphAttr_t * pA = pFilter->GetValueArray();
			for ( int i = 0; i < pFilter->GetNumValues(); ++i )
				pCmd->m_dDeleteQueries.Add ( pA[i] );
		} else if ( pFilter->m_eType==SPH_FILTER_STRING && pFilter->m_sAttrName=="tags" && pFilter->m_dStrings.GetLength() )
		{
			pCmd->m_sDeleteTags = pFilter->m_dStrings[0].cstr();
		} else if ( pFilter->m_eType==SPH_FILTER_STRING_LIST && pFilter->m_sAttrName=="tags" && pFilter->m_dStrings.GetLength() )
		{
			StringBuilder_c tBuf;
			tBuf.StartBlock ( "," );
			for ( const CSphString & sVal : pFilter->m_dStrings )
				tBuf << sVal;
			tBuf.FinishBlock ();

			pCmd->m_sDeleteTags = tBuf.cstr();
		}
		else
		{
			sError.SetSprintf ( "unsupported filter type %d, attribute '%s'", pFilter->m_eType, pFilter->m_sAttrName.cstr() );
			return;
		}
	}

	tAccum.m_dCmd.Add ( pCmd.LeakPtr() );
}


static int LocalIndexDoDeleteDocuments ( const CSphString & sName, const CSphString & sCluster, const char * sDistributed,
	const SqlStmt_t & tStmt, const DocID_t * pDocs, int iCount, SearchFailuresLog_c & dErrors, bool bCommit, CSphSessionAccum & tAcc )
{
	CSphString sError;

	ServedDescRPtr_c pLocked ( GetServed ( sName ) );
	if ( !pLocked || !pLocked->m_pIndex )
	{
		dErrors.Submit ( sName, sDistributed, "index not available" );
		return 0;
	}

	auto * pIndex = static_cast<RtIndex_i *> ( pLocked->m_pIndex );
	if ( !ServedDesc_t::IsMutable ( pLocked ) )
	{
		sError.SetSprintf ( "does not support DELETE" );
		dErrors.Submit ( sName, sDistributed, sError.cstr() );
		return 0;
	}

	RtAccum_t * pAccum = tAcc.GetAcc ( pIndex, sError );
	if ( !sError.IsEmpty() )
	{
		dErrors.Submit ( sName, sDistributed, sError.cstr() );
		return 0;
	}

	if ( !CheckIndexCluster ( sName, *pLocked, sCluster, IsHttpStmt ( tStmt ), sError ) )
	{
		dErrors.Submit ( sName, sDistributed, sError.cstr() );
		return 0;
	} 

	int iAffected = 0;

	// goto to percolate path with unlocked index
	if ( pLocked->m_eType==IndexType_e::PERCOLATE )
	{
		PercolateDeleteDocuments ( sName, sCluster, tStmt, *pAccum, sError );
		if ( !sError.IsEmpty() )
		{
			dErrors.Submit ( sName, sDistributed, sError.cstr() );
			return 0;
		}
	} else
	{
		CSphScopedPtr<SearchHandler_c> pHandler ( nullptr );
		CSphVector<DocID_t> dValues;
		if ( !pDocs ) // needs to be deleted via query
		{
			pHandler = new SearchHandler_c ( 1, CreateQueryParser ( tStmt.m_bJson ), tStmt.m_tQuery.m_eQueryType, false );
			pHandler->m_dLocked.AddUnmanaged ( sName, pLocked );
			pHandler->RunDeletes ( tStmt.m_tQuery, sName, &sError, &dValues );
			pDocs = dValues.Begin();
			iCount = dValues.GetLength();
		}

		if ( !pIndex->DeleteDocument ( pDocs, iCount, sError, pAccum ) )
		{
			dErrors.Submit ( sName, sDistributed, sError.cstr() );
			return 0;
		}

		pAccum->AddCommand ( ReplicationCommand_e::RT_TRX, sCluster, sName );
	}

	if ( bCommit )
		HandleCmdReplicate ( *pAccum, sError, iAffected );

	return iAffected;
}


void sphHandleMysqlDelete ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt, const CSphString & sQuery, bool bCommit,
		CSphSessionAccum & tAcc )
{
	MEMORY ( MEM_SQL_DELETE );

	CSphString sError;

	StrVec_t dNames;
	ParseIndexList ( tStmt.m_sIndex, dNames );
	if ( dNames.IsEmpty() )
	{
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	DistrPtrs_t dDistributed;
	CSphString sMissed;
	if ( !ExtractDistributedIndexes ( dNames, dDistributed, sMissed ) )
	{
		sError.SetSprintf ( "unknown index '%s' in delete request", sMissed.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// delete to agents works only with commit=1
	if ( !bCommit  )
	{
		for ( auto &pDist : dDistributed )
		{
			if ( !pDist || pDist->m_dAgents.IsEmpty() )
				continue;

			sError.SetSprintf ( "index '%s': DELETE is not supported on agents when autocommit=0", tStmt.m_sIndex.cstr() );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
	}

	const DocID_t * pDocs = nullptr;
	int iDocsCount = 0;
	CSphVector<DocID_t> dDeleteIds;

	// now check the short path - if we have clauses 'id=smth' or 'id in (xx,yy)' or 'id in @uservar' - we know
	// all the values list immediatelly and don't have to run the heavy query here.
	const CSphQuery & tQuery = tStmt.m_tQuery; // shortcut

	if ( tQuery.m_sQuery.IsEmpty() && tQuery.m_dFilters.GetLength()==1 && !tQuery.m_dFilterTree.GetLength() )
	{
		const CSphFilterSettings* pFilter = tQuery.m_dFilters.Begin();
		if ( ( pFilter->m_bHasEqualMin || pFilter->m_bHasEqualMax ) && pFilter->m_eType==SPH_FILTER_VALUES
			&& ( pFilter->m_sAttrName=="@id" || pFilter->m_sAttrName=="id" ) && !pFilter->m_bExclude )
		{
			pDocs = (DocID_t *)pFilter->GetValueArray();
			iDocsCount = pFilter->GetNumValues();
		}
	}

	// do delete
	SearchFailuresLog_c dErrors;
	int iAffected = 0;

	// delete for local indexes
	ARRAY_FOREACH ( iIdx, dNames )
	{
		const CSphString & sName = dNames[iIdx];
		bool bLocal = g_pLocalIndexes->Contains ( sName );
		if ( bLocal )
		{
			iAffected += LocalIndexDoDeleteDocuments ( sName, tStmt.m_sCluster, nullptr,
				tStmt, pDocs, iDocsCount, dErrors, bCommit, tAcc );
		}
		else if ( dDistributed[iIdx] )
		{
			assert ( !dDistributed[iIdx]->IsEmpty() );
			for ( const CSphString& sLocal : dDistributed[iIdx]->m_dLocal )
			{
				bool bDistLocal = g_pLocalIndexes->Contains ( sLocal );
				if ( bDistLocal )
				{
					iAffected += LocalIndexDoDeleteDocuments ( sLocal, tStmt.m_sCluster, sName.cstr(),
						tStmt, pDocs, iDocsCount, dErrors, bCommit, tAcc );
				}
			}
		}

		// delete for remote agents
		if ( dDistributed[iIdx] && dDistributed[iIdx]->m_dAgents.GetLength() )
		{
			const DistributedIndex_t * pDist = dDistributed[iIdx];
			VecRefPtrsAgentConn_t dAgents;
			pDist->GetAllHosts ( dAgents );

			int iGot = 0;
			int iWarns = 0;

			// connect to remote agents and query them
			CSphScopedPtr<RequestBuilder_i> pRequestBuilder ( CreateRequestBuilder ( sQuery, tStmt ) ) ;
			CSphScopedPtr<ReplyParser_i> pReplyParser ( CreateReplyParser ( tStmt.m_bJson, iGot, iWarns ) );
			PerformRemoteTasks ( dAgents, pRequestBuilder.Ptr (), pReplyParser.Ptr () );

			// FIXME!!! report error & warnings from agents
			// FIXME? profile update time too?
			iAffected += iGot;
		}
	}

	if ( !dErrors.IsEmpty() )
	{
		StringBuilder_c sReport;
		dErrors.BuildReport ( sReport );
		tOut.Error ( tStmt.m_sStmt, sReport.cstr() );
		return;
	}

	tOut.Ok ( iAffected );
}


struct SessionVars_t
{
	bool			m_bAutoCommit = true;
	bool			m_bInTransaction = false;
	ESphCollation	m_eCollation { g_eCollation };
	bool			m_bProfile = false;
	int				m_iTimeoutS = -1;
	int				m_iThrottlingMS = -1;
	CSphVector<int64_t> m_dLastIds;
};

// fwd
void HandleMysqlShowProfile ( RowBuffer_i & tOut, const CSphQueryProfile & p, bool bMoreResultsFollow );
static void HandleMysqlShowPlan ( RowBuffer_i & tOut, const CSphQueryProfile & p, bool bMoreResultsFollow );


class CSphQueryProfileMysql final : public CSphQueryProfile
{
public:
	void			BuildResult ( XQNode_t * pRoot, const CSphSchema & tSchema, const StrVec_t& dZones ) final;
	const char *	GetResultAsStr() const final;

	CSphQueryProfile * Clone () const final
	{
		return new CSphQueryProfileMysql;
	}

private:
	CSphString				m_sResult;
};


void CSphQueryProfileMysql::BuildResult ( XQNode_t * pRoot, const CSphSchema & tSchema, const StrVec_t& dZones )
{
	m_sResult = sphExplainQuery ( pRoot, tSchema, dZones );
}


const char* CSphQueryProfileMysql::GetResultAsStr() const
{
	return m_sResult.cstr();
}


void HandleMysqlMultiStmt ( const CSphVector<SqlStmt_t> & dStmt, CSphQueryResultMeta & tLastMeta, RowBuffer_i & dRows,
		const CSphString & sWarning )
{
	// select count
	int iSelect = 0;
	ARRAY_FOREACH ( i, dStmt )
		if ( dStmt[i].m_eStmt==STMT_SELECT )
			iSelect++;

	CSphQueryResultMeta tPrevMeta = tLastMeta;

	myinfo::SetCommand ( g_dSqlStmts[STMT_SELECT] );
	for ( int i=0; i<iSelect; i++ )
		StatCountCommand ( SEARCHD_COMMAND_SEARCH );

	// setup query for searching
	SearchHandler_c tHandler ( iSelect, sphCreatePlainQueryParser(), QUERY_SQL, true );
	SessionVars_t tVars;
	CSphQueryProfileMysql tProfile;

	iSelect = 0;
	ARRAY_FOREACH ( i, dStmt )
	{
		if ( dStmt[i].m_eStmt==STMT_SELECT )
		{
			tHandler.SetQuery ( iSelect, dStmt[i].m_tQuery, dStmt[i].m_pTableFunc );
			dStmt[i].m_pTableFunc = nullptr;
			iSelect++;
		}
		else if ( dStmt[i].m_eStmt==STMT_SET && dStmt[i].m_eSet==SET_LOCAL )
		{
			CSphString sSetName ( dStmt[i].m_sSetName );
			sSetName.ToLower();
			tVars.m_bProfile = ( sSetName=="profiling" && dStmt[i].m_iSetValue!=0 );
		}
	}

	// use first meta for faceted search
	bool bUseFirstMeta = ( tHandler.m_dQueries.GetLength()>1 && !tHandler.m_dQueries[0].m_bFacet && tHandler.m_dQueries[1].m_bFacet );
	if ( tVars.m_bProfile )
		tHandler.SetProfile ( &tProfile );

	// do search
	bool bSearchOK = true;
	if ( iSelect )
	{
		bSearchOK = HandleMysqlSelect ( dRows, tHandler );

		// save meta for SHOW *
		tLastMeta = bUseFirstMeta ? tHandler.m_dResults[0] : tHandler.m_dResults.Last();

		// fix up overall query time
		if ( bUseFirstMeta )
			for ( int i=1; i<tHandler.m_dResults.GetLength(); i++ )
			{
				tLastMeta.m_iQueryTime += tHandler.m_dResults[i].m_iQueryTime;
				tLastMeta.m_iCpuTime += tHandler.m_dResults[i].m_iCpuTime;
				tLastMeta.m_iAgentCpuTime += tHandler.m_dResults[i].m_iAgentCpuTime;
			}
	}

	if ( !bSearchOK )
		return;

	// send multi-result set
	iSelect = 0;
	ARRAY_FOREACH ( i, dStmt )
	{
		SqlStmt_e eStmt = dStmt[i].m_eStmt;
		myinfo::SetCommand ( g_dSqlStmts[eStmt] );

		const CSphQueryResultMeta & tMeta = bUseFirstMeta ? tHandler.m_dResults[0] : ( iSelect-1>=0 ? tHandler.m_dResults[iSelect-1] : tPrevMeta );
		bool bMoreResultsFollow = (i+1)<dStmt.GetLength();

		if ( eStmt==STMT_SELECT )
		{
			AggrResult_t & tRes = tHandler.m_dResults[iSelect++];
			if ( !sWarning.IsEmpty() )
				tRes.m_sWarning = sWarning;
			SendMysqlSelectResult ( dRows, tRes, bMoreResultsFollow, false, nullptr, ( tVars.m_bProfile ? &tProfile : nullptr ) );
			// mysql server breaks send on error
			if ( !tRes.m_iSuccesses )
				break;
		} else if ( eStmt==STMT_SHOW_WARNINGS )
			HandleMysqlWarning ( tMeta, dRows, bMoreResultsFollow );
		else if ( eStmt==STMT_SHOW_STATUS || eStmt==STMT_SHOW_META || eStmt==STMT_SHOW_AGENT_STATUS )
			HandleMysqlMeta ( dRows, dStmt[i], tMeta, bMoreResultsFollow ); // FIXME!!! add prediction counters
		else if ( eStmt==STMT_SET ) // TODO implement all set statements and make them handle bMoreResultsFollow flag
			dRows.Ok ( 0, 0, NULL, bMoreResultsFollow );
		else if ( eStmt==STMT_SHOW_PROFILE )
			HandleMysqlShowProfile ( dRows, tProfile, bMoreResultsFollow );
		else if ( eStmt==STMT_SHOW_PLAN )
			HandleMysqlShowPlan ( dRows, tProfile, bMoreResultsFollow );

		if ( sphInterrupted() )
		{
			sphLogDebug ( "HandleMultiStmt: got SIGTERM, sending the packet MYSQL_ERR_SERVER_SHUTDOWN" );
			dRows.Error ( NULL, "Server shutdown in progress", MYSQL_ERR_SERVER_SHUTDOWN );
			return;
		}
	}
}

static ESphCollation sphCollationFromName ( const CSphString & sName, CSphString * pError )
{
	assert ( pError );

	// FIXME! replace with a hash lookup?
	if ( sName=="libc_ci" )
		return SPH_COLLATION_LIBC_CI;
	else if ( sName=="libc_cs" )
		return SPH_COLLATION_LIBC_CS;
	else if ( sName=="utf8_general_ci" )
		return SPH_COLLATION_UTF8_GENERAL_CI;
	else if ( sName=="binary" )
		return SPH_COLLATION_BINARY;

	pError->SetSprintf ( "Unknown collation: '%s'", sName.cstr() );
	return SPH_COLLATION_DEFAULT;
}

void HandleMysqlSet ( RowBuffer_i & tOut, SqlStmt_t & tStmt, SessionVars_t & tVars, CSphSessionAccum & tAcc )
{
	MEMORY ( MEM_SQL_SET );
	CSphString sError;

	tStmt.m_sSetName.ToLower();
	switch ( tStmt.m_eSet )
	{
	case SET_LOCAL:

		if ( tStmt.m_sSetName=="wait_timeout" )
		{
			tVars.m_iTimeoutS = tStmt.m_iSetValue;
			break;
		}

		if ( tStmt.m_sSetName=="throttling_period" )
		{
			tVars.m_iThrottlingMS = tStmt.m_iSetValue;
			break;
		}

		// move check here from bison parser. Only boolean allowed below.
		if ( tStmt.m_iSetValue!=0 && tStmt.m_iSetValue!=1 )
		{
			tOut.ErrorEx ( tStmt.m_sStmt, "sphinxql: only 0 and 1 could be used as boolean values near '%d'", tStmt.m_iSetValue );
			return;
		}

		if ( tStmt.m_sSetName=="autocommit" )
		{
			// per-session AUTOCOMMIT
			tVars.m_bAutoCommit = ( tStmt.m_iSetValue!=0 );
			tVars.m_bInTransaction = false;

			// commit all pending changes
			if ( tVars.m_bAutoCommit )
			{
				RtIndex_i * pIndex = tAcc.GetIndex();
				if ( pIndex )
				{
					RtAccum_t * pAccum = tAcc.GetAcc ( pIndex, sError );
					if ( !sError.IsEmpty() )
					{
						tOut.Error ( tStmt.m_sStmt, sError.cstr() );
						return;
					} else
					{
						HandleCmdReplicate ( *pAccum, sError );
					}
				}
			}
		} else if ( tStmt.m_sSetName=="collation_connection" )
		{
			// per-session COLLATION_CONNECTION
			CSphString & sVal = tStmt.m_sSetValue;
			sVal.ToLower();

			tVars.m_eCollation = sphCollationFromName ( sVal, &sError );
			if ( !sError.IsEmpty() )
			{
				tOut.Error ( tStmt.m_sStmt, sError.cstr() );
				return;
			}
		} else if ( tStmt.m_sSetName=="character_set_results"
			|| tStmt.m_sSetName=="sql_auto_is_null"
			|| tStmt.m_sSetName=="sql_safe_updates"
			|| tStmt.m_sSetName=="sql_mode" )
		{
			// per-session CHARACTER_SET_RESULTS et al; just ignore for now

		} else if ( tStmt.m_sSetName=="profiling" )
		{
			// per-session PROFILING
			tVars.m_bProfile = ( tStmt.m_iSetValue!=0 );

		} else
		{
			// unknown variable, return error
			tOut.ErrorEx ( tStmt.m_sStmt, "Unknown session variable '%s' in SET statement", tStmt.m_sSetName.cstr () );
			return;
		}
		break;

	case SET_GLOBAL_UVAR:
	{
		// global user variable
		// INT_SET type must be sorted
		tStmt.m_dSetValues.Sort();
		SetLocalUserVar ( tStmt.m_sSetName, tStmt.m_dSetValues );
		break;
	}

	case SET_GLOBAL_SVAR:
		// global server variable
		if ( tStmt.m_sSetName=="query_log_format" )
		{
			if ( tStmt.m_sSetValue=="plain" )
				g_eLogFormat = LOG_FORMAT_PLAIN;
			else if ( tStmt.m_sSetValue=="sphinxql" )
				g_eLogFormat = LOG_FORMAT_SPHINXQL;
			else
			{
				tOut.Error ( tStmt.m_sStmt, "Unknown query_log_format value (must be plain or sphinxql)" );
				return;
			}
		} else if ( tStmt.m_sSetName=="log_level" )
		{
			if ( tStmt.m_sSetValue=="info" )
				g_eLogLevel = SPH_LOG_INFO;
			else if ( tStmt.m_sSetValue=="debug" )
				g_eLogLevel = SPH_LOG_DEBUG;
			else if ( tStmt.m_sSetValue=="debugv" )
				g_eLogLevel = SPH_LOG_VERBOSE_DEBUG;
			else if ( tStmt.m_sSetValue=="debugvv" )
				g_eLogLevel = SPH_LOG_VERY_VERBOSE_DEBUG;
			else if ( tStmt.m_sSetValue=="replication" )
				g_eLogLevel = SPH_LOG_RPL_DEBUG;
			else
			{
				tOut.Error ( tStmt.m_sStmt, "Unknown log_level value (must be one of info, debug, debugv, debugvv, replication)" );
				return;
			}
		} else if ( tStmt.m_sSetName=="query_log_min_msec" )
		{
			g_iQueryLogMinMs = (int)tStmt.m_iSetValue;
		} else if ( tStmt.m_sSetName=="qcache_max_bytes" )
		{
			const QcacheStatus_t & s = QcacheGetStatus();
			QcacheSetup ( tStmt.m_iSetValue, s.m_iThreshMs, s.m_iTtlS );
		} else if ( tStmt.m_sSetName=="qcache_thresh_msec" )
		{
			const QcacheStatus_t & s = QcacheGetStatus();
			QcacheSetup ( s.m_iMaxBytes, (int)tStmt.m_iSetValue, s.m_iTtlS );
		} else if ( tStmt.m_sSetName=="qcache_ttl_sec" )
		{
			const QcacheStatus_t & s = QcacheGetStatus();
			QcacheSetup ( s.m_iMaxBytes, s.m_iThreshMs, (int)tStmt.m_iSetValue );
		} else if ( tStmt.m_sSetName=="log_debug_filter" )
		{
			int iLen = tStmt.m_sSetValue.Length();
			iLen = Min ( iLen, SPH_MAX_FILENAME_LEN );
			memcpy ( g_sLogFilter, tStmt.m_sSetValue.cstr(), iLen );
			g_sLogFilter[iLen] = '\0';
			g_iLogFilterLen = iLen;
		} else if ( tStmt.m_sSetName=="net_wait" )
		{
			g_tmWait = (int)tStmt.m_iSetValue;
		} else if ( tStmt.m_sSetName=="grouping_in_utc")
		{
			g_bGroupingInUtc = !!tStmt.m_iSetValue;
			setGroupingInUtc ( g_bGroupingInUtc );
		} else if ( tStmt.m_sSetName=="cpustats")
		{
			g_bCpuStats = !!tStmt.m_iSetValue;
		} else if ( tStmt.m_sSetName=="iostats")
		{
			g_bIOStats = !!tStmt.m_iSetValue;
		} else if ( tStmt.m_sSetName=="coredump")
		{
			g_bCoreDump = !!tStmt.m_iSetValue;
		} else if ( tStmt.m_sSetName=="maintenance")
		{
			if ( myinfo::IsVIP() )
				g_bMaintenance = !!tStmt.m_iSetValue;
			else
			{
				tOut.Error ( tStmt.m_sStmt, "Only VIP connections can set maintenance mode" );
				return;
			}
		} else if ( tStmt.m_sSetName=="wait_timeout" )
		{
			if ( myinfo::IsVIP() )
				g_iClientQlTimeoutS = tStmt.m_iSetValue;
			else
			{
				tOut.Error ( tStmt.m_sStmt, "Only VIP connections can change global wait_timeout value" );
				return;
			}
		} else if ( tStmt.m_sSetName=="throttling_period" )
		{
			if ( myinfo::IsVIP() )
				Threads::CoThrottler_c::SetDefaultThrottlingPeriodMS ( tStmt.m_iSetValue );
			else
			{
				tOut.Error ( tStmt.m_sStmt, "Only VIP connections can change global throttling_period value" );
				return;
			}
		} else
		{
			tOut.ErrorEx ( tStmt.m_sStmt, "Unknown system variable '%s'", tStmt.m_sSetName.cstr () );
			return;
		}
		break;

	case SET_INDEX_UVAR:
		if ( !SendUserVar ( tStmt.m_sIndex.cstr(), tStmt.m_sSetName.cstr(), tStmt.m_dSetValues, sError ) )
		{
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
		break;

	case SET_CLUSTER_UVAR:
	{
		if ( !ReplicateSetOption ( tStmt.m_sIndex, tStmt.m_sSetName, tStmt.m_sSetValue, sError ) )
		{
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
	}
	break;

	default:
		tOut.ErrorEx ( tStmt.m_sStmt, "internal error: unhandled SET mode %d", (int) tStmt.m_eSet );
		return;
	}

	// it went ok
	tOut.Ok();
}


void HandleMysqlAttach ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	const CSphString & sFrom = tStmt.m_sIndex;
	const CSphString & sTo = tStmt.m_sStringParam;
	bool bTruncate = ( tStmt.m_iIntParam==1 );
	CSphString sError;

	auto pServedFrom = GetServed ( sFrom );
	auto pServedTo = GetServed ( sTo );

	// need just read lock for initial checks and prevent deadlock of attaching index to itself
	{
		ServedDescRPtr_c pFrom ( pServedFrom );
		ServedDescRPtr_c pTo ( pServedTo ) ;

		if ( !pFrom
			|| !pTo
			|| pFrom->m_eType!=IndexType_e::PLAIN
			|| pTo->m_eType!=IndexType_e::RT )
		{
			if ( !pFrom )
				tOut.ErrorEx ( nullptr, "no such index '%s'", sFrom.cstr() );
			else if ( !pTo )
				tOut.ErrorEx ( nullptr, "no such index '%s'", sTo.cstr() );
			else if ( pFrom->m_eType!=IndexType_e::PLAIN )
				tOut.Error ( tStmt.m_sStmt, "1st argument to ATTACH must be a plain index" );
			else if ( pTo->m_eType!=IndexType_e::RT )
				tOut.Error ( tStmt.m_sStmt, "2nd argument to ATTACH must be a RT index" );
			return;
		}

		// cluster does not implement ATTACH for now
		if ( ClusterOperationProhibit ( pTo, sError, "ATTACH" ) )
		{
			tOut.ErrorEx ( nullptr, "index %s: %s", sTo.cstr(), sError.cstr () );
			return;
		}
	}

	ServedDescWPtr_c pFrom ( pServedFrom ); // write-lock
	ServedDescWPtr_c pTo ( pServedTo ) ; // write-lock

	auto * pRtTo = ( RtIndex_i * ) pTo->m_pIndex;

	bool bFatal = false;
	if ( !pRtTo->AttachDiskIndex ( pFrom->m_pIndex, bTruncate, bFatal, sError ) )
	{
		if ( bFatal )
			g_pLocalIndexes->Delete(sFrom);

		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	// after a successfull Attach() RT index owns it
	pFrom->m_pIndex = nullptr;
	g_pLocalIndexes->Delete ( sFrom );
	tOut.Ok();
}


void HandleMysqlFlushRtindex ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	ServedDescRPtr_c pIndex ( GetServed ( tStmt.m_sIndex ) );

	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tStmt.m_sStmt, "FLUSH RTINDEX requires an existing RT index" );
		return;
	}

	auto * pRt = ( RtIndex_i * ) pIndex->m_pIndex;
	pRt->ForceRamFlush();
	tOut.Ok();
}


void HandleMysqlFlushRamchunk ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	ServedDescRPtr_c pIndex ( GetServed ( tStmt.m_sIndex ) );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tStmt.m_sStmt, "FLUSH RAMCHUNK requires an existing RT index" );
		return;
	}

	auto * pRt = (RtIndex_i*)pIndex->m_pIndex;
	if ( !pRt->ForceDiskChunk() )
	{
		CSphString sError;
		sError.SetSprintf ( "index '%s': FLUSH RAMCHUNK failed; INDEX UNUSABLE (%s)", tStmt.m_sIndex.cstr(), pRt->GetLastError().cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		g_pLocalIndexes->Delete ( tStmt.m_sIndex );
		return;
	}

	tOut.Ok();
}


void HandleMysqlFlush ( RowBuffer_i & tOut, const SqlStmt_t & )
{
	int iTag = CommandFlush();
	tOut.HeadBegin(1);
	tOut.HeadColumn ( "tag", MYSQL_COL_LONG );
	tOut.HeadEnd();

	// data packet, var value
	tOut.PutNumAsString ( iTag );
	tOut.Commit();

	// done
	tOut.Eof();
}

inline static CSphString strSHA1 ( const CSphString& sLine )
{
	return CalcSHA1 ( sLine.cstr(), sLine.Length() );
}

int GetLogFD ()
{
	if ( g_bLogStdout && g_iLogFile!=STDOUT_FILENO )
		return STDOUT_FILENO;
	return g_iLogFile;
}

void HandleMysqlOptimizeManual ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	auto sIndex = tStmt.m_sStringParam;
	ServedDescRPtr_c pIndex ( GetServed ( sIndex ) );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tStmt.m_sStmt, "MERGE requires an existing RT index" );
		return;
	}

	tOut.Ok ();
	auto iFrom = tStmt.m_iListStart;
	auto iTo = tStmt.m_iListEnd;

	if ( tStmt.m_tQuery.m_bSync )
	{
		if ( pIndex->m_pIndex )
			static_cast<RtIndex_i *>( pIndex->m_pIndex )->Optimize ( iFrom, iTo );

		return;
	}

	EnqueueForOptimize ( sIndex, iFrom, iTo );
}

void HandleMysqlDebug ( RowBuffer_i &tOut, const SqlStmt_t &tStmt )
{
	CSphString sCommand = tStmt.m_sIndex;
	CSphString sParam = tStmt.m_sStringParam;
	sCommand.ToLower ();
	sCommand.Trim ();
	sParam.Trim ();
	bool bVipConn = myinfo::IsVIP();

	if ( bVipConn && ( sCommand=="shutdown" || sCommand=="crash" ) )
	{
		if ( g_sShutdownToken.IsEmpty () )
		{

			tOut.HeadTuplet ("command","error");
			tOut.DataTuplet ("debug shutdown", "shutdown_token is empty. Provide it in searchd config section.");
		}
		else {
			if ( strSHA1(sParam) == g_sShutdownToken )
			{
				tOut.HeadTuplet ( "command", "result" );
				if ( sCommand=="shutdown" )
				{
					tOut.DataTuplet ( "debug shutdown <password>", "SUCCESS" );
#if USE_WINDOWS
					sigterm(1);
#else
					kill(0, SIGTERM);
#endif
				} else if ( sCommand=="crash")
				{
					tOut.DataTuplet ( "debug crash <password>", "SUCCESS" );
					BYTE * pSegv = ( BYTE * ) ( 0 );
					*pSegv = 'a';
				}
			} else
				{
					tOut.HeadTuplet ( "command", "result" );
					tOut.DataTuplet ( "debug shutdown <password>", "FAIL" );
			}
			// perform challenge here...
		}
	} else if ( bVipConn && sCommand=="token" )
	{
		auto sSha = strSHA1(sParam);
		tOut.HeadTuplet ( "command", "result" );
		tOut.DataTuplet ( "debug token", sSha.cstr() );
	}
#if HAVE_MALLOC_STATS
	else if ( sCommand=="malloc_stats" )
	{
		tOut.HeadTuplet ( "command", "result" );
		// check where is stderr...
		int iOldErr = ::dup ( STDERR_FILENO );
		::dup2 ( GetLogFD (), STDERR_FILENO );
		malloc_stats();
		::dup2 ( iOldErr, STDERR_FILENO );
		::close ( iOldErr );
		tOut.DataTuplet ( "malloc_stats", g_sLogFile.cstr());
	}
#endif
#if HAVE_MALLOC_TRIM
	else if ( sCommand=="malloc_trim" )
	{
		tOut.HeadTuplet ( "command", "result" );
		CSphString sResult;
		sResult.SetSprintf ( "%d", PerformMallocTrim (0));
		tOut.DataTuplet ( "malloc_trim", sResult.cstr () );
	}
#endif
#if !USE_WINDOWS
	else if ( bVipConn && sCommand=="procdump" )
	{
		tOut.HeadTuplet ( "command", "result" );
		if ( g_iParentPID<=0 )
			tOut.DataTuplet ( "procdump", "Unavailable (no watchdog)" );
		else
		{
			kill ( g_iParentPID, SIGUSR1 );
			tOut.DataTupletf ( "procdump", "Sent USR1 to wathcdog (%d)", g_iParentPID );
		}
	} else if ( bVipConn && sCommand=="setgdb" )
	{
		tOut.HeadTuplet ( "command", "result" );
		const auto& g_bSafeGDB = getSafeGDB ();
		if ( sParam=="status" )
		{
			if ( g_iParentPID>0 )
				tOut.DataTupletf ( "setgdb", "Enabled, managed by watchdog (pid=%d)", g_iParentPID );
			else if ( g_bSafeGDB )
				tOut.DataTupletf ( "setgdb", "Enabled, managed locally because of jemalloc", g_iParentPID );
			else if ( g_iParentPID==-1 )
				tOut.DataTuplet ( "setgdb", "Enabled locally, MAY HANG!" );
			else
				tOut.DataTuplet ( "setgdb", "Disabled" );
		} else
		{
			if ( g_iParentPID>0 )
				tOut.DataTupletf ( "setgdb", "Enabled by watchdog (pid=%d)", g_iParentPID );
			else if ( g_bSafeGDB )
				tOut.DataTuplet ( "setgdb", "Enabled locally because of jemalloc" );
			else if ( sParam=="on" )
			{
				g_iParentPID = -1;
				tOut.DataTuplet ( "setgdb", "Ok, enabled locally, MAY HANG!" );
			} else if ( sParam=="off" )
			{
				g_iParentPID = 0;
				tOut.DataTuplet ( "setgdb", "Ok, disabled" );
			}
		}
	}
#endif
	else if ( sCommand == "sleep" )
	{
		int64_t tmStart = sphMicroTimer();
		sphSleepMsec ( Max ( tStmt.m_iIntParam, 1 )*1000 );
		int64_t tmDelta = sphMicroTimer() - tmStart;

		tOut.HeadTuplet ( "command", "result" );
		CSphString sResult;
		sResult.SetSprintf ( "%.3f", (float)tmDelta/1000000.0f );
		tOut.DataTuplet ( "sleep", sResult.cstr () );
	}
	else if ( sCommand=="tasks" )
	{
		const char* dHeader[] = { "Name", "MaxRunners", "MaxQueue", "CurrentRunners", "TotalSpent",
			"LastFinished", "Executed", "Dropped", "In Queue" };
		tOut.HeadOfStrings ( &dHeader[0], sizeof ( dHeader ) / sizeof ( dHeader[0] ));

		auto dTasks = TaskManager::GetTaskInfo ();
		for ( const auto& dTask : dTasks )
		{
			tOut.PutString ( dTask.m_sName );
			if ( dTask.m_iMaxRunners>0 )
				tOut.PutNumAsString ( dTask.m_iMaxRunners );
			else
				tOut.PutString ( "Scheduling" );
			switch ( dTask.m_iMaxQueueSize )
			{
				case -1 : tOut.PutString ( "Unlimited" ); break;
				case 0 : tOut.PutString ( "Disabled" ); break;
				default : tOut.PutNumAsString ( dTask.m_iMaxQueueSize );
			}
			tOut.PutNumAsString ( dTask.m_iCurrentRunners );
			tOut.PutTimeAsString ( dTask.m_iTotalSpent );
			tOut.PutTimestampAsString ( dTask.m_iLastFinished );
			tOut.PutNumAsString ( dTask.m_iTotalRun );
			tOut.PutNumAsString ( dTask.m_iTotalDropped );
			tOut.PutNumAsString ( dTask.m_inQueue );
			tOut.Commit ();
		}
	}
	else if ( sCommand=="systhreads" )
	{
		const char* dHeader[] = {
			"ID", "ThID", "Run time", "Load time", "Total ticks", "Jobs done", "Last job take", "Idle for" };
		tOut.HeadOfStrings ( &dHeader[0], sizeof ( dHeader ) / sizeof ( dHeader[0] ));
		auto dTasks = TaskManager::GetThreadsInfo ();
		for ( auto& dTask : dTasks )
		{
			tOut.PutNumAsString ( dTask.m_iMyThreadID );
			tOut.PutNumAsString ( dTask.m_iMyOSThreadID );
			tOut.PutTimestampAsString ( dTask.m_iMyStartTimestamp );
			tOut.PutTimeAsString ( dTask.m_iTotalWorkedTime );
			tOut.PutNumAsString ( dTask.m_iTotalTicked );
			tOut.PutNumAsString ( dTask.m_iTotalJobsDone );
			if ( dTask.m_iLastJobDoneTime<0 )
				tOut.PutString ( "In work" );
			else
				tOut.PutTimeAsString ( dTask.m_iLastJobDoneTime - dTask.m_iLastJobStartTime );
			if ( dTask.m_iLastJobDoneTime<0 )
				tOut.PutString ( "0 (working)" );
			else
				tOut.PutTimestampAsString ( dTask.m_iLastJobDoneTime );
			tOut.Commit ();
		}

	}
	else if ( sCommand=="merge" )
	{
		HandleMysqlOptimizeManual ( tOut, tStmt );
		return;
	}
	else if ( sCommand=="sched" )
	{
		const char* dHeader[] = {
			"Time rest", "Task"
		};
		tOut.HeadOfStrings ( &dHeader[0], sizeof ( dHeader ) / sizeof ( dHeader[0] ));
		auto dTasks = TaskManager::GetSchedInfo ();
		for ( auto& dTask : dTasks )
		{
			tOut.PutTimestampAsString ( dTask.m_iTimeoutStamp );
			tOut.PutString ( dTask.m_sTask );
			tOut.Commit ();
		}
	} else
	{
		// no known command; provide short help.
		tOut.HeadTuplet ( "command", "meaning" );
		if ( bVipConn )
		{
			tOut.DataTuplet ( "debug shutdown <password>", "emulate TERM signal");
			tOut.DataTuplet ( "debug crash <password>", "crash daemon (make SIGSEGV action)" );
			tOut.DataTuplet ( "debug token <password>", "calculate token for password" );
#if !USE_WINDOWS
			tOut.DataTuplet ( "debug procdump", "ask watchdog to dump us" );
			tOut.DataTuplet ( "debug setgdb on|off|status", "enable or disable potentially dangerous crash dumping with gdb" );
#endif
		}
		tOut.DataTuplet ( "flush logs", "emulate USR1 signal" );
		tOut.DataTuplet ( "reload indexes", "emulate HUP signal" );
#if HAVE_MALLOC_STATS
		tOut.DataTuplet ( "malloc_stats", "perform 'malloc_stats', result in searchd.log" );
#endif
#if HAVE_MALLOC_TRIM
		tOut.DataTuplet ( "malloc_trim", "pefrorm 'malloc_trim' call" );
#endif
		tOut.DataTuplet ( "sleep Nsec", "sleep for N seconds" );
		tOut.DataTuplet ( "tasks", "display global tasks stat" );
		tOut.DataTuplet ( "systhreads", "display task manager threads" );
		tOut.DataTuplet ( "sched", "display task manager schedule" );
		tOut.DataTuplet ( "merge <IDX> N1 N2", "For RT index IDX merge disk chunk N1 into disk chunk N2" );
	}
	// done
	tOut.Eof ();
}

// fwd
static bool PrepareReconfigure ( const CSphString & sIndex, CSphReconfigureSettings & tSettings, CSphString & sError );

void HandleMysqlTruncate ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	bool bReconfigure = ( tStmt.m_iIntParam==1 );

	CSphScopedPtr<ReplicationCommand_t> pCmd ( new ReplicationCommand_t() );
	CSphString sError;
	const CSphString & sIndex = tStmt.m_sIndex;

	if ( bReconfigure )
		pCmd->m_tReconfigure = new CSphReconfigureSettings();

	if ( bReconfigure && !PrepareReconfigure ( sIndex, *pCmd->m_tReconfigure.Ptr(), sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	// get an exclusive lock for operation
	// but only read lock for check
	{
		ServedDescRPtr_c pIndex ( GetServed ( sIndex ) );
		if ( !ServedDesc_t::IsMutable ( pIndex ) )
		{
			tOut.Error ( tStmt.m_sStmt, "TRUNCATE RTINDEX requires an existing RT index" );
			return;
		}

		if ( !CheckIndexCluster ( sIndex, *pIndex, tStmt.m_sCluster, IsHttpStmt ( tStmt ), sError ) )
		{
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
	}

	pCmd->m_eCommand = ReplicationCommand_e::TRUNCATE;
	pCmd->m_sIndex = sIndex;
	pCmd->m_sCluster = tStmt.m_sCluster;
	pCmd->m_bReconfigure = bReconfigure;

	RtAccum_t tAcc ( false );
	tAcc.m_dCmd.Add ( pCmd.LeakPtr() );

	bool bRes = HandleCmdReplicate ( tAcc, sError );
	if ( !bRes )
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
	else
		tOut.Ok();
}


void HandleMysqlOptimize ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	ServedDescRPtr_c pIndex ( GetServed ( tStmt.m_sIndex ) );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tStmt.m_sStmt, "OPTIMIZE INDEX requires an existing RT index" );
		return;
	}

	tOut.Ok();

	if ( tStmt.m_tQuery.m_bSync )
	{
		if ( pIndex->m_pIndex )
			static_cast<RtIndex_i *>( pIndex->m_pIndex )->Optimize(-1,-1);

		return;
	}

	EnqueueForOptimize ( tStmt.m_sIndex );
}

void HandleMysqlSelectSysvar ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, const SessionVars_t & tVars )
{
	const struct SysVar_t
	{
		const MysqlColumnType_e m_eType;
		const char * m_sName;
		std::function<CSphString ( void )> m_fnValue;
	} dSysvars[] =
	{	{ MYSQL_COL_STRING,	nullptr, [] {return "";}}, // stub
		{ MYSQL_COL_LONG,	"@@session.auto_increment_increment",	[] {return "1";}},
		{ MYSQL_COL_STRING,	"@@character_set_client", [] {return "utf8";}},
		{ MYSQL_COL_STRING,	"@@character_set_connection", [] {return "utf8";}},
		{ MYSQL_COL_LONG,	"@@max_allowed_packet", [] { StringBuilder_c s; s << g_iMaxPacketSize; return s; }},
		{ MYSQL_COL_STRING,	"@@version_comment", [] { return szGIT_BRANCH_ID;}},
		{ MYSQL_COL_LONG,	"@@lower_case_table_names", [] { return "1"; }},
		{ MYSQL_COL_STRING,	"@@session.last_insert_id", [&tVars]
			{
				StringBuilder_c s ( "," );
				tVars.m_dLastIds.Apply ( [&s] ( int64_t iID ) { s << iID; } );
				return s;
			}},
	};

	auto fnVar = [&dSysvars] ( const CSphString & sVar )->const SysVar_t& {
		for ( const auto& dVar : dSysvars )
			if ( sVar==dVar.m_sName )
				return dVar;
		return dSysvars[0];
	};

	// fill header
	tOut.HeadBegin ( tStmt.m_tQuery.m_dItems.GetLength () );
	for ( const auto& dItem : tStmt.m_tQuery.m_dItems )
		tOut.HeadColumn ( dItem.m_sAlias.cstr (), fnVar ( dItem.m_sExpr ).m_eType );
	tOut.HeadEnd ();

	// fill values
	for ( const auto & dItem : tStmt.m_tQuery.m_dItems )
		tOut.PutString ( fnVar ( dItem.m_sExpr ).m_fnValue().cstr() );

	// finalize
	tOut.Commit ();
	tOut.Eof ();
}

struct ExtraLastInsertID_t : public ISphExtra
{
	explicit ExtraLastInsertID_t ( const CSphVector<int64_t> & dIds )
		: m_dIds ( dIds )
	{
	}

	virtual ~ExtraLastInsertID_t () = default;

	virtual bool ExtraDataImpl ( ExtraData_e eCmd, void ** pData )
	{
		if ( eCmd==EXTRA_GET_LAST_INSERT_ID )
		{
			StringBuilder_c tBuf ( "," );
			for ( int64_t iID : m_dIds )
				tBuf.Appendf ( INT64_FMT, iID );

			CSphString * sVal = ( CSphString *)pData;
			assert ( sVal );
			*sVal = tBuf.cstr();

			return true;
		}
		return false;
	}

	const CSphVector<int64_t> & m_dIds;
};



void HandleMysqlSelectDual ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, const SessionVars_t & tVars )
{
	CSphString sVar = tStmt.m_tQuery.m_sQuery;
	CSphSchema	tSchema;
	ESphAttr eAttrType;
	CSphString sError;
	ExprParseArgs_t tExprArgs;
	tExprArgs.m_pAttrType = &eAttrType;

	CSphRefcountedPtr<ISphExpr> pExpr { sphExprParse ( sVar.cstr(), tSchema, sError, tExprArgs ) };

	if ( !pExpr )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	tOut.HeadBegin(1);
	tOut.HeadColumn ( sVar.cstr(), MYSQL_COL_STRING );
	tOut.HeadEnd();

	ExtraLastInsertID_t tIds ( tVars.m_dLastIds );
	pExpr->Command ( SPH_EXPR_SET_EXTRA_DATA, &tIds );

	CSphMatch tMatch;
	const BYTE * pStr = nullptr;

	switch ( eAttrType )
	{
		case SPH_ATTR_STRINGPTR:
		{
			int  iLen = pExpr->StringEval ( tMatch, &pStr );
			tOut.PutArray ( pStr, iLen );
			if ( pExpr->IsDataPtrAttr() )
				SafeDeleteArray ( pStr );
			break;
		}
		case SPH_ATTR_INTEGER: tOut.PutNumAsString ( pExpr->IntEval ( tMatch ) ); break;
		case SPH_ATTR_BIGINT: tOut.PutNumAsString ( pExpr->Int64Eval ( tMatch ) ); break;
		case SPH_ATTR_FLOAT:	tOut.PutFloatAsString ( pExpr->Eval ( tMatch ) ); break;
		default:
			tOut.PutNULL();
			break;
	}

	// done
	tOut.Commit();
	tOut.Eof();
}


void HandleMysqlShowCollations ( RowBuffer_i & tOut )
{
	// MySQL Connector/J really expects an answer here
	// field packets
	tOut.HeadBegin(6);
	tOut.HeadColumn ( "Collation" );
	tOut.HeadColumn ( "Charset" );
	tOut.HeadColumn ( "Id", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "Default" );
	tOut.HeadColumn ( "Compiled" );
	tOut.HeadColumn ( "Sortlen" );
	tOut.HeadEnd();

	// data packets
	tOut.PutString ( "utf8_general_ci" );
	tOut.PutString ( "utf8" );
	tOut.PutString ( "33" );
	tOut.PutString ( "Yes" );
	tOut.PutString ( "Yes" );
	tOut.PutString ( "1" );
	tOut.Commit();

	// done
	tOut.Eof();
	return;
}

void HandleMysqlShowCharacterSet ( RowBuffer_i & tOut )
{
	// MySQL Connector/J really expects an answer here
	// field packets
	tOut.HeadBegin(4);
	tOut.HeadColumn ( "Charset" );
	tOut.HeadColumn ( "Description" );
	tOut.HeadColumn ( "Default collation" );
	tOut.HeadColumn ( "Maxlen" );
	tOut.HeadEnd();

	// data packets
	tOut.PutString ( "utf8" );
	tOut.PutString ( "UTF-8 Unicode" );
	tOut.PutString ( "utf8_general_ci" );
	tOut.PutString ( "3" );
	tOut.Commit();

	// done
	tOut.Eof();
}

const char * sphCollationToName ( ESphCollation eColl )
{
	switch ( eColl )
	{
		case SPH_COLLATION_LIBC_CI:				return "libc_ci";
		case SPH_COLLATION_LIBC_CS:				return "libc_cs";
		case SPH_COLLATION_UTF8_GENERAL_CI:		return "utf8_general_ci";
		case SPH_COLLATION_BINARY:				return "binary";
		default:								return "unknown";
	}
}


static const char * LogLevelName ( ESphLogLevel eLevel )
{
	switch ( eLevel )
	{
		case SPH_LOG_FATAL:					return "fatal";
		case SPH_LOG_WARNING:				return "warning";
		case SPH_LOG_INFO:					return "info";
		case SPH_LOG_DEBUG:					return "debug";
		case SPH_LOG_VERBOSE_DEBUG:			return "debugv";
		case SPH_LOG_VERY_VERBOSE_DEBUG:	return "debugvv";
		default:							return "unknown";
	}
}


void HandleMysqlShowVariables ( RowBuffer_i & dRows, const SqlStmt_t & tStmt, SessionVars_t & tVars )
{
	dRows.HeadTuplet ( "Variable_name", "Value" );

	TableLike dMatchRows ( dRows, tStmt.m_sStringParam.cstr () );
	dMatchRows.DataTuplet ("autocommit", tVars.m_bAutoCommit ? "1" : "0" );
	dMatchRows.DataTuplet ( "collation_connection", sphCollationToName ( tVars.m_eCollation ) );
	dMatchRows.DataTuplet ( "query_log_format", g_eLogFormat==LOG_FORMAT_PLAIN ? "plain" : "sphinxql" );
	dMatchRows.DataTuplet ( "log_level", LogLevelName ( g_eLogLevel ) );
	dMatchRows.DataTuplet ( "max_allowed_packet", g_iMaxPacketSize );
	dMatchRows.DataTuplet ( "character_set_client", "utf8" );
	dMatchRows.DataTuplet ( "character_set_connection", "utf8" );
	dMatchRows.DataTuplet ( "grouping_in_utc", g_bGroupingInUtc ? "1" : "0" );
	if ( dMatchRows.Match ( "last_insert_id" ) )
	{
		StringBuilder_c tBuf ( "," );
		for ( int64_t iID : tVars.m_dLastIds )
			tBuf.Appendf ( INT64_FMT, iID );
		dRows .DataTuplet ( "last_insert_id", tBuf.cstr() );
	}

	// fine
	dRows.Eof();
}

template <typename FORMATFN>
static void AddQueryStats ( VectorLike & dStatus, const char * szPrefix, const QueryStats_t & tStats,
	//void (*FormatFn)( StringBuilder_c & sBuf, uint64_t uQueries, uint64_t uStat, const char * sType ) )
	FORMATFN FormatFn )

{
	using namespace QueryStats;
	static const char * dStatIntervalNames[INTERVAL_TOTAL] =
	{
		"1min",
		"5min",
		"15min",
		"total"
	};

	static const char * dStatTypeNames[TYPE_TOTAL] =
	{
		"avg",
		"min",
		"max",
		"pct95",
		"pct99"
	};

	StringBuilder_c sBuf;
	StringBuilder_c sName;
	for ( int i = 0; i < INTERVAL_TOTAL; ++i )
	{
		sBuf.Clear();
		{
			ScopedComma_c VARIABLE_IS_NOT_USED tRootBlock( sBuf, ", ", R"({"queries":)", "}" );
			sBuf << tStats.m_dStats[i].m_uTotalQueries;
			for ( int j = 0; j < TYPE_TOTAL; ++j )
				FormatFn ( sBuf, tStats.m_dStats[i].m_uTotalQueries,
						tStats.m_dStats[i].m_dData[j], dStatTypeNames[j] );
		}

		sName.Clear();
		sName << szPrefix << "_" << dStatIntervalNames[i];

		if ( dStatus.MatchAdd ( sName.cstr() ) )
			dStatus.Add ( sBuf.cstr() );
	}
}


static void AddQueryTimeStatsToOutput ( VectorLike & dStatus, const char * szPrefix, const QueryStats_t & tQueryTimeStats )
{
	AddQueryStats ( dStatus, szPrefix, tQueryTimeStats,
		[]( StringBuilder_c & sBuf, uint64_t uQueries, uint64_t uStat, const char * sType )
		{
			uQueries ? sBuf.Sprintf( R"("%s_sec":%.3F)", sType, uStat ) : sBuf.AppendName( sType ) << R"("-")";
		} );
}


static void AddFoundRowsStatsToOutput ( VectorLike & dStatus, const char * szPrefix, const QueryStats_t & tRowsFoundStats )
{
	AddQueryStats ( dStatus, szPrefix, tRowsFoundStats,
		[]( StringBuilder_c & sBuf, uint64_t uQueries, uint64_t uStat, const char * sType )
		{
			sBuf.AppendName( sType );
			uQueries ? sBuf << uStat : sBuf << R"("-")";
		} );
}


static void AddIndexQueryStats ( VectorLike & dStatus, const ServedStats_c * pStats )
{
	assert ( pStats );

	QueryStats_t tQueryTimeStats, tRowsFoundStats;
	pStats->CalculateQueryStats ( tRowsFoundStats, tQueryTimeStats );
	AddQueryTimeStatsToOutput ( dStatus, "query_time", tQueryTimeStats );

#ifndef NDEBUG
	QueryStats_t tExactQueryTimeStats, tExactRowsFoundStats;
	pStats->CalculateQueryStatsExact ( tExactQueryTimeStats, tExactRowsFoundStats );
	AddQueryTimeStatsToOutput ( dStatus, "exact_query_time", tQueryTimeStats );
#endif

	AddFoundRowsStatsToOutput ( dStatus, "found_rows", tRowsFoundStats );
}

static void AddFederatedIndexStatus ( const CSphSourceStats & tStats, const CSphString & sName, RowBuffer_i & tOut )
{
	const char * dHeader[] = { "Name", "Engine", "Version", "Row_format", "Rows", "Avg_row_length", "Data_length", "Max_data_length", "Index_length", "Data_free",
		"Auto_increment", "Create_time", "Update_time", "Check_time", "Collation", "Checksum", "Create_options", "Comment" };
	tOut.HeadOfStrings ( &dHeader[0], sizeof(dHeader)/sizeof(dHeader[0]) );

	tOut.PutString ( sName );	// Name
	tOut.PutString ( "InnoDB" );		// Engine
	tOut.PutString ( "10" );			// Version
	tOut.PutString ( "Dynamic" );		// Row_format
	tOut.PutNumAsString ( tStats.m_iTotalDocuments );	// Rows
	tOut.PutString ( "4096" );			// Avg_row_length
	tOut.PutString ( "0" );				// Data_length
	tOut.PutString ( "0" );				// Max_data_length
	tOut.PutString ( "0" );				// Index_length
	tOut.PutString ( "0" );				// Data_free
	tOut.PutString ( "5" );				// Auto_increment
	tOut.PutNULL();						// Create_time
	tOut.PutNULL();						// Update_time
	tOut.PutNULL();						// Check_time
	tOut.PutString ( "utf8" );			// Collation
	tOut.PutNULL();						// Checksum
	tOut.PutString ( "" );				// Create_options
	tOut.PutString ( "" );				// Comment

	tOut.Commit();
}

static void AddMatchedColumns ( const VectorLike & dStatus, RowBuffer_i & tOut )
{
	// send rows
	for ( int iRow=0; iRow<dStatus.GetLength(); iRow+=2 )
		tOut.DataTuplet ( dStatus[iRow+0].cstr(), dStatus[iRow+1].cstr() );
}

static void AddDiskIndexStatus ( VectorLike & dStatus, const CSphIndex * pIndex, bool bMutable )
{
	if ( dStatus.MatchAdd ( "indexed_documents" ) )
		dStatus.Add().SetSprintf ( INT64_FMT, pIndex->GetStats().m_iTotalDocuments );
	if ( dStatus.MatchAdd ( "indexed_bytes" ) )
		dStatus.Add().SetSprintf ( INT64_FMT, pIndex->GetStats().m_iTotalBytes );

	const int64_t * pFieldLens = pIndex->GetFieldLens();
	if ( pFieldLens )
	{
		int64_t iTotalTokens = 0;
		for ( int i=0; i < pIndex->GetMatchSchema().GetFieldsCount(); i++ )
		{
			if ( dStatus.MatchAddVa ( "field_tokens_%s", pIndex->GetMatchSchema().GetFieldName(i) ) )
				dStatus.Add().SetSprintf ( INT64_FMT, pFieldLens[i] );
			iTotalTokens += pFieldLens[i];
		}
		if ( dStatus.MatchAdd ( "total_tokens" ) )
			dStatus.Add().SetSprintf ( INT64_FMT, iTotalTokens );
	}

	CSphIndexStatus tStatus;
	pIndex->GetStatus ( &tStatus );
	if ( dStatus.MatchAdd ( "ram_bytes" ) )
		dStatus.Add().SetSprintf ( INT64_FMT, tStatus.m_iRamUse );
	if ( dStatus.MatchAdd ( "disk_bytes" ) )
		dStatus.Add().SetSprintf ( INT64_FMT, tStatus.m_iDiskUse );
	if ( bMutable )
	{
		if ( dStatus.MatchAdd ( "ram_chunk" ) )
			dStatus.Add().SetSprintf ( INT64_FMT, tStatus.m_iRamChunkSize );
		if ( dStatus.MatchAdd ( "ram_chunk_segments_count" ) )
			dStatus.Add().SetSprintf ( "%d", tStatus.m_iNumRamChunks );
		if ( dStatus.MatchAdd ( "disk_chunks" ) )
			dStatus.Add().SetSprintf ( "%d", tStatus.m_iNumChunks );
		if ( dStatus.MatchAdd ( "mem_limit" ) )
			dStatus.Add().SetSprintf ( INT64_FMT, tStatus.m_iMemLimit );
		if ( dStatus.MatchAdd ( "ram_bytes_retired" ) )
			dStatus.Add().SetSprintf ( INT64_FMT, tStatus.m_iRamRetired );
		if ( dStatus.MatchAdd ( "tid" ) )
			dStatus.Add().SetSprintf ( INT64_FMT, tStatus.m_iTID );
		if ( dStatus.MatchAdd ( "tid_saved" ) )
			dStatus.Add().SetSprintf ( INT64_FMT, tStatus.m_iSavedTID );
	}
}

static void AddPlainIndexStatus ( RowBuffer_i & tOut, const ServedDesc_t * pLocked, const ServedStats_c * pStats, bool bModeFederated, const CSphString & sName, const CSphString & sPattern )

{
	assert ( pLocked );
	CSphIndex * pIndex = pLocked->m_pIndex;
	assert ( pIndex );

	if ( !bModeFederated )
	{
		VectorLike dStatus ( sPattern );

		if ( dStatus.MatchAdd ( "index_type" ) )
		{
			switch ( pLocked->m_eType )
			{
			case IndexType_e::PLAIN: dStatus.Add ( "disk" ); break;
			case IndexType_e::RT: dStatus.Add ( "rt" ); break;
			case IndexType_e::PERCOLATE: dStatus.Add ( "percolate" ); break;
			case IndexType_e::TEMPLATE: dStatus.Add ( "template" ); break;
			case IndexType_e::DISTR: dStatus.Add ( "distributed" ); break;
			default:
				break;
			}
		}

		AddDiskIndexStatus ( dStatus, pIndex, ServedDesc_t::IsMutable ( pLocked ) );
		AddIndexQueryStats ( dStatus, pStats );

		tOut.HeadTuplet ( "Variable_name", "Value" );
		AddMatchedColumns ( dStatus, tOut );

	} else
	{
		const CSphSourceStats & tStats = pIndex->GetStats();
		AddFederatedIndexStatus ( tStats, sName, tOut );
	}

	tOut.Eof();
}


static void AddDistibutedIndexStatus ( RowBuffer_i & tOut, DistributedIndex_t * pIndex, bool bFederatedUser, const CSphString & sName, const CSphString & sPattern )
{
	assert ( pIndex );

	if ( !bFederatedUser )
	{
		VectorLike dStatus ( sPattern );

		if ( dStatus.MatchAdd ( "index_type" ) )
			dStatus.Add ( "distributed" );

		AddIndexQueryStats ( dStatus, pIndex );

		tOut.HeadTuplet ( "Variable_name", "Value" );
		AddMatchedColumns ( dStatus, tOut );
	} else
	{
		CSphSourceStats tStats;
		tStats.m_iTotalDocuments = 1000; // TODO: check is it worth to query that number from agents
		AddFederatedIndexStatus ( tStats, sName, tOut );
	}

	tOut.Eof();
}


void HandleMysqlShowIndexStatus ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, bool bFederatedUser )
{
	CSphString sError;
	ServedDescRPtr_c pServed ( GetServed ( tStmt.m_sIndex ));

	int iChunk = tStmt.m_iIntParam;

	if ( pServed )
	{
		CSphIndex * pIndex = pServed ? pServed->m_pIndex : nullptr;

		if ( iChunk>=0 && pServed && pIndex && pIndex->IsRT ())
			pIndex = static_cast<RtIndex_i *>( pIndex )->GetDiskChunk ( iChunk );

		if ( !pServed || !pIndex ) {
			tOut.Error ( tStmt.m_sStmt, "SHOW INDEX STATUS requires an existing index" );
			return;
		}

		if ( iChunk>=0 ) {
			VectorLike dStatus ( tStmt.m_sStringParam );
			AddDiskIndexStatus ( dStatus, pIndex, false );
			tOut.HeadTuplet ( "Variable_name", "Value" );
			AddMatchedColumns ( dStatus, tOut );
			tOut.Eof ();
		} else
			AddPlainIndexStatus ( tOut, pServed, (const ServedStats_c *) pServed, bFederatedUser, tStmt.m_sIndex, tStmt.m_sStringParam );
		return;
	}

	auto pIndex = GetDistr ( tStmt.m_sIndex );

	if ( pIndex )
		AddDistibutedIndexStatus ( tOut, pIndex, bFederatedUser, tStmt.m_sIndex, tStmt.m_sStringParam );
	else
		tOut.Error ( tStmt.m_sStmt, "SHOW INDEX STATUS requires an existing index" );
}


void HandleMysqlShowIndexSettings ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	ServedDescRPtr_c pServed ( GetServed ( tStmt.m_sIndex ) );

	int iChunk = tStmt.m_iIntParam;
	CSphIndex * pIndex = pServed ? pServed->m_pIndex : nullptr;

	if ( iChunk>=0 && pServed && pIndex && pIndex->IsRT() )
		pIndex = static_cast<RtIndex_i *>( pIndex )->GetDiskChunk ( iChunk );

	if ( !pServed || !pIndex )
	{
		tOut.Error ( tStmt.m_sStmt, "SHOW INDEX SETTINGS requires an existing index" );
		return;
	}

	tOut.HeadTuplet ( "Variable_name", "Value" );

	StringBuilder_c tBuf;
	CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( CreateFilenameBuilder ( pIndex->GetName() ) );
	DumpSettings ( tBuf, *pIndex, pFilenameBuilder.Ptr() );

	tOut.DataTuplet ( "settings", tBuf.cstr() );

	tOut.Eof();
}


void HandleMysqlShowProfile ( RowBuffer_i & tOut, const CSphQueryProfile & p, bool bMoreResultsFollow )
{
	#define SPH_QUERY_STATE(_name,_desc) _desc,
	static const char * dStates [ SPH_QSTATE_TOTAL ] = { SPH_QUERY_STATES };
	#undef SPH_QUERY_STATES

	tOut.HeadBegin ( 4 );
	tOut.HeadColumn ( "Status" );
	tOut.HeadColumn ( "Duration" );
	tOut.HeadColumn ( "Switches" );
	tOut.HeadColumn ( "Percent" );
	tOut.HeadEnd ( bMoreResultsFollow );

	int64_t tmTotal = 0;
	int iCount = 0;
	for ( int i=0; i<SPH_QSTATE_TOTAL; i++ )
	{
		if ( p.m_dSwitches[i]<=0 )
			continue;
		tmTotal += p.m_tmTotal[i];
		iCount += p.m_dSwitches[i];
	}

	char sTime[32];
	for ( int i=0; i<SPH_QSTATE_TOTAL; i++ )
	{
		if ( p.m_dSwitches[i]<=0 )
			continue;
		snprintf ( sTime, sizeof(sTime), "%d.%06d", int(p.m_tmTotal[i]/1000000), int(p.m_tmTotal[i]%1000000) );
		tOut.PutString ( dStates[i] );
		tOut.PutString ( sTime );
		tOut.PutNumAsString ( p.m_dSwitches[i] );
		if ( tmTotal )
			tOut.PutFloatAsString ( 100.0f * p.m_tmTotal[i]/tmTotal, "%.2f" );
		else
			tOut.PutString ( "INF" );
		tOut.Commit();
	}
	snprintf ( sTime, sizeof(sTime), "%d.%06d", int(tmTotal/1000000), int(tmTotal%1000000) );
	tOut.PutString ( "total" );
	tOut.PutString ( sTime );
	tOut.PutNumAsString ( iCount );
	tOut.PutString ( "0" );
	tOut.Commit();
	tOut.Eof ( bMoreResultsFollow );
}


static void AddAttrToIndex ( const SqlStmt_t & tStmt, const ServedDesc_t * pServed, CSphString & sError )
{
	CSphString sAttrToAdd = tStmt.m_sAlterAttr;
	sAttrToAdd.ToLower();

	if ( pServed->m_pIndex->GetMatchSchema().GetAttr ( sAttrToAdd.cstr() ) )
	{
		sError.SetSprintf ( "'%s' attribute already in schema", sAttrToAdd.cstr() );
		return;
	}

	if ( tStmt.m_eAlterColType!=SPH_ATTR_STRING && pServed->m_pIndex->GetMatchSchema().GetFieldIndex ( sAttrToAdd.cstr () )!=-1 )
	{
		sError.SetSprintf ( "cannot add attribute that shadows '%s' field", sAttrToAdd.cstr () );
		return;
	}

	pServed->m_pIndex->AddRemoveAttribute ( true, sAttrToAdd, tStmt.m_eAlterColType, sError );
}


static void RemoveAttrFromIndex ( const SqlStmt_t & tStmt, const ServedDesc_t * pServed, CSphString & sError )
{
	CSphString sAttrToRemove = tStmt.m_sAlterAttr;
	sAttrToRemove.ToLower();

	const CSphColumnInfo * pAttr = pServed->m_pIndex->GetMatchSchema().GetAttr ( sAttrToRemove.cstr() );
	if ( !pAttr )
	{
		sError.SetSprintf ( "attribute '%s' does not exist", sAttrToRemove.cstr() );
		return;
	}

	if ( sAttrToRemove==sphGetDocidName() || sphIsInternalAttr ( *pAttr ) )
	{
		sError.SetSprintf ( "unable to remove built-in attribute '%s'", sAttrToRemove.cstr() );
		return;
	}

	if ( pServed->m_pIndex->GetMatchSchema().GetAttrsCount()==1 )
	{
		sError.SetSprintf ( "unable to remove last attribute '%s'", sAttrToRemove.cstr() );
		return;
	}

	pServed->m_pIndex->AddRemoveAttribute ( false, sAttrToRemove, pAttr->m_eAttrType, sError );
}


static void HandleMysqlAlter ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, bool bAdd )
{
	MEMORY ( MEM_SQL_ALTER );

	SearchFailuresLog_c dErrors;
	CSphString sError;

	if ( bAdd && tStmt.m_eAlterColType==SPH_ATTR_NONE )
	{
		sError.SetSprintf ( "unsupported attribute type '%d'", tStmt.m_eAlterColType );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	StrVec_t dNames;
	ParseIndexList ( tStmt.m_sIndex, dNames );
	if ( dNames.IsEmpty() )
	{
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	for ( const auto & sName : dNames )
		if ( !g_pLocalIndexes->Contains ( sName )
			&& g_pDistIndexes->Contains ( sName ) )
		{
			sError.SetSprintf ( "ALTER is only supported for local (not distributed) indexes" );
			tOut.Error ( tStmt.m_sStmt, sError.cstr () );
			return;
		}

	for ( const auto &sName : dNames )
	{
		auto pLocal = GetServed ( sName );
		if ( !pLocal )
		{
			dErrors.Submit ( sName, nullptr, "unknown local index in ALTER request" );
			continue;
		}

		ServedDescWPtr_c pWriteLocked ( pLocal ); // write-lock

		// cluster does not implement ALTER for now
		if ( ClusterOperationProhibit ( pWriteLocked, sError, "ALTER" ) )
		{
			dErrors.Submit ( sName, nullptr, sError.cstr() );
			continue;
		}

		CSphString sAddError;

		if ( bAdd )
			AddAttrToIndex ( tStmt, pWriteLocked, sAddError );
		else
			RemoveAttrFromIndex ( tStmt, pWriteLocked, sAddError );

		if ( !sAddError.IsEmpty() )
			dErrors.Submit ( sName, nullptr, sAddError.cstr() );
	}

	if ( !dErrors.IsEmpty() )
	{
		StringBuilder_c sReport;
		dErrors.BuildReport ( sReport );
		tOut.Error ( tStmt.m_sStmt, sReport.cstr() );
		return;
	}
	tOut.Ok();
}


static bool PrepareReconfigure ( const CSphString & sIndex, const CSphConfigSection & hIndex, CSphReconfigureSettings & tSettings, CSphString & sWarning, CSphString & sError )
{
	CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( CreateFilenameBuilder ( sIndex.cstr() ) );

	// fixme: report warnings
	tSettings.m_tTokenizer.Setup ( hIndex, sWarning );
	tSettings.m_tDict.Setup ( hIndex, pFilenameBuilder.Ptr(), sWarning );
	tSettings.m_tFieldFilter.Setup ( hIndex, sWarning );
	tSettings.m_iMemLimit = hIndex.GetSize64 ( "rt_mem_limit", DEFAULT_RT_MEM_LIMIT );

	sphRTSchemaConfigure ( hIndex, tSettings.m_tSchema, sError, true );

	if ( !tSettings.m_tIndex.Setup ( hIndex, sIndex.cstr(), sWarning, sError ) )
	{
		sError.SetSprintf ( "'%s' failed to parse index settings, error '%s'", sIndex.cstr(), sError.cstr() );
		return false;
	}

	if ( !CheckStoredFields ( tSettings.m_tSchema, tSettings.m_tIndex, sError ) )
		return false;

	return true;
}


static bool PrepareReconfigure ( const CSphString & sIndex, CSphReconfigureSettings & tSettings, CSphString & sError )
{
	CSphConfigParser tCfg;
	if ( !tCfg.ReParse ( g_sConfigFile.cstr () ) )
	{
		sError.SetSprintf ( "failed to parse config file '%s': %s; using previous settings", g_sConfigFile.cstr (), TlsMsg::szError() );
		return false;
	}

	if ( !tCfg.m_tConf.Exists ( "index" ) )
	{
		sError.SetSprintf ( "failed to find any index in config file '%s'; using previous settings", g_sConfigFile.cstr () );
		return false;
	}

	const CSphConfig & hConf = tCfg.m_tConf;
	if ( !hConf["index"].Exists ( sIndex ) )
	{
		sError.SetSprintf ( "failed to find index '%s' in config file '%s'; using previous settings", sIndex.cstr(), g_sConfigFile.cstr () );
		return false;
	}

	CSphString sWarning;
	if ( !PrepareReconfigure ( sIndex, hConf["index"][sIndex], tSettings, sWarning, sError ) )
		return false;

	return true;
}


static void HandleMysqlReconfigure ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	MEMORY ( MEM_SQL_ALTER );

	if ( IsConfigless() )
	{
		tOut.Error ( tStmt.m_sStmt, "ALTER RECONFIGURE is not supported in RT mode" );
		return;
	}

	const CSphString & sIndex = tStmt.m_sIndex.cstr();
	CSphString sError;
	CSphReconfigureSettings tSettings;
	CSphReconfigureSetup tSetup;

	if ( !PrepareReconfigure ( sIndex, tSettings, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( !pServed )
	{
		if ( g_pDistIndexes->Contains ( tStmt.m_sIndex ) )
			sError.SetSprintf ( "ALTER is only supported for local (not distributed) indexes" );
		else
			sError.SetSprintf ( "index '%s' not found", tStmt.m_sIndex.cstr () );

		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	ServedDescWPtr_c dWLocked ( pServed );
	if ( !ServedDesc_t::IsMutable ( dWLocked ))
	{
		sError.SetSprintf ( "'%s' does not support ALTER (enabled, not mutable)", sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	StrVec_t dWarnings;
	bool bSame = ( (const RtIndex_i *) dWLocked->m_pIndex )->IsSameSettings ( tSettings, tSetup, dWarnings, sError );
	sWarning = ConcatWarnings(dWarnings);

	if ( !bSame && sError.IsEmpty() )
	{
		auto pRT = (RtIndex_i *) dWLocked->m_pIndex;
		bool bOk = pRT->Reconfigure(tSetup);
		if ( !bOk )
		{
			sError.SetSprintf ( "index '%s': reconfigure failed; INDEX UNUSABLE (%s)", tStmt.m_sIndex.cstr(), pRT->GetLastError().cstr() );
			g_pLocalIndexes->Delete ( tStmt.m_sIndex );
		}
	}

	if ( sError.IsEmpty() )
		tOut.Ok ( 0, dWarnings.GetLength() );
	else
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
}


static bool ApplyIndexKillList ( CSphIndex * pIndex, CSphString & sWarning, CSphString & sError, bool bShowMessage = false );

static void HandleMysqlAlterKlist ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	MEMORY ( MEM_SQL_ALTER );

	CSphString sError;

	KillListTargets_c tNewTargets;
	if ( !tNewTargets.Parse ( tStmt.m_sAlterOption, tStmt.m_sIndex.cstr(), sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	auto pServed = GetServed ( tStmt.m_sIndex.cstr () );
	ServedDescWPtr_c pWriteLocked ( pServed );

	if ( !pServed )
	{
		if ( g_pDistIndexes->Contains ( tStmt.m_sIndex ) )
			sError.SetSprintf ( "ALTER is only supported for local (not distributed) indexes" );
		else
			sError.SetSprintf ( "index '%s' not found", tStmt.m_sIndex.cstr () );
	}
	else if ( ServedDesc_t::IsMutable ( pWriteLocked ) )
		sError.SetSprintf ( "'%s' does not support ALTER (real-time or percolate)", tStmt.m_sIndex.cstr () );

	if ( !sError.IsEmpty () )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	if ( !pWriteLocked->m_pIndex->AlterKillListTarget ( tNewTargets, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// apply killlist to new targets
	if ( !ApplyIndexKillList ( pWriteLocked->m_pIndex, sWarning, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( sError.IsEmpty() )
		tOut.Ok();
	else
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
}


static bool SubstituteExternalIndexFiles ( const StrVec_t & dOldExternalFiles, const StrVec_t & dNewExternalFiles, CSphString & sIndexPath, StrVec_t & dBackupFiles, CSphString & sError )
{
	StrVec_t dOnlyNew;
	for ( const auto & i : dNewExternalFiles )
	{
		bool bDupe = false;
		for ( const auto & j : dOldExternalFiles )
			bDupe |= i==j;

		if ( !bDupe )
			dOnlyNew.Add(i);
	}

	StrVec_t dOnlyOld;
	for ( const auto & i : dOldExternalFiles )
	{
		bool bDupe = false;
		for ( const auto & j : dNewExternalFiles )
			bDupe |= i==j;

		if ( !bDupe )
			dOnlyOld.Add(i);
	}

	for ( const auto & i : dOnlyOld )
		dBackupFiles.Add().SetSprintf ( "%s.tmp", i.cstr() );

	if ( !RenameWithRollback ( dOnlyOld, dBackupFiles, sError ) )
		return false;

	if ( !dOnlyNew.GetLength() )
		return true;

	StrVec_t dCopied;
	if ( !CopyExternalIndexFiles ( dOnlyNew, sIndexPath, dCopied, sError ) )
	{
		// try to rename files back
		CSphString sTmp;
		RenameFiles ( dBackupFiles, dOnlyOld, sTmp );
		return false;
	}

	return true;
}


static void HandleMysqlAlterIndexSettings ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	MEMORY ( MEM_SQL_ALTER );

	CSphString sError;
	if ( !IsConfigless() )
	{
		sError = "ALTER TABLE requires data_dir to be set in the config file";
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	auto pServed = GetServed ( tStmt.m_sIndex.cstr() );
	if ( !pServed )
	{
		if ( g_pDistIndexes->Contains ( tStmt.m_sIndex ) )
			sError.SetSprintf ( "ALTER is only supported for local (not distributed) indexes" );
		else
			sError.SetSprintf ( "index '%s' not found", tStmt.m_sIndex.cstr () );

		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	ServedDescWPtr_c pWriteLocked ( pServed );
	if ( !pWriteLocked->m_pIndex->IsRT() )
	{
		sError.SetSprintf ( "index '%s' is not real-time", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	RtIndex_i * pRtIndex = (RtIndex_i*)pWriteLocked->m_pIndex;

	// get all table settings as a string
	CSphString sCreateTable = BuildCreateTable ( pWriteLocked->m_pIndex->GetName(), pWriteLocked->m_pIndex, pRtIndex->GetInternalSchema() );

	CSphVector<SqlStmt_t> dCreateTableStmts;
	if ( !ParseDdl ( sCreateTable.cstr(), sCreateTable.Length(), dCreateTableStmts, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( dCreateTableStmts.GetLength()!=1 )
	{
		tOut.Error ( tStmt.m_sStmt, "Unable to alter index settings" );
		return;
	}

	// parse the options string to old-style config hash
	IndexSettingsContainer_c tContainer;
	tContainer.Populate ( dCreateTableStmts[0].m_tCreateTable );

	StrVec_t dOldExternalFiles = tContainer.GetFiles();

	// force override for old options
	for ( const auto & i : tStmt.m_tCreateTable.m_dOpts )
		tContainer.RemoveKeys ( i.m_sName );

	for ( const auto & i : tStmt.m_tCreateTable.m_dOpts )
	{
		if ( !i.m_sValue.IsEmpty() )
			tContainer.AddOption ( i.m_sName, i.m_sValue );
	}

	if ( !tContainer.CheckPaths() )
	{
		tOut.Error ( tStmt.m_sStmt, tContainer.GetError().cstr() );
		return;
	}

	StrVec_t dBackupFiles;
	CSphString sIndexPath = GetPathOnly ( pRtIndex->GetFilename() );
	if ( !SubstituteExternalIndexFiles ( dOldExternalFiles, tContainer.GetFiles(), sIndexPath, dBackupFiles, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	StrVec_t dWarnings;
	CSphReconfigureSettings tSettings;
	if ( !PrepareReconfigure ( tStmt.m_sIndex, tContainer.AsCfg(), tSettings, sWarning, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	if ( !sWarning.IsEmpty() )
		dWarnings.Add(sWarning);

	CSphReconfigureSetup tSetup;
	bool bSame = pRtIndex->IsSameSettings ( tSettings, tSetup, dWarnings, sError );
	sWarning = ConcatWarnings(dWarnings);

	if ( !bSame && sError.IsEmpty() )
	{
		bool bOk = pRtIndex->Reconfigure(tSetup);
		if ( !bOk )
		{
			sError.SetSprintf ( "index '%s': alter failed; INDEX UNUSABLE (%s)", tStmt.m_sIndex.cstr(), pRtIndex->GetLastError().cstr() );
			g_pLocalIndexes->Delete ( tStmt.m_sIndex );
		}
	}

	if ( sError.IsEmpty() )
	{
		// all ok, delete old files
		for ( const auto & i : dBackupFiles )
			::unlink ( i.cstr() );

		tOut.Ok ( 0, dWarnings.GetLength() );
	}
	else
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
}


static void HandleMysqlShowPlan ( RowBuffer_i & tOut, const CSphQueryProfile & p, bool bMoreResultsFollow )
{
	tOut.HeadBegin ( 2 );
	tOut.HeadColumn ( "Variable" );
	tOut.HeadColumn ( "Value" );
	tOut.HeadEnd ( bMoreResultsFollow );

	tOut.PutString ( "transformed_tree" );
	tOut.PutString ( p.GetResultAsStr() );
	tOut.Commit();

	tOut.Eof ( bMoreResultsFollow );
}

static bool RotateIndexMT ( ServedIndex_c* pIndex, const CSphString & sIndex, StrVec_t & dWarnings, CSphString & sError );
static bool RotateIndexGreedy ( ServedDesc_t & tServed, const char * sIndex, CSphString & sError );
static void HandleMysqlReloadIndex ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	CSphString sError;
	auto pIndex = GetServed ( tStmt.m_sIndex );
	if ( !pIndex )
	{
		sError.SetSprintf ( "unknown local index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	{
		ServedDescRPtr_c pServed ( pIndex );
		if ( ServedDesc_t::IsMutable ( pServed ) )
		{
			sError.SetSprintf ( "can not reload real-time or percolate index" );
			tOut.Error ( tStmt.m_sStmt, sError.cstr () );
			return;
		}

		if ( !tStmt.m_sStringParam.IsEmpty () )
		{
			// try move files from arbitrary path to current index path before rotate, if needed.
			// fixme! what about concurrency? if 2 sessions simultaneously ask to rotate,
			// or if we have unapplied rotates from indexer - seems that it will garbage .new files?
			IndexFiles_c sIndexFiles ( pServed->m_sIndexPath );
			if ( !sIndexFiles.RelocateToNew ( tStmt.m_sStringParam.cstr () ) )
			{
				tOut.Error ( tStmt.m_sStmt, sIndexFiles.ErrorMsg () );
				return;
			}
		}
	}

	StrVec_t dWarnings;
	if ( g_bSeamlessRotate )
	{
		if ( !RotateIndexMT ( pIndex, tStmt.m_sIndex, dWarnings, sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
	} else
	{
		ServedDescWPtr_c pServedWL( pIndex );
		if ( !RotateIndexGreedy ( *pServedWL, tStmt.m_sIndex.cstr(), sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			g_pLocalIndexes->Delete ( tStmt.m_sIndex ); // since it unusable - no sense just to disable it.
			// fixme! RotateIndexGreedy does prealloc. Do we need to perform/signal preload also?
			return;
		}
	}

	if ( dWarnings.GetLength() )
	{
		StringBuilder_c sWarn ( "; " );
		for ( const auto & i : dWarnings )
			sWarn << i;

		sWarning = sWarn.cstr();
	}

	tOut.Ok();
}

void HandleMysqlExplain ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	CSphString sProc ( tStmt.m_sCallProc );
	CSphString sError;
	if ( sProc.ToLower()!="query" )
	{
		sError.SetSprintf ( "no such explain procedure %s", tStmt.m_sCallProc.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	auto pIndex = GetServed ( tStmt.m_sIndex );
	if ( !pIndex )
	{
		sError.SetSprintf ( "unknown local index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	CSphString sRes;
	ServedDescRPtr_c pServed ( pIndex );

	if ( !pServed->m_pIndex->ExplainQuery ( tStmt.m_tQuery.m_sQuery, sRes, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	tOut.HeadBegin ( 2 );
	tOut.HeadColumn ( "Variable" );
	tOut.HeadColumn ( "Value" );
	tOut.HeadEnd ();

	tOut.PutString ( "transformed_tree" );
	tOut.PutString ( sRes );
	tOut.Commit();
	tOut.Eof ();
}


void HandleMysqlImportTable ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	CSphString sError;

	if ( !IsConfigless() )
	{
		sError = "IMPORT TABLE requires data_dir to be set in the config file";
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	auto pLocal = GetServed ( tStmt.m_sIndex );
	auto pDist = GetDistr ( tStmt.m_sIndex );
	if ( pLocal || pDist )
	{
		sError.SetSprintf ( "index '%s' already exists", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	bool bPQ = false;
	if ( !CopyIndexFiles ( tStmt.m_sIndex, tStmt.m_sStringParam, bPQ, sError ) )
	{
		sError.SetSprintf ( "unable to import index '%s': %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	StrVec_t dWarnings;
	if ( !AddExistingIndexInt ( tStmt.m_sIndex, bPQ ? IndexType_e::PERCOLATE : IndexType_e::RT, dWarnings, sError ) )
	{
		sError.SetSprintf ( "unable to import index '%s': %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( dWarnings.GetLength() )
	{
		StringBuilder_c sWarn ( "; " );
		for ( const auto & i : dWarnings )
			sWarn << i;

		sWarning = sWarn.cstr();
	}

	tOut.Ok();
}

//////////////////////////////////////////////////////////////////////////

CSphSessionAccum::~CSphSessionAccum()
{
	SafeDelete ( m_pAcc );
}

RtAccum_t * CSphSessionAccum::GetAcc ( RtIndex_i * pIndex, CSphString & sError )
{
	assert ( pIndex );
	RtAccum_t * pAcc = pIndex->CreateAccum ( m_pAcc, sError );
	if ( !pAcc && !sError.IsEmpty() )
		return pAcc;

	if ( pAcc!=m_pAcc )
		SafeDelete ( m_pAcc );
	m_pAcc = pAcc;
	return pAcc;
}

RtIndex_i * CSphSessionAccum::GetIndex ()
{
	if ( m_pAcc )
		return m_pAcc->GetIndex();
	else
		return sphGetCurrentIndexRT();
}

static bool FixupFederatedQuery ( ESphCollation eCollation, CSphVector<SqlStmt_t> & dStmt, CSphString & sError, CSphString & sFederatedQuery );

class CSphinxqlSession
{
private:
	CSphString			m_sError;
	CSphQueryResultMeta m_tLastMeta;
	CSphSessionAccum	m_tAcc;
	CPqResult			m_tPercolateMeta;
	SqlStmt_e			m_eLastStmt { STMT_DUMMY };
	bool				m_bFederatedUser = false;
	CSphString			m_sFederatedQuery;

public:
	SessionVars_t			m_tVars;
	CSphQueryProfileMysql	m_tProfile;
	CSphQueryProfileMysql	m_tLastProfile;

public:
	// just execute one sphinxql statement
	//
	// IMPORTANT! this does NOT start or stop profiling, as there a few external
	// things (client net reads and writes) that we want to profile, too
	//
	// returns true if the current profile should be kept (default)
	// returns false if profile should be discarded (eg. SHOW PROFILE case)
	bool Execute ( const CSphString & sQuery, RowBuffer_i & tOut )
	{
		// set on query guard
		myinfo::ThdState ( ThdState_e::QUERY );
		auto iQuerySize = sQuery.Length ();
		auto& tCrashQuery = GlobalCrashQueryGetRef();
		tCrashQuery.m_pQuery = (const BYTE *)sQuery.cstr();
		tCrashQuery.m_iSize = iQuerySize;
		tCrashQuery.m_bMySQL = true;

		if (sQuery=="select DATABASE(), USER() limit 1")
		{
			// result set header packet
			tOut.HeadTuplet ( "DATABASE()", "USER()" );
			tOut.DataTuplet ( g_sDbName.cstr(), myinfo::IsVIP() ? "VIP" : "Usual" );
			tOut.Eof ( false );
			return true;
		}

		// parse SQL query
		if ( m_tVars.m_bProfile )
			m_tProfile.Switch ( SPH_QSTATE_SQL_PARSE );

		CSphVector<SqlStmt_t> dStmt;
		bool bParsedOK = sphParseSqlQuery ( sQuery.cstr(), iQuerySize, dStmt, m_sError, m_tVars.m_eCollation );

		if ( m_tVars.m_bProfile )
			m_tProfile.Switch ( SPH_QSTATE_UNKNOWN );

		SqlStmt_e eStmt = STMT_PARSE_ERROR;
		if ( bParsedOK )
		{
			eStmt = dStmt[0].m_eStmt;
			dStmt[0].m_sStmt = sQuery.cstr();
		}
		const SqlStmt_e ePrevStmt = m_eLastStmt;
		if ( eStmt!=STMT_SHOW_META )
			m_eLastStmt = eStmt;

		SqlStmt_t * pStmt = dStmt.Begin();
		assert ( !bParsedOK || pStmt );

		myinfo::SetCommand ( g_dSqlStmts[eStmt] );

		if ( bParsedOK && m_bFederatedUser )
		{
			if ( !FixupFederatedQuery ( m_tVars.m_eCollation, dStmt,  m_sError, m_sFederatedQuery ) )
			{
				m_tLastMeta = CSphQueryResultMeta();
				m_tLastMeta.m_sError = m_sError;
				m_tLastMeta.m_sWarning = "";
				tOut.Error ( sQuery.cstr(), m_sError.cstr() );
				return true;
			}
		}

		// handle multi SQL query
		if ( bParsedOK && dStmt.GetLength()>1 )
		{
			m_sError = "";
			HandleMysqlMultiStmt ( dStmt, m_tLastMeta, tOut, m_sError );
			return true; // FIXME? how does this work with profiling?
		}

		// handle SQL query
		switch ( eStmt )
		{
		case STMT_PARSE_ERROR:
			m_tLastMeta = CSphQueryResultMeta();
			m_tLastMeta.m_sError = m_sError;
			m_tLastMeta.m_sWarning = "";
			tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			return true;

		case STMT_SELECT:
			{
				MEMORY ( MEM_SQL_SELECT );

				StatCountCommand ( SEARCHD_COMMAND_SEARCH );
				SearchHandler_c tHandler ( 1, sphCreatePlainQueryParser(), QUERY_SQL, true );
				tHandler.SetQuery ( 0, dStmt.Begin()->m_tQuery, dStmt.Begin()->m_pTableFunc );
				dStmt.Begin()->m_pTableFunc = nullptr;

				if ( m_tVars.m_bProfile )
					tHandler.SetProfile ( &m_tProfile );
				if ( m_bFederatedUser )
					tHandler.SetFederatedUser();

				if ( HandleMysqlSelect ( tOut, tHandler ) )
				{
					// query just completed ok; reset out error message
					m_sError = "";
					AggrResult_t & tLast = tHandler.m_dResults.Last();
					SendMysqlSelectResult ( tOut, tLast, false, m_bFederatedUser, &m_sFederatedQuery, ( m_tVars.m_bProfile ? &m_tProfile : nullptr ) );
				}

				// save meta for SHOW META (profile is saved elsewhere)
				m_tLastMeta = tHandler.m_dResults.Last();
				return true;
			}
		case STMT_SHOW_WARNINGS:
			HandleMysqlWarning ( m_tLastMeta, tOut, false );
			return true;

		case STMT_SHOW_STATUS:
		case STMT_SHOW_META:
		case STMT_SHOW_AGENT_STATUS:
			if ( eStmt==STMT_SHOW_STATUS )
			{
				StatCountCommand ( SEARCHD_COMMAND_STATUS );
			}
			if ( ePrevStmt!=STMT_CALL )
				HandleMysqlMeta ( tOut, *pStmt, m_tLastMeta, false );
			else
				HandleMysqlPercolateMeta ( m_tPercolateMeta, m_tLastMeta.m_sWarning, tOut );
			return true;

		case STMT_INSERT:
		case STMT_REPLACE:
			{
				m_tLastMeta = CSphQueryResultMeta();
				m_tLastMeta.m_sError = m_sError;
				m_tLastMeta.m_sWarning = "";
				StatCountCommand ( eStmt==STMT_INSERT ? SEARCHD_COMMAND_INSERT : SEARCHD_COMMAND_REPLACE );
				StmtErrorReporter_c tErrorReporter ( tOut );
				sphHandleMysqlInsert ( tErrorReporter, *pStmt, eStmt==STMT_REPLACE,
					m_tVars.m_bAutoCommit && !m_tVars.m_bInTransaction, m_tLastMeta.m_sWarning, m_tAcc,
					m_tVars.m_eCollation, m_tVars.m_dLastIds );
				return true;
			}

		case STMT_DELETE:
			{
				m_tLastMeta = CSphQueryResultMeta();
				m_tLastMeta.m_sError = m_sError;
				m_tLastMeta.m_sWarning = "";
				StatCountCommand ( SEARCHD_COMMAND_DELETE );
				StmtErrorReporter_c tErrorReporter ( tOut );
				sphHandleMysqlDelete ( tErrorReporter, *pStmt, sQuery, m_tVars.m_bAutoCommit && !m_tVars.m_bInTransaction, m_tAcc );
				return true;
			}

		case STMT_SET:
			StatCountCommand ( SEARCHD_COMMAND_UVAR );
			HandleMysqlSet ( tOut, *pStmt, m_tVars, m_tAcc );
			return false;

		case STMT_BEGIN:
			{
				MEMORY ( MEM_SQL_BEGIN );

				m_tVars.m_bInTransaction = true;
				RtIndex_i * pIndex = m_tAcc.GetIndex();
				if ( pIndex )
				{
					RtAccum_t * pAccum = m_tAcc.GetAcc ( pIndex, m_sError );
					if ( !m_sError.IsEmpty() )
					{
						tOut.Error ( sQuery.cstr(), m_sError.cstr() );
						return true;
					}
					HandleCmdReplicate ( *pAccum, m_sError );
				}
				tOut.Ok();
				return true;
			}
		case STMT_COMMIT:
		case STMT_ROLLBACK:
			{
				MEMORY ( MEM_SQL_COMMIT );

				m_tVars.m_bInTransaction = false;
				RtIndex_i * pIndex = m_tAcc.GetIndex();
				if ( pIndex )
				{
					RtAccum_t * pAccum = m_tAcc.GetAcc ( pIndex, m_sError );
					if ( !m_sError.IsEmpty() )
					{
						tOut.Error ( sQuery.cstr(), m_sError.cstr() );
						return true;
					}
					if ( eStmt==STMT_COMMIT )
					{
						StatCountCommand ( SEARCHD_COMMAND_COMMIT );
						HandleCmdReplicate ( *pAccum, m_sError );
					} else
					{
						pIndex->RollBack ( pAccum );
					}
				}
				tOut.Ok();
				return true;
			}
		case STMT_CALL:
			// IMPORTANT! if you add a new builtin here, do also add it
			// in the comment to STMT_CALL line in SqlStmt_e declaration,
			// the one that lists expansions for doc/check.pl
			pStmt->m_sCallProc.ToUpper();
			if ( pStmt->m_sCallProc=="SNIPPETS" )
			{
				StatCountCommand ( SEARCHD_COMMAND_EXCERPT );
				HandleMysqlCallSnippets ( tOut, *pStmt );
			} else if ( pStmt->m_sCallProc=="KEYWORDS" )
			{
				StatCountCommand ( SEARCHD_COMMAND_KEYWORDS );
				HandleMysqlCallKeywords ( tOut, *pStmt, m_tLastMeta.m_sWarning );
			} else if ( pStmt->m_sCallProc=="SUGGEST" )
			{
				StatCountCommand ( SEARCHD_COMMAND_SUGGEST );
				HandleMysqlCallSuggest ( tOut, *pStmt, false );
			} else if ( pStmt->m_sCallProc=="QSUGGEST" )
			{
				StatCountCommand ( SEARCHD_COMMAND_SUGGEST );
				HandleMysqlCallSuggest ( tOut, *pStmt, true );
			} else if ( pStmt->m_sCallProc=="PQ" )
			{
				StatCountCommand ( SEARCHD_COMMAND_CALLPQ );
				HandleMysqlCallPQ ( tOut, *pStmt, m_tAcc, m_tPercolateMeta );
				m_tPercolateMeta.m_dResult.m_sMessages.MoveWarningsTo ( m_tLastMeta.m_sWarning );
				m_tPercolateMeta.m_dDocids.Reset ( 0 ); // free occupied mem
			} else
			{
				m_sError.SetSprintf ( "no such built-in procedure %s", pStmt->m_sCallProc.cstr() );
				tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			}
			return true;

		case STMT_DESCRIBE:
			HandleMysqlDescribe ( tOut, *pStmt );
			return true;

		case STMT_SHOW_TABLES:
			HandleMysqlShowTables ( tOut, *pStmt );
			return true;

		case STMT_CREATE_TABLE:
			m_tLastMeta = CSphQueryResultMeta();
			HandleMysqlCreateTable ( tOut, *pStmt, m_tLastMeta.m_sWarning );
			return true;

		case STMT_CREATE_TABLE_LIKE:
			m_tLastMeta = CSphQueryResultMeta();
			HandleMysqlCreateTableLike ( tOut, *pStmt, m_tLastMeta.m_sWarning );
			return true;

		case STMT_DROP_TABLE:
			HandleMysqlDropTable ( tOut, *pStmt );
			return true;

		case STMT_SHOW_CREATE_TABLE:
			HandleMysqlShowCreateTable ( tOut, *pStmt );
			return true;

		case STMT_UPDATE:
			{
				m_tLastMeta = CSphQueryResultMeta();
				m_tLastMeta.m_sError = m_sError;
				m_tLastMeta.m_sWarning = "";
				StatCountCommand ( SEARCHD_COMMAND_UPDATE );
				StmtErrorReporter_c tErrorReporter ( tOut );
				sphHandleMysqlUpdate ( tErrorReporter, *pStmt, sQuery, m_tLastMeta.m_sWarning );
				return true;
			}

		case STMT_DUMMY:
			tOut.Ok();
			return true;

		case STMT_CREATE_FUNCTION:
			if ( !sphPluginCreate ( pStmt->m_sUdfLib.cstr(), PLUGIN_FUNCTION, pStmt->m_sUdfName.cstr(), pStmt->m_eUdfType, m_sError ) )
				tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			else
				tOut.Ok();
			SphinxqlStateFlush ();
			return true;

		case STMT_DROP_FUNCTION:
			if ( !sphPluginDrop ( PLUGIN_FUNCTION, pStmt->m_sUdfName.cstr(), m_sError ) )
				tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			else
				tOut.Ok();
			SphinxqlStateFlush ();
			return true;

		case STMT_CREATE_PLUGIN:
		case STMT_DROP_PLUGIN:
			{
				// convert plugin type string to enum
				PluginType_e eType = sphPluginGetType ( pStmt->m_sStringParam );
				if ( eType==PLUGIN_TOTAL )
				{
					tOut.Error ( "unknown plugin type '%s'", pStmt->m_sStringParam.cstr() );
					break;
				}

				// action!
				bool bRes;
				if ( eStmt==STMT_CREATE_PLUGIN )
					bRes = sphPluginCreate ( pStmt->m_sUdfLib.cstr(), eType, pStmt->m_sUdfName.cstr(), SPH_ATTR_NONE, m_sError );
				else
					bRes = sphPluginDrop ( eType, pStmt->m_sUdfName.cstr(), m_sError );

				// report
				if ( !bRes )
					tOut.Error ( sQuery.cstr(), m_sError.cstr() );
				else
					tOut.Ok();
				SphinxqlStateFlush ();
				return true;
			}

		case STMT_RELOAD_PLUGINS:
			if ( sphPluginReload ( pStmt->m_sUdfLib.cstr(), m_sError ) )
				tOut.Ok();
			else
				tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			return true;

		case STMT_ATTACH_INDEX:
			HandleMysqlAttach ( tOut, *pStmt );
			return true;

		case STMT_FLUSH_RTINDEX:
			HandleMysqlFlushRtindex ( tOut, *pStmt );
			return true;

		case STMT_FLUSH_RAMCHUNK:
			HandleMysqlFlushRamchunk ( tOut, *pStmt );
			return true;

		case STMT_SHOW_VARIABLES:
			HandleMysqlShowVariables ( tOut, *pStmt, m_tVars );
			return true;

		case STMT_TRUNCATE_RTINDEX:
			HandleMysqlTruncate ( tOut, *pStmt );
			return true;

		case STMT_OPTIMIZE_INDEX:
			HandleMysqlOptimize ( tOut, *pStmt );
			return true;

		case STMT_SELECT_SYSVAR:
			HandleMysqlSelectSysvar ( tOut, *pStmt, m_tVars );
			return true;

		case STMT_SHOW_COLLATION:
			HandleMysqlShowCollations ( tOut );
			return true;

		case STMT_SHOW_CHARACTER_SET:
			HandleMysqlShowCharacterSet ( tOut );
			return true;

		case STMT_SHOW_INDEX_STATUS:
			HandleMysqlShowIndexStatus ( tOut, *pStmt, m_bFederatedUser );
			return true;

		case STMT_SHOW_INDEX_SETTINGS:
			HandleMysqlShowIndexSettings ( tOut, *pStmt );
			return true;

		case STMT_SHOW_PROFILE:
			HandleMysqlShowProfile ( tOut, m_tLastProfile, false );
			return false; // do not profile this call, keep last query profile

		case STMT_ALTER_ADD:
			HandleMysqlAlter ( tOut, *pStmt, true );
			return true;

		case STMT_ALTER_DROP:
			HandleMysqlAlter ( tOut, *pStmt, false );
			return true;

		case STMT_SHOW_PLAN:
			HandleMysqlShowPlan ( tOut, m_tLastProfile, false );
			return false; // do not profile this call, keep last query profile

		case STMT_SELECT_DUAL:
			HandleMysqlSelectDual ( tOut, *pStmt, m_tVars );
			return true;

		case STMT_SHOW_DATABASES:
			HandleMysqlShowDatabases ( tOut, *pStmt );
			return true;

		case STMT_SHOW_PLUGINS:
			HandleMysqlShowPlugins ( tOut, *pStmt );
			return true;

		case STMT_SHOW_THREADS:
			HandleMysqlShowThreads ( tOut, *pStmt );
			return true;

		case STMT_ALTER_RECONFIGURE:
			m_tLastMeta = CSphQueryResultMeta();
			m_tLastMeta.m_sError = m_sError;
			m_tLastMeta.m_sWarning = "";

			HandleMysqlReconfigure ( tOut, *pStmt, m_tLastMeta.m_sWarning );
			return true;

		case STMT_ALTER_KLIST_TARGET:
			m_tLastMeta = CSphQueryResultMeta();
			m_tLastMeta.m_sError = m_sError;
			m_tLastMeta.m_sWarning = "";

			HandleMysqlAlterKlist ( tOut, *pStmt, m_tLastMeta.m_sWarning );
			return true;

		case STMT_ALTER_INDEX_SETTINGS:
			m_tLastMeta = CSphQueryResultMeta();
			m_tLastMeta.m_sError = m_sError;
			m_tLastMeta.m_sWarning = "";

			HandleMysqlAlterIndexSettings ( tOut, *pStmt, m_tLastMeta.m_sWarning );
			return true;

		case STMT_FLUSH_INDEX:
			HandleMysqlFlush ( tOut, *pStmt );
			return true;

		case STMT_RELOAD_INDEX:
			m_tLastMeta = CSphQueryResultMeta();
			m_tLastMeta.m_sError = m_sError;
			m_tLastMeta.m_sWarning = "";

			HandleMysqlReloadIndex ( tOut, *pStmt, m_tLastMeta.m_sWarning );
			return true;

		case STMT_FLUSH_HOSTNAMES:
			HandleMysqlFlushHostnames ( tOut );
			return true;

		case STMT_FLUSH_LOGS:
			HandleMysqlFlushLogs ( tOut );
			return true;

		case STMT_RELOAD_INDEXES:
			HandleMysqlReloadIndexes ( tOut );
			return true;

		case STMT_DEBUG:
			HandleMysqlDebug ( tOut, *pStmt );
			return true;

		case STMT_JOIN_CLUSTER:
			if ( ClusterJoin ( pStmt->m_sIndex, pStmt->m_dCallOptNames, pStmt->m_dCallOptValues, pStmt->m_bClusterUpdateNodes, m_sError ) )
				tOut.Ok();
			else
				tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			return true;
		case STMT_CLUSTER_CREATE:
			if ( ClusterCreate ( pStmt->m_sIndex, pStmt->m_dCallOptNames, pStmt->m_dCallOptValues, m_sError ) )
				tOut.Ok();
			else
				tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			return true;

		case STMT_CLUSTER_DELETE:
			m_tLastMeta = CSphQueryResultMeta();
			if ( ClusterDelete ( pStmt->m_sIndex, m_tLastMeta.m_sError, m_tLastMeta.m_sWarning ) )
				tOut.Ok ( 0, m_tLastMeta.m_sWarning.IsEmpty() ? 0 : 1 );
			else
				tOut.Error ( sQuery.cstr(), m_tLastMeta.m_sError.cstr() );
			return true;

		case STMT_CLUSTER_ALTER_ADD:
		case STMT_CLUSTER_ALTER_DROP:
			m_tLastMeta = CSphQueryResultMeta();
			if ( ClusterAlter ( pStmt->m_sCluster, pStmt->m_sIndex, ( eStmt==STMT_CLUSTER_ALTER_ADD ), m_tLastMeta.m_sError, m_tLastMeta.m_sWarning ) )
				tOut.Ok ( 0, m_tLastMeta.m_sWarning.IsEmpty() ? 0 : 1 );
			else
				tOut.Error ( sQuery.cstr(), m_tLastMeta.m_sError.cstr() );
			return true;

		case STMT_CLUSTER_ALTER_UPDATE:
			m_tLastMeta = CSphQueryResultMeta();
			if ( ClusterAlterUpdate ( pStmt->m_sCluster, pStmt->m_sSetName, m_tLastMeta.m_sError ) )
				tOut.Ok();
			else
				tOut.Error ( sQuery.cstr(), m_tLastMeta.m_sError.cstr() );
			return true;

		case STMT_EXPLAIN:
			HandleMysqlExplain ( tOut, *pStmt );
			return true;

		case STMT_IMPORT_TABLE:
			m_tLastMeta = CSphQueryResultMeta();
			m_tLastMeta.m_sError = m_sError;
			m_tLastMeta.m_sWarning = "";

			HandleMysqlImportTable ( tOut, *pStmt, m_tLastMeta.m_sWarning );
			return true;

		default:
			m_sError.SetSprintf ( "internal error: unhandled statement type (value=%d)", eStmt );
			tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			return true;
		} // switch

		return true; // for cases that break early
	}

	void SetFederatedUser ()
	{
		m_bFederatedUser = true;
	}
};

SphinxqlSessionPublic::SphinxqlSessionPublic()
	: m_pImpl { new CSphinxqlSession }
{}

SphinxqlSessionPublic::~SphinxqlSessionPublic ()
{
	SafeDelete ( m_pImpl );
}

bool SphinxqlSessionPublic::Execute ( const CSphString & sQuery, RowBuffer_i & tOut )
{
	assert ( m_pImpl );
	return m_pImpl->Execute ( sQuery, tOut );
}

void SphinxqlSessionPublic::SetFederatedUser ()
{
	assert ( m_pImpl );
	m_pImpl->SetFederatedUser ();
}

bool SphinxqlSessionPublic::IsAutoCommit () const
{
	assert ( m_pImpl );
	return m_pImpl->m_tVars.m_bAutoCommit;
}

CSphQueryProfile * SphinxqlSessionPublic::StartProfiling ( ESphQueryState eState )
{
	assert ( m_pImpl );
	CSphQueryProfile * pProfile = nullptr;
	if ( m_pImpl->m_tVars.m_bProfile ) // the current statement might change it
	{
		pProfile = &m_pImpl->m_tProfile;
		pProfile->Start ( eState );
	}
	return pProfile;
}

void SphinxqlSessionPublic::SaveLastProfile ()
{
	assert ( m_pImpl );
	m_pImpl->m_tLastProfile = m_pImpl->m_tProfile;
}

int SphinxqlSessionPublic::GetBackendTimeoutS () const
{
	assert ( m_pImpl );
	return m_pImpl->m_tVars.m_iTimeoutS;
}

int SphinxqlSessionPublic::GetBackendThrottlingMS () const
{
	assert ( m_pImpl );
	return m_pImpl->m_tVars.m_iThrottlingMS;
}

/// sphinxql command over API
void HandleCommandSphinxql ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq ) REQUIRES (HandlerThread)
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_SPHINXQL, tOut ) )
		return;

	auto tReply = APIAnswer ( tOut, VER_COMMAND_SPHINXQL );

	// parse and run request
	RunSingleSphinxqlCommand ( tReq.GetString (), tOut );
}

/// json command over API
void HandleCommandJson ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_JSON, tOut ) )
		return;

	// parse request
	CSphString sEndpoint = tReq.GetString ();
	CSphString sCommand = tReq.GetString ();
	
	ESphHttpEndpoint eEndpoint = sphStrToHttpEndpoint ( sEndpoint );

	CSphVector<BYTE> dResult;
	SmallStringHash_T<CSphString> tOptions;
	sphProcessHttpQueryNoResponce ( eEndpoint, sCommand.cstr(), tOptions, dResult );

	auto tReply = APIAnswer ( tOut, VER_COMMAND_JSON );
	tOut.SendString ( sEndpoint.cstr() );
	tOut.SendArray ( dResult );
}


void StatCountCommand ( SearchdCommand_e eCmd )
{
	if ( eCmd<SEARCHD_COMMAND_TOTAL )
		++g_tStats.m_iCommandCount[eCmd];
}

bool FixupFederatedQuery ( ESphCollation eCollation, CSphVector<SqlStmt_t> & dStmt, CSphString & sError, CSphString & sFederatedQuery )
{
	if ( !dStmt.GetLength() )
		return true;

	if ( dStmt.GetLength()>1 )
	{
		sError.SetSprintf ( "multi-query not supported" );
		return false;
	}


	SqlStmt_t & tStmt = dStmt[0];
	if ( tStmt.m_eStmt!=STMT_SELECT && tStmt.m_eStmt!=STMT_SHOW_INDEX_STATUS )
	{
		sError.SetSprintf ( "unhandled statement type (value=%d)", tStmt.m_eStmt );
		return false;
	}
	if ( tStmt.m_eStmt==STMT_SHOW_INDEX_STATUS )
		return true;

	CSphQuery & tSrcQuery = tStmt.m_tQuery;

	// remove query column as it got generated
	ARRAY_FOREACH ( i, tSrcQuery.m_dItems )
	{
		if ( tSrcQuery.m_dItems[i].m_sAlias=="query" )
		{
			tSrcQuery.m_dItems.Remove ( i );
			break;
		}
	}

	// move actual query from filter to query itself
	if ( tSrcQuery.m_dFilters.GetLength()!=1 ||
		tSrcQuery.m_dFilters[0].m_sAttrName!="query" || tSrcQuery.m_dFilters[0].m_eType!=SPH_FILTER_STRING || tSrcQuery.m_dFilters[0].m_dStrings.GetLength()!=1 )
		return true;

	const CSphString & sRealQuery = tSrcQuery.m_dFilters[0].m_dStrings[0];

	// parse real query
	CSphVector<SqlStmt_t> dRealStmt;
	bool bParsedOK = sphParseSqlQuery ( sRealQuery.cstr(), sRealQuery.Length(), dRealStmt, sError, eCollation );
	if ( !bParsedOK )
		return false;

	if ( dRealStmt.GetLength()!=1 )
	{
		sError.SetSprintf ( "multi-query not supported, got queries=%d", dRealStmt.GetLength() );
		return false;
	}

	SqlStmt_t & tRealStmt = dRealStmt[0];
	if ( tRealStmt.m_eStmt!=STMT_SELECT )
	{
		sError.SetSprintf ( "unhandled statement type (value=%d)", tRealStmt.m_eStmt );
		return false;
	}

	// keep originals
	CSphQuery & tRealQuery = tRealStmt.m_tQuery;
	tRealQuery.m_dRefItems = tSrcQuery.m_dItems; //select list items
	tRealQuery.m_sIndexes = tSrcQuery.m_sIndexes; // index name
	sFederatedQuery = sRealQuery;

	// merge select list items
	SmallStringHash_T<int> hItems;
	ARRAY_FOREACH ( i, tRealQuery.m_dItems )
		hItems.Add ( i, tRealQuery.m_dItems[i].m_sAlias );
	ARRAY_FOREACH ( i, tSrcQuery.m_dItems )
	{
		const CSphQueryItem & tItem = tSrcQuery.m_dItems[i];
		if ( !hItems.Exists ( tItem.m_sAlias ) )
			tRealQuery.m_dItems.Add ( tItem );
	}

	// query setup
	tSrcQuery = tRealQuery;
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// INDEX ROTATION
/////////////////////////////////////////////////////////////////////////////
static bool ApplyIndexKillList ( CSphIndex * pIndex, CSphString & sWarning, CSphString & sError, bool bShowMessage )
{
	CSphFixedVector<DocID_t> dKillList(0);
	KillListTargets_c tTargets;
	if ( !pIndex->LoadKillList ( &dKillList, tTargets, sError ) )
		return false;

	if ( !tTargets.m_dTargets.GetLength() )
		return true;

	if ( bShowMessage )
		sphInfo ( "applying killlist of index '%s'", pIndex->GetName() );

	for ( const auto & tIndex : tTargets.m_dTargets )
	{
		// just in case; otherwise we'll be rlocking an already rlocked index
		if ( tIndex.m_sIndex==pIndex->GetName() )
		{
			sWarning.SetSprintf ( "index '%s': appying killlist to itself", tIndex.m_sIndex.cstr() );
			continue;
		}

		ServedDescRPtr_c pTarget ( GetServed ( tIndex.m_sIndex ) );
		if ( pTarget )
		{
			// kill the docids provided by sql_query_killlist and similar
			if ( tIndex.m_uFlags & KillListTarget_t::USE_KLIST )
				pTarget->m_pIndex->KillMulti ( dKillList );

			// kill all the docids present in this index
			if ( tIndex.m_uFlags & KillListTarget_t::USE_DOCIDS )
				pIndex->KillExistingDocids ( pTarget->m_pIndex );
		}
		else
			sWarning.SetSprintf ( "index '%s' from killlist_target not found", tIndex.m_sIndex.cstr() );
	}

	return true;
}


// we don't rlock/wlock the index because we assume that we are being called from a place that already did that for us
bool ApplyKillListsTo ( ServedDesc_t & tLockedServed, CSphString & sError )
{
	CSphIndex * pKillListTarget = tLockedServed.m_pIndex;

	KillListTargets_c tTargets;

	for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next(); )
	{
		if ( it.GetName ()==pKillListTarget->GetName () )
			continue;

		ServedDescRPtr_c tServedWithKlist ( it.Get () );
		if ( !tServedWithKlist)
			continue;

		CSphIndex * pIndexWithKillList = tServedWithKlist->m_pIndex;
		CSphFixedVector<DocID_t> dKillList(0);
		tTargets.m_dTargets.Resize(0);
		if ( !pIndexWithKillList->LoadKillList ( &dKillList, tTargets, sError ) )
			return false;

		if ( !dKillList.GetLength() )
			continue;

		// if this index has 'our' index as its killlist_target, apply the killlist
		for ( const auto & tIndex : tTargets.m_dTargets )
			if ( tIndex.m_sIndex==tLockedServed.m_pIndex->GetName() )
			{
				if ( tIndex.m_uFlags & KillListTarget_t::USE_KLIST )
					pKillListTarget->KillMulti ( dKillList );

				// kill all the docids present in this index
				if ( tIndex.m_uFlags & KillListTarget_t::USE_DOCIDS )
					pKillListTarget->KillExistingDocids ( pIndexWithKillList );
			}
	}

	return true;
}


enum class RotateFrom_e
{
	NONE,
	NEW,		// move index from idx.new.ext into idx.ext
	REENABLE,	// just load the index
	PATH_NEW,	// move from some external path, idx.new.ext into idx.ext
	PATH_COPY	// move from some external path to current path.
};


static bool PreloadKlistTarget ( const ServedDesc_t & tServed, RotateFrom_e eFrom, StrVec_t & dKlistTarget )
{
	switch ( eFrom )
	{
	case RotateFrom_e::NEW:
		return IndexFiles_c ( tServed.m_sIndexPath ).ReadKlistTargets ( dKlistTarget, ".new" );

	case RotateFrom_e::REENABLE:
		return IndexFiles_c ( tServed.m_sIndexPath ).ReadKlistTargets ( dKlistTarget );

	case RotateFrom_e::PATH_NEW:
		return IndexFiles_c ( tServed.m_sNewPath ).ReadKlistTargets ( dKlistTarget, ".new" );

	case RotateFrom_e::PATH_COPY:
		return IndexFiles_c ( tServed.m_sNewPath ).ReadKlistTargets ( dKlistTarget );

	default:
		return false;
	}
}


RotateFrom_e CheckIndexHeaderRotate ( const ServedDesc_t & tServed )
{
	// check order:
	// current_path/idx.new.sph		- rotation of current index
	// current_path/idx.sph			- enable back current index
	// new_path/idx.new.sph			- rotation of current index but with new path via indexer --rotate
	// new_path/idx.sph				- rotation of current index but with new path via files copy

	if ( IndexFiles_c ( tServed.m_sIndexPath ).CheckHeader (".new") )
		return RotateFrom_e::NEW;

	if ( tServed.m_bOnlyNew && IndexFiles_c ( tServed.m_sIndexPath ).CheckHeader() )
		return RotateFrom_e::REENABLE;

	if ( tServed.m_sNewPath.IsEmpty () || tServed.m_sNewPath==tServed.m_sIndexPath )
		return RotateFrom_e::NONE;

	if ( IndexFiles_c ( tServed.m_sNewPath ).CheckHeader ( ".new" ) )
		return RotateFrom_e::PATH_NEW;

	if ( IndexFiles_c ( tServed.m_sNewPath ).CheckHeader () )
		return RotateFrom_e::PATH_COPY;

	return RotateFrom_e::NONE;
}

/// returns true if any version of the index (old or new one) has been preread
bool RotateIndexGreedy (ServedDesc_t &tWlockedIndex, const char * szIndex, CSphString & sError )
{
	sphLogDebug ( "RotateIndexGreedy for '%s' invoked", szIndex );
	IndexFiles_c dFiles ( tWlockedIndex.m_sIndexPath, szIndex );
	RotateFrom_e eRot = CheckIndexHeaderRotate ( tWlockedIndex );
	bool bReEnable = ( eRot==RotateFrom_e::REENABLE );
	if ( eRot==RotateFrom_e::PATH_NEW || eRot==RotateFrom_e::PATH_COPY )
	{
		sError.SetSprintf ( "rotating index '%s': cannot rotate from new path, switch to seamless_rotate=1; using old index", szIndex );
		return false;
	}

	const char * sFromSuffix = bReEnable ? "" : ".new";
	if ( !dFiles.CheckHeader ( sFromSuffix ) )
	{
		// no files or wrong files - no rotation
		sError = dFiles.ErrorMsg();
		return false;
	}

	if ( !dFiles.HasAllFiles ( sFromSuffix ) )
	{
		sphWarning ( "rotating index '%s': unreadable: %s; %s", szIndex, strerrorm ( errno ),
			tWlockedIndex.m_bOnlyNew ? "NOT SERVING" : "using old index" );
		return false;
	}
	sphLogDebug ( bReEnable ? "RotateIndexGreedy: re-enabling index" : "RotateIndexGreedy: new index is readable" );
	if ( !tWlockedIndex.m_bOnlyNew )
	{
		if ( !dFiles.RenameSuffix ( "", ".old" ) )
		{
			if ( dFiles.IsFatal () )
				sphFatal ( "%s", dFiles.FatalMsg ( "rotating" ).cstr () );
			sError.SetSprintf ( "rotating index '%s': %s; using old index", szIndex, dFiles.ErrorMsg() );
			return false;
		}
		sphLogDebug ( "RotateIndexGreedy: Current index renamed to .old" );
	}


	// rename new to current
	if ( !bReEnable )
	{
		if ( !dFiles.RenameSuffix ( ".new" ) )
		{
			if ( dFiles.IsFatal () )
				sphFatal ( "%s", dFiles.FatalMsg ( "rotating" ).cstr () );

			// rollback old ones
			if ( tWlockedIndex.m_bOnlyNew )
				sError.SetSprintf ( "rotating index '%s': %s; using old index", szIndex, dFiles.ErrorMsg() );
			else if ( !dFiles.RollbackSuff ( ".old" ) )
				sphFatal ( "%s", dFiles.FatalMsg ( "rotating" ).cstr () );
			return false;
		}

		sphLogDebug ( "RotateIndexGreedy: New renamed to current" );
	}

	// try to use new index
	TokenizerRefPtr_c	pTokenizer { tWlockedIndex.m_pIndex->LeakTokenizer () }; // FIXME! disable support of that old indexes and remove this bullshit
	DictRefPtr_c		pDictionary { tWlockedIndex.m_pIndex->LeakDictionary () };

//	bool bRolledBack = false;
	bool bPreallocSuccess = tWlockedIndex.m_pIndex->Prealloc ( g_bStripPath, nullptr );
	if ( !bPreallocSuccess )
	{
		if ( tWlockedIndex.m_bOnlyNew )
		{
			sError.SetSprintf ( "rotating index '%s': .new prealloc failed: %s; NOT SERVING", szIndex, tWlockedIndex.m_pIndex->GetLastError().cstr() );
			return false;
		}
		sphWarning ( "rotating index '%s': .new preload failed: %s", szIndex, tWlockedIndex.m_pIndex->GetLastError().cstr() );
		// try to recover: rollback cur to .new, .old to cur.
		if ( !dFiles.RollbackSuff ( "", ".new" ) )
			sphFatal ( "%s", dFiles.FatalMsg ( "rotating" ).cstr () );
		if ( !dFiles.RollbackSuff ( ".old" ) )
			sphFatal ( "%s", dFiles.FatalMsg ( "rotating" ).cstr () );

		sphLogDebug ( "RotateIndexGreedy: has recovered. Prealloc it." );
		bPreallocSuccess = tWlockedIndex.m_pIndex->Prealloc ( g_bStripPath, nullptr );
		if ( !bPreallocSuccess )
			sError.SetSprintf ( "rotating index '%s': .new preload failed; ROLLBACK FAILED; INDEX UNUSABLE", szIndex );
//		bRolledBack = true;
	}

	if ( !tWlockedIndex.m_pIndex->GetLastWarning().IsEmpty() )
		sphWarning ( "rotating index '%s': %s", szIndex, tWlockedIndex.m_pIndex->GetLastWarning().cstr() );

	if ( !tWlockedIndex.m_pIndex->GetTokenizer () )
		tWlockedIndex.m_pIndex->SetTokenizer ( pTokenizer );

	if ( !tWlockedIndex.m_pIndex->GetDictionary () )
		tWlockedIndex.m_pIndex->SetDictionary ( pDictionary );

//	if ( bRolledBack )
//		return bPreallocSuccess;

	// unlink .old
	if ( !tWlockedIndex.m_bOnlyNew && sphGetUnlinkOld ())
	{
		dFiles.Unlink (".old");
		sphLogDebug ( "RotateIndexGreedy: the old index unlinked" );
	}

	sphLogDebug ( "rotating index '%s': applying other indexes' killlists", szIndex );

	// apply other indexes' killlists to THIS index
	if ( !ApplyKillListsTo ( tWlockedIndex, sError ) )
		return false;

	sphLogDebug ( "rotating index '%s': applying other indexes' killlists... DONE", szIndex );

	sphLogDebug ( "rotating index '%s': apply killlist from this index to other indexes (killlist_target)", szIndex );

	// apply killlist from this index to other indexes (killlist_target)
	CSphString sWarning;
	if ( !ApplyIndexKillList ( tWlockedIndex.m_pIndex, sWarning, sError ) )
		return false;

	if ( sWarning.Length() )
		sphWarning ( "rotating index '%s': %s", szIndex, sWarning.cstr() );

	sphLogDebug ( "rotating index '%s': apply killlist from this index to other indexes (killlist_target)... DONE", szIndex );

	// uff. all done
	tWlockedIndex.m_bOnlyNew = false;
	sphInfo ( "rotating index '%s': success", szIndex );
	return bPreallocSuccess;
}

void DumpMemStat ()
{
#if SPH_ALLOCS_PROFILER
	sphMemStatDump ( g_iLogFile );
#endif
}

/// check and report if there were any leaks since last call
void CheckLeaks () REQUIRES ( MainThread )
{
#if SPH_DEBUG_LEAKS
	static int iHeadAllocs = sphAllocsCount ();
	static int iHeadCheckpoint = sphAllocsLastID ();

	if ( g_dThd.GetLength()==0 && !g_bInRotate && iHeadAllocs!=sphAllocsCount() )
	{
		sphSeek ( g_iLogFile, 0, SEEK_END );
		sphAllocsDump ( g_iLogFile, iHeadCheckpoint );

		iHeadAllocs = sphAllocsCount ();
		iHeadCheckpoint = sphAllocsLastID ();
	}
#endif

#if SPH_ALLOCS_PROFILER
	int iAllocLogPeriod = 60 * 1000000;
	static int64_t tmLastLog = -iAllocLogPeriod*10;

	const int iAllocCount = sphAllocsCount();
	const float fMemTotal = (float)sphAllocBytes();

	if ( iAllocLogPeriod>0 && tmLastLog+iAllocLogPeriod<sphMicroTimer() )
	{
		tmLastLog = sphMicroTimer ();
		const int iThdsCount = g_dThd.GetLength ();
		const float fMB = 1024.0f*1024.0f;
		sphInfo ( "--- allocs-count=%d, mem-total=%.4f Mb, active-threads=%d", iAllocCount, fMemTotal/fMB, iThdsCount );
		DumpMemStat ();
	}
#endif
}

/// this gets called for every new physical index
/// that is, local and RT indexes, but not distributed once
bool PreallocNewIndex ( ServedDesc_t & tIdx, const CSphConfigSection * pConfig, const char * szIndexName, StrVec_t & dWarnings, CSphString & sError )
{
	CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( CreateFilenameBuilder(szIndexName) );
	if ( !tIdx.m_pIndex->Prealloc ( g_bStripPath, pFilenameBuilder.Ptr() ) )
	{
		sError.SetSprintf ( "prealloc: %s", tIdx.m_pIndex->GetLastError().cstr() );
		return false;
	}

	// tricky bit
	// fixup was initially intended for (very old) index formats that did not store dict/tokenizer settings
	// however currently it also ends up configuring dict/tokenizer for fresh RT indexes!
	// (and for existing RT indexes, settings get loaded during the Prealloc() call)
	if ( pConfig && !sphFixupIndexSettings ( tIdx.m_pIndex, *pConfig, g_bStripPath, pFilenameBuilder.Ptr(), dWarnings, sError ) )
		return false;

	// try to lock it
	if ( !g_bOptNoLock && !tIdx.m_pIndex->Lock () )
	{
		sError.SetSprintf ( "lock: %s", tIdx.m_pIndex->GetLastError().cstr() );
		return false;
	}

	CSphIndexStatus tStatus;
	tIdx.m_pIndex->GetStatus ( &tStatus );
	tIdx.m_iMass = CalculateMass ( tStatus );
	return true;
}

// same as above, but self-load config section for given index
static bool PreallocNewIndex ( ServedDesc_t & tIdx, const char * szIndexName, StrVec_t & dWarnings, CSphString & sError ) REQUIRES ( !g_tRotateConfigMutex )
{
	const CSphConfigSection * pIndexConfig = nullptr;
	CSphConfigSection tIndexConfig;
	{
		ScRL_t dRLockConfig { g_tRotateConfigMutex };
		if ( g_pCfg.m_tConf ( "index" ) )
			pIndexConfig = g_pCfg.m_tConf["index"] ( szIndexName );
		if ( pIndexConfig )
		{
			tIndexConfig = *pIndexConfig;
			pIndexConfig = &tIndexConfig;
		}
	}
	return PreallocNewIndex ( tIdx, pIndexConfig, szIndexName, dWarnings, sError );
}

static CSphMutex g_tRotateThreadMutex;
// called either from MysqlReloadIndex, either from Rotation task (never from main thread).
static bool RotateIndexMT ( ServedIndex_c* pIndex, const CSphString & sIndex, StrVec_t & dWarnings, CSphString & sError )
	EXCLUDES ( MainThread, g_tRotateThreadMutex )
{
	// only one rotation and reload thread allowed to prevent deadlocks
	ScopedMutex_t tBlockRotations ( g_tRotateThreadMutex );

	// get existing index. Look first to disabled hash.
	SafeAddRef ( pIndex );
	ServedIndexRefPtr_c pRotating { pIndex };
	if ( !pRotating || !ServedDescRPtr_c(pRotating)->m_pIndex )
	{
		pRotating = GetServed ( sIndex );
		if ( !pRotating )
		{
			sError.SetSprintf ( "rotating index '%s': internal error, index went AWOL", sIndex.cstr() );
			return false;
		}
	}

	//////////////////
	// load new index
	//////////////////
	sphInfo ( "rotating index '%s': started", sIndex.cstr() );

	// create new index, copy some settings from existing one
	ServedDesc_t tNewIndex;
	tNewIndex.m_pIndex = sphCreateIndexPhrase ( sIndex.cstr (), nullptr );
	tNewIndex.m_pIndex->m_iExpansionLimit = g_iExpansionLimit;

	IndexFiles_c dActivePath, dNewPath;
	RotateFrom_e eRot = RotateFrom_e::NONE;
	{
		ServedDescRPtr_c pCurrentlyServed ( pRotating );
		if ( !pCurrentlyServed->m_pIndex )
		{
			sError.SetSprintf ( "rotating index '%s': internal error, entry does not have an index", sIndex.cstr() );
			return false;
		}
		// keep settings from current index description
		tNewIndex.m_tFileAccessSettings = pCurrentlyServed->m_tFileAccessSettings;
		tNewIndex.m_iExpandKeywords = pCurrentlyServed->m_iExpandKeywords;
		tNewIndex.m_bPreopen = pCurrentlyServed->m_bPreopen;
		tNewIndex.m_sGlobalIDFPath = pCurrentlyServed->m_sGlobalIDFPath;

		// set settings into index
		tNewIndex.m_pIndex->m_iExpandKeywords = pCurrentlyServed->m_iExpandKeywords;
		tNewIndex.m_bOnlyNew = pCurrentlyServed->m_bOnlyNew;
		tNewIndex.m_pIndex->SetPreopen ( pCurrentlyServed->m_bPreopen || g_bPreopenIndexes );
		tNewIndex.m_pIndex->SetGlobalIDFPath ( pCurrentlyServed->m_sGlobalIDFPath );
		tNewIndex.m_pIndex->SetMemorySettings ( tNewIndex.m_tFileAccessSettings );
		dActivePath.SetBase ( pCurrentlyServed->m_sIndexPath );
		dNewPath.SetBase ( pCurrentlyServed->m_sNewPath );
		eRot = CheckIndexHeaderRotate ( *pCurrentlyServed );
	}

	if ( eRot==RotateFrom_e::NONE )
	{
		sError.SetSprintf ( "nothing to rotate for index '%s'", sIndex.cstr() );
		return false;
	}

	CSphString sPathTo;
	switch ( eRot )
	{
	case RotateFrom_e::PATH_NEW: // move from new_path/idx.new.sph -> new_path/idx.sph
		tNewIndex.m_pIndex->SetBase ( dNewPath.MakePath (".new").cstr() );
		sPathTo = dNewPath.GetBase();
		break;

	case RotateFrom_e::PATH_COPY: // load from new_path/idx.sph
		tNewIndex.m_pIndex->SetBase ( dNewPath.GetBase ().cstr() );
		break;

	case RotateFrom_e::REENABLE: // load from current_path/idx.sph
		tNewIndex.m_pIndex->SetBase ( dActivePath.GetBase ().cstr() );
		break;

	//case RotateFrom_e::NEW:  // move from current_path/idx.new.sph -> current_path/idx.sph
	default:
		tNewIndex.m_pIndex->SetBase ( dActivePath.MakePath ( ".new" ).cstr () );
		sPathTo = dActivePath.GetBase ();
	}

	// prealloc enough RAM and lock new index
	sphLogDebug ( "prealloc enough RAM and lock new index" );

	if ( tNewIndex.m_bOnlyNew )
	{
		if ( !PreallocNewIndex ( tNewIndex, sIndex.cstr(), dWarnings, sError ) )
			return false;
	} else if ( !PreallocNewIndex ( tNewIndex, nullptr, sIndex.cstr(), dWarnings, sError ) )
		return false;

	tNewIndex.m_pIndex->Preread();

	//////////////////////
	// activate new index
	//////////////////////

	sphLogDebug ( "activate new index" );

	ServedDescRPtr_c pCurrentlyServed ( pRotating );

	CSphIndex * pOld = pCurrentlyServed->m_pIndex;
	CSphIndex * pNew = tNewIndex.m_pIndex;

	bool bHaveBackup = false;
	if ( !sPathTo.IsEmpty () )
	{
		if ( dActivePath.GetBase ()==sPathTo && !pCurrentlyServed->m_bOnlyNew && dActivePath.CheckHeader () )
		{
			// moving to active path; need backup to .old!
			bHaveBackup = pOld->Rename ( dActivePath.MakePath ( ".old" ).cstr () );
			if ( !bHaveBackup )
			{
				sError.SetSprintf ( "rotating index '%s': cur to old rename failed: %s", sIndex.cstr ()
									, pOld->GetLastError ().cstr () );
				return false;
			}
		}

		if ( !pNew->Rename ( sPathTo.cstr() ) )
		{
			sError.SetSprintf ( "rotating index '%s': new to cur rename failed: %s", sIndex.cstr ()
								, pNew->GetLastError ().cstr () );
			if ( bHaveBackup )
			{
				if ( !pOld->Rename( dActivePath.GetBase ().cstr ()))
				{
					sError.SetSprintf ( "rotating index '%s': old to cur rename failed: %s; INDEX UNUSABLE"
										, sIndex.cstr (), pOld->GetLastError ().cstr () );
					g_pLocalIndexes->Delete ( sIndex );
				}
			}
			return false;
		}
		if ( bHaveBackup )
			pCurrentlyServed->m_sUnlink = dActivePath.MakePath ( ".old" );
	}

	sphLogDebug ( "rotating index '%s': applying other indexes' killlists", sIndex.cstr() );

	// apply other indexes' killlists to THIS index
	if ( !ApplyKillListsTo ( tNewIndex, sError ) )
		return false;

	sphLogDebug ( "rotating index '%s': applying other indexes' killlists... DONE", sIndex.cstr() );

	// all went fine; swap them
	sphLogDebug ( "all went fine; swap them" );

	pNew->m_iTID = pOld->m_iTID;
	if ( g_pBinlog )
		g_pBinlog->NotifyIndexFlush ( sIndex.cstr(), pOld->m_iTID, false );

	// set new index to hash
	tNewIndex.m_sIndexPath = tNewIndex.m_pIndex->GetFilename();
	g_pLocalIndexes->AddOrReplace ( RefCountedRefPtr_t ( new ServedIndex_c ( tNewIndex )), sIndex );
	sphInfo ( "rotating index '%s': success", sIndex.cstr() );
	tNewIndex.m_pIndex = nullptr;

	sphLogDebug ( "rotating index '%s': apply killlist from this index to other indexes (killlist_target)", sIndex.cstr() );

	auto pServedForKlist = GetServed ( sIndex );
	if ( pServedForKlist )
	{
		ServedDescRPtr_c pIndexForKlist ( pServedForKlist );

		// apply killlist from this index to other indexes (killlist_target)
		// if this fails, only show a warning
		CSphString sWarning;
		if ( !ApplyIndexKillList ( pIndexForKlist->m_pIndex, sWarning, sError ) )
			sphWarning ( "rotating index '%s': %s", sIndex.cstr(), sError.cstr() );

		if ( sWarning.Length() )
			sphWarning ( "rotating index '%s': %s", sIndex.cstr(), sWarning.cstr() );

		sphLogDebug ( "rotating index '%s': apply killlist from this index to other indexes (killlist_target)... DONE", sIndex.cstr() );
	}

	return true;
}

struct IndexForRotation_t
{
	int m_iPriority;
	CSphString m_sIndex;
	ServedIndexRefPtr_c m_pIndex;
};

static bool operator<( const IndexForRotation_t& dLeft, const IndexForRotation_t& dRight)
{
	return dLeft.m_iPriority < dRight.m_iPriority;
}

using VecIndexForRotation_t = CSphVector<IndexForRotation_t>;

static void TaskRotation ( void* pRaw ) EXCLUDES ( MainThread )
{
	CSphScopedPtr<VecIndexForRotation_t> pUncompletedIndexes (( VecIndexForRotation_t* ) pRaw );

	// want to track rotation thread only at work
	auto pDesc = PublishSystemInfo ( "ROTATION" );

	for ( const auto & tIndex : *pUncompletedIndexes.Ptr() )
	{
		bool bMutable = false;
		{ // rlock scope
			ServedDescRPtr_c tLocked ( tIndex.m_pIndex );
			bMutable = ServedDesc_t::IsMutable ( tLocked );
			// cluster indexes got managed by different path
			if ( ServedDesc_t::IsCluster ( tLocked ))
			{
				assert ( 0 && "Rotation of clusters MUST never happens!" );
				return;
			}
		}

		// prealloc RT and percolate here
		StrVec_t dWarnings;
		CSphString sError;
		if ( bMutable )
		{
			ServedDescWPtr_c wLocked( tIndex.m_pIndex );
			if ( PreallocNewIndex ( *wLocked, tIndex.m_sIndex.cstr(), dWarnings, sError ) )
			{
				wLocked->m_bOnlyNew = false;
				g_pLocalIndexes->AddOrReplace ( tIndex.m_pIndex, tIndex.m_sIndex );
			} else
			{
				sphWarning ( "index '%s': %s", tIndex.m_sIndex.cstr(), sError.cstr() );
				g_pLocalIndexes->DeleteIfNull ( tIndex.m_sIndex );
			}
		} else
		{
			if ( !RotateIndexMT ( tIndex.m_pIndex, tIndex.m_sIndex, dWarnings, sError ) )
				sphWarning ( "index '%s': %s", tIndex.m_sIndex.cstr(), sError.cstr() );
		}

		for ( const auto & i : dWarnings )
			sphWarning ( "index '%s': %s", tIndex.m_sIndex.cstr(), i.cstr() );

		g_pDistIndexes->Delete ( tIndex.m_sIndex ); // postponed delete of same-named distributed (if any)
	}

	g_bInRotate = false;
	RotateGlobalIdf ();
	sphInfo ( "rotating index: all indexes done" );
}

static void AbortRotation ( void* pIndexes )
{
	auto* pToDelete = ( VecIndexForRotation_t* ) pIndexes;
	SafeDelete ( pToDelete );
	g_bInRotate = false;
}

static void InvokeRotation() REQUIRES (MainThread)
{
	assert ( g_dPostIndexes.GetLength () && "Rotation queue must be checked before invoking rotation!");

	auto* pIndexesForRotation = new VecIndexForRotation_t;
	for ( RLockedServedIt_c it ( &g_dPostIndexes ); it.Next (); )
	{
		ServedDescRPtr_c rLocked( it.Get());
		pIndexesForRotation->Add( { rLocked->m_iRotationPriority, it.GetName(), it.Get() });
	}
	g_dPostIndexes.ReleaseAndClear ();

	// sort the indexes by rotation priority
	pIndexesForRotation->Sort();

	static TaskID iRotationTask = -1;
	if ( iRotationTask<0 )
		iRotationTask = TaskManager::RegisterGlobal ("Rotation task", TaskRotation, AbortRotation, 1, 1);

	TaskManager::StartJob ( iRotationTask, pIndexesForRotation );
}


static int ParseKeywordExpansion ( const char * sValue )
{
	if ( !sValue || *sValue=='\0' )
		return KWE_DISABLED;

	int iOpt = KWE_DISABLED;
	while ( sValue && *sValue )
	{
		if ( !sphIsAlpha ( *sValue ) )
		{
			sValue++;
			continue;
		}

		if ( *sValue>='0' && *sValue<='9' )
		{
			int iVal = atoi ( sValue );
			if ( iVal!=0 )
				iOpt = KWE_ENABLED;
			break;
		}

		if ( sphStrMatchStatic ( "exact", sValue ) )
		{
			iOpt |= KWE_EXACT;
			sValue += 5;
		} else if ( sphStrMatchStatic ( "star", sValue ) )
		{
			iOpt |= KWE_STAR;
			sValue += 4;
		} else
		{
			sValue++;
		}
	}

	return iOpt;
}


void ConfigureTemplateIndex ( ServedDesc_t * pIdx, const CSphConfigSection & hIndex )
{
	pIdx->m_iExpandKeywords = ParseKeywordExpansion ( hIndex.GetStr( "expand_keywords" ).cstr() );
}


static FileAccess_e GetFileAccess (  const CSphConfigSection & hIndex, const char * sKey, bool bList, FileAccess_e eDefault )
{
	// should use original value as default due to deprecated options
	auto eValue = eDefault;
	auto sVal = hIndex.GetStr(sKey);
	if ( !sVal.IsEmpty() )
		eValue = ParseFileAccess ( sVal.cstr() );
	if ( eValue==FileAccess_e::UNKNOWN )
	{
		sphWarning( "%s unknown value %s, use default %s", sKey, sVal.cstr(), FileAccessName( eDefault ));
		eValue = eDefault;
	}

	// but then check might reset invalid value to real default
	if ( ( bList && eValue==FileAccess_e::MMAP_PREREAD) ||
		( !bList && eValue==FileAccess_e::FILE) )
	{
		sphWarning( "%s invalid value %s, use default %s", sKey, FileAccessName(eValue), FileAccessName(eDefault));
		return eDefault;
	}
	return eValue;
}


void ConfigureLocalIndex ( ServedDesc_t * pIdx, const CSphConfigSection & hIndex )
{
	ConfigureTemplateIndex ( pIdx, hIndex);
	pIdx->m_bPreopen = ( hIndex.GetInt ( "preopen", 0 )!=0 );
	pIdx->m_sGlobalIDFPath = hIndex.GetStr ( "global_idf" );
	auto& tFileSettings = pIdx->m_tFileAccessSettings;

	tFileSettings = g_tDefaultFA;
	// DEPRICATED - remove these 2 options
	if ( hIndex.GetBool ( "mlock", false ) )
	{
		tFileSettings.m_eAttr = FileAccess_e::MLOCK;
		tFileSettings.m_eBlob = FileAccess_e::MLOCK;
	}
	if ( hIndex.Exists ( "ondisk_attrs" ) )
	{
		bool bOnDiskAttrs = hIndex.GetBool ( "ondisk_attrs", false );
		bool bOnDiskPools = ( hIndex.GetStr ( "ondisk_attrs" )=="pool" );

		if ( bOnDiskAttrs || bOnDiskPools )
			tFileSettings.m_eAttr = FileAccess_e::MMAP;
		if ( bOnDiskPools )
			tFileSettings.m_eBlob = FileAccess_e::MMAP;
	}

	// need to keep value from deprecated options for some time - use it as defaults on parse for now
	tFileSettings.m_eAttr = GetFileAccess( hIndex, "access_plain_attrs", false, tFileSettings.m_eAttr );
	tFileSettings.m_eBlob = GetFileAccess( hIndex, "access_blob_attrs", false, tFileSettings.m_eBlob );
	tFileSettings.m_eDoclist = GetFileAccess( hIndex, "access_doclists", true, tFileSettings.m_eDoclist );
	tFileSettings.m_eHitlist = GetFileAccess( hIndex, "access_hitlists", true, tFileSettings.m_eHitlist );

	// inherit global value, might be absent - 0
	// set correct value via GetReadBuffer
	pIdx->m_tFileAccessSettings.m_iReadBufferDocList = GetReadBuffer ( hIndex.GetInt ( "read_buffer_docs", g_tDefaultFA.m_iReadBufferDocList ) );
	pIdx->m_tFileAccessSettings.m_iReadBufferHitList = GetReadBuffer ( hIndex.GetInt ( "read_buffer_hits", g_tDefaultFA.m_iReadBufferHitList ) );
}


static void ConfigureDistributedIndex ( DistributedIndex_t & tIdx, const char * szIndexName, const CSphConfigSection & hIndex, StrVec_t * pWarnings=nullptr ) REQUIRES ( MainThread )
{
	assert ( hIndex("type") && hIndex["type"]=="distributed" );

	bool bSetHA = false;
	// configure ha_strategy
	if ( hIndex("ha_strategy") )
	{
		bSetHA = ParseStrategyHA ( hIndex["ha_strategy"].cstr(), tIdx.m_eHaStrategy );
		if ( !bSetHA )
			sphWarning ( "index '%s': ha_strategy (%s) is unknown for me, will use random", szIndexName, hIndex["ha_strategy"].cstr() );
	}

	bool bEnablePersistentConns = ( g_iPersistentPoolSize>0 );
	if ( hIndex ( "agent_persistent" ) && !bEnablePersistentConns )
	{
			sphWarning ( "index '%s': agent_persistent used, but no persistent_connections_limit defined. Fall back to non-persistent agent", szIndexName );
			bEnablePersistentConns = false;
	}

	// add local agents
	StrVec_t dLocs;
	for ( CSphVariant * pLocal = hIndex("local"); pLocal; pLocal = pLocal->m_pNext )
	{
		dLocs.Resize(0);
		sphSplit ( dLocs, pLocal->cstr(), " \t," );
		for ( const auto & sLocal: dLocs )
		{
			if ( !g_pLocalIndexes->Contains ( sLocal ) )
			{
				sphWarning ( "index '%s': no such local index '%s', SKIPPED", szIndexName, sLocal.cstr() );
				continue;
			}
			tIdx.m_dLocal.Add ( sLocal );
		}
	}

	// index-level agent_retry_count
	if ( hIndex ( "agent_retry_count" ) )
	{
		if ( hIndex["agent_retry_count"].intval ()<=0 )
			sphWarning ( "index '%s': agent_retry_count must be positive, ignored", szIndexName );
		else
			tIdx.m_iAgentRetryCount = hIndex["agent_retry_count"].intval ();
	}

	if ( hIndex ( "mirror_retry_count" ) )
	{
		if ( hIndex["mirror_retry_count"].intval ()<=0 )
			sphWarning ( "index '%s': mirror_retry_count must be positive, ignored", szIndexName );
		else
		{
			if ( tIdx.m_iAgentRetryCount>0 )
				sphWarning ("index '%s': `agent_retry_count` and `mirror_retry_count` both specified (they are aliases)."
					"Value of `mirror_retry_count` will be used", szIndexName );
			tIdx.m_iAgentRetryCount = hIndex["mirror_retry_count"].intval ();
		}
	}

	if ( !tIdx.m_iAgentRetryCount )
		tIdx.m_iAgentRetryCount = g_iAgentRetryCount;


	// add remote agents
	struct { const char* sSect; bool bBlh; bool bPrs; } dAgentVariants[] =
	{
		{ "agent", 				false,	false},
		{ "agent_persistent", 	false,	bEnablePersistentConns },
		{ "agent_blackhole", 	true,	false }
	};

	for ( auto & tAg : dAgentVariants )
	{
		for ( CSphVariant * pAgentCnf = hIndex ( tAg.sSect ); pAgentCnf; pAgentCnf = pAgentCnf->m_pNext )
		{
			AgentOptions_t tAgentOptions { tAg.bBlh, tAg.bPrs, tIdx.m_eHaStrategy, tIdx.m_iAgentRetryCount, 0 };
			auto pAgent = ConfigureMultiAgent ( pAgentCnf->cstr(), szIndexName, tAgentOptions, pWarnings );
			if ( pAgent )
				tIdx.m_dAgents.Add ( pAgent );
		}
	}

	// configure options
	if ( hIndex("agent_connect_timeout") )
	{
		if ( hIndex["agent_connect_timeout"].intval()<=0 )
			sphWarning ( "index '%s': agent_connect_timeout must be positive, ignored", szIndexName );
		else
			tIdx.m_iAgentConnectTimeoutMs = hIndex.GetMsTimeMs ( "agent_connect_timeout" );
	}

	tIdx.m_bDivideRemoteRanges = hIndex.GetInt ( "divide_remote_ranges", 0 )!=0;

	if ( hIndex("agent_query_timeout") )
	{
		if ( hIndex["agent_query_timeout"].intval()<=0 )
			sphWarning ( "index '%s': agent_query_timeout must be positive, ignored", szIndexName );
		else
			tIdx.m_iAgentQueryTimeoutMs = hIndex.GetMsTimeMs ( "agent_query_timeout");
	}

	bool bHaveHA = tIdx.m_dAgents.FindFirst ( [] ( MultiAgentDesc_c * ag ) { return ag->IsHA (); } );

	// configure ha_strategy
	if ( bSetHA && !bHaveHA )
		sphWarning ( "index '%s': ha_strategy defined, but no ha agents in the index", szIndexName );
}

//////////////////////////////////////////////////
/// configure distributed index and add it to hash
//////////////////////////////////////////////////
static ESphAddIndex AddDistributedIndex ( const char * szIndexName, const CSphConfigSection & hIndex, CSphString & sError, StrVec_t * pWarnings=nullptr ) REQUIRES ( MainThread )
{
	DistributedIndexRefPtr_t pIdx ( new DistributedIndex_t );
	ConfigureDistributedIndex ( *pIdx, szIndexName, hIndex, pWarnings );

	// finally, check and add distributed index to global table
	if ( pIdx->IsEmpty () )
	{
		sError.SetSprintf ( "index '%s': no valid local/remote indexes in distributed index", szIndexName );
		return ADD_ERROR;
	}

	if ( !g_pDistIndexes->AddUniq ( pIdx, szIndexName ) )
	{
		sError.SetSprintf ( "index '%s': unable to add name (duplicate?)", szIndexName );
		return ADD_ERROR;
	}

	return ADD_DISTR;
}

// hash any local index (plain, rt, etc.)
static bool AddLocallyServedIndex ( GuardedHash_c& dPost, const CSphString& sIndexName, ServedDesc_t & tIdx, bool bReplace, CSphString & sError )
{
	assert ( tIdx.m_eType!=IndexType_e::TEMPLATE );

	ServedIndexRefPtr_c pIdx ( new ServedIndex_c ( tIdx ) );
	g_pLocalIndexes->AddUniq ( nullptr, sIndexName );
	if ( !dPost.AddUniq ( pIdx, sIndexName ) )
	{
		sError = "internal error: hash add failed";
		g_pLocalIndexes->DeleteIfNull ( sIndexName );
		return false;
	}

	// leak pointer, so it's destructor won't delete it
	tIdx.m_pIndex = nullptr;
	return true;
}

// common preconfiguration of mutable indexes
static bool ConfigureRTPercolate ( CSphSchema & tSchema, CSphIndexSettings & tSettings, const char * szIndexName, const CSphConfigSection & hIndex,
	bool bReplace, bool bWordDict, bool bPercolate, CSphString & sError )
{
	if ( !sphRTSchemaConfigure ( hIndex, tSchema, sError, bPercolate ) )
	{
		sphWarning ( "index '%s': %s - NOT SERVING", szIndexName, sError.cstr () );
		return false;
	}

	if ( bPercolate )
		FixPercolateSchema ( tSchema );

	if ( !sError.IsEmpty () )
		sphWarning ( "index '%s': %s", szIndexName, sError.cstr () );

	// path
	if ( !hIndex ( "path" ) )
	{
		sphWarning ( "index '%s': path must be specified - NOT SERVING", szIndexName );
		return false;
	}

	// check name
	if ( !bReplace && g_pLocalIndexes->Contains ( szIndexName ) )
	{
		sphWarning ( "index '%s': duplicate name - NOT SERVING", szIndexName );
		return false;
	}

	// pick config settings
	// they should be overriden later by Preload() if needed
	CSphString sWarning;
	if ( !tSettings.Setup ( hIndex, szIndexName, sWarning, sError ) )
	{
		sphWarning ( "ERROR: index '%s': %s - NOT SERVING", szIndexName, sError.cstr() );
		return false;
	}

	if ( !sWarning.IsEmpty() )
		sphWarning ( "WARNING: index '%s': %s", szIndexName, sWarning.cstr() );

	if ( !CheckStoredFields ( tSchema, tSettings, sError ) )
	{
		sphWarning ( "ERROR: index '%s': %s - NOT SERVING", szIndexName, sError.cstr() );
		return false;
	}

	int iIndexSP = hIndex.GetInt ( "index_sp" );
	auto sIndexZones = hIndex.GetStr ( "index_zones" );
	bool bHasStripEnabled ( hIndex.GetInt ( "html_strip" )!=0 );
	if ( ( iIndexSP!=0 || !sIndexZones.IsEmpty() ) && !bHasStripEnabled )
	{
		// SENTENCE indexing w\o stripper is valid combination
		if ( !sIndexZones.IsEmpty() )
		{
			sphWarning ( "ERROR: index '%s': has index_sp=%d, index_zones='%s' but disabled html_strip - NOT SERVING", szIndexName, iIndexSP, sIndexZones.cstr() );
			return false;
		} else
			sphWarning ( "index '%s': has index_sp=%d but disabled html_strip - PARAGRAPH unavailable", szIndexName, iIndexSP );
	}

	// upgrading schema to store field lengths
	if ( tSettings.m_bIndexFieldLens )
		if ( !AddFieldLens ( tSchema, false, sError ) )
		{
			sphWarning ( "index '%s': failed to create field lengths attributes: %s", szIndexName, sError.cstr () );
			return false;
		}

	if ( bWordDict && ( tSettings.m_dPrefixFields.GetLength () || tSettings.m_dInfixFields.GetLength () ) )
		sphWarning ( "WARNING: index '%s': prefix_fields and infix_fields has no effect with dict=keywords, ignoring\n", szIndexName);

	if ( bWordDict && tSettings.m_iMinInfixLen==1 )
	{
		sphWarn ( "min_infix_len must be greater than 1, changed to 2" );
		tSettings.m_iMinInfixLen = 2;
	}

	tSchema.SetupStoredFields ( tSettings.m_dStoredFields, tSettings.m_dStoredOnlyFields );

	return true;
}

// common configuration and add percolate or rt index
static ESphAddIndex AddRTPercolate ( GuardedHash_c & dPost, const char * szIndexName, ServedDesc_t & tIdx, const CSphConfigSection & hIndex, const CSphIndexSettings & tSettings,
	bool bReplace, CSphString & sError )
{
	tIdx.m_sIndexPath = hIndex["path"].strval ();
	ConfigureLocalIndex ( &tIdx, hIndex );
	tIdx.m_pIndex->m_iExpandKeywords = tIdx.m_iExpandKeywords;
	tIdx.m_pIndex->m_iExpansionLimit = g_iExpansionLimit;
	tIdx.m_pIndex->SetPreopen ( tIdx.m_bPreopen || g_bPreopenIndexes );
	tIdx.m_pIndex->SetGlobalIDFPath ( tIdx.m_sGlobalIDFPath );
	tIdx.m_pIndex->SetMemorySettings ( tIdx.m_tFileAccessSettings );

	tIdx.m_pIndex->Setup ( tSettings );
	tIdx.m_pIndex->SetCacheSize ( g_iMaxCachedDocs, g_iMaxCachedHits );

	CSphIndexStatus tStatus;
	tIdx.m_pIndex->GetStatus ( &tStatus );
	tIdx.m_iMass = CalculateMass ( tStatus );

	// hash it
	if ( AddLocallyServedIndex ( dPost, szIndexName, tIdx, bReplace, sError ) )
		return ADD_DSBLED;
	return ADD_ERROR;
}
///////////////////////////////////////////////
/// configure realtime index and add it to hash
///////////////////////////////////////////////
ESphAddIndex AddRTIndex ( GuardedHash_c & dPost, const char * szIndexName, const CSphConfigSection & hIndex, bool bReplace, CSphString & sError, StrVec_t * pWarnings )
{
	auto sIndexType = hIndex.GetStr ( "dict", "keywords" );
	bool bWordDict = true;
	if ( sIndexType=="crc" )
		bWordDict = false;
	else if ( sIndexType!="keywords" )
	{
		sError.SetSprintf ( "index '%s': unknown dict=%s; only 'keywords' or 'crc' values allowed", szIndexName, sIndexType.cstr() );
		return ADD_ERROR;
	}

	// RAM chunk size
	int64_t iRamSize = hIndex.GetSize64 ( "rt_mem_limit", DEFAULT_RT_MEM_LIMIT );
	if ( iRamSize<128 * 1024 )
	{
		if ( pWarnings )
			pWarnings->Add ( "rt_mem_limit extremely low, using 128K instead" );
		iRamSize = 128 * 1024;
	} else if ( iRamSize<8 * 1024 * 1024 )
	{
		if ( pWarnings )
			pWarnings->Add ( "rt_mem_limit very low (under 8 MB)" );
	}

	CSphSchema tSchema ( szIndexName );
	CSphIndexSettings tSettings;
	if ( !ConfigureRTPercolate ( tSchema, tSettings, szIndexName, hIndex, bReplace, bWordDict, false, sError ))
		return ADD_ERROR;

	// index
	ServedDesc_t tIdx;
	tIdx.m_pIndex = sphCreateIndexRT ( tSchema, szIndexName, iRamSize, hIndex["path"].cstr (), bWordDict );
	tIdx.m_eType = IndexType_e::RT;
	tIdx.m_iMemLimit = iRamSize;
	return AddRTPercolate ( dPost, szIndexName, tIdx, hIndex, tSettings, bReplace, sError );
}

////////////////////////////////////////////////
/// configure percolate index and add it to hash
////////////////////////////////////////////////
ESphAddIndex AddPercolateIndex ( GuardedHash_c & dPost, const char * szIndexName, const CSphConfigSection & hIndex, bool bReplace, CSphString & sError )
{
	CSphSchema tSchema ( szIndexName );
	CSphIndexSettings tSettings;
	if ( !ConfigureRTPercolate ( tSchema, tSettings, szIndexName, hIndex, bReplace, true, true, sError ) )
		return ADD_ERROR;

	// index
	ServedDesc_t tIdx;
	tIdx.m_pIndex = CreateIndexPercolate ( tSchema, szIndexName, hIndex["path"].cstr () );
	tIdx.m_eType = IndexType_e::PERCOLATE;
	return AddRTPercolate ( dPost, szIndexName, tIdx, hIndex, tSettings, bReplace, sError );
}

////////////////////////////////////////////
/// configure local index and add it to hash
////////////////////////////////////////////
static ESphAddIndex AddPlainIndex ( const char * szIndexName, const CSphConfigSection & hIndex, bool bReplace, CSphString & sError ) REQUIRES ( MainThread )
{
	ServedDesc_t tIdx;

	// check path
	if ( !hIndex.Exists ( "path" ) )
	{
		sError = "key 'path' not found";
		return ADD_ERROR;
	}

	// check name
	if ( !bReplace && g_pLocalIndexes->Contains ( szIndexName ) )
	{
		sError = "duplicate name";
		return ADD_ERROR;
	}

	// configure memlocking, star
	ConfigureLocalIndex ( &tIdx, hIndex );

	// try to create index
	tIdx.m_sIndexPath = hIndex["path"].strval ();
	tIdx.m_pIndex = sphCreateIndexPhrase ( szIndexName, tIdx.m_sIndexPath.cstr () );
	tIdx.m_pIndex->m_iExpandKeywords = tIdx.m_iExpandKeywords;
	tIdx.m_pIndex->m_iExpansionLimit = g_iExpansionLimit;
	tIdx.m_pIndex->SetPreopen ( tIdx.m_bPreopen || g_bPreopenIndexes );
	tIdx.m_pIndex->SetGlobalIDFPath ( tIdx.m_sGlobalIDFPath );
	tIdx.m_pIndex->SetMemorySettings ( tIdx.m_tFileAccessSettings );
	tIdx.m_pIndex->SetCacheSize ( g_iMaxCachedDocs, g_iMaxCachedHits );
	CSphIndexStatus tStatus;
	tIdx.m_pIndex->GetStatus ( &tStatus );
	tIdx.m_iMass = CalculateMass ( tStatus );

	// done
	if ( AddLocallyServedIndex ( g_dPostIndexes, szIndexName, tIdx, bReplace, sError ) )
		return ADD_DSBLED;
	return ADD_ERROR;
}

///////////////////////////////////////////////
/// configure template index and add it to hash
///////////////////////////////////////////////
static ESphAddIndex AddTemplateIndex ( const char * szIndexName, const CSphConfigSection &hIndex, bool bReplace )
	REQUIRES ( MainThread )
{
	ServedDesc_t tIdx;

	// check name
	if ( !bReplace && g_pLocalIndexes->Contains ( szIndexName ) )
	{
		sphWarning ( "index '%s': duplicate name - NOT SERVING", szIndexName );
		return ADD_ERROR;
	}

	// configure memlocking, star
	ConfigureTemplateIndex ( &tIdx, hIndex );

	// try to create index
	tIdx.m_pIndex = sphCreateIndexTemplate ( szIndexName );
	tIdx.m_pIndex->m_iExpandKeywords = tIdx.m_iExpandKeywords;
	tIdx.m_pIndex->m_iExpansionLimit = g_iExpansionLimit;

	CSphIndexSettings s;
	CSphString sWarning, sError;
	if ( !s.Setup ( hIndex, szIndexName, sWarning, sError ) )
	{
		sphWarning ( "failed to configure index %s: %s", szIndexName, sError.cstr () );
		return ADD_ERROR;
	}

	if ( !sWarning.IsEmpty() )
		sphWarning ( "ERROR: index '%s': %s - NOT SERVING", szIndexName, sWarning.cstr () );

	tIdx.m_pIndex->Setup ( s );

	CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( CreateFilenameBuilder(szIndexName) );
	StrVec_t dWarnings;
	if ( !sphFixupIndexSettings ( tIdx.m_pIndex, hIndex, g_bStripPath, pFilenameBuilder.Ptr(), dWarnings, sError ) )
	{
		sphWarning ( "index '%s': %s - NOT SERVING", szIndexName, sError.cstr () );
		return ADD_ERROR;
	}

	for ( const auto & i : dWarnings )
		sphWarning ( "index '%s': %s", szIndexName, i.cstr() );

	CSphIndexStatus tStatus;
	tIdx.m_pIndex->GetStatus ( &tStatus );
	tIdx.m_iMass = CalculateMass ( tStatus );
	tIdx.m_eType = IndexType_e::TEMPLATE;

	// templates we either add, either replace depending on requiested action
	// at this point they are production-ready
	ServedIndexRefPtr_c pIdx( new ServedIndex_c( tIdx ));
	if ( bReplace )
		g_pLocalIndexes->AddOrReplace( pIdx, szIndexName );
	else if ( !g_pLocalIndexes->AddUniq( pIdx, szIndexName ))
	{
		sphWarning( "internal error: index '%s': hash add failed - NOT SERVING", szIndexName );
		return ADD_ERROR;
	}
	tIdx.m_pIndex = nullptr; // leak pointer, so it's destructor won't delete it
	return ADD_SERVED;
}

// AddIndex() -> AddIndexMT() // from main thread
// RemoteLoadIndex() -> LoadIndex() -> AddIndexMT() // only Percolate! From other threads
ESphAddIndex AddIndexMT ( GuardedHash_c & dPost, const char * szIndexName, const CSphConfigSection & hIndex, bool bReplace, CSphString & sError, StrVec_t * pWarnings )
{
	switch ( TypeOfIndexConfig ( hIndex.GetStr ( "type", nullptr ) ) )
	{
		case IndexType_e::RT:		return AddRTIndex ( dPost, szIndexName, hIndex, bReplace, sError, pWarnings );
		case IndexType_e::PERCOLATE:return AddPercolateIndex ( dPost, szIndexName, hIndex, bReplace, sError );
		case IndexType_e::DISTR:	return AddDistributedIndex ( szIndexName, hIndex, sError, pWarnings );

		default:; // shut up warnings
	}
	assert ( 0 && "AddIndexMT expects ONLY rt, percolate or distr");
	return ADD_ERROR;
}

// ServiceMain() -> ConfigureAndPreload() -> AddIndex()
// ServiceMain() -> TickHead() -> CheckRotate() -> ReloadIndexSettings() -> AddIndex()
static ESphAddIndex AddIndex ( const char * szIndexName, const CSphConfigSection & hIndex, bool bReplace, CSphString & sError ) REQUIRES (	MainThread )
{
	switch ( TypeOfIndexConfig ( hIndex.GetStr ( "type", nullptr )))
	{
		case IndexType_e::DISTR:
			return AddDistributedIndex ( szIndexName, hIndex, sError );
		case IndexType_e::RT:
		case IndexType_e::PERCOLATE:
			return AddIndexMT ( g_dPostIndexes, szIndexName, hIndex, bReplace, sError );
		case IndexType_e::TEMPLATE:
			return AddTemplateIndex ( szIndexName, hIndex, bReplace );
		case IndexType_e::PLAIN:
			return AddPlainIndex ( szIndexName, hIndex, bReplace, sError );
		case IndexType_e::ERROR_:
		default:
			break;
	}

	sphWarning ( "index '%s': unknown type '%s' - NOT SERVING", szIndexName, hIndex["type"].cstr() );
	return ADD_ERROR;
}

// check if config changed, and also cache content into g_dConfig (will be used instead of one more config touching)
CSphVector<char> g_dConfig;
bool LoadAndCheckConfig ()
{
	static DWORD			uCfgCRC32		= 0;
	static struct stat		tCfgStat;

	g_dConfig.Reset();
	DWORD uCRC32 = 0;
	struct_stat tStat = {0};

	const size_t BUF_SIZE = 8192;
	char sBuf [ BUF_SIZE ];
	FILE * fp = nullptr;

	fp = fopen ( g_sConfigFile.scstr(), "rb" );
	if ( !fp )
		return true;
	if ( fstat ( fileno ( fp ), &tStat )<0 )
		memset ( &tStat, 0, sizeof ( tStat ) );
	bool bGotLine = !!fgets ( sBuf, sizeof(sBuf), fp );

	if ( !bGotLine )
	{
		fclose ( fp );
		return true;
	}

	char * p;
	const char* pEnd = sBuf + BUF_SIZE;
	for ( p = sBuf; p<pEnd; ++p )
		if ( !isspace(*p) )
			break;

#if USE_WINDOWS
	bool bIsWindows = true;
#else
	bool bIsWindows = false;
#endif

	if ( !bIsWindows && p<sBuf+BUF_SIZE-1 && p[0]=='#' && p[1]=='!' )
	{
		sBuf[BUF_SIZE-1] = '\0'; // just safety
		fclose ( fp );
		if ( !TryToExec ( p+2, g_sConfigFile.cstr(), g_dConfig ) )
		{
			g_dConfig.Reset();
			return true;
		}

		uCRC32 = sphCRC32 ( g_dConfig.Begin(), g_dConfig.GetLength() );
	} else
	{
		while ( bGotLine ) {
			auto iLen = (int) strlen ( sBuf );
			g_dConfig.Append ( sBuf, iLen );
			bGotLine = !!fgets ( sBuf, sizeof ( sBuf ), fp );
		}
		g_dConfig.Add('\0');
		fclose ( fp );
		uCRC32 = sphCRC32 ( g_dConfig.Begin (), g_dConfig.GetLength ());
	}

	if ( uCfgCRC32==uCRC32 && tStat.st_size==tCfgStat.st_size
		&& tStat.st_mtime==tCfgStat.st_mtime && tStat.st_ctime==tCfgStat.st_ctime )
			return false;

	uCfgCRC32 = uCRC32;
	tCfgStat = tStat;

	return true;
}

void CleanLoadedConfig ()
{
	if ( g_bCleanLoadedConfig )
		g_dConfig.Reset();
}

// add or remove persistent pools to hosts
void InitPersistentPool()
{
	if ( !g_iPersistentPoolSize )
	{
		ClosePersistentSockets();
		return;
	}

	Dashboard::GetActiveHosts ().Apply ( [] ( HostDashboard_t *&pHost ) {
		if ( !pHost->m_pPersPool )
			pHost->m_pPersPool = new PersistentConnectionsPool_c;
		pHost->m_pPersPool->ReInit ( g_iPersistentPoolSize );
	} );
}


// refactor!
// make possible changing of an index role. I.e., was template, became distr, as example.
//
// Reloading called always from same thread (so, for now not need to be th-safe for itself)
// ServiceMain() -> TickHead() -> CheckRotate() -> ReloadIndexSettings().
static void ReloadIndexSettings ( CSphConfigParser & tCP ) REQUIRES ( MainThread, g_tRotateConfigMutex )
	REQUIRES ( MainThread )
{
	// collect names of all existing local indexes as assumed for deletion
	SmallStringHash_T<bool> dLocalToDelete;
	for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next (); )
	{
		// skip JSON indexes or indexes belong to cluster - no need to manage them
		ServedDescRPtr_c pServed ( it.Get() );
		if ( ServedDesc_t::IsCluster ( pServed ) )
			continue;

		dLocalToDelete.Add ( true, it.GetName() );
	}

	// collect names of all existing distr indexes as assumed for deletion
	SmallStringHash_T<bool> dDistrToDelete;
	for ( RLockedDistrIt_c it ( g_pDistIndexes ); it.Next (); )
		dDistrToDelete.Add ( true, it.GetName () );

	const CSphConfig &hConf = tCP.m_tConf;
	if ( !hConf.Exists ("index") )
	{
		sphWarning ( "No indexes found in config came to rotation. Abort reloading");
		return;
	}

	for ( const auto& dIndex : hConf["index"] )
	{
		const auto & sIndexName = dIndex.first;
		const CSphConfigSection & hIndex = dIndex.second;
		IndexType_e eNewType = TypeOfIndexConfig ( hIndex.GetStr ( "type", nullptr ) );

		if ( eNewType==IndexType_e::ERROR_ )
			continue;

		bool bReplaceLocal = false;
		auto pServedIndex = GetServed ( sIndexName );
		if ( pServedIndex )
		{
			bool bGotLocal = false;
			bool bReconfigure = false;
			// Rlock scope for settings compare
			{
				ServedDescRPtr_c pServedRLocked ( pServedIndex );
				if ( pServedRLocked )
				{
					bGotLocal = true;
					bReplaceLocal = ( eNewType!=pServedRLocked->m_eType );
					if ( !bReplaceLocal )
					{
						ServedDesc_t tDesc;
						ConfigureLocalIndex ( &tDesc, hIndex );
						bReconfigure = ( tDesc.m_iExpandKeywords!=pServedRLocked->m_iExpandKeywords ||
							tDesc.m_tFileAccessSettings!=pServedRLocked->m_tFileAccessSettings ||
							tDesc.m_bPreopen!=pServedRLocked->m_bPreopen ||
							tDesc.m_sGlobalIDFPath!=pServedRLocked->m_sGlobalIDFPath );
						bReconfigure |= ( pServedRLocked->m_eType!=IndexType_e::TEMPLATE
								&& hIndex.Exists ( "path" )
								&& hIndex["path"].strval ()!=pServedRLocked->m_sIndexPath );
					}
				}

			}
			// Wlock'ing only in case settings changed
			if ( bReconfigure )
			{
				ServedDescWPtr_c pServedWLocked ( pServedIndex );
				ConfigureLocalIndex ( pServedWLocked, hIndex );
				if ( pServedWLocked->m_eType!=IndexType_e::TEMPLATE
					&& hIndex.Exists ( "path" )
					&& hIndex["path"].strval ()!=pServedWLocked->m_sIndexPath )
				{
					pServedWLocked->m_sNewPath = hIndex["path"].strval ();

					ServedDesc_t tDesc;
					tDesc.m_sIndexPath = pServedWLocked->m_sIndexPath;
					tDesc.m_sNewPath = pServedWLocked->m_sNewPath;
					tDesc.m_eType = pServedWLocked->m_eType;
					if ( !PreloadKlistTarget ( tDesc, CheckIndexHeaderRotate(tDesc), tDesc.m_dKilllistTargets ) )
						tDesc.m_dKilllistTargets.Reset();

					g_dPostIndexes.AddUniq ( RefCountedRefPtr_t ( new ServedIndex_c ( tDesc )), sIndexName );
				}
			}

			if ( bGotLocal && !bReplaceLocal )
			{
				dLocalToDelete[sIndexName] = false;
				continue;
			}
			if ( bGotLocal && bReplaceLocal && eNewType==IndexType_e::TEMPLATE )
			{
				dLocalToDelete[sIndexName] = false;
			}
		}

		auto pDistrIndex = GetDistr ( sIndexName );
		if ( pDistrIndex && eNewType==IndexType_e::DISTR )
		{
			DistributedIndexRefPtr_t ptrIdx ( new DistributedIndex_t );
			ConfigureDistributedIndex ( *ptrIdx, sIndexName.cstr (), hIndex );

			if ( !ptrIdx->IsEmpty () )
				g_pDistIndexes->AddOrReplace ( ptrIdx, sIndexName );
			else
				sphWarning ( "index '%s': no valid local/remote indexes in distributed index; using last valid definition", sIndexName.cstr () );

			dDistrToDelete[sIndexName] = false;
			continue;
		}

		// if index was distr and switched to local - it will be added into queue as local.
		// if it was local and now distr - it is already added as distr.
		// dupes will vanish with deletion pass then.

		// fixme: report errors
		CSphString sError;
		ESphAddIndex eType = AddIndex ( sIndexName.cstr(), hIndex, bReplaceLocal, sError );

		// If we've added disabled index (i.e. one which need to be prealloced first)
		// instead of existing distributed - we don't have to delete last right now,
		// let rotate it later.
		if ( eType==ADD_DSBLED )
		{
			auto pAddedIndex = GetDisabled ( sIndexName.cstr() );
			assert ( pAddedIndex );
			ServedDescWPtr_c pWlockedDisabled ( pAddedIndex );
			if ( pWlockedDisabled->m_eType==IndexType_e::PLAIN )
				pWlockedDisabled->m_bOnlyNew = true;
			if ( pDistrIndex )
				dDistrToDelete[sIndexName] = false;
		}
	}

	for ( dDistrToDelete.IterateStart (); dDistrToDelete.IterateNext ();)
		if ( dDistrToDelete.IterateGet() )
			g_pDistIndexes->Delete (dDistrToDelete.IterateGetKey ());

	for ( dLocalToDelete.IterateStart (); dLocalToDelete.IterateNext (); )
		if ( dLocalToDelete.IterateGet () )
			g_pLocalIndexes->Delete ( dLocalToDelete.IterateGetKey () );

	InitPersistentPool();
}


struct IndexWithPriority_t
{
	CSphString	m_sIndex;
	StrVec_t	m_dKilllistTargets;
	int			m_iPriority {-1};
	int			m_nReferences {0};
};


static void SetIndexPriority ( IndexWithPriority_t & tIndex, int iPriority, const SmallStringHash_T<IndexWithPriority_t> & tIndexHash )
{
	tIndex.m_iPriority = Max ( tIndex.m_iPriority, iPriority );
	for ( const auto & i : tIndex.m_dKilllistTargets )
	{
		IndexWithPriority_t * pIdx = tIndexHash(i);
		if ( pIdx )
			SetIndexPriority ( *pIdx, iPriority+1, tIndexHash );
	}
}


static void CalcRotationPriorities() REQUIRES ( MainThread, g_tRotateThreadMutex )
{
	SmallStringHash_T<IndexWithPriority_t> tIndexesToRotate;

	// don't need wlock since rotation thread is already waiting for g_tRotateThreadMutex
	for ( RLockedServedIt_c it ( &g_dPostIndexes ); it.Next (); )
	{
		auto pIndex = it.Get();
		assert(pIndex);

		ServedDescRPtr_c pRlockedServedPtr(pIndex);

		// check for rt/percolate. they don't need killlist_target
		if ( !ServedDesc_t::IsMutable ( pRlockedServedPtr ) && g_pLocalIndexes->Contains ( it.GetName() ) )
		{
			ServedDescRPtr_c pDesc ( GetServed ( it.GetName() ) );

			IndexWithPriority_t tToRotate;
			tToRotate.m_sIndex = it.GetName();
			tToRotate.m_dKilllistTargets = pRlockedServedPtr->m_dKilllistTargets;

			tIndexesToRotate.Add ( tToRotate, it.GetName() );
		}
	}

	// set priorities
	for ( tIndexesToRotate.IterateStart (); tIndexesToRotate.IterateNext (); )
		for ( const auto & i : tIndexesToRotate.IterateGet().m_dKilllistTargets )
		{
			IndexWithPriority_t * pIdx = tIndexesToRotate(i);
			if ( pIdx )
				pIdx->m_nReferences++;
		}

	// start with the least-referenced index
	IndexWithPriority_t * pMin;
	do
	{
		pMin = nullptr;
		for ( tIndexesToRotate.IterateStart (); tIndexesToRotate.IterateNext (); )
		{
			IndexWithPriority_t & tIdx = tIndexesToRotate.IterateGet();
			if ( tIdx.m_iPriority==-1 && ( !pMin || tIdx.m_nReferences<pMin->m_nReferences ) )
				pMin = &tIdx;
		}

		if ( pMin )
			SetIndexPriority ( *pMin, 0, tIndexesToRotate );
	}
	while ( pMin );
	
	// copy priorities to disabled indexes
	for ( tIndexesToRotate.IterateStart (); tIndexesToRotate.IterateNext (); )
	{
		const IndexWithPriority_t & tIdx = tIndexesToRotate.IterateGet();
		assert ( tIdx.m_iPriority>=0 );
		ServedDescWPtr_c pDisabled ( GetDisabled ( tIdx.m_sIndex ) );
		if ( pDisabled )
			pDisabled->m_iRotationPriority = tIdx.m_iPriority;
	}
}

// ServiceMain() -> TickHead() -> CheckRotate() -> CheckIndexesForSeamless()
static void CheckIndexesForSeamless() REQUIRES ( MainThread )
{
	// check what indexes need to be rotated
	SmallStringHash_T<bool> dNotCapableForRotation;
	for ( RLockedServedIt_c it ( &g_dPostIndexes ); it.Next(); )
	{
		ServedDescRPtr_c rLocked ( it.Get() );
		const CSphString & sIndex = it.GetName ();
		assert ( rLocked );
		if ( !rLocked->m_pIndex )
			continue;

		if ( CheckIndexHeaderRotate(*rLocked)==RotateFrom_e::NONE && !ServedDesc_t::IsMutable (rLocked) )
		{
			dNotCapableForRotation.Add ( true, sIndex );
			sphLogDebug ( "Index %s (%s) is not capable for seamless rotate. Skipping", sIndex.cstr ()
				, rLocked->m_sIndexPath.cstr () );
		}
	}

	if ( dNotCapableForRotation.GetLength () )
	{
		sphWarning ( "internal error: non-empty queue on a rotation cycle start, got %d elements"
			, dNotCapableForRotation.GetLength () );
		for ( dNotCapableForRotation.IterateStart (); dNotCapableForRotation.IterateNext (); )
		{
			// need also to remove empty description from local index list
			g_pLocalIndexes->DeleteIfNull ( dNotCapableForRotation.IterateGetKey() );
			g_dPostIndexes.Delete ( dNotCapableForRotation.IterateGetKey() );
			sphWarning ( "queue[] = %s", dNotCapableForRotation.IterateGetKey().cstr () );
		}
	}

	if ( g_dPostIndexes.GetLength () )
	{
		InvokeRotation();
		return;
	}

	sphWarning ( "nothing to rotate after SIGHUP" );
	g_bInRotate = false;
}


static void DoGreedyRotation() REQUIRES ( MainThread )
{
	assert ( !g_bSeamlessRotate );
	ScRL_t tRotateConfigMutex { g_tRotateConfigMutex };

	while ( g_dPostIndexes.GetLength() )
	{
		CSphString sDisabledIndex, sError;
		ServedIndexRefPtr_c pDisabledIndex ( nullptr );

		// find the index with the best (least) rotation priority
		int iMinPriority = INT_MAX;
		for ( RLockedServedIt_c it ( &g_dPostIndexes ); it.Next(); )
		{
			ServedDescRPtr_c rLocked(it.Get());

			if ( rLocked->m_iRotationPriority < iMinPriority )
			{
				sDisabledIndex = it.GetName();
				pDisabledIndex = it.Get();
				iMinPriority = rLocked->m_iRotationPriority;
			}
		}

		assert ( pDisabledIndex );

		// special processing for plain old rotation cur.new.* -> cur.*
		if ( !ServedDescRPtr_c( pDisabledIndex)->m_pIndex )
		{
			auto pRotating = GetServed ( sDisabledIndex );
			assert ( pRotating );
			ServedDescWPtr_c pWRotating ( pRotating );
			assert ( pWRotating->m_eType==IndexType_e::PLAIN );
			if ( !RotateIndexGreedy ( *pWRotating, sDisabledIndex.cstr(), sError ) )
			{
				sphWarning ( "%s", sError.cstr () );
				g_pLocalIndexes->Delete ( sDisabledIndex );
			}
		}
		else
		{
			ServedDescWPtr_c pWlockedServedDisabledPtr ( pDisabledIndex );

			assert ( pWlockedServedDisabledPtr->m_pIndex );
			assert ( g_pLocalIndexes->Contains ( sDisabledIndex ) );

			StrVec_t dWarnings;

			// prealloc RT and percolate here
			if ( ServedDesc_t::IsMutable ( pWlockedServedDisabledPtr ) )
			{
				pWlockedServedDisabledPtr->m_bOnlyNew = false;
				if ( PreallocNewIndex ( *pWlockedServedDisabledPtr, &g_pCfg.m_tConf["index"][sDisabledIndex], sDisabledIndex.cstr(), dWarnings, sError ) )
					g_pLocalIndexes->AddOrReplace ( pDisabledIndex, sDisabledIndex );
				else
				{
					sphWarning ( "index '%s': %s - NOT SERVING", sDisabledIndex.cstr(), sError.cstr() );
					g_pLocalIndexes->DeleteIfNull ( sDisabledIndex );
				}
			}
			else if ( pWlockedServedDisabledPtr->m_eType==IndexType_e::PLAIN )
			{
				bool bWasAdded = pWlockedServedDisabledPtr->m_bOnlyNew;
				bool bOk = RotateIndexGreedy ( *pWlockedServedDisabledPtr, sDisabledIndex.cstr(), sError );
				if ( !bOk )
					sphWarning ( "index '%s': %s - NOT SERVING", sDisabledIndex.cstr(), sError.cstr() );

				if ( bWasAdded && bOk && !sphFixupIndexSettings ( pWlockedServedDisabledPtr->m_pIndex, g_pCfg.m_tConf["index"][sDisabledIndex], g_bStripPath, nullptr, dWarnings, sError ) )
				{
					sphWarning ( "index '%s': %s - NOT SERVING", sDisabledIndex.cstr(), sError.cstr() );
					bOk = false;
				}

				if ( bOk )
				{
					pWlockedServedDisabledPtr->m_pIndex->Preread();
					g_pLocalIndexes->AddOrReplace ( pDisabledIndex, sDisabledIndex );
				} else
					g_pLocalIndexes->DeleteIfNull ( sDisabledIndex );
			}

			for ( const auto & i : dWarnings )
				sphWarning ( "index '%s': %s", sDisabledIndex.cstr(), i.cstr() );
		}

		g_dPostIndexes.Delete ( sDisabledIndex );
		g_pDistIndexes->Delete ( sDisabledIndex ); // postponed delete of same-named distributed (if any)
	}

	g_dPostIndexes.ReleaseAndClear ();
	g_bInRotate = false;
	RotateGlobalIdf ();
	sphInfo ( "rotating finished" );
}


// ServiceMain() -> TickHead() -> CheckRotate()
static void CheckRotate () REQUIRES ( MainThread ) EXCLUDES ( g_tRotateThreadMutex )
{
	// do we need to rotate now? If no sigHUP received, or if we are already rotating - no.
	if ( !g_bNeedRotate || g_bInRotate || IsConfigless() )
		return;

	g_bInRotate = true; // ok, another rotation cycle just started
	g_bNeedRotate = false; // which therefore clears any previous HUP signals

	sphLogDebug ( "CheckRotate invoked" );

	// fixme! disabled hash protected by g_bInRotate exclusion,
	// what about more explicit protection?
	g_dPostIndexes.ReleaseAndClear ();
	{
		ScWL_t dRotateConfigMutexWlocked { g_tRotateConfigMutex };
		if ( LoadAndCheckConfig () || g_bReloadForced )
		{
			sphInfo( "Config changed (read %d chars)", g_dConfig.GetLength());
			if ( !g_dConfig.IsEmpty() && g_pCfg.ReParse ( g_sConfigFile.cstr (), g_dConfig.begin ()))
				ReloadIndexSettings ( g_pCfg );
			else
				sphWarning ( "failed to parse config file '%s': %s; using previous settings",
						g_sConfigFile.cstr (), TlsMsg::szError ());
		}
		CleanLoadedConfig();
		g_bReloadForced = false;
	}

	{
		// we want rotation thread to wait until we're done with our new rotation priorities
		ScopedMutex_t tBlockRotations ( g_tRotateThreadMutex );

		// special pass for 'simple' rotation (i.e. *.new to current)
		for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next (); )
		{
			auto pIdx = it.Get();
			if ( !pIdx )
				continue;
			ServedDescRPtr_c rLocked ( pIdx );
			const CSphString & sIndex = it.GetName();
			assert ( rLocked->m_pIndex );
			if ( rLocked->m_eType==IndexType_e::PLAIN && CheckIndexHeaderRotate ( *rLocked )==RotateFrom_e::NEW )
			{
				ServedDesc_t tDesc;
				tDesc.m_sIndexPath = rLocked->m_sIndexPath;
				tDesc.m_sNewPath = rLocked->m_sNewPath;
				tDesc.m_eType = rLocked->m_eType;
				if ( !PreloadKlistTarget ( tDesc, RotateFrom_e::NEW, tDesc.m_dKilllistTargets ) )
					tDesc.m_dKilllistTargets.Reset();

				g_dPostIndexes.AddUniq ( RefCountedRefPtr_t ( new ServedIndex_c ( tDesc )), sIndex );
			}
		}

		CalcRotationPriorities();
	}

	if ( g_bSeamlessRotate )
		CheckIndexesForSeamless();
	else
		DoGreedyRotation();
}


void CheckReopenLogs () REQUIRES ( MainThread )
{
	if ( !g_bGotSigusr1 )
		return;

	// reopen searchd log
	if ( g_iLogFile>=0 && !g_bLogTty )
	{
		int iFD = ::open ( g_sLogFile.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
		if ( iFD<0 )
		{
			sphWarning ( "failed to reopen log file '%s': %s", g_sLogFile.cstr(), strerrorm(errno) );
		} else
		{
			::close ( g_iLogFile );
			g_iLogFile = iFD;
			g_bLogTty = ( isatty ( g_iLogFile )!=0 );
			LogChangeMode ( g_iLogFile, g_iLogFileMode );
			sphInfo ( "log reopened" );
		}
	}

	// reopen query log
	if ( !g_bQuerySyslog && g_iQueryLogFile!=g_iLogFile && g_iQueryLogFile>=0 && !isatty ( g_iQueryLogFile ) )
	{
		int iFD = ::open ( g_sQueryLogFile.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
		if ( iFD<0 )
		{
			sphWarning ( "failed to reopen query log file '%s': %s", g_sQueryLogFile.cstr(), strerrorm(errno) );
		} else
		{
			::close ( g_iQueryLogFile );
			g_iQueryLogFile = iFD;
			LogChangeMode ( g_iQueryLogFile, g_iLogFileMode );
			sphInfo ( "query log reopened" );
		}
	}

	g_bGotSigusr1 = 0;
}

#if !USE_WINDOWS
#define WINAPI
#else

SERVICE_STATUS			g_ss;
SERVICE_STATUS_HANDLE	g_ssHandle;


void MySetServiceStatus ( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint )
{
	static DWORD dwCheckPoint = 1;

	if ( dwCurrentState==SERVICE_START_PENDING )
		g_ss.dwControlsAccepted = 0;
	else
		g_ss.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

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
		case SERVICE_CONTROL_SHUTDOWN:
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
	snprintf ( sBuf, sizeof(sBuf), "code=%d, error=", uErr );

	auto iLen = (int) strlen(sBuf);
	if ( !FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErr, 0, sBuf+iLen, sizeof(sBuf)-iLen, NULL ) ) // FIXME? force US-english langid?
		snprintf ( sBuf+iLen, sizeof(sBuf)-iLen, "(no message)" );

	return sBuf;
}


SC_HANDLE ServiceOpenManager ()
{
	SC_HANDLE hSCM = OpenSCManager (
		NULL,						// local computer
		NULL,						// ServicesActive database
		SC_MANAGER_ALL_ACCESS );	// full access rights

	if ( hSCM==NULL )
		sphFatal ( "OpenSCManager() failed: %s", WinErrorInfo() );

	return hSCM;
}


void AppendArg ( char * sBuf, int iBufLimit, const char * sArg )
{
	char * sBufMax = sBuf + iBufLimit - 2; // reserve place for opening space and trailing zero
	sBuf += strlen(sBuf);

	if ( sBuf>=sBufMax )
		return;

	auto iArgLen = (int) strlen(sArg);
	bool bQuote = false;
	for ( int i=0; i<iArgLen && !bQuote; i++ )
		if ( sArg[i]==' ' || sArg[i]=='"' )
			bQuote = true;

	*sBuf++ = ' ';
	if ( !bQuote )
	{
		// just copy
		int iToCopy = Min ( sBufMax-sBuf, iArgLen );
		memcpy ( sBuf, sArg, iToCopy );
		sBuf[iToCopy] = '\0';

	} else
	{
		// quote
		sBufMax -= 2; // reserve place for quotes
		if ( sBuf>=sBufMax )
			return;

		*sBuf++ = '"';
		while ( sBuf<sBufMax && *sArg )
		{
			if ( *sArg=='"' )
			{
				// quote
				if ( sBuf<sBufMax-1 )
				{
					*sBuf++ = '\\';
					*sBuf++ = *sArg++;
				}
			} else
			{
				// copy
				*sBuf++ = *sArg++;
			}
		}
		*sBuf++ = '"';
		*sBuf++ = '\0';
	}
}


void ServiceInstall ( int argc, char ** argv )
{
	if ( g_bService )
		return;

	sphInfo ( "Installing service..." );

	char szBinary[MAX_PATH];
	if ( !GetModuleFileName ( NULL, szBinary, MAX_PATH ) )
		sphFatal ( "GetModuleFileName() failed: %s", WinErrorInfo() );

	char szPath[MAX_PATH];
	szPath[0] = '\0';

	AppendArg ( szPath, sizeof(szPath), szBinary );
	AppendArg ( szPath, sizeof(szPath), "--ntservice" );
	for ( int i=1; i<argc; i++ )
		if ( strcmp ( argv[i], "--install" ) )
			AppendArg ( szPath, sizeof(szPath), argv[i] );

	SC_HANDLE hSCM = ServiceOpenManager ();
	SC_HANDLE hService = CreateService (
		hSCM,							// SCM database
		g_sServiceName,					// name of service
		g_sServiceName,					// service name to display
		SERVICE_ALL_ACCESS,				// desired access
		SERVICE_WIN32_OWN_PROCESS,		// service type
		SERVICE_AUTO_START,				// start type
		SERVICE_ERROR_NORMAL,			// error control type
		szPath+1,						// path to service's binary
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
		sphInfo ( "Service '%s' installed successfully.", g_sServiceName );
	}

	CSphString sDesc;
	sDesc.SetSprintf ( "%s-%s", g_sServiceName, szMANTICORE_VERSION );

	SERVICE_DESCRIPTION tDesc;
	tDesc.lpDescription = (LPSTR) sDesc.cstr();
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
	SC_HANDLE hService = OpenService ( hSCM, g_sServiceName, DELETE );
	if ( !hService )
	{
		CloseServiceHandle ( hSCM );
		sphFatal ( "OpenService() failed: %s", WinErrorInfo() );
	}

	// do delete
	bool bRes = !!DeleteService ( hService );
	CloseServiceHandle ( hService );
	CloseServiceHandle ( hSCM );

	if ( !bRes )
		sphFatal ( "DeleteService() failed: %s", WinErrorInfo() );
	else
		sphInfo ( "Service '%s' deleted successfully.", g_sServiceName );
}
#endif // USE_WINDOWS


void ShowHelp ()
{
	fprintf ( stdout,
		"Usage: searchd [OPTIONS]\n"
		"\n"
		"Options are:\n"
		"-h, --help\t\tdisplay this help message\n"
		"-v\t\t\tdisplay version information\n"
		"-c, --config <file>\tread configuration from specified file\n"
		"\t\t\t(default is manticore.conf)\n"
		"--stop\t\t\tsend SIGTERM to currently running searchd\n"
		"--stopwait\t\tsend SIGTERM and wait until actual exit\n"
		"--status\t\tget ant print status variables\n"
		"\t\t\t(PID is taken from pid_file specified in config file)\n"
		"--iostats\t\tlog per-query io stats\n"
		"--cpustats\t\tlog per-query cpu stats\n"
#if USE_WINDOWS
		"--install\t\tinstall as Windows service\n"
		"--delete\t\tdelete Windows service\n"
		"--servicename <name>\tuse given service name (default is 'searchd')\n"
		"--ntservice\t\tinternal option used to invoke a Windows service\n"
#endif
		"--strip-path\t\tstrip paths from stopwords, wordforms, exceptions\n"
		"\t\t\tand other file names stored in the index header\n"
		"--replay-flags=<OPTIONS>\n"
		"\t\t\textra binary log replay options (current options \n"
		"\t\t\tare 'accept-desc-timestamp' and 'ignore-open-errors')\n"
		"\n"
		"Debugging options are:\n"
		"--console\t\trun in console mode (do not fork, do not log to files)\n"
		"-p, --port <port>\tlisten on given port (overrides config setting)\n"
		"-l, --listen <spec>\tlisten on given address, port or path (overrides\n"
		"\t\t\tconfig settings)\n"
		"-i, --index <index>\tonly serve given index(es)\n"
#if !USE_WINDOWS
		"--nodetach\t\tdo not detach into background\n"
#endif
		"--logdebug, --logdebugv, --logdebugvv\n"
		"\t\t\tenable additional debug information logging\n"
		"\t\t\t(with different verboseness)\n"
		"--pidfile\t\tforce using the PID file (useful with --console)\n"
		"--safetrace\t\tonly use system backtrace() call in crash reports\n"
		"--coredump\t\tsave core dump file on crash\n"
		"\n"
		"Examples:\n"
		"searchd --config /usr/local/sphinx/etc/manticore.conf\n"
#if USE_WINDOWS
		"searchd --install --config c:\\sphinx\\manticore.conf\n"
#endif
		);
}


void InitSharedBuffer ()
{
	static CSphLargeBuffer<SharedData_t, true> g_dShared;
	CSphString sError;
	if ( !g_dShared.Alloc ( 1, sError ) )
		sphDie ( "failed to allocate shared buffer (msg=%s)", sError.cstr() );

	// reset
	g_pShared = g_dShared.GetWritePtr();
	g_pShared->m_bDaemonAtShutdown = false;
	g_pShared->m_bHaveTTY = false;
}


#if USE_WINDOWS
BOOL WINAPI CtrlHandler ( DWORD )
{
	if ( !g_bService )
		sphInterruptNow();
	return TRUE;
}
#endif

static char g_sNameBuf[512] = { 0 };
static char g_sPid[30] = { 0 };


#if !USE_WINDOWS
// returns 'true' only once - at the very start, to show it beatiful way.
bool SetWatchDog ( int iDevNull ) REQUIRES ( MainThread )
{
	InitSharedBuffer ();

	// Fork #1 - detach from controlling terminal
	switch ( fork() )
	{
		case -1:
			// error
			sphFatalLog ( "fork() failed (reason: %s)", strerrorm ( errno ) );
			exit ( 1 );
		case 0:
			// daemonized child - or new and free watchdog :)
			break;

		default:
			// tty-controlled parent
			while ( !g_pShared->m_bHaveTTY )
				sphSleepMsec ( 100 );

			exit ( 0 );
	}

	// became the session leader
	if ( setsid()==-1 )
	{
		sphFatalLog ( "setsid() failed (reason: %s)", strerrorm ( errno ) );
		exit ( 1 );
	}

	// Fork #2 - detach from session leadership (may be not necessary, however)
	switch ( fork() )
	{
		case -1:
			// error
			sphFatalLog ( "fork() failed (reason: %s)", strerrorm ( errno ) );
			exit ( 1 );
		case 0:
			// daemonized child - or new and free watchdog :)
			break;

		default:
			// tty-controlled parent
			exit ( 0 );
	}

	// save path to our binary
	g_sNameBuf[::readlink ( "/proc/self/exe", g_sNameBuf, 511 )] = 0;

	// now we are the watchdog. Let us fork the actual process
	enum class EFork { Startup, Disabled, Restart } eReincarnate = EFork::Startup;
	bool bShutdown = false;
	bool bStreamsActive = true;
	int iChild = 0;
	g_iParentPID = getpid();
	assert ( g_pShared );
	while (true)
	{
		if ( eReincarnate!=EFork::Disabled )
			iChild = fork();

		if ( iChild==-1 )
		{
			sphFatalLog ( "fork() failed during watchdog setup (error=%s)", strerrorm(errno) );
			exit ( 1 );
		}

		// child process; return true to show that we have to reload everything
		if ( iChild==0 )
		{
			atexit ( &ReleaseTTYFlag );
			return bStreamsActive;
		}

		// parent process, watchdog
		// close the io files
		if ( bStreamsActive )
		{
			close ( STDIN_FILENO );
			close ( STDOUT_FILENO );
			close ( STDERR_FILENO );
			dup2 ( iDevNull, STDIN_FILENO );
			dup2 ( iDevNull, STDOUT_FILENO );
			dup2 ( iDevNull, STDERR_FILENO );
			bStreamsActive = false;
		}

		if ( eReincarnate!=EFork::Disabled )
		{
			sphInfo ( "watchdog: main process %d forked ok", iChild );
			sprintf ( g_sPid, "%d", iChild);
		}

		SetSignalHandlers();

		eReincarnate = EFork::Disabled;
		int iPid, iStatus;
		while ( ( iPid = wait ( &iStatus ) )>0 )
		{
			const char * sWillRestart = ( g_pShared->m_bDaemonAtShutdown ? "will not be restarted (daemon is shutting down)" : "will be restarted" );

			assert ( iPid==iChild );
			if ( WIFEXITED ( iStatus ) )
			{
				int iExit = WEXITSTATUS ( iStatus );
				if ( iExit==2 || iExit==6 ) // really crash
				{
					sphInfo ( "watchdog: main process %d crashed via CRASH_EXIT (exit code %d), %s", iPid, iExit, sWillRestart );
					eReincarnate = EFork::Restart;
				} else
				{
					sphInfo ( "watchdog: main process %d exited cleanly (exit code %d), shutting down", iPid, iExit );
					bShutdown = true;
				}
			} else if ( WIFSIGNALED ( iStatus ) )
			{
				int iSig = WTERMSIG ( iStatus );
				const char * sSig = NULL;
				if ( iSig==SIGINT )
					sSig = "SIGINIT";
				else if ( iSig==SIGTERM )
					sSig = "SIGTERM";
				else if ( WATCHDOG_SIGKILL && iSig==SIGKILL )
					sSig = "SIGKILL";
				if ( sSig )
				{
					sphInfo ( "watchdog: main process %d killed cleanly with %s, shutting down", iPid, sSig );
					bShutdown = true;
				} else
				{
					if ( WCOREDUMP ( iStatus ) )
						sphInfo ( "watchdog: main process %d killed dirtily with signal %d, core dumped, %s",
							iPid, iSig, sWillRestart );
					else
						sphInfo ( "watchdog: main process %d killed dirtily with signal %d, %s",
							iPid, iSig, sWillRestart );
					eReincarnate = EFork::Restart;
				}
			} else if ( WIFSTOPPED ( iStatus ) )
				sphInfo ( "watchdog: main process %d stopped with signal %d", iPid, WSTOPSIG ( iStatus ) );
#ifdef WIFCONTINUED
			else if ( WIFCONTINUED ( iStatus ) )
				sphInfo ( "watchdog: main process %d resumed", iPid );
#endif
		}

		if ( iPid==-1 )
		{
			if ( g_bGotSigusr1 )
			{
				g_bGotSigusr1 = 0;
				sphInfo ( "watchdog: got USR1, performing dump of child's stack" );
				sphDumpGdb ( g_iLogFile, g_sNameBuf, g_sPid );
			} else
				sphInfo ( "watchdog: got error %d, %s", errno, strerrorm ( errno ));
		}

		if ( bShutdown || sphInterrupted() || g_pShared->m_bDaemonAtShutdown )
		{
			exit ( 0 );
		}
	}
}
#else
const int		WIN32_PIPE_BUFSIZE=32;
#endif // !USE_WINDOWS

/// check for incoming signals, and react on them
void CheckSignals () REQUIRES ( MainThread )
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
		sphInfo ( "caught SIGHUP (seamless=%d, in_rotate=%d, need_rotate=%d)", (int)g_bSeamlessRotate, (int)g_bInRotate, (int)g_bNeedRotate );
		g_bNeedRotate = true;
		g_bGotSighup = 0;
	}

	if ( sphInterrupted() )
	{
		sphInfo ( "caught SIGTERM, shutting down" );
		Shutdown ();
		exit ( 0 );
	}

#if USE_WINDOWS

	BYTE dPipeInBuf [ WIN32_PIPE_BUFSIZE ];
	DWORD nBytesRead = 0;
	BOOL bSuccess = ReadFile ( g_hPipe, dPipeInBuf, WIN32_PIPE_BUFSIZE, &nBytesRead, NULL );
	if ( nBytesRead > 0 && bSuccess )
	{
		for ( DWORD i=0; i<nBytesRead; i++ )
		{
			switch ( dPipeInBuf[i] )
			{
			case 0:
				g_bGotSighup = 1;
				break;

			case 1:
				sphInterruptNow();
				if ( g_bService )
					g_bServiceStop = true;
				break;
			}
		}

		DisconnectNamedPipe ( g_hPipe );
		ConnectNamedPipe ( g_hPipe, NULL );
	}
#endif
}


void ShowProgress ( const CSphIndexProgress * pProgress, bool bPhaseEnd )
{
	assert ( pProgress );
	if ( bPhaseEnd )
	{
		fprintf ( stdout, "\r                                                            \r" );
	} else
	{
		fprintf ( stdout, "%s\r", pProgress->BuildMessage() );
	}
	fflush ( stdout );
}

void TickHead () REQUIRES ( MainThread )
{
	CheckSignals ();
	CheckLeaks ();
	CheckReopenLogs ();
	Threads::CallCoroutine ( CheckRotate );

	sphInfo ( nullptr ); // flush dupes
#if USE_WINDOWS
	// at windows there is no signals that interrupt sleep
	// need to sleep less to make main loop more responsible
	int tmSleep = 100;
#else
	int tmSleep = 500;
#endif
	sphSleepMsec ( tmSleep );
}

bool g_bVtune = false;
int64_t g_tmStarted = 0;

static int	g_iNetWorkers = 1;

/////////////////////////////////////////////////////////////////////////////
// DAEMON OPTIONS
/////////////////////////////////////////////////////////////////////////////

static const QueryParser_i * PercolateQueryParserFactory ( bool bJson )
{
	if ( bJson )
		return sphCreateJsonQueryParser();
	else
		return sphCreatePlainQueryParser();
}


static void ParsePredictedTimeCosts ( const char * p )
{
	// yet another mini-parser!
	// ident=value [, ident=value [...]]
	sph::ParseKeyValues ( p, [] (CSphString&& sIdent, CSphString&& sValue)
	{
		// bind value
		if ( sIdent=="skip" )
			g_iPredictorCostSkip = atoi ( sValue.cstr ());
		else if ( sIdent=="doc" )
			g_iPredictorCostDoc = atoi ( sValue.cstr ());
		else if ( sIdent=="hit" )
			g_iPredictorCostHit = atoi ( sValue.cstr ());
		else if ( sIdent=="match" )
			g_iPredictorCostMatch = atoi ( sValue.cstr ());
		else
			sphDie ( "predicted_time_costs: unknown identifier '%s' (known ones are skip, doc, hit, match)",
					 sIdent.cstr ());
	});
}

// read system TFO settings and init g_ITFO according to it.
/* From https://www.kernel.org/doc/Documentation/networking/ip-sysctl.txt
 * possible bitmask values are:

	  0x1: (client) enables sending data in the opening SYN on the client.
	  0x2: (server) enables the server support, i.e., allowing data in
			a SYN packet to be accepted and passed to the
			application before 3-way handshake finishes.
	  0x4: (client) send data in the opening SYN regardless of cookie
			availability and without a cookie option.
	0x200: (server) accept data-in-SYN w/o any cookie option present.
	0x400: (server) enable all listeners to support Fast Open by
			default without explicit TCP_FASTOPEN socket option.

 Actually we interested only in first 2 bits.
 */
static void CheckSystemTFO ()
{
#if defined (MSG_FASTOPEN)
	char sBuf[20] = { 0 };
	g_iTFO = TFO_ABSENT;

	FILE * fp = fopen ( "/proc/sys/net/ipv4/tcp_fastopen", "rb" );
	if ( !fp )
	{
		sphWarning ( "TCP fast open unavailable (can't read /proc/sys/net/ipv4/tcp_fastopen, unsupported kernel?)" );
		return;
	}

	auto szResult = fgets ( sBuf, 20, fp );
	fclose ( fp );
	if ( !szResult )
		return;

	g_iTFO = atoi ( szResult );
#else
	g_iTFO = 3; // suggest it is available.
#endif
}


void ConfigureSearchd ( const CSphConfig & hConf, bool bOptPIDFile, bool bTestMode ) REQUIRES ( MainThread )
{
	if ( !hConf.Exists ( "searchd" ) || !hConf["searchd"].Exists ( "searchd" ) )
		sphFatal ( "'searchd' config section not found in '%s'", g_sConfigFile.cstr () );

	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];
	sphCheckDuplicatePaths ( hConf );

	if ( bOptPIDFile )
		if ( !hSearchd ( "pid_file" ) )
			sphFatal ( "mandatory option 'pid_file' not found in 'searchd' section" );

	// read_timeout is now deprecated
	g_iReadTimeoutS = hSearchd.GetSTimeS ( "read_timeout", 5);

	// network_timeout overrides read_timeout
	g_iReadTimeoutS = hSearchd.GetSTimeS ( "network_timeout", g_iReadTimeoutS );
	g_iWriteTimeoutS = g_iReadTimeoutS;

	g_iClientQlTimeoutS = hSearchd.GetSTimeS( "sphinxql_timeout", 900);
	g_iClientTimeoutS = hSearchd.GetSTimeS ( "client_timeout", 300 );

	g_iMaxConnection = hSearchd.GetInt ( "max_connections", g_iMaxConnection );
	g_iThreads = hSearchd.GetInt ( "threads", sphCpuThreadsCount() );
	SetMaxChildrenThreads ( g_iThreads );
	g_iThdQueueMax = hSearchd.GetInt ( "jobs_queue_size", g_iThdQueueMax );

	g_iPersistentPoolSize = hSearchd.GetInt ("persistent_connections_limit");
	g_bPreopenIndexes = hSearchd.GetBool ( "preopen_indexes" );
	sphSetUnlinkOld ( hSearchd.GetBool ( "unlink_old" ) );
	g_iExpansionLimit = hSearchd.GetInt ( "expansion_limit" );

	// initialize buffering settings
	SetUnhintedBuffer ( hSearchd.GetSize( "read_unhinted", DEFAULT_READ_UNHINTED ) );
	int iReadBuffer = hSearchd.GetSize ( "read_buffer", DEFAULT_READ_BUFFER );
	g_tDefaultFA.m_iReadBufferDocList = hSearchd.GetSize ( "read_buffer_docs", iReadBuffer );
	g_tDefaultFA.m_iReadBufferHitList = hSearchd.GetSize ( "read_buffer_hits", iReadBuffer );
	g_tDefaultFA.m_eDoclist = GetFileAccess( hSearchd, "access_doclists", true, FileAccess_e::FILE );
	g_tDefaultFA.m_eHitlist = GetFileAccess( hSearchd, "access_hitlists", true, FileAccess_e::FILE );

	g_tDefaultFA.m_eAttr = FileAccess_e::MMAP_PREREAD;
	g_tDefaultFA.m_eBlob = FileAccess_e::MMAP_PREREAD;

	bool bOnDiskAttrs = hSearchd.GetBool ( "ondisk_attrs_default", false );
	bool bOnDiskPools = hSearchd.GetStr ( "ondisk_attrs_default" )=="pool";
	if ( bOnDiskAttrs || bOnDiskPools )
		g_tDefaultFA.m_eAttr = FileAccess_e::MMAP;

	if ( bOnDiskPools )
		g_tDefaultFA.m_eBlob = FileAccess_e::MMAP;

	g_tDefaultFA.m_eAttr = GetFileAccess( hSearchd, "access_plain_attrs", false, g_tDefaultFA.m_eAttr );
	g_tDefaultFA.m_eBlob = GetFileAccess( hSearchd, "access_blob_attrs", false, g_tDefaultFA.m_eBlob );

	if ( hSearchd("subtree_docs_cache") )
		g_iMaxCachedDocs = hSearchd.GetSize ( "subtree_docs_cache", g_iMaxCachedDocs );

	if ( hSearchd("subtree_hits_cache") )
		g_iMaxCachedHits = hSearchd.GetSize ( "subtree_hits_cache", g_iMaxCachedHits );

	if ( hSearchd("seamless_rotate") )
		g_bSeamlessRotate = ( hSearchd["seamless_rotate"].intval()!=0 );

	if ( hSearchd ( "grouping_in_utc" ) )
	{
		g_bGroupingInUtc = (hSearchd["grouping_in_utc"].intval ()!=0);
		setGroupingInUtc ( g_bGroupingInUtc );
	}

	// sha1 password hash for shutdown action
	g_sShutdownToken = hSearchd.GetStr ("shutdown_token");

	if ( !g_bSeamlessRotate && g_bPreopenIndexes && !bTestMode )
		sphWarning ( "preopen_indexes=1 has no effect with seamless_rotate=0" );

	SetAttrFlushPeriod ( hSearchd.GetUsTime64S ( "attr_flush_period", 0 ));
	g_iMaxPacketSize = hSearchd.GetSize ( "max_packet_size", g_iMaxPacketSize );
	g_iMaxFilters = hSearchd.GetInt ( "max_filters", g_iMaxFilters );
	g_iMaxFilterValues = hSearchd.GetInt ( "max_filter_values", g_iMaxFilterValues );
	g_iMaxBatchQueries = hSearchd.GetInt ( "max_batch_queries", g_iMaxBatchQueries );
	g_iDistThreads = hSearchd.GetInt ( "dist_threads", g_iDistThreads );
	sphSetThrottling ( hSearchd.GetInt ( "rt_merge_iops", 0 ), hSearchd.GetSize ( "rt_merge_maxiosize", 0 ) );
	g_iPingIntervalUs = hSearchd.GetUsTime64Ms ( "ha_ping_interval", 1000000 );
	g_uHAPeriodKarmaS = hSearchd.GetSTimeS ( "ha_period_karma", 60 );
	g_iQueryLogMinMs = hSearchd.GetMsTimeMs ( "query_log_min_msec", g_iQueryLogMinMs );
	g_iAgentConnectTimeoutMs = hSearchd.GetMsTimeMs ( "agent_connect_timeout", g_iAgentConnectTimeoutMs );
	g_iAgentQueryTimeoutMs = hSearchd.GetMsTimeMs ( "agent_query_timeout", g_iAgentQueryTimeoutMs );
	g_iAgentRetryDelayMs = hSearchd.GetMsTimeMs ( "agent_retry_delay", g_iAgentRetryDelayMs );
	if ( g_iAgentRetryDelayMs > MAX_RETRY_DELAY )
		sphWarning ( "agent_retry_delay %d exceeded max recommended %d", g_iAgentRetryDelayMs, MAX_RETRY_DELAY );
	g_iAgentRetryCount = hSearchd.GetInt ( "agent_retry_count", g_iAgentRetryCount );
	if ( g_iAgentRetryCount > MAX_RETRY_COUNT )
		sphWarning ( "agent_retry_count %d exceeded max recommended %d", g_iAgentRetryCount, MAX_RETRY_COUNT );
	g_tmWait = hSearchd.GetInt ( "net_wait_tm", g_tmWait );
	g_iThrottleAction = hSearchd.GetInt ( "net_throttle_action", g_iThrottleAction );
	g_iThrottleAccept = hSearchd.GetInt ( "net_throttle_accept", g_iThrottleAccept );
	g_iNetWorkers = hSearchd.GetInt ( "net_workers", g_iNetWorkers );
	g_iNetWorkers = Max ( g_iNetWorkers, 1 );
	CheckSystemTFO();
	if ( g_iTFO!=TFO_ABSENT && hSearchd.GetInt ( "listen_tfo", 1 )==0 )
	{
		g_iTFO &= ~TFO_LISTEN;
	}
	if ( hSearchd ( "collation_libc_locale" ) )
	{
		auto sLocale = hSearchd.GetStr ( "collation_libc_locale" );
		if ( !setlocale ( LC_COLLATE, sLocale.cstr() ) )
			sphWarning ( "setlocale failed (locale='%s')", sLocale.cstr() );
	}

	if ( hSearchd ( "collation_server" ) )
	{
		CSphString sCollation = hSearchd.GetStr ( "collation_server" );
		CSphString sError;
		g_eCollation = sphCollationFromName ( sCollation, &sError );
		if ( !sError.IsEmpty() )
			sphWarning ( "%s", sError.cstr() );
	}

	if ( hSearchd("thread_stack") ) // fixme! rename? That is limit for stack of the coro, not of the thread!
	{
		int iThreadStackSizeMin = 65536;
		int iThreadStackSizeMax = 8*1024*1024;
		int iStackSize = hSearchd.GetSize ( "thread_stack", iThreadStackSizeMin );
		if ( iStackSize<iThreadStackSizeMin || iStackSize>iThreadStackSizeMax )
			sphWarning ( "thread_stack %d out of bounds (64K..8M); clamped", iStackSize );

		iStackSize = Min ( iStackSize, iThreadStackSizeMax );
		iStackSize = Max ( iStackSize, iThreadStackSizeMin );
		Threads::SetMaxCoroStackSize ( iStackSize );
	}

	if ( hSearchd("predicted_time_costs") )
		ParsePredictedTimeCosts ( hSearchd["predicted_time_costs"].cstr() );

	if ( hSearchd("shutdown_timeout") )
		g_iShutdownTimeoutUs = hSearchd.GetUsTime64S ( "shutdown_timeout", 3000000);

	g_iDocstoreCache = hSearchd.GetSize ( "docstore_cache_size", 16777216 );

	if ( hSearchd.Exists ( "max_open_files" ) )
	{
#if HAVE_GETRLIMIT & HAVE_SETRLIMIT
		auto uLimit = ( rlim_t ) hSearchd["max_open_files"].intval ();
		bool bMax = hSearchd["max_open_files"].strval ()=="max";
		if ( !uLimit && !bMax )
			sphWarning ( "max_open_files is %d, expected positive value; ignored", (int) uLimit );
		else
		{
			struct rlimit dRlimit;
			if ( 0!=getrlimit ( RLIMIT_NOFILE, &dRlimit ) )
				sphWarning ( "Failed to getrlimit (RLIMIT_NOFILE), error %d: %s", errno, strerrorm ( errno ) );
			else
			{
				auto uPrevLimit = dRlimit.rlim_cur;
				if ( bMax )
					uLimit = dRlimit.rlim_max;
				dRlimit.rlim_cur = Min ( dRlimit.rlim_max, uLimit );
				if ( 0!=setrlimit ( RLIMIT_NOFILE, &dRlimit ) )
					sphWarning ( "Failed to setrlimit on %d, error %d: %s", (int)uLimit, errno, strerrorm ( errno ) );
				else
					sphInfo ( "Set max_open_files to %d (previous was %d), hardlimit is %d.",
						(int)uLimit, (int)uPrevLimit, (int)dRlimit.rlim_max );
			}
		}
#else
		sphWarning ("max_open_files defined, but this binary don't know about setrlimit() function");
#endif
	}

	QcacheStatus_t s = QcacheGetStatus();
	s.m_iMaxBytes = hSearchd.GetSize64 ( "qcache_max_bytes", s.m_iMaxBytes );
	s.m_iThreshMs = hSearchd.GetMsTimeMs ( "qcache_thresh_msec", s.m_iThreshMs );
	s.m_iTtlS = hSearchd.GetSTimeS ( "qcache_ttl_sec", s.m_iTtlS );
	QcacheSetup ( s.m_iMaxBytes, s.m_iThreshMs, s.m_iTtlS );

	// hostname_lookup = {config_load | request}
	g_bHostnameLookup = ( hSearchd.GetStr ( "hostname_lookup" ) == "request" );

	CSphVariant * pLogMode = hSearchd ( "query_log_mode" );
	if ( pLogMode && !pLogMode->strval().IsEmpty() )
	{
		errno = 0;
		int iMode = strtol ( pLogMode->strval().cstr(), NULL, 8 );
		int iErr = errno;
		if ( iErr==ERANGE || iErr==EINVAL )
		{
			sphWarning ( "query_log_mode invalid value (value=%o, error=%s); skipped", iMode, strerrorm(iErr) );
		} else
		{
			g_iLogFileMode = iMode;
		}
	}
	if ( hSearchd ( "server_id" ) )
	{
		g_iServerID = hSearchd.GetInt ( "server_id", g_iServerID );
		g_bServerID = true;
		const int iServerMask = 0x7f;
		if ( g_iServerID>iServerMask )
		{
			g_iServerID &= iServerMask;
			sphWarning ( "server_id out of range 0 - 127, clamped to %d", g_iServerID );
		}
	}
	g_sMySQLVersion = hSearchd.GetStr ( "mysql_version_string", szMANTICORE_VERSION );
}

// ServiceMain -> ConfigureAndPreload -> ConfigureAndPreloadIndex
// ServiceMain -> ConfigureAndPreload -> ConfigureAndPreloadInt -> PreloadIndex -> ConfigureAndPreloadIndex
// from any another thread:
// CSphinxqlSession::Execute -> HandleMysqlImportTable -> AddExistingIndexInt -> PreloadIndex -> ConfigureAndPreloadIndex
ESphAddIndex ConfigureAndPreloadIndex ( const CSphConfigSection & hIndex, const char * sIndexName,
		StrVec_t & dWarnings, CSphString & sError )
{
	ESphAddIndex eAdd = AddIndex ( sIndexName, hIndex, false, sError );

	// local plain, rt, percolate added, but need to be at least prealloced before they could work.
	if ( eAdd!=ADD_DSBLED )
		return eAdd;

	auto pHandle = GetDisabled ( sIndexName );
	ServedDescWPtr_c pJustAddedLocalWl ( pHandle );

	fprintf ( stdout, "precaching index '%s'\n", sIndexName );
	fflush ( stdout );

	IndexFiles_c dJustAddedFiles ( pJustAddedLocalWl->m_sIndexPath );
	bool bPreloadOk = true;
	if ( dJustAddedFiles.HasAllFiles ( ".new" ) )
	{
		pJustAddedLocalWl->m_bOnlyNew = !dJustAddedFiles.HasAllFiles();
		if ( RotateIndexGreedy ( *pJustAddedLocalWl, sIndexName, sError ) )
		{
			CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( CreateFilenameBuilder(sIndexName) );
			StrVec_t dWarnings;
			bPreloadOk = sphFixupIndexSettings ( pJustAddedLocalWl->m_pIndex, hIndex, g_bStripPath, pFilenameBuilder.Ptr(), dWarnings, sError );
		} else
		{
			pJustAddedLocalWl->m_bOnlyNew = false;
			dWarnings.Add(sError);
			bPreloadOk = PreallocNewIndex ( *pJustAddedLocalWl, &hIndex, sIndexName, dWarnings, sError );
		}

	} else
		bPreloadOk = PreallocNewIndex ( *pJustAddedLocalWl, &hIndex, sIndexName, dWarnings, sError );

	if ( !bPreloadOk )
	{
		g_pLocalIndexes->DeleteIfNull ( sIndexName );
		return ADD_ERROR;
	}

	// finally add the index to the hash of enabled.
	g_pLocalIndexes->AddOrReplace ( pHandle, sIndexName );

	if ( !pJustAddedLocalWl->m_sGlobalIDFPath.IsEmpty() && !sph::PrereadGlobalIDF ( pJustAddedLocalWl->m_sGlobalIDFPath, sError ) )
		dWarnings.Add ( "global IDF unavailable - IGNORING" );

	return eAdd;
}

// invoked once on start from ServiceMain (actually it creates the hashes)
// ServiceMain -> ConfigureAndPreload
static void ConfigureAndPreload ( const CSphConfig & hConf, const StrVec_t & dOptIndexes ) REQUIRES (MainThread)
{
	int iCounter = 0;
	int iValidIndexes = 0;
	int64_t tmLoad = -sphMicroTimer();

	g_dPostIndexes.ReleaseAndClear ();

	if ( hConf.Exists ( "index" ) )
	{
		hConf["index"].IterateStart ();
		while ( hConf["index"].IterateNext() )
		{
			const CSphConfigSection & hIndex = hConf["index"].IterateGet();
			const char * sIndexName = hConf["index"].IterateGetKey().cstr();

			if ( !dOptIndexes.IsEmpty() && !dOptIndexes.FindFirst ( [&] ( const CSphString &rhs )	{ return rhs.EqN ( sIndexName ); } ) )
				continue;

			StrVec_t dWarnings;
			CSphString sError;
			ESphAddIndex eAdd = ConfigureAndPreloadIndex ( hIndex, sIndexName, dWarnings, sError );
			for ( const auto & i : dWarnings )
				sphWarning ( "index '%s': %s", sIndexName, i.cstr() );

			if ( eAdd==ADD_ERROR )
				sphWarning ( "index '%s': %s - NOT SERVING", sIndexName, sError.cstr() );

			iValidIndexes += ( eAdd!=ADD_ERROR ? 1 : 0 );
			iCounter +=  ( eAdd==ADD_DSBLED ? 1 : 0 );
		}
	}

	ConfigureAndPreloadInt ( iValidIndexes, iCounter );

	// we don't have any more unprocessed disabled indexes during startup
	g_dPostIndexes.ReleaseAndClear ();

	InitPersistentPool();

	for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next(); )
	{
		ServedDescRPtr_c tServed ( it.Get () );
		if ( tServed )
		{
			CSphString sWarning, sError;
			if ( !ApplyIndexKillList ( tServed->m_pIndex, sWarning, sError, true ) )
				sphWarning ( "index '%s': error applying killlist: %s", tServed->m_pIndex->GetName(), sError.cstr() );

			if ( sWarning.Length() )
				sphWarning ( "%s", sWarning.cstr() );
		}
	}

	tmLoad += sphMicroTimer();
	if ( !iValidIndexes )
		sphLogDebug ( "no valid indexes to serve" );
	else
		fprintf ( stdout, "precached %d indexes in %0.3f sec\n", iCounter, float(tmLoad)/1000000 );
}

void OpenDaemonLog ( const CSphConfigSection & hSearchd, bool bCloseIfOpened=false )
{
	// create log
		const char * sLog = "searchd.log";
		if ( hSearchd.Exists ( "log" ) )
		{
			if ( hSearchd["log"]=="syslog" )
			{
#if !USE_SYSLOG
				if ( g_iLogFile<0 )
				{
					g_iLogFile = STDOUT_FILENO;
					sphWarning ( "failed to use syslog for logging. You have to reconfigure --with-syslog and rebuild the daemon!" );
					sphInfo ( "will use default file 'searchd.log' for logging." );
				}
#else
				g_bLogSyslog = true;
#endif
			} else
			{
				sLog = hSearchd["log"].cstr();
			}
		}

		umask ( 066 );
		if ( bCloseIfOpened && g_iLogFile!=STDOUT_FILENO )
		{
			close ( g_iLogFile );
			g_iLogFile = STDOUT_FILENO;
		}
		if ( !g_bLogSyslog )
		{
			g_iLogFile = open ( sLog, O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
			if ( g_iLogFile<0 )
			{
				g_iLogFile = STDOUT_FILENO;
				sphFatal ( "failed to open log file '%s': %s", sLog, strerrorm(errno) );
			}
			LogChangeMode ( g_iLogFile, g_iLogFileMode );
		}

		g_sLogFile = sLog;
		g_bLogTty = isatty ( g_iLogFile )!=0;
}

static void SetUidShort ( bool bTestMode )
{
	const int iServerMask = 0x7f;
	int iServerId = g_iServerID;
	uint64_t uStartedSec = 0;

	if ( !bTestMode )
	{
		// server id as high part of counter
		if ( g_bServerID )
		{
			iServerId = g_iServerID;
		} else
		{
			CSphString sMAC = GetMacAddress();
			sphLogDebug ( "MAC address %s for uuid-short server_id", sMAC.cstr() );
			if ( sMAC.IsEmpty() )
			{
				DWORD uSeed = sphRand();
				sMAC.SetSprintf ( "%u", uSeed );
				sphWarning ( "failed to get MAC address, using random number %s", sMAC.cstr()  );
			}
			// fold MAC into 1 byte
			iServerId = Pearson8 ( (const BYTE *)sMAC.cstr(), sMAC.Length() );
			iServerId &= iServerMask;
		}

		// start time Unix timestamp as middle part of counter
		uStartedSec = sphMicroTimer() / 1000000;
		// base timestamp is 01 May of 2019
		const uint64_t uBaseSec = 1556668800;
		if ( uStartedSec>uBaseSec )
			uStartedSec -= uBaseSec;
	} else
	{
		// need constant seed across all environments for tests
		uStartedSec = 100000;
		iServerId = g_iServerID;
	}
	UidShortSetup ( iServerId, (int)uStartedSec );
}

namespace { // static

// implement '--stop' and '--stopwait' (connect and stop another instance by pid file from config)
void StopOrStopWaitAnother ( CSphVariant * v, bool bWait ) REQUIRES ( MainThread )
{
	if ( !v )
		sphFatal ( "stop: option 'pid_file' not found in '%s' section 'searchd'", g_sConfigFile.cstr () );

	const char * sPid = v->cstr (); // shortcut
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

	int iWaitTimeout = g_iShutdownTimeoutUs + 100000;

#if USE_WINDOWS
	bool bTerminatedOk = false;

	char szPipeName[64];
	snprintf ( szPipeName, sizeof(szPipeName), "\\\\.\\pipe\\searchd_%d", iPid );

	HANDLE hPipe = INVALID_HANDLE_VALUE;

	while ( hPipe==INVALID_HANDLE_VALUE )
	{
		hPipe = CreateFile ( szPipeName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );

		if ( hPipe==INVALID_HANDLE_VALUE )
		{
			if ( GetLastError()!=ERROR_PIPE_BUSY )
			{
				fprintf ( stdout, "WARNING: could not open pipe (GetLastError()=%d)\n", GetLastError () );
				break;
			}

			if ( !WaitNamedPipe ( szPipeName, iWaitTimeout/1000 ) )
			{
				fprintf ( stdout, "WARNING: could not open pipe (GetLastError()=%d)\n", GetLastError () );
				break;
			}
		}
	}

	if ( hPipe!=INVALID_HANDLE_VALUE )
	{
		DWORD uWritten = 0;
		BYTE uWrite = 1;
		BOOL bResult = WriteFile ( hPipe, &uWrite, 1, &uWritten, NULL );
		if ( !bResult )
			fprintf ( stdout, "WARNING: failed to send SIGHTERM to searchd (pid=%d, GetLastError()=%d)\n", iPid, GetLastError () );

		bTerminatedOk = !!bResult;

		CloseHandle ( hPipe );
	}

	if ( bTerminatedOk )
	{
		sphInfo ( "stop: successfully terminated pid %d", iPid );
		exit ( 0 );
	} else
		sphFatal ( "stop: error terminating pid %d", iPid );
#else
	CSphString sPipeName;
	int iPipeCreated = -1;
	int fdPipe = -1;
	if ( bWait )
	{
		sPipeName = GetNamedPipeName ( iPid );
		::unlink ( sPipeName.cstr () ); // avoid garbage to pollute us
		iPipeCreated = mkfifo ( sPipeName.cstr(), 0666 );
		if ( iPipeCreated!=-1 )
			fdPipe = ::open ( sPipeName.cstr(), O_RDONLY | O_NONBLOCK );

		if ( iPipeCreated==-1 )
			sphWarning ( "mkfifo failed (path=%s, err=%d, msg=%s); will NOT wait", sPipeName.cstr(), errno, strerrorm(errno) );
		else if ( fdPipe<0 )
			sphWarning ( "open failed (path=%s, err=%d, msg=%s); will NOT wait", sPipeName.cstr(), errno, strerrorm(errno) );
	}

	if ( kill ( iPid, SIGTERM ) )
		sphFatal ( "stop: kill() on pid %d failed: %s", iPid, strerrorm(errno) );
	else
		sphInfo ( "stop: successfully sent SIGTERM to pid %d", iPid );

	int iExitCode = ( bWait && ( iPipeCreated==-1 || fdPipe<0 ) ) ? 1 : 0;
	bool bHandshake = true;
	if ( bWait && fdPipe>=0 )
		while ( true )
		{
			int iReady = sphPoll ( fdPipe, iWaitTimeout );

			// error on wait
			if ( iReady<0 )
			{
				iExitCode = 3;
				sphWarning ( "stopwait%s error '%s'", ( bHandshake ? " handshake" : " " ), strerrorm(errno) );
				break;
			}

			// timeout
			if ( iReady==0 )
			{
				if ( !bHandshake )
					continue;

				iExitCode = 1;
				break;
			}

			// reading data
			DWORD uStatus = 0;
			int iRead = ::read ( fdPipe, &uStatus, sizeof(DWORD) );
			if ( iRead!=sizeof(DWORD) )
			{
				sphWarning ( "stopwait read fifo error '%s'", strerrorm(errno) );
				iExitCode = 3; // stopped demon crashed during stop
				break;
			} else
			{
				iExitCode = ( uStatus==1 ? 0 : 2 ); // uStatus == 1 - AttributeSave - ok, other values - error
			}

			if ( !bHandshake )
				break;

			bHandshake = false;
		}
	::unlink ( sPipeName.cstr () ); // is ok on linux after it is opened.

	if ( fdPipe>=0 )
		::close ( fdPipe );

	exit ( iExitCode );
#endif
}
} // static namespace

int WINAPI ServiceMain ( int argc, char **argv ) REQUIRES (!MainThread)
{
	ScopedRole_c thMain (MainThread);
	g_bLogTty = isatty ( g_iLogFile )!=0;

#ifdef USE_VTUNE
	__itt_pause ();
#endif // USE_VTUNE
	g_tmStarted = sphMicroTimer();

#if USE_WINDOWS
	CSphVector<char *> dArgs;
	if ( g_bService )
	{
		g_ssHandle = RegisterServiceCtrlHandler ( g_sServiceName, ServiceControl );
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

	char szPipeName[64];
	snprintf ( szPipeName, sizeof(szPipeName), "\\\\.\\pipe\\searchd_%d", getpid() );
	g_hPipe = CreateNamedPipe ( szPipeName, PIPE_ACCESS_INBOUND,
		PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
		PIPE_UNLIMITED_INSTANCES, 0, WIN32_PIPE_BUFSIZE, NMPWAIT_NOWAIT, NULL );
	ConnectNamedPipe ( g_hPipe, NULL );
#endif

	tzset();

	if ( !g_bService )
		fprintf ( stdout, "%s", szMANTICORE_BANNER );

	const char* sEndian = sphCheckEndian();
	if ( sEndian )
		sphDie ( "%s", sEndian );

	//////////////////////
	// parse command line
	//////////////////////

	CSphConfig		conf;
	bool			bOptStop = false;
	bool			bOptStopWait = false;
	bool			bOptStatus = false;
	bool			bOptPIDFile = false;
	StrVec_t		dOptIndexes; // indexes explicitly pointed in cmdline options

	int				iOptPort = 0;
	bool			bOptPort = false;

	CSphString		sOptListen;
	bool			bOptListen = false;
	bool			bTestMode = false;
	bool			bOptDebugQlog = true;
	bool			bForcedPreread = false;
	bool			bNewCluster = false;
	bool			bNewClusterForce = false;
	const char*		szCmdConfigFile = nullptr;

	DWORD			uReplayFlags = 0;

	#define OPT(_a1,_a2)	else if ( !strcmp(argv[i],_a1) || !strcmp(argv[i],_a2) )
	#define OPT1(_a1)		else if ( !strcmp(argv[i],_a1) )

	int i;
	for ( i=1; i<argc; i++ )
	{
		// handle non-options
		if ( argv[i][0]!='-' )		break;

		// handle no-arg options
		OPT ( "-h", "--help" )		{ ShowHelp(); return 0; }
		OPT ( "-?", "--?" )			{ ShowHelp(); return 0; }
		OPT1 ( "-v" )				{ return 0; }
		OPT1 ( "--console" )		{ g_bOptNoLock = true; g_bOptNoDetach = true; bTestMode = true; }
		OPT1 ( "--stop" )			bOptStop = true;
		OPT1 ( "--stopwait" )		{ bOptStop = true; bOptStopWait = true; }
		OPT1 ( "--status" )			bOptStatus = true;
		OPT1 ( "--pidfile" )		bOptPIDFile = true;
		OPT1 ( "--iostats" )		g_bIOStats = true;
		OPT1 ( "--cpustats" )		g_bCpuStats = true;
#if USE_WINDOWS
		OPT1 ( "--install" )		{ if ( !g_bService ) { ServiceInstall ( argc, argv ); return 0; } }
		OPT1 ( "--delete" )			{ if ( !g_bService ) { ServiceDelete (); return 0; } }
		OPT1 ( "--ntservice" )		{} // it's valid but handled elsewhere
#else
		OPT1 ( "--nodetach" )		g_bOptNoDetach = true;
#endif
		OPT1 ( "--logdebug" )		g_eLogLevel = Max ( g_eLogLevel, SPH_LOG_DEBUG );
		OPT1 ( "--logdebugv" )		g_eLogLevel = Max ( g_eLogLevel, SPH_LOG_VERBOSE_DEBUG );
		OPT1 ( "--logdebugvv" )		g_eLogLevel = Max ( g_eLogLevel, SPH_LOG_VERY_VERBOSE_DEBUG );
		OPT1 ( "--logreplication" )	g_eLogLevel = Max ( g_eLogLevel, SPH_LOG_RPL_DEBUG );
		OPT1 ( "--safetrace" )		g_bSafeTrace = true;
		OPT1 ( "--test" )			{ g_bWatchdog = false; bTestMode = true; } // internal option, do NOT document
		OPT1 ( "--test-thd-pool" )	{ g_bWatchdog = false; bTestMode = true; } // internal option, do NOT document
		OPT1 ( "--strip-path" )		g_bStripPath = true;
		OPT1 ( "--vtune" )			g_bVtune = true;
		OPT1 ( "--noqlog" )			bOptDebugQlog = false;
		OPT1 ( "--force-preread" )	bForcedPreread = true;
		OPT1 ( "--coredump" )		g_bCoreDump = true;
		OPT1 ( "--new-cluster" )	bNewCluster = true;
		OPT1 ( "--new-cluster-force" )	bNewClusterForce = true;

		// FIXME! add opt=(csv)val handling here
		OPT1 ( "--replay-flags=accept-desc-timestamp" )		uReplayFlags |= SPH_REPLAY_ACCEPT_DESC_TIMESTAMP;
		OPT1 ( "--replay-flags=ignore-open-errors" )			uReplayFlags |= SPH_REPLAY_IGNORE_OPEN_ERROR;

		// handle 1-arg options
		else if ( (i+1)>=argc )		break;
		OPT ( "-c", "--config" )	szCmdConfigFile = argv[++i];
		OPT ( "-p", "--port" )		{ bOptPort = true; iOptPort = atoi ( argv[++i] ); }
		OPT ( "-l", "--listen" )	{ bOptListen = true; sOptListen = argv[++i]; }
		OPT ( "-i", "--index" )		dOptIndexes.Add ( argv[++i] );
#if USE_WINDOWS
		OPT1 ( "--servicename" )	++i; // it's valid but handled elsewhere
#endif

		// handle unknown options
		else
			break;
	}
	if ( i!=argc )
		sphFatal ( "malformed or unknown option near '%s'; use '-h' or '--help' to see available options.", argv[i] );

	g_sConfigFile = sphGetConfigFile ( szCmdConfigFile );
#if USE_WINDOWS
	// init WSA on Windows
	// we need to do it this early because otherwise gethostbyname() from config parser could fail
	WSADATA tWSAData;
	int iStartupErr = WSAStartup ( WINSOCK_VERSION, &tWSAData );
	if ( iStartupErr )
		sphFatal ( "failed to initialize WinSock2: %s", sphSockError ( iStartupErr ) );

	if ( !LoadExFunctions () )
		sphFatal ( "failed to initialize extended socket functions: %s", sphSockError ( iStartupErr ) );


	// i want my windows sessions to log onto stdout
	// both in Debug and Release builds
	if ( !g_bService )
		g_bOptNoDetach = true;

#ifndef NDEBUG
	// i also want my windows debug builds to skip locking by default
	// NOTE, this also skips log files!
	g_bOptNoLock = true;
#endif
#endif

	if ( !bOptPIDFile )
		bOptPIDFile = !g_bOptNoLock;

	// check port and listen arguments early
	if ( !g_bOptNoDetach && ( bOptPort || bOptListen ) )
	{
		sphWarning ( "--listen and --port are only allowed in --console debug mode; switch ignored" );
		bOptPort = bOptListen = false;
	}

	if ( bOptPort )
	{
		if ( bOptListen )
			sphFatal ( "please specify either --port or --listen, not both" );

		CheckPort ( iOptPort );
	}

	/////////////////////
	// parse config file
	/////////////////////

	LoadAndCheckConfig ();
	sphInfo( "using config file '%s' (%d chars)...", g_sConfigFile.cstr(), g_dConfig.GetLength());
	// do parse
	// don't aqcuire wlock, since we're in single main thread here.
	if ( !g_pCfg.Parse ( g_sConfigFile.scstr(), g_dConfig.begin() ) )
		sphFatal ( "failed to parse config file '%s': %s", g_sConfigFile.cstr (), TlsMsg::szError() );
	CleanLoadedConfig();

	const CSphConfig & hConf = g_pCfg.m_tConf;

	if ( !hConf.Exists ( "searchd" ) || !hConf["searchd"].Exists ( "searchd" ) )
		sphFatal ( "'searchd' config section not found in '%s'", g_sConfigFile.cstr () );

	const CSphConfigSection & hSearchdpre = hConf["searchd"]["searchd"];

	CSphString sError;
	if ( !sphInitCharsetAliasTable ( sError ) )
		sphFatal ( "failed to init charset alias table: %s", sError.cstr() );

	////////////////////////
	// stop running searchd
	////////////////////////

	if ( bOptStop )
	{
		StopOrStopWaitAnother ( hSearchdpre ( "pid_file" ), bOptStopWait );
		assert ( 0 && "StopOrStopWaitAnother should not return " );
		exit ( 0 );
	}

	////////////////////////////////
	// query running searchd status
	////////////////////////////////

	if ( bOptStatus )
	{
		QueryStatus ( hSearchdpre("listen") );
		exit ( 0 );
	}

	/////////////////////
	// configure searchd
	/////////////////////

	sphInitCJson();
	if ( LoadConfigInt ( hConf, g_sConfigFile, sError ) )
		g_bJsonConfigLoadedOk = true;
	else
		sphFatal ( "%s", sError.cstr() );

	ConfigureSearchd ( hConf, bOptPIDFile, bTestMode );
	sphConfigureCommon ( hConf ); // this also inits plugins now

	g_bWatchdog = hSearchdpre.GetInt ( "watchdog", g_bWatchdog )!=0;

	if ( g_iMaxPacketSize<128*1024 || g_iMaxPacketSize>128*1024*1024 )
		sphFatal ( "max_packet_size out of bounds (128K..128M)" );

	if ( g_iMaxFilters<1 || g_iMaxFilters>10240 )
		sphFatal ( "max_filters out of bounds (1..10240)" );

	if ( g_iMaxFilterValues<1 || g_iMaxFilterValues>10485760 )
		sphFatal ( "max_filter_values out of bounds (1..10485760)" );

	bool bVisualLoad = true;
	bool bWatched = false;
#if !USE_WINDOWS
	// Let us start watchdog right now, on foreground first.
	int iDevNull = open ( "/dev/null", O_RDWR );
	if ( g_bWatchdog && !g_bOptNoDetach )
	{
		bWatched = true;
		if ( !g_bOptNoLock )
			OpenDaemonLog ( hConf["searchd"]["searchd"] );
		bVisualLoad = SetWatchDog ( iDevNull );
		OpenDaemonLog ( hConf["searchd"]["searchd"], true ); // just the 'IT Happens' magic - switch off, then on.
	}
#endif

	// here we either since plain startup, either being resurrected (forked) by watchdog.
	// create the pid
	if ( bOptPIDFile )
	{
		g_sPidFile = hSearchdpre["pid_file"].cstr();

		g_iPidFD = ::open ( g_sPidFile.scstr(), O_CREAT | O_WRONLY, S_IREAD | S_IWRITE );
		if ( g_iPidFD<0 )
			sphFatal ( "failed to create pid file '%s': %s", g_sPidFile.scstr(), strerrorm(errno) );
	}
	if ( bOptPIDFile && !sphLockEx ( g_iPidFD, false ) )
		sphFatal ( "failed to lock pid file '%s': %s (searchd already running?)", g_sPidFile.scstr(), strerrorm(errno) );

	g_bPidIsMine = true;

	// Actions on resurrection
	if ( bWatched && !bVisualLoad && LoadAndCheckConfig () )
	{
		// reparse the config file
		sphInfo ( "Reloading the config (%d chars)", g_dConfig.GetLength() );

		// don't aqcuire wlock, since we're in single main thread here.
		if ( !g_pCfg.ReParse ( g_sConfigFile.cstr (), g_dConfig.begin () ) )
			sphFatal ( "failed to parse config file '%s': %s", g_sConfigFile.cstr (), TlsMsg::szError() );

		sphInfo ( "Reconfigure the daemon" );
		ConfigureSearchd ( hConf, bOptPIDFile, bTestMode );
	}
	CleanLoadedConfig();

	// hSearchdpre might be dead if we reloaded the config.
	CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];

	// handle my signals
	SetSignalHandlers ( g_bOptNoDetach );

	// create logs
	if ( !g_bOptNoLock )
	{
		// create log
		OpenDaemonLog ( hSearchd, true );

		// create query log if required
		if ( hSearchd.Exists ( "query_log" ) )
		{
			if ( hSearchd["query_log"]=="syslog" )
				g_bQuerySyslog = true;
			else
			{
				g_iQueryLogFile = open ( hSearchd["query_log"].cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
				if ( g_iQueryLogFile<0 )
					sphFatal ( "failed to open query log file '%s': %s", hSearchd["query_log"].cstr(), strerrorm(errno) );

				LogChangeMode ( g_iQueryLogFile, g_iLogFileMode );
			}
			g_sQueryLogFile = hSearchd["query_log"].cstr();
		}
	}

#if !USE_WINDOWS
	if ( !g_bOptNoDetach && !bWatched )
	{
		switch ( fork () )
		{
			case -1:
			// error
			sphFatalLog ( "fork() failed (reason: %s)", strerrorm ( errno ) );
			exit ( 1 );

			case 0:
			// daemonized child
			break;

			default:
			// tty-controlled parent
			exit ( 0 );
		}
	}
#endif

	// after next line executed we're in mt env, need to take rwlock accessing config.
	StartGlobalWorkPool ();

	// since that moment any 'fatal' will assume calling 'shutdown' function.
	sphSetDieCallback ( DieOrFatalWithShutdownCb );

	////////////////////
	// network startup
	////////////////////
	CSphVector<ListenerDesc_t> dListenerDescs;

	// command line arguments override config (but only in --console)
	if ( bOptListen )
	{
		auto tDesc = ParseListener ( sOptListen.cstr() );
		dListenerDescs.Add ( tDesc );
		AddGlobalListener ( tDesc );
	} else if ( bOptPort )
	{
		AddGlobalListener ( MakeAnyListener ( iOptPort ) );
	} else
	{
		// listen directives in configuration file
		for ( CSphVariant * v = hSearchd("listen"); v; v = v->m_pNext )
		{
			auto tDesc = ParseListener ( v->cstr () );
			dListenerDescs.Add ( tDesc );
			AddGlobalListener ( tDesc );
		}

		// default is to listen on our two ports
		if ( g_dListeners.IsEmpty() )
		{
			AddGlobalListener ( MakeAnyListener ( SPHINXAPI_PORT ) );
			AddGlobalListener ( MakeAnyListener ( SPHINXQL_PORT, Proto_e::MYSQL41 ) );
		}
	}

	SetServerSSLKeys ( hSearchd ( "ssl_cert" ), hSearchd ( "ssl_key" ), hSearchd ( "ssl_ca" ) );

	// set up ping service (if necessary) before loading indexes
	// (since loading ha-mirrors of distributed already assumes ping is usable).
	if ( g_iPingIntervalUs>0 )
		Ping::Start();

	ScheduleMallocTrim();

	// initialize timeouts since hook will use them
	auto iRtFlushPeriodUs = hSearchd.GetUsTime64S ( "rt_flush_period", 36000000000ll ); // 10h
	SetRtFlushPeriod ( Max ( iRtFlushPeriodUs, 10 * 1000000 ) );
	g_pLocalIndexes->SetAddOrReplaceHook ( HookSubscribeMutableFlush );

	//////////////////////
	// build indexes hash
	//////////////////////

	// configure and preload
	if ( bTestMode ) // pass this flag here prior to index config
		sphRTSetTestMode();

	StrVec_t dExactIndexes;
	for ( const auto &dOptIndex : dOptIndexes )
		sphSplit ( dExactIndexes, dOptIndex.cstr (), "," );

	SetPercolateQueryParserFactory ( PercolateQueryParserFactory );
	Threads::CallCoroutine ( [&hConf, &dExactIndexes] // clang doesn't need hConf to capture, but win/gcc fails, if not.
	{
		ScopedRole_c thMain ( MainThread );
		ConfigureAndPreload ( hConf, dExactIndexes );
	} );

	///////////
	// startup
	///////////

	ModifyDaemonPaths ( hSearchd );
	sphRTInit ( hSearchd, bTestMode, hConf("common") ? hConf["common"]("common") : nullptr );

	if ( hSearchd.Exists ( "snippets_file_prefix" ) )
		g_sSnippetsFilePrefix = hSearchd["snippets_file_prefix"].cstr();
	else
		g_sSnippetsFilePrefix.SetSprintf("%s/", sphGetCwd().scstr());

	auto sLogFormat = hSearchd.GetStr ( "query_log_format", "plain" );
	if ( sLogFormat=="sphinxql" )
		g_eLogFormat = LOG_FORMAT_SPHINXQL;
	else if ( sLogFormat=="plain" )
	{
		StrVec_t dParams;
		sphSplit ( dParams, sLogFormat.cstr() );
		for ( const auto& sParam : dParams )
		{
			if ( sParam=="sphinxql" )
				g_eLogFormat = LOG_FORMAT_SPHINXQL;
			else if ( sParam=="plain" )
				g_eLogFormat = LOG_FORMAT_PLAIN;
			else if ( sParam=="compact_in" )
				g_bLogCompactIn = true;
		}
	}
	if ( g_bLogCompactIn && g_eLogFormat==LOG_FORMAT_PLAIN )
		sphWarning ( "compact_in option only supported with query_log_format=sphinxql" );

	// prepare to detach
	if ( !g_bOptNoDetach )
	{
		ReleaseTTYFlag();

#if !USE_WINDOWS
		if ( !bWatched || bVisualLoad )
		{
			close ( STDIN_FILENO );
			close ( STDOUT_FILENO );
			close ( STDERR_FILENO );
			dup2 ( iDevNull, STDIN_FILENO );
			dup2 ( iDevNull, STDOUT_FILENO );
			dup2 ( iDevNull, STDERR_FILENO );
		}
#endif
	}

	if ( bOptPIDFile && !bWatched )
		sphLockUn ( g_iPidFD );

	sphRTConfigure ( hSearchd, bTestMode );
	SetUidShort ( bTestMode );
	InitDocstore ( g_iDocstoreCache );

	if ( bOptPIDFile )
	{
#if !USE_WINDOWS
		// re-lock pid
		// FIXME! there's a potential race here
		if ( !sphLockEx ( g_iPidFD, true ) )
			sphFatal ( "failed to re-lock pid file '%s': %s", g_sPidFile.scstr(), strerrorm(errno) );
#endif

		char sPid[16];
		snprintf ( sPid, sizeof(sPid), "%d\n", (int)getpid() );
		auto iPidLen = (int) strlen(sPid);

		sphSeek ( g_iPidFD, 0, SEEK_SET );
		if ( !sphWrite ( g_iPidFD, sPid, iPidLen ) )
			sphFatal ( "failed to write to pid file '%s' (errno=%d, msg=%s)", g_sPidFile.scstr(),
				errno, strerrorm(errno) );

		if ( ::ftruncate ( g_iPidFD, iPidLen ) )
			sphFatal ( "failed to truncate pid file '%s' (errno=%d, msg=%s)", g_sPidFile.scstr(),
				errno, strerrorm(errno) );
	}

#if USE_WINDOWS
	SetConsoleCtrlHandler ( CtrlHandler, TRUE );
#endif

	Threads::CallCoroutine( [bWatched] {
	StrVec_t dFailed;
	if ( !g_bOptNoDetach && !bWatched && !g_bService )
	{
		// re-lock indexes
		for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next(); )
		{
			sphWarning ( "Relocking %s", it.GetName ().cstr () );
			ServedDescRPtr_c pServed ( it.Get() );
			// obtain exclusive lock
			if ( !pServed )
				dFailed.Add ( it.GetName() );
			else if ( !pServed->m_pIndex->Lock() )
			{
				sphWarning ( "index '%s': lock: %s; INDEX UNUSABLE", it.GetName ().cstr(), pServed->m_pIndex->GetLastError().cstr() );
				dFailed.Add ( it.GetName());
			}
		}
		for ( const auto& sFailed : dFailed )
			g_pLocalIndexes->Delete ( sFailed );
	}
	});

	// if we're running in test console mode, dump queries to tty as well
	// unless we're explicitly asked not to!
	if ( hSearchd ( "query_log" ) && g_bOptNoLock && g_bOptNoDetach && bOptDebugQlog )
	{
		g_bQuerySyslog = false;
		g_bLogSyslog = false;
		g_iQueryLogFile = g_iLogFile;
	}

#if USE_SYSLOG
	if ( g_bLogSyslog || g_bQuerySyslog )
	{
		openlog ( "searchd", LOG_PID, LOG_DAEMON );
	}
#else
	if ( g_bQuerySyslog )
		sphFatal ( "Wrong query_log file! You have to reconfigure --with-syslog and rebuild daemon if you want to use syslog there." );
#endif

	/////////////////
	// serve clients
	/////////////////

#if USE_WINDOWS
	if ( g_bService )
		MySetServiceStatus ( SERVICE_RUNNING, NO_ERROR, 0 );
#endif

	// replay last binlog
	SmallStringHash_T<CSphIndex*> hIndexes;
	Threads::CallCoroutine ([&hIndexes]
	{
		for ( RLockedServedIt_c it ( g_pLocalIndexes ); it.Next(); ) // FIXME!!!
		{
			ServedDescRPtr_c pLocked ( it.Get () );
			if ( pLocked )
				hIndexes.Add ( pLocked->m_pIndex, it.GetName () );
		}
	});

	sphReplayBinlog ( hIndexes, uReplayFlags, DumpMemStat );
	hIndexes.Reset();

	// no need to create another cluster on restart by watchdog resurrection
	if ( bWatched && !bVisualLoad )
	{
		bNewCluster = false;
		bNewClusterForce = false;
	}
	
	StartRtBinlogFlushing();

	ScheduleFlushAttrs();

	g_tStats.m_uStarted = (DWORD)time(NULL);

	// threads mode
	if ( !InitSphinxqlState ( hSearchd.GetStr ( "sphinxql_state" ), sError ))
		sphWarning ( "sphinxql_state flush disabled: %s", sError.cstr ());

	ServeUserVars ();

	if ( bForcedPreread )
		Threads::CallCoroutine (DoPreread);
	else
		StartPreread();

	// almost ready, time to start listening
	g_iBacklog = hSearchd.GetInt ( "listen_backlog", g_iBacklog );
	for ( const auto& dListener : g_dListeners )
		if ( listen ( dListener.m_iSock, g_iBacklog )==-1 )
		{
			if ( sphSockGetErrno()==EADDRINUSE )
				sphFatal ( "listen() failed with EADDRINUSE. A listener with other UID on same address:port?");
			else
				sphFatal ( "listen() failed: %s", sphSockError () );
		}

	// net thread needs non-blocking sockets
	for ( const auto& dListener : g_dListeners )
	{
		if ( sphSetSockNB ( dListener.m_iSock )<0 )
		{
			sphWarning ( "sphSetSockNB() failed: %s", sphSockError() );
			sphSockClose ( dListener.m_iSock );
		}

#if defined (TCP_FASTOPEN)
		if ( ( g_iTFO!=TFO_ABSENT ) && ( g_iTFO & TFO_LISTEN ) )
		{
			int iOn = 1;
			if ( setsockopt ( dListener.m_iSock, IPPROTO_TCP, TCP_FASTOPEN, ( char * ) &iOn, sizeof ( iOn ) ) )
				sphLogDebug ( "setsockopt(TCP_FASTOPEN) failed: %s", sphSockError () );
		}
#endif
	}

	g_pTickPoolThread = Threads::MakeThreadPool ( g_iNetWorkers, "TickPool" );
	WipeSchedulerOnFork ( g_pTickPoolThread );

	g_dNetLoops.Resize ( g_iNetWorkers );
	for ( auto & pNetLoop : g_dNetLoops )
	{
		pNetLoop = new CSphNetLoop ( g_dListeners );
		g_pTickPoolThread->Schedule ( [pNetLoop] { pNetLoop->LoopNetPoll (); }, false );
	}

	// until no threads started, schedule stopping of alone threads to very bottom
	WipeGlobalSchedulerOnShutdownAndFork();
	Detached::AloneShutdowncatch ();

	// time for replication to sync with cluster
	ReplicationStart ( hSearchd, dListenerDescs, bNewCluster, bNewClusterForce );
	searchd::AddShutdownCb ( ReplicateClustersDelete );

	// ready, steady, go
	sphInfo ( "accepting connections" );

	// disable startup logging to stdout
	if ( !g_bOptNoDetach )
		g_bLogStdout = false;

	while (true)
	{
		CrashLogger::SetupTimePID();
		TickHead();
	}
} // NOLINT ServiceMain() function length

inline int mainimpl ( int argc, char **argv )
{
	// threads should be initialized before memory allocations
	char cTopOfMainStack;
	Threads::Init();
	MemorizeStack ( &cTopOfMainStack );
	sphSetDieCallback ( DieOrFatalCb );
	g_pLogger() = sphLog;
	sphCollationInit ();
	sphBacktraceSetBinaryName ( argv[0] );
	GeodistInit();

#if USE_WINDOWS
	int iNameIndex = -1;
	for ( int i=1; i<argc; i++ )
	{
		if ( strcmp ( argv[i], "--ntservice" )==0 )
			g_bService = true;

		if ( strcmp ( argv[i], "--servicename" )==0 && (i+1)<argc )
		{
			iNameIndex = i+1;
			g_sServiceName = argv[iNameIndex];
		}
	}

	if ( g_bService )
	{
		for ( int i=0; i<argc; i++ )
			g_dArgs.Add ( argv[i] );

		if ( iNameIndex>=0 )
			g_sServiceName = g_dArgs[iNameIndex].cstr ();

		SERVICE_TABLE_ENTRY dDispatcherTable[] =
		{
			{ (LPSTR) g_sServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
			{ NULL, NULL }
		};
		if ( !StartServiceCtrlDispatcher ( dDispatcherTable ) )
			sphFatal ( "StartServiceCtrlDispatcher() failed: %s", WinErrorInfo() );

		return 0;
	} else
#endif

	return ServiceMain ( argc, argv );
}

#ifndef SUPRESS_SEARCHD_MAIN
int main ( int argc, char ** argv )
{
	return mainimpl ( argc, argv );
}
#endif

volatile bool& sphGetGotSighup()
{
	static bool bGotSighup = false;
	return bGotSighup;
}

volatile bool& sphGetGotSigusr1()
{
	static bool bGotSigusr1 = false;
	return bGotSigusr1;
}
