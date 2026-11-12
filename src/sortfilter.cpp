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

#include <algorithm>


SortBoundFilter_c::SortBoundFilter_c ( const SortBound_c & tBound, const CSphAttrLocator & tLocator, const DWORD * pRows, const DWORD * pMinMax, RowID_t tRows, int iStride, const SIContainer_c * pSI )
	: m_tBound ( tBound )
	, m_tLocator ( tLocator )
	, m_pRows ( pRows )
	, m_pMinMax ( pMinMax )
	, m_tRows ( tRows )
	, m_iStride ( iStride )
	, m_pSI ( pSI )
{}


std::unique_ptr<PostingCandidateFilter_i> SortBoundFilter_c::CloneCursor()
{
	auto pBranch = std::make_unique<SortBoundFilter_c> ( m_tBound, m_tLocator, m_pRows, m_pMinMax, m_tRows, m_iStride, m_pSI );
	pBranch->m_pSource = m_pSource;
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

		m_bSIReady = tSource.m_bSI;
	} else if ( tSource.m_bSI )
	{
		// Every branch uses the same immutable SI range and its own iterator.
		m_pIterator = CreateSortBoundIterator ( *m_pSI, m_tBound.GetAttr(), tSource.m_tSourceBound, m_tBound.IsDescending(), m_tRows, m_bSIReady );
	}
}


RowID_t SortBoundFilter_c::AdvanceSI ( RowID_t tMin )
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
	if ( !m_tBound.IsSet() )
		return tMin;

	// Reject a wholly noncompetitive chunk before allocating an SI iterator.
	// Once SI is selected, it remains the only range source for this chunk.
	if ( !m_pSource->m_bSelected || !m_pSource->m_bSI )
	{
		uint64_t uBlocks = (uint64_t(m_tRows)+DOCINFO_INDEX_FREQ-1)/DOCINFO_INDEX_FREQ;
		const DWORD * pBound = m_pMinMax + (2*uBlocks + int(m_tBound.IsDescending()))*m_iStride;
		if ( !Accepts ( sphGetRowAttr ( pBound, m_tLocator ) ) )
		{
			m_pSource->m_bSelected = true;
			m_pSource->m_bPruned = true;
			return INVALID_ROWID;
		}
	}

	SelectSource();

	RowID_t tNext = tMin;
	if ( m_pSource->m_bSI )
		tNext = m_bSIReady ? AdvanceSI(tMin) : tMin;
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
	if ( !m_tBound.IsSet() )
		return iCount;

	int iKept = 0;
	for ( int i=0; i<iCount; ++i )
		if ( Accepts ( sphGetRowAttr ( m_pRows+uint64_t(pRows[i])*m_iStride, m_tLocator ) ) )
			pRows[iKept++] = pRows[i];

	m_pSource->m_bPruned |= iKept<iCount;
	return iKept;
}
