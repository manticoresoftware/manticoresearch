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

#include "std/fnv64.h"

inline CSphAttrLocator::CSphAttrLocator ( int iBitOffset, int iBitCount )
	: m_iBitOffset ( iBitOffset )
	, m_iBitCount ( iBitCount )
{}

inline bool CSphAttrLocator::IsBitfield() const
{
	return ( m_iBitCount < ROWITEM_BITS || ( m_iBitOffset % ROWITEM_BITS ) != 0 );
}

inline int CSphAttrLocator::CalcRowitem() const
{
	return IsBitfield() ? -1 : ( m_iBitOffset / ROWITEM_BITS );
}

inline bool CSphAttrLocator::IsBlobAttr() const
{
	return m_iBlobAttrId >= 0;
}

inline void CSphAttrLocator::Reset()
{
	m_iBitOffset = -1;
	m_iBitCount = -1;
	m_iBlobAttrId = -1;
	m_iBlobRowOffset = 1;
	m_nBlobAttrs = 0;
	m_bDynamic = true;
}

// just sphFNV64 (&loc,sizeof(loc)) isn't correct as members are not aligned
inline uint64_t CSphAttrLocator::FNV ( uint64_t uHash ) const
{
	BYTE sBuf[sizeof ( CSphAttrLocator )];
	auto* p = sBuf;
	sphUnalignedWrite ( p, m_iBitOffset );
	p += sizeof ( m_iBitOffset );
	sphUnalignedWrite ( p, m_iBitCount );
	p += sizeof ( m_iBitCount );
	sphUnalignedWrite ( p, m_iBlobAttrId );
	p += sizeof ( m_iBlobAttrId );
	sphUnalignedWrite ( p, m_iBlobRowOffset );
	p += sizeof ( m_iBlobRowOffset );
	sphUnalignedWrite ( p, m_nBlobAttrs );
	p += sizeof ( m_nBlobAttrs );
	sphUnalignedWrite ( p, m_bDynamic );
	p += sizeof ( m_bDynamic );

	return sphFNV64 ( sBuf, (int)( p - sBuf ), uHash );
}


#ifndef NDEBUG
/// get last item touched by this attr (for debugging checks only)
inline int CSphAttrLocator::GetMaxRowitem() const
{
	return ( m_iBitOffset + m_iBitCount - 1 ) / ROWITEM_BITS;
}
#endif

inline bool CSphAttrLocator::operator== ( const CSphAttrLocator& rhs ) const
{
	if ( m_iBlobAttrId == -1 )
		return m_bDynamic == rhs.m_bDynamic && m_iBitOffset == rhs.m_iBitOffset && m_iBitCount == rhs.m_iBitCount;
	else
		return m_bDynamic == rhs.m_bDynamic && m_iBlobAttrId == rhs.m_iBlobAttrId && m_nBlobAttrs == rhs.m_nBlobAttrs;
}

FORCE_INLINE SphAttr_t sphGetRowAttr ( const CSphRowitem * pRow, const CSphAttrLocator & tLoc )
{
	assert(pRow);
	assert ( tLoc.m_iBitCount );

	int iItem = tLoc.m_iBitOffset >> ROWITEM_SHIFT;

	switch ( tLoc.m_iBitCount )
	{
	case ROWITEM_BITS:
		return SphAttr_t ( pRow[iItem] );
	case 2*ROWITEM_BITS:
#if USE_LITTLE_ENDIAN
		return *(SphAttr_t*) (const_cast<CSphRowitem*>(pRow) + iItem);
#else
		return SphAttr_t ( pRow[iItem] ) + ( SphAttr_t ( pRow[iItem+1] ) << ROWITEM_BITS ); break;
#endif
	default: break;
	}
	auto iShift = tLoc.m_iBitOffset & (( 1 << ROWITEM_SHIFT )-1 );
	return ( pRow[iItem] >> iShift ) & (( 1UL << tLoc.m_iBitCount )-1 );
}


