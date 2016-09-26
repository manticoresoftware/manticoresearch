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


const int	STATS_MAX_AGENTS	= 8192;				///< we'll track stats for this much remote agents
const int	STATS_MAX_DASH	= STATS_MAX_AGENTS / 4;	///< we'll track stats for RR of this much remote agents
const int	STATS_DASH_TIME = 15;	///< store the history for last periods

/////////////////////////////////////////////////////////////////////////////
// MISC GLOBALS
/////////////////////////////////////////////////////////////////////////////

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

/// per-agent query stats
enum AgentStats_e
{
	eTimeoutsQuery = 0,	///< number of time-outed queries
	eTimeoutsConnect,	///< number of time-outed connections
	eConnectFailures,	///< failed to connect
	eNetworkErrors,		///< network error
	eWrongReplies,		///< incomplete reply
	eUnexpectedClose,	///< agent closed the connection
	eWarnings,			///< agent answered, but with warnings
	eNetworkCritical = eWarnings,
	eNoErrors,			///< successful queries, no errors
	eNetworkNonCritical = eNoErrors,
	eTotalMsecs,		///< number of microseconds in queries, total
	eMaxCounters = eTotalMsecs,
	eConnTries,		///< total number of connec tries
	eAverageMsecs,		///< average connect time
	eMaxMsecs,			///< maximal connect time
	eMaxStat
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

// manages persistent connections to agents
class RentPersistent_c
{
	int				m_iPersPool;

public:
	explicit RentPersistent_c ( int iPersPool )
		: m_iPersPool ( iPersPool )
	{}

	void Init ( int iPersPool )
	{
		m_iPersPool = iPersPool;
	}

	int RentConnection ();
	void ReturnConnection ( int iSocket );
};

void ResizePersistentConnectionsPool ( int iSize );
void ClosePersistentSockets();

// generic descriptor of remote host (agent)
class AgentDesc_c
{
public:
	CSphString		m_sHost;		///< remote searchd host
	int				m_iPort;		///< remote searchd port, 0 if local
	CSphString		m_sPath;		///< local searchd UNIX socket path
	CSphString		m_sIndexes;		///< remote index names to query
	bool			m_bBlackhole;	///< blackhole agent flag
	int				m_iFamily;		///< TCP or UNIX socket
	DWORD			m_uAddr;		///< IP address
	int				m_iStatsIndex;	///< index into global searchd stats array
	int				m_iDashIndex;	///< index into global searchd host stats array (1 host can hold >1 agents)
	bool			m_bPersistent;	///< whether to keep the persistent connection to the agent.
	RentPersistent_c	m_dPersPool;	///< socket number, -1 if not connected (has sense only if the connection is persistent)

public:
	AgentDesc_c ()
		: m_iPort ( -1 )
		, m_bBlackhole ( false )
		, m_iFamily ( AF_INET )
		, m_uAddr ( 0 )
		, m_iStatsIndex ( -1 )
		, m_iDashIndex ( 0 )
		, m_bPersistent ( false )
		, m_dPersPool ( -1 )
	{}

	CSphString GetMyUrl() const;
};

struct MetaAgentDesc_t;
/// remote agent connection (local per-query state)
struct AgentConn_t : public AgentDesc_c
{
	int				m_iSock;		///< socket number, -1 if not connected
	bool			m_bFresh;		///< just created persistent connection, need SEARCHD_COMMAND_PERSIST
	AgentState_e	m_eState;		///< current state

	bool			m_bSuccess;		///< whether last request was successful (ie. there are available results)
	CSphString		m_sFailure;		///< failure message

	int				m_iReplyStatus;	///< reply status code
	int				m_iReplySize;	///< how many reply bytes are there
	int				m_iReplyRead;	///< how many reply bytes are alredy received
	BYTE *			m_pReplyBuf;	///< reply buffer

	CSphVector<CSphQueryResult>		m_dResults;		///< multi-query results

	int64_t			m_iWall;		///< wall time spent vs this agent
	int64_t			m_iWaited;		///< statistics of waited
	int64_t			m_iStartQuery;	///< the timestamp of the latest request
	int64_t			m_iEndQuery;	///< the timestamp of the end of the latest operation
	int				m_iWorkerTag;	///< worker tag
	int				m_iStoreTag;
	int				m_iWeight;
	bool			m_bPing;
	MetaAgentDesc_t*	m_pMirrorChooser; ///< used to select another mirror if this one is broken

public:
	AgentConn_t ();
	~AgentConn_t ();

	void Close ( bool bClosePersist=true );
	void Fail ( AgentStats_e eStat, const char* sMessage, ... ) __attribute__ ( ( format ( printf, 3, 4 ) ) );
	AgentConn_t & operator = ( const AgentDesc_c & rhs );

	// works like =, but also adopt the persistent connection, if any.
	void SpecifyAndSelectMirror ( MetaAgentDesc_t * pMirrorChooser = nullptr );
	int NumOfMirrors () const;
};

void agent_stats_inc ( AgentConn_t & tAgent, AgentStats_e iCounter );

struct AgentStats_t
{
	uint64_t		m_iStats[eMaxStat];
	static const char * m_sNames[eMaxStat];

