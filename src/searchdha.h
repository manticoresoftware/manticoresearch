//
// $Id$
//

//
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

/// @file searchdha.h
/// Declarations for the stuff specifically needed by searchd to work with remote agents
/// and high availability funcs


#ifndef _searchdha_
#define _searchdha_

#include <utility>

#if HAVE_KQUEUE
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif

/////////////////////////////////////////////////////////////////////////////
// SOME SHARED GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////

extern int				g_iReadTimeout; // defined in searchd.cpp

extern int				g_iPingInterval;		// by default ping HA agents every 1 second
extern DWORD			g_uHAPeriodKarma;		// by default use the last 1 minute statistic to determine the best HA agent
extern int				g_iPersistentPoolSize;

extern int				g_iAgentConnectTimeout;
extern int				g_iAgentQueryTimeout;	// global (default). May be override by index-scope values, if one specified
extern bool				g_bHostnameLookup;

const int	STATS_DASH_TIME = 15;	///< store the history for last periods

/////////////////////////////////////////////////////////////////////////////
// MISC GLOBALS
/////////////////////////////////////////////////////////////////////////////

/// known default sphinx ports
/// (assigned by IANA, see http://www.iana.org/assignments/port-numbers for details)
enum IanaPorts_e
{
	IANA_PORT_SPHINXQL	= 9306,
	IANA_PORT_SPHINXAPI	= 9312
};

/// known status return codes
enum SearchdStatus_e : WORD
{
	SEARCHD_OK		= 0,	///< general success, command-specific reply follows
	SEARCHD_ERROR	= 1,	///< general failure, error message follows
	SEARCHD_RETRY	= 2,	///< temporary failure, error message follows, client should retry later
	SEARCHD_WARNING	= 3		///< general success, warning message and command-specific reply follow
};

/// remote agent state
enum AgentState_e
{
	AGENT_UNUSED = 0,				///< agent is unused for this request
	AGENT_CONNECTING,				///< connecting to agent in progress, write handshake on socket ready
	AGENT_HANDSHAKE,				///< waiting for "VER x" hello, read response on socket ready
	AGENT_ESTABLISHED,				///< handshake completed. Ready to sent query, write query on socket ready
	AGENT_QUERYED,					///< query sent, waiting for reply. read reply on socket ready
	AGENT_PREREPLY,					///< query sent, activity detected, need to read reply
	AGENT_REPLY,					///< reading reply
	AGENT_RETRY						///< should retry
};

/// per-agent query stats (raw, filled atomically on-the-fly)
enum AgentStats_e
{
	eTimeoutsQuery = 0,	///< number of time-outed queries
	eTimeoutsConnect,	///< number of time-outed connections
	eConnectFailures,	///< failed to connect
	eNetworkErrors,		///< network error
	eWrongReplies,		///< incomplete reply
	eUnexpectedClose,	///< agent closed the connection
	eNetworkCritical,		///< agent answered, but with warnings
	eNetworkNonCritical,	///< successful queries, no errors
	eMaxAgentStat
};

/// per-host query stats (calculated)
enum HostStats_e
{
	ehTotalMsecs=0,		///< number of microseconds in queries, total
	ehConnTries,		///< total number of connect tries
	ehAverageMsecs,		///< average connect time
	ehMaxMsecs,			///< maximal connect time
	ehMaxStat
};

enum HAStrategies_e {
	HA_RANDOM,
	HA_ROUNDROBIN,
	HA_AVOIDDEAD,
	HA_AVOIDERRORS,
	HA_AVOIDDEADTM,			///< the same as HA_AVOIDDEAD, but uses just min timeout instead of weighted random
	HA_AVOIDERRORSTM,		///< the same as HA_AVOIDERRORS, but uses just min timeout instead of weighted random

	HA_DEFAULT = HA_RANDOM
};

// manages persistent connections to a host
// serves a FIFO queue.
// I.e. if we have 2 connections to a host, and one task rent the connection,
// we will return 1-st socket. And the next rent request will definitely 2-nd socket
// whenever 1-st socket already released or not.
// (previous code used LIFO strategy)
struct PersistentConnectionsPool_t
{
	mutable CSphMutex	 m_dDataLock;
private:
	bool			m_bShutdown = false;			// will cause ReturnConnection to close the socket instead of returning it
	CSphVector<int>	m_dSockets GUARDED_BY ( m_dDataLock );
	int				m_iRit GUARDED_BY ( m_dDataLock ) = 0; // pos where we take the next socket to rent.
	int				m_iWit GUARDED_BY ( m_dDataLock ) = 0; // pos where we will put returned socket.
	int				m_iFreeWindow GUARDED_BY ( m_dDataLock ) = 0; // # of free sockets in the existing ring
	using CGuard = CSphScopedLock<CSphMutex>;

