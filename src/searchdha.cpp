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

#include "sphinx.h"
#include "sphinxstd.h"
#include "sphinxrt.h"
#include "sphinxint.h"
#include <errno.h>

#include "searchdaemon.h"
#include "searchdha.h"

#include <utility>

#if !USE_WINDOWS
	#include <netinet/in.h>
	#include <netdb.h>
#else
	#include <WinSock2.h>
	#include <MSWSock.h>
	#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#endif

#if HAVE_EVENTFD
	#include <sys/eventfd.h>
#endif

#if HAVE_GETADDRINFO_A
	#include <signal.h>
#endif

int				g_iPingInterval		= 0;		// by default ping HA agents every 1 second
DWORD			g_uHAPeriodKarma	= 60;		// by default use the last 1 minute statistic to determine the best HA agent

int				g_iPersistentPoolSize	= 0;

CSphString HostDesc_t::GetMyUrl() const
{
	CSphString sName;
	switch ( m_iFamily )
	{
	case AF_INET: sName.SetSprintf ( "%s:%u", m_sAddr.cstr(), m_iPort ); break;
	case AF_UNIX: sName = m_sAddr; break;
	}
	return sName;
}

static DWORD g_uTimePrefix = 0;
void StartLogTime()
{
	g_uTimePrefix = sphMicroTimer () / 1000;
}

void sphLogDebugTimeredPrefix ( const char* sPrefix, const char * sFmt, ... )
{
	StringBuilder_c sMyPrefix;
	sMyPrefix+=sPrefix;
	sMyPrefix.Appendf ("[%04d] ", (int)(sphMicroTimer ()/1000-g_uTimePrefix) );
	sMyPrefix+=sFmt;
	va_list ap;
	va_start ( ap, sFmt );
	sphLogVa ( sMyPrefix.cstr(), ap, SPH_LOG_VERBOSE_DEBUG );
	va_end ( ap );
}

#define VERBOSE_NETLOOP 0

#if VERBOSE_NETLOOP
	#define sphLogDebugA( ... ) sphLogDebugTimeredPrefix ("A ", __VA_ARGS__)
	#define sphLogDebugL( ... ) sphLogDebugTimeredPrefix ("L ", __VA_ARGS__)
#else
#if USE_WINDOWS
#pragma warning(disable:4390)
#endif
	#define sphLogDebugA( ... )
	#define sphLogDebugL( ... )
#endif

/////////////////////////////////////////////////////////////////////////////
// HostDashboard_t
//
// Dashboard contains generic statistic for a host like query time, answer time,
// num of errors a row, and all this data in retrospective on time
/////////////////////////////////////////////////////////////////////////////
HostDashboard_t::HostDashboard_t ( const HostDesc_t & tHost )
{
	assert ( !tHost.m_pDash );
	m_tHost.CloneFromHost ( tHost );
	m_iLastQueryTime = m_iLastAnswerTime = sphMicroTimer () - g_iPingInterval*1000;
	for ( auto & dStat : m_dStats )
		dStat.m_dData.Reset();
}

HostDashboard_t::~HostDashboard_t ()
{
	SafeDelete ( m_pPersPool );
}

bool HostDashboard_t::IsOlder ( int64_t iTime ) const
{
	return ( (iTime-m_iLastAnswerTime)>g_iPingInterval*1000LL );
}

DWORD HostDashboard_t::GetCurSeconds()
{
	int64_t iNow = sphMicroTimer()/1000000;
	return DWORD ( iNow & 0xFFFFFFFF );
}

bool HostDashboard_t::IsHalfPeriodChanged ( DWORD * pLast )
{
	assert ( pLast );
	DWORD uSeconds = GetCurSeconds();
	if ( ( uSeconds - *pLast )>( g_uHAPeriodKarma / 2 ) )
	{
		*pLast = uSeconds;
		return true;
	}
	return false;
}

AgentDash_t& HostDashboard_t::GetCurrentStat ()
{
	DWORD uCurrentPeriod = GetCurSeconds()/g_uHAPeriodKarma;
	auto & dCurrentStats = m_dStats[uCurrentPeriod % STATS_DASH_PERIODS];
	if ( dCurrentStats.m_uPeriod!=uCurrentPeriod ) // we have new or reused stat
	{
		dCurrentStats.m_dData.Reset ();
		dCurrentStats.m_uPeriod = uCurrentPeriod;
	}

	return dCurrentStats.m_dData;
}

void HostDashboard_t::GetCollectedStat ( HostStatSnapshot_t& dResult, int iPeriods ) const
{
	DWORD uSeconds = GetCurSeconds();

	if ( (uSeconds % g_uHAPeriodKarma) < (g_uHAPeriodKarma/2) )
		++iPeriods;
	iPeriods = Min ( iPeriods, STATS_DASH_PERIODS );

	DWORD uCurrentPeriod = uSeconds/g_uHAPeriodKarma;
	AgentDash_t tAccum;
	tAccum.Reset ();

	CSphScopedRLock tRguard ( m_dDataLock );
	for ( ; iPeriods>0; --iPeriods, --uCurrentPeriod )
		// it might be no queries at all in the fixed time
		if ( m_dStats[uCurrentPeriod % STATS_DASH_PERIODS].m_uPeriod==uCurrentPeriod )
			tAccum.Add ( m_dStats[uCurrentPeriod % STATS_DASH_PERIODS].m_dData );

	for ( int i = 0; i<eMaxAgentStat; ++i )
		dResult[i] = tAccum.m_dCounters[i];
	for ( int i = 0; i<ehMaxStat; ++i )
		dResult[i+eMaxAgentStat] = tAccum.m_dMetrics[i];
}

/////////////////////////////////////////////////////////////////////////////
// PersistentConnectionsPool_c
//
// This is pool of sockets which can be rented/returned by workers.
// returned socket may be in connected state, so later usage will not require
// connection, but can work immediately (in persistent mode).
/////////////////////////////////////////////////////////////////////////////

// modify size of the pool
// (actually we just need to close any extra sockets not fit in new limit. And do nothing otherwize).
void PersistentConnectionsPool_c::ReInit ( int iPoolSize )
{
	assert ( iPoolSize >0 );
	// close any extra sockets which are not fit in new limit
	ScopedMutex_t tGuard ( m_dDataLock );
	m_iLimit = iPoolSize;
	m_dSockets.Reserve ( m_iLimit );
	while ( m_iFreeWindow>m_iLimit )
	{
		--m_iFreeWindow;
		int iSock = m_dSockets[Step ( &m_iRit )];
		if ( iSock>=0 )
			sphSockClose ( iSock );
	}

}

// move the iterator to the next item, or loop the ring.
int PersistentConnectionsPool_c::Step ( int* pVar )
{
	assert ( pVar );
	int iRes = *pVar++;
	if ( *pVar>=m_dSockets.GetLength () )
		*pVar = 0;
	return iRes;
}

// Rent first try to return the sockets which already were in work (i.e. which are connected)
// If no such socket available and the limit is not reached, it will add the new one.
int PersistentConnectionsPool_c::RentConnection ()
{
	ScopedMutex_t tGuard ( m_dDataLock );
	if ( m_iFreeWindow>0 )
	{
		--m_iFreeWindow;
		return m_dSockets[Step ( &m_iRit )];
	}
	if ( m_dSockets.GetLength () >= m_iLimit )
		return -2; // no more slots available;

	// this branch will be executed only during initial 'heating'
	m_dSockets.Add ( -1 );
	return -1;
}

// write given socket into the pool.
void PersistentConnectionsPool_c::ReturnConnection ( int iSocket )
{
	ScopedMutex_t tGuard ( m_dDataLock );

	// overloaded pool
	if ( m_iFreeWindow >= m_dSockets.GetLength () )
	{
		// no place at all (i.e. if pool was resized, but some sockets rented before,
		// and now they are returned, but we have no place for them)
		if ( m_dSockets.GetLength () >= m_iLimit )
		{
			sphSockClose ( iSocket );
			return;
		}
		// add the place for one more returned socket
		m_dSockets.Add ();
		m_iWit = m_dSockets.GetLength ()-1;
	}
	++m_iFreeWindow;
	if ( m_bShutdown )
	{
		sphSockClose ( iSocket );
		iSocket = -1;
	}
	// if there were no free sockets until now, point the reading iterator to just released socket
	if ( m_iFreeWindow==1 )
		m_iRit = m_iWit;
	m_dSockets[Step ( &m_iWit )] = iSocket;
}

// close all the sockets in the pool.
void PersistentConnectionsPool_c::Shutdown ()
{
	ScopedMutex_t tGuard ( m_dDataLock );
	m_bShutdown = true;
	for ( int i = 0; i<m_iFreeWindow; ++i )
	{
		int& iSock = m_dSockets[Step ( &m_iRit )];
		if ( iSock>=0 )
		{
			sphSockClose ( iSock );
			iSock = -1;
		}
	}
}

void ClosePersistentSockets()
{
	VecRefPtrs_t<HostDashboard_t *> dHosts;
	g_tDashes.GetActiveDashes ( dHosts );
	dHosts.Apply ( [] ( HostDashboard_t * &pHost ) { SafeDelete ( pHost->m_pPersPool ); } );
}

// check whether sURL contains plain ip-address, and so, m.b. no need to resolve it many times.
// ip-address assumed as string from four .-separated positive integers, each in range 0..255
static bool IsIpAddress ( const char * sURL )
{
	StrVec_t dParts;
	sphSplit ( dParts, sURL, "." );
	if ( dParts.GetLength ()!=4 )
		return false;
	for ( auto &dPart : dParts )
	{
		dPart.Trim ();
		auto iLen = dPart.Length ();
		for ( auto i = 0; i<iLen; ++i )
			if ( !isdigit ( dPart.cstr ()[i] ) )
				return false;
		auto iRes = atoi ( dPart.cstr () );
		if ( iRes<0 || iRes>255 )
			return false;
	}
	return true;
}

/// Set flag m_bNeedResolve if address is AF_INET, host is not empty and not plain IP address,
/// and also if global flag for postponed resolving is set.
/// Otherwise resolve address now (if appliable) and then forbid any name resolving in future.
bool ResolveAddress ( AgentDesc_t &tAgent, const WarnInfo_t &tInfo )
{
	tAgent.m_bNeedResolve = false;
	if ( tAgent.m_iFamily!=AF_INET )
		return true;

	if ( tAgent.m_sAddr.IsEmpty () )
		return tInfo.ErrSkip ( "invalid host name 'empty'" );

	if ( IsIpAddress ( tAgent.m_sAddr.cstr () ) )
	{
		tAgent.m_uAddr = sphGetAddress ( tAgent.m_sAddr.cstr (), false, true );
		if ( tAgent.m_uAddr )
			return true;

		// got plain ip and couldn't resolve it? Hm, strange...
		if ( !g_bHostnameLookup )
			return tInfo.ErrSkip ( "failed to lookup host name '%s' (error=%s)", tAgent.m_sAddr.cstr ()
								   , sphSockError () );
	}

	// if global flag set, don't resolve right now, it will be done on place.
	tAgent.m_bNeedResolve = g_bHostnameLookup;
	if ( tAgent.m_bNeedResolve )
		return true;

	tAgent.m_uAddr = sphGetAddress ( tAgent.m_sAddr.cstr () );
	if ( tAgent.m_uAddr )
		return true;

	return tInfo.ErrSkip ( "failed to lookup host name '%s' (error=%s)", tAgent.m_sAddr.cstr (), sphSockError () );
}

/// Async resolving
using CallBack_f = std::function<void ( DWORD )>;
class DNSResolver_c
{
	struct addrinfo m_tHints { 0 };
	CallBack_f	m_pCallback			= nullptr;
	bool 		m_bCallbackInvoked	= false;
	CSphString	m_sHost;

	DNSResolver_c ( const char * sHost, CallBack_f &&pFunc )
		: m_pCallback ( std::move ( pFunc ) )
		, m_sHost ( sHost )
	{
		m_tHints.ai_family = AF_INET;
		m_tHints.ai_socktype = SOCK_STREAM;
	}

	~DNSResolver_c ()
	{
		assert ( m_pCallback );
		if ( !m_bCallbackInvoked )
			m_pCallback ( 0 );
	};

	void FinishResolve ( struct addrinfo * pResult )
	{
		assert ( m_pCallback );
		if ( !pResult )
			return;
		auto * pSockaddr_ipv4 = ( struct sockaddr_in * ) pResult->ai_addr;
		auto uIP = pSockaddr_ipv4->sin_addr.s_addr;
		freeaddrinfo ( pResult );
		m_pCallback ( uIP );
		m_bCallbackInvoked = true;
	};

	static void ResolvingRoutine ( void * pResolver )
	{
		auto * pThis = ( DNSResolver_c * ) pResolver;
		if ( pThis )
			pThis->Resolve ();
		SafeDelete ( pThis );
	}

// platform-specific part starts here.
#if HAVE_GETADDRINFO_A
	struct gaicb m_dReq, *m_pReq;

	static void ResolvingWrapper ( sigval dSigval )
	{
		ResolvingRoutine ( dSigval.sival_ptr );
	};

	void ResolveBegin ()
	{
		m_dReq = { 0 };
		m_dReq.ar_name = m_sHost.cstr ();
		m_dReq.ar_request = &m_tHints;
		m_pReq = &m_dReq;

		sigevent_t dCallBack = { 0 };
		dCallBack.sigev_notify = SIGEV_THREAD;
		dCallBack.sigev_value.sival_ptr = this;
		dCallBack.sigev_notify_function = ResolvingWrapper;
		getaddrinfo_a ( GAI_NOWAIT, &m_pReq, 1, &dCallBack );
	}

	void Resolve ()
	{
		auto * pResult = m_dReq.ar_result;
		FinishResolve ( pResult );
	}
#else
	SphThread_t m_dResolverThread;

	void ResolveBegin ()
	{
		sphThreadCreate ( &m_dResolverThread, ResolvingRoutine, this, true );
	}

	void Resolve ()
	{
		struct addrinfo * pResult = nullptr;
		if ( !getaddrinfo ( m_sHost.cstr (), nullptr, &m_tHints, &pResult ) )
			FinishResolve ( pResult );
	}
#endif

public:
	//! Non-blocking DNS resolver
	//! Will call provided callback with single DWORD parameter - resolved IP. (or 0 on fail)
	//! If there is no need to go background (i.e. we have plain ip address which not need to be
	//! resolved any complex way), will call the callback immediately.
	//! \param sHost - host address, as 'www.google.com', or '127.0.0.1'
	//! \param pFunc - callback func. May be simple functor, or even lambda with acquired resources.
	static void GetAddress_a ( const char * sHost, CallBack_f && pFunc )
	{
		if ( IsIpAddress ( sHost ) )
		{
			DWORD uResult = sphGetAddress ( sHost, false, true );
			if ( uResult )
			{
				if ( pFunc )
					pFunc ( uResult );
				return;
			}
		}

		auto * pPayload = new DNSResolver_c ( sHost, std::forward<CallBack_f>(pFunc) );
		pPayload->ResolveBegin();
	}
};