	void Reset()
	{
		for ( int i=0; i<eMaxStat; ++i )
			m_iStats[i] = 0;
	}
	void Add ( const AgentStats_t& rhs )
	{
		for ( int i=0; i<=eMaxCounters; ++i )
			m_iStats[i] += rhs.m_iStats[i];

		if ( m_iStats[eConnTries] )
			m_iStats[eAverageMsecs] =
				( m_iStats[eAverageMsecs] * m_iStats[eConnTries] + rhs.m_iStats[eAverageMsecs] * rhs.m_iStats[eConnTries] )
				/ (m_iStats[eConnTries] + rhs.m_iStats[eConnTries] );
		else
			m_iStats[eAverageMsecs] = rhs.m_iStats[eAverageMsecs];
		m_iStats[eMaxMsecs] = Max ( m_iStats[eMaxMsecs], rhs.m_iStats[eMaxMsecs] );
		m_iStats[eConnTries] += rhs.m_iStats[eConnTries];
	}
};

struct AgentDash_t : AgentStats_t
{
	DWORD			m_uTimestamp;	///< adds the minutes timestamp to AgentStats_t
};


/// per-host dashboard
struct HostDashboard_t
{
	int				m_iRefCount;			// several indexes can use this one agent
	int64_t			m_iLastAnswerTime;		// updated when we get an answer from the host
	int64_t			m_iLastQueryTime;		// updated when we send a query to a host
	int64_t			m_iErrorsARow;			// num of errors a row, updated when we update the general statistic.
	AgentDesc_c		m_tDescriptor;			// only host info, no indices. Used for ping.
	bool			m_bNeedPing;			// we'll ping only HA agents, not everyone

private:
	AgentDash_t	m_dStats[STATS_DASH_TIME];

public:
	void Init ( const AgentDesc_c * pAgent );
	bool IsOlder ( int64_t iTime ) const;
	static DWORD GetCurSeconds();
	static bool IsHalfPeriodChanged ( DWORD * pLast );
	AgentDash_t*	GetCurrentStat();
	AgentDash_t GetCollectedStat ( int iPeriods=1 ) const;
};

template <class DATA, int SIZE> class StaticStorage_T : public ISphNoncopyable
{
	DWORD			m_bmItemStats[SIZE/32];	///< per-item storage usage bitmap
public:
	DATA			m_dItemStats[SIZE];		///< per-item storage
public:
	explicit StaticStorage_T()
	{
		for ( int i=0; i<SIZE/32; ++i )
			m_bmItemStats[i] = 0;
	}
	int AllocItem()
	{
		int iRes = -1;
		for ( int i=0; i<SIZE/32; i++ )
			if ( m_bmItemStats[i]!=0xffffffffUL )
			{
				int j = FindBit ( m_bmItemStats[i] );
				m_bmItemStats[i] |= ( 1<<j );
				iRes = i*32 + j;
				memset ( &m_dItemStats[iRes], 0, sizeof(DATA) );
				break;
			}
			return iRes;
	}
	void FreeItem ( int iItem )
	{
		if ( iItem<0 || iItem>=SIZE )
			return;

		assert ( m_bmItemStats[iItem>>5] & ( 1UL<<( iItem & 31 ) ) );
		m_bmItemStats[iItem>>5] &= ~( 1UL<<( iItem & 31 ) );
	}
};

typedef StaticStorage_T<HostDashboard_t,STATS_MAX_DASH> DashBoard_t;

struct AgentOptions_t
{
	bool m_bBlackhole;
	bool m_bPersistent;
	HAStrategies_e m_eStrategy;
};

/// descriptor for set of agents (mirrors) (stored in a global hash)
/// remote agent descriptor (stored in a global hash)
struct MetaAgentDesc_t
{
private:
	CSphVector<AgentDesc_c> m_dHosts;
	CSphAtomic				m_iRRCounter;	/// round-robin counter
	mutable CSphManagedRwlock		m_dWeightLock;	/// manages access to m_pWeights
	CSphFixedVector<WORD>	m_dWeights;		/// the weights of the hosts
	DWORD					m_uTimestamp;	/// timestamp of last weight's actualization
	HAStrategies_e			m_eStrategy;

public:
	MetaAgentDesc_t ()
		: m_dWeights {0}
		, m_uTimestamp { HostDashboard_t::GetCurSeconds () }
		, m_eStrategy { HA_DEFAULT }
	{
	}

	MetaAgentDesc_t ( const MetaAgentDesc_t & rhs )
		: m_dWeights(0)
	{
		*this = rhs;
	}

	MetaAgentDesc_t & operator= ( const MetaAgentDesc_t & rhs );

	AgentDesc_c * NewAgent ();
	AgentDesc_c * GetAgent ( int iAgent=0 );
	const AgentDesc_c & ChooseAgent ();

	void SetOptions ( const AgentOptions_t& tOpt );
	void FinalizeInitialization ();
	void QueuePings ();

