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

#include "api_commands.h"
#include "api_search.h"

#include "searchdaemon.h"
#include "search_handler.h"
#include "logger.h"

/////////////////////////////////////////////////////////////////////////////

/// qflag means Query Flag
/// names are internal to searchd and may be changed for clarity
/// values are communicated over network between searchds and APIs and MUST NOT CHANGE
enum
{
	QFLAG_REVERSE_SCAN			= 1UL << 0,		// deprecated
	QFLAG_SORT_KBUFFER			= 1UL << 1,
	QFLAG_MAX_PREDICTED_TIME	= 1UL << 2,
	QFLAG_SIMPLIFY				= 1UL << 3,
	QFLAG_PLAIN_IDF				= 1UL << 4,
	QFLAG_GLOBAL_IDF			= 1UL << 5,
	QFLAG_NORMALIZED_TF			= 1UL << 6,
	QFLAG_LOCAL_DF				= 1UL << 7,
	QFLAG_LOW_PRIORITY			= 1UL << 8,
	QFLAG_FACET					= 1UL << 9,
	QFLAG_FACET_HEAD			= 1UL << 10,
	QFLAG_JSON_QUERY			= 1UL << 11,
	QFLAG_NOT_ONLY_ALLOWED		= 1UL << 12,
	QFLAG_LOCAL_DF_SET			= 1UL << 13,
	QFLAG_SIMPLIFY_SET			= 1UL << 14
};

void operator<< ( ISphOutputBuffer & tOut, const CSphNamedInt & tValue )
{
	tOut.SendString ( tValue.first.cstr () );
	tOut.SendInt ( tValue.second );
}

void operator>> ( InputBuffer_c & dIn, CSphNamedInt & tValue )
{
	tValue.first = dIn.GetString ();
	tValue.second = dIn.GetInt ();
}

void SearchRequestBuilder_c::SendQuery ( const char * sIndexes, ISphOutputBuffer & tOut, const CSphQuery & q, int iWeight ) const
{
	bool bAgentWeight = ( iWeight!=-1 );
	// starting with command version 1.27, flags go first
	// reason being, i might add flags that affect *any* of the subsequent data (eg. qflag_pack_ints)
	DWORD uFlags = 0;
	uFlags |= QFLAG_SORT_KBUFFER * q.m_bSortKbuffer;
	uFlags |= QFLAG_MAX_PREDICTED_TIME * ( q.m_iMaxPredictedMsec > 0 );
	uFlags |= QFLAG_SIMPLIFY * q.m_bSimplify.value_or ( CSphQuery::m_bDefaultSimplify );
	uFlags |= QFLAG_PLAIN_IDF * q.m_bPlainIDF;
	uFlags |= QFLAG_GLOBAL_IDF * q.m_bGlobalIDF;
	uFlags |= QFLAG_NORMALIZED_TF * q.m_bNormalizedTFIDF;
	uFlags |= QFLAG_LOCAL_DF * q.m_bLocalDF.value_or ( false );
	uFlags |= QFLAG_LOW_PRIORITY * q.m_bLowPriority;
	uFlags |= QFLAG_FACET * q.m_bFacet;
	uFlags |= QFLAG_FACET_HEAD * q.m_bFacetHead;
	uFlags |= QFLAG_NOT_ONLY_ALLOWED * q.m_bNotOnlyAllowed;
	uFlags |= QFLAG_LOCAL_DF_SET * q.m_bLocalDF.has_value();
	uFlags |= QFLAG_SIMPLIFY_SET * q.m_bSimplify.has_value();

	if ( q.m_eQueryType==QUERY_JSON )
		uFlags |= QFLAG_JSON_QUERY;

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

	if ( q.m_eQueryType==QUERY_JSON )
		tOut.SendString ( q.m_sQuery.cstr() );
	else
	{
		if ( q.m_sRawQuery.IsEmpty() )
			tOut.SendString ( q.m_sQuery.cstr() );
		else
			tOut.SendString ( q.m_sRawQuery.cstr() ); // query
	}

	tOut.SendInt ( q.m_dWeights.GetLength() );
	ARRAY_FOREACH ( j, q.m_dWeights )
		tOut.SendInt ( q.m_dWeights[j] ); // weights
	tOut.SendString ( sIndexes ); // indexes
	tOut.SendInt ( 1 ); // id range bits
	tOut.SendUint64 ( uint64_t(0) ); // default full id range (any client range must be in filters at this stage)
	tOut.SendUint64 ( UINT64_MAX );
	tOut.SendInt ( q.m_dFilters.GetLength() );
	ARRAY_FOREACH ( j, q.m_dFilters )
	{
		const CSphFilterSettings & tFilter = q.m_dFilters[j];
		tOut.SendString ( tFilter.m_sAttrName.cstr() );
		tOut.SendInt ( tFilter.m_eType );
		switch ( tFilter.m_eType )
		{
			case SPH_FILTER_VALUES:
				tOut.SendInt ( tFilter.GetNumValues() );
				for ( auto uValue : tFilter.GetValues () )
					tOut.SendUint64 ( uValue );
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
				tOut.SendString ( tFilter.m_dStrings.GetLength()==1 ? tFilter.m_dStrings[0].cstr() : nullptr );
				tOut.SendByte ( (BYTE)tFilter.m_eStrCmpDir );
				break;

			case SPH_FILTER_NULL:
				tOut.SendByte ( tFilter.m_bIsNull );
				break;

			case SPH_FILTER_STRING_LIST:
				tOut.SendInt ( tFilter.m_dStrings.GetLength() );
				ARRAY_FOREACH ( iString, tFilter.m_dStrings )
					tOut.SendString ( tFilter.m_dStrings[iString].cstr() );
				break;
			case SPH_FILTER_EXPRESSION: // need only name and type
				break;
		}
		tOut.SendInt ( tFilter.m_bExclude );
		tOut.SendInt ( tFilter.m_bHasEqualMin );
		tOut.SendInt ( tFilter.m_bHasEqualMax );
		tOut.SendInt ( tFilter.m_bOpenLeft );
		tOut.SendInt ( tFilter.m_bOpenRight );
		tOut.SendInt ( tFilter.m_eMvaFunc );
	}
	tOut.SendInt ( q.m_eGroupFunc );
	tOut.SendString ( q.m_sGroupBy.cstr() );
	if ( m_iDivideLimits==1 )
		tOut.SendInt ( q.m_iMaxMatches );
	else
		tOut.SendInt ( 1+(q.m_iMaxMatches/m_iDivideLimits) ); // Reduce the max_matches also.
	tOut.SendString ( q.m_sGroupSortBy.cstr() );
	tOut.SendInt ( q.m_iCutoff );
	tOut.SendInt ( q.m_iRetryCount<0 ? 0 : q.m_iRetryCount ); // runaround for old clients.
	tOut.SendInt ( q.m_iRetryDelay<0 ? 0 : q.m_iRetryDelay );
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
		for ( const auto& dWeight : q.m_dIndexWeights )
			tOut << dWeight;
	}
	tOut.SendDword ( q.m_uMaxQueryMsec );
	tOut.SendInt ( q.m_dFieldWeights.GetLength() );
	for ( const auto & dWeight : q.m_dFieldWeights )
		tOut << dWeight;

	tOut.SendString ( q.m_sComment.cstr() );
	tOut.SendInt ( 0 ); // WAS: overrides
	tOut.SendString ( q.m_sSelect.cstr() );
	if ( q.m_iMaxPredictedMsec>0 )
		tOut.SendInt ( q.m_iMaxPredictedMsec );

	// emulate empty sud-select for agent (client ver 1.29) as master sends fixed outer offset+limits
	tOut.SendString ( NULL );
	tOut.SendInt ( 0 );
	tOut.SendInt ( 0 );
	tOut.SendInt ( q.m_bHasOuter );
	// v.1.36
	tOut.SendInt ( q.m_iExpansionLimit );

	// master-agent extensions
	tOut.SendDword ( q.m_eCollation ); // v.1
	tOut.SendString ( q.m_sOuterOrderBy.cstr() ); // v.2
	if ( q.m_bHasOuter )
		tOut.SendInt ( q.m_iOuterOffset + q.m_iOuterLimit );
	tOut.SendInt ( q.m_iGroupbyLimit );
	tOut.SendString ( q.m_sUDRanker.cstr() );
	tOut.SendString ( q.m_sUDRankerOpts.cstr() );
	tOut.SendString ( q.m_sQueryTokenFilterLib.cstr() );
	tOut.SendString ( q.m_sQueryTokenFilterName.cstr() );
	tOut.SendString ( q.m_sQueryTokenFilterOpts.cstr() );
	tOut.SendInt ( q.m_dFilterTree.GetLength() );
	ARRAY_FOREACH ( i, q.m_dFilterTree )
	{
		tOut.SendInt ( q.m_dFilterTree[i].m_iLeft );
		tOut.SendInt ( q.m_dFilterTree[i].m_iRight );
		tOut.SendInt ( q.m_dFilterTree[i].m_iFilterItem );
		tOut.SendInt ( q.m_dFilterTree[i].m_bOr );
	}
	tOut.SendInt( q.m_dItems.GetLength() );
	ARRAY_FOREACH ( i, q.m_dItems )
	{
		const CSphQueryItem & tItem = q.m_dItems[i];
		tOut.SendString ( tItem.m_sAlias.cstr() );
		tOut.SendString ( tItem.m_sExpr.cstr() );
		tOut.SendDword ( tItem.m_eAggrFunc );
	}
	tOut.SendInt( q.m_dRefItems.GetLength() );
	ARRAY_FOREACH ( i, q.m_dRefItems )
	{
		const CSphQueryItem & tItem = q.m_dRefItems[i];
		tOut.SendString ( tItem.m_sAlias.cstr() );
		tOut.SendString ( tItem.m_sExpr.cstr() );
		tOut.SendDword ( tItem.m_eAggrFunc );
	}
	tOut.SendDword ( q.m_eExpandKeywords );

	tOut.SendInt ( q.m_dIndexHints.GetLength() );
	for ( const auto & i : q.m_dIndexHints )
	{
		tOut.SendString ( i.m_sIndex.cstr() );
		tOut.SendDword ( (DWORD)i.m_eType );
		tOut.SendDword ( (DWORD)i.m_bForce );
	}

	tOut.SendInt ( (int)q.m_eJoinType );
	tOut.SendString ( q.m_sJoinIdx.cstr() );
	tOut.SendString ( q.m_sJoinQuery.cstr() );
	tOut.SendInt ( q.m_dOnFilters.GetLength() );
	for ( const auto & i : q.m_dOnFilters )
	{
		tOut.SendString ( i.m_sIdx1.cstr() );
		tOut.SendString ( i.m_sAttr1.cstr() );
		tOut.SendString ( i.m_sIdx2.cstr() );
		tOut.SendString ( i.m_sAttr2.cstr() );
		tOut.SendInt ( (int)i.m_eTypeCast1 );
		tOut.SendInt ( (int)i.m_eTypeCast2 );
	}

	const auto & tKNN = q.m_tKnnSettings;
	tOut.SendString ( tKNN.m_sAttr.cstr() );
	if ( !tKNN.m_sAttr.IsEmpty() )
	{
		tOut.SendInt ( tKNN.m_iK );
		tOut.SendInt ( tKNN.m_iEf );
		tOut.SendInt ( tKNN.m_bRescore );
		tOut.SendFloat ( tKNN.m_fOversampling );
		tOut.SendInt ( tKNN.m_bPrefilter );
		tOut.SendInt ( tKNN.m_bFullscan );
		tOut.SendInt ( tKNN.m_dVec.GetLength() );
		for ( const auto & i : tKNN.m_dVec )
			tOut.SendFloat(i);

		tOut.SendByte ( !!tKNN.m_sEmbStr );
		if ( tKNN.m_sEmbStr )
			tOut.SendString ( tKNN.m_sEmbStr->cstr() );
	}

	tOut.SendInt ( (int)q.m_eJiebaMode );

	tOut.SendString ( q.m_tScrollSettings.m_sSortBy.cstr() );
	tOut.SendInt ( q.m_tScrollSettings.m_bRequested );
	tOut.SendInt ( q.m_tScrollSettings.m_dAttrs.GetLength() );
	for ( const auto & i : q.m_tScrollSettings.m_dAttrs )
	{
		tOut.SendString ( i.m_sSortAttr.cstr() );
		tOut.SendInt ( i.m_bDesc );
		tOut.SendInt ( (int)i.m_eType );
		tOut.SendUint64 ( i.m_tValue );
		tOut.SendFloat ( i.m_fValue );
		tOut.SendString ( i.m_sValue.cstr() );
	}

	tOut.SendString ( q.m_sExpandBlended.cstr() );
}


