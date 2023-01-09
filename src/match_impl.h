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

#pragma once

#include "conversion.h"
#include "attribute.h"

/// dtor. frees everything
inline CSphMatch::~CSphMatch()
{
	ResetDynamic();
}

/// reset
inline void CSphMatch::Reset ( int iDynamic )
{
	// check that we're either initializing a new one, or NOT changing the current size
	assert ( iDynamic >= 0 );
	assert ( !m_pDynamic || iDynamic == (int)m_pDynamic[-1] );

	m_tRowID = INVALID_ROWID;
	if ( !m_pDynamic && iDynamic )
	{
#ifndef NDEBUG
		m_pDynamic = new CSphRowitem[iDynamic + 1];
		*m_pDynamic++ = iDynamic;
#else
		m_pDynamic = new CSphRowitem[iDynamic];
#endif
		// dynamic stuff might contain pointers now (STRINGPTR type)
		// so we gotta cleanup
		memset ( m_pDynamic, 0, iDynamic * sizeof ( CSphRowitem ) );
	}
}

// if mem region reinterpreted as match - ensure d-tr will not do sad things as delete garbaged m_pDynamic
inline void CSphMatch::CleanGarbage()
{
	m_pDynamic = nullptr;
}

inline void CSphMatch::ResetDynamic()
{
#ifndef NDEBUG
	if ( m_pDynamic )
		--m_pDynamic;
#endif
	SafeDeleteArray ( m_pDynamic );
}

/// assignment
inline void CSphMatch::Combine ( const CSphMatch& rhs, int iDynamic )
{
	// check that we're either initializing a new one, or NOT changing the current size
	assert ( iDynamic >= 0 );
	assert ( !m_pDynamic || iDynamic == (int)m_pDynamic[-1] );

	if ( this != &rhs )
	{
		m_tRowID = rhs.m_tRowID;
		m_iWeight = rhs.m_iWeight;
		m_pStatic = rhs.m_pStatic;
		m_iTag = rhs.m_iTag;
	}

	if ( iDynamic )
	{
		if ( !m_pDynamic )
		{
#ifndef NDEBUG
			m_pDynamic = new CSphRowitem[iDynamic + 1];
			*m_pDynamic++ = iDynamic;
#else
			m_pDynamic = new CSphRowitem[iDynamic];
#endif
		}

		if ( this != &rhs )
		{
			assert ( rhs.m_pDynamic );
			assert ( m_pDynamic[-1] == rhs.m_pDynamic[-1] ); // ensure we're not changing X to Y
			memcpy ( m_pDynamic, rhs.m_pDynamic, iDynamic * sizeof ( CSphRowitem ) );
		}
	}
}

/// integer getter
FORCE_INLINE SphAttr_t CSphMatch::GetAttr ( const CSphAttrLocator& tLoc ) const
{
	// m_pRowpart[tLoc.m_bDynamic] is 30% faster on MSVC 2005
	// same time on gcc 4.x though, ~1 msec per 1M calls, so lets avoid the hassle for now
	if ( tLoc.m_iBitOffset >= 0 )
		return sphGetRowAttr ( tLoc.m_bDynamic ? m_pDynamic : m_pStatic, tLoc );
	assert ( false && "Unknown negative-bitoffset locator" );
	return 0;
}

/// integer setter
FORCE_INLINE void CSphMatch::SetAttr ( const CSphAttrLocator& tLoc, SphAttr_t uValue ) const
{
	assert ( tLoc.m_bDynamic );
	assert ( tLoc.GetMaxRowitem() < (int)m_pDynamic[-1] );
	sphSetRowAttr ( m_pDynamic, tLoc, uValue );
}

FORCE_INLINE SphAttr_t ExchangeAttr ( const CSphMatch& tMatch, const CSphAttrLocator& tLoc, SphAttr_t uNewValue )
{
	auto uOldValue = tMatch.GetAttr ( tLoc );
	tMatch.SetAttr ( tLoc, uNewValue );
	return uOldValue;
}

/// add scalar value to attribute
inline void CSphMatch::AddCounterScalar ( const CSphAttrLocator& tLoc, SphAttr_t uValue ) const
{
	assert ( tLoc.m_bDynamic );
	assert ( tLoc.GetMaxRowitem() < (int)m_pDynamic[-1] );
	sphAddCounterScalar ( m_pDynamic, tLoc, uValue );
}

/// add same-located value from another match
inline void CSphMatch::AddCounterAttr ( const CSphAttrLocator& tLoc, const CSphMatch& tValue ) const
{
	assert ( tLoc.m_bDynamic );
	assert ( tLoc.GetMaxRowitem() < (int)m_pDynamic[-1] );
	sphAddCounterAttr ( m_pDynamic, tLoc.m_bDynamic ? tValue.m_pDynamic : tValue.m_pStatic, tLoc );
}


inline float CSphMatch::GetAttrFloat ( const CSphAttrLocator& tLoc ) const
{
	return sphDW2F ( (DWORD)sphGetRowAttr ( tLoc.m_bDynamic ? m_pDynamic : m_pStatic, tLoc ) );
}


inline double CSphMatch::GetAttrDouble ( const CSphAttrLocator & tLoc ) const
{
	return sphQW2D ( sphGetRowAttr ( tLoc.m_bDynamic ? m_pDynamic : m_pStatic, tLoc ) );
}


inline void CSphMatch::SetAttrFloat ( const CSphAttrLocator& tLoc, float fValue ) const
{
	assert ( tLoc.m_bDynamic );
	assert ( tLoc.GetMaxRowitem() < (int)m_pDynamic[-1] );
	sphSetRowAttr ( m_pDynamic, tLoc, sphF2DW ( fValue ) );
}

inline void CSphMatch::SetAttrDouble ( const CSphAttrLocator & tLoc, double fValue ) const
{
	assert ( tLoc.m_bDynamic );
	assert ( tLoc.GetMaxRowitem() < (int)m_pDynamic[-1] );
	sphSetRowAttr ( m_pDynamic, tLoc, sphD2QW ( fValue ) );
}

/// fetches blobs from both data ptr attrs and pooled blob attrs
inline ByteBlob_t CSphMatch::FetchAttrData ( const CSphAttrLocator& tLoc, const BYTE* pPool ) const
{
	if ( tLoc.IsBlobAttr() )
		return sphGetBlobAttr ( *this, tLoc, pPool );
	return sphUnpackPtrAttr ( (const BYTE*)GetAttr ( tLoc ) );
}

/// specialized swapper
inline void Swap ( CSphMatch& a, CSphMatch& b )
{
	if ( &a == &b )
		return;

	Swap ( a.m_tRowID, b.m_tRowID );
	Swap ( a.m_pStatic, b.m_pStatic );
	Swap ( a.m_pDynamic, b.m_pDynamic );
	Swap ( a.m_iWeight, b.m_iWeight );
	Swap ( a.m_iTag, b.m_iTag );
}