/// initialize address resolving,
/// add to agent records of stats,
/// add agent into global dashboard hash
static bool ValidateAndAddDashboard ( AgentDesc_t& dAgent, const WarnInfo_t &tInfo )
{
	assert ( !dAgent.m_pDash && !dAgent.m_pStats );
	if ( !ResolveAddress ( dAgent, tInfo ) )
		return false;

	g_tDashes.LinkHost ( dAgent );
	dAgent.m_pStats = new AgentDash_t;
	assert ( dAgent.m_pDash );
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// MultiAgentDesc_c
//
// That is set of hosts serving one and same agent (i.e., 'mirrors').
// class also provides mirror choosing using different strategies
/////////////////////////////////////////////////////////////////////////////

bool MultiAgentDesc_c::Init ( const CSphVector<AgentDesc_t*> &dHosts, const AgentOptions_t &tOpt,
								const WarnInfo_t &tWarn ) NO_THREAD_SAFETY_ANALYSIS
{
	// initialize options
	m_eStrategy = tOpt.m_eStrategy;
	m_iMultiRetryCount = tOpt.m_iRetryCount * tOpt.m_iRetryCountMultiplier;

	// initialize hosts & weights
	auto iLen = dHosts.GetLength ();
	Reset ( iLen );
	m_dWeights.Reset ( iLen );
	if (!iLen)
		return tWarn.ErrSkip ("Unable to initialize empty agent");

	auto fFrac = 100.0f / iLen;
	ARRAY_FOREACH ( i, dHosts )
	{
		assert ( !dHosts[i]->m_pDash && !dHosts[i]->m_pStats ); // we have templates parsed from config, NOT real working hosts!
		if ( !ValidateAndAddDashboard ( (m_pData+i)->CloneFrom ( *dHosts[i] ), tWarn ) )
			return false;
		m_dWeights[i] = fFrac;
	}

	// require pings for HA agent
	if ( IsHA () )
		for ( int i = 0; i<GetLength (); ++i )
			m_pData[i].m_pDash->m_bNeedPing = true;

	return true;
}

const AgentDesc_t &MultiAgentDesc_c::RRAgent ()
{
	if ( !IsHA() )
		return *m_pData;

	auto iRRCounter = (int) m_iRRCounter++;
	while ( iRRCounter<0 || iRRCounter> ( GetLength ()-1 ) )
	{
		if ( iRRCounter+1 == (int) m_iRRCounter.CAS ( iRRCounter+1, 1 ) )
			iRRCounter = 0;
		else
			iRRCounter = (int) m_iRRCounter++;
	}

	return m_pData[iRRCounter];
}

const AgentDesc_t &MultiAgentDesc_c::RandAgent ()
{
	return m_pData[sphRand () % GetLength ()];
}

void MultiAgentDesc_c::ChooseWeightedRandAgent ( int * pBestAgent, CSphVector<int> & dCandidates )
{
	assert ( pBestAgent );
	CSphScopedRLock tLock ( m_dWeightLock );
	auto fBound = m_dWeights[*pBestAgent];
	auto fLimit = fBound;
	for ( auto j : dCandidates )
		fLimit += m_dWeights[j];
	auto fChance = sphRand () * fLimit / UINT_MAX;

	if ( fChance<=fBound )
		return;

	for ( auto j : dCandidates )
	{
		fBound += m_dWeights[j];
		*pBestAgent = j;
		if ( fChance<=fBound )
			break;
	}
}

static void LogAgentWeights ( const float * pOldWeights, const float * pCurWeights, const int64_t * pTimers, const CSphFixedVector<AgentDesc_t> & dAgents )
{
	ARRAY_FOREACH ( i, dAgents )
		sphLogDebug ( "client=%s, mirror=%d, weight=%0.2f%%, %0.2f%%, timer=" INT64_FMT
					  , dAgents[i].GetMyUrl ().cstr (), i, pCurWeights[i], pOldWeights[i], pTimers[i] );
}

const AgentDesc_t &MultiAgentDesc_c::StDiscardDead ()
{
	if ( !IsHA() )
		return m_pData[0];

	// threshold errors-a-row to be counted as dead
	int iDeadThr = 3;

	int iBestAgent = -1;
	int64_t iErrARow = -1;
	CSphVector<int> dCandidates;
	CSphFixedVector<int64_t> dTimers ( GetLength() );
	dCandidates.Reserve ( GetLength() );

	for (int i=0; i<GetLength(); ++i)
	{
		// no locks for g_pStats since we just reading, and read data is not critical.
		const HostDashboard_t * pDash = m_pData[i].m_pDash;

		HostStatSnapshot_t dDashStat;
		pDash->GetCollectedStat (dDashStat);// look at last 30..90 seconds.
		uint64_t uQueries = 0;
		for ( int j=0; j<eMaxAgentStat; ++j )
			uQueries += dDashStat[j];
		if ( uQueries > 0 )
			dTimers[i] = dDashStat[ehTotalMsecs]/uQueries;
		else
			dTimers[i] = 0;

		CSphScopedRLock tRguard ( pDash->m_dDataLock );
		int64_t iThisErrARow = ( pDash->m_iErrorsARow<=iDeadThr ) ? 0 : pDash->m_iErrorsARow;

		if ( iErrARow < 0 )
			iErrARow = iThisErrARow;

		// 2. Among good nodes - select the one(s) with lowest errors/query rating
		if ( iErrARow > iThisErrARow )
		{
			dCandidates.Reset();
			iBestAgent = i;
			iErrARow = iThisErrARow;
		} else if ( iErrARow==iThisErrARow )
		{
			if ( iBestAgent>=0 )
				dCandidates.Add ( iBestAgent );
			iBestAgent = i;
		}
	}

	// check if it is a time to recalculate the agent's weights
	CheckRecalculateWeights ( dTimers );


	// nothing to select, sorry. Just random agent...
	if ( iBestAgent < 0 )
	{
		sphLogDebug ( "HA selector discarded all the candidates and just fall into simple Random" );
		return RandAgent();
	}

	// only one node with lowest error rating. Return it.
	if ( !dCandidates.GetLength() )
	{
		sphLogDebug ( "client=%s, HA selected %d node with best num of errors a row (" INT64_FMT ")"
					  , m_pData[iBestAgent].GetMyUrl().cstr(), iBestAgent, iErrARow );
		return m_pData[iBestAgent];
	}

	// several nodes. Let's select the one.
	ChooseWeightedRandAgent ( &iBestAgent, dCandidates );
	if ( g_eLogLevel>=SPH_LOG_VERBOSE_DEBUG )
	{
		float fAge = 0.0;
		const HostDashboard_t & dDash = *m_pData[iBestAgent].m_pDash;
		CSphScopedRLock tRguard ( dDash.m_dDataLock );
		fAge = ( dDash.m_iLastAnswerTime-dDash.m_iLastQueryTime ) / 1000.0f;
		sphLogDebugv ("client=%s, HA selected %d node by weighted random, with best EaR ("
						  INT64_FMT "), last answered in %.3f milliseconds, among %d candidates"
					  , m_pData[iBestAgent].GetMyUrl ().cstr(), iBestAgent, iErrARow, fAge, dCandidates.GetLength()+1 );
	}

	return m_pData[iBestAgent];
}

// Check the time and recalculate mirror weights, if necessary.
void MultiAgentDesc_c::CheckRecalculateWeights ( const CSphFixedVector<int64_t> & dTimers )
{
	if ( !dTimers.GetLength () || !HostDashboard_t::IsHalfPeriodChanged ( &m_uTimestamp ) )
		return;

	CSphFixedVector<float> dWeights {0};

	// since we'll update values anyway, acquire w-lock.
	CSphScopedWLock tWguard ( m_dWeightLock );
	dWeights.CopyFrom ( m_dWeights );
	RebalanceWeights ( dTimers, dWeights );
	if ( g_eLogLevel>=SPH_LOG_DEBUG )
		LogAgentWeights ( m_dWeights.Begin(), dWeights.Begin (), dTimers.Begin (), *this );
	m_dWeights.SwapData (dWeights);
}

const AgentDesc_t &MultiAgentDesc_c::StLowErrors ()
{
	if ( !IsHA() )
		return *m_pData;

	// how much error rating is allowed
	float fAllowedErrorRating = 0.03f; // i.e. 3 errors per 100 queries is still ok

	int iBestAgent = -1;
	float fBestCriticalErrors = 1.0;
	float fBestAllErrors = 1.0;
	CSphVector<int> dCandidates;
	CSphFixedVector<int64_t> dTimers ( GetLength() );
	dCandidates.Reserve ( GetLength() );

	for ( int i=0; i<GetLength(); ++i )
	{
		// no locks for g_pStats since we just reading, and read data is not critical.
		const HostDashboard_t & dDash = *m_pData[i].m_pDash;

		HostStatSnapshot_t dDashStat;
		dDash.GetCollectedStat (dDashStat); // look at last 30..90 seconds.
		uint64_t uQueries = 0;
		uint64_t uCriticalErrors = 0;
		uint64_t uAllErrors = 0;
		uint64_t uSuccesses = 0;
		for ( int j=0; j<eMaxAgentStat; ++j )
		{
			if ( j==eNetworkCritical )
				uCriticalErrors = uQueries;
			else if ( j==eNetworkNonCritical )
			{
				uAllErrors = uQueries;
				uSuccesses = dDashStat[j];
			}
			uQueries += dDashStat[j];
		}

		if ( uQueries > 0 )
			dTimers[i] = dDashStat[ehTotalMsecs]/uQueries;
		else
			dTimers[i] = 0;

		// 1. No successes queries last period (it includes the pings). Skip such node!
		if ( !uSuccesses )
			continue;

		if ( uQueries )
		{
			// 2. Among good nodes - select the one(s) with lowest errors/query rating
			float fCriticalErrors = (float) uCriticalErrors/uQueries;
			float fAllErrors = (float) uAllErrors/uQueries;
			if ( fCriticalErrors<=fAllowedErrorRating )
				fCriticalErrors = 0.0f;
			if ( fAllErrors<=fAllowedErrorRating )
				fAllErrors = 0.0f;
			if ( fCriticalErrors < fBestCriticalErrors )
			{
				dCandidates.Reset();
				iBestAgent = i;
				fBestCriticalErrors = fCriticalErrors;
				fBestAllErrors = fAllErrors;
			} else if ( fCriticalErrors==fBestCriticalErrors )
			{
				if ( fAllErrors < fBestAllErrors )
				{
					dCandidates.Reset();
					iBestAgent = i;
					fBestAllErrors = fAllErrors;
				} else if ( fAllErrors==fBestAllErrors )
				{
					if ( iBestAgent>=0 )
						dCandidates.Add ( iBestAgent );
					iBestAgent = i;
				}
			}
		}
	}

	// check if it is a time to recalculate the agent's weights
	CheckRecalculateWeights ( dTimers );

	// nothing to select, sorry. Just plain RR...
	if ( iBestAgent < 0 )
	{
		sphLogDebug ( "HA selector discarded all the candidates and just fall into simple Random" );
		return RandAgent();
	}

	// only one node with lowest error rating. Return it.
	if ( !dCandidates.GetLength() )
	{
		sphLogDebug ( "client=%s, HA selected %d node with best error rating (%.2f)",
			m_pData[iBestAgent].GetMyUrl().cstr(), iBestAgent, fBestCriticalErrors );
		return m_pData[iBestAgent];
	}

	// several nodes. Let's select the one.
	ChooseWeightedRandAgent ( &iBestAgent, dCandidates );
	if ( g_eLogLevel>=SPH_LOG_VERBOSE_DEBUG )
	{
		float fAge = 0.0f;
		const HostDashboard_t & dDash = *m_pData[iBestAgent].m_pDash;
		CSphScopedRLock tRguard ( dDash.m_dDataLock );
		fAge = ( dDash.m_iLastAnswerTime-dDash.m_iLastQueryTime ) / 1000.0f;
		sphLogDebugv (
			"client=%s, HA selected %d node by weighted random, "
				"with best error rating (%.2f), answered %f seconds ago"
			, m_pData[iBestAgent].GetMyUrl ().cstr(), iBestAgent, fBestCriticalErrors, fAge );
	}

	return m_pData[iBestAgent];
}


const AgentDesc_t &MultiAgentDesc_c::ChooseAgent ()
{
	if ( !IsHA() )
		return *m_pData;

	switch ( m_eStrategy )
	{
	case HA_AVOIDDEAD:
		return StDiscardDead();
	case HA_AVOIDERRORS:
		return StLowErrors();
	case HA_ROUNDROBIN:
		return RRAgent();
	default:
		return RandAgent();
	}
}

const char * Agent_e_Name ( Agent_e eState )
{
	switch ( eState )
	{
	case Agent_e::HEALTHY: return "HEALTHY";
	case Agent_e::CONNECTING: return "CONNECTING";
	case Agent_e::RETRY: return "RETRY";
	}
	assert ( 0 && "UNKNOWN_STATE" );
	return "UNKNOWN_STATE";
}

SearchdStats_t g_tStats;
cDashStorage g_tDashes;

// generic stats track - always to agent stats, separately to dashboard.
void agent_stats_inc ( AgentConn_t &tAgent, AgentStats_e iCountID )
{
	assert ( iCountID<=eMaxAgentStat );
	assert ( tAgent.m_tDesc.m_pDash );

	if ( tAgent.m_tDesc.m_pStats )
		++tAgent.m_tDesc.m_pStats->m_dCounters[iCountID];

	HostDashboard_t &tIndexDash = *tAgent.m_tDesc.m_pDash;
	CSphScopedWLock tWguard ( tIndexDash.m_dDataLock );
	AgentDash_t &tAgentDash = tIndexDash.GetCurrentStat ();
	tAgentDash.m_dCounters[iCountID]++;
	if ( iCountID>=eNetworkNonCritical && iCountID<eMaxAgentStat )
		tIndexDash.m_iErrorsARow = 0;
	else
		tIndexDash.m_iErrorsARow++;

	tAgent.m_iEndQuery = sphMicroTimer ();
	tIndexDash.m_iLastQueryTime = tAgent.m_iStartQuery;
	tIndexDash.m_iLastAnswerTime = tAgent.m_iEndQuery;

	// do not count query time for unlinked connections (pings)
	// only count errors
	if ( tAgent.m_tDesc.m_pStats )
	{
		tAgentDash.m_dMetrics[ehTotalMsecs] += tAgent.m_iEndQuery - tAgent.m_iStartQuery;
		tAgent.m_tDesc.m_pStats->m_dMetrics[ehTotalMsecs] += tAgent.m_iEndQuery - tAgent.m_iStartQuery;
	}
}

// special case of stats - all is ok, just need to track the time in dashboard.
void track_processing_time ( AgentConn_t &tAgent )
{
	// first we count temporary statistic (into dashboard)
	assert ( tAgent.m_tDesc.m_pDash );
	uint64_t uConnTime = ( uint64_t ) sphMicroTimer () - tAgent.m_iStartQuery;
	{
		CSphScopedWLock tWguard ( tAgent.m_tDesc.m_pDash->m_dDataLock );
		uint64_t * pMetrics = tAgent.m_tDesc.m_pDash->GetCurrentStat ().m_dMetrics;

		++pMetrics[ehConnTries];
		if ( uint64_t ( uConnTime )>pMetrics[ehMaxMsecs] )
			pMetrics[ehMaxMsecs] = uConnTime;

		if ( pMetrics[ehConnTries]>1 )
			pMetrics[ehAverageMsecs] =
				( pMetrics[ehAverageMsecs] * ( pMetrics[ehConnTries] - 1 ) + uConnTime ) / pMetrics[ehConnTries];
		else
			pMetrics[ehAverageMsecs] = uConnTime;
	} // no need to hold dashboard anymore


	if ( !tAgent.m_tDesc.m_pStats )
		return;

	// then we count permanent statistic (for show status)
	uint64_t * pHStat = tAgent.m_tDesc.m_pStats->m_dMetrics;
	++pHStat[ehConnTries];
	if ( uint64_t ( uConnTime )>pHStat[ehMaxMsecs] )
		pHStat[ehMaxMsecs] = uConnTime;
	if ( pHStat[ehConnTries]>1 )
		pHStat[ehAverageMsecs] =
			( pHStat[ehAverageMsecs] * ( pHStat[ehConnTries] - 1 ) + uConnTime ) / pHStat[ehConnTries];
	else
		pHStat[ehAverageMsecs] = uConnTime;
}

/// try to parse hostname/ip/port or unixsocket on current pConfigLine.
/// fill pAgent fields on success and move ppLine pointer next after parsed instance
/// test cases and test group 'T_ParseAddressPort', are in gtest_searchdaemon.cpp.
bool ParseAddressPort ( HostDesc_t& dHost, const char ** ppLine, const WarnInfo_t &dInfo )
{
	// extract host name or path
	const char *&p = *ppLine;
	const char * pAnchor = p;

	if ( !p )
		return false;

	enum AddressType_e { apIP, apUNIX };
	AddressType_e eState = apIP;
	if ( *p=='/' ) // whether we parse inet or unix socket
		eState = apUNIX;

	while ( sphIsAlpha ( *p ) || *p=='.' || *p=='-' || *p=='/' )
		++p;
	if ( p==pAnchor )
		return dInfo.ErrSkip ( "host name or path expected" );

	CSphString sSub ( pAnchor, p-pAnchor );
	if ( eState==apUNIX )
	{
#if !USE_WINDOWS
		if ( strlen ( sSub.cstr () ) + 1>sizeof(((struct sockaddr_un *)0)->sun_path) )
			return dInfo.ErrSkip ( "UNIX socket path is too long" );
#endif
		dHost.m_iFamily = AF_UNIX;
		dHost.m_sAddr = sSub;
		return true;
	}

	// below is only deal with inet sockets
	dHost.m_iFamily = AF_INET;
	dHost.m_sAddr = sSub;

	// expect ':' (and then portnum) after address
	if ( *p!=':' )
	{
		dHost.m_iPort = IANA_PORT_SPHINXAPI;
		dInfo.Warn ( "colon and portnum expected before '%s' - Using default IANA %d port", p, dHost.m_iPort );
		return true;
	}
	pAnchor = ++p;

	// parse portnum
	while ( isdigit(*p) )
		++p;

	if ( p==pAnchor )
	{
		dHost.m_iPort = IANA_PORT_SPHINXAPI;
		dInfo.Warn ( "portnum expected before '%s' - Using default IANA %d port", p, dHost.m_iPort );
		--p; /// step back to ':'
		return true;
	}
	dHost.m_iPort = atoi ( pAnchor );

	if ( !IsPortInRange ( dHost.m_iPort ) )
		return dInfo.ErrSkip ( "invalid port number near '%s'", p );

	return true;
}

bool ParseStrategyHA ( const char * sName, HAStrategies_e * pStrategy )
{
	if ( sphStrMatchStatic ( "random", sName ) )
		*pStrategy = HA_RANDOM;
	else if ( sphStrMatchStatic ( "roundrobin", sName ) )
		*pStrategy = HA_ROUNDROBIN;
	else if ( sphStrMatchStatic ( "nodeads", sName ) )
		*pStrategy = HA_AVOIDDEAD;
	else if ( sphStrMatchStatic ( "noerrors", sName ) )
		*pStrategy = HA_AVOIDERRORS;
	else
		return false;

	return true;
}

void ParseIndexList ( const CSphString &sIndexes, StrVec_t &dOut )
{
	CSphString sSplit = sIndexes;
	if ( sIndexes.IsEmpty () )
		return;

	auto * p = ( char * ) sSplit.cstr ();
	while ( *p )
	{
		// skip non-alphas
		while ( *p && !isalpha ( *p ) && !isdigit ( *p ) && *p!='_' )
			p++;
		if ( !( *p ) )
			break;

		// FIXME?
		// We no not check that index name shouldn't start with '_'.
		// That means it's de facto allowed for API queries.
		// But not for SphinxQL ones.

		// this is my next index name
		const char * sNext = p;
		while ( isalpha ( *p ) || isdigit ( *p ) || *p=='_' )
			p++;

		assert ( sNext!=p );
		if ( *p )
			*p++ = '\0'; // if it was not the end yet, we'll continue from next char

		dOut.Add ( sNext );
	}
}

// parse agent's options line and modify pOptions
bool ParseOptions ( AgentOptions_t * pOptions, const CSphString& sOptions, const WarnInfo_t &dWI )
{
	StrVec_t dSplitParts;
	sphSplit ( dSplitParts, sOptions.cstr (), "," ); // diff. options are ,-separated
	for ( auto &sOption : dSplitParts )
	{
		if ( sOption.IsEmpty () )
			continue;

		// split '=' separated pair into tokens
		StrVec_t dOption;
		sphSplit ( dOption, sOption.cstr (), "=" );
		if ( dOption.GetLength ()!=2 )
			return dWI.ErrSkip ( "option %s error: option and value must be =-separated pair", sOption.cstr () );

		for ( auto &sOpt : dOption )
			sOpt.ToLower ().Trim ();

		const char * sOptName = dOption[0].cstr ();
		const char * sOptValue = dOption[1].cstr ();
		if ( sphStrMatchStatic ( "conn", sOptName ) )
		{
			if ( sphStrMatchStatic ( "pconn", sOptValue ) || sphStrMatchStatic ( "persistent", sOptValue ) )
			{
				pOptions->m_bPersistent = true;
				continue;
			}
		} else if ( sphStrMatchStatic ( "ha_strategy", sOptName ) )
		{
			if ( ParseStrategyHA ( sOptValue, &pOptions->m_eStrategy ) )
				continue;
		} else if ( sphStrMatchStatic ( "blackhole", sOptName ) )
		{
			pOptions->m_bBlackhole = ( atoi ( sOptValue )!=0 );
			continue;
		} else if ( sphStrMatchStatic ( "retry_count", sOptName ) )
		{
			pOptions->m_iRetryCount = atoi ( sOptValue );
			pOptions->m_iRetryCountMultiplier = 1;
			continue;
		}
		return dWI.ErrSkip ( "unknown agent option '%s'", sOption.cstr () );
	}
	return true;
}

// check whether all index(es) in list are valid index names
bool CheckIndexNames ( const CSphString &sIndexes, const WarnInfo_t& dWI )
{
	StrVec_t dRawIndexes, dParsedIndexes;

	// compare two lists: one made by raw splitting with ',' character,
	// second made by our ParseIndexList function.
	sphSplit(dRawIndexes, sIndexes.cstr(), ",");
	ParseIndexList ( sIndexes, dParsedIndexes );

	if ( dParsedIndexes.GetLength ()==dRawIndexes.GetLength () )
		return true;

	ARRAY_FOREACH( i, dParsedIndexes )
	{
		dRawIndexes[i].Trim();
		if ( dRawIndexes[i]!= dParsedIndexes[i] )
			return dWI.ErrSkip ("no such index: %s", dRawIndexes[i].cstr());
	}
	return true;
}

// parse agent string into template vec of AgentDesc_t, then configure them.
static bool ConfigureMirrorSet ( CSphVector<AgentDesc_t*> &tMirrors, AgentOptions_t * pOptions, const WarnInfo_t& dWI )
{
	assert ( tMirrors.IsEmpty () );

	StrVec_t dSplitParts;
	sphSplit ( dSplitParts, dWI.m_szAgent, "[]" );
	if ( dSplitParts.IsEmpty () )
		return dWI.ErrSkip ( "empty agent definition" );

	if ( dSplitParts[0].IsEmpty () )
		return dWI.ErrSkip ( "one or more hosts/sockets expected before [" );

	if ( dSplitParts.GetLength ()<1 || dSplitParts.GetLength ()>2 )
		return dWI.ErrSkip ( "wrong syntax: expected one or more hosts/sockets, then m.b. []-enclosed options" );

	// parse agents
	// separate '|'-splitted records, normalize result
	StrVec_t dRawAgents;
	sphSplit ( dRawAgents, dSplitParts[0].cstr (), "|" );
	for ( auto &sAgent : dRawAgents )
		sAgent.Trim ();

	if ( dSplitParts.GetLength ()==2 && !ParseOptions ( pOptions, dSplitParts[1], dWI ) )
		return false;

	assert ( dRawAgents.GetLength ()>0 );

	// parse separate strings into agent descriptors
	for ( auto &sAgent: dRawAgents )
	{
		if ( sAgent.IsEmpty () )
			continue;

		tMirrors.Add ( new AgentDesc_t );
		AgentDesc_t &dMirror = *tMirrors.Last ();
		const char * sRawAgent = sAgent.cstr ();
		if ( !ParseAddressPort ( dMirror, &sRawAgent, dWI ) )
			return false;

		// apply per-mirror options
		dMirror.m_bPersistent = pOptions->m_bPersistent;
		dMirror.m_bBlackhole = pOptions->m_bBlackhole;

		if ( *sRawAgent )
		{
			if ( *sRawAgent!=':' )
				return dWI.ErrSkip ( "after host/socket expected ':', then index(es), but got '%s')", sRawAgent );

			CSphString sIndexList = ++sRawAgent;
			sIndexList.Trim ();
			if ( sIndexList.IsEmpty () )
				continue;

			if ( !CheckIndexNames ( sIndexList, dWI ) )
				return false;

			dMirror.m_sIndexes = sIndexList;
		}
	}

	// fixup multiplier (if it is 0, it is still not defined).
	if ( !pOptions->m_iRetryCountMultiplier )
		pOptions->m_iRetryCountMultiplier = tMirrors.GetLength ();

	// fixup agent's indexes name: if no index assigned, stick the next one (or the parent).
	CSphString sLastIndex = dWI.m_szIndexName;
	for ( int i = tMirrors.GetLength () - 1; i>=0; --i )
		if ( tMirrors[i]->m_sIndexes.IsEmpty () )
			tMirrors[i]->m_sIndexes = sLastIndex;
		else
			sLastIndex = tMirrors[i]->m_sIndexes;

	return true;
}

// different cases are tested in T_ConfigureMultiAgent, see gtests_searchdaemon.cpp
bool ConfigureMultiAgent ( MultiAgentDesc_c &tAgent, const char * szAgent, const char * szIndexName
						   , AgentOptions_t tOptions )
{
	CSphVector<AgentDesc_t *> tMirrors;
	WarnInfo_t dWI { szIndexName, szAgent };

	if ( !ConfigureMirrorSet ( tMirrors, &tOptions, dWI ) )
		return false;

	return tAgent.Init ( tMirrors, tOptions, dWI );
}

HostDesc_t &HostDesc_t::CloneFromHost ( const HostDesc_t &rhs )
{
	if ( &rhs==this )
		return *this;
	m_pDash = rhs.m_pDash;
	m_bBlackhole = rhs.m_bBlackhole;
	m_uAddr = rhs.m_uAddr;
	m_bNeedResolve = rhs.m_bNeedResolve;
	m_bPersistent = rhs.m_bPersistent;
	m_iFamily = rhs.m_iFamily;
	m_sAddr = rhs.m_sAddr;
	m_iPort = rhs.m_iPort;
	return *this;
}

AgentDesc_t &AgentDesc_t::CloneFrom ( const AgentDesc_t &rhs )
{
	if ( &rhs==this )
		return *this;
	CloneFromHost ( rhs );
	m_sIndexes = rhs.m_sIndexes;
	m_pStats = rhs.m_pStats;
	return *this;
}

void cDashStorage::LinkHost ( HostDesc_t &dHost )
{
	assert ( !dHost.m_pDash );
	CSphScopedWLock tWguard ( m_tDashLock );

	// a little trick: simultaneously add new and free deleted entries
	// (with refcount==1, i.e. owned only here)
	// this is why we don't return immediately as found value for addref
	ARRAY_FOREACH ( i, m_dDashes )
	{
		auto pDash = m_dDashes[i];
		if ( pDash->IsLast () )
		{
			m_dDashes.RemoveFast ( i-- ); // remove, and then step back
			SafeRelease ( pDash );
		} else if ( !dHost.m_pDash && dHost.GetMyUrl()==pDash->m_tHost.GetMyUrl() )
			dHost.m_pDash = pDash;
	}

	if ( !dHost.m_pDash )
	{
		dHost.m_pDash = new HostDashboard_t ( dHost );
		m_dDashes.Add ( dHost.m_pDash );
	}

	dHost.m_pDash->AddRef(); // one link here in vec, other returned with the host
}

// Due to very rare template of usage, linear search is quite enough here
HostDashboardPtr_t cDashStorage::FindAgent ( const CSphString & sAgent ) const
{
	CSphScopedRLock tRguard ( m_tDashLock );
	for ( auto * pDash : m_dDashes )
	{
		if ( pDash->IsLast() )
			continue;
		
		if ( pDash->m_tHost.GetMyUrl () == sAgent )
		{
			pDash->AddRef();
			return HostDashboardPtr_t ( pDash );
		}
	}
	return HostDashboardPtr_t(); // not found
}

void cDashStorage::GetActiveDashes ( VecRefPtrs_t<HostDashboard_t *> & dAgents ) const
{
	assert ( dAgents.IsEmpty ());
	CSphScopedRLock tRguard ( m_tDashLock );
	for ( auto * pDash : m_dDashes )
	{
		if ( pDash->IsLast() )
			continue;

		pDash->AddRef ();
		dAgents.Add ( pDash );
	}
}

/// SmartOutputBuffer_t : an event which could be watched by poll/epoll/kqueue
/////////////////////////////////////////////////////////////////////////////
SmartOutputBuffer_t::~SmartOutputBuffer_t ()
{
	m_dChunks.Apply ([] ( ISphOutputBuffer* &pChunk )
	{
		SafeRelease ( pChunk );
	});
}

int SmartOutputBuffer_t::GetSentCount () const
{
	int iSize = 0;
	m_dChunks.Apply ( [&iSize] ( ISphOutputBuffer *&pChunk ) {
		iSize+=pChunk->GetSentCount ();
	} );
	return iSize + m_dBuf.GetLength ();
}

void SmartOutputBuffer_t::StartNewChunk ()
{
	m_dChunks.Add ( new ISphOutputBuffer ( m_dBuf ) );
	m_dBuf.Reserve ( NETOUTBUF );
}

/*
void SmartOutputBuffer_t::AppendBuf ( SmartOutputBuffer_t &dBuf )
{
	if ( !dBuf.m_dBuf.IsEmpty () )
		dBuf.StartNewChunk ();
	for ( auto * pChunk : dBuf.m_dChunks )
	{
		pChunk->AddRef ();
		m_dChunks.Add ( pChunk );
	}
}

void SmartOutputBuffer_t::PrependBuf ( SmartOutputBuffer_t &dBuf )
{
	CSphVector<ISphOutputBuffer *> dChunks;
	if ( !dBuf.m_dBuf.IsEmpty () )
		dBuf.StartNewChunk ();
	for ( auto * pChunk : dBuf.m_dChunks )
	{
		pChunk->AddRef ();
		dChunks.Add ( pChunk );
	}
	dChunks.Append ( m_dChunks );
	m_dChunks.SwapData ( dChunks );
}
*/

void SmartOutputBuffer_t::Reset ()
{
	m_dChunks.Apply ( [] ( ISphOutputBuffer *&pChunk ) {
		SafeRelease ( pChunk );
	} );
	m_dChunks.Reset ();
	m_dBuf.Reset ();
	m_dBuf.Reserve ( NETOUTBUF );
};

#ifndef UIO_MAXIOV
#define UIO_MAXIOV (1024)
#endif

size_t SmartOutputBuffer_t::GetIOVec ( CSphVector<sphIovec> &dOut )
{
	size_t iOutSize = 0;
	dOut.Reset();
	m_dChunks.Apply ( [&dOut, &iOutSize] ( ISphOutputBuffer *&pChunk ) {
		auto& dIovec = dOut.Add();
		IOPTR(dIovec) = IOBUFTYPE ( pChunk->GetBufPtr () );
		IOLEN (dIovec) = pChunk->GetSentCount ();
		iOutSize += IOLEN ( dIovec );
	} );
	if (!m_dBuf.IsEmpty ())
	{
		auto& dIovec = dOut.Add ();
		IOPTR ( dIovec ) = IOBUFTYPE ( GetBufPtr () );
		IOLEN ( dIovec ) = m_dBuf.GetLengthBytes ();
		iOutSize += IOLEN ( dIovec );
	}
	assert ( dOut.GetLength ()<UIO_MAXIOV );
	return iOutSize;
};

/// IOVec_c : wrapper over vector of system iovec/WSABuf
/////////////////////////////////////////////////////////////////////////////
void IOVec_c::Build ()
{
	m_dSource.GetIOVec ( m_dIOVec );
	if ( m_dIOVec.IsEmpty () )
		return;
	m_iIOChunks = ( size_t ) m_dIOVec.GetLength ();
}

void IOVec_c::Reset()
{
	m_dIOVec.Reset();
}

void IOVec_c::StepForward ( size_t uStep )
{
	auto iLen = m_dIOVec.GetLength ();
	for ( ; m_iIOChunks>0; --m_iIOChunks )
	{
		auto &dIOVec = m_dIOVec[iLen - m_iIOChunks];
		if ( uStep<IOLEN( dIOVec ) )
		{
			IOPTR ( dIOVec ) = IOBUFTYPE ( ( BYTE * ) IOPTR ( dIOVec ) + uStep );
			IOLEN( dIOVec ) -= uStep;
			break;
		}
		uStep -= IOLEN ( dIOVec );
	}
}

/// PollableEvent_c : an event which could be watched by poll/epoll/kqueue
/////////////////////////////////////////////////////////////////////////////

void SafeCloseSocket ( int & iFD )
{
	if ( iFD>=0 )
		sphSockClose ( iFD );
	iFD = -1;
}

#if !HAVE_EVENTFD
static bool CreateSocketPair ( int &iSock1, int &iSock2, CSphString &sError )
{
#if USE_WINDOWS
	union {
		struct sockaddr_in inaddr;
		struct sockaddr addr;
	} tAddr;

	int iListen = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if ( iListen<0 )
	{
		sError.SetSprintf ( "failed to create listen socket: %s", sphSockError() );
		return false;
	}

	memset ( &tAddr, 0, sizeof ( tAddr ) );
	tAddr.inaddr.sin_family = AF_INET;
	tAddr.inaddr.sin_addr.s_addr = htonl ( INADDR_LOOPBACK );
	tAddr.inaddr.sin_port = 0;
	auto tCloseListen = AtScopeExit  ( [&iListen] { if ( iListen>=0 ) sphSockClose (iListen); } );

	if ( bind ( iListen, &tAddr.addr, sizeof ( tAddr.inaddr ) )<0 )
	{
		sError.SetSprintf ( "failed to bind listen socket: %s", sphSockError() );
		return false;
	}

	int iAddrBufLen = sizeof ( tAddr );
	memset ( &tAddr, 0, sizeof ( tAddr ) );
	if ( getsockname ( iListen, &tAddr.addr, &iAddrBufLen )<0 )
	{
		sError.SetSprintf ( "failed to get socket description: %s", sphSockError() );
		return false;
	}

	tAddr.inaddr.sin_addr.s_addr = htonl ( INADDR_LOOPBACK );
	tAddr.inaddr.sin_family = AF_INET;

	if ( listen ( iListen, 5 )<0 )
	{
		sError.SetSprintf ( "failed to listen socket: %s", sphSockError() );
		return false;
	}

	int iWrite = socket ( AF_INET, SOCK_STREAM, 0 );
	auto tCloseWrite = AtScopeExit  ( [&iWrite] { if ( iWrite>=0 ) sphSockClose (iWrite); } );

	if ( iWrite<0 )
	{
		sError.SetSprintf ( "failed to create write socket: %s", sphSockError() );
		return false;
	}

	if ( connect ( iWrite, &tAddr.addr, sizeof(tAddr.addr) )<0 )
	{
		sError.SetSprintf ( "failed to connect to loopback: %s\n", sphSockError() );
		return false;
	}

	int iRead = accept ( iListen, NULL, NULL );
	if ( iRead<0 )
	{
		sError.SetSprintf ( "failed to accept loopback: %s\n", sphSockError() );
	}

	iSock1 = iRead;
	iSock2 = iWrite;
	iWrite = -1; // protect from tCloseWrite

#else
	int dSockets[2] = { -1, -1 };
	if ( socketpair ( AF_LOCAL, SOCK_STREAM, 0, dSockets )!=0 )
	{
		sError.SetSprintf ( "failed to create socketpair: %s", sphSockError () );
		return false;
	}

	iSock1 = dSockets[0];
	iSock2 = dSockets[1];

#endif

	if ( sphSetSockNB ( iSock1 )<0 || sphSetSockNB ( iSock2 )<0 )
	{
		sError.SetSprintf ( "failed to set socket non-block: %s", sphSockError () );
		SafeCloseSocket ( iSock1 );
		SafeCloseSocket ( iSock2 );
		iSock1 = iSock2 = -1;
		return false;
	}

#ifdef TCP_NODELAY
	int iOn = 1;
	if ( setsockopt ( iSock2, IPPROTO_TCP, TCP_NODELAY, (char*)&iOn, sizeof ( iOn ) )<0 )
		sphWarning ( "failed to set nodelay option: %s", sphSockError() );
#endif
	return true;
}
#endif
PollableEvent_t::PollableEvent_t ()
{
	int iRead = -1;
	int iWrite = -1;
#if HAVE_EVENTFD
	int iFD = eventfd ( 0, EFD_NONBLOCK );
	if ( iFD==-1 )
		m_sError.SetSprintf ( "failed to create eventfd: %s", strerrorm ( errno ) );
	iRead = iWrite = iFD;
#else
	CreateSocketPair ( iRead, iWrite, m_sError );
#endif

	if (iRead==-1 || iWrite==-1)
		sphWarning ( "PollableEvent_t create error:%s", m_sError.cstr () );
	m_iPollablefd = iRead;
	m_iSignalEvent = iWrite;
}

PollableEvent_t::~PollableEvent_t ()
{
	Close();
}

void PollableEvent_t::Close ()
{
	SafeCloseSocket ( m_iPollablefd );
#if !HAVE_EVENTFD
	SafeCloseSocket ( m_iSignalEvent );
#endif
}

int PollableEvent_t::PollableErrno()
{
	return sphSockGetErrno();
}

bool PollableEvent_t::FireEvent () const
{
	if ( m_iSignalEvent==-1 )
		return true;

	int iErrno = EAGAIN;
	while ( iErrno==EAGAIN || iErrno==EWOULDBLOCK )
	{
		uint64_t uVal = 1;
#if HAVE_EVENTFD
		int iPut = ::write ( m_iSignalEvent, &uVal, 8 );
#else
		int iPut = sphSockSend ( m_iSignalEvent, (const char * )&uVal, 8 );
#endif
		if ( iPut==8 )
			return true;
		iErrno = PollableErrno ();
	};
	return false;
}

// just wipe-out a fired event to free queue, we don't need the value itself
void PollableEvent_t::DisposeEvent () const
{
	assert ( m_iPollablefd!=-1 );
	uint64_t uVal = 0;
	while ( true )
	{
#if HAVE_EVENTFD
		auto iRead = ::read ( m_iPollablefd, &uVal, 8 );
		if ( iRead==8 )
			break;
#else
	// socket-pair case might stack up some values and these should be read
		int iRead = sphSockRecv ( m_iPollablefd, (char *)&uVal, 8 );
		if ( iRead<=0 )
			break;
#endif
	}
}

inline static bool IS_PENDING ( int iErr )
{
#if USE_WINDOWS
	return iErr==ERROR_IO_PENDING || iErr==0;
#else
	return iErr==EAGAIN || iErr==EWOULDBLOCK;
#endif
}

/////////////////////////////////////////////////////////////////////////////
// some extended win-specific stuff
#if USE_WINDOWS

static LPFN_CONNECTEX ConnectEx = nullptr;

#ifndef WSAID_CONNECTEX
#define WSAID_CONNECTEX \
	{0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}}
