//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

#if _WIN32
bool LoadExFunctions ();
#endif

#include "sphinxutils.h"
#include "searchdaemon.h"
#include "timeout_queue.h"

/////////////////////////////////////////////////////////////////////////////
// SOME SHARED GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////

extern int				g_iReadTimeoutS; // defined in searchd.cpp

extern int64_t			g_iPingIntervalUs;
extern DWORD			g_uHAPeriodKarmaS;		// by default use the last 1 minute statistic to determine the best HA agent
extern int				g_iPersistentPoolSize;

extern int				g_iAgentConnectTimeoutMs;
extern int				g_iAgentQueryTimeoutMs;	// global (default). May be override by index-scope values, if one specified
extern bool				g_bHostnameLookup;

const int	STATS_DASH_PERIODS = 15;	///< store the history for last periods

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
enum class Agent_e
{
	HEALTHY,			///< agent is in normal state
	CONNECTING,		///< connecting to agent in progress, write handshake on socket ready
	RETRY,			///< should retry, but after scheduled timeout
};

const char * Agent_e_Name ( Agent_e eState );

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

enum HAStrategies_e
{
	HA_RANDOM,
	HA_ROUNDROBIN,
	HA_AVOIDDEAD,
	HA_AVOIDERRORS,

	HA_DEFAULT = HA_RANDOM
};

// manages persistent connections to a host
// serves a FIFO queue.
// I.e. if we have 2 connections to a host, and one task rent the connection,
// we will return 1-st socket. And the next rent request will definitely 2-nd socket
// whenever 1-st socket already released or not.
// (previous code used LIFO strategy)
class PersistentConnectionsPool_c
{
	mutable CSphMutex	 m_dDataLock;
	bool			m_bShutdown = false;			// will cause ReturnConnection to close the socket instead of returning it
	CSphTightVector<int>	m_dSockets GUARDED_BY ( m_dDataLock );
	int				m_iRit GUARDED_BY ( m_dDataLock ) = 0; // pos where we take the next socket to rent.
	int				m_iWit GUARDED_BY ( m_dDataLock ) = 0; // pos where we will put returned socket.
	int				m_iFreeWindow GUARDED_BY ( m_dDataLock ) = 0; // # of free sockets in the existing ring
	int				m_iLimit GUARDED_BY ( m_dDataLock ) = 0; // exact limit (embedded vector's limit is not exact)

	int Step ( int* ) REQUIRES ( m_dDataLock ); // step over the ring

public:
	~PersistentConnectionsPool_c ()	{ Shutdown (); }
	void	ReInit ( int iPoolSize ) REQUIRES ( !m_dDataLock );
	int		RentConnection () REQUIRES ( !m_dDataLock );
	void	ReturnConnection ( int iSocket ) REQUIRES ( !m_dDataLock );
	void	Shutdown () REQUIRES ( !m_dDataLock );
};

void ClosePersistentSockets();

struct MetricsAndCounters_t final : ISphRefcountedMT
{
	// was uint64_t, but for atomic it creates extra tmpl instantiation without practical difference
	std::atomic<int64_t> m_dCounters[eMaxAgentStat];	// event counters
	uint64_t m_dMetrics[ehMaxStat];			// calculated metrics

	MetricsAndCounters_t ()
	{
		Reset();
	}

	void Reset ()
	{
		for ( auto &iCounter : m_dCounters )
			iCounter = 0;
		for ( auto &uMetric : m_dMetrics )
			uMetric = 0;
	}

	void Add ( const MetricsAndCounters_t &rhs )
	{
		for ( int i = 0; i<eMaxAgentStat; ++i )
			m_dCounters[i].fetch_add ( rhs.m_dCounters[i].load ( std::memory_order_relaxed ), std::memory_order_relaxed );

		if ( m_dMetrics[ehConnTries] )
			m_dMetrics[ehAverageMsecs] =
				( m_dMetrics[ehAverageMsecs] * m_dMetrics[ehConnTries]
					+ rhs.m_dMetrics[ehAverageMsecs] * rhs.m_dMetrics[ehConnTries] )
					/ ( m_dMetrics[ehConnTries] + rhs.m_dMetrics[ehConnTries] );
		else
			m_dMetrics[ehAverageMsecs] = rhs.m_dMetrics[ehAverageMsecs];
		m_dMetrics[ehMaxMsecs] = Max ( m_dMetrics[ehMaxMsecs], rhs.m_dMetrics[ehMaxMsecs] );
		m_dMetrics[ehConnTries] += rhs.m_dMetrics[ehConnTries];
	}
private:
	~MetricsAndCounters_t () final = default;
	friend struct HostDashboard_t; // the only struct allowed to directly declare/destroy me.
};