	int Step ( int* ) REQUIRES ( m_dDataLock ); // step helper over the ring

public:
	PersistentConnectionsPool_t () = default;
	void Init ( int iPoolSize ) EXCLUDES ( m_dDataLock );
	int RentConnection () EXCLUDES ( m_dDataLock );
	void ReturnConnection ( int iSocket ) EXCLUDES ( m_dDataLock );
	void Shutdown () EXCLUDES ( m_dDataLock );
};

void ClosePersistentSockets();

struct HostDashboard_t;
struct AgentDash_t;

// generic descriptor of remote index (agent)
class AgentDesc_c : public ISphNoncopyable
{
public:
	CSphString			m_sIndexes;				///< remote index names to query

	int					m_iFamily = AF_INET;	///< TCP or UNIX socket
	CSphString			m_sAddr;				///< remote searchd host (used to update m_uAddr with resolver)
	int					m_iPort = -1;		///< remote searchd port, 0 if local
	DWORD				m_uAddr = 0;		///< IP address
	bool				m_bNeedResolve = false;	///< whether we keep m_uAddr, or call GetAddrInfo each time.

	bool		m_bBlackhole	= false;		///< blackhole agent flag
	bool		m_bPersistent	= false;		///< whether to keep the persistent connection to the agent.

	mutable AgentDash_t *		m_pStats = nullptr;	/// global agent stats (one copy shared over all clones, refcounted)
	mutable HostDashboard_t *	m_pDash = nullptr;	/// ha dashboard of the host

public:
	AgentDesc_c () = default;
	~AgentDesc_c ();

	void CloneTo ( AgentDesc_c & tOther ) const;

	CSphString GetMyUrl () const;
};

class MultiAgentDesc_c;
/// remote agent connection (local per-query state)
struct AgentConn_t : public ISphNoncopyable
{
	int				m_iSock = -1;		///< socket number, -1 if not connected
	bool			m_bFresh = true;	///< if persistent, need SEARCHD_COMMAND_PERSIST
	bool			m_bSuccess = false;	///< agent got processed, no need to retry

	SearchdStatus_e	m_eReplyStatus { SEARCHD_ERROR };	///< reply status code
	int				m_iReplySize = 0;	///< how many reply bytes are there
	int				m_iReplyRead = 0;	///< how many reply bytes are already received
	int 			m_iRetries = 0;		///< count from 0 to m_iRetryLimit
	int 			m_iRetryLimit = 0;	///< how many times retry (m.b. with another mirror)

	CSphFixedVector<BYTE>			m_dReplyBuf { 0 };


	int64_t			m_iWall = 0;		///< wall time spent vs this agent
	int64_t			m_iWaited = 0;		///< statistics of waited
	int64_t			m_iStartQuery = 0;	///< the timestamp of the latest request
	int64_t			m_iEndQuery = 0;	///< the timestamp of the end of the latest operation
	AgentDesc_c		m_tDesc;			///< desc of my host

	// some external stuff
	CSphVector<CSphQueryResult> m_dResults;	///< multi-query results
	CSphString		m_sFailure;				///< failure message (both network and logical)
	mutable int		m_iStoreTag = 0;	///< cookie, m.b. used to 'glue' to concrete connection
	int				m_iWeight = -1;		///< weight of the index, will be send with query to remote host
	bool			m_bPing = false;	///< to avoid polluting stats by ping query time

private:
	int				m_iMirrorsCount = 1;
	AgentState_e	m_eConnState { AGENT_UNUSED };	///< current state
	MultiAgentDesc_c * m_pMultiAgent = nullptr;

public:
	AgentConn_t () = default;
	~AgentConn_t ();

	void Close ( bool bClosePersist=true );
	void Fail ( AgentStats_e eStat, const char* sMessage, ... ) __attribute__ ( ( format ( printf, 3, 4 ) ) );

	void SetMultiAgent ( const CSphString &sIndex, MultiAgentDesc_c * pMirror );
	void NextMirror();
	int GetMirrorsCount() const { return m_iMirrorsCount; }
	AgentState_e State() const { return m_eConnState; } 
	void State ( AgentState_e eState ); 

private:
	void SetupPersist();
};

using AgentsVector = CSphVector<AgentConn_t *>;