#endif

bool LoadExFunctions ()
{
	GUID uGuidConnectX = WSAID_CONNECTEX;

	// Dummy socket (for WSAIoctl call)
	SOCKET iSocket = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( iSocket == INVALID_SOCKET )
		return false;

	// fill addr of ConnectX function
	DWORD	m_Bytes;
	auto iRes = WSAIoctl ( iSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&uGuidConnectX, sizeof ( uGuidConnectX ),
		&ConnectEx, sizeof ( ConnectEx ),
		&m_Bytes, NULL, NULL );

	closesocket ( iSocket );
	return ( iRes==0 );
}
#endif

/////////////////////////////////////////////////////////////////////////////
/// AgentConn_t
///
/// Network task: socket, io buffers, query/result parsers,
/// helper structs.
/////////////////////////////////////////////////////////////////////////////
AgentConn_t::~AgentConn_t ()
{
	sphLogDebugA ( "A AgentConn %p destroyed", this );
	if ( m_iSock>=0 )
		Finish ();
}

void AgentConn_t::State ( Agent_e eState )
{
	sphLogDebugv ( "state %s > %s, sock %d, order %d, %p",
		Agent_e_Name ( m_eConnState ), Agent_e_Name ( eState ), m_iSock, m_iStoreTag, this );
	m_eConnState = eState;
}

// initialize socket from persistent pool (it m.b. disconnected or not initialized, however).
bool AgentConn_t::GetPersist ()
{

	if ( !m_tDesc.m_bPersistent || !m_tDesc.m_pDash->m_pPersPool )
		return false;

	assert ( m_iSock==-1 );
	m_iSock = m_tDesc.m_pDash->m_pPersPool->RentConnection ();
	m_tDesc.m_bPersistent = m_iSock!=-2;
	return true;
}

// return socket to the pool (it m.b. connected!)
void AgentConn_t::ReturnPersist ()
{
	if ( m_tDesc.m_bPersistent && m_tDesc.m_pDash->m_pPersPool )
		m_tDesc.m_pDash->m_pPersPool->ReturnConnection ( m_iSock );
	m_iSock = -1;
}

/// 'soft' failure - log and return false
bool AgentConn_t::Fail ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	m_sFailure.SetSprintfVa ( sFmt, ap );
	va_end ( ap );
	sphLogDebugA ( "%d Fail() %s, ref=%d", m_iStoreTag, m_sFailure.cstr (), ( int ) GetRefcount () );
	return false;
}

//! 'hard' failure - close socket, switch to 'RETRY' state, then write stats, log and return false
//! \param eStat - error category
bool AgentConn_t::Fatal ( AgentStats_e eStat, const char * sMessage, ... )
{
	va_list ap;
	va_start ( ap, sMessage );
	m_sFailure.SetSprintfVa ( sMessage, ap );
	va_end ( ap );
	sphLogDebugA ( "%d FATAL: %s", m_iStoreTag, m_sFailure.cstr () );
	State ( Agent_e::RETRY );
	Finish ( true );
	agent_stats_inc ( *this, eStat );
	return false;
}

/// correct way to close connection:
void AgentConn_t::Finish ( bool bFail )
{
	if ( m_iSock>=0 && ( bFail || !m_tDesc.m_bPersistent ) )
	{
		sphLogDebugA ( "%d Socket %d closed and turned to -1", m_iStoreTag, m_iSock );
		sphSockClose ( m_iSock );
		m_iSock = -1;
	}

	if ( m_pPollerTask )
	{
		sphLogDebugA ( "%d Abort all callbacks ref=%d", m_iStoreTag, ( int ) GetRefcount () );
		TimeoutTask (); // remove timer and all callbacks, if any
		m_pPollerTask = nullptr;
	}

	ReturnPersist ();
	if ( m_iStartQuery )
		m_iWall += sphMicroTimer () - m_iStartQuery; // imitated old behaviour
}