using MetricsAndCountersRefPtr_t = CSphRefcountedPtr<MetricsAndCounters_t>;

struct HostDashboard_t;
using HostDashboardRefPtr_t = CSphRefcountedPtr<HostDashboard_t>;
using cHostDashboardRefPtr_t = CSphRefcountedPtr<const HostDashboard_t>;

/// generic descriptor of remote host
struct HostDesc_t : ISphNoncopyable
{
	int m_iFamily = AF_INET;	///< TCP or UNIX socket
	CSphString m_sAddr;			///< remote searchd host (used to update m_uAddr with resolver)
	int m_iPort = -1;			///< remote searchd port, 0 if local
	DWORD m_uAddr = 0;			///< IP address
	bool m_bNeedResolve = false;	///< whether we keep m_uAddr, or call GetAddrInfo each time.

	bool m_bBlackhole = false;	///< blackhole agent flag
	bool m_bPersistent = false;	///< whether to keep the persistent connection to the agent.

	mutable HostDashboardRefPtr_t m_pDash;	///< ha dashboard of the host

	HostDesc_t &CloneFromHost ( const HostDesc_t &tOther );
	CSphString GetMyUrl () const;
};

/// generic descriptor of the mirror: host + indexes.
struct AgentDesc_t : HostDesc_t
{
	CSphString			m_sIndexes;		///< remote index names to query
	mutable MetricsAndCountersRefPtr_t m_pMetrics;	///< source for ShowStatus (one copy shared over all clones, refcounted).
	AgentDesc_t &CloneFrom ( const AgentDesc_t &tOther );
};

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


extern const char * sAgentStatsNames[(int)eMaxAgentStat + (int)ehMaxStat];
using HostMetricsSnapshot_t = uint64_t[(int)eMaxAgentStat + (int)ehMaxStat];

/// per-host dashboard
struct HostDashboard_t : public ISphRefcountedMT
{
	HostDesc_t m_tHost;          // only host info, no indices. Used for ping.
	volatile int m_iNeedPing = 0;    // we'll ping only HA agents, not everyone
	PersistentConnectionsPool_c * m_pPersPool = nullptr;    // persistence pool also lives here, one per dashboard

	mutable RwLock_t m_dMetricsLock;        // guards everything essential (see thread annotations)
	int64_t m_iLastAnswerTime GUARDED_BY ( m_dMetricsLock );    // updated when we get an answer from the host
	int64_t m_iLastQueryTime GUARDED_BY ( m_dMetricsLock ) = 0;    // updated when we send a query to a host
	int64_t m_iErrorsARow GUARDED_BY ( m_dMetricsLock ) = 0;        // num of errors a row, updated when we update the general statistic.
	DWORD m_uPingTripUS = 0;		// round-trip in uS. We send ping with current time, on receive answer compare with current time and fix that difference

public:
	explicit HostDashboard_t ( const HostDesc_t &tAgent );
	int64_t EngageTime () const;
	MetricsAndCounters_t &GetCurrentMetrics () REQUIRES ( m_dMetricsLock );
	void GetCollectedMetrics ( HostMetricsSnapshot_t &dResult, int iPeriods = 1 ) const REQUIRES ( !m_dMetricsLock );

	static DWORD GetCurSeconds ();
	static bool IsHalfPeriodChanged ( DWORD * pLast );

private:
	struct
	{
		MetricsAndCounters_t m_dMetrics;
		DWORD m_uPeriod = 0xFFFFFFFF;
	} m_dPeriodicMetrics[STATS_DASH_PERIODS] GUARDED_BY ( m_dMetricsLock );

