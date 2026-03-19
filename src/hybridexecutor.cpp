//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "hybridexecutor.h"
#include "queuecreator.h"
#include "attribute.h"
#include "sphinxexpr.h"
#include "coroutine.h"
#include "knnmisc.h"
#include "sphinxquery/xqparser.h"

struct SubQueryResult_t
{
	CSphQueryResultMeta				m_tMeta;
	CSphFixedVector<CSphMatch>		m_dMatches {0};
	int								m_iCount = 0;
	std::unique_ptr<ISphMatchSorter> m_pSorter;
	CSphString						m_sError;
	bool							m_bOk = false;
};


struct RRFEntry_t
{
	DocID_t					m_tDocID = 0;
	float					m_fScore = 0.0f;
	int						m_iTextMatchIdx = -1;		// index into text result set (set 0), or -1 if not present
	CSphVector<int>			m_dKnnMatchIdx;				// per-KNN-set match index (-1 if not present in that set)
};


const char * GetHybridScoreAttrName()
{
	static const char * szName = "@hybrid_score";
	return szName;
}


// Generalized RRF fusion for N result sets.
// Each result set contributes ranks 1..count. Documents appearing in multiple sets
// accumulate score = sum( weight_i / (rank_in_set_i + k) ) across all sets they appear in.
// Weights default to 1.0 when not specified.
static void FuseRRF ( CSphVector<SubQueryResult_t> & dResults, int iRankConstant, const CSphVector<float> & dWeights, CSphVector<RRFEntry_t> & dFused )
{
	int iTotalMatches = 0;
	for ( const auto & tRes : dResults )
		iTotalMatches += tRes.m_iCount;

	OpenHashTable_T<DocID_t, int> hDoc2Idx ( Max ( iTotalMatches, 16 ) );

	int iKnnCount = dResults.GetLength() - 1; // sets 1..N are KNN

	for ( int iSet = 0; iSet < dResults.GetLength(); iSet++ )
	{
		float fWeight = iSet < dWeights.GetLength() ? dWeights[iSet] : 1.0f;
		auto & tRes = dResults[iSet];
		for ( int i = 0; i < tRes.m_iCount; i++ )
		{
			DocID_t tDocID = sphGetDocID ( tRes.m_dMatches[i].m_pDynamic );
			float fContribution = fWeight / ( (i + 1) + iRankConstant );

			int * pIdx = hDoc2Idx.Find ( tDocID );
			if ( pIdx )
			{
				auto & tEntry = dFused[*pIdx];
				tEntry.m_fScore += fContribution;

				if ( iSet==0 )
					tEntry.m_iTextMatchIdx = i;
				else
					tEntry.m_dKnnMatchIdx[iSet - 1] = i;
			}
			else
			{
				int iFusedIdx = dFused.GetLength();
				auto & tEntry = dFused.Add();
				tEntry.m_tDocID = tDocID;
				tEntry.m_fScore = fContribution;
				tEntry.m_dKnnMatchIdx.Resize ( iKnnCount );
				tEntry.m_dKnnMatchIdx.Fill ( -1 );

				if ( iSet==0 )
					tEntry.m_iTextMatchIdx = i;
				else
					tEntry.m_dKnnMatchIdx[iSet - 1] = i;

				hDoc2Idx.Add ( tDocID, iFusedIdx );
			}
		}
	}
}


struct ExprEval_t
{
	CSphAttrLocator		m_tLoc;
	ESphAttr			m_eType;
	ISphExpr *			m_pExpr;
};


static void CollectDependentExprs ( const ISphSchema * pSchema, const char * szAttrName, CSphVector<ExprEval_t> & dExprs )
{
	for ( int i = 0; i < pSchema->GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = pSchema->GetAttr(i);
		if ( !tAttr.m_pExpr || tAttr.m_sName==szAttrName )
			continue;

		StrVec_t dDeps;
		dDeps.Add ( tAttr.m_sName );
		FetchAttrDependencies ( dDeps, *pSchema );
		if ( dDeps.any_of ( [szAttrName] ( const CSphString & s ) { return s==szAttrName; } ) )
			dExprs.Add ( { tAttr.m_tLocator, tAttr.m_eAttrType, tAttr.m_pExpr } );
	}
}


