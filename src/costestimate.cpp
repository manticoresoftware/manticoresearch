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
#include <math.h>

class CostEstimate_c : public CostEstimate_i
{
public:
			CostEstimate_c ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, int64_t iTotalDocs, int iCutoff );

	float	CalcQueryCost() final;

private:
	static constexpr float SCALE = 1.0f/1000000.0f;

	static constexpr float COST_PUSH				= 12.5f;
	static constexpr float COST_FILTER				= 8.5f;
	static constexpr float COST_COLUMNAR_FILTER		= 1.5f;
	static constexpr float COST_INTERSECT			= 5.0f;
	static constexpr float COST_INDEX_READ			= 1.0f;
	static constexpr float COST_INDEX_UNION_BITMAP	= 6.0f;
	static constexpr float COST_INDEX_UNION_COEFF	= 0.9f;
	static constexpr float COST_LOOKUP_READ			= 33.0f;

	const CSphVector<SecondaryIndexInfo_t> &	m_dSIInfo;
	const CSphVector<CSphFilterSettings> &		m_dFilters;

	int64_t		m_iTotalDocs = 0;
	int			m_iCutoff = 0;

	float Cost_Filter ( int64_t iDocs ) const			{ return COST_FILTER*float(iDocs)*SCALE; }
	float Cost_BlockFilter ( int64_t iDocs ) const		{ return Cost_Filter ( iDocs/DOCINFO_INDEX_FREQ ); }
	float Cost_ColumnarFilter ( int64_t iDocs ) const	{ return COST_COLUMNAR_FILTER*float(iDocs)*SCALE; }
	float Cost_Push ( int64_t iDocs ) const				{ return COST_PUSH*float(iDocs)*SCALE; }
	float Cost_Intersect ( int64_t iDocs ) const		{ return COST_INTERSECT*float(iDocs)*SCALE; }
	float Cost_IndexRead ( int64_t iDocs ) const		{ return COST_INDEX_READ*float(iDocs)*SCALE; }
	float Cost_IndexUnionBitmap ( int64_t iDocs ) const	{ return COST_INDEX_UNION_BITMAP*float(iDocs)*SCALE; }
	float Cost_IndexUnionQueue ( int64_t iDocs ) const	{ return COST_INDEX_UNION_COEFF*float(iDocs)*log2f(iDocs)*SCALE; }
	float Cost_LookupRead ( int64_t iDocs ) const		{ return COST_LOOKUP_READ*float(iDocs)*SCALE; }

	bool NeedBitmapUnion ( const CSphFilterSettings & tFilter, int64_t iRsetSize ) const;
};


CostEstimate_c::CostEstimate_c ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, int64_t iTotalDocs, int iCutoff )
	: m_dSIInfo ( dSIInfo )
	, m_dFilters ( dFilters )
	, m_iTotalDocs ( iTotalDocs )
	, m_iCutoff ( iCutoff )
{}


bool CostEstimate_c::NeedBitmapUnion ( const CSphFilterSettings & tFilter, int64_t iRsetSize ) const
{
	// this needs to be in sync with iterator construction code
	const size_t BITMAP_ITERATOR_THRESH = 16;
	const float	BITMAP_RATIO_THRESH = 0.002;

	bool bFitsIteratorThresh = false;
	if ( tFilter.m_eType==SPH_FILTER_RANGE )
		bFitsIteratorThresh = ( tFilter.m_iMaxValue-tFilter.m_iMinValue+1 ) >= BITMAP_ITERATOR_THRESH;
	else if ( tFilter.m_eType==SPH_FILTER_FLOATRANGE )
		bFitsIteratorThresh = true;

	if ( m_iCutoff>=0 )
		iRsetSize = Min ( iRsetSize, m_iCutoff );

	float fRsetRatio = float ( iRsetSize ) / m_iTotalDocs;
	return bFitsIteratorThresh && fRsetRatio >= BITMAP_RATIO_THRESH;
}


