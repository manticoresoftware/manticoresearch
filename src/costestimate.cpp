//
//
// Copyright (c) 2018-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "costestimate.h"

#include <cfloat>
#include <math.h>
#include <algorithm>
#include <utility>

#include "sphinxint.h"
#include "sphinxsort.h"
#include "sortbound.h"
#include "querycontext.h"
#include "columnarfilter.h"
#include "secondaryindex.h"
#include "geodist.h"
#include "histogram.h"
#include "std/sys.h"


int64_t EstimateFilterSelectivity ( const CSphFilterSettings & tSettings, const CreateFilterContext_t & tCtx )
{
	if ( !tCtx.m_pHistograms )
		return tCtx.m_iTotalDocs;

	Histogram_i * pHistogram = tCtx.m_pHistograms->Get ( tSettings.m_sAttrName );
	if ( !pHistogram || pHistogram->IsOutdated() )
		return tCtx.m_iTotalDocs;

	HistogramRset_t tEstimate;
	if ( !pHistogram->EstimateRsetSize ( tSettings, tEstimate ) )
		return tCtx.m_iTotalDocs;

	return tEstimate.m_iTotal;
}


class FilterSelectivityEstimator_c
{
public:
				FilterSelectivityEstimator_c ( const VecTraits_T<CSphFilterSettings> & dFilters, const VecTraits_T<FilterTreeItem_t> * pFilterTree, const CreateFilterContext_t & tCtx );

	int64_t		Estimate() const;

private:
	const VecTraits_T<CSphFilterSettings> &	m_dFilters;
	const VecTraits_T<FilterTreeItem_t> *	m_pFilterTree = nullptr;
	const CreateFilterContext_t &			m_tCtx;
	const int64_t							m_iTotalDocs = 0;

	int64_t		ClampEstimate ( int64_t iValue ) const	{ return std::clamp ( iValue, (int64_t)0, m_iTotalDocs ); }
	int64_t		EstimateLeaf ( int iFilterItem ) const;
	int64_t		EstimateImplicitAnd () const;
	int64_t		EstimateNode ( int iNode ) const;
};


FilterSelectivityEstimator_c::FilterSelectivityEstimator_c ( const VecTraits_T<CSphFilterSettings> & dFilters, const VecTraits_T<FilterTreeItem_t> * pFilterTree, const CreateFilterContext_t & tCtx )
	: m_dFilters ( dFilters )
	, m_pFilterTree ( pFilterTree )
	, m_tCtx ( tCtx )
	, m_iTotalDocs ( Max ( int64_t(0), tCtx.m_iTotalDocs ) )
{}


int64_t FilterSelectivityEstimator_c::Estimate () const
{
	if ( !m_iTotalDocs )
		return 0;

	if ( !m_pFilterTree || !m_pFilterTree->GetLength() )
		return EstimateImplicitAnd();

	return EstimateNode ( m_pFilterTree->GetLength()-1 );
}


int64_t FilterSelectivityEstimator_c::EstimateLeaf ( int iFilterItem ) const
{
	if ( iFilterItem<0 || iFilterItem>=m_dFilters.GetLength() )
		return m_iTotalDocs;

	return ClampEstimate ( EstimateFilterSelectivity ( m_dFilters[iFilterItem], m_tCtx ) );
}


int64_t FilterSelectivityEstimator_c::EstimateImplicitAnd () const
{
	double fEst = m_iTotalDocs;
	double fTotal = m_iTotalDocs;

	for ( const auto & tFilter : m_dFilters )
	{
		int64_t iSel = ClampEstimate ( EstimateFilterSelectivity ( tFilter, m_tCtx ) );
		fEst = fEst*iSel/fTotal;
	}

	return ClampEstimate ( (int64_t)round(fEst) );
}


int64_t FilterSelectivityEstimator_c::EstimateNode ( int iNode ) const
{
	assert(m_pFilterTree);

	if ( iNode<0 || iNode>=m_pFilterTree->GetLength() )
		return m_iTotalDocs;

	const auto & tNode = (*m_pFilterTree)[iNode];
	if ( tNode.m_iFilterItem!=-1 )
		return EstimateLeaf ( tNode.m_iFilterItem );

	int64_t iLeft = EstimateNode ( tNode.m_iLeft );
	int64_t iRight = EstimateNode ( tNode.m_iRight );

	double fTotal = m_iTotalDocs;
	if ( tNode.m_bOr )
	{
		double fEst = (double)iLeft + (double)iRight - ((double)iLeft*(double)iRight) / fTotal;
		return ClampEstimate ( (int64_t)fEst );
	}

	double fEst = ((double)iLeft * (double)iRight) / fTotal;
	return ClampEstimate ( (int64_t)round(fEst) );
}

////////////////////////////////////////////////////////////////////////////////

int64_t EstimateFilterSelectivity ( const VecTraits_T<CSphFilterSettings> & dFilters, const VecTraits_T<FilterTreeItem_t> * pFilterTree, const CreateFilterContext_t & tCtx )
{
	if ( tCtx.m_iTotalDocs<=0 )
		return 0;

	if ( !dFilters.GetLength() )
		return tCtx.m_iTotalDocs;

	FilterSelectivityEstimator_c tEstimator ( dFilters, pFilterTree, tCtx );
	return tEstimator.Estimate();
}


////////////////////////////////////////////////////////////////////////////////

static float EstimateMTCost ( float fCost, int iThreads, float fKPerf, float fBPerf )
{
	if ( iThreads==1 )
		return fCost;

	int iMaxThreads = GetNumLogicalCPUs();
	float fMaxPerfCoeff = fKPerf*iMaxThreads + fBPerf;
	float fMinCost = fCost/fMaxPerfCoeff;

	if ( iThreads==iMaxThreads )
		return fMinCost;

	const float fX1 = 1.0f;
	float fX2 = iMaxThreads;
	float fY1 = fCost;
	float fY2 = fMinCost;

	// cost = A/sqrt(num_threads) + B
	float fA = ( fY2-fY1 ) / ( 1.0f/float(sqrt(fX2)) - 1.0f/float(sqrt(fX1)) );
	float fB = fY1 - fA / float(sqrt(fX1));
	float fX = iThreads;
	float fY = fA/float(sqrt(fX)) + fB;

	return fY;
}


float EstimateMTCost ( float fCost, int iThreads )
{
	const float fKPerf = 0.45f;
	const float fBPerf = 1.40f;

	return EstimateMTCost ( fCost, iThreads, fKPerf, fBPerf );
}


float EstimateMTCostCS ( float fCost, int iThreads )
{
	const float fKPerf = 0.16f;
	const float fBPerf = 1.38f;

	return EstimateMTCost ( fCost, iThreads, fKPerf, fBPerf );
}


