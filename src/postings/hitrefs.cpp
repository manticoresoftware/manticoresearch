// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "hitrefs.h"

namespace postings
{
namespace
{

struct ByteMap_t
{
	std::array<uint8_t,8> m_dOffset {};
	std::array<uint8_t,8> m_dInlineMask {};
	unsigned int m_uInline = 0;
};


constexpr std::array<ByteMap_t,256> BuildHitRefOrdinalMaps()
{
	std::array<ByteMap_t,256> dMaps {};
	for ( unsigned int u=0; u<256; ++u )
		for ( unsigned int i=0; i<8; ++i )
		{
			bool bInline = (u & (1U<<i))!=0;
			dMaps[u].m_dInlineMask[i] = bInline ? 255 : 0;
			dMaps[u].m_dOffset[i] = uint8_t(bInline ? dMaps[u].m_uInline : i-dMaps[u].m_uInline);
			dMaps[u].m_uInline += bInline;
		}

	return dMaps;
}

}


void HitRefView_c::UseDecodedRefs ( const uint64_t * pRefs )
{
	m_pRefs = pRefs;
	m_pIndices = nullptr;
	m_pInlineBytes = nullptr;
}


void HitRefView_c::UseInlineBytes ( const uint8_t * pHits )
{
	m_pRefs = nullptr;
	m_pIndices = nullptr;
	m_pInlineBytes = pHits;
}


uint64_t * HitRefView_c::PrepareDecodedRefs()
{
	UseDecodedRefs ( m_dValues.data() );
	return m_dValues.data();
}


// Each bitmap byte routes eight posting ordinals to offsets in the two compact arrays.
uint64_t * HitRefView_c::PrepareMixedRefs ( const uint8_t * pBitmap, unsigned int uCount, unsigned int uNumExternal )
{
	static constexpr auto dMaps = BuildHitRefOrdinalMaps();

	unsigned int iInline = uNumExternal;
	unsigned int iExternal = 0;
	for ( unsigned int i=0; i<uCount; i+=8 )
	{
		const auto & t = dMaps[pBitmap[i/8]];
		for ( unsigned int j=0; j<8; ++j )
		{
			unsigned int uBase = iExternal ^ ((iExternal ^ iInline) & t.m_dInlineMask[j]);
			m_dIndices[i+j] = uint8_t(uBase+t.m_dOffset[j]);
		}

		iInline += t.m_uInline;
		iExternal += 8-t.m_uInline;
	}

	m_pRefs = m_dValues.data();
	m_pIndices = m_dIndices.data();
	m_pInlineBytes = nullptr;
	return m_dValues.data();
}

}
