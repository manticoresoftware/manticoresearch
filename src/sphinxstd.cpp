//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"
#include "sphinxutils.h"

#include <math.h>

#if !USE_WINDOWS
#include <sys/time.h> // for gettimeofday

// define this if you want to run gprof over the threads model - to track children threads also.
#define USE_GPROF 0

#endif

int g_iThreadStackSize = 1024*1024;

//////////////////////////////////////////////////////////////////////////

char CSphString::EMPTY[] = "";

#if USE_WINDOWS
#ifndef NDEBUG

void sphAssert ( const char * sExpr, const char * sFile, int iLine )
{
	char sBuffer [ 1024 ];
	_snprintf ( sBuffer, sizeof(sBuffer), "%s(%d): assertion %s failed\n", sFile, iLine, sExpr );

	if ( MessageBox ( NULL, sBuffer, "Assert failed! Cancel to debug.",
		MB_OKCANCEL | MB_TOPMOST | MB_SYSTEMMODAL | MB_ICONEXCLAMATION )!=IDOK )
	{
		__debugbreak ();
	} else
	{
		fprintf ( stdout, "%s", sBuffer );
		exit ( 1 );
	}
}

#endif // !NDEBUG
#endif // USE_WINDOWS

/////////////////////////////////////////////////////////////////////////////
// DEBUG MEMORY MANAGER
/////////////////////////////////////////////////////////////////////////////

#if SPH_DEBUG_LEAKS

#undef new
#define SPH_DEBUG_DOFREE 1 // 0 will not actually free returned blocks; helps to catch double deletes etc

const DWORD MEMORY_MAGIC_PLAIN		= 0xbbbbbbbbUL;
const DWORD MEMORY_MAGIC_ARRAY		= 0xaaaaaaaaUL;
const DWORD MEMORY_MAGIC_END		= 0xeeeeeeeeUL;
const DWORD MEMORY_MAGIC_DELETED	= 0xdedededeUL;


struct CSphMemHeader
{
	DWORD			m_uMagic;
	const char *	m_sFile;
#if SPH_DEBUG_BACKTRACES
	const char *	m_sBacktrace;
#endif
	int				m_iLine;
	size_t			m_iSize;
	int				m_iAllocId;
	BYTE *			m_pPointer;
	CSphMemHeader *	m_pNext;
	CSphMemHeader *	m_pPrev;
};

static CSphMutex		g_tAllocsMutex;

static int				g_iCurAllocs	= 0;
static int				g_iAllocsId		= 0;
static CSphMemHeader *	g_pAllocs		= NULL;
static int64_t			g_iCurBytes		= 0;
static int				g_iTotalAllocs	= 0;
static int				g_iPeakAllocs	= 0;
static int64_t			g_iPeakBytes	= 0;
#if SPH_ALLOC_FILL
static bool				g_bFirstRandomAlloc = true;
#endif

