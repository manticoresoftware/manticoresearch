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


DiskIndexQwordTraits_c * sphCreateDiskIndexQword ( bool bInlineHits )
{
	if ( bInlineHits )
		return new DiskIndexQword_c<true,false,BlockReader_t> ( false, false, 0 );

	return new DiskIndexQword_c<false,false,BlockReader_t> ( false, false, 0 );
}


QwordType_e DiskIndexQwordSetup_c::GetQwordType() const
{
	const bool bInline = m_pIndex->GetSettings().m_eHitFormat==SPH_HIT_FORMAT_INLINE;
	// Use the actual factories: mixed file/mmap access must keep the block reader.
	const bool bRaw = m_pDoclist && m_pHitlist && m_pDoclist->IsMapped() && m_pHitlist->IsMapped();
	if ( bRaw )
		return bInline ? QwordType_e::DISK_INLINE_RAW : QwordType_e::DISK_PLAIN_RAW;
	return bInline ? QwordType_e::DISK_INLINE_BLOCK : QwordType_e::DISK_PLAIN_BLOCK;
}


template <bool INLINE_HITS, typename READER>
static ISphQword * SpawnDiskQword ( const XQKeyword_t & tWord, DataReaderFactory_c * pDoclist, DataReaderFactory_c * pHitlist, int64_t iIndexId )
{
	if ( tWord.m_pPayload )
		return new DiskPayloadQword_c<INLINE_HITS,READER> ( (const DiskSubstringPayload_t *)tWord.m_pPayload, tWord.m_bExcluded, pDoclist, pHitlist, iIndexId );

	return new DiskIndexQword_c<INLINE_HITS,false,READER> ( tWord.m_bExpanded, tWord.m_bExcluded, iIndexId );
}


ISphQword * DiskIndexQwordSetup_c::QwordSpawn ( const XQKeyword_t & tWord ) const
{
	if ( tWord.m_pPayload && m_bSetupReaders )
		PrefetchPayload ( *(const DiskSubstringPayload_t *)tWord.m_pPayload );

	const int64_t iIndexId = m_pIndex->GetIndexId();
	switch ( GetQwordType() )
	{
	case QwordType_e::DISK_INLINE_RAW:   return SpawnDiskQword<true,RawReader_t> ( tWord, m_pDoclist, m_pHitlist, iIndexId );
	case QwordType_e::DISK_PLAIN_RAW:    return SpawnDiskQword<false,RawReader_t> ( tWord, m_pDoclist, m_pHitlist, iIndexId );
	case QwordType_e::DISK_INLINE_BLOCK: return SpawnDiskQword<true,BlockReader_t> ( tWord, m_pDoclist, m_pHitlist, iIndexId );
	case QwordType_e::DISK_PLAIN_BLOCK:  return SpawnDiskQword<false,BlockReader_t> ( tWord, m_pDoclist, m_pHitlist, iIndexId );
	default:
		assert ( 0 && "Unexpected disk qword type" );
		return nullptr;
	}
}
