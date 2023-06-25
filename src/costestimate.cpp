//
//
// Copyright (c) 2018-2023, Manticore Software LTD (https://manticoresearch.com)
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
#include "secondarylib.h"
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
			CostEstimate_c ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx );

	float	CalcQueryCost() final;

private:
	static constexpr float SCALE = 1.0f/1000000.0f;

	static constexpr float COST_PUSH					= 6.0f;
	static constexpr float COST_PUSH_IG					= 3.0f;
	static constexpr float COST_FILTER					= 8.5f;
	static constexpr float COST_COLUMNAR_FILTER			= 4.0f;
	static constexpr float COST_INTERSECT				= 5.0f;
	static constexpr float COST_INDEX_READ_SINGLE		= 4.0f;
	static constexpr float COST_INDEX_READ_BITMAP		= 4.5f;
	static constexpr float COST_INDEX_UNION_COEFF		= 4.0f;
	static constexpr float COST_LOOKUP_READ				= 20.0f;
	static constexpr float COST_INDEX_ITERATOR_INIT		= 30.0f;

	const CSphVector<SecondaryIndexInfo_t> &	m_dSIInfo;
	const SelectIteratorCtx_t &					m_tCtx;

	static float	Cost_Filter ( int64_t iDocs, float fComplexity )		{ return COST_FILTER*fComplexity*iDocs*SCALE; }
	static float	Cost_BlockFilter ( int64_t iDocs, float fComplexity )	{ return Cost_Filter ( iDocs/DOCINFO_INDEX_FREQ, fComplexity ); }
	static float	Cost_ColumnarFilter ( int64_t iDocs, float fComplexity ){ return COST_COLUMNAR_FILTER*fComplexity*iDocs*SCALE; }
	static float	Cost_Push ( int64_t iDocs )								{ return COST_PUSH*iDocs*SCALE; }
	static float	Cost_PushImplicitGroupby ( int64_t iDocs )				{ return COST_PUSH_IG*iDocs*SCALE; }
	static float	Cost_Intersect ( int64_t iDocs )						{ return COST_INTERSECT*iDocs*SCALE; }
	static float	Cost_IndexReadSingle ( int64_t iDocs )					{ return COST_INDEX_READ_SINGLE*iDocs*SCALE; }
	static float	Cost_IndexReadBitmap ( int64_t iDocs )					{ return COST_INDEX_READ_BITMAP*iDocs*SCALE; }
	static float	Cost_IndexUnionQueue ( int64_t iDocs )					{ return COST_INDEX_UNION_COEFF*iDocs*log2f(iDocs)*SCALE; }
	static float	Cost_LookupRead ( int64_t iDocs )						{ return COST_LOOKUP_READ*iDocs*SCALE; }
	static float	Cost_IndexIteratorInit ( int64_t iNumIterators )		{ return COST_INDEX_ITERATOR_INIT*iNumIterators*SCALE; }

	float	CalcIndexCost() const;
	float	CalcFilterCost ( bool bFromIterator, float fDocsAfterIndexes ) const;
	float	CalcAnalyzerCost() const;
	float	CalcLookupCost() const;
	float	CalcPushCost ( float fDocsAfterFilters ) const;
	float	CalcMTCost ( float fCost ) const;
	float	CalcMTCostSI ( float fCost ) const;

	float	CalcGetFilterComplexity ( const SecondaryIndexInfo_t & tSIInfo, const CSphFilterSettings & tFilter ) const;
	bool	NeedBitmapUnion ( const CSphFilterSettings & tFilter, int64_t iRsetSize ) const;
	uint32_t CalcNumSIIterators ( const CSphFilterSettings & tFilter, int64_t iDocs ) const;
	int64_t	ApplyCutoff ( int64_t iDocs ) const;
};


CostEstimate_c::CostEstimate_c ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
	: m_dSIInfo ( dSIInfo )
	, m_tCtx ( tCtx )
{}