extern const char * sAgentStatsNames[eMaxAgentStat + ehMaxStat];

struct AgentDash_t : ISphRefcountedMT
{
	// was uint64_t, but for atomic it creates extra tmpl instantiation without practical difference
	CSphAtomic_T<int64_t>	m_dStats[eMaxAgentStat]; // atomic counters
	uint64_t				m_dHostStats[ehMaxStat];
	DWORD					m_uTimestamp;

	void Reset ()
	{
		for ( auto& dStat : m_dStats )
			dStat = 0;

		for ( auto& dHostStat: m_dHostStats )
			dHostStat = 0;
	}

	void Add ( const AgentDash_t& rhs )
	{
		for ( int i = 0; i<eMaxAgentStat; ++i )
			m_dStats[i] += rhs.m_dStats[i];

		if ( m_dHostStats[ehConnTries] )
			m_dHostStats[ehAverageMsecs] =
			( m_dHostStats[ehAverageMsecs] * m_dHostStats[ehConnTries] 
				+ rhs.m_dHostStats[ehAverageMsecs] * rhs.m_dHostStats[ehConnTries] )
			/ ( m_dHostStats[ehConnTries] + rhs.m_dHostStats[ehConnTries] );
		else
			m_dHostStats[ehAverageMsecs] = rhs.m_dHostStats[ehAverageMsecs];
		m_dHostStats[ehMaxMsecs] = Max ( m_dHostStats[ehMaxMsecs], rhs.m_dHostStats[ehMaxMsecs] );
		m_dHostStats[ehConnTries] += rhs.m_dHostStats[ehConnTries];
	}
};

using AgentDashPtr_t = CSphRefcountedPtr<AgentDash_t>;

using HostStatSnapshot_t = uint64_t[eMaxAgentStat+ehMaxStat];

/// per-host dashboard
struct HostDashboard_t : public ISphRefcountedMT
{
	AgentDesc_c		m_tDescriptor;			// only host info, no indices. Used for ping.
	bool			m_bNeedPing = false;	// we'll ping only HA agents, not everyone

	mutable CSphRwlock	 m_dDataLock;	// fixme! delete. (make guarded vars as atomics and eliminate the lock).
	int64_t		m_iLastAnswerTime GUARDED_BY ( m_dDataLock );		// updated when we get an answer from the host
	int64_t		m_iLastQueryTime GUARDED_BY ( m_dDataLock );		// updated when we send a query to a host
	int64_t		m_iErrorsARow GUARDED_BY ( m_dDataLock ) = 0;		// num of errors a row, updated when we update the general statistic.

	PersistentConnectionsPool_t*	m_pPersPool = nullptr;			// persistence pool also lives here, one per dashboard

private:
	AgentDash_t	m_dStats[STATS_DASH_TIME] GUARDED_BY ( m_dDataLock );

public:
	explicit HostDashboard_t ( const AgentDesc_c & tAgent );
	~HostDashboard_t ();
	bool IsOlder ( int64_t iTime ) const REQUIRES_SHARED ( m_dDataLock );
	static DWORD GetCurSeconds();
	static bool IsHalfPeriodChanged ( DWORD * pLast );
	AgentDash_t*	GetCurrentStat() REQUIRES ( m_dDataLock );
	void GetCollectedStat ( HostStatSnapshot_t& dResult, int iPeriods=1 ) const REQUIRES ( !m_dDataLock );
};

using HostDashboardPtr_t = CSphRefcountedPtr<HostDashboard_t>;


// set of options which are applied to every agent line
// and come partially from global config, partially m.b. set immediately in agent line as an option.
struct AgentOptions_t
{
	bool m_bBlackhole;
	bool m_bPersistent;
	HAStrategies_e m_eStrategy;
	int m_iRetryCount;
	int m_iRetryCountMultiplier;
};

// for error-reporting
struct WarnInfo_t
{
	const char * m_szIndexName;
	const char * m_szAgent;

	void Warn ( const char * sFmt, ... ) const
	{
		va_list ap;
		va_start ( ap, sFmt );
		if ( m_szIndexName )
			sphLogVa ( CSphString ().SetSprintf ( "index '%s': agent '%s': %s", m_szIndexName, m_szAgent, sFmt ).cstr ()
					   , ap, SPH_LOG_INFO );
		else
			sphLogVa ( CSphString ().SetSprintf ( "host '%s': %s", m_szAgent, sFmt ).cstr ()
					   , ap, SPH_LOG_INFO );
		va_end ( ap );
	}

