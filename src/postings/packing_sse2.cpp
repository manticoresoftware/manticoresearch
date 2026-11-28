// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

#include "packing.h"
#include "fastpfor/simdbitpacking.h"

namespace postings
{

static void Pack ( const uint32_t * pIn, uint32_t * pOut, unsigned uWidth )
{
	FastPForLib::SIMD_fastpack_32 ( pIn, reinterpret_cast<__m128i *>(pOut), uWidth );
}

static void Unpack ( const uint32_t * pIn, uint32_t * pOut, unsigned uWidth )
{
	FastPForLib::SIMD_fastunpack_32 ( reinterpret_cast<const __m128i *>(pIn), pOut, uWidth );
}

extern const Packing_t g_tSse2 { Pack, Unpack, "fastpfor-sse2" };

}
