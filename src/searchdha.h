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
enum SearchdStatus_e
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

// per-host query stats (calculated, 
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
	bool			m_bShutdown;			// will cause ReturnConnection to close the socket instead of returning it
	CSphVector<int>	m_dSockets GUARDED_BY ( m_dDataLock );
	int				m_iRit GUARDED_BY ( m_dDataLock ); // pos where we take the next socket to rent.
	int				m_iWit GUARDED_BY ( m_dDataLock ); // pos where we will put returned socket.
	int				m_iFreeWindow GUARDED_BY ( m_dDataLock ); // # of free sockets in the existing ring
	using CGuard = CSphScopedLock<CSphMutex>;

	int Step ( int* ) REQUIRES ( m_dDataLock ); // step helper over the ring

public:
	PersistentConnectionsPool_t ();
	void Init ( int iPoolSize ) EXCLUDES ( m_dDataLock );
	int RentConnection () EXCLUDES ( m_dDataLock );
	void ReturnConnection ( int iSocket ) EXCLUDES ( m_dDataLock );
	void Shutdown () EXCLUDES ( m_dDataLock );
};


void ClosePersistentSockets();

struct AgentStats_t : public ISphRefcountedMT
{
	// was uint64_t, but for atomic it creates extra tmpl instantiation without practical difference
	CSphAtomic_T<int64_t>		m_dStats[eMaxAgentStat];

	void Reset ()
	{
		for ( int i = 0; i<eMaxAgentStat; ++i )
			m_dStats[i] = 0;
	}
	void Add ( const AgentStats_t& rhs )
	{
		for ( int i = 0; i<eMaxAgentStat; ++i )
			m_dStats[i] += rhs.m_dStats[i];
	}
};

struct HostDashboard_t;
struct AgentDash_t;

// generic descriptor of remote index (agent)
class AgentDesc_c : public ISphNoncopyable
{
public:
	CSphString		m_sIndexes;		///< remote index names to query
	bool			m_bBlackhole;	///< blackhole agent flag
	DWORD			m_uAddr;		///< IP address
	mutable AgentDash_t *		m_pStats;		/// global agent stats
	mutable HostDashboard_t *	m_pDash;		/// ha dashboard of the host
	bool			m_bPersistent;	///< whether to keep the persistent connection to the agent.

	int				m_iFamily;		///< TCP or UNIX socket
	CSphString		m_sPath;		///< local searchd UNIX socket path
	CSphString		m_sHost;		///< remote searchd host
	int				m_iPort;		///< remote searchd port, 0 if local

public:
	AgentDesc_c ()
		: m_bBlackhole ( false )
		, m_uAddr ( 0 )
		, m_pStats ( nullptr )
		, m_pDash ( nullptr )
		, m_bPersistent ( false )
		, m_iFamily ( AF_INET )
		, m_iPort ( -1 )
	{
	}
	~AgentDesc_c ();

	void CloneTo ( AgentDesc_c & tOther ) const;

	CSphString GetMyUrl () const;
};

/// remote agent connection (local per-query state)
struct AgentConn_t : public ISphNoncopyable
{
	int				m_iSock;		///< socket number, -1 if not connected
	bool			m_bFresh;		///< just created persistent connection, need SEARCHD_COMMAND_PERSIST

	bool			m_bSuccess;		///< whether last request was successful (ie. there are available results)
	CSphString		m_sFailure;		///< failure message
	bool			m_bDone;		///< agent got processed, no need to retry

	int				m_iReplyStatus;	///< reply status code
	int				m_iReplySize;	///< how many reply bytes are there
	int				m_iReplyRead;	///< how many reply bytes are already received
	int 			m_iRetries;		///< count from 0 to m_iRetryLimit
	int 			m_iRetryLimit;	///< how many times retry (m.b. with another mirror)

	CSphFixedVector<BYTE>			m_dReplyBuf;
	CSphVector<CSphQueryResult>		m_dResults;		///< multi-query results

	int64_t			m_iWall;		///< wall time spent vs this agent
	int64_t			m_iWaited;		///< statistics of waited
	int64_t			m_iStartQuery;	///< the timestamp of the latest request
	int64_t			m_iEndQuery;	///< the timestamp of the end of the latest operation
	int				m_iWorkerTag;	///< worker tag
	int				m_iStoreTag;
	int				m_iWeight;
	bool			m_bPing;

	AgentDesc_c		m_tDesc;

private:
	CSphString      m_sDistIndex;
	int				m_iMirror;			// FIXME!!! got broken on agent removal
	int				m_iMirrorsCount;
	AgentState_e	m_eConnState;		///< current state

public:
	AgentConn_t ();
	~AgentConn_t ();

