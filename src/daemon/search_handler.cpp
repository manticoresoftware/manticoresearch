//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "search_handler.h"

#include "sphinxdefs.h"
#include "joinsorter.h"
#include "pseudosharding.h"
#include "api_search.h"
#include "logger.h"
#include "schematransform.h"
#include "minimize_aggr_result.h"

#include "std/string.h"

static bool LOG_LEVEL_LOCAL_SEARCH = val_from_env ( "MANTICORE_LOG_LOCAL_SEARCH", false ); // verbose logging local search events, ruled by this env variable
#define LOG_COMPONENT_LOCSEARCHINFO __LINE__ << " "
#define LOCSEARCHINFO LOGINFO ( LOCAL_SEARCH, LOCSEARCHINFO )

extern int g_iQueryLogMinMs; // log 'slow' threshold for query, defined in searchd.cpp

using namespace Threads;

/////////////////////////////////////////////////////////////////////////////
SearchHandler_c::SearchHandler_c ( int iQueries ) noexcept
	: m_dQueries { 0 }
	, m_dJoinQueryOptions { iQueries }
	, m_dAggrResults { iQueries }
	, m_dFailuresSet { iQueries }
	, m_dAgentTimes { iQueries }
	, m_dTables { iQueries }
	, m_dResults { iQueries }
{
	for ( int i=0; i<iQueries; ++i )
		m_dResults[i].m_pMeta = &m_dAggrResults[i];

	// initial slices (when nothing explicitly asked)
	m_dNJoinQueryOptions = m_dJoinQueryOptions;
	m_dNAggrResults = m_dAggrResults;
	m_dNResults = m_dResults;
	m_dNFailuresSet = m_dFailuresSet;
}


SearchHandler_c::SearchHandler_c ( int iQueries, std::unique_ptr<QueryParser_i> pQueryParser, QueryType_e eQueryType, bool bMaster ) noexcept
	: SearchHandler_c { iQueries }
{
	m_bMaster = bMaster;
	m_eQueryType = eQueryType;
	m_pQueryParser = std::move ( pQueryParser );
	m_dQueries.Reset ( iQueries );
	m_dNQueries = m_dQueries;

	for ( auto & dQuery: m_dQueries )
	{
		dQuery.m_eQueryType = eQueryType;
		dQuery.m_pQueryParser = m_pQueryParser.get();
	}
}

SearchHandler_c::SearchHandler_c ( CSphFixedVector<CSphQuery> dQueries, bool bMaster ) noexcept
	: SearchHandler_c { dQueries.GetLength() }
{
	m_bMaster = bMaster;
	m_eQueryType = dQueries.First().m_eQueryType;
	m_pQueryParser = CreateQueryParser (m_eQueryType == QUERY_JSON);

	for ( auto & dQuery: dQueries )
	{
		assert ( dQuery.m_eQueryType==m_eQueryType ); // we assume that all incoming queries have the same type
		dQuery.m_pQueryParser = m_pQueryParser.get();
	}
	m_dQueries = std::move ( dQueries );
	m_dNQueries = m_dQueries;
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
	CSphFixedVector<CSphQuery> m_dQueries {0}; // given queries I'll finally remove
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
	void EngageRetiring ( CSphFixedVector<CSphQuery> dQueries, CSphVector<int> dRetired )
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
	auto dPointed = hazard::GetListOfPointed ( m_dQueries );
	if ( !dPointed.IsEmpty () )
	{
		// pQueryHolder will be self-removed when all used queries retired
		auto pQueryHolder = new RetireQueriesVec_c;
		pQueryHolder->EngageRetiring ( std::move ( m_dQueries ), std::move ( dPointed ) );
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
	g_tStats.m_iQueryCpuTime.fetch_add ( tmCPU, std::memory_order_relaxed );
	g_tStats.m_iDiskReads.fetch_add ( tIO.m_iReadOps, std::memory_order_relaxed );
	g_tStats.m_iDiskReadTime.fetch_add ( tIO.m_iReadTime, std::memory_order_relaxed );
	g_tStats.m_iDiskReadBytes.fetch_add ( tIO.m_iReadBytes, std::memory_order_relaxed );

	if ( m_bQueryLog )
		LogQuery ( m_dQueries[0], m_dJoinQueryOptions[0], m_dAggrResults[0], m_dAgentTimes[0] );
}

void SearchHandler_c::SetQuery ( int iQuery, const CSphQuery & tQuery, std::unique_ptr<ISphTableFunc> pTableFunc )
{
	m_dQueries[iQuery] = tQuery;
	m_dQueries[iQuery].m_pQueryParser = m_pQueryParser.get();
	m_dQueries[iQuery].m_eQueryType = m_eQueryType;
	m_dTables[iQuery] = std::move ( pTableFunc );
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
	if ( !m_bQueryLog )
		return;

	ARRAY_FOREACH ( i, m_dQueries )
		LogQuery ( m_dQueries[i], m_dJoinQueryOptions[i], m_dAggrResults[i], m_dAgentTimes[i] );
	// no need to call OnRunFinished() as meta.matches already calculated at search
}


// final fixup
void SearchHandler_c::OnRunFinished()
{
	for ( auto & tResult : m_dAggrResults )
		tResult.m_iMatches = tResult.GetLength();
}

// return sequence of columns as 'show create table', or 'describe' reveal
static StrVec_t GetDefaultSchema ( const CSphIndex* pIndex )
{
	StrVec_t dRes;
	auto& tSchema = pIndex->GetMatchSchema();
	if ( tSchema.GetAttrsCount()==0 )
		return dRes;
	assert ( tSchema.GetAttr ( 0 ).m_sName == sphGetDocidName() );
	const auto& tId = tSchema.GetAttr ( 0 );
	dRes.Add ( tId.m_sName );

	for ( int i = 0; i < tSchema.GetFieldsCount(); ++i )
	{
		const auto& tField = tSchema.GetField ( i );
		dRes.Add ( tField.m_sName );
	}

	for ( int i = 1; i < tSchema.GetAttrsCount(); ++i ) // from 1, as 0 is docID and already emerged
	{
		const auto& tAttr = tSchema.GetAttr ( i );
		if ( sphIsInternalAttr ( tAttr ) )
			continue;

		if ( tSchema.GetField ( tAttr.m_sName.cstr() ) )
			continue; // already described it as a field property

		dRes.Add ( tAttr.m_sName );
	}
	return dRes;
}

static int GetMaxMatches ( int iQueryMaxMatches, const CSphIndex * pIndex )
{
	if ( iQueryMaxMatches<=DEFAULT_MAX_MATCHES )
		return iQueryMaxMatches;

	int64_t iDocs = Min ( (int)INT_MAX, pIndex->GetStats().m_iTotalDocuments ); // clamp to int max
	return Min ( iQueryMaxMatches, Max ( iDocs, DEFAULT_MAX_MATCHES ) ); // do not want 0 sorter and sorter longer than query.max_matches
}

SphQueueSettings_t SearchHandler_c::MakeQueueSettings ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const char * szJoinedParent, int iMaxMatches, bool bForceSingleThread, ISphExprHook * pHook ) const
{
	auto& tSess = session::Info();

	SphQueueSettings_t tQS ( pIndex->GetMatchSchema (), m_pProfile, tSess.m_pSqlRowBuffer, &tSess.m_pSessionOpaque1, &tSess.m_pSessionOpaque2 );
	tQS.m_bComputeItems = true;
	tQS.m_pCollection = m_pCollectedDocs;
	tQS.m_pHook = pHook;
	tQS.m_iMaxMatches = GetMaxMatches ( iMaxMatches, pIndex );
	tQS.m_bNeedDocids = m_bNeedDocIDs;	// need docids to merge results from indexes
	tQS.m_fnGetCountDistinct	= [pIndex]( const CSphString & sAttr, CSphString & sModifiedAttr ){ return pIndex->GetCountDistinct ( sAttr, sModifiedAttr ); };
	tQS.m_fnGetCountFilter		= [pIndex]( const CSphFilterSettings & tFilter, CSphString & sModifiedAttr ){ return pIndex->GetCountFilter ( tFilter, sModifiedAttr ); };
	tQS.m_fnGetCount			= [pIndex](){ return pIndex->GetCount(); };
	tQS.m_bEnableFastDistinct = m_dLocal.GetLength()<=1;
	tQS.m_bForceSingleThread = bForceSingleThread;
	tQS.m_dCreateSchema = GetDefaultSchema ( pIndex );
	if ( pJoinedIndex )
		tQS.m_pJoinArgs = std::make_unique<JoinArgs_t> ( pJoinedIndex->GetMatchSchema(), pIndex->GetName(), szJoinedParent ? szJoinedParent : pJoinedIndex->GetName() );

	return tQS;
}