static void CopyAttrs ( CSphMatch & tDstMatch, CSphMatch & tSrcMatch, const ISphSchema * pSrcSchema, const ISphSchema * pDstSchema, const CSphVector<int> & dRemap )
{
	for ( int i = 0; i < pSrcSchema->GetAttrsCount(); i++ )
	{
		int iDst = dRemap[i];
		if ( iDst < 0 )
			continue;

		const CSphAttrLocator & tSrcLoc = pSrcSchema->GetAttr(i).m_tLocator;
		const CSphAttrLocator & tDstLoc = pDstSchema->GetAttr(iDst).m_tLocator;

		// for ptr attrs, free existing dst value (if any) before overwriting, then null out source
		if ( sphIsDataPtrAttr ( pSrcSchema->GetAttr(i).m_eAttrType ) )
		{
			sphDeallocatePacked ( (BYTE*)tDstMatch.GetAttr ( tDstLoc ) );
			tDstMatch.SetAttr ( tDstLoc, tSrcMatch.GetAttr ( tSrcLoc ) );
			tSrcMatch.SetAttr ( tSrcLoc, 0 );
		}
		else
			tDstMatch.SetAttr ( tDstLoc, tSrcMatch.GetAttr ( tSrcLoc ) );
	}
}


static void EvalDependentExprs ( const CSphVector<ExprEval_t> & dExprs, CSphMatch & tMatch )
{
	for ( auto & tExpr : dExprs )
	{
		switch ( tExpr.m_eType )
		{
		case SPH_ATTR_FLOAT:	tMatch.SetAttrFloat ( tExpr.m_tLoc, tExpr.m_pExpr->Eval ( tMatch ) ); break;
		case SPH_ATTR_BIGINT:	tMatch.SetAttr ( tExpr.m_tLoc, tExpr.m_pExpr->Int64Eval ( tMatch ) ); break;
		default:				tMatch.SetAttr ( tExpr.m_tLoc, tExpr.m_pExpr->IntEval ( tMatch ) ); break;
		}
	}
}


static void RemoveQueryItems ( CSphVector<CSphQueryItem> & dItems, std::initializer_list<const char *> dNames )
{
	ARRAY_FOREACH ( i, dItems )
	{
		for ( auto * szName : dNames )
			if ( dItems[i].m_sExpr==szName )
			{
				dItems.Remove(i);
				--i;
				break;
			}
	}
}


static void BuildAttrRemap ( const ISphSchema * pSrcSchema, const ISphSchema * pDstSchema, CSphVector<int> & dRemap )
{
	dRemap.Resize ( pSrcSchema->GetAttrsCount() );
	for ( int i = 0; i < pSrcSchema->GetAttrsCount(); i++ )
		dRemap[i] = pDstSchema->GetAttrIndex ( pSrcSchema->GetAttr(i).m_sName.cstr() );
}


////////////////////////////////////////////////////////////////////////////////

class HybridExecutor_c
{
public:
			HybridExecutor_c ( const CSphIndex * pIndex, const CSphQuery & tQuery, const SphQueueSettings_t & tQueueSettings );

