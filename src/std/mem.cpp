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

#include "mem.h"
#include "format.h"
#include "fatal.h"
#include <cstdlib>

/////////////////////////////////////////////////////////////////////////////
// DEBUG MEMORY MANAGER
/////////////////////////////////////////////////////////////////////////////

#if SPH_DEBUG_LEAKS

#undef new
#define SPH_DEBUG_DOFREE 1 // 0 will not actually free returned blocks; helps to catch double deletes etc

const DWORD MEMORY_MAGIC_PLAIN = 0xbbbbbbbbUL;
const DWORD MEMORY_MAGIC_ARRAY = 0xaaaaaaaaUL;
const DWORD MEMORY_MAGIC_END = 0xeeeeeeeeUL;
const DWORD MEMORY_MAGIC_DELETED = 0xdedededeUL;


struct CSphMemHeader
{
	DWORD m_uMagic;
	const char* m_sFile;
#if SPH_DEBUG_BACKTRACES
	const char* m_sBacktrace;
#endif
	int m_iLine;
	size_t m_iSize;
	int m_iAllocId;
	BYTE* m_pPointer;
	CSphMemHeader* m_pNext;
	CSphMemHeader* m_pPrev;
};

static CSphMutex g_tAllocsMutex;

static int g_iCurAllocs = 0;
static int g_iAllocsId = 0;
static CSphMemHeader* g_pAllocs = NULL;
static int64_t g_iCurBytes = 0;
static int g_iTotalAllocs = 0;
static int g_iPeakAllocs = 0;
static int64_t g_iPeakBytes = 0;
#if SPH_ALLOC_FILL
static bool g_bFirstRandomAlloc = true;
#endif