float EstimateMTCostSI ( float fCost, int iThreads )
{
	const float fKPerf = 0.10f;
	const float fBPerf = 1.56f;

	return EstimateMTCost ( fCost, iThreads, fKPerf, fBPerf );
}


float EstimateMTCostSIFT ( float fCost, int iThreads )
{
	const float fKPerf = 0.235f;
	const float fBPerf = 1.25f;

	return EstimateMTCost ( fCost, iThreads, fKPerf, fBPerf );
}

/////////////////////////////////////////////////////////////////////

class CostEstimate_c : public CostEstimate_i
{
	friend struct SortBoundScanCost_t;
	friend float CalcIntersectCost ( int64_t iDocs );
	friend float CalcFTIntersectCost ( const NodeEstimate_t & tEst1, const NodeEstimate_t & tEst2, int64_t iTotalDocs, int iDocsPerBlock1, int iDocsPerBlock2 );

public:
			CostEstimate_c ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx, int iCutoff );

	float	CalcQueryCost() final;

private:
	static constexpr float SCALE = 1.0f/1000000.0f;

	static constexpr float COST_PUSH					= 6.0f;
	static constexpr float COST_PUSH_IG					= 3.0f;
	static constexpr float COST_FILTER					= 8.5f;
	static constexpr float COST_COLUMNAR_FILTER			= 4.0f;
	static constexpr float COST_INDEX_READ_SINGLE		= 4.0f;
	static constexpr float COST_INDEX_READ_BITMAP		= 4.5f;
	static constexpr float COST_INDEX_UNION_COEFF		= 4.0f;
	static constexpr float COST_LOOKUP_READ				= 20.0f;
	static constexpr float COST_INDEX_ITERATOR_INIT		= 30.0f;
	static constexpr float COST_ITERATOR_INTERSECT		= 20.0f;

	const CSphVector<SecondaryIndexInfo_t> &	m_dSIInfo;
	const SelectIteratorCtx_t &					m_tCtx;
	int											m_iCutoff = -1;
	CSphVector<int>								m_dSorted;

	static float	Cost_Filter ( int64_t iDocs, float fComplexity )		{ return COST_FILTER*fComplexity*iDocs*SCALE; }
	static float	Cost_BlockFilter ( int64_t iDocs, float fComplexity )	{ return Cost_Filter ( iDocs/DOCINFO_INDEX_FREQ, fComplexity ); }
	static float	Cost_ColumnarFilter ( int64_t iDocs, float fComplexity ){ return COST_COLUMNAR_FILTER*fComplexity*iDocs*SCALE; }
	static float	Cost_Push ( int64_t iDocs )								{ return COST_PUSH*iDocs*SCALE; }
	static float	Cost_PushImplicitGroupby ( int64_t iDocs )				{ return COST_PUSH_IG*iDocs*SCALE; }

	static float	Cost_IndexReadSingle ( int64_t iDocs )					{ return COST_INDEX_READ_SINGLE*iDocs*SCALE; }
	static float	Cost_IndexReadBitmap ( int64_t iDocs )					{ return COST_INDEX_READ_BITMAP*iDocs*SCALE; }
	static float	Cost_IndexUnionQueue ( int64_t iDocs )					{ return COST_INDEX_UNION_COEFF*iDocs*log2f(iDocs)*SCALE; }
	static float	Cost_LookupRead ( int64_t iDocs )						{ return COST_LOOKUP_READ*iDocs*SCALE; }
	static float	Cost_IndexIteratorInit ( int64_t iNumIterators )		{ return COST_INDEX_ITERATOR_INIT*iNumIterators*SCALE; }

	float	CalcFilterCost ( const SecondaryIndexInfo_t & tIndex, const CSphFilterSettings & tFilter, bool bFromIterator, bool bFilterOverExpr, float fDocsLeft ) const;
	float	CalcLookupCost ( const SecondaryIndexInfo_t & tIndex ) const;
	float	CalcAnalyzerCost ( const SecondaryIndexInfo_t & tIndex, const CSphFilterSettings & tFilter, float fDocsLeft ) const;
	float	CalcIndexCost ( const SecondaryIndexInfo_t & tIndex, const CSphFilterSettings & tFilter, float fDocsLeft ) const;

	float	CalcIteratorIntersectCost ( float fFirstIteratorDocs, int iNumIterators );
	float	CalcPushCost ( float fDocsAfterFilters ) const;
	float	CalcMTCost ( float fCost ) const	{ return EstimateMTCost ( fCost, m_tCtx.m_iThreads );}
	float	CalcMTCostCS ( float fCost ) const	{ return EstimateMTCostCS ( fCost, m_tCtx.m_iThreads );}
	float	CalcMTCostSI ( float fCost ) const	{ return EstimateMTCostSI ( fCost, m_tCtx.m_iThreads ); }

	bool	NeedBitmapUnion ( int iNumIterators ) const;
	uint32_t CalcNumSIIterators ( const CSphFilterSettings & tFilter, int64_t iDocs ) const;
	int64_t	ApplyCutoff ( int64_t iDocs ) const;

	void	SortIndexes();
	int		GetNumIndexes() const								{ return m_dSIInfo.GetLength(); }
	const SecondaryIndexInfo_t & GetIndex ( int iIndex ) const	{ return m_dSIInfo[m_dSorted[iIndex]]; }
	const CSphFilterSettings & GetFilter ( int iIndex ) const	{ return m_tCtx.m_dFilters[m_dSorted[iIndex]]; }
	bool	IsFilterOverExpr ( int iIndex ) const;
};


// Compare scan strategies for one worker's row range.
struct SortBoundScanCost_t
{
	int64_t		m_iRows;
	int64_t		m_iMatches;
	int			m_iCapacity;
	float		m_fFilterComplexity = 0.0f;
	float		m_fOrdinaryCost = FLT_MAX;

	void		EstimateOrdinary ( const SelectIteratorCtx_t & tCtx, int iFilter, CSphVector<SecondaryIndexInfo_t> & dPlan, float fBestCost, int64_t iChunkMatches );
	float		CalcOrdinaryCost() const;
	float		CalcSortBoundCost ( int64_t iBoundsRead, int64_t iBlocks, int64_t iVisitedBlocks, int iBlockRows ) const;
};


