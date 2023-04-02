//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "fnv64.h"
#include "ints.h"


uint64_t sphFNV64 ( const void * s )
{
	return sphFNV64cont ( s, SPH_FNV64_SEED );
}


uint64_t sphFNV64 ( const void * s, int iLen, uint64_t uPrev )
{
	const BYTE * p = (const BYTE*)s;
	uint64_t hval = uPrev;
	for ( ; iLen>0; iLen-- )
	{
		// xor the bottom with the current octet
		hval ^= (uint64_t)*p++;

		// multiply by the 64 bit FNV magic prime mod 2^64
		hval += (hval << 1) + (hval << 4) + (hval << 5) + (hval << 7) + (hval << 8) + (hval << 40); // gcc optimization
	}
	return hval;
}


uint64_t sphFNV64cont ( const void * s, uint64_t uPrev )
{
	const BYTE * p = (const BYTE*)s;
	if ( !p )
		return uPrev;

	uint64_t hval = uPrev;
	while ( *p )
	{
		// xor the bottom with the current octet
		hval ^= (uint64_t)*p++;

		// multiply by the 64 bit FNV magic prime mod 2^64
		hval += (hval << 1) + (hval << 4) + (hval << 5) + (hval << 7) + (hval << 8) + (hval << 40); // gcc optimization
	}
	return hval;
}


uint64_t sphFNV64 ( uint64_t uVal, uint64_t uPrev )
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