//! Failure from successfully ended session
//! (i.e. no network issues, but error in packet itself - like bad syntax, or simple 'try again').
//! so, we don't have to corrupt agent's stat in the case.
void AgentConn_t::BadResult ( int iError )
{
	sphLogDebugA ( "%d BadResult() ref=%d", m_iStoreTag, ( int ) GetRefcount () );
	if ( iError==-1 )
		sphLogDebugA ( "%d text error is %s", m_iStoreTag, m_sFailure.cstr () );
	else if ( iError )
		sphLogDebugA ( "%d error is %d, %s", m_iStoreTag, iError, sphSockError ( iError ) );

	State ( Agent_e::RETRY );
	Finish ();
	m_dResults.Reset ();
}

void AgentConn_t::ReportFinish ( bool bSuccess )
{
	if ( m_pReporter )
		m_pReporter->Report ( bSuccess );
}

/// switch from 'connecting' to 'healthy' state.
/// track the time, modify timeout from 'connect' to 'query', inform poller about it.
void AgentConn_t::SendingState ()
{
	if ( StateIs ( Agent_e::CONNECTING ) )
	{
		track_processing_time ( *this );
		State ( Agent_e::HEALTHY );
		m_iPoolerTimeout = sphMicroTimer () + 1000 * m_iMyQueryTimeout;
		TimeoutTask ( m_iPoolerTimeout ); // assign new time value, don't touch the handler
	}
}

/// for persistent conn take socket from the pool.
/// Set need-to-initiate flag if it is failed/not-connected.
void AgentConn_t::SetupPersist()
{
	if ( !GetPersist() )
		return;

	if ( m_iSock>=0 && sphNBSockEof ( m_iSock ) )
	{
		sphSockClose ( m_iSock );
		m_iSock = -1;
	}
	m_bNeedInitiatePersist = m_iSock==-1;
}

/// prepare all necessary things to connect
/// assume socket is NOT connected
bool AgentConn_t::StartNextRetry ()
{
	sphLogDebugA ( "%d StartNextRetry() retries=%d, ref=%d", m_iStoreTag, m_iRetries, ( int ) GetRefcount () );
	m_iSock = -1;
	if ( m_iRetries--<0 )
		return m_bManyTries ? Fail ( "retries limit exceeded" ) : false;

	m_bNeedInitiatePersist = false;
	if ( m_pMultiAgent )
	{
		m_tDesc.CloneFrom ( m_pMultiAgent->ChooseAgent () );
		SetupPersist ();
	}
	return true;
}

//! Check connection and prepare pre-query messages to be send
//! \return false, if handshake injected and we have to parse answer on it later.
bool AgentConn_t::ConnectionAlive ()
{
	if ( m_iSock>=0 )
		return true;

	m_tOutput.SendDword ( SPHINX_CLIENT_VERSION );
	m_tOutput.StartNewChunk ();

	if ( m_bNeedInitiatePersist )
	{
		m_tOutput.SendWord ( SEARCHD_COMMAND_PERSIST );
		m_tOutput.SendWord ( 0 ); // dummy version
		m_tOutput.SendInt ( 4 ); // request body length
		m_tOutput.SendInt ( 1 ); // set persistent to 1.
		m_tOutput.StartNewChunk ();
		m_bNeedInitiatePersist = false;
	}
	return false;
}

void AgentConn_t::ScheduleCallbacks ()
{
	// that is 'hard' timeout. If it happened, we have to cancel all current IO.
	// Then we need to reschedule (if appliable) the whole work
	TimeoutTask ( m_iPoolerTimeout, [this]
	{
		m_bInNetLoop = true;
		if ( StateIs ( Agent_e::CONNECTING ) )
			Fatal ( eTimeoutsConnect, "connect timed out" );
		else
			Fatal ( eTimeoutsQuery, "query timed out" );
		StartRemoteLoopTry ();
		sphLogDebugA ( "%d <- finished lambda timeout (ref=%d)", m_iStoreTag, ( int ) GetRefcount () );
	}, m_dIOVec.HasUnsent () ? 1 : 2 );
}

void AgentConn_t::ErrorCallback ( int64_t iWaited )
{
	m_bInNetLoop = true;
	m_iWaited += iWaited;
	int iErr = sphSockGetErrno ();
	Fatal ( eNetworkErrors, "detected the error (errno=%d, msg=%s)", iErr, sphSockError ( iErr ) );
	StartRemoteLoopTry ();
}

void AgentConn_t::SendCallback ( int64_t iWaited, DWORD uSent )
{
	if ( !m_pPollerTask )
		return;
	m_bInNetLoop = true;
	if ( m_dIOVec.HasUnsent () )
	{
		m_iWaited += iWaited;
		if ( !SendQuery ( uSent ) )
			StartRemoteLoopTry ();
		sphLogDebugA ( "%d <- finished SendCallback (ref=%d)", m_iStoreTag, ( int ) GetRefcount () );
	}
}

void AgentConn_t::RecvCallback ( int64_t iWaited, DWORD uReceived )
{
	if ( !m_pPollerTask )
		return;
	m_bInNetLoop = true;
	m_iWaited += iWaited;
	if ( !ReceiveAnswer ( uReceived ) )
		StartRemoteLoopTry ();
	sphLogDebugA ( "%d <- finished RecvCallback", m_iStoreTag );
}

/// if iovec is empty, prepare (build) the request.
void AgentConn_t::BuildData ()
{
	if ( m_pBuilder && m_dIOVec.IsEmpty () )
	{
		sphLogDebugA ( "%d BuildData for this=%p, m_pBuilder=%p", m_iStoreTag, this, m_pBuilder );
		// prepare our data to send.
		m_pBuilder->BuildRequest ( *this, m_tOutput );
		m_dIOVec.Build ();
	} else
		sphLogDebugA ( "%d BuildData, already done", m_iStoreTag );
}

//! How many bytes we can read to m_pReplyCur
//! \return available space in bytes
size_t AgentConn_t::ReplyBufPlace () const
{
	if ( !m_pReplyCur )
		return 0;
	if ( m_dReplyBuf.IsEmpty () )
		return REPLY_HEADER_SIZE - ( m_pReplyCur - m_dReplyHeader );
	return m_dReplyBuf.begin () + m_dReplyBuf.GetLength () - m_pReplyCur;
}

void AgentConn_t::InitReplyBuf ( int iSize )
{
	sphLogDebugA ( "%d InitReplyBuf ( %d )", m_iStoreTag, iSize );
	m_dReplyBuf.Reset ( iSize );
	if ( m_dReplyBuf.IsEmpty () )
	{
		m_iReplySize = -1;
		m_pReplyCur = m_dReplyHeader;
	} else {
		m_pReplyCur = m_dReplyBuf.begin ();
		m_iReplySize = iSize;
	}
}

/// raw (platform specific) send (scattered - from several buffers)
inline SSIZE_T AgentConn_t::SendChunk ()
{
#if !USE_WINDOWS
	struct msghdr dHdr = { 0 };
	dHdr.msg_iov = m_dIOVec.IOPtr ();
	dHdr.msg_iovlen = m_dIOVec.IOSize ();
	return ::sendmsg ( m_iSock, &dHdr, MSG_NOSIGNAL );
#else
	SendingState ();
	if (!m_pPollerTask)
		ScheduleCallbacks ();
	m_pPollerTask->m_dWrite.Zero();
	sphLogDebugA ( "%d overlaped WSASend called for %d chunks", m_iStoreTag, m_dIOVec.IOSize () );
	WSASend ( m_iSock, m_dIOVec.IOPtr (), m_dIOVec.IOSize (), nullptr, 0, &m_pPollerTask->m_dWrite, nullptr );
	++m_pPollerTask->m_iRefs;
	return -1;
#endif
}

/// raw (platform specific) receive
inline SSIZE_T AgentConn_t::RecvChunk ()
{
#if !USE_WINDOWS
	return sphSockRecv ( m_iSock, ( char * ) m_pReplyCur, ReplyBufPlace () );
#else
	if ( !m_pPollerTask )
		ScheduleCallbacks ();
	WSABUF dBuf;
	dBuf.buf = (CHAR*) m_pReplyCur;
	dBuf.len = ReplyBufPlace ();
	DWORD uFlags = 0;
	m_pPollerTask->m_dRead.Zero();
	sphLogDebugA ( "%d Scheduling overlapped WSARecv for %d bytes", m_iStoreTag, ReplyBufPlace () );
	WSARecv ( m_iSock, &dBuf, 1, nullptr, &uFlags, &m_pPollerTask->m_dRead, nullptr );
	++m_pPollerTask->m_iRefs;
	return -1;
#endif
}

/// try to establish connection in the modern fast way, and also perform some data sending, if possible.
/// @return 1 on success, 0 if need fallback into usual (::connect), -1 on failure.
//!
int AgentConn_t::DoTFO ( struct sockaddr * pSs, int iLen )
{
	if ( pSs->sa_family==AF_UNIX )
		return 0;
	m_iStartQuery = sphMicroTimer (); // copied old behaviour
#if USE_WINDOWS
	if ( !ConnectEx )
		return 0;

	struct sockaddr_in sAddr;
	ZeroMemory ( &sAddr, sizeof ( sAddr ) );
	sAddr.sin_family = AF_INET;
	sAddr.sin_addr.s_addr = INADDR_ANY;
	sAddr.sin_port = 0;
	int iRes = bind ( m_iSock, (SOCKADDR*) &sAddr, sizeof ( sAddr ) );
	if ( iRes != 0 )
	{
		Fail ( "bind failed: %d %s", m_tDesc.m_sAddr.cstr () );
		return 0;
	}

#if defined ( TCP_FASTOPEN )
	int iOn = 1;
	iRes = setsockopt ( m_iSock, IPPROTO_TCP, TCP_FASTOPEN, (char*) &iOn, sizeof ( iOn ) );
	if ( iRes )
		sphWarning ( "setsockopt (TCP_FASTOPEN) failed: %s", sphSockError () );
	sphLogDebugA ( "%d TFO branch", m_iStoreTag );
	// fixme! ConnectEx doesn't accept scattered buffer. Need to prepare plain one for at least MSS size
#endif

	BuildData ();
	if ( !m_pPollerTask )
		ScheduleCallbacks ();
	sphLogDebugA ( "%d overlaped ConnectEx called", m_iStoreTag );
	m_pPollerTask->m_dWrite.Zero();
	// let us also send first chunk of the buff
	sphIovec * pChunk = m_dIOVec.IOPtr ();
	assert ( pChunk );
	iRes = ConnectEx ( m_iSock, pSs, iLen, pChunk->buf, pChunk->len, NULL, &m_pPollerTask->m_dWrite );
	++m_pPollerTask->m_iRefs;
	if ( iRes )
	{
		State ( Agent_e::CONNECTING );
		return 1;
	}

	int iErr = sphSockGetErrno ();
	if ( !IS_PENDING ( iErr ) )
	{
		Fatal ( eConnectFailures, "ConnectEx failed with %d, %s", iErr, sphSockError ( iErr ) );
		return -1;
	}
	State ( Agent_e::CONNECTING );
	return 1;
#else // USE_WINDOWS
#if defined (MSG_FASTOPEN)

	BuildData ();
	struct msghdr dHdr = { 0 };
	dHdr.msg_iov = m_dIOVec.IOPtr ();
	dHdr.msg_iovlen = m_dIOVec.IOSize ();
	dHdr.msg_name = pSs;
	dHdr.msg_namelen = iLen;

	auto iRes = ::sendmsg ( m_iSock, &dHdr, MSG_NOSIGNAL | MSG_FASTOPEN );
#elif defined (__APPLE__)
	struct sa_endpoints sAddr = { 0 };
	sAddr.sae_dstaddr = pSs;
	sAddr.sae_dstaddrlen = iLen;

//	BuildData ();
	auto iRes = connectx ( m_iSock, &sAddr, SAE_ASSOCID_ANY, CONNECT_RESUME_ON_READ_WRITE | CONNECT_DATA_IDEMPOTENT
						   , nullptr, 0, nullptr, nullptr );
	if ( !iRes )
		State ( Agent_e::CONNECTING );
#else
	int iRes = 0;
	return iRes;
#endif
	if ( iRes>=0 ) // lucky; we already sent something!
	{
		track_processing_time ( *this );
		sphLogDebugA ( "%d sendmsg/connectx returned %zu", m_iStoreTag, ( size_t ) iRes );
		sphLogDebugv ( "TFO send succeeded, %zu bytes sent", ( size_t ) iRes );
		// now 'connect' and 'query' merged, so timeout became common.
		m_iPoolerTimeout += 1000*m_iMyQueryTimeout;
		return SendQuery ( iRes ) ? 1 : -1;
	}

	auto iErr = sphSockGetErrno ();
	if ( iErr!=EINPROGRESS )
	{
		Fatal ( eConnectFailures, "sendmsg/connectx() failed: errno=%d, %s", iErr, sphSockError ( iErr ) );
		return -1;
	}
	sphLogDebugA ( "%d TFO returned EINPROGRESS (usuall connect in game; scheduling callbacks)", m_iStoreTag );
	ScheduleCallbacks ();
	return 1;
#endif
}

/// add works to the queue; return int cookie for future references to this jobs group.
/// jobs themselves are ref-counted and owned by nobody (they're just released on finish, so
/// if nobody waits them (say, blackhole), they just dissapeared).
/// on return blackholes removed from dRemotes
void ScheduleDistrJobs ( VectorAgentConn_t &dRemotes, IRequestBuilder_t * pQuery, IReplyParser_t * pParser
						 , IRemoteAgentsObserver * pReporter, int iQueryRetry, int iQueryDelay )
{
//	sphLogSupress ( "L ", SPH_LOG_VERBOSE_DEBUG );
//	sphLogSupress ( "- ", SPH_LOG_VERBOSE_DEBUG );
	StartLogTime();
	sphLogDebugv ( "S ==========> ScheduleDistrJobs() for %d remotes", dRemotes.GetLength () );

	bool bNeedKick = false; // if some of connections falled to waiting and need to kick the poller.
	int iManaged = 0;
	for ( AgentConn_t *&pConnection : dRemotes )
	{
		pConnection->InitRemoteTask ( pQuery, pParser, ( IReporter_t * ) pReporter, iQueryRetry, iQueryDelay );
		if ( pConnection->m_pReporter )
			++iManaged;
		pConnection->StartRemoteLoopTry (true);
		bNeedKick |= pConnection->NeedKick ();
	}
	if ( pReporter )
		pReporter->SetTotal ( iManaged );

	// now wipe out blackholes - they're live own life.
	ARRAY_FOREACH ( i, dRemotes )
		if ( dRemotes[i]->IsBl () )
		{
			sphLogDebugv ( "S Removed blackhole()" );
			SafeRelease ( dRemotes[i] );
			dRemotes.RemoveFast ( i-- );
		}

	if ( bNeedKick )
	{
		sphLogDebugA ( "A Events need fire. Do it..." );
		FirePoller ();
	}

	sphLogDebugv ( "S ScheduleDistrJobs() done. Total %d, remotes rest %d", iManaged, dRemotes.GetLength () );

}

// this is run once entering query loop for all retries (and all mirrors).
void AgentConn_t::InitRemoteTask ( IRequestBuilder_t * pQuery, IReplyParser_t * pParser,
	IReporter_t * pReporter, int iQueryRetry, int iQueryDelay )
{
	sphLogDebugA ( "%d InitRemoteTask() pBuilder %p, pParser %p, retry %d, delay %d ref=%d", m_iStoreTag, pQuery
				   , pParser, iQueryRetry, iQueryDelay, ( int ) GetRefcount ());
	if ( iQueryRetry>=0 )
		SetRetryLimit ( iQueryRetry );
	if ( iQueryDelay>=0 )
		m_iDelay = iQueryDelay;

	m_pBuilder = pQuery;
	State ( Agent_e::HEALTHY );
	m_iWall = 0;
	m_iWaited = 0;
	m_bInNetLoop = false;
	m_bNeedKick = false;
	m_pPollerTask = nullptr;
	StartNextRetry ();

	if ( IsBl () ) // for blackholes we have only 1 try, and don't need to parse the answer.
	{
		sphLogDebugA ( "%d conn %p is blackhole (host %s)", m_iStoreTag, this, m_tDesc.m_sAddr.cstr () );
		m_pReporter = nullptr;
		m_pParser = nullptr;
		m_iRetries = -1;
	} else
	{
		sphLogDebugA ( "%d conn %p is normal (host %s)", m_iStoreTag, this, m_tDesc.m_sAddr.cstr () );
		m_pParser = pParser;
		m_pReporter = pReporter;
	}
}


/// an entry point to the whole remote agent's work
void AgentConn_t::StartRemoteLoopTry ( bool bSkipFirstRetry )
{
	sphLogDebugA ( "%d StartRemoteLoopTry() ref=%d", m_iStoreTag, ( int ) GetRefcount () );
	while ( bSkipFirstRetry || StartNextRetry () )
	{
		/// reset state before every retry
		bSkipFirstRetry = false;
		m_dIOVec.Reset ();
		m_tOutput.Reset ();
		InitReplyBuf ();
		m_bConnectHandshake = true;
		m_bSuccess = false;
		m_iStartQuery = 0;
		m_pPollerTask = nullptr;

		if ( StateIs ( Agent_e::RETRY ) && m_iDelay>0 )
		{
			// can't start right now; need to postpone until timeout
			sphLogDebugA ( "%d postpone DoQuery() for %d msecs", m_iStoreTag, m_iDelay );
			TimeoutTask ( sphMicroTimer () + 1000 * m_iDelay, [this] {
				m_bInNetLoop = true;
				State ( Agent_e::HEALTHY );
				if ( !DoQuery () )
					StartRemoteLoopTry ();
				FirePoller ();
				sphLogDebugA ( "%d finished lambda timeout from postponer ref=%d", m_iStoreTag, ( int ) GetRefcount () );
			} );
			return;
		}
		if ( DoQuery () )
			return;
	};
	ReportFinish ( false );
	sphLogDebugA ( "%d StartRemoteLoopTry() finished ref=%d", m_iStoreTag, ( int ) GetRefcount () );
}

bool AgentConn_t::DoQuery()
{
	sphLogDebugA ( "%d DoQuery() ref=%d", m_iStoreTag, ( int ) GetRefcount () );
	auto iNow = sphMicroTimer ();
	if ( ConnectionAlive () )
	{
		sphLogDebugA ( "%d branch for established(%d). Timeout %d", m_iStoreTag, m_iSock, m_iMyQueryTimeout );
		m_bConnectHandshake = false;
		m_pReplyCur += sizeof ( int );
		m_iStartQuery = iNow; /// copied from old behaviour
		m_iPoolerTimeout = iNow + 1000 * m_iMyQueryTimeout;
		return SendQuery ();
	}

	sphLogDebugA ( "%d branch for not established. Timeout %d", m_iStoreTag, m_iMyConnectTimeout );
	m_iPoolerTimeout = iNow + 1000 * m_iMyConnectTimeout;
	if ( !m_tDesc.m_bNeedResolve )
		return EstablishConnection ();

	// here we're in case agent's remote addr need to be resolved (DNS required)
	assert ( m_tDesc.m_iFamily==AF_INET );
	AddRef ();
	sphLogDebugA ( "%d -> async GetAddress_a scheduled() ref=%d", m_iStoreTag, ( int ) GetRefcount () );
	DNSResolver_c::GetAddress_a ( m_tDesc.m_sAddr.cstr (), [this] ( DWORD uIP )
	{
		sphLogDebugA ( "%d :- async GetAddress_a callback (ip is %u) ref=%d", m_iStoreTag, uIP, ( int ) GetRefcount () );
		m_tDesc.m_uAddr = uIP;
		if ( !EstablishConnection () )
			StartRemoteLoopTry ();
		sphLogDebugA ( "%d <- async GetAddress_a returned() ref=%d", m_iStoreTag, ( int ) GetRefcount () );
		if ( NeedKick () )
			FirePoller ();
		Release ();
	} );

	// for blackholes we parse query immediately, since builder will be disposed
	// outside once we returned from the function
	if ( IsBl () )
		BuildData ();
	return true;
}

