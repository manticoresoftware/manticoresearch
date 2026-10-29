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


DiskIndexQwordTraits_c * sphCreateDiskIndexQword ( bool bInlineHits )
{
	if ( bInlineHits )
		return new DiskIndexQword_c<true,false,BlockReader_t> ( false, false, 0 );

	return new DiskIndexQword_c<false,false,BlockReader_t> ( false, false, 0 );
}


ISphQword * DiskIndexQwordSetup_c::QwordSpawn ( const XQKeyword_t & tWord ) const
{
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
