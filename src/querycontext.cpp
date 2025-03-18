//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "querycontext.h"

#include "sphinxint.h"
#include "sphinxsearch.h"
#include "sphinxsort.h"
#include "docstore.h"


static ESphEvalStage GetEarliestStage ( ESphEvalStage eStage, const CSphColumnInfo & tIn, const CSphVector<const ISphSchema *> & dSchemas )
{
	for ( const auto * pSchema : dSchemas )
	{
		const CSphColumnInfo * pCol = pSchema->GetAttr ( tIn.m_sName.cstr() );
		if ( !pCol || ( pCol->IsColumnar() && pCol->m_eStage==SPH_EVAL_STATIC ) )
			continue;

		eStage = Min ( eStage, pCol->m_eStage );
	}

	return eStage;
}

///////////////////////////////////////////////////////////////////////////////

CSphQueryContext::CSphQueryContext ( const CSphQuery & tQuery )
	: m_tQuery ( tQuery )
{
}


void CSphQueryContext::ResetFilters()
{
	m_pFilter.reset();
	m_pWeightFilter.reset();
	m_dUserVals.Reset();
}


void CSphQueryContext::BindWeights ( const CSphQuery & tQuery, const CSphSchema & tSchema, CSphString & sWarning )
{
	const int HEAVY_FIELDS = SPH_MAX_FIELDS;

	// defaults
	m_iWeights = Min ( tSchema.GetFieldsCount(), HEAVY_FIELDS );
	for ( int i=0; i<m_iWeights; ++i )
		m_dWeights[i] = 1;

	// name-bound weights
	CSphString sFieldsNotFound;
	if ( !tQuery.m_dFieldWeights.IsEmpty() )
	{
		for ( auto& tWeight : tQuery.m_dFieldWeights )
		{
			int j = tSchema.GetFieldIndex ( tWeight.first.cstr() );
			if ( j<0 )
			{
				if ( sFieldsNotFound.IsEmpty() )
					sFieldsNotFound = tWeight.first;
				else
					sFieldsNotFound.SetSprintf ( "%s %s", sFieldsNotFound.cstr(), tWeight.first.cstr() );
			}

			if ( j>=0 && j<HEAVY_FIELDS )
				m_dWeights[j] = tWeight.second;
		}

		if ( !sFieldsNotFound.IsEmpty() )
			sWarning.SetSprintf ( "Fields specified in field_weights option not found: [%s]", sFieldsNotFound.cstr() );

		return;
	}

	// order-bound weights
	if ( !tQuery.m_dWeights.IsEmpty() )
	{
		for ( int i=0, iLim=Min ( m_iWeights, tQuery.m_dWeights.GetLength() ); i<iLim; ++i )
			m_dWeights[i] = (int) tQuery.m_dWeights[i];
	}
}


