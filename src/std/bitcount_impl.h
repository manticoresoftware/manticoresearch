//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "ints.h"

#if __GNUC__ || __clang__

inline int sphBitCount ( DWORD n ) noexcept
{
	return __builtin_popcount ( n );
}


inline int sphBitCount ( uint64_t n ) noexcept
{
	return __builtin_popcountll ( n );
}

#else

// MIT HACKMEM count
// works for 32-bit numbers only
// fix last line for 64-bit numbers
inline int sphBitCount ( DWORD n ) noexcept
{
	n -= ( n >> 1 ) & 0x55555555;
	const auto tmp = n & 0x33333333;
	n = ( ( n >> 2 ) & 0x33333333 ) + tmp;
	n = ( n + ( n >> 4 ) ) & 0x0F0F0F0F;
    n = n * 0x01010101;
    return n >> 24;
}

// MIT HACKMEM count without division (equals to __builtin_popcountll when no popcount op in cpu)
inline int sphBitCount ( uint64_t n ) noexcept
{
	n -= ( n >> 1 ) & 0x5555555555555555UL;
	const auto tmp = n & 0x3333333333333333UL;
	n = ( ( n >> 2 ) & 0x3333333333333333UL ) + tmp;
	n = ( n + ( n >> 4 ) ) & 0x0F0F0F0F0F0F0F0FUL;
    n = n * 0x0101010101010101UL;
    return n >> 56;
}
#endif

// that is the fastest variant of MIT HACKMEM count specified to single byte argument
// benches of different variants are available in gbenches/popcount
inline int sphBitCount ( BYTE n ) noexcept
{
	return (int) (((((DWORD ( n ) * 0x08040201) >> 3) & 0x11111111) * 0x11111111) >> 28);
}


// with LUT - just for benching
inline int sphBitCount_case ( BYTE n ) noexcept
{
	constexpr BYTE nbits[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
	return nbits [ n >> 4 ] + nbits [ n & 0x0F ];
}
