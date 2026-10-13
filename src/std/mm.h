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
	DONTNEED,
};

void* mmalloc ( size_t uSize, Mode_e = Mode_e::RW, Share_e = Share_e::ANON_PRIVATE );
bool mmapvalid ( const void* pMem );
int mmfree ( void* pMem, size_t uSize );
void mmadvise ( void* pMem, size_t uSize, Advise_e = Advise_e::NODUMP );
bool mmlock ( void* pMem, size_t uSize );
bool mmunlock ( void* pMem, size_t uSize );

struct MemRange_t
{
	const void *	m_pData = nullptr;
	size_t			m_uLen = 0;
};

void mmprefetch ( const MemRange_t * pRanges, int iRanges );

// whether every page of this range of a mapped file is in memory right now
bool mmresident ( const void * pData, size_t uLen );