void SearchRequestBuilder_c::BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const
{
	auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_SEARCH, VER_COMMAND_SEARCH ); // API header

	tOut.SendInt ( VER_COMMAND_SEARCH_MASTER );
	tOut.SendInt ( m_dQueries.GetLength() );
	for ( auto& dQuery : m_dQueries )
		SendQuery ( tAgent.m_tDesc.m_sIndexes.cstr (), tOut, dQuery, tAgent.m_iWeight );
}


void cSearchResult::Reset ()
{
	m_dResults.Reset();
}

bool cSearchResult::HasWarnings () const
{
	return m_dResults.any_of ( [] ( const AggrResult_t &dRes ) { return !dRes.m_sWarning.IsEmpty (); } );
}

/////////////////////////////////////////////////////////////////////////////

static void ParseMatch ( CSphMatch & tMatch, MemInputBuffer_c & tReq, const CSphSchema & tSchema, bool bAgent64 )
{
	tMatch.Reset ( tSchema.GetRowSize() );

	// WAS: docids
	if ( bAgent64 )
		tReq.GetUint64();
	else
		tReq.GetDword();

	tMatch.m_iWeight = tReq.GetInt ();
	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);

		assert ( sphPlainAttrToPtrAttr(tAttr.m_eAttrType)==tAttr.m_eAttrType );

		switch ( tAttr.m_eAttrType )
		{
		case SPH_ATTR_UINT32SET_PTR:
		case SPH_ATTR_INT64SET_PTR:
			{
				int iValues = tReq.GetDword ();
				BYTE * pData = nullptr;
				BYTE * pPacked = sphPackPtrAttr ( iValues*sizeof(DWORD), &pData );
				tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t)pPacked );
				auto * pMVA = (DWORD *)pData;
				if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET_PTR )
				{
					while ( iValues-- )
						sphUnalignedWrite ( pMVA++, tReq.GetDword() );
				} else
				{
					assert ( ( iValues%2 )==0 );
					for ( ; iValues; iValues -= 2 )
					{
						uint64_t uMva = tReq.GetUint64();
						sphUnalignedWrite ( pMVA, uMva );
						pMVA += 2;
					}
				}
			}
			break;

		case SPH_ATTR_FLOAT_VECTOR_PTR:
		{
			int iValues = tReq.GetDword ();
			BYTE * pData = nullptr;
			BYTE * pPacked = sphPackPtrAttr ( iValues*sizeof(DWORD), &pData );
			tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t)pPacked );
			auto * pFloatVec = (float *)pData;
			while ( iValues-- )
				sphUnalignedWrite ( pFloatVec++, tReq.GetFloat() );
		}
		break;

		case SPH_ATTR_STRINGPTR:
		case SPH_ATTR_JSON_PTR:
		case SPH_ATTR_FACTORS:
		case SPH_ATTR_FACTORS_JSON:
			{
				int iLen = tReq.GetDword();
				BYTE * pData = nullptr;
				if (iLen)
				{
					tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t)sphPackPtrAttr ( iLen, &pData ) );
					tReq.GetBytes ( pData, iLen );
				} else
					tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t) 0 );
			}
			break;

		case SPH_ATTR_JSON_FIELD_PTR:
			{
				// FIXME: no reason for json_field to be any different from other *_PTR attributes
				auto eJson = (ESphJsonType)tReq.GetByte();
				if ( eJson==JSON_EOF )
					tMatch.SetAttr ( tAttr.m_tLocator, 0 );
				else
				{
					int iLen = tReq.GetDword();
					BYTE * pData = nullptr;
					tMatch.SetAttr ( tAttr.m_tLocator, (SphAttr_t)sphPackPtrAttr ( iLen+1, &pData ) );
					*pData++ = (BYTE)eJson;
					tReq.GetBytes ( pData, iLen );
				}
			}
			break;

		case SPH_ATTR_FLOAT:
			tMatch.SetAttrFloat ( tAttr.m_tLocator, tReq.GetFloat() );
			break;

		case SPH_ATTR_DOUBLE:
			tMatch.SetAttrDouble ( tAttr.m_tLocator, tReq.GetDouble() );
			break;

		case SPH_ATTR_BIGINT:
		case SPH_ATTR_UINT64:
			tMatch.SetAttr ( tAttr.m_tLocator, tReq.GetUint64() );
			break;

		default:
			tMatch.SetAttr ( tAttr.m_tLocator, tReq.GetDword() );
			break;
		}
	}
}