bool CSphQueryContext::SetupCalc ( CSphQueryResultMeta & tMeta, const ISphSchema & tInSchema, const ISphSchema & tSchema, const BYTE * pBlobPool, const columnar::Columnar_i * pColumnar, const CSphVector<const ISphSchema *> & dInSchemas )
{
	m_dCalcFilter.Resize(0);
	m_dCalcSort.Resize(0);
	m_dCalcFinal.Resize(0);

	m_dCalcFilterPtrAttrs.Resize(0);
	m_dCalcSortPtrAttrs.Resize(0);

	// quickly verify that all my real attributes can be stashed there
	if ( tInSchema.GetAttrsCount() < tSchema.GetAttrsCount() )
	{
		tMeta.m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema mismatch (incount=%d, mycount=%d)",
			tInSchema.GetAttrsCount(), tSchema.GetAttrsCount() );
		return false;
	}

	// now match everyone
	for ( int iIn=0; iIn<tInSchema.GetAttrsCount(); iIn++ )
	{
		const CSphColumnInfo & tIn = tInSchema.GetAttr(iIn);

		// recalculate stage as sorters set column at earlier stage
		// FIXME!!! should we update column?
		ESphEvalStage eStage = GetEarliestStage ( tIn.m_eStage, tIn, dInSchemas );

		switch ( eStage )
		{
		case SPH_EVAL_STATIC:
		{
			// this check may significantly slow down queries with huge schema attribute count
#ifndef NDEBUG
			const CSphColumnInfo * pMy = tSchema.GetAttr ( tIn.m_sName.cstr() );
			if ( !pMy )
			{
				tMeta.m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema attr missing from table-schema (in=%s)",
					sphDumpAttr(tIn).cstr() );
				return false;
			}

			// static; check for full match
			if (!( tIn==*pMy ))
			{
				assert ( 0 );
				tMeta.m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema mismatch (in=%s, my=%s)",
					sphDumpAttr(tIn).cstr(), sphDumpAttr(*pMy).cstr() );
				return false;
			}
#endif
			break;
		}

		case SPH_EVAL_PREFILTER:
		case SPH_EVAL_PRESORT:
		case SPH_EVAL_FINAL:
		{
			ISphExprRefPtr_c pExpr { tIn.m_pExpr };
			if ( !pExpr )
			{
				tMeta.m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema expression missing evaluator (stage=%d, in=%s)",
					(int)eStage, sphDumpAttr(tIn).cstr() );
				return false;
			}

			// an expression that index/searcher should compute
			ContextCalcItem_t tCalc;
			tCalc.m_eType = tIn.m_eAttrType;
			tCalc.m_tLoc = tIn.m_tLocator;
			tCalc.m_pExpr = std::move(pExpr);
			tCalc.m_pExpr->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
			tCalc.m_pExpr->Command ( SPH_EXPR_SET_COLUMNAR, (void*)pColumnar );

			switch ( eStage )
			{
			case SPH_EVAL_PREFILTER:	AddToFilterCalc(tCalc); break;
			case SPH_EVAL_PRESORT:		AddToSortCalc(tCalc); break;
			case SPH_EVAL_FINAL:		m_dCalcFinal.Add(tCalc); break;
			default:					break;
			}
			break;
		}

		case SPH_EVAL_SORTER:
			// sorter tells it will compute itself; so just skip it
		case SPH_EVAL_POSTLIMIT:
			break;

		default:
			tMeta.m_sError.SetSprintf ( "INTERNAL ERROR: unhandled eval stage=%d", (int)eStage );
			return false;
		}
	}

	// ok, we can emit matches in this schema (incoming for sorter, outgoing for index/searcher)
	return true;
}


bool CSphQueryContext::CreateFilters ( CreateFilterContext_t & tCtx, CSphString & sError, CSphString & sWarning )
{
	if ( !tCtx.m_pFilters || tCtx.m_pFilters->IsEmpty () )
		return true;
	if ( !sphCreateFilters ( tCtx, sError, sWarning ) )
		return false;

	m_pFilter = std::move ( tCtx.m_pFilter );
	m_pWeightFilter = std::move ( tCtx.m_pWeightFilter );
	m_dUserVals.SwapData ( tCtx.m_dUserVals );
	return true;
}


void CSphQueryContext::AddToFilterCalc ( const ContextCalcItem_t & tCalc )
{
	m_dCalcFilter.Add(tCalc);
	if ( sphIsDataPtrAttr ( tCalc.m_eType ) )
		m_dCalcFilterPtrAttrs.Add ( m_dCalcFilter.GetLength()-1 );
}


void CSphQueryContext::AddToSortCalc ( const ContextCalcItem_t & tCalc )
{
	m_dCalcSort.Add(tCalc);
	if ( sphIsDataPtrAttr ( tCalc.m_eType ) )
		m_dCalcSortPtrAttrs.Add ( m_dCalcSort.GetLength()-1 );
}


void CSphQueryContext::FreeDataFilter ( CSphMatch & tMatch ) const
{
	FreeDataPtrAttrs ( tMatch, m_dCalcFilter, m_dCalcFilterPtrAttrs );
}


void CSphQueryContext::FreeDataSort ( CSphMatch & tMatch ) const
{
	FreeDataPtrAttrs ( tMatch, m_dCalcSort, m_dCalcSortPtrAttrs );
}


void CSphQueryContext::ExprCommand ( ESphExprCommand eCmd, void * pArg )
{
	ARRAY_FOREACH ( i, m_dCalcFilter )
		m_dCalcFilter[i].m_pExpr->Command ( eCmd, pArg );

	ARRAY_FOREACH ( i, m_dCalcSort )
		m_dCalcSort[i].m_pExpr->Command ( eCmd, pArg );

	ARRAY_FOREACH ( i, m_dCalcFinal )
		m_dCalcFinal[i].m_pExpr->Command ( eCmd, pArg );
}


