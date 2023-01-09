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