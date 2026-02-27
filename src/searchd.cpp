//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxplugin.h"
#include "sphinxqcache.h"
#include "docstore.h"
#include "searchdha.h"
#include "searchdreplication.h"
#include "replication/api_command_cluster.h"
#include "global_idf.h"
#include "searchdssl.h"
#include "searchdddl.h"
#include "query_status.h"
#include "debug_cmds.h"
#include "stackmock.h"
#include "binlog.h"
#include "indexfiles.h"
#include "tokenizer/charset_definition_parser.h"
#include "client_session.h"
#include "sphinx_alter.h"
#include "docs_collector.h"
#include "index_rotator.h"
#include "config_reloader.h"
#include "secondarylib.h"
#include "knnlib.h"
#include "knnmisc.h"
#include "tracer.h"
#include "netfetch.h"
#include "queryfilter.h"
#include "exprdatetime.h"
#include "geodist.h"
#include "joinsorter.h"
#include "schematransform.h"
#include "skip_cache.h"
#include "jieba.h"
#include "sphinxexcerpt.h"
#include "sphinxquery/xqparser.h"
#include "daemon/winservice.h"
#include "daemon/crash_logger.h"
#include "daemon/logger.h"
#include "daemon/search_handler.h"
#include "daemon/api_commands.h"
#include "dict/stem/sphinxstem.h"

// services
#include "taskping.h"
#include "taskmalloctrim.h"
#include "taskglobalidf.h"
#include "tasksavestate.h"
#include "taskflushbinlog.h"
#include "taskflushattrs.h"
#include "taskflushmutable.h"
#include "taskpreread.h"
#include "searchdbuddy.h"
#include "detail/indexlink.h"
#include "detail/expmeter.h"

#include <csignal>
#include <clocale>

#include <sys/stat.h>
#if !_WIN32
	#include <sys/wait.h>
	#include <netinet/in.h>
#endif

// for HAVE_GETRLIMIT, HAVE_SETRLIMIT
#include "config.h"
#if HAVE_GETRLIMIT & HAVE_SETRLIMIT
	#include <sys/resource.h>
#endif

/////////////////////////////////////////////////////////////////////////////

#define SEARCHD_BACKLOG			5

// don't shutdown on SIGKILL (debug purposes)
// 1 - SIGKILL will shut down the whole daemon; 0 - watchdog will reincarnate the daemon
#define WATCHDOG_SIGKILL		1

/////////////////////////////////////////////////////////////////////////////

using namespace Threads;

static auto& 			g_iParentPID		= getParentPID ();  // set by watchdog
int						g_iQueryLogMinMs	= 0;				// log 'slow' threshold for query

int						g_iReadTimeoutS		= 5;	// sec
int						g_iWriteTimeoutS	= 5;	// sec
bool					g_bTimeoutEachPacket = true;
int						g_iClientTimeoutS	= 300;
int						g_iClientQlTimeoutS	= 900;	// sec
static int				g_iMaxConnection	= 0; // unlimited
static bool				g_bWatchdog			= true;
static int				g_iExpansionLimit	= 0;
static int				g_iShutdownTimeoutUs	= 3000000; // default timeout on daemon shutdown and stopwait is 3 seconds
static int				g_iBacklog			= SEARCHD_BACKLOG;
static int				g_iThdQueueMax		= 0;
static auto&			g_iTFO = sphGetTFO ();
static bool				g_bJsonConfigLoadedOk = false;
static auto&			g_iAutoOptimizeCutoffMultiplier = AutoOptimizeCutoffMultiplier();
static constexpr bool	AUTOOPTIMIZE_NEEDS_VIP = false; // whether non-VIP can issue 'SET GLOBAL auto_optimize = X'
static constexpr bool	THREAD_EX_NEEDS_VIP = false; // whether non-VIP can issue 'SET GLOBAL auto_optimize = X'

static CSphVector<Listener_t>	g_dListeners;

CSphString				g_sPidFile;
static bool				g_bPidIsMine = false;		// if PID is not mine, don't unlink it on fail
static int				g_iPidFD		= -1;

static int				g_iMaxCachedDocs	= 0;	// in bytes
static int				g_iMaxCachedHits	= 0;	// in bytes

int						g_iMaxPacketSize	= 128*1024*1024;	// in bytes; for both query packets from clients and response packets from agents
int						g_iMaxFilters		= 256;
int						g_iMaxFilterValues	= 4096;
int						g_iMaxBatchQueries	= 32;

static int64_t			g_iDocstoreCache = 0;
static int64_t			g_iSkipCache = 0;

static auto &	g_iDistThreads		= getDistThreads();

const int DAEMON_DEFAULT_CONNECT_TIMEOUT	= 1000;
const int DAEMON_DEFAULT_QUERY_TIMEOUT		= 3000;

int g_iAgentConnectTimeoutMs		= DAEMON_DEFAULT_CONNECT_TIMEOUT;
int g_iAgentQueryTimeoutMs			= DAEMON_DEFAULT_QUERY_TIMEOUT;	// global (default). May be override by index-scope values, if one specified
int g_iAgentRetryCount				= 0;
int g_iAgentRetryDelayMs			= DAEMON_MAX_RETRY_DELAY/2;	// global (default) values. May be override by the query options 'retry_count' and 'retry_timeout'

static int g_iReplConnectTimeoutMs	= DAEMON_DEFAULT_CONNECT_TIMEOUT;
static int g_iReplQueryTimeoutMs	= DAEMON_DEFAULT_QUERY_TIMEOUT;
static int g_iReplRetryCount		= 3;
static int g_iReplRetryDelayMs		= DAEMON_MAX_RETRY_DELAY/2;

bool					g_bHostnameLookup = false;
CSphString				g_sMySQLVersion { szMANTICORE_VERSION };
CSphString				g_sBannerVersion { szMANTICORE_NAME };
CSphString				g_sBanner;
CSphString				g_sStatusVersion = szMANTICORE_VERSION;
CSphString				g_sSecondaryError;
static CSphString		g_sBuddyPath;
static bool				g_bTelemetry = val_from_env ( "MANTICORE_TELEMETRY", true );
static bool				g_bHasBuddyPath = false;
static bool				g_bAutoSchema = true;
static bool				g_bNoChangeCwd = val_from_env ( "MANTICORE_NO_CHANGE_CWD", false );
static bool				g_bCwdChanged = false;
static CSphString		g_sKbnVersion;
static bool				g_bAttrAutoconvStrict = false;

// for CLang thread-safety analysis
ThreadRole MainThread; // functions which called only from main thread
ThreadRole HandlerThread; // thread which serves clients

//////////////////////////////////////////////////////////////////////////

static CSphString		g_sConfigFile;
static bool				LOG_LEVEL_SHUTDOWN = val_from_env("MANTICORE_TRACK_DAEMON_SHUTDOWN",false); // verbose logging when daemon shutdown, ruled by this env variable
static CSphString		g_sConfigPath; // for resolve paths to absolute
static CSphString		g_sExePath;

static auto&			g_bSeamlessRotate	= sphGetSeamlessRotate ();

bool					g_bIOStats		= false;
static auto&			g_bCpuStats 	= sphGetbCpuStat ();
static bool				g_bOptNoDetach	= false; // whether to detach from console, or work in front
static bool				g_bOptNoLock	= false; // whether to lock indexes (with .spl) or not
static bool				g_bOptQuiet		= false; // suppress startup output except errors
static bool				g_bStripPath	= false;

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

constexpr int 								g_iExpMeterPeriod = 5000000; // once per 5s
static ExpMeter_c							g_tStat1m { 12 }; // once a minute (12 * 5s)
static ExpMeter_c							g_tStat5m { 12*5 }; // once a 5 minutes
static ExpMeter_c							g_tStat15m { 12*15 }; // once a 15 minutes
static ExpMeter_c							g_tPriStat1m { 12 }; // once a minute (12 * 5s)
static ExpMeter_c							g_tPriStat5m { 12*5 }; // once a 5 minutes
static ExpMeter_c							g_tPriStat15m { 12*15 }; // once a 15 minutes
static ExpMeter_c							g_tSecStat1m { 12 }; // once a minute (12 * 5s)
static ExpMeter_c							g_tSecStat5m { 12*5 }; // once a 5 minutes
static ExpMeter_c							g_tSecStat15m { 12*15 }; // once a 15 minutes
int64_t g_iNextExpMeterTimestamp = sphMicroTimer() + g_iExpMeterPeriod;

static CSphString							g_sClusterUser { "cluster" }; // user with this name will see cluster:table in show tables

/// command names
static const char * g_dApiCommands[SEARCHD_COMMAND_TOTAL] =
{
	"search", "excerpt", "update", "keywords", "persist", "status", "query", "flushattrs", "query", "ping", "delete", "set",  "insert", "replace", "commit", "suggest", "json",
	"callpq", "clusterpq", "getfield"
};

//////////////////////////////////////////////////////////////////////////

const char * sAgentStatsNames[eMaxAgentStat+ehMaxStat]=
	{ "query_timeouts", "connect_timeouts", "connect_failures",
		"network_errors", "wrong_replies", "unexpected_closings",
		"warnings", "succeeded_queries", "total_query_time",
		"connect_count", "connect_avg", "connect_max" };


// fixme! g_tLastMeta updated *ONLY* from HandleCommandSearch, that is binary API, i.e. no sphinxql, no http
// m.b. we need to expand it to other protos, or in opposite, deprecate it. 'Just API' looks obsolete.
static RwLock_t g_tLastMetaLock;
static CSphQueryResultMeta		g_tLastMeta GUARDED_BY ( g_tLastMetaLock );

/////////////////////////////////////////////////////////////////////////////
// MISC
/////////////////////////////////////////////////////////////////////////////

static void ReleaseTTYFlag()
{
	if ( g_pShared )
		g_pShared->m_bHaveTTY = true;
}

const char * szStatusVersion () noexcept
{
	return g_sStatusVersion.cstr();
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

static CSphString GetNamedPipeName ( int iPid )
{
	CSphString sRes;
	sRes.SetSprintf ( "/tmp/searchd_%d", iPid );
	return sRes;
}

/////////////////////////////////////////////////////////////////////////////


#define LOG_COMPONENT_SEARCHD __LINE__ << " "

#define SHUTINFO LOGINFO (SHUTDOWN,SEARCHD)

/////////////////////////////////////////////////////////////////////////////
// SIGNAL HANDLERS
/////////////////////////////////////////////////////////////////////////////
void Shutdown () REQUIRES ( MainThread ) NO_THREAD_SAFETY_ANALYSIS
{
	// force even long time searches to shut
	SHUTINFO << "Trigger g_bInterruptNow ...";
	sphInterruptNow ();

	SHUTINFO << "Shutdown curl query subsystem ...";
	ShutdownCurl();

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
	SHUTINFO << "Finally save tables ...";
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
	SHUTINFO << "Unlock tables ...";
	{
		ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
		for ( const auto& tIt : *hLocal )
			RWIdx_c ( tIt.second )->Unlock();
	}

	CallCoroutine ( [] {
		SHUTINFO << "Abandon local tables list ...";
		g_pLocalIndexes->ReleaseAndClear();

		// unlock Distr indexes automatically done by d-tr
		SHUTINFO << "Abandon distr tables list ...";
		g_pDistIndexes->ReleaseAndClear();
	} );

	SHUTINFO << "Shutdown alone threads (if any) ...";
	Detached::ShutdownAllAlones();

	SHUTINFO << "Shutdown main work pool ...";
	StopGlobalWorkPool();

	SHUTINFO << "Remove local tables list ...";
	g_pLocalIndexes.reset();

	SHUTINFO << "Remove distr tables list ...";
	g_pDistIndexes.reset();

	// clear shut down of rt indexes + binlog
	SHUTINFO << "Finish IO stats collecting ...";
	sphDoneIOStats();

	SHUTINFO << "Finish binlog serving ...";
	Binlog::Deinit();
	ReplicationBinlogStop();

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

	SHUTINFO << "Shutdown secondary ...";
	ShutdownSecondary();

	SHUTINFO << "Shutdown knn ...";
	ShutdownKNN();

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

	Threads::Done ( GetDaemonLogFD() );

#if _WIN32
	CloseWinPipe();
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


#if SPH_ALLOCS_PROFILER
const char		g_sMemoryStatBanner[] = "\n--- memory statistics ---\n";
#endif


#if _WIN32
void SetSignalHandlers ( bool )
{
	sphBacktraceInit ();
	CrashLogger::SetCrashHandler ();
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
	bSignalsSet = CrashLogger::SetCrashHandler ();
}
#endif

#if !_WIN32
static int sphCreateUnixSocket ( const char * sPath ) REQUIRES ( MainThread )
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


static int sphCreateInetSocket ( const ListenerDesc_t & tDesc ) REQUIRES ( MainThread )
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

static ListenerDesc_t MakeAnyListener ( int iPort, Proto_e eProto=Proto_e::SPHINX )
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

static ListenerDesc_t MakeLocalhostListener ( int iPort, Proto_e eProto )
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

void LogToConsole(const char* szKind, const char* szMsg) noexcept
{
	if ( g_bOptNoDetach && LogFormat()!=LOG_FORMAT::SPHINXQL )
		sphInfo ( "query %s: %s", szKind, szMsg );
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
	LogToConsole ( "error", sError.cstr() );
}

void DistributedIndex_t::GetAllHosts ( VectorAgentConn_t &dTarget ) const
{
	for ( const auto& pMultiAgent : m_dAgents )
		for ( const auto & dHost : *pMultiAgent )
		{
			auto * pAgent = new AgentConn_t;
			pAgent->m_tDesc.CloneFrom ( dHost );
			pAgent->m_iMyQueryTimeoutMs = GetAgentConnectTimeoutMs();
			pAgent->m_iMyConnectTimeoutMs = GetAgentQueryTimeoutMs();
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

int DistributedIndex_t::GetAgentConnectTimeoutMs ( bool bRaw ) const
{
	return ( ( m_iAgentConnectTimeoutMs || bRaw ) ? m_iAgentConnectTimeoutMs : g_iAgentConnectTimeoutMs );
}

int DistributedIndex_t::GetAgentQueryTimeoutMs ( bool bRaw ) const
{
	return ( ( m_iAgentQueryTimeoutMs || bRaw ) ? m_iAgentQueryTimeoutMs : g_iAgentQueryTimeoutMs );
}

void DistributedIndex_t::SetAgentConnectTimeoutMs ( int iAgentConnectTimeoutMs )
{
	m_iAgentConnectTimeoutMs = iAgentConnectTimeoutMs;
}

void DistributedIndex_t::SetAgentQueryTimeoutMs ( int iAgentQueryTimeoutMs )
{
	m_iAgentQueryTimeoutMs = iAgentQueryTimeoutMs;
}

DistributedIndex_t * DistributedIndex_t::Clone() const
{

	DistributedIndex_t * pDist ( new DistributedIndex_t );
	pDist->m_dAgents = m_dAgents;
	pDist->m_dLocal = m_dLocal;
	pDist->m_iAgentRetryCount = m_iAgentRetryCount;
	pDist->m_bDivideRemoteRanges = m_bDivideRemoteRanges;
	pDist->m_eHaStrategy = m_eHaStrategy;
	pDist->m_sCluster = m_sCluster;
	pDist->m_iAgentConnectTimeoutMs = m_iAgentConnectTimeoutMs;
	pDist->m_iAgentQueryTimeoutMs = m_iAgentQueryTimeoutMs;

	return pDist;
}

void UpdateLastMeta ( VecTraits_T<AggrResult_t> tResults )
{
	int64_t iTotalPredictedTime = 0;
	int64_t iTotalAgentPredictedTime = 0;
	for ( const auto & dResult: tResults )
	{
		iTotalPredictedTime += dResult.m_iPredictedTime;
		iTotalAgentPredictedTime += dResult.m_iAgentPredictedTime;
	}

	auto & g_tStats = gStats();
	g_tStats.m_iPredictedTime.fetch_add ( iTotalPredictedTime, std::memory_order_relaxed );
	g_tStats.m_iAgentPredictedTime.fetch_add ( iTotalAgentPredictedTime, std::memory_order_relaxed );

	ScWL_t dLastMetaLock ( g_tLastMetaLock );
	g_tLastMeta = tResults.Last();
}


//////////////////////////////////////////////////////////////////////////

using QuotationEscapedBuilder = EscapedStringBuilder_T<BaseQuotation_T<EscapeQuotator_t>>;

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
	tOneRes.m_tSchema = m_tSchema;
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

inline static bool ClusterFlavour () noexcept
{
	return !g_sClusterUser.IsEmpty () && session::GetClientSession ()->m_sUser==g_sClusterUser;
}


static CSphString ApplyClusterName ( const CSphString& sIndex ) noexcept
{
	if ( !ClusterFlavour () )
		return sIndex;

	auto dParts = sphSplit ( sIndex.cstr (), ":" );
	if ( dParts.GetLength ()>1 )
		return dParts[1];
	return sIndex;
}

// process system.table and remove 1-st subkey
inline static void FixupSystemTableName ( SqlStmt_t * pStmt ) noexcept
{
	auto sName = ApplyClusterName ( pStmt->m_sIndex );
	bool bNameFromQuery = false;
	if ( sName.EqN ( "system" ) )
	{
		if ( !pStmt->m_dStringSubkeys.IsEmpty () )
		{
			sName = SphSprintf ( "system%s", pStmt->m_dStringSubkeys[0].cstr () );
			pStmt->m_dStringSubkeys.Remove ( 0 );
			pStmt->m_sIndex = sName;
		}
		else if ( !pStmt->m_tQuery.m_dStringSubkeys.IsEmpty () )
		{
			sName = SphSprintf ( "system%s", pStmt->m_tQuery.m_dStringSubkeys[0].cstr () );
			pStmt->m_tQuery.m_dStringSubkeys.Remove ( 0 );
			pStmt->m_sIndex = sName;
			bNameFromQuery = true;
		}
	}

	if ( ApplyClusterName ( pStmt->m_tQuery.m_sIndexes ).EqN ( "system" ) && bNameFromQuery )
		pStmt->m_tQuery.m_sIndexes = sName;
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

	if ( session::GetBuddy() )
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

bool sphCheckWeCanModify ( StmtErrorReporter_i & tOut )
{
	if ( sphCheckWeCanModify() )
		return true;

	tOut.Error ( "connection is read-only");
	return false;
}

bool sphCheckWeCanModify ( CSphString & sError )
{
	if ( sphCheckWeCanModify() )
		return true;

	sError = "connection is read-only";
	return false;
}

bool sphCheckWeCanModify ( RowBuffer_i & tOut )
{
	if ( sphCheckWeCanModify() )
		return true;

	tOut.Error ( "connection is read-only" );
	return false;
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


std::unique_ptr<ISphTableFunc> CreateRemoveRepeats()
{
	return std::make_unique<CSphTableFuncRemoveRepeats>();
}

#undef LOC_ERROR1
#undef LOC_ERROR

//////////////////////////////////////////////////////////////////////////

class CSphMatchVariant 
{
public:
	inline static SphAttr_t ToInt ( const SqlInsert_t & tVal, CSphString & sError )
	{
		switch ( tVal.m_iType )
		{
			case SqlInsert_t::QUOTED_STRING:
			{
				const char * sStart = tVal.m_sVal.cstr();
				char * sEnd = nullptr;
				errno = 0;
				SphAttr_t uVal = strtoul ( sStart, &sEnd, 10 );
					
				if ( g_bAttrAutoconvStrict && !ValidateConv ( sStart, sEnd, tVal.m_sVal.Length(), errno, "integer", sError ) )
					return 0;
					
				return uVal;
			}
			case SqlInsert_t::CONST_INT:		return int(tVal.GetValueInt());
			case SqlInsert_t::CONST_FLOAT:		return int(tVal.m_fVal); // FIXME? report conversion error
		}
		return 0;
	}

	inline static SphAttr_t ToBigInt ( const SqlInsert_t & tVal, CSphString & sError )
	{
		switch ( tVal.m_iType )
		{
			case SqlInsert_t::QUOTED_STRING:
			{
				const char * sStart = tVal.m_sVal.cstr();
				char * sEnd = nullptr;
				errno = 0;
				SphAttr_t iVal = strtoll ( sStart, &sEnd, 10 );
					
				if ( g_bAttrAutoconvStrict && !ValidateConv ( sStart, sEnd, tVal.m_sVal.Length(), errno, "bigint", sError ) )
					return 0;
					
				return iVal;
			}
			case SqlInsert_t::CONST_INT:		return tVal.GetValueInt();
			case SqlInsert_t::CONST_FLOAT:		return int64_t(tVal.m_fVal); // FIXME? report conversion error?
		}
		return 0;
	}

	inline static SphAttr_t ToBigUint ( const SqlInsert_t & tVal, CSphString & sError )
	{
		switch ( tVal.m_iType )
		{
		case SqlInsert_t::QUOTED_STRING:
		{
			const char * sStart = tVal.m_sVal.cstr();
			char * sEnd = nullptr;
			errno = 0;
			SphAttr_t uVal = strtoull ( sStart, &sEnd, 10 );
				
			if ( g_bAttrAutoconvStrict && !ValidateConv ( sStart, sEnd, errno, tVal.m_sVal.Length(), "bigint", sError ) )
				return 0;
				
			return uVal;
		}
		case SqlInsert_t::CONST_INT:		return tVal.GetValueUint();
		case SqlInsert_t::CONST_FLOAT:		return uint64_t(int64_t(tVal.m_fVal)); // FIXME? report conversion error?
		}
		return 0;
	}

	static bool ConvertBool ( const SqlInsert_t & tVal, SphAttr_t & tAttr )
	{
		if ( tVal.m_iType!=SqlInsert_t::QUOTED_STRING )
			return false;

		if ( tVal.m_sVal.EqN ( "true" ) )
		{
			tAttr = 1;
			return true;
		}
		if ( tVal.m_sVal.EqN ( "false" ) )
		{
			tAttr = 0;
			return true;
		}

		return false;
	}

	static bool ConvertPlainAttr ( const SqlInsert_t & tVal, ESphAttr eTargetType, const CSphString * pName, SphAttr_t & tAttr, bool bDocID, CSphString & sError )
	{
		tAttr = 0;

		switch ( eTargetType )
		{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TOKENCOUNT:
			tAttr = ToInt ( tVal, sError );
			if ( !sError.IsEmpty() )
				return false;
			break;

		case SPH_ATTR_BOOL:
			if ( !ConvertBool ( tVal, tAttr ) ) // try to convert true \ false string then number
			{
				tAttr = ToInt ( tVal, sError );
				if ( !sError.IsEmpty() )
					return false;
			}
			break;

		case SPH_ATTR_BIGINT:
			if ( bDocID )
			{
				if ( tVal.IsNegativeInt() )
				{
					sError = "Negative document ids are not allowed";
					return false;
				}

				tAttr = ToBigUint ( tVal, sError );
				if ( !sError.IsEmpty() )
					return false;
			}
			else
			{
				tAttr = ToBigInt ( tVal, sError );
				if ( !sError.IsEmpty() )
					return false;
			}
			break;

		case SPH_ATTR_FLOAT:
			if ( tVal.m_iType==SqlInsert_t::QUOTED_STRING )
				tAttr = sphF2DW ( (float)strtod ( tVal.m_sVal.cstr(), NULL ) ); // FIXME? report conversion error?
			else if ( tVal.m_iType==SqlInsert_t::CONST_INT )
				tAttr = sphF2DW ( float(tVal.GetValueInt()) ); // FIXME? report conversion error?
			else if ( tVal.m_iType==SqlInsert_t::CONST_FLOAT )
				tAttr = sphF2DW ( tVal.m_fVal );
			break;

		case SPH_ATTR_STRINGPTR:
			break;

		case SPH_ATTR_TIMESTAMP:
		{
			if ( pName && tVal.m_iType==SqlInsert_t::QUOTED_STRING )
				tAttr = GetUTC ( tVal.m_sVal );
			else
			{
				tAttr = ToInt ( tVal, sError );
				if ( !sError.IsEmpty() )
					return false;
			}
		}
		break;

		default:
			return false;
		};

		return true;
	}

	inline static bool SetAttr ( CSphMatch & tMatch, const CSphAttrLocator & tLoc, const CSphString * pName, const SqlInsert_t & tVal, ESphAttr eTargetType, bool bDocID, CSphString & sError )
	{
		SphAttr_t tAttr;
		if ( ConvertPlainAttr ( tVal, eTargetType, pName, tAttr, bDocID, sError ) )
		{
			tMatch.SetAttr ( tLoc, tAttr );
			return true;
		}

		return false;
	}

	inline static void SetDefaultAttr ( CSphMatch & tMatch, const CSphAttrLocator & tLoc, ESphAttr eTargetType )
	{
		SqlInsert_t tVal;
		tVal.m_iType = SqlInsert_t::CONST_INT;
		tVal.SetValueInt(0);
		CSphString sError;
		SetAttr ( tMatch, tLoc, nullptr, tVal, eTargetType, false, sError );
	}

	static bool IsOnlySpace ( const char * sStart, char * sEnd, int iLen )
	{
		if ( *sEnd=='\0' )
			return true;


		while ( sEnd<sStart+iLen && *sEnd!='\0' )
		{
			if ( !sphIsSpace ( *sEnd ) )
				return false;

			sEnd++;
		}

		return true;
	}

	static bool ValidateConv ( const char * sStart, char * sEnd, int iLen, int iErrno, const char * sAttrType, CSphString & sError )
	{
		if ( sStart==sEnd )
		{
			sError.SetSprintf ( "invalid value '%s' for %s attribute", sStart, sAttrType );
			return false;
		}
		
		if ( !IsOnlySpace ( sStart, sEnd, iLen ) )
		{
			sError.SetSprintf ( "invalid value '%s' for %s attribute (trailing characters)", sStart, sAttrType );
			return false;
		}
		
		if ( iErrno==ERANGE )
		{
			sError.SetSprintf ( "value '%s' overflow for %s attribute", sStart, sAttrType );
			return false;
		}
		
		return true;
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
		pConn->m_iMyConnectTimeoutMs = pDist->GetAgentConnectTimeoutMs();
		pConn->m_iMyQueryTimeoutMs = pDist->GetAgentQueryTimeoutMs();
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
	ClonableCtx_T<SnippedBuilderCtxRef_t, SnippedBuilderCtxClone_t, Threads::ECONTEXT::UNORDERED> dCtx { pBuilder };
	auto pDispatcher = Dispatcher::Make ( iJobs, 0, GetEffectiveBaseDispatcherTemplate(), dCtx.IsSingle() );
	dCtx.LimitConcurrency ( pDispatcher->GetConcurrency() );

	Coro::ExecuteN ( dCtx.Concurrency ( iJobs ), [&]
	{
		sphLogDebug ( "MakeSnippetsCoro Coro started" );
		auto pSource = pDispatcher->MakeSource();
		int iJob = -1; // make it consumed

		if ( !pSource->FetchTask ( iJob ) )
		{
			sphLogDebug ( "Early finish parallel MakeSnippetsCoro because of empty queue" );
			return; // already nothing to do, early finish.
		}

		auto tJobContext = dCtx.CloneNewContext();
		auto& tCtx = tJobContext.first;
		sphLogDebug ( "MakeSnippetsCoro cloned context %d", tJobContext.second );
		Threads::Coro::SetThrottlingPeriodMS ( session::GetThrottlingPeriodMS() );
		while (true)
		{
			myinfo::SetTaskInfo ( "s %d:", iJob );
			sphLogDebugv ( "MakeSnippetsCoro %d %d[%d]", tJobContext.second, iJob, dTasks[iJob] );
			MakeSingleLocalSnippetWithFields ( dQueries[dTasks[iJob]], q, tCtx.m_pBuilder, dStubFields );
			sphLogDebug ( "MakeSnippetsCoro Coro loop tick %d finished", iJob );
			iJob = -1; // mark it consumed

			if ( !pSource->FetchTask ( iJob ) )
				return; // already nothing to do, early finish.

			// yield and reschedule every quant of time. It gives work to other tasks
			Threads::Coro::ThrottleAndKeepCrashQuery ();
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
			sError.SetSprintf ( "%s", "distributed table for snippets must have exactly one local agent" );
			return false;
		}

		if ( !q.m_uFilesMode )
		{
			sError.SetSprintf ( "%s", "distributed table for snippets available only when using external files" );
			return false;
		}

		// for remotes index is 1-st local agent of the distr, so move on!
		sIndex = pDist->m_dLocal[0];
	}

	auto pServed = GetServed ( sIndex );
	if ( !pServed )
	{
		sError.SetSprintf ( "unknown local table '%s' in search request", sIndex.cstr() );
		return false;
	}

	pServed->m_pStats->IncCmd ( SEARCHD_COMMAND_EXCERPT );

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

	if ( uVer>=0x102 )
		tSettings.m_eJiebaMode = (JiebaMode_e)tReq.GetInt();

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
	TRACE_CORO ( "rt", "DoCommandUpdate" );

	int iUpd = 0;
	CSphString sWarning;
	RtAccum_t tAcc;
	ReplicationCommand_t* pCmd = tAcc.AddCommand ( ReplCmd_e::UPDATE_API, sIndex, sCluster );
	assert ( pCmd );
	pCmd->m_pUpdateAPI = std::move(pUpd);
	pCmd->m_bBlobUpdate = bBlobUpdate;

	HandleCmdReplicateUpdate ( tAcc, sWarning, iUpd );

	if ( iUpd<0 )
	{
		dFails.Submit ( sIndex, sDistributed, TlsMsg::szError() );
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
			return SendErrorReply ( tOut, "'id' attribute cannot be updated" );

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

	for ( int i=0; i<iNumUpdates; ++i )
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
							return SendErrorReply ( tOut, "error reading string" );

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
		return SendErrorReply ( tOut, "invalid or truncated request" );

	// check index names
	StrVec_t dIndexNames;
	ParseIndexList ( sIndexes, dIndexNames );

	if ( dIndexNames.IsEmpty() )
		return SendErrorReply ( tOut, "no valid tables in update request" );

	DistrPtrs_t dDistributed;
	// copy distributed indexes description
	CSphString sMissed;
	if ( !ExtractDistributedIndexes ( dIndexNames, dDistributed, sMissed ) )
		return SendErrorReply ( tOut, "unknown table '%s' in update request", sMissed.cstr() );

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
		return SendErrorReply ( tOut, "%s", sReport.cstr() );

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
	const auto & tStats = gStats();
	const char * OFF = "OFF";

	const int64_t iQueriesDiv = Max ( tStats.m_iQueries.load ( std::memory_order_relaxed ), 1 );
	const int64_t iDistQueriesDiv = Max ( tStats.m_iDistQueries.load ( std::memory_order_relaxed ), 1 );
	const int64_t iDiv1000 = iQueriesDiv * 1000;
	const int64_t iDDiv1000 = iDistQueriesDiv * 1000;

	dStatus.SetColName ( "Counter" );

	// FIXME? non-transactional!!!
	dStatus.MatchTupletf ( "uptime", "%u", (DWORD) time ( nullptr )-tStats.m_uStarted );
	dStatus.MatchTupletf ( "connections", "%l", tStats.m_iConnections.load ( std::memory_order_relaxed ) );
	dStatus.MatchTupletf ( "maxed_out", "%l", tStats.m_iMaxedOut.load ( std::memory_order_relaxed ) );
	dStatus.MatchTuplet ( "version" , g_sStatusVersion.cstr() );
	dStatus.MatchTuplet ( "mysql_version", g_sMySQLVersion.cstr() );

	for ( auto i=0; i<SEARCHD_COMMAND_TOTAL; ++i)
	{
		if ( i==SEARCHD_COMMAND_UNUSED_6 )
			continue;
		dStatus.MatchTupletf ( szCommand ( i ), "%l", tStats.Get ( (SearchdCommand_e)i ) );
	}

	const SearchdStats_t & tGlobalStats = gStats();
	FormatCmdStats ( tGlobalStats, "insert_replace", SearchdStats_t::eReplace, dStatus );
	FormatCmdStats ( tGlobalStats, "search", SearchdStats_t::eSearch, dStatus );
	FormatCmdStats ( tGlobalStats, "update", SearchdStats_t::eUpdate, dStatus);

	auto iConnects = tStats.m_iAgentConnectTFO.load ( std::memory_order_relaxed ) + tStats.m_iAgentConnect.load ( std::memory_order_relaxed );
	dStatus.MatchTupletf ( "agent_connect", "%l", iConnects );
	dStatus.MatchTupletf ( "agent_tfo", "%l", tStats.m_iAgentConnectTFO.load ( std::memory_order_relaxed ) );
	dStatus.MatchTupletf ( "agent_retry", "%l", tStats.m_iAgentRetry.load ( std::memory_order_relaxed ) );
	dStatus.MatchTupletf ( "queries", "%l", tStats.m_iQueries.load ( std::memory_order_relaxed ) );
	dStatus.MatchTupletf ( "dist_queries", "%l", tStats.m_iDistQueries.load ( std::memory_order_relaxed ) );

	// status of thread pool
	dStatus.MatchTupletf ( "workers_total", "%d", GlobalWorkPool ()->WorkingThreads () );
	dStatus.MatchTupletf ( "workers_active", "%d", myinfo::CountTasks () );
	dStatus.MatchTupletf ( "workers_clients", "%d", myinfo::CountClients () );
	dStatus.MatchTupletf ( "workers_clients_vip", "%u", session::GetVips() );
	dStatus.MatchTupletf ( "workers_clients_buddy", "%u", session::GetBuddyCount() );
	dStatus.MatchTupletf ( "work_queue_length", "%d", GlobalWorkPool ()->Works () );
	dStatus.MatchTupletf ( "load", "%0.2f %0.2f %0.2f", g_tStat1m.Value(), g_tStat5m.Value(), g_tStat15m.Value() );
	dStatus.MatchTupletf ( "load_primary", "%0.2f %0.2f %0.2f", g_tPriStat1m.Value(), g_tPriStat5m.Value(), g_tPriStat15m.Value() );
	dStatus.MatchTupletf ( "load_secondary", "%0.2f %0.2f %0.2f", g_tSecStat1m.Value(), g_tSecStat5m.Value(), g_tSecStat15m.Value() );

// macro defined in fileio.h
#if TRACE_UNZIP
	{
		StringBuilder_c sstat {", "};
		auto& stats = CSphReader::GetStat32();
		for ( const auto& stat : stats )
			sstat << stat.load(std::memory_order_relaxed);
		dStatus.MatchTupletf ( "unzip32_hist", "%s", sstat.cstr() );
	}

	{
		StringBuilder_c sstat { ", " };
		auto& stats = CSphReader::GetStat64();
		for ( const auto& stat : stats )
			sstat << stat.load ( std::memory_order_relaxed );
		dStatus.MatchTupletf ( "unzip64_hist", "%s", sstat.cstr() );
	}
#endif

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

	dStatus.MatchTupletf ( "query_wall", "%0.3F", tStats.m_iQueryTime.load ( std::memory_order_relaxed ) / 1000 );

	if ( g_bCpuStats )
		dStatus.MatchTupletf ( "query_cpu", "%0.3F", tStats.m_iQueryCpuTime.load ( std::memory_order_relaxed ) / 1000 );
	else
		dStatus.MatchTuplet ( "query_cpu", OFF);

	dStatus.MatchTupletf ( "dist_wall", "%0.3F", tStats.m_iDistWallTime.load ( std::memory_order_relaxed ) / 1000 );
	dStatus.MatchTupletf ( "dist_local", "%0.3F", tStats.m_iDistLocalTime.load ( std::memory_order_relaxed ) / 1000 );
	dStatus.MatchTupletf ( "dist_wait", "%0.3F", tStats.m_iDistWaitTime.load ( std::memory_order_relaxed ) / 1000 );

	if ( g_bIOStats )
	{
		dStatus.MatchTupletf ( "query_reads", "%l", tStats.m_iDiskReads.load ( std::memory_order_relaxed ) );
		dStatus.MatchTupletf ( "query_readkb", "%l", tStats.m_iDiskReadBytes.load ( std::memory_order_relaxed )/ 1024 );
		dStatus.MatchTupletf ( "query_readtime", "%l", tStats.m_iDiskReadTime.load ( std::memory_order_relaxed ) );
	} else
	{
		dStatus.MatchTuplet ( "query_reads", OFF );
		dStatus.MatchTuplet ( "query_readkb", OFF );
		dStatus.MatchTuplet ( "query_readtime", OFF );
	}

	if ( tStats.m_iPredictedTime.load ( std::memory_order_relaxed ) || tStats.m_iAgentPredictedTime.load ( std::memory_order_relaxed ) )
	{
		dStatus.MatchTupletf ( "predicted_time", "%l", tStats.m_iPredictedTime.load ( std::memory_order_relaxed ) );
		dStatus.MatchTupletf ( "dist_predicted_time", "%l", tStats.m_iAgentPredictedTime.load ( std::memory_order_relaxed ) );
	}

	dStatus.MatchTupletf ( "avg_query_wall", "%0.3F", tStats.m_iQueryTime.load ( std::memory_order_relaxed ) / iDiv1000 );

	if ( g_bCpuStats )
		dStatus.MatchTupletf ( "avg_query_cpu", "%0.3F", tStats.m_iQueryCpuTime.load ( std::memory_order_relaxed ) / iDiv1000 );
	else
		dStatus.MatchTuplet ( "avg_query_cpu", OFF );

	dStatus.MatchTupletf ( "avg_dist_wall", "%0.3F", tStats.m_iDistWallTime.load ( std::memory_order_relaxed ) / iDDiv1000 );
	dStatus.MatchTupletf ( "avg_dist_local", "%0.3F", tStats.m_iDistLocalTime.load ( std::memory_order_relaxed ) / iDDiv1000 );
	dStatus.MatchTupletf ( "avg_dist_wait", "%0.3F", tStats.m_iDistWaitTime.load ( std::memory_order_relaxed ) / iDDiv1000 );

	if ( g_bIOStats )
	{
		dStatus.MatchTupletf ( "avg_query_reads", "%0.1F", tStats.m_iDiskReads.load ( std::memory_order_relaxed ) * 10 / iQueriesDiv );
		dStatus.MatchTupletf ( "avg_query_readkb", "%0.1F", tStats.m_iDiskReadBytes.load ( std::memory_order_relaxed ) * 10 / (iQueriesDiv*1024) );
		dStatus.MatchTupletf ( "avg_query_readtime", "%0.3F", tStats.m_iDiskReadTime.load ( std::memory_order_relaxed ) / iDiv1000 );
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
	auto tSample = GlobalWorkPool()->Tasks();
	auto tCurrent = GlobalWorkPool()->CurTasks();
	auto iQueue = tSample.iPri + tSample.iSec + tCurrent;
	auto iTasks = myinfo::CountTasks ();
	auto & g_tStats = gStats ();
	sOut.StartBlock ( " " );
	sOut
	<< "Uptime:" << (DWORD) time ( NULL )-g_tStats.m_uStarted
	<< " Threads:" << iThreads;
	sOut.Sprintf (" Queue now+pri+sec=total: %d+%d+%d=%d", tCurrent, tSample.iPri, tSample.iSec, iQueue );
	sOut
	<< " Clients:" << myinfo::CountClients()
	<< " Vip clients:" << session::GetVips()
	<< " Buddy clients:" << session::GetBuddyCount()
	<< " Tasks:" << iTasks
	<< " Queries:" << g_tStats.m_iQueries.load ( std::memory_order_relaxed );
	sOut.Sprintf ( " Wall: %t", (int64_t)g_tStats.m_iQueryTime.load ( std::memory_order_relaxed ) );
	sOut.Sprintf ( " CPU: %t", (int64_t)g_tStats.m_iQueryCpuTime.load ( std::memory_order_relaxed ) );
	sOut.Sprintf ( "\nQueue/Th: %0.1F%", iQueue * 10 / iThreads );
	sOut.Sprintf ( " Tasks/Th: %0.1F%", iTasks * 10 / iThreads );
	sOut.Sprintf ( "\nLoad average: %0.2f, %0.2f, %0.2f", g_tStat1m.Value(), g_tStat5m.Value(), g_tStat15m.Value() );
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
				dStatus.MatchTupletf ( "status_error", "No such distributed table or agent: %s", sIndexOrAgent.cstr () );
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

void HandleCommandSphinxql ( GenericOutputBuffer_c & tOut, WORD uVer, InputBuffer_c & tReq ); // definition is below
void HandleCommandJson ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq );
void StatCountCommand ( SearchdCommand_e eCmd );
void HandleCommandUserVar ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq );
void HandleCommandCallPq ( ISphOutputBuffer &tOut, WORD uVer, InputBuffer_c &tReq );
static void HandleCommandSuggest ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq );

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


void ExecuteApiCommand ( SearchdCommand_e eCommand, WORD uCommandVer, int iLength, InputBuffer_c & tBuf, GenericOutputBuffer_c & tOut )
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
	AT_SCOPE_EXIT ( []() { myinfo::SetCommandDone(); } );

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
		case SEARCHD_COMMAND_CLUSTER:	HandleAPICommandCluster ( tOut, uCommandVer, tBuf, tSess.szClientName() ); break;
		case SEARCHD_COMMAND_GETFIELD:	HandleCommandGetField ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_SUGGEST:	HandleCommandSuggest ( tOut, uCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_PERSIST: break; // already processes, here just for stat

		default:
			SendErrorReply ( tOut, "internal error: unhandled command" );
			break;
	}
}


void StmtErrorReporter_i::Error ( const char * sTemplate, ... )
{
	StringBuilder_c sBuf;

	va_list ap;
	va_start ( ap, sTemplate );
	sBuf.vAppendf ( sTemplate, ap );
	va_end ( ap );

	ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sBuf.cstr () );
}

class StmtErrorReporter_c final : public StmtErrorReporter_i
{
public:
	explicit StmtErrorReporter_c ( RowBuffer_i & tBuffer )
		: m_tRowBuffer ( tBuffer )
	{}

	void Ok ( int iAffectedRows, const CSphString & sWarning, int64_t iLastInsertId ) final
	{
		m_tRowBuffer.Ok ( iAffectedRows, ( sWarning.IsEmpty() ? 0 : 1 ), sWarning.cstr(), false, iLastInsertId );
	}

	void Ok ( int iAffectedRows, int nWarnings ) final
	{
		m_tRowBuffer.Ok ( iAffectedRows, nWarnings );
	}

	void ErrorEx ( EMYSQL_ERR iErr, const char * sError ) final
	{
		m_tRowBuffer.Error ( sError, iErr );
	}

	RowBuffer_i * GetBuffer() final { return &m_tRowBuffer; }

private:
	RowBuffer_i & m_tRowBuffer;
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
		tAttr.SetValueInt ( dBson.Int() );
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

	CSphString sError;
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
				CSphMatchVariant::SetAttr ( tDoc, pItem->m_tLoc, &sName, tAttr, pItem->m_eType, false, sError );
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
	tOut.HeadBegin ();
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

		if ( !tOut.Commit() )
			return;
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
		return sMsg.Warn ( "No more docs for sparse matching" );

	auto pServed = GetServed ( sIndex );
	if ( !pServed )
		return sMsg.Err ( "unknown local table '%s' in search request", sIndex.cstr () );

	if ( pServed->m_eType!=IndexType_e::PERCOLATE )
		return sMsg.Err ( "table '%s' is not percolate", sIndex.cstr () );

	pServed->m_pStats->IncCmd ( SEARCHD_COMMAND_CALLPQ );

	RIdx_T<PercolateIndex_i*> pIndex { pServed };
	RtAccum_t * pAccum = tAcc.GetAcc ( pIndex, sError );
	sMsg.Err ( sError );

	if ( !sMsg.ErrEmpty () )
		return;

	const CSphSchema & tSchema = pIndex->GetInternalSchema();
	int iFieldsCount = tSchema.GetFieldsCount();

	InsertDocData_c tDoc(tSchema);

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

		tDoc.FixParsedMVAs ( dMvaParsed, iMvaCounter );

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
			pConn->m_iMyConnectTimeoutMs = pDist->GetAgentConnectTimeoutMs();
			pConn->m_iMyQueryTimeoutMs = pDist->GetAgentQueryTimeoutMs();
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
		tOut.Error ( "PQ() expects exactly 2 arguments (table, document(s))" );
		return;
	}
	auto &dStmtIndex = tStmt.m_dInsertValues[0];
	auto &dStmtDocs = tStmt.m_dInsertValues[1];

	if ( dStmtIndex.m_iType!=SqlInsert_t::QUOTED_STRING )
	{
		tOut.Error ( "PQ() argument 1 must be a string" );
		return;
	}
	if ( dStmtDocs.m_iType!=SqlInsert_t::QUOTED_STRING && dStmtDocs.m_iType!=SqlInsert_t::CONST_STRINGS )
	{
		tOut.Error ( "PQ() argument 2 must be a string or a string list" );
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

		} else if ( sOpt=="docs" )		tOpts.m_bGetDocs = ( v.GetValueInt()!=0 );
		else if ( sOpt=="verbose" )		tOpts.m_bVerbose = ( v.GetValueInt()!=0 );
		else if ( sOpt=="docs_json" )	tOpts.m_bJsonDocs = ( v.GetValueInt()!=0 );
		else if ( sOpt=="query" )		tOpts.m_bGetQuery = ( v.GetValueInt()!=0 );
		else if ( sOpt=="skip_bad_json" )	tOpts.m_bSkipBadJson = ( v.GetValueInt()!=0 );
		else if ( sOpt=="skip_empty" ) 	bSkipEmpty = true;
		else if ( sOpt=="shift" ) 		tOpts.m_iShift = v.GetValueInt();
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
		tOut.Error ( sError.cstr() );
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
			tOut.Error ( sBad.cstr ());
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
		tOut.Error ( sError.cstr () );
		return;
	}

	SendMysqlPercolateReply ( tOut, tResult, tOpts.m_iShift );
}

void HandleMysqlPercolateMeta ( const CPqResult &tResult, const CSphString & sWarning, RowBuffer_i & tOut )
{
	// shortcuts
	const PercolateMatchResult_t &tMeta = tResult.m_dResult;

	tOut.HeadTuplet ( "Variable name", "Value" );
	tOut.DataTupletf ( "total", "%.3D sec", tMeta.m_tmTotal / 1000 );
	if ( tMeta.m_tmSetup && tMeta.m_tmSetup>0 )
		tOut.DataTupletf ( "setup", "%.3D sec", tMeta.m_tmSetup / 1000 );
	tOut.DataTuplet ( "queries_matched", tMeta.m_iQueriesMatched );
	tOut.DataTuplet ( "queries_failed", tMeta.m_iQueriesFailed );
	tOut.DataTuplet ( "document_matched", tMeta.m_iDocsMatched );
	tOut.DataTuplet ( "total_queries_stored", tMeta.m_iTotalQueries );
	tOut.DataTuplet ( "term_only_queries", tMeta.m_iOnlyTerms );
	tOut.DataTuplet ( "fast_rejected_queries", tMeta.m_iEarlyOutQueries );

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
		tOut.DataTuplet ( "time_per_query", sList.cstr() );
		tOut.DataTuplet ( "time_of_matched_queries", tmMatched );
	}
	if ( !sWarning.IsEmpty() )
		tOut.DataTuplet ( "warning", sWarning.cstr() );

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
		if ( sphIsInternalAttr ( tSchema.GetAttr(i) ) )
			dAttrSchema[i]=-1;
		else
		{
			// check for string field/attr with the same name
			int iFieldId = tSchema.GetFieldIndex ( tSchema.GetAttr(i).m_sName.cstr() );
			if ( iFieldId!=-1 )
				dAttrSchema[i] = iFieldId+1;
			else
				dAttrSchema[i] = iAttrId++;
		}
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
			tOut.ErrorEx ( EMYSQL_ERR::FIELD_SPECIFIED_TWICE, sError.cstr() );
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

class AttributeConverter_c : public InsertDocData_c
{
public:
				AttributeConverter_c ( const CSphSchema & tSchema, const CSphVector<bool> & dFieldAttrs, CSphString & sError, CSphString & sWarning );

	bool		SetAttrValue ( int iCol, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx, CSphString & sError );
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
	CSphVector<float>		m_dTmpFloats;
	CSphVector<int>			m_dColumnarRemap;

	CSphString &			m_sError;
	CSphString &			m_sWarning;

	bool		String2JsonPack ( char * pStr, CSphVector<BYTE> & dBuf );

	bool		CheckStrings ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iCol, int iRow );
	bool		CheckJson ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal );
	bool		CheckMVA ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iCol, int iRow );
	bool		CheckInsertTypes ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx );
};


AttributeConverter_c::AttributeConverter_c ( const CSphSchema & tSchema, const CSphVector<bool> & dFieldAttrs, CSphString & sError, CSphString & sWarning )
	: InsertDocData_c ( tSchema )
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


bool AttributeConverter_c::CheckMVA ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iCol, int iRow )
{
	if ( tCol.m_eAttrType!=SPH_ATTR_UINT32SET && tCol.m_eAttrType!=SPH_ATTR_INT64SET && tCol.m_eAttrType!=SPH_ATTR_FLOAT_VECTOR )
		return true;

	if ( !tVal.m_pVals )
	{
		AddMVALength(0);
		return true;
	}

	auto & tAddVals = *tVal.m_pVals;
	if ( tCol.m_eAttrType==SPH_ATTR_FLOAT_VECTOR )
	{
		AddMVALength ( tAddVals.GetLength() );

		if ( tCol.IsIndexedKNN() && tCol.m_tKNN.m_eHNSWSimilarity==knn::HNSWSimilarity_e::COSINE && tAddVals.GetLength() )
		{
			m_dTmpFloats.Resize ( tAddVals.GetLength() );
			ARRAY_FOREACH ( i, m_dTmpFloats )
				m_dTmpFloats[i] = tAddVals[i].m_fValue;

			NormalizeVec(m_dTmpFloats);

			for ( const auto & i : m_dTmpFloats )
				AddMVAValue ( sphF2DW(i) );
		}
		else
		{
			for ( const auto & i : tAddVals )
				AddMVAValue ( sphF2DW ( i.m_fValue ) );
		}

		return true;
	}

	// collect data from scattered insvals
	// FIXME! maybe remove this mess, and just have a single m_dMvas pool in parser instead?
	bool bFloatInMVA = false;
	for ( const auto & i : tAddVals )
		bFloatInMVA |= i.m_bFloat;

	if ( bFloatInMVA )
		m_sWarning.SetSprintf ( "MVA attribute %d at row %d: inserting float value", iCol, iRow );

	tAddVals.Uniq();
	AddMVALength ( tAddVals.GetLength() );
	for ( const auto & i : tAddVals )
		AddMVAValue ( i.m_iValue );

	return true;
}


bool AttributeConverter_c::CheckInsertTypes ( const CSphColumnInfo & tCol, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx )
{
	// null fits all values as for now it sets default value for any type
	if ( tVal.m_iType==SqlInsert_t::TOK_NULL )
		return true;

	if ( tVal.m_iType!=SqlInsert_t::QUOTED_STRING
		&& tVal.m_iType!=SqlInsert_t::CONST_INT
		&& tVal.m_iType!=SqlInsert_t::CONST_FLOAT
		&& tVal.m_iType!=SqlInsert_t::CONST_MVA )
	{
		m_sError.SetSprintf ( "row %d, column %d: internal error: unknown insval type %d", 1+iRow, 1+iQuerySchemaIdx, tVal.m_iType ); // 1 for human base
		return false;
	}

	if ( tVal.m_iType==SqlInsert_t::CONST_MVA && !( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET || tCol.m_eAttrType==SPH_ATTR_JSON || tCol.m_eAttrType==SPH_ATTR_FLOAT_VECTOR ) )
	{
		m_sError.SetSprintf ( "row %d, column %d: MVA value specified for a non-MVA column", 1+iRow, 1+iQuerySchemaIdx ); // 1 for human base
		return false;
	}

	if ( ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET ) && tVal.m_iType!=SqlInsert_t::CONST_MVA )
	{
		m_sError.SetSprintf ( "row %d, column %d: non-MVA value specified for a MVA column", 1+iRow, 1+iQuerySchemaIdx ); // 1 for human base
		return false;
	}

	if ( tCol.m_eAttrType==SPH_ATTR_FLOAT_VECTOR && tVal.m_iType!=SqlInsert_t::CONST_MVA )
	{
		m_sError.SetSprintf ( "row %d, column %d: incompatible value specified for a float vector column", 1+iRow, 1+iQuerySchemaIdx ); // 1 for human base
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

	if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET || tCol.m_eAttrType==SPH_ATTR_FLOAT_VECTOR )
		AddMVALength ( 0, true );

	SqlInsert_t tDefaultVal;
	tDefaultVal.m_iType = SqlInsert_t::CONST_INT;
	tDefaultVal.SetValueInt(0);

	SphAttr_t tAttr;
	CSphString sError;
	if ( CSphMatchVariant::ConvertPlainAttr ( tDefaultVal, tCol.m_eAttrType, &tCol.m_sName, tAttr, false, sError ) )
	{
		if ( tCol.IsColumnar() )
			m_dColumnarAttrs [ m_dColumnarRemap[iCol] ] = tAttr;
		else
			m_tDoc.SetAttr ( tLoc, tAttr );
	}
}


bool AttributeConverter_c::SetAttrValue ( int iCol, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx, CSphString & sError )
{
	const CSphColumnInfo & tCol = m_tSchema.GetAttr(iCol);
	bool bDocId = tCol.m_sName == sphGetDocidName();
	CSphAttrLocator tLoc = tCol.m_tLocator;
	tLoc.m_bDynamic = true;

	if ( !CheckInsertTypes ( tCol, tVal, iRow, iQuerySchemaIdx ) )
		return false;

	SphAttr_t tAttr;
	if ( CSphMatchVariant::ConvertPlainAttr ( tVal, tCol.m_eAttrType, &tCol.m_sName, tAttr, bDocId, sError ) )
	{
		if ( tCol.IsColumnar() )
			m_dColumnarAttrs [ m_dColumnarRemap[iCol] ] = tAttr;
		else
			m_tDoc.SetAttr ( tLoc, tAttr );
	}
	else
	{
		if ( !sError.IsEmpty() )
			return false;
	}

	if ( !CheckStrings ( tCol, tVal, iCol, iRow ) )	return false;
	if ( !CheckJson ( tCol, tVal ) )				return false;
	if ( !CheckMVA ( tCol, tVal, iCol, iRow ) )		return false;

	return true;
}


bool AttributeConverter_c::SetFieldValue ( int iField, const SqlInsert_t & tVal, int iRow, int iQuerySchemaIdx )
{
	if ( tVal.m_iType!=SqlInsert_t::QUOTED_STRING && tVal.m_iType!=SqlInsert_t::TOK_NULL )
	{
		m_sError.SetSprintf ( "row %d, column %d: string expected", 1+iRow, 1+iQuerySchemaIdx ); // 1 for human base
		return false;
	}

	const char * szFieldValue = tVal.m_sVal.scstr();
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
	ResetMVAs();
}


void AttributeConverter_c::Finalize()
{
	// remap JSON to string pointers
	m_tStrings.SavePointersTo ( m_dStrings );
}

/////////////////////////////////////////////////////////////////////

static bool InsertToPQ ( const SqlStmt_t & tStmt, RtIndex_i * pIndex, RtAccum_t * pAccum, CSphVector<int64_t> & dIds, const CSphMatch & tDoc, const CSphAttrLocator & tIdLoc, const CSphVector<const char *> & dStrings,
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
		auto * pCmd = pAccum->AddCommand ( ReplCmd_e::PQUERY_ADD, tStmt.m_sIndex, tStmt.m_sCluster );
		dIds.Add ( pStored->m_iQUID );
		pCmd->m_pStored = std::move ( pStored );
	}

	return true;
}

static bool CleanupAcc ( bool bMissed, RtAccum_t * pAccum, CSphString & sError )
{
	assert ( pAccum );
	sError.SetSprintf ( "can not finish transaction, table %s '%s'", ( bMissed ? "missed" : "changed" ), pAccum->GetIndexName().cstr() );
	pAccum->Cleanup();
	return false;
}

static bool CheckAccIndex ( CSphSessionAccum & tSession, CSphString & sError, bool bIncServedCmd )
{
	RtAccum_t * pAccum = tSession.GetAcc();
	assert ( pAccum );
	auto pServed = GetServed ( pAccum->GetIndexName() );
	if ( !pServed )
		return CleanupAcc ( true, pAccum, sError );

	if ( bIncServedCmd )
		pServed->m_pStats->IncCmd ( SEARCHD_COMMAND_COMMIT );

	if ( pAccum->GetIndexId()!=RIdx_T<RtIndex_i*>( pServed )->GetIndexId() )
		return CleanupAcc ( false, pAccum, sError );

	return true;
}

void sphHandleMysqlBegin ( StmtErrorReporter_i& tOut, Str_t sQuery )
{
	auto* pSession = session::GetClientSession();
	auto& tAcc = pSession->m_tAcc;
	auto& sError = pSession->m_sError;

	MEMORY ( MEM_SQL_BEGIN );
	if ( tAcc.GetIndex() )
	{
		if ( !CheckAccIndex ( tAcc, sError, false ) )
			return tOut.Error ( "%s", sError.cstr() );

		if ( !HandleCmdReplicate ( *tAcc.GetAcc() ) )
		{
			TlsMsg::MoveError ( sError );
			return tOut.Error ( "%s", sError.cstr() );
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
	TRACE_CONN ( "conn", "sphHandleMysqlCommitRollback" );

	MEMORY ( MEM_SQL_COMMIT );
	pSession->m_bInTransaction = false;
	RtIndex_i* pIndex = tAcc.GetIndex();
	int iDeleted = 0;
	if ( pIndex )
	{
		RtAccum_t * pAccum = tAcc.GetAcc();
		tCrashQuery.m_dIndex = FromStr ( pAccum->GetIndexName() );

		if ( !CheckAccIndex ( tAcc, sError, true ) )
			return tOut.Error ( "%s", sError.cstr() );

		if ( bCommit )
		{
			StatCountCommand ( SEARCHD_COMMAND_COMMIT );
			if ( !HandleCmdReplicateDelete ( *pAccum, iDeleted ) )
			{
				TlsMsg::MoveError(sError);
				tOut.Error ( "%s", sError.cstr() );
				return;
			}
		} else
		{
			pIndex->RollBack ( pAccum );
		}
	}
	tOut.Ok ( iDeleted );
}

static bool AddDocument ( const SqlStmt_t & tStmt, cServedIndexRefPtr_c & pServed, StmtErrorReporter_i & tOut );
static void CommitAcc ( const SqlStmt_t & tStmt, cServedIndexRefPtr_c & pServed, StmtErrorReporter_i & tOut );

void sphHandleMysqlInsert ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	auto* pSession = session::GetClientSession();
	pSession->FreezeLastMeta();
	bool bReplace = ( tStmt.m_eStmt == STMT_REPLACE );
	StatCountCommand ( bReplace ? SEARCHD_COMMAND_REPLACE : SEARCHD_COMMAND_INSERT );

	MEMORY ( MEM_SQL_INSERT );

	auto tmStart = sphMicroTimer ();

	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( !ServedDesc_t::IsMutable ( pServed ) )
	{
		bool bDistTable = false;
		if ( !pServed )
		{
			bDistTable = GetDistr ( tStmt.m_sIndex );
		}

		if ( pServed || bDistTable )
			tOut.Error ( "table '%s' does not support INSERT", tStmt.m_sIndex.cstr ());
		else
			tOut.Error ( "table '%s' absent", tStmt.m_sIndex.cstr ());

		return;
	}

	assert ( pServed );
	Threads::Coro::ScopedWriteTable_c tWriting { pServed->Locker() };
	if ( !tWriting.CanWrite() )
	{
		tOut.Error ( "table '%s' is locked", tStmt.m_sIndex.cstr ());
		return;
	}

	GlobalCrashQueryGetRef().m_dIndex = FromStr ( tStmt.m_sIndex );

	// with index RLocked at the
	if ( !AddDocument ( tStmt, pServed, tOut ) )
		return;

	// index lock after replication takes place
	CommitAcc ( tStmt, pServed, tOut );
	pServed->m_pStats->AddWriteStat ( SearchdStats_t::eReplace, bReplace, tStmt.m_iRowsAffected, tmStart );
	gStats().AddDetailed ( SearchdStats_t::eReplace, tStmt.m_iRowsAffected, tmStart );
}

// when index name came as `cluster:name`, it came to the name, and should be splitted to cluster and index name
void MaybeFixupIndexNameFromMysqldump ( SqlStmt_t & tStmt )
{
	if ( g_pLocalIndexes->Contains ( tStmt.m_sIndex ) )
		return;

	auto dParts = sphSplit ( tStmt.m_sIndex.cstr (), ":" );
	if ( dParts.GetLength ()!=2 )
		return;

	tStmt.m_sCluster = dParts[0];
	tStmt.m_sIndex = dParts[1];
}

static bool AddDocument ( const SqlStmt_t & tStmt, cServedIndexRefPtr_c & pServed, StmtErrorReporter_i & tOut )
{
	auto * pSession = session::GetClientSession();
	auto & tAcc = pSession->m_tAcc;
	bool bReplace = ( tStmt.m_eStmt == STMT_REPLACE );
	auto & dLastIds = pSession->m_dLastIds;

	CSphString sError;
	auto & sWarning = pSession->m_tLastMeta.m_sWarning;

	bool bPq = ( pServed->m_eType==IndexType_e::PERCOLATE );

	RIdx_T<RtIndex_i*> pIndex { pServed };

	// get schema, check values count
	const CSphSchema & tSchema = pIndex->GetMatchSchema ();
	int iSchemaSz = tSchema.GetAttrsCount() + tSchema.GetFieldsCount();
	if ( pIndex->GetSettings().m_bIndexFieldLens )
		iSchemaSz -= tSchema.GetFieldsCount();

	// check for 'string indexed attribute'
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		auto & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_eAttrType==SPH_ATTR_STRING && tSchema.GetField ( tAttr.m_sName.cstr() ) )
			iSchemaSz--;
	}

	if ( tSchema.GetAttr ( sphGetBlobLocatorName() ) )
		iSchemaSz--;

	int iExp = tStmt.m_iSchemaSz;
	int iGot = tStmt.m_dInsertValues.GetLength();
	if ( !tStmt.m_dInsertSchema.GetLength()	&& iSchemaSz!=tStmt.m_iSchemaSz )
	{
		tOut.Error ( "column count does not match schema (expected %d, got %d)", iSchemaSz, iGot );
		return false;
	}

	if ( ( iGot % iExp )!=0 )
	{
		tOut.Error ( "column count does not match value count (expected %d, got %d)", iExp, iGot );
		return false;
	}

	if ( !ValidateClusterStatement ( tStmt.m_sIndex, *pServed, tStmt.m_sCluster, IsHttpStmt ( tStmt ) ) )
		TlsMsg::MoveError ( sError );

	if ( !sError.IsEmpty() )
	{
		tOut.ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sError.cstr() );
		return false;
	}

	CSphVector<int> dAttrSchema ( tSchema.GetAttrsCount() );
	CSphVector<int> dFieldSchema ( tSchema.GetFieldsCount() );
	CSphVector<bool> dFieldAttrs ( tSchema.GetFieldsCount() );
	if ( !CreateAttrMaps ( dAttrSchema, dFieldSchema, dFieldAttrs, tSchema, tStmt.m_dInsertSchema, tOut ) )
		return false;

	RtAccum_t * pAccum = tAcc.GetAcc ( pIndex, sError );
	if ( !sError.IsEmpty() )
	{
		tOut.ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sError.cstr() );
		return false;
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
		int iFieldLenAttr = tSchema.GetAttrId_FirstFieldLen();
		if ( iFieldLenAttr>=0 )
			iSchemaAttrCount = iFieldLenAttr;

		// process attrs up to the first autogenerated field length attr
		bool bOk = true;
		for ( int i=0; i<iSchemaAttrCount && bOk; i++ )
		{
			int iQuerySchemaIdx = dAttrSchema[i];
			if ( iQuerySchemaIdx < 0 )
				tConverter.SetDefaultAttrValue(i);
			else
				bOk = tConverter.SetAttrValue ( i, tStmt.m_dInsertValues[iQuerySchemaIdx + iRow * iExp], iRow, iQuerySchemaIdx, sError );
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

			pAccum->AddCommand ( ReplCmd_e::RT_TRX, tStmt.m_sIndex, tStmt.m_sCluster );
		}

		if ( !sError.IsEmpty() )
			break;
	}

	// fire exit
	if ( !sError.IsEmpty() )
	{
		pIndex->RollBack ( pAccum ); // clean up collected data
		tOut.ErrorEx ( EMYSQL_ERR::PARSE_ERROR, sError.cstr() );
		return false;
	}

	dLastIds.SwapData ( dIds );
	return true;
}

static void CommitAcc ( const SqlStmt_t & tStmt, cServedIndexRefPtr_c & pServed, StmtErrorReporter_i & tOut )
{
	auto * pSession = session::GetClientSession();

	CSphString sError;
	auto & sWarning = pSession->m_tLastMeta.m_sWarning;

	bool bCommit = ( pSession->m_bAutoCommit && !pSession->m_bInTransaction );
	auto & dLastIds = pSession->m_dLastIds;

	// no errors so far
	if ( bCommit )
	{
		RtAccum_t * pAccum = pSession->m_tAcc.GetAcc();
		// scoped lock only for assert
		{
			RIdx_T<RtIndex_i *> pIndex { pServed };
			assert ( pSession->m_tAcc.GetAcc ( pIndex, sError )==pAccum );
		}

		if ( !HandleCmdReplicate ( *pAccum ) )
		{
			TlsMsg::MoveError ( sError );
			// scoped lock for rollback call on index
			{
				RIdx_T<RtIndex_i *> pIndex { pServed };
				pIndex->RollBack ( pAccum ); // clean up collected data
			}
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
		tOut.Error ( "SNIPPETS() expects exactly 3 arguments (data, table, query)" );
		return;
	}
	if ( tStmt.m_dInsertValues[0].m_iType!=SqlInsert_t::QUOTED_STRING && tStmt.m_dInsertValues[0].m_iType!=SqlInsert_t::CONST_STRINGS )
	{
		tOut.Error ( "SNIPPETS() argument 1 must be a string or a string list" );
		return;
	}
	if ( tStmt.m_dInsertValues[1].m_iType!=SqlInsert_t::QUOTED_STRING )
	{
		tOut.Error ( "SNIPPETS() argument 2 must be a string" );
		return;
	}
	if ( tStmt.m_dInsertValues[2].m_iType!=SqlInsert_t::QUOTED_STRING )
	{
		tOut.Error ( "SNIPPETS() argument 3 must be a string" );
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

		else if ( sOpt=="limit" )				{ q.m_iLimit = (int)v.GetValueInt(); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="limit_words" )			{ q.m_iLimitWords = (int)v.GetValueInt(); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="limit_passages" || sOpt=="limit_snippets" ) { q.m_iLimitPassages = (int)v.GetValueInt(); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="around" )				{ q.m_iAround = (int)v.GetValueInt(); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="start_passage_id" || sOpt=="start_snippet_id" ) { q.m_iPassageId = (int)v.GetValueInt(); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="exact_phrase" )
		{
			sError.SetSprintf ( "exact_phrase is deprecated" );
			break;
		}
		else if ( sOpt=="use_boundaries" )		{ q.m_bUseBoundaries = ( v.GetValueInt()!=0 ); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="weight_order" )		{ q.m_bWeightOrder = ( v.GetValueInt()!=0 ); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="query_mode" )
		{
			bool bQueryMode = ( v.GetValueInt()!=0 );
			iExpType = SqlInsert_t::CONST_INT;
			if ( !bQueryMode )
			{
				sError.SetSprintf ( "query_mode=0 is deprecated" );
				break;
			}
		}
		else if ( sOpt=="force_all_words" )		{ q.m_bForceAllWords = ( v.GetValueInt()!=0 ); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="load_files" )			{ q.m_uFilesMode = ( v.GetValueInt()!=0 )?1:0; iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="load_files_scattered" ) { q.m_uFilesMode |= ( v.GetValueInt()!=0 )?2:0; iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="allow_empty" )			{ q.m_bAllowEmpty = ( v.GetValueInt()!=0 ); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="emit_zones" )			{ q.m_bEmitZones = ( v.GetValueInt()!=0 ); iExpType = SqlInsert_t::CONST_INT; }
		else if ( sOpt=="force_passages" || sOpt=="force_snippets" ) { q.m_bForcePassages = ( v.GetValueInt()!=0 ); iExpType = SqlInsert_t::CONST_INT; }
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
		tOut.Error ( sError.cstr() );
		return;
	}

	if ( !sphCheckOptionsSPZ ( q, q.m_ePassageSPZ, sError ) )
	{
		tOut.Error ( sError.cstr() );
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
		tOut.Error ( sError.cstr() );
		return;
	}

	if ( !dQueries.any_of ( [] ( const ExcerptQuery_t & tQuery ) { return tQuery.m_sError.IsEmpty(); } ) )
	{
		// just one last error instead of all errors is hopefully ok
		sError.SetSprintf ( "highlighting failed: %s", sError.cstr() );
		tOut.Error ( sError.cstr() );
		return;
	}

	// result set header packet
	tOut.HeadBegin ();
	tOut.HeadColumn("snippet");
	tOut.HeadEnd ();

	// data
	for ( auto & i : dQueries )
	{
		FixupResultTail ( i.m_dResult );
		tOut.PutArray ( i.m_dResult );
		if ( !tOut.Commit() )
			break;
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

static void LimitKeywords ( int iLimit, CSphVector<CSphKeywordInfo> & dKeywords );
static void SortKeywords ( const GetKeywordsSettings_t & tSettings, CSphVector<CSphKeywordInfo> & dKeywords );
bool DoGetKeywords ( const CSphString & sIndex, const CSphString & sQuery, const GetKeywordsSettings_t & tSettings, CSphVector <CSphKeywordInfo> & dKeywords, CSphString & sError, SearchFailuresLog_c & tFailureLog )
{
	auto pLocal = GetServed ( sIndex );
	auto pDistributed = GetDistr ( sIndex );

	if ( !pLocal && !pDistributed )
	{
		sError.SetSprintf ( "no such table %s", sIndex.cstr() );
		return false;
	}

	bool bOk = false;
	// just local plain or template index
	if ( pLocal )
	{
		pLocal->m_pStats->IncCmd ( SEARCHD_COMMAND_KEYWORDS );
		bOk = RIdx_c(pLocal)->GetKeywords ( dKeywords, sQuery.cstr(), tSettings, &sError );
	} else
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
				tFailureLog.Submit ( sLocal.cstr(), sIndex.cstr(), "missed table" );
				continue;
			}
			pServed->m_pStats->IncCmd ( SEARCHD_COMMAND_KEYWORDS );

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
			UniqKeywords ( dKeywords );

		bOk = true;
	}

	SortKeywords ( tSettings, dKeywords );
	if ( tSettings.m_iExpansionLimit )
		LimitKeywords ( tSettings.m_iExpansionLimit, dKeywords );

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
		tOut.Error ( "bad argument count or types in KEYWORDS() call" );
		return;
	}

	GetKeywordsSettings_t tSettings;
	tSettings.m_bStats = ( iArgs==3 && tStmt.m_dInsertValues[2].GetValueInt()!=0 );
	ARRAY_FOREACH ( i, tStmt.m_dCallOptNames )
	{
		CSphString & sOpt = tStmt.m_dCallOptNames[i];
		sOpt.ToLower ();
		const auto & sVal = tStmt.m_dCallOptValues[i].m_sVal;
		bool bEnabled = ( tStmt.m_dCallOptValues[i].GetValueInt()!=0 );
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
			tSettings.m_iExpansionLimit = int ( tStmt.m_dCallOptValues[i].GetValueInt() );
		else if ( sOpt=="sort_mode" )
		{
			// FIXME!!! add more sorting modes
			if ( sVal!="docs" && sVal!="hits" )
			{
				sError.SetSprintf ( "unknown option %s mode '%s'", sOpt.cstr(), sVal.cstr() );
				tOut.Error ( sError.cstr() );
				return;
			}

			tSettings.m_bSortByDocs = sVal=="docs";
			tSettings.m_bSortByHits = sVal=="hits";
			bOptInt = false;
						
		}
		else if ( sOpt=="jieba_mode" )
		{
			if ( !StrToJiebaMode ( tSettings.m_eJiebaMode, sVal, sError ) )
			{
				tOut.Error ( sError.cstr() );
				return;
			}
			bOptInt = false;
		}
		else
		{
			sError.SetSprintf ( "unknown option %s", sOpt.cstr () );
			tOut.Error ( sError.cstr() );
			return;
		}

		// post-conf type check
		if ( bOptInt && tStmt.m_dCallOptValues[i].m_iType!=SqlInsert_t::CONST_INT )
		{
			sError.SetSprintf ( "unexpected option %s type", sOpt.cstr () );
			tOut.Error ( sError.cstr () );
			return;
		}
	}
	const CSphString & sTerm = tStmt.m_dInsertValues[0].m_sVal;
	const CSphString & sIndex = tStmt.m_dInsertValues[1].m_sVal;
	CSphVector<CSphKeywordInfo> dKeywords;
	SearchFailuresLog_c tFailureLog;

	if ( !DoGetKeywords ( sIndex, sTerm, tSettings, dKeywords, sError, tFailureLog ) )
	{
		tOut.Error ( sError.cstr() );
		return;
	}


	// result set header packet
	tOut.HeadBegin();
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
		if ( !tOut.Commit() )
			break;
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
	tOut.SendInt ( (int)m_tSettings.m_eJiebaMode );
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

struct KeywordLimiter_fn
{
	const int m_iLimit = 0;
	int m_iCount = 0;

	KeywordLimiter_fn ( int iLimit )
		: m_iLimit ( iLimit )
	{}

	bool IsEq ( const CSphKeywordInfo & tKw1, const CSphKeywordInfo & tKw2 )
	{
		if ( tKw1.m_iQpos!=tKw2.m_iQpos )
		{
			m_iCount = 0;
			return false;
		}

		m_iCount++;
		return ( m_iCount>=m_iLimit );
	}
};

void LimitKeywords ( int iLimit, CSphVector<CSphKeywordInfo> & dKeywords )
{
	assert ( iLimit>0 );
	KeywordLimiter_fn tLimit ( iLimit );
	int iLen = sphUniq ( dKeywords.Begin(), dKeywords.GetLength(), tLimit );
	dKeywords.Resize ( iLen );
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

// sort by distance asc, document count desc, ABC asc
struct CmpFoundDocs_fn
{
	const char * m_pBuf;
	explicit CmpFoundDocs_fn ( const char * pBuf ) : m_pBuf ( pBuf ) {}

	inline bool IsLess ( const SuggestWord_t & a, const SuggestWord_t & b ) const
	{
		// sort by found_docs desc, dist asc
		if ( a.m_iFoundDocs!=b.m_iFoundDocs )
			return a.m_iFoundDocs>b.m_iFoundDocs;  // descending

		if ( a.m_iDistance==b.m_iDistance && a.m_iDocs==b.m_iDocs )
		{
			return ( sphDictCmpStrictly ( m_pBuf + a.m_iNameOff, a.m_iLen, m_pBuf + b.m_iNameOff, b.m_iLen )<0 );
		}

		if ( a.m_iDistance==b.m_iDistance )
			return a.m_iDocs>=b.m_iDocs;
		return a.m_iDistance<b.m_iDistance;
	}
};

static bool HasSuggestFoundDocs ( const SuggestArgs_t & tArgs ) { return tArgs.m_eSearch!=SuggestArgs_t::SearchMode_e::NONE; }

static void SuggestSendResult ( const SuggestArgs_t & tArgs, SuggestResultSet_t & tRes, const CSphString & sSentence, RowBuffer_i & tOut )
{
	if ( tArgs.m_bResultOneline )
	{
		// let's resort by alphabet to better compare result sets
		CmpDistDocABC_fn tCmp ( (const char *)( tRes.m_dBuf.Begin() ) );
		tRes.m_dMatched.Sort ( tCmp );

		// result set header packet
		tOut.HeadBegin ();
		tOut.HeadColumn ( "name" );
		tOut.HeadColumn ( "value" );
		tOut.HeadEnd ();

		StringBuilder_c sBuf ( "," );
		for ( auto& dMatched : tRes.m_dMatched )
			sBuf << (const char*) tRes.m_dBuf.Begin() + dMatched.m_iNameOff;
		tOut.PutString ( "suggests" );
		tOut.PutString ( sBuf.cstr() );
		if ( !tOut.Commit() )
			return;

		if ( tArgs.m_bResultStats )
		{
			sBuf.Clear ();
			sBuf.StartBlock ( "," );
			for ( auto &dMatched : tRes.m_dMatched )
				sBuf.Appendf ( "%d", dMatched.m_iDistance );
			tOut.PutString ( "distance" );
			tOut.PutString ( sBuf.cstr () );
			if ( !tOut.Commit() )
				return;
			sBuf.Clear ();
			sBuf.StartBlock ( "," );
			for ( auto &dMatched : tRes.m_dMatched )
				sBuf.Appendf ( "%d", dMatched.m_iDocs );
			tOut.PutString ( "docs" );
			tOut.PutString ( sBuf );
			if ( !tOut.Commit() )
				return;

			if ( HasSuggestFoundDocs ( tArgs ) )
			{
				sBuf.Clear ();
				sBuf.StartBlock ( "," );
				for ( const auto & dMatched : tRes.m_dMatched )
					sBuf.Appendf ( "%d", dMatched.m_iFoundDocs );
				tOut.PutString ( "found_docs" );
				tOut.PutString ( sBuf );
				if ( !tOut.Commit() )
					return;
			}
		}
	} else
	{
		// result set header packet
		tOut.HeadBegin ();
		tOut.HeadColumn ( "suggest" );
		if ( tArgs.m_bResultStats )
		{
			tOut.HeadColumn ( "distance" );
			tOut.HeadColumn ( "docs" );
			if ( HasSuggestFoundDocs ( tArgs ) )
				tOut.HeadColumn ( "found_docs" );
		}
		tOut.HeadEnd ();

		StringBuilder_c sBuf;
		auto * szResult = (const char *)( tRes.m_dBuf.Begin() );
		ARRAY_FOREACH ( i, tRes.m_dMatched )
		{
			const SuggestWord_t & tWord = tRes.m_dMatched[i];
			if ( tArgs.m_bSentence && !sSentence.IsEmpty() )
			{
				sBuf.Clear();
				sBuf.Appendf ( "%s %s", sSentence.cstr(), ( szResult + tWord.m_iNameOff ) );
				tOut.PutString ( sBuf );
			} else
			{
				tOut.PutString ( szResult + tWord.m_iNameOff );
			}
			if ( tArgs.m_bResultStats )
			{
				tOut.PutNumAsString ( tWord.m_iDistance );
				tOut.PutNumAsString ( tWord.m_iDocs );
				if ( HasSuggestFoundDocs ( tArgs ) )
					tOut.PutNumAsString ( tWord.m_iFoundDocs );
			}
			if ( !tOut.Commit() )
				return;
		}
	}

	tOut.Eof();
}

static void SuggestSearch ( const char * sIndexName, const SuggestArgs_t & tArgs, SuggestResult_t & tRes, bool bFinal );

static bool SuggestLocalIndexGet ( const cServedIndexRefPtr_c & pServed, const SuggestArgs_t & tArgs, const char * sWord, SuggestResult_t & tRes, CSphString & sError, bool bFinal )
{
	assert ( pServed );
	pServed->m_pStats->IncCmd ( SEARCHD_COMMAND_SUGGEST );

	RIdx_c pIdx { pServed };
	if ( !pIdx->GetSettings().m_iMinInfixLen || !pIdx->GetDictionary()->GetSettings().m_bWordDict )
	{
		sError.SetSprintf ( "suggests work only for keywords dictionary with infix enabled" );
		return false;
	}

	if ( tRes.SetWord ( sWord, pIdx->GetQueryTokenizer(), tArgs.m_bQueryMode, tArgs.m_bSentence, tArgs.m_bForceBigrams ) )
		pIdx->GetSuggest ( tArgs, tRes );

	SuggestSearch ( pIdx->GetName(), tArgs, tRes, bFinal );
	return true;
}

static void SuggestSearchSort ( const SuggestArgs_t & tArgs, SuggestResult_t & tRes )
{
	if ( tRes.m_sSentence.IsEmpty() || !HasSuggestFoundDocs ( tArgs ) || tRes.m_dMatched.IsEmpty() )
		return;

	// sort by found_docs desc, dist asc
	CmpFoundDocs_fn tCmp ( (const char *)( tRes.m_dBuf.Begin() ) );
	tRes.m_dMatched.Sort ( tCmp );
}

void SuggestSearch ( const char * sIndexName, const SuggestArgs_t & tArgs, SuggestResult_t & tRes, bool bFinal )
{
	if ( tRes.m_sSentence.IsEmpty() || !HasSuggestFoundDocs ( tArgs ) || tRes.m_dMatched.IsEmpty() )
		return;

	const CSphString & sSentence = tRes.m_sSentence;
	const BYTE * pBuf = tRes.m_dBuf.Begin();

	for ( auto & tWord : tRes.m_dMatched )
	{
		// char * vs Str_t is safe as all words are null-terminated
		const BYTE * sSuggested = pBuf + tWord.m_iNameOff;

		SearchHandler_c tHandler ( 1, sphCreatePlainQueryParser(), QUERY_SQL, true );
		CSphQuery & tQuery = tHandler.m_dQueries[0];
		tQuery.m_sIndexes = sIndexName;
		tQuery.m_iLimit = 1;
		tQuery.m_iMaxMatches = 1;
		// phrase or bag of words
		if ( tArgs.m_eSearch==SuggestArgs_t::SearchMode_e::PHRASE )
			tQuery.m_sQuery.SetSprintf ( "\"%s %s\"", sSentence.cstr(), sSuggested );
		else
			tQuery.m_sQuery.SetSprintf ( "%s %s", sSentence.cstr(), sSuggested );

		// only count(*) select list item
		CSphQueryItem & tItem = tQuery.m_dItems.Add();
		tItem.m_sExpr = "count(*)";
		tItem.m_sAlias = "count(*)";

		tHandler.RunQueries();

		// count(*) value from the result set into found_docs
		const auto & tResult = tHandler.m_dAggrResults[0];
		const CSphColumnInfo * pCol = tResult.m_tSchema.GetAttr ( "count(*)" );
		if ( tResult.m_iSuccesses > 0 && !tResult.m_dResults.IsEmpty() && !tResult.m_dResults[0].m_dMatches.IsEmpty() && pCol )
		{
			const CSphMatch & tMatch = tResult.m_dResults[0].m_dMatches[0];
			tWord.m_iFoundDocs = (int)tMatch.GetAttr ( pCol->m_tLocator );
		}
	}

	if ( bFinal )
		SuggestSearchSort ( tArgs, tRes );
}

static void MergetRestultSets ( const SuggestResult_t & tSrc, SuggestResult_t & tDst )
{
	if ( tDst.m_sSentence.IsEmpty() )
		tDst.m_sSentence = tSrc.m_sSentence;

	const BYTE * sBuf = ( tSrc.m_dBuf.Begin() );
	for ( const auto & tSrcWord : tSrc.m_dMatched )
	{
		auto & tDstWord = tDst.m_dMatched.Add();
		tDstWord = tSrcWord;
		tDstWord.m_iNameOff = tDst.m_dBuf.GetLength();

		BYTE * pDst = tDst.m_dBuf.AddN ( tSrcWord.m_iLen + 1 );
		memcpy ( pDst, sBuf + tSrcWord.m_iNameOff, tSrcWord.m_iLen + 1 );
	}
}

static void UniqRestultSets ( int iLimit, SuggestResult_t & tRes, bool bSortByFoundDocs )
{
	SuggestMergeDocs ( tRes.m_dMatched );

	if ( bSortByFoundDocs )
	{
		// sort by found_docs desc, dist asc
		CmpFoundDocs_fn tCmp ( (const char *)( tRes.m_dBuf.Begin() ) );
		tRes.m_dMatched.Sort ( tCmp );

	} else
	{
		CmpDistDocABC_fn tCmp ( (const char *)( tRes.m_dBuf.Begin() ) );
		tRes.m_dMatched.Sort ( tCmp );
	}

	tRes.Flattern ( iLimit );
}

/// Suggest Flags
enum class SuggestFlags_e : DWORD
{
	NON_CHAR_ALLOWED			= 1UL << 0,
	SENTENCE					= 1UL << 1,
	QUERY_MODE					= 1UL << 2,
	SEARCH_PHRASE				= 1UL << 3,
	SEARCH_WORDS				= 1UL << 4,
};

static void SendSuggestReply ( const SuggestResult_t & tRes, ISphOutputBuffer & tOut )
{
	auto tReply = APIAnswer ( tOut, VER_COMMAND_SUGGEST );
	tOut.SendString ( tRes.m_sSentence.cstr() );

	const BYTE * pBuf = tRes.m_dBuf.Begin();
	tOut.SendInt ( tRes.m_dMatched.GetLength() );
	for ( const auto & tWord : tRes.m_dMatched )
	{
		tOut.SendInt ( tWord.m_iDistance );
		tOut.SendInt ( tWord.m_iDocs );
		tOut.SendInt ( tWord.m_iFoundDocs );

		tOut.SendInt ( tWord.m_iLen );
		tOut.SendBytes ( pBuf + tWord.m_iNameOff, tWord.m_iLen );
	}
}


void HandleCommandSuggest ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( uVer, VER_COMMAND_SUGGEST, tOut ) )
		return;

	// parse request
	CSphString sIndex = tReq.GetString();
	CSphString sWord = tReq.GetString();

	SuggestArgs_t tArgs;
	tArgs.m_iLimit = tReq.GetInt();
	tArgs.m_iDeltaLen = tReq.GetInt();
	tArgs.m_iQueueLen = tReq.GetInt();
	tArgs.m_iRejectThr = tReq.GetInt();
	tArgs.m_iMaxEdits = tReq.GetInt();

	DWORD uFlags = tReq.GetDword();
	tArgs.m_bNonCharAllowed = !!( uFlags & (DWORD)SuggestFlags_e::NON_CHAR_ALLOWED );
	tArgs.m_bSentence = !!( uFlags & (DWORD)SuggestFlags_e::SENTENCE );
	tArgs.m_bQueryMode = !!( uFlags & (DWORD)SuggestFlags_e::QUERY_MODE );
	if ( uFlags & (DWORD)SuggestFlags_e::SEARCH_PHRASE )
		tArgs.m_eSearch = SuggestArgs_t::SearchMode_e::PHRASE;
	else if ( uFlags & (DWORD)SuggestFlags_e::SEARCH_WORDS )
		tArgs.m_eSearch = SuggestArgs_t::SearchMode_e::WORDS;

	if ( tReq.GetError() )
	{
		SendErrorReply ( tOut, "invalid or truncated request" );
		return;
	}

	auto pServed = GetServed ( sIndex );
	if ( !pServed )
	{
		SendErrorReply ( tOut, "missed table %s", sIndex.cstr() );
		return;
	}

	CSphString sError;
	SuggestResult_t tRes;
	if ( !SuggestLocalIndexGet ( pServed, tArgs, sWord.cstr(), tRes, sError, false ) )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return;
	}

	SendSuggestReply ( tRes, tOut );
}

class SuggestRequestBuilder_c : public RequestBuilder_i
{
public:
	SuggestRequestBuilder_c ( const SuggestArgs_t & tArgs, const char * sWord )
		: m_tArgs ( tArgs )
		, m_sWord ( sWord )
	{}

	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_SUGGEST, VER_COMMAND_SUGGEST );

		tOut.SendString ( tAgent.m_tDesc.m_sIndexes.cstr() );
		tOut.SendString ( m_sWord );

		tOut.SendInt ( m_tArgs.m_iLimit );
		tOut.SendInt ( m_tArgs.m_iDeltaLen );
		tOut.SendInt ( m_tArgs.m_iQueueLen );
		tOut.SendInt ( m_tArgs.m_iRejectThr );
		tOut.SendInt ( m_tArgs.m_iMaxEdits );

		DWORD uFlags = 0;
		uFlags |= (DWORD)SuggestFlags_e::NON_CHAR_ALLOWED * m_tArgs.m_bNonCharAllowed;
		uFlags |= (DWORD)SuggestFlags_e::SENTENCE * m_tArgs.m_bSentence;
		uFlags |= (DWORD)SuggestFlags_e::QUERY_MODE * m_tArgs.m_bQueryMode;
		uFlags |= (DWORD)SuggestFlags_e::SEARCH_PHRASE * ( m_tArgs.m_eSearch==SuggestArgs_t::SearchMode_e::PHRASE );
		uFlags |= (DWORD)SuggestFlags_e::SEARCH_WORDS * ( m_tArgs.m_eSearch==SuggestArgs_t::SearchMode_e::WORDS );
		tOut.SendDword ( uFlags );
	}

protected:
	const SuggestArgs_t & m_tArgs;
	const char * m_sWord;
};


class SuggestReplyParser_c : public ReplyParser_i
{
public:
	SuggestReplyParser_c ( SuggestResultSet_t & tRes, CSphString & sSentence )
		: m_tRes ( tRes )
		, m_sSentence ( sSentence )
	{}
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final
	{
		CSphString sSentence = tReq.GetString();
		if ( m_sSentence.IsEmpty() )
			m_sSentence = sSentence;

		int iWords = tReq.GetInt();
		int iOff = m_tRes.m_dMatched.GetLength();
		m_tRes.m_dMatched.Resize ( iOff + iWords );
		for ( int i=0; i<iWords; i++ )
		{
			SuggestWord_t & tWord = m_tRes.m_dMatched[iOff + i];
			tWord.m_iDistance = tReq.GetInt();
			tWord.m_iDocs = tReq.GetInt();
			tWord.m_iFoundDocs = tReq.GetInt();

			int iWordLen = tReq.GetInt();
			tWord.m_iNameOff = m_tRes.m_dBuf.GetLength();
			tWord.m_iLen = iWordLen + 1;
			BYTE * pDst = m_tRes.m_dBuf.AddN ( iWordLen + 1 );
			tReq.GetBytes ( pDst, iWordLen );
			pDst[iWordLen] = '\0';
			tWord.m_iNameHash = sphCRC32 ( pDst, iWordLen );
		}

		return true;
	}
	SuggestResultSet_t & m_tRes;
	CSphString & m_sSentence;
};

static bool SuggestDistIndexGet ( const cDistributedIndexRefPtr_t & pDistributed, const CSphString & sIndex, const SuggestArgs_t & tArgs, const char * sWord, SuggestResult_t & tRes, CSphString & sError )
{
	const StrVec_t & dLocals = pDistributed->m_dLocal;
	for ( const CSphString & sLocal : dLocals )
	{
		auto pServed = GetServed ( sLocal );
		if ( !pServed )
		{
			sError.SetSprintf ( sLocal.cstr(), sIndex.cstr(), "missed table %s at %s ", sLocal.cstr(), sIndex.cstr() );
			return false;
		}

		// search for locals but sort all suggestions in the end
		SuggestResult_t tCur;
		if ( !SuggestLocalIndexGet ( pServed, tArgs, sWord, tCur, sError, false ) )
			return false;

		MergetRestultSets ( tCur, tRes );
	}

	// remote agents requests send off thread
	VecRefPtrsAgentConn_t dAgents;
	// fixme! We don't need all hosts here, only usual selected mirrors
	pDistributed->GetAllHosts ( dAgents );

	int iAgentsReply = 0;
	if ( !dAgents.IsEmpty() )
	{
		SuggestResult_t tCur;
		// connect to remote agents and query them
		SuggestRequestBuilder_c tReqBuilder ( tArgs, sWord );
		SuggestReplyParser_c tParser ( tRes, tRes.m_sSentence );
		iAgentsReply = PerformRemoteTasks ( dAgents, &tReqBuilder, &tParser );

		for ( const AgentConn_t * pAgent : dAgents )
			if ( !pAgent->m_bSuccess && !pAgent->m_sFailure.IsEmpty() )
				sError.SetSprintf ( "agent %s: %s", pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
	}

	// sort and relimit results sets
	if ( ( iAgentsReply + dLocals.GetLength() )>1 )
		UniqRestultSets ( tArgs.m_iLimit, tRes, HasSuggestFoundDocs ( tArgs ) );
	else
		SuggestSearchSort ( tArgs, tRes ); // need to sort by found_docs

	return true;
}

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
		tOut.Error ( "bad argument count or types in KEYWORDS() call" );
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
			tArgs.m_iLimit = int ( tStmt.m_dCallOptValues[i].GetValueInt() );
		} else if ( sOpt=="delta_len" )
		{
			tArgs.m_iDeltaLen = int ( tStmt.m_dCallOptValues[i].GetValueInt() );
		} else if ( sOpt=="max_matches" )
		{
			tArgs.m_iQueueLen = int ( tStmt.m_dCallOptValues[i].GetValueInt() );
		} else if ( sOpt=="reject" )
		{
			tArgs.m_iRejectThr = int ( tStmt.m_dCallOptValues[i].GetValueInt() );
		} else if ( sOpt=="max_edits" )
		{
			tArgs.m_iMaxEdits = int ( tStmt.m_dCallOptValues[i].GetValueInt() );
		} else if ( sOpt=="result_line" )
		{
			tArgs.m_bResultOneline = ( tStmt.m_dCallOptValues[i].GetValueInt()!=0 );
		} else if ( sOpt=="result_stats" )
		{
			tArgs.m_bResultStats = ( tStmt.m_dCallOptValues[i].GetValueInt()!=0 );
		} else if ( sOpt=="non_char" )
		{
			tArgs.m_bNonCharAllowed = ( tStmt.m_dCallOptValues[i].GetValueInt()!=0 );
		} else if ( sOpt=="sentence" )
		{
			tArgs.m_bSentence = ( tStmt.m_dCallOptValues[i].GetValueInt()!=0 );
		} else if ( sOpt=="force_bigrams" )
		{
			tArgs.m_bForceBigrams = ( tStmt.m_dCallOptValues[i].GetValueInt()!=0 );
		} else if ( sOpt=="search_mode" )
		{
			if ( tStmt.m_dCallOptValues[i].m_sVal=="phrase" )
			{
				tArgs.m_eSearch = SuggestArgs_t::SearchMode_e::PHRASE;
				iTokType = SqlInsert_t::QUOTED_STRING;
			}
			else if ( tStmt.m_dCallOptValues[i].m_sVal=="words" )
			{
				tArgs.m_eSearch = SuggestArgs_t::SearchMode_e::WORDS;
				iTokType = SqlInsert_t::QUOTED_STRING;
			}

		} else
		{
			sError.SetSprintf ( "unknown option %s", sOpt.cstr () );
			tOut.Error ( sError.cstr () );
			return;
		}

		// post-conf type check
		if ( tStmt.m_dCallOptValues[i].m_iType!=iTokType )
		{
			sError.SetSprintf ( "unexpected option %s type", sOpt.cstr () );
			tOut.Error ( sError.cstr () );
			return;
		}
	}

	const CSphString & sIndex = tStmt.m_dInsertValues[1].m_sVal;

	{
		auto pLocal = GetServed ( sIndex );
		auto pDistributed = GetDistr ( sIndex );

		if ( !pLocal && !pDistributed )
		{
			sError.SetSprintf ( "no such table %s", sIndex.cstr() );
			tOut.Error ( sError.cstr () );
			return;
		}

		if ( pLocal && !SuggestLocalIndexGet ( pLocal, tArgs, sWord, tRes, sError, true ) )
		{
			tOut.Error ( sError.cstr () );
			return;
		}
		if ( pDistributed && !SuggestDistIndexGet ( pDistributed, sIndex, tArgs, sWord, tRes, sError ) )
		{
			tOut.Error ( sError.cstr () );
			return;
		}
	}

	SuggestSendResult ( tArgs, tRes, tRes.m_sSentence, tOut );
}


static void HandleMysqlCallUuid ( RowBuffer_i & tOut, SqlStmt_t & tStmt )
{
	// the only int agrument
	int iArgs = tStmt.m_dInsertValues.GetLength ();
	if ( iArgs!=1 )
	{
		tOut.Error ( "bad argument count in UUID_SHORT() call" );
		return;
	}
	if ( tStmt.m_dInsertValues[0].m_iType!=SqlInsert_t::CONST_INT )
	{
		tOut.Error ( "bad argument type in UUID_SHORT() call" );
		return;
	}
	int64_t iCount = tStmt.m_dInsertValues[0].GetValueInt();
	if ( iCount<1 || iCount>INT_MAX )
	{
		tOut.Error ( "bad argument value in UUID_SHORT() call" );
		return;
	}

	tOut.HeadBegin ();
	tOut.HeadColumn ( "uuid_short()" );
	tOut.HeadEnd ();

	for ( int i=0; i<iCount; i++ )
	{
		tOut.PutNumAsString ( UidShort() );
		tOut.Commit();
	}

	tOut.Eof();
}


static CSphString DescribeAttributeProperties ( const CSphColumnInfo & tAttr )
{
	StringBuilder_c sProps(" ");
	if ( tAttr.IsColumnar() )
		sProps << "columnar";

	if ( tAttr.IsIndexedKNN() )
		sProps << "knn";

	if ( tAttr.IsIndexedSI() )
		sProps << "indexed";

	if ( tAttr.IsStored() )
		sProps << "fast_fetch";

	if ( tAttr.IsColumnar() && tAttr.m_eAttrType==SPH_ATTR_STRING && !(tAttr.m_uAttrFlags & CSphColumnInfo::ATTR_COLUMNAR_HASHES) )
		sProps << "no_hash";

	return sProps.cstr();
}


static void AddFieldDesc ( VectorLike & dOut, const CSphColumnInfo & tField, const CSphSchema & tSchema )
{
	if ( !dOut.MatchAdd ( tField.m_sName.cstr() ) )
		return;

	const CSphColumnInfo * pAttr = tSchema.GetAttr ( tField.m_sName.cstr() );
	dOut.Add ( pAttr ? "string" : "text" );

	StringBuilder_c sProperties ( " " );
	DWORD uFlags = tField.m_uFieldFlags;
	if ( uFlags & CSphColumnInfo::FIELD_INDEXED )
		sProperties << "indexed";

	if ( uFlags & CSphColumnInfo::FIELD_STORED )
		sProperties << "stored";

	if ( pAttr )
	{
		sProperties << "attribute";
		CSphString sProps = DescribeAttributeProperties ( *pAttr );
		if ( !sProps.IsEmpty() )
			sProperties << sProps;
	}

	dOut.Add ( sProperties.cstr () );
}


static void AddAttributeDesc ( VectorLike & dOut, const CSphColumnInfo & tAttr, const CSphSchema & tSchema )
{
	if ( sphIsInternalAttr ( tAttr ) )
		return;

	if ( tSchema.GetField ( tAttr.m_sName.cstr() ) )
		return; // already described it as a field property

	if ( dOut.MatchAdd ( tAttr.m_sName.cstr() ) )
	{
		if ( tAttr.m_eAttrType==SPH_ATTR_INTEGER && tAttr.m_tLocator.m_iBitCount!=ROWITEM_BITS && tAttr.m_tLocator.m_iBitCount>0 )
		{
			StringBuilder_c sName;
			sName.Sprintf ( "%s:%d", sphTypeName ( tAttr.m_eAttrType ), tAttr.m_tLocator.m_iBitCount );
			dOut.Add ( sName.cstr() );
		} else
			dOut.Add ( sphTypeName ( tAttr.m_eAttrType ) );

		dOut.Add ( DescribeAttributeProperties(tAttr) );
	}
}


void ShowFields ( VectorLike& dOut, const CSphSchema& tSchema )
{
	// result set header packet
	dOut.SetColNames ( { "Field", "Type", "Null", "Key", "Default", "Extra" } );

	auto Tail = [&dOut](const auto& tCol) { dOut.Add (tCol); dOut.Add ( "NO" ); dOut.Add ( "" ); dOut.Add ( "" ); dOut.Add ( "" ); };

	assert ( tSchema.GetAttr ( 0 ).m_sName == sphGetDocidName() );
	const auto& tId = tSchema.GetAttr ( 0 );
	if ( dOut.MatchAdd ( tId.m_sName.cstr() ) )
		Tail ( sphTypeName ( tId.m_eAttrType ) );

	for ( int i = 0; i < tSchema.GetFieldsCount(); ++i )
	{
		const auto& tField = tSchema.GetField ( i );
		if ( !dOut.MatchAdd ( tField.m_sName.cstr() ) )
			continue;

		const CSphColumnInfo* pAttr = tSchema.GetAttr ( tField.m_sName.cstr() );
		Tail ( pAttr ? "string" : "text" );
	}

	for ( int i = 1; i < tSchema.GetAttrsCount(); ++i ) // from 1, as 0 is docID and already emerged
	{
		const auto& tAttr = tSchema.GetAttr ( i );
		if ( sphIsInternalAttr ( tAttr ) )
			continue;

		if ( tAttr.m_eAttrType==SPH_ATTR_TOKENCOUNT )
			continue;

		if ( tSchema.GetField ( tAttr.m_sName.cstr() ) )
			continue; // already described it as a field property

		if ( !dOut.MatchAdd ( tAttr.m_sName.cstr() ) )
			continue;

		if ( tAttr.m_eAttrType == SPH_ATTR_INTEGER && tAttr.m_tLocator.m_iBitCount != ROWITEM_BITS && tAttr.m_tLocator.m_iBitCount > 0 )
			Tail ( SphSprintf ( "%s:%d", sphTypeName ( tAttr.m_eAttrType ), tAttr.m_tLocator.m_iBitCount ) );
		else
			Tail ( sphTypeName ( tAttr.m_eAttrType ) );
	}
}

void DescribeLocalSchema ( VectorLike & dOut, const CSphSchema & tSchema, bool bIsTemplate, bool bShowFields )
{
	if ( bShowFields )
		return ShowFields ( dOut, tSchema );

	// result set header packet
	dOut.SetColNames ( { "Field", "Type", "Properties" } );

	// id comes before fields
	if ( !bIsTemplate )
	{
		assert ( tSchema.GetAttr(0).m_sName==sphGetDocidName() );
		AddAttributeDesc ( dOut, tSchema.GetAttr(0), tSchema );
	}

	for ( int i = 0; i<tSchema.GetFieldsCount (); ++i )
		AddFieldDesc ( dOut, tSchema.GetField(i), tSchema );

	for ( int i = 1; i<tSchema.GetAttrsCount (); ++i )
		AddAttributeDesc ( dOut, tSchema.GetAttr(i), tSchema );
}


bool DescribeDistributedSchema ( VectorLike& dOut, const cDistributedIndexRefPtr_t & pDistr, bool bForce )
{
	if ( IsDistrTableHasSystem ( *pDistr, bForce ) )
		return false;

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

	return true;
}

void HandleCmdDescribe ( RowBuffer_i & tOut, SqlStmt_t * pStmt )
{
	auto & tStmt = *pStmt;
	VectorLike dOut ( tStmt.m_sStringParam, 0 );

	MaybeFixupIndexNameFromMysqldump ( tStmt );
	FixupSystemTableName (&tStmt);
	auto sName = tStmt.m_sIndex;
	auto pServed = GetServed ( sName );
	if ( pServed )
	{
		// data
		const CSphSchema *pSchema = &RIdx_c(pServed)->GetMatchSchema ();
		bool bNeedInternal = false;

		if ( tStmt.m_iIntParam==SqlInsert_t::TABLE ) // user wants internal schema instead
			bNeedInternal = true;

		if ( tStmt.m_dStringSubkeys.GetLength()==1 && tStmt.m_dStringSubkeys[0].EqN(".table") )
			bNeedInternal = true;

		bool bShowFields = tStmt.m_iIntParam == -2; // -2 emitted in parser

		if ( bNeedInternal && ServedDesc_t::IsMutable ( pServed ) && !bShowFields )
		{
			RIdx_T<const RtIndex_i*> pRtIndex { pServed };
			pSchema = &pRtIndex->GetInternalSchema();
		}

		const CSphSchema &tSchema = *pSchema;
		assert ( pServed->m_eType==IndexType_e::TEMPLATE || tSchema.GetAttr(0).m_sName==sphGetDocidName() );
		DescribeLocalSchema ( dOut, tSchema, pServed->m_eType==IndexType_e::TEMPLATE, bShowFields );
	} else
	{
		auto pDistr = GetDistr ( sName );
		if ( !pDistr )
		{
			tOut.ErrorAbsent ( "no such table '%s'", tStmt.m_sIndex.cstr () );
			return;
		}

		CSphString sError;
		if ( !DescribeDistributedSchema ( dOut, pDistr, tStmt.m_bForce ) )
		{
			tOut.ErrorAbsent ( "can not describe table '%s' because it contains system table", tStmt.m_sIndex.cstr() );
			return;
		}
	}

	tOut.DataTable ( dOut );
}

struct NamedIndexType_t
{
	CSphString m_sName;
	CSphString m_sCluster;
	IndexType_e m_eType;


	NamedIndexType_t() = default;
	NamedIndexType_t ( NamedIndexType_t && ) noexcept = default;
	NamedIndexType_t & operator= ( NamedIndexType_t && ) noexcept = default;
	NamedIndexType_t ( const NamedIndexType_t & ) noexcept = default;
	NamedIndexType_t & operator= ( const NamedIndexType_t & ) noexcept = default;


	NamedIndexType_t ( CSphString sName, CSphString sCluster, IndexType_e eType )
		: m_sName { std::move (sName) }
		, m_sCluster { std::move (sCluster) }
		, m_eType { eType }
	{}

	NamedIndexType_t ( CSphString sName, IndexType_e eType )
		: m_sName { std::move (sName) }
		, m_eType { eType }
	{}
};

CSphVector<NamedIndexType_t> GetAllServedIndexes()
{
	CSphVector<NamedIndexType_t> dIndexes;

	// collect local, rt, percolate
	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( const auto& tIt : *hLocal )
	{
		if ( !tIt.second )
			continue;

		switch ( tIt.second->m_eType )
		{
		case IndexType_e::PLAIN:
		case IndexType_e::RT:
		case IndexType_e::PERCOLATE:
		case IndexType_e::TEMPLATE:
			dIndexes.Add ( { tIt.first, tIt.second->m_sCluster, tIt.second->m_eType } );
			break;
		default:
			dIndexes.Add ( { tIt.first, IndexType_e::ERROR_ } );
		}
	}

	// collect distributed
	assert ( g_pDistIndexes );
	auto pDistSnapshot = g_pDistIndexes->GetHash();
	for ( auto& tIt : *pDistSnapshot )
		// no need to check distr's it, iterating guarantees index existance.
		dIndexes.Add ( { tIt.first, IndexType_e::DISTR } );

	dIndexes.Sort ( Lesser ( [] ( const NamedIndexType_t& a, const NamedIndexType_t& b ) { return strcasecmp ( a.m_sName.cstr(), b.m_sName.cstr() ) < 0; } ) );
	return dIndexes;
}

void HandleShowTables ( RowBuffer_i & tOut, const SqlStmt_t * pStmt )
{
	auto dIndexes = GetAllServedIndexes();
	bool bWithClusters = ClusterFlavour();
	auto fnFilter = [bSystem = pStmt->m_iIntParam==1] ( const NamedIndexType_t& tIdx )
	{
		bool bIsSystem = tIdx.m_sName.SubString ( 0, 7 ).EqN ("system.");
		return bSystem == bIsSystem;
	};

	// output the results
	VectorLike dTable ( pStmt->m_sStringParam, { "Table", "Type" } );
	for ( auto& dPair : dIndexes )
	{
		if ( !fnFilter ( dPair ) )
			continue;
		if ( bWithClusters && !dPair.m_sCluster.IsEmpty ())
			dTable.MatchTuplet ( SphSprintf ("%s:%s", dPair.m_sCluster.cstr(), dPair.m_sName.cstr()).cstr(), szIndexType ( dPair.m_eType ) );
		else
			dTable.MatchTuplet( dPair.m_sName.cstr (), szIndexType(dPair.m_eType) );
	}
	tOut.DataTable ( dTable );
}

void HandleShowInformationTables ( RowBuffer_i & tOut, const SqlStmt_t * pStmt )
{
	auto dIndexes = GetAllServedIndexes();
	// output the results
	VectorLike dTable ( pStmt->m_sStringParam, { "table_schema", "table_name" } );
	for ( auto& dPair : dIndexes )
	{
		if ( !dTable.Match ( dPair.m_sName.cstr() ) )
			continue;

		dTable.MatchTuplet( dPair.m_sName.cstr (), szIndexType(dPair.m_eType) );
	}
	tOut.DataTable ( dTable );
}

template <typename T, typename GETNAME>
static bool CheckAttrs ( const VecTraits_T<T> & dAttrs, GETNAME && fnGetName, CSphString & sError )
{
	ARRAY_FOREACH ( i, dAttrs )
	{
		const CSphString & sName = fnGetName(dAttrs[i]);
		if ( sphIsInternalAttr ( sName ) )
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

		sError.SetSprintf ( "table '%s' already exists", tStmt.m_sIndex.cstr() );
		return false;
	}

	if ( CSphSchema::IsReserved ( tStmt.m_sIndex.cstr() ) )
	{
		sError.SetSprintf ( "'%s' is a reserved keyword", tStmt.m_sIndex.cstr() );
		return false;
	}

	return true;
}

static int CheckShardIntOpt ( const char * sName, const SqlStmt_t & tStmt, CSphString & sError )
{
	int iPos = tStmt.m_tCreateTable.m_dOpts.GetFirst ( [&]( const auto & tItem ) { return tItem.m_sName==sName; } );
	if ( iPos==-1 )
		return iPos;

	CSphVariant tVal ( tStmt.m_tCreateTable.m_dOpts[iPos].m_sValue.cstr() );
	if ( tVal.int64val()<0 )
		sError.SetSprintf ( "table '%s': CREATE TABLE failed: negative '%s' option is not allowed", tStmt.m_sIndex.cstr(), sName );

	return iPos;
}

static bool CheckCreateTable ( const SqlStmt_t & tStmt, CSphString & sError )
{
	if ( !CheckExistingTables ( tStmt, sError ) )
		return false;

	if ( !CheckAttrs ( tStmt.m_tCreateTable.m_dAttrs, []( const CreateTableAttr_t & tAttr ) { return tAttr.m_tAttr.m_sName; }, sError ) )
		return false;

	if ( !CheckAttrs ( tStmt.m_tCreateTable.m_dFields, []( const CSphColumnInfo & tAttr ) { return tAttr.m_sName; }, sError ) )
		return false;

	for ( auto & i : tStmt.m_tCreateTable.m_dAttrs )
		if ( i.m_bKNN && !i.m_tKNNModel.m_sModelName.empty() && !IsKNNEmbeddingsLibLoaded() )
		{
			sError.SetSprintf ( "model_name specified for '%s', but embeddings library is not loaded", i.m_tAttr.m_sName.cstr() );
			return false;
		}


	// cross-checks attrs and fields
	for ( const auto & i : tStmt.m_tCreateTable.m_dAttrs )
		for ( const auto & j : tStmt.m_tCreateTable.m_dFields )
			if ( i.m_tAttr.m_sName==j.m_sName && i.m_tAttr.m_eAttrType!=SPH_ATTR_STRING )
			{
				sError.SetSprintf ( "duplicate attribute name '%s'", i.m_tAttr.m_sName.cstr() );
				return false;
			}

	// shard related options
	const CSphVector<NameValueStr_t> & dOpts = tStmt.m_tCreateTable.m_dOpts;
	if ( dOpts.GetLength() )
	{
		int iShardsPos = CheckShardIntOpt ("shards", tStmt, sError );
		if ( !sError.IsEmpty() )
			return false;
		int iRfPos = CheckShardIntOpt ( "rf", tStmt, sError );
		if ( !sError.IsEmpty() )
			return false;

		// should be routerd into buddy with good error message
		if ( iShardsPos!=-1 || iRfPos!=-1 )
		{
			sError.SetSprintf ( "table '%s': CREATE TABLE failed: 'shards' and 'rf' options require Buddy", tStmt.m_sIndex.cstr() );
			return false;
		}
	}

	return true;
}

static Threads::Coro::Mutex_c g_tCreateTableMutex;

static void HandleMysqlCreateTable ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	SearchFailuresLog_c dErrors;
	CSphString sError;

	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	if ( !IsConfigless() )
	{
		sError = "CREATE TABLE requires data_dir to be set in the config file";
		tOut.Error ( sError.cstr() );
		return;
	}

	// only one create table at the time allowed and multiple concurent CREATE TABLE if not exists passes well
	Threads::ScopedCoroMutex_t tCreateTableLock ( g_tCreateTableMutex );

	if ( !CheckCreateTable ( tStmt, sError ) )
	{
		sError.SetSprintf ( "table '%s': CREATE TABLE failed: %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( sError.cstr() );
		return;
	}

	StrVec_t dWarnings;
	bool bCreatedOk = CreateNewIndexConfigless ( tStmt.m_sIndex, tStmt.m_tCreateTable, dWarnings, sError );
	sWarning = ConcatWarnings(dWarnings);

	if ( !bCreatedOk )
	{
		sError.SetSprintf ( "error adding table '%s': %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( sError.cstr() );
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


static CSphString BuildCreateTableRt ( const CSphString & sName, const CSphIndex * pIndex, const CSphSchema & tSchema, ExtFilesFormat_e eExt )
{
	assert(pIndex);

	CSphString sCreateTable = BuildCreateTable ( sName, pIndex, tSchema, eExt );
	return sCreateTable;
}


static void HandleMysqlCreateTableLike ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	SearchFailuresLog_c dErrors;
	CSphString sError;

	if ( !IsConfigless() )
	{
		sError = "CREATE TABLE requires data_dir to be set in the config file";
		tOut.Error ( sError.cstr() );
		return;
	}

	if ( !CheckExistingTables ( tStmt, sError ) )
	{
		sError.SetSprintf ( "table '%s': CREATE TABLE failed: %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( sError.cstr() );
		return;
	}

	const CSphString & sLike = tStmt.m_tCreateTable.m_sLike;
	CSphString sCreateTable;
	switch ( IndexIsServed ( sLike ) )
	{
	case RunIdx_e::NOTSERVED:
		sError.SetSprintf ( "table '%s': CREATE TABLE LIKE failed: no table '%s' found", tStmt.m_sIndex.cstr(), sLike.cstr() );
		tOut.Error ( sError.cstr() );
		return;
	case RunIdx_e::LOCAL:
	{
		auto pServed = GetServed ( sLike );
		assert ( pServed );
		if ( !ServedDesc_t::IsMutable ( pServed ) )
		{
			tOut.ErrorAbsent ( "table '%s' is not real-time or percolate", sError.cstr() );
			return;
		}
		RIdx_c pIdx { pServed };
		sCreateTable = BuildCreateTableRt ( tStmt.m_sIndex, pIdx, GetSchemaForCreateTable ( pIdx ), ExtFilesFormat_e::FILE );
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
	if ( !ParseDdl ( FromStr ( sCreateTable ), dCreateTableStmts, sError ) )
	{
		tOut.Error ( sError.cstr() );
		return;
	}

	if ( dCreateTableStmts.GetLength()!=1 )
	{
		tOut.Error ( "CREATE TABLE LIKE failed" );
		return;
	}

	SqlStmt_t & tNewCreateTable = dCreateTableStmts[0];
	tNewCreateTable.m_tCreateTable.m_bIfNotExists = tStmt.m_tCreateTable.m_bIfNotExists;

	HandleMysqlCreateTable ( tOut, tNewCreateTable, sWarning );
}


static void HandleMysqlDropTable ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	CSphString sError;

	if ( !IsConfigless() )
	{
		sError = "DROP TABLE requires data_dir to be set in the config file";
		tOut.Error ( sError.cstr() );
		return;
	}

	bool bDropped = DropIndexInt ( tStmt.m_sIndex.cstr(), tStmt.m_bIfExists, tStmt.m_bForce, sError, &sWarning );
	sphLogDebug ( "dropped table %s, ok %d, error %s", tStmt.m_sIndex.cstr(), (int)bDropped, sError.scstr() ); // FIXME!!! remove
	if ( !bDropped )
		tOut.Error ( sError.cstr() );
	else
		tOut.Ok ( 0, ( sWarning.IsEmpty() ? 0 : 1 ) );
}


static void ClearSecondaryIndexCaches()
{
	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( const auto & i : *hLocal )
	{
		if ( !i.second )
			continue;
		
		RIdx_c pIdx { i.second };
		const SIContainer_c * pSI = pIdx->GetSI();
		if ( pSI && !pSI->IsEmpty() )
			const_cast<SIContainer_c*>(pSI)->ClearCache(); // need non-const access to clear caches
	}
}


static void HandleMysqlDropCache ( RowBuffer_i & tOut )
{
	QcacheClearAll();
	ClearDocstoreCache();
	SkipCache::ClearAll();
	ClearSecondaryIndexCaches();
	
	tOut.Ok ( 0, 0 );
}


void HandleMysqlShowCreateTable ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	auto pServed = GetServed ( tStmt.m_sIndex );
	auto pDist = GetDistr ( tStmt.m_sIndex );
	if ( !pServed && !pDist )
	{
		tOut.ErrorAbsent ( "no such table '%s'", tStmt.m_sIndex.cstr () );
		return;
	}

	if ( pServed && !ServedDesc_t::IsMutable ( pServed ) )
	{
		tOut.ErrorAbsent ( "table '%s' is not real-time or percolate", tStmt.m_sIndex.cstr () );
		return;
	}

	if ( pDist && IsDistrTableHasSystem ( *pDist, tStmt.m_bForce ) )
	{
		tOut.ErrorAbsent ( "can not show table '%s' because it contains system table", tStmt.m_sIndex.cstr() );
		return;
	}

	// result set header packet
	tOut.HeadTuplet ( "Table", "Create Table" );
	CSphString sCreateTable;
	if ( pServed )
	{
		RIdx_c pIdx { pServed };
		sCreateTable = BuildCreateTableRt ( pIdx->GetName(), pIdx, GetSchemaForCreateTable ( pIdx ), ( tStmt.m_bFormatOutWordsFile ? ExtFilesFormat_e::FILE : ExtFilesFormat_e::LIST ) );
	} else
		sCreateTable = BuildCreateTableDistr ( tStmt.m_sIndex, *pDist );

	tOut.DataTuplet ( tStmt.m_sIndex.cstr(), sCreateTable.cstr() );
	tOut.Eof();
}


// MySQL Workbench (and maybe other clients) crashes without it
void HandleMysqlShowDatabases ( RowBuffer_i & tOut, SqlStmt_t & )
{
	tOut.HeadBegin ();
	tOut.HeadColumn ( "Database" );
	tOut.HeadEnd();
	tOut.PutString ( "information_schema" );
	tOut.Commit ();
	tOut.PutString ( "Manticore" );
	tOut.Commit ();
	tOut.Eof();
}


void HandleMysqlShowPlugins ( RowBuffer_i & tOut, SqlStmt_t & )
{
	CSphVector<PluginInfo_t> dPlugins;
	sphPluginList ( dPlugins );

	tOut.HeadBegin ();
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
		if ( !tOut.Commit() )
			return;
	}
	tOut.Eof();
}

enum ThreadInfoFormat_e
{
	THD_FORMAT_NATIVE,
	THD_FORMAT_SPHINXQL
};

static Str_t FormatInfo ( const PublicThreadDesc_t & tThd, ThreadInfoFormat_e eFmt, QuotationEscapedBuilder & tBuf )
{
	if ( tThd.m_pQuery && eFmt==THD_FORMAT_SPHINXQL && tThd.m_eProto!=Proto_e::MYSQL41 )
	{
		bool bGotQuery = false;
		if ( tThd.m_pQuery )
		{
			tBuf.Clear();
			FormatSphinxql ( *tThd.m_pQuery, {}, 0, tBuf );
			bGotQuery = true;
		}

		// query might be removed prior to lock then go to common path
		if ( bGotQuery )
			return (Str_t)tBuf;
	}

	if ( tThd.m_sDescription.IsEmpty () && tThd.m_szCommand )
		return FromSz ( tThd.m_szCommand );
	return (Str_t)tThd.m_sDescription;
}

void HandleShowThreads ( RowBuffer_i & tOut, const SqlStmt_t * pStmt )
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

	tOut.HeadBegin ();
	tOut.HeadColumn ( "TID", MYSQL_COL_LONG );
	tOut.HeadColumn ( "Name" );
	tOut.HeadColumn ( "Proto" );
	tOut.HeadColumn ( "State" );
	tOut.HeadColumn ( "Connection from" );
	tOut.HeadColumn ( "ConnID", MYSQL_COL_LONGLONG );
//	tOut.HeadColumn ( "Time", MYSQL_COL_FLOAT );
	tOut.HeadColumn ( "This/prev job time" );
	if ( g_bCpuStats )
	{
//		tOut.HeadColumn ( "Work time CPU" );
		tOut.HeadColumn ( "CPU activity" );
	}
	tOut.HeadColumn ( "Jobs done", MYSQL_COL_LONG );
	tOut.HeadColumn ( "Thread status" );
	if ( bAll )
		tOut.HeadColumn ( "Chain" );
	tOut.HeadColumn ( "Info" );
	if (!tOut.HeadEnd())
		return;

	QuotationEscapedBuilder tBuf;

//	sphLogDebug ( "^^ Show threads. Current info is %p", GetTaskInfo () );

	CSphSwapVector<PublicThreadDesc_t> dFinal;
	IterateActive([&dFinal, iCols] ( LowThreadDesc_t * pThread ){
		if ( pThread )
			dFinal.Add ( GatherPublicThreadInfo ( pThread, iCols ) );
	});

	for ( const auto & dThd : dFinal )
	{
		if ( !bAll && dThd.m_eTaskState==TaskState_e::UNKNOWN )
			continue;
		tOut.PutNumAsString ( dThd.m_iThreadID ); // TID
		tOut.PutString ( dThd.m_sThreadName ); // Name
		tOut.PutString ( dThd.m_sProto ); // Proto
		tOut.PutString ( TaskStateName ( dThd.m_eTaskState ) ); // State
		tOut.PutString ( dThd.m_sClientName ); // Connection from
		tOut.PutNumAsString ( dThd.m_iConnID ); // ConnID
		int64_t tmNow = sphMicroTimer (); // short-term cache
//		tOut.PutMicrosec ( tmNow-dThd.m_tmStart.value_or(tmNow) ); // time
//		tOut.PutTimeAsString ( dThd.m_tmTotalWorkedTimeUS ); // work time
		// This/prev job time
		if ( dThd.m_tmLastJobStartTimeUS < 0 )
			tOut.PutString ( "-" ); // last job take
		else if ( dThd.m_tmLastJobDoneTimeUS < 0 )
			tOut.PutTimeAsString ( tmNow - dThd.m_tmLastJobStartTimeUS );
		else
			tOut.PutTimeAsString ( dThd.m_tmLastJobDoneTimeUS - dThd.m_tmLastJobStartTimeUS, " (prev)" );


		if ( g_bCpuStats )
		{
//			tOut.PutTimeAsString ( dThd.m_tmTotalWorkedCPUTimeUS ); // work CPU time
			tOut.PutPercentAsString ( dThd.m_tmTotalWorkedCPUTimeUS, dThd.m_tmTotalWorkedTimeUS ); // CPU activity as integer
		}
		tOut.PutNumAsString ( dThd.m_iTotalJobsDone ); // jobs done
		if ( dThd.m_tmLastJobStartTimeUS<0 )
		{
			tOut.PutString ( "idling" ); // idle for
		} else if ( dThd.m_tmLastJobDoneTimeUS<0 )
		{
			tOut.PutString ( "working" ); // idle for
		} else
		{
			tOut.PutString ( "idling" ); // notice, just 'idling' instead of 'idling for N seconds'. So, value of dThd.m_tmLastJobDoneTimeUS is never more displayed.
		}

		if ( bAll )
			tOut.PutString ( dThd.m_sChain ); // Chain
		auto sInfo = FormatInfo ( dThd, eFmt, tBuf );
		if ( iCols >= 0 && iCols < sInfo.second )
			sInfo.second = iCols;
		tOut.PutString ( sInfo ); // Info m_pTaskInfo
		if ( !tOut.Commit () )
			break;
	}

	tOut.Eof();
}

// helper; available only via 'select ... from @@system.sessions...'
void HandleShowSessions ( RowBuffer_i& tOut, const SqlStmt_t* pStmt )
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

	tOut.HeadBegin ();
	tOut.HeadColumn ( "Proto" );
	tOut.HeadColumn ( "State" );
	tOut.HeadColumn ( "Host" );
	tOut.HeadColumn ( "ConnID", MYSQL_COL_LONGLONG );
	tOut.HeadColumn ( "Killed" );
	if ( bAll )
		tOut.HeadColumn ( "Chain" );
	tOut.HeadColumn ( "Last cmd time" );
	tOut.HeadColumn ( "Last cmd" );
	if ( !tOut.HeadEnd() )
		return;

	QuotationEscapedBuilder tBuf;

	//	sphLogDebug ( "^^ Show threads. Current info is %p", GetTaskInfo () );

	CSphSwapVector<PublicThreadDesc_t> dFinal;
	IterateTasks ( [&dFinal, iCols] ( ClientTaskInfo_t* pTask ) {
		if ( pTask )
		{
			PublicThreadDesc_t& tDesc = dFinal.Add();
			tDesc.m_iDescriptionLimit = iCols;
			GatherPublicTaskInfo ( tDesc, pTask );
		}
	} );

	for ( const auto& dThd : dFinal )
	{
		if ( !bAll && dThd.m_eTaskState == TaskState_e::UNKNOWN )
			continue;
		tOut.PutString ( dThd.m_sProto );
		tOut.PutString ( TaskStateName ( dThd.m_eTaskState ) );
		tOut.PutString ( dThd.m_sClientName );												   // Host
		tOut.PutNumAsString ( dThd.m_iConnID );												   // ConnID
		tOut.PutNumAsString ( dThd.m_bKilled ? 1 : 0);
		if ( bAll )
			tOut.PutString ( dThd.m_sChain ); // Chain
		if ( dThd.m_tmLastJobDoneTimeUS==-1 ) // not yet finished
			tOut.PutTimestampAsString ( dThd.m_tmLastJobStartTimeUS );
		else
			tOut.PutTimeAsString ( dThd.m_tmLastJobDoneTimeUS - dThd.m_tmLastJobStartTimeUS );
		auto sInfo = FormatInfo ( dThd, eFmt, tBuf );
		if ( iCols >= 0 && iCols < sInfo.second )
			sInfo.second = iCols;
		tOut.PutString ( sInfo ); // Info m_pTaskInfo
		if ( !tOut.Commit() )
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
			pCur += ZipToPtrLE ( pCur, dValue - iLast );
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
		sError.SetSprintf ( "unknown table '%s' in Set statement", sIndex );
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
		auto iDelta = UnzipValueLE<int64_t> ( [&pCur]() mutable { return *pCur++; } );
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
	, m_pWarning ( nullptr )
{}

SphinxqlReplyParser_c::SphinxqlReplyParser_c ( int * pUpd, int * pWarns, CSphString * pWarning )
	: m_pUpdated ( pUpd )
	, m_pWarns ( pWarns )
	, m_pWarning ( pWarning )
{}

// fixme! reuse code from sphinxql, leave only refs here
bool SphinxqlReplyParser_c::ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const
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
		if ( uSize > 0 )
		{
			// extract info string (contains warning message if warnings present)
			// info string is length-encoded in MySQL protocol
			int iInfoLen = MysqlUnpack ( tReq, &uSize );
			if ( iInfoLen > 0 && iInfoLen <= (int)uSize )
			{
				CSphString sInfo = tReq.GetRawString ( iInfoLen );
				if ( *m_pWarns>0 && m_pWarning && !sInfo.IsEmpty() )
				{
					if ( !m_pWarning->IsEmpty() )
						m_pWarning->SetSprintf ( "%s; %s", m_pWarning->cstr(), sInfo.cstr() );
					else
						*m_pWarning = sInfo;
				}
			}
		}
		return true;
	}
	if ( uCommand==0xff ) // error packet
	{
		tReq.GetByte();
		tReq.GetByte(); ///< num of errors (2 bytes), we don't use it for now.
		uSize -= 2;
		if ( uSize )
		{
			// MySQL error packet format: 6-byte SQLSTATE prefix (e.g., "#42000") followed by error message
			// Skip the SQLSTATE prefix if present
			if ( uSize > 6 )
			{
				tReq.GetRawString ( 6 ); // skip SQLSTATE
				uSize -= 6;
			}
			tAgent.m_sFailure = tReq.GetRawString ( uSize );
		}
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

static void DoExtendedUpdate ( const SqlStmt_t & tStmt, const CSphString & sIndex, const char * szDistributed, bool bBlobUpdate, int & iSuccesses, int & iUpdated, SearchFailuresLog_c & dFails, CSphString & sWarning, const cServedIndexRefPtr_c & pServed )
{
	TRACE_CORO ( "rt", "DoExtendedUpdate" );

	TlsMsg::ResetErr();
	// checks
	if ( !pServed )
	{
		dFails.Submit ( sIndex, szDistributed, "table not available" );
		return;
	}

	if ( !ValidateClusterStatement ( sIndex, *pServed, tStmt.m_sCluster, IsHttpStmt ( tStmt ) ) )
	{
		dFails.Submit ( sIndex, szDistributed, TlsMsg::szError() );
		return;
	}

	Threads::Coro::ScopedWriteTable_c tWriting { pServed->Locker() };
	if ( !tWriting.CanWrite() )
	{
		dFails.Submit ( sIndex, szDistributed, "table is locked" );
		return;
	}

	RtAccum_t tAcc;
	ReplicationCommand_t * pCmd = tAcc.AddCommand ( tStmt.m_bJson ? ReplCmd_e::UPDATE_JSON : ReplCmd_e::UPDATE_QL, sIndex, tStmt.m_sCluster );
	assert ( pCmd );
	pCmd->m_pUpdateAPI = tStmt.AttrUpdatePtr();
	pCmd->m_bBlobUpdate = bBlobUpdate;
	pCmd->m_pUpdateCond = &tStmt.m_tQuery;

	HandleCmdReplicateUpdate ( tAcc, sWarning, iUpdated );

	if ( TlsMsg::HasErr() )
	{
		dFails.Submit ( sIndex, szDistributed, TlsMsg::szError() );
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

static void DoUpdate( DocsCollector_c& tCollector, AttrUpdateArgs& tArgs, CSphIndex * pIndex )
{
	TRACE_CORO ( "rt", "DoUpdate" );
	AttrUpdateSharedPtr_t& pUpdate = tArgs.m_pUpdate;
	while ( tCollector.GetValuesChunk ( pUpdate->m_dDocids, tArgs.m_pQuery->m_iMaxMatches ) )
	{
		int iChanged = 0;
		Verify ( HandleUpdateAPI ( tArgs, pIndex, iChanged ) ); // fixme! handle this
		tArgs.m_iAffected += iChanged;
	}
}

void UpdateWlocked ( AttrUpdateArgs& tArgs, const cServedIndexRefPtr_c& pDesc, int& iUpdated )
{
	TRACE_CORO ( "sph", "UpdateWlocked" );
	// short-living r-lock m.b. acquired and released by collector when running query
	auto tCollector = InitUpdate ( tArgs, pDesc );
	BEGIN_CORO ( "wait", "take w-lock" );
	WIdx_c wLocked { pDesc }; // exclusive lock for process of update. Note, between collecting and updating m.b. race! To eliminate it, need to trace index generation and recollect if it changed.
	END_CORO ( "wait" );
	DoUpdate ( tCollector, tArgs, wLocked );
}

void UpdateRlocked ( AttrUpdateArgs& tArgs, const cServedIndexRefPtr_c& pDesc, int& iUpdated)
{
	TRACE_CORO ( "sph", "UpdateRlocked" );
	// wide r-lock over whole update. r-locks acquired by collector should be re-enterable.
	BEGIN_CORO ( "wait", "take r-lock" );
	RWIdx_c rLocked { pDesc };
	END_CORO ( "wait" );
	auto tCollector = InitUpdate ( tArgs, pDesc );
	DoUpdate ( tCollector, tArgs, rLocked );
}
} // unnamed namespace

void HandleMySqlExtendedUpdate ( AttrUpdateArgs& tArgs, const cServedIndexRefPtr_c& pDesc, int& iUpdated, bool bNeedWlock )
{
	TRACE_CORO ( "sph", "HandleMySqlExtendedUpdate" );
	return bNeedWlock ? UpdateWlocked ( tArgs, pDesc, iUpdated ) : UpdateRlocked ( tArgs, pDesc, iUpdated );
}

void sphHandleMysqlUpdate ( StmtErrorReporter_i & tOut, const SqlStmt_t & tStmt, Str_t sQuery )
{
	TRACE_CORO ( "sph", "sphHandleMysqlUpdate" );
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
		tOut.Error ( "no such table '%s'", tStmt.m_sIndex.cstr() );
		return;
	}

	DistrPtrs_t dDistributed;
	// copy distributed indexes description
	CSphString sMissed;
	if ( !ExtractDistributedIndexes ( dIndexNames, dDistributed, sMissed ) )
	{
		tOut.Error ( "unknown table '%s' in update request", sMissed.cstr() );
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
			int64_t tmStartIdx = sphMicroTimer();
			int iUpdatedIdx = 0;

			DoExtendedUpdate ( tStmt, sReqIndex, nullptr, bBlobUpdate, iSuccesses, iUpdatedIdx, dFails, sWarning, pLocked );

			iUpdated += iUpdatedIdx;
			pLocked->m_pStats->AddWriteStat ( SearchdStats_t::eUpdate, false, iUpdatedIdx, tmStartIdx );

		} else if ( dDistributed[iIdx] )
		{
			assert ( !dDistributed[iIdx]->IsEmpty() );
			const StrVec_t & dLocal = dDistributed[iIdx]->m_dLocal;

			ARRAY_FOREACH ( i, dLocal )
			{
				const char * sLocal = dLocal[i].cstr();
				int64_t tmStartIdx = sphMicroTimer();
				int iUpdatedIdx = 0;

				auto pServed = GetServed ( sLocal );
				DoExtendedUpdate ( tStmt, sLocal, sReqIndex, bBlobUpdate, iSuccesses, iUpdatedIdx, dFails, sWarning, pServed );

				iUpdated += iUpdatedIdx;
				if ( pServed )
					pServed->m_pStats->AddWriteStat ( SearchdStats_t::eUpdate, false, iUpdatedIdx, tmStartIdx );
			}
		}

		// update remote agents
		if ( dDistributed[iIdx] && !dDistributed[iIdx]->m_dAgents.IsEmpty() )
		{
			const DistributedIndex_t * pDist = dDistributed[iIdx];

			VecRefPtrs_t<AgentConn_t *> dAgents;
			pDist->GetAllHosts ( dAgents );

			// connect to remote agents and query them
			// validation happens on remote side; errors will be returned via dFails
			if ( !dAgents.IsEmpty() )
			{
				std::unique_ptr<RequestBuilder_i> pRequestBuilder = CreateRequestBuilder ( sQuery, tStmt );
				std::unique_ptr<ReplyParser_i> pReplyParser = CreateReplyParser ( tStmt.m_bJson, iUpdated, iWarns, dFails, &sWarning );
				iSuccesses += PerformRemoteTasks ( dAgents, pRequestBuilder.get (), pReplyParser.get () );

				// collect errors from failed agents
				for ( const AgentConn_t * pAgent : dAgents )
					if ( !pAgent->m_bSuccess && !pAgent->m_sFailure.IsEmpty() )
					{
						// remote error may already include "table X: " prefix, strip it to avoid duplication
						const char * sError = pAgent->m_sFailure.cstr();
						const char * sPrefix = "table ";
						if ( pAgent->m_sFailure.Begins ( sPrefix ) )
						{
							const char * pAfterPrefix = sError + strlen ( sPrefix );
							const char * pColon = strchr ( pAfterPrefix, ':' );
							if ( pColon && ( pColon[1] == ' ' || pColon[1] == '\0' ) )
								sError = pColon[1] == ' ' ? ( pColon + 2 ) : ( pColon + 1 );
						}
						dFails.Submit ( pAgent->m_tDesc.m_sIndexes, sReqIndex, sError );
					}
			}
		}
	}

	StringBuilder_c sReport;
	dFails.BuildReport ( sReport );

	gStats().AddDetailed ( SearchdStats_t::eUpdate, iUpdated, tmStart );
	if ( !iSuccesses )
	{
		tOut.Error ( "%s", sReport.cstr() );
		return;
	}

	int64_t tmRealTimeMs = ( sphMicroTimer() - tmStart ) / 1000;
	if ( !g_iQueryLogMinMs || tmRealTimeMs>g_iQueryLogMinMs )
		LogSphinxqlClause ( sQuery, (int)( tmRealTimeMs ) );

	if ( sWarning.IsEmpty() )
		tOut.Ok ( iUpdated, iWarns );
	else
		tOut.Ok ( iUpdated, sWarning, 0 );
}

bool HandleMysqlSelect ( RowBuffer_i & dRows, SearchHandler_c & tHandler )
{
	// lets check all query for errors
	StringBuilder_c sError { "; " };
	CSphVector<int64_t> dAgentTimes; // dummy for error reporting
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
	{
		CheckQuery ( tHandler.m_dQueries[i], tHandler.m_dAggrResults[i].m_sError, tHandler.m_dQueries.GetLength() == 1 );
		if ( !tHandler.m_dAggrResults[i].m_sError.IsEmpty() )
		{
			LogQuery ( tHandler.m_dQueries[i], tHandler.m_dJoinQueryOptions[i], tHandler.m_dAggrResults[i], dAgentTimes );
			if ( tHandler.m_dQueries.GetLength()==1 )
				sError << tHandler.m_dAggrResults[0].m_sError;
			else
				sError.Sprintf( "query %d error: %s", i, tHandler.m_dAggrResults[i].m_sError.cstr() );
		}
	}

	if ( !sError.IsEmpty() )
	{
		// stmt is intentionally NULL, as we did all the reporting just above
		dRows.Error ( sError.cstr() );
		return false;
	}

	// actual searching
	tHandler.RunQueries();

	if ( sphInterrupted() )
	{
		sphLogDebug ( "HandleClientMySQL: got SIGTERM, sending the packet MYSQL_ERR_SERVER_SHUTDOWN" );
		dRows.Error ( "Server shutdown in progress", EMYSQL_ERR::SERVER_SHUTDOWN );
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
			ISphExprRefPtr_c pExpr { sphExprParse ( tCol.m_sName.cstr(), tSchema, nullptr, sError, tExprArgs )};

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

CSphString BuildMetaOneline ( const CSphQueryResultMeta & tMeta )
{
	// --- 0 out of 1115 results in 115ms ---
	// --- 20 out of >= 20 results in 5.123s ---

	StringBuilder_c sMeta;
	// since we have us precision, printing 0 will output '0us', which is not necessary true.
	if ( tMeta.m_iQueryTime > 0 )
		sMeta.Sprintf ( "--- %d out of %s%l results in %.3t ---", tMeta.m_iMatches, ( tMeta.m_bTotalMatchesApprox ? ">=" : "" ), tMeta.m_iTotalMatches, (int64_t)tMeta.m_iQueryTime * 1000 );
	else
		sMeta.Sprintf ( "--- %d out of %s%l results in 0ms ---", tMeta.m_iMatches, ( tMeta.m_bTotalMatchesApprox ? ">=" : "" ), tMeta.m_iTotalMatches );
	return (CSphString)sMeta;
}


static bool IsNullSet ( const CSphMatch	& tMatch, int iAttr, SphAttr_t tNullMask, const CSphColumnInfo * pNullBitmaskAttr )
{
	if ( !pNullBitmaskAttr )
		return false;

	if ( pNullBitmaskAttr->m_eAttrType==SPH_ATTR_STRINGPTR )
	{
		ByteBlob_t tBlob = sphUnpackPtrAttr ( (const BYTE*)tNullMask );
		BitVec_T<const BYTE> tVec ( tBlob.first, tBlob.second*8 );
		return tVec.BitGet(iAttr);
	}

	assert ( iAttr < 64 );
	return !!( tNullMask & ( 1ULL << iAttr ) );
}


static void SendMysqlMatch ( const CSphMatch & tMatch, const CSphBitvec & tAttrsToSend, const ISphSchema & tSchema, RowBuffer_i & dRows, const CSphColumnInfo * pNullBitmaskAttr )
{
	SphAttr_t tNullMask = pNullBitmaskAttr ? tMatch.GetAttr ( pNullBitmaskAttr->m_tLocator ) : 0;
	const int iAttrs = tSchema.GetAttrsCount();
	const BYTE* pBitmask = nullptr;
	if ( pNullBitmaskAttr )
	{
		if ( pNullBitmaskAttr->m_eAttrType==SPH_ATTR_STRINGPTR )
		{
			ByteBlob_t tBlob = sphUnpackPtrAttr ( (const BYTE*)tNullMask );
			assert ( iAttrs <= tBlob.second*8 );
			pBitmask = tBlob.first;
		} else
		{
			pBitmask = (const BYTE*) &tNullMask;
			assert ( iAttrs < 64 );
		}
	}
	dRows.DataStart ( pBitmask );

	for ( int i=0; i < iAttrs; i++ )
	{
		if ( !tAttrsToSend.BitGet(i) )
			continue;

		if ( IsNullSet ( tMatch, i, tNullMask, pNullBitmaskAttr ) )
		{
			dRows.SkipNULL();
			continue;
		}

		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		const CSphAttrLocator & tLoc = tAttr.m_tLocator;
		ESphAttr eAttrType = tAttr.m_eAttrType;

		assert ( sphPlainAttrToPtrAttr(eAttrType)==eAttrType );

		switch ( eAttrType )
		{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TIMESTAMP:
		case SPH_ATTR_BOOL:
		case SPH_ATTR_TOKENCOUNT:
			dRows.PutDWORD ( ( DWORD ) tMatch.GetAttr ( tLoc ) );
			break;

		case SPH_ATTR_BIGINT:
			dRows.PutInt64 ( tMatch.GetAttr(tLoc) );
			break;

		case SPH_ATTR_UINT64:
			dRows.PutUint64 ( (uint64_t)tMatch.GetAttr(tLoc) );
			break;

		case SPH_ATTR_FLOAT:
			dRows.PutFloat ( tMatch.GetAttrFloat(tLoc) );
			break;

		case SPH_ATTR_DOUBLE:
			dRows.PutDouble ( tMatch.GetAttrDouble(tLoc) );
			break;

		case SPH_ATTR_INT64SET_PTR:
		case SPH_ATTR_UINT32SET_PTR:
		{
			StringBuilder_c dStr;
			sphPackedMVA2Str ( (const BYTE *)tMatch.GetAttr(tLoc), eAttrType==SPH_ATTR_INT64SET_PTR, dStr );
			dRows.PutArray ( dStr, false );
		}
		break;

		case SPH_ATTR_FLOAT_VECTOR_PTR:
		{
			StringBuilder_c dStr;
			sphPackedFloatVec2Str ( (const BYTE *)tMatch.GetAttr(tLoc), dStr );
			dRows.PutArray ( dStr, false );
		}
		break;

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
			if ( !IsEmpty ( dFactors ))
				sphFormatFactors ( sTmp, (const unsigned int *)dFactors.first, eAttrType==SPH_ATTR_FACTORS_JSON );
			dRows.PutArray ( sTmp, false );
		}
		break;

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
		}
		break;

		default:
			dRows.Add(1);
			dRows.Add('-');
			break;
		}
	}
}

// returns N of matches in resultset
uint64_t SendMysqlSelectResult ( RowBuffer_i & dRows, const AggrResult_t & tRes, bool bMoreResultsFollow, bool bAddQueryColumn, const CSphString * pQueryColumn, QueryProfile_c * pProfile )
{
	CSphScopedProfile tProf ( pProfile, SPH_QSTATE_NET_WRITE );

	if ( !tRes.m_iSuccesses )
	{
		if ( !tRes.m_sError.IsEmpty() )
		{
			// at this point, SELECT error logging should have been handled, so pass a NULL stmt to logger
			dRows.Error ( tRes.m_sError.cstr() );
			return 0;
		}
		assert ( tRes.m_sError.IsEmpty() );
		auto iWarns = tRes.m_sWarning.IsEmpty() ? 0 : 1;
		CSphString sMeta = BuildMetaOneline ( tRes );

		dRows.HeadBegin();
		dRows.HeadColumn ( "" );
		dRows.HeadEnd();
		dRows.Eof ( bMoreResultsFollow, iWarns, sMeta.cstr() );
		return 0;
	}

	// empty result sets just might carry the full uberschema
	// bummer! lets protect ourselves against that
	CSphBitvec tAttrsToSend;
	bool bReturnZeroCount = !tRes.m_dZeroCount.IsEmpty();
	assert ( bReturnZeroCount || tRes.m_tSchema.GetAttrsCount() );
	sphGetAttrsToSend ( tRes.m_tSchema, false, true, tAttrsToSend );

	dRows.HeadBegin ();
	for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); ++i )
	{
		if ( !tAttrsToSend.BitGet(i) )
			continue;

		const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
		dRows.HeadColumnRaw ( tCol.m_sName.cstr(), tCol.m_eAttrType );
	}

	if ( bAddQueryColumn )
		dRows.HeadColumn ( "query" );

	// EOF packet is sent explicitly due to non-default params.
	auto iWarns = tRes.m_sWarning.IsEmpty() ? 0 : 1;
	dRows.HeadEnd ( bMoreResultsFollow, iWarns );

	// FIXME!!! replace that vector relocations by SqlRowBuffer

	const CSphColumnInfo * pNullBitmaskAttr = tRes.m_tSchema.GetAttr ( GetNullMaskAttrName() );

	assert ( tRes.m_bSingle );
	auto dMatches = tRes.m_dResults.First ().m_dMatches.Slice ( tRes.m_iOffset, tRes.m_iCount );
	uint64_t uMatches = tRes.m_dResults.First ().m_dMatches.GetLength();
	for ( const auto & tMatch : dMatches )
	{
		SendMysqlMatch ( tMatch, tAttrsToSend, tRes.m_tSchema, dRows, pNullBitmaskAttr );

		if ( bAddQueryColumn )
		{
			assert ( pQueryColumn );
			dRows.PutString ( *pQueryColumn );
		}

		if ( !dRows.Commit() )
			return uMatches;
	}

	if ( bReturnZeroCount )
		ReturnZeroCount ( tRes.m_tSchema, tAttrsToSend, tRes.m_dZeroCount, dRows );

	CSphString sMeta = BuildMetaOneline ( tRes );

	// eof packet
	dRows.Eof ( bMoreResultsFollow, iWarns, sMeta.cstr() );
	return uMatches;
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
	dRows.HeadBegin ();
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


void HandleMysqlShowScroll ( RowBuffer_i & dRows, const SqlStmt_t & tStmt, const CSphQueryResultMeta & tLastMeta, bool bMoreResultsFollow )
{
	assert ( tStmt.m_eStmt==STMT_SHOW_SCROLL );

	if ( !dRows.HeadOfStrings ( { "scroll_token" } ) )
		return;

	if ( !tLastMeta.m_sScroll.IsEmpty() )
	{
		dRows.PutString ( tLastMeta.m_sScroll.cstr() );
		dRows.Commit();
	}

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

	auto pCmd = MakeReplicationCommand ( ReplCmd_e::PQUERY_DELETE, std::move ( sIndex ), std::move ( sCluster ) );
	if ( ( pFilter->m_bHasEqualMin || pFilter->m_bHasEqualMax ) && !pFilter->m_bExclude && pFilter->m_eType==SPH_FILTER_VALUES && ( pFilter->m_sAttrName=="@id" || pFilter->m_sAttrName=="id" || pFilter->m_sAttrName=="uid" ) )
	{
		pCmd->m_dDeleteQueries.Append ( pFilter->GetValues() );
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
		return err ( "table not available, or does not support DELETE" );

	pServed->m_pStats->IncCmd ( SEARCHD_COMMAND_DELETE );

	GlobalCrashQueryGetRef().m_dIndex = FromStr ( sName );
	if ( !ValidateClusterStatement ( sName, *pServed, sCluster, IsHttpStmt ( tStmt ) ) )
		return err ( TlsMsg::szError() );

	// process store to local variable instead of deletion (here we don't need any stuff like accum, txn, replication)
	if ( bOnlyStoreDocIDs )
	{
		if ( pServed->m_eType == IndexType_e::PERCOLATE )
			return err ( "Storing del subset not implemented for PQ tables" );

		assert ( sStore.Begins ( "@" ) );
		DocsCollector_c dData { tStmt.m_tQuery, tStmt.m_bJson, sName, pServed, &sError };
		auto dDocs = dData.GetValuesSlice();
		if ( !sError.IsEmpty() )
			return err();

		SetLocalTemporaryUserVar ( sStore, dDocs );
		return 0;
	}

	Threads::Coro::ScopedWriteTable_c tWriting { pServed->Locker() };
	if ( !tWriting.CanWrite() )
		return err ( "table is locked" );

	RtAccum_t* pAccum = nullptr;

	// goto to percolate path with unlocked index
	if ( pServed->m_eType==IndexType_e::PERCOLATE )
	{
		auto pCmd = MakePercolateDeleteDocumentsCommand ( sName, sCluster, tStmt, sError );
		if ( !sError.IsEmpty() )
			return err();

		if ( !pCmd )
			return 0;

		RIdx_T<RtIndex_i*> pRtIndex { pServed };
		pAccum = tAcc.GetAcc ( pRtIndex, sError );
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
		if ( !sError.IsEmpty() )
			return err();

		if ( !pRtIndex->DeleteDocument ( dDocs, sError, pAccum ) ) // assume dData is alive, as we use slice from internal vec
			return err();

		assert ( pAccum );
		pAccum->AddCommand ( ReplCmd_e::RT_TRX, sName, sCluster );
	}

	int iAffected = 0;
	if ( bCommit )
	{
		if ( !HandleCmdReplicateDelete ( *pAccum, iAffected ) )
		{
			dErrors.Submit ( sName, sDistributed, TlsMsg::szError() );
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
		tOut.Error ( "no such table '%s'", tStmt.m_sIndex.cstr () );
		return;
	}

	DistrPtrs_t dDistributed;
	CSphString sMissed;
	if ( !ExtractDistributedIndexes ( dNames, dDistributed, sMissed ) )
	{
		tOut.Error ( "unknown table '%s' in delete request", sMissed.cstr () );
		return;
	}

	// delete to agents works only with commit=1
	if ( !bCommit  )
	{
		for ( auto &pDist : dDistributed )
		{
			if ( !pDist || pDist->m_dAgents.IsEmpty() )
				continue;

			tOut.Error ( "table '%s': DELETE is not supported on agents when autocommit=0", tStmt.m_sIndex.cstr() );
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
			std::unique_ptr<ReplyParser_i> pReplyParser = CreateReplyParser ( tStmt.m_bJson, iGot, iWarns, dErrors, nullptr );
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

// fwd
void HandleMysqlShowProfile ( RowBuffer_i & tOut, const QueryProfile_c & p, bool bMoreResultsFollow );

static void HandleMysqlShowPlan ( RowBuffer_i & tOut, const QueryProfile_c & p, bool bMoreResultsFollow, bool bDot );

bool IsDot ( const SqlStmt_t & tStmt )
{
	if ( tStmt.m_sThreadFormat=="dot" )
		return true;
	if ( tStmt.m_sThreadFormat=="plain" )
		return false;
	return session::IsDot();
}

Profile_e ParseProfileFormat ( const SqlStmt_t & tStmt )
{
	if ( tStmt.m_sSetValue=="dot" )
		return DOT;
	if ( tStmt.m_sSetValue=="expr" )
		return DOTEXPR;
	if ( tStmt.m_sSetValue=="exprurl" )
		return DOTEXPRURL;
	if ( tStmt.m_iSetValue!=0 )
		return PLAIN;
	return NONE;
}

void HandleMysqlMultiStmt ( const CSphVector<SqlStmt_t> & dStmt, CSphQueryResultMeta & tLastMeta, RowBuffer_i & dRows,
		const CSphString & sWarning )
{
	auto& tSess = session::Info();

	// select count
	int iSelect = dStmt.count_of ( [] ( const auto& tStmt ) { return tStmt.m_eStmt == STMT_SELECT; } );

	CSphQueryResultMeta tPrevMeta = tLastMeta;

	myinfo::SetCommand ( SqlStmt2Str(STMT_SELECT) );
	AT_SCOPE_EXIT ( []() { myinfo::SetCommandDone(); } );
	for ( int i=0; i<iSelect; i++ )
		StatCountCommand ( SEARCHD_COMMAND_SEARCH );

	// setup query for searching
	SearchHandler_c tHandler ( iSelect, sphCreatePlainQueryParser(), QUERY_SQL, true );
	QueryProfile_c tProfile;

	iSelect = 0;
	for ( auto & tStmt : dStmt )
	{
		switch ( tStmt.m_eStmt )
		{
		case STMT_SELECT:
			{
				if ( !tHandler.m_pStmt )
					tHandler.m_pStmt = &tStmt;

				// no log for search queries from the buddy in the info verbosity
				if ( session::IsQueryLogDisabled() )
					tStmt.m_tQuery.m_uDebugFlags |= QUERY_DEBUG_NO_LOG;

				tHandler.SetQuery ( iSelect, tStmt.m_tQuery, std::move ( tStmt.m_pTableFunc ) );
				tHandler.SetJoinQueryOptions ( iSelect, tStmt.m_tJoinQueryOptions );
				++iSelect;
				break;
			}
		case STMT_SET:
			if ( tStmt.m_eSet == SET_LOCAL )
			{
				CSphString sSetName ( tStmt.m_sSetName );
				sSetName.ToLower();
				if ( sSetName == "profiling" )
					tSess.SetProfile ( ParseProfileFormat ( tStmt ) );
			}
		default: break;
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
		myinfo::SetCommand ( SqlStmt2Str(eStmt) );
		AT_SCOPE_EXIT ( []() { myinfo::SetCommandDone(); } );

		const CSphQueryResultMeta & tMeta = bUseFirstMeta ? tHandler.m_dAggrResults[0] : ( iSelect-1>=0 ? tHandler.m_dAggrResults[iSelect-1] : tPrevMeta );
		bool bMoreResultsFollow = (i+1)<dStmt.GetLength();
		bool bBreak = false;

		switch ( eStmt )
		{
		case STMT_SELECT:
		{
			AggrResult_t & tRes = tHandler.m_dAggrResults[iSelect++];
			// mysql server breaks send on error
			bBreak = !tRes.m_iSuccesses;

			if ( !sWarning.IsEmpty() )
				tRes.m_sWarning = sWarning;
			if ( bBreak )
				bMoreResultsFollow = false;

			auto uMatches = SendMysqlSelectResult ( dRows, tRes, bMoreResultsFollow, false, nullptr, ( tSess.IsProfile() ? &tProfile : nullptr ) );
			gStats().AddDeltaDetailed ( SearchdStats_t::eSearch, uMatches, (uint64_t)tRes.m_iQueryTime * 1000 );
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
			dRows.Error ( "Server shutdown in progress", EMYSQL_ERR::SERVER_SHUTDOWN );
			return;
		}
	}
}

static bool HandleSetLocal ( CSphString& sError, const CSphString& sName, int64_t iSetValue, CSphString sSetValue, CSphSessionAccum& tAcc )
{
	auto& tSess = session::Info();
	if ( sName == "wait_timeout" || sName == "net_read_timeout" )
	{
		tSess.SetTimeoutS ( iSetValue );
		return true;
	}

	if ( sName == "throttling_period" )
	{
		tSess.SetThrottlingPeriodMS ( iSetValue );
		return true;
	}


	if ( sName == "net_write_timeout" )
	{
		tSess.SetWTimeoutS ( iSetValue );
		return true;
	}

	if ( sName == "thread_stack" )
	{
		session::SetMaxStackSize ( Max ( iSetValue, 1024 * 1024 ) );
		return true;
	}

	if ( sName == "optimize_by_id" )
	{
		session::SetOptimizeById ( !!iSetValue );
		return true;
	}

	if ( sName == "max_threads_per_query" )
	{
		tSess.SetDistThreads ( iSetValue );
		return true;
	}

	if ( sName == "ro" )
	{
		if ( !tSess.GetVip() )
		{
			if ( !sphCheckWeCanModify (sError) )
				return true;
		}
		tSess.SetReadOnly ( !!iSetValue );
		return true;
	}

	if ( sName == "threads_ex" )
	{
		auto dDispatchers = Dispatcher::ParseTemplates ( sSetValue.cstr() );
		tSess.SetBaseDispatcherTemplate ( dDispatchers.first );
		tSess.SetPseudoShardingDispatcherTemplate ( dDispatchers.second );
		return true;
	}

	// move check here from bison parser. Only boolean allowed below.
	if ( iSetValue != 0 && iSetValue != 1 )
	{
		sError = SphSprintf ( "sphinxql: only 0 and 1 could be used as boolean values near '%d'", iSetValue );
		return true;
	}

	if ( sName == "autocommit" )
	{
		// per-session AUTOCOMMIT
		bool bAutoCommit = ( iSetValue != 0 );
		auto pSession = session::Info().GetClientSession();
		pSession->m_bAutoCommit = bAutoCommit;
		pSession->m_bInTransaction = false;

		// commit all pending changes
		if ( bAutoCommit && tAcc.GetIndex() && !HandleCmdReplicate ( *tAcc.GetAcc() ) )
		{
			TlsMsg::MoveError(sError);
			return false;
		}
		return true;
	}

	if ( sName == "collation_connection" )
	{
		// per-session COLLATION_CONNECTION
		CSphString& sVal = sSetValue;
		sVal.ToLower();

		tSess.SetCollation ( sphCollationFromName ( sVal, &sError ) );
		return true;
	}

	if ( sName == "sql_quote_show_create" )
	{
		// per-session sql_quote_show_create
		tSess.SetSqlQuoteShowCreate ( iSetValue!=0 );
		return true;
	}

	if ( sName == "character_set_results"
				|| sName == "sql_auto_is_null"
				|| sName == "sql_safe_updates"
				|| sName == "sql_mode"
				|| sName == "time_zone" )
	{
		// per-session CHARACTER_SET_RESULTS at all; just ignore for now
		return true;
	}

	return false;
}

static void SetIOStats ( bool bIOStats = true )
{
	g_bIOStats = bIOStats;
	SetLogStatsFlags ( g_bIOStats, g_bCpuStats );
}

static void SetCPUStats ( bool bCPUStats = true )
{
	g_bCpuStats = bCPUStats;
	SetLogStatsFlags ( g_bIOStats, g_bCpuStats );
}

static bool HandleSetGlobal ( CSphString & sError, const CSphString & sName, int64_t iSetValue, CSphString sSetValue )
{
	auto& tSess = session::Info();
	if ( !tSess.GetVip() && !sphCheckWeCanModify ( sError ) )
		return true;

	// global server variable
	if ( sName == "query_log_format" )
	{
		if ( sSetValue == "plain" )
			SetLogFormat ( LOG_FORMAT::_PLAIN );
		else if ( sSetValue == "sphinxql" )
			SetLogFormat ( LOG_FORMAT::SPHINXQL );
		else
			sError = "Unknown query_log_format value (must be plain or sphinxql)";
		return true;
	}

	if ( sName == "log_level" )
	{
		if ( sSetValue == "info" )
			g_eLogLevel = SPH_LOG_INFO;
		else if ( sSetValue == "debug" )
			g_eLogLevel = SPH_LOG_DEBUG;
		else if ( sSetValue == "debugv" )
			g_eLogLevel = SPH_LOG_VERBOSE_DEBUG;
		else if ( sSetValue == "debugvv" )
			g_eLogLevel = SPH_LOG_VERY_VERBOSE_DEBUG;
		else if ( sSetValue == "replication" )
			g_eLogLevel = SPH_LOG_RPL_DEBUG;
		else if ( sSetValue.Begins ( "http" ) )
		{
			if ( !HttpSetLogVerbosity ( sSetValue ) )
				sError = "Unknown log_level value (http_on, http_off, http_bad_req_on, http_bad_req_off)";
		} else
		{
			sError = "Unknown log_level value (must be one of info, debug, debugv, debugvv, replication)";
		}
		if ( sError.IsEmpty() )
			BuddySetLogLevel ( g_eLogLevel );

		return true;
	}

	if ( sName == "query_log_min_msec" )
	{
		g_iQueryLogMinMs = (int)iSetValue;
		return true;
	}

	if ( sName == "qcache_max_bytes" )
	{
		const QcacheStatus_t& s = QcacheGetStatus();
		QcacheSetup ( iSetValue, s.m_iThreshMs, s.m_iTtlS );
		return true;
	}

	if ( sName == "qcache_thresh_msec" )
	{
		const QcacheStatus_t& s = QcacheGetStatus();
		QcacheSetup ( s.m_iMaxBytes, (int)iSetValue, s.m_iTtlS );
		return true;
	}

	if ( sName == "qcache_ttl_sec" )
	{
		const QcacheStatus_t& s = QcacheGetStatus();
		QcacheSetup ( s.m_iMaxBytes, s.m_iThreshMs, (int)iSetValue );
		return true;
	}

	if ( sName == "log_debug_filter" )
	{
		SetLogFilter ( sName);
		return true;
	}

	if ( sName == "log_http_filter" )
	{
		SetLogHttpFilter ( sSetValue );
		return true;
	}
	if ( sName == "es_compat" )
	{
		return SetLogManagement ( sSetValue, sError );
	}

	if ( sName == "net_wait" )
	{
		g_tmWaitUS = iSetValue * 1000LL;
		return true;
	}

	if ( sName == "grouping_in_utc" )
	{
		if ( IsTimeZoneSet() )
		{
			sError = "grouping_in_utc=1 conflicts with 'timezone'";
			return true;
		}

		SetGroupingInUTC ( !!iSetValue );
		return true;
	}

	if ( sName == "timezone" )
	{
		if ( GetGroupingInUTC() )
		{
			sError = "grouping_in_utc=1 conflicts with 'timezone'";
			return true;
		}

		SetTimeZone ( sSetValue.cstr(), sError );
		return true;
	}

	if ( sName == "cpustats" )
	{
		SetCPUStats ( !!iSetValue );
		return true;
	}

	if ( sName == "iostats" )
	{
		SetIOStats ( !!iSetValue );
		return true;
	}

	if ( sName == "coredump" )
	{
		CrashLogger::SetCoredump ( !!iSetValue );
		return true;
	}

	if ( sName == "maintenance" )
	{
		if ( tSess.GetVip() )
			g_bMaintenance = !!iSetValue;
		else
			sError = "Only VIP connections can set maintenance mode";
		return true;
	}

	if ( sName == "thread_stack" )
	{
		if ( tSess.GetVip() )
			Threads::SetMaxCoroStackSize ( Max ( iSetValue, 1024 * 1024 ) );
		else
			sError = "Only VIP connections can change global thread_stack value";
		return true;
	}

	if ( sName == "wait_timeout" )
	{
		if ( tSess.GetVip() )
			g_iClientQlTimeoutS = iSetValue;
		else
			sError = "Only VIP connections can change global wait_timeout value";
		return true;
	}

	if ( sName == "net_read_timeout" || sName == "read_timeout")
	{
		if ( tSess.GetVip() )
			g_iReadTimeoutS = iSetValue;
		else
			sError = "Only VIP connections can change global net_read_timeout value";
		return true;
	}

	if ( sName == "net_write_timeout" )
	{
		if ( tSess.GetVip() )
			g_iWriteTimeoutS = iSetValue;
		else
			sError = "Only VIP connections can change global net_write_timeout value";
		return true;
	}

	if ( sName == "network_timeout" )
	{
		if ( tSess.GetVip() )
		{
			g_iWriteTimeoutS = iSetValue;
			g_iReadTimeoutS = iSetValue;
		}
		else
			sError = "Only VIP connections can change global network_timeout value";
		return true;
	}

	if ( sName == "reset_network_timeout_on_packet" )
	{
		if ( tSess.GetVip() )
		{
			g_bTimeoutEachPacket = !!iSetValue;
		} else
			sError = "Only VIP connections can change global reset_network_timeout_on_packet value";
		return true;
	}

	if ( sName == "throttling_period" )
	{
		if ( tSess.GetVip() )
			Threads::Coro::SetDefaultThrottlingPeriodMS ( iSetValue );
		else
			sError = "Only VIP connections can change global throttling_period value";
		return true;
	}

	if ( sName == "max_threads_per_query" )
	{
		g_iDistThreads = iSetValue; // that is not dangerous to allow everybody change the value
		return true;
	}

	if ( sName == "auto_optimize" )
	{
		if ( !AUTOOPTIMIZE_NEEDS_VIP || tSess.GetVip() )
			g_iAutoOptimizeCutoffMultiplier = iSetValue;
		else
			sError = "Only VIP connections can change global auto_optimize value";
		return true;
	}

	if ( sName == "optimize_cutoff" )
	{
		if ( iSetValue < 1 )
			sError = SphSprintf( "optimize_cutoff should be greater than 0, got %d", iSetValue );
		else
			MutableIndexSettings_c::GetDefaults().m_iOptimizeCutoff = iSetValue;
		return true;
	}

	if ( sName == "pseudo_sharding" )
	{
		SetPseudoSharding ( !!iSetValue );
		return true;
	}

	if ( sName == "secondary_indexes" )
	{
		SetSecondaryIndexDefault ( iSetValue != 0 ? SIDefault_e::ENABLED : SIDefault_e::DISABLED );
		return true;
	}

	if ( sName == "accurate_aggregation" )
	{
		SetAccurateAggregationDefault ( !!iSetValue );
		return true;
	}

	if ( sName == "distinct_precision_threshold" )
	{
		SetDistinctThreshDefault ( iSetValue );
		return true;
	}

	if ( sName == "threads_ex" )
	{
		if ( !THREAD_EX_NEEDS_VIP || tSess.GetVip() )
			Dispatcher::SetGlobalDispatchers ( sSetValue.cstr() );
		else
			sError = "Only VIP connections can change global threads_ex value";
		return true;
	}

	if ( sName=="expansion_merge_threshold_docs" )
	{
		if ( iSetValue<0 )
			sError.SetSprintf ( "expansion_merge_threshold_docs should be positive value, got " INT64_FMT, iSetValue );
		else
			ExpandedMergeThdDocs ( iSetValue );
		return true;
	}
	if ( sName=="expansion_merge_threshold_hits" )
	{
		if ( iSetValue<0 )
			sError.SetSprintf ( "expansion_merge_threshold_hits should be positive value, got " INT64_FMT, iSetValue );
		else
			ExpandedMergeThdHits ( iSetValue );
		return true;
	}

	if ( sName=="cluster_user" )
	{
		g_sClusterUser = std::move ( sSetValue );
		return true;
	}

	return false;
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

		if ( !HandleSetLocal ( sError, tStmt.m_sSetName, tStmt.m_iSetValue, tStmt.m_sSetValue, tAcc) )
		{
			if ( tStmt.m_sSetName == "profiling" )
			{
				// per-session PROFILING
				tSess.SetProfile ( ParseProfileFormat ( tStmt ) );

			} else
			{
				// unknown variable, return error
				tOut.ErrorEx ( "Unknown session variable '%s' in SET statement", tStmt.m_sSetName.cstr() );
				return;
			}
		}

		if ( sError.IsEmpty() )
			break;
		else {
			tOut.ErrorEx ( "%s", sError.cstr() );
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

		if ( !HandleSetGlobal ( sError, tStmt.m_sSetName, tStmt.m_iSetValue, tStmt.m_sSetValue ) )
		{
			tOut.ErrorEx ( "Unknown system variable '%s'", tStmt.m_sSetName.cstr() );
			return;
		}

		if ( sError.IsEmpty() )
			break;
		else {
			tOut.ErrorEx ( "%s", sError.cstr() );
			return;
		}
		break;

	case SET_INDEX_UVAR: //  SET INDEX bar GLOBAL @foo = (values)
		if ( !SendUserVar ( tStmt.m_sIndex.cstr(), tStmt.m_sSetName.cstr(), tStmt.m_dSetValues, sError ) )
		{
			tOut.Error ( sError.cstr() );
			return;
		}
		break;

	case SET_CLUSTER_UVAR: // SET CLUSTER ident GLOBAL 'variable' = string|int
		{
			if ( !ReplicateSetOption ( tStmt.m_sIndex, tStmt.m_sSetName, tStmt.m_sSetValue ) )
			{
				tOut.Error ( TlsMsg::szError() );
				return;
			}
		}
		break;
	case SET_EXTRA: // relaxed SET SESSION foo=1, GLOBAL fee='bar', SESSION a='b', etc.
		ARRAY_FOREACH (i, tStmt.m_dInsertSchema)
		{
			auto sName = tStmt.m_dInsertSchema[i];
			sName.ToLower();
			if ( tStmt.m_dInsertValues[i].m_iType & 1) // lowest bit 1 means 'session', 0 means 'global'
			{
				if ( !HandleSetLocal ( sError, sName, tStmt.m_dInsertValues[i].GetValueInt(), tStmt.m_dInsertValues[i].m_sVal, tAcc ) )
				{
					// unknown variable, return error
					tOut.ErrorEx ( "Unknown session variable '%s' in SET statement", sName.cstr() );
					return;
				}
			} else {
				if ( !HandleSetGlobal ( sError, sName, tStmt.m_dInsertValues[i].GetValueInt(), tStmt.m_dInsertValues[i].m_sVal ) )
				{
					// unknown variable, return error
					tOut.ErrorEx ( "Unknown system variable '%s'", sName.cstr() );
					return;
				}
			}
		}

		if ( !sError.IsEmpty() )
		{
			tOut.ErrorEx ( "%s", sError.cstr() );
			return;
		}
		break;

	default:
		tOut.ErrorEx ( "internal error: unhandled SET mode %d", (int) tStmt.m_eSet );
		return;
	}

	// it went ok
	tOut.Ok();
}


void HandleMysqlAttach ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	const CSphString & sFrom = tStmt.m_sIndex;
	const CSphString & sTo = tStmt.m_sStringParam;
	bool bTruncate = ( tStmt.m_iIntParam==1 );

	if ( sFrom==sTo )
	{
		tOut.ErrorEx ( "can not ATTACH table '%s' to itself", sFrom.cstr() );
		return;
	}

	auto pServedFrom = GetServed ( sFrom );
	auto pServedTo = GetServed ( sTo );

	if ( !pServedFrom )
	{
		tOut.ErrorEx ( "no such table '%s'", sFrom.cstr() );
		return;

	} else if ( pServedFrom->m_eType!=IndexType_e::PLAIN && pServedFrom->m_eType!=IndexType_e::RT )
	{
		tOut.Error ( "1st argument to ATTACH must be a plain or a RT table" );
		return;

	} else if ( !pServedTo )
	{
		tOut.ErrorEx ( "no such table '%s'", sTo.cstr() );
		return;

	} else if ( pServedTo->m_eType!=IndexType_e::RT )
	{
		tOut.Error ( "2nd argument to ATTACH must be a RT table" );
		return;
	}

	// cluster does not implement ATTACH for now
	auto tClusterTo = IsPartOfCluster ( pServedTo );
	auto tClusterFrom = IsPartOfCluster ( pServedFrom );
	if ( tClusterTo || tClusterFrom )
	{
		if ( tClusterTo )
			tOut.ErrorEx ( "table %s is part of cluster %s, can not issue ATTACH", sTo.cstr(), tClusterTo->cstr() );
		else
			tOut.ErrorEx ( "table %s is part of cluster %s, can not issue ATTACH", sFrom.cstr(), tClusterFrom->cstr() );
		return;
	}

	bool bAttached = false;
	CSphString sError;
	WIdx_T<RtIndex_i *> pTo { pServedTo };

	if ( pServedFrom->m_eType==IndexType_e::PLAIN )
	{
		bool bFatal = false;
		WIdx_c pPlainFrom { pServedFrom };
		bAttached = pTo->AttachDiskIndex ( pPlainFrom, bTruncate, bFatal, sError );

		if ( bAttached || bFatal )
			g_pLocalIndexes->Delete ( sFrom );

		if ( bAttached )
			pServedFrom->ReleaseIdx(); // since index no more belong to us

	} else
	{
		WIdx_T<RtIndex_i*> pFrom { pServedFrom };
		AttachArgs_t tArgs { pFrom };
		tArgs.m_bTruncate = bTruncate;
		tArgs.m_bConfigless = IsConfigless();

		bAttached = pTo->AttachRtIndex ( tArgs, sError );

		if ( tArgs.m_bFatal )
			g_pLocalIndexes->Delete ( sFrom );
	}

	if ( bAttached )
		tOut.Ok();
	 else
		tOut.Error ( sError.cstr() );
}


void HandleMysqlFlushRtindex ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	auto pIndex = GetServed ( tStmt.m_sIndex );

	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( "FLUSH RTINDEX requires an existing RT table" );
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
		tOut.Error ( "FLUSH RAMCHUNK requires an existing RT table" );
		return;
	}

	RIdx_T<RtIndex_i*> pRt { pIndex };
	if ( !pRt->ForceDiskChunk() )
	{
		CSphString sError;
		sError.SetSprintf ( "table '%s': FLUSH RAMCHUNK failed; TABLE UNUSABLE (%s)", tStmt.m_sIndex.cstr(), pRt->GetLastError().cstr() );
		tOut.Error ( sError.cstr () );
		g_pLocalIndexes->Delete ( tStmt.m_sIndex );
		return;
	}

	tOut.Ok();
}


void HandleMysqlFlush ( RowBuffer_i & tOut, const SqlStmt_t & )
{
	int iTag = CommandFlush();
	tOut.HeadBegin ();
	tOut.HeadColumn ( "tag", MYSQL_COL_LONG );
	tOut.HeadEnd ();

	// data packet, var value
	tOut.PutNumAsString ( iTag );
	tOut.Commit();

	// done
	tOut.Eof();
}

// same for select ... from index.files
void HandleSelectFiles ( RowBuffer_i & tOut, const CSphString & sIndex, const CSphString & sThreadFormat )
{
	tOut.HeadBegin ();
	tOut.HeadColumn ( "file" );
	tOut.HeadColumn ( "normalized" );
	tOut.HeadColumn ( "size", MYSQL_COL_LONGLONG );
	if ( !tOut.HeadEnd () )
		return;

	auto pServed = GetServed ( sIndex );
	if ( !ServedDesc_t::IsLocal ( pServed ) )
	{
		tOut.Error ( "FILES requires an existing local table" );
		return;
	}

	StrVec_t dFiles;
	StrVec_t dExt;
	RIdx_c ( pServed )->GetIndexFiles ( dFiles, dExt );

	auto sFormat = sThreadFormat;
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

// fwd
static bool PrepareReconfigure ( const char * szIndex, CSphReconfigureSettings & tSettings, StrVec_t * pWarnings, CSphString & sError );

void HandleMysqlTruncate ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	bool bReconfigure = ( tStmt.m_iIntParam==1 );

	auto pCmd = MakeReplicationCommand ( ReplCmd_e::TRUNCATE, tStmt.m_sIndex, tStmt.m_sCluster );
	CSphString sError;
	StrVec_t dWarnings;
	const CSphString & sIndex = tStmt.m_sIndex;

	if ( bReconfigure )
	{
		pCmd->m_tReconfigure = std::make_unique<CSphReconfigureSettings>();
		pCmd->m_tReconfigure->m_bChangeSchema = true;
	}

	if ( bReconfigure && !PrepareReconfigure ( sIndex.cstr(), *pCmd->m_tReconfigure, &dWarnings, sError ) )
	{
		tOut.Error ( sError.cstr () );
		return;
	}

	// get an exclusive lock for operation
	// but only read lock for check
	auto pIndex = GetServed ( sIndex );
	{
		if ( !ServedDesc_t::IsMutable ( pIndex ) )
		{
			tOut.Error ( "TRUNCATE RTINDEX requires an existing RT table" );
			return;
		}

		if ( !ValidateClusterStatement ( sIndex, *pIndex, tStmt.m_sCluster, IsHttpStmt ( tStmt ) ) )
		{
			tOut.Error ( TlsMsg::szError() );
			return;
		}
	}

	assert ( pIndex );
	Threads::Coro::ScopedWriteTable_c tWriting { pIndex->Locker() };
	if ( !tWriting.CanWrite () )
	{
		tOut.Error ( "table is locked" );
		return;
	}

	auto* pSession = session::GetClientSession();
	auto& tAcc = pSession->m_tAcc;
	auto* pAccum = tAcc.GetAcc();
	pAccum->m_dCmd.Add ( std::move ( pCmd ) );

	sWarning = ConcatWarnings ( dWarnings );

	bool bRes = HandleCmdReplicate ( *pAccum );
	if ( !bRes )
		tOut.Error ( TlsMsg::szError() );
	else
		tOut.Ok ( 0, dWarnings.GetLength() );
}


void HandleMysqlOptimize ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	auto sIndex = tStmt.m_sIndex;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( "OPTIMIZE TABLE requires an existing RT table" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eManualOptimize;
	tTask.m_iCutoff = tStmt.m_tQuery.m_iCutoff<=0 ? 0 : tStmt.m_tQuery.m_iCutoff;

	auto bOptimizeStarted = RIdx_T<RtIndex_i *> ( pIndex )->StartOptimize ( std::move ( tTask ) );

	if ( tStmt.m_tQuery.m_bSync && !bOptimizeStarted )
	{
		tOut.Error ( "Can't optimize frozen table" );
		return;
	}

	if ( tStmt.m_tQuery.m_bSync && !PollOptimizeRunning ( sIndex ) )
		tOut.Error ( "RT table went away during waiting" );
	else
		tOut.Ok ();
}

static CSphString GetLastInsertId ( const ClientSession_c * pSession )
{
    StringBuilder_c sBuf ( "," );

	if ( pSession->m_dLastIds.IsEmpty() )
		sBuf << 0;
	else
		pSession->m_dLastIds.Apply ( [&sBuf] ( int64_t iID ) { sBuf << iID; } );

    return CSphString ( sBuf );
}

class ExtraLastInsertID_c final: public ISphExtra
{
	bool ExtraDataImpl ( ExtraData_e eCmd, void** pData ) final
	{
		if ( eCmd != EXTRA_GET_LAST_INSERT_ID )
			return false;

		auto* sVal = (CSphString*)pData;
		assert ( sVal );
		*sVal = GetLastInsertId ( session::Info().GetClientSession() );
		return true;
	}
};


// STMT_SELECT_COLUMNS: SELECT @@sysvar1 [ as alias] [@@sysvarN [ as alias]] [limit M]
// SELECT expr, @@sysvar1, expr2, ... [limit M]
void HandleMysqlSelectColumns ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, ClientSession_c* pSession )
{
	struct SysVar_t
	{
		const MysqlColumnType_e m_eType;
		const char* m_szName;
		std::function<CSphString ( void )> m_fnValue;
	};

	const SysVar_t dSysvars[] =
	{
		{ MYSQL_COL_STRING,	nullptr, [] { return "<empty>"; } }, // stub
		{ MYSQL_COL_LONG,	"@@session.auto_increment_increment",	[] {return "1";}},
		{ MYSQL_COL_STRING,	"@@character_set_client", [] {return "utf8";}},
		{ MYSQL_COL_STRING,	"@@character_set_connection", [] {return "utf8";}},
		{ MYSQL_COL_LONG,	"@@max_allowed_packet", [] { StringBuilder_c s; s << g_iMaxPacketSize; return CSphString(s); }},
		{ MYSQL_COL_STRING,	"@@version_comment", [] { return szGIT_BRANCH_ID;}},
		{ MYSQL_COL_LONG,	"@@lower_case_table_names", [] { return "1"; }},
		{ MYSQL_COL_STRING,	"@@session.last_insert_id", [pSession] { return GetLastInsertId ( pSession ); } },
		{ MYSQL_COL_LONG, "@@autocommit", [pSession] { return pSession->m_bAutoCommit ? "1" : "0"; } },
	};

	auto fnVarIdxByName = [&dSysvars] ( const CSphString& sName ) noexcept -> int
	{
		constexpr auto iSysvars = sizeof ( dSysvars ) / sizeof ( dSysvars[0] );
		for ( int i = 1; i<(int)iSysvars; ++i )
			if ( sName == dSysvars[i].m_szName )
				return i;
		return 0;
	};

	const auto& dItems = tStmt.m_tQuery.m_dItems;

	struct PreparedItem_t {
		ESphAttr m_eType;
		MysqlColumnType_e m_eTypeMysql;
		ISphExprRefPtr_c m_pExpr;
		int m_iSysvarIdx;
		const char* m_szAlias;
		CSphString m_sError;
	};

	CSphVector<PreparedItem_t> dColumns;

	const bool bHasBuddy = HasBuddy();
	bool bHaveValidExpressions = false; // whether we have at least one expression among @@sysvars
	bool bHaveInvalidExpressions = false; // whether at least one expression is erroneous

	for ( const auto & tItem : dItems )
	{
		CSphString sError;
		auto iVar = fnVarIdxByName ( tItem.m_sExpr );
		if ( !iVar )
		{
			CSphString sVar = tItem.m_sExpr;
			CSphSchema tSchema;
			ESphAttr eAttrType;
			ExprParseArgs_t tExprArgs;
			tExprArgs.m_pAttrType = &eAttrType;
			ISphExprRefPtr_c pExpr { sphExprParse ( sVar.cstr(), tSchema, nullptr, sError, tExprArgs ) };
			if ( pExpr )
			{
				dColumns.Add ( { eAttrType, ESphAttr2MysqlColumn ( eAttrType ), pExpr, -1, tItem.m_sAlias.cstr() } );
				bHaveValidExpressions = true;
				continue;
			}

			// failure:
			// - if a buddy exists and any unknown sysvar is requested
			// - if no buddy exists and a non-sysvar is requested or the expression parser failed
			bool bSysVar = tItem.m_sExpr.Begins ( "@@" );
			if ( bSysVar && bHasBuddy )
			{
				sError.SetSprintf ( "unknown sysvar %s", tItem.m_sExpr.cstr() );
				bHaveInvalidExpressions = true;
			} else if ( !bSysVar )
				bHaveInvalidExpressions = true;
		}
		dColumns.Add ( { SPH_ATTR_NONE, dSysvars[iVar].m_eType, nullptr, iVar, tItem.m_sAlias.cstr(), sError } );
	}

	assert ( dColumns.GetLength() == dItems.GetLength() );

	// fail when we have error in expression or any unknown sysvar - buddy should handle that
	if ( bHaveInvalidExpressions )
	{
		StringBuilder_c sError ("; ");
		dColumns.for_each( [&sError] (const PreparedItem_t& dCol) { if ( !dCol.m_sError.IsEmpty()) sError << dCol.m_sError; });
		tOut.Error ( sError.cstr() );
		return;
	}

	// fill header
	tOut.HeadBegin ();
	for ( const auto& dColumn : dColumns )
		tOut.HeadColumn ( dColumn.m_szAlias, dColumn.m_eTypeMysql );

	if ( !tOut.HeadEnd() )
		return;

	if ( bHaveValidExpressions )
	{
		ExtraLastInsertID_c tIds;
		for ( auto& pExpr : dColumns )
			if ( pExpr.m_pExpr )
				pExpr.m_pExpr->Command ( SPH_EXPR_SET_EXTRA_DATA, &tIds );
	}

	std::optional<ExtraLastInsertID_c> tIds;

	// fill values
	for ( auto& dColumn : dColumns )
	{
		if ( dColumn.m_pExpr ) // expression
		{
			if ( !tIds.has_value() )
				tIds.emplace();

			auto& pExpr = dColumn.m_pExpr;
			pExpr->Command ( SPH_EXPR_SET_EXTRA_DATA, &tIds.value() );

			CSphMatch tMatch;

			switch ( dColumn.m_eType )
			{
			case SPH_ATTR_STRINGPTR:
				{
					const BYTE* pStr = nullptr;
					int iLen = pExpr->StringEval ( tMatch, &pStr );
					tOut.PutArray ( { pStr, iLen } );
					FreeDataPtr ( *pExpr, pStr );
					break;
				}
			case SPH_ATTR_INTEGER:	tOut.PutNumAsString ( pExpr->IntEval ( tMatch ) ); break;
			case SPH_ATTR_BIGINT:	tOut.PutNumAsString ( pExpr->Int64Eval ( tMatch ) ); break;
			case SPH_ATTR_UINT64:	tOut.PutNumAsString ( (uint64_t)pExpr->Int64Eval ( tMatch ) ); break;
			case SPH_ATTR_FLOAT:	tOut.PutFloatAsString ( pExpr->Eval ( tMatch ) ); break;
			case SPH_ATTR_DOUBLE:	tOut.PutDoubleAsString ( pExpr->Eval ( tMatch ) ); break;
			default:
				tOut.PutNULL();
				break;
			}
		}
		else
			tOut.PutString ( dSysvars[dColumn.m_iSysvarIdx].m_fnValue() );
	}

	// finalize
	tOut.Commit ();
	tOut.Eof ();
}


void HandleMysqlShowCollations ( RowBuffer_i & tOut )
{
	// MySQL Connector/J really expects an answer here
	// field packets
	tOut.HeadBegin ();
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
	tOut.HeadBegin ();
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
		case SPH_LOG_RPL_DEBUG:				return "replication";
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
		dTable.MatchTuplet ( "query_log_format", LogFormat()==LOG_FORMAT::_PLAIN ? "plain" : "sphinxql" );
		dTable.MatchTuplet ( "session_read_only", session::GetReadOnly() ? "1" : "0" );
		dTable.MatchTuplet ( "log_level", LogLevelName ( g_eLogLevel ) );
		dTable.MatchTupletf ( "max_allowed_packet", "%d", g_iMaxPacketSize );
		dTable.MatchTuplet ( "character_set_client", "utf8" );
		dTable.MatchTuplet ( "character_set_connection", "utf8" );
		dTable.MatchTuplet ( "grouping_in_utc", GetGroupingInUTC() ? "1" : "0" );
		dTable.MatchTuplet ( "timezone", GetTimeZoneName().cstr() );
		dTable.MatchTupletFn ( "last_insert_id" , [&pVars]  { return GetLastInsertId ( pVars ); } );
	}
	dTable.MatchTuplet ( "pseudo_sharding", GetPseudoSharding() ? "1" : "0" );

	switch ( GetSecondaryIndexDefault() )
	{
	case SIDefault_e::FORCE:
		dTable.MatchTuplet ( "secondary_indexes", "force" ); break;
	case SIDefault_e::ENABLED:
		dTable.MatchTuplet ( "secondary_indexes", "1" ); break;
	default:
		dTable.MatchTuplet ( "secondary_indexes", "0" );
	}

	dTable.MatchTuplet ( "accurate_aggregation", GetAccurateAggregationDefault() ? "1" : "0" );
	dTable.MatchTupletf ( "distinct_precision_threshold", "%d", GetDistinctThreshDefault() );
	dTable.MatchTupletFn ( "threads_ex_effective", [] {
		StringBuilder_c tBuf;
		auto x = GetEffectiveBaseDispatcherTemplate();
		auto y = GetEffectivePseudoShardingDispatcherTemplate();
		Dispatcher::RenderTemplates ( tBuf, { x, y } );
		return tBuf;
	} );
	dTable.MatchTuplet ( "cluster_user", g_sClusterUser.scstr() );

	if ( tStmt.m_iIntParam>=0 ) // that is SHOW GLOBAL VARIABLES
	{
		dTable.MatchTupletf ( "thread_stack", "%d", Threads::GetMaxCoroStackSize() );
		dTable.MatchTupletFn ( "threads_ex", [] {
			StringBuilder_c tBuf;
			auto x = Dispatcher::GetGlobalBaseDispatcherTemplate();
			auto y = Dispatcher::GetGlobalPseudoShardingDispatcherTemplate();
			Dispatcher::RenderTemplates ( tBuf, { x, y } );
			return tBuf;
		} );
		Uservar_e eType = tStmt.m_iIntParam==0 ? USERVAR_INT_SET : USERVAR_INT_SET_TMP;
		IterateUservars ( [&dTable, eType] ( const NamedRefVectorPair_t &dVar ) {
			if ( dVar.second.m_eType==eType )
				dTable.MatchTupletf ( dVar.first.cstr(), "%d", dVar.second.m_pVal ? dVar.second.m_pVal->GetLength() : 0 );
		});
	} else { // that is local (session) variables
		dTable.MatchTupletf ( "thread_stack", "%d", session::GetMaxStackSize() );
		dTable.MatchTupletFn ( "threads_ex", [] {
			StringBuilder_c tBuf;
			auto x = ClientTaskInfo_t::Info().GetBaseDispatcherTemplate();
			auto y = ClientTaskInfo_t::Info().GetPseudoShardingDispatcherTemplate();
			Dispatcher::RenderTemplates ( tBuf, { x, y } );
			return tBuf;
		});
		dTable.MatchTuplet ( "user", session::GetClientSession()->m_sUser.scstr () );
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
			uQueries ? sBuf.Sprintf( R"("%s_sec":%.3F)", sType, uStat / 1000 ) : sBuf.AppendName( sType ) << R"("-")";
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


void ServedStats_c::GetIndexQueryStats ( VectorLike & dStatus ) const
{
	QueryStats_t tQueryTimeStats, tRowsFoundStats;
	CalculateQueryStats ( tRowsFoundStats, tQueryTimeStats );
	AddQueryTimeStatsToOutput ( dStatus, "query_time", tQueryTimeStats );

#ifndef NDEBUG
	QueryStats_t tExactQueryTimeStats, tExactRowsFoundStats;
	CalculateQueryStatsExact ( tExactQueryTimeStats, tExactRowsFoundStats );
	AddQueryTimeStatsToOutput ( dStatus, "exact_query_time", tQueryTimeStats );
#endif

	AddFoundRowsStatsToOutput ( dStatus, "found_rows", tRowsFoundStats );

	// command stats
	SearchdCommand_e dCommands[] = { SEARCHD_COMMAND_SEARCH, SEARCHD_COMMAND_EXCERPT, SEARCHD_COMMAND_UPDATE, SEARCHD_COMMAND_KEYWORDS, SEARCHD_COMMAND_STATUS, SEARCHD_COMMAND_DELETE, SEARCHD_COMMAND_INSERT, SEARCHD_COMMAND_REPLACE, SEARCHD_COMMAND_COMMIT, SEARCHD_COMMAND_SUGGEST, SEARCHD_COMMAND_CALLPQ, SEARCHD_COMMAND_GETFIELD };
	for ( auto eCmd : dCommands )
		dStatus.MatchTupletf ( szCommand ( eCmd ), "%l", m_tCommandsStats.Get ( eCmd ) );

	// detailed stats
	FormatCmdStats ( m_tCommandsStats, "insert_replace", SearchdStats_t::eReplace, dStatus );
	FormatCmdStats ( m_tCommandsStats, "search", SearchdStats_t::eSearch, dStatus );
	FormatCmdStats ( m_tCommandsStats, "update", SearchdStats_t::eUpdate, dStatus );
}

static void AddDiskIndexStatus ( VectorLike & dStatus, const CSphIndex * pIndex, bool bRt, bool bPq )
{
	auto iDocs = pIndex->GetStats ().m_iTotalDocuments;
	if ( bPq )
	{
		dStatus.MatchTupletf ( "stored_queries", "%l", iDocs );
	} else {
		dStatus.MatchTupletf ( "indexed_documents", "%l", iDocs );
		dStatus.MatchTupletf ( "indexed_bytes", "%l", pIndex->GetStats ().m_iTotalBytes );
	}

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
	if ( !bPq )
	{
		dStatus.MatchTupletf ( "disk_mapped", "%l", tStatus.m_iMapped );
		dStatus.MatchTupletf ( "disk_mapped_cached", "%l", tStatus.m_iMappedResident );
		dStatus.MatchTupletf ( "disk_mapped_doclists", "%l", tStatus.m_iMappedDocs );
		dStatus.MatchTupletf ( "disk_mapped_cached_doclists", "%l", tStatus.m_iMappedResidentDocs );
		dStatus.MatchTupletf ( "disk_mapped_hitlists", "%l", tStatus.m_iMappedHits );
		dStatus.MatchTupletf ( "disk_mapped_cached_hitlists", "%l", tStatus.m_iMappedResidentHits );
		dStatus.MatchTupletf ( "killed_documents", "%l", tStatus.m_iDead );
		dStatus.MatchTupletFn ( "killed_rate", [&tStatus, iDocs] {
			StringBuilder_c sPercent;
			auto iTotalDocs = iDocs + tStatus.m_iDead;
			if ( iTotalDocs )
				sPercent.Sprintf ( "%0.2F%%", tStatus.m_iDead * 10000 / iTotalDocs );
			else
				sPercent << "0.00%";
			return CSphString ( sPercent.cstr () );
		} );
	}
	if ( bRt )
	{
		dStatus.MatchTupletf ( "ram_chunk", "%l", tStatus.m_iRamChunkSize );
		dStatus.MatchTupletf ( "ram_chunk_segments_count", "%d", tStatus.m_iNumRamChunks );
		dStatus.MatchTupletf ( "disk_chunks", "%d", tStatus.m_iNumChunks );
		dStatus.MatchTupletf ( "mem_limit", "%l", tStatus.m_iMemLimit );
		dStatus.MatchTupletf ( "mem_limit_rate", "%0.2F%%", PercentOf ( tStatus.m_fSaveRateLimit, 1.0, 2 ) );
		dStatus.MatchTupletf ( "ram_bytes_retired", "%l", tStatus.m_iRamRetired );
		dStatus.MatchTupletf ( "optimizing", "%l", tStatus.m_iOptimizesCount );
		dStatus.MatchTupletf ( "locked", "%d", tStatus.m_iLockCount );
	}
	if ( bPq )
	{
		dStatus.MatchTupletf ( "max_stack_need", "%l", tStatus.m_iStackNeed );
		dStatus.MatchTupletf ( "average_stack_base", "%l", tStatus.m_iStackBase );
		dStatus.MatchTupletf ( "desired_thread_stack", "%l", sphRoundUp ( tStatus.m_iStackNeed + tStatus.m_iStackBase, 128 ) );
		dStatus.MatchTupletf ( "locked", "%d", tStatus.m_iLockCount );
	}

	if ( bRt || bPq )
	{
		dStatus.MatchTupletf ( "tid", "%l", tStatus.m_iTID );
		dStatus.MatchTupletf ( "tid_saved", "%l", tStatus.m_iSavedTID );
	}
}

const char * szIndexType ( IndexType_e eType )
{
	switch ( eType )
	{
	case IndexType_e::PLAIN: return "local";
	case IndexType_e::TEMPLATE: return "template";
	case IndexType_e::RT: return "rt";
	case IndexType_e::PERCOLATE: return "percolate";
	case IndexType_e::DISTR: return "distributed";
	default: return "unknown";
	}
}

static void AddPlainIndexStatus ( RowBuffer_i & tOut, const cServedIndexRefPtr_c& pServed, const ServedStats_c& tStats, const CSphString & sName, const CSphString & sPattern )
{
	assert ( pServed );
	RIdx_c pIndex { pServed };
	assert ( pIndex );

	VectorLike dStatus ( sPattern );
	dStatus.MatchTuplet ( "table_type", szIndexType ( pServed->m_eType ) );
	if ( pServed->m_eType != IndexType_e::TEMPLATE )
	{
		AddDiskIndexStatus ( dStatus, pIndex, pServed->m_eType == IndexType_e::RT, pServed->m_eType == IndexType_e::PERCOLATE );
		tStats.GetIndexQueryStats ( dStatus );
	}
	tOut.DataTable ( dStatus );
}


static void AddDistibutedIndexStatus ( RowBuffer_i & tOut, const cDistributedIndexRefPtr_t& pIndex, const CSphString & sName, const CSphString & sPattern )
{
	assert ( pIndex );
	VectorLike dStatus ( sPattern );
	dStatus.MatchTuplet( "table_type", "distributed" );
	pIndex->m_tStats.GetIndexQueryStats ( dStatus );
	tOut.DataTable ( dStatus );
}

void HandleMysqlShowIndexStatus ( RowBuffer_i& tOut, const SqlStmt_t& tStmt )
{
	CSphString sError;
	auto pServed = GetServed ( tStmt.m_sIndex );

	int iChunk = tStmt.m_iIntParam;
	if ( tStmt.m_dIntSubkeys.GetLength() >= 1 )
		iChunk = tStmt.m_dIntSubkeys[0];

	if ( pServed )
	{
		pServed->m_pStats->IncCmd ( SEARCHD_COMMAND_STATUS );

		if ( iChunk >= 0 && pServed->m_eType == IndexType_e::RT )
		{
			RIdx_T<const RtIndex_i*> ( pServed )->ProcessDiskChunk ( iChunk, [&tOut, &tStmt] ( const CSphIndex* pIndex ) {
				if ( !pIndex )
				{
					tOut.Error ( "SHOW TABLE STATUS requires an existing table" );
					return;
				}

				VectorLike dStatus ( tStmt.m_sStringParam );
				AddDiskIndexStatus ( dStatus, pIndex, false, false );
				tOut.DataTable ( dStatus );
			} );
		} else
			AddPlainIndexStatus ( tOut, pServed, *pServed->m_pStats, tStmt.m_sIndex, tStmt.m_sStringParam );
		return;
	}

	auto pIndex = GetDistr ( tStmt.m_sIndex );

	if ( pIndex )
	{
		pIndex->m_tStats.IncCmd ( SEARCHD_COMMAND_STATUS );
		AddDistibutedIndexStatus ( tOut, pIndex, tStmt.m_sIndex, tStmt.m_sStringParam );
	} else
	{
		tOut.Error ( "SHOW TABLE STATUS requires an existing table" );
	}
}

static bool AddFederatedIndexStatusHeader ( RowBuffer_i& tOut )
{
	return tOut.HeadOfStrings ( { "Name", "Engine", "Version", "Row_format", "Rows", "Avg_row_length", "Data_length", "Max_data_length", "Index_length", "Data_free", "Auto_increment", "Create_time", "Update_time", "Check_time", "Collation", "Checksum", "Create_options", "Comment" } );
}

static void AddFederatedIndexStatusLine ( const CSphSourceStats& tStats, const CSphString& sName, RowBuffer_i& tOut )
{
	tOut.PutString ( sName );						  // Name
	tOut.PutString ( "InnoDB" );					  // Engine
	tOut.PutString ( "10" );						  // Version
	tOut.PutString ( "Dynamic" );					  // Row_format
	tOut.PutNumAsString ( tStats.m_iTotalDocuments ); // Rows
	tOut.PutString ( "4096" );						  // Avg_row_length
	tOut.PutString ( "0" );							  // Data_length
	tOut.PutString ( "0" );							  // Max_data_length
	tOut.PutString ( "0" );							  // Index_length
	tOut.PutString ( "0" );							  // Data_free
	tOut.PutString ( "5" );							  // Auto_increment
	tOut.PutNULL();									  // Create_time
	tOut.PutNULL();									  // Update_time
	tOut.PutNULL();									  // Check_time
	tOut.PutString ( "utf8" );						  // Collation
	tOut.PutNULL();									  // Checksum
	tOut.PutString ( "" );							  // Create_options
	tOut.PutString ( "" );							  // Comment
	tOut.Commit();
}

void HandleMysqlShowFederatedIndexStatus ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;

	if ( !AddFederatedIndexStatusHeader ( tOut ) )
		return;

	CheckLike tSelector { tStmt.m_sStringParam.cstr() };
	auto dIndexes = GetAllServedIndexes();

	bool bWithClusters = ClusterFlavour();

	// fake stat for distrs
	CSphSourceStats tFakeStats;
	tFakeStats.m_iTotalDocuments = 1000; // TODO: check is it worth to query that number from agents

	for ( const NamedIndexType_t& tIndex : dIndexes )
	{
		CSphString sFullName;
		if ( bWithClusters && !tIndex.m_sCluster.IsEmpty () )
			sFullName.SetSprintf ("%s:%s", tIndex.m_sCluster.cstr(), tIndex.m_sName.cstr());
		const CSphString& sName = ( bWithClusters && !tIndex.m_sCluster.IsEmpty () ) ? sFullName : tIndex.m_sName;

		if ( !tSelector.Match ( sName.cstr() ) )
			continue;

		if ( tIndex.m_eType == IndexType_e::DISTR )
			AddFederatedIndexStatusLine ( tFakeStats, sName, tOut );
		else {
			auto pServed = GetServed ( tIndex.m_sName );
			if ( !pServed )
				continue; // really rare case when between GetAllServedIndexes and that moment table was removed.
			RIdx_c pIndex { pServed };
			assert ( pIndex );
			AddFederatedIndexStatusLine ( pIndex->GetStats(), sName, tOut );
		}
	}

	tOut.Eof ();
}

void PutIndexStatus ( RowBuffer_i & tOut, const CSphIndex * pIndex )
{
	tOut.PutString ( pIndex->GetFilebase () );

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
	tOut.HeadBegin ();
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
		tOut.Error ( "select TABLE.@status requires an existing table" );
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
		tOut.Error ( "SHOW TABLE SETTINGS requires an existing table" );
		return;
	}

	int iChunk = tStmt.m_iIntParam;
	if ( tStmt.m_dIntSubkeys.GetLength ()>=1 )
		iChunk = (int) tStmt.m_dIntSubkeys[0];

	auto fnShowSettings = [&tOut, szStmt=tStmt.m_sStmt] ( const CSphIndex* pIndex )
	{
		if ( !pIndex )
		{
			tOut.Error ( "SHOW TABLE SETTINGS requires an existing table" );
			return;
		}
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


static void OutputSIStats ( RowBuffer_i & tOut, const CheckLike & tLike, const SI::IndexAttrInfo_t & tInfo, float fPercent )
{
	const char * szName = tInfo.m_sName.c_str();
	CSphString sType = ColumnarAttrType2Str ( tInfo.m_eType );
	CSphString sEnabled = tInfo.m_bEnabled ? "1" : "0";
	if ( !tLike.Match(szName) && !tLike.Match ( sType.cstr() ) && !tLike.Match ( sEnabled.cstr() ) )
		return;

	tOut.PutString(szName);
	tOut.PutString(sType);
	tOut.PutString(sEnabled);
	tOut.PutNumAsString ( (int)(fPercent*100.0f) );
	tOut.Commit();
}


static bool operator == ( const SI::IndexAttrInfo_t & tA, const SI::IndexAttrInfo_t & tB )
{
	return tA.m_sName==tB.m_sName && tA.m_eType==tB.m_eType && tA.m_bEnabled==tB.m_bEnabled;
}


static void ShowTableIndexes ( RowBuffer_i & tOut, const CSphString & sPattern, std::vector<SI::IndexAttrInfo_t> & dInfo, int iNumChunks )
{
	assert( iNumChunks>0 );
	if ( dInfo.empty() )
		return;

	std::sort ( dInfo.begin(), dInfo.end(), []( const auto & a, const auto & b )
		{
			if ( a.m_eType!=b.m_eType )
				return a.m_eType<b.m_eType;

			if ( a.m_sName!=b.m_sName )
				return a.m_sName<b.m_sName;

			return a.m_bEnabled > b.m_bEnabled;
		}
	);

	CheckLike tLike ( sPattern.cstr() );

	int iCount = 0;
	for ( int i = 1; i < (int)dInfo.size(); i++ )
	{
		if ( dInfo[i]==dInfo[i-1] )
			iCount++;
		else
		{
			OutputSIStats ( tOut, tLike, dInfo[i-1], float(iCount+1)/iNumChunks );
			iCount = 0;
		}
	}

	OutputSIStats ( tOut, tLike, dInfo.back(), float(iCount+1)/iNumChunks );
}


void HandleMysqlShowTableIndexes ( RowBuffer_i & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( !pServed )
	{
		tOut.Error ( "SHOW TABLE INDEXES requires an existing table" );
		return;
	}

	int iChunk = tStmt.m_iIntParam;
	if ( tStmt.m_dIntSubkeys.GetLength ()>=1 )
		iChunk = (int) tStmt.m_dIntSubkeys[0];

	if ( !tOut.HeadOfStrings ( { "Name", "Type", "Enabled", "Percent" } ) )
		return;

	std::vector<SI::IndexAttrInfo_t> dInfo;
	bool bWarn = pServed->m_eType == IndexType_e::RT && iChunk>=0;
	bool bKeepIteration = true;
	auto fnCollectIndexes = [&tOut,&bKeepIteration,&dInfo,bWarn] ( const CSphIndex* pIndex )
	{
		if ( !pIndex )
		{
			if ( bWarn )
				tOut.Error ( "SHOW TABLE INDEXES requires an existing table" );

			bKeepIteration = false;
			return;
		}

		const SIContainer_c * pSI = pIndex->GetSI();
		if ( pSI )
			pSI->GetIndexAttrInfo(dInfo);
	};

	if ( pServed->m_eType==IndexType_e::RT )
	{
		if ( iChunk>=0 )
			RIdx_T<const RtIndex_i*> ( pServed )->ProcessDiskChunk ( iChunk, fnCollectIndexes );
		else
		{
			iChunk = 0;
			while ( bKeepIteration )
			{
				RIdx_T<const RtIndex_i*> ( pServed )->ProcessDiskChunk ( iChunk, fnCollectIndexes );
				if ( bKeepIteration )
					iChunk++;
			}
		}
	}
	else
		fnCollectIndexes ( RIdx_c(pServed) );

	ShowTableIndexes ( tOut, tStmt.m_sStringParam, dInfo, Max(iChunk,1) );
	tOut.Eof();
}


void HandleMysqlShowProfile ( RowBuffer_i & tOut, const QueryProfile_c & p, bool bMoreResultsFollow )
{
	#define SPH_QUERY_STATE(_name,_desc) _desc,
	static const char * dStates [ SPH_QSTATE_TOTAL ] = { SPH_QUERY_STATES };
	#undef SPH_QUERY_STATES

	tOut.HeadBegin ();
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
	for ( int i=0; i<SPH_QSTATE_TOTAL; ++i )
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
		if ( !tOut.Commit() )
			return;
	}
	snprintf ( sTime, sizeof(sTime), "%d.%06d", int(tmTotal/1000000), int(tmTotal%1000000) );
	tOut.PutString ( "total" );
	tOut.PutString ( sTime );
	tOut.PutNumAsString ( iCount );
	tOut.PutString ( "0" );
	tOut.Commit();
	tOut.Eof ( bMoreResultsFollow );
}


static void AddAttrToIndex ( const SqlStmt_t & tStmt, CSphIndex * pIdx, CSphString & sError, bool bModify )
{
	CSphString sAttrToAdd = tStmt.m_sAlterAttr;
	sAttrToAdd.ToLower();

	bool bIndexed = tStmt.m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED;
	bool bStored = tStmt.m_uFieldFlags & CSphColumnInfo::FIELD_STORED;
	bool bAttribute = tStmt.m_uFieldFlags & CSphColumnInfo::FIELD_IS_ATTRIBUTE; // beware, m.b. true only for strings

	auto pHasAttr = pIdx->GetMatchSchema ().GetAttr ( sAttrToAdd.cstr () );
	bool bHasField = pIdx->GetMatchSchema ().GetFieldIndex ( sAttrToAdd.cstr () )!=-1;
	const bool bInt2Bigint = pHasAttr
			&& pHasAttr->m_eAttrType==SPH_ATTR_INTEGER
			&& pHasAttr->m_eEngine==AttrEngine_e::DEFAULT
			&& tStmt.m_eAlterColType==SPH_ATTR_BIGINT
			&& tStmt.m_eEngine==AttrEngine_e::DEFAULT;

	if ( !bIndexed && pHasAttr )
	{
		if ( !bModify || !bInt2Bigint )
		{
			sError.SetSprintf ( "'%s' attribute already in schema", sAttrToAdd.cstr () );
			return;
		}
	}

	if ( bModify )
	{
		if ( !pHasAttr )
		{
			sError.SetSprintf ( "attribute '%s' does not exist", sAttrToAdd.cstr() );
			return;
		}
		if ( !bInt2Bigint )
		{
			sError.SetSprintf ( "attribute '%s': only alter from rowise int to bigint supported", sAttrToAdd.cstr () );
			return;
		}
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
	tCtx.m_tKNN = tStmt.m_tAlterKNN;
	tCtx.m_tKNNModel = tStmt.m_tAlterKNNModel;
	tCtx.m_sKNNFrom = tStmt.m_sAlterKnnFrom;

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

	auto pAttr = pIdx->GetMatchSchema().GetAttr ( sAttrToRemove.cstr() );
	auto pField = pIdx->GetMatchSchema().GetField ( sAttrToRemove.cstr() );
	if ( !pAttr && !pField )
	{
		sError.SetSprintf ( "attribute '%s' does not exist", sAttrToRemove.cstr() );
		return;
	}

	if ( pAttr && ( sAttrToRemove==sphGetDocidName () || sphIsInternalAttr ( *pAttr ) ) )
	{
		sError.SetSprintf ( "unable to remove built-in attribute '%s'", sAttrToRemove.cstr() );
		return;
	}

	if ( pAttr && pIdx->GetMatchSchema().GetAttrsCount()==1 )
	{
		sError.SetSprintf ( "unable to remove last attribute '%s'", sAttrToRemove.cstr() );
		return;
	}

	if ( pAttr )
	{
		AttrAddRemoveCtx_t tCtx;
		tCtx.m_sName = sAttrToRemove;
		tCtx.m_eType = pAttr->m_eAttrType;
		pIdx->AddRemoveAttribute ( false, tCtx, sError );
	}

	if ( pField )
		pIdx->AddRemoveField ( false, sAttrToRemove, 0, sError );
}

enum class Alter_e
{
	AddColumn,
	DropColumn,
	ModifyColumn,
	RebuildSI,
	RebuildKNN,
	ApiKey
};

static void HandleMysqlAlter ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, Alter_e eAction )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;
	MEMORY ( MEM_SQL_ALTER );

	SearchFailuresLog_c dErrors;
	CSphString sError;

	if ( eAction==Alter_e::AddColumn && tStmt.m_eAlterColType==SPH_ATTR_NONE )
	{
		sError.SetSprintf ( "unsupported attribute type '%d'", tStmt.m_eAlterColType );
		tOut.Error ( sError.cstr() );
		return;
	}

	StrVec_t dNames;
	ParseIndexList ( tStmt.m_sIndex, dNames );
	if ( dNames.IsEmpty() )
	{
		sError.SetSprintf ( "no such table '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( sError.cstr() );
		return;
	}

	for ( const auto & sName : dNames )
		if ( !g_pLocalIndexes->Contains ( sName )
			&& g_pDistIndexes->Contains ( sName ) )
		{
			sError.SetSprintf ( "ALTER is only supported for local (not distributed) tables" );
			tOut.Error ( sError.cstr () );
			return;
		}

	for ( const auto &sName : dNames )
	{
		auto pServed = GetServed ( sName );
		if ( !pServed )
		{
			dErrors.Submit ( sName, nullptr, "unknown local table in ALTER request" );
			continue;
		}

		// cluster does not implement ALTER for now
		auto tCluster = IsPartOfCluster ( pServed );
		if ( tCluster )
		{
			dErrors.SubmitEx ( sName, nullptr, "is part of cluster %s, ALTER is not supported for tables in cluster", tCluster->cstr() );
			continue;
		}

		CSphString sAlterError;

		switch ( eAction )
		{
		case Alter_e::AddColumn:
		case Alter_e::ModifyColumn:
			{
				Threads::Coro::ScopedWriteTable_c tWriting { pServed->Locker() };
				if ( !tWriting.CanWrite() )
				{
					dErrors.SubmitEx ( sName, nullptr, "is locked, ALTER is not supported for locked tables" );
					break;
				}
				AddAttrToIndex ( tStmt, WIdx_c ( pServed ), sAlterError, eAction == Alter_e::ModifyColumn );
				break;
			}

		case Alter_e::DropColumn:
			{
				Threads::Coro::ScopedWriteTable_c tWriting { pServed->Locker() };
				if ( !tWriting.CanWrite() )
				{
					dErrors.SubmitEx ( sName, nullptr, "is locked, ALTER is not supported for locked tables" );
					break;
				}
				RemoveAttrFromIndex ( tStmt, WIdx_c ( pServed ), sAlterError );
				break;
			}

		case Alter_e::RebuildSI:
			WIdx_c(pServed)->AlterSI(sAlterError);
			break;

		case Alter_e::RebuildKNN:
			WIdx_c(pServed)->AlterKNN(sAlterError);
			break;

		case Alter_e::ApiKey:
			WIdx_c(pServed)->AlterApiKey ( tStmt.m_sAlterAttr, tStmt.m_sAlterOption, sAlterError );
			break;
		}

		if ( !sAlterError.IsEmpty() )
			dErrors.Submit ( sName, nullptr, sAlterError.cstr() );
	}

	if ( !dErrors.IsEmpty() )
	{
		StringBuilder_c sReport;
		dErrors.BuildReport ( sReport );
		tOut.Error ( sReport.cstr() );
		return;
	}
	tOut.Ok();
}


static bool PrepareReconfigure ( const char * szIndex, const CSphConfigSection & hIndex, CSphReconfigureSettings & tSettings, StrVec_t * pWarnings, CSphString & sError )
{
	std::unique_ptr<FilenameBuilder_i> pFilenameBuilder = CreateFilenameBuilder ( szIndex );

	{
		CSphString sWarning;
		tSettings.m_tTokenizer.Setup ( hIndex, sWarning );
		tSettings.m_tDict.Setup ( hIndex, pFilenameBuilder.get(), sWarning );
		tSettings.m_tFieldFilter.Setup ( hIndex, sWarning );
		tSettings.m_tMutableSettings.Load ( hIndex, false, nullptr );

		if ( pWarnings && !sWarning.IsEmpty() )
			pWarnings->Add(sWarning);
	}

	if ( !sphRTSchemaConfigure ( hIndex, tSettings.m_tSchema, tSettings.m_tIndex, pWarnings, sError, !tSettings.m_bChangeSchema, false ) )
	{
		sError.SetSprintf ( "failed to parse table '%s' schema, error: '%s'", szIndex, sError.cstr() );
		return false;
	}

	{
		CSphString sWarning;
		if ( !tSettings.m_tIndex.Setup ( hIndex, szIndex, sWarning, sError ) )
		{
			sError.SetSprintf ( "failed to parse table '%s' settings, error: '%s'", szIndex, sError.cstr() );
			return false;
		}

		if ( pWarnings && !sWarning.IsEmpty() )
			pWarnings->Add(sWarning);
	}

	if ( tSettings.m_tIndex.m_bIndexFieldLens && !AddFieldLens ( tSettings.m_tSchema, false, sError ) )
		return false;

	tSettings.m_tSchema.SetupFlags ( tSettings.m_tIndex, false, nullptr );

	return CheckStoredFields ( tSettings.m_tSchema, tSettings.m_tIndex, sError );
}


static bool PrepareReconfigure ( const char * szIndex, CSphReconfigureSettings & tSettings, StrVec_t * pWarnings, CSphString & sError )
{
	CSphConfig hCfg;
	auto [bChanged, dConfig] = FetchAndCheckIfChanged ( g_sConfigFile );
	if ( !ParseConfig ( &hCfg, g_sConfigFile, dConfig ) )
	{
		sError.SetSprintf ( "failed to parse config file '%s': %s; using previous settings", g_sConfigFile.cstr (), TlsMsg::szError() );
		return false;
	}

	if ( !hCfg.Exists ( "index" ) )
	{
		sError.SetSprintf ( "failed to find any table in config file '%s'; using previous settings", g_sConfigFile.cstr () );
		return false;
	}

	if ( !hCfg["index"].Exists ( szIndex ) )
	{
		sError.SetSprintf ( "failed to find table '%s' in config file '%s'; using previous settings", szIndex, g_sConfigFile.cstr () );
		return false;
	}

	return PrepareReconfigure ( szIndex, hCfg["index"][szIndex], tSettings, pWarnings, sError );
}

// ALTER RTINDEX/TABLE <idx> RECONFIGURE
static void HandleMysqlReconfigure ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	MEMORY ( MEM_SQL_ALTER );

	if ( IsConfigless() )
	{
		tOut.Error ( "ALTER RECONFIGURE is not supported in RT mode" );
		return;
	}

	const char * szIndex = tStmt.m_sIndex.cstr();
	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( !ServedDesc_t::IsMutable ( pServed ) )
	{
		tOut.ErrorEx ( "'%s' is absent, or does not support ALTER", szIndex );
		return;
	}

	CSphString sError;
	StrVec_t dWarnings;
	CSphReconfigureSettings tSettings;
	CSphReconfigureSetup tSetup;

	if ( !PrepareReconfigure ( szIndex, tSettings, &dWarnings, sError ) )
	{
		tOut.Error ( sError.cstr () );
		return;
	}

	WIdx_T<RtIndex_i*> pRT { pServed };
	if ( !pRT->IsSameSettings ( tSettings, tSetup, dWarnings, sError ) && sError.IsEmpty() )
	{
		if ( !pRT->Reconfigure ( tSetup ) )
		{
			sError.SetSprintf ( "table '%s': reconfigure failed; TABLE UNUSABLE (%s)", tStmt.m_sIndex.cstr(), pRT->GetLastError().cstr() );
			g_pLocalIndexes->Delete ( tStmt.m_sIndex );
		}
	}

	sWarning = ConcatWarnings ( dWarnings );
	if ( sError.IsEmpty() )
		tOut.Ok ( 0, dWarnings.GetLength() );
	else
		tOut.Error ( sError.cstr() );
}


static bool ApplyIndexKillList ( const CSphIndex * pIndex, CSphString & sWarning, CSphString & sError, bool bShowMessage = false );


// STMT_ALTER_KLIST_TARGET: ALTER TABLE index KILLLIST_TARGET = 'string'
static void HandleMysqlAlterKlist ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	MEMORY ( MEM_SQL_ALTER );

	CSphString sError;

	KillListTargets_c tNewTargets;
	if ( !tNewTargets.Parse ( tStmt.m_sAlterOption, tStmt.m_sIndex.cstr(), sError ) )
	{
		tOut.Error ( sError.cstr() );
		return;
	}

	auto pServed = GetServed ( tStmt.m_sIndex.cstr () );
	if ( !pServed )
	{
		if ( g_pDistIndexes->Contains ( tStmt.m_sIndex ) )
			sError.SetSprintf ( "ALTER is only supported for local (not distributed) tables" );
		else
			sError.SetSprintf ( "table '%s' not found", tStmt.m_sIndex.cstr () );
	}
	else if ( ServedDesc_t::IsMutable ( pServed ) )
		sError.SetSprintf ( "'%s' does not support ALTER (real-time or percolate)", tStmt.m_sIndex.cstr () );

	if ( !sError.IsEmpty () )
	{
		tOut.Error ( sError.cstr () );
		return;
	}

	WIdx_c pIdx { pServed };
	if ( !pIdx->AlterKillListTarget ( tNewTargets, sError ) )
	{
		tOut.Error ( sError.cstr() );
		return;
	}

	// apply killlist to new targets
	if ( !ApplyIndexKillList ( pIdx, sWarning, sError ) )
	{
		tOut.Error ( sError.cstr() );
		return;
	}

	if ( sError.IsEmpty() )
		tOut.Ok();
	else
		tOut.Error ( sError.cstr() );
}

// STMT_ALTER_INDEX_SETTINGS: ALTER TABLE index [ident = 'string']*
static void HandleMysqlAlterIndexSettings ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	MEMORY ( MEM_SQL_ALTER );

	CSphString sError;
	if ( !IsConfigless() )
	{
		sError = "ALTER TABLE requires data_dir to be set in the config file";
		tOut.Error ( sError.cstr() );
		return;
	}

	auto pServed = GetServed ( tStmt.m_sIndex.cstr() );
	if ( !pServed || pServed->m_eType != IndexType_e::RT )
	{
		tOut.ErrorEx ( "table '%s' is not found, or not real-time", tStmt.m_sIndex.cstr() );
		return;
	}

	// cluster does not implement ALTER for now
	auto tCluster = IsPartOfCluster ( pServed );
	if ( tCluster )
	{
		tOut.ErrorEx ( "table '%s' is part of cluster %s, ALTER is not supported for tables in cluster", tStmt.m_sIndex.cstr(), tCluster->cstr() );
		return;
	}

	WIdx_T<RtIndex_i*> pRtIndex { pServed };

	// get all table settings as a string
	CSphString sCreateTable = BuildCreateTable ( pRtIndex->GetName(), pRtIndex, pRtIndex->GetInternalSchema(), ExtFilesFormat_e::FILE );

	CSphVector<SqlStmt_t> dCreateTableStmts;
	if ( !ParseDdl ( FromStr ( sCreateTable ), dCreateTableStmts, sError ) )
	{
		tOut.Error ( sError.cstr() );
		return;
	}

	if ( dCreateTableStmts.GetLength()!=1 )
	{
		tOut.Error ( "Unable to alter table settings" );
		return;
	}

	int iSuffix = pRtIndex->GetChunkId();
	CSphString sIndexPath = GetPathOnly ( pRtIndex->GetFilebase() );

	// parse the options string to old-style config hash
	std::unique_ptr<IndexSettingsContainer_i> pContainer { CreateIndexSettingsContainer() };
	pContainer->Populate ( dCreateTableStmts[0].m_tCreateTable, false );

	// force override for old options options from alter should override currect options
	for ( const auto & i : tStmt.m_tCreateTable.m_dOpts )
	{
		pContainer->RemoveKeys ( i.m_sName );
	}

	// should be able to remove settings with the empty option or remove the prev options by the last empty option
	for ( const auto & i : tStmt.m_tCreateTable.m_dOpts )
	{
		pContainer->AddOption ( i.m_sName, i.m_sValue, true );
	}

	if ( !pContainer->CheckPaths() )
	{
		tOut.Error ( pContainer->GetError().cstr() );
		return;
	}

	// keep list of index files prior to alter
	StrVec_t dOldFiles;
	pRtIndex->GetIndexFiles ( dOldFiles, dOldFiles );

	if ( !pContainer->CopyExternalFiles ( sIndexPath, iSuffix ) )
	{
		tOut.Error ( pContainer->GetError().cstr () );
		return;
	}

	StrVec_t dWarnings;
	CSphReconfigureSettings tSettings;
	if ( !PrepareReconfigure ( tStmt.m_sIndex.cstr(), pContainer->AsCfg(), tSettings, &dWarnings, sError ) )
	{
		tOut.Error ( sError.cstr () );
		return;
	}

	CSphReconfigureSetup tSetup;
	bool bSame = pRtIndex->IsSameSettings ( tSettings, tSetup, dWarnings, sError );
	sWarning = ConcatWarnings(dWarnings);

	if ( !bSame && sError.IsEmpty() )
	{
		bool bOk = pRtIndex->Reconfigure(tSetup);

		if ( tSetup.m_tMutableSettings.IsSet ( MutableName_e::GLOBAL_IDF ) )
		{
			const CSphString sNewIDF = tSetup.m_tMutableSettings.m_sGlobalIDFPath;
			sph::PrereadGlobalIDF ( sNewIDF, sError );
			if ( pServed->m_sGlobalIDFPath != sNewIDF )
			{
				auto& tConstServed = *pServed;
				auto& tServed = const_cast<ServedIndex_c&> ( tConstServed );
				tServed.m_sGlobalIDFPath = sNewIDF;
				RotateGlobalIdf();
			}
		}

		if ( !bOk )
		{
			sError.SetSprintf ( "table '%s': alter failed; TABLE UNUSABLE (%s)", tStmt.m_sIndex.cstr(), pRtIndex->GetLastError().cstr() );
			g_pLocalIndexes->Delete ( tStmt.m_sIndex );
		}
	}

	if ( sError.IsEmpty() )
	{
		StrVec_t dNewFiles;
		pRtIndex->GetIndexFiles ( dNewFiles, dNewFiles );

		// all ok, delete old files
		RemoveOutdatedFiles ( dNewFiles, dOldFiles );
		pContainer->ResetCleanup();

		tOut.Ok ( 0, dWarnings.GetLength() );
	}
	else
		tOut.Error ( sError.cstr() );
}


// STMT_SHOW_PLAN: SHOW PLAN
static void HandleMysqlShowPlan ( RowBuffer_i & tOut, const QueryProfile_c & p, bool bMoreResultsFollow, bool bDot )
{
	tOut.HeadBegin ();
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
		auto pIdx = sphCreateIndexPhrase ( sIndex, pRes->m_sIndexPath );
		pIdx->m_iExpansionLimit = g_iExpansionLimit;
		pIdx->SetMutableSettings ( pRes->m_tSettings );
		pIdx->SetGlobalIDFPath ( pRes->m_sGlobalIDFPath );
		pIdx->SetCacheSize ( g_iMaxCachedDocs, g_iMaxCachedHits );
		pRes->SetIdx ( std::move ( pIdx ) );
	} else
		pRes->SetIdxAndStatsFrom ( *pSource );
	return pRes;
}

bool LockIndex ( const ServedIndex_c& tIdx, CSphIndex* pIdx, CSphString& sError )
{
	if ( !g_bOptNoLock && !pIdx->Lock() )
	{
		sError.SetSprintf ( "lock: %s", pIdx->GetLastError().cstr() );
		return false;
	}

	tIdx.UpdateMass();
	return true;
}

static bool LimitedRotateIndexMT ( ServedIndexRefPtr_c& pNewServed, const CSphString& sIndex, StrVec_t& dWarnings, CSphString& sError ) EXCLUDES ( MainThread );
static bool RotateIndexGreedy ( const ServedIndex_c& tServed, const char* szIndex, CSphString& sError ) REQUIRES ( tServed.m_pIndex->Locker() );

static bool SwitchoverIndexSeamless ( const cServedIndexRefPtr_c& pServed, const char* szIndex, const CSphString& sBase, const CSphString& sNewPath, StrVec_t& dWarnings, CSphString& sError ) EXCLUDES ( MainThread );
static bool SwitchoverIndexGreedy ( CSphIndex* pIdx, const char* szIndex, const CSphString& sBase, const CSphString& sNewPath, StrVec_t& dWarnings, CSphString& sError ) EXCLUDES ( MainThread );

static void HandleMysqlReloadIndex ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString & sWarning )
{
	// preflight check
	cServedIndexRefPtr_c pServed = GetServed ( tStmt.m_sIndex );
	if ( !pServed )
		return tOut.ErrorEx ( "unknown local table '%s'", tStmt.m_sIndex.cstr() );

	if ( ServedDesc_t::IsMutable ( pServed ) )
		return tOut.Error ( "can not reload real-time or percolate table" );

	assert ( pServed->m_eType == IndexType_e::PLAIN );

	CSphString sError;
	StrVec_t dWarnings;
	AT_SCOPE_EXIT ( [&sWarning, &dWarnings]() {
		if ( dWarnings.IsEmpty() )
			return;
		StringBuilder_c sWarn ( "; " );
		dWarnings.for_each ( [&sWarn] ( const auto& i ) { sWarn << i; } );
		sWarn.MoveTo ( sWarning );
	});

	if ( tStmt.m_iIntParam == 1 )
	{
		if ( tStmt.m_sStringParam.IsEmpty() )
			return tOut.Error ( "reload with switchover requires explicit new path to the index" );

		// here switchover=1 logic goes...
		if ( g_bSeamlessRotate )
		{
			if ( !SwitchoverIndexSeamless ( pServed, tStmt.m_sIndex.cstr(), pServed->m_sIndexPath, tStmt.m_sStringParam, dWarnings, sError ) )
				g_pLocalIndexes->Delete ( tStmt.m_sIndex ); // since it unusable - no sense just to disable it.
		} else
		{
			WIdx_c WLock { pServed };
			CSphIndex* pIdx = UnlockedHazardIdxFromServed ( *pServed );

			if ( !SwitchoverIndexGreedy ( pIdx, tStmt.m_sIndex.cstr(), pServed->m_sIndexPath, tStmt.m_sStringParam, dWarnings, sError ) )
				g_pLocalIndexes->Delete ( tStmt.m_sIndex ); // since it unusable - no sense just to disable it.
				// fixme! SwitchoverIndexGreedy does prealloc. Do we need to perform/signal preload also?
			else
				LockIndex ( *pServed, pIdx, sError );
		}
		if ( sError.IsEmpty() )
			return tOut.Ok();

		sphWarning ( "%s", sError.cstr() );
		return tOut.Error ( sError.cstr() );
	}
	assert ( tStmt.m_iIntParam!=1 );

	if ( !tStmt.m_sStringParam.IsEmpty () )
	{
		// try move files from arbitrary path to current index path before rotate, if needed.
		// fixme! what about concurrency? if 2 sessions simultaneously ask to rotate,
		// or if we have unapplied rotates from indexer - seems that it will garbage .new files?
		IndexFiles_c sIndexFiles ( pServed->m_sIndexPath );
		if ( !sIndexFiles.RelocateToNew ( tStmt.m_sStringParam ) )
			return tOut.Error ( sIndexFiles.ErrorMsg () );
	}

	if ( g_bSeamlessRotate )
	{
		ServedIndexRefPtr_c pNewServed = MakeCloneForRotation ( pServed, tStmt.m_sIndex );
		if ( !LimitedRotateIndexMT ( pNewServed, tStmt.m_sIndex, dWarnings, sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			return tOut.Error ( sError.cstr() );
		}
	} else {
		WIdx_c WLock { pServed };
		if ( !RotateIndexGreedy ( *pServed, tStmt.m_sIndex.cstr(), sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			tOut.Error ( sError.cstr() );
			g_pLocalIndexes->Delete ( tStmt.m_sIndex ); // since it unusable - no sense just to disable it.
			// fixme! RotateIndexGreedy does prealloc. Do we need to perform/signal preload also?
			return;
		}
	}

	tOut.Ok();
}

void HandleMysqlExplain ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, bool bDot )
{
	CSphString sProc ( tStmt.m_sCallProc );
	if ( sProc.ToLower()!="query" )
	{
		tOut.ErrorEx ( "no such explain procedure %s", tStmt.m_sCallProc.cstr () );
		return;
	}

	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( !pServed )
	{
		tOut.ErrorEx ( "unknown local table '%s'", tStmt.m_sIndex.cstr ());
		return;
	}

	TlsMsg::ResetErr (); // reset error
	auto dPlan = RIdx_c ( pServed )->ExplainQuery ( tStmt.m_tQuery.m_sQuery );
	if ( TlsMsg::HasErr ())
	{
		tOut.Error ( TlsMsg::szError ());
		return;
	}

	StringBuilder_c sRes;
	sph::RenderBsonPlan ( sRes, bson::MakeHandle ( dPlan ), bDot );

	tOut.HeadBegin ();
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
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	CSphString sError;

	if ( !IsConfigless() )
	{
		sError = "IMPORT TABLE requires data_dir to be set in the config file";
		tOut.Error ( sError.cstr() );
		return;
	}

	if ( IndexIsServed ( tStmt.m_sIndex ) )
	{
		sError.SetSprintf ( "table '%s' already exists", tStmt.m_sIndex.cstr() );
		tOut.Error ( sError.cstr() );
		return;
	}

	bool bPQ = false;
	StrVec_t dWarnings;
	if ( !CopyIndexFiles ( tStmt.m_sIndex, tStmt.m_sStringParam, bPQ, dWarnings, sError ) )
	{
		sError.SetSprintf ( "unable to import table '%s': %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( sError.cstr() );
		return;
	}

	if ( !AddExistingIndexConfigless ( tStmt.m_sIndex, bPQ ? IndexType_e::PERCOLATE : IndexType_e::RT, dWarnings, sError ) )
	{
		sError.SetSprintf ( "unable to import table '%s': %s", tStmt.m_sIndex.cstr(), sError.cstr() );
		tOut.Error ( sError.cstr() );
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
void HandleMysqlFreezeIndexes ( RowBuffer_i& tOut, const SqlStmt_t& tStmt, CSphString& sWarningOut )
{
	// search through specified local indexes
	StrVec_t dIndexes, dNonlockedIndexes, dIndexFiles;

	ParseIndexList ( tStmt.m_sIndex, dIndexes );
	for ( const auto& sIndex : dIndexes )
	{
		auto pIndex = GetServed ( sIndex );
		if ( !ServedDesc_t::IsMutable ( pIndex ) )
		{
			dNonlockedIndexes.Add ( sIndex );
			continue;
		}

		// here we get non-locked instance to avoid deadlock with update.
		// Deadlock happens by this sequence:
		// 1. Update protects index from bein frozen.
		// 2. We lock index here.
		// 3. We wait until protection released.
		// 4. Update tries to w-lock the index, locked by us.
		auto * pRt = static_cast<RtIndex_i *> ( UnlockedHazardIdxFromServed ( *pIndex ) );
		pRt->LockFileState ( dIndexFiles );
	}

	int iWarnings=0;
	if ( !dNonlockedIndexes.IsEmpty() )
	{
		StringBuilder_c sWarning;
		sWarning << "Some tables are not suitable for freezing: ";
		sWarning.StartBlock();
		dNonlockedIndexes.for_each ( [&sWarning] ( const auto& sValue ) { sWarning << sValue; } );
		sWarning.FinishBlocks ();
		sWarning.MoveTo ( sWarningOut );
		++iWarnings;
	}

	CheckLike tSelector { tStmt.m_sStringParam.cstr() };

	tOut.HeadBegin ();
	tOut.HeadColumn ( "file" );
	tOut.HeadColumn ( "normalized" );
	tOut.HeadEnd();

	for ( const auto& sFile : dIndexFiles )
	{
		if ( !tSelector.Match ( sFile.cstr() ) )
			continue;

		tOut.PutString ( sFile );
		tOut.PutString ( RealPath ( sFile ) );
		if ( !tOut.Commit() )
			return;
	};
	tOut.Eof ( false, iWarnings );
}

void HandleMysqlUnfreezeIndexes ( RowBuffer_i& tOut, const CSphString& sIndexes )
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

void HandleMysqlKill ( RowBuffer_i& tOut, int iKill )
{
	int iKilled = 0;
	IterateTasks ( [&iKilled, iKill] ( ClientTaskInfo_t* pTask ) {
		if ( pTask && pTask->GetConnID() == iKill && !pTask->GetKilled())
		{
			pTask->SetKilled(true);
			++iKilled;
		}
	} );

	if ( !iKilled )
	{
		tOut.ErrorEx ( SphSprintf ( "Unknown connection id: %d", iKill ).cstr(), EMYSQL_ERR::NO_SUCH_THREAD );
	} else
	{
		tOut.Ok ( iKilled );
	}
}


void HandleMysqlShowLocks ( RowBuffer_i & tOut )
{
	tOut.HeadBegin ();
	tOut.HeadColumn ( "Type" );
	tOut.HeadColumn ( "Name" );
	tOut.HeadColumn ( "Lock Type" );
	tOut.HeadColumn ( "Additional Info" );
	if ( !tOut.HeadEnd () )
		return;

	auto fnLine = [&tOut](const NamedIndexType_t& dPair, int iLocks, const char* szType ) noexcept -> bool
	{
		tOut.PutString ( GetIndexTypeName ( dPair.m_eType ) );
		tOut.PutString ( dPair.m_sName );
		tOut.PutString ( szType );
		tOut.PutStringf ( "Count: %d", iLocks );
		return tOut.Commit ();
	};

	// collect local, rt, percolate
	auto dIndexes = GetAllServedIndexes ();
	for ( auto & dPair: dIndexes )
	{
		auto pIndex = GetServed ( dPair.m_sName );
		if ( ServedDesc_t::IsMutable ( pIndex ) )
		{
			RIdx_T<RtIndex_i *> pRt { pIndex };
			const int iLocks = pRt->GetNumOfLocks ();
			if ( iLocks>0 && !fnLine ( dPair, iLocks, "freeze" ) )
				return;
		}
		if ( ServedDesc_t::IsLocal ( pIndex ) )
		{
			const int iRLocks = pIndex->GetReadLocks();
			if ( iRLocks>0 && !fnLine ( dPair, iRLocks, "read" ) )
				return;
		}
	}

	tOut.Eof ( false );
}

void UnlockTables(ClientSession_c* pSess)
{
	assert (pSess);
	for ( const auto& sIndex : pSess->m_dLockedTables )
	{
		auto pLocal = GetServed ( sIndex );
		if ( pLocal && pLocal->UnlockRead() )
			sphLogDebug ( "Unlocked %s", sIndex.cstr() );
	}
	pSess->m_dLockedTables.Reset();
}

void HandleMysqlLockTables ( RowBuffer_i & tOut, const SqlStmt_t & tStmt, CSphString& sWarningOut )
{
	StrVec_t dIndexes;
	bool bHasWrite = false;
	tStmt.m_dInsertValues.for_each ( [&] (const SqlInsert_t& tVal) {
		if ( tVal.m_iType>10 )
			bHasWrite = true;
		else
			dIndexes.Add(tVal.m_sVal);
	});

	sWarningOut = bHasWrite ? "Write lock is not implemented." : "";

	if ( session::GetProto()!=Proto_e::MYSQL41 )
	{
		tOut.Error ( "Locking works only for mysql session." );
		return;
	}

	auto* pSess = session::GetClientSession();
	assert (pSess);

	// by spec, any lock first unlock all the tables.
	UnlockTables(pSess);

	for ( const auto& sIndex : dIndexes )
	{
		auto pLocal = GetServed ( sIndex );
		if ( !(pLocal && ServedDesc_t::IsLocal ( pLocal ) ) )
		{
			tOut.ErrorEx ( "Table %s absent, or not suitable for lock", sIndex.cstr() );
			return;
		}
		if ( ServedDesc_t::IsCluster ( pLocal ) )
		{
			tOut.ErrorEx ( "Table %s is member of a cluster; not suitable for lock", sIndex.cstr() );
			return;
		}

		pLocal->LockRead();
		pSess->m_dLockedTables.Add(sIndex);
		sphLogDebug ( "Locked %s", sIndex.cstr() );
	}

	tOut.Ok ( 0, bHasWrite ? 1 : 0 );
}

void HandleMysqlUnlockTables ( RowBuffer_i & tOut )
{
	if ( session::GetProto()!=Proto_e::MYSQL41 )
	{
		tOut.Error ( "Locking works only for mysql session." );
		return;
	}

	UnlockTables(session::GetClientSession());

	tOut.Ok ( 0 );
}


RtAccum_t* CSphSessionAccum::GetAcc ( RtIndex_i* pIndex, CSphString& sError )
{
	assert ( pIndex );
	if ( !m_tAcc )
		m_tAcc.emplace();

	if ( !pIndex->BindAccum ( &m_tAcc.value(), &sError ) )
		return nullptr;

	return &m_tAcc.value();
}

RtAccum_t* CSphSessionAccum::GetAcc()
{
	if ( !m_tAcc )
		m_tAcc.emplace();
	return &m_tAcc.value();
}

RtIndex_i * CSphSessionAccum::GetIndex ()
{
	if ( !m_tAcc )
		return nullptr;
	return m_tAcc->GetIndex();
}

static bool FixupFederatedQuery ( ESphCollation eCollation, CSphVector<SqlStmt_t> & dStmt, CSphString & sError, CSphString & sFederatedQuery );

void ClientSession_c::FreezeLastMeta()
{
	m_tLastMeta = CSphQueryResultMeta();
	m_tLastMeta.m_sError = m_sError;
	m_tLastMeta.m_sWarning = "";
}

ClientSession_c::~ClientSession_c ()
{
	UnlockTables(this);
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

	// fixme! That is dirty hack, should be handled another way
	constexpr Str_t mysqldump_8_0_39_30_hack {FROMS("SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES WHERE table_schema = 'performance_schema' AND table_name = 'session_variables'")};
	if ( StrEq ( mysqldump_8_0_39_30_hack, sQuery) )
		return tOut.DataTableOneline ( "count(*)" );

	// parse SQL query
	if ( tSess.IsProfile() )
		m_tProfile.Switch ( SPH_QSTATE_SQL_PARSE );

	m_sError = "";

	CSphVector<SqlStmt_t> dStmt;
	bool bParsedOK = sphParseSqlQuery ( sQuery, dStmt, m_sError, tSess.GetCollation () );

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
	FixupSystemTableName ( pStmt );
	assert ( !bParsedOK || pStmt );

	myinfo::SetCommand ( SqlStmt2Str(eStmt) );
	AT_SCOPE_EXIT ( []() { myinfo::SetCommandDone(); } );

	LogStmtGuard_c tLogGuard ( sQuery, eStmt, dStmt.GetLength()>1 );

	if ( bParsedOK && m_bFederatedUser )
	{
		if ( !FixupFederatedQuery ( tSess.GetCollation (), dStmt,  m_sError, m_sFederatedQuery ) )
		{
			FreezeLastMeta();
			tOut.Error ( m_sError.cstr() );
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
		if ( m_sError.IsEmpty() )
			tOut.Ok ( 0 );
		else {
			FreezeLastMeta();
			tOut.Error ( m_sError.cstr() );
		}
		return true;

	case STMT_SELECT:
		{
			MEMORY ( MEM_SQL_SELECT );

			if ( ClusterFlavour () )
			{
				auto dParts = sphSplit ( pStmt->m_sIndex.cstr (), ":" );
				if ( dParts.GetLength ()>1 )
				{
					pStmt->m_sCluster = dParts[0];
					pStmt->m_tQuery.m_sIndexes = pStmt->m_sIndex = dParts[1];
				}
			}

			StatCountCommand ( SEARCHD_COMMAND_SEARCH );
			SearchHandler_c tHandler ( 1, sphCreatePlainQueryParser(), QUERY_SQL, true );
			// no log for search queries from the buddy in the info verbosity
			if ( session::IsQueryLogDisabled() )
				dStmt.Begin()->m_tQuery.m_uDebugFlags |= QUERY_DEBUG_NO_LOG;
			tHandler.SetQuery ( 0, dStmt.Begin()->m_tQuery, std::move ( dStmt.Begin()->m_pTableFunc ) );
			tHandler.SetJoinQueryOptions ( 0, dStmt.Begin()->m_tJoinQueryOptions );
			tHandler.m_pStmt = pStmt;

			if ( tSess.IsProfile() )
				tHandler.SetProfile ( &m_tProfile );
			if ( m_bFederatedUser )
				tHandler.SetFederatedUser();

			tOut.SomethingWasSent();
			if ( HandleMysqlSelect ( tOut, tHandler ) && !tOut.SomethingWasSent() )
			{
				// query just completed ok; reset out error message
				m_sError = "";
				AggrResult_t & tLast = tHandler.m_dAggrResults.Last();
				auto uMatches = SendMysqlSelectResult ( tOut, tLast, false, m_bFederatedUser, &m_sFederatedQuery, ( tSess.IsProfile() ? &m_tProfile : nullptr ) );
				gStats().AddDeltaDetailed ( SearchdStats_t::eSearch, uMatches, (uint64_t)tLast.m_iQueryTime * 1000 );
			}

			// save meta for SHOW META (profile is saved elsewhere)
			m_tLastMeta = tHandler.m_dAggrResults.Last();
			if ( pStmt && pStmt->m_eStmt==STMT_SELECT )
				FormatScrollSettings ( tHandler.m_dAggrResults.Last(), pStmt->m_tQuery, m_tLastMeta.m_sScroll );
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

	case STMT_SHOW_SCROLL:
		HandleMysqlShowScroll ( tOut, *pStmt, m_tLastMeta, false );
		return true;

	case STMT_INSERT:
	case STMT_REPLACE:
		{
			StmtErrorReporter_c tErrorReporter ( tOut );
			MaybeFixupIndexNameFromMysqldump ( *pStmt );
			sphHandleMysqlInsert ( tErrorReporter, *pStmt );
			return true;
		}

	case STMT_DELETE:
		{
			StmtErrorReporter_c tErrorReporter ( tOut );
			sphHandleMysqlDelete ( tErrorReporter, *pStmt, sQuery );
			return true;
		}

	case STMT_SET:
		StatCountCommand ( SEARCHD_COMMAND_UVAR );
		HandleMysqlSet ( tOut, *pStmt, m_tAcc );
		return false;

	case STMT_BEGIN:
		{
			StmtErrorReporter_c tErrorReporter ( tOut );
			sphHandleMysqlBegin ( tErrorReporter, sQuery );
			return true;
		}

	case STMT_COMMIT:
	case STMT_ROLLBACK:
		{
			StmtErrorReporter_c tErrorReporter ( tOut );
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
		} else if ( pStmt->m_sCallProc=="UUID_SHORT" )
		{
			HandleMysqlCallUuid ( tOut, *pStmt );
		} else
		{
			m_sError.SetSprintf ( "no such built-in procedure %s", pStmt->m_sCallProc.cstr() );
			tOut.Error ( m_sError.cstr() );
		}
		return true;

	case STMT_DESCRIBE:
		HandleCmdDescribe ( tOut, pStmt );
		return true;

	case STMT_SHOW_TABLES:
		HandleShowTables ( tOut, pStmt );
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
		m_tLastMeta.m_sWarning = "";
		HandleMysqlDropTable ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_DROP_CACHE:
		m_tLastMeta = CSphQueryResultMeta();
		HandleMysqlDropCache ( tOut );
		return true;

	case STMT_SHOW_CREATE_TABLE:
		HandleMysqlShowCreateTable ( tOut, *pStmt );
		return true;

	case STMT_UPDATE:
		{
			StmtErrorReporter_c tErrorReporter ( tOut );
			sphHandleMysqlUpdate ( tErrorReporter, *pStmt, sQuery );
			return true;
		}

	case STMT_DUMMY:
		if ( !pStmt->m_dInsertSchema.IsEmpty() )
		{
			// empty with schema (something like 'show triggers' expects schema even if there are no results)
			tOut.HeadOfStrings ( pStmt->m_dInsertSchema );
			tOut.Eof();
			return true;
		}
		tOut.Ok();
		return true;

	case STMT_CREATE_FUNCTION:
		if ( !sphPluginCreate ( pStmt->m_sUdfLib.cstr(), PLUGIN_FUNCTION, pStmt->m_sUdfName.cstr(), pStmt->m_eUdfType, m_sError ) )
			tOut.Error ( m_sError.cstr() );
		else
			tOut.Ok();
		SphinxqlStateFlush ();
		return true;

	case STMT_DROP_FUNCTION:
		if ( !sphPluginDrop ( PLUGIN_FUNCTION, pStmt->m_sUdfName.cstr(), m_sError ) )
			tOut.Error ( m_sError.cstr() );
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
				tOut.ErrorEx ( "unknown plugin type '%s'", pStmt->m_sStringParam.cstr() );
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
				tOut.Error ( m_sError.cstr() );
			else
				tOut.Ok();
			SphinxqlStateFlush ();
			return true;
		}

	case STMT_RELOAD_PLUGINS:
		if ( sphPluginReload ( pStmt->m_sUdfLib.cstr(), m_sError ) )
			tOut.Ok();
		else
			tOut.Error ( m_sError.cstr() );
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
		m_tLastMeta.m_sWarning = "";
		HandleMysqlTruncate ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_OPTIMIZE_INDEX:
		HandleMysqlOptimize ( tOut, *pStmt );
		return true;

	case STMT_SELECT_COLUMNS:
		HandleMysqlSelectColumns ( tOut, *pStmt, this );
		return true;

	case STMT_SHOW_COLLATION:
		HandleMysqlShowCollations ( tOut );
		return true;

	case STMT_SHOW_CHARACTER_SET:
		HandleMysqlShowCharacterSet ( tOut );
		return true;

	case STMT_SHOW_INDEX_STATUS:
		HandleMysqlShowIndexStatus ( tOut, *pStmt );
		return true;

	case STMT_SHOW_FEDERATED_INDEX_STATUS:
		HandleMysqlShowFederatedIndexStatus ( tOut, *pStmt );
		return true;

	case STMT_SHOW_INDEX_SETTINGS:
		HandleMysqlShowIndexSettings ( tOut, *pStmt );
		return true;

	case STMT_SHOW_PROFILE:
		HandleMysqlShowProfile ( tOut, m_tLastProfile, false );
		return false; // do not profile this call, keep last query profile

	case STMT_ALTER_ADD:
		HandleMysqlAlter ( tOut, *pStmt, Alter_e::AddColumn );
		return true;

	case STMT_ALTER_MODIFY:
		HandleMysqlAlter ( tOut, *pStmt, Alter_e::ModifyColumn );
		return true;

	case STMT_ALTER_DROP:
		HandleMysqlAlter ( tOut, *pStmt, Alter_e::DropColumn );
		return true;

	case STMT_ALTER_REBUILD_SI:
		HandleMysqlAlter ( tOut, *pStmt, Alter_e::RebuildSI );
		return true;

	case STMT_ALTER_REBUILD_KNN:
		HandleMysqlAlter ( tOut, *pStmt, Alter_e::RebuildKNN );
		return true;

	case STMT_ALTER_EMBEDDINGS_API_KEY:
		HandleMysqlAlter ( tOut, *pStmt, Alter_e::ApiKey );
		return true;

	case STMT_SHOW_PLAN:
		HandleMysqlShowPlan ( tOut, m_tLastProfile, false, ::IsDot ( *pStmt ) );
		return false; // do not profile this call, keep last query profile

	case STMT_SHOW_DATABASES:
		HandleMysqlShowDatabases ( tOut, *pStmt );
		return true;

	case STMT_SHOW_PLUGINS:
		HandleMysqlShowPlugins ( tOut, *pStmt );
		return true;

	case STMT_SHOW_THREADS:
		HandleShowThreads ( tOut, pStmt );
		return true;

	case STMT_ALTER_RECONFIGURE: // ALTER RTINDEX/TABLE <idx> RECONFIGURE
		FreezeLastMeta();
		HandleMysqlReconfigure ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_ALTER_KLIST_TARGET: // ALTER TABLE <idx> KILLLIST_TARGET = 'the string'
		FreezeLastMeta();
		HandleMysqlAlterKlist ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_ALTER_INDEX_SETTINGS: // ALTER TABLE <idx> create_table_option_list
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
		HandleMysqlDebug ( tOut, pStmt->m_pDebugCmd.get(), m_tLastProfile );
		return false; // do not profile this call, keep last query profile

	case STMT_JOIN_CLUSTER:
		if ( ClusterJoin ( pStmt->m_sCluster, pStmt->m_dCallOptNames, pStmt->m_dCallOptValues, pStmt->m_bClusterUpdateNodes ) )
			tOut.Ok();
		else
		{
			TlsMsg::MoveError ( m_sError );
			tOut.Error ( m_sError.cstr() );
		}
		return true;
	case STMT_CLUSTER_CREATE:
		if ( ClusterCreate ( pStmt->m_sCluster, pStmt->m_dCallOptNames, pStmt->m_dCallOptValues ) )
			tOut.Ok();
		else
		{
			TlsMsg::MoveError ( m_sError );
			tOut.Error ( m_sError.cstr() );
		}
		return true;

	case STMT_CLUSTER_DELETE:
		m_tLastMeta = CSphQueryResultMeta();
		if ( GloballyDeleteCluster ( pStmt->m_sIndex, m_tLastMeta.m_sError ) )
			tOut.Ok ( 0, m_tLastMeta.m_sWarning.IsEmpty() ? 0 : 1 );
		else
			tOut.Error ( m_tLastMeta.m_sError.cstr() );
		return true;

	case STMT_CLUSTER_ALTER_ADD:
	case STMT_CLUSTER_ALTER_DROP:
		m_tLastMeta = CSphQueryResultMeta();
		if ( ClusterAlter ( pStmt->m_sCluster, pStmt->m_dStringSubkeys, ( eStmt==STMT_CLUSTER_ALTER_ADD ), m_tLastMeta.m_sError ) )
			tOut.Ok ( 0, m_tLastMeta.m_sWarning.IsEmpty() ? 0 : 1 );
		else
			tOut.Error ( m_tLastMeta.m_sError.cstr() );
		return true;

	case STMT_CLUSTER_ALTER_UPDATE:
		m_tLastMeta = CSphQueryResultMeta();
		if ( ClusterAlterUpdate ( pStmt->m_sCluster, pStmt->m_sSetName, m_tLastMeta.m_sError ) )
			tOut.Ok();
		else
			tOut.Error ( m_tLastMeta.m_sError.cstr() );
		return true;

	case STMT_EXPLAIN:
		HandleMysqlExplain ( tOut, *pStmt, IsDot ( *pStmt ) );
		return true;

	case STMT_IMPORT_TABLE:
		FreezeLastMeta();
		HandleMysqlImportTable ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_FREEZE:
		HandleMysqlFreezeIndexes ( tOut, *pStmt, m_tLastMeta.m_sWarning);
		return true;

	case STMT_UNFREEZE:
		HandleMysqlUnfreezeIndexes ( tOut, pStmt->m_sIndex );
		return true;

	case STMT_SHOW_SETTINGS:
		{
			ScRL_t dRotateConfigMutexRlocked { g_tRotateConfigMutex };
			HandleMysqlShowSettings ( g_hCfg, tOut );
		}
		return true;

	case STMT_KILL:
		HandleMysqlKill ( tOut, pStmt->m_iIntParam );
		return true;

	case STMT_SHOW_LOCKS:
		HandleMysqlShowLocks ( tOut );
		return true;

	case STMT_SHOW_TABLE_INDEXES:
		HandleMysqlShowTableIndexes ( tOut, *pStmt );
		return true;

	case STMT_LOCK_TABLES:
		HandleMysqlLockTables ( tOut, *pStmt, m_tLastMeta.m_sWarning );
		return true;

	case STMT_UNLOCK_TABLES:
		HandleMysqlUnlockTables ( tOut );
		return true;

	default:
		m_sError.SetSprintf ( "internal error: unhandled statement type (value=%d)", eStmt );
		tOut.Error ( m_sError.cstr() );
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

void session::SetOptimizeById ( bool bOptimizeById )
{
	GetClientSession()->m_bOptimizeById = bOptimizeById;
}

bool session::GetOptimizeById()
{
	return GetClientSession()->m_bOptimizeById;
}

void session::SetDeprecatedEOF ( bool bDeprecatedEOF )
{
	GetClientSession()->m_bDeprecatedEOF = bDeprecatedEOF;
}

bool session::GetDeprecatedEOF()
{
	return GetClientSession()->m_bDeprecatedEOF;
}

bool session::Execute ( Str_t sQuery, RowBuffer_i& tOut )
{
	return GetClientSession()->Execute ( sQuery, tOut );
}

void session::SetFederatedUser ()
{
	GetClientSession()->m_bFederatedUser = true;
}

void session::SetUser ( const CSphString & sUser )
{
	GetClientSession()->m_sUser = sUser;
}

void session::SetCurrentDbName ( CSphString sDb )
{
	GetClientSession()->m_sCurrentDbName = std::move(sDb);
}

const char* session::GetCurrentDbName ()
{
	return GetClientSession() ? GetClientSession()->m_sCurrentDbName.cstr() : nullptr;
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
void HandleCommandSphinxql ( GenericOutputBuffer_c & tOut, WORD uVer, InputBuffer_c & tReq ) REQUIRES (HandlerThread)
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

	OptionsHash_t hOptions;
	hOptions.AddUnique ( "endpoint" ) = sEndpoint;
	if ( uVer>=0x102 )
	{
		hOptions.AddUnique ( "raw_query" ) = tReq.GetString ();
		hOptions.AddUnique ( "full_url" ) = tReq.GetString ();
	}
	
	CSphVector<BYTE> dResult;
	ProcessHttpJsonQuery ( sCommand, hOptions, dResult );

	auto tReply = APIAnswer ( tOut, VER_COMMAND_JSON );
	tOut.SendString ( sEndpoint.cstr() );
	tOut.SendArray ( dResult );
}


void StatCountCommand ( SearchdCommand_e eCmd )
{
	gStats().Inc ( eCmd );
}

// fixme! move federated stuff to another parser and remove all kind of 'fixups'
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

	if ( tStmt.m_eStmt==STMT_SHOW_FEDERATED_INDEX_STATUS )
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
	bool bParsedOK = sphParseSqlQuery ( FromStr ( sRealQuery ), dRealStmt, sError, eCollation );
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
	for ( CSphQueryItem & tItem : tRealQuery.m_dRefItems )
	{
		int * pRealItem = hItems ( tItem.m_sAlias );
		if ( !pRealItem )
		{
			tRealQuery.m_dItems.Add ( tItem );
		} else
		{
			// change original item name to match on minimize result set
			CSphQueryItem & tRealItem = tRealQuery.m_dItems[*pRealItem];
			if ( tItem.m_sExpr!=tRealItem.m_sExpr )
				tItem.m_sExpr = tRealItem.m_sExpr;
		}
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
		sphInfo ( "applying killlist of table '%s'", pIndex->GetName() );

	for ( const auto & tIndex : tTargets.m_dTargets )
	{
		// just in case; otherwise we'll be rlocking an already rlocked index
		if ( tIndex.m_sIndex==pIndex->GetName() )
		{
			sWarning.SetSprintf ( "table '%s': applying killlist to itself", tIndex.m_sIndex.cstr() );
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
			sWarning.SetSprintf ( "table '%s' from killlist_target not found", tIndex.m_sIndex.cstr() );
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
					pIndexWithKillList->KillExistingDocids ( pKillListTarget );
			}
	}

	return true;
}

bool PreloadKlistTarget ( const CSphString& sBase, RotateFrom_e eFrom, StrVec_t & dKlistTarget )
{
	switch ( eFrom )
	{
	case RotateFrom_e::NEW:
	case RotateFrom_e::NEW_AND_OLD:
		return IndexFiles_c ( sBase ).ReadKlistTargets ( dKlistTarget, ".new" );

	case RotateFrom_e::REENABLE:
		return IndexFiles_c ( sBase ).ReadKlistTargets ( dKlistTarget );

	default:
		return false;
	}
}

static bool ApplyOthersKillListsToMe ( CSphIndex* pIndex, const char* szIndex, CSphString& sError )
{
	sphLogDebug ( "rotating table '%s': applying other tables killlists", szIndex );

	// apply other indexes' killlists to THIS index
	if ( !ApplyKillListsTo ( pIndex, sError ) )
	{
		sphWarning ( "rotating table '%s': %s", szIndex, sError.cstr() );
		return false;
	}

	sphLogDebug ( "rotating table '%s': applying other tables killlists... DONE", szIndex );
	return true;
}

static bool ApplyMyKillListsToOthers ( const CSphIndex* pIndex, const char* szIndex, CSphString& sError )
{
	sphLogDebug ( "rotating table '%s': apply killlist from this table to other tables (killlist_target)", szIndex );

	// apply killlist from this index to other indexes (killlist_target)
	// if this fails, only show a warning
	CSphString sWarning;
	if ( !ApplyIndexKillList ( pIndex, sWarning, sError ) )
	{
		return false;
		sphWarning ( "rotating table '%s': %s", szIndex, sError.cstr() );
	}

	if ( sWarning.Length() )
		sphWarning ( "rotating table '%s': %s", szIndex, sWarning.cstr() );

	sphLogDebug ( "rotating table '%s': apply killlist from this table to other tables (killlist_target)... DONE", szIndex );
	return true;
}

bool ApplyKilllistsMyAndToMe ( CSphIndex* pIdx, const char* szIndex, CSphString& sError )
{
	return ApplyOthersKillListsToMe ( pIdx, szIndex, sError ) && ApplyMyKillListsToOthers ( pIdx, szIndex, sError );
}

// tServed here might be one of:
// 1. Not yet served, and with .new ext. Need to be rotated, then loaded from scratch
// 2. Not yet served, need to be loaded from scratch
// 3. Served, but with now with .new. Need to be rotated, then loaded and need to be rotated
bool RotateIndexGreedy ( const ServedIndex_c& tServed, const char* szIndex, CSphString& sError )
{
	assert ( tServed.m_eType == IndexType_e::PLAIN );
	sphLogDebug ( "RotateIndexGreedy for '%s' invoked", szIndex );

	//////////////////
	/// bool RotateIndexFilesGreedy ( const ServedDesc_t& tServed, const char* szIndex, CSphString& sError )
	//////////////////

	CSphIndex* pIdx = UnlockedHazardIdxFromServed ( tServed ); // it should be locked, if necessary, before
	auto sIndexPath = tServed.m_sIndexPath;
	if ( pIdx )
		sIndexPath = pIdx->GetFilebase();

	CheckIndexRotate_c tCheck ( sIndexPath );
	if ( tCheck.NothingToRotate() )
		return false;

	IndexFiles_c dServedFiles ( sIndexPath, szIndex );
	IndexFiles_c dFreshFiles ( dServedFiles.MakePath ( tCheck.RotateFromNew() ? ".new" : "" ), szIndex );

//	if ( !dFreshFiles.CheckHeader() )... // no need to check, since CheckIndexRotate_c already did it.

	if ( !dFreshFiles.HasAllFiles() )
	{
		sphWarning ( "rotating table '%s': unreadable: %s; abort rotation", szIndex, strerrorm ( errno ) );
		return false;
	}

	bool bHasOldServedFiles = dServedFiles.HasAllFiles();
	std::optional<ActionSequence_c> tActions;

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
	StrVec_t dWarnings;
	if ( !pIdx->Prealloc ( g_bStripPath, nullptr, dWarnings ) )
	{
		sphWarning ( "rotating table '%s': .new preload failed: %s", szIndex, pIdx->GetLastError().cstr() );
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
				sError.SetSprintf ( "rotating table '%s': .new preload failed; ROLLBACK FAILED; TABLE UNUSABLE", szIndex );
				return false;
			}
		}
	}

	assert ( pIdx->GetTokenizer() && pIdx->GetDictionary() );

	for ( const auto& i : dWarnings )
		sphWarning ( "rotating table '%s': %s", szIndex, i.cstr() );

	if ( !pIdx->GetLastWarning().IsEmpty() )
		sphWarning ( "rotating table '%s': %s", szIndex, pIdx->GetLastWarning().cstr() );

	// unlink .old
	if ( bHasOldServedFiles )
		dServedFiles.Unlink (".old");

	// finalize
	if ( !ApplyKilllistsMyAndToMe ( pIdx, szIndex, sError ) )
		return false;

	// uff. all done
	sphInfo ( "rotating table '%s': success", szIndex );
	return true;
}

void DumpMemStat ()
{
#if SPH_ALLOCS_PROFILER
	sphMemStatDump ( GetDaemonLogFD() );
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
		sphSeek ( GetDaemonLogFD(), 0, SEEK_END );
		sphAllocsDump ( GetDaemonLogFD(), iHeadCheckpoint );

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

// helper to switch index to another path
// (used both, greedy and seamless)
class SwitchOver_c : public ISphNoncopyable
{
	const char *		m_szIndex;
	const CSphString &	m_sBase;
	const CSphString &	m_sNewPath;
	StrVec_t &			m_dWarnings;
	CSphString &		m_sError;
	bool 				m_bHaveOldFiles = false;
	CSphIndex*			m_pIdx = nullptr;
	CSphString			m_sOldPath;
	std::optional<ActionSequence_c> m_tActions;
	std::optional<IndexFiles_c> m_tFreshCurFiles;

public:
	SwitchOver_c ( const char* szIndex, const CSphString& sBase, const CSphString& sNewPath, StrVec_t& dWarnings, CSphString& sError )
	: m_szIndex { szIndex }
	, m_sBase { sBase }
	, m_sNewPath { sNewPath }
	, m_dWarnings { dWarnings }
	, m_sError { sError }
	{
		m_sError = "";
	}

	~SwitchOver_c()
	{
		for ( const auto& i : m_dWarnings )
			sphWarning ( "switchover table '%s': %s", m_szIndex, i.cstr() );

		if ( m_pIdx && !m_pIdx->GetLastWarning().IsEmpty() )
			sphWarning ( "switchover table '%s': %s", m_szIndex, m_pIdx->GetLastWarning().cstr() );
	}

	void SetIdx( CSphIndex* pIdx ) noexcept
	{
		assert ( pIdx );
		m_pIdx = pIdx;
		m_sOldPath = pIdx->GetFilebase();
	}

	bool CheckSameBase () const noexcept
	{
		assert ( m_pIdx );
		if ( m_sNewPath != m_sOldPath )
			return true;

		m_sError.SetSprintf ( "reload path should be different from current path" );
		return false;
	}

	bool RotateFiles()
	{
		CheckIndexRotate_c tCheckNew ( m_sNewPath );
		if ( tCheckNew.NothingToRotate() )
		{
			m_sError.SetSprintf ( "No index found by given %s path, do nothing.", m_sNewPath.cstr() );
			return false;
		}

		bool bHaveAllFreshNewFiles = tCheckNew.RotateFromNew() && IndexFiles_c { IndexFiles_c::MakePath ( ".new", m_sNewPath ), m_szIndex }.HasAllFiles();
		if ( tCheckNew.RotateReenable() )
		{
			IndexFiles_c tCur { IndexFiles_c::MakePath ( "", m_sNewPath ), m_szIndex };
			if ( tCur.HasAllFiles() )
				m_tFreshCurFiles.emplace ( std::move ( tCur ) );
		}
		auto bHaveAllFreshCurFiles = (bool)m_tFreshCurFiles;

		if ( !bHaveAllFreshNewFiles && !bHaveAllFreshCurFiles )
		{
			m_sError.SetSprintf ( "rotating table '%s': unreadable: %s; abort rotation", m_szIndex, strerrorm ( errno ) );
			return false;
		}

		if ( bHaveAllFreshNewFiles )
		{
			m_tActions.emplace();
			if ( bHaveAllFreshCurFiles )
			{
				m_tActions->Defer ( RenameFiles ( *m_tFreshCurFiles, "", ".old" ) );
				m_bHaveOldFiles = true;
			}
			m_tActions->Defer ( RenameFiles ( *m_tFreshCurFiles, ".new", "" ) );

			// do files rotation
			if ( !m_tActions->RunDefers() )
			{
				bool bFatal;
				std::tie ( m_sError, bFatal ) = m_tActions->GetError();
				m_sError.SetSprintf ( "SwitchoverIndexGreedy error: %s", m_sError.cstr() );
				return false;
			}
		}
		return true;
	}

	bool UnRotateFiles()
	{
		if ( m_tActions && !m_tActions->UnRunDefers() )
		{
			auto [sError, bFatal] = m_tActions->GetError();
			m_sError.SetSprintf ( "UnRotateFiles error: %s", sError.cstr() );
			return false;
		}
		m_bHaveOldFiles = false;
		return true;
	}

	bool Finalize()
	{
		if ( !WriteLinkFile ( m_sBase, m_sNewPath, m_sError ) )
			m_sError.SetSprintf ( "switchover wasn't able to populate %s.link; new persistent path to the index is NOT saved: %s", m_sBase.cstr(), m_sError.cstr() );

		// finalize
		assert ( m_pIdx->GetTokenizer() && m_pIdx->GetDictionary() );

		if ( !ApplyKilllistsMyAndToMe ( m_pIdx, m_szIndex, m_sError ) )
			sphWarning ( "switchover error when applying kill-lists: %s", m_sError.cstr() );

		// unlink .old from new location (it is temporary anyway!)
		if ( m_bHaveOldFiles && m_tFreshCurFiles )
			m_tFreshCurFiles->Unlink ( ".old" );

		// uff. all done
		sphInfo ( "switchover table '%s': success", m_szIndex );
		return true;
	}

	bool SwitchGreedy ( CSphIndex* pIdx )
	{
		SetIdx ( pIdx );
		sphLogDebug ( "SwitchGreedy for '%s' invoked. Base %s, path %s", m_szIndex, m_sBase.cstr(), m_sNewPath.cstr() );

		if ( !CheckSameBase() )
			return true;

		if ( !RotateFiles() )
			return true;

		// try to use new index
		pIdx->Unlock();
		pIdx->SetFilebase ( m_sNewPath );
		if ( pIdx->Prealloc ( g_bStripPath, nullptr, m_dWarnings ) )
			return Finalize();

		// load previous version of index
		pIdx->SetFilebase ( m_sOldPath );
		bool bPreallocOld = pIdx->Prealloc ( g_bStripPath, nullptr, m_dWarnings );

		// roll-back rotated files
		UnRotateFiles();

		// collect all errors
		StringBuilder_c sError { "; " };
		if ( !m_sError.IsEmpty() )
			sError << m_sError;
		if ( !bPreallocOld )
			sError.Sprintf ( "SwitchGreedy table '%s': preload of new index failed, rollback also failed; TABLE UNUSABLE", m_szIndex );
		sError.MoveTo ( m_sError );

		return bPreallocOld;
	}

	// this function always returns true; which means - existing index can't be damaged by this call.
	bool SwitchSeamless ( const cServedIndexRefPtr_c& pServed )
	{
		assert ( pServed && pServed->m_eType == IndexType_e::PLAIN );
		CSphIndex* pIdx = UnlockedHazardIdxFromServed ( *pServed );

		SetIdx ( pIdx );
		sphLogDebug ( "SwitchSeamless for '%s' invoked. Base %s, path %s", m_szIndex, m_sBase.cstr(), m_sNewPath.cstr() );

		if ( !CheckSameBase() )
			return true;

		if ( !RotateFiles() )
		{
			UnRotateFiles();
			return true;
		}

		//////////////////
		/// load new index
		//////////////////
		ServedIndexRefPtr_c pNewServed = MakeCloneForRotation ( pServed, m_szIndex );
		pIdx = UnlockedHazardIdxFromServed ( *pNewServed );
		pIdx->SetFilebase ( m_sNewPath );

		// prealloc enough RAM and lock new index
		sphLogDebug ( "prealloc enough RAM and lock new table" );

		if ( !PreallocNewIndex ( *pNewServed, m_szIndex, m_dWarnings, m_sError ) )
			return true;

		pIdx->Preread();
		pNewServed->UpdateMass(); // that is second update, first was at the end of Prealloc, this one is to correct after preread

		RIdx_c pOldIdx { pServed };
		pIdx->m_iTID = pOldIdx->m_iTID;
//		pServed->SetUnlink ( pOldIdx->GetFilebase() );
		SetIdx ( pIdx );
		Finalize();

		// all went fine; swap them
		sphLogDebug ( "all went fine; swap them" );
		Binlog::NotifyIndexFlush ( pIdx->m_iTID, m_szIndex, Binlog::NoShutdown, Binlog::NoSave );
		g_pLocalIndexes->AddOrReplace ( pNewServed, m_szIndex );
		sphInfo ( "rotating table '%s': success", m_szIndex );

		// actually we always return true, because rotating from new place is always safe.
		return true;
	}
};

// return false, if index should not be served.
// return sError
bool SwitchoverIndexGreedy ( CSphIndex* pIdx, const char* szIndex, const CSphString& sBase, const CSphString& sNewPath, StrVec_t& dWarnings, CSphString& sError )
{
	SwitchOver_c tSwitcher ( szIndex, sBase, sNewPath, dWarnings, sError );
	return tSwitcher.SwitchGreedy ( pIdx );
}

bool DoSwitchoverIndexSeamless ( const cServedIndexRefPtr_c& pServed, const char* szIndex, const CSphString& sBase, const CSphString& sNewPath, StrVec_t& dWarnings, CSphString& sError ) EXCLUDES ( MainThread )
{
	SwitchOver_c tSwitcher ( szIndex, sBase, sNewPath, dWarnings, sError );
	return tSwitcher.SwitchSeamless ( pServed );
}

// called either from MysqlReloadIndex, either from Rotation task (never from main thread).
bool RotateIndexMT ( ServedIndexRefPtr_c& pNewServed, const CSphString & sIndex, StrVec_t & dWarnings, CSphString & sError ) EXCLUDES ( MainThread )
{
	assert ( pNewServed && pNewServed->m_eType == IndexType_e::PLAIN );

	sphInfo ( "rotating table '%s': started", sIndex.cstr() );

	auto sRealPath = RedirectToRealPath ( pNewServed->m_sIndexPath );
	CheckIndexRotate_c tCheck ( sRealPath );
	if ( tCheck.NothingToRotate() )
	{
		sError.SetSprintf ( "nothing to rotate for table '%s'", sIndex.cstr() );
		return false;
	}

	//////////////////
	/// load new index
	//////////////////
	CSphIndex* pNewIndex = UnlockedHazardIdxFromServed ( *pNewServed );
	if ( tCheck.RotateFromNew() )
		pNewIndex->SetFilebase ( IndexFiles_c::MakePath ( ".new", sRealPath ) );

	// prealloc enough RAM and lock new index
	sphLogDebug ( "prealloc enough RAM and lock new table" );

	if ( !PreallocNewIndex ( *pNewServed, sIndex.cstr(), dWarnings, sError ) )
		return false;

	pNewIndex->Preread();
	pNewServed->UpdateMass(); // that is second update, first was at the end of Prealloc, this one is to correct after preread

	//////////////////////
	/// activate new index
	//////////////////////

	sphLogDebug ( "activate new table" );
	if ( tCheck.RotateFromNew() )
	{
		ActionSequence_c tActions;

		auto pServed = GetServed ( sIndex );
		if ( pServed && RedirectToRealPath ( pServed->m_sIndexPath ) == sRealPath )
			tActions.Defer ( RenameIdxSuffix ( pServed, ".old" ) );
		tActions.Defer ( RenameIdx ( pNewIndex, sRealPath ) ); // rename 'new' to 'current'

		if ( !tActions.RunDefers() )
		{
			bool bFatal;
			std::tie ( sError, bFatal ) = tActions.GetError();
			sphWarning ( "RotateIndexMT error: table %s, error %s", sIndex.cstr(), sError.cstr() );
			if ( bFatal )
				g_pLocalIndexes->Delete ( sIndex );
			return false;
		}
		if ( pServed )
		{
			RIdx_c pOldIdx { pServed };
			pNewIndex->m_iTID = pOldIdx->m_iTID;
			pServed->SetUnlink ( pOldIdx->GetFilebase() );
		}
	}

	if ( !ApplyKilllistsMyAndToMe ( pNewIndex, sIndex.cstr(), sError ) )
		return false;

	// all went fine; swap them
	sphLogDebug ( "all went fine; swap them" );
	Binlog::NotifyIndexFlush ( pNewIndex->m_iTID, sIndex.cstr(), Binlog::NoShutdown, Binlog::NoSave );
	g_pLocalIndexes->AddOrReplace ( pNewServed, sIndex );
	sphInfo ( "rotating table '%s': success", sIndex.cstr() );
	return true;
}

static void InvokeRotation ( VecOfServed_c&& dDeferredIndexes ) REQUIRES ( MainThread )
{
	assert ( !dDeferredIndexes.IsEmpty () && "Rotation queue must be checked before invoking rotation!");
	Threads::StartJob ( [dIndexes = std::move ( dDeferredIndexes )] () mutable
	{
		// want to track rotation thread only at work
		auto pDesc = PublishSystemInfo ( "ROTATION" );

		sphLogDebug ( "TaskRotation starts with %d deferred tables", dIndexes.GetLength() );
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
				sphLogDebug ( "seamless rotate (prealloc) mutable table %s", sIndex.cstr() );
				if ( PreallocNewIndex ( *pReplacementServed, sIndex.cstr(), dWarnings, sError ) )
					g_pLocalIndexes->AddOrReplace ( pReplacementServed, sIndex );
				else
					sphWarning ( "table '%s': %s", sIndex.cstr(), sError.cstr() );
			} else
			{
				sphLogDebug ( "seamless rotate local table %s", sIndex.cstr() );
				if ( !RotateIndexMT ( pReplacementServed, sIndex, dWarnings, sError ) )
					sphWarning ( "table '%s': %s", sIndex.cstr(), sError.cstr() );
			}

			for ( const auto& i : dWarnings )
				sphWarning ( "table '%s': %s", sIndex.cstr(), i.cstr() );

			g_pDistIndexes->Delete ( sIndex ); // postponed delete of same-named distributed (if any)
		}

		g_bInRotate = false;
		RotateGlobalIdf();
		sphInfo ( "rotating table: all tables done" );
	});
}

template<typename FN_ACTION>
bool LimitedParallelRotationMT ( FN_ACTION&& fnAction ) EXCLUDES ( MainThread )
{
	assert ( Threads::IsInsideCoroutine() );

	// allow to run several rotations a time (in parallel)
	// vip conns has no limit
	if ( session::GetVip() )
		return fnAction();

	// limit is arbitrary set to N/2 of threadpool
	static Coro::Waitable_T<int> iParallelRotations { 0 };
	iParallelRotations.Wait ( [] ( int i ) { return i < Max ( 1, NThreads() / 2 ); } );
	iParallelRotations.ModifyValue ( [] ( int& i ) { ++i; } );
	AT_SCOPE_EXIT ( [] { iParallelRotations.ModifyValueAndNotifyOne ( [] ( int& i ) { --i; } ); } );
	return fnAction();
}

bool LimitedRotateIndexMT ( ServedIndexRefPtr_c& pNewServed, const CSphString& sIndex, StrVec_t& dWarnings, CSphString& sError ) EXCLUDES ( MainThread )
{
	return LimitedParallelRotationMT ( [&]() { return RotateIndexMT ( pNewServed, sIndex, dWarnings, sError ); } );
}

bool SwitchoverIndexSeamless ( const cServedIndexRefPtr_c& pServed, const char* szIndex, const CSphString& sBase, const CSphString& sNewPath, StrVec_t& dWarnings, CSphString& sError ) EXCLUDES ( MainThread )
{
	return LimitedParallelRotationMT ( [&]() { return DoSwitchoverIndexSeamless ( pServed, szIndex, sBase, sNewPath, dWarnings, sError ); } );
}

void ConfigureLocalIndex ( ServedDesc_t * pIdx, const CSphConfigSection & hIndex, bool bMutableOpt, StrVec_t * pWarnings )
{
	pIdx->m_tSettings.Load ( hIndex, bMutableOpt, pWarnings );
	pIdx->m_sGlobalIDFPath = pIdx->m_tSettings.m_sGlobalIDFPath;
}

bool ConfigureDistributedIndex ( std::function<bool(const CSphString&)>&& fnCheck, DistributedIndex_t & tIdx, const char * szIndexName, const CSphConfigSection & hIndex, CSphString & sError, StrVec_t * pWarnings )
{
	assert ( hIndex("type") && hIndex["type"]=="distributed" );

	bool bSetHA = false;
	// configure ha_strategy
	if ( hIndex("ha_strategy") )
	{
		bSetHA = ParseStrategyHA ( hIndex["ha_strategy"].cstr(), tIdx.m_eHaStrategy );
		if ( !bSetHA )
			sphWarning ( "table '%s': ha_strategy (%s) is unknown for me, will use random", szIndexName, hIndex["ha_strategy"].cstr() );
	}

	bool bEnablePersistentConns = ( g_iPersistentPoolSize>0 );
	if ( hIndex ( "agent_persistent" ) && !bEnablePersistentConns )
	{
			sphWarning ( "table '%s': agent_persistent used, but no persistent_connections_limit defined. Fall back to non-persistent agent", szIndexName );
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
				sphWarning ( "table '%s': no such local table '%s', SKIPPED", szIndexName, sLocal.cstr() );
				sError.SetSprintf ( "no such local table '%s'", sLocal.cstr() );
				return false;
			}
			tIdx.m_dLocal.Add ( sLocal );
		}
	}

	// index-level agent_retry_count
	if ( hIndex ( "agent_retry_count" ) )
	{
		if ( hIndex["agent_retry_count"].intval ()<=0 )
			sphWarning ( "table '%s': agent_retry_count must be positive, ignored", szIndexName );
		else
			tIdx.m_iAgentRetryCount = hIndex["agent_retry_count"].intval ();
	}

	if ( hIndex ( "mirror_retry_count" ) )
	{
		if ( hIndex["mirror_retry_count"].intval ()<=0 )
			sphWarning ( "table '%s': mirror_retry_count must be positive, ignored", szIndexName );
		else
		{
			if ( tIdx.m_iAgentRetryCount>0 )
				sphWarning ("table '%s': `agent_retry_count` and `mirror_retry_count` both specified (they are aliases)."
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
			auto pAgent = ConfigureMultiAgent ( pAgentCnf->cstr(), szIndexName, tAgentOptions, sError, pWarnings );
			if ( !pAgent )
				return false;

			tIdx.m_dAgents.Add ( pAgent );
		}
	}

	// configure options
	if ( hIndex("agent_connect_timeout") )
	{
		if ( hIndex["agent_connect_timeout"].intval()<=0 )
			sphWarning ( "table '%s': agent_connect_timeout must be positive, ignored", szIndexName );
		else
			tIdx.SetAgentConnectTimeoutMs ( hIndex.GetMsTimeMs ( "agent_connect_timeout" ) );
	}

	tIdx.m_bDivideRemoteRanges = hIndex.GetInt ( "divide_remote_ranges", 0 )!=0;

	if ( hIndex("agent_query_timeout") )
	{
		if ( hIndex["agent_query_timeout"].intval()<=0 )
			sphWarning ( "table '%s': agent_query_timeout must be positive, ignored", szIndexName );
		else
			tIdx.SetAgentQueryTimeoutMs ( hIndex.GetMsTimeMs ( "agent_query_timeout") );
	}

	bool bHaveHA = tIdx.m_dAgents.any_of ( [] ( const auto& ag ) { return ag->IsHA (); } );

	// configure ha_strategy
	if ( bSetHA && !bHaveHA && !IsConfigless() )
		sphWarning ( "table '%s': ha_strategy defined, but no ha agents in the table", szIndexName );

	return true;
}

//////////////////////////////////////////////////
/// configure distributed index and add it to hash
//////////////////////////////////////////////////
// AddIndex -> AddDistributedIndex
static ResultAndIndex_t AddDistributedIndex ( const char * szIndexName, const CSphConfigSection & hIndex, CSphString & sError, StrVec_t * pWarnings=nullptr )
{
	DistributedIndexRefPtr_t pIdx ( new DistributedIndex_t );
	bool bOk = ConfigureDistributedIndex ( [] ( const auto& sIdx ) { return g_pLocalIndexes->Contains ( sIdx ); }, *pIdx, szIndexName, hIndex, sError, pWarnings );

	if ( !bOk || pIdx->IsEmpty () )
	{
		if ( !bOk )
			sError.SetSprintf ( "table '%s': %s", szIndexName, sError.cstr() );
		else
			sError.SetSprintf ( "table '%s': no valid local/remote tables in distributed table", szIndexName );
		return { ADD_ERROR, nullptr };
	}

	// finally, check and add distributed index to global table
	if ( !g_pDistIndexes->Add ( pIdx, szIndexName ) )
	{
		sError.SetSprintf ( "table '%s': unable to add name (duplicate?)", szIndexName );
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
			sphWarning ( "table '%s': %s - NOT SERVING", szIndexName, sError.cstr() );
			return false;
		}

		if ( !sWarning.IsEmpty() )
			sphWarning ( "table '%s': %s", szIndexName, sWarning.cstr() );
	}

	if ( !sphRTSchemaConfigure ( hIndex, tSchema, tSettings, pWarnings, sError, bPercolate, bPercolate ) )
	{
		sphWarning ( "table '%s': %s - NOT SERVING", szIndexName, sError.cstr () );
		return false;
	}

	if ( bPercolate )
		FixPercolateSchema ( tSchema );

	if ( !sError.IsEmpty() )
	{
		if ( pWarnings )
			pWarnings->Add(sError);
		else
			sphWarning ( "table '%s': %s", szIndexName, sError.cstr () );
	}

	// path
	if ( !hIndex ( "path" ) )
	{
		sphWarning ( "table '%s': path must be specified - NOT SERVING", szIndexName );
		return false;
	}

	if ( !CheckStoredFields ( tSchema, tSettings, sError ) )
	{
		sphWarning ( "table '%s': %s - NOT SERVING", szIndexName, sError.cstr() );
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
			sphWarning ( "table '%s': has index_sp=%d, index_zones='%s' but disabled html_strip - NOT SERVING", szIndexName, iIndexSP, sIndexZones.cstr() );
			return false;
		}
		CSphString sWarning;
		sWarning.SetSprintf ( "has index_sp=%d but disabled html_strip - PARAGRAPH unavailable", iIndexSP );
		if ( pWarnings )
			pWarnings->Add(sWarning);
		else
			sphWarning ( "table '%s': %s", szIndexName, sWarning.cstr() );
	}

	// upgrading schema to store field lengths
	if ( tSettings.m_bIndexFieldLens )
		if ( !AddFieldLens ( tSchema, false, sError ) )
		{
			sphWarning ( "table '%s': failed to create field lengths attributes: %s", szIndexName, sError.cstr () );
			return false;
		}

	if ( bWordDict && ( tSettings.m_dPrefixFields.GetLength () || tSettings.m_dInfixFields.GetLength () ) )
	{
		CSphString sWarning = "prefix_fields and infix_fields has no effect with dict=keywords, ignoring";
		if ( pWarnings )
			pWarnings->Add(sWarning);
		else
			sphWarning ( "table '%s': %s", szIndexName, sWarning.cstr() );
	}

	if ( bWordDict && tSettings.m_iMinInfixLen==1 )
	{
		CSphString sWarning = "min_infix_len must be greater than 1, changed to 2";
		if ( pWarnings )
			pWarnings->Add(sWarning);
		else
			sphWarning ( "table '%s': %s", szIndexName, sWarning.cstr() );

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
			sError.SetSprintf ( "table '%s': unknown dict=%s; only 'keywords' or 'crc' values allowed", szIndexName, sIndexType.cstr() );
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
	auto bNeedBinlog = hIndex.GetBool ( "binlog" );

	std::unique_ptr<CSphIndex> pIdx;
	if ( bRT )
	{
		pIdx = sphCreateIndexRT ( szIndexName, pServed->m_sIndexPath, std::move ( tSchema ), pServed->m_tSettings.m_iMemLimit, bWordDict );
		pServed->m_eType = IndexType_e::RT;
		tSettings.m_bBinlog = bNeedBinlog;
		if ( !bNeedBinlog )
			pIdx->m_iTID = -1;
	} else
	{
		if ( !bNeedBinlog )
		{
			sError.SetSprintf ( "table '%s': percolate without binlog not implemented", szIndexName );
			return { ADD_ERROR, nullptr };
		}
		pIdx = CreateIndexPercolate ( szIndexName, pServed->m_sIndexPath, std::move ( tSchema ) );
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
	auto pIdx = sphCreateIndexPhrase ( szIndexName, RedirectToRealPath ( pServed->m_sIndexPath ) );
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
		sphWarning ( "failed to configure table %s: %s", szIndexName, sError.cstr () );
		return { ADD_ERROR, nullptr };
	}

	if ( !sWarning.IsEmpty() )
		sphWarning ( "table '%s': %s - NOT SERVING", szIndexName, sWarning.cstr () );

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
		sphWarning ( "table '%s': %s - NOT SERVING", szIndexName, sError.cstr () );
		return { ADD_ERROR, nullptr };
	}

	for ( const auto & i : dWarnings )
		sphWarning ( "table '%s': %s", szIndexName, i.cstr() );

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
		sError = "duplicate name";
		sphWarning ( "table '%s': duplicate name - NOT SERVING", szIndexName );
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

	sphWarning ( "table '%s': unknown type '%s' - NOT SERVING", szIndexName, hIndex["type"].cstr() );
	return { ADD_ERROR, nullptr };
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
		sphInfo ( "No tables found in config came to rotation. Abort reloading");
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

		if ( !ServedDesc_t::IsMutable ( pIndex ) && CheckIndexRotate_c ( *pIndex, CheckIndexRotate_c::CheckLink ).NothingToRotate() )
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
				sphWarning ( "table '%s': %s - NOT SERVING", sDeferredIndex.cstr(), sError.cstr() );
		}
		else if ( pDeferredIndex->m_eType==IndexType_e::PLAIN )
		{
			sphLogDebug ( "greedy rotate local %s", sDeferredIndex.cstr() );
			auto pRotating = GetServed ( sDeferredIndex );
			bool bSame = pRotating && pRotating.Ptr() == pDeferredIndex.Ptr();
			WIdx_c WIdx { pDeferredIndex };
			bool bOk = RotateIndexGreedy ( *pDeferredIndex, sDeferredIndex.cstr(), sError );
			if ( !bOk )
				sphWarning ( "table '%s': %s - NOT SERVING", sDeferredIndex.cstr(), sError.cstr() );

			if ( !bSame && bOk && !sphFixupIndexSettings ( WIdx, g_hCfg["index"][sDeferredIndex], g_bStripPath, nullptr, dWarnings, sError ) )
			{
				sphWarning ( "table '%s': %s - NOT SERVING", sDeferredIndex.cstr(), sError.cstr() );
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
			sphWarning ( "table '%s': %s", sDeferredIndex.cstr(), i.cstr() );

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
		auto [bChanged, dConfig] = FetchAndCheckIfChanged ( g_sConfigFile );
		if ( bChanged || g_bReloadForced )
		{
			sphInfo( "Config changed (read %d chars)", dConfig.GetLength());
			if ( !dConfig.IsEmpty() )
			{
				{
					ScWL_t dRotateConfigMutexWlocked { g_tRotateConfigMutex };
					bReloadHappened = ParseConfig ( &g_hCfg, g_sConfigFile, dConfig );
				}
				if ( bReloadHappened )
				{
					ScRL_t dRotateConfigMutexRlocked { g_tRotateConfigMutex };
					ReloadIndexesFromConfig ( g_hCfg, hDeferredIndexes );
				} else
					sphWarning ( "failed to parse config file '%s': %s; using previous settings", g_sConfigFile.cstr(), TlsMsg::szError() );
			}
		}
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

	ReopenDaemonLog ();
	ReopenQueryLog ();
	ReopenHttpLog();

	g_bGotSigusr1 = 0;
}


void ShowHelp ()
{
	fprintf ( stdout,
		"Usage: searchd [OPTIONS]\n"
		"\n"
		"Options are:\n"
		"-h, --help\t\tdisplay this help message\n"
		"-v, --version\t\tdisplay version information\n"
		"-q, --quiet\t\tonly print errors on startup\n"
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
		"\t\t\tand other file names stored in the table header\n"
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
		"-i, --index <index>\tonly serve given table(s)\n"
		"-t, --table <table>\tonly serve given table(s)\n"
#if !_WIN32
		"--nodetach\t\tdo not detach into background\n"
#endif
		"--logdebug, --logdebugv, --logdebugvv\n"
		"\t\t\tenable additional debug information logging\n"
		"\t\t\t(with different verboseness)\n"
		"--pidfile\t\tforce using the PID file (useful with --console)\n"
		"--safetrace\t\tonly use system backtrace() call in crash reports\n"
		"--coredump\t\tsave core dump file on crash\n"
		"--mockstack\t\tdump safe stack sizes and exit\n"
		"\n"
		"Examples:\n"
		"searchd --config /usr/local/sphinx/etc/manticore.conf\n"
#if _WIN32
		"searchd --install --config c:\\sphinx\\manticore.conf\n"
#endif
		);
}

static bool HasQuietFlag ( int argc, char ** argv )
{
	for ( int i = 1; i < argc; ++i )
		if ( !strcmp ( argv[i], "-q" ) || !strcmp ( argv[i], "--quiet" ) )
			return true;

	return false;
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


#if !_WIN32

static char g_sNameBuf[512] = { 0 };
static char g_sPid[30] = { 0 };

// returns 'true' only once - at the very start, to show it beautiful way.
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
			snprintf ( g_sPid, sizeof(g_sPid), "%d", iChild);
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
				sphDumpGdb ( GetDaemonLogFD(), g_sNameBuf, g_sPid );
			}
		}

		if ( bShutdown || sphInterrupted() || g_pShared->m_bDaemonAtShutdown )
		{
			exit ( 0 );
		}
	}
}
#endif // !_WIN32

/// check for incoming signals, and react on them
void CheckSignals () REQUIRES ( MainThread )
{
#if _WIN32
	if ( WinService() && StopWinService() )
	{
		Shutdown ();
		SetWinServiceStopped();
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
	CheckWinSignals();
#endif
}

static bool g_bLoadInfo = val_from_env ( "MANTICORE_TRACE_LOAD", false );


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

	if ( sphMicroTimer() > g_iNextExpMeterTimestamp )
	{
		g_iNextExpMeterTimestamp += g_iExpMeterPeriod;
		auto tSample = GlobalWorkPool()->Tasks();
		auto tCurrent = GlobalWorkPool()->CurTasks();
		g_tPriStat1m.Tick ( tSample.iPri );
		g_tPriStat5m.Tick ( tSample.iPri );
		g_tPriStat15m.Tick ( tSample.iPri );
		g_tSecStat1m.Tick ( tSample.iSec );
		g_tSecStat5m.Tick ( tSample.iSec );
		g_tSecStat15m.Tick ( tSample.iSec );

		g_tStat1m.Tick ( tSample.iPri + tSample.iSec + tCurrent);
		g_tStat5m.Tick ( tSample.iPri + tSample.iSec + tCurrent);
		g_tStat15m.Tick ( tSample.iPri + tSample.iSec + tCurrent);
		if ( g_bLoadInfo )
			sphInfo("Sample: %d, %d, %d; Load average: %0.2f, %0.2f, %0.2f, sec: %0.2f, %0.2f, %0.2f, pri: %0.2f, %0.2f, %0.2f", tCurrent, tSample.iSec, tSample.iPri, g_tStat1m.Value(), g_tStat5m.Value(), g_tStat15m.Value(), g_tSecStat1m.Value(), g_tSecStat5m.Value(), g_tSecStat15m.Value(), g_tPriStat1m.Value(), g_tPriStat5m.Value(), g_tPriStat15m.Value());
	}
}

bool g_bVtune = false;
int64_t g_tmStarted = 0;

static int	g_iNetWorkers = 1;

/////////////////////////////////////////////////////////////////////////////
// DAEMON OPTIONS
/////////////////////////////////////////////////////////////////////////////

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

static void SetOptionSI ( const CSphConfigSection & hSearchd, bool bTestMode )
{
	SIDefault_e eState = GetSecondaryIndexDefault();
	if ( bTestMode )
		eState = SIDefault_e::DISABLED;

	CSphVariant * pOption = hSearchd ( "secondary_indexes" );
	if ( pOption )
	{
		if ( pOption->strval()=="force" )
			eState = SIDefault_e::FORCE;
		else if ( pOption->intval()==0 )
			eState = SIDefault_e::DISABLED;
		else
			eState = SIDefault_e::ENABLED;

		if ( eState != SIDefault_e::DISABLED && !IsSecondaryLibLoaded() )
			sphWarning ( "secondary_indexes set but failed to initialize secondary library: %s", g_sSecondaryError.cstr() );
	}

	SetSecondaryIndexDefault ( eState );
}


static void ConfigureMerge ( const CSphConfigSection & hSearchd )
{
	BuildBufferSettings_t tSettings;
	tSettings.m_iBufferAttributes	= hSearchd.GetSize ( "merge_buffer_attributes",	tSettings.m_iBufferAttributes );
	tSettings.m_iBufferColumnar		= hSearchd.GetSize ( "merge_buffer_columnar",	tSettings.m_iBufferColumnar );
	tSettings.m_iBufferStorage		= hSearchd.GetSize ( "merge_buffer_storage",	tSettings.m_iBufferStorage );
	tSettings.m_iBufferFulltext		= hSearchd.GetSize ( "merge_buffer_fulltext",	tSettings.m_iBufferFulltext );
	tSettings.m_iBufferDict			= hSearchd.GetSize ( "merge_buffer_dict",		tSettings.m_iBufferDict );
	tSettings.m_iSIMemLimit			= hSearchd.GetSize ( "merge_si_memlimit",		tSettings.m_iSIMemLimit );

	SetMergeSettings(tSettings);
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
	g_bTimeoutEachPacket = hSearchd.GetBool( "reset_network_timeout_on_packet" );

	g_iClientQlTimeoutS = hSearchd.GetSTimeS( "sphinxql_timeout", 900);
	g_iClientTimeoutS = hSearchd.GetSTimeS ( "client_timeout", 300 );

	g_iMaxConnection = hSearchd.GetInt ( "max_connections", g_iMaxConnection );
	auto iThreads = hSearchd.GetInt ( "threads", GetNumLogicalCPUs() );
	SetMaxChildrenThreads ( iThreads );
	g_iThdQueueMax = hSearchd.GetInt ( "jobs_queue_size", g_iThdQueueMax );

	g_iPersistentPoolSize = hSearchd.GetInt ("persistent_connections_limit");

	// FIXME!!! remove depricated preopen_indexes
	if ( hSearchd.Exists ( "preopen_tables" ) )
		MutableIndexSettings_c::GetDefaults().m_bPreopen = hSearchd.GetBool ( "preopen_tables" );
	else
		MutableIndexSettings_c::GetDefaults().m_bPreopen = hSearchd.GetBool ( "preopen_indexes" );

	sphSetUnlinkOld ( hSearchd.GetBool ( "unlink_old" ) );
	g_iExpansionLimit = hSearchd.GetInt ( "expansion_limit" );
	if ( hSearchd.Exists ( "expansion_merge_threshold_docs" ) )
		ExpandedMergeThdDocs ( hSearchd.GetInt ( "expansion_merge_threshold_docs" ) );
	if ( hSearchd.Exists ( "expansion_merge_threshold_hits" ) )
		ExpandedMergeThdHits ( hSearchd.GetInt ( "expansion_merge_threshold_hits" ) );

	// initialize buffering settings
	SetUnhintedBuffer ( hSearchd.GetSize( "read_unhinted", DEFAULT_READ_UNHINTED ) );
	int iReadBuffer = hSearchd.GetSize ( "read_buffer", DEFAULT_READ_BUFFER );
	FileAccessSettings_t & tDefaultFA = MutableIndexSettings_c::GetDefaults().m_tFileAccess;
	tDefaultFA.m_iReadBufferDocList = hSearchd.GetSize ( "read_buffer_docs", iReadBuffer );
	tDefaultFA.m_iReadBufferHitList = hSearchd.GetSize ( "read_buffer_hits", iReadBuffer );
	tDefaultFA.m_eDoclist = GetFileAccess( hSearchd, "access_doclists", true, FileAccess_e::FILE );
	tDefaultFA.m_eHitlist = GetFileAccess( hSearchd, "access_hitlists", true, FileAccess_e::FILE );
	tDefaultFA.m_eDict = GetFileAccess( hSearchd, "access_dict", false, tDefaultFA.m_eDict );

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
		if ( IsTimeZoneSet() )
			sphWarning ( "grouping_in_utc=1 conflicts with 'timezone'" );

		SetGroupingInUTC ( hSearchd["grouping_in_utc"].intval ()!=0 );
	}

	if ( hSearchd ( "timezone" ) )
	{
		if ( GetGroupingInUTC() )
			sphWarning ( "grouping_in_utc=1 conflicts with 'timezone'" );

		CSphString sWarn;
		SetTimeZone ( hSearchd["timezone"].cstr(), sWarn );
		if ( !sWarn.IsEmpty() )
			sphWarning ( "%s", sWarn.cstr() );
		else
			sphInfo ( "Using time zone '%s'", GetTimeZoneName().cstr() );
	}

	if ( hSearchd("join_cache_size") )
		SetJoinCacheSize ( hSearchd.GetSize64 ( "join_cache_size", GetJoinCacheSize() ) );

	// sha1 password hash for shutdown action
	SetShutdownToken ( hSearchd.GetStr ( "shutdown_token" ) );

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
	if ( g_iAgentRetryDelayMs > DAEMON_MAX_RETRY_DELAY )
		sphWarning ( "agent_retry_delay %d exceeded max recommended %d", g_iAgentRetryDelayMs, DAEMON_MAX_RETRY_DELAY );
	g_iAgentRetryCount = hSearchd.GetInt ( "agent_retry_count", g_iAgentRetryCount );
	if ( g_iAgentRetryCount > DAEMON_MAX_RETRY_COUNT )
		sphWarning ( "agent_retry_count %d exceeded max recommended %d", g_iAgentRetryCount, DAEMON_MAX_RETRY_COUNT );

	g_iReplConnectTimeoutMs = hSearchd.GetMsTimeMs ( "replication_connect_timeout", g_iReplConnectTimeoutMs );
	g_iReplQueryTimeoutMs = hSearchd.GetMsTimeMs ( "replication_query_timeout", g_iReplQueryTimeoutMs );
	g_iReplRetryCount = hSearchd.GetInt ( "replication_retry_count", g_iReplRetryCount );
	g_iReplRetryDelayMs = hSearchd.GetMsTimeMs ( "replication_retry_delay", g_iReplRetryDelayMs );
	ReplicationSetTimeouts ( g_iReplConnectTimeoutMs, g_iReplQueryTimeoutMs, g_iReplRetryCount, g_iReplRetryDelayMs );

	g_tmWaitUS = hSearchd.GetUsTime64Ms ( "net_wait_tm", g_tmWaitUS );
	g_iThrottleAction = hSearchd.GetInt ( "net_throttle_action", g_iThrottleAction );
	g_iThrottleAccept = hSearchd.GetInt ( "net_throttle_accept", g_iThrottleAccept );
	g_iNetWorkers = hSearchd.GetInt ( "net_workers", g_iNetWorkers );
	g_iNetWorkers = Max ( g_iNetWorkers, 1 );
	CheckSystemTFO();
	if ( g_iTFO!=TFO_ABSENT && hSearchd.GetInt ( "listen_tfo", 1 )==0 )
	{
		g_iTFO &= ~TFO_LISTEN;
	}

	bool bLocaleSet = false;
	if ( hSearchd ( "collation_libc_locale" ) )
	{
		auto sLocale = hSearchd.GetStr ( "collation_libc_locale" );
		bLocaleSet = setlocale ( LC_COLLATE, sLocale.cstr() );
		if ( !bLocaleSet )
			sphWarning ( "setlocale failed (locale='%s')", sLocale.cstr() );
	}
	CSphString sLoc = setlocale ( LC_COLLATE, nullptr );
	SetLocale( sLoc, bLocaleSet );

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
			SetLogFileMode ( iMode );
		}
	}
	if ( hSearchd ( "server_id" ) )
	{
		int iServerID = hSearchd.GetInt ( "server_id", 0 );
		const int iServerMask = 0x7f;
		if ( iServerID>iServerMask )
		{
			iServerID &= iServerMask;
			sphWarning ( "server_id out of range 0 - 127, clamped to %d", iServerID );
		}
		SetServerID ( iServerID );
	}

	g_sMySQLVersion = hSearchd.GetStr ( "mysql_version_string", g_sMySQLVersion.cstr() );
	sphinxexpr::MySQLVersion() = g_sMySQLVersion;
	g_sKbnVersion = hSearchd.GetStr ( "kibana_version_string" );

	AllowOnlyNot ( hSearchd.GetInt ( "not_terms_only_allowed", 0 )!=0 );
	SetBooleanSimplify ( hSearchd.GetBool ( "boolean_simplify", CSphQuery::m_bDefaultSimplify ) );

	ConfigureQueryLogCommands ( hSearchd.GetStr ( "query_log_commands" ) );

	g_iAutoOptimizeCutoffMultiplier = hSearchd.GetInt ( "auto_optimize", 1 );
	MutableIndexSettings_c::GetDefaults().m_iOptimizeCutoff = hSearchd.GetInt ( "optimize_cutoff", AutoOptimizeCutoff() );
	MutableIndexSettings_c::GetDefaults().m_iOptimizeCutoffKNN = hSearchd.GetInt ( "optimize_cutoff", AutoOptimizeCutoffKNN() );

	SetPseudoSharding ( hSearchd.GetInt ( "pseudo_sharding", 1 )!=0 );
	SetOptionSI ( hSearchd, bTestMode );
	SetSIBlockCacheSize ( hSearchd.GetSize64 ( "secondary_index_block_cache", GetSIBlockCacheSize() ) );

	CSphString sWarning;
	AttrEngine_e eEngine = AttrEngine_e::DEFAULT;
	if ( StrToAttrEngine ( eEngine, AttrEngine_e::ROWWISE, hSearchd.GetStr("engine"), sWarning ) )
		SetDefaultAttrEngine(eEngine);
	else
		sphWarning ( "%s", sWarning.cstr() );

	g_bHasBuddyPath = hSearchd.Exists ( "buddy_path" );
	g_sBuddyPath = hSearchd.GetStr ( "buddy_path" );
	g_bTelemetry = ( hSearchd.GetInt ( "telemetry", g_bTelemetry ? 1 : 0 )!=0 );
	g_bAutoSchema = ( hSearchd.GetInt ( "auto_schema", g_bAutoSchema ? 1 : 0 )!=0 );

	SetAccurateAggregationDefault ( hSearchd.GetInt ( "accurate_aggregation", GetAccurateAggregationDefault() )!=0 );
	SetDistinctThreshDefault ( hSearchd.GetInt ( "distinct_precision_threshold", GetDistinctThreshDefault() ) );

	ConfigureMerge(hSearchd);
	SetJoinBatchSize ( hSearchd.GetInt ( "join_batch_size", GetJoinBatchSize() ) );
	SetRtFlushDiskPeriod ( hSearchd.GetSTimeS ( "diskchunk_flush_write_timeout", bTestMode ? -1 : 1 ), hSearchd.GetSTimeS ( "diskchunk_flush_search_timeout", 30 ) );

	int iExpansionPhraseLimit = hSearchd.GetInt ( "expansion_phrase_limit", 1024 );
	bool bExpansionPhraseWarning = hSearchd.GetBool ( "expansion_phrase_warning", false );
	SetExpansionPhraseLimit ( iExpansionPhraseLimit, bExpansionPhraseWarning );

	g_bAttrAutoconvStrict = hSearchd.GetBool ( "attr_autoconv_strict", false );
}

static void DirMustWritable ( const CSphString & sDataDir )
{
	CSphString sError;
	CSphString sTmpName;
	sTmpName.SetSprintf ( "%s/gmb_%d", sDataDir.cstr(), (int)getpid() );

	CSphWriter tFile;
	if ( !tFile.OpenFile ( sTmpName, sError ) )
		sphFatal ( "The directory Manticore starts from must be writable for the daemon, error: %s", sError.cstr() );

	tFile.PutDword( 1 );
	tFile.Flush();

	if ( tFile.IsError() )
		sphFatal ( "The directory Manticore starts from must be writable for the daemon, error: %s", sError.cstr() );
}

static void CheckSetCwd () REQUIRES ( MainThread )
{
	if ( g_bNoChangeCwd || !IsConfigless() )
		return;

	CSphString sDataDir = GetDataDirInt();
	if ( !IsPathAbsolute ( sDataDir ) )
	{
		DirMustWritable ( "." );
		return;
	}

	int iRes = chdir ( sDataDir.cstr() );
	if ( iRes!=0 )
		sphFatal ( "failed to change current working directory to '%s': %s", sDataDir.cstr(), strerror(errno) );

	sphLogDebug ( "current working directory changed to '%s'", sDataDir.cstr() );
	DirMustWritable ( sDataDir );
	g_bCwdChanged = true;
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

static void DumpSettingsSection ( const CSphConfigSection & hNode, const char * sSectionName, RowBuffer_i & tOut )
{
	StringBuilder_c tTmp;

	for ( const auto & tIt : hNode )
	{
		const CSphVariant * pVal = &tIt.second;

		// empty binlog_path should not have default value
		if ( tIt.first=="binlog_path" && pVal->strval().IsEmpty() )
			continue;

		tTmp.Clear();
		tTmp.Appendf ( "%s.%s", sSectionName, tIt.first.cstr() );

		do
		{
			// data packets
			tOut.PutString ( tTmp.cstr() );

			if ( g_hSearchdPathVars[tIt.first] )
				PutPath ( g_sConfigPath, pVal->strval(), tOut );
			else
				tOut.PutString ( pVal->strval() );

			if ( !tOut.Commit() )
				return;

			pVal = pVal->m_pNext;
		} while ( pVal );
	}
}

static void DumpSettingsSection ( const CSphConfig & hConf, const char * sSectionName, RowBuffer_i & tOut )
{
	if ( !hConf.Exists ( sSectionName ) || !hConf[sSectionName].Exists ( sSectionName ) )
		return;

	DumpSettingsSection ( hConf[sSectionName][sSectionName], sSectionName, tOut );
}

static void DumpCommonSection ( const CSphConfig & hConf, RowBuffer_i & tOut )
{
	CSphString sCommonName ( "common" );
	CSphString sPDirName ( "plugin_dir" );
	if ( hConf.Exists ( sCommonName ) && hConf[sCommonName].Exists ( sCommonName ) && hConf[sCommonName][sCommonName].Exists ( sPDirName ) )
	{
		DumpSettingsSection ( hConf, sCommonName.cstr(), tOut );
		return;
	}

	// plugin_dir should be printed always
	CSphConfigSection hCommon;
	if ( hConf.Exists ( sCommonName ) && hConf[sCommonName].Exists ( sCommonName ) )
		hCommon = hConf[sCommonName][sCommonName];

	hCommon.AddEntry ( sPDirName.cstr(), PluginGetDir().cstr() );
	DumpSettingsSection ( hCommon, sCommonName.cstr(), tOut );
}

void HandleMysqlShowSettings ( const CSphConfig & hConf, RowBuffer_i & tOut )
{
	tOut.HeadBegin ();
	tOut.HeadColumn ( "Setting_name" );
	tOut.HeadColumn ( "Value" );
	tOut.HeadEnd ();

	// configuration file path
	tOut.PutString ( "configuration_file" );
	PutPath ( g_sConfigPath, g_sConfigFile, tOut );
	tOut.Commit();
	// pid 
	tOut.PutString ( "worker_pid" );
	tOut.PutNumAsString ( (int)getpid() );
	tOut.Commit();

	DumpSettingsSection ( hConf, "searchd", tOut );
	DumpCommonSection ( hConf, tOut );
	DumpSettingsSection ( hConf, "indexer", tOut );

	// done
	tOut.Eof();

}

// load index which is not yet load, and publish it in served indexes.
// ServiceMain -> ConfigureAndPreloadOnStartup -> ConfigureAndPreloadIndex
// ServiceMain -> ConfigureAndPreloadOnStartup -> ConfigureAndPreloadConfiglessIndexes -> ConfiglessPreloadIndex -> ConfigureAndPreloadIndex
// from any another thread:
// ClientSession_c::Execute -> HandleMysqlImportTable -> AddExistingIndexConfigless -> ConfiglessPreloadIndex -> ConfigureAndPreloadIndex
ESphAddIndex ConfigureAndPreloadIndex ( const CSphConfigSection & hIndex, const char * szIndexName, StrVec_t& dWarnings, CSphString& sError )
{
	auto [eAdd, pJustLoadedLocal] = AddIndex ( szIndexName, hIndex, true, false, nullptr, sError );

	// local plain, rt, percolate added, but need to be at least preallocated before they could work.
	switch ( eAdd )
	{
	case ADD_NEEDLOAD:
	{
		assert ( pJustLoadedLocal );
		if ( !g_bOptQuiet )
		{
			fprintf ( stdout, "precaching table '%s'\n", szIndexName );
			fflush ( stdout );
		}

		IndexFiles_c dJustAddedFiles ( pJustLoadedLocal->m_sIndexPath );

		if ( dJustAddedFiles.HasAllFiles ( ".new" ) )
		{
			WIdx_c WFake { pJustLoadedLocal }; // as RotateIndexGreedy wants w-locked
			if ( RotateIndexGreedy ( *pJustLoadedLocal, szIndexName, sError ) )
			{
				if ( !FixupAndLockIndex ( *pJustLoadedLocal, UnlockedHazardIdxFromServed ( *pJustLoadedLocal ), &hIndex, szIndexName, dWarnings, sError ) )
					return ADD_ERROR;
			} else
			{
				dWarnings.Add ( sError );
				if ( !PreallocNewIndex ( *pJustLoadedLocal, &hIndex, szIndexName, dWarnings, sError ) )
					return ADD_ERROR;
			}
		} else
		{
			if ( !PreallocNewIndex ( *pJustLoadedLocal, &hIndex, szIndexName, dWarnings, sError ) )
				return ADD_ERROR;

			// index could load global_idf from the settings
			// need to pass and load global idf below
			CSphIndex * pIdx = UnlockedHazardIdxFromServed ( *pJustLoadedLocal );
			if ( pIdx->GetMutableSettings().IsSet ( MutableName_e::GLOBAL_IDF ) )
				pJustLoadedLocal->m_sGlobalIDFPath = pIdx->GetMutableSettings().m_sGlobalIDFPath;
		}
	}
	// no break
	case ADD_SERVED:
	{
		if ( !pJustLoadedLocal->m_sGlobalIDFPath.IsEmpty() && !sph::PrereadGlobalIDF ( pJustLoadedLocal->m_sGlobalIDFPath, sError ) )
			dWarnings.Add ( "global IDF unavailable - IGNORING" );

		// finally add the index to the hash of enabled.
		g_pLocalIndexes->Add ( pJustLoadedLocal, szIndexName );
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
			const char * szIndexName = tIndex.first.cstr();

			if ( !dOptIndexes.IsEmpty() && !dOptIndexes.any_of ( [&] ( const CSphString &rhs ) { return rhs.EqN ( szIndexName ); } ) )
				continue;

			StrVec_t dWarnings;
			CSphString sError;
			ESphAddIndex eAdd = ConfigureAndPreloadIndex ( hIndex, szIndexName, dWarnings, sError );
			for ( const auto & i : dWarnings )
				sphWarning ( "table '%s': %s", szIndexName, i.cstr() );

			if ( eAdd==ADD_ERROR )
				sphWarning ( "table '%s': %s - NOT SERVING", szIndexName, sError.cstr() );

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
				sphWarning ( "table '%s': error applying killlist: %s", pIdx->GetName(), sError.cstr() );

			if ( sWarning.Length() )
				sphWarning ( "%s", sWarning.cstr() );
		}
	}

	// set index cluster name for check
	for ( const ClusterDesc_t & tClusterDesc : GetClustersInt() )
		for ( const auto & tIndex : tClusterDesc.m_hIndexes )
			AssignClusterToIndex ( tIndex.first, tClusterDesc.m_sName );
	sphLogDebugRpl ( "%d clusters loaded from config", GetClustersInt().GetLength() );


	tmLoad += sphMicroTimer();
	if ( !iValidIndexes )
		sphLogDebug ( "no valid tables to serve" );
	else
		if ( !g_bOptQuiet )
			fprintf ( stdout, "precached %d tables in %0.3f sec\n", iCounter, float(tmLoad)/1000000 );
}

// if data_dir changes cwd then paths at sections searchd and common should be fixed from realtive into absolute
void FixPathAbsolute ( CSphString & sPath )
{
	if ( !g_bCwdChanged )
		return;

	if ( sPath.IsEmpty() || IsPathAbsolute ( sPath ) )
		return;

	assert ( !g_sExePath.IsEmpty() );
	CSphString sFullPath;
	sFullPath.SetSprintf ( "%s/%s", g_sExePath.cstr(), sPath.cstr() );

	sPath = sphNormalizePath ( sFullPath );
}

static void OpenDaemonLog ( const CSphConfigSection & hSearchd, bool bCloseIfOpened=false )
{
	auto sLog = hSearchd.Opt<CSphString>("log").value_or("searchd.log");
	SetDaemonLog ( std::move(sLog), bCloseIfOpened );
}


namespace { // static

// implement '--stop' and '--stopwait' (connect and stop another instance by pid file from config)
int StopOrStopWaitAnother ( CSphVariant * v, bool bWait ) REQUIRES ( MainThread )
{
	if ( !v )
		sphFatal ( "stop: option 'pid_file' not found in '%s' section 'searchd'", g_sConfigFile.cstr () );

	CSphString sPidFile = v->cstr();
	FixPathAbsolute ( sPidFile );
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

	int iExitCode = 0;
#if _WIN32
	WinStopOrWaitAnother(iPid, iWaitTimeout);
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
	sphInfo ( "stop: successfully sent SIGTERM to pid %d", iPid );

	iExitCode = ( bWait && ( iPipeCreated==-1 || fdPipe<0 ) ) ? 1 : 0;
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
			}
			iExitCode = ( uStatus==1 ? 0 : 2 ); // uStatus == 1 - AttributeSave - ok, other values - error

			if ( !bHandshake )
				break;

			bHandshake = false;
		}
	::unlink ( sPipeName.cstr () ); // is ok on linux after it is opened.

	if ( fdPipe>=0 )
		::close ( fdPipe );

#endif
	return iExitCode;}
} // static namespace


static void InitBanner()
{
	const char * szColumnarVer = GetColumnarVersionStr();
	CSphString sColumnar = "";
	if ( szColumnarVer )
		sColumnar.SetSprintf ( " (columnar %s)", szColumnarVer );

	const char * szSiVer = GetSecondaryVersionStr();
	CSphString sSi = "";
	if ( szSiVer )
		sSi.SetSprintf ( " (secondary %s)", szSiVer );

	const char * szKNNVer = GetKNNVersionStr();
	CSphString sKNN = "";
	if ( szKNNVer )
		sKNN.SetSprintf ( " (knn %s)", szKNNVer );

	const char * szKNNEmbVer = GetKNNEmbeddingsVersionStr();
	CSphString sKNNEmb = "";
	if ( szKNNEmbVer )
		sKNNEmb.SetSprintf ( " (embeddings %s)", szKNNEmbVer );

	g_sBannerVersion.SetSprintf ( "%s%s%s%s%s", szMANTICORE_NAME, sColumnar.cstr(), sSi.cstr(), sKNN.cstr(), sKNNEmb.cstr() );
	g_sBanner.SetSprintf ( "%s%s", g_sBannerVersion.cstr(), szMANTICORE_BANNER_TEXT );
	g_sMySQLVersion.SetSprintf ( "%s%s%s%s%s", szMANTICORE_VERSION, sColumnar.cstr(), sSi.cstr(), sKNN.cstr(), sKNNEmb.cstr() );
	g_sStatusVersion.SetSprintf ( "%s%s%s%s%s", szMANTICORE_VERSION, sColumnar.cstr(), sSi.cstr(), sKNN.cstr(), sKNNEmb.cstr() );
}


static void CheckSSL()
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


static void CacheCPUInfo()
{
	// these funcs do caching inside
	GetNumLogicalCPUs();
	GetNumPhysicalCPUs();
}

#ifndef LOCALDATADIR
#define LOCALDATADIR "."
#endif

#if !defined WINAPI
#define WINAPI
#endif

int WINAPI ServiceMain ( int argc, char **argv ) EXCLUDES (MainThread)
{
	ScopedRole_c thMain (MainThread);
	RefreshIsAtty();

#ifdef USE_VTUNE
	__itt_pause ();
#endif // USE_VTUNE
	g_tmStarted = sphMicroTimer();

#if _WIN32
	SetupWinService (argc, argv);
#endif

	tzset();

	Tracer::Init();

#if _WIN32
	CheckWinInstall();
#endif

	CSphString sError, sKNNError;
	// initialize it before other code to fetch version string for banner
	bool bColumnarError = !InitColumnar ( sError );
	bool bSecondaryError = !InitSecondary ( g_sSecondaryError );
	bool bKNNError = !InitKNN ( sKNNError );
	sphCollationInit ();

	InitBanner();

	g_bOptQuiet = HasQuietFlag ( argc, argv );
	ESphLogLevel eQuietRestoreLevel = g_eLogLevel;
	if ( g_bOptQuiet )
		g_eLogLevel = SPH_LOG_FATAL;

	if ( !WinService() && !g_bOptQuiet )
		fprintf ( stdout, "%s",  g_sBanner.cstr() );

	if ( bColumnarError )
		sphWarning ( "Error initializing columnar storage: %s", sError.cstr() );

	if ( bSecondaryError )
		sphWarning ( "Error initializing secondary index: %s", g_sSecondaryError.cstr() );

	if ( bKNNError )
		sphWarning ( "Error initializing knn index: %s", sKNNError.cstr() );

	if ( !sError.IsEmpty() )
		sError = "";

	if ( !sKNNError.IsEmpty() )
		sKNNError = "";

	CSphString sTZWarning;
	{
		StrVec_t dWarnings;
		InitTimeZones ( dWarnings );
		sTZWarning = ConcatWarnings(dWarnings);
	}

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
	bool			bMeasureStack = false;

	DWORD			uReplayFlags = 0;

	#define OPT(_a1,_a2)	else if ( !strcmp(argv[i],_a1) || !strcmp(argv[i],_a2) )
	#define OPT1(_a1)		else if ( !strcmp(argv[i],_a1) )

	auto UpdateLogLevel = [&eQuietRestoreLevel] ( ESphLogLevel eLevel )
	{
		if ( g_bOptQuiet )
			eQuietRestoreLevel = Max ( eQuietRestoreLevel, eLevel );
		else
			g_eLogLevel = Max ( g_eLogLevel, eLevel );
	};

	int i;
	for ( i=1; i<argc; i++ )
	{
		// handle non-options
		if ( argv[i][0]!='-' )		break;

		// handle no-arg options
		OPT ( "-h", "--help" )		{ ShowHelp(); return 0; }
		OPT ( "-?", "--?" )		{ ShowHelp(); return 0; }
		OPT ( "-v", "--version" )	{ return 0; }
		OPT ( "-q", "--quiet" )		g_bOptQuiet = true;
		OPT1 ( "--console" )		{ g_bOptNoLock = true; g_bOptNoDetach = true; bTestMode = true; }
		OPT1 ( "--stop" )			bOptStop = true;
		OPT1 ( "--stopwait" )		{ bOptStop = true; bOptStopWait = true; }
		OPT1 ( "--status" )			bOptStatus = true;
		OPT1 ( "--pidfile" )		bOptPIDFile = true;
		OPT1 ( "--iostats" )		SetIOStats();
		OPT1 ( "--cpustats" )		SetCPUStats();
		OPT1 ( "--mockstack" )		{ bMeasureStack = true; g_bOptNoLock = true; g_bOptNoDetach = true; }
#if _WIN32
		OPT1 ( "--install" )		{ if ( !WinService() ) { ServiceInstall ( argc, argv ); return 0; } }
		OPT1 ( "--delete" )			{ if ( !WinService() ) { ServiceDelete (); return 0; } }
		OPT1 ( "--ntservice" )		{} // it's valid but handled elsewhere
#else
		OPT1 ( "--nodetach" )		g_bOptNoDetach = true;
#endif
		OPT1 ( "--logdebug" )		UpdateLogLevel ( SPH_LOG_DEBUG );
		OPT1 ( "--logdebugv" )		UpdateLogLevel ( SPH_LOG_VERBOSE_DEBUG );
		OPT1 ( "--logdebugvv" )		UpdateLogLevel ( SPH_LOG_VERY_VERBOSE_DEBUG );
		OPT1 ( "--logreplication" )	UpdateLogLevel ( SPH_LOG_RPL_DEBUG );
		OPT1 ( "--safetrace" )		CrashLogger::SetSafeTrace ( true );
		OPT1 ( "--test" )			{ g_bWatchdog = false; bTestMode = true; } // internal option, do NOT document
		OPT1 ( "--force-pseudo-sharding" ) { bForcePseudoSharding = true; } // internal option, do NOT document
		OPT1 ( "--strip-path" )		g_bStripPath = true;
		OPT1 ( "--vtune" )			g_bVtune = true;
		OPT1 ( "--noqlog" )			bOptDebugQlog = false;
		OPT1 ( "--force-preread" )	bForcedPreread = true;
		OPT1 ( "--coredump" )		CrashLogger::SetCoredump (true);
		OPT1 ( "--new-cluster" )	bNewCluster = true;
		OPT1 ( "--new-cluster-force" )	bNewClusterForce = true;
		OPT1 ( "--no_change_cwd" )	g_bNoChangeCwd = true;

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
		OPT ( "-i", "--index" )		dOptIndexes.Add ( argv[++i] ); // FIXME!!! remove depricated cli option
		OPT ( "-t", "--table" )		dOptIndexes.Add ( argv[++i] );
#if _WIN32
		OPT1 ( "--servicename" )	++i; // it's valid but handled elsewhere
#endif

		// handle unknown options
		else
			break;
	}
	if ( i!=argc )
		sphFatal ( "malformed or unknown option near '%s'; use '-h' or '--help' to see available options.", argv[i] );

	StringBuilder_c sStack;
	DetermineNodeItemStackSize ( sStack );
	DetermineFilterItemStackSize ( sStack );
	DetermineMatchStackSize ( sStack );

	if ( bMeasureStack )
	{
		if ( !WinService() )
		{
			sStack << "export NO_STACK_CALCULATION=1\n";
			fprintf ( stdout, "%s", sStack.cstr() );
		}
		return 0;
	}
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
	if ( !WinService() )
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

	auto dConfig = FetchAndCheckIfChanged ( g_sConfigFile ).second;
	sphInfo( "using config file '%s' (%d chars)...", g_sConfigFile.cstr(), dConfig.GetLength());
	// do parse
	// don't aqcuire wlock, since we're in single main thread here.
	FakeScopedWLock_T<> wFakeLock { g_tRotateConfigMutex };
	if ( !ParseConfig ( &g_hCfg, g_sConfigFile, dConfig ) )
		sphFatal ( "failed to parse config file '%s': %s", g_sConfigFile.cstr (), TlsMsg::szError() );

	dConfig.Reset(); // make valgrind happy (that is not a leak, but produce 'still reachable' message)

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
		return StopOrStopWaitAnother ( hSearchdpre ( "pid_file" ), bOptStopWait );

	////////////////////////////////
	// query running searchd status
	////////////////////////////////

	if ( bOptStatus )
	{
		QueryStatus ( hSearchdpre("listen") );
		return 0;
	}

	/////////////////////
	// configure searchd
	/////////////////////

	sphInitCJson();
	if ( !LoadConfigInt ( hConf, g_sConfigFile, sError ) )
		sphFatal ( "%s", sError.cstr() );

	ConfigureSearchd ( hConf, bOptPIDFile, bTestMode );
	g_sExePath = sphGetCwd();
	CheckSetCwd();
	g_sConfigPath = sphGetCwd();
	sphConfigureCommon ( hConf, FixPathAbsolute ); // this also inits plugins now

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
		g_sPidFile = hSearchdpre["pid_file"].cstr();
		FixPathAbsolute ( g_sPidFile );

		g_iPidFD = ::open ( g_sPidFile.scstr(), O_CREAT | O_WRONLY, S_IREAD | S_IWRITE );
		if ( g_iPidFD<0 )
			sphFatal ( "failed to create pid file '%s': %s", g_sPidFile.scstr(), strerrorm(errno) );
	}
	if ( bOptPIDFile && !sphLockEx ( g_iPidFD, false ) )
		sphFatal ( "failed to lock pid file '%s': %s (searchd already running?)", g_sPidFile.scstr(), strerrorm(errno) );

	g_bPidIsMine = true;

	// Actions on resurrection
	if ( bWatched && !bVisualLoad )
	{
		if ( !LoadConfigInt ( hConf, g_sConfigFile, sError ) )
			sphFatal ( "%s", sError.cstr() );

		auto [bChanged, dNewConfig] = FetchAndCheckIfChanged ( g_sConfigFile );
		if ( bChanged )
		{
			// reparse the config file
			sphInfo ( "Reloading the config (%d chars)", dNewConfig.GetLength() );

			// fake lock is acquired; no warnings will be fired
			if ( !ParseConfig ( &g_hCfg, g_sConfigFile, dNewConfig ) )
				sphFatal ( "failed to parse config file '%s': %s", g_sConfigFile.cstr (), TlsMsg::szError() );

			if ( !LoadConfigInt ( hConf, g_sConfigFile, sError ) )
				sphFatal ( "%s", sError.cstr() );

			sphInfo ( "Reconfigure the daemon" );
			ConfigureSearchd ( hConf, bOptPIDFile, bTestMode );
		}
	}

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
			SetupQueryLogDrain ( hSearchd["query_log"] );

		// create http log if required
		if ( hSearchd.Exists ( "log_http" ) )
			SetupHttpLog ( hSearchd["log_http"] );
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
			return 0;
		}
	}
#endif

	LogTimeZoneStartup(sTZWarning);

	// init before workpool, as last checks binlog
	ModifyDaemonPaths ( hSearchd, FixPathAbsolute );
	sphRTInit ( hSearchd.GetStr ( "binlog_path", bTestMode ? "" : LOCALDATADIR ),
		hSearchd.GetBool ( "binlog_common", val_from_env ( "MANTICORE_BINLOG_COMMON", false ) ),
		hConf("common") ? hConf["common"]("common") : nullptr );
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

	CSphString sSslCert ( hSearchd.GetStr ( "ssl_cert" ) );
	CSphString sSslKey ( hSearchd.GetStr ( "ssl_key" ) );
	CSphString sSslCa ( hSearchd.GetStr ( "ssl_ca" ) );
	FixPathAbsolute ( sSslCert );
	FixPathAbsolute ( sSslKey );
	FixPathAbsolute ( sSslCa );
	SetServerSSLKeys ( sSslCert, sSslKey, sSslCa );
	CheckSSL();

	// set up ping service (if necessary) before loading indexes
	// (since loading ha-mirrors of distributed already assumes ping is usable).
	if ( g_iPingIntervalUs>0 )
		Ping::Start();

	ScheduleMallocTrim();

	CacheCPUInfo();

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

	SetPercolateQueryParserFactory ( CreateQueryParser );
	CallCoroutine ( [&hConf, &dExactIndexes]() REQUIRES_SHARED ( g_tRotateConfigMutex )
	{
		ScopedRole_c thMain ( MainThread );
		ConfigureAndPreloadOnStartup ( hConf, dExactIndexes );
	} );

	///////////
	// startup
	///////////

//	ModifyDaemonPaths ( hSearchd );
//	sphRTInit ( hSearchd, bTestMode, hConf("common") ? hConf["common"]("common") : nullptr );

	if ( hSearchd.Exists ( "snippets_file_prefix" ) )
		g_sSnippetsFilePrefix = hSearchd["snippets_file_prefix"].cstr();
	else
		g_sSnippetsFilePrefix.SetSprintf ( "%s/", g_sExePath.scstr() );
	FixPathAbsolute ( g_sSnippetsFilePrefix );

	bool bLogCompactIn = false;
	LOG_FORMAT eFormat = LOG_FORMAT::SPHINXQL;

	{ // scope for sLogFormat to avoid valgrind's complains
		auto sLogFormat = hSearchd.GetStr ( "query_log_format", "sphinxql" );
		if ( sLogFormat != "sphinxql" )
		{
			StrVec_t dParams;
			sphSplit ( dParams, sLogFormat.cstr() );
			for ( const auto& sParam : dParams )
			{
				if ( sParam=="sphinxql" )
					eFormat = LOG_FORMAT::SPHINXQL;
				else if ( sParam=="plain" )
					eFormat = LOG_FORMAT::_PLAIN;
				else if ( sParam=="compact_in" )
					bLogCompactIn = true;
			}
		}
	}
	if ( bLogCompactIn && eFormat==LOG_FORMAT::_PLAIN )
		sphWarning ( "compact_in option only supported with query_log_format=sphinxql" );
	SetLogFormat ( eFormat );
	SetLogCompact ( bLogCompactIn );

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

	Binlog::Configure ( hSearchd, uReplayFlags );
	SetUidShort ( GetMacAddress(), g_sPidFile, bTestMode );
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
	WinSetConsoleCtrlHandler();
#endif

	Threads::CallCoroutine( [bWatched] {
	StrVec_t dFailed;
	if ( !g_bOptNoDetach && !bWatched && !WinService() )
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
				sphWarning ( "table '%s': lock: %s; TABLE UNUSABLE", tIt.first.cstr(), pIdx->GetLastError().cstr() );
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
		RedirectQueryLogToDaemonLog();

	OpenSyslogIfNecessary();

	/////////////////
	// serve clients
	/////////////////

#if _WIN32
	SetWinServiceRunning();
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
	ReplicationBinlogStart ( GetDataDirInt(), !Binlog::IsActive() );
	
	StartRtBinlogFlushing();

	ScheduleFlushAttrs();
	SetupCompatHttp();

	InitSearchdStats();

	{
		CSphString sSQLStateDefault;
		if ( IsConfigless() )
			sSQLStateDefault.SetSprintf ( "%s/state.sql", GetDataDirInt().cstr() );
		CSphString sSQLStatePath { hSearchd.GetStr ( "sphinxql_state", sSQLStateDefault.scstr() ) };
		FixPathAbsolute ( sSQLStatePath );
		if ( !InitSphinxqlState ( sSQLStatePath, sError ))
			sphWarning ( "sphinxql_state flush disabled: %s", sError.cstr ());
	}

	ServeUserVars ();

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

	g_pTickPoolThread = MakeThreadPool ( g_iNetWorkers, "TickPool" );
	WipeSchedulerOnFork ( g_pTickPoolThread );
	PrepareClustersOnStartup ( dListenerDescs, bNewClusterForce );

	g_dNetLoops.Resize ( g_iNetWorkers );
	for ( auto & pNetLoop : g_dNetLoops )
	{
		pNetLoop = new CSphNetLoop;
		pNetLoop->SetListeners ( g_dListeners );
		if ( !GetAvailableNetLoop() )
			SetAvailableNetLoop ( pNetLoop );
		g_pTickPoolThread->Schedule ( [pNetLoop] { ScopedRole_c thPoll ( NetPoollingThread ); pNetLoop->LoopNetPoll (); }, false );
	}

	// until no threads started, schedule stopping of alone threads to very bottom
	WipeGlobalSchedulerOnShutdownAndFork();
	Detached::MakeAloneIteratorAvailable ();

	// time for replication to sync with cluster
	searchd::AddShutdownCb ( ReplicationServiceShutdown );
	ReplicationServiceStart ( bNewCluster || bNewClusterForce );
	searchd::AddShutdownCb ( BuddyShutdown );
	// --test should not guess buddy path
	// otherwise daemon generates warning message that counts as bad daemon restart by ubertest
	if ( !bTestMode )
		BuddyStart ( g_sBuddyPath, PluginGetDir(), g_bHasBuddyPath, dListenerDescs, g_bTelemetry, MaxChildrenThreads(), g_sConfigFile, RealPath ( GetDataDirInt() ) );

	g_bJsonConfigLoadedOk = true;

	dListenerDescs.Reset(); // make valgrind happy

	// ready, steady, go
	sphInfo ( "accepting connections" );

	// disable startup logging to stdout
	if ( !g_bOptNoDetach )
		StopLogStdout();

	if ( g_bOptQuiet )
		g_eLogLevel = eQuietRestoreLevel;

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
	SetLogger ( &sphLog );
	sphBacktraceSetBinaryName ( argv[0] );
	GeodistInit();

#if _WIN32
	if ( ParseArgsAndStartWinService ( argc, argv, (void*)&ServiceMain ) )
		return 0;
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
