//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "ischema.h"
#include "match.h"

/// helper class that is used by CSphSchema and CSphRsetSchema
class CSphSchemaHelper : public ISphSchema
{
public:
	void	FreeDataPtrs ( CSphMatch & tMatch ) const final;
	int		GetAttrIndexOriginal ( const char * szName ) const { return GetAttrIndex(szName); }
	void	CloneMatch ( CSphMatch & tDst, const CSphMatch & rhs ) const final;

	/// clone all raw attrs and only specified ptrs
	FORCE_INLINE void CloneMatchSpecial ( CSphMatch & tDst, const CSphMatch & rhs, const VecTraits_T<int> & dSpecials ) const;

	/// exclude vec of rowitems from dataPtrAttrs and return diff back
	CSphVector<int> SubsetPtrs ( CSphVector<int> &dSpecials ) const ;

	/// get dynamic row part size
	int		GetDynamicSize () const final { return m_dDynamicUsed.GetLength (); }

	void	Swap ( CSphSchemaHelper& rhs ) noexcept;

	// free/copy by specified vec of rowitems, assumed to be from SubsetPtrs() call.
	static FORCE_INLINE void FreeDataSpecial ( CSphMatch & tMatch, const VecTraits_T<int> & dSpecials );
	static FORCE_INLINE void CopyPtrsSpecial ( CSphMatch & tDst, const CSphMatch & tSrc, const VecTraits_T<int> & dSpecials );
	static void MovePtrsSpecial ( CSphMatch & tDst, CSphMatch & tSrc, const VecTraits_T<int> & dSpecials );

protected:
	CSphVector<int>	m_dDataPtrAttrs;		///< rowitems of pointers to data that are stored inside matches
	CSphVector<int> m_dDynamicUsed;			///< dynamic row part map

	/// generic InsertAttr() implementation that tracks data ptr attributes
	void	InsertAttr ( CSphVector<CSphColumnInfo> & dAttrs, CSphVector<int> & dUsed, int iPos, const CSphColumnInfo & tCol, bool bDynamic );
	void	ResetSchemaHelper();

	void	CopyPtrs ( CSphMatch & tDst, const CSphMatch & rhs ) const;
};


void CSphSchemaHelper::CloneMatchSpecial ( CSphMatch & tDst, const CSphMatch & rhs, const VecTraits_T<int> & dSpecials ) const
{
	FreeDataSpecial ( tDst, dSpecials );
	tDst.Combine ( rhs, GetDynamicSize() );
	for ( auto i : m_dDataPtrAttrs )
		*(BYTE**)( tDst.m_pDynamic + i ) = nullptr;
	CopyPtrsSpecial ( tDst, rhs, dSpecials );
}

void CSphSchemaHelper::FreeDataSpecial ( CSphMatch & tMatch, const VecTraits_T<int> & dSpecials )
{
	if ( !tMatch.m_pDynamic )
		return;

	for ( auto iOffset : dSpecials )
	{
		const SmallAttrLocator_t tLoc { iOffset*ROWITEM_BITS, ROWITEMPTR_BITS };
		const auto * pData = (const BYTE *) sphGetRowAttr ( tMatch.m_pDynamic, tLoc );
		sphDeallocatePacked ( pData );
		sphSetRowAttr ( tMatch.m_pDynamic, tLoc, (SphAttr_t) nullptr );
	}
}


void CSphSchemaHelper::CopyPtrsSpecial ( CSphMatch & tDst, const CSphMatch & tSrc, const VecTraits_T<int> & dSpecials )
{
	auto pSrc = tSrc.m_pDynamic;
	assert ( pSrc || dSpecials.IsEmpty() );
	for ( auto i : dSpecials )
	{
		const SmallAttrLocator_t tLoc { i*ROWITEM_BITS, ROWITEMPTR_BITS };
		const auto* pData = (const BYTE*) sphGetRowAttr ( pSrc, tLoc );
		if ( pData )
			sphSetRowAttr ( tDst.m_pDynamic, tLoc, (SphAttr_t) sphCopyPackedAttr ( pData ) );
	}
}