void * sphDebugNew ( size_t iSize, const char * sFile, int iLine, bool bArray )
{
	BYTE * pBlock = (BYTE*) ::malloc ( iSize+sizeof(CSphMemHeader)+sizeof(DWORD) );
	if ( !pBlock )
		sphDie ( "out of memory (unable to allocate " UINT64_FMT " bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too

	*(DWORD*)( pBlock+iSize+sizeof(CSphMemHeader) ) = MEMORY_MAGIC_END;
	g_tAllocsMutex.Lock();

	CSphMemHeader * pHeader = (CSphMemHeader*) pBlock;
	pHeader->m_uMagic = bArray ? MEMORY_MAGIC_ARRAY : MEMORY_MAGIC_PLAIN;
	pHeader->m_sFile = sFile;
#if SPH_ALLOC_FILL
	if ( g_bFirstRandomAlloc )
	{
		sphAutoSrand();
		g_bFirstRandomAlloc = false;
	}

	BYTE * pBlockPtr = (BYTE*)(pHeader+1);
	for ( size_t i = 0; i < iSize; i++ )
		*pBlockPtr++ = BYTE(sphRand () & 0xFF);
#endif
#if SPH_DEBUG_BACKTRACES
	const char * sTrace = DoBacktrace ( 0, 3 );
	if ( sTrace )
	{
		char * pTrace = (char*) ::malloc ( strlen(sTrace) + 1 );
		strcpy ( pTrace, sTrace ); //NOLINT
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
	return pHeader+1;
}


void sphDebugDelete ( void * pPtr, bool bArray )
{
	if ( !pPtr )
		return;
	g_tAllocsMutex.Lock();

	CSphMemHeader * pHeader = ((CSphMemHeader*)pPtr)-1;
	switch ( pHeader->m_uMagic )
	{
		case MEMORY_MAGIC_ARRAY:
			if ( !bArray )
				sphDie ( "delete [] on non-array block %d allocated at %s(%d)",
					pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );
			break;

		case MEMORY_MAGIC_PLAIN:
			if ( bArray )
				sphDie ( "delete on array block %d allocated at %s(%d)",
					pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );
			break;

		case MEMORY_MAGIC_DELETED:
			sphDie ( "double delete on block %d allocated at %s(%d)",
				pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );
			break;

		default:
			sphDie ( "delete on unmanaged block at 0x%08x", pPtr );
			return;
	}

	BYTE * pBlock = (BYTE*) pHeader;
	if ( *(DWORD*)( pBlock+pHeader->m_iSize+sizeof(CSphMemHeader) )!=MEMORY_MAGIC_END )
		sphDie ( "out-of-bounds write beyond block %d allocated at %s(%d)",
			pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );

	// unchain
	if ( pHeader==g_pAllocs )
		g_pAllocs = g_pAllocs->m_pNext;

	if ( pHeader->m_pPrev )
	{
		assert ( pHeader->m_pPrev->m_uMagic==MEMORY_MAGIC_PLAIN || pHeader->m_pPrev->m_uMagic==MEMORY_MAGIC_ARRAY );
		pHeader->m_pPrev->m_pNext = pHeader->m_pNext;
	}
	if ( pHeader->m_pNext )
	{
		assert ( pHeader->m_pNext->m_uMagic==MEMORY_MAGIC_PLAIN || pHeader->m_pNext->m_uMagic==MEMORY_MAGIC_ARRAY );
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
		::free ( (void*) pHeader->m_sBacktrace );
#endif

#if SPH_DEBUG_DOFREE
	::free ( pHeader );
#endif

	g_tAllocsMutex.Unlock();
}


int64_t	sphAllocBytes ()
{
	return g_iCurBytes;
}


int sphAllocsCount ()
{
	return g_iCurAllocs;
}


int sphAllocsLastID ()
{
	return g_iAllocsId;
}


void sphAllocsDump ( int iFile, int iSinceID )
{
	g_tAllocsMutex.Lock();

	sphSafeInfo ( iFile, "--- dumping allocs since %d ---\n", iSinceID );

	uint64_t iTotalBytes = 0;
	int iTotal = 0;

	for ( CSphMemHeader * pHeader = g_pAllocs;
		pHeader && pHeader->m_iAllocId > iSinceID;
		pHeader = pHeader->m_pNext )
	{
		sphSafeInfo ( iFile, "alloc %d at %s(%d): 0x%0p %d bytes\n", pHeader->m_iAllocId,
			pHeader->m_sFile, pHeader->m_iLine, pHeader->m_pPointer, (int)pHeader->m_iSize );

#if SPH_DEBUG_BACKTRACES
		sphSafeInfo ( iFile, "Backtrace:\n%s\n", pHeader->m_sBacktrace );
#endif

		iTotalBytes += pHeader->m_iSize;
		iTotal++;
	}

	sphSafeInfo ( iFile, "total allocs %d: %d.%03d bytes", iTotal, (int)(iTotalBytes/1024), (int)(iTotalBytes%1000) );
	sphSafeInfo ( iFile, "--- end of dump ---\n" );

	g_tAllocsMutex.Unlock();
}


void sphAllocsStats ()
{
	fprintf ( stdout, "--- total-allocs=%d, peak-allocs=%d, peak-bytes=" INT64_FMT "\n",
		g_iTotalAllocs, g_iPeakAllocs, g_iPeakBytes );
}


void sphAllocsCheck ()
{
	g_tAllocsMutex.Lock();
	for ( CSphMemHeader * pHeader=g_pAllocs; pHeader; pHeader=pHeader->m_pNext )
	{
		BYTE * pBlock = (BYTE*) pHeader;

		if (!( pHeader->m_uMagic==MEMORY_MAGIC_ARRAY || pHeader->m_uMagic==MEMORY_MAGIC_PLAIN ))
			sphDie ( "corrupted header in block %d allocated at %s(%d)",
				pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );

		if ( *(DWORD*)( pBlock+pHeader->m_iSize+sizeof(CSphMemHeader) )!=MEMORY_MAGIC_END )
			sphDie ( "out-of-bounds write beyond block %d allocated at %s(%d)",
				pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );
	}
	g_tAllocsMutex.Unlock();
}

void sphMemStatInit () {}
void sphMemStatDone () {}
void sphMemStatDump ( int ) {}

//////////////////////////////////////////////////////////////////////////

void * operator new ( size_t iSize, const char * sFile, int iLine )
{
	return sphDebugNew ( iSize, sFile, iLine, false );
}


void * operator new [] ( size_t iSize, const char * sFile, int iLine )
{
	return sphDebugNew ( iSize, sFile, iLine, true );
}


void operator delete ( void * pPtr ) MYTHROW()
{
	sphDebugDelete ( pPtr, false );
}


void operator delete [] ( void * pPtr ) MYTHROW()
{
	sphDebugDelete ( pPtr, true );
}

/// debug allocate to use in custom allocator
void * debugallocate ( size_t  iSize )
{
	return sphDebugNew ( iSize, __FILE__, __LINE__, false );
}

/// debug deallocate to use in custom allocator
void debugdeallocate ( void * pPtr )
{
	sphDebugDelete ( pPtr, false);
}


//////////////////////////////////////////////////////////////////////////////
// ALLOCACTIONS COUNT/SIZE PROFILER
//////////////////////////////////////////////////////////////////////////////

#else
#if SPH_ALLOCS_PROFILER

#undef new

static CSphMutex		g_tAllocsMutex;
static int				g_iAllocsId		= 0;
static int				g_iCurAllocs	= 0;
static int64_t			g_iCurBytes		= 0;
static int				g_iTotalAllocs	= 0;
static int				g_iPeakAllocs	= 0;
static int64_t			g_iPeakBytes	= 0;

// statictic's per memory category
struct MemCategorized_t
{
	int64_t	m_iSize;
	int		m_iCount;

	MemCategorized_t()
		: m_iSize ( 0 )
		, m_iCount ( 0 )
	{
	}
};

static MemCategory_e sphMemStatGet ();

// memory categories storage
static MemCategorized_t g_dMemCategoryStat [ MEM_TOTAL ];

//////////////////////////////////////////////////////////////////////////
// ALLOCATIONS COUNT/SIZE PROFILER
//////////////////////////////////////////////////////////////////////////

void * sphDebugNew ( size_t iSize )
{
	BYTE * pBlock = (BYTE*) ::malloc ( iSize+sizeof(size_t)*2 );
	if ( !pBlock )
		sphDie ( "out of memory (unable to allocate %" PRIu64 " bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too

	const int iMemType = sphMemStatGet();
	assert ( iMemType>=0 && iMemType<MEM_TOTAL );

	g_tAllocsMutex.Lock ();

	g_iAllocsId++;
	g_iCurAllocs++;
	g_iCurBytes += iSize;
	g_iTotalAllocs++;
	g_iPeakAllocs = Max ( g_iCurAllocs, g_iPeakAllocs );
	g_iPeakBytes = Max ( g_iCurBytes, g_iPeakBytes );

	g_dMemCategoryStat[iMemType].m_iSize += iSize;
	g_dMemCategoryStat[iMemType].m_iCount++;

	g_tAllocsMutex.Unlock ();

	size_t * pData = (size_t *)pBlock;
	pData[0] = iSize;
	pData[1] = iMemType;

	return pBlock + sizeof(size_t)*2;
}

void sphDebugDelete ( void * pPtr )
{
	if ( !pPtr )
		return;

	size_t * pBlock = (size_t*) pPtr;
	pBlock -= 2;

	const int iSize = pBlock[0];
	const int iMemType = pBlock[1];
	assert ( iMemType>=0 && iMemType<MEM_TOTAL );

	g_tAllocsMutex.Lock ();

	g_iCurAllocs--;
	g_iCurBytes -= iSize;

	g_dMemCategoryStat[iMemType].m_iSize -= iSize;
	g_dMemCategoryStat[iMemType].m_iCount--;

	g_tAllocsMutex.Unlock ();

	::free ( pBlock );
}

void sphAllocsStats ()
{
	g_tAllocsMutex.Lock ();
	fprintf ( stdout, "--- total-allocs=%d, peak-allocs=%d, peak-bytes=" INT64_FMT "\n",
		g_iTotalAllocs, g_iPeakAllocs, g_iPeakBytes );
	g_tAllocsMutex.Unlock ();
}

int64_t sphAllocBytes ()		{ return g_iCurBytes; }
int sphAllocsCount ()			{ return g_iCurAllocs; }
int sphAllocsLastID ()			{ return g_iAllocsId; }
void sphAllocsDump ( int, int )	{}
void sphAllocsCheck ()			{}

void * operator new ( size_t iSize, const char *, int )		{ return sphDebugNew ( iSize ); }
void * operator new [] ( size_t iSize, const char *, int )	{ return sphDebugNew ( iSize ); }
void operator delete ( void * pPtr ) MYTHROW()				{ sphDebugDelete ( pPtr ); }
void operator delete [] ( void * pPtr )	MYTHROW()			{ sphDebugDelete ( pPtr ); }

/// debug allocate to use in custom allocator
void * debugallocate ( size_t iSize )
{
	return sphDebugNew ( iSize );
}

/// debug deallocate to use in custom allocator
void debugdeallocate ( void * pPtr )
{
	sphDebugDelete ( pPtr );
}

//////////////////////////////////////////////////////////////////////////////
// MEMORY STATISTICS
//////////////////////////////////////////////////////////////////////////////

/// TLS key of memory category stack
SphThreadKey_t g_tTLSMemCategory;

STATIC_ASSERT ( MEM_TOTAL<255, TOO_MANY_MEMORY_CATEGORIES );

// stack of memory categories as we move deeper and deeper
class MemCategoryStack_t // NOLINT
{
#define MEM_STACK_MAX 1024
	BYTE m_dStack[MEM_STACK_MAX];
	int m_iDepth;

public:

	// ctor ( cross platform )
	void Reset ()
	{
		m_iDepth = 0;
		m_dStack[0] = MEM_CORE;
	}

	void Push ( MemCategory_e eCategory )
	{
		assert ( eCategory>=0 && eCategory<MEM_TOTAL );
		assert ( m_iDepth+1<MEM_STACK_MAX );
		m_dStack[++m_iDepth] = (BYTE)eCategory;
	}

#ifndef NDEBUG
	void Pop ( MemCategory_e eCategory )
	{
		assert ( eCategory>=0 && eCategory<MEM_TOTAL );
#else
	void Pop ( MemCategory_e )
	{
#endif

		assert ( m_iDepth-1>=0 );
		assert ( m_dStack[m_iDepth]==eCategory );
		m_iDepth--;
	}

	MemCategory_e Top () const
	{
		assert ( m_iDepth>= 0 && m_iDepth<MEM_STACK_MAX );
		assert ( m_dStack[m_iDepth]>=0 && m_dStack[m_iDepth]<MEM_TOTAL );
		return MemCategory_e ( m_dStack[m_iDepth] );
	}
};

static MemCategoryStack_t * g_pMainTLS = NULL; // category stack of main thread

// memory statistic's per thread factory
static MemCategoryStack_t * sphMemStatThdInit ()
{
	MemCategoryStack_t * pTLS = (MemCategoryStack_t *)sphDebugNew ( sizeof ( MemCategoryStack_t ) );
	pTLS->Reset();

	Verify ( sphThreadSet ( g_tTLSMemCategory, pTLS ) );
	return pTLS;
}

// per thread cleanup of memory statistic's
static void sphMemStatThdCleanup ( MemCategoryStack_t * pTLS )
{
	sphDebugDelete ( pTLS );
}

// init of memory statistic's data
static void sphMemStatInit ()
{
	Verify ( sphThreadKeyCreate ( &g_tTLSMemCategory ) );

	// main thread statistic's creation
	assert ( g_pMainTLS==NULL );
	g_pMainTLS = sphMemStatThdInit();
	assert ( g_pMainTLS!=NULL );
}

// cleanup of memory statistic's data
static void sphMemStatDone ()
{
	assert ( g_pMainTLS!=NULL );
	sphMemStatThdCleanup ( g_pMainTLS );

	sphThreadKeyDelete ( g_tTLSMemCategory );
}

// direct access for special category
void sphMemStatMMapAdd ( int64_t iSize )
{
	g_tAllocsMutex.Lock ();

	g_iCurAllocs++;
	g_iCurBytes += iSize;
	g_iTotalAllocs++;
	g_iPeakAllocs = Max ( g_iCurAllocs, g_iPeakAllocs );
	g_iPeakBytes = Max ( g_iCurBytes, g_iPeakBytes );

	g_dMemCategoryStat[MEM_MMAPED].m_iSize += iSize;
	g_dMemCategoryStat[MEM_MMAPED].m_iCount++;

	g_tAllocsMutex.Unlock ();
}

void sphMemStatMMapDel ( int64_t iSize )
{
	g_tAllocsMutex.Lock ();

	g_iCurAllocs--;
	g_iCurBytes -= iSize;

	g_dMemCategoryStat[MEM_MMAPED].m_iSize -= iSize;
	g_dMemCategoryStat[MEM_MMAPED].m_iCount--;

	g_tAllocsMutex.Unlock ();
}

// push new category on arrival
void sphMemStatPush ( MemCategory_e eCategory )
{
	MemCategoryStack_t * pTLS = (MemCategoryStack_t*) sphThreadGet ( g_tTLSMemCategory );
	if ( pTLS )
		pTLS->Push ( eCategory );
};

// restore last category
void sphMemStatPop ( MemCategory_e eCategory )
{
	MemCategoryStack_t * pTLS = (MemCategoryStack_t*) sphThreadGet ( g_tTLSMemCategory );
	if ( pTLS )
		pTLS->Pop ( eCategory );
};

// get current category
static MemCategory_e sphMemStatGet ()
{
	MemCategoryStack_t * pTLS = (MemCategoryStack_t*) sphThreadGet ( g_tTLSMemCategory );
	return pTLS ? pTLS->Top() : MEM_CORE;
}


// human readable category names
#define MEM_CATEGORY(_arg) #_arg
static const char* g_dMemCategoryName[] = { MEM_CATEGORIES };
#undef MEM_CATEGORY


void sphMemStatDump ( int iFD )
{
	int64_t iSize = 0;
	int iCount = 0;
	for ( int i=0; i<MEM_TOTAL; i++ )
	{
		iSize += (int64_t) g_dMemCategoryStat[i].m_iSize;
		iCount += g_dMemCategoryStat[i].m_iCount;
	}

	sphSafeInfo ( iFD, "%-24s allocs-count=%d, mem-total=%d.%d Mb", "(total)", iCount,
		(int)(iSize/1048576), (int)( (iSize*10/1048576)%10 ) );

	for ( int i=0; i<MEM_TOTAL; i++ )
		if ( g_dMemCategoryStat[i].m_iCount>0 )
	{
		iSize = (int64_t) g_dMemCategoryStat[i].m_iSize;
		sphSafeInfo ( iFD, "%-24s allocs-count=%d, mem-total=%d.%d Mb",
			g_dMemCategoryName[i], g_dMemCategoryStat[i].m_iCount,
			(int)(iSize/1048576), (int)( (iSize*10/1048576)%10 ) );
	}
}

//////////////////////////////////////////////////////////////////////////////
// PRODUCTION MEMORY MANAGER
//////////////////////////////////////////////////////////////////////////////

#else
#ifndef SPH_DONT_OVERRIDE_MEMROUTINES

void * operator new ( size_t iSize )
{
	void * pResult = ::malloc ( iSize );
	if ( !pResult )
		sphDieRestart ( "out of memory (unable to allocate " UINT64_FMT " bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too
	return pResult;
}


void * operator new [] ( size_t iSize )
{
	void * pResult = ::malloc ( iSize );
	if ( !pResult )
		sphDieRestart ( "out of memory (unable to allocate " UINT64_FMT " bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too
	return pResult;
}

void operator delete ( void * pPtr ) throw ()
{
	if ( pPtr )
		::free ( pPtr );
}

void operator delete [] ( void * pPtr ) throw ()
{
	if ( pPtr )
		::free ( pPtr );
}

#endif // SPH_DONT_OVERRIDE_MEMROUTINES
#endif // SPH_ALLOCS_PROFILER
#endif // SPH_DEBUG_LEAKS

//////////////////////////////////////////////////////////////////////////

// now let the rest of sphinxstd use proper new
#if SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER
#undef new
#define new		new(__FILE__,__LINE__)
#endif

/////////////////////////////////////////////////////////////////////////////
// HELPERS
/////////////////////////////////////////////////////////////////////////////

static SphDieCallback_t g_pfDieCallback = NULL;


void sphSetDieCallback ( SphDieCallback_t pfDieCallback )
{
	g_pfDieCallback = pfDieCallback;
}


void sphDie ( const char * sTemplate, ... )
{
	char sBuf[1024];

	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf, sizeof(sBuf), sTemplate, ap );
	va_end ( ap );

	// if there's no callback,
	// or if callback returns true,
	// log to stdout
	if ( !g_pfDieCallback || g_pfDieCallback ( sBuf ) )
		fprintf ( stdout, "FATAL: %s\n", sBuf );

	exit ( 1 );
}


void sphDieRestart ( const char * sTemplate, ... )
{
	char sBuf[1024];

	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf, sizeof(sBuf), sTemplate, ap );
	va_end ( ap );

	// if there's no callback,
	// or if callback returns true,
	// log to stdout
	if ( !g_pfDieCallback || g_pfDieCallback ( sBuf ) )
		fprintf ( stdout, "FATAL: %s\n", sBuf );

	exit ( 2 ); // almost CRASH_EXIT
}

//////////////////////////////////////////////////////////////////////////
// RANDOM NUMBERS GENERATOR
//////////////////////////////////////////////////////////////////////////

/// MWC (Multiply-With-Carry) RNG, invented by George Marsaglia
static DWORD g_dRngState[5] = { 0x95d3474bUL, 0x035cf1f7UL, 0xfd43995fUL, 0x5dfc55fbUL, 0x334a9229UL };


/// seed
void sphSrand ( DWORD uSeed )
{
	for ( int i=0; i<5; i++ )
	{
		uSeed = uSeed*29943829 - 1;
		g_dRngState[i] = uSeed;
	}
	for ( int i=0; i<19; i++ )
		sphRand();
}


/// auto-seed RNG based on time and PID
void sphAutoSrand ()
{
	// get timestamp
#if !USE_WINDOWS
	struct timeval tv;
	gettimeofday ( &tv, NULL );
#else
	#define getpid() GetCurrentProcessId()

	struct
	{
		time_t	tv_sec;
		DWORD	tv_usec;
	} tv;

	FILETIME ft;
	GetSystemTimeAsFileTime ( &ft );

	uint64_t ts = ( uint64_t(ft.dwHighDateTime)<<32 ) + uint64_t(ft.dwLowDateTime) - 116444736000000000ULL; // Jan 1, 1970 magic
	ts /= 10; // to microseconds
	tv.tv_sec = (DWORD)(ts/1000000);
	tv.tv_usec = (DWORD)(ts%1000000);
#endif

	// twist and shout
	sphSrand ( sphRand() ^ DWORD(tv.tv_sec) ^ (DWORD(tv.tv_usec) + DWORD(getpid())) );
}


/// generate another dword
DWORD sphRand ()
{
	uint64_t uSum;
	uSum =
		(uint64_t)g_dRngState[0] * (uint64_t)5115 +
		(uint64_t)g_dRngState[1] * (uint64_t)1776 +
		(uint64_t)g_dRngState[2] * (uint64_t)1492 +
		(uint64_t)g_dRngState[3] * (uint64_t)2111111111UL +
		(uint64_t)g_dRngState[4];
	g_dRngState[3] = g_dRngState[2];
	g_dRngState[2] = g_dRngState[1];
	g_dRngState[1] = g_dRngState[0];
	g_dRngState[4] = (DWORD)( uSum>>32 );
	g_dRngState[0] = (DWORD)uSum;
	return g_dRngState[0];
}

//////////////////////////////////////////////////////////////////////////
// THREADING FUNCTIONS
//////////////////////////////////////////////////////////////////////////

// This is a working context for a thread wrapper. It wraps every thread to
// store information about it's stack size, cleanup threads and something else.
// This struct always should be allocated in the heap, cause wrapper need
// to see it all the time and it frees it out of the heap by itself. Wrapper thread function
// receives as an argument a pointer to ThreadCall_t with one function pointer to
// a main thread function. Afterwards, thread can set up one or more cleanup functions
// which will be executed by a wrapper in the linked list order after it dies.
struct ThreadCall_t
{
	void			( *m_pCall )( void * pArg );
	void *			m_pArg;
#if USE_GPROF
	pthread_mutex_t	m_dlock;
	pthread_cond_t	m_dwait;
	itimerval		m_ditimer;
#endif
	union {
		ThreadCall_t *	m_pNext; 		// used in exit chain
		char 			m_sName[16];	// used in main thread
	};
};
static SphThreadKey_t g_tThreadCleanupKey;
static SphThreadKey_t g_tMyThreadStack;


#if USE_WINDOWS
#define SPH_THDFUNC DWORD __stdcall
#else
#define SPH_THDFUNC void *
#endif

SPH_THDFUNC sphThreadProcWrapper ( void * pArg )
{
	// This is the first local variable in the new thread. So, its address is the top of the stack.
	// We need to know thread stack size for both expression and query evaluating engines.
	// We store expressions as a linked tree of structs and execution is a calls of mutually
	// recursive methods. Before executing we compute tree height and multiply it by a constant
	// with experimentally measured value to check whether we have enough stack to execute current query.
	// The check is not ideal and do not work for all compilers and compiler settings.
	char	cTopOfMyStack;
	assert ( sphThreadGet ( g_tThreadCleanupKey )==NULL );
	assert ( sphThreadGet ( g_tMyThreadStack )==NULL );

#if SPH_ALLOCS_PROFILER
	MemCategoryStack_t * pTLS = sphMemStatThdInit();
#endif

#if USE_GPROF
	// Set the profile timer value
	setitimer ( ITIMER_PROF, &( (ThreadCall_t*) pArg )->m_ditimer, NULL );

	// Tell the calling thread that we don't need its data anymore
	pthread_mutex_lock ( &( (ThreadCall_t*) pArg)->m_dlock );
	pthread_cond_signal ( &( (ThreadCall_t*) pArg)->m_dwait );
	pthread_mutex_unlock ( &( (ThreadCall_t*) pArg)->m_dlock );
#endif

	ThreadCall_t * pCall = (ThreadCall_t*) pArg;
	MemorizeStack ( & cTopOfMyStack );

	// set name of self
#if HAVE_PTHREAD_SETNAME_NP
	if ( pCall->m_sName[0]!='\0' )
	{
		assert ( strlen ( pCall->m_sName )<16 );
#if HAVE_PTHREAD_SETNAME_NP_1ARG
		pthread_setname_np ( pCall->m_sName );
#else
		pthread_setname_np ( pthread_self(), pCall->m_sName );
#endif
	}
#endif

	pCall->m_pCall ( pCall->m_pArg );
	SafeDelete ( pCall );

	ThreadCall_t * pCleanup = (ThreadCall_t*) sphThreadGet ( g_tThreadCleanupKey );
	while ( pCleanup )
	{
		pCall = pCleanup;
		pCall->m_pCall ( pCall->m_pArg );
		pCleanup = pCall->m_pNext;
		SafeDelete ( pCall );
	}

#if SPH_ALLOCS_PROFILER
	sphMemStatThdCleanup ( pTLS );
#endif

	return 0;
}

#if !USE_WINDOWS
void * sphThreadInit ( bool bDetached )
#else
void * sphThreadInit ( bool )
#endif
{
	static bool bInit = false;
#if !USE_WINDOWS
	static pthread_attr_t tJoinableAttr;
	static pthread_attr_t tDetachedAttr;
#endif

	if ( !bInit )
	{
#if SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER
		sphMemStatInit();
#endif

		// we're single-threaded yet, right?!
		if ( !sphThreadKeyCreate ( &g_tThreadCleanupKey ) )
			sphDie ( "FATAL: sphThreadKeyCreate() failed" );

		if ( !sphThreadKeyCreate ( &g_tMyThreadStack ) )
			sphDie ( "FATAL: sphThreadKeyCreate() failed" );

#if !USE_WINDOWS
		if ( pthread_attr_init ( &tJoinableAttr ) )
			sphDie ( "FATAL: pthread_attr_init( joinable ) failed" );

		if ( pthread_attr_init ( &tDetachedAttr ) )
			sphDie ( "FATAL: pthread_attr_init( detached ) failed" );

		if ( pthread_attr_setdetachstate ( &tDetachedAttr, PTHREAD_CREATE_DETACHED ) )
			sphDie ( "FATAL: pthread_attr_setdetachstate( detached ) failed" );
#endif
		bInit = true;
	}
#if !USE_WINDOWS
	if ( pthread_attr_setstacksize ( &tJoinableAttr, g_iThreadStackSize + PTHREAD_STACK_MIN ) )
		sphDie ( "FATAL: pthread_attr_setstacksize( joinable ) failed" );

	if ( pthread_attr_setstacksize ( &tDetachedAttr, g_iThreadStackSize + PTHREAD_STACK_MIN ) )
		sphDie ( "FATAL: pthread_attr_setstacksize( detached ) failed" );

	return bDetached ? &tDetachedAttr : &tJoinableAttr;
#else
	return NULL;
#endif
}


#if SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER
void sphThreadDone ( int iFD )
{
	sphMemStatDump ( iFD );
	sphMemStatDone();
}
#else
void sphThreadDone ( int )
{
}
#endif


bool sphThreadCreate ( SphThread_t * pThread, void (*fnThread)(void*), void * pArg, bool bDetached, const char * sName )
{
	// we can not put this on current stack because wrapper need to see
	// it all the time and it will destroy this data from heap by itself
	ThreadCall_t * pCall = new ThreadCall_t;
	pCall->m_pCall = fnThread;
	pCall->m_pArg = pArg;
	pCall->m_pNext = nullptr;

#if HAVE_PTHREAD_SETNAME_NP
	if ( sName )
		strncpy ( pCall->m_sName, sName, 15 );
	pCall->m_sName[15] = '\0';
#endif

	// create thread
#if USE_WINDOWS
	sphThreadInit ( bDetached );
	*pThread = CreateThread ( NULL, g_iThreadStackSize, sphThreadProcWrapper, pCall, 0, NULL );
	if ( *pThread )
		return true;
#else

#if USE_GPROF
	getitimer ( ITIMER_PROF, &pCall->m_ditimer );
	pthread_cond_init ( &pCall->m_dwait, NULL );
	pthread_mutex_init ( &pCall->m_dlock, NULL );
	pthread_mutex_lock ( &pCall->m_dlock );
#endif

	void * pAttr = sphThreadInit ( bDetached );
	errno = pthread_create ( pThread, (pthread_attr_t*) pAttr, sphThreadProcWrapper, pCall );

#if USE_GPROF
	if ( !errno )
		pthread_cond_wait ( &pCall->m_dwait, &pCall->m_dlock );

	pthread_mutex_unlock ( &pCall->m_dlock );
	pthread_mutex_destroy ( &pCall->m_dlock );
	pthread_cond_destroy ( &pCall->m_dwait );
#endif

	if ( !errno )
		return true;

#endif

	// thread creation failed so we need to cleanup ourselves
	SafeDelete ( pCall );
	return false;
}

CSphString GetThreadName ( SphThread_t * pThread )
{
	if ( !pThread || !*pThread )
		return "";

#if HAVE_PTHREAD_GETNAME_NP
	char sClippedName[16];
	pthread_getname_np ( *pThread, sClippedName, 16 );
	return sClippedName;
#else
	return "";
#endif
}


bool sphThreadJoin ( SphThread_t * pThread )
{
#if USE_WINDOWS
	DWORD uWait = WaitForSingleObject ( *pThread, INFINITE );
	CloseHandle ( *pThread );
	*pThread = NULL;
	return ( uWait==WAIT_OBJECT_0 || uWait==WAIT_ABANDONED );
#else
	return pthread_join ( *pThread, NULL )==0;
#endif
}

// Adds a function call (a new task for a wrapper) to a linked list
// of thread contexts. They will be executed one by one right after
// the main thread ends its execution. This is a way for a wrapper
// to free local resources allocated by its main thread.
void sphThreadOnExit ( void (*fnCleanup)(void*), void * pArg )
{
	auto pCleanup = new ThreadCall_t;
	pCleanup->m_pCall = fnCleanup;
	pCleanup->m_pArg = pArg;
	pCleanup->m_pNext = (ThreadCall_t*) sphThreadGet ( g_tThreadCleanupKey );
	sphThreadSet ( g_tThreadCleanupKey, pCleanup );
}


bool sphThreadKeyCreate ( SphThreadKey_t * pKey )
{
#if USE_WINDOWS
	*pKey = TlsAlloc();
	return *pKey!=TLS_OUT_OF_INDEXES;
#else
	return pthread_key_create ( pKey, NULL )==0;
#endif
}


void sphThreadKeyDelete ( SphThreadKey_t tKey )
{
#if USE_WINDOWS
	TlsFree ( tKey );
#else
	pthread_key_delete ( tKey );
#endif
}


void * sphThreadGet ( SphThreadKey_t tKey )
{
#if USE_WINDOWS
	return TlsGetValue ( tKey );
#else
	return pthread_getspecific ( tKey );
#endif
}

void * sphMyStack ()
{
	return sphThreadGet ( g_tMyThreadStack );
}


int64_t sphGetStackUsed()
{
	BYTE cStack;
	BYTE * pStackTop = (BYTE*)sphMyStack();
	if ( !pStackTop )
		return 0;
	int64_t iHeight = pStackTop - &cStack;
	return ( iHeight>=0 ) ? iHeight : -iHeight;
}

void sphSetMyStackSize ( int iStackSize )
{
	g_iThreadStackSize = iStackSize;
	sphThreadInit ( false );
}


void MemorizeStack ( void* PStack )
{
	sphThreadSet ( g_tMyThreadStack, PStack );
}


bool sphThreadSet ( SphThreadKey_t tKey, void * pValue )
{
#if USE_WINDOWS
	return TlsSetValue ( tKey, pValue )!=FALSE;
#else
	return pthread_setspecific ( tKey, pValue )==0;
#endif
}

#if !USE_WINDOWS
bool sphIsLtLib()
{
#ifndef _CS_GNU_LIBPTHREAD_VERSION
	return false;
#else
	char buff[64];
	confstr ( _CS_GNU_LIBPTHREAD_VERSION, buff, 64 );

	if ( !strncasecmp ( buff, "linuxthreads", 12 ) )
		return true;
	return false;
#endif
}
#endif

//////////////////////////////////////////////////////////////////////////
// MUTEX and EVENT
//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS

// Windows mutex implementation

CSphMutex::CSphMutex ()
{
	m_hMutex = CreateMutex ( NULL, FALSE, NULL );
	if ( !m_hMutex )
		sphDie ( "CreateMutex() failed" );
}

CSphMutex::~CSphMutex ()
{
	if ( CloseHandle ( m_hMutex )==FALSE )
		sphDie ( "CloseHandle() failed" );
}

bool CSphMutex::Lock ()
{
	DWORD uWait = WaitForSingleObject ( m_hMutex, INFINITE );
	return ( uWait!=WAIT_FAILED && uWait!=WAIT_TIMEOUT );
}

bool CSphMutex::TimedLock ( int iMsec )
{
	DWORD uWait = WaitForSingleObject ( m_hMutex, iMsec );
	return ( uWait!=WAIT_FAILED && uWait!=WAIT_TIMEOUT );
}

bool CSphMutex::Unlock ()
{
	return ReleaseMutex ( m_hMutex )==TRUE;
}

EventWrapper_c::EventWrapper_c ()
{
	m_hEvent = CreateEvent ( NULL, TRUE, FALSE, NULL );
	m_bInitialized = ( m_hEvent!=0 );
}

EventWrapper_c::~EventWrapper_c ()
{
	if ( !m_bInitialized )
		return;

	m_bInitialized = false;
	CloseHandle ( m_hEvent );
}

template<>
void AutoEvent_T<true>::SetEvent()
{
	m_iSent = 1;
	::SetEvent ( m_hEvent );
}

template<>
void AutoEvent_T<false>::SetEvent ()
{
	++m_iSent;
	::SetEvent ( m_hEvent );
}

template<>
bool AutoEvent_T<true>::WaitEvent ( int iMsec )
{
	if ( !m_bInitialized )
		return false;

	if ( !m_iSent )
	{
		DWORD iTime=( iMsec == -1 )?INFINITE:iMsec;
		auto iRes=WaitForSingleObject ( m_hEvent, iTime );
		if ( iRes == WAIT_TIMEOUT )
			return false;
	}

	ResetEvent ( m_hEvent );
	m_iSent=0;
	return true;
}

template<>
bool AutoEvent_T<false>::WaitEvent ( int iMsec )
{
	if ( !m_bInitialized )
		return false;

	if ( !m_iSent )
	{
		DWORD iTime=( iMsec == -1 )?INFINITE:iMsec;
		auto iRes=WaitForSingleObject ( m_hEvent, iTime );
		if ( iRes == WAIT_TIMEOUT )
			return false;
	}

	ResetEvent ( m_hEvent );
	--m_iSent;
	return true;
}

#else

// UNIX mutex implementation

CSphMutex::CSphMutex()
{
	if ( pthread_mutex_init ( &m_tMutex, NULL ) )
		sphDie ( "pthread_mutex_init() failed %s", strerrorm ( errno ) );
}

CSphMutex::~CSphMutex()
{
	if ( pthread_mutex_destroy ( &m_tMutex ) )
		sphDie ( "pthread_mutex_destroy() failed %s", strerrorm ( errno ) );
}

bool CSphMutex::Lock ()
{
	return ( pthread_mutex_lock ( &m_tMutex )==0 );
}

bool CSphMutex::TimedLock ( int iMsec )
{
// pthread_mutex_timedlock is not available on Mac Os. Fallback to lock without a timer.
#if defined (HAVE_PTHREAD_MUTEX_TIMEDLOCK)
	struct timespec ts;
	clock_gettime ( CLOCK_REALTIME, &ts );

	int ns = ts.tv_nsec + ( iMsec % 1000 )*1000000;
	ts.tv_sec += ( ns / 1000000000 ) + ( iMsec / 1000 );
	ts.tv_nsec = ( ns % 1000000000 );

	int iRes = pthread_mutex_timedlock ( &m_tMutex, &ts );
	return iRes==0;

#else
	int iRes = EBUSY;
	int64_t tmTill = sphMicroTimer () + iMsec * 1000;
	do
	{
		iRes = pthread_mutex_trylock ( &m_tMutex );
		if ( iRes!=EBUSY )
			break;
		// below is inlined sphSleepMsec(1) - placed here to avoid dependency from libsphinx (for wordbreaker)
#if USE_WINDOWS
		Sleep ( 1 );
#else
		struct timeval tvTimeout;
		tvTimeout.tv_sec = 0;
		tvTimeout.tv_usec = 1000;
		select ( 0, NULL, NULL, NULL, &tvTimeout );
#endif

	} while ( sphMicroTimer ()<tmTill );
	if ( iRes==EBUSY )
		iRes = pthread_mutex_trylock ( &m_tMutex );

	return iRes!=EBUSY;

#endif
}

bool CSphMutex::Unlock ()
{
	return ( pthread_mutex_unlock ( &m_tMutex )==0 );
}

EventWrapper_c::EventWrapper_c ()
{
	m_bInitialized = ( pthread_mutex_init ( &m_tMutex, nullptr )==0 );
	m_bInitialized &= ( pthread_cond_init ( &m_tCond, nullptr )==0 );
}

EventWrapper_c::~EventWrapper_c ()
{
	if ( !m_bInitialized )
		return;

	pthread_cond_destroy ( &m_tCond );
	pthread_mutex_destroy ( &m_tMutex );
}

template<>
void AutoEvent_T<false>::SetEvent ()
{
	if ( !m_bInitialized )
		return;

	pthread_mutex_lock ( &m_tMutex );
	++m_iSent;
	pthread_cond_signal ( &m_tCond );
	pthread_mutex_unlock ( &m_tMutex );
}

template <>
void AutoEvent_T<true>::SetEvent ()
{
	if ( !m_bInitialized )
		return;

	pthread_mutex_lock ( &m_tMutex );
	m_iSent=1;
	pthread_cond_signal ( &m_tCond );
	pthread_mutex_unlock ( &m_tMutex );
}

template <>
bool AutoEvent_T<false>::WaitEvent ( int iMsec )
{
	if ( !m_bInitialized )
		return false;

	if ( iMsec==-1 )
	{
		pthread_mutex_lock ( &m_tMutex );
		while (  !m_iSent )
			pthread_cond_wait ( &m_tCond, &m_tMutex );

		--m_iSent;
		pthread_mutex_unlock ( &m_tMutex );
		return true;
	}

#ifdef HAVE_PTHREAD_COND_TIMEDWAIT
	struct timespec ts;
	clock_gettime ( CLOCK_REALTIME, &ts );

	int ns = ts.tv_nsec + ( iMsec % 1000 ) * 1000000;
	ts.tv_sec += ( ns / 1000000000 ) + ( iMsec / 1000 );
	ts.tv_nsec = ( ns % 1000000000 );

	int iRes = 0;
	pthread_mutex_lock ( &m_tMutex );
	while ( !m_iSent && !iRes )
		iRes = pthread_cond_timedwait ( &m_tCond, &m_tMutex, &ts );

	--m_iSent;
	pthread_mutex_unlock ( &m_tMutex );
	return iRes!=ETIMEDOUT;
#endif
}

template <>
bool AutoEvent_T<true>::WaitEvent ( int iMsec )
{
	if ( !m_bInitialized )
		return false;

	if ( iMsec == -1 )
	{
		pthread_mutex_lock ( &m_tMutex );
		while ( !m_iSent )
			pthread_cond_wait ( &m_tCond, &m_tMutex );

		m_iSent = 0;
		pthread_mutex_unlock ( &m_tMutex );
		return true;
	}

#ifdef HAVE_PTHREAD_COND_TIMEDWAIT
	struct timespec ts;
	clock_gettime ( CLOCK_REALTIME, &ts );

	int ns = ts.tv_nsec + ( iMsec % 1000 ) * 1000000;
	ts.tv_sec += ( ns / 1000000000 ) + ( iMsec / 1000 );
	ts.tv_nsec = ( ns % 1000000000 );

	int iRes = 0;
	pthread_mutex_lock ( &m_tMutex );
	while ( !m_iSent && !iRes )
		iRes = pthread_cond_timedwait ( &m_tCond, &m_tMutex, &ts );

	m_iSent=0;
	pthread_mutex_unlock ( &m_tMutex );
	return iRes!=ETIMEDOUT;
#endif
}

#endif

//////////////////////////////////////////////////////////////////////////
// RWLOCK
//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS

// Windows rwlock implementation

CSphRwlock::CSphRwlock ()
{}


bool CSphRwlock::Init ( bool )
{
	assert ( !m_bInitialized );
	assert ( !m_hWriteMutex && !m_hReadEvent && !m_iReaders );

	m_hReadEvent = CreateEvent ( NULL, TRUE, FALSE, NULL );
	if ( !m_hReadEvent )
		return false;

	m_hWriteMutex = CreateMutex ( NULL, FALSE, NULL );
	if ( !m_hWriteMutex )
	{
		CloseHandle ( m_hReadEvent );
		m_hReadEvent = NULL;
		return false;
	}
	m_bInitialized = true;
	return true;
}


bool CSphRwlock::Done ()
{
	if ( !m_bInitialized )
		return true;

	if ( !CloseHandle ( m_hReadEvent ) )
		return false;
	m_hReadEvent = NULL;

	if ( !CloseHandle ( m_hWriteMutex ) )
		return false;
	m_hWriteMutex = NULL;

	m_iReaders = 0;
	m_bInitialized = false;
	return true;
}


bool CSphRwlock::ReadLock ()
{
	assert ( m_bInitialized );

	DWORD uWait = WaitForSingleObject ( m_hWriteMutex, INFINITE );
	if ( uWait==WAIT_FAILED || uWait==WAIT_TIMEOUT )
		return false;

	// got the writer mutex, can't be locked for write
	// so it's OK to add the reader lock, then free the writer mutex
	// writer mutex also protects readers counter
	InterlockedIncrement ( &m_iReaders );

	// reset writer lock event, we just got ourselves a reader
	if ( !ResetEvent ( m_hReadEvent ) )
		return false;

	// release writer lock
	return ReleaseMutex ( m_hWriteMutex )==TRUE;
}


bool CSphRwlock::WriteLock ()
{
	assert ( m_bInitialized );

	// try to acquire writer mutex
	DWORD uWait = WaitForSingleObject ( m_hWriteMutex, INFINITE );
	if ( uWait==WAIT_FAILED || uWait==WAIT_TIMEOUT )
		return false;

	// got the writer mutex, no pending readers, rock'n'roll
	if ( !m_iReaders )
		return true;

	// got the writer mutex, but still have to wait for all readers to complete
	uWait = WaitForSingleObject ( m_hReadEvent, INFINITE );
	if ( uWait==WAIT_FAILED || uWait==WAIT_TIMEOUT )
	{
		// wait failed, well then, release writer mutex
		ReleaseMutex ( m_hWriteMutex );
		return false;
	}
	return true;
}


bool CSphRwlock::Unlock ()
{
	assert ( m_bInitialized );

	// are we unlocking a writer?
	if ( ReleaseMutex ( m_hWriteMutex ) )
		return true; // yes we are

	if ( GetLastError()!=ERROR_NOT_OWNER )
		return false; // some unexpected error

	// writer mutex wasn't mine; we must have a read lock
	if ( !m_iReaders )
		return true; // could this ever happen?

	// atomically decrement reader counter
	if ( InterlockedDecrement ( &m_iReaders ) )
		return true; // there still are pending readers

	// no pending readers, fire the event for write lock
	return SetEvent ( m_hReadEvent )==TRUE;
}

#else

// UNIX rwlock implementation (pthreads wrapper)

CSphRwlock::CSphRwlock ()
{
	m_pLock = new pthread_rwlock_t;
}

bool CSphRwlock::Init ( bool bPreferWriter )
{
	assert ( !m_bInitialized );
	assert ( m_pLock );

	pthread_rwlockattr_t tAttr;
	pthread_rwlockattr_t * pAttr = nullptr;

	if ( bPreferWriter )
	{
		bool bOk = (pthread_rwlockattr_init ( &tAttr )==0);
		assert ( bOk );

		if ( bOk )
		{

#if HAVE_RWLOCK_PREFER_WRITER
			bOk = ( pthread_rwlockattr_setkind_np ( &tAttr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP )==0 );
#else
			// Mac OS X knows nothing about PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP
			m_pWritePreferHelper = new CSphMutex();
#endif
			assert ( bOk );

			if ( !bOk )
				pthread_rwlockattr_destroy ( &tAttr );
			else
				pAttr = &tAttr;
		}
	}
	m_bInitialized = ( pthread_rwlock_init ( m_pLock, pAttr )==0 );

	if ( pAttr )
		pthread_rwlockattr_destroy ( &tAttr );

	return m_bInitialized;
}

bool CSphRwlock::Done ()
{
	assert ( m_pLock );
	if ( !m_bInitialized )
		return true;

	m_bInitialized = pthread_rwlock_destroy ( m_pLock )!=0;
	return !m_bInitialized;
}

bool CSphRwlock::ReadLock ()
{
	assert ( m_bInitialized );
	assert ( m_pLock );

	if ( !m_pWritePreferHelper )
		return pthread_rwlock_rdlock ( m_pLock )==0;

	CSphScopedLock<CSphMutex> tScopedLock (*m_pWritePreferHelper);
	return pthread_rwlock_rdlock ( m_pLock )==0;
}

bool CSphRwlock::WriteLock ()
{
	assert ( m_bInitialized );
	assert ( m_pLock );

	if ( !m_pWritePreferHelper )
		return pthread_rwlock_wrlock ( m_pLock )==0;

	CSphScopedLock<CSphMutex> tScopedLock(*m_pWritePreferHelper);
	return pthread_rwlock_wrlock ( m_pLock )==0;
}

bool CSphRwlock::Unlock ()
{
	assert ( m_bInitialized );
	assert ( m_pLock );

	return pthread_rwlock_unlock ( m_pLock )==0;
}

#endif

//////////////////////////////////////////////////////////////////////////

/// microsecond precision timestamp
int64_t sphMicroTimer()
{
#if USE_WINDOWS
	// Windows time query
	static int64_t iBase = 0;
	static int64_t iStart = 0;
	static int64_t iFreq = 0;

	LARGE_INTEGER iLarge;
	if ( !iBase )
	{
		// get start QPC value
		QueryPerformanceFrequency ( &iLarge ); iFreq = iLarge.QuadPart;
		QueryPerformanceCounter ( &iLarge ); iStart = iLarge.QuadPart;

		// get start UTC timestamp
		// assuming it's still approximately the same moment as iStart, give or take a msec or three
		FILETIME ft;
		GetSystemTimeAsFileTime ( &ft );

		iBase = ( int64_t(ft.dwHighDateTime)<<32 ) + int64_t(ft.dwLowDateTime);
		iBase = ( iBase - 116444736000000000ULL ) / 10; // rebase from 01 Jan 1601 to 01 Jan 1970, and rescale to 1 usec from 100 ns
	}

	// we can't easily drag iBase into parens because iBase*iFreq/1000000 overflows 64bit int!
	QueryPerformanceCounter ( &iLarge );
	return iBase + ( iLarge.QuadPart - iStart )*1000000/iFreq;

#else
	// UNIX time query
	struct timeval tv;
	gettimeofday ( &tv, NULL );
	return int64_t(tv.tv_sec)*int64_t(1000000) + int64_t(tv.tv_usec);
#endif // USE_WINDOWS
}

//////////////////////////////////////////////////////////////////////////

int CSphStrHashFunc::Hash ( const CSphString & sKey )
{
	return sKey.IsEmpty() ? 0 : sphCRC32 ( sKey.cstr() );
}

//////////////////////////////////////////////////////////////////////////

DWORD g_dSphinxCRC32 [ 256 ] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};


DWORD sphCRC32 ( const void * s )
{
	// calc CRC
	DWORD crc = ~((DWORD)0);
	for ( const BYTE * p=(const BYTE*)s; *p; p++ )
		crc = (crc >> 8) ^ g_dSphinxCRC32 [ (crc ^ (*p)) & 0xff ];
	return ~crc;
}

DWORD sphCRC32 ( const void * s, int iLen )
{
	// calc CRC
	DWORD crc = ~((DWORD)0);
	const BYTE * p = (const BYTE*)s;
	const BYTE * pMax = p + iLen;
	while ( p<pMax )
		crc = (crc >> 8) ^ g_dSphinxCRC32 [ (crc ^ *p++) & 0xff ];
	return ~crc;
}

DWORD sphCRC32 ( const void * s, int iLen, DWORD uPrevCRC )
{
	// calc CRC
	DWORD crc = ~((DWORD)uPrevCRC);
	const BYTE * p = (const BYTE*)s;
	const BYTE * pMax = p + iLen;
	while ( p<pMax )
		crc = (crc >> 8) ^ g_dSphinxCRC32 [ (crc ^ *p++) & 0xff ];
	return ~crc;
}

#if USE_WINDOWS
template<> long CSphAtomic_T<long>::Inc ()
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedIncrement ( &m_iValue )-1;
}

template<> int64_t CSphAtomic_T<int64_t>::Inc ()
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedIncrement64 ( &m_iValue )-1;
}

template<> long CSphAtomic_T<long>::Dec ()
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedDecrement ( &m_iValue )+1;
}

template<> int64_t CSphAtomic_T<int64_t>::Dec ()
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedDecrement64 ( &m_iValue )+1;
}

