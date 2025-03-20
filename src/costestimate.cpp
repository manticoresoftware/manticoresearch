//
//
// Copyright (c) 2018-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "costestimate.h"

#include "sphinxint.h"
#include "sphinxsort.h"
#include "columnarfilter.h"
#include "secondaryindex.h"
#include "geodist.h"
#include <math.h>
#include "std/sys.h"


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

	bool	IsGeodistFilter ( const CSphFilterSettings & tFilter ) const;
	bool	IsPoly2dFilter ( const CSphFilterSettings & tFilter, int & iNumPoints ) const;
	float	CalcGetFilterComplexity ( const SecondaryIndexInfo_t & tSIInfo, const CSphFilterSettings & tFilter ) const;
	bool	NeedBitmapUnion ( int iNumIterators ) const;
	uint32_t CalcNumSIIterators ( const CSphFilterSettings & tFilter, int64_t iDocs ) const;
	int64_t	ApplyCutoff ( int64_t iDocs ) const;

	void	SortIndexes();
	int		GetNumIndexes() const								{ return m_dSIInfo.GetLength(); }
	const SecondaryIndexInfo_t & GetIndex ( int iIndex ) const	{ return m_dSIInfo[m_dSorted[iIndex]]; }
	const CSphFilterSettings & GetFilter ( int iIndex ) const	{ return m_tCtx.m_dFilters[m_dSorted[iIndex]]; }
	bool	IsFilterOverExpr ( int iIndex ) const;
};


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


bool CostEstimate_c::IsGeodistFilter ( const CSphFilterSettings & tFilter ) const
{
	int iAttr = GetAliasedAttrIndex ( tFilter.m_sAttrName, m_tCtx.m_tQuery, m_tCtx.m_tSorterSchema );
	if ( iAttr<0 )
		return false;

	const CSphColumnInfo & tAttr = m_tCtx.m_tSorterSchema.GetAttr(iAttr);
	if ( !tAttr.m_pExpr )
		return false;

	std::pair<GeoDistSettings_t *, bool> tSettingsPair { nullptr, false };
	tAttr.m_pExpr->Command ( SPH_EXPR_GET_GEODIST_SETTINGS, &tSettingsPair );
	return tSettingsPair.second;
}


bool CostEstimate_c::IsPoly2dFilter ( const CSphFilterSettings & tFilter, int & iNumPoints ) const
{
	int iAttr = GetAliasedAttrIndex ( tFilter.m_sAttrName, m_tCtx.m_tQuery, m_tCtx.m_tSorterSchema );
	if ( iAttr<0 )
		return false;

	const CSphColumnInfo & tAttr = m_tCtx.m_tSorterSchema.GetAttr(iAttr);
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


float CostEstimate_c::CalcGetFilterComplexity ( const SecondaryIndexInfo_t & tSIInfo, const CSphFilterSettings & tFilter ) const
{
	if ( IsGeodistFilter(tFilter) )
		return 3.0f;

	int iNumPoints = 0;
	if ( IsPoly2dFilter ( tFilter, iNumPoints ) )
	{
		const float COMPLEXITY_PER_POINT = 0.007f;
		return 1.2f + COMPLEXITY_PER_POINT*iNumPoints;
	}

	auto pAttr = m_tCtx.m_tIndexSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
	if ( !pAttr )
		return 1.0f;

	ESphAttr eAttrType = pAttr->m_eAttrType;
	float fFilterComplexity = 1.0f;
	if ( ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET ) && tFilter.m_eType==SPH_FILTER_VALUES )
	{
		float fCoeff = Max ( float( tSIInfo.m_iTotalValues )/m_tCtx.m_iTotalDocs, 2.0f );
		fFilterComplexity *= log2f(fCoeff)*tFilter.m_dValues.GetLength();
	}
	else if ( tFilter.m_eType==SPH_FILTER_STRING || tFilter.m_eType==SPH_FILTER_STRING_LIST )
	{
		const float STRING_COMPLEXITY = 10.0f;
		float fCoeff = Max ( float( tSIInfo.m_iTotalValues )/m_tCtx.m_iTotalDocs, 2.0f );
		fFilterComplexity *= STRING_COMPLEXITY*log2f(fCoeff)*tFilter.m_dStrings.GetLength();
	}

	return fFilterComplexity;
}


float CostEstimate_c::CalcFilterCost ( const SecondaryIndexInfo_t & tIndex, const CSphFilterSettings & tFilter, bool bFromIterator, bool bFilterOverExpr, float fDocsLeft ) const
{
	assert ( tIndex.m_eType==SecondaryIndexType_e::FILTER );

	float fFilterComplexity = CalcGetFilterComplexity ( tIndex, tFilter );

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

	float fFilterComplexity = CalcGetFilterComplexity ( tIndex, tFilter );
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


bool SelectIteratorCtx_t::IsEnabled_SI ( const CSphFilterSettings & tFilter ) const
{
	if ( m_tSI.IsEmpty() )
		return false;

	if ( tFilter.m_eType!=SPH_FILTER_VALUES && tFilter.m_eType!=SPH_FILTER_STRING && tFilter.m_eType!=SPH_FILTER_STRING_LIST && tFilter.m_eType!=SPH_FILTER_RANGE && tFilter.m_eType!=SPH_FILTER_FLOATRANGE && tFilter.m_eType!=SPH_FILTER_NULL )
		return false;

	// all(mva\string) need to scan whole row
	if ( tFilter.m_eMvaFunc==SPH_MVAFUNC_ALL )
		return false;

	const CSphColumnInfo * pCol = m_tIndexSchema.GetAttr ( tFilter.m_sAttrName.cstr() );

	// FIXME!!! warn in case force index used but index was skipped
	if ( pCol && ( pCol->m_eAttrType==SPH_ATTR_STRING && m_tQuery.m_eCollation!=SPH_COLLATION_DEFAULT ) )
		return false;

	return m_tSI.IsEnabled ( tFilter.m_sAttrName );
}


bool SelectIteratorCtx_t::IsEnabled_Analyzer ( const CSphFilterSettings & tFilter ) const
{
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
		float fIntersection = float(tEst1.m_iDocs)/iTotalDocs*float(tEst2.m_iDocs)/iTotalDocs;

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