bool CostEstimate_c::NeedBitmapUnion ( const CSphFilterSettings & tFilter, int64_t iRsetSize ) const
{
	// this needs to be in sync with iterator construction code
	const int BITMAP_ITERATOR_THRESH = 8;
	if ( tFilter.m_eType==SPH_FILTER_RANGE )
	{
		if ( tFilter.m_bOpenRight || tFilter.m_bOpenLeft )
			return true;
		else
			return ( tFilter.m_iMaxValue-tFilter.m_iMinValue+1 ) >= BITMAP_ITERATOR_THRESH;
	}

	return tFilter.m_eType==SPH_FILTER_FLOATRANGE;
}


int64_t CostEstimate_c::ApplyCutoff ( int64_t iDocs ) const
{
	if ( m_tCtx.m_iCutoff<0 )
		return iDocs;

	return Min ( iDocs, m_tCtx.m_iCutoff );
}


float CostEstimate_c::CalcIndexCost() const
{
	int iNumIndexes = 0;
	int64_t iSumDocsFromIndexes = 0;
	float fCost = 0.0f;
	ARRAY_FOREACH ( i, m_dSIInfo )
	{
		const auto & tSIInfo = m_dSIInfo[i];
		int64_t iDocs = ApplyCutoff ( tSIInfo.m_iRsetEstimate );

		if ( tSIInfo.m_eType==SecondaryIndexType_e::LOOKUP ||
			tSIInfo.m_eType==SecondaryIndexType_e::ANALYZER ||
			tSIInfo.m_eType==SecondaryIndexType_e::INDEX )
			iSumDocsFromIndexes += iDocs;

		if ( tSIInfo.m_eType!=SecondaryIndexType_e::INDEX )
			continue;

		iNumIndexes++;

		uint32_t uNumIterators = tSIInfo.m_uNumSIIterators;
		if ( uNumIterators )
		{
			const auto & tFilter = m_tCtx.m_tQuery.m_dFilters[i];

			if ( uNumIterators>1 && !NeedBitmapUnion ( tFilter, iDocs ) )
				fCost += Cost_IndexUnionQueue(iDocs);

			if ( uNumIterators==1 )
				fCost += Cost_IndexReadSingle(iDocs);
			else
				fCost += Cost_IndexReadBitmap(iDocs);

			fCost += Cost_IndexIteratorInit(uNumIterators);
		}
	}
	
	if ( iNumIndexes>1 )
		fCost += Cost_Intersect ( iSumDocsFromIndexes );

	return fCost;
}