int SearchHandler_c::CreateMultiQueryOrFacetSorters ( const CSphIndex * pIndex, CSphVector<JoinedIndexes_t> & dJoinedIndexes, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook, const char * szParent ) const
{
	int iValidSorters = 0;

	const CSphIndex * pJoinedIndex = dJoinedIndexes[0].m_dIndexes.GetLength() ? dJoinedIndexes[0].m_dIndexes[0] : nullptr;
	auto tQueueSettings = MakeQueueSettings ( pIndex, pJoinedIndex, dJoinedIndexes[0].m_szParent, m_dNQueries.First ().m_iMaxMatches, m_dPSInfo.First().m_bForceSingleThread, pHook );
	sphCreateMultiQueue ( tQueueSettings, m_dNQueries, dSorters, dErrors, tQueueRes, pExtra, m_pProfile, szParent );

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

	int iBatchSize = m_dNQueries[0].m_iJoinBatchSize==-1 ? GetJoinBatchSize() : m_dNQueries[0].m_iJoinBatchSize;
	if ( m_bFacetQueue && !CreateJoinMultiSorter ( pIndex, dJoinedIndexes[0].m_dIndexes, tQueueSettings, m_dNQueries, m_dNJoinQueryOptions, dSorters, iBatchSize, szParent, dJoinedIndexes[0].m_szParent, dErrors[0] ) )
	{
		dSorters.Apply ( [] ( ISphMatchSorter *& pSorter ) { SafeDelete (pSorter); } );
		return 0;
	}

	return iValidSorters;
}


int SearchHandler_c::CreateSingleSorters ( const CSphIndex * pIndex, CSphVector<JoinedIndexes_t> & dJoinedIndexes, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook, const char * szParent ) const
{
	int iValidSorters = 0;
	tQueueRes.m_bAlowMulti = false;
	const int iQueries = m_dNQueries.GetLength();
	for ( int iQuery = 0; iQuery<iQueries; ++iQuery )
	{
		CSphQuery & tQuery = m_dNQueries[iQuery];
		const CSphIndex * pJoinedIndex = dJoinedIndexes[iQuery].m_dIndexes.GetLength() ? dJoinedIndexes[iQuery].m_dIndexes[0] : nullptr;

		// create queue
		auto tQueueSettings = MakeQueueSettings ( pIndex, pJoinedIndex, dJoinedIndexes[0].m_szParent, tQuery.m_iMaxMatches, m_dPSInfo.First().m_bForceSingleThread, pHook );
		ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings, tQuery, dErrors[iQuery], tQueueRes, pExtra, m_pProfile, szParent );
		if ( !pSorter )
			continue;

		// possibly create a wrapper (if we have JOIN)
		int iBatchSize = tQuery.m_iJoinBatchSize==-1 ? GetJoinBatchSize() : tQuery.m_iJoinBatchSize;
		pSorter = CreateJoinSorter ( pIndex, dJoinedIndexes[iQuery].m_dIndexes, tQueueSettings, tQuery, pSorter, m_dNJoinQueryOptions[iQuery], tQueueRes.m_bJoinedGroupSort, iBatchSize, szParent, dJoinedIndexes[iQuery].m_szParent, dErrors[iQuery] );
		if ( !pSorter )
			continue;

		tQuery.m_bZSlist = tQueueRes.m_bZonespanlist;
		dSorters[iQuery] = pSorter;
		++iValidSorters;
	}
	return iValidSorters;
}


int SearchHandler_c::CreateSorters ( const CSphIndex * pIndex, CSphVector<JoinedIndexes_t> & dJoinedIndexes, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t* pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook, const char * szParent ) const
{
	if ( m_bMultiQueue || m_bFacetQueue )
		return CreateMultiQueryOrFacetSorters ( pIndex, dJoinedIndexes, dSorters, dErrors, pExtra, tQueueRes, pHook, szParent );

	return CreateSingleSorters ( pIndex, dJoinedIndexes, dSorters, dErrors, pExtra, tQueueRes, pHook, szParent );
}