	void Close ( bool bClosePersist=true );
	void Fail ( AgentStats_e eStat, const char* sMessage, ... ) __attribute__ ( ( format ( printf, 3, 4 ) ) );

	void SetMirror ( const CSphString & sIndex, int iAgent, const AgentDesc_c & tDesc, int iMirrorsCount );
	void NextMirror();
	int GetMirrorsCount() const { return m_iMirrorsCount; }
	AgentState_e State() const { return m_eConnState; } 
	void State ( AgentState_e eState ); 

private:
	void SetupPersist();
};

typedef CSphVector<AgentConn_t *> AgentsVector;

template <typename T>
struct VectorPtrsGuard_T : public ISphNoncopyable
{
	CSphVector < T * > m_dPtrs;
	VectorPtrsGuard_T() {}
	~VectorPtrsGuard_T()
	{
		ARRAY_FOREACH ( i, m_dPtrs )
			SafeDelete ( m_dPtrs[i] );
	}
};


extern const char * sAgentStatsNames[eMaxAgentStat + ehMaxStat];

struct AgentDash_t : AgentStats_t
{
	uint64_t		m_dHostStats[ehMaxStat];
	DWORD			m_uTimestamp;	///< adds the minutes timestamp to AgentStats_t
	void Reset ()
	{
		AgentStats_t::Reset ();
		for ( int i = 0; i<ehMaxStat; ++i )
			m_dHostStats[i] = 0;
	}
	void Add ( const AgentDash_t& rhs )
	{
		AgentStats_t::Add ( rhs );

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

using HostStatSnapshot_t = uint64_t[eMaxAgentStat+ehMaxStat];

/// per-host dashboard
struct HostDashboard_t : public ISphRefcountedMT
{
	AgentDesc_c		m_tDescriptor;			// only host info, no indices. Used for ping.
	bool			m_bNeedPing;			// we'll ping only HA agents, not everyone

	mutable CSphRwlock	 m_dDataLock;	
	int64_t		m_iLastAnswerTime GUARDED_BY ( m_dDataLock );		// updated when we get an answer from the host
	int64_t		m_iLastQueryTime GUARDED_BY ( m_dDataLock );		// updated when we send a query to a host
	int64_t		m_iErrorsARow GUARDED_BY ( m_dDataLock );			// num of errors a row, updated when we update the general statistic.

	PersistentConnectionsPool_t*	m_pPersPool;					// persistence pool also lives here, one per dashboard

private:
	AgentDash_t	m_dStats[STATS_DASH_TIME] GUARDED_BY ( m_dDataLock );

public:
	explicit HostDashboard_t ( const AgentDesc_c & tAgent );
	~HostDashboard_t ();
	bool IsOlder ( int64_t iTime ) const REQUIRES_SHARED ( m_dDataLock );
	static DWORD GetCurSeconds();
	static bool IsHalfPeriodChanged ( DWORD * pLast );
	AgentDash_t*	GetCurrentStat() REQUIRES ( m_dDataLock );
	void GetCollectedStat ( HostStatSnapshot_t& dResult, int iPeriods=1 ) const EXCLUDES ( m_dDataLock );
};

struct AgentOptions_t
{
	bool m_bBlackhole;
	bool m_bPersistent;
	HAStrategies_e m_eStrategy;
};

struct WarnInfo_t;

/// descriptor for set of agents (mirrors) (stored in a global hash)
struct MultiAgentDesc_t : public ISphNoncopyable
{
private:
	CSphFixedVector<AgentDesc_c> m_dHosts;
	CSphAtomic				m_iRRCounter;	/// round-robin counter
	mutable CSphRwlock		m_dWeightLock;	/// manages access to m_pWeights
	CSphFixedVector<WORD>	m_dWeights		/// the weights of the hosts
			GUARDED_BY (m_dWeightLock);
	DWORD					m_uTimestamp;	/// timestamp of last weight's actualization
	HAStrategies_e			m_eStrategy;

public:
	MultiAgentDesc_t ()
		: m_dHosts ( 0 )
		, m_dWeights ( 0 )
		, m_uTimestamp ( HostDashboard_t::GetCurSeconds () )
		, m_eStrategy ( HA_DEFAULT )
	{
		m_dWeightLock.Init();
	}

	~MultiAgentDesc_t()
	{
		m_dWeightLock.Done();
	}

	bool SetHosts ( const CSphVector<AgentDesc_c *> & dHosts, WarnInfo_t & tWarning );
	const AgentDesc_c & GetAgent ( int iAgent ) const;
	AgentDesc_c & GetAgent ( int iAgent );
	const AgentDesc_c & ChooseAgent ();

	void SetOptions ( const AgentOptions_t& tOpt );
	void FinalizeInitialization () NO_THREAD_SAFETY_ANALYSIS;
	void QueuePings ();

	inline bool IsHA() const
	{
		return GetLength() > 1;
	}

	inline int GetLength() const
	{
		return m_dHosts.GetLength();
	}

	const CSphFixedVector<AgentDesc_c> & GetAgents() const
	{
		return m_dHosts;
	}

	CSphFixedVector<WORD> GetWeights () const EXCLUDES (m_dWeightLock)
	{
		CSphScopedRLock tRguard ( m_dWeightLock );
		CSphFixedVector<WORD> dResult ( m_dWeights.GetLength () );
		memcpy ( dResult.Begin (), m_dWeights.Begin (), m_dWeights.GetSizeBytes () );
		return dResult;
	}

private:

	const AgentDesc_c & RRAgent ();
	const AgentDesc_c & RandAgent ();
	const AgentDesc_c & StDiscardDead ();
	const AgentDesc_c & StLowErrors ();

	void ChooseWeightedRandAgent ( int * pBestAgent, CSphVector<int> & dCandidates ) EXCLUDES ( m_dWeightLock );
	void CheckRecalculateWeights ( const CSphFixedVector<int64_t> &dTimers ) EXCLUDES (m_dWeightLock);
	inline const HostDashboard_t& GetDashForAgent ( int iAgent ) const { return *m_dHosts[iAgent].m_pDash; }

};

struct SearchdStats_t
{
	DWORD		m_uStarted;
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


	SearchdStats_t ()
		: m_uStarted { 0 }
	{
	}
};

class cDashStorage : public ISphNoncopyable
{
	using cVector = CSphVector<HostDashboard_t*>;
	cVector							m_dDashes GUARDED_BY(m_tLock);	// stores atomic pointer to active tRefVector
	mutable CSphRwlock				m_tLock;

public:
	void				AddAgent ( AgentDesc_c * pAgent );
	HostDashboard_t *	FindAgent ( const char* sAgent ) const;
	void				GetActiveDashes ( CSphVector<HostDashboard_t *> & dAgents ) const;

	cDashStorage ()
	{
		m_tLock.Init();
	}

	~cDashStorage()
	{
		m_tLock.Done();
	}

	void Lock () { m_tLock.ReadLock(); }
	void Unlock () { m_tLock.Unlock(); }
};



extern SearchdStats_t			g_tStats;
extern cDashStorage				g_tDashes;


// for error-reporting
struct WarnInfo_t
{
	const char * m_szIndexName;
	const char * m_szAgent;
	WarnInfo_t ( const char * szIndexName, const char * szAgent )
		: m_szIndexName ( szIndexName )
		, m_szAgent ( szAgent )
	{}
};

bool ParseStrategyHA ( const char * sName, HAStrategies_e & eStrategy );

// try to parse hostname/ip/port or unixsocket on current pConfigLine.
// fill pAgent fields on success and move ppLine pointer next after parsed instance
// bUseDefaultPort allows to skip :port in the line (IANA 9312 will be used in the case)
bool ParseAddressPort ( AgentDesc_c * pAgent, const char ** ppLine, const WarnInfo_t& dInfo, bool bUseDefaultPort = false );

bool ConfigureAgent ( MultiAgentDesc_t & tAgent, const char * szAgent, const char * szIndexName, AgentOptions_t tDesc );

struct IRequestBuilder_t : public ISphNoncopyable
{
	virtual ~IRequestBuilder_t () {} // to avoid gcc4 warns
	virtual void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const = 0;
};


struct IReplyParser_t
{
	virtual ~IReplyParser_t () {} // to avoid gcc4 warns
	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const = 0;
};


struct AgentConnectionContext_t
{
	const IRequestBuilder_t * m_pBuilder;
	AgentConn_t	** m_ppAgents;
	int m_iAgentCount;
	int m_iTimeout;
	int m_iDelay;

	AgentConnectionContext_t ()
		: m_pBuilder ( NULL )
		, m_ppAgents ( NULL )
		, m_iAgentCount ( 0 )
		, m_iTimeout ( 0 )
		, m_iDelay ( 0 )
	{}
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
	virtual bool FetchReadyAgents () = 0;

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
	const void * 		m_pData;
	bool 				m_bReadable;
	bool 				m_bWritable;
	DWORD				m_uEvents;
	void Reset()
	{
		m_pData = nullptr;
		m_uEvents = 0;
		m_bReadable = false;
		m_bWritable = false;
	}
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