// Estimate the cost of running the already selected ordinary filter plan over one worker's row range.
// The sort-bound planner compares against this cost, using filter complexity to estimate a plain scan if no plan cost is available.
void SortBoundScanCost_t::EstimateOrdinary ( const SelectIteratorCtx_t & tCtx, int iFilter, CSphVector<SecondaryIndexInfo_t> & dPlan, float fBestCost, int64_t iChunkMatches )
{
	SecondaryIndexInfo_t tDefault;
	m_fFilterComplexity = CalcGetFilterComplexity ( dPlan.IsEmpty() ? tDefault : dPlan[iFilter], tCtx.m_dFilters[iFilter], tCtx );
	if ( fBestCost==FLT_MAX )
		return;

	// Price the selected plan for one worker, using the exact predicate frequency when available.
	if ( tCtx.m_iThreads!=1 || ( iChunkMatches>=0 && iChunkMatches!=dPlan[iFilter].m_iRsetEstimate ) )
	{
		SelectIteratorCtx_t tWorker = tCtx;
		tWorker.m_iThreads = 1;
		if ( iChunkMatches>=0 )
			dPlan[iFilter].m_iRsetEstimate = iChunkMatches;

		std::unique_ptr<CostEstimate_i> pEstimate ( CreateCostEstimate ( dPlan, tWorker, -1 ) );
		fBestCost = pEstimate->CalcQueryCost();
	}

	m_fOrdinaryCost = fBestCost*(double(m_iRows)/tCtx.m_iTotalDocs);
}


float SortBoundScanCost_t::CalcOrdinaryCost() const
{
	if ( m_fOrdinaryCost!=FLT_MAX )
		return m_fOrdinaryCost;

	return CostEstimate_c::Cost_Filter ( m_iRows, m_fFilterComplexity ) + CostEstimate_c::Cost_Push(m_iMatches);
}


float SortBoundScanCost_t::CalcSortBoundCost ( int64_t iBoundsRead, int64_t iBlocks, int64_t iVisitedBlocks, int iBlockRows ) const
{
	using Cost = CostEstimate_c;
	int64_t iVisited = Min ( m_iRows, iVisitedBlocks*iBlockRows );
	int64_t iPushed = int64_t(ceil ( double(m_iMatches)*iVisited/m_iRows ));

	// Read the block bounds and build a priority queue. Each visit removes the block with the best remaining bound.
	int64_t iHeapWork = iBoundsRead + iBlocks + iVisitedBlocks*sphLog2(iBlocks);
	return Cost::Cost_Filter ( iHeapWork, 1.0f ) + Cost::Cost_Filter ( iVisited, m_fFilterComplexity ) + Cost::Cost_Push(iPushed);
}


SphAttr_t SortBoundScanPlan_t::GetBlockBound ( RowID_t tBlock, bool bMax ) const
{
	if ( IsColumnar() )
	{
		auto tBounds = m_pColumnarBounds->GetMinMax ( m_iLeafLevel, tBlock );
		return bMax ? tBounds.second : tBounds.first;
	}

	return sphGetRowAttr ( m_pMinMax+(uint64_t(tBlock)*2+int(bMax))*m_iStride, m_tLocator );
}


template <int NUM_SAMPLES>
static int64_t EstimateVisitedBlocks ( RowID_t tFirst, int64_t iBlocks, int64_t iNeeded, const SortBound_c & tBound, const SortBoundScanPlan_t & tPlan )
{
	// Sample evenly across the worker's range. Store (worst, best) keys with larger values ranking first for either sort direction.
	std::pair<SphAttr_t,SphAttr_t> dSample[NUM_SAMPLES];
	int iSample = int(Min ( iBlocks, int64_t(NUM_SAMPLES) ));
	for ( int i=0; i<iSample; ++i )
	{
		RowID_t tBlock = tFirst+(iBlocks-1)*i/(iSample-1);
		dSample[i] = { tBound.CompetitiveKey ( tPlan.GetBlockBound ( tBlock, !tBound.IsDescending() ) ), tBound.CompetitiveKey ( tPlan.GetBlockBound ( tBlock, tBound.IsDescending() ) ) };
	}

	// Rank blocks by their worst key because every row in a block is at least as competitive as that key.
	std::sort ( dSample, dSample+iSample, std::greater<>() );

	// Map the last of the iNeeded blocks into the sample. Its worst key estimates the threshold when the sorter fills.
	int iPivot = int((iNeeded-1)*iSample/iBlocks);
	int iRemaining = iSample-iPivot-1;
	if ( !iRemaining )
		return iNeeded;

	// Count remaining sample blocks whose best key reaches the threshold. A tie can still win on a secondary sort key.
	int iOverlap = int(std::count_if ( dSample+iPivot+1, dSample+iSample, [&]( const auto & tBlock ){ return tBlock.second>=dSample[iPivot].first; } ));

	// Apply the sampled overlap fraction to the blocks beyond iNeeded, rounding up to a whole block.
	return iNeeded + ( (iBlocks-iNeeded)*iOverlap+iRemaining-1 )/iRemaining;
}


static bool ShouldUseSortBoundScan ( const SortBoundScanCost_t & tCost, const SortBoundScanPlan_t & tPlan, const SortBound_c & tBound )
{
	int iBlockRows = tPlan.m_iBlockRows;
	RowID_t tFirst = tPlan.m_tRange.m_tMinRowID/iBlockRows;
	RowID_t tLast = tPlan.m_tRange.m_tMaxRowID/iBlockRows;
	if ( tFirst==tLast )
		return false;

	int64_t iBlocks = int64_t(tLast)-tFirst+1;
	int64_t iBoundsRead = iBlocks;

	// Estimate how many blocks fill the sorter if matching rows are distributed uniformly.
	int64_t iNeeded = int64_t(ceil ( double(tCost.m_iCapacity)*tCost.m_iRows/(double(tCost.m_iMatches)*iBlockRows) ));
	if ( iNeeded>=iBlocks )
		return false;

	const float fOrdinaryCost = tCost.CalcOrdinaryCost();
	const float fOptimisticCost = tCost.CalcSortBoundCost ( iBoundsRead, iBlocks, iNeeded, iBlockRows );
	if ( fOptimisticCost>=fOrdinaryCost )
		return false;

	constexpr int INITIAL_SAMPLE_SIZE = 8;
	constexpr int REFINED_SAMPLE_SIZE = 64;
	int64_t iVisited = EstimateVisitedBlocks<INITIAL_SAMPLE_SIZE> ( tFirst, iBlocks, iNeeded, tBound, tPlan );
	if ( iVisited==iBlocks )
		return false;

	if ( iVisited==iNeeded )
		return true;

	const float fSampledCost = tCost.CalcSortBoundCost ( iBoundsRead, iBlocks, iVisited, iBlockRows );
	if ( fSampledCost<fOrdinaryCost )
		return true;

	// One overlap in a small sample can stand for thousands of blocks. Refine a close decision.
	if ( iBlocks<=INITIAL_SAMPLE_SIZE )
		return false;

	iVisited = EstimateVisitedBlocks<REFINED_SAMPLE_SIZE> ( tFirst, iBlocks, iNeeded, tBound, tPlan );
	if ( iVisited==iBlocks )
		return false;

	const float fRefinedCost = tCost.CalcSortBoundCost ( iBoundsRead, iBlocks, iVisited, iBlockRows );
	return fRefinedCost<fOrdinaryCost;
}