	bool	Execute ( CSphQueryResult & tResult, const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphMultiQueryArgs & tArgs );

private:
	void	SetupKnnQueries ( const CSphQuery & tQuery );
	void	SetupTextQuery ( const CSphQuery & tQuery );
	void	SetupSubQueryLimits();
	bool	ResolveKnnAttrs();
	bool	RunSubQuery ( const CSphQuery & tQuery, const CSphMultiQueryArgs & tArgs, const char * szPhase, SubQueryResult_t & tResult );
	void	PushFusedMatches ( ISphMatchSorter * pSorter, const CSphVector<RRFEntry_t> & dFused );
	void	SetMinKnnDist ( CSphMatch & tMatch, const RRFEntry_t & tEntry, const CSphColumnInfo * pDstKnnDist, const CSphVector<const CSphColumnInfo *> & dKnnDistAttrs );
	void	PushSingleFusedMatch ( const RRFEntry_t & tEntry, ISphMatchSorter * pSorter, int iDynSize, const CSphVector<CSphVector<int>> & dRemaps, const CSphAttrLocator & tScoreLoc, const CSphColumnInfo * pDstKnnDist, const CSphVector<const CSphColumnInfo *> & dKnnDistAttrs, const CSphVector<ExprEval_t> & dExprs );
	void	ResolveWeights ( const CSphQuery & tQuery );
	int		ResolveSubQueryIdx ( const CSphQuery & tQuery, const CSphString & sName ) const;

	const CSphIndex *			m_pIndex;
	CSphVector<CSphQuery>		m_dKnnQueries;		///< one per KNN entry
	CSphQuery					m_tTextQuery;
	HybridSearchSettings_t		m_tHybridSettings;
	CSphVector<float>			m_dWeights;			///< positional RRF weights resolved from m_tHybridSettings.m_dNamedWeights
	CSphVector<SubQueryResult_t>	m_dSubResults;	///< [0]=text, [1..N]=KNN sub-query results
	const SphQueueSettings_t &	m_tQueueSettings;
	CSphString					m_sError;
	std::unique_ptr<QueryParser_i>	m_pTextQueryParser;		///< plain parser for text sub-query (used when original is JSON)
};


HybridExecutor_c::HybridExecutor_c ( const CSphIndex * pIndex, const CSphQuery & tQuery, const SphQueueSettings_t & tQueueSettings )
	: m_pIndex ( pIndex )
	, m_tTextQuery ( tQuery )
	, m_tHybridSettings ( tQuery.m_tHybridSettings )
	, m_tQueueSettings ( tQueueSettings )
{
	SetupKnnQueries(tQuery);
	SetupTextQuery(tQuery);
	ResolveKnnAttrs();
	SetupSubQueryLimits();
	ResolveWeights(tQuery);
}


void HybridExecutor_c::SetupKnnQueries ( const CSphQuery & tQuery )
{
	// create one sub-query per KNN entry
	for ( int i = 0; i < tQuery.m_dKnnSettings.GetLength(); i++ )
	{
		m_dKnnQueries.Add ( tQuery );
		auto & tKnnQuery = m_dKnnQueries.Last();
		tKnnQuery.m_sQuery = "";
		tKnnQuery.m_sRawQuery = "";
		tKnnQuery.m_bHybridSearch = false;
		tKnnQuery.m_eSort = SPH_SORT_EXTENDED;
		tKnnQuery.m_sSortBy = "@weight desc";
		RemoveQueryItems ( tKnnQuery.m_dItems, { "hybrid_score()", GetHybridScoreAttrName() } );

		// each KNN sub-query gets exactly one KNN entry
		tKnnQuery.m_dKnnSettings.Reset();
		tKnnQuery.m_dKnnSettings.Add ( tQuery.m_dKnnSettings[i] );
	}
}


void HybridExecutor_c::SetupTextQuery ( const CSphQuery & tQuery )
{
	m_tTextQuery.m_dKnnSettings.Reset();
	m_tTextQuery.m_bHybridSearch = false;
	m_tTextQuery.m_eSort = SPH_SORT_EXTENDED;
	m_tTextQuery.m_sSortBy = "@weight desc";
	RemoveQueryItems ( m_tTextQuery.m_dItems, { "hybrid_score()", GetHybridScoreAttrName(), GetKnnDistAttrName(), "knn_dist()" } );

	// when the original query uses a JSON parser but m_sQuery contains plain text
	// (e.g. from "hybrid" shorthand), the JSON parser treats it as fullscan because
	// it has no JSON keywords like "match". Switch to a plain parser so the text
	// is parsed as an extended full-text query. Don't switch when m_sQuery contains
	// actual JSON (explicit path) - the JSON parser handles that correctly.
	if ( m_tTextQuery.m_pQueryParser && m_tTextQuery.m_pQueryParser->IsFullscan ( m_tTextQuery ) && !m_tTextQuery.m_sQuery.IsEmpty() )
	{
		m_pTextQueryParser = sphCreatePlainQueryParser();
		m_tTextQuery.m_pQueryParser = m_pTextQueryParser.get();
	}
}


