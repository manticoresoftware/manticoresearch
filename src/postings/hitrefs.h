// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include "shortlist.h"
#include "../std/bitmap/bitmap_count.h"

namespace postings
{

// Decode a packed 32-bit hit from up to five canonical LEB128 bytes.
template<bool CHECKED>
bool InlineHitValue ( Bytes_t & tData, uint32_t & uValue, std::string & sError )
{
	uValue = 0;
	for ( unsigned i=0; i<5; ++i )
	{
		if ( !ShortCheck<CHECKED> ( tData.m_iSize!=0, sError ) )
			return false;

		uint8_t uByte = *tData.m_pData++;
		--tData.m_iSize;
		if ( !ShortCheck<CHECKED> ( i!=4 || uByte<=15, sError ) )
			return false;

		uValue |= uint32_t(uByte&127)<<(7*i);
		if ( !(uByte&128) )
			return ShortCheck<CHECKED> ( !i || uByte!=0, sError );
	}

	return ShortCheck<CHECKED> ( false, sError );
}

// Split a mixed section into its selection bitmap, inline values and external references.
// Count the inline postings so each compact array can be decoded independently.
template<bool CHECKED>
bool ReadMixedHitRefsHeader ( Bytes_t & tData, unsigned uCount, const uint8_t * & pBitmap, unsigned & uInline, Bytes_t & tHits, std::string & sError )
{
	unsigned uBytes = (uCount+7)/8;
	if ( !ShortCheck<CHECKED> ( tData.m_iSize>=uBytes+2, sError ) )
		return false;

	pBitmap = tData.m_pData;
	unsigned uHitBytes = pBitmap[uBytes] | (unsigned(pBitmap[uBytes+1])<<8);
	uInline = unsigned(bitmap::CountBytes ( pBitmap, uBytes ));

	if ( !ShortCheck<CHECKED> ( uInline && uInline<uCount && (!(uCount%8) || !(pBitmap[uBytes-1]>>(uCount%8))) && uHitBytes<=tData.m_iSize-uBytes-2, sError ) )
		return false;

	tHits = { pBitmap+uBytes+2, uHitBytes };
	tData.m_pData += uBytes+2+uHitBytes;
	tData.m_iSize -= uBytes+2+uHitBytes;
	return true;
}

// Decode absolute .spp offsets from short-list varints or a base plus PFOR frames.
template<bool CHECKED, typename FRAME>
bool ReadExternalHitRefs ( Bytes_t & tData, unsigned uCount, bool bShort, uint64_t * pExternal, FRAME fnFrame, std::string & sError )
{
	std::fill_n ( pExternal, uCount, 0 );
	if ( !uCount )
		return true;

	if ( bShort )
	{
		if ( !ShortHitRefs<CHECKED> ( tData, uCount, pExternal, sError ) )
			return false;

		tData.m_iSize = 0;
		return true;
	}

	if ( !ShortCheck<CHECKED> ( tData.m_iSize>=9 && tData.m_pData[8]<=1, sError ) )
		return false;

	uint64_t uBase = 0;
	for ( unsigned i=0; i<8; ++i )
		uBase |= uint64_t(tData.m_pData[i])<<(8*i);

	unsigned uParts = 1+tData.m_pData[8];
	tData.m_pData += 9;
	tData.m_iSize -= 9;
	if ( !ShortCheck<CHECKED> ( uBase && uBase<=INT64_MAX, sError ) )
		return false;

	alignas(16) std::array<uint32_t,MAX_VALUES> dValues;
	for ( unsigned part=0; part<uParts; ++part )
	{
		if ( !fnFrame ( StreamDesc_t{Codec_e::PFOR}, tData, dValues.data(), uCount, sError ) )
			return false;

		for ( unsigned i=0; i<uCount; ++i )
			pExternal[i] |= uint64_t(dValues[i])<<(32*part);
	}

	for ( unsigned i=0; i<uCount; ++i )
	{
		if ( !ShortCheck<CHECKED> ( pExternal[i]<=uint64_t(INT64_MAX)-uBase, sError ) )
			return false;

		pExternal[i] += uBase;
	}

	return true;
}

// Restore posting order, tagging inline hits and interleaving decoded external offsets.
// All-inline sections fill every output entry directly from the inline values.
template<bool CHECKED>
bool MergeInlineHitRefs ( Bytes_t & tHits, HitRefMode_e eMode, const uint8_t * pBitmap, unsigned uCount, const uint64_t * pExternal, uint64_t * pRefs, std::string & sError )
{
	unsigned uNext = 0;
	for ( unsigned i=0; i<uCount; ++i )
	{
		if ( eMode==HitRefMode_e::ALL_INLINE || (pBitmap && (pBitmap[i/8] & (1U<<(i%8)))) )
		{
			if ( !ShortCheck<CHECKED> ( tHits.m_iSize!=0, sError ) )
				return false;

			uint32_t uHit = *tHits.m_pData;
			if ( uHit<128 )
			{
				++tHits.m_pData;
				--tHits.m_iSize;
			} else if ( !InlineHitValue<CHECKED> ( tHits, uHit, sError ) )
				return false;

			if ( !ShortCheck<CHECKED> ( uHit!=0, sError ) )
				return false;

			pRefs[i] = INLINE_HIT | uHit;
		} else
			pRefs[i] = pExternal[uNext++];
	}

	return ShortCheck<CHECKED> ( !tHits.m_iSize, sError );
}

// Read inline positions and external bookmarks without reading the TF column.
template<bool CHECKED, typename FRAME>
bool ReadInlineHitRefs ( Bytes_t tData, unsigned uCount, bool bShort, uint64_t * pRefs, FRAME fnFrame, std::string & sError )
{
	if ( !ShortCheck<CHECKED> ( tData.m_iSize && tData.m_pData[0]<=uint8_t(HitRefMode_e::MIXED), sError ) )
		return false;

	auto eMode = HitRefMode_e(*tData.m_pData++);
	--tData.m_iSize;

	const uint8_t * pBitmap = nullptr;
	unsigned uInline = eMode==HitRefMode_e::ALL_INLINE ? uCount : 0;
	Bytes_t tHits;
	if ( eMode==HitRefMode_e::MIXED )
	{
		if ( !ReadMixedHitRefsHeader<CHECKED> ( tData, uCount, pBitmap, uInline, tHits, sError ) )
			return false;
	} else if ( eMode==HitRefMode_e::ALL_INLINE )
	{
		tHits = tData;
		tData.m_iSize = 0;
	}

	// External-only sections decode directly into the output; mixed sections
	// need a compact temporary array before restoring posting order.
	std::array<uint64_t,MAX_VALUES> dExternal;
	uint64_t * pExternal = eMode==HitRefMode_e::ALL_EXTERNAL ? pRefs : dExternal.data();
	if ( !ReadExternalHitRefs<CHECKED> ( tData, uCount-uInline, bShort, pExternal, fnFrame, sError ) )
		return false;

	if ( uInline && !MergeInlineHitRefs<CHECKED> ( tHits, eMode, pBitmap, uCount, pExternal, pRefs, sError ) )
		return false;

	return ShortCheck<CHECKED> ( !tData.m_iSize, sError );
}

}