	~HostDashboard_t() override;
};

class IPinger
{
public:
	virtual void Subscribe ( HostDashboardRefPtr_t pHost ) = 0;
	virtual ~IPinger() {}
};

void SetGlobalPinger ( IPinger* pPinger );


/// context which keeps name of the index and agent
/// (mainly for error-reporting)
class WarnInfo_c
{
public:
	const char *	m_szIndexName = nullptr;
	const char *	m_szAgent = nullptr;

	WarnInfo_c ( const char * szIndexName, const char *	szAgent, StrVec_t * pWarnings=nullptr )
		: m_szIndexName ( szIndexName )
		, m_szAgent ( szAgent )
		, m_pWarnings ( pWarnings )
	{}

	void Warn ( const char * szFmt, ... ) const
	{
		va_list ap;
		va_start ( ap, szFmt );

		CSphString sWarning;
		if ( m_szIndexName )
			sWarning.SetSprintf ( "table '%s': agent '%s': %s", m_szIndexName, m_szAgent, szFmt );
		else
			sWarning.SetSprintf ( "host '%s': %s", m_szAgent, szFmt );

		sWarning.SetSprintfVa ( sWarning.cstr(), ap );
		sphInfo ( "%s", sWarning.cstr() );

		if ( m_pWarnings )
			m_pWarnings->Add(sWarning);

		va_end ( ap );
	}

	/// format an error message using idx and agent names from own context
	/// \return always false, to simplify statements
	bool ErrSkip ( const char * szFmt, ... ) const
	{
		va_list ap;
		va_start ( ap, szFmt );

		CSphString sWarning;

		if ( m_szIndexName )
			sWarning.SetSprintf ( "table '%s': agent '%s': %s, - SKIPPING AGENT", m_szIndexName, m_szAgent, szFmt );
		else
			sWarning.SetSprintf ( "host '%s': %s, - SKIPPING AGENT", m_szAgent, szFmt );

		sWarning.SetSprintfVa ( sWarning.cstr(), ap );
		sphWarning ( "%s", sWarning.cstr() );

		if ( m_pWarnings )
			m_pWarnings->Add(sWarning);

		va_end ( ap );
		return false;
	}

private:
	StrVec_t *		m_pWarnings = nullptr;
};

/// descriptor for set of agents (mirrors) (stored in a global hash)
class MultiAgentDesc_c;
using MultiAgentDescRefPtr_c = CSphRefcountedPtr<MultiAgentDesc_c>;
using cMultiAgentDescRefPtr_c = CSphRefcountedPtr<const MultiAgentDesc_c>;
class MultiAgentDesc_c final : public ISphRefcountedMT, public CSphFixedVector<AgentDesc_t>
{
	std::atomic<int>	m_iRRCounter {0};    /// round-robin counter
	mutable RwLock_t	m_dWeightLock;   /// manages access to m_pWeights
	CSphFixedVector<float> m_dWeights    /// the weights of the hosts
		GUARDED_BY ( m_dWeightLock ) { 0 };
	DWORD				m_uTimestamp { HostDashboard_t::GetCurSeconds () };    /// timestamp of last weight's actualization
	HAStrategies_e		m_eStrategy { HA_DEFAULT };
	int					m_iMultiRetryCount = 0;
	bool 				m_bNeedPing = false;	/// ping need to hosts if we're HA and NOT bl.
	CSphString			m_sConfigStr;	/// agent configuration string, straight from .conf

	~MultiAgentDesc_c () final;

public:
	MultiAgentDesc_c ()
		: CSphFixedVector<AgentDesc_t> { 0 }
	{}

	// configure using dTemplateHosts as source of urls/indexes
	static MultiAgentDescRefPtr_c GetAgent ( const CSphVector<AgentDesc_t *> & dTemplateHosts, const AgentOptions_t & tOpt, const WarnInfo_c & tWarn );

	// housekeeping: walk throw global hash and finally release all 1-refs agents
	static void CleanupOrphaned();

	const AgentDesc_t & ChooseAgent() REQUIRES ( !m_dWeightLock );

