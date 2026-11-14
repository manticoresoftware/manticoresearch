//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "diskqword.h"

DiskIndexQwordSetup_c::DiskIndexQwordSetup_c ( DataReaderFactoryPtr_c pDoclist, DataReaderFactoryPtr_c pHitlist, const BYTE * pSkips, int iSkiplistBlockSize, bool bSetupReaders, RowID_t iRowsCount )
	: m_pDoclist ( std::move ( pDoclist ) )
	, m_pHitlist ( std::move ( pHitlist ) )
	, m_pSkips ( pSkips )
	, m_iSkiplistBlockSize ( iSkiplistBlockSize )
	, m_bSetupReaders ( bSetupReaders )
	, m_iRowsCount ( iRowsCount )
{}


// the hint is the dictionary's estimate of the list length; unknown or long lists get the head only
static SphOffset_t PrefetchBytes ( int iHint )
{
	return ( iHint>0 && iHint<=PREFETCH_WHOLE_DOCLIST_BYTES ) ? iHint : PREFETCH_DOCLIST_HEAD_BYTES;
}


void DiskIndexQwordSetup_c::PrefetchDoclist ( SphOffset_t iOff, int iHint ) const
{
	if ( !m_pDoclist || !m_pDoclist->IsMapped() || !m_tPrefetchGate.ShouldProbe() )
		return;

	if ( m_pDoclist->Prefetch ( iOff, PrefetchBytes ( iHint ) ) )
		m_tPrefetchGate.OnMiss();
}


void DiskIndexQwordSetup_c::PrefetchPayload ( const DiskSubstringPayload_t & tPayload ) const
{
	if ( !m_pDoclist || !m_pDoclist->IsMapped() || tPayload.m_dDoclist.IsEmpty() || !m_tPrefetchGate.ShouldProbe() )
		return;

	// every slice in one request; the factory probes the first one and lets the OS coalesce the rest
	CSphFixedVector<FileRange_t> dRanges ( tPayload.m_dDoclist.GetLength() );
	ARRAY_FOREACH ( i, dRanges )
		dRanges[i] = { (SphOffset_t)tPayload.m_dDoclist[i].m_uOff, PrefetchBytes ( tPayload.m_dDoclist[i].m_iLen ) };

	if ( m_pDoclist->Prefetch ( dRanges.Begin(), dRanges.GetLength() ) )
		m_tPrefetchGate.OnMiss();
}


bool DiskIndexQwordTraits_c::WantHitPrefetch()
{
	return m_pHitFactory && m_pHitFactory->IsMapped() && m_tHitGate.ShouldProbe();
}


void DiskIndexQwordTraits_c::PrefetchHits ( const SphOffset_t * pOffsets, const DWORD * pHits, int iCount )
{
	if ( !m_pHitFactory || iCount<=0 )
		return;

	// exactly the hitlists about to be seeked to: inlined single hits live in the doclist and need nothing;
	// a document's range is sized from its hit count (a couple of bytes per hit), bounded, or one page when unknown
	CSphFixedVector<FileRange_t> dRanges ( iCount );
	int iRanges = 0;
	for ( int i = 0; i < iCount; i++ )
	{
		if ( pOffsets[i]>>63 )
			continue; // inline hit: no hitlist bytes to prefetch.

		SphOffset_t iLen = pHits ? Min ( Max ( (SphOffset_t)pHits[i]*2 + 16, (SphOffset_t)256 ), (SphOffset_t)PREFETCH_DOCLIST_HEAD_BYTES ) : (SphOffset_t)4096;
		dRanges[iRanges++] = { pOffsets[i], iLen };
	}

	if ( iRanges && m_pHitFactory->Prefetch ( dRanges.Begin(), iRanges ) )
		m_tHitGate.OnMiss();
}


