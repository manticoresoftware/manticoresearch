//
// $Id$
//

//
// Copyright (c) 2001-2015, Andrew Aksyonoff
// Copyright (c) 2008-2015, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxutils.h"
#include "sphinxexcerpt.h"
#include "sphinxrt.h"
#include "sphinxint.h"
#include "sphinxquery.h"
#include "sphinxjson.h"
#include "sphinxplugin.h"
#include "sphinxqcache.h"

extern "C"
{
#include "sphinxudf.h"
}

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
#include <locale.h>

#include "searchdaemon.h"
#include "searchdha.h"

#define SEARCHD_BACKLOG			5
#define SPHINXAPI_PORT			9312
#define SPHINXQL_PORT			9306
#define SPH_ADDRESS_SIZE		sizeof("000.000.000.000")
#define SPH_ADDRPORT_SIZE		sizeof("000.000.000.000:00000")
#define MVA_UPDATES_POOL		1048576
#define NETOUTBUF				8192
#define PING_INTERVAL			1000
#define QLSTATE_FLUSH_MSEC		50

// don't shutdown on SIGKILL (debug purposes)
// 1 - SIGKILL will shut down the whole daemon; 0 - watchdog will reincarnate the daemon
#define WATCHDOG_SIGKILL		1

#define SPH_MYSQL_FLAG_MORE_RESULTS 8 // mysql.h: SERVER_MORE_RESULTS_EXISTS

/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
	// Win-specific headers and calls
	#include <io.h>
	#include <tlhelp32.h>

	#define sphSeek		_lseeki64
	#define stat		_stat

#else
	// UNIX-specific headers and calls
	#include <unistd.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <sys/file.h>
	#include <sys/time.h>
	#include <sys/wait.h>
	#include <netdb.h>
	#include <sys/syscall.h>


	// for thr_self()
	#ifdef __FreeBSD__
	#include <sys/thr.h>
	#endif

	#define sphSeek		lseek
#endif

#if USE_SYSLOG
	#include <syslog.h>
#endif

/////////////////////////////////////////////////////////////////////////////

enum ProtocolType_e
{
	PROTO_SPHINX = 0,
	PROTO_MYSQL41,
	PROTO_HTTP,

	PROTO_TOTAL
};


static const char * g_dProtoNames[PROTO_TOTAL] =
{
	"sphinxapi", "sphinxql", "http"
};


static bool				g_bService		= false;
#if USE_WINDOWS
static bool				g_bServiceStop	= false;
static const char *		g_sServiceName	= "searchd";
HANDLE					g_hPipe			= INVALID_HANDLE_VALUE;
#endif

static CSphVector<CSphString>	g_dArgs;


enum LogFormat_e
{
	LOG_FORMAT_PLAIN,
	LOG_FORMAT_SPHINXQL
};

#define					LOG_COMPACT_IN	128						// upto this many IN(..) values allowed in query_log

ESphLogLevel		g_eLogLevel			= SPH_LOG_INFO;		// current log level, can be changed on the fly
static int				g_iLogFile			= STDOUT_FILENO;	// log file descriptor
static bool				g_bLogSyslog		= false;
static bool				g_bQuerySyslog		= false;
static CSphString		g_sLogFile;								// log file name
static bool				g_bLogTty			= false;			// cached isatty(g_iLogFile)
static bool				g_bLogStdout		= true;				// extra copy of startup log messages to stdout; true until around "accepting connections", then MUST be false
static LogFormat_e		g_eLogFormat		= LOG_FORMAT_PLAIN;
static bool				g_bLogCompactIn		= false;			// whether to cut list in IN() clauses.
static int				g_iQueryLogMinMsec	= 0;				// log 'slow' threshold for query

static const int64_t	MS2SEC = I64C ( 1000000 );
int						g_iReadTimeout		= 5;	// sec
static int				g_iWriteTimeout		= 5;
static int				g_iClientTimeout	= 300;
static int				g_iClientQlTimeout	= 900;	// sec
static int				g_iMaxChildren		= 0;
#if !USE_WINDOWS
static bool				g_bPreopenIndexes	= true;
#else
static bool				g_bPreopenIndexes	= false;
#endif
static bool				g_bWatchdog			= true;
static int				g_iExpansionLimit	= 0;
static bool				g_bOnDiskAttrs		= false;
static bool				g_bOnDiskPools		= false;
static int				g_iShutdownTimeout	= 3000000; // default timeout on daemon shutdown and stopwait is 3 seconds
static int				g_iBacklog			= SEARCHD_BACKLOG;
static int				g_iThdPoolCount		= 2;
static int				g_iThdQueueMax		= 0;

struct Listener_t
{
	int					m_iSock;
	bool				m_bTcp;
	ProtocolType_e		m_eProto;
	bool				m_bVIP;
};
static CSphVector<Listener_t>	g_dListeners;

static int				g_iQueryLogFile	= -1;
static CSphString		g_sQueryLogFile;
static CSphString		g_sPidFile;
static int				g_iPidFD		= -1;

static int				g_iMaxCachedDocs	= 0;	// in bytes
static int				g_iMaxCachedHits	= 0;	// in bytes

static int				g_iAttrFlushPeriod	= 0;			// in seconds; 0 means "do not flush"
int				g_iMaxPacketSize	= 8*1024*1024;	// in bytes; for both query packets from clients and response packets from agents
static int				g_iMaxFilters		= 256;
static int				g_iMaxFilterValues	= 4096;
static int				g_iMaxBatchQueries	= 32;
static ESphCollation	g_eCollation = SPH_COLLATION_DEFAULT;
static CSphString		g_sSnippetsFilePrefix;

static ISphThdPool *	g_pThdPool			= NULL;
int				g_iDistThreads		= 0;

int				g_iAgentConnectTimeout = 1000;
int				g_iAgentQueryTimeout = 3000;	// global (default). May be override by index-scope values, if one specified

const int	MAX_RETRY_COUNT		= 8;
const int	MAX_RETRY_DELAY		= 1000;

static int				g_iAgentRetryCount = 0;
static int				g_iAgentRetryDelay = MAX_RETRY_DELAY/2;	// global (default) values. May be override by the query options 'retry_count' and 'retry_timeout'


struct ListNode_t
{
	ListNode_t * m_pPrev;
	ListNode_t * m_pNext;
	ListNode_t ()
		: m_pPrev ( NULL )
		, m_pNext ( NULL )
	{ }
};


class List_t
{
public:
	List_t ()
	{
		m_tStub.m_pPrev = &m_tStub;
		m_tStub.m_pNext = &m_tStub;
		m_iCount = 0;
	}

	void Add ( ListNode_t * pNode )
	{
		assert ( !pNode->m_pNext && !pNode->m_pPrev );
		pNode->m_pNext = m_tStub.m_pNext;
		pNode->m_pPrev = &m_tStub;
		m_tStub.m_pNext->m_pPrev = pNode;
		m_tStub.m_pNext = pNode;

		m_iCount++;
	}

	void Remove ( ListNode_t * pNode )
	{
		assert ( pNode->m_pNext && pNode->m_pPrev );
		pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		pNode->m_pPrev->m_pNext = pNode->m_pNext;
		pNode->m_pNext = NULL;
		pNode->m_pPrev = NULL;

		m_iCount--;
	}

	int GetLength () const
	{
		return m_iCount;
	}

	const ListNode_t * Begin () const
	{
		return m_tStub.m_pNext;
	}

	const ListNode_t * End () const
	{
		return &m_tStub;
	}

private:
	ListNode_t	m_tStub;	// stub node
	int			m_iCount;	// elements counter
};


enum ThdState_e
{
	THD_HANDSHAKE = 0,
	THD_NET_READ,
	THD_NET_WRITE,
	THD_QUERY,
	THD_NET_IDLE,

	THD_STATE_TOTAL
};

static const char * g_dThdStates[THD_STATE_TOTAL] = {
	"handshake", "net_read", "net_write", "query", "net_idle"
};

struct ThdDesc_t : public ListNode_t
{
	SphThread_t		m_tThd;
	ProtocolType_e	m_eProto;
	int				m_iClientSock;
	CSphString		m_sClientName;

	ThdState_e		m_eThdState;
	const char *	m_sCommand;
	int				m_iConnID;		///< current conn-id for this thread

	// stuff for SHOW THREADS
	int				m_iTid;			///< OS thread id, or 0 if unknown
	int64_t			m_tmConnect;	///< when did the client connect?
	int64_t			m_tmStart;		///< when did the current request start?
	CSphFixedVector<char> m_dBuf;	///< current request description

	ThdDesc_t ()
		: m_eProto ( PROTO_MYSQL41 )
		, m_iClientSock ( 0 )
		, m_eThdState ( THD_HANDSHAKE )
		, m_sCommand ( NULL )
		, m_iConnID ( -1 )
		, m_iTid ( 0 )
		, m_tmConnect ( 0 )
		, m_tmStart ( 0 )
		, m_dBuf ( 512 )
	{
		m_dBuf[0] = '\0';
		m_dBuf.Last() = '\0';
	}

	void SetSnippetThreadInfo ( const CSphVector<ExcerptQuery_t> & dSnippets )
	{
		int64_t iSize = 0;
		ARRAY_FOREACH ( i, dSnippets )
		{
			if ( dSnippets[i].m_iSize )
				iSize -= dSnippets[i].m_iSize; // because iSize negative for sorting purpose
			else
				iSize += dSnippets[i].m_sSource.Length();
		}

		iSize /= 100;
		SetThreadInfo ( "api-snippet datasize=%d.%d""k query=\"%s\"", int ( iSize/10 ), int ( iSize%10 ), dSnippets[0].m_sWords.scstr() );
	}

	void SetThreadInfo ( const char * sTemplate, ... )
	{
		va_list ap;

		va_start ( ap, sTemplate );
		int iPrinted = vsnprintf ( m_dBuf.Begin(), m_dBuf.GetLength()-1, sTemplate, ap );
		va_end ( ap );

		if ( iPrinted>0 )
			m_dBuf[Min ( iPrinted, m_dBuf.GetLength()-1 )] = '\0';
	}
};


static CSphMutex				g_tThdMutex;
static List_t					g_dThd;				///< existing threads table

static int						g_iConnectionID = 0;		///< global conn-id

// handshake
static char						g_sMysqlHandshake[128];
static int						g_iMysqlHandshake = 0;

//////////////////////////////////////////////////////////////////////////

static CSphString		g_sConfigFile;
static DWORD			g_uCfgCRC32		= 0;
static struct stat		g_tCfgStat;

static CSphConfigParser g_pCfg;

#if USE_WINDOWS
static bool				g_bSeamlessRotate	= false;
#else
static bool				g_bSeamlessRotate	= true;
#endif

static bool				g_bIOStats		= false;
static bool				g_bCpuStats		= false;
static bool				g_bOptNoDetach	= false;
static bool				g_bOptNoLock	= false;
static bool				g_bSafeTrace	= false;
static bool				g_bStripPath	= false;

static volatile bool	g_bDoDelete			= false;	// do we need to delete any indexes?

static volatile sig_atomic_t g_bGotSighup		= 0;	// we just received SIGHUP; need to log
static volatile sig_atomic_t g_bGotSigterm		= 0;	// we just received SIGTERM; need to shutdown
static volatile sig_atomic_t g_bGotSigusr1		= 0;	// we just received SIGUSR1; need to reopen logs

// pipe to watchdog to inform that daemon is going to close, so no need to restart it in case of crash
static CSphLargeBuffer<DWORD, true>	g_bDaemonAtShutdown;
volatile bool					g_bShutdown = false;
static CSphLargeBuffer<DWORD, true>	g_bHaveTTY;

IndexHash_c *								g_pLocalIndexes = new IndexHash_c();	// served (local) indexes hash
IndexHash_c *								g_pTemplateIndexes = new IndexHash_c(); // template (tokenizer) indexes hash

static CSphMutex							g_tRotateQueueMutex;
static CSphVector<CSphString>				g_dRotateQueue;
static CSphMutex							g_tRotateConfigMutex;
static SphThread_t							g_tRotateThread;
static SphThread_t							g_tRotationServiceThread;
static volatile bool						g_bInvokeRotationService = false;
static volatile bool						g_bNeedRotate = false;		// true if there were pending HUPs to handle (they could fly in during previous rotate)
static volatile bool						g_bInRotate = false;		// true while we are rotating

static CSphVector<SphThread_t>				g_dTickPoolThread;

/// flush parameters of rt indexes
static SphThread_t							g_tRtFlushThread;

// optimize thread
static SphThread_t							g_tOptimizeThread;
static CSphMutex							g_tOptimizeQueueMutex;
static CSphVector<CSphString>				g_dOptimizeQueue;
static ThrottleState_t						g_tRtThrottle;

static CSphMutex							g_tDistLock;
static CSphMutex							g_tPersLock;

static CSphAtomic							g_iPersistentInUse;

/// master-agent API protocol extensions version
enum
{
	VER_MASTER = 11
};


/// command names
static const char * g_dApiCommands[SEARCHD_COMMAND_TOTAL] =
{
	"search", "excerpt", "update", "keywords", "persist", "status", "query", "flushattrs", "query", "ping", "delete", "uvar"
};

//////////////////////////////////////////////////////////////////////////

const char * AgentStats_t::m_sNames[eMaxStat]=
	{ "query_timeouts", "connect_timeouts", "connect_failures",
		"network_errors", "wrong_replies", "unexpected_closings",
		"warnings", "succeeded_queries", "total_query_time", "connect_count",
		"connect_avg", "connect_max" };

static CSphQueryResultMeta		g_tLastMeta;
static CSphMutex				g_tLastMetaMutex;

//////////////////////////////////////////////////////////////////////////

struct FlushState_t
{
	int		m_bFlushing;		///< update flushing in progress
	int		m_iFlushTag;		///< last flushed tag
	bool	m_bForceCheck;		///< forced check/flush flag
};

static FlushState_t				g_tFlush;

//////////////////////////////////////////////////////////////////////////

/// available uservar types
enum Uservar_e
{
	USERVAR_INT_SET
};

/// uservar name to value binding
struct Uservar_t
{
	Uservar_e			m_eType;
	UservarIntSet_c *	m_pVal;

	Uservar_t ()
		: m_eType ( USERVAR_INT_SET )
		, m_pVal ( NULL )
	{}
};

static CSphMutex					g_tUservarsMutex;
static SmallStringHash_T<Uservar_t>	g_hUservars;

static volatile int64_t				g_tmSphinxqlState; // last state (uservars+udfs+...) update timestamp
static SphThread_t					g_tSphinxqlStateFlushThread;
static CSphString					g_sSphinxqlState;

/////////////////////////////////////////////////////////////////////////////
// MISC
/////////////////////////////////////////////////////////////////////////////

void ReleaseTTYFlag()
{
	if ( !g_bHaveTTY.IsEmpty() )
		*g_bHaveTTY.GetWritePtr() = 1;
}

ServedDesc_t::ServedDesc_t ()
	: m_pIndex ( NULL )
	, m_bEnabled ( true )
	, m_bMlock ( false )
	, m_bPreopen ( false )
	, m_bExpand ( false )
	, m_bToDelete ( false )
	, m_bOnlyNew ( false )
	, m_bRT ( false )
	, m_bOnDiskAttrs ( false )
	, m_bOnDiskPools ( false )
	, m_iMass ( 0 )
{}

ServedDesc_t::~ServedDesc_t ()
{
	SafeDelete ( m_pIndex );
}

ServedIndex_c::~ServedIndex_c ()
{
	Verify ( m_tLock.Done() );
}

void ServedIndex_c::ReadLock () const
{
	if ( m_tLock.ReadLock() )
		sphLogDebugvv ( "ReadLock %p", this );
	else
	{
		sphLogDebug ( "ReadLock %p failed", this );
		assert ( false );
	}
}

void ServedIndex_c::WriteLock () const
{
	if ( m_tLock.WriteLock() )
		sphLogDebugvv ( "WriteLock %p", this );
	else
	{
		sphLogDebug ( "WriteLock %p failed", this );
		assert ( false );
	}
}

bool ServedIndex_c::InitLock() const
{
	return m_tLock.Init();
}

void ServedIndex_c::Unlock () const
{
	if ( m_tLock.Unlock() )
		sphLogDebugvv ( "Unlock %p", this );
	else
	{
		sphLogDebug ( "Unlock %p failed", this );
		assert ( false );
	}
}

//////////////////////////////////////////////////////////////////////////

IndexHashIterator_c::IndexHashIterator_c ( const IndexHash_c * pHash, bool bWrite )
	: m_pHash ( pHash )
	, m_pIterator ( NULL )
{
	if ( !bWrite )
		m_pHash->Rlock();
	else
		m_pHash->Wlock();
}

IndexHashIterator_c::~IndexHashIterator_c ()
{
	m_pHash->Unlock();
}

bool IndexHashIterator_c::Next ()
{
	m_pIterator = m_pIterator ? m_pIterator->m_pNextByOrder : m_pHash->m_pFirstByOrder;
	return m_pIterator!=NULL;
}

ServedIndex_c & IndexHashIterator_c::Get ()
{
	assert ( m_pIterator );
	return m_pIterator->m_tValue;
}

const CSphString & IndexHashIterator_c::GetKey ()
{
	assert ( m_pIterator );
	return m_pIterator->m_tKey;
}

//////////////////////////////////////////////////////////////////////////

IndexHash_c::IndexHash_c ()
{
	if ( !m_tLock.Init() )
		sphDie ( "failed to init hash indexes rwlock" );
}


IndexHash_c::~IndexHash_c()
{
	Verify ( m_tLock.Done() );
}


void IndexHash_c::Rlock () const
{
	Verify ( m_tLock.ReadLock() );
}


void IndexHash_c::Wlock () const
{
	Verify ( m_tLock.WriteLock() );
}


void IndexHash_c::Unlock () const
{
	Verify ( m_tLock.Unlock() );
}


bool IndexHash_c::Add ( const ServedDesc_t & tDesc, const CSphString & tKey )
{
	Wlock();
	int iPrevSize = GetLength ();
	ServedIndex_c & tVal = BASE::AddUnique ( tKey );
	bool bAdded = ( iPrevSize<GetLength() );
	if ( bAdded )
	{
		*( (ServedDesc_t *)&tVal ) = tDesc;
		Verify ( tVal.InitLock() );
	}
	Unlock();
	return bAdded;
}


bool IndexHash_c::Delete ( const CSphString & tKey )
{
	// tricky part
	// hash itself might be unlocked, but entry (!) might still be locked
	// hence, we also need to acquire a lock on entry, and an exclusive one
	Wlock();
	bool bRes = false;
	ServedIndex_c * pEntry = BASE::operator() ( tKey );
	if ( pEntry )
	{
		pEntry->WriteLock();
		pEntry->Unlock();
		bRes = BASE::Delete ( tKey );
	}
	Unlock();
	return bRes;
}


const ServedIndex_c * IndexHash_c::GetRlockedEntry ( const CSphString & tKey ) const
{
	Rlock();
	ServedIndex_c * pEntry = BASE::operator() ( tKey );
	if ( pEntry )
		pEntry->ReadLock();
	Unlock();
	return pEntry;
}


ServedIndex_c * IndexHash_c::GetWlockedEntry ( const CSphString & tKey ) const
{
	Rlock();
	ServedIndex_c * pEntry = BASE::operator() ( tKey );
	if ( pEntry )
		pEntry->WriteLock();
	Unlock();
	return pEntry;
}


ServedIndex_c & IndexHash_c::GetUnlockedEntry ( const CSphString & tKey ) const
{
	Rlock();
	ServedIndex_c & tRes = BASE::operator[] ( tKey );
	Unlock();
	return tRes;
}

ServedIndex_c * IndexHash_c::GetUnlockedEntryPtr ( const CSphString & tKey ) const
{
	Rlock();
	ServedIndex_c * pRes = BASE::operator() ( tKey );
	Unlock();
	return pRes;
}



bool IndexHash_c::Exists ( const CSphString & tKey ) const
{
	Rlock();
	bool bRes = BASE::Exists ( tKey );
	Unlock();
	return bRes;
}


/////////////////////////////////////////////////////////////////////////////
// LOGGING
/////////////////////////////////////////////////////////////////////////////

void Shutdown (); // forward ref for sphFatal()
int GetOsThreadId ();


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
			sphWrite ( g_iLogFile, sTtyBuf, strlen(sTtyBuf) );
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
		const int levels[] = { LOG_EMERG, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG };
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

	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf)-1, "[%s] [%d] ", sTimeBuf, GetOsThreadId() );

	char * sTtyBuf = sBuf + strlen(sBuf);
	strncpy ( sTtyBuf, sBanner, 32 ); // 32 is arbitrary; just something that is enough and keeps lint happy

	int iLen = strlen(sBuf);

	// format the message
	if ( sFmt )
	{
		// need more space for tail zero and "\n" that added at sphLogEntry
		int iSafeGap = 4;
		int iBufSize = sizeof(sBuf)-iLen-iSafeGap;
		vsnprintf ( sBuf+iLen, iBufSize, sFmt, ap );
		sBuf[ sizeof(sBuf)-iSafeGap ] = '\0';
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
		strncpy ( sLast, sBuf, iLen );
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


void sphFatal ( const char * sFmt, ... ) __attribute__ ( ( format ( printf, 1, 2 ) ) );
void sphFatal ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	sphLog ( SPH_LOG_FATAL, sFmt, ap );
	va_end ( ap );
	Shutdown ();
	exit ( 1 );
}


void sphFatalLog ( const char * sFmt, ... ) __attribute__ ( ( format ( printf, 1, 2 ) ) );
void sphFatalLog ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	sphLog ( SPH_LOG_FATAL, sFmt, ap );
	va_end ( ap );
}


#if !USE_WINDOWS
static CSphString GetNamedPipeName ( int iPid )
{
	CSphString sRes;
	sRes.SetSprintf ( "/tmp/searchd_%d", iPid );
	return sRes;
}
#endif

void LogWarning ( const char * sWarning )
{
	sphWarning ( "%s", sWarning );
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
	CSphString	m_sIndex;	///< searched index name
	CSphString	m_sError;	///< search error message

public:
	SearchFailure_t () {}

public:
	bool operator == ( const SearchFailure_t & r ) const
	{
		return m_sIndex==r.m_sIndex && m_sError==r.m_sError;
	}

	bool operator < ( const SearchFailure_t & r ) const
	{
		int iRes = CmpString ( m_sError.cstr(), r.m_sError.cstr() );
		if ( !iRes )
			iRes = CmpString ( m_sIndex.cstr(), r.m_sIndex.cstr() );
		return iRes<0;
	}

	const SearchFailure_t & operator = ( const SearchFailure_t & r )
	{
		if ( this!=&r )
		{
			m_sIndex = r.m_sIndex;
			m_sError = r.m_sError;
		}
		return *this;
	}
};


class SearchFailuresLog_c
{
protected:
	CSphVector<SearchFailure_t>		m_dLog;

public:
	void Submit ( const char * sIndex, const char * sError )
	{
		SearchFailure_t & tEntry = m_dLog.Add ();
		tEntry.m_sIndex = sIndex;
		tEntry.m_sError = sError;
	}

	void SubmitEx ( const char * sIndex, const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 3, 4 ) ) )
	{
		SearchFailure_t & tEntry = m_dLog.Add ();
		va_list ap;
		va_start ( ap, sTemplate );
		tEntry.m_sIndex = sIndex;
		tEntry.m_sError.SetSprintfVa ( sTemplate, ap );
		va_end ( ap );
	}

public:
	bool IsEmpty ()
	{
		return m_dLog.GetLength()==0;
	}

	void BuildReport ( CSphStringBuilder & sReport )
	{
		if ( IsEmpty() )
			return;

		// collapse same messages
		m_dLog.Uniq ();
		int iSpanStart = 0;

		for ( int i=1; i<=m_dLog.GetLength(); i++ )
		{
			// keep scanning while error text is the same
			if ( i!=m_dLog.GetLength() )
				if ( m_dLog[i].m_sError==m_dLog[i-1].m_sError )
					continue;

			// build current span
			CSphStringBuilder sSpan;
			if ( iSpanStart )
				sSpan += ";\n";
			sSpan += "index ";
			for ( int j=iSpanStart; j<i; j++ )
			{
				if ( j!=iSpanStart )
					sSpan += ",";
				sSpan += m_dLog[j].m_sIndex.cstr();
			}
			sSpan += ": ";
			sSpan += m_dLog[iSpanStart].m_sError.cstr();

			// flush current span
			sReport += sSpan.cstr();

			// done
			iSpanStart = i;
		}
	}
};

/////////////////////////////////////////////////////////////////////////////
// SIGNAL HANDLERS
/////////////////////////////////////////////////////////////////////////////


static bool SaveIndexes ()
{
	CSphString sError;
	bool bAllSaved = true;
	for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
	{
		const ServedIndex_c & tServed = it.Get();
		if ( !tServed.m_bEnabled )
			continue;

		tServed.ReadLock();
		if ( !tServed.m_pIndex->SaveAttributes ( sError ) )
		{
			sphWarning ( "index %s: attrs save failed: %s", it.GetKey().cstr(), sError.cstr() );
			bAllSaved = false;
		}
		tServed.Unlock();
	}
	return bAllSaved;
}


void Shutdown ()
{
#if !USE_WINDOWS
	int fdStopwait = -1;
#endif
	bool bAttrsSaveOk = true;
	g_bShutdown = true;
	if ( !g_bDaemonAtShutdown.IsEmpty() )
	{
		*g_bDaemonAtShutdown.GetWritePtr() = 1;
	}

#if !USE_WINDOWS
	// stopwait handshake
	CSphString sPipeName = GetNamedPipeName ( getpid() );
	fdStopwait = ::open ( sPipeName.cstr(), O_WRONLY | O_NONBLOCK );
	if ( fdStopwait>=0 )
	{
		DWORD uHandshakeOk = 0;
		int iDummy; // to avoid gcc unused result warning
		iDummy = ::write ( fdStopwait, &uHandshakeOk, sizeof(DWORD) );
		iDummy++; // to avoid gcc set but not used variable warning
	}
#endif

	// tell flush-rt thread to shutdown, and wait until it does
	sphThreadJoin ( &g_tRtFlushThread );

	// tell rotation thread to shutdown, and wait until it does
	if ( g_bSeamlessRotate )
	{
		sphThreadJoin ( &g_tRotateThread );
	}

	// tell uservars flush thread to shutdown, and wait until it does
	if ( !g_sSphinxqlState.IsEmpty() )
		sphThreadJoin ( &g_tSphinxqlStateFlushThread );

	sphThreadJoin ( &g_tOptimizeThread );

	int64_t tmShutStarted = sphMicroTimer();
	// stop search threads; up to shutdown_timeout seconds
	while ( g_dThd.GetLength() > 0 && ( sphMicroTimer()-tmShutStarted )<g_iShutdownTimeout )
		sphSleepMsec ( 50 );

	if ( g_pThdPool )
	{
		g_pThdPool->Shutdown();
		SafeDelete ( g_pThdPool );
		ARRAY_FOREACH ( i, g_dTickPoolThread )
			sphThreadJoin ( g_dTickPoolThread.Begin() + i );
	}

	sphThreadJoin ( &g_tRotationServiceThread );

	CSphString sError;
	// save attribute updates for all local indexes
	bAttrsSaveOk = SaveIndexes();

	// unlock indexes and release locks if needed
	for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
		if ( it.Get().m_pIndex )
			it.Get().m_pIndex->Unlock();
	g_pLocalIndexes->Reset();

	// unlock indexes and release locks if needed
	for ( IndexHashIterator_c it ( g_pTemplateIndexes ); it.Next(); )
		if ( it.Get().m_pIndex )
			it.Get().m_pIndex->Unlock();

	g_pTemplateIndexes->Reset();

	// clear shut down of rt indexes + binlog
	SafeDelete ( g_pLocalIndexes );
	SafeDelete ( g_pTemplateIndexes );
	sphDoneIOStats();
	sphRTDone();

	sphShutdownWordforms ();
	sphShutdownGlobalIDFs ();
	sphAotShutdown ();

	ARRAY_FOREACH ( i, g_dListeners )
		if ( g_dListeners[i].m_iSock>=0 )
			sphSockClose ( g_dListeners[i].m_iSock );

	ClosePersistentSockets();

	// remove pid
	if ( !g_sPidFile.IsEmpty() )
	{
		::close ( g_iPidFD );
		::unlink ( g_sPidFile.cstr() );
	}

	sphInfo ( "shutdown complete" );

	SphCrashLogger_c::Done();
	sphThreadDone ( g_iLogFile );

#if USE_WINDOWS
	CloseHandle ( g_hPipe );
#else
	if ( fdStopwait>=0 )
	{
		DWORD uStatus = bAttrsSaveOk;
		int iDummy; // to avoid gcc unused result warning
		iDummy = ::write ( fdStopwait, &uStatus, sizeof(DWORD) );
		iDummy++; // to avoid gcc set but not used variable warning
		::close ( fdStopwait );
	}
#endif
}

#if !USE_WINDOWS
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
	g_bGotSigterm = 1;
	sphInterruptNow();
}


void sigusr1 ( int )
{
	g_bGotSigusr1 = 1;
}
#endif // !USE_WINDOWS


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

typedef bool CopyQuery_fn ( QueryCopyState_t & tState );

#define SPH_TIME_PID_MAX_SIZE 256
const char		g_sCrashedBannerAPI[] = "\n--- crashed SphinxAPI request dump ---\n";
const char		g_sCrashedBannerMySQL[] = "\n--- crashed SphinxQL request dump ---\n";
const char		g_sCrashedBannerTail[] = "\n--- request dump end ---\n";
#if USE_WINDOWS
const char		g_sMinidumpBanner[] = "minidump located at: ";
const char		g_sMemoryStatBanner[] = "\n--- memory statistics ---\n";
#endif
static BYTE		g_dCrashQueryBuff [4096];
static char		g_sCrashInfo [SPH_TIME_PID_MAX_SIZE] = "[][]\n";
static int		g_iCrashInfoLen = 0;

#if USE_WINDOWS
static char		g_sMinidump[SPH_TIME_PID_MAX_SIZE] = "";
#endif

SphThreadKey_t SphCrashLogger_c::m_tTLS = SphThreadKey_t ();

// lets invalidate pointer when this instance goes out of scope to get immediate crash
// instead of a reference to incorrect stack frame in case of some programming error
SphCrashLogger_c::~SphCrashLogger_c () { sphThreadSet ( m_tTLS, NULL ); }

void SphCrashLogger_c::Init ()
{
	Verify ( sphThreadKeyCreate ( &m_tTLS ) );
}

void SphCrashLogger_c::Done ()
{
	sphThreadKeyDelete ( m_tTLS );
}


#if !USE_WINDOWS
void SphCrashLogger_c::HandleCrash ( int sig )
#else
LONG WINAPI SphCrashLogger_c::HandleCrash ( EXCEPTION_POINTERS * pExc )
#endif // !USE_WINDOWS
{
	if ( g_iLogFile<0 )
		CRASH_EXIT;

	// log [time][pid]
	sphSeek ( g_iLogFile, 0, SEEK_END );
	sphWrite ( g_iLogFile, g_sCrashInfo, g_iCrashInfoLen );

	// log query
	CrashQuery_t tQuery = SphCrashLogger_c::GetQuery();

	// request dump banner
	int iBannerLen = ( tQuery.m_bMySQL ? sizeof(g_sCrashedBannerMySQL) : sizeof(g_sCrashedBannerAPI) ) - 1;
	const char * pBanner = tQuery.m_bMySQL ? g_sCrashedBannerMySQL : g_sCrashedBannerAPI;
	sphWrite ( g_iLogFile, pBanner, iBannerLen );

	// query
	if ( tQuery.m_iSize )
	{
		QueryCopyState_t tCopyState;
		tCopyState.m_pDst = g_dCrashQueryBuff;
		tCopyState.m_pDstEnd = g_dCrashQueryBuff + sizeof(g_dCrashQueryBuff);
		tCopyState.m_pSrc = tQuery.m_pQuery;
		tCopyState.m_pSrcEnd = tQuery.m_pQuery + tQuery.m_iSize;

		CopyQuery_fn * pfnCopy = NULL;
		if ( !tQuery.m_bMySQL )
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

	sphSafeInfo ( g_iLogFile, "Sphinx " SPHINX_VERSION );

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
	// FIXME? should we try to lock threads table somehow?
	sphSafeInfo ( g_iLogFile, "--- %d active threads ---", g_dThd.GetLength() );
	const ListNode_t * pIt = g_dThd.Begin();
	int iThd = 0;
	while ( pIt!=g_dThd.End() )
	{
		ThdDesc_t * pThd = (ThdDesc_t *)pIt;
		sphSafeInfo ( g_iLogFile, "thd %d, proto %s, state %s, command %s",
			iThd,
			g_dProtoNames[pThd->m_eProto],
			g_dThdStates[pThd->m_eThdState],
			pThd->m_sCommand ? pThd->m_sCommand : "-" );
		pIt = pIt->m_pNext;
		iThd++;
	}

	// memory info
#if SPH_ALLOCS_PROFILER
	sphWrite ( g_iLogFile, g_sMemoryStatBanner, sizeof ( g_sMemoryStatBanner )-1 );
	sphMemStatDump ( g_iLogFile );
#endif

	sphSafeInfo ( g_iLogFile, "------- CRASH DUMP END -------" );

	CRASH_EXIT;
}

void SphCrashLogger_c::SetLastQuery ( const CrashQuery_t & tQuery )
{
	SphCrashLogger_c * pCrashLogger = (SphCrashLogger_c *)sphThreadGet ( m_tTLS );
	assert ( pCrashLogger );
	pCrashLogger->m_tQuery = tQuery;
}

void SphCrashLogger_c::SetupTimePID ()
{
	char sTimeBuf[SPH_TIME_PID_MAX_SIZE];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	g_iCrashInfoLen = snprintf ( g_sCrashInfo, SPH_TIME_PID_MAX_SIZE-1,
		"------- FATAL: CRASH DUMP -------\n[%s] [%5d]\n", sTimeBuf, (int)getpid() );
}

void SphCrashLogger_c::SetupTLS ()
{
	Verify ( sphThreadSet ( m_tTLS, this ) );
}

static CrashQuery_t g_tDummyQuery;

CrashQuery_t SphCrashLogger_c::GetQuery()
{
	SphCrashLogger_c * pCrashLogger = (SphCrashLogger_c *)sphThreadGet ( m_tTLS );
	return ( pCrashLogger ? pCrashLogger->m_tQuery : g_tDummyQuery );
}

bool SphCrashLogger_c::ThreadCreate ( SphThread_t * pThread, void (*pCall)(void*), void * pArg, bool bDetached )
{
	CallArgPair_t * pWrapperArg = new CallArgPair_t ( pCall, pArg );
	bool bSuccess = sphThreadCreate ( pThread, ThreadWrapper, pWrapperArg, bDetached );
	if ( !bSuccess )
		delete pWrapperArg;
	return bSuccess;
}

void SphCrashLogger_c::ThreadWrapper ( void * pArg )
{
	CallArgPair_t * pPair = static_cast<CallArgPair_t *> ( pArg );
	SphCrashLogger_c tQueryTLS;
	tQueryTLS.SetupTLS();
	pPair->m_pCall ( pPair->m_pArg );
	delete pPair;
}


#if USE_WINDOWS
void SetSignalHandlers ( bool )
{
	SphCrashLogger_c::Init();
	snprintf ( g_sMinidump, SPH_TIME_PID_MAX_SIZE-1, "%s.%d", g_sPidFile.scstr(), (int)getpid() );
	SetUnhandledExceptionFilter ( SphCrashLogger_c::HandleCrash );
}
#else
void SetSignalHandlers ( bool bAllowCtrlC=false )
{
	SphCrashLogger_c::Init();
	struct sigaction sa;
	sigfillset ( &sa.sa_mask );
	sa.sa_flags = SA_NOCLDSTOP;

	bool bSignalsSet = false;
	for ( ;; )
	{
		sa.sa_handler = sigterm;	if ( sigaction ( SIGTERM, &sa, NULL )!=0 ) break;
		if ( !bAllowCtrlC )
		{
			sa.sa_handler = sigterm;
			if ( sigaction ( SIGINT, &sa, NULL )!=0 )
				break;
		}
		sa.sa_handler = sighup;		if ( sigaction ( SIGHUP, &sa, NULL )!=0 ) break;
		sa.sa_handler = sigusr1;	if ( sigaction ( SIGUSR1, &sa, NULL )!=0 ) break;
		sa.sa_handler = SIG_IGN;	if ( sigaction ( SIGPIPE, &sa, NULL )!=0 ) break;

		sa.sa_flags |= SA_RESETHAND;
		sa.sa_handler = SphCrashLogger_c::HandleCrash;	if ( sigaction ( SIGSEGV, &sa, NULL )!=0 ) break;
		sa.sa_handler = SphCrashLogger_c::HandleCrash;	if ( sigaction ( SIGBUS, &sa, NULL )!=0 ) break;
		sa.sa_handler = SphCrashLogger_c::HandleCrash;	if ( sigaction ( SIGABRT, &sa, NULL )!=0 ) break;
		sa.sa_handler = SphCrashLogger_c::HandleCrash;	if ( sigaction ( SIGILL, &sa, NULL )!=0 ) break;
		sa.sa_handler = SphCrashLogger_c::HandleCrash;	if ( sigaction ( SIGFPE, &sa, NULL )!=0 ) break;

		bSignalsSet = true;
		break;
	}
	if ( !bSignalsSet )
		sphFatal ( "sigaction(): %s", strerror(errno) );
}
#endif


/////////////////////////////////////////////////////////////////////////////
// NETWORK STUFF
/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS

const int		WIN32_PIPE_BUFSIZE		= 32;

/// on Windows, the wrapper just prevents the warnings
void sphFDSet ( int fd, fd_set * fdset )
{
	#pragma warning(push) // store current warning values
	#pragma warning(disable:4127) // conditional expr is const
	#pragma warning(disable:4389) // signed/unsigned mismatch

	FD_SET ( fd, fdset );

	#pragma warning(pop) // restore warnings
}

#else // !USE_WINDOWS

#define SPH_FDSET_OVERFLOW(_fd) ( (_fd)<0 || (_fd)>=(int)FD_SETSIZE )

/// on UNIX, we also check that the descript won't corrupt the stack
void sphFDSet ( int fd, fd_set * set )
{
	if ( SPH_FDSET_OVERFLOW(fd) )
		sphFatal ( "sphFDSet() failed fd=%d, FD_SETSIZE=%d", fd, FD_SETSIZE );
	else
		FD_SET ( fd, set );
}

#endif // USE_WINDOWS


#if USE_WINDOWS
const char * sphSockError ( int iErr )
{
	if ( iErr==0 )
		iErr = WSAGetLastError ();

	static char sBuf [ 256 ];
	_snprintf ( sBuf, sizeof(sBuf), "WSA error %d", iErr );
	return sBuf;
}
#else
const char * sphSockError ( int )
{
	return strerror ( errno );
}
#endif


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


int sphSockPeekErrno ()
{
	int iRes = sphSockGetErrno();
	sphSockSetErrno ( iRes );
	return iRes;
}


/// formats IP address given in network byte order into sBuffer
/// returns the buffer
char * sphFormatIP ( char * sBuffer, int iBufferSize, DWORD uAddress )
{
	const BYTE *a = (const BYTE *)&uAddress;
	snprintf ( sBuffer, iBufferSize, "%u.%u.%u.%u", a[0], a[1], a[2], a[3] );
	return sBuffer;
}


static const bool GETADDR_STRICT = true; ///< strict check, will die with sphFatal() on failure

DWORD sphGetAddress ( const char * sHost, bool bFatal )
{
	struct hostent * pHost = gethostbyname ( sHost );

	if ( pHost==NULL || pHost->h_addrtype!=AF_INET )
	{
		if ( bFatal )
			sphFatal ( "no AF_INET address found for: %s", sHost );
		return 0;
	}

	struct in_addr ** ppAddrs = (struct in_addr **)pHost->h_addr_list;
	assert ( ppAddrs[0] );

	assert ( sizeof(DWORD)==pHost->h_length );
	DWORD uAddr;
	memcpy ( &uAddr, ppAddrs[0], sizeof(DWORD) );

	if ( ppAddrs[1] )
	{
		char sBuf [ SPH_ADDRESS_SIZE ];
		sphWarning ( "multiple addresses found for '%s', using the first one (ip=%s)",
			sHost, sphFormatIP ( sBuf, sizeof(sBuf), uAddr ) );
	}

	return uAddr;
}


#if !USE_WINDOWS
int sphCreateUnixSocket ( const char * sPath )
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


int sphCreateInetSocket ( DWORD uAddr, int iPort )
{
	char sAddress[SPH_ADDRESS_SIZE];
	sphFormatIP ( sAddress, SPH_ADDRESS_SIZE, uAddr );

	if ( uAddr==htonl ( INADDR_ANY ) )
		sphInfo ( "listening on all interfaces, port=%d", iPort );
	else
		sphInfo ( "listening on %s:%d", sAddress, iPort );

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
		sphWarning ( "setsockopt() failed: %s", sphSockError() );
#ifdef TCP_NODELAY
	if ( setsockopt ( iSock, IPPROTO_TCP, TCP_NODELAY, (char*)&iOn, sizeof(iOn) ) )
		sphWarning ( "setsockopt() failed: %s", sphSockError() );
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


bool IsPortInRange ( int iPort )
{
	return ( iPort>0 ) && ( iPort<=0xFFFF );
}


void CheckPort ( int iPort )
{
	if ( !IsPortInRange(iPort) )
		sphFatal ( "port %d is out of range", iPort );
}

struct ListenerDesc_t
{
	ProtocolType_e	m_eProto;
	CSphString		m_sUnix;
	DWORD			m_uIP;
	int				m_iPort;
	bool			m_bVIP;
};

void ProtoByName ( const CSphString & sProto, ListenerDesc_t & tDesc )
{
	if ( sProto=="sphinx" )			tDesc.m_eProto = PROTO_SPHINX;
	else if ( sProto=="mysql41" )	tDesc.m_eProto = PROTO_MYSQL41;
	else if ( sProto=="http" )		tDesc.m_eProto = PROTO_HTTP;
	else if ( sProto=="sphinx_vip" )
	{
		tDesc.m_eProto = PROTO_SPHINX;
		tDesc.m_bVIP = true;
	} else if ( sProto=="mysql41_vip" )
	{
		tDesc.m_eProto = PROTO_MYSQL41;
		tDesc.m_bVIP = true;
	} else
	{
		sphFatal ( "unknown listen protocol type '%s'", sProto.cstr() ? sProto.cstr() : "(NULL)" );
	}
}

ListenerDesc_t ParseListener ( const char * sSpec )
{
	ListenerDesc_t tRes;
	tRes.m_eProto = PROTO_SPHINX;
	tRes.m_sUnix = "";
	tRes.m_uIP = htonl ( INADDR_ANY );
	tRes.m_iPort = SPHINXAPI_PORT;
	tRes.m_bVIP = false;

	// split by colon
	int iParts = 0;
	CSphString sParts[3];

	const char * sPart = sSpec;
	for ( const char * p = sSpec; ; p++ )
		if ( *p=='\0' || *p==':' )
	{
		if ( iParts==3 )
			sphFatal ( "invalid listen format (too many fields)" );

		sParts[iParts++].SetBinary ( sPart, p-sPart );
		if ( !*p )
			break; // bail out on zero

		sPart = p+1;
	}
	assert ( iParts>=1 && iParts<=3 );

	// handle UNIX socket case
	// might be either name on itself (1 part), or name+protocol (2 parts)
	sPart = sParts[0].cstr();
	if ( sPart[0]=='/' )
	{
		if ( iParts>2 )
			sphFatal ( "invalid listen format (too many fields)" );

		if ( iParts==2 )
			ProtoByName ( sParts[1], tRes );

#if USE_WINDOWS
		sphFatal ( "UNIX sockets are not supported on Windows" );
#else
		tRes.m_sUnix = sPart;
		return tRes;
#endif
	}

	// check if it all starts with a valid port number
	sPart = sParts[0].cstr();
	int iLen = strlen(sPart);

	bool bAllDigits = true;
	for ( int i=0; i<iLen && bAllDigits; i++ )
		if ( !isdigit ( sPart[i] ) )
			bAllDigits = false;

	int iPort = 0;
	if ( bAllDigits && iLen<=5 )
	{
		iPort = atol(sPart);
		CheckPort ( iPort ); // lets forbid ambiguous magic like 0:sphinx or 99999:mysql41
	}

	// handle TCP port case
	// one part. might be either port name, or host name, or UNIX socket name
	if ( iParts==1 )
	{
		if ( iPort )
		{
			// port name on itself
			tRes.m_uIP = htonl ( INADDR_ANY );
			tRes.m_iPort = iPort;
		} else
		{
			// host name on itself
			tRes.m_uIP = sphGetAddress ( sSpec, GETADDR_STRICT );
			tRes.m_iPort = SPHINXAPI_PORT;
		}
		return tRes;
	}

	// two or three parts
	if ( iPort )
	{
		// 1st part is a valid port number; must be port:proto
		if ( iParts!=2 )
			sphFatal ( "invalid listen format (expected port:proto, got extra trailing part in listen=%s)", sSpec );

		tRes.m_uIP = htonl ( INADDR_ANY );
		tRes.m_iPort = iPort;
		ProtoByName ( sParts[1], tRes );

	} else
	{
		// 1st part must be a host name; must be host:port[:proto]
		if ( iParts==3 )
			ProtoByName ( sParts[2], tRes );

		tRes.m_iPort = atol ( sParts[1].cstr() );
		CheckPort ( tRes.m_iPort );

		tRes.m_uIP = sParts[0].IsEmpty()
			? htonl ( INADDR_ANY )
			: sphGetAddress ( sParts[0].cstr(), GETADDR_STRICT );
	}
	return tRes;
}


void AddListener ( const CSphString & sListen, bool bHttpAllowed )
{
	ListenerDesc_t tDesc = ParseListener ( sListen.cstr() );

	Listener_t tListener;
	tListener.m_eProto = tDesc.m_eProto;
	tListener.m_bTcp = true;
	tListener.m_bVIP = tDesc.m_bVIP;

	if ( tDesc.m_eProto==PROTO_HTTP && !bHttpAllowed )
	{
		sphWarning ( "thread_pool disabled, can not listen for http interface, port=%d, use workers=thread_pool", tDesc.m_iPort );
		return;
	}

#if !USE_WINDOWS
	if ( !tDesc.m_sUnix.IsEmpty() )
	{
		tListener.m_iSock = sphCreateUnixSocket ( tDesc.m_sUnix.cstr() );
		tListener.m_bTcp = false;
	} else
#endif
		tListener.m_iSock = sphCreateInetSocket ( tDesc.m_uIP, tDesc.m_iPort );

	g_dListeners.Add ( tListener );
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

class CloseOnDestroy : public ISphNoncopyable
{
	int m_id;
public:
	explicit CloseOnDestroy ( int id ) : m_id ( id ) {}
	~CloseOnDestroy() { close ( m_id ); }
};

/// wait until socket is readable or writable
int sphPoll ( int iSock, int64_t tmTimeout, bool bWrite=false )
{
#if HAVE_EPOLL
	int eid = epoll_create ( 1 );
	CloseOnDestroy dEid ( eid );
	epoll_event dEvent;
	dEvent.events = bWrite ? EPOLLOUT : EPOLLIN;
	epoll_ctl ( eid, EPOLL_CTL_ADD, iSock, &dEvent );
	// do poll
	return ::epoll_wait ( eid, &dEvent, 1, int ( tmTimeout/1000 ) );
#elif HAVE_POLL
	struct pollfd pfd;
	pfd.fd = iSock;
	pfd.events = bWrite ? POLLOUT : POLLIN;

	return ::poll ( &pfd, 1, int ( tmTimeout/1000 ) );
#else
	fd_set fdSet;
	FD_ZERO ( &fdSet );
	sphFDSet ( iSock, &fdSet );

	struct timeval tv;
	tv.tv_sec = (int)( tmTimeout / 1000000 );
	tv.tv_usec = (int)( tmTimeout % 1000000 );

	return ::select ( iSock+1, bWrite ? NULL : &fdSet, bWrite ? &fdSet : NULL, NULL, &tv );
#endif
}

/// check if a socket is still connected
bool sphSockEof ( int iSock )
{
	if ( iSock<0 )
		return true;

	char cBuf;
#if HAVE_EPOLL
	int eid = epoll_create ( 1 );
	CloseOnDestroy dEid ( eid );
	epoll_event dEvent;
	dEvent.events = EPOLLPRI | EPOLLIN;
	epoll_ctl ( eid, EPOLL_CTL_ADD, iSock, &dEvent );
	if ( ::epoll_wait ( eid, &dEvent, 1, 0 )<0 )
		return true;

	if ( dEvent.events & (EPOLLPRI|EPOLLIN) )
#elif HAVE_POLL
	struct pollfd pfd;
	pfd.fd = iSock;
	pfd.events = POLLPRI | POLLIN;
	if ( ::poll ( &pfd, 1, 0 )<0 )
		return true;

	if ( pfd.revents & (POLLIN|POLLPRI) )
#else
	fd_set fdrSet, fdeSet;
	FD_ZERO ( &fdrSet );
	FD_ZERO ( &fdeSet );
	sphFDSet ( iSock, &fdrSet );
	sphFDSet ( iSock, &fdeSet );
	struct timeval tv = {0};
	if ( ::select ( iSock+1, &fdrSet, NULL, &fdeSet, &tv )<0 )
		return true;

	if ( FD_ISSET ( iSock, &fdrSet ) || FD_ISSET ( iSock, &fdeSet ) )
#endif
		if ( ::recv ( iSock, &cBuf, sizeof(cBuf), MSG_PEEK )<=0 )
			if ( sphSockGetErrno()!=EWOULDBLOCK )
				return true;
	return false;
}


int sphSockRead ( int iSock, void * buf, int iLen, int iReadTimeout, bool bIntr )
{
	assert ( iLen>0 );

	int64_t tmMaxTimer = sphMicroTimer() + I64C(1000000)*Max ( 1, iReadTimeout ); // in microseconds
	int iLeftBytes = iLen; // bytes to read left

	char * pBuf = (char*) buf;
	int iRes = -1, iErr = 0;

	while ( iLeftBytes>0 )
	{
		int64_t tmMicroLeft = tmMaxTimer - sphMicroTimer();
		if ( tmMicroLeft<=0 )
			break; // timed out

#if USE_WINDOWS
		// Windows EINTR emulation
		// Ctrl-C will not interrupt select on Windows, so let's handle that manually
		// forcibly limit select() to 100 ms, and check flag afterwards
		if ( bIntr )
			tmMicroLeft = Min ( tmMicroLeft, 100000 );
#endif

		// wait until there is data
		iRes = sphPoll ( iSock, tmMicroLeft );

		// if there was EINTR, retry
		// if any other error, bail
		if ( iRes==-1 )
		{
			// only let SIGTERM (of all them) to interrupt, and only if explicitly allowed
			iErr = sphSockGetErrno();
			if ( iErr==EINTR && !( g_bGotSigterm && bIntr ))
				continue;

			if ( iErr==EINTR )
				sphLogDebug ( "sphSockRead: select got SIGTERM, exit -1" );

			sphSockSetErrno ( iErr );
			return -1;
		}

		// if there was a timeout, report it as an error
		if ( iRes==0 )
		{
#if USE_WINDOWS
			// Windows EINTR emulation
			if ( bIntr )
			{
				// got that SIGTERM
				if ( g_bGotSigterm )
				{
					sphLogDebug ( "sphSockRead: got SIGTERM emulation on Windows, exit -1" );
					sphSockSetErrno ( EINTR );
					return -1;
				}

				// timeout might not be fully over just yet, so re-loop
				continue;
			}
#endif

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
		// if any other error, bail
		if ( iRes==-1 )
		{
			// only let SIGTERM (of all them) to interrupt, and only if explicitly allowed
			iErr = sphSockGetErrno();
			if ( iErr==EINTR && !( g_bGotSigterm && bIntr ))
				continue;

			if ( iErr==EINTR )
				sphLogDebug ( "sphSockRead: select got SIGTERM, exit -1" );

			sphSockSetErrno ( iErr );
			return -1;
		}

		// update
		pBuf += iRes;
		iLeftBytes -= iRes;

		// avoid partial buffer loss in case of signal during the 2nd (!) read
		bIntr = false;
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

ISphOutputBuffer::ISphOutputBuffer ()
{
	m_dBuf.Reserve ( NETOUTBUF );
}

ISphOutputBuffer::ISphOutputBuffer ( CSphVector<BYTE> & dBuf )
{
	m_dBuf.SwapData ( dBuf );
}


void ISphOutputBuffer::SendString ( const char * sStr )
{
	int iLen = sStr ? strlen(sStr) : 0;
	SendInt ( iLen );
	SendBytes ( sStr, iLen );
}


int MysqlPackedLen ( int iLen )
{
	if ( iLen<251 )
		return 1;
	if ( iLen<=0xffff )
		return 3;
	if ( iLen<=0xffffff )
		return 4;
	return 9;
}


int MysqlPackedLen ( const char * sStr )
{
	int iLen = strlen(sStr);
	return MysqlPackedLen ( iLen ) + iLen;
}



// encodes Mysql Length-coded binary
void * MysqlPack ( void * pBuffer, int iValue )
{
	char * pOutput = (char*)pBuffer;
	if ( iValue<0 )
		return (void*)pOutput;

	if ( iValue<251 )
	{
		*pOutput++ = (char)iValue;
		return (void*)pOutput;
	}

	if ( iValue<=0xFFFF )
	{
		*pOutput++ = '\xFC';
		*pOutput++ = (char)iValue;
		*pOutput++ = (char)( iValue>>8 );
		return (void*)pOutput;
	}

	if ( iValue<=0xFFFFFF )
	{
		*pOutput++ = '\xFD';
		*pOutput++ = (char)iValue;
		*pOutput++ = (char)( iValue>>8 );
		*pOutput++ = (char)( iValue>>16 );
		return (void *) pOutput;
	}

	*pOutput++ = '\xFE';
	*pOutput++ = (char)iValue;
	*pOutput++ = (char)( iValue>>8 );
	*pOutput++ = (char)( iValue>>16 );
	*pOutput++ = (char)( iValue>>24 );
	*pOutput++ = 0;
	*pOutput++ = 0;
	*pOutput++ = 0;
	*pOutput++ = 0;
	return (void*)pOutput;
}

int MysqlUnpack ( InputBuffer_c & tReq, DWORD * pSize )
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


void ISphOutputBuffer::SendMysqlInt ( int iVal )
{
	BYTE dBuf[12];
	BYTE * pBuf = (BYTE*) MysqlPack ( dBuf, iVal );
	SendBytes ( dBuf, (int)( pBuf-dBuf ) );
}


void ISphOutputBuffer::SendMysqlString ( const char * sStr )
{
	int iLen = strlen(sStr);

	BYTE dBuf[12];
	BYTE * pBuf = (BYTE*) MysqlPack ( dBuf, iLen );
	SendBytes ( dBuf, (int)( pBuf-dBuf ) );
	SendBytes ( sStr, iLen );
}


void ISphOutputBuffer::SendBytes ( const void * pBuf, int iLen )
{
	int iOff = m_dBuf.GetLength();
	m_dBuf.Resize ( iOff + iLen );
	memcpy ( m_dBuf.Begin() + iOff, pBuf, iLen );
}


void ISphOutputBuffer::SendOutput ( const ISphOutputBuffer & tOut )
{
	int iLen = tOut.m_dBuf.GetLength();
	SendInt ( iLen );
	if ( iLen )
		SendBytes ( tOut.m_dBuf.Begin(), iLen );
}


NetOutputBuffer_c::NetOutputBuffer_c ( int iSock )
	: m_pProfile ( NULL )
	, m_iSock ( iSock )
	, m_iSent ( 0 )
	, m_bError ( false )
{
	assert ( m_iSock>0 );
}

void NetOutputBuffer_c::Flush ()
{
	if ( m_bError )
		return;

	int iLen = m_dBuf.GetLength();
	if ( !iLen )
		return;

	if ( g_bGotSigterm )
		sphLogDebug ( "SIGTERM in NetOutputBuffer::Flush" );

	const char * pBuffer = (const char *)m_dBuf.Begin();

	CSphScopedProfile tProf ( m_pProfile, SPH_QSTATE_NET_WRITE );

	const int64_t tmMaxTimer = sphMicroTimer() + MS2SEC * g_iWriteTimeout; // in microseconds
	while ( !m_bError )
	{
		int iRes = sphSockSend ( m_iSock, pBuffer, iLen );
		if ( iRes < 0 )
		{
			int iErrno = sphSockGetErrno();
			if ( iErrno==EINTR ) // interrupted before any data was sent; just loop
				continue;
			if ( iErrno!=EAGAIN && iErrno!=EWOULDBLOCK )
			{
				sphWarning ( "send() failed: %d: %s", iErrno, sphSockError(iErrno) );
				m_bError = true;
				break;
			}
		} else
		{
			m_iSent += iRes;
			pBuffer += iRes;
			iLen -= iRes;
			if ( iLen==0 )
				break;
		}

		// wait until we can write
		int64_t tmMicroLeft = tmMaxTimer - sphMicroTimer();
		if ( tmMicroLeft>0 )
			iRes = sphPoll ( m_iSock, tmMicroLeft, true );
		else
			iRes = 0; // time out

		switch ( iRes )
		{
			case 1: // ready for writing
				break;

			case 0: // timed out
			{
				sphWarning ( "timed out while trying to flush network buffers" );
				m_bError = true;
				break;
			}

			case -1: // error
			{
				int iErrno = sphSockGetErrno();
				if ( iErrno==EINTR )
					break;
				sphWarning ( "select() failed: %d: %s", iErrno, sphSockError(iErrno) );
				m_bError = true;
				break;
			}
		}
	}

	m_dBuf.Resize ( 0 );
}


/////////////////////////////////////////////////////////////////////////////

InputBuffer_c::InputBuffer_c ( const BYTE * pBuf, int iLen )
	: m_pBuf ( pBuf )
	, m_pCur ( pBuf )
	, m_bError ( !pBuf || iLen<0 )
	, m_iLen ( iLen )
{}


CSphString InputBuffer_c::GetString ()
{
	CSphString sRes;

	int iLen = GetInt ();
	if ( m_bError || iLen<0 || iLen>g_iMaxPacketSize || ( m_pCur+iLen > m_pBuf+m_iLen ) )
	{
		SetError ( true );
		return sRes;
	}

	sRes.SetBinary ( (char*)m_pCur, iLen );
	m_pCur += iLen;
	return sRes;
}


CSphString InputBuffer_c::GetRawString ( int iLen )
{
	CSphString sRes;

	if ( m_bError || iLen<0 || iLen>g_iMaxPacketSize || ( m_pCur+iLen > m_pBuf+m_iLen ) )
	{
		SetError ( true );
		return sRes;
	}

	sRes.SetBinary ( (char*)m_pCur, iLen );
	m_pCur += iLen;
	return sRes;
}


bool InputBuffer_c::GetString ( CSphVector<BYTE> & dBuffer )
{
	int iLen = GetInt ();
	if ( m_bError || iLen<0 || iLen>g_iMaxPacketSize || ( m_pCur+iLen > m_pBuf+m_iLen ) )
	{
		SetError ( true );
		return false;
	}

	if ( !iLen )
		return true;

	int iSize = dBuffer.GetLength();
	dBuffer.Resize ( iSize + iLen + 1 );
	dBuffer[iSize+iLen] = '\0';
	return GetBytes ( dBuffer.Begin()+iSize, iLen );
}


bool InputBuffer_c::GetBytes ( void * pBuf, int iLen )
{
	assert ( pBuf );
	assert ( iLen>0 && iLen<=g_iMaxPacketSize );

	if ( m_bError || ( m_pCur+iLen > m_pBuf+m_iLen ) )
	{
		SetError ( true );
		return false;
	}

	memcpy ( pBuf, m_pCur, iLen );
	m_pCur += iLen;
	return true;
}


template < typename T > bool InputBuffer_c::GetDwords ( CSphVector<T> & dBuffer, int & iGot, int iMax )
{
	iGot = GetInt ();
	if ( iGot<0 || iGot>iMax )
	{
		SetError ( true );
		return false;
	}

	dBuffer.Resize ( iGot );
	ARRAY_FOREACH ( i, dBuffer )
		dBuffer[i] = GetDword ();

	if ( m_bError )
		dBuffer.Reset ();

	return !m_bError;
}


template < typename T > bool InputBuffer_c::GetQwords ( CSphVector<T> & dBuffer, int & iGot, int iMax )
{
	iGot = GetInt ();
	if ( iGot<0 || iGot>iMax )
	{
		SetError ( true );
		return false;
	}

	dBuffer.Resize ( iGot );
	ARRAY_FOREACH ( i, dBuffer )
		dBuffer[i] = GetUint64 ();

	if ( m_bError )
		dBuffer.Reset ();

	return !m_bError;
}
/////////////////////////////////////////////////////////////////////////////

NetInputBuffer_c::NetInputBuffer_c ( int iSock )
	: InputBuffer_c ( m_dMinibufer, sizeof(m_dMinibufer) )
	, m_iSock ( iSock )
	, m_bIntr ( false )
	, m_iMaxibuffer ( 0 )
	, m_pMaxibuffer ( NULL )
{}


NetInputBuffer_c::~NetInputBuffer_c ()
{
	SafeDeleteArray ( m_pMaxibuffer );
}


bool NetInputBuffer_c::ReadFrom ( int iLen, int iTimeout, bool bIntr, bool bAppend )
{
	assert (!( bAppend && m_pCur!=m_pBuf && m_pBuf!=m_pMaxibuffer )); // only allow appends to untouched maxi-buffers
	int iCur = bAppend ? m_iLen : 0;

	m_bIntr = false;
	if ( iLen<=0 || iLen>g_iMaxPacketSize || m_iSock<0 )
		return false;

	BYTE * pBuf = m_dMinibufer + iCur;
	if ( ( iCur+iLen )>NET_MINIBUFFER_SIZE )
	{
		if ( ( iCur+iLen )>m_iMaxibuffer )
		{
			if ( iCur )
			{
				BYTE * pNew = new BYTE [ iCur+iLen ];
				memcpy ( pNew, m_pCur, iCur );
				SafeDeleteArray ( m_pMaxibuffer );
				m_pMaxibuffer = pNew;
				m_iMaxibuffer = iCur+iLen;
			} else
			{
				SafeDeleteArray ( m_pMaxibuffer );
				m_pMaxibuffer = new BYTE [ iLen ];
				m_iMaxibuffer = iLen;
			}
		}
		pBuf = m_pMaxibuffer;
	}

	m_pCur = m_pBuf = pBuf;
	int iGot = sphSockRead ( m_iSock, pBuf + iCur, iLen, iTimeout, bIntr );
	if ( g_bGotSigterm )
	{
		sphLogDebug ( "NetInputBuffer_c::ReadFrom: got SIGTERM, return false" );
		m_bError = true;
		m_bIntr = true;
		return false;
	}

	m_bError = ( iGot!=iLen );
	m_bIntr = m_bError && ( sphSockPeekErrno()==EINTR );
	m_iLen = m_bError ? 0 : iCur+iLen;
	return !m_bError;
}


void SendErrorReply ( ISphOutputBuffer & tOut, const char * sTemplate, ... )
{
	CSphString sError;
	va_list ap;
	va_start ( ap, sTemplate );
	sError.SetSprintfVa ( sTemplate, ap );
	va_end ( ap );

	tOut.SendWord ( SEARCHD_ERROR );
	tOut.SendWord ( 0 );
	tOut.SendInt ( sError.Length() + 4 );
	tOut.SendString ( sError.cstr() );
	// send!
	tOut.Flush();

	// --console logging
	if ( g_bOptNoDetach && g_eLogFormat!=LOG_FORMAT_SPHINXQL )
		sphInfo ( "query error: %s", sError.cstr() );
}

// fix MSVC 2005 fuckup
#if USE_WINDOWS
#pragma conform(forScope,on)
#endif

/////////////////////////////////////////////////////////////////////////////
// DISTRIBUTED QUERIES
/////////////////////////////////////////////////////////////////////////////

void InterWorkerStorage::Init ( int iBufSize )
{
	assert ( !m_dBuffer.GetLength() );
	m_dBuffer.Reset ( iBufSize );
}

bool InterWorkerStorage::Lock()
{
	return m_tThdMutex.Lock();
}

bool InterWorkerStorage::Unlock()
{
	return m_tThdMutex.Unlock();
}

/// distributed index
struct DistributedIndex_t
{
	CSphVector<MetaAgentDesc_t>		m_dAgents;					///< remote agents
	CSphVector<CSphString>		m_dLocal;					///< local indexes
	int							m_iAgentConnectTimeout;		///< in msec
	int							m_iAgentQueryTimeout;		///< in msec
	bool						m_bToDelete;				///< should be deleted
	bool						m_bDivideRemoteRanges;			///< whether we divide big range onto agents or not
	HAStrategies_e				m_eHaStrategy;				///< how to select the best of my agents
	InterWorkerStorage *		m_pHAStorage;				///< IPC HA arrays

public:
	DistributedIndex_t ()
		: m_iAgentConnectTimeout ( g_iAgentConnectTimeout )
		, m_iAgentQueryTimeout ( g_iAgentQueryTimeout )
		, m_bToDelete ( false )
		, m_bDivideRemoteRanges ( false )
		, m_eHaStrategy ( HA_DEFAULT )
		, m_pHAStorage ( NULL )
	{}
	~DistributedIndex_t()
	{
		// m_pHAStorage has to be freed separately.
	}
	void GetAllAgents ( CSphVector<AgentConn_t> * pTarget ) const;
	void ShareHACounters();
	void RemoveHACounters();
};

void DistributedIndex_t::GetAllAgents ( CSphVector<AgentConn_t> * pTarget ) const
{
	assert ( pTarget );
	ARRAY_FOREACH ( i, m_dAgents )
		ARRAY_FOREACH ( j, m_dAgents[i].GetAgents() )
	{
		AgentDesc_c & dAgent = pTarget->Add();
		dAgent = m_dAgents[i].GetAgents()[j];
	}
}

void DistributedIndex_t::ShareHACounters()
{
	int iSharedValues = 0;
	int iRRCounters = 0;
	ARRAY_FOREACH ( i, m_dAgents )
		if ( m_dAgents[i].IsHA() )
		{
			iSharedValues += m_dAgents[i].GetLength();
			++iRRCounters;
		}

		// nothing to share.
		if ( !iSharedValues )
			return;

		// so, we need to share between workers iFloatValues floats and iRRCounters ints.
		int iBufSize = iRRCounters * sizeof(int) + iSharedValues * sizeof(WORD) * 2; // NOLINT
		m_pHAStorage = new InterWorkerStorage;
		m_pHAStorage->Init ( iBufSize );

		// do the sharing.
		BYTE* pBuffer = m_pHAStorage->GetSharedData();
		ARRAY_FOREACH ( i, m_dAgents )
			if ( m_dAgents[i].IsHA() )
			{
				MetaAgentDesc_t & dAgent = m_dAgents[i];
				WORD* pWeights = (WORD*) ( pBuffer + sizeof(int) ); // NOLINT
				WORD dFrac = WORD ( 0xFFFF / dAgent.GetLength() );
				ARRAY_FOREACH ( j, dAgent ) ///< works since dAgent has method GetLength()
					pWeights[j] = dFrac;
				dAgent.SetHAData ( (int*)pBuffer, pWeights, m_pHAStorage );
				pBuffer += sizeof(int) + sizeof(float)*dAgent.GetLength(); // NOLINT
			}
}

void DistributedIndex_t::RemoveHACounters()
{
	ARRAY_FOREACH ( i, m_dAgents )
		if ( m_dAgents[i].IsHA() )
			m_dAgents[i].SetHAData ( NULL, NULL, NULL );
	SafeDelete ( m_pHAStorage );
}

/// global distributed index definitions hash
static SmallStringHash_T < DistributedIndex_t >		g_hDistIndexes;

/////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////////
// SEARCH HANDLER
/////////////////////////////////////////////////////////////////////////////

struct SearchRequestBuilder_t : public IRequestBuilder_t
{
	SearchRequestBuilder_t ( const CSphVector<CSphQuery> & dQueries, int iStart, int iEnd, int iDivideLimits )
		: m_dQueries ( dQueries ), m_iStart ( iStart ), m_iEnd ( iEnd ), m_iDivideLimits ( iDivideLimits )
	{}

	virtual void		BuildRequest ( AgentConn_t & tAgent, NetOutputBuffer_c & tOut ) const;

protected:
	int					CalcQueryLen ( const char * sIndexes, const CSphQuery & q, bool bAgentWeight ) const;
	void				SendQuery ( const char * sIndexes, NetOutputBuffer_c & tOut, const CSphQuery & q, bool bAgentWeight, int iWeight ) const;

protected:
	const CSphVector<CSphQuery> &		m_dQueries;
	const int							m_iStart;
	const int							m_iEnd;
	const int							m_iDivideLimits;
};


struct SearchReplyParser_t : public IReplyParser_t, public ISphNoncopyable
{
	SearchReplyParser_t ( int iStart, int iEnd, CSphVector<DWORD> & dMvaStorage, CSphVector<BYTE> & dStringsStorage )
		: m_iStart ( iStart )
		, m_iEnd ( iEnd )
		, m_dMvaStorage ( dMvaStorage )
		, m_dStringsStorage ( dStringsStorage )
	{}

	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const;

protected:
	int					m_iStart;
	int					m_iEnd;
	CSphVector<DWORD> &	m_dMvaStorage;
	CSphVector<BYTE> &	m_dStringsStorage;
};

/////////////////////////////////////////////////////////////////////////////

int SearchRequestBuilder_t::CalcQueryLen ( const char * sIndexes, const CSphQuery & q, bool bAgentWeight ) const
{
	int iReqSize = 136 + 2*sizeof(SphDocID_t) + 4*q.m_dWeights.GetLength()
		+ q.m_sSortBy.Length()
		+ strlen ( sIndexes )
		+ q.m_sGroupBy.Length()
		+ q.m_sGroupSortBy.Length()
		+ q.m_sGroupDistinct.Length()
		+ q.m_sComment.Length()
		+ q.m_sSelect.Length()
		+ q.m_sOuterOrderBy.Length();
	iReqSize += q.m_sRawQuery.IsEmpty()
		? q.m_sQuery.Length()
		: q.m_sRawQuery.Length();
	if ( q.m_eRanker==SPH_RANK_EXPR || q.m_eRanker==SPH_RANK_EXPORT )
		iReqSize += q.m_sRankerExpr.Length() + 4;
	ARRAY_FOREACH ( j, q.m_dFilters )
	{
		const CSphFilterSettings & tFilter = q.m_dFilters[j];
		iReqSize += 16 + tFilter.m_sAttrName.Length(); // string attr-name; int type; int exclude-flag; int equal-flag
		switch ( tFilter.m_eType )
		{
			case SPH_FILTER_VALUES:		iReqSize += 4 + 8*tFilter.GetNumValues (); break; // int values-count; uint64[] values
			case SPH_FILTER_RANGE:		iReqSize += 16; break; // uint64 min-val, max-val
			case SPH_FILTER_FLOATRANGE:	iReqSize += 8; break; // int/float min-val,max-val
			case SPH_FILTER_USERVAR:
			case SPH_FILTER_STRING:		iReqSize += 4 + tFilter.m_sRefString.Length(); break;
			case SPH_FILTER_NULL:		iReqSize += 1; break; // boolean value
		}
	}
	if ( q.m_bGeoAnchor )
		iReqSize += 16 + q.m_sGeoLatAttr.Length() + q.m_sGeoLongAttr.Length(); // string lat-attr, long-attr; float lat, long
	if ( bAgentWeight )
	{
		iReqSize += 9; // "*" (length=1) + length itself + weight
	} else
	{
		ARRAY_FOREACH ( i, q.m_dIndexWeights )
			iReqSize += 8 + q.m_dIndexWeights[i].m_sName.Length(); // string index-name; int index-weight
	}
	ARRAY_FOREACH ( i, q.m_dFieldWeights )
		iReqSize += 8 + q.m_dFieldWeights[i].m_sName.Length(); // string field-name; int field-weight
	ARRAY_FOREACH ( i, q.m_dOverrides )
		iReqSize += 12 + q.m_dOverrides[i].m_sAttr.Length() + // string attr-name; int type; int values-count
			( q.m_dOverrides[i].m_eAttrType==SPH_ATTR_BIGINT ? 16 : 12 )*q.m_dOverrides[i].m_dValues.GetLength(); // ( bigint id; int/float/bigint value )[] values
	if ( q.m_bHasOuter )
		iReqSize += 4; // outer limit
	if ( q.m_iMaxPredictedMsec>0 )
		iReqSize += 4;
	return iReqSize;
}


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
	QFLAG_LOW_PRIORITY			= 1UL << 8
};

void SearchRequestBuilder_t::SendQuery ( const char * sIndexes, NetOutputBuffer_c & tOut, const CSphQuery & q, bool bAgentWeight, int iWeight ) const
{
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
	if ( q.m_sRawQuery.IsEmpty() )
		tOut.SendString ( q.m_sQuery.cstr() );
	else
		tOut.SendString ( q.m_sRawQuery.cstr() ); // query
	tOut.SendInt ( q.m_dWeights.GetLength() );
	ARRAY_FOREACH ( j, q.m_dWeights )
		tOut.SendInt ( q.m_dWeights[j] ); // weights
	tOut.SendString ( sIndexes ); // indexes
	tOut.SendInt ( USE_64BIT ); // id range bits
	tOut.SendDocid ( 0 ); // default full id range (any client range must be in filters at this stage)
	tOut.SendDocid ( DOCID_MAX );
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
				tOut.SendString ( tFilter.m_sRefString.cstr() );
				break;

			case SPH_FILTER_NULL:
				tOut.SendByte ( tFilter.m_bHasEqual );
				break;
		}
		tOut.SendInt ( tFilter.m_bExclude );
		tOut.SendInt ( tFilter.m_bHasEqual );
	}
	tOut.SendInt ( q.m_eGroupFunc );
	tOut.SendString ( q.m_sGroupBy.cstr() );
	if ( m_iDivideLimits==1 )
		tOut.SendInt ( q.m_iMaxMatches );
	else
		tOut.SendInt ( 1+(q.m_iMaxMatches/m_iDivideLimits) ); // Reduce the max_matches also.
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
	if ( bAgentWeight )
	{
		tOut.SendInt ( 1 );
		tOut.SendString ( "*" );
		tOut.SendInt ( iWeight );
	} else
	{
		tOut.SendInt ( q.m_dIndexWeights.GetLength() );
		ARRAY_FOREACH ( i, q.m_dIndexWeights )
		{
			tOut.SendString ( q.m_dIndexWeights[i].m_sName.cstr() );
			tOut.SendInt ( q.m_dIndexWeights[i].m_iValue );
		}
	}
	tOut.SendDword ( q.m_uMaxQueryMsec );
	tOut.SendInt ( q.m_dFieldWeights.GetLength() );
	ARRAY_FOREACH ( i, q.m_dFieldWeights )
	{
		tOut.SendString ( q.m_dFieldWeights[i].m_sName.cstr() );
		tOut.SendInt ( q.m_dFieldWeights[i].m_iValue );
	}
	tOut.SendString ( q.m_sComment.cstr() );
	tOut.SendInt ( q.m_dOverrides.GetLength() );
	ARRAY_FOREACH ( i, q.m_dOverrides )
	{
		const CSphAttrOverride & tEntry = q.m_dOverrides[i];
		tOut.SendString ( tEntry.m_sAttr.cstr() );
		tOut.SendDword ( tEntry.m_eAttrType );
		tOut.SendInt ( tEntry.m_dValues.GetLength() );
		ARRAY_FOREACH ( j, tEntry.m_dValues )
		{
			tOut.SendUint64 ( tEntry.m_dValues[j].m_uDocID );
			switch ( tEntry.m_eAttrType )
			{
				case SPH_ATTR_FLOAT:	tOut.SendFloat ( tEntry.m_dValues[j].m_fValue ); break;
				case SPH_ATTR_BIGINT:	tOut.SendUint64 ( tEntry.m_dValues[j].m_uValue ); break;
				default:				tOut.SendDword ( (DWORD)tEntry.m_dValues[j].m_uValue ); break;
			}
		}
	}
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
}


void SearchRequestBuilder_t::BuildRequest ( AgentConn_t & tAgent, NetOutputBuffer_c & tOut ) const
{
	const char* sIndexes = tAgent.m_sIndexes.cstr();
	bool bAgentWeigth = ( tAgent.m_iWeight!=-1 );
	int iReqLen = 8; // int num-queries
	for ( int i=m_iStart; i<=m_iEnd; i++ )
		iReqLen += CalcQueryLen ( sIndexes, m_dQueries[i], bAgentWeigth );

	tOut.SendWord ( SEARCHD_COMMAND_SEARCH ); // command id
	tOut.SendWord ( VER_COMMAND_SEARCH ); // command version
	tOut.SendInt ( iReqLen ); // request body length

	tOut.SendInt ( VER_MASTER );
	tOut.SendInt ( m_iEnd-m_iStart+1 );
	for ( int i=m_iStart; i<=m_iEnd; i++ )
		SendQuery ( sIndexes, tOut, m_dQueries[i], bAgentWeigth, tAgent.m_iWeight );
}

/////////////////////////////////////////////////////////////////////////////

bool SearchReplyParser_t::ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const
{
	int iResults = m_iEnd-m_iStart+1;
	assert ( iResults>0 );

	tAgent.m_dResults.Resize ( iResults );
	for ( int iRes=0; iRes<iResults; iRes++ )
		tAgent.m_dResults[iRes].m_iSuccesses = 0;

	for ( int iRes=0; iRes<iResults; iRes++ )
	{
		CSphQueryResult & tRes = tAgent.m_dResults [ iRes ];
		tRes.m_sError = "";
		tRes.m_sWarning = "";

		// get status and message
		DWORD eStatus = tReq.GetDword ();
		if ( eStatus!=SEARCHD_OK )
		{
			CSphString sMessage = tReq.GetString ();
			switch ( eStatus )
			{
				case SEARCHD_ERROR:		tRes.m_sError = sMessage; continue;
				case SEARCHD_RETRY:		tRes.m_sError = sMessage; break;
				case SEARCHD_WARNING:	tRes.m_sWarning = sMessage; break;
				default:				tAgent.m_sFailure.SetSprintf ( "internal error: unknown status %d", eStatus ); break;
			}
		}

		// get schema
		CSphRsetSchema & tSchema = tRes.m_tSchema;
		tSchema.Reset ();

		tSchema.m_dFields.Resize ( tReq.GetInt() ); // FIXME! add a sanity check
		ARRAY_FOREACH ( j, tSchema.m_dFields )
			tSchema.m_dFields[j].m_sName = tReq.GetString ();

		int iNumAttrs = tReq.GetInt(); // FIXME! add a sanity check
		for ( int j=0; j<iNumAttrs; j++ )
		{
			CSphColumnInfo tCol;
			tCol.m_sName = tReq.GetString ();
			tCol.m_eAttrType = (ESphAttr) tReq.GetDword (); // FIXME! add a sanity check
			tSchema.AddDynamicAttr ( tCol ); // all attributes received from agents are dynamic
		}

		// get matches
		int iMatches = tReq.GetInt ();
		if ( iMatches<0 )
		{
			tAgent.m_sFailure.SetSprintf ( "invalid match count received (count=%d)", iMatches );
			return false;
		}

		int bAgent64 = tReq.GetInt ();
#if !USE_64BIT
		if ( bAgent64 )
			tAgent.m_sFailure.SetSprintf ( "id64 agent, id32 master, docids might be wrapped" );
#endif

		assert ( !tRes.m_dMatches.GetLength() );
		if ( iMatches )
		{
			tRes.m_dMatches.Resize ( iMatches );
			ARRAY_FOREACH ( i, tRes.m_dMatches )
			{
				CSphMatch & tMatch = tRes.m_dMatches[i];
				tMatch.Reset ( tSchema.GetRowSize() );
				tMatch.m_uDocID = bAgent64 ? (SphDocID_t)tReq.GetUint64() : tReq.GetDword();
				tMatch.m_iWeight = tReq.GetInt ();
				for ( int j=0; j<tSchema.GetAttrsCount(); j++ )
				{
					const CSphColumnInfo & tAttr = tSchema.GetAttr(j);
					if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET )
					{
						tMatch.SetAttr ( tAttr.m_tLocator, m_dMvaStorage.GetLength() );

						int iValues = tReq.GetDword ();
						m_dMvaStorage.Add ( iValues );
						if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET )
						{
							while ( iValues-- )
								m_dMvaStorage.Add ( tReq.GetDword() );
						} else
						{
							assert ( ( iValues%2 )==0 );
							for ( ; iValues; iValues -= 2 )
							{
								uint64_t uMva = tReq.GetUint64();
								m_dMvaStorage.Add ( (DWORD)uMva );
								m_dMvaStorage.Add ( (DWORD)( uMva>>32 ) );
							}
						}

					} else if ( tAttr.m_eAttrType==SPH_ATTR_FLOAT )
					{
						float fRes = tReq.GetFloat();
						tMatch.SetAttr ( tAttr.m_tLocator, sphF2DW(fRes) );

					} else if ( tAttr.m_eAttrType==SPH_ATTR_BIGINT )
					{
						tMatch.SetAttr ( tAttr.m_tLocator, tReq.GetUint64() );

					} else if ( tAttr.m_eAttrType==SPH_ATTR_STRING || tAttr.m_eAttrType==SPH_ATTR_JSON )
					{
						int iLen = tReq.GetDword();
						if ( !iLen )
						{
							tMatch.SetAttr ( tAttr.m_tLocator, 0 );
						} else
						{
							int iOff = m_dStringsStorage.GetLength();
							tMatch.SetAttr ( tAttr.m_tLocator, iOff );

							m_dStringsStorage.Resize ( iOff+4+iLen );
							int iPackedLen = sphPackStrlen ( m_dStringsStorage.Begin() + iOff, iLen );
							tReq.GetBytes ( m_dStringsStorage.Begin() + iOff + iPackedLen, iLen );
							m_dStringsStorage.Resize ( iOff+iPackedLen+iLen );
						}

					} else if ( tAttr.m_eAttrType==SPH_ATTR_STRINGPTR )
					{
						CSphString sValue = tReq.GetString();
						tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t) sValue.Leak() );
					} else if ( tAttr.m_eAttrType==SPH_ATTR_FACTORS || tAttr.m_eAttrType==SPH_ATTR_FACTORS_JSON )
					{
						DWORD uLength = tReq.GetDword();
						BYTE * pData = new BYTE[uLength];
						*(DWORD *)pData = uLength;
						tReq.GetBytes ( pData+sizeof(DWORD), uLength-sizeof(DWORD) );
						tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t) pData );

					} else if ( tAttr.m_eAttrType==SPH_ATTR_JSON_FIELD )
					{
						ESphJsonType eJson = (ESphJsonType)tReq.GetByte();
						if ( eJson==JSON_EOF )
						{
							tMatch.SetAttr ( tAttr.m_tLocator, 0 );
						} else
						{
							int iOff = m_dStringsStorage.GetLength();
							int64_t iTypeOffset = ( ( (int64_t)iOff ) | ( ( (int64_t)eJson )<<32 ) );
							tMatch.SetAttr ( tAttr.m_tLocator, iTypeOffset );

							// read node length if needed
							int iLen = sphJsonNodeSize ( eJson, NULL );
							if ( iLen<0 )
								iLen = tReq.GetDword ();

							m_dStringsStorage.Resize ( iOff+iLen );
							tReq.GetBytes ( m_dStringsStorage.Begin()+iOff, iLen );
						}

					} else
					{
						tMatch.SetAttr ( tAttr.m_tLocator, tReq.GetDword() );
					}
				}
			}
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
bool MinimizeSchema ( CSphRsetSchema & tDst, const ISphSchema & tSrc )
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
		CSphVector<CSphColumnInfo> dFields;
		Swap ( dFields, tDst.m_dFields );
		tDst.Reset();
		ARRAY_FOREACH ( i, dDst )
			tDst.AddDynamicAttr ( dDst[i] );
		Swap ( dFields, tDst.m_dFields );

	} else
	{
		tDst.SwapAttrs ( dDst );
	}

	return bEqual;
}

static void ParseIndexList ( const CSphString & sIndexes, CSphVector<CSphString> & dOut )
{
	CSphString sSplit = sIndexes;
	char * p = (char*)sSplit.cstr();
	while ( *p )
	{
		// skip non-alphas
		while ( *p && !isalpha ( *p ) && !isdigit ( *p ) && *p!='_' ) p++;
		if ( !(*p) ) break;

		// FIXME?
		// We no not check that index name shouldn't start with '_'.
		// That means it's de facto allowed for API queries.
		// But not for SphinxQL ones.

		// this is my next index name
		const char * sNext = p;
		while ( isalpha ( *p ) || isdigit ( *p ) || *p=='_' ) p++;

		assert ( sNext!=p );
		if ( *p ) *p++ = '\0'; // if it was not the end yet, we'll continue from next char

		dOut.Add ( sNext );
	}
}


static void CheckQuery ( const CSphQuery & tQuery, CSphString & sError )
{
	#define LOC_ERROR(_msg) { sError.SetSprintf ( _msg ); return; }
	#define LOC_ERROR1(_msg,_arg1) { sError.SetSprintf ( _msg, _arg1 ); return; }
	#define LOC_ERROR2(_msg,_arg1,_arg2) { sError.SetSprintf ( _msg, _arg1, _arg2 ); return; }

	sError = NULL;

	if ( tQuery.m_eMode<0 || tQuery.m_eMode>SPH_MATCH_TOTAL )
		LOC_ERROR1 ( "invalid match mode %d", tQuery.m_eMode );

	if ( tQuery.m_eRanker<0 || tQuery.m_eRanker>SPH_RANK_TOTAL )
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

	if ( ( tQuery.m_iRetryCount!=g_iAgentRetryCount )
		&& ( tQuery.m_iRetryCount<0 || tQuery.m_iRetryCount>MAX_RETRY_COUNT ) )
			LOC_ERROR1 ( "retry count out of bounds (count=%d)", tQuery.m_iRetryCount );

	if ( ( tQuery.m_iRetryCount!=g_iAgentRetryDelay )
		&& ( tQuery.m_iRetryDelay<0 || tQuery.m_iRetryDelay>MAX_RETRY_DELAY ) )
			LOC_ERROR1 ( "retry delay out of bounds (delay=%d)", tQuery.m_iRetryDelay );

	if ( tQuery.m_iOffset>0 && tQuery.m_bHasOuter )
		LOC_ERROR1 ( "inner offset must be 0 when using outer order by (offset=%d)", tQuery.m_iOffset );

	#undef LOC_ERROR
	#undef LOC_ERROR1
	#undef LOC_ERROR2
}


void PrepareQueryEmulation ( CSphQuery * pQuery )
{
	assert ( pQuery && pQuery->m_sRawQuery.cstr() );

	// sort filters
	ARRAY_FOREACH ( i, pQuery->m_dFilters )
		pQuery->m_dFilters[i].m_dValues.Sort();

	// sort overrides
	ARRAY_FOREACH ( i, pQuery->m_dOverrides )
		pQuery->m_dOverrides[i].m_dValues.Sort ();

	// fixup query
	pQuery->m_sQuery = pQuery->m_sRawQuery;

	if ( pQuery->m_eMode==SPH_MATCH_BOOLEAN )
		pQuery->m_eRanker = SPH_RANK_NONE;

	if ( pQuery->m_eMode==SPH_MATCH_FULLSCAN )
		pQuery->m_sQuery = "";

	if ( pQuery->m_eMode!=SPH_MATCH_ALL && pQuery->m_eMode!=SPH_MATCH_ANY && pQuery->m_eMode!=SPH_MATCH_PHRASE )
		return;

	const char * szQuery = pQuery->m_sRawQuery.cstr ();
	int iQueryLen = strlen(szQuery);

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

	if ( !pQuery->m_bHasOuter )
	{
		pQuery->m_sOuterOrderBy = "";
		pQuery->m_iOuterOffset = 0;
		pQuery->m_iOuterLimit = 0;
	}
}


bool ParseSearchQuery ( InputBuffer_c & tReq, ISphOutputBuffer & tOut, CSphQuery & tQuery, int iVer, int iMasterVer )
{
	// daemon-level defaults
	tQuery.m_iRetryCount = g_iAgentRetryCount;
	tQuery.m_iRetryDelay = g_iAgentRetryDelay;
	tQuery.m_iAgentQueryTimeout = g_iAgentQueryTimeout;

	// v.1.27+ flags come first
	DWORD uFlags = 0;
	if ( iVer>=0x11B )
		uFlags = tReq.GetDword();

	// v.1.0. mode, limits, weights, ID/TS ranges
	tQuery.m_iOffset = tReq.GetInt ();
	tQuery.m_iLimit = tReq.GetInt ();
	tQuery.m_eMode = (ESphMatchMode) tReq.GetInt ();
	if ( iVer>=0x110 )
	{
		tQuery.m_eRanker = (ESphRankMode) tReq.GetInt ();
		if ( tQuery.m_eRanker==SPH_RANK_EXPR || tQuery.m_eRanker==SPH_RANK_EXPORT )
			tQuery.m_sRankerExpr = tReq.GetString();
	}
	tQuery.m_eSort = (ESphSortOrder) tReq.GetInt ();
	if ( iVer>=0x102 )
	{
		tQuery.m_sSortBy = tReq.GetString ();
		sphColumnToLowercase ( const_cast<char *>( tQuery.m_sSortBy.cstr() ) );
	}
	tQuery.m_sRawQuery = tReq.GetString ();
	int iGot = 0;
	if ( !tReq.GetDwords ( tQuery.m_dWeights, iGot, SPH_MAX_FIELDS ) )
	{
		SendErrorReply ( tOut, "invalid weight count %d (should be in 0..%d range)", iGot, SPH_MAX_FIELDS );
		return false;
	}
	tQuery.m_sIndexes = tReq.GetString ();

	bool bIdrange64 = false;
	if ( iVer>=0x108 )
		bIdrange64 = ( tReq.GetInt()!=0 );

	SphDocID_t uMinID = 0;
	SphDocID_t uMaxID = DOCID_MAX;
	if ( bIdrange64 )
	{
		uMinID = (SphDocID_t)tReq.GetUint64 ();
		uMaxID = (SphDocID_t)tReq.GetUint64 ();
		// FIXME? could report clamp here if I'm id32 and client passed id64 range,
		// but frequently this won't affect anything at all
	} else
	{
		uMinID = tReq.GetDword ();
		uMaxID = tReq.GetDword ();
	}

	if ( iVer<0x108 && uMaxID==0xffffffffUL )
		uMaxID = 0; // fixup older clients which send 32-bit UINT_MAX by default

	if ( uMaxID==0 )
		uMaxID = DOCID_MAX;

	// v.1.2
	if ( iVer>=0x102 )
	{
		int iAttrFilters = tReq.GetInt ();
		if ( iAttrFilters>g_iMaxFilters )
		{
			SendErrorReply ( tOut, "too much attribute filters (req=%d, max=%d)", iAttrFilters, g_iMaxFilters );
			return false;
		}

		tQuery.m_dFilters.Resize ( iAttrFilters );
		ARRAY_FOREACH ( iFilter, tQuery.m_dFilters )
		{
			CSphFilterSettings & tFilter = tQuery.m_dFilters[iFilter];
			tFilter.m_sAttrName = tReq.GetString ();
			sphColumnToLowercase ( const_cast<char *>( tFilter.m_sAttrName.cstr() ) );

			if ( iVer>=0x10E )
			{
				// v.1.14+
				tFilter.m_eType = (ESphFilter) tReq.GetDword ();
				switch ( tFilter.m_eType )
				{
					case SPH_FILTER_RANGE:
						tFilter.m_iMinValue = ( iVer>=0x114 ) ? tReq.GetUint64() : tReq.GetDword ();
						tFilter.m_iMaxValue = ( iVer>=0x114 ) ? tReq.GetUint64() : tReq.GetDword ();
						break;

					case SPH_FILTER_FLOATRANGE:
						tFilter.m_fMinValue = tReq.GetFloat ();
						tFilter.m_fMaxValue = tReq.GetFloat ();
						break;

					case SPH_FILTER_VALUES:
						{
							int iGot = 0;
							bool bRes = ( iVer>=0x114 )
								? tReq.GetQwords ( tFilter.m_dValues, iGot, g_iMaxFilterValues )
								: tReq.GetDwords ( tFilter.m_dValues, iGot, g_iMaxFilterValues );
							if ( !bRes )
							{
								SendErrorReply ( tOut, "invalid attribute '%s'(%d) set length %d (should be in 0..%d range)", tFilter.m_sAttrName.cstr(), iFilter, iGot, g_iMaxFilterValues );
								return false;
							}
						}
						break;
					case SPH_FILTER_STRING:
						tFilter.m_sRefString = tReq.GetString();
						break;

					case SPH_FILTER_NULL:
						tFilter.m_bHasEqual = tReq.GetByte()!=0;
						break;

					case SPH_FILTER_USERVAR:
						tFilter.m_sRefString = tReq.GetString();
						break;

					default:
						SendErrorReply ( tOut, "unknown filter type (type-id=%d)", tFilter.m_eType );
						return false;
				}

			} else
			{
				// pre-1.14
				int iGot = 0;
				if ( !tReq.GetDwords ( tFilter.m_dValues, iGot, g_iMaxFilterValues ) )
				{
					SendErrorReply ( tOut, "invalid attribute '%s'(%d) set length %d (should be in 0..%d range)", tFilter.m_sAttrName.cstr(), iFilter, iGot, g_iMaxFilterValues );
					return false;
				}

				if ( !tFilter.m_dValues.GetLength() )
				{
					// 0 length means this is range, not set
					tFilter.m_iMinValue = tReq.GetDword ();
					tFilter.m_iMaxValue = tReq.GetDword ();
				}

				tFilter.m_eType = tFilter.m_dValues.GetLength() ? SPH_FILTER_VALUES : SPH_FILTER_RANGE;
			}

			if ( iVer>=0x106 )
				tFilter.m_bExclude = !!tReq.GetDword ();

			if ( iMasterVer>=5 )
				tFilter.m_bHasEqual = !!tReq.GetDword();
		}
	}

	// now add id range filter
	if ( uMinID!=0 || uMaxID!=DOCID_MAX )
	{
		CSphFilterSettings & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_sAttrName = "@id";
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_iMinValue = uMinID;
		tFilter.m_iMaxValue = uMaxID;
	}

	// v.1.3
	if ( iVer>=0x103 )
	{
		tQuery.m_eGroupFunc = (ESphGroupBy) tReq.GetDword ();
		tQuery.m_sGroupBy = tReq.GetString ();
		tQuery.m_sGroupBy.ToLower ();
	}

	// v.1.4
	tQuery.m_iMaxMatches = DEFAULT_MAX_MATCHES;
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
				SendErrorReply ( tOut, "INTERNAL ERROR: unsupported sort mode %d in groupby sort fixup", tQuery.m_eSort );
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
	{
		tQuery.m_sGroupDistinct = tReq.GetString ();
		tQuery.m_sGroupDistinct.ToLower();
	}

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

	// v.1.17
	if ( iVer>=0x111 )
		tQuery.m_uMaxQueryMsec = tReq.GetDword ();

	// v.1.18
	if ( iVer>=0x112 )
	{
		tQuery.m_dFieldWeights.Resize ( tReq.GetInt() ); // FIXME! add sanity check
		ARRAY_FOREACH ( i, tQuery.m_dFieldWeights )
		{
			tQuery.m_dFieldWeights[i].m_sName = tReq.GetString ();
			tQuery.m_dFieldWeights[i].m_iValue = tReq.GetInt ();
		}
	}

	// v.1.19
	if ( iVer>=0x113 )
		tQuery.m_sComment = tReq.GetString ();

	// v.1.21
	if ( iVer>=0x115 )
	{
		tQuery.m_dOverrides.Resize ( tReq.GetInt() ); // FIXME! add sanity check
		ARRAY_FOREACH ( i, tQuery.m_dOverrides )
		{
			CSphAttrOverride & tOverride = tQuery.m_dOverrides[i];
			tOverride.m_sAttr = tReq.GetString ();
			tOverride.m_eAttrType = (ESphAttr) tReq.GetDword ();

			tOverride.m_dValues.Resize ( tReq.GetInt() ); // FIXME! add sanity check
			ARRAY_FOREACH ( iVal, tOverride.m_dValues )
			{
				CSphAttrOverride::IdValuePair_t & tEntry = tOverride.m_dValues[iVal];
				tEntry.m_uDocID = (SphDocID_t) tReq.GetUint64 ();

				if ( tOverride.m_eAttrType==SPH_ATTR_FLOAT )		tEntry.m_fValue = tReq.GetFloat ();
				else if ( tOverride.m_eAttrType==SPH_ATTR_BIGINT )	tEntry.m_uValue = tReq.GetUint64 ();
				else												tEntry.m_uValue = tReq.GetDword ();
			}
		}
	}

	// v.1.22
	if ( iVer>=0x116 )
	{
		tQuery.m_sSelect = tReq.GetString ();
		tQuery.m_bAgent = ( iMasterVer>0 );
		if ( tQuery.m_sSelect.Begins ( "*,*" ) ) // this is the legacy mark of agent for debug purpose
		{
			tQuery.m_bAgent = true;
			int iSelectLen = tQuery.m_sSelect.Length();
			tQuery.m_sSelect = ( iSelectLen>4 ? tQuery.m_sSelect.SubString ( 4, iSelectLen-4 ) : "*" );
		}
		// fixup select list
		if ( tQuery.m_sSelect.IsEmpty () )
			tQuery.m_sSelect = "*";

		CSphString sError;
		if ( !tQuery.ParseSelectList ( sError ) )
		{
			SendErrorReply ( tOut, "select: %s", sError.cstr() );

			// we want to see a parse error in query_log_format=sphinxql mode too
			if ( g_eLogFormat==LOG_FORMAT_SPHINXQL && g_iQueryLogFile>=0 )
			{
				CSphStringBuilder tBuf;
				char sTimeBuf [ SPH_TIME_PID_MAX_SIZE ];
				sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

				tBuf += "/""* ";
				tBuf += sTimeBuf;
				tBuf.Appendf ( "*""/ %s # error=%s\n", tQuery.m_sSelect.cstr(), sError.cstr() );

				sphSeek ( g_iQueryLogFile, 0, SEEK_END );
				sphWrite ( g_iQueryLogFile, tBuf.cstr(), tBuf.Length() );
			}

			return false;
		}
	}

	// v.1.27
	if ( iVer>=0x11B )
	{
		// parse simple flags
		tQuery.m_bReverseScan = !!( uFlags & QFLAG_REVERSE_SCAN );
		tQuery.m_bSortKbuffer = !!( uFlags & QFLAG_SORT_KBUFFER );
		tQuery.m_bSimplify = !!( uFlags & QFLAG_SIMPLIFY );
		tQuery.m_bPlainIDF = !!( uFlags & QFLAG_PLAIN_IDF );
		tQuery.m_bGlobalIDF = !!( uFlags & QFLAG_GLOBAL_IDF );
		tQuery.m_bLocalDF = !!( uFlags & QFLAG_LOCAL_DF );
		tQuery.m_bLowPriority = !!( uFlags & QFLAG_LOW_PRIORITY );

		if ( iMasterVer>0 || iVer==0x11E )
			tQuery.m_bNormalizedTFIDF = !!( uFlags & QFLAG_NORMALIZED_TF );

		// fetch optional stuff
		if ( uFlags & QFLAG_MAX_PREDICTED_TIME )
			tQuery.m_iMaxPredictedMsec = tReq.GetInt();
	}

	// v.1.29
	if ( iVer>=0x11D )
	{
		tQuery.m_sOuterOrderBy = tReq.GetString();
		tQuery.m_iOuterOffset = tReq.GetDword();
		tQuery.m_iOuterLimit = tReq.GetDword();
		tQuery.m_bHasOuter = ( tReq.GetInt()!=0 );
	}

	// extension v.1
	tQuery.m_eCollation = g_eCollation;
	if ( iMasterVer>=1 )
		tQuery.m_eCollation = (ESphCollation)tReq.GetDword();

	// extension v.2
	if ( iMasterVer>=2 )
	{
		tQuery.m_sOuterOrderBy = tReq.GetString();
		if ( tQuery.m_bHasOuter )
			tQuery.m_iOuterLimit = tReq.GetInt();
	}

	if ( iMasterVer>=6 )
	{
		tQuery.m_iGroupbyLimit = tReq.GetInt();
	}

	/////////////////////
	// additional checks
	/////////////////////

	if ( tReq.GetError() )
	{
		SendErrorReply ( tOut, "invalid or truncated request" );
		return false;
	}

	CSphString sError;
	CheckQuery ( tQuery, sError );
	if ( !sError.IsEmpty() )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return false;
	}

	// now prepare it for the engine
	PrepareQueryEmulation ( &tQuery );

	// all ok
	return true;
}

//////////////////////////////////////////////////////////////////////////

void LogQueryPlain ( const CSphQuery & tQuery, const CSphQueryResult & tRes )
{
	assert ( g_eLogFormat==LOG_FORMAT_PLAIN );
	if ( ( !g_bQuerySyslog && g_iQueryLogFile<0 ) || !tRes.m_sError.IsEmpty() )
		return;

	CSphStringBuilder tBuf;

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
	tBuf.Appendf ( " [%s/%d/%s "INT64_FMT" (%d,%d)",
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
		tBuf.AppendEscaped ( sQuery.cstr(), false, true );
	}

#if USE_SYSLOG
	if ( !g_bQuerySyslog )
	{
#endif

	// line feed
	tBuf += "\n";

	sphSeek ( g_iQueryLogFile, 0, SEEK_END );
	sphWrite ( g_iQueryLogFile, tBuf.cstr(), tBuf.Length() );

#if USE_SYSLOG
	} else
	{
		syslog ( LOG_INFO, "%s", tBuf.cstr() );
	}
#endif
}

class UnBackquote_fn : public ISphNoncopyable
{
	CSphString m_sBuf;
	const char * m_pDst;

public:
	explicit UnBackquote_fn ( const char * pSrc )
	{
		m_pDst = pSrc;
		int iLen = 0;
		if ( pSrc && *pSrc )
			iLen = strlen ( pSrc );

		if ( iLen && memchr ( pSrc, '`', iLen ) )
		{
			m_sBuf = pSrc;
			char * pDst = const_cast<char *>( m_sBuf.cstr() );
			const char * pEnd = pSrc + iLen;

			while ( pSrc<pEnd )
			{
				*pDst = *pSrc++;
				if ( *pDst!='`' )
					pDst++;
			}
			*pDst = '\0';
			m_pDst = m_sBuf.cstr();
		}
	}

	const char * cstr() { return m_pDst; }
};

static void FormatOrderBy ( CSphStringBuilder * pBuf, const char * sPrefix, ESphSortOrder eSort, const CSphString & sSort )
{
	assert ( pBuf );
	if ( eSort==SPH_SORT_EXTENDED && sSort=="@weight desc" )
		return;

	const char * sSubst = "@weight";
	if ( sSort!="@relevance" )
			sSubst = sSort.cstr();

	UnBackquote_fn tUnquoted ( sSubst );
	sSubst = tUnquoted.cstr();

	switch ( eSort )
	{
	case SPH_SORT_ATTR_DESC:		pBuf->Appendf ( " %s %s DESC", sPrefix, sSubst ); break;
	case SPH_SORT_ATTR_ASC:			pBuf->Appendf ( " %s %s ASC", sPrefix, sSubst ); break;
	case SPH_SORT_TIME_SEGMENTS:	pBuf->Appendf ( " %s TIME_SEGMENT(%s)", sPrefix, sSubst ); break;
	case SPH_SORT_EXTENDED:			pBuf->Appendf ( " %s %s", sPrefix, sSubst ); break;
	case SPH_SORT_EXPR:				pBuf->Appendf ( " %s BUILTIN_EXPR()", sPrefix ); break;
	case SPH_SORT_RELEVANCE:		pBuf->Appendf ( " %s weight() desc%s%s", sPrefix, ( sSubst && *sSubst ? ", " : "" ), ( sSubst && *sSubst ? sSubst : "" ) ); break;
	default:						pBuf->Appendf ( " %s mode-%d", sPrefix, (int)eSort ); break;
	}
}

static void LogQuerySphinxql ( const CSphQuery & q, const CSphQueryResult & tRes, const CSphVector<int64_t> & dAgentTimes, int iCid )
{
	assert ( g_eLogFormat==LOG_FORMAT_SPHINXQL );
	if ( g_iQueryLogFile<0 )
		return;

	CSphStringBuilder tBuf;

	// time, conn id, wall, found
	int iQueryTime = Max ( tRes.m_iQueryTime, 0 );
	int iRealTime = Max ( tRes.m_iRealQueryTime, 0 );

	char sTimeBuf[SPH_TIME_PID_MAX_SIZE];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	tBuf += "/""* ";
	tBuf += sTimeBuf;

	if ( tRes.m_iMultiplier>1 )
		tBuf.Appendf ( " conn %d real %d.%03d wall %d.%03d x%d found "INT64_FMT" *""/ ",
			iCid, iRealTime/1000, iRealTime%1000, iQueryTime/1000, iQueryTime%1000, tRes.m_iMultiplier, tRes.m_iTotalMatches );
	else
		tBuf.Appendf ( " conn %d real %d.%03d wall %d.%03d found "INT64_FMT" *""/ ",
			iCid, iRealTime/1000, iRealTime%1000, iQueryTime/1000, iQueryTime%1000, tRes.m_iTotalMatches );

	///////////////////////////////////
	// format request as SELECT query
	///////////////////////////////////

	if ( q.m_bHasOuter )
		tBuf += "SELECT * FROM (";

	UnBackquote_fn tUnquoted ( q.m_sSelect.cstr() );
	tBuf.Appendf ( "SELECT %s FROM %s", tUnquoted.cstr(), q.m_sIndexes.cstr() );

	// WHERE clause
	// (m_sRawQuery is empty when using MySQL handler)
	const CSphString & sQuery = q.m_sQuery;
	if ( !sQuery.IsEmpty() || q.m_dFilters.GetLength() )
	{
		bool bDeflowered = false;

		tBuf += " WHERE";
		if ( !sQuery.IsEmpty() )
		{
			tBuf += " MATCH('";
			tBuf.AppendEscaped ( sQuery.cstr() );
			tBuf += "')";
			bDeflowered = true;
		}

		ARRAY_FOREACH ( i, q.m_dFilters )
		{
			if ( bDeflowered )
				tBuf += " AND";
			bDeflowered = true;

			const CSphFilterSettings & f = q.m_dFilters[i];
			switch ( f.m_eType )
			{
				case SPH_FILTER_VALUES:
					if ( f.m_dValues.GetLength()==1 )
					{
						if ( f.m_bExclude )
							tBuf.Appendf ( " %s!="INT64_FMT, f.m_sAttrName.cstr(), (int64_t)f.m_dValues[0] );
						else
							tBuf.Appendf ( " %s="INT64_FMT, f.m_sAttrName.cstr(), (int64_t)f.m_dValues[0] );
					} else
					{
						if ( f.m_bExclude )
							tBuf.Appendf ( " %s NOT IN (", f.m_sAttrName.cstr() );
						else
							tBuf.Appendf ( " %s IN (", f.m_sAttrName.cstr() );

						if ( g_bLogCompactIn && ( LOG_COMPACT_IN+1<f.m_dValues.GetLength() ) )
						{
							// for really long IN-lists optionally format them as N,N,N,N,...N,N,N, with ellipsis inside.
							int iLimit = LOG_COMPACT_IN-3;
							for ( int j=0; j<iLimit; ++j )
							{
								if ( j )
									tBuf.Appendf ( ","INT64_FMT, (int64_t)f.m_dValues[j] );
								else
									tBuf.Appendf ( INT64_FMT, (int64_t)f.m_dValues[j] );
							}
							iLimit = f.m_dValues.GetLength();
							tBuf.Appendf ( "%s", ",..." );
							for ( int j=iLimit-3; j<iLimit; ++j )
							{
								if ( j )
									tBuf.Appendf ( ","INT64_FMT, (int64_t)f.m_dValues[j] );
								else
									tBuf.Appendf ( INT64_FMT, (int64_t)f.m_dValues[j] );
							}

						} else
							ARRAY_FOREACH ( j, f.m_dValues )
							{
								if ( j )
									tBuf.Appendf ( ","INT64_FMT, (int64_t)f.m_dValues[j] );
								else
									tBuf.Appendf ( INT64_FMT, (int64_t)f.m_dValues[j] );
							}
						tBuf += ")";
					}
					break;

				case SPH_FILTER_RANGE:
					if ( f.m_iMinValue==int64_t(INT64_MIN) || ( f.m_iMinValue==0 && f.m_sAttrName=="@id" ) )
					{
						// no min, thus (attr<maxval)
						const char * sOps[2][2] = { { "<", "<=" }, { ">=", ">" } };
						tBuf.Appendf ( " %s%s"INT64_FMT, f.m_sAttrName.cstr(),
							sOps [ f.m_bExclude ][ f.m_bHasEqual ], f.m_iMaxValue );
					} else if ( f.m_iMaxValue==INT64_MAX || ( f.m_iMaxValue==-1 && f.m_sAttrName=="@id" ) )
					{
						// mo max, thus (attr>minval)
						const char * sOps[2][2] = { { ">", ">=" }, { "<", "<=" } };
						tBuf.Appendf ( " %s%s"INT64_FMT, f.m_sAttrName.cstr(),
							sOps [ f.m_bExclude ][ f.m_bHasEqual ], f.m_iMinValue );
					} else
					{
						tBuf.Appendf ( " %s%s BETWEEN "INT64_FMT" AND "INT64_FMT,
							f.m_sAttrName.cstr(), f.m_bExclude ? " NOT" : "",
							f.m_iMinValue + !f.m_bHasEqual, f.m_iMaxValue - !f.m_bHasEqual );
					}
					break;

				case SPH_FILTER_FLOATRANGE:
					if ( f.m_fMinValue==-FLT_MAX )
					{
						// no min, thus (attr<maxval)
						const char * sOps[2][2] = { { "<", "<=" }, { ">=", ">" } };
						tBuf.Appendf ( " %s%s%f", f.m_sAttrName.cstr(),
							sOps [ f.m_bExclude ][ f.m_bHasEqual ], f.m_fMaxValue );
					} else if ( f.m_fMaxValue==FLT_MAX )
					{
						// mo max, thus (attr>minval)
						const char * sOps[2][2] = { { ">", ">=" }, { "<", "<=" } };
						tBuf.Appendf ( " %s%s%f", f.m_sAttrName.cstr(),
							sOps [ f.m_bExclude ][ f.m_bHasEqual ], f.m_fMinValue );
					} else
					{
						// FIXME? need we handle m_bHasEqual here?
						tBuf.Appendf ( " %s%s BETWEEN %f AND %f",
							f.m_sAttrName.cstr(), f.m_bExclude ? " NOT" : "",
							f.m_fMinValue, f.m_fMaxValue );
					}
					break;

				case SPH_FILTER_USERVAR:
				case SPH_FILTER_STRING:
					tBuf.Appendf ( " %s%s'%s'", f.m_sAttrName.cstr(), ( f.m_bHasEqual ? "=" : "!=" ), f.m_sRefString.cstr() );
					break;

				case SPH_FILTER_NULL:
					tBuf.Appendf ( " %s %s", f.m_sAttrName.cstr(), ( f.m_bHasEqual ? "IS NULL" : "IS NOT NULL" ) );
					break;

				default:
					tBuf += " 1 /""* oops, unknown filter type *""/";
					break;
			}
		}
	}

	// ORDER BY and/or GROUP BY clause
	if ( q.m_sGroupBy.IsEmpty() )
	{
		if ( !q.m_sSortBy.IsEmpty() ) // case API SPH_MATCH_EXTENDED2 - SPH_SORT_RELEVANCE
			FormatOrderBy ( &tBuf, " ORDER BY", q.m_eSort, q.m_sSortBy );
	} else
	{
		tBuf.Appendf ( " GROUP BY %s", q.m_sGroupBy.cstr() );
		FormatOrderBy ( &tBuf, "WITHIN GROUP ORDER BY", q.m_eSort, q.m_sSortBy );
		if ( q.m_sGroupSortBy!="@group desc" )
			FormatOrderBy ( &tBuf, "ORDER BY", SPH_SORT_EXTENDED, q.m_sGroupSortBy );
	}

	// LIMIT clause
	if ( q.m_iOffset!=0 || q.m_iLimit!=20 )
		tBuf.Appendf ( " LIMIT %d,%d", q.m_iOffset, q.m_iLimit );

	// OPTION clause
	int iOpts = 0;

	if ( q.m_iMaxMatches!=DEFAULT_MAX_MATCHES )
	{
		tBuf.Appendf ( iOpts++ ? ", " : " OPTION " );
		tBuf.Appendf ( "max_matches=%d", q.m_iMaxMatches );
	}

	if ( !q.m_sComment.IsEmpty() )
	{
		tBuf.Appendf ( iOpts++ ? ", " : " OPTION " );
		tBuf.Appendf ( "comment='%s'", q.m_sComment.cstr() ); // FIXME! escape, replace newlines..
	}

	if ( q.m_eRanker!=SPH_RANK_DEFAULT )
	{
		const char * sRanker = sphGetRankerName ( q.m_eRanker );
		if ( !sRanker )
			sRanker = sphGetRankerName ( SPH_RANK_DEFAULT );

		tBuf.Appendf ( iOpts++ ? ", " : " OPTION " );
		tBuf.Appendf ( "ranker=%s", sRanker );

		if ( !q.m_sRankerExpr.IsEmpty() )
			tBuf.Appendf ( "(\'%s\')", q.m_sRankerExpr.scstr() );
	}

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

	///////////////
	// query stats
	///////////////

	if ( !tRes.m_sError.IsEmpty() )
	{
		// all we have is an error
		tBuf.Appendf ( " /""* error=%s */", tRes.m_sError.cstr() );

	} else if ( g_bIOStats || g_bCpuStats || dAgentTimes.GetLength() || !tRes.m_sWarning.IsEmpty() )
	{
		// got some extra data, add a comment
		tBuf += " /""*";

		// performance counters
		if ( g_bIOStats || g_bCpuStats )
		{
			const CSphIOStats & IOStats = tRes.m_tIOStats;

			if ( g_bIOStats )
				tBuf.Appendf ( " ios=%d kb=%d.%d ioms=%d.%d",
				IOStats.m_iReadOps, (int)( IOStats.m_iReadBytes/1024 ), (int)( IOStats.m_iReadBytes%1024 )*10/1024,
				(int)( IOStats.m_iReadTime/1000 ), (int)( IOStats.m_iReadTime%1000 )/100 );

			if ( g_bCpuStats )
				tBuf.Appendf ( " cpums=%d.%d", (int)( tRes.m_iCpuTime/1000 ), (int)( tRes.m_iCpuTime%1000 )/100 );
		}

		// per-agent times
		if ( dAgentTimes.GetLength() )
		{
			tBuf += " agents=(";
			ARRAY_FOREACH ( i, dAgentTimes )
				tBuf.Appendf ( i ? ", %d.%03d" : "%d.%03d",
					(int)(dAgentTimes[i]/1000),
					(int)(dAgentTimes[i]%1000) );

			tBuf += ")";
		}

		// warning
		if ( !tRes.m_sWarning.IsEmpty() )
			tBuf.Appendf ( " warning=%s", tRes.m_sWarning.cstr() );

		// close the comment
		tBuf += " */";
	}

	// line feed
	tBuf += "\n";

	sphSeek ( g_iQueryLogFile, 0, SEEK_END );
	sphWrite ( g_iQueryLogFile, tBuf.cstr(), tBuf.Length() );
}


static void LogQuery ( const CSphQuery & q, const CSphQueryResult & tRes, const CSphVector<int64_t> & dAgentTimes, int iCid )
{
	if ( g_iQueryLogMinMsec>0 && tRes.m_iQueryTime<g_iQueryLogMinMsec )
		return;

	switch ( g_eLogFormat )
	{
		case LOG_FORMAT_PLAIN:		LogQueryPlain ( q, tRes ); break;
		case LOG_FORMAT_SPHINXQL:	LogQuerySphinxql ( q, tRes, dAgentTimes, iCid ); break;
	}
}


static void LogSphinxqlError ( const char * sStmt, const char * sError, int iCid )
{
	if ( g_eLogFormat!=LOG_FORMAT_SPHINXQL || g_iQueryLogFile<0 || !sStmt || !sError )
		return;

	// time, conn id, query, error
	CSphStringBuilder tBuf;

	char sTimeBuf[SPH_TIME_PID_MAX_SIZE];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	tBuf += "/""* ";
	tBuf += sTimeBuf;
	tBuf.Appendf ( " conn %d *""/ %s # error=%s\n", iCid, sStmt, sError );

	sphSeek ( g_iQueryLogFile, 0, SEEK_END );
	sphWrite ( g_iQueryLogFile, tBuf.cstr(), tBuf.Length() );
}

//////////////////////////////////////////////////////////////////////////

// internals attributes are last no need to send them
static int SendGetAttrCount ( const ISphSchema & tSchema, bool bAgentMode=false )
{
	int iCount = tSchema.GetAttrsCount();

	if ( bAgentMode )
		return iCount;

	if ( iCount && sphIsSortStringInternal ( tSchema.GetAttr ( iCount-1 ).m_sName.cstr() ) )
	{
		for ( int i=iCount-1; i>=0 && sphIsSortStringInternal ( tSchema.GetAttr(i).m_sName.cstr() ); i-- )
		{
			iCount = i;
		}
	}

	return iCount;
}

static char g_sJsonNull[] = "{}";

int CalcResultLength ( int iVer, const CSphQueryResult * pRes, const CSphTaggedVector & dTag2Pools, bool bAgentMode, const CSphQuery & tQuery, int iMasterVer )
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

	int iAttrsCount = SendGetAttrCount ( pRes->m_tSchema, bAgentMode );

	// schema
	if ( iVer>=0x102 )
	{
		iRespLen += 8; // 4 for field count, 4 for attr count
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dFields )
			iRespLen += 4 + strlen ( pRes->m_tSchema.m_dFields[i].m_sName.cstr() ); // namelen, name
		for ( int i=0; i<iAttrsCount; i++ )
			iRespLen += 8 + strlen ( pRes->m_tSchema.GetAttr(i).m_sName.cstr() ); // namelen, name, type
	}

	// matches
	if ( iVer<0x102 )
		iRespLen += 16*pRes->m_iCount; // matches
	else if ( iVer<0x108 )
		iRespLen += ( 8+4*iAttrsCount )*pRes->m_iCount; // matches
	else
		iRespLen += 4 + ( 8+4*USE_64BIT+4*iAttrsCount )*pRes->m_iCount; // id64 tag and matches

	if ( iVer>=0x114 )
	{
		// 64bit matches
		int iWideAttrs = 0;
		for ( int i=0; i<iAttrsCount; i++ )
			if ( pRes->m_tSchema.GetAttr(i).m_eAttrType==SPH_ATTR_BIGINT )
				iWideAttrs++;
		iRespLen += 4*pRes->m_iCount*iWideAttrs; // extra 4 bytes per attr per match
	}

	// agents send additional flag from words statistics
	if ( bAgentMode )
		iRespLen += pRes->m_hWordStats.GetLength();

	pRes->m_hWordStats.IterateStart();
	while ( pRes->m_hWordStats.IterateNext() ) // per-word stats
		iRespLen += 12 + strlen ( pRes->m_hWordStats.IterateGetKey().cstr() ); // wordlen, word, docs, hits

	bool bSendJson = ( bAgentMode && iMasterVer>=3 );
	bool bSendJsonField = ( bAgentMode && iMasterVer>=4 );

	// all pooled values
	CSphVector<CSphAttrLocator> dMvaItems;
	CSphVector<CSphAttrLocator> dStringItems;
	CSphVector<CSphAttrLocator> dStringPtrItems;
	CSphVector<CSphAttrLocator> dJsonItems;
	CSphVector<CSphAttrLocator> dJsonFieldsItems;
	CSphVector<CSphAttrLocator> dFactorItems;
	for ( int i=0; i<iAttrsCount; i++ )
	{
		const CSphColumnInfo & tCol = pRes->m_tSchema.GetAttr(i);
		switch ( tCol.m_eAttrType )
		{
		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
			dMvaItems.Add ( tCol.m_tLocator );
			break;
		case SPH_ATTR_STRING:
			dStringItems.Add ( tCol.m_tLocator );
			break;
		case SPH_ATTR_STRINGPTR:
			dStringPtrItems.Add ( tCol.m_tLocator );
			break;
		case SPH_ATTR_JSON:
			dJsonItems.Add ( tCol.m_tLocator );
			break;
		case SPH_ATTR_JSON_FIELD:
			dJsonFieldsItems.Add ( tCol.m_tLocator );
			break;
		case SPH_ATTR_FACTORS:
		case SPH_ATTR_FACTORS_JSON:
			dFactorItems.Add ( tCol.m_tLocator );
			break;
		default:
			break;
		}
	}

	if ( iVer>=0x10C && dMvaItems.GetLength() )
	{
		for ( int i=0; i<pRes->m_iCount; i++ )
		{
			const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];
			const DWORD * pMvaPool = dTag2Pools [ tMatch.m_iTag ].m_pMva;
			bool bArenaProhibit = dTag2Pools[tMatch.m_iTag].m_bArenaProhibit;
			ARRAY_FOREACH ( j, dMvaItems )
			{
				assert ( tMatch.GetAttr ( dMvaItems[j] )==0 || pMvaPool );
				const DWORD * pMva = tMatch.GetAttrMVA ( dMvaItems[j], pMvaPool, bArenaProhibit );
				if ( pMva )
					iRespLen += pMva[0]*4; // FIXME? maybe add some sanity check here
			}
		}
	}

	if ( iVer>=0x117 && dStringItems.GetLength() )
	{
		for ( int i=0; i<pRes->m_iCount; i++ )
		{
			const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];
			const BYTE * pStrings = dTag2Pools [ tMatch.m_iTag ].m_pStrings;
			ARRAY_FOREACH ( j, dStringItems )
			{
				DWORD uOffset = (DWORD) tMatch.GetAttr ( dStringItems[j] );
				assert ( !uOffset || pStrings );
				if ( uOffset ) // magic zero
					iRespLen += sphUnpackStr ( pStrings+uOffset, NULL );
			}
		}
	}

	if ( iVer>=0x11A && bAgentMode )
	{
		iRespLen += 1;			// stats mask
		if ( g_bIOStats )
			iRespLen += 40;		// IO Stats

		if ( g_bCpuStats )
			iRespLen += 8;		// CPU Stats

		if ( tQuery.m_iMaxPredictedMsec > 0 )
			iRespLen += 8;		// predicted time
	}
	// fetched_docs and fetched_hits from agent to master
	if ( bAgentMode && iMasterVer>=7 )
		iRespLen += 12;

	if ( iVer>=0x117 && dStringPtrItems.GetLength() )
	{
		for ( int i=0; i<pRes->m_iCount; i++ )
		{
			const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];
			ARRAY_FOREACH ( j, dStringPtrItems )
			{
				const char* pStr = (const char*) tMatch.GetAttr ( dStringPtrItems[j] );
				if ( pStr )
					iRespLen += strlen ( pStr );
			}
		}
	}

	if ( iVer>=0x117 && dJsonItems.GetLength() )
	{
		CSphVector<BYTE> dJson ( 512 );
		// to master pass JSON as raw data
		for ( int i=0; i<pRes->m_iCount; i++ )
		{
			const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];
			const BYTE * pPool = dTag2Pools [ tMatch.m_iTag ].m_pStrings;
			ARRAY_FOREACH ( j, dJsonItems )
			{
				DWORD uOffset = (DWORD) tMatch.GetAttr ( dJsonItems[j] );
				assert ( !uOffset || pPool );
				if ( !uOffset ) // magic zero
				{
					if ( !bSendJson ) // for client JSON magic zero is {}
						iRespLen += sizeof(g_sJsonNull)-1;
					continue;
				}

				const BYTE * pStr = NULL;
				int iRawLen = sphUnpackStr ( pPool + uOffset, &pStr );

				if ( bSendJson )
				{
					iRespLen += iRawLen;
				} else
				{
					dJson.Resize ( 0 );
					sphJsonFormat ( dJson, pStr );
					iRespLen += dJson.GetLength();
				}
			}
		}
	}

	if ( iVer>=0x117 && dJsonFieldsItems.GetLength() )
	{
		CSphVector<BYTE> dJson ( 512 );

		for ( int i=0; i<pRes->m_iCount; i++ )
		{
			const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];
			const BYTE * pStrings = dTag2Pools [ tMatch.m_iTag ].m_pStrings;
			ARRAY_FOREACH ( j, dJsonFieldsItems )
			{
				// sizeof(DWORD) count already
				uint64_t uTypeOffset = tMatch.GetAttr ( dJsonFieldsItems[j] );
				assert ( !uTypeOffset || pStrings );
				if ( !uTypeOffset ) // magic zero
				{
					if ( bSendJsonField )
						iRespLen -= 3; // agent sends to master JSON type as BYTE
					continue;
				}

				ESphJsonType eJson = ESphJsonType ( uTypeOffset>>32 );
				DWORD uOff = (DWORD)uTypeOffset;
				if ( bSendJsonField )
				{
					const BYTE * pData = pStrings+uOff;
					iRespLen -= 3; // JSON type as BYTE
					iRespLen += sphJsonNodeSize ( eJson, pData );
					if ( sphJsonNodeSize ( eJson, NULL )<0 )
						iRespLen += 4;
				} else
				{
					// to client send as string
					dJson.Resize ( 0 );
					sphJsonFieldFormat ( dJson, pStrings+uOff, eJson, false );
					iRespLen += dJson.GetLength();
				}
			}
		}
	}

	if ( iVer>=0x11C && dFactorItems.GetLength() )
	{
		for ( int i=0; i<pRes->m_iCount; i++ )
		{
			const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];
			ARRAY_FOREACH ( j, dFactorItems )
			{
				DWORD * pData = (DWORD *) tMatch.GetAttr ( dFactorItems[j] );
				if ( pData )
					iRespLen += *pData-sizeof(DWORD);
			}
		}
	}

	return iRespLen;
}


void SendResult ( int iVer, ISphOutputBuffer & tOut, const CSphQueryResult * pRes,
					const CSphTaggedVector & dTag2Pools, bool bAgentMode, const CSphQuery & tQuery, int iMasterVer )
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
			if ( g_bOptNoDetach && g_eLogFormat!=LOG_FORMAT_SPHINXQL )
				sphInfo ( "query error: %s", pRes->m_sError.cstr() );
			return;

		} else if ( bWarning )
		{
			tOut.SendInt ( SEARCHD_WARNING );
			tOut.SendString ( pRes->m_sWarning.cstr() );
			if ( g_bOptNoDetach && g_eLogFormat!=LOG_FORMAT_SPHINXQL )
				sphInfo ( "query warning: %s", pRes->m_sWarning.cstr() );
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

	int iAttrsCount = SendGetAttrCount ( pRes->m_tSchema, bAgentMode );

	bool bSendJson = ( bAgentMode && iMasterVer>=3 );
	bool bSendJsonField = ( bAgentMode && iMasterVer>=4 );

	// send schema
	if ( iVer>=0x102 )
	{
		tOut.SendInt ( pRes->m_tSchema.m_dFields.GetLength() );
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dFields )
			tOut.SendString ( pRes->m_tSchema.m_dFields[i].m_sName.cstr() );

		tOut.SendInt ( iAttrsCount );
		for ( int i=0; i<iAttrsCount; i++ )
		{
			const CSphColumnInfo & tCol = pRes->m_tSchema.GetAttr(i);
			tOut.SendString ( tCol.m_sName.cstr() );

			ESphAttr eCol = tCol.m_eAttrType;
			if ( ( tCol.m_eAttrType==SPH_ATTR_JSON && !bSendJson ) || ( tCol.m_eAttrType==SPH_ATTR_JSON_FIELD && !bSendJsonField ) )
				eCol = SPH_ATTR_STRING;
			tOut.SendDword ( (DWORD)eCol );
		}
	}

	// send matches
	CSphAttrLocator iGIDLoc, iTSLoc;
	if ( iVer<=0x101 )
	{
		for ( int i=0; i<pRes->m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tAttr = pRes->m_tSchema.GetAttr(i);

			if ( iTSLoc.m_iBitOffset<0 && tAttr.m_eAttrType==SPH_ATTR_TIMESTAMP )
				iTSLoc = tAttr.m_tLocator;

			if ( iGIDLoc.m_iBitOffset<0 && tAttr.m_eAttrType==SPH_ATTR_INTEGER )
				iGIDLoc = tAttr.m_tLocator;
		}
	}

	tOut.SendInt ( pRes->m_iCount );
	if ( iVer>=0x108 )
		tOut.SendInt ( USE_64BIT );

	CSphVector<BYTE> dJson ( 512 );

	for ( int i=0; i<pRes->m_iCount; i++ )
	{
		const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];
#if USE_64BIT
		if ( iVer>=0x108 )
			tOut.SendUint64 ( tMatch.m_uDocID );
		else
#endif
			tOut.SendDword ( (DWORD)tMatch.m_uDocID );

		if ( iVer<=0x101 )
		{
			tOut.SendDword ( iGIDLoc.m_iBitOffset>=0 ? (DWORD) tMatch.GetAttr ( iGIDLoc ) : 1 );
			tOut.SendDword ( iTSLoc.m_iBitOffset>=0 ? (DWORD) tMatch.GetAttr ( iTSLoc ) : 1 );
			tOut.SendInt ( tMatch.m_iWeight );
		} else
		{
			tOut.SendInt ( tMatch.m_iWeight );

			const DWORD * pMvaPool = dTag2Pools [ tMatch.m_iTag ].m_pMva;
			const BYTE * pStrings = dTag2Pools [ tMatch.m_iTag ].m_pStrings;
			bool bArenaProhibit = dTag2Pools[tMatch.m_iTag].m_bArenaProhibit;

			assert ( tMatch.m_pStatic || !pRes->m_tSchema.GetStaticSize() );
#if 0
			// not correct any more because of internal attrs (such as string sorting ptrs)
			assert ( tMatch.m_pDynamic || !pRes->m_tSchema.GetDynamicSize() );
			assert ( !tMatch.m_pDynamic || (int)tMatch.m_pDynamic[-1]==pRes->m_tSchema.GetDynamicSize() );
#endif

			for ( int j=0; j<iAttrsCount; j++ )
			{
				const CSphColumnInfo & tAttr = pRes->m_tSchema.GetAttr(j);
				switch ( tAttr.m_eAttrType )
				{
				case SPH_ATTR_UINT32SET:
				case SPH_ATTR_INT64SET:
					{
						assert ( tMatch.GetAttr ( tAttr.m_tLocator )==0 || pMvaPool );
						const DWORD * pValues = tMatch.GetAttrMVA ( tAttr.m_tLocator, pMvaPool, bArenaProhibit );
						if ( iVer<0x10C || !pValues )
						{
							// for older clients, fixups column value to 0
							// for newer clients, means that there are 0 values
							tOut.SendDword ( 0 );
						} else
						{
							// send MVA values
							int iValues = *pValues++;
							tOut.SendDword ( iValues );
							if ( tAttr.m_eAttrType==SPH_ATTR_INT64SET )
							{
								assert ( ( iValues%2 )==0 );
								while ( iValues )
								{
									uint64_t uVal = (uint64_t)MVA_UPSIZE ( pValues );
									tOut.SendUint64 ( uVal );
									pValues += 2;
									iValues -= 2;
								}
							} else
							{
								while ( iValues-- )
									tOut.SendDword ( *pValues++ );
							}
						}
						break;
					}
				case SPH_ATTR_JSON:
					{
						if ( iVer<0x117 )
						{
							tOut.SendDword ( 0 );
							break;
						}
						if ( !bSendJson )
						{
							// formatted string to client
							DWORD uOffset = (DWORD) tMatch.GetAttr ( tAttr.m_tLocator );
							assert ( !uOffset || pStrings );
							if ( !uOffset ) // magic zero
							{
								tOut.SendDword ( sizeof(g_sJsonNull)-1 );
								tOut.SendBytes ( g_sJsonNull, sizeof(g_sJsonNull)-1 );
							} else
							{
								dJson.Resize ( 0 );
								const BYTE * pStr = NULL;
								sphUnpackStr ( pStrings + uOffset, &pStr );
								sphJsonFormat ( dJson, pStr );

								tOut.SendDword ( dJson.GetLength() );
								tOut.SendBytes ( dJson.Begin(), dJson.GetLength() );
							}
							break;
						}
						// no break at the end, pass to SPH_ATTR_STRING
					}
				case SPH_ATTR_STRING:
					{
						if ( iVer<0x117 )
						{
							tOut.SendDword ( 0 );
							break;
						}
						// for newer clients, send binary string either STRING or JSON attribute
						DWORD uOffset = (DWORD) tMatch.GetAttr ( tAttr.m_tLocator );
						if ( !uOffset ) // magic zero
						{
							tOut.SendDword ( 0 ); // null string
						} else
						{
							const BYTE * pStr;
							assert ( pStrings );
							int iLen = sphUnpackStr ( pStrings+uOffset, &pStr );
							tOut.SendDword ( iLen );
							tOut.SendBytes ( pStr, iLen );
						}
						break;
					}
				case SPH_ATTR_STRINGPTR:
					{
						if ( iVer<0x117 )
						{
							tOut.SendDword ( 0 );
							break;
						}
						// for newer clients, send binary string
						const char* pString = (const char*) tMatch.GetAttr ( tAttr.m_tLocator );
						if ( !pString ) // magic zero
						{
							tOut.SendDword ( 0 ); // null string
						} else
						{
							int iLen = strlen ( pString );
							tOut.SendDword ( iLen );
							tOut.SendBytes ( pString, iLen );
						}
						break;
					}
				case SPH_ATTR_JSON_FIELD:
					{
						if ( iVer<0x117 )
						{
							tOut.SendDword ( 0 );
							break;
						}

						uint64_t uTypeOffset = tMatch.GetAttr ( tAttr.m_tLocator );
						ESphJsonType eJson = ESphJsonType ( uTypeOffset>>32 );
						DWORD uOff = (DWORD)uTypeOffset;
						assert ( !uOff || pStrings );
						if ( !uOff )
						{
							// no key found - NULL value
							if ( bSendJsonField )
								tOut.SendByte ( JSON_EOF );
							else
								tOut.SendDword ( 0 );

						} else if ( bSendJsonField )
						{
							// to master send packed data
							tOut.SendByte ( (BYTE)eJson );

							const BYTE * pData = pStrings+uOff;
							int iLen = sphJsonNodeSize ( eJson, pData );
							if ( sphJsonNodeSize ( eJson, NULL )<0 )
								tOut.SendDword ( iLen );
							tOut.SendBytes ( pData, iLen );
						} else
						{
							// to client send data as string
							dJson.Resize ( 0 );
							sphJsonFieldFormat ( dJson, pStrings+uOff, eJson, false );
							tOut.SendDword ( dJson.GetLength() );
							tOut.SendBytes ( dJson.Begin(), dJson.GetLength() );
						}
						break;
					}
				case SPH_ATTR_FACTORS:
				case SPH_ATTR_FACTORS_JSON:
					{
						if ( iVer<0x11C )
						{
							tOut.SendDword ( 0 );
							break;
						}
						BYTE * pData = (BYTE*) tMatch.GetAttr ( tAttr.m_tLocator );
						if ( !pData )
							tOut.SendDword ( 0 );
						else
						{
							DWORD uLength = *(DWORD*)pData;
							tOut.SendDword ( uLength );
							tOut.SendBytes ( pData+sizeof(DWORD), uLength-sizeof(DWORD) );
						}
						break;
					}
				case SPH_ATTR_FLOAT:
					tOut.SendFloat ( tMatch.GetAttrFloat ( tAttr.m_tLocator ) );
					break;
				case SPH_ATTR_BIGINT:
					if ( iVer>=0x114 )
					{
						tOut.SendUint64 ( tMatch.GetAttr ( tAttr.m_tLocator ) );
						break;
					}
					// no break here
				default:
					tOut.SendDword ( (DWORD)tMatch.GetAttr ( tAttr.m_tLocator ) );
					break;
				} /// end switch ( tAttr.m_eAttrType )
			} /// end for ( int j=0; j<iAttrsCount; j++ )
		} /// end else if ( iVer<=0x101 )
	} /// end for ( int i=0; i<pRes->m_iCount; i++ )

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
	if ( bAgentMode && iMasterVer>=7 )
	{
		tOut.SendDword ( pRes->m_tStats.m_iFetchedDocs + pRes->m_iAgentFetchedDocs );
		tOut.SendDword ( pRes->m_tStats.m_iFetchedHits + pRes->m_iAgentFetchedHits );
		if ( iMasterVer>=8 )
			tOut.SendDword ( pRes->m_tStats.m_iSkips + pRes->m_iAgentFetchedSkips );
	}

	tOut.SendInt ( pRes->m_hWordStats.GetLength() );

	pRes->m_hWordStats.IterateStart();
	while ( pRes->m_hWordStats.IterateNext() )
	{
		const CSphQueryResultMeta::WordStat_t & tStat = pRes->m_hWordStats.IterateGet();
		tOut.SendString ( pRes->m_hWordStats.IterateGetKey().cstr() );
		tOut.SendAsDword ( tStat.m_iDocs );
		tOut.SendAsDword ( tStat.m_iHits );
		if ( bAgentMode )
			tOut.SendByte ( false ); // statistics have no expanded terms for now
	}
}

/////////////////////////////////////////////////////////////////////////////

void AggrResult_t::ClampMatches ( int iLimit, bool bCommonSchema )
{
	if ( m_dMatches.GetLength()<=iLimit )
		return;

	if ( bCommonSchema )
	{
		for ( int i = iLimit; i < m_dMatches.GetLength(); i++ )
			m_tSchema.FreeStringPtrs ( &m_dMatches[i] );
	} else
	{
		int nMatches = 0;
		ARRAY_FOREACH ( i, m_dMatchCounts )
		{
			nMatches += m_dMatchCounts[i];

			if ( iLimit < nMatches )
			{
				int iFrom = Max ( iLimit, nMatches-m_dMatchCounts[i] );
				for ( int j=iFrom; j<nMatches; j++ )
					m_dSchemas[i].FreeStringPtrs ( &m_dMatches[j] );
			}
		}
	}

	m_dMatches.Resize ( iLimit );
}


struct TaggedMatchSorter_fn : public SphAccessor_T<CSphMatch>
{
	void CopyKey ( CSphMatch * pMed, CSphMatch * pVal ) const
	{
		pMed->m_uDocID = pVal->m_uDocID;
		pMed->m_iTag = pVal->m_iTag;
	}

	bool IsLess ( const CSphMatch & a, const CSphMatch & b ) const
	{
		bool bDistA = ( ( a.m_iTag & 0x80000000 )==0x80000000 );
		bool bDistB = ( ( b.m_iTag & 0x80000000 )==0x80000000 );
		// sort by doc_id, dist_tag, tag
		return ( a.m_uDocID < b.m_uDocID ) ||
			( a.m_uDocID==b.m_uDocID && ( ( !bDistA && bDistB ) || ( ( a.m_iTag & 0x7FFFFFFF )>( b.m_iTag & 0x7FFFFFFF ) ) ) );
	}

	// inherited swap does not work on gcc
	void Swap ( CSphMatch * a, CSphMatch * b ) const
	{
		::Swap ( *a, *b );
	}
};


void RemapResult ( const ISphSchema * pTarget, AggrResult_t * pRes )
{
	int iCur = 0;
	CSphVector<int> dMapFrom ( pTarget->GetAttrsCount() );

	ARRAY_FOREACH ( iSchema, pRes->m_dSchemas )
	{
		dMapFrom.Resize ( 0 );
		CSphRsetSchema & dSchema = pRes->m_dSchemas[iSchema];
		for ( int i=0; i<pTarget->GetAttrsCount(); i++ )
		{
			dMapFrom.Add ( dSchema.GetAttrIndex ( pTarget->GetAttr(i).m_sName.cstr() ) );
			assert ( dMapFrom[i]>=0
				|| pTarget->GetAttr(i).m_tLocator.IsID()
				|| sphIsSortStringInternal ( pTarget->GetAttr(i).m_sName.cstr() )
				|| pTarget->GetAttr(i).m_sName=="@groupbystr"
				);
		}
		int iLimit = Min ( iCur + pRes->m_dMatchCounts[iSchema], pRes->m_dMatches.GetLength() );
		for ( int i=iCur; i<iLimit; i++ )
		{
			CSphMatch & tMatch = pRes->m_dMatches[i];

			// create new and shiny (and properly sized) match
			CSphMatch tRow;
			tRow.Reset ( pTarget->GetDynamicSize() );
			tRow.m_uDocID = tMatch.m_uDocID;
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
		}

		iCur = iLimit;
	}
	assert ( iCur==pRes->m_dMatches.GetLength() );
}

// rebuild the results itemlist expanding stars
const CSphVector<CSphQueryItem> * ExpandAsterisk ( const ISphSchema & tSchema,
	const CSphVector<CSphQueryItem> & tItems, CSphVector<CSphQueryItem> * pExpanded, bool bNoID )
{
	// the result schema usually is the index schema + calculated items + @-items
	// we need to extract the index schema only - so, look at the items
	// and cutoff from calculated or @.
	int iSchemaBound = tSchema.GetAttrsCount();
	bool bStar = false;
	ARRAY_FOREACH ( i, tItems )
	{
		const CSphQueryItem & tItem = tItems[i];
		if ( tItem.m_sAlias.cstr() )
		{
			int j = tSchema.GetAttrIndex ( tItem.m_sAlias.cstr() );
			if ( j>=0 )
				iSchemaBound = Min ( iSchemaBound, j );
		}
		bStar = bStar || tItem.m_sExpr=="*";
	}
	// no stars? Nothing to do.
	if ( !bStar )
		return & tItems;

	while ( iSchemaBound && tSchema.GetAttr ( iSchemaBound-1 ).m_sName.cstr()[0]=='@' )
		iSchemaBound--;
	ARRAY_FOREACH ( i, tItems )
	{
		if ( tItems[i].m_sExpr=="*" )
		{ // asterisk expands to 'id' + all the items from the schema
			if ( tSchema.GetAttrIndex ( "id" )<0 && !bNoID )
			{
				CSphQueryItem& tItem = pExpanded->Add();
				tItem.m_sExpr = "id";
			}
			for ( int j=0; j<iSchemaBound; j++ )
			{
				if ( !j && bNoID && tSchema.GetAttr(j).m_sName=="id" )
					continue;
				CSphQueryItem& tItem = pExpanded->Add();
				tItem.m_sExpr = tSchema.GetAttr ( j ).m_sName;
			}
		} else
			pExpanded->Add ( tItems[i] );
	}
	return pExpanded;
}


static void RemapStrings ( ISphMatchSorter * pSorter, AggrResult_t & tRes )
{
	// do match ptr pre-calc if its "order by string" case
	CSphVector<SphStringSorterRemap_t> dRemapAttr;
	if ( pSorter && pSorter->UsesAttrs() && sphSortGetStringRemap ( pSorter->GetSchema(), tRes.m_tSchema, dRemapAttr ) )
	{
		int iCur = 0;
		ARRAY_FOREACH ( iSchema, tRes.m_dSchemas )
		{
			for ( int i=iCur; i<iCur+tRes.m_dMatchCounts[iSchema]; i++ )
			{
				CSphMatch & tMatch = tRes.m_dMatches[i];
				const BYTE * pStringBase = tRes.m_dTag2Pools[tMatch.m_iTag].m_pStrings;

				ARRAY_FOREACH ( iAttr, dRemapAttr )
				{
					SphAttr_t uOff = tMatch.GetAttr ( dRemapAttr[iAttr].m_tSrc );
					SphAttr_t uPtr = (SphAttr_t)( pStringBase && uOff ? pStringBase + uOff : 0 );
					tMatch.SetAttr ( dRemapAttr[iAttr].m_tDst, uPtr );
				}
			}
			iCur += tRes.m_dMatchCounts[iSchema];
		}
	}
}


static int KillAllDupes ( ISphMatchSorter * pSorter, AggrResult_t & tRes )
{
	assert ( pSorter );
	int iDupes = 0;

	if ( pSorter->IsGroupby () )
	{
		// groupby sorter does that automagically
		pSorter->SetMVAPool ( NULL, false ); // because we must be able to group on @groupby anyway
		pSorter->SetStringPool ( NULL );
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
		// normal sorter needs massasging
		// sort by docid and then by tag to guarantee the replacement order
		TaggedMatchSorter_fn fnSort;
		sphSort ( tRes.m_dMatches.Begin(), tRes.m_dMatches.GetLength(), fnSort, fnSort );

		// by default, simply remove dupes (select first by tag)
		ARRAY_FOREACH ( i, tRes.m_dMatches )
		{
			if ( i==0 || tRes.m_dMatches[i].m_uDocID!=tRes.m_dMatches[i-1].m_uDocID )
				pSorter->Push ( tRes.m_dMatches[i] );
			else
				iDupes++;
		}
	}

	ARRAY_FOREACH ( i, tRes.m_dMatches )
		tRes.m_tSchema.FreeStringPtrs ( &(tRes.m_dMatches[i]) );

	tRes.m_dMatches.Reset ();
	sphFlattenQueue ( pSorter, &tRes, -1 );
	SafeDelete ( pSorter );

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
			CSphColumnInfo & tCol = const_cast<CSphColumnInfo&> ( tRes.m_tSchema.GetAttr(j) );
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
			case SPH_KEYPART_ID:
				if ( a->m_uDocID==b->m_uDocID )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( a->m_uDocID < b->m_uDocID );

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
		return c.m_eAggrFunc!=SPH_AGGR_NONE || c.m_sName=="@groupby" || c.m_sName=="@count" || c.m_sName=="@distinct" || c.m_sName=="@groupbystr";
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


static void ExtractPostlimit ( const CSphRsetSchema & tSchema, CSphVector<const CSphColumnInfo *> & dPostlimit )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr ( i );
		if ( tCol.m_eStage==SPH_EVAL_POSTLIMIT )
			dPostlimit.Add ( &tCol );
	}
}


static void ProcessPostlimit ( const CSphVector<const CSphColumnInfo *> & dPostlimit, int iFrom, int iTo, AggrResult_t & tRes )
{
	if ( !dPostlimit.GetLength() )
		return;

	for ( int i=iFrom; i<iTo; i++ )
	{
		CSphMatch & tMatch = tRes.m_dMatches[i];
		// remote match (tag highest bit 1) == everything is already computed
		if ( tMatch.m_iTag & 0x80000000 )
			continue;

		ARRAY_FOREACH ( j, dPostlimit )
		{
			const CSphColumnInfo * pCol = dPostlimit[j];
			assert ( pCol->m_pExpr.Ptr() );

			// OPTIMIZE? only if the tag did not change?
			pCol->m_pExpr->Command ( SPH_EXPR_SET_MVA_POOL, &tRes.m_dTag2Pools [ tMatch.m_iTag ] );
			pCol->m_pExpr->Command ( SPH_EXPR_SET_STRING_POOL, (void*)tRes.m_dTag2Pools [ tMatch.m_iTag ].m_pStrings );

			if ( pCol->m_eAttrType==SPH_ATTR_INTEGER )
				tMatch.SetAttr ( pCol->m_tLocator, pCol->m_pExpr->IntEval(tMatch) );
			else if ( pCol->m_eAttrType==SPH_ATTR_BIGINT )
				tMatch.SetAttr ( pCol->m_tLocator, pCol->m_pExpr->Int64Eval(tMatch) );
			else if ( pCol->m_eAttrType==SPH_ATTR_STRINGPTR )
			{
				const BYTE * pStr = NULL;
				pCol->m_pExpr->StringEval ( tMatch, &pStr );
				tMatch.SetAttr ( pCol->m_tLocator, (SphAttr_t) pStr ); // FIXME! a potential leak of *previous* value?
			} else
			{
				tMatch.SetAttrFloat ( pCol->m_tLocator, pCol->m_pExpr->Eval(tMatch) );
			}
		}
	}
}


static void ProcessLocalPostlimit ( const CSphQuery & tQuery, AggrResult_t & tRes )
{
	bool bGotPostlimit = false;
	for ( int i=0; i<tRes.m_tSchema.GetAttrsCount() && !bGotPostlimit; i++ )
		bGotPostlimit = ( tRes.m_tSchema.GetAttr(i).m_eStage==SPH_EVAL_POSTLIMIT );

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
		ExtractPostlimit ( tRes.m_dSchemas[iSchema], dPostlimit );
		if ( !dPostlimit.GetLength() )
			continue;

		int iTo = iSetCount;
		int iOff = Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
		int iCount = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );
		iTo = Max ( Min ( iOff + iCount, iTo ), 0 );
		int iFrom = Min ( iOff, iTo );

		iFrom += iSetStart;
		iTo += iSetStart;

		ProcessPostlimit ( dPostlimit, iFrom, iTo, tRes );
	}
}


/// merges multiple result sets, remaps columns, does reorder for outer selects
/// query is only (!) non-const to tweak order vs reorder clauses
bool MinimizeAggrResult ( AggrResult_t & tRes, CSphQuery & tQuery, int iLocals, int ,
	CSphSchema * pExtraSchema, CSphQueryProfile * pProfiler, bool bFromSphinxql, const CSphFilterSettings * pAggrFilter )
{
	// sanity check
	// verify that the match counts are consistent
	int iExpected = 0;
	ARRAY_FOREACH ( i, tRes.m_dMatchCounts )
		iExpected += tRes.m_dMatchCounts[i];
	if ( iExpected!=tRes.m_dMatches.GetLength() )
	{
		tRes.m_sError.SetSprintf ( "INTERNAL ERROR: expected %d matches in combined result set, got %d",
			iExpected, tRes.m_dMatches.GetLength() );
		return false;
	}

	bool bReturnZeroCount = !tRes.m_sZeroCountName.IsEmpty();

	// 0 matches via SphinxAPI? no fiddling with schemes is necessary
	// (and via SphinxQL, we still need to return the right schema)
	if ( !bFromSphinxql && !tRes.m_dMatches.GetLength() )
		return true;

	// 0 result set schemes via SphinxQL? just bail
	if ( bFromSphinxql && !tRes.m_dSchemas.GetLength() && !bReturnZeroCount )
		return true;

	// build a minimal schema over all the (potentially different) schemes
	// that we have in our aggregated result set
	assert ( tRes.m_dSchemas.GetLength() || bReturnZeroCount );
	if ( tRes.m_dSchemas.GetLength() )
		tRes.m_tSchema = tRes.m_dSchemas[0];
	bool bAgent = tQuery.m_bAgent;
	bool bUsualApi = !bAgent && !bFromSphinxql;
	bool bAllEqual = true;

	// FIXME? add assert ( tRes.m_tSchema==tRes.m_dSchemas[0] );
	for ( int i=1; i<tRes.m_dSchemas.GetLength(); i++ )
		if ( !MinimizeSchema ( tRes.m_tSchema, tRes.m_dSchemas[i] ) )
			bAllEqual = false;

	// build a list of select items that the query asked for
	CSphVector<CSphQueryItem> tExtItems;
	const CSphVector<CSphQueryItem> * pItems = ExpandAsterisk ( tRes.m_tSchema, tQuery.m_dItems, &tExtItems, !bFromSphinxql );
	const CSphVector<CSphQueryItem> & tItems = *pItems;

	// api + index without attributes + select * case
	// can not skip aggregate filtering
	if ( !bFromSphinxql && !tItems.GetLength() && !pAggrFilter )
		return true;

	// build the final schemas!
	// ???
	CSphVector<CSphColumnInfo> dFrontend ( tItems.GetLength() );

	// no select items => must be nothing in minimized schema, right?
	assert ( !( tItems.GetLength()==0 && tRes.m_tSchema.GetAttrsCount()>0 ) );

	// track select items that made it into the internal schema
	CSphVector<int> dKnownItems;

	// we use this vector to reduce amount of work in next nested loop
	// instead of looping through all dItems we FOREACH only unmapped ones
	CSphVector<int> dUnmappedItems;
	ARRAY_FOREACH ( i, tItems )
	{
		int iCol = -1;
		if ( bFromSphinxql && tItems[i].m_sAlias.IsEmpty() )
			iCol = tRes.m_tSchema.GetAttrIndex ( tItems[i].m_sExpr.cstr() );

		if ( iCol>=0 )
		{
			dFrontend[i].m_sName = tItems[i].m_sExpr;
			dFrontend[i].m_iIndex = iCol;
			dKnownItems.Add(i);
		} else
			dUnmappedItems.Add(i);
	}

	// ???
	for ( int iCol=0; iCol<tRes.m_tSchema.GetAttrsCount(); iCol++ )
	{
		const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(iCol);

		assert ( !tCol.m_sName.IsEmpty() );
		bool bMagic = ( *tCol.m_sName.cstr()=='@' );

		if ( !bMagic && tCol.m_pExpr.Ptr() )
		{
			ARRAY_FOREACH ( j, dUnmappedItems )
				if ( tItems[ dUnmappedItems[j] ].m_sAlias==tCol.m_sName )
			{
				int k = dUnmappedItems[j];
				dFrontend[k].m_iIndex = iCol;
				dFrontend[k].m_sName = tItems[k].m_sAlias;
				dKnownItems.Add(k);
				dUnmappedItems.Remove ( j-- ); // do not skip an element next to removed one!
			}

			// FIXME?
			// really not sure if this is the right thing to do
			// but it fixes a couple queries in test_163 in compaitbility mode
			if ( bAgent && !dFrontend.Contains ( bind ( &CSphColumnInfo::m_sName ), tCol.m_sName ) )
			{
				CSphColumnInfo & t = dFrontend.Add();
				t.m_iIndex = iCol;
				t.m_sName = tCol.m_sName;
			}
		} else if ( bMagic && ( tCol.m_pExpr.Ptr() || bUsualApi ) )
		{
			ARRAY_FOREACH ( j, dUnmappedItems )
				if ( tCol.m_sName==GetMagicSchemaName ( tItems[ dUnmappedItems[j] ].m_sExpr ) )
			{
				int k = dUnmappedItems[j];
				dFrontend[k].m_iIndex = iCol;
				dFrontend[k].m_sName = tItems[k].m_sAlias;
				dKnownItems.Add(k);
				dUnmappedItems.Remove ( j-- ); // do not skip an element next to removed one!
			}
			if ( !dFrontend.Contains ( bind ( &CSphColumnInfo::m_sName ), tCol.m_sName ) )
			{
				CSphColumnInfo & t = dFrontend.Add();
				t.m_iIndex = iCol;
				t.m_sName = tCol.m_sName;
			}
		} else
		{
			bool bAdded = false;
			ARRAY_FOREACH ( j, dUnmappedItems )
			{
				int k = dUnmappedItems[j];
				const CSphQueryItem & t = tItems[k];
				if ( ( tCol.m_sName==GetMagicSchemaName ( t.m_sExpr ) && t.m_eAggrFunc==SPH_AGGR_NONE )
						|| ( t.m_sAlias==tCol.m_sName &&
							( tRes.m_tSchema.GetAttrIndex ( GetMagicSchemaName ( t.m_sExpr ) )==-1 || t.m_eAggrFunc!=SPH_AGGR_NONE ) ) )
				{
					// tricky bit about naming
					//
					// in master mode, we can just use the alias or expression or whatever
					// the data will be fetched using the locator anyway, column name does not matter anymore
					//
					// in agent mode, however, we need to keep the original column names in our response
					// otherwise, queries like SELECT col1 c, count(*) c FROM dist will fail on master
					// because it won't be able to identify the count(*) aggregate by its name
					dFrontend[k].m_iIndex = iCol;
					dFrontend[k].m_sName = bAgent
						? tCol.m_sName
						: ( tItems[k].m_sAlias.IsEmpty()
							? tItems[k].m_sExpr
							: tItems[k].m_sAlias );
					dKnownItems.Add(k);
					bAdded = true;
					dUnmappedItems.Remove ( j-- ); // do not skip an element next to removed one!
				}
			}

			// column was not found in the select list directly
			// however we might need it anyway because of a non-NULL extra-schema
			// ??? explain extra-schemas here
			// bMagic condition added for @groupbystr in the agent mode
			if ( !bAdded && pExtraSchema && ( pExtraSchema->GetAttrIndex ( tCol.m_sName.cstr() )>=0 || iLocals==0 || bMagic ) )
			{
				CSphColumnInfo & t = dFrontend.Add();
				t.m_iIndex = iCol;
				t.m_sName = tCol.m_sName;
			}
		}
	}

	// sanity check
	// verify that we actually have all the queried select items
	assert ( !dUnmappedItems.GetLength() || ( dUnmappedItems.GetLength()==1 && tItems [ dUnmappedItems[0] ].m_sExpr=="id" ) );
	dKnownItems.Sort();
	ARRAY_FOREACH ( i, tItems )
		if ( !dKnownItems.BinarySearch(i) && tItems[i].m_sExpr!="id" )
	{
		tRes.m_sError.SetSprintf ( "INTERNAL ERROR: column '%s/%s' not found in result set schema",
			tItems[i].m_sExpr.cstr(), tItems[i].m_sAlias.cstr() );
		return false;
	}

	// finalize the frontend schema columns
	// we kept indexes into internal schema there, now use them to lookup and copy column data
	ARRAY_FOREACH ( i, dFrontend )
	{
		CSphColumnInfo & d = dFrontend[i];
		if ( d.m_iIndex<0 && tItems[i].m_sExpr=="id" )
		{
			// handle one single exception to select-list-to-minimized-schema mapping loop above
			// "id" is not a part of a schema, so we gotta handle it here
			d.m_tLocator.m_bDynamic = true;
			d.m_sName = tItems[i].m_sAlias.IsEmpty() ? "id" : tItems[i].m_sAlias;
			d.m_eAttrType = USE_64BIT ? SPH_ATTR_BIGINT : SPH_ATTR_INTEGER;
			d.m_tLocator.m_iBitOffset = -8*(int)sizeof(SphDocID_t); // FIXME? move to locator method?
			d.m_tLocator.m_iBitCount = 8*sizeof(SphDocID_t);
		} else
		{
			// everything else MUST have been mapped in the loop just above
			// so use GetAttr(), and let it assert() at will
			const CSphColumnInfo & s = tRes.m_tSchema.GetAttr ( d.m_iIndex );
			d.m_tLocator = s.m_tLocator;
			d.m_eAttrType = s.m_eAttrType;
			d.m_eAggrFunc = s.m_eAggrFunc; // for a sort loop just below
		}
		d.m_iIndex = i; // to make the aggr sort loop just below stable
	}

	// tricky bit
	// in agents only, push aggregated columns, if any, to the end
	// for that, sort the schema by (is_aggregate ASC, column_index ASC)
	if ( bAgent )
		dFrontend.Sort ( AggregateColumnSort_fn() );

	// tricky bit
	// in purely distributed case, all schemas are received from the wire, and miss aggregate functions info
	// thus, we need to re-assign that info
	if ( !iLocals )
		RecoverAggregateFunctions ( tQuery, tRes );

	// if there's more than one result set,
	// we now have to merge and order all the matches
	// this is a good time to apply outer order clause, too
	if ( tRes.m_iSuccesses>1 || pAggrFilter )
	{
		ESphSortOrder eQuerySort = ( tQuery.m_sOuterOrderBy.IsEmpty() ? SPH_SORT_RELEVANCE : SPH_SORT_EXTENDED );
		// got outer order? gotta do a couple things
		if ( tQuery.m_bHasOuter )
		{
			// first, temporarily patch up sorting clause and max_matches (we will restore them later)
			Swap ( tQuery.m_sOuterOrderBy, tQuery.m_sGroupBy.IsEmpty() ? tQuery.m_sSortBy : tQuery.m_sGroupSortBy );
			Swap ( eQuerySort, tQuery.m_eSort );
			tQuery.m_iMaxMatches *= tRes.m_dMatchCounts.GetLength();
			// FIXME? probably not right; 20 shards with by 300 matches might be too much
			// but propagating too small inner max_matches to the outer is not right either

			// second, apply inner limit now, before (!) reordering
			int iOut = 0;
			int iSetStart = 0;
			ARRAY_FOREACH ( iSet, tRes.m_dMatchCounts )
			{
				assert ( tQuery.m_iLimit>=0 );
				int iOldOut = iOut;
				int iStart = iSetStart;
				int iSetEnd = iSetStart + tRes.m_dMatchCounts[iSet];
				int iEnd = Min ( iStart + tQuery.m_iLimit, iSetEnd );
				iStart = Min ( iStart, iEnd );
				for ( int i=iStart; i<iEnd; i++ )
					Swap ( tRes.m_dMatches[iOut++], tRes.m_dMatches[i] );
				iSetStart = iSetEnd;
				tRes.m_dMatchCounts[iSet] = iOut - iOldOut;
			}
			tRes.ClampMatches ( iOut, bAllEqual ); // false means no common schema; true == use common schema
		}

		// so we need to bring matches to the schema that the *sorter* wants
		// so we need to create the sorter before conversion
		//
		// create queue
		// at this point, we do not need to compute anything; it all must be here
		SphQueueSettings_t tQueueSettings ( tQuery, tRes.m_tSchema, tRes.m_sError, NULL );
		tQueueSettings.m_bComputeItems = false;
		tQueueSettings.m_pAggrFilter = pAggrFilter;
		ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings );

		// restore outer order related patches, or it screws up the query log
		if ( tQuery.m_bHasOuter )
		{
			Swap ( tQuery.m_sOuterOrderBy, tQuery.m_sGroupBy.IsEmpty() ? tQuery.m_sSortBy : tQuery.m_sGroupSortBy );
			Swap ( eQuerySort, tQuery.m_eSort );
			tQuery.m_iMaxMatches /= tRes.m_dMatchCounts.GetLength();
		}

		if ( !pSorter )
			return false;

		// reset bAllEqual flag if sorter makes new attributes
		if ( bAllEqual )
		{
			// at first we count already existed internal attributes
			// then check if sorter makes more
			CSphVector<SphStringSorterRemap_t> dRemapAttr;
			sphSortGetStringRemap ( tRes.m_tSchema, tRes.m_tSchema, dRemapAttr );
			int iRemapCount = dRemapAttr.GetLength();
			sphSortGetStringRemap ( pSorter->GetSchema(), tRes.m_tSchema, dRemapAttr );

			bAllEqual = ( dRemapAttr.GetLength()<=iRemapCount );
		}

		// sorter expects this
		tRes.m_tSchema = pSorter->GetSchema();

		// convert all matches to sorter schema - at least to manage all static to dynamic
		if ( !bAllEqual )
		{
			// post-limit stuff first
			if ( iLocals )
			{
				CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_EVAL_POST );
				ProcessLocalPostlimit ( tQuery, tRes );
			}

			RemapResult ( &tRes.m_tSchema, &tRes );
		}
		RemapStrings ( pSorter, tRes );

		// do the sort work!
		tRes.m_iTotalMatches -= KillAllDupes ( pSorter, tRes );
	}

	// apply outer order clause to single result set
	// (multiple combined sets just got reordered above)
	// apply inner limit first
	if ( tRes.m_iSuccesses==1 && tQuery.m_bHasOuter )
		tRes.ClampMatches ( tQuery.m_iLimit, bAllEqual );

	if ( tRes.m_iSuccesses==1 && tQuery.m_bHasOuter && !tQuery.m_sOuterOrderBy.IsEmpty() )
	{
		// reorder (aka outer order)
		ESphSortFunc eFunc;
		GenericMatchSort_fn tReorder;

		ESortClauseParseResult eRes = sphParseSortClause ( &tQuery, tQuery.m_sOuterOrderBy.cstr(),
			tRes.m_tSchema, eFunc, tReorder, tRes.m_sError );
		if ( eRes==SORT_CLAUSE_RANDOM )
			tRes.m_sError = "order by rand() not supported in outer select";
		if ( eRes!=SORT_CLAUSE_OK )
			return false;

		assert ( eFunc==FUNC_GENERIC2 || eFunc==FUNC_GENERIC3 || eFunc==FUNC_GENERIC4 || eFunc==FUNC_GENERIC5 );
		sphSort ( tRes.m_dMatches.Begin(), tRes.m_dMatches.GetLength(), tReorder, MatchSortAccessor_t() );
	}

	// compute post-limit stuff
	if ( bAllEqual && iLocals )
	{
		CSphScopedProfile ( pProfiler, SPH_QSTATE_EVAL_POST );

		CSphVector<const CSphColumnInfo *> dPostlimit;
		ExtractPostlimit ( tRes.m_tSchema, dPostlimit );

		// post compute matches only between offset - limit
		// however at agent we can't estimate limit.offset at master merged result set
		// but master don't provide offset to agents only offset+limit as limit
		// so computing all matches up to iiner.limit \ outer.limit
		int iTo = tRes.m_dMatches.GetLength();
		int iOff = Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
		int iCount = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );
		iTo = Max ( Min ( iOff + iCount, iTo ), 0 );
		int iFrom = Min ( iOff, iTo );

		ProcessPostlimit ( dPostlimit, iFrom, iTo, tRes );
	}

	// remap groupby() and aliased groupby() to @groupbystr or string attribute
	const CSphColumnInfo * p = tRes.m_tSchema.GetAttr ( "@groupbystr" );
	if ( !p )
	{
		// try string attribute (multiple group-by still displays hashes)
		if ( !tQuery.m_sGroupBy.IsEmpty() )
		{
			p = tRes.m_tSchema.GetAttr ( tQuery.m_sGroupBy.cstr() );
			if ( p && ( p->m_eAttrType!=SPH_ATTR_STRING && p->m_eAttrType!=SPH_ATTR_STRINGPTR ) )
				p = NULL;
		}
	}
	if ( p )
	{
		ARRAY_FOREACH ( i, dFrontend )
		{
			CSphColumnInfo & d = dFrontend[i];
			if ( d.m_sName=="groupby()" )
			{
				d.m_tLocator = p->m_tLocator;
				d.m_eAttrType = p->m_eAttrType;
				d.m_eAggrFunc = p->m_eAggrFunc;
			}
		}

		// check aliases too
		ARRAY_FOREACH ( j, tQuery.m_dItems )
		{
			const CSphQueryItem & tItem = tQuery.m_dItems[j];
			if ( tItem.m_sExpr=="groupby()" )
			{
				ARRAY_FOREACH ( i, dFrontend )
				{
					CSphColumnInfo & d = dFrontend[i];
					if ( d.m_sName==tItem.m_sAlias )
					{
						d.m_tLocator = p->m_tLocator;
						d.m_eAttrType = p->m_eAttrType;
						d.m_eAggrFunc = p->m_eAggrFunc;
					}
				}
			}
		}
	}

	// facets
	if ( tQuery.m_bFacet )
	{
		// remap MVA/JSON column to @groupby/@groupbystr in facet queries
		const CSphColumnInfo * p = tRes.m_tSchema.GetAttr ( "@groupbystr" );
		if ( !p )
			p = tRes.m_tSchema.GetAttr ( "@groupby" );
		if ( p )
		{
			ARRAY_FOREACH ( i, dFrontend )
			{
				CSphColumnInfo & d = dFrontend[i];
				if ( tQuery.m_sGroupBy==d.m_sName &&
					( d.m_eAttrType==SPH_ATTR_INT64SET
					|| d.m_eAttrType==SPH_ATTR_UINT32SET
					|| d.m_eAttrType==SPH_ATTR_JSON_FIELD ) )
				{
					d.m_tLocator = p->m_tLocator;
					d.m_eAttrType = p->m_eAttrType;
					d.m_eAggrFunc = p->m_eAggrFunc;
				}
			}
		}

		// skip all fields after count(*)
		ARRAY_FOREACH ( i, dFrontend )
			if ( dFrontend[i].m_sName=="count(*)" )
			{
				dFrontend.Resize ( i+1 );
				break;
			}
	}

	// all the merging and sorting is now done
	// replace the minimized matches schema with its subset, the result set schema
	tRes.m_tSchema.SwapAttrs ( dFrontend );
	return true;
}

/////////////////////////////////////////////////////////////////////////////

class CSphSchemaMT : public CSphSchema
{
public:
	explicit				CSphSchemaMT ( const char * sName="(nameless)" ) : CSphSchema ( sName ), m_pLock ( NULL )
	{}

	void AwareMT()
	{
		if ( m_pLock )
			return;
		m_pLock = new CSphRwlock();
		m_pLock->Init();
	}

	~CSphSchemaMT()
	{
		if ( m_pLock )
			Verify ( m_pLock->Done() );
		SafeDelete ( m_pLock )
	}

	// get wlocked entry, only if it is not yet touched
	inline CSphSchemaMT * GetVirgin ()
	{
		if ( !m_pLock )
			return this;

		if ( m_pLock->WriteLock() )
		{
			if ( m_dAttrs.GetLength()!=0 ) // not already a virgin
			{
				m_pLock->Unlock();
				return NULL;
			}
			return this;
		} else
		{
			sphLogDebug ( "WriteLock %p failed", this );
			assert ( false );
		}

		return NULL;
	}

	inline CSphSchemaMT * RLock()
	{
		if ( !m_pLock )
			return this;

		if ( !m_pLock->ReadLock() )
		{
			sphLogDebug ( "ReadLock %p failed", this );
			assert ( false );
		}
		return this;
	}

	inline void UnLock() const
	{
		if ( m_pLock )
			m_pLock->Unlock();
	}

private:
	mutable CSphRwlock * m_pLock;
};

class UnlockOnDestroy
{
public:
	explicit UnlockOnDestroy ( const CSphSchemaMT * lock ) : m_pLock ( lock )
	{}
	inline ~UnlockOnDestroy()
	{
		if ( m_pLock )
			m_pLock->UnLock();
	}
private:
	const CSphSchemaMT * m_pLock;
};


static int StringBinary2Number ( const char * sStr, int iLen )
{
	if ( !sStr || !iLen )
		return 0;

	char sBuf[64];
	if ( (int)(sizeof ( sBuf )-1 )<iLen )
		iLen = sizeof ( sBuf )-1;
	memcpy ( sBuf, sStr, iLen );
	sBuf[iLen] = '\0';

	return atoi ( sBuf );
}

static bool SnippetTransformPassageMacros ( CSphString & sSrc, CSphString & sPost )
{
	const char sPassageMacro[] = "%PASSAGE_ID%";

	const char * sPass = NULL;
	if ( !sSrc.IsEmpty() )
		sPass = strstr ( sSrc.cstr(), sPassageMacro );

	if ( !sPass )
		return false;

	int iSrcLen = sSrc.Length();
	int iPassLen = sizeof ( sPassageMacro ) - 1;
	int iTailLen = iSrcLen - iPassLen - ( sPass - sSrc.cstr() );

	// copy tail
	if ( iTailLen )
		sPost.SetBinary ( sPass+iPassLen, iTailLen );

	CSphString sPre;
	sPre.SetBinary ( sSrc.cstr(), sPass - sSrc.cstr() );
	sSrc.Swap ( sPre );

	return true;
}


/// suddenly, searchd-level expression function!
struct Expr_Snippet_c : public ISphStringExpr
{
	ISphExpr *					m_pArgs;
	ISphExpr *					m_pText;
	const BYTE *				m_sWords;
	CSphIndex *					m_pIndex;
	SnippetContext_t			m_tCtx;
	mutable ExcerptQuery_t		m_tHighlight;
	CSphQueryProfile *			m_pProfiler;

	explicit Expr_Snippet_c ( ISphExpr * pArglist, CSphIndex * pIndex, CSphQueryProfile * pProfiler, CSphString & sError )
		: m_pArgs ( pArglist )
		, m_pText ( NULL )
		, m_sWords ( NULL )
		, m_pIndex ( pIndex )
		, m_pProfiler ( pProfiler )
	{
		assert ( pArglist->IsArglist() );
		m_pText = pArglist->GetArg(0);

		CSphMatch tDummy;
		char * pWords;
		assert ( !pArglist->GetArg(1)->IsStringPtr() ); // aware of memleaks potentially caused by StringEval()
		pArglist->GetArg(1)->StringEval ( tDummy, (const BYTE**)&pWords );
		m_tHighlight.m_sWords = pWords;

		for ( int i = 2; i < pArglist->GetNumArgs(); i++ )
		{
			assert ( !pArglist->GetArg(i)->IsStringPtr() ); // aware of memleaks potentially caused by StringEval()
			int iLen = pArglist->GetArg(i)->StringEval ( tDummy, (const BYTE**)&pWords );
			if ( !pWords || !iLen )
				continue;

			const char * sEnd = pWords + iLen;
			while ( pWords<sEnd && *pWords && sphIsSpace ( *pWords ) )	pWords++;
			char * szOption = pWords;
			while ( pWords<sEnd && *pWords && sphIsAlpha ( *pWords ) )	pWords++;
			char * szOptEnd = pWords;
			while ( pWords<sEnd && *pWords && sphIsSpace ( *pWords ) )	pWords++;

			if ( *pWords++!='=' )
			{
				sError.SetSprintf ( "Error parsing SNIPPET options: %s", pWords );
				return;
			}

			*szOptEnd = '\0';
			while ( pWords<sEnd && *pWords && sphIsSpace ( *pWords ) )	pWords++;
			char * sValue = pWords;

			if ( !*sValue )
			{
				sError.SetSprintf ( "Error parsing SNIPPET options" );
				return;
			}

			while ( pWords<sEnd && *pWords ) pWords++;
			int iStrValLen = pWords - sValue;

			if ( !strcasecmp ( szOption, "before_match" ) )					{ m_tHighlight.m_sBeforeMatch.SetBinary ( sValue, iStrValLen ); }
			else if ( !strcasecmp ( szOption, "after_match" ) )				{ m_tHighlight.m_sAfterMatch.SetBinary ( sValue, iStrValLen ); }
			else if ( !strcasecmp ( szOption, "chunk_separator" ) )			{ m_tHighlight.m_sChunkSeparator.SetBinary ( sValue, iStrValLen ); }
			else if ( !strcasecmp ( szOption, "limit" ) )					{ m_tHighlight.m_iLimit = StringBinary2Number ( sValue, iStrValLen ); }
			else if ( !strcasecmp ( szOption, "around" ) )					{ m_tHighlight.m_iAround = StringBinary2Number ( sValue, iStrValLen ); }
			else if ( !strcasecmp ( szOption, "exact_phrase" ) )			{ m_tHighlight.m_bExactPhrase = ( StringBinary2Number ( sValue, iStrValLen )!=0 ); }
			else if ( !strcasecmp ( szOption, "use_boundaries" ) )			{ m_tHighlight.m_bUseBoundaries = ( StringBinary2Number ( sValue, iStrValLen )!=0 ); }
			else if ( !strcasecmp ( szOption, "weight_order" ) )			{ m_tHighlight.m_bWeightOrder = ( StringBinary2Number ( sValue, iStrValLen )!=0 ); }
			else if ( !strcasecmp ( szOption, "query_mode" ) )				{ m_tHighlight.m_bHighlightQuery = ( StringBinary2Number ( sValue, iStrValLen )!=0 ); }
			else if ( !strcasecmp ( szOption, "force_all_words" ) )			{ m_tHighlight.m_bForceAllWords = ( StringBinary2Number ( sValue, iStrValLen )!=0 ); }
			else if ( !strcasecmp ( szOption, "limit_passages" ) )			{ m_tHighlight.m_iLimitPassages = StringBinary2Number ( sValue, iStrValLen ); }
			else if ( !strcasecmp ( szOption, "limit_words" ) )				{ m_tHighlight.m_iLimitWords = StringBinary2Number ( sValue, iStrValLen ); }
			else if ( !strcasecmp ( szOption, "start_passage_id" ) )		{ m_tHighlight.m_iPassageId = StringBinary2Number ( sValue, iStrValLen ); }
			else if ( !strcasecmp ( szOption, "load_files" ) )				{ m_tHighlight.m_iLoadFiles |= ( StringBinary2Number ( sValue, iStrValLen )!=0 ? 1 : 0 ); }
			else if ( !strcasecmp ( szOption, "load_files_scattered" ) )	{ m_tHighlight.m_iLoadFiles |= ( StringBinary2Number ( sValue, iStrValLen )!=0 ? 2 : 0 ); }
			else if ( !strcasecmp ( szOption, "html_strip_mode" ) )			{ m_tHighlight.m_sStripMode.SetBinary ( sValue, iStrValLen ); }
			else if ( !strcasecmp ( szOption, "allow_empty" ) )				{ m_tHighlight.m_bAllowEmpty= ( StringBinary2Number ( sValue, iStrValLen )!=0 ); }
			else if ( !strcasecmp ( szOption, "passage_boundary" ) )		{ m_tHighlight.m_sRawPassageBoundary.SetBinary ( sValue, iStrValLen ); }
			else if ( !strcasecmp ( szOption, "emit_zones" ) )				{ m_tHighlight.m_bEmitZones = ( StringBinary2Number ( sValue, iStrValLen )!=0 ); }
			else
			{
				CSphString sBuf;
				sBuf.SetBinary ( sValue, iStrValLen );
				sError.SetSprintf ( "Unknown SNIPPET option: %s=%s", szOption, sBuf.cstr() );
				return;
			}
		}

		m_tHighlight.m_bHasBeforePassageMacro = SnippetTransformPassageMacros ( m_tHighlight.m_sBeforeMatch, m_tHighlight.m_sBeforeMatchPassage );
		m_tHighlight.m_bHasAfterPassageMacro = SnippetTransformPassageMacros ( m_tHighlight.m_sAfterMatch, m_tHighlight.m_sAfterMatchPassage );

		m_tCtx.Setup ( m_pIndex, m_tHighlight, sError );
	}

	~Expr_Snippet_c()
	{
		SafeRelease ( m_pArgs );
	}

	virtual int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
	{
		CSphScopedProfile ( m_pProfiler, SPH_QSTATE_SNIPPET );

		*ppStr = NULL;

		const BYTE * sSource = NULL;
		int iLen = m_pText->StringEval ( tMatch, &sSource );

		if ( !iLen )
		{
			if ( m_pText->IsStringPtr() )
				SafeDeleteArray ( sSource );
			return 0;
		}

		// for dynamic strings (eg. fetched by UDFs), just take ownership
		// for static ones (eg. attributes), treat as binary (ie. mind that
		// the trailing zero is NOT guaranteed), and copy them
		if ( m_pText->IsStringPtr() )
			m_tHighlight.m_sSource.Adopt ( (char**)&sSource );
		else
			m_tHighlight.m_sSource.SetBinary ( (const char*)sSource, iLen );

		// FIXME! fill in all the missing options; use consthash?
		CSphString sWarning, sError;
		sphBuildExcerpt ( m_tHighlight, m_pIndex, m_tCtx.m_tStripper.Ptr(), m_tCtx.m_tExtQuery, m_tCtx.m_eExtQuerySPZ,
			sWarning, sError, m_tCtx.m_pDict, m_tCtx.m_tTokenizer.Ptr(), m_tCtx.m_pQueryTokenizer );

		int iRes = m_tHighlight.m_dRes.GetLength();
		*ppStr = m_tHighlight.m_dRes.LeakData();
		return iRes;
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		if ( eCmd!=SPH_EXPR_SET_STRING_POOL )
			return;

		if ( m_pArgs )
			m_pArgs->Command ( SPH_EXPR_SET_STRING_POOL, pArg );
		if ( m_pText )
			m_pText->Command ( SPH_EXPR_SET_STRING_POOL, pArg );
	}
};


/// searchd expression hook
/// needed to implement functions that are builtin for searchd,
/// but can not be builtin in the generic expression engine itself,
/// like SNIPPET() function that must know about indexes, tokenizers, etc
struct ExprHook_t : public ISphExprHook
{
	static const int HOOK_SNIPPET = 1;
	CSphIndex * m_pIndex; /// BLOODY HACK
	CSphQueryProfile * m_pProfiler;

	ExprHook_t ()
		: m_pIndex ( NULL )
		, m_pProfiler ( NULL )
	{}

	virtual int IsKnownIdent ( const char * )
	{
		return -1;
	}

	virtual int IsKnownFunc ( const char * sFunc )
	{
		if ( !strcasecmp ( sFunc, "SNIPPET" ) )
			return HOOK_SNIPPET;
		else
			return -1;
	}

	virtual ISphExpr * CreateNode ( int DEBUGARG(iID), ISphExpr * pLeft, ESphEvalStage * pEvalStage, CSphString & sError )
	{
		assert ( iID==HOOK_SNIPPET );
		if ( pEvalStage )
			*pEvalStage = SPH_EVAL_POSTLIMIT;

		ISphExpr * pRes = new Expr_Snippet_c ( pLeft, m_pIndex, m_pProfiler, sError );
		if ( sError.Length() )
			SafeDelete ( pRes );

		return pRes;
	}

	virtual ESphAttr GetIdentType ( int )
	{
		assert ( 0 );
		return SPH_ATTR_NONE;
	}

	virtual ESphAttr GetReturnType ( int DEBUGARG(iID), const CSphVector<ESphAttr> & dArgs, bool, CSphString & sError )
	{
		assert ( iID==HOOK_SNIPPET );
		if ( dArgs.GetLength()<2 )
		{
			sError = "SNIPPET() requires 2 or more arguments";
			return SPH_ATTR_NONE;
		}
		if ( dArgs[0]!=SPH_ATTR_STRINGPTR && dArgs[0]!=SPH_ATTR_STRING )
		{
			sError = "1st argument to SNIPPET() must be a string expression";
			return SPH_ATTR_NONE;
		}

		for ( int i = 1; i < dArgs.GetLength(); i++ )
			if ( dArgs[i]!=SPH_ATTR_STRING )
			{
				sError.SetSprintf ( "%d argument to SNIPPET() must be a constant string", i );
				return SPH_ATTR_NONE;
			}

		return SPH_ATTR_STRINGPTR;
	}

	virtual void CheckEnter ( int ) {}
	virtual void CheckExit ( int ) {}
};

struct LocalIndex_t
{
	CSphString	m_sName;
	int			m_iOrderTag;
	int			m_iWeight;
	int64_t		m_iMass;
	LocalIndex_t ()
		: m_iOrderTag ( 0 )
		, m_iWeight ( 1 )
	{ }
};


class SearchHandler_c : public ISphSearchHandler
{
	friend void LocalSearchThreadFunc ( void * pArg );

public:
									SearchHandler_c ( int iQueries, bool bSphinxql, bool bMaster, int iCID );
	virtual							~SearchHandler_c();
	virtual void					RunQueries ();					///< run all queries, get all results
	void							RunUpdates ( const CSphQuery & tQuery, const CSphString & sIndex, CSphAttrUpdateEx * pUpdates ); ///< run Update command instead of Search
	void							RunDeletes ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors, CSphVector<SphDocID_t>* ); ///< run Delete command instead of Search

	virtual CSphQuery *				GetQuery ( int iQuery ) { return m_dQueries.Begin() + iQuery; }
	virtual AggrResult_t *			GetResult ( int iResult ) { return m_dResults.Begin() + iResult; }

public:
	CSphVector<CSphQuery>			m_dQueries;						///< queries which i need to search
	CSphVector<AggrResult_t>		m_dResults;						///< results which i obtained
	CSphVector<SearchFailuresLog_c>	m_dFailuresSet;					///< failure logs for each query
	CSphVector < CSphVector<int64_t> >	m_dAgentTimes;				///< per-agent time stats
	CSphQueryProfile *				m_pProfile;
	CSphVector<DWORD *>				m_dMva2Free;
	CSphVector<BYTE *>				m_dString2Free;
	int								m_iCid;

protected:
	void							RunSubset ( int iStart, int iEnd );	///< run queries against index(es) from first query in the subset
	void							RunLocalSearches ( ISphMatchSorter * pLocalSorter, const char * sDistName, DWORD uFactorFlags );
	void							RunLocalSearchesMT ();
	bool							RunLocalSearch ( int iLocal, ISphMatchSorter ** ppSorters, CSphQueryResult ** pResults, bool * pMulti ) const;
	bool							AllowsMulti ( int iStart, int iEnd ) const;
	void							SetupLocalDF ( int iStart, int iEnd );

	int								m_iStart;		///< subset start
	int								m_iEnd;			///< subset end
	bool							m_bMultiQueue;	///< whether current subset is subject to multi-queue optimization
	bool							m_bFacetQueue;	///< whether current subset is subject to facet-queue optimization
	CSphVector<LocalIndex_t>		m_dLocal;		///< local indexes for the current subset
	mutable CSphVector<CSphSchemaMT>		m_dExtraSchemas; ///< the extra fields for agents
	bool							m_bSphinxql;	///< if the query get from sphinxql - to avoid applying sphinxql magick for others
	CSphAttrUpdateEx *				m_pUpdates;		///< holder for updates
	CSphVector<SphDocID_t> *		m_pDelete;		///< this query is for deleting

	mutable CSphMutex				m_tLock;
	mutable SmallStringHash_T<int>	m_hUsed;

	mutable ExprHook_t				m_tHook;

	SmallStringHash_T < int64_t >	m_hLocalDocs;
	int64_t							m_iTotalDocs;
	bool							m_bGotLocalDF;
	bool							m_bMaster;

	const ServedIndex_c *			UseIndex ( int iLocal ) const;
	void							ReleaseIndex ( int iLocal ) const;

	void							OnRunFinished ();
};


ISphSearchHandler * sphCreateSearchHandler ( int iQueries, bool bSphinxql, bool bMaster, int iCID )
{
	return new SearchHandler_c ( iQueries, bSphinxql, bMaster, iCID );
}


SearchHandler_c::SearchHandler_c ( int iQueries, bool bSphinxql, bool bMaster, int iCid )
{
	m_iStart = 0;
	m_iEnd = 0;
	m_bMultiQueue = false;
	m_bFacetQueue = false;

	m_dQueries.Resize ( iQueries );
	m_dResults.Resize ( iQueries );
	m_dFailuresSet.Resize ( iQueries );
	m_dExtraSchemas.Resize ( iQueries );
	m_dAgentTimes.Resize ( iQueries );
	m_bSphinxql = bSphinxql;
	m_pUpdates = NULL;
	m_pDelete = NULL;

	m_pProfile = NULL;
	m_tHook.m_pProfiler = NULL;
	m_iTotalDocs = 0;
	m_bGotLocalDF = false;
	m_bMaster = bMaster;
	m_iCid = iCid;
}


SearchHandler_c::~SearchHandler_c ()
{
	m_hUsed.IterateStart();
	while ( m_hUsed.IterateNext() )
	{
		if ( m_hUsed.IterateGet()>0 )
			g_pLocalIndexes->GetUnlockedEntry ( m_hUsed.IterateGetKey() ).Unlock();
	}

	ARRAY_FOREACH ( i, m_dMva2Free )
		SafeDeleteArray ( m_dMva2Free[i] );
	ARRAY_FOREACH ( i, m_dString2Free )
		SafeDeleteArray ( m_dString2Free[i] );
}


const ServedIndex_c * SearchHandler_c::UseIndex ( int iLocal ) const
{
	assert ( iLocal>=0 && iLocal<m_dLocal.GetLength() );
	const CSphString & sName = m_dLocal[iLocal].m_sName;

	m_tLock.Lock();
	int * pUseCount = m_hUsed ( sName );
	assert ( ( m_pUpdates && pUseCount && *pUseCount>0 ) || !m_pUpdates );

	const ServedIndex_c * pServed = NULL;
	if ( pUseCount && *pUseCount>0 )
	{
		pServed = &g_pLocalIndexes->GetUnlockedEntry ( sName );
		*pUseCount += 1;
	} else
	{
		pServed = g_pLocalIndexes->GetRlockedEntry ( sName );
		if ( pServed )
		{
			if ( pUseCount )
				(*pUseCount)++;
			else
				m_hUsed.Add ( 1, sName );
		}
	}

	m_tLock.Unlock();
	return pServed;
}


void SearchHandler_c::ReleaseIndex ( int iLocal ) const
{
	assert ( iLocal>=0 && iLocal<m_dLocal.GetLength() );

	const CSphString & sName = m_dLocal[iLocal].m_sName;
	m_tLock.Lock();

	int * pUseCount = m_hUsed ( sName );
	assert ( pUseCount && *pUseCount>=0 );
	(*pUseCount)--;

	if ( !*pUseCount )
		g_pLocalIndexes->GetUnlockedEntry ( sName ).Unlock();

	assert ( ( m_pUpdates && pUseCount && *pUseCount ) || !m_pUpdates );

	m_tLock.Unlock();
}

void SearchHandler_c::RunUpdates ( const CSphQuery & tQuery, const CSphString & sIndex, CSphAttrUpdateEx * pUpdates )
{
	m_pUpdates = pUpdates;

	m_dQueries[0] = tQuery;
	m_dQueries[0].m_sIndexes = sIndex;

	// lets add index to prevent deadlock
	// as index already r-locker or w-locked at this point
	m_dLocal.Add().m_sName = sIndex;
	m_hUsed.Add ( 1, sIndex );
	m_dResults[0].m_dTag2Pools.Resize ( 1 );

	CheckQuery ( tQuery, *pUpdates->m_pError );
	if ( !pUpdates->m_pError->IsEmpty() )
		return;

	int64_t tmLocal = -sphMicroTimer();

	RunLocalSearches ( NULL, NULL, SPH_FACTOR_DISABLE );
	tmLocal += sphMicroTimer();

	OnRunFinished();

	CSphQueryResult & tRes = m_dResults[0];

	tRes.m_iOffset = tQuery.m_iOffset;
	tRes.m_iCount = Max ( Min ( tQuery.m_iLimit, tRes.m_dMatches.GetLength()-tQuery.m_iOffset ), 0 );

	tRes.m_iQueryTime += (int)(tmLocal/1000);
	tRes.m_iCpuTime += tmLocal;

	if ( !tRes.m_iSuccesses )
	{
		CSphStringBuilder sFailures;
		m_dFailuresSet[0].BuildReport ( sFailures );
		*pUpdates->m_pError = sFailures.cstr();

	} else if ( !tRes.m_sError.IsEmpty() )
	{
		CSphStringBuilder sFailures;
		m_dFailuresSet[0].BuildReport ( sFailures );
		tRes.m_sWarning = sFailures.cstr(); // FIXME!!! commit warnings too
	}

	const CSphIOStats & tIO = tRes.m_tIOStats;

	g_tStatsMutex.Lock();
	g_tStats.m_iQueries += 1;
	g_tStats.m_iQueryTime += tmLocal;
	g_tStats.m_iQueryCpuTime += tmLocal;
	g_tStats.m_iDiskReads += tIO.m_iReadOps;
	g_tStats.m_iDiskReadTime += tIO.m_iReadTime;
	g_tStats.m_iDiskReadBytes += tIO.m_iReadBytes;
	g_tStatsMutex.Unlock();

	LogQuery ( m_dQueries[0], m_dResults[0], m_dAgentTimes[0], m_iCid );
}

void SearchHandler_c::RunDeletes ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors, CSphVector<SphDocID_t>* pValues )
{
	m_pDelete = pValues;
	m_dQueries[0] = tQuery;
	m_dQueries[0].m_sIndexes = sIndex;

	// lets add index to prevent deadlock
	// as index already w-locked at this point
	m_dLocal.Add().m_sName = sIndex;
	m_hUsed.Add ( 1, sIndex );
	m_dResults[0].m_dTag2Pools.Resize ( 1 );

	CheckQuery ( tQuery, *pErrors );
	if ( !pErrors->IsEmpty() )
		return;

	int64_t tmLocal = -sphMicroTimer();

	RunLocalSearches ( NULL, NULL, SPH_FACTOR_DISABLE );
	tmLocal += sphMicroTimer();

	OnRunFinished();

	CSphQueryResult & tRes = m_dResults[0];

	tRes.m_iOffset = tQuery.m_iOffset;
	tRes.m_iCount = Max ( Min ( tQuery.m_iLimit, tRes.m_dMatches.GetLength()-tQuery.m_iOffset ), 0 );

	tRes.m_iQueryTime += (int)(tmLocal/1000);
	tRes.m_iCpuTime += tmLocal;

	if ( !tRes.m_iSuccesses )
	{
		CSphStringBuilder sFailures;
		m_dFailuresSet[0].BuildReport ( sFailures );
		*pErrors = sFailures.cstr();

	} else if ( !tRes.m_sError.IsEmpty() )
	{
		CSphStringBuilder sFailures;
		m_dFailuresSet[0].BuildReport ( sFailures );
		tRes.m_sWarning = sFailures.cstr(); // FIXME!!! commit warnings too
	}

	const CSphIOStats & tIO = tRes.m_tIOStats;

	g_tStatsMutex.Lock();
	g_tStats.m_iQueries += 1;
	g_tStats.m_iQueryTime += tmLocal;
	g_tStats.m_iQueryCpuTime += tmLocal;
	g_tStats.m_iDiskReads += tIO.m_iReadOps;
	g_tStats.m_iDiskReadTime += tIO.m_iReadTime;
	g_tStats.m_iDiskReadBytes += tIO.m_iReadBytes;
	g_tStatsMutex.Unlock();

	LogQuery ( m_dQueries[0], m_dResults[0], m_dAgentTimes[0], m_iCid );
}

void SearchHandler_c::RunQueries()
{
	// check if all queries are to the same index
	bool bSameIndex = ARRAY_ALL ( bSameIndex, m_dQueries, m_dQueries[_all].m_sIndexes==m_dQueries[0].m_sIndexes );
	if ( bSameIndex )
	{
		// batch queries to same index
		RunSubset ( 0, m_dQueries.GetLength()-1 );
		ARRAY_FOREACH ( i, m_dQueries )
			LogQuery ( m_dQueries[i], m_dResults[i], m_dAgentTimes[i], m_iCid );
	} else
	{
		// fallback; just work each query separately
		ARRAY_FOREACH ( i, m_dQueries )
		{
			RunSubset ( i, i );
			LogQuery ( m_dQueries[i], m_dResults[i], m_dAgentTimes[i], m_iCid );
		}
	}
	OnRunFinished();
}


// final fixup
void SearchHandler_c::OnRunFinished()
{
	ARRAY_FOREACH ( i, m_dResults )
	{
		m_dResults[i].m_iMatches = m_dResults[i].m_dMatches.GetLength();
	}
}


/// return cpu time, in microseconds
int64_t sphCpuTimer ()
{
#ifdef HAVE_CLOCK_GETTIME
	if ( !g_bCpuStats )
		return 0;

#if defined (CLOCK_THREAD_CPUTIME_ID)
// CPU time (user+sys), Linux style, current thread
#define LOC_CLOCK CLOCK_THREAD_CPUTIME_ID
#elif defined(CLOCK_PROCESS_CPUTIME_ID)
// CPU time (user+sys), Linux style
#define LOC_CLOCK CLOCK_PROCESS_CPUTIME_ID
#elif defined(CLOCK_PROF)
// CPU time (user+sys), FreeBSD style
#define LOC_CLOCK CLOCK_PROF
#else
// POSIX fallback (wall time)
#define LOC_CLOCK CLOCK_REALTIME
#endif

	struct timespec tp;
	if ( clock_gettime ( LOC_CLOCK, &tp ) )
		return 0;

	return tp.tv_sec*1000000 + tp.tv_nsec/1000;
#else
	return 0;
#endif
}


struct LocalSearch_t
{
	int					m_iLocal;
	ISphMatchSorter **	m_ppSorters;
	CSphQueryResult **	m_ppResults;
	bool				m_bResult;
	int64_t				m_iMass;
};


struct LocalSearchThreadContext_t
{
	SphThread_t					m_tThd;
	SearchHandler_c *			m_pHandler;
	CrashQuery_t				m_tCrashQuery;
	int							m_iSearches;
	LocalSearch_t *				m_pSearches;
	CSphAtomic *				m_pCurSearch;

	LocalSearchThreadContext_t()
		: m_pHandler ( NULL )
		, m_iSearches ( 0 )
		, m_pSearches ( NULL )
		, m_pCurSearch ( NULL )
	{}
};


void LocalSearchThreadFunc ( void * pArg )
{
	LocalSearchThreadContext_t * pContext = (LocalSearchThreadContext_t*) pArg;

	SphCrashLogger_c::SetLastQuery ( pContext->m_tCrashQuery );

	for ( ;; )
	{
		if ( pContext->m_pCurSearch->GetValue()>=pContext->m_iSearches )
			return;

		long iCurSearch = pContext->m_pCurSearch->Inc();
		if ( iCurSearch>=pContext->m_iSearches )
			return;
		LocalSearch_t * pCall = pContext->m_pSearches + iCurSearch;
		pCall->m_bResult = pContext->m_pHandler->RunLocalSearch ( pCall->m_iLocal,
			pCall->m_ppSorters, pCall->m_ppResults, &pContext->m_pHandler->m_bMultiQueue );
	}
}


static void MergeWordStats ( CSphQueryResultMeta & tDstResult,
	const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hSrc,
	SearchFailuresLog_c * pLog, const char * sIndex )
{
	assert ( pLog );

	if ( !tDstResult.m_hWordStats.GetLength() )
	{
		// nothing has been set yet; just copy
		tDstResult.m_hWordStats = hSrc;
		return;
	}

	CSphString sDiff;
	SphWordStatChecker_t tDiff;
	tDiff.Set ( hSrc );
	tDiff.DumpDiffer ( tDstResult.m_hWordStats, NULL, sDiff );
	if ( !sDiff.IsEmpty() )
		pLog->SubmitEx ( sIndex, "%s", sDiff.cstr() );

	hSrc.IterateStart();
	while ( hSrc.IterateNext() )
	{
		const CSphQueryResultMeta::WordStat_t & tSrcStat = hSrc.IterateGet();
		tDstResult.AddStat ( hSrc.IterateGetKey(), tSrcStat.m_iDocs, tSrcStat.m_iHits );
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
		tRes.m_dSchemas.Add ( pSorter->GetSchema() );
		PoolPtrs_t & tPoolPtrs = tRes.m_dTag2Pools[iTag];
		assert ( !tPoolPtrs.m_pMva && !tPoolPtrs.m_pStrings );
		tPoolPtrs.m_pMva = tRes.m_pMva;
		tPoolPtrs.m_pStrings = tRes.m_pStrings;
		tPoolPtrs.m_bArenaProhibit = tRes.m_bArenaProhibit;
		int iCopied = sphFlattenQueue ( pSorter, &tRes, iTag );
		tRes.m_dMatchCounts.Add ( iCopied );

		// clean up for next index search
		tRes.m_pMva = NULL;
		tRes.m_pStrings = NULL;
		tRes.m_bArenaProhibit = false;
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
			tRes.m_tSchema.FreeStringPtrs ( pSrc );
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


void SearchHandler_c::RunLocalSearchesMT ()
{
	int64_t tmLocal = sphMicroTimer();

	// setup local searches
	const int iQueries = m_iEnd-m_iStart+1;
	CSphVector<LocalSearch_t> dWorks ( m_dLocal.GetLength() );
	CSphVector<CSphQueryResult> dResults ( m_dLocal.GetLength()*iQueries );
	CSphVector<ISphMatchSorter*> dSorters ( m_dLocal.GetLength()*iQueries );
	CSphVector<CSphQueryResult*> dResultPtrs ( m_dLocal.GetLength()*iQueries );

	ARRAY_FOREACH ( i, dResultPtrs )
		dResultPtrs[i] = &dResults[i];

	ARRAY_FOREACH ( i, m_dLocal )
	{
		dWorks[i].m_iLocal = i;
		dWorks[i].m_iMass = -m_dLocal[i].m_iMass; // minus for reverse order
		dWorks[i].m_ppSorters = &dSorters [ i*iQueries ];
		dWorks[i].m_ppResults = &dResultPtrs [ i*iQueries ];
	}
	dWorks.Sort ( bind ( &LocalSearch_t::m_iMass ) );

	// setup threads
	CSphVector<LocalSearchThreadContext_t> dThreads ( Min ( g_iDistThreads, dWorks.GetLength() ) );

	// prepare for multithread extra schema processing
	for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
		m_dExtraSchemas[iQuery].AwareMT();

	CrashQuery_t tCrashQuery = SphCrashLogger_c::GetQuery(); // transfer query info for crash logger to new thread
	// fire searcher threads

	CSphAtomic iaCursor;
	ARRAY_FOREACH ( i, dThreads )
	{
		LocalSearchThreadContext_t & t = dThreads[i];
		t.m_pHandler = this;
		t.m_tCrashQuery = tCrashQuery;
		t.m_pCurSearch = &iaCursor;
		t.m_iSearches = dWorks.GetLength();
		t.m_pSearches = dWorks.Begin();
		SphCrashLogger_c::ThreadCreate ( &dThreads[i].m_tThd, LocalSearchThreadFunc, (void*)&dThreads[i] ); // FIXME! check result
	}

	// wait for them to complete
	ARRAY_FOREACH ( i, dThreads )
		sphThreadJoin ( &dThreads[i].m_tThd );

	// now merge the results
	ARRAY_FOREACH ( iLocal, dWorks )
	{
		bool bResult = dWorks[iLocal].m_bResult;
		const char * sLocal = m_dLocal[iLocal].m_sName.cstr();
		int iOrderTag = m_dLocal[iLocal].m_iOrderTag;

		if ( !bResult )
		{
			// failed
			for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
			{
				int iResultIndex = iLocal*iQueries;
				if ( !m_bMultiQueue )
					iResultIndex += iQuery - m_iStart;
				m_dFailuresSet[iQuery].Submit ( sLocal, dResults[iResultIndex].m_sError.cstr() );
			}
			continue;
		}

		// multi-query succeeded
		for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
		{
			// base result set index
			// in multi-queue case, the only (!) result set actually filled with meta info
			// in non-multi-queue case, just a first index, we fix it below
			int iResultIndex = iLocal*iQueries;

			// current sorter ALWAYS resides at this index, in all cases
			// (current as in sorter for iQuery-th query against iLocal-th index)
			int iSorterIndex = iLocal*iQueries + iQuery - m_iStart;

			if ( !m_bMultiQueue )
			{
				// non-multi-queue case
				// means that we have mere 1:1 mapping between results and sorters
				// so let's adjust result set index
				iResultIndex = iSorterIndex;

			} else if ( dResults[iResultIndex].m_iMultiplier==-1 )
			{
				// multi-queue case
				// need to additionally check per-query failures of MultiQueryEx
				// those are reported through multiplier
				// note that iSorterIndex just below is NOT a typo
				// separate errors still go into separate result sets
				// even though regular meta does not
				m_dFailuresSet[iQuery].Submit ( sLocal, dResults[iSorterIndex].m_sError.cstr() );
				continue;
			}

			// no sorter, no fun
			ISphMatchSorter * pSorter = dSorters [ iSorterIndex ];
			if ( !pSorter )
				continue;

			// this one seems OK
			AggrResult_t & tRes = m_dResults[iQuery];
			CSphQueryResult & tRaw = dResults[iResultIndex];

			tRes.m_iSuccesses++;
			tRes.m_iTotalMatches += pSorter->GetTotalCount();

			tRes.m_pMva = tRaw.m_pMva;
			tRes.m_pStrings = tRaw.m_pStrings;
			tRes.m_bArenaProhibit = tRaw.m_bArenaProhibit;
			MergeWordStats ( tRes, tRaw.m_hWordStats, &m_dFailuresSet[iQuery], sLocal );

			// move external attributes storage from tRaw to actual result
			tRaw.LeakStorages ( tRes );

			tRes.m_bHasPrediction |= tRaw.m_bHasPrediction;
			tRes.m_iMultiplier = m_bMultiQueue ? iQueries : 1;
			tRes.m_iCpuTime += tRaw.m_iCpuTime / tRes.m_iMultiplier;
			tRes.m_tIOStats.Add ( tRaw.m_tIOStats );
			if ( tRaw.m_bHasPrediction )
			{
				tRes.m_tStats.Add ( tRaw.m_tStats );
				tRes.m_iPredictedTime = CalcPredictedTimeMsec ( tRes );
			}
			if ( tRaw.m_iBadRows )
				tRes.m_sWarning.SetSprintf ( "query result is inaccurate because of "INT64_FMT" missed documents", tRaw.m_iBadRows );

			// extract matches from sorter
			FlattenToRes ( pSorter, tRes, iOrderTag+iQuery-m_iStart );

			if ( tRaw.m_iBadRows )
				RemoveMissedRows ( tRes );

			// take over the schema from sorter, it doesn't need it anymore
			tRes.m_tSchema = pSorter->GetSchema(); // can SwapOut

			if ( !tRaw.m_sWarning.IsEmpty() )
				m_dFailuresSet[iQuery].Submit ( sLocal, tRaw.m_sWarning.cstr() );
		}
	}

	ARRAY_FOREACH ( i, dSorters )
		SafeDelete ( dSorters[i] );

	// update our wall time for every result set
	tmLocal = sphMicroTimer() - tmLocal;
	for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
	{
		m_dResults[iQuery].m_iQueryTime += (int)( tmLocal/1000 );
	}
}

int64_t sphCpuTimer();

// invoked from MT searches. So, must be MT-aware!
bool SearchHandler_c::RunLocalSearch ( int iLocal, ISphMatchSorter ** ppSorters, CSphQueryResult ** ppResults, bool * pMulti ) const
{
	int64_t iCpuTime = -sphCpuTimer();

	const int iQueries = m_iEnd-m_iStart+1;
	const ServedIndex_c * pServed = UseIndex ( iLocal );
	if ( !pServed || !pServed->m_bEnabled )
	{
		// FIXME! submit a failure?
		if ( pServed )
			ReleaseIndex ( iLocal );
		return false;
	}
	assert ( pServed->m_pIndex );
	assert ( pServed->m_bEnabled );
	assert ( pMulti );

	// create sorters
	int iValidSorters = 0;
	DWORD uFactorFlags = SPH_FACTOR_DISABLE;
	for ( int i=0; i<iQueries; i++ )
	{
		CSphString & sError = ppResults[i]->m_sError;
		const CSphQuery & tQuery = m_dQueries[i+m_iStart];
		CSphSchemaMT * pExtraSchemaMT = tQuery.m_bAgent?m_dExtraSchemas[i+m_iStart].GetVirgin():NULL;
		UnlockOnDestroy dSchemaLock ( pExtraSchemaMT );

		m_tHook.m_pIndex = pServed->m_pIndex;
		SphQueueSettings_t tQueueSettings ( tQuery, pServed->m_pIndex->GetMatchSchema(), sError, m_pProfile );
		tQueueSettings.m_bComputeItems = true;
		tQueueSettings.m_pExtra = pExtraSchemaMT;
		tQueueSettings.m_pUpdate = m_pUpdates;
		tQueueSettings.m_pDeletes = m_pDelete;
		tQueueSettings.m_pHook = &m_tHook;

		ppSorters[i] = sphCreateQueue ( tQueueSettings );

		uFactorFlags |= tQueueSettings.m_uPackedFactorFlags;

		if ( ppSorters[i] )
			iValidSorters++;

		// can't use multi-query for sorter with string attribute at group by or sort
		if ( ppSorters[i] && *pMulti )
			*pMulti = ppSorters[i]->CanMulti();
	}
	if ( !iValidSorters )
	{
		ReleaseIndex ( iLocal );
		return false;
	}

	CSphVector<int> dLocked;

	// setup kill-lists
	KillListVector dKillist;
	for ( int i=iLocal+1; i<m_dLocal.GetLength(); i++ )
	{
		const ServedIndex_c * pKillListIndex = UseIndex(i);
		if ( !pKillListIndex )
			continue;

		if ( pKillListIndex->m_bEnabled && pKillListIndex->m_pIndex->GetKillListSize() )
		{
			KillListTrait_t & tElem = dKillist.Add ();
			tElem.m_pBegin = pKillListIndex->m_pIndex->GetKillList();
			tElem.m_iLen = pKillListIndex->m_pIndex->GetKillListSize();
			dLocked.Add(i);
		} else
		{
			ReleaseIndex(i);
		}
	}

	int iIndexWeight = m_dLocal[iLocal].m_iWeight;

	// do the query
	CSphMultiQueryArgs tMultiArgs ( dKillist, iIndexWeight );
	tMultiArgs.m_uPackedFactorFlags = uFactorFlags;
	if ( m_bGotLocalDF )
	{
		tMultiArgs.m_bLocalDF = true;
		tMultiArgs.m_pLocalDocs = &m_hLocalDocs;
		tMultiArgs.m_iTotalDocs = m_iTotalDocs;
	}

	bool bResult = false;
	ppResults[0]->m_tIOStats.Start();
	if ( *pMulti )
	{
		bResult = pServed->m_pIndex->MultiQuery ( &m_dQueries[m_iStart], ppResults[0], iQueries, ppSorters, tMultiArgs );
	} else
	{
		bResult = pServed->m_pIndex->MultiQueryEx ( iQueries, &m_dQueries[m_iStart], ppResults, ppSorters, tMultiArgs );
	}
	ppResults[0]->m_tIOStats.Stop();

	iCpuTime += sphCpuTimer();
	for ( int i=0; i<iQueries; ++i )
		ppResults[i]->m_iCpuTime = iCpuTime;

	ARRAY_FOREACH ( i, dLocked )
		ReleaseIndex ( dLocked[i] );

	return bResult;
}


void SearchHandler_c::RunLocalSearches ( ISphMatchSorter * pLocalSorter, const char * sDistName, DWORD uFactorFlags )
{
	if ( g_iDistThreads>1 && m_dLocal.GetLength()>1 )
	{
		RunLocalSearchesMT();
		return;
	}

	CSphVector <int> dLocked;
	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		const char * sLocal = m_dLocal[iLocal].m_sName.cstr();
		int iOrderTag = m_dLocal[iLocal].m_iOrderTag;
		int iIndexWeight = m_dLocal[iLocal].m_iWeight;

		const ServedIndex_c * pServed = UseIndex ( iLocal );
		if ( !pServed || !pServed->m_bEnabled )
		{
			if ( sDistName )
				for ( int i=m_iStart; i<=m_iEnd; i++ )
					m_dFailuresSet[i].SubmitEx ( sDistName, "local index %s missing", sLocal );

			if ( pServed )
				ReleaseIndex ( iLocal );
			continue;
		}

		assert ( pServed->m_pIndex );
		assert ( pServed->m_bEnabled );

		// create sorters
		CSphVector<ISphMatchSorter*> dSorters ( m_iEnd-m_iStart+1 );
		ARRAY_FOREACH ( i, dSorters )
			dSorters[i] = NULL;

		DWORD uTotalFactorFlags = uFactorFlags;
		int iValidSorters = 0;
		for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
		{
			CSphString sError;
			CSphQuery & tQuery = m_dQueries[iQuery];
			CSphSchemaMT * pExtraSchema = tQuery.m_bAgent?m_dExtraSchemas[iQuery].GetVirgin():NULL;
			UnlockOnDestroy dSchemaLock ( pExtraSchema );

			// create sorter, if needed
			ISphMatchSorter * pSorter = pLocalSorter;
			if ( !pLocalSorter )
			{
				// create queue
				m_tHook.m_pIndex = pServed->m_pIndex;
				SphQueueSettings_t tQueueSettings ( tQuery, pServed->m_pIndex->GetMatchSchema(), sError, m_pProfile );
				tQueueSettings.m_bComputeItems = true;
				tQueueSettings.m_pExtra = pExtraSchema;
				tQueueSettings.m_pUpdate = m_pUpdates;
				tQueueSettings.m_pDeletes = m_pDelete;
				tQueueSettings.m_pHook = &m_tHook;

				pSorter = sphCreateQueue ( tQueueSettings );

				uTotalFactorFlags |= tQueueSettings.m_uPackedFactorFlags;
				tQuery.m_bZSlist = tQueueSettings.m_bZonespanlist;
				if ( !pSorter )
				{
					m_dFailuresSet[iQuery].Submit ( sLocal, sError.cstr() );
					continue;
				}
				if ( m_bMultiQueue )
				{
					// can't use multi-query for sorter with string attribute at group by or sort
					m_bMultiQueue = pSorter->CanMulti();

					if ( !m_bMultiQueue )
						m_bFacetQueue = false;
				}
				if ( !sError.IsEmpty() )
					m_dFailuresSet[iQuery].Submit ( sLocal, sError.cstr() );
			}

			dSorters[iQuery-m_iStart] = pSorter;
			iValidSorters++;
		}
		if ( !iValidSorters )
		{
			ReleaseIndex ( iLocal );
			continue;
		}

		// if sorter schemes have dynamic part, its lengths should be the same for queries to be optimized
		const ISphMatchSorter * pLastMulti = dSorters[0];
		for ( int i=1; i<dSorters.GetLength() && m_bMultiQueue; i++ )
		{
			if ( !dSorters[i] )
				continue;

			if ( !pLastMulti )
			{
				pLastMulti = dSorters[i];
				continue;
			}

			assert ( pLastMulti && dSorters[i] );
			m_bMultiQueue = pLastMulti->GetSchema().GetDynamicSize()==dSorters[i]->GetSchema().GetDynamicSize();
		}

		// facets, sanity check for json fields (can't be multi-queried yet)
		for ( int i=1; i<dSorters.GetLength() && m_bFacetQueue; i++ )
		{
			if ( !dSorters[i] )
				continue;
			for ( int j=0; j<dSorters[i]->GetSchema().GetAttrsCount(); j++ )
				if ( dSorters[i]->GetSchema().GetAttr(j).m_eAttrType==SPH_ATTR_JSON_FIELD )
				{
					m_bMultiQueue = m_bFacetQueue = false;
					break;
				}
		}

		if ( m_bFacetQueue )
			m_bMultiQueue = true;

		// me shortcuts
		AggrResult_t tStats;

		// set kill-list
		KillListVector dKillist;
		for ( int i=iLocal+1; i<m_dLocal.GetLength(); i++ )
		{
			const ServedIndex_c * pKillListIndex = UseIndex(i);
			if ( !pKillListIndex )
				continue;

			if ( pKillListIndex->m_bEnabled && pKillListIndex->m_pIndex->GetKillListSize() )
			{
				KillListTrait_t & tElem = dKillist.Add ();
				tElem.m_pBegin = pKillListIndex->m_pIndex->GetKillList();
				tElem.m_iLen = pKillListIndex->m_pIndex->GetKillListSize();
				dLocked.Add(i);
			} else
			{
				ReleaseIndex(i);
			}
		}

		// do the query
		CSphMultiQueryArgs tMultiArgs ( dKillist, iIndexWeight );
		tMultiArgs.m_uPackedFactorFlags = uTotalFactorFlags;
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
		} else
		{
			CSphVector<CSphQueryResult*> dResults ( m_dResults.GetLength() );
			ARRAY_FOREACH ( i, m_dResults )
				dResults[i] = &m_dResults[i];

			dResults[m_iStart]->m_tIOStats.Start();
			bResult = pServed->m_pIndex->MultiQueryEx ( dSorters.GetLength(), &m_dQueries[m_iStart], &dResults[m_iStart], &dSorters[0], tMultiArgs );
			dResults[m_iStart]->m_tIOStats.Stop();
		}

		// handle results
		if ( !bResult )
		{
			// failed, submit local (if not empty) or global error string
			for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
				m_dFailuresSet[iQuery].Submit ( sLocal, tStats.m_sError.IsEmpty()
					? m_dResults [ m_bMultiQueue ? m_iStart : iQuery ].m_sError.cstr()
					: tStats.m_sError.cstr() );
		} else
		{
			// multi-query succeeded
			for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
			{
				// but some of the sorters could had failed at "create sorter" stage
				ISphMatchSorter * pSorter = dSorters [ iQuery-m_iStart ];
				if ( !pSorter )
					continue;

				// this one seems OK
				AggrResult_t & tRes = m_dResults[iQuery];

				int64_t iBadRows = m_bMultiQueue ? tStats.m_iBadRows : tRes.m_iBadRows;
				if ( iBadRows )
					tRes.m_sWarning.SetSprintf ( "query result is inaccurate because of "INT64_FMT" missed documents", iBadRows );

				// multi-queue only returned one result set meta, so we need to replicate it
				if ( m_bMultiQueue )
				{
					// these times will be overridden below, but let's be clean
					tRes.m_iQueryTime += tStats.m_iQueryTime / ( m_iEnd-m_iStart+1 );
					tRes.m_iCpuTime += tStats.m_iCpuTime / ( m_iEnd-m_iStart+1 );
					tRes.m_tIOStats.Add ( tStats.m_tIOStats );
					tRes.m_pMva = tStats.m_pMva;
					tRes.m_pStrings = tStats.m_pStrings;
					tRes.m_bArenaProhibit = tStats.m_bArenaProhibit;
					MergeWordStats ( tRes, tStats.m_hWordStats, &m_dFailuresSet[iQuery], sLocal );
					tRes.m_iMultiplier = m_iEnd-m_iStart+1;
				} else if ( tRes.m_iMultiplier==-1 )
				{
					m_dFailuresSet[iQuery].Submit ( sLocal, tRes.m_sError.cstr() );
					continue;
				}

				tRes.m_iSuccesses++;
				// lets do this schema copy just once
				tRes.m_tSchema = pSorter->GetSchema();
				tRes.m_iTotalMatches += pSorter->GetTotalCount();
				tRes.m_iPredictedTime = tRes.m_bHasPrediction ? CalcPredictedTimeMsec ( tRes ) : 0;

				// extract matches from sorter
				FlattenToRes ( pSorter, tRes, iOrderTag+iQuery-m_iStart );

				if ( iBadRows )
					RemoveMissedRows ( tRes );

				// move external attributes storage from tStats to actual result
				tStats.LeakStorages ( tRes );
			}
		}

		ARRAY_FOREACH ( i, dLocked )
			ReleaseIndex ( dLocked[i] );

		dLocked.Resize ( 0 );

		// cleanup sorters
		if ( !pLocalSorter )
			ARRAY_FOREACH ( i, dSorters )
				SafeDelete ( dSorters[i] );
	}
}


// check expressions into a query to make sure that it's ready for multi query optimization
bool SearchHandler_c::AllowsMulti ( int iStart, int iEnd ) const
{
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
	ARRAY_FOREACH ( i, m_dLocal )
	{
		const ServedIndex_c * pServedIndex = UseIndex ( i );

		// check that it exists
		if ( !pServedIndex || !pServedIndex->m_bEnabled )
		{
			if ( pServedIndex )
				ReleaseIndex ( i );
			continue;
		}

		bool bHasExpression = false;
		const CSphSchema & tSchema = pServedIndex->m_pIndex->GetMatchSchema();
		for ( int iCheck=iStart; iCheck<=iEnd && !bHasExpression; iCheck++ )
			bHasExpression = sphHasExpressions ( m_dQueries[iCheck], tSchema );

		ReleaseIndex ( i );

		if ( bHasExpression )
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
		const ServedIndex_c * pIndex = UseIndex ( i );
		if ( pIndex && pIndex->m_bEnabled )
			bGlobalIDF = !pIndex->m_sGlobalIDFPath.IsEmpty();

		if ( pIndex )
			ReleaseIndex ( i );
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
		int iOff = dQuery.GetLength();
		dQuery.Resize ( iOff + iLen + 1 );
		memcpy ( dQuery.Begin()+iOff, tQuery.m_sQuery.cstr(), iLen );
		dQuery[iOff+iLen] = ' '; // queries delimiter
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
		const ServedIndex_c * pIndex = UseIndex ( i );
		if ( !pIndex || !pIndex->m_bEnabled )
		{
			if ( pIndex )
				ReleaseIndex ( i );
			continue;
		}

		dLocal.Add();
		dLocal.Last().m_iLocal = i;
		// TODO: cache settingsFNV on index load
		// FIXME!!! no need to count dictionary hash
		dLocal.Last().m_uHash = pIndex->m_pIndex->GetTokenizer()->GetSettingsFNV() ^ pIndex->m_pIndex->GetDictionary()->GetSettingsFNV();

		ReleaseIndex ( i );
	}
	dLocal.Sort ( bind ( &IndexSettings_t::m_uHash ) );

	// gather per-term docs count
	CSphVector < CSphKeywordInfo > dKeywords;
	ARRAY_FOREACH ( i, dLocal )
	{
		int iLocalIndex = dLocal[i].m_iLocal;
		const ServedIndex_c * pIndex = UseIndex ( iLocalIndex );
		if ( !pIndex || !pIndex->m_bEnabled )
		{
			if ( pIndex )
				ReleaseIndex ( iLocalIndex );
			continue;
		}

		m_iTotalDocs += pIndex->m_pIndex->GetStats().m_iTotalDocuments;

		if ( i && dLocal[i].m_uHash==dLocal[i-1].m_uHash )
		{
			ARRAY_FOREACH ( kw, dKeywords )
				dKeywords[kw].m_iDocs = 0;

			// no need to tokenize query just fill docs count
			pIndex->m_pIndex->FillKeywords ( dKeywords );
		} else
		{
			dKeywords.Resize ( 0 );
			pIndex->m_pIndex->GetKeywords ( dKeywords, dQuery.Begin(), true, NULL );

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

		ReleaseIndex ( dLocal[i].m_iLocal );
	}

	m_bGotLocalDF = true;
}


static int GetIndexWeight ( const char * sName, const CSphVector<CSphNamedInt> & dIndexWeights, int iDefaultWeight )
{
	ARRAY_FOREACH ( i, dIndexWeights )
		if ( dIndexWeights[i].m_sName==sName )
			return dIndexWeights[i].m_iValue;

	// distributed index adds {'*', weight} to all agents in case it got custom weight
	if ( dIndexWeights.GetLength() && dIndexWeights.Last().m_sName=="*" )
		return dIndexWeights.Begin()->m_iValue;

	return iDefaultWeight;
}

static uint64_t CalculateMass ( const CSphIndexStatus & dStats )
{
	return dStats.m_iNumChunks * 1000000 + dStats.m_iRamUse + dStats.m_iDiskUse * 10;
}

static uint64_t GetIndexMass ( const char * sName )
{
	ServedIndex_c * pIdx = g_pLocalIndexes->GetUnlockedEntryPtr ( sName );
	if ( pIdx )
		return pIdx->m_iMass;
	return 0;
}

struct TaggedLocalSorter_fn
{
	bool IsLess ( const LocalIndex_t & a, const LocalIndex_t & b ) const
	{
		return ( a.m_sName < b.m_sName ) || ( a.m_sName==b.m_sName && ( a.m_iOrderTag & 0x7FFFFFFF )>( b.m_iOrderTag & 0x7FFFFFFF ) );
	}
};

void SearchHandler_c::RunSubset ( int iStart, int iEnd )
{
	m_iStart = iStart;
	m_iEnd = iEnd;
	m_dLocal.Reset();

	// all my stats
	int64_t tmSubset = sphMicroTimer();
	int64_t tmLocal = 0;
	int64_t tmCpu = sphCpuTimer ();

	// prepare for descent
	CSphQuery & tFirst = m_dQueries[iStart];

	for ( int iRes=iStart; iRes<=iEnd; iRes++ )
		m_dResults[iRes].m_iSuccesses = 0;

	if ( iStart==iEnd && m_pProfile )
	{
		m_dResults[iStart].m_pProfile = m_pProfile;
		m_tHook.m_pProfiler = m_pProfile;
	}

	////////////////////////////////////////////////////////////////
	// check for single-query, multi-queue optimization possibility
	////////////////////////////////////////////////////////////////

	m_bMultiQueue = ( iStart<iEnd );
	for ( int iCheck=iStart+1; iCheck<=iEnd && m_bMultiQueue; iCheck++ )
	{
		const CSphQuery & qFirst = m_dQueries[iStart];
		const CSphQuery & qCheck = m_dQueries[iCheck];

		// these parameters must be the same
		if (
			( qCheck.m_sRawQuery!=qFirst.m_sRawQuery ) || // query string
			( qCheck.m_dWeights.GetLength()!=qFirst.m_dWeights.GetLength() ) || // weights count
			( qCheck.m_dWeights.GetLength() && memcmp ( qCheck.m_dWeights.Begin(), qFirst.m_dWeights.Begin(), sizeof(qCheck.m_dWeights[0])*qCheck.m_dWeights.GetLength() ) ) || // weights
			( qCheck.m_eMode!=qFirst.m_eMode ) || // search mode
			( qCheck.m_eRanker!=qFirst.m_eRanker ) || // ranking mode
			( qCheck.m_dFilters.GetLength()!=qFirst.m_dFilters.GetLength() ) || // attr filters count
			( qCheck.m_iCutoff!=qFirst.m_iCutoff ) || // cutoff
			( qCheck.m_eSort==SPH_SORT_EXPR && qFirst.m_eSort==SPH_SORT_EXPR && qCheck.m_sSortBy!=qFirst.m_sSortBy ) || // sort expressions
			( qCheck.m_bGeoAnchor!=qFirst.m_bGeoAnchor ) || // geodist expression
			( qCheck.m_bGeoAnchor && qFirst.m_bGeoAnchor
				&& ( qCheck.m_fGeoLatitude!=qFirst.m_fGeoLatitude || qCheck.m_fGeoLongitude!=qFirst.m_fGeoLongitude ) ) ) // some geodist cases
		{
			m_bMultiQueue = false;
			break;
		}

		// filters must be the same too
		assert ( qCheck.m_dFilters.GetLength()==qFirst.m_dFilters.GetLength() );
		ARRAY_FOREACH ( i, qCheck.m_dFilters )
			if ( qCheck.m_dFilters[i]!=qFirst.m_dFilters[i] )
			{
				m_bMultiQueue = false;
				break;
			}
	}

	// check for facets
	m_bFacetQueue = iEnd>iStart;
	for ( int iCheck=iStart+1; iCheck<=iEnd && m_bFacetQueue; iCheck++ )
		if ( !m_dQueries[iCheck].m_bFacet )
			m_bFacetQueue = false;

	if ( m_bFacetQueue )
		m_bMultiQueue = true;

	////////////////////////////
	// build local indexes list
	////////////////////////////

	CSphVector<AgentConn_t> dAgents;
	int iDivideLimits = 1;
	int iAgentConnectTimeout = 0, iAgentQueryTimeout = 0;
	int iTagsCount = 0;
	int iTagStep = iEnd - iStart + 1;

	// they're all local, build the list
	if ( tFirst.m_sIndexes=="*" )
	{
		// search through all local indexes
		for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
			if ( it.Get ().m_bEnabled )
			{
				m_dLocal.Add().m_sName = it.GetKey();
				m_dLocal.Last().m_iOrderTag = iTagsCount;
				m_dLocal.Last().m_iWeight = GetIndexWeight ( it.GetKey().cstr(), tFirst.m_dIndexWeights, 1 );
				m_dLocal.Last().m_iMass = it.Get().m_iMass;
				iTagsCount += iTagStep;
			}
	} else
	{
		CSphVector<CSphString> dLocal;
		// search through specified local indexes
		ParseIndexList ( tFirst.m_sIndexes, dLocal );

		int iDistCount = 0;
		bool bDevideRemote = false;

		g_tDistLock.Lock();
		ARRAY_FOREACH ( i, dLocal )
		{
			const CSphString & sIndex = dLocal[i];
			DistributedIndex_t * pDist = g_hDistIndexes ( sIndex );
			if ( !pDist )
			{
				m_dLocal.Add().m_sName = dLocal[i];
				m_dLocal.Last().m_iOrderTag = iTagsCount;
				m_dLocal.Last().m_iWeight = GetIndexWeight ( sIndex.cstr(), tFirst.m_dIndexWeights, 1 );
				m_dLocal.Last().m_iMass = GetIndexMass ( sIndex.cstr() );
				iTagsCount += iTagStep;

			} else
			{
				iDistCount++;
				iAgentConnectTimeout = iAgentConnectTimeout ? Min ( pDist->m_iAgentConnectTimeout, iAgentConnectTimeout ) : pDist->m_iAgentConnectTimeout;
				iAgentQueryTimeout = iAgentQueryTimeout ? Min ( pDist->m_iAgentQueryTimeout, iAgentQueryTimeout ) : pDist->m_iAgentQueryTimeout;
				int iWeight = GetIndexWeight ( sIndex.cstr(), tFirst.m_dIndexWeights, -1 );

				dAgents.Reserve ( dAgents.GetLength() + pDist->m_dAgents.GetLength() );
				ARRAY_FOREACH ( j, pDist->m_dAgents )
				{
					dAgents.Add().TakeTraits ( *pDist->m_dAgents[j].GetRRAgent() );
					dAgents.Last().m_iStoreTag = iTagsCount;
					dAgents.Last().m_iWeight = iWeight;
					iTagsCount += iTagStep;
				}

				m_dLocal.Reserve ( m_dLocal.GetLength() + pDist->m_dLocal.GetLength() );
				ARRAY_FOREACH ( j, pDist->m_dLocal )
				{
					m_dLocal.Add().m_sName = pDist->m_dLocal[j];
					m_dLocal.Last().m_iOrderTag = iTagsCount;
					if ( iWeight!=-1 )
						m_dLocal.Last().m_iWeight = iWeight;
					iTagsCount += iTagStep;
					m_dLocal.Last().m_iMass = GetIndexMass ( pDist->m_dLocal[j].cstr() );
				}

				bDevideRemote |= pDist->m_bDivideRemoteRanges;
			}
		}
		g_tDistLock.Unlock();

		// set remote devider
		if ( bDevideRemote )
		{
			if ( iDistCount==1 )
				iDivideLimits = dAgents.GetLength();
			else
			{
				for ( int iRes=iStart; iRes<=iEnd; iRes++ )
					m_dResults[iRes].m_sWarning.SetSprintf ( "distribute multi-index query '%s' doesn't support divide_remote_ranges", tFirst.m_sIndexes.cstr() );
			}
		}

		// eliminate local dupes that come from distributed indexes
		if ( m_dLocal.GetLength() && dAgents.GetLength() )
		{
			m_dLocal.Sort ( TaggedLocalSorter_fn() );
			int iSrc = 1, iDst = 1;
			while ( iSrc<m_dLocal.GetLength() )
			{
				if ( m_dLocal[iDst-1].m_sName==m_dLocal[iSrc].m_sName )
					iSrc++;
				else
					m_dLocal[iDst++] = m_dLocal[iSrc++];
			}
			m_dLocal.Resize ( iDst );
			m_dLocal.Sort ( bind ( &LocalIndex_t::m_iOrderTag ) ); // keep initial order of locals
		}

		ARRAY_FOREACH ( i, m_dLocal )
		{
			const ServedIndex_c * pServedIndex = UseIndex ( i );
			bool bEnabled = pServedIndex && pServedIndex->m_bEnabled;
			if ( pServedIndex )
				ReleaseIndex ( i );

			// check that it exists
			if ( !pServedIndex )
			{
				if ( tFirst.m_bIgnoreNonexistentIndexes )
				{
					m_dLocal.Remove ( i-- );
					continue;
				}
				for ( int iRes=iStart; iRes<=iEnd; iRes++ )
					m_dResults[iRes].m_sError.SetSprintf ( "unknown local index '%s' in search request", m_dLocal[i].m_sName.cstr() );
				return;
			}

			// if it exists but is not enabled, remove it from the list and force recheck
			if ( !bEnabled )
				m_dLocal.Remove ( i-- );
		}
	}

	// sanity check
	if ( !dAgents.GetLength() && !m_dLocal.GetLength() )
	{
		const char * sIndexType = ( dAgents.GetLength() ? "" : "local" );
		for ( int iRes=iStart; iRes<=iEnd; iRes++ )
			m_dResults[iRes].m_sError.SetSprintf ( "no enabled %s indexes to search", sIndexType );
		return;
	}
	ARRAY_FOREACH ( i, m_dResults )
		m_dResults[i].m_dTag2Pools.Resize ( iTagsCount );

	/////////////////////////////////////////////////////
	// optimize single-query, same-schema local searches
	/////////////////////////////////////////////////////

	DWORD uLocalPFFlags = SPH_FACTOR_DISABLE;
	ISphMatchSorter * pLocalSorter = NULL;
	while ( iStart==iEnd && m_dLocal.GetLength()>1 )
	{
		CSphString sError;

		// check if all schemes are equal
		bool bAllEqual = true;

		const ServedIndex_c * pFirstIndex = UseIndex ( 0 );
		if ( !pFirstIndex || !pFirstIndex->m_bEnabled )
		{
			if ( pFirstIndex )
				ReleaseIndex ( 0 );
			break;
		}

		const CSphSchema & tFirstSchema = pFirstIndex->m_pIndex->GetMatchSchema();
		for ( int i=1; i<m_dLocal.GetLength() && bAllEqual; i++ )
		{
			const ServedIndex_c * pNextIndex = UseIndex ( i );
			if ( !pNextIndex || !pNextIndex->m_bEnabled )
			{
				bAllEqual = false;
				if ( pNextIndex )
					ReleaseIndex ( i );
				break;
			}

			if ( !tFirstSchema.CompareTo ( pNextIndex->m_pIndex->GetMatchSchema(), sError ) )
				bAllEqual = false;

			ReleaseIndex ( i );
		}

		// we can reuse the very same sorter
		if ( bAllEqual )
		{
			CSphSchemaMT * pExtraSchemaMT = m_dQueries[iStart].m_bAgent?m_dExtraSchemas[iStart].GetVirgin():NULL;
			UnlockOnDestroy ExtraLocker ( pExtraSchemaMT );

			m_tHook.m_pIndex = pFirstIndex->m_pIndex;
			SphQueueSettings_t tQueueSettings ( m_dQueries[iStart], tFirstSchema, sError, m_pProfile );
			tQueueSettings.m_bComputeItems = true;
			tQueueSettings.m_pExtra = pExtraSchemaMT;
			tQueueSettings.m_pHook = &m_tHook;

			pLocalSorter = sphCreateQueue ( tQueueSettings );

			uLocalPFFlags = tQueueSettings.m_uPackedFactorFlags;
		}

		ReleaseIndex ( 0 );
		break;
	}

	// select lists must have no expressions
	if ( m_bMultiQueue )
		m_bMultiQueue = AllowsMulti ( iStart, iEnd );

	if ( !m_bMultiQueue )
		m_bFacetQueue = false;

	// these are mutual exclusive
	assert ( !( m_bMultiQueue && pLocalSorter ) );

	///////////////////////////////////////////////////////////
	// main query loop (with multiple retries for distributed)
	///////////////////////////////////////////////////////////

	// connect to remote agents and query them, if required
	CSphScopedPtr<SearchRequestBuilder_t> tReqBuilder ( NULL );
	CSphScopedPtr<ISphRemoteAgentsController> tDistCtrl ( NULL );
	if ( dAgents.GetLength() )
	{
		if ( m_pProfile )
			m_pProfile->Switch ( SPH_QSTATE_DIST_CONNECT );

		int iRetryCount = Min ( Max ( tFirst.m_iRetryCount, 0 ), MAX_RETRY_COUNT ); // paranoid clamp

		tReqBuilder = new SearchRequestBuilder_t ( m_dQueries, iStart, iEnd, iDivideLimits );
		tDistCtrl = GetAgentsController ( g_iDistThreads, dAgents,
			*tReqBuilder.Ptr(), iAgentConnectTimeout, iRetryCount, tFirst.m_iRetryDelay );
	}

	/////////////////////
	// run local queries
	//////////////////////

	// while the remote queries are running, do local searches
	// FIXME! what if the remote agents finish early, could they timeout?
	if ( m_dLocal.GetLength() )
	{
		SetupLocalDF ( iStart, iEnd );

		if ( m_pProfile )
			m_pProfile->Switch ( SPH_QSTATE_LOCAL_SEARCH );

		tmLocal = -sphMicroTimer();
		RunLocalSearches ( pLocalSorter, dAgents.GetLength() ? tFirst.m_sIndexes.cstr() : NULL, uLocalPFFlags );
		tmLocal += sphMicroTimer();
	}

	///////////////////////
	// poll remote queries
	///////////////////////

	bool bDistDone = false;
	if ( dAgents.GetLength() )
	{
		if ( m_pProfile )
			m_pProfile->Switch ( SPH_QSTATE_DIST_WAIT );

		while ( !bDistDone )
		{
			// don't forget to check incoming replies after send was over
			tDistCtrl->WaitAgentsEvent();
			bDistDone = tDistCtrl->IsDone();
			// wait for remote queries to complete
			if ( tDistCtrl->HasReadyAgents() )
			{
				CSphVector<DWORD> dMvaStorage;
				CSphVector<BYTE> dStringStorage;
				dMvaStorage.Add ( 0 );
				dStringStorage.Add ( 0 );
				SearchReplyParser_t tParser ( iStart, iEnd, dMvaStorage, dStringStorage );
				int iMsecLeft = iAgentQueryTimeout - (int)( tmLocal/1000 );
				int iReplys = RemoteWaitForAgents ( dAgents, Max ( iMsecLeft, 0 ), tParser );
				// check if there were valid (though might be 0-matches) replies, and merge them
				if ( iReplys )
				{
					DWORD * pMva = dMvaStorage.Begin();
					BYTE * pString = dStringStorage.Begin();
					ARRAY_FOREACH ( iAgent, dAgents )
					{
						AgentConn_t & tAgent = dAgents[iAgent];
						if ( !tAgent.m_bSuccess )
							continue;

						int iOrderTag = tAgent.m_iStoreTag;
						// merge this agent's results
						for ( int iRes=iStart; iRes<=iEnd; iRes++ )
						{
							const CSphQueryResult & tRemoteResult = tAgent.m_dResults[iRes-iStart];

							// copy errors or warnings
							if ( !tRemoteResult.m_sError.IsEmpty() )
								m_dFailuresSet[iRes].SubmitEx ( tFirst.m_sIndexes.cstr(),
									"agent %s: remote query error: %s",
									tAgent.GetMyUrl().cstr(), tRemoteResult.m_sError.cstr() );
							if ( !tRemoteResult.m_sWarning.IsEmpty() )
								m_dFailuresSet[iRes].SubmitEx ( tFirst.m_sIndexes.cstr(),
									"agent %s: remote query warning: %s",
									tAgent.GetMyUrl().cstr(), tRemoteResult.m_sWarning.cstr() );

							if ( tRemoteResult.m_iSuccesses<=0 )
								continue;

							AggrResult_t & tRes = m_dResults[iRes];
							tRes.m_iSuccesses++;
							tRes.m_tSchema = tRemoteResult.m_tSchema;

							assert ( !tRes.m_dTag2Pools[iOrderTag + iRes - iStart].m_pMva && !tRes.m_dTag2Pools[iOrderTag + iRes - iStart].m_pStrings );

							tRes.m_dMatches.Reserve ( tRemoteResult.m_dMatches.GetLength() );
							ARRAY_FOREACH ( i, tRemoteResult.m_dMatches )
							{
								tRes.m_dMatches.Add();
								tRemoteResult.m_tSchema.CloneWholeMatch ( &tRes.m_dMatches.Last(), tRemoteResult.m_dMatches[i] );
								tRes.m_dMatches.Last().m_iTag = ( iOrderTag + iRes - iStart ) | 0x80000000;
							}

							tRes.m_pMva = pMva;
							tRes.m_pStrings = pString;
							tRes.m_dTag2Pools[iOrderTag+iRes-iStart].m_pMva = pMva;
							tRes.m_dTag2Pools[iOrderTag+iRes-iStart].m_pStrings = pString;
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

							// merge this agent's words
							MergeWordStats ( tRes, tRemoteResult.m_hWordStats, &m_dFailuresSet[iRes], tFirst.m_sIndexes.cstr() );
						}

						// dismissed
						tAgent.m_dResults.Reset ();
						tAgent.m_bSuccess = false;
						tAgent.m_sFailure = "";
					}

					m_dMva2Free.Add ( dMvaStorage.LeakData() );
					m_dString2Free.Add ( dStringStorage.LeakData() );
				}
			}
		} // while ( !bDistDone )
	} // if ( bDist && dAgents.GetLength() )

	// submit failures from failed agents
	// copy timings from all agents
	if ( dAgents.GetLength() )
	{
		ARRAY_FOREACH ( i, dAgents )
		{
			const AgentConn_t & tAgent = dAgents[i];

			for ( int j=iStart; j<=iEnd; j++ )
			{
				assert ( tAgent.m_iWall>=0 );
				if ( tAgent.m_iWall<0 )
					m_dAgentTimes[j].Add ( ( tAgent.m_iWall + sphMicroTimer() ) / ( 1000 * ( iEnd-iStart+1 ) ) );
				else
					m_dAgentTimes[j].Add ( ( tAgent.m_iWall ) / ( 1000 * ( iEnd-iStart+1 ) ) );
			}

			if ( !tAgent.m_bSuccess && !tAgent.m_sFailure.IsEmpty() )
				for ( int j=iStart; j<=iEnd; j++ )
					m_dFailuresSet[j].SubmitEx ( tFirst.m_sIndexes.cstr(), tAgent.m_bBlackhole ? "blackhole %s: %s" : "agent %s: %s",
						tAgent.GetMyUrl().cstr(), tAgent.m_sFailure.cstr() );
		}
	}

	// cleanup
	bool bWasLocalSorter = pLocalSorter!=NULL;
	SafeDelete ( pLocalSorter );

	/////////////////////
	// merge all results
	/////////////////////

	if ( m_pProfile )
		m_pProfile->Switch ( SPH_QSTATE_AGGREGATE );

	CSphIOStats tIO;

	for ( int iRes=iStart; iRes<=iEnd; iRes++ )
	{
		AggrResult_t & tRes = m_dResults[iRes];
		CSphQuery & tQuery = m_dQueries[iRes];
		CSphSchemaMT * pExtraSchema = tQuery.m_bAgent ? m_dExtraSchemas.Begin() + ( bWasLocalSorter ? 0 : iRes ) : NULL;

		// minimize sorters needs these pointers
		tIO.Add ( tRes.m_tIOStats );

		// if there were no successful searches at all, this is an error
		if ( !tRes.m_iSuccesses )
		{
			CSphStringBuilder sFailures;
			m_dFailuresSet[iRes].BuildReport ( sFailures );

			tRes.m_sError = sFailures.cstr();
			continue;
		}

		// minimize schema and remove dupes
		// assuming here ( tRes.m_tSchema==tRes.m_dSchemas[0] )
		const CSphFilterSettings * pAggrFilter = NULL;
		if ( m_bMaster && !tQuery.m_tHaving.m_sAttrName.IsEmpty() )
			pAggrFilter = &tQuery.m_tHaving;

		if ( tRes.m_iSuccesses>1 || tQuery.m_dItems.GetLength() || pAggrFilter )
		{
			if ( pExtraSchema )
				pExtraSchema->RLock();
			UnlockOnDestroy SchemaLocker ( pExtraSchema );

			if ( m_bMaster && tRes.m_iSuccesses && tQuery.m_dItems.GetLength() && tQuery.m_sGroupBy.IsEmpty() && tRes.m_dMatches.GetLength()==0 )
			{
				ARRAY_FOREACH ( i, tQuery.m_dItems )
				{
					if ( tQuery.m_dItems[i].m_sExpr=="count(*)" )
					{
						tRes.m_sZeroCountName = tQuery.m_dItems[i].m_sAlias;
						break;
					}
				}
			}

			if ( !MinimizeAggrResult ( tRes, tQuery, m_dLocal.GetLength(), dAgents.GetLength(), pExtraSchema, m_pProfile, m_bSphinxql, pAggrFilter ) )
			{
				tRes.m_iSuccesses = 0;
				return; // FIXME? really return, not just continue?
			}
		}

		if ( !m_dFailuresSet[iRes].IsEmpty() )
		{
			CSphStringBuilder sFailures;
			m_dFailuresSet[iRes].BuildReport ( sFailures );
			tRes.m_sWarning = sFailures.cstr();
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

	for ( int iRes=iStart; iRes<=iEnd; iRes++ )
	{
		AggrResult_t & tRes = m_dResults[iRes];
		CSphQuery & tQuery = m_dQueries[iRes];

		// FIXME! log such queries properly?
		if ( tQuery.m_pTableFunc )
		{
			if ( m_pProfile )
				m_pProfile->Switch ( SPH_QSTATE_TABLE_FUNC );
			if ( !tQuery.m_pTableFunc->Process ( &tRes, tRes.m_sError ) )
				tRes.m_iSuccesses = 0;
			// we don't need it anymore
			SafeDelete ( tQuery.m_pTableFunc ); // FIXME? find more obvious place to delete
		}
	}

	/////////
	// stats
	/////////

	tmSubset = sphMicroTimer() - tmSubset;
	tmCpu = sphCpuTimer() - tmCpu;

	// in multi-queue case (1 actual call per N queries), just divide overall query time evenly
	// otherwise (N calls per N queries), divide common query time overheads evenly
	const int iQueries = iEnd-iStart+1;
	if ( m_bMultiQueue )
	{
		for ( int iRes=iStart; iRes<=iEnd; iRes++ )
		{
			m_dResults[iRes].m_iQueryTime = (int)( tmSubset/1000/iQueries );
			m_dResults[iRes].m_iRealQueryTime = (int)( tmSubset/1000/iQueries );
			m_dResults[iRes].m_iCpuTime = tmCpu/iQueries;
		}
	} else
	{
		int64_t tmAccountedWall = 0;
		int64_t tmAccountedCpu = 0;
		for ( int iRes=iStart; iRes<=iEnd; iRes++ )
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

		for ( int iRes=iStart; iRes<=iEnd; iRes++ )
		{
			m_dResults[iRes].m_iQueryTime += (int)(tmDeltaWall/1000);
			m_dResults[iRes].m_iRealQueryTime = (int)( tmSubset/1000/iQueries );
			m_dResults[iRes].m_iCpuTime = tmCpu/iQueries;
			if ( bExternalWork )
				m_dResults[iRes].m_iCpuTime += tmAccountedCpu;
		}
		// don't forget to add this to stats
		if ( bExternalWork )
			tmCpu += tmAccountedCpu;
	}

	g_tStatsMutex.Lock();
	g_tStats.m_iQueries += iQueries;
	g_tStats.m_iQueryTime += tmSubset;
	g_tStats.m_iQueryCpuTime += tmCpu;
	if ( dAgents.GetLength() )
	{
		int64_t tmWait = 0;
		ARRAY_FOREACH ( i, dAgents )
		{
			tmWait += dAgents[i].m_iWaited;
		}
		// do *not* count queries to dist indexes w/o actual remote agents
		g_tStats.m_iDistQueries++;
		g_tStats.m_iDistWallTime += tmSubset;
		g_tStats.m_iDistLocalTime += tmLocal;
		g_tStats.m_iDistWaitTime += tmWait;
	}
	g_tStats.m_iDiskReads += tIO.m_iReadOps;
	g_tStats.m_iDiskReadTime += tIO.m_iReadTime;
	g_tStats.m_iDiskReadBytes += tIO.m_iReadBytes;
	g_tStatsMutex.Unlock();

	if ( m_pProfile )
		m_pProfile->Switch ( SPH_QSTATE_UNKNOWN );
}


bool CheckCommandVersion ( int iVer, int iDaemonVersion, ISphOutputBuffer & tOut )
{
	if ( (iVer>>8)!=(iDaemonVersion>>8) )
	{
		SendErrorReply ( tOut, "major command version mismatch (expected v.%d.x, got v.%d.%d)",
			iDaemonVersion>>8, iVer>>8, iVer&0xff );
		return false;
	}
	if ( iVer>iDaemonVersion )
	{
		SendErrorReply ( tOut, "client version is higher than daemon version (client is v.%d.%d, daemon is v.%d.%d)",
			iVer>>8, iVer&0xff, iDaemonVersion>>8, iDaemonVersion&0xff );
		return false;
	}
	return true;
}


void SendSearchResponse ( SearchHandler_c & tHandler, ISphOutputBuffer & tOut, int iVer, int iMasterVer )
{
	// serve the response
	int iReplyLen = 0;
	bool bAgentMode = ( iMasterVer>0 );

	if ( iVer<=0x10C )
	{
		assert ( tHandler.m_dQueries.GetLength()==1 );
		assert ( tHandler.m_dResults.GetLength()==1 );
		const AggrResult_t & tRes = tHandler.m_dResults[0];

		if ( !tRes.m_sError.IsEmpty() )
		{
			SendErrorReply ( tOut, "%s", tRes.m_sError.cstr() );
			return;
		}

		iReplyLen = CalcResultLength ( iVer, &tRes, tRes.m_dTag2Pools, bAgentMode, tHandler.m_dQueries[0], iMasterVer );
		bool bWarning = ( iVer>=0x106 && !tRes.m_sWarning.IsEmpty() );

		// send it
		tOut.SendWord ( (WORD)( bWarning ? SEARCHD_WARNING : SEARCHD_OK ) );
		tOut.SendWord ( VER_COMMAND_SEARCH );
		tOut.SendInt ( iReplyLen );

		SendResult ( iVer, tOut, &tRes, tRes.m_dTag2Pools, bAgentMode, tHandler.m_dQueries[0], iMasterVer );

	} else
	{
		ARRAY_FOREACH ( i, tHandler.m_dQueries )
			iReplyLen += CalcResultLength ( iVer, &tHandler.m_dResults[i], tHandler.m_dResults[i].m_dTag2Pools, bAgentMode, tHandler.m_dQueries[i], iMasterVer );

		// send it
		tOut.SendWord ( (WORD)SEARCHD_OK );
		tOut.SendWord ( VER_COMMAND_SEARCH );
		tOut.SendInt ( iReplyLen );

		ARRAY_FOREACH ( i, tHandler.m_dQueries )
			SendResult ( iVer, tOut, &tHandler.m_dResults[i], tHandler.m_dResults[i].m_dTag2Pools, bAgentMode, tHandler.m_dQueries[i], iMasterVer );
	}

	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==iReplyLen+8 );
}


void HandleCommandSearch ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq, ThdDesc_t * pThd )
{
	MEMORY ( MEM_API_SEARCH );

	if ( !CheckCommandVersion ( iVer, VER_COMMAND_SEARCH, tOut ) )
		return;

	int iMasterVer = 0;
	if ( iVer>=0x118 )
		iMasterVer = tReq.GetInt();
	if ( iMasterVer<0 || iMasterVer>VER_MASTER )
	{
		SendErrorReply ( tOut, "master-agent version mismatch; update me first, then update master!" );
		return;
	}

	// parse request
	int iQueries = 1;
	if ( iVer>=0x10D )
		iQueries = tReq.GetDword ();

	if ( g_iMaxBatchQueries>0 && ( iQueries<=0 || iQueries>g_iMaxBatchQueries ) )
	{
		SendErrorReply ( tOut, "bad multi-query count %d (must be in 1..%d range)", iQueries, g_iMaxBatchQueries );
		return;
	}

	SearchHandler_c tHandler ( iQueries, false, ( iMasterVer==0 ), pThd->m_iConnID );
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
		if ( !ParseSearchQuery ( tReq, tOut, tHandler.m_dQueries[i], iVer, iMasterVer ) )
			return;

	if ( pThd && tHandler.m_dQueries.GetLength() )
	{
		const CSphQuery & q = tHandler.m_dQueries[0];
		pThd->SetThreadInfo ( "api-search query=\"%s\" comment=\"%s\"", q.m_sQuery.scstr(), q.m_sComment.scstr() );
	}

	// run queries, send response
	tHandler.RunQueries();
	SendSearchResponse ( tHandler, tOut, iVer, iMasterVer );

	int64_t iTotalPredictedTime = 0;
	int64_t iTotalAgentPredictedTime = 0;
	ARRAY_FOREACH ( i, tHandler.m_dResults )
	{
		iTotalPredictedTime += tHandler.m_dResults[i].m_iPredictedTime;
		iTotalAgentPredictedTime += tHandler.m_dResults[i].m_iAgentPredictedTime;
	}

	g_tLastMetaMutex.Lock();
	g_tLastMeta = tHandler.m_dResults[tHandler.m_dResults.GetLength()-1];
	g_tLastMetaMutex.Unlock();

	g_tStatsMutex.Lock();
	g_tStats.m_iPredictedTime += iTotalPredictedTime;
	g_tStats.m_iAgentPredictedTime += iTotalAgentPredictedTime;
	g_tStatsMutex.Unlock();
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
	virtual bool ValidateArgs ( const CSphVector<CSphString> & dArgs, const CSphQuery &, CSphString & sError )
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
			if ( t==SPH_ATTR_STRING && iCur!=0 )
				iCur = (SphAttr_t)( pResult->m_dTag2Pools [ dMatches[i].m_iTag ].m_pStrings + iCur );

			if ( iCur==iLastValue )
				continue;
			if ( iCur && iLastValue && t==SPH_ATTR_STRINGPTR && strcmp ( (const char*)iCur, (const char*)iLastValue )==0 )
				continue;
			if ( iCur && iLastValue && t==SPH_ATTR_STRING )
			{
				const BYTE * a = (const BYTE*) iCur;
				const BYTE * b = (const BYTE*) iLastValue;
				int iLen1 = sphUnpackStr ( a, &a );
				int iLen2 = sphUnpackStr ( b, &b );
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
	"desc", "show_tables", "update", "create_func", "drop_func", "attach_index",
	"flush_rtindex", "flush_ramchunk", "show_variables", "truncate_rtindex", "select_sysvar",
	"show_collation", "show_character_set", "optimize_index", "show_agent_status",
	"show_index_status", "show_profile", "alter_add", "alter_drop", "show_plan",
	"select_dual", "show_databases", "create_plugin", "drop_plugin", "show_plugins", "show_threads",
	"facet", "alter_reconfigure", "show_index_settings", "flush_index", "reload_plugins", "reload_index"
};


STATIC_ASSERT ( sizeof(g_dSqlStmts)/sizeof(g_dSqlStmts[0])==STMT_TOTAL, STMT_DESC_SHOULD_BE_SAME_AS_STMT_TOTAL );


/// parser view on a generic node
/// CAUTION, nodes get copied in the parser all the time, must keep assignment slim
struct SqlNode_t
{
	int						m_iStart;	///< first byte relative to m_pBuf, inclusive
	int						m_iEnd;		///< last byte relative to m_pBuf, exclusive! thus length = end - start
	int64_t					m_iValue;
	int						m_iType;	///< TOK_xxx type for insert values; SPHINXQL_TOK_xxx code for special idents
	float					m_fValue;
	AttrValues_p			m_pValues;	///< filter values vector (FIXME? replace with numeric handles into parser state?)

	SqlNode_t()
		: m_iValue ( 0 )
		, m_iType ( 0 )
		, m_pValues ( NULL )
	{}
};
#define YYSTYPE SqlNode_t

SqlStmt_t::SqlStmt_t ()
	: m_eStmt ( STMT_PARSE_ERROR )
	, m_iRowsAffected ( 0 )
	, m_sStmt ( NULL )
	, m_iSchemaSz ( 0 )
	, m_eSet ( SET_LOCAL )
	, m_iSetValue ( 0 )
	, m_bSetNull ( false )
	, m_iListStart ( -1 )
	, m_iListEnd ( -1 )
	, m_eUdfType ( SPH_ATTR_NONE )
	, m_iThreadsCols ( 0 )
	, m_iIntParam ( -1 )
{
	m_tQuery.m_eMode = SPH_MATCH_EXTENDED2; // only new and shiny matching and sorting
	m_tQuery.m_eSort = SPH_SORT_EXTENDED;
	m_tQuery.m_sSortBy = "@weight desc"; // default order
	m_tQuery.m_sOrderBy = "@weight desc";
	m_tQuery.m_iAgentQueryTimeout = g_iAgentQueryTimeout;
	m_tQuery.m_iRetryCount = g_iAgentRetryCount;
	m_tQuery.m_iRetryDelay = g_iAgentRetryDelay;
}

bool SqlStmt_t::AddSchemaItem ( const char * psName )
{
	m_dInsertSchema.Add ( psName );
	m_dInsertSchema.Last().ToLower();
	m_iSchemaSz = m_dInsertSchema.GetLength();
	return true; // stub; check if the given field actually exists in the schema
}

// check if the number of fields which would be inserted is in accordance to the given schema
bool SqlStmt_t::CheckInsertIntegrity()
{
	// cheat: if no schema assigned, assume the size of schema as the size of the first row.
	// (if it is wrong, it will be revealed later)
	if ( !m_iSchemaSz )
		m_iSchemaSz = m_dInsertValues.GetLength();

	m_iRowsAffected++;
	return m_dInsertValues.GetLength()==m_iRowsAffected*m_iSchemaSz;
}


/// magic codes passed via SqlNode_t::m_iStart to handle certain special tokens
/// for instance, to fixup "count(*)" as "@count" easily
enum
{
	SPHINXQL_TOK_COUNT		= -1,
	SPHINXQL_TOK_GROUPBY	= -2,
	SPHINXQL_TOK_WEIGHT		= -3,
	SPHINXQL_TOK_ID			= -4
};


struct SqlParser_c : ISphNoncopyable
{
public:
	void *			m_pScanner;
	const char *	m_pBuf;
	const char *	m_pLastTokenStart;
	CSphString *	m_pParseError;
	CSphQuery *		m_pQuery;
	bool			m_bGotQuery;
	SqlStmt_t *		m_pStmt;
	CSphVector<SqlStmt_t> & m_dStmt;
	ESphCollation	m_eCollation;
	BYTE			m_uSyntaxFlags;

public:
	explicit		SqlParser_c ( CSphVector<SqlStmt_t> & dStmt, ESphCollation eCollation );

	void			PushQuery ();

	bool			AddOption ( const SqlNode_t & tIdent );
	bool			AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue );
	bool			AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue, const SqlNode_t & sArg );
	bool			AddOption ( const SqlNode_t & tIdent, CSphVector<CSphNamedInt> & dNamed );
	bool			AddInsertOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue );
	void			AddItem ( SqlNode_t * pExpr, ESphAggrFunc eFunc=SPH_AGGR_NONE, SqlNode_t * pStart=NULL, SqlNode_t * pEnd=NULL );
	bool			AddItem ( const char * pToken, SqlNode_t * pStart=NULL, SqlNode_t * pEnd=NULL );
	bool			AddCount ();
	void			AliasLastItem ( SqlNode_t * pAlias );

	/// called on transition from an outer select to inner select
	void ResetSelect()
	{
		if ( m_pQuery )
			m_pQuery->m_iSQLSelectStart = m_pQuery->m_iSQLSelectEnd = -1;
	}

	/// called every time we capture a select list item
	/// (i think there should be a simpler way to track these though)
	void SetSelect ( SqlNode_t * pStart, SqlNode_t * pEnd=NULL )
	{
		if ( m_pQuery )
		{
			if ( pStart && ( m_pQuery->m_iSQLSelectStart<0 || m_pQuery->m_iSQLSelectStart>pStart->m_iStart ) )
				m_pQuery->m_iSQLSelectStart = pStart->m_iStart;
			if ( !pEnd )
				pEnd = pStart;
			if ( pEnd && ( m_pQuery->m_iSQLSelectEnd<0 || m_pQuery->m_iSQLSelectEnd<pEnd->m_iEnd ) )
				m_pQuery->m_iSQLSelectEnd = pEnd->m_iEnd;
		}
	}

	inline CSphString & ToString ( CSphString & sRes, const SqlNode_t & tNode ) const
	{
		if ( tNode.m_iType>=0 )
			sRes.SetBinary ( m_pBuf + tNode.m_iStart, tNode.m_iEnd - tNode.m_iStart );
		else switch ( tNode.m_iType )
		{
			case SPHINXQL_TOK_COUNT:	sRes = "@count"; break;
			case SPHINXQL_TOK_GROUPBY:	sRes = "@groupby"; break;
			case SPHINXQL_TOK_WEIGHT:	sRes = "@weight"; break;
			case SPHINXQL_TOK_ID:		sRes = "@id"; break;
			default:					assert ( 0 && "INTERNAL ERROR: unknown parser ident code" );
		}
		return sRes;
	}

	inline void ToStringUnescape ( CSphString & sRes, const SqlNode_t & tNode ) const
	{
		assert ( tNode.m_iType>=0 );
		SqlUnescape ( sRes, m_pBuf + tNode.m_iStart, tNode.m_iEnd - tNode.m_iStart );
	}

	bool			AddSchemaItem ( SqlNode_t * pNode );
	bool			SetMatch ( const SqlNode_t & tValue );
	void			AddConst ( int iList, const SqlNode_t& tValue );
	void			SetStatement ( const SqlNode_t & tName, SqlSet_e eSet );
	bool			AddFloatRangeFilter ( const SqlNode_t & tAttr, float fMin, float fMax, bool bHasEqual, bool bExclude=false );
	bool			AddIntRangeFilter ( const SqlNode_t & tAttr, int64_t iMin, int64_t iMax, bool bExclude );
	bool			AddIntFilterGreater ( const SqlNode_t & tAttr, int64_t iVal, bool bHasEqual );
	bool			AddIntFilterLesser ( const SqlNode_t & tAttr, int64_t iVal, bool bHasEqual );
	bool			AddUservarFilter ( const SqlNode_t & tCol, const SqlNode_t & tVar, bool bExclude );
	void			AddGroupBy ( const SqlNode_t & tGroupBy );
	bool			AddDistinct ( SqlNode_t * pNewExpr, SqlNode_t * pStart, SqlNode_t * pEnd );
	CSphFilterSettings *	AddFilter ( const SqlNode_t & tCol, ESphFilter eType );
	bool					AddStringFilter ( const SqlNode_t & tCol, const SqlNode_t & tVal, bool bExclude );
	CSphFilterSettings *	AddValuesFilter ( const SqlNode_t & tCol ) { return AddFilter ( tCol, SPH_FILTER_VALUES ); }
	bool					AddNullFilter ( const SqlNode_t & tCol, bool bEqualsNull );
	void			AddHaving ();

	inline bool		SetOldSyntax()
	{
		m_uSyntaxFlags |= 1;
		return IsGoodSyntax ();
	}

	inline bool		SetNewSyntax()
	{
		m_uSyntaxFlags |= 2;
		return IsGoodSyntax ();
	}
	bool IsGoodSyntax ();
	inline bool IsDeprecatedSyntax () const
	{
		return m_uSyntaxFlags & 1;
	}

	int							AllocNamedVec ();
	CSphVector<CSphNamedInt> &	GetNamedVec ( int iIndex );
	void						FreeNamedVec ( int iIndex );
	bool						UpdateStatement ( SqlNode_t * pNode );
	bool						DeleteStatement ( SqlNode_t * pNode );

	void						AddUpdatedAttr ( const SqlNode_t & tName, ESphAttr eType ) const;
	void						UpdateMVAAttr ( const SqlNode_t & tName, const SqlNode_t& dValues );
	void						SetGroupbyLimit ( int iLimit );

private:
	void						AutoAlias ( CSphQueryItem & tItem, SqlNode_t * pStart, SqlNode_t * pEnd );
	void						GenericStatement ( SqlNode_t * pNode, SqlStmt_e iStmt );

protected:
	bool						m_bNamedVecBusy;
	CSphVector<CSphNamedInt>	m_dNamedVec;
};

//////////////////////////////////////////////////////////////////////////

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_DECL static int my_lex ( YYSTYPE * lvalp, void * yyscanner, SqlParser_c * pParser )

#if USE_WINDOWS
#define YY_NO_UNISTD_H 1
#endif

#include "llsphinxql.c"


void yyerror ( SqlParser_c * pParser, const char * sMessage )
{
	// flex put a zero at last token boundary; make it undo that
	yylex_unhold ( pParser->m_pScanner );

	// create our error message
	pParser->m_pParseError->SetSprintf ( "sphinxql: %s near '%s'", sMessage,
		pParser->m_pLastTokenStart ? pParser->m_pLastTokenStart : "(null)" );

	// fixup TOK_xxx thingies
	char * s = const_cast<char*> ( pParser->m_pParseError->cstr() );
	char * d = s;
	while ( *s )
	{
		if ( strncmp ( s, "TOK_", 4 )==0 )
			s += 4;
		else
			*d++ = *s++;
	}
	*d = '\0';
}


#ifndef NDEBUG
// using a proxy to be possible to debug inside yylex
static int yylex ( YYSTYPE * lvalp, SqlParser_c * pParser )
{
	int res = my_lex ( lvalp, pParser->m_pScanner, pParser );
	return res;
}
#else
static int yylex ( YYSTYPE * lvalp, SqlParser_c * pParser )
{
	return my_lex ( lvalp, pParser->m_pScanner, pParser );
}
#endif

#include "yysphinxql.c"

//////////////////////////////////////////////////////////////////////////

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
			case TOK_CONST_FLOAT:		return int(tVal.m_fVal); // FIXME? report conversion error?
		}
		return 0;
	}

	inline static SphDocID_t ToDocid ( const SqlInsert_t & tVal )
	{
		// FIXME? report conversion errors?
		SphDocID_t uRes = DOCID_MAX;
		switch ( tVal.m_iType )
		{
			case TOK_QUOTED_STRING :	uRes = (SphDocID_t) strtoull ( tVal.m_sVal.cstr(), NULL, 10 ); break;
			case TOK_CONST_INT:			uRes = (SphDocID_t) tVal.m_iVal; break;
			case TOK_CONST_FLOAT:		uRes = (SphDocID_t) tVal.m_fVal; break;
		}
		if ( uRes==DOCID_MAX )
			uRes = 0;
		return uRes;
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

SqlParser_c::SqlParser_c ( CSphVector<SqlStmt_t> & dStmt, ESphCollation eCollation )
	: m_pQuery ( NULL )
	, m_pStmt ( NULL )
	, m_dStmt ( dStmt )
	, m_eCollation ( eCollation )
	, m_uSyntaxFlags ( 0 )
	, m_bNamedVecBusy ( false )
{
	assert ( !m_dStmt.GetLength() );
	PushQuery ();
}

void SqlParser_c::PushQuery ()
{
	assert ( m_dStmt.GetLength() || ( !m_pQuery && !m_pStmt ) );

	// post set proper result-set order
	if ( m_dStmt.GetLength() )
	{
		if ( m_pQuery->m_sGroupBy.IsEmpty() )
			m_pQuery->m_sSortBy = m_pQuery->m_sOrderBy;
		else
			m_pQuery->m_sGroupSortBy = m_pQuery->m_sOrderBy;
	}

	// add new
	m_dStmt.Add ( SqlStmt_t() );
	m_pStmt = &m_dStmt.Last();
	m_pQuery = &m_pStmt->m_tQuery;
	m_pQuery->m_eCollation = m_eCollation;

	m_bGotQuery = false;
}


bool SqlParser_c::AddOption ( const SqlNode_t & tIdent )
{
	CSphString sOpt, sVal;
	ToString ( sOpt, tIdent ).ToLower();

	if ( sOpt=="low_priority" )
	{
		m_pQuery->m_bLowPriority = true;
	} else if ( sOpt=="debug_no_payload" )
	{
		m_pStmt->m_tQuery.m_uDebugFlags |= QUERY_DEBUG_NO_PAYLOAD;
	} else
	{
		m_pParseError->SetSprintf ( "unknown option '%s'", sOpt.cstr() );
		return false;
	}

	return true;
}


bool SqlParser_c::AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue )
{
	CSphString sOpt, sVal;
	ToString ( sOpt, tIdent ).ToLower();
	ToString ( sVal, tValue ).ToLower().Unquote();

	// OPTIMIZE? hash possible sOpt choices?
	if ( sOpt=="ranker" )
	{
		m_pQuery->m_eRanker = SPH_RANK_TOTAL;
		for ( int iRanker = SPH_RANK_PROXIMITY_BM25; iRanker<=SPH_RANK_SPH04; iRanker++ )
			if ( sVal==sphGetRankerName ( ESphRankMode ( iRanker ) ) )
			{
				m_pQuery->m_eRanker = ESphRankMode ( iRanker );
				break;
			}

		if ( m_pQuery->m_eRanker==SPH_RANK_TOTAL )
		{
			if ( sVal==sphGetRankerName ( SPH_RANK_EXPR ) || sVal==sphGetRankerName ( SPH_RANK_EXPORT ) )
			{
				m_pParseError->SetSprintf ( "missing ranker expression (use OPTION ranker=expr('1+2') for example)" );
				return false;
			} else if ( sphPluginExists ( PLUGIN_RANKER, sVal.cstr() ) )
			{
				m_pQuery->m_eRanker = SPH_RANK_PLUGIN;
				m_pQuery->m_sUDRanker = sVal;
			}
			m_pParseError->SetSprintf ( "unknown ranker '%s'", sVal.cstr() );
			return false;
		}
	} else if ( sOpt=="token_filter" )	// tokfilter = hello.dll:hello:some_opts
	{
		CSphVector<CSphString> dParams;
		if ( !sphPluginParseSpec ( sVal, dParams, *m_pParseError ) )
			return false;

		if ( !dParams.GetLength() )
		{
			m_pParseError->SetSprintf ( "missing token filter spec string" );
			return false;
		}

		m_pQuery->m_sQueryTokenFilterLib = dParams[0];
		m_pQuery->m_sQueryTokenFilterName = dParams[1];
		m_pQuery->m_sQueryTokenFilterOpts = dParams[2];
	} else if ( sOpt=="max_matches" )
	{
		m_pQuery->m_iMaxMatches = (int)tValue.m_iValue;

	} else if ( sOpt=="cutoff" )
	{
		m_pQuery->m_iCutoff = (int)tValue.m_iValue;

	} else if ( sOpt=="max_query_time" )
	{
		m_pQuery->m_uMaxQueryMsec = (int)tValue.m_iValue;

	} else if ( sOpt=="retry_count" )
	{
		m_pQuery->m_iRetryCount = (int)tValue.m_iValue;

	} else if ( sOpt=="retry_delay" )
	{
		m_pQuery->m_iRetryDelay = (int)tValue.m_iValue;

	} else if ( sOpt=="reverse_scan" )
	{
		m_pQuery->m_bReverseScan = ( tValue.m_iValue!=0 );

	} else if ( sOpt=="ignore_nonexistent_columns" )
	{
		m_pQuery->m_bIgnoreNonexistent = ( tValue.m_iValue!=0 );

	} else if ( sOpt=="comment" )
	{
		ToStringUnescape ( m_pQuery->m_sComment, tValue );

	} else if ( sOpt=="sort_method" )
	{
		if ( sVal=="pq" )			m_pQuery->m_bSortKbuffer = false;
		else if ( sVal=="kbuffer" )	m_pQuery->m_bSortKbuffer = true;
		else
		{
			m_pParseError->SetSprintf ( "unknown sort_method=%s (known values are pq, kbuffer)", sVal.cstr() );
			return false;
		}

	} else if ( sOpt=="agent_query_timeout" )
	{
		m_pQuery->m_iAgentQueryTimeout = (int)tValue.m_iValue;

	} else if ( sOpt=="max_predicted_time" )
	{
		m_pQuery->m_iMaxPredictedMsec = int ( tValue.m_iValue > INT_MAX ? INT_MAX : tValue.m_iValue );

	} else if ( sOpt=="boolean_simplify" )
	{
		m_pQuery->m_bSimplify = true;

	} else if ( sOpt=="idf" )
	{
		CSphVector<CSphString> dOpts;
		sphSplit ( dOpts, sVal.cstr() );

		ARRAY_FOREACH ( i, dOpts )
		{
			if ( dOpts[i]=="normalized" )
				m_pQuery->m_bPlainIDF = false;
			else if ( dOpts[i]=="plain" )
				m_pQuery->m_bPlainIDF = true;
			else if ( dOpts[i]=="tfidf_normalized" )
				m_pQuery->m_bNormalizedTFIDF = true;
			else if ( dOpts[i]=="tfidf_unnormalized" )
				m_pQuery->m_bNormalizedTFIDF = false;
			else
			{
				m_pParseError->SetSprintf ( "unknown flag %s in idf=%s (known values are plain, normalized, tfidf_normalized, tfidf_unnormalized)",
					dOpts[i].cstr(), sVal.cstr() );
				return false;
			}
		}
	} else if ( sOpt=="global_idf" )
	{
		m_pQuery->m_bGlobalIDF = ( tValue.m_iValue!=0 );

	} else if ( sOpt=="local_df" )
	{
		m_pQuery->m_bLocalDF = ( tValue.m_iValue!=0 );

	} else if ( sOpt=="ignore_nonexistent_indexes" )
	{
		m_pQuery->m_bIgnoreNonexistentIndexes = ( tValue.m_iValue!=0 );

	} else if ( sOpt=="strict" )
	{
		m_pQuery->m_bStrict = ( tValue.m_iValue!=0 );

	} else if ( sOpt=="columns" ) // for SHOW THREADS
	{
		m_pStmt->m_iThreadsCols = Max ( (int)tValue.m_iValue, 0 );

	} else if ( sOpt=="rand_seed" )
	{
		m_pStmt->m_tQuery.m_iRandSeed = int64_t(DWORD(tValue.m_iValue));

	} else
	{
		m_pParseError->SetSprintf ( "unknown option '%s' (or bad argument type)", sOpt.cstr() );
		return false;
	}

	return true;
}


bool SqlParser_c::AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue, const SqlNode_t & tArg )
{
	CSphString sOpt, sVal;
	ToString ( sOpt, tIdent ).ToLower();
	ToString ( sVal, tValue ).ToLower().Unquote();

	if ( sOpt=="ranker" )
	{
		bool bUDR = sphPluginExists ( PLUGIN_RANKER, sVal.cstr() );
		if ( sVal=="expr" || sVal=="export" || bUDR )
		{
			if ( bUDR )
			{
				m_pQuery->m_eRanker = SPH_RANK_PLUGIN;
				m_pQuery->m_sUDRanker = sVal;
				ToStringUnescape ( m_pQuery->m_sUDRankerOpts, tArg );
			} else
			{
				m_pQuery->m_eRanker = sVal=="expr" ? SPH_RANK_EXPR : SPH_RANK_EXPORT;
				ToStringUnescape ( m_pQuery->m_sRankerExpr, tArg );
			}

			return true;
		}
	}

	m_pParseError->SetSprintf ( "unknown option or extra argument to '%s=%s'", sOpt.cstr(), sVal.cstr() );
	return false;
}


bool SqlParser_c::AddOption ( const SqlNode_t & tIdent, CSphVector<CSphNamedInt> & dNamed )
{
	CSphString sOpt;
	ToString ( sOpt, tIdent ).ToLower ();

	if ( sOpt=="field_weights" )
	{
		m_pQuery->m_dFieldWeights.SwapData ( dNamed );

	} else if ( sOpt=="index_weights" )
	{
		m_pQuery->m_dIndexWeights.SwapData ( dNamed );

	} else
	{
		m_pParseError->SetSprintf ( "unknown option '%s' (or bad argument type)", sOpt.cstr() );
		return false;
	}

	return true;
}


bool SqlParser_c::AddInsertOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue )
{
	CSphString sOpt, sVal;
	ToString ( sOpt, tIdent ).ToLower();
	ToString ( sVal, tValue ).Unquote();

	if ( sOpt=="token_filter_options" )
	{
		m_pStmt->m_sStringParam = sVal;
	} else
	{
		m_pParseError->SetSprintf ( "unknown option '%s' (or bad argument type)", sOpt.cstr() );
		return false;
	}
	return true;
}


void SqlParser_c::AliasLastItem ( SqlNode_t * pAlias )
{
	if ( pAlias )
	{
		CSphQueryItem & tItem = m_pQuery->m_dItems.Last();
		tItem.m_sAlias.SetBinary ( m_pBuf + pAlias->m_iStart, pAlias->m_iEnd - pAlias->m_iStart );
		tItem.m_sAlias.ToLower();
		SetSelect ( pAlias );
	}
}

void SqlParser_c::AutoAlias ( CSphQueryItem & tItem, SqlNode_t * pStart, SqlNode_t * pEnd )
{
	if ( pStart && pEnd )
	{
		tItem.m_sAlias.SetBinary ( m_pBuf + pStart->m_iStart, pEnd->m_iEnd - pStart->m_iStart );
		sphColumnToLowercase ( const_cast<char *>( tItem.m_sAlias.cstr() ) );
	} else
	{
		tItem.m_sAlias = tItem.m_sExpr;
	}
	SetSelect ( pStart, pEnd );
}

void SqlParser_c::AddItem ( SqlNode_t * pExpr, ESphAggrFunc eAggrFunc, SqlNode_t * pStart, SqlNode_t * pEnd )
{
	CSphQueryItem & tItem = m_pQuery->m_dItems.Add();
	tItem.m_sExpr.SetBinary ( m_pBuf + pExpr->m_iStart, pExpr->m_iEnd - pExpr->m_iStart );
	sphColumnToLowercase ( const_cast<char *>( tItem.m_sExpr.cstr() ) );
	tItem.m_eAggrFunc = eAggrFunc;
	AutoAlias ( tItem, pStart?pStart:pExpr, pEnd?pEnd:pExpr );
}

bool SqlParser_c::AddItem ( const char * pToken, SqlNode_t * pStart, SqlNode_t * pEnd )
{
	CSphQueryItem & tItem = m_pQuery->m_dItems.Add();
	tItem.m_sExpr = pToken;
	tItem.m_eAggrFunc = SPH_AGGR_NONE;
	sphColumnToLowercase ( const_cast<char *>( tItem.m_sExpr.cstr() ) );
	AutoAlias ( tItem, pStart, pEnd );
	return SetNewSyntax();
}

bool SqlParser_c::AddCount ()
{
	CSphQueryItem & tItem = m_pQuery->m_dItems.Add();
	tItem.m_sExpr = tItem.m_sAlias = "count(*)";
	tItem.m_eAggrFunc = SPH_AGGR_NONE;
	return SetNewSyntax();
}

void SqlParser_c::AddGroupBy ( const SqlNode_t & tGroupBy )
{
	if ( m_pQuery->m_sGroupBy.IsEmpty() )
	{
		m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
		m_pQuery->m_sGroupBy.SetBinary ( m_pBuf + tGroupBy.m_iStart, tGroupBy.m_iEnd - tGroupBy.m_iStart );
		sphColumnToLowercase ( const_cast<char *>( m_pQuery->m_sGroupBy.cstr() ) );
	} else
	{
		m_pQuery->m_eGroupFunc = SPH_GROUPBY_MULTIPLE;
		CSphString sTmp;
		sTmp.SetBinary ( m_pBuf + tGroupBy.m_iStart, tGroupBy.m_iEnd - tGroupBy.m_iStart );
		sphColumnToLowercase ( const_cast<char *>( sTmp.cstr() ) );
		m_pQuery->m_sGroupBy.SetSprintf ( "%s, %s", m_pQuery->m_sGroupBy.cstr(), sTmp.cstr() );
	}
}

void SqlParser_c::SetGroupbyLimit ( int iLimit )
{
	m_pQuery->m_iGroupbyLimit = iLimit;
}

bool SqlParser_c::AddDistinct ( SqlNode_t * pNewExpr, SqlNode_t * pStart, SqlNode_t * pEnd )
{
	if ( !m_pQuery->m_sGroupDistinct.IsEmpty() )
	{
		yyerror ( this, "too many COUNT(DISTINCT) clauses" );
		return false;
	}

	ToString ( m_pQuery->m_sGroupDistinct, *pNewExpr );
	return AddItem ( "@distinct", pStart, pEnd );
}

bool SqlParser_c::AddSchemaItem ( YYSTYPE * pNode )
{
	assert ( m_pStmt );
	CSphString sItem;
	sItem.SetBinary ( m_pBuf + pNode->m_iStart, pNode->m_iEnd - pNode->m_iStart );
	return m_pStmt->AddSchemaItem ( sItem.cstr() );
}

bool SqlParser_c::SetMatch ( const YYSTYPE& tValue )
{
	if ( m_bGotQuery )
	{
		yyerror ( this, "too many MATCH() clauses" );
		return false;
	};

	ToStringUnescape ( m_pQuery->m_sQuery, tValue );
	m_pQuery->m_sRawQuery = m_pQuery->m_sQuery;
	return m_bGotQuery = true;
}

void SqlParser_c::AddConst ( int iList, const YYSTYPE& tValue )
{
	CSphVector<CSphNamedInt> & dVec = GetNamedVec ( iList );

	dVec.Add();
	ToString ( dVec.Last().m_sName, tValue ).ToLower();
	dVec.Last().m_iValue = (int) tValue.m_iValue;
}

void SqlParser_c::SetStatement ( const YYSTYPE & tName, SqlSet_e eSet )
{
	m_pStmt->m_eStmt = STMT_SET;
	m_pStmt->m_eSet = eSet;
	ToString ( m_pStmt->m_sSetName, tName );
}

void SqlParser_c::GenericStatement ( SqlNode_t * pNode, SqlStmt_e iStmt )
{
	m_pStmt->m_eStmt = iStmt;
	m_pStmt->m_iListStart = pNode->m_iStart;
	m_pStmt->m_iListEnd = pNode->m_iEnd;
	ToString ( m_pStmt->m_sIndex, *pNode );
}

bool SqlParser_c::UpdateStatement ( SqlNode_t * pNode )
{
	GenericStatement ( pNode, STMT_UPDATE );
	m_pStmt->m_tUpdate.m_dRowOffset.Add ( 0 );
	return true;
}

bool SqlParser_c::DeleteStatement ( SqlNode_t * pNode )
{
	GenericStatement ( pNode, STMT_DELETE );
	return true;
}

void SqlParser_c::AddUpdatedAttr ( const SqlNode_t & tName, ESphAttr eType ) const
{
	CSphAttrUpdate & tUpd = m_pStmt->m_tUpdate;
	CSphString sAttr;
	tUpd.m_dAttrs.Add ( ToString ( sAttr, tName ).ToLower().Leak() );
	tUpd.m_dTypes.Add ( eType ); // sorry, ints only for now, riding on legacy shit!
}


void SqlParser_c::UpdateMVAAttr ( const SqlNode_t & tName, const SqlNode_t & dValues )
{
	CSphAttrUpdate & tUpd = m_pStmt->m_tUpdate;
	ESphAttr eType = SPH_ATTR_UINT32SET;

	if ( dValues.m_pValues.Ptr() && dValues.m_pValues->GetLength()>0 )
	{
		// got MVA values, let's process them
		dValues.m_pValues->Uniq(); // don't need dupes within MVA
		tUpd.m_dPool.Add ( dValues.m_pValues->GetLength()*2 );
		SphAttr_t * pVal = dValues.m_pValues.Ptr()->Begin();
		SphAttr_t * pValMax = pVal + dValues.m_pValues->GetLength();
		for ( ;pVal<pValMax; pVal++ )
		{
			SphAttr_t uVal = *pVal;
			if ( uVal>UINT_MAX )
			{
				eType = SPH_ATTR_INT64SET;
			}
			tUpd.m_dPool.Add ( (DWORD)uVal );
			tUpd.m_dPool.Add ( (DWORD)( uVal>>32 ) );
		}
	} else
	{
		// no values, means we should delete the attribute
		// we signal that to the update code by putting a single zero
		// to the values pool (meaning a zero-length MVA values list)
		tUpd.m_dPool.Add ( 0 );
	}

	AddUpdatedAttr ( tName, eType );
}

CSphFilterSettings * SqlParser_c::AddFilter ( const SqlNode_t & tCol, ESphFilter eType )
{
	CSphString sCol;
	ToString ( sCol, tCol ); // do NOT lowercase just yet, might have to retain case for JSON cols

	CSphFilterSettings * pFilter = &m_pQuery->m_dFilters.Add();
	pFilter->m_sAttrName = ( !strcasecmp ( sCol.cstr(), "id" ) ) ? "@id" : sCol;
	pFilter->m_eType = eType;
	sphColumnToLowercase ( const_cast<char *>( pFilter->m_sAttrName.cstr() ) );
	return pFilter;
}

bool SqlParser_c::AddFloatRangeFilter ( const SqlNode_t & sAttr, float fMin, float fMax, bool bHasEqual, bool bExclude )
{
	CSphFilterSettings * pFilter = AddFilter ( sAttr, SPH_FILTER_FLOATRANGE );
	if ( !pFilter )
		return false;
	pFilter->m_fMinValue = fMin;
	pFilter->m_fMaxValue = fMax;
	pFilter->m_bHasEqual = bHasEqual;
	pFilter->m_bExclude = bExclude;
	return true;
}

bool SqlParser_c::AddIntRangeFilter ( const SqlNode_t & sAttr, int64_t iMin, int64_t iMax, bool bExclude )
{
	CSphFilterSettings * pFilter = AddFilter ( sAttr, SPH_FILTER_RANGE );
	if ( !pFilter )
		return false;
	pFilter->m_iMinValue = iMin;
	pFilter->m_iMaxValue = iMax;
	pFilter->m_bExclude = bExclude;
	return true;
}

bool SqlParser_c::AddIntFilterGreater ( const SqlNode_t & tAttr, int64_t iVal, bool bHasEqual )
{
	CSphFilterSettings * pFilter = AddFilter ( tAttr, SPH_FILTER_RANGE );
	if ( !pFilter )
		return false;
	bool bId = ( pFilter->m_sAttrName=="@id" ) || ( pFilter->m_sAttrName=="id" );
	pFilter->m_iMaxValue = bId ? (SphAttr_t)ULLONG_MAX : LLONG_MAX;
	if ( iVal==LLONG_MAX )
	{
		pFilter->m_iMinValue = iVal;
		pFilter->m_bHasEqual = bHasEqual;
	} else
	{
		pFilter->m_iMinValue = bHasEqual ? iVal : iVal+1;
		pFilter->m_bHasEqual = true;
	}
	return true;
}

bool SqlParser_c::AddIntFilterLesser ( const SqlNode_t & tAttr, int64_t iVal, bool bHasEqual )
{
	CSphFilterSettings * pFilter = AddFilter ( tAttr, SPH_FILTER_RANGE );
	if ( !pFilter )
		return false;
	bool bId = ( pFilter->m_sAttrName=="@id" ) || ( pFilter->m_sAttrName=="id" );
	pFilter->m_iMinValue = bId ? 0 : LLONG_MIN;
	if ( iVal==LLONG_MIN )
	{
		pFilter->m_iMaxValue = iVal;
		pFilter->m_bHasEqual = bHasEqual;
	} else
	{
		pFilter->m_iMaxValue = bHasEqual ? iVal : iVal-1;
		pFilter->m_bHasEqual = true;
	}
	return true;
}

bool SqlParser_c::AddUservarFilter ( const SqlNode_t & tCol, const SqlNode_t & tVar, bool bExclude )
{
	CSphFilterSettings * pFilter = AddFilter ( tCol, SPH_FILTER_USERVAR );
	if ( !pFilter )
		return false;
	ToString ( pFilter->m_sRefString, tVar ).ToLower();
	pFilter->m_bExclude = bExclude;
	return true;
}


bool SqlParser_c::AddStringFilter ( const SqlNode_t & tCol, const SqlNode_t & tVal, bool bExclude )
{
	CSphFilterSettings * pFilter = AddFilter ( tCol, SPH_FILTER_STRING );
	if ( !pFilter )
		return false;
	ToStringUnescape ( pFilter->m_sRefString, tVal );
	pFilter->m_bExclude = bExclude;
	return true;
}


bool SqlParser_c::AddNullFilter ( const SqlNode_t & tCol, bool bEqualsNull )
{
	CSphFilterSettings * pFilter = AddFilter ( tCol, SPH_FILTER_NULL );
	if ( !pFilter )
		return false;
	pFilter->m_bHasEqual = bEqualsNull;
	return true;
}

void SqlParser_c::AddHaving ()
{
	assert ( m_pQuery->m_dFilters.GetLength() );
	m_pQuery->m_tHaving = m_pQuery->m_dFilters.Pop();
}


bool SqlParser_c::IsGoodSyntax ()
{
	if ( ( m_uSyntaxFlags & 3 )!=3 )
		return true;
	yyerror ( this, "Mixing the old-fashion internal vars (@id, @count, @weight) with new acronyms like count(*), weight() is prohibited" );
	return false;
}


int SqlParser_c::AllocNamedVec ()
{
	// we only allow one such vector at a time, right now
	assert ( !m_bNamedVecBusy );
	m_bNamedVecBusy = true;
	m_dNamedVec.Resize ( 0 );
	return 0;
}

#ifndef NDEBUG
CSphVector<CSphNamedInt> & SqlParser_c::GetNamedVec ( int iIndex )
#else
CSphVector<CSphNamedInt> & SqlParser_c::GetNamedVec ( int )
#endif
{
	assert ( m_bNamedVecBusy && iIndex==0 );
	return m_dNamedVec;
}

#ifndef NDEBUG
void SqlParser_c::FreeNamedVec ( int iIndex )
#else
void SqlParser_c::FreeNamedVec ( int )
#endif
{
	assert ( m_bNamedVecBusy && iIndex==0 );
	m_bNamedVecBusy = false;
	m_dNamedVec.Resize ( 0 );
}

bool sphParseSqlQuery ( const char * sQuery, int iLen, CSphVector<SqlStmt_t> & dStmt, CSphString & sError, ESphCollation eCollation )
{
	assert ( sQuery );

	SqlParser_c tParser ( dStmt, eCollation );
	tParser.m_pBuf = sQuery;
	tParser.m_pLastTokenStart = NULL;
	tParser.m_pParseError = &sError;
	tParser.m_eCollation = eCollation;

	char * sEnd = const_cast<char *>( sQuery ) + iLen;
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	yylex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy_scan_buffer ( const_cast<char *>( sQuery ), iLen+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: yy_scan_buffer() failed";
		return false;
	}

	int iRes = yyparse ( &tParser );
	yy_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yylex_destroy ( tParser.m_pScanner );

	dStmt.Pop(); // last query is always dummy

	ARRAY_FOREACH ( i, dStmt )
	{
		// select expressions will be reparsed again, by an expression parser,
		// when we have an index to actually bind variables, and create a tree
		//
		// so at SQL parse stage, we only do quick validation, and at this point,
		// we just store the select list for later use by the expression parser
		CSphQuery & tQuery = dStmt[i].m_tQuery;
		if ( tQuery.m_iSQLSelectStart>=0 )
		{
			if ( tQuery.m_iSQLSelectStart-1>=0 && tParser.m_pBuf[tQuery.m_iSQLSelectStart-1]=='`' )
				tQuery.m_iSQLSelectStart--;
			if ( tQuery.m_iSQLSelectEnd<iLen && tParser.m_pBuf[tQuery.m_iSQLSelectEnd]=='`' )
				tQuery.m_iSQLSelectEnd++;

			tQuery.m_sSelect.SetBinary ( tParser.m_pBuf + tQuery.m_iSQLSelectStart,
				tQuery.m_iSQLSelectEnd - tQuery.m_iSQLSelectStart );
		}

		// validate tablefuncs
		// tablefuncs are searchd-level builtins rather than common expression-level functions
		// so validation happens here, expression parser does not know tablefuncs (ignorance is bliss)
		if ( dStmt[i].m_eStmt==STMT_SELECT && !dStmt[i].m_sTableFunc.IsEmpty() )
		{
			CSphString & sFunc = dStmt[i].m_sTableFunc;
			sFunc.ToUpper();

			ISphTableFunc * pFunc = NULL;
			if ( sFunc=="REMOVE_REPEATS" )
				pFunc = new CSphTableFuncRemoveRepeats();

			if ( !pFunc )
			{
				sError.SetSprintf ( "unknown table function %s()", sFunc.cstr() );
				return false;
			}
			if ( !pFunc->ValidateArgs ( dStmt[i].m_dTableFuncArgs, tQuery, sError ) )
			{
				SafeDelete ( pFunc );
				return false;
			}
			tQuery.m_pTableFunc = pFunc;
		}

		// validate filters
		ARRAY_FOREACH ( i, tQuery.m_dFilters )
		{
			const CSphString & sCol = tQuery.m_dFilters[i].m_sAttrName;
			if ( !strcasecmp ( sCol.cstr(), "@count" ) || !strcasecmp ( sCol.cstr(), "count(*)" ) )
			{
				sError.SetSprintf ( "sphinxql: Aggregates in 'where' clause prohibited, use 'having'" );
				return false;
			}
		}
	}

	if ( iRes!=0 || !dStmt.GetLength() )
		return false;

	if ( tParser.IsDeprecatedSyntax() )
	{
		sError = "Using the old-fashion @variables (@count, @weight, etc.) is deprecated";
		return false;
	}

	// facets
	ARRAY_FOREACH ( i, dStmt )
	{
		CSphQuery & tQuery = dStmt[i].m_tQuery;
		if ( dStmt[i].m_eStmt==STMT_SELECT )
		{
			while ( i+1<dStmt.GetLength() )
			{
				SqlStmt_t & tStmt = dStmt[i+1];
				if ( tStmt.m_eStmt!=STMT_FACET )
					break;

				tStmt.m_tQuery.m_bFacet = true;

				tStmt.m_eStmt = STMT_SELECT;
				tStmt.m_tQuery.m_sIndexes = tQuery.m_sIndexes;
				tStmt.m_tQuery.m_sSelect = tStmt.m_tQuery.m_sFacetBy;
				tStmt.m_tQuery.m_sQuery = tQuery.m_sQuery;

				// append top-level expressions to a facet schema (for filtering)
				ARRAY_FOREACH ( k, tQuery.m_dItems )
					if ( tQuery.m_dItems[k].m_sAlias!=tQuery.m_dItems[k].m_sExpr )
						tStmt.m_tQuery.m_dItems.Add ( tQuery.m_dItems[k] );

				// append filters
				ARRAY_FOREACH ( k, tQuery.m_dFilters )
					tStmt.m_tQuery.m_dFilters.Add ( tQuery.m_dFilters[k] );

				i++;
			}
		}
	}

	return true;
}


/////////////////////////////////////////////////////////////////////////////

ESphSpz sphGetPassageBoundary ( const CSphString & sPassageBoundaryMode )
{
	if ( sPassageBoundaryMode.IsEmpty() )
		return SPH_SPZ_NONE;

	ESphSpz eSPZ = SPH_SPZ_NONE;
	if ( sPassageBoundaryMode=="sentence" )
		eSPZ = SPH_SPZ_SENTENCE;
	else if ( sPassageBoundaryMode=="paragraph" )
		eSPZ = SPH_SPZ_PARAGRAPH;
	else if ( sPassageBoundaryMode=="zone" )
		eSPZ = SPH_SPZ_ZONE;

	return eSPZ;
}

bool sphCheckOptionsSPZ ( const ExcerptQuery_t & q, const CSphString & sPassageBoundaryMode, CSphString & sError )
{
	if ( q.m_ePassageSPZ )
	{
		if ( q.m_iAround==0 )
		{
			sError.SetSprintf ( "invalid combination of passage_boundary=%s and around=%d", sPassageBoundaryMode.cstr(), q.m_iAround );
			return false;
		} else if ( q.m_bUseBoundaries )
		{
			sError.SetSprintf ( "invalid combination of passage_boundary=%s and use_boundaries", sPassageBoundaryMode.cstr() );
			return false;
		}
	}

	if ( q.m_bEmitZones )
	{
		if ( q.m_ePassageSPZ!=SPH_SPZ_ZONE )
		{
			sError.SetSprintf ( "invalid combination of passage_boundary=%s and emit_zones", sPassageBoundaryMode.cstr() );
			return false;
		}
		if ( !( q.m_sStripMode=="strip" || q.m_sStripMode=="index" ) )
		{
			sError.SetSprintf ( "invalid combination of strip=%s and emit_zones", q.m_sStripMode.cstr() );
			return false;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// EXCERPTS HANDLER
/////////////////////////////////////////////////////////////////////////////

enum eExcerpt_Flags
{
	EXCERPT_FLAG_REMOVESPACES		= 1,
	EXCERPT_FLAG_EXACTPHRASE		= 2,
	EXCERPT_FLAG_SINGLEPASSAGE		= 4,
	EXCERPT_FLAG_USEBOUNDARIES		= 8,
	EXCERPT_FLAG_WEIGHTORDER		= 16,
	EXCERPT_FLAG_QUERY				= 32,
	EXCERPT_FLAG_FORCE_ALL_WORDS	= 64,
	EXCERPT_FLAG_LOAD_FILES			= 128,
	EXCERPT_FLAG_ALLOW_EMPTY		= 256,
	EXCERPT_FLAG_EMIT_ZONES			= 512,
	EXCERPT_FLAG_FILES_SCATTERED	= 1024
};

enum
{
	PROCESSED_ITEM					= -2,
	EOF_ITEM						= -1
};
struct SnippetWorker_t
{
	int64_t						m_iTotal;
	int							m_iHead;

	SnippetWorker_t()
		: m_iTotal ( 0 )
		, m_iHead ( EOF_ITEM )
	{}
};

struct SnippetsRemote_t : ISphNoncopyable
{
	CSphVector<AgentConn_t>			m_dAgents;
	CSphVector<SnippetWorker_t>		m_dWorkers;
	CSphVector<ExcerptQuery_t> &	m_dQueries;
	int								m_iAgentConnectTimeout;
	int								m_iAgentQueryTimeout;

	explicit SnippetsRemote_t ( CSphVector<ExcerptQuery_t> & dQueries )
		: m_dQueries ( dQueries )
		, m_iAgentConnectTimeout ( 0 )
		, m_iAgentQueryTimeout ( 0 )
	{}
};

struct SnippetThread_t
{
	SphThread_t					m_tThd;
	CSphMutex *					m_pLock;
	int							m_iQueries;
	ExcerptQuery_t *			m_pQueries;
	volatile int *				m_pCurQuery;
	CSphIndex *					m_pIndex;
	CrashQuery_t				m_tCrashQuery;

	SnippetThread_t()
		: m_pLock ( NULL )
		, m_iQueries ( 0 )
		, m_pQueries ( NULL )
		, m_pCurQuery ( NULL )
		, m_pIndex ( NULL )
	{}
};


struct SnippetRequestBuilder_t : public IRequestBuilder_t
{
	explicit SnippetRequestBuilder_t ( const SnippetsRemote_t * pWorker )
		: m_pWorker ( pWorker )
		, m_iNumDocs ( -1 )
		, m_iReqLen ( -1 )
		, m_bScattered ( false )
		, m_iWorker ( 0 )
	{
	}
	virtual void BuildRequest ( AgentConn_t & tAgent, NetOutputBuffer_c & tOut ) const;

private:
	const SnippetsRemote_t * m_pWorker;
	mutable int m_iNumDocs;		///< optimize numdocs/length calculation in scattered case
	mutable int m_iReqLen;
	mutable bool m_bScattered;
	mutable int m_iWorker;
	mutable CSphMutex m_tWorkerMutex;
};


struct SnippetReplyParser_t : public IReplyParser_t
{
	explicit SnippetReplyParser_t ( SnippetsRemote_t * pWorker )
		: m_pWorker ( pWorker )
	{}

	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const;

private:
	const SnippetsRemote_t * m_pWorker;
};


void SnippetRequestBuilder_t::BuildRequest ( AgentConn_t & tAgent, NetOutputBuffer_c & tOut ) const
{
	// it sends either all queries to each agent or sequence of queries to current agent
	m_tWorkerMutex.Lock();
	int iWorker = m_iWorker++;
	m_tWorkerMutex.Unlock();

	const CSphVector<ExcerptQuery_t> & dQueries = m_pWorker->m_dQueries;
	const ExcerptQuery_t & q = dQueries[0];
	const SnippetWorker_t & tWorker = m_pWorker->m_dWorkers[iWorker];
	tAgent.m_iWorkerTag = iWorker;

	const char* sIndex = tAgent.m_sIndexes.cstr();

	if ( m_iNumDocs < 0 )
		m_bScattered = ( q.m_iLoadFiles & 2 )!=0;

	if ( !m_bScattered || ( m_bScattered && m_iNumDocs<0 ) )
	{
		m_iReqLen = 60 // 15 ints/dwords - params, strlens, etc.
		+ strlen ( sIndex )
		+ q.m_sWords.Length()
		+ q.m_sBeforeMatch.Length()
		+ q.m_sAfterMatch.Length()
		+ q.m_sChunkSeparator.Length()
		+ q.m_sStripMode.Length()
		+ q.m_sRawPassageBoundary.Length();

		m_iNumDocs = 0;
		for ( int iDoc = tWorker.m_iHead; iDoc!=EOF_ITEM; iDoc=dQueries[iDoc].m_iNext )
		{
			++m_iNumDocs;
			m_iReqLen += 4 + dQueries[iDoc].m_sSource.Length();
		}
	}

	tOut.SendWord ( SEARCHD_COMMAND_EXCERPT );
	tOut.SendWord ( VER_COMMAND_EXCERPT );

	tOut.SendInt ( m_iReqLen );

	tOut.SendInt ( 0 );

	if ( m_bScattered )
		tOut.SendInt ( q.m_iRawFlags & ~EXCERPT_FLAG_LOAD_FILES );
	else
		tOut.SendInt ( q.m_iRawFlags );

	tOut.SendString ( sIndex );
	tOut.SendString ( q.m_sWords.cstr() );
	tOut.SendString ( q.m_sBeforeMatch.cstr() );
	tOut.SendString ( q.m_sAfterMatch.cstr() );
	tOut.SendString ( q.m_sChunkSeparator.cstr() );
	tOut.SendInt ( q.m_iLimit );
	tOut.SendInt ( q.m_iAround );

	tOut.SendInt ( q.m_iLimitPassages );
	tOut.SendInt ( q.m_iLimitWords );
	tOut.SendInt ( q.m_iPassageId );
	tOut.SendString ( q.m_sStripMode.cstr() );
	tOut.SendString ( q.m_sRawPassageBoundary.cstr() );

	tOut.SendInt ( m_iNumDocs );
	for ( int iDoc = tWorker.m_iHead; iDoc!=EOF_ITEM; iDoc=dQueries[iDoc].m_iNext )
		tOut.SendString ( dQueries[iDoc].m_sSource.cstr() );
}

bool SnippetReplyParser_t::ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const
{
	int iWorker = tAgent.m_iWorkerTag;
	CSphVector<ExcerptQuery_t> & dQueries = m_pWorker->m_dQueries;
	const SnippetWorker_t & tWorker = m_pWorker->m_dWorkers[iWorker];

	int iDoc = tWorker.m_iHead;
	bool bOk = true;
	while ( iDoc!=EOF_ITEM )
	{
		if ( ( dQueries[iDoc].m_iLoadFiles&2 )!=0 ) // NOLINT
		{
			if ( !tReq.GetString ( dQueries[iDoc].m_dRes ) )
			{
				bOk = false;
				dQueries[iDoc].m_dRes.Resize ( 0 );
			} else
				dQueries[iDoc].m_sError = "";

			iDoc = dQueries[iDoc].m_iNext;
			continue;
		}
		tReq.GetString ( dQueries[iDoc].m_dRes );
		int iNextDoc = dQueries[iDoc].m_iNext;
		dQueries[iDoc].m_iNext = PROCESSED_ITEM;
		iDoc = iNextDoc;
	}

	return bOk;
}


void SnippetThreadFunc ( void * pArg )
{
	SnippetThread_t * pDesc = (SnippetThread_t*) pArg;

	SphCrashLogger_c::SetLastQuery ( pDesc->m_tCrashQuery );

	SnippetContext_t tCtx;
	tCtx.Setup ( pDesc->m_pIndex, *pDesc->m_pQueries, pDesc->m_pQueries->m_sError );

	for ( ;; )
	{
		pDesc->m_pLock->Lock();
		if ( *pDesc->m_pCurQuery==pDesc->m_iQueries )
		{
			pDesc->m_pLock->Unlock();
			return;
		}

		ExcerptQuery_t * pQuery = pDesc->m_pQueries + (*pDesc->m_pCurQuery);
		(*pDesc->m_pCurQuery)++;
		bool bDone = ( *pDesc->m_pCurQuery==pDesc->m_iQueries );
		pDesc->m_pLock->Unlock();

		if ( pQuery->m_iNext!=PROCESSED_ITEM )
			continue;

		sphBuildExcerpt ( *pQuery, pDesc->m_pIndex, tCtx.m_tStripper.Ptr(), tCtx.m_tExtQuery, tCtx.m_eExtQuerySPZ,
			pQuery->m_sWarning, pQuery->m_sError, tCtx.m_pDict, tCtx.m_tTokenizer.Ptr(), tCtx.m_pQueryTokenizer );

		if ( bDone )
			return;
	}
}

int GetRawSnippetFlags ( const ExcerptQuery_t& q )
{
	int iRawFlags = 0;

	iRawFlags |= q.m_bRemoveSpaces ? EXCERPT_FLAG_REMOVESPACES : 0;
	iRawFlags |= q.m_bUseBoundaries ? EXCERPT_FLAG_USEBOUNDARIES : 0;
	iRawFlags |= q.m_bWeightOrder ? EXCERPT_FLAG_WEIGHTORDER : 0;
	iRawFlags |= q.m_bHighlightQuery ? EXCERPT_FLAG_QUERY : 0;
	iRawFlags |= q.m_bForceAllWords ? EXCERPT_FLAG_FORCE_ALL_WORDS : 0;
	iRawFlags |= q.m_iLimitPassages ? EXCERPT_FLAG_SINGLEPASSAGE : 0;
	iRawFlags |= ( q.m_iLoadFiles & 1 ) ? EXCERPT_FLAG_LOAD_FILES : 0;
	iRawFlags |= ( q.m_iLoadFiles & 2 ) ? EXCERPT_FLAG_FILES_SCATTERED : 0;
	iRawFlags |= q.m_bAllowEmpty ? EXCERPT_FLAG_ALLOW_EMPTY : 0;
	iRawFlags |= q.m_bEmitZones ? EXCERPT_FLAG_EMIT_ZONES : 0;

	return iRawFlags;
}

bool SnippetFormatErrorMessage ( CSphString * pError, const CSphString & sQueryError )
{
	assert ( pError );
	if ( sQueryError.IsEmpty() )
		return false;

	if ( pError->IsEmpty() )
		pError->SetSprintf ( "%s", sQueryError.cstr() );
	else
		pError->SetSprintf ( "%s; %s", pError->cstr(), sQueryError.cstr() );

	return true;
}

bool MakeSnippets ( CSphString sIndex, CSphVector<ExcerptQuery_t> & dQueries, CSphString & sError, ThdDesc_t * pThd )
{
	SnippetsRemote_t dRemoteSnippets ( dQueries );
	CSphVector<CSphString> dDistLocal;
	ExcerptQuery_t & q = dQueries[0];

	g_tDistLock.Lock();
	DistributedIndex_t * pDist = g_hDistIndexes ( sIndex );
	bool bRemote = ( pDist!=NULL );

	// hack! load_files && load_files_scattered is the 'final' call. It will report the absent files as errors.
	// simple load_files_scattered without load_files just omits the absent files (returns empty strings).
	bool bScattered = ( q.m_iLoadFiles & 2 )!=0;
	bool bSkipAbsentFiles = !( q.m_iLoadFiles & 1 );

	if ( bRemote )
	{
		dRemoteSnippets.m_iAgentConnectTimeout = pDist->m_iAgentConnectTimeout;
		dRemoteSnippets.m_iAgentQueryTimeout = pDist->m_iAgentQueryTimeout;
		dDistLocal = pDist->m_dLocal;
		dRemoteSnippets.m_dAgents.Resize ( pDist->m_dAgents.GetLength() );
		ARRAY_FOREACH ( i, pDist->m_dAgents )
			dRemoteSnippets.m_dAgents[i].TakeTraits ( *pDist->m_dAgents[i].GetRRAgent() );
	}
	g_tDistLock.Unlock();

	if ( bRemote )
	{
		if ( dDistLocal.GetLength()!=1 )
		{
			sError.SetSprintf ( "%s", "The distributed index for snippets must have exactly one local agent" );
			return false;
		}

		if ( !q.m_iLoadFiles )
		{
			sError.SetSprintf ( "%s", "The distributed index for snippets available only when using external files" );
			return false;
		}

		if ( g_iDistThreads<=1 && bScattered )
		{
			sError.SetSprintf ( "%s", "load_files_scattered works only together with dist_threads>1" );
			return false;
		}
		sIndex = dDistLocal[0];

		// no remote - roll back to simple local query
		if ( dRemoteSnippets.m_dAgents.GetLength()==0 )
			bRemote = false;
	}

	const ServedIndex_c * pServed = g_pLocalIndexes->GetRlockedEntry ( sIndex );

	if ( !pServed || !pServed->m_bEnabled || !pServed->m_pIndex )
	{
		if ( pServed )
			pServed->Unlock();
		pServed = g_pTemplateIndexes->GetRlockedEntry ( sIndex );
		if ( !pServed || !pServed->m_bEnabled || !pServed->m_pIndex )
		{
			sError.SetSprintf ( "unknown local index '%s' in search request", sIndex.cstr() );
			if ( pServed )
				pServed->Unlock();
			return false;
		}
	}

	CSphIndex * pIndex = pServed->m_pIndex;

	SnippetContext_t tCtx;
	if ( !tCtx.Setup ( pIndex, q, sError ) ) // same path for single - threaded snippets, bail out here on error
	{
		sError.SetSprintf ( "%s", sError.cstr() );
		pServed->Unlock();
		return false;
	}

	///////////////////
	// do highlighting
	///////////////////

	bool bOk = true;
	int iAbsentHead = EOF_ITEM;
	if ( g_iDistThreads<=1 || dQueries.GetLength()<2 )
	{
		// boring single threaded loop
		ARRAY_FOREACH ( i, dQueries )
		{
			sphBuildExcerpt ( dQueries[i], pIndex, tCtx.m_tStripper.Ptr(), tCtx.m_tExtQuery, tCtx.m_eExtQuerySPZ,
				dQueries[i].m_sWarning, dQueries[i].m_sError, tCtx.m_pDict, tCtx.m_tTokenizer.Ptr(), tCtx.m_pQueryTokenizer );
			bOk = ( bOk && ( !SnippetFormatErrorMessage ( &sError, dQueries[i].m_sError ) ) );
		}
	} else
	{
		// get file sizes
		ARRAY_FOREACH ( i, dQueries )
		{
			dQueries[i].m_iNext = PROCESSED_ITEM;
			if ( dQueries[i].m_iLoadFiles )
			{
				struct stat st;
				CSphString sFilename;
				sFilename.SetSprintf ( "%s%s", g_sSnippetsFilePrefix.cstr(), dQueries[i].m_sSource.cstr() );
				if ( ::stat ( sFilename.cstr(), &st )<0 )
				{
					if ( !bScattered )
					{
						sError.SetSprintf ( "failed to stat %s: %s", dQueries[i].m_sSource.cstr(), strerror(errno) );
						pServed->Unlock();
						return false;
					}
					dQueries[i].m_iNext = EOF_ITEM;
				} else
				{
					assert ( st.st_size>0 );
					dQueries[i].m_iSize = -st.st_size; // so that sort would put bigger ones first
				}
			} else
			{
				dQueries[i].m_iSize = -dQueries[i].m_sSource.Length();
			}
			dQueries[i].m_iSeq = i;
		}

		// set correct data size for snippets
		pThd->SetSnippetThreadInfo ( dQueries );

		// tough jobs first
		if ( !bScattered )
			dQueries.Sort ( bind ( &ExcerptQuery_t::m_iSize ) );

		ARRAY_FOREACH ( i, dQueries )
			if ( dQueries[i].m_iNext==EOF_ITEM )
			{
				dQueries[i].m_iNext = iAbsentHead;
				iAbsentHead = i;
				if ( !bSkipAbsentFiles )
					dQueries[i].m_sError.SetSprintf ( "failed to stat %s: %s", dQueries[i].m_sSource.cstr(), strerror(errno) );
			}



		// check if all files are available locally.
		if ( bScattered && iAbsentHead==EOF_ITEM )
		{
			bRemote = false;
			dRemoteSnippets.m_dAgents.Reset();
		}

		if ( bRemote )
		{
			// schedule jobs across workers (the worker is remote agent).
			// simple LPT (Least Processing Time) scheduling for now
			// might add dynamic programming or something later if needed
			int iRemoteAgents = dRemoteSnippets.m_dAgents.GetLength();
			dRemoteSnippets.m_dWorkers.Resize ( iRemoteAgents );

			if ( bScattered )
			{
				// on scattered case - the queries with m_iNext==PROCESSED_ITEM are here, and has to be scheduled to local agent
				// the rest has to be sent to remotes, all of them!
				for ( int i=0; i<iRemoteAgents; i++ )
					dRemoteSnippets.m_dWorkers[i].m_iHead = iAbsentHead;
			} else
			{
				ARRAY_FOREACH ( i, dQueries )
				{
					dRemoteSnippets.m_dWorkers[0].m_iTotal -= dQueries[i].m_iSize;
					// queries sheduled for local still have iNext==-2
					dQueries[i].m_iNext = dRemoteSnippets.m_dWorkers[0].m_iHead;
					dRemoteSnippets.m_dWorkers[0].m_iHead = i;

					dRemoteSnippets.m_dWorkers.Sort ( bind ( &SnippetWorker_t::m_iTotal ) );
				}
			}
		}

		// do MT searching
		CSphMutex tLock;

		CrashQuery_t tCrashQuery = SphCrashLogger_c::GetQuery(); // transfer query info for crash logger to new thread
		int iCurQuery = 0;
		CSphVector<SnippetThread_t> dThreads ( g_iDistThreads );
		for ( int i=0; i<g_iDistThreads; i++ )
		{
			SnippetThread_t & t = dThreads[i];
			t.m_pLock = &tLock;
			t.m_iQueries = dQueries.GetLength();
			t.m_pQueries = dQueries.Begin();
			t.m_pCurQuery = &iCurQuery;
			t.m_pIndex = pIndex;
			t.m_tCrashQuery = tCrashQuery;
			if ( i )
				SphCrashLogger_c::ThreadCreate ( &dThreads[i].m_tThd, SnippetThreadFunc, &dThreads[i] );
		}

		CSphScopedPtr<SnippetRequestBuilder_t> tReqBuilder ( NULL );
		CSphScopedPtr<ISphRemoteAgentsController> tDistCtrl ( NULL );
		if ( bRemote && dRemoteSnippets.m_dAgents.GetLength() )
		{
			// connect to remote agents and query them
			tReqBuilder = new SnippetRequestBuilder_t ( &dRemoteSnippets );
			tDistCtrl = GetAgentsController ( g_iDistThreads, dRemoteSnippets.m_dAgents,
				*tReqBuilder.Ptr(), dRemoteSnippets.m_iAgentConnectTimeout );
		}

		SnippetThreadFunc ( &dThreads[0] );

		int iAgentsDone = 0;
		if ( bRemote && dRemoteSnippets.m_dAgents.GetLength() )
		{
			iAgentsDone = tDistCtrl->Finish();
		}

		int iSuccesses = 0;
		if ( iAgentsDone )
		{
			SnippetReplyParser_t tParser ( &dRemoteSnippets );
			iSuccesses = RemoteWaitForAgents ( dRemoteSnippets.m_dAgents, dRemoteSnippets.m_iAgentQueryTimeout, tParser ); // FIXME? profile update time too?
		}

		for ( int i=1; i<dThreads.GetLength(); i++ )
			sphThreadJoin ( &dThreads[i].m_tThd );

		if ( iSuccesses!=dRemoteSnippets.m_dAgents.GetLength() )
		{
			sphWarning ( "Remote snippets: some of the agents didn't answered: %d queried, %d available, %d answered",
				dRemoteSnippets.m_dAgents.GetLength(),
				iAgentsDone,
				iSuccesses );

			if ( !bScattered )
			{
				// inverse the success/failed state - so that the queries with negative m_iNext are treated as failed
				ARRAY_FOREACH ( i, dQueries )
					dQueries[i].m_iNext = (dQueries[i].m_iNext==PROCESSED_ITEM)?0:PROCESSED_ITEM;

				// failsafe - one more turn for failed queries on local agent
				SnippetThread_t & t = dThreads[0];
				t.m_pQueries = dQueries.Begin();
				iCurQuery = 0;
				SnippetThreadFunc ( &dThreads[0] );
			}
		}

		// back in query order
		dQueries.Sort ( bind ( &ExcerptQuery_t::m_iSeq ) );

		ARRAY_FOREACH ( i, dQueries )
		{
			bOk = ( bOk && ( !SnippetFormatErrorMessage ( &sError, dQueries[i].m_sError ) ) );
		}
	}

	pServed->Unlock();
	return bOk;
}


void HandleCommandExcerpt ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq, ThdDesc_t * pThd )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_EXCERPT, tOut ) )
		return;

	/////////////////////////////
	// parse and process request
	/////////////////////////////

	const int EXCERPT_MAX_ENTRIES			= 1024;

	// v.1.1
	ExcerptQuery_t q;

	tReq.GetInt (); // mode field is for now reserved and ignored
	int iFlags = tReq.GetInt ();
	q.m_iRawFlags = iFlags;
	CSphString sIndex = tReq.GetString ();

	q.m_sWords = tReq.GetString ();
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

	q.m_bHasBeforePassageMacro = SnippetTransformPassageMacros ( q.m_sBeforeMatch, q.m_sBeforeMatchPassage );
	q.m_bHasAfterPassageMacro = SnippetTransformPassageMacros ( q.m_sAfterMatch, q.m_sAfterMatchPassage );

	CSphString sPassageBoundaryMode;
	if ( iVer>=0x103 )
		q.m_sRawPassageBoundary = tReq.GetString();

	q.m_bRemoveSpaces = ( iFlags & EXCERPT_FLAG_REMOVESPACES )!=0;
	q.m_bExactPhrase = ( iFlags & EXCERPT_FLAG_EXACTPHRASE )!=0;
	q.m_bUseBoundaries = ( iFlags & EXCERPT_FLAG_USEBOUNDARIES )!=0;
	q.m_bWeightOrder = ( iFlags & EXCERPT_FLAG_WEIGHTORDER )!=0;
	q.m_bHighlightQuery = ( iFlags & EXCERPT_FLAG_QUERY )!=0;
	q.m_bForceAllWords = ( iFlags & EXCERPT_FLAG_FORCE_ALL_WORDS )!=0;
	if ( iFlags & EXCERPT_FLAG_SINGLEPASSAGE )
		q.m_iLimitPassages = 1;
	q.m_iLoadFiles = (( iFlags & EXCERPT_FLAG_LOAD_FILES )!=0)?1:0;
	bool bScattered = ( iFlags & EXCERPT_FLAG_FILES_SCATTERED )!=0;
	q.m_iLoadFiles |= bScattered?2:0;
	if ( q.m_iLoadFiles )
		q.m_sFilePrefix = g_sSnippetsFilePrefix;
	q.m_bAllowEmpty = ( iFlags & EXCERPT_FLAG_ALLOW_EMPTY )!=0;
	q.m_bEmitZones = ( iFlags & EXCERPT_FLAG_EMIT_ZONES )!=0;

	int iCount = tReq.GetInt ();
	if ( iCount<=0 || iCount>EXCERPT_MAX_ENTRIES )
	{
		SendErrorReply ( tOut, "invalid entries count %d", iCount );
		return;
	}

	q.m_ePassageSPZ = sphGetPassageBoundary ( q.m_sRawPassageBoundary );

	CSphString sError;

	if ( !sphCheckOptionsSPZ ( q, q.m_sRawPassageBoundary, sError ) )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return;
	}

	CSphVector<ExcerptQuery_t> dQueries ( iCount );

	ARRAY_FOREACH ( i, dQueries )
	{
		dQueries[i] = q; // copy settings
		dQueries[i].m_sSource = tReq.GetString (); // fetch data
		if ( tReq.GetError() )
		{
			SendErrorReply ( tOut, "invalid or truncated request" );
			return;
		}
	}

	pThd->SetSnippetThreadInfo ( dQueries );

	if ( !MakeSnippets ( sIndex, dQueries, sError, pThd ) )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return;
	}

	////////////////
	// serve result
	////////////////

	int iRespLen = 0;
	ARRAY_FOREACH ( i, dQueries )
	{
		// handle errors
		if ( !dQueries[i].m_dRes.GetLength() )
		{
			if ( !bScattered )
			{
				SendErrorReply ( tOut, "highlighting failed: %s", dQueries[i].m_sError.cstr() );
				return;
			}
			iRespLen += 4;
		} else
			iRespLen += 4 + strlen ( (const char *)dQueries[i].m_dRes.Begin() );
	}

	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_EXCERPT );
	tOut.SendInt ( iRespLen );
	ARRAY_FOREACH ( i, dQueries )
	{
		if ( dQueries[i].m_dRes.GetLength() )
			tOut.SendString ( (const char *)dQueries[i].m_dRes.Begin() );
		else
			tOut.SendString ( "" );
	}

	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==iRespLen+8 );
}

/////////////////////////////////////////////////////////////////////////////
// KEYWORDS HANDLER
/////////////////////////////////////////////////////////////////////////////

void HandleCommandKeywords ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_KEYWORDS, tOut ) )
		return;

	CSphString sQuery = tReq.GetString ();
	CSphString sIndex = tReq.GetString ();
	bool bGetStats = !!tReq.GetInt ();

	const ServedIndex_c * pIndex = g_pLocalIndexes->GetRlockedEntry ( sIndex );
	if ( !pIndex )
	{
		pIndex = g_pTemplateIndexes->GetRlockedEntry ( sIndex );
		if ( !pIndex )
		{
			SendErrorReply ( tOut, "unknown local index '%s' in search request", sIndex.cstr() );
			return;
		}
	}

	CSphString sError;
	CSphVector < CSphKeywordInfo > dKeywords;
	if ( !pIndex->m_pIndex->GetKeywords ( dKeywords, sQuery.cstr (), bGetStats, &sError ) )
	{
		SendErrorReply ( tOut, "error generating keywords: %s", sError.cstr () );
		pIndex->Unlock();
		return;
	}

	pIndex->Unlock();

	int iRespLen = 4;
	ARRAY_FOREACH ( i, dKeywords )
	{
		iRespLen += 4 + strlen ( dKeywords[i].m_sTokenized.cstr () );
		iRespLen += 4 + strlen ( dKeywords[i].m_sNormalized.cstr () );
		if ( bGetStats )
			iRespLen += 8;
	}

	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_KEYWORDS );
	tOut.SendInt ( iRespLen );
	tOut.SendInt ( dKeywords.GetLength () );
	ARRAY_FOREACH ( i, dKeywords )
	{
		tOut.SendString ( dKeywords[i].m_sTokenized.cstr () );
		tOut.SendString ( dKeywords[i].m_sNormalized.cstr () );
		if ( bGetStats )
		{
			tOut.SendInt ( dKeywords[i].m_iDocs );
			tOut.SendInt ( dKeywords[i].m_iHits );
		}
	}

	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==iRespLen+8 );
}

/////////////////////////////////////////////////////////////////////////////
// UPDATES HANDLER
/////////////////////////////////////////////////////////////////////////////

struct UpdateRequestBuilder_t : public IRequestBuilder_t
{
	explicit UpdateRequestBuilder_t ( const CSphAttrUpdate & pUpd ) : m_tUpd ( pUpd ) {}
	virtual void BuildRequest ( AgentConn_t & tAgent, NetOutputBuffer_c & tOut ) const;

protected:
	const CSphAttrUpdate & m_tUpd;
};


struct UpdateReplyParser_t : public IReplyParser_t
{
	explicit UpdateReplyParser_t ( int * pUpd )
		: m_pUpdated ( pUpd )
	{}

	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const
	{
		*m_pUpdated += tReq.GetDword ();
		return true;
	}

protected:
	int * m_pUpdated;
};


void UpdateRequestBuilder_t::BuildRequest ( AgentConn_t & tAgent, NetOutputBuffer_c & tOut ) const
{
	const char* sIndexes = tAgent.m_sIndexes.cstr();
	int iReqSize = 4+strlen(sIndexes); // indexes string
	iReqSize += 8; // attrs array len, data, non-existent flags
	ARRAY_FOREACH ( i, m_tUpd.m_dAttrs )
		iReqSize += 8 + strlen ( m_tUpd.m_dAttrs[i] );
	iReqSize += 4; // number of updates
	iReqSize += 8 * m_tUpd.m_dDocids.GetLength() + 4 * m_tUpd.m_dPool.GetLength(); // 64bit ids, 32bit values

	bool bMva = false;
	ARRAY_FOREACH ( i, m_tUpd.m_dTypes )
	{
		assert ( m_tUpd.m_dTypes[i]!=SPH_ATTR_INT64SET ); // mva64 goes only via SphinxQL (SphinxqlRequestBuilder_t)
		bMva |= ( m_tUpd.m_dTypes[i]==SPH_ATTR_UINT32SET );
	}

	if ( bMva )
	{
		int iMvaCount = 0;
		ARRAY_FOREACH ( iDoc, m_tUpd.m_dDocids )
		{
			const DWORD * pPool = m_tUpd.m_dPool.Begin() + m_tUpd.m_dRowOffset[iDoc];
			ARRAY_FOREACH ( iAttr, m_tUpd.m_dTypes )
			{
				if ( m_tUpd.m_dTypes[iAttr]==SPH_ATTR_UINT32SET )
				{
					DWORD uVal = *pPool++;
					iMvaCount += uVal;
					pPool += uVal;
				} else
				{
					pPool++;
				}
			}
		}

		iReqSize -= ( iMvaCount * 4 / 2 ); // mva64 only via SphinxQL
	}

	// header
	tOut.SendWord ( SEARCHD_COMMAND_UPDATE );
	tOut.SendWord ( VER_COMMAND_UPDATE );
	tOut.SendInt ( iReqSize );

	tOut.SendString ( sIndexes );
	tOut.SendInt ( m_tUpd.m_dAttrs.GetLength() );
	tOut.SendInt ( m_tUpd.m_bIgnoreNonexistent ? 1 : 0 );
	ARRAY_FOREACH ( i, m_tUpd.m_dAttrs )
	{
		tOut.SendString ( m_tUpd.m_dAttrs[i] );
		tOut.SendInt ( ( m_tUpd.m_dTypes[i]==SPH_ATTR_UINT32SET ) ? 1 : 0 );
	}
	tOut.SendInt ( m_tUpd.m_dDocids.GetLength() );

	if ( !bMva )
	{
		ARRAY_FOREACH ( iDoc, m_tUpd.m_dDocids )
		{
			tOut.SendUint64 ( m_tUpd.m_dDocids[iDoc] );
			int iHead = m_tUpd.m_dRowOffset[iDoc];
			int iTail = m_tUpd.m_dPool.GetLength ();
			if ( (iDoc+1)<m_tUpd.m_dDocids.GetLength() )
				iTail = m_tUpd.m_dRowOffset[iDoc+1];

			for ( int j=iHead; j<iTail; j++ )
				tOut.SendDword ( m_tUpd.m_dPool[j] );
		}
	} else
	{
		// size down in case of MVA
		// MVA stored as mva64 in pool but API could handle only mva32 due to HandleCommandUpdate
		// SphinxQL only could work either mva32 or mva64 and only SphinxQL could receive mva64 updates
		// SphinxQL master communicate to agent via SphinxqlRequestBuilder_t

		ARRAY_FOREACH ( iDoc, m_tUpd.m_dDocids )
		{
			tOut.SendUint64 ( m_tUpd.m_dDocids[iDoc] );

			const DWORD * pPool = m_tUpd.m_dPool.Begin() + m_tUpd.m_dRowOffset[iDoc];
			ARRAY_FOREACH ( iAttr, m_tUpd.m_dTypes )
			{
				DWORD uVal = *pPool++;
				if ( m_tUpd.m_dTypes[iAttr]!=SPH_ATTR_UINT32SET )
				{
					tOut.SendDword ( uVal );
				} else
				{
					const DWORD * pEnd = pPool + uVal;
					tOut.SendDword ( uVal/2 );
					while ( pPool<pEnd )
					{
						tOut.SendDword ( *pPool );
						pPool += 2;
					}
				}
			}
		}
	}
}

static void DoCommandUpdate ( const char * sIndex, const CSphAttrUpdate & tUpd,
	int & iSuccesses, int & iUpdated,
	SearchFailuresLog_c & dFails, const ServedIndex_c * pServed )
{
	if ( !pServed || !pServed->m_pIndex || !pServed->m_bEnabled )
	{
		dFails.Submit ( sIndex, "index not available" );
		return;
	}

	CSphString sError, sWarning;
	int iUpd = pServed->m_pIndex->UpdateAttributes ( tUpd, -1, sError, sWarning );

	if ( iUpd<0 )
	{
		dFails.Submit ( sIndex, sError.cstr() );

	} else
	{
		iUpdated += iUpd;
		iSuccesses++;
		if ( sWarning.Length() )
			dFails.Submit ( sIndex, sWarning.cstr() );
	}
}

static const ServedIndex_c * UpdateGetLockedIndex ( const CSphString & sName, bool bMvaUpdate )
{
	// MVA updates have to be done sequentially
	if ( !bMvaUpdate )
		return g_pLocalIndexes->GetRlockedEntry ( sName );

	return g_pLocalIndexes->GetWlockedEntry ( sName );
}


void HandleCommandUpdate ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_UPDATE, tOut ) )
		return;

	// parse request
	CSphString sIndexes = tReq.GetString ();
	CSphAttrUpdate tUpd;
	CSphVector<DWORD> dMva;

	bool bMvaUpdate = false;

	tUpd.m_dAttrs.Resize ( tReq.GetDword() ); // FIXME! check this
	tUpd.m_dTypes.Resize ( tUpd.m_dAttrs.GetLength() );
	if ( iVer>=0x103 )
		tUpd.m_bIgnoreNonexistent = ( tReq.GetDword() & 1 )!=0;
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		tUpd.m_dAttrs[i] = tReq.GetString().ToLower().Leak();
		tUpd.m_dTypes[i] = SPH_ATTR_INTEGER;
		if ( iVer>=0x102 )
		{
			if ( tReq.GetDword() )
			{
				tUpd.m_dTypes[i] = SPH_ATTR_UINT32SET;
				bMvaUpdate = true;
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

		tUpd.m_dDocids.Add ( (SphDocID_t)uDocid ); // FIXME! check this
		tUpd.m_dRowOffset.Add ( tUpd.m_dPool.GetLength() );

		ARRAY_FOREACH ( iAttr, tUpd.m_dAttrs )
		{
			if ( tUpd.m_dTypes[iAttr]==SPH_ATTR_UINT32SET )
			{
				DWORD uCount = tReq.GetDword ();
				if ( !uCount )
				{
					tUpd.m_dPool.Add ( 0 );
					continue;
				}

				dMva.Resize ( uCount );
				for ( DWORD j=0; j<uCount; j++ )
				{
					dMva[j] = tReq.GetDword();
				}
				dMva.Uniq(); // don't need dupes within MVA

				tUpd.m_dPool.Add ( dMva.GetLength()*2 );
				ARRAY_FOREACH ( j, dMva )
				{
					tUpd.m_dPool.Add ( dMva[j] );
					tUpd.m_dPool.Add ( 0 ); // dummy expander mva32 -> mva64
				}
			} else
			{
				tUpd.m_dPool.Add ( tReq.GetDword() );
			}
		}
	}

	if ( tReq.GetError() )
	{
		SendErrorReply ( tOut, "invalid or truncated request" );
		return;
	}

	// check index names
	CSphVector<CSphString> dIndexNames;
	ParseIndexList ( sIndexes, dIndexNames );

	if ( !dIndexNames.GetLength() )
	{
		SendErrorReply ( tOut, "no valid indexes in update request" );
		return;
	}

	CSphVector<DistributedIndex_t> dDistributed ( dIndexNames.GetLength() ); // lock safe storage for distributed indexes
	ARRAY_FOREACH ( i, dIndexNames )
	{
		if ( !g_pLocalIndexes->Exists ( dIndexNames[i] ) )
		{
			// search amongst distributed and copy for further processing
			g_tDistLock.Lock();
			const DistributedIndex_t * pDistIndex = g_hDistIndexes ( dIndexNames[i] );

			if ( pDistIndex )
			{
				dDistributed[i] = *pDistIndex;
			}

			g_tDistLock.Unlock();

			if ( pDistIndex )
				continue;
			else
			{
				SendErrorReply ( tOut, "unknown index '%s' in update request", dIndexNames[i].cstr() );
				return;
			}
		}
	}

	// do update
	SearchFailuresLog_c dFails;
	int iSuccesses = 0;
	int iUpdated = 0;
	tUpd.m_dRows.Resize ( tUpd.m_dDocids.GetLength() );
	ARRAY_FOREACH ( i, tUpd.m_dRows ) tUpd.m_dRows[i] = NULL;

	ARRAY_FOREACH ( iIdx, dIndexNames )
	{
		const char * sReqIndex = dIndexNames[iIdx].cstr();
		const ServedIndex_c * pLocked = UpdateGetLockedIndex ( sReqIndex, bMvaUpdate );
		if ( pLocked )
		{
			DoCommandUpdate ( sReqIndex, tUpd, iSuccesses, iUpdated, dFails, pLocked );
			pLocked->Unlock();
		} else
		{
			assert ( dDistributed[iIdx].m_dLocal.GetLength() || dDistributed[iIdx].m_dAgents.GetLength() );
			CSphVector<CSphString>& dLocal = dDistributed[iIdx].m_dLocal;

			ARRAY_FOREACH ( i, dLocal )
			{
				const char * sLocal = dLocal[i].cstr();
				const ServedIndex_c * pServed = UpdateGetLockedIndex ( sLocal, bMvaUpdate );
				DoCommandUpdate ( sLocal, tUpd, iSuccesses, iUpdated, dFails, pServed );
				if ( pServed )
					pServed->Unlock();
			}
		}

		// update remote agents
		if ( dDistributed[iIdx].m_dAgents.GetLength() )
		{
			DistributedIndex_t & tDist = dDistributed[iIdx];

			CSphVector<AgentConn_t> dAgents;
			tDist.GetAllAgents ( &dAgents );

			// connect to remote agents and query them
			UpdateRequestBuilder_t tReqBuilder ( tUpd );
			CSphScopedPtr<ISphRemoteAgentsController> pDistCtrl ( GetAgentsController ( g_iDistThreads, dAgents, tReqBuilder, tDist.m_iAgentConnectTimeout ) );
			int iAgentsDone = pDistCtrl->Finish();
			if ( iAgentsDone )
			{
				UpdateReplyParser_t tParser ( &iUpdated );
				iSuccesses += RemoteWaitForAgents ( dAgents, tDist.m_iAgentQueryTimeout, tParser ); // FIXME? profile update time too?
			}
		}
	}

	// serve reply to client
	CSphStringBuilder sReport;
	dFails.BuildReport ( sReport );

	if ( !iSuccesses )
	{
		SendErrorReply ( tOut, "%s", sReport.cstr() );
		return;
	}

	if ( dFails.IsEmpty() )
	{
		tOut.SendWord ( SEARCHD_OK );
		tOut.SendWord ( VER_COMMAND_UPDATE );
		tOut.SendInt ( 4 );
	} else
	{
		tOut.SendWord ( SEARCHD_WARNING );
		tOut.SendWord ( VER_COMMAND_UPDATE );
		tOut.SendInt ( 8 + strlen ( sReport.cstr() ) );
		tOut.SendString ( sReport.cstr() );
	}
	tOut.SendInt ( iUpdated );
	tOut.Flush ();
}

// 'like' matcher
class CheckLike
{
private:
	CSphString m_sPattern;

public:
	explicit CheckLike ( const char * sPattern )
	{
		if ( !sPattern )
			return;

		m_sPattern.Reserve ( 2*strlen ( sPattern ) );
		char * d = const_cast<char*> ( m_sPattern.cstr() );

		// remap from SQL LIKE syntax to Sphinx wildcards syntax
		// '_' maps to '?', match any single char
		// '%' maps to '*', match zero or mor chars
		for ( const char * s = sPattern; *s; s++ )
		{
			switch ( *s )
			{
				case '_':	*d++ = '?'; break;
				case '%':	*d++ = '*'; break;
				case '?':	*d++ = '\\'; *d++ = '?'; break;
				case '*':	*d++ = '\\'; *d++ = '*'; break;
				default:	*d++ = *s; break;
			}
		}
		*d = '\0';
	}

	bool Match ( const char * sValue )
	{
		return sValue && ( m_sPattern.IsEmpty() || sphWildcardMatch ( sValue, m_sPattern.cstr() ) );
	}
};

// string vector with 'like' matcher
class VectorLike : public CSphVector<CSphString>, public CheckLike
{
public:
	CSphString m_sColKey;
	CSphString m_sColValue;

public:

	VectorLike ()
		: CheckLike ( NULL )
	{}

	explicit VectorLike ( const CSphString& sPattern )
		: CheckLike ( sPattern.cstr() )
		, m_sColKey ( "Variable_name" )
		, m_sColValue ( "Value" )
	{}

	inline const char * szColKey() const
	{
		return m_sColKey.cstr();
	}

	inline const char * szColValue() const
	{
		return m_sColValue.cstr();
	}

	bool MatchAdd ( const char* sValue )
	{
		if ( Match ( sValue ) )
		{
			Add ( sValue );
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
};

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
	const char * OFF = "OFF";

	const int64_t iQueriesDiv = Max ( g_tStats.m_iQueries, 1 );
	const int64_t iDistQueriesDiv = Max ( g_tStats.m_iDistQueries, 1 );

	dStatus.m_sColKey = "Counter";

	// FIXME? non-transactional!!!
	if ( dStatus.MatchAdd ( "uptime" ) )
		dStatus.Add().SetSprintf ( "%u", (DWORD)time(NULL)-g_tStats.m_uStarted );
	if ( dStatus.MatchAdd ( "connections" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iConnections );
	if ( dStatus.MatchAdd ( "maxed_out" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iMaxedOut );
	if ( dStatus.MatchAdd ( "command_search" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iCommandCount[SEARCHD_COMMAND_SEARCH] );
	if ( dStatus.MatchAdd ( "command_excerpt" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iCommandCount[SEARCHD_COMMAND_EXCERPT] );
	if ( dStatus.MatchAdd ( "command_update" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iCommandCount[SEARCHD_COMMAND_UPDATE] );
	if ( dStatus.MatchAdd ( "command_delete" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iCommandCount[SEARCHD_COMMAND_DELETE] );
	if ( dStatus.MatchAdd ( "command_keywords" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iCommandCount[SEARCHD_COMMAND_KEYWORDS] );
	if ( dStatus.MatchAdd ( "command_persist" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iCommandCount[SEARCHD_COMMAND_PERSIST] );
	if ( dStatus.MatchAdd ( "command_status" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iCommandCount[SEARCHD_COMMAND_STATUS] );
	if ( dStatus.MatchAdd ( "command_flushattrs" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iCommandCount[SEARCHD_COMMAND_FLUSHATTRS] );
	if ( dStatus.MatchAdd ( "agent_connect" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iAgentConnect );
	if ( dStatus.MatchAdd ( "agent_retry" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iAgentRetry );
	if ( dStatus.MatchAdd ( "queries" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iQueries );
	if ( dStatus.MatchAdd ( "dist_queries" ) )
		dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iDistQueries );

	// status of thread pool
	if ( g_pThdPool )
	{
		if ( dStatus.MatchAdd ( "workers_total" ) )
			dStatus.Add().SetSprintf ( "%d", g_pThdPool->GetTotalWorkerCount() );
		if ( dStatus.MatchAdd ( "workers_active" ) )
			dStatus.Add().SetSprintf ( "%d", g_pThdPool->GetActiveWorkerCount() );
		if ( dStatus.MatchAdd ( "work_queue_length" ) )
			dStatus.Add().SetSprintf ( "%d", g_pThdPool->GetQueueLength() );
	}

	g_tDistLock.Lock();
	g_hDistIndexes.IterateStart();
	while ( g_hDistIndexes.IterateNext() )
	{
		const char * sIdx = g_hDistIndexes.IterateGetKey().cstr();
		CSphVector<MetaAgentDesc_t> & dAgents = g_hDistIndexes.IterateGet().m_dAgents;
		ARRAY_FOREACH ( i, dAgents )
			ARRAY_FOREACH ( j, dAgents[i].GetAgents() )
		{
			int iIndex = dAgents[i].GetAgents()[j].m_iStatsIndex;
			if ( iIndex<0 || iIndex>=STATS_MAX_AGENTS )
				continue;

			const AgentStats_t & tStats = g_tStats.m_dAgentStats.m_dItemStats[iIndex];
			for ( int k=0; k<eMaxStat; ++k )
				if ( dStatus.MatchAddVa ( "ag_%s_%d_%d_%s", sIdx, i+1, j+1, tStats.m_sNames[k] ) )
					dStatus.Add().SetSprintf ( FMT64, tStats.m_iStats[k] );
		}
	}
	g_tDistLock.Unlock();

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
			dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iDiskReads );
		if ( dStatus.MatchAdd ( "query_readkb" ) )
			dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iDiskReadBytes/1024 );
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
			dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iPredictedTime );
		if ( dStatus.MatchAdd ( "dist_predicted_time" ) )
			dStatus.Add().SetSprintf ( FMT64, g_tStats.m_iAgentPredictedTime );
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
		dStatus.Add().SetSprintf ( "%d", s.m_iThreshMsec );
	if ( dStatus.MatchAdd ( "qcache_ttl_sec" ) )
		dStatus.Add().SetSprintf ( "%d", s.m_iTtlSec );

	if ( dStatus.MatchAdd ( "qcache_cached_queries" ) )
		dStatus.Add().SetSprintf ( "%d", s.m_iCachedQueries );
	if ( dStatus.MatchAdd ( "qcache_used_bytes" ) )
		dStatus.Add().SetSprintf ( INT64_FMT, s.m_iUsedBytes );
	if ( dStatus.MatchAdd ( "qcache_hits" ) )
		dStatus.Add().SetSprintf ( INT64_FMT, s.m_iHits );
}

void BuildOneAgentStatus ( VectorLike & dStatus, const CSphString& sAgent, const char * sPrefix="agent" )
{
	g_tStatsMutex.Lock ();
	const int * pIndex = g_tStats.m_hDashBoard ( sAgent );
	if ( !pIndex )
	{
		if ( dStatus.MatchAdd ( "status_error" ) )
			dStatus.Add().SetSprintf ( "No such agent: %s", sAgent.cstr() );
		g_tStatsMutex.Unlock ();
		return;
	}

	const int & iIndex = *pIndex;
	const char * FMT64 = UINT64_FMT;
	const char * FLOAT = "%.2f";

	if ( dStatus.MatchAddVa ( "%s_hostname", sPrefix ) )
		dStatus.Add().SetSprintf ( "%s", sAgent.cstr() );

	const HostDashboard_t & dDash = g_tStats.m_dDashboard.m_dItemStats[iIndex];
	if ( dStatus.MatchAddVa ( "%s_references", sPrefix ) )
		dStatus.Add().SetSprintf ( "%d", dDash.m_iRefCount );
	uint64_t iCur = sphMicroTimer();
	uint64_t iLastAccess = iCur - dDash.m_iLastQueryTime;
	float fPeriod = (float)iLastAccess/1000000.0f;
	if ( dStatus.MatchAddVa ( "%s_lastquery", sPrefix ) )
		dStatus.Add().SetSprintf ( FLOAT, fPeriod );
	iLastAccess = iCur - dDash.m_iLastAnswerTime;
	fPeriod = (float)iLastAccess/1000000.0f;
	if ( dStatus.MatchAddVa ( "%s_lastanswer", sPrefix ) )
		dStatus.Add().SetSprintf ( FLOAT, fPeriod );
	uint64_t iLastTimer = dDash.m_iLastAnswerTime-dDash.m_iLastQueryTime;
	if ( dStatus.MatchAddVa ( "%s_lastperiodmsec", sPrefix ) )
		dStatus.Add().SetSprintf ( FMT64, iLastTimer/1000 );
	if ( dStatus.MatchAddVa ( "%s_errorsarow", sPrefix ) )
		dStatus.Add().SetSprintf ( FMT64, dDash.m_iErrorsARow );

	AgentDash_t dDashStat;
	int iPeriods = 1;
	int iHadPeriods = 0;
	int iHavePeriods = 0;

	while ( iPeriods>0 )
	{
		iHavePeriods = dDash.GetDashStat ( &dDashStat, iPeriods );
		uint64_t uQueries = 0;

		if ( iHavePeriods!=iHadPeriods )
		{
			for ( int j=0; j<eMaxStat; ++j )
				if ( j==eTotalMsecs ) // hack. Avoid microseconds in human-readable statistic
				{
					if ( dStatus.MatchAddVa ( "%s_%dperiods_msecsperqueryy", sPrefix, iPeriods ) )
					{
						if ( uQueries>0 )
						{
							float fAverageLatency = (float)(( dDashStat.m_iStats[eTotalMsecs] / 1000.0 ) / uQueries );
							dStatus.Add().SetSprintf ( FLOAT, fAverageLatency );
						} else
							dStatus.Add("n/a");
					}
				} else
				{
					if ( dStatus.MatchAddVa ( "%s_%dperiods_%s", sPrefix, iPeriods, dDashStat.m_sNames[j] ) )
						dStatus.Add().SetSprintf ( FMT64, dDashStat.m_iStats[j] );
					uQueries += dDashStat.m_iStats[j];
				}
			iHadPeriods = iHavePeriods;
		}

		if ( iPeriods==1 )
			iPeriods = 5;
		else if ( iPeriods==5 )
			iPeriods = STATS_DASH_TIME;
		else if ( iPeriods==STATS_DASH_TIME )
			iPeriods = -1;
	}
	g_tStatsMutex.Unlock ();
}

void BuildDistIndexStatus ( VectorLike & dStatus, const CSphString& sIndex )
{
	g_tDistLock.Lock();
	DistributedIndex_t * pDistr = g_hDistIndexes(sIndex);
	g_tDistLock.Unlock();

	if ( !pDistr )
	{
		if ( dStatus.MatchAdd ( "status_error" ) )
			dStatus.Add().SetSprintf ( "No such index: %s", sIndex.cstr() );
		return;
	}

	ARRAY_FOREACH ( i, pDistr->m_dLocal )
	{
		if ( dStatus.MatchAddVa ( "dstindex_local_%d", i+1 ) )
			dStatus.Add ( pDistr->m_dLocal[i].cstr() );
	}

	CSphString sKey;
	ARRAY_FOREACH ( i, pDistr->m_dAgents )
	{
		MetaAgentDesc_t & tAgents = pDistr->m_dAgents[i];
		if ( dStatus.MatchAddVa ( "dstindex_%d_is_ha", i+1 ) )
			dStatus.Add ( tAgents.IsHA()? "1": "0" );

		ARRAY_FOREACH ( j, tAgents.GetAgents() )
		{
			if ( tAgents.IsHA() )
				sKey.SetSprintf ( "dstindex_%dmirror%d", i+1, j+1 );
			else
				sKey.SetSprintf ( "dstindex_%dagent", i+1 );

			const AgentDesc_c & dDesc = tAgents.GetAgents()[j];

			if ( dStatus.MatchAddVa ( "%s_id", sKey.cstr() ) )
				dStatus.Add().SetSprintf ( "%s:%s", dDesc.GetMyUrl().cstr(), dDesc.m_sIndexes.cstr() );

			if ( tAgents.IsHA() && dStatus.MatchAddVa ( "%s_probability_weight", sKey.cstr() ) )
				dStatus.Add().SetSprintf ( "%d", (DWORD)(tAgents.GetWeights()[j]) ); // FIXME! IPC unsafe, if critical need to be locked.

			if ( dStatus.MatchAddVa ( "%s_is_blackhole", sKey.cstr() ) )
				dStatus.Add ( dDesc.m_bBlackhole ? "1" : "0" );

			if ( dStatus.MatchAddVa ( "%s_is_persistent", sKey.cstr() ) )
				dStatus.Add ( dDesc.m_bPersistent ? "1" : "0" );
		}
	}
}

void BuildAgentStatus ( VectorLike & dStatus, const CSphString& sAgent )
{
	if ( !sAgent.IsEmpty() )
	{
		if ( g_hDistIndexes.Exists(sAgent) )
			BuildDistIndexStatus ( dStatus, sAgent );
		else
			BuildOneAgentStatus ( dStatus, sAgent );
		return;
	}

	dStatus.m_sColKey = "Key";

	if ( dStatus.MatchAdd ( "status_period_seconds" ) )
		dStatus.Add().SetSprintf ( "%d", g_uHAPeriodKarma );
	if ( dStatus.MatchAdd ( "status_stored_periods" ) )
		dStatus.Add().SetSprintf ( "%d", STATS_DASH_TIME );

	g_tStats.m_hDashBoard.IterateStart();

	CSphString sPrefix;
	while ( g_tStats.m_hDashBoard.IterateNext() )
	{
		int iIndex = g_tStats.m_hDashBoard.IterateGet();
		const CSphString sKey = g_tStats.m_hDashBoard.IterateGetKey();
		sPrefix.SetSprintf ( "ag_%d", iIndex );
		BuildOneAgentStatus ( dStatus, sKey, sPrefix.cstr() );
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
	tMeta.m_hWordStats.IterateStart();
	while ( tMeta.m_hWordStats.IterateNext() )
	{
		const CSphQueryResultMeta::WordStat_t & tStat = tMeta.m_hWordStats.IterateGet();

		if ( dStatus.MatchAddVa ( "keyword[%d]", iWord ) )
			dStatus.Add ( tMeta.m_hWordStats.IterateGetKey() );

		if ( dStatus.MatchAddVa ( "docs[%d]", iWord ) )
			dStatus.Add().SetSprintf ( INT64_FMT, tStat.m_iDocs );

		if ( dStatus.MatchAddVa ( "hits[%d]", iWord ) )
			dStatus.Add().SetSprintf ( INT64_FMT, tStat.m_iHits );

		iWord++;
	}
}


void HandleCommandStatus ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_STATUS, tOut ) )
		return;

	bool bGlobalStat = tReq.GetDword ()!=0;

	VectorLike dStatus;

	if ( bGlobalStat )
		BuildStatus ( dStatus );
	else
	{
		g_tLastMetaMutex.Lock();
		BuildMeta ( dStatus, g_tLastMeta );
		if ( g_tStats.m_iPredictedTime || g_tStats.m_iAgentPredictedTime )
		{
			if ( dStatus.MatchAdd ( "predicted_time" ) )
				dStatus.Add().SetSprintf ( INT64_FMT, g_tStats.m_iPredictedTime );
			if ( dStatus.MatchAdd ( "dist_predicted_time" ) )
				dStatus.Add().SetSprintf ( INT64_FMT, g_tStats.m_iAgentPredictedTime );
		}
		g_tLastMetaMutex.Unlock();
	}

	int iRespLen = 8; // int rows, int cols
	ARRAY_FOREACH ( i, dStatus )
		iRespLen += 4 + strlen ( dStatus[i].cstr() );

	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_STATUS );
	tOut.SendInt ( iRespLen );

	tOut.SendInt ( dStatus.GetLength()/2 ); // rows
	tOut.SendInt ( 2 ); // cols
	ARRAY_FOREACH ( i, dStatus )
		tOut.SendString ( dStatus[i].cstr() );

	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==8+iRespLen );
}

//////////////////////////////////////////////////////////////////////////
// FLUSH HANDLER
//////////////////////////////////////////////////////////////////////////

static int CommandFlush ()
{
	// force a check in head process, and wait it until completes
	// FIXME! semi active wait..
	sphLogDebug ( "attrflush: forcing check, tag=%d", g_tFlush.m_iFlushTag );
	g_tFlush.m_bForceCheck = true;
	while ( g_tFlush.m_bForceCheck )
		sphSleepMsec ( 1 );

	// if we are flushing now, wait until flush completes
	while ( g_tFlush.m_bFlushing )
		sphSleepMsec ( 10 );
	sphLogDebug ( "attrflush: check finished, tag=%d", g_tFlush.m_iFlushTag );

	return g_tFlush.m_iFlushTag;
}

void HandleCommandFlush ( ISphOutputBuffer & tOut, int iVer )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_FLUSHATTRS, tOut ) )
		return;

	int iTag = CommandFlush ();
	// return last flush tag, just for the fun of it
	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_FLUSHATTRS );
	tOut.SendInt ( 4 ); // resplen, 1 dword
	tOut.SendInt ( iTag );
	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==12 ); // 8+resplen
}


/////////////////////////////////////////////////////////////////////////////
// GENERAL HANDLER
/////////////////////////////////////////////////////////////////////////////

#define THD_STATE(_state) \
{ \
	DISABLE_CONST_COND_CHECK \
	{ \
		pThd->m_eThdState = _state; \
		pThd->m_tmStart = sphMicroTimer(); \
		if_const ( _state==THD_NET_IDLE ) \
			pThd->m_dBuf[0] = '\0'; \
	} \
	ENABLE_CONST_COND_CHECK \
}

void HandleCommandSphinxql ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq, ThdDesc_t * pThd ); // definition is below
void StatCountCommand ( int iCmd, int iCount=1 );
void HandleCommandUserVar ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq );

/// ping/pong exchange over API
void HandleCommandPing ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_PING, tOut ) )
		return;

	// parse ping
	int iCookie = tReq.GetInt();

	// return last flush tag, just for the fun of it
	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_PING );
	tOut.SendInt ( 4 ); // resplen, 1 dword
	tOut.SendInt ( iCookie ); // echo the cookie back
	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==12 ); // 8+resplen
}

static bool LoopClientSphinx ( int iCommand, int iCommandVer, int iLength, const char * sClientIP, int64_t iCID, ThdDesc_t * pThd, InputBuffer_c & tBuf, ISphOutputBuffer & tOut, bool bManagePersist );

static void HandleClientSphinx ( int iSock, const char * sClientIP, ThdDesc_t * pThd )
{
	MEMORY ( MEM_API_HANDLE );
	THD_STATE ( THD_HANDSHAKE );

	int64_t iCID = pThd->m_iConnID;

	// send my version
	DWORD uServer = htonl ( SPHINX_SEARCHD_PROTO );
	if ( sphSockSend ( iSock, (char*)&uServer, sizeof(uServer) )!=sizeof(uServer) )
	{
		sphWarning ( "failed to send server version (client=%s("INT64_FMT"))", sClientIP, iCID );
		return;
	}

	// get client version and request
	int iMagic = 0;
	int iGot = 0;
	if ( iSock>=0 )
		iGot = sphSockRead ( iSock, &iMagic, sizeof(iMagic), g_iReadTimeout, false );

	bool bReadErr = ( iGot!=sizeof(iMagic) );
	sphLogDebugv ( "conn %s("INT64_FMT"): got handshake, major v.%d, err %d", sClientIP, iCID, iMagic, (int)bReadErr );
	if ( bReadErr )
	{
		sphLogDebugv ( "conn %s("INT64_FMT"): exiting on handshake error", sClientIP, iCID );
		return;
	}

	bool bPersist = false;
	int iPconnIdle = 0;
	do
	{
		NetInputBuffer_c tBuf ( iSock );
		NetOutputBuffer_c tOut ( iSock );

		int iTimeout = bPersist ? 1 : g_iReadTimeout;

		// in "persistent connection" mode, we want interruptible waits
		// so that the worker child could be forcibly restarted
		//
		// currently, the only signal allowed to interrupt this read is SIGTERM
		// letting SIGHUP interrupt causes trouble under query/rotation pressure
		// see sphSockRead() and ReadFrom() for details
		THD_STATE ( THD_NET_IDLE );
		bool bCommand = tBuf.ReadFrom ( 8, iTimeout, bPersist );

		// on SIGTERM, bail unconditionally and immediately, at all times
		if ( !bCommand && g_bGotSigterm )
		{
			sphLogDebugv ( "conn %s("INT64_FMT"): bailing on SIGTERM", sClientIP, iCID );
			break;
		}

		// on SIGHUP vs pconn, bail if a pconn was idle for 1 sec
		if ( bPersist && !bCommand && g_bGotSighup && sphSockPeekErrno()==ETIMEDOUT )
		{
			sphLogDebugv ( "conn %s("INT64_FMT"): bailing idle pconn on SIGHUP", sClientIP, iCID );
			break;
		}

		// on pconn that was idle for 300 sec (client_timeout), bail
		if ( bPersist && !bCommand && sphSockPeekErrno()==ETIMEDOUT )
		{
			iPconnIdle += iTimeout;
			bool bClientTimedout = ( iPconnIdle>=g_iClientTimeout );
			if ( bClientTimedout )
				sphLogDebugv ( "conn %s("INT64_FMT"): bailing idle pconn on client_timeout", sClientIP, iCID );

			if ( !bClientTimedout )
				continue;
			else
				break;
		}
		iPconnIdle = 0;

		// on any other signals vs pconn, ignore and keep looping
		// (redundant for now, as the only allowed interruption is SIGTERM, but.. let's keep it)
		if ( bPersist && !bCommand && tBuf.IsIntr() )
			continue;

		// okay, signal related mess should be over, try to parse the command
		// (but some other socket error still might had happened, so beware)
		THD_STATE ( THD_NET_READ );
		int iCommand = tBuf.GetWord ();
		int iCommandVer = tBuf.GetWord ();
		int iLength = tBuf.GetInt ();
		if ( tBuf.GetError() )
		{
			// under high load, there can be pretty frequent accept() vs connect() timeouts
			// lets avoid agent log flood
			//
			// sphWarning ( "failed to receive client version and request (client=%s, error=%s)", sClientIP, sphSockError() );
			sphLogDebugv ( "conn %s("INT64_FMT"): bailing on failed request header (sockerr=%s)", sClientIP, iCID, sphSockError() );
			break;
		}

		// check request
		if ( iCommand<0 || iCommand>=SEARCHD_COMMAND_TOTAL || iLength<0 || iLength>g_iMaxPacketSize )
		{
			// unknown command, default response header
			SendErrorReply ( tOut, "invalid command (code=%d, len=%d)", iCommand, iLength );

			// if request length is insane, low level comm is broken, so we bail out
			if ( iLength<0 || iLength>g_iMaxPacketSize )
				sphWarning ( "ill-formed client request (length=%d out of bounds)", iLength );

			// if command is insane, low level comm is broken, so we bail out
			if ( iCommand<0 || iCommand>=SEARCHD_COMMAND_TOTAL )
				sphWarning ( "ill-formed client request (command=%d, SEARCHD_COMMAND_TOTAL=%d)", iCommand, SEARCHD_COMMAND_TOTAL );

			break;
		}

		// get request body
		assert ( iLength>=0 && iLength<=g_iMaxPacketSize );
		if ( iLength && !tBuf.ReadFrom ( iLength ) )
		{
			sphWarning ( "failed to receive client request body (client=%s("INT64_FMT"), exp=%d, error='%s')", sClientIP, iCID, iLength, sphSockError() );
			break;
		}

		bPersist |= LoopClientSphinx ( iCommand, iCommandVer, iLength, sClientIP, iCID, pThd, tBuf, tOut, true );
	} while ( bPersist );

	if ( bPersist )
		g_iPersistentInUse.Dec();

	sphLogDebugv ( "conn %s("INT64_FMT"): exiting", sClientIP, iCID );
}


bool LoopClientSphinx ( int iCommand, int iCommandVer, int iLength, const char * sClientIP, int64_t iCID, ThdDesc_t * pThd, InputBuffer_c & tBuf, ISphOutputBuffer & tOut, bool bManagePersist )
{
	// set on query guard
	CrashQuery_t tCrashQuery;
	tCrashQuery.m_pQuery = tBuf.GetBufferPtr();
	tCrashQuery.m_iSize = iLength;
	tCrashQuery.m_bMySQL = false;
	tCrashQuery.m_uCMD = (WORD)iCommand;
	tCrashQuery.m_uVer = (WORD)iCommandVer;
	SphCrashLogger_c::SetLastQuery ( tCrashQuery );

	// handle known commands
	assert ( iCommand>=0 && iCommand<SEARCHD_COMMAND_TOTAL );

	// count commands
	StatCountCommand ( iCommand );

	pThd->m_sCommand = g_dApiCommands[iCommand];
	THD_STATE ( THD_QUERY );

	bool bPersist = false;
	sphLogDebugv ( "conn %s("INT64_FMT"): got command %d, handling", sClientIP, iCID, iCommand );
	switch ( iCommand )
	{
		case SEARCHD_COMMAND_SEARCH:	HandleCommandSearch ( tOut, iCommandVer, tBuf, pThd ); break;
		case SEARCHD_COMMAND_EXCERPT:	HandleCommandExcerpt ( tOut, iCommandVer, tBuf, pThd ); break;
		case SEARCHD_COMMAND_KEYWORDS:	HandleCommandKeywords ( tOut, iCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_UPDATE:	HandleCommandUpdate ( tOut, iCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_PERSIST:
			{
				bPersist = ( tBuf.GetInt()!=0 );
				sphLogDebugv ( "conn %s("INT64_FMT"): pconn is now %s", sClientIP, iCID, bPersist ? "on" : "off" );
				if ( !bManagePersist ) // thread pool handles all persist connections
					break;

				// FIXME!!! remove that mess
				if ( bPersist )
				{
					bPersist = ( g_iMaxChildren && 1+g_iPersistentInUse.GetValue()<g_iMaxChildren );
					if ( bPersist )
						g_iPersistentInUse.Inc();
				} else
				{
					if ( g_iPersistentInUse.GetValue()>=1 )
						g_iPersistentInUse.Dec();
				}
			}
			break;
		case SEARCHD_COMMAND_STATUS:	HandleCommandStatus ( tOut, iCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_FLUSHATTRS:HandleCommandFlush ( tOut, iCommandVer ); break;
		case SEARCHD_COMMAND_SPHINXQL:	HandleCommandSphinxql ( tOut, iCommandVer, tBuf, pThd ); break;
		case SEARCHD_COMMAND_PING:		HandleCommandPing ( tOut, iCommandVer, tBuf ); break;
		case SEARCHD_COMMAND_UVAR:		HandleCommandUserVar ( tOut, iCommandVer, tBuf ); break;
		default:						assert ( 0 && "INTERNAL ERROR: unhandled command" ); break;
	}

	// set off query guard
	SphCrashLogger_c::SetLastQuery ( CrashQuery_t() );
	return bPersist;
}

//////////////////////////////////////////////////////////////////////////
// MYSQLD PRETENDER
//////////////////////////////////////////////////////////////////////////

// our copy of enum_field_types
// we can't rely on mysql_com.h because it might be unavailable
//
// MYSQL_TYPE_DECIMAL = 0
// MYSQL_TYPE_TINY = 1
// MYSQL_TYPE_SHORT = 2
// MYSQL_TYPE_LONG = 3
// MYSQL_TYPE_FLOAT = 4
// MYSQL_TYPE_DOUBLE = 5
// MYSQL_TYPE_NULL = 6
// MYSQL_TYPE_TIMESTAMP = 7
// MYSQL_TYPE_LONGLONG = 8
// MYSQL_TYPE_INT24 = 9
// MYSQL_TYPE_DATE = 10
// MYSQL_TYPE_TIME = 11
// MYSQL_TYPE_DATETIME = 12
// MYSQL_TYPE_YEAR = 13
// MYSQL_TYPE_NEWDATE = 14
// MYSQL_TYPE_VARCHAR = 15
// MYSQL_TYPE_BIT = 16
// MYSQL_TYPE_NEWDECIMAL = 246
// MYSQL_TYPE_ENUM = 247
// MYSQL_TYPE_SET = 248
// MYSQL_TYPE_TINY_BLOB = 249
// MYSQL_TYPE_MEDIUM_BLOB = 250
// MYSQL_TYPE_LONG_BLOB = 251
// MYSQL_TYPE_BLOB = 252
// MYSQL_TYPE_VAR_STRING = 253
// MYSQL_TYPE_STRING = 254
// MYSQL_TYPE_GEOMETRY = 255

enum MysqlColumnType_e
{
	MYSQL_COL_DECIMAL	= 0,
	MYSQL_COL_LONG		= 3,
	MYSQL_COL_FLOAT	= 4,
	MYSQL_COL_LONGLONG	= 8,
	MYSQL_COL_STRING	= 254
};



void SendMysqlFieldPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, const char * sCol, MysqlColumnType_e eType )
{
	const char * sDB = "";
	const char * sTable = "";

	int iLen = 17 + MysqlPackedLen(sDB) + 2*( MysqlPackedLen(sTable) + MysqlPackedLen(sCol) );

	int iColLen = 0;
	switch ( eType )
	{
		case MYSQL_COL_DECIMAL:		iColLen = 20; break;
		case MYSQL_COL_LONG:		iColLen = 11; break;
		case MYSQL_COL_FLOAT:		iColLen = 20; break;
		case MYSQL_COL_LONGLONG:	iColLen = 20; break;
		case MYSQL_COL_STRING:		iColLen = 255; break;
	}

	tOut.SendLSBDword ( (uPacketID<<24) + iLen );
	tOut.SendMysqlString ( "def" ); // catalog
	tOut.SendMysqlString ( sDB ); // db
	tOut.SendMysqlString ( sTable ); // table
	tOut.SendMysqlString ( sTable ); // org_table
	tOut.SendMysqlString ( sCol ); // name
	tOut.SendMysqlString ( sCol ); // org_name

	tOut.SendByte ( 12 ); // filler, must be 12 (following pseudo-string length)
	tOut.SendByte ( 0x21 ); // charset_nr, 0x21 is utf8
	tOut.SendByte ( 0 ); // charset_nr
	tOut.SendLSBDword ( iColLen ); // length
	tOut.SendByte ( BYTE(eType) ); // type (0=decimal)
	tOut.SendWord ( 0 ); // flags
	tOut.SendByte ( 0 ); // decimals
	tOut.SendWord ( 0 ); // filler
}


// from mysqld_error.h
enum MysqlErrors_e
{
	MYSQL_ERR_UNKNOWN_COM_ERROR			= 1047,
	MYSQL_ERR_SERVER_SHUTDOWN			= 1053,
	MYSQL_ERR_PARSE_ERROR				= 1064,
	MYSQL_ERR_FIELD_SPECIFIED_TWICE		= 1110,
	MYSQL_ERR_NO_SUCH_TABLE				= 1146
};


void SendMysqlErrorPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, const char * sStmt,
	const char * sError, int iCID, MysqlErrors_e iErr )
{
	if ( sError==NULL )
		sError = "(null)";

	LogSphinxqlError ( sStmt, sError, iCID );

	int iErrorLen = strlen(sError)+1; // including the trailing zero
	int iLen = 9 + iErrorLen;
	int iError = iErr; // pretend to be mysql syntax error for now

	// send packet header
	tOut.SendLSBDword ( (uPacketID<<24) + iLen );
	tOut.SendByte ( 0xff ); // field count, always 0xff for error packet
	tOut.SendByte ( (BYTE)( iError & 0xff ) );
	tOut.SendByte ( (BYTE)( iError>>8 ) );

	// send sqlstate (1 byte marker, 5 byte state)
	switch ( iErr )
	{
		case MYSQL_ERR_SERVER_SHUTDOWN:
		case MYSQL_ERR_UNKNOWN_COM_ERROR:
			tOut.SendBytes ( "#08S01", 6 );
			break;
		case MYSQL_ERR_NO_SUCH_TABLE:
			tOut.SendBytes ( "#42S02", 6 );
			break;
		default:
			tOut.SendBytes ( "#42000", 6 );
			break;
	}

	// send error message
	tOut.SendBytes ( sError, iErrorLen );
}

void SendMysqlEofPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, int iWarns, bool bMoreResults=false )
{
	if ( iWarns<0 ) iWarns = 0;
	if ( iWarns>65535 ) iWarns = 65535;
	if ( bMoreResults )
		iWarns |= ( SPH_MYSQL_FLAG_MORE_RESULTS<<16 );

	tOut.SendLSBDword ( (uPacketID<<24) + 5 );
	tOut.SendByte ( 0xfe );
	tOut.SendLSBDword ( iWarns ); // N warnings, 0 status
}


void SendMysqlOkPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, int iAffectedRows=0, int iWarns=0, const char * sMessage=NULL )
{
	DWORD iInsert_id = 0;
	char sVarLen[20] = {0}; // max 18 for packed number, +1 more just for fun
	void * pBuf = sVarLen;
	pBuf = MysqlPack ( pBuf, iAffectedRows );
	pBuf = MysqlPack ( pBuf, iInsert_id );
	int iLen = (char *) pBuf - sVarLen;

	int iMsgLen = 0;
	if ( sMessage )
		iMsgLen = strlen(sMessage) + 1; // FIXME! does or doesn't the trailing zero necessary in Ok packet?

	tOut.SendLSBDword ( (uPacketID<<24) + iLen + iMsgLen + 5);
	tOut.SendByte ( 0 );				// ok packet
	tOut.SendBytes ( sVarLen, iLen );	// packed affected rows & insert_id
	if ( iWarns<0 ) iWarns = 0;
	if ( iWarns>65535 ) iWarns = 65535;
	DWORD uWarnStatus = iWarns<<16;
	tOut.SendLSBDword ( uWarnStatus );		// N warnings, 0 status
	if ( iMsgLen > 0 )
		tOut.SendBytes ( sMessage, iMsgLen );
}


//////////////////////////////////////////////////////////////////////////
// Mysql row buffer and command handler
#define SPH_MAX_NUMERIC_STR 64
class SqlRowBuffer_c : public ISphNoncopyable
{
public:

	SqlRowBuffer_c ( BYTE * pPacketID, ISphOutputBuffer * pOut, int iCID )
		: m_pBuf ( NULL )
		, m_iLen ( 0 )
		, m_iLimit ( sizeof ( m_dBuf ) )
		, m_uPacketID ( *pPacketID )
		, m_tOut ( *pOut )
		, m_iSize ( 0 )
		, m_iCID ( iCID )
	{}

	~SqlRowBuffer_c ()
	{
		SafeDeleteArray ( m_pBuf );
	}

	char * Reserve ( int iLen )
	{
		int iNewSize = m_iLen+iLen;
		if ( iNewSize<=m_iLimit )
			return Get();

		int iNewLimit = Max ( m_iLimit*2, iNewSize );
		char * pBuf = new char [iNewLimit];
		memcpy ( pBuf, m_pBuf ? m_pBuf : m_dBuf, m_iLen );
		SafeDeleteArray ( m_pBuf );
		m_pBuf = pBuf;
		m_iLimit = iNewLimit;
		return Get();
	}

	char * Get ()
	{
		return m_pBuf ? m_pBuf+m_iLen : m_dBuf+m_iLen;
	}

	char * Off ( int iOff )
	{
		assert ( iOff<m_iLimit );
		return m_pBuf ? m_pBuf+iOff : m_dBuf+iOff;
	}

	int Length () const
	{
		return m_iLen;
	}

	void IncPtr ( int iLen	)
	{
		assert ( m_iLen+iLen<=m_iLimit );
		m_iLen += iLen;
	}

	void Reset ()
	{
		m_iLen = 0;
	}

	template < typename T>
	void PutNumeric ( const char * sFormat, T tVal )
	{
		Reserve ( SPH_MAX_NUMERIC_STR );
		int iLen = snprintf ( Get()+1, SPH_MAX_NUMERIC_STR-1, sFormat, tVal );
		*Get() = BYTE(iLen);
		IncPtr ( 1+iLen );
	}

	void PutString ( const char * sMsg )
	{
		int iLen = 0;
		if ( sMsg && *sMsg )
			iLen = strlen ( sMsg );

		Reserve ( 9+iLen ); // 9 is taken from MysqlPack() implementation (max possible offset)
		char * pBegin = Get();
		char * pStr = (char *)MysqlPack ( pBegin, iLen );
		if ( pStr>pBegin )
		{
			memcpy ( pStr, sMsg, iLen );
			IncPtr ( ( pStr-pBegin )+iLen );
		}
	}

	void PutMicrosec ( int64_t iUsec )
	{
		iUsec = Max ( iUsec, 0 );
		int iSec = (int)( iUsec / 1000000 );
		int iFrac = (int)( iUsec % 1000000 );
		Reserve ( 18 ); // 1..10 bytes for sec, 1 for dot, 6 for frac, 1 for len
		int iLen = snprintf ( Get()+1, 18, "%d.%06d", iSec, iFrac ); //NOLINT
		*Get() = BYTE(iLen);
		IncPtr ( 1+iLen );
	}

	void PutNULL ()
	{
		Reserve ( 1 );
		*( (BYTE *) Get() ) = 0xfb; // MySQL NULL is 0xfb at VLB length
		IncPtr ( 1 );
	}

	/// more high level. Processing the whole tables.
	// sends collected data, then reset
	void Commit()
	{
		m_tOut.SendLSBDword ( ((m_uPacketID++)<<24) + ( Length() ) );
		m_tOut.SendBytes ( Off ( 0 ), Length() );
		Reset();
	}

	// wrappers for popular packets
	inline void Eof ( bool bMoreResults=false, int iWarns=0 )
	{
		SendMysqlEofPacket ( m_tOut, m_uPacketID++, iWarns, bMoreResults );
	}

	inline void Error ( const char * sStmt, const char * sError, MysqlErrors_e iErr = MYSQL_ERR_PARSE_ERROR )
	{
		SendMysqlErrorPacket ( m_tOut, m_uPacketID, sStmt, sError, m_iCID, iErr );
	}

	inline void ErrorEx ( MysqlErrors_e iErr, const char * sTemplate, ... )
	{
		char sBuf[1024];
		va_list ap;

		va_start ( ap, sTemplate );
		vsnprintf ( sBuf, sizeof(sBuf), sTemplate, ap );
		va_end ( ap );

		Error ( NULL, sBuf, iErr );
	}

	inline void Ok ( int iAffectedRows=0, int iWarns=0, const char * sMessage=NULL )
	{
		SendMysqlOkPacket ( m_tOut, m_uPacketID, iAffectedRows, iWarns, sMessage );
	}

	// Header of the table with defined num of columns
	inline void HeadBegin ( int iColumns )
	{
		m_tOut.SendLSBDword ( ((m_uPacketID++)<<24) + MysqlPackedLen ( iColumns ) );
		m_tOut.SendMysqlInt ( iColumns );
		m_iSize = iColumns;
	}

	inline void HeadEnd ( bool bMoreResults=false, int iWarns=0 )
	{
		Eof ( bMoreResults, iWarns );
		Reset();
	}

	// add the next column. The EOF after the tull set will be fired automatically
	inline void HeadColumn ( const char * sName, MysqlColumnType_e uType=MYSQL_COL_STRING )
	{
		assert ( m_iSize>0 && "you try to send more mysql columns than declared in InitHead" );
		SendMysqlFieldPacket ( m_tOut, m_uPacketID++, sName, uType );
		--m_iSize;
	}

	// Fire he header for table with iSize string columns
	inline void HeadOfStrings ( const char ** ppNames, size_t iSize )
	{
		HeadBegin(iSize);
		for ( ; iSize>0 ; --iSize )
			HeadColumn ( *ppNames++ );
		HeadEnd();
	}

	// table of 2 columns (we really often use them!)
	inline void HeadTuplet ( const char * pLeft, const char * pRight )
	{
		HeadBegin(2);
		HeadColumn(pLeft);
		HeadColumn(pRight);
		HeadEnd();
	}

	// popular pattern of 2 columns of data
	inline void DataTuplet ( const char * pLeft, const char * pRight )
	{
		PutString ( pLeft );
		PutString ( pRight );
		Commit();
	}

	inline void DataTuplet ( const char * pLeft, int64_t iRight )
	{
		char sTmp[SPH_MAX_NUMERIC_STR];
		snprintf ( sTmp, sizeof(sTmp), INT64_FMT, iRight );
		DataTuplet ( pLeft, sTmp );
	}

private:
	char m_dBuf[4096];
	char * m_pBuf;
	int m_iLen;
	int m_iLimit;

private:
	BYTE &				m_uPacketID;
	ISphOutputBuffer & m_tOut;
	size_t				m_iSize;
	int					m_iCID; // connection ID for error report
};

class TableLike : public CheckLike
				, public ISphNoncopyable
{
	SqlRowBuffer_c & m_tOut;
public:

	explicit TableLike ( SqlRowBuffer_c & tOut, const char * sPattern = NULL )
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
	inline void MatchDataTuplet ( const char * pLeft, const char * pRight )
	{
		if ( Match ( pLeft ) )
			m_tOut.DataTuplet ( pLeft, pRight );
	}

	inline void MatchDataTuplet ( const char * pLeft, int iRight )
	{
		if ( Match ( pLeft ) )
			m_tOut.DataTuplet ( pLeft, iRight );
	}
};

class CSphSessionAccum
{
public:
	explicit CSphSessionAccum ( bool bManage );
	~CSphSessionAccum();
	ISphRtAccum * GetAcc ( ISphRtIndex * pIndex, CSphString & sError );
	ISphRtIndex * GetIndex ();

private:
	ISphRtAccum *		m_pAcc;
	bool				m_bManage;
};


void HandleMysqlInsert ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt,
	bool bReplace, bool bCommit, CSphString & sWarning, CSphSessionAccum & tAcc )
{
	MEMORY ( MEM_SQL_INSERT );

	CSphString sError;

	// get that index
	const ServedIndex_c * pServed = g_pLocalIndexes->GetRlockedEntry ( tStmt.m_sIndex );
	if ( !pServed )
	{
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( !pServed->m_bRT || !pServed->m_bEnabled )
	{
		pServed->Unlock();
		sError.SetSprintf ( "index '%s' does not support INSERT (enabled=%d)", tStmt.m_sIndex.cstr(), pServed->m_bEnabled );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	ISphRtIndex * pIndex = (ISphRtIndex*)pServed->m_pIndex;

	// get schema, check values count
	const CSphSchema & tSchema = pIndex->GetInternalSchema();
	int iSchemaSz = tSchema.GetAttrsCount() + tSchema.m_dFields.GetLength() + 1;
	if ( pIndex->GetSettings().m_bIndexFieldLens )
		iSchemaSz -= tSchema.m_dFields.GetLength();
	int iExp = tStmt.m_iSchemaSz;
	int iGot = tStmt.m_dInsertValues.GetLength();
	if ( !tStmt.m_dInsertSchema.GetLength() && ( iSchemaSz!=tStmt.m_iSchemaSz ) )
	{
		pServed->Unlock();
		sError.SetSprintf ( "column count does not match schema (expected %d, got %d)", iSchemaSz, iGot );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	if ( ( iGot % iExp )!=0 )
	{
		pServed->Unlock();
		sError.SetSprintf ( "column count does not match value count (expected %d, got %d)", iExp, iGot );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	ISphRtAccum * pAccum = tAcc.GetAcc ( pIndex, sError );
	if ( !sError.IsEmpty() )
	{
		pServed->Unlock();
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	CSphVector<int> dAttrSchema ( tSchema.GetAttrsCount() );
	CSphVector<int> dFieldSchema ( tSchema.m_dFields.GetLength() );
	int iIdIndex = 0;
	if ( !tStmt.m_dInsertSchema.GetLength() )
	{
		// no columns list, use index schema
		ARRAY_FOREACH ( i, dFieldSchema )
			dFieldSchema[i] = i+1;
		int iFields = dFieldSchema.GetLength();
		ARRAY_FOREACH ( j, dAttrSchema )
			dAttrSchema[j] = j+iFields+1;
	} else
	{
		// got a list of columns, check for 1) existance, 2) dupes
		CSphVector<CSphString> dCheck = tStmt.m_dInsertSchema;
		ARRAY_FOREACH ( i, dCheck )
			// OPTIMIZE! GetAttrIndex and GetFieldIndex use the linear searching. M.b. hash instead?
			if ( dCheck[i]!="id" && tSchema.GetAttrIndex ( dCheck[i].cstr() )==-1 && tSchema.GetFieldIndex ( dCheck[i].cstr() )==-1 )
			{
				pServed->Unlock();
				sError.SetSprintf ( "unknown column: '%s'", dCheck[i].cstr() );
				tOut.Error ( tStmt.m_sStmt, sError.cstr(), MYSQL_ERR_PARSE_ERROR );
				return;
			}

		dCheck.Sort();
		for ( int i=1; i<dCheck.GetLength(); i++ )
			if ( dCheck[i-1]==dCheck[i] )
			{
				pServed->Unlock();
				sError.SetSprintf ( "column '%s' specified twice", dCheck[i].cstr() );
				tOut.Error ( tStmt.m_sStmt, sError.cstr(), MYSQL_ERR_FIELD_SPECIFIED_TWICE );
				return;
			}

		// hash column list
		// OPTIMIZE! hash index columns once (!) instead
		SmallStringHash_T<int> dInsertSchema;
		ARRAY_FOREACH ( i, tStmt.m_dInsertSchema )
			dInsertSchema.Add ( i, tStmt.m_dInsertSchema[i] );

		// get id index
		if ( !dInsertSchema.Exists("id") )
		{
			pServed->Unlock();
			tOut.Error ( tStmt.m_sStmt, "column list must contain an 'id' column" );
			return;
		}
		iIdIndex = dInsertSchema["id"];

		// map fields
		bool bIdDupe = false;
		ARRAY_FOREACH ( i, dFieldSchema )
		{
			if ( dInsertSchema.Exists ( tSchema.m_dFields[i].m_sName ) )
			{
				int iField = dInsertSchema[tSchema.m_dFields[i].m_sName];
				if ( iField==iIdIndex )
				{
					bIdDupe = true;
					break;
				}
				dFieldSchema[i] = iField;
			} else
				dFieldSchema[i] = -1;
		}
		if ( bIdDupe )
		{
			pServed->Unlock();
			tOut.Error ( tStmt.m_sStmt, "fields must never be named 'id' (fix your config)" );
			return;
		}

		// map attrs
		ARRAY_FOREACH ( j, dAttrSchema )
		{
			if ( dInsertSchema.Exists ( tSchema.GetAttr(j).m_sName ) )
			{
				int iField = dInsertSchema[tSchema.GetAttr(j).m_sName];
				if ( iField==iIdIndex )
				{
					bIdDupe = true;
					break;
				}
				dAttrSchema[j] = iField;
			} else
				dAttrSchema[j] = -1;
		}
		if ( bIdDupe )
		{
			pServed->Unlock();
			sError.SetSprintf ( "attributes must never be named 'id' (fix your config)" );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
	}

	CSphVector<const char *> dStrings;
	CSphVector<DWORD> dMvas;
	CSphScopedPtr<ISphTokenizer> pTokenizer ( pIndex->CloneIndexingTokenizer() );

	// convert attrs
	for ( int c=0; c<tStmt.m_iRowsAffected; c++ )
	{
		assert ( sError.IsEmpty() );

		CSphMatchVariant tDoc;
		tDoc.Reset ( tSchema.GetRowSize() );
		tDoc.m_uDocID = CSphMatchVariant::ToDocid ( tStmt.m_dInsertValues[iIdIndex + c * iExp] );
		dStrings.Resize ( 0 );
		dMvas.Resize ( 0 );

		int iSchemaAttrCount = tSchema.GetAttrsCount();
		if ( pIndex->GetSettings().m_bIndexFieldLens )
			iSchemaAttrCount -= tSchema.m_dFields.GetLength();
		for ( int i=0; i<iSchemaAttrCount; i++ )
		{
			// shortcuts!
			const CSphColumnInfo & tCol = tSchema.GetAttr(i);
			CSphAttrLocator tLoc = tCol.m_tLocator;
			tLoc.m_bDynamic = true;

			int iQuerySchemaIdx = dAttrSchema[i];
			bool bResult;
			if ( iQuerySchemaIdx < 0 )
			{
				bResult = tDoc.SetDefaultAttr ( tLoc, tCol.m_eAttrType );
				if ( tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_JSON )
					dStrings.Add ( NULL );
				if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET )
					dMvas.Add ( 0 );
			} else
			{
				const SqlInsert_t & tVal = tStmt.m_dInsertValues[iQuerySchemaIdx + c * iExp];

				// sanity checks
				if ( tVal.m_iType!=TOK_QUOTED_STRING && tVal.m_iType!=TOK_CONST_INT && tVal.m_iType!=TOK_CONST_FLOAT && tVal.m_iType!=TOK_CONST_MVA )
				{
					sError.SetSprintf ( "row %d, column %d: internal error: unknown insval type %d", 1+c, 1+iQuerySchemaIdx, tVal.m_iType ); // 1 for human base
					break;
				}
				if ( tVal.m_iType==TOK_CONST_MVA && !( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET ) )
				{
					sError.SetSprintf ( "row %d, column %d: MVA value specified for a non-MVA column", 1+c, 1+iQuerySchemaIdx ); // 1 for human base
					break;
				}
				if ( ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET ) && tVal.m_iType!=TOK_CONST_MVA )
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
					if ( tVal.m_pVals.Ptr() )
					{
						tVal.m_pVals->Uniq();
						iLen = tVal.m_pVals->GetLength();
					}
					if ( tCol.m_eAttrType==SPH_ATTR_INT64SET )
					{
						dMvas.Add ( iLen*2 );
						for ( int j=0; j<iLen; j++ )
						{
							uint64_t uVal = ( *tVal.m_pVals.Ptr() )[j];
							DWORD uLow = (DWORD)uVal;
							DWORD uHi = (DWORD)( uVal>>32 );
							dMvas.Add ( uLow );
							dMvas.Add ( uHi );
						}
					} else
					{
						dMvas.Add ( iLen );
						for ( int j=0; j<iLen; j++ )
							dMvas.Add ( (DWORD)( *tVal.m_pVals.Ptr() )[j] );
					}
				}

				// FIXME? index schema is lawfully static, but our temp match obviously needs to be dynamic
				bResult = tDoc.SetAttr ( tLoc, tVal, tCol.m_eAttrType );
				if ( tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_JSON )
					dStrings.Add ( tVal.m_sVal.cstr() );
			}

			if ( !bResult )
			{
				sError.SetSprintf ( "internal error: unknown attribute type in INSERT (typeid=%d)", tCol.m_eAttrType );
				break;
			}
		}
		if ( !sError.IsEmpty() )
			break;

		// convert fields
		CSphVector<const char*> dFields;
		ARRAY_FOREACH ( i, tSchema.m_dFields )
		{
			int iQuerySchemaIdx = dFieldSchema[i];
			if ( iQuerySchemaIdx < 0 )
				dFields.Add ( "" ); // default value
			else
			{
				if ( tStmt.m_dInsertValues [ iQuerySchemaIdx + c * iExp ].m_iType!=TOK_QUOTED_STRING )
				{
					sError.SetSprintf ( "row %d, column %d: string expected", 1+c, 1+iQuerySchemaIdx ); // 1 for human base
					break;
				}
				dFields.Add ( tStmt.m_dInsertValues[ iQuerySchemaIdx + c * iExp ].m_sVal.cstr() );
			}
		}
		if ( !sError.IsEmpty() )
			break;

		// do add
		pIndex->AddDocument ( pTokenizer.Ptr(), dFields.GetLength(), dFields.Begin(), tDoc,
			bReplace, tStmt.m_sStringParam,
			dStrings.Begin(), dMvas, sError, sWarning, pAccum );

		if ( !sError.IsEmpty() )
			break;
	}

	// fire exit
	if ( !sError.IsEmpty() )
	{
		pIndex->RollBack ( pAccum ); // clean up collected data
		pServed->Unlock();
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// no errors so far
	if ( bCommit )
		pIndex->Commit ( NULL, pAccum );

	pServed->Unlock();

	// my OK packet
	tOut.Ok ( tStmt.m_iRowsAffected, sWarning.IsEmpty() ? 0 : 1 );
}


// our copy of enum_server_command
// we can't rely on mysql_com.h because it might be unavailable
//
// MYSQL_COM_SLEEP = 0
// MYSQL_COM_QUIT = 1
// MYSQL_COM_INIT_DB = 2
// MYSQL_COM_QUERY = 3
// MYSQL_COM_FIELD_LIST = 4
// MYSQL_COM_CREATE_DB = 5
// MYSQL_COM_DROP_DB = 6
// MYSQL_COM_REFRESH = 7
// MYSQL_COM_SHUTDOWN = 8
// MYSQL_COM_STATISTICS = 9
// MYSQL_COM_PROCESS_INFO = 10
// MYSQL_COM_CONNECT = 11
// MYSQL_COM_PROCESS_KILL = 12
// MYSQL_COM_DEBUG = 13
// MYSQL_COM_PING = 14
// MYSQL_COM_TIME = 15
// MYSQL_COM_DELAYED_INSERT = 16
// MYSQL_COM_CHANGE_USER = 17
// MYSQL_COM_BINLOG_DUMP = 18
// MYSQL_COM_TABLE_DUMP = 19
// MYSQL_COM_CONNECT_OUT = 20
// MYSQL_COM_REGISTER_SLAVE = 21
// MYSQL_COM_STMT_PREPARE = 22
// MYSQL_COM_STMT_EXECUTE = 23
// MYSQL_COM_STMT_SEND_LONG_DATA = 24
// MYSQL_COM_STMT_CLOSE = 25
// MYSQL_COM_STMT_RESET = 26
// MYSQL_COM_SET_OPTION = 27
// MYSQL_COM_STMT_FETCH = 28

enum
{
	MYSQL_COM_QUIT		= 1,
	MYSQL_COM_INIT_DB	= 2,
	MYSQL_COM_QUERY		= 3,
	MYSQL_COM_PING		= 14,
	MYSQL_COM_SET_OPTION	= 27
};


void HandleMysqlCallSnippets ( SqlRowBuffer_c & tOut, SqlStmt_t & tStmt, ThdDesc_t * pThd )
{
	CSphString sError;

	// check arguments
	// string data, string index, string query, [named opts]
	if ( tStmt.m_dInsertValues.GetLength()!=3 )
	{
		tOut.Error ( tStmt.m_sStmt, "SNIPPETS() expectes exactly 3 arguments (data, index, query)" );
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

	ExcerptQuery_t q;
	q.m_sWords = tStmt.m_dInsertValues[2].m_sVal;

	ARRAY_FOREACH ( i, tStmt.m_dCallOptNames )
	{
		CSphString & sOpt = tStmt.m_dCallOptNames[i];
		const SqlInsert_t & v = tStmt.m_dCallOptValues[i];

		sOpt.ToLower();
		int iExpType = -1;

		if ( sOpt=="before_match" )				{ q.m_sBeforeMatch = v.m_sVal; iExpType = TOK_QUOTED_STRING; }
		else if ( sOpt=="after_match" )			{ q.m_sAfterMatch = v.m_sVal; iExpType = TOK_QUOTED_STRING; }
		else if ( sOpt=="chunk_separator" )		{ q.m_sChunkSeparator = v.m_sVal; iExpType = TOK_QUOTED_STRING; }
		else if ( sOpt=="html_strip_mode" )		{ q.m_sStripMode = v.m_sVal; iExpType = TOK_QUOTED_STRING; }
		else if ( sOpt=="passage_boundary" )	{ q.m_sRawPassageBoundary = v.m_sVal; iExpType = TOK_QUOTED_STRING; }

		else if ( sOpt=="limit" )				{ q.m_iLimit = (int)v.m_iVal; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="limit_words" )			{ q.m_iLimitWords = (int)v.m_iVal; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="limit_passages" )		{ q.m_iLimitPassages = (int)v.m_iVal; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="around" )				{ q.m_iAround = (int)v.m_iVal; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="start_passage_id" )	{ q.m_iPassageId = (int)v.m_iVal; iExpType = TOK_CONST_INT; }

		else if ( sOpt=="exact_phrase" )		{ q.m_bExactPhrase = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="use_boundaries" )		{ q.m_bUseBoundaries = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="weight_order" )		{ q.m_bWeightOrder = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="query_mode" )			{ q.m_bHighlightQuery = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="force_all_words" )		{ q.m_bForceAllWords = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="load_files" )			{ q.m_iLoadFiles = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="load_files_scattered" ) { q.m_iLoadFiles |= ( v.m_iVal!=0 )?2:0; iExpType = TOK_CONST_INT; }
		else if ( sOpt=="allow_empty" )			{ q.m_bAllowEmpty = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
		else if ( sOpt=="emit_zones" )			{ q.m_bEmitZones = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }

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

	if ( q.m_iLoadFiles )
		q.m_sFilePrefix = g_sSnippetsFilePrefix;

	q.m_ePassageSPZ = sphGetPassageBoundary ( q.m_sRawPassageBoundary );

	if ( !sphCheckOptionsSPZ ( q, q.m_sRawPassageBoundary, sError ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	q.m_bHasBeforePassageMacro = SnippetTransformPassageMacros ( q.m_sBeforeMatch, q.m_sBeforeMatchPassage );
	q.m_bHasAfterPassageMacro = SnippetTransformPassageMacros ( q.m_sAfterMatch, q.m_sAfterMatchPassage );
	q.m_iRawFlags = GetRawSnippetFlags ( q );

	CSphVector<ExcerptQuery_t> dQueries;
	if ( tStmt.m_dInsertValues[0].m_iType==TOK_QUOTED_STRING )
	{
		q.m_sSource = tStmt.m_dInsertValues[0].m_sVal; // OPTIMIZE?
		dQueries.Add ( q );
	} else
	{
		dQueries.Resize ( tStmt.m_dCallStrings.GetLength() );
		ARRAY_FOREACH ( i, tStmt.m_dCallStrings )
		{
			dQueries[i] = q; // copy the settings
			dQueries[i].m_sSource = tStmt.m_dCallStrings[i]; // OPTIMIZE?
		}
	}

	// FIXME!!! SphinxQL but need to provide data size too
	if ( !MakeSnippets ( sIndex, dQueries, sError, pThd ) )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	CSphVector<const char*> dResults ( dQueries.GetLength() );
	ARRAY_FOREACH ( i, dResults )
		dResults[i] = (const char *)dQueries[i].m_dRes.Begin();

	bool bGotData = ARRAY_ANY ( bGotData, dResults, dResults[_any]!=NULL );
	if ( !bGotData )
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
	ARRAY_FOREACH ( i, dResults )
	{
		tOut.PutString ( dResults[i] ? dResults[i] : "" );
		tOut.Commit();
	}
	tOut.Eof();
}


void HandleMysqlCallKeywords ( SqlRowBuffer_c & tOut, SqlStmt_t & tStmt )
{
	CSphString sError;

	// string query, string index, [bool hits]
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

	const ServedIndex_c * pServed = g_pLocalIndexes->GetRlockedEntry ( tStmt.m_dInsertValues[1].m_sVal );
	if ( !pServed || !pServed->m_bEnabled || !pServed->m_pIndex )
	{
		if ( pServed )
			pServed->Unlock();

		pServed = g_pTemplateIndexes->GetRlockedEntry ( tStmt.m_dInsertValues[1].m_sVal );
		if ( !pServed || !pServed->m_bEnabled || !pServed->m_pIndex )
		{
			sError.SetSprintf ( "no such index %s", tStmt.m_dInsertValues[1].m_sVal.cstr() );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			if ( pServed )
				pServed->Unlock();
			return;
		}
	}

	CSphVector<CSphKeywordInfo> dKeywords;
	bool bStats = ( iArgs==3 && tStmt.m_dInsertValues[2].m_iVal!=0 );
	bool bRes = pServed->m_pIndex->GetKeywords ( dKeywords, tStmt.m_dInsertValues[0].m_sVal.cstr(), bStats, &sError );
	pServed->Unlock ();

	if ( !bRes )
	{
		sError.SetSprintf ( "keyword extraction failed: %s", sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// result set header packet
	tOut.HeadBegin ( bStats ? 5 : 3 );
	tOut.HeadColumn("qpos");
	tOut.HeadColumn("tokenized");
	tOut.HeadColumn("normalized");
	if ( bStats )
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
		tOut.PutString ( dKeywords[i].m_sTokenized.cstr() );
		tOut.PutString ( dKeywords[i].m_sNormalized.cstr() );
		if ( bStats )
		{
			snprintf ( sBuf, sizeof(sBuf), "%d", dKeywords[i].m_iDocs );
			tOut.PutString ( sBuf );
			snprintf ( sBuf, sizeof(sBuf), "%d", dKeywords[i].m_iHits );
			tOut.PutString ( sBuf );
		}
		tOut.Commit();
	}
	tOut.Eof();
}


void HandleMysqlDescribe ( SqlRowBuffer_c & tOut, SqlStmt_t & tStmt )
{
	const ServedIndex_c * pServed = g_pLocalIndexes->GetRlockedEntry ( tStmt.m_sIndex );
	DistributedIndex_t * pDistr = NULL;
	if ( !pServed || !pServed->m_bEnabled || !pServed->m_pIndex )
	{
		if ( pServed )
			pServed->Unlock();
		pServed = g_pTemplateIndexes->GetRlockedEntry ( tStmt.m_sIndex );
		if ( !pServed || !pServed->m_bEnabled || !pServed->m_pIndex )
		{
			if ( pServed )
				pServed->Unlock();
			g_tDistLock.Lock();
			pDistr = g_hDistIndexes ( tStmt.m_sIndex );
			g_tDistLock.Unlock();

			if ( !pDistr )
			{
				CSphString sError;
				sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
				tOut.Error ( tStmt.m_sStmt, sError.cstr(), MYSQL_ERR_NO_SUCH_TABLE );
				return;
			}
		}
	}

	TableLike dCondOut ( tOut, tStmt.m_sStringParam.cstr() );

	if ( pDistr )
	{
		tOut.HeadTuplet ( "Agent", "Type" );
		ARRAY_FOREACH ( i, pDistr->m_dLocal )
			dCondOut.MatchDataTuplet ( pDistr->m_dLocal[i].cstr(), "local" );

		ARRAY_FOREACH ( i, pDistr->m_dAgents )
		{
			MetaAgentDesc_t & tAgents = pDistr->m_dAgents[i];
			if ( tAgents.GetAgents().GetLength() > 1 )
			{
				ARRAY_FOREACH ( j, tAgents.GetAgents() )
				{
					CSphString sKey;
					sKey.SetSprintf ( "remote_%d_mirror_%d", i+1, j+1 );
					const AgentDesc_c & dDesc = tAgents.GetAgents()[j];
					CSphString sValue;
					sValue.SetSprintf ( "%s:%s", dDesc.GetMyUrl().cstr(), dDesc.m_sIndexes.cstr() );
					dCondOut.MatchDataTuplet ( sValue.cstr(), sKey.cstr() );
				}
			} else
			{
				CSphString sKey;
				sKey.SetSprintf ( "remote_%d", i+1 );
				CSphString sValue;
				sValue.SetSprintf ( "%s:%s", tAgents.GetAgents()[0].GetMyUrl().cstr(), tAgents.GetAgents()[0].m_sIndexes.cstr() );
				dCondOut.MatchDataTuplet ( sValue.cstr(), sKey.cstr() );
			}
		}

		tOut.Eof();
		return;
	}

	// result set header packet
	tOut.HeadTuplet ( "Field", "Type" );

	// data
	dCondOut.MatchDataTuplet ( "id", USE_64BIT ? "bigint" : "uint" );

	const CSphSchema & tSchema = pServed->m_pIndex->GetMatchSchema();
	ARRAY_FOREACH ( i, tSchema.m_dFields )
		dCondOut.MatchDataTuplet ( tSchema.m_dFields[i].m_sName.cstr(), "field" );

	char sTmp[SPH_MAX_WORD_LEN];
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);
		if ( tCol.m_eAttrType==SPH_ATTR_INTEGER && tCol.m_tLocator.m_iBitCount!=ROWITEM_BITS )
		{
			snprintf ( sTmp, sizeof(sTmp), "%s:%d", sphTypeName ( tCol.m_eAttrType ), tCol.m_tLocator.m_iBitCount );
			dCondOut.MatchDataTuplet ( tCol.m_sName.cstr(), sTmp );
		} else
		{
			dCondOut.MatchDataTuplet ( tCol.m_sName.cstr(), sphTypeName ( tCol.m_eAttrType ) );
		}
	}

	pServed->Unlock();
	tOut.Eof();
}


struct IndexNameLess_fn
{
	inline bool IsLess ( const CSphNamedInt & a, const CSphNamedInt & b ) const
	{
		return strcasecmp ( a.m_sName.cstr(), b.m_sName.cstr() )<0;
	}
};


void HandleMysqlShowTables ( SqlRowBuffer_c & tOut, SqlStmt_t & tStmt )
{
	// result set header packet
	tOut.HeadTuplet ( "Index", "Type" );

	// all the indexes
	// 0 local, 1 distributed, 2 rt
	CSphVector<CSphNamedInt> dIndexes;

	for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
		if ( it.Get().m_bEnabled )
	{
		CSphNamedInt & tIdx = dIndexes.Add();
		tIdx.m_sName = it.GetKey();
		tIdx.m_iValue = it.Get().m_bRT ? 2 : 0;
	}

	for ( IndexHashIterator_c it ( g_pTemplateIndexes ); it.Next(); )
	if ( it.Get().m_bEnabled )
	{
		CSphNamedInt & tIdx = dIndexes.Add();
		tIdx.m_sName = it.GetKey();
		tIdx.m_iValue = 3;
	}

	g_tDistLock.Lock();
	g_hDistIndexes.IterateStart();
	while ( g_hDistIndexes.IterateNext() )
	{
		CSphNamedInt & tIdx = dIndexes.Add();
		tIdx.m_sName = g_hDistIndexes.IterateGetKey();
		tIdx.m_iValue = 1;
	}
	g_tDistLock.Unlock();

	dIndexes.Sort ( IndexNameLess_fn() );

	TableLike dCondOut ( tOut, tStmt.m_sStringParam.cstr() );
	ARRAY_FOREACH ( i, dIndexes )
	{
		const char * sType = "?";
		switch ( dIndexes[i].m_iValue )
		{
			case 0: sType = "local"; break;
			case 1: sType = "distributed"; break;
			case 2: sType = "rt"; break;
			case 3: sType = "template"; break;
		}

		dCondOut.MatchDataTuplet ( dIndexes[i].m_sName.cstr(), sType );
	}

	tOut.Eof();
}

// MySQL Workbench (and maybe other clients) crashes without it
void HandleMysqlShowDatabases ( SqlRowBuffer_c & tOut, SqlStmt_t & )
{
	tOut.HeadBegin ( 1 );
	tOut.HeadColumn ( "Databases" );
	tOut.HeadEnd();
	tOut.Eof();
}


void HandleMysqlShowPlugins ( SqlRowBuffer_c & tOut, SqlStmt_t & )
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
		tOut.PutString ( p.m_sName.cstr() );
		tOut.PutString ( p.m_sLib.cstr() );
		tOut.PutNumeric ( "%d", p.m_iUsers );
		tOut.PutString ( p.m_sExtra.cstr() ? p.m_sExtra.cstr() : "" );
		tOut.Commit();
	}
	tOut.Eof();
}


void HandleMysqlShowThreads ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	int64_t tmNow = sphMicroTimer();

	g_tThdMutex.Lock();
	tOut.HeadBegin ( 5 );
	tOut.HeadColumn ( "Tid" );
	tOut.HeadColumn ( "Proto" );
	tOut.HeadColumn ( "State" );
	tOut.HeadColumn ( "Time" );
	tOut.HeadColumn ( "Info" );
	tOut.HeadEnd();

	const ListNode_t * pIt = g_dThd.Begin();
	while ( pIt!=g_dThd.End() )
	{
		const ThdDesc_t * pThd = (const ThdDesc_t *)pIt;

		int iLen = strnlen ( pThd->m_dBuf.Begin(), pThd->m_dBuf.GetLength() );
		if ( tStmt.m_iThreadsCols>0 && iLen>tStmt.m_iThreadsCols )
		{
			const_cast<ThdDesc_t *>(pThd)->m_dBuf[tStmt.m_iThreadsCols] = '\0';
		}

		tOut.PutNumeric ( "%d", pThd->m_iTid );
		tOut.PutString ( g_dProtoNames [ pThd->m_eProto ] );
		tOut.PutString ( g_dThdStates [ pThd->m_eThdState ] );
		tOut.PutMicrosec ( tmNow - pThd->m_tmStart );
		tOut.PutString ( pThd->m_dBuf.Begin() );

		tOut.Commit();
		pIt = pIt->m_pNext;
	}

	tOut.Eof();
	g_tThdMutex.Unlock();
}


// The pinger
struct PingRequestBuilder_t : public IRequestBuilder_t
{
	explicit PingRequestBuilder_t ( int iCookie = 0 )
		: m_iCookie ( iCookie )
	{}
	virtual void BuildRequest ( AgentConn_t &, NetOutputBuffer_c & tOut ) const
	{
		// header
		tOut.SendWord ( SEARCHD_COMMAND_PING );
		tOut.SendWord ( VER_COMMAND_PING );
		tOut.SendInt ( 4 );
		tOut.SendInt ( m_iCookie );
	}

protected:
	const int m_iCookie;
};

struct PingReplyParser_t : public IReplyParser_t
{
	explicit PingReplyParser_t ( int * pCookie )
		: m_pCookie ( pCookie )
	{}

	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const
	{
		*m_pCookie += tReq.GetDword ();
		return true;
	}

protected:
	int * m_pCookie;
};

void CheckPing()
{
	if ( g_iPingInterval<=0 )
		return;

	static int64_t iLastCheck = 0;
	CSphVector<AgentConn_t> dAgents;
	int64_t iNow = sphMicroTimer();
	if ( (iNow-iLastCheck)<g_iPingInterval*1000 )
		return;

	int iCookie = (int)iNow;
	iLastCheck = iNow;

	g_tStats.m_hDashBoard.IterateStart();
	while ( g_tStats.m_hDashBoard.IterateNext() )
	{
		int iIndex = g_tStats.m_hDashBoard.IterateGet();
		const HostDashboard_t & dDash = g_tStats.m_dDashboard.m_dItemStats[iIndex];
		if ( dDash.m_bNeedPing && dDash.IsOlder(iNow) )
		{
			AgentConn_t & dAgent = dAgents.Add ();
			dAgent = dDash.m_tDescriptor;
		}
	}

	CSphScopedPtr<PingRequestBuilder_t> tReqBuilder ( NULL );
	CSphScopedPtr<ISphRemoteAgentsController> tDistCtrl ( NULL );
	if ( dAgents.GetLength() )
	{
		// connect to remote agents and query them
		tReqBuilder = new PingRequestBuilder_t ( iCookie );
		tDistCtrl = GetAgentsController ( g_iDistThreads, dAgents, *tReqBuilder.Ptr(), g_iPingInterval );
	}

	int iAgentsDone = 0;
	if ( dAgents.GetLength() )
	{
		iAgentsDone = tDistCtrl->Finish();
	}

	int iReplyCookie = 0;
	if ( iAgentsDone )
	{
		PingReplyParser_t tParser ( &iReplyCookie );
		RemoteWaitForAgents ( dAgents, g_iPingInterval, tParser );
	}
}


/////////////////////////////////////////////////////////////////////////////
// user variables these send from master to agents
/////////////////////////////////////////////////////////////////////////////

struct UVarRequestBuilder_t : public IRequestBuilder_t
{
	explicit UVarRequestBuilder_t ( const char * sName, int iUserVars, const BYTE * pBuf, int iLength )
		: m_sName ( sName )
		, m_iUserVars ( iUserVars )
		, m_pBuf ( pBuf )
		, m_iLength ( iLength )
	{}

	virtual void BuildRequest ( AgentConn_t &, NetOutputBuffer_c & tOut ) const
	{
		// header
		tOut.SendWord ( SEARCHD_COMMAND_UVAR );
		tOut.SendWord ( VER_COMMAND_UVAR );
		tOut.SendInt ( strlen ( m_sName ) + 12 + m_iLength );

		tOut.SendString ( m_sName );
		tOut.SendInt ( m_iUserVars );
		tOut.SendInt ( m_iLength );
		tOut.SendBytes ( m_pBuf, m_iLength );
	}

	const char * m_sName;
	int m_iUserVars;
	const BYTE * m_pBuf;
	int m_iLength;
};

struct UVarReplyParser_t : public IReplyParser_t
{
	explicit UVarReplyParser_t ()
	{}

	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const
	{
		// error got handled at call site
		bool bOk = ( tReq.GetByte()==1 );
		return bOk;
	}
};


static void UservarAdd ( const CSphString & sName, CSphVector<SphAttr_t> & dVal );

// create or update the variable
static void SetLocalUserVar ( const CSphString & sName, CSphVector<SphAttr_t> & dSetValues )
{
	g_tUservarsMutex.Lock();
	UservarAdd ( sName, dSetValues );
	g_tmSphinxqlState = sphMicroTimer();
	g_tUservarsMutex.Unlock();
}


static bool SendUserVar ( const char * sIndex, const char * sUserVarName, CSphVector<SphAttr_t> & dSetValues, CSphString & sError )
{
	CSphVector<AgentConn_t> dAgents;
	bool bGotLocal = false;
	g_tDistLock.Lock();
	const DistributedIndex_t * pIndex = g_hDistIndexes ( sIndex );
	if ( pIndex )
	{
		pIndex->GetAllAgents ( &dAgents );
		bGotLocal = ( pIndex->m_dLocal.GetLength()>0 );
	}
	g_tDistLock.Unlock();

	if ( !pIndex )
	{
		sError.SetSprintf ( "unknown index '%s' in Set statement", sIndex );
		return false;
	}

	// FIXME!!! warn on missed agents
	if ( !dAgents.GetLength() && !bGotLocal )
		return true;

	dSetValues.Uniq();

	// FIXME!!! warn on empty agents
	if ( dAgents.GetLength() )
	{
		int iUserVarsCount = dSetValues.GetLength();
		CSphFixedVector<BYTE> dBuf ( iUserVarsCount * sizeof(dSetValues[0]) );
		int iLength = 0;
		if ( iUserVarsCount )
		{
			SphAttr_t iLast = 0;
			BYTE * pCur = dBuf.Begin();
			ARRAY_FOREACH ( i, dSetValues )
			{
				SphAttr_t iDelta = dSetValues[i] - iLast;
				assert ( iDelta>0 );
				iLast = dSetValues[i];
				pCur += sphEncodeVLB8 ( pCur, iDelta );
			}

			iLength = pCur - dBuf.Begin();
		}

		// connect to remote agents and query them
		if ( dAgents.GetLength() )
		{
			// connect to remote agents and query them
			UVarRequestBuilder_t tReqBuilder ( sUserVarName, iUserVarsCount, dBuf.Begin(), iLength );
			CSphScopedPtr<ISphRemoteAgentsController> pDistCtrl ( GetAgentsController ( g_iDistThreads, dAgents, tReqBuilder, g_iPingInterval ) );

			int iAgentsDone = pDistCtrl->Finish();
			if ( iAgentsDone )
			{
				UVarReplyParser_t tParser;
				RemoteWaitForAgents ( dAgents, g_iPingInterval, tParser );
			}
		}
	}

	// should be at the end due to swap of dSetValues values
	if ( bGotLocal )
		SetLocalUserVar ( sUserVarName, dSetValues );

	return true;
}


void HandleCommandUserVar ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_UVAR, tOut ) )
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

	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_UVAR );
	tOut.SendInt ( 4 ); // resplen, 1 dword
	tOut.SendInt ( 1 );
	tOut.Flush ();
	assert ( tOut.GetError() || tOut.GetSentCount()==12 );
}



/////////////////////////////////////////////////////////////////////////////
// SMART UPDATES HANDLER
/////////////////////////////////////////////////////////////////////////////

struct SphinxqlRequestBuilder_t : public IRequestBuilder_t
{
	explicit SphinxqlRequestBuilder_t ( const CSphString& sQuery, const SqlStmt_t & tStmt )
		: m_sBegin ( sQuery.cstr(), tStmt.m_iListStart )
		, m_sEnd ( sQuery.cstr() + tStmt.m_iListEnd, sQuery.Length() - tStmt.m_iListEnd )
	{
	}
	virtual void BuildRequest ( AgentConn_t & tAgent, NetOutputBuffer_c & tOut ) const;

protected:
	const CSphString m_sBegin;
	const CSphString m_sEnd;
};


struct SphinxqlReplyParser_t : public IReplyParser_t
{
	explicit SphinxqlReplyParser_t ( int * pUpd, int * pWarns )
		: m_pUpdated ( pUpd )
		, m_pWarns ( pWarns )
	{}

	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const
	{
		DWORD uSize = ( tReq.GetLSBDword() & 0x00ffffff ) - 1;
		BYTE uCommand = tReq.GetByte();

		if ( uCommand==0 ) // ok packet
		{
			*m_pUpdated += MysqlUnpack ( tReq, &uSize );
			MysqlUnpack ( tReq, &uSize ); ///< int Insert_id (don't used).
			*m_pWarns += tReq.GetLSBDword(); ///< num of warnings
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

protected:
	int * m_pUpdated;
	int * m_pWarns;
};


void SphinxqlRequestBuilder_t::BuildRequest ( AgentConn_t & tAgent, NetOutputBuffer_c & tOut ) const
{
	const char* sIndexes = tAgent.m_sIndexes.cstr();
	int iReqSize = strlen(sIndexes) + m_sBegin.Length() + m_sEnd.Length(); // indexes string

	// header
	tOut.SendWord ( SEARCHD_COMMAND_SPHINXQL );
	tOut.SendWord ( VER_COMMAND_SPHINXQL );
	tOut.SendInt ( iReqSize + 4 );

	tOut.SendInt ( iReqSize );
	tOut.SendBytes ( m_sBegin.cstr(), m_sBegin.Length() );
	tOut.SendBytes ( sIndexes, strlen(sIndexes) );
	tOut.SendBytes ( m_sEnd.cstr(), m_sEnd.Length() );
}

//////////////////////////////////////////////////////////////////////////
static void DoExtendedUpdate ( const char * sIndex, const SqlStmt_t & tStmt,
							int & iSuccesses, int & iUpdated,
							SearchFailuresLog_c & dFails, const ServedIndex_c * pServed, CSphString & sWarning, int iCID )
{
	if ( !pServed || !pServed->m_pIndex || !pServed->m_bEnabled )
	{
		if ( pServed )
			pServed->Unlock();
		dFails.Submit ( sIndex, "index not available" );
		return;
	}

	SearchHandler_c tHandler ( 1, true, false, iCID ); // handler unlocks index at destructor - no need to do it manually
	CSphAttrUpdateEx tUpdate;
	CSphString sError;

	tUpdate.m_pUpdate = &tStmt.m_tUpdate;
	tUpdate.m_pIndex = pServed->m_pIndex;
	tUpdate.m_pError = &sError;
	tUpdate.m_pWarning = &sWarning;

	tHandler.RunUpdates ( tStmt.m_tQuery, sIndex, &tUpdate );

	if ( sError.Length() )
	{
		dFails.Submit ( sIndex, sError.cstr() );
		return;
	}

	iUpdated += tUpdate.m_iAffected;
	iSuccesses++;
}


static const char * ExtractDistributedIndexes ( const CSphVector<CSphString> & dNames, CSphVector<DistributedIndex_t> & dDistributed )
{
	assert ( dNames.GetLength()==dDistributed.GetLength() );
	ARRAY_FOREACH ( i, dNames )
	{
		if ( !g_pLocalIndexes->Exists ( dNames[i] ) )
		{
			// search amongst distributed and copy for further processing
			g_tDistLock.Lock();
			const DistributedIndex_t * pDistIndex = g_hDistIndexes ( dNames[i] );

			if ( pDistIndex )
			{
				dDistributed[i] = *pDistIndex;
			}

			g_tDistLock.Unlock();

			if ( !pDistIndex )
				return dNames[i].cstr();
		}
	}

	return NULL;
}


void HandleMysqlUpdate ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt, const CSphString & sQuery, CSphString & sWarning, int iCID )
{
	CSphString sError;

	// extract index names
	CSphVector<CSphString> dIndexNames;
	ParseIndexList ( tStmt.m_sIndex, dIndexNames );
	if ( !dIndexNames.GetLength() )
	{
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// lock safe storage for distributed indexes
	CSphVector<DistributedIndex_t> dDistributed ( dIndexNames.GetLength() );
	// copy distributed indexes description
	const char * sMissedDist = NULL;
	if ( ( sMissedDist = ExtractDistributedIndexes ( dIndexNames, dDistributed ) )!=NULL )
	{
		sError.SetSprintf ( "unknown index '%s' in update request", sMissedDist );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// do update
	SearchFailuresLog_c dFails;
	int iSuccesses = 0;
	int iUpdated = 0;
	int iWarns = 0;

	bool bMvaUpdate = false;
	ARRAY_FOREACH_COND ( i, tStmt.m_tUpdate.m_dAttrs, !bMvaUpdate )
	{
		bMvaUpdate = ( tStmt.m_tUpdate.m_dTypes[i]==SPH_ATTR_UINT32SET
			|| tStmt.m_tUpdate.m_dTypes[i]==SPH_ATTR_INT64SET );
	}

	ARRAY_FOREACH ( iIdx, dIndexNames )
	{
		const char * sReqIndex = dIndexNames[iIdx].cstr();
		const ServedIndex_c * pLocked = UpdateGetLockedIndex ( sReqIndex, bMvaUpdate );
		if ( pLocked )
		{
			DoExtendedUpdate ( sReqIndex, tStmt, iSuccesses, iUpdated, dFails, pLocked, sWarning, iCID );
		} else
		{
			assert ( dDistributed[iIdx].m_dLocal.GetLength() || dDistributed[iIdx].m_dAgents.GetLength() );
			CSphVector<CSphString>& dLocal = dDistributed[iIdx].m_dLocal;

			ARRAY_FOREACH ( i, dLocal )
			{
				const char * sLocal = dLocal[i].cstr();
				const ServedIndex_c * pServed = UpdateGetLockedIndex ( sLocal, bMvaUpdate );
				DoExtendedUpdate ( sLocal, tStmt, iSuccesses, iUpdated, dFails, pServed, sWarning, iCID );
			}
		}

		// update remote agents
		if ( dDistributed[iIdx].m_dAgents.GetLength() )
		{
			DistributedIndex_t & tDist = dDistributed[iIdx];

			CSphVector<AgentConn_t> dAgents;
			tDist.GetAllAgents ( &dAgents );

			// connect to remote agents and query them
			SphinxqlRequestBuilder_t tReqBuilder ( sQuery, tStmt );
			CSphScopedPtr<ISphRemoteAgentsController> pDistCtrl ( GetAgentsController ( g_iDistThreads, dAgents, tReqBuilder, tDist.m_iAgentConnectTimeout ) );
			int iAgentsDone = pDistCtrl->Finish();
			if ( iAgentsDone )
			{
				SphinxqlReplyParser_t tParser ( &iUpdated, &iWarns );
				iSuccesses += RemoteWaitForAgents ( dAgents, tDist.m_iAgentQueryTimeout, tParser ); // FIXME? profile update time too?
			}
		}
	}

	CSphStringBuilder sReport;
	dFails.BuildReport ( sReport );

	if ( !iSuccesses )
	{
		tOut.Error ( tStmt.m_sStmt, sReport.cstr() );
		return;
	}

	tOut.Ok ( iUpdated, iWarns );
}

bool HandleMysqlSelect ( SqlRowBuffer_c & dRows, SearchHandler_c & tHandler )
{
	// lets check all query for errors
	CSphString sError;
	CSphVector<int64_t> dAgentTimes; // dummy for error reporting
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
	{
		CheckQuery ( tHandler.m_dQueries[i], tHandler.m_dResults[i].m_sError );
		if ( !tHandler.m_dResults[i].m_sError.IsEmpty() )
		{
			LogQuery ( tHandler.m_dQueries[i], tHandler.m_dResults[i], dAgentTimes, tHandler.m_iCid );
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

	if ( g_bGotSigterm )
	{
		sphLogDebug ( "HandleClientMySQL: got SIGTERM, sending the packet MYSQL_ERR_SERVER_SHUTDOWN" );
		dRows.Error ( NULL, "Server shutdown in progress", MYSQL_ERR_SERVER_SHUTDOWN );
		return false;
	}

	return true;
}


void sphFormatFactors ( CSphVector<BYTE> & dOut, const unsigned int * pFactors, bool bJson )
{
	const int MAX_STR_LEN = 512;
	int iLen;
	int iOff = dOut.GetLength();
	dOut.Resize ( iOff+MAX_STR_LEN );
	if ( !bJson )
	{
		iLen = snprintf ( (char *)dOut.Begin()+iOff, MAX_STR_LEN, "bm25=%d, bm25a=%f, field_mask=%u, doc_word_count=%d",
			sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_BM25 ), sphinx_get_doc_factor_float ( pFactors, SPH_DOCF_BM25A ),
			sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_MATCHED_FIELDS ), sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_DOC_WORD_COUNT ) );
	} else
	{
		iLen = snprintf ( (char *)dOut.Begin()+iOff, MAX_STR_LEN, "{\"bm25\":%d, \"bm25a\":%f, \"field_mask\":%u, \"doc_word_count\":%d",
			sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_BM25 ), sphinx_get_doc_factor_float ( pFactors, SPH_DOCF_BM25A ),
			sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_MATCHED_FIELDS ), sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_DOC_WORD_COUNT ) );
	}
	dOut.Resize ( iOff+iLen );

	if ( bJson )
	{
		iOff = dOut.GetLength();
		dOut.Resize ( iOff+MAX_STR_LEN );
		iLen = snprintf ( (char *)dOut.Begin()+iOff, MAX_STR_LEN, ", \"fields\":[" );
		dOut.Resize ( iOff+iLen );
	}

	const unsigned int * pExactHit = sphinx_get_doc_factor_ptr ( pFactors, SPH_DOCF_EXACT_HIT_MASK );
	const unsigned int * pExactOrder = sphinx_get_doc_factor_ptr ( pFactors, SPH_DOCF_EXACT_ORDER_MASK );

	int iFields = sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_NUM_FIELDS );
	bool bFields = false;
	for ( int i = 0; i<iFields; i++ )
	{
		const unsigned int * pField = sphinx_get_field_factors ( pFactors, i );
		int iHits = sphinx_get_field_factor_int ( pField, SPH_FIELDF_HIT_COUNT );
		if ( !iHits )
			continue;

		iOff = dOut.GetLength();
		dOut.Resize ( iOff+MAX_STR_LEN );

		int iFieldPos = i/32;
		int iFieldBit = 1<<( i % 32 );
		int iGotExactHit = ( ( pExactHit [ iFieldPos ] & iFieldBit )==0 ? 0 : 1 );
		int iGotExactOrder = ( ( pExactOrder [ iFieldPos ] & iFieldBit )==0 ? 0 : 1 );

		if ( !bJson )
		{
			iLen = snprintf ( (char *)dOut.Begin()+iOff, MAX_STR_LEN, ", field%d="
				"(lcs=%u, hit_count=%u, word_count=%u, "
				"tf_idf=%f, min_idf=%f, max_idf=%f, sum_idf=%f, "
				"min_hit_pos=%d, min_best_span_pos=%d, exact_hit=%u, max_window_hits=%d, "
				"min_gaps=%d, exact_order=%d, lccs=%d, wlccs=%f, atc=%f)",
				i,
				sphinx_get_field_factor_int ( pField, SPH_FIELDF_LCS ), sphinx_get_field_factor_int ( pField, SPH_FIELDF_HIT_COUNT ), sphinx_get_field_factor_int ( pField, SPH_FIELDF_WORD_COUNT ),
				sphinx_get_field_factor_float ( pField, SPH_FIELDF_TF_IDF ), sphinx_get_field_factor_float ( pField, SPH_FIELDF_MIN_IDF ),
				sphinx_get_field_factor_float ( pField, SPH_FIELDF_MAX_IDF ), sphinx_get_field_factor_float ( pField, SPH_FIELDF_SUM_IDF ),
				sphinx_get_field_factor_int ( pField, SPH_FIELDF_MIN_HIT_POS ), sphinx_get_field_factor_int ( pField, SPH_FIELDF_MIN_BEST_SPAN_POS ),
				iGotExactHit, sphinx_get_field_factor_int ( pField, SPH_FIELDF_MAX_WINDOW_HITS ),
				sphinx_get_field_factor_int ( pField, SPH_FIELDF_MIN_GAPS ), iGotExactOrder,
				sphinx_get_field_factor_int ( pField, SPH_FIELDF_LCCS ), sphinx_get_field_factor_float ( pField, SPH_FIELDF_WLCCS ), sphinx_get_field_factor_float ( pField, SPH_FIELDF_ATC ) );
		} else
		{
			iLen = snprintf ( (char *)dOut.Begin()+iOff, MAX_STR_LEN,
				"%s{\"field\":%d, \"lcs\":%u, \"hit_count\":%u, \"word_count\":%u, "
				"\"tf_idf\":%f, \"min_idf\":%f, \"max_idf\":%f, \"sum_idf\":%f, "
				"\"min_hit_pos\":%d, \"min_best_span_pos\":%d, \"exact_hit\":%u, \"max_window_hits\":%d, "
				"\"min_gaps\":%d, \"exact_order\":%d, \"lccs\":%d, \"wlccs\":%f, \"atc\":%f}",
				bFields ? ", " : "", i, sphinx_get_field_factor_int ( pField, SPH_FIELDF_LCS ), sphinx_get_field_factor_int ( pField, SPH_FIELDF_HIT_COUNT ), sphinx_get_field_factor_int ( pField, SPH_FIELDF_WORD_COUNT ),
				sphinx_get_field_factor_float ( pField, SPH_FIELDF_TF_IDF ), sphinx_get_field_factor_float ( pField, SPH_FIELDF_MIN_IDF ),
				sphinx_get_field_factor_float ( pField, SPH_FIELDF_MAX_IDF ), sphinx_get_field_factor_float ( pField, SPH_FIELDF_SUM_IDF ),
				sphinx_get_field_factor_int ( pField, SPH_FIELDF_MIN_HIT_POS ), sphinx_get_field_factor_int ( pField, SPH_FIELDF_MIN_BEST_SPAN_POS ),
				iGotExactHit, sphinx_get_field_factor_int ( pField, SPH_FIELDF_MAX_WINDOW_HITS ),
				sphinx_get_field_factor_int ( pField, SPH_FIELDF_MIN_GAPS ), iGotExactOrder,
				sphinx_get_field_factor_int ( pField, SPH_FIELDF_LCCS ), sphinx_get_field_factor_float ( pField, SPH_FIELDF_WLCCS ), sphinx_get_field_factor_float ( pField, SPH_FIELDF_ATC ) );
			bFields = true;
		}

		dOut.Resize ( iOff+iLen );
	}

	int iUniqQpos = sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_MAX_UNIQ_QPOS );
	if ( bJson )
	{
		iOff = dOut.GetLength();
		dOut.Resize ( iOff+MAX_STR_LEN );
		iLen = snprintf ( (char *)dOut.Begin()+iOff, MAX_STR_LEN, "], \"words\":[" );
		dOut.Resize ( iOff+iLen );
	}
	bool bWord = false;
	for ( int i = 0; i<iUniqQpos; i++ )
	{
		const unsigned int * pTerm = sphinx_get_term_factors ( pFactors, i+1 );
		int iKeywordMask = sphinx_get_term_factor_int ( pTerm, SPH_TERMF_KEYWORD_MASK );
		if ( !iKeywordMask )
			continue;

		iOff = dOut.GetLength();
		dOut.Resize ( iOff+MAX_STR_LEN );
		if ( !bJson )
		{
			iLen = snprintf ( (char *)dOut.Begin()+iOff, MAX_STR_LEN, ", word%d=(tf=%d, idf=%f)", i,
				sphinx_get_term_factor_int ( pTerm, SPH_TERMF_TF ), sphinx_get_term_factor_float ( pTerm, SPH_TERMF_IDF ) );
		} else
		{
			iLen = snprintf ( (char *)dOut.Begin()+iOff, MAX_STR_LEN, "%s{\"tf\":%d, \"idf\":%f}", ( bWord ? ", " : "" ),
				sphinx_get_term_factor_int ( pTerm, SPH_TERMF_TF ), sphinx_get_term_factor_float ( pTerm, SPH_TERMF_IDF ) );
			bWord = true;
		}
		dOut.Resize ( iOff+iLen );
	}

	if ( bJson )
	{
		iOff = dOut.GetLength();
		dOut.Resize ( iOff+MAX_STR_LEN );
		iLen = snprintf ( (char *)dOut.Begin()+iOff, MAX_STR_LEN, "]}" );
		dOut.Resize ( iOff+iLen );
	}
}


static void ReturnZeroCount ( const CSphRsetSchema & tSchema, int iAttrsCount, const CSphString & sName, SqlRowBuffer_c & dRows )
{
	for ( int i=0; i<iAttrsCount; i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);

		if ( tCol.m_sName==sName ) // @count or its alias
			dRows.PutNumeric<DWORD> ( "%u", 0 );
		else
		{
			// essentially the same as SELECT_DUAL, parse and print constant expressions
			ESphAttr eAttrType;
			CSphString sError;
			ISphExpr * pExpr = sphExprParse ( tCol.m_sName.cstr(), tSchema, &eAttrType, NULL, sError, NULL );

			if ( !pExpr || !pExpr->IsConst() )
				eAttrType = SPH_ATTR_NONE;

			CSphMatch tMatch;
			const BYTE * pStr = NULL;

			switch ( eAttrType )
			{
				case SPH_ATTR_STRINGPTR:
					pExpr->StringEval ( tMatch, &pStr );
					dRows.PutString ( (const char*)pStr );
					SafeDelete ( pStr );
					break;
				case SPH_ATTR_INTEGER:	dRows.PutNumeric<int> ( "%d", pExpr->IntEval ( tMatch ) ); break;
				case SPH_ATTR_BIGINT:	dRows.PutNumeric<SphAttr_t> ( INT64_FMT, pExpr->Int64Eval ( tMatch ) ); break;
				case SPH_ATTR_FLOAT:	dRows.PutNumeric<float> ( "%f", pExpr->Eval ( tMatch ) ); break;
				default:
					dRows.PutNULL();
					break;
			}

			SafeDelete ( pExpr );
		}
	}
	dRows.Commit();
}


void SendMysqlSelectResult ( SqlRowBuffer_c & dRows, const AggrResult_t & tRes, bool bMoreResultsFollow )
{
	if ( !tRes.m_iSuccesses )
	{
		// at this point, SELECT error logging should have been handled, so pass a NULL stmt to logger
		dRows.Error ( NULL, tRes.m_sError.cstr() );
		return;
	}

	// empty result sets just might carry the full uberschema
	// bummer! lets protect ourselves against that
	int iSchemaAttrsCount = 0;
	int iAttrsCount = 1;
	bool bReturnZeroCount = !tRes.m_sZeroCountName.IsEmpty();
	if ( tRes.m_dMatches.GetLength() || bReturnZeroCount )
	{
		iSchemaAttrsCount = SendGetAttrCount ( tRes.m_tSchema );
		iAttrsCount = iSchemaAttrsCount;
	}

	// result set header packet. We will attach EOF manually at the end.
	dRows.HeadBegin ( iAttrsCount );

	// field packets
	if ( !tRes.m_dMatches.GetLength() && !bReturnZeroCount )
	{
		// in case there are no matches, send a dummy schema
		dRows.HeadColumn ( "id", USE_64BIT ? MYSQL_COL_LONGLONG : MYSQL_COL_LONG );
	} else
	{
		for ( int i=0; i<iSchemaAttrsCount; i++ )
		{
			const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
			MysqlColumnType_e eType = MYSQL_COL_STRING;
			if ( tCol.m_eAttrType==SPH_ATTR_INTEGER || tCol.m_eAttrType==SPH_ATTR_TIMESTAMP || tCol.m_eAttrType==SPH_ATTR_BOOL )
				eType = MYSQL_COL_LONG;
			if ( tCol.m_eAttrType==SPH_ATTR_FLOAT )
				eType = MYSQL_COL_FLOAT;
			if ( tCol.m_eAttrType==SPH_ATTR_BIGINT )
				eType = MYSQL_COL_LONGLONG;
			if ( tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_STRINGPTR || tCol.m_eAttrType==SPH_ATTR_FACTORS || tCol.m_eAttrType==SPH_ATTR_FACTORS_JSON )
				eType = MYSQL_COL_STRING;
			dRows.HeadColumn ( tCol.m_sName.cstr(), eType );
		}
	}

	// EOF packet is sent explicitly due to non-default params.
	BYTE iWarns = ( !tRes.m_sWarning.IsEmpty() ) ? 1 : 0;
	dRows.HeadEnd ( bMoreResultsFollow, iWarns );

	// FIXME!!! replace that vector relocations by SqlRowBuffer
	CSphVector<BYTE> dTmp;

	// rows
	for ( int iMatch = tRes.m_iOffset; iMatch < tRes.m_iOffset + tRes.m_iCount; iMatch++ )
	{
		const CSphMatch & tMatch = tRes.m_dMatches [ iMatch ];

		const CSphRsetSchema & tSchema = tRes.m_tSchema;
		for ( int i=0; i<iSchemaAttrsCount; i++ )
		{
			CSphAttrLocator tLoc = tSchema.GetAttr(i).m_tLocator;
			ESphAttr eAttrType = tSchema.GetAttr(i).m_eAttrType;

			switch ( eAttrType )
			{
			case SPH_ATTR_INTEGER:
			case SPH_ATTR_TIMESTAMP:
			case SPH_ATTR_BOOL:
			case SPH_ATTR_TOKENCOUNT:
				dRows.PutNumeric<DWORD> ( "%u", (DWORD)tMatch.GetAttr(tLoc) );
				break;

			case SPH_ATTR_BIGINT:
			{
				const char * sName = tSchema.GetAttr(i).m_sName.cstr();
				// how to get rid of this if?
				if ( sName[0]=='i' && sName[1]=='d' && sName[2]=='\0' )
					dRows.PutNumeric<SphDocID_t> ( DOCID_FMT, tMatch.m_uDocID );
				else
					dRows.PutNumeric<SphAttr_t> ( INT64_FMT, tMatch.GetAttr(tLoc) );
				break;
				}

			case SPH_ATTR_FLOAT:
				dRows.PutNumeric ( "%f", tMatch.GetAttrFloat(tLoc) );
				break;

			case SPH_ATTR_INT64SET:
			case SPH_ATTR_UINT32SET:
				{
					int iLenOff = dRows.Length();
					dRows.Reserve ( 4 );
					dRows.IncPtr ( 4 );

					assert ( tMatch.GetAttr ( tLoc )==0 || tRes.m_dTag2Pools [ tMatch.m_iTag ].m_pMva || ( MVA_DOWNSIZE ( tMatch.GetAttr ( tLoc ) ) & MVA_ARENA_FLAG ) );
					const PoolPtrs_t & tPools = tRes.m_dTag2Pools [ tMatch.m_iTag ];
					const DWORD * pValues = tMatch.GetAttrMVA ( tLoc, tPools.m_pMva, tPools.m_bArenaProhibit );
					if ( pValues )
					{
						DWORD nValues = *pValues++;
						assert ( eAttrType==SPH_ATTR_UINT32SET || ( nValues%2 )==0 );
						if ( eAttrType==SPH_ATTR_UINT32SET )
						{
							while ( nValues-- )
							{
								dRows.Reserve ( SPH_MAX_NUMERIC_STR );
								int iLen = snprintf ( dRows.Get(), SPH_MAX_NUMERIC_STR, nValues>0 ? "%u," : "%u", *pValues++ );
								dRows.IncPtr ( iLen );
							}
						} else
						{
							for ( ; nValues; nValues-=2, pValues+=2 )
							{
								int64_t iVal = MVA_UPSIZE ( pValues );
								dRows.Reserve ( SPH_MAX_NUMERIC_STR );
								int iLen = snprintf ( dRows.Get(), SPH_MAX_NUMERIC_STR, nValues>2 ? INT64_FMT"," : INT64_FMT, iVal );
								dRows.IncPtr ( iLen );
							}
						}
					}

					// manually pack length, forcibly into exactly 3 bytes
					int iLen = dRows.Length()-iLenOff-4;
					char * pLen = dRows.Off ( iLenOff );
					pLen[0] = (BYTE)0xfd;
					pLen[1] = (BYTE)( iLen & 0xff );
					pLen[2] = (BYTE)( ( iLen>>8 ) & 0xff );
					pLen[3] = (BYTE)( ( iLen>>16 ) & 0xff );
					break;
				}

			case SPH_ATTR_STRING:
			case SPH_ATTR_JSON:
				{
					const BYTE * pStrings = tRes.m_dTag2Pools [ tMatch.m_iTag ].m_pStrings;

					// get that string
					const BYTE * pStr = NULL;
					int iLen = 0;

					DWORD uOffset = (DWORD) tMatch.GetAttr ( tLoc );
					if ( uOffset )
					{
						assert ( pStrings );
						iLen = sphUnpackStr ( pStrings+uOffset, &pStr );
					}

					if ( eAttrType==SPH_ATTR_JSON )
					{
						// no object at all? return NULL
						if ( !pStr )
						{
							dRows.PutNULL();
							break;
						}
						dTmp.Resize ( 0 );
						sphJsonFormat ( dTmp, pStr );
						pStr = dTmp.Begin();
						iLen = dTmp.GetLength();
						if ( iLen==0 )
						{
							// empty string (no objects) - return NULL
							// (canonical "{}" and "[]" are handled by sphJsonFormat)
							dRows.PutNULL();
							break;
						}
					}

					// send length
					dRows.Reserve ( iLen+4 );
					char * pOutStr = (char*)MysqlPack ( dRows.Get(), iLen );

					// send string data
					if ( iLen )
						memcpy ( pOutStr, pStr, iLen );

					dRows.IncPtr ( pOutStr-dRows.Get()+iLen );
					break;
				}

			case SPH_ATTR_STRINGPTR:
				{
					int iLen = 0;
					const char* pString = (const char*) tMatch.GetAttr ( tLoc );
					if ( pString )
						iLen = strlen ( pString );
					else
					{
						// stringptr is NULL - send NULL value
						dRows.PutNULL();
						break;
					}

					// send length
					dRows.Reserve ( iLen+4 );
					char * pOutStr = (char*)MysqlPack ( dRows.Get(), iLen );

					// send string data
					if ( iLen )
						memcpy ( pOutStr, pString, iLen );

					dRows.IncPtr ( pOutStr-dRows.Get()+iLen );
					break;
				}

			case SPH_ATTR_FACTORS:
			case SPH_ATTR_FACTORS_JSON:
				{
					int iLen = 0;
					const BYTE * pStr = NULL;
					const unsigned int * pFactors = (unsigned int*) tMatch.GetAttr ( tLoc );
					if ( pFactors )
					{
						dTmp.Resize ( 0 );
						sphFormatFactors ( dTmp, pFactors, eAttrType==SPH_ATTR_FACTORS_JSON );
						iLen = dTmp.GetLength();
						pStr = dTmp.Begin();
					}

					// send length
					dRows.Reserve ( iLen+4 );
					char * pOutStr = (char*)MysqlPack ( dRows.Get(), iLen );

					// send string data
					if ( iLen )
						memcpy ( pOutStr, pStr, iLen );

					dRows.IncPtr ( pOutStr-dRows.Get()+iLen );
					break;
				}

			case SPH_ATTR_JSON_FIELD:
				{
					uint64_t uTypeOffset = tMatch.GetAttr ( tLoc );
					ESphJsonType eJson = ESphJsonType ( uTypeOffset>>32 );
					DWORD uOff = (DWORD)uTypeOffset;
					if ( !uOff || eJson==JSON_NULL )
					{
						// no key found - NULL value
						dRows.PutNULL();

					} else
					{
						// send string to client
						dTmp.Resize ( 0 );
						const BYTE * pStrings = tRes.m_dTag2Pools [ tMatch.m_iTag ].m_pStrings;
						sphJsonFieldFormat ( dTmp, pStrings+uOff, eJson, false );

						// send length
						int iLen = dTmp.GetLength();
						dRows.Reserve ( iLen+4 );
						char * pOutStr = (char*)MysqlPack ( dRows.Get(), iLen );

						// send string data
						if ( iLen )
							memcpy ( pOutStr, dTmp.Begin(), iLen );

						dRows.IncPtr ( pOutStr-dRows.Get()+iLen );
					}
					break;
				}

			default:
				char * pDef = dRows.Reserve ( 2 );
				pDef[0] = 1;
				pDef[1] = '-';
				dRows.IncPtr ( 2 );
				break;
			}
		}
		dRows.Commit();
	}

	if ( bReturnZeroCount )
		ReturnZeroCount ( tRes.m_tSchema, iSchemaAttrsCount, tRes.m_sZeroCountName, dRows );

	// eof packet
	dRows.Eof ( bMoreResultsFollow, iWarns );
}


void HandleMysqlWarning ( const CSphQueryResultMeta & tLastMeta, SqlRowBuffer_c & dRows, bool bMoreResultsFollow )
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
	dRows.PutString ( tLastMeta.m_sWarning.cstr() );
	dRows.Commit();

	// cleanup
	dRows.Eof ( bMoreResultsFollow );
}

void HandleMysqlMeta ( SqlRowBuffer_c & dRows, const SqlStmt_t & tStmt, const CSphQueryResultMeta & tLastMeta, bool bMoreResultsFollow )
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


static int LocalIndexDoDeleteDocuments ( const char * sName, const SqlStmt_t & tStmt, const SphDocID_t * pDocs, int iCount,
											const ServedIndex_c * pLocked, SearchFailuresLog_c & dErrors, bool bCommit, CSphSessionAccum & tAcc, int iCID )
{
	if ( !pLocked || !pLocked->m_pIndex )
	{
		if ( pLocked )
			pLocked->Unlock();
		dErrors.Submit ( sName, "index not available" );
		return 0;
	}

	CSphString sError;
	ISphRtIndex * pIndex = static_cast<ISphRtIndex *> ( pLocked->m_pIndex );
	if ( !pLocked->m_bRT || !pLocked->m_bEnabled )
	{
		sError.SetSprintf ( "does not support DELETE (enabled=%d)", pLocked->m_bEnabled );
		dErrors.Submit ( sName, sError.cstr() );
		pLocked->Unlock();
		return 0;
	}

	ISphRtAccum * pAccum = tAcc.GetAcc ( pIndex, sError );
	if ( !sError.IsEmpty() )
	{
		dErrors.Submit ( sName, sError.cstr() );
		pLocked->Unlock();
		return 0;
	}

	SearchHandler_c * pHandler = NULL;
	CSphVector<SphDocID_t> dValues;
	if ( !pDocs ) // needs to be deleted via query
	{
		pHandler = new SearchHandler_c ( 1, true, false, iCID ); // handler unlocks index at destructor - no need to do it manually
		pHandler->RunDeletes ( tStmt.m_tQuery, sName, &sError, &dValues );
		pDocs = dValues.Begin();
		iCount = dValues.GetLength();
	}

	if ( !pIndex->DeleteDocument ( pDocs, iCount, sError, pAccum ) )
	{
		dErrors.Submit ( sName, sError.cstr() );
		if ( pHandler )
			delete pHandler;
		else
			pLocked->Unlock();
		return 0;
	}

	int iAffected = 0;
	if ( bCommit )
		pIndex->Commit ( &iAffected, pAccum );

	if ( pHandler )
		delete pHandler;
	else
		pLocked->Unlock();
	return iAffected;
}


void HandleMysqlDelete ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt, const CSphString & sQuery, bool bCommit, CSphSessionAccum & tAcc, int iCID )
{
	MEMORY ( MEM_SQL_DELETE );

	CSphString sError;

	CSphVector<CSphString> dNames;
	ParseIndexList ( tStmt.m_sIndex, dNames );
	if ( !dNames.GetLength() )
	{
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	CSphVector<DistributedIndex_t> dDistributed ( dNames.GetLength() );
	const char * sMissedDist = NULL;
	if ( ( sMissedDist = ExtractDistributedIndexes ( dNames, dDistributed ) )!=NULL )
	{
		sError.SetSprintf ( "unknown index '%s' in delete request", sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// delete to agents works only with commit=1
	if ( !bCommit && dDistributed.GetLength() )
	{
		ARRAY_FOREACH ( i, dDistributed )
		{
			if ( !dDistributed[i].m_dAgents.GetLength() )
				continue;

			sError.SetSprintf ( "index '%s': DELETE not working on agents when autocommit=0", tStmt.m_sIndex.cstr() );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
	}

	const SphDocID_t * pDocs = NULL;
	int iDocsCount = 0;
	CSphVector<SphDocID_t> dDeleteIds;

	// now check the short path - if we have clauses 'id=smth' or 'id in (xx,yy)' or 'id in @uservar' - we know
	// all the values list immediatelly and don't have to run the heavy query here.
	const CSphQuery& tQuery = tStmt.m_tQuery; // shortcut

	if ( tQuery.m_sQuery.IsEmpty() && tQuery.m_dFilters.GetLength()==1 )
	{
		const CSphFilterSettings* pFilter = tQuery.m_dFilters.Begin();
		if ( pFilter->m_bHasEqual && pFilter->m_eType==SPH_FILTER_VALUES
			&& pFilter->m_sAttrName=="@id" && !pFilter->m_bExclude )
		{
			if_const ( sizeof(SphAttr_t)==sizeof(SphDocID_t) ) // avoid copying in this case
			{
				pDocs = (SphDocID_t*)pFilter->GetValueArray();
				iDocsCount = pFilter->GetNumValues();
			} else
			{
				const SphAttr_t* pA = pFilter->GetValueArray();
				for ( int i=0; i<pFilter->GetNumValues(); ++i )
					dDeleteIds.Add ( pA[i] );
				pDocs = dDeleteIds.Begin();
				iDocsCount = dDeleteIds.GetLength();
			}
		}
	}

	// do delete
	SearchFailuresLog_c dErrors;
	int iAffected = 0;

	// delete for local indexes
	ARRAY_FOREACH ( iIdx, dNames )
	{
		const char * sName = dNames[iIdx].cstr();
		const ServedIndex_c * pLocal = g_pLocalIndexes->GetRlockedEntry ( sName );
		if ( pLocal )
		{
			iAffected += LocalIndexDoDeleteDocuments ( sName, tStmt, pDocs, iDocsCount, pLocal, dErrors, bCommit, tAcc, iCID );
		} else
		{
			assert ( dDistributed[iIdx].m_dLocal.GetLength() || dDistributed[iIdx].m_dAgents.GetLength() );
			const CSphVector<CSphString> & dDistLocal = dDistributed[iIdx].m_dLocal;

			ARRAY_FOREACH ( i, dDistLocal )
			{
				const char * sDistLocal = dDistLocal[i].cstr();
				const ServedIndex_c * pDistLocal = g_pLocalIndexes->GetRlockedEntry ( sDistLocal );
				iAffected += LocalIndexDoDeleteDocuments ( sDistLocal, tStmt, pDocs, iDocsCount, pDistLocal, dErrors, bCommit, tAcc, iCID );
			}
		}

		// delete for remote agents
		if ( dDistributed[iIdx].m_dAgents.GetLength() )
		{
			const DistributedIndex_t & tDist = dDistributed[iIdx];
			CSphVector<AgentConn_t> dAgents;
			tDist.GetAllAgents ( &dAgents );

			// connect to remote agents and query them
			SphinxqlRequestBuilder_t tReqBuilder ( sQuery, tStmt );
			CSphScopedPtr<ISphRemoteAgentsController> pDistCtrl ( GetAgentsController ( g_iDistThreads, dAgents, tReqBuilder, tDist.m_iAgentConnectTimeout ) );
			int iAgentsDone = pDistCtrl->Finish();
			if ( iAgentsDone )
			{
				// FIXME!!! report error & warnings from agents
				int iGot = 0;
				int iWarns = 0;
				SphinxqlReplyParser_t tParser ( &iGot, &iWarns );
				RemoteWaitForAgents ( dAgents, tDist.m_iAgentQueryTimeout, tParser ); // FIXME? profile update time too?
				iAffected += iGot;
			}
		}
	}

	if ( !dErrors.IsEmpty() )
	{
		CSphStringBuilder sReport;
		dErrors.BuildReport ( sReport );
		tOut.Error ( tStmt.m_sStmt, sReport.cstr() );
		return;
	}

	tOut.Ok ( iAffected );
}


void HandleMysqlMultiStmt ( const CSphVector<SqlStmt_t> & dStmt, CSphQueryResultMeta & tLastMeta,
	SqlRowBuffer_c & dRows, ThdDesc_t * pThd, const CSphString& sWarning )
{
	// select count
	int iSelect = 0;
	ARRAY_FOREACH ( i, dStmt )
		if ( dStmt[i].m_eStmt==STMT_SELECT )
			iSelect++;

	CSphQueryResultMeta tPrevMeta = tLastMeta;

	pThd->m_sCommand = g_dSqlStmts[STMT_SELECT];
	StatCountCommand ( SEARCHD_COMMAND_SEARCH, iSelect );

	// setup query for searching
	SearchHandler_c tHandler ( iSelect, true, true, pThd->m_iConnID );
	iSelect = 0;
	ARRAY_FOREACH ( i, dStmt )
	{
		if ( dStmt[i].m_eStmt==STMT_SELECT )
			tHandler.m_dQueries[iSelect++] = dStmt[i].m_tQuery;
	}

	// use first meta for faceted search
	bool bUseFirstMeta = tHandler.m_dQueries.GetLength()>1 && !tHandler.m_dQueries[0].m_bFacet && tHandler.m_dQueries[1].m_bFacet;

	// do search
	bool bSearchOK = true;
	if ( iSelect )
	{
		bSearchOK = HandleMysqlSelect ( dRows, tHandler );

		// save meta for SHOW *
		tLastMeta = bUseFirstMeta ? tHandler.m_dResults[0] : tHandler.m_dResults.Last();
	}

	if ( !bSearchOK )
		return;

	// send multi-result set
	iSelect = 0;
	ARRAY_FOREACH ( i, dStmt )
	{
		SqlStmt_e eStmt = dStmt[i].m_eStmt;

		THD_STATE ( THD_QUERY );
		pThd->m_sCommand = g_dSqlStmts[eStmt];

		const CSphQueryResultMeta & tMeta = bUseFirstMeta ? tHandler.m_dResults[0] : ( iSelect-1>=0 ? tHandler.m_dResults[iSelect-1] : tPrevMeta );
		bool bMoreResultsFollow = (i+1)<dStmt.GetLength();

		if ( eStmt==STMT_SELECT )
		{
			AggrResult_t & tRes = tHandler.m_dResults[iSelect++];
			if ( !sWarning.IsEmpty() )
				tRes.m_sWarning = sWarning;
			SendMysqlSelectResult ( dRows, tRes, bMoreResultsFollow );
			// mysql server breaks send on error
			if ( !tRes.m_iSuccesses )
				break;
		} else if ( eStmt==STMT_SHOW_WARNINGS )
			HandleMysqlWarning ( tMeta, dRows, bMoreResultsFollow );
		else if ( eStmt==STMT_SHOW_STATUS || eStmt==STMT_SHOW_META || eStmt==STMT_SHOW_AGENT_STATUS )
			HandleMysqlMeta ( dRows, dStmt[i], tMeta, bMoreResultsFollow ); // FIXME!!! add profiler and prediction counters
		else if ( eStmt==STMT_FACET )
			dRows.Ok();

		if ( g_bGotSigterm )
		{
			sphLogDebug ( "HandleMultiStmt: got SIGTERM, sending the packet MYSQL_ERR_SERVER_SHUTDOWN" );
			dRows.Error ( NULL, "Server shutdown in progress", MYSQL_ERR_SERVER_SHUTDOWN );
			return;
		}
	}
}


struct SessionVars_t
{
	bool			m_bAutoCommit;
	bool			m_bInTransaction;
	ESphCollation	m_eCollation;
	bool			m_bProfile;

	SessionVars_t ()
		: m_bAutoCommit ( true )
		, m_bInTransaction ( false )
		, m_eCollation ( g_eCollation )
		, m_bProfile ( false )
	{}
};

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


static void UservarAdd ( const CSphString & sName, CSphVector<SphAttr_t> & dVal )
{
	Uservar_t * pVar = g_hUservars ( sName );
	if ( pVar )
	{
		// variable exists, release previous value
		// actual destruction of the value (aka data) might happen later
		// as the concurrent queries might still be using and holding that data
		// from here, the old value becomes nameless, though
		assert ( pVar->m_eType==USERVAR_INT_SET );
		assert ( pVar->m_pVal );
		pVar->m_pVal->Release();
		pVar->m_pVal = NULL;
	} else
	{
		// create a shiny new variable
		Uservar_t tVar;
		g_hUservars.Add ( tVar, sName );
		pVar = g_hUservars ( sName );
	}

	// swap in the new value
	assert ( pVar );
	assert ( !pVar->m_pVal );
	pVar->m_eType = USERVAR_INT_SET;
	pVar->m_pVal = new UservarIntSet_c();
	pVar->m_pVal->SwapData ( dVal );
}


void HandleMysqlSet ( SqlRowBuffer_c & tOut, SqlStmt_t & tStmt, SessionVars_t & tVars, CSphSessionAccum & tAcc )
{
	MEMORY ( MEM_SQL_SET );
	CSphString sError;

	tStmt.m_sSetName.ToLower();
	switch ( tStmt.m_eSet )
	{
	case SET_LOCAL:
		if ( tStmt.m_sSetName=="autocommit" )
		{
			// per-session AUTOCOMMIT
			tVars.m_bAutoCommit = ( tStmt.m_iSetValue!=0 );
			tVars.m_bInTransaction = false;

			// commit all pending changes
			if ( tVars.m_bAutoCommit )
			{
				ISphRtIndex * pIndex = tAcc.GetIndex();
				if ( pIndex )
				{
					ISphRtAccum * pAccum = tAcc.GetAcc ( pIndex, sError );
					if ( !sError.IsEmpty() )
					{
						tOut.Error ( tStmt.m_sStmt, sError.cstr() );
						return;
					} else
					{
						pIndex->Commit ( NULL, pAccum );
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
			sError.SetSprintf ( "Unknown session variable '%s' in SET statement", tStmt.m_sSetName.cstr() );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
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
			else
			{
				tOut.Error ( tStmt.m_sStmt, "Unknown log_level value (must be one of info, debug, debugv, debugvv)" );
				return;
			}
		} else if ( tStmt.m_sSetName=="query_log_min_msec" )
		{
			g_iQueryLogMinMsec = (int)tStmt.m_iSetValue;
		} else if ( tStmt.m_sSetName=="qcache_max_bytes" )
		{
			const QcacheStatus_t & s = QcacheGetStatus();
			QcacheSetup ( tStmt.m_iSetValue, s.m_iThreshMsec, s.m_iTtlSec );
		} else if ( tStmt.m_sSetName=="qcache_thresh_msec" )
		{
			const QcacheStatus_t & s = QcacheGetStatus();
			QcacheSetup ( s.m_iMaxBytes, (int)tStmt.m_iSetValue, s.m_iTtlSec );
		} else if ( tStmt.m_sSetName=="qcache_ttl_sec" )
		{
			const QcacheStatus_t & s = QcacheGetStatus();
			QcacheSetup ( s.m_iMaxBytes, s.m_iThreshMsec, (int)tStmt.m_iSetValue );
		} else
		{
			sError.SetSprintf ( "Unknown system variable '%s'", tStmt.m_sSetName.cstr() );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
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

	default:
		sError.SetSprintf ( "INTERNAL ERROR: unhandle SET mode %d", (int)tStmt.m_eSet );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	// it went ok
	tOut.Ok();
}


// fwd
void PreCreatePlainIndex ( ServedDesc_t & tServed, const char * sName );
bool PrereadNewIndex ( ServedDesc_t & tIdx, const CSphConfigSection & hIndex, const char * szIndexName );


void HandleMysqlAttach ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	const CSphString & sFrom = tStmt.m_sIndex;
	const CSphString & sTo = tStmt.m_sStringParam;
	CSphString sError;

	ServedIndex_c * pFrom = g_pLocalIndexes->GetWlockedEntry ( sFrom );
	const ServedIndex_c * pTo = g_pLocalIndexes->GetWlockedEntry ( sTo );

	if ( !pFrom || !pFrom->m_bEnabled
		|| !pTo || !pTo->m_bEnabled
		|| pFrom->m_bRT
		|| !pTo->m_bRT )
	{
		if ( !pFrom || !pFrom->m_bEnabled )
			tOut.ErrorEx ( MYSQL_ERR_PARSE_ERROR, "no such index '%s'", sFrom.cstr() );
		else if ( !pTo || !pTo->m_bEnabled )
			tOut.ErrorEx ( MYSQL_ERR_PARSE_ERROR, "no such index '%s'", sTo.cstr() );
		else if ( pFrom->m_bRT )
			tOut.Error ( tStmt.m_sStmt, "1st argument to ATTACH must be a plain index" );
		else if ( pTo->m_bRT )
			tOut.Error ( tStmt.m_sStmt, "2nd argument to ATTACH must be a RT index" );

		if ( pFrom )
			pFrom->Unlock();
		if ( pTo )
			pTo->Unlock();
		return;
	}

	ISphRtIndex * pRtTo = (ISphRtIndex*)pTo->m_pIndex;
	bool bOk = pRtTo->AttachDiskIndex ( pFrom->m_pIndex, sError );
	if ( !bOk )
	{
		pFrom->Unlock();
		pTo->Unlock();
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	pTo->Unlock();

	// after a successfull Attach() RT index owns it
	// so we need to create dummy disk index until further notice
	pFrom->m_pIndex = NULL;
	pFrom->m_bEnabled = false;
	PreCreatePlainIndex ( *pFrom, sFrom.cstr() );
	if ( pFrom->m_pIndex )
		pFrom->m_bEnabled = PrereadNewIndex ( *pFrom, g_pCfg.m_tConf["index"][sFrom], sFrom.cstr() );
	pFrom->Unlock();

	tOut.Ok();
}


void HandleMysqlFlushRtindex ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	const ServedIndex_c * pIndex = g_pLocalIndexes->GetRlockedEntry ( tStmt.m_sIndex );

	if ( !pIndex || !pIndex->m_bEnabled || !pIndex->m_bRT )
	{
		if ( pIndex )
		pIndex->Unlock();
		tOut.Error ( tStmt.m_sStmt, "FLUSH RTINDEX requires an existing RT index" );
		return;
	}

	ISphRtIndex * pRt = (ISphRtIndex*)pIndex->m_pIndex;

	pRt->ForceRamFlush();
	pIndex->Unlock();
	tOut.Ok();
}


void HandleMysqlFlushRamchunk ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	const ServedIndex_c * pIndex = g_pLocalIndexes->GetRlockedEntry ( tStmt.m_sIndex );

	if ( !pIndex || !pIndex->m_bEnabled || !pIndex->m_bRT )
	{
		if ( pIndex )
			pIndex->Unlock();
		tOut.Error ( tStmt.m_sStmt, "FLUSH RAMCHUNK requires an existing RT index" );
		return;
	}

	ISphRtIndex * pRt = (ISphRtIndex*)pIndex->m_pIndex;

	pRt->ForceDiskChunk();
	pIndex->Unlock();
	tOut.Ok();
}


void HandleMysqlFlush ( SqlRowBuffer_c & tOut, const SqlStmt_t & )
{
	int iTag = CommandFlush();
	tOut.HeadBegin(1);
	tOut.HeadColumn ( "tag", MYSQL_COL_LONG );
	tOut.HeadEnd();

	// data packet, var value
	tOut.PutNumeric ( "%d", iTag );
	tOut.Commit();

	// done
	tOut.Eof();
}


void HandleMysqlTruncate ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	// get an exclusive lock
	const ServedIndex_c * pIndex = g_pLocalIndexes->GetWlockedEntry ( tStmt.m_sIndex );

	if ( !pIndex || !pIndex->m_bEnabled || !pIndex->m_bRT )
	{
		if ( pIndex )
			pIndex->Unlock();
		tOut.Error ( tStmt.m_sStmt, "TRUNCATE RTINDEX requires an existing RT index" );
		return;
	}

	ISphRtIndex * pRt = (ISphRtIndex*)pIndex->m_pIndex;

	CSphString sError;
	bool bRes = pRt->Truncate ( sError );
	pIndex->Unlock();

	if ( !bRes )
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
	else
		tOut.Ok();
}


void HandleMysqlOptimize ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	// get an exclusive lock
	const ServedIndex_c * pIndex = g_pLocalIndexes->GetRlockedEntry ( tStmt.m_sIndex );
	bool bValid = pIndex && pIndex->m_bEnabled && pIndex->m_bRT;
	if ( pIndex )
		pIndex->Unlock();

	if ( !bValid )
		tOut.Error ( tStmt.m_sStmt, "OPTIMIZE INDEX requires an existing RT index" );
	else
		tOut.Ok();

	if ( bValid )
	{
		g_tOptimizeQueueMutex.Lock();
		g_dOptimizeQueue.Add ( tStmt.m_sIndex );
		g_tOptimizeQueueMutex.Unlock();
	}
}

void HandleMysqlSelectSysvar ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	CSphString sVar = tStmt.m_tQuery.m_sQuery;
	sVar.ToLower();

	if ( sVar=="@@session.auto_increment_increment" )
	{
		// MySQL Connector/J really expects an answer here
		tOut.HeadBegin(1);
		tOut.HeadColumn ( sVar.cstr(), MYSQL_COL_LONG );
		tOut.HeadEnd();

		// data packet, var value
		tOut.PutString("1");
		tOut.Commit();

		// done
		tOut.Eof();
	} else if ( sVar=="@@max_allowed_packet" )
	{
		// MySQL Go connector really expects an answer here
		tOut.HeadBegin(1);
		tOut.HeadColumn ( sVar.cstr(), MYSQL_COL_LONG );
		tOut.HeadEnd();

		// data packet, var value
		tOut.PutNumeric ( "%d", g_iMaxPacketSize );
		tOut.Commit();

		// done
		tOut.Eof();
	} else
	{
		// generally, just send empty response
		tOut.Ok();
	}
}


void HandleMysqlSelectDual ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	CSphString sVar = tStmt.m_tQuery.m_sQuery;
	CSphSchema	tSchema;
	ESphAttr eAttrType;
	CSphString sError;

	ISphExpr * pExpr = sphExprParse ( sVar.cstr(), tSchema, &eAttrType, NULL, sError, NULL );

	if ( !pExpr )
	{
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	tOut.HeadBegin(1);
	tOut.HeadColumn ( sVar.cstr(), MYSQL_COL_STRING );
	tOut.HeadEnd();

	CSphMatch tMatch;
	const BYTE * pStr = NULL;

	switch ( eAttrType )
	{
		case SPH_ATTR_STRINGPTR:
			pExpr->StringEval ( tMatch, &pStr );
			tOut.PutString ( (const char*)pStr );
			SafeDelete ( pStr );
			break;
		case SPH_ATTR_INTEGER:	tOut.PutNumeric<int> ( "%d", pExpr->IntEval ( tMatch ) ); break;
		case SPH_ATTR_BIGINT:	tOut.PutNumeric<SphAttr_t> ( INT64_FMT, pExpr->Int64Eval ( tMatch ) ); break;
		case SPH_ATTR_FLOAT:	tOut.PutNumeric<float> ( "%f", pExpr->Eval ( tMatch ) ); break;
		default:
			tOut.PutNULL();
			break;
	}

	SafeDelete ( pExpr );

	// done
	tOut.Commit();
	tOut.Eof();
}


void HandleMysqlShowCollations ( SqlRowBuffer_c & tOut )
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

void HandleMysqlShowCharacterSet ( SqlRowBuffer_c & tOut )
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


void HandleMysqlShowVariables ( SqlRowBuffer_c & dRows, const SqlStmt_t & tStmt, SessionVars_t & tVars )
{
	VectorLike dStatus ( tStmt.m_sStringParam );

	if ( dStatus.MatchAdd ( "autocommit" ) )
		dStatus.Add ( tVars.m_bAutoCommit ? "1" : "0" );

	if ( dStatus.MatchAdd ( "collation_connection" ) )
		dStatus.Add ( sphCollationToName ( tVars.m_eCollation ) );

	if ( dStatus.MatchAdd ( "query_log_format" ) )
		dStatus.Add ( g_eLogFormat==LOG_FORMAT_PLAIN ? "plain" : "sphinxql" );

	if ( dStatus.MatchAdd ( "log_level" ) )
		dStatus.Add ( LogLevelName ( g_eLogLevel ) );

	if ( dStatus.MatchAdd ( "max_allowed_packet" ) )
		dStatus.Add().SetSprintf ( "%d", g_iMaxPacketSize );

	// .NET connector requires character_set_* vars
	if ( dStatus.MatchAdd ( "character_set_client" ) )
		dStatus.Add ( "utf8" );

	if ( dStatus.MatchAdd ( "character_set_connection" ) )
		dStatus.Add ( "utf8" );

	// result set header packet
	dRows.HeadTuplet ( "Variable_name", "Value" );

	// send rows
	for ( int iRow=0; iRow<dStatus.GetLength(); iRow+=2 )
		dRows.DataTuplet ( dStatus[iRow+0].cstr(), dStatus[iRow+1].cstr() );

	// cleanup
	dRows.Eof();
}


void HandleMysqlShowIndexStatus ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	const ServedIndex_c * pServed = g_pLocalIndexes->GetRlockedEntry ( tStmt.m_sIndex );

	if ( !pServed || !pServed->m_bEnabled )
	{
		if ( pServed )
			pServed->Unlock();
		tOut.Error ( tStmt.m_sStmt, "SHOW INDEX STATUS requires an existing index" );
		return;
	}

	CSphIndex * pIndex = pServed->m_pIndex;

	tOut.HeadTuplet ( "Variable_name", "Value" );

	tOut.DataTuplet ( "index_type", pServed->m_bRT ? "rt" : "disk" );
	tOut.DataTuplet ( "indexed_documents", pIndex->GetStats().m_iTotalDocuments );
	tOut.DataTuplet ( "indexed_bytes", pIndex->GetStats().m_iTotalBytes );

	const int64_t * pFieldLens = pIndex->GetFieldLens();
	if ( pFieldLens )
	{
		int64_t iTotalTokens = 0;
		const CSphVector<CSphColumnInfo> & dFields = pIndex->GetMatchSchema().m_dFields;
		ARRAY_FOREACH ( i, dFields )
		{
			CSphString sKey;
			sKey.SetSprintf ( "field_tokens_%s", dFields[i].m_sName.cstr() );
			tOut.DataTuplet ( sKey.cstr(), pFieldLens[i] );
			iTotalTokens += pFieldLens[i];
		}
		tOut.DataTuplet ( "total_tokens", iTotalTokens );
	}

	CSphIndexStatus tStatus;
	pIndex->GetStatus ( &tStatus );
	tOut.DataTuplet ( "ram_bytes", tStatus.m_iRamUse );
	tOut.DataTuplet ( "disk_bytes", tStatus.m_iDiskUse );
	if ( pIndex->IsRT() )
	{
		tOut.DataTuplet ( "ram_chunk", tStatus.m_iRamChunkSize );
		tOut.DataTuplet ( "disk_chunks", tStatus.m_iNumChunks );
		tOut.DataTuplet ( "mem_limit", tStatus.m_iMemLimit );
	}

	pServed->Unlock();
	tOut.Eof();
}


void DumpKey ( CSphStringBuilder & tBuf, const char * sKey, const char * sVal, bool bCond )
{
	if ( bCond )
		tBuf.Appendf ( "%s = %s\n", sKey, sVal );
}


void DumpKey ( CSphStringBuilder & tBuf, const char * sKey, int iVal, bool bCond )
{
	if ( bCond )
		tBuf.Appendf ( "%s = %d\n", sKey, iVal );
}


void DumpIndexSettings ( CSphStringBuilder & tBuf, CSphIndex * pIndex )
{
	const CSphIndexSettings & tSettings = pIndex->GetSettings();
	DumpKey ( tBuf, "docinfo",				"inline",								tSettings.m_eDocinfo==SPH_DOCINFO_INLINE );
	DumpKey ( tBuf, "min_prefix_len",		tSettings.m_iMinPrefixLen,				tSettings.m_iMinPrefixLen!=0 );
	DumpKey ( tBuf, "max_substring_len",	tSettings.m_iMaxSubstringLen,			tSettings.m_iMaxSubstringLen!=0 );
	DumpKey ( tBuf, "index_exact_words",	1,										tSettings.m_bIndexExactWords );
	DumpKey ( tBuf, "html_strip",			1,										tSettings.m_bHtmlStrip );
	DumpKey ( tBuf, "html_index_attrs",		tSettings.m_sHtmlIndexAttrs.cstr(),		!tSettings.m_sHtmlIndexAttrs.IsEmpty() );
	DumpKey ( tBuf, "html_remove_elements", tSettings.m_sHtmlRemoveElements.cstr(), !tSettings.m_sHtmlRemoveElements.IsEmpty() );
	DumpKey ( tBuf, "index_zones",			tSettings.m_sZones.cstr(),				!tSettings.m_sZones.IsEmpty() );
	DumpKey ( tBuf, "index_field_lengths",	1,										tSettings.m_bIndexFieldLens );

	if ( pIndex->GetTokenizer() )
	{
		const CSphTokenizerSettings & tTokSettings = pIndex->GetTokenizer()->GetSettings();
		DumpKey ( tBuf, "charset_type",		( tTokSettings.m_iType==TOKENIZER_UTF8 || tTokSettings.m_iType==TOKENIZER_NGRAM )
											? "utf-8"
											: "unknown tokenizer (deprecated sbcs?)", true );
		DumpKey ( tBuf, "charset_table",	tTokSettings.m_sCaseFolding.cstr(),		!tTokSettings.m_sCaseFolding.IsEmpty() );
		DumpKey ( tBuf, "min_word_len",		tTokSettings.m_iMinWordLen,				tTokSettings.m_iMinWordLen>1 );
		DumpKey ( tBuf, "ngram_len",		tTokSettings.m_iNgramLen,				tTokSettings.m_iNgramLen && !tTokSettings.m_sNgramChars.IsEmpty() );
		DumpKey ( tBuf, "ngram_chars",		tTokSettings.m_sNgramChars.cstr(),		tTokSettings.m_iNgramLen && !tTokSettings.m_sNgramChars.IsEmpty() );
		DumpKey ( tBuf, "exceptions",		tTokSettings.m_sSynonymsFile.cstr(),	!tTokSettings.m_sSynonymsFile.IsEmpty() );
		DumpKey ( tBuf, "phrase_boundary",	tTokSettings.m_sBoundary.cstr(),		!tTokSettings.m_sBoundary.IsEmpty() );
		DumpKey ( tBuf, "ignore_chars",		tTokSettings.m_sIgnoreChars.cstr(),		!tTokSettings.m_sIgnoreChars.IsEmpty() );
		DumpKey ( tBuf, "blend_chars",		tTokSettings.m_sBlendChars.cstr(),		!tTokSettings.m_sBlendChars.IsEmpty() );
		DumpKey ( tBuf, "blend_mode",		tTokSettings.m_sBlendMode.cstr(),		!tTokSettings.m_sBlendMode.IsEmpty() );
	}

	if ( pIndex->GetDictionary() )
	{
		const CSphDictSettings & tDictSettings = pIndex->GetDictionary()->GetSettings();
		CSphStringBuilder tWordforms;
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
			tWordforms.Appendf ( " %s", tDictSettings.m_dWordforms[i].cstr () );
		DumpKey ( tBuf, "dict",				"keywords",								tDictSettings.m_bWordDict );
		DumpKey ( tBuf, "morphology",		tDictSettings.m_sMorphology.cstr(),		!tDictSettings.m_sMorphology.IsEmpty() );
		DumpKey ( tBuf, "stopwords",		tDictSettings.m_sStopwords.cstr (),		!tDictSettings.m_sStopwords.IsEmpty() );
		DumpKey ( tBuf, "wordforms",		tWordforms.cstr()+1,					tDictSettings.m_dWordforms.GetLength()>0 );
		DumpKey ( tBuf, "min_stemming_len",	tDictSettings.m_iMinStemmingLen,		tDictSettings.m_iMinStemmingLen>1 );
	}
}


void HandleMysqlShowIndexSettings ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	const ServedIndex_c * pServed = g_pLocalIndexes->GetRlockedEntry ( tStmt.m_sIndex );

	int iChunk = tStmt.m_iIntParam;
	CSphIndex * pIndex = pServed ? pServed->m_pIndex : NULL;

	if ( iChunk>=0 && pServed && pIndex && pIndex->IsRT() )
		pIndex = static_cast<ISphRtIndex *>( pIndex )->GetDiskChunk ( iChunk );

	if ( !pServed || !pServed->m_bEnabled || !pIndex )
	{
		if ( pServed )
			pServed->Unlock();
		tOut.Error ( tStmt.m_sStmt, "SHOW INDEX SETTINGS requires an existing index" );
		return;
	}

	tOut.HeadTuplet ( "Variable_name", "Value" );

	CSphStringBuilder sBuf;
	DumpIndexSettings ( sBuf, pIndex );
	tOut.DataTuplet ( "settings", sBuf.cstr() );

	pServed->Unlock();
	tOut.Eof();
}


void HandleMysqlShowProfile ( SqlRowBuffer_c & tOut, const CSphQueryProfile & p )
{
	#define SPH_QUERY_STATE(_name,_desc) _desc,
	static const char * dStates [ SPH_QSTATE_TOTAL ] = { SPH_QUERY_STATES };
	#undef SPH_QUERY_STATES

	tOut.HeadBegin ( 4 );
	tOut.HeadColumn ( "Status" );
	tOut.HeadColumn ( "Duration" );
	tOut.HeadColumn ( "Switches" );
	tOut.HeadColumn ( "Percent" );
	tOut.HeadEnd();

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
		tOut.PutNumeric ( "%d", p.m_dSwitches[i] );
		tOut.PutNumeric ( "%.2f", 100.0f * p.m_tmTotal[i]/tmTotal );
		tOut.Commit();
	}
	snprintf ( sTime, sizeof(sTime), "%d.%06d", int(tmTotal/1000000), int(tmTotal%1000000) );
	tOut.PutString ( "total" );
	tOut.PutString ( sTime );
	tOut.PutNumeric ( "%d", iCount );
	tOut.PutString ( "0" );
	tOut.Commit();
	tOut.Eof();
}


static void AddAttrToIndex ( const SqlStmt_t & tStmt, const ServedIndex_c * pServed, CSphString & sError )
{
	CSphString sAttrToAdd = tStmt.m_sAlterAttr;
	sAttrToAdd.ToLower();

	if ( pServed->m_pIndex->GetMatchSchema().GetAttr ( sAttrToAdd.cstr() ) )
	{
		sError.SetSprintf ( "'%s' attribute already in schema", sAttrToAdd.cstr() );
		return;
	}

	pServed->m_pIndex->AddRemoveAttribute ( true, sAttrToAdd, tStmt.m_eAlterColType, sError );
}


static void RemoveAttrFromIndex ( const SqlStmt_t & tStmt, const ServedIndex_c * pServed, CSphString & sError )
{
	CSphString sAttrToRemove = tStmt.m_sAlterAttr;
	sAttrToRemove.ToLower();

	if ( !pServed->m_pIndex->GetMatchSchema().GetAttr ( sAttrToRemove.cstr() ) )
	{
		sError.SetSprintf ( "attribute '%s' does not exist", sAttrToRemove.cstr() );
		return;
	}

	if ( pServed->m_pIndex->GetMatchSchema().GetAttrsCount()==1 )
	{
		sError.SetSprintf ( "unable to remove last attribute '%s'", sAttrToRemove.cstr() );
		return;
	}

	pServed->m_pIndex->AddRemoveAttribute ( false, sAttrToRemove, SPH_ATTR_NONE, sError );
}


static void HandleMysqlAlter ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt, bool bAdd )
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

	CSphVector<CSphString> dNames;
	ParseIndexList ( tStmt.m_sIndex, dNames );
	if ( !dNames.GetLength() )
	{
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	bool bHaveDist = false;
	ARRAY_FOREACH_COND ( i, dNames, !bHaveDist )
		if ( !g_pLocalIndexes->Exists ( dNames[i] ) )
		{
			g_tDistLock.Lock();
			bHaveDist = !!g_hDistIndexes ( dNames[i] );
			g_tDistLock.Unlock();
		}

	if ( bHaveDist )
	{
		sError.SetSprintf ( "ALTER is only supported on local (not distributed) indexes" );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	ARRAY_FOREACH ( iIdx, dNames )
	{
		const char * sName = dNames[iIdx].cstr();
		const ServedIndex_c * pLocal = g_pLocalIndexes->GetWlockedEntry ( sName );
		if ( pLocal )
		{
			if ( !pLocal->m_bEnabled )
			{
				dErrors.Submit ( sName, "does not support ALTER (enabled=false)" );
				pLocal->Unlock();
				continue;
			}

			if ( pLocal->m_pIndex->GetSettings().m_eDocinfo==SPH_DOCINFO_INLINE )
			{
				dErrors.Submit ( sName, "docinfo is inline: ALTER disabled" );
				pLocal->Unlock();
				continue;
			}

			CSphString sAddError;

			if ( bAdd )
				AddAttrToIndex ( tStmt, pLocal, sAddError );
			else
				RemoveAttrFromIndex ( tStmt, pLocal, sAddError );

			if ( sAddError.Length() )
				dErrors.Submit ( sName, sAddError.cstr() );

			pLocal->Unlock();
		}
	}

	if ( !dErrors.IsEmpty() )
	{
		CSphStringBuilder sReport;
		dErrors.BuildReport ( sReport );
		tOut.Error ( tStmt.m_sStmt, sReport.cstr() );
		return;
	} else
		tOut.Ok();
}


static void HandleMysqlReconfigure ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	MEMORY ( MEM_SQL_ALTER );

	const char * sName = tStmt.m_sIndex.cstr();
	CSphString sError;
	CSphReconfigureSettings tSettings;
	CSphReconfigureSetup tSetup;

	CSphConfigParser tCfg;
	if ( !tCfg.ReParse ( g_sConfigFile.cstr () ) )
	{
		sError.SetSprintf ( "failed to parse config file '%s'; using previous settings", g_sConfigFile.cstr () );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	const CSphConfig & hConf = tCfg.m_tConf;
	if ( !hConf["index"].Exists ( sName ) )
	{
		sError.SetSprintf ( "failed to find index '%s' at config file '%s'; using previous settings", sName, g_sConfigFile.cstr () );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	const CSphConfigSection & hIndex = hConf["index"][sName];
	sphConfTokenizer ( hIndex, tSettings.m_tTokenizer );

	sphConfDictionary ( hIndex, tSettings.m_tDict );

	if ( !sphConfIndex ( hIndex, tSettings.m_tIndex, sError ) )
	{
		sError.SetSprintf ( "'%s' failed to parse index settings, error '%s'", sName, sError.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	const ServedIndex_c * pServed = g_pLocalIndexes->GetRlockedEntry ( tStmt.m_sIndex.cstr() );
	if ( !pServed || !pServed->m_bEnabled || !pServed->m_pIndex->IsRT() )
	{
		if ( pServed )
			sError.SetSprintf ( "'%s' does not support ALTER (enabled=%d, RT=%d)", sName, pServed->m_bEnabled, pServed->m_pIndex->IsRT() );
		else
			sError.SetSprintf ( "ALTER is only supported on local (not distributed) indexes" );

		tOut.Error ( tStmt.m_sStmt, sError.cstr() );

		if ( pServed )
			pServed->Unlock();

		return;
	}

	bool bSame = ( (const ISphRtIndex *)pServed->m_pIndex )->IsSameSettings ( tSettings, tSetup, sError );

	pServed->Unlock();

	// replace going with exclusive lock
	if ( !bSame && sError.IsEmpty() )
	{
		const ServedIndex_c * pServed = g_pLocalIndexes->GetWlockedEntry ( tStmt.m_sIndex.cstr() );
		( (ISphRtIndex *)pServed->m_pIndex )->Reconfigure ( tSetup );
		pServed->Unlock();
	}

	if ( sError.IsEmpty() )
		tOut.Ok();
	else
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
}


static void HandleMysqlShowPlan ( SqlRowBuffer_c & tOut, const CSphQueryProfile & p )
{
	tOut.HeadBegin ( 2 );
	tOut.HeadColumn ( "Variable" );
	tOut.HeadColumn ( "Value" );
	tOut.HeadEnd();

	tOut.PutString ( "transformed_tree" );
	tOut.PutString ( p.m_sTransformedTree.cstr() );
	tOut.Commit();

	tOut.Eof();
}

static bool RotateIndexMT ( const CSphString & sIndex, CSphString & sError );
static bool RotateIndexGreedy ( ServedDesc_t & tIndex, const char * sIndex, CSphString & sError );
static void HandleMysqlReloadIndex ( SqlRowBuffer_c & tOut, const SqlStmt_t & tStmt )
{
	CSphString sError;
	const ServedIndex_c * pIndex = g_pLocalIndexes->GetRlockedEntry ( tStmt.m_sIndex );
	if ( !pIndex )
	{
		sError.SetSprintf ( "unknown plain index '%s'", tStmt.m_sIndex.cstr() );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}

	const char * sIndexPath = pIndex->m_sIndexPath.cstr();

	if ( pIndex->m_bRT )
	{
		pIndex->Unlock();
		sError.SetSprintf ( "can not rotate RT index" );
		tOut.Error ( tStmt.m_sStmt, sError.cstr() );
		return;
	}
	pIndex->Unlock();

	// try move files from arbitrary path to current index path before rotate, if needed
	if ( !tStmt.m_sStringParam.IsEmpty() )
	{
		for ( int i=0; i<sphGetExtCount ( INDEX_FORMAT_VERSION ); ++i )
		{
			char sFrom [ SPH_MAX_FILENAME_LEN ];
			char sTo [ SPH_MAX_FILENAME_LEN ];
			snprintf ( sFrom, sizeof(sFrom), "%s%s", tStmt.m_sStringParam.cstr(), sphGetExts ( SPH_EXT_TYPE_CUR, INDEX_FORMAT_VERSION )[i] );
			snprintf ( sTo, sizeof(sTo), "%s%s", sIndexPath, sphGetExts ( SPH_EXT_TYPE_NEW, INDEX_FORMAT_VERSION )[i] );
			if ( rename ( sFrom, sTo ) )
			{
				sError.SetSprintf ( "moving file from '%s' to '%s' failed: %s", sFrom, sTo, strerror(errno) );
				tOut.Error ( tStmt.m_sStmt, sError.cstr() );
				return;
			}
		}
	}

	if ( g_bSeamlessRotate )
	{
		if ( !RotateIndexMT ( tStmt.m_sIndex, sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
	} else
	{
		ServedIndex_c * pIndex = g_pLocalIndexes->GetWlockedEntry ( tStmt.m_sIndex );
		if ( !RotateIndexGreedy ( *pIndex, tStmt.m_sIndex.cstr(), sError ) )
		{
			pIndex->Unlock();
			sphWarning ( "%s", sError.cstr() );
			tOut.Error ( tStmt.m_sStmt, sError.cstr() );
			return;
		}
		pIndex->Unlock();
	}

	tOut.Ok();
}

//////////////////////////////////////////////////////////////////////////

CSphSessionAccum::CSphSessionAccum ( bool bManage )
	: m_pAcc ( NULL )
	, m_bManage ( bManage )
{ }

CSphSessionAccum::~CSphSessionAccum()
{
	SafeDelete ( m_pAcc );
}

ISphRtAccum * CSphSessionAccum::GetAcc ( ISphRtIndex * pIndex, CSphString & sError )
{
	if ( !m_bManage )
		return NULL;

	assert ( pIndex );
	if ( m_pAcc )
		return m_pAcc;

	m_pAcc = pIndex->CreateAccum ( sError );
	return m_pAcc;
}

ISphRtIndex * CSphSessionAccum::GetIndex ()
{
	if ( !m_bManage )
		return sphGetCurrentIndexRT();

	return ( m_pAcc ? m_pAcc->GetIndex() : NULL );
}


class CSphinxqlSession : public ISphNoncopyable
{
private:
	CSphString			m_sError;
	CSphQueryResultMeta m_tLastMeta;
	CSphSessionAccum	m_tAcc;

public:
	SessionVars_t		m_tVars;
	CSphQueryProfile	m_tProfile;
	CSphQueryProfile	m_tLastProfile;

public:
	explicit CSphinxqlSession ( bool bManage )
		: m_tAcc ( bManage )
	{}

	// just execute one sphinxql statement
	//
	// IMPORTANT! this does NOT start or stop profiling, as there a few external
	// things (client net reads and writes) that we want to profile, too
	//
	// returns true if the current profile should be kept (default)
	// returns false if profile should be discarded (eg. SHOW PROFILE case)
	bool Execute ( const CSphString & sQuery, ISphOutputBuffer & tOutput, BYTE & uPacketID, ThdDesc_t * pThd )
	{
		// set on query guard
		CrashQuery_t tCrashQuery;
		tCrashQuery.m_pQuery = (const BYTE *)sQuery.cstr();
		tCrashQuery.m_iSize = sQuery.Length();
		tCrashQuery.m_bMySQL = true;
		SphCrashLogger_c::SetLastQuery ( tCrashQuery );

		// parse SQL query
		if ( m_tVars.m_bProfile )
			m_tProfile.Switch ( SPH_QSTATE_SQL_PARSE );

		CSphVector<SqlStmt_t> dStmt;
		bool bParsedOK = sphParseSqlQuery ( sQuery.cstr(), tCrashQuery.m_iSize, dStmt, m_sError, m_tVars.m_eCollation );

		if ( m_tVars.m_bProfile )
			m_tProfile.Switch ( SPH_QSTATE_UNKNOWN );

		SqlStmt_e eStmt = STMT_PARSE_ERROR;
		if ( bParsedOK )
		{
			eStmt = dStmt[0].m_eStmt;
			dStmt[0].m_sStmt = sQuery.cstr();
		}

		SqlStmt_t * pStmt = dStmt.Begin();
		assert ( !bParsedOK || pStmt );

		pThd->m_sCommand = g_dSqlStmts[eStmt];
		THD_STATE ( THD_QUERY );

		SqlRowBuffer_c tOut ( &uPacketID, &tOutput, pThd->m_iConnID );

		// handle multi SQL query
		if ( bParsedOK && dStmt.GetLength()>1 )
		{
			m_sError = "";
			HandleMysqlMultiStmt ( dStmt, m_tLastMeta, tOut, pThd, m_sError );
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
				SearchHandler_c tHandler ( 1, true, true, pThd->m_iConnID );
				tHandler.m_dQueries[0] = dStmt.Begin()->m_tQuery;
				if ( m_tVars.m_bProfile )
					tHandler.m_pProfile = &m_tProfile;

				if ( HandleMysqlSelect ( tOut, tHandler ) )
				{
					// query just completed ok; reset out error message
					m_sError = "";
					AggrResult_t & tLast = tHandler.m_dResults.Last();
					SendMysqlSelectResult ( tOut, tLast, false );
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
			HandleMysqlMeta ( tOut, *pStmt, m_tLastMeta, false );
			return true;

		case STMT_INSERT:
		case STMT_REPLACE:
			m_tLastMeta = CSphQueryResultMeta();
			m_tLastMeta.m_sError = m_sError;
			m_tLastMeta.m_sWarning = "";
			HandleMysqlInsert ( tOut, *pStmt, eStmt==STMT_REPLACE,
				m_tVars.m_bAutoCommit && !m_tVars.m_bInTransaction, m_tLastMeta.m_sWarning, m_tAcc );
			return true;

		case STMT_DELETE:
			m_tLastMeta = CSphQueryResultMeta();
			m_tLastMeta.m_sError = m_sError;
			m_tLastMeta.m_sWarning = "";
			StatCountCommand ( SEARCHD_COMMAND_DELETE );
			HandleMysqlDelete ( tOut, *pStmt, sQuery, m_tVars.m_bAutoCommit && !m_tVars.m_bInTransaction, m_tAcc, pThd->m_iConnID );
			return true;

		case STMT_SET:
			HandleMysqlSet ( tOut, *pStmt, m_tVars, m_tAcc );
			return false;

		case STMT_BEGIN:
			{
				MEMORY ( MEM_SQL_BEGIN );

				m_tVars.m_bInTransaction = true;
				ISphRtIndex * pIndex = m_tAcc.GetIndex();
				if ( pIndex )
				{
					ISphRtAccum * pAccum = m_tAcc.GetAcc ( pIndex, m_sError );
					if ( !m_sError.IsEmpty() )
					{
						tOut.Error ( sQuery.cstr(), m_sError.cstr() );
						return true;
					}
					pIndex->Commit ( NULL, pAccum );
				}
				tOut.Ok();
				return true;
			}
		case STMT_COMMIT:
		case STMT_ROLLBACK:
			{
				MEMORY ( MEM_SQL_COMMIT );

				m_tVars.m_bInTransaction = false;
				ISphRtIndex * pIndex = m_tAcc.GetIndex();
				if ( pIndex )
				{
					ISphRtAccum * pAccum = m_tAcc.GetAcc ( pIndex, m_sError );
					if ( !m_sError.IsEmpty() )
					{
						tOut.Error ( sQuery.cstr(), m_sError.cstr() );
						return true;
					}
					if ( eStmt==STMT_COMMIT )
						pIndex->Commit ( NULL, pAccum );
					else
						pIndex->RollBack ( pAccum );
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
				HandleMysqlCallSnippets ( tOut, *pStmt, pThd );
			} else if ( pStmt->m_sCallProc=="KEYWORDS" )
			{
				StatCountCommand ( SEARCHD_COMMAND_KEYWORDS );
				HandleMysqlCallKeywords ( tOut, *pStmt );
			} else
			{
				m_sError.SetSprintf ( "no such builtin procedure %s", pStmt->m_sCallProc.cstr() );
				tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			}
			return true;

		case STMT_DESCRIBE:
			HandleMysqlDescribe ( tOut, *pStmt );
			return true;

		case STMT_SHOW_TABLES:
			HandleMysqlShowTables ( tOut, *pStmt );
			return true;

		case STMT_UPDATE:
			m_tLastMeta = CSphQueryResultMeta();
			m_tLastMeta.m_sError = m_sError;
			m_tLastMeta.m_sWarning = "";
			StatCountCommand ( SEARCHD_COMMAND_UPDATE );
			HandleMysqlUpdate ( tOut, *pStmt, sQuery, m_tLastMeta.m_sWarning, pThd->m_iConnID );
			return true;

		case STMT_DUMMY:
			tOut.Ok();
			return true;

		case STMT_CREATE_FUNCTION:
			if ( !sphPluginCreate ( pStmt->m_sUdfLib.cstr(), PLUGIN_FUNCTION, pStmt->m_sUdfName.cstr(), pStmt->m_eUdfType, m_sError ) )
				tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			else
				tOut.Ok();
			g_tmSphinxqlState = sphMicroTimer();
			return true;

		case STMT_DROP_FUNCTION:
			if ( !sphPluginDrop ( PLUGIN_FUNCTION, pStmt->m_sUdfName.cstr(), m_sError ) )
				tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			else
				tOut.Ok();
			g_tmSphinxqlState = sphMicroTimer();
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
				g_tmSphinxqlState = sphMicroTimer();
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
			HandleMysqlSelectSysvar ( tOut, *pStmt );
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

		case STMT_SHOW_INDEX_SETTINGS:
			HandleMysqlShowIndexSettings ( tOut, *pStmt );
			return true;

		case STMT_SHOW_PROFILE:
			HandleMysqlShowProfile ( tOut, m_tLastProfile );
			return false; // do not profile this call, keep last query profile

		case STMT_ALTER_ADD:
			HandleMysqlAlter ( tOut, *pStmt, true );
			return true;

		case STMT_ALTER_DROP:
			HandleMysqlAlter ( tOut, *pStmt, false );
			return true;

		case STMT_SHOW_PLAN:
			HandleMysqlShowPlan ( tOut, m_tLastProfile );
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
			HandleMysqlShowThreads ( tOut, *pStmt );
			return true;

		case STMT_ALTER_RECONFIGURE:
			HandleMysqlReconfigure ( tOut, *pStmt );
			return true;

		case STMT_FLUSH_INDEX:
			HandleMysqlFlush ( tOut, *pStmt );
			return true;

		case STMT_RELOAD_INDEX:
			HandleMysqlReloadIndex ( tOut, *pStmt );
			return true;

		default:
			m_sError.SetSprintf ( "internal error: unhandled statement type (value=%d)", eStmt );
			tOut.Error ( sQuery.cstr(), m_sError.cstr() );
			return true;
		} // switch
		return true; // for cases that break early
	}
};


/// sphinxql command over API
void HandleCommandSphinxql ( ISphOutputBuffer & tOut, int iVer, InputBuffer_c & tReq, ThdDesc_t * pThd )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_SPHINXQL, tOut ) )
		return;

	// parse request
	CSphString sCommand = tReq.GetString ();

	BYTE uDummy = 0;

	// todo: move upper, if the session variables are also necessary in API access mode.
	CSphinxqlSession tSession ( true ); // FIXME!!! check that no accum related command used via API

	tOut.Flush();
	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_SPHINXQL );

	ISphOutputBuffer tMemOut;
	tSession.Execute ( sCommand, tMemOut, uDummy, pThd );

	tOut.SendOutput ( tMemOut );
	tOut.Flush();
}


void StatCountCommand ( int iCmd, int iCount )
{
	if ( iCmd>=0 && iCmd<SEARCHD_COMMAND_TOTAL )
	{
		g_tStatsMutex.Lock();
		g_tStats.m_iCommandCount[iCmd] += iCount;
		g_tStatsMutex.Unlock();
	}
}

static bool LoopClientMySQL ( BYTE & uPacketID, CSphinxqlSession & tSession, CSphString & sQuery, int iPacketLen, bool bProfile, ThdDesc_t * pThd, InputBuffer_c & tIn, ISphOutputBuffer & tOut );

static void HandleClientMySQL ( int iSock, const char * sClientIP, ThdDesc_t * pThd )
{
	MEMORY ( MEM_SQL_HANDLE );
	THD_STATE ( THD_HANDSHAKE );

	// set off query guard
	CrashQuery_t tCrashQuery;
	tCrashQuery.m_bMySQL = true;
	SphCrashLogger_c::SetLastQuery ( tCrashQuery );

	int iCID = pThd->m_iConnID;

	if ( sphSockSend ( iSock, g_sMysqlHandshake, g_iMysqlHandshake )!=g_iMysqlHandshake )
	{
		int iErrno = sphSockGetErrno ();
		sphWarning ( "failed to send server version (client=%s(%d), error: %d '%s')", sClientIP, iCID, iErrno, sphSockError ( iErrno ) );
		return;
	}

	CSphString sQuery; // to keep data alive for SphCrashQuery_c
	CSphinxqlSession tSession ( false ); // session variables and state
	bool bAuthed = false;
	BYTE uPacketID = 1;

	for ( ;; )
	{
		NetOutputBuffer_c tOut ( iSock ); // OPTIMIZE? looks like buffer size matters a lot..
		NetInputBuffer_c tIn ( iSock );

		// get next packet
		// we want interruptible calls here, so that shutdowns could be honored
		THD_STATE ( THD_NET_IDLE );
		if ( !tIn.ReadFrom ( 4, g_iClientQlTimeout, true ) )
		{
			sphLogDebugv ( "conn %s(%d): bailing on failed MySQL header (sockerr=%s)", sClientIP, iCID, sphSockError() );
			break;
		}

		// setup per-query profiling
		bool bProfile = tSession.m_tVars.m_bProfile; // the current statement might change it
		if ( bProfile )
		{
			tSession.m_tProfile.Start ( SPH_QSTATE_NET_READ );
			tOut.SetProfiler ( &tSession.m_tProfile );
		}

		// keep getting that packet
		THD_STATE ( THD_NET_READ );
		const int MAX_PACKET_LEN = 0xffffffL; // 16777215 bytes, max low level packet size
		DWORD uPacketHeader = tIn.GetLSBDword ();
		int iPacketLen = ( uPacketHeader & MAX_PACKET_LEN );
		if ( !tIn.ReadFrom ( iPacketLen, g_iClientQlTimeout, true ) )
		{
			sphWarning ( "failed to receive MySQL request body (client=%s(%d), exp=%d, error='%s')", sClientIP, iCID, iPacketLen, sphSockError() );
			break;
		}

		if ( bProfile )
			tSession.m_tProfile.Switch ( SPH_QSTATE_UNKNOWN );

		// handle it!
		uPacketID = 1 + (BYTE)( uPacketHeader>>24 ); // client will expect this id

		// handle big packets
		if ( iPacketLen==MAX_PACKET_LEN )
		{
			NetInputBuffer_c tIn2 ( iSock );
			int iAddonLen = -1;
			do
			{
				if ( !tIn2.ReadFrom ( 4, g_iClientQlTimeout, true ) )
				{
					sphLogDebugv ( "conn %s(%d): bailing on failed MySQL header2 (sockerr=%s)", sClientIP, iCID, sphSockError() );
					break;
				}

				DWORD uAddon = tIn2.GetLSBDword();
				uPacketID = 1 + (BYTE)( uAddon>>24 );
				iAddonLen = ( uAddon & MAX_PACKET_LEN );
				if ( !tIn.ReadFrom ( iAddonLen, g_iClientQlTimeout, true, true ) )
				{
					sphWarning ( "failed to receive MySQL request body2 (client=%s(%d), exp=%d, error='%s')", sClientIP, iCID, iAddonLen, sphSockError() );
					iAddonLen = -1;
					break;
				}
				iPacketLen += iAddonLen;
			} while ( iAddonLen==MAX_PACKET_LEN );
			if ( iAddonLen<0 )
				break;
			if ( iPacketLen<0 || iPacketLen>g_iMaxPacketSize )
			{
				sphWarning ( "ill-formed client request (length=%d out of bounds)", iPacketLen );
				break;
			}
		}

		// handle auth packet
		if ( !bAuthed )
		{
			THD_STATE ( THD_NET_WRITE );
			bAuthed = true;
			SendMysqlOkPacket ( tOut, uPacketID );
			tOut.Flush();
			if ( tOut.GetError() )
				break;
			else
				continue;
		}

		bool bRun = LoopClientMySQL ( uPacketID, tSession, sQuery, iPacketLen, bProfile, pThd, tIn, tOut );

		if ( !bRun )
			break;
	}

	// set off query guard
	SphCrashLogger_c::SetLastQuery ( CrashQuery_t() );
}

bool LoopClientMySQL ( BYTE & uPacketID, CSphinxqlSession & tSession, CSphString & sQuery, int iPacketLen, bool bProfile, ThdDesc_t * pThd, InputBuffer_c & tIn, ISphOutputBuffer & tOut )
{
	// get command, handle special packets
	const BYTE uMysqlCmd = tIn.GetByte ();
	if ( uMysqlCmd==MYSQL_COM_QUIT )
		return false;

	CSphString sError;
	bool bKeepProfile = true;
	switch ( uMysqlCmd )
	{
		case MYSQL_COM_PING:
		case MYSQL_COM_INIT_DB:
			// client wants a pong
			SendMysqlOkPacket ( tOut, uPacketID );
			break;

		case MYSQL_COM_SET_OPTION:
			// bMulti = ( tIn.GetWord()==MYSQL_OPTION_MULTI_STATEMENTS_ON ); // that's how we could double check and validate multi query
			// server reporting success in response to COM_SET_OPTION and COM_DEBUG
			SendMysqlEofPacket ( tOut, uPacketID, 0 );
			break;

		case MYSQL_COM_QUERY:
			// handle query packet
			assert ( uMysqlCmd==MYSQL_COM_QUERY );
			sQuery = tIn.GetRawString ( iPacketLen-1 ); // OPTIMIZE? could be huge; avoid copying?
			assert ( !tIn.GetError() );
			THD_STATE ( THD_QUERY );
			pThd->SetThreadInfo ( "%s", sQuery.cstr() ); // OPTIMIZE? could be huge; avoid copying?
			bKeepProfile = tSession.Execute ( sQuery, tOut, uPacketID, pThd );
			break;

		default:
			// default case, unknown command
			sError.SetSprintf ( "unknown command (code=%d)", uMysqlCmd );
				SendMysqlErrorPacket ( tOut, uPacketID, NULL, sError.cstr(), pThd->m_iConnID, MYSQL_ERR_UNKNOWN_COM_ERROR );
			break;
	}

	// send the response packet
	THD_STATE ( THD_NET_WRITE );
	tOut.Flush();
	if ( tOut.GetError() )
		return false;

	// finalize query profile
	if ( bProfile )
		tSession.m_tProfile.Stop();
	if ( uMysqlCmd==MYSQL_COM_QUERY && bKeepProfile )
		tSession.m_tLastProfile = tSession.m_tProfile;
	tOut.SetProfiler ( NULL );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// HANDLE-BY-LISTENER
//////////////////////////////////////////////////////////////////////////

void HandleClient ( ProtocolType_e eProto, int iSock, const char * sClientIP, ThdDesc_t * pThd )
{
	switch ( eProto )
	{
		case PROTO_SPHINX:		HandleClientSphinx ( iSock, sClientIP, pThd ); break;
		case PROTO_MYSQL41:		HandleClientMySQL ( iSock, sClientIP, pThd ); break;
		default:				assert ( 0 && "unhandled protocol type" ); break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// INDEX ROTATION
/////////////////////////////////////////////////////////////////////////////

bool TryRename ( const char * sIndex, const char * sPrefix, const char * sFromPostfix, const char * sToPostfix, const char * sAction, bool bFatal, bool bCheckExist )
{
	char sFrom [ SPH_MAX_FILENAME_LEN ];
	char sTo [ SPH_MAX_FILENAME_LEN ];

	snprintf ( sFrom, sizeof(sFrom), "%s%s", sPrefix, sFromPostfix );
	snprintf ( sTo, sizeof(sTo), "%s%s", sPrefix, sToPostfix );

#if USE_WINDOWS
	::unlink ( sTo );
#endif

	// if there is no file we have nothing to do
	if ( !bCheckExist && !sphIsReadable ( sFrom ) )
		return true;

	if ( rename ( sFrom, sTo ) )
	{
		if ( bFatal )
		{
			sphFatal ( "%s index '%s': rollback rename '%s' to '%s' failed: %s",
				sAction, sIndex, sFrom, sTo, strerror(errno) );
		} else
		{
			sphWarning ( "%s index '%s': rename '%s' to '%s' failed: %s",
				sAction, sIndex, sFrom, sTo, strerror(errno) );
		}
		return false;
	}

	return true;
}


bool HasFiles ( const char * sPath, const char ** dExts )
{
	char sFile [ SPH_MAX_FILENAME_LEN ];

	for ( int i=0; i<sphGetExtCount(); i++ )
	{
		snprintf ( sFile, sizeof(sFile), "%s%s", sPath, dExts[i] );
		if ( !sphIsReadable ( sFile ) )
			return false;
	}

	return true;
}

enum RotateFrom_e
{
	ROTATE_FROM_NONE,
	ROTATE_FROM_NEW,
	ROTATE_FROM_REENABLE,
	ROTATE_FROM_PATH_NEW,
	ROTATE_FROM_PATH_COPY
};

RotateFrom_e CheckIndexHeaderRotate ( const char * sIndexBase, const char * sNewPath, bool bOnlyNew )
{
	char sHeaderName [ SPH_MAX_FILENAME_LEN ];

	// check order:
	// current_path.new		- rotation of current index
	// current_path			- enable back current index
	// new_path.new			- rotation of current index but with new path via indexer --rotate
	// new_path				- rotation of current index but with new path via files copy

	snprintf ( sHeaderName, sizeof(sHeaderName), "%s%s", sIndexBase, sphGetExt ( SPH_EXT_TYPE_NEW, SPH_EXT_SPH ) );
	if ( sphIsReadable ( sHeaderName ) )
		return ROTATE_FROM_NEW;

	if ( bOnlyNew )
	{
		snprintf ( sHeaderName, sizeof(sHeaderName), "%s%s", sIndexBase, sphGetExt( SPH_EXT_TYPE_CUR, SPH_EXT_SPH ) );
		if ( sphIsReadable ( sHeaderName ) )
			return ROTATE_FROM_REENABLE;
	}

	if ( !sNewPath )
		return ROTATE_FROM_NONE;

	snprintf ( sHeaderName, sizeof ( sHeaderName ), "%s%s", sNewPath, sphGetExt ( SPH_EXT_TYPE_NEW, SPH_EXT_SPH ) );
	if ( sphIsReadable ( sHeaderName ) )
		return ROTATE_FROM_PATH_NEW;

	snprintf ( sHeaderName, sizeof ( sHeaderName ), "%s%s", sNewPath, sphGetExt ( SPH_EXT_TYPE_CUR, SPH_EXT_SPH ) );
	if ( sphIsReadable ( sHeaderName ) )
		return ROTATE_FROM_PATH_COPY;

	return ROTATE_FROM_NONE;
}


/// returns true if any version of the index (old or new one) has been preread
bool RotateIndexGreedy ( ServedDesc_t & tIndex, const char * sIndex, CSphString & sError )
{
	sphLogDebug ( "RotateIndexGreedy for '%s' invoked", sIndex );
	char sFile [ SPH_MAX_FILENAME_LEN ];
	const char * sPath = tIndex.m_sIndexPath.cstr();
	const char * sAction = "rotating";

	RotateFrom_e eRot = CheckIndexHeaderRotate ( sPath, tIndex.m_sNewPath.cstr(), tIndex.m_bOnlyNew );
	bool bReEnable = ( eRot==ROTATE_FROM_REENABLE );
	if ( eRot==ROTATE_FROM_PATH_NEW || eRot==ROTATE_FROM_PATH_COPY )
	{
		sError.SetSprintf ( "rotating index '%s': can not rotate from new path, switch to seamless_rotate=1; using old index", sIndex );
		return false;
	}

	snprintf ( sFile, sizeof( sFile ), "%s%s", sPath, sphGetExt ( bReEnable ? SPH_EXT_TYPE_CUR : SPH_EXT_TYPE_NEW, SPH_EXT_SPH ) );
	DWORD uVersion = ReadVersion ( sFile, sError );

	if ( !sError.IsEmpty() )
	{
		// no files - no rotation
		return false;
	}

	for ( int i=0; i<sphGetExtCount ( uVersion ); i++ )
	{
		snprintf ( sFile, sizeof( sFile ), "%s%s", sPath, sphGetExts ( bReEnable ? SPH_EXT_TYPE_CUR : SPH_EXT_TYPE_NEW, uVersion )[i] );
		if ( !sphIsReadable ( sFile ) )
		{
			if ( tIndex.m_bOnlyNew )
				sphWarning ( "rotating index '%s': '%s' unreadable: %s; NOT SERVING", sIndex, sFile, strerror ( errno ) );
			else
				sphWarning ( "rotating index '%s': '%s' unreadable: %s; using old index", sIndex, sFile, strerror ( errno ) );
			return false;
		}
	}

	if ( bReEnable )
	{
		sphLogDebug ( "RotateIndexGreedy: re-enabling index" );
	} else
	{
		sphLogDebug ( "RotateIndexGreedy: new index is readable" );
	}

	bool bNoMVP = true;
	if ( !tIndex.m_bOnlyNew )
	{
		// rename current to old
		for ( int i=0; i<sphGetExtCount ( uVersion ); i++ )
		{
			snprintf ( sFile, sizeof(sFile), "%s%s", sPath, sphGetExts ( SPH_EXT_TYPE_CUR, uVersion )[i] );
			if ( !sphIsReadable ( sFile ) || TryRename ( sIndex, sPath, sphGetExts ( SPH_EXT_TYPE_CUR, uVersion )[i], sphGetExts ( SPH_EXT_TYPE_OLD, uVersion )[i], sAction, false, true ) )
				continue;

			// rollback
			for ( int j=0; j<i; j++ )
				TryRename ( sIndex, sPath, sphGetExts ( SPH_EXT_TYPE_OLD, uVersion )[j], sphGetExts ( SPH_EXT_TYPE_CUR, uVersion )[j], sAction, true, true );

			sError.SetSprintf ( "rotating index '%s': rename to .old failed; using old index", sIndex );
			return false;
		}

		// holding the persistent MVA updates (.mvp).
		for ( ;; )
		{
			char sBuf [ SPH_MAX_FILENAME_LEN ];
			snprintf ( sBuf, sizeof(sBuf), "%s%s", sPath, sphGetExt ( SPH_EXT_TYPE_CUR, SPH_EXT_MVP ) );

			CSphString sFakeError;
			CSphAutofile fdTest ( sBuf, SPH_O_READ, sFakeError );
			bNoMVP = ( fdTest.GetFD()<0 );
			fdTest.Close();
			if ( bNoMVP )
				break; ///< no file, nothing to hold

			if ( TryRename ( sIndex, sPath, sphGetExt ( SPH_EXT_TYPE_CUR, SPH_EXT_MVP ), sphGetExt ( SPH_EXT_TYPE_OLD, SPH_EXT_MVP ), sAction, false, false ) )
				break;

			// rollback
			for ( int j=0; j<sphGetExtCount ( uVersion ); j++ )
				TryRename ( sIndex, sPath, sphGetExts ( SPH_EXT_TYPE_OLD, uVersion )[j], sphGetExts ( SPH_EXT_TYPE_CUR, uVersion )[j], sAction, true, true );

			break;
		}
		sphLogDebug ( "RotateIndexGreedy: Current index renamed to .old" );
	}

	// rename new to current
	if ( !bReEnable )
	{
		for ( int i=0; i<sphGetExtCount ( uVersion ); i++ )
		{
			if ( TryRename ( sIndex, sPath, sphGetExts ( SPH_EXT_TYPE_NEW, uVersion )[i], sphGetExts ( SPH_EXT_TYPE_CUR, uVersion )[i], sAction, false, true ) )
				continue;

			// rollback new ones we already renamed
			for ( int j=0; j<i; j++ )
				TryRename ( sIndex, sPath, sphGetExts ( SPH_EXT_TYPE_CUR, uVersion )[j], sphGetExts ( SPH_EXT_TYPE_NEW, uVersion )[j], sAction, true, true );

			// rollback old ones
			if ( !tIndex.m_bOnlyNew )
			{
				for ( int j=0; j<sphGetExtCount ( uVersion ); j++ )
					TryRename ( sIndex, sPath, sphGetExts ( SPH_EXT_TYPE_OLD, uVersion )[j], sphGetExts ( SPH_EXT_TYPE_CUR, uVersion )[j], sAction, true, true );

				if ( !bNoMVP )
					TryRename ( sIndex, sPath, sphGetExt ( SPH_EXT_TYPE_OLD, SPH_EXT_MVP ), sphGetExt ( SPH_EXT_TYPE_CUR, SPH_EXT_MVP ), sAction, true, true );
			}

			return false;
		}
		sphLogDebug ( "RotateIndexGreedy: New renamed to current" );
	}

	bool bPreread = false;

	// try to use new index
	ISphTokenizer * pTokenizer = tIndex.m_pIndex->LeakTokenizer (); // FIXME! disable support of that old indexes and remove this bullshit
	CSphDict * pDictionary = tIndex.m_pIndex->LeakDictionary ();
	tIndex.m_pIndex->SetGlobalIDFPath ( tIndex.m_sGlobalIDFPath );

	if ( !tIndex.m_pIndex->Prealloc ( g_bStripPath ) )
	{
		if ( tIndex.m_bOnlyNew )
		{
			sError.SetSprintf ( "rotating index '%s': .new preload failed: %s; NOT SERVING", sIndex, tIndex.m_pIndex->GetLastError().cstr() );
			return false;

		} else
		{
			sphWarning ( "rotating index '%s': .new preload failed: %s", sIndex, tIndex.m_pIndex->GetLastError().cstr() );

			// try to recover
			for ( int j=0; j<sphGetExtCount ( uVersion ); j++ )
			{
				TryRename ( sIndex, sPath, sphGetExts ( SPH_EXT_TYPE_CUR, uVersion )[j], sphGetExts ( SPH_EXT_TYPE_NEW, uVersion )[j], sAction, true, true );
				TryRename ( sIndex, sPath, sphGetExts ( SPH_EXT_TYPE_OLD, uVersion )[j], sphGetExts ( SPH_EXT_TYPE_CUR, uVersion )[j], sAction, true, true );
			}
			TryRename ( sIndex, sPath, sphGetExt ( SPH_EXT_TYPE_OLD, SPH_EXT_MVP ), sphGetExt ( SPH_EXT_TYPE_CUR, SPH_EXT_MVP ), sAction, false, false );
			sphLogDebug ( "RotateIndexGreedy: has recovered" );

			if ( !tIndex.m_pIndex->Prealloc ( g_bStripPath ) )
			{
				sError.SetSprintf ( "rotating index '%s': .new preload failed; ROLLBACK FAILED; INDEX UNUSABLE", sIndex );
				tIndex.m_bEnabled = false;

			} else
			{
				tIndex.m_bEnabled = true;
				bPreread = true;
			}

			if ( !tIndex.m_pIndex->GetLastWarning().IsEmpty() )
				sphWarning ( "rotating index '%s': %s", sIndex, tIndex.m_pIndex->GetLastWarning().cstr() );

			if ( !tIndex.m_pIndex->GetTokenizer () )
				tIndex.m_pIndex->SetTokenizer ( pTokenizer );
			else
				SafeDelete ( pTokenizer );

			if ( !tIndex.m_pIndex->GetDictionary () )
				tIndex.m_pIndex->SetDictionary ( pDictionary );
			else
				SafeDelete ( pDictionary );
		}

		return bPreread;

	} else
	{
		bPreread = true;

		if ( !tIndex.m_pIndex->GetLastWarning().IsEmpty() )
			sphWarning ( "rotating index '%s': %s", sIndex, tIndex.m_pIndex->GetLastWarning().cstr() );
	}

	if ( !tIndex.m_pIndex->GetTokenizer () )
		tIndex.m_pIndex->SetTokenizer ( pTokenizer );
	else
		SafeDelete ( pTokenizer );

	if ( !tIndex.m_pIndex->GetDictionary () )
		tIndex.m_pIndex->SetDictionary ( pDictionary );
	else
		SafeDelete ( pDictionary );

	// unlink .old
	if ( !tIndex.m_bOnlyNew )
	{
		snprintf ( sFile, sizeof(sFile), "%s.old", sPath );
		sphUnlinkIndex ( sFile, false );
	}

	sphLogDebug ( "RotateIndexGreedy: the old index unlinked" );

	// uff. all done
	tIndex.m_bEnabled = true;
	tIndex.m_bOnlyNew = false;
	sphInfo ( "rotating index '%s': success", sIndex );
	return bPreread;
}

/////////////////////////////////////////////////////////////////////////////
// MAIN LOOP
/////////////////////////////////////////////////////////////////////////////


void DumpMemStat ()
{
#if SPH_ALLOCS_PROFILER
	sphMemStatDump ( g_iLogFile );
#endif
}

/// check and report if there were any leaks since last call
void CheckLeaks ()
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

static bool CheckServedEntry ( const ServedIndex_c * pEntry, const char * sIndex, CSphString & sError )
{
	if ( !pEntry )
	{
		sError.SetSprintf ( "rotating index '%s': INTERNAL ERROR, index went AWOL", sIndex );
		return false;
	}

	if ( pEntry->m_bToDelete || !pEntry->m_pIndex )
	{
		if ( pEntry->m_bToDelete )
			sError.SetSprintf ( "rotating index '%s': INTERNAL ERROR, entry marked for deletion", sIndex );

		if ( !pEntry->m_pIndex )
			sError.SetSprintf ( "rotating index '%s': INTERNAL ERROR, entry does not have an index", sIndex );

		return false;
	}

	return true;
}


#define SPH_RT_AUTO_FLUSH_CHECK_PERIOD ( 5000000 )

static void RtFlushThreadFunc ( void * )
{
	int64_t tmNextCheck = sphMicroTimer() + SPH_RT_AUTO_FLUSH_CHECK_PERIOD;
	while ( !g_bShutdown )
	{
		// stand still till save time
		if ( tmNextCheck>sphMicroTimer() )
		{
			sphSleepMsec ( 50 );
			continue;
		}

		// collecting available rt indexes at save time
		CSphVector<CSphString> dRtIndexes;
		for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
			if ( it.Get().m_bRT )
				dRtIndexes.Add ( it.GetKey() );

		// do check+save
		ARRAY_FOREACH_COND ( i, dRtIndexes, !g_bShutdown )
		{
			const ServedIndex_c * pServed = g_pLocalIndexes->GetRlockedEntry ( dRtIndexes[i] );
			if ( !pServed )
				continue;

			if ( !pServed->m_bEnabled )
			{
				pServed->Unlock();
				continue;
			}

			ISphRtIndex * pRT = (ISphRtIndex *)pServed->m_pIndex;
			pRT->CheckRamFlush();

			pServed->Unlock();
		}

		tmNextCheck = sphMicroTimer() + SPH_RT_AUTO_FLUSH_CHECK_PERIOD;
	}
}


static bool RotateIndexMT ( const CSphString & sIndex, CSphString & sError )
{
	//////////////////
	// load new index
	//////////////////

	// create new index, copy some settings from existing one
	const ServedIndex_c * pRotating = g_pLocalIndexes->GetRlockedEntry ( sIndex );
	if ( !CheckServedEntry ( pRotating, sIndex.cstr(), sError ) )
	{
		if ( pRotating )
			pRotating->Unlock();

		return false;
	}

	sphInfo ( "rotating index '%s': started", sIndex.cstr() );

	ServedDesc_t tNewIndex;
	tNewIndex.m_bOnlyNew = pRotating->m_bOnlyNew;

	tNewIndex.m_pIndex = sphCreateIndexPhrase ( sIndex.cstr(), NULL );
	tNewIndex.m_pIndex->m_bExpandKeywords = pRotating->m_bExpand;
	tNewIndex.m_pIndex->m_iExpansionLimit = g_iExpansionLimit;
	tNewIndex.m_pIndex->SetPreopen ( pRotating->m_bPreopen || g_bPreopenIndexes );
	tNewIndex.m_pIndex->SetGlobalIDFPath ( pRotating->m_sGlobalIDFPath );
	tNewIndex.m_bMlock = pRotating->m_bMlock;
	tNewIndex.m_bOnDiskAttrs = pRotating->m_bOnDiskAttrs;
	tNewIndex.m_bOnDiskPools = pRotating->m_bOnDiskPools;
	tNewIndex.m_pIndex->SetMemorySettings ( tNewIndex.m_bMlock, tNewIndex.m_bOnDiskAttrs, tNewIndex.m_bOnDiskPools );

	CSphString sIndexPath = pRotating->m_sIndexPath;
	CSphString sNewPath = pRotating->m_sNewPath;
	// don't need to hold the existing index any more now
	pRotating->Unlock();
	pRotating = NULL;

	RotateFrom_e eRot = CheckIndexHeaderRotate ( sIndexPath.cstr(), sNewPath.cstr(), tNewIndex.m_bOnlyNew );
	if ( eRot==ROTATE_FROM_NONE )
	{
		sphWarning ( "nothing to rotate for index '%s'", sIndex.cstr() );
		return false;
	}

	bool bRenameIncoming = true;
	char sPathFrom[SPH_MAX_FILENAME_LEN];
	char sPathTo[SPH_MAX_FILENAME_LEN];
	switch ( eRot )
	{
		case ROTATE_FROM_PATH_NEW:
			snprintf ( sPathFrom, sizeof ( sPathFrom ), "%s.new", sNewPath.cstr() );
			snprintf ( sPathTo, sizeof ( sPathTo ), "%s", sNewPath.cstr() );
		break;

		case ROTATE_FROM_PATH_COPY:
			snprintf ( sPathFrom, sizeof ( sPathFrom ), "%s", sNewPath.cstr() );
			sPathTo[0] = '\0';
			bRenameIncoming = false;
		break;

		case ROTATE_FROM_REENABLE:
			snprintf ( sPathFrom, sizeof ( sPathFrom ), "%s", sIndexPath.cstr() );
			sPathTo[0] = '\0';
			bRenameIncoming = false;
		break;

		case ROTATE_FROM_NEW:
		default:
			snprintf ( sPathFrom, sizeof ( sPathFrom ), "%s.new", sIndexPath.cstr() );
			snprintf ( sPathTo, sizeof ( sPathTo ), "%s", sIndexPath.cstr() );
		break;
	}

	tNewIndex.m_pIndex->SetBase ( sPathFrom );

	// prealloc enough RAM and lock new index
	sphLogDebug ( "prealloc enough RAM and lock new index" );
	if ( !tNewIndex.m_pIndex->Prealloc ( g_bStripPath ) )
	{
		sError.SetSprintf ( "rotating index '%s': prealloc: %s; using old index", sIndex.cstr(), tNewIndex.m_pIndex->GetLastError().cstr() );
		return false;
	}

	if ( !tNewIndex.m_pIndex->Lock() )
	{
		sError.SetSprintf ( "rotating index '%s': lock: %s; using old index", sIndex.cstr (), tNewIndex.m_pIndex->GetLastError().cstr() );
		return false;
	}

	// fixup settings if needed
	sphLogDebug ( "fixup settings if needed" );

	bool bFixedOk = true;
	const CSphConfigSection * pIndexConfig = NULL;

	g_tRotateConfigMutex.Lock ();
	if ( tNewIndex.m_bOnlyNew && g_pCfg.m_tConf ( "index" ) )
		pIndexConfig = g_pCfg.m_tConf["index"]( sIndex.cstr() );

	if ( pIndexConfig && !sphFixupIndexSettings ( tNewIndex.m_pIndex, *pIndexConfig, sError ) )
	{
		sError.SetSprintf ( "rotating index '%s': fixup: %s; using old index", sIndex.cstr(), sError.cstr() );
		bFixedOk = false;
	}
	g_tRotateConfigMutex.Unlock ();
	if ( !bFixedOk )
		return false;

	tNewIndex.m_pIndex->Preread();

	CSphIndexStatus tStatus;
	tNewIndex.m_pIndex->GetStatus ( &tStatus );
	tNewIndex.m_iMass = CalculateMass ( tStatus );

	//////////////////////
	// activate new index
	//////////////////////

	sphLogDebug ( "activate new index" );

	ServedIndex_c * pServed = g_pLocalIndexes->GetWlockedEntry ( sIndex );
	pServed->m_sNewPath = "";
	if ( !CheckServedEntry ( pServed, sIndex.cstr(), sError ) )
	{
		if ( pServed )
			pServed->Unlock();
		return false;
	}

	CSphIndex * pOld = pServed->m_pIndex;
	CSphIndex * pNew = tNewIndex.m_pIndex;

	// rename files, active current to old
	// FIXME! factor out a common function w/ non-threaded rotation code
	char sRollback [SPH_MAX_FILENAME_LEN];
	snprintf ( sRollback, sizeof ( sRollback ), "%s", pServed->m_sIndexPath.cstr () );
	char sOld [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sOld, sizeof(sOld), "%s.old", pServed->m_sIndexPath.cstr() );
	char sHeader [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sHeader, sizeof(sHeader), "%s%s", pServed->m_sIndexPath.cstr(), sphGetExt ( SPH_EXT_TYPE_CUR, SPH_EXT_SPH ) );

	if ( !pServed->m_bOnlyNew && sphIsReadable ( sHeader ) && !pOld->Rename ( sOld ) )
	{
		// FIXME! rollback inside Rename() call potentially fail
		sError.SetSprintf ( "rotating index '%s': cur to old rename failed: %s", sIndex.cstr(), pOld->GetLastError().cstr() );
		pServed->Unlock();
		return false;
	}

	// FIXME! at this point there's no cur lock file; ie. potential race
	sphLogDebug ( "no cur lock file; ie. potential race" );
	if ( bRenameIncoming && !pNew->Rename ( sPathTo ) )
	{
		sError.SetSprintf ( "rotating index '%s': new to cur rename failed: %s", sIndex.cstr(), pNew->GetLastError().cstr() );
		if ( !pServed->m_bOnlyNew && !pOld->Rename ( sRollback ) )
		{
			sError.SetSprintf ( "rotating index '%s': old to cur rename failed: %s; INDEX UNUSABLE", sIndex.cstr(), pOld->GetLastError().cstr() );
			pServed->m_bEnabled = false;
		}
		pServed->Unlock();
		return false;
	}

	// TODO: on Windows should lock again index in case it was enabled back or picked from new location

	// all went fine; swap them
	sphLogDebug ( "all went fine; swap them" );

	tNewIndex.m_pIndex->m_iTID = pServed->m_pIndex->m_iTID;
	if ( g_pBinlog )
		g_pBinlog->NotifyIndexFlush ( sIndex.cstr(), pServed->m_pIndex->m_iTID, false );

	if ( !tNewIndex.m_pIndex->GetTokenizer() )
		tNewIndex.m_pIndex->SetTokenizer ( pServed->m_pIndex->LeakTokenizer() );

	if ( !tNewIndex.m_pIndex->GetDictionary() )
		tNewIndex.m_pIndex->SetDictionary ( pServed->m_pIndex->LeakDictionary() );

	Swap ( pServed->m_pIndex, tNewIndex.m_pIndex );
	pServed->m_bEnabled = true;
	pServed->m_sIndexPath = pServed->m_pIndex->GetFilename();
	tNewIndex.m_pIndex->Dealloc(); // unlink does not work on windows for open mmap'ed file with write access

	// rename current MVP to old one to unlink it as MVP was not renamed at pOld->Rename
	if ( bRenameIncoming )
		TryRename ( sIndex.cstr (), pServed->m_sIndexPath.cstr(), sphGetExt ( SPH_EXT_TYPE_CUR, SPH_EXT_MVP ), sphGetExt ( SPH_EXT_TYPE_OLD, SPH_EXT_MVP ), "rotating", false, false );

	if ( !pServed->m_bOnlyNew )
	{
		// unlink .old
		sphLogDebug ( "unlink .old" );
		sphUnlinkIndex ( sOld, false );
	}

	pServed->m_bOnlyNew = false;
	sphInfo ( "rotating index '%s': success", sIndex.cstr() );

	pServed->Unlock();
	return true;
}

void RotationThreadFunc ( void * )
{
	while ( !g_bShutdown )
	{
		// check if we have work to do
		g_tRotateQueueMutex.Lock();
		if ( !g_dRotateQueue.GetLength() )
		{
			g_tRotateQueueMutex.Unlock();
			sphSleepMsec ( 50 );
			continue;
		}

		CSphString sIndex = g_dRotateQueue.Pop();
		g_tRotateQueueMutex.Unlock();

		CSphString sError;
		if ( !RotateIndexMT ( sIndex, sError ) )
			sphWarning ( "%s", sError.cstr() );

		// FIXME!!! fix that mess with queue and atomic and sleep by event and queue
		// FIXME!!! move IDF rotation here too
		g_tRotateQueueMutex.Lock();
		if ( !g_dRotateQueue.GetLength() )
		{
			g_bInRotate = false;
			g_bInvokeRotationService = true;
			sphInfo ( "rotating index: all indexes done" );
		}
		g_tRotateQueueMutex.Unlock();
	}
}

static void PrereadFunc ( void * )
{
	int64_t tmStart = sphMicroTimer();

	CSphVector<CSphString> dIndexes;
	for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
	{
		ServedIndex_c & tIndex = it.Get();
		if ( tIndex.m_bEnabled )
			dIndexes.Add ( it.GetKey() );
	}

	sphInfo ( "prereading %d indexes", dIndexes.GetLength() );
	int iReaded = 0;

	for ( int i=0; i<dIndexes.GetLength() && !g_bShutdown; i++ )
	{
		const CSphString & sName = dIndexes[i];
		const ServedIndex_c * pServed = g_pLocalIndexes->GetRlockedEntry ( sName );
		if ( !pServed )
			continue;

		if ( !pServed->m_bEnabled )
		{
			pServed->Unlock();
			continue;
		}

		int64_t tmReading = sphMicroTimer();
		sphLogDebug ( "prereading index '%s'", pServed->m_pIndex->GetName() );

		pServed->m_pIndex->Preread();
		if ( !pServed->m_pIndex->GetLastWarning().IsEmpty() )
			sphWarning ( "'%s' preread: %s", pServed->m_pIndex->GetName(), pServed->m_pIndex->GetLastWarning().cstr() );

		int64_t tmReaded = sphMicroTimer() - tmReading;
		sphLogDebug ( "prereaded index '%s' in %0.3f sec", pServed->m_pIndex->GetName(), float(tmReaded)/1000000.0f );

		pServed->Unlock();

		iReaded++;
	}

	int64_t tmFinished = sphMicroTimer() - tmStart;
	sphInfo ( "prereaded %d indexes in %0.3f sec", iReaded, float(tmFinished)/1000000.0f );
}


//////////////////////////////////////////////////////////////////////////
// SPHINXQL STATE
//////////////////////////////////////////////////////////////////////////

struct NamedRefVectorPair_t
{
	CSphString			m_sName;
	UservarIntSet_c *	m_pVal;
};


/// SphinxQL state writer thread
/// periodically flushes changes of uservars, UDFs
static void SphinxqlStateThreadFunc ( void * )
{
	assert ( !g_sSphinxqlState.IsEmpty() );
	CSphString sNewState;
	sNewState.SetSprintf ( "%s.new", g_sSphinxqlState.cstr() );

	char dBuf[512];
	const int iMaxString = 80;
	assert ( (int)sizeof(dBuf) > iMaxString );

	CSphString sError;
	CSphWriter tWriter;

	int64_t tmLast = g_tmSphinxqlState;
	while ( !g_bShutdown )
	{
		// stand still till save time
		if ( tmLast==g_tmSphinxqlState )
		{
			sphSleepMsec ( QLSTATE_FLUSH_MSEC );
			continue;
		}

		// close and truncate the .new file
		tWriter.CloseFile ( true );
		if ( !tWriter.OpenFile ( sNewState, sError ) )
		{
			sphWarning ( "sphinxql_state flush failed: %s", sError.cstr() );
			sphSleepMsec ( QLSTATE_FLUSH_MSEC );
			continue;
		}

		/////////////
		// save UDFs
		/////////////

		sphPluginSaveState ( tWriter );

		/////////////////
		// save uservars
		/////////////////

		tmLast = g_tmSphinxqlState;
		CSphVector<NamedRefVectorPair_t> dUservars;
		dUservars.Reserve ( g_hUservars.GetLength() );
		g_tUservarsMutex.Lock();
		g_hUservars.IterateStart();
		while ( g_hUservars.IterateNext() )
		{
			if ( !g_hUservars.IterateGet().m_pVal->GetLength() )
				continue;

			NamedRefVectorPair_t & tPair = dUservars.Add();
			tPair.m_sName = g_hUservars.IterateGetKey();
			tPair.m_pVal = g_hUservars.IterateGet().m_pVal;
			tPair.m_pVal->AddRef();
		}
		g_tUservarsMutex.Unlock();

		dUservars.Sort ( bind ( &NamedRefVectorPair_t::m_sName ) );

		// reinitiate store process on new variables added
		ARRAY_FOREACH_COND ( i, dUservars, tmLast==g_tmSphinxqlState )
		{
			const CSphVector<SphAttr_t> & dVals = *dUservars[i].m_pVal;
			int iLen = snprintf ( dBuf, sizeof ( dBuf ), "SET GLOBAL %s = ( "INT64_FMT, dUservars[i].m_sName.cstr(), dVals[0] );
			for ( int j=1; j<dVals.GetLength(); j++ )
			{
				iLen += snprintf ( dBuf+iLen, sizeof ( dBuf ), ", "INT64_FMT, dVals[j] );

				if ( iLen>=iMaxString && j<dVals.GetLength()-1 )
				{
					iLen += snprintf ( dBuf+iLen, sizeof ( dBuf ), " \\\n" );
					tWriter.PutBytes ( dBuf, iLen );
					iLen = 0;
				}
			}

			if ( iLen )
				tWriter.PutBytes ( dBuf, iLen );

			char sTail[] = " );\n";
			tWriter.PutBytes ( sTail, sizeof ( sTail )-1 );
		}

		// release all locked uservars
		ARRAY_FOREACH ( i, dUservars )
			dUservars[i].m_pVal->Release();

		/////////////////////////////////
		// writing done, flip the burger
		/////////////////////////////////

		tWriter.CloseFile();
		if ( ::rename ( sNewState.cstr(), g_sSphinxqlState.cstr() )==0 )
		{
			::unlink ( sNewState.cstr() );
		} else
		{
			sphWarning ( "sphinxql_state flush: rename %s to %s failed: %s",
				sNewState.cstr(), g_sSphinxqlState.cstr(), strerror(errno) );
		}
	}
}


/// process a single line from sphinxql state/startup script
static bool SphinxqlStateLine ( CSphVector<char> & dLine, CSphString * sError )
{
	assert ( sError );
	if ( !dLine.GetLength() )
		return true;

	// parser expects CSphString buffer with gap bytes at the end
	if ( dLine.Last()==';' )
		dLine.Pop();
	dLine.Add ( '\0' );
	dLine.Add ( '\0' );
	dLine.Add ( '\0' );

	CSphVector<SqlStmt_t> dStmt;
	bool bParsedOK = sphParseSqlQuery ( dLine.Begin(), dLine.GetLength(), dStmt, *sError, SPH_COLLATION_DEFAULT );
	if ( !bParsedOK )
		return false;

	bool bOk = true;
	ARRAY_FOREACH ( i, dStmt )
	{
		SqlStmt_t & tStmt = dStmt[i];
		if ( tStmt.m_eStmt==STMT_SET && tStmt.m_eSet==SET_GLOBAL_UVAR )
		{
			tStmt.m_dSetValues.Sort();
			UservarAdd ( tStmt.m_sSetName, tStmt.m_dSetValues );
		} else if ( tStmt.m_eStmt==STMT_CREATE_FUNCTION )
		{
			bOk &= sphPluginCreate ( tStmt.m_sUdfLib.cstr(), PLUGIN_FUNCTION, tStmt.m_sUdfName.cstr(), tStmt.m_eUdfType, *sError );

		} else if ( tStmt.m_eStmt==STMT_CREATE_PLUGIN )
		{
			bOk &= sphPluginCreate ( tStmt.m_sUdfLib.cstr(), sphPluginGetType ( tStmt.m_sStringParam ),
				tStmt.m_sUdfName.cstr(), SPH_ATTR_NONE, *sError );
		} else
		{
			bOk = false;
			sError->SetSprintf ( "unsupported statement (must be one of SET GLOBAL, CREATE FUNCTION, CREATE PLUGIN)" );
		}
	}

	return bOk;
}


/// uservars table reader
static void SphinxqlStateRead ( const CSphString & sName )
{
	if ( sName.IsEmpty() )
		return;

	CSphString sError;
	CSphAutoreader tReader;
	if ( !tReader.Open ( sName, sError ) )
		return;

	const int iReadBlock = 32*1024;
	const int iGapLen = 2;
	CSphVector<char> dLine;
	dLine.Reserve ( iReadBlock + iGapLen );

	bool bEscaped = false;
	int iLines = 0;
	for ( ;; )
	{
		const BYTE * pData = NULL;
		int iRead = tReader.GetBytesZerocopy ( &pData, iReadBlock );
		// all uservars got read
		if ( iRead<=0 )
			break;

		// read escaped line
		dLine.Reserve ( dLine.GetLength() + iRead + iGapLen );
		const BYTE * s = pData;
		const BYTE * pEnd = pData+iRead;
		while ( s<pEnd )
		{
			// goto next line for escaped string
			if ( *s=='\\' || ( bEscaped && ( *s=='\n' || *s=='\r' ) ) )
			{
				s++;
				while ( s<pEnd && ( *s=='\n' || *s=='\r' ) )
				{
					iLines += ( *s=='\n' );
					s++;
				}
				bEscaped = ( s>=pEnd );
				continue;
			}

			bEscaped = false;
			if ( *s=='\n' || *s=='\r' )
			{
				if ( !SphinxqlStateLine ( dLine, &sError ) )
					sphWarning ( "sphinxql_state: parse error at line %d: %s", 1+iLines, sError.cstr() );

				dLine.Resize ( 0 );
				s++;
				while ( s<pEnd && ( *s=='\n' || *s=='\r' ) )
				{
					iLines += ( *s=='\n' );
					s++;
				}
				continue;
			}

			dLine.Add ( *s );
			s++;
		}
	}

	if ( !SphinxqlStateLine ( dLine, &sError ) )
		sphWarning ( "sphinxql_state: parse error at line %d: %s", 1+iLines, sError.cstr() );
}

//////////////////////////////////////////////////////////////////////////

void OptimizeThreadFunc ( void * )
{
	while ( !g_bShutdown )
	{
		// stand still till optimize time
		if ( !g_dOptimizeQueue.GetLength() )
		{
			sphSleepMsec ( 50 );
			continue;
		}

		CSphString sIndex;
		g_tOptimizeQueueMutex.Lock();
		if ( g_dOptimizeQueue.GetLength() )
		{
			sIndex = g_dOptimizeQueue[0];
			g_dOptimizeQueue.Remove(0);
		}
		g_tOptimizeQueueMutex.Unlock();

		const ServedIndex_c * pServed = g_pLocalIndexes->GetRlockedEntry ( sIndex );
		if ( !pServed )
		{
			continue;
		}
		if ( !pServed->m_pIndex || !pServed->m_bEnabled )
		{
			pServed->Unlock();
			continue;
		}

		// FIXME: MVA update would wait w-lock here for a very long time
		assert ( pServed->m_bRT );
		static_cast<ISphRtIndex *>( pServed->m_pIndex )->Optimize ( &g_bShutdown, &g_tRtThrottle );

		pServed->Unlock();
	}
}


void ConfigureTemplateIndex ( ServedDesc_t & tIdx, const CSphConfigSection & hIndex )
{
	tIdx.m_bExpand = ( hIndex.GetInt ( "expand_keywords", 0 )!=0 );
}

void ConfigureLocalIndex ( ServedDesc_t & tIdx, const CSphConfigSection & hIndex )
{
	tIdx.m_bMlock = ( hIndex.GetInt ( "mlock", 0 )!=0 ) && !g_bOptNoLock;
	tIdx.m_bExpand = ( hIndex.GetInt ( "expand_keywords", 0 )!=0 );
	tIdx.m_bPreopen = ( hIndex.GetInt ( "preopen", 0 )!=0 );
	tIdx.m_sGlobalIDFPath = hIndex.GetStr ( "global_idf" );
	tIdx.m_bOnDiskAttrs = ( hIndex.GetInt ( "ondisk_attrs", 0 )==1 );
	tIdx.m_bOnDiskPools = ( strcmp ( hIndex.GetStr ( "ondisk_attrs", "" ), "pool" )==0 );
	tIdx.m_bOnDiskAttrs |= g_bOnDiskAttrs;
	tIdx.m_bOnDiskPools |= g_bOnDiskPools;
}


/// this gets called for every new physical index
/// that is, local and RT indexes, but not distributed once
bool PrereadNewIndex ( ServedDesc_t & tIdx, const CSphConfigSection & hIndex, const char * szIndexName )
{
	bool bOk = tIdx.m_pIndex->Prealloc ( g_bStripPath );
	if ( !bOk )
	{
		sphWarning ( "index '%s': prealloc: %s; NOT SERVING", szIndexName, tIdx.m_pIndex->GetLastError().cstr() );
		return false;
	}

	// tricky bit
	// fixup was initially intended for (very old) index formats that did not store dict/tokenizer settings
	// however currently it also ends up configuring dict/tokenizer for fresh RT indexes!
	// (and for existing RT indexes, settings get loaded during the Prealloc() call)
	CSphString sError;
	if ( !sphFixupIndexSettings ( tIdx.m_pIndex, hIndex, sError ) )
	{
		sphWarning ( "index '%s': %s - NOT SERVING", szIndexName, sError.cstr() );
		return false;
	}

	// try to lock it
	if ( !g_bOptNoLock && !tIdx.m_pIndex->Lock() )
	{
		sphWarning ( "index '%s': lock: %s; NOT SERVING", szIndexName, tIdx.m_pIndex->GetLastError().cstr() );
		return false;
	}

	CSphIndexStatus tStatus;
	tIdx.m_pIndex->GetStatus ( &tStatus );
	tIdx.m_iMass = CalculateMass ( tStatus );
	return true;
}

static void ConfigureDistributedIndex ( DistributedIndex_t * pIdx, const char * szIndexName, const CSphConfigSection & hIndex )
{
	assert ( hIndex("type") && hIndex["type"]=="distributed" );
	assert ( pIdx!=NULL );

	DistributedIndex_t & tIdx = *pIdx;

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
	CSphVector<CSphString> dLocs;
	for ( CSphVariant * pLocal = hIndex("local"); pLocal; pLocal = pLocal->m_pNext )
	{
		dLocs.Resize(0);
		sphSplit ( dLocs, pLocal->cstr(), " \t," );
		ARRAY_FOREACH ( i, dLocs )
		{
			if ( !g_pLocalIndexes->Exists ( dLocs[i] ) )
			{
				sphWarning ( "index '%s': no such local index '%s', SKIPPED", szIndexName, dLocs[i].cstr() );
				continue;
			}
			tIdx.m_dLocal.Add ( dLocs[i] );
		}
	}

	AgentOptions_t tAgentOptions;
	tAgentOptions.m_bBlackhole = false;
	tAgentOptions.m_bPersistent = false;
	tAgentOptions.m_eStrategy = tIdx.m_eHaStrategy;
	// add remote agents
	for ( CSphVariant * pAgent = hIndex("agent"); pAgent; pAgent = pAgent->m_pNext )
	{
		MetaAgentDesc_t& tAgent = tIdx.m_dAgents.Add();
		if ( !ConfigureAgent ( tAgent, pAgent, szIndexName, tAgentOptions ) )
			tIdx.m_dAgents.Pop();
	}

	// for now work with client persistent connections only on per-thread basis,
	// to avoid locks, etc.
	tAgentOptions.m_bBlackhole = false;
	tAgentOptions.m_bPersistent = bEnablePersistentConns;
	for ( CSphVariant * pAgent = hIndex("agent_persistent"); pAgent; pAgent = pAgent->m_pNext )
	{
		MetaAgentDesc_t& tAgent = tIdx.m_dAgents.Add ();
		if ( !ConfigureAgent ( tAgent, pAgent, szIndexName, tAgentOptions ) )
			tIdx.m_dAgents.Pop();
	}

	tAgentOptions.m_bBlackhole = true;
	tAgentOptions.m_bPersistent = false;
	for ( CSphVariant * pAgent = hIndex("agent_blackhole"); pAgent; pAgent = pAgent->m_pNext )
	{
		MetaAgentDesc_t& tAgent = tIdx.m_dAgents.Add ();
		if ( !ConfigureAgent ( tAgent, pAgent, szIndexName, tAgentOptions ) )
			tIdx.m_dAgents.Pop();
	}

	// configure options
	if ( hIndex("agent_connect_timeout") )
	{
		if ( hIndex["agent_connect_timeout"].intval()<=0 )
			sphWarning ( "index '%s': agent_connect_timeout must be positive, ignored", szIndexName );
		else
			tIdx.m_iAgentConnectTimeout = hIndex["agent_connect_timeout"].intval();
	}

	tIdx.m_bDivideRemoteRanges = hIndex.GetInt ( "divide_remote_ranges", 0 )!=0;

	if ( hIndex("agent_query_timeout") )
	{
		if ( hIndex["agent_query_timeout"].intval()<=0 )
			sphWarning ( "index '%s': agent_query_timeout must be positive, ignored", szIndexName );
		else
			tIdx.m_iAgentQueryTimeout = hIndex["agent_query_timeout"].intval();
	}

	bool bHaveHA = ARRAY_ANY ( bHaveHA, tIdx.m_dAgents, tIdx.m_dAgents[_any].IsHA() );

	// configure ha_strategy
	if ( bSetHA && !bHaveHA )
		sphWarning ( "index '%s': ha_strategy defined, but no ha agents in the index", szIndexName );

	tIdx.ShareHACounters();
}

void FreeAgentsStats ( DistributedIndex_t & tIndex )
{
	ARRAY_FOREACH ( i, tIndex.m_dAgents )
		FreeHostStats ( tIndex.m_dAgents[i] );

	g_tStatsMutex.Lock();
	tIndex.RemoveHACounters();
	g_tStatsMutex.Unlock();
}


void PreCreateTemplateIndex ( ServedDesc_t & tServed, const CSphConfigSection & )
{
	tServed.m_pIndex = sphCreateIndexTemplate ( );
	tServed.m_pIndex->m_bExpandKeywords = tServed.m_bExpand;
	tServed.m_pIndex->m_iExpansionLimit = g_iExpansionLimit;
	tServed.m_bEnabled = false;
}

void PreCreatePlainIndex ( ServedDesc_t & tServed, const char * sName )
{
	tServed.m_pIndex = sphCreateIndexPhrase ( sName, tServed.m_sIndexPath.cstr() );
	tServed.m_pIndex->m_bExpandKeywords = tServed.m_bExpand;
	tServed.m_pIndex->m_iExpansionLimit = g_iExpansionLimit;
	tServed.m_pIndex->SetPreopen ( tServed.m_bPreopen || g_bPreopenIndexes );
	tServed.m_pIndex->SetGlobalIDFPath ( tServed.m_sGlobalIDFPath );
	tServed.m_pIndex->SetMemorySettings ( tServed.m_bMlock, tServed.m_bOnDiskAttrs, tServed.m_bOnDiskPools );
	tServed.m_bEnabled = false;
}


ESphAddIndex AddIndex ( const char * szIndexName, const CSphConfigSection & hIndex )
{
	if ( hIndex("type") && hIndex["type"]=="distributed" )
	{
		///////////////////////////////
		// configure distributed index
		///////////////////////////////

		DistributedIndex_t tIdx;
		ConfigureDistributedIndex ( &tIdx, szIndexName, hIndex );

		// finally, check and add distributed index to global table
		if ( tIdx.m_dAgents.GetLength()==0 && tIdx.m_dLocal.GetLength()==0 )
		{
			FreeAgentsStats ( tIdx );
			sphWarning ( "index '%s': no valid local/remote indexes in distributed index - NOT SERVING", szIndexName );
			return ADD_ERROR;

		} else
		{
			g_tDistLock.Lock ();
			if ( !g_hDistIndexes.Add ( tIdx, szIndexName ) )
			{
				g_tDistLock.Unlock ();
				FreeAgentsStats ( tIdx );
				sphWarning ( "index '%s': duplicate name - NOT SERVING", szIndexName );
				return ADD_ERROR;
			}
			g_tDistLock.Unlock ();
		}

		return ADD_DISTR;

	} else if ( hIndex("type") && hIndex["type"]=="rt" )
	{
		////////////////////////////
		// configure realtime index
		////////////////////////////
		CSphString sError;
		CSphSchema tSchema ( szIndexName );
		if ( !sphRTSchemaConfigure ( hIndex, &tSchema, &sError ) )
		{
			sphWarning ( "index '%s': %s - NOT SERVING", szIndexName, sError.cstr() );
			return ADD_ERROR;
		}

		if ( !sError.IsEmpty() )
			sphWarning ( "index '%s': %s", szIndexName, sError.cstr() );

		// path
		if ( !hIndex("path") )
		{
			sphWarning ( "index '%s': path must be specified - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		// pick config settings
		// they should be overriden later by Preload() if needed
		CSphIndexSettings tSettings;
		if ( !sphConfIndex ( hIndex, tSettings, sError ) )
		{
			sphWarning ( "ERROR: index '%s': %s - NOT SERVING", szIndexName, sError.cstr() );
			return ADD_ERROR;
		}

		int iIndexSP = hIndex.GetInt ( "index_sp" );
		const char * sIndexZones = hIndex.GetStr ( "index_zones", "" );
		bool bHasStripEnabled ( hIndex.GetInt ( "html_strip" )!=0 );
		if ( ( iIndexSP!=0 || ( *sIndexZones ) ) && !bHasStripEnabled )
		{
			// SENTENCE indexing w\o stripper is valid combination
			if ( *sIndexZones )
			{
				sphWarning ( "ERROR: index '%s': has index_sp=%d, index_zones='%s' but disabled html_strip - NOT SERVING",
					szIndexName, iIndexSP, sIndexZones );
				return ADD_ERROR;
			} else
			{
				sphWarning ( "index '%s': has index_sp=%d but disabled html_strip - PARAGRAPH unavailable",
					szIndexName, iIndexSP );
			}
		}

		// RAM chunk size
		int64_t iRamSize = hIndex.GetSize64 ( "rt_mem_limit", 128*1024*1024 );
		if ( iRamSize<128*1024 )
		{
			sphWarning ( "index '%s': rt_mem_limit extremely low, using 128K instead", szIndexName );
			iRamSize = 128*1024;
		} else if ( iRamSize<8*1024*1024 )
			sphWarning ( "index '%s': rt_mem_limit very low (under 8 MB)", szIndexName );

		// upgrading schema to store field lengths
		if ( tSettings.m_bIndexFieldLens )
			if ( !AddFieldLens ( tSchema, false, sError ) )
				{
					sphWarning ( "index '%s': failed to create field lengths attributes: %s", szIndexName, sError.cstr() );
					return ADD_ERROR;
				}

		// index
		ServedDesc_t tIdx;
		bool bWordDict = strcmp ( hIndex.GetStr ( "dict", "keywords" ), "keywords" )==0;
		if ( !bWordDict )
			sphWarning ( "dict=crc deprecated, use dict=keywords instead" );
		if ( bWordDict && ( tSettings.m_dPrefixFields.GetLength() || tSettings.m_dInfixFields.GetLength() ) )
			sphWarning ( "WARNING: index '%s': prefix_fields and infix_fields has no effect with dict=keywords, ignoring\n", szIndexName );
		tIdx.m_pIndex = sphCreateIndexRT ( tSchema, szIndexName, iRamSize, hIndex["path"].cstr(), bWordDict );
		tIdx.m_bEnabled = false;
		tIdx.m_sIndexPath = hIndex["path"].strval();
		tIdx.m_bRT = true;

		ConfigureLocalIndex ( tIdx, hIndex );
		tIdx.m_pIndex->m_bExpandKeywords = tIdx.m_bExpand;
		tIdx.m_pIndex->m_iExpansionLimit = g_iExpansionLimit;
		tIdx.m_pIndex->SetPreopen ( tIdx.m_bPreopen || g_bPreopenIndexes );
		tIdx.m_pIndex->SetGlobalIDFPath ( tIdx.m_sGlobalIDFPath );
		tIdx.m_pIndex->SetMemorySettings ( tIdx.m_bMlock, tIdx.m_bOnDiskAttrs, tIdx.m_bOnDiskPools );

		tIdx.m_pIndex->Setup ( tSettings );
		tIdx.m_pIndex->SetCacheSize ( g_iMaxCachedDocs, g_iMaxCachedHits );

		CSphIndexStatus tStatus;
		tIdx.m_pIndex->GetStatus ( &tStatus );
		tIdx.m_iMass = CalculateMass ( tStatus );

		// hash it
		if ( !g_pLocalIndexes->Add ( tIdx, szIndexName ) )
		{
			sphWarning ( "INTERNAL ERROR: index '%s': hash add failed - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		// leak pointer, so it's destructor won't delete it
		tIdx.m_pIndex = NULL;

		return ADD_RT;

	} else if ( !hIndex("type") || hIndex["type"]=="plain" )
	{
		/////////////////////////
		// configure local index
		/////////////////////////

		ServedDesc_t tIdx;

		// check path
		if ( !hIndex.Exists ( "path" ) )
		{
			sphWarning ( "index '%s': key 'path' not found - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		// check name
		if ( g_pLocalIndexes->Exists ( szIndexName ) )
		{
			sphWarning ( "index '%s': duplicate name - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		// configure memlocking, star
		ConfigureLocalIndex ( tIdx, hIndex );

		// try to create index
		tIdx.m_sIndexPath = hIndex["path"].strval();
		PreCreatePlainIndex ( tIdx, szIndexName );
		tIdx.m_pIndex->SetCacheSize ( g_iMaxCachedDocs, g_iMaxCachedHits );
		CSphIndexStatus tStatus;
		tIdx.m_pIndex->GetStatus ( &tStatus );
		tIdx.m_iMass = CalculateMass ( tStatus );

		// done
		if ( !g_pLocalIndexes->Add ( tIdx, szIndexName ) )
		{
			sphWarning ( "INTERNAL ERROR: index '%s': hash add failed - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		// leak pointer, so it's destructor won't delete it
		tIdx.m_pIndex = NULL;

		return ADD_LOCAL;

	} else if ( hIndex["type"]=="template" )
	{
		/////////////////////////
		// configure template index - just tokenizer and common settings
		/////////////////////////

		ServedDesc_t tIdx;

		// check name
		if ( g_pTemplateIndexes->Exists ( szIndexName ) )
		{
			sphWarning ( "index '%s': duplicate name - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		// configure memlocking, star
		ConfigureTemplateIndex ( tIdx, hIndex );

		// try to create index
		PreCreateTemplateIndex ( tIdx, hIndex );
		tIdx.m_bEnabled = true;

		CSphIndexSettings s;
		CSphString sError;
		if ( !sphConfIndex ( hIndex, s, sError ) )
		{
			sphWarning ( "failed to configure index %s: %s", szIndexName, sError.cstr() );
			return ADD_ERROR;
		}
		tIdx.m_pIndex->Setup(s);

		if ( !sphFixupIndexSettings ( tIdx.m_pIndex, hIndex, sError ) )
		{
			sphWarning ( "index '%s': %s - NOT SERVING", szIndexName, sError.cstr() );
			return ADD_ERROR;
		}

		CSphIndexStatus tStatus;
		tIdx.m_pIndex->GetStatus ( &tStatus );
		tIdx.m_iMass = CalculateMass ( tStatus );

		// done
		if ( !g_pTemplateIndexes->Add ( tIdx, szIndexName ) )
		{
			sphWarning ( "INTERNAL ERROR: index '%s': hash add failed - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		// leak pointer, so it's destructor won't delete it
		tIdx.m_pIndex = NULL;

		return ADD_TMPL;

	} else
	{
		sphWarning ( "index '%s': unknown type '%s' - NOT SERVING", szIndexName, hIndex["type"].cstr() );
		return ADD_ERROR;
	}
}


bool CheckConfigChanges ()
{
	struct stat tStat;
	memset ( &tStat, 0, sizeof ( tStat ) );
	if ( stat ( g_sConfigFile.cstr (), &tStat ) < 0 )
		memset ( &tStat, 0, sizeof ( tStat ) );

	DWORD uCRC32 = 0;

#if !USE_WINDOWS
	char sBuf [ 8192 ];
	FILE * fp = NULL;

	fp = fopen ( g_sConfigFile.cstr (), "rb" );
	if ( !fp )
		return true;
	bool bGotLine = fgets ( sBuf, sizeof(sBuf), fp );
	fclose ( fp );
	if ( !bGotLine )
		return true;

	char * p = sBuf;
	while ( isspace(*p) )
		p++;
	if ( p[0]=='#' && p[1]=='!' )
	{
		p += 2;

		CSphVector<char> dContent;
		char sError [ 1024 ];
		if ( !TryToExec ( p, g_sConfigFile.cstr(), dContent, sError, sizeof(sError) ) )
			return true;

		uCRC32 = sphCRC32 ( dContent.Begin(), dContent.GetLength() );
	} else
		sphCalcFileCRC32 ( g_sConfigFile.cstr (), uCRC32 );
#else
	sphCalcFileCRC32 ( g_sConfigFile.cstr (), uCRC32 );
#endif

	if ( g_uCfgCRC32==uCRC32 && tStat.st_size==g_tCfgStat.st_size
		&& tStat.st_mtime==g_tCfgStat.st_mtime && tStat.st_ctime==g_tCfgStat.st_ctime )
			return false;

	g_uCfgCRC32 = uCRC32;
	g_tCfgStat = tStat;

	return true;
}

void InitPersistentPool()
{
	if ( g_iPersistentPoolSize )
	{
		// always close all persistent connections before (re)calculation.
		ClosePersistentSockets();

		// Set global pools for every uniq persistent host (addr:port or socket).
		// 1-st host pooled at g_dPersistentConnections[0],
		// 2-nd at g_dPersistentConnections[iStride],
		// n-th at g_dPersistentConnections[iStride*(n-1)].(iStride==g_iPersistentPoolSize)
		CSphOrderedHash < int, int, IdentityHash_fn, STATS_MAX_DASH > hPersCounter;
		int iPoolSize = 0;
		int iStride = g_iPersistentPoolSize + 1; // 1 int for # of free item, then the data
		g_hDistIndexes.IterateStart ();
		while ( g_hDistIndexes.IterateNext () )
		{
			DistributedIndex_t & tIndex = g_hDistIndexes.IterateGet ();
			if ( tIndex.m_dAgents.GetLength() )
				ARRAY_FOREACH ( i, tIndex.m_dAgents )
				ARRAY_FOREACH ( j, tIndex.m_dAgents[i] )
				if ( tIndex.m_dAgents[i].GetAgent(j)->m_bPersistent )
				{
					AgentDesc_c* pAgent = tIndex.m_dAgents[i].GetAgent(j);
					if ( hPersCounter.Exists ( pAgent->m_iDashIndex ) )
						// host already met. Copy existing offset
						pAgent->m_dPersPool.Init ( hPersCounter[pAgent->m_iDashIndex] );
					else
					{
						// New host. Allocate new stride.
						pAgent->m_dPersPool.Init ( iPoolSize );
						hPersCounter.Add ( iPoolSize, pAgent->m_iDashIndex );
						iPoolSize += iStride;
					}
				}
		}
		ResizePersistentConnectionsPool ( iPoolSize );
	}
}

static void ReloadIndexSettings ( CSphConfigParser & tCP )
{
	if ( !tCP.ReParse ( g_sConfigFile.cstr () ) )
	{
		sphWarning ( "failed to parse config file '%s'; using previous settings", g_sConfigFile.cstr () );
		return;
	}

	g_bDoDelete = false;

	for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
		it.Get().m_bToDelete = true; ///< FIXME! What about write lock before doing this?

	g_hDistIndexes.IterateStart ();
	while ( g_hDistIndexes.IterateNext () )
		g_hDistIndexes.IterateGet().m_bToDelete = true;

	for ( IndexHashIterator_c it ( g_pTemplateIndexes ); it.Next(); )
		it.Get().m_bToDelete = true;

	int nTotalIndexes = g_pLocalIndexes->GetLength () + g_hDistIndexes.GetLength () + g_pTemplateIndexes->GetLength ();
	int nChecked = 0;

	const CSphConfig & hConf = tCP.m_tConf;
	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext() )
	{
		const CSphConfigSection & hIndex = hConf["index"].IterateGet();
		const char * sIndexName = hConf["index"].IterateGetKey().cstr();

		if ( ServedIndex_c * pServedIndex = g_pLocalIndexes->GetWlockedEntry ( sIndexName ) )
		{
			ConfigureLocalIndex ( *pServedIndex, hIndex );
			if ( hIndex["path"].strval()!=pServedIndex->m_sIndexPath )
				pServedIndex->m_sNewPath = hIndex["path"].strval();
			pServedIndex->m_bToDelete = false;
			nChecked++;
			pServedIndex->Unlock();

		} else if ( g_hDistIndexes.Exists ( sIndexName ) && hIndex.Exists("type") && hIndex["type"]=="distributed" )
		{
			DistributedIndex_t tIdx;
			ConfigureDistributedIndex ( &tIdx, sIndexName, hIndex );

			// finally, check and add distributed index to global table
			if ( tIdx.m_dAgents.GetLength()==0 && tIdx.m_dLocal.GetLength()==0 )
			{
				sphWarning ( "index '%s': no valid local/remote indexes in distributed index; using last valid definition", sIndexName );
				g_hDistIndexes[sIndexName].m_bToDelete = false;

			} else
			{
				g_tDistLock.Lock();
				FreeAgentsStats ( g_hDistIndexes[sIndexName] );
				g_hDistIndexes[sIndexName] = tIdx;
				g_tDistLock.Unlock();
			}

			nChecked++;

		} else if ( ServedIndex_c * pTemplateIndex = g_pTemplateIndexes->GetWlockedEntry ( sIndexName ) )
		{
			ConfigureTemplateIndex ( *pTemplateIndex, hIndex );
			pTemplateIndex->m_bToDelete = false;
			nChecked++;
			pTemplateIndex->Unlock();

		} else // add new index
		{
			ESphAddIndex eType = AddIndex ( sIndexName, hIndex );
			if ( eType==ADD_LOCAL )
			{
				if ( ServedIndex_c * pIndex = g_pLocalIndexes->GetWlockedEntry ( sIndexName ) )
				{
					pIndex->m_bOnlyNew = true;
					pIndex->Unlock();
				}
			} else if ( eType==ADD_RT )
			{
				ServedIndex_c & tIndex = g_pLocalIndexes->GetUnlockedEntry ( sIndexName );

				tIndex.m_bOnlyNew = false;
				if ( PrereadNewIndex ( tIndex, hIndex, sIndexName ) )
					tIndex.m_bEnabled = true;
			}
		}
	}

	InitPersistentPool();

	if ( nChecked < nTotalIndexes )
		g_bDoDelete = true;
}


void CheckDelete ()
{
	if ( !g_bDoDelete )
		return;

	CSphVector<const CSphString *> dToDelete;
	CSphVector<const CSphString *> dTmplToDelete;
	CSphVector<const CSphString *> dDistToDelete;
	dToDelete.Reserve ( 8 );
	dTmplToDelete.Reserve ( 8 );
	dDistToDelete.Reserve ( 8 );

	for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
	{
		ServedIndex_c & tIndex = it.Get();
		if ( tIndex.m_bToDelete )
			dToDelete.Add ( &it.GetKey() );
	}

	for ( IndexHashIterator_c it ( g_pTemplateIndexes ); it.Next(); )
	{
		ServedIndex_c & tIndex = it.Get();
		if ( tIndex.m_bToDelete )
			dTmplToDelete.Add ( &it.GetKey() );
	}


	g_hDistIndexes.IterateStart ();
	while ( g_hDistIndexes.IterateNext () )
	{
		DistributedIndex_t & tIndex = g_hDistIndexes.IterateGet ();
		if ( tIndex.m_bToDelete )
			dDistToDelete.Add ( &g_hDistIndexes.IterateGetKey () );
	}

	ARRAY_FOREACH ( i, dToDelete )
		g_pLocalIndexes->Delete ( *dToDelete[i] ); // should result in automatic CSphIndex::Unlock() via dtor call

	ARRAY_FOREACH ( i, dTmplToDelete )
		g_pTemplateIndexes->Delete ( *dTmplToDelete[i] ); // should result in automatic CSphIndex::Unlock() via dtor call

	g_tDistLock.Lock();

	ARRAY_FOREACH ( i, dDistToDelete )
	{
		FreeAgentsStats ( g_hDistIndexes [ *dDistToDelete[i] ] );
		g_hDistIndexes.Delete ( *dDistToDelete[i] );
	}

	g_tDistLock.Unlock();

	g_bDoDelete = false;
}


void CheckRotateGlobalIDFs ()
{
	CSphVector <CSphString> dFiles;
	for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
	{
		ServedIndex_c & tIndex = it.Get();
		if ( tIndex.m_bEnabled && !tIndex.m_sGlobalIDFPath.IsEmpty() )
			dFiles.Add ( tIndex.m_sGlobalIDFPath );
	}
	sphUpdateGlobalIDFs ( dFiles );
}


void RotationServiceThreadFunc ( void * )
{
	while ( !g_bShutdown )
	{
		if ( g_bInvokeRotationService )
		{
			CheckRotateGlobalIDFs ();
			g_bInvokeRotationService = false;
		}
		sphSleepMsec ( 50 );
	}
}


void CheckRotate ()
{
	// do we need to rotate now?
	if ( !g_bNeedRotate || g_bInRotate )
		return;

	g_bInRotate = true; // ok, another rotation cycle just started
	g_bNeedRotate = false; // which therefore clears any previous HUP signals

	sphLogDebug ( "CheckRotate invoked" );

	/////////////////////
	// RAM-greedy rotate
	/////////////////////

	if ( !g_bSeamlessRotate )
	{
		if ( CheckConfigChanges () )
		{
			ReloadIndexSettings ( g_pCfg );
		}

		for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
		{
			ServedIndex_c & tIndex = it.Get();
			tIndex.WriteLock();
			const char * sIndex = it.GetKey().cstr();
			assert ( tIndex.m_pIndex );

			if ( tIndex.m_bRT )
			{
				tIndex.Unlock();
				continue;
			}

			bool bWasAdded = tIndex.m_bOnlyNew;
			CSphString sError;
			if ( !RotateIndexGreedy ( tIndex, sIndex, sError ) )
				sphWarning ( "%s", sError.cstr() );
			if ( bWasAdded && tIndex.m_bEnabled )
			{
				const CSphConfigType & hConf = g_pCfg.m_tConf ["index"];
				if ( hConf.Exists ( sIndex ) )
				{
					CSphString sError;
					if ( !sphFixupIndexSettings ( tIndex.m_pIndex, hConf [sIndex], sError ) )
					{
						sphWarning ( "index '%s': %s - NOT SERVING", sIndex, sError.cstr() );
						tIndex.m_bEnabled = false;
					}
				}

				if ( tIndex.m_bEnabled )
					tIndex.m_pIndex->Preread();
			}
			tIndex.Unlock();
		}

		g_bInRotate = false;
		g_bInvokeRotationService = true;
		sphInfo ( "rotating finished" );
		return;
	}

	///////////////////
	// seamless rotate
	///////////////////

	assert ( g_dRotateQueue.GetLength()==0 );

	g_tRotateConfigMutex.Lock();
	if ( CheckConfigChanges() )
	{
		ReloadIndexSettings ( g_pCfg );
	}
	g_tRotateConfigMutex.Unlock();

	// check what indexes need to be rotated
	CSphVector<CSphString> dQueue;
	for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
	{
		const ServedIndex_c & tIndex = it.Get();
		const CSphString & sIndex = it.GetKey();
		assert ( tIndex.m_pIndex );

		if ( CheckIndexHeaderRotate ( tIndex.m_sIndexPath.cstr(), tIndex.m_sNewPath.cstr(), tIndex.m_bOnlyNew )==ROTATE_FROM_NONE )
		{
			sphLogDebug ( "%s.new.sph is not readable. Skipping", tIndex.m_sIndexPath.cstr() );
			continue;
		}

		dQueue.Add ( sIndex );
	}

	if ( !dQueue.GetLength() )
	{
		sphWarning ( "nothing to rotate after SIGHUP" );
		g_bInvokeRotationService = false;
		g_bInRotate = false;
		return;
	}

	g_tRotateQueueMutex.Lock();
	g_dRotateQueue.SwapData ( dQueue );
	g_tRotateQueueMutex.Unlock();

	if ( dQueue.GetLength() )
	{
		sphWarning ( "INTERNAL ERROR: non-empty queue on a rotation cycle start, got %d elements", dQueue.GetLength() );
		ARRAY_FOREACH ( i, g_dRotateQueue )
			sphWarning ( "queue[%d] = %s", i, dQueue[i].cstr() );
	}
}


void CheckReopen ()
{
	if ( !g_bGotSigusr1 )
		return;

	// reopen searchd log
	if ( g_iLogFile>=0 && !g_bLogTty )
	{
		int iFD = ::open ( g_sLogFile.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
		if ( iFD<0 )
		{
			sphWarning ( "failed to reopen log file '%s': %s", g_sLogFile.cstr(), strerror(errno) );
		} else
		{
			::close ( g_iLogFile );
			g_iLogFile = iFD;
			g_bLogTty = ( isatty ( g_iLogFile )!=0 );
			sphInfo ( "log reopened" );
		}
	}

	// reopen query log
	if ( !g_bQuerySyslog && g_iQueryLogFile!=g_iLogFile && g_iQueryLogFile>=0 && !isatty ( g_iQueryLogFile ) )
	{
		int iFD = ::open ( g_sQueryLogFile.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
		if ( iFD<0 )
		{
			sphWarning ( "failed to reopen query log file '%s': %s", g_sQueryLogFile.cstr(), strerror(errno) );
		} else
		{
			::close ( g_iQueryLogFile );
			g_iQueryLogFile = iFD;
			sphInfo ( "query log reopened" );
		}
	}

	g_bGotSigusr1 = 0;
}


static void ThdSaveIndexes ( void * )
{
	SaveIndexes ();

	// we're no more flushing
	g_tFlush.m_bFlushing = false;
}

void CheckFlush ()
{
	if ( g_tFlush.m_bFlushing )
		return;

	// do a periodic check, unless we have a forced check
	if ( !g_tFlush.m_bForceCheck )
	{
		static int64_t tmLastCheck = -1000;
		int64_t tmNow = sphMicroTimer();

		if ( !g_iAttrFlushPeriod || ( tmLastCheck + int64_t(g_iAttrFlushPeriod)*I64C(1000000) )>=tmNow )
			return;

		tmLastCheck = tmNow;
		sphLogDebug ( "attrflush: doing periodic check" );
	} else
	{
		sphLogDebug ( "attrflush: doing forced check" );
	}

	// check if there are dirty indexes
	bool bDirty = false;
	for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
	{
		const ServedIndex_c & tServed = it.Get();
		if ( tServed.m_bEnabled && tServed.m_pIndex->GetAttributeStatus() )
		{
			bDirty = true;
			break;
		}
	}

	// need to set this before clearing check flag
	if ( bDirty )
		g_tFlush.m_bFlushing = true;

	// if there was a forced check in progress, it no longer is
	if ( g_tFlush.m_bForceCheck )
		g_tFlush.m_bForceCheck = false;

	// nothing to do, no indexes were updated
	if ( !bDirty )
	{
		sphLogDebug ( "attrflush: no dirty indexes found" );
		return;
	}

	// launch the flush!
	g_tFlush.m_iFlushTag++;

	sphLogDebug ( "attrflush: starting writer, tag ( %d )", g_tFlush.m_iFlushTag );

	ThdDesc_t tThd;
	if ( !sphThreadCreate ( &tThd.m_tThd, ThdSaveIndexes, NULL, true ) )
		sphWarning ( "failed to create attribute save thread, error[%d] %s", errno, strerror(errno) );
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
	snprintf ( sBuf, sizeof(sBuf), "code=%d, error=", uErr );

	int iLen = strlen(sBuf);
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

	int iArgLen = strlen(sArg);
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
	sDesc.SetSprintf ( "%s-%s", g_sServiceName, SPHINX_VERSION );

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
		"-c, --config <file>\tread configuration from specified file\n"
		"\t\t\t(default is sphinx.conf)\n"
		"--stop\t\t\tsend SIGTERM to currently running searchd\n"
		"--stopwait\t\tsend SIGTERM and wait until actual exit\n"
		"--status\t\tget ant print status variables\n"
		"\t\t\t(PID is taken from pid_file specified in config file)\n"
		"--iostats\t\tlog per-query io stats\n"
#ifdef HAVE_CLOCK_GETTIME
		"--cpustats\t\tlog per-query cpu stats\n"
#endif
#if USE_WINDOWS
		"--install\t\tinstall as Windows service\n"
		"--delete\t\tdelete Windows service\n"
		"--servicename <name>\tuse given service name (default is 'searchd')\n"
		"--ntservice\t\tinternal option used to invoke a Windows service\n"
#endif
		"--strip-path\t\tstrip paths from stopwords, wordforms, exceptions\n"
		"\t\t\tand other file names stored in the index header\n"
		"--replay-flags=<OPTIONS>\n"
		"\t\t\textra binary log replay options (the only current one\n"
		"\t\t\tis 'accept-desc-timestamp')\n"
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
		"\n"
		"Examples:\n"
		"searchd --config /usr/local/sphinx/etc/sphinx.conf\n"
#if USE_WINDOWS
		"searchd --install --config c:\\sphinx\\sphinx.conf\n"
#endif
		);
}


void InitSharedBuffer ( CSphLargeBuffer<DWORD, true> & tBuffer )
{
	CSphString sError, sWarning;
	if ( !tBuffer.Alloc ( 1, sError ) )
		sphDie ( "failed to allocate shared buffer (msg=%s)", sError.cstr() );

	DWORD * pRes = tBuffer.GetWritePtr();
	memset ( pRes, 0, sizeof(DWORD) ); // reset
}


#if USE_WINDOWS
BOOL WINAPI CtrlHandler ( DWORD )
{
	if ( !g_bService )
	{
		g_bGotSigterm = 1;
		sphInterruptNow();
	}
	return TRUE;
}
#endif


#if !USE_WINDOWS
// returns 'true' only once - at the very start, to show it beatiful way.
bool SetWatchDog ( int iDevNull )
{
	InitSharedBuffer ( g_bDaemonAtShutdown );
	InitSharedBuffer ( g_bHaveTTY );

	// Fork #1 - detach from controlling terminal
	switch ( fork() )
	{
		case -1:
			// error
			sphFatalLog ( "fork() failed (reason: %s)", strerror ( errno ) );
			exit ( 1 );
		case 0:
			// daemonized child - or new and free watchdog :)
			break;

		default:
			// tty-controlled parent
			while ( g_bHaveTTY[0]==0 )
				sphSleepMsec ( 100 );

			sphSetProcessInfo ( false );
			exit ( 0 );
	}

	// became the session leader
	if ( setsid()==-1 )
	{
		sphFatalLog ( "setsid() failed (reason: %s)", strerror ( errno ) );
		exit ( 1 );
	}

	// Fork #2 - detach from session leadership (may be not necessary, however)
	switch ( fork() )
	{
		case -1:
			// error
			sphFatalLog ( "fork() failed (reason: %s)", strerror ( errno ) );
			exit ( 1 );
		case 0:
			// daemonized child - or new and free watchdog :)
			break;

		default:
			// tty-controlled parent
			sphSetProcessInfo ( false );
			exit ( 0 );
	}

	// now we are the watchdog. Let us fork the actual process
	int iReincarnate = 1;
	bool bShutdown = false;
	bool bStreamsActive = true;
	int iRes = 0;
	for ( ;; )
	{
		if ( iReincarnate!=0 )
			iRes = fork();

		if ( iRes==-1 )
		{
			sphFatalLog ( "fork() failed during watchdog setup (error=%s)", strerror(errno) );
			exit ( 1 );
		}

		// child process; return true to show that we have to reload everything
		if ( iRes==0 )
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

		sphInfo ( "watchdog: main process %d forked ok", iRes );

		SetSignalHandlers();

		iReincarnate = 0;
		int iPid, iStatus;
		bool bDaemonAtShutdown = 0;
		while ( ( iPid = wait ( &iStatus ) )>0 )
		{
			bDaemonAtShutdown = ( g_bDaemonAtShutdown[0]!=0 );
			const char * sWillRestart = ( bDaemonAtShutdown ? "will not be restarted (daemon is shutting down)" : "will be restarted" );

			assert ( iPid==iRes );
			if ( WIFEXITED ( iStatus ) )
			{
				int iExit = WEXITSTATUS ( iStatus );
				if ( iExit==2 || iExit==6 ) // really crash
				{
					sphInfo ( "watchdog: main process %d crashed via CRASH_EXIT (exit code %d), %s", iPid, iExit, sWillRestart );
					iReincarnate = -1;
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
					iReincarnate = -1;
				}
			} else if ( WIFSTOPPED ( iStatus ) )
				sphInfo ( "watchdog: main process %d stopped with signal %d", iPid, WSTOPSIG ( iStatus ) );
#ifdef WIFCONTINUED
			else if ( WIFCONTINUED ( iStatus ) )
				sphInfo ( "watchdog: main process %d resumed", iPid );
#endif
		}

		if ( bShutdown || g_bGotSigterm || bDaemonAtShutdown )
		{
			exit ( 0 );
		}
	}
}
#endif // !USE_WINDOWS

/// check for incoming signals, and react on them
void CheckSignals ()
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

	if ( g_bGotSigterm )
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
				g_bGotSigterm = 1;
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


void QueryStatus ( CSphVariant * v )
{
	char sBuf [ SPH_ADDRESS_SIZE ];
	char sListen [ 256 ];
	CSphVariant tListen;

	if ( !v )
	{
		snprintf ( sListen, sizeof ( sListen ), "127.0.0.1:%d:sphinx", SPHINXAPI_PORT );
		tListen = CSphVariant ( sListen, 0 );
		v = &tListen;
	}

	for ( ; v; v = v->m_pNext )
	{
		ListenerDesc_t tDesc = ParseListener ( v->cstr() );
		if ( tDesc.m_eProto!=PROTO_SPHINX )
			continue;

		int iSock = -1;
#if !USE_WINDOWS
		if ( !tDesc.m_sUnix.IsEmpty() )
		{
			// UNIX connection
			struct sockaddr_un uaddr;

			size_t len = strlen ( tDesc.m_sUnix.cstr() );
			if ( len+1 > sizeof(uaddr.sun_path ) )
				sphFatal ( "UNIX socket path is too long (len=%d)", (int)len );

			memset ( &uaddr, 0, sizeof(uaddr) );
			uaddr.sun_family = AF_UNIX;
			memcpy ( uaddr.sun_path, tDesc.m_sUnix.cstr(), len+1 );

			iSock = socket ( AF_UNIX, SOCK_STREAM, 0 );
			if ( iSock<0 )
				sphFatal ( "failed to create UNIX socket: %s", sphSockError() );

			if ( connect ( iSock, (struct sockaddr*)&uaddr, sizeof(uaddr) )<0 )
			{
				sphWarning ( "failed to connect to unix://%s: %s\n", tDesc.m_sUnix.cstr(), sphSockError() );
				continue;
			}

		} else
#endif
		{
			// TCP connection
			struct sockaddr_in sin;
			memset ( &sin, 0, sizeof(sin) );
			sin.sin_family = AF_INET;
			sin.sin_addr.s_addr = ( tDesc.m_uIP==htonl ( INADDR_ANY ) )
				? htonl ( INADDR_LOOPBACK )
				: tDesc.m_uIP;
			sin.sin_port = htons ( (short)tDesc.m_iPort );

			iSock = socket ( AF_INET, SOCK_STREAM, 0 );
			if ( iSock<0 )
				sphFatal ( "failed to create TCP socket: %s", sphSockError() );

#ifdef TCP_NODELAY
			int iOn = 1;
			if ( setsockopt ( iSock, IPPROTO_TCP, TCP_NODELAY, (char*)&iOn, sizeof(iOn) ) )
				sphWarning ( "setsockopt() failed: %s", sphSockError() );
#endif

			if ( connect ( iSock, (struct sockaddr*)&sin, sizeof(sin) )<0 )
			{
				sphWarning ( "failed to connect to %s:%d: %s\n", sphFormatIP ( sBuf, sizeof(sBuf), tDesc.m_uIP ), tDesc.m_iPort, sphSockError() );
				continue;
			}
		}

		// send request
		NetOutputBuffer_c tOut ( iSock );
		tOut.SendDword ( SPHINX_CLIENT_VERSION );
		tOut.SendWord ( SEARCHD_COMMAND_STATUS );
		tOut.SendWord ( VER_COMMAND_STATUS );
		tOut.SendInt ( 4 ); // request body length
		tOut.SendInt ( 1 ); // dummy body
		tOut.Flush ();

		// get reply
		NetInputBuffer_c tIn ( iSock );
		if ( !tIn.ReadFrom ( 12, 5 ) ) // magic_header_size=12, magic_timeout=5
			sphFatal ( "handshake failure (no response)" );

		DWORD uVer = tIn.GetDword();
		if ( uVer!=SPHINX_SEARCHD_PROTO && uVer!=0x01000000UL ) // workaround for all the revisions that sent it in host order...
			sphFatal ( "handshake failure (unexpected protocol version=%d)", uVer );

		if ( tIn.GetWord()!=SEARCHD_OK )
			sphFatal ( "status command failed" );

		if ( tIn.GetWord()!=VER_COMMAND_STATUS )
			sphFatal ( "status command version mismatch" );

		if ( !tIn.ReadFrom ( tIn.GetDword(), 5 ) ) // magic_timeout=5
			sphFatal ( "failed to read status reply" );

		fprintf ( stdout, "\nsearchd status\n--------------\n" );

		int iRows = tIn.GetDword();
		int iCols = tIn.GetDword();
		for ( int i=0; i<iRows && !tIn.GetError(); i++ )
		{
			for ( int j=0; j<iCols && !tIn.GetError(); j++ )
			{
				fprintf ( stdout, "%s", tIn.GetString().cstr() );
				fprintf ( stdout, ( j==0 ) ? ": " : " " );
			}
			fprintf ( stdout, "\n" );
		}

		// all done
		sphSockClose ( iSock );
		return;
	}
	sphFatal ( "failed to connect to daemon: please specify listen with sphinx protocol in your config file" );
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


void FailClient ( int iSock, SearchdStatus_e eStatus, const char * sMessage )
{
	assert ( eStatus==SEARCHD_RETRY || eStatus==SEARCHD_ERROR );

	int iRespLen = 4 + strlen(sMessage);

	NetOutputBuffer_c tOut ( iSock );
	tOut.SendInt ( SPHINX_CLIENT_VERSION );
	tOut.SendWord ( (WORD)eStatus );
	tOut.SendWord ( 0 ); // version doesn't matter
	tOut.SendInt ( iRespLen );
	tOut.SendString ( sMessage );
	tOut.Flush ();

	// FIXME? without some wait, client fails to receive the response on windows
	sphSockClose ( iSock );
}

static const char * g_dSphinxQLMaxedOutPacket = "\x17\x00\x00\x00\xff\x10\x04Too many connections";
static const int g_iSphinxQLMaxedOutLen = 27;


void MysqlMaxedOut ( int iSock )
{
	sphSockSend ( iSock, g_dSphinxQLMaxedOutPacket, g_iSphinxQLMaxedOutLen );
	sphSockClose ( iSock );
}


Listener_t * DoAccept ( int * pClientSock, char * sClientName )
{
	assert ( pClientSock );
	assert ( *pClientSock==-1 );

	int iMaxFD = 0;
	fd_set fdsAccept;
	FD_ZERO ( &fdsAccept );

	ARRAY_FOREACH ( i, g_dListeners )
	{
		sphFDSet ( g_dListeners[i].m_iSock, &fdsAccept );
		iMaxFD = Max ( iMaxFD, g_dListeners[i].m_iSock );
	}
	iMaxFD++;

	struct timeval tvTimeout;
	tvTimeout.tv_sec = USE_WINDOWS ? 0 : 1;
	tvTimeout.tv_usec = USE_WINDOWS ? 50000 : 0;

	// select should be OK here as listener sockets are created early and get low FDs
	int iRes = ::select ( iMaxFD, &fdsAccept, NULL, NULL, &tvTimeout );
	if ( iRes==0 )
		return NULL;

	if ( iRes<0 )
	{
		int iErrno = sphSockGetErrno();
		if ( iErrno==EINTR || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
			return NULL;

		static int iLastErrno = -1;
		if ( iLastErrno!=iErrno )
			sphWarning ( "select() failed: %s", sphSockError(iErrno) );
		iLastErrno = iErrno;
		return NULL;
	}

	ARRAY_FOREACH ( i, g_dListeners )
	{
		if ( !FD_ISSET ( g_dListeners[i].m_iSock, &fdsAccept ) )
			continue;

		// accept
		struct sockaddr_storage saStorage;
		socklen_t uLength = sizeof(saStorage);
		int iClientSock = accept ( g_dListeners[i].m_iSock, (struct sockaddr *)&saStorage, &uLength );

		// handle failures
		if ( iClientSock<0 )
		{
			const int iErrno = sphSockGetErrno();
			if ( iErrno==EINTR || iErrno==ECONNABORTED || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
				return NULL;

			sphFatal ( "accept() failed: %s", sphSockError(iErrno) );
		}

#ifdef TCP_NODELAY
		int iOn = 1;
		if ( g_dListeners[i].m_bTcp && setsockopt ( iClientSock, IPPROTO_TCP, TCP_NODELAY, (char*)&iOn, sizeof(iOn) ) )
			sphWarning ( "setsockopt() failed: %s", sphSockError() );
#endif

		g_tStatsMutex.Lock();
		g_tStats.m_iConnections++;
		g_tStatsMutex.Unlock();

		if ( ++g_iConnectionID<0 )
			g_iConnectionID = 0;

		// format client address
		if ( sClientName )
		{
			sClientName[0] = '\0';
			if ( saStorage.ss_family==AF_INET )
			{
				struct sockaddr_in * pSa = ((struct sockaddr_in *)&saStorage);
				sphFormatIP ( sClientName, SPH_ADDRESS_SIZE, pSa->sin_addr.s_addr );

				char * d = sClientName;
				while ( *d )
					d++;
				snprintf ( d, 7, ":%d", (int)ntohs ( pSa->sin_port ) ); //NOLINT
			}
			if ( saStorage.ss_family==AF_UNIX )
				strncpy ( sClientName, "(local)", SPH_ADDRESS_SIZE );
		}

		// accepted!
#if !USE_WINDOWS && !HAVE_POLL
		// when there is no poll(), we use select(),
		// which can only handle a limited range of fds..
		if ( SPH_FDSET_OVERFLOW ( iClientSock ) )
		{
			// otherwise, we fail this client (we have to)
			FailClient ( iClientSock, SEARCHD_RETRY, "server maxed out, retry in a second" );
			sphWarning ( "maxed out, dismissing client (socket=%d)", iClientSock );
			sphSockClose ( iClientSock );
			return NULL;
		}
#endif

		*pClientSock = iClientSock;
		return &g_dListeners[i];
	}

	return NULL;
}


int GetOsThreadId()
{
#if USE_WINDOWS
	return GetCurrentThreadId();
#elif defined(SYS_gettid)
	return syscall ( SYS_gettid );
#elif defined(__FreeBSD__)
	long tid;
	thr_self(&tid);
	return (int)tid;
#else
	return 0;
#endif
}


void HandlerThread ( void * pArg )
{
	// handle that client
	ThdDesc_t * pThd = (ThdDesc_t*) pArg;
	pThd->m_iTid = GetOsThreadId();
	HandleClient ( pThd->m_eProto, pThd->m_iClientSock, pThd->m_sClientName.cstr(), pThd );
	sphSockClose ( pThd->m_iClientSock );

	// done; remove myself from the table
#if USE_WINDOWS
	// FIXME? this is sort of automatic on UNIX (pthread_exit() gets implicitly called on return)
	CloseHandle ( pThd->m_tThd );
#endif
	g_tThdMutex.Lock ();
	g_dThd.Remove ( pThd );
	g_tThdMutex.Unlock ();
	SafeDelete ( pThd );
}


void TickHead ()
{
	CheckSignals ();
	CheckLeaks ();
	CheckReopen ();
	CheckDelete ();
	CheckRotate ();
	CheckFlush ();
	CheckPing();

	if ( g_pThdPool )
	{
		sphInfo ( NULL ); // flush dupes
		sphSleepMsec ( 500 );
		return;
	}

	int iClientSock = -1;
	char sClientName[SPH_ADDRPORT_SIZE];
	Listener_t * pListener = DoAccept ( &iClientSock, sClientName );
	if ( !pListener )
		return;

	if ( ( g_iMaxChildren && !g_pThdPool && g_dThd.GetLength()>=g_iMaxChildren )
		|| ( g_bInRotate && !g_bSeamlessRotate ) )
	{
		if ( pListener->m_eProto==PROTO_SPHINX )
			FailClient ( iClientSock, SEARCHD_RETRY, "server maxed out, retry in a second" );
		else
			MysqlMaxedOut ( iClientSock );
		sphWarning ( "maxed out, dismissing client" );

		g_tStats.m_iMaxedOut++;
		return;
	}

	ThdDesc_t * pThd = new ThdDesc_t ();
	pThd->m_eProto = pListener->m_eProto;
	pThd->m_iClientSock = iClientSock;
	pThd->m_sClientName = sClientName;
	pThd->m_iConnID = g_iConnectionID;
	pThd->m_tmConnect = sphMicroTimer();

	g_tThdMutex.Lock ();
	g_dThd.Add ( pThd );
	g_tThdMutex.Unlock ();

	if ( !SphCrashLogger_c::ThreadCreate ( &pThd->m_tThd, HandlerThread, pThd, true ) )
	{
		sphSockClose ( iClientSock );
		int iErr = errno;
		g_tThdMutex.Lock ();
		g_dThd.Remove ( pThd );
		g_tThdMutex.Unlock ();
		SafeDelete ( pThd );

		FailClient ( iClientSock, SEARCHD_RETRY, "failed to create worker thread" );
		sphWarning ( "failed to create worker thread, threads(%d), error[%d] %s", g_dThd.GetLength(), iErr, strerror(iErr) );
	}
}


/////////////////////////////////////////////////////////////////////////////
// NETWORK THREAD
/////////////////////////////////////////////////////////////////////////////

enum NetEvent_e
{
	NE_KEEP = 0,
	NE_IN = 1UL<<0,
	NE_OUT = 1UL<<1,
	NE_HUP = 1UL<<2,
	NE_ERR = 1UL<<3,
	NE_REMOVE = 1UL<<4,
};

struct NetStateCommon_t;
class CSphNetLoop;
struct ISphNetAction : ISphNoncopyable, public ListNode_t
{
	int64_t				m_tmTimeout;
	const int			m_iSock;

	explicit ISphNetAction ( int iSock ) : m_tmTimeout ( 0 ), m_iSock ( iSock ) {}
	virtual ~ISphNetAction () {}
	virtual NetEvent_e		Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop ) = 0;
	virtual NetEvent_e		Setup ( int64_t tmNow ) = 0;
	virtual bool			GetStats ( int & ) { return false; }
	virtual void			CloseSocket () = 0;
};

struct NetStateCommon_t
{
	int					m_iClientSock;
	int					m_iConnID;
	char				m_sClientName[SPH_ADDRPORT_SIZE];
	bool				m_bKeepSocket;
	bool				m_bVIP;

	CSphVector<BYTE>	m_dBuf;
	int					m_iLeft;
	int					m_iPos;

	NetStateCommon_t ();
	~NetStateCommon_t ();

	void CloseSocket ();
};

struct NetActionAccept_t : public ISphNetAction
{
	Listener_t			m_tListener;
	int					m_iConnections;
	NetStateCommon_t	m_tDummy;

	explicit NetActionAccept_t ( const Listener_t & tListener );

	virtual NetEvent_e		Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop );
	virtual NetEvent_e		Setup ( int64_t tmNow );
	virtual bool			GetStats ( int & iConnections );
	virtual void			CloseSocket () {}

	void					FillNetState ( NetStateCommon_t * pState, int iClientSock, int iConnID, bool bVIP, const sockaddr_storage & saStorage ) const;
};

// just new typedef for API state
typedef NetStateCommon_t NetStateAPI_t;

struct NetStateQL_t : public NetStateCommon_t
{
	CSphinxqlSession	m_tSession;
	bool				m_bAuthed;
	BYTE				m_uPacketID;
	NetStateQL_t ();
};

enum ActionAPI_e
{
	AAPI_HANDSHAKE_OUT = 0,
	AAPI_HANDSHAKE_IN,
	AAPI_COMMAND,
	AAPI_BODY,
	AAPI_TOTAL
};

struct NetReceiveDataAPI_t : public ISphNetAction
{
	CSphScopedPtr<NetStateAPI_t>	m_tState;

	ActionAPI_e			m_ePhase;

	int					m_iCommand;
	int					m_iCommandVer;

	explicit NetReceiveDataAPI_t ( NetStateAPI_t * pState );
	virtual ~NetReceiveDataAPI_t() {}

	virtual NetEvent_e		Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop );
	virtual NetEvent_e		Setup ( int64_t tmNow );
	virtual void			CloseSocket ();

	void				SetupBodyPhase();
	void				AddJobAPI ( CSphNetLoop * pLoop );
};

enum ActionQL_e
{
	AQL_HANDSHAKE = 0,
	AQL_LENGTH,
	AQL_BODY,
	AQL_AUTH,
	AQL_TOTAL
};

struct NetReceiveDataQL_t : public ISphNetAction
{
	CSphScopedPtr<NetStateQL_t>		m_tState;

	ActionQL_e			m_ePhase;

	bool				m_bAppend;
	bool				m_bWrite;

	explicit NetReceiveDataQL_t ( NetStateQL_t * pState );
	virtual ~NetReceiveDataQL_t() {}

	virtual NetEvent_e		Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop );
	virtual NetEvent_e		Setup ( int64_t tmNow );
	virtual void			CloseSocket ();

	void				SetupHandshakePhase();
	void				SetupBodyPhase();
};

struct NetSendData_t : public ISphNetAction
{
	CSphScopedPtr<NetStateCommon_t>		m_tState;
	ProtocolType_e						m_eProto;

	NetSendData_t ( NetStateCommon_t * pState, ProtocolType_e eProto );
	virtual ~NetSendData_t () {}

	virtual NetEvent_e		Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop );
	virtual NetEvent_e		Setup ( int64_t tmNow );
	virtual void			CloseSocket ();
};

struct HttpHeaderStreamParser_t
{
	int m_iHeaderEnd;
	int m_iFieldContentLenStart;
	int m_iFieldContentLenVal;

	int m_iCur;
	int m_iCRLF;
	int m_iName;

	HttpHeaderStreamParser_t ();
	bool HeaderFound ( const BYTE * pBuf, int iLen );
};

struct NetReceiveDataHttp_t : public ISphNetAction
{
	CSphScopedPtr<NetStateQL_t>		m_tState;
	HttpHeaderStreamParser_t		m_tHeadParser;

	explicit NetReceiveDataHttp_t ( NetStateQL_t * pState );
	virtual ~NetReceiveDataHttp_t() {}

	virtual NetEvent_e		Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop );
	virtual NetEvent_e		Setup ( int64_t tmNow );
	virtual void			CloseSocket ();
};

struct EventsIterator_t
{
	ISphNetAction * m_pWork;
	DWORD			m_uEvents;
};


#if HAVE_POLL
class CSphEventsPoll
{
private:
	CSphVector<ISphNetAction *> m_dWork;
	int					m_iIter;
	EventsIterator_t	m_tIter;

	CSphVector<pollfd>	m_dEvents;
	int					m_iLastReportedErrno;
	int					m_iReady;

public:
	CSphEventsPoll ()
		: m_iIter ( -1 )
		, m_iLastReportedErrno ( -1 )
		, m_iReady ( 0 )
	{
		m_dWork.Reserve ( 1000 );
		m_tIter.m_pWork = NULL;
		m_tIter.m_uEvents = 0;
	}

	~CSphEventsPoll()
	{
		ARRAY_FOREACH ( i, m_dWork )
			SafeDelete ( m_dWork[i] );
	}

	void SetupEvent ( ISphNetAction * pWork, int64_t tmNow )
	{
		assert ( pWork && pWork->m_iSock>=0 );

		NetEvent_e eSetup = pWork->Setup ( tmNow );
		assert ( eSetup==NE_IN || eSetup==NE_OUT );

		pollfd tEvent;
		tEvent.fd = pWork->m_iSock;
		tEvent.events = ( eSetup==NE_IN ? POLLIN : POLLOUT );

		m_dEvents.Add ( tEvent );
		m_dWork.Add ( pWork );
	}

	bool Wait ( int timeoutMs )
	{
		// need positive timeout for communicate threads back and shutdown
		m_iReady = ::poll ( m_dEvents.Begin(), m_dEvents.GetLength(), timeoutMs );

		if ( m_iReady<0 )
		{
			int iErrno = sphSockGetErrno();
			// common recoverable errors
			if ( iErrno==EINTR || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
				return false;

			if ( m_iLastReportedErrno!=iErrno )
			{
				sphWarning ( "poll tick failed: %s", sphSockError(iErrno) );
				m_iLastReportedErrno = iErrno;
			}
			return false;
		}

		return ( m_iReady>0 );
	}

	bool IterateNextAll ()
	{
		assert ( m_dEvents.GetLength()==m_dWork.GetLength() );

		m_iIter++;
		m_tIter.m_pWork = ( m_iIter<m_dWork.GetLength() ? m_dWork[m_iIter] : NULL );

		return ( m_iIter<m_dWork.GetLength() );
	}

	bool IterateNextReady ()
	{
		m_tIter.m_pWork = NULL;
		m_tIter.m_uEvents = 0;

		if ( m_iReady<=0 || m_iIter>=m_dEvents.GetLength() )
			return false;

		for ( ;; )
		{
			m_iIter++;
			if ( m_iIter>=m_dEvents.GetLength() )
				return false;

			if ( m_dEvents[m_iIter].revents==0 )
				continue;

			m_iReady--;

			m_tIter.m_pWork = m_dWork[m_iIter];
			m_tIter.m_uEvents = 0;

			pollfd & tEv = m_dEvents[m_iIter];
			if ( tEv.revents & POLLIN )
				m_tIter.m_uEvents |= NE_IN;
			if ( tEv.revents & POLLOUT )
				m_tIter.m_uEvents |= NE_OUT;
			if ( tEv.revents & POLLHUP )
				m_tIter.m_uEvents |= NE_HUP;
			if ( tEv.revents & POLLERR )
				m_tIter.m_uEvents |= NE_ERR;

			tEv.revents = 0;

			return true;
		}
	}

	void IterateChangeEvent ( NetEvent_e eEv, ISphNetAction * )
	{
		assert ( m_iIter>=0 && m_iIter<m_dEvents.GetLength() );
		m_dEvents[m_iIter].events = ( ( eEv & NE_IN )!=0 ? POLLIN : POLLOUT );
	}

	void IterateRemove ()
	{
		assert ( m_iIter>=0 && m_iIter<m_dEvents.GetLength() );
		assert ( m_dEvents.GetLength()==m_dWork.GetLength() );

		m_dEvents.RemoveFast ( m_iIter );
		// SafeDelete ( m_dWork[m_iIter] );
		m_dWork.RemoveFast ( m_iIter );

		m_iIter--;
		m_tIter.m_pWork = NULL;
	}

	void IterateStart ()
	{
		m_iIter = -1;
		m_tIter.m_pWork = NULL;
		m_tIter.m_uEvents = 0;
	}

	EventsIterator_t & IterateGet ()
	{
		assert ( m_iIter>=0 && m_iIter<m_dWork.GetLength() );
		return m_tIter;
	}
};
#endif

#if HAVE_EPOLL

class CSphEventsEpoll
{
private:
	List_t						m_tWork;
	CSphVector<epoll_event>		m_dReady;
	int							m_iLastReportedErrno;
	int							m_iReady;
	int							m_iEFD;
	EventsIterator_t			m_tIter;
	int							m_iIterEv;

public:
	CSphEventsEpoll ()
		: m_iLastReportedErrno ( -1 )
		, m_iReady ( 0 )
	{
		m_iEFD = epoll_create ( 1000 );
		if ( m_iEFD==-1 )
			sphDie ( "failed to create epoll main FD, errno=%d, %s", errno, strerror(errno) );

		m_dReady.Reserve ( 1000 );
		m_tIter.m_pWork = NULL;
		m_tIter.m_uEvents = 0;
		m_iIterEv = -1;
	}

	~CSphEventsEpoll()
	{
		const ListNode_t * pIt = m_tWork.Begin();
		while ( pIt!=m_tWork.End() )
		{
			ISphNetAction * pWork = (ISphNetAction *)pIt;
			pIt = pIt->m_pNext;
			SafeDelete ( pWork );
		}

		SafeClose ( m_iEFD );
	}

	void SetupEvent ( ISphNetAction * pWork, int64_t tmNow )
	{
		assert ( pWork && pWork->m_iSock>=0 );

		NetEvent_e eSetup = pWork->Setup ( tmNow );
		assert ( eSetup==NE_IN || eSetup==NE_OUT );

		m_tWork.Add ( pWork );

		epoll_event tEv;
		tEv.data.ptr = pWork;
		tEv.events = ( eSetup==NE_IN ? EPOLLIN : EPOLLOUT );

		sphLogDebugv ( "%p epoll setup, ev=0x%u, sock=%d", pWork, tEv.events, pWork->m_iSock );

		int iRes = epoll_ctl ( m_iEFD, EPOLL_CTL_ADD, pWork->m_iSock, &tEv );
		if ( iRes==-1 )
			sphWarning ( "failed to setup epoll event for sock %d, errno=%d, %s", pWork->m_iSock, errno, strerror(errno) );
	}

	bool Wait ( int timeoutMs )
	{
		m_dReady.Resize ( m_tWork.GetLength() );
		// need positive timeout for communicate threads back and shutdown
		m_iReady = epoll_wait ( m_iEFD, m_dReady.Begin(), m_dReady.GetLength(), timeoutMs );

		if ( m_iReady<0 )
		{
			int iErrno = sphSockGetErrno();
			// common recoverable errors
			if ( iErrno==EINTR || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
				return false;

			if ( m_iLastReportedErrno!=iErrno )
			{
				sphWarning ( "poll tick failed: %s", sphSockError(iErrno) );
				m_iLastReportedErrno = iErrno;
			}
			return false;
		}

		return ( m_iReady>0 );
	}

	bool IterateNextAll ()
	{
		if ( !m_tIter.m_pWork )
		{
			if ( m_tWork.Begin()==m_tWork.End() )
				return false;

			m_tIter.m_pWork = (ISphNetAction *)m_tWork.Begin();
			return true;
		} else
		{
			m_tIter.m_pWork = (ISphNetAction *)m_tIter.m_pWork->m_pNext;
			if ( m_tIter.m_pWork!=m_tWork.End() )
				return true;

			m_tIter.m_pWork = NULL;
			return false;
		}
	}

	bool IterateNextReady ()
	{
		m_iIterEv++;
		if ( m_iReady<=0 || m_iIterEv>=m_iReady )
		{
			m_tIter.m_pWork = NULL;
			m_tIter.m_uEvents = 0;
			return false;
		}

		const epoll_event & tEv = m_dReady[m_iIterEv];

		m_tIter.m_pWork = (ISphNetAction *)tEv.data.ptr;
		m_tIter.m_uEvents = 0;

		if ( tEv.events & EPOLLIN )
			m_tIter.m_uEvents |= NE_IN;
		if ( tEv.events & EPOLLOUT )
			m_tIter.m_uEvents |= NE_OUT;
		if ( tEv.events & EPOLLHUP )
			m_tIter.m_uEvents |= NE_HUP;
		if ( tEv.events & EPOLLERR )
			m_tIter.m_uEvents |= NE_ERR;

		return true;
	}

	void IterateChangeEvent ( NetEvent_e eEv, ISphNetAction * pWork )
	{
		epoll_event tEv;
		tEv.data.ptr = pWork;
		tEv.events = ( ( eEv & NE_IN )!=0 ? EPOLLIN : EPOLLOUT );

		sphLogDebugv ( "%p epoll change, ev=0x%u, sock=%d", pWork, tEv.events, pWork->m_iSock );

		int iRes = epoll_ctl ( m_iEFD, EPOLL_CTL_MOD, pWork->m_iSock, &tEv );
		if ( iRes==-1 )
			sphWarning ( "failed to modify epoll event for sock %d, errno=%d, %s", pWork->m_iSock, errno, strerror(errno) );
	}

	void IterateRemove ()
	{
		sphLogDebugv ( "%p epoll remove, ev=0x%u, sock=%d", m_tIter.m_pWork, m_tIter.m_uEvents, m_tIter.m_pWork->m_iSock );
		assert ( m_tIter.m_pWork );

		epoll_event tEv;
		int iRes = epoll_ctl ( m_iEFD, EPOLL_CTL_DEL, m_tIter.m_pWork->m_iSock, &tEv );
		if ( iRes==-1 )
			sphWarning ( "failed to remove epoll event for sock %d, errno=%d, %s", m_tIter.m_pWork->m_iSock, errno, strerror(errno) );

		ISphNetAction * pPrev = (ISphNetAction *)m_tIter.m_pWork->m_pPrev;
		m_tWork.Remove ( m_tIter.m_pWork );
		m_tIter.m_pWork = pPrev;
		// SafeDelete ( m_tIter.m_pWork );
	}

	void IterateStart ()
	{
		m_tIter.m_pWork = NULL;
		m_tIter.m_uEvents = 0;
		m_iIterEv = -1;
	}

	EventsIterator_t & IterateGet ()
	{
		return m_tIter;
	}
};
#endif

class CSphEventsDummy
{
	EventsIterator_t m_tIter;

public:
	void SetupEvent ( ISphNetAction * , int64_t ) {}
	bool Wait ( int ) { return false; } // NOLINT
	bool IterateNextAll () { return false; }
	bool IterateNextReady () { return false; }
	void IterateChangeEvent ( NetEvent_e , ISphNetAction * ) {}
	void IterateRemove () {}
	void IterateStart () {}
	EventsIterator_t & IterateGet () { return m_tIter; }
};

static bool g_bVtune = false;
static int64_t g_tmStarted = 0;

// vtune
#ifdef USE_VTUNE
#include "ittnotify.h"
#if USE_WINDOWS
#pragma comment(linker, "/defaultlib:libittnotify.lib")
#pragma message("Automatically linking with libittnotify.lib")
#endif
#endif

struct LoopProfiler_t
{
#ifdef USE_VTUNE
	__itt_domain *			m_pDomain;
	__itt_string_handle *	m_pTaskPoll;
	__itt_string_handle *	m_pTaskTick;
	__itt_string_handle *	m_pTaskActions;
	__itt_string_handle *	m_pTaskAT;
	__itt_string_handle *	m_pTaskAR;
	__itt_string_handle *	m_pTaskRemove;
	__itt_string_handle *	m_pTaskNext;
	__itt_string_handle *	m_pTaskExt;
	__itt_string_handle *	m_pTaskClean;
	__itt_string_handle *	m_pTaskStat;
#endif

	bool					m_bEnable;
	int64_t					m_tmTotal;
	int m_iPerfEv, m_iPerfNext, m_iPerfExt, m_iPerfClean;

	LoopProfiler_t ()
	{
		m_bEnable = g_bVtune;
#ifdef USE_VTUNE
		__itt_thread_set_name ( "net-loop" );
		m_pDomain = __itt_domain_create ( "Task Domain" );
		m_pTaskPoll = __itt_string_handle_create ( "poll" );
		m_pTaskTick = __itt_string_handle_create ( "tick" );
		m_pTaskActions = __itt_string_handle_create ( "actions" );
		m_pTaskAT = __itt_string_handle_create ( "Ta" );
		m_pTaskAR = __itt_string_handle_create ( "ra" );
		m_pTaskRemove = __itt_string_handle_create ( "remove" );
		m_pTaskNext = __itt_string_handle_create ( "next" );
		m_pTaskExt = __itt_string_handle_create ( "ext" );
		m_pTaskClean = __itt_string_handle_create ( "clean" );
		m_pTaskStat = __itt_string_handle_create ( "stat" );
		if ( !m_bEnable )
			m_pDomain->flags = 0;
		else
			__itt_resume ();
#endif
	}

	void End ()
	{
		EndTask();
#ifdef USE_VTUNE
		if ( m_bEnable )
		{
			int64_t tmNow = sphMicroTimer();
			int64_t tmDelta = tmNow - m_tmTotal;
			int64_t tmPassed = tmNow - g_tmStarted;
			sphLogDebug ( "loop=%.3f, (act=%d, next=%d, ext=%d, cln=%d), passed=%.3f", ((float)tmDelta)/1000.0f,
				m_iPerfEv, m_iPerfNext, m_iPerfExt, m_iPerfClean, ((float)tmPassed)/1000000.0f );
		}
#endif
	}
	void EndTask ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_end ( m_pDomain );
#endif
	}
	void Start ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
		{
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskTick );
			m_tmTotal = sphMicroTimer();
			m_iPerfEv = m_iPerfNext = m_iPerfExt = m_iPerfClean = 0;
		}
#endif
	}
	void StartPoll ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskPoll );
#endif
	}
	void StartTick ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskActions );
#endif
	}
	void StartRemove ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskRemove );
#endif
	}
	void StartExt ()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskExt );
#endif
	}
	void StartAt()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskAT );
#endif
	}
	void StartAr()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskAR );
#endif
	}
	void StartClean()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskClean );
#endif
	}
	void StartNext()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskNext );
#endif
	}
	void StartStat()
	{
#ifdef USE_VTUNE
		if ( m_bEnable )
			__itt_task_begin ( m_pDomain, __itt_null, __itt_null, m_pTaskStat );
#endif
	}
};

struct ThdJobCleanup_t : public ISphJob
{
	CSphVector<ISphNetAction *> m_dCleanup;

	explicit ThdJobCleanup_t ( CSphVector<ISphNetAction *> & dCleanup );
	virtual ~ThdJobCleanup_t ();

	virtual void		Call ();
	void				Clear();
};

static int	g_tmWait = 1;
static int	g_iNetWorkers = 1;
static int	g_iThrottleAction = 0;
static int	g_iThrottleAccept = 0;

class CSphNetLoop
{
private:
	CSphVector<ISphNetAction *>		m_dWorkExternal;
	volatile bool					m_bGotExternal;
	CSphMutex						m_tExtLock;
	LoopProfiler_t					m_tPrf;
#if HAVE_EPOLL
	CSphEventsEpoll					m_tEvents;
#elif HAVE_POLL
	CSphEventsPoll					m_tEvents;
#else
	CSphEventsDummy					m_tEvents;
#endif

	explicit CSphNetLoop ( CSphVector<Listener_t> & dListeners )
	{
		int64_t tmNow = sphMicroTimer();
		ARRAY_FOREACH ( i, dListeners )
		{
			NetActionAccept_t * pCur = new NetActionAccept_t ( dListeners[i] );
			m_tEvents.SetupEvent ( pCur, tmNow );
		}

		m_bGotExternal = false;
		m_dWorkExternal.Reserve ( 1000 );
	}

	~CSphNetLoop()
	{
		ARRAY_FOREACH ( i, m_dWorkExternal )
			SafeDelete ( m_dWorkExternal[i] );
	}

	void Tick ()
	{
		CSphVector<ISphNetAction *> dCleanup;
		CSphVector<ISphNetAction *> dWorkNext;
		dWorkNext.Reserve ( 1000 );
		dCleanup.Reserve ( 1000 );
		int64_t tmNextCheck = INT64_MAX;
		const int tmDefaultMs = g_tmWait;

		while ( !g_bShutdown )
		{
			m_tPrf.Start();

			m_tPrf.StartPoll();
			// need positive timeout for communicate threads back and shutdown
			bool bGot = m_tEvents.Wait ( tmDefaultMs );
			m_tPrf.EndTask();

			// try to remove outdated items on no signals
			if ( !bGot && !m_bGotExternal )
			{
				tmNextCheck = RemoveOutdated ( tmNextCheck, dCleanup );
				Cleanup ( dCleanup );
				m_tPrf.End();
				continue;
			}

			// add actions planned by jobs
			if ( m_bGotExternal )
			{
				m_tPrf.StartExt();
				m_tPrf.m_iPerfExt = m_dWorkExternal.GetLength();
				assert ( dWorkNext.GetLength()==0 );
				Verify ( m_tExtLock.Lock() );
				dWorkNext.SwapData ( m_dWorkExternal );
				m_bGotExternal = false;
				Verify ( m_tExtLock.Unlock() );
				m_tPrf.EndTask();
			}

			// handle events and collect stats
			m_tPrf.StartTick();
			int iConnections = 0;
			m_tEvents.IterateStart();
			int iMaxIters = 0;
			while ( m_tEvents.IterateNextReady() && ( !g_iThrottleAction || iMaxIters<g_iThrottleAction ) )
			{
				m_tPrf.StartAt();
				assert ( m_tEvents.IterateGet().m_pWork && m_tEvents.IterateGet().m_uEvents );
				ISphNetAction * pWork = m_tEvents.IterateGet().m_pWork;

				NetEvent_e eEv = pWork->Tick ( m_tEvents.IterateGet().m_uEvents, dWorkNext, this );
				pWork->GetStats ( iConnections );
				m_tPrf.m_iPerfEv++;
				iMaxIters++;

				if ( eEv==NE_KEEP )
				{
					m_tPrf.EndTask();
					continue;
				}

				m_tPrf.StartAr();
				if ( eEv==NE_REMOVE )
				{
					m_tEvents.IterateRemove ();
					dCleanup.Add ( pWork );
				} else
				{
					m_tEvents.IterateChangeEvent ( eEv, pWork );
				}
				m_tPrf.EndTask();
				m_tPrf.EndTask();
			}
			m_tPrf.EndTask();

			tmNextCheck = RemoveOutdated ( tmNextCheck, dCleanup );
			Cleanup ( dCleanup );

			m_tPrf.StartNext();
			int64_t	tmNow = sphMicroTimer();
			// setup new handlers
			ARRAY_FOREACH ( i, dWorkNext )
			{
				ISphNetAction * pWork = dWorkNext[i];
				m_tEvents.SetupEvent ( pWork, tmNow );
				if ( pWork->m_tmTimeout )
					tmNextCheck = Min ( tmNextCheck, pWork->m_tmTimeout );
			}
			m_tPrf.m_iPerfNext = dWorkNext.GetLength();
			dWorkNext.Resize ( 0 );
			m_tPrf.EndTask();

			// update stats
			if ( iConnections )
			{
				m_tPrf.StartStat();
				g_tStatsMutex.Lock();
				g_tStats.m_iConnections += iConnections;
				g_tStatsMutex.Unlock();
				m_tPrf.EndTask();
			}

			m_tPrf.End();
		}
	}

	int64_t RemoveOutdated ( int64_t tmNextCheck, CSphVector<ISphNetAction *> & dCleanup )
	{
		int64_t tmNow = sphMicroTimer();
		if ( tmNow<tmNextCheck )
			return tmNextCheck;

		m_tPrf.StartRemove();

		// remove outdated items on no signals
		tmNextCheck = INT64_MAX;
		m_tEvents.IterateStart();
		while ( m_tEvents.IterateNextAll() )
		{
			assert ( m_tEvents.IterateGet().m_pWork );
			ISphNetAction * pWork = m_tEvents.IterateGet().m_pWork;
			if ( !pWork->m_tmTimeout )
				continue;

			if ( tmNow<pWork->m_tmTimeout )
			{
				tmNextCheck = Min ( tmNextCheck, pWork->m_tmTimeout );
				continue;
			}

			sphLogDebugv ( "%p bailing on timeout no signal, sock=%d", pWork, pWork->m_iSock );
			m_tEvents.IterateRemove ();
			// SafeDelete ( pWork );
			// close socket immediately to prevent write by client into persist connection that just timed out
			// that does not need in case Work got removed at IterateRemove + SafeDelete ( pWork );
			// but deferred clean up by ThdJobCleanup_t needs to close socket here right after it was removed from e(poll) set
			pWork->CloseSocket();
			dCleanup.Add ( pWork );
		}

		m_tPrf.EndTask();

		return tmNextCheck;
	}

	void Cleanup ( CSphVector<ISphNetAction *> & dCleanup )
	{
		if ( !dCleanup.GetLength() )
			return;

		m_tPrf.m_iPerfClean = dCleanup.GetLength();
		m_tPrf.StartClean();
		ThdJobCleanup_t * pCleanup = new ThdJobCleanup_t ( dCleanup );
		g_pThdPool->AddJob ( pCleanup );
		m_tPrf.EndTask();
	}

public:
	void AddAction ( ISphNetAction * pElem )
	{
		Verify ( m_tExtLock.Lock() );
		m_bGotExternal = true;
		m_dWorkExternal.Add ( pElem );
		Verify ( m_tExtLock.Unlock() );
	}

	// main thread wrapper
	static void ThdTick ( void * )
	{
		SphCrashLogger_c tQueryTLS;
		tQueryTLS.SetupTLS ();

		CSphNetLoop tLoop ( g_dListeners );
		tLoop.Tick();
	}
};

struct ThdJobAPI_t : public ISphJob
{
	CSphScopedPtr<NetStateAPI_t>		m_tState;
	CSphNetLoop *		m_pLoop;
	int					m_iCommand;
	int					m_iCommandVer;

	ThdJobAPI_t ( CSphNetLoop * pLoop, NetStateAPI_t * pState );
	virtual ~ThdJobAPI_t () {}

	virtual void		Call ();
};

struct ThdJobQL_t : public ISphJob
{
	CSphScopedPtr<NetStateQL_t>		m_tState;
	CSphNetLoop *		m_pLoop;

	ThdJobQL_t ( CSphNetLoop * pLoop, NetStateQL_t * pState );
	virtual ~ThdJobQL_t () {}

	virtual void		Call ();
};


struct ThdJobHttp_t : public ISphJob
{
	CSphScopedPtr<NetStateQL_t>		m_tState;
	CSphNetLoop *		m_pLoop;

	ThdJobHttp_t ( CSphNetLoop * pLoop, NetStateQL_t * pState );
	virtual ~ThdJobHttp_t () {}

	virtual void		Call ();
};


static void LogSocketError ( const char * sMsg, const NetStateCommon_t * pConn, bool bDebug )
{
	if ( bDebug && g_eLogLevel<SPH_LOG_VERBOSE_DEBUG )
		return;

	assert ( pConn );
	int iErrno = sphSockGetErrno();
	if ( iErrno==0 && pConn->m_iClientSock>=0 )
	{
		socklen_t iLen = sizeof(iErrno);
		int iRes = getsockopt ( pConn->m_iClientSock, SOL_SOCKET, SO_ERROR, (char*)&iErrno, &iLen );
		if ( iRes<0 )
			sphWarning ( "%s (client=%s(%d)), failed to get error: %d '%s'", sMsg, pConn->m_sClientName, pConn->m_iConnID, errno, strerror ( errno ) );
	}

	if ( bDebug || iErrno==ESHUTDOWN )
		sphLogDebugv ( "%s (client=%s(%d)), error: %d '%s', sock=%d", sMsg, pConn->m_sClientName, pConn->m_iConnID, iErrno, sphSockError ( iErrno ), pConn->m_iClientSock );
	else
		sphWarning ( "%s (client=%s(%d)), error: %d '%s', sock=%d", sMsg, pConn->m_sClientName, pConn->m_iConnID, iErrno, sphSockError ( iErrno ), pConn->m_iClientSock );
}

static bool CheckSocketError ( DWORD uGotEvents, const char * sMsg, const NetStateCommon_t * pConn, bool bDebug )
{
	bool bReadError = ( ( uGotEvents & NE_IN ) && ( uGotEvents & ( NE_ERR | NE_HUP ) ) );
	bool bWriteError = ( ( uGotEvents & NE_OUT ) && ( uGotEvents & NE_ERR ) );

	if ( bReadError || bWriteError )
	{
		LogSocketError ( sMsg, pConn, bDebug );
		return true;
	} else
	{
		return false;
	}
}

static int NetManageSocket ( int iSock, char * pBuf, int iSize, bool bWrite )
{
	// try next chunk
	int iRes = 0;
	if ( bWrite )
		iRes = sphSockSend ( iSock, pBuf, iSize );
	else
		iRes = sphSockRecv ( iSock, pBuf, iSize );

	// if there was EINTR, retry
	// if any other error, bail
	if ( iRes==-1 )
	{
		// only let SIGTERM (of all them) to interrupt
		int iErr = sphSockPeekErrno();
		return ( ( iErr==EINTR || iErr==EAGAIN || iErr==EWOULDBLOCK ) ? 0 : -1 );
	}

	// if there was eof, we're done
	if ( !bWrite && iRes==0 )
	{
		sphSockSetErrno ( ESHUTDOWN );
		return -1;
	}

	return iRes;
}

static WORD NetBufGetWord ( const BYTE * pBuf )
{
	WORD uVal = sphUnalignedRead ( (WORD &)*pBuf );
	return ntohs ( uVal );
}

static DWORD NetBufGetLSBDword ( const BYTE * pBuf )
{
	return pBuf[0] + ( ( pBuf[1] ) <<8 ) + ( ( pBuf[2] )<<16 ) + ( ( pBuf[3] )<<24 );
}

static int NetBufGetInt ( const BYTE * pBuf )
{
	int uVal = sphUnalignedRead ( (int &)*pBuf );
	return ntohl ( uVal );
}


NetActionAccept_t::NetActionAccept_t ( const Listener_t & tListener )
	: ISphNetAction ( tListener.m_iSock )
{
	m_tListener = tListener;
	m_iConnections = 0;
}

NetEvent_e NetActionAccept_t::Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * )
{
	if ( CheckSocketError ( uGotEvents, "accept err WTF???", &m_tDummy, true ) )
		return NE_KEEP;

	// handle all incoming requests at once but not too much
	int iLastConn = m_iConnections;
	sockaddr_storage saStorage;
	socklen_t uLength = sizeof(saStorage);

	for ( ;; )
	{
		if ( g_iThrottleAccept && g_iThrottleAccept<m_iConnections-iLastConn )
		{
			sphLogDebugv ( "%p accepted %d connections throttled", this, m_iConnections-iLastConn );
			return NE_KEEP;
		}

		// accept
		int iClientSock = accept ( m_tListener.m_iSock, (struct sockaddr *)&saStorage, &uLength );

		// handle failures and no more incoming clients
		if ( iClientSock<0 )
		{
			const int iErrno = sphSockGetErrno();
			if ( iErrno==EINTR || iErrno==ECONNABORTED || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
			{
				if ( m_iConnections!=iLastConn )
					sphLogDebugv ( "%p accepted %d connections all", this, m_iConnections-iLastConn );

				return NE_KEEP;
			}

			if ( iErrno==EMFILE || iErrno==ENFILE )
			{
				sphWarning ( "accept() failed, raise ulimit -n and restart searchd: %s", sphSockError(iErrno) );
				return NE_KEEP;
			}

			sphFatal ( "accept() failed: %s", sphSockError(iErrno) );
		}

		if ( sphSetSockNB ( iClientSock )<0 )
		{
			sphWarning ( "sphSetSockNB() failed: %s", sphSockError() );
			sphSockClose ( iClientSock );
			return NE_KEEP;
		}

#ifdef	TCP_NODELAY
		int bNoDelay = 1;
		if ( saStorage.ss_family==AF_INET && setsockopt ( iClientSock, IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(bNoDelay) )<0 )
		{
			sphWarning ( "set of TCP_NODELAY failed: %s", sphSockError() );
			sphSockClose ( iClientSock );
			return NE_KEEP;
		}
#endif

		m_iConnections++;
		int iConnID = ++g_iConnectionID;
		if ( g_iConnectionID<0 )
		{
			g_iConnectionID = 0;
			iConnID = 0;
		}

		ISphNetAction * pAction = NULL;
		if ( m_tListener.m_eProto==PROTO_SPHINX )
		{
			NetStateAPI_t * pStateAPI = new NetStateAPI_t ();
			pStateAPI->m_dBuf.Reserve ( 65535 );
			FillNetState ( pStateAPI, iClientSock, iConnID, m_tListener.m_bVIP, saStorage );
			pAction = new NetReceiveDataAPI_t ( pStateAPI );
		} else if ( m_tListener.m_eProto==PROTO_HTTP )
		{
			NetStateQL_t * pStateHttp = new NetStateQL_t ();
			pStateHttp->m_dBuf.Reserve ( 65535 );
			FillNetState ( pStateHttp, iClientSock, iConnID, false, saStorage );
			pAction = new NetReceiveDataHttp_t ( pStateHttp );
		} else
		{
			NetStateQL_t * pStateQL = new NetStateQL_t ();
			pStateQL->m_dBuf.Reserve ( 65535 );
			FillNetState ( pStateQL, iClientSock, iConnID, m_tListener.m_bVIP, saStorage );
			NetReceiveDataQL_t * pActionQL = new NetReceiveDataQL_t ( pStateQL );
			pActionQL->SetupHandshakePhase();
			pAction = pActionQL;
		}
		dNextTick.Add ( pAction );
		sphLogDebugv ( "%p accepted %s, sock=%d", this, g_dProtoNames[m_tListener.m_eProto], pAction->m_iSock );
	}
}

NetEvent_e NetActionAccept_t::Setup ( int64_t )
{
	return NE_IN;
}

bool NetActionAccept_t::GetStats ( int & iConnections )
{
	if ( !m_iConnections )
		return false;

	iConnections += m_iConnections;
	m_iConnections = 0;
	return true;
}

void NetActionAccept_t::FillNetState ( NetStateCommon_t * pState, int iClientSock, int iConnID, bool bVIP, const sockaddr_storage & saStorage ) const
{
	pState->m_iClientSock = iClientSock;
	pState->m_iConnID = iConnID;
	pState->m_bVIP = bVIP;

	// format client address
	pState->m_sClientName[0] = '\0';
	if ( saStorage.ss_family==AF_INET )
	{
		struct sockaddr_in * pSa = ((struct sockaddr_in *)&saStorage);
		sphFormatIP ( pState->m_sClientName, SPH_ADDRESS_SIZE, pSa->sin_addr.s_addr );

		char * d = pState->m_sClientName;
		while ( *d )
			d++;
		snprintf ( d, 7, ":%d", (int)ntohs ( pSa->sin_port ) ); //NOLINT
	} else if ( saStorage.ss_family==AF_UNIX )
	{
		strncpy ( pState->m_sClientName, "(local)", SPH_ADDRESS_SIZE );
	}
}


const char * g_sErrorNetAPI[] = { "failed to send server version", "exiting on handshake error", "bailing on failed request header", "failed to receive client request body" };
STATIC_ASSERT ( sizeof(g_sErrorNetAPI)/sizeof(g_sErrorNetAPI[0])==AAPI_TOTAL, NOT_ALL_EMUN_DESCRIBERD );

NetReceiveDataAPI_t::NetReceiveDataAPI_t ( NetStateAPI_t *	pState )
	: ISphNetAction ( pState->m_iClientSock )
	, m_tState ( pState )
{
	m_ePhase = AAPI_HANDSHAKE_OUT;
	m_tState->m_iPos = 0;
	m_iCommand = 0;
	m_iCommandVer = 0;

	m_tState->m_dBuf.Resize ( 4 );
	*(DWORD *)( m_tState->m_dBuf.Begin() ) = htonl ( SPHINX_SEARCHD_PROTO );
	m_tState->m_iLeft = 4;
	assert ( m_tState.Ptr() );
}

NetEvent_e NetReceiveDataAPI_t::Setup ( int64_t tmNow )
{
	assert ( m_tState.Ptr() );
	sphLogDebugv ( "%p receive API setup, phase=%d, keep=%d, client=%s, conn=%d, sock=%d", this, m_ePhase, m_tState->m_bKeepSocket, m_tState->m_sClientName, m_tState->m_iConnID, m_tState->m_iClientSock );

	if ( !m_tState->m_bKeepSocket )
	{
		m_tmTimeout = tmNow + MS2SEC * g_iReadTimeout;
	} else
	{
		m_tmTimeout = tmNow + MS2SEC * g_iClientTimeout;
	}

	return ( m_ePhase==AAPI_HANDSHAKE_OUT ? NE_OUT : NE_IN );
}


void NetReceiveDataAPI_t::SetupBodyPhase()
{
	m_tState->m_dBuf.Resize ( 8 );
	m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
	m_ePhase = AAPI_COMMAND;
}

void NetReceiveDataAPI_t::AddJobAPI ( CSphNetLoop * pLoop )
{
	assert ( m_tState.Ptr() );
	bool bStart = m_tState->m_bVIP;
	int iLen = m_tState->m_dBuf.GetLength();
	ThdJobAPI_t * pJob = new ThdJobAPI_t ( pLoop, m_tState.LeakPtr() );
	pJob->m_iCommand = m_iCommand;
	pJob->m_iCommandVer = m_iCommandVer;
	sphLogDebugv ( "%p receive API job created (%p), buf=%d, sock=%d", this, pJob, iLen, m_iSock );
	if ( bStart )
		g_pThdPool->StartJob ( pJob );
	else
		g_pThdPool->AddJob ( pJob );
}

static char g_sMaxedOutMessage[] = "maxed out, dismissing client";

NetEvent_e NetReceiveDataAPI_t::Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop )
{
	assert ( m_tState.Ptr() );
	bool bDebug = ( m_ePhase==AAPI_HANDSHAKE_IN || m_ePhase==AAPI_COMMAND );
	if ( CheckSocketError ( uGotEvents, g_sErrorNetAPI[m_ePhase], m_tState.Ptr(), bDebug ) )
		return NE_REMOVE;

	bool bWasWrite = ( m_ePhase==AAPI_HANDSHAKE_OUT );
	// loop to handle similar operations at once
	for ( ;; )
	{
		bool bWrite = ( m_ePhase==AAPI_HANDSHAKE_OUT );
		int iRes = NetManageSocket ( m_tState->m_iClientSock, (char *)( m_tState->m_dBuf.Begin() + m_tState->m_iPos ), m_tState->m_iLeft, bWrite );
		if ( iRes==-1 )
		{
			LogSocketError ( g_sErrorNetAPI[m_ePhase], m_tState.Ptr(), false );
			return NE_REMOVE;
		}
		m_tState->m_iLeft -= iRes;
		m_tState->m_iPos += iRes;

		// socket would block - going back to polling
		if ( iRes==0 )
		{
			bool bNextWrite = ( m_ePhase==AAPI_HANDSHAKE_OUT );
			if ( bWasWrite!=bNextWrite )
				return ( bNextWrite ? NE_OUT : NE_IN );
			else
				return NE_KEEP;
		}

		// keep using that socket
		if ( m_tState->m_iLeft )
			continue;

		sphLogDebugv ( "%p pre-API phase=%d, buf=%d, write=%d, sock=%d", this, m_ePhase, m_tState->m_dBuf.GetLength(), (int)bWrite, m_iSock );

		// FIXME!!! handle persist socket timeout
		m_tState->m_iPos = 0;
		switch ( m_ePhase )
		{
		case AAPI_HANDSHAKE_OUT:
			m_tState->m_dBuf.Resize ( 4 );
			m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
			m_ePhase = AAPI_HANDSHAKE_IN;
			break;

		case AAPI_HANDSHAKE_IN:
			// magic unused
			SetupBodyPhase();
			break;

		case AAPI_COMMAND:
		{
			m_iCommand = NetBufGetWord ( m_tState->m_dBuf.Begin() );
			m_iCommandVer = NetBufGetWord ( m_tState->m_dBuf.Begin() + 2 );
			m_tState->m_iLeft = NetBufGetInt ( m_tState->m_dBuf.Begin() + 4 );
			bool bBadCommand = ( m_iCommand<0 || m_iCommand>=SEARCHD_COMMAND_TOTAL );
			bool bBadLength = ( m_tState->m_iLeft<0 || m_tState->m_iLeft>g_iMaxPacketSize );
			bool bMaxedOut = ( g_iThdQueueMax && !m_tState->m_bVIP && g_pThdPool->GetQueueLength()>=g_iThdQueueMax );
			if ( bBadLength || bBadLength || bMaxedOut )
			{
				m_tState->m_bKeepSocket = false;

				// unknown command, default response header
				if ( bBadLength )
					sphWarning ( "ill-formed client request (length=%d out of bounds)", m_tState->m_iLeft );
				// if command is insane, low level comm is broken, so we bail out
				if ( bBadCommand )
					sphWarning ( "ill-formed client request (command=%d, SEARCHD_COMMAND_TOTAL=%d)", m_iCommand, SEARCHD_COMMAND_TOTAL );
				// warning on thread pool work maxed out
				if ( bMaxedOut )
					sphWarning ( "%s", g_sMaxedOutMessage );

				ISphOutputBuffer tOut ( m_tState->m_dBuf );
				if ( !bMaxedOut )
				{
					SendErrorReply ( tOut, "invalid command (code=%d, len=%d)", m_iCommand, m_tState->m_iLeft );
				} else
				{
					int iRespLen = 4 + strlen(g_sMaxedOutMessage);

					tOut.SendInt ( SPHINX_CLIENT_VERSION );
					tOut.SendWord ( (WORD)SEARCHD_RETRY );
					tOut.SendWord ( 0 ); // version doesn't matter
					tOut.SendInt ( iRespLen );
					tOut.SendString ( g_sMaxedOutMessage );
				}

				tOut.SwapData ( m_tState->m_dBuf );
				NetSendData_t * pSend = new NetSendData_t ( m_tState.LeakPtr(), PROTO_SPHINX );
				dNextTick.Add ( pSend );
				return NE_REMOVE;
			}
			m_tState->m_dBuf.Resize ( m_tState->m_iLeft );
			m_ePhase = AAPI_BODY;
			if ( !m_tState->m_iLeft ) // command without body
			{
				AddJobAPI ( pLoop );
				return NE_REMOVE;
			}
		}
		break;

		case AAPI_BODY:
		{
			AddJobAPI ( pLoop );
			return NE_REMOVE;
		}
		break;

		default: return NE_REMOVE;
		} // switch

		bool bNextWrite = ( m_ePhase==AAPI_HANDSHAKE_OUT );
		sphLogDebugv ( "%p post-API phase=%d, buf=%d, write=%d, sock=%d", this, m_ePhase, m_tState->m_dBuf.GetLength(), (int)bNextWrite, m_iSock );
	}
}

void NetReceiveDataAPI_t::CloseSocket()
{
	if ( m_tState.Ptr() )
		m_tState->CloseSocket();
}

const char * g_sErrorNetQL[] = { "failed to send SphinxQL handshake", "bailing on failed MySQL header", "failed to receive MySQL request body", "failed to send SphinxQL auth" };
STATIC_ASSERT ( sizeof(g_sErrorNetQL)/sizeof(g_sErrorNetQL[0])==AQL_TOTAL, NOT_ALL_EMUN_DESCRIBERD );

NetReceiveDataQL_t::NetReceiveDataQL_t ( NetStateQL_t * pState )
	: ISphNetAction ( pState->m_iClientSock )
	, m_tState ( pState )
{
	m_ePhase = AQL_HANDSHAKE;
	m_tState->m_iLeft = 0;
	m_tState->m_iPos = 0;
	m_bAppend = false;
	m_bWrite = false;
	assert ( m_tState.Ptr() );
}

void NetReceiveDataQL_t::SetupHandshakePhase()
{
	m_ePhase = AQL_HANDSHAKE;

	m_tState->m_dBuf.Resize ( g_iMysqlHandshake );
	memcpy ( m_tState->m_dBuf.Begin(), g_sMysqlHandshake, g_iMysqlHandshake );
	m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
	m_tState->m_iPos = 0;

	m_bAppend = false;
	m_bWrite = true;
}

void NetReceiveDataQL_t::SetupBodyPhase()
{
	m_tState->m_dBuf.Resize ( 4 );
	m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
	m_tState->m_iPos = 0;

	m_bAppend = false;
	m_bWrite = false;

	m_ePhase = AQL_LENGTH;
}

NetEvent_e NetReceiveDataQL_t::Setup ( int64_t tmNow )
{
	assert ( m_tState.Ptr() );
	sphLogDebugv ( "%p receive QL setup, phase=%d, client=%s, conn=%d, sock=%d", this, m_ePhase, m_tState->m_sClientName, m_tState->m_iConnID, m_tState->m_iClientSock );

	m_tmTimeout = tmNow + MS2SEC * g_iClientQlTimeout;

	NetEvent_e eEvent;
	if ( m_ePhase==AQL_HANDSHAKE )
	{
		eEvent = NE_OUT;
		m_bWrite = true;
	} else
	{
		eEvent = NE_IN;
		m_bWrite = false;
	}
	return eEvent;
}

NetEvent_e NetReceiveDataQL_t::Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> &, CSphNetLoop * pLoop )
{
	assert ( m_tState.Ptr() );
	bool bDebugErr = ( ( m_ePhase==AQL_LENGTH && !m_bAppend ) || m_ePhase==AQL_AUTH );
	if ( CheckSocketError ( uGotEvents, g_sErrorNetQL[m_ePhase], m_tState.Ptr(), bDebugErr ) )
		return NE_REMOVE;

	bool bWrite = m_bWrite;
	// loop to handle similar operations at once
	for ( ;; )
	{
		int iRes = NetManageSocket ( m_tState->m_iClientSock, (char *)( m_tState->m_dBuf.Begin() + m_tState->m_iPos ), m_tState->m_iLeft, m_bWrite );
		if ( iRes==-1 )
		{
			LogSocketError ( g_sErrorNetQL[m_ePhase], m_tState.Ptr(), false );
			return NE_REMOVE;
		}
		m_tState->m_iLeft -= iRes;
		m_tState->m_iPos += iRes;

		// socket would block - going back to polling
		if ( iRes==0 )
		{
			if ( bWrite!=m_bWrite )
				return ( m_bWrite ? NE_OUT : NE_IN );
			else
				return NE_KEEP;
		}

		// keep using that socket
		if ( m_tState->m_iLeft )
			continue;

		sphLogDebugv ( "%p pre-QL phase=%d, buf=%d, append=%d, write=%d, sock=%d", this, m_ePhase, m_tState->m_dBuf.GetLength(), (int)m_bAppend, (int)m_bWrite, m_iSock );

		switch ( m_ePhase )
		{
		case AQL_HANDSHAKE:
			// no action required - switching to next phase
			SetupBodyPhase();
		break;

		case AQL_LENGTH:
		{
			const int MAX_PACKET_LEN = 0xffffffL; // 16777215 bytes, max low level packet size
			DWORD uHeader = 0;
			const BYTE * pBuf = ( m_bAppend ? m_tState->m_dBuf.Begin() + m_tState->m_iPos - sizeof(uHeader) : m_tState->m_dBuf.Begin() );
			uHeader = NetBufGetLSBDword ( pBuf );
			m_tState->m_uPacketID = 1 + (BYTE)( uHeader>>24 ); // client will expect this id
			m_tState->m_iLeft = ( uHeader & MAX_PACKET_LEN );

			if ( m_bAppend ) // reading big packet
			{
				m_tState->m_iPos = m_tState->m_dBuf.GetLength() - sizeof(uHeader);
				m_tState->m_dBuf.Resize ( m_tState->m_iPos + m_tState->m_iLeft );
			} else // reading regular packet
			{
				m_tState->m_iPos = 0;
				m_tState->m_dBuf.Resize ( m_tState->m_iLeft );
			}

			// check request length
			if ( m_tState->m_dBuf.GetLength()>g_iMaxPacketSize )
			{
				sphWarning ( "ill-formed client request (length=%d out of bounds)", m_tState->m_dBuf.GetLength() );
				return NE_REMOVE;
			}

			m_bWrite = false;
			m_bAppend = ( m_tState->m_iLeft==MAX_PACKET_LEN ); // might be next big packet
			m_ePhase = AQL_BODY;
		}
		break;

		case AQL_BODY:
		{
			if ( m_bAppend )
			{
				m_tState->m_iLeft = 4;
				m_tState->m_dBuf.Resize ( m_tState->m_iPos + m_tState->m_iLeft );

				m_bWrite = false;
				m_ePhase = AQL_LENGTH;
			} else if ( !m_tState->m_bAuthed )
			{
				m_tState->m_bAuthed = true;
				m_tState->m_dBuf.Resize ( 0 );
				ISphOutputBuffer tOut ( m_tState->m_dBuf );
				SendMysqlOkPacket ( tOut, m_tState->m_uPacketID );
				tOut.SwapData ( m_tState->m_dBuf );

				m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
				m_tState->m_iPos = 0;

				m_bWrite = true;
				m_ePhase = AQL_AUTH;
			} else
			{
				CSphVector<BYTE> & dBuf = m_tState->m_dBuf;
				int iBufLen = dBuf.GetLength();
				int iCmd = ( iBufLen>0 ? dBuf[0] : 0 );
				int iStrLen = Min ( iBufLen, 80 );
				sphLogDebugv ( "%p receive-QL, cmd=%d, buf=%d, sock=%d, '%.*s'", this, iCmd, iBufLen, m_iSock, iStrLen, ( dBuf.GetLength() ? (const char *)dBuf.Begin() : "" ) );

				bool bEmptyBuf = !dBuf.GetLength();
				bool bMaxedOut = ( iCmd==MYSQL_COM_QUERY && !m_tState->m_bVIP && g_iThdQueueMax && g_pThdPool->GetQueueLength()>=g_iThdQueueMax );

				if ( bEmptyBuf || bMaxedOut )
				{
					dBuf.Resize ( 0 );
					ISphOutputBuffer tOut ( dBuf );

					if ( !bMaxedOut )
					{
						SendMysqlErrorPacket ( tOut, m_tState->m_uPacketID, "", "unknown command with size 0", m_tState->m_iConnID, MYSQL_ERR_UNKNOWN_COM_ERROR );
					} else
					{
						LogSphinxqlError ( "", g_sMaxedOutMessage, m_tState->m_iConnID );
						tOut.SendBytes ( g_dSphinxQLMaxedOutPacket, g_iSphinxQLMaxedOutLen );
					}

					tOut.SwapData ( dBuf );
					m_tState->m_iLeft = dBuf.GetLength();
					m_tState->m_iPos = 0;
					m_bWrite = true;
					m_ePhase = AQL_AUTH;
				} else
				{
					// all comes to an end
					if ( iCmd==MYSQL_COM_QUIT )
					{
						m_tState->m_bKeepSocket = false;
						return NE_REMOVE;
					}

					if ( iCmd==MYSQL_COM_QUERY )
					{
						// going to actual work now
						bool bStart = m_tState->m_bVIP;
						ThdJobQL_t * pJob = new ThdJobQL_t ( pLoop, m_tState.LeakPtr() );
						if ( bStart )
							g_pThdPool->StartJob ( pJob );
						else
							g_pThdPool->AddJob ( pJob );

						return NE_REMOVE;
					} else
					{
						// short-cuts to keep action in place and don't dive to job then get back here
						CSphString sError;
						m_tState->m_dBuf.Resize ( 0 );
						ISphOutputBuffer tOut ( m_tState->m_dBuf );
						switch ( iCmd )
						{
						case MYSQL_COM_PING:
						case MYSQL_COM_INIT_DB:
							// client wants a pong
							SendMysqlOkPacket ( tOut, m_tState->m_uPacketID );
							break;

						case MYSQL_COM_SET_OPTION:
							// bMulti = ( tIn.GetWord()==MYSQL_OPTION_MULTI_STATEMENTS_ON ); // that's how we could double check and validate multi query
							// server reporting success in response to COM_SET_OPTION and COM_DEBUG
							SendMysqlEofPacket ( tOut, m_tState->m_uPacketID, 0 );
							break;

						default:
							// default case, unknown command
							sError.SetSprintf ( "unknown command (code=%d)", iCmd );
							SendMysqlErrorPacket ( tOut, m_tState->m_uPacketID, "", sError.cstr(), m_tState->m_iConnID, MYSQL_ERR_UNKNOWN_COM_ERROR );
							break;
						}

						tOut.SwapData ( dBuf );
						m_tState->m_iLeft = dBuf.GetLength();
						m_tState->m_iPos = 0;
						m_bWrite = true;
						m_ePhase = AQL_AUTH;
					}
				}
			}
		}
		break;

		case AQL_AUTH:
			m_tState->m_iLeft = 4;
			m_tState->m_dBuf.Resize ( m_tState->m_iLeft );
			m_tState->m_iPos = 0;

			m_bWrite = false;
			m_ePhase = AQL_LENGTH;
			break;

		default: return NE_REMOVE;
		}

		sphLogDebugv ( "%p post-QL phase=%d, buf=%d, append=%d, write=%d, sock=%d", this, m_ePhase, m_tState->m_dBuf.GetLength(), (int)m_bAppend, (int)m_bWrite, m_iSock );
	}
}


void NetReceiveDataQL_t::CloseSocket()
{
	if ( m_tState.Ptr() )
		m_tState->CloseSocket();
}


NetSendData_t::NetSendData_t ( NetStateCommon_t * pState, ProtocolType_e eProto )
	: ISphNetAction ( pState->m_iClientSock )
	, m_tState ( pState )
	, m_eProto ( eProto )
{
	assert ( pState );
	m_tState->m_iPos = 0;
	m_tState->m_iLeft = 0;
}

NetEvent_e NetSendData_t::Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * )
{
	if ( CheckSocketError ( uGotEvents, "failed to send data", m_tState.Ptr(), false ) )
		return NE_REMOVE;

	for ( ; m_tState->m_iLeft>0; )
	{
		int iRes = NetManageSocket ( m_tState->m_iClientSock, (char *)m_tState->m_dBuf.Begin() + m_tState->m_iPos, m_tState->m_iLeft, true );
		if ( iRes==-1 )
		{
			LogSocketError ( "failed to send data", m_tState.Ptr(), false );
			return NE_REMOVE;
		}
		// for iRes==0 case might proceed after interruption
		m_tState->m_iLeft -= iRes;
		m_tState->m_iPos += iRes;

		// socket would block - going back to polling
		if ( iRes==0 )
			break;
	}

	if ( m_tState->m_iLeft>0 )
		return NE_KEEP;

	assert ( m_tState->m_iLeft==0 && m_tState->m_iPos==m_tState->m_dBuf.GetLength() );

	if ( m_tState->m_bKeepSocket )
	{
		sphLogDebugv ( "%p send %s job created, sent=%d, sock=%d", this, g_dProtoNames[m_eProto], m_tState->m_iPos, m_iSock );
		switch ( m_eProto )
		{
			case PROTO_SPHINX:
			{
				NetReceiveDataAPI_t * pAction = new NetReceiveDataAPI_t ( (NetStateAPI_t *)m_tState.LeakPtr() );
				pAction->SetupBodyPhase();
				dNextTick.Add ( pAction );
			}
			break;

			case PROTO_MYSQL41:
			{
				NetReceiveDataQL_t * pAction = new NetReceiveDataQL_t ( (NetStateQL_t *)m_tState.LeakPtr() );
				pAction->SetupBodyPhase();
				dNextTick.Add ( pAction );
			}
			break;

			case PROTO_HTTP:
			{
				NetReceiveDataHttp_t * pAction = new NetReceiveDataHttp_t ( (NetStateQL_t *)m_tState.LeakPtr() );
				dNextTick.Add ( pAction );
			}
			break;

			default: break;
		}
	}

	return NE_REMOVE;
}

NetEvent_e NetSendData_t::Setup ( int64_t tmNow )
{
	assert ( m_tState.Ptr() );
	sphLogDebugv ( "%p send %s setup, keep=%d, buf=%d, client=%s, conn=%d, sock=%d", this, g_dProtoNames[m_eProto], (int)(m_tState->m_bKeepSocket),
		m_tState->m_dBuf.GetLength(), m_tState->m_sClientName, m_tState->m_iConnID, m_tState->m_iClientSock );

	m_tmTimeout = tmNow + MS2SEC * g_iWriteTimeout;

	assert ( m_tState->m_dBuf.GetLength() );
	m_tState->m_iLeft = m_tState->m_dBuf.GetLength();
	m_tState->m_iPos = 0;

	return NE_OUT;
}


void NetSendData_t::CloseSocket()
{
	if ( m_tState.Ptr() )
		m_tState->CloseSocket();
}


NetStateCommon_t::NetStateCommon_t ()
	: m_iClientSock ( -1 )
	, m_iConnID ( 0 )
	, m_bKeepSocket ( false )
	, m_bVIP ( false )
	, m_iLeft ( 0 )
	, m_iPos ( 0 )
{
	m_sClientName[0] = '\0';
}

NetStateCommon_t::~NetStateCommon_t()
{
	CloseSocket();
}

void NetStateCommon_t::CloseSocket ()
{
	if ( m_iClientSock>=0 )
	{
		sphLogDebugv ( "%p state closing socket=%d", this, m_iClientSock );
		sphSockClose ( m_iClientSock );
		m_iClientSock = -1;
	}
}

NetStateQL_t::NetStateQL_t ()
	: m_tSession ( true )
	, m_bAuthed ( false )
	, m_uPacketID ( 1 )
{}


NetReceiveDataHttp_t::NetReceiveDataHttp_t ( NetStateQL_t *	pState )
	: ISphNetAction ( pState->m_iClientSock )
	, m_tState ( pState )
{
	assert ( m_tState.Ptr() );
	m_tState->m_iPos = 0;
	m_tState->m_iLeft = 4000;
	m_tState->m_dBuf.Resize ( Max ( m_tState->m_dBuf.GetLimit(), 4000 ) );
}

NetEvent_e NetReceiveDataHttp_t::Setup ( int64_t tmNow )
{
	assert ( m_tState.Ptr() );
	sphLogDebugv ( "%p receive HTTP setup, keep=%d, client=%s, conn=%d, sock=%d", this, m_tState->m_bKeepSocket, m_tState->m_sClientName, m_tState->m_iConnID, m_tState->m_iClientSock );

	if ( !m_tState->m_bKeepSocket )
	{
		m_tmTimeout = tmNow + MS2SEC * g_iReadTimeout;
	} else
	{
		m_tmTimeout = tmNow + MS2SEC * g_iClientTimeout;
	}
	return NE_IN;
}

static const char g_sContentLength[] = "\r\r\n\nCcOoNnTtEeNnTt--LlEeNnGgTtHh";
static const char g_sHeadEnd[] = "\r\n\r\n";

HttpHeaderStreamParser_t::HttpHeaderStreamParser_t ()
{
	m_iHeaderEnd = 0;
	m_iFieldContentLenStart = 0;
	m_iFieldContentLenVal = 0;
	m_iCur = 0;
	m_iCRLF = 0;
	m_iName = 0;
}

bool HttpHeaderStreamParser_t::HeaderFound ( const BYTE * pBuf, int iLen )
{
	// early exit at for already found request header
	if ( m_iHeaderEnd || m_iCur>=iLen )
		return true;

	const int iCNwoLFSize = ( sizeof(g_sContentLength)-5 )/2; // size of just Content-Length field name
	for ( ; m_iCur<iLen; m_iCur++ )
	{
		m_iCRLF = ( pBuf[m_iCur]==g_sHeadEnd[m_iCRLF] ? m_iCRLF+1 : 0 );
		m_iName = ( !m_iFieldContentLenStart && ( pBuf[m_iCur]==g_sContentLength[m_iName] || pBuf[m_iCur]==g_sContentLength[m_iName+1] ) ? m_iName+2 : 0 );

		// header end found
		if ( m_iCRLF==sizeof(g_sHeadEnd)-1 )
		{
			m_iHeaderEnd = m_iCur+1;
			break;
		}
		// Content-Length field found
		if ( !m_iFieldContentLenStart && m_iName==sizeof(g_sContentLength)-1 )
			m_iFieldContentLenStart = m_iCur - iCNwoLFSize + 1;
	}

	// parse Content-Length field value
	while ( m_iHeaderEnd && m_iFieldContentLenStart )
	{
		int iNumStart = m_iFieldContentLenStart + iCNwoLFSize;
		// skip spaces
		while ( iNumStart<m_iHeaderEnd && pBuf[iNumStart]==' ' )
			iNumStart++;
		if ( iNumStart>=m_iHeaderEnd || pBuf[iNumStart]!=':' )
			break;

		iNumStart++; // skip ':' delimiter
		m_iFieldContentLenVal = atoi ( (const char *)pBuf + iNumStart ); // atoi handles leading spaces and tail not digital chars
		break;
	}

	return ( m_iHeaderEnd>0 );
}

NetEvent_e NetReceiveDataHttp_t::Tick ( DWORD uGotEvents, CSphVector<ISphNetAction *> & , CSphNetLoop * pLoop )
{
	assert ( m_tState.Ptr() );
	const char * sHttpError = "http error";
	if ( CheckSocketError ( uGotEvents, sHttpError, m_tState.Ptr(), false ) )
		return NE_REMOVE;

	// loop to handle similar operations at once
	for ( ;; )
	{
		int iRes = NetManageSocket ( m_tState->m_iClientSock, (char *)( m_tState->m_dBuf.Begin() + m_tState->m_iPos ), m_tState->m_iLeft, false );
		if ( iRes==-1 )
		{
			// FIXME!!! report back to client buffer overflow with 413 error
			LogSocketError ( sHttpError, m_tState.Ptr(), false );
			return NE_REMOVE;
		}
		m_tState->m_iLeft -= iRes;
		m_tState->m_iPos += iRes;

		// socket would block - going back to polling
		if ( iRes==0 )
			return NE_KEEP;

		// keep fetching data till the end of a header
		if ( !m_tHeadParser.m_iHeaderEnd )
		{
			if ( !m_tHeadParser.HeaderFound ( m_tState->m_dBuf.Begin(), m_tState->m_iPos ) )
				continue;

			int iReqSize = m_tHeadParser.m_iHeaderEnd + m_tHeadParser.m_iFieldContentLenVal;
			if ( m_tHeadParser.m_iFieldContentLenVal && m_tState->m_iPos<iReqSize )
			{
				m_tState->m_dBuf.Resize ( iReqSize );
				m_tState->m_iLeft = iReqSize - m_tState->m_iPos;
				continue;
			}

			m_tState->m_iLeft = 0;
			m_tState->m_iPos = iReqSize;
			m_tState->m_dBuf.Resize ( iReqSize + 1 );
			m_tState->m_dBuf[m_tState->m_iPos] = '\0';
			m_tState->m_dBuf.Resize ( iReqSize );
		}

		sphLogDebugv ( "%p HTTP buf=%d, header=%d, content-len=%d, sock=%d", this, m_tState->m_dBuf.GetLength(), m_tHeadParser.m_iHeaderEnd, m_tHeadParser.m_iFieldContentLenVal, m_iSock );

		// no VIP for http for now
		ThdJobHttp_t * pJob = new ThdJobHttp_t ( pLoop, m_tState.LeakPtr() );
		g_pThdPool->AddJob ( pJob );

		return NE_REMOVE;
	}
}

void NetReceiveDataHttp_t::CloseSocket()
{
	if ( m_tState.Ptr() )
		m_tState->CloseSocket();
}

ThdJobAPI_t::ThdJobAPI_t ( CSphNetLoop * pLoop, NetStateAPI_t * pState )
	: m_tState ( pState )
	, m_pLoop ( pLoop )
{
	assert ( m_tState.Ptr() );
}

void ThdJobAPI_t::Call ()
{
	SphCrashLogger_c tQueryTLS;
	tQueryTLS.SetupTLS ();

	sphLogDebugv ( "%p API job started, command=%d", this, m_iCommand );

	int iTid = GetOsThreadId();

	ThdDesc_t tThdDesc;
	tThdDesc.m_eProto = PROTO_SPHINX;
	tThdDesc.m_iClientSock = m_tState->m_iClientSock;
	tThdDesc.m_sClientName = m_tState->m_sClientName;
	tThdDesc.m_iConnID = m_tState->m_iConnID;
	tThdDesc.m_tmConnect = sphMicroTimer();
	tThdDesc.m_iTid = iTid;

	g_tThdMutex.Lock ();
	g_dThd.Add ( &tThdDesc );
	g_tThdMutex.Unlock ();

	assert ( m_tState.Ptr() );

	// handle that client
	MemInputBuffer_c tBuf ( m_tState->m_dBuf.Begin(), m_tState->m_dBuf.GetLength() );
	ISphOutputBuffer tOut;
	bool bProceed = LoopClientSphinx ( m_iCommand, m_iCommandVer, m_tState->m_dBuf.GetLength(), m_tState->m_sClientName, m_tState->m_iConnID, &tThdDesc, tBuf, tOut, false );
	m_tState->m_bKeepSocket |= bProceed;

	g_tThdMutex.Lock ();
	g_dThd.Remove ( &tThdDesc );
	g_tThdMutex.Unlock ();

	sphLogDebugv ( "%p API job done, command=%d", this, m_iCommand );

	assert ( m_pLoop );
	if ( tOut.GetSentCount() )
	{
		tOut.SwapData ( m_tState->m_dBuf );
		NetSendData_t * pSend = new NetSendData_t ( m_tState.LeakPtr(), PROTO_SPHINX );
		m_pLoop->AddAction ( pSend );
	} else if ( m_tState->m_bKeepSocket ) // no response - switching to receive
	{
		NetReceiveDataAPI_t * pReceive = new NetReceiveDataAPI_t ( m_tState.LeakPtr() );
		pReceive->SetupBodyPhase();
		m_pLoop->AddAction ( pReceive );
	}
}


ThdJobQL_t::ThdJobQL_t ( CSphNetLoop * pLoop, NetStateQL_t * pState )
	: m_tState ( pState )
	, m_pLoop ( pLoop )
{
	assert ( m_tState.Ptr() );
}

void ThdJobQL_t::Call ()
{
	SphCrashLogger_c tQueryTLS;
	tQueryTLS.SetupTLS ();

	int iTid = GetOsThreadId();

	ThdDesc_t tThdDesc;
	tThdDesc.m_eProto = PROTO_MYSQL41;
	tThdDesc.m_iClientSock = m_tState->m_iClientSock;
	tThdDesc.m_sClientName = m_tState->m_sClientName;
	tThdDesc.m_iConnID = m_tState->m_iConnID;
	tThdDesc.m_tmConnect = sphMicroTimer();
	tThdDesc.m_iTid = iTid;

	g_tThdMutex.Lock ();
	g_dThd.Add ( &tThdDesc );
	g_tThdMutex.Unlock ();

	CSphString sQuery; // to keep data alive for SphCrashQuery_c
	bool bProfile = m_tState->m_tSession.m_tVars.m_bProfile; // the current statement might change it

	MemInputBuffer_c tIn ( m_tState->m_dBuf.Begin(), m_tState->m_dBuf.GetLength() );
	ISphOutputBuffer tOut;

	bool bProceed = LoopClientMySQL ( m_tState->m_uPacketID, m_tState->m_tSession, sQuery, m_tState->m_dBuf.GetLength(), bProfile, &tThdDesc, tIn, tOut );
	m_tState->m_bKeepSocket = bProceed;

	if ( bProceed )
	{
		assert ( m_pLoop );
		tOut.SwapData ( m_tState->m_dBuf );
		NetSendData_t * pSend = new NetSendData_t ( m_tState.LeakPtr(), PROTO_MYSQL41 );
		m_pLoop->AddAction ( pSend );
	}

	g_tThdMutex.Lock ();
	g_dThd.Remove ( &tThdDesc );
	g_tThdMutex.Unlock ();
}

ThdJobCleanup_t::ThdJobCleanup_t ( CSphVector<ISphNetAction *> & dCleanup )
{
	m_dCleanup.SwapData ( dCleanup );
	dCleanup.Reserve ( 1000 );
}

ThdJobCleanup_t::~ThdJobCleanup_t ()
{
	Clear();
}

void ThdJobCleanup_t::Call ()
{
	Clear();
}

void ThdJobCleanup_t::Clear ()
{
	ARRAY_FOREACH ( i, m_dCleanup )
		SafeDelete ( m_dCleanup[i] );

	m_dCleanup.Reset();
}

ThdJobHttp_t::ThdJobHttp_t ( CSphNetLoop * pLoop, NetStateQL_t * pState )
	: m_tState ( pState )
	, m_pLoop ( pLoop )
{
	assert ( m_tState.Ptr() );
}

void ThdJobHttp_t::Call ()
{
	SphCrashLogger_c tQueryTLS;
	tQueryTLS.SetupTLS ();

	sphLogDebugv ( "%p http job started, buffer len=%d", this, m_tState->m_dBuf.GetLength() );

	int iTid = GetOsThreadId();

	ThdDesc_t tThdDesc;
	tThdDesc.m_eProto = PROTO_HTTP;
	tThdDesc.m_iClientSock = m_tState->m_iClientSock;
	tThdDesc.m_sClientName = m_tState->m_sClientName;
	tThdDesc.m_iConnID = m_tState->m_iConnID;
	tThdDesc.m_tmConnect = sphMicroTimer();
	tThdDesc.m_iTid = iTid;

	g_tThdMutex.Lock ();
	g_dThd.Add ( &tThdDesc );
	g_tThdMutex.Unlock ();

	assert ( m_tState.Ptr() );

	m_tState->m_bKeepSocket = sphLoopClientHttp ( m_tState->m_dBuf, m_tState->m_iConnID );

	assert ( m_pLoop );
	NetSendData_t * pSend = new NetSendData_t ( m_tState.LeakPtr(), PROTO_HTTP );
	m_pLoop->AddAction ( pSend );

	g_tThdMutex.Lock ();
	g_dThd.Remove ( &tThdDesc );
	g_tThdMutex.Unlock ();
}



/////////////////////////////////////////////////////////////////////////////
// DAEMON OPTIONS
/////////////////////////////////////////////////////////////////////////////


static void ParsePredictedTimeCosts ( const char * p )
{
	// yet another mini-parser!
	// ident=value [, ident=value [...]]
	while ( *p )
	{
		// parse ident
		while ( sphIsSpace(*p) )
			p++;
		if ( !*p )
			break;
		if ( !sphIsAlpha(*p) )
			sphDie ( "predicted_time_costs: parse error near '%s' (identifier expected)", p );
		const char * q = p;
		while ( sphIsAlpha(*p) )
			p++;
		CSphString sIdent;
		sIdent.SetBinary ( q, p-q );
		sIdent.ToLower();

		// parse =value
		while ( sphIsSpace(*p) )
			p++;
		if ( *p!='=' )
			sphDie ( "predicted_time_costs: parse error near '%s' (expected '=' sign)", p );
		p++;
		while ( sphIsSpace(*p) )
			p++;
		if ( *p<'0' || *p>'9' )
			sphDie ( "predicted_time_costs: parse error near '%s' (number expected)", p );
		q = p;
		while ( *p>='0' && *p<='9' )
			p++;
		CSphString sValue;
		sValue.SetBinary ( q, p-q );
		int iValue = atoi ( sValue.cstr() );

		// parse comma
		while ( sphIsSpace(*p) )
			p++;
		if ( *p && *p!=',' )
			sphDie ( "predicted_time_costs: parse error near '%s' (expected ',' or end of line)", p );
		p++;

		// bind value
		if ( sIdent=="skip" )
			g_iPredictorCostSkip = iValue;
		else if ( sIdent=="doc" )
			g_iPredictorCostDoc = iValue;
		else if ( sIdent=="hit" )
			g_iPredictorCostHit = iValue;
		else if ( sIdent=="match" )
			g_iPredictorCostMatch = iValue;
		else
			sphDie ( "predicted_time_costs: unknown identifier '%s' (known ones are skip, doc, hit, match)", sIdent.cstr() );
	}
}


void ConfigureSearchd ( const CSphConfig & hConf, bool bOptPIDFile )
{
	if ( !hConf.Exists ( "searchd" ) || !hConf["searchd"].Exists ( "searchd" ) )
		sphFatal ( "'searchd' config section not found in '%s'", g_sConfigFile.cstr () );

	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];

	if ( !hConf.Exists ( "index" ) )
		sphFatal ( "no indexes found in '%s'", g_sConfigFile.cstr () );

	sphCheckDuplicatePaths ( hConf );

	if ( bOptPIDFile )
		if ( !hSearchd ( "pid_file" ) )
			sphFatal ( "mandatory option 'pid_file' not found in 'searchd' section" );

	if ( hSearchd.Exists ( "read_timeout" ) && hSearchd["read_timeout"].intval()>=0 )
		g_iReadTimeout = hSearchd["read_timeout"].intval();

	if ( hSearchd.Exists ( "sphinxql_timeout" ) && hSearchd["sphinxql_timeout"].intval()>=0 )
		g_iClientQlTimeout = hSearchd["sphinxql_timeout"].intval();

	if ( hSearchd.Exists ( "client_timeout" ) && hSearchd["client_timeout"].intval()>=0 )
		g_iClientTimeout = hSearchd["client_timeout"].intval();

	if ( hSearchd.Exists ( "max_children" ) && hSearchd["max_children"].intval()>=0 )
	{
		g_iMaxChildren = hSearchd["max_children"].intval();
	}

	if ( hSearchd.Exists ( "persistent_connections_limit" ) && hSearchd["persistent_connections_limit"].intval()>=0 )
		g_iPersistentPoolSize = hSearchd["persistent_connections_limit"].intval();

	g_bPreopenIndexes = hSearchd.GetInt ( "preopen_indexes", (int)g_bPreopenIndexes )!=0;
	sphSetUnlinkOld ( hSearchd.GetInt ( "unlink_old", 1 )!=0 );
	g_iExpansionLimit = hSearchd.GetInt ( "expansion_limit", 0 );
	g_bOnDiskAttrs = ( hSearchd.GetInt ( "ondisk_attrs_default", 0 )==1 );
	g_bOnDiskPools = ( strcmp ( hSearchd.GetStr ( "ondisk_attrs_default", "" ), "pool" )==0 );

	if ( hSearchd("subtree_docs_cache") )
		g_iMaxCachedDocs = hSearchd.GetSize ( "subtree_docs_cache", g_iMaxCachedDocs );

	if ( hSearchd("subtree_hits_cache") )
		g_iMaxCachedHits = hSearchd.GetSize ( "subtree_hits_cache", g_iMaxCachedHits );

	if ( hSearchd("seamless_rotate") )
		g_bSeamlessRotate = ( hSearchd["seamless_rotate"].intval()!=0 );

	if ( !g_bSeamlessRotate && g_bPreopenIndexes )
		sphWarning ( "preopen_indexes=1 has no effect with seamless_rotate=0" );

	g_iAttrFlushPeriod = hSearchd.GetInt ( "attr_flush_period", g_iAttrFlushPeriod );
	g_iMaxPacketSize = hSearchd.GetSize ( "max_packet_size", g_iMaxPacketSize );
	g_iMaxFilters = hSearchd.GetInt ( "max_filters", g_iMaxFilters );
	g_iMaxFilterValues = hSearchd.GetInt ( "max_filter_values", g_iMaxFilterValues );
	g_iMaxBatchQueries = hSearchd.GetInt ( "max_batch_queries", g_iMaxBatchQueries );
	g_iDistThreads = hSearchd.GetInt ( "dist_threads", g_iDistThreads );
	g_tRtThrottle.m_iMaxIOps = hSearchd.GetInt ( "rt_merge_iops", 0 );
	g_tRtThrottle.m_iMaxIOSize = hSearchd.GetSize ( "rt_merge_maxiosize", 0 );
	g_iPingInterval = hSearchd.GetInt ( "ha_ping_interval", 1000 );
	g_uHAPeriodKarma = hSearchd.GetInt ( "ha_period_karma", 60 );
	g_iQueryLogMinMsec = hSearchd.GetInt ( "query_log_min_msec", g_iQueryLogMinMsec );
	g_iAgentConnectTimeout = hSearchd.GetInt ( "agent_connect_timeout", g_iAgentConnectTimeout );
	g_iAgentQueryTimeout = hSearchd.GetInt ( "agent_query_timeout", g_iAgentQueryTimeout );
	g_iAgentRetryDelay = hSearchd.GetInt ( "agent_retry_delay", g_iAgentRetryDelay );
	if ( g_iAgentRetryDelay > MAX_RETRY_DELAY )
		sphWarning ( "agent_retry_delay %d exceeded max recommended %d", g_iAgentRetryDelay, MAX_RETRY_DELAY );
	g_iAgentRetryCount = hSearchd.GetInt ( "agent_retry_count", g_iAgentRetryCount );
	if ( g_iAgentRetryCount > MAX_RETRY_COUNT )
		sphWarning ( "agent_retry_count %d exceeded max recommended %d", g_iAgentRetryCount, MAX_RETRY_COUNT );
	g_tmWait = hSearchd.GetInt ( "net_wait_tm", g_tmWait );
	g_iThrottleAction = hSearchd.GetInt ( "net_throttle_action", g_iThrottleAction );
	g_iThrottleAccept = hSearchd.GetInt ( "net_throttle_accept", g_iThrottleAccept );
	g_iNetWorkers = hSearchd.GetInt ( "net_workers", g_iNetWorkers );
	g_iNetWorkers = Max ( g_iNetWorkers, 1 );

	if ( hSearchd ( "collation_libc_locale" ) )
	{
		const char * sLocale = hSearchd.GetStr ( "collation_libc_locale" );
		if ( !setlocale ( LC_COLLATE, sLocale ) )
			sphWarning ( "setlocale failed (locale='%s')", sLocale );
	}

	if ( hSearchd ( "collation_server" ) )
	{
		CSphString sCollation = hSearchd.GetStr ( "collation_server" );
		CSphString sError;
		g_eCollation = sphCollationFromName ( sCollation, &sError );
		if ( !sError.IsEmpty() )
			sphWarning ( "%s", sError.cstr() );
	}

	if ( hSearchd("thread_stack") )
	{
		int iThreadStackSizeMin = 65536;
		int iThreadStackSizeMax = 8*1024*1024;
		int iStackSize = hSearchd.GetSize ( "thread_stack", iThreadStackSizeMin );
		if ( iStackSize<iThreadStackSizeMin || iStackSize>iThreadStackSizeMax )
			sphWarning ( "thread_stack %d out of bounds (64K..8M); clamped", iStackSize );

		iStackSize = Min ( iStackSize, iThreadStackSizeMax );
		iStackSize = Max ( iStackSize, iThreadStackSizeMin );
		sphSetMyStackSize ( iStackSize );
	}

	if ( hSearchd("predicted_time_costs") )
		ParsePredictedTimeCosts ( hSearchd["predicted_time_costs"].cstr() );

	if ( hSearchd("shutdown_timeout") )
	{
		int iTimeout = hSearchd.GetInt ( "shutdown_timeout", 0 );
		if ( iTimeout )
			g_iShutdownTimeout = iTimeout * 1000000;
	}

	QcacheStatus_t s = QcacheGetStatus();
	s.m_iMaxBytes = hSearchd.GetSize64 ( "qcache_max_bytes", s.m_iMaxBytes );
	s.m_iThreshMsec = hSearchd.GetInt ( "qcache_thresh_msec", s.m_iThreshMsec );
	s.m_iTtlSec = hSearchd.GetInt ( "qcache_ttl_sec", s.m_iTtlSec );
	QcacheSetup ( s.m_iMaxBytes, s.m_iThreshMsec, s.m_iTtlSec );

	//////////////////////////////////////////////////
	// prebuild MySQL wire protocol handshake packets
	//////////////////////////////////////////////////

	char sHandshake1[] =
		"\x00\x00\x00" // packet length
		"\x00" // packet id
		"\x0A"; // protocol version; v.10

	char sHandshake2[] =
		"\x01\x00\x00\x00" // thread id
		"\x01\x02\x03\x04\x05\x06\x07\x08" // scramble buffer (for auth)
		"\x00" // filler
		"\x08\x82" // server capabilities low WORD; CLIENT_PROTOCOL_41 | CLIENT_CONNECT_WITH_DB | CLIENT_SECURE_CONNECTION
		"\x21" // server language; let it be ut8_general_ci to make different clients happy
		"\x02\x00" // server status
		"\x00\x00" // server capabilities hi WORD; no CLIENT_PLUGIN_AUTH
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // filler
		"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d"; // scramble buffer2 (for auth, 4.1+)

	const char * sVersion = hSearchd.GetStr ( "mysql_version_string", SPHINX_VERSION );
	int iLen = strlen ( sVersion );

	g_iMysqlHandshake = sizeof(sHandshake1) + strlen(sVersion) + sizeof(sHandshake2) - 1;
	if ( g_iMysqlHandshake>=(int)sizeof(g_sMysqlHandshake) )
	{
		sphWarning ( "mysql_version_string too long; using default (version=%s)", SPHINX_VERSION );
		g_iMysqlHandshake = sizeof(sHandshake1) + strlen(SPHINX_VERSION) + sizeof(sHandshake2) - 1;
		assert ( g_iMysqlHandshake < (int)sizeof(g_sMysqlHandshake) );
	}

	char * p = g_sMysqlHandshake;
	memcpy ( p, sHandshake1, sizeof(sHandshake1)-1 );
	memcpy ( p+sizeof(sHandshake1)-1, sVersion, iLen+1 );
	memcpy ( p+sizeof(sHandshake1)+iLen, sHandshake2, sizeof(sHandshake2)-1 );
	g_sMysqlHandshake[0] = (char)(g_iMysqlHandshake-4); // safe, as long as buffer size is 128
}

void ConfigureAndPreload ( const CSphConfig & hConf, const CSphVector<const char *> & dOptIndexes )
{
	int iCounter = 1;
	int iValidIndexes = 0;
	int64_t tmLoad = -sphMicroTimer();

	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext() )
	{
		const CSphConfigSection & hIndex = hConf["index"].IterateGet();
		const char * sIndexName = hConf["index"].IterateGetKey().cstr();

		if ( g_bOptNoDetach && dOptIndexes.GetLength()!=0 )
		{
			bool bSkipIndex = true;

			ARRAY_FOREACH_COND ( i, dOptIndexes, bSkipIndex )
				if ( !strcasecmp ( sIndexName, dOptIndexes[i] ) )
					bSkipIndex = false;

			if ( bSkipIndex )
				continue;
		}

		ESphAddIndex eAdd = AddIndex ( sIndexName, hIndex );
		if ( eAdd==ADD_LOCAL || eAdd==ADD_RT )
		{
			ServedIndex_c & tIndex = g_pLocalIndexes->GetUnlockedEntry ( sIndexName );
			iCounter++;

			fprintf ( stdout, "precaching index '%s'\n", sIndexName );
			fflush ( stdout );

			if ( HasFiles ( tIndex.m_sIndexPath.cstr(), sphGetExts ( SPH_EXT_TYPE_NEW ) ) )
			{
				tIndex.m_bOnlyNew = !HasFiles ( tIndex.m_sIndexPath.cstr(), sphGetExts ( SPH_EXT_TYPE_CUR ) );
				CSphString sError;
				if ( RotateIndexGreedy ( tIndex, sIndexName, sError ) )
				{
					if ( !sphFixupIndexSettings ( tIndex.m_pIndex, hIndex, sError ) )
					{
						sphWarning ( "index '%s': %s - NOT SERVING", sIndexName, sError.cstr() );
						tIndex.m_bEnabled = false;
					}
				} else
				{
					sphWarning ( "%s", sError.cstr() );
					if ( PrereadNewIndex ( tIndex, hIndex, sIndexName ) )
						tIndex.m_bEnabled = true;
				}
			} else
			{
				tIndex.m_bOnlyNew = false;
				if ( PrereadNewIndex ( tIndex, hIndex, sIndexName ) )
					tIndex.m_bEnabled = true;
			}

			if ( !tIndex.m_bEnabled )
				continue;

			CSphString sError;
			if ( !tIndex.m_sGlobalIDFPath.IsEmpty() )
				if ( !sphPrereadGlobalIDF ( tIndex.m_sGlobalIDFPath, sError ) )
					sphWarning ( "index '%s': global IDF unavailable - IGNORING", sIndexName );
		}

		if ( eAdd!=ADD_ERROR )
			iValidIndexes++;
	}

	InitPersistentPool();

	tmLoad += sphMicroTimer();
	if ( !iValidIndexes )
		sphFatal ( "no valid indexes to serve" );
	else
		fprintf ( stdout, "precached %d indexes in %0.3f sec\n", iCounter-1, float(tmLoad)/1000000 );
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
				sphFatal ( "failed to open log file '%s': %s", sLog, strerror(errno) );
			}
		}

		g_sLogFile = sLog;
		g_bLogTty = isatty ( g_iLogFile )!=0;
}


int WINAPI ServiceMain ( int argc, char **argv )
{
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
		fprintf ( stdout, SPHINX_BANNER );

	const char* sEndian = sphCheckEndian();
	if ( sEndian )
		sphDie ( "%s", sEndian );

	if_const ( sizeof(SphDocID_t)==4 )
		sphWarning ( "32-bit IDs are deprecated, rebuild your binaries with --enable-id64" );

	//////////////////////
	// parse command line
	//////////////////////

	CSphConfig		conf;
	bool			bOptStop = false;
	bool			bOptStopWait = false;
	bool			bOptStatus = false;
	bool			bOptPIDFile = false;
	CSphVector<const char *>	dOptIndexes;

	int				iOptPort = 0;
	bool			bOptPort = false;

	CSphString		sOptListen;
	bool			bOptListen = false;
	bool			bTestMode = false;
	bool			bTestThdPoolMode = false;
	bool			bOptDebugQlog = true;
	bool			bForcedPreread = false;

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
		OPT1 ( "--console" )		{ g_bOptNoLock = true; g_bOptNoDetach = true; bTestMode = true; }
		OPT1 ( "--stop" )			bOptStop = true;
		OPT1 ( "--stopwait" )		{ bOptStop = true; bOptStopWait = true; }
		OPT1 ( "--status" )			bOptStatus = true;
		OPT1 ( "--pidfile" )		bOptPIDFile = true;
		OPT1 ( "--iostats" )		g_bIOStats = true;
#if !USE_WINDOWS
		OPT1 ( "--cpustats" )		g_bCpuStats = true;
#endif
#if USE_WINDOWS
		OPT1 ( "--install" )		{ if ( !g_bService ) { ServiceInstall ( argc, argv ); return 0; } }
		OPT1 ( "--delete" )			{ if ( !g_bService ) { ServiceDelete (); return 0; } }
		OPT1 ( "--ntservice" )		{} // it's valid but handled elsewhere
#else
		OPT1 ( "--nodetach" )		g_bOptNoDetach = true;
#endif
		OPT1 ( "--logdebug" )		g_eLogLevel = SPH_LOG_DEBUG;
		OPT1 ( "--logdebugv" )		g_eLogLevel = SPH_LOG_VERBOSE_DEBUG;
		OPT1 ( "--logdebugvv" )		g_eLogLevel = SPH_LOG_VERY_VERBOSE_DEBUG;
		OPT1 ( "--safetrace" )		g_bSafeTrace = true;
		OPT1 ( "--test" )			{ g_bWatchdog = false; bTestMode = true; } // internal option, do NOT document
		OPT1 ( "--test-thd-pool" )	{ g_bWatchdog = false; bTestMode = true; bTestThdPoolMode = true; } // internal option, do NOT document
		OPT1 ( "--strip-path" )		g_bStripPath = true;
		OPT1 ( "--vtune" )			g_bVtune = true;
		OPT1 ( "--noqlog" )			bOptDebugQlog = false;
		OPT1 ( "--force-preread" )	bForcedPreread = true;

		// FIXME! add opt=(csv)val handling here
		OPT1 ( "--replay-flags=accept-desc-timestamp" )		uReplayFlags |= SPH_REPLAY_ACCEPT_DESC_TIMESTAMP;
		OPT1 ( "--replay-flags=ignore-open-errors" )			uReplayFlags |= SPH_REPLAY_IGNORE_OPEN_ERROR;

		// handle 1-arg options
		else if ( (i+1)>=argc )		break;
		OPT ( "-c", "--config" )	g_sConfigFile = argv[++i];
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

#if USE_WINDOWS
	// init WSA on Windows
	// we need to do it this early because otherwise gethostbyname() from config parser could fail
	WSADATA tWSAData;
	int iStartupErr = WSAStartup ( WINSOCK_VERSION, &tWSAData );
	if ( iStartupErr )
		sphFatal ( "failed to initialize WinSock2: %s", sphSockError ( iStartupErr ) );

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

	// fallback to defaults if there was no explicit config specified
	while ( !g_sConfigFile.cstr() )
	{
#ifdef SYSCONFDIR
		g_sConfigFile = SYSCONFDIR "/sphinx.conf";
		if ( sphIsReadable ( g_sConfigFile.cstr () ) )
			break;
#endif

		g_sConfigFile = "./sphinx.conf";
		if ( sphIsReadable ( g_sConfigFile.cstr () ) )
			break;

		g_sConfigFile = NULL;
		break;
	}

	if ( !g_sConfigFile.cstr () )
		sphFatal ( "no readable config file (looked in "
#ifdef SYSCONFDIR
			SYSCONFDIR "/sphinx.conf, "
#endif
			"./sphinx.conf)." );

	sphInfo ( "using config file '%s'...", g_sConfigFile.cstr () );

	CheckConfigChanges ();

	// do parse
	if ( !g_pCfg.Parse ( g_sConfigFile.cstr () ) )
		sphFatal ( "failed to parse config file '%s'", g_sConfigFile.cstr () );

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
		if ( !hSearchdpre("pid_file") )
			sphFatal ( "stop: option 'pid_file' not found in '%s' section 'searchd'", g_sConfigFile.cstr () );

		const char * sPid = hSearchdpre["pid_file"].cstr(); // shortcut
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

		int iWaitTimeout = g_iShutdownTimeout + 100000;

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
		if ( bOptStopWait )
		{
			sPipeName = GetNamedPipeName ( iPid );
			iPipeCreated = mkfifo ( sPipeName.cstr(), 0666 );
			if ( iPipeCreated!=-1 )
				fdPipe = ::open ( sPipeName.cstr(), O_RDONLY | O_NONBLOCK );

			if ( iPipeCreated==-1 )
				sphWarning ( "mkfifo failed (path=%s, err=%d, msg=%s); will NOT wait", sPipeName.cstr(), errno, strerror(errno) );
			else if ( fdPipe<0 )
				sphWarning ( "open failed (path=%s, err=%d, msg=%s); will NOT wait", sPipeName.cstr(), errno, strerror(errno) );
		}

		if ( kill ( iPid, SIGTERM ) )
			sphFatal ( "stop: kill() on pid %d failed: %s", iPid, strerror(errno) );
		else
			sphInfo ( "stop: successfully sent SIGTERM to pid %d", iPid );

		int iExitCode = ( bOptStopWait && ( iPipeCreated==-1 || fdPipe<0 ) ) ? 1 : 0;
		bool bHandshake = true;
		while ( bOptStopWait && fdPipe>=0 )
		{
			int iReady = sphPoll ( fdPipe, iWaitTimeout );

			// error on wait
			if ( iReady<0 )
			{
				iExitCode = 3;
				sphWarning ( "stopwait%s error '%s'", ( bHandshake ? " handshake" : " " ), strerror(errno) );
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
				sphWarning ( "stopwait read fifo error '%s'", strerror(errno) );
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
		if ( fdPipe>=0 )
			::close ( fdPipe );
		if ( iPipeCreated!=-1 )
			::unlink ( sPipeName.cstr() );

		exit ( iExitCode );
#endif
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

	ConfigureSearchd ( hConf, bOptPIDFile );
	sphConfigureCommon ( hConf ); // this also inits plugins now

	g_bWatchdog = hSearchdpre.GetInt ( "watchdog", g_bWatchdog )!=0;

	bool bThdPool = true;
	if ( hSearchdpre("workers") )
	{
		if ( hSearchdpre["workers"]=="threads" )
		{
			bThdPool = false;
		} else if ( hSearchdpre["workers"]=="thread_pool" )
		{
			bThdPool = true;
		}
	}

	if ( bThdPool || bTestThdPoolMode )
	{
#if HAVE_POLL || HAVE_EPOLL
		bThdPool = true;
#else
		bThdPool = false;
		sphWarning ( "no poll or epoll found, thread pool unavailable, going back to thread workers" );
#endif
	}
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
			sphFatal ( "failed to create pid file '%s': %s", g_sPidFile.scstr(), strerror(errno) );
	}
	if ( bOptPIDFile && !sphLockEx ( g_iPidFD, false ) )
		sphFatal ( "failed to lock pid file '%s': %s (searchd already running?)", g_sPidFile.scstr(), strerror(errno) );

	// Actions on resurrection
	if ( bWatched && !bVisualLoad && CheckConfigChanges() )
	{
		// reparse the config file
		sphInfo ( "Reloading the config" );
		if ( !g_pCfg.ReParse ( g_sConfigFile.cstr () ) )
			sphFatal ( "failed to parse config file '%s'", g_sConfigFile.cstr () );

		sphInfo ( "Reconfigure the daemon" );
		ConfigureSearchd ( hConf, bOptPIDFile );
	}

	// hSearchdpre might be dead if we reloaded the config.
	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];

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
					sphFatal ( "failed to open query log file '%s': %s", hSearchd["query_log"].cstr(), strerror(errno) );
			}
			g_sQueryLogFile = hSearchd["query_log"].cstr();
		}
	}

	////////////////////
	// network startup
	////////////////////

	Listener_t tListener;
	tListener.m_eProto = PROTO_SPHINX;
	tListener.m_bTcp = true;

	// command line arguments override config (but only in --console)
	if ( bOptListen )
	{
		AddListener ( sOptListen, bThdPool );

	} else if ( bOptPort )
	{
		tListener.m_iSock = sphCreateInetSocket ( htonl ( INADDR_ANY ), iOptPort );
		g_dListeners.Add ( tListener );

	} else
	{
		// listen directives in configuration file
		for ( CSphVariant * v = hSearchd("listen"); v; v = v->m_pNext )
			AddListener ( v->strval(), bThdPool );

		// default is to listen on our two ports
		if ( !g_dListeners.GetLength() )
		{
			tListener.m_iSock = sphCreateInetSocket ( htonl ( INADDR_ANY ), SPHINXAPI_PORT );
			tListener.m_eProto = PROTO_SPHINX;
			g_dListeners.Add ( tListener );

			tListener.m_iSock = sphCreateInetSocket ( htonl ( INADDR_ANY ), SPHINXQL_PORT );
			tListener.m_eProto = PROTO_MYSQL41;
			g_dListeners.Add ( tListener );
		}
	}

	//////////////////////
	// build indexes hash
	//////////////////////

	// setup mva updates arena here, since we could have saved persistent mva updates
	const char * sArenaError = sphArenaInit ( hSearchd.GetSize ( "mva_updates_pool", MVA_UPDATES_POOL ) );
	if ( sArenaError )
		sphWarning ( "process shared mutex unsupported, MVA update disabled ( %s )", sArenaError );

	// configure and preload

	ConfigureAndPreload ( hConf, dOptIndexes );

	///////////
	// startup
	///////////

	sphRTInit ( hSearchd, bTestMode );

	if ( hSearchd.Exists ( "snippets_file_prefix" ) )
		g_sSnippetsFilePrefix = hSearchd["snippets_file_prefix"].cstr();
	else
		g_sSnippetsFilePrefix = "";

	const char* sLogFormat = hSearchd.GetStr ( "query_log_format", "plain" );
	if ( !strcmp ( sLogFormat, "sphinxql" ) )
		g_eLogFormat = LOG_FORMAT_SPHINXQL;
	else if ( strcmp ( sLogFormat, "plain" ) )
	{
		CSphVector<CSphString> dParams;
		sphSplit ( dParams, sLogFormat );
		ARRAY_FOREACH ( j, dParams )
		{
			sLogFormat = dParams[j].cstr();
			if ( !strcmp ( sLogFormat, "sphinxql" ) )
				g_eLogFormat = LOG_FORMAT_SPHINXQL;
			else if ( !strcmp ( sLogFormat, "plain" ) )
				g_eLogFormat = LOG_FORMAT_PLAIN;
			else if ( !strcmp ( sLogFormat, "compact_in" ) )
				g_bLogCompactIn = true;
		}
	}
	if ( g_bLogCompactIn && g_eLogFormat==LOG_FORMAT_PLAIN )
		sphWarning ( "compact_in option only supported with query_log_format=sphinxql" );

	// prepare to detach
	if ( !g_bOptNoDetach )
	{
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
		ReleaseTTYFlag();

		// explicitly unlock everything in parent immediately before fork
		//
		// there's a race in case another instance is started before
		// child re-acquires all locks; but let's hope that's rare
		if ( !bWatched )
		{
			for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
			{
				ServedIndex_c & tServed = it.Get();
				if ( tServed.m_bEnabled )
					tServed.m_pIndex->Unlock();
			}
			for ( IndexHashIterator_c it ( g_pTemplateIndexes ); it.Next(); )
			{
				ServedIndex_c & tServed = it.Get();
				if ( tServed.m_bEnabled )
					tServed.m_pIndex->Unlock();
			}
		}
	}

	if ( bOptPIDFile && !bWatched )
		sphLockUn ( g_iPidFD );

#if !USE_WINDOWS
	if ( !g_bOptNoDetach && !bWatched )
	{
		switch ( fork() )
		{
			case -1:
				// error
				sphFatalLog ( "fork() failed (reason: %s)", strerror ( errno ) );
				exit ( 1 );

			case 0:
				// daemonized child
				break;

			default:
				// tty-controlled parent
				sphSetProcessInfo ( false );
				exit ( 0 );
		}
	}
#endif

	sphRTConfigure ( hSearchd, bTestMode );

	if ( bOptPIDFile )
	{
#if !USE_WINDOWS
		// re-lock pid
		// FIXME! there's a potential race here
		if ( !sphLockEx ( g_iPidFD, true ) )
			sphFatal ( "failed to re-lock pid file '%s': %s", g_sPidFile.scstr(), strerror(errno) );
#endif

		char sPid[16];
		snprintf ( sPid, sizeof(sPid), "%d\n", (int)getpid() );
		int iPidLen = strlen(sPid);

		sphSeek ( g_iPidFD, 0, SEEK_SET );
		if ( !sphWrite ( g_iPidFD, sPid, iPidLen ) )
			sphFatal ( "failed to write to pid file '%s' (errno=%d, msg=%s)", g_sPidFile.scstr(),
				errno, strerror(errno) );

		if ( ::ftruncate ( g_iPidFD, iPidLen ) )
			sphFatal ( "failed to truncate pid file '%s' (errno=%d, msg=%s)", g_sPidFile.scstr(),
				errno, strerror(errno) );
	}

#if USE_WINDOWS
	SetConsoleCtrlHandler ( CtrlHandler, TRUE );
#endif

	if ( !g_bOptNoDetach && !bWatched )
	{
		// re-lock indexes
		for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
		{
			ServedIndex_c & tServed = it.Get();
			if ( !tServed.m_bEnabled )
				continue;

			// obtain exclusive lock
			if ( !tServed.m_pIndex->Lock() )
			{
				sphWarning ( "index '%s': lock: %s; INDEX UNUSABLE", it.GetKey().cstr(),
					tServed.m_pIndex->GetLastError().cstr() );
				tServed.m_bEnabled = false;
				continue;
			}
		}
	}

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

	if ( bThdPool )
	{
		g_iThdQueueMax = hSearchd.GetInt ( "queue_max_length", g_iThdQueueMax );
		g_iThdPoolCount = Max ( 3*sphCpuThreadsCount()/2, 2 ); // default to 1.5*detected_cores but not less than 2 worker threads
		if ( hSearchd.Exists ( "max_children" ) && hSearchd["max_children"].intval()>=0 )
			g_iThdPoolCount = hSearchd["max_children"].intval();

		g_pThdPool = sphThreadPoolCreate ( g_iThdPoolCount );
	}
#if USE_WINDOWS
	if ( g_bService )
		MySetServiceStatus ( SERVICE_RUNNING, NO_ERROR, 0 );
#endif

	sphSetReadBuffers ( hSearchd.GetSize ( "read_buffer", 0 ), hSearchd.GetSize ( "read_unhinted", 0 ) );

	// in threaded mode, create a dedicated rotation thread
	if ( g_bSeamlessRotate && !sphThreadCreate ( &g_tRotateThread, RotationThreadFunc, 0 ) )
		sphDie ( "failed to create rotation thread" );

	// replay last binlog
	SmallStringHash_T<CSphIndex*> hIndexes;
	for ( IndexHashIterator_c it ( g_pLocalIndexes ); it.Next(); )
		if ( it.Get().m_bEnabled )
			hIndexes.Add ( it.Get().m_pIndex, it.GetKey() );

	sphReplayBinlog ( hIndexes, uReplayFlags, DumpMemStat );
	hIndexes.Reset();

	if ( g_bIOStats && !sphInitIOStats () )
		sphWarning ( "unable to init IO statistics" );

	g_tStats.m_uStarted = (DWORD)time(NULL);

	// threads mode
	// create optimize and flush threads, and load saved sphinxql state
	if ( !sphThreadCreate ( &g_tRtFlushThread, RtFlushThreadFunc, 0 ) )
		sphDie ( "failed to create rt-flush thread" );

	if ( !sphThreadCreate ( &g_tOptimizeThread, OptimizeThreadFunc, 0 ) )
		sphDie ( "failed to create optimize thread" );

	g_sSphinxqlState = hSearchd.GetStr ( "sphinxql_state" );
	if ( !g_sSphinxqlState.IsEmpty() )
	{
		SphinxqlStateRead ( g_sSphinxqlState );
		g_tmSphinxqlState = sphMicroTimer();

		CSphString sError;
		CSphWriter tWriter;
		CSphString sNewState;
		sNewState.SetSprintf ( "%s.new", g_sSphinxqlState.cstr() );
		// initial check that work can be done
		bool bCanWrite = tWriter.OpenFile ( sNewState, sError );
		tWriter.CloseFile();
		::unlink ( sNewState.cstr() );

		if ( !bCanWrite )
			sphWarning ( "sphinxql_state flush disabled: %s", sError.cstr() );
		else if ( !sphThreadCreate ( &g_tSphinxqlStateFlushThread, SphinxqlStateThreadFunc, NULL ) )
			sphDie ( "failed to create sphinxql_state writer thread" );
	}

	if ( !sphThreadCreate ( &g_tRotationServiceThread, RotationServiceThreadFunc, 0 ) )
		sphDie ( "failed to create rotation service thread" );

	if ( bForcedPreread )
	{
		PrereadFunc ( NULL );
	} else
	{
		SphThread_t tTmpPreread;
		if ( !sphThreadCreate ( &tTmpPreread, PrereadFunc, 0, true ) )
			sphWarning ( "failed to create preread thread" );
	}

	// almost ready, time to start listening
	g_iBacklog = hSearchd.GetInt ( "listen_backlog", g_iBacklog );
	ARRAY_FOREACH ( j, g_dListeners )
		if ( listen ( g_dListeners[j].m_iSock, g_iBacklog )==-1 )
			sphFatal ( "listen() failed: %s", sphSockError() );

	if ( g_pThdPool )
	{
		// net thread needs non-blocking sockets
		ARRAY_FOREACH ( i, g_dListeners )
		{
			if ( sphSetSockNB ( g_dListeners[i].m_iSock )<0 )
			{
				sphWarning ( "sphSetSockNB() failed: %s", sphSockError() );
				sphSockClose ( g_dListeners[i].m_iSock );
			}
		}

		g_dTickPoolThread.Resize ( g_iNetWorkers );
		ARRAY_FOREACH ( i, g_dTickPoolThread )
		{
			if ( !sphThreadCreate ( g_dTickPoolThread.Begin()+i, CSphNetLoop::ThdTick, 0 ) )
				sphDie ( "failed to create tick pool thread" );
		}
	}

	// crash logging for the main thread (for --console case)
	SphCrashLogger_c tQueryTLS;
	tQueryTLS.SetupTLS ();

	// ready, steady, go
	sphInfo ( "accepting connections" );

	// disable startup logging to stdout
	if ( !g_bOptNoDetach )
		g_bLogStdout = false;

	for ( ;; )
	{
		SphCrashLogger_c::SetupTimePID();
		TickHead();
	}
} // NOLINT ServiceMain() function length


bool DieCallback ( const char * sMessage )
{
	sphLogFatal ( "%s", sMessage );
	return false; // caller should not log
}


UservarIntSet_c * UservarsHook ( const CSphString & sUservar )
{
	CSphScopedLock<CSphMutex> tLock ( g_tUservarsMutex );

	Uservar_t * pVar = g_hUservars ( sUservar );
	if ( !pVar )
		return NULL;

	assert ( pVar->m_eType==USERVAR_INT_SET );
	pVar->m_pVal->AddRef();
	return pVar->m_pVal;
}


int main ( int argc, char **argv )
{
	// threads should be initialized before memory allocations
	char cTopOfMainStack;
	sphThreadInit();
	MemorizeStack ( &cTopOfMainStack );

	sphSetDieCallback ( DieCallback );
	sphSetLogger ( sphLog );
	g_pUservarsHook = UservarsHook;
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
	} else
#endif

	return ServiceMain ( argc, argv );
}

//
// $Id$
//
