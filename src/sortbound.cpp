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
#include "costestimate.h"

#include "secondaryindex.h"
#include "sortfilter.h"
#include "sphinxint.h"
#include "sphinxexpr.h"
#include "sphinxsearch.h"
#include "sphinxsort.h"

#include <functional>
#include <numeric>
#include <queue>
#include <utility>
#include <vector>


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


void WarnSortBoundHint ( const CSphQuery & tQuery, CSphQueryResultMeta & tMeta, const char * szReason )
{
	const auto * pHint = tQuery.m_tIndexHints.GetSortBoundHint();
	if ( !pHint || !pHint->m_bForce || !szReason || !tMeta.m_sError.IsEmpty() )
		return;

	CSphString sHint = SphSprintf ( "hint error: SortBound(%s) unavailable: %s", pHint->m_sIndex.cstr(), szReason );
	tMeta.m_sWarning = tMeta.m_sWarning.IsEmpty() ? sHint : SphSprintf ( "%s; %s", tMeta.m_sWarning.cstr(), sHint.cstr() );
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


namespace
{
struct SortBlock_t
{
	SphAttr_t	m_tBound;
	RowID_t		m_tFirst;
	bool		m_bParent = false;

	bool operator < ( const SortBlock_t & tOther ) const
	{
		if ( m_tBound!=tOther.m_tBound )
			return m_tBound<tOther.m_tBound;

		return m_tFirst>tOther.m_tFirst;
	}
};


// Yields row IDs from the best-bound block first and stops when all remaining blocks fall outside the sorter's threshold.
class SortBoundScan_c final : public RowidIterator_i
{
public:
	SortBoundScan_c ( SortBound_c & tBound, const SortBoundScanPlan_t & tPlan );

	bool	GetNextRowIdBlock ( RowIdBlock_t & dRows ) final;
	int64_t	GetNumProcessed() const final	{ return m_iProcessed; }
	bool	WasCutoffHit() const final		{ return m_bPruned; }
	void	AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const final;
	bool	HintRowID ( RowID_t ) final		{ assert(0); return false; }
	void	SetCutoff ( int ) final			{ assert(0); }

private:
	// Queue parents six levels above the leaves; each covers up to 64 leaves.
	static constexpr int MINMAX_LEVEL_SHIFT = 6;
	SortBlock_t				ColumnarBlock ( RowID_t tBlock, bool bParent ) const;
	bool					NextLeaf();

	SortBound_c &				m_tBound;
	RowIdBoundaries_t			m_tRange;
	int							m_iBlockRows;
	std::priority_queue<SortBlock_t>	m_tBlockQueue;
	RowID_t						m_tNext = 0;
	RowID_t						m_tEnd = 0;
	RowID_t						m_dRows[DOCINFO_INDEX_FREQ];
	int64_t						m_iProcessed = 0;
	bool						m_bPruned = false;
	const columnar::BlockBounds_i * m_pColumnarBounds;
	int							m_iLeafLevel;
};


SortBoundScan_c::SortBoundScan_c ( SortBound_c & tBound, const SortBoundScanPlan_t & tPlan )
	: m_tBound(tBound)
	, m_tRange(tPlan.m_tRange)
	, m_iBlockRows(tPlan.m_iBlockRows)
	, m_pColumnarBounds(tPlan.m_pColumnarBounds)
	, m_iLeafLevel(tPlan.m_iLeafLevel)
{
	RowID_t tFirst = m_tRange.m_tMinRowID/m_iBlockRows;
	RowID_t tLast = m_tRange.m_tMaxRowID/m_iBlockRows;
	// If the tree has a level six steps above the leaves and this worker spans more than 64 leaves,
	// convert the inclusive leaf indices to parent indices so the queue starts at that level.
	const bool bMultiLevel = m_pColumnarBounds && m_iLeafLevel>=MINMAX_LEVEL_SHIFT && tLast-tFirst>=(1U<<MINMAX_LEVEL_SHIFT);
	if ( bMultiLevel )
	{
		tFirst >>= MINMAX_LEVEL_SHIFT;
		tLast >>= MINMAX_LEVEL_SHIFT;
	}

	std::vector<SortBlock_t> dBlocks;
	dBlocks.reserve(tLast-tFirst+1);
	for ( RowID_t i=tFirst; i<=tLast; ++i )
		if ( bMultiLevel )
			dBlocks.push_back ( ColumnarBlock ( i, true ) );
		else
			dBlocks.push_back ( { tBound.CompetitiveKey ( tPlan.GetBlockBound ( i, tBound.IsDescending() ) ), i*RowID_t(m_iBlockRows) } );

	m_tBlockQueue = std::priority_queue ( std::less<SortBlock_t>{}, std::move(dBlocks) );
}


SortBlock_t SortBoundScan_c::ColumnarBlock ( RowID_t tBlock, bool bParent ) const
{
	int iShift = bParent ? MINMAX_LEVEL_SHIFT : 0;
	auto tBounds = m_pColumnarBounds->GetMinMax ( m_iLeafLevel-iShift, tBlock );
	SphAttr_t tBest = m_tBound.IsDescending() ? tBounds.second : tBounds.first;
	return { m_tBound.CompetitiveKey(tBest), (tBlock<<iShift)*RowID_t(m_iBlockRows), bParent };
}


bool SortBoundScan_c::NextLeaf()
{
	m_tBound.Update();
	while ( !m_tBlockQueue.empty() )
	{
		auto tBlock = m_tBlockQueue.top();
		if ( m_tBound.IsSet() && tBlock.m_tBound<m_tBound.CompetitiveKey ( m_tBound.GetValue() ) )
		{
			m_bPruned = true;
			return false;
		}

		RowID_t tFirst = tBlock.m_tFirst;
		m_tBlockQueue.pop();

		if ( tBlock.m_bParent )
		{
			// The worker's row range may start or end inside this parent.
			RowID_t tFirstLeaf = tFirst/m_iBlockRows;
			RowID_t tLastLeaf = Min ( tFirstLeaf+(1U<<MINMAX_LEVEL_SHIFT)-1, m_tRange.m_tMaxRowID/m_iBlockRows );
			for ( RowID_t i=Max ( tFirstLeaf, m_tRange.m_tMinRowID/m_iBlockRows ); i<=tLastLeaf; ++i )
				m_tBlockQueue.push ( ColumnarBlock ( i, false ) );

			continue;
		}

		m_tNext = Max ( tFirst, m_tRange.m_tMinRowID );
		m_tEnd = RowID_t ( Min ( uint64_t(tFirst)+m_iBlockRows, uint64_t(m_tRange.m_tMaxRowID)+1 ) );
		return true;
	}

	return false;
}


bool SortBoundScan_c::GetNextRowIdBlock ( RowIdBlock_t & dRows )
{
	if ( m_tNext==m_tEnd && !NextLeaf() )
		return false;

	int iRows = Min ( int(m_tEnd-m_tNext), int(DOCINFO_INDEX_FREQ) );
	std::iota ( m_dRows, m_dRows+iRows, m_tNext );
	m_tNext += iRows;
	m_iProcessed += iRows;
	dRows = RowIdBlock_t ( m_dRows, iRows );
	return true;
}


void SortBoundScan_c::AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const
{
	dDesc.Add ( { m_tBound.GetAttr(), m_pColumnarBounds ? "SortBoundColumnar" : "SortBoundMinMax" } );
}
} // namespace


std::unique_ptr<RowidIterator_i> CreateSortBoundScan ( SortBound_c & tBound, const SortBoundScanPlan_t & tPlan )
{
	return std::make_unique<SortBoundScan_c> ( tBound, tPlan );
}