FORCE_INLINE void sphSetRowAttr ( CSphRowitem * pRow, const CSphAttrLocator & tLoc, SphAttr_t uValue )
{
	assert(pRow);
	assert ( tLoc.m_iBitCount );

	int iItem = tLoc.m_iBitOffset >> ROWITEM_SHIFT;
	if ( tLoc.m_iBitCount==2*ROWITEM_BITS )
	{
		// FIXME? write a generalized version, perhaps
#if USE_LITTLE_ENDIAN
		memcpy( pRow+iItem, &uValue, ROWITEM_BITS/4 ); // actually it became 1 op in release asm
#else
		pRow[iItem] = CSphRowitem ( uValue & ( ( SphAttr_t(1) << ROWITEM_BITS )-1 ) );
		pRow[iItem+1] = CSphRowitem ( uValue >> ROWITEM_BITS );
#endif

	} else if ( tLoc.m_iBitCount==ROWITEM_BITS )
	{
		memcpy ( pRow+iItem, &uValue, ROWITEM_BITS / 8 );
	} else
	{
		int iShift = tLoc.m_iBitOffset & ( ( 1 << ROWITEM_SHIFT )-1);
		CSphRowitem uMask = ( ( 1UL << tLoc.m_iBitCount )-1 ) << iShift;
		pRow[iItem] &= ~uMask;
		pRow[iItem] |= ( uMask & ( uValue << iShift ) );
	}
}

/// add numeric value of another attribute
inline void sphAddCounterAttr ( CSphRowitem * pRow, const CSphRowitem * pVal, const CSphAttrLocator & tLoc )
{
	assert( pRow && pVal);
	int iItem = tLoc.m_iBitOffset >> ROWITEM_SHIFT;
	SphAttr_t uValue;

	switch (tLoc.m_iBitCount )
	{
	case ROWITEM_BITS:
		uValue = SphAttr_t ( pRow[iItem] ) + SphAttr_t ( pVal[iItem] );
		memcpy ( pRow+iItem, &uValue, ROWITEM_BITS / 8 );
		return;
	case 2*ROWITEM_BITS:
#if USE_LITTLE_ENDIAN
		uValue = *(SphAttr_t *) ( pRow+iItem ) +*(SphAttr_t *) ( const_cast<CSphRowitem*>(pVal)+iItem );
		memcpy ( pRow+iItem, &uValue, ROWITEM_BITS / 4 );
#else
		uValue = SphAttr_t ( pRow[iItem] ) + ( SphAttr_t ( pRow[iItem+1] ) << ROWITEM_BITS )
				+SphAttr_t ( pVal[iItem] ) + ( SphAttr_t ( pVal[iItem+1] ) << ROWITEM_BITS );
		pRow[iItem] = CSphRowitem ( uValue & ( ( SphAttr_t(1) << ROWITEM_BITS )-1 ) );
		pRow[iItem+1] = CSphRowitem ( uValue >> ROWITEM_BITS );
#endif
		return;
	default: break;
	}
	assert ( false && "Unable to add non-aligned attribute " );
}

/// add scalar value to aligned numeric attribute
inline void sphAddCounterScalar ( CSphRowitem * pRow, const CSphAttrLocator & tLoc, SphAttr_t uValue )
{
	assert( pRow );
	int iItem = tLoc.m_iBitOffset >> ROWITEM_SHIFT;

	switch (tLoc.m_iBitCount )
	{
	case ROWITEM_BITS:
		uValue += SphAttr_t ( pRow[iItem] );
		memcpy ( pRow+iItem, &uValue, ROWITEM_BITS / 8 );
		return;
	case 2*ROWITEM_BITS:
#if USE_LITTLE_ENDIAN
		uValue += *(SphAttr_t *) ( pRow+iItem );
		memcpy ( pRow+iItem, &uValue, ROWITEM_BITS / 4 );
#else
		uValue += SphAttr_t ( pRow[iItem] ) + ( SphAttr_t ( pRow[iItem+1] ) << ROWITEM_BITS );
		pRow[iItem] = CSphRowitem ( uValue & ( ( SphAttr_t(1) << ROWITEM_BITS )-1 ) );
		pRow[iItem+1] = CSphRowitem ( uValue >> ROWITEM_BITS );
#endif
		return;
	default: break;
	}
	assert ( false && "Unable to add non-aligned attribute " );
}
