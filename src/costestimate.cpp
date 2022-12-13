//
//
// Copyright (c) 2018-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "costestimate.h"

#include "sphinxint.h"
#include "columnarfilter.h"
#include "secondarylib.h"
#include <math.h>

class CostEstimate_c : public CostEstimate_i
{
public:
			CostEstimate_c ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx );

	float	CalcQueryCost() final;

private:
	static constexpr float SCALE = 1.0f/1000000.0f;

	static constexpr float COST_PUSH					= 6.0f;
	static constexpr float COST_FILTER					= 8.5f;
	static constexpr float COST_COLUMNAR_FILTER			= 4.0f;
	static constexpr float COST_INTERSECT				= 5.0f;
	static constexpr float COST_INDEX_READ_SINGLE		= 1.5f;
	static constexpr float COST_INDEX_READ_DENSE_BITMAP	= 1.5f;
	static constexpr float COST_INDEX_READ_SPARSE		= 30.0f;
	static constexpr float COST_INDEX_UNION_COEFF		= 4.0f;
	static constexpr float COST_LOOKUP_READ				= 7.0f;
	static constexpr float COST_INDEX_ITERATOR_INIT		= 150.0f;

	const CSphVector<SecondaryIndexInfo_t> &	m_dSIInfo;
	const SelectIteratorCtx_t &					m_tCtx;

	float	Cost_Filter ( int64_t iDocs, float fComplexity ) const			{ return COST_FILTER*fComplexity*iDocs*SCALE; }
	float	Cost_BlockFilter ( int64_t iDocs, float fComplexity ) const		{ return Cost_Filter ( iDocs/DOCINFO_INDEX_FREQ, fComplexity ); }
	float	Cost_ColumnarFilter ( int64_t iDocs, float fComplexity ) const	{ return COST_COLUMNAR_FILTER*fComplexity*iDocs*SCALE; }
	float	Cost_Push ( int64_t iDocs ) const								{ return COST_PUSH*iDocs*SCALE; }
	float	Cost_Intersect ( int64_t iDocs ) const							{ return COST_INTERSECT*iDocs*SCALE; }
	float	Cost_IndexReadSingle ( int64_t iDocs ) const					{ return COST_INDEX_READ_SINGLE*iDocs*SCALE; }
	float	Cost_IndexReadDenseBitmap ( int64_t iDocs ) const				{ return COST_INDEX_READ_DENSE_BITMAP*iDocs*SCALE; }
	float	Cost_IndexReadSparse ( int64_t iDocs ) const					{ return COST_INDEX_READ_SPARSE*iDocs*SCALE; }
	float	Cost_IndexUnionQueue ( int64_t iDocs ) const					{ return COST_INDEX_UNION_COEFF*iDocs*log2f(iDocs)*SCALE; }
	float	Cost_LookupRead ( int64_t iDocs ) const							{ return COST_LOOKUP_READ*iDocs*SCALE; }
	float	Cost_IndexIteratorInit ( int64_t iNumIterators ) const			{ return COST_INDEX_ITERATOR_INIT*iNumIterators*SCALE; }

	float	CalcIndexCost() const;
	float	CalcFilterCost ( bool bFromIterator, float fDocsAfterIndexes ) const;
	float	CalcAnalyzerCost() const;
	float	CalcLookupCost() const;
	float	CalcMTCost ( float fCost ) const;

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
	const size_t BITMAP_ITERATOR_THRESH = 8;

	bool bFitsIteratorThresh = false;
	if ( tFilter.m_eType==SPH_FILTER_RANGE )
	{
		if ( tFilter.m_bOpenRight || tFilter.m_bOpenLeft )
			bFitsIteratorThresh = true;
		else
			bFitsIteratorThresh = ( tFilter.m_iMaxValue-tFilter.m_iMinValue+1 ) >= BITMAP_ITERATOR_THRESH;
	}
	else if ( tFilter.m_eType==SPH_FILTER_FLOATRANGE )
		bFitsIteratorThresh = true;

	if ( m_tCtx.m_iCutoff>=0 )
		iRsetSize = Min ( iRsetSize, m_tCtx.m_iCutoff );

	const int QUEUE_RSET_THRESH = 4096;
	return bFitsIteratorThresh && iRsetSize>QUEUE_RSET_THRESH;
}


static bool IsSingleValueFilter ( const CSphFilterSettings & tFilter )
{
	return  ( tFilter.m_eType==SPH_FILTER_VALUES && tFilter.m_dValues.GetLength()==1 ) ||
			( tFilter.m_eType==SPH_FILTER_STRING && tFilter.m_dStrings.GetLength()==1 );
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
			const auto & tFilter = m_tCtx.m_dFilters[i];

			if ( uNumIterators>1 && !NeedBitmapUnion ( tFilter, iDocs ) )
				fCost += Cost_IndexUnionQueue(iDocs);

			const int COST_THRESH = 1024;
			if ( iDocs/uNumIterators < COST_THRESH )
				fCost += Cost_IndexReadSparse(iDocs);
			else
			{
				if ( IsSingleValueFilter(tFilter) )
					fCost += Cost_IndexReadSingle(iDocs);
				else
					fCost += Cost_IndexReadDenseBitmap(iDocs);
			}

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

	return fFilterComplexity;
}


