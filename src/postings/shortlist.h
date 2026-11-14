// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#pragma once

#include "stream.h"
#include <algorithm>
#include <cassert>

namespace postings
{

constexpr unsigned SHORT_LIST_MAX = 16;
constexpr size_t SHORT_HEADER_SIZE = 24;

// Trusted readers assert invariants; the checker rejects malformed short records.
template<bool CHECKED>
bool ShortCheck ( bool bValid, std::string & sError )
{
	if constexpr ( CHECKED )
	{
		if ( !bValid )
		{
			sError = "invalid short posting list";
			return false;
		}
	} else
		assert(bValid);

	return true;
}

template<bool CHECKED>
bool ShortValue ( Bytes_t & tData, uint64_t & uValue, std::string & sError )
{
	uValue = 0;
	for ( unsigned i=0; i<10; ++i )
	{
		if ( !ShortCheck<CHECKED> ( tData.m_iSize!=0, sError ) )
			return false;

		uint8_t uByte = *tData.m_pData++;
		--tData.m_iSize;
		if ( !ShortCheck<CHECKED> ( i!=9 || uByte<=1, sError ) )
			return false;

		uValue |= uint64_t(uByte&127)<<(7*i);
		if ( !(uByte&128) )
			return ShortCheck<CHECKED> ( !i || uByte!=0, sError );
	}

	return ShortCheck<CHECKED> ( false, sError );
}

template<bool CHECKED>
bool ShortRows ( Bytes_t tData, unsigned uCount, uint32_t * pRows, std::string & sError )
{
	uint64_t uRow = 0;
	for ( unsigned i=0; i<uCount; ++i )
	{
		uint64_t uDelta;
		if ( !ShortValue<CHECKED> ( tData, uDelta, sError ) || !ShortCheck<CHECKED> ( (!i || uDelta) && uDelta<UINT32_MAX-uRow, sError ) )
			return false;

		uRow += uDelta;
		pRows[i] = uint32_t(uRow);
	}

	return ShortCheck<CHECKED> ( !tData.m_iSize, sError );
}

template<bool CHECKED>
bool ShortIntegers ( Bytes_t tData, unsigned uCount, uint32_t * pValues, std::string & sError )
{
	for ( unsigned i=0; i<uCount; ++i )
	{
		uint64_t uValue;
		if ( !ShortValue<CHECKED> ( tData, uValue, sError ) || !ShortCheck<CHECKED> ( uValue<=UINT32_MAX, sError ) )
			return false;

		pValues[i] = uint32_t(uValue);
	}

	return ShortCheck<CHECKED> ( !tData.m_iSize, sError );
}

template<bool CHECKED>
bool ShortHitRefs ( Bytes_t tData, unsigned uCount, uint64_t * pRefs, std::string & sError )
{
	uint64_t uBase;
	if ( !ShortValue<CHECKED> ( tData, uBase, sError ) || !ShortCheck<CHECKED> ( uBase && uBase<=INT64_MAX, sError ) )
		return false;

	for ( unsigned i=0; i<uCount; ++i )
	{
		uint64_t uDelta = 0;
		if ( uCount>1 && !ShortValue<CHECKED> ( tData, uDelta, sError ) )
			return false;

		if ( !ShortCheck<CHECKED> ( uDelta<=uint64_t(INT64_MAX)-uBase, sError ) )
			return false;

		pRefs[i] = uBase+uDelta;
	}

	return ShortCheck<CHECKED> ( !tData.m_iSize, sError );
}

}
