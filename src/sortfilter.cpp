//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sortfilter.h"
#include "sphinxint.h"
#include "columnarlib.h"
#include "columnar/columnar.h"
#include "sphinxfilter.h"


class ColumnarSortFilter_c final
{
public:
				ColumnarSortFilter_c ( const SortBound_c & tBound, const columnar::Columnar_i & tColumnar );

	std::unique_ptr<ColumnarSortFilter_c> CloneCursor() const;
	RowID_t		AdvanceTo ( RowID_t tMin, RowID_t tRows ) const;

	bool		IsValid() const		{ return bool(m_pValues); }
	bool		ShouldRejectChunk() const;
	int			SelectRows ( RowID_t * pRows, int iCount );

private:
	const SortBound_c &				m_tBound;
	const columnar::Columnar_i &	m_tColumnar;

	std::unique_ptr<columnar::Iterator_i> m_pValues;
	const columnar::BlockBounds_i * m_pBounds = nullptr;
	int			m_iLeafLevel = 0;
	int			m_iLeafRows = 0;
	SphAttr_t	m_tChunkBest = 0;

	SphAttr_t	GetBestBound ( int iLevel, RowID_t tBlock ) const;
	bool		Accepts ( SphAttr_t tValue ) const;
};


ColumnarSortFilter_c::ColumnarSortFilter_c ( const SortBound_c & tBound, const columnar::Columnar_i & tColumnar )
	: m_tBound(tBound)
	, m_tColumnar(tColumnar)
{
	std::string sError;
	m_pValues = CreateColumnarIterator ( &tColumnar, tBound.GetAttr().cstr(), sError );
	m_pBounds = tColumnar.GetBlockBounds ( tBound.GetAttr().cstr() );
	if ( m_pBounds )
	{
		m_iLeafLevel = m_pBounds->GetNumMinMaxLevels()-1;
		m_iLeafRows = m_pBounds->GetLeafRows();
		m_tChunkBest = GetBestBound ( 0, 0 );
	}
}


std::unique_ptr<ColumnarSortFilter_c> ColumnarSortFilter_c::CloneCursor() const
{
	return std::make_unique<ColumnarSortFilter_c> ( m_tBound, m_tColumnar );
}


bool ColumnarSortFilter_c::Accepts ( SphAttr_t tValue ) const
{
	return m_tBound.IsDescending() ? tValue>=m_tBound.GetValue() : tValue<=m_tBound.GetValue();
}


SphAttr_t ColumnarSortFilter_c::GetBestBound ( int iLevel, RowID_t tBlock ) const
{
	const auto tRange = m_pBounds->GetMinMax ( iLevel, tBlock );
	return m_tBound.IsDescending() ? tRange.second : tRange.first;
}


bool ColumnarSortFilter_c::ShouldRejectChunk() const
{
	return m_pBounds && !Accepts(m_tChunkBest);
}


RowID_t ColumnarSortFilter_c::AdvanceTo ( RowID_t tMin, RowID_t tRows ) const
{
	if ( !m_pBounds )
		return tMin;

	while ( tMin<tRows )
	{
		RowID_t tBlock = tMin/m_iLeafRows;
		if ( Accepts ( GetBestBound ( m_iLeafLevel, tBlock ) ) )
			return tMin;

		// Skip the remainder of the largest enclosing block that cannot meet the current threshold.
		int iLevel = m_iLeafLevel;
		while ( iLevel>0 && !Accepts ( GetBestBound ( iLevel-1, tBlock>>1 ) ) )
		{
			--iLevel;
			tBlock >>= 1;
		}

		const uint64_t uNext = (uint64_t(tBlock+1)<<(m_iLeafLevel-iLevel))*m_iLeafRows;
		if ( uNext>=tRows )
			break;

		tMin = RowID_t(uNext);
	}

	return INVALID_ROWID;
}


int ColumnarSortFilter_c::SelectRows ( RowID_t * pRows, int iCount )
{
	int iKept = 0;
	for ( int i=0; i<iCount; ++i )
		if ( Accepts ( m_pValues->Get(pRows[i]) ) )
			pRows[iKept++] = pRows[i];

	return iKept;
}


SortBoundFilter_c::SortBoundFilter_c ( const SortBound_c & tBound, const CSphAttrLocator & tLocator, const DWORD * pRows, const DWORD * pMinMax, RowID_t tRows, int iStride, const SIContainer_c * pSI )
	: m_tBound ( tBound )
	, m_tLocator ( tLocator )
	, m_pRows ( pRows )
	, m_pMinMax ( pMinMax )
	, m_tRows ( tRows )
	, m_iStride ( iStride )
	, m_pSI ( pSI )
{}


SortBoundFilter_c::SortBoundFilter_c ( const SortBound_c & tBound, const columnar::Columnar_i & tColumnar, RowID_t tRows, const SIContainer_c * pSI )
	: SortBoundFilter_c ( tBound, {}, nullptr, nullptr, tRows, 0, pSI )
{
	m_pColumnar = std::make_unique<ColumnarSortFilter_c> ( tBound, tColumnar );
}


SortBoundFilter_c::~SortBoundFilter_c() = default;


bool SortBoundFilter_c::IsValid() const
{
	return !m_pColumnar || m_pColumnar->IsValid();
}