template<> long CSphAtomic_T<long>::Add ( long iValue )
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedExchangeAdd ( &m_iValue, iValue );
}

template<> int64_t CSphAtomic_T<int64_t>::Add ( int64_t iValue )
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedExchangeAdd64 ( &m_iValue, iValue );
}

template<> long CSphAtomic_T<long>::Sub ( long iValue )
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedExchangeAdd ( &m_iValue, -iValue );
}

template<> int64_t CSphAtomic_T<int64_t>::Sub ( int64_t iValue )
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedExchangeAdd64 ( &m_iValue, -iValue );
}

template<> void CSphAtomic_T<long>::SetValue ( long iValue )
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	InterlockedExchange ( &m_iValue, iValue );
}

template<> void CSphAtomic_T<int64_t>::SetValue ( int64_t iValue )
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	InterlockedExchange64 ( &m_iValue, iValue );
}

template<> unsigned int CSphAtomic_T<unsigned int>::CAS ( unsigned int uOldVal, unsigned int uNewVal )
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedCompareExchange ( &m_iValue, uNewVal, uOldVal );
}

template<> long CSphAtomic_T<long>::CAS ( long iOldVal, long iNewVal )
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedCompareExchange ( &m_iValue, iNewVal, iOldVal );
}

template<> int64_t CSphAtomic_T<int64_t>::CAS ( int64_t iOldVal, int64_t iNewVal )
{
	assert ( ( ( (size_t) &m_iValue )%( sizeof ( m_iValue ) ) )==0 && "unaligned atomic!" );
	return InterlockedCompareExchange64 ( &m_iValue, iNewVal, iOldVal );
}

