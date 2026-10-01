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

#pragma once

#include "ints.h"

enum class Mode_e {
	NONE,
	READ,
	WRITE,
	RW,
};

enum class Share_e {
	ANON_PRIVATE,
	ANON_SHARED,
	SHARED,
};

enum class Advise_e {
	NOFORK,
	NODUMP,
};

void* mmalloc ( size_t uSize, Mode_e = Mode_e::RW, Share_e = Share_e::ANON_PRIVATE );
bool mmapvalid ( const void* pMem );
int mmfree ( void* pMem, size_t uSize );
void mmadvise ( void* pMem, size_t uSize, Advise_e = Advise_e::NODUMP );
bool mmlock ( void* pMem, size_t uSize );
bool mmunlock ( void* pMem, size_t uSize );

/// a memory range to prefetch, see mmprefetch()
struct MemRange_t
{
	const void *	m_pData = nullptr;
	size_t			m_uLen = 0;
};

enum class PrefetchResult_e
{
	NONE,			///< nothing was requested: unsupported here, or the single call is unavailable and no fallback was allowed
	SINGLE_CALL,	///< all ranges went to the OS in one call (one per 1024 ranges)
	PER_RANGE		///< one call per range
};

/// Ask the OS to start reading these ranges of mapped files now, without waiting for the reads to finish.
/// Only the pages the ranges cover are requested (no read-around), and all the reads are submitted before any is
/// awaited, so they overlap instead of being served one page fault at a time.
/// On Linux this is a single process_madvise() call. Where the kernel does not offer it to this process, the result
/// is NONE unless bAllowPerRange is set, in which case every range gets its own madvise() call.
PrefetchResult_e mmprefetch ( const MemRange_t * pRanges, int iRanges, bool bAllowPerRange );

/// errno that made the single call unavailable to this process; 0 while it works or was never tried
int mmprefetch_single_call_errno();

/// Whether every page of this range of a mapped file is in memory right now. Reads nothing, has no side effects.
/// False also when it can't be told (unsupported platform, bad range), so that callers err towards prefetching.
/// NB: since Linux 5.0 the kernel answers truthfully only for files the caller owns or may write to; for any other
/// file it reports every page as present.
bool mmresident ( const void * pData, size_t uLen );