static int64_t GetSortBoundMatchCount ( const CSphIndex & tIndex, const CSphFilterSettings * pFilter, ESphCollation eCollation )
{
	if ( !pFilter )
		return tIndex.GetCount();

	if ( eCollation==SPH_COLLATION_DEFAULT && ( pFilter->m_eType==SPH_FILTER_STRING || ( pFilter->m_eType==SPH_FILTER_VALUES && pFilter->GetNumValues()==1 ) ) )
	{
		// A single SI lookup gives the frequency even for heavily skewed strings.
		CSphString sAttr;
		return tIndex.GetCountFilter ( *pFilter, sAttr );
	}

	return -1;
}


namespace
{
class SortBoundScanPlanner_c
{
public:
										SortBoundScanPlanner_c ( const SelectIteratorCtx_t & tSelect, const CreateFilterContext_t & tFlx, const CSphQueryContext & tCtx, const VecTraits_T<ISphMatchSorter *> & dSorters, const DWORD * pMinMax, SortBound_c & tBound, const char * & szReason );

	std::optional<SortBoundScanPlan_t>	Plan ( const CSphMultiQueryArgs & tArgs, const CSphIndex & tIndex, bool bSelectIterators, CSphVector<SecondaryIndexInfo_t> & dOrdinaryPlan, CSphString & sWarning );

private:
	static constexpr const char * BOUNDS_UNAVAILABLE = "block bounds are unavailable for this row range or sort attribute";

	const SelectIteratorCtx_t &				m_tSelect;
	const CreateFilterContext_t &			m_tFlx;
	const CSphQueryContext &				m_tCtx;
	const VecTraits_T<ISphMatchSorter *> &	m_dSorters;
	const DWORD *							m_pMinMax;
	SortBound_c &							m_tBound;
	const char * &							m_szReason;

	SortBoundScanPlan_t			m_tPlan;
	bool						m_bForce = false;
	const CSphColumnInfo *		m_pSortAttr = nullptr;
	const CSphFilterSettings *	m_pRowIdFilter = nullptr;
	const CSphFilterSettings *	m_pFilter = nullptr;