#endif

// fast check if we are built with right endianess settings
const char*		sphCheckEndian()
{
	const char* sErrorMsg = "Oops! It seems that manticore was built with wrong endianess (cross-compiling?)\n"
#if USE_LITTLE_ENDIAN
		"either reconfigure and rebuild, defining ac_cv_c_bigendian=yes in the environment of ./configure script,\n"
		"either ensure that '#define USE_LITTLE_ENDIAN = 0' in config/config.h\n";
#else
		"either reconfigure and rebuild, defining ac_cv_c_bigendian=no in the environment of ./configure script,\n"
		"either ensure that '#define USE_LITTLE_ENDIAN = 1' in config/config.h\n";
#endif

	char sMagic[] = "\x01\x02\x03\x04\x05\x06\x07\x08";
	unsigned long *pMagic;
	unsigned long uResult;
	pMagic = (unsigned long*)sMagic;
	uResult = 0xFFFFFFFF & (*pMagic);
#if USE_LITTLE_ENDIAN
	if ( uResult==0x01020304 || uResult==0x05060708 )
#else
	if ( uResult==0x08070605 || uResult==0x04030201 )
#endif
		return sErrorMsg;
	return NULL;
}

struct ThdJob_t
{
	ISphJob *	m_pItem = nullptr;
	ThdJob_t *	m_pNext = nullptr;
	ThdJob_t *	m_pPrev = nullptr;