	inline bool IsHA () const
	{
		return GetLength ()>1;
	}

	inline int GetRetryLimit () const
	{
		return m_iMultiRetryCount;
	}

	const CSphString & GetConfigStr() const { return m_sConfigStr; }

	CSphFixedVector<float> GetWeights () const REQUIRES ( !m_dWeightLock )
	{
		ScRL_t tRguard ( m_dWeightLock );
		CSphFixedVector<float> dResult { 0 };
		dResult.CopyFrom ( m_dWeights );
		return dResult;
	}

private:

	const AgentDesc_t &RRAgent ();
	const AgentDesc_t &RandAgent ();
	const AgentDesc_t &StDiscardDead () REQUIRES ( !m_dWeightLock );
	const AgentDesc_t &StLowErrors () REQUIRES ( !m_dWeightLock );

	void ChooseWeightedRandAgent ( int * pBestAgent, CSphVector<int> &dCandidates ) REQUIRES ( !m_dWeightLock );
	void CheckRecalculateWeights ( const CSphFixedVector<int64_t> &dTimers ) REQUIRES ( !m_dWeightLock );
	static CSphString GetKey( const CSphVector<AgentDesc_t *> &dTemplateHosts, const AgentOptions_t &tOpt );
	bool Init ( const CSphVector<AgentDesc_t *> &dTemplateHosts, const AgentOptions_t &tOpt, const WarnInfo_c & tWarn );
};

extern int g_iAgentRetryCount;
extern int g_iAgentRetryDelayMs;

class Reporter_i : public ISphRefcountedMT
{
public:
	// called by netloop - initially feeds reporter with tasks
	// For every task just before start querying it calls FeedTask(true).
	// If task is not to be traced (blackhole), it then calls FeedTask(false).
	virtual void FeedTask ( bool bAdd ) = 0;

	// called by netloop - when one of the task is finished (and tells, success or not). Good point for callback!
	// false returned in case of permanent error (dead; retries limit exceeded) and when aborting due to shutdown.
	virtual void Report ( bool bSuccess ) = 0;

	// called by outline observer, or by netloop checking for orphaned
	// must return 'true' if reporter is abandoned - i.e. if all expected connections are finished.
	virtual bool IsDone () const = 0;
};

#if _WIN32
	struct SingleOverlapped_t : public OVERLAPPED
	{
		ULONG_PTR	m_uParentOffset; // how many bytes add to this to take pointer to parent
		volatile bool		m_bInUse = false;
		inline void Zero ()
		{
			ZeroMemory ( this, sizeof ( OVERLAPPED ) );
		}
	};
	struct DoubleOverlapped_t
	{
		SingleOverlapped_t					m_dWrite;
		SingleOverlapped_t					m_dRead;
		CSphFixedVector<BYTE>				m_dReadBuf { 0 };	// used for canceling recv operation
		CSphVector<ISphOutputBuffer *>		m_dWriteBuf;		// used for canceling send operation
		CSphVector<sphIovec>				m_dOutIO;			// used for canceling send operation

		inline bool IsInUse()
		{
			return m_dWrite.m_bInUse || m_dRead.m_bInUse;
		}

		DoubleOverlapped_t()
		{
			m_dWrite.Zero();
			m_dRead.Zero();
			m_dWrite.m_uParentOffset = (LPBYTE) &m_dWrite-(LPBYTE) this;
			m_dRead.m_uParentOffset = (LPBYTE) &m_dRead-(LPBYTE) this;
		}

		~DoubleOverlapped_t()
		{
			for ( auto* pWriteBuf : m_dWriteBuf )
				SafeDelete (pWriteBuf);
		}
	};
	using LPKEY = DoubleOverlapped_t *;
#else
	using LPKEY = void *;
#endif

class IOVec_c
{
	CSphVector<sphIovec> m_dIOVec;
	size_t m_iIOChunks = 0;

public:

	void BuildFrom ( const SmartOutputBuffer_t& tSource ); /// take data from linked source
	void Reset ();

	/// consume received chunk
	void StepForward ( size_t uStep );

	inline bool HasUnsent () const noexcept
	{
		return m_iIOChunks!=0;
	}