float CostEstimate_c::CalcFilterCost ( bool bFromIterator, float fDocsAfterIndexes ) const
{
	float fCost = 0.0f;
	ARRAY_FOREACH ( i, m_dSIInfo )
	{
		const auto & tSIInfo = m_dSIInfo[i];
		const auto & tFilter = m_tCtx.m_dFilters[i];

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
			int64_t iDocs = ApplyCutoff ( tSIInfo.m_iRsetEstimate );

			// the idea is that block filter rejects most docs and 50% of the remaining docs are filtered out
			fCost += Cost_Filter ( Min ( iDocs*2, m_tCtx.m_iTotalDocs ), fFilterComplexity );
			fCost += Cost_BlockFilter ( m_tCtx.m_iTotalDocs, fFilterComplexity );
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
		const auto & tFilter = m_tCtx.m_dFilters[i];

		if ( tSIInfo.m_eType!=SecondaryIndexType_e::ANALYZER )
			continue;

		assert ( m_tCtx.m_pColumnar );
		columnar::AttrInfo_t tAttrInfo;
		m_tCtx.m_pColumnar->GetAttrInfo ( tFilter.m_sAttrName.cstr(), tAttrInfo );

		float fFilterComplexity = CalcGetFilterComplexity ( tSIInfo, tFilter );

		// filters that process but reject values are 2x faster
		int64_t iDocsBeforeFilter = tSIInfo.m_iPartialColumnarMinMax==-1 ? m_tCtx.m_iTotalDocs : std::min ( tSIInfo.m_iPartialColumnarMinMax, m_tCtx.m_iTotalDocs );
		float fAcceptCoeff = std::min ( float(tSIInfo.m_iRsetEstimate)/iDocsBeforeFilter, 1.0f ) / 2.0f + 0.5f;
		float fTotalCoeff = fFilterComplexity*tAttrInfo.m_fComplexity*fAcceptCoeff;

		int64_t iDocsAfterCutoff = ApplyCutoff ( m_tCtx.m_iTotalDocs );

		if ( tSIInfo.m_iPartialColumnarMinMax==-1 ) // no minmax? scan whole index
			fCost += Cost_ColumnarFilter ( iDocsAfterCutoff, fTotalCoeff );
		else
		{
			// minmax tree eval
			const int MINMAX_NODE_SIZE = 1024;
			int iMatchingNodes = ( tSIInfo.m_iRsetEstimate + MINMAX_NODE_SIZE - 1 ) / MINMAX_NODE_SIZE;
			int iTreeLevels = sphLog2 ( m_tCtx.m_iTotalDocs );
			fCost += Cost_Filter ( iMatchingNodes*iTreeLevels, fFilterComplexity );
			fCost += Cost_ColumnarFilter ( std::min ( tSIInfo.m_iPartialColumnarMinMax, iDocsAfterCutoff ), fTotalCoeff );
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

	iDocsToReadLookup = ApplyCutoff(iDocsToReadLookup);
	return Cost_LookupRead ( iDocsToReadLookup );
}


float CostEstimate_c::CalcMTCost ( float fCost ) const
{
	if ( m_tCtx.m_iThreads==1 )
		return fCost;

	int iMaxThreads = sphCpuThreadsCount();

	const float fKPerf = 0.16f;
	const float fBPerf = 1.38f;

	float fMaxPerfCoeff = fKPerf*iMaxThreads + fBPerf;
	float fMinCost = fCost/fMaxPerfCoeff;

	if ( m_tCtx.m_iThreads==iMaxThreads )
		return fMinCost;

	const float fX1 = 1.0f;
	float fX2 = iMaxThreads;
	float fY1 = fCost;
	float fY2 = fMinCost;

	float fA = ( fY2-fY1 ) / ( 1.0f/float(sqrt(fX2)) - 1.0f/float(sqrt(fX1)) );
	float fB = fY1 - fA / float(sqrt(fX1));
	float fX = m_tCtx.m_iThreads;
	float fY = fA/float(sqrt(fX)) + fB;

	return fY;
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
		fCost += CalcFilterCost ( iToIntersect>0, fDocsAfterIndexes );

	if ( iNumLookups )
		fCost += CalcLookupCost();

	if ( iNumAnalyzers )
		fCost += CalcAnalyzerCost();

	if ( iNumIndexes )
		fCost += CalcIndexCost();

	fCost += Cost_Push ( uint64_t(fDocsAfterFilters*m_tCtx.m_iTotalDocs) );

	if ( !iNumIndexes && !iNumLookups ) // SI and docid lookups always run in a single thread
		fCost = CalcMTCost(fCost);

	return fCost;
}

/////////////////////////////////////////////////////////////////////

SelectIteratorCtx_t::SelectIteratorCtx_t ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> & dFilterTree, const CSphVector<IndexHint_t> & dHints, const ISphSchema &	tSchema, const HistogramContainer_c * pHistograms, columnar::Columnar_i * pColumnar, SI::Index_i * pSI, ESphCollation eCollation, int iCutoff, int64_t iTotalDocs, int iThreads )
	: m_dFilters ( dFilters )
	, m_dFilterTree ( dFilterTree )
	, m_dHints ( dHints )
	, m_tSchema ( tSchema )
	, m_pHistograms ( pHistograms )
	, m_pColumnar ( pColumnar )
	, m_pSI ( pSI )
	, m_eCollation ( eCollation )
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
	if ( pCol->m_eAttrType==SPH_ATTR_STRING && m_eCollation!=SPH_COLLATION_DEFAULT )
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