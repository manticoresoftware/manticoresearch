//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

inline int sphBitCount ( DWORD n )
{
	return __builtin_popcount ( n );
}

inline int sphBitCount ( uint64_t n )
{
	return __builtin_popcountll ( n );
}

#else

// MIT HACKMEM count
// works for 32-bit numbers only
// fix last line for 64-bit numbers
inline int sphBitCount ( DWORD n )
{
	DWORD tmp;
	tmp = n - ( ( n >> 1 ) & 033333333333 ) - ( ( n >> 2 ) & 011111111111 );
	return ( ( tmp + ( tmp >> 3 ) ) & 030707070707 ) % 63;
}

// MIT HACKMEM count without division
inline int sphBitCount ( uint64_t n )
{
	unsigned long tmp = ( n >> 1 ) & 0x7777777777777777UL;
	n -= tmp;
	tmp = ( tmp >> 1 ) & 0x7777777777777777UL;
	n -= tmp;
	tmp = ( tmp >> 1 ) & 0x7777777777777777UL;
	n -= tmp;
	n = ( n + ( n >> 4 ) ) & 0x0F0F0F0F0F0F0F0FUL;
	n = n * 0x0101010101010101UL;
	return n >> 56;
}
#endif

// that is the fastest variant of MIT HACKMEM count specified to single byte argument
// benches of different variants are available in gbenches/popcount
inline int sphBitCount ( BYTE n )
{
	return (int)( ( ( ( ( DWORD ( n ) * 0x08040201 ) >> 3 ) & 0x11111111 ) * 0x11111111 ) >> 28 );
}