void CSphQueryContext::SetBlobPool ( const BYTE * pBlobPool )
{
	ExprCommand ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
	if ( m_pFilter )
		m_pFilter->SetBlobStorage ( pBlobPool );
	if ( m_pWeightFilter )
		m_pWeightFilter->SetBlobStorage ( pBlobPool );
}


void CSphQueryContext::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	ExprCommand ( SPH_EXPR_SET_COLUMNAR, (void*)pColumnar );
}


void CSphQueryContext::SetDocstore ( const Docstore_i * pDocstore, int64_t iDocstoreSessionId )
{
	DocstoreSession_c::InfoRowID_t tSessionInfo;
	tSessionInfo.m_pDocstore = pDocstore;
	tSessionInfo.m_iSessionId = iDocstoreSessionId;

	ExprCommand ( SPH_EXPR_SET_DOCSTORE_ROWID, &tSessionInfo );
}


/// FIXME, perhaps
/// this rather crappy helper class really serves exactly 1 (one) simple purpose
///
/// it passes a sorting queue internals (namely, weight and float sortkey, if any,
/// of the current-worst queue element) to the MIN_TOP_WORST() and MIN_TOP_SORTVAL()
/// expression classes that expose those to the cruel outside world
///
/// all the COM-like EXTRA_xxx message back and forth is needed because expressions
/// are currently parsed and created earlier than the sorting queue
///
/// that also is the reason why we mischievously return 0 instead of clearly failing
/// with an error when the sortval is not a dynamic float; by the time we are parsing
/// expressions, we do not *yet* know that; but by the time we create a sorting queue,
/// we do not *want* to leak select expression checks into it
///
/// alternatively, we probably want to refactor this and introduce Bind(), to parse
/// expressions once, then bind them to actual searching contexts (aka index or segment,
/// and ranker, and sorter, and whatever else might be referenced by the expressions)
struct ContextExtra final : public ISphExtra
{
	ISphRanker * m_pRanker;
	ISphMatchSorter * m_pSorter;

	ContextExtra ( ISphRanker* pRanker, ISphMatchSorter* pSorter)
		: m_pRanker ( pRanker )
		, m_pSorter ( pSorter )
	{}

	bool ExtraDataImpl ( ExtraData_e eData, void ** ppArg ) final
	{
		if ( eData!=EXTRA_GET_QUEUE_WORST && eData!=EXTRA_GET_QUEUE_SORTVAL )
			return m_pRanker->ExtraData ( eData, ppArg );

		if ( !m_pSorter )
			return false;

		const CSphMatch * pWorst = m_pSorter->GetWorst();
		if ( !pWorst )
			return false;

		if ( eData==EXTRA_GET_QUEUE_WORST )
		{
			*ppArg = (void*)pWorst;
			return true;
		};

		assert ( eData==EXTRA_GET_QUEUE_SORTVAL );
		const CSphMatchComparatorState & tCmp = m_pSorter->GetState();
		if ( tCmp.m_eKeypart[0]==SPH_KEYPART_FLOAT && tCmp.m_tLocator[0].m_bDynamic
			&& tCmp.m_tLocator[0].m_iBitCount==32 && ( tCmp.m_tLocator[0].m_iBitOffset%32==0 )
			&& tCmp.m_dAttrs[1]==-1 )
		{
			*(int*)ppArg = tCmp.m_tLocator[0].m_iBitOffset/32;
			return true;
		}

		// min_top_sortval() only works with order by float_expr for now
		return false;
	}
};


void CSphQueryContext::SetupExtraData ( ISphRanker * pRanker, ISphMatchSorter * pSorter )
{
	ContextExtra tExtra ( pRanker, pSorter );
	ExprCommand ( SPH_EXPR_SET_EXTRA_DATA, &tExtra );
}

void CSphQueryContext::SetPackedFactor ( DWORD uFlags )
{
	m_uPackedFactorFlags = uFlags;
	// query with the packed factors should pass the full match flow as query cache does not has these factors
	if ( !m_bSkipQCache )
		m_bSkipQCache = ( m_uPackedFactorFlags!=SPH_FACTOR_DISABLE );
}

DWORD CSphQueryContext::GetPackedFactor () const
{
	return m_uPackedFactorFlags;
}

