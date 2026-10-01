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

PrefetchResult_e mmprefetch ( const MemRange_t *, int, bool )
{
	return PrefetchResult_e::NONE;
}

int mmprefetch_single_call_errno()
{
	return 0;
}

bool mmresident ( const void *, size_t )
{
	return false;
}

#else

#include <sys/mman.h>
#include <sys/uio.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <cerrno>
#include <vector>

#ifdef __linux__
#include <sys/syscall.h>

// both calls are newer than many libc headers; the numbers are the same on every architecture
#ifndef SYS_pidfd_open
#define SYS_pidfd_open 434
#endif
#ifndef SYS_process_madvise
#define SYS_process_madvise 440
#endif
#endif

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


#ifdef __linux__
static std::atomic<int> g_iPrefetchSingleCallErrno { 0 };

// a pidfd of this very process: process_madvise() wants one even when the target is the caller
static int SelfPidFD()
{
	static std::atomic<int> iFD { -2 };	// -2: not opened yet, -1: unavailable
	int iCur = iFD.load ( std::memory_order_acquire );
	if ( iCur!=-2 )
		return iCur;

	int iNew = (int)syscall ( SYS_pidfd_open, getpid(), 0 );
	if ( iNew<0 )
	{
		g_iPrefetchSingleCallErrno.store ( errno, std::memory_order_relaxed );
		iNew = -1;
	}

	if ( iFD.compare_exchange_strong ( iCur, iNew, std::memory_order_acq_rel ) )
		return iNew;

	// another thread opened it first; iCur now holds its result
	if ( iNew>=0 )
		close(iNew);

	return iCur;
}


static bool PrefetchSingleCall ( std::vector<iovec> & dVec )
{
	static std::atomic<bool> bUnavailable { false };
	if ( bUnavailable.load ( std::memory_order_relaxed ) )
		return false;

	int iFD = SelfPidFD();
	if ( iFD<0 )
	{
		bUnavailable.store ( true, std::memory_order_relaxed );
		return false;
	}

	const size_t MAX_VEC = 1024;	// UIO_MAXIOV
	for ( size_t i = 0; i < dVec.size(); i += MAX_VEC )
	{
		size_t uCount = std::min ( MAX_VEC, dVec.size()-i );
		long iRes = syscall ( SYS_process_madvise, iFD, &dVec[i], uCount, MADV_WILLNEED, 0U );
		if ( iRes>=0 )
			continue;

		// The kernel lacks the call (ENOSYS), does not take this advice through it (EINVAL), or wants CAP_SYS_NICE
		// even for the caller's own memory (EPERM, older kernels). None of that changes while the process lives.
		if ( errno==ENOSYS || errno==EINVAL || errno==EPERM || errno==EBADF )
		{
			g_iPrefetchSingleCallErrno.store ( errno, std::memory_order_relaxed );
			bUnavailable.store ( true, std::memory_order_relaxed );
			return false;
		}

		// anything else (say, ENOMEM for a range that is not mapped) concerns these ranges only
	}

	return true;
}
#endif // __linux__


PrefetchResult_e mmprefetch ( const MemRange_t * pRanges, int iRanges, bool bAllowPerRange )
{
	if ( !pRanges || iRanges<=0 )
		return PrefetchResult_e::NONE;

	static const size_t uPage = (size_t)sysconf ( _SC_PAGESIZE );
	const size_t uMask = ~( uPage-1 );

	// advice works on whole pages; ranges that touch or overlap their predecessor after rounding are merged into it
	std::vector<iovec> dVec;
	dVec.reserve ( iRanges );
	for ( int i = 0; i < iRanges; i++ )
	{
		if ( !pRanges[i].m_pData || !pRanges[i].m_uLen )
			continue;

		size_t uStart = (size_t)pRanges[i].m_pData & uMask;
		size_t uEnd = ( (size_t)pRanges[i].m_pData + pRanges[i].m_uLen + uPage - 1 ) & uMask;
		if ( !dVec.empty() )
		{
			iovec & tLast = dVec.back();
			size_t uLastStart = (size_t)tLast.iov_base;
			size_t uLastEnd = uLastStart + tLast.iov_len;
			if ( uStart>=uLastStart && uStart<=uLastEnd )
			{
				if ( uEnd>uLastEnd )
					tLast.iov_len = uEnd-uLastStart;

				continue;
			}
		}

		iovec tVec;
		tVec.iov_base = (void*)uStart;
		tVec.iov_len = uEnd-uStart;
		dVec.push_back(tVec);
	}

	if ( dVec.empty() )
		return PrefetchResult_e::NONE;

#ifdef __linux__
	if ( PrefetchSingleCall(dVec) )
		return PrefetchResult_e::SINGLE_CALL;
#endif

	if ( !bAllowPerRange )
		return PrefetchResult_e::NONE;

	for ( const iovec & tVec : dVec )
		madvise ( tVec.iov_base, tVec.iov_len, MADV_WILLNEED );

	return PrefetchResult_e::PER_RANGE;
}


int mmprefetch_single_call_errno()
{
#ifdef __linux__
	return g_iPrefetchSingleCallErrno.load ( std::memory_order_relaxed );
#else
	return ENOSYS;
#endif
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