bool HybridExecutor_c::ResolveKnnAttrs()
{
	for ( auto & tKnnQuery : m_dKnnQueries )
	{
		auto & tKNN = tKnnQuery.SingleKnnSettings();
		if ( !tKNN.m_sAttr.IsEmpty() )
			continue;

		// no attr specified — find the single float_vector with auto-embeddings
		const CSphSchema & tSchema = m_pIndex->GetMatchSchema();
		CSphString sFound;
		int iFound = 0;

		for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
			if ( tAttr.m_eAttrType==SPH_ATTR_FLOAT_VECTOR && tAttr.IsIndexedKNN() && !tAttr.m_tKNNModel.m_sModelName.empty() )
			{
				sFound = tAttr.m_sName;
				iFound++;
			}
		}

		if ( iFound==0 )
		{
			m_sError = "hybrid search requires a float_vector attribute with auto-embeddings, but none found in the schema";
			return false;
		}

		if ( iFound>1 )
		{
			m_sError = "hybrid search requires specifying a vector attribute because the table has multiple float_vector attributes with auto-embeddings";
			return false;
		}

		tKNN.m_sAttr = sFound;
	}

	return true;
}


void HybridExecutor_c::SetupSubQueryLimits()
{
	int64_t iMaxKnnDocs = 0;
	for ( const auto & tKnnQuery : m_dKnnQueries )
		iMaxKnnDocs = Max ( iMaxKnnDocs, tKnnQuery.SingleKnnSettings().GetRequestedDocs() );

	int iWindow = m_tHybridSettings.m_iWindowSize > 0 ? m_tHybridSettings.m_iWindowSize : Max ( iMaxKnnDocs, (int64_t)m_tTextQuery.m_iLimit );
	for ( auto & tKnnQuery : m_dKnnQueries )
		tKnnQuery.m_iLimit = iWindow;

	m_tTextQuery.m_iLimit = iWindow;
}


bool HybridExecutor_c::RunSubQuery ( const CSphQuery & tQuery, const CSphMultiQueryArgs & tArgs, const char * szPhase, SubQueryResult_t & tSubResult )
{
	SphQueueRes_t tQueueRes;
	CSphString sError;
	tSubResult.m_pSorter.reset ( sphCreateQueue ( m_tQueueSettings, tQuery, sError, tQueueRes ) );
	if ( !tSubResult.m_pSorter )
	{
		tSubResult.m_sError.SetSprintf ( "hybrid %s phase: %s", szPhase, sError.cstr() );
		return false;
	}

	CSphQueryResult tResult;
	tResult.m_pMeta = &tSubResult.m_tMeta;

	CSphMultiQueryArgs tSubArgs ( tArgs.m_iIndexWeight );
	tSubArgs.m_uPackedFactorFlags = tArgs.m_uPackedFactorFlags;
	tSubArgs.m_bLocalDF = tArgs.m_bLocalDF;
	tSubArgs.m_pLocalDocs = tArgs.m_pLocalDocs;
	tSubArgs.m_iTotalDocs = tArgs.m_iTotalDocs;
	tSubArgs.m_iThreads = tArgs.m_iThreads;
	tSubArgs.m_iTotalThreads = tArgs.m_iTotalThreads;

	ISphMatchSorter * pSorterRaw = tSubResult.m_pSorter.get();
	if ( !m_pIndex->MultiQuery ( tResult, tQuery, { &pSorterRaw, 1 }, tSubArgs ) )
	{
		tSubResult.m_sError.SetSprintf ( "hybrid %s phase: %s", szPhase, tSubResult.m_tMeta.m_sError.cstr() );
		return false;
	}

	tSubResult.m_dMatches.Reset ( tSubResult.m_pSorter->GetLength() );
	if ( tSubResult.m_dMatches.GetLength() )
		tSubResult.m_iCount = tSubResult.m_pSorter->Flatten ( tSubResult.m_dMatches.Begin() );

	return true;
}