	// always returns false - to simplify commonly used 'warn ...; return false;'
	bool ErrSkip ( const char * sFmt, ... ) const
	{
		va_list ap;
		va_start ( ap, sFmt );
		if ( m_szIndexName )
			sphLogVa (
				CSphString ().SetSprintf ( "index '%s': agent '%s': %s, - SKIPPING AGENT", m_szIndexName, m_szAgent, sFmt ).cstr ()
				   , ap );
		else
			sphLogVa (
				CSphString ().SetSprintf ( "host '%s': %s, - SKIPPING AGENT", m_szAgent , sFmt ).cstr ()
				, ap );
		va_end ( ap );
		return false;
	}
};

/// descriptor for set of agents (mirrors) (stored in a global hash)
class MultiAgentDesc_c : public ISphRefcountedMT, public CSphFixedVector<AgentDesc_c>
{
	CSphAtomic				m_iRRCounter;	/// round-robin counter
	mutable RwLock_t		m_dWeightLock;	/// manages access to m_pWeights
	CSphFixedVector<float>	m_dWeights		/// the weights of the hosts
			GUARDED_BY (m_dWeightLock) { 0 };
	DWORD					m_uTimestamp { HostDashboard_t::GetCurSeconds() };	/// timestamp of last weight's actualization
	HAStrategies_e			m_eStrategy { HA_DEFAULT };
	int 					m_iMultiRetryCount = 0;

	~MultiAgentDesc_c () final = default;

public:
	MultiAgentDesc_c()
		: CSphFixedVector<AgentDesc_c> {0}
	{}

	bool Init ( const AgentOptions_t &tOpt, const CSphVector<AgentDesc_c *> &dHosts, const WarnInfo_t &tWarn );

	const AgentDesc_c & ChooseAgent () REQUIRES ( !m_dWeightLock );

	inline bool IsHA() const
	{
		return GetLength() > 1;
	}

	inline int GetRetryLimit() const
	{
		return m_iMultiRetryCount;
	}

	CSphFixedVector<float> GetWeights () const REQUIRES (!m_dWeightLock)
	{
		CSphScopedRLock tRguard ( m_dWeightLock );
		CSphFixedVector<float> dResult {0};
		dResult.CopyFrom ( m_dWeights );
		return dResult;
	}

private:

	const AgentDesc_c & RRAgent ();
	const AgentDesc_c & RandAgent ();
	const AgentDesc_c & StDiscardDead () REQUIRES ( !m_dWeightLock );
	const AgentDesc_c & StLowErrors () REQUIRES ( !m_dWeightLock );

	void ChooseWeightedRandAgent ( int * pBestAgent, CSphVector<int> & dCandidates ) REQUIRES ( !m_dWeightLock );
	void CheckRecalculateWeights ( const CSphFixedVector<int64_t> &dTimers ) REQUIRES ( !m_dWeightLock );
};

using MultiAgentDescPtr_c = CSphRefcountedPtr<MultiAgentDesc_c>;

/////////////////////////////////////////////////////////////////////////////
// DISTRIBUTED QUERIES
/////////////////////////////////////////////////////////////////////////////

struct AgentProcessor_t
{
	AgentProcessor_t () = default;
	virtual ~AgentProcessor_t ()
	{};
	virtual void Process ( AgentDesc_c &tDesc ) = 0;
};

/// distributed index
struct DistributedIndex_t : public ServedStats_c, public ISphRefcountedMT
{
protected:
	~DistributedIndex_t () override
	{
		// m_pHAStorage has to be freed separately.
		for ( auto *&pAgent : m_dAgents ) SafeRelease ( pAgent );
	}

public:
	CSphVector<MultiAgentDesc_c *> m_dAgents;	///< remote agents
	StrVec_t m_dLocal;							///< local indexes
	CSphBitvec m_dKillBreak;
	int m_iAgentConnectTimeout		{ g_iAgentConnectTimeout };	///< in msec
	int m_iAgentQueryTimeout		{ g_iAgentQueryTimeout };	///< in msec
	int m_iAgentRetryCount			= 0;			///< overrides global one
	bool m_bDivideRemoteRanges		= false;		///< whether we divide big range onto agents or not
	HAStrategies_e m_eHaStrategy	= HA_DEFAULT;	///< how to select the best of my agents

	// fixme! remove this.
	bool m_bToDelete = false;        ///< should be deleted

	// get hive of all index'es hosts (not agents, but hosts, i.e. all mirrors as simple vector)
	void GetAllHosts ( AgentsVector &dTarget ) const;