static void ParseSchema ( OneResultset_t & tRes, MemInputBuffer_c & tReq )
{
	CSphSchema & tSchema = tRes.m_tSchema;
	tSchema.Reset ();

	int nFields = tReq.GetInt(); // FIXME! add a sanity check
	for ( int j = 0; j < nFields; ++j )
		tSchema.AddField ( tReq.GetString().cstr() );

	int iNumAttrs = tReq.GetInt(); // FIXME! add a sanity check
	for ( int j=0; j<iNumAttrs; ++j )
	{
		CSphColumnInfo tCol;
		tCol.m_sName = tReq.GetString ();
		tCol.m_eAttrType = (ESphAttr) tReq.GetDword (); // FIXME! add a sanity check

		// we always work with plain attrs (not *_PTR) when working with agents
		tCol.m_eAttrType = sphPlainAttrToPtrAttr ( tCol.m_eAttrType );
		if ( tCol.m_eAttrType==SPH_ATTR_STORED_FIELD )
		{
			tCol.m_eAttrType = SPH_ATTR_STRINGPTR;
			tCol.m_uFieldFlags = CSphColumnInfo::FIELD_STORED;
		}
		tSchema.AddAttr ( tCol, true ); // all attributes received from agents are dynamic
	}
}


bool SearchReplyParser_c::ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const
{
	const int iResults = m_iResults;
	assert ( iResults>0 );

	if ( !tAgent.m_pResult )
		tAgent.m_pResult = std::make_unique<cSearchResult>();

	auto pResult = (cSearchResult*)tAgent.m_pResult.get();
	auto &dResults = pResult->m_dResults;

	dResults.Resize ( iResults );
	for ( auto & tRes : dResults )
	{
		tRes.m_iSuccesses = 0;
		OneResultset_t tChunk;
		tChunk.m_iTag = tAgent.m_iStoreTag;
		tChunk.m_bTag = true;
		tChunk.m_pAgent = &tAgent;
		tRes.m_sError = "";
		tRes.m_sWarning = "";

		// get status and message
		auto eStatus = ( SearchdStatus_e ) tReq.GetDword ();
		switch ( eStatus )
		{
			case SEARCHD_ERROR:		tRes.m_sError = tReq.GetString (); continue;
			case SEARCHD_RETRY:		tRes.m_sError = tReq.GetString (); break;
			case SEARCHD_WARNING:	tRes.m_sWarning = tReq.GetString (); break;
			default:				tAgent.m_sFailure.SetSprintf ( "internal error: unknown status %d, message %s", eStatus, tReq.GetString ().cstr() );
			case SEARCHD_OK: break;
		}

		ParseSchema ( tChunk, tReq );

		// get matches
		int iMatches = tReq.GetInt ();
		if ( iMatches<0 )
		{
			tAgent.m_sFailure.SetSprintf ( "invalid match count received (count=%d)", iMatches );
			return false;
		}

		bool bAgent64 = !!tReq.GetInt();
		if ( !bAgent64 )
		{
			tAgent.m_sFailure.SetSprintf ( "agent has 32-bit docids; no longer supported" );
			return false;
		}

		tChunk.m_dMatches.Resize ( iMatches );
		for ( auto & tMatch : tChunk.m_dMatches )
			ParseMatch ( tMatch, tReq, tChunk.m_tSchema, bAgent64 );

		// read totals (retrieved count, total count, query time, word count)
		int iRetrieved = tReq.GetInt ();
		tRes.m_iTotalMatches = tReq.GetInt ();
		tRes.m_bTotalMatchesApprox = !!tReq.GetInt();
		tRes.m_iQueryTime = tReq.GetInt ();

		// agents always send IO/CPU stats to master
		BYTE uStatMask = tReq.GetByte();
		if ( uStatMask & 1U )
		{
			tRes.m_tIOStats.m_iReadTime = tReq.GetUint64();
			tRes.m_tIOStats.m_iReadOps = tReq.GetDword();
			tRes.m_tIOStats.m_iReadBytes = tReq.GetUint64();
			tRes.m_tIOStats.m_iWriteTime = tReq.GetUint64();
			tRes.m_tIOStats.m_iWriteOps = tReq.GetDword();
			tRes.m_tIOStats.m_iWriteBytes = tReq.GetUint64();
		}

		if ( uStatMask & 2U )
			tRes.m_iCpuTime = tReq.GetUint64();

		if ( uStatMask & 4U )
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
		for ( int i=0; i<iWordsCount; ++i )
		{
			const CSphString sWord = tReq.GetString ();
			const int64_t iDocs = (unsigned int)tReq.GetInt ();
			const int64_t iHits = (unsigned int)tReq.GetInt ();
			tReq.GetByte(); // statistics have no expanded terms for now

			tRes.AddStat ( sWord, iDocs, iHits );
		}

		// mark this result as ok
		auto& tNewChunk = tRes.m_dResults.Add ();
		::Swap ( tNewChunk, tChunk );
		tRes.m_iSuccesses = 1;
	}

	// all seems OK (and buffer length checks are performed by caller)
	return true;
}

extern int g_iMaxBatchQueries;
extern int g_iMaxFilters;
extern int g_iMaxFilterValues;
extern bool	g_bIOStats;
static auto& g_bCpuStats 	= sphGetbCpuStat ();


