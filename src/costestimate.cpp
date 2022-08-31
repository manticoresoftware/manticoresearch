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

	static constexpr float COST_PUSH					= 12.5f;
	static constexpr float COST_FILTER					= 8.5f;
	static constexpr float COST_COLUMNAR_FILTER			= 1.5f;
	static constexpr float COST_INTERSECT				= 5.0f;
	static constexpr float COST_INDEX_READ_SINGLE		= 1.0f;
	static constexpr float COST_INDEX_READ_DENSE_BITMAP	= 30.0f;
	static constexpr float COST_INDEX_READ_SPARSE		= 700.0f;
	static constexpr float COST_INDEX_UNION_COEFF		= 0.7f;
	static constexpr float COST_LOOKUP_READ				= 33.0f;
	static constexpr float COST_INDEX_ITERATOR_INIT		= 200.0f;

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

	float	CalcGetFilterComplexity ( const SecondaryIndexInfo_t & tSIInfo, const CSphFilterSettings & tFilter ) const;
	bool	NeedBitmapUnion ( const CSphFilterSettings & tFilter, int64_t iRsetSize ) const;
	bool	IsWideRange ( const CSphFilterSettings & tFilter ) const;
	uint32_t CalcNumSIIterators ( const CSphFilterSettings & tFilter, int64_t iDocs ) const;
	bool	HasSeveralSIIterators ( const CSphFilterSettings & tFilter ) const;
};


CostEstimate_c::CostEstimate_c ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const SelectIteratorCtx_t & tCtx )
	: m_dSIInfo ( dSIInfo )
	, m_tCtx ( tCtx )
{}


bool CostEstimate_c::NeedBitmapUnion ( const CSphFilterSettings & tFilter, int64_t iRsetSize ) const
{
	// this needs to be in sync with iterator construction code
	const size_t BITMAP_ITERATOR_THRESH = 16;
	const float	BITMAP_RATIO_THRESH = 0.002;

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

	float fRsetRatio = float ( iRsetSize ) / m_tCtx.m_iTotalDocs;
	return bFitsIteratorThresh && fRsetRatio >= BITMAP_RATIO_THRESH;
}


bool CostEstimate_c::IsWideRange ( const CSphFilterSettings & tFilter ) const
{
	if ( tFilter.m_eType==SPH_FILTER_FLOATRANGE )
		return true;

	if ( tFilter.m_eType!=SPH_FILTER_RANGE )
		return false;

	if ( tFilter.m_bOpenLeft || tFilter.m_bOpenRight )
		return true;

	const int WIDE_RANGE_THRESH=10000;
	return ( tFilter.m_iMaxValue-tFilter.m_iMinValue ) >= WIDE_RANGE_THRESH;
}


static bool IsSingleValueFilter ( const CSphFilterSettings & tFilter )
{
	return tFilter.m_eType==SPH_FILTER_VALUES && tFilter.m_dValues.GetLength()==1;
}