	/// if we have data (despite it is sent or not)
	inline bool IsEmpty () const noexcept
	{
		return m_dIOVec.IsEmpty ();
	}

	/// buf for sendmsg/WSAsend
	inline sphIovec * IOPtr () const noexcept
	{
		if ( !m_iIOChunks )
			return nullptr;
		return m_dIOVec.end () - m_iIOChunks;
	}

	/// num of io vecs for sendmsg/WSAsend
	inline size_t IOSize () const noexcept
	{
		return m_iIOChunks;
	}

#if _WIN32
	inline void LeakTo ( CSphVector<sphIovec>& dIOVec )
	{
		m_dIOVec.SwapData ( dIOVec );
	}
#endif
};

struct iQueryResult
{
	virtual ~iQueryResult() {}
	virtual void Reset() = 0;
	virtual bool HasWarnings() const = 0;
};

/// remote agent connection (local per-query state)
struct AgentConn_t : public ISphRefcountedMT
{
	enum ETimeoutKind { TIMEOUT_UNKNOWN, TIMEOUT_RETRY, TIMEOUT_HARD, };
public:
	AgentDesc_t		m_tDesc;			///< desc of my host // fixme! turn to ref to MultiAgent mirror?
	int				m_iSock = -1;

	// time-tracking and timeout settings
	int				m_iMyConnectTimeoutMs { g_iAgentConnectTimeoutMs };	///< populated from parent distr
	int64_t			m_iMyQueryTimeoutMs { g_iAgentQueryTimeoutMs };		///< in msec
	int64_t			m_iStartQuery = 0;	///< the timestamp of the latest request // actualized
	int64_t			m_iEndQuery = 0;	///< the timestamp of the end of the latest operation // actual
	int64_t			m_iWall = 0;		///< wall time spent vs this agent // actualized
	int64_t			m_iWaited = 0;		///< statistics of waited

	// some external stuff
	std::unique_ptr<iQueryResult> m_pResult;	///< multi-query results
	CSphString		m_sFailure;				///< failure message (both network and logical)
	mutable int		m_iStoreTag = -1;	///< cookie, m.b. used to 'glue' to concrete connection
	int				m_iWeight = -1;		///< weight of the index, will be send with query to remote host

	CSphRefcountedPtr<Reporter_i>	m_pReporter { nullptr };	///< used to report back when we're finished
	LPKEY			m_pPollerTask = nullptr; ///< internal for poller. fixme! privatize?
	volatile bool	m_bSuccess {false};	///< agent got processed, no need to retry

public:
	AgentConn_t () = default;

	void SetMultiAgent ( MultiAgentDescRefPtr_c pMirror );
	inline bool IsBlackhole () const { return m_tDesc.m_bBlackhole; }
	inline bool InNetLoop() const { return m_bInNetLoop; }
	inline void SetNetLoop ( bool bInNetLoop = true ) { m_bInNetLoop = bInNetLoop; }
	inline bool FireKick () { bool bRes = m_bNeedKick; m_bNeedKick = false; return bRes; }

	void GenericInit ( RequestBuilder_i * pQuery, ReplyParser_i * pParser, Reporter_i * pReporter, int iQueryRetry, int iQueryDelay );
	void StartRemoteLoopTry ();

	void ErrorCallback ( int64_t iWaited );
	void SendCallback ( int64_t iWaited, DWORD uSent );
	void RecvCallback ( int64_t iWaited, DWORD uReceived );
	void TimeoutCallback ();
	void AbortCallback();
	bool CheckOrphaned();
	void SetNoLimitReplySize();

#if _WIN32
	// move recv buffer to dOut, reinit mine.
	void LeakRecvTo ( CSphFixedVector<BYTE>& dOut );
	void LeakSendTo ( CSphVector <ISphOutputBuffer* >& dOut, CSphVector<sphIovec>& dOutIO );
#endif

	// helper for beautiful logging
	inline const char * StateName () const 	{ return Agent_e_Name ( m_eConnState ); }

private:

	// prepare buf, parse result
	RequestBuilder_i * m_pBuilder = nullptr; ///< fixme! check if it is ok to have as the member, or we don't need it actually
	ReplyParser_i *	m_pParser = nullptr;