	~ThdJob_t ()
	{
		SafeDelete ( m_pItem );
	}
};

#ifdef USE_VTUNE
#include "ittnotify.h"
static void SetThdName ( const char * )
{
	__itt_thread_set_name ( "job" );
}
#else
static void SetThdName ( const char * ) {}
#endif

class CSphThdPool : public ISphThdPool
{
	CSphAutoEvent					m_tWakeup;
	CSphMutex						m_tJobLock;

	CSphFixedVector<SphThread_t>	m_dWorkers { 0 };
	ThdJob_t *						m_pHead GUARDED_BY ( m_tJobLock ) = nullptr;
	ThdJob_t *						m_pTail GUARDED_BY ( m_tJobLock ) = nullptr;

	volatile bool					m_bShutdown = false;

	CSphAtomic						m_tStatActiveWorkers;
	int								m_iStatQueuedJobs = 0;

	CSphString						m_sName;

public:
	CSphThdPool ( int iThreads, const char * sName, CSphString & sError )
		: m_sName ( sName )
	{
		if ( !m_tWakeup.Initialized () )
		{
			m_bShutdown = true;
			sError.SetSprintf ( "thread-pool create failed %s", strerrorm ( errno ) );
			return;
		}

		iThreads = Max ( iThreads, 1 );
		m_dWorkers.Reset ( iThreads );
		int iStarted = 0;
		for ( auto& dWorker : m_dWorkers )
		{
			if ( sphThreadCreate ( &dWorker, Tick, this, false,
				Str_b ().Sprintf ( "%s_%d", m_sName.cstr (), iStarted ).cstr() ) )
				++iStarted;
		}
		assert ( iStarted == iThreads );
	}