// here ip resolved; socket is NOT connected.
// We can initiate connect, or even send the chunk using TFO.
bool AgentConn_t::EstablishConnection ()
{
	sphLogDebugA ( "%d EstablishConnection() ref=%d", m_iStoreTag, ( int ) GetRefcount () );
	// first check if we're in bounds of timeout.
	// usually it is done by outside callback, however in case of deffered DNS we may be here out of sync and so need
	// to check it explicitly.
	if ( m_iPoolerTimeout<sphMicroTimer () )
		return Fatal ( eConnectFailures, "connect timeout reached resolving address for %s", m_tDesc.m_sAddr.cstr () );

	if ( m_tDesc.m_iFamily==AF_INET && !m_tDesc.m_uAddr )
		return Fatal ( eConnectFailures, "can't get address for %s", m_tDesc.m_sAddr.cstr () );

	assert (m_iSock==-1); ///< otherwize why we're here?

	socklen_t len = 0;
	sockaddr_storage ss = {0};
	ss.ss_family = m_tDesc.m_iFamily;

	if ( ss.ss_family==AF_INET )
	{
		auto * pIn = ( struct sockaddr_in * ) &ss;
		pIn->sin_port = htons ( ( unsigned short ) m_tDesc.m_iPort );
		pIn->sin_addr.s_addr = m_tDesc.m_uAddr;
		len = sizeof ( *pIn );
	}
#if !USE_WINDOWS
	else if ( ss.ss_family==AF_UNIX )
	{
		auto * pUn = ( struct sockaddr_un * ) &ss;
		strncpy ( pUn->sun_path, m_tDesc.m_sAddr.cstr (), sizeof ( pUn->sun_path ) );
		len = sizeof ( *pUn );
	}
#endif

	m_iSock = socket ( m_tDesc.m_iFamily, SOCK_STREAM, 0 );
	sphLogDebugA ( "%d Created new socket %d", m_iStoreTag, m_iSock );

	if ( m_iSock<0 )
		return Fatal ( eConnectFailures, "socket() failed: %s", sphSockError () );

	if ( sphSetSockNB ( m_iSock )<0 )
		return Fatal ( eConnectFailures, "sphSetSockNB() failed: %s", sphSockError () );

	// prepare our data to send.
	auto iTfoRes = DoTFO ( ( struct sockaddr * ) &ss, len );
	if ( iTfoRes==1 )
		return true;
	else if ( iTfoRes==-1 )
		return false;

	m_iStartQuery = sphMicroTimer (); // copied old behaviour
	sphLogDebugA ( "%d usual ::connect invoked for %d", m_iStoreTag, m_iSock );
	int iRes = ::connect ( m_iSock, ( struct sockaddr * ) &ss, len );
	if ( iRes<0 )
	{
		int iErr = sphSockGetErrno ();
		if ( !( IS_PENDING(iErr) || iErr==EINTR ) ) // check for EWOULDBLOCK is for winsock only
			return Fatal ( eConnectFailures, "connect() failed: errno=%d, %s", iErr, sphSockError ( iErr ) );
	}

	// connection in progress
	State ( Agent_e::CONNECTING );
	return SendQuery ();
}

/// send query (whole, or chunk)
/// if data is sent by external routine, param says how many bytes are sent.
bool AgentConn_t::SendQuery ( DWORD uSent )
{
	sphLogDebugA ( "%d SendQuery() ref=%d", m_iStoreTag, ( int ) GetRefcount () );

	// here we have connected socket and are in process of sending blob there.
	// prepare our data to send.
	if ( !uSent )
		BuildData ();
	SSIZE_T iRes = 0;
	while ( m_dIOVec.HasUnsent () )
	{
		if ( !uSent )
			iRes = SendChunk ();
		else
		{
			iRes = uSent;
			uSent = 0;
		}

		if ( iRes==-1 )
			break;
		sphLogDebugA ( "%d sent %d bytes", m_iStoreTag, (int)iRes );
		m_dIOVec.StepForward ( iRes );
		if ( iRes>0 )
			SendingState ();
	}

	if ( !m_dIOVec.HasUnsent () ) // we've abandoned output queue
	{
		sphLogDebugA ( "%d sending finished", m_iStoreTag );
		DisableWrite();
		return ReceiveAnswer ();
	}

	assert ( iRes==-1 );

	int iErr = sphSockGetErrno ();
	if ( !IS_PENDING(iErr) && iErr!=EINPROGRESS )
	{
		if ( !( iErr==ENOTCONN && StateIs ( Agent_e::CONNECTING ) ) )
			return Fatal ( eNetworkErrors, "error when sending data: %s", sphSockError ( iErr ) );
		else
			sphLogDebugA ( "%d Not connected, schedule... ref=%d", m_iStoreTag, ( int ) GetRefcount () );
	}

	sphLogDebugA ( "%d -> Schedule sender ref=%d", m_iStoreTag, ( int ) GetRefcount () );
	ScheduleCallbacks ();
	return true;
}

// here we fall when we're expect to read something from remote side
bool AgentConn_t::ReceiveAnswer ( DWORD uRecv )
{
	sphLogDebugA ( "%d ReceiveAnswer() ref=%d", m_iStoreTag, ( int ) GetRefcount () );
	// check if we just read anything, of already read something and have to continue.
	SSIZE_T iRes = 0;
	while ( ReplyBufPlace () )
	{
		if ( uRecv )
		{
			iRes = uRecv;
			uRecv = 0;
		} else
			iRes = RecvChunk();

		if ( iRes<=0 )
			break;

		m_pReplyCur += iRes;
		auto iRest = ReplyBufPlace ();
		sphLogDebugA ( "%d RecvChunk returned %d (%zu bytes rest in input buffer)", m_iStoreTag, ( int ) iRes, iRest );

		// We're in state of receiving the header (m_iReplySize==-1 is the indicator)
		if ( IsReplyHeader () && iRest<=( REPLY_HEADER_SIZE - 4 ))
		{
			MemInputBuffer_c dBuf ( m_dReplyHeader, REPLY_HEADER_SIZE );
			auto iVer = dBuf.GetInt ();
			sphLogDebugA ( "%d Handshake is %d (this message may appear >1 times)", m_iStoreTag, iVer );

			// parse handshake answer (if necessary)
			if ( m_bConnectHandshake && iVer!=SPHINX_SEARCHD_PROTO && iVer!=0x01000000UL )
				return Fatal ( eWrongReplies, "handshake failure (unexpected protocol version=%d)", iVer );

			if ( !iRest ) // not only handshake, but whole header is here
			{
				auto uStat = dBuf.GetWord ();
				auto uVer = dBuf.GetWord (); // there is version here. But it is not used.
				auto iReplySize = dBuf.GetInt ();

				sphLogDebugA ( "%d Header (Status=%d, Version=%d, answer need %d bytes)", m_iStoreTag, uStat, uVer, iReplySize );

				if ( iReplySize<0
					|| iReplySize>g_iMaxPacketSize ) // FIXME! add reasonable max packet len too
					return Fatal ( eWrongReplies, "invalid packet size (status=%d, len=%d, max_packet_size=%d)"
								   , uStat, iReplySize, g_iMaxPacketSize );

				// allocate buf for reply
				InitReplyBuf ( iReplySize );
				m_eReplyStatus = ( SearchdStatus_e ) uStat;
			}
		}
	}

	if ( !ReplyBufPlace () ) // we've received full reply
	{
		auto bRes = CommitResult ();
		if ( bRes )
			ReportFinish ( true );
		return bRes;
	}

	if ( !iRes ) // timeout or eof happens; retry.
		return Fatal ( eUnexpectedClose, "agent closed connection" );

	assert ( iRes==-1 );

	int iErr = sphSockGetErrno ();
	if ( !IS_PENDING ( iErr ) )
		return Fatal ( eNetworkErrors, "receiving failure (errno=%d, msg=%s)", iErr, sphSockError ( iErr ) );

	ScheduleCallbacks();
	return true;
}

// when full blob with expected size is received...
// just a fine: parse the answer, collect results, dispose agent as one is done.
bool AgentConn_t::CommitResult ()
{
	sphLogDebugA ( "%d CommitResult() ref=%d, parser %p", m_iStoreTag, ( int ) GetRefcount (), m_pParser );
	if ( !m_pParser )
	{
		Finish();
		return true;
	}

	MemInputBuffer_c tReq ( m_dReplyBuf.Begin (), m_iReplySize );

	if ( m_eReplyStatus == SEARCHD_RETRY )
	{
		m_sFailure.SetSprintf ( "remote warning: %s", tReq.GetString ().cstr () );
		BadResult ( -1 );
		return false;
	}

	if ( m_eReplyStatus == SEARCHD_ERROR )
	{
		m_sFailure.SetSprintf ( "remote error: %s", tReq.GetString ().cstr () );
		BadResult ( -1 );
		return false;
	}

	bool bWarnings = ( m_eReplyStatus == SEARCHD_WARNING );
	if ( bWarnings )
		m_sFailure.SetSprintf ( "remote warning: %s", tReq.GetString ().cstr () );

	if ( !m_pParser->ParseReply ( tReq, *this ) )
	{
		BadResult ();
		return false;
	}

	Finish();
	m_bSuccess = true;

	if ( !bWarnings )
		bWarnings = m_dResults.FindFirst ( [] ( const CSphQueryResult &dRes ) { return !dRes.m_sWarning.IsEmpty(); } );

	agent_stats_inc ( *this, bWarnings ? eNetworkCritical : eNetworkNonCritical );
	return true;
}


void AgentConn_t::SetMultiAgent ( const CSphString &sIndex, MultiAgentDesc_c * pAgent )
{
	m_pMultiAgent = pAgent;
	m_iMirrorsCount = pAgent->GetLength ();
	SetRetryLimit ( pAgent->GetRetryLimit (), false );
}

//! Initializes num of tries during networking
void AgentConn_t::SetRetryLimit ( int iValue, bool bTryPerMirror )
{
	m_iRetries = iValue;
	if ( bTryPerMirror )
		m_iRetries *= m_iMirrorsCount;
	m_bManyTries = m_iRetries>0;
}
#if 0

// here is async dns resolution made on mac os

//#include <CoreFoundation/CoreFoundation.h>
#include <CFNetwork/CFNetwork.h>

// just a template based on https://developer.apple.com/library/content/documentation/NetworkingInternet/Conceptual/NetworkingTopics/Articles/ResolvingDNSHostnames.html
void clbck ( CFHostRef theHost, CFHostInfoType typeInfo
			 , const CFStreamError * error
			 , void * info)
{
	Boolean bOk;
	auto arr = CFHostGetAddressing( theHost, &bOk );
	if ( bOk )
	{
		auto iSize = CFArrayGetCount ( arr );
		for ( auto i=0; i<iSize; ++i )
		{
			auto * pValue = ( CFDataRef )CFArrayGetValueAtIndex (arr, i);
			struct sockaddr_in * remoteAddr = ( struct sockaddr_in * ) CFDataGetBytePtr ( pValue );

		}

	}
}

// see also https://stackoverflow.com/questions/1363787/is-it-safe-to-call-cfrunloopstop-from-another-thread

void macresolver()
{
	CFStringRef a = CFStringCreateWithCString ( kCFAllocatorDefault, "hello.world.com", kCFStringEncodingUTF8 );
	auto hostRef = CFHostCreateWithName ( kCFAllocatorDefault, a );
	CFRelease (a);

	CFHostClientContext ctx { 0 };
	// ctx.info = this
	Boolean set_ok = CFHostSetClient ( hostRef, &clbck, &ctx );

	CFHostScheduleWithRunLoop ( hostRef, CFRunLoopGetCurrent (), kCFRunLoopCommonModes );

	CFStreamError sError;
	Boolean is_resolving = CFHostStartInfoResolution ( hostRef, kCFHostAddresses, &sError );
	// CFRelease ( hostRef ); // fixme! where to put it? Here? Or in callback?
}

#endif

/// wrap raw void* into ListNode_t to store it in List_t
struct ListedData_t : public ListNode_t
{
	const void * m_pData = nullptr;

	explicit ListedData_t ( const void * pData )
		: m_pData ( pData )
	{}
};


struct Task_t
#if USE_WINDOWS
	: DoubleOverlapped_t
#endif
{
	int					m_ifd = -1;
	int					m_iTimeoutIdx = -1;	// idx inside timeouts bin heap (or -1 if not there)
	int64_t				m_iTimeoutTime = 0;	// timeout (used for bin heap morph)
	Clbck_f				m_fTimeoutAction = nullptr;
	AgentConn_t *		m_pPayload = nullptr;	// ext conn we hold
	BYTE				m_uIOActive = 0;		// active IO callbacks: 0-none, 1-r+w, 2-r
	BYTE				m_uIOChanged = 0;		// need IO changes: dequeue (if !m_uIOActive), 1-set to rw, 2-set to ro
};

inline static bool operator< (const Task_t& dLeft, const Task_t& dRight )
{
	return dLeft.m_iTimeoutTime<dRight.m_iTimeoutTime;
}

/// priority queue for timeouts - as CSphQueue, but specific (can resize, stores internal index in an object)
class TimeoutQueue_c
{
	CSphTightVector<Task_t*> m_dQueue;

	inline void ShiftUp ( int iHole )
	{
		if ( m_dQueue.IsEmpty () )
			return;
		int iParent = ( iHole - 1 ) / 2;
		// shift up if needed, so that worst (lesser) ones float to the top
		while ( iHole && *m_dQueue[iHole]<*m_dQueue[iParent] )
		{
			Swap ( m_dQueue[iHole], m_dQueue[iParent] );
			m_dQueue[iHole]->m_iTimeoutIdx = iHole;
			iHole = iParent;
			iParent = ( iHole - 1 ) / 2;
		}
		m_dQueue[iHole]->m_iTimeoutIdx = iHole;
	};

	inline void ShiftDown ( int iHole )
	{
		if ( m_dQueue.IsEmpty () || iHole==m_dQueue.GetLength () )
			return;
		auto iMinChild = iHole * 2 + 1;
		auto iUsed = m_dQueue.GetLength ();
		while ( iMinChild<iUsed )
		{
			// select smallest child
			if ( iMinChild + 1<iUsed && *m_dQueue[iMinChild + 1]<*m_dQueue[iMinChild] )
				++iMinChild;

			// if smallest child is less than entry, do float it to the top
			if ( *m_dQueue[iHole]<*m_dQueue[iMinChild] )
				break;

			Swap ( m_dQueue[iHole], m_dQueue[iMinChild] );
			m_dQueue[iHole]->m_iTimeoutIdx = iHole;
			iHole = iMinChild;
			iMinChild = iHole * 2 + 1;
		}
		m_dQueue[iHole]->m_iTimeoutIdx = iHole;
	}

public:
	void Push ( Task_t * pTask )
	{
		if ( !pTask )
			return;

		m_dQueue.Add ( pTask );
		ShiftUp ( m_dQueue.GetLength () - 1 );
	}

	/// remove root (ie. top priority) entry
	void Pop ()
	{
		if ( m_dQueue.IsEmpty() )
			return;

		m_dQueue[0]->m_iTimeoutIdx = -1;
		m_dQueue.RemoveFast (0);
		ShiftDown(0);
	}

	/// entry m.b. already added, but timeout changed and it need to be rebalanced
	void Change ( Task_t * pTask )
	{
		if ( !pTask )
			return;

		auto iHole = pTask->m_iTimeoutIdx;
		if ( iHole<0 )
		{
			Push ( pTask );
			return;
		}

		if ( iHole && *m_dQueue[iHole]<*m_dQueue[( iHole - 1 ) / 2] )
			ShiftUp ( iHole );
		else
			ShiftDown ( iHole );
	}

	/// erase elem using stored idx
	void Remove ( Task_t * pTask )
	{
		if ( !pTask )
			return;

		auto iHole = pTask->m_iTimeoutIdx;
		if ( iHole<0 || iHole>=m_dQueue.GetLength () )
			return;

		m_dQueue.RemoveFast ( iHole );
		if ( iHole<m_dQueue.GetLength () )
		{
			if ( iHole && iHole<m_dQueue.GetLength() && *m_dQueue[iHole]<*m_dQueue[( iHole - 1 ) / 2] )
				ShiftUp ( iHole );
			else
				ShiftDown ( iHole );
		}
		pTask->m_iTimeoutIdx = -1;
	}

	inline bool IsEmpty () const
	{
		return m_dQueue.IsEmpty ();
	}

	/// get minimal (root) elem
	inline Task_t * Root () const
	{
		if ( m_dQueue.IsEmpty () )
			return nullptr;
		return m_dQueue[0];
	}

	CSphString DebugDump ( const char * sPrefix ) const
	{
		StringBuilder_c tBuild;
		for ( auto * cTask : m_dQueue )
			tBuild.Appendf ( tBuild.IsEmpty ()?"%p(" INT64_FMT ")":", %p(" INT64_FMT ")", cTask, cTask->m_iTimeoutTime);
		CSphString sRes;
		if ( !m_dQueue.IsEmpty () )
			sRes.SetSprintf ("%s%d:%s", sPrefix, m_dQueue.GetLength (),tBuild.cstr());
		else
			sRes.SetSprintf ( "%sHeap empty.", sPrefix );
		return sRes;
	}
};

ThreadRole LazyThread;

// low-level ops depends from epoll/kqueue/iocp availability
class NetEventsFlavour_c
{
protected:
	int m_iEvents = 0;    ///< how many events are in queue.

	// schedule all actions a time...
	// on kqueue we don't need to add them one-by-one
	void poll_batch_action ( Task_t * pTask )
	{
		auto iEvents = add_change_task ( pTask );
		m_iEvents += iEvents;
		pTask->m_uIOActive = pTask->m_uIOChanged;
		pTask->m_uIOChanged = 0;
		sphLogDebugL ( "L poll_one_action returned %d, now %d events counted", iEvents, m_iEvents );
	}

#if USE_WINDOWS
						  // for windows it is one more level of indirection:
						  // we set and wait for the tasks in one thread,
						  // and iocp also have several working threads.
	HANDLE				m_IOCP = INVALID_HANDLE_VALUE;
	CSphVector<DWORD>	m_dIOThreads;
	CSphVector<OVERLAPPED_ENTRY>	m_dReady;
	LPOVERLAPPED_ENTRY	m_pEntry = nullptr;

	inline void poll_create ( int iSizeHint )
	{
		// fixme! m.b. more workers, or just one enough?
		m_IOCP = CreateIoCompletionPort ( INVALID_HANDLE_VALUE, NULL, 0, 1 );
		sphLogDebugL ( "L IOCP %d created", m_IOCP );

	}

