//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sortbound.h"

#include "sortfilter.h"
#include "sphinxexpr.h"
#include "sphinxsearch.h"
#include "sphinxsort.h"


bool SortBound_c::Setup ( const CSphQuery & tQuery, const ISphSchema & tIndexSchema, const ISphMatchSorter & tSorter )
{
	m_pSorter = nullptr;
	m_sAttr = "";
	m_tValue = 0;
	m_bSet = false;
	m_bDescending = false;

	if ( tQuery.m_eSort!=SPH_SORT_EXTENDED || tSorter.IsGroupby() || tSorter.IsRandom() || tQuery.m_eJoinType!=JoinType_e::NONE || tQuery.HasKnn() || !tQuery.m_tScrollSettings.m_dAttrs.IsEmpty() )
		return false;

	const auto & tState = tSorter.GetState();
	const auto * pSchema = tSorter.GetSchema();
	int iAttr = tState.m_dAttrs[0];
	if ( !pSchema || tState.m_eKeypart[0]!=SPH_KEYPART_INT || iAttr<0 || iAttr>=pSchema->GetAttrsCount() )
		return false;

	const auto & tAttr = pSchema->GetAttr(iAttr);
	CSphString sAttr = tAttr.m_sName;
	bool bDirectAlias = false;
	for ( const auto & tItem : tQuery.m_dItems )
		if ( tItem.m_sAlias==sAttr )
		{
			if ( tItem.m_eAggrFunc!=SPH_AGGR_NONE || !tIndexSchema.GetAttr ( tItem.m_sExpr.cstr() ) )
				return false;

			sAttr = tItem.m_sExpr;
			bDirectAlias = true;
			break;
		}

	if ( tAttr.IsColumnarExpr() )
		tAttr.m_pExpr->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sAttr );
	else if ( tAttr.m_pExpr && !bDirectAlias )
		return false;

	const auto * pAttr = tIndexSchema.GetAttr ( sAttr.cstr() );
	if ( !pAttr || pAttr->IsJoined() || tAttr.IsJoined() || tAttr.m_eAggrFunc!=SPH_AGGR_NONE || pAttr->m_eAttrType!=tAttr.m_eAttrType )
		return false;

	if ( pAttr->m_eAttrType!=SPH_ATTR_INTEGER && pAttr->m_eAttrType!=SPH_ATTR_TIMESTAMP && pAttr->m_eAttrType!=SPH_ATTR_BIGINT )
		return false;

	m_sAttr = sAttr;
	m_tLocator = tState.m_tLocator[0];
	m_bDescending = ( tState.m_uAttrDesc & 1 )!=0;
	m_pSorter = &tSorter;
	return true;
}


bool SortBound_c::Update()
{
	if ( !m_pSorter )
		return false;

	bool bFull = false;
	const auto * pWorst = m_pSorter->GetWorst ( &bFull );
	if ( !bFull || !pWorst )
		return false;

	SphAttr_t tValue = pWorst->GetAttr(m_tLocator);
	if ( m_bSet && ( m_bDescending ? tValue<=m_tValue : tValue>=m_tValue ) )
		return false;

	m_tValue = tValue;
	m_bSet = true;
	return true;
}


bool CanUseSortBound ( const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs, bool bRanklessCount )
{
	return tQuery.m_iCutoff==-1 && dSorters.GetLength()==1 && !bRanklessCount
		&& !tQuery.m_bFacet && !tQuery.m_bFacetHead && !tArgs.m_uPackedFactorFlags
		&& !dSorters[0]->IsCutoffDisabled() && !dSorters[0]->IsPrecalc();
}


void FinalizeSortBound ( const SortBoundFilter_c & tFilter, const SortBound_c & tBound, ISphRanker & tRanker, CSphQueryResultMeta & tMeta )
{
	tMeta.m_bTotalMatchesApprox |= tFilter.HasPruned();
	if ( tFilter.HasPruned() )
	{
		tMeta.m_tIteratorStats.m_dIterators.Add ( { tBound.GetAttr(), tFilter.GetName() } );
		tMeta.m_tIteratorStats.m_iTotal = 1;
	}

	tRanker.SetCandidateFilter(nullptr);
}