float CostEstimate_c::CalcGetFilterComplexity ( const SecondaryIndexInfo_t & tSIInfo, const CSphFilterSettings & tFilter ) const
{
	auto pAttr = m_tCtx.m_tSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
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


float CostEstimate_c::CalcFilterCost ( bool bFromIterator, float fDocsAfterIndexes ) const
{
	float fCost = 0.0f;
	ARRAY_FOREACH ( i, m_dSIInfo )
	{
		const auto & tSIInfo = m_dSIInfo[i];
		const auto & tFilter = m_tCtx.m_tQuery.m_dFilters[i];

		if ( tSIInfo.m_eType!=SecondaryIndexType_e::FILTER )
			continue;

		float fFilterComplexity = CalcGetFilterComplexity ( tSIInfo, tFilter );

		if ( bFromIterator )
		{
			int64_t iDocsToProcess = int64_t(fDocsAfterIndexes*m_tCtx.m_iTotalDocs);
			iDocsToProcess = ApplyCutoff ( iDocsToProcess );
			fCost += Cost_Filter ( iDocsToProcess, fFilterComplexity );
		}
		else
		{
			if ( tFilter.m_eType==SPH_FILTER_STRING || tFilter.m_eType==SPH_FILTER_STRING_LIST )
				fCost += Cost_Filter ( m_tCtx.m_iTotalDocs, fFilterComplexity );
			else
			{
				int64_t iDocsToFilter = int64_t ( (float)ApplyCutoff ( tSIInfo.m_iRsetEstimate ) * m_tCtx.m_iTotalDocs / ( tSIInfo.m_iRsetEstimate + 1 ) );
				fCost += Cost_Filter ( iDocsToFilter, fFilterComplexity );
				fCost += Cost_BlockFilter ( m_tCtx.m_iTotalDocs, fFilterComplexity );
			}
		}
	}

	return fCost;
}


float CostEstimate_c::CalcAnalyzerCost() const
{
	float fCost = 0.0f;
	ARRAY_FOREACH ( i, m_dSIInfo )
	{
		const auto & tSIInfo = m_dSIInfo[i];
		const auto & tFilter = m_tCtx.m_tQuery.m_dFilters[i];

		if ( tSIInfo.m_eType!=SecondaryIndexType_e::ANALYZER )
			continue;

		assert ( m_tCtx.m_pColumnar );
		columnar::AttrInfo_t tAttrInfo;
		m_tCtx.m_pColumnar->GetAttrInfo ( tFilter.m_sAttrName.cstr(), tAttrInfo );

		float fFilterComplexity = CalcGetFilterComplexity ( tSIInfo, tFilter );
		int64_t iDocsBeforeFilter = tSIInfo.m_iPartialColumnarMinMax==-1 ? m_tCtx.m_iTotalDocs : std::min ( tSIInfo.m_iPartialColumnarMinMax, m_tCtx.m_iTotalDocs );

		// filters that process but reject values are 2x faster
		float fAcceptCoeff = std::min ( float(tSIInfo.m_iRsetEstimate)/iDocsBeforeFilter, 1.0f ) / 2.0f + 0.5f;
		float fTotalCoeff = fFilterComplexity*tAttrInfo.m_fComplexity*fAcceptCoeff;

		int64_t iDocsToFilter = int64_t ( (float)ApplyCutoff ( tSIInfo.m_iRsetEstimate ) * m_tCtx.m_iTotalDocs / ( tSIInfo.m_iRsetEstimate + 1 ) );

		if ( tSIInfo.m_iPartialColumnarMinMax==-1 ) // no minmax? scan whole index
			fCost += Cost_ColumnarFilter ( iDocsToFilter, fTotalCoeff );
		else
		{
			// minmax tree eval
			const int MINMAX_NODE_SIZE = 1024;
			int iMatchingNodes = ( tSIInfo.m_iRsetEstimate + MINMAX_NODE_SIZE - 1 ) / MINMAX_NODE_SIZE;
			int iTreeLevels = sphLog2 ( m_tCtx.m_iTotalDocs );
			fCost += Cost_Filter ( iMatchingNodes*iTreeLevels, fFilterComplexity );

			const float MINMAX_RATIO = 0.9f;
			if ( (float)tSIInfo.m_iPartialColumnarMinMax / m_tCtx.m_iTotalDocs >= MINMAX_RATIO )
				fCost += Cost_ColumnarFilter ( iDocsToFilter, fTotalCoeff );
			else
				fCost += Cost_ColumnarFilter ( std::min ( iDocsBeforeFilter, iDocsToFilter ), fTotalCoeff );
		}
	}

	return fCost;
}


float CostEstimate_c::CalcLookupCost() const
{
	int64_t iDocsToReadLookup = 0;
	for ( const auto & i : m_dSIInfo )
		if ( i.m_eType==SecondaryIndexType_e::LOOKUP )
			iDocsToReadLookup += i.m_iRsetEstimate;

	// no cutoff here since lookup reader fetches all docs and sorts them
	return Cost_LookupRead ( iDocsToReadLookup );
}


float CostEstimate_c::CalcPushCost ( float fDocsAfterFilters ) const
{
	int64_t iDocsToPush = fDocsAfterFilters*m_tCtx.m_iTotalDocs;
	iDocsToPush = ApplyCutoff(iDocsToPush);
	if ( HasImplicitGrouping ( m_tCtx.m_tQuery ) )
		return Cost_PushImplicitGroupby ( iDocsToPush );

	return Cost_Push ( iDocsToPush );
}


float CostEstimate_c::CalcMTCost ( float fCost ) const
{
	return EstimateMTCost ( fCost, m_tCtx.m_iThreads );
}


float CostEstimate_c::CalcMTCostSI ( float fCost ) const
{
	return EstimateMTCostSI ( fCost, m_tCtx.m_iThreads );
}


float CostEstimate_c::CalcQueryCost()
{
	float fCost = 0.0f;
	float fDocsAfterIndexes = 1.0f;
	float fDocsAfterFilters = 1.0f;
	int iToIntersect = 0;

	int iNumFilters = 0;
	int iNumAnalyzers = 0;
	int iNumIndexes = 0;
	int iNumLookups = 0;

	for ( const auto & i : m_dSIInfo )
	{
		if ( !i.m_dCapabilities.GetLength() )
			continue;

		int64_t iDocs = i.m_iRsetEstimate;
		float fIndexProbability = float(iDocs) / m_tCtx.m_iTotalDocs;

		if ( i.m_eType==SecondaryIndexType_e::LOOKUP ||
			 i.m_eType==SecondaryIndexType_e::ANALYZER ||
			 i.m_eType==SecondaryIndexType_e::INDEX )
		{
			fDocsAfterIndexes *= fIndexProbability;
			iToIntersect++;
		}

		fDocsAfterFilters *= fIndexProbability;

		switch ( i.m_eType )
		{
		case SecondaryIndexType_e::LOOKUP:
			iNumLookups++;
			break;

		case SecondaryIndexType_e::ANALYZER:
			iNumAnalyzers++;
			break;

		case SecondaryIndexType_e::INDEX:
			iNumIndexes++;
			break;

		case SecondaryIndexType_e::FILTER:
			iNumFilters++;
			break;

		default:
			break;
		}
	}

	if ( iNumFilters )
		fCost += CalcFilterCost ( m_tCtx.m_bFromIterator || iToIntersect>0, fDocsAfterIndexes );

	if ( iNumLookups )
		fCost += CalcLookupCost();

	if ( iNumAnalyzers )
		fCost += CalcAnalyzerCost();

	if ( iNumIndexes )
		fCost += CalcIndexCost();

	if ( m_tCtx.m_bCalcPushCost )
		fCost += CalcPushCost(fDocsAfterFilters);

	if ( !iNumLookups ) // docid lookups always run in a single thread
		fCost = iNumIndexes ? CalcMTCostSI(fCost) : CalcMTCost(fCost);

	return fCost;
}

/////////////////////////////////////////////////////////////////////

SelectIteratorCtx_t::SelectIteratorCtx_t ( const CSphQuery & tQuery, const ISphSchema & tSchema, const HistogramContainer_c * pHistograms, columnar::Columnar_i * pColumnar, SI::Index_i * pSI, int iCutoff, int64_t iTotalDocs, int iThreads )
	: m_tQuery ( tQuery )
	, m_tSchema ( tSchema )
	, m_pHistograms ( pHistograms )
	, m_pColumnar ( pColumnar )
	, m_pSI ( pSI )
	, m_iCutoff ( iCutoff )
	, m_iTotalDocs ( iTotalDocs )
	, m_iThreads ( iThreads )
{}


bool SelectIteratorCtx_t::IsEnabled_SI ( const CSphFilterSettings & tFilter ) const
{
	if ( !m_pSI )
		return false;

	if ( tFilter.m_eType!=SPH_FILTER_VALUES && tFilter.m_eType!=SPH_FILTER_STRING && tFilter.m_eType!=SPH_FILTER_STRING_LIST && tFilter.m_eType!=SPH_FILTER_RANGE && tFilter.m_eType!=SPH_FILTER_FLOATRANGE )
		return false;

	// all(mva\string) need to scan whole row
	if ( tFilter.m_eMvaFunc==SPH_MVAFUNC_ALL )
		return false;

	// need to handle only plain or columnar attr but not dynamic \ expressions
	const CSphColumnInfo * pCol = m_tSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
	if ( !pCol )
		return false;

	if ( pCol->m_pExpr.Ptr() && !pCol->IsColumnarExpr() )
		return false;

	// FIXME!!! warn in case force index used but index was skipped
	if ( pCol->m_eAttrType==SPH_ATTR_STRING && m_tQuery.m_eCollation!=SPH_COLLATION_DEFAULT )
		return false;

	return m_pSI->IsEnabled( tFilter.m_sAttrName.cstr() );
}


bool SelectIteratorCtx_t::IsEnabled_Analyzer ( const CSphFilterSettings & tFilter ) const
{
	auto pAttr = m_tSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
	return pAttr && ( pAttr->IsColumnar() || pAttr->IsColumnarExpr() );
}

/////////////////////////////////////////////////////////////////////

CostEstimate_i * CreateCostEstimate ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
{
	return new CostEstimate_c ( dSIInfo, tCtx );
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