	bool						SetupQuery ( const CSphMultiQueryArgs & tArgs );
	bool						SetupRangeAndSortAttr();
	bool						SetupFilter();
	bool						EstimateCost ( SortBoundScanCost_t & tCost, const CSphIndex & tIndex, bool bSelectIterators, CSphVector<SecondaryIndexInfo_t> & dOrdinaryPlan, CSphString & sWarning ) const;
	bool						SelectBlockScan ( const CSphIndex & tIndex, bool bSelectIterators, CSphVector<SecondaryIndexInfo_t> & dOrdinaryPlan, CSphString & sWarning );
	bool						Reject ( const char * szReason );
	bool						SetupBlocks ( const CSphColumnInfo & tAttr );
};


SortBoundScanPlanner_c::SortBoundScanPlanner_c ( const SelectIteratorCtx_t & tSelect, const CreateFilterContext_t & tFlx, const CSphQueryContext & tCtx, const VecTraits_T<ISphMatchSorter *> & dSorters, const DWORD * pMinMax, SortBound_c & tBound, const char * & szReason )
	: m_tSelect(tSelect)
	, m_tFlx(tFlx)
	, m_tCtx(tCtx)
	, m_dSorters(dSorters)
	, m_pMinMax(pMinMax)
	, m_tBound(tBound)
	, m_szReason(szReason)
{}


bool SortBoundScanPlanner_c::Reject ( const char * szReason )
{
	m_szReason = szReason;
	return false;
}


bool SortBoundScanPlanner_c::SetupQuery ( const CSphMultiQueryArgs & tArgs )
{
	const auto & tQuery = m_tSelect.m_tQuery;
	if ( tQuery.m_tIndexHints.HasFilterIteratorHints() )
		return Reject ( "an explicit filter iterator hint selects the ordinary scan plan" );

	if ( tQuery.m_eSort!=SPH_SORT_EXTENDED || tQuery.HasKnn() || m_tSelect.m_iCutoff>=0 || !CanUseSortBound ( tQuery, m_dSorters, tArgs, false )
		|| !tQuery.m_dFilterTree.IsEmpty() || tQuery.m_bHybridSearch
		|| m_tCtx.m_pWeightFilter || m_dSorters[0]->GetMatchCapacity()<=0 || !m_tBound.Setup ( tQuery, m_tSelect.m_tIndexSchema, *m_dSorters[0] ) )
		return Reject ( "requires a single eligible ORDER BY sorter on an integer attribute and automatic cutoff" );

	const auto * pHint = tQuery.m_tIndexHints.GetSortBoundHint();
	m_bForce = pHint && pHint->m_bForce;
	if ( pHint )
	{
		bool bPrimaryAttr = pHint->m_sIndex==m_tBound.GetAttr();
		if ( m_bForce!=bPrimaryAttr )
			return Reject ( "the hint must name the primary ORDER BY attribute" );
	}

	return true;
}


bool SortBoundScanPlanner_c::SetupRangeAndSortAttr()
{
	auto & tRange = m_tPlan.m_tRange;
	tRange = { 0, RowID_t(m_tSelect.m_iTotalDocs)-1 };
	m_pRowIdFilter = GetRowIdFilter ( *m_tFlx.m_pFilters, RowID_t(m_tSelect.m_iTotalDocs), tRange );
	if ( tRange.m_tMinRowID>tRange.m_tMaxRowID || tRange.m_tMaxRowID>=m_tSelect.m_iTotalDocs )
		return Reject ( BOUNDS_UNAVAILABLE );

	m_pSortAttr = m_tSelect.m_tIndexSchema.GetAttr ( m_tBound.GetAttr().cstr() );
	if ( !m_pSortAttr || ( m_pSortAttr->IsColumnar() ? !m_tSelect.m_pColumnar : m_pSortAttr->m_tLocator.m_bDynamic || !m_pMinMax ) )
		return Reject ( BOUNDS_UNAVAILABLE );

	return true;
}


bool SortBoundScanPlanner_c::SetupFilter()
{
	constexpr const char * szUnsupported = "requires at most one supported attribute predicate after filter simplification";
	// The sort-bound scan evaluates one ordinary predicate in addition to the worker's row range.
	for ( const auto & tFilter : *m_tFlx.m_pFilters )
	{
		if ( &tFilter==m_pRowIdFilter )
			continue;

		const auto * pCol = m_tSelect.m_tIndexSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
		if ( m_pFilter || !pCol || pCol->m_pExpr || tFilter.m_bOptional || tFilter.m_bExclude
			|| ( tFilter.m_eType!=SPH_FILTER_STRING && tFilter.m_eType!=SPH_FILTER_VALUES && tFilter.m_eType!=SPH_FILTER_RANGE ) )
			return Reject ( szUnsupported );

		if ( m_tSelect.m_tQuery.m_dItems.any_of ( [&]( const auto & tItem ) { return tItem.m_sAlias==tFilter.m_sAttrName && tItem.m_sExpr!=tItem.m_sAlias; } ) )
			return Reject ( szUnsupported );

		m_pFilter = &tFilter;
	}

	if ( m_pFilter && !m_tCtx.m_pFilter )
		return Reject ( szUnsupported );

	return true;
}


bool SortBoundScanPlanner_c::SetupBlocks ( const CSphColumnInfo & tAttr )
{
	if ( tAttr.IsColumnar() )
	{
		auto * pBounds = m_tSelect.m_pColumnar->GetBlockBounds ( tAttr.m_sName.cstr() );
		if ( !pBounds )
			return false;

		m_tPlan.m_pColumnarBounds = pBounds;
		m_tPlan.m_iLeafLevel = pBounds->GetNumMinMaxLevels()-1;
		m_tPlan.m_iBlockRows = pBounds->GetLeafRows();
		return m_tPlan.m_iBlockRows>0 && m_tPlan.m_tRange.m_tMaxRowID/m_tPlan.m_iBlockRows<RowID_t(pBounds->GetNumMinMaxBlocks(m_tPlan.m_iLeafLevel));
	}

	m_tPlan.m_iBlockRows = DOCINFO_INDEX_FREQ;
	m_tPlan.m_pMinMax = m_pMinMax;
	m_tPlan.m_iStride = m_tSelect.m_tIndexSchema.GetRowSize();
	m_tPlan.m_tLocator = tAttr.m_tLocator;
	return true;
}


bool SortBoundScanPlanner_c::EstimateCost ( SortBoundScanCost_t & tCost, const CSphIndex & tIndex, bool bSelectIterators, CSphVector<SecondaryIndexInfo_t> & dOrdinaryPlan, CSphString & sWarning ) const
{
	int64_t iRows = m_tSelect.m_iTotalDocs;
	int64_t iChunkMatches = GetSortBoundMatchCount ( tIndex, m_pFilter, m_tSelect.m_tQuery.m_eCollation );
	int64_t iEstimate = iChunkMatches;
	if ( iEstimate<0 )
		iEstimate = m_pFilter ? EstimateFilterSelectivity ( *m_pFilter, m_tFlx ) : iRows;

	const auto & tRange = m_tPlan.m_tRange;
	int64_t iRangeRows = int64_t(tRange.m_tMaxRowID) - tRange.m_tMinRowID + 1;
	tCost.m_iRows = iRangeRows;
	tCost.m_iMatches = int64_t ( double ( std::clamp ( iEstimate, int64_t(0), iRows ) ) * iRangeRows / iRows );
	if ( tCost.m_iMatches<=tCost.m_iCapacity )
		return false;

	if ( !m_pFilter )
		return true;

	float fBestCost = FLT_MAX;
	if ( bSelectIterators )
	{
		StrVec_t dWarnings;
		dOrdinaryPlan = SelectIterators ( m_tSelect, fBestCost, dWarnings );
		if ( !dWarnings.IsEmpty() )
			sWarning = ConcatWarnings(dWarnings);
	}

	tCost.EstimateOrdinary ( m_tSelect, int(m_pFilter-m_tSelect.m_dFilters.Begin()), dOrdinaryPlan, fBestCost, iChunkMatches );
	return true;
}


bool SortBoundScanPlanner_c::SelectBlockScan ( const CSphIndex & tIndex, bool bSelectIterators, CSphVector<SecondaryIndexInfo_t> & dOrdinaryPlan, CSphString & sWarning )
{
	SortBoundScanCost_t tCost { 0, 0, m_dSorters[0]->GetMatchCapacity() };
	if ( !m_bForce && !EstimateCost ( tCost, tIndex, bSelectIterators, dOrdinaryPlan, sWarning ) )
		return Reject ( BOUNDS_UNAVAILABLE );

	if ( !SetupBlocks(*m_pSortAttr) )
		return Reject ( BOUNDS_UNAVAILABLE );

	if ( !m_bForce && !ShouldUseSortBoundScan ( tCost, m_tPlan, m_tBound ) )
		return Reject ( BOUNDS_UNAVAILABLE );

	return true;
}


std::optional<SortBoundScanPlan_t> SortBoundScanPlanner_c::Plan ( const CSphMultiQueryArgs & tArgs, const CSphIndex & tIndex, bool bSelectIterators, CSphVector<SecondaryIndexInfo_t> & dOrdinaryPlan, CSphString & sWarning )
{
	if ( !SetupQuery(tArgs) )
		return std::nullopt;

	if ( !SetupRangeAndSortAttr() )
		return std::nullopt;

	if ( !SetupFilter() )
		return std::nullopt;

	if ( !SelectBlockScan ( tIndex, bSelectIterators, dOrdinaryPlan, sWarning ) )
		return std::nullopt;

	m_szReason = nullptr;
	return std::move(m_tPlan);
}
} // namespace


std::optional<SortBoundScanPlan_t> PlanSortBoundScan ( const SelectIteratorCtx_t & tSelect, const CreateFilterContext_t & tFlx, const CSphQueryContext & tCtx, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs, const CSphIndex & tIndex, const DWORD * pMinMax, bool bSelectIterators, SortBound_c & tBound, CSphVector<SecondaryIndexInfo_t> & dOrdinaryPlan, CSphString & sWarning, const char * & szReason )
{
	SortBoundScanPlanner_c tPlanner ( tSelect, tFlx, tCtx, dSorters, pMinMax, tBound, szReason );
	return tPlanner.Plan ( tArgs, tIndex, bSelectIterators, dOrdinaryPlan, sWarning );
}


CostEstimate_c::CostEstimate_c ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx, int iCutoff )
	: m_dSIInfo ( dSIInfo )
	, m_tCtx ( tCtx )
	, m_iCutoff ( iCutoff )
{}


bool CostEstimate_c::NeedBitmapUnion ( int iNumIterators ) const
{
	// this needs to be in sync with iterator construction code
	const int BITMAP_ITERATOR_THRESH = 8;
	return iNumIterators>BITMAP_ITERATOR_THRESH;
}


int64_t CostEstimate_c::ApplyCutoff ( int64_t iDocs ) const
{
	if ( m_iCutoff<0 )
		return iDocs;

	return Min ( iDocs, m_iCutoff );
}