static bool ParseSearchFilter ( CSphFilterSettings & tFilter, InputBuffer_c & tReq, ISphOutputBuffer & tOut, int iMasterVer, DWORD uVer )
{
	tFilter.m_sAttrName = tReq.GetString ();
	sphColumnToLowercase ( const_cast<char *>( tFilter.m_sAttrName.cstr() ) );

	tFilter.m_eType = (ESphFilter) tReq.GetDword ();
	switch ( tFilter.m_eType )
	{
	case SPH_FILTER_RANGE:
		tFilter.m_iMinValue = tReq.GetUint64();
		tFilter.m_iMaxValue = tReq.GetUint64();
		break;

	case SPH_FILTER_FLOATRANGE:
		tFilter.m_fMinValue = tReq.GetFloat ();
		tFilter.m_fMaxValue = tReq.GetFloat ();
		break;

	case SPH_FILTER_VALUES:
		{
			int iGot = 0;
			bool bRes = tReq.GetQwords ( tFilter.m_dValues, iGot, g_iMaxFilterValues );
			if ( !bRes )
			{
				SendErrorReply ( tOut, "invalid attribute '%s' set length %d (should be in 0..%d range)", tFilter.m_sAttrName.cstr(), iGot, g_iMaxFilterValues );
				return false;
			}
		}
		break;

	case SPH_FILTER_USERVAR:
	case SPH_FILTER_STRING:
		tFilter.m_dStrings.Add ( tReq.GetString() );
		if ( uVer>=0x126 )
			tFilter.m_eStrCmpDir = (EStrCmpDir) tReq.GetByte();
		break;

	case SPH_FILTER_NULL:
		tFilter.m_bIsNull = tReq.GetByte()!=0;
		break;

	case SPH_FILTER_STRING_LIST:
		{
			int iCount = tReq.GetDword();
			if ( iCount<0 || iCount>g_iMaxFilterValues )
			{
				SendErrorReply ( tOut, "invalid attribute '%s' set length %d (should be in 0..%d range)", tFilter.m_sAttrName.cstr(), iCount, g_iMaxFilterValues );
				return false;
			}
			tFilter.m_dStrings.Resize ( iCount );
			ARRAY_FOREACH ( iString, tFilter.m_dStrings )
				tFilter.m_dStrings[iString] = tReq.GetString();
		}
		break;
	case SPH_FILTER_EXPRESSION: // need only name and type
		break;

	default:
		SendErrorReply ( tOut, "unknown filter type (type-id=%d)", tFilter.m_eType );
		return false;
	}

	if ( tFilter.m_sAttrName=="@id" )
	{
		// request coming from old master, need to fix attribute name
		tFilter.m_sAttrName = "id";

		// and clamp values from uint64_t to int64_t
		if ( (uint64_t)tFilter.m_iMinValue > (uint64_t)LLONG_MAX )
			tFilter.m_iMinValue = LLONG_MAX;

		if ( (uint64_t)tFilter.m_iMaxValue > (uint64_t)LLONG_MAX )
			tFilter.m_iMaxValue = LLONG_MAX;
	}

	tFilter.m_bExclude = !!tReq.GetDword ();

	if ( iMasterVer>=15 )
	{
		tFilter.m_bHasEqualMin = !!tReq.GetDword();
		tFilter.m_bHasEqualMax = !!tReq.GetDword();
	} else if ( iMasterVer>=5 )
		tFilter.m_bHasEqualMin = tFilter.m_bHasEqualMax = !!tReq.GetDword();

	if ( iMasterVer>=15 )
	{
		tFilter.m_bOpenLeft = !!tReq.GetDword();
		tFilter.m_bOpenRight = !!tReq.GetDword();
	}

	tFilter.m_eMvaFunc = SPH_MVAFUNC_ANY;
	if ( iMasterVer>=13 )
		tFilter.m_eMvaFunc = (ESphMvaFunc)tReq.GetDword();

	return true;
}

static void AddDocids ( CSphVector<CSphQueryItem> & dItems )
{
	if ( !dItems.GetLength() )
		return;

	bool bHaveDocID = false;
	for ( const auto & i : dItems )
		bHaveDocID |= i.m_sAlias==sphGetDocidName() || i.m_sExpr=="*";

	if ( !bHaveDocID )
	{
		CSphQueryItem tId;
		tId.m_sExpr = tId.m_sAlias = sphGetDocidName();
		dItems.Insert ( 0, tId );
	}
}


void PrepareQueryEmulation ( CSphQuery * pQuery )
{
	if ( pQuery->m_eMode == SPH_MATCH_BOOLEAN )
		pQuery->m_eRanker = SPH_RANK_NONE;

	if ( pQuery->m_eMode == SPH_MATCH_FULLSCAN )
		pQuery->m_sQuery = "";

	if ( pQuery->m_eMode != SPH_MATCH_ALL && pQuery->m_eMode != SPH_MATCH_ANY && pQuery->m_eMode != SPH_MATCH_PHRASE )
		return;

	const char * szQuery = pQuery->m_sRawQuery.cstr();
	int iQueryLen = szQuery ? (int) strlen ( szQuery ) : 0;

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
}


static void FixupQuerySettings ( CSphQuery & tQuery )
{
	// sort filters
	for ( auto & i : tQuery.m_dFilters )
		i.m_dValues.Sort();

	if ( !tQuery.m_bHasOuter )
	{
		tQuery.m_sOuterOrderBy = "";
		tQuery.m_iOuterOffset = 0;
		tQuery.m_iOuterLimit = 0;
	}
}

