//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#ifndef _fnv64_
#define _fnv64_

#include "ints.h"
#include "blobs.h"
#include "generics.h"

constexpr uint64_t SPH_FNV64_SEED = 0xcbf29ce484222325ULL;

uint64_t	sphFNV64 ( const void * pString );
uint64_t	sphFNV64 ( const void * s, int iLen, uint64_t uPrev = SPH_FNV64_SEED );
uint64_t	sphFNV64cont ( const void * pString, uint64_t uPrev );

template<typename INT, int iLen = sizeof(INT)>
FORCE_INLINE typename std::enable_if<std::is_integral<INT>::value, uint64_t>::type sphFNV64 ( INT uVal, uint64_t uPrev = SPH_FNV64_SEED )
{
	constexpr uint64_t FNV_64_PRIME = 0x100000001b3ULL;
	if constexpr ( iLen==1 )
		return (uPrev ^ (uint64_t)uVal) * FNV_64_PRIME;

	if constexpr ( iLen==2 )
	{
		const auto * p = (const BYTE*)&uVal;
		return sphFNV64(p[1],sphFNV64(p[0],uPrev));
	}

	if constexpr ( iLen==4 )
	{
		const auto * p = (const uint16_t*)&uVal;
		return sphFNV64(p[1],sphFNV64(p[0],uPrev));
	}

	if constexpr ( iLen==8 )
	{
		const auto * p = (const uint32_t*)&uVal;
		return sphFNV64(p[1],sphFNV64(p[0],uPrev));
	}
}

// same as sphFNV64(0,sphFNV64(0,sphFNV64(0,sphFNV64((BYTE)uVal))))
FORCE_INLINE uint64_t sphFNV64_4x ( BYTE uVal, uint64_t uPrev = SPH_FNV64_SEED )
{
	constexpr uint64_t FNV_64_PRIMEx4 = 0x9FFAAC085635BC91ULL;
	return (uPrev ^ (uint64_t)uVal) * FNV_64_PRIMEx4;
}

inline uint64_t	sphFNV64 ( const ByteBlob_t & dBlob ) { return sphFNV64 ( dBlob.first, dBlob.second, SPH_FNV64_SEED ); }
inline uint64_t	sphFNV64cont ( const ByteBlob_t & dBlob, uint64_t uPrev ) { return sphFNV64 ( dBlob.first, dBlob.second, uPrev ); }

#endif // _fnv64_