static int64_t CalcPredictedTimeMsec ( const CSphQueryResultMeta & tMeta )
{
	assert ( tMeta.m_bHasPrediction );

	int64_t iNanoResult = int64_t(g_iPredictorCostSkip)* tMeta.m_tStats.m_iSkips
		+ g_iPredictorCostDoc * tMeta.m_tStats.m_iFetchedDocs
		+ g_iPredictorCostHit * tMeta.m_tStats.m_iFetchedHits
		+ g_iPredictorCostMatch * tMeta.m_iTotalMatches;

	return iNanoResult/1000000;
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


cServedIndexRefPtr_c SearchHandler_c::CheckIndexSelectable ( const CSphString & sLocal, VecTraits_T<SearchFailuresLog_c> * pNFailuresSet ) const
{
	const auto& pServed = m_dAcquired.Get ( sLocal );
	assert ( pServed );

	if ( ServedDesc_t::IsSelectable ( pServed ) )
		return pServed;

	if ( pNFailuresSet )
		for ( auto & dFailureSet : *pNFailuresSet )
			dFailureSet.SubmitEx ( sLocal, nullptr, "%s", "table is not suitable for select" );

	return {};
}


bool SearchHandler_c::AddJoinedIndex ( const CSphString & sIdx, CSphVector<JoinedServedIndex_t> & dJoinedServed, CSphVector<CSphVector<JoinedServedIndex_t>> & dAllJoinedServed, const CSphString & sParent, VecTraits_T<SearchFailuresLog_c> & dFailuresSet ) const
{
	const auto & pServed = m_dAcquired.Get ( sIdx );
	if ( !pServed )
	{
		for ( auto & tFailureSet : dFailuresSet )
			tFailureSet.SubmitEx ( sIdx, nullptr, "%s", "table not found" );

		return false;
	}

	if ( !ServedDesc_t::IsSelectable ( pServed ) )
	{
		for ( auto & tFailureSet : dFailuresSet )
			tFailureSet.SubmitEx ( sIdx, nullptr, "%s", "table is not suitable for select" );

		return false;
	}

	bool bDupe = false;
	ARRAY_FOREACH ( i, dAllJoinedServed )
		ARRAY_FOREACH ( j, dAllJoinedServed[i] )
			if ( dAllJoinedServed[i][j].m_pServed==pServed )
			{
				bDupe = true;
				break;
			}

	dJoinedServed.Add ( { pServed, sParent, bDupe } );

	return true;
}


bool SearchHandler_c::PopulateJoinedIndexes ( CSphVector<CSphVector<JoinedServedIndex_t>> & dJoinedServed, VecTraits_T<SearchFailuresLog_c> & dFailuresSet ) const
{
	dJoinedServed.Resize ( m_dNQueries.GetLength() );
	ARRAY_FOREACH ( i, m_dNQueries )
	{
		const auto & tQuery = m_dNQueries[i];
		if ( tQuery.m_sJoinIdx.IsEmpty() )
			continue;

		// check for distributed right index
		auto pServedDistIndex = GetDistr ( tQuery.m_sJoinIdx );
		if ( pServedDistIndex )
		{
			for ( auto & sIdx : pServedDistIndex->m_dLocal )
				if ( !AddJoinedIndex ( sIdx, dJoinedServed[i], dJoinedServed, tQuery.m_sJoinIdx, dFailuresSet ) )
					return false;			
		}
		else
			if ( !AddJoinedIndex ( tQuery.m_sJoinIdx, dJoinedServed[i], dJoinedServed, "", dFailuresSet ) )
				return false;
	}

	return true;
}


bool SearchHandler_c::CreateValidSorters ( VecTraits_T<ISphMatchSorter *> & dSrt, SphQueueRes_t * pQueueRes, VecTraits_T<SearchFailuresLog_c> & dFlr, StrVec_t * pExtra, const CSphIndex * pIndex, CSphVector<JoinedIndexes_t> & dJoinedIndexes, const CSphString & sLocal, const char * szParent, ISphExprHook * pHook )
{
	auto iQueries = dSrt.GetLength();
	#if PARANOID
	for ( const auto* pSorter : dSrt )
		assert ( !pSorter );
	#endif

	CSphFixedVector<CSphString> dErrors ( iQueries );
	int iValidSorters = CreateSorters ( pIndex, dJoinedIndexes, dSrt, dErrors, pExtra, *pQueueRes, pHook, szParent );
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


void SearchHandler_c::PopulateCountDistinct ( CSphVector<CSphVector<int64_t>> & dCountDistinct ) const
{
	dCountDistinct.Resize ( m_dLocal.GetLength() );

	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		const LocalIndex_t & tLocal = m_dLocal[iLocal];
		auto pIndex = CheckIndexSelectable ( tLocal.m_sName );
		if ( !pIndex )
			continue;

		auto & dIndexCountDistinct = dCountDistinct[iLocal];
		dIndexCountDistinct.Resize ( m_dNQueries.GetLength() );
		dIndexCountDistinct.Fill(-1);
		ARRAY_FOREACH ( i, dIndexCountDistinct )
		{
			auto & tQuery = m_dNQueries[i];
			int iGroupby = GetAliasedAttrIndex ( tQuery.m_sGroupBy, tQuery, RIdx_c(pIndex)->GetMatchSchema() );
			if ( iGroupby<0 )
				continue;

			auto & sAttr = RIdx_c(pIndex)->GetMatchSchema().GetAttr(iGroupby).m_sName;
			CSphString sModifiedAttr;
			dIndexCountDistinct[i] = RIdx_c(pIndex)->GetCountDistinct ( sAttr, sModifiedAttr );
		}
	}
}


int SearchHandler_c::CalcMaxThreadsPerIndex ( int iConcurrency ) const
{
	auto iNumValid = m_dLocal.count_of ( [this] (const auto& i) { return CheckIndexSelectable ( i.m_sName ); } );
	return ::CalcMaxThreadsPerIndex ( iConcurrency, iNumValid );
}


void SearchHandler_c::CalcThreadsPerIndex ( int iConcurrency )
{
	if ( !iConcurrency )
		iConcurrency =  MaxChildrenThreads();

	int iBusyWorkers = Max ( GlobalWorkPool()->CurTasks() - 1, 0 ); // ignore current task
	int iAvailableWorkers = Max ( Coro::CurrentScheduler()->WorkingThreads() - iBusyWorkers, 1 );
	iAvailableWorkers = Min ( iAvailableWorkers, iConcurrency );

	// this is need to obey ps dispatcher template, if it defines concurrency
	// that will help to perform reproducable queries, see test 261
	auto tDispatch = GetEffectivePseudoShardingDispatcherTemplate ();
	Dispatcher::Unify ( tDispatch, m_dNQueries.First ().m_tPseudoShardingDispatcher );
	if ( tDispatch.concurrency )
	{
//		sphWarning ( "correct iAvailableWorkers %d to defined %d", iAvailableWorkers, tDispatch.concurrency );
		iAvailableWorkers = tDispatch.concurrency;
	}

	CSphVector<CSphVector<int64_t>> dCountDistinct;
	PopulateCountDistinct ( dCountDistinct );

	int iMaxThreadsPerIndex = CalcMaxThreadsPerIndex ( iAvailableWorkers );

	CSphVector<SplitData_t> dSplitData ( m_dLocal.GetLength() );

	int iEnabledIndexes = 0;
	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		const LocalIndex_t & tLocal = m_dLocal[iLocal];
		auto pIndex = CheckIndexSelectable ( tLocal.m_sName );
		if ( !pIndex )
			continue;

		iEnabledIndexes++;
		auto & tPSInfo = m_dPSInfo[iLocal];
		auto & tSplitData = dSplitData[iLocal];
		RIdx_c pIdx { pIndex };
		if ( GetPseudoSharding () || pIdx->IsRT() )
		{
			// do metric calcs
			tPSInfo.m_iMaxThreads = iMaxThreadsPerIndex;
			auto tMetric = pIdx->GetPseudoShardingMetric ( m_dNQueries, dCountDistinct[iLocal], tPSInfo.m_iMaxThreads, tPSInfo.m_bForceSingleThread );
			assert ( tMetric.first>=0 );

			tSplitData.m_iMetric = tMetric.first;

			bool bExplicitConcurrency = m_dNQueries.any_of ( []( auto & tQuery ){ return tQuery.m_iConcurrency>0; } );
			tSplitData.m_iThreadCap = bExplicitConcurrency ? 0 : tMetric.second;	// ignore thread cap if concurrency is explicitly specified
		}
		else
		{
			// don't do metric calcs; we are guaranteed to have one thread
			// set the 'force single thread' flag to make sure max_matches won't be increased when it is not necessary
			tPSInfo = { 1, 1, true };
			tSplitData.m_iThreadCap = 1;
		}
	}

	if ( iAvailableWorkers>iEnabledIndexes )
	{
		IntVec_t dThreads;
		DistributeThreadsOverIndexes ( dThreads, dSplitData, iAvailableWorkers );
		ARRAY_FOREACH ( i, dThreads )
			m_dPSInfo[i].m_iThreads = dThreads[i];
	}
}