	inline bool IsHA() const
	{
		return GetLength() > 1;
	}

	inline int GetLength() const
	{
		return m_dHosts.GetLength();
	}

	inline bool IsInitFinished() const
	{
		return m_dHosts.GetLength () == m_dWeights.GetLength ();
	}

	const CSphVector<AgentDesc_c>& GetAgents() const
	{
		return m_dHosts;
	}

	CSphFixedVector<WORD> GetWeights () const
	{
		CSphFixedVector<WORD> dResult ( m_dWeights.GetLength () );
		CSphScopedRWLock tLock ( m_dWeightLock, true );
		memcpy ( dResult.Begin (), m_dWeights.Begin (), m_dWeights.GetSizeBytes () );
		return dResult;
	}

private:

	AgentDesc_c * RRAgent ();
	AgentDesc_c * RandAgent ();
	AgentDesc_c * StDiscardDead ();
	AgentDesc_c * StLowErrors ();

	void ChooseWeightedRandAgent ( int * pBestAgent, CSphVector<int> & dCandidates );
	void CheckRecalculateWeights ( const CSphFixedVector<int64_t> &dTimers );
	const HostDashboard_t& GetDashForAgent ( int iAgent ) const;
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

	int64_t		m_iPredictedTime;	///< total agent predicted query time
	int64_t		m_iAgentPredictedTime;	///< total agent predicted query time

	// FIXME!!! all these are still not thread safe (read at CheckPing vs modify at ValidateAgentDesc)
	// and not concurent friendly due to g_tStatsMutex ( read at BuildOneAgentStatus might block often write at agent_stats_inc )
	StaticStorage_T<AgentStats_t,STATS_MAX_AGENTS> m_dAgentStats;
	StaticStorage_T<HostDashboard_t,STATS_MAX_DASH> m_dDashboard;
	SmallStringHash_T<int>							m_hDashBoard; ///< find hosts for agents and sort them all

	SearchdStats_t ()
	{
		m_uStarted = 0;
		m_iConnections = 0;
		m_iMaxedOut = 0;
		memset ( m_iCommandCount, 0, sizeof ( m_iCommandCount ) );
		m_iAgentConnect = 0;
		m_iAgentRetry = 0;
		m_iQueries = 0;
		m_iQueryTime = 0;
		m_iQueryCpuTime = 0;
		m_iDistQueries = 0;
		m_iDistWallTime = 0;
		m_iDistLocalTime = 0;
		m_iDistWaitTime = 0;
		m_iDiskReads = 0;
		m_iDiskReadBytes = 0;
		m_iDiskReadTime = 0;
		m_iPredictedTime = 0;
		m_iAgentPredictedTime = 0;
	}
};


extern SearchdStats_t			g_tStats;
extern CSphMutex				g_tStatsMutex;

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
bool ParseAddressPort ( AgentDesc_c * pAgent, const char ** ppLine, const WarnInfo_t& dInfo );

bool ConfigureAgent ( MetaAgentDesc_t & tAgent, const CSphVariant * pAgent, const char * szIndexName, AgentOptions_t tDesc );

void FreeAgentStats ( const AgentDesc_c& dAgent );
void FreeDashboard ( const AgentDesc_c&, DashBoard_t&, SmallStringHash_T<int>& );
void FreeHostStats ( MetaAgentDesc_t & tHosts );

struct IRequestBuilder_t : public ISphNoncopyable
{
	virtual ~IRequestBuilder_t () {} // to avoid gcc4 warns
	virtual void BuildRequest ( AgentConn_t & tAgent, NetOutputBuffer_c & tOut ) const = 0;
};


struct IReplyParser_t
{
	virtual ~IReplyParser_t () {} // to avoid gcc4 warns
	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const = 0;
};


struct AgentConnectionContext_t
{
	const IRequestBuilder_t * m_pBuilder;
	AgentConn_t	* m_pAgents;
	int m_iAgentCount;
	int m_iTimeout;
	int m_iRetriesMax;
	int m_iDelay;

	AgentConnectionContext_t ()
		: m_pBuilder ( NULL )
		, m_pAgents ( NULL )
		, m_iAgentCount ( 0 )
		, m_iTimeout ( 0 )
		, m_iRetriesMax ( 0 )
		, m_iDelay ( 0 )
	{}
};

void RemoteConnectToAgent ( AgentConn_t & tAgent );
int RemoteQueryAgents ( AgentConnectionContext_t * pCtx );

// processing states AGENT_QUERY, AGENT_PREREPLY and AGENT_REPLY
// may work in parallel with RemoteQueryAgents, so the state MAY change during a call.
int RemoteWaitForAgents ( CSphVector<AgentConn_t> & dAgents, int iTimeout, IReplyParser_t & tParser );


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

ISphRemoteAgentsController* GetAgentsController ( int iThreads, CSphVector<AgentConn_t> & dAgents,
		const IRequestBuilder_t & tBuilder, int iTimeout, int iRetryMax=0, int iDelay=0 );


#endif // _searchdha_

//
// $Id$
//
