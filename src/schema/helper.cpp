//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "helper.h"

#include "attribute.h"
#include "match.h"


void CSphSchemaHelper::InsertAttr ( CSphVector<CSphColumnInfo>& dAttrs, CSphVector<int>& dUsed, int iPos, const CSphColumnInfo& tCol, bool bDynamic )
{
	assert ( 0 <= iPos && iPos <= dAttrs.GetLength() );
	assert ( tCol.m_eAttrType != SPH_ATTR_NONE );
	if ( tCol.m_eAttrType == SPH_ATTR_NONE )
		return;

	dAttrs.Insert ( iPos, tCol );
	CSphAttrLocator& tLoc = dAttrs[iPos].m_tLocator;

	int iBits = ROWITEM_BITS;
	if ( tLoc.m_iBitCount > 0 )
		iBits = tLoc.m_iBitCount;
	if ( tCol.m_eAttrType == SPH_ATTR_BOOL )
		iBits = 1;
	if ( tCol.m_eAttrType == SPH_ATTR_BIGINT || tCol.m_eAttrType == SPH_ATTR_JSON_FIELD || tCol.m_eAttrType==SPH_ATTR_DOUBLE )
		iBits = 64;

	if ( tCol.IsDataPtr() )
	{
		assert ( bDynamic );
		iBits = ROWITEMPTR_BITS;
		m_dDataPtrAttrs.Add ( dUsed.GetLength() );
	}

	tLoc.m_iBitCount = iBits;
	tLoc.m_bDynamic = bDynamic;

	if ( tCol.IsColumnar() )
	{
		// can't have an attribute that is stored in columnar and common storage at the same time
		// FIXME: maybe add extra flags here?
		tLoc.m_iBitOffset = 0;
	} else if ( sphIsBlobAttr ( tCol ) )
	{
		bool bColumnarID = !dAttrs.IsEmpty() && dAttrs[0].m_sName == sphGetDocidName() && dAttrs[0].IsColumnar();

		tLoc.m_iBitCount = 0;
		tLoc.m_iBitOffset = 0;

		int iBlobAttrId = 0;
		for ( auto& i : dAttrs )
			if ( sphIsBlobAttr ( i ) )
				i.m_tLocator.m_iBlobAttrId = iBlobAttrId++;

		for ( auto& i : dAttrs )
			if ( sphIsBlobAttr ( i ) )
			{
				i.m_tLocator.m_nBlobAttrs = iBlobAttrId;
				i.m_tLocator.m_iBlobRowOffset = bColumnarID ? 0 : 1;
			}
	} else if ( iBits >= ROWITEM_BITS )
	{
		tLoc.m_iBitOffset = dUsed.GetLength() * ROWITEM_BITS;
		int iItems = ( iBits + ROWITEM_BITS - 1 ) / ROWITEM_BITS;
		for ( int i = 0; i < iItems; i++ )
			dUsed.Add ( ROWITEM_BITS );
	} else
	{
		int iItem;
		for ( iItem = 0; iItem < dUsed.GetLength(); iItem++ )
			if ( dUsed[iItem] + iBits <= ROWITEM_BITS )
				break;
		if ( iItem == dUsed.GetLength() )
			dUsed.Add ( 0 );
		tLoc.m_iBitOffset = iItem * ROWITEM_BITS + dUsed[iItem];
		dUsed[iItem] += iBits;
	}
}

void CSphSchemaHelper::Swap ( CSphSchemaHelper& rhs ) noexcept
{
	rhs.m_dDataPtrAttrs.SwapData ( m_dDataPtrAttrs );
	rhs.m_dDynamicUsed.SwapData ( m_dDynamicUsed );
}

void CSphSchemaHelper::ResetSchemaHelper()
{
	m_dDataPtrAttrs.Reset();
	m_dDynamicUsed.Reset();
}

void CSphSchemaHelper::CloneMatch ( CSphMatch& tDst, const CSphMatch& rhs ) const
{
	CloneMatchSpecial ( tDst, rhs, m_dDataPtrAttrs );
}