	// call tProcessor::Process over every single host in the hive
	void ForEveryHost ( AgentProcessor_t &tProcessor );
};

using DistributedIndexPtr = CSphRefcountedPtr<DistributedIndex_t>;

class SCOPED_CAPABILITY RLockedDistrIt_c : public RLockedHashIt_c
{
public:
	explicit RLockedDistrIt_c ( const GuardedHash_c * pHash ) ACQUIRE_SHARED ( pHash->IndexesRWLock ()
																				, m_pHash->IndexesRWLock () )
		: RLockedHashIt_c ( pHash )
	{}

	~RLockedDistrIt_c () UNLOCK_FUNCTION() {};

	DistributedIndexPtr Get () REQUIRES_SHARED ( m_pHash->IndexesRWLock () )
	{
		auto pDistr = ( DistributedIndex_t * ) RLockedHashIt_c::Get ();
		DistributedIndexPtr pRes ( pDistr );
		return pRes;
	}
};

extern GuardedHash_c * g_pDistIndexes;    // distributed indexes hash

inline DistributedIndexPtr GetDistr ( const CSphString &sName )
{
	auto pDistr = ( DistributedIndex_t * ) g_pDistIndexes->Get ( sName );
	DistributedIndexPtr pRes ( pDistr );
	return pRes;
}


struct SearchdStats_t
{
	DWORD		m_uStarted = 0;
	CSphAtomicL		m_iConnections;
	CSphAtomicL		m_iMaxedOut;
	CSphAtomicL		m_iCommandCount[SEARCHD_COMMAND_TOTAL];
	CSphAtomicL		m_iAgentConnect;
	CSphAtomicL		m_iAgentRetry;

	CSphAtomicL		m_iQueries;			///< search queries count (differs from search commands count because of multi-queries)
	CSphAtomicL		m_iQueryTime;		///< wall time spent (including network wait time)
	CSphAtomicL		m_iQueryCpuTime;	///< CPU time spent

	CSphAtomicL		m_iDistQueries;		///< distributed queries count
	CSphAtomicL		m_iDistWallTime;	///< wall time spent on distributed queries
	CSphAtomicL		m_iDistLocalTime;	///< wall time spent searching local indexes in distributed queries
	CSphAtomicL		m_iDistWaitTime;	///< time spent waiting for remote agents in distributed queries

	CSphAtomicL		m_iDiskReads;		///< total read IO calls (fired by search queries)
	CSphAtomicL		m_iDiskReadBytes;	///< total read IO traffic
	CSphAtomicL		m_iDiskReadTime;	///< total read IO time

	CSphAtomicL		m_iPredictedTime;	///< total agent predicted query time
	CSphAtomicL		m_iAgentPredictedTime;	///< total agent predicted query time
};

class cDashStorage : public ISphNoncopyable
{
	using cDashVector = CSphVector<HostDashboard_t*>;
	cDashVector						m_dDashes GUARDED_BY(m_tDashLock);
	mutable CSphRwlock				m_tDashLock;

public:
	void				AddAgent ( AgentDesc_c * pAgent ) EXCLUDES ( m_tDashLock );
	HostDashboard_t *	FindAgent ( const char* sAgent ) const EXCLUDES ( m_tDashLock );
	void				GetActiveDashes ( cDashVector & dAgents ) const EXCLUDES ( m_tDashLock );

	cDashStorage ()
	{
		m_tDashLock.Init();
	}

	~cDashStorage() // fixme! release m_dDashes also!
	{
		m_tDashLock.Done();
	}
};



extern SearchdStats_t			g_tStats;
extern cDashStorage				g_tDashes;



// parse strategy name into enum value
bool ParseStrategyHA ( const char * sName, HAStrategies_e * pStrategy );

// parse ','-delimited list of indexes
void ParseIndexList ( const CSphString &sIndexes, StrVec_t &dOut );

// try to parse hostname/ip/port or unixsocket on current pConfigLine.
// fill pAgent fields on success and move ppLine pointer next after parsed instance
// if :port is skipped in the line, IANA 9312 will be used in the case
bool ParseAddressPort ( AgentDesc_c * pAgent, const char ** ppLine, const WarnInfo_t& dInfo );

//bool ParseAgentLine ( MultiAgentDesc_c &tAgent, const char * szAgent, const char * szIndexName, AgentOptions_t tDesc );
bool ConfigureMultiAgent ( MultiAgentDesc_c &tAgent, const char * szAgent, const char * szIndexName
						   , AgentOptions_t tOptions );