static bool IsGeodistFilter ( const CSphFilterSettings & tFilter, const SelectIteratorCtx_t & tCtx )
{
	int iAttr = GetAliasedAttrIndex ( tFilter.m_sAttrName, tCtx.m_tQuery, tCtx.m_tSorterSchema );
	if ( iAttr<0 )
		return false;

	const CSphColumnInfo & tAttr = tCtx.m_tSorterSchema.GetAttr(iAttr);
	if ( !tAttr.m_pExpr )
		return false;

	std::pair<GeoDistSettings_t *, bool> tSettingsPair { nullptr, false };
	tAttr.m_pExpr->Command ( SPH_EXPR_GET_GEODIST_SETTINGS, &tSettingsPair );
	return tSettingsPair.second;
}


static bool IsPoly2dFilter ( const CSphFilterSettings & tFilter, const SelectIteratorCtx_t & tCtx, int & iNumPoints )
{
	int iAttr = GetAliasedAttrIndex ( tFilter.m_sAttrName, tCtx.m_tQuery, tCtx.m_tSorterSchema );
	if ( iAttr<0 )
		return false;

	const CSphColumnInfo & tAttr = tCtx.m_tSorterSchema.GetAttr(iAttr);
	if ( !tAttr.m_pExpr )
		return false;

	std::pair<Poly2dBBox_t *, bool> tSettingsPair { nullptr, false };
	tAttr.m_pExpr->Command ( SPH_EXPR_GET_POLY2D_BBOX, &tSettingsPair );
	if ( tSettingsPair.second )
	{
		iNumPoints = tSettingsPair.first->m_iNumPoints;
		return true;
	}

	return false;
}


float CalcGetFilterComplexity ( const SecondaryIndexInfo_t & tSIInfo, const CSphFilterSettings & tFilter, const SelectIteratorCtx_t & tCtx )
{
	if ( IsGeodistFilter ( tFilter, tCtx ) )
		return 3.0f;

	int iNumPoints = 0;
	if ( IsPoly2dFilter ( tFilter, tCtx, iNumPoints ) )
	{
		const float COMPLEXITY_PER_POINT = 0.007f;
		return 1.2f + COMPLEXITY_PER_POINT*iNumPoints;
	}

	auto pAttr = tCtx.m_tIndexSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
	if ( !pAttr )
		return 1.0f;

	ESphAttr eAttrType = pAttr->m_eAttrType;
	float fFilterComplexity = 1.0f;
	if ( ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET ) && tFilter.m_eType==SPH_FILTER_VALUES )
	{
		float fCoeff = Max ( float( tSIInfo.m_iTotalValues )/tCtx.m_iTotalDocs, 2.0f );
		fFilterComplexity *= log2f(fCoeff)*tFilter.m_dValues.GetLength();
	}
	else if ( tFilter.m_eType==SPH_FILTER_STRING || tFilter.m_eType==SPH_FILTER_STRING_LIST )
	{
		const float STRING_COMPLEXITY = 10.0f;
		float fCoeff = Max ( float( tSIInfo.m_iTotalValues )/tCtx.m_iTotalDocs, 2.0f );
		fFilterComplexity *= STRING_COMPLEXITY*log2f(fCoeff)*tFilter.m_dStrings.GetLength();
	}

	return fFilterComplexity;
}


float CostEstimate_c::CalcFilterCost ( const SecondaryIndexInfo_t & tIndex, const CSphFilterSettings & tFilter, bool bFromIterator, bool bFilterOverExpr, float fDocsLeft ) const
{
	assert ( tIndex.m_eType==SecondaryIndexType_e::FILTER );

	float fFilterComplexity = CalcGetFilterComplexity ( tIndex, tFilter, m_tCtx );

	if ( bFromIterator || bFilterOverExpr )
	{
		int64_t iDocsToProcess = int64_t(fDocsLeft*m_tCtx.m_iTotalDocs);
		iDocsToProcess = ApplyCutoff ( iDocsToProcess );
		return Cost_Filter ( iDocsToProcess, fFilterComplexity );
	}

	if ( tFilter.m_eType==SPH_FILTER_STRING || tFilter.m_eType==SPH_FILTER_STRING_LIST )
		return Cost_Filter ( m_tCtx.m_iTotalDocs, fFilterComplexity );

	int64_t iDocsToFilter = int64_t ( (float)ApplyCutoff ( tIndex.m_iRsetEstimate ) * m_tCtx.m_iTotalDocs / ( tIndex.m_iRsetEstimate + 1 ) );
	float fCost = Cost_Filter ( iDocsToFilter, fFilterComplexity );
	fCost += Cost_BlockFilter ( m_tCtx.m_iTotalDocs, fFilterComplexity );

	return fCost;
}


float CostEstimate_c::CalcLookupCost ( const SecondaryIndexInfo_t & tIndex ) const
{
	// no cutoff here since lookup reader fetches all docs and sorts them
	return Cost_LookupRead ( tIndex.m_iRsetEstimate );
}


float CostEstimate_c::CalcAnalyzerCost ( const SecondaryIndexInfo_t & tIndex, const CSphFilterSettings & tFilter, float fDocsLeft ) const
{
	assert ( tIndex.m_eType==SecondaryIndexType_e::ANALYZER );
	assert ( m_tCtx.m_pColumnar );

	columnar::AttrInfo_t tAttrInfo;
	m_tCtx.m_pColumnar->GetAttrInfo ( tFilter.m_sAttrName.cstr(), tAttrInfo );

	float fFilterComplexity = CalcGetFilterComplexity ( tIndex, tFilter, m_tCtx );
	int64_t iDocsBeforeFilter = tIndex.m_iPartialColumnarMinMax==-1 ? m_tCtx.m_iTotalDocs : std::min ( tIndex.m_iPartialColumnarMinMax, m_tCtx.m_iTotalDocs );

	// filters that process but reject values are 2x faster
	float fAcceptCoeff = std::min ( float(tIndex.m_iRsetEstimate)/iDocsBeforeFilter, 1.0f ) / 2.0f + 0.5f;
	float fTotalCoeff = fFilterComplexity*tAttrInfo.m_fComplexity*fAcceptCoeff;

	int64_t iDocsToFilter = int64_t ( (float)ApplyCutoff ( tIndex.m_iRsetEstimate ) * m_tCtx.m_iTotalDocs / ( tIndex.m_iRsetEstimate + 1 ) );

	const int64_t READ_BLOCK_SIZE = 1024;
	iDocsBeforeFilter = Max ( int64_t(iDocsBeforeFilter*fDocsLeft), READ_BLOCK_SIZE );
	iDocsToFilter = Max ( int64_t(iDocsToFilter*fDocsLeft), READ_BLOCK_SIZE );

	if ( tIndex.m_iPartialColumnarMinMax==-1 ) // no minmax? scan whole index
		return Cost_ColumnarFilter ( iDocsToFilter, fTotalCoeff );

	// minmax tree eval
	const int MINMAX_NODE_SIZE = 1024;
	int iMatchingNodes = ( tIndex.m_iRsetEstimate + MINMAX_NODE_SIZE - 1 ) / MINMAX_NODE_SIZE;
	int iTreeLevels = sphLog2 ( m_tCtx.m_iTotalDocs );
	float fCost = Cost_Filter ( iMatchingNodes*iTreeLevels, fFilterComplexity );

	const float MINMAX_RATIO = 0.9f;
	if ( (float)tIndex.m_iPartialColumnarMinMax / m_tCtx.m_iTotalDocs >= MINMAX_RATIO )
		fCost += Cost_ColumnarFilter ( iDocsToFilter, fTotalCoeff );
	else
		fCost += Cost_ColumnarFilter ( std::min ( iDocsBeforeFilter, iDocsToFilter ), fTotalCoeff );

	return fCost;
}