void CSphSchemaHelper::FreeDataPtrs ( CSphMatch& tMatch ) const
{
	FreeDataSpecial ( tMatch, m_dDataPtrAttrs );
}

void CSphSchemaHelper::CopyPtrs ( CSphMatch& tDst, const CSphMatch& rhs ) const
{
	// notes on PACKEDFACTORS
	// not immediately obvious: this is not needed while pushing matches to sorters; factors are held in an outer hash table
	// but it is necessary to copy factors when combining results from several indexes via a sorter because at this moment matches are the owners of factor data
	CopyPtrsSpecial ( tDst, rhs, m_dDataPtrAttrs );
}

CSphVector<int> CSphSchemaHelper::SubsetPtrs ( CSphVector<int>& dDiscarded ) const
{
	CSphVector<int> dFiltered;
	dDiscarded.Uniq();
	for ( int iPtr : m_dDataPtrAttrs )
		if ( !dDiscarded.BinarySearch ( iPtr ) )
			dFiltered.Add ( iPtr );
	dFiltered.Uniq();
	return dFiltered;
}

// in destination free listed attrs, then copy new from rhs
void CSphSchemaHelper::CloneMatchSpecial ( CSphMatch& tDst, const CSphMatch& rhs, const VecTraits_T<int>& dSpecials ) const
{
	FreeDataSpecial ( tDst, dSpecials );
	tDst.Combine ( rhs, GetDynamicSize() );
	for ( auto i : m_dDataPtrAttrs )
		*(BYTE**)( tDst.m_pDynamic + i ) = nullptr;
	CopyPtrsSpecial ( tDst, rhs, dSpecials );
}

// declared in sphinxstd.h
void sphDeallocatePacked ( BYTE* pBlob )
{
	if ( !pBlob )
		return;
#if WITH_SMALLALLOC
	const BYTE * pFoo = pBlob;
	sphDeallocateSmall ( pBlob, sphCalcPackedLength ( UnzipIntBE ( pFoo ) ) );
#else
	sphDeallocateSmall ( pBlob );
#endif
}

// fixme! direct reinterpreting rows is not good idea. Use sphGetAttr/sphSetAttr!
/*
 * wide (64bit) attributes occupies 2 rows and placed order lo,high
 * On LE arch (intel) it is ok to reinterpret them back as 64-bit pointer
 * However on BE (mips) you have problems since such cast gives garbage.
 */
void CSphSchemaHelper::FreeDataSpecial ( CSphMatch& tMatch, const VecTraits_T<int>& dSpecials )
{
	if ( !tMatch.m_pDynamic )
		return;

	for ( auto iOffset : dSpecials )
	{
		BYTE*& pData = *(BYTE**)( tMatch.m_pDynamic + iOffset );
		sphDeallocatePacked ( pData );
		pData = nullptr;
	}
}

void CSphSchemaHelper::CopyPtrsSpecial ( CSphMatch& tDst, const CSphMatch& tSrc, const VecTraits_T<int>& dSpecials )
{
	auto pSrc = tSrc.m_pDynamic;
	assert ( pSrc || dSpecials.IsEmpty() );
	for ( auto i : dSpecials )
	{
		const BYTE* pData = *(BYTE**)( pSrc + i );
		if ( pData )
			*(BYTE**)( tDst.m_pDynamic + i ) = sph::CopyPackedAttr ( pData );
	}
}

void CSphSchemaHelper::MovePtrsSpecial ( CSphMatch& tDst, CSphMatch& tSrc, const VecTraits_T<int>& dSpecials )
{
	auto pSrc = tSrc.m_pDynamic;
	assert ( pSrc || dSpecials.IsEmpty() );
	for ( auto i : dSpecials ) {
		memcpy ( tDst.m_pDynamic + i, pSrc + i, sizeof ( BYTE* ) );
		*(BYTE**)( pSrc + i ) = nullptr;
	}
}