class AssignTag_c final: public MatchProcessor_i
{
public:
	explicit AssignTag_c ( int iTag )
		: m_iTag ( iTag )
	{}

	void Process ( CSphMatch * pMatch ) 			{ ProcessMatch(pMatch); }
	bool ProcessInRowIdOrder() const 				{ return false;	}
	void Process ( VecTraits_T<CSphMatch *> & dMatches )  { dMatches.for_each ( [this]( CSphMatch * pMatch ){ ProcessMatch(pMatch); } ); }

private:
	int	m_iTag = 0;
	void ProcessMatch ( CSphMatch * pMatch )		{ pMatch->m_iTag = m_iTag; }
};


class GlobalSorters_c final
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

	bool StoreSorter ( int iQuery, int iIndex, ISphMatchSorter * & pSorter, const DocstoreReader_i * pDocstore, int iTag ) const
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


CSphVector<JoinedIndexes_t> SearchHandler_c::GetRlockedJoinedIndexes ( const CSphVector<CSphVector<JoinedServedIndex_t>> & dJoinedServed, std::vector<RIdx_c> & dRLockedJoined ) const
{
	CSphVector<JoinedIndexes_t> dJoinedIndexes;
	int iTotal = 0;
	for ( auto & i : dJoinedServed )
		iTotal += i.GetLength();

	dRLockedJoined.reserve(iTotal);

	for ( auto & i : dJoinedServed )
	{
		auto & tJoinedVec = dJoinedIndexes.Add();
		for ( auto & j : i )
		{
			tJoinedVec.m_szParent = j.m_sParent.IsEmpty() ? nullptr : j.m_sParent.cstr();

			if ( !j.m_pServed )
			{
				tJoinedVec.m_dIndexes.Add( nullptr );
				continue;
			}

			if ( j.m_bDupe )
				tJoinedVec.m_dIndexes.Add ( RIdx_c(j.m_pServed) );
			else
			{
				dRLockedJoined.emplace_back ( j.m_pServed );
				tJoinedVec.m_dIndexes.Add ( dRLockedJoined.back() );
			}
		}
	}

	return dJoinedIndexes;
}


bool SearchHandler_c::SubmitSuccess ( CSphVector<ISphMatchSorter *> & dSorters, GlobalSorters_c & tGlobalSorters, LocalSearchRef_t & tCtx, int64_t & iCpuTime, int iQuery, int iLocal, const CSphQueryResultMeta & tMqMeta, const CSphQueryResult & tMqRes )
{
	auto & dNFailuresSet = tCtx.m_dFailuresSet;
	auto & dNAggrResults = tCtx.m_dAggrResults;
	auto & dNResults = tCtx.m_dResults;
	int iNumQueries = m_dNQueries.GetLength();
	const LocalIndex_t & tLocal = m_dLocal[iLocal];
	const CSphString & sLocal = tLocal.m_sName;
	const char * szParent = tLocal.m_sParentIndex.cstr();
	int iOrderTag = tLocal.m_iOrderTag;
	ISphMatchSorter * pSorter = dSorters[iQuery];

	AggrResult_t & tNRes = dNAggrResults[iQuery];
	int iQTimeForStats = tNRes.m_iQueryTime;
	auto pDocstore = m_bMultiQueue ? tMqRes.m_pDocstore : dNResults[iQuery].m_pDocstore;

	// multi-queue only returned one result set meta, so we need to replicate it
	if ( m_bMultiQueue )
	{
		// these times will be overridden below, but let's be clean
		iQTimeForStats = tMqMeta.m_iQueryTime / iNumQueries;
		tNRes.m_iQueryTime += iQTimeForStats;
		tNRes.MergeWordStats ( tMqMeta );
		tNRes.m_iMultiplier = iNumQueries;
		tNRes.m_iCpuTime += tMqMeta.m_iCpuTime / iNumQueries;
		tNRes.m_bTotalMatchesApprox |= tMqMeta.m_bTotalMatchesApprox;

		iCpuTime /= iNumQueries;
	}
	else if ( tNRes.m_iMultiplier==-1 ) // multiplier -1 means 'error'
	{
		dNFailuresSet[iQuery].Submit ( sLocal, szParent, tNRes.m_sError.cstr() );
		return false;
	}

	++tNRes.m_iSuccesses;
	tNRes.m_iCpuTime = iCpuTime;
	tNRes.m_iTotalMatches += pSorter->GetTotalCount();
	tNRes.m_iPredictedTime = tNRes.m_bHasPrediction ? CalcPredictedTimeMsec ( tNRes ) : 0;

	m_dQueryIndexStats[iLocal].m_dStats[iQuery].m_iSuccesses = 1;
	m_dQueryIndexStats[iLocal].m_dStats[iQuery].m_tmQueryTime = iQTimeForStats * 1000; // FIME!!! change time in meta into miscroseconds
	m_dQueryIndexStats[iLocal].m_dStats[iQuery].m_uFoundRows = pSorter->GetTotalCount();

	// extract matches from sorter
	if ( !tGlobalSorters.StoreSorter ( iQuery, iLocal, dSorters[iQuery], pDocstore, iOrderTag ) )
		tNRes.AddResultset ( pSorter, pDocstore, iOrderTag, m_dNQueries[iQuery].m_iCutoff );

	if ( !tNRes.m_sWarning.IsEmpty() )
		dNFailuresSet[iQuery].Submit ( sLocal, szParent, tNRes.m_sWarning.cstr() );

	return true;
}


void SearchHandler_c::RunLocalSearches()
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
		dLocalIndexes.Add ( CheckIndexSelectable ( i.m_sName ) );

	GlobalSorters_c tGlobalSorters ( m_dNQueries, dLocalIndexes );

	m_dPSInfo.Resize(iNumLocals);
	m_dPSInfo.Fill ( { 1, 1, false } );

	CSphFixedVector<int> dOrder { iNumLocals };
	for ( int i = 0; i<iNumLocals; ++i )
		dOrder[i] = i;

	auto tDispatch = GetEffectiveBaseDispatcherTemplate();
	Dispatcher::Unify ( tDispatch, m_dNQueries.First().m_tMainDispatcher );

	// the context
	ClonableCtx_T<LocalSearchRef_t, LocalSearchClone_t, Threads::ECONTEXT::UNORDERED> dCtx { m_tHook, pMainExtra, m_dNFailuresSet, m_dNAggrResults, m_dNResults };
	auto pDispatcher = Dispatcher::Make ( iNumLocals, m_dNQueries.First().m_iConcurrency, tDispatch, dCtx.IsSingle() );
	dCtx.LimitConcurrency ( pDispatcher->GetConcurrency() );

	bool bSingle = pDispatcher->GetConcurrency()==1;