float CostEstimate_c::CalcQueryCost()
{
	int64_t iDocsToReadLookup = 0;
	int64_t iDocsToReadAnalyzer = 0;
	int64_t iDocsToReadIndex = 0;

	float fCost = 0.0f;
	int iSumDocsFromIndexes = 0;
	int iSumDocsFromFilters = 0;
	float fDocsAfterIndexes = 1.0f;
	float fDocsAfterFilters = 1.0f;
	int iToIntersect = 0;
	int iNumFilters = 0;
	int iNumAnalyzers = 0;
	int iNumIndexes = 0;

	for ( const auto & i : m_dSIInfo )
	{
		int64_t iDocs = i.m_iRsetEstimate;
		float fIndexProbability = float(iDocs) / m_iTotalDocs;

		if ( i.m_eType==SecondaryIndexType_e::LOOKUP ||
			 i.m_eType==SecondaryIndexType_e::ANALYZER ||
			 i.m_eType==SecondaryIndexType_e::INDEX )
		{
			iSumDocsFromIndexes += iDocs;
			fDocsAfterIndexes *= fIndexProbability;
			iToIntersect++;
		}
		else
			iSumDocsFromFilters += iDocs;

		fDocsAfterFilters *= fIndexProbability;

		switch ( i.m_eType )
		{
		case SecondaryIndexType_e::LOOKUP:
			iDocsToReadLookup += iDocs; 
			break;

		case SecondaryIndexType_e::ANALYZER:
			iDocsToReadAnalyzer += iDocs;
			iNumAnalyzers++;
			break;

		case SecondaryIndexType_e::INDEX:
			iDocsToReadIndex += iDocs;
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
	{
		bool bFromIterator = iToIntersect>0;
		if ( bFromIterator )
		{
			int64_t iDocsLeftToFilter = uint64_t(fDocsAfterIndexes*m_iTotalDocs);
			fCost += Cost_Filter ( iDocsLeftToFilter );
		}
		else
		{
			// the idea is that block filter rejects most docs and 50% of the remaining docs are filtered out
			fCost += Cost_Filter ( Min ( iSumDocsFromFilters*2, m_iTotalDocs ) );
			fCost += Cost_BlockFilter ( m_iTotalDocs*iNumFilters );
		}
	}

	fCost += Cost_LookupRead ( iDocsToReadLookup );

	if ( iNumAnalyzers )
	{
		// minmax tree eval
		fCost += Cost_Filter ( m_iTotalDocs/1024*1.33f*iNumAnalyzers );

		// the idea is that minmax rejects most docs and 50% of the remaining docs are filtered out
		fCost += Cost_ColumnarFilter ( Min ( iDocsToReadAnalyzer*2, m_iTotalDocs ) );
	}

	if ( iNumIndexes )
	{
		fCost += Cost_IndexRead ( iDocsToReadIndex );

		ARRAY_FOREACH ( i, m_dSIInfo )
		{
			const auto & tSIInfo = m_dSIInfo[i];
			if ( tSIInfo.m_eType!=SecondaryIndexType_e::INDEX )
				continue;

			int64_t iDocs = tSIInfo.m_iRsetEstimate;
			if ( NeedBitmapUnion ( m_dFilters[i], iDocs ) )
				fCost += Cost_IndexUnionBitmap ( iDocs );
			else 
				fCost += Cost_IndexUnionQueue ( iDocs );
		}
	}
	
	if ( iToIntersect>1 )
		fCost += Cost_Intersect ( iSumDocsFromIndexes );

	int64_t iDocsToPush = uint64_t(fDocsAfterFilters*m_iTotalDocs);
	if ( m_iCutoff>=0 )
		iDocsToPush = Min ( iDocsToPush, m_iCutoff );

	fCost += Cost_Push ( iDocsToPush );

	return fCost;
}

/////////////////////////////////////////////////////////////////////

CostEstimate_i * CreateCostEstimate ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, int64_t iTotalDocs, int iCutoff )
{
	return new CostEstimate_c ( dSIInfo, dFilters, iTotalDocs, iCutoff );
}