	inline void poll_destroy ()
	{
		sphLogDebugv ( "iocp poller %d closed", m_IOCP );
		// that is have to be done only when ALL reference sockets are closed.
		// what about persistent? Actually we have to first close them, then close iocp.
		// m.b. on finish of the daemon that is not so important, but just mentioned to be known.
		CloseHandle ( m_IOCP );
	}

	inline void fire_event ()
	{
		if ( !PostQueuedCompletionStatus ( m_IOCP, 0, 0, 0 ) )
			sphLogDebugv ( "L PostQueuedCompletionStatus failed with error %d", GetLastError () );
	}

	// each action added one-by-one...
	int add_change_task ( Task_t * pTask )
	{
		bool bApply = pTask->m_ifd!=-1;
		bool bWrite = pTask->m_uIOChanged==1;
		bool bRead = ( pTask->m_uIOChanged!=0 );
		bool bWasWrite = pTask->m_uIOActive==1;
		bool bWasRead = ( pTask->m_uIOActive!=0 );

		if ( !bRead && !bWrite ) // requested delete
		{
			sphLogDebugL ( "L request to remove event (%d), %d events rest", pTask->m_ifd, m_iEvents );
			return pTask->m_ifd==-1 ? -1 : 0;
			/*
			Hackers way to unbind from IOCP:

			Call NtSetInformationFile with the FileReplaceCompletionInformationenumerator value for 
			FileInformationClass and a pointer to a FILE_COMPLETION_INFORMATION structure for the FileInformation parameter. 
			In this structure, set the Port member to NULL (or nullptr, in C++) to disassociate the file from the port it's
			currently attached to (I guess if it isn't attached to any port, nothing would happen), or set Port to a valid
			HANDLE to another completion port to associate the file with that one instead.

			However it 1-st, require win >=8.1, and also invoke DDK stuff which is highly non-desirable. So, leave it 'as is'.
			*/
		}

		if ( !bWasWrite && !bWasRead )
		{
			sphLogDebugL ( "L Associate %d with iocp %d, %d events before", pTask->m_ifd, m_IOCP, m_iEvents );
			if ( !CreateIoCompletionPort ( (HANDLE) pTask->m_ifd, m_IOCP, (ULONG_PTR) pTask->m_ifd, 0 ) )
				sphLogDebugv ( "L Associate %d with port %d failed with error %d", pTask->m_ifd, m_IOCP, GetLastError () );
			return 1;
		}
		sphLogDebugL ( "L According to state, %d already associated with iocp %d, no action", pTask->m_ifd, m_IOCP );
		return 0;
	}

	inline void poll_correct_events ( BYTE uIOmode )
	{
		sphLogDebugL ( "L poll_correct_events(%d) invoked", uIOmode );
		if ( uIOmode )
			--m_iEvents;
	}

// 	void unsubscribecpio ( Task_t * pTask )
// 	{
// 		auto tmpIOCP = CreateIoCompletionPort ( INVALID_HANDLE_VALUE, NULL, 0, 1 );
// 		sphLogDebugL ( "L temporary IOCP %d created", tmpIOCP );
// 		if ( !CreateIoCompletionPort ( (HANDLE) pTask->m_ifd, tmpIOCP, (ULONG_PTR) pTask, 0 ) )
// 			sphLogDebugv ( "L Associate %d with port %d failed with error %d", pTask->m_ifd, tmpIOCP, GetLastError () );
// 		CloseHandle ( tmpIOCP );
// 	}

	// always return 0 (timeout) or 1 (since iocp returns per event, not the bigger group).
	inline int poller_wait ( int64_t iTimeoutUS )
	{
		ULONG uReady = 0;
		DWORD uTimeout = ( iTimeoutUS>=0 ) ? ( iTimeoutUS/1000 ) : INFINITE;
		m_dReady.Resize ( m_iEvents+1 ); // +1 since our signaler is not added as resident of the queue
		if ( !GetQueuedCompletionStatusEx ( m_IOCP, m_dReady.Begin (), m_dReady.GetLength (), &uReady, uTimeout, FALSE ) )
		{
			auto iErr = GetLastError ();
			if ( iErr==WAIT_TIMEOUT )
				return 0;

			sphLogDebugL ( "L GetQueuedCompletionStatusEx failed with error %d", iErr );
			return 0;
		}
		return uReady;
	}

	// returns task and also selects current event for all the functions below
	inline Task_t * ready_iterate ( int iReady )
	{
		if ( iReady>=0 )
			m_pEntry = &m_dReady[iReady];
		assert ( m_pEntry );
		if ( !m_pEntry->lpCompletionKey )
			return nullptr;

		auto pOvl = (SingleOverlapped_t *) m_pEntry->lpOverlapped;
		auto pTask = (Task_t *) ( (BYTE*) pOvl - pOvl->m_uParentOffset );
		--pTask->m_iRefs;
		return pTask;
	}

	inline bool ready_signaler ( Task_t * pTask )
	{
		assert ( m_pEntry );
		if ( !pTask )
			return true;

		if ( pTask->m_ifd==-1 && pTask->m_pPayload==nullptr )
		{
			sphLogDebugL ( "L ready_signaler for deffered %p, refs=%d", pTask, (int)pTask->m_iRefs );
			if ( pTask->m_iRefs<=0 )
				delete pTask;
			return true;
		}
		return false;
	}

	inline int ready_events()
	{
		assert ( m_pEntry );
		// return 0 for internal signal, or 1 for write, 1+sizeof(OVERLAPPED) for read.
		return ( m_pEntry->lpCompletionKey ? 1 : 0 ) + ( (SingleOverlapped_t*) m_pEntry->lpOverlapped )->m_uParentOffset;
	}

	inline bool ready_error()
	{
		return false;
//		assert ( m_pEntry );
	//	return ( m_pEntry->dwNumberOfBytesTransferred==0 );
		//auto & dEvent = m_dReady[iReady];
		//bool bRes = ( dEvent.flags & EV_ERROR )!=0;
		//if ( bRes )
		//			sphLogDebugL ( "L error for %d, errno=%d, %s", dEvent.ident, dEvent.data, sphSockError ( dEvent.data ) );
		//	return bRes;
	}

	inline bool ready_eof()
	{
		assert ( m_pEntry );
		return ready_read() && !ready_transferred();
	}

	inline int delta_ovl()
	{
		// return 0 for internal signal, or 1 for write, 1+sizeof(OVERLAPPED) for read.
		return ( (SingleOverlapped_t*) m_pEntry->lpOverlapped )->m_uParentOffset;
	}

	inline bool ready_read()
	{
		assert ( m_pEntry );
		return !ready_write();
	}

	inline bool ready_write()
	{
		assert ( m_pEntry );
		return delta_ovl ()<sizeof ( OVERLAPPED );
	}

	inline DWORD ready_transferred ()
	{
		assert ( m_pEntry );
		return m_pEntry->dwNumberOfBytesTransferred;
	}

#else
	int m_iEFD = -1;
	PollableEvent_t m_dSignaler;
	Task_t			m_dSignalerTask;

	inline void poll_create ( int iSizeHint )
	{
		poll_create_impl ( iSizeHint );
		m_dSignalerTask.m_ifd = m_dSignaler.m_iPollablefd;
		sphLogDebugv( "Add internal signaller");
		poll_batch_action ( &m_dSignalerTask );
		sphLogDebugv ( "Internal signal action (for epoll/kqueue) added (%d), %p",
			m_dSignaler.m_iPollablefd, &m_dSignalerTask );

	}

	inline bool ready_signaler ( Task_t * pTask )
	{
		if ( pTask==&m_dSignalerTask )
			m_dSignaler.DisposeEvent ();

		return pTask==&m_dSignalerTask;
	}

	inline void fire_event ()
	{
		m_dSignaler.FireEvent ();
	}

	// epoll/kqueue doesn't perform read/write itself, so transferred size is always 0.
	inline DWORD ready_transferred ()
	{
		return 0;
	}

#if POLLING_EPOLL
	CSphVector<epoll_event> m_dReady;
	epoll_event	* m_pEntry = nullptr;

	inline void poll_create_impl ( int iSizeHint )
	{
		m_iEFD = epoll_create ( iSizeHint ); // 1000 is dummy, see man
		if ( m_iEFD==-1 )
			sphDie ( "failed to create epoll main FD, errno=%d, %s", errno, strerrorm ( errno ) );

		sphLogDebugv ( "epoll %d created", m_iEFD );
		m_dReady.Reserve ( iSizeHint );
	}

	inline void poll_destroy ()
	{
		sphLogDebugv ( "epoll %d closed", m_iEFD );
		SafeClose ( m_iEFD );
	}

	// each action added one-by-one...
	int add_change_task ( Task_t * pTask )
	{
		auto iEvents = 0;
		bool bApply = pTask->m_ifd!=-1;
		bool bWrite = pTask->m_uIOChanged==1;
		bool bRead = ( pTask->m_uIOChanged!=0 ) || ( pTask==&m_dSignalerTask );
		bool bWasWrite = pTask->m_uIOActive==1;
		bool bWasRead = ( pTask->m_uIOActive!=0 );


		int iOp = 0;
		epoll_event tEv = { 0 };
		tEv.data.ptr = pTask;

		if ( !bRead && !bWrite )
		{
			iOp = EPOLL_CTL_DEL;
			--iEvents;
			sphLogDebugL ( "L EPOLL_CTL_DEL(%d), %d+%d events", pTask->m_ifd, m_iEvents, iEvents );
		} else
		{
			tEv.events = ( bRead ? EPOLLIN : 0 ) | ( bWrite ? EPOLLOUT : 0 );
			if ( !bWasWrite && !bWasRead )
			{
				iOp = EPOLL_CTL_ADD;
				++iEvents;
				sphLogDebugL ( "L EPOLL_CTL_ADD(%d) -> %d, %d+%d events", pTask->m_ifd, tEv.events, m_iEvents, iEvents );
			} else
			{
				iOp = EPOLL_CTL_MOD;
				sphLogDebugL ( "L EPOLL_CTL_MOD(%d) -> %d, %d+%d events", pTask->m_ifd, tEv.events, m_iEvents, iEvents );
			}
		}

		if ( bApply )
		{
			auto iRes = epoll_ctl ( m_iEFD, iOp, pTask->m_ifd, &tEv );
			if ( iRes==-1 )
				sphLogDebugL ( "L failed to perform epollctl for sock %d(%p), errno=%d, %s"
					  , pTask->m_ifd, pTask, errno, strerrorm ( errno ) );
		} else
			sphLogDebugL ( "L epoll_ctl not called since sock is closed" );
		return iEvents;
	}

	inline void poll_correct_events ( BYTE uIOmode )
	{
		if ( uIOmode )
			--m_iEvents;
	}

	inline int poller_wait ( int64_t iTimeoutUS )
	{
		m_dReady.Resize ( m_iEvents );
		int iTimeoutMS = iTimeoutUS<0 ? -1 : ( ( iTimeoutUS + 500 ) / 1000 );
		return epoll_wait ( m_iEFD, m_dReady.Begin (), m_dReady.GetLength (), iTimeoutMS );
	};

	inline Task_t * ready_iterate ( int iReady )
	{
		if ( iReady>=0 )
			m_pEntry = &m_dReady[iReady];
		assert ( m_pEntry );
		return ( Task_t *) m_pEntry->data.ptr;
	}

	inline int ready_events ()
	{
		assert ( m_pEntry );
		return m_pEntry->events;
	}

	inline bool ready_error ()
	{
		assert ( m_pEntry );
		return ( m_pEntry->events & EPOLLERR )!=0;
	}

	inline bool ready_eof ()
	{
		assert ( m_pEntry );
		return ( m_pEntry->events & EPOLLHUP )!=0;
	}

	inline bool ready_read ()
	{
		assert ( m_pEntry );
		return ( m_pEntry->events & EPOLLIN )!=0;
	}

	inline bool ready_write ()
	{
		assert ( m_pEntry );
		return ( m_pEntry->events & EPOLLOUT )!=0;
	}

#elif POLLING_KQUEUE
	CSphVector<struct kevent> m_dReady;
	CSphVector<struct kevent> m_dScheduled; // prepared group of events
	struct kevent * m_pEntry = nullptr;

	inline void poll_create_impl ( int iSizeHint )
	{
		m_iEFD = kqueue ();
		if ( m_iEFD==-1 )
			sphDie ( "failed to create kqueue main FD, errno=%d, %s", errno, strerrorm ( errno ) );

		sphLogDebugv ( "kqueue %d created", m_iEFD );
		m_dReady.Reserve ( iSizeHint );
		m_dScheduled.Reserve ( iSizeHint * 2 );
	}

	inline void poll_destroy ()
	{
		sphLogDebugv ( "kqueue %d closed", m_iEFD );
		SafeClose ( m_iEFD );
	}


	inline int poll_one_action ( CSphVector<struct kevent> &dSchedule, Task_t * pTask  )
	{

		int iEvents = 0;
		bool bWrite = pTask->m_uIOChanged==1;
		bool bRead = ( pTask->m_uIOChanged!=0 ) || ( pTask==&m_dSignalerTask );
		bool bWasWrite = pTask->m_uIOActive==1;
		bool bWasRead = ( pTask->m_uIOActive!=0 );
		bool bApply = pTask->m_ifd!=-1;

		// boring combination matrix below
		if ( bRead && !bWasRead )
		{
			if ( bApply )
				EV_SET ( &dSchedule.Add (), pTask->m_ifd, EVFILT_READ, EV_ADD, 0, 0, pTask );
			++iEvents;
			sphLogDebugL ( "L EVFILT_READ, EV_ADD, %d (%d enqueued), %d in call", pTask->m_ifd, dSchedule.GetLength ()
						   , iEvents );
		}

		if ( bWrite && !bWasWrite )
		{
			if ( bApply )
				EV_SET ( &dSchedule.Add (), pTask->m_ifd, EVFILT_WRITE, EV_ADD, 0, 0, pTask );
			++iEvents;
			sphLogDebugL ( "L EVFILT_WRITE, EV_ADD, %d (%d enqueued), %d in call", pTask->m_ifd, dSchedule.GetLength ()
						   , iEvents );
		}

		if ( !bRead && bWasRead )
		{
			if ( bApply )
				EV_SET ( &dSchedule.Add (), pTask->m_ifd, EVFILT_READ, EV_DELETE, 0, 0, pTask );
			--iEvents;
			sphLogDebugL ( "L EVFILT_READ, EV_DELETE, %d (%d enqueued), %d in call", pTask->m_ifd, dSchedule.GetLength ()
						   , iEvents );
		}

		if ( !bWrite && bWasWrite )
		{
			if ( bApply )
				EV_SET ( &dSchedule.Add (), pTask->m_ifd, EVFILT_WRITE, EV_DELETE, 0, 0, pTask );
			--iEvents;
			sphLogDebugL ( "L EVFILT_WRITE, EV_DELETE, %d (%d enqueued), %d in call", pTask->m_ifd, dSchedule.GetLength ()
						   , iEvents );
		}
		return iEvents;
	}

	inline void poll_correct_events ( BYTE uIOmode )
	{
		if ( uIOmode )
			--m_iEvents;
		if ( uIOmode==1 )
			--m_iEvents;
	}

	int add_change_task ( Task_t * pTask )
	{
		return poll_one_action ( m_dScheduled, pTask );
	}

	inline int poller_wait ( int64_t iTimeoutUS )
	{
		m_dReady.Resize ( m_iEvents + m_dScheduled.GetLength () );
		timespec ts;
		timespec * pts = nullptr;
		if ( iTimeoutUS>=0 )
		{
			ts.tv_sec = iTimeoutUS / 1000000;
			ts.tv_nsec = ( long ) ( iTimeoutUS - ts.tv_sec * 1000000 ) * 1000;
			pts = &ts;
		}
		// need positive timeout for communicate threads back and shutdown
		auto iRes = kevent ( m_iEFD, m_dScheduled.begin (), m_dScheduled.GetLength (), m_dReady.begin ()
								, m_dReady.GetLength (), pts );
		m_dScheduled.Reset();
		return iRes;

	};

	inline Task_t * ready_iterate ( int iReady )
	{
		if ( iReady>=0 )
			m_pEntry = &m_dReady[iReady];
		assert ( m_pEntry );
		return ( Task_t * ) m_pEntry->udata;
	}

	inline int ready_events ()
	{
		assert ( m_pEntry );
		return m_pEntry->filter;
	}

	inline bool ready_error ()
	{
		assert ( m_pEntry );
		if ( ( m_pEntry->flags & EV_ERROR ) == 0 )
			return false;
		
		sphLogDebugL ( "L error for %lu, errno=%lu, %s", m_pEntry->ident, m_pEntry->data, sphSockError ( m_pEntry->data ) );
		return true;
	}

	inline bool ready_eof ()
	{
		assert ( m_pEntry );
		return ( m_pEntry->flags | EV_EOF )!=0;
	}

	inline bool ready_read ()
	{
		assert ( m_pEntry );
		return ( m_pEntry->filter == EVFILT_READ )!=0;
	}

	inline bool ready_write ()
	{
		assert ( m_pEntry );
		return ( m_pEntry->filter == EVFILT_WRITE )!=0;
	}
#endif
#endif
};

/// Like ISphNetEvents, but most syscalls optimized out
class LazyNetEvents_c : ISphNoncopyable, protected NetEventsFlavour_c
{
	using VectorTask_c = CSphVector<Task_t*>;

	// stuff to transfer (enqueue) tasks
	VectorTask_c *	m_pEnqueuedTasks GUARDED_BY (m_dActiveLock) = nullptr; // ext. mt queue where we add tasks
	VectorTask_c	m_dInternalTasks; // internal queue where we add our tasks without mutex
	CSphMutex	m_dActiveLock;
	TimeoutQueue_c m_dTimeouts;
	SphThread_t m_dWorkingThread;
	int			m_iLastReportedErrno = -1;

private:
	/// maps AgentConn_t -> Task_c for new/existing task
	inline Task_t * TaskFor ( AgentConn_t * pConnection )
	{
		auto pTask = ( Task_t * ) pConnection->m_pPollerTask;
		if ( !pTask )
		{
			pTask = new Task_t;
			pTask->m_ifd = pConnection->m_iSock;
			pTask->m_pPayload = pConnection;
			pConnection->m_pPollerTask = pTask;
			pConnection->AddRef ();
			sphLogDebugv ( "- TaskFor(%p)->%p, new, ref=%d", pConnection, pTask, (int) pConnection->GetRefcount () );
		} else
			sphLogDebugv ( "- TaskFor(%p)->%p, exst, ref=%d", pConnection, pTask, ( int ) pConnection->GetRefcount () );
		return pTask;
	}

	// Simply deletes the task, but some tricks exist:
	// 1. (general): keeping payload nesessary when we fire timeout: the task is not necessary anyway,
	// however timeout callback need to be called with still valid (if any) payload.
	// 2. (win specific): On windows, however, another trick is in game: timeout condition we get from
	// internal GetQueuedCompletionStatusEx function. At the same time overlapped ops (WSAsend or recv, 
	// or even both) are still in work, and so we need to keep the 'overlapped' structs alive for them.
	// So, we can't just delete the task in the case. Instead we invalidate it (all, excluding overlapped),
	// so that the next return from poller_wait will recognize it and finally totally destroy the task for us.
	AgentConn_t * DeleteTask ( Task_t * pTask, bool bReleasePayload=true )
	{
		assert ( pTask );
		auto pConnection = pTask->m_pPayload;

		sphLogDebugL ( "L DeleteTask for %p, (conn %p), release=%d", pTask, pTask->m_pPayload, bReleasePayload );
		// if payload already invoked in another task (remember, we process deffered action!)
		// we won't nullify it.
		if ( pConnection && pConnection->m_pPollerTask==pTask )
			pConnection->m_pPollerTask = nullptr;

		if ( bReleasePayload )
		{
			SafeRelease ( pConnection );
#if USE_WINDOWS
			delete pTask;
		} else
		{
			pTask->m_ifd = -1;
			pTask->m_pPayload = nullptr;
			if (pTask->m_iRefs<=0)
#endif
				delete pTask;
		}
		return pConnection;
	}