void HybridExecutor_c::SetMinKnnDist ( CSphMatch & tMatch, const RRFEntry_t & tEntry, const CSphColumnInfo * pDstKnnDist, const CSphVector<const CSphColumnInfo *> & dKnnDistAttrs )
{
	float fMinDist = FLT_MAX;
	for ( int i = 0; i < tEntry.m_dKnnMatchIdx.GetLength(); i++ )
	{
		if ( tEntry.m_dKnnMatchIdx[i] < 0 )
			continue;

		const CSphColumnInfo * pSrcKnnDist = dKnnDistAttrs[i];
		if ( pSrcKnnDist )
		{
			float fDist = m_dSubResults[i + 1].m_dMatches[tEntry.m_dKnnMatchIdx[i]].GetAttrFloat ( pSrcKnnDist->m_tLocator );
			fMinDist = Min ( fMinDist, fDist );
		}
	}

	if ( fMinDist < FLT_MAX )
		tMatch.SetAttrFloat ( pDstKnnDist->m_tLocator, fMinDist );
}


void HybridExecutor_c::PushSingleFusedMatch ( const RRFEntry_t & tEntry, ISphMatchSorter * pSorter, int iDynSize, const CSphVector<CSphVector<int>> & dRemaps, const CSphAttrLocator & tScoreLoc, const CSphColumnInfo * pDstKnnDist, const CSphVector<const CSphColumnInfo *> & dKnnDistAttrs, const CSphVector<ExprEval_t> & dExprs )
{
	const ISphSchema * pDstSchema = pSorter->GetSchema();

	CSphMatch tNewMatch;
	tNewMatch.Reset ( iDynSize );
	tNewMatch.m_pStatic = nullptr;

	// pick the best source for attribute copying: prefer first KNN set the doc appears in (richer schema), fall back to text
	int iBestSet = -1;
	int iBestMatch = -1;

	for ( int i = 0; i < tEntry.m_dKnnMatchIdx.GetLength(); i++ )
	{
		if ( tEntry.m_dKnnMatchIdx[i] >= 0 )
		{
			iBestSet = i + 1; // KNN sets are 1..N in m_dSubResults
			iBestMatch = tEntry.m_dKnnMatchIdx[i];
			break;
		}
	}

	if ( iBestSet < 0 && tEntry.m_iTextMatchIdx >= 0 )
	{
		iBestSet = 0;
		iBestMatch = tEntry.m_iTextMatchIdx;
	}

	if ( iBestSet >= 0 )
	{
		auto & tSrcResult = m_dSubResults[iBestSet];
		CSphMatch & tSrcMatch = tSrcResult.m_dMatches[iBestMatch];

		tNewMatch.m_tRowID = tSrcMatch.m_tRowID;
		tNewMatch.m_iWeight = tSrcMatch.m_iWeight;
		tNewMatch.m_iTag = tSrcMatch.m_iTag;
		CopyAttrs ( tNewMatch, tSrcMatch, tSrcResult.m_pSorter->GetSchema(), pDstSchema, dRemaps[iBestSet] );
	}

	// copy weight from text rset
	if ( tEntry.m_iTextMatchIdx>=0 )
		tNewMatch.m_iWeight = m_dSubResults[0].m_dMatches[tEntry.m_iTextMatchIdx].m_iWeight;

	// set the hybrid score
	tNewMatch.SetAttrFloat ( tScoreLoc, tEntry.m_fScore );

	if ( pDstKnnDist )
		SetMinKnnDist ( tNewMatch, tEntry, pDstKnnDist, dKnnDistAttrs );

	EvalDependentExprs ( dExprs, tNewMatch );

	pSorter->Push ( tNewMatch );
}