	~CSphThdPool () final
	{
		Shutdown();
	}

	void Shutdown () NO_THREAD_SAFETY_ANALYSIS final
	{
		if ( m_bShutdown )
			return;

		m_bShutdown = true;

		ARRAY_FOREACH ( i, m_dWorkers )
			m_tWakeup.SetEvent();

		ARRAY_FOREACH ( i, m_dWorkers )
			sphThreadJoin ( m_dWorkers.Begin()+i );

		while ( m_pHead && m_pHead!=m_pTail )
		{
			ThdJob_t * pNext = m_pHead->m_pNext;
			SafeDelete ( m_pHead );
			m_pHead = pNext;
		}
	}

	void AddJob ( ISphJob * pItem ) final
	{
		assert ( pItem );
		assert ( !m_bShutdown );

		auto * pJob = new ThdJob_t;
		pJob->m_pItem = pItem;

		m_tJobLock.Lock();

		if ( !m_pHead )
		{
			m_pHead = pJob;
			m_pTail = pJob;
		} else
		{
			pJob->m_pNext = m_pHead;
			m_pHead->m_pPrev = pJob;
			m_pHead = pJob;
		}

		++m_iStatQueuedJobs;
		m_tJobLock.Unlock();

		m_tWakeup.SetEvent();
	}

	bool StartJob ( ISphJob * pItem ) final
	{
		// FIXME!!! start thread only in case of no workers available to offload call site
		SphThread_t tThd;
		return sphThreadCreate ( &tThd, Start, pItem, true, ( Str_b () << m_sName << "_Job" ).cstr () );
	}

private:
	static void Tick ( void * pArg )
	{
		SetThdName ( "job" );

		auto * pPool = (CSphThdPool *)pArg;
		pPool->TickImpl();
	}

	void TickImpl()
	{
		while ( !m_bShutdown )
		{
			m_tWakeup.WaitEvent ();

			if ( m_bShutdown )
				break;

			m_tJobLock.Lock ();

			ThdJob_t * pJob = m_pTail;
			if ( m_pHead==m_pTail ) // either 0 or 1 job case
			{
				m_pHead = m_pTail = nullptr;
			} else
			{
				pJob->m_pPrev->m_pNext = nullptr;
				m_pTail = pJob->m_pPrev;
			}

			if ( pJob )
				--m_iStatQueuedJobs;

			m_tJobLock.Unlock ();

			if ( !pJob )
				continue;

			++m_tStatActiveWorkers;

			pJob->m_pItem->Call ();
			SafeDelete ( pJob );

			--m_tStatActiveWorkers;

			// FIXME!!! work stealing case (check another job prior going to sem)
		}
	}

	static void Start ( void * pArg )
	{
		auto * pJob = (ISphJob *)pArg;
		if ( pJob )
			pJob->Call();
		SafeDelete ( pJob );
	}

	int GetActiveWorkerCount () const final
	{
		return m_tStatActiveWorkers;
	}

	int GetTotalWorkerCount () const final
	{
		return m_dWorkers.GetLength();
	}

	int GetQueueLength () const final
	{
		return m_iStatQueuedJobs;
	}
};


ISphThdPool * sphThreadPoolCreate ( int iThreads, const char * sName, CSphString & sError )
{
	auto * pPool = new CSphThdPool ( iThreads, sName, sError );
	if ( !sError.IsEmpty() )
		SafeDelete ( pPool );

	return pPool;
}

int sphCpuThreadsCount ()
{
#if USE_WINDOWS
	SYSTEM_INFO tInfo;
	GetSystemInfo ( &tInfo );
	return tInfo.dwNumberOfProcessors;
#else
	return sysconf ( _SC_NPROCESSORS_ONLN );
#endif
}


//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4530)
#endif

#if NEW_IS_OVERRIDED
#undef new
#endif

#include <map>

#if NEW_IS_OVERRIDED
#define new        new(__FILE__,__LINE__)
#endif

#if USE_WINDOWS
#pragma warning(pop)
#endif

class TDigest_c : public TDigest_i
{
public:
	TDigest_c ()
	{
		Reset();
	}

	void Add ( double fValue, int64_t iWeight ) final
	{
		if ( m_dMap.empty() )
		{
			m_dMap.insert ( std::pair<double, int64_t> ( fValue, iWeight ) );
			m_iCount = iWeight;
			return;
		}

		auto tStart = m_dMap.lower_bound(fValue);
		if ( tStart==m_dMap.end() )
			tStart = m_dMap.begin();
		else
		{
			while ( tStart!=m_dMap.begin() && tStart->first==fValue )
				tStart--;
		}

		double fMinDist = DBL_MAX;
		auto tLastNeighbor = m_dMap.end();
		for ( auto i=tStart; i!=m_dMap.end(); ++i )
		{
			double fDist = fabs ( i->first - fValue );
			if ( fDist < fMinDist )
			{
				tStart = i;
				fMinDist = fDist;
			} else if ( fDist > fMinDist )
			{
				// we've passed the nearest nearest neighbor
				tLastNeighbor = i;
				break;
			}
		}

		auto tClosest = m_dMap.end();
		int64_t iSum = 0;
		for ( auto i=m_dMap.begin(); i!=tStart; ++i )
			iSum += i->second;

		int64_t iN = 0;
		const double COMPRESSION = 200.0;
		for ( auto i=tStart; i!=tLastNeighbor; ++i )
		{
			double fQuantile = m_iCount==1 ? 0.5 : (iSum + (i->second - 1) / 2.0) / (m_iCount - 1);
			double fThresh = 4.0 * m_iCount * fQuantile * (1 - fQuantile) / COMPRESSION;

			if ( i->second+iWeight<=fThresh )
			{
				iN++;
				if ( ( double ( sphRand() ) / UINT_MAX )<1.0/iN )
					tClosest = i;
			}

			iSum += i->second;
		}

		if ( tClosest==m_dMap.end() )
			m_dMap.insert ( std::pair<double, int64_t> ( fValue, iWeight ) );
		else
		{
			double fNewMean = WeightedAvg ( tClosest->first, tClosest->second, fValue, iWeight );
			int64_t iNewCount = tClosest->second+iWeight;
			m_dMap.erase ( tClosest );
			m_dMap.insert ( std::pair<double, int64_t> ( fNewMean, iNewCount ) );
		}

		m_iCount += iWeight;

		const DWORD K=20;
		if ( m_dMap.size() > K * COMPRESSION )
			Compress();
	}


	double Percentile ( int iPercent ) const final
	{
		assert ( iPercent>=0 && iPercent<=100 );

		if ( m_dMap.empty() )
			return 0.0;

		int64_t iTotalCount = 0;
		double fPercent = double ( iPercent ) / 100.0;
		fPercent *= m_iCount;

		auto iMapFirst = m_dMap.begin();
		auto iMapLast = m_dMap.end();
		--iMapLast;

		for ( auto i = iMapFirst; i!=m_dMap.end(); ++i )
		{
			if ( fPercent < iTotalCount + i->second )
			{
				if ( i==iMapFirst || i==iMapLast )
					return i->first;
				else
				{
					// get mean from previous iterator; get mean from next iterator; calc delta
					auto iPrev = i;
					auto iNext = i;
					iPrev--;
					iNext++;

					double fDelta = ( iNext->first - iPrev->first ) / 2.0;
					return i->first + ((fPercent - iTotalCount) / i->second - 0.5) * fDelta;
				}
			}

			iTotalCount += i->second;
		}

		return iMapLast->first;
	}

private:
	using BalancedTree_c = std::multimap<double, int64_t, std::less<double>, managed_allocator<std::pair<const double, int64_t>>>;
	BalancedTree_c		m_dMap;
	int64_t				m_iCount;

	double WeightedAvg ( double fX1, int64_t iW1, double fX2, int64_t iW2 )
	{
		return ( fX1*iW1 + fX2*iW2 ) / ( iW1 + iW2 );
	}

	void Reset()
	{
		m_dMap.clear();
		m_iCount = 0;
	}

	void Compress()
	{
		struct Centroid_t
		{
			double	m_fMean;
			int64_t	m_iCount;
		};

		CSphTightVector<Centroid_t> dValues;
		dValues.Reserve ( m_dMap.size() );
		for ( auto i : m_dMap )
		{
			Centroid_t & tCentroid = dValues.Add();
			tCentroid.m_fMean = i.first;
			tCentroid.m_iCount = i.second;
		}

		Reset();

		while ( dValues.GetLength() )
		{
			int iValue = sphRand() % dValues.GetLength();
			Add ( dValues[iValue].m_fMean, dValues[iValue].m_iCount );
			dValues.RemoveFast(iValue);
		}
	}
};


TDigest_i * sphCreateTDigest()
{
	return new TDigest_c;
}

//////////////////////////////////////////////////////////////////////////
/// StringBuilder implementation
//////////////////////////////////////////////////////////////////////////
StringBuilder_c::StringBuilder_c ( const char * sDel, const char * sPref, const char * sTerm )
{
	if ( sDel || sPref || sTerm )
		StartBlock ( sDel, sPref, sTerm );
}

StringBuilder_c::~StringBuilder_c ()
{
	SafeDelete ( m_pDelimiter );
	SafeDeleteArray ( m_sBuffer );
}

StringBuilder_c::StringBuilder_c ( StringBuilder_c&& rhs ) noexcept
{
	Swap (rhs);
}

void StringBuilder_c::Swap ( StringBuilder_c& rhs ) noexcept
{
	::Swap ( m_sBuffer, rhs.m_sBuffer );
	::Swap ( m_iSize, rhs.m_iSize );
	::Swap ( m_iUsed, rhs.m_iUsed );
	::Swap ( m_pDelimiter, rhs.m_pDelimiter );
}

StringBuilder_c & StringBuilder_c::operator= ( StringBuilder_c rhs ) noexcept
{
	Swap (rhs);
	return *this;
}

StringBuilder_c::LazyComma_c * StringBuilder_c::StartBlock ( const char * sDel, const char * sPref, const char * sTerm )
{
	m_pDelimiter = new LazyComma_c (m_pDelimiter, sDel, sPref, sTerm );
	return m_pDelimiter;
}

