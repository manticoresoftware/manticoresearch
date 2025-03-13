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

#pragma once
#include "config.h"

#include "ints.h"
#include "datamove.h"
#include "relimit.h"
#include "vector.h"

/// Allocation for small objects (namely - for movable dynamic attributes).
/// internals based on Alexandresku's 'loki' implementation - 'Allocator for small objects'
static const int MAX_SMALL_OBJECT_SIZE = 64;

BYTE * sphAllocateSmall ( int iBytes );

#if WITH_SMALLALLOC
void sphDeallocateSmall ( const BYTE* pBlob, int iBytes );
#else
void sphDeallocateSmall ( const BYTE* pBlob );
#endif

size_t sphGetSmallAllocatedSize ();	// how many allocated right now
size_t sphGetSmallReservedSize ();	// how many pooled from the sys right now

// helper to use in vector as custom allocator
namespace sph {
	template<typename T>
	class CustomStorage_T
	{
	public:
		using TYPE=T;

		/// grow enough to hold that much entries.
		static TYPE * Allocate ( int64_t iLimit );

#if WITH_SMALLALLOC
		static void Deallocate ( TYPE* pData, int64_t iLimit );
#else
		static void Deallocate ( TYPE * pData );
#endif

		static constexpr bool is_sized = (bool)WITH_SMALLALLOC;
		static constexpr bool is_constructed = true;
		static constexpr bool is_owned = false;
	};
}

template<typename T>
using TightPackedVec_T = sph::Vector_T<T, sph::DefaultCopy_T<T>, sph::TightRelimit, sph::CustomStorage_T<T>>;

#include "smalloc_impl.h"