	// working with mirrors
	MultiAgentDescRefPtr_c m_pMultiAgent { nullptr }; ///< my manager, could turn me into another mirror
	int			m_iRetries = 0;						///< initialized to max num of tries. 0 mean 1 try, no re-tries.
	int			m_iMirrorsCount = 1;
	int			m_iDelay { g_iAgentRetryDelayMs };	///< delay between retries

	// active timeout (directly used by poller)
	int64_t			m_iPoolerTimeoutUS = -1;	///< m.b. query, or connect+query when TCP_FASTOPEN
	ETimeoutKind 	m_eTimeoutKind { TIMEOUT_UNKNOWN };

	// receiving buffer stuff
	CSphFixedVector<BYTE>	m_dReplyBuf { 0 };
	int						m_iReplySize = -1;    ///< how many reply bytes are there
	static const size_t	REPLY_HEADER_SIZE = 12;
	CSphFixedVector<BYTE>	m_dReplyHeader { REPLY_HEADER_SIZE };
	BYTE *					m_pReplyCur = nullptr;
	bool					m_bReplyLimitSize = true;

	// sending buffer stuff
	SmartOutputBuffer_t m_tOutput;		///< chain of blobs we're sending to a host
	IOVec_c 			m_dIOVec;

	// states and flags
	bool m_bConnectHandshake = false;	///< if we need to establish new connection, and so, wait back handshake version
	bool m_bInNetLoop	= false;		///< if we're inside netloop (1-thread work with schedule)
	bool m_bNeedKick	= false;		///< if we've installed callback from outside th and need to kick netloop
	bool m_bManyTries = false;			///< to avoid report 'retries limit esceeded' if we have ONLY one retry

	Agent_e			m_eConnState { Agent_e::HEALTHY };	///< current state
	SearchdStatus_e m_eReplyStatus { SEARCHD_ERROR };    ///< reply status code

private:
	~AgentConn_t () override;

	// switch/check internal state
	inline bool StateIs ( Agent_e eState ) const { return eState==m_eConnState; }
	void State ( Agent_e eState );

	bool IsPersistent();
	void ReturnPersist ();

	bool Fail ( const char * sFmt, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
	bool Fatal ( AgentStats_e eStat, const char * sMessage, ... ) __attribute__ ( ( format ( printf, 3, 4 ) ) );
	void Finish ( bool bFailed = false ); /// finish the task, stat time.
	bool BadResult ( int iError = 0 );	/// always return false
	void ReportFinish ( bool bSuccess = true );
	void SendingState (); ///< from CONNECTING state go to HEALTHY and switch timer to QUERY timeout.

	bool StartNextRetry ();

	void LazyTask ( int64_t iTimeoutMS, bool bHardTimeout = false, BYTE ActivateIO = 0 ); // 1=RW, 2=RO.
	void LazyDeleteOrChange ( int64_t iTimeoutMS = -1 );
	void ScheduleCallbacks ();
	void DisableWrite();

	void BuildData ();
	size_t ReplyBufPlace () const;
	void InitReplyBuf ( int iSize = 0 );
	inline bool IsReplyHeader() const { return m_iReplySize<0; }

	SSIZE_T SendChunk (); // low-level (platform specific) send
	SSIZE_T RecvChunk (); // low-level (platform specific) recv

	int DoTFO ( struct sockaddr * pSs, int iLen );

	bool DoQuery ();
	bool EstablishConnection ();
	bool SendQuery (DWORD uSent = 0);
	bool ReceiveAnswer (DWORD uReceived = 0);
	bool CommitResult ();
	bool SwitchBlackhole ();
};

using VectorAgentConn_t = CSphVector<AgentConn_t *>;
using VecRefPtrsAgentConn_t = VecRefPtrs_t<AgentConn_t *>;
class RemoteAgentsObserver_i : public Reporter_i
{
public:

	// get num of succeeded agents
	virtual long GetSucceeded () const = 0;

	// get num of succeeded agents
	virtual long GetFinished () const = 0;

	// block execution while there are works to do
	virtual void Finish () = 0;