bool ParseSearchQuery ( InputBuffer_c & tReq, ISphOutputBuffer & tOut, CSphQuery & tQuery, WORD uVer, WORD uMasterVer )
{
	// daemon-level defaults
	tQuery.m_iRetryCount = DEFAULT_QUERY_RETRY;
	tQuery.m_iRetryDelay = DEFAULT_QUERY_RETRY;
	tQuery.m_iAgentQueryTimeoutMs = DEFAULT_QUERY_TIMEOUT;

	// v.1.27+ flags come first
	DWORD uFlags = 0;
	if ( uVer>=0x11B )
		uFlags = tReq.GetDword();

	// v.1.0. mode, limits, weights, ID/TS ranges
	tQuery.m_iOffset = tReq.GetInt ();
	tQuery.m_iLimit = tReq.GetInt ();
	tQuery.m_eMode = (ESphMatchMode) tReq.GetInt ();
	tQuery.m_eRanker = (ESphRankMode) tReq.GetInt ();
	if ( tQuery.m_eRanker==SPH_RANK_EXPR || tQuery.m_eRanker==SPH_RANK_EXPORT )
		tQuery.m_sRankerExpr = tReq.GetString();

	tQuery.m_eSort = (ESphSortOrder) tReq.GetInt ();
	tQuery.m_sSortBy = tReq.GetString ();

	// here we once eliminate SPH_SORT_ATTR_ASC in flavour of SPH_SORT_EXTENDED
	if ( tQuery.m_eSort == SPH_SORT_ATTR_ASC )
	{
		tQuery.m_sSortBy = SphSprintf ( "%s ASC", tQuery.m_sSortBy.cstr() );
		tQuery.m_eSort = SPH_SORT_EXTENDED;
	}

	// here we once eliminate SPH_SORT_ATTR_DESC in flavour of SPH_SORT_EXTENDED
	if ( tQuery.m_eSort == SPH_SORT_ATTR_DESC )
	{
		tQuery.m_sSortBy = SphSprintf ( "%s DESC", tQuery.m_sSortBy.cstr() );
		tQuery.m_eSort = SPH_SORT_EXTENDED;
	}

	sphColumnToLowercase ( const_cast<char *>( tQuery.m_sSortBy.cstr() ) );
	tQuery.m_sRawQuery = tReq.GetString ();
	{
		int iGot = 0;
		if ( !tReq.GetDwords ( tQuery.m_dWeights, iGot, SPH_MAX_FIELDS ) )
		{
			SendErrorReply ( tOut, "invalid weight count %d (should be in 0..%d range)", iGot, SPH_MAX_FIELDS );
			return false;
		}
	}

	tQuery.m_sIndexes = tReq.GetString ();

	// legacy id range filter
	bool bIdrange64 = tReq.GetInt()!=0;
	DocID_t tMinDocID = bIdrange64 ? (DocID_t)tReq.GetUint64 () : tReq.GetDword ();
	DocID_t tMaxDocID = bIdrange64 ? (DocID_t)tReq.GetUint64 () : tReq.GetDword ();

	if ( tMaxDocID==0 || (uint64_t)tMaxDocID==UINT64_MAX )
		tMaxDocID = INT64_MAX;

	int iAttrFilters = tReq.GetInt ();
	if ( iAttrFilters>g_iMaxFilters )
	{
		SendErrorReply ( tOut, "too many attribute filters (req=%d, max=%d)", iAttrFilters, g_iMaxFilters );
		return false;
	}

	tQuery.m_dFilters.Resize ( iAttrFilters );
	for ( auto & i : tQuery.m_dFilters )
		if ( !ParseSearchFilter ( i, tReq, tOut, uMasterVer, uVer ) )
			return false;

	// now add id range filter
	if ( tMinDocID!=0 || tMaxDocID!=INT64_MAX )
	{
		CSphFilterSettings & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_sAttrName = sphGetDocidName();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_iMinValue = tMinDocID;
		tFilter.m_iMaxValue = tMaxDocID;
	}

	tQuery.m_eGroupFunc = (ESphGroupBy) tReq.GetDword ();
	tQuery.m_sGroupBy = tReq.GetString ();
	sphColumnToLowercase ( const_cast<char *>( tQuery.m_sGroupBy.cstr() ) );

	tQuery.m_iMaxMatches = tReq.GetInt ();
	tQuery.m_bExplicitMaxMatches = tQuery.m_iMaxMatches!=DEFAULT_MAX_MATCHES; // fixme?
	tQuery.m_sGroupSortBy = tReq.GetString ();
	tQuery.m_iCutoff = tReq.GetInt();
	tQuery.m_iRetryCount = tReq.GetInt ();
	tQuery.m_iRetryDelay = tReq.GetInt ();
	tQuery.m_sGroupDistinct = tReq.GetString ();
	sphColumnToLowercase ( const_cast<char *>( tQuery.m_sGroupDistinct.cstr() ) );

	tQuery.m_bGeoAnchor = ( tReq.GetInt()!=0 );
	if ( tQuery.m_bGeoAnchor )
	{
		tQuery.m_sGeoLatAttr = tReq.GetString ();
		tQuery.m_sGeoLongAttr = tReq.GetString ();
		tQuery.m_fGeoLatitude = tReq.GetFloat ();
		tQuery.m_fGeoLongitude = tReq.GetFloat ();
	}

	tQuery.m_dIndexWeights.Resize ( tReq.GetInt() ); // FIXME! add sanity check
	for ( auto& dIndexWeight : tQuery.m_dIndexWeights )
		tReq >> dIndexWeight;

	tQuery.m_uMaxQueryMsec = tReq.GetDword ();

	tQuery.m_dFieldWeights.Resize ( tReq.GetInt() ); // FIXME! add sanity check
	for ( auto & dFieldWeight : tQuery.m_dFieldWeights )
		tReq >> dFieldWeight;

	tQuery.m_sComment = tReq.GetString ();

	int nOverrides = tReq.GetInt();
	if ( nOverrides>0 )
	{
		SendErrorReply ( tOut, "overrides are now deprecated" );
		return false;
	}

	tQuery.m_sSelect = tReq.GetString ();
	tQuery.m_bAgent = ( uMasterVer>0 );
	if ( tQuery.m_sSelect.Begins ( "*,*" ) ) // this is the legacy mark of agent for debug purpose
	{
		tQuery.m_bAgent = true;
		int iSelectLen = tQuery.m_sSelect.Length();
		tQuery.m_sSelect = ( iSelectLen>4 ? tQuery.m_sSelect.SubString ( 4, iSelectLen-4 ) : "*" );
	}
	// fixup select list
	if ( tQuery.m_sSelect.IsEmpty () )
		tQuery.m_sSelect = "*";

	// master sends items to agents since master.version=15
	CSphString sError;
	if ( uMasterVer<15 && !ParseSelectList ( sError, tQuery ) )
	{
		// we want to see a parse error in query_log_format=sphinxql mode too
		LogQueryToSphinxlLog ( tQuery.m_sSelect, sError );
		SendErrorReply ( tOut, "select: %s", sError.cstr () );
		return false;
	}

	// v.1.27
	if ( uVer>=0x11B )
	{
		// parse simple flags
		tQuery.m_bSortKbuffer = !!( uFlags & QFLAG_SORT_KBUFFER );

		// simplify is optional query option and might be set by the daemon config
		if ( uVer<0x127 || ( uVer>=0x127 && ( uFlags & QFLAG_SIMPLIFY_SET )==QFLAG_SIMPLIFY_SET ) )
			tQuery.m_bSimplify = !!( uFlags & QFLAG_SIMPLIFY );

		tQuery.m_bPlainIDF = !!( uFlags & QFLAG_PLAIN_IDF );
		tQuery.m_bGlobalIDF = !!( uFlags & QFLAG_GLOBAL_IDF );
		if ( uVer<0x125 || ( uVer>=0x125 && ( uFlags & QFLAG_LOCAL_DF_SET )==QFLAG_LOCAL_DF_SET ) )
			tQuery.m_bLocalDF = !!( uFlags & QFLAG_LOCAL_DF );
		tQuery.m_bLowPriority = !!( uFlags & QFLAG_LOW_PRIORITY );
		tQuery.m_bFacet = !!( uFlags & QFLAG_FACET );
		tQuery.m_bFacetHead = !!( uFlags & QFLAG_FACET_HEAD );
		tQuery.m_eQueryType = (uFlags & QFLAG_JSON_QUERY) ? QUERY_JSON : QUERY_API;
		tQuery.m_bNotOnlyAllowed = !!( uFlags & QFLAG_NOT_ONLY_ALLOWED );

		if ( uMasterVer>0 || uVer==0x11E )
			tQuery.m_bNormalizedTFIDF = !!( uFlags & QFLAG_NORMALIZED_TF );

		// fetch optional stuff
		if ( uFlags & QFLAG_MAX_PREDICTED_TIME )
			tQuery.m_iMaxPredictedMsec = tReq.GetInt();
	}

	// v.1.29
	if ( uVer>=0x11D )
	{
		tQuery.m_sOuterOrderBy = tReq.GetString();
		tQuery.m_iOuterOffset = tReq.GetDword();
		tQuery.m_iOuterLimit = tReq.GetDword();
		tQuery.m_bHasOuter = ( tReq.GetInt()!=0 );
	}
	if ( uVer>=0x124 )
		tQuery.m_iExpansionLimit = tReq.GetInt();

	// extension v.1
	tQuery.m_eCollation = GlobalCollation ();
	if ( uMasterVer>=1 )
		tQuery.m_eCollation = (ESphCollation)tReq.GetDword();

	// extension v.2
	if ( uMasterVer>=2 )
	{
		tQuery.m_sOuterOrderBy = tReq.GetString();
		if ( tQuery.m_bHasOuter )
			tQuery.m_iOuterLimit = tReq.GetInt();
	}

	if ( uMasterVer>=6 )
		tQuery.m_iGroupbyLimit = tReq.GetInt();

	if ( uMasterVer>=14 )
	{
		tQuery.m_sUDRanker = tReq.GetString();
		tQuery.m_sUDRankerOpts = tReq.GetString();
	}

	if ( uMasterVer>=14 || uVer>=0x120 )
	{
		tQuery.m_sQueryTokenFilterLib = tReq.GetString();
		tQuery.m_sQueryTokenFilterName = tReq.GetString();
		tQuery.m_sQueryTokenFilterOpts = tReq.GetString();
	}

	if ( uVer>=0x121 )
	{
		tQuery.m_dFilterTree.Resize ( tReq.GetInt() );
		for ( FilterTreeItem_t &tItem : tQuery.m_dFilterTree )
		{
			tItem.m_iLeft = tReq.GetInt();
			tItem.m_iRight = tReq.GetInt();
			tItem.m_iFilterItem = tReq.GetInt();
			tItem.m_bOr = ( tReq.GetInt()!=0 );
		}
	}

	if ( uMasterVer>=15 )
	{
		tQuery.m_dItems.Resize ( tReq.GetInt() );
		for ( CSphQueryItem &tItem : tQuery.m_dItems )
		{
			tItem.m_sAlias = tReq.GetString();
			tItem.m_sExpr = tReq.GetString();
			tItem.m_eAggrFunc = (ESphAggrFunc)tReq.GetDword();
		}
		tQuery.m_dRefItems.Resize ( tReq.GetInt() );
		for ( CSphQueryItem &tItem : tQuery.m_dRefItems )
		{
			tItem.m_sAlias = tReq.GetString();
			tItem.m_sExpr = tReq.GetString();
			tItem.m_eAggrFunc = (ESphAggrFunc)tReq.GetDword();
		}
	}

	if ( uMasterVer>=16 )
		tQuery.m_eExpandKeywords = (QueryOption_e)tReq.GetDword();

	// pre-v.20 had old-style index hints, but they were not documented anyway
	if ( uMasterVer>=20 )
	{
		tQuery.m_dIndexHints.Resize ( tReq.GetDword() );
		for ( auto & i : tQuery.m_dIndexHints )
		{
			i.m_sIndex = tReq.GetString();
			i.m_eType = (SecondaryIndexType_e)tReq.GetDword();
			i.m_bForce = !!tReq.GetDword();
		}
	}

	if ( uMasterVer>=21 )
	{
		tQuery.m_eJoinType	= (JoinType_e)tReq.GetDword();
		tQuery.m_sJoinIdx	= tReq.GetString();
		tQuery.m_sJoinQuery	= tReq.GetString();

		tQuery.m_dOnFilters.Resize ( tReq.GetDword() );
		for ( auto & i : tQuery.m_dOnFilters )
		{
			i.m_sIdx1	= tReq.GetString();
			i.m_sAttr1	= tReq.GetString();
			i.m_sIdx2	= tReq.GetString();
			i.m_sAttr2	= tReq.GetString();

			if ( uMasterVer>=22 )
			{
				i.m_eTypeCast1 = (ESphAttr)tReq.GetInt();
				i.m_eTypeCast2 = (ESphAttr)tReq.GetInt();
			}
		}
	}

	if ( uMasterVer>=22 )
	{
		auto & tKNN = tQuery.m_tKnnSettings;
		tKNN.m_sAttr = tReq.GetString();
		if ( !tKNN.m_sAttr.IsEmpty() )
		{
			tKNN.m_iK = tReq.GetInt();
			if ( tKNN.m_iK <= 0 )
			{
				SendErrorReply ( tOut, "k parameter must be positive" );
				return false;
			}
			tKNN.m_iEf = tReq.GetInt();
			if ( tKNN.m_iEf < 0 )
			{
				SendErrorReply ( tOut, "ef parameter must be non-negative" );
				return false;
			}
			if ( uMasterVer>=25 )
			{
				tKNN.m_bRescore = !!tReq.GetInt();
				tKNN.m_fOversampling = tReq.GetFloat();
				if ( tKNN.m_fOversampling < 1.0f )
				{
					SendErrorReply ( tOut, "oversampling parameter must be >= 1.0" );
					return false;
				}
			}

			if ( uMasterVer>=28 )
			{
				tKNN.m_bPrefilter = !!tReq.GetInt();
				tKNN.m_bFullscan = !!tReq.GetInt();
			}

			tKNN.m_dVec.Resize ( tReq.GetInt() );
			for ( auto & i : tKNN.m_dVec )
				i = tReq.GetFloat();

			if ( uMasterVer>=26 )
			{
				bool bHasEmb = !!tReq.GetByte();
				if ( bHasEmb )
					tKNN.m_sEmbStr = tReq.GetString();
			}
		}
	}

	if ( uMasterVer>=23 )
		tQuery.m_eJiebaMode = (JiebaMode_e)tReq.GetInt();

	if ( uMasterVer>=24 )
	{
		tQuery.m_tScrollSettings.m_sSortBy = tReq.GetString();
		tQuery.m_tScrollSettings.m_bRequested = !!tReq.GetInt();
		tQuery.m_tScrollSettings.m_dAttrs.Resize ( tReq.GetInt() );

		for ( auto & i : tQuery.m_tScrollSettings.m_dAttrs )
		{
			i.m_sSortAttr = tReq.GetString();
			i.m_bDesc = !!tReq.GetInt();
			i.m_eType = (ESphAttr)tReq.GetInt();
			i.m_tValue = (SphAttr_t)tReq.GetUint64();
			i.m_fValue = tReq.GetFloat();
			i.m_sValue = tReq.GetString();
		}
	}

	if ( uMasterVer>=27 )
		tQuery.m_sExpandBlended = tReq.GetString();

	/////////////////////
	// additional checks
	/////////////////////

	// queries coming from API may not request docids
	// but we still need docids when sending result sets
	AddDocids ( tQuery.m_dItems );
	AddDocids ( tQuery.m_dRefItems );

	if ( tReq.GetError() )
	{
		SendErrorReply ( tOut, "invalid or truncated request" );
		return false;
	}

	CheckQuery ( tQuery, sError );
	if ( !sError.IsEmpty() )
	{
		SendErrorReply ( tOut, "%s", sError.cstr() );
		return false;
	}

	// now prepare it for the engine
	tQuery.m_sQuery = tQuery.m_sRawQuery;

	if ( tQuery.m_eQueryType!=QUERY_JSON )
		PrepareQueryEmulation ( &tQuery );

	FixupQuerySettings ( tQuery );

	// all ok
	return true;
}