void HybridExecutor_c::PushFusedMatches ( ISphMatchSorter * pSorter, const CSphVector<RRFEntry_t> & dFused )
{
	const ISphSchema * pDstSchema = pSorter->GetSchema();
	int iDynSize = pDstSchema->GetDynamicSize();

	// build remaps for all result schemas
	CSphVector<CSphVector<int>> dRemaps ( m_dSubResults.GetLength() );
	for ( int i = 0; i < m_dSubResults.GetLength(); i++ )
	{
		if ( m_dSubResults[i].m_pSorter )
			BuildAttrRemap ( m_dSubResults[i].m_pSorter->GetSchema(), pDstSchema, dRemaps[i] );
	}

	const CSphColumnInfo * pScoreAttr = pDstSchema->GetAttr ( GetHybridScoreAttrName() );
	CSphAttrLocator tScoreLoc = pScoreAttr->m_tLocator;

	const CSphColumnInfo * pDstKnnDist = pDstSchema->GetAttr ( GetKnnDistAttrName() );

	// precalculate per-KNN-set knn_dist attr pointers (schemas are constant across all entries)
	CSphVector<const CSphColumnInfo *> dKnnDistAttrs ( m_dSubResults.GetLength() - 1 );
	for ( int i = 0; i < dKnnDistAttrs.GetLength(); i++ )
	{
		if ( m_dSubResults[i + 1].m_pSorter )
			dKnnDistAttrs[i] = m_dSubResults[i + 1].m_pSorter->GetSchema()->GetAttr ( GetKnnDistAttrName() );
	}

	CSphVector<ExprEval_t> dExprs;
	CollectDependentExprs ( pDstSchema, GetHybridScoreAttrName(), dExprs );

	for ( auto & tEntry : dFused )
		PushSingleFusedMatch ( tEntry, pSorter, iDynSize, dRemaps, tScoreLoc, pDstKnnDist, dKnnDistAttrs, dExprs );
}


int HybridExecutor_c::ResolveSubQueryIdx ( const CSphQuery & tQuery, const CSphString & sName ) const
{
	// in JSON, "query" is a fixed alias for the fulltext sub-query;
	// in SQL, there is no default - an explicit AS on MATCH() is required
	CSphString sMatchAlias = tQuery.m_tHybridSettings.m_sMatchAlias;
	if ( sMatchAlias.IsEmpty() && tQuery.m_eQueryType==QUERY_JSON )
		sMatchAlias = "query";

	if ( !sMatchAlias.IsEmpty() && sName==sMatchAlias )
		return 0;

	// check KNN aliases (indices 1..N); requires an explicit alias set via "name" (JSON) or AS (SQL)
	for ( int i = 0; i < tQuery.m_dKnnSettings.GetLength(); i++ )
	{
		const auto & tKNN = tQuery.m_dKnnSettings[i];
		if ( !tKNN.m_sAlias.IsEmpty() && sName==tKNN.m_sAlias )
			return i + 1;
	}

	return -1;
}


void HybridExecutor_c::ResolveWeights ( const CSphQuery & tQuery )
{
	auto & dNamed = m_tHybridSettings.m_dNamedWeights;
	if ( dNamed.IsEmpty() )
		return;

	// positional layout: [0] = text, [1..N] = KNN sub-queries
	int iTotal = 1 + m_dKnnQueries.GetLength();
	m_dWeights.Resize ( iTotal );
	m_dWeights.Fill ( 1.0f );

	// in JSON, "query" is a fixed alias for the fulltext sub-query;
	// verify no KNN entry uses "query" as its explicit alias (would collide)
	if ( tQuery.m_eQueryType==QUERY_JSON )
		for ( int i = 0; i < tQuery.m_dKnnSettings.GetLength(); i++ )
			if ( tQuery.m_dKnnSettings[i].m_sAlias=="query" )
			{
				m_sError = "fusion_weights: KNN alias 'query' conflicts with the fixed fulltext query alias in JSON";
				return;
			}

	for ( const auto & tNamed : dNamed )
	{
		int iIdx = ResolveSubQueryIdx ( tQuery, tNamed.m_sName );

		if ( iIdx < 0 )
		{
			m_sError.SetSprintf ( "fusion_weights: unknown alias '%s'; use explicit aliases to reference sub-queries", tNamed.m_sName.cstr() );
			return;
		}

		if ( tNamed.m_fWeight < 0.0f )
		{
			m_sError.SetSprintf ( "fusion_weights: weight for '%s' must be non-negative", tNamed.m_sName.cstr() );
			return;
		}

		m_dWeights[iIdx] = tNamed.m_fWeight;
	}
}


