//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "bitmap_backend.h"
#include "bitmap_kernels.h"
#include <immintrin.h>

namespace
{
struct SSSE3Traits_t
{
	using Vec_t = __m128i;
	static constexpr size_t WORDS = 4;
	static Vec_t Zero() { return _mm_setzero_si128(); }
	static Vec_t Load ( const DWORD * p ) { return _mm_loadu_si128 ( reinterpret_cast<const Vec_t *>(p) ); }
	static void Store ( DWORD * p, Vec_t u ) { _mm_storeu_si128 ( reinterpret_cast<Vec_t *>(p), u ); }
	template<Op_e OP> static Vec_t Apply ( Vec_t a, Vec_t b )
	{
		if constexpr ( OP==Op_e::AND ) return _mm_and_si128 ( a, b );
		if constexpr ( OP==Op_e::OR ) return _mm_or_si128 ( a, b );
		if constexpr ( OP==Op_e::XOR ) return _mm_xor_si128 ( a, b );
		if constexpr ( OP==Op_e::AND_NOT ) return _mm_andnot_si128 ( b, a );
		return a;
	}
	static Vec_t Count ( Vec_t u )
	{
		const auto tMask = _mm_set1_epi8(15);
		const auto tLookup = _mm_setr_epi8 ( 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 );
		const auto tLow = _mm_shuffle_epi8 ( tLookup, _mm_and_si128 ( u, tMask ) );
		const auto tHigh = _mm_shuffle_epi8 ( tLookup, _mm_and_si128 ( _mm_srli_epi16 ( u, 4 ), tMask ) );
		// Reduce each vector immediately: byte accumulators would overflow on long inputs.
		return _mm_sad_epu8 ( _mm_add_epi8 ( tLow, tHigh ), Zero() );
	}
	static Vec_t Add ( Vec_t a, Vec_t b ) { return _mm_add_epi64 ( a, b ); }
	static uint64_t Sum ( Vec_t u )
	{
		uint64_t dLanes[2];
		std::memcpy ( dLanes, &u, sizeof(u) );
		uint64_t uSum = 0;
		for ( auto uLane : dLanes ) uSum += uLane;
		return uSum;
	}
};
}

const bitmap::Operations_t bitmap::detail::g_tSSSE3 = MakeOperations<SSSE3Traits_t> ( bitmap::Backend_e::SSSE3, "ssse3" );