	// block execution while some works finished
	virtual void WaitChanges () = 0;
};

RemoteAgentsObserver_i * GetObserver ();

void ScheduleDistrJobs ( VectorAgentConn_t &dRemotes, RequestBuilder_i * pQuery, ReplyParser_i * pParser,
	Reporter_i * pReporter, int iQueryRetry = -1, int iQueryDelay = -1 );

// schedule one job. Returns false if connection s blackhole (and so, will not report anything
using Deffered_f = std::function<void ( bool )>;
bool RunRemoteTask ( AgentConn_t* pConnection, RequestBuilder_i* pQuery, ReplyParser_i* pParser,
	Deffered_f && pAction, int iQueryRetry = -1, int iQueryDelay = -1 );
bool RunRemoteTask ( AgentConn_t* pConnection, RequestBuilder_i* pQuery, ReplyParser_i* pParser,
	Deffered_f & pAction, int iQueryRetry = -1, int iQueryDelay = -1 );

// simplified full task - schedule jobs, wait for complete, report num of succeeded
// uses cooperated wait - i.e. yield instead of pause
int PerformRemoteTasks ( VectorAgentConn_t &dRemotes, RequestBuilder_i * pQuery, ReplyParser_i * pParser, int iQueryRetry = -1, int iQueryDelay = -1 );

/////////////////////////////////////////////////////////////////////////////
// DISTRIBUTED QUERIES
/////////////////////////////////////////////////////////////////////////////

/// distributed index
struct DistributedIndex_t : public ISphRefcountedMT
{
	CSphVector<MultiAgentDescRefPtr_c> m_dAgents;	///< remote agents
	StrVec_t m_dLocal;								///< local indexes
	int m_iAgentConnectTimeoutMs	{ g_iAgentConnectTimeoutMs };	///< in msec
	int m_iAgentQueryTimeoutMs		{ g_iAgentQueryTimeoutMs };		///< in msec
	int m_iAgentRetryCount			= 0;			///< overrides global one
	bool m_bDivideRemoteRanges		= false;		///< whether we divide big range onto agents or not
	HAStrategies_e m_eHaStrategy	= HA_DEFAULT;	///< how to select the best of my agents
	mutable ServedStats_c			m_tStats;

	// get hive of all index'es hosts (not agents, but hosts, i.e. all mirrors as simple vector)
	void GetAllHosts ( VectorAgentConn_t &dTarget ) const;

	inline bool IsEmpty() const
	{
		return m_dAgents.IsEmpty() && m_dLocal.IsEmpty();
	}

	template<typename FUNC>
	void ForEveryHost ( FUNC&& pFunc )
	{
		for ( auto& pAgent : m_dAgents )
			for ( auto& dHost : *pAgent )
				pFunc ( dHost );
	}

	template<typename FUNC>
	void ForEveryHost ( FUNC&& pFunc ) const
	{
		for ( auto& pAgent : m_dAgents )
			for ( auto& dHost : *pAgent )
				pFunc ( dHost );
	}

private:
	~DistributedIndex_t() override;

	mutable SharedPtr_t<CSphIndex>		m_pRtMadeFromDistrIndex;
};

using DistributedIndexRefPtr_t = CSphRefcountedPtr<DistributedIndex_t>;
using cDistributedIndexRefPtr_t = CSphRefcountedPtr<const DistributedIndex_t>;

using ReadOnlyDistrHash_c = ReadOnlyHash_T<DistributedIndex_t>;
using WriteableDistrHash_c = WriteableHash_T<DistributedIndex_t>;
extern std::unique_ptr<ReadOnlyDistrHash_c> g_pDistIndexes; // distributed indexes hash
inline cDistributedIndexRefPtr_t GetDistr ( const CSphString& sName )
{
	assert ( g_pDistIndexes );
	return g_pDistIndexes->Get ( sName );
}

struct SearchdStats_t
{
	DWORD					m_uStarted;
	std::atomic<int64_t>	m_iConnections;
	std::atomic<int64_t>	m_iMaxedOut;
	std::atomic<int64_t>	m_iCommandCount[SEARCHD_COMMAND_TOTAL];
	std::atomic<int64_t>	m_iAgentConnect;
	std::atomic<int64_t>	m_iAgentConnectTFO;
	std::atomic<int64_t>	m_iAgentRetry;

