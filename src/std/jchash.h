//
// Copyright (c) 2024-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//
// Jump Consistent Hash - deterministic shard selection
// Reference: "A Fast, Minimal Memory, Consistent Hash Algorithm" by Lamping & Veach (Google, 2014)
//

#ifndef _jchash_
#define _jchash_

#include "ints.h"

/// Jump consistent hash: maps a 64-bit key to a bucket in [0, num_buckets)
/// Deterministic: same key always maps to the same bucket for a given num_buckets.
inline int JumpConsistentHash ( uint64_t uKey, int iNumBuckets )
{
	int64_t b = -1;
	int64_t j = 0;
	while ( j < iNumBuckets )
	{
		b = j;
		uKey = uKey * 2862933555777941757ULL + 1;
		j = (int64_t)( ( b + 1 ) * ( (double)( 1LL << 31 ) / (double)( ( uKey >> 33 ) + 1 ) ) );
	}
	return (int)b;
}

#endif // _jchash_
