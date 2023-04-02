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

#ifndef _fnv64_
#define _fnv64_

#include "ints.h"
#include "blobs.h"

constexpr uint64_t SPH_FNV64_SEED = 0xcbf29ce484222325ULL;

uint64_t	sphFNV64 ( const void * pString );
uint64_t	sphFNV64 ( const void * s, int iLen, uint64_t uPrev = SPH_FNV64_SEED );
uint64_t	sphFNV64cont ( const void * pString, uint64_t uPrev );
uint64_t	sphFNV64 ( uint64_t uVal, uint64_t uPrev = SPH_FNV64_SEED );

inline uint64_t	sphFNV64 ( const ByteBlob_t & dBlob ) { return sphFNV64 ( dBlob.first, dBlob.second ); }
inline uint64_t	sphFNV64cont ( const ByteBlob_t & dBlob, uint64_t uPrev ) { return sphFNV64 ( dBlob.first, dBlob.second, uPrev ); }

#endif // _fnv64_