	std::atomic<int64_t>	m_iQueries;			///< search queries count (differs from search commands count because of multi-queries)
	std::atomic<int64_t>	m_iQueryTime;		///< wall time spent (including network wait time)
	std::atomic<int64_t>	m_iQueryCpuTime;	///< CPU time spent

	std::atomic<int64_t>	m_iDistQueries;		///< distributed queries count
	std::atomic<int64_t>	m_iDistWallTime;	///< wall time spent on distributed queries
	std::atomic<int64_t>	m_iDistLocalTime;	///< wall time spent searching local indexes in distributed queries
	std::atomic<int64_t>	m_iDistWaitTime;	///< time spent waiting for remote agents in distributed queries

	std::atomic<int64_t>	m_iDiskReads;		///< total read IO calls (fired by search queries)
	std::atomic<int64_t>	m_iDiskReadBytes;	///< total read IO traffic
	std::atomic<int64_t>	m_iDiskReadTime;	///< total read IO time

	std::atomic<int64_t>	m_iPredictedTime;	///< total agent predicted query time
	std::atomic<int64_t>	m_iAgentPredictedTime;	///< total agent predicted query time

	void Init();
};

SearchdStats_t&	gStats();

namespace Dashboard
{
	void LinkHost ( HostDesc_t& dHost ); ///< put host into global dashboard and init link to it
	HostDashboardRefPtr_t FindAgent ( const CSphString& sAgent );
	CSphVector<HostDashboardRefPtr_t> GetActiveHosts ();
	void CleanupOrphaned ();
}

// parse strategy name into enum value
bool ParseStrategyHA ( const char * sName, HAStrategies_e & eStrategy );
CSphString HAStrategyToStr ( HAStrategies_e eStrategy );

// parse ','-delimited list of indexes
void ParseIndexList ( const CSphString &sIndexes, StrVec_t &dOut );

// try to parse hostname/ip/port or unixsocket on current pConfigLine.
// fill pAgent fields on success and move ppLine pointer next after parsed instance
// if :port is skipped in the line, IANA 9312 will be used in the case
bool ParseAddressPort ( HostDesc_t & pAgent, const char ** ppLine, const WarnInfo_c & tInfo );

//! Parse line with agent definition and return addreffed pointer to multiagent (new or from global cache)
//! \param szAgent - line with agent definition from config
//! \param szIndexName - index we apply to
//! \param tOptions - global options affecting agent
//! \return configured multiagent, or null if failed
MultiAgentDescRefPtr_c ConfigureMultiAgent ( const char * szAgent, const char * szIndexName, AgentOptions_t tOptions, StrVec_t * pWarnings=nullptr );

class RequestBuilder_i : public ISphNoncopyable
{
public:
	virtual ~RequestBuilder_i () {} // to avoid gcc4 warns
	virtual void BuildRequest ( const AgentConn_t &tAgent, ISphOutputBuffer &tOut ) const = 0;
};


class ReplyParser_i
{
public:
	virtual ~ReplyParser_i () {} // to avoid gcc4 warns
	virtual bool ParseReply ( MemInputBuffer_c &tReq, AgentConn_t &tAgent ) const = 0;
};

/// check if a non-blocked socket is still connected
bool sphNBSockEof ( int iSock );

class SphinxqlRequestBuilder_c : public RequestBuilder_i
{
public:
			SphinxqlRequestBuilder_c ( Str_t sQuery, const SqlStmt_t & tStmt );
	void	BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final;

protected:
	const Str_t m_sBegin;
	const Str_t m_sEnd;
};

class SphinxqlReplyParser_c : public ReplyParser_i
{
public:
	explicit SphinxqlReplyParser_c ( int * pUpd, int * pWarns );
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final;

protected:
	int * m_pUpdated;
	int * m_pWarns;
};

void RemotesGetField ( AggrResult_t & tRes, const CSphQuery & tQuery );
void HandleCommandGetField ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq );

#endif // _searchdha_