float CostEstimate_c::CalcIndexCost ( const SecondaryIndexInfo_t & tIndex, const CSphFilterSettings & tFilter, float fDocsLeft ) const
{
	assert ( tIndex.m_eType==SecondaryIndexType_e::INDEX );

	float fCost = 0.0f;
	int64_t iDocs = ApplyCutoff ( tIndex.m_iRsetEstimate );
	uint32_t uNumIterators = tIndex.m_uNumSIIterators;
	if ( !uNumIterators )
		return 0.0f;

	const int64_t READ_BLOCK_SIZE = 1024;
	int64_t iDocsToRead = Max ( int64_t(iDocs*fDocsLeft), READ_BLOCK_SIZE );
	if ( uNumIterators==1 )
		fCost += Cost_IndexReadSingle(iDocsToRead);
	else
	{
		if ( NeedBitmapUnion(uNumIterators) )
			fCost += Cost_IndexReadBitmap(iDocs); // read all docs (when constructing the bitmap), not only the ones left
		else
			fCost += Cost_IndexUnionQueue(iDocsToRead);
	}

	fCost += Cost_IndexIteratorInit(uNumIterators);
	return fCost;
}


void CostEstimate_c::SortIndexes()
{
	m_dSorted.Resize ( m_dSIInfo.GetLength() );
	ARRAY_FOREACH ( i, m_dSorted )
		m_dSorted[i] = i;

	m_dSorted.Sort ( Lesser ( [this] ( int iA, int iB )
		{
			if ( m_dSIInfo[iA].m_eType==SecondaryIndexType_e::FILTER && m_dSIInfo[iB].m_eType!=SecondaryIndexType_e::FILTER )
				return false;

			if ( m_dSIInfo[iA].m_eType!=SecondaryIndexType_e::FILTER && m_dSIInfo[iB].m_eType==SecondaryIndexType_e::FILTER )
				return true;

			return m_dSIInfo[iA].m_iRsetEstimate < m_dSIInfo[iB].m_iRsetEstimate;
		} ) );
}


bool CostEstimate_c::IsFilterOverExpr ( int iIndex ) const
{
	auto & tIndex = GetIndex(iIndex);
	if ( tIndex.m_eType!=SecondaryIndexType_e::FILTER )
		return false;

	auto & tFilter = GetFilter(iIndex);
	int iAttr = GetAliasedAttrIndex ( tFilter.m_sAttrName, m_tCtx.m_tQuery, m_tCtx.m_tSorterSchema );
	if ( iAttr<0 )
		return true;

	return !!m_tCtx.m_tSorterSchema.GetAttr(iAttr).m_pExpr;
}


float CostEstimate_c::CalcIteratorIntersectCost ( float fFirstIteratorDocs, int iNumIterators )
{
	int64_t iDocs = fFirstIteratorDocs*m_tCtx.m_iTotalDocs;
	return COST_ITERATOR_INTERSECT*iDocs*(iNumIterators-1)*SCALE;
}


float CostEstimate_c::CalcPushCost ( float fDocsAfterFilters ) const
{
	int64_t iDocsToPush = fDocsAfterFilters*m_tCtx.m_iTotalDocs;
	iDocsToPush = ApplyCutoff(iDocsToPush);
	if ( HasImplicitGrouping ( m_tCtx.m_tQuery ) )
		return Cost_PushImplicitGroupby ( iDocsToPush );

	return Cost_Push ( iDocsToPush );
}


float CostEstimate_c::CalcQueryCost()
{
	SortIndexes();

	int iNumLookups = 0;
	int iNumAnalyzers = 0;
	int iNumIndexes = 0;

	float fFirstIteratorDocs = 0.0f;
	bool bFirstDocsAssigned = false;

	float fCost = 0.0f;
	float fDocsLeft = m_tCtx.m_fDocsLeft;
	for ( int i = 0; i < GetNumIndexes(); i++ )
	{
		const auto & tIndex = GetIndex(i);
		const auto & tFilter = GetFilter(i);
		float fIndexProbability = float(tIndex.m_iRsetEstimate) / m_tCtx.m_iTotalDocs;

		switch ( tIndex.m_eType )
		{
		case SecondaryIndexType_e::LOOKUP:
			fCost += CalcLookupCost(tIndex);
			iNumLookups++;
			break;

		case SecondaryIndexType_e::ANALYZER:
			fCost += CalcAnalyzerCost ( tIndex, tFilter, fDocsLeft );
			iNumAnalyzers++;
			break;

		case SecondaryIndexType_e::INDEX:
			fCost += CalcIndexCost ( tIndex, tFilter, fDocsLeft );
			iNumIndexes++;
			break;

		case SecondaryIndexType_e::FILTER:
			fCost += CalcFilterCost ( tIndex, tFilter, m_tCtx.m_bFromIterator || ( iNumLookups + iNumAnalyzers + iNumIndexes ) >0, IsFilterOverExpr(i), fDocsLeft );
			break;

		case SecondaryIndexType_e::NONE:
			continue;

		default:
			break;
		}

		fDocsLeft *= fIndexProbability;

		if ( iNumLookups + iNumAnalyzers + iNumIndexes > 0 && !bFirstDocsAssigned )
		{
			fFirstIteratorDocs = fDocsLeft;
			bFirstDocsAssigned = true;
		}
	}

	int iToIntersect = iNumLookups + iNumAnalyzers + iNumIndexes;
	if ( iToIntersect > 1 )
		fCost += CalcIteratorIntersectCost ( fFirstIteratorDocs, iToIntersect );

	if ( m_tCtx.m_bCalcPushCost )
		fCost += CalcPushCost(fDocsLeft);

	if ( !iNumLookups ) // docid lookups always run in a single thread
		fCost = iNumIndexes ? CalcMTCostSI(fCost) : ( iNumAnalyzers ? CalcMTCostCS(fCost) : CalcMTCost(fCost) );

	return fCost;
}

/////////////////////////////////////////////////////////////////////

