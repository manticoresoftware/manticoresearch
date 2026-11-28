// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "packing.h"

#include <algorithm>

namespace postings
{

static void PackScalar ( const uint32_t * pIn, uint32_t * pOut, unsigned uWidth )
{
	std::fill_n ( pOut, 4*uWidth, 0 );
	if ( !uWidth )
		return;

	const uint32_t uMask = UINT32_MAX >> (32-uWidth);
	for ( unsigned i=0; i<128; ++i )
	{
		const unsigned uBit = (i/4)*uWidth;
		const unsigned uWord = (uBit/32)*4+i%4;
		const unsigned uShift = uBit%32;
		const uint32_t uValue = pIn[i] & uMask;
		pOut[uWord] |= uValue<<uShift;
		if ( uShift+uWidth>32 )
			pOut[uWord+4] |= uValue>>(32-uShift);
	}
}

static void UnpackScalar ( const uint32_t * pIn, uint32_t * pOut, unsigned uWidth )
{
	if ( !uWidth )
	{
		std::fill_n ( pOut, 128, 0 );
		return;
	}

	const uint32_t uMask = UINT32_MAX >> (32-uWidth);
	for ( unsigned i=0; i<128; ++i )
	{
		const unsigned uBit = (i/4)*uWidth;
		const unsigned uWord = (uBit/32)*4+i%4;
		const unsigned uShift = uBit%32;
		uint32_t uValue = pIn[uWord]>>uShift;
		if ( uShift+uWidth>32 )
			uValue |= pIn[uWord+4]<<(32-uShift);

		pOut[i] = uValue & uMask;
	}
}

bool HasSimd()
{
#ifdef POSTINGS_HAVE_SSE2
	return true; // SSE2 is part of the x86-64 baseline; never built for 32-bit x86.
#else
	return false;
#endif
}

const Packing_t * GetPacking ( Backend_e eBackend )
{
	static const Packing_t tScalar { PackScalar, UnpackScalar, "scalar" };
	if ( eBackend==Backend_e::SCALAR )
		return &tScalar;

#ifdef POSTINGS_HAVE_SSE2
	if ( eBackend==Backend_e::AUTO || eBackend==Backend_e::SIMD )
		return &g_tSse2;
#else
	if ( eBackend==Backend_e::AUTO )
		return &tScalar;
#endif
	return nullptr;
}

const char * BackendName ( Backend_e eBackend )
{
	const auto * pPacking = GetPacking(eBackend);
	return pPacking ? pPacking->m_sName : "unavailable";
}

}
