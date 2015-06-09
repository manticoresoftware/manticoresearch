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
#include "sphinxstd.h"
#include "sphinxrt.h"
#include "sphinxint.h"

#include "searchdaemon.h"
#include "searchdha.h"


int				g_iPingInterval		= 0;		// by default ping HA agents every 1 second
DWORD			g_uHAPeriodKarma	= 60;		// by default use the last 1 minute statistic to determine the best HA agent

int				g_iPersistentPoolSize	= 0;

CSphString AgentDesc_c::GetMyUrl() const
{
	CSphString sName;
	switch ( m_iFamily )
	{
	case AF_INET: sName.SetSprintf ( "%s:%u", m_sHost.cstr(), m_iPort ); break;
	case AF_UNIX: sName = m_sPath; break;
	}
	return sName;
}


void HostDashboard_t::Init ( AgentDesc_c * pAgent )
{
	m_iRefCount = 1;
	m_iLastQueryTime = m_iLastAnswerTime = sphMicroTimer() - g_iPingInterval*1000;
	m_iErrorsARow = 0;
	m_tDescriptor = *pAgent;
	m_bNeedPing = false;
}

bool HostDashboard_t::IsOlder ( int64_t iTime ) const
{
	return ( (iTime-m_iLastAnswerTime)>g_iPingInterval*1000 );
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

AgentDash_t*	HostDashboard_t::GetCurrentStat()
{
	DWORD uTime = GetCurSeconds()/g_uHAPeriodKarma;
	int iIdx = uTime % STATS_DASH_TIME;
	AgentDash_t & dStats = m_dStats[iIdx];
	if ( dStats.m_uTimestamp!=uTime ) // we have new or reused stat
		dStats.Reset();
	dStats.m_uTimestamp = uTime;
	return &dStats;
}

int HostDashboard_t::GetDashStat ( AgentDash_t* pRes, int iPeriods ) const
{
	assert ( pRes );
	pRes->Reset();

	int iCollected = 0;

	DWORD uSeconds = GetCurSeconds();
	if ( (uSeconds % g_uHAPeriodKarma) < (g_uHAPeriodKarma/2) )
		++iPeriods;

	int iLimit = Min ( iPeriods, STATS_DASH_TIME );
	int iMaxExpected = iLimit + 1;
	DWORD uTime = uSeconds/g_uHAPeriodKarma;
	int iIdx = uTime % STATS_DASH_TIME;
	// pRes->m_uTimestamp = uTime;

	for ( ; iLimit>0 ; --iLimit )
	{
		const AgentDash_t & dStats = m_dStats[iIdx];
		if ( dStats.m_uTimestamp==uTime ) // it might be no queries at all in the fixed time
		{
			pRes->Add ( dStats );
			iCollected = iLimit;
		}

		--uTime;
		--iIdx;
		if ( iIdx<0 )
			iIdx = STATS_DASH_TIME-1;
	}

	return iMaxExpected - iCollected;
}

static CSphVector<int>	g_dPersistentConnections; // protect by CSphScopedLock<ThreadsOnlyMutex_t> tLock ( g_tPersLock );
CSphMutex				g_tPersLock;


int RentPersistent_c::RentConnection ()
{
	// for the moment we try to return already connected socket.
	CSphScopedLock<CSphMutex> tLock ( g_tPersLock );
	int* pFree = &g_dPersistentConnections[m_iPersPool];
	if ( *pFree==-1 ) // fresh pool, just cleared. All slots are free
		*pFree = g_iPersistentPoolSize;
	if ( !*pFree )
		return -2; // means 'no free slots'
	int iSocket = g_dPersistentConnections[m_iPersPool+*pFree];
	g_dPersistentConnections[m_iPersPool+*pFree] = -1;
	--(*pFree);
	return iSocket;
}

void RentPersistent_c::ReturnConnection ( int iSocket )
{
	// for the moment we try to return already connected socket.
	CSphScopedLock<CSphMutex> tLock ( g_tPersLock );
	int* pFree = &g_dPersistentConnections[m_iPersPool];
	assert ( *pFree<g_iPersistentPoolSize );
	if ( *pFree==g_iPersistentPoolSize )
	{
		sphSockClose ( iSocket );
		return;
	}
	++(*pFree);
	g_dPersistentConnections[m_iPersPool+*pFree] = iSocket;
}

void ResizePersistentConnectionsPool ( int iSize )
{
	g_dPersistentConnections.Resize ( iSize );
	ARRAY_FOREACH ( i, g_dPersistentConnections )
		g_dPersistentConnections[i] = -1; // means "Not in use"
}

void ClosePersistentSockets()
{
	CSphScopedLock<CSphMutex> tLock ( g_tPersLock );
	ARRAY_FOREACH ( i, g_dPersistentConnections )
	{
		if ( ( i % g_iPersistentPoolSize ) && g_dPersistentConnections[i]>=0 )
			sphSockClose ( g_dPersistentConnections[i] );
		g_dPersistentConnections[i] = -1;
	}
}


//////////////////////////////////////////////////////////////////////////

void MetaAgentDesc_t::SetHAData ( int * pRRCounter, WORD * pWeights, InterWorkerStorage * pLock )
{
	m_pRRCounter = pRRCounter;
	m_pWeights = pWeights;
	m_pLock = pLock;
}

AgentDesc_c * MetaAgentDesc_t::GetAgent ( int iAgent )
{
	assert ( iAgent>=0 );
	return &m_dHosts[iAgent];
}

AgentDesc_c * MetaAgentDesc_t::NewAgent()
{
	AgentDesc_c & tAgent = m_dHosts.Add();
	return & tAgent;
}

AgentDesc_c * MetaAgentDesc_t::LastAgent()
{
	assert ( m_dHosts.GetLength()>0 );
	return &m_dHosts.Last();
}

AgentDesc_c * MetaAgentDesc_t::RRAgent ()
{
	assert ( m_pRRCounter );
	assert ( m_pLock );

	if ( m_dHosts.GetLength()==1 )
		return GetAgent(0);

	CSphScopedLock<InterWorkerStorage> tLock ( *m_pLock );

	++*m_pRRCounter;
	if ( *m_pRRCounter<0 || *m_pRRCounter>(m_dHosts.GetLength()-1) )
		*m_pRRCounter = 0;


	AgentDesc_c * pAgent = GetAgent ( *m_pRRCounter );
	return pAgent;
}

AgentDesc_c * MetaAgentDesc_t::RandAgent ()
{
	return GetAgent ( sphRand() % m_dHosts.GetLength() );
}

void MetaAgentDesc_t::RecalculateWeights ( const CSphVector<int64_t> &dTimers )
{
	// minimal probability must not fall below the original one with this coef.
	const float fMin_coef = 0.1f;
	DWORD uMin_value = DWORD ( 65535*fMin_coef/m_dHosts.GetLength() );

	if ( m_pWeights && HostDashboard_t::IsHalfPeriodChanged ( &m_uTimestamp ) )
	{
		int64_t dMin = -1;
		ARRAY_FOREACH ( i, dTimers )
			if ( dTimers[i] > 0 )
			{
				if ( dMin<=0 )
					dMin = dTimers[i];
				else
					dMin = Min ( dMin, dTimers[i] );
			}

			if ( dMin<=0 ) // no statistics, all timers bad.
				return;

			// apply coefficients
			float fNormale = 0;
			CSphVector<float> dCoefs ( dTimers.GetLength() );
			assert ( m_pLock );
			CSphScopedLock<InterWorkerStorage> tLock ( *m_pLock );
			ARRAY_FOREACH ( i, dTimers )
			{
				if ( dTimers[i] > 0 )
				{
					dCoefs[i] = (float)dMin/dTimers[i];
					if ( m_pWeights[i]*dCoefs[i] < uMin_value )
						dCoefs[i] = (float)uMin_value/m_pWeights[i]; // restrict balancing like 1/0 into 0.9/0.1
				} else
					dCoefs[i] = (float)uMin_value/m_pWeights[i];

				fNormale += m_pWeights[i]*dCoefs[i];
			}

			// renormalize the weights
			fNormale = 65535/fNormale;
			DWORD uCheck = 0;
			ARRAY_FOREACH ( i, m_dHosts )
			{
				m_pWeights[i] = WORD ( m_pWeights[i]*dCoefs[i]*fNormale );
				uCheck += m_pWeights[i];
				sphLogDebug ( "Mirror %d, new weight (%d)", i, m_pWeights[i] );
			}
			sphLogDebug ( "Rebalancing finished. The whole sum is %d", uCheck );
	}
}

void MetaAgentDesc_t::WeightedRandAgent ( int * pBestAgent, CSphVector<int> & dCandidates )
{
	assert ( m_pWeights );
	assert ( pBestAgent );
	assert ( m_pLock );
	CSphScopedLock<InterWorkerStorage> tLock ( *m_pLock );
	DWORD uBound = m_pWeights[*pBestAgent];
	DWORD uLimit = uBound;
	ARRAY_FOREACH ( i, dCandidates )
		uLimit += m_pWeights[dCandidates[i]];
	DWORD uChance = sphRand() % uLimit;

	if ( uChance<=uBound )
		return;

	ARRAY_FOREACH ( i, dCandidates )
	{
		uBound += m_pWeights[dCandidates[i]];
		*pBestAgent = dCandidates[i];
		if ( uChance<=uBound )
			break;
	}
}

inline const HostDashboard_t& MetaAgentDesc_t::GetCommonStat ( int iAgent ) const
{
	return g_tStats.m_dDashboard.m_dItemStats[m_dHosts[iAgent].m_iDashIndex];
}

inline int64_t MetaAgentDesc_t::GetBestDelay ( int * pBestAgent, CSphVector<int> & dCandidates ) const
{
	assert ( pBestAgent );
	int64_t iBestAnswerTime = GetCommonStat ( *pBestAgent ).m_iLastAnswerTime
		- GetCommonStat ( *pBestAgent ).m_iLastQueryTime;
	ARRAY_FOREACH ( i, dCandidates )
	{
		if ( iBestAnswerTime > ( GetCommonStat ( dCandidates[i] ).m_iLastAnswerTime
			- GetCommonStat ( dCandidates[i] ).m_iLastQueryTime ) )
		{
			*pBestAgent = dCandidates[i];
			iBestAnswerTime = GetCommonStat ( *pBestAgent ).m_iLastAnswerTime
				- GetCommonStat ( *pBestAgent ).m_iLastQueryTime;
		}
	}
	return iBestAnswerTime;
}


AgentDesc_c * MetaAgentDesc_t::StDiscardDead ()
{
	if ( m_dHosts.GetLength()==1 )
		return GetAgent(0);

	// threshold errors-a-row to be counted as dead
	int iDeadThr = 3;

	int iBestAgent = -1;
	int64_t iErrARow = -1;
	int64_t iThisErrARow = -1;
	CSphVector<int> dCandidates;
	CSphVector<int64_t> dTimers;
	dCandidates.Reserve ( m_dHosts.GetLength() );
	dTimers.Resize ( m_dHosts.GetLength() );

	ARRAY_FOREACH ( i, m_dHosts )
	{
		// no locks for g_pStats since we just reading, and read data is not critical.
		const HostDashboard_t & dDash = GetCommonStat ( i );

		AgentDash_t dDashStat;
		dDash.GetDashStat ( &dDashStat, 1 ); // look at last 30..90 seconds.
		uint64_t uQueries = 0;
		for ( int j=0; j<eMaxCounters; ++j )
			uQueries += dDashStat.m_iStats[j];
		if ( uQueries > 0 )
			dTimers[i] = dDashStat.m_iStats[eTotalMsecs]/uQueries;
		else
			dTimers[i] = -1;

		iThisErrARow = ( dDash.m_iErrorsARow<=iDeadThr ) ? 0 : dDash.m_iErrorsARow;

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
	RecalculateWeights ( dTimers );

	// nothing to select, sorry. Just plain RR...
	if ( iBestAgent < 0 )
	{
		sphLogDebug ( "HA selector discarded all the candidates and just fall into simple Random" );
		return RandAgent();
	}

	// only one node with lowest error rating. Return it.
	if ( !dCandidates.GetLength() )
	{
		sphLogDebug ( "HA selected %d node with best num of errors a row ("INT64_FMT")", iBestAgent, iErrARow );
		return &m_dHosts[iBestAgent];
	}

	// several nodes. Let's select the one.
	float fAge = 0.0;
	const char * sLogStr = NULL;

	WeightedRandAgent ( &iBestAgent, dCandidates );
	if ( g_eLogLevel>=SPH_LOG_DEBUG )
	{
		const HostDashboard_t & dDash = GetCommonStat ( iBestAgent );
		fAge = ( dDash.m_iLastAnswerTime-dDash.m_iLastQueryTime ) / 1000.0f;
		sLogStr = "HA selected %d node by weighted random, with best EaR ("INT64_FMT"), last answered in %f milliseconds";
	}

	sphLogDebug ( sLogStr, iBestAgent, iErrARow, fAge );
	return &m_dHosts[iBestAgent];
}

AgentDesc_c * MetaAgentDesc_t::StLowErrors()
{
	if ( m_dHosts.GetLength()==1 )
		return GetAgent(0);

	// how much error rating is allowed
	float fAllowedErrorRating = 0.03f; // i.e. 3 errors per 100 queries is still ok

	int iBestAgent = -1;
	float fBestCriticalErrors = 1.0;
	float fBestAllErrors = 1.0;
	CSphVector<int> dCandidates;
	CSphVector<int64_t> dTimers;
	dCandidates.Reserve ( m_dHosts.GetLength() );
	dTimers.Resize ( m_dHosts.GetLength() );

	ARRAY_FOREACH ( i, m_dHosts )
	{
		// no locks for g_pStats since we just reading, and read data is not critical.
		const HostDashboard_t & dDash = GetCommonStat ( i );

		AgentDash_t dDashStat;
		dDash.GetDashStat ( &dDashStat, 1 ); // look at last 30..90 seconds.
		uint64_t uQueries = 0;
		uint64_t uCriticalErrors = 0;
		uint64_t uAllErrors = 0;
		uint64_t uSuccesses = 0;
		for ( int j=0; j<eMaxCounters; ++j )
		{
			if ( j==eNetworkCritical )
				uCriticalErrors = uQueries;
			else if ( j==eNetworkNonCritical )
			{
				uAllErrors = uQueries;
				uSuccesses = dDashStat.m_iStats[j];
			}
			uQueries += dDashStat.m_iStats[j];
		}

		if ( uQueries > 0 )
			dTimers[i] = dDashStat.m_iStats[eTotalMsecs]/uQueries;
		else
			dTimers[i] = -1;

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
	RecalculateWeights ( dTimers );

	// nothing to select, sorry. Just plain RR...
	if ( iBestAgent < 0 )
	{
		sphLogDebug ( "HA selector discarded all the candidates and just fall into simple Random" );
		return RandAgent();
	}

	// only one node with lowest error rating. Return it.
	if ( !dCandidates.GetLength() )
	{
		sphLogDebug ( "HA selected %d node with best error rating (%.2f)", iBestAgent, fBestCriticalErrors );
		return &m_dHosts[iBestAgent];
	}

	// several nodes. Let's select the one.
	float fAge = 0.0f;
	const char * sLogStr = NULL;
	WeightedRandAgent ( &iBestAgent, dCandidates );
	if ( g_eLogLevel>=SPH_LOG_DEBUG )
	{
		const HostDashboard_t & dDash = GetCommonStat ( iBestAgent );
		fAge = ( dDash.m_iLastAnswerTime-dDash.m_iLastQueryTime ) / 1000.0f;
		sLogStr = "HA selected %d node by weighted random, with best error rating (%.2f), answered %f seconds ago";
	}

	sphLogDebug ( sLogStr, iBestAgent, fBestCriticalErrors, fAge );
	return &m_dHosts[iBestAgent];
}


AgentDesc_c * MetaAgentDesc_t::GetRRAgent ()
{
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


void MetaAgentDesc_t::QueuePings()
{
	if ( !IsHA() )
		return;

	g_tStatsMutex.Lock();
	ARRAY_FOREACH ( i, m_dHosts )
		g_tStats.m_dDashboard.m_dItemStats[m_dHosts[i].m_iDashIndex].m_bNeedPing = true;
	g_tStatsMutex.Unlock();
}

const CSphVector<AgentDesc_c>& MetaAgentDesc_t::GetAgents() const
{
	return m_dHosts;
}

const WORD* MetaAgentDesc_t::GetWeights() const
{
	return m_pWeights;
}

MetaAgentDesc_t & MetaAgentDesc_t::operator= ( const MetaAgentDesc_t & rhs )
{
	if ( this==&rhs )
		return *this;
	m_dHosts = rhs.GetAgents();
	m_pWeights = rhs.m_pWeights;
	m_pRRCounter = rhs.m_pRRCounter;
	m_pLock = rhs.m_pLock;
	m_eStrategy = rhs.m_eStrategy;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
AgentConn_t::AgentConn_t ()
	: m_iSock ( -1 )
	, m_bFresh ( true )
	, m_eState ( AGENT_UNUSED )
	, m_bSuccess ( false )
	, m_iReplyStatus ( -1 )
	, m_iReplySize ( 0 )
	, m_iReplyRead ( 0 )
	, m_pReplyBuf ( NULL )
	, m_iWall ( 0 )
	, m_iWaited ( 0 )
	, m_iStartQuery ( 0 )
	, m_iEndQuery ( 0 )
	, m_iWorkerTag ( -1 )
	, m_iStoreTag ( 0 )
	, m_iWeight ( -1 )
{}

AgentConn_t::~AgentConn_t ()
{
	Close ( false );
	if ( m_bPersistent )
		m_dPersPool.ReturnConnection ( m_iSock );
}

void AgentConn_t::Close ( bool bClosePersist )
{
	SafeDeleteArray ( m_pReplyBuf );
	if ( m_iSock>0 )
	{
		m_bFresh = false;
		if ( ( m_bPersistent && bClosePersist ) || !m_bPersistent )
		{
			sphSockClose ( m_iSock );
			m_iSock = -1;
			m_bFresh = true;
		}
		if ( m_eState!=AGENT_RETRY )
			m_eState = AGENT_UNUSED;
	}
	m_iWall += sphMicroTimer ();
}

AgentConn_t & AgentConn_t::operator = ( const AgentDesc_c & rhs )
{
	m_sHost = rhs.m_sHost;
	m_iPort = rhs.m_iPort;
	m_sPath = rhs.m_sPath;
	m_sIndexes = rhs.m_sIndexes;
	m_bBlackhole = rhs.m_bBlackhole;
	m_iFamily = rhs.m_iFamily;
	m_uAddr = rhs.m_uAddr;
	m_iStatsIndex = rhs.m_iStatsIndex;
	m_iDashIndex = rhs.m_iDashIndex;
	m_bPersistent = rhs.m_bPersistent;
	m_dPersPool = rhs.m_dPersPool;

	return *this;
}

// works like =, but also adopt the persistent connection, if any.
void AgentConn_t::TakeTraits ( AgentDesc_c & rhs )
{
	*this = rhs;
	if ( m_bPersistent )
	{
		m_iSock = m_dPersPool.RentConnection();
		if ( m_iSock==-2 ) // no free persistent connections. This connection will be not persistent
			m_bPersistent = false;
	}
	m_bFresh = ( m_bPersistent && m_iSock<0 );
}

void AgentConn_t::Fail ( AgentStats_e eStat, const char* sMessage, ... )
{
	Close ();
	va_list ap;
	va_start ( ap, sMessage );
	m_sFailure.SetSprintfVa ( sMessage, ap );
	va_end ( ap );
	agent_stats_inc ( *this, eStat );
}

SearchdStats_t			g_tStats;
CSphMutex				g_tStatsMutex;

void agent_stats_inc ( AgentConn_t & tAgent, AgentStats_e iCounter )
{
	if ( tAgent.m_iStatsIndex>=0 && tAgent.m_iStatsIndex<STATS_MAX_AGENTS )
	{
		g_tStatsMutex.Lock ();
		uint64_t* pCurStat = g_tStats.m_dDashboard.m_dItemStats [ tAgent.m_iDashIndex ].GetCurrentStat()->m_iStats;
		if ( iCounter==eMaxMsecs || iCounter==eAverageMsecs )
		{
			++pCurStat[eConnTries];
			int64_t iConnTime = sphMicroTimer() - tAgent.m_iStartQuery;
			if ( uint64_t(iConnTime)>pCurStat[eMaxMsecs] )
				pCurStat[eMaxMsecs] = iConnTime;
			if ( pCurStat[eConnTries]>1 )
				pCurStat[eAverageMsecs] = (pCurStat[eAverageMsecs]*(pCurStat[eConnTries]-1)+iConnTime)/pCurStat[eConnTries];
			else
				pCurStat[eAverageMsecs] = iConnTime;
		} else
		{
			g_tStats.m_dAgentStats.m_dItemStats [ tAgent.m_iStatsIndex ].m_iStats[iCounter]++;
			if ( tAgent.m_iDashIndex>=0 && tAgent.m_iDashIndex<STATS_MAX_DASH )
			{
				++pCurStat[iCounter];
				if ( iCounter>=eNoErrors && iCounter<eMaxCounters )
					g_tStats.m_dDashboard.m_dItemStats [ tAgent.m_iDashIndex ].m_iErrorsARow = 0;
				else
					g_tStats.m_dDashboard.m_dItemStats [ tAgent.m_iDashIndex ].m_iErrorsARow += 1;
				tAgent.m_iEndQuery = sphMicroTimer();
				g_tStats.m_dDashboard.m_dItemStats [ tAgent.m_iDashIndex ].m_iLastQueryTime = tAgent.m_iStartQuery;
				g_tStats.m_dDashboard.m_dItemStats [ tAgent.m_iDashIndex ].m_iLastAnswerTime = tAgent.m_iEndQuery;
				pCurStat[eTotalMsecs]+=tAgent.m_iEndQuery-tAgent.m_iStartQuery;
			}
		}
		g_tStatsMutex.Unlock ();
	}
}

#define sphStrMatchStatic(_str, _cstr) ( strncmp ( _str, _cstr, sizeof(_str)-1 )==0 )


bool ParseStrategyHA ( const char * sName, HAStrategies_e & eStrategy )
{
	if ( sphStrMatchStatic ( "random", sName ) )
		eStrategy = HA_RANDOM;
	else if ( sphStrMatchStatic ( "roundrobin", sName ) )
		eStrategy = HA_ROUNDROBIN;
	else if ( sphStrMatchStatic ( "nodeads", sName ) )
		eStrategy = HA_AVOIDDEAD;
	else if ( sphStrMatchStatic ( "noerrors", sName ) )
		eStrategy = HA_AVOIDERRORS;
	else
		return false;

	return true;
}

static bool IsAgentDelimiter ( char c )
{
	return c=='|' || c=='[' || c==']';
}


enum AgentParse_e { apInHost, apInPort, apStartIndexList, apIndexList, apOptions, apDone };

bool ConfigureAgent ( MetaAgentDesc_t & tAgent, const CSphVariant * pAgent, const char * szIndexName, AgentOptions_t tDesc )
{
	AgentDesc_c * pCurrent = tAgent.NewAgent();

	// extract host name or path
	const char * p = pAgent->cstr();
	while ( *p && isspace ( *p ) )
		p++;
	AgentParse_e eState = apDone;
	// might be agent options at head
	if ( *p )
	{
		if ( *p=='[' )
		{
			eState = apOptions;
			p += 1;
		} else
			eState = apInHost;
	}
	const char * pAnchor = p;

	while ( eState!=apDone )
	{
		switch ( eState )
		{
		case apInHost:
			{
				if ( !*p )
				{
					eState = apDone;
					break;
				}

				if ( sphIsAlpha(*p) || *p=='.' || *p=='-' || *p=='/' )
					break;

				if ( p==pAnchor )
				{
					sphWarning ( "index '%s': agent '%s': host name or path expected - SKIPPING AGENT",
						szIndexName, pAnchor );
					return false;
				}
				if ( *p!=':' )
				{
					sphWarning ( "index '%s': agent '%s': colon expected near '%s' - SKIPPING AGENT",
						szIndexName, pAgent->cstr(), p );
					return false;
				}
				CSphString sSub = pAgent->strval().SubString ( pAnchor-pAgent->cstr(), p-pAnchor );
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

					pCurrent->m_iFamily = AF_UNIX;
					pCurrent->m_sPath = sSub;
					p--;
#endif
				} else
				{
					pCurrent->m_iFamily = AF_INET;
					pCurrent->m_sHost = sSub;
				}
				eState = apInPort;
				pAnchor = p+1;
				break;
			}
		case apInPort:
			{
				if ( isdigit(*p) )
					break;

#if !USE_WINDOWS
				if ( pCurrent->m_iFamily!=AF_UNIX )
				{
#endif
					if ( p==pAnchor )
					{
						sphWarning ( "index '%s': agent '%s': port number expected near '%s' - SKIPPING AGENT",
							szIndexName, pAgent->cstr(), p );
						return false;
					}
					pCurrent->m_iPort = atoi ( pAnchor );

					if ( !IsPortInRange ( pCurrent->m_iPort ) )
					{
						sphWarning ( "index '%s': agent '%s': invalid port number near '%s' - SKIPPING AGENT",
							szIndexName, pAgent->cstr(), p );
						return false;
					}
#if !USE_WINDOWS
				}
#endif

				if ( IsAgentDelimiter ( *p ) )
				{
					eState = ( *p=='|' ? apInHost : apOptions );
					pAnchor = p+1;
					if ( !ValidateAgentDesc ( tAgent, pAgent, szIndexName ) )
						return false;
					pCurrent = tAgent.NewAgent();
					break;
				}

				if ( *p!=':' )
				{
					sphWarning ( "index '%s': agent '%s': colon expected near '%s' - SKIPPING AGENT",
						szIndexName, pAgent->cstr(), p );
					return false;
				}

				eState = apStartIndexList;
				pAnchor = p+1;
				break;
			}
		case apStartIndexList:
			if ( isspace ( *p ) )
				break;

			pAnchor = p;
			eState = apIndexList;
			// no break;
		case apIndexList:
			{
				if ( sphIsAlpha(*p) || isspace(*p) || *p==',' )
					break;

				CSphString sIndexes = pAgent->strval().SubString ( pAnchor-pAgent->cstr(), p-pAnchor );

				if ( *p && !IsAgentDelimiter ( *p ) )
				{
					sphWarning ( "index '%s': agent '%s': index list expected near '%s' - SKIPPING AGENT",
						szIndexName, pAgent->cstr(), p );
					return false;
				}
				pCurrent->m_sIndexes = sIndexes;

				if ( IsAgentDelimiter ( *p ) )
				{
					if ( *p=='|' )
					{
						eState = apInHost;
						if ( !ValidateAgentDesc ( tAgent, pAgent, szIndexName ) )
							return false;
						pCurrent = tAgent.NewAgent();
					} else
						eState = apOptions;

					pAnchor = p+1;
					break;
				} else
					eState = apDone;
			}
			break;

		case apOptions:
			{
				const char * sOptName = NULL;
				const char * sOptValue = NULL;
				bool bGotEq = false;
				while ( *p )
				{
					bool bNextOpt = ( *p==',' );
					bool bNextAgent = IsAgentDelimiter ( *p );
					bGotEq |= ( *p=='=' );

					if ( bNextOpt || bNextAgent )
					{
						if ( sOptName && sOptValue )
						{
							bool bParsed = false;
							if ( sphStrMatchStatic ( "conn", sOptName ) )
							{
								if ( sphStrMatchStatic ( "pconn", sOptValue ) || sphStrMatchStatic ( "persistent", sOptValue ) )
								{
									tDesc.m_bPersistent = true;
									bParsed = true;
								}
							} else if ( sphStrMatchStatic ( "ha_strategy", sOptName ) )
							{
								bParsed = ParseStrategyHA ( sOptValue, tDesc.m_eStrategy );
							} else if ( sphStrMatchStatic ( "blackhole", sOptName ) )
							{
								tDesc.m_bBlackhole = ( atoi ( sOptValue )!=0 );
								bParsed = true;
							}

							if ( !bParsed )
							{
								CSphString sInvalid;
								sInvalid.SetBinary ( sOptName, p-sOptName );
								sphWarning ( "index '%s': agent '%s': unknown agent option '%s' ", szIndexName, pAgent->cstr(), sInvalid.cstr() );
							}
						}

						sOptName = sOptValue = NULL;
						bGotEq = false;
						if ( bNextAgent )
							break;
					}

					if ( sphIsAlpha ( *p ) )
					{
						if ( !sOptName )
							sOptName = p;
						else if ( bGotEq && !sOptValue )
							sOptValue = p;
					}

					p++;
				}

				if ( IsAgentDelimiter ( *p ) )
				{
					eState = apInHost;
					pAnchor = p+1;
				} else
					eState = apDone;
			}
			break;

		case apDone:
		default:
			break;
		} // switch (eState)
		p++;
	} // while (eState!=apDone)

	bool bRes = ValidateAgentDesc ( tAgent, pAgent, szIndexName );
	tAgent.QueuePings();
	if ( tDesc.m_bPersistent )
		tAgent.SetPersistent();
	if ( tDesc.m_bBlackhole )
		tAgent.SetBlackhole();
	tAgent.SetStrategy ( tDesc.m_eStrategy );

	return bRes;
}

#undef sphStrMatchStatic

bool ValidateAgentDesc ( MetaAgentDesc_t & tAgent, const CSphVariant * pLine, const char * szIndexName )
{
	AgentDesc_c * pAgent = tAgent.LastAgent();
	assert ( pAgent );

	// lookup address (if needed)
	if ( pAgent->m_iFamily==AF_INET )
	{
		if ( pAgent->m_sHost.IsEmpty() )
		{
			sphWarning ( "index '%s': agent '%s': invalid host name 'empty' - SKIPPING AGENT",
						szIndexName, pLine->cstr() );
			return false;
		}

		pAgent->m_uAddr = sphGetAddress ( pAgent->m_sHost.cstr() );
		if ( pAgent->m_uAddr==0 )
		{
			sphWarning ( "index '%s': agent '%s': failed to lookup host name '%s' (error=%s) - SKIPPING AGENT",
				szIndexName, pLine->cstr(), pAgent->m_sHost.cstr(), sphSockError() );
			return false;
		}
	}

	// hash for dashboard
	CSphString sHashKey = pAgent->GetMyUrl();
	// allocate stats slot
	// let us cheat and also allocate the dashboard slot under the same lock
	g_tStatsMutex.Lock();

	pAgent->m_iStatsIndex = g_tStats.m_dAgentStats.AllocItem();
	if ( pAgent->m_iStatsIndex<0 )
		sphWarning ( "index '%s': agent '%s': failed to allocate slot for stats%s",
		szIndexName, pLine->cstr(), ( tAgent.IsHA() ? ", HA might be wrong" : "" ) );

	if ( g_tStats.m_hDashBoard.Exists ( sHashKey ) )
	{
		pAgent->m_iDashIndex = g_tStats.m_hDashBoard[sHashKey];
		g_tStats.m_dDashboard.m_dItemStats[pAgent->m_iDashIndex].m_iRefCount++;
	} else
	{
		pAgent->m_iDashIndex = g_tStats.m_dDashboard.AllocItem();
		if ( pAgent->m_iDashIndex<0 )
		{
			sphWarning ( "index '%s': agent '%s': failed to allocate slot for stat-dashboard%s",
				szIndexName, pLine->cstr(), ( tAgent.IsHA() ? ", HA might be wront" : "" ) );
		} else
		{
			g_tStats.m_dDashboard.m_dItemStats[pAgent->m_iDashIndex].Init ( pAgent );
			g_tStats.m_hDashBoard.Add ( pAgent->m_iDashIndex, sHashKey );
		}
	}

	g_tStatsMutex.Unlock();

	// for now just convert all 'host mirrors' (i.e. agents without indices) into 'index mirrors'
	if ( tAgent.GetLength()>1 && !pAgent->m_sIndexes.IsEmpty() )
	{
		for ( int i=tAgent.GetLength()-2; i>=0; --i )
		{
			AgentDesc_c * pMyAgent = tAgent.GetAgent(i);
			if ( pMyAgent->m_sIndexes.IsEmpty() )
				pMyAgent->m_sIndexes = pAgent->m_sIndexes;
			else
				break;
		}
	}
	return true;
}

void FreeDashboard ( const AgentDesc_c& dAgent, DashBoard_t& dDashBoard, SmallStringHash_T<int>& hDashBoard )
{
	HostDashboard_t& dDash = dDashBoard.m_dItemStats[dAgent.m_iDashIndex];

	--dDash.m_iRefCount;
	assert ( dDash.m_iRefCount>=0 );
	if ( dDash.m_iRefCount<=0 ) // no more agents use this host. Delete record.
	{
		dDashBoard.FreeItem ( dAgent.m_iDashIndex );
		hDashBoard.Delete ( dAgent.GetMyUrl() );
	}
}

void FreeAgentStats ( const AgentDesc_c& dAgent )
{
	g_tStatsMutex.Lock();
	g_tStats.m_dDashboard.FreeItem ( dAgent.m_iStatsIndex );

	// now free the dashboard hosts also
	FreeDashboard ( dAgent, g_tStats.m_dDashboard, g_tStats.m_hDashBoard );
}

void FreeHostStats ( MetaAgentDesc_t & tHosts )
{
	ARRAY_FOREACH ( j, tHosts.GetAgents() )
		FreeAgentStats ( tHosts.GetAgents()[j] );
}

void RemoteConnectToAgent ( AgentConn_t & tAgent )
{
	bool bAgentRetry = ( tAgent.m_eState==AGENT_RETRY );
	tAgent.m_eState = AGENT_UNUSED;

	if ( tAgent.m_iSock>=0 ) // already connected
	{
		if ( !sphSockEof ( tAgent.m_iSock ) )
		{
			tAgent.m_eState = AGENT_ESTABLISHED;
			tAgent.m_iStartQuery = sphMicroTimer();
			tAgent.m_iWall -= tAgent.m_iStartQuery;
			return;
		}
		tAgent.Close();
	}

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

#ifdef TCP_NODELAY
	int iOn = 1;
	if ( setsockopt ( tAgent.m_iSock, IPPROTO_TCP, TCP_NODELAY, (char*)&iOn, sizeof(iOn) ) )
	{
		tAgent.m_sFailure.SetSprintf ( "setsockopt() failed: %s", sphSockError() );
		return;
	}
#endif

	// count connects
	g_tStatsMutex.Lock();
	g_tStats.m_iAgentConnect++;
	g_tStats.m_iAgentRetry += ( bAgentRetry );
	g_tStatsMutex.Unlock();

	tAgent.m_iStartQuery = sphMicroTimer();
	tAgent.m_iWall -= tAgent.m_iStartQuery;
	if ( connect ( tAgent.m_iSock, (struct sockaddr*)&ss, len )<0 )
	{
		int iErr = sphSockGetErrno();
		if ( iErr!=EINPROGRESS && iErr!=EINTR && iErr!=EWOULDBLOCK ) // check for EWOULDBLOCK is for winsock only
		{
			tAgent.Fail ( eConnectFailures, "connect() failed: %s", sphSockError(iErr) );
			tAgent.m_eState = AGENT_RETRY; // do retry on connect() failures
			return;

		} else
		{
			// connection in progress
			tAgent.m_eState = AGENT_CONNECTING;
		}
	} else
	{
		// connect() success
		agent_stats_inc ( tAgent, eMaxMsecs );
		// send the client's proto version right now to avoid w-w-r pattern.
		NetOutputBuffer_c tOut ( tAgent.m_iSock );
		tOut.SendDword ( SPHINX_CLIENT_VERSION );
		tOut.Flush (); // FIXME! handle flush failure?

		// socket connected, ready to read hello message
		tAgent.m_eState = AGENT_HANDSHAKE;
	}
}

#if HAVE_EPOLL
// copy-pasted version with epoll; plain version below
// process states AGENT_CONNECTING, AGENT_HANDSHAKE, AGENT_ESTABLISHED and notes AGENT_QUERYED
// called in serial order with RemoteConnectToAgents (so, the context is NOT changed during the call).
int RemoteQueryAgents ( AgentConnectionContext_t * pCtx )
{
	assert ( pCtx->m_iTimeout>=0 );
	assert ( pCtx->m_pAgents );
	assert ( pCtx->m_iAgentCount );

	int iAgents = 0;
	int64_t tmMaxTimer = sphMicroTimer() + pCtx->m_iTimeout*1000; // in microseconds

	int eid = epoll_create ( pCtx->m_iAgentCount );
	CSphVector<epoll_event> dEvents ( pCtx->m_iAgentCount );
	epoll_event dEvent;
	int iEvents = 0;
	bool bTimeout = false;

	for ( ;; )
	{
		if ( !iEvents )
		{
			for ( int i=0; i<pCtx->m_iAgentCount; i++ )
			{
				AgentConn_t & tAgent = pCtx->m_pAgents[i];
				// select only 'initial' agents - which are not send query response.
				if ( tAgent.m_eState<AGENT_CONNECTING || tAgent.m_eState>AGENT_QUERYED )
					continue;

				assert ( !tAgent.m_sPath.IsEmpty() || tAgent.m_iPort>0 );
				assert ( tAgent.m_iSock>0 );
				if ( tAgent.m_iSock<=0 || ( tAgent.m_sPath.IsEmpty() && tAgent.m_iPort<=0 ) )
				{
					tAgent.Fail ( eConnectFailures, "invalid agent in querying. Socket %d, Path %s, Port %d", tAgent.m_iSock, tAgent.m_sPath.cstr(), tAgent.m_iPort );
					tAgent.m_eState = AGENT_RETRY; // do retry on connect() failures
					continue;
				}
				dEvent.events = ( tAgent.m_eState==AGENT_CONNECTING || tAgent.m_eState==AGENT_ESTABLISHED ) ? EPOLLOUT : EPOLLIN;
				dEvent.data.ptr = &tAgent;
				epoll_ctl ( eid, EPOLL_CTL_ADD, tAgent.m_iSock, &dEvent );
				++iEvents;
			}
		}

		bool bDone = true;
		for ( int i=0; i<pCtx->m_iAgentCount; i++ )
		{
			AgentConn_t & tAgent = pCtx->m_pAgents[i];
			// select only 'initial' agents - which are not send query response.
			if ( tAgent.m_eState<AGENT_CONNECTING || tAgent.m_eState>AGENT_QUERYED )
				continue;
			if ( tAgent.m_eState!=AGENT_QUERYED )
				bDone = false;
		}
		if ( bDone )
			break;

		// compute timeout
		int64_t tmSelect = sphMicroTimer();
		int64_t tmMicroLeft = tmMaxTimer - tmSelect;
		if ( tmMicroLeft<=0 )
		{
			bTimeout = true;
			break; // FIXME? what about iTimeout==0 case?
		}


		// do poll
		int iSelected = ::epoll_wait ( eid, dEvents.Begin(), dEvents.GetLength(), int( tmMicroLeft/1000 ) );

		// update counters, and loop again if nothing happened
		pCtx->m_pAgents->m_iWaited += sphMicroTimer() - tmSelect;
		// todo: do we need to check for EINTR here? Or the fact of timeout is enough anyway?
		if ( iSelected<=0 )
			continue;

		// ok, something did happen, so loop the agents and do them checks
		for ( int i=0; i<iSelected; ++i )
		{
			AgentConn_t & tAgent = *(AgentConn_t*)dEvents[i].data.ptr;
			bool bReadable = ( dEvents[i].events & EPOLLIN )!=0;
			bool bWriteable = ( dEvents[i].events & EPOLLOUT )!=0;
			bool bErr = ( ( dEvents[i].events & ( EPOLLERR | EPOLLHUP ) )!=0 );

			if ( tAgent.m_eState==AGENT_CONNECTING && ( bWriteable || bErr ) )
			{
				if ( bErr )
				{
					epoll_ctl ( eid, EPOLL_CTL_DEL, tAgent.m_iSock, &dEvent );
					--iEvents;

					int iErr = 0;
					socklen_t iErrLen = sizeof(iErr);
					getsockopt ( tAgent.m_iSock, SOL_SOCKET, SO_ERROR, (char*)&iErr, &iErrLen );
					// connect() failure
					tAgent.Fail ( eConnectFailures, "connect() failed: %s", sphSockError(iErr) );
					continue;
				}

				// connect() success
				agent_stats_inc ( tAgent, eMaxMsecs );

				// send the client's proto version right now to avoid w-w-r pattern.
				NetOutputBuffer_c tOut ( tAgent.m_iSock );
				tOut.SendDword ( SPHINX_CLIENT_VERSION );
				tOut.Flush (); // FIXME! handle flush failure?
				dEvent.events = EPOLLIN;
				dEvent.data.ptr = &tAgent;
				epoll_ctl ( eid, EPOLL_CTL_MOD, tAgent.m_iSock, &dEvent );

				tAgent.m_eState = AGENT_HANDSHAKE;

				continue;
			}

			// check if hello was received
			if ( tAgent.m_eState==AGENT_HANDSHAKE && bReadable )
			{
				// read reply
				int iRemoteVer;
				int iRes = sphSockRecv ( tAgent.m_iSock, (char*)&iRemoteVer, sizeof(iRemoteVer) );
				if ( iRes!=sizeof(iRemoteVer) )
				{
					epoll_ctl ( eid, EPOLL_CTL_DEL, tAgent.m_iSock, &dEvent );
					--iEvents;
					if ( iRes<0 )
					{
						// network error
						int iErr = sphSockGetErrno();
						tAgent.Fail ( eNetworkErrors, "handshake failure (errno=%d, msg=%s)", iErr, sphSockError(iErr) );
					} else if ( iRes>0 )
					{
						// incomplete reply
						tAgent.Fail ( eWrongReplies, "handshake failure (exp=%d, recv=%d)", (int)sizeof(iRemoteVer), iRes );
					} else
					{
						// agent closed the connection
						// this might happen in out-of-sync connect-accept case; so let's retry
						tAgent.Fail ( eUnexpectedClose, "handshake failure (connection was closed)" );
						tAgent.m_eState = AGENT_RETRY;
					}
					continue;
				}

				iRemoteVer = ntohl ( iRemoteVer );
				if (!( iRemoteVer==SPHINX_SEARCHD_PROTO || iRemoteVer==0x01000000UL ) ) // workaround for all the revisions that sent it in host order...
				{
					tAgent.Fail ( eWrongReplies, "handshake failure (unexpected protocol version=%d)", iRemoteVer );
					continue;
				}

				NetOutputBuffer_c tOut ( tAgent.m_iSock );
				// check if we need to reset the persistent connection
				if ( tAgent.m_bFresh && tAgent.m_bPersistent )
				{
					tOut.SendWord ( SEARCHD_COMMAND_PERSIST );
					tOut.SendWord ( 0 ); // dummy version
					tOut.SendInt ( 4 ); // request body length
					tOut.SendInt ( 1 ); // set persistent to 1.
					tOut.Flush ();
					tAgent.m_bFresh = false;
				}

				tAgent.m_eState = AGENT_ESTABLISHED;
				dEvent.events = EPOLLOUT;
				dEvent.data.ptr = &tAgent;
				epoll_ctl ( eid, EPOLL_CTL_MOD, tAgent.m_iSock, &dEvent );
				continue;
			}

			if ( tAgent.m_eState==AGENT_ESTABLISHED && bWriteable )
			{
				// send request
				NetOutputBuffer_c tOut ( tAgent.m_iSock );
				pCtx->m_pBuilder->BuildRequest ( tAgent, tOut );
				tOut.Flush (); // FIXME! handle flush failure?
				tAgent.m_eState = AGENT_QUERYED;
				iAgents++;
				dEvent.events = EPOLLIN;
				dEvent.data.ptr = &tAgent;
				epoll_ctl ( eid, EPOLL_CTL_MOD, tAgent.m_iSock, &dEvent );
				continue;
			}

			// check if queried agent replied while we were querying others
			if ( tAgent.m_eState==AGENT_QUERYED && bReadable )
			{
				// do not account agent wall time from here; agent is probably ready
				tAgent.m_iWall += sphMicroTimer();
				tAgent.m_eState = AGENT_PREREPLY;
				epoll_ctl ( eid, EPOLL_CTL_DEL, tAgent.m_iSock, &dEvent );
				--iEvents;
				continue;
			}
		}
	}
	close ( eid );

	// check if connection timed out
	for ( int i=0; i<pCtx->m_iAgentCount; i++ )
	{
		AgentConn_t & tAgent = pCtx->m_pAgents[i];
		if ( bTimeout && ( tAgent.m_eState!=AGENT_QUERYED && tAgent.m_eState!=AGENT_UNUSED && tAgent.m_eState!=AGENT_RETRY && tAgent.m_eState!=AGENT_PREREPLY
			&& tAgent.m_eState!=AGENT_REPLY ) )
		{
			// technically, we can end up here via two different routes
			// a) connect() never finishes in given time frame
			// b) agent actually accept()s the connection but keeps silence
			// however, there's no way to tell the two from each other
			// so we just account both cases as connect() failure
			tAgent.Fail ( eTimeoutsConnect, "connect() timed out" );
			tAgent.m_eState = AGENT_RETRY; // do retry on connect() failures
		}
	}

	return iAgents;
}

// epoll version. Plain version below
// processing states AGENT_QUERY, AGENT_PREREPLY and AGENT_REPLY
// may work in parallel with RemoteQueryAgents, so the state MAY change duirng a call.
int RemoteWaitForAgents ( CSphVector<AgentConn_t> & dAgents, int iTimeout, IReplyParser_t & tParser )
{
	assert ( iTimeout>=0 );

	int iAgents = 0;
	int64_t tmMaxTimer = sphMicroTimer() + iTimeout*1000; // in microseconds

	int eid = epoll_create ( dAgents.GetLength() );
	CSphVector<epoll_event> dEvents ( dAgents.GetLength() );
	epoll_event dEvent;
	int iEvents = 0;
	bool bTimeout = false;

	for ( ;; )
	{
		if ( !iEvents )
		{
			bool bDone = true;
			ARRAY_FOREACH ( iAgent, dAgents )
			{
				AgentConn_t & tAgent = dAgents[iAgent];
				if ( tAgent.m_bBlackhole )
					continue;

				if ( tAgent.m_eState==AGENT_QUERYED || tAgent.m_eState==AGENT_REPLY || tAgent.m_eState==AGENT_PREREPLY )
				{
					assert ( !tAgent.m_sPath.IsEmpty() || tAgent.m_iPort>0 );
					assert ( tAgent.m_iSock>0 );
					dEvent.events = EPOLLIN;
					dEvent.data.ptr = &tAgent;
					epoll_ctl ( eid, EPOLL_CTL_ADD, tAgent.m_iSock, &dEvent );
					++iEvents;
					bDone = false;
				}
			}

			if ( bDone )
				break;
		}



		int64_t tmSelect = sphMicroTimer();
		int64_t tmMicroLeft = tmMaxTimer - tmSelect;
		if ( tmMicroLeft<=0 ) // FIXME? what about iTimeout==0 case?
		{
			bTimeout = true;
			break;
		}

		int iSelected = ::epoll_wait ( eid, dEvents.Begin(), dEvents.GetLength(), int( tmMicroLeft/1000 ) );
		dAgents.Begin()->m_iWaited += sphMicroTimer() - tmSelect;

		if ( iSelected<=0 )
			continue;

		for ( int i=0; i<iSelected; ++i )
		{
			AgentConn_t & tAgent = *(AgentConn_t*)dEvents[i].data.ptr;
			if ( tAgent.m_bBlackhole )
				continue;
			if (!( tAgent.m_eState==AGENT_QUERYED || tAgent.m_eState==AGENT_REPLY || tAgent.m_eState==AGENT_PREREPLY ))
				continue;

			if (!( dEvents[i].events & EPOLLIN ))
				continue;

			// if there was no reply yet, read reply header
			bool bFailure = true;
			bool bWarnings = false;

			for ( ;; )
			{
				if ( tAgent.m_eState==AGENT_QUERYED || tAgent.m_eState==AGENT_PREREPLY )
				{
					if ( tAgent.m_eState==AGENT_PREREPLY )
					{
						tAgent.m_iWall -= sphMicroTimer();
						tAgent.m_eState = AGENT_QUERYED;
					}

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
						agent_stats_inc ( tAgent, eNetworkErrors );
						break;
					}

					tReplyHeader.m_iStatus = ntohs ( tReplyHeader.m_iStatus );
					tReplyHeader.m_iVer = ntohs ( tReplyHeader.m_iVer );
					tReplyHeader.m_iLength = ntohl ( tReplyHeader.m_iLength );

					// check the packet
					if ( tReplyHeader.m_iLength<0 || tReplyHeader.m_iLength>g_iMaxPacketSize ) // FIXME! add reasonable max packet len too
					{
						tAgent.m_sFailure.SetSprintf ( "invalid packet size (status=%d, len=%d, max_packet_size=%d)",
							tReplyHeader.m_iStatus, tReplyHeader.m_iLength, g_iMaxPacketSize );
						agent_stats_inc ( tAgent, eWrongReplies );
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
						agent_stats_inc ( tAgent, eNetworkErrors );
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
						bWarnings = true;

					} else if ( tAgent.m_iReplyStatus==SEARCHD_RETRY )
					{
						tAgent.m_eState = AGENT_RETRY;
						CSphString sAgentError = tReq.GetString ();
						tAgent.m_sFailure.SetSprintf ( "remote warning: %s", sAgentError.cstr() );
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
						agent_stats_inc ( tAgent, eWrongReplies );
						break;
					}

					epoll_ctl ( eid, EPOLL_CTL_DEL, tAgent.m_iSock, &dEvent );
					--iEvents;
					// all is well
					iAgents++;
					tAgent.Close ( false );
					tAgent.m_bSuccess = true;
				}

				bFailure = false;
				break;
			}

			if ( bFailure )
			{
				epoll_ctl ( eid, EPOLL_CTL_DEL, tAgent.m_iSock, &dEvent );
				--iEvents;
				tAgent.Close ();
				tAgent.m_dResults.Reset ();
			} else if ( tAgent.m_bSuccess )
			{
				ARRAY_FOREACH_COND ( i, tAgent.m_dResults, !bWarnings )
					bWarnings = !tAgent.m_dResults[i].m_sWarning.IsEmpty();
				agent_stats_inc ( tAgent, bWarnings ? eWarnings : eNoErrors );
			}
		}
	}

	close ( eid );

	// close timed-out agents
	ARRAY_FOREACH ( iAgent, dAgents )
	{
		AgentConn_t & tAgent = dAgents[iAgent];
		if ( tAgent.m_bBlackhole )
			tAgent.Close ();
		else if ( bTimeout && ( tAgent.m_eState==AGENT_QUERYED || tAgent.m_eState==AGENT_PREREPLY ) )
		{
			assert ( !tAgent.m_dResults.GetLength() );
			assert ( !tAgent.m_bSuccess );
			tAgent.Fail ( eTimeoutsQuery, "query timed out" );
		}
	}

	return iAgents;
}

#else // !HAVE_EPOLL

// process states AGENT_CONNECTING, AGENT_HANDSHAKE, AGENT_ESTABLISHED and notes AGENT_QUERYED
// called in serial order with RemoteConnectToAgents (so, the context is NOT changed during the call).
int RemoteQueryAgents ( AgentConnectionContext_t * pCtx )
{
	assert ( pCtx->m_iTimeout>=0 );
	assert ( pCtx->m_pAgents );
	assert ( pCtx->m_iAgentCount );

	int iAgents = 0;
	int64_t tmMaxTimer = sphMicroTimer() + pCtx->m_iTimeout*1000; // in microseconds
	CSphVector<int> dWorkingSet;
	dWorkingSet.Reserve ( pCtx->m_iAgentCount );

#if HAVE_POLL
	CSphVector<struct pollfd> fds;
	fds.Reserve ( pCtx->m_iAgentCount );
#endif

	// main connection loop
	// break if a) all connects in AGENT_QUERY state, or b) timeout
	for ( ;; )
	{
		// prepare socket sets
		dWorkingSet.Reset();
#if HAVE_POLL
		fds.Reset();
#else
		int iMax = 0;
		fd_set fdsRead, fdsWrite;
		FD_ZERO ( &fdsRead );
		FD_ZERO ( &fdsWrite );
#endif

		bool bDone = true;
		for ( int i=0; i<pCtx->m_iAgentCount; i++ )
		{
			AgentConn_t & tAgent = pCtx->m_pAgents[i];
			// select only 'initial' agents - which are not send query response.
			if ( tAgent.m_eState<AGENT_CONNECTING || tAgent.m_eState>AGENT_QUERYED )
				continue;

			assert ( !tAgent.m_sPath.IsEmpty() || tAgent.m_iPort>0 );
			assert ( tAgent.m_iSock>0 );
			if ( tAgent.m_iSock<=0 || ( tAgent.m_sPath.IsEmpty() && tAgent.m_iPort<=0 ) )
			{
				tAgent.Fail ( eConnectFailures, "invalid agent in querying. Socket %d, Path %s, Port %d",
					tAgent.m_iSock, tAgent.m_sPath.cstr(), tAgent.m_iPort );
				tAgent.m_eState = AGENT_RETRY; // do retry on connect() failures
				continue;
			}

			bool bWr = ( tAgent.m_eState==AGENT_CONNECTING || tAgent.m_eState==AGENT_ESTABLISHED );
			dWorkingSet.Add(i);
#if HAVE_POLL
			pollfd& pfd = fds.Add();
			pfd.fd = tAgent.m_iSock;
			pfd.events = bWr ? POLLOUT : POLLIN;
#else
			sphFDSet ( tAgent.m_iSock, bWr ? &fdsWrite : &fdsRead );
			iMax = Max ( iMax, tAgent.m_iSock );
#endif
			if ( tAgent.m_eState!=AGENT_QUERYED )
				bDone = false;
		}

		if ( bDone )
			break;

		// compute timeout
		int64_t tmSelect = sphMicroTimer();
		int64_t tmMicroLeft = tmMaxTimer - tmSelect;
		if ( tmMicroLeft<=0 )
			break; // FIXME? what about iTimeout==0 case?

		// do poll
#if HAVE_POLL
		int iSelected = ::poll ( fds.Begin(), fds.GetLength(), int( tmMicroLeft/1000 ) );
#else
		struct timeval tvTimeout;
		tvTimeout.tv_sec = (int)( tmMicroLeft/ 1000000 ); // full seconds
		tvTimeout.tv_usec = (int)( tmMicroLeft % 1000000 ); // microseconds
		int iSelected = ::select ( 1+iMax, &fdsRead, &fdsWrite, NULL, &tvTimeout ); // exceptfds are OOB only
#endif

		// update counters, and loop again if nothing happened
		pCtx->m_pAgents->m_iWaited += sphMicroTimer() - tmSelect;
		// todo: do we need to check for EINTR here? Or the fact of timeout is enough anyway?
		if ( iSelected<=0 )
			continue;

		// ok, something did happen, so loop the agents and do them checks
		ARRAY_FOREACH ( i, dWorkingSet )
		{
			AgentConn_t & tAgent = pCtx->m_pAgents[dWorkingSet[i]];

#if HAVE_POLL
			bool bReadable = ( fds[i].revents & POLLIN )!=0;
			bool bWriteable = ( fds[i].revents & POLLOUT )!=0;
			bool bErr = ( ( fds[i].revents & ( POLLERR | POLLHUP ) )!=0 );
#else
			bool bReadable = FD_ISSET ( tAgent.m_iSock, &fdsRead )!=0;
			bool bWriteable = FD_ISSET ( tAgent.m_iSock, &fdsWrite )!=0;
			bool bErr = !bWriteable; // just poll and check for error
#endif

			if ( tAgent.m_eState==AGENT_CONNECTING && ( bWriteable || bErr ) )
			{
				if ( bErr )
				{
					// check if connection completed
					// tricky part, with select, we MUST use write-set ONLY here at this check
					// even though we can't tell connect() success from just OS send buffer availability
					// but any check involving read-set just never ever completes, so...
					int iErr = 0;
					socklen_t iErrLen = sizeof(iErr);
					getsockopt ( tAgent.m_iSock, SOL_SOCKET, SO_ERROR, (char*)&iErr, &iErrLen );
					if ( iErr )
					{
						// connect() failure
						tAgent.Fail ( eConnectFailures, "connect() failed: %s", sphSockError(iErr) );
					}
					continue;
				}

				assert ( bWriteable ); // should never get empty or readable state
				// connect() success
				agent_stats_inc ( tAgent, eMaxMsecs );

				// send the client's proto version right now to avoid w-w-r pattern.
				NetOutputBuffer_c tOut ( tAgent.m_iSock );
				tOut.SendDword ( SPHINX_CLIENT_VERSION );
				tOut.Flush (); // FIXME! handle flush failure?

				tAgent.m_eState = AGENT_HANDSHAKE;
				continue;
			}

			// check if hello was received
			if ( tAgent.m_eState==AGENT_HANDSHAKE && bReadable )
			{
				// read reply
				int iRemoteVer;
				int iRes = sphSockRecv ( tAgent.m_iSock, (char*)&iRemoteVer, sizeof(iRemoteVer) );
				if ( iRes!=sizeof(iRemoteVer) )
				{
					if ( iRes<0 )
					{
						int iErr = sphSockGetErrno();
						tAgent.Fail ( eNetworkErrors, "handshake failure (errno=%d, msg=%s)", iErr, sphSockError(iErr) );
					} else if ( iRes>0 )
					{
						// incomplete reply
						tAgent.Fail ( eWrongReplies, "handshake failure (exp=%d, recv=%d)", (int)sizeof(iRemoteVer), iRes );
					} else
					{
						// agent closed the connection
						// this might happen in out-of-sync connect-accept case; so let's retry
						tAgent.Fail ( eUnexpectedClose, "handshake failure (connection was closed)" );
						tAgent.m_eState = AGENT_RETRY;
					}
					continue;
				}

				iRemoteVer = ntohl ( iRemoteVer );
				if (!( iRemoteVer==SPHINX_SEARCHD_PROTO || iRemoteVer==0x01000000UL ) ) // workaround for all the revisions that sent it in host order...
				{
					tAgent.Fail ( eWrongReplies, "handshake failure (unexpected protocol version=%d)", iRemoteVer );
					continue;
				}

				NetOutputBuffer_c tOut ( tAgent.m_iSock );
				// check if we need to reset the persistent connection
				if ( tAgent.m_bFresh && tAgent.m_bPersistent )
				{
					tOut.SendWord ( SEARCHD_COMMAND_PERSIST );
					tOut.SendWord ( 0 ); // dummy version
					tOut.SendInt ( 4 ); // request body length
					tOut.SendInt ( 1 ); // set persistent to 1.
					tOut.Flush ();
					tAgent.m_bFresh = false;
				}

				tAgent.m_eState = AGENT_ESTABLISHED;
				continue;
			}

			if ( tAgent.m_eState==AGENT_ESTABLISHED && bWriteable )
			{
				// send request
				NetOutputBuffer_c tOut ( tAgent.m_iSock );
				pCtx->m_pBuilder->BuildRequest ( tAgent, tOut );
				tOut.Flush (); // FIXME! handle flush failure?
				tAgent.m_eState = AGENT_QUERYED;
				iAgents++;
				continue;
			}

			// check if queried agent replied while we were querying others
			if ( tAgent.m_eState==AGENT_QUERYED && bReadable )
			{
				// do not account agent wall time from here; agent is probably ready
				tAgent.m_iWall += sphMicroTimer();
				tAgent.m_eState = AGENT_PREREPLY;
				continue;
			}
		}
	}


	// check if connection timed out
	for ( int i=0; i<pCtx->m_iAgentCount; i++ )
	{
		AgentConn_t & tAgent = pCtx->m_pAgents[i];
		if ( tAgent.m_eState!=AGENT_QUERYED && tAgent.m_eState!=AGENT_UNUSED && tAgent.m_eState!=AGENT_RETRY && tAgent.m_eState!=AGENT_PREREPLY
			&& tAgent.m_eState!=AGENT_REPLY )
		{
			// technically, we can end up here via two different routes
			// a) connect() never finishes in given time frame
			// b) agent actually accept()s the connection but keeps silence
			// however, there's no way to tell the two from each other
			// so we just account both cases as connect() failure
			tAgent.Fail ( eTimeoutsConnect, "connect() timed out" );
			tAgent.m_eState = AGENT_RETRY; // do retry on connect() failures
		}
	}

	return iAgents;
}

// processing states AGENT_QUERY, AGENT_PREREPLY and AGENT_REPLY
int RemoteWaitForAgents ( CSphVector<AgentConn_t> & dAgents, int iTimeout, IReplyParser_t & tParser )
{
	assert ( iTimeout>=0 );

	int iAgents = 0;
	int64_t tmMaxTimer = sphMicroTimer() + iTimeout*1000; // in microseconds

	CSphVector<int> dWorkingSet;
	dWorkingSet.Reserve ( dAgents.GetLength() );

#if HAVE_POLL
	CSphVector<struct pollfd> fds;
	fds.Reserve ( dAgents.GetLength() );
#endif

	for ( ;; )
	{
		dWorkingSet.Reset();

#if HAVE_POLL
		fds.Reset();
#else
		int iMax = 0;
		fd_set fdsRead;
		FD_ZERO ( &fdsRead );
#endif
		bool bDone = true;
		ARRAY_FOREACH ( iAgent, dAgents )
		{
			AgentConn_t & tAgent = dAgents[iAgent];
			if ( tAgent.m_bBlackhole )
				continue;

			if ( tAgent.m_eState==AGENT_QUERYED || tAgent.m_eState==AGENT_REPLY || tAgent.m_eState==AGENT_PREREPLY )
			{
				assert ( !tAgent.m_sPath.IsEmpty() || tAgent.m_iPort>0 );
				assert ( tAgent.m_iSock>0 );
				dWorkingSet.Add(iAgent);
#if HAVE_POLL
				pollfd & pfd = fds.Add();
				pfd.fd = tAgent.m_iSock;
				pfd.events = POLLIN;
#else
				sphFDSet ( tAgent.m_iSock, &fdsRead );
				iMax = Max ( iMax, tAgent.m_iSock );
#endif
				bDone = false;
			}
		}

		if ( bDone )
			break;

		int64_t tmSelect = sphMicroTimer();
		int64_t tmMicroLeft = tmMaxTimer - tmSelect;
		if ( tmMicroLeft<=0 ) // FIXME? what about iTimeout==0 case?
			break;

#if HAVE_POLL
		int iSelected = ::poll ( fds.Begin(), fds.GetLength(), int( tmMicroLeft/1000 ) );
#else
		struct timeval tvTimeout;
		tvTimeout.tv_sec = (int)( tmMicroLeft / 1000000 ); // full seconds
		tvTimeout.tv_usec = (int)( tmMicroLeft % 1000000 ); // microseconds
		int iSelected = ::select ( 1+iMax, &fdsRead, NULL, NULL, &tvTimeout );
#endif

		dAgents.Begin()->m_iWaited += sphMicroTimer() - tmSelect;

		if ( iSelected<=0 )
			continue;

		ARRAY_FOREACH ( i, dWorkingSet )
		{
			AgentConn_t & tAgent = dAgents[dWorkingSet[i]];
			if ( tAgent.m_bBlackhole )
				continue;
			if (!( tAgent.m_eState==AGENT_QUERYED || tAgent.m_eState==AGENT_REPLY || tAgent.m_eState==AGENT_PREREPLY ))
				continue;

#if HAVE_POLL
			if (!( fds[i].revents & POLLIN ))
#else
			if ( !FD_ISSET ( tAgent.m_iSock, &fdsRead ) )
#endif
				continue;

			// if there was no reply yet, read reply header
			bool bFailure = true;
			bool bWarnings = false;
			for ( ;; )
			{
				if ( tAgent.m_eState==AGENT_QUERYED || tAgent.m_eState==AGENT_PREREPLY )
				{
					if ( tAgent.m_eState==AGENT_PREREPLY )
					{
						tAgent.m_iWall -= sphMicroTimer();
						tAgent.m_eState = AGENT_QUERYED;
					}

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
						agent_stats_inc ( tAgent, eNetworkErrors );
						break;
					}

					tReplyHeader.m_iStatus = ntohs ( tReplyHeader.m_iStatus );
					tReplyHeader.m_iVer = ntohs ( tReplyHeader.m_iVer );
					tReplyHeader.m_iLength = ntohl ( tReplyHeader.m_iLength );

					// check the packet
					if ( tReplyHeader.m_iLength<0 || tReplyHeader.m_iLength>g_iMaxPacketSize ) // FIXME! add reasonable max packet len too
					{
						tAgent.m_sFailure.SetSprintf ( "invalid packet size (status=%d, len=%d, max_packet_size=%d)",
							tReplyHeader.m_iStatus, tReplyHeader.m_iLength, g_iMaxPacketSize );
						agent_stats_inc ( tAgent, eWrongReplies );
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
						agent_stats_inc ( tAgent, eNetworkErrors );
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
						bWarnings = true;

					} else if ( tAgent.m_iReplyStatus==SEARCHD_RETRY )
					{
						tAgent.m_eState = AGENT_RETRY;
						CSphString sAgentError = tReq.GetString ();
						tAgent.m_sFailure.SetSprintf ( "remote warning: %s", sAgentError.cstr() );
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
						agent_stats_inc ( tAgent, eWrongReplies );
						break;
					}

					// all is well
					iAgents++;
					tAgent.Close ( false );
					tAgent.m_bSuccess = true;
				}

				bFailure = false;
				break;
			}

			if ( bFailure )
			{
				tAgent.Close ();
				tAgent.m_dResults.Reset ();
			} else if ( tAgent.m_bSuccess )
			{
				ARRAY_FOREACH_COND ( i, tAgent.m_dResults, !bWarnings )
					bWarnings = !tAgent.m_dResults[i].m_sWarning.IsEmpty();
				agent_stats_inc ( tAgent, bWarnings ? eWarnings : eNoErrors );
			}
		}
	}

	// close timed-out agents
	ARRAY_FOREACH ( iAgent, dAgents )
	{
		AgentConn_t & tAgent = dAgents[iAgent];
		if ( tAgent.m_bBlackhole )
			tAgent.Close ();
		else if ( tAgent.m_eState==AGENT_QUERYED || tAgent.m_eState==AGENT_PREREPLY )
		{
			assert ( !tAgent.m_dResults.GetLength() );
			assert ( !tAgent.m_bSuccess );
			tAgent.Fail ( eTimeoutsQuery, "query timed out" );
		}
	}

	return iAgents;
}

#endif // HAVE_EPOLL


struct AgentWorkContext_t;
typedef void ( *ThdWorker_fn ) ( AgentWorkContext_t * );


struct AgentWorkContext_t : public AgentConnectionContext_t
{
	ThdWorker_fn	m_pfn;			///< work functor & flag of dummy element
	int64_t			m_tmWait;
	int				m_iRetries;
	int				m_iAgentsDone;

	AgentWorkContext_t ()
		: m_pfn ( NULL )
		, m_tmWait ( 0 )
		, m_iRetries ( 0 )
		, m_iAgentsDone ( 0 )
	{}
};


class ThdWorkPool_c : ISphNoncopyable
{
private:
	CSphMutex m_tDataLock;
	CSphMutex m_tStatLock;
public:
	CSphAutoEvent m_tChanged;
private:
	AgentWorkContext_t * m_dData;	// works array
	int m_iLen;

	int m_iHead;					// ring buffer begin
	int m_iTail;					// ring buffer end

	volatile int m_iWorksCount;			// count of works to be done
	volatile int m_iAgentsDone;				// count of agents that finished their works
	volatile int m_iAgentsReported;			// count of agents that reported of their work done

	CrashQuery_t m_tCrashQuery;		// query that got reported on crash

public:
	explicit ThdWorkPool_c ( int iLen );
	~ThdWorkPool_c ();


	AgentWorkContext_t Pop();

	void Push ( const AgentWorkContext_t & tElem );
	void RawPush ( const AgentWorkContext_t & tElem );
	int GetReadyCount ();

	int GetReadyTotal () const
	{
		return m_iAgentsDone;
	}

	bool HasIncompleteWorks () const
	{
		return ( m_iWorksCount>0 );
	}

	void SetWorksCount ( int iWorkers )
	{
		m_iWorksCount = iWorkers;
	}

	static void PoolThreadFunc ( void * pArg );
};

ThdWorkPool_c::ThdWorkPool_c ( int iLen )
{
	m_tCrashQuery = SphCrashLogger_c::GetQuery(); // transfer query info for crash logger to new thread

	m_iLen = iLen+1;
	m_iTail = m_iHead = 0;
	m_iWorksCount = 0;
	m_iAgentsDone = m_iAgentsReported = 0;

	m_dData = new AgentWorkContext_t[m_iLen];
#ifndef NDEBUG
	for ( int i=0; i<m_iLen; i++ ) // to make sure that we don't rewrite valid elements
		m_dData[i] = AgentWorkContext_t();
#endif

	m_tChanged.Init ( &m_tStatLock );
}

ThdWorkPool_c::~ThdWorkPool_c ()
{
	m_tChanged.Done();
	SafeDeleteArray ( m_dData );
}

AgentWorkContext_t ThdWorkPool_c::Pop()
{
	AgentWorkContext_t tRes;
	if ( m_iTail==m_iHead ) // quick path for empty pool
		return tRes;

	CSphScopedLock<CSphMutex> tData ( m_tDataLock ); // lock on create, unlock on destroy

	if ( m_iTail==m_iHead ) // it might be empty now as another thread could steal work till that moment
		return tRes;

	tRes = m_dData[m_iHead];
	assert ( tRes.m_pfn );
#ifndef NDEBUG
	m_dData[m_iHead] = AgentWorkContext_t(); // to make sure that we don't rewrite valid elements
#endif
	m_iHead = ( m_iHead+1 ) % m_iLen;

	return tRes;
}

void ThdWorkPool_c::Push ( const AgentWorkContext_t & tElem )
{
	if ( !tElem.m_pfn )
		return;

	m_tDataLock.Lock();
	RawPush ( tElem );
	m_tDataLock.Unlock();
}

void ThdWorkPool_c::RawPush ( const AgentWorkContext_t & tElem )
{
	assert ( !m_dData[m_iTail].m_pfn ); // to make sure that we don't rewrite valid elements
	m_dData[m_iTail] = tElem;
	m_iTail = ( m_iTail+1 ) % m_iLen;
}

int ThdWorkPool_c::GetReadyCount ()
{
	// it could be better to lock here to get accurate value of m_iAgentsDone
	// however that make lock contention of 1 sec on 1000 query ~ total 3.2sec vs 2.2 sec ( trunk )
	int iAgentsDone = m_iAgentsDone;
	int iNowDone = iAgentsDone - m_iAgentsReported;
	m_iAgentsReported = iAgentsDone;
	return iNowDone;
}

void ThdWorkPool_c::PoolThreadFunc ( void * pArg )
{
	ThdWorkPool_c * pPool = (ThdWorkPool_c *)pArg;

	SphCrashLogger_c::SetLastQuery ( pPool->m_tCrashQuery );

	int iSpinCount = 0;
	int iPopCount = 0;
	AgentWorkContext_t tNext;
	for ( ;; )
	{
		if ( !tNext.m_pfn ) // pop new work if current is done
		{
			iSpinCount = 0;
			++iPopCount;
			tNext = pPool->Pop();
			if ( !tNext.m_pfn ) // if there is no work at queue - worker done
				break;
		}

		tNext.m_pfn ( &tNext );
		if ( tNext.m_iAgentsDone || !tNext.m_pfn )
		{
			CSphScopedLock<CSphMutex> tStat ( pPool->m_tStatLock );
			pPool->m_iAgentsDone += tNext.m_iAgentsDone;
			pPool->m_iWorksCount -= ( tNext.m_pfn==NULL );
			pPool->m_tChanged.SetEvent();
		}

		iSpinCount++;
		if ( iSpinCount>1 && iSpinCount<4 ) // it could be better not to do the same work
		{
			pPool->Push ( tNext );
			tNext = AgentWorkContext_t();
		} else if ( pPool->m_iWorksCount>1 && iPopCount>pPool->m_iWorksCount ) // should sleep on queue wrap
		{
			iPopCount = 0;
			sphSleepMsec ( 1 );
		}
	}
}

void ThdWorkParallel ( AgentWorkContext_t * );
void ThdWorkWait ( AgentWorkContext_t * pCtx )
{
	pCtx->m_pfn = ( pCtx->m_tmWait<sphMicroTimer() ) ? ThdWorkWait : ThdWorkParallel;
}

void SetNextRetry ( AgentWorkContext_t * pCtx )
{
	pCtx->m_pfn = NULL;
	pCtx->m_pAgents->m_eState = AGENT_UNUSED;

	if ( !pCtx->m_iRetriesMax || !pCtx->m_iDelay || pCtx->m_iRetries>pCtx->m_iRetriesMax )
		return;

	int64_t tmNextTry = sphMicroTimer() + pCtx->m_iDelay*1000;
	pCtx->m_pfn = ThdWorkWait;
	pCtx->m_iRetries++;
	pCtx->m_tmWait = tmNextTry;
	pCtx->m_pAgents->m_eState = AGENT_RETRY;
}

void ThdWorkParallel ( AgentWorkContext_t * pCtx )
{
	RemoteConnectToAgent ( *pCtx->m_pAgents );
	if ( pCtx->m_pAgents->m_eState==AGENT_UNUSED )
	{
		SetNextRetry ( pCtx );
		return;
	}

	RemoteQueryAgents ( pCtx );
	if ( pCtx->m_pAgents->m_eState==AGENT_RETRY ) // next round of connect try
	{
		SetNextRetry ( pCtx );
	} else
	{
		pCtx->m_pfn = NULL;
		pCtx->m_iAgentsDone = 1;
	}
}


void ThdWorkSequental ( AgentWorkContext_t * pCtx )
{
	if ( pCtx->m_iRetries )
		sphSleepMsec ( pCtx->m_iDelay );

	for ( int iAgent=0; iAgent<pCtx->m_iAgentCount; iAgent++ )
	{
		AgentConn_t & tAgent = pCtx->m_pAgents[iAgent];
		if ( !pCtx->m_iRetries || tAgent.m_eState==AGENT_RETRY )
		{
			RemoteConnectToAgent ( tAgent );
		}
	}

	pCtx->m_iAgentsDone += RemoteQueryAgents ( pCtx );

	int iToRetry = 0;
	for ( int i=0; i<pCtx->m_iAgentCount; i++ )
		iToRetry += ( pCtx->m_pAgents[i].m_eState==AGENT_RETRY );

	if ( iToRetry )
		pCtx->m_iRetries++;

	pCtx->m_pfn = NULL;
	if ( iToRetry && pCtx->m_iRetriesMax && pCtx->m_iRetries<=pCtx->m_iRetriesMax )
		pCtx->m_pfn = ThdWorkSequental;
}

class CSphRemoteAgentsController : public ISphRemoteAgentsController
{
public:
	CSphRemoteAgentsController ( int iThreads, CSphVector<AgentConn_t> & dAgents,
		const IRequestBuilder_t & tBuilder, int iTimeout, int iRetryMax=0, int iDelay=0 );

	virtual ~CSphRemoteAgentsController ();


	// check that there are no works to do
	virtual bool IsDone () const
	{
		return m_tWorkerPool.HasIncompleteWorks()==0;
	}

	// block execution while there are works to do
	virtual int Finish ();

	// check that some agents are done at this iteration
	virtual bool HasReadyAgents ()
	{
		return ( m_tWorkerPool.GetReadyCount()>0 );
	}

	virtual void WaitAgentsEvent ();

private:
	ThdWorkPool_c m_tWorkerPool;
	CSphVector<SphThread_t> m_dThds;
};

CSphRemoteAgentsController::CSphRemoteAgentsController ( int iThreads,
														CSphVector<AgentConn_t> & dAgents, const IRequestBuilder_t & tBuilder,
														int iTimeout, int iRetryMax, int iDelay )
														: m_tWorkerPool ( dAgents.GetLength() )
{
	assert ( dAgents.GetLength() );

	iThreads = Max ( 1, Min ( iThreads, dAgents.GetLength() ) );
	m_dThds.Resize ( iThreads );

	AgentWorkContext_t tCtx;
	tCtx.m_pBuilder = &tBuilder;
	tCtx.m_iAgentCount = 1;
	tCtx.m_pfn = ThdWorkParallel;
	tCtx.m_iDelay = iDelay;
	tCtx.m_iRetriesMax = iRetryMax;
	tCtx.m_iTimeout = iTimeout;


	if ( iThreads>1 )
	{
		m_tWorkerPool.SetWorksCount ( dAgents.GetLength() );
		ARRAY_FOREACH ( i, dAgents )
		{
			tCtx.m_pAgents = dAgents.Begin()+i;
			m_tWorkerPool.RawPush ( tCtx );
		}
	} else
	{
		m_tWorkerPool.SetWorksCount ( 1 );
		tCtx.m_pAgents = dAgents.Begin();
		tCtx.m_iAgentCount = dAgents.GetLength();
		tCtx.m_pfn = ThdWorkSequental;
		m_tWorkerPool.RawPush ( tCtx );
	}

	ARRAY_FOREACH ( i, m_dThds )
		SphCrashLogger_c::ThreadCreate ( m_dThds.Begin()+i, ThdWorkPool_c::PoolThreadFunc, &m_tWorkerPool );
}

CSphRemoteAgentsController::~CSphRemoteAgentsController ()
{
	ARRAY_FOREACH ( i, m_dThds )
		sphThreadJoin ( m_dThds.Begin()+i );

	m_dThds.Resize ( 0 );
}

// block execution while there are works to do
int CSphRemoteAgentsController::Finish ()
{
	while ( !IsDone() )
		WaitAgentsEvent();

	return m_tWorkerPool.GetReadyTotal();
}

void CSphRemoteAgentsController::WaitAgentsEvent ()
{
	m_tWorkerPool.m_tChanged.WaitEvent();
}

ISphRemoteAgentsController* GetAgentsController ( int iThreads, CSphVector<AgentConn_t> & dAgents,
		const IRequestBuilder_t & tBuilder, int iTimeout, int iRetryMax, int iDelay )
{
	return new CSphRemoteAgentsController ( iThreads, dAgents, tBuilder, iTimeout, iRetryMax, iDelay );
}