std::unique_ptr<PostingCandidateFilter_c> SortBoundFilter_c::CloneCursor()
{
	auto pBranch = std::make_unique<SortBoundFilter_c> ( m_tBound, m_tLocator, m_pRows, m_pMinMax, m_tRows, m_iStride, m_pSI );
	if ( m_pColumnar )
		pBranch->m_pColumnar = m_pColumnar->CloneCursor();

	pBranch->m_pSource = m_pSource;
	pBranch->SetFilterComplexity(GetFilterComplexity());
	return pBranch;
}


bool SortBoundFilter_c::Accepts ( SphAttr_t tValue ) const
{
	return m_tBound.IsDescending() ? tValue>=m_tBound.GetValue() : tValue<=m_tBound.GetValue();
}


void SortBoundFilter_c::SelectSource()
{
	if ( m_bCursorReady )
		return;

	m_bCursorReady = true;
	auto & tSource = *m_pSource;
	if ( !tSource.m_bSelected )
	{
		tSource.m_bSelected = true;
		tSource.m_tSourceBound = m_tBound.GetValue();
		if ( m_pSI )
			m_pIterator = CreateSortBoundIterator ( *m_pSI, m_tBound.GetAttr(), tSource.m_tSourceBound, m_tBound.IsDescending(), m_tRows, tSource.m_bSI );

		m_bIteratorReady = tSource.m_bSI;
	} else if ( tSource.m_bSI )
	{
		// Every branch uses the same immutable SI range and its own iterator.
		m_pIterator = CreateSortBoundIterator ( *m_pSI, m_tBound.GetAttr(), tSource.m_tSourceBound, m_tBound.IsDescending(), m_tRows, m_bIteratorReady );
	}
}


bool SortBoundFilter_c::ShouldRejectChunk()
{
	if ( m_pColumnar )
		return m_pSource->m_pColumnar->ShouldRejectChunk();

	uint64_t uBlocks = (uint64_t(m_tRows)+DOCINFO_INDEX_FREQ-1)/DOCINFO_INDEX_FREQ;
	const DWORD * pBound = m_pMinMax + (2*uBlocks + int(m_tBound.IsDescending()))*m_iStride;
	return !Accepts ( sphGetRowAttr ( pBound, m_tLocator ) );
}


RowID_t SortBoundFilter_c::AdvanceIterator ( RowID_t tMin )
{
	while ( m_pIterator )
	{
		if ( m_iRead<m_dRows.GetLength() )
		{
			m_iRead = int(std::lower_bound ( m_dRows.Begin()+m_iRead, m_dRows.End(), tMin )-m_dRows.Begin());
			if ( m_iRead<m_dRows.GetLength() )
				return m_dRows[m_iRead];
		}

		if ( !m_pIterator->HintRowID(tMin) || !m_pIterator->GetNextRowIdBlock(m_dRows) )
		{
			m_pIterator.reset();
			break;
		}

		m_iRead = 0;
	}

	return INVALID_ROWID;
}


RowID_t SortBoundFilter_c::AdvanceTo ( RowID_t tMin )
{
	if ( !m_tBound.IsSet() || !IsValid() )
		return tMin;

	// Reject a wholly noncompetitive chunk before allocating an SI iterator.
	// Once SI is selected, it remains the only range source for this chunk.
	if ( (!m_pSource->m_bSelected || !m_pSource->m_bSI) && ShouldRejectChunk() )
	{
		m_pSource->m_bSelected = true;
		m_pSource->m_bPruned = true;
		return INVALID_ROWID;
	}

	SelectSource();

	RowID_t tNext = tMin;
	if ( m_pSource->m_bSI )
		tNext = m_bIteratorReady ? AdvanceIterator(tMin) : tMin;
	else if ( m_pColumnar )
		tNext = m_pColumnar->AdvanceTo ( tMin, m_tRows );
	else
	{
		// Return a conservative candidate periodically so the caller can check interruption.
		for ( int i=0; i<256 && tNext<m_tRows; ++i )
		{
			uint64_t uBlock = tNext/DOCINFO_INDEX_FREQ;
			const DWORD * pBound = m_pMinMax + (2*uBlock + int(m_tBound.IsDescending()))*m_iStride;
			if ( Accepts ( sphGetRowAttr ( pBound, m_tLocator ) ) )
				break;

			tNext = RowID_t(std::min ( (uBlock+1)*DOCINFO_INDEX_FREQ, uint64_t(m_tRows) ));
		}

		if ( tNext>=m_tRows )
			tNext = INVALID_ROWID;
	}

	m_pSource->m_bPruned |= tNext>tMin;
	return tNext;
}


int SortBoundFilter_c::SelectRows ( RowID_t * pRows, int iCount )
{
	if ( !m_tBound.IsSet() || !IsValid() )
		return iCount;

	int iKept = 0;
	if ( m_pColumnar )
		iKept = m_pColumnar->SelectRows ( pRows, iCount );
	else
		for ( int i=0; i<iCount; ++i )
			if ( Accepts ( sphGetRowAttr ( m_pRows+uint64_t(pRows[i])*m_iStride, m_tLocator ) ) )
				pRows[iKept++] = pRows[i];

	m_pSource->m_bPruned |= iKept<iCount;
	return iKept;
}