void* sphDebugNew ( size_t iSize, const char* sFile, int iLine, bool bArray )
{
	BYTE* pBlock = (BYTE*)::malloc ( iSize + sizeof ( CSphMemHeader ) + sizeof ( DWORD ) );
	if ( !pBlock )
		sphDie ( "out of memory (unable to allocate " UINT64_FMT " bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too

	*(DWORD*)( pBlock + iSize + sizeof ( CSphMemHeader ) ) = MEMORY_MAGIC_END;
	g_tAllocsMutex.Lock();

	CSphMemHeader* pHeader = (CSphMemHeader*)pBlock;
	pHeader->m_uMagic = bArray ? MEMORY_MAGIC_ARRAY : MEMORY_MAGIC_PLAIN;
	pHeader->m_sFile = sFile;
#if SPH_ALLOC_FILL
	if ( g_bFirstRandomAlloc )
	{
		sphAutoSrand();
		g_bFirstRandomAlloc = false;
	}

	BYTE* pBlockPtr = (BYTE*)( pHeader + 1 );
	for ( size_t i = 0; i < iSize; i++ )
		*pBlockPtr++ = BYTE ( sphRand() & 0xFF );
#endif
#if SPH_DEBUG_BACKTRACES
	const char* sTrace = DoBacktrace ( 0, 3 );
	if ( sTrace )
	{
		char* pTrace = (char*)::malloc ( strlen ( sTrace ) + 1 );
		strcpy ( pTrace, sTrace ); // NOLINT
		pHeader->m_sBacktrace = pTrace;
	} else
		pHeader->m_sBacktrace = NULL;
#endif
	pHeader->m_iLine = iLine;
	pHeader->m_iSize = iSize;
	pHeader->m_iAllocId = ++g_iAllocsId;
	pHeader->m_pPointer = pBlock;
	pHeader->m_pNext = g_pAllocs;
	pHeader->m_pPrev = NULL;
	if ( g_pAllocs )
	{
		assert ( !g_pAllocs->m_pPrev );
		g_pAllocs->m_pPrev = pHeader;
	}
	g_pAllocs = pHeader;

	g_iCurAllocs++;
	g_iCurBytes += iSize;

	g_iTotalAllocs++;
	g_iPeakAllocs = Max ( g_iPeakAllocs, g_iCurAllocs );
	g_iPeakBytes = Max ( g_iPeakBytes, g_iCurBytes );

	g_tAllocsMutex.Unlock();
	return pHeader + 1;
}


void sphDebugDelete ( void* pPtr, bool bArray )
{
	if ( !pPtr )
		return;
	g_tAllocsMutex.Lock();

	CSphMemHeader* pHeader = ( (CSphMemHeader*)pPtr ) - 1;
	switch ( pHeader->m_uMagic )
	{
	case MEMORY_MAGIC_ARRAY:
		if ( !bArray )
			sphDie ( "delete [] on non-array block %d allocated at %s(%d)",
				pHeader->m_iAllocId,
				pHeader->m_sFile,
				pHeader->m_iLine );
		break;

	case MEMORY_MAGIC_PLAIN:
		if ( bArray )
			sphDie ( "delete on array block %d allocated at %s(%d)",
				pHeader->m_iAllocId,
				pHeader->m_sFile,
				pHeader->m_iLine );
		break;

	case MEMORY_MAGIC_DELETED:
		sphDie ( "double delete on block %d allocated at %s(%d)",
			pHeader->m_iAllocId,
			pHeader->m_sFile,
			pHeader->m_iLine );
		break;

	default:
		sphDie ( "delete on unmanaged block at 0x%08x", pPtr );
		return;
	}

	BYTE* pBlock = (BYTE*)pHeader;
	if ( *(DWORD*)( pBlock + pHeader->m_iSize + sizeof ( CSphMemHeader ) ) != MEMORY_MAGIC_END )
		sphDie ( "out-of-bounds write beyond block %d allocated at %s(%d)",
			pHeader->m_iAllocId,
			pHeader->m_sFile,
			pHeader->m_iLine );

	// unchain
	if ( pHeader == g_pAllocs )
		g_pAllocs = g_pAllocs->m_pNext;

	if ( pHeader->m_pPrev )
	{
		assert ( pHeader->m_pPrev->m_uMagic == MEMORY_MAGIC_PLAIN || pHeader->m_pPrev->m_uMagic == MEMORY_MAGIC_ARRAY );
		pHeader->m_pPrev->m_pNext = pHeader->m_pNext;
	}
	if ( pHeader->m_pNext )
	{
		assert ( pHeader->m_pNext->m_uMagic == MEMORY_MAGIC_PLAIN || pHeader->m_pNext->m_uMagic == MEMORY_MAGIC_ARRAY );
		pHeader->m_pNext->m_pPrev = pHeader->m_pPrev;
	}

	pHeader->m_pPrev = NULL;
	pHeader->m_pNext = NULL;

	// mark and delete
	pHeader->m_uMagic = MEMORY_MAGIC_DELETED;

	g_iCurAllocs--;
	g_iCurBytes -= pHeader->m_iSize;

#if SPH_DEBUG_BACKTRACES
	if ( pHeader->m_sBacktrace )
		::free ( (void*)pHeader->m_sBacktrace );
#endif

#if SPH_DEBUG_DOFREE
	::free ( pHeader );
#endif

	g_tAllocsMutex.Unlock();
}


int64_t sphAllocBytes()
{
	return g_iCurBytes;
}


int sphAllocsCount()
{
	return g_iCurAllocs;
}


int sphAllocsLastID()
{
	return g_iAllocsId;
}


void sphAllocsDump ( int iFile, int iSinceID )
{
	g_tAllocsMutex.Lock();

	sphSafeInfo ( iFile, "--- dumping allocs since %d ---\n", iSinceID );

	uint64_t iTotalBytes = 0;
	int iTotal = 0;

	for ( CSphMemHeader* pHeader = g_pAllocs;
		  pHeader && pHeader->m_iAllocId > iSinceID;
		  pHeader = pHeader->m_pNext )
	{
		sphSafeInfo ( iFile, "alloc %d at %s(%d): 0x%0p %d bytes\n", pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine, pHeader->m_pPointer, (int)pHeader->m_iSize );

#if SPH_DEBUG_BACKTRACES
		sphSafeInfo ( iFile, "Backtrace:\n%s\n", pHeader->m_sBacktrace );
#endif

		iTotalBytes += pHeader->m_iSize;
		iTotal++;
	}

	sphSafeInfo ( iFile, "total allocs %d: %d.%03d bytes", iTotal, (int)( iTotalBytes / 1024 ), (int)( iTotalBytes % 1000 ) );
	sphSafeInfo ( iFile, "--- end of dump ---\n" );

	g_tAllocsMutex.Unlock();
}


void sphAllocsStats()
{
	fprintf ( stdout, "--- total-allocs=%d, peak-allocs=%d, peak-bytes=" INT64_FMT "\n", g_iTotalAllocs, g_iPeakAllocs, g_iPeakBytes );
}


void sphAllocsCheck()
{
	g_tAllocsMutex.Lock();
	for ( CSphMemHeader* pHeader = g_pAllocs; pHeader; pHeader = pHeader->m_pNext )
	{
		BYTE* pBlock = (BYTE*)pHeader;

		if ( !( pHeader->m_uMagic == MEMORY_MAGIC_ARRAY || pHeader->m_uMagic == MEMORY_MAGIC_PLAIN ) )
			sphDie ( "corrupted header in block %d allocated at %s(%d)",
				pHeader->m_iAllocId,
				pHeader->m_sFile,
				pHeader->m_iLine );

		if ( *(DWORD*)( pBlock + pHeader->m_iSize + sizeof ( CSphMemHeader ) ) != MEMORY_MAGIC_END )
			sphDie ( "out-of-bounds write beyond block %d allocated at %s(%d)",
				pHeader->m_iAllocId,
				pHeader->m_sFile,
				pHeader->m_iLine );
	}
	g_tAllocsMutex.Unlock();
}

void sphMemStatInit() {}
void sphMemStatDone() {}
void sphMemStatDump ( int ) {}

//////////////////////////////////////////////////////////////////////////

void* operator new ( size_t iSize, const char* sFile, int iLine )
{
	return sphDebugNew ( iSize, sFile, iLine, false );
}


void* operator new[] ( size_t iSize, const char* sFile, int iLine )
{
	return sphDebugNew ( iSize, sFile, iLine, true );
}


void operator delete ( void* pPtr ) MYTHROW()
{
	sphDebugDelete ( pPtr, false );
}


void operator delete[] ( void* pPtr ) MYTHROW()
{
	sphDebugDelete ( pPtr, true );
}

/// debug allocate to use in custom allocator
void* debugallocate ( size_t iSize )
{
	return sphDebugNew ( iSize, __FILE__, __LINE__, false );
}

/// debug deallocate to use in custom allocator
void debugdeallocate ( void* pPtr )
{
	sphDebugDelete ( pPtr, false );
}


//////////////////////////////////////////////////////////////////////////////
// ALLOCACTIONS COUNT/SIZE PROFILER
//////////////////////////////////////////////////////////////////////////////

#elif SPH_ALLOCS_PROFILER

#undef new

static CSphMutex g_tAllocsMutex;
static int g_iAllocsId = 0;
static int g_iCurAllocs = 0;
static int64_t g_iCurBytes = 0;
static int g_iTotalAllocs = 0;
static int g_iPeakAllocs = 0;
static int64_t g_iPeakBytes = 0;

// statictic's per memory category
struct MemCategorized_t
{
	int64_t m_iSize;
	int m_iCount;

	MemCategorized_t()
		: m_iSize ( 0 )
		, m_iCount ( 0 )
	{
	}
};

static MemCategory_e sphMemStatGet();

// memory categories storage
static MemCategorized_t g_dMemCategoryStat[MEM_TOTAL];

//////////////////////////////////////////////////////////////////////////
// ALLOCATIONS COUNT/SIZE PROFILER
//////////////////////////////////////////////////////////////////////////

void* sphDebugNew ( size_t iSize )
{
	BYTE* pBlock = (BYTE*)::malloc ( iSize + sizeof ( size_t ) * 2 );
	if ( !pBlock )
		sphDie ( "out of memory (unable to allocate %" PRIu64 " bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too

	const int iMemType = sphMemStatGet();
	assert ( iMemType >= 0 && iMemType < MEM_TOTAL );

	g_tAllocsMutex.Lock();

	g_iAllocsId++;
	g_iCurAllocs++;
	g_iCurBytes += iSize;
	g_iTotalAllocs++;
	g_iPeakAllocs = Max ( g_iCurAllocs, g_iPeakAllocs );
	g_iPeakBytes = Max ( g_iCurBytes, g_iPeakBytes );

	g_dMemCategoryStat[iMemType].m_iSize += iSize;
	g_dMemCategoryStat[iMemType].m_iCount++;

	g_tAllocsMutex.Unlock();

	size_t* pData = (size_t*)pBlock;
	pData[0] = iSize;
	pData[1] = iMemType;

	return pBlock + sizeof ( size_t ) * 2;
}

void sphDebugDelete ( void* pPtr )
{
	if ( !pPtr )
		return;

	size_t* pBlock = (size_t*)pPtr;
	pBlock -= 2;

	const int iSize = pBlock[0];
	const int iMemType = pBlock[1];
	assert ( iMemType >= 0 && iMemType < MEM_TOTAL );

	g_tAllocsMutex.Lock();

	g_iCurAllocs--;
	g_iCurBytes -= iSize;

	g_dMemCategoryStat[iMemType].m_iSize -= iSize;
	g_dMemCategoryStat[iMemType].m_iCount--;

	g_tAllocsMutex.Unlock();

	::free ( pBlock );
}

void sphAllocsStats()
{
	g_tAllocsMutex.Lock();
	fprintf ( stdout, "--- total-allocs=%d, peak-allocs=%d, peak-bytes=" INT64_FMT "\n", g_iTotalAllocs, g_iPeakAllocs, g_iPeakBytes );
	g_tAllocsMutex.Unlock();
}

int64_t sphAllocBytes()
{
	return g_iCurBytes;
}
int sphAllocsCount()
{
	return g_iCurAllocs;
}
int sphAllocsLastID()
{
	return g_iAllocsId;
}
void sphAllocsDump ( int, int ) {}
void sphAllocsCheck() {}

void* operator new ( size_t iSize, const char*, int )
{
	return sphDebugNew ( iSize );
}
void* operator new[] ( size_t iSize, const char*, int )
{
	return sphDebugNew ( iSize );
}
void operator delete ( void* pPtr ) MYTHROW()
{
	sphDebugDelete ( pPtr );
}
void operator delete[] ( void* pPtr ) MYTHROW()
{
	sphDebugDelete ( pPtr );
}

/// debug allocate to use in custom allocator
void* debugallocate ( size_t iSize )
{
	return sphDebugNew ( iSize );
}

/// debug deallocate to use in custom allocator
void debugdeallocate ( void* pPtr )
{
	sphDebugDelete ( pPtr );
}

//////////////////////////////////////////////////////////////////////////////
// MEMORY STATISTICS
//////////////////////////////////////////////////////////////////////////////

STATIC_ASSERT ( MEM_TOTAL < 255, TOO_MANY_MEMORY_CATEGORIES );

// stack of memory categories as we move deeper and deeper
class MemCategoryStack_t // NOLINT
{
#define MEM_STACK_MAX 1024
	BYTE m_dStack[MEM_STACK_MAX];
	int m_iDepth;

public:
	// ctor ( cross platform )
	void Reset()
	{
		m_iDepth = 0;
		m_dStack[0] = MEM_CORE;
	}

	void Push ( MemCategory_e eCategory )
	{
		assert ( eCategory >= 0 && eCategory < MEM_TOTAL );
		assert ( m_iDepth + 1 < MEM_STACK_MAX );
		m_dStack[++m_iDepth] = (BYTE)eCategory;
	}

#ifndef NDEBUG
	void Pop ( MemCategory_e eCategory )
	{
		assert ( eCategory >= 0 && eCategory < MEM_TOTAL );
#else
	void Pop ( MemCategory_e )
	{
#endif

		assert ( m_iDepth - 1 >= 0 );
		assert ( m_dStack[m_iDepth] == eCategory );
		m_iDepth--;
	}

	MemCategory_e Top() const
	{
		assert ( m_iDepth >= 0 && m_iDepth < MEM_STACK_MAX );
		assert ( m_dStack[m_iDepth] >= 0 && m_dStack[m_iDepth] < MEM_TOTAL );
		return MemCategory_e ( m_dStack[m_iDepth] );
	}
};

/// TLS key of memory category stack
thread_local MemCategoryStack_t* g_tTLSMemCategory;

static MemCategoryStack_t* g_pMainTLS = NULL; // category stack of main thread

// memory statistic's per thread factory
void* sphMemStatThdInit()
{
	MemCategoryStack_t* pTLS = (MemCategoryStack_t*)sphDebugNew ( sizeof ( MemCategoryStack_t ) );
	pTLS->Reset();

	g_tTLSMemCategory = pTLS;
	return pTLS;
}

// per thread cleanup of memory statistic's
void sphMemStatThdCleanup ( void* pTLS )
{
	sphDebugDelete ( (MemCategoryStack_t*)pTLS );
}

// init of memory statistic's data
void sphMemStatInit()
{
	// main thread statistic's creation
	assert ( g_pMainTLS == NULL );
	g_pMainTLS = (MemCategoryStack_t*)sphMemStatThdInit();
	assert ( g_pMainTLS != NULL );
}

// cleanup of memory statistic's data
void sphMemStatDone()
{
	assert ( g_pMainTLS != NULL );
	sphMemStatThdCleanup ( g_pMainTLS );
}

// direct access for special category
void sphMemStatMMapAdd ( int64_t iSize )
{
	g_tAllocsMutex.Lock();

	g_iCurAllocs++;
	g_iCurBytes += iSize;
	g_iTotalAllocs++;
	g_iPeakAllocs = Max ( g_iCurAllocs, g_iPeakAllocs );
	g_iPeakBytes = Max ( g_iCurBytes, g_iPeakBytes );

	g_dMemCategoryStat[MEM_MMAPED].m_iSize += iSize;
	g_dMemCategoryStat[MEM_MMAPED].m_iCount++;

	g_tAllocsMutex.Unlock();
}

void sphMemStatMMapDel ( int64_t iSize )
{
	g_tAllocsMutex.Lock();

	g_iCurAllocs--;
	g_iCurBytes -= iSize;

	g_dMemCategoryStat[MEM_MMAPED].m_iSize -= iSize;
	g_dMemCategoryStat[MEM_MMAPED].m_iCount--;

	g_tAllocsMutex.Unlock();
}

// push new category on arrival
void sphMemStatPush ( MemCategory_e eCategory )
{
	MemCategoryStack_t* pTLS = g_tTLSMemCategory;
	if ( pTLS )
		pTLS->Push ( eCategory );
};

// restore last category
void sphMemStatPop ( MemCategory_e eCategory )
{
	MemCategoryStack_t* pTLS = g_tTLSMemCategory;
	if ( pTLS )
		pTLS->Pop ( eCategory );
};

// get current category
static MemCategory_e sphMemStatGet()
{
	MemCategoryStack_t* pTLS = g_tTLSMemCategory;
	return pTLS ? pTLS->Top() : MEM_CORE;
}


// human readable category names
#define MEM_CATEGORY( _arg ) #_arg
static const char* g_dMemCategoryName[] = { MEM_CATEGORIES };
#undef MEM_CATEGORY


void sphMemStatDump ( int iFD )
{
	int64_t iSize = 0;
	int iCount = 0;
	for ( int i = 0; i < MEM_TOTAL; i++ )
	{
		iSize += (int64_t)g_dMemCategoryStat[i].m_iSize;
		iCount += g_dMemCategoryStat[i].m_iCount;
	}

	sphSafeInfo ( iFD, "%-24s allocs-count=%d, mem-total=%d.%d Mb", "(total)", iCount, (int)( iSize / 1048576 ), (int)( ( iSize * 10 / 1048576 ) % 10 ) );

	for ( int i = 0; i < MEM_TOTAL; i++ )
		if ( g_dMemCategoryStat[i].m_iCount > 0 )
		{
			iSize = (int64_t)g_dMemCategoryStat[i].m_iSize;
			sphSafeInfo ( iFD, "%-24s allocs-count=%d, mem-total=%d.%d Mb", g_dMemCategoryName[i], g_dMemCategoryStat[i].m_iCount, (int)( iSize / 1048576 ), (int)( ( iSize * 10 / 1048576 ) % 10 ) );
		}
}

//////////////////////////////////////////////////////////////////////////////
// PRODUCTION MEMORY MANAGER
//////////////////////////////////////////////////////////////////////////////

#elif !DISABLE_MEMROUTINES

void* operator new ( std::size_t iSize )
{
	void* pResult = std::malloc ( iSize );
	if ( !pResult )
		sphDieRestart ( "out of memory (unable to allocate " UINT64_FMT " bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too
	return pResult;
}

void operator delete ( void* pPtr ) noexcept
{
	std::free ( pPtr );
}

/* According to https://en.cppreference.com/w/cpp/memory/new/operator_new, code below is not necessary

void* operator new[] ( size_t iSize )
{
	void* pResult = ::malloc ( iSize );
	if ( !pResult )
		sphDieRestart ( "out of memory (unable to allocate " UINT64_FMT " bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too
	return pResult;
}

void operator delete[] ( void* pPtr ) throw()
{
	if ( pPtr )
		::free ( pPtr );
}
 */

#endif // DISABLE_MEMROUTINES

//////////////////////////////////////////////////////////////////////////

// now let the rest of sphinxstd use proper new
#if SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER
#undef new
#define new new ( __FILE__, __LINE__ )
#endif