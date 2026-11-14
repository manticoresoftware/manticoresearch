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
struct AVX512Traits_t
{
	using Vec_t = __m512i;
	static constexpr size_t WORDS = 16;
	static Vec_t Zero() { return _mm512_setzero_si512(); }
	static Vec_t Load ( const DWORD * p ) { return _mm512_loadu_si512(p); }
	static void Store ( DWORD * p, Vec_t u ) { _mm512_storeu_si512 ( p, u ); }
	template<Op_e OP> static Vec_t Apply ( Vec_t a, Vec_t b )
	{
		if constexpr ( OP==Op_e::AND ) return _mm512_and_si512 ( a, b );
		if constexpr ( OP==Op_e::OR ) return _mm512_or_si512 ( a, b );
		if constexpr ( OP==Op_e::XOR ) return _mm512_xor_si512 ( a, b );
		if constexpr ( OP==Op_e::AND_NOT ) return _mm512_andnot_si512 ( b, a );
		return a;
	}
	static Vec_t Count ( Vec_t u )
	{
		return _mm512_popcnt_epi64(u);
	}
	static Vec_t Add ( Vec_t a, Vec_t b ) { return _mm512_add_epi64 ( a, b ); }
	static uint64_t Sum ( Vec_t u )
	{
		uint64_t dLanes[8];
		std::memcpy ( dLanes, &u, sizeof(u) );
		uint64_t uSum = 0;
		for ( auto uLane : dLanes ) uSum += uLane;
		return uSum;
	}
};
}

const bitmap::Operations_t bitmap::detail::g_tAVX512 = MakeOperations<AVX512Traits_t> ( bitmap::Backend_e::AVX512, "avx512" );