	/// Atomically move m-t queue to single-thread internal queue.
	VectorTask_c * PopQueue () REQUIRES ( LazyThread ) EXCLUDES ( m_dActiveLock )
	{
		// atomically get current vec; put zero instead.
		VectorTask_c * pReadyQueue = nullptr;
		{
			ScopedMutex_t tLock ( m_dActiveLock );
			pReadyQueue = m_pEnqueuedTasks;
			m_pEnqueuedTasks = nullptr;
		}
		return pReadyQueue;
	}


	void ProcessChanges ( Task_t * pTask )
	{
		sphLogDebugL ( "L PerformTask for %p, (conn %p)", pTask, pTask->m_pPayload );

		if ( pTask->m_uIOChanged )
			poll_batch_action ( pTask );

		// positive timeout - task is about add/change timeout
		if ( pTask->m_iTimeoutTime>0 )
		{
			sphLogDebugL ( "L change/add timeout for %p, " INT64_FMT " (%d) is changed one", pTask
						   , pTask->m_iTimeoutTime, ( int ) ( pTask->m_iTimeoutTime - sphMicroTimer () ) );
			m_dTimeouts.Change ( pTask );
			sphLogDebugL ( "%s", m_dTimeouts.DebugDump ( "L " ).cstr () );
		}
		// -1 timeout - task is about delete everything
		else if ( pTask->m_iTimeoutTime==-1 ) // process 'finish' socket
		{
			sphLogDebugL ( "L remove timeout for %p", pTask );
			m_dTimeouts.Remove ( pTask );
			// real unclosed socket provided, need to unsubscribe it
			pTask->m_uIOChanged = 0;
			poll_batch_action ( pTask );
			DeleteTask ( pTask );
			sphLogDebugL ( "%s", m_dTimeouts.DebugDump ( "L " ).cstr () );
		}
	}

	/// take current internal and external queues, parse it and enqueue changes.
	void ProcessEnqueuedTasks () REQUIRES ( LazyThread )
	{
		sphLogDebugL ( "L ProcessEnqueuedTasks" );

		if ( !m_dInternalTasks.IsEmpty () )
		{
			sphLogDebugL ( "L starting processing %d internal events", m_dInternalTasks.GetLength () );
			Task_t * pLastTask = nullptr;
			for ( auto * pTask : m_dInternalTasks )
			{
				if ( pTask==pLastTask )
					continue;
				sphLogDebugL ( "L Internal event" );
				ProcessChanges ( pTask );
				pLastTask = pTask;
			}
			sphLogDebugL ( "L Internal events processed" );
			m_dInternalTasks.Reset();
		}

		auto pExternalQueue = PopQueue ();
		if ( !pExternalQueue )
			return;

		if ( !pExternalQueue->IsEmpty () )
		{
			sphLogDebugL ( "L starting adding %d events", pExternalQueue->GetLength () );
			Task_t * pLastTask = nullptr;
			for ( auto * pTask : *pExternalQueue )
			{
				if ( pTask==pLastTask )
					continue;
				sphLogDebugL ( "L Event" );
				ProcessChanges ( pTask );
				pLastTask = pTask;
			}
			sphLogDebugL ( "L Events processed" );
		}

		delete pExternalQueue;
	}

	/// main event loop run in separate thread.
	void EventLoop () REQUIRES ( LazyThread )
	{
		while ( true )
			if ( !EventTick () )
				break;
	}

	/// abandon and release all tiemouted events.
	/// \return next active timeout (in uS), or -1 for infinite.
	int64_t GetNextTimeout()
	{
		while ( !m_dTimeouts.IsEmpty () )
		{
			auto pTask = m_dTimeouts.Root ();
			assert ( pTask->m_iTimeoutTime>0 );

			int64_t iNextTimeout = pTask->m_iTimeoutTime - sphMicroTimer ();
			if ( iNextTimeout>0 )
				return iNextTimeout;

			sphLogDebugL ( "L timeout happens for %p task", pTask );
			m_dTimeouts.Pop ();

			// adopt timeout action, keep connection
			Clbck_f m_fTimeoutAction = std::move ( pTask->m_fTimeoutAction );
			BYTE uSubscribes = pTask->m_uIOActive;

			CSphRefcountedPtr<AgentConn_t> pKeepConn ( DeleteTask ( pTask, false ) );

			sphLogDebugL ( "%s", m_dTimeouts.DebugDump ( "L heap:" ).cstr () );
			if ( m_fTimeoutAction )
			{
				sphLogDebugL ( "L timeout action started" );
				m_fTimeoutAction ();
				sphLogDebugL ( "L timeout action finished" );
			}
			poll_correct_events ( uSubscribes );
		}
		return -1; /// means 'infinite'
	}

	/// one event cycle.
	/// \return false to stop event loop and exit.
	bool EventTick () REQUIRES ( LazyThread )
	{
		sphLogDebugL ( "L EventTick()" );
		ProcessEnqueuedTasks ();

		auto iTimeToWailtUS = GetNextTimeout ();
		sphLogDebugL ( "L calculated timeout is " INT64_FMT " useconds", iTimeToWailtUS );

		auto iStarted = sphMicroTimer ();
		auto iEvents = poller_wait ( iTimeToWailtUS );
		auto iWaited = sphMicroTimer() - iStarted;

		if ( g_bShutdown )
		{
			sphInfo ( "EventTick() exit because of shutdown=%d", g_bShutdown );
			return false;
		}

		if ( iEvents<0 )
		{
			int iErrno = sphSockGetErrno ();
			if ( m_iLastReportedErrno!=iErrno )
			{
				sphLogDebugL ( "L poller tick failed: %s", sphSockError ( iErrno ) );
				m_iLastReportedErrno = iErrno;
			}
			sphLogDebugL ( "L poller tick failed: %s", sphSockError ( iErrno ) );
			return true;
		}
		sphLogDebugL ( "L poller wait returned %d events from %d", iEvents, m_iEvents );

		/// we have some events to speak about...
		for ( int i = 0; i<iEvents; ++i )
		{
			Task_t * pTask = ready_iterate ( i );
			if ( ready_signaler ( pTask ) )
			{
				sphLogDebugL ( "L internal event. Disposed" );
				continue;
			}

			if ( !pTask )
				continue;
			else
				sphLogDebugL ( "L event action for %p(%d), %d", pTask, pTask->m_ifd, ready_events () );

			bool bError = ready_error ();
			bool bEof = ready_eof ();
			if ( bError )
			{
				sphLogDebugL ( "L error happened" );
				if ( bEof )
				{
					sphLogDebugL ( "L assume that is eof, discard the error" );
					bError = false;
				}
			}

			if ( pTask->m_uIOActive )
			{
				if ( bError )
				{
					sphLogDebugL ( "L error action %p, waited " INT64_FMT, pTask, iWaited );
					pTask->m_pPayload->ErrorCallback ( iWaited );
					sphLogDebugL ( "L error action %p completed", pTask );
				} else
				{
					if ( ready_write () )
					{
						if ( !bEof )
						{
							sphLogDebugL ( "L write action %p, waited " INT64_FMT ", transferred %d", pTask, iWaited, ready_transferred() );
							pTask->m_pPayload->SendCallback ( iWaited, ready_transferred () );
							sphLogDebugL ( "L write action %p completed", pTask );
						} else
							sphLogDebugL ( "L write action avoid because of eof", pTask );
					}

					if ( ready_read() )
					{
						sphLogDebugL ( "L read action %p, waited " INT64_FMT ", transferred %d", pTask, iWaited, ready_transferred () );
						pTask->m_pPayload->RecvCallback ( iWaited, ready_transferred () );
						sphLogDebugL ( "L read action %p completed", pTask );
					}
				}
			}
		}
		return true;
	}


public:
	explicit LazyNetEvents_c ( int iSizeHint )
	{
		poll_create ( iSizeHint );
		SphCrashLogger_c::ThreadCreate ( &m_dWorkingThread, WorkerFunc, this );
	}

	~LazyNetEvents_c ()
	{
		assert ( g_bShutdown );
		sphLogDebug ( "~LazyNetEvents_c. Shutdown=%d", g_bShutdown );
		Fire();
		sphThreadJoin ( &m_dWorkingThread );
		poll_destroy();
	}

	void AddToQueue ( Task_t * pTask, bool bInternal )
	{
		if ( bInternal )
		{
			sphLogDebugL ( "L AddToQueue, int=%d", m_dInternalTasks.GetLength () );
			m_dInternalTasks.Add ( pTask );
		} else
		{
			sphLogDebugL ( "- AddToQueue, ext=%d", m_pEnqueuedTasks ? m_pEnqueuedTasks->GetLength () : 0 );
			ScopedMutex_t tLock ( m_dActiveLock );
			if ( !m_pEnqueuedTasks )
				m_pEnqueuedTasks = new VectorTask_c;
			m_pEnqueuedTasks->Add ( pTask );
		}
	}

	/// Enqueue or perform a timer functor
	/// caller guarantee that interhal task exists
	void EnqueueTask ( AgentConn_t * pConnection, int64_t iTimeoutMS, Clbck_f &&fTimeout, BYTE uActivateIO )
	{
		Task_t * pTask = TaskFor ( pConnection );
		assert ( pTask );

		// check for same timeout as we have. Avoid dupes, if so.
		if ( pTask->m_iTimeoutTime==iTimeoutMS )
			return;

		sphLogDebugv ( "- %d EnqueueTask invoked with pconn=%p, ts=" INT64_FMT ", ActivateIO=%d"
					   , pConnection->m_iStoreTag, pConnection, iTimeoutMS, uActivateIO );

		pTask->m_iTimeoutTime = iTimeoutMS;
		if ( fTimeout )
			pTask->m_fTimeoutAction = std::move ( fTimeout );

		// case of finish: we need to unsubscribe IO, and so need actual socket for it.
		if ( iTimeoutMS==-1 )
			pTask->m_ifd = pConnection->m_iSock;

		if ( uActivateIO != pTask->m_uIOActive )
		{
			pTask->m_uIOChanged = uActivateIO;
#if USE_WINDOWS
			// for win it is vitable important to apply changes immediately,
			// since iocp has to be enqueued before read/write op, not after!
			if ( pTask->m_uIOChanged )
				poll_batch_action ( pTask );
#endif
		}

		sphLogDebugv ( "- %d EnqueueTask enqueueing (task %p) " INT64_FMT "Us, ChangeIO=%d", pConnection->m_iStoreTag, pTask, pTask->m_iTimeoutTime, pTask->m_uIOChanged );
		AddToQueue ( pTask, pConnection->InNetLoop () );
	}

	void DisableWrite ( AgentConn_t * pConnection )
	{
		Task_t * pTask = TaskFor ( pConnection );
		assert ( pTask );

		if ( 2!=pTask->m_uIOActive )
		{
			pTask->m_uIOChanged = 2;
			AddToQueue ( pTask, pConnection->InNetLoop () );
		}
	}

	/// then signal the poller.
	void Fire ()
	{
		sphLogDebugL ("L Fire an event invoked");
		fire_event ();
	}

private:
	static void WorkerFunc ( void * pArg ) REQUIRES ( !LazyThread )
	{
		ScopedRole_c thLazy ( LazyThread );
		auto * pThis = ( LazyNetEvents_c * ) pArg;
		sphLogDebugL ( "L RemoteAgentsPoller_t::WorkerFunc started" );
		pThis->EventLoop ();
	}
};

//! Get static (singletone) instance of lazy poller
//! C++11 guarantees it to be mt-safe (magic-static).
LazyNetEvents_c & LazyPoller ()
{
	static LazyNetEvents_c dEvents ( 1000 );
	return dEvents;
}

//! Add or change timeout task
void AgentConn_t::TimeoutTask ( int64_t iTimeoutMS, Clbck_f &&fTimeout, BYTE uActivateIO )
{
	// skip del/change for not scheduled conns
	if ( !m_pPollerTask && !fTimeout )
		return;

	if ( fTimeout )
		m_bNeedKick = !m_bInNetLoop;
	LazyPoller ().EnqueueTask ( this, iTimeoutMS, std::forward<Clbck_f> ( fTimeout ), uActivateIO );
}

void AgentConn_t::DisableWrite ()
{
	// skip del/change for not scheduled conns
	if ( !m_pPollerTask )
		return;

	LazyPoller ().DisableWrite ( this );
}

void FirePoller ()
{
	LazyPoller ().Fire ();
}


class CRemoteAgentsObserver : public IRemoteAgentsObserver
{
public:
	void Report ( bool bSuccess ) final
	{
		if ( bSuccess )
			++m_iSucceeded;
		++m_iFinished;
		m_tChanged.SetEvent ();
	}

	void SetTotal ( int iTasks ) final
	{
		m_iTasks = iTasks;
	}

	// check that there are no works to do
	bool IsDone () const final
	{
		return m_iFinished==m_iTasks;
	}

	// block execution untill all tasks are finished
	void Finish () final
	{
		while (!IsDone())
			WaitChanges ();
	}

	// block execution while some works finished
	void WaitChanges () final
	{
		m_tChanged.WaitEvent ();
	}

	inline long GetSucceeded() const final
	{
		return m_iSucceeded;
	}

	inline long GetFinished () const final
	{
		return m_iFinished;
	}

private:
	CSphAutoEvent	m_tChanged;			///< the signaller
	CSphAtomic		m_iSucceeded { 0 };	//< num of tasks finished successfully
	CSphAtomic		m_iFinished { 0 };	//< num of tasks finished.
	int				m_iTasks = 0;		//< total num of tasks

};

IRemoteAgentsObserver * GetObserver ()
{
	return new CRemoteAgentsObserver;
}

/// check if a non-blocked socket is still connected
bool sphNBSockEof ( int iSock )
{
	if ( iSock<0 )
		return true;

	char cBuf;
	// since socket is non-blocked, ::recv will not block anyway
	int iRes = ::recv ( iSock, &cBuf, sizeof ( cBuf ), MSG_PEEK );
	if ( (!iRes) || (iRes<0
		&& sphSockGetErrno ()!=EWOULDBLOCK
		&& sphSockGetErrno ()!=EAGAIN ))
		return true;
	return false;
}

// in case of epoll/kqueue the full set of polled sockets are stored
// in a cache inside kernel, so once added, we can't iterate over all of the items.
// So, we store them in linked list for that purpose.


// store and iterate over the list of items
class IterableEvents_c : public ISphNetEvents
{
protected:
	List_t m_tWork;
	NetEventsIterator_t m_tIter;
	ListedData_t * m_pIter = nullptr;

protected:
	ListedData_t * AddNewEventData ( const void * pData )
	{
		assert ( pData );
		auto * pIntData = new ListedData_t ( pData );
		m_tWork.Add ( pIntData );
		return pIntData;
	}

	void ResetIterator ()
	{
		m_tIter.Reset();
		m_pIter = nullptr;
	}

	void RemoveCurrentItem ()
	{
		assert ( m_pIter );
		assert ( m_pIter->m_pData==m_tIter.m_pData );
		assert ( m_tIter.m_pData );

		auto * pPrev = (ListedData_t *)m_pIter->m_pPrev;
		m_tWork.Remove ( m_pIter );
		SafeDelete( m_pIter );
		m_pIter = pPrev;
		m_tIter.m_pData = m_pIter->m_pData;
	}

public:
	IterableEvents_c () = default;

	~IterableEvents_c () override
	{
		while ( m_tWork.GetLength() )
		{
			auto * pIter = (ListedData_t *)m_tWork.Begin();
			m_tWork.Remove ( pIter );
			SafeDelete( pIter );
		}
		ResetIterator();
	}

	bool IterateNextAll () override
	{
		if ( !m_pIter )
		{
			if ( m_tWork.Begin()==m_tWork.End() )
				return false;

			m_pIter = (ListedData_t *)m_tWork.Begin();
			m_tIter.m_pData = m_pIter->m_pData;
			return true;
		} else
		{
			m_pIter = (ListedData_t *)m_pIter->m_pNext;
			m_tIter.m_pData = m_pIter->m_pData;
			if ( m_pIter!=m_tWork.End() )
				return true;

			ResetIterator();
			return false;
		}
	}

	NetEventsIterator_t & IterateGet() override
	{
		return m_tIter;
	}
};


#if POLLING_EPOLL
class EpollEvents_c : public IterableEvents_c
{
private:
	CSphVector<epoll_event>		m_dReady;
	int							m_iLastReportedErrno;
	int							m_iReady;
	int							m_iEFD;
	int							m_iIterEv;

public:
	explicit EpollEvents_c ( int iSizeHint )
		: m_iLastReportedErrno ( -1 )
		, m_iReady ( 0 )
	{
		m_iEFD = epoll_create ( iSizeHint ); // 1000 is dummy, see man
		if ( m_iEFD==-1 )
			sphDie ( "failed to create epoll main FD, errno=%d, %s", errno, strerrorm ( errno ) );

		sphLogDebugv ( "epoll %d created", m_iEFD );
		m_dReady.Reserve ( iSizeHint );
		m_iIterEv = -1;
	}

	~EpollEvents_c ()
	{
		sphLogDebugv ( "epoll %d closed", m_iEFD );
		SafeClose ( m_iEFD );
	}

	void SetupEvent ( int iSocket, PoolEvents_e eFlags, const void * pData ) override
	{
		assert ( pData && iSocket>=0 );
		assert ( eFlags==SPH_POLL_WR || eFlags==SPH_POLL_RD );

		ListedData_t * pIntData = AddNewEventData ( pData );

		epoll_event tEv;
		tEv.data.ptr = pIntData;
		tEv.events = (eFlags==SPH_POLL_RD ? EPOLLIN : EPOLLOUT);

		sphLogDebugv ( "%p epoll %d setup, ev=0x%u, sock=%d", pData, m_iEFD, tEv.events, iSocket );

		int iRes = epoll_ctl ( m_iEFD, EPOLL_CTL_ADD, iSocket, &tEv );
		if ( iRes==-1 )
			sphWarning ( "failed to setup epoll event for sock %d, errno=%d, %s", iSocket, errno, strerrorm ( errno ) );
	}