DiskIndexQwordTraits_c * sphCreateDiskIndexQword ( bool bInlineHits, bool bBlockPostings )
{
	if ( bBlockPostings )
		return new BlockIndexQword_c ( false, false, 0 );

	if ( bInlineHits )
		return new DiskIndexQword_c<true,false,BlockReader_t> ( false, false, 0 );

	return new DiskIndexQword_c<false,false,BlockReader_t> ( false, false, 0 );
}


template<bool MAPPED>
static DiskIndexQwordTraits_c * SpawnBlockQword ( const XQKeyword_t & tWord, DataReaderFactory_c * pDocs, DataReaderFactory_c * pHits, int64_t iIndexId )
{
	using Qword_t = BlockIndexQword_T<MAPPED>;
	if ( !tWord.m_pPayload )
		return new Qword_t ( tWord.m_bExpanded, tWord.m_bExcluded, iIndexId );

	return new DiskPayloadQword_c<false,BlockReader_t,Qword_t> ( (const DiskSubstringPayload_t *)tWord.m_pPayload, tWord.m_bExcluded, pDocs, pHits, iIndexId );
}


ISphQword * DiskIndexQwordSetup_c::QwordSpawn ( const XQKeyword_t & tWord ) const
{
	if ( tWord.m_pPayload && m_bSetupReaders )
		PrefetchPayload ( *(const DiskSubstringPayload_t *)tWord.m_pPayload );

	if ( m_pIndex->GetSettings().m_uPostingsFormat )
	{
		DiskIndexQwordTraits_c * pWord = m_pHitlist && m_pHitlist->IsMapped()
			? SpawnBlockQword<true> ( tWord, m_pDoclist, m_pHitlist, m_pIndex->GetIndexId() )
			: SpawnBlockQword<false> ( tWord, m_pDoclist, m_pHitlist, m_pIndex->GetIndexId() );
		pWord->SetReadErrorTarget(m_pError);
		return pWord;
	}

	const bool bInline = m_pIndex->GetSettings().m_eHitFormat==SPH_HIT_FORMAT_INLINE;
	// raw-pointer decoding when both lists are memory-mapped, buffered block reading otherwise
	const bool bRaw = m_pDoclist && m_pHitlist && m_pDoclist->IsMapped() && m_pHitlist->IsMapped();
	const int64_t iIndexId = m_pIndex->GetIndexId();

	if ( !tWord.m_pPayload )
	{
		if ( bInline )
			return bRaw
				? (ISphQword *) new DiskIndexQword_c<true,false,RawReader_t> ( tWord.m_bExpanded, tWord.m_bExcluded, iIndexId )
				: (ISphQword *) new DiskIndexQword_c<true,false,BlockReader_t> ( tWord.m_bExpanded, tWord.m_bExcluded, iIndexId );

		return bRaw
			? (ISphQword *) new DiskIndexQword_c<false,false,RawReader_t> ( tWord.m_bExpanded, tWord.m_bExcluded, iIndexId )
			: (ISphQword *) new DiskIndexQword_c<false,false,BlockReader_t> ( tWord.m_bExpanded, tWord.m_bExcluded, iIndexId );
	}

	auto * pPayload = (const DiskSubstringPayload_t *)tWord.m_pPayload;

	if ( bInline )
		return bRaw
			? (ISphQword *) new DiskPayloadQword_c<true,RawReader_t> ( pPayload, tWord.m_bExcluded, m_pDoclist, m_pHitlist, iIndexId )
			: (ISphQword *) new DiskPayloadQword_c<true,BlockReader_t> ( pPayload, tWord.m_bExcluded, m_pDoclist, m_pHitlist, iIndexId );

	return bRaw
		? (ISphQword *) new DiskPayloadQword_c<false,RawReader_t> ( pPayload, tWord.m_bExcluded, m_pDoclist, m_pHitlist, iIndexId )
		: (ISphQword *) new DiskPayloadQword_c<false,BlockReader_t> ( pPayload, tWord.m_bExcluded, m_pDoclist, m_pHitlist, iIndexId );
}