void StringBuilder_c::FinishBlock ( bool bAllowEmpty ) // finish last pushed block
{
	if ( !m_pDelimiter )
		return;

	if ( !bAllowEmpty && !m_pDelimiter->Started() )
	{
		AppendChars ( "\0", 1 );
		--m_iUsed;
	}

	if ( m_pDelimiter->Started () )
		AppendRawChars ( m_pDelimiter->m_sSuffix );

	auto pDel = m_pDelimiter;
	m_pDelimiter = pDel->m_pPrevious;
	pDel->m_pPrevious = nullptr;
	SafeDelete ( pDel );
}

void StringBuilder_c::FinishBlocks ( StringBuilder_c::LazyComma_c * pLevel, bool bAllowEmpty )
{
	while ( m_pDelimiter && m_pDelimiter!=pLevel )
		FinishBlock ( bAllowEmpty );
	if ( m_pDelimiter )
		FinishBlock ( bAllowEmpty );
}

StringBuilder_c & StringBuilder_c::vAppendf ( const char * sTemplate, va_list ap )
{
	if ( !m_sBuffer )
		InitBuffer ();

	assert ( m_sBuffer );
	assert ( m_iUsed<m_iSize );

	int iComma = 0;
	const char * sPrefix = m_pDelimiter ? m_pDelimiter->RawComma ( iComma, *this ) : nullptr;

	while (true)
	{
		int iLeft = m_iSize - m_iUsed;
		if ( iComma && iComma < iLeft ) // prepend delimiter first...
		{
			if ( sPrefix )
				memcpy ( m_sBuffer + m_iUsed, sPrefix, iComma );
			iLeft -= iComma;
			m_iUsed += iComma;
			iComma = 0;
		}

		// try to append
		va_list cp;
		va_copy ( cp, ap );
		int iPrinted = vsnprintf ( m_sBuffer + m_iUsed, iLeft, sTemplate, cp );
		va_end( cp );

		// success? bail
		// note that we check for strictly less, not less or equal
		// that is because vsnprintf does *not* count the trailing zero
		// meaning that if we had N bytes left, and N bytes w/o the zero were printed,
		// we do not have a trailing zero anymore, but vsnprintf succeeds anyway
		if ( iPrinted>=0 && iPrinted<iLeft )
		{
			m_iUsed += iPrinted;
			break;
		}

		// we need more chars!
		// either 256 (happens on Windows; lets assume we need 256 more chars)
		// or get all the needed chars and 64 more for future calls
		GrowEnough ( iPrinted<0 ? 256 : iPrinted + iComma );
	}
	return *this;
}

StringBuilder_c & StringBuilder_c::Appendf ( const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	vAppendf ( sTemplate, ap );
	va_end ( ap );
	return *this;
}

StringBuilder_c &StringBuilder_c::vSprintf ( const char * sTemplate, va_list ap )
{
	if ( !m_sBuffer )
		InitBuffer ();

	assert ( m_iUsed==0 || m_iUsed<m_iSize );

	int iComma = 0;
	const char * sPrefix = m_pDelimiter ? m_pDelimiter->RawComma ( iComma, *this ) : nullptr;

	if ( iComma && sPrefix ) // prepend delimiter first...
	{
		GrowEnough ( iComma );
		memcpy ( m_sBuffer + m_iUsed, sPrefix, iComma );
		m_iUsed += iComma;
	}
	sph::vSprintf ( *this, sTemplate, ap );
	return *this;
}

StringBuilder_c &StringBuilder_c::Sprintf ( const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	vSprintf ( sTemplate, ap );
	va_end ( ap );
	return *this;
}

BYTE * StringBuilder_c::Leak ()
{
	auto pRes = ( BYTE * ) m_sBuffer;
	NewBuffer ();
	return pRes;
}

void StringBuilder_c::MoveTo ( CSphString &sTarget )
{
	sTarget.Adopt ( &m_sBuffer );
	NewBuffer ();
}

void StringBuilder_c::AppendRawChars ( const char * sText ) // append without any commas
{
	if ( !sText || !*sText )
		return;

	int iLen = strlen ( sText );

	GrowEnough ( iLen + 1 ); // +1 because we'll put trailing \0 also

	memcpy ( m_sBuffer + m_iUsed, sText, iLen );
	m_iUsed += iLen;
	m_sBuffer[m_iUsed] = '\0';
}

StringBuilder_c &StringBuilder_c::SkipNextComma()
{
	if ( m_pDelimiter )
		m_pDelimiter->SkipNext ();
	return *this;
}

StringBuilder_c &StringBuilder_c::AppendName ( const char * sName )
{
	if ( !sName || !strlen ( sName ) )
		return *this;

	AppendChars ( sName, strlen ( sName ), '"' );
	GrowEnough(2);
	m_sBuffer[m_iUsed] = ':';
	m_sBuffer[m_iUsed+1] = '\0';
	m_iUsed+=1;
	return SkipNextComma ();
}

StringBuilder_c & StringBuilder_c::AppendChars ( const char * sText, int iLen, char cQuote )
{
	if ( !iLen )
		return *this;

	int iComma = 0;
	const char * sPrefix = m_pDelimiter ? m_pDelimiter->RawComma ( iComma, *this ) : nullptr;

	int iQuote = 0;
	if ( cQuote!='\0' )
		++iQuote;

	GrowEnough ( iLen + iComma + iQuote + iQuote + 1 ); // +1 because we'll put trailing \0 also

	if ( sPrefix )
		memcpy ( m_sBuffer + m_iUsed, sPrefix, iComma );
	if (iQuote)
		m_sBuffer[m_iUsed + iComma] = cQuote;
	memcpy ( m_sBuffer + m_iUsed + iComma + iQuote, sText, iLen );
	m_iUsed += iLen+iComma+iQuote+iQuote;
	if ( iQuote )
		m_sBuffer[m_iUsed-1] = cQuote;
	m_sBuffer[m_iUsed] = '\0';
	return *this;
}

StringBuilder_c &StringBuilder_c::AppendString ( const CSphString &sText, char cQuote)
{
	return AppendChars ( sText.cstr(), sText.Length (), cQuote );
}

StringBuilder_c& StringBuilder_c::operator+= ( const char * sText )
{
	if ( !sText || *sText=='\0' )
		return *this;

	return AppendChars ( sText, strlen ( sText ) );
}

StringBuilder_c &StringBuilder_c::operator<< ( const VecTraits_T<char> &sText )
{
	if ( sText.IsEmpty () )
		return *this;

	return AppendChars ( sText.begin(), sText.GetLength() );
}

void StringBuilder_c::Grow ( int iLen )
{
	assert ( m_iSize<m_iUsed + iLen + uSTEP );
	m_iSize = sph::TightRelimit::Relimit( m_iSize, m_iUsed + iLen + uSTEP );
	auto * pNew = new char[m_iSize];
	if ( m_sBuffer )
		memcpy ( pNew, m_sBuffer, m_iUsed + 1 );
	::Swap ( pNew, m_sBuffer );
	SafeDeleteArray ( pNew );
}

void StringBuilder_c::InitBuffer ()
{
	m_iSize = 256;
	m_sBuffer = new char[m_iSize];
}

void StringBuilder_c::NewBuffer ()
{
	InitBuffer();
	Clear ();
}

void StringBuilder_c::Clear ()
{
	if ( m_sBuffer )
		m_sBuffer[0] = '\0';
	m_iUsed = 0;
	SafeDelete ( m_pDelimiter );
}

#ifdef USE_SMALLALLOC

////////////////////////////////////////////////////////////////////////////////
/// FixedAllocator_c, PtrAttrAllocator_c
/// Provides fast alloc/dealloc for small objects
/// (large objects will be redirected to standard new/delete)
////////////////////////////////////////////////////////////////////////////////
static const size_t DEFAULT_CHUNK_SIZE = 4096;


////////////////////////////////////////////////////////////////////////////////
/// FixedAllocator_c
/// Represents storage for any num of objects of fixed size
////////////////////////////////////////////////////////////////////////////////
class FixedAllocator_c : ISphNoncopyable
{
	struct Chunk_t
	{
		void Init ( int iBlockSize, BYTE uBlocks );
		void Release ();

		BYTE * Allocate ( int iBlockSize );
		void Deallocate ( BYTE * pBlob, int iBlockSize );

		BYTE * m_pData;
		BYTE m_uFirstAvailable;
		BYTE m_uAvailable;
	};

	int m_iBlockSize;        // size of blobs I can serve
	Chunk_t * m_pAllocChunk = nullptr;      // shortcut to last alloc
	Chunk_t * m_pDeallocChunk = nullptr;    // shortcut to last dealloc
	Chunk_t * m_pLastFree = nullptr;		//
	CSphVector<Chunk_t> m_dChunks;    // my chunks
	BYTE m_uNumBlocks;        // # of blocks per chunk

private:
	void Swap ( FixedAllocator_c &rhs );
	void DoDeallocate ( BYTE * pBlob );
	Chunk_t * VicinityFind ( BYTE * pBlob );

public:
	explicit FixedAllocator_c ( int iBlockSize = 0 );
	FixedAllocator_c ( FixedAllocator_c && ) noexcept;
	FixedAllocator_c &operator= ( FixedAllocator_c && ) noexcept;
	~FixedAllocator_c ();

	// allocate block of my m_iBlockSize
	BYTE * Allocate ();

	// Deallocate a memory block previously allocated with Allocate()
	// (if that's not the case, the behavior is undefined)
	void Deallocate ( BYTE * pBlob );

	inline int BlockSize () const
	{ return m_iBlockSize; }

	// debug/diagnostic
	size_t GetAllocatedSize () const;	// how many allocated right now
	size_t GetReservedSize () const;	// how many pooled from the sys right now
};

////////////////////////////////////////////////////////////////////////////////
/// FixedAllocator_c::Chunk_t
/// Represents a blob of memory for few objects of fixed size
////////////////////////////////////////////////////////////////////////////////
void FixedAllocator_c::Chunk_t::Init ( int iBlockSize, BYTE uBlocks )
{
	assert( iBlockSize>0 );
	assert( uBlocks>0 );

	// Overflow check
	assert( ( iBlockSize * uBlocks ) / iBlockSize==uBlocks );

	m_pData = new BYTE[iBlockSize * uBlocks];
	m_uAvailable = uBlocks;
	m_uFirstAvailable = 0;

	auto p = m_pData;
	for ( BYTE i=0; i<uBlocks; p += iBlockSize )
		*p = ++i;
}

void FixedAllocator_c::Chunk_t::Release ()
{
	delete[] m_pData;
}

BYTE * FixedAllocator_c::Chunk_t::Allocate ( int iBlockSize )
{
	if ( !m_uAvailable )
		return nullptr;

	assert( ( m_uFirstAvailable * iBlockSize ) / iBlockSize==m_uFirstAvailable );

	auto * pResult = m_pData + ( m_uFirstAvailable * iBlockSize );
	m_uFirstAvailable = *pResult;
	--m_uAvailable;

	return pResult;
}

void FixedAllocator_c::Chunk_t::Deallocate ( BYTE * pBlob, int iBlockSize )
{
	assert( pBlob>=m_pData );

	// Alignment check
	assert( ( pBlob - m_pData ) % iBlockSize==0 );

	*pBlob = m_uFirstAvailable;
	m_uFirstAvailable = ( BYTE ) ( ( pBlob - m_pData ) / iBlockSize );

	// Truncation check
	assert( m_uFirstAvailable==( pBlob - m_pData ) / iBlockSize );
	++m_uAvailable;
}