static void SendDataPtrAttr ( ISphOutputBuffer& tOut, const BYTE * pData )
{
	auto dData = sphUnpackPtrAttr ( pData );
	tOut.SendArray ( dData );
}


static char g_sJsonNull[] = "{}";

static void SendJsonAsString ( ISphOutputBuffer& tOut, const BYTE * pJSON )
{
	if ( pJSON )
	{
		auto dData = sphUnpackPtrAttr ( pJSON );
		JsonEscapedBuilder dJson;
		dJson.GrowEnough ( dData.second * 2 );
		sphJsonFormat ( dJson, dData.first );
		tOut.SendArray ( dJson );
	} else
		// magic zero - "{}"
		tOut.SendArray ( g_sJsonNull, sizeof ( g_sJsonNull )-1 );
}


static void SendJson ( ISphOutputBuffer& tOut, const BYTE * pJSON, bool bSendJson )
{
	if ( bSendJson )
		SendDataPtrAttr ( tOut, pJSON ); // send BSON
	else
		SendJsonAsString ( tOut, pJSON ); // send string
}

static void SendJsonFieldAsString ( ISphOutputBuffer& tOut, const BYTE * pJSON )
{
	if ( !pJSON )
	{
		tOut.SendDword(0);
		return;
	}

	auto dData = sphUnpackPtrAttr ( pJSON );
	auto eJson = (ESphJsonType) *dData.first++;

	JsonEscapedBuilder dJson;
	dJson.GrowEnough ( dData.second * 2 );
	sphJsonFieldFormat ( dJson, dData.first, eJson, false );
	tOut.SendArray ( dJson );
}