float CostEstimate_c::CalcIndexCost() const
{
	int iNumIndexes = 0;
	int64_t iSumDocsFromIndexes = 0;
	float fCost = 0.0f;
	ARRAY_FOREACH ( i, m_dSIInfo )
	{
		const auto & tSIInfo = m_dSIInfo[i];
		int64_t iDocs = tSIInfo.m_iRsetEstimate;

		if ( tSIInfo.m_eType==SecondaryIndexType_e::LOOKUP ||
			tSIInfo.m_eType==SecondaryIndexType_e::ANALYZER ||
			tSIInfo.m_eType==SecondaryIndexType_e::INDEX )
			iSumDocsFromIndexes += iDocs;

		if ( tSIInfo.m_eType!=SecondaryIndexType_e::INDEX )
			continue;

		iNumIndexes++;

		uint32_t uNumIterators = 1;
		const auto & tFilter = m_tCtx.m_dFilters[i];
		if ( HasSeveralSIIterators(tFilter) )
		{
			if ( !NeedBitmapUnion ( tFilter, iDocs ) )
				fCost += Cost_IndexUnionQueue(iDocs);

			// we fetch real number of iterators from PGM only when we suspect that the query may have a lot of iterators
			// otherwise we just try to guess
			uNumIterators = CalcNumSIIterators ( tFilter, iDocs );
		}

		if ( uNumIterators )
		{
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
		int64_t iDocs = tSIInfo.m_iRsetEstimate;

		if ( bFromIterator )
			fCost += Cost_Filter ( int64_t(fDocsAfterIndexes*m_tCtx.m_iTotalDocs), fFilterComplexity );
		else
		{
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

		float fFilterComplexity = CalcGetFilterComplexity ( tSIInfo, tFilter );
		int64_t iDocs = tSIInfo.m_iRsetEstimate;

		// minmax tree eval
		fCost += Cost_Filter ( m_tCtx.m_iTotalDocs/1024*1.33f, fFilterComplexity );

		// the idea is that minmax rejects most docs and 50% of the remaining docs are filtered out
		fCost += Cost_ColumnarFilter ( Min ( iDocs*2, m_tCtx.m_iTotalDocs ), fFilterComplexity );
	}

	return fCost;
}



uint32_t CostEstimate_c::CalcNumSIIterators ( const CSphFilterSettings & tFilter, int64_t iDocs ) const
{
	uint32_t uNumIterators = 1;
	if ( !m_tCtx.m_pSI )
		return uNumIterators;

	// if we suspect that that index may fetch A LOT of iterators,
	// we ask the SI about it explicitly
	const int DOC_THRESH = 10000;
	if ( !IsWideRange(tFilter) || iDocs<DOC_THRESH )
		return uNumIterators;

	common::Filter_t tColumnarFilter;
	CSphString sWarning;
	if ( !ToColumnarFilter ( tColumnarFilter, tFilter, m_tCtx.m_eCollation, m_tCtx.m_tSchema, sWarning ) )
		return 0;

	return m_tCtx.m_pSI->GetNumIterators(tColumnarFilter);
}


bool CostEstimate_c::HasSeveralSIIterators ( const CSphFilterSettings & tFilter ) const
{
	if ( tFilter.m_eType==SPH_FILTER_VALUES && tFilter.m_dValues.GetLength()==1 )
		return false;

	return true;
}


float CostEstimate_c::CalcQueryCost()
{
	int64_t iDocsToReadLookup = 0;

	float fCost = 0.0f;
	float fDocsAfterIndexes = 1.0f;
	float fDocsAfterFilters = 1.0f;
	int iToIntersect = 0;
	int iNumFilters = 0;
	int iNumAnalyzers = 0;
	int iNumIndexes = 0;

	ARRAY_FOREACH ( i, m_dSIInfo )
	{
		const auto & tSIInfo = m_dSIInfo[i];
		const auto & tFilter = m_tCtx.m_dFilters[i];
		if ( tFilter.m_sAttrName=="@rowid" )
			continue;

		int64_t iDocs = tSIInfo.m_iRsetEstimate;
		float fIndexProbability = float(iDocs) / m_tCtx.m_iTotalDocs;

		if ( tSIInfo.m_eType==SecondaryIndexType_e::LOOKUP ||
			 tSIInfo.m_eType==SecondaryIndexType_e::ANALYZER ||
			 tSIInfo.m_eType==SecondaryIndexType_e::INDEX )
		{
			fDocsAfterIndexes *= fIndexProbability;
			iToIntersect++;
		}

		fDocsAfterFilters *= fIndexProbability;

		switch ( tSIInfo.m_eType )
		{
		case SecondaryIndexType_e::LOOKUP:
			iDocsToReadLookup += iDocs; 
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

	fCost += Cost_LookupRead ( iDocsToReadLookup );

	if ( iNumAnalyzers )
		fCost += CalcAnalyzerCost();

	if ( iNumIndexes )
		fCost += CalcIndexCost();

	int64_t iDocsToPush = uint64_t(fDocsAfterFilters*m_tCtx.m_iTotalDocs);
	if ( m_tCtx.m_iCutoff>=0 )
		iDocsToPush = Min ( iDocsToPush, m_tCtx.m_iCutoff );

	fCost += Cost_Push ( iDocsToPush );

	return fCost;
}

/////////////////////////////////////////////////////////////////////

SelectIteratorCtx_t::SelectIteratorCtx_t ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> &	dFilterTree, const CSphVector<IndexHint_t> & dHints, const ISphSchema &	tSchema, const HistogramContainer_c * pHistograms, SI::Index_i * pSI, ESphCollation eCollation, int iCutoff, int64_t iTotalDocs )
	: m_dFilters ( dFilters )
	, m_dFilterTree ( dFilterTree )
	, m_dHints ( dHints )
	, m_tSchema ( tSchema )
	, m_pHistograms ( pHistograms )
	, m_pSI ( pSI )
	, m_eCollation ( eCollation )
	, m_iCutoff ( iCutoff )
	, m_iTotalDocs ( iTotalDocs )
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