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

#ifndef _fnv64_
#define _fnv64_

#include "ints.h"
#include "blobs.h"
#include "generics.h"

constexpr uint64_t SPH_FNV64_SEED = 0xcbf29ce484222325ULL;

uint64_t	sphFNV64 ( const void * pString );
uint64_t	sphFNV64 ( const void * s, int iLen, uint64_t uPrev = SPH_FNV64_SEED );
uint64_t	sphFNV64cont ( const void * pString, uint64_t uPrev );
FORCE_INLINE uint64_t sphFNV64 ( uint64_t uVal, uint64_t uPrev = SPH_FNV64_SEED )
{
	const uint64_t FNV_64_PRIME = 0x100000001b3ULL;
	const BYTE * p = (const BYTE*)&uVal;
	uint64_t hval = uPrev;

	hval ^= (uint64_t)*(p+0);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+1);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+2);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+3);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+4);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+5);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+6);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+7);
	hval *= FNV_64_PRIME;

	return hval;
}

inline uint64_t	sphFNV64 ( const ByteBlob_t & dBlob ) { return sphFNV64 ( dBlob.first, dBlob.second ); }
inline uint64_t	sphFNV64cont ( const ByteBlob_t & dBlob, uint64_t uPrev ) { return sphFNV64 ( dBlob.first, dBlob.second, uPrev ); }

#endif // _fnv64_
