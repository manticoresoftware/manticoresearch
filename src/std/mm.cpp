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

#include "mm.h"

#include <cassert>

#if _WIN32
#include <cstdlib>

void* mmalloc ( size_t uSize, Mode_e, Share_e )
{
	return ::malloc ( (size_t)uSize );
}

bool mmapvalid ( const void* pMem )
{
	return pMem != nullptr;
}

int mmfree ( void* pMem, size_t )
{
	assert ( mmapvalid ( pMem ) );
	::free ( pMem );
	return 0;
}

void mmadvise ( void*, size_t, Advise_e ) {}

bool mmlock ( void* pMem, size_t uSize )
{
	return VirtualLock ( pMem, uSize ) != 0;
}

bool mmunlock ( void* pMem, size_t uSize )
{
	return VirtualUnlock ( pMem, uSize ) != 0;
}

#else

#include <sys/mman.h>

// couple of helpers
int hwShare ( Share_e eAccess )
{
	switch ( eAccess )
	{
	case Share_e::ANON_PRIVATE: return MAP_ANON | MAP_PRIVATE;
	case Share_e::ANON_SHARED: return MAP_ANON | MAP_SHARED;
	case Share_e::SHARED: return MAP_SHARED;
	}
	return MAP_SHARED;
}

int hwMode ( Mode_e eMode )
{
	switch ( eMode )
	{
	case Mode_e::NONE: return PROT_NONE;
	case Mode_e::READ: return PROT_READ;
	case Mode_e::WRITE: return PROT_WRITE;
	case Mode_e::RW: return PROT_READ | PROT_WRITE;
	}
	return PROT_READ | PROT_WRITE;
}

void* mmalloc ( size_t uSize, Mode_e eMode, Share_e eAccess )
{
	return mmap ( NULL, uSize, hwMode ( eMode ), hwShare ( eAccess ), -1, 0 );
}

bool mmapvalid ( const void* pMem )
{
	return pMem != MAP_FAILED;
}

int mmfree ( void* pMem, size_t uSize )
{
	assert ( mmapvalid ( pMem ) );
	return munmap ( pMem, uSize );
}

void mmadvise ( void* pMem, size_t uSize, Advise_e eAdvise )
{
	switch ( eAdvise )
	{
	case Advise_e::NODUMP:
#ifdef MADV_DONTDUMP
		madvise ( pMem, uSize, MADV_DONTDUMP );
#endif
		break;
	case Advise_e::NOFORK:
		madvise ( pMem, uSize,
#ifdef MADV_DONTFORK
			MADV_DONTFORK
#else
			MADV_NORMAL
#endif
		);
		break;
	}
}

bool mmlock ( void* pMem, size_t uSize )
{
	return mlock ( pMem, uSize ) == 0;
}

bool mmunlock ( void* pMem, size_t uSize )
{
	return munlock ( pMem, uSize ) == 0;
}

#endif // _WIN32