//	sphWarning ( "iConcurrency: %d", iConcurrency );

	if ( !bSingle )
	{
//		sphWarning ( "Reordering..." );
		// if run parallel - start in mass order, if single - in natural order
		// set order by decreasing index mass (heaviest one comes first). That is why 'less' implemented by '>'
		dOrder.Sort ( Lesser ( [this] ( int a, int b ) {
			return m_dLocal[a].m_iMass>m_dLocal[b].m_iMass;
		} ) );

		CalcThreadsPerIndex ( pDispatcher->GetConcurrency() );
	}

//	for ( int iOrder : dOrder )
//		sphWarning ( "Sorted: %d, Order %d, mass %d", !!bSingle, iOrder, (int) m_dLocal[iOrder].m_iMass );

	std::atomic iTotalSuccesses { 0 };
	Coro::ExecuteN ( dCtx.Concurrency ( iNumLocals ), [&]
	{
		auto pSource = pDispatcher->MakeSource();
		int iJob = -1; // make it consumed

		if ( !pSource->FetchTask ( iJob ) )
		{
			LOCSEARCHINFO << "Early finish parallel RunLocalSearches because of empty queue";
			return; // already nothing to do, early finish.
		}

		// these two moved from inside the loop to avoid construction on every turn
		CSphVector<ISphMatchSorter *> dSorters ( iQueries );
		dSorters.ZeroVec ();

		auto tJobContext = dCtx.CloneNewContext();
		auto& tCtx = tJobContext.first;
		LOCSEARCHINFO << "RunLocalSearches cloned context " << tJobContext.second;
		Coro::SetThrottlingPeriodMS ( session::GetThrottlingPeriodMS() );
		while ( true )
		{
			if ( !pSource->FetchTask ( iJob ) )
				return; // all is done

			auto iLocal = dOrder[iJob];
			LOCSEARCHINFO << "RunLocalSearches " << tJobContext.second << ", iJob: " << iJob << ", iLocal: " << iLocal << ", mass " << ( (int)m_dLocal[iLocal].m_iMass );
			iJob = -1; // mark it consumed

			int64_t iCpuTime = -sphTaskCpuTimer ();

			// FIXME!!! handle different proto
			myinfo::SetTaskInfo( R"(api-search query="%s" comment="%s" index="%s")",
									 m_dNQueries.First().m_sQuery.scstr (),
									 m_dNQueries.First().m_sComment.scstr (),
									 m_dLocal[iLocal].m_sName.scstr ());

			const LocalIndex_t & dLocal = m_dLocal[iLocal];
			const CSphString& sLocal = dLocal.m_sName;
			const char * szParent = dLocal.m_sParentIndex.cstr ();
			int iIndexWeight = dLocal.m_iWeight;
			auto& dNFailuresSet = tCtx.m_dFailuresSet;
			auto& dNAggrResults = tCtx.m_dAggrResults;
			auto& dNResults = tCtx.m_dResults;
			auto* pExtra = tCtx.m_pExtra;

			// publish crash query index
			GlobalCrashQueryGetRef().m_dIndex = FromStr ( sLocal );

			// prepare and check the index
			cServedIndexRefPtr_c pServed = CheckIndexSelectable ( sLocal, &dNFailuresSet );
			if ( !pServed )
				continue;

			CSphVector<CSphVector<JoinedServedIndex_t>> dJoinedServed;
			if ( !PopulateJoinedIndexes ( dJoinedServed, dNFailuresSet ) )
				continue;

			bool bResult = false;
			CSphQueryResultMeta tMqMeta;
			CSphQueryResult tMqRes;
			tMqRes.m_pMeta = &tMqMeta;

			{	// scope for r-locking the index
				RIdx_c pIndex { pServed };

				tCtx.m_tHook.SetIndex ( pIndex );
				tCtx.m_tHook.SetQueryType ( m_eQueryType );

				std::vector<RIdx_c> dRLockedJoined;
				CSphVector<JoinedIndexes_t> dJoinedIndexes = GetRlockedJoinedIndexes ( dJoinedServed, dRLockedJoined );

				// create sorters
				SphQueueRes_t tQueueRes;
				if ( !CreateValidSorters ( dSorters, &tQueueRes, dNFailuresSet, pExtra, pIndex, dJoinedIndexes, sLocal, szParent, &tCtx.m_tHook ) )
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
				tMultiArgs.m_iThreads = bCanBeCloned ? m_dPSInfo[iLocal].m_iThreads : 1;
				tMultiArgs.m_iTotalThreads = m_dPSInfo[iLocal].m_iMaxThreads;
				tMultiArgs.m_bFinalizeSorters = !tGlobalSorters.NeedGlobalSorters();

				LOCSEARCHINFO << "RunLocalSearches index:" << pIndex->GetName();

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
					// but some sorters could have failed at "create sorter" stage
					ISphMatchSorter * pSorter = dSorters[i];
					if ( !pSorter )
						continue;

					if ( SubmitSuccess ( dSorters, tGlobalSorters, tCtx, iCpuTime, i, iLocal, tMqMeta, tMqRes ) )
						iTotalSuccesses.fetch_add ( 1, std::memory_order_relaxed );
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

			LOCSEARCHINFO << "RunLocalSearches result " << bResult << " index " << sLocal;

			if ( !pSource->FetchTask ( iJob ) )
				return; // all is done

			Coro::ThrottleAndKeepCrashQuery (); // we set CrashQuery anyway at the start of the loop
		}
	});
	LOCSEARCHINFO << "RunLocalSearches processed in " << dCtx.NumWorked() << " thread(s)";
	dCtx.Finalize (); // merge mt results (if any)

	tGlobalSorters.MergeResults(m_dNAggrResults);

	// m_iQueryTime is already accumulated from query metadata; adding tmLocal again causes double accounting
	tmLocal = sphMicroTimer ()-tmLocal;

	auto iTotalSuccessesInt = iTotalSuccesses.load ( std::memory_order_relaxed );

	for ( auto iLocal = 0; iLocal<iNumLocals; ++iLocal )
		for ( int iQuery = 0; iQuery<iQueries; ++iQuery )
		{
			QueryStat_t & tStat = m_dQueryIndexStats[iLocal].m_dStats[iQuery];
			if ( tStat.m_iSuccesses )
				tStat.m_tmQueryTime = tmLocal / iTotalSuccessesInt;
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

		bHasLocalDF |= tQuery.m_bLocalDF.value_or ( false );
		if ( !tQuery.m_sQuery.IsEmpty() && tQuery.m_bLocalDF.value_or ( false ) )
			bOnlyNoneRanker &= ( tQuery.m_eRanker==SPH_RANK_NONE );
	}
	// bail out queries: full-scan, ranker=none, local_idf=0
	if ( bOnlyFullScan || bOnlyNoneRanker || !bHasLocalDF )
		return;

	CSphVector<char> dQuery;
	dQuery.Reserve ( 512 );
	for ( const CSphQuery & tQuery : m_dNQueries )
	{
		if ( tQuery.m_sQuery.IsEmpty() || !tQuery.m_bLocalDF.value_or ( false ) || tQuery.m_eRanker==SPH_RANK_NONE )
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
	CSphVector<IndexSettings_t> dLocal;
	dLocal.Reserve ( m_dLocal.GetLength() );
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


// process system.table
static void FixupSystemTableW ( StrVec_t & dNames, CSphQuery & tQuery ) noexcept
{
	if ( dNames.GetLength ()==1 && dNames.First ().EqN ( "system" ) && !tQuery.m_dStringSubkeys.IsEmpty () )
	{
		dNames[0] = SphSprintf ( "system%s", tQuery.m_dStringSubkeys[0].cstr () );
		tQuery.m_dStringSubkeys.Remove(0);
	}
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
	// add indexes required by JOIN
	// but don't try to acquire local indexes if query is issued only for remote distributed
	if ( m_dLocal.GetLength() )
	{
		StringBuilder_c sFailed (", ");
		for ( const auto & tQuery : m_dNQueries )
		{
			if ( tQuery.m_sJoinIdx.IsEmpty() )
				continue;

			auto pServedDistIndex = GetDistr ( tQuery.m_sJoinIdx );
			if ( pServedDistIndex )
			{
				if ( pServedDistIndex->m_dAgents.GetLength() )
				{
					m_sError << "join not allowed on distributed table containing remote agents '" << tQuery.m_sJoinIdx << "'";
					return false;
				}

				for ( const auto & i : pServedDistIndex->m_dLocal )
					if ( !m_dAcquired.AddUniqIndex(i) )
						sFailed << i;
			}
			else if ( !m_dAcquired.AddUniqIndex ( tQuery.m_sJoinIdx ) )
				sFailed << tQuery.m_sJoinIdx;
		}

		if ( !sFailed.IsEmpty() )
		{
			m_sError << "unknown local table(s) '" << sFailed << "' in search request";
			return false;
		}
	}

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

	// report failed
	m_sError << "unknown local table(s) '" << sFailed << "' in search request";
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

	// distribute overhead \  tmDelta proportionally to all tables
	// if table has 0 successes - it gets 0 overhead as (tmDelta * 0 / Divider = 0)
	int64_t nDistrDivider = iTotalSuccesses;
	if ( nDistrDivider )
	{
		for ( auto &tDistrStat : dDistrServedByAgent )
		{
			for ( QueryStat_t& tStat : tDistrStat.m_dStats )
			{
				auto tmDeltaWallAgent = tmDelta * tStat.m_iSuccesses / nDistrDivider;
				tStat.m_tmQueryTime += tmDeltaWallAgent;
			}
		}
	}

	int64_t nLocalDivider = iTotalSuccesses;
	if ( nLocalDivider )
	{
		for ( auto &dQueryIndexStat : m_dQueryIndexStats )
		{
			for ( QueryStat_t& tStat : dQueryIndexStat.m_dStats )
			{
				// do not need to check tStat.m_iSuccesses>0 here
				// if m_iSuccesses is 0 - the added time is 0
				int64_t tmDeltaWallLocal = tmDelta * tStat.m_iSuccesses / nLocalDivider;
				tStat.m_tmQueryTime += tmDeltaWallLocal;
			}
		}
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

			pServed->m_pStats->AddQueryStat ( tStat.m_uFoundRows, tStat.m_tmQueryTime );
			for ( auto &tDistr : dDistrServedByAgent )
			{
				if ( tDistr.m_dLocalNames.Contains ( m_dLocal[iLocal].m_sName ) )
				{
					tDistr.m_dStats[iQuery].m_tmQueryTime += tStat.m_tmQueryTime;
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

				pServedDistIndex->m_tStats.AddQueryStat ( tStat.m_uFoundRows, tStat.m_tmQueryTime );
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
	CSphQuery & tQuery = m_dNQueries.First ();

	if ( tQuery.m_sIndexes=="*" )
	{
		// they're all local, build the list
		m_dLocal = CollectAllLocalIndexes ( tQuery.m_dIndexWeights );
		return true;
	}

	m_dLocal.Reset ();
	int iOrderTag = 0;
	bool bSysVar = tQuery.m_sIndexes.Begins ( "@@" );
//		bSysVar = bSysVar || StrEqN ( FROMS ("information_schema"), tQuery.m_sIndexes.cstr() );

	// search through specified local indexes
	StrVec_t dIdxNames;
	if ( bSysVar )
		dIdxNames.Add ( tQuery.m_sIndexes );
	else
	{
		ParseIndexList ( tQuery.m_sIndexes, dIdxNames );
		FixupSystemTableW ( dIdxNames, tQuery );
	}

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
				pConn->m_iMyConnectTimeoutMs = pDist->GetAgentConnectTimeoutMs();
				pConn->m_iMyQueryTimeoutMs = ( tQuery.m_iAgentQueryTimeoutMs!=DEFAULT_QUERY_TIMEOUT ? tQuery.m_iAgentQueryTimeoutMs : pDist->GetAgentQueryTimeoutMs() );
				dRemotes.Add ( pConn );
			}

			for ( const CSphString& sLocalAgent : pDist->m_dLocal )
			{
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
				dResult.m_sWarning.SetSprintf ( "distributed multi-table query '%s' doesn't support divide_remote_ranges", tQuery.m_sIndexes.cstr() );
		}
	}

	// eliminate local dupes that come from distributed indexes
	if ( bHasLocalsAgents )
		UniqLocals ( dLocals );
	else
		m_dLocal.SwapData ( dLocals );

	return !bSysVar;
}

// generate warning about slow full text expansion for queries there
// merged terms is less than expanded terms
// slower than query_log_min_msec or slower 100ms
static void CheckExpansion ( CSphQueryResultMeta & tMeta )
{
	if ( tMeta.m_hWordStats.IsEmpty() || !tMeta.m_tExpansionStats.m_iTerms )
		return;

	if ( tMeta.m_tExpansionStats.m_iMerged>=tMeta.m_tExpansionStats.m_iTerms )
		return;

	if ( tMeta.m_iQueryTime<100 || ( g_iQueryLogMinMs>0 && tMeta.m_iQueryTime<g_iQueryLogMinMs ) )
		return;

	int iTotal = tMeta.m_tExpansionStats.m_iMerged + tMeta.m_tExpansionStats.m_iTerms;
	int iMerged = (int)( float(tMeta.m_tExpansionStats.m_iMerged) * 100.0f / iTotal );

	StringBuilder_c sBuf;
	// notice, msg should not be finished with dot, and start with capital. That is because several messages can be unified with '; ' delimiter.
	sBuf.Appendf ( "current merge of expanded terms is %d%%, with a total of %d. Read manual about 'expansion_merge_threshold_docs/hits'", iMerged, iTotal );
	if ( !tMeta.m_sWarning.IsEmpty() )
		sBuf.Appendf ( "; %s", tMeta.m_sWarning.cstr() );

	sBuf.MoveTo ( tMeta.m_sWarning );
}

// query info - render query into the view
struct QueryInfo_t : TaskInfo_t
{
	DECLARE_RENDER( QueryInfo_t );

	// actually it is 'virtually hazard'. Don't care about query* itself, however later in dtr of Searchandler_t
	// will work with refs to members of it's m_dQueries and retire of whole vec.
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

static void FillupFacetError ( int iQueries, const VecTraits_T<CSphQuery> & dQueries, VecTraits_T<AggrResult_t> & dAggrResults )
{
	if ( iQueries>1 && !dAggrResults.Begin()->m_iSuccesses && dAggrResults.Begin()->m_sError.IsEmpty() && dQueries.Begin()->m_bFacetHead )
	{
		const CSphString * pError = nullptr;
		for ( int iRes=0; iRes<iQueries; ++iRes )
		{
			const AggrResult_t & tRes = dAggrResults[iRes];
			if ( !tRes.m_iSuccesses && !tRes.m_sError.IsEmpty() )
			{
				pError = &tRes.m_sError;
				break;
			}
		}

		if ( !pError )
			return;

		for ( int iRes=0; iRes<iQueries; ++iRes )
		{
			AggrResult_t & tRes = dAggrResults[iRes];
			if ( !tRes.m_sError.IsEmpty() )
				break;

			tRes.m_sError = *pError;
		}
	}
}

// one or more queries against one and same set of indexes
void SearchHandler_c::RunSubset ( int iStart, int iEnd )
{
	int iQueries = iEnd - iStart;
	m_dNQueries = m_dQueries.Slice ( iStart, iQueries );
	m_dNJoinQueryOptions = m_dJoinQueryOptions.Slice ( iStart, iQueries );
	m_dNAggrResults = m_dAggrResults.Slice ( iStart, iQueries );
	m_dNResults = m_dResults.Slice ( iStart, iQueries );
	m_dNFailuresSet = m_dFailuresSet.Slice ( iStart, iQueries );

	// we've own scoped context here
	auto pQueryInfo = new QueryInfo_t;
	pQueryInfo->m_pHazardQuery.store ( m_dNQueries.begin(), std::memory_order_release );
	ScopedInfo_T pTlsQueryInfo ( pQueryInfo );

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

	auto fnError = AtScopeExit ( [this]()
	{
		if ( !m_sError.IsEmpty() )
			m_dNAggrResults.for_each ( [this] ( auto& r ) { r.m_sError = (CSphString) m_sError; } );
	});

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
		// process query to @@*, as @@system.threads, information_schema, etc.
		if ( !ParseSysVarsAndTables () )
			return;
		// here we deal
	}

	// at this point m_dLocal contains list of valid local indexes (i.e., existing ones),
	// and these indexes are also rlocked and available by calling m_dAcquired.Get()

	// sanity check
	if ( dRemotes.IsEmpty() && m_dLocal.IsEmpty() )
	{
		m_sError << "no enabled tables to search";
		return;
	}

	if ( m_dNQueries[0].m_iLimit==-1 && ( !dRemotes.IsEmpty () || m_dLocal.GetLength ()>1 ) )
	{
		m_sError << "only one local table allowed in streaming select";
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
	std::unique_ptr<ReplyParser_i> tParser;
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
						pDistr->m_dStats[iRes].m_tmQueryTime += tRemoteResult.m_iQueryTime * 1000; // FIME!!! change time in meta into miscroseconds
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
		CheckExpansion ( tRes );

		////////////
		// finalize
		////////////

		tRes.m_iOffset = Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
		auto iLimit = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );
		tRes.m_iCount = Max ( Min ( iLimit, tRes.GetLength()-tRes.m_iOffset ), 0 );
		tRes.m_iMatches = tRes.m_iCount;
		for ( const auto & tLocal : m_dLocal )
			tRes.m_dIndexNames.Add ( tLocal.m_sName );
	}

	// pop up facet error from one of the query to the front
	FillupFacetError ( iQueries, m_dQueries, m_dNAggrResults );

	/////////////////////////////////
	// functions on a table argument
	/////////////////////////////////

	for ( int i=0; i<iQueries; ++i )
	{
		AggrResult_t & tRes = m_dNAggrResults[i];
		auto& pTableFunc = m_dTables[iStart+i];

		if ( !pTableFunc )
			continue;

		if ( !tRes.m_iSuccesses )
			continue;

		// FIXME! log such queries properly?
		SwitchProfile ( m_pProfile, SPH_QSTATE_TABLE_FUNC );
		if ( !pTableFunc->Process ( &tRes, tRes.m_sError ) )
			tRes.m_iSuccesses = 0;
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

static ESphAggrFunc GetAggr ( Aggr_e eAggrFunc )
{
	switch ( eAggrFunc )
	{
	case Aggr_e::MIN: return SPH_AGGR_MIN;
	case Aggr_e::MAX: return SPH_AGGR_MAX;
	case Aggr_e::SUM: return SPH_AGGR_SUM;
	case Aggr_e::AVG: return SPH_AGGR_AVG;
	default: return SPH_AGGR_NONE;
	}
}

template<bool HAS_ATTRS>
void AddCompositeItems ( const CSphString & sCol, CSphVector<CSphQueryItem> & dItems, sph::StringSet * pAttrs )
{
	if_const ( HAS_ATTRS )
	{
		assert ( pAttrs );
	}

	StrVec_t dAttrs;
	sphSplit ( dAttrs, sCol.cstr(), "," );
	for ( const CSphString & sCol : dAttrs )
	{
		if_const ( HAS_ATTRS )
			if ( (*pAttrs)[sCol] )
				continue;

		CSphQueryItem & tItem = dItems.Add();
		tItem.m_sExpr = sCol;
		tItem.m_sAlias = sCol;
		if_const ( HAS_ATTRS )
			(*pAttrs).Add ( sCol );
	}
}

SearchHandler_c CreateMsearchHandler ( std::unique_ptr<QueryParser_i> pQueryParser, QueryType_e eQueryType, ParsedJsonQuery_t & tParsed )
{
	JsonQuery_c & tQuery = tParsed.m_tQuery;
	tQuery.m_pQueryParser = pQueryParser.get();

	int iQueries = ( 1 + tQuery.m_dAggs.GetLength() );

	// make single grouper / sorter to match plain query with only group by (wo FACET) if single aggs set and main query limit=0
	if ( eQueryType==QueryType_e::QUERY_JSON && tQuery.m_dAggs.GetLength()==1 && tQuery.m_iLimit==0 && tQuery.m_dAggs[0].m_eAggrFunc==Aggr_e::NONE )
	{
		iQueries = 1;
		tQuery.m_bGroupEmulation = true;
		const JsonAggr_t & tAggs = tQuery.m_dAggs[0];
		tQuery.m_iLimit = tAggs.m_iSize;
		tQuery.m_sGroupBy = tAggs.m_sCol;
		if ( tAggs.m_sSort.IsEmpty() )
			tQuery.m_sGroupSortBy = tQuery.m_sOrderBy;
		else
			tQuery.m_sGroupSortBy = tAggs.m_sSort;

		tQuery.m_dRefItems = tQuery.m_dItems;
		CSphQueryItem & tCountItem = tQuery.m_dItems.Add();
		tCountItem.m_sExpr = "count(*)";
		tCountItem.m_sAlias = "count(*)";
	}

	SearchHandler_c tHandler { iQueries, std::move ( pQueryParser ), eQueryType, true };

	if ( !tQuery.m_dAggs.GetLength() || eQueryType==QUERY_SQL || tQuery.m_bGroupEmulation )
	{
		tHandler.SetQuery ( 0, tQuery, nullptr );
		tHandler.SetJoinQueryOptions ( 0, tParsed.m_tJoinQueryOptions );
		return tHandler;
	}

	tQuery.m_dRefItems = tQuery.m_dItems;
	// FIXME!!! no need to add count for AggrFunc aggregates
	CSphQueryItem & tCountItem = tQuery.m_dItems.Add();
	tCountItem.m_sExpr = "count(*)";
	tCountItem.m_sAlias = "count(*)";

	sph::StringSet hAttrs;
	for ( const auto & tItem : tQuery.m_dItems )
		hAttrs.Add ( tItem.m_sAlias );

	ARRAY_FOREACH ( i, tQuery.m_dAggs )
	{
		const JsonAggr_t & tBucket = tQuery.m_dAggs[i];

		// add only new items
		if ( hAttrs[tBucket.m_sCol] )
			continue;

		if ( tBucket.m_eAggrFunc==Aggr_e::COUNT )
			continue;

		if ( tBucket.m_eAggrFunc==Aggr_e::COMPOSITE )
		{
			AddCompositeItems<true> ( tBucket.m_sCol, tQuery.m_dItems, &hAttrs );
			continue;
		}

		CSphQueryItem & tItem = tQuery.m_dItems.Add();
		if ( tBucket.m_eAggrFunc!=Aggr_e::NONE )
		{
			tItem.m_sExpr = DumpAggr ( tBucket.m_sCol.cstr(), tBucket );
			tItem.m_sAlias = GetAggrName ( i, tBucket.m_sCol );
			tItem.m_eAggrFunc = GetAggr ( tBucket.m_eAggrFunc );
		} else
		{
			tItem.m_sExpr = tBucket.m_sCol;
			tItem.m_sAlias = tBucket.m_sCol;
			hAttrs.Add ( tBucket.m_sCol );
		}
	}

	tQuery.m_bFacetHead = true;
	tHandler.SetQuery ( 0, tQuery, nullptr );
	tHandler.SetJoinQueryOptions ( 0, tParsed.m_tJoinQueryOptions );
	int iRefLimit = tQuery.m_iLimit;
	int iRefOffset = tQuery.m_iOffset;

	ARRAY_FOREACH ( i, tQuery.m_dAggs )
	{
		const JsonAggr_t & tBucket = tQuery.m_dAggs[i];

		// common to main query but flags, select list and ref items should uniq
		tQuery.m_eGroupFunc = SPH_GROUPBY_ATTR;

		// facet flags
		tQuery.m_bFacetHead = false;
		tQuery.m_bFacet = true;

		// select list to facet query
		tQuery.m_sSelect.SetSprintf ( "%s", tBucket.m_sCol.cstr() );

		// ref items to facet query
		tQuery.m_dRefItems.Resize ( 0 );
		switch ( tBucket.m_eAggrFunc )
		{
			case Aggr_e::SIGNIFICANT:
			case Aggr_e::HISTOGRAM:
			case Aggr_e::DATE_HISTOGRAM:
			case Aggr_e::RANGE:
			case Aggr_e::DATE_RANGE:
			{
				CSphQueryItem & tItem = tQuery.m_dRefItems.Add();
				tItem.m_sExpr = DumpAggr ( tBucket.m_sCol.cstr(), tBucket );
				tItem.m_sAlias = GetAggrName ( i, tBucket.m_sCol );
			}
			break;

			case Aggr_e::COMPOSITE:
				AddCompositeItems<false> ( tBucket.m_sCol, tQuery.m_dRefItems, nullptr );
				break;

			case Aggr_e::COUNT:
			break;

			case Aggr_e::MIN:
			case Aggr_e::MAX:
			case Aggr_e::SUM:
			case Aggr_e::AVG:
			{
				CSphQueryItem & tItem = tQuery.m_dRefItems.Add();
				tItem.m_sExpr = DumpAggr ( tBucket.m_sCol.cstr(), tBucket );
				tItem.m_sAlias = GetAggrName ( i, tBucket.m_sCol );
				tItem.m_eAggrFunc = GetAggr ( tBucket.m_eAggrFunc );
			}
			break;

			default:
			{
				CSphQueryItem & tItem = tQuery.m_dRefItems.Add();
				tItem.m_sExpr = tBucket.m_sCol;
				tItem.m_sAlias = tBucket.m_sCol;
			}
			break;
		}

		// FIXME!!! no need to add count for AggrFunc aggregates
		CSphQueryItem & tAggCountItem = tQuery.m_dRefItems.Add();
		tAggCountItem.m_sExpr = "count(*)";
		tAggCountItem.m_sAlias = "count(*)";

		switch ( tBucket.m_eAggrFunc )
		{
			case Aggr_e::SIGNIFICANT:
			case Aggr_e::HISTOGRAM:
			case Aggr_e::DATE_HISTOGRAM:
			case Aggr_e::RANGE:
			case Aggr_e::DATE_RANGE:
				tQuery.m_sFacetBy = tQuery.m_sGroupBy = GetAggrName ( i, tBucket.m_sCol );
			break;

			// GroupBy \ FacetBy should be empty for explicit grouper
			case Aggr_e::COUNT:
			case Aggr_e::MIN:
			case Aggr_e::MAX:
			case Aggr_e::SUM:
			case Aggr_e::AVG:
				break;

			case Aggr_e::COMPOSITE:
			default:
				tQuery.m_sGroupBy = tBucket.m_sCol;
				tQuery.m_sFacetBy = tBucket.m_sCol;
			break;
		}
		tQuery.m_sOrderBy = "@weight desc";
		if ( tBucket.m_eAggrFunc==Aggr_e::COMPOSITE )
			tQuery.m_eGroupFunc = SPH_GROUPBY_MULTIPLE;

		if ( tBucket.m_sSort.IsEmpty() )
		{
			switch ( tBucket.m_eAggrFunc )
			{
			case Aggr_e::SIGNIFICANT:
			case Aggr_e::HISTOGRAM:
			case Aggr_e::DATE_HISTOGRAM:
			case Aggr_e::RANGE:
			case Aggr_e::DATE_RANGE:
				tQuery.m_sGroupSortBy = "@groupby asc";
				break;
			case Aggr_e::COMPOSITE:
			default:
				tQuery.m_sGroupSortBy = tQuery.m_sOrderBy;
				break;
			}
		} else
		{
			tQuery.m_sGroupSortBy = tBucket.m_sSort;
		}

		// aggregate and main query could have different sizes
		if ( tBucket.m_iSize )
		{
			tQuery.m_iLimit = tBucket.m_iSize;
			tQuery.m_iOffset = 0;
		} else
		{
			tQuery.m_iLimit = iRefLimit;
			tQuery.m_iOffset = iRefOffset;
		}

		tHandler.SetQuery ( i+1, tQuery, nullptr );
	}

	return tHandler;
}