SelectIteratorCtx_t::SelectIteratorCtx_t ( const CSphQuery & tQuery, const CSphVector<CSphFilterSettings> & dFilters, const ISphSchema & tIndexSchema, const ISphSchema & tSorterSchema, const HistogramContainer_c * pHistograms, columnar::Columnar_i * pColumnar, const SIContainer_c & tSI, int iCutoff, int64_t iTotalDocs, int iThreads )
	: m_tQuery ( tQuery )
	, m_dFilters ( dFilters )
	, m_tIndexSchema ( tIndexSchema )
	, m_tSorterSchema ( tSorterSchema )
	, m_pHistograms ( pHistograms )
	, m_pColumnar ( pColumnar )
	, m_tSI ( tSI )
	, m_iCutoff ( iCutoff )
	, m_iTotalDocs ( iTotalDocs )
	, m_iThreads ( iThreads )
{}


static bool JsonSIFilterFits ( const SIContainer_c & tSI, const CSphFilterSettings & tFilter )
{
	if ( tFilter.m_eType==SPH_FILTER_NULL )
		return true;

	common::AttrType_e eType;
	if ( !tSI.GetAttrType ( tFilter.m_sAttrName, eType ) )
		return true;

	switch ( eType )
	{
	case common::AttrType_e::UINT32:
	case common::AttrType_e::INT64:		return tFilter.m_eType==SPH_FILTER_VALUES || tFilter.m_eType==SPH_FILTER_RANGE;
	case common::AttrType_e::FLOAT:		return tFilter.m_eType==SPH_FILTER_FLOATRANGE;	// the index turns an int range into a float32 one and leaves several int values as they are
	case common::AttrType_e::STRING:	return tFilter.m_eType==SPH_FILTER_STRING || tFilter.m_eType==SPH_FILTER_STRING_LIST;
	default:							return false;
	}
}


bool SelectIteratorCtx_t::IsEnabled_SI ( const CSphFilterSettings & tFilter ) const
{
	if ( m_tSI.IsEmpty() )
		return false;

	if ( tFilter.m_eType!=SPH_FILTER_VALUES && tFilter.m_eType!=SPH_FILTER_STRING && tFilter.m_eType!=SPH_FILTER_STRING_LIST && tFilter.m_eType!=SPH_FILTER_RANGE && tFilter.m_eType!=SPH_FILTER_FLOATRANGE && tFilter.m_eType!=SPH_FILTER_NULL )
		return false;

	// string hashes do not preserve ordering
	if ( tFilter.m_eType==SPH_FILTER_STRING && tFilter.m_eStrCmpDir!=EStrCmpDir::EQ )
		return false;

	// all(mva\string) need to scan whole row
	if ( tFilter.m_eMvaFunc==SPH_MVAFUNC_ALL )
		return false;

	const CSphColumnInfo * pCol = m_tIndexSchema.GetAttr ( tFilter.m_sAttrName.cstr() );

	// FIXME!!! warn in case force index used but index was skipped
	if ( pCol && ( pCol->m_eAttrType==SPH_ATTR_STRING && m_tQuery.m_eCollation!=SPH_COLLATION_DEFAULT ) )
		return false;

	// all\any(string list) need to scan whole row
	if ( pCol && pCol->m_eAttrType==SPH_ATTR_STRING && tFilter.m_eType==SPH_FILTER_STRING_LIST && tFilter.m_eMvaFunc!=SPH_MVAFUNC_NONE )
		return false;

	// json keys get their secondary index column type from the chunk data (the widest type of the key),
	// so the filter type is not adapted to it: use the index only when the column answers this filter type
	if ( ( !pCol || pCol->m_eAttrType==SPH_ATTR_JSON ) && !JsonSIFilterFits ( m_tSI, tFilter ) )
		return false;

	return m_tSI.IsEnabled ( tFilter.m_sAttrName );
}


bool SelectIteratorCtx_t::IsEnabled_Analyzer ( const CSphFilterSettings & tFilter ) const
{
	// columnar string analyzers only support equality comparisons
	if ( tFilter.m_eType==SPH_FILTER_STRING && tFilter.m_eStrCmpDir!=EStrCmpDir::EQ )
		return false;

	// they also compare whole strings, while ANY() / ALL() over a string list match words
	if ( tFilter.m_eType==SPH_FILTER_STRING_LIST && tFilter.m_eMvaFunc!=SPH_MVAFUNC_NONE )
		return false;

	auto pAttr = m_tIndexSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
	return pAttr && ( pAttr->IsColumnar() || pAttr->IsColumnarExpr() );
}

/////////////////////////////////////////////////////////////////////

CostEstimate_i * CreateCostEstimate ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx, int iCutoff )
{
	return new CostEstimate_c ( dSIInfo, tCtx, iCutoff );
}


float CalcFTIntersectCost ( const NodeEstimate_t & tEst1, const NodeEstimate_t & tEst2, int64_t iTotalDocs, int iDocsPerBlock1, int iDocsPerBlock2 )
{
	if ( !tEst1.m_iDocs || !tEst2.m_iDocs )
		return 0.0f;

	int64_t iCorrectedDocs1 = tEst1.m_iDocs;
	int64_t iCorrectedDocs2 = tEst2.m_iDocs;
	float fCorrectedCost1 = tEst1.m_fCost;
	float fCorrectedCost2 = tEst2.m_fCost;

	float fIntersection = float(tEst1.m_iDocs)/iTotalDocs*float(tEst2.m_iDocs)/iTotalDocs;
	int64_t iHintCalls = tEst1.m_iDocs * tEst1.m_iTerms / iDocsPerBlock1 + tEst2.m_iDocs * tEst2.m_iTerms / iDocsPerBlock2;

	const float THRESH = 0.05f;
	if ( fIntersection > THRESH )
	{
		iCorrectedDocs1 = int64_t(tEst1.m_iDocs*fIntersection);
		iCorrectedDocs2 = int64_t(tEst2.m_iDocs*fIntersection);

		fCorrectedCost1 *= fIntersection;
		fCorrectedCost2 *= fIntersection;

	} else
	{
		// intersection of left and right result sets is small
		// best case scenario: rowid ranges do not overlap; one hint call is enough to stop the search
		// worst case scenario: rowid ranges fully overlap; hint calls do nothing
		// let's evaluate mid scenario: hint calls have some effect, but we still have to evaluate half of all docs
		// since we are comparing this estimate with full FT match cost, it will ok
		iCorrectedDocs1 /= 2;
		fCorrectedCost1 /= 2.0f;
		iCorrectedDocs2 /= 2;
		fCorrectedCost2 /= 2.0f;
	}

	const float COST_INTERSECT = 20.0f;
	const float COST_HINTCALL = 35.0f;

	return fCorrectedCost1 + fCorrectedCost2 + ( COST_INTERSECT*(iCorrectedDocs1+iCorrectedDocs2) + COST_HINTCALL*iHintCalls )*CostEstimate_c::SCALE;
}
