//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxutils.h"
#include "fileutils.h"
#include "sphinxexcerpt.h"
#include "sphinxrt.h"
#include "sphinxpq.h"
#include "sphinxint.h"
#include "sphinxquery.h"
#include "sphinxsort.h"
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
#include "sphinxql_debug.h"
#include "stackmock.h"
#include "binlog.h"
#include "indexfiles.h"
#include "digest_sha1.h"
#include "tokenizer/charset_definition_parser.h"
#include "client_session.h"
#include "sphinx_alter.h"
#include "docs_collector.h"
#include "index_rotator.h"
#include "config_reloader.h"
#include "secondarylib.h"

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
#include "dynamic_idx.h"
#include "netreceive_ql.h"

extern "C"
{
#include "sphinxudf.h"
}

#include <csignal>
#include <clocale>
#include <cmath>
#include <ctime>

#define SEARCHD_BACKLOG			5

// don't shutdown on SIGKILL (debug purposes)
// 1 - SIGKILL will shut down the whole daemon; 0 - watchdog will reincarnate the daemon
#define WATCHDOG_SIGKILL		1

/////////////////////////////////////////////////////////////////////////////

#if _WIN32
	// Win-specific headers and calls
	#include <io.h>

#else
	// UNIX-specific headers and calls
	#include <sys/wait.h>
	#include <netdb.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
#endif

#if USE_SYSLOG
	#include <syslog.h>
#endif

#if HAVE_GETRLIMIT & HAVE_SETRLIMIT
	#include <sys/resource.h>
#endif

/////////////////////////////////////////////////////////////////////////////

using namespace Threads;

static bool				g_bService		= false;
#if _WIN32
static bool				g_bServiceStop	= false;
static const char *		g_sServiceName	= "searchd";
static HANDLE			g_hPipe			= INVALID_HANDLE_VALUE;
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
static LogFormat_e		g_eLogFormat		= LOG_FORMAT_SPHINXQL;
static bool				g_bLogCompactIn		= false;			// whether to cut list in IN() clauses.
static int				g_iQueryLogMinMs	= 0;				// log 'slow' threshold for query
static char				g_sLogFilter[SPH_MAX_FILENAME_LEN+1] = "\0";
static int				g_iLogFilterLen = 0;
static int				g_iLogFileMode = 0;
static CSphBitvec		g_tLogStatements;

int						g_iReadTimeoutS		= 5;	// sec
int						g_iWriteTimeoutS	= 5;	// sec
int						g_iClientTimeoutS	= 300;
int						g_iClientQlTimeoutS	= 900;	// sec
static int				g_iMaxConnection	= 0; // unlimited
static int				g_iThreads;				// defined in config, or =cpu cores
static bool				g_bWatchdog			= true;
static int				g_iExpansionLimit	= 0;
static int				g_iShutdownTimeoutUs	= 3000000; // default timeout on daemon shutdown and stopwait is 3 seconds
static int				g_iBacklog			= SEARCHD_BACKLOG;
static int				g_iThdQueueMax		= 0;
static bool				g_bGroupingInUtc	= false;
static auto&			g_iTFO = sphGetTFO ();
static CSphString		g_sShutdownToken;
static int				g_iServerID = 0;
static bool				g_bServerID = false;
static bool				g_bJsonConfigLoadedOk = false;
static auto&			g_iAutoOptimizeCutoffMultiplier = AutoOptimizeCutoffMultiplier();
static constexpr bool	AUTOOPTIMIZE_NEEDS_VIP = false; // whether non-VIP can issue 'SET GLOBAL auto_optimize = X'

static bool				g_bSplit = true;

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

static int64_t			g_iDocstoreCache = 0;
static int64_t			g_iSkipCache = 0;

static auto &	g_iDistThreads		= getDistThreads();
int				g_iAgentConnectTimeoutMs = 1000;
int				g_iAgentQueryTimeoutMs = 3000;	// global (default). May be override by index-scope values, if one specified

const int	MAX_RETRY_COUNT		= 8;
const int	MAX_RETRY_DELAY		= 1000;

int						g_iAgentRetryCount = 0;
int						g_iAgentRetryDelayMs = MAX_RETRY_DELAY/2;	// global (default) values. May be override by the query options 'retry_count' and 'retry_timeout'
bool					g_bHostnameLookup = false;
CSphString				g_sMySQLVersion = szMANTICORE_VERSION;
CSphString				g_sDbName = "Manticore";

CSphString				g_sBannerVersion { szMANTICORE_NAME };
CSphString				g_sBanner;
CSphString				g_sStatusVersion = szMANTICORE_VERSION;
CSphString				g_sSecondaryError;
bool					g_bSecondaryError { false };

// for CLang thread-safety analysis
ThreadRole MainThread; // functions which called only from main thread
ThreadRole HandlerThread; // thread which serves clients

//////////////////////////////////////////////////////////////////////////

static CSphString		g_sConfigFile;
static bool				g_bCleanLoadedConfig = true; // whether to clean config when it parsed and no more necessary
static bool				LOG_LEVEL_SHUTDOWN = val_from_env("MANTICORE_TRACK_DAEMON_SHUTDOWN",false); // verbose logging when daemon shutdown, ruled by this env variable
static CSphString		g_sConfigPath; // for resolve paths to absolute

static auto&			g_bSeamlessRotate	= sphGetSeamlessRotate ();

static bool				g_bIOStats		= false;
static auto&			g_bCpuStats 	= sphGetbCpuStat ();
static bool				g_bOptNoDetach	= false;
static bool				g_bOptNoLock	= false;
static bool				g_bSafeTrace	= false;
static bool				g_bStripPath	= false;
static bool				g_bCoreDump		= false;

static auto& g_bGotSighup		= sphGetGotSighup();	// we just received SIGHUP; need to log
static auto& g_bGotSigusr1		= sphGetGotSigusr1();	// we just received SIGUSR1; need to reopen logs
static auto& g_bGotSigusr2		= sphGetGotSigusr2();   // we just received SIGUSR2; need to dump daemon's bt

// pipe to watchdog to inform that daemon is going to close, so no need to restart it in case of crash
struct SharedData_t
{
	bool m_bDaemonAtShutdown;
	bool m_bHaveTTY;
};

static SharedData_t* 		g_pShared = nullptr;
volatile bool				g_bMaintenance = false;

std::unique_ptr<ReadOnlyServedHash_c>	g_pLocalIndexes = std::make_unique<ReadOnlyServedHash_c>();	// served (local) indexes hash
std::unique_ptr<ReadOnlyDistrHash_c>	g_pDistIndexes = std::make_unique<ReadOnlyDistrHash_c>();	// distributed indexes hash

// this is internal deal of the daemon; don't expose it outside!
// fixme! move all this stuff to dedicated file.

static RwLock_t								g_tRotateConfigMutex;
static CSphConfig							g_hCfg GUARDED_BY ( g_tRotateConfigMutex );
static volatile bool						g_bNeedRotate = false;		// true if there were pending HUPs to handle (they could fly in during previous rotate)
static volatile bool						g_bInRotate = false;		// true while we are rotating
static volatile bool						g_bReloadForced = false;	// true in case reload issued via SphinxQL

static WorkerSharedPtr_t					g_pTickPoolThread;
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

static void ReleaseTTYFlag()
{
	if ( g_pShared )
		g_pShared->m_bHaveTTY = true;
}

/////////////////////////////////////////////////////////////////////////////
// LOGGING
/////////////////////////////////////////////////////////////////////////////


/// physically emit log entry
/// buffer must have 1 extra byte for linefeed
#if _WIN32
static void sphLogEntry ( ESphLogLevel eLevel, char * sBuf, char * sTtyBuf )
#else
static void sphLogEntry ( ESphLogLevel , char * sBuf, char * sTtyBuf )
#endif
{
#if _WIN32
	if ( g_bService && g_iLogFile==STDOUT_FILENO )
	{
		HANDLE hEventSource;
		LPCTSTR lpszStrings[2];

		hEventSource = RegisterEventSource ( NULL, g_sServiceName );
		if ( hEventSource )
		{
			lpszStrings[0] = g_sServiceName;
			lpszStrings[1] = sBuf;

			WORD eType;
			switch ( eLevel )
			{
				case SPH_LOG_FATAL:		eType = EVENTLOG_ERROR_TYPE; break;
				case SPH_LOG_WARNING:	eType = EVENTLOG_WARNING_TYPE; break;
				case SPH_LOG_INFO:		eType = EVENTLOG_INFORMATION_TYPE; break;
				default:				eType = EVENTLOG_INFORMATION_TYPE; break;
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

#if !_WIN32
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

#if !_WIN32
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
	CSphString	m_sParentIndex;
	CSphString	m_sIndex;	///< searched index name
	CSphString	m_sError;	///< search error message

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

	SearchFailure_t & operator = ( const SearchFailure_t & r )
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
	void Submit ( const CSphString& sIndex, const char * sParentIndex , const char * sError )
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

	inline void Append ( const SearchFailuresLog_c& rhs )
	{
		m_dLog.Append ( rhs.m_dLog );
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

			sReport << sColon;

			ReportIndexesName ( iSpanStart, i, m_dLog, sReport );
			sReport << m_dLog[iSpanStart].m_sError;

			// done
			iSpanStart = i;
		}
	}
};

#define LOG_COMPONENT_SEARCHD __LINE__ << " "

#define SHUTINFO LOGINFO (SHUTDOWN,SEARCHD)

/////////////////////////////////////////////////////////////////////////////
// SIGNAL HANDLERS
/////////////////////////////////////////////////////////////////////////////
void Shutdown () REQUIRES ( MainThread ) NO_THREAD_SAFETY_ANALYSIS
{
	// force even long time searches to shut
	sphInterruptNow ();

#if !_WIN32
	int fdStopwait = -1;
#endif
	bool bAttrsSaveOk = true;
	if ( g_pShared )
		g_pShared->m_bDaemonAtShutdown = true;

#if !_WIN32
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
	SHUTINFO << "Shut down mini timer ...";
	sph::ShutdownMiniTimer();

	SHUTINFO << "Shut down flushing mutable ...";
	ShutdownFlushingMutable();

	// stop search threads; up to shutdown_timeout seconds
	SHUTINFO << "Wait preread (if any) finished ...";
	WaitPrereadFinished ( g_iShutdownTimeoutUs );

	// save attribute updates for all local indexes
	SHUTINFO << "Finally save indexes ...";
	bAttrsSaveOk = FinallySaveIndexes();

	// right before unlock loop
	if ( g_bJsonConfigLoadedOk )
	{
		CSphString sError;
		SHUTINFO << "Save json config ...";
		SaveConfigInt(sError);
	}

	// stop netloop processing
	SHUTINFO << "Stop netloop processing ...";
	for ( auto & pNetLoop : g_dNetLoops )
	{
		pNetLoop->StopNetLoop ();
		SafeRelease ( pNetLoop );
	}

	// stop netloop threads
	SHUTINFO << "Stop netloop pool ...";
	if ( g_pTickPoolThread )
		g_pTickPoolThread->StopAll ();

	// call scheduled callbacks:
	// shutdown replication,
	// shutdown ssl,
	// shutdown tick threads,
	SHUTINFO << "Invoke shutdown callbacks ...";
	searchd::FireShutdownCbs ();

	SHUTINFO << "Waiting clients to finish ... (" << myinfo::CountClients() << ")";
	while ( ( myinfo::CountClients ()>0 ) && ( sphMicroTimer ()-tmShutStarted )<g_iShutdownTimeoutUs )
		sphSleepMsec ( 50 );

	if ( myinfo::CountClients ()>0 )
	{
		int64_t tmDelta = sphMicroTimer ()-tmShutStarted;
		sphWarning ( "still %d alive tasks during shutdown, after %d.%03d sec", myinfo::CountClients (), (int) ( tmDelta
				/ 1000000 ), (int) ( ( tmDelta / 1000 ) % 1000 ) );
	}

	// unlock indexes and release locks if needed
	SHUTINFO << "Unlock indexes ...";
	{
		ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
		for ( const auto& tIt : *hLocal )
			RWIdx_c ( tIt.second )->Unlock();
	}

	Threads::CallCoroutine ( [] {
		SHUTINFO << "Abandon local indexes list ...";
		g_pLocalIndexes->ReleaseAndClear();

		// unlock Distr indexes automatically done by d-tr
		SHUTINFO << "Abandon distr indexes list ...";
		g_pDistIndexes->ReleaseAndClear();
	} );

	SHUTINFO << "Shutdown alone threads (if any) ...";
	Detached::ShutdownAllAlones();

	SHUTINFO << "Shutdown main work pool ...";
	auto pPool = GlobalWorkPool();
	if ( pPool )
		pPool->StopAll();

	SHUTINFO << "Remove local indexes list ...";
	g_pLocalIndexes.reset();

	SHUTINFO << "Remove distr indexes list ...";
	g_pDistIndexes.reset();

	// clear shut down of rt indexes + binlog
	SHUTINFO << "Finish IO stats collecting ...";
	sphDoneIOStats();

	SHUTINFO << "Finish RT serving ...";
	Binlog::Deinit();

	SHUTINFO << "Shutdown docstore ...";
	ShutdownDocstore();

	SHUTINFO << "Shutdown skip cache ...";
	ShutdownSkipCache();

	SHUTINFO << "Shutdown global IDFs ...";
	sph::ShutdownGlobalIDFs ();

	SHUTINFO << "Shutdown aot ...";
	sphAotShutdown ();

	SHUTINFO << "Shutdown columnar ...";
	ShutdownColumnar();

	SHUTINFO << "Shutdown listeners ...";
	for ( auto& dListener : g_dListeners )
		if ( dListener.m_iSock>=0 )
			sphSockClose ( dListener.m_iSock );

	SHUTINFO << "Close persistent sockets ...";
	ClosePersistentSockets();

	// close pid
	SHUTINFO << "Release (close) pid file ...";
	if ( g_iPidFD!=-1 )
		::close ( g_iPidFD );
	g_iPidFD = -1;

	// remove pid file, if we owned it
	if ( g_bPidIsMine && !g_sPidFile.IsEmpty() )
		::unlink ( g_sPidFile.cstr() );

	SHUTINFO << "Shutdown hazard pointers ...";
	hazard::Shutdown ();

	// wordforms till there might be referenced from accum (rt-index), which, in turn, is part of client session.
	// so, shutdown them before will probably fail.
	// after hazard shutdown, all sessions are surely done, so wordforms is good to be destroyed at this point.
	SHUTINFO << "Shutdown wordforms ...";
	sphShutdownWordforms();

	sphInfo ( "shutdown daemon version '%s' ...", g_sStatusVersion.cstr() );
	sphInfo ( "shutdown complete" );

	Threads::Done ( g_iLogFile );

#if _WIN32
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


static void sigterm ( int )
{
	// tricky bit
	// we can't call exit() here because malloc()/free() are not re-entrant
	// we could call _exit() but let's try to die gracefully on TERM
	// and let signal sender wait and send KILL as needed
	sphInterruptNow();
}


static void sigusr1 ( int )
{
	g_bGotSigusr1 = true;
}


static void sigusr2 ( int )
{
	g_bGotSigusr2 = true;
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
static bool sphCopyEncodedBase64 ( QueryCopyState_t & tEnc )
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
#if _WIN32
const char		g_sMinidumpBanner[] = "minidump located at: ";
#endif
#if SPH_ALLOCS_PROFILER
const char		g_sMemoryStatBanner[] = "\n--- memory statistics ---\n";
#endif
static BYTE		g_dCrashQueryBuff [4096];
static char		g_sCrashInfo [SPH_TIME_PID_MAX_SIZE] = "[][]\n";
static int		g_iCrashInfoLen = 0;

#if _WIN32
static char		g_sMinidump[SPH_TIME_PID_MAX_SIZE] = "";
#endif

#if !_WIN32
void CrashLogger::HandleCrash ( int sig ) NO_THREAD_SAFETY_ANALYSIS
#else
LONG WINAPI CrashLogger::HandleCrash ( EXCEPTION_POINTERS * pExc )
#endif // !_WIN32
{
	sphSetDied();
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

	bool bValidQuery = IsFilled ( tQuery.m_dQuery );
#if !_WIN32
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

		auto pPageStart = (uintptr_t) tQuery.m_dQuery.first;
		pPageStart &= ~( iPageSize - 1 );
		bValidQuery &= ( mincore ( ( void * ) pPageStart, 1, &dPages )==0 );

		auto pPageEnd = (uintptr_t) ( tQuery.m_dQuery.first+tQuery.m_dQuery.second-1 );
		pPageEnd &= ~( iPageSize - 1 );
		bValidQuery &= ( mincore ( ( void * ) pPageEnd, 1, &dPages )==0 );
	}
#endif

	// request dump banner
	Str_t dBanner = { g_sCrashedBannerAPI, sizeof ( g_sCrashedBannerAPI )-1 };
	if ( tQuery.m_eType==QUERY_SQL )
		dBanner = { g_sCrashedBannerMySQL, sizeof ( g_sCrashedBannerMySQL )-1 };
	else if ( tQuery.m_eType==QUERY_JSON )
		dBanner = { g_sCrashedBannerHTTP, sizeof ( g_sCrashedBannerHTTP )-1 };

	if ( !bValidQuery )
		dBanner = { g_sCrashedBannerBad, sizeof ( g_sCrashedBannerBad )-1 };

	sphWrite ( g_iLogFile, dBanner );

	// query
	if ( bValidQuery )
	{
		QueryCopyState_t tCopyState;
		tCopyState.m_pDst = g_dCrashQueryBuff;
		tCopyState.m_pDstEnd = g_dCrashQueryBuff + sizeof(g_dCrashQueryBuff);
		tCopyState.m_pSrc = tQuery.m_dQuery.first;
		tCopyState.m_pSrcEnd = tQuery.m_dQuery.first + tQuery.m_dQuery.second;

		CopyQuery_fn * pfnCopy = NULL;
		if ( tQuery.m_eType==QUERY_API )
		{
			pfnCopy = &sphCopyEncodedBase64;

			// should be power of 3 to seamlessly convert to BASE64
			BYTE dHeader[] = {
				(BYTE)( ( tQuery.m_uCMD>>8 ) & 0xff ),
				(BYTE)( tQuery.m_uCMD & 0xff ),
				(BYTE)( ( tQuery.m_uVer>>8 ) & 0xff ),
				(BYTE)( tQuery.m_uVer & 0xff ),
				(BYTE)( ( tQuery.m_dQuery.second>>24 ) & 0xff ),
				(BYTE)( ( tQuery.m_dQuery.second>>16 ) & 0xff ),
				(BYTE)( ( tQuery.m_dQuery.second>>8 ) & 0xff ),
				(BYTE)( tQuery.m_dQuery.second & 0xff ),
				*tQuery.m_dQuery.first
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
		} else if ( tQuery.m_eType==QUERY_JSON )
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

		int iLeft = int ( tCopyState.m_pDst-g_dCrashQueryBuff );
		if ( iLeft>0 )
			sphWrite ( g_iLogFile, g_dCrashQueryBuff, iLeft );
	}

	// tail
	sphWrite ( g_iLogFile, g_sCrashedBannerTail, sizeof(g_sCrashedBannerTail)-1 );

	// index name
	sphWrite ( g_iLogFile, g_sCrashedIndex, sizeof (g_sCrashedIndex)-1 );
	if ( IsFilled ( tQuery.m_dIndex ) )
		sphWrite ( g_iLogFile, tQuery.m_dIndex );
	sphWrite ( g_iLogFile, g_sEndLine, sizeof (g_sEndLine)-1 );

	sphSafeInfo ( g_iLogFile, g_sBannerVersion.cstr() );

#if _WIN32
	// mini-dump reference
	int iMiniDumpLen = snprintf ( (char *)g_dCrashQueryBuff, sizeof(g_dCrashQueryBuff),
		"%s %s.%p.mdmp\n", g_sMinidumpBanner, g_sMinidump, tQuery.m_dQuery.first );
	sphWrite ( g_iLogFile, g_dCrashQueryBuff, iMiniDumpLen );
	snprintf ( (char *)g_dCrashQueryBuff, sizeof(g_dCrashQueryBuff), "%s.%p.mdmp",
		g_sMinidump, tQuery.m_dQuery.first );
#endif

	// log trace
#if !_WIN32
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
							ProtoName (pSrc->GetProto()), TaskStateName ( pSrc->GetTaskState() ),
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

#if _WIN32
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
	sa.sa_handler = sigusr2;	if ( sigaction ( SIGUSR2, &sa, NULL )!=0 )		return;
	sa.sa_handler = SIG_IGN;	if ( sigaction ( SIGPIPE, &sa, NULL )!=0 ) return;

	sa.sa_flags |= SA_RESETHAND;

	static CSphVector<BYTE> exception_handler_stack ( Max ( SIGSTKSZ, 65536 ) );
	stack_t ss;
	ss.ss_sp = exception_handler_stack.begin();
	ss.ss_flags = 0;
	ss.ss_size = exception_handler_stack.GetLength();
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

#if !_WIN32
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
#endif // !_WIN32


int sphCreateInetSocket ( const ListenerDesc_t & tDesc ) REQUIRES ( MainThread )
{
	auto uAddr = tDesc.m_uIP;
	auto iPort = tDesc.m_iPort;
	char sAddress[SPH_ADDRESS_SIZE];
	sphFormatIP ( sAddress, SPH_ADDRESS_SIZE, uAddr );
	const char * sVip = tDesc.m_bVIP ? "VIP " : "";
	const char * sRO = tDesc.m_bReadOnly ? "RO " : "";

	if ( uAddr==htonl ( INADDR_ANY ) )
		sphInfo ( "listening on all interfaces for %s%s%s, port=%d", sVip, sRO, RelaxedProtoName ( tDesc.m_eProto), iPort );
	else
		sphInfo ( "listening on %s:%d for %s%s%s", sAddress, iPort, sVip, sRO, RelaxedProtoName ( tDesc.m_eProto ) );

	static struct sockaddr_in iaddr;
	memset ( &iaddr, 0, sizeof(iaddr) );
	iaddr.sin_family = AF_INET;
	iaddr.sin_addr.s_addr = uAddr;
	iaddr.sin_port = htons ( (short)iPort );

	int iSock = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( iSock==-1 )
		sphFatal ( "failed to create TCP socket: %s", sphSockError() );

	sphSetSockReuseAddr ( iSock );
	sphSetSockReusePort ( iSock );
	sphSetSockNodelay ( iSock );

	int iTries = 12;
	int iRes;
	do
	{
		iRes = bind ( iSock, (struct sockaddr *)&iaddr, sizeof(iaddr) );
		if ( iRes==0 )
			break;

		sphInfo ( "bind() failed on %s, retrying...", sAddress );
		sphLogDebug ( "bind() failed on %s:%d, error: %s", sAddress, iPort, sphSockError() );
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
	tDesc.m_bReadOnly = false;
	return tDesc;
}

ListenerDesc_t MakeLocalhostListener ( int iPort, Proto_e eProto )
{
	ListenerDesc_t tDesc;
	tDesc.m_eProto = eProto;
	tDesc.m_uIP = htonl ( INADDR_LOOPBACK );
	tDesc.m_iPort = iPort;
	tDesc.m_iPortsCount = 0;
	tDesc.m_bVIP = false;
	tDesc.m_bReadOnly = false;
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
	tListener.m_bReadOnly = tDesc.m_bReadOnly;

#if !_WIN32
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

struct ListenerPortRange_t
{
	DWORD m_uIP { 0 };
	int m_iPort { 0 };
	int m_iCount { 0 };

	static inline bool IsLess ( const ListenerPortRange_t & tA, const ListenerPortRange_t & tB )
	{
		if ( tA.m_uIP==tB.m_uIP )
			return ( tA.m_iPort<tB.m_iPort );

		return ( tA.m_uIP<tB.m_uIP );
	}

	CSphString Dump () const
	{
		char sAddress[SPH_ADDRESS_SIZE];
		sphFormatIP ( sAddress, SPH_ADDRESS_SIZE, m_uIP );

		CSphString sRes;
		if ( m_iCount )
			sRes.SetSprintf ( "%s:%d-%d", sAddress, m_iPort, ( m_iPort+m_iCount-1 ) );
		else
			sRes.SetSprintf ( "%s:%d", sAddress, m_iPort );

		return sRes;
	}
};


static bool ValidateListenerRanges ( const VecTraits_T<ListenerDesc_t> & dListeners, CSphString & sError )
{
	CSphVector<ListenerPortRange_t> dPorts;
	for ( const ListenerDesc_t & tDesc : dListeners )
	{
		if ( !tDesc.m_sUnix.IsEmpty() )
			continue;

		ListenerPortRange_t & tPort = dPorts.Add();
		tPort.m_uIP = tDesc.m_uIP;
		tPort.m_iPort = tDesc.m_iPort;
		tPort.m_iCount = tDesc.m_iPortsCount;
	}
	dPorts.Sort ( ListenerPortRange_t() );

	for ( int i=1; i<dPorts.GetLength(); i++ )
	{
		const ListenerPortRange_t & tPrev = dPorts[i-1];
		const ListenerPortRange_t & tCur = dPorts[i];
		if ( tPrev.m_uIP!=tCur.m_uIP )
			continue;
		if ( ( !tPrev.m_iCount && tPrev.m_iPort<tCur.m_iPort ) || ( tPrev.m_iCount && tPrev.m_iPort+tPrev.m_iCount-1<tCur.m_iPort ) )
			continue;

		sError.SetSprintf ( "invalid listener ports intersection %s -> %s", tPrev.Dump().cstr(), tCur.Dump().cstr() );
		return false;
	}

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

void ISphOutputBuffer::SendBytes ( const Str_t& sStr )
{
	m_dBuf.Append ( sStr );
}

void ISphOutputBuffer::SendBytes ( const VecTraits_T<BYTE> & dBuf )
{
	m_dBuf.Append ( dBuf );
}

void ISphOutputBuffer::SendBytes ( const StringBuilder_c &dBuf )
{
	SendBytes ( dBuf.begin(), dBuf.GetLength () );
}

void ISphOutputBuffer::SendBytes ( ByteBlob_t dData )
{
	SendBytes ( dData.first, dData.second );
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

void ISphOutputBuffer::SendArray ( ByteBlob_t dData )
{
	SendArray ( dData.first, dData.second );
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

void DistributedIndex_t::GetAllHosts ( VectorAgentConn_t &dTarget ) const
{
	for ( const auto& pMultiAgent : m_dAgents )
		for ( const auto & dHost : *pMultiAgent )
		{
			auto * pAgent = new AgentConn_t;
			pAgent->m_tDesc.CloneFrom ( dHost );
			pAgent->m_iMyQueryTimeoutMs = m_iAgentQueryTimeoutMs;
			pAgent->m_iMyConnectTimeoutMs = m_iAgentConnectTimeoutMs;
			dTarget.Add ( pAgent );
		}
}

DistributedIndex_t::~DistributedIndex_t ()
{
	sphLogDebugv ( "DistributedIndex_t %p removed", this );

	// cleanup global
	m_dAgents.Reset();
	MultiAgentDesc_c::CleanupOrphaned ();
};

/////////////////////////////////////////////////////////////////////////////
// SEARCH HANDLER
/////////////////////////////////////////////////////////////////////////////

class SearchRequestBuilder_c : public RequestBuilder_i
{
public:
	SearchRequestBuilder_c ( const VecTraits_T<CSphQuery> & dQueries, int iDivideLimits )
		: m_dQueries ( dQueries ), m_iDivideLimits ( iDivideLimits )
	{}

	void		BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final;

protected:
	void		SendQuery ( const char * sIndexes, ISphOutputBuffer & tOut, const CSphQuery & q, int iWeight, int iAgentQueryTimeout ) const;

protected:
	const VecTraits_T<CSphQuery> &		m_dQueries;
	const int							m_iDivideLimits;
};


class SearchReplyParser_c : public ReplyParser_i, public ISphNoncopyable
{
public:
	explicit SearchReplyParser_c ( int iResults )
		: m_iResults ( iResults )
	{}

	bool	ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final;

private:
	int		m_iResults;

	static void	ParseSchema ( OneResultset_t & tRes, MemInputBuffer_c & tReq );
	static void	ParseMatch ( CSphMatch & tMatch, MemInputBuffer_c & tReq, const CSphSchema & tSchema, bool bAgent64 );
};

/////////////////////////////////////////////////////////////////////////////

/// qflag means Query Flag
/// names are internal to searchd and may be changed for clarity
/// values are communicated over network between searchds and APIs and MUST NOT CHANGE
enum
{
	QFLAG_REVERSE_SCAN			= 1UL << 0,		// deprecated
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
	QFLAG_JSON_QUERY			= 1UL << 11,
	QFLAG_NOT_ONLY_ALLOWED		= 1UL << 12
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
	uFlags |= QFLAG_NOT_ONLY_ALLOWED * q.m_bNotOnlyAllowed;

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
		tOut.SendDword ( (DWORD)i.m_dHints[int(SecondaryIndexType_e::INDEX)] );
		tOut.SendString ( i.m_sIndex.cstr() );
	}
}


void SearchRequestBuilder_c::BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const
{
	auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_SEARCH, VER_COMMAND_SEARCH ); // API header

	tOut.SendInt ( VER_COMMAND_SEARCH_MASTER );
	tOut.SendInt ( m_dQueries.GetLength() );
	for ( auto& dQuery : m_dQueries )
		SendQuery ( tAgent.m_tDesc.m_sIndexes.cstr (), tOut, dQuery, tAgent.m_iWeight, tAgent.m_iMyQueryTimeoutMs );
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
		return m_dResults.any_of ( [] ( const AggrResult_t &dRes ) { return !dRes.m_sWarning.IsEmpty (); } );
	}
};


/////////////////////////////////////////////////////////////////////////////

void SearchReplyParser_c::ParseMatch ( CSphMatch & tMatch, MemInputBuffer_c & tReq, const CSphSchema & tSchema, bool bAgent64 )
{
	tMatch.Reset ( tSchema.GetRowSize() );

	// WAS: docids
	if ( bAgent64 )
		tReq.GetUint64();
	else
		tReq.GetDword();

	tMatch.m_iWeight = tReq.GetInt ();
	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
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
				auto * pMVA = (DWORD *)pData;
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
				if (iLen)
				{
					tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t)sphPackPtrAttr ( iLen, &pData ) );
					tReq.GetBytes ( pData, iLen );
				} else
					tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t) 0 );
			}
			break;

		case SPH_ATTR_JSON_FIELD_PTR:
			{
				// FIXME: no reason for json_field to be any different from other *_PTR attributes
				auto eJson = (ESphJsonType)tReq.GetByte();
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
			tMatch.SetAttrFloat ( tAttr.m_tLocator, tReq.GetFloat() );
			break;

		case SPH_ATTR_DOUBLE:
			tMatch.SetAttrDouble ( tAttr.m_tLocator, tReq.GetDouble() );
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


void SearchReplyParser_c::ParseSchema ( OneResultset_t & tRes, MemInputBuffer_c & tReq )
{
	CSphSchema & tSchema = tRes.m_tSchema;
	tSchema.Reset ();

	int nFields = tReq.GetInt(); // FIXME! add a sanity check
	for ( int j = 0; j < nFields; ++j )
		tSchema.AddField ( tReq.GetString().cstr() );

	int iNumAttrs = tReq.GetInt(); // FIXME! add a sanity check
	for ( int j=0; j<iNumAttrs; ++j )
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
	const int iResults = m_iResults;
	assert ( iResults>0 );

	if ( !tAgent.m_pResult )
		tAgent.m_pResult = std::make_unique<cSearchResult>();

	auto pResult = (cSearchResult*)tAgent.m_pResult.get();
	auto &dResults = pResult->m_dResults;

	dResults.Resize ( iResults );
	for ( auto & tRes : dResults )
	{
		tRes.m_iSuccesses = 0;
		OneResultset_t tChunk;
		tChunk.m_iTag = tAgent.m_iStoreTag;
		tChunk.m_bTag = true;
		tChunk.m_pAgent = &tAgent;
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

		ParseSchema ( tChunk, tReq );

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

		tChunk.m_dMatches.Resize ( iMatches );
		for ( auto & tMatch : tChunk.m_dMatches )
			ParseMatch ( tMatch, tReq, tChunk.m_tSchema, bAgent64 );

		// read totals (retrieved count, total count, query time, word count)
		int iRetrieved = tReq.GetInt ();
		tRes.m_iTotalMatches = tReq.GetInt ();
		tRes.m_bTotalMatchesApprox = !!tReq.GetInt();
		tRes.m_iQueryTime = tReq.GetInt ();

		// agents always send IO/CPU stats to master
		BYTE uStatMask = tReq.GetByte();
		if ( uStatMask & 1U )
		{
			tRes.m_tIOStats.m_iReadTime = tReq.GetUint64();
			tRes.m_tIOStats.m_iReadOps = tReq.GetDword();
			tRes.m_tIOStats.m_iReadBytes = tReq.GetUint64();
			tRes.m_tIOStats.m_iWriteTime = tReq.GetUint64();
			tRes.m_tIOStats.m_iWriteOps = tReq.GetDword();
			tRes.m_tIOStats.m_iWriteBytes = tReq.GetUint64();
		}

		if ( uStatMask & 2U )
			tRes.m_iCpuTime = tReq.GetUint64();

		if ( uStatMask & 4U )
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
		for ( int i=0; i<iWordsCount; ++i )
		{
			const CSphString sWord = tReq.GetString ();
			const int64_t iDocs = (unsigned int)tReq.GetInt ();
			const int64_t iHits = (unsigned int)tReq.GetInt ();
			tReq.GetByte(); // statistics have no expanded terms for now

			tRes.AddStat ( sWord, iDocs, iHits );
		}

		// mark this result as ok
		auto& tNewChunk = tRes.m_dResults.Add ();
		::Swap ( tNewChunk, tChunk );
		tRes.m_iSuccesses = 1;
	}

	// all seems OK (and buffer length checks are performed by caller)
	return true;
}

/////////////////////////////////////////////////////////////////////////////

// returns true if incoming schema (src) is compatible with existing (dst); false otherwise
bool MinimizeSchema ( CSphSchema & tDst, const ISphSchema & tSrc )
{
	// if dst is empty, result is also empty
	if ( tDst.GetAttrsCount()==0 )
		return tSrc.GetAttrsCount()==0;

	// check for equality, and remove all dst attributes that are not present in src
	CSphVector<CSphColumnInfo> dDst;
	for ( int i = 0, iAttrsCount = tDst.GetAttrsCount (); i<iAttrsCount; ++i )
		dDst.Add ( tDst.GetAttr(i) );

	bool bEqual = ( tDst.GetAttrsCount()==tSrc.GetAttrsCount() );
	ARRAY_FOREACH ( i, dDst )
	{
		auto& tDstAttr = dDst[i];
		int iSrcIdx = tSrc.GetAttrIndex ( tDstAttr.m_sName.cstr() );

		// check for index mismatch
		if ( iSrcIdx!=i )
			bEqual = false;

		// check for type/size mismatch (and fixup if needed)
		if ( iSrcIdx>=0 )
		{
			const CSphColumnInfo & tSrcAttr = tSrc.GetAttr ( iSrcIdx );

			// should seamlessly convert ( bool > float ) | ( bool > int > bigint )
			ESphAttr eDst = tDstAttr.m_eAttrType;
			ESphAttr eSrc = tSrcAttr.m_eAttrType;
			bool bSame = ( eDst==eSrc )
				|| ( ( eDst==SPH_ATTR_FLOAT && eSrc==SPH_ATTR_BOOL ) || ( eDst==SPH_ATTR_BOOL && eSrc==SPH_ATTR_FLOAT ) )
				|| ( ( eDst==SPH_ATTR_BOOL || eDst==SPH_ATTR_INTEGER || eDst==SPH_ATTR_BIGINT )
					&& ( eSrc==SPH_ATTR_BOOL || eSrc==SPH_ATTR_INTEGER || eSrc==SPH_ATTR_BIGINT ) );

			int iDstBitCount = tDstAttr.m_tLocator.m_iBitCount;
			int iSrcBitCount = tSrcAttr.m_tLocator.m_iBitCount;

			if ( !bSame )
			{
				// different types? remove the attr
				iSrcIdx = -1;
				bEqual = false;

			} else if ( iDstBitCount!=iSrcBitCount )
			{
				// different bit sizes? choose the max one
				tDstAttr.m_tLocator.m_iBitCount = Max ( iDstBitCount, iSrcBitCount );
				bEqual = false;
				if ( iDstBitCount<iSrcBitCount )
					tDstAttr.m_eAttrType = tSrcAttr.m_eAttrType;
			}

			if ( tSrcAttr.m_tLocator.m_iBitOffset!=tDstAttr.m_tLocator.m_iBitOffset )
			{
				// different offsets? have to force target dynamic then, since we can't use one locator for all matches
				bEqual = false;
			}

			if ( tSrcAttr.m_tLocator.m_bDynamic!=tDstAttr.m_tLocator.m_bDynamic )
			{
				// different location? have to force target dynamic then
				bEqual = false;
			}
		}

		// check for presence
		if ( iSrcIdx<0 )
		{
			dDst.Remove ( i );
			--i;
		}
	}

	if ( !bEqual )
	{
		CSphVector<CSphColumnInfo> dFields { tDst.GetFieldsCount() };
		for ( int i = 0, iFieldsCount = tDst.GetFieldsCount (); i<iFieldsCount; ++i )
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
	#define LOC_ERROR( ... ) do { sError.SetSprintf (__VA_ARGS__); return; } while(0)

	sError = nullptr;

	if ( (int)tQuery.m_eMode<0 || tQuery.m_eMode>SPH_MATCH_TOTAL )
		LOC_ERROR ( "invalid match mode %d", tQuery.m_eMode );

	if ( (int)tQuery.m_eRanker<0 || tQuery.m_eRanker>SPH_RANK_TOTAL )
		LOC_ERROR ( "invalid ranking mode %d", tQuery.m_eRanker );

	if ( tQuery.m_iMaxMatches<1 )
		LOC_ERROR ( "max_matches can not be less than one" );

	if ( tQuery.m_iOffset<0 || tQuery.m_iOffset>=tQuery.m_iMaxMatches )
		LOC_ERROR ( "offset out of bounds (offset=%d, max_matches=%d)", tQuery.m_iOffset, tQuery.m_iMaxMatches );

	if ( tQuery.m_iLimit<0 )
		LOC_ERROR ( "limit out of bounds (limit=%d)", tQuery.m_iLimit );

	if ( tQuery.m_iCutoff<-1 )
		LOC_ERROR ( "cutoff out of bounds (cutoff=%d)", tQuery.m_iCutoff );

	if ( ( tQuery.m_iRetryCount!=-1 ) && ( tQuery.m_iRetryCount>MAX_RETRY_COUNT ) )
		LOC_ERROR ( "retry count out of bounds (count=%d)", tQuery.m_iRetryCount );

	if ( ( tQuery.m_iRetryDelay!=-1 ) && ( tQuery.m_iRetryDelay>MAX_RETRY_DELAY ) )
		LOC_ERROR ( "retry delay out of bounds (delay=%d)", tQuery.m_iRetryDelay );

	if ( tQuery.m_iOffset>0 && tQuery.m_bHasOuter )
		LOC_ERROR ( "inner offset must be 0 when using outer order by (offset=%d)", tQuery.m_iOffset );

	#undef LOC_ERROR
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


static void AddDocids ( CSphVector<CSphQueryItem> & dItems )
{
	if ( !dItems.GetLength() )
		return;

	bool bHaveDocID = false;
	for ( const auto & i : dItems )
		bHaveDocID |= i.m_sAlias==sphGetDocidName() || i.m_sExpr=="*";

	if ( !bHaveDocID )
	{
		CSphQueryItem tId;
		tId.m_sExpr = tId.m_sAlias = sphGetDocidName();
		dItems.Insert ( 0, tId );
	}
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
	tQuery.m_bExplicitMaxMatches = tQuery.m_iMaxMatches!=DEFAULT_MAX_MATCHES; // fixme?
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
			tBuf << "/* ";
			sphFormatCurrentTime ( tBuf );
			tBuf << "*/ " << tQuery.m_sSelect << " # error=" << sError << '\n';
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
		tQuery.m_bSortKbuffer = !!( uFlags & QFLAG_SORT_KBUFFER );
		tQuery.m_bSimplify = !!( uFlags & QFLAG_SIMPLIFY );
		tQuery.m_bPlainIDF = !!( uFlags & QFLAG_PLAIN_IDF );
		tQuery.m_bGlobalIDF = !!( uFlags & QFLAG_GLOBAL_IDF );
		tQuery.m_bLocalDF = !!( uFlags & QFLAG_LOCAL_DF );
		tQuery.m_bLowPriority = !!( uFlags & QFLAG_LOW_PRIORITY );
		tQuery.m_bFacet = !!( uFlags & QFLAG_FACET );
		tQuery.m_bFacetHead = !!( uFlags & QFLAG_FACET_HEAD );
		tQuery.m_eQueryType = (uFlags & QFLAG_JSON_QUERY) ? QUERY_JSON : QUERY_API;
		tQuery.m_bNotOnlyAllowed = !!( uFlags & QFLAG_NOT_ONLY_ALLOWED );

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
	tQuery.m_eCollation = GlobalCollation ();
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
			i.m_dHints[int(SecondaryIndexType_e::INDEX)] = (IndexHint_e)tReq.GetDword();
			i.m_sIndex = tReq.GetString();
		}
	}

	/////////////////////
	// additional checks
	/////////////////////

	// queries coming from API may not request docids
	// but we still need docids when sending result sets
	AddDocids ( tQuery.m_dItems );
	AddDocids ( tQuery.m_dRefItems );

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
	static constexpr bool IsEscapeChar ( char c )
	{
		return ( c=='\\' || c=='\'' );
	}
};


using QuotationEscapedBuilder = EscapedStringBuilder_T<EscapeQuotation_t>;


void LogQueryPlain ( const CSphQuery & tQuery, const CSphQueryResultMeta & tMeta )
{
	assert ( g_eLogFormat==LOG_FORMAT_PLAIN );
	if ( ( !g_bQuerySyslog && g_iQueryLogFile<0 ) || !tMeta.m_sError.IsEmpty() )
		return;

	QuotationEscapedBuilder tBuf;

	// [time]
#if USE_SYSLOG
	if ( !g_bQuerySyslog )
	{
#endif

		tBuf << '[';
		sphFormatCurrentTime ( tBuf );
		tBuf << ']';

#if USE_SYSLOG
	} else
		tBuf += "[query]";
#endif

	// querytime sec
	int iQueryTime = Max ( tMeta.m_iQueryTime, 0 );
	int iRealTime = Max ( tMeta.m_iRealQueryTime, 0 );
	tBuf.Appendf ( " %d.%03d sec", iRealTime/1000, iRealTime%1000 );
	tBuf.Appendf ( " %d.%03d sec", iQueryTime/1000, iQueryTime%1000 );

	// optional multi-query multiplier
	if ( tMeta.m_iMultiplier>1 )
		tBuf.Appendf ( " x%d", tMeta.m_iMultiplier );

	// [matchmode/numfilters/sortmode matches (offset,limit)
	static const char * sModes [ SPH_MATCH_TOTAL ] = { "all", "any", "phr", "bool", "ext", "scan", "ext2" };
	static const char * sSort [ SPH_SORT_TOTAL ] = { "rel", "attr-", "attr+", "tsegs", "ext", "expr" };
	tBuf.Appendf ( " [%s/%d/%s " INT64_FMT " (%d,%d)",
		sModes [ tQuery.m_eMode ], tQuery.m_dFilters.GetLength(), sSort [ tQuery.m_eSort ], tMeta.m_iTotalMatches,
		tQuery.m_iOffset, tQuery.m_iLimit );

	// optional groupby info
	if ( !tQuery.m_sGroupBy.IsEmpty() )
		tBuf.Appendf ( " @%s", tQuery.m_sGroupBy.cstr() );

	// ] [indexes]
	tBuf.Appendf ( "] [%s]", tQuery.m_sIndexes.cstr() );

	// optional performance counters
	if ( g_bIOStats || g_bCpuStats )
	{
		const CSphIOStats & IOStats = tMeta.m_tIOStats;

		tBuf += " [";

		if ( g_bIOStats )
			tBuf.Appendf ( "ios=%d kb=%d.%d ioms=%d.%d",
				IOStats.m_iReadOps, (int)( IOStats.m_iReadBytes/1024 ), (int)( IOStats.m_iReadBytes%1024 )*10/1024,
				(int)( IOStats.m_iReadTime/1000 ), (int)( IOStats.m_iReadTime%1000 )/100 );

		if ( g_bIOStats && g_bCpuStats )
			tBuf += " ";

		if ( g_bCpuStats )
			tBuf.Sprintf ( "cpums=%.1D", tMeta.m_iCpuTime/100 );

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
	ScopedComma_c tComma ( tBuf, ",", " INDEX (", ")" );
	for ( const auto & sIndex : dIndexes )
		tBuf << sIndex;
}


static void FormatIndexHints ( const CSphQuery & tQuery, StringBuilder_c & tBuf )
{
	ScopedComma_c sMatch ( tBuf, nullptr );
	StrVec_t dUse, dForce, dIgnore;
	for ( const auto & i : tQuery.m_dIndexHints )
	{
		switch ( i.m_dHints[int(SecondaryIndexType_e::INDEX)] )
		{
		case IndexHint_e::USE:
			dUse.Add(i.m_sIndex);
			break;
		case IndexHint_e::FORCE:
			dForce.Add(i.m_sIndex);
			break;
		case IndexHint_e::IGNORE_:
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

static void LogQueryJson ( const CSphQuery & q, StringBuilder_c & tBuf )
{
	tBuf << " /*" << q.m_sRawQuery << " */";
}

static void LogQuerySphinxql ( const CSphQuery & q, const CSphQueryResultMeta & tMeta, const CSphVector<int64_t> & dAgentTimes )
{
	assert ( g_eLogFormat==LOG_FORMAT_SPHINXQL );
	if ( g_iQueryLogFile<0 )
		return;

	QuotationEscapedBuilder tBuf;
	int iCompactIN = ( g_bLogCompactIn ? LOG_COMPACT_IN : 0 );

	// time, conn id, wall, found
	int iQueryTime = Max ( tMeta.m_iQueryTime, 0 );
	int iRealTime = Max ( tMeta.m_iRealQueryTime, 0 );

	tBuf  << "/* ";
	sphFormatCurrentTime ( tBuf );

	if ( tMeta.m_iMultiplier>1 )
		tBuf << " conn " << session::GetConnID() << " real " << FixedFrac ( iRealTime )
			 << " wall " << FixedFrac ( iQueryTime ) << " x" << tMeta.m_iMultiplier << " found " << tMeta.m_iTotalMatches << " */ ";
	else
		tBuf << " conn " << session::GetConnID() << " real " << FixedFrac ( iRealTime )
			 << " wall " << FixedFrac ( iQueryTime ) << " found " << tMeta.m_iTotalMatches << " */ ";

	///////////////////////////////////
	// format request as SELECT query
	///////////////////////////////////

	if ( q.m_eQueryType==QUERY_JSON )
		LogQueryJson ( q, tBuf );
	else
		FormatSphinxql ( q, iCompactIN, tBuf );

	///////////////
	// query stats
	///////////////

	// next block ecnlosed in /* .. */, space-separated
	tBuf.StartBlock ( " ", " /*", " */" );
	if ( !tMeta.m_sError.IsEmpty() )
	{
		// all we have is an error
		tBuf.Appendf ( "error=%s", tMeta.m_sError.cstr() );

	} else if ( g_bIOStats || g_bCpuStats || dAgentTimes.GetLength() || !tMeta.m_sWarning.IsEmpty() )
	{
		// performance counters
		if ( g_bIOStats || g_bCpuStats )
		{
			const CSphIOStats & IOStats = tMeta.m_tIOStats;

			if ( g_bIOStats )
				tBuf.Sprintf ( "ios=%d kb=%d.%d ioms=%.1D",
				IOStats.m_iReadOps, (int)( IOStats.m_iReadBytes/1024 ), (int)( IOStats.m_iReadBytes%1024 )*10/1024,
				IOStats.m_iReadTime/100 );

			if ( g_bCpuStats )
				tBuf.Sprintf ( "cpums=%.1D", tMeta.m_iCpuTime/100 );
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
		if ( !tMeta.m_sWarning.IsEmpty() )
			tBuf.Appendf ( "warning=%s", tMeta.m_sWarning.cstr() );
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
		tBuf << "SELECT * FROM (";

	if ( q.m_sSelect.IsEmpty() )
		tBuf << "SELECT * FROM " << q.m_sIndexes;
	else
		tBuf << "SELECT " << RemoveBackQuotes ( q.m_sSelect.cstr() ) << " FROM " << q.m_sIndexes;

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
		tBuf << " GROUP BY " << q.m_sGroupBy;
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
		tBuf << " LIMIT " << q.m_iOffset << ',' << q.m_iLimit;

	// OPTION clause
	FormatOption ( q, tBuf );
	FormatIndexHints ( q, tBuf );

	// outer order by, limit
	if ( q.m_bHasOuter )
	{
		tBuf << ')';
		if ( !q.m_sOuterOrderBy.IsEmpty() )
			tBuf << " ORDER BY " << q.m_sOuterOrderBy;
		if ( q.m_iOuterOffset>0 )
			tBuf << " LIMIT " << q.m_iOuterOffset << ", " << q.m_iOuterLimit;
		else if ( q.m_iOuterLimit>0 )
			tBuf << " LIMIT " << q.m_iOuterLimit;
	}

	// finish SQL statement
	tBuf << ';';
}

static void LogQuery ( const CSphQuery & q, const CSphQueryResultMeta & tMeta, const CSphVector<int64_t> & dAgentTimes )
{
	if ( g_iQueryLogMinMs>0 && tMeta.m_iQueryTime<g_iQueryLogMinMs )
		return;

	switch ( g_eLogFormat )
	{
		case LOG_FORMAT_PLAIN:		LogQueryPlain ( q, tMeta ); break;
		case LOG_FORMAT_SPHINXQL:	LogQuerySphinxql ( q, tMeta, dAgentTimes ); break;
	}
}


void LogSphinxqlError ( const char * sStmt, const Str_t& sError )
{
	if ( g_eLogFormat!=LOG_FORMAT_SPHINXQL || g_iQueryLogFile<0 || !sStmt || IsEmpty(sError) )
		return;

	StringBuilder_c tBuf;
	tBuf << "/* ";
	sphFormatCurrentTime ( tBuf );
	tBuf << " conn " << session::GetConnID() << " */ " << sStmt << " # error=" << sError << '\n';

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
			sOut << "index " << dLog[iSpanStart].m_sParentIndex << ": ";
			return;
		}
	}

	// report only first indexes up to 4
	int iEndReport = ( iSpanLen>4 ) ? iSpanStart+3 : iSpandEnd;
	sOut.StartBlock ( ",", "index " );
	for ( int j = iSpanStart; j<iEndReport; ++j )
		sOut << dLog[j].m_sIndex;
	sOut.FinishBlock ();

	// add total index count
	if ( iEndReport!=iSpandEnd )
		sOut.Sprintf ( " and %d more: ", iSpandEnd-iEndReport );
	else
		sOut += ": ";
}


static void LogStatementSphinxql ( Str_t sQuery, int iRealTime )
{
	if ( g_iQueryLogFile<0 || g_eLogFormat!=LOG_FORMAT_SPHINXQL || !IsFilled ( sQuery ) )
		return;

	StringBuilder_c tBuf;
	tBuf << "/* ";
	sphFormatCurrentTime ( tBuf );
	tBuf << " conn " << session::GetConnID() << " real " << FixedFrac ( iRealTime ) << " */ "

	// query
		<< sQuery

	// finish statement and line feed
		<< ";\n";

	sphSeek ( g_iQueryLogFile, 0, SEEK_END );
	sphWrite ( g_iQueryLogFile, tBuf.cstr(), tBuf.GetLength() );
}


static int64_t LogFilterStatementSphinxql ( Str_t sQuery, SqlStmt_e eStmt )
{
	if ( g_tLogStatements.IsEmpty() )
		return 0;

	if ( !g_tLogStatements.BitGet ( eStmt ) )
		return 0;

	int64_t tmStarted = sphMicroTimer();
	LogStatementSphinxql ( sQuery, 0 );
	return tmStarted;
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

	for ( int i = 0; i < iCount; ++i )
		if ( !sphIsInternalAttr ( tSchema.GetAttr(i) ) )
			tAttrs.BitSet(i);

	int iId = tSchema.GetAttrIndex ( sphGetDocidName() );
	if ( !bAgentMode && iId!=-1 && !bNeedId )
		tAttrs.BitClear(iId);
}

static void SendDataPtrAttr ( ISphOutputBuffer& tOut, const BYTE * pData )
{
	auto dData = sphUnpackPtrAttr ( pData );
	tOut.SendArray ( dData );
}

static char g_sJsonNull[] = "{}";

static void SendJsonAsString ( ISphOutputBuffer& tOut, const BYTE * pJSON )
{
	if ( pJSON )
	{
		auto dData = sphUnpackPtrAttr ( pJSON );
		JsonEscapedBuilder dJson;
		dJson.GrowEnough ( dData.second * 2 );
		sphJsonFormat ( dJson, dData.first );
		tOut.SendArray ( dJson );
	} else
		// magic zero - "{}"
		tOut.SendArray ( g_sJsonNull, sizeof ( g_sJsonNull )-1 );
}


static void SendJson ( ISphOutputBuffer& tOut, const BYTE * pJSON, bool bSendJson )
{
	if ( bSendJson )
		SendDataPtrAttr ( tOut, pJSON ); // send BSON
	else
		SendJsonAsString ( tOut, pJSON ); // send string
}


static void SendJsonFieldAsString ( ISphOutputBuffer& tOut, const BYTE * pJSON )
{
	if ( !pJSON )
	{
		tOut.SendDword(0);
		return;
	}

	auto dData = sphUnpackPtrAttr ( pJSON );
	auto eJson = (ESphJsonType) *dData.first++;

	JsonEscapedBuilder dJson;
	dJson.GrowEnough ( dData.second * 2 );
	sphJsonFieldFormat ( dJson, dData.first, eJson, false );
	tOut.SendArray ( dJson );
}


static void SendJsonField ( ISphOutputBuffer& tOut, const BYTE * pJSON, bool bSendJsonField )
{
	if ( !bSendJsonField )
	{
		SendJsonFieldAsString ( tOut, pJSON );
		return;
	}

	auto dData = sphUnpackPtrAttr ( pJSON );
	if ( IsNull ( dData ) || *dData.first==JSON_EOF )
		tOut.SendByte ( JSON_EOF );
	else
	{
		tOut.SendByte ( *dData.first );
		tOut.SendArray ( dData.first+1, dData.second-1 );
	}
}


static void SendMVA ( ISphOutputBuffer& tOut, const BYTE * pMVA, bool b64bit )
{
	if ( !pMVA )
	{
		tOut.SendDword ( 0 );
		return;
	}

	auto dMVA = sphUnpackPtrAttr ( pMVA );
	DWORD uValues = dMVA.second / sizeof(DWORD);
	tOut.SendDword(uValues);

	const auto * pValues = (const DWORD *) dMVA.first;

	if ( b64bit )
	{
		assert ( ( uValues%2 )==0 );
		while ( uValues )
		{
			auto uMVA = MVA_BE ( pValues );
			tOut.SendDword ( uMVA.first );
			tOut.SendDword ( uMVA.second );
			pValues += 2;
			uValues -= 2;
		}
	} else
	{
		while ( uValues-- )
			tOut.SendDword ( *pValues++ );
	}
}


static ESphAttr FixupAttrForNetwork ( const CSphColumnInfo & tCol, const CSphSchema & tSchema, int iVer, WORD uMasterVer, bool bAgentMode )
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
		if ( bAgentMode && uMasterVer>=18 && IsNotRealAttribute ( tCol ) )
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

	case SPH_ATTR_DOUBLE:
		return iVer<0x122 ? SPH_ATTR_FLOAT : SPH_ATTR_DOUBLE;

	default: return tCol.m_eAttrType;
	} 
}


static void SendSchema ( ISphOutputBuffer & tOut, const AggrResult_t & tRes, const CSphBitvec & tAttrsToSend, int iVer, WORD uMasterVer, bool bAgentMode )
{
	int iFieldsCount = tRes.m_tSchema.GetFieldsCount();
	tOut.SendInt ( iFieldsCount );
	for ( int i=0; i < iFieldsCount; ++i )
		tOut.SendString ( tRes.m_tSchema.GetFieldName(i) );

	tOut.SendInt ( tAttrsToSend.BitCount() );
	for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); ++i )
	{
		if ( !tAttrsToSend.BitGet(i) )
			continue;

		const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
		tOut.SendString ( tCol.m_sName.cstr() );

		ESphAttr eCol = FixupAttrForNetwork ( tCol, tRes.m_tSchema, iVer, uMasterVer, bAgentMode );
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
		SendMVA ( tOut, (const BYTE*)tMatch.GetAttr(tLoc), tAttr.m_eAttrType==SPH_ATTR_INT64SET_PTR );
		break;

	case SPH_ATTR_JSON_PTR:
		SendJson ( tOut, (const BYTE*)tMatch.GetAttr(tLoc), bSendJson );
		break;

	case SPH_ATTR_STRINGPTR:
		SendDataPtrAttr ( tOut, (const BYTE*)tMatch.GetAttr(tLoc) );
		break;

	case SPH_ATTR_JSON_FIELD_PTR:
		SendJsonField ( tOut, (const BYTE*)tMatch.GetAttr(tLoc), bSendJsonField );
		break;

	case SPH_ATTR_FACTORS:
	case SPH_ATTR_FACTORS_JSON:
		if ( iVer<0x11C )
		{
			tOut.SendDword ( 0 );
			break;
		}

		SendDataPtrAttr ( tOut, (const BYTE*)tMatch.GetAttr(tLoc) );
		break;

	case SPH_ATTR_FLOAT:
		tOut.SendFloat ( tMatch.GetAttrFloat(tLoc) );
		break;

	case SPH_ATTR_DOUBLE:
		if ( iVer<0x122 )
			tOut.SendFloat ( (float)tMatch.GetAttrDouble(tLoc) );
		else
			tOut.SendDouble ( tMatch.GetAttrDouble(tLoc) );
		break;

	case SPH_ATTR_BIGINT:
		tOut.SendUint64 ( tMatch.GetAttr(tLoc) );
		break;

	default:
		tOut.SendDword ( (DWORD)tMatch.GetAttr(tLoc) );
		break;
	}
}


void SendResult ( int iVer, ISphOutputBuffer & tOut, const AggrResult_t& tRes, bool bAgentMode, const CSphQuery & tQuery, WORD uMasterVer )
{
	// multi-query status
	bool bError = !tRes.m_sError.IsEmpty();
	bool bWarning = !bError && !tRes.m_sWarning.IsEmpty();

	assert ( bError || tRes.m_bSingle );
	assert ( bError || tRes.m_bOneSchema );

	if ( bError )
	{
		tOut.SendInt ( SEARCHD_ERROR ); // fixme! m.b. use APICommand_t and refactor to common API way
		tOut.SendString ( tRes.m_sError.cstr() );
		if ( g_bOptNoDetach && g_eLogFormat!=LOG_FORMAT_SPHINXQL )
			sphInfo ( "query error: %s", tRes.m_sError.cstr() );
		return;

	} else if ( bWarning )
	{
		tOut.SendDword ( SEARCHD_WARNING );
		tOut.SendString ( tRes.m_sWarning.cstr() );
		if ( g_bOptNoDetach && g_eLogFormat!=LOG_FORMAT_SPHINXQL )
			sphInfo ( "query warning: %s", tRes.m_sWarning.cstr() );
	} else
		tOut.SendDword ( SEARCHD_OK );

	CSphBitvec tAttrsToSend;
	sphGetAttrsToSend ( tRes.m_tSchema, bAgentMode, false, tAttrsToSend );

	// send schema
	SendSchema ( tOut, tRes, tAttrsToSend, iVer, uMasterVer, bAgentMode );

	// send matches
	tOut.SendInt ( tRes.m_iCount );
	tOut.SendInt ( 1 ); // was USE_64BIT

	CSphVector<BYTE> dJson ( 512 );

	auto& dResult = tRes.m_dResults.First();
	auto dMatches = dResult.m_dMatches.Slice ( tRes.m_iOffset, tRes.m_iCount );

	for ( const CSphMatch & tMatch : dMatches )
	{
		Verify ( tRes.m_tSchema.GetAttr(sphGetDocidName()) );
		tOut.SendUint64 ( sphGetDocID(tMatch.m_pDynamic) );
		tOut.SendInt ( tMatch.m_iWeight );

		assert ( tMatch.m_pStatic || !tRes.m_tSchema.GetStaticSize() );
#if 0
		// not correct any more because of internal attrs (such as string sorting ptrs)
		assert ( tMatch.m_pDynamic || !pRes->m_tSchema.GetDynamicSize() );
		assert ( !tMatch.m_pDynamic || (int)tMatch.m_pDynamic[-1]==pRes->m_tSchema.GetDynamicSize() );
#endif
		for ( int j=0; j<tRes.m_tSchema.GetAttrsCount(); ++j )
			if ( tAttrsToSend.BitGet(j) )
				SendAttribute ( tOut, tMatch, tRes.m_tSchema.GetAttr(j), iVer, uMasterVer, bAgentMode );
	}

	if ( tQuery.m_bAgent && tQuery.m_iLimit )
		tOut.SendInt ( tRes.m_iCount );
	else
		tOut.SendInt ( dResult.m_dMatches.GetLength() );

	tOut.SendAsDword ( tRes.m_iTotalMatches );
	if ( bAgentMode && uMasterVer>=19 )
		tOut.SendInt ( tRes.m_bTotalMatchesApprox ? 1 : 0 );

	tOut.SendInt ( Max ( tRes.m_iQueryTime, 0 ) );

	if ( iVer>=0x11A && bAgentMode )
	{
		bool bNeedPredictedTime = tQuery.m_iMaxPredictedMsec > 0;

		BYTE uStatMask = ( bNeedPredictedTime ? 4U : 0U ) | ( g_bCpuStats ? 2U : 0U ) | ( g_bIOStats ? 1U : 0U );
		tOut.SendByte ( uStatMask );

		if ( g_bIOStats )
		{
			CSphIOStats tStats = tRes.m_tIOStats;
			tStats.Add ( tRes.m_tAgentIOStats );
			tOut.SendUint64 ( tStats.m_iReadTime );
			tOut.SendDword ( tStats.m_iReadOps );
			tOut.SendUint64 ( tStats.m_iReadBytes );
			tOut.SendUint64 ( tStats.m_iWriteTime );
			tOut.SendDword ( tStats.m_iWriteOps );
			tOut.SendUint64 ( tStats.m_iWriteBytes );
		}

		if ( g_bCpuStats )
		{
			int64_t iCpuTime = tRes.m_iCpuTime + tRes.m_iAgentCpuTime;
			tOut.SendUint64 ( iCpuTime );
		}

		if ( bNeedPredictedTime )
			tOut.SendUint64 ( tRes.m_iPredictedTime + tRes.m_iAgentPredictedTime );
	}
	if ( bAgentMode && uMasterVer>=7 )
	{
		tOut.SendDword ( tRes.m_tStats.m_iFetchedDocs + tRes.m_iAgentFetchedDocs );
		tOut.SendDword ( tRes.m_tStats.m_iFetchedHits + tRes.m_iAgentFetchedHits );
		if ( uMasterVer>=8 )
			tOut.SendDword ( tRes.m_tStats.m_iSkips + tRes.m_iAgentFetchedSkips );
	}

	auto dWords = tRes.MakeSortedWordStat ();
	tOut.SendInt ( dWords.GetLength() );
	for( auto * pWord : dWords )
	{
		assert ( pWord );
		tOut.SendString ( pWord->first.cstr () );
		tOut.SendAsDword ( pWord->second.first );
		tOut.SendAsDword ( pWord->second.second );
		if ( bAgentMode )
			tOut.SendByte ( 0 ); // statistics have no expanded terms for now
	}
}

/////////////////////////////////////////////////////////////////////////////

int AggrResult_t::GetLength () const
{
	int iCount = 0;
	m_dResults.Apply ( [&iCount] ( const OneResultset_t & a ) { iCount += a.m_dMatches.GetLength (); } );
	return iCount;
}

bool AggrResult_t::AddResultset ( ISphMatchSorter * pQueue, const DocstoreReader_i * pDocstore, int iTag, int iCutoff )
{
	assert ( pQueue );

	if ( !pQueue->GetLength () )
	{
		m_tSchema = *pQueue->GetSchema ();
		return false;
	}

	// extract matches from sorter
	auto & tOneRes = m_dResults.Add ();
	tOneRes.m_pDocstore = pDocstore;
	tOneRes.m_iTag = iTag;
	tOneRes.FillFromSorter ( pQueue );

	// in MT case each thread has its own cutoff, so we have to enforce it again on the result set
	if ( iCutoff>0 )
	{
		m_iTotalMatches = Min ( iCutoff, m_iTotalMatches );
		tOneRes.ClampMatches(iCutoff);
	}

	return true;
}

void AggrResult_t::AddEmptyResultset ( const DocstoreReader_i * pDocstore, int iTag )
{
	auto & tOneRes = m_dResults.Add();
	tOneRes.m_pDocstore = pDocstore;
	tOneRes.m_iTag = iTag;
}


void AggrResult_t::ClampMatches ( int iLimit )
{
	assert ( m_bSingle );
	m_dResults.First ().ClampMatches ( iLimit );
}

void AggrResult_t::ClampAllMatches ()
{
	for ( auto& dResult : m_dResults )
		dResult.ClampAllMatches();
}

int OneResultset_t::FillFromSorter ( ISphMatchSorter * pQueue )
{
	if ( !pQueue )
		return 0;

	assert ( m_dMatches.IsEmpty () );
	m_tSchema = *pQueue->GetSchema ();
	if ( !pQueue->GetLength () )
		return 0;

	int iCopied = pQueue->Flatten ( m_dMatches.AddN ( pQueue->GetLength () ) );
	m_dMatches.Resize ( iCopied );
	return iCopied;
}

void OneResultset_t::ClampAllMatches ()
{
	for ( auto& dMatch : m_dMatches )
	{
		m_tSchema.FreeDataPtrs ( dMatch );
		dMatch.ResetDynamic();
	}
	m_dMatches.Reset();
}

void OneResultset_t::ClampMatches ( int iLimit )
{
	assert ( iLimit>0 );

	int iMatches = m_dMatches.GetLength ();
	for ( int i = iLimit; i<iMatches; ++i )
	{
		m_tSchema.FreeDataPtrs ( m_dMatches[i] );
		m_dMatches[i].ResetDynamic();
	}
	m_dMatches.Resize ( Min (iMatches, iLimit ) );
}

OneResultset_t::~OneResultset_t()
{
	ClampAllMatches();
}

namespace { // static

void RemapResult ( AggrResult_t & dResult )
{
	const ISphSchema & tSchema = dResult.m_tSchema;
	int iAttrsCount = tSchema.GetAttrsCount();
	CSphVector<int> dMapFrom(iAttrsCount);
	CSphVector<int> dRowItems(iAttrsCount);
	static const int SIZE_OF_ROW = 8 * sizeof ( CSphRowitem );

	for ( auto & tRes : dResult.m_dResults )
	{
		if ( tRes.m_dMatches.IsEmpty() )
			continue;

		dMapFrom.Resize ( 0 );
		dRowItems.Resize ( 0 );
		CSphSchema & dSchema = tRes.m_tSchema;
		for ( int i = 0; i<iAttrsCount; ++i )
		{
			auto iSrcCol = dSchema.GetAttrIndex ( tSchema.GetAttr ( i ).m_sName.cstr () );
			dMapFrom.Add ( iSrcCol );
			dRowItems.Add ( dSchema.GetAttr ( iSrcCol ).m_tLocator.m_iBitOffset / SIZE_OF_ROW );
			assert ( dMapFrom[i]>=0
				|| IsSortStringInternal ( tSchema.GetAttr(i).m_sName )
				|| IsSortJsonInternal ( tSchema.GetAttr(i).m_sName )
				);
		}

		// inverse dRowItems - we'll free only those NOT enumerated yet
		dRowItems = dSchema.SubsetPtrs ( dRowItems );
		for ( auto& tMatch : tRes.m_dMatches )
		{
			// create new and shiny (and properly sized) match
			CSphMatch tNewMatch;
			tNewMatch.Reset ( tSchema.GetDynamicSize () );
			tNewMatch.m_tRowID = tMatch.m_tRowID;
			tNewMatch.m_iWeight = tMatch.m_iWeight;

			// remap attrs
			for ( int j = 0; j<iAttrsCount; ++j )
			{
				const CSphColumnInfo & tDst = tSchema.GetAttr ( j );
				// we could keep some of the rows static
				// and so, avoid the duplication of the data.
				int iMapFrom = dMapFrom[j];
				const CSphColumnInfo & tSrc = dSchema.GetAttr ( iMapFrom );
				if ( !tDst.m_tLocator.m_bDynamic )
				{
					assert ( iMapFrom<0 || !dSchema.GetAttr ( iMapFrom ).m_tLocator.m_bDynamic );
					tNewMatch.m_pStatic = tMatch.m_pStatic;
				} else if ( iMapFrom>=0 )
				{
					if ( tDst.m_eAttrType==SPH_ATTR_FLOAT && tSrc.m_eAttrType==SPH_ATTR_BOOL )
					{
						tNewMatch.SetAttrFloat ( tDst.m_tLocator, ( tMatch.GetAttr ( tSrc.m_tLocator )>0 ? 1.0f : 0.0f ) );
					} else
					{
						tNewMatch.SetAttr ( tDst.m_tLocator, tMatch.GetAttr ( tSrc.m_tLocator ) );
					}
				}
			}
			// swap out old (most likely wrong sized) match
			Swap ( tMatch, tNewMatch );
			CSphSchemaHelper::FreeDataSpecial ( tNewMatch, dRowItems );
		}
	}
}


bool GetIndexSchemaItems ( const ISphSchema & tSchema, const CSphVector<CSphQueryItem> & dItems, CSphVector<int> & dAttrs )
{
	bool bHaveAsterisk = false;
	for ( const auto & i : dItems )
	{
		if ( i.m_sAlias.cstr() )
		{
			int iAttr = tSchema.GetAttrIndex ( i.m_sAlias.cstr() );
			if ( iAttr>=0 )
				dAttrs.Add(iAttr);
		}

		bHaveAsterisk |= i.m_sExpr=="*";
	}

	dAttrs.Sort();
	return bHaveAsterisk;
}


bool GetItemsLeftInSchema ( const ISphSchema & tSchema, bool bOnlyPlain, const CSphVector<int> & dAttrs, CSphVector<int> & dAttrsInSchema )
{	
	bool bHaveExprs = false;

	for ( int i = 0, iAttrsCount = tSchema.GetAttrsCount (); i<iAttrsCount; ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);

		if ( tAttr.m_pExpr )
		{
			bHaveExprs = true;

			// need to keep post-limit expression (stored field) for multi-query \ facet
			// also keep columnar attributes (with expressions)
			if ( bOnlyPlain && !tAttr.m_pExpr->IsColumnar() && tAttr.m_eStage!=SPH_EVAL_POSTLIMIT )
				continue;
		}

		if ( tAttr.m_sName.cstr()[0]!='@' && !dAttrs.BinarySearch(i) )
			dAttrsInSchema.Add(i);
	}

	return bHaveExprs;
}


void DoExpansion ( const ISphSchema & tSchema, const CSphVector<int> & dAttrsInSchema, const CSphVector<CSphQueryItem> & dItems, CSphVector<CSphQueryItem> & dExpanded )
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
	CSphVector<int> dIndexSchemaAttrs;
	bool bHaveAsterisk = GetIndexSchemaItems ( tSchema, dItems, dIndexSchemaAttrs );

	// no stars? Nothing to do.
	if ( !bHaveAsterisk )
		return dItems;

	// find items that are in index schema but not in our requested item list
	// not do not include @-items
	CSphVector<int> dAttrsLeftInSchema;
	bHaveExprs = GetItemsLeftInSchema ( tSchema, bOnlyPlain, dIndexSchemaAttrs, dAttrsLeftInSchema );

	DoExpansion ( tSchema, dAttrsLeftInSchema, dItems, tExpanded );

	return tExpanded;
}

// in MatchIterator_c we need matches sorted assending by DocID.
// also we don't want to sort matches themselves; sorted vec of indexes quite enough
// also we wont to avoid allocating vec for the matches as it may be huge.
// There are several possible solutions to have vec of indexes:
// 1. Use matches tags, as they're not used in this part of code. With intensive working it is however not a good in
// terms of cache misses (i.e. 'min' match is match[N] where N is match[0].tag, then match[M] where M is match[1] tag.
// So each time we make about random jumps.
// 2. Use space between last match and end of the vector (assuming reserved space > used space). If it is enough space,
// we can use it either as vec or WORDS, or as vec or DWORDS, depending from N of matches. First case need at most 128K
// of RAM, second needs more, but that RAM is compact.
// So, let's try with tail space first, but if it is not available (no, or not enough space), use tags.


// That is to sort tags in matches without moving rest of them.
class MatchTagSortAccessor_c
{
	const VecTraits_T<CSphMatch> & m_dTagOrder;
public:
	explicit MatchTagSortAccessor_c ( const VecTraits_T<CSphMatch> & dTagOrder) : m_dTagOrder ( dTagOrder ) {}
	using T = CSphMatch;
	using MEDIAN_TYPE = int;
	static MEDIAN_TYPE Key ( T * a ) { return a->m_iTag; }
	static void Swap ( T * a, T * b ) { ::Swap ( a->m_iTag, b->m_iTag ); }
	static T * Add ( T * p, int i ) { return p+i; }
	static int Sub ( T * b, T * a ) { return (int)(b-a); }
	static void CopyKey ( MEDIAN_TYPE * pMed, CSphMatch * pVal ) { *pMed = Key ( pVal ); }

	bool IsLess ( int a, int b ) const
	{
		return sphGetDocID ( m_dTagOrder[a].m_pDynamic )<sphGetDocID ( m_dTagOrder[b].m_pDynamic );
	}
};


class MatchIterator_c
{
	int m_iRawIdx;    // raw iteration index (internal)
	int m_iLimit;
	std::function<int(int)> m_fnOrder;	// use to access matches by accending docid order
	bool m_bTailClean = false;

	// use space after end of matches to store indexes, WORD per match
	bool MaybeUseWordOrder ( const CSphSwapVector<CSphMatch>& dMatches ) const
	{
		if ( dMatches.GetLength()>0x10000 )
			return false;

		int64_t iTail = dMatches.AllocatedBytes ()-dMatches.GetLengthBytes64 ();
		if ( iTail<(int64_t) ( dMatches.GetLength () * sizeof ( WORD ) ) )
			return false;

		// will use tail of the vec as blob of WORDs
		VecTraits_T<WORD> dOrder = { (WORD *) dMatches.end (), m_iLimit };
		ARRAY_CONSTFOREACH( i, dOrder )
			dOrder[i] = i;
		dOrder.Sort ( Lesser ( [&dMatches] ( WORD a, WORD b ) {
			return sphGetDocID ( dMatches[a].m_pDynamic )<sphGetDocID ( dMatches[b].m_pDynamic );
		} ) );
		return true;
	}

	// use space after end of matches to store indexes, DWORD per match
	bool MaybeUseDwordOrder ( const CSphSwapVector<CSphMatch>& dMatches ) const
	{
		if ( dMatches.GetLength64()>0x100000000 )
			return false;

		int64_t iTail = dMatches.AllocatedBytes ()-dMatches.GetLengthBytes64 ();
		if ( iTail<(int64_t) ( dMatches.GetLength () * sizeof ( DWORD ) ) )
			return false;

		// will use tail of the vec as blob of WORDs
		VecTraits_T<DWORD> dOrder = { (DWORD *) dMatches.end (), m_iLimit };
		for( DWORD i=0, uLen=dOrder.GetLength(); i<uLen; ++i )
			dOrder[i] = i;
		dOrder.Sort ( Lesser ( [&dMatches] ( DWORD a, DWORD b ) {
			return sphGetDocID ( dMatches[a].m_pDynamic )<sphGetDocID ( dMatches[b].m_pDynamic );
		} ) );
		return true;
	}

	// use tags to store indexes. No extra space, but random access order, many cash misses expected
	void UseTags ( VecTraits_T<CSphMatch> & dOrder )
	{
		ARRAY_CONSTFOREACH( i, dOrder )
			dOrder[i].m_iTag = i;

		MatchTagSortAccessor_c tOrder ( dOrder );
		sphSort ( dOrder.Begin (), dOrder.GetLength (), tOrder, tOrder );
		m_bTailClean = true;
	}

public:
	OneResultset_t&			m_tResult;
	DocID_t					m_tDocID;
	int						m_iIdx;		// ordering index (each step gives matches in sorted by Docid order)

	explicit MatchIterator_c ( OneResultset_t & tResult )
		: m_tResult ( tResult )
	{
		auto& dMatches = tResult.m_dMatches;
		m_iLimit = dMatches.GetLength();

		if ( MaybeUseWordOrder ( dMatches ) )
			m_fnOrder = [pData = (WORD *) m_tResult.m_dMatches.end ()] ( int i ) { return pData[i]; };
		else if ( MaybeUseDwordOrder ( dMatches ) )
			m_fnOrder = [pData = (DWORD *) m_tResult.m_dMatches.end ()] ( int i ) { return pData[i]; };
		else
		{
			UseTags ( dMatches );
			m_fnOrder = [this] ( int i ) { return m_tResult.m_dMatches[m_iRawIdx].m_iTag; };
		}

		m_iRawIdx = 0;
		m_iIdx = m_fnOrder(0);

		assert ( m_tResult.m_tSchema.GetAttr ( sphGetDocidName() ) );
		m_tDocID = sphGetDocID ( m_tResult.m_dMatches[m_iIdx].m_pDynamic );
	}

	~MatchIterator_c()
	{
		if ( m_bTailClean )
			return;

		// need to reset state of some tail matches in order to avoid issues when deleting the vec of them
		// (since we used that memory region for own purposes)
		int iDirtyMatches = m_iLimit>0x10000 ? m_iLimit * sizeof ( DWORD ) : m_iLimit * sizeof ( WORD );
		iDirtyMatches = ( iDirtyMatches+sizeof ( CSphMatch )-1 ) / sizeof ( CSphMatch );
		for ( int i = 0; i<iDirtyMatches; ++i )
			( m_tResult.m_dMatches.end ()+i )->CleanGarbage();
	}

	inline bool Step()
	{
		++m_iRawIdx;
		if ( m_iRawIdx>=m_iLimit )
			return false;
		m_iIdx = m_fnOrder ( m_iRawIdx );
		m_tDocID = sphGetDocID ( m_tResult.m_dMatches[m_iIdx].m_pDynamic );
		return true;
	}

	static inline bool IsLess ( MatchIterator_c *a, MatchIterator_c *b )
	{
		if ( a->m_tDocID!=b->m_tDocID )
			return a->m_tDocID<b->m_tDocID;

		// that mean local matches always preffered over remote, but it seems that is not necessary
//		if ( !a->m_dResult.m_bTag && b->m_dResult.m_bTag )
//			return true;

		return a->m_tResult.m_iTag>b->m_tResult.m_iTag;
	}
};

int KillPlainDupes ( ISphMatchSorter * pSorter, AggrResult_t & tRes )
{
	int iDupes = 0;

	auto& dResults = tRes.m_dResults;

	// normal sorter needs massage
	// queue by docid and then ascending by tag to guarantee the replacement order
	RawVector_T <MatchIterator_c> dIterators;
	dIterators.Reserve_static ( dResults.GetLength () );
	CSphQueue<MatchIterator_c *, MatchIterator_c> qMatches ( dResults.GetLength () );

	for ( auto & tResult : dResults )
		if ( !tResult.m_dMatches.IsEmpty() )
		{
			dIterators.Emplace_back(tResult);
			qMatches.Push ( &dIterators.Last() );
		}

	DocID_t tPrevDocID = DOCID_MIN;
	while ( qMatches.GetLength() )
	{
		auto * pMin = qMatches.Root();
		DocID_t tDocID = pMin->m_tDocID;
		if ( tDocID!=tPrevDocID ) // by default, simply remove dupes (select first by tag)
		{
			CSphMatch & tMatch = pMin->m_tResult.m_dMatches[pMin->m_iIdx];
			auto iTag = tMatch.m_iTag;	// as we may use tag for ordering
			if ( !pMin->m_tResult.m_bTagsAssigned )
				tMatch.m_iTag = pMin->m_tResult.m_iTag; // that will link us back to docstore

			pSorter->Push ( tMatch );
			tMatch.m_iTag = iTag;	// restore tag
			tPrevDocID = tDocID;
		}
		else
			++iDupes;

		qMatches.Pop ();
		if ( pMin->Step() )
			qMatches.Push ( pMin );
	}
	tRes.m_bTagsAssigned = true;
	return iDupes;
}

int KillGroupbyDupes ( ISphMatchSorter * pSorter, AggrResult_t & tRes, const VecTraits_T<int>& dOrd )
{
	int iDupes = 0;
	pSorter->SetBlobPool ( nullptr );
	for ( int iOrd : dOrd )
	{
		auto & tResult = tRes.m_dResults[iOrd];
		ARRAY_CONSTFOREACH( i, tResult.m_dMatches )
		{
			CSphMatch & tMatch = tResult.m_dMatches[i];
			if ( !tResult.m_bTagsAssigned )
				tMatch.m_iTag = tResult.m_iTag; // that will link us back to docstore

			if ( !pSorter->PushGrouped ( tMatch, i==0 ) )  // groupby sorter does that automagically
				++iDupes;
		}
	}
	tRes.m_bTagsAssigned = true;
	return iDupes;
}

// rearrange results so thet the're placed by accending tags order
// dOrd contains indexes to access results in descending tag order
void SortTagsAndDocstores ( AggrResult_t & tRes, const VecTraits_T<int>& dOrd )
{
	auto iTags = dOrd.GetLength ();
	CSphFixedVector<DocstoreAndTag_t> dTmp { iTags };
	auto & dResults = tRes.m_dResults;

	for ( int i=0; i<iTags; ++i )
		dTmp[iTags-i-1].Assign ( dResults[dOrd[i]] );

	for ( int i = 0; i<iTags; ++i )
		dResults[i].Assign ( dTmp[i] );

	Debug ( tRes.m_bIdxByTag = true; )
}

int KillDupesAndFlatten ( ISphMatchSorter * pSorter, AggrResult_t & tRes )
{
	assert ( pSorter );

	int iTags = tRes.m_dResults.GetLength();
	CSphFixedVector<int> dOrd ( iTags );
	ARRAY_CONSTFOREACH( i, dOrd )
		dOrd[i] = i;

	// sort resultsets in descending tag order
	dOrd.Sort ( Lesser ( [&tRes] ( int l, int r ) { return tRes.m_dResults[r].m_iTag<tRes.m_dResults[l].m_iTag; } ) );

	// remap to compact (non-fragmented) range of tags
	for ( int iRes : dOrd )
		tRes.m_dResults[iRes].m_iTag = --iTags;
	Debug ( tRes.m_bTagsCompacted = true );

	// do actual deduplication
	int iDup = pSorter->IsGroupby() ? KillGroupbyDupes ( pSorter, tRes, dOrd ) : KillPlainDupes ( pSorter, tRes );

	// ALL matches have same schema, as KillAllDupes called after RemapResults(), or already having identical schemas.
	for ( auto& dResult : tRes.m_dResults )
	{
		for ( auto& dMatch : dResult.m_dMatches )
			tRes.m_tSchema.FreeDataPtrs ( dMatch );
		dResult.m_dMatches.Reset();
	}

	// don't issue tRes.m_dResults.reset since each result still has a docstore by tag

	// flatten all results into single chunk
	auto & tFinalMatches = tRes.m_dResults.First ();
	tFinalMatches.FillFromSorter ( pSorter );
	Debug ( tRes.m_bSingle = true; )
	Debug ( tRes.m_bOneSchema = true; )

	// now all matches properly tagged located in tRes.m_dResults.First()
	// each tRes.m_dResults has proper tag and corresponding docstore pointer in random order
	// and we have dOrd wich enumerates them in descending tag order
	SortTagsAndDocstores ( tRes, dOrd );
	return iDup;
}


void RecoverAggregateFunctions ( const CSphQuery & tQuery, const AggrResult_t & tRes )
{
	for ( const auto& tItem : tQuery.m_dItems )
	{
		if ( tItem.m_eAggrFunc==SPH_AGGR_NONE )
			continue;

		for ( int j = 0, iAttrsCount = tRes.m_tSchema.GetAttrsCount (); j<iAttrsCount; ++j )
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
			case SPH_KEYPART_DOUBLE:
			{
				register double aa = a->GetAttrDouble ( m_tLocator[i] );
				register double bb = b->GetAttrDouble ( m_tLocator[i] );
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

		return a->m_tRowID<b->m_tRowID;
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


void ExtractPostlimit ( const ISphSchema & tSchema, bool bMaster, CSphVector<const CSphColumnInfo *> & dPostlimit )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr ( i );
		if ( tCol.m_eStage==SPH_EVAL_POSTLIMIT && ( bMaster || tCol.m_uFieldFlags==CSphColumnInfo::FIELD_NONE ) )
			dPostlimit.Add ( &tCol );
	}
}

// for single chunk of matches return list of tags with docstores
CSphVector<int> GetUniqueTagsWithDocstores ( const AggrResult_t & tRes, int iOff, int iLim )
{
	assert ( tRes.m_bTagsCompacted );
	assert ( tRes.m_bSingle );

	CSphVector<bool> dBoolTags;
	dBoolTags.Resize ( tRes.m_dResults.GetLength() );
	dBoolTags.ZeroVec();

	auto dMatches = tRes.m_dResults.First ().m_dMatches.Slice ( iOff, iLim );
	for ( const auto& dMatch : dMatches )
	{
		assert ( dMatch.m_iTag < tRes.m_dResults.GetLength() );
		if ( tRes.m_dResults[dMatch.m_iTag].Docstore() )
			dBoolTags[dMatch.m_iTag] = true;
	}

	CSphVector<int> dTags;
	ARRAY_CONSTFOREACH( i, dBoolTags )
		if ( dBoolTags[i] )
			dTags.Add(i);

	return dTags;
}


void SetupPostlimitExprs ( const DocstoreReader_i * pDocstore, const CSphColumnInfo * pCol, const char * sQuery, int64_t iDocstoreSessionId )
{
	DocstoreSession_c::InfoDocID_t tSessionInfo;
	tSessionInfo.m_pDocstore = pDocstore;
	tSessionInfo.m_iSessionId = iDocstoreSessionId;

	assert ( pCol && pCol->m_pExpr );
	pCol->m_pExpr->Command ( SPH_EXPR_SET_DOCSTORE_DOCID, &tSessionInfo ); // value is copied; no leak of pointer to local here.
	pCol->m_pExpr->Command ( SPH_EXPR_SET_QUERY, (void *)sQuery);
}


void EvalPostlimitExprs ( CSphMatch & tMatch, const CSphColumnInfo * pCol )
{
	assert ( pCol && pCol->m_pExpr );

	switch ( pCol->m_eAttrType )
	{
	case SPH_ATTR_TIMESTAMP:
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_BOOL:
		tMatch.SetAttr ( pCol->m_tLocator, pCol->m_pExpr->IntEval ( tMatch ) );
		break;

	case SPH_ATTR_BIGINT:
		tMatch.SetAttr ( pCol->m_tLocator, pCol->m_pExpr->Int64Eval ( tMatch ) );
		break;

	case SPH_ATTR_STRINGPTR:
		// FIXME! a potential leak of *previous* value?
		tMatch.SetAttr ( pCol->m_tLocator, (SphAttr_t) pCol->m_pExpr->StringEvalPacked ( tMatch ) );
		break;

	default:
		tMatch.SetAttrFloat ( pCol->m_tLocator, pCol->m_pExpr->Eval ( tMatch ) );
		break;
	}
}

// single resultset cunk, but has many tags
void ProcessMultiPostlimit ( AggrResult_t & tRes, VecTraits_T<const CSphColumnInfo *> & dPostlimit, const char * sQuery, int iOff, int iLim )
{
	if ( dPostlimit.IsEmpty() )
		return;

	assert ( tRes.m_bSingle );
	assert ( tRes.m_bOneSchema );
	assert ( tRes.m_bTagsAssigned );
	assert ( tRes.m_bTagsCompacted );
	assert ( tRes.m_bIdxByTag );

	// collect unique tags from matches
	CSphVector<int> dDocstoreTags = GetUniqueTagsWithDocstores ( tRes, iOff, iLim );

	// generates docstore session id
	DocstoreSession_c tSession;
	auto iSessionUID = tSession.GetUID();

	// spawn buffered readers for the current session
	// put them to a global hash
	for ( int iTag : dDocstoreTags )
		tRes.m_dResults[iTag].m_pDocstore->CreateReader ( iSessionUID );

	int iLastTag = -1;
	auto dMatches = tRes.m_dResults.First ().m_dMatches.Slice ( iOff, iLim );
	for ( auto & dMatch : dMatches )
	{
		int iTag = dMatch.m_iTag;
		if ( tRes.m_dResults[iTag].m_bTag )
			continue; // remote match; everything should be precalculated

		auto * pDocstore = tRes.m_dResults[iTag].Docstore ();
		assert ( iTag<tRes.m_dResults.GetLength () );

		if ( iTag!=iLastTag )
		{
			for ( const auto & pCol : dPostlimit )
				SetupPostlimitExprs ( pDocstore, pCol, sQuery, iSessionUID );
			iLastTag = iTag;
		}

		for ( const auto & pCol : dPostlimit )
			EvalPostlimitExprs ( dMatch, pCol );
	}
}

void ProcessSinglePostlimit ( OneResultset_t & tRes, VecTraits_T<const CSphColumnInfo *> & dPostlimit, const char * sQuery, int iOff, int iLim )
{
	auto dMatches = tRes.m_dMatches.Slice ( iOff, iLim );
	if ( dMatches.IsEmpty() )
		return;

	// generates docstore session id
	DocstoreSession_c tSession;
	auto iSessionUID = tSession.GetUID();

	// spawn buffered reader for the current session
	// put it to a global hash
	if ( tRes.Docstore () )
		tRes.m_pDocstore->CreateReader ( iSessionUID );

	for ( const auto & pCol : dPostlimit )
		SetupPostlimitExprs ( tRes.Docstore (), pCol, sQuery, iSessionUID );

	for ( auto & tMatch : dMatches )
		for ( const auto & pCol : dPostlimit )
			EvalPostlimitExprs ( tMatch, pCol );
}

void ProcessLocalPostlimit ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bMaster )
{
	assert ( !tRes.m_bOneSchema );
	assert ( !tRes.m_bSingle );

	bool bGotPostlimit = false;
	for ( int i = 0, iAttrsCount = tRes.m_tSchema.GetAttrsCount (); i<iAttrsCount && !bGotPostlimit; ++i )
	{
		const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
		bGotPostlimit = ( tCol.m_eStage==SPH_EVAL_POSTLIMIT && ( bMaster || tCol.m_uFieldFlags==CSphColumnInfo::FIELD_NONE ) );
	}

	if ( !bGotPostlimit )
		return;

	int iLimit = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );
	iLimit += Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
	CSphVector<const CSphColumnInfo *> dPostlimit;
	for ( auto & tResult : tRes.m_dResults )
	{
		dPostlimit.Resize ( 0 );
		ExtractPostlimit ( tResult.m_tSchema, bMaster, dPostlimit );
		if ( dPostlimit.IsEmpty () )
			continue;

		iLimit = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );

		// we can't estimate limit.offset per result set
		// as matches got merged and sort next step
		if ( !tResult.m_bTag )
			ProcessSinglePostlimit ( tResult, dPostlimit, tQuery.m_sQuery.cstr (), 0, iLimit );
	}
}


bool MinimizeSchemas ( AggrResult_t & tRes )
{
	bool bAllEqual = true;
	bool bSchemaBaseSet = false;

	auto iResults = tRes.m_dResults.GetLength();

	for ( int i=0; i<iResults; ++i )
	{
		// skip empty result set
		if ( !tRes.m_dResults[i].m_dMatches.GetLength() )
			continue;

		// set base schema only from non-empty result set
		if ( !bSchemaBaseSet )
		{
			bSchemaBaseSet = true;
			tRes.m_tSchema = tRes.m_dResults[i].m_tSchema;
			continue;
		}

		if ( !MinimizeSchema ( tRes.m_tSchema, tRes.m_dResults[i].m_tSchema ) )
			bAllEqual = false;
	}

	if ( !bSchemaBaseSet && iResults>0 )
		tRes.m_tSchema = tRes.m_dResults[0].m_tSchema;

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


FrontendSchemaBuilder_c::FrontendSchemaBuilder_c ( const AggrResult_t & tRes, const CSphQuery & tQuery, const CSphVector<CSphQueryItem> & dItems, const CSphVector<CSphQueryItem> & dQueryItems,
		const sph::StringSet & hExtraColumns, bool bQueryFromAPI, bool bHaveLocals )
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
	ARRAY_CONSTFOREACH ( i, m_dItems )
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

	for ( int iCol=0; iCol<m_tRes.m_tSchema.GetAttrsCount(); ++iCol )
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
	ARRAY_CONSTFOREACH ( i, m_dItems )
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
	ARRAY_CONSTFOREACH ( i, m_dFrontend )
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

bool MergeAllMatches ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bHaveLocals, bool bAllEqual, bool bMaster, const CSphFilterSettings * pAggrFilter, QueryProfile_c * pProfiler )
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
		for ( auto & tResult : tRes.m_dResults )
			tResult.ClampMatches ( tQueryCopy.m_iLimit );
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
		tQueueSettings.m_iMaxMatches = Min ( tQuery.m_iMaxMatches * tRes.m_dResults.GetLength(), tRes.GetLength() );
	else
		tQueueSettings.m_iMaxMatches = Min ( tQuery.m_iMaxMatches, tRes.GetLength() );

	tQueueSettings.m_iMaxMatches = Max ( tQueueSettings.m_iMaxMatches, 1 );

	SphQueueRes_t tQueueRes;
	std::unique_ptr<ISphMatchSorter> pSorter ( sphCreateQueue ( tQueueSettings, tQueryCopy, tRes.m_sError, tQueueRes ) );

	// restore outer order related patches, or it screws up the query log
	if ( tQueryCopy.m_bHasOuter )
	{
		Swap ( tQueryCopy.m_sOuterOrderBy, tQueryCopy.m_sGroupBy.IsEmpty() ? tQueryCopy.m_sSortBy : tQueryCopy.m_sGroupSortBy );
		Swap ( eQuerySort, tQueryCopy.m_eSort );
	}

	if ( !pSorter )
		return false;

	pSorter->SetMerge(true);

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
	{
		CSphSchema tSchemaCopy;
		tSchemaCopy = *pSorter->GetSchema();
		tRes.m_tSchema.Swap ( tSchemaCopy );
	}

	// convert all matches to sorter schema - at least to manage all static to dynamic
	if ( !bAllEqual )
	{
		// post-limit stuff first
		if ( bHaveLocals )
		{
			CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_EVAL_POST );
			ProcessLocalPostlimit ( tRes, tQueryCopy, bMaster );
		}

		RemapResult ( tRes );
	}

	// do the sort work!
	tRes.m_iTotalMatches -= KillDupesAndFlatten ( pSorter.get(), tRes );
	return true;
}


bool ApplyOuterOrder ( AggrResult_t & tRes, const CSphQuery & tQuery )
{
	assert ( !tRes.m_dResults.IsEmpty() );
	// reorder (aka outer order)
	ESphSortFunc eFunc;
	GenericMatchSort_fn tReorder;
	CSphVector<ExtraSortExpr_t> dExtraExprs;

	ESortClauseParseResult eRes = sphParseSortClause ( tQuery, tQuery.m_sOuterOrderBy.cstr(), tRes.m_tSchema, eFunc, tReorder, dExtraExprs, true, tRes.m_sError );
	if ( eRes==SORT_CLAUSE_RANDOM )
		tRes.m_sError = "order by rand() not supported in outer select";

	if ( eRes!=SORT_CLAUSE_OK )
		return false;

	assert ( eFunc==FUNC_GENERIC1 ||eFunc==FUNC_GENERIC2 || eFunc==FUNC_GENERIC3 || eFunc==FUNC_GENERIC4 || eFunc==FUNC_GENERIC5 );
	auto& dMatches = tRes.m_dResults.First().m_dMatches;
	sphSort ( dMatches.Begin(), dMatches.GetLength(), tReorder, MatchSortAccessor_t() );
	return true;
}


void ComputePostlimit ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bMaster )
{
	assert ( tRes.m_bSingle );
	assert ( tRes.m_bOneSchema );
	assert ( !tRes.m_dResults.IsEmpty () );

	CSphVector<const CSphColumnInfo *> dPostlimit;
	ExtractPostlimit ( tRes.m_tSchema, bMaster, dPostlimit );

	// post compute matches only between offset..limit
	// however at agent we can't estimate limit.offset at master merged result set
	// but master don't provide offset to agents only offset+limit as limit
	// so computing all matches from 0 up to inner.limit/outer.limit
	assert ( tRes.GetLength ()==tRes.m_dResults.First().m_dMatches.GetLength() );
	int iOff = Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
	int iLimit = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );

	if ( tRes.m_bTagsAssigned )
		ProcessMultiPostlimit ( tRes, dPostlimit, tQuery.m_sQuery.cstr (), iOff, iLimit );
	else
		ProcessSinglePostlimit ( tRes.m_dResults.First(), dPostlimit, tQuery.m_sQuery.cstr(), iOff, iLimit );
}

int64_t CalcPredictedTimeMsec ( const CSphQueryResultMeta & tMeta )
{
	assert ( tMeta.m_bHasPrediction );

	int64_t iNanoResult = int64_t(g_iPredictorCostSkip)* tMeta.m_tStats.m_iSkips
		+ g_iPredictorCostDoc * tMeta.m_tStats.m_iFetchedDocs
		+ g_iPredictorCostHit * tMeta.m_tStats.m_iFetchedHits
		+ g_iPredictorCostMatch * tMeta.m_iTotalMatches;

	return iNanoResult/1000000;
}


int GetMaxMatches ( int iQueryMaxMatches, const CSphIndex * pIndex )
{
	if ( iQueryMaxMatches<=DEFAULT_MAX_MATCHES )
		return iQueryMaxMatches;

	int64_t iDocs = Min ( (int)INT_MAX, pIndex->GetStats().m_iTotalDocuments ); // clamp to int max
	return Min ( iQueryMaxMatches, Max ( iDocs, DEFAULT_MAX_MATCHES ) ); // do not want 0 sorter and sorter longer than query.max_matches
}


} // namespace static


/// merges multiple result sets, remaps columns, does reorder for outer selects
bool MinimizeAggrResult ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bHaveLocals, const sph::StringSet & hExtraColumns, QueryProfile_c * pProfiler, const CSphFilterSettings * pAggrFilter, bool bForceRefItems, bool bMaster )
{
	bool bReturnZeroCount = !tRes.m_dZeroCount.IsEmpty();
	bool bQueryFromAPI = tQuery.m_eQueryType==QUERY_API;

	// 0 matches via SphinxAPI? no fiddling with schemes is necessary
	// (and via SphinxQL, we still need to return the right schema)
	// 0 result set schemes via SphinxQL? just bail
	if ( tRes.IsEmpty() && ( bQueryFromAPI || !bReturnZeroCount ) )
	{
		Debug ( tRes.m_bSingle = true; )
		if ( !tRes.m_dResults.IsEmpty () )
		{
			tRes.m_tSchema = tRes.m_dResults.First ().m_tSchema;
			Debug( tRes.m_bOneSchema = true; )
		}
		return true;
	}

	Debug ( tRes.m_bSingle = tRes.m_dResults.GetLength ()==1; )

	// build a minimal schema over all the (potentially different) schemes
	// that we have in our aggregated result set
	assert ( tRes.m_dResults.GetLength() || bReturnZeroCount );

	bool bAllEqual = MinimizeSchemas(tRes);

	Debug ( tRes.m_bOneSchema = tRes.m_bSingle; )

	const CSphVector<CSphQueryItem> & dQueryItems = ( tQuery.m_bFacet || tQuery.m_bFacetHead || bForceRefItems ) ? tQuery.m_dRefItems : tQuery.m_dItems;

	// build a list of select items that the query asked for
	bool bHaveExprs = false;
	CSphVector<CSphQueryItem> tExtItems;
	const CSphVector<CSphQueryItem> & dItems = ExpandAsterisk ( tRes.m_tSchema, dQueryItems, tExtItems, tQuery.m_bFacetHead, bHaveExprs );

	// api + index without attributes + select * case
	// can not skip aggregate filtering
	if ( bQueryFromAPI && dItems.IsEmpty() && !pAggrFilter && !bHaveExprs )
	{
		tRes.ClampAllMatches();
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
	} else
	{
		tRes.m_dResults.First().m_iTag = 0;
		Debug ( tRes.m_bTagsCompacted = true );
		Debug ( tRes.m_bIdxByTag = true; )
	}

	// apply outer order clause to single result set
	// (multiple combined sets just got reordered above)
	// apply inner limit first
	if ( tRes.m_iSuccesses==1 && tQuery.m_bHasOuter )
	{
		tRes.ClampMatches ( tQuery.m_iLimit );
		if ( !tQuery.m_sOuterOrderBy.IsEmpty() )
		{
			if ( !ApplyOuterOrder ( tRes, tQuery ) )
				return false;
		}
		Debug ( tRes.m_bSingle = true; )
		Debug ( tRes.m_bTagsCompacted = true );
		Debug ( tRes.m_bIdxByTag = true; )
	}

	if ( bAllEqual && bHaveLocals )
	{
		CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_EVAL_POST );
		ComputePostlimit ( tRes, tQuery, bMaster );
	}

	if ( bMaster )
	{
		CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_EVAL_GETFIELD );
		RemotesGetField ( tRes, tQuery );
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

/// manage collection of indexes (to keep them alive)
/// Get(name) - returns an index from collection.
/// AddUniqIndex(name) - add local idx to collection, addref is implied by design
/// AddIndex(name,pidx) - add custom idx, to make it available with Get()
class KeepCollection_c : public ISphNoncopyable
{
	SmallStringHash_T<cServedIndexRefPtr_c> m_hIndexes;

public:
	// add from globally served
	bool AddUniqIndex ( const CSphString& sName );

	// add custom
	void AddIndex ( const CSphString& sName, cServedIndexRefPtr_c pIdx );

	// use idx
	cServedIndexRefPtr_c Get ( const CSphString &sName ) const;
};


class SearchHandler_c
{
public:
									SearchHandler_c ( int iQueries, std::unique_ptr<QueryParser_i> pParser, QueryType_e eQueryType, bool bMaster );
									~SearchHandler_c();

	void							RunQueries ();					///< run all queries, get all results
	void							RunCollect ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors, CSphVector<BYTE> * pCollectedDocs );
	void							SetQuery ( int iQuery, const CSphQuery & tQuery, ISphTableFunc * pTableFunc );
	void							SetQueryParser ( std::unique_ptr<QueryParser_i> pParser, QueryType_e eQueryType );
	void							SetProfile ( QueryProfile_c * pProfile );
	AggrResult_t *					GetResult ( int iResult ) { return m_dAggrResults.Begin() + iResult; }
	void							SetFederatedUser () { m_bFederatedUser = true; }

public:
	CSphVector<CSphQuery>			m_dQueries;						///< queries which i need to search
	CSphVector<AggrResult_t>		m_dAggrResults;					///< results which i obtained
	CSphVector<StatsPerQuery_t>		m_dQueryIndexStats;				///< statistics for current query
	CSphVector<SearchFailuresLog_c>	m_dFailuresSet;					///< failure logs for each query
	CSphVector<CSphVector<int64_t>>	m_dAgentTimes;					///< per-agent time stats
	KeepCollection_c				m_dAcquired;					/// locked indexes
	CSphFixedVector<ISphTableFunc *>	m_dTables;
	SqlStmt_t *						m_pStmt = nullptr;				///< original (one) statement to take extra options

protected:
	void							RunSubset ( int iStart, int iEnd );	///< run queries against index(es) from first query in the subset
	void							RunLocalSearches();
	bool							AllowsMulti() const;
	void							SetupLocalDF();

	bool							m_bMultiQueue = false;	///< whether current subset is subject to multi-queue optimization
	bool							m_bFacetQueue = false;	///< whether current subset is subject to facet-queue optimization
	CSphVector<LocalIndex_t>		m_dLocal;				///< local indexes for the current subset
	StrVec_t 						m_dExtraSchema;		 	///< the extra attrs for agents. One vec per index*threads
	CSphVector<BYTE> *				m_pCollectedDocs = nullptr;	///< this query is for deleting

	QueryProfile_c *				m_pProfile = nullptr;
	QueryType_e						m_eQueryType {QUERY_API}; ///< queries from sphinxql require special handling
	std::unique_ptr<QueryParser_i>	m_pQueryParser;	///< parser used for queries in this handler. e.g. plain or json-style

	bool							m_bNeedDocIDs = false;	///< do we need docids returned from local searches (remotes return them anyway)?

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
	CSphVector<CSphQueryResult>			m_dResults;
	VecTraits_T<CSphQuery>				m_dNQueries;		///< working subset of queries
	VecTraits_T<AggrResult_t>			m_dNAggrResults;	///< working subset of results
	VecTraits_T<CSphQueryResult>		m_dNResults;		///< working subset of result pointers
	VecTraits_T<SearchFailuresLog_c>	m_dNFailuresSet;	///< working subset of failures

private:
	bool							ParseSysVar();
	bool							ParseIdxSubkeys();
	bool							CheckMultiQuery() const;
	bool							AcquireInvokedIndexes();
	void							UniqLocals ( VecTraits_T<LocalIndex_t>& dLocals );
	void							RunActionQuery ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors ); ///< run delete/update
	bool							BuildIndexList ( int & iDivideLimits, VecRefPtrsAgentConn_t & dRemotes, CSphVector<DistrServedByAgent_t> & dDistrServedByAgent ); // fixme!
	void							CalcTimeStats ( int64_t tmCpu, int64_t tmSubset, const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent );
	void							CalcPerIndexStats ( const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent ) const;
	void							CalcGlobalStats ( int64_t tmCpu, int64_t tmSubset, int64_t tmLocal, const CSphIOStats & tIO, const VecRefPtrsAgentConn_t & dRemotes ) const;
	int								CreateSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const;
	int								CreateSingleSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const;
	int								CreateMultiQueryOrFacetSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const;

	SphQueueSettings_t				MakeQueueSettings ( const CSphIndex * pIndex, int iMaxMatches, ISphExprHook * pHook ) const;
	cServedIndexRefPtr_c			CheckIndexSelectable ( const CSphString& sLocal, const char * szParent, VecTraits_T<SearchFailuresLog_c> * pNFailuresSet=nullptr ) const;
	bool							CreateValidSorters ( VecTraits_T<ISphMatchSorter *> & dSrt, SphQueueRes_t * pQueueRes, VecTraits_T<SearchFailuresLog_c> & dFlr, StrVec_t * pExtra, const CSphIndex* pIndex, const CSphString & sLocal, const char * szParent, ISphExprHook * pHook );
	void							CalcSplits ( int iConcurrency, CSphFixedVector<int> & dSplits );
};

PubSearchHandler_c::PubSearchHandler_c ( int iQueries, std::unique_ptr<QueryParser_i> pQueryParser, QueryType_e eQueryType, bool bMaster )
{
	m_pImpl = new SearchHandler_c ( iQueries, std::move ( pQueryParser ), eQueryType, bMaster );
}

PubSearchHandler_c::~PubSearchHandler_c ()
{
	delete m_pImpl;
}

void PubSearchHandler_c::RunQueries ()
{
	assert ( m_pImpl );
	m_pImpl->RunQueries();
}

void PubSearchHandler_c::SetQuery ( int iQuery, const CSphQuery & tQuery, ISphTableFunc * pTableFunc )
{
	assert ( m_pImpl );
	m_pImpl->SetQuery ( iQuery, tQuery, pTableFunc );
}

void PubSearchHandler_c::SetProfile ( QueryProfile_c * pProfile )
{
	assert ( m_pImpl );
	m_pImpl->SetProfile ( pProfile );
}

void PubSearchHandler_c::SetStmt ( SqlStmt_t & tStmt )
{
	assert ( m_pImpl );
	m_pImpl->m_pStmt = &tStmt;
}

AggrResult_t * PubSearchHandler_c::GetResult ( int iResult )
{
	assert ( m_pImpl );
	return m_pImpl->GetResult (iResult);
}

void PubSearchHandler_c::PushIndex ( const CSphString& sIndex, const cServedIndexRefPtr_c& pDesc )
{
	assert ( m_pImpl );
	m_pImpl->m_dAcquired.AddIndex ( sIndex, pDesc );
}

void PubSearchHandler_c::RunCollect ( const CSphQuery& tQuery, const CSphString& sIndex, CSphString* pErrors, CSphVector<BYTE>* pCollectedDocs )
{
	assert ( m_pImpl );
	m_pImpl->RunCollect ( tQuery, sIndex, pErrors, pCollectedDocs );
}


SearchHandler_c::SearchHandler_c ( int iQueries, std::unique_ptr<QueryParser_i> pQueryParser, QueryType_e eQueryType, bool bMaster )
	: m_dTables ( iQueries )
{
	m_dQueries.Resize ( iQueries );
	m_dAggrResults.Resize ( iQueries );
	m_dFailuresSet.Resize ( iQueries );
	m_dAgentTimes.Resize ( iQueries );
	m_bMaster = bMaster;
	m_bFederatedUser = false;
	ARRAY_FOREACH ( i, m_dTables )
		m_dTables[i] = nullptr;

	SetQueryParser ( std::move ( pQueryParser ), eQueryType );
	m_dResults.Resize ( iQueries );
	for ( int i=0; i<iQueries; ++i )
		m_dResults[i].m_pMeta = &m_dAggrResults[i];

	// initial slices (when nothing explicitly asked)
	m_dNQueries = m_dQueries;
	m_dNAggrResults = m_dAggrResults;
	m_dNResults = m_dResults;
	m_dNFailuresSet = m_dFailuresSet;
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

void SearchHandler_c::SetQueryParser ( std::unique_ptr<QueryParser_i> pParser, QueryType_e eQueryType )
{
	m_pQueryParser = std::move ( pParser );
	m_eQueryType = eQueryType;
	for ( auto & dQuery : m_dQueries )
	{
		dQuery.m_pQueryParser = m_pQueryParser.get();
		dQuery.m_eQueryType = eQueryType;
	}
}

bool KeepCollection_c::AddUniqIndex ( const CSphString & sName )
{
	if ( m_hIndexes.Exists ( sName ) )
		return true;

	auto pIdx = GetServed ( sName );
	if ( !pIdx )
		return false;

	m_hIndexes.Add ( std::move ( pIdx ), sName );
	return true;
}

void KeepCollection_c::AddIndex ( const CSphString & sName, cServedIndexRefPtr_c pIdx )
{
	if ( m_hIndexes.Exists ( sName ) )
		return;

	m_hIndexes.Add ( std::move ( pIdx ), sName );
}

cServedIndexRefPtr_c KeepCollection_c::Get ( const CSphString & sName ) const
{
	auto * ppIndex = m_hIndexes ( sName );
	assert ( ppIndex && "KeepCollection_c::Get called with absent key");
	return *ppIndex;
}

void SearchHandler_c::RunCollect ( const CSphQuery &tQuery, const CSphString &sIndex, CSphString * pErrors, CSphVector<BYTE> * pCollectedDocs )
{
	m_bQueryLog = false;
	m_pCollectedDocs = pCollectedDocs;
	RunActionQuery ( tQuery, sIndex, pErrors );
}

void SearchHandler_c::RunActionQuery ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors )
{
	SetQuery ( 0, tQuery, nullptr );
	m_dQueries[0].m_sIndexes = sIndex;
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

	auto & tRes = m_dAggrResults[0];

	tRes.m_iOffset = tQuery.m_iOffset;
	tRes.m_iCount = Max ( Min ( tQuery.m_iLimit, tRes.GetLength()-tQuery.m_iOffset ), 0 );
	// actualy tRes.m_iCount=0 since delete/update produces no matches

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

	auto & g_tStats = gStats ();
	g_tStats.m_iQueries.fetch_add ( 1, std::memory_order_relaxed );
	g_tStats.m_iQueryTime.fetch_add ( tmLocal, std::memory_order_relaxed );
	g_tStats.m_iQueryCpuTime.fetch_add ( tmLocal, std::memory_order_relaxed );
	g_tStats.m_iDiskReads.fetch_add ( tIO.m_iReadOps, std::memory_order_relaxed );
	g_tStats.m_iDiskReadTime.fetch_add ( tIO.m_iReadTime, std::memory_order_relaxed );
	g_tStats.m_iDiskReadBytes.fetch_add ( tIO.m_iReadBytes, std::memory_order_relaxed );

	if ( m_bQueryLog )
		LogQuery ( m_dQueries[0], m_dAggrResults[0], m_dAgentTimes[0] );
}

void SearchHandler_c::SetQuery ( int iQuery, const CSphQuery & tQuery, ISphTableFunc * pTableFunc )
{
	m_dQueries[iQuery] = tQuery;
	m_dQueries[iQuery].m_pQueryParser = m_pQueryParser.get();
	m_dQueries[iQuery].m_eQueryType = m_eQueryType;
	m_dTables[iQuery] = pTableFunc;
}


void SearchHandler_c::SetProfile ( QueryProfile_c * pProfile )
{
	assert ( pProfile );
	m_pProfile = pProfile;
}


void SearchHandler_c::RunQueries()
{
	// batch queries to same index(es)
	// or work each query separately if indexes are different

	int iStart = 0;
	ARRAY_FOREACH ( i, m_dQueries )
	{
		if ( m_dQueries[i].m_sIndexes!=m_dQueries[iStart].m_sIndexes )
		{
			RunSubset ( iStart, i );
			iStart = i;
		}
	}
	RunSubset ( iStart, m_dQueries.GetLength() );
	if ( m_bQueryLog )
	{
		ARRAY_FOREACH ( i, m_dQueries )
			LogQuery ( m_dQueries[i], m_dAggrResults[i], m_dAgentTimes[i] );
	}
	OnRunFinished();
}


// final fixup
void SearchHandler_c::OnRunFinished()
{
	for ( auto & tResult : m_dAggrResults )
		tResult.m_iMatches = tResult.GetLength();
}

SphQueueSettings_t SearchHandler_c::MakeQueueSettings ( const CSphIndex * pIndex, int iMaxMatches, ISphExprHook * pHook ) const
{
	SphQueueSettings_t tQS ( pIndex->GetMatchSchema (), m_pProfile );
	tQS.m_bComputeItems = true;
	tQS.m_pCollection = m_pCollectedDocs;
	tQS.m_pHook = pHook;
	tQS.m_iMaxMatches = GetMaxMatches ( iMaxMatches, pIndex );
	tQS.m_bNeedDocids = m_bNeedDocIDs;	// need docids to merge results from indexes
	tQS.m_fnGetCountDistinct = [pIndex]( const CSphString & sAttr ){ return pIndex->GetCountDistinct(sAttr); };
	return tQS;
}


int SearchHandler_c::CreateMultiQueryOrFacetSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const
{
	int iValidSorters = 0;

	auto tQueueSettings = MakeQueueSettings ( pIndex, m_dNQueries.First ().m_iMaxMatches, pHook );
	sphCreateMultiQueue ( tQueueSettings, m_dNQueries, dSorters, dErrors, tQueueRes, pExtra );

	m_dNQueries.First().m_bZSlist = tQueueRes.m_bZonespanlist;
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


int SearchHandler_c::CreateSingleSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const
{
	int iValidSorters = 0;
	tQueueRes.m_bAlowMulti = false;
	const int iQueries = m_dNQueries.GetLength();
	for ( int iQuery = 0; iQuery<iQueries; ++iQuery )
	{
		CSphQuery & tQuery = m_dNQueries[iQuery];

		// create queue
		auto tQueueSettings = MakeQueueSettings ( pIndex, tQuery.m_iMaxMatches, pHook );
		ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings, tQuery, dErrors[iQuery], tQueueRes, pExtra );
		if ( !pSorter )
			continue;

		tQuery.m_bZSlist = tQueueRes.m_bZonespanlist;
		dSorters[iQuery] = pSorter;
		++iValidSorters;
	}
	return iValidSorters;
}


int SearchHandler_c::CreateSorters ( const CSphIndex * pIndex, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t* pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const
{
	if ( m_bMultiQueue || m_bFacetQueue )
		return CreateMultiQueryOrFacetSorters ( pIndex, dSorters, dErrors, pExtra, tQueueRes, pHook );
	return CreateSingleSorters ( pIndex, dSorters, dErrors, pExtra, tQueueRes, pHook );
}

struct LocalSearchRef_t
{
	ExprHook_c&	m_tHook;
	StrVec_t* m_pExtra;
	VecTraits_T<SearchFailuresLog_c>& m_dFailuresSet;
	VecTraits_T<AggrResult_t>& m_dAggrResults;
	VecTraits_T<CSphQueryResult>& m_dResults;

	LocalSearchRef_t ( ExprHook_c & tHook, StrVec_t* pExtra, VecTraits_T<SearchFailuresLog_c> & dFailures, VecTraits_T<AggrResult_t> & dAggrResults, VecTraits_T<CSphQueryResult> & dResults )
		: m_tHook ( tHook )
		, m_pExtra ( pExtra )
		, m_dFailuresSet ( dFailures )
		, m_dAggrResults ( dAggrResults )
		, m_dResults ( dResults )
	{}

	void MergeChild ( LocalSearchRef_t dChild ) const
	{
		if ( m_pExtra )
		{
			assert ( dChild.m_pExtra );
			m_pExtra->Append ( *dChild.m_pExtra );
		}

		auto & dChildAggrResults = dChild.m_dAggrResults;
		for ( int i = 0, iQueries = m_dAggrResults.GetLength (); i<iQueries; ++i )
		{
			auto & tResult = m_dAggrResults[i];
			auto & tChild = dChildAggrResults[i];

			tResult.m_dResults.Append ( tChild.m_dResults );

			// word statistics
			tResult.MergeWordStats ( tChild );

			// other data (warnings, errors, etc.)
			// errors
			if ( !tChild.m_sError.IsEmpty ())
				tResult.m_sError = tChild.m_sError;

			// warnings
			if ( !tChild.m_sWarning.IsEmpty ())
				tResult.m_sWarning = tChild.m_sWarning;

			// prediction counters
			tResult.m_bHasPrediction |= tChild.m_bHasPrediction;
			if ( tChild.m_bHasPrediction )
			{
				tResult.m_tStats.Add ( tChild.m_tStats );
				tResult.m_iPredictedTime = CalcPredictedTimeMsec ( tResult );
			}

			// profiling
			if ( tChild.m_pProfile )
				tResult.m_pProfile->AddMetric ( *tChild.m_pProfile );

			tResult.m_iCpuTime += tChild.m_iCpuTime;
			tResult.m_iTotalMatches += tChild.m_iTotalMatches;
			tResult.m_bTotalMatchesApprox |= tChild.m_bTotalMatchesApprox;
			tResult.m_iSuccesses += tChild.m_iSuccesses;
			tResult.m_tIOStats.Add ( tChild.m_tIOStats );

			tResult.m_tIteratorStats.Merge ( tChild.m_tIteratorStats );

			// failures
			m_dFailuresSet[i].Append ( dChild.m_dFailuresSet[i] );
		}
	}

	inline static bool IsClonable()
	{
		return true;
	}
};

struct LocalSearchClone_t
{
	ExprHook_c m_tHook;
	StrVec_t m_dExtra;
	StrVec_t* m_pExtra;
	CSphVector<SearchFailuresLog_c> m_dFailuresSet;
	CSphVector<AggrResult_t>	m_dAggrResults;
	CSphVector<CSphQueryResult> m_dResults;

	explicit LocalSearchClone_t ( const LocalSearchRef_t & dParent)
	{
		int iQueries = dParent.m_dFailuresSet.GetLength ();
		m_dFailuresSet.Resize ( iQueries );
		m_dAggrResults.Resize ( iQueries );
		m_dResults.Resize ( iQueries );
		for ( int i=0; i<iQueries; ++i )
			m_dResults[i].m_pMeta = &m_dAggrResults[i];
		m_pExtra = dParent.m_pExtra ? &m_dExtra : nullptr;

		// set profiler complementary to one in RunSubset (search by `if ( iQueries==1 && m_pProfile )` clause)
		if ( iQueries==1 && dParent.m_dAggrResults.First ().m_pProfile )
		{
			auto pProfile = new QueryProfile_c;
			m_dAggrResults.First().m_pProfile = pProfile;
			m_tHook.SetProfiler ( pProfile );
		}
	}
	explicit operator LocalSearchRef_t ()
	{
		return { m_tHook, m_pExtra, m_dFailuresSet, m_dAggrResults, m_dResults };
	}

	~LocalSearchClone_t()
	{
		if ( !m_dAggrResults.IsEmpty () )
			SafeDelete ( m_dAggrResults.First().m_pProfile );
	}
};


cServedIndexRefPtr_c SearchHandler_c::CheckIndexSelectable ( const CSphString & sLocal, const char * szParent, VecTraits_T<SearchFailuresLog_c> * pNFailuresSet ) const
{
	const auto& pServed = m_dAcquired.Get ( sLocal );
	assert ( pServed );

	if ( !ServedDesc_t::IsSelectable ( pServed ) )
	{
		if ( pNFailuresSet )
			for ( auto & dFailureSet : *pNFailuresSet )
				dFailureSet.SubmitEx ( sLocal, nullptr, "%s", "index is not suitable for select" );

		return cServedIndexRefPtr_c{};
	}

	return pServed;
}


bool SearchHandler_c::CreateValidSorters ( VecTraits_T<ISphMatchSorter *> & dSrt, SphQueueRes_t * pQueueRes, VecTraits_T<SearchFailuresLog_c> & dFlr, StrVec_t * pExtra, const CSphIndex * pIndex, const CSphString & sLocal, const char * szParent, ISphExprHook * pHook )
{
	auto iQueries = dSrt.GetLength();
	#if PARANOID
	for ( const auto* pSorter : dSrt)
		assert ( !pSorter );
	#endif

	CSphFixedVector<CSphString> dErrors ( iQueries );
	int iValidSorters = CreateSorters ( pIndex, dSrt, dErrors, pExtra, *pQueueRes, pHook );
	if ( iValidSorters<dSrt.GetLength() )
	{
		ARRAY_FOREACH ( i, dErrors )
		{
			if ( !dErrors[i].IsEmpty () )
				dFlr[i].Submit ( sLocal, szParent, dErrors[i].cstr () );
		}
	}

	m_bMultiQueue = pQueueRes->m_bAlowMulti;
	return !!iValidSorters;
}


void SearchHandler_c::CalcSplits ( int iConcurrency, CSphFixedVector<int> & dSplits )
{
	// dSplits should already be initialized with 1s
	if ( !g_bSplit )
		return;

	if ( !iConcurrency )
		iConcurrency = g_iThreads;

	struct SplitData_t
	{
		bool	m_bEnabled = false;
		int64_t	m_iMetric = 0;
	};

	CSphFixedVector<SplitData_t> dSplitData { m_dLocal.GetLength() };

	// FIXME! what about PQ?
	int64_t iTotalMetric = 0;
	int iSingleSplits = 0;
	int iEnabled = 0;
	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		const LocalIndex_t & tLocal = m_dLocal[iLocal];
		auto pIndex = CheckIndexSelectable ( tLocal.m_sName, tLocal.m_sParentIndex.cstr(), nullptr );
		if ( !pIndex )
			continue;

		SplitData_t & tSplitData = dSplitData[iLocal];
		int64_t iMetric = RIdx_c ( pIndex )->GetPseudoShardingMetric ( m_dNQueries );
		if ( iMetric==-1 )
		{
			iSingleSplits++;
			continue;
		}

		tSplitData.m_bEnabled = true;
		tSplitData.m_iMetric = iMetric;
		iTotalMetric += tSplitData.m_iMetric;
		iEnabled++;
	}

	if ( iConcurrency>iSingleSplits+iEnabled )
	{
		int iLeft = iConcurrency-iSingleSplits;
		ARRAY_FOREACH ( i, dSplitData )
		{
			const SplitData_t & tSplitData = dSplitData[i];
			if ( !tSplitData.m_bEnabled )
				continue;

			dSplits[i] = Max ( (int)round ( double(tSplitData.m_iMetric) / iTotalMetric * iLeft ), 1 );
		}
	}
}


class AssignTag_c : public MatchProcessor_i
{
public:
	AssignTag_c ( int iTag )
		: m_iTag ( iTag )
	{}

	void Process ( CSphMatch * pMatch ) final			{ ProcessMatch(pMatch); }
	bool ProcessInRowIdOrder() const final				{ return false;	}
	void Process ( VecTraits_T<CSphMatch *> & dMatches ) final { dMatches.for_each ( [this]( CSphMatch * pMatch ){ ProcessMatch(pMatch); } ); }

private:
	int	m_iTag = 0;

	inline void ProcessMatch ( CSphMatch * pMatch )		{ pMatch->m_iTag = m_iTag; }
};


class GlobalSorters_c
{
public:
	GlobalSorters_c ( const VecTraits_T<CSphQuery> & dQueries, const CSphVector<cServedIndexRefPtr_c> & dIndexes )
		: m_dQueries ( dQueries )
		, m_dSorters { dQueries.GetLength() }
	{
		auto iValidIndexes = (int)dIndexes.count_of ( [&] ( const auto& pIndex ) { return pIndex; } );

		m_bNeedGlobalSorters = iValidIndexes>1 && !dQueries.First().m_sGroupDistinct.IsEmpty();
		if ( m_bNeedGlobalSorters )
		{
			// check if schemas are same
			const CSphSchema * pFirstSchema = nullptr;
			for ( auto i : dIndexes )
			{
				if ( !i )
					continue;

				if ( !pFirstSchema )
				{
					pFirstSchema = &RIdx_c ( i )->GetMatchSchema();
					continue;
				}

				CSphString sCmpError;
				if ( !pFirstSchema->CompareTo ( RIdx_c ( i )->GetMatchSchema(), sCmpError ) )
				{
					m_bNeedGlobalSorters = false;
					break;
				}
			}
		}

		for ( auto & i : m_dSorters )
			i.Resize ( dIndexes.GetLength() );
	}

	~GlobalSorters_c()
	{
		for ( auto & i : m_dSorters )
			for ( auto & j : i )
				SafeDelete ( j.m_pSorter );
	}

	bool StoreSorter ( int iQuery, int iIndex, ISphMatchSorter * & pSorter, const DocstoreReader_i * pDocstore, int iTag )
	{
		// FACET head is the plain query wo group sorter and can not move all result set into single sorter
		// could be replaced with !pSorter->IspSorter->IsGroupby()
		if ( !NeedGlobalSorters() || m_dQueries[iQuery].m_bFacetHead )
			return false;

		// take ownership of the sorter
		m_dSorters[iQuery][iIndex] = { pSorter, pDocstore, iTag };
		pSorter = nullptr;

		return true;
	}

	bool NeedGlobalSorters() const
	{
		return m_bNeedGlobalSorters;
	}

	void MergeResults ( VecTraits_T<AggrResult_t> & dResults )
	{
		if ( !NeedGlobalSorters() )
			return;

		ARRAY_FOREACH ( iQuery, m_dSorters )
		{
			CSphVector<ISphMatchSorter *> dValidSorters;
			for ( auto i : m_dSorters[iQuery] )
			{
				if ( !i.m_pSorter )
					continue;

				dValidSorters.Add ( i.m_pSorter );

				// assign order tag here so we can link to docstore later
				AssignTag_c tAssign ( i.m_iTag );
				i.m_pSorter->Finalize ( tAssign, false, false );
			}

			int iNumIndexes = dValidSorters.GetLength();
			if ( !iNumIndexes )
				continue;

			ISphMatchSorter * pLastSorter = dValidSorters[iNumIndexes-1];

			// merge all results to the last sorter. this is done to try to keep some compatibility with no-global-sorters code branch
			for ( int iIndex = iNumIndexes-2; iIndex>=0; iIndex-- )
				dValidSorters[iIndex]->MoveTo ( pLastSorter, true );

			dResults[iQuery].m_iTotalMatches = pLastSorter->GetTotalCount();
			dResults[iQuery].AddResultset ( pLastSorter, m_dSorters[iQuery][0].m_pDocstore, m_dSorters[iQuery][0].m_iTag, m_dQueries[iQuery].m_iCutoff );

			// we already assigned index/docstore tags to all matches; no need to do it again
			if ( dResults[iQuery].m_dResults.GetLength() )
				dResults[iQuery].m_dResults[0].m_bTagsAssigned = true;

			// add fake empty result sets (for tag->docstore lookup)
			for ( int i = 1; i < m_dSorters[iQuery].GetLength(); i++ )
				dResults[iQuery].AddEmptyResultset ( m_dSorters[iQuery][i].m_pDocstore, m_dSorters[iQuery][i].m_iTag );
		}
	}

private:
	struct SorterData_t
	{
		ISphMatchSorter *			m_pSorter = nullptr;
		const DocstoreReader_i *	m_pDocstore = nullptr;
		int							m_iTag = 0;
	};

	const VecTraits_T<CSphQuery> &			m_dQueries;
	CSphVector<CSphVector<SorterData_t>>	m_dSorters;
	bool									m_bNeedGlobalSorters = false;
};

void SearchHandler_c::RunLocalSearches ()
{
	int64_t tmLocal = sphMicroTimer ();

	// setup local searches
	const int iQueries = m_dNQueries.GetLength ();
	const int iNumLocals = m_dLocal.GetLength();

//	sphWarning ( "%s:%d", __FUNCTION__, __LINE__ );
//	sphWarning ("Locals: %d, queries %d", iNumLocals, iQueries );

	m_dQueryIndexStats.Resize ( iNumLocals );
	for ( auto & dQueryIndexStats : m_dQueryIndexStats )
		dQueryIndexStats.m_dStats.Resize ( iQueries );

	StrVec_t * pMainExtra = nullptr;
	if ( m_dNQueries.First ().m_bAgent )
	{
		m_dExtraSchema.Reset (); // cleanup from any possible previous usages
		pMainExtra = &m_dExtraSchema;
	}

	CSphVector<cServedIndexRefPtr_c> dLocalIndexes;
	for ( const auto& i : m_dLocal )
		dLocalIndexes.Add ( CheckIndexSelectable ( i.m_sName, nullptr ) );

	GlobalSorters_c tGlobalSorters ( m_dNQueries, dLocalIndexes );

	CSphFixedVector<int> dSplits { iNumLocals };
	dSplits.Fill(1);

	CSphFixedVector<int> dOrder { iNumLocals };
	for ( int i = 0; i<iNumLocals; ++i )
		dOrder[i] = i;

	// the context
	ClonableCtx_T<LocalSearchRef_t, LocalSearchClone_t> dCtx { m_tHook, pMainExtra, m_dNFailuresSet, m_dNAggrResults, m_dNResults };

	auto iConcurrency = m_dNQueries.First().m_iCouncurrency;
	if ( !iConcurrency )
		iConcurrency = GetEffectiveDistThreads ();
	dCtx.LimitConcurrency ( iConcurrency );

	bool bSingle = iConcurrency==1;

//	sphWarning ( "iConcurrency: %d", iConcurrency );

	if ( !bSingle )
	{
//		sphWarning ( "Reordering..." );
		// if run parallel - start in mass order, if single - in natural order
		// set order by decreasing index mass (heaviest comes first). That is why 'less' implemented by '>'
		dOrder.Sort ( Lesser ( [this] ( int a, int b ) {
			return m_dLocal[a].m_iMass>m_dLocal[b].m_iMass;
		} ) );

		CalcSplits ( iConcurrency, dSplits );
	}

//	for ( int iOrder : dOrder )
//		sphWarning ( "Sorted: %d, Order %d, mass %d", !!bSingle, iOrder, (int) m_dLocal[iOrder].m_iMass );

	std::atomic<int32_t> iTotalSuccesses { 0 };
	const auto iJobs = iNumLocals;
	std::atomic<int32_t> iCurJob { 0 };
	Coro::ExecuteN ( dCtx.Concurrency ( iJobs ), [&]
	{
		auto iJob = iCurJob.load ( std::memory_order_relaxed );
		if ( iJob>=iJobs )
			return; // already nothing to do, early finish.

		// these two moved from inside of the loop to avoid construction on every turn
		CSphVector<ISphMatchSorter *> dSorters ( iQueries );
		dSorters.ZeroVec ();

		auto tCtx = dCtx.CloneNewContext();
		Threads::Coro::Throttler_c tThrottler ( session::GetThrottlingPeriodMS () );
		while ( iJob<iJobs )
		{
			iJob = iCurJob.fetch_add ( 1, std::memory_order_acq_rel );
			if ( iJob>=iJobs )
				return; // all is done

			auto iLocal = dOrder[iJob];
//			sphWarning ( "iJob: %d, iLocal: %d, mass %d", iJob, iLocal, (int) m_dLocal[iLocal].m_iMass );

			sphLogDebugv ("Tick coro search");
			int64_t iCpuTime = -sphTaskCpuTimer ();

			// FIXME!!! handle different proto
			myinfo::SetTaskInfo( R"(api-search query="%s" comment="%s" index="%s")",
									 m_dNQueries.First().m_sQuery.scstr (),
									 m_dNQueries.First().m_sComment.scstr (),
									 m_dLocal[iLocal].m_sName.scstr ());

			const LocalIndex_t & dLocal = m_dLocal[iLocal];
			const CSphString& sLocal = dLocal.m_sName;
			const char * szParent = dLocal.m_sParentIndex.cstr ();
			int iOrderTag = dLocal.m_iOrderTag;
			int iIndexWeight = dLocal.m_iWeight;
			auto& dNFailuresSet = tCtx.m_dFailuresSet;
			auto& dNAggrResults = tCtx.m_dAggrResults;
			auto& dNResults = tCtx.m_dResults;
			auto* pExtra = tCtx.m_pExtra;

			// publish crash query index
			GlobalCrashQueryGetRef().m_dIndex = FromStr ( sLocal );

			// prepare and check the index
			cServedIndexRefPtr_c pServed = CheckIndexSelectable ( sLocal, szParent, &dNFailuresSet );
			if ( !pServed )
				continue;

			bool bResult = false;
			CSphQueryResultMeta tMqMeta;
			CSphQueryResult tMqRes;
			tMqRes.m_pMeta = &tMqMeta;

			{	// scope for r-locking the index
				RIdx_c pIndex { pServed };

				tCtx.m_tHook.SetIndex ( pIndex );
				tCtx.m_tHook.SetQueryType ( m_eQueryType );

				// create sorters
				SphQueueRes_t tQueueRes;
				if ( !CreateValidSorters ( dSorters, &tQueueRes, dNFailuresSet, pExtra, pIndex, sLocal, szParent, &tCtx.m_tHook ) )
					continue;

				// do the query
				CSphMultiQueryArgs tMultiArgs ( iIndexWeight );
				tMultiArgs.m_uPackedFactorFlags = tQueueRes.m_uPackedFactorFlags;
				if ( m_bGotLocalDF )
				{
					tMultiArgs.m_bLocalDF = true;
					tMultiArgs.m_pLocalDocs = &m_hLocalDocs;
					tMultiArgs.m_iTotalDocs = m_iTotalDocs;
				}

				bool bCanBeCloned = dSorters.all_of ( []( auto * pSorter ){ return pSorter ? pSorter->CanBeCloned() : true; } );

				// fixme: previous calculations are wrong; we are not splitting the query if we are using non-clonable sorters
				tMultiArgs.m_iSplit = bCanBeCloned ? dSplits[iLocal] : 1;
				tMultiArgs.m_bFinalizeSorters = !tGlobalSorters.NeedGlobalSorters();

				dNAggrResults.First().m_tIOStats.Start ();
				if ( m_bMultiQueue )
					bResult = pIndex->MultiQuery ( tMqRes, m_dNQueries.First(), dSorters, tMultiArgs );
				else
					bResult = pIndex->MultiQueryEx ( iQueries, &m_dNQueries[0], &dNResults[0], &dSorters[0], tMultiArgs );
				dNAggrResults.First ().m_tIOStats.Stop ();
			}

			iCpuTime += sphTaskCpuTimer ();

			// handle results
			if ( bResult )
			{
				// multi-query succeeded
				for ( int i=0; i<iQueries; ++i )
				{
					// in mt here kind of tricky index calculation, up to the next lines with sorter
					// but some of the sorters could had failed at "create sorter" stage
					ISphMatchSorter * pSorter = dSorters[i];
					if ( !pSorter )
						continue;

					// this one seems OK
					AggrResult_t & tNRes = dNAggrResults[i];
					int iQTimeForStats = tNRes.m_iQueryTime;
					auto pDocstore = m_bMultiQueue ? tMqRes.m_pDocstore : dNResults[i].m_pDocstore;
					// multi-queue only returned one result set meta, so we need to replicate it
					if ( m_bMultiQueue )
					{
						// these times will be overridden below, but let's be clean
						iQTimeForStats = tMqMeta.m_iQueryTime / iQueries;
						tNRes.m_iQueryTime += iQTimeForStats;
						iCpuTime /= iQueries;
						tNRes.MergeWordStats ( tMqMeta );
						tNRes.m_iMultiplier = iQueries;
						tNRes.m_iCpuTime += tMqMeta.m_iCpuTime / iQueries;
					} else if ( tNRes.m_iMultiplier==-1 ) // multiplier -1 means 'error'
					{
						dNFailuresSet[i].Submit ( sLocal, szParent, tNRes.m_sError.cstr() );
						continue;
					}
					++tNRes.m_iSuccesses;
					tNRes.m_iCpuTime = iCpuTime;
					tNRes.m_iTotalMatches += pSorter->GetTotalCount();
					tNRes.m_iPredictedTime = tNRes.m_bHasPrediction ? CalcPredictedTimeMsec ( tNRes ) : 0;

					m_dQueryIndexStats[iLocal].m_dStats[i].m_iSuccesses = 1;
					m_dQueryIndexStats[iLocal].m_dStats[i].m_uQueryTime = iQTimeForStats;
					m_dQueryIndexStats[iLocal].m_dStats[i].m_uFoundRows = pSorter->GetTotalCount();

					iTotalSuccesses.fetch_add ( 1, std::memory_order_relaxed );

					// extract matches from sorter
					if ( !tGlobalSorters.StoreSorter ( i, iLocal, dSorters[i], pDocstore, iOrderTag ) )
						tNRes.AddResultset( pSorter, pDocstore, iOrderTag, m_dNQueries[i].m_iCutoff );

					if ( !tNRes.m_sWarning.IsEmpty () )
						dNFailuresSet[i].Submit ( sLocal, szParent, tNRes.m_sWarning.cstr () );
				}
			} else
				// failed, submit local (if not empty) or global error string
				for ( int i = 0; i<iQueries; ++i )
					dNFailuresSet[i].Submit ( sLocal, szParent, tMqMeta.m_sError.IsEmpty ()
							? dNAggrResults[m_bMultiQueue ? 0 : i].m_sError.cstr ()
							: tMqMeta.m_sError.cstr () );

			// cleanup sorters
			for ( auto &pSorter : dSorters )
				SafeDelete ( pSorter );

			if ( iJob+1<iJobs ) // no need to throttle here if we're done
				tThrottler.ThrottleAndKeepCrashQuery (); // we set CrashQuery anyway at the start of the loop
		}
	});
	dCtx.Finalize (); // merge mt results (if any)

	tGlobalSorters.MergeResults(m_dNAggrResults);

	// update our wall time for every result set
	tmLocal = sphMicroTimer ()-tmLocal;
	for ( int iQuery = 0; iQuery<iQueries; ++iQuery )
		m_dNAggrResults[iQuery].m_iQueryTime += (int) ( tmLocal / 1000 );

	auto iTotalSuccessesInt = iTotalSuccesses.load ( std::memory_order_relaxed );

	for ( auto iLocal = 0; iLocal<iNumLocals; ++iLocal )
		for ( int iQuery = 0; iQuery<iQueries; ++iQuery )
		{
			QueryStat_t & tStat = m_dQueryIndexStats[iLocal].m_dStats[iQuery];
			if ( tStat.m_iSuccesses )
				tStat.m_uQueryTime = (int) ( tmLocal / 1000 / iTotalSuccessesInt );
		}
}

// check expressions into a query to make sure that it's ready for multi query optimization
bool SearchHandler_c::AllowsMulti() const
{
	if ( m_bFacetQueue )
		return true;

	// in some cases the same select list allows queries to be multi query optimized
	// but we need to check dynamic parts size equality and we do it later in RunLocalSearches()
	const CSphVector<CSphQueryItem> & tFirstQueryItems = m_dNQueries.First().m_dItems;
	bool bItemsSameLen = true;
	for ( int i=1; i<m_dNQueries.GetLength() && bItemsSameLen; ++i )
		bItemsSameLen = ( tFirstQueryItems.GetLength()==m_dNQueries[i].m_dItems.GetLength() );
	if ( bItemsSameLen )
	{
		bool bSameItems = true;
		ARRAY_FOREACH_COND ( i, tFirstQueryItems, bSameItems )
		{
			const CSphQueryItem & tItem1 = tFirstQueryItems[i];
			for ( int j=1; j<m_dNQueries.GetLength () && bSameItems; ++j )
			{
				const CSphQueryItem & tItem2 = m_dNQueries[j].m_dItems[i];
				bSameItems = tItem1.m_sExpr==tItem2.m_sExpr && tItem1.m_eAggrFunc==tItem2.m_eAggrFunc;
			}
		}

		if ( bSameItems )
			return true;
	}

	// if select lists do not contain any expressions we can optimize queries too
	for ( const auto & dLocal : m_dLocal )
	{
		RIdx_c pServedIndex ( m_dAcquired.Get ( dLocal.m_sName ) );
		// FIXME!!! compare expressions as m_pExpr->GetHash
		const CSphSchema & tSchema = pServedIndex->GetMatchSchema();
		if ( m_dNQueries.any_of ( [&tSchema] ( const CSphQuery & tQ ) { return sphHasExpressions ( tQ, tSchema ); } ) )
			return false;
	}
	return true;
}


struct IndexSettings_t
{
	uint64_t	m_uHash;
	int			m_iLocal;
};

void SearchHandler_c::SetupLocalDF ()
{
	if ( m_dLocal.GetLength()<2 )
		return;

	SwitchProfile ( m_pProfile, SPH_QSTATE_LOCAL_DF );

	bool bGlobalIDF = true;
	ARRAY_FOREACH_COND ( i, m_dLocal, bGlobalIDF )
	{
		auto pDesc = GetServed( m_dLocal[i].m_sName );
		bGlobalIDF = ( pDesc && !pDesc->m_sGlobalIDFPath.IsEmpty () );
	}
	// bail out on all indexes with global idf set
	if ( bGlobalIDF )
		return;

	bool bOnlyNoneRanker = true;
	bool bOnlyFullScan = true;
	bool bHasLocalDF = false;
	for ( const CSphQuery & tQuery : m_dNQueries )
	{
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
	for ( const CSphQuery & tQuery : m_dNQueries )
	{
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
		dLocal.Add();
		dLocal.Last().m_iLocal = i;
		// TODO: cache settingsFNV on index load
		// FIXME!!! no need to count dictionary hash
		RIdx_c pIndex ( m_dAcquired.Get ( m_dLocal[i].m_sName ) );
		dLocal.Last().m_uHash = pIndex->GetTokenizer()->GetSettingsFNV() ^ pIndex->GetDictionary()->GetSettingsFNV();
	}
	dLocal.Sort ( bind ( &IndexSettings_t::m_uHash ) );

	// gather per-term docs count
	CSphVector < CSphKeywordInfo > dKeywords;
	ARRAY_FOREACH ( i, dLocal )
	{
		int iLocalIndex = dLocal[i].m_iLocal;
		RIdx_c pIndex ( m_dAcquired.Get ( m_dLocal[iLocalIndex].m_sName ) );
		m_iTotalDocs += pIndex->GetStats().m_iTotalDocuments;

		if ( i && dLocal[i].m_uHash==dLocal[i-1].m_uHash )
		{
			dKeywords.Apply ( [] ( CSphKeywordInfo & tKw ) { tKw.m_iDocs = 0; } );

			// no need to tokenize query just fill docs count
			pIndex->FillKeywords ( dKeywords );
		} else
		{
			GetKeywordsSettings_t tSettings;
			tSettings.m_bStats = true;
			dKeywords.Resize ( 0 );
			pIndex->GetKeywords ( dKeywords, dQuery.Begin(), tSettings, NULL );

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

		for ( auto& tKw: dKeywords )
		{
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

uint64_t CalculateMass ( const CSphIndexStatus & dStats )
{
	auto iOvermapped = dStats.m_iMapped-dStats.m_iMappedResident;
	if ( iOvermapped<0 ) // it could be negative since resident is rounded up to page edge
		iOvermapped = 0;
	return 1000000 * dStats.m_iNumChunks
			+ 10 * iOvermapped
			+ dStats.m_iRamUse;
}

static uint64_t GetIndexMass ( const CSphString & sName )
{
	return ServedIndex_c::GetIndexMass ( GetServed ( sName ) );
}

// declared to be used in ParseSysVar
void HandleMysqlShowThreads ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleMysqlShowTables ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleTasks ( RowBuffer_i & tOut );
void HandleSched ( RowBuffer_i & tOut );
void HandleMysqlDescribe ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleSelectIndexStatus ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleSelectFiles ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );

bool SearchHandler_c::ParseSysVar ()
{
	const auto& sVar = m_dLocal.First().m_sName;
	const auto & dSubkeys = m_dNQueries.First ().m_dStringSubkeys;

	if ( sVar=="@@system" )
	{
		if ( !dSubkeys.IsEmpty () )
		{
			bool bSchema = ( dSubkeys.Last ()==".table" );
			bool bValid = true;
			TableFeeder_fn fnFeed;
			if ( dSubkeys[0]==".threads" ) // select .. from @@system.threads
			{
				if ( m_pStmt->m_sThreadFormat.IsEmpty() ) // override format to show all columns by default
					m_pStmt->m_sThreadFormat="all";

				fnFeed = [this] ( RowBuffer_i * pBuf ) { HandleMysqlShowThreads ( *pBuf, m_pStmt ); };
			}
			else if ( dSubkeys[0]==".tables" ) // select .. from @@system.tables
			{
				fnFeed = [this] ( RowBuffer_i * pBuf ) { HandleMysqlShowTables ( *pBuf, m_pStmt ); };
			}
			else if ( dSubkeys[0]==".tasks" ) // select .. from @@system.tasks
			{
				fnFeed = [] ( RowBuffer_i * pBuf ) { HandleTasks ( *pBuf ); };
			}
			else if ( dSubkeys[0]==".sched" ) // select .. from @@system.sched
			{
				fnFeed = [] ( RowBuffer_i * pBuf ) { HandleSched ( *pBuf ); };
			}
			else
				bValid = false;

			if ( bValid )
			{
				cServedIndexRefPtr_c pIndex;
				if ( bSchema )
				{
					m_dLocal.First ().m_sName.SetSprintf( "@@system.%s.table", dSubkeys[0].cstr() );
					pIndex = MakeDynamicIndexSchema ( std::move ( fnFeed ) );

				} else {
					m_dLocal.First ().m_sName.SetSprintf ( "@@system.%s", dSubkeys[0].cstr () );
					pIndex = MakeDynamicIndex ( std::move ( fnFeed ) );
				}
				m_dAcquired.AddIndex ( m_dLocal.First ().m_sName, std::move (pIndex) );
				return true;
			}
		}
	}

	StringBuilder_c sN;
	sN << sVar;
	dSubkeys.for_each ( [&sN] ( CSphString & sVal ) { sN << sVal; } );
	m_dNAggrResults.for_each (
			[&sN] ( AggrResult_t & r ) { r.m_sError.SetSprintf ( "no such variable %s", sN.cstr () ); } );
	return false;
}

bool SearchHandler_c::ParseIdxSubkeys ()
{
	const auto & sVar = m_dLocal.First ().m_sName;
	const auto & dSubkeys = m_dNQueries.First ().m_dStringSubkeys;

	if ( dSubkeys.IsEmpty () )
		return false;

	bool bSchema = ( dSubkeys.GetLength()>1 && dSubkeys.Last ()==".table" );
	TableFeeder_fn fnFeed;
	if ( dSubkeys[0]==".table" ) // select .. idx.table
		fnFeed = [this] ( RowBuffer_i * pBuf ) { HandleMysqlDescribe ( *pBuf, m_pStmt ); };
	else if ( dSubkeys[0]==".status" ) // select .. idx.status
		fnFeed = [this] ( RowBuffer_i * pBuf ) { HandleSelectIndexStatus ( *pBuf, m_pStmt ); };
	else if ( dSubkeys[0]==".files" ) // select .. from idx.files
		fnFeed = [this] ( RowBuffer_i * pBuf ) { HandleSelectFiles ( *pBuf, m_pStmt ); };
	else
		return false;

	cServedIndexRefPtr_c pIndex;
	if ( bSchema )
	{
		m_dLocal.First ().m_sName.SetSprintf ( "%s%s.table", sVar.cstr (), dSubkeys[0].cstr () );
		pIndex = MakeDynamicIndexSchema ( std::move ( fnFeed ) );
	} else
	{
		m_dLocal.First ().m_sName.SetSprintf ( "%s%s", sVar.cstr (), dSubkeys[0].cstr () );
		pIndex = MakeDynamicIndex ( std::move ( fnFeed ) );
	}

	m_dAcquired.AddIndex ( m_dLocal.First().m_sName, std::move ( pIndex ) );
	return true;
}

////////////////////////////////////////////////////////////////
// check for single-query, multi-queue optimization possibility
////////////////////////////////////////////////////////////////
bool SearchHandler_c::CheckMultiQuery() const
{
	const int iQueries = m_dNQueries.GetLength();
	if ( iQueries<=1 )
		return false;

	const CSphQuery & qFirst = m_dNQueries.First();
	auto dQueries = m_dNQueries.Slice ( 1 );

	// queries over special indexes as status/meta are not capable for multiquery
	if ( !qFirst.m_dStringSubkeys.IsEmpty() )
		return false;

	for ( const CSphQuery & qCheck : dQueries )
	{
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
bool SearchHandler_c::AcquireInvokedIndexes()
{
	// if unexistent allowed, short flow
	if ( m_dNQueries.First().m_bIgnoreNonexistentIndexes )
	{
		ARRAY_FOREACH ( i, m_dLocal )
			if ( !m_dAcquired.AddUniqIndex ( m_dLocal[i].m_sName ) )
				m_dLocal.Remove ( i-- );
		return true;
	}

	// _build the list of non-existent
	StringBuilder_c sFailed (", ");
	for ( const auto & dLocal : m_dLocal )
		if ( !m_dAcquired.AddUniqIndex ( dLocal.m_sName ) )
			sFailed << dLocal.m_sName;

	// no absent indexes, viola!
	if ( sFailed.IsEmpty ())
		return true;

	// report failed for each result
	for ( auto& dResult : m_dNAggrResults )
		dResult.m_sError.SetSprintf ( "unknown local index(es) '%s' in search request", sFailed.cstr() );

	return false;
}

// uniq dLocals and copy into m_dLocal only uniq part.
void SearchHandler_c::UniqLocals ( VecTraits_T<LocalIndex_t> & dLocals )
{
	int iLen = dLocals.GetLength ();
	if ( !iLen )
		return;

	CSphVector<int> dOrder;
	dOrder.Resize ( dLocals.GetLength() );
	dOrder.FillSeq();

	dOrder.Sort ( Lesser ( [&dLocals] ( int a, int b )
	{
		return ( dLocals[a].m_sName<dLocals[b].m_sName )
			|| ( dLocals[a].m_sName==dLocals[b].m_sName && dLocals[a].m_iOrderTag>dLocals[b].m_iOrderTag );
	}));

	int iSrc = 1, iDst = 1;
	while ( iSrc<iLen )
	{
		if ( dLocals[dOrder[iDst-1]].m_sName==dLocals[dOrder[iSrc]].m_sName )
			++iSrc;
		else
			dOrder[iDst++] = dOrder[iSrc++];
	}

	dOrder.Resize ( iDst );
	m_dLocal.Resize ( iDst );
	ARRAY_FOREACH ( i, dOrder )
		m_dLocal[i] = std::move ( dLocals[dOrder[i]] );
}


void SearchHandler_c::CalcTimeStats ( int64_t tmCpu, int64_t tmSubset, const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent )
{
	// in multi-queue case (1 actual call per N queries), just divide overall query time evenly
	// otherwise (N calls per N queries), divide common query time overheads evenly
	const int iQueries = m_dNQueries.GetLength();
	if ( m_bMultiQueue )
	{
		for ( auto & dResult : m_dNAggrResults )
		{
			dResult.m_iQueryTime = (int)( tmSubset/1000/iQueries );
			dResult.m_iRealQueryTime = (int)( tmSubset/1000/iQueries );
			dResult.m_iCpuTime = tmCpu/iQueries;
		}
		return;
	}

	int64_t tmAccountedWall = 0;
	int64_t tmAccountedCpu = 0;
	for ( const auto & dResult : m_dNAggrResults )
	{
		tmAccountedWall += dResult.m_iQueryTime*1000;
		assert ( ( dResult.m_iCpuTime==0 && dResult.m_iAgentCpuTime==0 ) ||	// all work was done in this thread
			( dResult.m_iCpuTime>0 && dResult.m_iAgentCpuTime==0 ) ||		// children threads work
			( dResult.m_iAgentCpuTime>0 && dResult.m_iCpuTime==0 ) );		// agents work
		tmAccountedCpu += dResult.m_iCpuTime;
		tmAccountedCpu += dResult.m_iAgentCpuTime;
	}

	// whether we had work done in children threads (dist_threads>1) or in agents
	bool bExternalWork = tmAccountedCpu!=0;
	int64_t tmDeltaWall = ( tmSubset - tmAccountedWall ) / iQueries;

	for ( auto & dResult : m_dNAggrResults )
	{
		dResult.m_iQueryTime += (int)(tmDeltaWall/1000);
		dResult.m_iRealQueryTime = (int)( tmSubset/1000/iQueries );
		dResult.m_iCpuTime = tmCpu/iQueries;
		if ( bExternalWork )
			dResult.m_iCpuTime += tmAccountedCpu;
	}

	// don't forget to add this to stats
	if ( bExternalWork )
		tmCpu += tmAccountedCpu;

	// correct per-index stats from agents
	int iTotalSuccesses = 0;
	for ( const auto & dResult : m_dNAggrResults )
		iTotalSuccesses += dResult.m_iSuccesses;

	if ( !iTotalSuccesses )
		return;

	int64_t tmDelta = tmSubset - tmAccountedWall;

	auto nValidDistrIndexes = dDistrServedByAgent.count_of ( [] ( auto& t ) { return t.m_dStats.any_of ( [] ( auto& i ) { return i.m_iSuccesses; } ); } );
	int64_t nDistrDivider = iTotalSuccesses * nValidDistrIndexes * 1000;
	if ( nDistrDivider )
		for ( auto &tDistrStat : dDistrServedByAgent )
			for ( QueryStat_t& tStat : tDistrStat.m_dStats )
			{
				auto tmDeltaWallAgent = tmDelta * tStat.m_iSuccesses / nDistrDivider;
				tStat.m_uQueryTime += (int)tmDeltaWallAgent;
			}

	auto nValidLocalIndexes = m_dQueryIndexStats.count_of ( [] ( auto& t ) { return t.m_dStats.any_of ( [] ( auto& i ) { return i.m_iSuccesses; } ); } );
	int64_t nLocalDivider = iTotalSuccesses * nValidLocalIndexes * 1000;
	if ( nLocalDivider )
		for ( auto &dQueryIndexStat : m_dQueryIndexStats )
			for ( QueryStat_t& tStat : dQueryIndexStat.m_dStats )
			{
				int64_t tmDeltaWallLocal = tmDelta * tStat.m_iSuccesses / nLocalDivider;
				tStat.m_uQueryTime += (int)tmDeltaWallLocal;
			}
}


void SearchHandler_c::CalcPerIndexStats ( const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent ) const
{
	const int iQueries = m_dNQueries.GetLength();
	// calculate per-index stats
	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		const auto& pServed = m_dAcquired.Get ( m_dLocal[iLocal].m_sName );
		for ( int iQuery=0; iQuery<iQueries; ++iQuery )
		{
			QueryStat_t & tStat = m_dQueryIndexStats[iLocal].m_dStats[iQuery];
			if ( !tStat.m_iSuccesses )
				continue;

			pServed->m_pStats->AddQueryStat ( tStat.m_uFoundRows, tStat.m_uQueryTime );
			for ( auto &tDistr : dDistrServedByAgent )
			{
				if ( tDistr.m_dLocalNames.Contains ( m_dLocal[iLocal].m_sName ) )
				{
					tDistr.m_dStats[iQuery].m_uQueryTime += tStat.m_uQueryTime;
					tDistr.m_dStats[iQuery].m_uFoundRows += tStat.m_uFoundRows;
					++tDistr.m_dStats[iQuery].m_iSuccesses;
				}
			}
		}
	}

	for ( auto &tDistr : dDistrServedByAgent )
	{
		auto pServedDistIndex = GetDistr ( tDistr.m_sIndex );
		if ( pServedDistIndex )
			for ( int iQuery=0; iQuery<iQueries; ++iQuery )
			{
				auto & tStat = tDistr.m_dStats[iQuery];
				if ( !tStat.m_iSuccesses )
					continue;

				pServedDistIndex->m_tStats.AddQueryStat ( tStat.m_uFoundRows, tStat.m_uQueryTime );
			}
	}
}


void SearchHandler_c::CalcGlobalStats ( int64_t tmCpu, int64_t tmSubset, int64_t tmLocal, const CSphIOStats & tIO, const VecRefPtrsAgentConn_t & dRemotes ) const
{
	auto & g_tStats = gStats ();
	g_tStats.m_iQueries.fetch_add ( m_dNQueries.GetLength (), std::memory_order_relaxed );
	g_tStats.m_iQueryTime.fetch_add ( tmSubset, std::memory_order_relaxed );
	g_tStats.m_iQueryCpuTime.fetch_add ( tmCpu, std::memory_order_relaxed );
	if ( dRemotes.GetLength() )
	{
		int64_t tmWait = 0;
		for ( const AgentConn_t * pAgent : dRemotes )
			tmWait += pAgent->m_iWaited;

		// do *not* count queries to dist indexes w/o actual remote agents
		g_tStats.m_iDistQueries.fetch_add ( 1, std::memory_order_relaxed );
		g_tStats.m_iDistWallTime.fetch_add ( tmSubset, std::memory_order_relaxed );
		g_tStats.m_iDistLocalTime.fetch_add ( tmLocal, std::memory_order_relaxed );
		g_tStats.m_iDistWaitTime.fetch_add ( tmWait, std::memory_order_relaxed );
	}
	g_tStats.m_iDiskReads.fetch_add ( tIO.m_iReadOps, std::memory_order_relaxed );
	g_tStats.m_iDiskReadTime.fetch_add ( tIO.m_iReadTime, std::memory_order_relaxed );
	g_tStats.m_iDiskReadBytes.fetch_add ( tIO.m_iReadBytes, std::memory_order_relaxed );
}

static CSphVector<LocalIndex_t> CollectAllLocalIndexes ( const CSphVector<CSphNamedInt> & dIndexWeights )
{
	CSphVector<LocalIndex_t> dIndexes;
	int iOrderTag = 0;
	// search through all local indexes
	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( auto& tIt : *hLocal )
	{
		if ( !tIt.second ) // fixme! should never be...
			continue;
		auto & dLocal = dIndexes.Add ();
		dLocal.m_sName = tIt.first;
		dLocal.m_iOrderTag = iOrderTag++;
		dLocal.m_iWeight = GetIndexWeight ( tIt.first, dIndexWeights, 1 );
		dLocal.m_iMass = ServedIndex_c::GetIndexMass ( tIt.second );
	}
	return dIndexes;
}

// returns true = real indexes, false = sysvar (i.e. only one 'index' named from @@)
bool SearchHandler_c::BuildIndexList ( int & iDivideLimits, VecRefPtrsAgentConn_t & dRemotes, CSphVector<DistrServedByAgent_t> & dDistrServedByAgent )
{
	const CSphQuery & tQuery = m_dNQueries.First ();

	if ( tQuery.m_sIndexes=="*" )
	{
		// they're all local, build the list
		m_dLocal = CollectAllLocalIndexes ( tQuery.m_dIndexWeights );
		return true;
	}

	m_dLocal.Reset ();
	int iOrderTag = 0;
	bool bSysVar = tQuery.m_sIndexes.Begins ( "@@" );

	// search through specified local indexes
	StrVec_t dIdxNames;
	if ( bSysVar )
		dIdxNames.Add ( tQuery.m_sIndexes );
	else
		ParseIndexList ( tQuery.m_sIndexes, dIdxNames );

	const int iQueries = m_dNQueries.GetLength ();
	CSphVector<LocalIndex_t> dLocals;

	int iDistCount = 0;
	bool bDivideRemote = false;
	bool bHasLocalsAgents = false;

	for ( const auto& sIndex : dIdxNames )
	{
		auto pDist = GetDistr ( sIndex );
		if ( !pDist )
		{
			auto &dLocal = dLocals.Add ();
			dLocal.m_sName = sIndex;
			dLocal.m_iOrderTag = iOrderTag++;
			dLocal.m_iWeight = GetIndexWeight ( sIndex, tQuery.m_dIndexWeights, 1 );
			dLocal.m_iMass = GetIndexMass ( sIndex );
		} else
		{
			++iDistCount;
			int iWeight = GetIndexWeight ( sIndex, tQuery.m_dIndexWeights, -1 );
			auto & tDistrStat = dDistrServedByAgent.Add();
			tDistrStat.m_sIndex = sIndex;
			tDistrStat.m_dStats.Resize ( iQueries );
			tDistrStat.m_dStats.ZeroVec();

			for ( const auto& pAgent : pDist->m_dAgents )
			{
				tDistrStat.m_dAgentIds.Add ( dRemotes.GetLength() );
				auto * pConn = new AgentConn_t;
				pConn->SetMultiAgent ( pAgent );
				pConn->m_iStoreTag = iOrderTag++;
				pConn->m_iWeight = iWeight;
				pConn->m_iMyConnectTimeoutMs = pDist->m_iAgentConnectTimeoutMs;
				pConn->m_iMyQueryTimeoutMs = pDist->m_iAgentQueryTimeoutMs;
				dRemotes.Add ( pConn );
			}

			ARRAY_CONSTFOREACH ( j, pDist->m_dLocal )
			{
				const CSphString& sLocalAgent = pDist->m_dLocal[j];
				tDistrStat.m_dLocalNames.Add ( sLocalAgent );
				auto &dLocal = dLocals.Add ();
				dLocal.m_sName = sLocalAgent;
				dLocal.m_iOrderTag = iOrderTag++;
				if ( iWeight!=-1 )
					dLocal.m_iWeight = iWeight;
				dLocal.m_iMass = GetIndexMass ( sLocalAgent );
				dLocal.m_sParentIndex = sIndex;
				bHasLocalsAgents = true;
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
			for ( auto& dResult : m_dNAggrResults )
				dResult.m_sWarning.SetSprintf ( "distributed multi-index query '%s' doesn't support divide_remote_ranges", tQuery.m_sIndexes.cstr() );
		}
	}

	// eliminate local dupes that come from distributed indexes
	if ( bHasLocalsAgents )
		UniqLocals ( dLocals );
	else
		m_dLocal.SwapData ( dLocals );

	return !bSysVar;
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
	dDst.m_sChain << "Query ";
	hazard::Guard_c tGuard;
	auto pQuery = tGuard.Protect ( tInfo.m_pHazardQuery );
	if ( pQuery && session::GetProto()!=Proto_e::MYSQL41 ) // cheat: for mysql query not used, so will not copy it then
		dDst.m_pQuery = std::make_unique<CSphQuery> ( *pQuery );
}

// one or more queries against one and same set of indexes
void SearchHandler_c::RunSubset ( int iStart, int iEnd )
{
	int iQueries = iEnd - iStart;
	m_dNQueries = m_dQueries.Slice ( iStart, iQueries );
	m_dNAggrResults = m_dAggrResults.Slice ( iStart, iQueries );
	m_dNResults = m_dResults.Slice ( iStart, iQueries );
	m_dNFailuresSet = m_dFailuresSet.Slice ( iStart, iQueries );

	// we've own scoped context here
	auto pQueryInfo = new QueryInfo_t;
	pQueryInfo->m_pHazardQuery.store ( m_dNQueries.begin(), std::memory_order_release );
	ScopedInfo_T<QueryInfo_t> pTlsQueryInfo ( pQueryInfo );

	// all my stats
	int64_t tmSubset = -sphMicroTimer();
	int64_t tmLocal = 0;
	int64_t tmCpu = -sphTaskCpuTimer ();

	CSphScopedProfile tProf ( m_pProfile, SPH_QSTATE_UNKNOWN );

	// prepare for descent
	const CSphQuery & tFirst = m_dNQueries.First();
	m_dNAggrResults.Apply ( [] ( AggrResult_t & r ) { r.m_iSuccesses = 0; } );

	if ( iQueries==1 && m_pProfile )
	{
		m_dNAggrResults.First().m_pProfile = m_pProfile;
		m_tHook.SetProfiler ( m_pProfile );
	}

	// check for facets
	m_bFacetQueue = iQueries>1;
	for ( int iCheck = 1; iCheck<m_dNQueries.GetLength () && m_bFacetQueue; ++iCheck )
		if ( !m_dNQueries[iCheck].m_bFacet )
			m_bFacetQueue = false;

	m_bMultiQueue = m_bFacetQueue || CheckMultiQuery();

	////////////////////////////
	// build local indexes list
	////////////////////////////
	VecRefPtrsAgentConn_t dRemotes;
	CSphVector<DistrServedByAgent_t> dDistrServedByAgent;
	int iDivideLimits = 1;

	if ( BuildIndexList ( iDivideLimits, dRemotes, dDistrServedByAgent ) )
	{
		// process query to meta, as myindex.status, etc.
		if ( !tFirst.m_dStringSubkeys.IsEmpty () )
		{
			// if apply subkeys ... else return
			if ( !ParseIdxSubkeys () )
				return;
		} else if ( !AcquireInvokedIndexes () ) // usual query processing
			return;
	} else
	{
		// process query to @@*, as @@system.threads, etc.
		if ( !ParseSysVar () )
			return;
		// here we deal
	}

	// at this point m_dLocal contains list of valid local indexes (i.e., existing ones),
	// and these indexes are also rlocked and available by calling m_dAcquired.Get()

	// sanity check
	if ( dRemotes.IsEmpty() && m_dLocal.IsEmpty() )
	{
		const char * szT = ( m_dLocal.IsEmpty () ? "indexes" : "local indexes" );
		m_dNAggrResults.Apply ( [szT] ( AggrResult_t & r ) { r.m_sError.SetSprintf ( "no enabled %s to search", szT );} );
		return;
	}

	// select lists must have no expressions
	if ( m_bMultiQueue )
		m_bMultiQueue = AllowsMulti ();

	assert ( !m_bFacetQueue || AllowsMulti () );
	if ( !m_bMultiQueue )
		m_bFacetQueue = false;

	///////////////////////////////////////////////////////////
	// main query loop (with multiple retries for distributed)
	///////////////////////////////////////////////////////////

	// connect to remote agents and query them, if required
	std::unique_ptr<SearchRequestBuilder_c> tReqBuilder;
	CSphRefcountedPtr<RemoteAgentsObserver_i> tReporter { nullptr };
	std::unique_ptr<ReplyParser_i> tParser;;
	if ( !dRemotes.IsEmpty() )
	{
		SwitchProfile(m_pProfile, SPH_QSTATE_DIST_CONNECT);
		tReqBuilder = std::make_unique<SearchRequestBuilder_c> ( m_dNQueries, iDivideLimits );
		tParser = std::make_unique<SearchReplyParser_c> ( iQueries );
		tReporter = GetObserver();

		// run remote queries. tReporter will tell us when they're finished.
		// also blackholes will be removed from this flow of remotes.
		ScheduleDistrJobs ( dRemotes, tReqBuilder.get (),
			tParser.get (),
			tReporter, tFirst.m_iRetryCount, tFirst.m_iRetryDelay );
	}

	/////////////////////
	// run local queries
	//////////////////////

	// while the remote queries are running, do local searches
	if ( m_dLocal.GetLength() )
	{
		SetupLocalDF();

		SwitchProfile ( m_pProfile, SPH_QSTATE_LOCAL_SEARCH );
		m_bNeedDocIDs = m_dLocal.GetLength()+dRemotes.GetLength()>1;
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
		SwitchProfile ( m_pProfile, SPH_QSTATE_DIST_WAIT );

		bool bDistDone = false;
		while ( !bDistDone )
		{
			// don't forget to check incoming replies after send was over
			bDistDone = tReporter->IsDone();
			if ( !bDistDone )
				tReporter->WaitChanges (); /// wait one or more remote queries to complete. Note! M.b. context switch!

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

				// merge this agent's results
				for ( int iRes = 0; iRes<iQueries; ++iRes )
				{
					auto pResult = ( cSearchResult * ) pAgent->m_pResult.get ();
					if ( !pResult )
						continue;

					auto &tRemoteResult = pResult->m_dResults[iRes];

					// copy errors or warnings
					if ( !tRemoteResult.m_sError.IsEmpty() )
						m_dNFailuresSet[iRes].SubmitEx ( tFirst.m_sIndexes, nullptr,
							"agent %s: remote query error: %s",
							pAgent->m_tDesc.GetMyUrl().cstr(), tRemoteResult.m_sError.cstr() );
					if ( !tRemoteResult.m_sWarning.IsEmpty() )
						m_dNFailuresSet[iRes].SubmitEx ( tFirst.m_sIndexes, nullptr,
							"agent %s: remote query warning: %s",
							pAgent->m_tDesc.GetMyUrl().cstr(), tRemoteResult.m_sWarning.cstr() );

					if ( tRemoteResult.m_iSuccesses<=0 )
						continue;

					AggrResult_t & tRes = m_dNAggrResults[iRes];
					++tRes.m_iSuccesses;

					assert ( tRemoteResult.m_dResults.GetLength() == 1 ); // by design remotes return one chunk
					auto & dRemoteChunk = tRes.m_dResults.Add ();
					::Swap ( dRemoteChunk, *tRemoteResult.m_dResults.begin () );

					// note how we do NOT add per-index weight here

					// merge this agent's stats
					tRes.m_iTotalMatches += tRemoteResult.m_iTotalMatches;
					tRes.m_bTotalMatchesApprox |= tRemoteResult.m_bTotalMatchesApprox;
					tRes.m_iQueryTime += tRemoteResult.m_iQueryTime;
					tRes.m_iAgentCpuTime += tRemoteResult.m_iCpuTime;
					tRes.m_tAgentIOStats.Add ( tRemoteResult.m_tIOStats );
					tRes.m_iAgentPredictedTime += tRemoteResult.m_iPredictedTime;
					tRes.m_iAgentFetchedDocs += tRemoteResult.m_iAgentFetchedDocs;
					tRes.m_iAgentFetchedHits += tRemoteResult.m_iAgentFetchedHits;
					tRes.m_iAgentFetchedSkips += tRemoteResult.m_iAgentFetchedSkips;
					tRes.m_bHasPrediction |= ( m_dNQueries[iRes].m_iMaxPredictedMsec>0 );

					if ( pDistr )
					{
						pDistr->m_dStats[iRes].m_uQueryTime += tRemoteResult.m_iQueryTime;
						pDistr->m_dStats[iRes].m_uFoundRows += tRemoteResult.m_iTotalMatches;
						++pDistr->m_dStats[iRes].m_iSuccesses;
					}

					// merge this agent's words
					tRes.MergeWordStats ( tRemoteResult );
				}

				// dismissed
				if ( pAgent->m_pResult )
					pAgent->m_pResult->Reset ();
				pAgent->m_bSuccess = false;
				pAgent->m_sFailure = "";
			}
		} // while ( !bDistDone )

		// submit failures from failed agents
		// copy timings from all agents

		for ( const AgentConn_t * pAgent : dRemotes )
		{
			assert ( !pAgent->IsBlackhole () ); // must not be any blacknole here.

			for ( int j=iStart; j<iEnd; ++j )
			{
				assert ( pAgent->m_iWall>=0 );
				m_dAgentTimes[j].Add ( ( pAgent->m_iWall ) / ( 1000 * iQueries ) );
			}

			if ( !pAgent->m_bSuccess && !pAgent->m_sFailure.IsEmpty() )
				for ( int j=0; j<iQueries; ++j )
					m_dNFailuresSet[j].SubmitEx ( tFirst.m_sIndexes, nullptr, "agent %s: %s",
						pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
		}
	}

	/////////////////////
	// merge all results
	/////////////////////

	SwitchProfile ( m_pProfile, SPH_QSTATE_AGGREGATE );

	CSphIOStats tIO;

	for ( int iRes=0; iRes<iQueries; ++iRes )
	{
		sph::StringSet hExtra;
		for ( const CSphString & sExtra : m_dExtraSchema )
			hExtra.Add ( sExtra );

		AggrResult_t & tRes = m_dNAggrResults[iRes];
		const CSphQuery & tQuery = m_dNQueries[iRes];

		// minimize sorters needs these pointers
		tIO.Add ( tRes.m_tIOStats );

		// if there were no successful searches at all, this is an error
		if ( !tRes.m_iSuccesses )
		{
			StringBuilder_c sFailures;
			m_dNFailuresSet[iRes].BuildReport ( sFailures );
			sFailures.MoveTo (tRes.m_sError);
			continue;
		}

		if ( tRes.m_dResults.IsEmpty () ) // fixup. It is easier to have single empty result, then check each time.
		{
			auto& tEmptyRes = tRes.m_dResults.Add ();
			tEmptyRes.m_tSchema = tRes.m_tSchema;
		}

		// minimize schema and remove dupes
		// assuming here ( tRes.m_tSchema==tRes.m_dSchemas[0] )
		const CSphFilterSettings * pAggrFilter = nullptr;
		if ( m_bMaster && !tQuery.m_tHaving.m_sAttrName.IsEmpty() )
			pAggrFilter = &tQuery.m_tHaving;

		const CSphVector<CSphQueryItem> & dItems = ( tQuery.m_dRefItems.GetLength() ? tQuery.m_dRefItems : tQuery.m_dItems );

		if ( tRes.m_iSuccesses>1 || dItems.GetLength() || pAggrFilter )
		{
			if ( m_bMaster && tRes.m_iSuccesses && dItems.GetLength() && tQuery.m_sGroupBy.IsEmpty() && tRes.GetLength()==0 )
			{
				for ( auto& dItem : dItems )
				{
					if ( dItem.m_sExpr=="count(*)" || ( dItem.m_sExpr=="@distinct" ) )
						tRes.m_dZeroCount.Add ( dItem.m_sAlias );
				}
			}

			bool bOk = MinimizeAggrResult ( tRes, tQuery, !m_dLocal.IsEmpty(), hExtra, m_pProfile, pAggrFilter, m_bFederatedUser, m_bMaster );

			if ( !bOk )
			{
				tRes.m_iSuccesses = 0;
				continue;
			}
		} else if ( !tRes.m_dResults.IsEmpty() )
		{
			tRes.m_tSchema = tRes.m_dResults.First ().m_tSchema;
			Debug ( tRes.m_bOneSchema = true; )
		}

		if ( !m_dNFailuresSet[iRes].IsEmpty() )
		{
			StringBuilder_c sFailures;
			m_dNFailuresSet[iRes].BuildReport ( sFailures );
			sFailures.MoveTo ( tRes.m_sWarning );
		}

		////////////
		// finalize
		////////////

		tRes.m_iOffset = Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
		auto iLimit = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );
		tRes.m_iCount = Max ( Min ( iLimit, tRes.GetLength()-tRes.m_iOffset ), 0 );
	}

	/////////////////////////////////
	// functions on a table argument
	/////////////////////////////////

	for ( int i=0; i<iQueries; ++i )
	{
		AggrResult_t & tRes = m_dNAggrResults[i];
		ISphTableFunc * pTableFunc = m_dTables[iStart+i];

		// FIXME! log such queries properly?
		if ( pTableFunc )
		{
			SwitchProfile ( m_pProfile, SPH_QSTATE_TABLE_FUNC );
			if ( !pTableFunc->Process ( &tRes, tRes.m_sError ) )
				tRes.m_iSuccesses = 0;
		}
	}

	/////////
	// stats
	/////////

	tmSubset += sphMicroTimer();
	tmCpu += sphTaskCpuTimer();

	CalcTimeStats ( tmCpu, tmSubset, dDistrServedByAgent );
	CalcPerIndexStats ( dDistrServedByAgent );
	CalcGlobalStats ( tmCpu, tmSubset, tmLocal, tIO, dRemotes );
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
	if ( session::GetVip () )
		return false;

	if ( g_iThdQueueMax!=0 )
		return GlobalWorkPool()->Works() > g_iThdQueueMax; // that is "jobs_queue_size" param of searchd conf, "work_queue_length" in 'show status', or "Queue:" in 'status'

	if ( g_iMaxConnection!=0 )
		return myinfo::CountClients() > g_iMaxConnection; // that is "max_connections" param of searchd.conf, "workers_clients" in 'show status', or "Clients:" in 'status'

	return false;
}

bool IsReadOnly ()
{
	return session::GetReadOnly();
}

bool sphCheckWeCanModify()
{
	return !IsReadOnly();
}

bool sphCheckWeCanModify ( StmtErrorReporter_i& tOut )
{
	if ( sphCheckWeCanModify() )
		return true;

	tOut.Error ( "connection is read-only");
	return false;
}

bool sphCheckWeCanModify ( const char* szStmt, RowBuffer_i& tOut )
{
	if ( sphCheckWeCanModify() )
		return true;

	tOut.Error ( szStmt, "connection is read-only" );
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

		std::unique_ptr<QueryParser_i> pParser;
		if ( eQueryType==QUERY_JSON )
			pParser = sphCreateJsonQueryParser();
		else
			pParser = sphCreatePlainQueryParser();

		assert ( pParser );
		tHandler.SetQueryParser ( std::move ( pParser ), eQueryType );

		const CSphQuery & q = tHandler.m_dQueries[0];
		myinfo::SetTaskInfo ( R"(api-search query="%s" comment="%s" index="%s")", q.m_sQuery.scstr (), q.m_sComment.scstr (), q.m_sIndexes.scstr () );
	}

	// run queries, send response
	tHandler.RunQueries();

	auto tReply = APIAnswer ( tOut, VER_COMMAND_SEARCH );
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
		SendResult ( uVer, tOut, tHandler.m_dAggrResults[i], bAgentMode, tHandler.m_dQueries[i], uMasterVer );

	int64_t iTotalPredictedTime = 0;
	int64_t iTotalAgentPredictedTime = 0;
	for ( const auto& dResult : tHandler.m_dAggrResults )
	{
		iTotalPredictedTime += dResult.m_iPredictedTime;
		iTotalAgentPredictedTime += dResult.m_iAgentPredictedTime;
	}

	auto & g_tStats = gStats ();
	g_tStats.m_iPredictedTime.fetch_add ( iTotalPredictedTime, std::memory_order_relaxed );
	g_tStats.m_iAgentPredictedTime.fetch_add ( iTotalAgentPredictedTime, std::memory_order_relaxed );

	ScWL_t dLastMetaLock ( g_tLastMetaLock );
	g_tLastMeta = tHandler.m_dAggrResults.Last();
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

class CSphTableFuncRemoveRepeats final : public ISphTableFunc
{
	CSphString	m_sCol;
	int			m_iOffset;
	int			m_iLimit;

public:
	bool ValidateArgs ( const StrVec_t & dArgs, const CSphQuery &, CSphString & sError ) final
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


	bool Process ( AggrResult_t * pResult, CSphString & sError ) final
	{
		assert ( pResult );

		assert ( pResult->m_bOneSchema );
		assert ( pResult->m_bSingle );
		assert ( !pResult->m_dResults.IsEmpty () );

		auto& dMatches = pResult->m_dResults.First().m_dMatches;
		if ( dMatches.IsEmpty() )
			return true;

		// get subset expressing 'LIMIT N,M'
		// LIMIT N,M clause must be applied before (!) table function
		// so we scan source matches N to N+M-1
		//
		// within those matches, we filter out repeats in a given column,
		// skip first m_iOffset eligible ones, and emit m_iLimit more
		auto dSubMatches = dMatches.Slice ( pResult->m_iOffset, pResult->m_iCount );
		if ( dSubMatches.IsEmpty() )
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
			: ( dSubMatches.First().GetAttr ( pCol->m_tLocator ) - 1 );

		int iOutPos = 0;
		for ( auto& dMatch : dSubMatches )
		{
			// get value, skip repeats
			SphAttr_t iCur = dMatch.GetAttr ( pCol->m_tLocator );
			if ( iCur==iLastValue )
				continue;

			if ( iCur && iLastValue && t==SPH_ATTR_STRINGPTR )
			{
				auto a = sphUnpackPtrAttr ((const BYTE *) iCur );
				auto b = sphUnpackPtrAttr ((const BYTE *) iLastValue );
				if ( a.second==b.second && memcmp ( a.first, b.first, a.second )==0 )
					continue;
			}

			iLastValue = iCur;

			// skip eligible rows according to tablefunc offset
			if ( m_iOffset>0 )
			{
				--m_iOffset;
				continue;
			}

			// emit!
			Swap ( dMatches[iOutPos], dMatch );

			// break if we reached the tablefunc limit
			if ( ++iOutPos==m_iLimit )
				break;
		}

		// adjust the result set limits
		pResult->ClampMatches ( iOutPos );
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
	"cluster_index_delete", "cluster_update", "explain", "import_table", "lock_indexes", "unlock_indexes",
	"show_settings"
};


STATIC_ASSERT ( sizeof(g_dSqlStmts)/sizeof(g_dSqlStmts[0])==STMT_TOTAL, STMT_DESC_SHOULD_BE_SAME_AS_STMT_TOTAL );

//////////////////////////////////////////////////////////////////////////

class CSphMatchVariant 
{
public:
	inline static SphAttr_t ToInt ( const SqlInsert_t & tVal )
	{
		switch ( tVal.m_iType )
		{
			case SqlInsert_t::QUOTED_STRING :	return strtoul ( tVal.m_sVal.cstr(), NULL, 10 ); // FIXME? report conversion error?
			case SqlInsert_t::CONST_INT:			return int(tVal.m_iVal);
			case SqlInsert_t::CONST_FLOAT:		return int(tVal.m_fVal); // FIXME? report conversion error
		}
		return 0;
	}

	inline static SphAttr_t ToBigInt ( const SqlInsert_t & tVal )
	{
		switch ( tVal.m_iType )
		{
			case SqlInsert_t::QUOTED_STRING :	return strtoll ( tVal.m_sVal.cstr(), NULL, 10 ); // FIXME? report conversion error?
			case SqlInsert_t::CONST_INT:			return tVal.m_iVal;
			case SqlInsert_t::CONST_FLOAT:		return int64_t(tVal.m_fVal); // FIXME? report conversion error?
		}
		return 0;
	}

	static bool ConvertPlainAttr ( const SqlInsert_t & tVal, ESphAttr eTargetType, SphAttr_t & tAttr )
	{
		tAttr = 0;

		switch ( eTargetType )
		{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TIMESTAMP:
		case SPH_ATTR_BOOL:
		case SPH_ATTR_TOKENCOUNT:
			tAttr = ToInt(tVal);
			break;

		case SPH_ATTR_BIGINT:
			tAttr = ToBigInt(tVal);
			break;

		case SPH_ATTR_FLOAT:
			if ( tVal.m_iType==SqlInsert_t::QUOTED_STRING )
				tAttr = sphF2DW ( (float)strtod ( tVal.m_sVal.cstr(), NULL ) ); // FIXME? report conversion error?
			else if ( tVal.m_iType==SqlInsert_t::CONST_INT )
				tAttr = sphF2DW ( float(tVal.m_iVal) ); // FIXME? report conversion error?
			else if ( tVal.m_iType==SqlInsert_t::CONST_FLOAT )
				tAttr = sphF2DW ( tVal.m_fVal );
			break;

		case SPH_ATTR_STRINGPTR:
			break;

		default:
			return false;
		};

		return true;
	}

	inline static void SetAttr ( CSphMatch & tMatch, const CSphAttrLocator & tLoc, const SqlInsert_t & tVal, ESphAttr eTargetType )
	{
		SphAttr_t tAttr;
		if ( ConvertPlainAttr ( tVal, eTargetType, tAttr ) )
			tMatch.SetAttr ( tLoc, tAttr );
	}

	inline static void SetDefaultAttr ( CSphMatch & tMatch, const CSphAttrLocator & tLoc, ESphAttr eTargetType )
	{
		SqlInsert_t tVal;
		tVal.m_iType = SqlInsert_t::CONST_INT;
		tVal.m_iVal = 0;
		SetAttr ( tMatch, tLoc, tVal, eTargetType );
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
	mutable std::atomic<int>				m_iWorker {0};

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
		iWorker = m_iWorker.fetch_add ( 1, std::memory_order_relaxed );
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

static VecRefPtrsAgentConn_t GetDistrAgents ( const cDistributedIndexRefPtr_t& pDist )
{
	assert ( pDist );
	VecRefPtrsAgentConn_t tRemotes;
	for ( const auto& pAgent : pDist->m_dAgents )
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

	std::unique_ptr<TextSource_i> pSource = CreateSnippetSource ( q.m_uFilesMode, (const BYTE*)tQuery.m_sSource.cstr(), tQuery.m_sSource.Length() );

	SnippetResult_t tRes;
	if ( !pBuilder->Build ( pSource, tRes ) )
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
	auto iJobs = dTasks.GetLength();
	if ( !iJobs )
		return;
	sphLogDebug ( "MakeSnippetsCoro invoked for %d tasks", iJobs );

	CSphVector<int> dStubFields;
	dStubFields.Add ( 0 );

	// the context
	ClonableCtx_T<SnippedBuilderCtxRef_t, SnippedBuilderCtxClone_t> dCtx { pBuilder };
	dCtx.LimitConcurrency ( GetEffectiveDistThreads () );

	std::atomic<int32_t> iCurJob { 0 };
	Coro::ExecuteN ( dCtx.Concurrency ( iJobs ), [&]
	{
		sphLogDebug ( "MakeSnippetsCoro Coro started" );
		auto iJob = iCurJob.fetch_add ( 1, std::memory_order_acq_rel );
		if ( iJob>=iJobs )
			return; // already nothing to do, early finish.

		auto tCtx = dCtx.CloneNewContext ();
		Threads::Coro::Throttler_c tThrottler ( session::GetThrottlingPeriodMS () );
		while (true)
		{
			myinfo::SetTaskInfo ( "s %d:", iJob );
			sphLogDebug ( "MakeSnippetsCoro Coro loop tick %d[%d]", iJob, dTasks[iJob] );
			MakeSingleLocalSnippetWithFields ( dQueries[dTasks[iJob]], q, tCtx.m_pBuilder, dStubFields );
			sphLogDebug ( "MakeSnippetsCoro Coro loop tick %d finished", iJob );

			iJob = iCurJob.fetch_add ( 1, std::memory_order_acq_rel );
			if ( iJob>=iJobs )
				return;

			// yield and reschedule every quant of time. It gives work to other tasks
			tThrottler.ThrottleAndKeepCrashQuery ();
		}
	});
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
		cDistributedIndexRefPtr_t pDist,
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
		cDistributedIndexRefPtr_t pDist,
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

	auto pServed = GetServed ( sIndex );
	if ( !pServed )
	{
		sError.SetSprintf ( "unknown local index '%s' in search request", sIndex.cstr() );
		return false;
	}

	RIdx_c pLocalIndex { pServed };
	assert ( pLocalIndex );

	///////////////////
	/// do highlighting
	///////////////////

	auto pBuilder = std::make_unique<SnippetBuilder_c>();
	pBuilder->Setup ( pLocalIndex, q );

	if ( !pBuilder->SetQuery ( q.m_sQuery.cstr(), true, sError ) )
		return false;

	// boring single snippet
	if ( dQueries.GetLength ()==1 )
		return MakeSingleLocalSnippet ( dQueries[0], q, pBuilder.get(), sError );

	if ( !CollectSourceSizes ( dQueries, q.m_uFilesMode, !bScattered, sError ) )
		return false;

	// set correct data size for snippets
	myinfo::SetTaskInfo ( R"(snippet datasize=%.1Dk query="%s")", GetSnippetDataSize ( dQueries ), q.m_sQuery.scstr () );

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
		MakeSnippetsCoro ( dPresent, dQueries, q, pBuilder.get() );

	} else
	{
		assert ( pDist );

		// multithreaded with remotes (scattered and full)
		if ( bScattered )
			MakeRemoteScatteredSnippets ( dQueries, pDist, pBuilder.get(), q, dPresent, dAbsent );
		else
			MakeRemoteNonScatteredSnippets ( dQueries, pDist, pBuilder.get (), q, dPresent );
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
	myinfo::SetTaskInfo ( R"(api-snippet datasize=%.1Dk query="%s")", GetSnippetDataSize ( dQueries ), q.m_sQuery.scstr ());

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

static void HandleCommandKeywords ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
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
	explicit UpdateRequestBuilder_c ( AttrUpdateSharedPtr_t pUpd ) : m_pUpd ( pUpd ) {}
	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer& tOut ) const final;

protected:
	AttrUpdateSharedPtr_t m_pUpd;
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
	assert ( m_pUpd->m_dAttributes.all_of ( [&] ( const TypedAttribute_t & tAttr ) { return ( tAttr.m_eType!=SPH_ATTR_INT64SET ); } ) );
	auto& tUpd = *m_pUpd;

	// API header
	auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_UPDATE, VER_COMMAND_UPDATE );

	tOut.SendString ( sIndexes );
	tOut.SendInt ( tUpd.m_dAttributes.GetLength() );
	tOut.SendInt ( tUpd.m_bIgnoreNonexistent ? 1 : 0 );
	for ( const auto & i : tUpd.m_dAttributes )
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

	tOut.SendInt ( tUpd.m_dDocids.GetLength() );

	ARRAY_FOREACH ( iDoc, tUpd.m_dDocids )
	{
		tOut.SendUint64 ( tUpd.m_dDocids[iDoc] );

		const DWORD* pPool = tUpd.m_dPool.Begin() + tUpd.GetRowOffset ( iDoc );
		for ( const auto & i : tUpd.m_dAttributes )
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
					tOut.SendBytes ( tUpd.m_dBlobs.Begin()+uVal, uBlobLen );
				}
				break;

			default:
				tOut.SendDword ( uVal );
				break;
			}
		}
	}
}

static void DoCommandUpdate ( const CSphString & sIndex, const CSphString& sCluster, const char * sDistributed, AttrUpdateSharedPtr_t pUpd,
	bool bBlobUpdate, int & iSuccesses, int & iUpdated, SearchFailuresLog_c & dFails )
{
	int iUpd = 0;
	CSphString sError, sWarning;
	RtAccum_t tAcc ( false );
	ReplicationCommand_t* pCmd = tAcc.AddCommand ( ReplicationCommand_e::UPDATE_API, sIndex, sCluster );
	assert ( pCmd );
	pCmd->m_pUpdateAPI = std::move(pUpd);
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
	AttrUpdateSharedPtr_t pUpd { new CSphAttrUpdate };
	CSphAttrUpdate& tUpd = *pUpd;
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
			auto eUpdate = (UpdateType_e)tReq.GetDword();
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
			DoCommandUpdate ( sReqIndex, pLocal->m_sCluster, nullptr, pUpd, bBlobUpdate, iSuccesses, iUpdated, dFails );

		} else if ( dDistributed[iIdx] )
		{
			auto * pDist = dDistributed[iIdx];

			assert ( !pDist->IsEmpty() );

			for ( const CSphString & sLocal : pDist->m_dLocal )
			{
				auto pServed = GetServed ( sLocal );
				if ( !pServed )
					continue;

				DoCommandUpdate ( sLocal, pServed->m_sCluster, sReqIndex.cstr(), pUpd, bBlobUpdate, iSuccesses, iUpdated, dFails );
			}

			// update remote agents
			if ( !dDistributed[iIdx]->m_dAgents.IsEmpty() )
			{
				VecRefPtrsAgentConn_t dAgents;
				pDist->GetAllHosts ( dAgents );

				// connect to remote agents and query them
				UpdateRequestBuilder_c tReqBuilder ( pUpd );
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

void BuildStatus ( VectorLike & dStatus )
{
	auto & g_tStats = gStats ();
	const char * OFF = "OFF";

	const int64_t iQueriesDiv = Max ( g_tStats.m_iQueries.load ( std::memory_order_relaxed ), 1 );
	const int64_t iDistQueriesDiv = Max ( g_tStats.m_iDistQueries.load ( std::memory_order_relaxed ), 1 );
	const int64_t iDiv1000 = iQueriesDiv * 1000;
	const int64_t iDDiv1000 = iDistQueriesDiv * 1000;

	dStatus.SetColName ( "Counter" );

	// FIXME? non-transactional!!!
	dStatus.MatchTupletf ( "uptime", "%u", (DWORD) time ( nullptr )-g_tStats.m_uStarted );
	dStatus.MatchTupletf ( "connections", "%l", g_tStats.m_iConnections.load ( std::memory_order_relaxed ) );
	dStatus.MatchTupletf ( "maxed_out", "%l", g_tStats.m_iMaxedOut.load ( std::memory_order_relaxed ) );
	dStatus.MatchTuplet ( "version" , g_sStatusVersion.cstr() );
	dStatus.MatchTuplet ( "mysql_version", g_sMySQLVersion.cstr() );

	for ( auto i=0; i<SEARCHD_COMMAND_TOTAL; ++i)
	{
		if ( i==SEARCHD_COMMAND_UNUSED_6 )
			continue;
		dStatus.MatchTupletf ( szCommand ( i ), "%l", g_tStats.m_iCommandCount[i].load ( std::memory_order_relaxed ) );
	}

	auto iConnects = g_tStats.m_iAgentConnectTFO.load ( std::memory_order_relaxed )
			+g_tStats.m_iAgentConnect.load ( std::memory_order_relaxed );
	dStatus.MatchTupletf ( "agent_connect", "%l", iConnects );
	dStatus.MatchTupletf ( "agent_tfo", "%l", g_tStats.m_iAgentConnectTFO.load ( std::memory_order_relaxed ) );
	dStatus.MatchTupletf ( "agent_retry", "%l", g_tStats.m_iAgentRetry.load ( std::memory_order_relaxed ) );
	dStatus.MatchTupletf ( "queries", "%l", g_tStats.m_iQueries.load ( std::memory_order_relaxed ) );
	dStatus.MatchTupletf ( "dist_queries", "%l", g_tStats.m_iDistQueries.load ( std::memory_order_relaxed ) );

	// status of thread pool
	dStatus.MatchTupletf ( "workers_total", "%d", GlobalWorkPool ()->WorkingThreads () );
	dStatus.MatchTupletf ( "workers_active", "%d", myinfo::CountTasks () );
	dStatus.MatchTupletf ( "workers_clients", "%d", myinfo::CountClients () );
	dStatus.MatchTupletf ( "workers_clients_vip", "%u", session::GetVips() );
	dStatus.MatchTupletf ( "work_queue_length", "%d", GlobalWorkPool ()->Works () );

	assert ( g_pDistIndexes );
	auto pDistSnapshot = g_pDistIndexes->GetHash();
	for ( auto& tIt : *pDistSnapshot )
	{
		const char * sIdx = tIt.first.cstr();
		const auto& dAgents = tIt.second->m_dAgents;
		StringBuilder_c sKey;
		ARRAY_FOREACH ( i, dAgents )
		{
			MultiAgentDescRefPtr_c pMultiAgent = dAgents[i];
			MultiAgentDesc_c& dMultiAgent = *pMultiAgent;
			ARRAY_FOREACH ( j, dMultiAgent )
			{
				const auto pMetrics = dMultiAgent[j].m_pMetrics;
				for ( int k = 0; k<eMaxAgentStat; ++k )
				{
					sKey.Clear();
					sKey.Sprintf ( "ag_%s_%d_%d_%s", sIdx, i+1, j+1, sAgentStatsNames[k] );
					dStatus.MatchTupletf ( sKey.cstr (), "%l", pMetrics->m_dCounters[k].load (std::memory_order_relaxed ) );
				}

				for ( int k = 0; k<ehMaxStat; ++k )
				{
					sKey.Clear ();
					sKey.Sprintf ( "ag_%s_%d_%d_%s", sIdx, i+1, j+1, sAgentStatsNames[eMaxAgentStat+k] );
					const char * sFmt = ( k==ehTotalMsecs || k==ehAverageMsecs || k==ehMaxMsecs ) ? "%0.3F" : "%l";
					dStatus.MatchTupletf ( sKey.cstr (), sFmt, pMetrics->m_dMetrics[k] );
				}
			}
		}
	}

	dStatus.MatchTupletf ( "query_wall", "%0.3F", g_tStats.m_iQueryTime.load ( std::memory_order_relaxed ) / 1000 );

	if ( g_bCpuStats )
		dStatus.MatchTupletf ( "query_cpu", "%0.3F", g_tStats.m_iQueryCpuTime.load ( std::memory_order_relaxed ) / 1000 );
	else
		dStatus.MatchTuplet ( "query_cpu", OFF);

	dStatus.MatchTupletf ( "dist_wall", "%0.3F", g_tStats.m_iDistWallTime.load ( std::memory_order_relaxed ) / 1000 );
	dStatus.MatchTupletf ( "dist_local", "%0.3F", g_tStats.m_iDistLocalTime.load ( std::memory_order_relaxed ) / 1000 );
	dStatus.MatchTupletf ( "dist_wait", "%0.3F", g_tStats.m_iDistWaitTime.load ( std::memory_order_relaxed ) / 1000 );

	if ( g_bIOStats )
	{
		dStatus.MatchTupletf ( "query_reads", "%l", g_tStats.m_iDiskReads.load ( std::memory_order_relaxed ) );
		dStatus.MatchTupletf ( "query_readkb", "%l", g_tStats.m_iDiskReadBytes.load ( std::memory_order_relaxed )/ 1024 );
		dStatus.MatchTupletf ( "query_readtime", "%l", g_tStats.m_iDiskReadTime.load ( std::memory_order_relaxed ) );
	} else
	{
		dStatus.MatchTuplet ( "query_reads", OFF );
		dStatus.MatchTuplet ( "query_readkb", OFF );
		dStatus.MatchTuplet ( "query_readtime", OFF );
	}

	if ( g_tStats.m_iPredictedTime.load ( std::memory_order_relaxed )
		|| g_tStats.m_iAgentPredictedTime.load ( std::memory_order_relaxed ) )
	{
		dStatus.MatchTupletf ( "predicted_time", "%l", g_tStats.m_iPredictedTime.load ( std::memory_order_relaxed ) );
		dStatus.MatchTupletf ( "dist_predicted_time", "%l", g_tStats.m_iAgentPredictedTime.load ( std::memory_order_relaxed ) );
	}

	dStatus.MatchTupletf ( "avg_query_wall", "%0.3F", g_tStats.m_iQueryTime.load ( std::memory_order_relaxed ) / iDiv1000 );

	if ( g_bCpuStats )
		dStatus.MatchTupletf ( "avg_query_cpu", "%0.3F", g_tStats.m_iQueryCpuTime.load ( std::memory_order_relaxed ) / iDiv1000 );
	else
		dStatus.MatchTuplet ( "avg_query_cpu", OFF );

	dStatus.MatchTupletf ( "avg_dist_wall", "%0.3F", g_tStats.m_iDistWallTime.load ( std::memory_order_relaxed ) / iDDiv1000 );
	dStatus.MatchTupletf ( "avg_dist_local", "%0.3F", g_tStats.m_iDistLocalTime.load ( std::memory_order_relaxed ) / iDDiv1000 );
	dStatus.MatchTupletf ( "avg_dist_wait", "%0.3F", g_tStats.m_iDistWaitTime.load ( std::memory_order_relaxed ) / iDDiv1000 );

	if ( g_bIOStats )
	{
		dStatus.MatchTupletf ( "avg_query_reads", "%0.1F", g_tStats.m_iDiskReads.load ( std::memory_order_relaxed ) * 10 / iQueriesDiv );
		dStatus.MatchTupletf ( "avg_query_readkb", "%0.1F", g_tStats.m_iDiskReadBytes.load ( std::memory_order_relaxed ) * 10 / (iQueriesDiv*1024) );
		dStatus.MatchTupletf ( "avg_query_readtime", "%0.3F", g_tStats.m_iDiskReadTime.load ( std::memory_order_relaxed ) / iDiv1000 );
	} else
	{
		dStatus.MatchTuplet ( "avg_query_reads", OFF );
		dStatus.MatchTuplet ( "avg_query_readkb", OFF );
		dStatus.MatchTuplet ( "avg_query_readtime", OFF );
	}

	const QcacheStatus_t & s = QcacheGetStatus();
	dStatus.MatchTupletf ( "qcache_max_bytes", "%l", s.m_iMaxBytes );
	dStatus.MatchTupletf ( "qcache_thresh_msec", "%d", s.m_iThreshMs );
	dStatus.MatchTupletf ( "qcache_ttl_sec", "%d", s.m_iTtlS );
	dStatus.MatchTupletf ( "qcache_cached_queries", "%d", s.m_iCachedQueries );
	dStatus.MatchTupletf ( "qcache_used_bytes", "%l", s.m_iUsedBytes );
	dStatus.MatchTupletf ( "qcache_hits", "%l", s.m_iHits );

	// clusters
	ReplicateClustersStatus ( dStatus );
}

// that is returned to MySQL 'statistic' command ('status' in mysql cli)
void BuildStatusOneline ( StringBuilder_c & sOut )
{
	auto iThreads = GlobalWorkPool ()->WorkingThreads ();
	auto iQueue = GlobalWorkPool ()->Works ();
	auto iTasks = myinfo::CountTasks ();
	auto & g_tStats = gStats ();
	sOut.StartBlock ( " " );
	sOut
	<< "Uptime:" << (DWORD) time ( NULL )-g_tStats.m_uStarted
	<< " Threads:" << iThreads
	<< " Queue:" << iQueue
	<< " Clients:" << myinfo::CountClients()
	<< " Vip clients:" << session::GetVips()
	<< " Tasks:" << iTasks
	<< " Queries:" << g_tStats.m_iQueries.load ( std::memory_order_relaxed );
	sOut.Sprintf ( " Wall: %t", (int64_t)g_tStats.m_iQueryTime.load ( std::memory_order_relaxed ) );
	sOut.Sprintf ( " CPU: %t", (int64_t)g_tStats.m_iQueryCpuTime.load ( std::memory_order_relaxed ) );
	sOut.Sprintf ( "\nQueue/Th: %0.1F%", iQueue * 10 / iThreads );
	sOut.Sprintf ( " Tasks/Th: %0.1F%", iTasks * 10 / iThreads );
}

void BuildOneAgentStatus ( VectorLike & dStatus, HostDashboardRefPtr_t pDash, const char * sPrefix="agent" )
{
	assert ( pDash );
	{
		ScRL_t tGuard ( pDash->m_dMetricsLock );
		if ( dStatus.MatchAddf ( "%s_hostname", sPrefix ) )
			dStatus.Add ( pDash->m_tHost.GetMyUrl ().cstr () );

		if ( dStatus.MatchAddf ( "%s_references", sPrefix ) )
			dStatus.Addf( "%d", (int) pDash->GetRefcount()-1 ); // -1 since we currently also 'use' the agent, reading it's stats
		if ( dStatus.MatchAddf ( "%s_ping", sPrefix ) )
			dStatus.Add ( pDash->m_iNeedPing ? "yes" : "no" );
		if ( dStatus.MatchAddf ( "%s_has_perspool", sPrefix ) )
			dStatus.Add ( pDash->m_pPersPool ? "yes" : "no" );
		if ( dStatus.MatchAddf ( "%s_need_resolve", sPrefix ) )
			dStatus.Add ( pDash->m_tHost.m_bNeedResolve ? "yes" : "no" );
		uint64_t iCur = sphMicroTimer();
		uint64_t iLastAccess = iCur - pDash->m_iLastQueryTime;
		if ( dStatus.MatchAddf ( "%s_lastquery", sPrefix ) )
			dStatus.Addf ( "%.2F", iLastAccess / 10000 );
		iLastAccess = iCur - pDash->m_iLastAnswerTime;
		if ( dStatus.MatchAddf ( "%s_lastanswer", sPrefix ) )
			dStatus.Addf ( "%.2F", iLastAccess / 10000 );
		uint64_t iLastTimer = pDash->m_iLastAnswerTime-pDash->m_iLastQueryTime;
		if ( dStatus.MatchAddf ( "%s_lastperiodmsec", sPrefix ) )
			dStatus.Addf ( "%.3D", iLastTimer );
		if ( dStatus.MatchAddf ( "%s_pingtripmsec", sPrefix ) )
			dStatus.Addf ( "%.3F", pDash->m_uPingTripUS );
		if ( dStatus.MatchAddf ( "%s_errorsarow", sPrefix ) )
			dStatus.Addf ( "%l", pDash->m_iErrorsARow );
	}
	int iPeriods = 1;

	while ( iPeriods>0 )
	{
		HostMetricsSnapshot_t dMetricsSnapshot;
		pDash->GetCollectedMetrics ( dMetricsSnapshot, iPeriods );
		{
			for ( int j = 0; j<ehMaxStat+eMaxAgentStat; ++j )
				// hack. Avoid microseconds in human-readable statistic
				if ( j==ehTotalMsecs && dStatus.MatchAddf ( "%s_%dperiods_msecsperqueryy", sPrefix, iPeriods ) )
				{
					if ( dMetricsSnapshot[ehConnTries]>0 )
						dStatus.Addf ( "%.2F", dMetricsSnapshot[ehTotalMsecs] / dMetricsSnapshot[ehConnTries] / 10 );
					else
						dStatus.Add ( "n/a" );
				} else if ( dStatus.MatchAddf ( "%s_%dperiods_%s", sPrefix, iPeriods, sAgentStatsNames[j] ) )
				{
					if ( j==ehMaxMsecs || j==ehAverageMsecs )
						dStatus.Addf ( "%.2F", dMetricsSnapshot[j] / 10 );
					else
						dStatus.Addf ( "%l", dMetricsSnapshot[j] );
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
		if ( dStatus.MatchAddf ( "dstindex_local_%d", i+1 ) )
			dStatus.Add ( pDistr->m_dLocal[i].cstr() );
	}

	CSphString sKey;
	ARRAY_FOREACH ( i, pDistr->m_dAgents )
	{
		MultiAgentDescRefPtr_c pAgents = pDistr->m_dAgents[i];
		const MultiAgentDesc_c& tAgents = *pAgents;
		if ( dStatus.MatchAddf ( "dstindex_%d_is_ha", i+1 ) )
			dStatus.Add ( tAgents.IsHA()? "1": "0" );

		auto dWeights = tAgents.GetWeights ();

		ARRAY_FOREACH ( j, tAgents )
		{
			if ( tAgents.IsHA() )
				sKey.SetSprintf ( "dstindex_%dmirror%d", i+1, j+1 );
			else
				sKey.SetSprintf ( "dstindex_%dagent", i+1 );

			const AgentDesc_t & dDesc = tAgents[j];

			if ( dStatus.MatchAddf ( "%s_id", sKey.cstr () ) )
				dStatus.Addf ( "%s:%s", dDesc.GetMyUrl ().cstr (), dDesc.m_sIndexes.cstr () );

			if ( tAgents.IsHA() && dStatus.MatchAddf ( "%s_probability_weight", sKey.cstr () ) )
				dStatus.Addf ( "%0.2f%%", dWeights[j] );

			if ( dStatus.MatchAddf ( "%s_is_blackhole", sKey.cstr () ) )
				dStatus.Add ( dDesc.m_bBlackhole ? "1" : "0" );

			if ( dStatus.MatchAddf ( "%s_is_persistent", sKey.cstr () ) )
				dStatus.Add ( dDesc.m_bPersistent ? "1" : "0" );
		}
	}
	return true;
}


static bool operator < ( const IteratorDesc_t & tA, const IteratorDesc_t & tB )
{
	if ( tA.m_sAttr < tB.m_sAttr )
		return true;

	return tA.m_sType<tB.m_sType;
}


static bool operator == ( const IteratorDesc_t & tA, const IteratorDesc_t & tB )
{
	return tA.m_sAttr==tB.m_sAttr && tA.m_sType==tB.m_sType;
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
			else
				dStatus.MatchTupletf ( "status_error", "No such distributed index or agent: %s", sIndexOrAgent.cstr () );
		}
		return;
	}

	dStatus.SetColName ( "Key" );

	dStatus.MatchTupletf( "status_period_seconds", "%d", g_uHAPeriodKarmaS );
	dStatus.MatchTupletf ( "status_stored_periods", "%d", STATS_DASH_PERIODS );

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
	if ( dStatus.MatchAddf ( "%s%s", sPrefix, "io_read_time" ) )
		dStatus.Addf( "%.3F", tStats.m_iReadTime);

	if ( dStatus.MatchAddf ( "%s%s", sPrefix, "io_read_ops" ) )
		dStatus.Addf ( "%u", tStats.m_iReadOps );

	if ( dStatus.MatchAddf ( "%s%s", sPrefix, "io_read_kbytes" ) )
		dStatus.Addf ( "%d.%d", (int) ( tStats.m_iReadBytes / 1024 ), (int) ( tStats.m_iReadBytes % 1024 ) / 100 );

	if ( dStatus.MatchAddf ( "%s%s", sPrefix, "io_write_time" ) )
		dStatus.Addf ( "%.3F", tStats.m_iWriteTime );

	if ( dStatus.MatchAddf ( "%s%s", sPrefix, "io_write_ops" ) )
		dStatus.Addf ( "%u", tStats.m_iWriteOps );

	if ( dStatus.MatchAddf ( "%s%s", sPrefix, "io_write_kbytes" ) )
		dStatus.Addf ( "%d.%d", (int)( tStats.m_iWriteBytes/1024 ), (int)( tStats.m_iWriteBytes%1024 )/100 );
}

void BuildMeta ( VectorLike & dStatus, const CSphQueryResultMeta & tMeta )
{
	if ( !tMeta.m_sError.IsEmpty() )
		dStatus.MatchTuplet ( "error", tMeta.m_sError.cstr () );

	if ( !tMeta.m_sWarning.IsEmpty() )
		dStatus.MatchTuplet ( "warning", tMeta.m_sWarning.cstr () );

	dStatus.MatchTupletf ( "total", "%d", tMeta.m_iMatches );
	dStatus.MatchTupletf ( "total_found", "%l", tMeta.m_iTotalMatches );
	dStatus.MatchTupletf ( "total_relation", "%s", tMeta.m_bTotalMatchesApprox ? "gte" : "eq" );
	dStatus.MatchTupletf ( "time", "%.3F", tMeta.m_iQueryTime );

	if ( tMeta.m_iMultiplier>1 )
		dStatus.MatchTupletf ( "multiplier", "%d", tMeta.m_iMultiplier );

	if ( g_bCpuStats )
	{
		dStatus.MatchTupletf ( "cpu_time", "%.3F", tMeta.m_iCpuTime );
		dStatus.MatchTupletf ( "agents_cpu_time", "%.3F", tMeta.m_iAgentCpuTime );
	}

	if ( g_bIOStats )
	{
		AddIOStatsToMeta ( dStatus, tMeta.m_tIOStats, "" );
		AddIOStatsToMeta ( dStatus, tMeta.m_tAgentIOStats, "agent_" );
	}

	if ( tMeta.m_bHasPrediction )
	{
		dStatus.MatchTupletf ( "local_fetched_docs", "%d", tMeta.m_tStats.m_iFetchedDocs );
		dStatus.MatchTupletf ( "local_fetched_hits", "%d", tMeta.m_tStats.m_iFetchedHits );
		dStatus.MatchTupletf ( "local_fetched_skips", "%d", tMeta.m_tStats.m_iSkips );

		dStatus.MatchTupletf ( "predicted_time", "%l", tMeta.m_iPredictedTime );
		if ( tMeta.m_iAgentPredictedTime )
			dStatus.MatchTupletf ( "dist_predicted_time", "%l", tMeta.m_iAgentPredictedTime );
		if ( tMeta.m_iAgentFetchedDocs || tMeta.m_iAgentFetchedHits || tMeta.m_iAgentFetchedSkips )
		{
			dStatus.MatchTupletf ( "dist_fetched_docs", "%d", tMeta.m_tStats.m_iFetchedDocs+tMeta.m_iAgentFetchedDocs );
			dStatus.MatchTupletf ( "dist_fetched_hits", "%d", tMeta.m_tStats.m_iFetchedHits+tMeta.m_iAgentFetchedHits );
			dStatus.MatchTupletf ( "dist_fetched_skips", "%d", tMeta.m_tStats.m_iSkips+tMeta.m_iAgentFetchedSkips );
		}
	}

	auto dWords = tMeta.MakeSortedWordStat();
	ARRAY_CONSTFOREACH( iWord, dWords )
	{
		auto * pWord = dWords[iWord];
		assert ( pWord );
		if ( dStatus.MatchAddf ( "keyword[%d]", iWord ) )
			dStatus.Add ( pWord->first );

		if ( dStatus.MatchAddf ( "docs[%d]", iWord ) )
			dStatus.Addf ( "%l", pWord->second.first );

		if ( dStatus.MatchAddf ( "hits[%d]", iWord ) )
			dStatus.Addf ( "%l", pWord->second.second );
	}

	StringBuilder_c sIterators { ", " };
	for ( const auto & i : tMeta.m_tIteratorStats.m_dIterators )
		sIterators.Appendf ( "%s:%s (%d%%)", i.m_sAttr.cstr(), i.m_sType.cstr(), int(float(i.m_iUsed)/tMeta.m_tIteratorStats.m_iTotal*100.0f) );

	if ( !sIterators.IsEmpty() )
		dStatus.MatchTuplet ( "index", sIterators.cstr() );
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
		auto & g_tStats = gStats ();
		BuildMeta ( dStatus, g_tLastMeta );
		if ( g_tStats.m_iPredictedTime.load ( std::memory_order_relaxed )
			|| g_tStats.m_iAgentPredictedTime.load ( std::memory_order_relaxed ) )
		{
			dStatus.MatchTupletf ( "predicted_time", "%l", g_tStats.m_iPredictedTime.load ( std::memory_order_relaxed ) );
			dStatus.MatchTupletf ( "dist_predicted_time", "%l", g_tStats.m_iAgentPredictedTime.load ( std::memory_order_relaxed ) );
		}
	}

	auto tReply = APIAnswer ( tOut, VER_COMMAND_STATUS );
	tOut.SendInt ( dStatus.GetLength () / dStatus.Header ().GetLength () ); // rows
	tOut.SendInt ( dStatus.Header ().GetLength () ); // cols
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


void ExecuteApiCommand ( SearchdCommand_e eCommand, WORD uCommandVer, int iLength, InputBuffer_c & tBuf, ISphOutputBuffer & tOut )
{
	auto & tSess = session::Info();
	tSess.SetTaskState ( TaskState_e::QUERY );
	// set on query guard
	auto& tCrashQuery = GlobalCrashQueryGetRef();
	tCrashQuery.m_eType = QUERY_API;
	tCrashQuery.m_dQuery = { tBuf.GetBufferPtr(), iLength };
	tCrashQuery.m_uCMD = eCommand;
	tCrashQuery.m_uVer = uCommandVer;

	// handle known commands
	assert ( eCommand<SEARCHD_COMMAND_WRONG );

	// count commands
	StatCountCommand ( eCommand );
	myinfo::SetCommand ( g_dApiCommands[eCommand] );

	sphLogDebugv ( "conn %s(%d): got command %d, handling", tSess.szClientName(), tSess.GetConnID(), eCommand );
	switch ( eCommand )
	{
		case SEARCHD_COMMAND_SEARCH:	HandleCommandSearch ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_EXCERPT:	HandleCommandExcerpt ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_KEYWORDS:	HandleCommandKeywords ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_UPDATE:	HandleCommandUpdate ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_STATUS:	HandleCommandStatus ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_FLUSHATTRS:HandleCommandFlush ( tOut, uCommandVer ); break;
		case SEARCHD_COMMAND_SPHINXQL:	HandleCommandSphinxql ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_JSON:		HandleCommandJson ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_PING:		HandleCommandPing ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_UVAR:		HandleCommandUserVar ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_CALLPQ:	HandleCommandCallPq ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_CLUSTERPQ:	HandleCommandClusterPq ( tOut, uCommandVer, tBuf, tSess.szClientName () ); break;
		case SEARCHD_COMMAND_GETFIELD:	HandleCommandGetField ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_PERSIST: break; // already processes, here just for stat
		default:						assert ( 0 && "internal error: unhandled command" ); break;
	}
}


void StmtErrorReporter_i::Error ( const char * sTemplate, ... )
{
	StringBuilder_c sBuf;

	va_list ap;
	va_start ( ap, sTemplate );
	sBuf.vAppendf ( sTemplate, ap );
	va_end ( ap );

	ErrorEx ( MYSQL_ERR_PARSE_ERROR, sBuf.cstr () );
}

class StmtErrorReporter_c final : public StmtErrorReporter_i
{
public:
	explicit StmtErrorReporter_c ( RowBuffer_i & tBuffer, const char* szStmt = nullptr )
		: m_tRowBuffer ( tBuffer )
		, m_szStmt ( szStmt )
	{}

	void Ok ( int iAffectedRows, const CSphString & sWarning, int64_t iLastInsertId ) final
	{
		m_tRowBuffer.Ok ( iAffectedRows, ( sWarning.IsEmpty() ? 0 : 1 ), nullptr, false, iLastInsertId );
	}

	void Ok ( int iAffectedRows, int nWarnings ) final
	{
		m_tRowBuffer.Ok ( iAffectedRows, nWarnings );
	}

	void ErrorEx ( MysqlErrors_e iErr, const char * sError ) final
	{
		m_tRowBuffer.Error ( m_szStmt, sError, iErr );
	}

	RowBuffer_i * GetBuffer() final { return &m_tRowBuffer; }

private:
	RowBuffer_i & m_tRowBuffer;
	const char * m_szStmt;
};


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
		pPacked += ZipToPtrBE ( pPacked, iBlobSize );
		return pPacked;
	}
};

static void BsonToSqlInsert ( const bson::Bson_c& dBson, SqlInsert_t& tAttr )
{
	switch ( dBson.GetType () )
	{
	case JSON_INT32:
	case JSON_INT64: tAttr.m_iType = SqlInsert_t::CONST_INT;
		tAttr.m_iVal = dBson.Int ();
		break;
	case JSON_DOUBLE: tAttr.m_iType = SqlInsert_t::CONST_FLOAT;
		tAttr.m_fVal = float ( dBson.Double () );
		break;
	case JSON_STRING: tAttr.m_iType = SqlInsert_t::QUOTED_STRING;
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


static bool ParseBsonDocument ( const VecTraits_T<BYTE> & dDoc, const SchemaItemHash_c & tLoc, const CSphString & sIdAlias, int iRow, VecTraits_T<VecTraits_T<const char>>& dFields, CSphMatch & tDoc,
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
				CSphMatchVariant::SetAttr ( tDoc, pItem->m_tLoc, tAttr, pItem->m_eType );
				if ( pId==pItem )
					tDoc.SetAttr ( tIdLoc, (DocID_t)dChild.Int() );

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
	mutable std::atomic<int> m_iWorker {0};
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
			iWorker = m_iWorker.fetch_add ( 1, std::memory_order_relaxed );
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

		if ( !tAgent.m_pResult )
			tAgent.m_pResult = std::make_unique<CPqResult>();

		auto pResult = (CPqResult*)tAgent.m_pResult.get();
		auto &dResult = pResult->m_dResult;
		auto uFlags = tReq.GetDword ();
		bool bDumpDocs = !!(uFlags & 1U);
		bool bQuery = !!(uFlags & 2U);
		bool bDeduplicatedDocs = !!(uFlags & 4U);

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
				if ( uDescFlags & 1U )
					tDesc.m_sQuery = tReq.GetString ();
				if ( uDescFlags & 2U )
					tDesc.m_sTags = tReq.GetString ();
				if ( uDescFlags & 4U )
					tDesc.m_sFilters = tReq.GetString ();
				tDesc.m_bQL = !!(uDescFlags & 8U);
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
			if ( !tDesc.m_sTags.IsEmpty () )
				uDescFlags |= 2;
			if ( !tDesc.m_sFilters.IsEmpty () )
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

	tOut.HeadColumn ( "id", MYSQL_COL_LONGLONG );
	if ( bDumpDocs )
		tOut.HeadColumn ( "documents" );
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

			tOut.PutString ( sDocs );
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
static void PQLocalMatch ( const BlobVec_t & dDocs, const CSphString & sIndex, const PercolateOptions_t & tOpt,	CSphSessionAccum & tAcc, CPqResult & tResult, int iStart, int iDocs )
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

	auto pServed = GetServed ( sIndex );
	if ( !pServed )
	{
		sMsg.Err ( "unknown local index '%s' in search request", sIndex.cstr () );
		return;
	}

	if ( pServed->m_eType!=IndexType_e::PERCOLATE )
	{
		sMsg.Err ( "index '%s' is not percolate", sIndex.cstr () );
		return;
	}

	RIdx_T<PercolateIndex_i*> pIndex { pServed };
	RtAccum_t * pAccum = tAcc.GetAcc ( pIndex, sError );
	sMsg.Err ( sError );

	if ( !sMsg.ErrEmpty () )
		return;

	const CSphSchema & tSchema = pIndex->GetInternalSchema();
	int iFieldsCount = tSchema.GetFieldsCount();

	InsertDocData_t tDoc(tSchema);

	// set defaults
	int iAttrsCount = tSchema.GetAttrsCount ();
	for ( int i = 0; i<iAttrsCount; ++i )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);
		CSphAttrLocator tLoc = tCol.m_tLocator;
		tLoc.m_bDynamic = true;
		CSphMatchVariant::SetDefaultAttr ( tDoc.m_tDoc, tLoc, tCol.m_eAttrType );
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

	tDoc.m_dStrings.Resize(iStrCounter);
	StringPtrTraits_t tStrings;
	tStrings.m_dOff.Reset ( iStrCounter );
	CSphVector<int64_t> dMvaParsed ( iMvaCounter );

	CSphString sTokenFilterOpts;
	RowID_t tRowID = 0;
	for ( auto iDoc = iStart; iDoc<iStart+iDocs; ++iDoc )
	{
		// doc-id
		tDoc.m_tDoc.SetAttr ( tIdLoc, 0 );
		tDoc.m_dFields[0] = dDocs[iDoc];

		dMvaParsed.Resize ( iMvaCounter );
		dMvaParsed.Fill ( 0 );

		if ( tOpt.m_bJsonDocs )
		{
			// reset all back to defaults
			tDoc.m_dFields.Fill ( { nullptr, 0 } );
			for ( int i = 0; i<iAttrsCount; ++i )
			{
				const CSphColumnInfo &tCol = tSchema.GetAttr ( i );
				CSphAttrLocator tLoc = tCol.m_tLocator;
				tLoc.m_bDynamic = true;
				CSphMatchVariant::SetDefaultAttr ( tDoc.m_tDoc, tLoc, tCol.m_eAttrType );
			}

			tStrings.Reset();

			if ( !ParseBsonDocument ( dDocs[iDoc], hSchemaLocators, tOpt.m_sIdAlias, iDoc, tDoc.m_dFields, tDoc.m_tDoc, tStrings, dMvaParsed, tSchema, sMsg ) )
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

			tStrings.SavePointersTo ( tDoc.m_dStrings, false );

			// convert back offset into tStrings buffer into pointers
			for ( VecTraits_T<const char> & tField : tDoc.m_dFields )
			{
				if ( !tField.GetLength() )
					continue;

				int64_t iOff = int64_t( tField.Begin() );
				int iLen = tField.GetLength();
				tField = VecTraits_T<const char> ( (const char *)( tStrings.m_dPackedData.Begin()+iOff ), iLen );
			}
		}

		FixParsedMva ( dMvaParsed, tDoc.m_dMvas, iMvaCounter );

		if ( !sMsg.ErrEmpty () )
			break;


		tDoc.m_tDoc.m_tRowID = ( RowID_t ) tRowID++;

		if ( !bAutoId )
		{
			// in user-provides-id mode let's skip all docs without id
			if ( !sphGetDocID ( tDoc.m_tDoc.m_pDynamic ) )
			{
				++iDocsNoIdCount;
				continue;
			}

			// store provided doc-id for result set sending
			tResult.m_dDocids[uSeqDocid] = ( int64_t ) sphGetDocID ( tDoc.m_tDoc.m_pDynamic );
			tDoc.m_tDoc.SetAttr ( tIdLoc, uSeqDocid++ );
		} else
			tDoc.m_tDoc.SetAttr ( tIdLoc, iDoc + 1 ); // +1 since docid is 1-based

		// PQ work with sequential document numbers, 0 element unused

		// add document
		pIndex->AddDocument ( tDoc, true, sTokenFilterOpts, sError, sWarning, pAccum );
		sMsg.Err ( sError );
		sMsg.Warn ( sWarning );

		if ( !sMsg.ErrEmpty() )
			break;
	}

	// fire exit
	if ( !sMsg.ErrEmpty() )
	{
		pIndex->RollBack ( pAccum ); // clean up collected data
		return;
	}

	pIndex->MatchDocuments ( pAccum, tResult.m_dResult );

	if ( iDocsNoIdCount )
		sMsg.Warn ( "skipped %d document(s) without id field '%s'", iDocsNoIdCount, tOpt.m_sIdAlias.cstr() );
}


void PercolateMatchDocuments ( const BlobVec_t & dDocs, const PercolateOptions_t & tOpts, CSphSessionAccum & tAcc, CPqResult & tResult )
{
	CSphString sIndex = tOpts.m_sIndex;
	CSphString sWarning, sError;

	StrVec_t dLocalIndexes;
	const auto * pLocalIndexes = &dLocalIndexes;

	VecRefPtrsAgentConn_t dAgents;
	auto pDist = GetDistr ( sIndex );
	if ( pDist )
	{
		for ( const auto& pAgent : pDist->m_dAgents )
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
	std::unique_ptr<PqRequestBuilder_c> pReqBuilder;
	std::unique_ptr<ReplyParser_i> pParser;
	CSphRefcountedPtr<RemoteAgentsObserver_i> pReporter { nullptr };
	if ( bHaveRemotes )
	{
		pReqBuilder = std::make_unique<PqRequestBuilder_c> ( dDocs, tOpts, iStart, iStep );
		iStart += iStep * dAgents.GetLength ();
		pParser = std::make_unique<PqReplyParser_c>();
		pReporter = GetObserver();
		ScheduleDistrJobs ( dAgents, pReqBuilder.get(), pParser.get(), pReporter );
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

			auto pResult = ( CPqResult * ) pAgent->m_pResult.get ();
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
	StatCountCommand ( SEARCHD_COMMAND_CALLPQ );
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

	if ( dStmtIndex.m_iType!=SqlInsert_t::QUOTED_STRING )
	{
		tOut.Error ( tStmt.m_sStmt, "PQ() argument 1 must be a string" );
		return;
	}
	if ( dStmtDocs.m_iType!=SqlInsert_t::QUOTED_STRING && dStmtDocs.m_iType!=SqlInsert_t::CONST_STRINGS )
	{
		tOut.Error ( tStmt.m_sStmt, "PQ() argument 2 must be a string or a string list" );
		return;
	}

	// document(s)
	StrVec_t dDocs;
	if ( dStmtDocs.m_iType==SqlInsert_t::QUOTED_STRING )
		dDocs.Add ( dStmtDocs.m_sVal );
	else
		dDocs.SwapData ( tStmt.m_dCallStrings );

	// options last
	CSphString sError;
	PercolateOptions_t tOpts;
	tOpts.m_sIndex = dStmtIndex.m_sVal;
	SqlParser_SplitClusterIndex ( tOpts.m_sIndex, nullptr );
	bool bSkipEmpty = false;
	ARRAY_FOREACH ( i, tStmt.m_dCallOptNames )
	{
		CSphString & sOpt = tStmt.m_dCallOptNames[i];
		const SqlInsert_t & v = tStmt.m_dCallOptValues[i];

		sOpt.ToLower();
		int iExpType = SqlInsert_t::CONST_INT;

		if ( sOpt=="docs_id" )
		{
			tOpts.m_sIdAlias = v.m_sVal;
			iExpType = SqlInsert_t::QUOTED_STRING;
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
			iExpType = SqlInsert_t::QUOTED_STRING;
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


static void PopulateMapsFromIndexSchema ( CSphVector<int> & dAttrSchema, CSphVector<int> & dFieldSchema, const CSphSchema & tSchema )
{
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
}


static bool CreateAttrMaps ( CSphVector<int> & dAttrSchema, CSphVector<int> & dFieldSchema, CSphVector<bool> & dFieldAttrs, const CSphSchema & tSchema, const StrVec_t & dStmtInsertSchema, StmtErrorReporter_i & tOut )
{
	ARRAY_FOREACH ( i, dFieldAttrs )
		dFieldAttrs[i] = false;

	if ( !dStmtInsertSchema.GetLength() )
	{
		PopulateMapsFromIndexSchema ( dAttrSchema, dFieldSchema, tSchema );
		return true;
	}

	// got a list of columns, check for 1) existance, 2) dupes
	StrVec_t dCheck = dStmtInsertSchema;
	ARRAY_FOREACH ( i, dCheck )
		// OPTIMIZE! GetFieldIndex use linear searching. M.b. hash instead?
		if ( tSchema.GetAttrIndex ( dCheck[i].cstr() )==-1 && tSchema.GetFieldIndex ( dCheck[i].cstr() )==-1 )
		{
			tOut.Error ( "unknown column: '%s'", dCheck[i].cstr() );
			return false;
		}

	dCheck.Sort();
	for ( int i=1; i<dCheck.GetLength(); i++ )
		if ( dCheck[i-1]==dCheck[i] )
		{
			CSphString sError;
			sError.SetSprintf ( "column '%s' specified twice", dCheck[i].cstr() );
			tOut.ErrorEx ( MYSQL_ERR_FIELD_SPECIFIED_TWICE, sError.cstr() );
			return false;
		}

	// hash column list
	// OPTIMIZE! hash index columns once (!) instead
	SmallStringHash_T<int> dInsertSchema;
	ARRAY_FOREACH ( i, dStmtInsertSchema )
		dInsertSchema.Add ( i, dStmtInsertSchema[i] );

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

	return true;
}

/////////////////////////////////////////////////////////////////////

class AttributeConverter_c : public InsertDocData_t
{
public:
				AttributeConverter_c ( const CSphSchema & tSchema, const CSphVector<bool> & dFieldAttrs, CSphString & sError, CSphString & sWarning );

	bool		SetAttrValue ( int iCol, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx );
	void		SetDefaultAttrValue ( int iCol );

	bool		SetFieldValue ( int iField, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx );
	void		SetDefaultFieldValue ( int iField );

	void		NewRow();
	void		Finalize();

private:
	const CSphSchema &		m_tSchema;
	const CSphColumnInfo *	m_pDocId = nullptr;

	const CSphVector<bool> & m_dFieldAttrs;

	StringPtrTraits_t 		m_tStrings;
	StrVec_t				m_dTmpFieldStorage;
	CSphVector<int>			m_dColumnarRemap;

	CSphString &			m_sError;
	CSphString &			m_sWarning;

	bool		String2JsonPack ( char * pStr, CSphVector<BYTE> & dBuf );

	bool		CheckDocId ( const CSphColumnInfo & tCol, SphAttr_t tAttr );
	bool		CheckStrings ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iCol, int iRow );
	bool		CheckJson ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal );
	bool		CheckInsertTypes ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx );
};


AttributeConverter_c::AttributeConverter_c ( const CSphSchema & tSchema, const CSphVector<bool> & dFieldAttrs, CSphString & sError, CSphString & sWarning )
	: InsertDocData_t ( tSchema )
	, m_tSchema ( tSchema )
	, m_pDocId ( tSchema.GetAttr ( sphGetDocidName() ) )
	, m_dFieldAttrs ( dFieldAttrs )
	, m_sError ( sError )
	, m_sWarning ( sWarning )
{
	int iAttrs = tSchema.GetAttrsCount();

	m_dTmpFieldStorage.Resize ( tSchema.GetFieldsCount() );
	m_dColumnarRemap.Resize(iAttrs);

	int iColumnarAttr = 0;
	for ( int i = 0; i < iAttrs; i++ )
		if ( m_tSchema.GetAttr(i).IsColumnar() )
			m_dColumnarRemap[i] = iColumnarAttr++;
		else
			m_dColumnarRemap[i] = -1;

	m_dColumnarAttrs.Resize(iColumnarAttr);
	m_tStrings.m_dOff.Reset(iAttrs);
}


bool AttributeConverter_c::String2JsonPack ( char * pStr, CSphVector<BYTE> & dBuf )
{
	dBuf.Resize ( 0 ); // buffer for JSON parser must be empty to properly set JSON_ROOT data
	if ( !pStr )
		return true;

	if ( !sphJsonParse ( dBuf, pStr, g_bJsonAutoconvNumbers, g_bJsonKeynamesToLowercase, true, m_sError ) )
	{
		if ( g_bJsonStrict )
			return false;

		if ( m_sWarning.IsEmpty() )
			m_sWarning = m_sError;
		else
			m_sWarning.SetSprintf ( "%s; %s", m_sWarning.cstr(), m_sError.cstr() );

		m_sError = "";
	}

	return true;
}


bool AttributeConverter_c::CheckDocId ( const CSphColumnInfo & tCol, SphAttr_t tAttr )
{
	if ( &tCol==m_pDocId && tAttr<0 )
	{
		m_sError.SetSprintf ( "'id' column is " INT64_FMT ". Must be positive.", tAttr );
		return false;
	}

	return true;
}


bool AttributeConverter_c::CheckStrings ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iCol, int iRow )
{
	if ( tCol.m_eAttrType!=SPH_ATTR_STRING && tCol.m_eAttrType!=SPH_ATTR_STRINGPTR )
		return true;

	if ( tVal.m_sVal.Length() > 0x3FFFFF )
	{
		*( char * ) ( tVal.m_sVal.cstr () + 0x3FFFFF ) = '\0';
		m_sWarning.SetSprintf ( "String column %d at row %d too long, truncated to 4MB", iCol, iRow );
	}

	m_dStrings.Add ( tVal.m_sVal.cstr() );

	return true;
}


bool AttributeConverter_c::CheckJson ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal )
{
	if ( tCol.m_eAttrType!=SPH_ATTR_JSON )
		return true;

	int iStrCount = m_dStrings.GetLength();
	m_dStrings.Add ( nullptr );

	// empty source string means NULL attribute
	if ( tVal.m_sVal.IsEmpty() )
		return true;

	// sphJsonParse must be terminated with a double zero however usual CSphString have SAFETY_GAP of 4 zeros
	if ( !String2JsonPack ( (char *)tVal.m_sVal.cstr(), m_tStrings.m_dParserBuf ) )
		return false;

	int iParsedLength = m_tStrings.m_dParserBuf.GetLength();
	if ( iParsedLength )
	{
		m_tStrings.m_dOff[iStrCount] = m_tStrings.m_dPackedData.GetLength();
		BYTE * pPacked = m_tStrings.m_dPackedData.AddN ( sphCalcPackedLength ( iParsedLength ) );
		sphPackPtrAttr ( pPacked, m_tStrings.m_dParserBuf );
	}

	return true;
}


bool AttributeConverter_c::CheckInsertTypes ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx )
{
	if ( tVal.m_iType!=SqlInsert_t::QUOTED_STRING
		&& tVal.m_iType!=SqlInsert_t::CONST_INT
		&& tVal.m_iType!=SqlInsert_t::CONST_FLOAT
		&& tVal.m_iType!=SqlInsert_t::CONST_MVA )
	{
		m_sError.SetSprintf ( "row %d, column %d: internal error: unknown insval type %d", 1+iRow, 1+iQuerySchemaIdx, tVal.m_iType ); // 1 for human base
		return false;
	}

	if ( tVal.m_iType==SqlInsert_t::CONST_MVA && !( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET ) )
	{
		m_sError.SetSprintf ( "row %d, column %d: MVA value specified for a non-MVA column", 1+iRow, 1+iQuerySchemaIdx ); // 1 for human base
		return false;
	}

	if ( ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET ) && tVal.m_iType!=SqlInsert_t::CONST_MVA )
	{
		m_sError.SetSprintf ( "row %d, column %d: non-MVA value specified for a MVA column", 1+iRow, 1+iQuerySchemaIdx ); // 1 for human base
		return false;
	}

	return true;
}


void AttributeConverter_c::SetDefaultAttrValue ( int iCol )
{
	const CSphColumnInfo & tCol = m_tSchema.GetAttr(iCol);
	CSphAttrLocator tLoc = tCol.m_tLocator;
	tLoc.m_bDynamic = true;

	if ( tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_STRINGPTR || tCol.m_eAttrType==SPH_ATTR_JSON )
		m_dStrings.Add(nullptr);
	if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET )
		m_dMvas.Add(0);

	SqlInsert_t tDefaultVal;
	tDefaultVal.m_iType = SqlInsert_t::CONST_INT;
	tDefaultVal.m_iVal = 0;

	SphAttr_t tAttr;
	if ( CSphMatchVariant::ConvertPlainAttr ( tDefaultVal, tCol.m_eAttrType, tAttr ) )
	{
		if ( tCol.IsColumnar() )
			m_dColumnarAttrs [ m_dColumnarRemap[iCol] ] = tAttr;
		else
			m_tDoc.SetAttr ( tLoc, tAttr );
	}
}


bool AttributeConverter_c::SetAttrValue ( int iCol, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx )
{
	const CSphColumnInfo & tCol = m_tSchema.GetAttr(iCol);
	CSphAttrLocator tLoc = tCol.m_tLocator;
	tLoc.m_bDynamic = true;

	if ( !CheckInsertTypes ( tCol, tVal, iRow, iQuerySchemaIdx ) )
		return false;

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

		m_dMvas.Add ( iLen );
		for ( int j=0; j<iLen; j++ )
			m_dMvas.Add ( (*tVal.m_pVals)[j] );
	}

	SphAttr_t tAttr;
	if ( CSphMatchVariant::ConvertPlainAttr ( tVal, tCol.m_eAttrType, tAttr ) )
	{
		if ( tCol.IsColumnar() )
			m_dColumnarAttrs [ m_dColumnarRemap[iCol] ] = tAttr;
		else
			m_tDoc.SetAttr ( tLoc, tAttr );
	}

	if ( !CheckDocId ( tCol, tAttr ) )				return false;
	if ( !CheckStrings ( tCol, tVal, iCol, iRow ) )	return false;
	if ( !CheckJson ( tCol, tVal ) )				return false;

	return true;
}


bool AttributeConverter_c::SetFieldValue ( int iField, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx )
{
	if ( tVal.m_iType!=SqlInsert_t::QUOTED_STRING )
	{
		m_sError.SetSprintf ( "row %d, column %d: string expected", 1+iRow, 1+iQuerySchemaIdx ); // 1 for human base
		return false;
	}

	const char * szFieldValue = tVal.m_sVal.cstr();
	if ( m_dFieldAttrs[iField] )
	{
		m_dTmpFieldStorage[iField] = szFieldValue;
		m_dFields[iField] = { m_dTmpFieldStorage[iField].cstr(), m_dTmpFieldStorage[iField].Length() };
	} else
		m_dFields[iField] = { szFieldValue, ( int64_t) strlen(szFieldValue) };

	return true;
}


void AttributeConverter_c::SetDefaultFieldValue ( int iField )
{
	m_dFields[iField] = { nullptr, 0 };
}


void AttributeConverter_c::NewRow()
{
	m_dStrings.Resize(0);
	m_tStrings.Reset();
	m_dMvas.Resize(0);
}


void AttributeConverter_c::Finalize()
{
	// remap JSON to string pointers
	m_tStrings.SavePointersTo ( m_dStrings );
}

/////////////////////////////////////////////////////////////////////

static bool InsertToPQ ( SqlStmt_t & tStmt, RtIndex_i * pIndex, RtAccum_t * pAccum, CSphVector<int64_t> & dIds, const CSphMatch & tDoc, const CSphAttrLocator & tIdLoc, const CSphVector<const char *> & dStrings,
	const CSphSchema & tSchemaInt, bool bReplace, CSphString & sError )
{
	CSphVector<CSphFilterSettings> dFilters;
	CSphVector<FilterTreeItem_t>   dFilterTree;
	if ( !PercolateParseFilters ( dStrings[2], session::GetCollation(), tSchemaInt, dFilters, dFilterTree, sError ) )
		return false;

	PercolateQueryArgs_t tArgs ( dFilters, dFilterTree );
	tArgs.m_sQuery		= dStrings[0];
	tArgs.m_sTags		= dStrings[1];
	tArgs.m_iQUID		= tDoc.GetAttr(tIdLoc);
	tArgs.m_bReplace	= bReplace;
	tArgs.m_bQL			= true;

	// add query
	auto * pQIndex = (PercolateIndex_i *)pIndex;
	auto pStored = pQIndex->CreateQuery ( tArgs, sError );
	if ( pStored )
	{
		auto * pCmd = pAccum->AddCommand ( ReplicationCommand_e::PQUERY_ADD, tStmt.m_sIndex, tStmt.m_sCluster );
		dIds.Add ( pStored->m_iQUID );
		pCmd->m_pStored = std::move ( pStored );
	}

	return true;
}

void sphHandleMysqlBegin ( StmtErrorReporter_i& tOut, Str_t sQuery )
{
	auto* pSession = session::GetClientSession();
	auto& tAcc = pSession->m_tAcc;
	auto& sError = pSession->m_sError;

	MEMORY ( MEM_SQL_BEGIN );
	RtIndex_i* pIndex = tAcc.GetIndex();
	if ( pIndex )
	{
		RtAccum_t* pAccum = tAcc.GetAcc ( pIndex, sError );
		if ( !sError.IsEmpty() || !HandleCmdReplicate ( *pAccum, sError ) )
		{
			tOut.Error ( sQuery.first, sError.cstr() );
			return;
		}
	}
	pSession->m_bInTransaction = true;
	tOut.Ok ( 0 );
}

void sphHandleMysqlCommitRollback ( StmtErrorReporter_i& tOut, Str_t sQuery, bool bCommit )
{
	auto* pSession = session::GetClientSession();
	auto& tAcc = pSession->m_tAcc;
	auto& sError = pSession->m_sError;
	auto& tCrashQuery = GlobalCrashQueryGetRef();

	MEMORY ( MEM_SQL_COMMIT );
	pSession->m_bInTransaction = false;
	RtIndex_i* pIndex = tAcc.GetIndex();
	int iDeleted = 0;
	if ( pIndex )
	{
		tCrashQuery.m_dIndex = FromStr ( pIndex->GetName() );
		RtAccum_t* pAccum = tAcc.GetAcc ( pIndex, sError );
		if ( !sError.IsEmpty() )
		{
			tOut.Error ( sQuery.first, sError.cstr() );
			return;
		}
		if ( bCommit )
		{
			StatCountCommand ( SEARCHD_COMMAND_COMMIT );
			if ( !HandleCmdReplicate ( *pAccum, sError, iDeleted ) )
			{
				tOut.Error ( sQuery.first, sError.cstr() );
				return;
			}
		} else
		{
			pIndex->RollBack ( pAccum );
		}
	}
	tOut.Ok ( iDeleted );
}

void sphHandleMysqlInsert ( StmtErrorReporter_i & tOut, SqlStmt_t & tStmt )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	auto* pSession = session::GetClientSession();
	pSession->FreezeLastMeta();
	bool bReplace = ( tStmt.m_eStmt == STMT_REPLACE );
	bool bCommit = pSession->m_bAutoCommit && !pSession->m_bInTransaction;
	auto& sWarning = pSession->m_tLastMeta.m_sWarning;
	auto& tAcc = pSession->m_tAcc;
	auto& dLastIds = pSession->m_dLastIds;
	StatCountCommand ( bReplace ? SEARCHD_COMMAND_REPLACE : SEARCHD_COMMAND_INSERT );

	MEMORY ( MEM_SQL_INSERT );

	CSphString sError;
	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( !ServedDesc_t::IsMutable ( pServed ) )
	{
		tOut.Error ( "index '%s' absent, or does not support INSERT", tStmt.m_sIndex.cstr ());
		return;
	}

	GlobalCrashQueryGetRef().m_dIndex = FromStr ( tStmt.m_sIndex );

	bool bPq = ( pServed->m_eType==IndexType_e::PERCOLATE );

	RIdx_T<RtIndex_i*> pIndex { pServed };

	// get schema, check values count
	const CSphSchema & tSchema = pIndex->GetMatchSchema ();
	int iSchemaSz = tSchema.GetAttrsCount() + tSchema.GetFieldsCount();
	if ( pIndex->GetSettings().m_bIndexFieldLens )
		iSchemaSz -= tSchema.GetFieldsCount();

	if ( tSchema.GetAttr ( sphGetBlobLocatorName() ) )
		iSchemaSz--;

	int iExp = tStmt.m_iSchemaSz;
	int iGot = tStmt.m_dInsertValues.GetLength();
	if ( !tStmt.m_dInsertSchema.GetLength()	&& iSchemaSz!=tStmt.m_iSchemaSz )
	{
		tOut.Error ( "column count does not match schema (expected %d, got %d)", iSchemaSz, iGot );
		return;
	}

	if ( ( iGot % iExp )!=0 )
	{
		tOut.Error ( "column count does not match value count (expected %d, got %d)", iExp, iGot );
		return;
	}

	if ( !CheckIndexCluster ( tStmt.m_sIndex, *pServed, tStmt.m_sCluster, IsHttpStmt ( tStmt ), sError ) )
	{
		tOut.Error ( "%s", sError.cstr() );
		return;
	}

	if ( !sError.IsEmpty() )
	{
		tOut.Error ( "%s", sError.cstr() );
		return;
	}

	CSphVector<int> dAttrSchema ( tSchema.GetAttrsCount() );
	CSphVector<int> dFieldSchema ( tSchema.GetFieldsCount() );
	CSphVector<bool> dFieldAttrs ( tSchema.GetFieldsCount() );
	if ( !CreateAttrMaps ( dAttrSchema, dFieldSchema, dFieldAttrs, tSchema, tStmt.m_dInsertSchema, tOut ) )
		return;

	RtAccum_t * pAccum = tAcc.GetAcc ( pIndex, sError );
	if ( !pAccum )
	{
		tOut.Error ( "%s", sError.cstr() );
		return;
	}

	CSphVector<int64_t> dIds;
	dIds.Reserve ( tStmt.m_iRowsAffected );

	const CSphColumnInfo * pDocid = tSchema.GetAttr(sphGetDocidName());
	assert ( pDocid );
	CSphAttrLocator tIdLoc = pDocid->m_tLocator;
	tIdLoc.m_bDynamic = true;

	AttributeConverter_c tConverter ( tSchema, dFieldAttrs, sError, sWarning );

	// convert attrs
	for ( int iRow=0; iRow<tStmt.m_iRowsAffected; iRow++ )
	{
		assert ( sError.IsEmpty() );
		tConverter.NewRow();

		int iSchemaAttrCount = tSchema.GetAttrsCount();
		if ( pIndex->GetSettings().m_bIndexFieldLens )
			iSchemaAttrCount -= tSchema.GetFieldsCount();

		bool bOk = true;
		for ( int i=0; i<iSchemaAttrCount && bOk; i++ )
		{
			int iQuerySchemaIdx = dAttrSchema[i];
			if ( iQuerySchemaIdx < 0 )
				tConverter.SetDefaultAttrValue(i);
			else
				bOk = tConverter.SetAttrValue ( i, tStmt.m_dInsertValues[iQuerySchemaIdx + iRow * iExp], iRow, iQuerySchemaIdx );
		}

		if ( !bOk )
			break;

		// if strings and fields share one value, it might be modified by html stripper etc
		// we need to use separate storage for such string attributes and fields
		for ( int i = 0; i < tSchema.GetFieldsCount() && bOk; i++ )
		{
			int iQuerySchemaIdx = dFieldSchema[i];
			if ( iQuerySchemaIdx < 0 )
				tConverter.SetDefaultFieldValue(i);	
			else
				bOk = tConverter.SetFieldValue( i, tStmt.m_dInsertValues [ iQuerySchemaIdx + iRow * iExp ], iRow, iQuerySchemaIdx );
		}

		if ( !bOk )
			break;

		tConverter.Finalize();

		// do add
		if ( bPq )
		{
			if ( !InsertToPQ ( tStmt, pIndex, pAccum, dIds, tConverter.m_tDoc, tIdLoc, tConverter.m_dStrings, pIndex->GetInternalSchema(), bReplace, sError ) )
				break;
		}
		else
		{
			pIndex->AddDocument ( tConverter, bReplace, tStmt.m_sStringParam, sError, sWarning, pAccum );
			dIds.Add ( tConverter.GetID() );

			pAccum->AddCommand ( ReplicationCommand_e::RT_TRX, tStmt.m_sIndex, tStmt.m_sCluster );
		}

		if ( !sError.IsEmpty() )
			break;
	}

	// fire exit
	if ( !sError.IsEmpty() )
	{
		pIndex->RollBack ( pAccum ); // clean up collected data
		tOut.Error ( "%s", sError.cstr() );
		return;
	}

	dLastIds.SwapData ( dIds );

	// no errors so far
	if ( bCommit )
	{
		if ( !HandleCmdReplicate ( *pAccum, sError ) )
		{
			pIndex->RollBack ( pAccum ); // clean up collected data
			tOut.Error ( "%s", sError.cstr() );
			return;
		}
	}

	int64_t iLastInsertId = 0;
	if ( dLastIds.GetLength() )
		iLastInsertId = dLastIds.Last();

	// my OK packet
	tOut.Ok ( tStmt.m_iRowsAffected, sWarning, iLastInsertId );
}


void HandleMysqlCallSnippets ( RowBuffer_i & tOut, SqlStmt_t & tStmt )
{
	StatCountCommand ( SEARCHD_COMMAND_EXCERPT );
	CSphString sError;

	// check arguments
	// string data, string index, string query, [named opts]
	if ( tStmt.m_dInsertValues.GetLength()!=3 )
	{
		tOut.Error ( tStmt.m_sStmt, "SNIPPETS() expects exactly 3 arguments (data, index, query)" );
		return;
	}
	if ( tStmt.m_dInsertValues[0].m_iType!=SqlInsert_t::QUOTED_STRING && tStmt.m_dInsertValues[0].m_iType!=SqlInsert_t::CONST_STRINGS )
	{
		tOut.Error ( tStmt.m_sStmt, "SNIPPETS() argument 1 must be a string or a string list" );
		return;
	}
	if ( tStmt.m_dInsertValues[1].m_iType!=SqlInsert_t::QUOTED_STRING )
	{
		tOut.Error ( tStmt.m_sStmt, "SNIPPETS() argument 2 must be a string" );
		return;
	}
	if ( tStmt.m_dInsertValues[2].m_iType!=SqlInsert_t::QUOTED_STRING )
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

		if ( sOpt=="before_match" )				{ q.m_sBeforeMatch = v.m_sVal; iExpType = SqlInsert_t::QUOTED_STRING; }
		else if ( sOpt=="after_match" )			{ q.m_sAfterMatch = v.m_sVal; iExpType = SqlInsert_t::QUOTED_STRING; }
		else if ( sOpt=="chunk_separator" || sOpt=="snippet_separator" ) { q.m_sChunkSeparator = v.m_sVal; iExpType = SqlInsert_t::QUOTED_STRING; }
		else if ( sOpt=="html_strip_mode" )		{ q.m_sStripMode = v.m_sVal; iExpType = SqlInsert_t::QUOTED_STRING; }
		else if ( sOpt=="passage_boundary" || sOpt=="snippet_boundary" ) { q.m_ePassageSPZ = GetPassageBoundary(v.m_sVal); iExpType = SqlInsert_t::QUOTED_STRING; }

		else if ( sOpt=="limit" )				{ q.m_iLimit = (int)v.m_iVal; iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="limit_words" )			{ q.m_iLimitWords = (int)v.m_iVal; iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="limit_passages" || sOpt=="limit_snippets" ) { q.m_iLimitPassages = (int)v.m_iVal; iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="around" )				{ q.m_iAround = (int)v.m_iVal; iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="start_passage_id" || sOpt=="start_snippet_id" ) { q.m_iPassageId = (int)v.m_iVal; iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="exact_phrase" )
		{
			sError.SetSprintf ( "exact_phrase is deprecated" );
			break;
		}
		else if ( sOpt=="use_boundaries" )		{ q.m_bUseBoundaries = ( v.m_iVal!=0 ); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="weight_order" )		{ q.m_bWeightOrder = ( v.m_iVal!=0 ); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="query_mode" )
		{
			bool bQueryMode = ( v.m_iVal!=0 );
			iExpType = SqlInsert_t::CONST_INT;
			if ( !bQueryMode )
			{
				sError.SetSprintf ( "query_mode=0 is deprecated" );
				break;
			}
		}
		else if ( sOpt=="force_all_words" )		{ q.m_bForceAllWords = ( v.m_iVal!=0 ); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="load_files" )			{ q.m_uFilesMode = ( v.m_iVal!=0 )?1:0; iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="load_files_scattered" ) { q.m_uFilesMode |= ( v.m_iVal!=0 )?2:0; iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="allow_empty" )			{ q.m_bAllowEmpty = ( v.m_iVal!=0 ); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="emit_zones" )			{ q.m_bEmitZones = ( v.m_iVal!=0 ); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="force_passages" || sOpt=="force_snippets" ) { q.m_bForcePassages = ( v.m_iVal!=0 ); iExpType = SqlInsert_t::CONST_INT; }
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
	if ( tStmt.m_dInsertValues[0].m_iType==SqlInsert_t::QUOTED_STRING )
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

	myinfo::SetTaskInfo ( R"(sphinxql-snippet datasize=%.1Dk query="%s")", GetSnippetDataSize ( dQueries ), q.m_sQuery.scstr ());

	if ( !MakeSnippets ( sIndex, dQueries, q, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !dQueries.any_of ( [] ( const ExcerptQuery_t & tQuery ) { return tQuery.m_sError.IsEmpty(); } ) )
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
		bOk = RIdx_c(pLocal)->GetKeywords ( dKeywords, sQuery.cstr(), tSettings, &sError );
	else
	{
		// FIXME!!! g_iDistThreads thread pool for locals.
		// locals
		const StrVec_t & dLocals = pDistributed->m_dLocal;
		CSphVector<CSphKeywordInfo> dKeywordsLocal;
		for ( const CSphString &sLocal : dLocals )
		{
			auto pServed = GetServed ( sLocal );
			if ( !pServed )
			{
				tFailureLog.Submit ( sLocal.cstr(), sIndex.cstr(), "missed index" );
				continue;
			}

			dKeywordsLocal.Resize(0);
			if ( RIdx_c ( pServed )->GetKeywords ( dKeywordsLocal, sQuery.cstr(), tSettings, &sError ) )
				dKeywords.Append ( dKeywordsLocal );
			else
				tFailureLog.SubmitEx ( sLocal, sIndex.cstr (), "keyword extraction failed: %s", sError.cstr () );
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
					tFailureLog.SubmitEx ( pAgent->m_tDesc.m_sIndexes, sIndex.cstr(),
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
	StatCountCommand ( SEARCHD_COMMAND_KEYWORDS );
	CSphString sError;

	// string query, string index, [bool hits] || [value as option_name, ...]
	int iArgs = tStmt.m_dInsertValues.GetLength();
	if ( iArgs<2
		|| iArgs>3
		|| tStmt.m_dInsertValues[0].m_iType!=SqlInsert_t::QUOTED_STRING
		|| tStmt.m_dInsertValues[1].m_iType!=SqlInsert_t::QUOTED_STRING
		|| ( iArgs==3 && tStmt.m_dInsertValues[2].m_iType!=SqlInsert_t::CONST_INT ) )
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
		if ( bOptInt && tStmt.m_dCallOptValues[i].m_iType!=SqlInsert_t::CONST_INT )
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
	for ( const auto& tWord : hWords )
		dSrc.Add ( tWord.second );

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
	StatCountCommand ( SEARCHD_COMMAND_SUGGEST );
	CSphString sError;

	// string query, string index, [value as option_name, ...]
	int iArgs = tStmt.m_dInsertValues.GetLength ();
	if ( iArgs<2
			|| iArgs>3
			|| tStmt.m_dInsertValues[0].m_iType!=SqlInsert_t::QUOTED_STRING
			|| tStmt.m_dInsertValues[1].m_iType!=SqlInsert_t::QUOTED_STRING
			|| ( iArgs==3 && tStmt.m_dInsertValues[2].m_iType!=SqlInsert_t::CONST_INT ) )
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
		int iTokType = SqlInsert_t::CONST_INT;

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
		auto pServed = GetServed ( tStmt.m_dInsertValues[1].m_sVal );
		if ( !pServed )
		{
			sError.SetSprintf ( "no such index %s", tStmt.m_dInsertValues[1].m_sVal.cstr () );
			tOut.Error ( tStmt.m_sStmt, sError.cstr () );
			return;
		}
		RIdx_c pIdx { pServed };
		if ( !pIdx->GetSettings().m_iMinInfixLen || !pIdx->GetDictionary()->GetSettings().m_bWordDict )
		{
			sError.SetSprintf ( "suggests work only for keywords dictionary with infix enabled" );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}

		if ( tRes.SetWord ( sWord, pIdx->GetQueryTokenizer(), tArgs.m_bQueryMode ) )
		{
			pIdx->GetSuggest ( tArgs, tRes );
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
			tOut.PutString ( sBuf );
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


static void AddFieldDesc ( VectorLike & dOut, const CSphColumnInfo & tField )
{
	if ( !dOut.MatchAdd ( tField.m_sName.cstr() ) )
		return;

	dOut.Add ( "text" );
	StringBuilder_c sProperties ( " " );
	DWORD uFlags = tField.m_uFieldFlags;
	if ( uFlags & CSphColumnInfo::FIELD_INDEXED )
		sProperties << "indexed";

	if ( uFlags & CSphColumnInfo::FIELD_STORED )
		sProperties << "stored";
	dOut.Add ( sProperties.cstr () );
}


static void AddAttributeDesc ( VectorLike & dOut, const CSphColumnInfo & tAttr )
{
	if ( sphIsInternalAttr ( tAttr ) )
		return;

	if ( dOut.MatchAdd ( tAttr.m_sName.cstr() ) )
	{
		if ( tAttr.m_eAttrType==SPH_ATTR_INTEGER && tAttr.m_tLocator.m_iBitCount!=ROWITEM_BITS && tAttr.m_tLocator.m_iBitCount>0 )
		{
			StringBuilder_c sName;
			sName.Sprintf ( "%s:%d", sphTypeName ( tAttr.m_eAttrType ), tAttr.m_tLocator.m_iBitCount );
			dOut.Add ( sName.cstr() );
		} else
			dOut.Add ( sphTypeName ( tAttr.m_eAttrType ) );

		StringBuilder_c sProps(" ");
		if ( tAttr.IsColumnar() )
			sProps << "columnar";

		if ( tAttr.m_uAttrFlags & CSphColumnInfo::ATTR_STORED )
			sProps << "fast_fetch";

		if ( tAttr.IsColumnar() && tAttr.m_eAttrType==SPH_ATTR_STRING && !(tAttr.m_uAttrFlags & CSphColumnInfo::ATTR_COLUMNAR_HASHES) )
			sProps << "no_hash";

		dOut.Add ( sProps.cstr() );
	}
}


void DescribeLocalSchema ( VectorLike & dOut, const CSphSchema & tSchema, bool bIsTemplate )
{
	// result set header packet
	dOut.SetColNames ( { "Field", "Type", "Properties" } );

	// id comes before fields
	if ( !bIsTemplate )
	{
		assert ( tSchema.GetAttr(0).m_sName==sphGetDocidName() );
		AddAttributeDesc ( dOut, tSchema.GetAttr(0) );
	}

	for ( int i = 0; i<tSchema.GetFieldsCount (); ++i )
		AddFieldDesc ( dOut, tSchema.GetField(i) );

	for ( int i = 1; i<tSchema.GetAttrsCount (); ++i )
		AddAttributeDesc ( dOut, tSchema.GetAttr(i) );
}


void DescribeDistributedSchema ( VectorLike& dOut, const cDistributedIndexRefPtr_t& pDistr )
{
	// result set header packet
	dOut.SetColNames ( { "Agent", "Type" } );

	for ( const auto & sIdx : pDistr->m_dLocal )
		dOut.MatchTuplet( sIdx.cstr (), "local" );

	ARRAY_CONSTFOREACH ( i, pDistr->m_dAgents )
	{
		MultiAgentDescRefPtr_c pMultiAgent = pDistr->m_dAgents[i];
		const MultiAgentDesc_c & tMultiAgent = *pMultiAgent;
		if ( tMultiAgent.IsHA () )
		{
			int iNumMultiAgents = tMultiAgent.GetLength();
			for ( int j = 0; j < iNumMultiAgents; j++ )
			{
				const AgentDesc_t & tDesc = tMultiAgent[j];
				StringBuilder_c sValue;
				sValue << tDesc.GetMyUrl().cstr() << ":" << tDesc.m_sIndexes.cstr();
				dOut.MatchTupletf ( sValue.cstr (), "%s_%d_mirror_%d",
						tDesc.m_bBlackhole ? "blackhole" : "remote", i+1, j+1 );
			}
		} else
		{
			const AgentDesc_t & tDesc = tMultiAgent[0];
			StringBuilder_c sValue;
			sValue << tDesc.GetMyUrl ().cstr () << ":" << tDesc.m_sIndexes.cstr ();
			dOut.MatchTupletf ( sValue.cstr (), "%s_%d", tDesc.m_bBlackhole ? "blackhole" : "remote", i+1 );
		}
	}
}


void HandleMysqlDescribe ( RowBuffer_i & tOut, const SqlStmt_t * pStmt )
{
	auto & tStmt = *pStmt;
	VectorLike dOut ( tStmt.m_sStringParam, 0 );

	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( pServed )
	{
		// data
		const CSphSchema *pSchema = &RIdx_c(pServed)->GetMatchSchema ();
		bool bNeedInternal = false;

		if ( tStmt.m_iIntParam==SqlInsert_t::TABLE ) // user wants internal schema instead
			bNeedInternal = true;

		if ( tStmt.m_dStringSubkeys.GetLength()==1 && tStmt.m_dStringSubkeys[0].EqN(".table") )
			bNeedInternal = true;

		if ( bNeedInternal && ServedDesc_t::IsMutable ( pServed ) )
		{
			RIdx_T<const RtIndex_i*> pRtIndex { pServed };
			pSchema = &pRtIndex->GetInternalSchema();
		}

		const CSphSchema &tSchema = *pSchema;
		assert ( pServed->m_eType==IndexType_e::TEMPLATE || tSchema.GetAttr(0).m_sName==sphGetDocidName() );
		DescribeLocalSchema ( dOut, tSchema, pServed->m_eType==IndexType_e::TEMPLATE );
	} else
	{
		auto pDistr = GetDistr ( tStmt.m_sIndex );
		if ( !pDistr )
		{
			tOut.ErrorAbsent ( tStmt.m_sStmt, "no such index '%s'", tStmt.m_sIndex.cstr () );
			return;
		}
		DescribeDistributedSchema ( dOut, pDistr );
	}

	tOut.DataTable ( dOut );
}


void HandleMysqlShowTables ( RowBuffer_i & tOut, const SqlStmt_t * pStmt )
{
	// 0 local, 1 distributed, 2 rt, 3 template, 4 percolate, 5 unknown
	static const char* sTypes[] = {"local", "distributed", "rt", "template", "percolate", "unknown"};
	CSphVector<CSphNamedInt> dIndexes;

	// collect local, rt, percolate
	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( const auto& tIt : *hLocal )
	{
		if ( !tIt.second )
			continue;

		switch ( tIt.second->m_eType )
		{
			case IndexType_e::PLAIN:
				dIndexes.Add ( CSphNamedInt ( tIt.first, 0 ) );
				break;
			case IndexType_e::RT:
				dIndexes.Add ( CSphNamedInt ( tIt.first, 2 ) );
				break;
			case IndexType_e::PERCOLATE:
				dIndexes.Add ( CSphNamedInt ( tIt.first, 4 ) );
				break;
			case IndexType_e::TEMPLATE:
				dIndexes.Add ( CSphNamedInt ( tIt.first, 3 ) );
				break;
			default:
				dIndexes.Add ( CSphNamedInt ( tIt.first, 5 ) );
		}
	}

	// collect distributed
	assert ( g_pDistIndexes );
	auto pDistSnapshot = g_pDistIndexes->GetHash();
	for ( auto& tIt : *pDistSnapshot )
		// no need to check distr's it, iterating guarantees index existance.
		dIndexes.Add ( CSphNamedInt ( tIt.first, 1 ) );

	dIndexes.Sort ( Lesser ([] ( const CSphNamedInt & a, const CSphNamedInt & b)
			{ return strcasecmp ( a.first.cstr (), b.first.cstr () )<0; }));

	// output the results
	VectorLike dTable ( pStmt->m_sStringParam, { "Index", "Type" } );
	for ( auto& dPair : dIndexes )
		dTable.MatchTuplet( dPair.first.cstr (), sTypes[dPair.second] );
	tOut.DataTable ( dTable );
}

template <typename T, typename GETNAME>
static bool CheckAttrs ( const VecTraits_T<T> & dAttrs, GETNAME && fnGetName, CSphString & sError )
{
	ARRAY_FOREACH ( i, dAttrs )
	{
		const CSphString & sName = fnGetName(dAttrs[i]);
		if ( CSphSchema::IsReserved ( sName.cstr() ) || sphIsInternalAttr ( sName ) )
		{
			sError.SetSprintf ( "attribute name '%s' is a reserved keyword", sName.cstr() );
			return false;
		}

		for ( int j = i+1; j < dAttrs.GetLength(); j++ )
			if ( fnGetName(dAttrs[j])==sName )
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

	if ( !CheckAttrs ( tStmt.m_tCreateTable.m_dAttrs, []( const CreateTableAttr_t & tAttr ) { return tAttr.m_tAttr.m_sName; }, sError ) )
		return false;

	if ( !CheckAttrs ( tStmt.m_tCreateTable.m_dFields, []( const CSphColumnInfo & tAttr ) { return tAttr.m_sName; }, sError ) )
		return false;

	// cross-checks attrs and fields
	for ( const auto & i : tStmt.m_tCreateTable.m_dAttrs )
		for ( const auto & j : tStmt.m_tCreateTable.m_dFields )
			if ( i.m_tAttr.m_sName==j.m_sName && i.m_tAttr.m_eAttrType!=SPH_ATTR_STRING )
			{
				sError.SetSprintf ( "duplicate attribute name '%s'", i.m_tAttr.m_sName.cstr() );
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

	if ( !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
		return;

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
	bool bCreatedOk = CreateNewIndexConfigless ( tStmt.m_sIndex, tStmt.m_tCreateTable, dWarnings, sError );
	sWarning = ConcatWarnings(dWarnings);

	if ( !bCreatedOk )
	{
		sError.SetSprintf ( "error adding index '%s': %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	tOut.Ok ( 0, dWarnings.GetLength() );
}


static const CSphSchema & GetSchemaForCreateTable ( const CSphIndex * pIndex )
{
	assert ( pIndex );
	assert ( pIndex->IsRT() || pIndex->IsPQ() );

	return ((const RtIndex_i*)pIndex)->GetInternalSchema();
}


static CSphString BuildCreateTableRt ( const CSphString & sName, const CSphIndex * pIndex, const CSphSchema & tSchema )
{
	assert(pIndex);

	CSphString sCreateTable = BuildCreateTable ( sName, pIndex, tSchema );
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
	CSphString sCreateTable;
	switch ( IndexIsServed ( sLike ) )
	{
	case RunIdx_e::NOTSERVED:
		sError.SetSprintf ( "index '%s': CREATE TABLE LIKE failed: no index '%s' found", tStmt.m_sIndex.cstr(), sLike.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	case RunIdx_e::LOCAL:
	{
		auto pServed = GetServed ( sLike );
		assert ( pServed );
		if ( !ServedDesc_t::IsMutable ( pServed ) )
		{
			tOut.ErrorAbsent ( tStmt.m_sStmt, "index '%s' is not real-time or percolate", sError.cstr() );
			return;
		}
		RIdx_c pIdx { pServed };
		sCreateTable = BuildCreateTableRt ( tStmt.m_sIndex, pIdx, GetSchemaForCreateTable ( pIdx ) );
		break;
	}
	case RunIdx_e::DISTR:
	{
		auto pDist = GetDistr ( sLike );
		sCreateTable = BuildCreateTableDistr ( tStmt.m_sIndex, *pDist );
	}
	default: break;
	};

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
	if ( !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
		return;

	CSphString sError;

	if ( !IsConfigless() )
	{
		sError = "DROP TABLE requires data_dir to be set in the config file";
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	bool bDropped = DropIndexInt ( tStmt.m_sIndex.cstr(), tStmt.m_bIfExists, sError );
	sphLogDebug ( "dropped table %s, ok %d, error %s", tStmt.m_sIndex.cstr(), (int)bDropped, sError.scstr() ); // FIXME!!! remove
	if ( !bDropped )
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
	else
		tOut.Ok();
}


void HandleMysqlShowCreateTable ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	auto pServed = GetServed ( tStmt.m_sIndex );
	auto pDist = GetDistr ( tStmt.m_sIndex );
	if ( !pServed && !pDist )
	{
		tOut.ErrorAbsent ( tStmt.m_sStmt, "no such index '%s'", tStmt.m_sIndex.cstr () );
		return;
	}

	if ( pServed && !ServedDesc_t::IsMutable ( pServed ) )
	{
		tOut.ErrorAbsent ( tStmt.m_sStmt, "index '%s' is not real-time or percolate", tStmt.m_sIndex.cstr () );
		return;
	}

	// result set header packet
	tOut.HeadTuplet ( "Table", "Create Table" );
	CSphString sCreateTable;
	if ( pServed )
	{
		RIdx_c pIdx { pServed };
		sCreateTable = BuildCreateTableRt ( pIdx->GetName(), pIdx, GetSchemaForCreateTable ( pIdx ) );
	} else
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
			FormatSphinxql ( *tThd.m_pQuery, 0, tBuf );
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

void HandleMysqlShowThreads ( RowBuffer_i & tOut, const SqlStmt_t * pStmt )
{
	ThreadInfoFormat_e eFmt = THD_FORMAT_NATIVE;
	bool bAll = false;
	int iCols = -1;
	if ( pStmt )
	{
		if ( pStmt->m_sThreadFormat == "sphinxql" )
			eFmt = THD_FORMAT_SPHINXQL;
		else if ( pStmt->m_sThreadFormat == "all" )
			bAll = true;
		iCols = pStmt->m_iThreadsCols;
	}

	tOut.HeadBegin ( bAll ? 15 : 14 ); // 15 with chain
	tOut.HeadColumn ( "Tid", MYSQL_COL_LONG );
	tOut.HeadColumn ( "Name" );
	tOut.HeadColumn ( "Proto" );
	tOut.HeadColumn ( "State" );
	tOut.HeadColumn ( "Host" );
	tOut.HeadColumn ( "ConnID", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "Time", MYSQL_COL_FLOAT );
	tOut.HeadColumn ( "Work time" );
	tOut.HeadColumn ( "Work time CPU" );
	tOut.HeadColumn ( "Thd efficiency", MYSQL_COL_FLOAT);
	tOut.HeadColumn ( "Jobs done", MYSQL_COL_LONG );
	tOut.HeadColumn ( "Last job took" );
	tOut.HeadColumn ( "In idle" );
	if ( bAll )
		tOut.HeadColumn ( "Chain" );
	tOut.HeadColumn ( "Info" );
	if (!tOut.HeadEnd())
		return;

	QuotationEscapedBuilder tBuf;

//	sphLogDebug ( "^^ Show threads. Current info is %p", GetTaskInfo () );

	CSphSwapVector<PublicThreadDesc_t> dFinal;
	Threads::IterateActive([&dFinal, iCols] ( Threads::LowThreadDesc_t * pThread ){
		if ( pThread )
			dFinal.Add ( GatherPublicTaskInfo ( pThread, iCols ) );
	});

	for ( const auto & dThd : dFinal )
	{
		if ( !bAll && dThd.m_eTaskState==TaskState_e::UNKNOWN )
			continue;
		tOut.PutNumAsString ( dThd.m_iThreadID );
		tOut.PutString ( dThd.m_sThreadName );
		tOut.PutString ( dThd.m_sProto );
		tOut.PutString ( TaskStateName ( dThd.m_eTaskState ) );
		tOut.PutString ( dThd.m_sClientName ); // Host
		tOut.PutNumAsString ( dThd.m_iConnID ); // ConnID
		int64_t tmNow = sphMicroTimer (); // short-term cache
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

		if ( bAll )
			tOut.PutString ( dThd.m_sChain ); // Chain
		auto tInfo = FormatInfo ( dThd, eFmt, tBuf );
		if ( iCols >= 0 && iCols < tInfo.second )
			tInfo.second = iCols;
		tOut.PutString ( tInfo.first, tInfo.second ); // Info m_pTaskInfo
		if ( !tOut.Commit () )
			break;
	}

	tOut.Eof();
}

void HandleMysqlFlushHostnames ( RowBuffer_i & tOut )
{
	SmallStringHash_T<DWORD> hHosts;

	// Collect all urls from all distr indexes
	assert ( g_pDistIndexes );
	auto pDistSnapshot = g_pDistIndexes->GetHash();
	for ( auto& tIt : *pDistSnapshot )
		tIt.second->ForEveryHost ( [&] ( AgentDesc_t& tDesc ) {
			if ( tDesc.m_bNeedResolve )
				hHosts.Add ( tDesc.m_uAddr, tDesc.m_sAddr );
		});


	for ( auto tHost : hHosts )
	{
		DWORD uRenew = sphGetAddress ( tHost.first.cstr() );
		if ( uRenew )
			tHost.second = uRenew;
	}

	// copy back renew hosts to distributed agents.
	// case when distr index list changed between collecting urls and applying them
	// is safe, since we are iterating over the list again, and also apply
	// only existing hosts.
	for ( auto& tIt : *pDistSnapshot )
		tIt.second->ForEveryHost ( [&] ( AgentDesc_t& tDesc ) {
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


SphinxqlRequestBuilder_c::SphinxqlRequestBuilder_c ( Str_t sQuery, const SqlStmt_t & tStmt )
	: m_sBegin { sQuery.first, tStmt.m_iListStart }
	, m_sEnd ( sQuery.first + tStmt.m_iListEnd, sQuery.second - tStmt.m_iListEnd )
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
	const cServedIndexRefPtr_c & pServed )
{
	CSphString sError;
	// checks
	{
		if ( !pServed )
		{
			dFails.Submit ( sIndex, sDistributed, "index not available" );
			return;
		}

		if ( !CheckIndexCluster ( sIndex, *pServed, tStmt.m_sCluster, IsHttpStmt ( tStmt ), sError ) )
		{
			dFails.Submit ( sIndex, sDistributed, sError.cstr() );
			return;
		}
	}

	RtAccum_t tAcc ( false );
	ReplicationCommand_t * pCmd = tAcc.AddCommand ( tStmt.m_bJson ? ReplicationCommand_e::UPDATE_JSON : ReplicationCommand_e::UPDATE_QL, sIndex, tStmt.m_sCluster );
	assert ( pCmd );
	pCmd->m_pUpdateAPI = tStmt.AttrUpdatePtr();
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

bool HandleUpdateAPI ( AttrUpdateArgs& tArgs, CSphIndex* pIndex, int& iUpdate )
{
	bool bCritical = false;
	iUpdate = pIndex->UpdateAttributes ( tArgs.m_pUpdate, bCritical, *tArgs.m_pError, *tArgs.m_pWarning );
	return !bCritical;
}

namespace {
DocsCollector_c InitUpdate( AttrUpdateArgs& tArgs, const cServedIndexRefPtr_c& pDesc )
{
	DocsCollector_c tCollector ( *tArgs.m_pQuery, tArgs.m_bJson, *tArgs.m_pIndexName, pDesc, tArgs.m_pError );
	AttrUpdateSharedPtr_t& pUpdate = tArgs.m_pUpdate;
	pUpdate->m_bReusable = false;
	pUpdate->m_bIgnoreNonexistent = tArgs.m_pQuery->m_bIgnoreNonexistent;
	pUpdate->m_bStrict = tArgs.m_pQuery->m_bStrict;
	return tCollector;
}

template<typename RWLOCKED>
void DoUpdate( DocsCollector_c& tCollector, AttrUpdateArgs& tArgs, RWLOCKED& rwLocked )
{
	AttrUpdateSharedPtr_t& pUpdate = tArgs.m_pUpdate;
	while ( tCollector.GetValuesChunk ( pUpdate->m_dDocids, tArgs.m_pQuery->m_iMaxMatches ) )
	{
		int iChanged = 0;
		Verify ( HandleUpdateAPI ( tArgs, rwLocked, iChanged ) ); // fixme! handle this
		tArgs.m_iAffected += iChanged;
	}
}

void UpdateWlocked ( AttrUpdateArgs& tArgs, const cServedIndexRefPtr_c& pDesc, int& iUpdated )
{
	// short-living r-lock m.b. acquired and released by collector when running query
	auto tCollector = InitUpdate ( tArgs, pDesc );
	WIdx_c wLocked { pDesc }; // exclusive lock for process of update. Note, between collecting and updating m.b. race! To eliminate it, need to trace index generation and recollect if it changed.
	DoUpdate ( tCollector, tArgs, wLocked );
}

void UpdateRlocked ( AttrUpdateArgs& tArgs, const cServedIndexRefPtr_c& pDesc, int& iUpdated)
{
	// wide r-lock over whole update. r-locks acquired by collector should be re-enterable.
	RWIdx_c rLocked { pDesc };
	auto tCollector = InitUpdate ( tArgs, pDesc );
	DoUpdate ( tCollector, tArgs, rLocked );
}
} // unnamed namespace

void HandleMySqlExtendedUpdate ( AttrUpdateArgs& tArgs, const cServedIndexRefPtr_c& pDesc, int& iUpdated, bool bNeedWlock )
{
	return bNeedWlock ? UpdateWlocked ( tArgs, pDesc, iUpdated ) : UpdateRlocked ( tArgs, pDesc, iUpdated );
}

void sphHandleMysqlUpdate ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt, Str_t sQuery )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	auto* pSession = session::GetClientSession();
	pSession->FreezeLastMeta();
	auto& sWarning = pSession->m_tLastMeta.m_sWarning;
	StatCountCommand ( SEARCHD_COMMAND_UPDATE );

	int64_t tmStart = sphMicroTimer();

	// extract index names
	StrVec_t dIndexNames;
	ParseIndexList ( tStmt.m_sIndex, dIndexNames );
	if ( dIndexNames.IsEmpty() )
	{
		tOut.Error ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		return;
	}

	DistrPtrs_t dDistributed;
	// copy distributed indexes description
	CSphString sMissed;
	if ( !ExtractDistributedIndexes ( dIndexNames, dDistributed, sMissed ) )
	{
		tOut.Error ( "unknown index '%s' in update request", sMissed.cstr() );
		return;
	}

	// do update
	SearchFailuresLog_c dFails;
	int iSuccesses = 0;
	int iUpdated = 0;
	int iWarns = 0;

	bool bBlobUpdate = false;
	for ( const auto & i : tStmt.AttrUpdate().m_dAttributes )
	{
		if ( i.m_sName==sphGetDocidName() )
		{
			tOut.Error ( "'id' attribute cannot be updated" );
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
			std::unique_ptr<RequestBuilder_i> pRequestBuilder = CreateRequestBuilder ( sQuery, tStmt );
			std::unique_ptr<ReplyParser_i> pReplyParser = CreateReplyParser ( tStmt.m_bJson, iUpdated, iWarns );
			iSuccesses += PerformRemoteTasks ( dAgents, pRequestBuilder.get (), pReplyParser.get () );
		}
	}

	StringBuilder_c sReport;
	dFails.BuildReport ( sReport );

	if ( !iSuccesses )
	{
		tOut.Error ( "%s", sReport.cstr() );
		return;
	} else
	{
		int64_t tmRealTime = sphMicroTimer() - tmStart;
		LogStatementSphinxql ( sQuery, (int)( tmRealTime / 1000 ) );
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
		CheckQuery ( tHandler.m_dQueries[i], tHandler.m_dAggrResults[i].m_sError );
		if ( !tHandler.m_dAggrResults[i].m_sError.IsEmpty() )
		{
			LogQuery ( tHandler.m_dQueries[i], tHandler.m_dAggrResults[i], dAgentTimes );
			if ( sError.IsEmpty() )
			{
				if ( tHandler.m_dQueries.GetLength()==1 )
					sError = tHandler.m_dAggrResults[0].m_sError;
				else
					sError.SetSprintf ( "query %d error: %s", i, tHandler.m_dAggrResults[i].m_sError.cstr() );
			} else
				sError.SetSprintf ( "%s; query %d error: %s", sError.cstr(), i, tHandler.m_dAggrResults[i].m_sError.cstr() );
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


static void ReturnZeroCount ( const CSphSchema & tSchema, const CSphBitvec & tAttrsToSend, const StrVec_t & dCounts, RowBuffer_i & dRows )
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
				case SPH_ATTR_INTEGER:	dRows.PutNumAsString ( pExpr->IntEval ( tMatch ) ); break;
				case SPH_ATTR_BIGINT:	dRows.PutNumAsString ( pExpr->Int64Eval ( tMatch ) ); break;
				case SPH_ATTR_FLOAT:	dRows.PutFloatAsString ( pExpr->Eval ( tMatch ) ); break;
				default:
					dRows.PutNULL();
					break;
			}
		}
	}
	dRows.Commit();
}


void SendMysqlSelectResult ( RowBuffer_i & dRows, const AggrResult_t & tRes, bool bMoreResultsFollow, bool bAddQueryColumn, const CSphString * pQueryColumn, QueryProfile_c * pProfile )
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
	if ( tRes.GetLength() || bReturnZeroCount )
		sphGetAttrsToSend ( tRes.m_tSchema, false, true, tAttrsToSend );

	// field packets
	if ( tRes.GetLength()==0 && !bReturnZeroCount )
	{
		// in case there are no matches, send a dummy schema
		// result set header packet. We will attach EOF manually at the end.
		dRows.HeadBegin ( bAddQueryColumn ? 2 : 1 );
		dRows.HeadColumn ( "id", MYSQL_COL_LONGLONG );
	} else
	{
		int iAttrsToSend = tAttrsToSend.BitCount();
		if ( bAddQueryColumn )
			++iAttrsToSend;

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
			case SPH_ATTR_BOOL:		eType = MYSQL_COL_LONG; break;
			case SPH_ATTR_FLOAT:	eType = MYSQL_COL_FLOAT; break;
			case SPH_ATTR_DOUBLE:	eType = MYSQL_COL_DOUBLE; break;
			case SPH_ATTR_BIGINT:	eType = MYSQL_COL_LONGLONG; break;
			default: break;
			}
			dRows.HeadColumn ( tCol.m_sName.cstr(), eType );
		}
	}

	if ( bAddQueryColumn )
		dRows.HeadColumn ( "query" );

	// EOF packet is sent explicitly due to non-default params.
	auto iWarns = tRes.m_sWarning.IsEmpty() ? 0 : 1;
	dRows.HeadEnd ( bMoreResultsFollow, iWarns );

	// FIXME!!! replace that vector relocations by SqlRowBuffer

	// rows
	const CSphSchema &tSchema = tRes.m_tSchema;
	assert ( tRes.m_bSingle );
	auto dMatches = tRes.m_dResults.First ().m_dMatches.Slice ( tRes.m_iOffset, tRes.m_iCount );
	for ( const auto& tMatch : dMatches )
	{
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

			case SPH_ATTR_DOUBLE:
				dRows.PutDoubleAsString ( tMatch.GetAttrDouble(tLoc) );
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
					auto dString = sphUnpackPtrAttr ( pString );
					if ( dString.second>1 && dString.first[dString.second-2]=='\0' )
						dString.second -= 2;
					dRows.PutArray ( dString );
				}
				break;
			case SPH_ATTR_JSON_PTR:
				{
					auto * pString = (const BYTE*) tMatch.GetAttr ( tLoc );
					JsonEscapedBuilder sTmp;
					if ( pString )
					{
						auto dJson = sphUnpackPtrAttr ( pString );
						sphJsonFormat ( sTmp, dJson.first );
					}
					dRows.PutArray ( sTmp );
				}
				break;

			case SPH_ATTR_FACTORS:
			case SPH_ATTR_FACTORS_JSON:
				{
					auto dFactors = sphUnpackPtrAttr ((const BYTE *) tMatch.GetAttr ( tLoc ));
					StringBuilder_c sTmp;
					if ( !IsNull ( dFactors ))
						sphFormatFactors ( sTmp, (const unsigned int *)dFactors.first, eAttrType==SPH_ATTR_FACTORS_JSON );
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

					auto dField = sphUnpackPtrAttr ( pField );
					auto eJson = ESphJsonType ( *dField.first++ );

					if ( eJson==JSON_NULL )
					{
						dRows.PutNULL();
						break;
					}

					// send string to client
					JsonEscapedBuilder sTmp;
					sphJsonFieldFormat ( sTmp, dField.first, eJson, false );
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

void HandleMysqlStatus ( RowBuffer_i & dRows, const SqlStmt_t & tStmt, bool bMoreResultsFollow )
{
	VectorLike dStatus ( tStmt.m_sStringParam );

	switch ( tStmt.m_eStmt )
	{
	case STMT_SHOW_STATUS:
		BuildStatus ( dStatus );
		break;
	case STMT_SHOW_AGENT_STATUS:
		BuildAgentStatus ( dStatus, tStmt.m_sIndex );
		break;
	default:
		assert(0); // only 'show' statements allowed here.
		break;
	}

	// result set header packet
	if (!dRows.HeadOfStrings ( dStatus.Header() ))
		return;

	// send rows
	for ( int iRow=0; iRow<dStatus.GetLength(); iRow+=2 )
		if ( !dRows.DataTuplet ( dStatus[iRow+0].cstr (), dStatus[iRow+1].cstr () ) )
			return;

	// cleanup
	dRows.Eof ( bMoreResultsFollow );
}

void HandleMysqlMeta ( RowBuffer_i & dRows, const SqlStmt_t & tStmt, const CSphQueryResultMeta & tLastMeta, bool bMoreResultsFollow )
{
	VectorLike dMeta ( tStmt.m_sStringParam );

	assert ( tStmt.m_eStmt==STMT_SHOW_META );
	BuildMeta ( dMeta, tLastMeta );

	// result set header packet
	if ( !dRows.HeadOfStrings ( dMeta.Header () ) )
		return;

	// send rows
	for ( int iRow=0; iRow<dMeta.GetLength(); iRow+=2 )
		if ( !dRows.DataTuplet ( dMeta[iRow+0].cstr (), dMeta[iRow+1].cstr () ) )
			return;

	// cleanup
	dRows.Eof ( bMoreResultsFollow );
}

static std::unique_ptr<ReplicationCommand_t> MakePercolateDeleteDocumentsCommand ( CSphString sIndex, CSphString sCluster, const SqlStmt_t & tStmt, CSphString & sError )
{
	// prohibit double copy of filters
	const CSphQuery& tQuery = tStmt.m_tQuery;
	if ( tQuery.m_dFilters.IsEmpty() || tQuery.m_dFilters.GetLength() > 1 )
	{
		sError.SetSprintf ( "only single filter supported, got %d", tQuery.m_dFilters.GetLength() );
		return nullptr;
	}

	const CSphFilterSettings* pFilter = tQuery.m_dFilters.Begin();

	auto pCmd = MakeReplicationCommand ( ReplicationCommand_e::PQUERY_DELETE, std::move ( sIndex ), std::move ( sCluster ) );
	if ( ( pFilter->m_bHasEqualMin || pFilter->m_bHasEqualMax ) && !pFilter->m_bExclude && pFilter->m_eType==SPH_FILTER_VALUES && ( pFilter->m_sAttrName=="@id" || pFilter->m_sAttrName=="id" || pFilter->m_sAttrName=="uid" ) )
	{
		pCmd->m_dDeleteQueries.Reserve ( pFilter->GetNumValues() );
		const SphAttr_t * pA = pFilter->GetValueArray();
		for ( int i = 0; i < pFilter->GetNumValues(); ++i )
			pCmd->m_dDeleteQueries.Add ( pA[i] );
		return pCmd;
	}

	if ( pFilter->m_eType==SPH_FILTER_STRING && pFilter->m_sAttrName=="tags" && !pFilter->m_dStrings.IsEmpty() )
	{
		pCmd->m_sDeleteTags = pFilter->m_dStrings[0];
		return pCmd;
	}

	if ( pFilter->m_eType==SPH_FILTER_STRING_LIST && pFilter->m_sAttrName=="tags" && !pFilter->m_dStrings.IsEmpty() )
	{
		StringBuilder_c tBuf ( "," );
		pFilter->m_dStrings.for_each ( [&tBuf] ( const auto& sVal ) { tBuf << sVal; } );
		tBuf.FinishBlocks ();
		tBuf.MoveTo ( pCmd->m_sDeleteTags );
		return pCmd;
	}

	sError.SetSprintf ( "unsupported filter type %d, attribute '%s'", pFilter->m_eType, pFilter->m_sAttrName.cstr() );
	return nullptr;
}


static int LocalIndexDoDeleteDocuments ( const CSphString & sName, const char * sDistributed, const SqlStmt_t & tStmt,
		SearchFailuresLog_c & dErrors, bool bCommit, CSphSessionAccum & tAcc )
{

	const CSphString & sCluster = tStmt.m_sCluster;
	const CSphString & sStore = tStmt.m_tQuery.m_sStore;
	bool bOnlyStoreDocIDs = !sStore.IsEmpty();

	CSphString sError;
	auto err = [&sName, &sDistributed, &sError, &dErrors] (const char* szErr = nullptr)
	{
		dErrors.Submit ( sName, sDistributed, szErr ? szErr : sError.cstr() );
		return 0;
	};

	cServedIndexRefPtr_c pServed { GetServed ( sName ) };
	if ( !ServedDesc_t::IsMutable ( pServed ) )
		return err ( "index not available, or does not support DELETE" );

	GlobalCrashQueryGetRef().m_dIndex = FromStr ( sName );
	if ( !CheckIndexCluster ( sName, *pServed, sCluster, IsHttpStmt ( tStmt ), sError ) )
		return err();

	// process store to local variable instead of deletion (here we don't need any stuff like accum, txn, replication)
	if ( bOnlyStoreDocIDs )
	{
		if ( pServed->m_eType == IndexType_e::PERCOLATE )
			return err ( "Storing del subset not implemented for PQ indexes" );

		assert ( sStore.Begins ( "@" ) );
		DocsCollector_c dData { tStmt.m_tQuery, tStmt.m_bJson, sName, pServed, &sError };
		auto dDocs = dData.GetValuesSlice();
		if ( !sError.IsEmpty() )
			return err();

		SetLocalTemporaryUserVar ( sStore, dDocs );
		return 0;
	}


	RtAccum_t* pAccum = nullptr;

	// goto to percolate path with unlocked index
	if ( pServed->m_eType==IndexType_e::PERCOLATE )
	{
		auto pCmd = MakePercolateDeleteDocumentsCommand ( sName, sCluster, tStmt, sError );
		if ( !sError.IsEmpty() )
			return err();

		if ( !pCmd )
			return 0;

		pAccum = tAcc.GetAcc ( RIdx_T<RtIndex_i*> ( pServed ), sError );
		if ( !sError.IsEmpty() )
			return err();

		assert ( pAccum );
		pAccum->m_dCmd.Add ( std::move ( pCmd ) );
	} else
	{
		DocsCollector_c dData { tStmt.m_tQuery, tStmt.m_bJson, sName, pServed, &sError};
		auto dDocs = dData.GetValuesSlice();
		if ( !sError.IsEmpty() )
			return err();

		RIdx_T<RtIndex_i*> pRtIndex { pServed };
		pAccum = tAcc.GetAcc ( pRtIndex, sError );
		if ( !pRtIndex->DeleteDocument ( dDocs, sError, pAccum ) ) // assume dData is alive, as we use slice from internal vec
			return err();

		pAccum = tAcc.GetAcc ( pRtIndex, sError );
		if ( !sError.IsEmpty() )
			return err();

		assert ( pAccum );
		pAccum->AddCommand ( ReplicationCommand_e::RT_TRX, sName, sCluster );
	}

	int iAffected = 0;
	if ( bCommit )
	{
		if ( !HandleCmdReplicate ( *pAccum, sError, iAffected ) )
		{
			dErrors.Submit ( sName, sDistributed, sError.cstr() );
			return 0;
		}
	}

	return iAffected;
}


void sphHandleMysqlDelete ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt, Str_t sQuery )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	auto* pSession = session::GetClientSession();
	pSession->FreezeLastMeta();
	bool bCommit = pSession->m_bAutoCommit && !pSession->m_bInTransaction;
	auto& tAcc = pSession->m_tAcc;
	StatCountCommand ( SEARCHD_COMMAND_DELETE );

	MEMORY ( MEM_SQL_DELETE );

	// shortcut
	const CSphQuery & tQuery = tStmt.m_tQuery;
	const CSphString & sStorevar = tQuery.m_sStore;
	bool bStoreVar = !sStorevar.IsEmpty();

	if ( bStoreVar && !sStorevar.Begins("@") )
	{
		tOut.Error ( "store var name must start with @, '%s' given", sStorevar.cstr() );
		return;
	}

	StrVec_t dNames;
	ParseIndexList ( tStmt.m_sIndex, dNames );
	if ( dNames.IsEmpty() )
	{
		tOut.Error ( "no such index '%s'", tStmt.m_sIndex.cstr () );
		return;
	}

	DistrPtrs_t dDistributed;
	CSphString sMissed;
	if ( !ExtractDistributedIndexes ( dNames, dDistributed, sMissed ) )
	{
		tOut.Error ( "unknown index '%s' in delete request", sMissed.cstr () );
		return;
	}

	// delete to agents works only with commit=1
	if ( !bCommit  )
	{
		for ( auto &pDist : dDistributed )
		{
			if ( !pDist || pDist->m_dAgents.IsEmpty() )
				continue;

			tOut.Error ( "index '%s': DELETE is not supported on agents when autocommit=0", tStmt.m_sIndex.cstr() );
			return;
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
			iAffected += LocalIndexDoDeleteDocuments ( sName, nullptr, tStmt, dErrors, bCommit, tAcc );
		}
		else if ( dDistributed[iIdx] )
		{
			assert ( !dDistributed[iIdx]->IsEmpty() );
			for ( const CSphString& sLocal : dDistributed[iIdx]->m_dLocal )
			{
				bool bDistLocal = g_pLocalIndexes->Contains ( sLocal );
				if ( bDistLocal )
				{
					iAffected += LocalIndexDoDeleteDocuments ( sLocal, sName.cstr(), tStmt, dErrors, bCommit, tAcc );
				}
			}
		}

		// delete for remote agents
		if ( !bStoreVar && dDistributed[iIdx] && !dDistributed[iIdx]->m_dAgents.IsEmpty() )
		{
			const DistributedIndex_t * pDist = dDistributed[iIdx];
			VecRefPtrsAgentConn_t dAgents;
			pDist->GetAllHosts ( dAgents );

			int iGot = 0;
			int iWarns = 0;

			// connect to remote agents and query them
			std::unique_ptr<RequestBuilder_i> pRequestBuilder = CreateRequestBuilder ( sQuery, tStmt );
			std::unique_ptr<ReplyParser_i> pReplyParser = CreateReplyParser ( tStmt.m_bJson, iGot, iWarns );
			PerformRemoteTasks ( dAgents, pRequestBuilder.get (), pReplyParser.get () );

			// FIXME!!! report error & warnings from agents
			// FIXME? profile update time too?
			iAffected += iGot;
		}
	}

	if ( !dErrors.IsEmpty() )
	{
		StringBuilder_c sReport;
		dErrors.BuildReport ( sReport );
		tOut.Error ( "%s", sReport.cstr () );
		return;
	}

	tOut.Ok ( iAffected );
}

struct SessionVars_t
{
	bool			m_bAutoCommit = true;
	bool			m_bInTransaction = false;
	CSphVector<int64_t> m_dLastIds;
};

// fwd
void HandleMysqlShowProfile ( RowBuffer_i & tOut, const QueryProfile_c & p, bool bMoreResultsFollow );

static void HandleMysqlShowPlan ( RowBuffer_i & tOut, const QueryProfile_c & p, bool bMoreResultsFollow, bool bDot );

bool IsDot ( const SqlStmt_t & tStmt )
{
	if ( tStmt.m_sThreadFormat=="dot" )
		return true;
	else if ( tStmt.m_sThreadFormat=="plain" )
		return false;
	return session::IsDot();
}

Profile_e ParseProfileFormat ( const SqlStmt_t & tStmt )
{
	if ( tStmt.m_sSetValue=="dot" )
		return Profile_e::DOT;
	else if ( tStmt.m_sSetValue=="expr" )
		return Profile_e::DOTEXPR;
	else if ( tStmt.m_sSetValue=="exprurl" )
		return Profile_e::DOTEXPRURL;
	else if ( tStmt.m_iSetValue!=0 )
		return Profile_e::PLAIN;
	return Profile_e::NONE;
}

void HandleMysqlMultiStmt ( const CSphVector<SqlStmt_t> & dStmt, CSphQueryResultMeta & tLastMeta, RowBuffer_i & dRows,
		const CSphString & sWarning )
{
	auto& tSess = session::Info();

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
	QueryProfile_c tProfile;

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
			if ( sSetName=="profiling" )
				tSess.SetProfile ( ParseProfileFormat ( dStmt[i] ) );
		}
	}

	// use first meta for faceted search
	bool bUseFirstMeta = ( tHandler.m_dQueries.GetLength()>1 && !tHandler.m_dQueries[0].m_bFacet && tHandler.m_dQueries[1].m_bFacet );
	if ( tSess.IsProfile() )
		tHandler.SetProfile ( &tProfile );

	// do search
	bool bSearchOK = true;
	if ( iSelect )
	{
		bSearchOK = HandleMysqlSelect ( dRows, tHandler );

		// save meta for SHOW *
		if ( bUseFirstMeta )
		{
			tLastMeta = tHandler.m_dAggrResults.First();
			// fix up overall query time
			for ( auto& tResult : tHandler.m_dAggrResults )
			{
				tLastMeta.m_iQueryTime += tResult.m_iQueryTime;
				tLastMeta.m_iCpuTime += tResult.m_iCpuTime;
				tLastMeta.m_iAgentCpuTime += tResult.m_iAgentCpuTime;
			}
		} else
			tLastMeta = tHandler.m_dAggrResults.Last();
	}

	if ( !bSearchOK )
		return;

	// send multi-result set
	iSelect = 0;
	ARRAY_FOREACH ( i, dStmt )
	{
		SqlStmt_e eStmt = dStmt[i].m_eStmt;
		myinfo::SetCommand ( g_dSqlStmts[eStmt] );

		const CSphQueryResultMeta & tMeta = bUseFirstMeta ? tHandler.m_dAggrResults[0] : ( iSelect-1>=0 ? tHandler.m_dAggrResults[iSelect-1] : tPrevMeta );
		bool bMoreResultsFollow = (i+1)<dStmt.GetLength();
		bool bBreak = false;

		switch ( eStmt )
		{
		case STMT_SELECT:
		{
			AggrResult_t & tRes = tHandler.m_dAggrResults[iSelect++];
			if ( !sWarning.IsEmpty() )
				tRes.m_sWarning = sWarning;
			SendMysqlSelectResult ( dRows, tRes, bMoreResultsFollow, false, nullptr, ( tSess.IsProfile() ? &tProfile : nullptr ) );
			// mysql server breaks send on error
			bBreak = !tRes.m_iSuccesses;
			break;
		}
		case STMT_SHOW_WARNINGS:
			HandleMysqlWarning ( tMeta, dRows, bMoreResultsFollow );
			break;
		case STMT_SHOW_STATUS:
		case STMT_SHOW_AGENT_STATUS:
			HandleMysqlStatus ( dRows, dStmt[i], bMoreResultsFollow ); // FIXME!!! add prediction counters
			break;
		case STMT_SHOW_META:
			HandleMysqlMeta ( dRows, dStmt[i], tMeta, bMoreResultsFollow ); // FIXME!!! add prediction counters
			break;
		case STMT_SET: // TODO implement all set statements and make them handle bMoreResultsFollow flag
			dRows.Ok ( 0, 0, NULL, bMoreResultsFollow );
			break;
		case STMT_SHOW_PROFILE:
			HandleMysqlShowProfile ( dRows, tProfile, bMoreResultsFollow );
			break;
		case STMT_SHOW_PLAN:
			HandleMysqlShowPlan ( dRows, tProfile, bMoreResultsFollow, ::IsDot ( dStmt[i] ) );
		default:
			break;
		}

		if ( bBreak )
			break;

		if ( sphInterrupted() )
		{
			sphLogDebug ( "HandleMultiStmt: got SIGTERM, sending the packet MYSQL_ERR_SERVER_SHUTDOWN" );
			dRows.Error ( NULL, "Server shutdown in progress", MYSQL_ERR_SERVER_SHUTDOWN );
			return;
		}
	}
}

void HandleMysqlSet ( RowBuffer_i & tOut, SqlStmt_t & tStmt, CSphSessionAccum & tAcc )
{
	auto& tSess = session::Info();
	MEMORY ( MEM_SQL_SET );
	CSphString sError;

	tStmt.m_sSetName.ToLower();
	switch ( tStmt.m_eSet )
	{
	case SET_LOCAL: // SET foo = value|'svalue'|null

		if ( tStmt.m_sSetName=="wait_timeout" )
		{
			tSess.SetTimeoutS ( tStmt.m_iSetValue );
			break;
		}

		if ( tStmt.m_sSetName=="throttling_period" )
		{
			tSess.SetThrottlingPeriodMS( tStmt.m_iSetValue );
			break;
		}

		if ( tStmt.m_sSetName == "optimize_by_id" )
		{
			tSess.SetOptimizeById ( !!tStmt.m_iSetValue );
			break;
		}

		if ( tStmt.m_sSetName=="max_threads_per_query" )
		{
			tSess.SetDistThreads ( tStmt.m_iSetValue );
			break;
		}

		if ( tStmt.m_sSetName == "ro" )
		{
			if ( !tSess.GetVip() )
			{
				if (!sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
					return;
			}
			tSess.SetReadOnly ( !!tStmt.m_iSetValue );
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
			bool bAutoCommit = ( tStmt.m_iSetValue != 0 );
			auto pSession = session::Info().GetClientSession();
			pSession->m_bAutoCommit = ( tStmt.m_iSetValue!=0 );
			pSession->m_bInTransaction = false;

			// commit all pending changes
			if ( bAutoCommit )
			{
				RtIndex_i * pIndex = tAcc.GetIndex();
				if ( pIndex )
				{
					RtAccum_t * pAccum = tAcc.GetAcc ( pIndex, sError );
					if ( !sError.IsEmpty() || !HandleCmdReplicate ( *pAccum, sError ) )
					{
						tOut.Error ( tStmt.m_sStmt, sError.cstr() );
						return;
					}
				}
			}
		} else if ( tStmt.m_sSetName=="collation_connection" )
		{
			// per-session COLLATION_CONNECTION
			CSphString & sVal = tStmt.m_sSetValue;
			sVal.ToLower();

			tSess.SetCollation ( sphCollationFromName ( sVal, &sError ) );
			if ( !sError.IsEmpty() )
			{
				tOut.Error ( tStmt.m_sStmt, sError.cstr() );
				return;
			}
		} else if ( tStmt.m_sSetName=="character_set_results"
			|| tStmt.m_sSetName=="sql_auto_is_null"
			|| tStmt.m_sSetName=="sql_safe_updates"
			|| tStmt.m_sSetName=="sql_mode"
			|| tStmt.m_sSetName=="time_zone" )
		{
			// per-session CHARACTER_SET_RESULTS et al; just ignore for now

		} else if ( tStmt.m_sSetName=="profiling" )
		{
			// per-session PROFILING
			tSess.SetProfile ( ParseProfileFormat ( tStmt ) );

		} else
		{
			// unknown variable, return error
			tOut.ErrorEx ( tStmt.m_sStmt, "Unknown session variable '%s' in SET statement", tStmt.m_sSetName.cstr () );
			return;
		}
		break;

	case SET_GLOBAL_UVAR: // SET GLOBAL @foo = (i1,i2,...)'
	{
		// global user variable
		// INT_SET type must be sorted
		tStmt.m_dSetValues.Sort();
		SetLocalUserVar ( tStmt.m_sSetName, tStmt.m_dSetValues );
		break;
	}

	case SET_GLOBAL_SVAR: // SET GLOBAL foo = iValue|'string'

		if ( !tSess.GetVip() && !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
			return;
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
			SetGroupingInUtcExpr ( g_bGroupingInUtc );
			SetGroupingInUtcSort ( g_bGroupingInUtc );
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
			if ( tSess.GetVip() )
				g_bMaintenance = !!tStmt.m_iSetValue;
			else
			{
				tOut.Error ( tStmt.m_sStmt, "Only VIP connections can set maintenance mode" );
				return;
			}
		} else if ( tStmt.m_sSetName=="wait_timeout" )
		{
			if ( tSess.GetVip() )
				g_iClientQlTimeoutS = tStmt.m_iSetValue;
			else
			{
				tOut.Error ( tStmt.m_sStmt, "Only VIP connections can change global wait_timeout value" );
				return;
			}
		} else if ( tStmt.m_sSetName=="throttling_period" )
		{
			if ( tSess.GetVip() )
				Threads::Coro::Throttler_c::SetDefaultThrottlingPeriodMS ( tStmt.m_iSetValue );
			else
			{
				tOut.Error ( tStmt.m_sStmt, "Only VIP connections can change global throttling_period value" );
				return;
			}
		} else if ( tStmt.m_sSetName=="max_threads_per_query" )
		{
			g_iDistThreads = tStmt.m_iSetValue; // that is not dangerous to allow everybody change the value
		} else if ( tStmt.m_sSetName=="auto_optimize")
		{
			if ( !AUTOOPTIMIZE_NEEDS_VIP || tSess.GetVip() )
				g_iAutoOptimizeCutoffMultiplier = tStmt.m_iSetValue;
			else
			{
				tOut.Error ( tStmt.m_sStmt, "Only VIP connections can change global auto_optimize value" );
				return;
			}
		} else if ( tStmt.m_sSetName=="optimize_cutoff")
		{
			if ( tStmt.m_iSetValue<1 )
			{
				tOut.ErrorEx ( tStmt.m_sStmt, "optimize_cutoff should be greater than 0, got %d", tStmt.m_iSetValue );
				return;
			}

			MutableIndexSettings_c::GetDefaults().m_iOptimizeCutoff = tStmt.m_iSetValue;

		} else if ( tStmt.m_sSetName=="pseudo_sharding")
		{
			g_bSplit = !!tStmt.m_iSetValue;
		} else if ( tStmt.m_sSetName=="secondary_indexes" )
		{
			SetSecondaryIndexDefault ( !!tStmt.m_iSetValue );

		} else {
			tOut.ErrorEx ( tStmt.m_sStmt, "Unknown system variable '%s'", tStmt.m_sSetName.cstr () );
			return;
		}
		break;

	case SET_INDEX_UVAR: //  SET INDEX bar GLOBAL @foo = (values)
		if ( !SendUserVar ( tStmt.m_sIndex.cstr(), tStmt.m_sSetName.cstr(), tStmt.m_dSetValues, sError ) )
		{
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
		break;

	case SET_CLUSTER_UVAR: // SET CLUSTER ident GLOBAL 'variable' = string|int
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


void HandleMysqlAttach ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	if ( !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
		return;

	const CSphString & sFrom = tStmt.m_sIndex;
	const CSphString & sTo = tStmt.m_sStringParam;
	bool bTruncate = ( tStmt.m_iIntParam==1 );
	CSphString sError;

	auto pServedFrom = GetServed ( sFrom );
	auto pServedTo = GetServed ( sTo );

	bool bOk = false;
	if ( !pServedFrom )
		tOut.ErrorEx ( nullptr, "no such index '%s'", sFrom.cstr() );
	else if ( pServedFrom->m_eType != IndexType_e::PLAIN )
		tOut.Error ( tStmt.m_sStmt, "1st argument to ATTACH must be a plain index" );
	else if ( !pServedTo )
		tOut.ErrorEx ( nullptr, "no such index '%s'", sTo.cstr() );
	else if ( pServedTo->m_eType!=IndexType_e::RT )
		tOut.Error ( tStmt.m_sStmt, "2nd argument to ATTACH must be a RT index" );
	else
		bOk = true;
	if (!bOk)
		return;

	// cluster does not implement ATTACH for now
	auto tCluster = IsPartOfCluster ( pServedTo );
	if ( tCluster )
	{
		tOut.ErrorEx ( nullptr, "index %s is part of cluster %s, can not issue ATTACH", sTo.cstr(), tCluster->cstr(), sError.cstr () );
		return;
	}

	WIdx_T<RtIndex_i*> pRtTo { pServedTo };
	WIdx_c pPlainFrom { pServedFrom };

	bool bFatal = false;
	StrVec_t dWarnings;
	auto bAttached = pRtTo->AttachDiskIndex ( pPlainFrom, bTruncate, bFatal, dWarnings, sError );

	sWarning = ConcatWarnings(dWarnings);
	if ( bAttached || bFatal )
		g_pLocalIndexes->Delete ( sFrom );

	if ( bAttached )
	{
		pServedFrom->ReleaseIdx(); // since index no more belong to us
		tOut.Ok();
	}
	else
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
}


void HandleMysqlFlushRtindex ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	auto pIndex = GetServed ( tStmt.m_sIndex );

	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tStmt.m_sStmt, "FLUSH RTINDEX requires an existing RT index" );
		return;
	}

	RIdx_T<RtIndex_i*> ( pIndex )->ForceRamFlush ( "forced" );
	tOut.Ok();
}


void HandleMysqlFlushRamchunk ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	auto pIndex = GetServed ( tStmt.m_sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tStmt.m_sStmt, "FLUSH RAMCHUNK requires an existing RT index" );
		return;
	}

	RIdx_T<RtIndex_i*> pRt { pIndex };
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

// stuff for command 'debug', isolated
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

void HandleMysqlOptimizeManual ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	if ( !sphCheckWeCanModify ( "optimize", tOut ) )
		return;

	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tCmd.m_szStmt, "MERGE requires an existing RT index" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eMerge;
	tTask.m_iFrom = (int)tCmd.m_iPar1;
	tTask.m_iTo = (int)tCmd.m_iPar2;
	tTask.m_bByOrder = !tCmd.bOpt ( "byid", session::GetOptimizeById() );
	tTask.m_iCutoff = (int)tCmd.iOpt("cutoff");
	tTask.m_sIndex = std::move (sIndex);

	if ( tCmd.bOpt ( "sync" ) )
		RIdx_T<RtIndex_i*> ( pIndex )->Optimize ( std::move ( tTask ) );
	else
		RunOptimizeRtIndex ( std::move ( tTask ) );
	tOut.Ok();
}

// command 'drop [chunk] X [from] <IDX> [option...]'
void HandleMysqlDropManual ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	if ( !sphCheckWeCanModify ( "drop", tOut ) )
		return;

	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tCmd.m_szStmt, "DROP requires an existing RT index" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eDrop;
	tTask.m_iFrom = (int)tCmd.m_iPar1;
	tTask.m_bByOrder = !tCmd.bOpt ( "byid", session::GetOptimizeById() );
	tTask.m_sIndex = std::move ( sIndex );

	if ( tCmd.bOpt ( "sync" ) )
		RIdx_T<RtIndex_i*> ( pIndex )->Optimize ( std::move ( tTask ) );
	else
		RunOptimizeRtIndex ( std::move ( tTask ) );
	tOut.Ok();
}

void HandleMysqlCompress ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	if ( !sphCheckWeCanModify ( "compress", tOut ) )
		return;

	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tCmd.m_szStmt, "COMPRESS requires an existing RT index" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eCompress;
	tTask.m_iFrom = (int) tCmd.m_iPar1;
	tTask.m_bByOrder = !tCmd.bOpt ( "byid", session::GetOptimizeById() );
	tTask.m_sIndex = std::move ( sIndex );

	if ( tCmd.bOpt ( "sync" ) )
		RIdx_T<RtIndex_i*> ( pIndex )->Optimize ( std::move ( tTask ) );
	else
		RunOptimizeRtIndex ( std::move ( tTask ) );
	tOut.Ok();
}

// command 'split <IDX> [chunk] N on @uservar [option...]'
// IDX is tCmd.m_sParam
// chunk is tCmd.m_iPar1
// uservar is tCmd.m_sParam2
void HandleMysqlSplit ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	if ( !sphCheckWeCanModify ( "split", tOut ) )
		return;

	// check index existance
	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tCmd.m_szStmt, "SPLIT requires an existing RT index" );
		return;
	}

	bool bVarFound = false;
	IterateUservars ( [&tCmd, &bVarFound] ( const NamedRefVectorPair_t & dVar ) {
		if ( dVar.first == tCmd.m_sParam2
//			&& dVar.second.m_eType==USERVAR_INT_SET_TMP // uncomment this to split only by session (result of delete .. store) variables
		)
			bVarFound = true;
	} );

	if ( !bVarFound )
	{
		tOut.Error ( tCmd.m_szStmt, "SPLIT requires an existing session @uservar" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eSplit;
	tTask.m_iFrom = (int)tCmd.m_iPar1;
	tTask.m_sUvarFilter = tCmd.m_sParam2;
	tTask.m_bByOrder = !tCmd.bOpt ( "byid", session::GetOptimizeById() );
	tTask.m_sIndex = std::move ( sIndex );

	if ( tCmd.bOpt ( "sync" ) )
		RIdx_T<RtIndex_i*> ( pIndex )->Optimize ( std::move ( tTask ) );
	else
		RunOptimizeRtIndex ( std::move ( tTask ) );
	tOut.Ok();
}

void HandleMysqlfiles ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsLocal ( pIndex ) )
	{
		tOut.Error ( tCmd.m_szStmt, "FILES requires an existing local index" );
		return;
	}

	StrVec_t dFiles;
	StrVec_t dExt;
	RIdx_c ( pIndex )->GetIndexFiles ( dFiles, dExt );

	VectorLike dOut ( 0 );
	dOut.SetColNames ( { "file" } );

	auto sFormat = tCmd.sOpt ( "format" );
	if ( sFormat!="external" )
		dFiles.Apply ( [&dOut] ( const CSphString & a ) { dOut.Add ( a ); } );

	if ( sFormat=="all" || sFormat=="external" )
	{
		dExt.Uniq ();
		dExt.Apply ( [&dOut] ( const CSphString & a ) { dOut.Add ( a ); } );
	}

	tOut.DataTable ( dOut );
}

void HandleMysqlclose ( RowBuffer_i & tOut )
{
	tOut.HeadTuplet ( "command", "result" );
	tOut.DataTuplet ( "Close", "SUCCESS" );
	tOut.Eof ();

	DebugClose();
}

// same for select ... from index.files
void HandleSelectFiles ( RowBuffer_i & tOut, const SqlStmt_t * pStmt )
{
	tOut.HeadBegin ( 3 );
	tOut.HeadColumn ( "file" );
	tOut.HeadColumn ( "normalized" );
	tOut.HeadColumn ( "size", MYSQL_COL_LONGLONG );
	if ( !tOut.HeadEnd () )
		return;

	const auto & tStmt = *pStmt;
	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( !ServedDesc_t::IsLocal ( pServed ) )
	{
		tOut.Error ( tStmt.m_sStmt, "FILES requires an existing local index" );
		return;
	}

	StrVec_t dFiles;
	StrVec_t dExt;
	RIdx_c ( pServed )->GetIndexFiles ( dFiles, dExt );

	auto sFormat = tStmt.m_sThreadFormat;
	if ( sFormat!="external" )
		ARRAY_CONSTFOREACH( i, dFiles )
		{
			tOut.PutString ( dFiles[i] );
			tOut.PutString ( RealPath ( dFiles[i] ) );
			tOut.PutNumAsString ( sphGetFileSize ( dFiles[i], nullptr ) );
			if ( !tOut.Commit () )
				return;
		}

	if ( sFormat=="all" || sFormat=="external" )
	{
		dExt.Uniq ();
		ARRAY_CONSTFOREACH( i, dExt )
		{
			tOut.PutString ( dExt[i] );
			tOut.PutString ( RealPath ( dExt[i] ) );
			tOut.PutNumAsString ( sphGetFileSize ( dExt[i], nullptr ) );
			if ( !tOut.Commit () )
				return;
		}
	}
	tOut.Eof();
}

void HandleShutdownCrash ( RowBuffer_i & tOut, const CSphString & sPasswd, DebugCmd::Cmd_e eCmd )
{
	const char * szCmd = DebugCmd::dCommands[(BYTE) eCmd].m_szExample;
	if ( g_sShutdownToken.IsEmpty () )
	{
		tOut.Error ( szCmd, "shutdown_token is empty. Provide it in searchd config section." );
		return;
	}

	if ( strSHA1 ( sPasswd )!=g_sShutdownToken )
	{
		tOut.Error ( szCmd, "FAIL" );
		return;
	}

	tOut.HeadTuplet ( "command", "result" );
	tOut.DataTuplet ( szCmd, "SUCCESS" );
	tOut.Eof ();
	if ( eCmd==DebugCmd::Cmd_e::SHUTDOWN )
	{
#if _WIN32
		sigterm(1);
#else
		kill ( 0, SIGTERM );
#endif
	} else // crash
	{
		BYTE * pSegv = (BYTE *) ( 0 );
		*pSegv = 'a';
	}
}

#if !_WIN32
void HandleProcDump ( RowBuffer_i & tOut )
{
	tOut.HeadTuplet ( "command", "result" );
	if ( g_iParentPID<=0 )
		tOut.DataTuplet ( "procdump", "Unavailable (no watchdog)" );
	else
	{
		kill ( g_iParentPID, SIGUSR1 );
		tOut.DataTupletf ( "procdump", "Sent USR1 to wathcdog (%d)", g_iParentPID );
	}
	tOut.Eof ();
}

void HandleGdbStatus ( RowBuffer_i & tOut )
{
	tOut.HeadTuplet ( "command", "result" );
	const auto & g_bSafeGDB = getSafeGDB ();
	if ( g_iParentPID>0 )
		tOut.DataTupletf ( "setgdb", "Enabled, managed by watchdog (pid=%d)", g_iParentPID );
	else if ( g_bSafeGDB )
		tOut.DataTupletf ( "setgdb", "Enabled, managed locally because of jemalloc", g_iParentPID );
	else if ( g_iParentPID==-1 )
		tOut.DataTuplet ( "setgdb", "Enabled locally, MAY HANG!" );
	else
		tOut.DataTuplet ( "setgdb", "Disabled" );
	tOut.Eof ();
}

void HandleSetGdb ( RowBuffer_i & tOut, bool bParam )
{
	tOut.HeadTuplet ( "command", "result" );
	const auto & g_bSafeGDB = getSafeGDB ();

	if ( g_iParentPID>0 )
		tOut.DataTupletf ( "setgdb", "Enabled by watchdog (pid=%d)", g_iParentPID );
	else if ( g_bSafeGDB )
		tOut.DataTuplet ( "setgdb", "Enabled locally because of jemalloc" );
	else if ( bParam )
	{
		g_iParentPID = -1;
		tOut.DataTuplet ( "setgdb", "Ok, enabled locally, MAY HANG!" );
	} else if ( !bParam )
	{
		g_iParentPID = 0;
		tOut.DataTuplet ( "setgdb", "Ok, disabled" );
	}
	tOut.Eof ();
}

void HandleWait ( RowBuffer_i& tOutBuf, const DebugCmd::DebugCommand_t& tCmd )
{
	auto iTimeoutS = tCmd.iOpt ( "timeout" );
	auto sCluster = tCmd.m_sParam;
	auto iTime = -sphMicroTimer();
	auto sState = WaitClusterReady ( sCluster, iTimeoutS );
	iTime += sphMicroTimer();
	VectorLike tOut { tCmd.sOpt ( "like" ) };
	tOut.SetColName("name");
	tOut.MatchTuplet ( "cluster", sCluster.cstr() );
	tOut.MatchTuplet ( "state", sState.cstr() );
	tOut.MatchTupletf ( "time", "%.2t", iTime );
	tOutBuf.DataTable ( tOut );
}

void HandleWaitStatus ( RowBuffer_i& tOutBuf, const DebugCmd::DebugCommand_t& tCmd )
{
	auto iTimeoutS = tCmd.iOpt ( "timeout" );
	auto sCluster = tCmd.m_sParam;
	auto iTxn = (int)tCmd.m_iPar1;
	auto iTime = -sphMicroTimer();
	auto tAchieved = WaitClusterCommit ( sCluster, iTxn, iTimeoutS );
	iTime += sphMicroTimer();
	VectorLike tOut { tCmd.sOpt ( "like" ) };
	tOut.SetColName ( "name" );
	tOut.MatchTuplet ( "cluster", sCluster.cstr() );
	tOut.MatchTupletf ( "wanted", "%d", iTxn );
	if ( tAchieved.first>=0 )
		tOut.MatchTupletf ( "state", "%d", tAchieved.first );
	else
		tOut.MatchTuplet ( "achieved", tAchieved.second.cstr() );
	tOut.MatchTupletf ( "time", "%.2t", iTime );
	tOutBuf.DataTable ( tOut );
}
#endif

void HandleToken ( RowBuffer_i & tOut, const CSphString & sParam )
{
	auto sSha = strSHA1 ( sParam );
	tOut.HeadTuplet ( "command", "result" );
	tOut.DataTuplet ( "debug token", sSha.cstr () );
	tOut.Eof ();
}

#if HAVE_MALLOC_STATS
void HandleMallocStats ( RowBuffer_i & tOut, const CSphString& sParam )
{
	tOut.HeadTuplet ( "command", "result" );
	// check where is stderr...
	int iOldErr = ::dup ( STDERR_FILENO );
	::dup2 ( GetLogFD (), STDERR_FILENO );
	sphMallocStats ( sParam.cstr() );
	::close ( STDERR_FILENO );
	::dup2 ( iOldErr, STDERR_FILENO );
	::close ( iOldErr );
	tOut.DataTuplet ( "malloc_stats", g_sLogFile.cstr () );
	tOut.Eof ();
}
#endif

#if HAVE_MALLOC_TRIM
void HandleMallocTrim ( RowBuffer_i & tOut )
{
	tOut.HeadTuplet ( "command", "result" );
	CSphString sResult;
	sResult.SetSprintf ( "%d", PerformMallocTrim ( 0 ) );
	tOut.DataTuplet ( "malloc_trim", sResult.cstr () );
	tOut.Eof ();
}
#endif

void HandleSleep ( RowBuffer_i & tOut, int64_t iParam )
{
	int64_t tmStart = sphMicroTimer ();
	Threads::Coro::SleepMsec ( Max ( iParam/1000, 1 ) );
	int64_t tmDelta = sphMicroTimer ()-tmStart;

	tOut.HeadTuplet ( "command", "result" );
	CSphString sResult;
	sResult.SetSprintf ( "%.3f", (float) tmDelta / 1000000.0f );
	tOut.DataTuplet ( "sleep", sResult.cstr () );
	tOut.Eof ();
}

void HandleTasks ( RowBuffer_i & tOut )
{
	if (!tOut.HeadOfStrings ( { "Name", "MaxRunners", "CurrentRunners", "TotalSpent", "LastFinished", "Executed", "Dropped", "Enqueued" } ))
		return;

	auto dTasks = TaskManager::GetTaskInfo ();
	for ( const auto & dTask : dTasks )
	{
		tOut.PutString ( dTask.m_sName );
		if ( dTask.m_iMaxRunners > 0 )
			tOut.PutNumAsString ( dTask.m_iMaxRunners );
		else
			tOut.PutString ( "unlimited" );
		tOut.PutNumAsString ( dTask.m_iCurrentRunners );
		tOut.PutTimeAsString ( dTask.m_iTotalSpent );
		tOut.PutTimestampAsString ( dTask.m_iLastFinished );
		tOut.PutNumAsString ( dTask.m_iTotalRun );
		tOut.PutNumAsString ( dTask.m_iTotalDropped );
		tOut.PutNumAsString ( dTask.m_iAllRunners );
		if ( !tOut.Commit () )
			return;
	}
	tOut.Eof ();
}

void HandleSched ( RowBuffer_i & tOut )
{
	if (!tOut.HeadOfStrings ( { "Time rest", "Task" } ))
		return;
	auto dTasks = sph::GetSchedInfo ();
	for ( auto& dTask : dTasks )
	{
		tOut.PutTimestampAsString ( dTask.m_iTimeoutStamp );
		tOut.PutString ( dTask.m_sTask );
		if (!tOut.Commit ())
			return;
	}
	tOut.Eof ();
}

void HandleMysqlDebug ( RowBuffer_i &tOut, Str_t sCommand )
{
	using namespace DebugCmd;
	CSphString sError;
	bool bVipConn = session::GetVip ();
	auto tCmd = DebugCmd::ParseDebugCmd ( sCommand, sError );

	if ( bVipConn )
	{
		switch ( tCmd.m_eCommand )
		{
		case Cmd_e::SHUTDOWN:
		case Cmd_e::CRASH: HandleShutdownCrash ( tOut, tCmd.m_sParam, tCmd.m_eCommand ); return;
#if !_WIN32
		case Cmd_e::PROCDUMP: HandleProcDump ( tOut ); return;
		case Cmd_e::SETGDB: HandleSetGdb ( tOut, tCmd.m_iPar1!=0 ); return;
		case Cmd_e::GDBSTATUS: HandleGdbStatus ( tOut ); return;
#endif
		default: break;
		}
	}


	switch ( tCmd.m_eCommand )
	{
#if HAVE_MALLOC_STATS
	case Cmd_e::MALLOC_STATS: HandleMallocStats ( tOut, tCmd.m_sParam ); return;
#endif

#if HAVE_MALLOC_TRIM
	case Cmd_e::MALLOC_TRIM: HandleMallocTrim ( tOut ); return;
#endif
	case Cmd_e::TOKEN: HandleToken ( tOut, tCmd.m_sParam ); return;
	case Cmd_e::SLEEP: HandleSleep ( tOut, tCmd.m_iPar1 ); return;
	case Cmd_e::TASKS: HandleTasks ( tOut ); return;
	case Cmd_e::SCHED: HandleSched ( tOut ); return;
	case Cmd_e::MERGE: HandleMysqlOptimizeManual ( tOut, tCmd ); return;
	case Cmd_e::DROP: HandleMysqlDropManual ( tOut, tCmd ); return;
	case Cmd_e::FILES: HandleMysqlfiles ( tOut, tCmd ); return;
	case Cmd_e::CLOSE: HandleMysqlclose ( tOut ); return;
	case Cmd_e::COMPRESS: HandleMysqlCompress ( tOut, tCmd ); return;
	case Cmd_e::SPLIT: HandleMysqlSplit ( tOut, tCmd ); return;
#if !_WIN32
	case Cmd_e::WAIT: HandleWait ( tOut, tCmd ); return;
	case Cmd_e::WAIT_STATUS: HandleWaitStatus ( tOut, tCmd ); return;
#endif
	default: break;
	}

	// no known command; provide short help.
	BYTE uMask = bVipConn ? DebugCmd::NEED_VIP : DebugCmd::NONE;
#if !_WIN32
	uMask |= DebugCmd::NO_WIN;
#endif

#if HAVE_MALLOC_STATS
	uMask |= DebugCmd::MALLOC_STATS;
#endif

#if HAVE_MALLOC_TRIM
	uMask |= DebugCmd::MALLOC_TRIM;
#endif

	// display a short help
	tOut.HeadTuplet ( "command", "meaning" );
	tOut.DataTuplet ( "flush logs", "emulate USR1 signal" );
	tOut.DataTuplet ( "reload indexes", "emulate HUP signal" );
	for ( const auto& dCommand : DebugCmd::dCommands )
		if ( ( dCommand.m_uTraits & uMask )==dCommand.m_uTraits )
			tOut.DataTuplet ( dCommand.m_szExample, dCommand.m_szExplanation );
	tOut.Eof ();
}

// fwd
static bool PrepareReconfigure ( const CSphString & sIndex, CSphReconfigureSettings & tSettings, CSphString & sError );

void HandleMysqlTruncate ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	if ( !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
		return;

	bool bReconfigure = ( tStmt.m_iIntParam==1 );

	auto pCmd = MakeReplicationCommand ( ReplicationCommand_e::TRUNCATE, tStmt.m_sIndex, tStmt.m_sCluster );
	CSphString sError;
	const CSphString & sIndex = tStmt.m_sIndex;

	if ( bReconfigure )
	{
		pCmd->m_tReconfigure = std::make_unique<CSphReconfigureSettings>();
		pCmd->m_tReconfigure->m_bChangeSchema = true;
	}

	if ( bReconfigure && !PrepareReconfigure ( sIndex, *pCmd->m_tReconfigure, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	// get an exclusive lock for operation
	// but only read lock for check
	{
		auto pIndex = GetServed ( sIndex );
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

	RtAccum_t tAcc ( false );
	tAcc.m_dCmd.Add ( std::move ( pCmd ) );

	bool bRes = HandleCmdReplicate ( tAcc, sError );
	if ( !bRes )
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
	else
		tOut.Ok();
}


void HandleMysqlOptimize ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	if ( !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
		return;

	auto sIndex = tStmt.m_sIndex;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( tStmt.m_sStmt, "OPTIMIZE INDEX requires an existing RT index" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eManualOptimize;
	tTask.m_iCutoff = tStmt.m_tQuery.m_iCutoff<=0 ? 0 : tStmt.m_tQuery.m_iCutoff;
	tTask.m_sIndex = std::move ( sIndex );

	if ( tStmt.m_tQuery.m_bSync )
		RIdx_T<RtIndex_i*> ( pIndex )->Optimize ( std::move ( tTask ) );
	else
		RunOptimizeRtIndex ( std::move ( tTask ) );
	tOut.Ok();
}

// STMT_SELECT_SYSVAR: SELECT @@sysvar1 [ as alias] [@@sysvarN [ as alias]] [limit M]
void HandleMysqlSelectSysvar ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	struct SysVar_t
	{
		const MysqlColumnType_e m_eType;
		const char * m_sName;
		std::function<CSphString ( void )> m_fnValue;
	};

	auto pVars = session::Info().GetClientSession();
	const SysVar_t dSysvars[] =
	{	{ MYSQL_COL_STRING,	nullptr, [] {return "";}}, // stub
		{ MYSQL_COL_LONG,	"@@session.auto_increment_increment",	[] {return "1";}},
		{ MYSQL_COL_STRING,	"@@character_set_client", [] {return "utf8";}},
		{ MYSQL_COL_STRING,	"@@character_set_connection", [] {return "utf8";}},
		{ MYSQL_COL_LONG,	"@@max_allowed_packet", [] { StringBuilder_c s; s << g_iMaxPacketSize; return CSphString(s); }},
		{ MYSQL_COL_STRING,	"@@version_comment", [] { return szGIT_BRANCH_ID;}},
		{ MYSQL_COL_LONG,	"@@lower_case_table_names", [] { return "1"; }},
		{ MYSQL_COL_STRING,	"@@session.last_insert_id", [&pVars]
			{
				StringBuilder_c s ( "," );
				pVars->m_dLastIds.Apply ( [&s] ( int64_t iID ) { s << iID; } );
				return CSphString(s);
			}},
	};

	auto fnVar = [&dSysvars] ( const CSphString & sVar )->const SysVar_t &
	{
		for ( const auto & tVar : dSysvars )
			if ( sVar==tVar.m_sName )
				return tVar;

		return dSysvars[0];
	};

	// fill header
	tOut.HeadBegin ( tStmt.m_tQuery.m_dItems.GetLength () );
	for ( const auto& dItem : tStmt.m_tQuery.m_dItems )
		tOut.HeadColumn ( dItem.m_sAlias.cstr (), fnVar ( dItem.m_sExpr ).m_eType );
	tOut.HeadEnd ();

	// fill values
	for ( const auto & tItem : tStmt.m_tQuery.m_dItems )
		tOut.PutString ( fnVar ( tItem.m_sExpr ).m_fnValue().cstr() );

	// finalize
	tOut.Commit ();
	tOut.Eof ();
}

struct ExtraLastInsertID_t : public ISphExtra
{
	explicit ExtraLastInsertID_t ( const CSphVector<int64_t> & dIds )
		: m_dIds ( dIds )
	{}

	bool ExtraDataImpl ( ExtraData_e eCmd, void ** pData ) override
	{
		if ( eCmd==EXTRA_GET_LAST_INSERT_ID )
		{
			StringBuilder_c tBuf ( "," );
			for ( int64_t iID : m_dIds )
				tBuf.Appendf ( INT64_FMT, iID );

			auto * sVal = ( CSphString *)pData;
			assert ( sVal );
			*sVal = tBuf.cstr();

			return true;
		}
		return false;
	}

	const CSphVector<int64_t> & m_dIds;
};



void HandleMysqlSelectDual ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
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
	tOut.HeadColumn ( sVar.cstr() );
	tOut.HeadEnd();

	auto pVars = session::Info().GetClientSession();
	ExtraLastInsertID_t tIds ( pVars->m_dLastIds );
	pExpr->Command ( SPH_EXPR_SET_EXTRA_DATA, &tIds );

	CSphMatch tMatch;
	const BYTE * pStr = nullptr;

	switch ( eAttrType )
	{
		case SPH_ATTR_STRINGPTR:
		{
			int  iLen = pExpr->StringEval ( tMatch, &pStr );
			tOut.PutArray ( pStr, iLen );
			FreeDataPtr ( *pExpr, pStr );
			break;
		}
		case SPH_ATTR_INTEGER:	tOut.PutNumAsString ( pExpr->IntEval ( tMatch ) ); break;
		case SPH_ATTR_BIGINT:	tOut.PutNumAsString ( pExpr->Int64Eval ( tMatch ) ); break;
		case SPH_ATTR_FLOAT:	tOut.PutFloatAsString ( pExpr->Eval ( tMatch ) ); break;
		case SPH_ATTR_DOUBLE:	tOut.PutDoubleAsString ( pExpr->Eval ( tMatch ) ); break;
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

// SHOW [GLOBAL|SESSION] VARIABLES WHERE variable_name='xxx' [OR variable_name='xxx']
// SHOW [GLOBAL|SESSION] VARIABLES WHERE variable_name='xxx' [OR variable_name='xxx']
void HandleMysqlShowVariables ( RowBuffer_i & dRows, const SqlStmt_t & tStmt )
{
	VectorLike dTable ( tStmt.m_sStringParam );
	{
		auto pVars = session::Info().GetClientSession();
		dTable.MatchTuplet ( "autocommit", pVars->m_bAutoCommit ? "1" : "0" );
		dTable.MatchTupletf ( "auto_optimize", "%d", g_iAutoOptimizeCutoffMultiplier );
		dTable.MatchTupletf ( "optimize_cutoff", "%d", MutableIndexSettings_c::GetDefaults().m_iOptimizeCutoff );
		dTable.MatchTuplet ( "collation_connection", sphCollationToName ( session::GetCollation() ) );
		dTable.MatchTuplet ( "query_log_format", g_eLogFormat==LOG_FORMAT_PLAIN ? "plain" : "sphinxql" );
		dTable.MatchTuplet ( "session_read_only", session::GetReadOnly() ? "1" : "0" );
		dTable.MatchTuplet ( "log_level", LogLevelName ( g_eLogLevel ) );
		dTable.MatchTupletf ( "max_allowed_packet", "%d", g_iMaxPacketSize );
		dTable.MatchTuplet ( "character_set_client", "utf8" );
		dTable.MatchTuplet ( "character_set_connection", "utf8" );
		dTable.MatchTuplet ( "grouping_in_utc", g_bGroupingInUtc ? "1" : "0" );
		dTable.MatchTupletFn ( "last_insert_id" , [&pVars]
		{
			StringBuilder_c tBuf ( "," );
			pVars->m_dLastIds.Apply ( [&tBuf] ( int64_t iID ) { tBuf << iID; } );
			return tBuf;
		});
	}
	dTable.MatchTuplet ( "pseudo_sharding", g_bSplit ? "1" : "0" );
	dTable.MatchTuplet ( "secondary_indexes", GetSecondaryIndexDefault() ? "1" : "0" );

	if ( tStmt.m_iIntParam>=0 ) // that is SHOW GLOBAL VARIABLES
	{
		Uservar_e eType = tStmt.m_iIntParam==0 ? USERVAR_INT_SET : USERVAR_INT_SET_TMP;
		IterateUservars ( [&dTable, eType] ( const NamedRefVectorPair_t &dVar ) {
			if ( dVar.second.m_eType==eType )
				dTable.MatchTupletf ( dVar.first.cstr(), "%d", dVar.second.m_pVal ? dVar.second.m_pVal->GetLength() : 0 );
		});
	}

	// fine
	dRows.DataTable ( dTable );
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
	for ( int i = 0; i < INTERVAL_TOTAL; ++i )
	{
		if ( dStatus.MatchAddf ( "%s_%s", szPrefix, dStatIntervalNames[i] ) )
		{
			sBuf.Clear();
			{
				ScopedComma_c VARIABLE_IS_NOT_USED tRootBlock( sBuf, ", ", R"({"queries":)", "}" );
				sBuf << tStats.m_dStats[i].m_uTotalQueries;
				for ( int j = 0; j < TYPE_TOTAL; ++j )
					FormatFn ( sBuf, tStats.m_dStats[i].m_uTotalQueries,
							tStats.m_dStats[i].m_dData[j], dStatTypeNames[j] );
			}
			dStatus.Add ( sBuf.cstr() );
		}
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


static void AddIndexQueryStats ( VectorLike & dStatus, const ServedStats_c& tStats )
{
	QueryStats_t tQueryTimeStats, tRowsFoundStats;
	tStats.CalculateQueryStats ( tRowsFoundStats, tQueryTimeStats );
	AddQueryTimeStatsToOutput ( dStatus, "query_time", tQueryTimeStats );

#ifndef NDEBUG
	QueryStats_t tExactQueryTimeStats, tExactRowsFoundStats;
	tStats.CalculateQueryStatsExact ( tExactQueryTimeStats, tExactRowsFoundStats );
	AddQueryTimeStatsToOutput ( dStatus, "exact_query_time", tQueryTimeStats );
#endif

	AddFoundRowsStatsToOutput ( dStatus, "found_rows", tRowsFoundStats );
}

static void AddFederatedIndexStatus ( const CSphSourceStats & tStats, const CSphString & sName, RowBuffer_i & tOut )
{
	if (!tOut.HeadOfStrings ( { "Name", "Engine", "Version", "Row_format", "Rows", "Avg_row_length", "Data_length",
		"Max_data_length", "Index_length", "Data_free",	"Auto_increment", "Create_time", "Update_time", "Check_time",
		"Collation", "Checksum", "Create_options", "Comment" } ))
		return;

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
	tOut.Eof ();
}

static void AddDiskIndexStatus ( VectorLike & dStatus, const CSphIndex * pIndex, bool bMutable )
{
	auto iDocs = pIndex->GetStats ().m_iTotalDocuments;
	dStatus.MatchTupletf ( "indexed_documents", "%l", iDocs );
	dStatus.MatchTupletf ( "indexed_bytes", "%l", pIndex->GetStats ().m_iTotalBytes );

	const int64_t * pFieldLens = pIndex->GetFieldLens();
	if ( pFieldLens )
	{
		int64_t iTotalTokens = 0;
		for ( int i=0; i < pIndex->GetMatchSchema().GetFieldsCount(); ++i )
		{
			if ( dStatus.MatchAddf ( "field_tokens_%s", pIndex->GetMatchSchema ().GetFieldName ( i ) ) )
				dStatus.Addf( "%l", pFieldLens[i] );
			iTotalTokens += pFieldLens[i];
		}
		dStatus.MatchTupletf ( "total_tokens", "%l", iTotalTokens );
	}

	CSphIndexStatus tStatus;
	pIndex->GetStatus ( &tStatus );
	dStatus.MatchTupletf ( "ram_bytes", "%l", tStatus.m_iRamUse );
	dStatus.MatchTupletf ( "disk_bytes", "%l", tStatus.m_iDiskUse );
	dStatus.MatchTupletf ( "disk_mapped", "%l", tStatus.m_iMapped );
	dStatus.MatchTupletf ( "disk_mapped_cached", "%l", tStatus.m_iMappedResident );
	dStatus.MatchTupletf ( "disk_mapped_doclists", "%l", tStatus.m_iMappedDocs );
	dStatus.MatchTupletf ( "disk_mapped_cached_doclists", "%l", tStatus.m_iMappedResidentDocs );
	dStatus.MatchTupletf ( "disk_mapped_hitlists", "%l", tStatus.m_iMappedHits );
	dStatus.MatchTupletf ( "disk_mapped_cached_hitlists", "%l", tStatus.m_iMappedResidentHits );
	dStatus.MatchTupletf ( "killed_documents", "%l", tStatus.m_iDead );
	dStatus.MatchTupletFn ( "killed_rate", [&tStatus, iDocs] {
		StringBuilder_c sPercent;
		if ( iDocs )
			sPercent.Sprintf ( "%0.2F%%", tStatus.m_iDead * 10000 / iDocs );
		else
			sPercent << "100%";
		return CSphString ( sPercent.cstr () );
	} );
	if ( bMutable )
	{
		dStatus.MatchTupletf ( "ram_chunk", "%l", tStatus.m_iRamChunkSize );
		dStatus.MatchTupletf ( "ram_chunk_segments_count", "%d", tStatus.m_iNumRamChunks );
		dStatus.MatchTupletf ( "disk_chunks", "%d", tStatus.m_iNumChunks );
		dStatus.MatchTupletf ( "mem_limit", "%l", tStatus.m_iMemLimit );
		dStatus.MatchTupletf ( "mem_limit_rate", "%0.2F%%", PercentOf ( tStatus.m_fSaveRateLimit, 1.0, 2 ) );
		dStatus.MatchTupletf ( "ram_bytes_retired", "%l", tStatus.m_iRamRetired );
		dStatus.MatchTupletf ( "tid", "%l", tStatus.m_iTID );
		dStatus.MatchTupletf ( "tid_saved", "%l", tStatus.m_iSavedTID );
	}
}

const char * szIndexType ( IndexType_e eType )
{
	switch ( eType )
	{
	case IndexType_e::PLAIN: return "disk";
	case IndexType_e::TEMPLATE: return "template";
	case IndexType_e::RT: return "rt";
	case IndexType_e::PERCOLATE: return "percolate";
	case IndexType_e::DISTR: return "distributed";
	default: return "unknown";
	}
}

static void AddPlainIndexStatus ( RowBuffer_i & tOut, const cServedIndexRefPtr_c& pServed, const ServedStats_c& tStats,
		bool bModeFederated, const CSphString & sName, const CSphString & sPattern )
{
	assert ( pServed );
	RIdx_c pIndex { pServed };
	assert ( pIndex );

	if ( bModeFederated )
	{
		AddFederatedIndexStatus ( pIndex->GetStats (), sName, tOut );
		return;
	}

	VectorLike dStatus ( sPattern );
	dStatus.MatchTuplet ( "index_type", szIndexType ( pServed->m_eType ) );
	AddDiskIndexStatus ( dStatus, pIndex, ServedDesc_t::IsMutable ( pServed ) );
	AddIndexQueryStats ( dStatus, tStats );
	tOut.DataTable ( dStatus );
}


static void AddDistibutedIndexStatus ( RowBuffer_i & tOut, const cDistributedIndexRefPtr_t& pIndex, bool bFederatedUser, const CSphString & sName, const CSphString & sPattern )
{
	assert ( pIndex );

	if ( bFederatedUser )
	{
		CSphSourceStats tStats;
		tStats.m_iTotalDocuments = 1000; // TODO: check is it worth to query that number from agents
		AddFederatedIndexStatus ( tStats, sName, tOut );
		return;
	}

	VectorLike dStatus ( sPattern );
	dStatus.MatchTuplet( "index_type", "distributed" );
	AddIndexQueryStats ( dStatus, pIndex->m_tStats );
	tOut.DataTable ( dStatus );
}


void HandleMysqlShowIndexStatus ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, bool bFederatedUser )
{
	CSphString sError;
	auto pServed = GetServed ( tStmt.m_sIndex );

	int iChunk = tStmt.m_iIntParam;
	if ( tStmt.m_dIntSubkeys.GetLength ()>=1 )
		iChunk = tStmt.m_dIntSubkeys[0];

	if ( pServed )
	{
		if ( iChunk>=0 && pServed->m_eType == IndexType_e::RT )
		{
			RIdx_T<const RtIndex_i*> ( pServed )->ProcessDiskChunk ( iChunk, [&tOut, &tStmt] ( const CSphIndex* pIndex )
			{
				if ( !pIndex )
				{
					tOut.Error ( tStmt.m_sStmt, "SHOW INDEX STATUS requires an existing index" );
					return;
				}

				VectorLike dStatus ( tStmt.m_sStringParam );
				AddDiskIndexStatus ( dStatus, pIndex, false );
				tOut.DataTable ( dStatus );
			});
		} else
			AddPlainIndexStatus ( tOut, pServed, *pServed->m_pStats, bFederatedUser, tStmt.m_sIndex, tStmt.m_sStringParam );
		return;
	}

	auto pIndex = GetDistr ( tStmt.m_sIndex );

	if ( pIndex )
		AddDistibutedIndexStatus ( tOut, pIndex, bFederatedUser, tStmt.m_sIndex, tStmt.m_sStringParam );
	else
		tOut.Error ( tStmt.m_sStmt, "SHOW INDEX STATUS requires an existing index" );
}

void PutIndexStatus ( RowBuffer_i & tOut, const CSphIndex * pIndex )
{
	tOut.PutString ( pIndex->GetFilename () );

	auto & tStats = pIndex->GetStats ();
	tOut.PutNumAsString ( tStats.m_iTotalDocuments );
	tOut.PutNumAsString ( tStats.m_iTotalBytes );

	CSphIndexStatus tStatus;
	pIndex->GetStatus ( &tStatus );
	tOut.PutNumAsString ( tStatus.m_iRamUse );
	tOut.PutNumAsString ( tStatus.m_iDiskUse );
	tOut.PutNumAsString ( tStatus.m_iMapped );
	tOut.PutNumAsString ( tStatus.m_iMappedResident );
	tOut.PutNumAsString ( tStatus.m_iMappedDocs );
	tOut.PutNumAsString ( tStatus.m_iMappedResidentDocs );
	tOut.PutNumAsString ( tStatus.m_iMappedHits );
	tOut.PutNumAsString ( tStatus.m_iMappedResidentHits );
	tOut.PutNumAsString ( tStatus.m_iDead );
}

void HandleSelectIndexStatus ( RowBuffer_i & tOut, const SqlStmt_t * pStmt )
{
	tOut.HeadBegin ( 13 );
	tOut.HeadColumn ( "chunk_id", MYSQL_COL_LONG );
	tOut.HeadColumn ( "base_name" );
	tOut.HeadColumn ( "indexed_documents", MYSQL_COL_LONG );
	tOut.HeadColumn ( "indexed_bytes", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "ram_bytes", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "disk_bytes", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "disk_mapped", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "disk_mapped_cached", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "disk_mapped_doclists", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "disk_mapped_cached_doclists", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "disk_mapped_hitlists", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "disk_mapped_cached_hitlists", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "killed_documents", MYSQL_COL_LONGLONG );
	if ( !tOut.HeadEnd () )
		return;

	const auto & tStmt = *pStmt;
	auto pServed = GetServed ( tStmt.m_sIndex );

	if ( !ServedDesc_t::IsLocal ( pServed ) )
	{
		tOut.Error ( tStmt.m_sStmt, "select INDEX.status requires an existing index" );
		return;
	}

	RIdx_c pIndex { pServed };
	if ( pIndex->IsRT () )
	{
		auto* pRtIndex = static_cast<const RtIndex_i*> ( pIndex.Ptr() );
		int iChunk = 0;
		bool bKeepIteration = true;
		while ( bKeepIteration )
		{
			pRtIndex->ProcessDiskChunk (iChunk,[&bKeepIteration, &tOut] (const CSphIndex* pChunk) {
				if ( !pChunk )
				{
					bKeepIteration = false;
					return;
				}
				tOut.PutNumAsString ( pChunk->m_iChunk );
				PutIndexStatus ( tOut, pChunk );
				if ( !tOut.Commit () )
				{
					bKeepIteration = false;
					return;
				}
			});
			++iChunk;
		}
	} else {
		tOut.PutNumAsString ( 0 ); // dummy 'chunk' of non-rt
		PutIndexStatus ( tOut, pIndex );
		tOut.Commit ();
	}
	tOut.Eof();
}


void HandleMysqlShowIndexSettings ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( !pServed )
	{
		tOut.Error ( tStmt.m_sStmt, "SHOW INDEX SETTINGS requires an existing index" );
		return;
	}

	int iChunk = tStmt.m_iIntParam;
	if ( tStmt.m_dIntSubkeys.GetLength ()>=1 )
		iChunk = (int) tStmt.m_dIntSubkeys[0];

	auto fnShowSettings = [&tOut, szStmt=tStmt.m_sStmt] ( const CSphIndex* pIndex )
	{
		if ( !tOut.HeadOfStrings ( { "Variable_name", "Value" } ) )
			return;

		StringBuilder_c tBuf;
		std::unique_ptr<FilenameBuilder_i> pFilenameBuilder = CreateFilenameBuilder ( pIndex->GetName () );
		DumpSettings ( tBuf, *pIndex, pFilenameBuilder.get () );

		tOut.DataTuplet ( "settings", tBuf.cstr () );
		tOut.Eof ();
	};

	if ( iChunk >= 0 && pServed->m_eType == IndexType_e::RT )
		RIdx_T<const RtIndex_i*> ( pServed )->ProcessDiskChunk ( iChunk, fnShowSettings );
	else
		fnShowSettings ( RIdx_c(pServed) );
}


void HandleMysqlShowProfile ( RowBuffer_i & tOut, const QueryProfile_c & p, bool bMoreResultsFollow )
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


static void AddAttrToIndex ( const SqlStmt_t & tStmt, CSphIndex * pIdx, CSphString & sError )
{
	CSphString sAttrToAdd = tStmt.m_sAlterAttr;
	sAttrToAdd.ToLower();

	bool bIndexed = tStmt.m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED;
	bool bStored = tStmt.m_uFieldFlags & CSphColumnInfo::FIELD_STORED;
	bool bAttribute = tStmt.m_uFieldFlags & CSphColumnInfo::FIELD_IS_ATTRIBUTE; // beware, m.b. true only for strings

	bool bHasAttr = pIdx->GetMatchSchema ().GetAttr ( sAttrToAdd.cstr () );
	bool bHasField = pIdx->GetMatchSchema ().GetFieldIndex ( sAttrToAdd.cstr () )!=-1;

	if ( !bIndexed && bHasAttr )
	{
		sError.SetSprintf ( "'%s' attribute already in schema", sAttrToAdd.cstr () );
		return;
	}

	if ( bIndexed && bHasField )
	{
		sError.SetSprintf ( "'%s' field already in schema", sAttrToAdd.cstr() );
		return;
	}

	if ( !bIndexed && bHasField && tStmt.m_eAlterColType!=SPH_ATTR_STRING )
	{
		sError.SetSprintf ( "cannot add attribute that shadows '%s' field", sAttrToAdd.cstr () );
		return;
	}

	AttrAddRemoveCtx_t tCtx;
	tCtx.m_sName = sAttrToAdd;
	tCtx.m_eType = tStmt.m_eAlterColType;
	tCtx.m_iBits = tStmt.m_iBits;
	tCtx.m_uFlags = tStmt.m_uAttrFlags;
	tCtx.m_eEngine = tStmt.m_eEngine;

	if ( bIndexed || bStored )
	{
		pIdx->AddRemoveField ( true, sAttrToAdd, tStmt.m_uFieldFlags, sError );
		if ( bAttribute )
			pIdx->AddRemoveAttribute ( true, tCtx, sError );
	}
	else
		pIdx->AddRemoveAttribute ( true, tCtx, sError );
}


static void RemoveAttrFromIndex ( const SqlStmt_t& tStmt, CSphIndex* pIdx, CSphString& sError )
{
	CSphString sAttrToRemove = tStmt.m_sAlterAttr;
	sAttrToRemove.ToLower();

	bool bIsAttr = true;
	const CSphColumnInfo * pAttr = pIdx->GetMatchSchema().GetAttr ( sAttrToRemove.cstr() );
	if ( !pAttr )
	{
		pAttr = pIdx->GetMatchSchema ().GetField ( sAttrToRemove.cstr () );
		if ( !pAttr )
		{
			sError.SetSprintf ( "attribute '%s' does not exist", sAttrToRemove.cstr() );
			return;
		}
		bIsAttr = false;
	}

	if ( bIsAttr && ( sAttrToRemove==sphGetDocidName () || sphIsInternalAttr ( *pAttr ) ) )
	{
		sError.SetSprintf ( "unable to remove built-in attribute '%s'", sAttrToRemove.cstr() );
		return;
	}

	if ( bIsAttr && pIdx->GetMatchSchema().GetAttrsCount()==1 )
	{
		sError.SetSprintf ( "unable to remove last attribute '%s'", sAttrToRemove.cstr() );
		return;
	}

	if ( bIsAttr )
	{
		AttrAddRemoveCtx_t tCtx;
		tCtx.m_sName = sAttrToRemove;
		tCtx.m_eType = pAttr->m_eAttrType;
		pIdx->AddRemoveAttribute ( false, tCtx, sError );
	}
	else
		pIdx->AddRemoveField ( false, sAttrToRemove, 0, sError );
}


static void HandleMysqlAlter ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, bool bAdd )
{
	if ( !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
		return;
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
		auto pServed = GetServed ( sName );
		if ( !pServed )
		{
			dErrors.Submit ( sName, nullptr, "unknown local index in ALTER request" );
			continue;
		}

		// cluster does not implement ALTER for now
		auto tCluster = IsPartOfCluster ( pServed );
		if ( tCluster )
		{
			dErrors.SubmitEx ( sName, nullptr, "is part of cluster %s, can not issue ALTER", tCluster->cstr() );
			continue;
		}

		CSphString sAddError;

		if ( bAdd )
			AddAttrToIndex ( tStmt, WIdx_c ( pServed ), sAddError );
		else
			RemoveAttrFromIndex ( tStmt, WIdx_c ( pServed ), sAddError );

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
	std::unique_ptr<FilenameBuilder_i> pFilenameBuilder = CreateFilenameBuilder ( sIndex.cstr() );

	// fixme: report warnings
	tSettings.m_tTokenizer.Setup ( hIndex, sWarning );
	tSettings.m_tDict.Setup ( hIndex, pFilenameBuilder.get(), sWarning );
	tSettings.m_tFieldFilter.Setup ( hIndex, sWarning );
	tSettings.m_tMutableSettings.Load ( hIndex, false, nullptr );

	if ( !sphRTSchemaConfigure ( hIndex, tSettings.m_tSchema, tSettings.m_tIndex, sError, !tSettings.m_bChangeSchema, false ) )
	{
		sError.SetSprintf ( "failed to parse index '%s' schema, error: '%s'", sIndex.cstr(), sError.cstr() );
		return false;
	}

	if ( !tSettings.m_tIndex.Setup ( hIndex, sIndex.cstr(), sWarning, sError ) )
	{
		sError.SetSprintf ( "failed to parse index '%s' settings, error: '%s'", sIndex.cstr(), sError.cstr() );
		return false;
	}

	tSettings.m_tSchema.SetupFlags ( tSettings.m_tIndex, false, nullptr );

	return CheckStoredFields ( tSettings.m_tSchema, tSettings.m_tIndex, sError );
}


static bool PrepareReconfigure ( const CSphString & sIndex, CSphReconfigureSettings & tSettings, CSphString & sError )
{
	CSphConfig hCfg;
	if ( !ParseConfig ( &hCfg, g_sConfigFile.cstr () ) )
	{
		sError.SetSprintf ( "failed to parse config file '%s': %s; using previous settings", g_sConfigFile.cstr (), TlsMsg::szError() );
		return false;
	}

	if ( !hCfg.Exists ( "index" ) )
	{
		sError.SetSprintf ( "failed to find any index in config file '%s'; using previous settings", g_sConfigFile.cstr () );
		return false;
	}

	if ( !hCfg["index"].Exists ( sIndex ) )
	{
		sError.SetSprintf ( "failed to find index '%s' in config file '%s'; using previous settings", sIndex.cstr(), g_sConfigFile.cstr () );
		return false;
	}

	CSphString sWarning;
	return PrepareReconfigure ( sIndex, hCfg["index"][sIndex], tSettings, sWarning, sError );
}


static void HandleMysqlReconfigure ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	if ( !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
		return;

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
	if ( !ServedDesc_t::IsMutable ( pServed ))
	{
		tOut.ErrorEx ( tStmt.m_sStmt, "'%s' is absent, or does not support ALTER", sIndex.cstr() );
		return;
	}

	StrVec_t dWarnings;
	WIdx_T<RtIndex_i*> pRT { pServed };
	if ( !pRT->IsSameSettings ( tSettings, tSetup, dWarnings, sError ) && sError.IsEmpty() )
	{
		if ( !pRT->Reconfigure ( tSetup ) )
		{
			sError.SetSprintf ( "index '%s': reconfigure failed; INDEX UNUSABLE (%s)", tStmt.m_sIndex.cstr(), pRT->GetLastError().cstr() );
			g_pLocalIndexes->Delete ( tStmt.m_sIndex );
		}
	}
	sWarning = ConcatWarnings ( dWarnings );
	if ( sError.IsEmpty() )
		tOut.Ok ( 0, dWarnings.GetLength() );
	else
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
}


static bool ApplyIndexKillList ( const CSphIndex * pIndex, CSphString & sWarning, CSphString & sError, bool bShowMessage = false );


// STMT_ALTER_KLIST_TARGET: ALTER TABLE index KILLLIST_TARGET = 'string'
static void HandleMysqlAlterKlist ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	if ( !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
		return;

	MEMORY ( MEM_SQL_ALTER );

	CSphString sError;

	KillListTargets_c tNewTargets;
	if ( !tNewTargets.Parse ( tStmt.m_sAlterOption, tStmt.m_sIndex.cstr(), sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	auto pServed = GetServed ( tStmt.m_sIndex.cstr () );
	if ( !pServed )
	{
		if ( g_pDistIndexes->Contains ( tStmt.m_sIndex ) )
			sError.SetSprintf ( "ALTER is only supported for local (not distributed) indexes" );
		else
			sError.SetSprintf ( "index '%s' not found", tStmt.m_sIndex.cstr () );
	}
	else if ( ServedDesc_t::IsMutable ( pServed ) )
		sError.SetSprintf ( "'%s' does not support ALTER (real-time or percolate)", tStmt.m_sIndex.cstr () );

	if ( !sError.IsEmpty () )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr () );
		return;
	}

	WIdx_c pIdx { pServed };
	if ( !pIdx->AlterKillListTarget ( tNewTargets, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// apply killlist to new targets
	if ( !ApplyIndexKillList ( pIdx, sWarning, sError ) )
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

// STMT_ALTER_INDEX_SETTINGS: ALTER TABLE index [ident = 'string']*
static void HandleMysqlAlterIndexSettings ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	if ( !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
		return;

	MEMORY ( MEM_SQL_ALTER );

	CSphString sError;
	if ( !IsConfigless() )
	{
		sError = "ALTER TABLE requires data_dir to be set in the config file";
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	auto pServed = GetServed ( tStmt.m_sIndex.cstr() );
	if ( !pServed || pServed->m_eType != IndexType_e::RT )
	{
		tOut.ErrorEx ( tStmt.m_sStmt, "index '%s' is not found, or not real-time", tStmt.m_sIndex.cstr() );
		return;
	}

	WIdx_T<RtIndex_i*> pRtIndex { pServed };

	// get all table settings as a string
	CSphString sCreateTable = BuildCreateTable ( pRtIndex->GetName(), pRtIndex, pRtIndex->GetInternalSchema() );

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

// STMT_SHOW_PLAN: SHOW PLAN
static void HandleMysqlShowPlan ( RowBuffer_i & tOut, const QueryProfile_c & p, bool bMoreResultsFollow, bool bDot )
{
	tOut.HeadBegin ( 2 );
	tOut.HeadColumn ( "Variable" );
	tOut.HeadColumn ( "Value" );
	tOut.HeadEnd ( bMoreResultsFollow );

	tOut.PutString ( "transformed_tree" );
	StringBuilder_c sPlan;
	sph::RenderBsonPlan ( sPlan, bson::MakeHandle ( p.m_dPlan ), bDot );
	tOut.PutString ( sPlan );
	tOut.Commit();

	tOut.Eof ( bMoreResultsFollow );
}

// for seamless we create new index and copy it's settings from previous definition. Indexes are NOT linked
// for greedy we just make light clone (original index add-reffed).
ServedIndexRefPtr_c MakeCloneForRotation ( const cServedIndexRefPtr_c& pSource, const CSphString& sIndex )
{
	assert ( pSource->m_eType == IndexType_e::PLAIN );
	auto pRes = MakeServedIndex();
	LightClone ( pRes, pSource );
	if ( g_bSeamlessRotate )
	{
		pRes->SetStatsFrom ( *pSource );
		auto pIdx = sphCreateIndexPhrase ( sIndex.cstr(), pRes->m_sIndexPath.cstr() );
		pIdx->m_iExpansionLimit = g_iExpansionLimit;
		pIdx->SetMutableSettings ( pRes->m_tSettings );
		pIdx->SetGlobalIDFPath ( pRes->m_sGlobalIDFPath );
		pIdx->SetCacheSize ( g_iMaxCachedDocs, g_iMaxCachedHits );
		pRes->SetIdx ( std::move ( pIdx ) );
	} else
		pRes->SetIdxAndStatsFrom ( *pSource );
	return pRes;
}

static bool LimitedRotateIndexMT ( ServedIndexRefPtr_c& pNewServed, const CSphString& sIndex, StrVec_t& dWarnings, CSphString& sError ) EXCLUDES ( MainThread );
static bool RotateIndexGreedy ( const ServedIndex_c& tServed, const char* szIndex, CSphString& sError ) REQUIRES ( tServed.m_pIndex->Locker() );

static void HandleMysqlReloadIndex ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	CSphString sError;
	cServedIndexRefPtr_c pServed = GetServed ( tStmt.m_sIndex );
	if ( !pServed )
	{
		tOut.ErrorEx ( tStmt.m_sStmt, "unknown local index '%s'", tStmt.m_sIndex.cstr() );
		return;
	}

	if ( ServedDesc_t::IsMutable ( pServed ) )
	{
		tOut.ErrorEx ( tStmt.m_sStmt, "can not reload real-time or percolate index" );
		return;
	}

	if ( tStmt.m_sStringParam == pServed->m_sIndexPath )
	{
		tOut.ErrorEx ( tStmt.m_sStmt, "reload path should be different from current path" );
		return;
	}

	if ( !tStmt.m_sStringParam.IsEmpty () )
	{
		// try move files from arbitrary path to current index path before rotate, if needed.
		// fixme! what about concurrency? if 2 sessions simultaneously ask to rotate,
		// or if we have unapplied rotates from indexer - seems that it will garbage .new files?
		IndexFiles_c sIndexFiles ( pServed->m_sIndexPath );
		if ( !sIndexFiles.RelocateToNew ( tStmt.m_sStringParam ) )
		{
			tOut.Error ( tStmt.m_sStmt, sIndexFiles.ErrorMsg () );
			return;
		}
	}

	StrVec_t dWarnings;
	if ( g_bSeamlessRotate )
	{
		ServedIndexRefPtr_c pNewServed = MakeCloneForRotation ( pServed, tStmt.m_sIndex );
		if ( !LimitedRotateIndexMT ( pNewServed, tStmt.m_sIndex, dWarnings, sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
	} else {
		WIdx_c WLock { pServed };
		if ( !RotateIndexGreedy ( *pServed, tStmt.m_sIndex.cstr(), sError ) )
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
		sWarn.MoveTo ( sWarning );
	}

	tOut.Ok();
}

void HandleMysqlExplain ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, bool bDot )
{
	CSphString sProc ( tStmt.m_sCallProc );
	if ( sProc.ToLower()!="query" )
	{
		tOut.ErrorEx ( tStmt.m_sStmt, "no such explain procedure %s", tStmt.m_sCallProc.cstr () );
		return;
	}

	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( !pServed )
	{
		tOut.ErrorEx ( tStmt.m_sStmt, "unknown local index '%s'", tStmt.m_sIndex.cstr ());
		return;
	}

	TlsMsg::Err (); // reset error
	auto dPlan = RIdx_c ( pServed )->ExplainQuery ( tStmt.m_tQuery.m_sQuery );
	if ( TlsMsg::HasErr ())
	{
		tOut.Error ( tStmt.m_sStmt, TlsMsg::szError ());
		return;
	}

	StringBuilder_c sRes;
	sph::RenderBsonPlan ( sRes, bson::MakeHandle ( dPlan ), bDot );

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
	if ( !sphCheckWeCanModify ( tStmt.m_sStmt, tOut ) )
		return;

	CSphString sError;

	if ( !IsConfigless() )
	{
		sError = "IMPORT TABLE requires data_dir to be set in the config file";
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( IndexIsServed ( tStmt.m_sIndex ) )
	{
		sError.SetSprintf ( "index '%s' already exists", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	bool bPQ = false;
	StrVec_t dWarnings;
	if ( !CopyIndexFiles ( tStmt.m_sIndex, tStmt.m_sStringParam, bPQ, dWarnings, sError ) )
	{
		sError.SetSprintf ( "unable to import index '%s': %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !AddExistingIndexConfigless ( tStmt.m_sIndex, bPQ ? IndexType_e::PERCOLATE : IndexType_e::RT, dWarnings, sError ) )
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
void HandleMysqlLockIndexes ( RowBuffer_i& tOut, const CSphString& sIndexes, CSphString& sWarningOut )
{
	// search through specified local indexes
	StrVec_t dIndexes, dNonlockedIndexes, dIndexFiles;

	ParseIndexList ( sIndexes, dIndexes );
	for ( const auto& sIndex : dIndexes )
	{
		auto pIndex = GetServed ( sIndex );
		if ( !ServedDesc_t::IsMutable ( pIndex ) )
		{
			dNonlockedIndexes.Add ( sIndex );
			continue;
		}

		RIdx_T<RtIndex_i*> pRt { pIndex };
		pRt->LockFileState ( dIndexFiles );
	}

	int iWarnings=0;
	if ( !dNonlockedIndexes.IsEmpty() )
	{
		StringBuilder_c sWarning;
		sWarning << "Some indexes are not suitable for locking: ";
		sWarning.StartBlock();
		dNonlockedIndexes.for_each ( [&sWarning] ( const auto& sValue ) { sWarning << sValue; } );
		sWarning.FinishBlocks ();
		sWarning.MoveTo ( sWarningOut );
		++iWarnings;
	}

	tOut.HeadBegin ( 2 );
	tOut.HeadColumn ( "file" );
	tOut.HeadColumn ( "normalized" );
	tOut.HeadEnd();

	dIndexFiles.for_each ( [&] ( const auto& sFile ) { tOut.PutString (sFile); tOut.PutString (RealPath (sFile)); tOut.Commit(); } );
	tOut.Eof ( false, iWarnings );
}

void HandleMysqlUnlockIndexes ( RowBuffer_i& tOut, const CSphString& sIndexes, CSphString& sWarningOut )
{
	// search through specified local indexes
	StrVec_t dIndexes;

	int iUnlocked=0;
	ParseIndexList ( sIndexes, dIndexes );
	for ( const auto& sIndex : dIndexes )
	{
		auto pIndex = GetServed ( sIndex );
		if ( !ServedDesc_t::IsMutable ( pIndex ) )
			continue;

		RIdx_T<RtIndex_i*> pRt { pIndex };
		pRt->EnableSave ();
		++iUnlocked;
	}

	tOut.Ok ( iUnlocked );
}

RtAccum_t * CSphSessionAccum::GetAcc ( RtIndex_i * pIndex, CSphString & sError )
{
	assert ( pIndex );
	RtAccum_t * pAcc = pIndex->CreateAccum ( m_pAcc.get(), sError );
	if ( !pAcc && !sError.IsEmpty() )
		return pAcc;

	if ( m_pAcc.get()!=pAcc )
		m_pAcc.reset(pAcc);
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

static const CSphString g_sLogDoneStmt = "/* DONE */";
static const Str_t g_tLogDoneStmt = FromStr ( g_sLogDoneStmt );

struct LogStmtGuard_t
{
	LogStmtGuard_t ( const Str_t & sQuery, SqlStmt_e eStmt, bool bMulti )
	{
		m_tmStarted = LogFilterStatementSphinxql ( sQuery, eStmt );
		m_bLogDone = ( m_tmStarted && eStmt!=STMT_UPDATE && eStmt!=STMT_SELECT && !bMulti ); // update and select will log differently
	}

	~LogStmtGuard_t ()
	{
		if ( m_bLogDone )
		{
			int64_t tmDelta = sphMicroTimer() - m_tmStarted;
			LogStatementSphinxql ( g_tLogDoneStmt, (int)( tmDelta / 1000 ) );
		}
	}

	int64_t m_tmStarted = 0;
	bool m_bLogDone = false;
};

void ClientSession_c::FreezeLastMeta()
{
	m_tLastMeta = CSphQueryResultMeta();
	m_tLastMeta.m_sError = m_sError;
	m_tLastMeta.m_sWarning = "";
}

static void HandleMysqlShowSettings ( const CSphConfig & hConf, RowBuffer_i & tOut );

// just execute one sphinxql statement
//
// IMPORTANT! this does NOT start or stop profiling, as there a few external
// things (client net reads and writes) that we want to profile, too
//
// returns true if the current profile should be kept (default)
// returns false if profile should be discarded (eg. SHOW PROFILE case)
bool ClientSession_c::Execute ( Str_t sQuery, RowBuffer_i & tOut )
{
	auto& tSess = session::Info();

	// set on query guard
	tSess.SetTaskState ( TaskState_e::QUERY );
	auto& tCrashQuery = GlobalCrashQueryGetRef();
	tCrashQuery.m_eType = QUERY_SQL;
	tCrashQuery.m_dQuery = { (const BYTE*) sQuery.first, sQuery.second };

	// ad-hoc, make generalized select()
	if ( StrEq ( sQuery.first, "select DATABASE(), USER() limit 1" ) )
	{
		// result set header packet
		tOut.HeadTuplet ( "DATABASE()", "USER()" );
		tOut.DataTuplet ( g_sDbName.cstr(), tSess.GetVip () ? "VIP" : "Usual" );
		tOut.Eof ( false );
		return true;
	}

	// parse SQL query
	if ( tSess.IsProfile() )
		m_tProfile.Switch ( SPH_QSTATE_SQL_PARSE );

	m_sError = "";

	CSphVector<SqlStmt_t> dStmt;
	bool bParsedOK = sphParseSqlQuery ( sQuery.first, sQuery.second, dStmt, m_sError, tSess.GetCollation () );

	if ( tSess.IsProfile() )
		m_tProfile.Switch ( SPH_QSTATE_UNKNOWN );

	SqlStmt_e eStmt = STMT_PARSE_ERROR;
	if ( bParsedOK )
	{
		eStmt = dStmt[0].m_eStmt;
		dStmt[0].m_sStmt = sQuery.first;
	}
	const SqlStmt_e ePrevStmt = m_eLastStmt;
	if ( eStmt!=STMT_SHOW_META )
		m_eLastStmt = eStmt;

	SqlStmt_t * pStmt = dStmt.Begin();
	assert ( !bParsedOK || pStmt );

	myinfo::SetCommand ( g_dSqlStmts[eStmt] );

	LogStmtGuard_t tLogGuard ( sQuery, eStmt, dStmt.GetLength()>1 );

	if ( bParsedOK && m_bFederatedUser )
	{
		if ( !FixupFederatedQuery ( tSess.GetCollation (), dStmt,  m_sError, m_sFederatedQuery ) )
		{
			FreezeLastMeta();
			tOut.Error ( sQuery.first, m_sError.cstr() );
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
		FreezeLastMeta();
		tOut.Error ( sQuery.first, m_sError.cstr() );
		return true;

	case STMT_SELECT:
		{
			MEMORY ( MEM_SQL_SELECT );

			StatCountCommand ( SEARCHD_COMMAND_SEARCH );
			SearchHandler_c tHandler ( 1, sphCreatePlainQueryParser(), QUERY_SQL, true );
			tHandler.SetQuery ( 0, dStmt.Begin()->m_tQuery, dStmt.Begin()->m_pTableFunc );
			dStmt.Begin()->m_pTableFunc = nullptr;
			tHandler.m_pStmt = pStmt;

			if ( tSess.IsProfile() )
				tHandler.SetProfile ( &m_tProfile );
			if ( m_bFederatedUser )
				tHandler.SetFederatedUser();

			if ( HandleMysqlSelect ( tOut, tHandler ) )
			{
				// query just completed ok; reset out error message
				m_sError = "";
				AggrResult_t & tLast = tHandler.m_dAggrResults.Last();
				SendMysqlSelectResult ( tOut, tLast, false, m_bFederatedUser, &m_sFederatedQuery, ( tSess.IsProfile() ? &m_tProfile : nullptr ) );
			}

			// save meta for SHOW META (profile is saved elsewhere)
			m_tLastMeta = tHandler.m_dAggrResults.Last();
			return true;
		}
	case STMT_SHOW_WARNINGS:
		HandleMysqlWarning ( m_tLastMeta, tOut, false );
		return true;

	case STMT_SHOW_STATUS:
	case STMT_SHOW_AGENT_STATUS:
		if ( eStmt==STMT_SHOW_STATUS )
		{
			StatCountCommand ( SEARCHD_COMMAND_STATUS );
		}
		HandleMysqlStatus ( tOut, *pStmt, false );
		return true;

	case STMT_SHOW_META:
		if ( ePrevStmt!=STMT_CALL )
			HandleMysqlMeta ( tOut, *pStmt, m_tLastMeta, false );
		else
			HandleMysqlPercolateMeta ( m_tPercolateMeta, m_tLastMeta.m_sWarning, tOut );
		return true;

	case STMT_INSERT:
	case STMT_REPLACE:
		{
			StmtErrorReporter_c tErrorReporter ( tOut, pStmt->m_sStmt );
			sphHandleMysqlInsert ( tErrorReporter, *pStmt );
			return true;
		}

	case STMT_DELETE:
		{
			StmtErrorReporter_c tErrorReporter ( tOut, pStmt->m_sStmt );
			sphHandleMysqlDelete ( tErrorReporter, *pStmt, sQuery );
			return true;
		}

	case STMT_SET:
		StatCountCommand ( SEARCHD_COMMAND_UVAR );
		HandleMysqlSet ( tOut, *pStmt, m_tAcc );
		return false;

	case STMT_BEGIN:
		{
			StmtErrorReporter_c tErrorReporter ( tOut, pStmt->m_sStmt );
			sphHandleMysqlBegin ( tErrorReporter, sQuery );
			return true;
		}

	case STMT_COMMIT:
	case STMT_ROLLBACK:
		{
			StmtErrorReporter_c tErrorReporter ( tOut, pStmt->m_sStmt );
			sphHandleMysqlCommitRollback ( tErrorReporter, sQuery, eStmt==STMT_COMMIT );
			return true;
		}
	case STMT_CALL:
		// IMPORTANT! if you add a new builtin here, do also add it
		// in the comment to STMT_CALL line in SqlStmt_e declaration,
		// the one that lists expansions for doc/check.pl
		pStmt->m_sCallProc.ToUpper();
		if ( pStmt->m_sCallProc=="SNIPPETS" )
			HandleMysqlCallSnippets ( tOut, *pStmt );
		else if ( pStmt->m_sCallProc=="KEYWORDS" )
			HandleMysqlCallKeywords ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		else if ( pStmt->m_sCallProc=="SUGGEST" )
			HandleMysqlCallSuggest ( tOut, *pStmt, false );
		else if ( pStmt->m_sCallProc=="QSUGGEST" )
			HandleMysqlCallSuggest ( tOut, *pStmt, true );
		else if ( pStmt->m_sCallProc=="PQ" )
		{
			HandleMysqlCallPQ ( tOut, *pStmt, m_tAcc, m_tPercolateMeta );
			m_tPercolateMeta.m_dResult.m_sMessages.MoveWarningsTo ( m_tLastMeta.m_sWarning );
			m_tPercolateMeta.m_dDocids.Reset ( 0 ); // free occupied mem
		} else
		{
			m_sError.SetSprintf ( "no such built-in procedure %s", pStmt->m_sCallProc.cstr() );
			tOut.Error ( sQuery.first, m_sError.cstr() );
		}
		return true;

	case STMT_DESCRIBE:
		HandleMysqlDescribe ( tOut, pStmt );
		return true;

	case STMT_SHOW_TABLES:
		HandleMysqlShowTables ( tOut, pStmt );
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
			StmtErrorReporter_c tErrorReporter ( tOut, pStmt->m_sStmt );
			sphHandleMysqlUpdate ( tErrorReporter, *pStmt, sQuery );
			return true;
		}

	case STMT_DUMMY:
		tOut.Ok();
		return true;

	case STMT_CREATE_FUNCTION:
		if ( !sphPluginCreate ( pStmt->m_sUdfLib.cstr(), PLUGIN_FUNCTION, pStmt->m_sUdfName.cstr(), pStmt->m_eUdfType, m_sError ) )
			tOut.Error ( sQuery.first, m_sError.cstr() );
		else
			tOut.Ok();
		SphinxqlStateFlush ();
		return true;

	case STMT_DROP_FUNCTION:
		if ( !sphPluginDrop ( PLUGIN_FUNCTION, pStmt->m_sUdfName.cstr(), m_sError ) )
			tOut.Error ( sQuery.first, m_sError.cstr() );
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
				tOut.Error ( sQuery.first, m_sError.cstr() );
			else
				tOut.Ok();
			SphinxqlStateFlush ();
			return true;
		}

	case STMT_RELOAD_PLUGINS:
		if ( sphPluginReload ( pStmt->m_sUdfLib.cstr(), m_sError ) )
			tOut.Ok();
		else
			tOut.Error ( sQuery.first, m_sError.cstr() );
		return true;

	case STMT_ATTACH_INDEX:
		m_tLastMeta.m_sWarning = "";
		HandleMysqlAttach ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_FLUSH_RTINDEX:
		HandleMysqlFlushRtindex ( tOut, *pStmt );
		return true;

	case STMT_FLUSH_RAMCHUNK:
		HandleMysqlFlushRamchunk ( tOut, *pStmt );
		return true;

	case STMT_SHOW_VARIABLES:
		HandleMysqlShowVariables ( tOut, *pStmt );
		return true;

	case STMT_TRUNCATE_RTINDEX:
		HandleMysqlTruncate ( tOut, *pStmt );
		return true;

	case STMT_OPTIMIZE_INDEX:
		HandleMysqlOptimize ( tOut, *pStmt );
		return true;

	case STMT_SELECT_SYSVAR:
		HandleMysqlSelectSysvar ( tOut, *pStmt );
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
		HandleMysqlShowPlan ( tOut, m_tLastProfile, false, ::IsDot ( *pStmt ) );
		return false; // do not profile this call, keep last query profile

	case STMT_SELECT_DUAL:
		HandleMysqlSelectDual ( tOut, *pStmt );
		return true;

	case STMT_SHOW_DATABASES:
		HandleMysqlShowDatabases ( tOut, *pStmt );
		return true;

	case STMT_SHOW_PLUGINS:
		HandleMysqlShowPlugins ( tOut, *pStmt );
		return true;

	case STMT_SHOW_THREADS:
		HandleMysqlShowThreads ( tOut, pStmt );
		return true;

	case STMT_ALTER_RECONFIGURE:
		FreezeLastMeta();
		HandleMysqlReconfigure ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_ALTER_KLIST_TARGET:
		FreezeLastMeta();
		HandleMysqlAlterKlist ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_ALTER_INDEX_SETTINGS:
		FreezeLastMeta();
		HandleMysqlAlterIndexSettings ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_FLUSH_INDEX:
		HandleMysqlFlush ( tOut, *pStmt );
		return true;

	case STMT_RELOAD_INDEX:
		FreezeLastMeta();
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
		HandleMysqlDebug ( tOut, sQuery );
		return true;

	case STMT_JOIN_CLUSTER:
		if ( ClusterJoin ( pStmt->m_sIndex, pStmt->m_dCallOptNames, pStmt->m_dCallOptValues, pStmt->m_bClusterUpdateNodes, m_sError ) )
			tOut.Ok();
		else
			tOut.Error ( sQuery.first, m_sError.cstr() );
		return true;
	case STMT_CLUSTER_CREATE:
		if ( ClusterCreate ( pStmt->m_sIndex, pStmt->m_dCallOptNames, pStmt->m_dCallOptValues, m_sError ) )
			tOut.Ok();
		else
			tOut.Error ( sQuery.first, m_sError.cstr() );
		return true;

	case STMT_CLUSTER_DELETE:
		m_tLastMeta = CSphQueryResultMeta();
		if ( ClusterDelete ( pStmt->m_sIndex, m_tLastMeta.m_sError, m_tLastMeta.m_sWarning ) )
			tOut.Ok ( 0, m_tLastMeta.m_sWarning.IsEmpty() ? 0 : 1 );
		else
			tOut.Error ( sQuery.first, m_tLastMeta.m_sError.cstr() );
		return true;

	case STMT_CLUSTER_ALTER_ADD:
	case STMT_CLUSTER_ALTER_DROP:
		m_tLastMeta = CSphQueryResultMeta();
		if ( ClusterAlter ( pStmt->m_sCluster, pStmt->m_sIndex, ( eStmt==STMT_CLUSTER_ALTER_ADD ), m_tLastMeta.m_sError, m_tLastMeta.m_sWarning ) )
			tOut.Ok ( 0, m_tLastMeta.m_sWarning.IsEmpty() ? 0 : 1 );
		else
			tOut.Error ( sQuery.first, m_tLastMeta.m_sError.cstr() );
		return true;

	case STMT_CLUSTER_ALTER_UPDATE:
		m_tLastMeta = CSphQueryResultMeta();
		if ( ClusterAlterUpdate ( pStmt->m_sCluster, pStmt->m_sSetName, true, m_tLastMeta.m_sError ) )
			tOut.Ok();
		else
			tOut.Error ( sQuery.first, m_tLastMeta.m_sError.cstr() );
		return true;

	case STMT_EXPLAIN:
		HandleMysqlExplain ( tOut, *pStmt, IsDot ( *pStmt ) );
		return true;

	case STMT_IMPORT_TABLE:
		FreezeLastMeta();
		HandleMysqlImportTable ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_LOCK:
		HandleMysqlLockIndexes ( tOut, pStmt->m_sIndex, m_tLastMeta.m_sWarning);
		return true;

	case STMT_UNLOCK:
		HandleMysqlUnlockIndexes ( tOut, pStmt->m_sIndex, m_tLastMeta.m_sWarning );
		return true;

	case STMT_SHOW_SETTINGS:
		HandleMysqlShowSettings ( g_hCfg, tOut );
		return true;

	default:
		m_sError.SetSprintf ( "internal error: unhandled statement type (value=%d)", eStmt );
		tOut.Error ( sQuery.first, m_sError.cstr() );
		return true;
	} // switch

	return true; // for cases that break early
}

bool session::IsAutoCommit ( const ClientSession_c* pSession )
{
	assert ( pSession );
	return pSession->m_bAutoCommit;
}

bool session::IsAutoCommit ()
{
	return IsAutoCommit ( GetClientSession() );
}

bool session::IsInTrans ( const ClientSession_c* pSession )
{
	assert ( pSession );
	return pSession->m_bInTransaction;
}

VecTraits_T<int64_t> session::LastIds ()
{
	return GetClientSession()->m_dLastIds;
}

bool session::Execute ( Str_t sQuery, RowBuffer_i& tOut )
{
	auto& tSess = *Info().GetClientSession();
	return tSess.Execute ( sQuery, tOut );
}

void session::SetFederatedUser ()
{
	GetClientSession()->m_bFederatedUser = true;
}

void session::SetAutoCommit ( bool bAutoCommit )
{
	GetClientSession()->m_bAutoCommit = bAutoCommit;
}

void session::SetInTrans ( bool bInTrans )
{
	GetClientSession()->m_bInTransaction = bInTrans;
}

bool session::IsInTrans ()
{
	return IsInTrans ( GetClientSession() );
}

QueryProfile_c * session::StartProfiling ( ESphQueryState eState )
{
	auto pSession = GetClientSession();
	QueryProfile_c* pProfile = nullptr;
	if ( session::IsProfile() ) // the current statement might change it
	{
		pProfile = &pSession->m_tProfile;
		pProfile->Start ( eState );
	}
	return pProfile;
}

void session::SaveLastProfile ()
{
	auto pSession = GetClientSession();
	pSession->m_tLastProfile = pSession->m_tProfile;
}

/// sphinxql command over API
void HandleCommandSphinxql ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq ) REQUIRES (HandlerThread)
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_SPHINXQL, tOut ) )
		return;

	auto tReply = APIAnswer ( tOut, VER_COMMAND_SPHINXQL );

	// parse and run request
	CSphVector<BYTE> dString;
	tReq.GetString ( dString );
	RunSingleSphinxqlCommand ( dString, tOut );
}

/// json command over API
void HandleCommandJson ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_JSON, tOut ) )
		return;

	// parse request
	CSphString sEndpoint = tReq.GetString ();
	CSphString sCommand = tReq.GetString ();
	
	CSphVector<BYTE> dResult;
	sphProcessHttpQueryNoResponce ( sEndpoint, sCommand, dResult );

	auto tReply = APIAnswer ( tOut, VER_COMMAND_JSON );
	tOut.SendString ( sEndpoint.cstr() );
	tOut.SendArray ( dResult );
}


void StatCountCommand ( SearchdCommand_e eCmd )
{
	if ( eCmd<SEARCHD_COMMAND_TOTAL )
		gStats ().m_iCommandCount[eCmd].fetch_add ( 1, std::memory_order_relaxed );
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

	if ( tStmt.m_eStmt==STMT_SHOW_INDEX_STATUS )
		return true;
	else if ( tStmt.m_eStmt == STMT_SET )
		return true;
	else if ( tStmt.m_eStmt != STMT_SELECT)
	{
		sError.SetSprintf ( "unhandled statement type (value=%d)", tStmt.m_eStmt );
		return false;
	}

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
static bool ApplyIndexKillList ( const CSphIndex * pIndex, CSphString & sWarning, CSphString & sError, bool bShowMessage )
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
			sWarning.SetSprintf ( "index '%s': applying killlist to itself", tIndex.m_sIndex.cstr() );
			continue;
		}

		auto pServed = GetServed ( tIndex.m_sIndex );
		if ( pServed )
		{
			RWIdx_c pTarget { pServed };
			// kill the docids provided by sql_query_killlist and similar
			if ( tIndex.m_uFlags & KillListTarget_t::USE_KLIST )
				pTarget->KillMulti ( dKillList );

			// kill all the docids present in this index
			if ( tIndex.m_uFlags & KillListTarget_t::USE_DOCIDS )
				pIndex->KillExistingDocids ( pTarget );
		}
		else
			sWarning.SetSprintf ( "index '%s' from killlist_target not found", tIndex.m_sIndex.cstr() );
	}

	return true;
}


// we don't rlock/wlock the index because we assume that we are being called from a place that already did that for us
bool ApplyKillListsTo ( CSphIndex* pKillListTarget, CSphString & sError )
{
	KillListTargets_c tTargets;

	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( const auto& tIt : *hLocal )
	{
		if ( tIt.first==pKillListTarget->GetName () || !tIt.second )
			continue;

		RWIdx_c pIndexWithKillList { tIt.second };
		CSphFixedVector<DocID_t> dKillList(0);
		tTargets.m_dTargets.Resize(0);
		if ( !pIndexWithKillList->LoadKillList ( &dKillList, tTargets, sError ) )
			return false;

		if ( !dKillList.GetLength() )
			continue;

		// if this index has 'our' index as its killlist_target, apply the killlist
		for ( const auto & tIndex : tTargets.m_dTargets )
			if ( tIndex.m_sIndex== pKillListTarget->GetName() )
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

bool PreloadKlistTarget ( const ServedDesc_t & tServed, RotateFrom_e eFrom, StrVec_t & dKlistTarget )
{
	switch ( eFrom )
	{
	case RotateFrom_e::NEW:
		return IndexFiles_c ( tServed.m_sIndexPath ).ReadKlistTargets ( dKlistTarget, ".new" );

	case RotateFrom_e::REENABLE:
		return IndexFiles_c ( tServed.m_sIndexPath ).ReadKlistTargets ( dKlistTarget );

	default:
		return false;
	}
}

static bool ApplyOthersKillListsToMe ( CSphIndex* pIndex, const char* szIndex, CSphString& sError )
{
	sphLogDebug ( "rotating index '%s': applying other indexes' killlists", szIndex );

	// apply other indexes' killlists to THIS index
	if ( !ApplyKillListsTo ( pIndex, sError ) )
	{
		sphWarning ( "rotating index '%s': %s", szIndex, sError.cstr() );
		return false;
	}

	sphLogDebug ( "rotating index '%s': applying other indexes' killlists... DONE", szIndex );
	return true;
}

static bool ApplyMyKillListsToOthers ( const CSphIndex* pIndex, const char* szIndex, CSphString& sError )
{
	sphLogDebug ( "rotating index '%s': apply killlist from this index to other indexes (killlist_target)", szIndex );

	// apply killlist from this index to other indexes (killlist_target)
	// if this fails, only show a warning
	CSphString sWarning;
	if ( !ApplyIndexKillList ( pIndex, sWarning, sError ) )
	{
		return false;
		sphWarning ( "rotating index '%s': %s", szIndex, sError.cstr() );
	}

	if ( sWarning.Length() )
		sphWarning ( "rotating index '%s': %s", szIndex, sWarning.cstr() );

	sphLogDebug ( "rotating index '%s': apply killlist from this index to other indexes (killlist_target)... DONE", szIndex );
	return true;
}

bool ApplyKilllistsMyAndToMe ( CSphIndex* pIdx, const char* szIndex, CSphString& sError )
{
	return ApplyOthersKillListsToMe ( pIdx, szIndex, sError ) && ApplyMyKillListsToOthers ( pIdx, szIndex, sError );
}

// tServed here might be one of:
// 1. Not yet served, and with .new ext. Need to be rotated, then loaded from scratch
// 2. Not yet served, need to be loaded from scratch
// 3. Served, but with now with .new. Need to be rotated, then loaded and and need to be rotated
bool RotateIndexGreedy ( const ServedIndex_c& tServed, const char* szIndex, CSphString& sError )
{
	assert ( tServed.m_eType == IndexType_e::PLAIN );
	sphLogDebug ( "RotateIndexGreedy for '%s' invoked", szIndex );

	//////////////////
	/// bool RotateIndexFilesGreedy ( const ServedDesc_t& tServed, const char* szIndex, CSphString& sError )
	//////////////////

	CheckIndexRotate_c tCheck ( tServed );
	if ( tCheck.NothingToRotate() )
		return false;

	IndexFiles_c dServedFiles ( tServed.m_sIndexPath, szIndex );
	IndexFiles_c dFreshFiles ( dServedFiles.MakePath ( tCheck.RotateFromNew() ? ".new" : "" ), szIndex );

	if ( !dFreshFiles.CheckHeader() )
	{
		// no files or wrong files - no rotation
		sError = dFreshFiles.ErrorMsg();
		return false;
	}

	if ( !dFreshFiles.HasAllFiles() )
	{
		sphWarning ( "rotating index '%s': unreadable: %s; abort rotation", szIndex, strerrorm ( errno ) );
		return false;
	}

	bool bHasOldServedFiles = dServedFiles.HasAllFiles();
	Optional_T<ActionSequence_c> tActions;

	if ( tCheck.RotateFromNew() )
	{
		tActions.emplace();
		if ( bHasOldServedFiles )
			tActions->Defer ( RenameFiles ( dServedFiles, "", ".old") );
		tActions->Defer ( RenameFiles ( dServedFiles, ".new", "" ) );

		// do files rotation
		if ( !tActions->RunDefers() )
		{
			bool bFatal;
			std::tie ( sError, bFatal ) = tActions->GetError();
			sphWarning ( "RotateIndexGreedy error: %s", sError.cstr() );
			if ( bFatal )
				sphFatal ( "RotateIndexGreedy error: %s", sError.cstr() ); // fixme! Do we really need to fatal? (adopted from prev version)
			return false;
		}
	}

	// try to use new index
	auto pIdx = UnlockedHazardIdxFromServed ( tServed ); // it should be locked, if necessary, before

	StrVec_t dWarnings;
	if ( !pIdx->Prealloc ( g_bStripPath, nullptr, dWarnings ) )
	{
		sphWarning ( "rotating index '%s': .new preload failed: %s", szIndex, pIdx->GetLastError().cstr() );
		if ( tActions )
		{
			if ( !tActions->UnRunDefers() )
			{
				bool bFatal;
				std::tie ( sError, bFatal ) = tActions->GetError();
				sphWarning ( "RotateIndexGreedy error: %s, NOT SERVING", sError.cstr() );
				if ( bFatal )
					sphFatal ( "RotateIndexGreedy error: %s", sError.cstr() ); // fixme! Do we really need to fatal? (adopted from prev version)
				return false;
			}

			sphLogDebug ( "PreallocIndexGreedy: has recovered. Prealloc it." );
			if ( !pIdx->Prealloc ( g_bStripPath, nullptr, dWarnings ) )
			{
				sError.SetSprintf ( "rotating index '%s': .new preload failed; ROLLBACK FAILED; INDEX UNUSABLE", szIndex );
				return false;
			}
		}
	}

	assert ( pIdx->GetTokenizer() && pIdx->GetDictionary() );

	for ( const auto& i : dWarnings )
		sphWarning ( "rotating index '%s': %s", szIndex, i.cstr() );

	if ( !pIdx->GetLastWarning().IsEmpty() )
		sphWarning ( "rotating index '%s': %s", szIndex, pIdx->GetLastWarning().cstr() );

	// unlink .old
	if ( bHasOldServedFiles )
		dServedFiles.Unlink (".old");

	// finalize
	if ( !ApplyKilllistsMyAndToMe ( pIdx, szIndex, sError ) )
		return false;

	// uff. all done
	sphInfo ( "rotating index '%s': success", szIndex );
	return true;
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

bool LockIndex ( ServedIndex_c& tIdx, CSphIndex* pIdx, CSphString& sError )
{
	if ( !g_bOptNoLock && !pIdx->Lock() )
	{
		sError.SetSprintf ( "lock: %s", pIdx->GetLastError().cstr() );
		return false;
	}

	tIdx.UpdateMass();
	return true;
}

// tricky bit
// fixup was initially intended for (very old) index formats that did not store dict/tokenizer settings
// however currently it also ends up configuring dict/tokenizer for fresh RT indexes!
// (and for existing RT indexes, settings get loaded during the Prealloc() call)
bool FixupAndLockIndex ( ServedIndex_c& tIdx, CSphIndex* pIdx, const CSphConfigSection* pConfig, const char* szIndexName, StrVec_t& dWarnings, CSphString& sError )
{
	if ( pConfig )
	{
		std::unique_ptr<FilenameBuilder_i> pFilenameBuilder = CreateFilenameBuilder ( szIndexName );
		if ( !sphFixupIndexSettings ( pIdx, *pConfig, g_bStripPath, pFilenameBuilder.get(), dWarnings, sError ) )
			return false;
	}

	// try to lock it
	return LockIndex ( tIdx, pIdx, sError );
}

/// this gets called for every new physical index
/// that is, local and RT indexes, but not distributed one
bool PreallocNewIndex ( ServedIndex_c & tIdx, const CSphConfigSection * pConfig, const char * szIndexName, StrVec_t & dWarnings, CSphString & sError )
{
	std::unique_ptr<FilenameBuilder_i> pFilenameBuilder = CreateFilenameBuilder ( szIndexName );
	CSphIndex* pIdx = UnlockedHazardIdxFromServed ( tIdx );
	assert (pIdx);
	if ( !pIdx->Prealloc ( g_bStripPath, pFilenameBuilder.get(), dWarnings ) )
	{
		sError.SetSprintf ( "prealloc: %s", pIdx->GetLastError().cstr() );
		return false;
	}
	return FixupAndLockIndex ( tIdx, pIdx, pConfig, szIndexName, dWarnings, sError );
}

// same as above, but self-load config section for given index
static bool PreallocNewIndex ( ServedIndex_c & tIdx, const char * szIndexName, StrVec_t & dWarnings, CSphString & sError )
{
	const CSphConfigSection * pIndexConfig = nullptr;
	CSphConfigSection tIndexConfig;
	{
		ScRL_t dRLockConfig { g_tRotateConfigMutex };
		if ( g_hCfg ( "index" ) )
			pIndexConfig = g_hCfg["index"] ( szIndexName );
		if ( pIndexConfig )
		{
			tIndexConfig = *pIndexConfig;
			pIndexConfig = &tIndexConfig;
		}
	}
	return PreallocNewIndex ( tIdx, pIndexConfig, szIndexName, dWarnings, sError );
}

// called either from MysqlReloadIndex, either from Rotation task (never from main thread).
bool RotateIndexMT ( ServedIndexRefPtr_c& pNewServed, const CSphString & sIndex, StrVec_t & dWarnings, CSphString & sError ) EXCLUDES ( MainThread )
{
	assert ( pNewServed && pNewServed->m_eType == IndexType_e::PLAIN );

	sphInfo ( "rotating index '%s': started", sIndex.cstr() );
	CheckIndexRotate_c tCheck ( *pNewServed );
	if ( tCheck.NothingToRotate() )
	{
		sError.SetSprintf ( "nothing to rotate for index '%s'", sIndex.cstr() );
		return false;
	}

	//////////////////
	/// load new index
	//////////////////
	CSphIndex* pNewIndex = UnlockedHazardIdxFromServed ( *pNewServed );
	if ( tCheck.RotateFromNew() )
		pNewIndex->SetBase ( IndexFiles_c::MakePath ( ".new", pNewServed->m_sIndexPath ) );

	// prealloc enough RAM and lock new index
	sphLogDebug ( "prealloc enough RAM and lock new index" );

	if ( !PreallocNewIndex ( *pNewServed, sIndex.cstr(), dWarnings, sError ) )
		return false;

	pNewIndex->Preread();
	pNewServed->UpdateMass(); // that is second update, first was at the end of Prealloc, this one is to correct after preread

	//////////////////////
	/// activate new index
	//////////////////////

	sphLogDebug ( "activate new index" );
	if ( tCheck.RotateFromNew() )
	{
		ActionSequence_c tActions;

		auto pServed = GetServed ( sIndex );
		if ( pServed && pServed->m_sIndexPath == pNewServed->m_sIndexPath )
			tActions.Defer ( RenameIdxSuffix ( pServed, ".old" ) );
		tActions.Defer ( RenameIdx ( pNewIndex, pNewServed->m_sIndexPath ) ); // rename 'new' to 'current'

		if ( !tActions.RunDefers() )
		{
			bool bFatal;
			std::tie ( sError, bFatal ) = tActions.GetError();
			sphWarning ( "RotateIndexMT error: index %s, error %s", sIndex.cstr(), sError.cstr() );
			if ( bFatal )
				g_pLocalIndexes->Delete ( sIndex );
			return false;
		}
		if ( pServed )
		{
			RIdx_c pOldIdx { pServed };
			pNewIndex->m_iTID = pOldIdx->m_iTID;
			pServed->SetUnlink ( pOldIdx->GetFilename() );
		}
	}

	if ( !ApplyKilllistsMyAndToMe ( pNewIndex, sIndex.cstr(), sError ) )
		return false;

	// all went fine; swap them
	sphLogDebug ( "all went fine; swap them" );
	Binlog::NotifyIndexFlush ( sIndex.cstr(), pNewIndex->m_iTID, false );
	g_pLocalIndexes->AddOrReplace ( pNewServed, sIndex );
	sphInfo ( "rotating index '%s': success", sIndex.cstr() );
	return true;
}

static void InvokeRotation ( VecOfServed_c&& dDeferredIndexes ) REQUIRES ( MainThread )
{
	assert ( !dDeferredIndexes.IsEmpty () && "Rotation queue must be checked before invoking rotation!");
	Threads::StartJob ( [dIndexes = std::move ( dDeferredIndexes )] () mutable
	{
		// want to track rotation thread only at work
		auto pDesc = PublishSystemInfo ( "ROTATION" );

		sphLogDebug ( "TaskRotation starts with %d deferred indexes", dIndexes.GetLength() );
		for ( auto& tIndex : dIndexes )
		{
			ServedIndexRefPtr_c& pReplacementServed = tIndex.second;
			const CSphString& sIndex = tIndex.first;

			// cluster indexes got managed by different path
			assert ( !ServedDesc_t::IsCluster ( pReplacementServed ) && "Rotation of clusters MUST never happens!" );

			// prealloc RT and percolate here
			StrVec_t dWarnings;
			CSphString sError;
			if ( ServedDesc_t::IsMutable ( pReplacementServed ) )
			{
				sphLogDebug ( "seamless rotate (prealloc) mutable index %s", sIndex.cstr() );
				if ( PreallocNewIndex ( *pReplacementServed, sIndex.cstr(), dWarnings, sError ) )
					g_pLocalIndexes->AddOrReplace ( pReplacementServed, sIndex );
				else
					sphWarning ( "index '%s': %s", sIndex.cstr(), sError.cstr() );
			} else
			{
				sphLogDebug ( "seamless rotate local index %s", sIndex.cstr() );
				if ( !RotateIndexMT ( pReplacementServed, sIndex, dWarnings, sError ) )
					sphWarning ( "index '%s': %s", sIndex.cstr(), sError.cstr() );
			}

			for ( const auto& i : dWarnings )
				sphWarning ( "index '%s': %s", sIndex.cstr(), i.cstr() );

			g_pDistIndexes->Delete ( sIndex ); // postponed delete of same-named distributed (if any)
		}

		g_bInRotate = false;
		RotateGlobalIdf();
		sphInfo ( "rotating index: all indexes done" );
	});
}

bool LimitedRotateIndexMT ( ServedIndexRefPtr_c& pNewServed, const CSphString& sIndex, StrVec_t& dWarnings, CSphString& sError ) EXCLUDES ( MainThread )
{
	assert ( Threads::IsInsideCoroutine() );

	// allow to run several rotations a time (in parallel)
	// vip conns has no limit
	if ( session::GetVip() )
		return RotateIndexMT ( pNewServed, sIndex, dWarnings, sError );

	// limit is arbitrary set to N/2 of threadpool
	static Coro::Waitable_T<int> iParallelRotations { 0 };
	iParallelRotations.Wait ( [] ( int i ) { return i < Max ( 1, NThreads() / 2 ); } );
	iParallelRotations.ModifyValue ( [] ( int& i ) { ++i; } );
	auto _ = AtScopeExit ( [] {
		iParallelRotations.ModifyValueAndNotifyOne ( [] ( int& i ) { --i; } );
	});
	return RotateIndexMT ( pNewServed, sIndex, dWarnings, sError );
}

void ConfigureLocalIndex ( ServedDesc_t * pIdx, const CSphConfigSection & hIndex, bool bMutableOpt, StrVec_t * pWarnings )
{
	pIdx->m_tSettings.Load ( hIndex, bMutableOpt, pWarnings );
	pIdx->m_sGlobalIDFPath = hIndex.GetStr ( "global_idf" );
}

void ConfigureDistributedIndex ( std::function<bool(const CSphString&)>&& fnCheck, DistributedIndex_t & tIdx, const char * szIndexName, const CSphConfigSection & hIndex, StrVec_t * pWarnings )
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
			if ( !fnCheck ( sLocal ) )
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

	bool bHaveHA = tIdx.m_dAgents.any_of ( [] ( const auto& ag ) { return ag->IsHA (); } );

	// configure ha_strategy
	if ( bSetHA && !bHaveHA )
		sphWarning ( "index '%s': ha_strategy defined, but no ha agents in the index", szIndexName );
}

//////////////////////////////////////////////////
/// configure distributed index and add it to hash
//////////////////////////////////////////////////
// AddIndex -> AddDistributedIndex
static ResultAndIndex_t AddDistributedIndex ( const char * szIndexName, const CSphConfigSection & hIndex, CSphString & sError, StrVec_t * pWarnings=nullptr )
{
	DistributedIndexRefPtr_t pIdx ( new DistributedIndex_t );
	ConfigureDistributedIndex ( [] ( const auto& sIdx ) { return g_pLocalIndexes->Contains ( sIdx ); }, *pIdx, szIndexName, hIndex, pWarnings );

	if ( pIdx->IsEmpty () )
	{
		sError.SetSprintf ( "index '%s': no valid local/remote indexes in distributed index", szIndexName );
		return { ADD_ERROR, nullptr };
	}

	// finally, check and add distributed index to global table
	if ( !g_pDistIndexes->Add ( pIdx, szIndexName ) )
	{
		sError.SetSprintf ( "index '%s': unable to add name (duplicate?)", szIndexName );
		return { ADD_ERROR, nullptr };
	}

	return ResultAndIndex_t { ADD_DISTR, nullptr };
}

// common preconfiguration of mutable indexes
static bool ConfigureRTPercolate ( CSphSchema & tSchema, CSphIndexSettings & tSettings, const char * szIndexName, const CSphConfigSection & hIndex, bool bWordDict, bool bPercolate, StrVec_t * pWarnings, CSphString & sError )
{
	// pick config settings
	// they should be overriden later by Preload() if needed
	{
		CSphString sWarning;
		if ( !tSettings.Setup ( hIndex, szIndexName, sWarning, sError ) )
		{
			sphWarning ( "index '%s': %s - NOT SERVING", szIndexName, sError.cstr() );
			return false;
		}

		if ( !sWarning.IsEmpty() )
			sphWarning ( "index '%s': %s", szIndexName, sWarning.cstr() );
	}

	if ( !sphRTSchemaConfigure ( hIndex, tSchema, tSettings, sError, bPercolate, bPercolate ) )
	{
		sphWarning ( "index '%s': %s - NOT SERVING", szIndexName, sError.cstr () );
		return false;
	}

	if ( bPercolate )
		FixPercolateSchema ( tSchema );

	if ( !sError.IsEmpty() )
	{
		if ( pWarnings )
			pWarnings->Add(sError);
		else
			sphWarning ( "index '%s': %s", szIndexName, sError.cstr () );
	}

	// path
	if ( !hIndex ( "path" ) )
	{
		sphWarning ( "index '%s': path must be specified - NOT SERVING", szIndexName );
		return false;
	}

	if ( !CheckStoredFields ( tSchema, tSettings, sError ) )
	{
		sphWarning ( "index '%s': %s - NOT SERVING", szIndexName, sError.cstr() );
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
			sphWarning ( "index '%s': has index_sp=%d, index_zones='%s' but disabled html_strip - NOT SERVING", szIndexName, iIndexSP, sIndexZones.cstr() );
			return false;
		}
		else
		{
			CSphString sWarning;
			sWarning.SetSprintf ( "has index_sp=%d but disabled html_strip - PARAGRAPH unavailable", iIndexSP );
			if ( pWarnings )
				pWarnings->Add(sWarning);
			else
				sphWarning ( "index '%s': %s", szIndexName, sWarning.cstr() );
		}
	}

	// upgrading schema to store field lengths
	if ( tSettings.m_bIndexFieldLens )
		if ( !AddFieldLens ( tSchema, false, sError ) )
		{
			sphWarning ( "index '%s': failed to create field lengths attributes: %s", szIndexName, sError.cstr () );
			return false;
		}

	if ( bWordDict && ( tSettings.m_dPrefixFields.GetLength () || tSettings.m_dInfixFields.GetLength () ) )
	{
		CSphString sWarning = "prefix_fields and infix_fields has no effect with dict=keywords, ignoring";
		if ( pWarnings )
			pWarnings->Add(sWarning);
		else
			sphWarning ( "index '%s': %s", szIndexName, sWarning.cstr() );
	}

	if ( bWordDict && tSettings.m_iMinInfixLen==1 )
	{
		CSphString sWarning = "min_infix_len must be greater than 1, changed to 2";
		if ( pWarnings )
			pWarnings->Add(sWarning);
		else
			sphWarning ( "index '%s': %s", szIndexName, sWarning.cstr() );

		tSettings.m_iMinInfixLen = 2;
	}

	tSchema.SetupFlags ( tSettings, bPercolate, pWarnings );
	return true;
}

///////////////////////////////////////////////
/// create, configure and load realtime index
///////////////////////////////////////////////
static ResultAndIndex_t LoadRTPercolate ( bool bRT, const char* szIndexName, const CSphConfigSection& hIndex, bool bMutableOpt, StrVec_t* pWarnings, CSphString& sError )
{
	bool bWordDict = true;
	if ( bRT )
	{
		auto sIndexType = hIndex.GetStr ( "dict", "keywords" );
		bWordDict = true;
		if ( sIndexType=="crc" )
			bWordDict = false;
		else if ( sIndexType!="keywords" )
		{
			sError.SetSprintf ( "index '%s': unknown dict=%s; only 'keywords' or 'crc' values allowed", szIndexName, sIndexType.cstr() );
			return { ADD_ERROR, nullptr };
		}
	}

	CSphSchema tSchema ( szIndexName );
	CSphIndexSettings tSettings;
	if ( !ConfigureRTPercolate ( tSchema, tSettings, szIndexName, hIndex, bWordDict, !bRT, pWarnings, sError ))
		return { ADD_ERROR, nullptr };

	// index
	auto pServed = MakeServedIndex();
	ConfigureLocalIndex ( pServed, hIndex, bMutableOpt, pWarnings );
	pServed->m_sIndexPath = hIndex["path"].strval();

	std::unique_ptr<CSphIndex> pIdx;
	if ( bRT )
	{
		pIdx = sphCreateIndexRT ( tSchema, szIndexName, pServed->m_tSettings.m_iMemLimit, pServed->m_sIndexPath.cstr(), bWordDict );
		pServed->m_eType = IndexType_e::RT;
	} else
	{
		pIdx = CreateIndexPercolate ( tSchema, szIndexName, pServed->m_sIndexPath.cstr() );
		pServed->m_eType = IndexType_e::PERCOLATE;
	}

	pIdx->SetMutableSettings ( pServed->m_tSettings );
	pIdx->m_iExpansionLimit = g_iExpansionLimit;
	pIdx->SetGlobalIDFPath ( pServed->m_sGlobalIDFPath );

	pIdx->Setup ( tSettings );
	pIdx->SetCacheSize ( g_iMaxCachedDocs, g_iMaxCachedHits );

	pServed->SetIdx ( std::move ( pIdx ) );
	return ResultAndIndex_t { ADD_NEEDLOAD, std::move ( pServed ) }; // use Leak to avoid extra addref/release on copying
}

////////////////////////////////////////////
/// configure and load local index
////////////////////////////////////////////
static ResultAndIndex_t LoadPlainIndex ( const char * szIndexName, const CSphConfigSection & hIndex, bool bMutableOpt, StrVec_t * pWarnings, CSphString & sError )
{
	// check path
	if ( !hIndex.Exists ( "path" ) )
	{
		sError = "key 'path' not found";
		return { ADD_ERROR, nullptr };
	}

	ServedIndexRefPtr_c pServed = MakeServedIndex();
	pServed->m_eType = IndexType_e::PLAIN;
	// configure memlocking, star
	ConfigureLocalIndex ( pServed, hIndex, bMutableOpt, pWarnings );

	// try to create index
	pServed->m_sIndexPath = hIndex["path"].strval ();
	auto pIdx = sphCreateIndexPhrase ( szIndexName, pServed->m_sIndexPath.cstr() );
	pIdx->m_iExpansionLimit = g_iExpansionLimit;
	pIdx->SetMutableSettings ( pServed->m_tSettings );
	pIdx->SetGlobalIDFPath ( pServed->m_sGlobalIDFPath );
	pIdx->SetCacheSize ( g_iMaxCachedDocs, g_iMaxCachedHits );
	pServed->SetIdx ( std::move ( pIdx ) );
	return ResultAndIndex_t { ADD_NEEDLOAD, std::move ( pServed ) };
}

///////////////////////////////////////////////
/// make and configure template index
///////////////////////////////////////////////
static ResultAndIndex_t LoadTemplateIndex ( const char * szIndexName, const CSphConfigSection &hIndex, bool bMutableOpt, StrVec_t * pWarnings )
{
	CSphIndexSettings tSettings;
	CSphString sWarning, sError;
	if ( !tSettings.Setup ( hIndex, szIndexName, sWarning, sError ) )
	{
		sphWarning ( "failed to configure index %s: %s", szIndexName, sError.cstr () );
		return { ADD_ERROR, nullptr };
	}

	if ( !sWarning.IsEmpty() )
		sphWarning ( "index '%s': %s - NOT SERVING", szIndexName, sWarning.cstr () );

	auto pIdx = sphCreateIndexTemplate ( szIndexName );
	pIdx->Setup ( tSettings );

	auto pServed = MakeServedIndex();
	pServed->m_eType = IndexType_e::TEMPLATE;

	// configure memlocking, star
	ConfigureLocalIndex ( pServed, hIndex, bMutableOpt, pWarnings );

	pIdx->SetMutableSettings ( pServed->m_tSettings );
	pIdx->m_iExpansionLimit = g_iExpansionLimit;

	std::unique_ptr<FilenameBuilder_i> pFilenameBuilder = CreateFilenameBuilder ( szIndexName );
	StrVec_t dWarnings;
	if ( !sphFixupIndexSettings ( pIdx.get(), hIndex, g_bStripPath, pFilenameBuilder.get(), dWarnings, sError ) )
	{
		sphWarning ( "index '%s': %s - NOT SERVING", szIndexName, sError.cstr () );
		return { ADD_ERROR, nullptr };
	}

	for ( const auto & i : dWarnings )
		sphWarning ( "index '%s': %s", szIndexName, i.cstr() );

	// templates we either add, either replace depending on requested action
	// at this point they are production-ready
	pServed->SetIdx ( std::move ( pIdx ) );
	return ResultAndIndex_t { ADD_SERVED, std::move ( pServed ) };
}

// HandleCommandClusterPq() -> RemoteLoadIndex() -> LoadIndex() -> AddIndex() // only Percolate! From other threads
// HandleMysqlCreateTable() -> CreateNewIndexConfigless() -> AddIndex() // from other threads
// ServiceMain() -> TickHead() -> CheckRotate() -> ReloadConfigAndRotateIndexes() -> AddIndex()
// ConfigureAndPreloadIndex() -> AddIndex() // maybe from non-main thread!
// 		ClientSession_c::Execute -> HandleMysqlImportTable -> AddExistingIndexConfigless -> ConfiglessPreloadIndex -> ConfigureAndPreloadIndex -> AddIndex
ResultAndIndex_t AddIndex ( const char * szIndexName, const CSphConfigSection & hIndex, bool bCheckDupe, bool bMutableOpt, StrVec_t * pWarnings, CSphString & sError )
{
	// check name
	if ( bCheckDupe && IndexIsServed ( szIndexName ) )
	{
		sphWarning ( "index '%s': duplicate name - NOT SERVING", szIndexName );
		return { ADD_ERROR, nullptr };
	}

	switch ( TypeOfIndexConfig ( hIndex.GetStr ( "type", nullptr )))
	{
		case IndexType_e::DISTR:
			return AddDistributedIndex ( szIndexName, hIndex, sError, pWarnings );
		case IndexType_e::RT:
			return LoadRTPercolate ( true, szIndexName, hIndex, bMutableOpt, pWarnings, sError );
		case IndexType_e::PERCOLATE:
			return LoadRTPercolate ( false, szIndexName, hIndex, bMutableOpt, pWarnings, sError );
		case IndexType_e::TEMPLATE:
			return LoadTemplateIndex ( szIndexName, hIndex, bMutableOpt, pWarnings );
		case IndexType_e::PLAIN:
			return LoadPlainIndex ( szIndexName, hIndex, bMutableOpt, pWarnings, sError );
		case IndexType_e::ERROR_:
		default:
			break;
	}

	sphWarning ( "index '%s': unknown type '%s' - NOT SERVING", szIndexName, hIndex["type"].cstr() );
	return { ADD_ERROR, nullptr };
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

#if _WIN32
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

	Dashboard::GetActiveHosts ().Apply ( [] ( HostDashboardRefPtr_t& pHost ) {
		if ( !pHost->m_pPersPool )
			pHost->m_pPersPool = new PersistentConnectionsPool_c;
		pHost->m_pPersPool->ReInit ( g_iPersistentPoolSize );
	} );
}

// special pass for 'simple' rotation (i.e. *.new to current)
static void IssuePlainOldRotation ( HashOfServed_c& hDeferred )
{
	ConfigReloader_c tReloader { hDeferred };
	tReloader.IssuePlainOldRotation();
}

// Reloading called always from same thread (so, for now not need to be th-safe for itself)
// ServiceMain() -> TickHead() -> CheckRotate() -> ReloadConfigAndRotateIndexes().
static void ReloadIndexesFromConfig ( const CSphConfig& hConf, HashOfServed_c& hDeferred ) REQUIRES ( MainThread )
{
	assert ( !IsConfigless() );
	if ( !hConf.Exists ("index") )
	{
		sphInfo ( "No indexes found in config came to rotation. Abort reloading");
		return;
	}

	ConfigReloader_c tReloader { hDeferred };
	for ( const auto& dIndex : hConf["index"] )
	{
		const auto & sIndexName = dIndex.first;
		const CSphConfigSection & hIndex = dIndex.second;
		IndexType_e eNewType = TypeOfIndexConfig ( hIndex.GetStr ( "type", nullptr ) );
		if ( eNewType==IndexType_e::ERROR_ )
			continue;

		tReloader.LoadIndexFromConfig ( sIndexName, eNewType, hIndex );
	}
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

static VecOfServed_c ConvertHashToPrioritySortedVec ( const HashOfServed_c& hDeferredIndexes ) REQUIRES ( MainThread )
{
	SmallStringHash_T<IndexWithPriority_t> tIndexesToRotate;
	VecOfServed_c dResult;

	for ( const auto& it : hDeferredIndexes )
	{
		assert ( it.second );

		// check for rt/percolate. they don't need killlist_target
		if ( !ServedDesc_t::IsMutable ( it.second ) && g_pLocalIndexes->Contains ( it.first ) )
		{
			IndexWithPriority_t tToRotate;
			tToRotate.m_sIndex = it.first;
			tToRotate.m_dKilllistTargets = it.second->m_dKilllistTargets;
			tIndexesToRotate.Add ( std::move ( tToRotate ), it.first );
		}
		else
			dResult.Add ( { it.first, it.second } ); // out or priority - will be processed first.
	};

	// set priorities
	for ( const auto& tIndexToRotate : tIndexesToRotate )
		for ( const auto & i : tIndexToRotate.second.m_dKilllistTargets )
		{
			IndexWithPriority_t * pIdx = tIndexesToRotate(i);
			if ( pIdx )
				++pIdx->m_nReferences;
		}

	// start with the least-referenced index
	IndexWithPriority_t * pMin;
	do
	{
		pMin = nullptr;
		for ( auto & tIndexToRotate : tIndexesToRotate )
		{
			auto & tIdx = tIndexToRotate.second;
			if ( tIdx.m_iPriority==-1 && ( !pMin || tIdx.m_nReferences<pMin->m_nReferences ) )
				pMin = &tIdx;
		}

		if ( pMin )
			SetIndexPriority ( *pMin, 0, tIndexesToRotate );
	}
	while ( pMin );

	// collect and sort by priority processed indexes names
	StrVec_t dSorted;
	for ( auto& tIndexToRotate : tIndexesToRotate )
		dSorted.Add ( tIndexToRotate.first );
	dSorted.Sort ( Lesser ( [&tIndexesToRotate] ( auto a, auto b ) { return tIndexesToRotate[a].m_iPriority < tIndexesToRotate[a].m_iPriority; } ) );

	// append priority names to non-prioritized
	for ( const auto& sIdx : dSorted )
		dResult.Add ( { sIdx, hDeferredIndexes[sIdx] } );

	return dResult;
}

// ServiceMain() -> TickHead() -> CheckRotate() -> CheckIndexesForSeamlessAndStartRotation()
static void CheckIndexesForSeamlessAndStartRotation ( VecOfServed_c dDeferredIndexes ) REQUIRES ( MainThread )
{
	// check what indexes need to be rotated
	int iNotCapableForSeamlessRotation = 0;
	ARRAY_FOREACH ( i, dDeferredIndexes )
	{
		const auto& sIdx = dDeferredIndexes[i].first;
		auto* pIndex = dDeferredIndexes[i].second.Ptr();
		assert ( pIndex );

		if ( !ServedDesc_t::IsMutable ( pIndex ) && CheckIndexRotate_c ( *pIndex ).NothingToRotate() )
		{
			++iNotCapableForSeamlessRotation;
			sphLogDebug ( "queue[] = %s", sIdx.cstr() );
			sphLogDebug ( "Index %s (%s) is not capable for seamless rotate. Skipping", sIdx.cstr ()
				, pIndex->m_sIndexPath.cstr () );
			dDeferredIndexes.Remove(i--);
		}
	}

	if ( iNotCapableForSeamlessRotation )
		sphWarning ( "internal error: non-empty queue on a rotation cycle start, got %d elements", iNotCapableForSeamlessRotation );

	if ( dDeferredIndexes.IsEmpty () )
	{
		sphInfo ( "nothing to rotate after SIGHUP" );
		g_bInRotate = false;
		return;
	}

	InvokeRotation ( std::move ( dDeferredIndexes ) );
}

// hDeferredIndexes includes both - fresh new, changed and 'just new to current' indexes.
static void DoGreedyRotation ( VecOfServed_c&& dDeferredIndexes ) REQUIRES ( MainThread )
{
	assert ( !g_bSeamlessRotate );
	ScRL_t tRotateConfigMutex { g_tRotateConfigMutex };

	for ( auto& dDeferredIndex :  dDeferredIndexes )
	{
		const CSphString& sDeferredIndex = dDeferredIndex.first;
		ServedIndexRefPtr_c& pDeferredIndex = dDeferredIndex.second;
		assert ( pDeferredIndex );

		CSphString sError;
		StrVec_t dWarnings;

		// prealloc RT and percolate here
		if ( ServedDesc_t::IsMutable ( pDeferredIndex ) )
		{
			sphLogDebug ( "greedy rotate (prealloc) mutable %s", sDeferredIndex.cstr() );

			if ( PreallocNewIndex ( *pDeferredIndex, &g_hCfg["index"][sDeferredIndex], sDeferredIndex.cstr(), dWarnings, sError ) )
				g_pLocalIndexes->AddOrReplace ( pDeferredIndex, sDeferredIndex );
			else
				sphWarning ( "index '%s': %s - NOT SERVING", sDeferredIndex.cstr(), sError.cstr() );
		}
		else if ( pDeferredIndex->m_eType==IndexType_e::PLAIN )
		{
			sphLogDebug ( "greedy rotate local %s", sDeferredIndex.cstr() );
			auto pRotating = GetServed ( sDeferredIndex );
			bool bSame = pRotating && pRotating.Ptr() == pDeferredIndex.Ptr();
			WIdx_c WIdx { pDeferredIndex };
			bool bOk = RotateIndexGreedy ( *pDeferredIndex, sDeferredIndex.cstr(), sError );
			if ( !bOk )
				sphWarning ( "index '%s': %s - NOT SERVING", sDeferredIndex.cstr(), sError.cstr() );

			if ( !bSame && bOk && !sphFixupIndexSettings ( WIdx, g_hCfg["index"][sDeferredIndex], g_bStripPath, nullptr, dWarnings, sError ) )
			{
				sphWarning ( "index '%s': %s - NOT SERVING", sDeferredIndex.cstr(), sError.cstr() );
				bOk = false;
			}

			if ( bOk )
			{
				WIdx->Preread();
				pDeferredIndex->UpdateMass();
				g_pLocalIndexes->AddOrReplace ( pDeferredIndex, sDeferredIndex );
			}
		}

		for ( const auto & i : dWarnings )
			sphWarning ( "index '%s': %s", sDeferredIndex.cstr(), i.cstr() );

		g_pDistIndexes->Delete ( sDeferredIndex ); // postponed delete of same-named distributed (if any)
	}

//	assert ( dDeferredIndexes.IsEmpty() );
	g_bInRotate = false;
	RotateGlobalIdf ();
	sphInfo ( "rotating finished" );
}


// ServiceMain() -> TickHead() -> [CallCoroutine] -> CheckRotate()
static void CheckRotate () REQUIRES ( MainThread )
{
	// do we need to rotate now? If no sigHUP received, or if we are already rotating - no.
//	if ( !g_bNeedRotate || g_bInRotate || IsConfigless() )
//		return;

	assert ( !IsConfigless() );

	g_bInRotate = true; // ok, another rotation cycle just started
	g_bNeedRotate = false; // which therefore clears any previous HUP signals

	sphLogDebug ( "CheckRotate invoked" );

	bool bReloadHappened = false;
	HashOfServed_c hDeferredIndexes;
	{
		if ( LoadAndCheckConfig () || g_bReloadForced )
		{
			sphInfo( "Config changed (read %d chars)", g_dConfig.GetLength());
			if ( !g_dConfig.IsEmpty() )
			{
				{
					ScWL_t dRotateConfigMutexWlocked { g_tRotateConfigMutex };
					bReloadHappened = ParseConfig ( &g_hCfg, g_sConfigFile.cstr (), g_dConfig.begin ());
				}
				if ( bReloadHappened )
				{
					ScRL_t dRotateConfigMutexRlocked { g_tRotateConfigMutex };
					ReloadIndexesFromConfig ( g_hCfg, hDeferredIndexes );
				} else
					sphWarning ( "failed to parse config file '%s': %s; using previous settings", g_sConfigFile.cstr(), TlsMsg::szError() );
			}
		}
		CleanLoadedConfig();
		g_bReloadForced = false;
	}

	if ( !bReloadHappened )
		IssuePlainOldRotation ( hDeferredIndexes );

	VecOfServed_c dDeferredIndexes = ConvertHashToPrioritySortedVec ( hDeferredIndexes );

	for ( const auto& s : dDeferredIndexes )
		sphLogDebug ( "will rotate %s", s.first.cstr() );

	if ( g_bSeamlessRotate )
		CheckIndexesForSeamlessAndStartRotation ( std::move ( dDeferredIndexes ) );
	else
		DoGreedyRotation ( std::move ( dDeferredIndexes ) );
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

#if !_WIN32
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
	sDesc.SetSprintf ( "%s-%s", g_sServiceName, g_sStatusVersion.cstr() );

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
#endif // _WIN32


void ShowHelp ()
{
	fprintf ( stdout,
		"Usage: searchd [OPTIONS]\n"
		"\n"
		"Options are:\n"
		"-h, --help\t\tdisplay this help message\n"
		"-v, --version\t\tdisplay version information\n"
		"-c, --config <file>\tread configuration from specified file\n"
		"\t\t\t(default is manticore.conf)\n"
		"--stop\t\t\tsend SIGTERM to currently running searchd\n"
		"--stopwait\t\tsend SIGTERM and wait until actual exit\n"
		"--status\t\tget ant print status variables\n"
		"\t\t\t(PID is taken from pid_file specified in config file)\n"
		"--iostats\t\tlog per-query io stats\n"
		"--cpustats\t\tlog per-query cpu stats\n"
#if _WIN32
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
		"--new-cluster\tbootstraps a replication cluster with cluster restart protection\n"
		"--new-cluster-force\tbootstraps a replication cluster without cluster restart protection\n"
		"\n"
		"Debugging options are:\n"
		"--console\t\trun in console mode (do not fork, do not log to files)\n"
		"-p, --port <port>\tlisten on given port (overrides config setting)\n"
		"-l, --listen <spec>\tlisten on given address, port or path (overrides\n"
		"\t\t\tconfig settings)\n"
		"-i, --index <index>\tonly serve given index(es)\n"
#if !_WIN32
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
#if _WIN32
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


#if _WIN32
BOOL WINAPI CtrlHandler ( DWORD )
{
	if ( !g_bService )
		sphInterruptNow();
	return TRUE;
}
#endif


#if !_WIN32

static char g_sNameBuf[512] = { 0 };
static char g_sPid[30] = { 0 };

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
					sSig = "SIGINT";
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
			if ( g_bGotSigusr2 )
			{
				g_bGotSigusr2 = 0;
				sphInfo ( "watchdog: got USR2, performing dump of child's stack" );
				sphDumpGdb ( g_iLogFile, g_sNameBuf, g_sPid );
			}
		}

		if ( bShutdown || sphInterrupted() || g_pShared->m_bDaemonAtShutdown )
		{
			exit ( 0 );
		}
	}
}
#else
const int		WIN32_PIPE_BUFSIZE=32;
#endif // !_WIN32

/// check for incoming signals, and react on them
void CheckSignals () REQUIRES ( MainThread )
{
#if _WIN32
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
		g_bGotSighup = false;
	}

	if ( sphInterrupted() )
	{
		sphInfo ( "caught SIGTERM, shutting down" );
		Shutdown ();
		exit ( 0 );
	}

#if _WIN32

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

void TickHead () REQUIRES ( MainThread )
{
	CheckSignals ();
	CheckLeaks ();
	CheckReopenLogs ();
	if ( g_bNeedRotate && !g_bInRotate && !IsConfigless() )
		Threads::CallCoroutine ( [] {
			ScopedRole_c thMain ( MainThread );
			CheckRotate();
		} );
	sphInfo ( nullptr ); // flush dupes
#if _WIN32
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

static std::unique_ptr<QueryParser_i> PercolateQueryParserFactory ( bool bJson )
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
		sphInfo ( "TCP fast open unavailable (can't read /proc/sys/net/ipv4/tcp_fastopen, look Server_settings/Searchd#Technical-details-about-Sphinx-API-protocol-and-TFO in manual)" );
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

static void ConfigureDaemonLog ( const CSphString & sMode )
{
	if ( sMode.IsEmpty() )
		return;

	StrVec_t dOpts = sphSplit ( sMode.cstr(), "," );

	SmallStringHash_T<int> hStmt;
	for ( int i=0; i<(int)( sizeof(g_dSqlStmts)/sizeof(g_dSqlStmts[0]) ); i++ )
		hStmt.Add ( i, g_dSqlStmts[i] );

	CSphBitvec tLogStatements ( STMT_TOTAL );
	StringBuilder_c sWrongModes ( "," );

	for ( const CSphString & sOpt : dOpts )
	{
		if ( sOpt=="0" ) // emplicitly disable all statements
			return;

		if ( sOpt=="1" || sOpt=="*" ) // enable all statements
		{
			tLogStatements.Set();
			g_tLogStatements = tLogStatements;
			return;
		}

		// check for whole statement enumerated
		int * pMode = hStmt ( sOpt );
		if ( pMode )
		{
			tLogStatements.BitSet ( *pMode );
			continue;
		}

		bool bHasWild = false;
		for ( const char * s = sOpt.cstr(); *s && !bHasWild; s++ )
			bHasWild = sphIsWild ( *s );

		if ( bHasWild )
		{
			bool bMatched = false;
			for ( int i=0; i<(int)( sizeof(g_dSqlStmts)/sizeof(g_dSqlStmts[0]) ); i++ )
			{
				if ( sphWildcardMatch ( g_dSqlStmts[i], sOpt.cstr() ) )
				{
					tLogStatements.BitSet ( i );
					bMatched = true;
					break;
				}
			}

			if ( bMatched )
				continue;
		}

		sWrongModes += sOpt.cstr();
	}

	if ( tLogStatements.BitCount() )
		g_tLogStatements = tLogStatements;

	if ( !sWrongModes.IsEmpty() )
		sphWarning ( "query_log_statements invalid values: %s", sWrongModes.cstr() );
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
	MutableIndexSettings_c::GetDefaults().m_bPreopen = hSearchd.GetBool ( "preopen_indexes" );
	sphSetUnlinkOld ( hSearchd.GetBool ( "unlink_old" ) );
	g_iExpansionLimit = hSearchd.GetInt ( "expansion_limit" );

	// initialize buffering settings
	SetUnhintedBuffer ( hSearchd.GetSize( "read_unhinted", DEFAULT_READ_UNHINTED ) );
	int iReadBuffer = hSearchd.GetSize ( "read_buffer", DEFAULT_READ_BUFFER );
	FileAccessSettings_t & tDefaultFA = MutableIndexSettings_c::GetDefaults().m_tFileAccess;
	tDefaultFA.m_iReadBufferDocList = hSearchd.GetSize ( "read_buffer_docs", iReadBuffer );
	tDefaultFA.m_iReadBufferHitList = hSearchd.GetSize ( "read_buffer_hits", iReadBuffer );
	tDefaultFA.m_eDoclist = GetFileAccess( hSearchd, "access_doclists", true, FileAccess_e::FILE );
	tDefaultFA.m_eHitlist = GetFileAccess( hSearchd, "access_hitlists", true, FileAccess_e::FILE );

	tDefaultFA.m_eAttr = FileAccess_e::MMAP_PREREAD;
	tDefaultFA.m_eBlob = FileAccess_e::MMAP_PREREAD;

	tDefaultFA.m_eAttr = GetFileAccess( hSearchd, "access_plain_attrs", false, tDefaultFA.m_eAttr );
	tDefaultFA.m_eBlob = GetFileAccess( hSearchd, "access_blob_attrs", false, tDefaultFA.m_eBlob );

	if ( hSearchd("subtree_docs_cache") )
		g_iMaxCachedDocs = hSearchd.GetSize ( "subtree_docs_cache", g_iMaxCachedDocs );

	if ( hSearchd("subtree_hits_cache") )
		g_iMaxCachedHits = hSearchd.GetSize ( "subtree_hits_cache", g_iMaxCachedHits );

	if ( hSearchd("seamless_rotate") )
		g_bSeamlessRotate = ( hSearchd["seamless_rotate"].intval()!=0 );

	if ( hSearchd ( "grouping_in_utc" ) )
	{
		g_bGroupingInUtc = (hSearchd["grouping_in_utc"].intval ()!=0);
		SetGroupingInUtcExpr ( g_bGroupingInUtc );
		SetGroupingInUtcSort ( g_bGroupingInUtc );
	}

	// sha1 password hash for shutdown action
	g_sShutdownToken = hSearchd.GetStr ("shutdown_token");

	if ( !g_bSeamlessRotate && MutableIndexSettings_c::GetDefaults().m_bPreopen && !bTestMode )
		sphWarning ( "preopen_indexes=1 has no effect with seamless_rotate=0" );

	SetAttrFlushPeriod ( hSearchd.GetUsTime64S ( "attr_flush_period", 0 ));
	g_iMaxPacketSize = hSearchd.GetSize ( "max_packet_size", g_iMaxPacketSize );
	g_iMaxFilters = hSearchd.GetInt ( "max_filters", g_iMaxFilters );
	g_iMaxFilterValues = hSearchd.GetInt ( "max_filter_values", g_iMaxFilterValues );
	g_iMaxBatchQueries = hSearchd.GetInt ( "max_batch_queries", g_iMaxBatchQueries );
	g_iDistThreads = hSearchd.GetInt ( "max_threads_per_query", g_iDistThreads );
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
		GlobalCollation () = sphCollationFromName ( sCollation, &sError );
		if ( !sError.IsEmpty() )
			sphWarning ( "%s", sError.cstr() );
	}

	if ( hSearchd("thread_stack") ) // fixme! rename? That is limit for stack of the coro, not of the thread!
	{
		constexpr int iThreadStackSizeMin = 128*1024;
		int iStackSize = hSearchd.GetSize ( "thread_stack", iThreadStackSizeMin );
		if ( iStackSize<iThreadStackSizeMin )
			sphWarning ( "thread_stack %d less than default (128K), increased", iStackSize );

		iStackSize = Max ( iStackSize, iThreadStackSizeMin );
		Threads::SetMaxCoroStackSize ( iStackSize );
	}

	if ( hSearchd("predicted_time_costs") )
		ParsePredictedTimeCosts ( hSearchd["predicted_time_costs"].cstr() );

	if ( hSearchd("shutdown_timeout") )
		g_iShutdownTimeoutUs = hSearchd.GetUsTime64S ( "shutdown_timeout", 60000000);

	g_iDocstoreCache = hSearchd.GetSize64 ( "docstore_cache_size", 16777216 );
	g_iSkipCache = hSearchd.GetSize64 ( "skiplist_cache_size", 67108864 );

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

	g_sMySQLVersion = hSearchd.GetStr ( "mysql_version_string", g_sMySQLVersion.cstr() );

	AllowOnlyNot ( hSearchd.GetInt ( "not_terms_only_allowed", 0 )!=0 );
	ConfigureDaemonLog ( hSearchd.GetStr ( "query_log_commands" ) );
	g_iAutoOptimizeCutoffMultiplier = hSearchd.GetInt ( "auto_optimize", 1 );
	MutableIndexSettings_c::GetDefaults().m_iOptimizeCutoff = hSearchd.GetInt ( "optimize_cutoff", AutoOptimizeCutoff() );

	g_bSplit = hSearchd.GetInt ( "pseudo_sharding", 1 )!=0;

	bool bGotSecondary = ( hSearchd.GetInt ( "secondary_indexes", GetSecondaryIndexDefault() )!=0 );
	if ( bGotSecondary && !IsSecondaryLibLoaded() )
		sphFatal ( "secondary_indexes set but failed to initialize secondary library: %s", g_sSecondaryError.cstr() );

	SetSecondaryIndexDefault ( bGotSecondary );
	g_sConfigPath = sphGetCwd();
}

static void PutPath ( const CSphString & sCwd, const CSphString & sVar, RowBuffer_i & tOut )
{
	if ( !IsPathAbsolute ( sVar ) )
	{
		CSphString sPath;
		sPath.SetSprintf ( "%s/%s", sCwd.cstr(), sVar.cstr() );
		tOut.PutString ( sPath );
	} else
	{
		tOut.PutString ( sVar );
	}
}

class StringSetStatic_c : public sph::StringSet
{
public:
	StringSetStatic_c ( std::initializer_list<const char *> dArgs )
	{
		for ( const char * sName : dArgs )
			Add ( sName );
	}
};

static StringSetStatic_c g_hSearchdPathVars {
  "binlog_path"
, "data_dir"
, "lemmatizer_base"
, "log"
, "pid_file"
, "plugin_dir"
, "query_log"
, "snippets_file_prefix"
, "sphinxql_state" 
, "ssl_ca"
, "ssl_cert"
, "ssl_key"
};

static void DumpSettingsSection ( const CSphConfig & hConf, const char * sSectionName, RowBuffer_i & tOut )
{
	if ( !hConf.Exists ( sSectionName ) || !hConf[sSectionName].Exists ( sSectionName ) )
		return;

	StringBuilder_c tTmp;
	const CSphConfigSection & hNode = hConf[sSectionName][sSectionName];

	for ( const auto & tIt : hNode )
	{
		tTmp.Clear();
		tTmp.Appendf ( "%s.%s", sSectionName, tIt.first.cstr() );

		const CSphVariant * pVal = &tIt.second;

		do
		{
			// data packets
			tOut.PutString ( tTmp.cstr() );

			if ( g_hSearchdPathVars[tIt.first] )
				PutPath ( g_sConfigPath, pVal->strval(), tOut );
			else
				tOut.PutString ( pVal->strval() );

			tOut.Commit();

			pVal = pVal->m_pNext;
		} while ( pVal );
	}
}

void HandleMysqlShowSettings ( const CSphConfig & hConf, RowBuffer_i & tOut )
{
	tOut.HeadBegin( 2 );
	tOut.HeadColumn ( "Setting_name" );
	tOut.HeadColumn ( "Value" );
	tOut.HeadEnd();

	// configuration file path
	tOut.PutString ( "configuration_file" );
	PutPath ( g_sConfigPath, g_sConfigFile, tOut );
	tOut.Commit();
	// pid 
	tOut.PutString ( "worker_pid" );
	tOut.PutNumAsString ( (int)getpid() );
	tOut.Commit();

	DumpSettingsSection ( hConf, "searchd", tOut );
	DumpSettingsSection ( hConf, "common", tOut );
	DumpSettingsSection ( hConf, "indexer", tOut );

	// done
	tOut.Eof();

}

// load index which is not yet load, and publish it in served indexes.
// ServiceMain -> ConfigureAndPreloadOnStartup -> ConfigureAndPreloadIndex
// ServiceMain -> ConfigureAndPreloadOnStartup -> ConfigureAndPreloadConfiglessIndexes -> ConfiglessPreloadIndex -> ConfigureAndPreloadIndex
// from any another thread:
// ClientSession_c::Execute -> HandleMysqlImportTable -> AddExistingIndexConfigless -> ConfiglessPreloadIndex -> ConfigureAndPreloadIndex
ESphAddIndex ConfigureAndPreloadIndex ( const CSphConfigSection & hIndex, const char * sIndexName, StrVec_t & dWarnings, CSphString & sError )
{
	ESphAddIndex eAdd;
	ServedIndexRefPtr_c pJustLoadedLocal;
	std::tie ( eAdd, pJustLoadedLocal ) = AddIndex ( sIndexName, hIndex, true, false, nullptr, sError );

	// local plain, rt, percolate added, but need to be at least preallocated before they could work.
	switch ( eAdd )
	{
	case ADD_NEEDLOAD:
	{
		assert ( pJustLoadedLocal );
		fprintf ( stdout, "precaching index '%s'\n", sIndexName );
		fflush ( stdout );

		IndexFiles_c dJustAddedFiles ( pJustLoadedLocal->m_sIndexPath );

		if ( dJustAddedFiles.HasAllFiles ( ".new" ) )
		{
			WIdx_c WFake { pJustLoadedLocal }; // as RotateIndexGreedy wants w-locked
			if ( RotateIndexGreedy ( *pJustLoadedLocal, sIndexName, sError ) )
			{
				if ( !FixupAndLockIndex ( *pJustLoadedLocal, UnlockedHazardIdxFromServed ( *pJustLoadedLocal ), &hIndex, sIndexName, dWarnings, sError ) )
					return ADD_ERROR;
			} else
			{
				dWarnings.Add ( sError );
				if ( !PreallocNewIndex ( *pJustLoadedLocal, &hIndex, sIndexName, dWarnings, sError ) )
					return ADD_ERROR;
			}
		} else if ( !PreallocNewIndex ( *pJustLoadedLocal, &hIndex, sIndexName, dWarnings, sError ) )
			return ADD_ERROR;
	}
	// no break
	case ADD_SERVED:
	{
		// finally add the index to the hash of enabled.
		g_pLocalIndexes->Add ( pJustLoadedLocal, sIndexName );

		if ( !pJustLoadedLocal->m_sGlobalIDFPath.IsEmpty() && !sph::PrereadGlobalIDF ( pJustLoadedLocal->m_sGlobalIDFPath, sError ) )
			dWarnings.Add ( "global IDF unavailable - IGNORING" );
	}
	// no sense to break
	case ADD_DISTR:
	case ADD_ERROR:
	default:
		break;
	}
	return eAdd;
}

// invoked once on start from ServiceMain (actually it creates the hashes)
// ServiceMain -> ConfigureAndPreloadOnStartup
static void ConfigureAndPreloadOnStartup ( const CSphConfig & hConf, const StrVec_t & dOptIndexes ) REQUIRES (MainThread)
{
	int iCounter = 0;
	int iValidIndexes = 0;
	int64_t tmLoad = -sphMicroTimer();

	if ( hConf.Exists ( "index" ) )
	{
		assert ( !IsConfigless() );
		for ( const auto& tIndex : hConf["index"] )
		{
			const CSphConfigSection & hIndex = tIndex.second;
			const char * sIndexName = tIndex.first.cstr();

			if ( !dOptIndexes.IsEmpty() && !dOptIndexes.any_of ( [&] ( const CSphString &rhs ) { return rhs.EqN ( sIndexName ); } ) )
				continue;

			StrVec_t dWarnings;
			CSphString sError;
			ESphAddIndex eAdd = ConfigureAndPreloadIndex ( hIndex, sIndexName, dWarnings, sError );
			for ( const auto & i : dWarnings )
				sphWarning ( "index '%s': %s", sIndexName, i.cstr() );

			if ( eAdd==ADD_ERROR )
				sphWarning ( "index '%s': %s - NOT SERVING", sIndexName, sError.cstr() );

			iValidIndexes += ( eAdd!=ADD_ERROR ? 1 : 0 );
			iCounter +=  ( eAdd== ADD_NEEDLOAD ? 1 : 0 );
		}
	} else {
		assert ( IsConfigless() );
		ConfigureAndPreloadConfiglessIndexes ( iValidIndexes, iCounter );
	}

	InitPersistentPool();

	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( const auto& tIt : *hLocal )
	{
		auto pServed = tIt.second;
		if ( pServed )
		{
			CSphString sWarning, sError;
			RIdx_c pIdx { pServed };
			if ( !ApplyIndexKillList ( pIdx, sWarning, sError, true ) )
				sphWarning ( "index '%s': error applying killlist: %s", pIdx->GetName(), sError.cstr() );

			if ( sWarning.Length() )
				sphWarning ( "%s", sWarning.cstr() );
		}
	}

	// set index cluster name for check
	for ( const ClusterDesc_t & tClusterDesc : GetClustersInt() )
		for ( const CSphString & sIndexName : tClusterDesc.m_dIndexes )
			SetIndexCluster ( sIndexName, tClusterDesc.m_sName );
	sphLogDebugRpl ( "%d clusters loaded from config", GetClustersInt().GetLength() );


	tmLoad += sphMicroTimer();
	if ( !iValidIndexes )
		sphLogDebug ( "no valid indexes to serve" );
	else
		fprintf ( stdout, "precached %d indexes in %0.3f sec\n", iCounter, float(tmLoad)/1000000 );
}


static CSphString FixupFilename ( const CSphString & sFilename )
{
	CSphString sFixed = sFilename;

#if _WIN32
	sFixed = AppendWinInstallDir(sFixed);
#endif

	return sFixed;
}


void OpenDaemonLog ( const CSphConfigSection & hSearchd, bool bCloseIfOpened=false )
{
	CSphString sLog = "searchd.log";
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
			sLog = FixupFilename ( hSearchd["log"].cstr() );
	}

	umask ( 066 );
	if ( bCloseIfOpened && g_iLogFile!=STDOUT_FILENO )
	{
		close ( g_iLogFile );
		g_iLogFile = STDOUT_FILENO;
	}
	if ( !g_bLogSyslog )
	{
		g_iLogFile = open ( sLog.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
		if ( g_iLogFile<0 )
		{
			g_iLogFile = STDOUT_FILENO;
			sphFatal ( "failed to open log file '%s': %s", sLog.cstr(), strerrorm(errno) );
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

	CSphString sPidFile = FixupFilename ( v->cstr () );
	FILE * fp = fopen ( sPidFile.cstr(), "r" );
	if ( !fp )
		sphFatal ( "stop: pid file '%s' does not exist or is not readable", sPidFile.cstr() );

	char sBuf[16];
	int iLen = (int) fread ( sBuf, 1, sizeof(sBuf)-1, fp );
	sBuf[iLen] = '\0';
	fclose ( fp );

	int iPid = atoi(sBuf);
	if ( iPid<=0 )
		sphFatal ( "stop: failed to read valid pid from '%s'", sPidFile.cstr() );

	int iWaitTimeout = g_iShutdownTimeoutUs + 100000;

#if _WIN32
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
                int iMask = umask ( 0 );
		iPipeCreated = mkfifo ( sPipeName.cstr(), 0666 );
                umask ( iMask );
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


static void InitBanner()
{
	const char * szColumnarVer = GetColumnarVersionStr();
	CSphString sColumnar = "";
	if ( szColumnarVer )
		sColumnar.SetSprintf ( " (columnar %s)", szColumnarVer );

	const char * sSiVer = GetSecondaryVersionStr();
	CSphString sSi = "";
	if ( sSiVer )
		sSi.SetSprintf ( " (secondary %s)", sSiVer );

	g_sBannerVersion.SetSprintf ( "%s%s%s", szMANTICORE_NAME, sColumnar.cstr(), sSi.cstr() );
	g_sBanner.SetSprintf ( "%s%s", g_sBannerVersion.cstr(), szMANTICORE_BANNER_TEXT );
	g_sMySQLVersion.SetSprintf ( "%s%s%s", szMANTICORE_VERSION, sColumnar.cstr(), sSi.cstr() );
	g_sStatusVersion.SetSprintf ( "%s%s%s", szMANTICORE_VERSION, sColumnar.cstr(), sSi.cstr() );
}

static void CheckSSL ()
{
	// check for SSL inited well
	for ( const auto & tListener : g_dListeners )
	{
		CSphString sError;
		if ( tListener.m_eProto==Proto_e::HTTPS )
		{
			if ( !CheckWeCanUseSSL ( &sError ) )
				sphWarning ( "SSL init error: %s", sError.cstr() );

			break;
		}
	}
}


int WINAPI ServiceMain ( int argc, char **argv ) EXCLUDES (MainThread)
{
	ScopedRole_c thMain (MainThread);
	g_bLogTty = isatty ( g_iLogFile )!=0;

#ifdef USE_VTUNE
	__itt_pause ();
#endif // USE_VTUNE
	g_tmStarted = sphMicroTimer();

#if _WIN32
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

	CSphString sError;
	// initialize it before other code to fetch version string for banner
	bool bColumnarError = !InitColumnar ( sError );
	g_bSecondaryError = !InitSecondary ( g_sSecondaryError );
	sphCollationInit ();

	InitBanner();

	if ( !g_bService )
		fprintf ( stdout, "%s",  g_sBanner.cstr() );

	if ( bColumnarError )
		sphWarning ( "Error initializing columnar storage: %s", sError.cstr() );
	if ( g_bSecondaryError )
		sphWarning ( "Error initializing secondary index: %s", g_sSecondaryError.cstr() );

	if ( !sError.IsEmpty() )
		sError = "";

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
	bool			bForcePseudoSharding = false;
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
		OPT ( "-?", "--?" )		{ ShowHelp(); return 0; }
		OPT ( "-v", "--version" )	{ return 0; }
		OPT1 ( "--console" )		{ g_bOptNoLock = true; g_bOptNoDetach = true; bTestMode = true; }
		OPT1 ( "--stop" )			bOptStop = true;
		OPT1 ( "--stopwait" )		{ bOptStop = true; bOptStopWait = true; }
		OPT1 ( "--status" )			bOptStatus = true;
		OPT1 ( "--pidfile" )		bOptPIDFile = true;
		OPT1 ( "--iostats" )		g_bIOStats = true;
		OPT1 ( "--cpustats" )		g_bCpuStats = true;
#if _WIN32
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
		OPT1 ( "--force-pseudo-sharding" ) { bForcePseudoSharding = true; } // internal option, do NOT document
		OPT1 ( "--strip-path" )		g_bStripPath = true;
		OPT1 ( "--vtune" )			g_bVtune = true;
		OPT1 ( "--noqlog" )			bOptDebugQlog = false;
		OPT1 ( "--force-preread" )	bForcedPreread = true;
		OPT1 ( "--coredump" )		g_bCoreDump = true;
		OPT1 ( "--new-cluster" )	bNewCluster = true;
		OPT1 ( "--new-cluster-force" )	bNewClusterForce = true;

		// FIXME! add opt=(csv)val handling here
		OPT1 ( "--replay-flags=accept-desc-timestamp" )		uReplayFlags |= Binlog::REPLAY_ACCEPT_DESC_TIMESTAMP;
		OPT1 ( "--replay-flags=ignore-open-errors" )		uReplayFlags |= Binlog::REPLAY_IGNORE_OPEN_ERROR;
		OPT1 ( "--replay-flags=ignore-trx-errors" )			uReplayFlags |= Binlog::REPLAY_IGNORE_TRX_ERROR;
		OPT1 ( "--replay-flags=ignore-all-errors" )			uReplayFlags |= Binlog::REPLAY_IGNORE_ALL_ERRORS;

		// handle 1-arg options
		else if ( (i+1)>=argc )		break;
		OPT ( "-c", "--config" )	szCmdConfigFile = argv[++i];
		OPT ( "-p", "--port" )		{ bOptPort = true; iOptPort = atoi ( argv[++i] ); }
		OPT ( "-l", "--listen" )	{ bOptListen = true; sOptListen = argv[++i]; }
		OPT ( "-i", "--index" )		dOptIndexes.Add ( argv[++i] );
#if _WIN32
		OPT1 ( "--servicename" )	++i; // it's valid but handled elsewhere
#endif

		// handle unknown options
		else
			break;
	}
	if ( i!=argc )
		sphFatal ( "malformed or unknown option near '%s'; use '-h' or '--help' to see available options.", argv[i] );

#if _WIN32
	CheckWinInstall();
#endif

	SetupLemmatizerBase();
	g_sConfigFile = sphGetConfigFile ( szCmdConfigFile );
#if _WIN32
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
	FakeScopedWLock_T<> wFakeLock { g_tRotateConfigMutex };
	if ( !ParseConfig ( &g_hCfg, g_sConfigFile.scstr(), g_dConfig.begin() ) )
		sphFatal ( "failed to parse config file '%s': %s", g_sConfigFile.cstr (), TlsMsg::szError() );
	CleanLoadedConfig();

	const CSphConfig& hConf = g_hCfg;

	if ( !hConf.Exists ( "searchd" ) || !hConf["searchd"].Exists ( "searchd" ) )
		sphFatal ( "'searchd' config section not found in '%s'", g_sConfigFile.cstr () );

	const CSphConfigSection & hSearchdpre = hConf["searchd"]["searchd"];

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
	if ( !LoadConfigInt ( hConf, g_sConfigFile, sError ) )
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
#if !_WIN32
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
		g_sPidFile = FixupFilename ( hSearchdpre["pid_file"].cstr() );

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

		// fake lock is acquired; no warnings will be fired
		if ( !ParseConfig ( &g_hCfg, g_sConfigFile.cstr (), g_dConfig.begin () ) )
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
	//if ( !g_bOptNoLock )
	{
		// create log
		OpenDaemonLog ( hSearchd, true );

		// create query log if required
		if ( hSearchd.Exists ( "query_log" ) )
		{
			CSphString sQueryLog = hSearchd["query_log"].cstr();
			if ( sQueryLog=="syslog" )
				g_bQuerySyslog = true;
			else
			{
#if _WIN32
				sQueryLog = AppendWinInstallDir(sQueryLog);
#endif
				g_iQueryLogFile = open ( sQueryLog.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
				if ( g_iQueryLogFile<0 )
					sphFatal ( "failed to open query log file '%s': %s", sQueryLog.cstr(), strerrorm(errno) );

				LogChangeMode ( g_iQueryLogFile, g_iLogFileMode );
			}
			g_sQueryLogFile = sQueryLog.cstr();
		}
	}

#if !_WIN32
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

	// init before workpool, as last checks binlog
	ModifyDaemonPaths ( hSearchd );
	sphRTInit ( hSearchd, bTestMode, hConf("common") ? hConf["common"]("common") : nullptr );

	// after next line executed we're in mt env, need to take rwlock accessing config.
	StartGlobalWorkPool ();

	// since that moment any 'fatal' will assume calling 'shutdown' function.
	sphSetDieCallback ( DieOrFatalWithShutdownCb );

	sphInfo( "starting daemon version '%s' ...", g_sStatusVersion.cstr() );

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
			AddGlobalListener ( MakeLocalhostListener ( SPHINXAPI_PORT, Proto_e::SPHINX ) );
			AddGlobalListener ( MakeLocalhostListener ( SPHINXQL_PORT, Proto_e::MYSQL41 ) );
		}
	}

	if ( !ValidateListenerRanges ( dListenerDescs, sError ) )
		sphFatal ( "%s", sError.cstr() );

	SetServerSSLKeys ( hSearchd ( "ssl_cert" ), hSearchd ( "ssl_key" ), hSearchd ( "ssl_ca" ) );
	CheckSSL();

	// set up ping service (if necessary) before loading indexes
	// (since loading ha-mirrors of distributed already assumes ping is usable).
	if ( g_iPingIntervalUs>0 )
		Ping::Start();

	ScheduleMallocTrim();

	// initialize timeouts since hook will use them
	auto iRtFlushPeriodUs = hSearchd.GetUsTime64S ( "rt_flush_period", 36000000000ll ); // 10h
	SetRtFlushPeriod ( Max ( iRtFlushPeriodUs, 3 * 1000000 ) ); // min 3S
	g_pLocalIndexes->SetAddOrReplaceHook ( HookSubscribeMutableFlush );

	//////////////////////
	// build indexes hash
	//////////////////////

	// configure and preload
	if ( bTestMode ) // pass this flag here prior to index config
		sphRTSetTestMode();

	if ( bForcePseudoSharding )
		SetPseudoShardingThresh(0);

	StrVec_t dExactIndexes;
	for ( const auto &dOptIndex : dOptIndexes )
		sphSplit ( dExactIndexes, dOptIndex.cstr (), "," );

	SetPercolateQueryParserFactory ( PercolateQueryParserFactory );
	Threads::CallCoroutine ( [&hConf, &dExactIndexes]() REQUIRES_SHARED ( g_tRotateConfigMutex )
	{
		ScopedRole_c thMain ( MainThread );
		ConfigureAndPreloadOnStartup ( hConf, dExactIndexes );
	} );

	///////////
	// startup
	///////////

	DetermineNodeItemStackSize();
	DetermineFilterItemStackSize();
//	ModifyDaemonPaths ( hSearchd );
//	sphRTInit ( hSearchd, bTestMode, hConf("common") ? hConf["common"]("common") : nullptr );

	if ( hSearchd.Exists ( "snippets_file_prefix" ) )
		g_sSnippetsFilePrefix = hSearchd["snippets_file_prefix"].cstr();
	else
		g_sSnippetsFilePrefix.SetSprintf("%s/", sphGetCwd().scstr());

	{
		auto sLogFormat = hSearchd.GetStr ( "query_log_format", "sphinxql" );
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
	}
	if ( g_bLogCompactIn && g_eLogFormat==LOG_FORMAT_PLAIN )
		sphWarning ( "compact_in option only supported with query_log_format=sphinxql" );

	// prepare to detach
	if ( !g_bOptNoDetach )
	{
		ReleaseTTYFlag();

#if !_WIN32
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

	Binlog::Configure ( hSearchd, bTestMode, uReplayFlags );
	SetUidShort ( bTestMode );
	InitDocstore ( g_iDocstoreCache );
	InitSkipCache ( g_iSkipCache );
	InitParserOption();

	if ( bOptPIDFile )
	{
#if !_WIN32
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

#if _WIN32
	SetConsoleCtrlHandler ( CtrlHandler, TRUE );
#endif

	Threads::CallCoroutine( [bWatched] {
	StrVec_t dFailed;
	if ( !g_bOptNoDetach && !bWatched && !g_bService )
	{
		// re-lock indexes
		ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
		for ( const auto& tIt : *hLocal )
		{
			sphLogDebug ( "Relocking %s", tIt.first.cstr () );
			auto pServed = tIt.second;
			// obtain exclusive lock
			if ( !pServed )
				dFailed.Add ( tIt.first );
			RWIdx_c pIdx { pServed };
			if ( !pIdx->Lock() )
			{
				sphWarning ( "index '%s': lock: %s; INDEX UNUSABLE", tIt.first.cstr(), pIdx->GetLastError().cstr() );
				dFailed.Add ( tIt.first );
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

#if _WIN32
	if ( g_bService )
		MySetServiceStatus ( SERVICE_RUNNING, NO_ERROR, 0 );
#endif

	// replay last binlog
	Threads::CallCoroutine ([]
	{
		auto _ = PublishSystemInfo ("replay binlog");
		SmallStringHash_T<CSphIndex*> hIndexes;
		ServedSnap_t hLocals = g_pLocalIndexes->GetHash();
		for ( auto& tIt : *hLocals )
		{
			if ( tIt.second )
				hIndexes.Add ( RWIdx_c ( tIt.second ), tIt.first );
		}

		Binlog::Replay ( hIndexes, DumpMemStat );
	} );

	// no need to create another cluster on restart by watchdog resurrection
	if ( bWatched && !bVisualLoad )
	{
		bNewCluster = false;
		bNewClusterForce = false;
	}
	
	StartRtBinlogFlushing();

	ScheduleFlushAttrs();

	gStats().m_uStarted = (DWORD)time(NULL);

	CSphString sSQLStateDefault;
	if ( IsConfigless() )
		sSQLStateDefault.SetSprintf ( "%s/state.sql", GetDataDirInt().cstr() );
	if ( !InitSphinxqlState ( hSearchd.GetStr ( "sphinxql_state", sSQLStateDefault.scstr() ), sError ))
		sphWarning ( "sphinxql_state flush disabled: %s", sError.cstr ());

	ServeUserVars ();

	ServeAutoOptimize();

	PrereadIndexes ( bForcedPreread );

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

		if ( ( g_iTFO!=TFO_ABSENT ) && ( g_iTFO & TFO_LISTEN ) )
			sphSetSockTFO ( dListener.m_iSock );
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
	Detached::MakeAloneIteratorAvailable ();

	// time for replication to sync with cluster
	searchd::AddShutdownCb ( ReplicateClustersDelete );
	ReplicationStart ( std::move ( dListenerDescs ), bNewCluster, bNewClusterForce );

	g_bJsonConfigLoadedOk = true;

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
	PrepareMainThread ( &cTopOfMainStack );
	sphSetDieCallback ( DieOrFatalCb );
	g_pLogger() = sphLog;
	sphBacktraceSetBinaryName ( argv[0] );
	GeodistInit();

#if _WIN32
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

volatile bool& sphGetGotSighup() noexcept
{
	static bool bGotSighup = false;
	return bGotSighup;
}

volatile bool& sphGetGotSigusr1() noexcept
{
	static bool bGotSigusr1 = false;
	return bGotSigusr1;
}

volatile bool & sphGetGotSigusr2 () noexcept
{
	static bool bGotSigusr2 = false;
	return bGotSigusr2;
}
