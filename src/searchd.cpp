//
// $Id$
//

//
// Copyright (c) 2001-2010, Andrew Aksyonoff
// Copyright (c) 2008-2010, Sphinx Technologies Inc
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

#define SEARCHD_BACKLOG			5
#define SPHINXAPI_PORT			9312
#define SPHINXQL_PORT			9306
#define SPH_ADDRESS_SIZE		sizeof("000.000.000.000")

/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
	// Win-specific headers and calls
	#include <io.h>
	#include <winsock2.h>
	#include <tlhelp32.h>

	#define sphSockRecv(_sock,_buf,_len)	::recv(_sock,_buf,_len,0)
	#define sphSockSend(_sock,_buf,_len)	::send(_sock,_buf,_len,0)
	#define sphSockClose(_sock)				::closesocket(_sock)

	#define stat		_stat

#else
	// UNIX-specific headers and calls
	#include <unistd.h>
	#include <netinet/in.h>
	#include <sys/file.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/wait.h>
	#include <sys/un.h>
	#include <netdb.h>

	// there's no MSG_NOSIGNAL on OS X
	#ifndef MSG_NOSIGNAL
	#define MSG_NOSIGNAL 0
	#endif

	#define sphSockRecv(_sock,_buf,_len)	::recv(_sock,_buf,_len,MSG_NOSIGNAL)
	#define sphSockSend(_sock,_buf,_len)	::send(_sock,_buf,_len,MSG_NOSIGNAL)
	#define sphSockClose(_sock)				::close(_sock)

#endif

/////////////////////////////////////////////////////////////////////////////
// MISC GLOBALS
/////////////////////////////////////////////////////////////////////////////

struct ServedIndex_t
{
	CSphIndex *			m_pIndex;
	CSphString			m_sIndexPath;
	bool				m_bEnabled;		///< to disable index in cases when rotation fails
	bool				m_bMlock;
	bool				m_bPreopen;
	bool				m_bOnDiskDict;
	bool				m_bStar;
	bool				m_bExpand;
	bool				m_bToDelete;
	bool				m_bOnlyNew;
	int					m_iUpdateTag;
	bool				m_bRT;

public:
						ServedIndex_t ();
						~ServedIndex_t ();
	void				Reset ();

	void				ReadLock () const;
	void				WriteLock () const;
	void				Unlock () const;

private:
	mutable CSphRwlock	m_tLock;
};

/////////////////////////////////////////////////////////////////////////////

enum ESphAddIndex
{
	ADD_ERROR	= 0,
	ADD_LOCAL	= 1,
	ADD_DISTR	= 2,
	ADD_RT		= 3
};


enum ProtocolType_e
{
	PROTO_SPHINX = 0,
	PROTO_MYSQL41,

	PROTO_TOTAL
};


const char * g_dProtoNames[PROTO_TOTAL] =
{
	"sphinxapi", "sphinxql"
};


static bool				g_bService		= false;
#if USE_WINDOWS
static bool				g_bServiceStop	= false;
static const char *		g_sServiceName	= "searchd";
HANDLE					g_hPipe			= INVALID_HANDLE_VALUE;
#endif

static CSphVector<CSphString>	g_dArgs;

static bool				g_bHeadDaemon	= false;
static bool				g_bLogStdout	= true;

class SphCrashLogger_c
{
public:
	SphCrashLogger_c ();

	static void Init ();
	static void Done ();

#if !USE_WINDOWS
	static void HandleCrash ( int );
#else
	static LONG WINAPI HandleCrash ( EXCEPTION_POINTERS * pExc );
#endif
	static void SetLastQuery ( const BYTE * pQuery, int iSize, bool bMySQL, int uCmd=0, int uVer=0 );
	static void SetupTimePID ();
	void SetupTLS ();

private:
	const BYTE *			m_pQuery;	// last query
	int						m_iSize;	// last query size
	WORD					m_uCMD;		// last command (header)
	WORD					m_uVer;		// last command's version (header)
	bool					m_bMySQL;	// is query from MySQL or API

	static SphCrashLogger_c	m_tLastQuery; // non threaded mode last query
	static SphThreadKey_t	m_tLastQueryTLS; // threaded mode last query
};

static ESphLogLevel		g_eLogLevel		= LOG_INFO;
static int				g_iLogFile		= STDOUT_FILENO;	// log file descriptor
static CSphString		g_sLogFile;							// log file name
static bool				g_bLogTty		= false;			// cached isatty(g_iLogFile)

static int				g_iReadTimeout	= 5;	// sec
static int				g_iWriteTimeout	= 5;
static int				g_iClientTimeout = 300;
static int				g_iChildren		= 0;
static int				g_iMaxChildren	= 0;
static bool				g_bPreopenIndexes = false;
static bool				g_bOnDiskDicts	= false;
static bool				g_bUnlinkOld	= true;
static bool				g_bWatchdog		= true;
static int				g_iExpansionLimit = 0;
static bool				g_bCompatResults = true;

struct Listener_t
{
	int					m_iSock;
	ProtocolType_e		m_eProto;
};
static CSphVector<Listener_t>	g_dListeners;

static int				g_iQueryLogFile	= -1;
static CSphString		g_sQueryLogFile;
static const char *		g_sPidFile		= NULL;
static int				g_iPidFD		= -1;
static int				g_iMaxMatches	= 1000;

static int				g_iMaxCachedDocs	= 0;	// in bytes
static int				g_iMaxCachedHits	= 0;	// in bytes

static int				g_iAttrFlushPeriod	= 0;			// in seconds; 0 means "do not flush"
static int				g_iMaxPacketSize	= 8*1024*1024;	// in bytes; for both query packets from clients and response packets from agents
static int				g_iMaxFilters		= 256;
static int				g_iMaxFilterValues	= 4096;
static int				g_iMaxBatchQueries	= 32;

enum Mpm_e
{
	MPM_NONE,		///< process queries in a loop one by one (eg. in --console)
	MPM_FORK,		///< fork a worker process for each query
	MPM_PREFORK,	///< keep a number of pre-forked processes
	MPM_THREADS		///< create a worker thread for each query
};

static Mpm_e			g_eWorkers			= USE_WINDOWS ? MPM_THREADS : MPM_FORK;

static int				g_iPreforkChildren	= 10;		// how much workers to keep
static CSphVector<int>	g_dChildren;
static volatile bool	g_bAcceptUnlocked	= true;		// whether this preforked child is guaranteed to be *not* holding a lock around accept
static int				g_iClientFD			= -1;
static int				g_iDistThreads		= 0;

enum ThdState_e
{
	THD_HANDSHAKE,
	THD_NET_READ,
	THD_NET_WRITE,
	THD_QUERY,

	THD_STATE_TOTAL
};

const char * g_dThdStates[THD_STATE_TOTAL] = {
	"handshake", "net_read", "net_write", "query"
};

struct ThdDesc_t
{
	SphThread_t		m_tThd;
	ProtocolType_e	m_eProto;
	int				m_iClientSock;
	CSphString		m_sClientName;

	ThdState_e		m_eThdState;
	const char *	m_sCommand;

	ThdDesc_t ()
		: m_iClientSock ( 0 )
		, m_sCommand ( NULL )
	{}
};

static CSphStaticMutex			g_tThdMutex;
static CSphVector<ThdDesc_t*>	g_dThd;			///< existing threads table

//////////////////////////////////////////////////////////////////////////

static CSphString		g_sConfigFile;
static DWORD			g_uCfgCRC32		= 0;
static struct stat		g_tCfgStat;

static CSphConfigParser * g_pCfg			= NULL;

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

static volatile bool	g_bDoDelete			= false;	// do we need to delete any indexes?
static volatile bool	g_bDoRotate			= false;	// flag that we are rotating now; set from SIGHUP; cleared on rotation success
static volatile bool	g_bGotSighup		= false;	// we just received SIGHUP; need to log
static volatile bool	g_bGotSigterm		= false;	// we just received SIGTERM; need to shutdown
static volatile bool	g_bGotSigchld		= false;	// we just received SIGCHLD; need to count dead children
static volatile bool	g_bGotSigusr1		= false;	// we just received SIGUSR1; need to reopen logs


/// global index hash
/// used in both non-threaded and multi-threaded modes
///
/// hash entry is a CSphIndex pointer, rwlock, and a few flags (see ServedIndex_t)
/// rlock on entry guarantees it won't change, eg. that index pointer will stay alive
/// wlock on entry allows to change (delete/replace) the index pointer
///
/// note that entry locks are held outside the hash
/// and Delete() honours that by acquiring wlock on an entry first
class IndexHash_c : protected SmallStringHash_T<ServedIndex_t>
{
	friend class IndexHashIterator_c;
	typedef SmallStringHash_T<ServedIndex_t> BASE;

public:
	explicit				IndexHash_c ();
	virtual					~IndexHash_c ();

	int						GetLength () const { return BASE::GetLength(); }
	void					Reset () { BASE::Reset(); }

	bool					Add ( const ServedIndex_t & tValue, const CSphString & tKey );
	bool					Delete ( const CSphString & tKey );

	const ServedIndex_t *	GetRlockedEntry ( const CSphString & tKey ) const;
	ServedIndex_t *			GetWlockedEntry ( const CSphString & tKey ) const;
	ServedIndex_t &			GetUnlockedEntry ( const CSphString & tKey ) const;
	bool					Exists ( const CSphString & tKey ) const;

protected:
	void					Rlock () const;
	void					Wlock () const;
	void					Unlock () const;

private:
	mutable CSphRwlock		m_tLock;
};


/// multi-threaded hash iterator
class IndexHashIterator_c : public ISphNoncopyable
{
public:
	explicit			IndexHashIterator_c ( const IndexHash_c * pHash, bool bWrite=false );
						~IndexHashIterator_c ();

	bool				Next ();
	ServedIndex_t &		Get ();
	const CSphString &	GetKey ();

private:
	const IndexHash_c *			m_pHash;
	IndexHash_c::HashEntry_t *	m_pIterator;
};


static IndexHash_c *						g_pIndexes = NULL;		// served indexes hash
static CSphVector<const char *>				g_dRotating;			// names of indexes to be rotated this time
static const char *							g_sPrereading	= NULL;	// name of index currently being preread
static CSphIndex *							g_pPrereading	= NULL;	// rotation "buffer"

static CSphMutex							g_tRotateQueueMutex;
static CSphVector<CSphString>				g_dRotateQueue;		// FIXME? maybe replace it with lockless ring buffer
static CSphMutex							g_tRotateConfigMutex;
static SphThread_t							g_tRotateThread;
static volatile bool						g_bRotateShutdown = false;

/// flush parameters of rt indexes
static SphThread_t							g_tRtFlushThread;
static volatile bool						g_bRtFlushShutdown = false;

struct DistributedMutex_t
{
	void Init ();
	void Done ();
	void Lock ();
	void Unlock ();

private:
	CSphMutex m_tLock;
};
static DistributedMutex_t					g_tDistLock;

enum
{
	SPH_PIPE_UPDATED_ATTRS,
	SPH_PIPE_SAVED_ATTRS,
	SPH_PIPE_PREREAD
};

struct PipeInfo_t
{
	int		m_iFD;			///< read-pipe to child
	int		m_iHandler;		///< who's my handler (SPH_PIPE_xxx)

	PipeInfo_t () : m_iFD ( -1 ), m_iHandler ( -1 ) {}
};

static CSphVector<PipeInfo_t>	g_dPipes;		///< currently open read-pipes to children processes

struct PoolPtrs_t
{
	const DWORD *	m_pMva;
	const BYTE *	m_pStrings;

	PoolPtrs_t ()
		: m_pMva ( NULL )
		, m_pStrings ( NULL )
	{}
};

/////////////////////////////////////////////////////////////////////////////

/// known commands
enum SearchdCommand_e
{
	SEARCHD_COMMAND_SEARCH		= 0,
	SEARCHD_COMMAND_EXCERPT		= 1,
	SEARCHD_COMMAND_UPDATE		= 2,
	SEARCHD_COMMAND_KEYWORDS	= 3,
	SEARCHD_COMMAND_PERSIST		= 4,
	SEARCHD_COMMAND_STATUS		= 5,
	SEARCHD_COMMAND_QUERY		= 6,
	SEARCHD_COMMAND_FLUSHATTRS	= 7,

	SEARCHD_COMMAND_TOTAL
};


/// known command versions
enum
{
	VER_COMMAND_SEARCH		= 0x117,
	VER_COMMAND_EXCERPT		= 0x103,
	VER_COMMAND_UPDATE		= 0x102,
	VER_COMMAND_KEYWORDS	= 0x100,
	VER_COMMAND_STATUS		= 0x100,
	VER_COMMAND_QUERY		= 0x100,
	VER_COMMAND_FLUSHATTRS	= 0x100
};


/// known status return codes
enum SearchdStatus_e
{
	SEARCHD_OK		= 0,	///< general success, command-specific reply follows
	SEARCHD_ERROR	= 1,	///< general failure, error message follows
	SEARCHD_RETRY	= 2,	///< temporary failure, error message follows, client should retry later
	SEARCHD_WARNING	= 3		///< general success, warning message and command-specific reply follow
};


/// command names
const char * g_dApiCommands[SEARCHD_COMMAND_TOTAL] =
{
	"search", "excerpt", "update", "keywords", "persist", "status", "query", "flushattrs"
};

const int	MAX_RETRY_COUNT		= 8;
const int	MAX_RETRY_DELAY		= 1000;

//////////////////////////////////////////////////////////////////////////

const int	STATS_MAX_AGENTS	= 1024;	///< we'll track stats for this much remote agents

/// per-agent query stats
struct AgentStats_t
{
	int64_t		m_iTimeoutsQuery;	///< number of time-outed queries
	int64_t		m_iTimeoutsConnect;	///< number of time-outed connections
	int64_t		m_iConnectFailures;	///< failed to connect
	int64_t		m_iNetworkErrors;	///< network error
	int64_t		m_iWrongReplies;	///< incomplete reply
	int64_t		m_iUnexpectedClose;	///< agent closed the connection
};

struct SearchdStats_t
{
	DWORD		m_uStarted;
	int64_t		m_iConnections;
	int64_t		m_iMaxedOut;
	int64_t		m_iCommandCount[SEARCHD_COMMAND_TOTAL];
	int64_t		m_iAgentConnect;
	int64_t		m_iAgentRetry;

	int64_t		m_iQueries;			///< search queries count (differs from search commands count because of multi-queries)
	int64_t		m_iQueryTime;		///< wall time spent (including network wait time)
	int64_t		m_iQueryCpuTime;	///< CPU time spent

	int64_t		m_iDistQueries;		///< distributed queries count
	int64_t		m_iDistWallTime;	///< wall time spent on distributed queries
	int64_t		m_iDistLocalTime;	///< wall time spent searching local indexes in distributed queries
	int64_t		m_iDistWaitTime;	///< time spent waiting for remote agents in distributed queries

	int64_t		m_iDiskReads;		///< total read IO calls (fired by search queries)
	int64_t		m_iDiskReadBytes;	///< total read IO traffic
	int64_t		m_iDiskReadTime;	///< total read IO time

	DWORD			m_bmAgentStats[STATS_MAX_AGENTS/32];	///< per-agent storage usage bitmap
	AgentStats_t	m_dAgentStats[STATS_MAX_AGENTS];		///< per-agent storage
};

static SearchdStats_t *			g_pStats		= NULL;
static CSphSharedBuffer<BYTE>	g_tStatsBuffer;
static CSphProcessSharedMutex	g_tStatsMutex;

struct FlushState_t
{
	int		m_iUpdateTag;		///< ever-growing update tag
	int		m_bFlushing;		///< update flushing in progress
	int		m_iFlushTag;		///< last flushed tag
	bool	m_bForceCheck;		///< forced check/flush flag
};

static volatile FlushState_t *	g_pFlush		= NULL;
static CSphSharedBuffer<BYTE>	g_tFlushBuffer;

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
#define ECONNABORTED	WSAECONNABORTED
#define socklen_t		int

#define ftruncate		_chsize
#define getpid			GetCurrentProcessId

#endif // USE_WINDOWS

const int EXT_COUNT = 8;
const int EXT_MVP = 8;
const char * g_dNewExts[EXT_COUNT] = { ".new.sph", ".new.spa", ".new.spi", ".new.spd", ".new.spp", ".new.spm", ".new.spk", ".new.sps" };
const char * g_dOldExts[] = { ".old.sph", ".old.spa", ".old.spi", ".old.spd", ".old.spp", ".old.spm", ".old.spk", ".old.sps", ".old.mvp" };
const char * g_dCurExts[] = { ".sph", ".spa", ".spi", ".spd", ".spp", ".spm", ".spk", ".sps", ".mvp" };

/////////////////////////////////////////////////////////////////////////////
// MISC
/////////////////////////////////////////////////////////////////////////////

ServedIndex_t::ServedIndex_t ()
{
	Reset ();
}

void ServedIndex_t::Reset ()
{
	m_pIndex = NULL;
	m_bEnabled = true;
	m_bMlock = false;
	m_bPreopen = false;
	m_bOnDiskDict = false;
	m_bStar = false;
	m_bExpand = false;
	m_bToDelete = false;
	m_bOnlyNew = false;
	m_iUpdateTag = 0;
	m_bRT = false;

	m_tLock = CSphRwlock();
	if ( g_eWorkers==MPM_THREADS )
		m_tLock.Init();
}

ServedIndex_t::~ServedIndex_t ()
{
	SafeDelete ( m_pIndex );
	if ( g_eWorkers==MPM_THREADS )
		Verify ( m_tLock.Done() );
}

void ServedIndex_t::ReadLock () const
{
	if ( g_eWorkers==MPM_THREADS )
	{
		if ( m_tLock.ReadLock() )
			sphLogDebug ( "ReadLock %p", this );
		else
		{
			sphLogDebug ( "ReadLock %p failed", this );
			assert (false);
		}
	}
}

void ServedIndex_t::WriteLock () const
{
	if ( g_eWorkers==MPM_THREADS )
	{
		if ( m_tLock.WriteLock() )
			sphLogDebug ( "WriteLock %p", this );
		else
		{
			sphLogDebug ( "WriteLock %p failed", this );
			assert (false);
		}
	}
}

void ServedIndex_t::Unlock () const
{
	if ( g_eWorkers==MPM_THREADS )
	{
		if ( m_tLock.Unlock() )
			sphLogDebug ( "Unlock %p", this );
		else
		{
			sphLogDebug ( "Unlock %p failed", this );
			assert (false);
		}
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

ServedIndex_t & IndexHashIterator_c::Get ()
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
	if ( g_eWorkers==MPM_THREADS )
		if ( !m_tLock.Init() )
			sphDie ( "failed to init hash indexes rwlock" );
}


IndexHash_c::~IndexHash_c()
{
	if ( g_eWorkers==MPM_THREADS )
		Verify ( m_tLock.Done() );
}


void IndexHash_c::Rlock () const
{
	if ( g_eWorkers==MPM_THREADS )
		Verify ( m_tLock.ReadLock() );
}


void IndexHash_c::Wlock () const
{
	if ( g_eWorkers==MPM_THREADS )
		Verify ( m_tLock.WriteLock() );
}


void IndexHash_c::Unlock () const
{
	if ( g_eWorkers==MPM_THREADS )
		Verify ( m_tLock.Unlock() );
}


bool IndexHash_c::Add ( const ServedIndex_t & tValue, const CSphString & tKey )
{
	Wlock();
	bool bRes = BASE::Add ( tValue, tKey );
	Unlock();
	return bRes;
}


bool IndexHash_c::Delete ( const CSphString & tKey )
{
	// tricky part
	// hash itself might be unlocked, but entry (!) might still be locked
	// hence, we also need to acquire a lock on entry, and an exclusive one
	Wlock();
	bool bRes = false;
	ServedIndex_t * pEntry = GetWlockedEntry ( tKey );
	if ( pEntry )
	{
		pEntry->Unlock();
		bRes = BASE::Delete ( tKey );
	}
	Unlock();
	return bRes;
}


const ServedIndex_t * IndexHash_c::GetRlockedEntry ( const CSphString & tKey ) const
{
	Rlock();
	ServedIndex_t * pEntry = BASE::operator() ( tKey );
	if ( pEntry )
		pEntry->ReadLock();
	Unlock();
	return pEntry;
}


ServedIndex_t * IndexHash_c::GetWlockedEntry ( const CSphString & tKey ) const
{
	Rlock();
	ServedIndex_t * pEntry = BASE::operator() ( tKey );
	if ( pEntry )
		pEntry->WriteLock();
	Unlock();
	return pEntry;
}


ServedIndex_t & IndexHash_c::GetUnlockedEntry ( const CSphString & tKey ) const
{
	return BASE::operator[] ( tKey );
}


bool IndexHash_c::Exists ( const CSphString & tKey ) const
{
	Rlock();
	bool bRes = BASE::Exists ( tKey );
	Unlock();
	return bRes;
}

//////////////////////////////////////////////////////////////////////////

static void DoUnlockClear ( CSphVector< const ServedIndex_t * > & dLocked )
{
	ARRAY_FOREACH ( i, dLocked )
	{
		dLocked[i]->Unlock();
	}
	dLocked.Resize(0);
}
//////////////////////////////////////////////////////////////////////////

void DistributedMutex_t::Init ()
{
	if ( g_eWorkers==MPM_THREADS )
		m_tLock.Init();
}

void DistributedMutex_t::Done ()
{
	if ( g_eWorkers==MPM_THREADS )
		m_tLock.Done();
}

void DistributedMutex_t::Lock ()
{
	if ( g_eWorkers==MPM_THREADS )
		m_tLock.Lock();
}

void DistributedMutex_t::Unlock()
{
	if ( g_eWorkers==MPM_THREADS )
		m_tLock.Unlock();
}

/////////////////////////////////////////////////////////////////////////////
// LOGGING
/////////////////////////////////////////////////////////////////////////////

void Shutdown (); // forward ref for sphFatal()


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
void sphLogEntry ( ESphLogLevel eLevel, char * sBuf, char * sTtyBuf )
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
		strcat ( sBuf, "\n" ); // NOLINT

		lseek ( g_iLogFile, 0, SEEK_END );
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

	static ESphLogLevel eLastLevel = LOG_INFO;
	static DWORD uLastEntry = 0;
	static int64_t tmLastStamp = -1000000-FLUSH_THRESH_TIME;
	static int iLastRepeats = 0;

	// only if we can
	if ( ( sFmt && eLevel>g_eLogLevel ) || ( g_iLogFile<0 && !g_bService ) )
		return;

	// format the banner
	char sTimeBuf[128];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	const char * sBanner = "";
	if ( sFmt==NULL ) eLevel = eLastLevel;
	if ( eLevel==LOG_FATAL ) sBanner = "FATAL: ";
	if ( eLevel==LOG_WARNING ) sBanner = "WARNING: ";
	if ( eLevel==LOG_DEBUG ) sBanner = "DEBUG: ";

	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf)-1, "[%s] [%5d] ", sTimeBuf, (int)getpid() );

	char * sTtyBuf = sBuf + strlen(sBuf);
	strncpy ( sTtyBuf, sBanner, 32 ); // 32 is arbitrary; just something that is enough and keeps lint happy

	int iLen = strlen(sBuf);

	// format the message
	if ( sFmt )
		vsnprintf ( sBuf+iLen, sizeof(sBuf)-iLen-1, sFmt, ap );

	// catch dupes
	DWORD uEntry = sFmt ? sphCRC32 ( (const BYTE*)( sBuf+iLen ) ) : 0;
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
		eLastLevel = LOG_INFO;
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

void sphFatal ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	sphLog ( LOG_FATAL, sFmt, ap );
	va_end ( ap );
	Shutdown ();
	exit ( 1 );
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

	void SubmitEx ( const char * sIndex, const char * sTemplate, ... )
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

	void BuildReport ( StrBuf_t & sReport )
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

/////////////////////////////////////////////////////////////////////////////
// SIGNAL HANDLERS
/////////////////////////////////////////////////////////////////////////////

void Shutdown ()
{
	bool bAttrsSaveOk = true;
	// some head-only shutdown procedures
	if ( g_bHeadDaemon )
	{
		if ( g_eWorkers==MPM_THREADS )
		{
			// tell flush-rt thread to shutdown, and wait until it does
			g_bRtFlushShutdown = true;
			sphThreadJoin ( &g_tRtFlushThread );

			// tell rotation thread to shutdown, and wait until it does
			g_bRotateShutdown = true;
			sphThreadJoin ( &g_tRotateThread );
			g_tRotateQueueMutex.Done();
			g_tRotateConfigMutex.Done();

			int64_t tmShutStarted = sphMicroTimer();
			// stop search threads; up to 3 seconds long
			while ( g_dThd.GetLength() > 0 && ( sphMicroTimer()-tmShutStarted )<3000000 )
				sphSleepMsec ( 50 );

			g_tThdMutex.Lock();
			g_dThd.Reset();
			g_tThdMutex.Unlock();
			g_tThdMutex.Done();
		}

		SafeDelete ( g_pCfg );

		// save attribute updates for all local indexes
		for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
		{
			const ServedIndex_t & tServed = it.Get();
			if ( !tServed.m_bEnabled )
				continue;

			if ( !tServed.m_pIndex->SaveAttributes() )
			{
				sphWarning ( "index %s: attrs save failed: %s", it.GetKey().cstr(), tServed.m_pIndex->GetLastError().cstr() );
				bAttrsSaveOk = false;
			}
		}

		// unlock indexes and release locks if needed
		for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
			it.Get().m_pIndex->Unlock();
		g_pIndexes->Reset();

		// clear shut down of rt indexes + binlog
		g_tDistLock.Done();
		SafeDelete ( g_pIndexes );
		sphRTDone();

		SphCrashLogger_c::Done();

		sphShutdownWordforms ();
	}

	ARRAY_FOREACH ( i, g_dListeners )
		if ( g_dListeners[i].m_iSock>=0 )
			sphSockClose ( g_dListeners[i].m_iSock );

#if USE_WINDOWS
	CloseHandle ( g_hPipe );
#else
	if ( g_bHeadDaemon )
	{
		const CSphString sPipeName = GetNamedPipeName ( getpid() );
		const int hFile = ::open ( sPipeName.cstr(), O_WRONLY | O_NONBLOCK );
		if ( hFile!=-1 )
		{
			DWORD uStatus = bAttrsSaveOk;
			int iDummy; // to avoid gcc unused result warning
			iDummy = ::write ( hFile, &uStatus, sizeof(DWORD) );
			::close ( hFile );
		}
	}
#endif

	// remove pid
	if ( g_bHeadDaemon && g_sPidFile )
	{
		::close ( g_iPidFD );
		::unlink ( g_sPidFile );
	}

	if ( g_bHeadDaemon )
		sphInfo ( "shutdown complete" );

	if ( g_bHeadDaemon )
		sphThreadDone ( g_iLogFile );
}

#if !USE_WINDOWS
void sighup ( int )
{
	g_bGotSighup = true;
}


void sigterm ( int )
{
	// tricky bit
	// we can't call exit() here because malloc()/free() are not re-entrant
	// we could call _exit() but let's try to die gracefully on TERM
	// and let signal sender wait and send KILL as needed
	g_bGotSigterm = true;
	sphInterruptNow();
}


void sigchld ( int )
{
	g_bGotSigchld = true;
}


void sigusr1 ( int )
{
	g_bGotSigusr1 = true;
}
#endif // !USE_WINDOWS


// crash query handler
static const char g_dEncodeBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
BYTE * sphEncodeBase64 ( BYTE * pDst, const BYTE * pSrc, int iLen )
{
	for ( int i=0; i<iLen/3; i++ )
	{
		// Convert to big endian
		DWORD uSrc = ( pSrc[i*3+0] << 16 ) | ( pSrc[i*3+1] << 8 ) | ( pSrc[i*3+2] );

		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x00FC0000 ) >> 18 ];
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x0003F000 ) >> 12 ];
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x00000FC0 ) >> 6 ];
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x0000003F ) ];
	}

	if ( iLen%3==1 )
	{
		DWORD uSrc = pSrc[iLen-1]<<16;
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x00FC0000 ) >> 18 ];
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x0003F000 ) >> 12 ];
		*pDst++ = '=';
		*pDst++ = '=';
	} else if ( iLen%3==2 )
	{
		DWORD uSrc = ( pSrc[iLen-2]<<16 ) | ( pSrc[iLen-1] << 8 );
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x00FC0000 ) >> 18 ];
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x0003F000 ) >> 12 ];
		*pDst++ = g_dEncodeBase64 [ ( uSrc & 0x00000FC0 ) >> 6 ];
		*pDst++ = '=';
	}

	return pDst;
}

#define SPH_LAST_QUERY_MAX_SIZE 4096
#define SPH_TIME_PID_MAX_SIZE 256
const char		g_sCrashedBannerAPI[] = "\n--- crashed SphinxAPI request dump ---\n";
const char		g_sCrashedBannerMySQL[] = "\n--- crashed SphinxQL request dump ---\n";
const char		g_sMemoryStatBanner[] = "\n--- memory statistics ---\n";
static BYTE		g_dEncoded[	SPH_TIME_PID_MAX_SIZE
							+ 2 * Max ( sizeof(g_sCrashedBannerAPI), sizeof(g_sCrashedBannerMySQL) )
							+ SPH_LAST_QUERY_MAX_SIZE*4/3 ];
static BYTE		g_dQuery [SPH_LAST_QUERY_MAX_SIZE*4/3];
static char		g_sCrashInfo [SPH_TIME_PID_MAX_SIZE] = "[][]\n";
static int		g_iCrashInfoLen = 0;

#if USE_WINDOWS
static char		g_sMinidump[SPH_TIME_PID_MAX_SIZE] = "\n";
static int		g_iMinidumpLen = 0;
#endif

SphCrashLogger_c SphCrashLogger_c::m_tLastQuery = SphCrashLogger_c ();
SphThreadKey_t SphCrashLogger_c::m_tLastQueryTLS = SphThreadKey_t ();

SphCrashLogger_c::SphCrashLogger_c ()
	: m_pQuery ( NULL )
	, m_iSize ( 0 )
	, m_uCMD ( 0 )
	, m_uVer ( 0 )
	, m_bMySQL ( false )
{
}

void SphCrashLogger_c::Init ()
{
	if ( g_eWorkers==MPM_THREADS )
		Verify ( sphThreadKeyCreate ( &m_tLastQueryTLS ) );
}

void SphCrashLogger_c::Done ()
{
	if ( g_eWorkers==MPM_THREADS )
		sphThreadKeyDelete ( m_tLastQueryTLS );
}


#if !USE_WINDOWS
#ifndef NDEBUG
void SphCrashLogger_c::HandleCrash ( int sig )
#else
void SphCrashLogger_c::HandleCrash ( int )
#endif // NDEBUG
#else
LONG WINAPI SphCrashLogger_c::HandleCrash ( EXCEPTION_POINTERS * pExc )
#endif // !USE_WINDOWS
{
	if ( g_iLogFile<0 )
		CRASH_EXIT;

	// log [time][pid]
	lseek ( g_iLogFile, 0, SEEK_END );
	sphWrite ( g_iLogFile, g_sCrashInfo, g_iCrashInfoLen );

	// log query
	SphCrashLogger_c tQuery = m_tLastQuery;
	if ( g_eWorkers==MPM_THREADS && !g_bSafeTrace )
	{
		const SphCrashLogger_c * pQueryTLS = (SphCrashLogger_c *)sphThreadGet ( m_tLastQueryTLS );
		if ( pQueryTLS )
			tQuery = *pQueryTLS;
	}

	tQuery.m_iSize = Min ( tQuery.m_iSize, SPH_LAST_QUERY_MAX_SIZE );

	BYTE * pEncoded = g_dEncoded;

	// request dump banner
	int iBannerLen = ( tQuery.m_bMySQL ? sizeof(g_sCrashedBannerMySQL) : sizeof(g_sCrashedBannerAPI) ) - 1;
	memcpy ( pEncoded, tQuery.m_bMySQL ? g_sCrashedBannerMySQL : g_sCrashedBannerAPI, iBannerLen );
	pEncoded += iBannerLen;

	// query
	if ( tQuery.m_iSize )
	{
		if ( tQuery.m_bMySQL )
		{
			memcpy ( pEncoded, tQuery.m_pQuery, tQuery.m_iSize );
			pEncoded += tQuery.m_iSize;
		} else
		{
			int iSize = Min ( tQuery.m_iSize, SPH_LAST_QUERY_MAX_SIZE-8 );
			*(g_dQuery+0) = (BYTE)( ( tQuery.m_uCMD>>8 ) & 0xff );
			*(g_dQuery+1) = (BYTE)( tQuery.m_uCMD & 0xff );
			*(g_dQuery+2) = (BYTE)( ( tQuery.m_uVer>>8 ) & 0xff );
			*(g_dQuery+3) = (BYTE)( tQuery.m_uVer & 0xff );
			*(g_dQuery+4) = (BYTE)( ( iSize>>24 ) & 0xff );
			*(g_dQuery+5) = (BYTE)( ( iSize>>16 ) & 0xff );
			*(g_dQuery+6) = (BYTE)( ( iSize>>8 ) & 0xff );
			*(g_dQuery+7) = (BYTE)( iSize & 0xff );
			memcpy ( g_dQuery+8, tQuery.m_pQuery, iSize );
			pEncoded = sphEncodeBase64 ( pEncoded, g_dQuery, Min ( tQuery.m_iSize+8, SPH_LAST_QUERY_MAX_SIZE ) );
		}
	}

	// tail
	const char sTail[] = "\n--- request dump end ---\n";
	memcpy ( pEncoded, sTail, sizeof(sTail)-1 );
	pEncoded += sizeof(sTail)-1;

#if USE_WINDOWS
	// mini-dump reference
	const char sMinidump[] = "minidump at: ";
	memcpy ( pEncoded, sMinidump, sizeof(sMinidump)-1 );
	pEncoded += sizeof(sMinidump)-1;

	memcpy ( pEncoded, g_sMinidump, g_iMinidumpLen );
	pEncoded += g_iMinidumpLen;
	*pEncoded++ = '\n';
#endif

	int iSize = pEncoded-g_dEncoded;

	lseek ( g_iLogFile, 0, SEEK_END );
	sphWrite ( g_iLogFile, g_dEncoded, iSize );

	// log trace
#if !USE_WINDOWS
	sphBacktrace ( g_iLogFile, g_bSafeTrace );
#else
	sphBacktrace ( pExc, g_sMinidump );
#endif

	// threads table
	if ( g_eWorkers==MPM_THREADS )
	{
		// FIXME? should we try to lock threads table somehow?
		sphSafeInfo ( g_iLogFile, "--- %d active threads ---", g_dThd.GetLength() );
		ARRAY_FOREACH ( iThd, g_dThd )
		{
			ThdDesc_t * pThd = g_dThd[iThd];
			sphSafeInfo ( g_iLogFile, "thd %d, proto %s, state %s, command %s",
				iThd,
				g_dProtoNames[pThd->m_eProto],
				g_dThdStates[pThd->m_eThdState],
				pThd->m_sCommand ? pThd->m_sCommand : "-" );
		}
	}

	// memory info
#if SPH_ALLOCS_PROFILER
	sphWrite ( g_iLogFile, g_sMemoryStatBanner, sizeof ( g_sMemoryStatBanner )-1 );
	sphMemStatDump ( g_iLogFile );
#endif

	sphSafeInfo ( g_iLogFile, "------- CRASH DUMP END -------" );

	CRASH_EXIT;
}

void SphCrashLogger_c::SetLastQuery ( const BYTE * pQuery, int iSize, bool bMySQL, int uCmd, int uVer )
{
	SphCrashLogger_c * pLastQuery = &m_tLastQuery;
	if ( g_eWorkers==MPM_THREADS )
		pLastQuery = (SphCrashLogger_c *)sphThreadGet ( m_tLastQueryTLS );

	pLastQuery->m_pQuery = pQuery;
	pLastQuery->m_iSize = iSize;
	pLastQuery->m_uCMD = (WORD)uCmd;
	pLastQuery->m_uVer = (WORD)uVer;
	pLastQuery->m_bMySQL = bMySQL;
}

void SphCrashLogger_c::SetupTimePID ()
{
	char sTimeBuf[SPH_TIME_PID_MAX_SIZE];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	g_iCrashInfoLen = snprintf ( g_sCrashInfo, SPH_TIME_PID_MAX_SIZE-1, "------- FATAL: CRASH DUMP -------\n[%s] [%5d]\n", sTimeBuf, (int)getpid() );
}

void SphCrashLogger_c::SetupTLS ()
{
	if ( g_eWorkers!=MPM_THREADS )
		return;

	Verify ( sphThreadSet ( m_tLastQueryTLS, this ) );
}


void SetSignalHandlers ()
{
	SphCrashLogger_c::Init();

#if !USE_WINDOWS
	struct sigaction sa;
	sigfillset ( &sa.sa_mask );
	sa.sa_flags = SA_NOCLDSTOP;

	bool bSignalsSet = false;
	for ( ;; )
	{
		sa.sa_handler = sigterm;	if ( sigaction ( SIGTERM, &sa, NULL )!=0 ) break;
		sa.sa_handler = sigterm;	if ( sigaction ( SIGINT, &sa, NULL )!=0 ) break;
		sa.sa_handler = sighup;		if ( sigaction ( SIGHUP, &sa, NULL )!=0 ) break;
		sa.sa_handler = sigusr1;	if ( sigaction ( SIGUSR1, &sa, NULL )!=0 ) break;
		sa.sa_handler = sigchld;	if ( sigaction ( SIGCHLD, &sa, NULL )!=0 ) break;
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
#else
	g_iMinidumpLen = snprintf ( g_sMinidump, SPH_TIME_PID_MAX_SIZE-1, "%s.%d.mdmp", g_sPidFile ? g_sPidFile : "", (int)getpid() );
	SetUnhandledExceptionFilter ( SphCrashLogger_c::HandleCrash );
#endif
}


/////////////////////////////////////////////////////////////////////////////
// NETWORK STUFF
/////////////////////////////////////////////////////////////////////////////

const int		WIN32_PIPE_BUFSIZE		= 32;


#if USE_WINDOWS

/// on Windows, the wrapper just prevents the warnings
void sphFDSet ( int fd, fd_set * fdset )
{
	#pragma warning(disable:4127) // conditional expr is const
	#pragma warning(disable:4389) // signed/unsigned mismatch

	FD_SET ( fd, fdset );

	#pragma warning(default:4127) // conditional expr is const
	#pragma warning(default:4389) // signed/unsigned mismatch
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

DWORD sphGetAddress ( const char * sHost, bool bFatal=false )
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
		sphFatal ( "UNIX socket path is too long (len=%d)", len );

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
		sphFatal ( "setsockopt() failed: %s", sphSockError() );

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


inline bool IsPortInRange ( int iPort )
{
	return ( iPort>0 ) && ( iPort<=0xFFFF );
}


void CheckPort ( int iPort )
{
	if ( !IsPortInRange(iPort) )
		sphFatal ( "port %d is out of range", iPort );
}


ProtocolType_e ProtoByName ( const CSphString & sProto )
{
	if ( sProto=="sphinx" )			return PROTO_SPHINX;
	else if ( sProto=="mysql41" )	return PROTO_MYSQL41;

	sphFatal ( "unknown listen protocol type '%s'", sProto.cstr() ? sProto.cstr() : "(NULL)" );

	// funny magic
	// MSVC -O2 whines about unreachable code
	// everyone else whines about missing return value
#if !(USE_WINDOWS && defined(NDEBUG))
	return PROTO_SPHINX;
#endif
}


struct ListenerDesc_t
{
	ProtocolType_e	m_eProto;
	CSphString		m_sUnix;
	DWORD			m_uIP;
	int				m_iPort;
};


ListenerDesc_t ParseListener ( const char * sSpec )
{
	ListenerDesc_t tRes;
	tRes.m_eProto = PROTO_SPHINX;
	tRes.m_sUnix = "";
	tRes.m_uIP = htonl ( INADDR_ANY );
	tRes.m_iPort = SPHINXAPI_PORT;

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
			tRes.m_eProto = ProtoByName ( sParts[1] );

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
		tRes.m_eProto = ProtoByName ( sParts[1] );

	} else
	{
		// 1st part must be a host name; must be host:port[:proto]
		if ( iParts==3 )
			tRes.m_eProto = ProtoByName ( sParts[2] );

		tRes.m_iPort = atol ( sParts[1].cstr() );
		CheckPort ( tRes.m_iPort );

		tRes.m_uIP = sParts[0].IsEmpty()
			? htonl ( INADDR_ANY )
			: sphGetAddress ( sParts[0].cstr(), GETADDR_STRICT );
	}
	return tRes;
}


void AddListener ( const CSphString & sListen )
{
	ListenerDesc_t tDesc = ParseListener ( sListen.cstr() );

	Listener_t tListener;
	tListener.m_eProto = tDesc.m_eProto;

#if !USE_WINDOWS
	if ( !tDesc.m_sUnix.IsEmpty() )
		tListener.m_iSock = sphCreateUnixSocket ( tDesc.m_sUnix.cstr() );
	else
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

		fd_set fdRead;
		FD_ZERO ( &fdRead );
		sphFDSet ( iSock, &fdRead );

		fd_set fdExcept;
		FD_ZERO ( &fdExcept );
		sphFDSet ( iSock, &fdExcept );

#if USE_WINDOWS
		// Windows EINTR emulation
		// Ctrl-C will not interrupt select on Windows, so let's handle that manually
		// forcibly limit select() to 100 ms, and check flag afterwards
		if ( bIntr )
			tmMicroLeft = Min ( tmMicroLeft, 100000 );
#endif

		struct timeval tv;
		tv.tv_sec = (int)( tmMicroLeft / 1000000 );
		tv.tv_usec = (int)( tmMicroLeft % 1000000 );

		iRes = ::select ( iSock+1, &fdRead, NULL, &fdExcept, &tv );

		// if there was EINTR, retry
		if ( iRes==-1 )
		{
			iErr = sphSockGetErrno();
			if ( iErr==EINTR && !g_bGotSigterm && !bIntr )
				continue;

			if ( g_bGotSigterm )
				sphLogDebug ( "sphSockRead: got SIGTERM, exit -1" );

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
		if ( iRes==-1 )
		{
			iErr = sphSockGetErrno();
			if ( iErr==EINTR && !bIntr )
				continue;

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
// NETWORK BUFFERS
/////////////////////////////////////////////////////////////////////////////

/// fixed-memory response buffer
/// tracks usage, and flushes to network when necessary
class NetOutputBuffer_c
{
public:
	explicit	NetOutputBuffer_c ( int iSock );

	bool		SendInt ( int iValue )			{ return SendT<int> ( htonl ( iValue ) ); }
	bool		SendDword ( DWORD iValue )		{ return SendT<DWORD> ( htonl ( iValue ) ); }
	bool		SendLSBDword ( DWORD v )		{ SendByte ( (BYTE)( v&0xff ) ); SendByte ( (BYTE)( (v>>8)&0xff ) ); SendByte ( (BYTE)( (v>>16)&0xff ) ); return SendByte ( (BYTE)( (v>>24)&0xff) ); }
	bool		SendWord ( WORD iValue )		{ return SendT<WORD> ( htons ( iValue ) ); }
	bool		SendUint64 ( uint64_t iValue )	{ SendT<DWORD> ( htonl ( (DWORD)(iValue>>32) ) ); return SendT<DWORD> ( htonl ( (DWORD)(iValue&0xffffffffUL) ) ); }
	bool		SendFloat ( float fValue )		{ return SendT<DWORD> ( htonl ( sphF2DW ( fValue ) ) ); }
	bool		SendByte ( BYTE uValue )		{ return SendT<BYTE> ( uValue ); }

#if USE_64BIT
	bool		SendDocid ( SphDocID_t iValue )	{ return SendUint64 ( iValue ); }
#else
	bool		SendDocid ( SphDocID_t iValue )	{ return SendDword ( iValue ); }
#endif

	bool		SendString ( const char * sStr );
	bool		SendMysqlString ( const char * sStr );

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

public:
	bool							SendBytes ( const void * pBuf, int iLen );	///< (was) protected to avoid network-vs-host order bugs
	template < typename T > bool	SendT ( T tValue );							///< (was) protected to avoid network-vs-host order bugs
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
	DWORD			GetLSBDword () { return GetByte() + ( GetByte()<<8 ) + ( GetByte()<<16 ) + ( GetByte()<<24 ); }
	uint64_t		GetUint64() { uint64_t uRes = GetDword(); return (uRes<<32)+GetDword(); }
	BYTE			GetByte () { return GetT<BYTE> (); }
	float			GetFloat () { return sphDW2F ( ntohl ( GetT<DWORD> () ) ); }
	CSphString		GetString ();
	CSphString		GetRawString ( int iLen );
	int				GetDwords ( DWORD ** pBuffer, int iMax, const char * sErrorTemplate );
	bool			GetError () { return m_bError; }

	template < typename T > bool	GetDwords ( CSphVector<T> & dBuffer, int iMax, const char * sErrorTemplate );
	template < typename T > bool	GetQwords ( CSphVector<T> & dBuffer, int iMax, const char * sErrorTemplate );

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
	explicit		NetInputBuffer_c ( int iSock );
	virtual			~NetInputBuffer_c ();

	bool			ReadFrom ( int iLen, int iTimeout, bool bIntr=false );
	bool			ReadFrom ( int iLen ) { return ReadFrom ( iLen, g_iReadTimeout ); }

	virtual void	SendErrorReply ( const char *, ... );

	const BYTE *	GetBufferPtr () const { return m_pBuf; }
	bool			IsIntr () const { return m_bIntr; }

protected:
	static const int	NET_MINIBUFFER_SIZE = 4096;

	int					m_iSock;
	bool				m_bIntr;

	BYTE				m_dMinibufer[NET_MINIBUFFER_SIZE];
	int					m_iMaxibuffer;
	BYTE *				m_pMaxibuffer;
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

	sphUnalignedWrite ( m_pBuffer, tValue );
	m_pBuffer += sizeof(T);
	assert ( m_pBuffer<m_dBuffer+sizeof(m_dBuffer) );
	return true;
}


bool NetOutputBuffer_c::SendString ( const char * sStr )
{
	if ( m_bError )
		return false;

	FlushIf ( sizeof(DWORD) );

	int iLen = sStr ? strlen(sStr) : 0;
	SendInt ( iLen );
	return SendBytes ( sStr, iLen );
}


int MysqlPackedLen ( const char * sStr )
{
	int iLen = strlen(sStr);
	if ( iLen<251 )
		return 1 + iLen;
	if ( iLen<=0xffff )
		return 3 + iLen;
	if ( iLen<=0xffffff )
		return 4 + iLen;
	return 9 + iLen;
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


bool NetOutputBuffer_c::SendMysqlString ( const char * sStr )
{
	if ( m_bError )
		return false;

	int iLen = strlen(sStr);

	BYTE dBuf[12];
	BYTE * pBuf = (BYTE*) MysqlPack ( dBuf, iLen );
	SendBytes ( dBuf, (int)( pBuf-dBuf ) );
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

	if ( g_bGotSigterm )
		sphLogDebug ( "SIGTERM in NetOutputBuffer::Flush" );

	assert ( iLen>0 );
	assert ( iLen<=(int)sizeof(m_dBuffer) );
	char * pBuffer = (char *)&m_dBuffer[0];

	const int64_t tmMaxTimer = sphMicroTimer() + g_iWriteTimeout*1000000; // in microseconds
	while ( !m_bError )
	{
		int iRes = sphSockSend ( m_iSock, pBuffer, iLen );
		if ( iRes < 0 )
		{
			int iErrno = sphSockGetErrno();
			if ( iErrno!=EAGAIN )
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

		int64_t tmMicroLeft = tmMaxTimer - sphMicroTimer();
		if ( tmMicroLeft>0 )
		{
			fd_set fdWrite;
			FD_ZERO ( &fdWrite );
			sphFDSet ( m_iSock, &fdWrite );

			struct timeval tvTimeout;
			tvTimeout.tv_sec = (int)( tmMicroLeft / 1000000 );
			tvTimeout.tv_usec = (int)( tmMicroLeft % 1000000 );

			iRes = select ( m_iSock+1, NULL, &fdWrite, NULL, &tvTimeout );
		} else
			iRes = 0;

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

	m_pBuffer = m_dBuffer;
	return !m_bError;
}


bool NetOutputBuffer_c::FlushIf ( int iToAdd )
{
	if ( ( m_pBuffer+iToAdd )>=( m_dBuffer+sizeof(m_dBuffer) ) )
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

	T iRes = sphUnalignedRead ( *(T*)m_pCur );
	m_pCur += sizeof(T);
	return iRes;
}


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
		if ( !GetBytes ( (*ppBuffer), sizeof(DWORD)*iCount ) )
		{
			SafeDeleteArray ( (*ppBuffer) );
			return -1;
		}
		for ( int i=0; i<iCount; i++ )
			(*ppBuffer)[i] = htonl ( (*ppBuffer)[i] );
	}
	return iCount;
}


template < typename T > bool InputBuffer_c::GetDwords ( CSphVector<T> & dBuffer, int iMax, const char * sErrorTemplate )
{
	int iCount = GetInt ();
	if ( iCount<0 || iCount>iMax )
	{
		SendErrorReply ( sErrorTemplate, iCount, iMax );
		SetError ( true );
		return false;
	}

	dBuffer.Resize ( iCount );
	ARRAY_FOREACH ( i, dBuffer )
		dBuffer[i] = GetDword ();

	if ( m_bError )
		dBuffer.Reset ();

	return !m_bError;
}


template < typename T > bool InputBuffer_c::GetQwords ( CSphVector<T> & dBuffer, int iMax, const char * sErrorTemplate )
{
	int iCount = GetInt ();
	if ( iCount<0 || iCount>iMax )
	{
		SendErrorReply ( sErrorTemplate, iCount, iMax );
		SetError ( true );
		return false;
	}

	dBuffer.Resize ( iCount );
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


bool NetInputBuffer_c::ReadFrom ( int iLen, int iTimeout, bool bIntr )
{
	m_bIntr = false;
	if ( iLen<=0 || iLen>g_iMaxPacketSize || m_iSock<0 )
		return false;

	BYTE * pBuf = m_dMinibufer;
	if ( iLen>NET_MINIBUFFER_SIZE )
	{
		if ( iLen>m_iMaxibuffer )
		{
			SafeDeleteArray ( m_pMaxibuffer );
			m_pMaxibuffer = new BYTE [ iLen ];
			m_iMaxibuffer = iLen;
		}
		pBuf = m_pMaxibuffer;
	}

	m_pCur = m_pBuf = pBuf;
	int iGot = sphSockRead ( m_iSock, pBuf, iLen, iTimeout, bIntr );
	if ( g_bGotSigterm )
	{
		sphLogDebug ( "NetInputBuffer_c::ReadFrom: got SIGTERM, return false" );
		return false;
	}

	m_bError = g_bGotSigterm || ( iGot!=iLen );
	m_bIntr = !g_bGotSigterm && m_bError && ( sphSockPeekErrno()==EINTR );
	m_iLen = m_bError ? 0 : iLen;
	return !m_bError;
}


void NetInputBuffer_c::SendErrorReply ( const char * sTemplate, ... )
{
	char dBuf [ 2048 ];

	const int iHeaderLen = 12;
	const int iMaxStrLen = sizeof(dBuf) - iHeaderLen - 1;

	// fill header
	WORD * p0 = (WORD*)&dBuf[0];
	p0[0] = htons ( SEARCHD_ERROR ); // error code
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
	p4[0] = htonl ( 4+iStrLen );
	p4[1] = htonl ( iStrLen );

	// send!
	sphSockSend ( m_iSock, dBuf, iHeaderLen+iStrLen );

	// --console logging
	if ( g_bOptNoDetach )
		sphInfo ( "query error: %s", sBuf );
}

// fix MSVC 2005 fuckup
#if USE_WINDOWS
#pragma conform(forScope,on)
#endif

/////////////////////////////////////////////////////////////////////////////
// DISTRIBUTED QUERIES
/////////////////////////////////////////////////////////////////////////////

/// remote agent descriptor (stored in a global hash)
struct AgentDesc_t
{
	CSphString		m_sHost;		///< remote searchd host
	int				m_iPort;		///< remote searchd port, 0 if local
	CSphString		m_sPath;		///< local searchd UNIX socket path
	CSphString		m_sIndexes;		///< remote index names to query
	bool			m_bBlackhole;	///< blackhole agent flag
	int				m_iFamily;		///< TCP or UNIX socket
	DWORD			m_uAddr;		///< IP address
	int				m_iStatsIndex;	///< index into global searchd stats array

public:
	AgentDesc_t ()
		: m_iPort ( -1 )
		, m_bBlackhole ( false )
		, m_iFamily ( AF_INET )
		, m_uAddr ( 0 )
		, m_iStatsIndex ( -1 )
	{}
};

/// remote agent state
enum AgentState_e
{
	AGENT_UNUSED,					///< agent is unused for this request
	AGENT_CONNECT,					///< connecting to agent
	AGENT_HELLO,					///< waiting for "VER x" hello
	AGENT_QUERY,					///< query sent, wating for reply
	AGENT_REPLY,					///< reading reply
	AGENT_RETRY						///< should retry
};

/// remote agent connection (local per-query state)
struct AgentConn_t : public AgentDesc_t
{
	int				m_iSock;		///< socket number, -1 if not connected
	AgentState_e	m_eState;		///< current state

	bool			m_bSuccess;		///< whether last request was succesful (ie. there are available results)
	CSphString		m_sFailure;		///< failure message

	int				m_iReplyStatus;	///< reply status code
	int				m_iReplySize;	///< how many reply bytes are there
	int				m_iReplyRead;	///< how many reply bytes are alredy received
	BYTE *			m_pReplyBuf;	///< reply buffer

	CSphVector<CSphQueryResult>		m_dResults;		///< multi-query results

public:
	AgentConn_t ()
		: m_iSock ( -1 )
		, m_eState ( AGENT_UNUSED )
		, m_bSuccess ( false )
		, m_iReplyStatus ( -1 )
		, m_iReplySize ( 0 )
		, m_iReplyRead ( 0 )
		, m_pReplyBuf ( NULL )
	{}

	~AgentConn_t ()
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

	CSphString GetName() const
	{
		CSphString sName;
		switch ( m_iFamily )
		{
			case AF_INET: sName.SetSprintf ( "%s:%u", m_sHost.cstr(), m_iPort ); break;
			case AF_UNIX: sName = m_sPath; break;
		}
		return sName;
	}

	AgentConn_t & operator = ( const AgentDesc_t & rhs )
	{
		m_sHost = rhs.m_sHost;
		m_iPort = rhs.m_iPort;
		m_sPath = rhs.m_sPath;
		m_sIndexes = rhs.m_sIndexes;
		m_bBlackhole = rhs.m_bBlackhole;
		m_iFamily = rhs.m_iFamily;
		m_uAddr = rhs.m_uAddr;
		m_iStatsIndex = rhs.m_iStatsIndex;
		return *this;
	}
};

/// distributed index
struct DistributedIndex_t
{
	CSphVector<AgentDesc_t>		m_dAgents;					///< remote agents
	CSphVector<CSphString>		m_dLocal;					///< local indexes
	int							m_iAgentConnectTimeout;		///< in msec
	int							m_iAgentQueryTimeout;		///< in msec
	bool						m_bToDelete;				///< should be deleted

public:
	DistributedIndex_t ()
		: m_iAgentConnectTimeout ( 1000 )
		, m_iAgentQueryTimeout ( 3000 )
		, m_bToDelete ( false )
	{}
};

/// global distributed index definitions hash
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
	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const = 0;
};


#define AGENT_STATS_INC(_agent,_counter) \
	if ( g_pStats && _agent.m_iStatsIndex>=0 && _agent.m_iStatsIndex<STATS_MAX_AGENTS ) \
	{ \
		g_tStatsMutex.Lock (); \
		g_pStats->m_dAgentStats [ _agent.m_iStatsIndex ]._counter++; \
		g_tStatsMutex.Unlock (); \
	}


void ConnectToRemoteAgents ( CSphVector<AgentConn_t> & dAgents, bool bRetryOnly )
{
	ARRAY_FOREACH ( iAgent, dAgents )
	{
		AgentConn_t & tAgent = dAgents[iAgent];
		if ( bRetryOnly && ( tAgent.m_eState!=AGENT_RETRY ) )
			continue;

		tAgent.m_eState = AGENT_UNUSED;
		tAgent.m_bSuccess = false;

		socklen_t len = 0;
		struct sockaddr_storage ss;
		memset ( &ss, 0, sizeof(ss) );
		ss.ss_family = (short)tAgent.m_iFamily;

		if ( ss.ss_family==AF_INET )
		{
			struct sockaddr_in *in = (struct sockaddr_in *)&ss;
			in->sin_port = htons ( (unsigned short)tAgent.m_iPort );
			in->sin_addr.s_addr = tAgent.m_uAddr;
			len = sizeof(*in);
		}
		#if !USE_WINDOWS
		else if ( ss.ss_family==AF_UNIX )
		{
			struct sockaddr_un *un = (struct sockaddr_un *)&ss;
			snprintf ( un->sun_path, sizeof(un->sun_path), "%s", tAgent.m_sPath.cstr() );
			len = sizeof(*un);
		}
		#endif

		tAgent.m_iSock = socket ( tAgent.m_iFamily, SOCK_STREAM, 0 );
		if ( tAgent.m_iSock<0 )
		{
			tAgent.m_sFailure.SetSprintf ( "socket() failed: %s", sphSockError() );
			return;
		}

		if ( sphSetSockNB ( tAgent.m_iSock )<0 )
		{
			tAgent.m_sFailure.SetSprintf ( "sphSetSockNB() failed: %s", sphSockError() );
			return;
		}

		// count connects
		if ( g_pStats )
		{
			g_tStatsMutex.Lock();
			g_pStats->m_iAgentConnect++;
			if ( bRetryOnly )
				g_pStats->m_iAgentRetry++;
			g_tStatsMutex.Unlock();
		}

		if ( connect ( tAgent.m_iSock, (struct sockaddr*)&ss, len )<0 )
		{
			int iErr = sphSockGetErrno();
			if ( iErr!=EINPROGRESS && iErr!=EINTR && iErr!=EWOULDBLOCK ) // check for EWOULDBLOCK is for winsock only
			{
				tAgent.Close ();
				tAgent.m_sFailure.SetSprintf ( "connect() failed: %s", sphSockError(iErr) );
				tAgent.m_eState = AGENT_RETRY; // do retry on connect() failures
				AGENT_STATS_INC ( tAgent, m_iConnectFailures );
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


int QueryRemoteAgents ( CSphVector<AgentConn_t> & dAgents, int iTimeout, const IRequestBuilder_t & tBuilder, int64_t * pWaited )
{
	int iAgents = 0;
	assert ( iTimeout>=0 );

	int64_t tmMaxTimer = sphMicroTimer() + iTimeout*1000; // in microseconds
	for ( ;; )
	{
		fd_set fdsRead, fdsWrite;
		FD_ZERO ( &fdsRead );
		FD_ZERO ( &fdsWrite );

		int iMax = 0;
		bool bDone = true;
		ARRAY_FOREACH ( i, dAgents )
		{
			const AgentConn_t & tAgent = dAgents[i];
			if ( tAgent.m_eState==AGENT_CONNECT || tAgent.m_eState==AGENT_HELLO )
			{
				assert ( !tAgent.m_sPath.IsEmpty() || tAgent.m_iPort>0 );
				assert ( tAgent.m_iSock>0 );

				sphFDSet ( tAgent.m_iSock, ( tAgent.m_eState==AGENT_CONNECT ) ? &fdsWrite : &fdsRead );
				iMax = Max ( iMax, tAgent.m_iSock );
				bDone = false;
			}
		}
		if ( bDone )
			break;

		int64_t tmSelect = sphMicroTimer();
		int64_t tmMicroLeft = tmMaxTimer - tmSelect;
		if ( tmMicroLeft<=0 )
			break; // FIXME? what about iTimeout==0 case?

		struct timeval tvTimeout;
		tvTimeout.tv_sec = (int)( tmMicroLeft/ 1000000 ); // full seconds
		tvTimeout.tv_usec = (int)( tmMicroLeft % 1000000 ); // microseconds

		// FIXME! check exceptfds for connect() failure as well, so that actively refused
		// connections would not stall for a full timeout
		int iSelected = select ( 1+iMax, &fdsRead, &fdsWrite, NULL, &tvTimeout );

		if ( pWaited )
			*pWaited += sphMicroTimer() - tmSelect;

		if ( iSelected<=0 )
			continue;

		ARRAY_FOREACH ( i, dAgents )
		{
			AgentConn_t & tAgent = dAgents[i];

			// check if connection completed
			if ( tAgent.m_eState==AGENT_CONNECT && FD_ISSET ( tAgent.m_iSock, &fdsWrite ) )
			{
				int iErr = 0;
				socklen_t iErrLen = sizeof(iErr);
				getsockopt ( tAgent.m_iSock, SOL_SOCKET, SO_ERROR, (char*)&iErr, &iErrLen );
				if ( iErr )
				{
					// connect() failure
					tAgent.m_sFailure.SetSprintf ( "connect() failed: %s", sphSockError(iErr) );
					tAgent.Close ();
					AGENT_STATS_INC ( tAgent, m_iConnectFailures );
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
				if ( iRes!=sizeof(iRemoteVer) )
				{
					tAgent.Close ();
					if ( iRes<0 )
					{
						// network error
						int iErr = sphSockGetErrno();
						tAgent.m_sFailure.SetSprintf ( "handshake failure (errno=%d, msg=%s)", iErr, sphSockError(iErr) );
						AGENT_STATS_INC ( tAgent, m_iNetworkErrors );

					} else if ( iRes>0 )
					{
						// incomplete reply
						tAgent.m_sFailure.SetSprintf ( "handshake failure (exp=%d, recv=%d)", sizeof(iRemoteVer), iRes );
						AGENT_STATS_INC ( tAgent, m_iWrongReplies );

					} else
					{
						// agent closed the connection
						// this might happen in out-of-sync connect-accept case; so let's retry
						tAgent.m_sFailure.SetSprintf ( "handshake failure (connection was closed)", iRes );
						tAgent.m_eState = AGENT_RETRY;
						AGENT_STATS_INC ( tAgent, m_iUnexpectedClose );
					}
					continue;
				}

				iRemoteVer = ntohl ( iRemoteVer );
				if (!( iRemoteVer==SPHINX_SEARCHD_PROTO || iRemoteVer==0x01000000UL ) ) // workaround for all the revisions that sent it in host order...
				{
					tAgent.m_sFailure.SetSprintf ( "handshake failure (unexpected protocol version=%d)", iRemoteVer );
					AGENT_STATS_INC ( tAgent, m_iWrongReplies );
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
		AgentConn_t & tAgent = dAgents[i];
		if ( tAgent.m_eState!=AGENT_QUERY && tAgent.m_eState!=AGENT_UNUSED && tAgent.m_eState!=AGENT_RETRY )
		{
			tAgent.Close ();
			if ( tAgent.m_eState==AGENT_HELLO )
			{
				tAgent.m_sFailure.SetSprintf ( "read() timed out" );
				AGENT_STATS_INC ( tAgent, m_iTimeoutsQuery );
			} else
			{
				tAgent.m_sFailure.SetSprintf ( "connect() timed out" );
				AGENT_STATS_INC ( tAgent,m_iTimeoutsConnect );
			}
			tAgent.m_eState = AGENT_RETRY; // do retry on connect() failures
		}
	}

	return iAgents;
}


int WaitForRemoteAgents ( CSphVector<AgentConn_t> & dAgents, int iTimeout, IReplyParser_t & tParser, int64_t * pWaited )
{
	assert ( iTimeout>=0 );

	int iAgents = 0;
	int64_t tmMaxTimer = sphMicroTimer() + iTimeout*1000; // in microseconds
	for ( ;; )
	{
		fd_set fdsRead;
		FD_ZERO ( &fdsRead );

		int iMax = 0;
		bool bDone = true;
		ARRAY_FOREACH ( iAgent, dAgents )
		{
			AgentConn_t & tAgent = dAgents[iAgent];
			if ( tAgent.m_bBlackhole )
				continue;

			if ( tAgent.m_eState==AGENT_QUERY || tAgent.m_eState==AGENT_REPLY )
			{
				assert ( !tAgent.m_sPath.IsEmpty() || tAgent.m_iPort>0 );
				assert ( tAgent.m_iSock>0 );

				sphFDSet ( tAgent.m_iSock, &fdsRead );
				iMax = Max ( iMax, tAgent.m_iSock );
				bDone = false;
			}
		}
		if ( bDone )
			break;

		int64_t tmSelect = sphMicroTimer();
		int64_t tmMicroLeft = tmMaxTimer - tmSelect;
		if ( tmMicroLeft<=0 ) // FIXME? what about iTimeout==0 case?
			break;

		struct timeval tvTimeout;
		tvTimeout.tv_sec = (int)( tmMicroLeft / 1000000 ); // full seconds
		tvTimeout.tv_usec = (int)( tmMicroLeft % 1000000 ); // microseconds

		int iSelected = select ( 1+iMax, &fdsRead, NULL, NULL, &tvTimeout );

		if ( pWaited )
			*pWaited += sphMicroTimer() - tmSelect;

		if ( iSelected<=0 )
			continue;

		ARRAY_FOREACH ( iAgent, dAgents )
		{
			AgentConn_t & tAgent = dAgents[iAgent];
			if ( tAgent.m_bBlackhole )
				continue;
			if (!( tAgent.m_eState==AGENT_QUERY || tAgent.m_eState==AGENT_REPLY ))
				continue;
			if ( !FD_ISSET ( tAgent.m_iSock, &fdsRead ) )
				continue;

			// if there was no reply yet, read reply header
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
						tAgent.m_sFailure.SetSprintf ( "failed to receive reply header" );
						AGENT_STATS_INC ( tAgent, m_iNetworkErrors );
						break;
					}

					tReplyHeader.m_iStatus = ntohs ( tReplyHeader.m_iStatus );
					tReplyHeader.m_iVer = ntohs ( tReplyHeader.m_iVer );
					tReplyHeader.m_iLength = ntohl ( tReplyHeader.m_iLength );

					// check the packet
					if ( tReplyHeader.m_iLength<0 || tReplyHeader.m_iLength>g_iMaxPacketSize ) // FIXME! add reasonable max packet len too
					{
						tAgent.m_sFailure.SetSprintf ( "invalid packet size (status=%d, len=%d, max_packet_size=%d)", tReplyHeader.m_iStatus, tReplyHeader.m_iLength, g_iMaxPacketSize );
						AGENT_STATS_INC ( tAgent, m_iWrongReplies );
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
						tAgent.m_sFailure.SetSprintf ( "failed to alloc %d bytes for reply buffer", tAgent.m_iReplySize );
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
						tAgent.m_sFailure.SetSprintf ( "failed to receive reply body: %s", sphSockError() );
						AGENT_STATS_INC ( tAgent, m_iNetworkErrors );
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

					// absolve thy former sins
					tAgent.m_sFailure = "";

					// check for general errors/warnings first
					if ( tAgent.m_iReplyStatus==SEARCHD_WARNING )
					{
						CSphString sAgentWarning = tReq.GetString ();
						tAgent.m_sFailure.SetSprintf ( "remote warning: %s", sAgentWarning.cstr() );

					} else if ( tAgent.m_iReplyStatus==SEARCHD_RETRY )
					{
						tAgent.m_eState = AGENT_RETRY;
						break;

					} else if ( tAgent.m_iReplyStatus!=SEARCHD_OK )
					{
						CSphString sAgentError = tReq.GetString ();
						tAgent.m_sFailure.SetSprintf ( "remote error: %s", sAgentError.cstr() );
						break;
					}

					// call parser
					if ( !tParser.ParseReply ( tReq, tAgent ) )
						break;

					// check if there was enough data
					if ( tReq.GetError() )
					{
						tAgent.m_sFailure.SetSprintf ( "incomplete reply" );
						AGENT_STATS_INC ( tAgent, m_iWrongReplies );
						break;
					}

					// all is well
					iAgents++;
					tAgent.Close ();

					tAgent.m_bSuccess = true;
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
		AgentConn_t & tAgent = dAgents[iAgent];
		if ( tAgent.m_bBlackhole )
			tAgent.Close ();
		else if ( tAgent.m_eState==AGENT_QUERY )
		{
			assert ( !tAgent.m_dResults.GetLength() );
			assert ( !tAgent.m_bSuccess );
			tAgent.Close ();
			tAgent.m_sFailure.SetSprintf ( "query timed out" );
			AGENT_STATS_INC ( tAgent, m_iTimeoutsQuery );
		}
	}

	return iAgents;
}

/////////////////////////////////////////////////////////////////////////////
// SEARCH HANDLER
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

int SearchRequestBuilder_t::CalcQueryLen ( const char * sIndexes, const CSphQuery & q ) const
{
	int iReqSize = 104 + 2*sizeof(SphDocID_t) + 4*q.m_iWeights
		+ q.m_sSortBy.Length()
		+ strlen ( sIndexes )
		+ q.m_sGroupBy.Length()
		+ q.m_sGroupSortBy.Length()
		+ q.m_sGroupDistinct.Length()
		+ q.m_sComment.Length()
		+ q.m_sSelect.Length();
	if ( !q.m_bAgent ) // send the magic to agent ("*,*," + real query)
		iReqSize += q.m_sSelect.IsEmpty() ? 3 : 4;
	iReqSize += q.m_sRawQuery.IsEmpty()
		? q.m_sQuery.Length()
		: q.m_sRawQuery.Length();
	ARRAY_FOREACH ( j, q.m_dFilters )
	{
		const CSphFilterSettings & tFilter = q.m_dFilters[j];
		iReqSize += 12 + tFilter.m_sAttrName.Length(); // string attr-name; int type; int exclude-flag
		switch ( tFilter.m_eType )
		{
			case SPH_FILTER_VALUES:		iReqSize += 4 + 8*tFilter.GetNumValues (); break; // int values-count; uint64[] values
			case SPH_FILTER_RANGE:		iReqSize += 16; break; // uint64 min-val, max-val
			case SPH_FILTER_FLOATRANGE:	iReqSize += 8; break; // int/float min-val,max-val
		}
	}
	if ( q.m_bGeoAnchor )
		iReqSize += 16 + q.m_sGeoLatAttr.Length() + q.m_sGeoLongAttr.Length(); // string lat-attr, long-attr; float lat, long
	ARRAY_FOREACH ( i, q.m_dIndexWeights )
		iReqSize += 8 + q.m_dIndexWeights[i].m_sName.Length(); // string index-name; int index-weight
	ARRAY_FOREACH ( i, q.m_dFieldWeights )
		iReqSize += 8 + q.m_dFieldWeights[i].m_sName.Length(); // string field-name; int field-weight
	ARRAY_FOREACH ( i, q.m_dOverrides )
		iReqSize += 12 + q.m_dOverrides[i].m_sAttr.Length() + // string attr-name; int type; int values-count
			( q.m_dOverrides[i].m_uAttrType==SPH_ATTR_BIGINT ? 16 : 12 )*q.m_dOverrides[i].m_dValues.GetLength(); // ( bigint id; int/float/bigint value )[] values
	return iReqSize;
}


void SearchRequestBuilder_t::SendQuery ( const char * sIndexes, NetOutputBuffer_c & tOut, const CSphQuery & q ) const
{
	tOut.SendInt ( 0 ); // offset is 0
	tOut.SendInt ( q.m_iMaxMatches ); // limit is MAX_MATCHES
	tOut.SendInt ( (DWORD)q.m_eMode ); // match mode
	tOut.SendInt ( (DWORD)q.m_eRanker ); // ranking mode
	tOut.SendInt ( q.m_eSort ); // sort mode
	tOut.SendString ( q.m_sSortBy.cstr() ); // sort attr
	if ( q.m_sRawQuery.IsEmpty() )
		tOut.SendString ( q.m_sQuery.cstr() );
	else
		tOut.SendString ( q.m_sRawQuery.cstr() ); // query
	tOut.SendInt ( q.m_iWeights );
	for ( int j=0; j<q.m_iWeights; j++ )
		tOut.SendInt ( q.m_pWeights[j] ); // weights
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
				tOut.SendUint64 ( tFilter.m_uMinValue );
				tOut.SendUint64 ( tFilter.m_uMaxValue );
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
		tOut.SendDword ( tEntry.m_uAttrType );
		tOut.SendInt ( tEntry.m_dValues.GetLength() );
		ARRAY_FOREACH ( j, tEntry.m_dValues )
		{
			tOut.SendUint64 ( tEntry.m_dValues[j].m_uDocID );
			switch ( tEntry.m_uAttrType )
			{
				case SPH_ATTR_FLOAT:	tOut.SendFloat ( tEntry.m_dValues[j].m_fValue ); break;
				case SPH_ATTR_BIGINT:	tOut.SendUint64 ( tEntry.m_dValues[j].m_uValue ); break;
				default:				tOut.SendDword ( (DWORD)tEntry.m_dValues[j].m_uValue ); break;
			}
		}
	}
	if ( q.m_bAgent )
	{
		tOut.SendString ( q.m_sSelect.cstr() );
	} else
	{
		int iLen = q.m_sSelect.Length();
		if ( !iLen )
		{
			tOut.SendString ( "*,*" );
		} else
		{
			// this was a fun subtle issue
			// SetSprintf() uses a static 1K buffer...
			tOut.SendInt ( iLen+4 );
			tOut.SendBytes ( "*,*,", 4 );
			tOut.SendBytes ( q.m_sSelect.cstr(), iLen );
		}
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
			tSchema.AddAttr ( tCol, true ); // all attributes received from agents are dynamic
		}

		// get matches
		int iMatches = tReq.GetInt ();
		if ( iMatches<0 || iMatches>g_iMaxMatches )
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
				tMatch.m_iDocID = bAgent64 ? (SphDocID_t)tReq.GetUint64() : tReq.GetDword();
				tMatch.m_iWeight = tReq.GetInt ();
				for ( int j=0; j<tSchema.GetAttrsCount(); j++ )
				{
					const CSphColumnInfo & tAttr = tSchema.GetAttr(j);
					if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
					{
						tMatch.SetAttr ( tAttr.m_tLocator, m_dMvaStorage.GetLength() );

						int iValues = tReq.GetDword ();
						m_dMvaStorage.Add ( iValues );
						while ( iValues-- )
							m_dMvaStorage.Add ( tReq.GetDword() );

					} else if ( tAttr.m_eAttrType==SPH_ATTR_FLOAT )
					{
						float fRes = tReq.GetFloat();
						tMatch.SetAttr ( tAttr.m_tLocator, sphF2DW(fRes) );

					} else if ( tAttr.m_eAttrType==SPH_ATTR_BIGINT )
					{
						tMatch.SetAttr ( tAttr.m_tLocator, tReq.GetUint64() );

					} else if ( tAttr.m_eAttrType==SPH_ATTR_STRING )
					{
						CSphString sValue = tReq.GetString();
						int iLen = sValue.Length();

						int iOff = m_dStringsStorage.GetLength();
						tMatch.SetAttr ( tAttr.m_tLocator, iOff );

						m_dStringsStorage.Resize ( iOff+3+iLen );
						BYTE * pBuf = &m_dStringsStorage[iOff];
						pBuf += sphPackStrlen ( pBuf, iLen );
						memcpy ( pBuf, sValue.cstr(), iLen );

					} else
					{
						tMatch.SetAttr ( tAttr.m_tLocator, tReq.GetDword() );
					}
				}
			}
		}

		// read totals (retrieved count, total count, query time, word count)
		int iRetrieved = tReq.GetInt ();
		tRes.m_iTotalMatches = tReq.GetInt ();
		tRes.m_iQueryTime = tReq.GetInt ();
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
			const int iDocs = tReq.GetInt ();
			const int iHits = tReq.GetInt ();

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
bool MinimizeSchema ( CSphSchema & tDst, const CSphSchema & tSrc )
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
			if ( tSrcAttr.m_eAttrType!=dDst[i].m_eAttrType )
			{
				// different types? remove the attr
				iSrcIdx = -1;
				bEqual = false;

			} else if ( tSrcAttr.m_tLocator.m_iBitCount!=dDst[i].m_tLocator.m_iBitCount )
			{
				// different bit sizes? choose the max one
				dDst[i].m_tLocator.m_iBitCount = Max ( dDst[i].m_tLocator.m_iBitCount, tSrcAttr.m_tLocator.m_iBitCount );
				bEqual = false;
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

	tDst.ResetAttrs ();
	ARRAY_FOREACH ( i, dDst )
		tDst.AddAttr ( dDst[i], dDst[i].m_tLocator.m_bDynamic | !bEqual ); // force dynamic attrs on inequality

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

		CSphFilterSettings tFilter;
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

		CSphFilterSettings tFilter;
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


void CheckQuery ( const CSphQuery & tQuery, CSphString & sError )
{
	sError = NULL;
	if ( tQuery.m_eMode<0 || tQuery.m_eMode>SPH_MATCH_TOTAL )
	{
		sError.SetSprintf ( "invalid match mode %d", tQuery.m_eMode );
		return;
	}
	if ( tQuery.m_eRanker<0 || tQuery.m_eRanker>SPH_RANK_TOTAL )
	{
		sError.SetSprintf ( "invalid ranking mode %d", tQuery.m_eRanker );
		return;
	}
	if ( tQuery.m_iMaxMatches<1 || tQuery.m_iMaxMatches>g_iMaxMatches )
	{
		sError.SetSprintf ( "per-query max_matches=%d out of bounds (per-server max_matches=%d)",
			tQuery.m_iMaxMatches, g_iMaxMatches );
		return;
	}
	if ( tQuery.m_iOffset<0 || tQuery.m_iOffset>=tQuery.m_iMaxMatches )
	{
		sError.SetSprintf ( "offset out of bounds (offset=%d, max_matches=%d)",
			tQuery.m_iOffset, tQuery.m_iMaxMatches );
		return;
	}
	if ( tQuery.m_iLimit<0 )
	{
		sError.SetSprintf ( "limit out of bounds (limit=%d)", tQuery.m_iLimit );
		return;
	}
	if ( tQuery.m_iCutoff<0 )
	{
		sError.SetSprintf ( "cutoff out of bounds (cutoff=%d)", tQuery.m_iCutoff );
		return;
	}
	if ( tQuery.m_iRetryCount<0 || tQuery.m_iRetryCount>MAX_RETRY_COUNT )
	{
		sError.SetSprintf ( "retry count out of bounds (count=%d)", tQuery.m_iRetryCount );
		return;
	}
	if ( tQuery.m_iRetryDelay<0 || tQuery.m_iRetryDelay>MAX_RETRY_DELAY )
	{
		sError.SetSprintf ( "retry delay out of bounds (delay=%d)", tQuery.m_iRetryDelay );
		return;
	}
}


void PrepareQueryEmulation ( CSphQuery * pQuery )
{
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

	if ( pQuery->m_eMode!=SPH_MATCH_ALL && pQuery->m_eMode!=SPH_MATCH_ANY && pQuery->m_eMode!=SPH_MATCH_PHRASE )
		return;

	const char * szQuery = pQuery->m_sRawQuery.cstr ();
	int iQueryLen = szQuery ? strlen(szQuery) : 0;

	pQuery->m_sQuery.Reserve ( iQueryLen*2+8 );
	char * szRes = (char*) pQuery->m_sQuery.cstr ();
	char c;

	if ( pQuery->m_eMode==SPH_MATCH_ANY || pQuery->m_eMode==SPH_MATCH_PHRASE )
		*szRes++ = '\"';

	while ( ( c = *szQuery++ )!=0 )
	{
		// must be in sync with EscapeString (php api)
		if ( c=='(' || c==')' || c=='|' || c=='-' || c=='!' || c=='@' || c=='~' || c=='\"' || c=='&' || c=='/' || c=='<' || c=='\\' )
			*szRes++ = '\\';

		*szRes++ = c;
	}

	switch ( pQuery->m_eMode )
	{
	case SPH_MATCH_ALL:		pQuery->m_eRanker = SPH_RANK_PROXIMITY; *szRes = '\0'; break;
	case SPH_MATCH_ANY:		pQuery->m_eRanker = SPH_RANK_MATCHANY; strncpy ( szRes, "\"/1", 8 ); break;
	case SPH_MATCH_PHRASE:	pQuery->m_eRanker = SPH_RANK_PROXIMITY; *szRes++ = '\"'; *szRes = '\0'; break;
	default:				return;
	}
}


bool ParseSearchQuery ( InputBuffer_c & tReq, CSphQuery & tQuery, int iVer )
{
	tQuery.m_iOldVersion = iVer;

	// v.1.0. mode, limits, weights, ID/TS ranges
	tQuery.m_iOffset = tReq.GetInt ();
	tQuery.m_iLimit = tReq.GetInt ();
	tQuery.m_eMode = (ESphMatchMode) tReq.GetInt ();
	if ( iVer>=0x110 )
		tQuery.m_eRanker = (ESphRankMode) tReq.GetInt ();
	tQuery.m_eSort = (ESphSortOrder) tReq.GetInt ();
	if ( iVer<=0x101 )
		tQuery.m_iOldGroups = tReq.GetDwords ( &tQuery.m_pOldGroups, g_iMaxFilterValues, "invalid group count %d (should be in 0..%d range)" );
	if ( iVer>=0x102 )
	{
		tQuery.m_sSortBy = tReq.GetString ();
		tQuery.m_sSortBy.ToLower ();
	}
	tQuery.m_sRawQuery = tReq.GetString ();
	tQuery.m_iWeights = tReq.GetDwords ( (DWORD**)&tQuery.m_pWeights, SPH_MAX_FIELDS, "invalid weight count %d (should be in 0..%d range)" );
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
		if ( iAttrFilters>g_iMaxFilters )
		{
			tReq.SendErrorReply ( "too much attribute filters (req=%d, max=%d)", iAttrFilters, g_iMaxFilters );
			return false;
		}

		const int MAX_ERROR_SET_BUFFER = 128;
		char sSetError[MAX_ERROR_SET_BUFFER];
		tQuery.m_dFilters.Resize ( iAttrFilters );
		ARRAY_FOREACH ( iFilter, tQuery.m_dFilters )
		{
			CSphFilterSettings & tFilter = tQuery.m_dFilters[iFilter];
			tFilter.m_sAttrName = tReq.GetString ();
			tFilter.m_sAttrName.ToLower ();

			snprintf ( sSetError, MAX_ERROR_SET_BUFFER
				, "invalid attribute '%s'(%d) set length %s (should be in 0..%s range)", tFilter.m_sAttrName.cstr(), iFilter, "%d", "%d" );

			if ( iVer>=0x10E )
			{
				// v.1.14+
				tFilter.m_eType = (ESphFilter) tReq.GetDword ();
				switch ( tFilter.m_eType )
				{
					case SPH_FILTER_RANGE:
						tFilter.m_uMinValue = ( iVer>=0x114 ) ? tReq.GetUint64() : tReq.GetDword ();
						tFilter.m_uMaxValue = ( iVer>=0x114 ) ? tReq.GetUint64() : tReq.GetDword ();
						break;

					case SPH_FILTER_FLOATRANGE:
						tFilter.m_fMinValue = tReq.GetFloat ();
						tFilter.m_fMaxValue = tReq.GetFloat ();
						break;

					case SPH_FILTER_VALUES:
						{
							bool bRes = ( iVer>=0x114 )
								? tReq.GetQwords ( tFilter.m_dValues, g_iMaxFilterValues, sSetError )
								: tReq.GetDwords ( tFilter.m_dValues, g_iMaxFilterValues, sSetError );
							if ( !bRes )
								return false;
						}
						break;

					default:
						tReq.SendErrorReply ( "unknown filter type (type-id=%d)", tFilter.m_eType );
						return false;
				}

			} else
			{
				// pre-1.14
				if ( !tReq.GetDwords ( tFilter.m_dValues, g_iMaxFilterValues, sSetError ) )
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

	// now add id range filter
	if ( uMinID!=0 || uMaxID!=DOCID_MAX )
	{
		CSphFilterSettings & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_sAttrName = "@id";
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_uMinValue = uMinID;
		tFilter.m_uMaxValue = uMaxID;
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
			tOverride.m_uAttrType = tReq.GetDword ();

			tOverride.m_dValues.Resize ( tReq.GetInt() ); // FIXME! add sanity check
			ARRAY_FOREACH ( iVal, tOverride.m_dValues )
			{
				CSphAttrOverride::IdValuePair_t & tEntry = tOverride.m_dValues[iVal];
				tEntry.m_uDocID = (SphDocID_t) tReq.GetUint64 ();

				if ( tOverride.m_uAttrType==SPH_ATTR_FLOAT )		tEntry.m_fValue = tReq.GetFloat ();
				else if ( tOverride.m_uAttrType==SPH_ATTR_BIGINT )	tEntry.m_uValue = tReq.GetUint64 ();
				else												tEntry.m_uValue = tReq.GetDword ();
			}
		}
	}

	// v.1.22
	if ( iVer>=0x116 )
	{
		CSphString sRawSelect = tReq.GetString ();
		CSphString sSelect = "";
		CSphString sError;
		bool bAgent = false;
		if ( sRawSelect.Begins ( "*,*" ) ) // this is the mark of agent.
		{
			bAgent = true;
			if ( sRawSelect.Length()>3 )
				sSelect = sRawSelect.SubString ( 4, sRawSelect.Length()-4 );
		}

		tQuery.m_sSelect = bAgent?sSelect:sRawSelect;
		if ( !tQuery.ParseSelectList ( sError ) )
		{
			tReq.SendErrorReply ( "select: %s", sError.cstr() );
			return false;
		}
		if ( bAgent )
		{
			tQuery.m_sSelect = sRawSelect;
			tQuery.m_bAgent = true;
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

	CSphString sError;
	CheckQuery ( tQuery, sError );
	if ( !sError.IsEmpty() )
	{
		tReq.SendErrorReply ( "%s", sError.cstr() );
		return false;
	}

	// now prepare it for the engine
	PrepareQueryEmulation ( &tQuery );

	// all ok
	return true;
}


void LogQuery ( const CSphQuery & tQuery, const CSphQueryResult & tRes )
{
	if ( g_iQueryLogFile<0 || !tRes.m_sError.IsEmpty() )
		return;

	char sBuf[2048];
	char * p = sBuf;
	char * pMax = sBuf+sizeof(sBuf)-4;

	// [time]
	*p++ = '[';
	p += sphFormatCurrentTime ( p, pMax-p );
	*p++ = ']';

	// querytime sec
	int iQueryTime = Max ( tRes.m_iQueryTime, 0 );
	p += snprintf ( p, pMax-p, " %d.%03d sec", iQueryTime/1000, iQueryTime%1000 );

	// optional multi-query multiplier
	if ( tRes.m_iMultiplier>1 )
		p += snprintf ( p, pMax-p, " x%d", tRes.m_iMultiplier );

	// [matchmode/numfilters/sortmode matches (offset,limit)
	static const char * sModes [ SPH_MATCH_TOTAL ] = { "all", "any", "phr", "bool", "ext", "scan", "ext2" };
	static const char * sSort [ SPH_SORT_TOTAL ] = { "rel", "attr-", "attr+", "tsegs", "ext", "expr" };
	p += snprintf ( p, pMax-p, " [%s/%d/%s %d (%d,%d)",
		sModes [ tQuery.m_eMode ], tQuery.m_dFilters.GetLength(), sSort [ tQuery.m_eSort ],
		tRes.m_iTotalMatches, tQuery.m_iOffset, tQuery.m_iLimit );

	// optional groupby info
	if ( !tQuery.m_sGroupBy.IsEmpty() )
		p += snprintf ( p, pMax-p, " @%s", tQuery.m_sGroupBy.cstr() );

	// ] [indexes]
	p += snprintf ( p, pMax-p, "] [%s]", tQuery.m_sIndexes.cstr() );

	// optional performance counters
	if ( g_bIOStats || g_bCpuStats )
	{
		const CSphIOStats & IOStats = sphStopIOStats ();

		*p++ = ' ';
		char * pBracket = p; // can't fill yet, will be overwritten by sprintfs

		if ( g_bIOStats )
			p += snprintf ( p, pMax-p, " ios=%d kb=%d.%d ioms=%d.%d",
				IOStats.m_iReadOps, (int)( IOStats.m_iReadBytes/1024 ), (int)( IOStats.m_iReadBytes%1024 )*10/1024,
				(int)( IOStats.m_iReadTime/1000 ), (int)( IOStats.m_iReadTime%1000 )/100 );

		if ( g_bCpuStats )
			p += snprintf ( p, pMax-p, " cpums=%d.%d", (int)( tRes.m_iCpuTime/1000 ), (int)( tRes.m_iCpuTime%1000 )/100 );

		*pBracket = '[';
		if ( p<pMax )
			*p++ = ']';
	}

	// optional query comment
	if ( !tQuery.m_sComment.IsEmpty() )
		p += snprintf ( p, pMax-p, " [%s]", tQuery.m_sComment.cstr() );

	// query
	// (m_sRawQuery is empty when using MySQL handler)
	const CSphString & sQuery = tQuery.m_sRawQuery.IsEmpty()
		? tQuery.m_sQuery
		: tQuery.m_sRawQuery;

	if ( !sQuery.IsEmpty() )
	{
		if ( p < pMax-1 )
			*p++ = ' ';

		for ( const char * q = sQuery.cstr(); p<pMax && *q; p++, q++ )
			*p = ( *q=='\n' ) ? ' ' : *q;
	}

	// line feed
	if ( p<pMax-1 )
	{
		*p++ = '\n';
		*p++ = '\0';
	}
	sBuf[sizeof(sBuf)-2] = '\n';
	sBuf[sizeof(sBuf)-1] = '\0';

	lseek ( g_iQueryLogFile, 0, SEEK_END );
	sphWrite ( g_iQueryLogFile, sBuf, strlen(sBuf) );
}


int CalcResultLength ( int iVer, const CSphQueryResult * pRes, const CSphVector<PoolPtrs_t> & dTag2Pools )
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

	if ( iVer>=0x114 )
	{
		// 64bit matches
		int iWideAttrs = 0;
		for ( int i=0; i<pRes->m_tSchema.GetAttrsCount(); i++ )
			if ( pRes->m_tSchema.GetAttr(i).m_eAttrType==SPH_ATTR_BIGINT )
				iWideAttrs++;
		iRespLen += 4*pRes->m_iCount*iWideAttrs; // extra 4 bytes per attr per match
	}

	pRes->m_hWordStats.IterateStart();
	while ( pRes->m_hWordStats.IterateNext() ) // per-word stats
		iRespLen += 12 + strlen ( pRes->m_hWordStats.IterateGetKey().cstr() ); // wordlen, word, docs, hits

	// MVA and string values
	CSphVector<CSphAttrLocator> dMvaItems;
	CSphVector<CSphAttrLocator> dStringItems;
	for ( int i=0; i<pRes->m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = pRes->m_tSchema.GetAttr(i);
		if ( tCol.m_eAttrType & SPH_ATTR_MULTI )
			dMvaItems.Add ( tCol.m_tLocator );
		if ( tCol.m_eAttrType==SPH_ATTR_STRING )
			dStringItems.Add ( tCol.m_tLocator );
	}

	if ( iVer>=0x10C && dMvaItems.GetLength() )
	{
		for ( int i=0; i<pRes->m_iCount; i++ )
		{
			const CSphMatch & tMatch = pRes->m_dMatches [ pRes->m_iOffset+i ];
			const DWORD * pMvaPool = dTag2Pools [ tMatch.m_iTag ].m_pMva;
			assert ( !dMvaItems.GetLength() || pMvaPool );
			ARRAY_FOREACH ( j, dMvaItems )
			{
				const DWORD * pMva = tMatch.GetAttrMVA ( dMvaItems[j], pMvaPool );
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

	return iRespLen;
}


void SendResult ( int iVer, NetOutputBuffer_c & tOut, const CSphQueryResult * pRes, const CSphVector<PoolPtrs_t> & dTag2Pools )
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
			if ( g_bOptNoDetach )
				sphInfo ( "query error: %s", pRes->m_sError.cstr() );
			return;

		} else if ( bWarning )
		{
			tOut.SendInt ( SEARCHD_WARNING );
			tOut.SendString ( pRes->m_sWarning.cstr() );
			if ( g_bOptNoDetach )
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

	// send schema
	if ( iVer>=0x102 )
	{
		tOut.SendInt ( pRes->m_tSchema.m_dFields.GetLength() );
		ARRAY_FOREACH ( i, pRes->m_tSchema.m_dFields )
			tOut.SendString ( pRes->m_tSchema.m_dFields[i].m_sName.cstr() );

		tOut.SendInt ( pRes->m_tSchema.GetAttrsCount() );
		for ( int i=0; i<pRes->m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tCol = pRes->m_tSchema.GetAttr(i);
			tOut.SendString ( tCol.m_sName.cstr() );
			tOut.SendDword ( (DWORD)tCol.m_eAttrType );
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
			tOut.SendDword ( iGIDLoc.m_iBitOffset>=0 ? (DWORD) tMatch.GetAttr ( iGIDLoc ) : 1 );
			tOut.SendDword ( iTSLoc.m_iBitOffset>=0 ? (DWORD) tMatch.GetAttr ( iTSLoc ) : 1 );
			tOut.SendInt ( tMatch.m_iWeight );
		} else
		{
			tOut.SendInt ( tMatch.m_iWeight );

			const DWORD * pMvaPool = dTag2Pools [ tMatch.m_iTag ].m_pMva;
			const BYTE * pStrings = dTag2Pools [ tMatch.m_iTag ].m_pStrings;

			assert ( tMatch.m_pStatic || !pRes->m_tSchema.GetStaticSize() );
			assert ( tMatch.m_pDynamic || !pRes->m_tSchema.GetDynamicSize() );
			assert ( !tMatch.m_pDynamic || (int)tMatch.m_pDynamic[-1]==pRes->m_tSchema.GetDynamicSize() );

			for ( int j=0; j<pRes->m_tSchema.GetAttrsCount(); j++ )
			{
				const CSphColumnInfo & tAttr = pRes->m_tSchema.GetAttr(j);
				if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
				{
					assert ( pMvaPool );
					const DWORD * pValues = tMatch.GetAttrMVA ( tAttr.m_tLocator, pMvaPool );
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
						while ( iValues-- )
							tOut.SendDword ( *pValues++ );
					}

				} else if ( tAttr.m_eAttrType==SPH_ATTR_STRING )
				{
					// send string attr
					if ( iVer<0x117 )
					{
						// for older clients, just send int value of 0
						tOut.SendDword ( 0 );
					} else
					{
						// for newer clients, send binary string
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
					}

				} else
				{
					// send plain attr
					if ( tAttr.m_eAttrType==SPH_ATTR_FLOAT )
						tOut.SendFloat ( tMatch.GetAttrFloat ( tAttr.m_tLocator ) );
					else if ( iVer>=0x114 && tAttr.m_eAttrType==SPH_ATTR_BIGINT )
						tOut.SendUint64 ( tMatch.GetAttr ( tAttr.m_tLocator ) );
					else
						tOut.SendDword ( (DWORD)tMatch.GetAttr ( tAttr.m_tLocator ) );
				}
			}
		}
	}
	tOut.SendInt ( pRes->m_dMatches.GetLength() );
	tOut.SendInt ( pRes->m_iTotalMatches );
	tOut.SendInt ( Max ( pRes->m_iQueryTime, 0 ) );
	tOut.SendInt ( pRes->m_hWordStats.GetLength() );

	pRes->m_hWordStats.IterateStart();
	while ( pRes->m_hWordStats.IterateNext() )
	{
		const CSphQueryResultMeta::WordStat_t & tStat = pRes->m_hWordStats.IterateGet();
		tOut.SendString ( pRes->m_hWordStats.IterateGetKey().cstr() );
		tOut.SendInt ( tStat.m_iDocs );
		tOut.SendInt ( tStat.m_iHits );
	}
}

/////////////////////////////////////////////////////////////////////////////

struct AggrResult_t : CSphQueryResult
{
	int							m_iTag;				///< current tag
	CSphVector<CSphSchema>		m_dSchemas;			///< aggregated resultsets schemas (for schema minimization)
	CSphVector<int>				m_dMatchCounts;		///< aggregated resultsets lengths (for schema minimization)
	CSphVector<int>				m_dIndexWeights;	///< aggregated resultsets per-index weights (optional)
	CSphVector<PoolPtrs_t>		m_dTag2Pools;		///< tag to MVA and strings storage pools mapping
};


template < typename T, typename U >
struct CSphPair
{
	T m_tFirst;
	U m_tSecond;
};


struct TaggedMatchSorter_fn : public SphAccessor_T<CSphMatch>
{
	void CopyKey ( CSphMatch * pMed, CSphMatch * pVal ) const
	{
		pMed->m_iDocID = pVal->m_iDocID;
		pMed->m_iTag = pVal->m_iTag;
	}

	bool IsLess ( const CSphMatch & a, const CSphMatch & b ) const
	{
		return ( a.m_iDocID < b.m_iDocID ) || ( a.m_iDocID==b.m_iDocID && a.m_iTag > b.m_iTag );
	}

	// inherited swap does not work on gcc
	void Swap ( CSphMatch * a, CSphMatch * b ) const
	{
		::Swap ( *a, *b );
	}
};

void RemapResult ( const CSphSchema& tTarget, AggrResult_t & tRes, bool bMultiSchema=true )
{
	int iCur = 0;
	int * dMapFrom = NULL;

	if ( tTarget.GetRowSize() )
		dMapFrom = new int [ tTarget.GetAttrsCount() ];

	ARRAY_FOREACH ( iSchema, tRes.m_dSchemas )
	{
		CSphSchema& dSchema = bMultiSchema?tRes.m_dSchemas[iSchema]:tRes.m_tSchema;
		for ( int i=0; i<tTarget.GetAttrsCount(); i++ )
		{
			dMapFrom[i] = dSchema.GetAttrIndex ( tTarget.GetAttr(i).m_sName.cstr() );
			assert ( dMapFrom[i]>=0 );
		}
		int iLimit = bMultiSchema
			? ( iCur + tRes.m_dMatchCounts[iSchema] )
			: tRes.m_iTotalMatches;
		iLimit = Min ( iLimit, tRes.m_dMatches.GetLength() );
		for ( int i=iCur;i<iLimit; i++ )
		{
			CSphMatch & tMatch = tRes.m_dMatches[i];

			// create new and shiny (and properly sized and fully dynamic) match
			CSphMatch tRow;
			tRow.Reset ( tTarget.GetDynamicSize() );
			tRow.m_iDocID = tMatch.m_iDocID;
			tRow.m_iWeight = tMatch.m_iWeight;
			tRow.m_iTag = tMatch.m_iTag;

			// remap attrs
			for ( int j=0; j<tTarget.GetAttrsCount(); j++ )
			{
				const CSphColumnInfo & tDst = tTarget.GetAttr(j);
				const CSphColumnInfo & tSrc = dSchema.GetAttr ( dMapFrom[j] );
				if ( !tDst.m_tLocator.m_bDynamic )
				{
					assert ( !tSrc.m_tLocator.m_bDynamic );
					tRow.m_pStatic = tMatch.m_pStatic;
				} else
					tRow.SetAttr ( tDst.m_tLocator, tMatch.GetAttr ( tSrc.m_tLocator ) );
			}

			// swap out old (most likely wrong sized) match
			Swap ( tMatch, tRow );
		}

		if ( !bMultiSchema )
			break;

		iCur = iLimit;
	}

	if ( bMultiSchema )
		assert ( iCur==tRes.m_dMatches.GetLength() );
	SafeDeleteArray ( dMapFrom );
	if ( &tRes.m_tSchema!=&tTarget )
		tRes.m_tSchema = tTarget;
}

// just to avoid the const_cast of the schema (i.e, return writable columns)
// also to make possible several members refer to one and same locator.
class CVirtualSchema : public CSphSchema
{
public:
	inline CSphColumnInfo & LastColumn() { return m_dAttrs.Last(); }
	inline CSphColumnInfo &	GetWAttr ( int iIndex ) { return m_dAttrs[iIndex]; }
	inline CSphVector<CSphColumnInfo> & GetWAttrs () { return m_dAttrs; }
	inline void AlignSizes ( const CSphSchema& tProof )
	{
		m_dDynamicUsed.Resize ( tProof.GetDynamicSize() );
		m_iStaticSize = tProof.GetStaticSize();
	}
};

// swap the schema into the new one
void RenameApplyAliases ( AggrResult_t & tRes, CVirtualSchema * tSchema )
{
	tSchema->AlignSizes ( tRes.m_tSchema );
	tSchema->m_dFields = tRes.m_tSchema.m_dFields;
	tRes.m_tSchema = *tSchema;
}

bool MinimizeAggrResult ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bHadLocalIndexes, CSphSchema* pExtraSchema=NULL )
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
	tRes.m_tSchema = tRes.m_dSchemas[0];
	bool bAllEqual = true;
	bool bAgent = tQuery.m_bAgent;

	for ( int i=1; i<tRes.m_dSchemas.GetLength(); i++ )
	{
		if ( !MinimizeSchema ( tRes.m_tSchema, tRes.m_dSchemas[i] ) )
			bAllEqual = false;
	}

	// apply select-items on top of that
	bool bStar = false;
	ARRAY_FOREACH ( i, tQuery.m_dItems )
		if ( tQuery.m_dItems[i].m_sExpr=="*" )
	{
		bStar = true;
		break;
	}

	// the final result schema - for collections, etc
	// we can't construct the random final schema right now, since
	// the final sorter needs the schema fields in specific order:
	CVirtualSchema tItems;
	// truly virtual schema for final result returning
	CVirtualSchema tFinalItems;
	if ( !bStar )
		tFinalItems.GetWAttrs().Resize ( tQuery.m_dItems.GetLength() );

	if ( !bStar && tQuery.m_dItems.GetLength() )
	{
		for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
			if ( !tCol.m_pExpr )
			{
				bool bAdd = false;
				ARRAY_FOREACH ( j, tQuery.m_dItems )
				{
					const CSphQueryItem & tQueryItem = tQuery.m_dItems[j];
					if ( tFinalItems.GetAttr(j).m_iIndex<0
						&& ( ( tQueryItem.m_sExpr.cstr() && tQueryItem.m_sExpr==tCol.m_sName && tQueryItem.m_eAggrFunc==SPH_AGGR_NONE )
						|| ( tQueryItem.m_sAlias.cstr() && tQueryItem.m_sAlias==tCol.m_sName ) ) )
					{
						bAdd = true;
						if ( !bAgent )
						{
							CSphColumnInfo & tItem = tFinalItems.GetWAttr(j);
							tItem.m_iIndex = tItems.GetAttrsCount(); // temporary idx, will change to locator by this index
							if ( tQueryItem.m_sAlias.cstr() )
								tItem.m_sName = tQueryItem.m_sAlias;
							else
								tItem.m_sName = tQueryItem.m_sExpr;
						} else
							break;
					}
				}
				if ( !bAdd && pExtraSchema!=NULL )
				{
					if ( pExtraSchema->GetAttrsCount() )
					{
						for ( int j=0; j<pExtraSchema->GetAttrsCount(); j++ )
							if ( pExtraSchema->GetAttr(j).m_sName==tCol.m_sName )
							{
								bAdd = true;
								break;
							}
					// the extra schema is not null, but empty - and we have no local agents
					// so, the schema of result is already aligned to the extra, just add it
					} else if ( !bHadLocalIndexes )
						bAdd = true;
				}

				if ( !bAdd )
					continue;
			} else if ( !bAgent )
			{
				ARRAY_FOREACH ( j, tQuery.m_dItems )
					if ( tFinalItems.GetAttr(j).m_iIndex<0
						&& ( tQuery.m_dItems[j].m_sAlias.cstr() && tQuery.m_dItems[j].m_sAlias==tCol.m_sName ) )
					{
						CSphColumnInfo & tItem = tFinalItems.GetWAttr(j);
						tItem.m_iIndex = tItems.GetAttrsCount();
						tItem.m_sName = tQuery.m_dItems[j].m_sAlias;
					}
			}
			// if before all schemas were proved as equal, and the tCol taken from current schema is static -
			// this is no reason now to make it dynamic.
			bool bDynamic = bAllEqual?tCol.m_tLocator.m_bDynamic:true;
			tItems.AddAttr ( tCol, bDynamic );
			if ( !bDynamic )
			{
				// all schemas are equal, so all offsets and bitcounts also equal.
				// If we Add the static attribute which already exists in result, we need
				// not to corrupt it's locator. So, in this case let us force the locator
				// to the old data.
				CSphColumnInfo & tNewCol = tItems.LastColumn();
				assert ( !tNewCol.m_tLocator.m_bDynamic );
				tNewCol.m_tLocator = tCol.m_tLocator;
			}
		}

		bAllEqual &= tRes.m_tSchema.GetAttrsCount()==tItems.GetAttrsCount();
	}

	// finalize the tFinalItems - switch back m_iIndex field
	// and set up the locators for the fields
	if ( !bAgent )
	{
		// fully custom schema
		if ( !bStar )
		{
			ARRAY_FOREACH ( i, tFinalItems.GetWAttrs() )
			{
				CSphColumnInfo & tCol = tFinalItems.GetWAttr(i);
				const CSphColumnInfo & tSource = tItems.GetAttr ( tCol.m_iIndex );
				tCol.m_tLocator = tSource.m_tLocator;
				tCol.m_eAttrType = tSource.m_eAttrType;
				tCol.m_iIndex = -1;
			}
		} else
		// star schema - remove the @ if it is not agent
		{
			// assign the star as the name of the schema
			// it will be used later as the mark to attach document ID at the beginning of resultset
			// when returning results to sphinxql client
			tFinalItems.m_sName = "*";
			for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); i++ )
			{
				const CSphColumnInfo & tSource = tRes.m_tSchema.GetAttr(i);
				if ( tSource.m_sName.cstr()[0]!='@')
					tFinalItems.GetWAttrs().Add ( tSource );
			}
		}
	}

	// tricky bit
	// in purely distributed case, all schemas are received from the wire, and miss aggregate functions info
	// thus, we need to re-assign that info
	if ( !bHadLocalIndexes )
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

	// we do not need to re-sort if there's exactly one result set
	if ( tRes.m_iSuccesses==1 )
	{
		// convert all matches to minimal schema
		if ( !bAllEqual )
			RemapResult ( bStar?tRes.m_tSchema:tItems, tRes );
		if ( !bAgent )
			RenameApplyAliases ( tRes, &tFinalItems );
		return true;
	}

	// if there's more than one result set, we need to re-sort the matches
	// so we need to bring matches to the schema that the *sorter* wants
	// so we need to create the sorter before conversion
	//
	// create queue
	// at this point, we do not need to compute anything; it all must be here
	ISphMatchSorter * pSorter = sphCreateQueue ( &tQuery, tRes.m_tSchema, tRes.m_sError, false );
	if ( !pSorter )
		return false;

	// sorter expects this
	tRes.m_tSchema = pSorter->GetSchema();

	// convert all matches to sorter schema - at least to manage all static to dynamic
	if ( !bAllEqual )
		RemapResult ( tRes.m_tSchema, tRes );

	// kill all dupes
	int iDupes = 0;
	assert ( pSorter );
	if ( pSorter->IsGroupby () )
	{
		// groupby sorter does that automagically
		pSorter->SetMVAPool ( NULL ); // because we must be able to group on @groupby anyway
		ARRAY_FOREACH ( i, tRes.m_dMatches )
		{
			CSphMatch & tMatch = tRes.m_dMatches[i];

			if ( tRes.m_dIndexWeights.GetLength() && tMatch.m_iTag>=0 )
				tMatch.m_iWeight *= tRes.m_dIndexWeights[tMatch.m_iTag];

			if ( !pSorter->PushGrouped ( tMatch ) )
				iDupes++;
		}
	} else
	{
		// normal sorter needs massasging
		// sort by docid and then by tag to guarantee the replacement order
		TaggedMatchSorter_fn fnSort;
		sphSort ( &tRes.m_dMatches[0], tRes.m_dMatches.GetLength(), fnSort, fnSort );

		// fold them matches
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

	if ( !bAllEqual )
		RemapResult ( bStar?tRes.m_tSchema:tItems, tRes, false );
	if ( !bAgent )
		RenameApplyAliases ( tRes, &tFinalItems );
	return true;
}

bool MinimizeAggrResultCompat ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bHadLocalIndexes )
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
	tRes.m_tSchema = tRes.m_dSchemas[0];
	for ( int i=1; i<tRes.m_dSchemas.GetLength(); i++ )
	{
		if ( !MinimizeSchema ( tRes.m_tSchema, tRes.m_dSchemas[i] ) )
			bAllEqual = false;
	}

	// apply select-items on top of that
	bool bStar = false;
	ARRAY_FOREACH ( i, tQuery.m_dItems )
		if ( tQuery.m_dItems[i].m_sExpr=="*" )
		{
			bStar = true;
			break;
		}

		if ( !bStar && tQuery.m_dItems.GetLength() )
		{
			CSphSchema tItems;
			for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); i++ )
			{
				const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
				if ( !tCol.m_pExpr )
				{
					bool bAdd = false;
					ARRAY_FOREACH ( i, tQuery.m_dItems )
						if ( tQuery.m_dItems[i].m_sExpr==tCol.m_sName )
						{
							bAdd = true;
							break;
						}

						if ( !bAdd )
							continue;
				}
				tItems.AddAttr ( tCol, true );
			}

			if ( tRes.m_tSchema.GetAttrsCount()!=tItems.GetAttrsCount() )
			{
				tRes.m_tSchema = tItems;
				bAllEqual = false;
			}
		}

		// tricky bit
		// in purely distributed case, all schemas are received from the wire, and miss aggregate functions info
		// thus, we need to re-assign that info
		if ( !bHadLocalIndexes )
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

		// if there's more than one result set, we need to re-sort the matches
		// so we need to bring matches to the schema that the *sorter* wants
		// so we need to create the sorter before conversion
		ISphMatchSorter * pSorter = NULL;
		if ( tRes.m_iSuccesses!=1 )
		{
			// create queue
			// at this point, we do not need to compute anything; it all must be here
			pSorter = sphCreateQueue ( &tQuery, tRes.m_tSchema, tRes.m_sError, false );
			if ( !pSorter )
				return false;

			// sorter expects this
			tRes.m_tSchema = pSorter->GetSchema();
		}

		// convert all matches to minimal schema
		if ( !bAllEqual )
		{
			int iCur = 0;
			int * dMapFrom = NULL;

			if ( tRes.m_tSchema.GetRowSize() )
				dMapFrom = new int [ tRes.m_tSchema.GetAttrsCount() ];

			ARRAY_FOREACH ( iSchema, tRes.m_dSchemas )
			{
				for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); i++ )
				{
					dMapFrom[i] = tRes.m_dSchemas[iSchema].GetAttrIndex ( tRes.m_tSchema.GetAttr(i).m_sName.cstr() );
					assert ( dMapFrom[i]>=0 );
				}

				for ( int i=iCur; i<iCur+tRes.m_dMatchCounts[iSchema]; i++ )
				{
					CSphMatch & tMatch = tRes.m_dMatches[i];

					// create new and shiny (and properly sized and fully dynamic) match
					CSphMatch tRow;
					tRow.Reset ( tRes.m_tSchema.GetRowSize() );
					tRow.m_iDocID = tMatch.m_iDocID;
					tRow.m_iWeight = tMatch.m_iWeight;
					tRow.m_iTag = tMatch.m_iTag;

					// remap attrs
					for ( int j=0; j<tRes.m_tSchema.GetAttrsCount(); j++ )
					{
						const CSphColumnInfo & tDst = tRes.m_tSchema.GetAttr(j);
						const CSphColumnInfo & tSrc = tRes.m_dSchemas[iSchema].GetAttr ( dMapFrom[j] );
						tRow.SetAttr ( tDst.m_tLocator, tMatch.GetAttr ( tSrc.m_tLocator ) );
					}

					// swap out old (most likely wrong sized) match
					Swap ( tMatch, tRow );
				}

				iCur += tRes.m_dMatchCounts[iSchema];
			}

			assert ( iCur==tRes.m_dMatches.GetLength() );
			SafeDeleteArray ( dMapFrom );
		}

		// we do not need to re-sort if there's exactly one result set
		if ( tRes.m_iSuccesses==1 )
			return true;

		// kill all dupes
		int iDupes = 0;
		if ( pSorter->IsGroupby () )
		{
			// groupby sorter does that automagically
			pSorter->SetMVAPool ( NULL ); // because we must be able to group on @groupby anyway
			ARRAY_FOREACH ( i, tRes.m_dMatches )
			{
				CSphMatch & tMatch = tRes.m_dMatches[i];

				if ( tRes.m_dIndexWeights.GetLength() && tMatch.m_iTag>=0 )
					tMatch.m_iWeight *= tRes.m_dIndexWeights[tMatch.m_iTag];

				if ( !pSorter->PushGrouped ( tMatch ) )
					iDupes++;
			}
		} else
		{
			// normal sorter needs massasging
			// sort by docid and then by tag to guarantee the replacement order
			TaggedMatchSorter_fn fnSort;
			sphSort ( &tRes.m_dMatches[0], tRes.m_dMatches.GetLength(), fnSort, fnSort );

			// fold them matches
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


void SetupKillListFilter ( CSphFilterSettings & tFilter, const SphAttr_t * pKillList, int nEntries )
{
	assert ( nEntries && pKillList );

	tFilter.m_bExclude = true;
	tFilter.m_eType = SPH_FILTER_VALUES;
	tFilter.m_uMinValue = pKillList[0];
	tFilter.m_uMaxValue = pKillList[nEntries-1];
	tFilter.m_sAttrName = "@id";
	tFilter.SetExternalValues ( pKillList, nEntries );
}

/////////////////////////////////////////////////////////////////////////////

class SearchHandler_c
{
	friend void LocalSearchThreadFunc ( void * pArg );

public:
	explicit						SearchHandler_c ( int iQueries );
	void							RunQueries ();					///< run all queries, get all results

public:
	CSphVector<CSphQuery>			m_dQueries;						///< queries which i need to search
	CSphVector<AggrResult_t>		m_dResults;						///< results which i obtained
	CSphVector<SearchFailuresLog_c>	m_dFailuresSet;					///< failure logs for each query

protected:
	void							RunSubset ( int iStart, int iEnd );	///< run queries against index(es) from first query in the subset
	void							RunLocalSearches ( ISphMatchSorter * pLocalSorter, const char * sDistName );
	void							RunLocalSearchesMT ();
	bool							RunLocalSearch ( int iLocal, ISphMatchSorter ** ppSorters, CSphQueryResult ** pResults ) const;

	CSphVector<DWORD>				m_dMvaStorage;
	CSphVector<BYTE>				m_dStringsStorage;

	int								m_iStart;		///< subset start
	int								m_iEnd;			///< subset end
	bool							m_bMultiQueue;	///< whether current subset is subject to multi-queue optimization
	CSphVector<CSphString>			m_dLocal;		///< local indexes for the current subset
	volatile bool							m_bIsLocked;	///< whether local indexes are already locked
	mutable CSphVector<CSphSchema>			m_dExtraSchemas; ///< the extra fields for agents
};


SearchHandler_c::SearchHandler_c ( int iQueries )
{
	m_dQueries.Resize ( iQueries );
	m_dResults.Resize ( iQueries );
	m_dFailuresSet.Resize ( iQueries );
	m_dExtraSchemas.Resize ( iQueries );

	ARRAY_FOREACH ( i, m_dResults )
	{
		assert ( m_dResults[i].m_dIndexWeights.GetLength()==0 );
		m_dResults[i].m_iTag = 1; // first avail tag for local storage ptrs
		m_dResults[i].m_dIndexWeights.Add ( 1 ); // reserved index 0 with weight 1 for remote matches
		m_dResults[i].m_dTag2Pools.Add (); // reserved index 0 for remote mva storage ptr; we'll fix this up later
	}
}


void SearchHandler_c::RunQueries ()
{
	///////////////////////////////
	// choose path and run queries
	///////////////////////////////

	m_dMvaStorage.Reserve ( 1024 );
	m_dMvaStorage.Resize ( 0 );
	m_dMvaStorage.Add ( 0 );	// dummy value
	m_dStringsStorage.Reserve ( 1024 );
	m_dStringsStorage.Resize ( 0 );
	m_dStringsStorage.Add ( 0 );

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
	ARRAY_FOREACH ( i, m_dResults )
	{
		m_dResults[i].m_dTag2Pools[0].m_pMva = m_dMvaStorage.Begin();
		m_dResults[i].m_dTag2Pools[0].m_pStrings = m_dStringsStorage.Begin();
		m_dResults[i].m_iMatches = m_dResults[i].m_dMatches.GetLength();
	}
}


/// return cpu time, in microseconds
int64_t sphCpuTimer ()
{
#ifdef HAVE_CLOCK_GETTIME
	if ( !g_bCpuStats )
		return 0;

#if defined(CLOCK_PROCESS_CPUTIME_ID)
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
};


struct LocalSearchThreadContext_t
{
	SphThread_t					m_tThd;
	SearchHandler_c *			m_pHandler;
	CSphVector<LocalSearch_t*>	m_pSearches;
};


void LocalSearchThreadFunc ( void * pArg )
{
	LocalSearchThreadContext_t * pContext = (LocalSearchThreadContext_t*) pArg;
	ARRAY_FOREACH ( i, pContext->m_pSearches )
	{
		LocalSearch_t * pCall = pContext->m_pSearches[i];
		pCall->m_bResult = pContext->m_pHandler->RunLocalSearch ( pCall->m_iLocal, pCall->m_ppSorters, pCall->m_ppResults );
	}
}


static void MergeWordStats ( CSphQueryResultMeta & tDstResult, const SmallStringHash_T<CSphQueryResultMeta::WordStat_t> & hSrc )
{
	if ( !tDstResult.m_hWordStats.GetLength() )
	{
		// nothing has been set yet; just copy
		tDstResult.m_hWordStats = hSrc;
		return;
	}

	hSrc.IterateStart();
	while ( hSrc.IterateNext() )
		tDstResult.AddStat ( hSrc.IterateGetKey(), hSrc.IterateGet().m_iDocs, hSrc.IterateGet().m_iHits );
}

void SearchHandler_c::RunLocalSearchesMT ()
{
	int64_t tmLocal = sphMicroTimer();

	// setup local searches
	const int iQueries = m_iEnd-m_iStart+1;
	CSphVector<LocalSearch_t> dLocals ( m_dLocal.GetLength() );
	CSphVector<CSphQueryResult> dResults ( m_dLocal.GetLength()*iQueries );
	CSphVector<ISphMatchSorter*> pSorters ( m_dLocal.GetLength()*iQueries );
	CSphVector<CSphQueryResult*> pResults ( m_dLocal.GetLength()*iQueries );

	ARRAY_FOREACH ( i, pResults )
		pResults[i] = &dResults[i];

	ARRAY_FOREACH ( i, m_dLocal )
	{
		dLocals[i].m_iLocal = i;
		dLocals[i].m_ppSorters = &pSorters [ i*iQueries ];
		dLocals[i].m_ppResults = &pResults [ i*iQueries ];
	}

	// setup threads
	// FIXME! implement better than naive index:thread mapping
	// FIXME! maybe implement a thread-shared jobs queue
	CSphVector<LocalSearchThreadContext_t> dThreads ( Min ( g_iDistThreads, dLocals.GetLength() ) );
	int iCurThread = 0;

	ARRAY_FOREACH ( i, dLocals )
	{
		dThreads[iCurThread].m_pSearches.Add ( &dLocals[i] );
		iCurThread = ( iCurThread+1 ) % g_iDistThreads;
	}

	// fire searcher threads
	ARRAY_FOREACH ( i, dThreads )
	{
		dThreads[i].m_pHandler = this;
		sphThreadCreate ( &dThreads[i].m_tThd, LocalSearchThreadFunc, (void*)&dThreads[i] ); // FIXME! check result
	}

	// wait for them to complete
	ARRAY_FOREACH ( i, dThreads )
		sphThreadJoin ( &dThreads[i].m_tThd );

	// now merge the results
	ARRAY_FOREACH ( iLocal, dLocals )
	{
		bool bResult = dLocals[iLocal].m_bResult;
		const char * sLocal = m_dLocal[iLocal].cstr();

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
			// but some of the sorters could had failed at "create sorter" stage
			int iSorterIndex = iLocal*iQueries;
			int iResultIndex = iSorterIndex + iQuery - m_iStart;
			if ( !m_bMultiQueue )
				iSorterIndex = iResultIndex;

			// check per-query failures of MultiQueryEx
			else if ( dResults[iResultIndex].m_iMultiplier==-1 )
			{
				iResultIndex += iQuery - m_iStart;
				m_dFailuresSet[iQuery].Submit ( sLocal, dResults[iResultIndex].m_sError.cstr() );
				continue;
			}

			ISphMatchSorter * pSorter = pSorters[iSorterIndex];
			if ( !pSorter )
				continue;

			// this one seems OK
			AggrResult_t & tRes = m_dResults[iQuery];
			CSphQueryResult & tRaw = dResults[iResultIndex];

			tRes.m_iSuccesses++;
			tRes.m_tSchema = pSorter->GetSchema();
			tRes.m_iTotalMatches += pSorter->GetTotalCount();

			tRes.m_pMva = tRaw.m_pMva;
			tRes.m_pStrings = tRaw.m_pStrings;
			MergeWordStats ( tRes, tRaw.m_hWordStats );

			// move external attributes storage from tRaw to actual result
			tRaw.LeakStorages ( tRes );

			tRes.m_iMultiplier = m_bMultiQueue ? iQueries : 1;
			tRes.m_iCpuTime += tRaw.m_iCpuTime / tRes.m_iMultiplier;

			// extract matches from sorter
			assert ( pSorter );

			if ( pSorter->GetLength() )
			{
				tRes.m_dMatchCounts.Add ( pSorter->GetLength() );
				tRes.m_dSchemas.Add ( tRes.m_tSchema );
				tRes.m_dIndexWeights.Add ( m_dQueries[iQuery].GetIndexWeight ( sLocal ) );
				PoolPtrs_t & tPoolPtrs = tRes.m_dTag2Pools.Add ();
				tPoolPtrs.m_pMva = tRes.m_pMva;
				tPoolPtrs.m_pStrings = tRes.m_pStrings;
				sphFlattenQueue ( pSorter, &tRes, tRes.m_iTag++ );

				// clean up for next index search
				tRes.m_pMva = NULL;
				tRes.m_pStrings = NULL;
			}
		}
	}

	ARRAY_FOREACH ( i, pSorters )
		SafeDelete ( pSorters[i] );

	// update our wall time for every result set
	tmLocal = sphMicroTimer() - tmLocal;
	for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
		m_dResults[iQuery].m_iQueryTime += (int)( tmLocal/1000 );
}


bool SearchHandler_c::RunLocalSearch ( int iLocal, ISphMatchSorter ** ppSorters, CSphQueryResult ** ppResults ) const
{
	const int iQueries = m_iEnd-m_iStart+1;
	const char * sLocal = m_dLocal[iLocal].cstr();

	const ServedIndex_t * pServed = m_bIsLocked ? &g_pIndexes->GetUnlockedEntry ( sLocal ) : g_pIndexes->GetRlockedEntry ( sLocal );
	if ( !pServed )
	{
		// FIXME! submit a failure?
		return false;
	}
	assert ( pServed->m_pIndex );
	assert ( pServed->m_bEnabled );

	// create sorters
	int iValidSorters = 0;
	for ( int i=0; i<iQueries; i++ )
	{
		CSphString sError;
		const CSphQuery & tQuery = m_dQueries[i+m_iStart];
		CSphSchema * pExtraSchema = tQuery.m_bAgent?&m_dExtraSchemas[i+m_iStart]:NULL;

		assert ( !tQuery.m_iOldVersion || tQuery.m_iOldVersion>=0x102 );

		ppSorters[i] = sphCreateQueue ( &tQuery, pServed->m_pIndex->GetMatchSchema(), sError, true, pExtraSchema );
		if ( !ppSorters[i] )
		{
			// FIXME! submit a failure?
			continue;
		}
		iValidSorters++;
	}
	if ( !iValidSorters )
	{
		if ( !m_bIsLocked )
			pServed->Unlock();
		return false;
	}

	CSphVector < const ServedIndex_t * > dLocked;

	// setup kill-lists
	CSphVector<CSphFilterSettings> dKlists;
	for ( int i=iLocal+1; i<m_dLocal.GetLength (); i++ )
	{
		const ServedIndex_t * pServedIndex = m_bIsLocked ? &g_pIndexes->GetUnlockedEntry ( m_dLocal[i] ) : g_pIndexes->GetRlockedEntry ( m_dLocal[i] );
		if ( !pServedIndex )
			continue;

		const CSphIndex * pIndex = pServedIndex->m_pIndex;
		if ( pIndex->GetKillListSize() )
		{
			SetupKillListFilter ( dKlists.Add(), pIndex->GetKillList(), pIndex->GetKillListSize() );
			if ( g_eWorkers==MPM_THREADS && !m_bIsLocked )
				dLocked.Add ( pServedIndex );
		} else if ( !m_bIsLocked )
			pServedIndex->Unlock();
	}

	// do the query
	bool bResult = false;
	pServed->m_pIndex->SetCacheSize ( g_iMaxCachedDocs, g_iMaxCachedHits );
	if ( m_bMultiQueue )
	{
		bResult = pServed->m_pIndex->MultiQuery ( &m_dQueries[m_iStart], ppResults[0], iQueries, ppSorters, &dKlists );
	} else
	{
		bResult = pServed->m_pIndex->MultiQueryEx ( iQueries, &m_dQueries[m_iStart], ppResults, ppSorters, &dKlists );
	}

	if ( !m_bIsLocked )
	{
		pServed->Unlock();
		DoUnlockClear ( dLocked );
	}

	return bResult;
}


void SearchHandler_c::RunLocalSearches ( ISphMatchSorter * pLocalSorter, const char * sDistName )
{
	m_bIsLocked = pLocalSorter!=NULL;

	if ( g_iDistThreads>1 && m_dLocal.GetLength()>1 )
	{
		RunLocalSearchesMT();
		return;
	}

	CSphVector < const ServedIndex_t * > dLocked;
	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		const char * sLocal = m_dLocal[iLocal].cstr();

		const ServedIndex_t * pServed = m_bIsLocked ? &g_pIndexes->GetUnlockedEntry ( sLocal ) : g_pIndexes->GetRlockedEntry ( sLocal );
		if ( !pServed )
		{
			if ( sDistName )
				for ( int i=m_iStart; i<=m_iEnd; i++ )
					m_dFailuresSet[i].SubmitEx ( sDistName, "local index %s missing", sLocal );
			continue;
		}

		assert ( pServed->m_pIndex );
		assert ( pServed->m_bEnabled );

		// create sorters
		CSphVector<ISphMatchSorter*> dSorters ( m_iEnd-m_iStart+1 );
		ARRAY_FOREACH ( i, dSorters )
			dSorters[i] = NULL;

		int iValidSorters = 0;
		for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
		{
			CSphString sError;
			CSphQuery & tQuery = m_dQueries[iQuery];
			CSphSchema * pExtraSchema = tQuery.m_bAgent?&m_dExtraSchemas[iQuery]:NULL;

			// create sorter, if needed
			ISphMatchSorter * pSorter = pLocalSorter;
			if ( !pLocalSorter )
			{
				// fixup old queries
				if ( !FixupQuery ( &tQuery, &pServed->m_pIndex->GetMatchSchema(), sLocal, sError ) )
				{
					m_dFailuresSet[iQuery].Submit ( sLocal, sError.cstr() );
					continue;
				}

				// create queue
				pSorter = sphCreateQueue ( &tQuery, pServed->m_pIndex->GetMatchSchema(), sError, true, pExtraSchema );
				if ( !pSorter )
				{
					m_dFailuresSet[iQuery].Submit ( sLocal, sError.cstr() );
					continue;
				}
			}

			dSorters[iQuery-m_iStart] = pSorter;
			iValidSorters++;
		}
		if ( !iValidSorters )
		{
			if ( !m_bIsLocked )
				pServed->Unlock();
			continue;
		}

		// me shortcuts
		AggrResult_t tStats;
		CSphQuery * pQuery = &m_dQueries[m_iStart];

		// set kill-list
		int iNumFilters = pQuery->m_dFilters.GetLength ();
		for ( int i=iLocal+1; i<m_dLocal.GetLength (); i++ )
		{
			const ServedIndex_t * pServed = m_bIsLocked ? &g_pIndexes->GetUnlockedEntry ( m_dLocal[i] ) : g_pIndexes->GetRlockedEntry ( m_dLocal[i] );
			if ( !pServed )
				continue;

			if ( pServed->m_pIndex->GetKillListSize () )
			{
				CSphFilterSettings tKillListFilter;
				SetupKillListFilter ( tKillListFilter, pServed->m_pIndex->GetKillList (), pServed->m_pIndex->GetKillListSize () );
				pQuery->m_dFilters.Add ( tKillListFilter );

				if ( g_eWorkers==MPM_THREADS && !m_bIsLocked )
					dLocked.Add ( pServed );
			} else if ( !m_bIsLocked )
				pServed->Unlock();
		}

		// do the query
		bool bResult = false;
		pServed->m_pIndex->SetCacheSize ( g_iMaxCachedDocs, g_iMaxCachedHits );
		if ( m_bMultiQueue )
		{
			bResult = pServed->m_pIndex->MultiQuery ( &m_dQueries[m_iStart], &tStats,
				dSorters.GetLength(), &dSorters[0], NULL );
		} else
		{
			CSphVector<CSphQueryResult*> dResults ( m_dResults.GetLength() );
			ARRAY_FOREACH ( i, m_dResults )
				dResults[i] = &m_dResults[i];

			bResult = pServed->m_pIndex->MultiQueryEx ( dSorters.GetLength(),
				&m_dQueries[m_iStart], &dResults[m_iStart], &dSorters[0], NULL );
		}

		// handle results
		if ( !bResult )
		{
			// failed
			for ( int iQuery=m_iStart; iQuery<=m_iEnd; iQuery++ )
				m_dFailuresSet[iQuery].Submit ( sLocal,
					m_dResults [ m_bMultiQueue ? m_iStart : iQuery ].m_sError.cstr() );
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
				// multi-queue only returned one result set meta, so we need to replicate it
				if ( m_bMultiQueue )
				{
					// these times will be overridden below, but let's be clean
					tRes.m_iQueryTime += tStats.m_iQueryTime / ( m_iEnd-m_iStart+1 );
					tRes.m_iCpuTime += tStats.m_iCpuTime / ( m_iEnd-m_iStart+1 );
					tRes.m_pMva = tStats.m_pMva;
					tRes.m_pStrings = tStats.m_pStrings;
					tRes.m_hWordStats = tStats.m_hWordStats;
					tRes.m_iMultiplier = m_iEnd-m_iStart+1;
				} else if ( tRes.m_iMultiplier==-1 )
				{
					m_dFailuresSet[iQuery].Submit ( sLocal, tRes.m_sError.cstr() );
					continue;
				}

				tRes.m_iSuccesses++;
				tRes.m_tSchema = pSorter->GetSchema();
				tRes.m_iTotalMatches += pSorter->GetTotalCount();

				// extract matches from sorter
				assert ( pSorter );

				if ( pSorter->GetLength() )
				{
					tRes.m_dMatchCounts.Add ( pSorter->GetLength() );
					tRes.m_dSchemas.Add ( tRes.m_tSchema );
					tRes.m_dIndexWeights.Add ( m_dQueries[iQuery].GetIndexWeight ( sLocal ) );
					PoolPtrs_t & tPoolPtrs = tRes.m_dTag2Pools.Add ();
					tPoolPtrs.m_pMva = tRes.m_pMva;
					tPoolPtrs.m_pStrings = tRes.m_pStrings;
					sphFlattenQueue ( pSorter, &tRes, tRes.m_iTag++ );

					// clean up for next index search
					tRes.m_pMva = NULL;
					tRes.m_pStrings = NULL;
				}

				// move external attributes storage from tStats to actual result
				tStats.LeakStorages ( tRes );
			}
		}
		if ( !m_bIsLocked )
			pServed->Unlock();

		// cleanup kill-list
		pQuery->m_dFilters.Resize ( iNumFilters );

		// clean up indexes locked by filters
		DoUnlockClear ( dLocked );

		// cleanup sorters
		if ( !pLocalSorter )
			ARRAY_FOREACH ( i, dSorters )
				SafeDelete ( dSorters[i] );
	}
}


// check expressions into a query to make sure that it's ready for multi query optimization
static bool HasExpresions ( const CSphQuery & tQuery )
{
	const ServedIndex_t * pServedIndex = g_pIndexes->GetRlockedEntry ( tQuery.m_sIndexes );

	// check that it exists
	if ( !pServedIndex )
		return false;

	bool bHasExpression = false;
	if ( pServedIndex->m_bEnabled )
		bHasExpression = sphHasExpressions ( tQuery, pServedIndex->m_pIndex->GetMatchSchema() );

	pServedIndex->Unlock();

	return bHasExpression;
}


void SearchHandler_c::RunSubset ( int iStart, int iEnd )
{
	m_iStart = iStart;
	m_iEnd = iEnd;
	m_dLocal.Reset ();

	// all my stats
	int64_t tmSubset = sphMicroTimer();
	int64_t tmLocal = 0;
	int64_t tmWait = 0;
	int64_t tmCpu = sphCpuTimer ();

	if ( g_bIOStats )
		sphStartIOStats ();

	// prepare for descent
	CSphQuery & tFirst = m_dQueries[iStart];

	for ( int iRes=iStart; iRes<=iEnd; iRes++ )
		m_dResults[iRes].m_iSuccesses = 0;

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
			( qCheck.m_iWeights!=qFirst.m_iWeights ) || // weights count
			( qCheck.m_pWeights && memcmp ( qCheck.m_pWeights, qFirst.m_pWeights, sizeof(int)*qCheck.m_iWeights ) ) || // weights; NOLINT
			( qCheck.m_eMode!=qFirst.m_eMode ) || // search mode
			( qCheck.m_eRanker!=qFirst.m_eRanker ) || // ranking mode
			( qCheck.m_dFilters.GetLength()!=qFirst.m_dFilters.GetLength() ) || // attr filters count
			( qCheck.m_iCutoff!=qFirst.m_iCutoff ) || // cutoff
			( qCheck.m_eSort==SPH_SORT_EXPR && qFirst.m_eSort==SPH_SORT_EXPR && qCheck.m_sSortBy!=qFirst.m_sSortBy ) || // sort expressions
			( qCheck.m_bGeoAnchor!=qFirst.m_bGeoAnchor ) || // geodist expression
			( qCheck.m_bGeoAnchor && qFirst.m_bGeoAnchor && ( qCheck.m_fGeoLatitude!=qFirst.m_fGeoLatitude || qCheck.m_fGeoLongitude!=qFirst.m_fGeoLongitude ) ) ) // some geodist cases
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

	// select lists must have no expressions
	for ( int iCheck=iStart; iCheck<=iEnd && m_bMultiQueue; iCheck++ )
	{
		m_bMultiQueue = !HasExpresions ( m_dQueries[iCheck] );
	}
	////////////////////////////
	// build local indexes list
	////////////////////////////

	CSphVector<AgentConn_t> dAgents;
	CSphVector<CSphString> dDistLocal;
	bool bDist = false;
	int iAgentConnectTimeout = 0, iAgentQueryTimeout = 0;

	{
		g_tDistLock.Lock();
		DistributedIndex_t * pDist = g_hDistIndexes ( tFirst.m_sIndexes );
		if ( pDist )
		{
			bDist = true;
			iAgentConnectTimeout = pDist->m_iAgentConnectTimeout;
			iAgentQueryTimeout = pDist->m_iAgentQueryTimeout;

			dDistLocal = pDist->m_dLocal;

			dAgents.Resize ( pDist->m_dAgents.GetLength() );
			ARRAY_FOREACH ( i, pDist->m_dAgents )
				dAgents[i] = pDist->m_dAgents[i];
		}
		g_tDistLock.Unlock();
	}

	if ( !bDist )
	{
		// they're all local, build the list
		if ( tFirst.m_sIndexes=="*" )
		{
			// search through all local indexes
			for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
				if ( it.Get ().m_bEnabled )
					m_dLocal.Add ( it.GetKey() );
		} else
		{
			// search through specified local indexes
			ParseIndexList ( tFirst.m_sIndexes, m_dLocal );

			// there should be no distributed indexes in multi-index query
			int iDistFound = -1;
			g_tDistLock.Lock();

			ARRAY_FOREACH ( i, m_dLocal )
				if ( g_hDistIndexes.Exists ( m_dLocal[i] ) )
				{
					iDistFound = i;
					break;
				}

			g_tDistLock.Unlock();

			if ( iDistFound!=-1 )
			{
				for ( int iRes=iStart; iRes<=iEnd; iRes++ )
					m_dResults[iRes].m_sError.SetSprintf ( "distributed index '%s' in multi-index query found", m_dLocal[iDistFound].cstr() );
				return;
			}

			ARRAY_FOREACH ( i, m_dLocal )
			{
				const ServedIndex_t * pServedIndex = g_pIndexes->GetRlockedEntry ( m_dLocal[i] );

				// check that it exists
				if ( !pServedIndex )
				{
					for ( int iRes=iStart; iRes<=iEnd; iRes++ )
						m_dResults[iRes].m_sError.SetSprintf ( "unknown local index '%s' in search request", m_dLocal[i].cstr() );
					return;
				}

				// if it exists but is not enabled, remove it from the list and force recheck
				if ( !pServedIndex->m_bEnabled )
					m_dLocal.RemoveFast ( i-- );

				pServedIndex->Unlock();
			}
		}

		// sanity check
		if ( !m_dLocal.GetLength() )
		{
			for ( int iRes=iStart; iRes<=iEnd; iRes++ )
				m_dResults[iRes].m_sError.SetSprintf ( "no enabled local indexes to search" );
			return;
		}

	} else
	{
		// copy local indexes list from distributed definition, but filter out disabled ones
		ARRAY_FOREACH ( i, dDistLocal )
		{
			const ServedIndex_t * pServedIndex = g_pIndexes->GetRlockedEntry ( dDistLocal[i] );
			if ( pServedIndex )
			{
				if ( pServedIndex->m_bEnabled )
					m_dLocal.Add ( dDistLocal[i] );

				pServedIndex->Unlock();
			}
		}
	}

	CSphVector < const ServedIndex_t * > dLockedEqualIndexes;

	/////////////////////////////////////////////////////
	// optimize single-query, same-schema local searches
	/////////////////////////////////////////////////////

	ISphMatchSorter * pLocalSorter = NULL;
	while ( iStart==iEnd && m_dLocal.GetLength()>1 )
	{
		CSphString sError;

		// check if all schemas are equal
		bool bAllEqual = true;

		const ServedIndex_t * pFirstIndex = g_pIndexes->GetRlockedEntry ( m_dLocal[0] );
		if ( !pFirstIndex )
			break;

		const CSphSchema & tFirstSchema = pFirstIndex->m_pIndex->GetMatchSchema();
		for ( int i=1; i<m_dLocal.GetLength() && bAllEqual; i++ )
		{
			const ServedIndex_t * pNextIndex = g_pIndexes->GetRlockedEntry ( m_dLocal[i] );
			if ( !pNextIndex )
			{
				bAllEqual = false;
				break;
			}

			if ( !tFirstSchema.CompareTo ( pNextIndex->m_pIndex->GetMatchSchema(), sError ) )
				bAllEqual = false;

			if ( bAllEqual && g_eWorkers==MPM_THREADS )
				dLockedEqualIndexes.Add ( pNextIndex );
			else
				pNextIndex->Unlock();
		}

		// we can reuse the very same sorter
		if ( bAllEqual )
			if ( FixupQuery ( &m_dQueries[iStart], &tFirstSchema, "local-sorter", sError ) )
				pLocalSorter = sphCreateQueue ( &m_dQueries[iStart], tFirstSchema, sError, true,
					m_dQueries[iStart].m_bAgent?&m_dExtraSchemas[iStart]:NULL);

		if ( g_eWorkers==MPM_THREADS )
		{
			if ( pLocalSorter )
				dLockedEqualIndexes.Add ( pFirstIndex );
			else
			{
				pFirstIndex->Unlock ();
				DoUnlockClear ( dLockedEqualIndexes );
			}
		}
		break;
	}

	// these are mutual exclusive
	assert ( !( m_bMultiQueue && pLocalSorter ) );

	///////////////////////////////////////////////////////////
	// main query loop (with multiple retries for distributed)
	///////////////////////////////////////////////////////////

	tFirst.m_iRetryCount = Min ( Max ( tFirst.m_iRetryCount, 0 ), MAX_RETRY_COUNT ); // paranoid clamp
	if ( !bDist )
		tFirst.m_iRetryCount = 0;

	for ( int iRetry=0; iRetry<=tFirst.m_iRetryCount; iRetry++ )
	{
		////////////////////////
		// issue remote queries
		////////////////////////

		// delay between retries
		if ( iRetry>0 )
			sphSleepMsec ( tFirst.m_iRetryDelay );

		// connect to remote agents and query them, if required
		int iRemote = 0;
		if ( bDist )
		{
			ConnectToRemoteAgents ( dAgents, iRetry!=0 );

			SearchRequestBuilder_t tReqBuilder ( m_dQueries, iStart, iEnd );
			iRemote = QueryRemoteAgents ( dAgents, iAgentConnectTimeout, tReqBuilder, &tmWait );
		}

		/////////////////////
		// run local queries
		//////////////////////

		// while the remote queries are running, do local searches
		// FIXME! what if the remote agents finish early, could they timeout?
		if ( iRetry==0 )
		{
			if ( bDist && !iRemote && !m_dLocal.GetLength() )
			{
				for ( int iRes=iStart; iRes<=iEnd; iRes++ )
					m_dResults[iRes].m_sError = "all remote agents unreachable and no available local indexes found";

				DoUnlockClear ( dLockedEqualIndexes );

				SafeDelete ( pLocalSorter );
				return;
			}

			tmLocal = -sphMicroTimer();
			RunLocalSearches ( pLocalSorter, bDist ? tFirst.m_sIndexes.cstr() : NULL );
			tmLocal += sphMicroTimer();
		}

		///////////////////////
		// poll remote queries
		///////////////////////

		// wait for remote queries to complete
		if ( iRemote )
		{
			SearchReplyParser_t tParser ( iStart, iEnd, m_dMvaStorage, m_dStringsStorage );
			int iMsecLeft = iAgentQueryTimeout - (int)( tmLocal/1000 );
			int iReplys = WaitForRemoteAgents ( dAgents, Max ( iMsecLeft, 0 ), tParser, &tmWait );

			// check if there were valid (though might be 0-matches) replys, and merge them
			if ( iReplys )
				ARRAY_FOREACH ( iAgent, dAgents )
			{
				AgentConn_t & tAgent = dAgents[iAgent];
				if ( !tAgent.m_bSuccess )
					continue;

				// merge this agent's results
				for ( int iRes=iStart; iRes<=iEnd; iRes++ )
				{
					const CSphQueryResult & tRemoteResult = tAgent.m_dResults[iRes-iStart];

					// copy errors or warnings
					if ( !tRemoteResult.m_sError.IsEmpty() )
						m_dFailuresSet[iRes].SubmitEx ( tFirst.m_sIndexes.cstr(), "agent %s: remote query error: %s", tAgent.GetName().cstr(), tRemoteResult.m_sError.cstr() );
					if ( !tRemoteResult.m_sWarning.IsEmpty() )
						m_dFailuresSet[iRes].SubmitEx ( tFirst.m_sIndexes.cstr(), "agent %s: remote query warning: %s", tAgent.GetName().cstr(), tRemoteResult.m_sWarning.cstr() );

					if ( tRemoteResult.m_iSuccesses<=0 )
						continue;

					AggrResult_t & tRes = m_dResults[iRes];
					tRes.m_iSuccesses++;

					ARRAY_FOREACH ( i, tRemoteResult.m_dMatches )
					{
						tRes.m_dMatches.Add();
						tRes.m_dMatches.Last().Clone ( tRemoteResult.m_dMatches[i], tRemoteResult.m_tSchema.GetRowSize() );
						tRes.m_dMatches.Last().m_iTag = 0; // all remote MVA values go to special pool which is at index 0
					}

					tRes.m_dMatchCounts.Add ( tRemoteResult.m_dMatches.GetLength() );
					tRes.m_dSchemas.Add ( tRemoteResult.m_tSchema );
					// note how we do NOT add per-index weight here; remote agents are all tagged 0 (which contains weight 1)

					// merge this agent's stats
					tRes.m_iTotalMatches += tRemoteResult.m_iTotalMatches;
					tRes.m_iQueryTime += tRemoteResult.m_iQueryTime;

					// merge this agent's words
					MergeWordStats ( tRes, tRemoteResult.m_hWordStats );
				}

				// dismissed
				tAgent.m_dResults.Reset ();
				tAgent.m_bSuccess = false;
				tAgent.m_sFailure = "";
			}
		}

		// check if we need to retry again
		int iToRetry = 0;
		if ( bDist )
			ARRAY_FOREACH ( i, dAgents )
				if ( dAgents[i].m_eState==AGENT_RETRY )
					iToRetry++;
		if ( !iToRetry )
			break;
	}

	// submit failures from failed agents
	if ( bDist )
	{
		ARRAY_FOREACH ( i, dAgents )
		{
			const AgentConn_t & tAgent = dAgents[i];
			if ( !tAgent.m_bSuccess && !tAgent.m_sFailure.IsEmpty() )
				for ( int j=iStart; j<=iEnd; j++ )
					m_dFailuresSet[j].SubmitEx ( tFirst.m_sIndexes.cstr(), tAgent.m_bBlackhole ? "blackhole %s: %s" : "agent %s: %s",
						tAgent.GetName().cstr(), tAgent.m_sFailure.cstr() );
		}
	}

	ARRAY_FOREACH ( i, m_dResults )
		assert ( m_dResults[i].m_iTag==m_dResults[i].m_dTag2Pools.GetLength() );

	// cleanup
	bool bWasLocalSorter = pLocalSorter!=NULL;
	SafeDelete ( pLocalSorter );
	DoUnlockClear ( dLockedEqualIndexes );

	/////////////////////
	// merge all results
	/////////////////////

	for ( int iRes=iStart; iRes<=iEnd; iRes++ )
	{
		AggrResult_t & tRes = m_dResults[iRes];
		CSphQuery & tQuery = m_dQueries[iRes];
		CSphSchema * pExtraSchema = tQuery.m_bAgent?&m_dExtraSchemas[bWasLocalSorter?0:iRes]:NULL;

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
		if ( tRes.m_iSuccesses>1 || tQuery.m_dItems.GetLength() )
		{
			if ( g_bCompatResults )
			{
				if ( !MinimizeAggrResultCompat ( tRes, tQuery, m_dLocal.GetLength()!=0 ) )
					return;
			} else
			{
				if ( !MinimizeAggrResult ( tRes, tQuery, m_dLocal.GetLength()!=0, pExtraSchema ) )
					return;
			}
		}

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
	}

	// stats
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
			m_dResults[iRes].m_iCpuTime = tmCpu/iQueries;
		}
	} else
	{
		int64_t tmAccountedWall = 0;
		int64_t tmAccountedCpu = 0;
		for ( int iRes=iStart; iRes<=iEnd; iRes++ )
		{
			tmAccountedWall += m_dResults[iRes].m_iQueryTime*1000;
			tmAccountedCpu += m_dResults[iRes].m_iCpuTime;
		}

		int64_t tmDeltaWall = ( tmSubset - tmAccountedWall ) / iQueries;
		int64_t tmDeltaCpu = ( tmCpu - tmAccountedCpu ) / iQueries;

		for ( int iRes=iStart; iRes<=iEnd; iRes++ )
		{
			m_dResults[iRes].m_iQueryTime += (int)(tmDeltaWall/1000);
			m_dResults[iRes].m_iCpuTime += tmDeltaCpu;
		}
	}

	const CSphIOStats & tIO = sphStopIOStats ();

	if ( g_pStats )
	{
		g_tStatsMutex.Lock();
		g_pStats->m_iQueries += iQueries;
		g_pStats->m_iQueryTime += tmSubset;
		g_pStats->m_iQueryCpuTime += tmCpu;
		if ( bDist && dAgents.GetLength() )
		{
			// do *not* count queries to dist indexes w/o actual remote agents
			g_pStats->m_iDistQueries++;
			g_pStats->m_iDistWallTime += tmSubset;
			g_pStats->m_iDistLocalTime += tmLocal;
			g_pStats->m_iDistWaitTime += tmWait;
		}
		g_pStats->m_iDiskReads += tIO.m_iReadOps;
		g_pStats->m_iDiskReadTime += tIO.m_iReadTime;
		g_pStats->m_iDiskReadBytes += tIO.m_iReadBytes;
		g_tStatsMutex.Unlock();
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


void SendSearchResponse ( SearchHandler_c & tHandler, InputBuffer_c & tReq, int iSock, int iVer )
{
	// serve the response
	NetOutputBuffer_c tOut ( iSock );
	int iReplyLen = 0;

	CSphVector<CSphString> dUntouched;

	ARRAY_FOREACH ( i, tHandler.m_dResults )
	{
		dUntouched.Resize ( 0 );

		AggrResult_t & tRes = tHandler.m_dResults[i];
		tRes.m_hWordStats.IterateStart();
		while ( tRes.m_hWordStats.IterateNext() )
			if ( tRes.m_hWordStats.IterateGet().m_bUntouched )
				dUntouched.Add ( tRes.m_hWordStats.IterateGetKey() );

		ARRAY_FOREACH ( i, dUntouched )
			tRes.m_hWordStats.Delete ( dUntouched[i] );
	}

	if ( iVer<=0x10C )
	{
		assert ( tHandler.m_dQueries.GetLength()==1 );
		assert ( tHandler.m_dResults.GetLength()==1 );
		const AggrResult_t & tRes = tHandler.m_dResults[0];

		if ( !tRes.m_sError.IsEmpty() )
		{
			tReq.SendErrorReply ( "%s", tRes.m_sError.cstr() );
			return;
		}

		iReplyLen = CalcResultLength ( iVer, &tRes, tRes.m_dTag2Pools );
		bool bWarning = ( iVer>=0x106 && !tRes.m_sWarning.IsEmpty() );

		// send it
		tOut.SendWord ( (WORD)( bWarning ? SEARCHD_WARNING : SEARCHD_OK ) );
		tOut.SendWord ( VER_COMMAND_SEARCH );
		tOut.SendInt ( iReplyLen );

		SendResult ( iVer, tOut, &tRes, tRes.m_dTag2Pools );

	} else
	{
		ARRAY_FOREACH ( i, tHandler.m_dQueries )
			iReplyLen += CalcResultLength ( iVer, &tHandler.m_dResults[i], tHandler.m_dResults[i].m_dTag2Pools );

		// send it
		tOut.SendWord ( (WORD)SEARCHD_OK );
		tOut.SendWord ( VER_COMMAND_SEARCH );
		tOut.SendInt ( iReplyLen );

		ARRAY_FOREACH ( i, tHandler.m_dQueries )
			SendResult ( iVer, tOut, &tHandler.m_dResults[i], tHandler.m_dResults[i].m_dTag2Pools );
	}

	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==iReplyLen+8 );

	// clean up
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
		SafeDeleteArray ( tHandler.m_dQueries[i].m_pWeights );
}


void HandleCommandSearch ( int iSock, int iVer, InputBuffer_c & tReq )
{
	MEMORY ( SPH_MEM_SEARCH_NONSQL );

	if ( !CheckCommandVersion ( iVer, VER_COMMAND_SEARCH, tReq ) )
		return;

	// parse request
	int iQueries = 1;
	if ( iVer>=0x10D )
		iQueries = tReq.GetDword ();

	if ( g_iMaxBatchQueries>0 && ( iQueries<=0 || iQueries>g_iMaxBatchQueries ) )
	{
		tReq.SendErrorReply ( "bad multi-query count %d (must be in 1..%d range)", iQueries, g_iMaxBatchQueries );
		return;
	}

	SearchHandler_c tHandler ( iQueries );
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
		if ( !ParseSearchQuery ( tReq, tHandler.m_dQueries[i], iVer ) )
			return;

	// run queries, send response
	tHandler.RunQueries ();
	SendSearchResponse ( tHandler, tReq, iSock, iVer );
}

//////////////////////////////////////////////////////////////////////////
// SQL PARSER
//////////////////////////////////////////////////////////////////////////

enum SqlStmt_e
{
	STMT_PARSE_ERROR = 0,
	STMT_SELECT,
	STMT_INSERT,
	STMT_REPLACE,
	STMT_DELETE,
	STMT_SHOW_WARNINGS,
	STMT_SHOW_STATUS,
	STMT_SHOW_META,
	STMT_SET,
	STMT_BEGIN,
	STMT_COMMIT,
	STMT_ROLLBACK,
	STMT_CALL,
	STMT_DESC,
	STMT_SHOW_TABLES,
	STMT_UPDATE,

	STMT_TOTAL
};


const char * g_dSqlStmts[STMT_TOTAL] =
{
	"parse_error", "select", "insert", "replace", "delete", "show_warnings",
	"show_status", "show_meta", "set", "begin", "commit", "rollback", "call",
	"desc", "show_tables", "update"
};


/// insert value
struct SqlInsert_t
{
	int						m_iType;
	CSphString				m_sVal;		// OPTIMIZE? use char* and point to node?
	int64_t					m_iVal;
	float					m_fVal;
};


/// parser view on a generic node
struct SqlNode_t
{
	int						m_iStart;
	int						m_iEnd;
	CSphString				m_sValue;
	int64_t					m_iValue;
	float					m_fValue;
	CSphVector<SphAttr_t>	m_dValues;
	int						m_iInstype;	// REMOVE? should not we know this somehow else?

	SqlNode_t() : m_iValue ( 0 ) {}
};
#define YYSTYPE SqlNode_t


/// parsing result
/// one day, we will start subclassing this
struct SqlStmt_t
{
	SqlStmt_e				m_eStmt;
	int						m_iRowsAffected;

	// SELECT specific
	CSphQuery				m_tQuery;

	// used by INSERT, DELETE, CALL, DESC
	CSphString				m_sIndex;

	// INSERT (and CALL) specific
	CSphVector<SqlInsert_t>	m_dInsertValues; // reused by CALL
	CSphVector<CSphString>	m_dInsertSchema;
	int						m_iSchemaSz;

	// DELETE specific
	SphDocID_t				m_iDeleteID;

	// SET specific
	CSphString				m_sSetName;
	int						m_iSetValue;

	// CALL specific
	CSphString				m_sCallProc;
	CSphVector<CSphString>	m_dCallOptNames;
	CSphVector<SqlInsert_t>	m_dCallOptValues;

	// UPDATE specific
	CSphAttrUpdate			m_tUpdate;

	SqlStmt_t ()
		: m_eStmt ( STMT_PARSE_ERROR )
		, m_iRowsAffected ( 0 )
		, m_iSchemaSz ( 0 )
	{
		m_tQuery.m_eMode = SPH_MATCH_EXTENDED2; // only new and shiny matching and sorting
		m_tQuery.m_eSort = SPH_SORT_EXTENDED;
		m_tQuery.m_sSortBy = "@weight desc"; // default order
		m_tQuery.m_sOrderBy = "@weight desc";
	}

	bool AddSchemaItem ( const char * psName )
	{
		m_dInsertSchema.Add ( psName );
		m_dInsertSchema.Last().ToLower();
		m_iSchemaSz = m_dInsertSchema.GetLength();
		return true; // stub; check if the given field actually exists in the schema
	}

	// check if the number of fields which would be inserted is in accordance to the given schema
	bool CheckInsertIntegrity()
	{
		// cheat: if no schema assigned, assume the size of schema as the size of the first row.
		// (if it is wrong, it will be revealed later)
		if ( !m_iSchemaSz )
			m_iSchemaSz = m_dInsertValues.GetLength();

		m_iRowsAffected++;
		return m_dInsertValues.GetLength()==m_iRowsAffected*m_iSchemaSz;
	}
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

public:
	explicit		SqlParser_c ( CSphVector<SqlStmt_t> & dStmt );

	void			PushQuery ();

	bool			AddOption ( SqlNode_t tIdent, SqlNode_t tValue );
	bool			AddOption ( SqlNode_t tIdent, CSphVector<CSphNamedInt> & dNamed );
	void			AddItem ( YYSTYPE * pExpr, YYSTYPE * pAlias, ESphAggrFunc eFunc=SPH_AGGR_NONE );
	void			SetSelect ( int iStart, int iEnd )
	{
		if ( m_pQuery )
		{
			if ( m_pQuery->m_iSQLSelectStart<0 || m_pQuery->m_iSQLSelectStart>iStart )
				m_pQuery->m_iSQLSelectStart = iStart;
			m_pQuery->m_iSQLSelectEnd = iEnd;
		}
	}
	bool			AddSchemaItem ( YYSTYPE * pNode );
	void			SetValue ( const char * sName, YYSTYPE tValue );

	int							AllocNamedVec ();
	CSphVector<CSphNamedInt> &	GetNamedVec ( int iIndex );
	void						FreeNamedVec ( int iIndex );

protected:
	bool						m_bNamedVecBusy;
	CSphVector<CSphNamedInt>	m_dNamedVec;
};


void SqlUnescape ( CSphString & sRes, const char * sEscaped, int iLen )
{
	assert ( iLen>=2 );
	assert ( sEscaped[0]=='\'' );
	assert ( sEscaped[iLen-1]=='\'' );

	// skip heading and trailing quotes
	const char * s = sEscaped+1;
	const char * sMax = s+iLen-2;

	sRes.Reserve ( iLen );
	char * d = (char*) sRes.cstr();

	while ( s<sMax )
	{
		if ( s[0]=='\\' )
		{
			switch ( s[1] )
			{
				case 'b': *d++ = '\b'; break;
				case 'n': *d++ = '\n'; break;
				case 'r': *d++ = '\r'; break;
				case 't': *d++ = '\t'; break;
				default:
					*d++ = s[1];
			}
			s += 2;
		} else
			*d++ = *s++;
	}

	*d++ = '\0';
}

//////////////////////////////////////////////////////////////////////////

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YYLEX_PARAM pParser->m_pScanner, pParser
#ifdef NDEBUG
#define YY_DECL int yylex ( YYSTYPE * lvalp, void * yyscanner, SqlParser_c * pParser )
#else
#define YY_DECL int yylexd ( YYSTYPE * lvalp, void * yyscanner, SqlParser_c * pParser )
#endif
#include "llsphinxql.c"


void yyerror ( SqlParser_c * pParser, const char * sMessage )
{
	// flex put a zero at last token boundary; make it undo that
	yylex_unhold ( pParser->m_pScanner );

	// create our error message
	pParser->m_pParseError->SetSprintf ( "%s near '%s'", sMessage, pParser->m_pLastTokenStart ? pParser->m_pLastTokenStart : "(null)" );

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
int yylex ( YYSTYPE * lvalp, void * yyscanner, SqlParser_c * pParser )
{
	int res = yylexd ( lvalp, yyscanner, pParser );
	return res;
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
#if USE_64BIT
#define ToDocid ToBigInt
#else
#define ToDocid ToInt
#endif // USE_64BIT

	bool SetAttr ( const CSphAttrLocator & tLoc, const SqlInsert_t & tVal, int iTargetType )
	{
		switch ( iTargetType )
		{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TIMESTAMP:
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
			CSphMatch::SetAttr ( tLoc, 0 );
			break;
		default:
			return false;
		};
		return true;
	}
	inline bool SetDefaultAttr ( const CSphAttrLocator & tLoc, int iTargetType )
	{
		SqlInsert_t tVal;
		tVal.m_iType = TOK_CONST_INT;
		tVal.m_iVal = 0;
		return SetAttr ( tLoc, tVal, iTargetType );
	}
};

SqlParser_c::SqlParser_c ( CSphVector<SqlStmt_t> & dStmt )
	: m_pQuery ( NULL )
	, m_pStmt ( NULL )
	, m_dStmt ( dStmt )
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

	m_bGotQuery = false;
}

bool SqlParser_c::AddOption ( SqlNode_t tIdent, SqlNode_t tValue )
{
	CSphString & sOpt = tIdent.m_sValue;
	CSphString & sVal = tValue.m_sValue;
	sOpt.ToLower ();
	sVal.ToLower ();

	if ( sOpt=="ranker" )
	{
		if ( sVal=="proximity_bm25" )	m_pQuery->m_eRanker = SPH_RANK_PROXIMITY_BM25;
		else if ( sVal=="bm25" )		m_pQuery->m_eRanker = SPH_RANK_BM25;
		else if ( sVal=="none" )		m_pQuery->m_eRanker = SPH_RANK_NONE;
		else if ( sVal=="wordcount" )	m_pQuery->m_eRanker = SPH_RANK_WORDCOUNT;
		else if ( sVal=="proximity" )	m_pQuery->m_eRanker = SPH_RANK_PROXIMITY;
		else if ( sVal=="matchany" )	m_pQuery->m_eRanker = SPH_RANK_MATCHANY;
		else if ( sVal=="fieldmask" )	m_pQuery->m_eRanker = SPH_RANK_FIELDMASK;
		else if ( sVal=="sph04" )		m_pQuery->m_eRanker = SPH_RANK_SPH04;
		else
		{
			m_pParseError->SetSprintf ( "unknown ranker '%s'", sVal.cstr() );
			return false;
		}

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

	} else
	{
		m_pParseError->SetSprintf ( "unknown option '%s' (or bad argument type)", tIdent.m_sValue.cstr() );
		return false;
	}

	return true;
}

bool SqlParser_c::AddOption ( SqlNode_t tIdent, CSphVector<CSphNamedInt> & dNamed )
{
	CSphString & sOpt = tIdent.m_sValue;
	sOpt.ToLower ();

	if ( sOpt=="field_weights" )
	{
		m_pQuery->m_dFieldWeights.SwapData ( dNamed );

	} else if ( sOpt=="index_weights" )
	{
		m_pQuery->m_dIndexWeights.SwapData ( dNamed );

	} else
	{
		m_pParseError->SetSprintf ( "unknown option '%s' (or bad argument type)", tIdent.m_sValue.cstr() );
		return false;
	}

	return true;
}

void SqlParser_c::AddItem ( YYSTYPE * pExpr, YYSTYPE * pAlias, ESphAggrFunc eFunc )
{
	CSphQueryItem tItem;
	tItem.m_sExpr.SetBinary ( m_pBuf + pExpr->m_iStart, pExpr->m_iEnd - pExpr->m_iStart );
	if ( pAlias )
		tItem.m_sAlias.SetBinary ( m_pBuf + pAlias->m_iStart, pAlias->m_iEnd - pAlias->m_iStart );
	tItem.m_eAggrFunc = eFunc;

	m_pQuery->m_dItems.Add ( tItem );
}

bool SqlParser_c::AddSchemaItem ( YYSTYPE * pNode )
{
	assert ( m_pStmt );
	CSphString sItem;
	sItem.SetBinary ( m_pBuf + pNode->m_iStart, pNode->m_iEnd - pNode->m_iStart );
	return m_pStmt->AddSchemaItem ( sItem.cstr() );
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

bool ParseSqlQuery ( const CSphString & sQuery, CSphVector<SqlStmt_t> & dStmt, CSphString & sError )
{
	SqlParser_c tParser ( dStmt );
	tParser.m_pBuf = sQuery.cstr();
	tParser.m_pLastTokenStart = NULL;
	tParser.m_pParseError = &sError;

	int iLen = strlen ( sQuery.cstr() );
	char * sEnd = (char*)sQuery.cstr() + iLen;
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	yylex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy_scan_buffer ( (char*)sQuery.cstr(), iLen+2, tParser.m_pScanner );
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
		CSphQuery & tQuery = dStmt[i].m_tQuery;
		if ( tQuery.m_iSQLSelectStart>=0 )
		{
			tQuery.m_sSelect.SetBinary ( tParser.m_pBuf + tQuery.m_iSQLSelectStart,
				tQuery.m_iSQLSelectEnd - tQuery.m_iSQLSelectStart );

			// finally check for agent magic
			if ( tQuery.m_sSelect.Begins ( "*,*" ) ) // this is the mark of agent.
			{
				tQuery.m_dItems.Remove(0);
				tQuery.m_dItems.Remove(0);
				tQuery.m_bAgent = true;
			}
		}
	}

	if ( iRes!=0 || !dStmt.GetLength() )
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////

void HandleCommandQuery ( int iSock, int iVer, InputBuffer_c & tReq )
{
	MEMORY ( SPH_MEM_QUERY_NONSQL );

	if ( !CheckCommandVersion ( iVer, VER_COMMAND_QUERY, tReq ) )
		return;

	// parse request packet
	DWORD uResponseVer = tReq.GetDword(); // how shall we pack the response packet?
	CSphString sQuery = tReq.GetString(); // what is our query?

	// check for errors
	if ( uResponseVer<0x100 || uResponseVer>VER_COMMAND_SEARCH )
	{
		tReq.SendErrorReply ( "unsupported search response format v.%d.%d requested", uResponseVer>>8, uResponseVer & 0xff );
		return;
	}
	if ( tReq.GetError() )
	{
		tReq.SendErrorReply ( "invalid or truncated request" );
		return;
	}

	// parse SQL query
	CSphString sError;
	CSphVector<SqlStmt_t> dStmt;

	if ( !ParseSqlQuery ( sQuery, dStmt, sError ) )
	{
		tReq.SendErrorReply ( "%s", sError.cstr() );
		return;
	}
	if ( dStmt.GetLength()!=1 )
	{
		tReq.SendErrorReply ( "this protocol currently supports single SELECT (got=%d)", dStmt.GetLength() );
		return;
	}
	if ( dStmt[0].m_eStmt!=STMT_SELECT )
	{
		tReq.SendErrorReply ( "this protocol currently supports SELECT only" );
		return;
	}

	SearchHandler_c tHandler ( 1 );
	tHandler.m_dQueries[0] = dStmt.Begin()->m_tQuery;

	// do the dew
	tHandler.RunQueries ();
	SendSearchResponse ( tHandler, tReq, iSock, uResponseVer );
}

/////////////////////////////////////////////////////////////////////////////

int sphGetPassageBoundary ( const CSphString & sPassageBoundaryMode )
{
	if ( sPassageBoundaryMode.IsEmpty() )
		return 0;

	int iMode = 0;
	if ( sPassageBoundaryMode=="sentence" )
		iMode = MAGIC_CODE_SENTENCE;
	else if ( sPassageBoundaryMode=="paragraph" )
		iMode = MAGIC_CODE_PARAGRAPH;
	else if ( sPassageBoundaryMode=="zone" )
		iMode = MAGIC_CODE_ZONE;

	return iMode;
}

bool sphCheckOptionsSPZ ( const ExcerptQuery_t & q, const CSphString & sPassageBoundaryMode, CSphString & sError )
{
	if ( q.m_iPassageBoundary )
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
		if ( q.m_iPassageBoundary!=MAGIC_CODE_ZONE )
		{
			sError.SetSprintf ( "invalid combination of passage_boundary=%s and emit_zones", sPassageBoundaryMode.cstr() );
			return false;
		}
		if ( !( q.m_sStripMode=="strip" || q.m_sStripMode=="index" ) )
		{
			sError.SetSprintf ( "invalid combination of strip=%s and emit_zones", q.m_sStripMode.cstr(), sPassageBoundaryMode.cstr() );
			return false;
		}
	}

	return true;
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
	const int EXCERPT_FLAG_EXACTPHRASE		= 2;
	const int EXCERPT_FLAG_SINGLEPASSAGE	= 4;
	const int EXCERPT_FLAG_USEBOUNDARIES	= 8;
	const int EXCERPT_FLAG_WEIGHTORDER		= 16;
	const int EXCERPT_FLAG_QUERY			= 32;
	const int EXCERPT_FLAG_FORCE_ALL_WORDS	= 64;
	const int EXCERPT_FLAG_LOAD_FILES		= 128;
	const int EXCERPT_FLAG_ALLOW_EMPTY		= 256;
	const int EXCERPT_FLAG_EMIT_ZONES		= 512;

	// v.1.1
	ExcerptQuery_t q;

	tReq.GetInt (); // mode field is for now reserved and ignored
	int iFlags = tReq.GetInt ();
	CSphString sIndex = tReq.GetString ();

	const ServedIndex_t * pServed = g_pIndexes->GetRlockedEntry ( sIndex );
	if ( !pServed )
	{
		tReq.SendErrorReply ( "unknown local index '%s' in search request", sIndex.cstr() );
		return;
	}

	CSphIndex * pIndex = pServed->m_pIndex;
	CSphDict * pDict = pIndex->GetDictionary ();
	CSphScopedPtr<ISphTokenizer> pTokenizer ( pIndex->GetTokenizer()->Clone ( true ) );

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
			tReq.SendErrorReply ( "unknown html_strip_mode=%s", q.m_sStripMode.cstr() );
			pServed->Unlock();
			return;
		}
	}

	CSphString sPassageBoundaryMode;
	if ( iVer>=0x103 )
		sPassageBoundaryMode = tReq.GetString();

	q.m_bRemoveSpaces = ( iFlags & EXCERPT_FLAG_REMOVESPACES )!=0;
	q.m_bExactPhrase = ( iFlags & EXCERPT_FLAG_EXACTPHRASE )!=0;
	q.m_bUseBoundaries = ( iFlags & EXCERPT_FLAG_USEBOUNDARIES )!=0;
	q.m_bWeightOrder = ( iFlags & EXCERPT_FLAG_WEIGHTORDER )!=0;
	q.m_bHighlightQuery = ( iFlags & EXCERPT_FLAG_QUERY )!=0;
	q.m_bForceAllWords = ( iFlags & EXCERPT_FLAG_FORCE_ALL_WORDS )!=0;
	if ( iFlags & EXCERPT_FLAG_SINGLEPASSAGE )
		q.m_iLimitPassages = 1;
	q.m_bLoadFiles = ( iFlags & EXCERPT_FLAG_LOAD_FILES )!=0;
	q.m_bAllowEmpty = ( iFlags & EXCERPT_FLAG_ALLOW_EMPTY )!=0;
	q.m_bEmitZones = ( iFlags & EXCERPT_FLAG_EMIT_ZONES )!=0;

	int iCount = tReq.GetInt ();
	if ( iCount<0 || iCount>EXCERPT_MAX_ENTRIES )
	{
		tReq.SendErrorReply ( "invalid entries count %d", iCount );
		pServed->Unlock();
		return;
	}

	q.m_iPassageBoundary = sphGetPassageBoundary ( sPassageBoundaryMode );

	CSphString sError;

	if ( !sphCheckOptionsSPZ ( q, sPassageBoundaryMode, sError ) )
	{
		tReq.SendErrorReply ( "%s", sError.cstr() );
		pServed->Unlock();
		return;
	}

	if ( q.m_iPassageBoundary &&
		( !pTokenizer->EnableSentenceIndexing ( sError ) || !pTokenizer->EnableZoneIndexing ( sError ) ) )
	{
		tReq.SendErrorReply ( "%s", sError.cstr() );
		pServed->Unlock();
		return;
	}

	CSphVector<char*> dExcerpts;
	for ( int i=0; i<iCount; i++ )
	{
		q.m_sSource = tReq.GetString ();
		if ( tReq.GetError() )
		{
			tReq.SendErrorReply ( "invalid or truncated request" );
			pServed->Unlock();
			return;
		}

		CSphHTMLStripper tStripper;
		const CSphHTMLStripper * pStripper = NULL;
		const CSphIndexSettings & tSettings = pIndex->GetSettings ();
		if ( q.m_sStripMode=="strip"
			|| ( q.m_sStripMode=="index" && tSettings.m_bHtmlStrip ) )
		{
			if ( q.m_sStripMode=="index" )
			{
				if (
					!tStripper.SetIndexedAttrs ( tSettings.m_sHtmlIndexAttrs.cstr (), sError ) ||
					!tStripper.SetRemovedElements ( tSettings.m_sHtmlRemoveElements.cstr (), sError ) )
				{
					tReq.SendErrorReply ( "HTML stripper config error: %s", sError.cstr() );
					pServed->Unlock();
					return;
				}
			}
			pStripper = &tStripper;

			if ( !tSettings.m_sZonePrefix.IsEmpty() )
				tStripper.SetZonePrefix ( tSettings.m_sZonePrefix.cstr() );
		}

		char * sResult = sphBuildExcerpt ( q, pDict, pTokenizer.Ptr(), &pIndex->GetMatchSchema(), pIndex, sError, pStripper );
		if ( !sResult )
		{
			tReq.SendErrorReply ( "highlighting failed: %s", sError.cstr() );
			pServed->Unlock();
			return;
		}
		dExcerpts.Add ( sResult );
	}

	pServed->Unlock();

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
// KEYWORDS HANDLER
/////////////////////////////////////////////////////////////////////////////

void HandleCommandKeywords ( int iSock, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_KEYWORDS, tReq ) )
		return;

	CSphString sQuery = tReq.GetString ();
	CSphString sIndex = tReq.GetString ();
	bool bGetStats = !!tReq.GetInt ();

	const ServedIndex_t * pIndex = g_pIndexes->GetRlockedEntry ( sIndex );
	if ( !pIndex )
	{
		tReq.SendErrorReply ( "unknown local index '%s' in search request", sIndex.cstr() );
		return;
	}

	CSphString sError;
	CSphVector < CSphKeywordInfo > dKeywords;
	if ( !pIndex->m_pIndex->GetKeywords ( dKeywords, sQuery.cstr (), bGetStats, sError ) )
	{
		tReq.SendErrorReply ( "error generating keywords: %s", sError.cstr () );
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

	NetOutputBuffer_c tOut ( iSock );
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
	virtual void BuildRequest ( const char * sIndexes, NetOutputBuffer_c & tOut ) const;

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


void UpdateRequestBuilder_t::BuildRequest ( const char * sIndexes, NetOutputBuffer_c & tOut ) const
{
	int iReqSize = 4+strlen(sIndexes); // indexes string
	iReqSize += 4; // attrs array len, data
	ARRAY_FOREACH ( i, m_tUpd.m_dAttrs )
		iReqSize += 8 + strlen ( m_tUpd.m_dAttrs[i].m_sName.cstr() );
	iReqSize += 4; // number of updates
	iReqSize += 8*m_tUpd.m_dDocids.GetLength() + 4*m_tUpd.m_dPool.GetLength(); // 64bit ids, 32bit values

	// header
	tOut.SendDword ( SPHINX_SEARCHD_PROTO );
	tOut.SendWord ( SEARCHD_COMMAND_UPDATE );
	tOut.SendWord ( VER_COMMAND_UPDATE );
	tOut.SendInt ( iReqSize );

	tOut.SendString ( sIndexes );
	tOut.SendInt ( m_tUpd.m_dAttrs.GetLength() );
	ARRAY_FOREACH ( i, m_tUpd.m_dAttrs )
	{
		tOut.SendString ( m_tUpd.m_dAttrs[i].m_sName.cstr() );
		tOut.SendInt ( ( m_tUpd.m_dAttrs[i].m_eAttrType & SPH_ATTR_MULTI ) ? 1 : 0 );
	}
	tOut.SendInt ( m_tUpd.m_dDocids.GetLength() );

	ARRAY_FOREACH ( i, m_tUpd.m_dDocids )
	{
		int iHead = m_tUpd.m_dRowOffset[i];
		int iTail = ( (i+1)<m_tUpd.m_dDocids.GetLength() ) ? m_tUpd.m_dRowOffset[i+1] : m_tUpd.m_dPool.GetLength ();

		tOut.SendUint64 ( m_tUpd.m_dDocids[i] );
		for ( int j=iHead; j<iTail; j++ )
			tOut.SendDword ( m_tUpd.m_dPool[j] );
	}
}

static void DoCommandUpdate ( const char * sIndex, const CSphAttrUpdate & tUpd,
	int & iSuccesses, int & iUpdated, CSphVector < CSphPair < CSphString, DWORD > > & dUpdated,
	SearchFailuresLog_c & dFails, const ServedIndex_t * pServed )
{
	if ( !pServed || !pServed->m_pIndex )
	{
		dFails.Submit ( sIndex, "index not available" );
		return;
	}

	CSphString sError;
	DWORD uStatusDelta = pServed->m_pIndex->m_uAttrsStatus;
	int iUpd = pServed->m_pIndex->UpdateAttributes ( tUpd, -1, sError );
	uStatusDelta = pServed->m_pIndex->m_uAttrsStatus & ~uStatusDelta;

	if ( iUpd<0 )
	{
		dFails.Submit ( sIndex, sError.cstr() );

	} else
	{
		iUpdated += iUpd;
		iSuccesses++;

		CSphPair<CSphString, DWORD> tAdd;
		tAdd.m_tFirst = sIndex;
		tAdd.m_tSecond = uStatusDelta;
		dUpdated.Add ( tAdd );
	}
}

void HandleCommandUpdate ( int iSock, int iVer, InputBuffer_c & tReq, int iPipeFD )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_UPDATE, tReq ) )
		return;

	// parse request
	CSphString sIndexes = tReq.GetString ();
	CSphAttrUpdate tUpd;

	tUpd.m_dAttrs.Resize ( tReq.GetDword() ); // FIXME! check this
	ARRAY_FOREACH ( i, tUpd.m_dAttrs )
	{
		tUpd.m_dAttrs[i].m_sName = tReq.GetString ();
		tUpd.m_dAttrs[i].m_sName.ToLower ();

		tUpd.m_dAttrs[i].m_eAttrType = SPH_ATTR_INTEGER;
		if ( iVer>=0x102 )
			if ( tReq.GetDword() )
				tUpd.m_dAttrs[i].m_eAttrType |= SPH_ATTR_MULTI;
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
			DWORD uCount = 1;
			if ( tUpd.m_dAttrs[iAttr].m_eAttrType & SPH_ATTR_MULTI )
			{
				uCount = tReq.GetDword ();
				tUpd.m_dPool.Add ( uCount );
			}

			for ( DWORD j=0; j<uCount; j++ )
				tUpd.m_dPool.Add ( tReq.GetDword() );
		}
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

	CSphVector<DistributedIndex_t> dDistributed ( dIndexNames.GetLength() ); // lock safe storage for distributed indexes
	ARRAY_FOREACH ( i, dIndexNames )
	{
		if ( !g_pIndexes->Exists ( dIndexNames[i] ) )
		{
			// search amongst distributed and copy for further processing
			g_tDistLock.Lock();
			const DistributedIndex_t * pDistIndex = g_hDistIndexes ( dIndexNames[i] );

			if ( pDistIndex )
			{
				dDistributed[i] = *pDistIndex;
				dDistributed[i].m_bToDelete = true; // our presence flag
			}

			g_tDistLock.Unlock();

			if ( pDistIndex )
				continue;
			else
			{
				tReq.SendErrorReply ( "unknown index '%s' in update request", dIndexNames[i].cstr() );
				return;
			}
		}
	}

	// do update
	SearchFailuresLog_c dFails;
	int iSuccesses = 0;
	int iUpdated = 0;
	CSphVector < CSphPair < CSphString, DWORD > > dUpdated;

	ARRAY_FOREACH ( iIdx, dIndexNames )
	{
		const char * sReqIndex = dIndexNames[iIdx].cstr();
		const ServedIndex_t * pLocked = g_pIndexes->GetRlockedEntry ( sReqIndex );
		if ( pLocked )
		{
			DoCommandUpdate ( sReqIndex, tUpd, iSuccesses, iUpdated, dUpdated, dFails, pLocked );
			pLocked->Unlock();
		} else
		{
			assert ( dDistributed[iIdx].m_bToDelete );
			CSphVector<CSphString>& dLocal = dDistributed[iIdx].m_dLocal;

			ARRAY_FOREACH ( i, dLocal )
			{
				const char * sLocal = dLocal[i].cstr();
				const ServedIndex_t * pServed = g_pIndexes->GetRlockedEntry ( sLocal );
				DoCommandUpdate ( sLocal, tUpd, iSuccesses, iUpdated, dUpdated, dFails, pServed );
				if ( pServed )
					pServed->Unlock();
			}
		}

		// update remote agents
		if ( dDistributed[iIdx].m_bToDelete )
		{
			DistributedIndex_t & tDist = dDistributed[iIdx];

			CSphVector<AgentConn_t> dAgents ( tDist.m_dAgents.GetLength() );
			ARRAY_FOREACH ( i, dAgents )
				dAgents[i] = tDist.m_dAgents[i];

			// connect to remote agents and query them
			ConnectToRemoteAgents ( dAgents, false );

			UpdateRequestBuilder_t tReqBuilder ( tUpd );
			int iRemote = QueryRemoteAgents ( dAgents, tDist.m_iAgentConnectTimeout, tReqBuilder, NULL ); // FIXME? profile update time too?

			if ( iRemote )
			{
				UpdateReplyParser_t tParser ( &iUpdated );
				iSuccesses += WaitForRemoteAgents ( dAgents, tDist.m_iAgentQueryTimeout, tParser, NULL ); // FIXME? profile update time too?
			}
		}
	}

	// notify head daemon of local updates
	if ( iPipeFD>=0 )
	{
		DWORD uTmp = SPH_PIPE_UPDATED_ATTRS;
		sphWrite ( iPipeFD, &uTmp, sizeof(DWORD) ); // FIXME? add buffering/checks?

		uTmp = dUpdated.GetLength();
		sphWrite ( iPipeFD, &uTmp, sizeof(DWORD) );

		ARRAY_FOREACH ( i, dUpdated )
		{
			uTmp = strlen ( dUpdated[i].m_tFirst.cstr() );
			sphWrite ( iPipeFD, &uTmp, sizeof(DWORD) );
			sphWrite ( iPipeFD, dUpdated[i].m_tFirst.cstr(), uTmp );
			uTmp = dUpdated[i].m_tSecond;
			sphWrite ( iPipeFD, &uTmp, sizeof(DWORD) );
		}
	}

	// serve reply to client
	StrBuf_t sReport;
	dFails.BuildReport ( sReport );

	if ( !iSuccesses )
	{
		tReq.SendErrorReply ( "%s", sReport.cstr() );
		return;
	}

	NetOutputBuffer_c tOut ( iSock );
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

//////////////////////////////////////////////////////////////////////////
// STATUS HANDLER
//////////////////////////////////////////////////////////////////////////

static inline void FormatMsec ( CSphString & sOut, int64_t tmTime )
{
	sOut.SetSprintf ( "%d.%03d", (int)( tmTime/1000000 ), (int)( (tmTime%1000000)/1000 ) );
}


void BuildStatus ( CSphVector<CSphString> & dStatus )
{
	assert ( g_pStats );
	const char * FMT64 = INT64_FMT;
	const char * OFF = "OFF";

	const int64_t iQueriesDiv = Max ( g_pStats->m_iQueries, 1 );
	const int64_t iDistQueriesDiv = Max ( g_pStats->m_iDistQueries, 1 );

	// FIXME? non-transactional!!!
	dStatus.Add ( "uptime" );					dStatus.Add().SetSprintf ( "%u", (DWORD)time(NULL)-g_pStats->m_uStarted );
	dStatus.Add ( "connections" );				dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iConnections );
	dStatus.Add ( "maxed_out" );				dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iMaxedOut );
	dStatus.Add ( "command_search" );			dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iCommandCount[SEARCHD_COMMAND_SEARCH] );
	dStatus.Add ( "command_excerpt" );			dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iCommandCount[SEARCHD_COMMAND_EXCERPT] );
	dStatus.Add ( "command_update" ); 			dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iCommandCount[SEARCHD_COMMAND_UPDATE] );
	dStatus.Add ( "command_keywords" );			dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iCommandCount[SEARCHD_COMMAND_KEYWORDS] );
	dStatus.Add ( "command_persist" );			dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iCommandCount[SEARCHD_COMMAND_PERSIST] );
	dStatus.Add ( "command_status" );			dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iCommandCount[SEARCHD_COMMAND_STATUS] );
	dStatus.Add ( "command_flushattrs" );		dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iCommandCount[SEARCHD_COMMAND_FLUSHATTRS] );
	dStatus.Add ( "agent_connect" );			dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iAgentConnect );
	dStatus.Add ( "agent_retry" );				dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iAgentRetry );
	dStatus.Add ( "queries" );					dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iQueries );
	dStatus.Add ( "dist_queries" );				dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iDistQueries );

	g_tDistLock.Lock();
	g_hDistIndexes.IterateStart();
	while ( g_hDistIndexes.IterateNext() )
	{
		const char * sIdx = g_hDistIndexes.IterateGetKey().cstr();
		CSphVector<AgentDesc_t> & dAgents = g_hDistIndexes.IterateGet().m_dAgents;
		ARRAY_FOREACH ( i, dAgents )
		{
			int iIndex = dAgents[i].m_iStatsIndex;
			if ( iIndex<0 || iIndex>=STATS_MAX_AGENTS )
				continue;

			AgentStats_t & tStats = g_pStats->m_dAgentStats[iIndex];
			dStatus.Add().SetSprintf ( "ag_%s_%d_query_timeouts", sIdx, i );		dStatus.Add().SetSprintf ( FMT64, tStats.m_iTimeoutsQuery );
			dStatus.Add().SetSprintf ( "ag_%s_%d_connect_timeouts", sIdx, i );		dStatus.Add().SetSprintf ( FMT64, tStats.m_iTimeoutsConnect );
			dStatus.Add().SetSprintf ( "ag_%s_%d_connect_failures", sIdx, i );		dStatus.Add().SetSprintf ( FMT64, tStats.m_iConnectFailures );
			dStatus.Add().SetSprintf ( "ag_%s_%d_network_errors", sIdx, i );		dStatus.Add().SetSprintf ( FMT64, tStats.m_iNetworkErrors );
			dStatus.Add().SetSprintf ( "ag_%s_%d_wrong_replies", sIdx, i );			dStatus.Add().SetSprintf ( FMT64, tStats.m_iWrongReplies );
			dStatus.Add().SetSprintf ( "ag_%s_%d_unexpected_closings", sIdx, i );	dStatus.Add().SetSprintf ( FMT64, tStats.m_iUnexpectedClose );
		}
	}
	g_tDistLock.Unlock();

	dStatus.Add ( "query_wall" );				FormatMsec ( dStatus.Add(), g_pStats->m_iQueryTime );

	dStatus.Add ( "query_cpu" );
	if ( g_bCpuStats )
		FormatMsec ( dStatus.Add(), g_pStats->m_iQueryCpuTime );
	else
		dStatus.Add() = OFF;

	dStatus.Add ( "dist_wall" );				FormatMsec ( dStatus.Add(), g_pStats->m_iDistWallTime );
	dStatus.Add ( "dist_local" );				FormatMsec ( dStatus.Add(), g_pStats->m_iDistLocalTime );
	dStatus.Add ( "dist_wait" );				FormatMsec ( dStatus.Add(), g_pStats->m_iDistWaitTime );

	if ( g_bIOStats )
	{
		dStatus.Add ( "query_reads" );			dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iDiskReads );
		dStatus.Add ( "query_readkb" );			dStatus.Add().SetSprintf ( FMT64, g_pStats->m_iDiskReadBytes/1024 );
		dStatus.Add ( "query_readtime" );		FormatMsec ( dStatus.Add(), g_pStats->m_iDiskReadTime );
	} else
	{
		dStatus.Add ( "query_reads" );			dStatus.Add() = OFF;
		dStatus.Add ( "query_readkb" );			dStatus.Add() = OFF;
		dStatus.Add ( "query_readtime" );		dStatus.Add() = OFF;
	}

	dStatus.Add ( "avg_query_wall" );			FormatMsec ( dStatus.Add(), g_pStats->m_iQueryTime / iQueriesDiv );
	dStatus.Add ( "avg_query_cpu" );
	if ( g_bCpuStats )
		FormatMsec ( dStatus.Add(), g_pStats->m_iQueryCpuTime / iQueriesDiv );
	else
		dStatus.Add ( OFF );
	dStatus.Add ( "avg_dist_wall" );			FormatMsec ( dStatus.Add(), g_pStats->m_iDistWallTime / iDistQueriesDiv );
	dStatus.Add ( "avg_dist_local" );			FormatMsec ( dStatus.Add(), g_pStats->m_iDistLocalTime / iDistQueriesDiv );
	dStatus.Add ( "avg_dist_wait" );			FormatMsec ( dStatus.Add(), g_pStats->m_iDistWaitTime / iDistQueriesDiv );
	if ( g_bIOStats )
	{
		dStatus.Add ( "avg_query_reads" );		dStatus.Add().SetSprintf ( "%.1f", (float)( g_pStats->m_iDiskReads*10/iQueriesDiv )/10.0f );
		dStatus.Add ( "avg_query_readkb" );		dStatus.Add().SetSprintf ( "%.1f", (float)( g_pStats->m_iDiskReadBytes/iQueriesDiv )/1024.0f );
		dStatus.Add ( "avg_query_readtime" );	FormatMsec ( dStatus.Add(), g_pStats->m_iDiskReadTime/iQueriesDiv );
	} else
	{
		dStatus.Add ( "avg_query_reads" );		dStatus.Add() = OFF;
		dStatus.Add ( "avg_query_readkb" );		dStatus.Add() = OFF;
		dStatus.Add ( "avg_query_readtime" );	dStatus.Add() = OFF;
	}
}


void BuildMeta ( CSphVector<CSphString> & dStatus, const CSphQueryResultMeta & tMeta )
{
	if ( !tMeta.m_sError.IsEmpty() )
	{
		dStatus.Add ( "error" );
		dStatus.Add ( tMeta.m_sError );
	}

	if ( !tMeta.m_sWarning.IsEmpty() )
	{
		dStatus.Add ( "warning" );
		dStatus.Add ( tMeta.m_sWarning );
	}

	dStatus.Add ( "total" );
	dStatus.Add().SetSprintf ( "%d", tMeta.m_iMatches );

	dStatus.Add ( "total_found" );
	dStatus.Add().SetSprintf ( "%d", tMeta.m_iTotalMatches );

	dStatus.Add ( "time" );
	dStatus.Add().SetSprintf ( "%d.%03d", tMeta.m_iQueryTime/1000, tMeta.m_iQueryTime%1000 );

	int iWord = 0;
	tMeta.m_hWordStats.IterateStart();
	while ( tMeta.m_hWordStats.IterateNext() )
	{
		const CSphQueryResultMeta::WordStat_t & tStat = tMeta.m_hWordStats.IterateGet();

		dStatus.Add().SetSprintf ( "keyword[%d]", iWord );
		dStatus.Add ( tMeta.m_hWordStats.IterateGetKey() );

		dStatus.Add().SetSprintf ( "docs[%d]", iWord );
		dStatus.Add().SetSprintf ( "%d", tStat.m_iDocs );

		dStatus.Add().SetSprintf ( "hits[%d]", iWord );
		dStatus.Add().SetSprintf ( "%d", tStat.m_iHits );

		iWord++;
	}
}


void HandleCommandStatus ( int iSock, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_STATUS, tReq ) )
		return;

	if ( !g_pStats )
	{
		tReq.SendErrorReply ( "performance counters disabled" );
		return;
	}

	CSphVector<CSphString> dStatus;
	BuildStatus ( dStatus );

	int iRespLen = 8; // int rows, int cols
	ARRAY_FOREACH ( i, dStatus )
		iRespLen += 4 + strlen ( dStatus[i].cstr() );

	NetOutputBuffer_c tOut ( iSock );
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

void HandleCommandFlush ( int iSock, int iVer, InputBuffer_c & tReq )
{
	if ( !CheckCommandVersion ( iVer, VER_COMMAND_FLUSHATTRS, tReq ) )
		return;

	// only if flushes are enabled
	if ( g_iAttrFlushPeriod<=0 )
	{
		// flushes are disabled
		sphLogDebug ( "attrflush: attr_flush_period<=0, command ignored" );

	} else if ( g_eWorkers==MPM_NONE )
	{
		// --console mode, no async thread/process to handle the check
		sphLogDebug ( "attrflush: --console mode, command ignored" );

	} else
	{
		// force a check in head process, and wait it until completes
		// FIXME! semi active wait..
		sphLogDebug ( "attrflush: forcing check, tag=%d", g_pFlush->m_iFlushTag );
		g_pFlush->m_bForceCheck = true;
		while ( g_pFlush->m_bForceCheck )
			sphSleepMsec ( 1 );

		// if we are flushing now, wait until flush completes
		while ( g_pFlush->m_bFlushing )
			sphSleepMsec ( 10 );
		sphLogDebug ( "attrflush: check finished, tag=%d", g_pFlush->m_iFlushTag );
	}

	// return last flush tag, just for the fun of it
	NetOutputBuffer_c tOut ( iSock );
	tOut.SendWord ( SEARCHD_OK );
	tOut.SendWord ( VER_COMMAND_FLUSHATTRS );
	tOut.SendInt ( 4 ); // resplen, 1 dword
	tOut.SendInt ( g_pFlush->m_iFlushTag );
	tOut.Flush ();
	assert ( tOut.GetError()==true || tOut.GetSentCount()==12 ); // 8+resplen
}

/////////////////////////////////////////////////////////////////////////////
// GENERAL HANDLER
/////////////////////////////////////////////////////////////////////////////

#define THD_STATE(_state) { if ( pThd ) pThd->m_eThdState = _state; }

void HandleClientSphinx ( int iSock, const char * sClientIP, int iPipeFD, ThdDesc_t * pThd )
{
	MEMORY ( SPH_MEM_HANDLE_NONSQL );
	THD_STATE ( THD_HANDSHAKE );

	bool bPersist = false;
	int iTimeout = g_iReadTimeout;
	NetInputBuffer_c tBuf ( iSock );

	// send my version
	DWORD uServer = htonl ( SPHINX_SEARCHD_PROTO );
	if ( sphSockSend ( iSock, (char*)&uServer, sizeof(DWORD) )!=sizeof(DWORD) )
	{
		sphWarning ( "failed to send server version (client=%s)", sClientIP );
		return;
	}

	// get client version and request
	tBuf.ReadFrom ( 4 ); // FIXME! magic
	tBuf.GetInt (); // client version is for now unused
	do
	{
		// in "persistent connection" mode, we want interruptible waits
		// so that the worker child could be forcibly restarted
		THD_STATE ( THD_NET_READ );
		if ( !tBuf.ReadFrom ( 8, iTimeout, bPersist ) && g_bGotSigterm )
			break;

		if ( bPersist && tBuf.IsIntr() )
		{
			// SIGHUP means restart
			if ( g_bGotSighup )
				break;

			// otherwise, keep waiting
			continue;
		}

		int iCommand = tBuf.GetWord ();
		int iCommandVer = tBuf.GetWord ();
		int iLength = tBuf.GetInt ();
		if ( tBuf.GetError() )
		{
			// under high load, there can be pretty frequent accept() vs connect() timeouts
			// lets avoid agent log flood
			//
			// sphWarning ( "failed to receive client version and request (client=%s, error=%s)", sClientIP, sphSockError() );
			return;
		}

		// check request
		if ( iCommand<0 || iCommand>=SEARCHD_COMMAND_TOTAL
			|| iLength<0 || iLength>g_iMaxPacketSize )
		{
			// unknown command, default response header
			tBuf.SendErrorReply ( "unknown command (code=%d)", iCommand );

			// if request length is insane, low level comm is broken, so we bail out
			if ( iLength<0 || iLength>g_iMaxPacketSize )
			{
				sphWarning ( "ill-formed client request (length=%d out of bounds)", iLength );
				return;
			}
		}

		// count commands
		if ( g_pStats && iCommand>=0 && iCommand<SEARCHD_COMMAND_TOTAL )
		{
			g_tStatsMutex.Lock();
			g_pStats->m_iCommandCount[iCommand]++;
			g_tStatsMutex.Unlock();
		}

		// get request body
		assert ( iLength>=0 && iLength<=g_iMaxPacketSize );
		if ( iLength && !tBuf.ReadFrom ( iLength ) )
		{
			sphWarning ( "failed to receive client request body (client=%s, exp=%d)", sClientIP, iLength );
			return;
		}

		// set on query guard
		SphCrashLogger_c::SetLastQuery ( tBuf.GetBufferPtr(), iLength, false, iCommand, iCommandVer );

		// handle known commands
		assert ( iCommand>=0 && iCommand<SEARCHD_COMMAND_TOTAL );

		if ( pThd )
			pThd->m_sCommand = g_dApiCommands[iCommand];
		THD_STATE ( THD_QUERY );

		switch ( iCommand )
		{
			case SEARCHD_COMMAND_SEARCH:	HandleCommandSearch ( iSock, iCommandVer, tBuf ); break;
			case SEARCHD_COMMAND_EXCERPT:	HandleCommandExcerpt ( iSock, iCommandVer, tBuf ); break;
			case SEARCHD_COMMAND_KEYWORDS:	HandleCommandKeywords ( iSock, iCommandVer, tBuf ); break;
			case SEARCHD_COMMAND_UPDATE:	HandleCommandUpdate ( iSock, iCommandVer, tBuf, iPipeFD ); break;
			case SEARCHD_COMMAND_PERSIST:	bPersist = ( tBuf.GetInt()!=0 ); iTimeout = g_iClientTimeout; break;
			case SEARCHD_COMMAND_STATUS:	HandleCommandStatus ( iSock, iCommandVer, tBuf ); break;
			case SEARCHD_COMMAND_QUERY:		HandleCommandQuery ( iSock, iCommandVer, tBuf ); break;
			case SEARCHD_COMMAND_FLUSHATTRS:HandleCommandFlush ( iSock, iCommandVer, tBuf ); break;
			default:						assert ( 0 && "INTERNAL ERROR: unhandled command" ); break;
		}

		// set off query guard
		SphCrashLogger_c::SetLastQuery ( NULL, 0, false );
	} while ( bPersist );
	SafeClose ( iPipeFD );
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
	MYSQL_COL_LONGLONG	= 8,
	MYSQL_COL_STRING	= 254
};


void SendMysqlFieldPacket ( NetOutputBuffer_c & tOut, BYTE uPacketID, const char * sCol, MysqlColumnType_e eType )
{
	const char * sDB = "";
	const char * sTable = "";

	int iLen = 17 + MysqlPackedLen(sDB) + 2*( MysqlPackedLen(sTable) + MysqlPackedLen(sCol) );

	int iColLen = 0;
	switch ( eType )
	{
		case MYSQL_COL_DECIMAL:		iColLen = 20; break;
		case MYSQL_COL_LONG:		iColLen = 11; break;
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
	tOut.SendByte ( 8 ); // charset_nr, 8 is latin1
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


void SendMysqlErrorPacket ( NetOutputBuffer_c & tOut, BYTE uPacketID, const char * sError, MysqlErrors_e iErr=MYSQL_ERR_PARSE_ERROR )
{
	if ( sError==NULL )
		sError = "(null)";

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


void SendMysqlEofPacket ( NetOutputBuffer_c & tOut, BYTE uPacketID, int iWarns, bool bMoreResults=false )
{
	if ( iWarns<0 ) iWarns = 0;
	if ( iWarns>65535 ) iWarns = 65535;
	if ( bMoreResults )
#if USE_MYSQL
		iWarns |= ( SERVER_MORE_RESULTS_EXISTS<<16 );
#else
		iWarns = iWarns;
#endif

	tOut.SendLSBDword ( (uPacketID<<24) + 5 );
	tOut.SendByte ( 0xfe );
	tOut.SendLSBDword ( iWarns ); // N warnings, 0 status
}


void SendMysqlOkPacket ( NetOutputBuffer_c & tOut, BYTE uPacketID, int iAffectedRows=0, int iWarns=0, const char * sMessage=NULL )
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
	tOut.SendLSBDword ( iWarns );		// N warnings, 0 status
	if ( iMsgLen > 0 )
		tOut.SendBytes ( sMessage, iMsgLen );
}


bool operator < ( const CSphString & a, const CSphString & b )
{
	return strcmp ( a.cstr(), b.cstr() ) < 0;
}


void HandleMysqlInsert ( const SqlStmt_t & tStmt, NetOutputBuffer_c & tOut, BYTE uPacketID, bool bReplace, bool bCommit )
{
	MEMORY ( SPH_MEM_INSERT_SQL );

	CSphString sError;

	// get that index
	const ServedIndex_t * pServed = g_pIndexes->GetRlockedEntry ( tStmt.m_sIndex );
	if ( !pServed )
	{
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	if ( !pServed->m_bRT || !pServed->m_bEnabled )
	{
		pServed->Unlock();
		sError.SetSprintf ( "index '%s' does not support INSERT (enabled=%d)", tStmt.m_sIndex.cstr(), pServed->m_bEnabled );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	ISphRtIndex * pIndex = dynamic_cast<ISphRtIndex*> ( pServed->m_pIndex ); // FIXME? remove dynamic_cast?
	assert ( pIndex );

	// get schema, check values count
	const CSphSchema & tSchema = pIndex->GetInternalSchema();
	int iSchemaSz = tSchema.GetAttrsCount() + tSchema.m_dFields.GetLength() + 1;
	int iExp = tStmt.m_iSchemaSz;
	int iGot = tStmt.m_dInsertValues.GetLength();
	if ( !tStmt.m_dInsertSchema.GetLength() && ( iSchemaSz!=tStmt.m_iSchemaSz ) )
	{
		pServed->Unlock();
		sError.SetSprintf ( "column count does not match schema (expected %d, got %d)", iSchemaSz, iGot );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	if ( ( iGot % iExp )!=0 )
	{
		pServed->Unlock();
		sError.SetSprintf ( "column count does not match value count (expected %d, got %d)", iExp, iGot );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
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
			SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr(), MYSQL_ERR_PARSE_ERROR );
			return;
		}

		dCheck.Sort();

		ARRAY_FOREACH ( i, dCheck )
		if ( i>0 && dCheck[i-1]==dCheck[i] )
		{
			pServed->Unlock();
			sError.SetSprintf ( "column '%s' specified twice", dCheck[i].cstr() );
			SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr(), MYSQL_ERR_FIELD_SPECIFIED_TWICE );
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
			SendMysqlErrorPacket ( tOut, uPacketID, "column list must contain an 'id' column" );
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
			SendMysqlErrorPacket ( tOut, uPacketID, "fields must never be named 'id' (fix your config)" );
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
			SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
			return;
		}
	}

	CSphVector< const char * > dStrings;
	// convert attrs
	for ( int c=0; c<tStmt.m_iRowsAffected; c++ )
	{
		assert ( sError.IsEmpty() );

		CSphMatchVariant tDoc;
		tDoc.Reset ( tSchema.GetRowSize() );
		tDoc.m_iDocID = (SphDocID_t)CSphMatchVariant::ToDocid ( tStmt.m_dInsertValues[iIdIndex + c * iExp] );
		dStrings.Resize ( 0 );

		for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
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
				if ( tCol.m_eAttrType==SPH_ATTR_STRING )
					dStrings.Add ( NULL );
			} else
			{
				const SqlInsert_t & tVal = tStmt.m_dInsertValues[iQuerySchemaIdx + c * iExp];

				// sanity checks
				if ( tVal.m_iType!=TOK_QUOTED_STRING && tVal.m_iType!=TOK_CONST_INT && tVal.m_iType!=TOK_CONST_FLOAT )
				{
					sError.SetSprintf ( "raw %d, column %d: internal error: unknown insval type %d", 1+c, 1+iQuerySchemaIdx, tVal.m_iType ); // 1 for human base
					break;
				}

				// FIXME? index schema is lawfully static, but our temp match obviously needs to be dynamic
				bResult = tDoc.SetAttr ( tLoc, tVal, tCol.m_eAttrType );
				if ( tCol.m_eAttrType==SPH_ATTR_STRING )
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
		pIndex->AddDocument ( dFields.GetLength(), dFields.Begin(), tDoc, bReplace, dStrings.Begin(), sError );

		if ( !sError.IsEmpty() )
			break;
	}

	// fire exit
	if ( !sError.IsEmpty() )
	{
		pServed->Unlock();
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	// no errors so far
	if ( bCommit )
		pIndex->Commit ();

	pServed->Unlock();

	// my OK packet
	SendMysqlOkPacket ( tOut, uPacketID, tStmt.m_iRowsAffected );
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


void HandleMysqlCallSnippets ( NetOutputBuffer_c & tOut, BYTE uPacketID, SqlStmt_t & tStmt )
{
	CSphString sError;

	// string data, string index, string query, [named opts]
	if ( tStmt.m_dInsertValues.GetLength()!=3
		|| tStmt.m_dInsertValues[0].m_iType!=TOK_QUOTED_STRING
		|| tStmt.m_dInsertValues[1].m_iType!=TOK_QUOTED_STRING
		|| tStmt.m_dInsertValues[2].m_iType!=TOK_QUOTED_STRING )
	{
		SendMysqlErrorPacket ( tOut, uPacketID, "bad argument count or types in SNIPPETS() call" );
		return;
	}

	const ServedIndex_t * pServed = g_pIndexes->GetRlockedEntry ( tStmt.m_dInsertValues[1].m_sVal );
	if ( !pServed || !pServed->m_bEnabled || !pServed->m_pIndex )
	{
		sError.SetSprintf ( "no such index %s", tStmt.m_dInsertValues[1].m_sVal.cstr() );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	ExcerptQuery_t q;
	q.m_sSource = tStmt.m_dInsertValues[0].m_sVal; // OPTIMIZE?
	q.m_sWords = tStmt.m_dInsertValues[2].m_sVal;

	CSphString sPassageBoundaryMode;

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
		else if ( sOpt=="passage_boundary" )	{ sPassageBoundaryMode = v.m_sVal; iExpType = TOK_QUOTED_STRING; }

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
		else if ( sOpt=="load_files" )			{ q.m_bLoadFiles = ( v.m_iVal!=0 ); iExpType = TOK_CONST_INT; }
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
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		pServed->Unlock();
		return;
	}

	q.m_iPassageBoundary = sphGetPassageBoundary ( sPassageBoundaryMode );

	if ( !sphCheckOptionsSPZ ( q, sPassageBoundaryMode, sError ) )
	{
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		pServed->Unlock();
		return;
	}

	CSphIndex * pIndex = pServed->m_pIndex;
	CSphDict * pDict = pIndex->GetDictionary ();
	CSphScopedPtr<ISphTokenizer> pTokenizer ( pIndex->GetTokenizer()->Clone ( true ) );

	if ( q.m_iPassageBoundary &&
		( !pTokenizer->EnableSentenceIndexing ( sError ) || !pTokenizer->EnableZoneIndexing ( sError ) ) )
	{
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		pServed->Unlock();
		return;
	}

	// FIXME? cut-paste
	const CSphIndexSettings & tSettings = pIndex->GetSettings ();
	CSphHTMLStripper tStripper;
	CSphHTMLStripper * pStripper = NULL;
	if ( q.m_sStripMode=="strip"
		|| ( q.m_sStripMode=="index" && tSettings.m_bHtmlStrip ) )
	{
		CSphString sError;
		if ( q.m_sStripMode=="index" )
		{
			if (
				!tStripper.SetIndexedAttrs ( tSettings.m_sHtmlIndexAttrs.cstr (), sError ) ||
				!tStripper.SetRemovedElements ( tSettings.m_sHtmlRemoveElements.cstr (), sError ) )
			{
				sError.SetSprintf ( "HTML stripper config error: %s", sError.cstr() );
				SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
				pServed->Unlock();
				return;
			}
		}

		if ( !tSettings.m_sZonePrefix.IsEmpty() )
			tStripper.SetZonePrefix ( tSettings.m_sZonePrefix.cstr() );

		pStripper = &tStripper;
	}

	char * sResult = sphBuildExcerpt ( q, pDict, pTokenizer.Ptr(), &pIndex->GetMatchSchema(), pIndex, sError, pStripper );
	if ( !sResult )
	{
		sError.SetSprintf ( "highlighting failed: %s", sError.cstr() );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		pServed->Unlock();
		return;
	}

	// all ok, ship it
	pServed->Unlock ();

	// result set header packet
	tOut.SendLSBDword ( ((uPacketID++)<<24) + 2 );
	tOut.SendByte ( 1 ); // field count (snippet)
	tOut.SendByte ( 0 ); // extra

	// fields
	SendMysqlFieldPacket ( tOut, uPacketID++, "snippet", MYSQL_COL_STRING );
	SendMysqlEofPacket ( tOut, uPacketID++, 0 );

	// data
	tOut.SendLSBDword ( ((uPacketID++)<<24) + MysqlPackedLen ( sResult ) );
	tOut.SendMysqlString ( sResult );

	SendMysqlEofPacket ( tOut, uPacketID++, 0 );
	SafeDeleteArray ( sResult );
}


void HandleMysqlCallKeywords ( NetOutputBuffer_c & tOut, BYTE uPacketID, SqlStmt_t & tStmt )
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
		SendMysqlErrorPacket ( tOut, uPacketID, "bad argument count or types in KEYWORDS() call" );
		return;
	}

	const ServedIndex_t * pServed = g_pIndexes->GetRlockedEntry ( tStmt.m_dInsertValues[1].m_sVal );
	if ( !pServed || !pServed->m_bEnabled || !pServed->m_pIndex )
	{
		sError.SetSprintf ( "no such index %s", tStmt.m_dInsertValues[1].m_sVal.cstr() );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	CSphVector<CSphKeywordInfo> dKeywords;
	bool bStats = ( iArgs==3 && tStmt.m_dInsertValues[2].m_iVal!=0 );
	bool bRes = pServed->m_pIndex->GetKeywords ( dKeywords, tStmt.m_dInsertValues[0].m_sVal.cstr(), bStats, sError );
	pServed->Unlock ();

	if ( !bRes )
	{
		sError.SetSprintf ( "keyword extraction failed: %s", sError.cstr() );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	// result set header packet
	tOut.SendLSBDword ( ((uPacketID++)<<24) + 2 );
	tOut.SendByte ( 2 + ( bStats ? 2 : 0 ) ); // field count (tokenized, normalized, docs, hits)
	tOut.SendByte ( 0 ); // extra

	// fields
	SendMysqlFieldPacket ( tOut, uPacketID++, "tokenized", MYSQL_COL_STRING );
	SendMysqlFieldPacket ( tOut, uPacketID++, "normalized", MYSQL_COL_STRING );
	if ( bStats )
	{
		SendMysqlFieldPacket ( tOut, uPacketID++, "docs", MYSQL_COL_STRING );
		SendMysqlFieldPacket ( tOut, uPacketID++, "hits", MYSQL_COL_STRING );
	}
	SendMysqlEofPacket ( tOut, uPacketID++, 0 );

	// data
	ARRAY_FOREACH ( i, dKeywords )
	{
		char sDocs[16], sHits[16];
		snprintf ( sDocs, sizeof(sDocs), "%d", dKeywords[i].m_iDocs );
		snprintf ( sHits, sizeof(sHits), "%d", dKeywords[i].m_iHits );

		int iPacketLen = MysqlPackedLen ( dKeywords[i].m_sTokenized.cstr() ) + MysqlPackedLen ( dKeywords[i].m_sNormalized.cstr() );
		if ( bStats )
			iPacketLen += MysqlPackedLen ( sDocs ) + MysqlPackedLen ( sHits );

		tOut.SendLSBDword ( ((uPacketID++)<<24) + iPacketLen );
		tOut.SendMysqlString ( dKeywords[i].m_sTokenized.cstr() );
		tOut.SendMysqlString ( dKeywords[i].m_sNormalized.cstr() );
		if ( bStats )
		{
			tOut.SendMysqlString ( sDocs );
			tOut.SendMysqlString ( sHits );
		}
	}

	SendMysqlEofPacket ( tOut, uPacketID++, 0 );
}


void HandleMysqlDescribe ( NetOutputBuffer_c & tOut, BYTE uPacketID, SqlStmt_t & tStmt )
{
	const ServedIndex_t * pServed = g_pIndexes->GetRlockedEntry ( tStmt.m_sIndex );
	if ( !pServed || !pServed->m_bEnabled || !pServed->m_pIndex )
	{
		CSphString sError;
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr(), MYSQL_ERR_NO_SUCH_TABLE );
		return;
	}

	// result set header packet
	tOut.SendLSBDword ( ((uPacketID++)<<24) + 2 );
	tOut.SendByte ( 2 ); // field count (field, type)
	tOut.SendByte ( 0 ); // extra

	// fields
	SendMysqlFieldPacket ( tOut, uPacketID++, "Field", MYSQL_COL_STRING );
	SendMysqlFieldPacket ( tOut, uPacketID++, "Type", MYSQL_COL_STRING );
	SendMysqlEofPacket ( tOut, uPacketID++, 0 );

	// data
	const char * sIdType = USE_64BIT ? "bigint" : "integer";
	tOut.SendLSBDword ( ((uPacketID++)<<24) + 3 + MysqlPackedLen ( sIdType ) );
	tOut.SendMysqlString ( "id" );
	tOut.SendMysqlString ( sIdType );

	const CSphSchema & tSchema = pServed->m_pIndex->GetMatchSchema();
	ARRAY_FOREACH ( i, tSchema.m_dFields )
	{
		const CSphColumnInfo & tCol = tSchema.m_dFields[i];
		tOut.SendLSBDword ( ((uPacketID++)<<24) + MysqlPackedLen ( tCol.m_sName.cstr() ) + 6 );
		tOut.SendMysqlString ( tCol.m_sName.cstr() );
		tOut.SendMysqlString ( "field" );
	}

	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(i);

		const char * sType = "?";
		switch ( tCol.m_eAttrType )
		{
			case SPH_ATTR_NONE:							sType = "none"; break;
			case SPH_ATTR_INTEGER:						sType = "integer"; break;
			case SPH_ATTR_TIMESTAMP:					sType = "timestamp"; break;
			case SPH_ATTR_ORDINAL:						sType = "ordinal"; break;
			case SPH_ATTR_BOOL:							sType = "bool"; break;
			case SPH_ATTR_FLOAT:						sType = "float"; break;
			case SPH_ATTR_BIGINT:						sType = "bigint"; break;
			case SPH_ATTR_STRING:						sType = "string"; break;
			case SPH_ATTR_INTEGER | SPH_ATTR_MULTI:		sType = "mva"; break;
		}

		tOut.SendLSBDword ( ((uPacketID++)<<24) + MysqlPackedLen ( tCol.m_sName.cstr() ) + MysqlPackedLen ( sType ) );
		tOut.SendMysqlString ( tCol.m_sName.cstr() );
		tOut.SendMysqlString ( sType );
	}

	pServed->Unlock();
	SendMysqlEofPacket ( tOut, uPacketID++, 0 );
}


struct IndexNameLess_fn
{
	inline bool IsLess ( const CSphNamedInt & a, const CSphNamedInt & b ) const
	{
		return strcasecmp ( a.m_sName.cstr(), b.m_sName.cstr() )<0;
	}
};


void HandleMysqlShowTables ( NetOutputBuffer_c & tOut, BYTE uPacketID )
{
	// result set header packet
	tOut.SendLSBDword ( ((uPacketID++)<<24) + 2 );
	tOut.SendByte ( 2 ); // field count (index, type)
	tOut.SendByte ( 0 ); // extra

	// fields
	SendMysqlFieldPacket ( tOut, uPacketID++, "Index", MYSQL_COL_STRING );
	SendMysqlFieldPacket ( tOut, uPacketID++, "Type", MYSQL_COL_STRING );
	SendMysqlEofPacket ( tOut, uPacketID++, 0 );

	// all the indexes
	// 0 local, 1 distributed, 2 rt
	CSphVector<CSphNamedInt> dIndexes;

	for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
		if ( it.Get().m_bEnabled )
	{
		CSphNamedInt & tIdx = dIndexes.Add();
		tIdx.m_sName = it.GetKey();
		tIdx.m_iValue = it.Get().m_bRT ? 2 : 0;
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
	ARRAY_FOREACH ( i, dIndexes )
	{
		const char * sType = "?";
		switch ( dIndexes[i].m_iValue )
		{
			case 0: sType = "local"; break;
			case 1: sType = "distributed"; break;
			case 2: sType = "rt"; break;
		}

		tOut.SendLSBDword ( ((uPacketID++)<<24) + MysqlPackedLen ( dIndexes[i].m_sName.cstr() ) + MysqlPackedLen ( sType ) );
		tOut.SendMysqlString ( dIndexes[i].m_sName.cstr() );
		tOut.SendMysqlString ( sType );
	}

	SendMysqlEofPacket ( tOut, uPacketID++, 0 );
}

//////////////////////////////////////////////////////////////////////////

void HandleMysqlUpdate ( NetOutputBuffer_c & tOut, BYTE uPacketID, const SqlStmt_t & tStmt, bool bCommit )
{
	CSphString sError;

	const ServedIndex_t * pServed = g_pIndexes->GetRlockedEntry ( tStmt.m_sIndex );
	if ( !pServed )
	{
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	if ( !pServed->m_bEnabled )
	{
		pServed->Unlock();
		sError.SetSprintf ( "index '%s' does not support Update (enabled=%d)", tStmt.m_sIndex.cstr(), pServed->m_bEnabled );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	int iAffected = pServed->m_pIndex->UpdateAttributes ( tStmt.m_tUpdate, -1, sError );
	if ( iAffected<0 )
	{
		pServed->Unlock();
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	if ( bCommit && pServed->m_bRT )
	{
		ISphRtIndex * pIndex = static_cast<ISphRtIndex *> ( pServed->m_pIndex );
		pIndex->Commit ();
	}

	pServed->Unlock();

	SendMysqlOkPacket ( tOut, uPacketID, iAffected );
}

//////////////////////////////////////////////////////////////////////////

#define SPH_MAX_NUMERIC_STR 32

class SqlRowBuffer_c
{
public:

	SqlRowBuffer_c ()
		: m_pBuf ( NULL )
		, m_iLen ( 0 )
		, m_iLimit ( sizeof ( m_dBuf ) )
	{
	}

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
		int iLen = sMsg ? strlen ( sMsg ) : 0;
		iLen = Min ( iLen, 0xff );
		Reserve ( 1+iLen );
		*Get() = BYTE(iLen);
		if ( iLen )
			memcpy ( Get()+1, sMsg, iLen );
		IncPtr ( 1+iLen );
	}


private:

	char m_dBuf[4096];
	char * m_pBuf;
	int m_iLen;
	int m_iLimit;
};


bool HandleMysqlSelect ( NetOutputBuffer_c & tOut, BYTE & uPacketID, SearchHandler_c & tHandler )
{
	// lets check all query for errors
	CSphString sError;
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
	{
		CheckQuery ( tHandler.m_dQueries[i], tHandler.m_dResults[i].m_sError );
		if ( !tHandler.m_dResults[i].m_sError.IsEmpty() )
			sError.SetSprintf ( "%squery %d error: %s ; ", sError.cstr(), i, tHandler.m_dResults[i].m_sError.cstr() );
	}

	if ( sError.Length() )
	{
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return false;
	}

	// actual searching
	tHandler.RunQueries ();

	if ( g_bGotSigterm )
	{
		sphLogDebug ( "HandleClientMySQL: got SIGTERM, sending the packet MYSQL_ERR_SERVER_SHUTDOWN" );
		SendMysqlErrorPacket ( tOut, uPacketID, "Server shutdown in progress", MYSQL_ERR_SERVER_SHUTDOWN );
		return false;
	}

	return true;
}


void SendMysqlSelectResult ( NetOutputBuffer_c & tOut, BYTE & uPacketID, SqlRowBuffer_c & dRows, const AggrResult_t & tRes, bool bMoreResultsFollow )
{
	if ( !tRes.m_iSuccesses )
	{
		SendMysqlErrorPacket ( tOut, uPacketID++, tRes.m_sError.cstr() );
		return;
	}

	bool bStar = tRes.m_tSchema.m_sName=="*";

	// result set header packet
	tOut.SendLSBDword ( ((uPacketID++)<<24) + 2 );
	if ( g_bCompatResults )
		tOut.SendByte ( BYTE ( 2+tRes.m_tSchema.GetAttrsCount() ) );
	else
		tOut.SendByte ( BYTE ( tRes.m_tSchema.GetAttrsCount() + (bStar?1:0) ) );
	tOut.SendByte ( 0 ); // extra

	// field packets
	if ( g_bCompatResults )
	{
		SendMysqlFieldPacket ( tOut, uPacketID++, "id", USE_64BIT ? MYSQL_COL_LONGLONG : MYSQL_COL_LONG );
		SendMysqlFieldPacket ( tOut, uPacketID++, "weight", MYSQL_COL_LONG );
	} else
	{
		// in case of star schema send first the DocID
		if ( bStar )
			SendMysqlFieldPacket ( tOut, uPacketID++, "id", USE_64BIT ? MYSQL_COL_LONGLONG : MYSQL_COL_LONG );
	}

	for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
		MysqlColumnType_e eType = MYSQL_COL_STRING;
		if ( tCol.m_eAttrType==SPH_ATTR_INTEGER || tCol.m_eAttrType==SPH_ATTR_TIMESTAMP || tCol.m_eAttrType==SPH_ATTR_BOOL )
			eType = MYSQL_COL_LONG;
		if ( tCol.m_eAttrType==SPH_ATTR_BIGINT )
			eType = MYSQL_COL_LONGLONG;
		if ( tCol.m_eAttrType==SPH_ATTR_STRING )
			eType = MYSQL_COL_STRING;
		SendMysqlFieldPacket ( tOut, uPacketID++, tCol.m_sName.cstr(), eType );
	}

	// eof packet
	BYTE iWarns = ( !tRes.m_sWarning.IsEmpty() ) ? 1 : 0;
	SendMysqlEofPacket ( tOut, uPacketID++, iWarns, bMoreResultsFollow );

	// rows
	dRows.Reset();

	for ( int iMatch = tRes.m_iOffset; iMatch < tRes.m_iOffset + tRes.m_iCount; iMatch++ )
	{
		const CSphMatch & tMatch = tRes.m_dMatches [ iMatch ];

		if ( g_bCompatResults )
		{
			dRows.PutNumeric<SphDocID_t> ( DOCID_FMT, tMatch.m_iDocID );
			dRows.PutNumeric ( "%u", tMatch.m_iWeight );
		} else
		{
			if ( bStar )
				dRows.PutNumeric<SphDocID_t> ( DOCID_FMT, tMatch.m_iDocID );
		}

		const CSphSchema & tSchema = tRes.m_tSchema;
		for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
		{
			CSphAttrLocator tLoc = tSchema.GetAttr(i).m_tLocator;
			DWORD eAttrType = tSchema.GetAttr(i).m_eAttrType;

			switch ( eAttrType )
			{
			case SPH_ATTR_INTEGER:
			case SPH_ATTR_TIMESTAMP:
			case SPH_ATTR_BOOL:
			case SPH_ATTR_BIGINT:
				if ( eAttrType==SPH_ATTR_BIGINT )
					dRows.PutNumeric<SphAttr_t> ( INT64_FMT, tMatch.GetAttr(tLoc) );
				else
					dRows.PutNumeric<DWORD> ( "%u", (DWORD)tMatch.GetAttr(tLoc) );
				break;

			case SPH_ATTR_FLOAT:
				dRows.PutNumeric ( "%f", tMatch.GetAttrFloat(tLoc) );
				break;

			case SPH_ATTR_INTEGER | SPH_ATTR_MULTI:
				{
					int iLenOff = dRows.Length();
					dRows.IncPtr ( 4 );

					assert ( tRes.m_dTag2Pools [ tMatch.m_iTag ].m_pMva );
					const DWORD * pValues = tMatch.GetAttrMVA ( tLoc, tRes.m_dTag2Pools [ tMatch.m_iTag ].m_pMva );
					if ( pValues )
					{
						DWORD nValues = *pValues++;
						while ( nValues-- )
						{
							dRows.Reserve ( SPH_MAX_NUMERIC_STR );
							int iLen = snprintf ( dRows.Get(), SPH_MAX_NUMERIC_STR, nValues>0 ? "%u," : "%u", *pValues++ );
							dRows.IncPtr ( iLen );
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

					// send length
					dRows.Reserve ( iLen+4 );
					char * pOutStr = (char*)MysqlPack ( dRows.Get(), iLen );

					// send string data
					if ( iLen )
						memcpy ( pOutStr, pStr, iLen );

					dRows.IncPtr ( pOutStr-dRows.Get()+iLen );
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

		tOut.SendLSBDword ( ((uPacketID++)<<24) + ( dRows.Length() ) );
		tOut.SendBytes ( dRows.Off ( 0 ), dRows.Length() );
		dRows.Reset();
	}

	// eof packet
	SendMysqlEofPacket ( tOut, uPacketID++, iWarns, bMoreResultsFollow );
}


void HandleMysqlWarning ( NetOutputBuffer_c & tOut, BYTE & uPacketID, const CSphQueryResultMeta & tLastMeta, SqlRowBuffer_c & dRows, bool bMoreResultsFollow )
{
	// can't send simple ok if there are more results to send
	// as it breaks order of multi-result output
	if ( tLastMeta.m_sWarning.IsEmpty() && !bMoreResultsFollow )
	{
		SendMysqlOkPacket ( tOut, uPacketID );
		return;
	}

	// result set header packet
	tOut.SendLSBDword ( ((uPacketID++)<<24) + 2 );
	tOut.SendByte ( 3 ); // field count (level+code+message)
	tOut.SendByte ( 0 ); // extra

	// field packets
	SendMysqlFieldPacket ( tOut, uPacketID++, "Level", MYSQL_COL_STRING );
	SendMysqlFieldPacket ( tOut, uPacketID++, "Code", MYSQL_COL_DECIMAL );
	SendMysqlFieldPacket ( tOut, uPacketID++, "Message", MYSQL_COL_STRING );
	SendMysqlEofPacket ( tOut, uPacketID++, 0, bMoreResultsFollow );

	// row
	dRows.Reset();
	dRows.PutString ( "warning" );
	dRows.PutString ( "1000" );
	dRows.PutString ( tLastMeta.m_sWarning.cstr() );

	tOut.SendLSBDword ( ((uPacketID++)<<24) + ( dRows.Length() ) );
	tOut.SendBytes ( dRows.Off ( 0 ), dRows.Length() );
	dRows.Reset();

	// cleanup
	SendMysqlEofPacket ( tOut, uPacketID++, 0, bMoreResultsFollow );
}


void HandleMysqlMeta ( NetOutputBuffer_c & tOut, BYTE & uPacketID, const CSphQueryResultMeta & tLastMeta, SqlRowBuffer_c & dRows, bool bStatus, bool bMoreResultsFollow )
{
	CSphVector<CSphString> dStatus;

	if ( bStatus )
		BuildStatus ( dStatus );
	else
		BuildMeta ( dStatus, tLastMeta );

	// result set header packet
	tOut.SendLSBDword ( ((uPacketID++)<<24) + 2 );
	tOut.SendByte ( 2 ); // field count (level+code+message)
	tOut.SendByte ( 0 ); // extra

	// field packets
	SendMysqlFieldPacket ( tOut, uPacketID++, "Variable_name", MYSQL_COL_STRING );
	SendMysqlFieldPacket ( tOut, uPacketID++, "Value", MYSQL_COL_STRING );
	SendMysqlEofPacket ( tOut, uPacketID++, 0, bMoreResultsFollow );

	// send rows
	dRows.Reset();

	for ( int iRow=0; iRow<dStatus.GetLength(); iRow+=2 )
	{
		dRows.PutString ( dStatus[iRow+0].cstr() );
		dRows.PutString ( dStatus[iRow+1].cstr() );

		tOut.SendLSBDword ( ((uPacketID++)<<24) + ( dRows.Length() ) );
		tOut.SendBytes ( dRows.Off ( 0 ), dRows.Length() );
		dRows.Reset();
	}

	// cleanup
	SendMysqlEofPacket ( tOut, uPacketID++, 0, bMoreResultsFollow );
}


void HandleMysqlDelete ( NetOutputBuffer_c & tOut, BYTE & uPacketID, const SqlStmt_t & tStmt, bool bCommit )
{
	MEMORY ( SPH_MEM_DELETE_SQL );

	CSphString sError;

	const ServedIndex_t * pServed = g_pIndexes->GetRlockedEntry ( tStmt.m_sIndex );
	if ( !pServed )
	{
		sError.SetSprintf ( "no such index '%s'", tStmt.m_sIndex.cstr() );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	if ( !pServed->m_bRT || !pServed->m_bEnabled )
	{
		pServed->Unlock();
		sError.SetSprintf ( "index '%s' does not support DELETE (enabled=%d)", tStmt.m_sIndex.cstr(), pServed->m_bEnabled );
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	ISphRtIndex * pIndex = static_cast<ISphRtIndex *> ( pServed->m_pIndex );

	if ( !pIndex->DeleteDocument ( tStmt.m_iDeleteID, sError ) )
	{
		pServed->Unlock();
		SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
		return;
	}

	if ( bCommit )
		pIndex->Commit ();

	pServed->Unlock();

	SendMysqlOkPacket ( tOut, uPacketID ); // FIXME? affected rows
}


void HandleMysqlMultiStmt ( NetOutputBuffer_c & tOut, BYTE uPacketID, const CSphVector<SqlStmt_t> & dStmt, CSphQueryResultMeta & tLastMeta
						, SqlRowBuffer_c & dRows, ThdDesc_t * pThd )
{
	// select count
	int iSelect = 0;
	ARRAY_FOREACH ( i, dStmt )
		if ( dStmt[i].m_eStmt==STMT_SELECT )
			iSelect++;

	if ( !iSelect )
	{
		tLastMeta = CSphQueryResultMeta();
		return;
	}

	if ( pThd )
		pThd->m_sCommand = g_dSqlStmts[STMT_SELECT];

	// setup query for searching
	SearchHandler_c tHandler ( iSelect );
	iSelect = 0;
	ARRAY_FOREACH ( i, dStmt )
	{
		if ( dStmt[i].m_eStmt==STMT_SELECT )
			tHandler.m_dQueries[iSelect++] = dStmt[i].m_tQuery;
	}

	// do search
	bool bSearchOK = HandleMysqlSelect ( tOut, uPacketID, tHandler );

	// save meta for SHOW *
	CSphQueryResultMeta tPrevMeta = tLastMeta;
	tLastMeta = tHandler.m_dResults.Last();

	if ( !bSearchOK )
		return;

	// send multi-result set
	iSelect = 0;
	ARRAY_FOREACH ( i, dStmt )
	{
		SqlStmt_e eStmt = dStmt[i].m_eStmt;

		THD_STATE ( THD_QUERY );
		if ( pThd )
			pThd->m_sCommand = g_dSqlStmts[eStmt];

		const CSphQueryResultMeta & tMeta = iSelect-1>=0 ? tHandler.m_dResults[iSelect-1] : tPrevMeta;
		bool bMoreResultsFollow = (i+1)<dStmt.GetLength();

		if ( eStmt==STMT_SELECT )
		{
			const AggrResult_t & tRes = tHandler.m_dResults[iSelect++];

			SendMysqlSelectResult ( tOut, uPacketID, dRows, tRes, bMoreResultsFollow );
		} else if ( eStmt==STMT_SHOW_WARNINGS )
			HandleMysqlWarning ( tOut, uPacketID, tMeta, dRows, bMoreResultsFollow );
		else if ( eStmt==STMT_SHOW_STATUS || eStmt==STMT_SHOW_META )
			HandleMysqlMeta ( tOut, uPacketID, tMeta, dRows, eStmt==STMT_SHOW_STATUS, bMoreResultsFollow );

		if ( g_bGotSigterm )
		{
			sphLogDebug ( "HandleMultiStmt: got SIGTERM, sending the packet MYSQL_ERR_SERVER_SHUTDOWN" );
			SendMysqlErrorPacket ( tOut, uPacketID, "Server shutdown in progress", MYSQL_ERR_SERVER_SHUTDOWN );
			return;
		}
	}
}


void HandleClientMySQL ( int iSock, const char * sClientIP, int iPipeFD, ThdDesc_t * pThd )
{
	MEMORY ( SPH_MEM_HANDLE_SQL );
	THD_STATE ( THD_HANDSHAKE );

	// handshake
	char sHandshake[] =
		"\x00\x00\x00" // packet length
		"\x00" // packet id
		"\x0A" // protocol version; v.10
		SPHINX_VERSION "\x00" // server version
		"\x01\x00\x00\x00" // thread id
		"\x01\x02\x03\x04\x05\x06\x07\x08" // scramble buffer (for auth)
		"\x00" // filler
		"\x08\x02" // server capabilities; CLIENT_PROTOCOL_41 | CLIENT_CONNECT_WITH_DB
		"\x00" // server language
		"\x02\x00" // server status
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // filler
		"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d"; // scramble buffer2 (for auth, 4.1+)

	const int INTERACTIVE_TIMEOUT = 900;
	NetInputBuffer_c tIn ( iSock );
	NetOutputBuffer_c tOut ( iSock ); // OPTIMIZE? looks like buffer size matters a lot..

	sHandshake[0] = sizeof(sHandshake)-5;
	if ( sphSockSend ( iSock, sHandshake, sizeof(sHandshake)-1 )!=sizeof(sHandshake)-1 )
	{
		sphWarning ( "failed to send server version (client=%s)", sClientIP );
		return;
	}

	bool bAuthed = false;
	BYTE uPacketID = 1;

	CSphQueryResultMeta tLastMeta;

	// set on client's level, not on query's
	bool bAutoCommit = true;
	bool bInTransaction = false; // ignores bAutoCommit inside transaction

	// to keep data alive for SphCrashQuery_c
	CSphString sQuery;

	for ( ;; )
	{
		// set off query guard
		SphCrashLogger_c::SetLastQuery ( NULL, 0, true );

		CSphString sError;

		// send the packet formed on the previous cycle
		THD_STATE ( THD_NET_WRITE );
		if ( !tOut.Flush() )
			break;

		// get next packet
		// we want interruptible calls here, so that shutdowns could be honoured
		THD_STATE ( THD_NET_READ );
		if ( !tIn.ReadFrom ( 4, INTERACTIVE_TIMEOUT, true ) )
			break;

		DWORD uPacketHeader = tIn.GetLSBDword ();
		int iPacketLen = ( uPacketHeader & 0xffffffUL );
		if ( !tIn.ReadFrom ( iPacketLen, INTERACTIVE_TIMEOUT, true ) )
			break;

		// handle it!
		uPacketID = 1 + (BYTE)( uPacketHeader>>24 ); // client will expect this id

		// handle auth packet
		if ( !bAuthed )
		{
			bAuthed = true;
			SendMysqlOkPacket ( tOut, uPacketID );
			continue;
		}

		// get command, handle special packets
		const BYTE uMysqlCmd = tIn.GetByte ();
		if ( uMysqlCmd==MYSQL_COM_QUIT )
		{
			// client is done
			break;

		} else if ( uMysqlCmd==MYSQL_COM_PING || uMysqlCmd==MYSQL_COM_INIT_DB )
		{
			// client wants a pong
			SendMysqlOkPacket ( tOut, uPacketID );
			continue;

		} else if ( uMysqlCmd==MYSQL_COM_SET_OPTION )
		{
			// bMulti = ( tIn.GetWord()==MYSQL_OPTION_MULTI_STATEMENTS_ON ); // that's how we could double check and validate multi query
			SendMysqlOkPacket ( tOut, uPacketID );
			continue;

		} else if ( uMysqlCmd!=MYSQL_COM_QUERY )
		{
			// default case, unknown command
			// (and query is handled just below)
			sError.SetSprintf ( "unknown command (code=%d)", uMysqlCmd );
			SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr(), MYSQL_ERR_UNKNOWN_COM_ERROR );
			continue;
		}

		// handle query packet
		assert ( uMysqlCmd==MYSQL_COM_QUERY );
		sQuery = tIn.GetRawString ( iPacketLen-1 );

		// set on query guard
		SphCrashLogger_c::SetLastQuery ( (const BYTE *)sQuery.cstr(), sQuery.Length(), true );

		// parse SQL query
		CSphVector<SqlStmt_t> dStmt;
		bool bParsedOK = ParseSqlQuery ( sQuery, dStmt, sError );

		SqlStmt_e eStmt = bParsedOK ? dStmt.Begin()->m_eStmt : STMT_PARSE_ERROR;

		SqlStmt_t * pStmt = dStmt.Begin();
		assert ( !bParsedOK || pStmt );

		if ( pThd )
			pThd->m_sCommand = g_dSqlStmts[eStmt];
		THD_STATE ( THD_QUERY );

		SqlRowBuffer_c dRows;

		// handle multi SQL query
		if ( bParsedOK && dStmt.GetLength()>1 )
		{
			HandleMysqlMultiStmt ( tOut, uPacketID, dStmt, tLastMeta, dRows, pThd );
			continue;
		}

		// handle SQL query
		switch ( eStmt )
		{
		case STMT_PARSE_ERROR:
			SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
			break;

		case STMT_SELECT:
		{
			MEMORY ( SPH_MEM_SELECT_SQL );

			SearchHandler_c tHandler ( 1 );
			tHandler.m_dQueries[0] = dStmt.Begin()->m_tQuery;

			if ( HandleMysqlSelect ( tOut, uPacketID, tHandler ) )
				SendMysqlSelectResult ( tOut, uPacketID, dRows, tHandler.m_dResults.Last(), false );

			// save meta for SHOW META
			tLastMeta = tHandler.m_dResults.Last();

			break;
		}
		case STMT_SHOW_WARNINGS:
			HandleMysqlWarning ( tOut, uPacketID, tLastMeta, dRows, false );
			break;

		case STMT_SHOW_STATUS:
		case STMT_SHOW_META:
			HandleMysqlMeta ( tOut, uPacketID, tLastMeta, dRows, eStmt==STMT_SHOW_STATUS, false );
			break;

		case STMT_INSERT:
		case STMT_REPLACE:
			HandleMysqlInsert ( *pStmt, tOut, uPacketID, eStmt==STMT_REPLACE, bAutoCommit && !bInTransaction );
			continue;

		case STMT_DELETE:
			HandleMysqlDelete ( tOut, uPacketID, *pStmt, bAutoCommit && !bInTransaction );
			continue;

		case STMT_SET:
			{
				MEMORY ( SPH_MEM_COMMIT_SET_SQL );

				pStmt->m_sSetName.ToLower();
				if ( pStmt->m_sSetName=="autocommit" )
				{
					bAutoCommit = (pStmt->m_iSetValue==0)?false:true;
					bInTransaction = false;

					// commit all pending changes
					if ( bAutoCommit )
					{
						ISphRtIndex * pIndex = sphGetCurrentIndexRT();
						if ( pIndex )
							pIndex->Commit();
					}

					SendMysqlOkPacket ( tOut, uPacketID );
					continue;
				}

				// unknown variable, return error
				sError.SetSprintf ( "Unknown variable '%s' in SET statement", pStmt->m_sSetName.cstr() );
				SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
				continue;
			}
		case STMT_BEGIN:
			{
				MEMORY ( SPH_MEM_COMMIT_BEGIN_SQL );

				bInTransaction = true;
				ISphRtIndex * pIndex = sphGetCurrentIndexRT();
				if ( pIndex )
					pIndex->Commit();
				SendMysqlOkPacket ( tOut, uPacketID );
				continue;
			}
		case STMT_COMMIT:
		case STMT_ROLLBACK:
			{
				MEMORY ( SPH_MEM_COMMIT_SQL );

				bInTransaction = false;
				ISphRtIndex * pIndex = sphGetCurrentIndexRT();
				if ( pIndex )
				{
					if ( eStmt==STMT_COMMIT )
						pIndex->Commit();
					else
						pIndex->RollBack();
				}
				SendMysqlOkPacket ( tOut, uPacketID );
				continue;
			}
		case STMT_CALL:
			pStmt->m_sCallProc.ToUpper();
			if ( pStmt->m_sCallProc=="SNIPPETS" )
				HandleMysqlCallSnippets ( tOut, uPacketID, *pStmt );
			else if ( pStmt->m_sCallProc=="KEYWORDS" )
				HandleMysqlCallKeywords ( tOut, uPacketID, *pStmt );
			else
			{
				sError.SetSprintf ( "no such builtin procedure %s", pStmt->m_sCallProc.cstr() );
				SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
			}
			continue;

		case STMT_DESC:
			HandleMysqlDescribe ( tOut, uPacketID, *pStmt );
			continue;

		case STMT_SHOW_TABLES:
			HandleMysqlShowTables ( tOut, uPacketID );
			continue;

		case STMT_UPDATE:
			HandleMysqlUpdate ( tOut, uPacketID, *pStmt, bAutoCommit && !bInTransaction );
			continue;

		default:
			sError.SetSprintf ( "internal error: unhandled statement type (value=%d)", eStmt );
			SendMysqlErrorPacket ( tOut, uPacketID, sError.cstr() );
			break;
		} // switch
	} // for ( ;; )

	// set off query guard
	SphCrashLogger_c::SetLastQuery ( NULL, 0, true );

	SafeClose ( iPipeFD );
}


//////////////////////////////////////////////////////////////////////////
// HANDLE-BY-LISTENER
//////////////////////////////////////////////////////////////////////////

void HandleClient ( ProtocolType_e eProto, int iSock, const char * sClientIP, int iPipeFD, ThdDesc_t * pThd )
{
	switch ( eProto )
	{
		case PROTO_SPHINX:		HandleClientSphinx ( iSock, sClientIP, iPipeFD, pThd ); break;
		case PROTO_MYSQL41:		HandleClientMySQL ( iSock, sClientIP, iPipeFD, pThd ); break;
		default:				assert ( 0 && "unhandled protocol type" ); break;
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

#if USE_WINDOWS
	::unlink ( sTo );
#endif

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


bool HasFiles ( const ServedIndex_t & tIndex, const char ** dExts )
{
	char sFile [ SPH_MAX_FILENAME_LEN ];
	const char * sPath = tIndex.m_sIndexPath.cstr();

	for ( int i=0; i<EXT_COUNT; i++ )
	{
		snprintf ( sFile, sizeof(sFile), "%s%s", sPath, dExts[i] );
		if ( !sphIsReadable ( sFile ) )
			return false;
	}

	return true;
}

/// returns true if any version of the index (old or new one) has been preread
bool RotateIndexGreedy ( ServedIndex_t & tIndex, const char * sIndex )
{
	sphLogDebug ( "RotateIndexGreedy for '%s' invoked", sIndex );
	char sFile [ SPH_MAX_FILENAME_LEN ];
	const char * sPath = tIndex.m_sIndexPath.cstr();


	for ( int i=0; i<EXT_COUNT; i++ )
	{
		snprintf ( sFile, sizeof(sFile), "%s%s", sPath, g_dNewExts[i] );
		if ( !sphIsReadable ( sFile ) )
		{
			if ( i>0 )
			{
				if ( tIndex.m_bOnlyNew )
					sphWarning ( "rotating index '%s': '%s' unreadable: %s; NOT SERVING", sIndex, sFile, strerror(errno) );
				else
					sphWarning ( "rotating index '%s': '%s' unreadable: %s; using old index", sIndex, sFile, strerror(errno) );
			}
			return false;
		}
	}
	sphLogDebug ( "RotateIndexGreedy: new index is readable" );

	if ( !tIndex.m_bOnlyNew )
	{
		// rename current to old
		for ( int i=0; i<EXT_COUNT; i++ )
		{
			if ( TryRename ( sIndex, sPath, g_dCurExts[i], g_dOldExts[i], false ) )
				continue;

			// rollback
			for ( int j=0; j<i; j++ )
				TryRename ( sIndex, sPath, g_dOldExts[j], g_dCurExts[j], true );

			sphWarning ( "rotating index '%s': rename to .old failed; using old index", sIndex );
			return false;
		}

		// holding the persistent MVA updates (.mvp).
		for ( ;; )
		{
			char sBuf [ SPH_MAX_FILENAME_LEN ];
			snprintf ( sBuf, sizeof(sBuf), "%s%s", sPath, g_dCurExts[EXT_MVP] );

			CSphString sFakeError;
			CSphAutofile fdTest ( sBuf, SPH_O_READ, sFakeError );
			if ( fdTest.GetFD()<0 )
				break; ///< no file, nothing to hold

			if ( TryRename ( sIndex, sPath, g_dCurExts[EXT_MVP], g_dOldExts[EXT_MVP], false ) )
				break;

			// rollback
			for ( int j=0; j<EXT_COUNT; j++ )
				TryRename ( sIndex, sPath, g_dOldExts[j], g_dCurExts[j], true );

			break;
		}
		sphLogDebug ( "RotateIndexGreedy: Current index renamed to .old" );
	}

	// rename new to current
	for ( int i=0; i<EXT_COUNT; i++ )
	{
		if ( TryRename ( sIndex, sPath, g_dNewExts[i], g_dCurExts[i], false ) )
			continue;

		// rollback new ones we already renamed
		for ( int j=0; j<i; j++ )
			TryRename ( sIndex, sPath, g_dCurExts[j], g_dNewExts[j], true );

		// rollback old ones
		if ( !tIndex.m_bOnlyNew )
			for ( int j=0; j<=EXT_COUNT; j++ ) ///< <=, not <, since we have the .mvp at the end of these lists
				TryRename ( sIndex, sPath, g_dOldExts[j], g_dCurExts[j], true );

		return false;
	}
	sphLogDebug ( "RotateIndexGreedy: New renamed to current" );

	bool bPreread = false;

	// try to use new index
	CSphString sWarning;
	ISphTokenizer * pTokenizer = tIndex.m_pIndex->LeakTokenizer ();
	CSphDict * pDictionary = tIndex.m_pIndex->LeakDictionary ();

	if ( !tIndex.m_pIndex->Prealloc ( tIndex.m_bMlock, false, sWarning ) || !tIndex.m_pIndex->Preread() )
	{
		if ( tIndex.m_bOnlyNew )
		{
			sphWarning ( "rotating index '%s': .new preload failed: %s; NOT SERVING", sIndex, tIndex.m_pIndex->GetLastError().cstr() );
			return false;

		} else
		{
			sphWarning ( "rotating index '%s': .new preload failed: %s", sIndex, tIndex.m_pIndex->GetLastError().cstr() );

			// try to recover
			for ( int j=0; j<EXT_COUNT; j++ )
			{
				TryRename ( sIndex, sPath, g_dCurExts[j], g_dNewExts[j], true );
				TryRename ( sIndex, sPath, g_dOldExts[j], g_dCurExts[j], true );
			}
			TryRename ( sIndex, sPath, g_dOldExts[EXT_MVP], g_dCurExts[EXT_MVP], true );
			sphLogDebug ( "RotateIndexGreedy: has recovered" );

			if ( !tIndex.m_pIndex->Prealloc ( tIndex.m_bMlock, false, sWarning ) || !tIndex.m_pIndex->Preread() )
			{
				sphWarning ( "rotating index '%s': .new preload failed; ROLLBACK FAILED; INDEX UNUSABLE", sIndex );
				tIndex.m_bEnabled = false;

			} else
			{
				tIndex.m_bEnabled = true;
				bPreread = true;
				sphWarning ( "rotating index '%s': .new preload failed; using old index", sIndex );
			}

			if ( !sWarning.IsEmpty() )
				sphWarning ( "rotating index '%s': %s", sIndex, sWarning.cstr() );

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

		if ( !sWarning.IsEmpty() )
			sphWarning ( "rotating index '%s': %s", sIndex, sWarning.cstr() );
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
	if ( g_bUnlinkOld && !tIndex.m_bOnlyNew )
	{
		for ( int i=0; i<EXT_COUNT; i++ )
		{
			snprintf ( sFile, sizeof(sFile), "%s%s", sPath, g_dOldExts[i] );
			if ( ::unlink ( sFile ) )
				sphWarning ( "rotating index '%s': unable to unlink '%s': %s", sIndex, sFile, strerror(errno) );
		}
		snprintf ( sFile, sizeof(sFile), "%s%s", sPath, g_dOldExts[EXT_MVP] );
		::unlink ( sFile );
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

#if USE_WINDOWS

int CreatePipe ( bool, int )	{ return -1; }
int PipeAndFork ( bool, int )	{ return -1; }

#else

// open new pipe to be able to receive notifications from children
// adds read-end fd to g_dPipes; returns write-end fd for child
int CreatePipe ( bool bFatal, int iHandler )
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

		PipeInfo_t tAdd;
		tAdd.m_iFD = dPipe[0];
		tAdd.m_iHandler = iHandler;
		g_dPipes.Add ( tAdd );
		break;
	}

	return dPipe[1];
}


/// create new worker child
/// creates a pipe to it, forks, and does some post-fork work
//
/// in child, returns write-end pipe fd (might be -1!) and sets g_bHeadDaemon to false
/// in parent, returns -1 and leaves g_bHeadDaemon unaffected
int PipeAndFork ( bool bFatal, int iHandler )
{
	int iChildPipe = CreatePipe ( bFatal, iHandler );
	int iFork = fork();
	switch ( iFork )
	{
		// fork() failed
		case -1:
			sphFatal ( "fork() failed (reason: %s)", strerror(errno) );

		// child process, handle client
		case 0:
			g_bHeadDaemon = false;
			sphSetProcessInfo ( false );
			ARRAY_FOREACH ( i, g_dPipes )
				SafeClose ( g_dPipes[i].m_iFD );
			break;

		// parent process, continue accept()ing
		default:
			g_iChildren++;
			g_dChildren.Add ( iFork );
			SafeClose ( iChildPipe );
			break;
	}
	return iChildPipe;
}

#endif // !USE_WINDOWS

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

	if ( g_dThd.GetLength()==0 && iHeadAllocs!=sphAllocsCount() )
	{
		lseek ( g_iLogFile, 0, SEEK_END );
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

bool CheckIndex ( const CSphIndex * pIndex, CSphString & sError )
{
	const CSphIndexSettings & tSettings = pIndex->GetSettings ();

	if ( ( tSettings.m_iMinPrefixLen>0 || tSettings.m_iMinInfixLen>0 ) && !pIndex->m_bEnableStar )
	{
		CSphDict * pDict = pIndex->GetDictionary ();
		assert ( pDict );
		if ( pDict->GetSettings ().HasMorphology () )
		{
			sError = "infixes and morphology are enabled, enable_star=0";
			return false;
		}
	}

	return true;
}


static bool CheckServedEntry ( const ServedIndex_t * pEntry, const char * sIndex )
{
	if ( !pEntry )
	{
		sphWarning ( "rotating index '%s': INTERNAL ERROR, index went AWOL", sIndex );
		return false;
	}

	if ( pEntry->m_bToDelete || !pEntry->m_pIndex )
	{
		if ( pEntry->m_bToDelete )
			sphWarning ( "rotating index '%s': INTERNAL ERROR, entry marked for deletion", sIndex );

		if ( !pEntry->m_pIndex )
			sphWarning ( "rotating index '%s': INTERNAL ERROR, entry does not have an index", sIndex );

		return false;
	}

	return true;
}

#define SPH_RT_AUTO_FLUSH_CHECK_PERIOD ( 5000000 )

static void RtFlushThreadFunc ( void * )
{
	int64_t tmNextCheck = sphMicroTimer() + SPH_RT_AUTO_FLUSH_CHECK_PERIOD;
	while ( !g_bRtFlushShutdown )
	{
		// stand still till save time
		if ( tmNextCheck>sphMicroTimer() )
		{
			sphSleepMsec ( 50 );
			continue;
		}

		// collecting available rt indexes at save time
		CSphVector<CSphString> dRtIndexes;
		for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
			if ( it.Get().m_bRT )
				dRtIndexes.Add ( it.GetKey() );

		// do check+save
		ARRAY_FOREACH_COND ( i, dRtIndexes, !g_bRtFlushShutdown )
		{
			const ServedIndex_t * pServed = g_pIndexes->GetRlockedEntry ( dRtIndexes[i] );
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


static void RotateIndexMT ( const CSphString & sIndex )
{
	//////////////////
	// load new index
	//////////////////

	// create new index, copy some settings from existing one
	const ServedIndex_t * pRotating = g_pIndexes->GetRlockedEntry ( sIndex );
	if ( !CheckServedEntry ( pRotating, sIndex.cstr() ) )
	{
		if ( pRotating )
			pRotating->Unlock();
		return;
	}

	sphInfo ( "rotating index '%s': started", sIndex.cstr() );

	ServedIndex_t tNewIndex;
	tNewIndex.m_bOnlyNew = pRotating->m_bOnlyNew;

	tNewIndex.m_pIndex = sphCreateIndexPhrase ( NULL, NULL ); // FIXME! check if it's ok
	tNewIndex.m_pIndex->m_bEnableStar = pRotating->m_bStar;
	tNewIndex.m_pIndex->m_bExpandKeywords = pRotating->m_bExpand;
	tNewIndex.m_pIndex->SetPreopen ( pRotating->m_bPreopen || g_bPreopenIndexes );
	tNewIndex.m_pIndex->SetWordlistPreload ( !pRotating->m_bOnDiskDict && !g_bOnDiskDicts );

	// rebase new index
	char sNewPath [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sNewPath, sizeof(sNewPath), "%s.new", pRotating->m_sIndexPath.cstr() );
	tNewIndex.m_pIndex->SetBase ( sNewPath );

	// don't need to hold the existing index any more now
	pRotating->Unlock();
	pRotating = NULL;

	// prealloc enough RAM and lock new index
	sphLogDebug ( "prealloc enough RAM and lock new index" );
	CSphString sWarn, sError;
	if ( !tNewIndex.m_pIndex->Prealloc ( tNewIndex.m_bMlock, false, sWarn ) )
	{
		sphWarning ( "rotating index '%s': prealloc: %s; using old index", sIndex.cstr(), tNewIndex.m_pIndex->GetLastError().cstr() );
		return;
	}

	if ( !tNewIndex.m_pIndex->Lock() )
	{
		sphWarning ( "rotating index '%s': lock: %s; using old index", sIndex.cstr (), tNewIndex.m_pIndex->GetLastError().cstr() );
		return;
	}

	// fixup settings if needed
	sphLogDebug ( "fixup settings if needed" );
	g_tRotateConfigMutex.Lock ();
	if ( tNewIndex.m_bOnlyNew && g_pCfg && g_pCfg->m_tConf ( "index" ) && g_pCfg->m_tConf["index"]( sIndex.cstr() ) )
	{
		if ( !sphFixupIndexSettings ( tNewIndex.m_pIndex, g_pCfg->m_tConf["index"][sIndex.cstr()], sError ) )
		{
			sphWarning ( "rotating index '%s': fixup: %s; using old index", sIndex.cstr(), sError.cstr() );
			g_tRotateConfigMutex.Unlock ();
			return;
		}
	}
	g_tRotateConfigMutex.Unlock();

	if ( !CheckIndex ( tNewIndex.m_pIndex, sError ) )
	{
		sphWarning ( "rotating index '%s': check: %s; using old index", sIndex.cstr(), sError.cstr() );
		return;
	}

	if ( !tNewIndex.m_pIndex->Preread() )
	{
		sphWarning ( "rotating index '%s': preread failed: %s; using old index", sIndex.cstr(), tNewIndex.m_pIndex->GetLastError().cstr() );
		return;
	}

	//////////////////////
	// activate new index
	//////////////////////

	sphLogDebug ( "activate new index" );

	ServedIndex_t * pServed = g_pIndexes->GetWlockedEntry ( sIndex );
	if ( !CheckServedEntry ( pServed, sIndex.cstr() ) )
	{
		if ( pServed )
			pServed->Unlock();
		return;
	}

	CSphIndex * pOld = pServed->m_pIndex;
	CSphIndex * pNew = tNewIndex.m_pIndex;

	// rename files
	// FIXME! factor out a common function w/ non-threaded rotation code
	char sOld [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sOld, sizeof(sOld), "%s.old", pServed->m_sIndexPath.cstr() );

	if ( !pServed->m_bOnlyNew && !pOld->Rename ( sOld ) )
	{
		// FIXME! rollback inside Rename() call potentially fail
		sphWarning ( "rotating index '%s': cur to old rename failed: %s", sIndex.cstr(), pOld->GetLastError().cstr() );

	} else
	{
		// FIXME! at this point there's no cur lock file; ie. potential race
		sphLogDebug ( "no cur lock file; ie. potential race" );
		if ( !pNew->Rename ( pServed->m_sIndexPath.cstr() ) )
		{
			sphWarning ( "rotating index '%s': new to cur rename failed: %s", sIndex.cstr(), pNew->GetLastError().cstr() );
			if ( !pServed->m_bOnlyNew && !pOld->Rename ( pServed->m_sIndexPath.cstr() ) )
			{
				sphWarning ( "rotating index '%s': old to cur rename failed: %s; INDEX UNUSABLE", sIndex.cstr(), pOld->GetLastError().cstr() );
				pServed->m_bEnabled = false;
			}
		} else
		{
			// all went fine; swap them
			sphLogDebug ( "all went fine; swap them" );
			if ( !tNewIndex.m_pIndex->GetTokenizer() )
				tNewIndex.m_pIndex->SetTokenizer ( pServed->m_pIndex->LeakTokenizer() );

			if ( !tNewIndex.m_pIndex->GetDictionary() )
				tNewIndex.m_pIndex->SetDictionary ( pServed->m_pIndex->LeakDictionary() );

			Swap ( pServed->m_pIndex, tNewIndex.m_pIndex );
			pServed->m_bEnabled = true;

			// unlink .old
			sphLogDebug ( "unlink .old" );
			if ( g_bUnlinkOld && !pServed->m_bOnlyNew )
			{
				char sFile [ SPH_MAX_FILENAME_LEN ];

				for ( int i=0; i<EXT_COUNT; i++ )
				{
					snprintf ( sFile, sizeof(sFile), "%s%s", sOld, g_dCurExts[i] );
					if ( ::unlink ( sFile ) )
						sphWarning ( "rotating index '%s': unable to unlink '%s': %s", sIndex.cstr(), sFile, strerror(errno) );
				}
			}

			pServed->m_bOnlyNew = false;
			sphInfo ( "rotating index '%s': success", sIndex.cstr() );
		}
	}

	pServed->Unlock();
}

void RotationThreadFunc ( void * )
{
	while ( !g_bRotateShutdown )
	{
		// check if we have work to do
		g_tRotateQueueMutex.Lock();
		if (!( g_bDoRotate && g_dRotateQueue.GetLength() ))
		{
			g_tRotateQueueMutex.Unlock();
			sphSleepMsec ( 50 );
			continue;
		}

		CSphString sIndex = g_dRotateQueue.Pop();
		g_sPrereading = sIndex.cstr();
		g_tRotateQueueMutex.Unlock();

		RotateIndexMT ( sIndex );

		g_tRotateQueueMutex.Lock();
		if ( !g_dRotateQueue.GetLength() )
		{
			g_bDoRotate = false;
			sphInfo ( "rotating index: all indexes done" );
		}
		g_sPrereading = NULL;
		g_tRotateQueueMutex.Unlock();
	}
}


void IndexRotationDone ()
{
#if !USE_WINDOWS
	// forcibly restart children serving persistent connections and/or preforked ones
	// FIXME! check how both signals are handled in both cases
	int iSignal = ( g_eWorkers==MPM_PREFORK ) ? SIGTERM : SIGHUP;
	ARRAY_FOREACH ( i, g_dChildren )
		kill ( g_dChildren[i], iSignal );
#endif

	g_bDoRotate = false;
	sphInfo ( "rotating finished" );
}


void SeamlessTryToForkPrereader ()
{
	sphLogDebug ( "Invoked SeamlessTryToForkPrereader" );

	// next in line
	const char * sPrereading = g_dRotating.Pop ();
	if ( !sPrereading || !g_pIndexes->Exists ( sPrereading ) )
	{
		sphWarning ( "INTERNAL ERROR: preread attempt on unknown index '%s'", sPrereading ? sPrereading : "(NULL)" );
		return;
	}
	const ServedIndex_t & tServed = g_pIndexes->GetUnlockedEntry ( sPrereading );

	// alloc buffer index (once per run)
	if ( !g_pPrereading )
		g_pPrereading = sphCreateIndexPhrase ( NULL, NULL ); // FIXME! check if it's ok

	g_pPrereading->m_bEnableStar = tServed.m_bStar;
	g_pPrereading->m_bExpandKeywords = tServed.m_bExpand;
	g_pPrereading->SetPreopen ( tServed.m_bPreopen || g_bPreopenIndexes );
	g_pPrereading->SetWordlistPreload ( !tServed.m_bOnDiskDict && !g_bOnDiskDicts );

	// rebase buffer index
	char sNewPath [ SPH_MAX_FILENAME_LEN ];
	snprintf ( sNewPath, sizeof(sNewPath), "%s.new", tServed.m_sIndexPath.cstr() );
	g_pPrereading->SetBase ( sNewPath );

	// prealloc enough RAM and lock new index
	sphLogDebug ( "prealloc enough RAM and lock new index" );
	CSphString sWarn, sError;
	if ( !g_pPrereading->Prealloc ( tServed.m_bMlock, false, sWarn ) )
	{
		sphWarning ( "rotating index '%s': prealloc: %s; using old index", sPrereading, g_pPrereading->GetLastError().cstr() );
		if ( !sWarn.IsEmpty() )
			sphWarning ( "rotating index: %s", sWarn.cstr() );
		return;
	}
	if ( !sWarn.IsEmpty() )
		sphWarning ( "rotating index: %s: %s", sPrereading, sWarn.cstr() );

	if ( !g_pPrereading->Lock() )
	{
		sphWarning ( "rotating index '%s': lock: %s; using old index", sPrereading, g_pPrereading->GetLastError().cstr() );
		g_pPrereading->Dealloc ();
		return;
	}

	if ( tServed.m_bOnlyNew && g_pCfg && g_pCfg->m_tConf.Exists ( "index" ) && g_pCfg->m_tConf["index"].Exists ( sPrereading ) )
		if ( !sphFixupIndexSettings ( g_pPrereading, g_pCfg->m_tConf["index"][sPrereading], sError ) )
		{
			sphWarning ( "rotating index '%s': fixup: %s; using old index", sPrereading, sError.cstr() );
			return;
		}

	if ( !CheckIndex ( g_pPrereading, sError ) )
	{
		sphWarning ( "rotating index '%s': check: %s; using old index", sPrereading, sError.cstr() );
		return;
	}

	// fork async reader
	sphLogDebug ( "fork async reader" );
	g_sPrereading = sPrereading;
	int iPipeFD = PipeAndFork ( true, SPH_PIPE_PREREAD );

	// in parent, wait for prereader process to finish
	if ( g_bHeadDaemon )
		return;

	// in child, do preread
	bool bRes = g_pPrereading->Preread ();
	if ( !bRes )
		sphWarning ( "rotating index '%s': preread failed: %s; using old index", g_sPrereading, g_pPrereading->GetLastError().cstr() );
	// report and exit
	DWORD uTmp = SPH_PIPE_PREREAD;
	sphWrite ( iPipeFD, &uTmp, sizeof(DWORD) ); // FIXME? add buffering/checks?

	uTmp = bRes;
	sphWrite ( iPipeFD, &uTmp, sizeof(DWORD) );

	::close ( iPipeFD );
	sphLogDebug ( "SeamlessTryToForkPrereader: finishing the fork and invoking exit ( 0 )" );
	exit ( 0 );
}


void SeamlessForkPrereader ()
{
	sphLogDebug ( "Invoked SeamlessForkPrereader" );
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
		IndexRotationDone ();
}


/// simple wrapper to simplify reading from pipes
struct PipeReader_t
{
	explicit PipeReader_t ( int iFD )
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
		if ( !GetBytes ( const_cast<char*> ( sRes.cstr() ), iLen ) )
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

		for ( ;; )
		{
			int iRes = ::read ( m_iFD, pBuf, iCount );
			if ( iRes<0 && errno==EINTR )
				continue;

			if ( iRes!=iCount )
			{
				m_bError = true;
				sphWarning ( "pipe read failed (exp=%d, res=%d, error=%s)",
					iCount, iRes, iRes>0 ? "(none)" : strerror(errno) );
				return false;
			}
			return true;
		}
	}

protected:
	int			m_iFD;
	bool		m_bError;
};


/// handle pipe notifications from attribute updating
void HandlePipeUpdate ( PipeReader_t & tPipe, bool bFailure )
{
	if ( bFailure )
		return; // silently ignore errors

	++g_pFlush->m_iUpdateTag;

	int iUpdIndexes = tPipe.GetInt ();
	for ( int i=0; i<iUpdIndexes; i++ )
	{
		// index name and status must follow
		CSphString sIndex = tPipe.GetString ();
		DWORD uStatus = tPipe.GetInt ();
		if ( tPipe.IsError() )
			break;

		ServedIndex_t * pServed = g_pIndexes->GetWlockedEntry ( sIndex );
		if ( pServed )
		{
			pServed->m_iUpdateTag = g_pFlush->m_iUpdateTag;
			pServed->m_pIndex->m_uAttrsStatus |= uStatus;
			pServed->Unlock();
		} else
			sphWarning ( "INTERNAL ERROR: unknown index '%s' in HandlePipeUpdate()", sIndex.cstr() );
	}
}


/// handle pipe notifications from prereading
void HandlePipePreread ( PipeReader_t & tPipe, bool bFailure )
{
	if ( bFailure )
	{
		// clean up previous one and launch next one
		g_sPrereading = NULL;

		// in any case, buffer index should now be deallocated
		g_pPrereading->Dealloc ();
		g_pPrereading->Unlock ();

		// work next one
		SeamlessForkPrereader ();
		return;
	}

	assert ( g_bDoRotate && g_bSeamlessRotate && g_sPrereading );

	// whatever the outcome, we will be done with this one
	const char * sPrereading = g_sPrereading;
	g_sPrereading = NULL;

	// notice that this will block!
	int iRes = tPipe.GetInt();
	if ( !tPipe.IsError() && iRes )
	{
		// if preread was succesful, exchange served index and prereader buffer index
		ServedIndex_t & tServed = g_pIndexes->GetUnlockedEntry ( sPrereading );
		CSphIndex * pOld = tServed.m_pIndex;
		CSphIndex * pNew = g_pPrereading;

		char sOld [ SPH_MAX_FILENAME_LEN ];
		snprintf ( sOld, sizeof(sOld), "%s.old", tServed.m_sIndexPath.cstr() );

		if ( !tServed.m_bOnlyNew && !pOld->Rename ( sOld ) )
		{
			// FIXME! rollback inside Rename() call potentially fail
			sphWarning ( "rotating index '%s': cur to old rename failed: %s", sPrereading, pOld->GetLastError().cstr() );

		} else
		{
			// FIXME! at this point there's no cur lock file; ie. potential race
			if ( !pNew->Rename ( tServed.m_sIndexPath.cstr() ) )
			{
				sphWarning ( "rotating index '%s': new to cur rename failed: %s", sPrereading, pNew->GetLastError().cstr() );
				if ( !tServed.m_bOnlyNew && !pOld->Rename ( tServed.m_sIndexPath.cstr() ) )
				{
					sphWarning ( "rotating index '%s': old to cur rename failed: %s; INDEX UNUSABLE", sPrereading, pOld->GetLastError().cstr() );
					tServed.m_bEnabled = false;
				}
			} else
			{
				// all went fine; swap them
				if ( !g_pPrereading->GetTokenizer () )
					g_pPrereading->SetTokenizer ( tServed.m_pIndex->LeakTokenizer () );

				if ( !g_pPrereading->GetDictionary () )
					g_pPrereading->SetDictionary ( tServed.m_pIndex->LeakDictionary () );

				Swap ( tServed.m_pIndex, g_pPrereading );
				tServed.m_bEnabled = true;

				// unlink .old
				if ( g_bUnlinkOld && !tServed.m_bOnlyNew )
				{
					char sFile [ SPH_MAX_FILENAME_LEN ];

					for ( int i=0; i<EXT_COUNT; i++ )
					{
						snprintf ( sFile, sizeof(sFile), "%s%s", sOld, g_dCurExts[i] );
						if ( ::unlink ( sFile ) )
							sphWarning ( "rotating index '%s': unable to unlink '%s': %s", sPrereading, sFile, strerror(errno) );
					}
				}

				tServed.m_bOnlyNew = false;
				sphInfo ( "rotating index '%s': success", sPrereading );
			}
		}

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
}


/// handle pipe notifications from attribute saving
void HandlePipeSave ( PipeReader_t & tPipe, bool bFailure )
{
	// in any case, we're no more flushing
	g_pFlush->m_bFlushing = false;

	// silently ignore errors
	if ( bFailure )
		return;

	// handle response
	int iSavedIndexes = tPipe.GetInt ();
	for ( int i=0; i<iSavedIndexes; i++ )
	{
		// index name must follow
		CSphString sIndex = tPipe.GetString ();
		if ( tPipe.IsError() )
			break;

		const ServedIndex_t * pServed = g_pIndexes->GetWlockedEntry ( sIndex );
		if ( pServed )
		{
			if ( pServed->m_iUpdateTag<=g_pFlush->m_iFlushTag )
				pServed->m_pIndex->m_uAttrsStatus = 0;

			pServed->Unlock();
		} else
			sphWarning ( "INTERNAL ERROR: unknown index '%s' in HandlePipeSave()", sIndex.cstr() );
	}
}


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
		int iHandler = g_dPipes[i].m_iHandler;
		g_dPipes.Remove ( i-- );

		// check for eof/error
		bool bFailure = false;
		if ( iRes!=sizeof(DWORD) )
		{
			bFailure = true;

			if ( iHandler<0 )
				continue; // no handler; we're not expecting anything

			if ( iRes!=0 || iHandler>=0 )
				sphWarning ( "pipe status read failed (handler=%d)", iHandler );
		}

		// check for handler/status mismatch
		if ( !bFailure && ( iHandler>=0 && (int)uStatus!=iHandler ) )
		{
			bFailure = true;
			sphWarning ( "INTERNAL ERROR: pipe status mismatch (handler=%d, status=%d)", iHandler, uStatus );
		}

		// check for handler promotion (ie: we did not expect anything particular, but something happened anyway)
		if ( !bFailure && iHandler<0 )
			iHandler = (int)uStatus;

		// run the proper handler
		switch ( iHandler )
		{
			case SPH_PIPE_UPDATED_ATTRS:	HandlePipeUpdate ( tPipe, bFailure ); break;
			case SPH_PIPE_SAVED_ATTRS:		HandlePipeSave ( tPipe, bFailure ); break;
			case SPH_PIPE_PREREAD:			HandlePipePreread ( tPipe, bFailure ); break;
			default:						if ( !bFailure ) sphWarning ( "INTERNAL ERROR: unknown pipe handler (handler=%d, status=%d)", iHandler, uStatus ); break;
		}
	}
}


void ConfigureIndex ( ServedIndex_t & tIdx, const CSphConfigSection & hIndex )
{
	tIdx.m_bMlock = ( hIndex.GetInt ( "mlock", 0 )!=0 ) && !g_bOptNoLock;
	tIdx.m_bStar = ( hIndex.GetInt ( "enable_star", 0 )!=0 );
	tIdx.m_bExpand = ( hIndex.GetInt ( "expand_keywords", 0 )!=0 );
	tIdx.m_bPreopen = ( hIndex.GetInt ( "preopen", 0 )!=0 );
	tIdx.m_bOnDiskDict = ( hIndex.GetInt ( "ondisk_dict", 0 )!=0 );
}


bool PrereadNewIndex ( ServedIndex_t & tIdx, const CSphConfigSection & hIndex, const char * szIndexName )
{
	CSphString sWarning;

	if ( !tIdx.m_pIndex->Prealloc ( tIdx.m_bMlock, false, sWarning ) || !tIdx.m_pIndex->Preread() )
	{
		sphWarning ( "index '%s': preload: %s; NOT SERVING", szIndexName, tIdx.m_pIndex->GetLastError().cstr() );
		return false;
	}

	if ( !sWarning.IsEmpty() )
		sphWarning ( "index '%s': %s", szIndexName, sWarning.cstr() );

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

	return true;
}


bool ConfigureAgent ( AgentDesc_t & tAgent, const CSphVariant * pAgent, const char * szIndexName, bool bBlackhole )
{
	// extract host name or path
	const char * p = pAgent->cstr();
	while ( sphIsAlpha(*p) || *p=='.' || *p=='-' || *p=='/' ) p++;
	if ( p==pAgent->cstr() )
	{
		sphWarning ( "index '%s': agent '%s': host name or path expected - SKIPPING AGENT",
			szIndexName, pAgent->cstr() );
		return false;
	}
	if ( *p++!=':' )
	{
		sphWarning ( "index '%s': agent '%s': colon expected near '%s' - SKIPPING AGENT",
			szIndexName, pAgent->cstr(), p );
		return false;
	}

	CSphString sSub = pAgent->SubString ( 0, p-1-pAgent->cstr() );
	if ( sSub.cstr()[0]=='/' )
	{
#if USE_WINDOWS
		sphWarning ( "index '%s': agent '%s': UNIX sockets are not supported on Windows - SKIPPING AGENT",
			szIndexName, pAgent->cstr() );
		return false;
#else
		if ( strlen ( sSub.cstr() ) + 1 > sizeof(((struct sockaddr_un *)0)->sun_path) )
		{
			sphWarning ( "index '%s': agent '%s': UNIX socket path is too long - SKIPPING AGENT",
				szIndexName, pAgent->cstr() );
			return false;
		}

		tAgent.m_iFamily = AF_UNIX;
		tAgent.m_sPath = sSub;
		p--;
#endif
	} else
	{
		tAgent.m_iFamily = AF_INET;
		tAgent.m_sHost = sSub;

		// extract port
		if ( !isdigit(*p) )
		{
			sphWarning ( "index '%s': agent '%s': port number expected near '%s' - SKIPPING AGENT",
				szIndexName, pAgent->cstr(), p );
			return false;
		}
		tAgent.m_iPort = atoi(p);

		if ( !IsPortInRange ( tAgent.m_iPort ) )
		{
			sphWarning ( "index '%s': agent '%s': invalid port number near '%s' - SKIPPING AGENT",
				szIndexName, pAgent->cstr(), p );
			return false;
		}

		while ( isdigit(*p) ) p++;
	}

	// extract index list
	if ( *p++!=':' )
	{
		sphWarning ( "index '%s': agent '%s': colon expected near '%s' - SKIPPING AGENT",
			szIndexName, pAgent->cstr(), p );
		return false;
	}
	while ( isspace(*p) )
		p++;
	const char * sIndexList = p;
	while ( sphIsAlpha(*p) || isspace(*p) || *p==',' )
		p++;
	if ( *p )
	{
		sphWarning ( "index '%s': agent '%s': index list expected near '%s' - SKIPPING AGENT",
			szIndexName, pAgent->cstr(), p );
		return false;
	}
	tAgent.m_sIndexes = sIndexList;

	// lookup address (if needed)
	if ( tAgent.m_iFamily==AF_INET )
	{
		tAgent.m_uAddr = sphGetAddress ( tAgent.m_sHost.cstr() );
		if ( tAgent.m_uAddr==0 )
		{
			sphWarning ( "index '%s': agent '%s': failed to lookup host name '%s' (error=%s) - SKIPPING AGENT",
				szIndexName, pAgent->cstr(), tAgent.m_sHost.cstr(), sphSockError() );
			return false;
		}
	}

	tAgent.m_bBlackhole = bBlackhole;

	// allocate stats slot
	if ( g_pStats )
	{
		g_tStatsMutex.Lock();
		for ( int i=0; i<STATS_MAX_AGENTS/32; i++ )
			if ( g_pStats->m_bmAgentStats[i]!=0xffffffffUL )
		{
			int j = FindBit ( g_pStats->m_bmAgentStats[i] );
			g_pStats->m_bmAgentStats[i] |= ( 1<<j );
			tAgent.m_iStatsIndex = i*32 + j;
			memset ( &g_pStats->m_dAgentStats[tAgent.m_iStatsIndex], 0, sizeof(AgentStats_t) );
			break;
		}
		g_tStatsMutex.Unlock();
	}

	return true;
}

static DistributedIndex_t ConfigureDistributedIndex ( const char * szIndexName, const CSphConfigSection & hIndex )
{
	assert ( hIndex("type") && hIndex["type"]=="distributed" );

	DistributedIndex_t tIdx;

	// add local agents
	for ( CSphVariant * pLocal = hIndex("local"); pLocal; pLocal = pLocal->m_pNext )
	{
		if ( !g_pIndexes->Exists ( pLocal->cstr() ) )
		{
			sphWarning ( "index '%s': no such local index '%s' - SKIPPING LOCAL INDEX",
				szIndexName, pLocal->cstr() );
			continue;
		}
		tIdx.m_dLocal.Add ( pLocal->cstr() );
	}

	// add remote agents
	for ( CSphVariant * pAgent = hIndex("agent"); pAgent; pAgent = pAgent->m_pNext )
	{
		AgentDesc_t tAgent;
		if ( ConfigureAgent ( tAgent, pAgent, szIndexName, false ) )
			tIdx.m_dAgents.Add ( tAgent );
	}

	for ( CSphVariant * pAgent = hIndex("agent_blackhole"); pAgent; pAgent = pAgent->m_pNext )
	{
		AgentDesc_t tAgent;
		if ( ConfigureAgent ( tAgent, pAgent, szIndexName, true ) )
			tIdx.m_dAgents.Add ( tAgent );
	}

	// configure options
	if ( hIndex("agent_connect_timeout") )
	{
		if ( hIndex["agent_connect_timeout"].intval()<=0 )
			sphWarning ( "index '%s': connect_timeout must be positive, ignored", szIndexName );
		else
			tIdx.m_iAgentConnectTimeout = hIndex["agent_connect_timeout"].intval();
	}

	if ( hIndex("agent_query_timeout") )
	{
		if ( hIndex["agent_query_timeout"].intval()<=0 )
			sphWarning ( "index '%s': query_timeout must be positive, ignored", szIndexName );
		else
			tIdx.m_iAgentQueryTimeout = hIndex["agent_query_timeout"].intval();
	}

	return tIdx;
}


void FreeAgentStats ( DistributedIndex_t & tIndex )
{
	if ( !g_pStats )
		return;

	g_tStatsMutex.Lock();
	ARRAY_FOREACH ( i, tIndex.m_dAgents )
	{
		int iIndex = tIndex.m_dAgents[i].m_iStatsIndex;
		if ( iIndex<0 || iIndex>=STATS_MAX_AGENTS )
			continue;

		assert ( g_pStats->m_bmAgentStats[iIndex>>5] & ( 1UL<<(iIndex&31) ) );
		g_pStats->m_bmAgentStats[iIndex>>5] &= ~( 1UL<<(iIndex&31) );
	}
	g_tStatsMutex.Unlock();
}


ESphAddIndex AddIndex ( const char * szIndexName, const CSphConfigSection & hIndex )
{
	if ( hIndex("type") && hIndex["type"]=="distributed" )
	{
		///////////////////////////////
		// configure distributed index
		///////////////////////////////

		DistributedIndex_t tIdx = ConfigureDistributedIndex ( szIndexName, hIndex );

		// finally, check and add distributed index to global table
		if ( tIdx.m_dAgents.GetLength()==0 && tIdx.m_dLocal.GetLength()==0 )
		{
			FreeAgentStats ( tIdx );
			sphWarning ( "index '%s': no valid local/remote indexes in distributed index - NOT SERVING", szIndexName );
			return ADD_ERROR;

		} else
		{
			g_tDistLock.Lock ();
			if ( !g_hDistIndexes.Add ( tIdx, szIndexName ) )
			{
				g_tDistLock.Unlock ();
				FreeAgentStats ( tIdx );
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

		if ( g_eWorkers!=MPM_THREADS )
		{
			sphWarning ( "index '%s': RT index requires workers=threads - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		CSphSchema tSchema ( szIndexName );
		CSphColumnInfo tCol;

		// fields
		for ( CSphVariant * v=hIndex("rt_field"); v; v=v->m_pNext )
		{
			tCol.m_sName = v->cstr();
			tSchema.m_dFields.Add ( tCol );
		}
		if ( !tSchema.m_dFields.GetLength() )
		{
			sphWarning ( "index '%s': no fields configured (use rt_field directive) - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		if ( tSchema.m_dFields.GetLength()>SPH_MAX_FIELDS )
		{
			sphWarning ( "index '%s': too many fields (fields=%d, max=%d) - NOT SERVING", szIndexName, tSchema.m_dFields.GetLength(), SPH_MAX_FIELDS );
			return ADD_ERROR;
		}

		// attrs
		const int iNumTypes = 5;
		const char * sTypes[iNumTypes] = { "rt_attr_uint", "rt_attr_bigint", "rt_attr_float", "rt_attr_timestamp", "rt_attr_string" };
		const int iTypes[iNumTypes] = { SPH_ATTR_INTEGER, SPH_ATTR_BIGINT, SPH_ATTR_FLOAT, SPH_ATTR_TIMESTAMP, SPH_ATTR_STRING };

		for ( int iType=0; iType<iNumTypes; iType++ )
		{
			for ( CSphVariant * v = hIndex ( sTypes[iType] ); v; v = v->m_pNext )
			{
				tCol.m_sName = v->cstr();
				tCol.m_eAttrType = iTypes[iType];
				tSchema.AddAttr ( tCol, false );
			}
		}

		// path
		if ( !hIndex("path") )
		{
			sphWarning ( "index '%s': path must be specified - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		// RAM chunk size
		DWORD uRamSize = hIndex.GetSize ( "rt_mem_limit", 32*1024*1024 );
		if ( uRamSize<128*1024 )
		{
			sphWarning ( "index '%s': rt_mem_limit extremely low, using 128K instead", szIndexName );
			uRamSize = 128*1024;
		} else if ( uRamSize<8*1024*1024 )
			sphWarning ( "index '%s': rt_mem_limit very low (under 8 MB)", szIndexName );

		// index
		ServedIndex_t tIdx;
		tIdx.m_pIndex = sphCreateIndexRT ( tSchema, szIndexName, uRamSize, hIndex["path"].cstr() );
		tIdx.m_bEnabled = false;
		tIdx.m_sIndexPath = hIndex["path"];
		tIdx.m_bRT = true;

		// pick config settings
		// they should be overriden later by Preload() if needed
		CSphIndexSettings tSettings;
		sphConfIndex ( hIndex, tSettings );
		tIdx.m_pIndex->Setup ( tSettings );

		// hash it
		if ( !g_pIndexes->Add ( tIdx, szIndexName ) )
		{
			sphWarning ( "INTERNAL ERROR: index '%s': hash add failed - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		tIdx.Reset (); // so that the dtor wouln't delete everything
		return ADD_RT;

	} else if ( !hIndex("type") || hIndex["type"]=="plain" )
	{
		/////////////////////////
		// configure local index
		/////////////////////////

		ServedIndex_t tIdx;

		// check path
		if ( !hIndex.Exists ( "path" ) )
		{
			sphWarning ( "index '%s': key 'path' not found - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		// check name
		if ( g_pIndexes->Exists ( szIndexName ) )
		{
			sphWarning ( "index '%s': duplicate name - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		// configure memlocking, star
		ConfigureIndex ( tIdx, hIndex );

		// try to create index
		CSphString sWarning;
		tIdx.m_pIndex = sphCreateIndexPhrase ( szIndexName, hIndex["path"].cstr() );
		tIdx.m_pIndex->m_bEnableStar = tIdx.m_bStar;
		tIdx.m_pIndex->m_bExpandKeywords = tIdx.m_bExpand;
		tIdx.m_pIndex->m_iExpansionLimit = g_iExpansionLimit;
		tIdx.m_pIndex->SetPreopen ( tIdx.m_bPreopen || g_bPreopenIndexes );
		tIdx.m_pIndex->SetWordlistPreload ( !tIdx.m_bOnDiskDict && !g_bOnDiskDicts );
		tIdx.m_bEnabled = false;

		// done
		tIdx.m_sIndexPath = hIndex["path"];
		if ( !g_pIndexes->Add ( tIdx, szIndexName ) )
		{
			sphWarning ( "INTERNAL ERROR: index '%s': hash add failed - NOT SERVING", szIndexName );
			return ADD_ERROR;
		}

		tIdx.Reset (); // so that the dtor wouldn't delete everything
		return ADD_LOCAL;

	} else
	{
		// unknown type
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
	sphCalcFileCRC32 ( g_sConfigFile.cstr (), uCRC32 );

	if ( g_uCfgCRC32==uCRC32 && tStat.st_mtime==g_tCfgStat.st_mtime && tStat.st_ctime==g_tCfgStat.st_ctime && tStat.st_size==g_tCfgStat.st_size )
		return false;

	g_uCfgCRC32 = uCRC32;
	g_tCfgStat = tStat;

	return true;
}


void ReloadIndexSettings ( CSphConfigParser * pCP )
{
	assert ( pCP );

	if ( !pCP->Parse ( g_sConfigFile.cstr () ) )
	{
		sphWarning ( "failed to parse config file '%s'; using previous settings", g_sConfigFile.cstr () );
		return;
	}

	g_bDoDelete = false;

	for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
		it.Get().m_bToDelete = true;

	g_hDistIndexes.IterateStart ();
	while ( g_hDistIndexes.IterateNext () )
		g_hDistIndexes.IterateGet().m_bToDelete = true;

	int nTotalIndexes = g_pIndexes->GetLength () + g_hDistIndexes.GetLength ();
	int nChecked = 0;

	const CSphConfig & hConf = pCP->m_tConf;
	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext() )
	{
		const CSphConfigSection & hIndex = hConf["index"].IterateGet();
		const char * sIndexName = hConf["index"].IterateGetKey().cstr();

		ServedIndex_t * pServedIndex = g_pIndexes->GetWlockedEntry ( sIndexName );
		if ( pServedIndex )
		{
			ConfigureIndex ( *pServedIndex, hIndex );
			pServedIndex->m_bToDelete = false;
			nChecked++;
			pServedIndex->Unlock();

		} else if ( g_hDistIndexes.Exists ( sIndexName ) && hIndex.Exists("type") && hIndex["type"]=="distributed" )
		{
			DistributedIndex_t tIdx = ConfigureDistributedIndex ( sIndexName, hIndex );

			// finally, check and add distributed index to global table
			if ( tIdx.m_dAgents.GetLength()==0 && tIdx.m_dLocal.GetLength()==0 )
			{
				FreeAgentStats ( tIdx );
				sphWarning ( "index '%s': no valid local/remote indexes in distributed index; using last valid definition", sIndexName );
				g_hDistIndexes[sIndexName].m_bToDelete = false;

			} else
			{
				g_tDistLock.Lock();
				FreeAgentStats ( g_hDistIndexes[sIndexName] );
				g_hDistIndexes[sIndexName] = tIdx;
				g_tDistLock.Unlock();
			}

			nChecked++;

		} else if ( AddIndex ( sIndexName, hIndex )==ADD_LOCAL )
		{
			ServedIndex_t * pIndex = g_pIndexes->GetWlockedEntry ( sIndexName );
			if ( pIndex )
			{
				pIndex->m_bOnlyNew = true;
				pIndex->Unlock();
			}
		}
	}

	if ( nChecked < nTotalIndexes )
		g_bDoDelete = true;
}


void CheckDelete ()
{
	if ( !g_bDoDelete )
		return;

	if ( g_iChildren )
		return;

	CSphVector<const CSphString *> dToDelete;
	CSphVector<const CSphString *> dDistToDelete;
	dToDelete.Reserve ( 8 );
	dDistToDelete.Reserve ( 8 );

	for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
	{
		ServedIndex_t & tIndex = it.Get();
		if ( tIndex.m_bToDelete )
			dToDelete.Add ( &it.GetKey() );
	}

	g_hDistIndexes.IterateStart ();
	while ( g_hDistIndexes.IterateNext () )
	{
		DistributedIndex_t & tIndex = g_hDistIndexes.IterateGet ();
		if ( tIndex.m_bToDelete )
			dDistToDelete.Add ( &g_hDistIndexes.IterateGetKey () );
	}

	ARRAY_FOREACH ( i, dToDelete )
		g_pIndexes->Delete ( *dToDelete[i] ); // should result in automatic CSphIndex::Unlock() via dtor call

	g_tDistLock.Lock();

	ARRAY_FOREACH ( i, dDistToDelete )
	{
		FreeAgentStats ( g_hDistIndexes [ *dDistToDelete[i] ] );
		g_hDistIndexes.Delete ( *dDistToDelete[i] );
	}

	g_tDistLock.Unlock();

	g_bDoDelete = false;
}


void CheckRotate ()
{
	// do we need to rotate now?
	if ( !g_bDoRotate )
		return;

	sphLogDebug ( "CheckRotate invoked" );

	/////////////////////
	// RAM-greedy rotate
	/////////////////////

	if ( !g_bSeamlessRotate || g_eWorkers==MPM_PREFORK )
	{
		// wait until there's no running queries
		if ( g_iChildren && g_eWorkers!=MPM_PREFORK )
			return;

		CSphConfigParser * pCP = NULL;

		if ( CheckConfigChanges () )
		{
			pCP = new CSphConfigParser;
			ReloadIndexSettings ( pCP );
		}

		for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
		{
			ServedIndex_t & tIndex = it.Get();
			tIndex.WriteLock();
			const char * sIndex = it.GetKey().cstr();
			assert ( tIndex.m_pIndex );

			bool bWasAdded = tIndex.m_bOnlyNew;
			RotateIndexGreedy ( tIndex, sIndex );
			if ( bWasAdded && tIndex.m_bEnabled )
			{
				const CSphConfigType & hConf = pCP->m_tConf ["index"];
				if ( hConf.Exists ( sIndex ) )
				{
					CSphString sError;
					if ( !sphFixupIndexSettings ( tIndex.m_pIndex, hConf [sIndex], sError ) )
					{
						sphWarning ( "index '%s': %s - NOT SERVING", sIndex, sError.cstr() );
						tIndex.m_bEnabled = false;
					}

					if ( tIndex.m_bEnabled && !CheckIndex ( tIndex.m_pIndex, sError ) )
					{
						sphWarning ( "index '%s': %s - NOT SERVING", sIndex, sError.cstr() );
						tIndex.m_bEnabled = false;
					}
				}
			}
			tIndex.Unlock();
		}

		SafeDelete ( pCP );
		IndexRotationDone ();
		return;
	}

	///////////////////
	// seamless rotate
	///////////////////

	assert ( g_bDoRotate && g_bSeamlessRotate );
	if ( g_dRotating.GetLength() || g_dRotateQueue.GetLength() || g_sPrereading )
		return; // rotate in progress already; will be handled in CheckPipes()

	g_tRotateConfigMutex.Lock();
	SafeDelete ( g_pCfg );
	if ( CheckConfigChanges() )
	{
		g_pCfg = new CSphConfigParser;
		ReloadIndexSettings ( g_pCfg );
	}
	g_tRotateConfigMutex.Unlock();

	// check what indexes need to be rotated
	for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
	{
		const ServedIndex_t & tIndex = it.Get();
		const CSphString & sIndex = it.GetKey();
		assert ( tIndex.m_pIndex );

		CSphString sNewPath;
		sNewPath.SetSprintf ( "%s.new", tIndex.m_sIndexPath.cstr() );

		// check if there's a .new index incoming
		// FIXME? move this code to index, and also check for exists-but-not-readable
		CSphString sTmp;
		sTmp.SetSprintf ( "%s.sph", sNewPath.cstr() );
		if ( !sphIsReadable ( sTmp.cstr() ) )
		{
			sphLogDebug ( "%s.sph is not readable. Skipping", sNewPath.cstr() );
			continue;
		}

		if ( g_eWorkers==MPM_THREADS )
		{
			g_tRotateQueueMutex.Lock();
			g_dRotateQueue.Add ( sIndex );
			g_tRotateQueueMutex.Unlock();
		} else
		{
			g_dRotating.Add ( sIndex.cstr() );

			if ( !( tIndex.m_bPreopen || g_bPreopenIndexes ) )
				sphWarning ( "rotating index '%s' without preopen option; use per-index propen=1 or searchd preopen_indexes=1", sIndex.cstr() );
		}
	}

	if ( g_eWorkers!=MPM_THREADS )
		SeamlessForkPrereader ();

	if ( g_eWorkers==MPM_THREADS && !g_dRotateQueue.GetLength() ) // set rotated at empty rotation request
		g_bDoRotate = false;
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
	if ( g_iQueryLogFile!=g_iLogFile && g_iQueryLogFile>=0 && !isatty ( g_iQueryLogFile ) )
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

	g_bGotSigusr1 = false;
}


void CheckFlush ()
{
	if ( g_iAttrFlushPeriod<=0 || g_pFlush->m_bFlushing )
		return;

	// do a periodic check, unless we have a forced check
	if ( !g_pFlush->m_bForceCheck )
	{
		static int64_t tmLastCheck = -1000;
		int64_t tmNow = sphMicroTimer();

		if ( ( tmLastCheck + int64_t(g_iAttrFlushPeriod)*I64C(1000000) )>=tmNow )
			return;

		tmLastCheck = tmNow;
		sphLogDebug ( "attrflush: doing periodic check" );
	} else
	{
		sphLogDebug ( "attrflush: doing forced check" );
	}

	// check if there are dirty indexes
	int iFlushTag = g_pFlush->m_iFlushTag;
	bool bDirty = false;
	for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
	{
		const ServedIndex_t & tServed = it.Get();
		if ( tServed.m_bEnabled && tServed.m_iUpdateTag>iFlushTag )
		{
			bDirty = true;
			break;
		}
	}

	// need to set this before clearing check flag
	if ( bDirty )
		g_pFlush->m_bFlushing = true;

	// if there was a forced check in progress, it no longer is
	if ( g_pFlush->m_bForceCheck )
		g_pFlush->m_bForceCheck = false;

	// nothing to do, no indexes were updated
	if ( !bDirty )
	{
		sphLogDebug ( "attrflush: no dirty indexes found" );
		return;
	}

	// launch the flush!
	sphLogDebug ( "attrflush: forking writer" );
	int iUpdateTag = g_pFlush->m_iUpdateTag; // avoid a race between forking writer and updating flush tag
	int iPipeFD = PipeAndFork ( false, SPH_PIPE_SAVED_ATTRS ); // FIXME! gracefully handle fork() failures, Windows, etc
	if ( g_bHeadDaemon )
	{
		g_pFlush->m_iFlushTag = iUpdateTag;
		return;
	}

	// child process, do the work
	CSphVector<CSphString> dSaved;

	for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
	{
		const ServedIndex_t & tServed = it.Get();
		tServed.ReadLock();
		if ( tServed.m_bEnabled && tServed.m_iUpdateTag > iFlushTag )
		{
			if ( tServed.m_pIndex->SaveAttributes () )
				dSaved.Add ( it.GetKey() );
			else
				sphWarning ( "index %s: attrs save failed: %s", it.GetKey().cstr(), tServed.m_pIndex->GetLastError().cstr() );
		}
		tServed.Unlock();
	}

	// report and exit
	DWORD uTmp = SPH_PIPE_SAVED_ATTRS;
	sphWrite ( iPipeFD, &uTmp, sizeof(DWORD) ); // FIXME? add buffering/checks?

	uTmp = dSaved.GetLength();
	sphWrite ( iPipeFD, &uTmp, sizeof(DWORD) );

	ARRAY_FOREACH ( i, dSaved )
	{
		uTmp = strlen ( dSaved[i].cstr() );
		sphWrite ( iPipeFD, &uTmp, sizeof(DWORD) );
		sphWrite ( iPipeFD, dSaved[i].cstr(), uTmp );
	}

	::close ( iPipeFD );
	exit ( 0 );
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
		sphInfo ( "Service '%s' installed succesfully.", g_sServiceName );
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
		sphInfo ( "Service '%s' deleted succesfully.", g_sServiceName );
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
#endif
		"\n"
		"Debugging options are:\n"
		"--console\t\trun in console mode (do not fork, do not log to files)\n"
		"-p, --port <port>\tlisten on given port (overrides config setting)\n"
		"-l, --listen <spec>\tlisten on given address, port or path (overrides\n"
		"\t\t\tconfig settings)\n"
		"-i, --index <index>\tonly serve one given index\n"
		"--logdebug\t\tenable additional debug information logging\n"
#if !USE_WINDOWS
		"--nodetach\t\tdo not detach into background\n"
#endif
		"\n"
		"Examples:\n"
		"searchd --config /usr/local/sphinx/etc/sphinx.conf\n"
#if USE_WINDOWS
		"searchd --install --config c:\\sphinx\\sphinx.conf\n"
#endif
		);
}


#if USE_WINDOWS
BOOL WINAPI CtrlHandler ( DWORD )
{
	if ( !g_bService )
	{
		g_bGotSigterm = true;
		sphInterruptNow();
	}
	return TRUE;
}
#endif


#if !USE_WINDOWS
int PreforkChild ()
{
	// next one
	int iRes = fork();
	if ( iRes==-1 )
		sphFatal ( "fork() failed during prefork (error=%s)", strerror(errno) );

	// child process
	if ( iRes==0 )
	{
		g_bHeadDaemon = false;
		sphSetProcessInfo ( false );
		return iRes;
	}

	// parent process
	g_iChildren++;
	g_dChildren.Add ( iRes );
	return iRes;
}

void SetWatchDog()
{
	bool bReincarnate = true;
	bool bShutdown = false;
	int iRes = 0;
	for ( ;; )
	{
		if ( bReincarnate )
			iRes = fork();

		if ( iRes==-1 )
		{
			Shutdown ();
			sphFatal ( "fork() failed during watchdog setup (error=%s)", strerror(errno) );
		}

		// child process; reload config if necessary and return
		if ( iRes==0 )
		{
			if ( CheckConfigChanges () )
			{
				CSphConfigParser tCP;
				ReloadIndexSettings ( &tCP );
			}
			return;
		}

		// parent process, watchdog
		bReincarnate = false;
		int iPid, iStatus;
		while ( ( iPid = wait ( &iStatus ) )>0 )
		{
			assert ( iPid==iRes );
			if ( WIFEXITED ( iStatus ) )
			{
				int iExit = WEXITSTATUS ( iStatus );
				if ( iExit==2 ) // really crash
				{
					sphInfo ( "Child process %d has been finished by CRASH_EXIT (exit code 2), will be restarted", iPid );
					bReincarnate = true;
				} else
				{
					sphInfo ( "Child process %d has been finished, exit code %d. Watchdog finishes also. Good bye!", iPid, iExit );
					bShutdown = true;
				}
			} else if ( WIFSIGNALED ( iStatus ) )
			{
				if ( WTERMSIG ( iStatus )==SIGINT || WTERMSIG ( iStatus )==SIGTERM || WTERMSIG ( iStatus )==SIGKILL )
				{
					sphInfo ( "Child process %d has been killed with kill or sigterm (%i). Watchdog finishes also. Good bye!", iPid, WTERMSIG ( iStatus ) );
					bShutdown = true;
				} else
				{
					if ( WCOREDUMP ( iStatus ) )
						sphInfo ( "Child process %i has been killed with signal %i, core dumped, will be restarted", iPid, WTERMSIG ( iStatus ) );
					else
						sphInfo ( "Child process %i has been killed with signal %i, will be restarted", iPid, WTERMSIG ( iStatus ) );
					bReincarnate = true;
				}
			} else if ( WIFSTOPPED ( iStatus ) )
				sphInfo ( "Child %i stopped with signal %i", iPid, WSTOPSIG ( iStatus ) );
			else if ( WIFCONTINUED ( iStatus ) )
				sphInfo ( "Child %i resumed", iPid );
		}

		if ( bShutdown || g_bGotSigterm )
		{
			Shutdown();
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
		g_tRotateQueueMutex.Lock();
		g_bDoRotate = true;
		g_tRotateQueueMutex.Unlock();
		sphInfo ( "rotating indices (seamless=%d)", (int)g_bSeamlessRotate ); // this might hang if performed from SIGHUP
		g_bGotSighup = false;
	}

	if ( g_bGotSigterm )
	{
		assert ( g_bHeadDaemon );
		sphInfo ( "caught SIGTERM, shutting down" );

#if !USE_WINDOWS
		// in preforked mode, explicitly kill all children
		ARRAY_FOREACH ( i, g_dChildren )
		{
			sphLogDebug ( "killing child %d", g_dChildren[i] );
			kill ( g_dChildren[i], SIGTERM );
		}
#endif

		Shutdown ();
		exit ( 0 );
	}

#if !USE_WINDOWS
	if ( g_bGotSigchld )
	{
		// handle gone children
		for ( ;; )
		{
			int iChildPid = waitpid ( -1, NULL, WNOHANG );
			if ( iChildPid<=0 )
				break;

			g_iChildren--;
			g_dChildren.RemoveValue ( iChildPid ); // FIXME! OPTIMIZE! can be slow
		}
		g_bGotSigchld = false;

		// prefork more children, if needed
		if ( g_eWorkers==MPM_PREFORK )
			while ( g_dChildren.GetLength() < g_iPreforkChildren )
				if ( PreforkChild()==0 ) // child process? break from here, go work
					return;
	}
#endif

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
				g_bGotSighup = true;
				break;

			case 1:
				g_bGotSigterm = true;
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
				sphFatal ( "UNIX socket path is too long (len=%d)", len );

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

			if ( connect ( iSock, (struct sockaddr*)&sin, sizeof(sin) )<0 )
			{
				sphWarning ( "failed to connect to %s:%d: %s\n", sphFormatIP ( sBuf, sizeof(sBuf), tDesc.m_uIP ), tDesc.m_iPort, sphSockError() );
				continue;
			}
		}

		// send request
		NetOutputBuffer_c tOut ( iSock );
		tOut.SendDword ( SPHINX_SEARCHD_PROTO );
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


Listener_t * DoAccept ( int * pClientSock, char * sClientName )
{
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

	int iRes = select ( iMaxFD, &fdsAccept, NULL, NULL, &tvTimeout );
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

		if ( g_pStats )
		{
			g_tStatsMutex.Lock();
			g_pStats->m_iConnections++;
			g_tStatsMutex.Unlock();
		}

		// format client address
		if ( sClientName )
		{
			sClientName[0] = '\0';
			if ( saStorage.ss_family==AF_INET )
				sphFormatIP ( sClientName, SPH_ADDRESS_SIZE, ((struct sockaddr_in *)&saStorage)->sin_addr.s_addr );
			if ( saStorage.ss_family==AF_UNIX )
				strncpy ( sClientName, "(local)", SPH_ADDRESS_SIZE );
		}

		// accepted!
#if !USE_WINDOWS
		if ( SPH_FDSET_OVERFLOW ( iClientSock ) )
			iClientSock = dup2 ( iClientSock, g_iClientFD );
#endif

		*pClientSock = iClientSock;
		return &g_dListeners[i];
	}

	return NULL;
}


void TickPreforked ( CSphProcessSharedMutex * pAcceptMutex )
{
	assert ( !g_bHeadDaemon );
	assert ( pAcceptMutex );

	if ( g_bGotSigterm )
		exit ( 0 );

	pAcceptMutex->Lock ();

	int iClientSock = -1;
	char sClientIP[SPH_ADDRESS_SIZE];
	Listener_t * pListener = NULL;
	if ( !g_bGotSigterm )
		pListener = DoAccept ( &iClientSock, sClientIP );

	pAcceptMutex->Unlock ();

	if ( g_bGotSigterm )
		exit ( 0 ); // clean shutdown (after mutex unlock)

	if ( pListener )
	{
		HandleClient ( pListener->m_eProto, iClientSock, sClientIP, -1, NULL );
		sphSockClose ( iClientSock );
	}
}


void FailClient ( int iSock, SearchdStatus_e eStatus, const char * sMessage )
{
	assert ( eStatus==SEARCHD_RETRY || eStatus==SEARCHD_ERROR );

	int iRespLen = 4 + strlen(sMessage);

	NetOutputBuffer_c tOut ( iSock );
	tOut.SendInt ( SPHINX_SEARCHD_PROTO );
	tOut.SendWord ( (WORD)eStatus );
	tOut.SendWord ( 0 ); // version doesn't matter
	tOut.SendInt ( iRespLen );
	tOut.SendString ( sMessage );
	tOut.Flush ();

	// FIXME? without some wait, client fails to receive the response on windows
	sphSockClose ( iSock );
}


void HandlerThread ( void * pArg )
{
	// setup query guard for threaded mode
	SphCrashLogger_c tQueryTLS;
	tQueryTLS.SetupTLS ();

	// handle that client
	ThdDesc_t * pThd = (ThdDesc_t*) pArg;
	HandleClient ( pThd->m_eProto, pThd->m_iClientSock, pThd->m_sClientName.cstr(), -1, pThd );
	sphSockClose ( pThd->m_iClientSock );

	// done; remove myself from the table
	g_tThdMutex.Lock ();
	ARRAY_FOREACH ( i, g_dThd )
		if ( g_dThd[i]==pThd )
	{
		g_dThd.RemoveFast(i);
#if USE_WINDOWS
		// FIXME? this is sort of automatic on UNIX (pthread_exit() gets implicitly called on return)
		CloseHandle ( pThd->m_tThd );
#endif
		SafeDelete ( pThd );
		break;
	}
	g_tThdMutex.Unlock ();

	// something went wrong while removing; report
	if ( pThd )
	{
		sphWarning ( "thread missing from thread table" );
#if USE_WINDOWS
		// FIXME? this is sort of automatic on UNIX (pthread_exit() gets implicitly called on return)
		CloseHandle ( pThd->m_tThd );
#endif
		SafeDelete ( pThd );
	}
}


void TickHead ( CSphProcessSharedMutex * pAcceptMutex )
{
	CheckSignals ();
	CheckLeaks ();
	CheckPipes ();
	CheckDelete ();
	CheckRotate ();
	CheckReopen ();
	CheckFlush ();
	sphInfo ( NULL ); // flush dupes

	if ( pAcceptMutex )
	{
		// FIXME! what if all children are busy; we might want to accept here and temp fork more
		sphSleepMsec ( 1000 );
		return;
	}

	int iClientSock;
	char sClientName[SPH_ADDRESS_SIZE];
	Listener_t * pListener = DoAccept ( &iClientSock, sClientName );
	if ( !pListener )
		return;

	if ( ( g_iMaxChildren && g_iChildren>=g_iMaxChildren )
		|| ( g_bDoRotate && !g_bSeamlessRotate ) )
	{
		FailClient ( iClientSock, SEARCHD_RETRY, "server maxed out, retry in a second" );
		sphWarning ( "maxed out, dismissing client" );

		if ( g_pStats )
			g_pStats->m_iMaxedOut++;
		return;
	}

	// handle the client
	if ( g_eWorkers==MPM_NONE )
	{
		HandleClient ( pListener->m_eProto, iClientSock, sClientName, -1, NULL );
		sphSockClose ( iClientSock );
		return;
	}

#if !USE_WINDOWS
	if ( g_eWorkers==MPM_FORK )
	{
		int iChildPipe = PipeAndFork ( false, -1 );
		if ( !g_bHeadDaemon )
		{
			// child process, handle client
			HandleClient ( pListener->m_eProto, iClientSock, sClientName, iChildPipe, NULL );
			sphSockClose ( iClientSock );
			exit ( 0 );
		} else
		{
			// parent process, continue accept()ing
			sphSockClose ( iClientSock );
		}
	}
#endif // !USE_WINDOWS

	if ( g_eWorkers==MPM_THREADS )
	{
		ThdDesc_t * pThd = new ThdDesc_t ();
		pThd->m_eProto = pListener->m_eProto;
		pThd->m_iClientSock = iClientSock;
		pThd->m_sClientName = sClientName;

		g_tThdMutex.Lock ();
		g_dThd.Add ( pThd );
		if ( !sphThreadCreate ( &pThd->m_tThd, HandlerThread, pThd, true ) )
		{
			g_dThd.Pop();
			SafeDelete ( pThd );

			FailClient ( iClientSock, SEARCHD_RETRY, "failed to create worker thread" );
			sphWarning ( "failed to create worker thread, threads(%d), error[%d] %s", g_dThd.GetLength(), errno, strerror(errno) );
		}
		g_tThdMutex.Unlock ();
		return;
	}

	// default (should not happen)
	sphSockClose ( iClientSock );
}


void * InitSharedBuffer ( CSphSharedBuffer<BYTE> & tBuffer, int iLen )
{
	CSphString sError, sWarning;
	if ( !tBuffer.Alloc ( iLen, sError, sWarning ) )
		sphDie ( "failed to allocate shared buffer (msg=%s)", sError.cstr() );

	void * pRes = tBuffer.GetWritePtr();
	memset ( pRes, 0, iLen ); // reset
	return pRes;
}


int WINAPI ServiceMain ( int argc, char **argv )
{
	g_bLogTty = isatty ( g_iLogFile )!=0;

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
	g_hPipe = CreateNamedPipe ( szPipeName, PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT, PIPE_UNLIMITED_INSTANCES, 0, WIN32_PIPE_BUFSIZE, NMPWAIT_NOWAIT, NULL );
	ConnectNamedPipe ( g_hPipe, NULL );
#endif

	if ( !g_bService )
		fprintf ( stdout, SPHINX_BANNER );

	//////////////////////
	// parse command line
	//////////////////////

	CSphConfig		conf;
	bool			bOptStop = false;
	bool			bOptStopWait = false;
	bool			bOptStatus = false;
	bool			bOptPIDFile = false;
	const char *	sOptIndex = NULL;

	int				iOptPort = 0;
	bool			bOptPort = false;

	CSphString		sOptListen;
	bool			bOptListen = false;
	bool			bTestMode = false;

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
		OPT1 ( "--console" )		{ g_eWorkers = MPM_NONE; g_bOptNoLock = true; g_bOptNoDetach = true; }
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
		OPT1 ( "--logdebug" )		g_eLogLevel = LOG_DEBUG;
		OPT1 ( "--safetrace" )		g_bSafeTrace = true;
		OPT1 ( "--test" )			{ g_bWatchdog = false; bTestMode = true; }

		// handle 1-arg options
		else if ( (i+1)>=argc )		break;
		OPT ( "-c", "--config" )	g_sConfigFile = argv[++i];
		OPT ( "-p", "--port" )		{ bOptPort = true; iOptPort = atoi ( argv[++i] ); }
		OPT ( "-l", "--listen" )	{ bOptListen = true; sOptListen = argv[++i]; }
		OPT ( "-i", "--index" )		sOptIndex = argv[++i];
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
	CSphConfigParser cp;
	if ( !cp.Parse ( g_sConfigFile.cstr () ) )
		sphFatal ( "failed to parse config file '%s'", g_sConfigFile.cstr () );

	const CSphConfig & hConf = cp.m_tConf;

	if ( !hConf.Exists ( "searchd" ) || !hConf["searchd"].Exists ( "searchd" ) )
		sphFatal ( "'searchd' config section not found in '%s'", g_sConfigFile.cstr () );

	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];

	////////////////////////
	// stop running searchd
	////////////////////////

	if ( bOptStop )
	{
		if ( !hSearchd("pid_file") )
			sphFatal ( "stop: option 'pid_file' not found in '%s' section 'searchd'", g_sConfigFile.cstr () );

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

				if ( !WaitNamedPipe ( szPipeName, 1000 ) )
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
			sphInfo ( "stop: succesfully terminated pid %d", iPid );
			exit ( 0 );
		} else
			sphFatal ( "stop: error terminating pid %d", iPid );
#else
		CSphString sPipeName;
		int iPipeCreated = -1;
		int hPipe = -1;
		if ( bOptStopWait )
		{
			sPipeName = GetNamedPipeName ( iPid );
			iPipeCreated = mkfifo ( sPipeName.cstr(), 0666 );
			if ( iPipeCreated!=-1 )
				hPipe = ::open ( sPipeName.cstr(), O_RDONLY | O_NONBLOCK );

			if ( iPipeCreated==-1 )
				sphWarning ( "mkfifo failed (path=%s, err=%d, msg=%s); will NOT wait", sPipeName.cstr(), errno, strerror(errno) );
			else if ( hPipe==-1 )
				sphWarning ( "open failed (path=%s, err=%d, msg=%s); will NOT wait", sPipeName.cstr(), errno, strerror(errno) );
		}

		if ( kill ( iPid, SIGTERM ) )
			sphFatal ( "stop: kill() on pid %d failed: %s", iPid, strerror(errno) );
		else
			sphInfo ( "stop: successfully sent SIGTERM to pid %d", iPid );

		int iExitCode = ( bOptStopWait && ( iPipeCreated==-1 || hPipe==-1 ) ) ? 1 : 0;
		if ( bOptStopWait && hPipe!=-1 )
		{
			while ( sphIsReadable ( sPid, NULL ) )
				sphSleepMsec ( 5 );

			DWORD uStatus = 0;
			if ( ::read ( hPipe, &uStatus, sizeof(DWORD) )!=sizeof(DWORD) )
				iExitCode = 3; // stopped demon crashed during stop
			else
				iExitCode = uStatus==1 ? 0 : 2; // uStatus == 1 - AttributeSave - ok, other values - error
		}
		if ( hPipe!=-1 )
			::close ( hPipe );
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
		QueryStatus ( hSearchd("listen") );
		exit ( 0 );
	}

	/////////////////////
	// configure searchd
	/////////////////////

	if ( !hConf.Exists ( "index" ) )
		sphFatal ( "no indexes found in '%s'", g_sConfigFile.cstr () );

	#define CONF_CHECK(_hash,_key,_msg,_add) \
		if (!( _hash.Exists ( _key ) )) \
			sphFatal ( "mandatory option '%s' not found " _msg, _key, _add );

	if ( bOptPIDFile )
		CONF_CHECK ( hSearchd, "pid_file", "in 'searchd' section", "" );

	if ( hSearchd.Exists ( "read_timeout" ) && hSearchd["read_timeout"].intval()>=0 )
		g_iReadTimeout = hSearchd["read_timeout"].intval();

	if ( hSearchd.Exists ( "client_timeout" ) && hSearchd["client_timeout"].intval()>=0 )
		g_iClientTimeout = hSearchd["client_timeout"].intval();

	if ( hSearchd.Exists ( "max_children" ) && hSearchd["max_children"].intval()>=0 )
		g_iMaxChildren = hSearchd["max_children"].intval();

	g_bPreopenIndexes = hSearchd.GetInt ( "preopen_indexes", (int)g_bPreopenIndexes )!=0;
	g_bOnDiskDicts = hSearchd.GetInt ( "ondisk_dict_default", (int)g_bOnDiskDicts )!=0;
	g_bUnlinkOld = hSearchd.GetInt ( "unlink_old", (int)g_bUnlinkOld )!=0;
	g_iExpansionLimit = hSearchd.GetInt ( "expansion_limit", 0 );
	g_bCompatResults = hSearchd.GetInt ( "compat_results", (int)g_bCompatResults )!=0;

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

	if ( hSearchd("thread_stack") )
	{
		int iThreadStackSizeMin = 65536;
		int iThreadStackSizeMax = 2*1024*1024;
		int iStackSize = hSearchd.GetSize ( "thread_stack", iThreadStackSizeMin );
		if ( iStackSize<iThreadStackSizeMin || iStackSize>iThreadStackSizeMax )
			sphWarning ( "thread_stack is %d will be clamped to range ( 65k to 2M )", iStackSize );

		iStackSize = Min ( iStackSize, iThreadStackSizeMax );
		iStackSize = Max ( iStackSize, iThreadStackSizeMin );
		sphSetMyStackSize ( iStackSize );
	}

	if ( hSearchd("workers") )
	{
		if ( hSearchd["workers"]=="none" )
			g_eWorkers = MPM_NONE;
		else if ( hSearchd["workers"]=="fork" )
			g_eWorkers = MPM_FORK;
		else if ( hSearchd["workers"]=="prefork" )
			g_eWorkers = MPM_PREFORK;
		else if ( hSearchd["workers"]=="threads" )
			g_eWorkers = MPM_THREADS;
		else
			sphFatal ( "unknown workers=%s value", hSearchd["workers"].cstr() );
	}
#if USE_WINDOWS
	if ( g_eWorkers==MPM_FORK || g_eWorkers==MPM_PREFORK )
		sphFatal ( "workers=fork and workers=prefork are not supported on Windows" );
#endif

	if ( g_iMaxPacketSize<128*1024 || g_iMaxPacketSize>128*1024*1024 )
		sphFatal ( "max_packet_size out of bounds (128K..128M)" );

	if ( g_iMaxFilters<1 || g_iMaxFilters>10240 )
		sphFatal ( "max_filters out of bounds (1..10240)" );

	if ( g_iMaxFilterValues<1 || g_iMaxFilterValues>1048576 )
		sphFatal ( "max_filter_values out of bounds (1..1048576)" );

	// create and lock pid
	if ( bOptPIDFile )
	{
		g_sPidFile = hSearchd["pid_file"].cstr();

		g_iPidFD = ::open ( g_sPidFile, O_CREAT | O_WRONLY, S_IREAD | S_IWRITE );
		if ( g_iPidFD<0 )
			sphFatal ( "failed to create pid file '%s': %s", g_sPidFile, strerror(errno) );

		if ( !sphLockEx ( g_iPidFD, false ) )
			sphFatal ( "failed to lock pid file '%s': %s (searchd already running?)", g_sPidFile, strerror(errno) );
	}


	//////////////////////////////////////////////////
	// shared stuff (perf counters, flushing) startup
	//////////////////////////////////////////////////

	g_pStats = (SearchdStats_t*) InitSharedBuffer ( g_tStatsBuffer, sizeof(SearchdStats_t) );
	g_pFlush = (FlushState_t*) InitSharedBuffer ( g_tFlushBuffer, sizeof(FlushState_t) );
	g_pStats->m_uStarted = (DWORD)time(NULL);

	////////////////////
	// network startup
	////////////////////

	Listener_t tListener;
	tListener.m_eProto = PROTO_SPHINX;

	// command line arguments override config (but only in --console)
	if ( bOptListen )
	{
		AddListener ( sOptListen );

	} else if ( bOptPort )
	{
		tListener.m_iSock = sphCreateInetSocket ( htonl ( INADDR_ANY ), iOptPort );
		g_dListeners.Add ( tListener );

	} else
	{
		// listen directives in configuration file
		for ( CSphVariant * v = hSearchd("listen"); v; v = v->m_pNext )
			AddListener ( *v );

		// handle deprecated directives
		if ( hSearchd("port") )
		{
			DWORD uAddr = hSearchd.Exists("address") ?
				sphGetAddress ( hSearchd["address"].cstr(), GETADDR_STRICT ) : htonl ( INADDR_ANY );

			int iPort = hSearchd["port"].intval();
			CheckPort(iPort);

			tListener.m_iSock = sphCreateInetSocket ( uAddr, iPort );
			g_dListeners.Add ( tListener );
		}

		// still nothing? default is to listen on our two ports
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

#if !USE_WINDOWS
	// reserve an fd for clients
	int iDevNull = open ( "/dev/null", O_RDWR );
	g_iClientFD = dup ( iDevNull );
#endif

	g_pIndexes = new IndexHash_c();

	//////////////////////
	// build indexes hash
	//////////////////////

	// setup mva updates arena here, since we could have saved persistent mva updates
	sphArenaInit ( hSearchd.GetSize ( "mva_updates_pool", 1048576 ) );

	// configure and preload
	int iValidIndexes = 0;
	int iCounter = 1;
	int64_t tmLoad = -sphMicroTimer();
	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext() )
	{
		const CSphConfigSection & hIndex = hConf["index"].IterateGet();
		const char * sIndexName = hConf["index"].IterateGetKey().cstr();

		if ( g_bOptNoDetach && sOptIndex && strcasecmp ( sIndexName, sOptIndex )!=0 )
			continue;

		ESphAddIndex eAdd = AddIndex ( sIndexName, hIndex );
		if ( eAdd==ADD_LOCAL || eAdd==ADD_RT )
		{
			ServedIndex_t & tIndex = g_pIndexes->GetUnlockedEntry ( sIndexName );
			iCounter++;

			fprintf ( stdout, "precaching index '%s'\n", sIndexName );
			fflush ( stdout );
			tIndex.m_pIndex->SetProgressCallback ( ShowProgress );

			if ( HasFiles ( tIndex, g_dNewExts ) )
			{
				tIndex.m_bOnlyNew = !HasFiles ( tIndex, g_dCurExts );
				if ( RotateIndexGreedy ( tIndex, sIndexName ) )
				{
					CSphString sError;
					if ( !sphFixupIndexSettings ( tIndex.m_pIndex, hIndex, sError ) )
					{
						sphWarning ( "index '%s': %s - NOT SERVING", sIndexName, sError.cstr() );
						tIndex.m_bEnabled = false;
					}
				} else
				{
					if ( PrereadNewIndex ( tIndex, hIndex, sIndexName ) )
						tIndex.m_bEnabled = true;
				}
			} else
			{
				tIndex.m_bOnlyNew = false;
				if ( PrereadNewIndex ( tIndex, hIndex, sIndexName ) )
					tIndex.m_bEnabled = true;
			}

			CSphString sError;
			if ( tIndex.m_bEnabled && !CheckIndex ( tIndex.m_pIndex, sError ) )
			{
				sphWarning ( "index '%s': %s - NOT SERVING", sIndexName, sError.cstr() );
				tIndex.m_bEnabled = false;
			}

			if ( !tIndex.m_bEnabled )
				continue;
		}

		iValidIndexes++;
	}

	tmLoad += sphMicroTimer();
	if ( !iValidIndexes )
		sphFatal ( "no valid indexes to serve" );
	else
		fprintf ( stdout, "precached %d indexes in %0.3f sec\n", iCounter-1, float(tmLoad)/1000000 );

	///////////
	// startup
	///////////

	if ( g_eWorkers==MPM_THREADS )
		sphRTInit();

	// handle my signals
	SetSignalHandlers ();

	// create logs
	if ( !g_bOptNoLock )
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

		g_sLogFile = sLog;
		g_bLogTty = isatty ( g_iLogFile )!=0;

		// create query log if required
		if ( hSearchd.Exists ( "query_log" ) )
		{
			g_iQueryLogFile = open ( hSearchd["query_log"].cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
			if ( g_iQueryLogFile<0 )
				sphFatal ( "failed to open query log file '%s': %s", hSearchd["query_log"].cstr(), strerror(errno) );
			g_sQueryLogFile = hSearchd["query_log"].cstr();
		}
	}

	// almost ready, time to start listening
	int iBacklog = hSearchd.GetInt ( "listen_backlog", SEARCHD_BACKLOG );
	ARRAY_FOREACH ( i, g_dListeners )
		if ( listen ( g_dListeners[i].m_iSock, iBacklog )==-1 )
			sphFatal ( "listen() failed: %s", sphSockError() );

	// prepare to detach
	if ( !g_bOptNoDetach )
	{
#if !USE_WINDOWS
		close ( STDIN_FILENO );
		close ( STDOUT_FILENO );
		close ( STDERR_FILENO );
		dup2 ( iDevNull, STDIN_FILENO );
		dup2 ( iDevNull, STDOUT_FILENO );
		dup2 ( iDevNull, STDERR_FILENO );
#endif

		// explicitly unlock everything in parent immediately before fork
		//
		// there's a race in case another instance is started before
		// child re-acquires all locks; but let's hope that's rare
		for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
		{
			ServedIndex_t & tServed = it.Get();
			if ( tServed.m_bEnabled )
				tServed.m_pIndex->Unlock();
		}
	}

	if ( bOptPIDFile )
		sphLockUn ( g_iPidFD );

#if !USE_WINDOWS
	if ( !g_bOptNoDetach )
	{
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
				sphSetProcessInfo ( false );
				exit ( 0 );
		}
	}
	if ( g_bWatchdog && g_eWorkers==MPM_THREADS )
		SetWatchDog();
#endif

	if ( g_eWorkers==MPM_THREADS )
		sphRTConfigure ( hSearchd, bTestMode );

	if ( bOptPIDFile )
	{
#if !USE_WINDOWS
		// re-lock pid
		// FIXME! there's a potential race here
		if ( !sphLockEx ( g_iPidFD, true ) )
			sphFatal ( "failed to re-lock pid file '%s': %s", g_sPidFile, strerror(errno) );
#endif

		char sPid[16];
		snprintf ( sPid, sizeof(sPid), "%d\n", (int)getpid() );
		int iPidLen = strlen(sPid);

		lseek ( g_iPidFD, 0, SEEK_SET );
		if ( !sphWrite ( g_iPidFD, sPid, iPidLen ) )
			sphFatal ( "failed to write to pid file '%s' (errno=%d, msg=%s)", g_sPidFile,
				errno, strerror(errno) );

		if ( ::ftruncate ( g_iPidFD, iPidLen ) )
			sphFatal ( "failed to truncate pid file '%s' (errno=%d, msg=%s)", g_sPidFile,
				errno, strerror(errno) );
	}

#if USE_WINDOWS
	SetConsoleCtrlHandler ( CtrlHandler, TRUE );
#endif

	if ( !g_bOptNoDetach )
	{
		// re-lock indexes
		for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
		{
			ServedIndex_t & tServed = it.Get();
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

			// try to mlock again because mlock does not survive over fork
			if ( !tServed.m_pIndex->Mlock() )
			{
				sphWarning ( "index '%s': %s", it.GetKey().cstr(),
					tServed.m_pIndex->GetLastError().cstr() );
			}
		}
	}

	// if we're running in console mode, dump queries to tty as well
	if ( g_bOptNoLock && hSearchd ( "query_log" ) )
		g_iQueryLogFile = g_iLogFile;

	/////////////////
	// serve clients
	/////////////////

	g_bHeadDaemon = true;

#if USE_WINDOWS
	if ( g_bService )
		MySetServiceStatus ( SERVICE_RUNNING, NO_ERROR, 0 );
#endif

	sphSetReadBuffers ( hSearchd.GetSize ( "read_buffer", 0 ), hSearchd.GetSize ( "read_unhinted", 0 ) );

	CSphProcessSharedMutex * pAcceptMutex = NULL;
#if !USE_WINDOWS
	if ( g_eWorkers==MPM_PREFORK )
	{
		pAcceptMutex = new CSphProcessSharedMutex();
		if ( !pAcceptMutex )
			sphFatal ( "failed to create process-shared mutex" );

		while ( g_dChildren.GetLength() < g_iPreforkChildren )
			if ( PreforkChild()==0 ) // child process? break from here, go work
				break;
	}
#endif

	// in threaded mode, create a dedicated rotation thread
	if ( g_eWorkers==MPM_THREADS )
	{
		if ( !g_tThdMutex.Init() || !g_tRotateQueueMutex.Init() || !g_tRotateConfigMutex.Init() )
			sphDie ( "failed to init mutex" );

		if ( g_bSeamlessRotate && !sphThreadCreate ( &g_tRotateThread, RotationThreadFunc , 0 ) )
			sphDie ( "failed to create rotation thread" );

		// reserving max to keep memory consumption constant between frames
		g_dThd.Reserve ( g_iMaxChildren*2 );

		g_tDistLock.Init();
	}

	// replay last binlog
	SmallStringHash_T<CSphIndex*> hIndexes;
	for ( IndexHashIterator_c it ( g_pIndexes ); it.Next(); )
		if ( it.Get().m_bEnabled )
			hIndexes.Add ( it.Get().m_pIndex, it.GetKey() );

	if ( g_eWorkers==MPM_THREADS )
		sphReplayBinlog ( hIndexes, DumpMemStat );

	if ( !g_bOptNoDetach )
		g_bLogStdout = false;

	// create flush-rt thread
	if ( g_eWorkers==MPM_THREADS && !sphThreadCreate ( &g_tRtFlushThread, RtFlushThreadFunc, 0 ) )
		sphDie ( "failed to create rt-flush thread" );

	sphInfo ( "accepting connections" );

	for ( ;; )
	{
		SphCrashLogger_c::SetupTimePID();

		if ( !g_bHeadDaemon && pAcceptMutex )
			TickPreforked ( pAcceptMutex );
		else
			TickHead ( pAcceptMutex );
	}
} // NOLINT function length


bool DieCallback ( const char * sMessage )
{
	sphLogFatal ( "%s", sMessage );
	return false; // caller should not log
}


int main ( int argc, char **argv )
{
	// threads should be initialized before memory allocations
	char cTopOfMainStack;
	sphThreadInit();
	MemorizeStack ( &cTopOfMainStack );
	sphSetDieCallback ( DieCallback );
	sphSetLogger ( sphLog );

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