static void SendJsonField ( ISphOutputBuffer& tOut, const BYTE * pJSON, bool bSendJsonField )
{
	if ( !bSendJsonField )
	{
		SendJsonFieldAsString ( tOut, pJSON );
		return;
	}

	auto dData = sphUnpackPtrAttr ( pJSON );
	if ( IsEmpty ( dData ) || *dData.first==JSON_EOF )
		tOut.SendByte ( JSON_EOF );
	else
	{
		tOut.SendByte ( *dData.first );
		tOut.SendArray ( dData.first+1, dData.second-1 );
	}
}

static void SendMVA ( ISphOutputBuffer& tOut, const BYTE * pMVA, bool b64bit )
{
	if ( !pMVA )
	{
		tOut.SendDword ( 0 );
		return;
	}

	auto dMVA = sphUnpackPtrAttr ( pMVA );
	DWORD uValues = dMVA.second / sizeof(DWORD);
	tOut.SendDword(uValues);

	const auto * pValues = (const DWORD *) dMVA.first;

	if ( b64bit )
	{
		assert ( ( uValues%2 )==0 );
		while ( uValues )
		{
			auto uMVA = MVA_BE ( pValues );
			tOut.SendDword ( uMVA.first );
			tOut.SendDword ( uMVA.second );
			pValues += 2;
			uValues -= 2;
		}
	} else
	{
		while ( uValues-- )
			tOut.SendDword ( *pValues++ );
	}
}

static void SendFloatVec ( ISphOutputBuffer & tOut, const BYTE * pData )
{
	if ( !pData )
	{
		tOut.SendDword(0);
		return;
	}

	auto dFloatVec = sphUnpackPtrAttr ( pData );
	DWORD uValues = dFloatVec.second / sizeof(float);
	tOut.SendDword(uValues);

	auto pValues = (const float *) dFloatVec.first;
	while ( uValues-- )
		tOut.SendFloat ( *pValues++ );
}

static ESphAttr FixupAttrForNetwork ( const CSphColumnInfo & tCol, const CSphSchema & tSchema, int iVer, WORD uMasterVer, bool bAgentMode )
{
	bool bSendJson = ( bAgentMode && uMasterVer>=3 );
	bool bSendJsonField = ( bAgentMode && uMasterVer>=4 );

	switch ( tCol.m_eAttrType )
	{
	case SPH_ATTR_UINT32SET_PTR:
		return SPH_ATTR_UINT32SET;

	case SPH_ATTR_INT64SET_PTR:
		return SPH_ATTR_INT64SET;

	case SPH_ATTR_FLOAT_VECTOR_PTR:
		return SPH_ATTR_FLOAT_VECTOR;

	case SPH_ATTR_STRINGPTR:
	{
		if ( bAgentMode && uMasterVer>=18 && IsNotRealAttribute ( tCol ) )
			return SPH_ATTR_STORED_FIELD;
		else
			return SPH_ATTR_STRING;
	}

	case SPH_ATTR_JSON:
	case SPH_ATTR_JSON_PTR:
		return bSendJson ? SPH_ATTR_JSON : SPH_ATTR_STRING;

	case SPH_ATTR_JSON_FIELD:
	case SPH_ATTR_JSON_FIELD_PTR:
		return bSendJsonField ? SPH_ATTR_JSON_FIELD : SPH_ATTR_STRING;

	case SPH_ATTR_DOUBLE:
		return iVer<0x122 ? SPH_ATTR_FLOAT : SPH_ATTR_DOUBLE;

	case SPH_ATTR_UINT64:
		return iVer<0x123 ? SPH_ATTR_BIGINT : SPH_ATTR_UINT64;

	default: return tCol.m_eAttrType;
	}
}

static void SendSchema ( ISphOutputBuffer & tOut, const AggrResult_t & tRes, const CSphBitvec & tAttrsToSend, int iVer, WORD uMasterVer, bool bAgentMode )
{
	int iFieldsCount = tRes.m_tSchema.GetFieldsCount();
	tOut.SendInt ( iFieldsCount );
	for ( int i=0; i < iFieldsCount; ++i )
		tOut.SendString ( tRes.m_tSchema.GetFieldName(i) );

	tOut.SendInt ( tAttrsToSend.BitCount() );
	for ( int i=0; i<tRes.m_tSchema.GetAttrsCount(); ++i )
	{
		if ( !tAttrsToSend.BitGet(i) )
			continue;

		const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
		tOut.SendString ( tCol.m_sName.cstr() );

		ESphAttr eCol = FixupAttrForNetwork ( tCol, tRes.m_tSchema, iVer, uMasterVer, bAgentMode );
		tOut.SendDword ( (DWORD)eCol );
	}
}

static void SendAttribute ( ISphOutputBuffer & tOut, const CSphMatch & tMatch, const CSphColumnInfo & tAttr, int iVer, WORD uMasterVer, bool bAgentMode )
{
	// at this point we should not have any attributes that point to pooled data
	assert ( sphPlainAttrToPtrAttr(tAttr.m_eAttrType)==tAttr.m_eAttrType );

	// send binary json only to master
	bool bSendJson = bAgentMode && uMasterVer>=3;
	bool bSendJsonField = bAgentMode && uMasterVer>=4;

	const CSphAttrLocator & tLoc = tAttr.m_tLocator;

	switch ( tAttr.m_eAttrType )
	{
	case SPH_ATTR_UINT32SET_PTR:
	case SPH_ATTR_INT64SET_PTR:
		SendMVA ( tOut, (const BYTE*)tMatch.GetAttr(tLoc), tAttr.m_eAttrType==SPH_ATTR_INT64SET_PTR );
		break;

	case SPH_ATTR_FLOAT_VECTOR_PTR:
		SendFloatVec ( tOut, (const BYTE*)tMatch.GetAttr(tLoc) );
		break;

	case SPH_ATTR_JSON_PTR:
		SendJson ( tOut, (const BYTE*)tMatch.GetAttr(tLoc), bSendJson );
		break;

	case SPH_ATTR_STRINGPTR:
		SendDataPtrAttr ( tOut, (const BYTE*)tMatch.GetAttr(tLoc) );
		break;

	case SPH_ATTR_JSON_FIELD_PTR:
		SendJsonField ( tOut, (const BYTE*)tMatch.GetAttr(tLoc), bSendJsonField );
		break;

	case SPH_ATTR_FACTORS:
	case SPH_ATTR_FACTORS_JSON:
		if ( iVer<0x11C )
		{
			tOut.SendDword ( 0 );
			break;
		}

		SendDataPtrAttr ( tOut, (const BYTE*)tMatch.GetAttr(tLoc) );
		break;

	case SPH_ATTR_FLOAT:
		tOut.SendFloat ( tMatch.GetAttrFloat(tLoc) );
		break;

	case SPH_ATTR_DOUBLE:
		if ( iVer<0x122 )
			tOut.SendFloat ( (float)tMatch.GetAttrDouble(tLoc) );
		else
			tOut.SendDouble ( tMatch.GetAttrDouble(tLoc) );
		break;

	case SPH_ATTR_BIGINT:
	case SPH_ATTR_UINT64:
		tOut.SendUint64 ( tMatch.GetAttr(tLoc) );
		break;

	default:
		tOut.SendDword ( (DWORD)tMatch.GetAttr(tLoc) );
		break;
	}
}

