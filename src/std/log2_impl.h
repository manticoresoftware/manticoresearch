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

constexpr inline int sphLog2const ( uint64_t uValue )
{
	int iBits = 0;
	while ( uValue )
	{
		uValue >>= 1;
		++iBits;
	}
	return iBits;
}

#if defined( __GNUC__ ) || defined( __clang__ )

inline int sphLog2 ( uint64_t uValue )
{
	if ( !uValue )
		return 0;
	return 64 - __builtin_clzll ( uValue );
}

#elif _WIN32

#include <intrin.h> // for bsr
#pragma intrinsic( _BitScanReverse )

inline int sphLog2 ( uint64_t uValue )
{
	if ( !uValue )
		return 0;
	DWORD uRes;
	if ( BitScanReverse ( &uRes, (DWORD)( uValue >> 32 ) ) )
		return 33 + uRes;
	BitScanReverse ( &uRes, DWORD ( uValue ) );
	return 1 + uRes;
}

#else

inline int sphLog2 ( uint64_t uValue )
{
	int iBits = 0;
	while ( uValue )
	{
		uValue >>= 1;
		++iBits;
	}
	return iBits;
}

#endif