FixedAllocator_c::FixedAllocator_c ( int iBlockSize )
	: m_iBlockSize ( iBlockSize )
{
	if ( !iBlockSize )
		return;

	assert( m_iBlockSize>0 );
	m_uNumBlocks = (BYTE) Min ( DEFAULT_CHUNK_SIZE / iBlockSize, BYTE ( 0xFF ) );
	assert ( m_uNumBlocks && "Too large iBlocksize passed" );
}

FixedAllocator_c::FixedAllocator_c ( FixedAllocator_c &&rhs ) noexcept
{
	Swap (rhs);
}

FixedAllocator_c &FixedAllocator_c::operator= ( FixedAllocator_c &&rhs ) noexcept
{
	Swap (rhs);
	return *this;
}

FixedAllocator_c::~FixedAllocator_c ()
{
	for ( auto & dChunk : m_dChunks )
	{
		assert (dChunk.m_uAvailable==m_uNumBlocks);
		dChunk.Release();
	}
}

void FixedAllocator_c::Swap ( FixedAllocator_c &rhs )
{
	::Swap ( m_iBlockSize, rhs.m_iBlockSize );
	::Swap ( m_uNumBlocks, rhs.m_uNumBlocks );
	m_dChunks.SwapData ( rhs.m_dChunks );
	::Swap ( m_pAllocChunk, rhs.m_pAllocChunk );
	::Swap ( m_pDeallocChunk, rhs.m_pDeallocChunk );
}

BYTE * FixedAllocator_c::Allocate ()
{
	if ( !m_pAllocChunk || !m_pAllocChunk->m_uAvailable )
	{
		for ( auto & dChunk : m_dChunks)
		{
			if ( dChunk.m_uAvailable )
			{
				m_pAllocChunk = &dChunk;
				return m_pAllocChunk->Allocate ( m_iBlockSize );
			}
		}

		// loop finished, we're still here. Create new chunk then
		Chunk_t &dNewChunk = m_dChunks.Add ();
		dNewChunk.Init ( m_iBlockSize, m_uNumBlocks );
		m_pDeallocChunk = m_dChunks.begin ();
		m_pAllocChunk = &m_dChunks.Last();
	}

	assert( m_pAllocChunk );
	assert( m_pAllocChunk->m_uAvailable );

	return m_pAllocChunk->Allocate ( m_iBlockSize );
}

void FixedAllocator_c::Deallocate ( BYTE * pBlob )
{
	assert( !m_dChunks.IsEmpty () );
	assert( m_dChunks.begin ()<=m_pDeallocChunk );
	assert( m_dChunks.end ()>m_pDeallocChunk );

	m_pDeallocChunk = VicinityFind ( pBlob );
	assert( m_pDeallocChunk );

	DoDeallocate ( pBlob );
}

// Finds the chunk corresponding to a pointer, using an efficient search
FixedAllocator_c::Chunk_t * FixedAllocator_c::VicinityFind ( BYTE * pBlob )
{
	assert( !m_dChunks.IsEmpty () );
	assert( m_pDeallocChunk );

	const int iChunkLength = m_uNumBlocks * m_iBlockSize;

	Chunk_t * pLo = m_pDeallocChunk;
	Chunk_t * pHi = m_pDeallocChunk + 1;
	Chunk_t * pLoBound = m_dChunks.begin();
	Chunk_t * pHiBound = m_dChunks.end();

	// Special case: deallocChunk_ is the last in the array
	if ( pHi==pHiBound )
		pHi = nullptr;

	while (true)
	{
		if ( pLo )
		{
			if ( pBlob>=pLo->m_pData && pBlob<pLo->m_pData + iChunkLength )
				return pLo;

			if ( pLo==pLoBound )
				pLo = nullptr;
			else
				--pLo;
		}

		if ( pHi )
		{
			if ( pBlob>=pHi->m_pData && pBlob<pHi->m_pData + iChunkLength )
				return pHi;

			if ( ++pHi==pHiBound )
				pHi = nullptr;
		}
	}
}

// Performs deallocation. Assumes m_pDeallocChunk points to the correct chunk
void FixedAllocator_c::DoDeallocate ( BYTE * pBlob )
{
	assert( m_pDeallocChunk->m_pData<=pBlob );
	assert( m_pDeallocChunk->m_pData + m_uNumBlocks * m_iBlockSize>pBlob );

	// call into the chunk, will adjust the inner list but won't release memory
	m_pDeallocChunk->Deallocate ( pBlob, m_iBlockSize );

	if ( m_pDeallocChunk->m_uAvailable==m_uNumBlocks )
	{
		// deallocChunk_ is completely free, should we release it?
		if ( m_pLastFree!=m_pDeallocChunk )
		{
			if ( m_pLastFree && m_pLastFree->m_uAvailable==m_uNumBlocks )
			{
				// Two free blocks, discard one
				m_pLastFree->Release();
				auto iIdx = m_pLastFree - m_dChunks.begin ();
				m_dChunks.RemoveFast ( iIdx );
				if ( m_pDeallocChunk == m_dChunks.end() )
					m_pDeallocChunk = m_pLastFree;
			}
			// move the empty chunk to the end
			m_pAllocChunk = m_pLastFree = m_pDeallocChunk;
			m_pDeallocChunk = &m_dChunks.Last();
		}
	}
}

size_t FixedAllocator_c::GetAllocatedSize () const
{
	size_t uAccum = 0;
	for ( const auto &dChunk : m_dChunks )
		uAccum += m_uNumBlocks-dChunk.m_uAvailable;
	return uAccum * m_iBlockSize;
}

size_t FixedAllocator_c::GetReservedSize () const
{
	auto uSize = Max ( m_iBlockSize * m_uNumBlocks, 4096 );
	return ( size_t ) m_dChunks.GetLength () * uSize;
}

////////////////////////////////////////////////////////////////////////////////
/// class PtrAttrAllocator_c
/// Offers fast allocations/deallocations
////////////////////////////////////////////////////////////////////////////////
class PtrAttrAllocator_c : public ISphNoncopyable
{
	CSphSwapVector<FixedAllocator_c> m_dPool;
	FixedAllocator_c * m_pLastAlloc;
	FixedAllocator_c * m_pLastDealloc;
	CSphMutex m_dAllocMutex;

private:
	int LowerBound ( int iBytes ) REQUIRES (m_dAllocMutex);

public:

	PtrAttrAllocator_c()
		: m_pLastAlloc (nullptr)
		, m_pLastDealloc (nullptr)
	{
		m_dPool.Reserve ( MAX_SMALL_OBJECT_SIZE );
	}

	BYTE * Allocate ( int iBytes );
	void Deallocate ( BYTE * pBlob, int iBytes );

	// debug/diagnostic
	size_t GetAllocatedSize();	// how many allocated right now
	size_t GetReservedSize();	// how many pooled from the sys right now
};

// returns lower bound for fixed allocators sized 'iBytes'.
// i.e. idx of pool (asc sorted) where you can insert new not breaking the sequence.
int PtrAttrAllocator_c::LowerBound ( int iBytes )
{
	if ( m_dPool.IsEmpty () )
		return -1;

	auto pStart = m_dPool.begin();
	auto pLast = &m_dPool.Last();

	if ( pStart->BlockSize ()>=iBytes )
		return 0;

	if ( pLast->BlockSize ()<iBytes )
		return -1;

	while ( pLast - pStart>1 )
	{
		auto pMid = pStart + ( pLast - pStart ) / 2;
		if ( pMid->BlockSize()<iBytes )
			pStart = pMid;
		else
			pLast = pMid;
	}
	return int ( pLast - m_dPool.begin () );
}

// Allocates 'iBytes' memory
// Uses an internal pool of FixedAllocator_c objects for small objects
BYTE * PtrAttrAllocator_c::Allocate ( int iBytes )
{
	if ( iBytes>MAX_SMALL_OBJECT_SIZE )
		return new BYTE[iBytes];

	CSphScopedLock<CSphMutex> tScopedLock ( m_dAllocMutex );
	if ( m_pLastAlloc && m_pLastAlloc->BlockSize ()==iBytes )
		return m_pLastAlloc->Allocate ();

	auto i = LowerBound ( iBytes );
	if ( i<0 ) // required size > any other in the pool
	{
		i = m_dPool.GetLength ();
		m_dPool.Add ( FixedAllocator_c ( iBytes ) );
		m_pLastDealloc = m_dPool.begin();
	}
	else if ( m_dPool[i].BlockSize ()!=iBytes )
	{
		FixedAllocator_c dAlloc ( iBytes );
		m_dPool.Insert ( i, dAlloc );
		m_pLastDealloc = m_dPool.begin ();
	}
	m_pLastAlloc = m_dPool.begin() + i;
	return m_pLastAlloc->Allocate ();
}

// Deallocates memory previously allocated with Allocate
// (undefined behavior if you pass any other pointer)
void PtrAttrAllocator_c::Deallocate ( BYTE * pBlob, int iBytes )
{
	if ( iBytes>MAX_SMALL_OBJECT_SIZE )
	{
		SafeDeleteArray ( pBlob );
		return;
	}

	CSphScopedLock<CSphMutex> tScopedLock ( m_dAllocMutex );
	if ( m_pLastDealloc && m_pLastDealloc->BlockSize ()==iBytes )
	{
		m_pLastDealloc->Deallocate ( pBlob );
		return;
	}

	auto i = LowerBound ( iBytes );
	assert( i>=0 );
	assert( m_dPool[i].BlockSize ()==iBytes );
	m_pLastDealloc = m_dPool.begin () + i;
	m_pLastDealloc->Deallocate ( pBlob );
}

size_t PtrAttrAllocator_c::GetAllocatedSize ()
{
	CSphScopedLock<CSphMutex> tScopedLock ( m_dAllocMutex );
	size_t uAccum = 0;
	for ( const auto &dAlloc : m_dPool )
		uAccum += dAlloc.GetAllocatedSize ();
	return uAccum;
}

size_t PtrAttrAllocator_c::GetReservedSize ()
{
	CSphScopedLock<CSphMutex> tScopedLock ( m_dAllocMutex );
	size_t uAccum = 0;
	for ( const auto &dAlloc : m_dPool )
		uAccum += dAlloc.GetReservedSize ();
	return uAccum;
}

PtrAttrAllocator_c &SmallAllocator ()
{
	static PtrAttrAllocator_c dAllocator;
	return dAllocator;
}



BYTE * sphAllocateSmall ( int iBytes )
{
	assert ( iBytes>0 );
	if ( iBytes>MAX_SMALL_OBJECT_SIZE )
		return new BYTE[iBytes];

	return SmallAllocator().Allocate (iBytes);
}

void sphDeallocateSmall ( BYTE * pBlob, int iBytes )
{
	if ( iBytes>MAX_SMALL_OBJECT_SIZE )
	{
		SafeDeleteArray ( pBlob );
		return;
	}
	SmallAllocator ().Deallocate (pBlob, iBytes);
}

size_t sphGetSmallAllocatedSize ()
{
	return SmallAllocator ().GetAllocatedSize ();
}
size_t sphGetSmallReservedSize ()
{
	return SmallAllocator ().GetReservedSize ();
}
#endif