void SendResult ( int iVer, ISphOutputBuffer & tOut, const AggrResult_t& tRes, bool bAgentMode, const CSphQuery & tQuery, WORD uMasterVer )
{
	// multi-query status
	bool bError = !tRes.m_sError.IsEmpty();
	bool bWarning = !bError && !tRes.m_sWarning.IsEmpty();
	bError |= tRes.m_dResults.IsEmpty() && tQuery.m_bFacet;

	assert ( bError || tRes.m_bSingle );
	assert ( bError || tRes.m_bOneSchema );

	if ( bError )
	{
		tOut.SendInt ( SEARCHD_ERROR ); // fixme! m.b. use APICommand_t and refactor to common API way
		tOut.SendString ( tRes.m_sError.cstr() );
		LogToConsole ( "error", tRes.m_sError.cstr() );
		return;
	}
	if ( bWarning )
	{
		tOut.SendDword ( SEARCHD_WARNING );
		tOut.SendString ( tRes.m_sWarning.cstr() );
		LogToConsole ( "warning", tRes.m_sWarning.cstr() );
	} else
		tOut.SendDword ( SEARCHD_OK );

	CSphBitvec tAttrsToSend;
	sphGetAttrsToSend ( tRes.m_tSchema, bAgentMode, false, tAttrsToSend );

	// send schema
	SendSchema ( tOut, tRes, tAttrsToSend, iVer, uMasterVer, bAgentMode );

	// send matches
	tOut.SendInt ( tRes.m_iCount );
	tOut.SendInt ( 1 ); // was USE_64BIT

	CSphVector<BYTE> dJson ( 512 );

	auto& dResult = tRes.m_dResults.First();
	auto dMatches = dResult.m_dMatches.Slice ( tRes.m_iOffset, tRes.m_iCount );

	for ( const CSphMatch & tMatch : dMatches )
	{
		Verify ( tRes.m_tSchema.GetAttr(sphGetDocidName()) );
		tOut.SendUint64 ( sphGetDocID(tMatch.m_pDynamic) );
		tOut.SendInt ( tMatch.m_iWeight );

		assert ( tMatch.m_pStatic || !tRes.m_tSchema.GetStaticSize() );
#if 0
		// not correct any more because of internal attrs (such as string sorting ptrs)
		assert ( tMatch.m_pDynamic || !pRes->m_tSchema.GetDynamicSize() );
		assert ( !tMatch.m_pDynamic || (int)tMatch.m_pDynamic[-1]==pRes->m_tSchema.GetDynamicSize() );
#endif
		for ( int j=0; j<tRes.m_tSchema.GetAttrsCount(); ++j )
			if ( tAttrsToSend.BitGet(j) )
				SendAttribute ( tOut, tMatch, tRes.m_tSchema.GetAttr(j), iVer, uMasterVer, bAgentMode );
	}

	if ( tQuery.m_bAgent && tQuery.m_iLimit )
		tOut.SendInt ( tRes.m_iCount );
	else
		tOut.SendInt ( dResult.m_dMatches.GetLength() );

	tOut.SendAsDword ( tRes.m_iTotalMatches );
	if ( bAgentMode && uMasterVer>=19 )
		tOut.SendInt ( tRes.m_bTotalMatchesApprox ? 1 : 0 );

	tOut.SendInt ( Max ( tRes.m_iQueryTime, 0 ) );

	if ( iVer>=0x11A && bAgentMode )
	{
		bool bNeedPredictedTime = tQuery.m_iMaxPredictedMsec > 0;

		BYTE uStatMask = ( bNeedPredictedTime ? 4U : 0U ) | ( g_bCpuStats ? 2U : 0U ) | ( g_bIOStats ? 1U : 0U );
		tOut.SendByte ( uStatMask );

		if ( g_bIOStats )
		{
			CSphIOStats tStats = tRes.m_tIOStats;
			tStats.Add ( tRes.m_tAgentIOStats );
			tOut.SendUint64 ( tStats.m_iReadTime );
			tOut.SendDword ( tStats.m_iReadOps );
			tOut.SendUint64 ( tStats.m_iReadBytes );
			tOut.SendUint64 ( tStats.m_iWriteTime );
			tOut.SendDword ( tStats.m_iWriteOps );
			tOut.SendUint64 ( tStats.m_iWriteBytes );
		}

		if ( g_bCpuStats )
		{
			int64_t iCpuTime = tRes.m_iCpuTime + tRes.m_iAgentCpuTime;
			tOut.SendUint64 ( iCpuTime );
		}

		if ( bNeedPredictedTime )
			tOut.SendUint64 ( tRes.m_iPredictedTime + tRes.m_iAgentPredictedTime );
	}
	if ( bAgentMode && uMasterVer>=7 )
	{
		tOut.SendDword ( tRes.m_tStats.m_iFetchedDocs + tRes.m_iAgentFetchedDocs );
		tOut.SendDword ( tRes.m_tStats.m_iFetchedHits + tRes.m_iAgentFetchedHits );
		if ( uMasterVer>=8 )
			tOut.SendDword ( tRes.m_tStats.m_iSkips + tRes.m_iAgentFetchedSkips );
	}

	auto dWords = tRes.MakeSortedWordStat ();
	tOut.SendInt ( dWords.GetLength() );
	for( auto * pWord : dWords )
	{
		assert ( pWord );
		tOut.SendString ( pWord->first.cstr () );
		tOut.SendAsDword ( pWord->second.first );
		tOut.SendAsDword ( pWord->second.second );
		if ( bAgentMode )
			tOut.SendByte ( 0 ); // statistics have no expanded terms for now
	}
}


void HandleCommandSearch ( ISphOutputBuffer & tOut, WORD uVer, InputBuffer_c & tReq )
{
	MEMORY ( MEM_API_SEARCH );

	if ( !CheckCommandVersion ( uVer, VER_COMMAND_SEARCH, tOut ) )
		return;

	const WORD MIN_VERSION = 0x119;
	if ( uVer<MIN_VERSION )
	{
		SendErrorReply ( tOut, "client version is too old; upgrade your client (client is v.%d.%d, min is v.%d.%d)", uVer>>8, uVer&0xff, MIN_VERSION>>8, MIN_VERSION&0xff );
		return;
	}

	int iMasterVer = tReq.GetInt();
	if ( iMasterVer<0 || iMasterVer>VER_COMMAND_SEARCH_MASTER )
	{
		SendErrorReply ( tOut, "master-agent version mismatch; update me first, then update master!" );
		return;
	}
	WORD uMasterVer { WORD (iMasterVer) };
	bool bAgentMode = ( uMasterVer>0 );

	// parse request
	int iQueries = tReq.GetDword ();

	if ( g_iMaxBatchQueries>0 && ( iQueries<=0 || iQueries>g_iMaxBatchQueries ) )
	{
		SendErrorReply ( tOut, "bad multi-query count %d (must be in 1..%d range)", iQueries, g_iMaxBatchQueries );
		return;
	}

	assert ( iQueries>0 );
	CSphFixedVector<CSphQuery> dQueries { iQueries };
	for ( auto & dQuery: dQueries )
		if ( !ParseSearchQuery ( tReq, tOut, dQuery, uVer, uMasterVer ) )
			return;

	// run queries, send response
	SearchHandler_c tHandler { std::move (dQueries), !bAgentMode };
	const CSphQuery & q = tHandler.m_dQueries.First();
	myinfo::SetTaskInfo ( R"(api-search query="%s" comment="%s" table="%s")", q.m_sQuery.scstr (), q.m_sComment.scstr (), q.m_sIndexes.scstr () );
	tHandler.RunQueries();

	auto tReply = APIAnswer ( tOut, VER_COMMAND_SEARCH );
	ARRAY_FOREACH ( i, tHandler.m_dQueries )
		SendResult ( uVer, tOut, tHandler.m_dAggrResults[i], bAgentMode, tHandler.m_dQueries[i], uMasterVer );

	UpdateLastMeta ( tHandler.m_dAggrResults );
}
