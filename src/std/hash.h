//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "generics.h"

#define XXH_STATIC_LINKING_ONLY
#define XXH_INLINE_ALL
#include "xxhash.h"

// a wrapper for xxHash
FORCE_INLINE uint64_t HashWithSeed ( const void * pInput, size_t tLength, uint64_t uSeed )
{
	return XXH3_64bits_withSeed ( pInput, tLength, uSeed );
}