struct IRequestBuilder_t : public ISphNoncopyable
{
	virtual ~IRequestBuilder_t () {} // to avoid gcc4 warns
	virtual void BuildRequest ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const = 0;
};


struct IReplyParser_t
{
	virtual ~IReplyParser_t () {} // to avoid gcc4 warns
	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const = 0;
};


struct AgentConnectionContext_t
{
	const IRequestBuilder_t * m_pBuilder = nullptr;
	AgentConn_t	** m_ppAgents = nullptr;
	int m_iAgentCount = 0;
	int m_iTimeout = 0;
	int m_iDelay = 0;
};

void RemoteConnectToAgent ( AgentConn_t & tAgent );
int RemoteQueryAgents ( AgentConnectionContext_t * pCtx );

// processing states AGENT_QUERY, AGENT_PREREPLY and AGENT_REPLY
// may work in parallel with RemoteQueryAgents, so the state MAY change during a call.
int RemoteWaitForAgents ( AgentsVector & dAgents, int iTimeout, IReplyParser_t & tParser );


class ISphRemoteAgentsController : ISphNoncopyable
{
public:

	virtual ~ISphRemoteAgentsController () {}

	// check that there are no works to do
	virtual bool IsDone () const = 0;

	// block execution while there are works to do
	virtual int Finish () = 0;

	// check that some agents are done at this iteration
	virtual bool HasReadyAgents () const = 0;

	// check that some agents are done, and reset counter of them
	virtual int FetchReadyAgents () = 0;

	// block execution while some works finished
	virtual void WaitAgentsEvent () = 0;

	// reschedule agents with retry status
	// returns num of them.
	virtual int RetryFailed () = 0;
};

ISphRemoteAgentsController* GetAgentsController ( int iThreads, AgentsVector & dAgents,
		const IRequestBuilder_t & tBuilder, int iTimeout, int iRetryMax=0, int iDelay=0 );


//////////////////////////////////////////////////////////////////////////
// Universal work with select/poll/epoll/kqueue
//////////////////////////////////////////////////////////////////////////

/// check if a non-blocked socket is still connected
bool sphNBSockEof ( int iSock );


// wrapper around epoll/kqueue/poll
struct NetEventsIterator_t
{
	const void * 		m_pData = nullptr;
	DWORD				m_uEvents = 0;

	void Reset ()
	{
		m_pData = nullptr;
		m_uEvents = 0;
	}

	bool IsReadable () const;
	bool IsWritable () const;
};

class ISphNetEvents : public ISphNoncopyable
{
public:
	enum PoolEvents_e
	{
		SPH_POLL_RD = 1UL << 0,
		SPH_POLL_WR = 1UL << 1,
		SPH_POLL_HUP = 1UL << 2,
		SPH_POLL_ERR = 1UL << 3,
		SPH_POLL_PRI = 1UL << 4,
	};
public:
	virtual ~ISphNetEvents ();
	virtual void SetupEvent ( int iSocket, PoolEvents_e eFlags, const void * pData ) = 0;
	virtual bool Wait ( int ) = 0;
	virtual int IterateStart () = 0;
	virtual bool IterateNextAll () = 0;
	virtual bool IterateNextReady () = 0;
	virtual void IterateChangeEvent ( int iSocket, PoolEvents_e eFlags ) = 0;
	virtual void IterateRemove ( int iSocket ) = 0;
	virtual NetEventsIterator_t & IterateGet () = 0;
};


inline bool NetEventsIterator_t::IsReadable () const
{
	return bool ( m_uEvents & ISphNetEvents::SPH_POLL_RD );
}

inline bool NetEventsIterator_t::IsWritable () const
{
	return bool ( m_uEvents & ISphNetEvents::SPH_POLL_WR );
}

// all fresh codeflows use version with poll/epoll/kqueue.
// legacy also set bFallbackSelect and it invokes 'select' for the case
// when nothing of poll/epoll/kqueue is available.
ISphNetEvents * sphCreatePoll ( int iSizeHint, bool bFallbackSelect = false );

// determine which branch will be used
// defs placed here for easy switch between/debug
#if HAVE_EPOLL
#define POLLING_EPOLL 1
#elif HAVE_KQUEUE
#define POLLING_KQUEUE 1
#elif HAVE_POLL
#define POLLING_POLL 1
#else
#define POLLING_SELECT 1
#endif

#endif // _searchdha_

//
// $Id$
//