	bool Wait ( int timeoutMs ) override
	{
		m_dReady.Resize ( m_tWork.GetLength () );
		// need positive timeout for communicate threads back and shutdown
		m_iReady = epoll_wait ( m_iEFD, m_dReady.Begin (), m_dReady.GetLength (), timeoutMs );

		sphLogDebugv ( "%d epoll wait returned %d events (timeout %d)", m_iEFD, m_iReady, timeoutMs );

		if ( m_iReady<0 )
		{
			int iErrno = sphSockGetErrno ();
			// common recoverable errors
			if ( iErrno==EINTR || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
				return false;

			if ( m_iLastReportedErrno!=iErrno )
			{
				sphWarning ( "epoll tick failed: %s", sphSockError ( iErrno ) );
				m_iLastReportedErrno = iErrno;
			}
			return false;
		}

		return ( m_iReady>0 );
	}

	int IterateStart () override
	{
		ResetIterator();
		m_iIterEv = -1;
		return m_iReady;
	}

	bool IterateNextReady () override
	{
		ResetIterator();
		++m_iIterEv;
		if ( m_iReady<=0 || m_iIterEv>=m_iReady )
			return false;

		const epoll_event & tEv = m_dReady[m_iIterEv];

		m_pIter = (ListedData_t *)tEv.data.ptr;
		m_tIter.m_pData = m_pIter->m_pData;

		if ( tEv.events & EPOLLIN )
			m_tIter.m_uEvents |= SPH_POLL_RD;
		if ( tEv.events & EPOLLOUT )
			m_tIter.m_uEvents |= SPH_POLL_WR;
		if ( tEv.events & EPOLLHUP )
			m_tIter.m_uEvents |= SPH_POLL_HUP;
		if ( tEv.events & EPOLLERR )
			m_tIter.m_uEvents |= SPH_POLL_ERR;
		if ( tEv.events & EPOLLPRI )
			m_tIter.m_uEvents |= SPH_POLL_PRI;

		return true;
	}

	void IterateChangeEvent ( int iSocket, PoolEvents_e eFlags ) override
	{
		epoll_event tEv;
		tEv.data.ptr = (void *)m_pIter;
		tEv.events = (eFlags==SPH_POLL_RD ? EPOLLIN : EPOLLOUT); ;

		sphLogDebugv ( "%p epoll change, ev=0x%u, sock=%d", m_tIter.m_pData, tEv.events, iSocket );

		int iRes = epoll_ctl ( m_iEFD, EPOLL_CTL_MOD, iSocket, &tEv );
		if ( iRes==-1 )
			sphWarning ( "failed to modify epoll event for sock %d, errno=%d, %s", iSocket, errno, strerrorm ( errno ) );
	}

	void IterateRemove ( int iSocket ) override
	{
		assert ( m_pIter->m_pData==m_tIter.m_pData );

		sphLogDebugv ( "%p epoll remove, ev=0x%u, sock=%d", m_tIter.m_pData, m_tIter.m_uEvents, iSocket );
		assert ( m_tIter.m_pData );

		epoll_event tEv;
		int iRes = epoll_ctl ( m_iEFD, EPOLL_CTL_DEL, iSocket, &tEv );

		// might be already closed by worker from thread pool
		if ( iRes==-1 )
			sphLogDebugv ( "failed to remove epoll event for sock %d(%p), errno=%d, %s", iSocket, m_tIter.m_pData, errno, strerrorm ( errno ) );

		RemoveCurrentItem();
	}
};


ISphNetEvents * sphCreatePoll ( int iSizeHint, bool )
{
	return new EpollEvents_c ( iSizeHint );
}

#endif
#if POLLING_KQUEUE

class KqueueEvents_c : public IterableEvents_c
{

private:
	CSphVector<struct kevent>			m_dReady;
	int							m_iLastReportedErrno;
	int							m_iReady;
	int							m_iKQ;
	int							m_iIterEv;

public:
	explicit KqueueEvents_c ( int iSizeHint )
		: m_iLastReportedErrno ( -1 )
		, m_iReady ( 0 )
	{
		m_iKQ = kqueue ();
		if ( m_iKQ==-1 )
			sphDie ( "failed to create kqueue main FD, errno=%d, %s", errno, strerrorm ( errno ) );

		sphLogDebugv ( "kqueue %d created", m_iKQ );
		m_dReady.Reserve ( iSizeHint );
		m_iIterEv = -1;
	}

	~KqueueEvents_c ()
	{
		sphLogDebugv ( "kqueue %d closed", m_iKQ );
		SafeClose ( m_iKQ );
	}

	void SetupEvent ( int iSocket, PoolEvents_e eFlags, const void * pData ) override
	{
		assert ( pData && iSocket>=0 );
		assert ( eFlags==SPH_POLL_WR || eFlags==SPH_POLL_RD );

		ListedData_t * pIntData = AddNewEventData ( pData );

		struct kevent tEv;
		EV_SET ( &tEv, iSocket, (eFlags==SPH_POLL_RD ? EVFILT_READ : EVFILT_WRITE), EV_ADD, 0, 0, pIntData );

		sphLogDebugv ( "%p kqueue %d setup, ev=%d, sock=%d", pData, m_iKQ, tEv.filter, iSocket );

		int iRes = kevent (m_iKQ, &tEv, 1, nullptr, 0, nullptr);
		if ( iRes==-1 )
			sphWarning ( "failed to setup kqueue event for sock %d, errno=%d, %s", iSocket, errno, strerrorm ( errno ) );
	}

	bool Wait ( int timeoutMs ) override
	{
		m_dReady.Resize ( m_tWork.GetLength () );

		timespec ts;
		timespec *pts = nullptr;
		if ( timeoutMs>=0 )
		{
			ts.tv_sec = timeoutMs/1000;
			ts.tv_nsec = (long)(timeoutMs-ts.tv_sec*1000)*1000000;
			pts = &ts;
		}
		// need positive timeout for communicate threads back and shutdown
		m_iReady = kevent (m_iKQ, nullptr, 0, m_dReady.begin(), m_dReady.GetLength(), pts);

		if ( timeoutMs>1 ) // avoid flood of log on very short waits
			sphLogDebugv ( "%d kqueue wait returned %d events (timeout %d)", m_iKQ, m_iReady, timeoutMs );

		if ( m_iReady<0 )
		{
			int iErrno = sphSockGetErrno ();
			// common recoverable errors
			if ( iErrno==EINTR || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
				return false;

			if ( m_iLastReportedErrno!=iErrno )
			{
				sphWarning ( "kqueue tick failed: %s", sphSockError ( iErrno ) );
				m_iLastReportedErrno = iErrno;
			}
			return false;
		}

		return ( m_iReady>0 );
	}

	int IterateStart () override
	{
		ResetIterator();
		m_iIterEv = -1;
		return m_iReady;
	}

	bool IterateNextReady () override
	{
		ResetIterator();
		++m_iIterEv;
		if ( m_iReady<=0 || m_iIterEv>=m_iReady )
			return false;

		const struct kevent & tEv = m_dReady[m_iIterEv];

		m_pIter = (ListedData_t *) tEv.udata;
		m_tIter.m_pData = m_pIter->m_pData;

		if ( tEv.filter == EVFILT_READ )
			m_tIter.m_uEvents = SPH_POLL_RD;

		if ( tEv.filter == EVFILT_WRITE )
			m_tIter.m_uEvents = SPH_POLL_WR;

		sphLogDebugv ( "%p kqueue iterate ready, ev=%d", m_tIter.m_pData, tEv.filter );

		return true;
	}

	void IterateChangeEvent ( int iSocket, PoolEvents_e eFlags ) override
	{
		assert ( eFlags==SPH_POLL_WR || eFlags==SPH_POLL_RD );



		struct kevent tEv;
		EV_SET(&tEv, iSocket, (eFlags==SPH_POLL_RD ? EVFILT_READ : EVFILT_WRITE), EV_ADD, 0, 0, (void*) m_pIter);

		sphLogDebugv ( "%p kqueue change, ev=%d, sock=%d", m_tIter.m_pData, tEv.filter, iSocket );


		int iRes = kevent (m_iKQ, &tEv, 1, nullptr, 0, nullptr);

		EV_SET( &tEv, iSocket, ( eFlags==SPH_POLL_RD ? EVFILT_WRITE : EVFILT_READ ), EV_DELETE | EV_CLEAR, 0, 0, ( void * ) m_pIter );
		kevent ( m_iKQ, &tEv, 1, nullptr, 0, nullptr );

		if ( iRes==-1 )
			sphWarning ( "failed to setup kqueue event for sock %d, errno=%d, %s", iSocket, errno, strerrorm ( errno ) );
	}

	void IterateRemove ( int iSocket ) override
	{
		assert ( m_pIter->m_pData==m_tIter.m_pData );

		sphLogDebugv ( "%p kqueue remove, uEv=0x%u, sock=%d", m_tIter.m_pData, m_tIter.m_uEvents, iSocket );
		assert ( m_tIter.m_pData );

		struct kevent tEv;
		EV_SET(&tEv, iSocket, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
		kevent (m_iKQ, &tEv, 1, nullptr, 0, nullptr);
		EV_SET( &tEv, iSocket, EVFILT_READ, EV_DELETE, 0, 0, nullptr );
		int iRes = kevent ( m_iKQ, &tEv, 1, nullptr, 0, nullptr );

		// might be already closed by worker from thread pool
		if ( iRes==-1 )
			sphLogDebugv ( "failed to remove kqueue event for sock %d(%p), errno=%d, %s", iSocket, m_tIter.m_pData, errno, strerrorm ( errno ) );

		RemoveCurrentItem ();
	}
};

ISphNetEvents * sphCreatePoll ( int iSizeHint, bool )
{
	return new KqueueEvents_c ( iSizeHint );
}

#endif
#if POLLING_POLL
class PollEvents_c : public ISphNetEvents
{
private:
	CSphVector<const void *>	m_dWork;
	CSphVector<pollfd>	m_dEvents;
	int					m_iLastReportedErrno;
	int					m_iReady;
	NetEventsIterator_t	m_tIter;
	int					m_iIter;

public:
	explicit PollEvents_c ( int iSizeHint )
		: m_iLastReportedErrno ( -1 )
		, m_iReady ( 0 )
		, m_iIter ( -1)
	{
		m_dWork.Reserve ( iSizeHint );
		m_tIter.Reset();
	}

	~PollEvents_c () = default;

	void SetupEvent ( int iSocket, PoolEvents_e eFlags, const void * pData ) override
	{
		assert ( pData && iSocket>=0 );
		assert ( eFlags==SPH_POLL_WR || eFlags==SPH_POLL_RD );

		pollfd tEvent;
		tEvent.fd = iSocket;
		tEvent.events = (eFlags==SPH_POLL_RD ? POLLIN : POLLOUT);

		assert ( m_dEvents.GetLength() == m_dWork.GetLength() );
		m_dEvents.Add ( tEvent );
		m_dWork.Add ( pData );
	}

	bool Wait ( int timeoutMs ) override
	{
		// need positive timeout for communicate threads back and shutdown
		m_iReady = ::poll ( m_dEvents.Begin (), m_dEvents.GetLength (), timeoutMs );

		if ( m_iReady<0 )
		{
			int iErrno = sphSockGetErrno ();
			// common recoverable errors
			if ( iErrno==EINTR || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
				return false;

			if ( m_iLastReportedErrno!=iErrno )
			{
				sphWarning ( "poll tick failed: %s", sphSockError ( iErrno ) );
				m_iLastReportedErrno = iErrno;
			}
			return false;
		}

		return ( m_iReady>0 );
	}

	bool IterateNextAll () override
	{
		assert ( m_dEvents.GetLength ()==m_dWork.GetLength () );

		++m_iIter;
		m_tIter.m_pData = ( m_iIter<m_dWork.GetLength () ? m_dWork[m_iIter] : nullptr );

		return ( m_iIter<m_dWork.GetLength () );
	}

	bool IterateNextReady () override
	{
		m_tIter.Reset();

		if ( m_iReady<=0 || m_iIter>=m_dEvents.GetLength () )
			return false;

		while (true)
		{
			++m_iIter;
			if ( m_iIter>=m_dEvents.GetLength () )
				return false;

			if ( m_dEvents[m_iIter].revents==0 )
				continue;

			--m_iReady;

			m_tIter.m_pData = m_dWork[m_iIter];
			pollfd & tEv = m_dEvents[m_iIter];
			if ( tEv.revents & POLLIN )
				m_tIter.m_uEvents |= SPH_POLL_RD;

			if ( tEv.revents & POLLOUT )
				m_tIter.m_uEvents |= SPH_POLL_WR;

			if ( tEv.revents & POLLHUP )
				m_tIter.m_uEvents |= SPH_POLL_HUP;

			if ( tEv.revents & POLLERR )
				m_tIter.m_uEvents |= SPH_POLL_ERR;

			tEv.revents = 0;
			return true;
		}
	}

	void IterateChangeEvent ( int iSocket, PoolEvents_e eFlags ) override
	{
		assert ( m_iIter>=0 && m_iIter<m_dEvents.GetLength () );
		assert ( (SOCKET)iSocket == m_dEvents[m_iIter].fd );
		m_dEvents[m_iIter].events = (eFlags==SPH_POLL_RD ? POLLIN : POLLOUT);
	}

	void IterateRemove ( int iSocket ) override
	{
		assert ( m_iIter>=0 && m_iIter<m_dEvents.GetLength () );
		assert ( m_dEvents.GetLength ()==m_dWork.GetLength () );
		assert ( (SOCKET)iSocket == m_dEvents[m_iIter].fd );

		m_dEvents.RemoveFast ( m_iIter );
		// SafeDelete ( m_dWork[m_iIter] );
		m_dWork.RemoveFast ( m_iIter );

		--m_iIter;
		m_tIter.m_pData = nullptr;
	}

	int IterateStart () override
	{
		m_iIter = -1;
		m_tIter.Reset();

		return m_iReady;
	}

	NetEventsIterator_t & IterateGet () override
	{
		assert ( m_iIter>=0 && m_iIter<m_dWork.GetLength () );
		return m_tIter;
	}
};

ISphNetEvents * sphCreatePoll ( int iSizeHint, bool )
{
	return new PollEvents_c ( iSizeHint );
}

#endif
#if POLLING_SELECT

// used as fallback if no of modern (at least poll) functions available
class SelectEvents_c : public ISphNetEvents
{
private:
	CSphVector<const void *> m_dWork;
	CSphVector<int> m_dSockets;
	fd_set			m_fdsRead;
	fd_set			m_fdsReadResult;
	fd_set			m_fdsWrite;
	fd_set 			m_fdsWriteResult;
	int	m_iMaxSocket;
	int m_iLastReportedErrno;
	int m_iReady;
	NetEventsIterator_t m_tIter;
	int m_iIter;

public:
	explicit SelectEvents_c ( int iSizeHint )
		: m_iMaxSocket ( 0 ),
		m_iLastReportedErrno ( -1 ),
		m_iReady ( 0 ),
		m_iIter ( -1 )
	{
		m_dWork.Reserve ( iSizeHint );

		FD_ZERO ( &m_fdsRead );
		FD_ZERO ( &m_fdsWrite );

		m_tIter.Reset();
	}

	~SelectEvents_c () = default;

	void SetupEvent ( int iSocket, PoolEvents_e eFlags, const void * pData ) override
	{
		assert ( pData && iSocket>=0 );
		assert ( eFlags==SPH_POLL_WR || eFlags==SPH_POLL_RD );

		sphFDSet ( iSocket, (eFlags==SPH_POLL_RD ? &m_fdsRead : &m_fdsWrite));
		m_iMaxSocket = Max ( m_iMaxSocket, iSocket );

		assert ( m_dSockets.GetLength ()==m_dWork.GetLength () );
		m_dWork.Add ( pData );
		m_dSockets.Add (iSocket);
	}

	bool Wait ( int timeoutMs ) override
	{
		struct timeval tvTimeout;

		tvTimeout.tv_sec = (int) (timeoutMs / 1000); // full seconds
		tvTimeout.tv_usec = (int) (timeoutMs % 1000) * 1000; // microseconds
		m_fdsReadResult = m_fdsRead;
		m_fdsWriteResult = m_fdsWrite;
		m_iReady = ::select ( 1 + m_iMaxSocket, &m_fdsReadResult, &m_fdsWriteResult, NULL, &tvTimeout );

		if ( m_iReady<0 )
		{
			int iErrno = sphSockGetErrno ();
			// common recoverable errors
			if ( iErrno==EINTR || iErrno==EAGAIN || iErrno==EWOULDBLOCK )
				return false;

			if ( m_iLastReportedErrno!=iErrno )
			{
				sphWarning ( "poll (select version) tick failed: %s", sphSockError ( iErrno ) );
				m_iLastReportedErrno = iErrno;
			}
			return false;
		}

		return (m_iReady>0);
	}

	bool IterateNextAll () override
	{
		assert ( m_dSockets.GetLength ()==m_dWork.GetLength () );

		++m_iIter;
		m_tIter.m_pData = (m_iIter<m_dWork.GetLength () ? m_dWork[m_iIter] : nullptr);

		return (m_iIter<m_dWork.GetLength ());
	}

	bool IterateNextReady () override
	{
		m_tIter.Reset ();
		if ( m_iReady<=0 || m_iIter>=m_dWork.GetLength () )
			return false;

		while (true)
		{
			++m_iIter;
			if ( m_iIter>=m_dWork.GetLength () )
				return false;

			bool bReadable = FD_ISSET ( m_dSockets[m_iIter], &m_fdsReadResult );
			bool bWritable = FD_ISSET ( m_dSockets[m_iIter], &m_fdsWriteResult );

			if ( !(bReadable || bWritable))
				continue;

			--m_iReady;

			m_tIter.m_pData = m_dWork[m_iIter];

			if ( bReadable )
				m_tIter.m_uEvents |= SPH_POLL_RD;
			if ( bWritable )
				m_tIter.m_uEvents |= SPH_POLL_WR;

			return true;
		}
	}

	void IterateChangeEvent ( int iSocket, PoolEvents_e eFlags ) override
	{
		assert ( m_iIter>=0 && m_iIter<m_dSockets.GetLength () );
		int iSock = m_dSockets[m_iIter];
		assert ( iSock == iSocket );
		fd_set * pseton = (eFlags==SPH_POLL_RD ? &m_fdsRead : &m_fdsWrite);
		fd_set * psetoff = (eFlags==SPH_POLL_RD ? &m_fdsWrite : &m_fdsRead);
		if ( FD_ISSET ( iSock, psetoff) ) sphFDClr ( iSock, psetoff );
		if ( !FD_ISSET ( iSock, pseton ) ) sphFDSet ( iSock, pseton );
	}

	void IterateRemove ( int iSocket ) override
	{
		assert ( m_iIter>=0 && m_iIter<m_dSockets.GetLength () );
		assert ( m_dSockets.GetLength ()==m_dWork.GetLength () );
		assert ( iSocket==m_dSockets[m_iIter] );

		int iSock = m_dSockets[m_iIter];
		if ( FD_ISSET ( iSock, &m_fdsWrite ) ) sphFDClr ( iSock, &m_fdsWrite );
		if ( FD_ISSET ( iSock, &m_fdsRead ) ) sphFDClr ( iSock, &m_fdsRead );
		m_dSockets.RemoveFast ( m_iIter );
		// SafeDelete ( m_dWork[m_iIter] );
		m_dWork.RemoveFast ( m_iIter );

		--m_iIter;
		m_tIter.Reset();
	}

	int IterateStart () override
	{
		m_iIter = -1;
		m_tIter.Reset();

		return m_iReady;
	}

	NetEventsIterator_t &IterateGet () override
	{
		assert ( m_iIter>=0 && m_iIter<m_dWork.GetLength () );
		return m_tIter;
	}
};

class DummyEvents_c : public ISphNetEvents
{
	NetEventsIterator_t m_tIter;

public:
	void SetupEvent ( int, PoolEvents_e, const void * ) override {}
	bool Wait ( int ) override { return false; } // NOLINT
	bool IterateNextAll () override { return false; }
	bool IterateNextReady () override { return false; }
	void IterateChangeEvent ( int, PoolEvents_e ) override {}
	void IterateRemove ( int ) override {}
	int IterateStart () override { return 0; }
	NetEventsIterator_t & IterateGet () override { return m_tIter; }
};

ISphNetEvents * sphCreatePoll (int iSizeHint, bool bFallbackSelect)
{
	if (!bFallbackSelect)
		return new DummyEvents_c;

	return new SelectEvents_c( iSizeHint);

}

#endif