bool HybridExecutor_c::Execute ( CSphQueryResult & tResult, const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphMultiQueryArgs & tArgs )
{
	auto & tMeta = *tResult.m_pMeta;

	if ( !m_sError.IsEmpty() )
	{
		tMeta.m_sError = m_sError;
		return false;
	}

	if ( dSorters.GetLength()!=1 || !dSorters[0] )
	{
		tMeta.m_sError = "hybrid search does not support multiple sorters";
		return false;
	}

	ISphMatchSorter * pOutputSorter = dSorters[0];
	int iTotalSubQueries = m_dKnnQueries.GetLength() + 1;
	m_dSubResults.Resize ( iTotalSubQueries ); // [0] = text, [1..N] = KNN sub-queries

	std::atomic<int> iJob { 0 };
	Threads::Coro::ExecuteN ( iTotalSubQueries, [&]
	{
		int iMyJob = iJob.fetch_add ( 1, std::memory_order_acq_rel );
		if ( iMyJob==0 )
		{
			// skip text sub-query when there is no actual text (KNN-only fusion);
			// running an empty query would be a fullscan that pollutes RRF scores
			if ( m_tTextQuery.m_sQuery.IsEmpty() )
				m_dSubResults[0].m_bOk = true;
			else
				m_dSubResults[0].m_bOk = RunSubQuery ( m_tTextQuery, tArgs, "text", m_dSubResults[0] );
		}
		else
		{
			CSphString sPhase;
			sPhase.SetSprintf ( "KNN-%d", iMyJob );
			m_dSubResults[iMyJob].m_bOk = RunSubQuery ( m_dKnnQueries[iMyJob-1], tArgs, sPhase.cstr(), m_dSubResults[iMyJob] );
		}
	} );


	for ( auto & i : m_dSubResults )
		if ( !i.m_bOk )
		{
			tMeta.m_sError = i.m_sError;
			return false;
		}

	// move the output sorter to standalone schema (it's empty, so this just transforms the schema)
	pOutputSorter->TransformPooled2StandalonePtrs ( [] ( const CSphMatch * ) -> const BYTE * { return nullptr; }, [] ( const CSphMatch * ) -> columnar::Columnar_i * { return nullptr; }, false );

	CSphVector<RRFEntry_t> dFused;
	FuseRRF ( m_dSubResults, m_tHybridSettings.m_iRankConstant, m_dWeights, dFused );
	PushFusedMatches ( pOutputSorter, dFused );

	for ( int i = 0; i < iTotalSubQueries; i++ )
		tMeta.MergeWordStats ( m_dSubResults[i].m_tMeta );

	// propagate docstore from the index so postlimit expressions (stored fields) work
	tResult.m_pDocstore = m_pIndex;

	return true;
}


bool ExecuteHybridSearch ( const CSphIndex * pIndex, const CSphQuery & tQuery, const SphQueueSettings_t & tQueueSettings, CSphQueryResult & tResult, const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphMultiQueryArgs & tArgs )
{
	HybridExecutor_c tHybrid ( pIndex, tQuery, tQueueSettings );
	return tHybrid.Execute ( tResult, dSorters, tArgs );
}
