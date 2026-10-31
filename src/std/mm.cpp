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

#include "mm.h"

#include <cassert>

#if _WIN32
#include <cstdlib>
#include <windows.h>
#define PSAPI_VERSION 2
#include <psapi.h>

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

// PrefetchVirtualMemory exists since Windows 8; resolve it at run time so that the binary still loads on older systems
void mmprefetch ( const MemRange_t * pRanges, int iRanges )
{
	if ( !pRanges || iRanges<=0 )
		return;

	struct RangeEntry_t { PVOID m_pAddress; SIZE_T m_uBytes; }; // layout of WIN32_MEMORY_RANGE_ENTRY
	using PrefetchFn_t = BOOL ( WINAPI * ) ( HANDLE, ULONG_PTR, RangeEntry_t *, ULONG );
	static PrefetchFn_t pfnPrefetch = (PrefetchFn_t) GetProcAddress ( GetModuleHandleA ( "kernel32.dll" ), "PrefetchVirtualMemory" );
	if ( !pfnPrefetch )
		return;

	const int BATCH = 64;
	RangeEntry_t dEntries[BATCH];
	int iBatch = 0;
	for ( int i = 0; i < iRanges; i++ )
	{
		if ( !pRanges[i].m_pData || !pRanges[i].m_uLen )
			continue;

		dEntries[iBatch].m_pAddress = (PVOID)pRanges[i].m_pData;
		dEntries[iBatch].m_uBytes = pRanges[i].m_uLen;
		if ( ++iBatch==BATCH )
		{
			pfnPrefetch ( GetCurrentProcess(), iBatch, dEntries, 0 );
			iBatch = 0;
		}
	}

	if ( iBatch )
		pfnPrefetch ( GetCurrentProcess(), iBatch, dEntries, 0 );
}

// a page counts as resident when it is valid in this process' working set; a page that sits in the system
// file cache but has not been touched through this mapping yet reports as missing, so a prefetch may be
// issued for it, which is cheap
bool mmresident ( const void * pData, size_t uLen )
{
	if ( !pData || !uLen )
		return false;

	static const size_t uPage = []() { SYSTEM_INFO tInfo; GetSystemInfo ( &tInfo ); return (size_t)tInfo.dwPageSize; }();
	const uintptr_t uMask = ~( (uintptr_t)uPage-1 );
	uintptr_t uStart = (uintptr_t)pData & uMask;
	const uintptr_t uEnd = ( (uintptr_t)pData + uLen + uPage - 1 ) & uMask;

	const size_t MAX_PAGES = 64;
	PSAPI_WORKING_SET_EX_INFORMATION dInfo[MAX_PAGES];
	while ( uStart<uEnd )
	{
		size_t uPages = ( uEnd-uStart )/uPage;
		if ( uPages>MAX_PAGES )
			uPages = MAX_PAGES;

		for ( size_t i = 0; i < uPages; i++ )
			dInfo[i].VirtualAddress = (PVOID)( uStart + i*uPage );

		if ( !QueryWorkingSetEx ( GetCurrentProcess(), dInfo, (DWORD)( uPages*sizeof ( dInfo[0] ) ) ) )
			return false;

		for ( size_t i = 0; i < uPages; i++ )
			if ( !dInfo[i].VirtualAttributes.Valid )
				return false;

		uStart += uPages*uPage;
	}

	return true;
}

#else

#include <sys/mman.h>
#include <unistd.h>
#include <algorithm>

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


void mmprefetch ( const MemRange_t * pRanges, int iRanges )
{
	if ( !pRanges || iRanges<=0 )
		return;

	static const size_t uPage = (size_t)sysconf ( _SC_PAGESIZE );
	const size_t uMask = ~( uPage-1 );

	// advice works on whole pages; a range that touches or overlaps its predecessor after rounding is merged into it,
	// so neighbouring vectors cost one call
	size_t uCurStart = 0;
	size_t uCurEnd = 0;
	bool bHaveRange = false;
	for ( int i = 0; i < iRanges; i++ )
	{
		if ( !pRanges[i].m_pData || !pRanges[i].m_uLen )
			continue;

		size_t uStart = (size_t)pRanges[i].m_pData & uMask;
		size_t uEnd = ( (size_t)pRanges[i].m_pData + pRanges[i].m_uLen + uPage - 1 ) & uMask;
		if ( bHaveRange && uStart>=uCurStart && uStart<=uCurEnd )
		{
			uCurEnd = std::max ( uCurEnd, uEnd );
			continue;
		}

		if ( bHaveRange )
			madvise ( (void*)uCurStart, uCurEnd-uCurStart, MADV_WILLNEED );

		uCurStart = uStart;
		uCurEnd = uEnd;
		bHaveRange = true;
	}

	if ( bHaveRange )
		madvise ( (void*)uCurStart, uCurEnd-uCurStart, MADV_WILLNEED );
}


bool mmresident ( const void * pData, size_t uLen )
{
	if ( !pData || !uLen )
		return false;

	static const size_t uPage = (size_t)sysconf ( _SC_PAGESIZE );
	const size_t uMask = ~( uPage-1 );
	size_t uStart = (size_t)pData & uMask;
	const size_t uEnd = ( (size_t)pData + uLen + uPage - 1 ) & uMask;

#ifdef __linux__
	using PageStatus_t = unsigned char;
#else
	using PageStatus_t = char;
#endif

	// one status byte per page; long ranges are walked in pieces so that the buffer can live on the stack
	const size_t MAX_PAGES = 64;
	PageStatus_t dStatus[MAX_PAGES];
	while ( uStart<uEnd )
	{
		size_t uPages = std::min ( MAX_PAGES, ( uEnd-uStart )/uPage );
		if ( mincore ( (void*)uStart, uPages*uPage, dStatus )!=0 )
			return false;

		for ( size_t i = 0; i < uPages; i++ )
			if ( !( dStatus[i] & 1 ) )
				return false;

		uStart += uPages*uPage;
	}

	return true;
}

#endif // _WIN32