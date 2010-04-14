//
// $Id$
//

//
// Copyright (c) 2001-2010, Andrew Aksyonoff
// Copyright (c) 2008-2010, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"

#if !USE_WINDOWS
#include <sys/time.h> // for gettimeofday
#endif

//////////////////////////////////////////////////////////////////////////

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

// for ::write
#if USE_WINDOWS
#include <io.h>
#else
#include <unistd.h>
#endif

const DWORD MEMORY_MAGIC_PLAIN		= 0xbbbbbbbbUL;
const DWORD MEMORY_MAGIC_ARRAY		= 0xaaaaaaaaUL;
const DWORD MEMORY_MAGIC_END		= 0xeeeeeeeeUL;
const DWORD MEMORY_MAGIC_DELETED	= 0xdedededeUL;


struct CSphMemHeader
{
	DWORD			m_uMagic;
	const char *	m_sFile;
	int				m_iLine;
	size_t			m_iSize;
	int				m_iAllocId;
	CSphMemHeader *	m_pNext;
	CSphMemHeader *	m_pPrev;
};

static CSphStaticMutex	g_tAllocsMutex;

static int				g_iCurAllocs	= 0;
static int				g_iAllocsId		= 0;
static CSphMemHeader *	g_pAllocs		= NULL;
static int64_t			g_iCurBytes		= 0;
static int				g_iTotalAllocs	= 0;
static int				g_iPeakAllocs	= 0;
static int64_t			g_iPeakBytes	= 0;

void * sphDebugNew ( size_t iSize, const char * sFile, int iLine, bool bArray )
{
	BYTE * pBlock = (BYTE*) ::malloc ( iSize+sizeof(CSphMemHeader)+sizeof(DWORD) );
	if ( !pBlock )
		sphDie ( "out of memory (unable to allocate "UINT64_FMT" bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too

	*(DWORD*)( pBlock+iSize+sizeof(CSphMemHeader) ) = MEMORY_MAGIC_END;
	g_tAllocsMutex.Lock();

	CSphMemHeader * pHeader = (CSphMemHeader*) pBlock;
	pHeader->m_uMagic = bArray ? MEMORY_MAGIC_ARRAY : MEMORY_MAGIC_PLAIN;
	pHeader->m_sFile = sFile;
	pHeader->m_iLine = iLine;
	pHeader->m_iSize = iSize;
	pHeader->m_iAllocId = ++g_iAllocsId;
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

	char sBuf [ 1024 ];
	snprintf ( sBuf, sizeof(sBuf), "--- dumping allocs since %d ---\n", iSinceID );
	write ( iFile, sBuf, strlen(sBuf) );

	for ( CSphMemHeader * pHeader = g_pAllocs;
		pHeader && pHeader->m_iAllocId > iSinceID;
		pHeader = pHeader->m_pNext )
	{
		snprintf ( sBuf, sizeof(sBuf), "alloc %d at %s(%d): %d bytes\n", pHeader->m_iAllocId,
			pHeader->m_sFile, pHeader->m_iLine, (int)pHeader->m_iSize );
		write ( iFile, sBuf, strlen(sBuf) );
	}

	snprintf ( sBuf, sizeof(sBuf), "--- end of dump ---\n" );
	write ( iFile, sBuf, strlen(sBuf) );

	g_tAllocsMutex.Unlock();
}


void sphAllocsStats ()
{
	fprintf ( stdout, "--- total-allocs=%d, peak-allocs=%d, peak-bytes="INT64_FMT"\n",
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

//////////////////////////////////////////////////////////////////////////

void * operator new ( size_t iSize, const char * sFile, int iLine )
{
	return sphDebugNew ( iSize, sFile, iLine, false );
}


void * operator new [] ( size_t iSize, const char * sFile, int iLine )
{
	return sphDebugNew ( iSize, sFile, iLine, true );
}


void operator delete ( void * pPtr )
{
	sphDebugDelete ( pPtr, false );
}


void operator delete [] ( void * pPtr )
{
	sphDebugDelete ( pPtr, true );
}

//////////////////////////////////////////////////////////////////////////////
// ALLOCACTIONS COUNT/SIZE PROFILER
//////////////////////////////////////////////////////////////////////////////

#else
#if SPH_ALLOCS_PROFILER

#undef new

static CSphStaticMutex	g_tAllocsMutex;
static int				g_iCurAllocs	= 0;
static int64_t			g_iCurBytes		= 0;
static int				g_iTotalAllocs	= 0;
static int				g_iPeakAllocs	= 0;
static int64_t			g_iPeakBytes	= 0;

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

static MemCategorized_t g_dMemCategoryStat[Memory::SPH_MEM_TOTAL];
static Memory::Category_e sphMemStatGet ();

//////////////////////////////////////////////////////////////////////////////
// ALLOCACTIONS COUNT/SIZE PROFILER

void * sphDebugNew ( size_t iSize )
{
	BYTE * pBlock = (BYTE*) ::malloc ( iSize+sizeof(size_t)*2 );
	if ( !pBlock )
		sphDie ( "out of memory (unable to allocate %"PRIu64" bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too

	const int iMemType = sphMemStatGet();
	assert ( iMemType>=0 && iMemType<Memory::SPH_MEM_TOTAL );

	g_tAllocsMutex.Lock ();

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
	assert ( iMemType>=0 && iMemType<Memory::SPH_MEM_TOTAL );

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
	fprintf ( stdout, "--- total-allocs=%d, peak-allocs=%d, peak-bytes="INT64_FMT"\n",
		g_iTotalAllocs, g_iPeakAllocs, g_iPeakBytes );
	g_tAllocsMutex.Unlock ();
}

int64_t sphAllocBytes ()		{ return g_iCurBytes; }
int sphAllocsCount ()			{ return g_iCurAllocs; }
int sphAllocsLastID ()			{ return -1; }
void sphAllocsDump ( int, int )	{}
void sphAllocsCheck ()			{}

void * operator new ( size_t iSize, const char *, int )		{ return sphDebugNew ( iSize ); }
void * operator new [] ( size_t iSize, const char *, int )	{ return sphDebugNew ( iSize ); }
void operator delete ( void * pPtr )						{ sphDebugDelete ( pPtr ); }
void operator delete [] ( void * pPtr )						{ sphDebugDelete ( pPtr ); }


//////////////////////////////////////////////////////////////////////////////
// MEMORY STATISTICS

/// TLS mem category (we keep Memory category stack here)
SphThreadKey_t g_tTLSMemCategory;

STATIC_ASSERT ( Memory::SPH_MEM_TOTAL<255, MEMORY_CATEGORY_EXCEED_LIMIT );

class MemCategoryStack_t // NOLINT
{
#define MEM_STACK_MAX_DEPHT 1024
	BYTE m_dStack[MEM_STACK_MAX_DEPHT];
	int m_iDepth;

public:

	void Reset ()
	{
		m_iDepth = 0;
		m_dStack[0] = Memory::SPH_MEM_CORE;
	}

	void Push ( Memory::Category_e eCategory )
	{
		assert ( eCategory>=0 && eCategory<Memory::SPH_MEM_TOTAL );
		assert ( m_iDepth+1<MEM_STACK_MAX_DEPHT );
		m_dStack[++m_iDepth] = (BYTE)eCategory;
	}

#ifndef NDEBUG
	void Pop ( Memory::Category_e eCategory )
	{
		assert ( eCategory>=0 && eCategory<Memory::SPH_MEM_TOTAL );
#else
	void Pop ( Memory::Category_e )
	{
#endif

		assert ( m_iDepth-1>=0 );
		assert ( m_dStack[m_iDepth]==eCategory );
		m_iDepth--;
	}

	Memory::Category_e Top () const
	{
		assert ( m_iDepth>= 0 && m_iDepth<MEM_STACK_MAX_DEPHT );
		assert ( m_dStack[m_iDepth]>=0 && m_dStack[m_iDepth]<Memory::SPH_MEM_TOTAL );
		return Memory::Category_e ( m_dStack[m_iDepth] );
	}
};

void sphMemStatInit ()
{
	Verify ( sphThreadKeyCreate ( &g_tTLSMemCategory ) );

	// main thread statictic creation
	sphMemStatThdInit();
}

void sphMemStatDone ()
{
	sphThreadKeyDelete ( g_tTLSMemCategory );
}

void sphMemStatThdCleanup ( void * pArg )
{
	MemCategoryStack_t * pTLS = (MemCategoryStack_t *)pArg;
	sphDebugDelete ( pTLS );
}

// memory statistics should be created first \ deleted last
void sphMemStatThdInit ()
{
	MemCategoryStack_t * pTLS = (MemCategoryStack_t *)sphDebugNew ( sizeof ( MemCategoryStack_t ) );
	pTLS->Reset();

	Verify ( sphThreadSet ( g_tTLSMemCategory, pTLS ) );
	sphThreadOnExit ( sphMemStatThdCleanup, pTLS );
}

void sphMemStatMMapAdd ( int64_t iSize )
{
	g_tAllocsMutex.Lock ();

	g_iCurAllocs++;
	g_iCurBytes += iSize;
	g_iTotalAllocs++;
	g_iPeakAllocs = Max ( g_iCurAllocs, g_iPeakAllocs );
	g_iPeakBytes = Max ( g_iCurBytes, g_iPeakBytes );

	g_dMemCategoryStat[Memory::SPH_MEM_MMAPED].m_iSize += iSize;
	g_dMemCategoryStat[Memory::SPH_MEM_MMAPED].m_iCount++;

	g_tAllocsMutex.Unlock ();
}

void sphMemStatMMapDel ( int64_t iSize )
{
	g_tAllocsMutex.Lock ();

	g_iCurAllocs--;
	g_iCurBytes -= iSize;

	g_dMemCategoryStat[Memory::SPH_MEM_MMAPED].m_iSize -= iSize;
	g_dMemCategoryStat[Memory::SPH_MEM_MMAPED].m_iCount--;

	g_tAllocsMutex.Unlock ();
}

void sphMemStatPush ( Memory::Category_e eCategory )
{
	MemCategoryStack_t * pTLS = (MemCategoryStack_t*) sphThreadGet ( g_tTLSMemCategory );
	if ( pTLS )
		pTLS->Push ( eCategory );
};

void sphMemStatPop ( Memory::Category_e eCategory )
{
	MemCategoryStack_t * pTLS = (MemCategoryStack_t*) sphThreadGet ( g_tTLSMemCategory );
	if ( pTLS )
		pTLS->Pop ( eCategory );
};

static Memory::Category_e sphMemStatGet ()
{
	MemCategoryStack_t * pTLS = (MemCategoryStack_t*) sphThreadGet ( g_tTLSMemCategory );
	return pTLS ? pTLS->Top() : Memory::SPH_MEM_CORE;
}

static const char* g_dMemCategoryName[] = {
	"core"
	, "index_disk", "index_rt", "index_rt_accum"
	, "mmaped", "binlog"
	, "hnd_disk", "hnd_sql"
	, "search_disk", "query_disk", "insert_sql", "select_sql", "delete_sql", "commit_set_sql", "commit_start_t_sql", "commit_sql"
	, "mquery_disk", "mqueryex_disk", "mquery_rt"
	};
STATIC_ASSERT ( sizeof(g_dMemCategoryName)/sizeof(g_dMemCategoryName[0])==Memory::SPH_MEM_TOTAL, MEM_STAT_NAME_MISMATCH );

extern void sphInfo ( const char * sFmt, ... );

void sphMemStatDump ()
{
	const float fMB = 1024.0f*1024.0f;
	float fSize = 0;
	int iCount = 0;
	for ( int i=0; i<Memory::SPH_MEM_TOTAL; i++ )
	{
		fSize += (float)g_dMemCategoryStat[i].m_iSize;
		iCount += g_dMemCategoryStat[i].m_iCount;
	}
	sphInfo ( "%-24s allocs-count=%d, mem-total=%.4f Mb", "(total)", iCount, fSize/fMB );

	for ( int i=0; i<Memory::SPH_MEM_TOTAL; i++ )
		if ( g_dMemCategoryStat[i].m_iCount>0 )
			sphInfo ( "%-24s allocs-count=%d, mem-total=%.4f Mb"
				, g_dMemCategoryName[i], g_dMemCategoryStat[i].m_iCount, (float)g_dMemCategoryStat[i].m_iSize/fMB );
}

//////////////////////////////////////////////////////////////////////////////
// PRODUCTION MEMORY MANAGER
//////////////////////////////////////////////////////////////////////////////

#else

void * operator new ( size_t iSize )
{
	void * pResult = ::malloc ( iSize );
	if ( !pResult )
		sphDie ( "out of memory (unable to allocate "UINT64_FMT" bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too
	return pResult;
}


void * operator new [] ( size_t iSize )
{
	void * pResult = ::malloc ( iSize );
	if ( !pResult )
		sphDie ( "out of memory (unable to allocate "UINT64_FMT" bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too
	return pResult;
}

void operator delete ( void * pPtr )
{
	if ( pPtr )
		::free ( pPtr );
}


void operator delete [] ( void * pPtr )
{
	if ( pPtr )
		::free ( pPtr );
}

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
	char sBuf[256];

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

CSphProcessSharedMutex::CSphProcessSharedMutex ()
{
#if !USE_WINDOWS
	m_pMutex = NULL;

	pthread_mutexattr_t tAttr;
	if ( pthread_mutexattr_init ( &tAttr ) || pthread_mutexattr_setpshared ( &tAttr, PTHREAD_PROCESS_SHARED ) )
		return;

	CSphString sError, sWarning;
	if ( !m_pStorage.Alloc ( sizeof(pthread_mutex_t), sError, sWarning ) )
		return;

	m_pMutex = (pthread_mutex_t*) m_pStorage.GetWritePtr ();
	if ( pthread_mutex_init ( m_pMutex, &tAttr ) )
	{
		m_pMutex = NULL;
		m_pStorage.Reset ();
		return;
	}
#endif
}


void CSphProcessSharedMutex::Lock ()
{
#if !USE_WINDOWS
	if ( m_pMutex )
		pthread_mutex_lock ( m_pMutex );
#endif
}


void CSphProcessSharedMutex::Unlock ()
{
#if !USE_WINDOWS
	if ( m_pMutex )
		pthread_mutex_unlock ( m_pMutex );
#endif
}

//////////////////////////////////////////////////////////////////////////
// THREADING FUNCTIONS
//////////////////////////////////////////////////////////////////////////

#define THREAD_STACK_SIZE 65536

struct ThreadCall_t
{
	void			( *m_pCall )( void * pArg );
	void *			m_pArg;
	ThreadCall_t *	m_pNext;
};
SphThreadKey_t g_tThreadCleanupKey;


#if USE_WINDOWS
#define SPH_THDFUNC DWORD __stdcall
#else
#define SPH_THDFUNC void *
#endif

SPH_THDFUNC sphThreadProcWrapper ( void * pArg )
{
	assert ( sphThreadGet ( g_tThreadCleanupKey )==NULL );

	ThreadCall_t * pCall = (ThreadCall_t*) pArg;
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

	return 0;
}

void * sphThreadInit()
{
	static bool bInit = false;
#if !USE_WINDOWS
	static pthread_attr_t tAttr;
#endif

	if ( !bInit )
	{
		// we're single-threaded yet, right?!
		if ( !sphThreadKeyCreate ( &g_tThreadCleanupKey ) )
			sphDie ( "FATAL: sphThreadKeyCreate() failed" );

#if !USE_WINDOWS
		if ( pthread_attr_init ( &tAttr ) )
			sphDie ( "FATAL: pthread_attr_init() failed" );

		if ( pthread_attr_setstacksize ( &tAttr, PTHREAD_STACK_MIN + THREAD_STACK_SIZE ) )
			sphDie ( "FATAL: pthread_attr_setstacksize() failed" );
#endif
		bInit = true;
	}
#if !USE_WINDOWS
	return &tAttr;
#else
	return NULL;
#endif
}

bool sphThreadCreate ( SphThread_t * pThread, void (*fnThread)(void*), void * pArg )
{
	// we can not merely put this on current stack
	// as it might get destroyed before wrapper sees it
	ThreadCall_t * pCall = new ThreadCall_t;
	pCall->m_pCall = fnThread;
	pCall->m_pArg = pArg;
	pCall->m_pNext = NULL;

	// create thread
#if USE_WINDOWS
	sphThreadInit();
	*pThread = CreateThread ( NULL, THREAD_STACK_SIZE, sphThreadProcWrapper, pCall, 0, NULL );
	if ( *pThread )
		return true;
#else
	void * pAttr = sphThreadInit();
	errno = pthread_create ( pThread, (pthread_attr_t*) pAttr, sphThreadProcWrapper, pCall );
	if ( !errno )
		return true;
#endif

	// thread creation failed so we need to cleanup ourselves
	SafeDelete ( pCall );
	return false;
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


void sphThreadOnExit ( void (*fnCleanup)(void*), void * pArg )
{
	ThreadCall_t * pCleanup = new ThreadCall_t;
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


bool sphThreadSet ( SphThreadKey_t tKey, void * pValue )
{
#if USE_WINDOWS
	return TlsSetValue ( tKey, pValue )!=FALSE;
#else
	return pthread_setspecific ( tKey, pValue )==0;
#endif
}

//////////////////////////////////////////////////////////////////////////
// MUTEX
//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS

// Windows mutex implementation

bool CSphMutex::Init ()
{
	m_hMutex = CreateMutex ( NULL, FALSE, NULL );
	m_bInitialized = ( m_hMutex!=NULL );
	return m_bInitialized;
}

bool CSphMutex::Done ()
{
	if ( !m_bInitialized )
		return true;

	m_bInitialized = false;
	return CloseHandle ( m_hMutex )==TRUE;
}

bool CSphMutex::Lock ()
{
	DWORD uWait = WaitForSingleObject ( m_hMutex, INFINITE );
	return ( uWait!=WAIT_FAILED && uWait!=WAIT_TIMEOUT );
}

bool CSphMutex::Unlock ()
{
	return ReleaseMutex ( m_hMutex )==TRUE;
}

#else

// UNIX mutex implementation

bool CSphMutex::Init ()
{
	m_bInitialized = ( pthread_mutex_init ( &m_tMutex, NULL )==0 );
	return m_bInitialized;
}

bool CSphMutex::Done ()
{
	if ( !m_bInitialized )
		return true;

	m_bInitialized = false;
	return pthread_mutex_destroy ( &m_tMutex )==0;
}

bool CSphMutex::Lock ()
{
	return ( pthread_mutex_lock ( &m_tMutex )==0 );
}

bool CSphMutex::Unlock ()
{
	return ( pthread_mutex_unlock ( &m_tMutex )==0 );
}

#endif

//////////////////////////////////////////////////////////////////////////
// RWLOCK
//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS

// Windows rwlock implementation

CSphRwlock::CSphRwlock ()
	: m_hWriteMutex ( NULL )
	, m_hReadEvent ( NULL )
	, m_iReaders ( 0 )
{}


bool CSphRwlock::Init ()
{
	assert ( !m_hWriteMutex && !m_hReadEvent && !m_iReaders );

	m_hReadEvent = CreateEvent ( NULL, TRUE, FALSE, NULL );
	if ( !m_hReadEvent )
		return false;

	m_hWriteMutex = CreateMutex ( NULL, FALSE, NULL );
	if ( !m_hWriteMutex )
	{
		CloseHandle ( m_hReadEvent );
		return false;
	}
	return true;
}


bool CSphRwlock::Done ()
{
	if ( !CloseHandle ( m_hReadEvent ) )
		return false;
	m_hReadEvent = NULL;

	if ( !CloseHandle ( m_hWriteMutex ) )
		return false;
	m_hWriteMutex = NULL;

	m_iReaders = 0;
	return true;
}


bool CSphRwlock::ReadLock ()
{
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
{}

bool CSphRwlock::Init ()
{
	return pthread_rwlock_init ( &m_tLock, NULL )==0;
}

bool CSphRwlock::Done ()
{
	return pthread_rwlock_destroy ( &m_tLock )==0;
}

bool CSphRwlock::ReadLock ()
{
	return pthread_rwlock_rdlock ( &m_tLock )==0;
}

bool CSphRwlock::WriteLock ()
{
	return pthread_rwlock_wrlock ( &m_tLock )==0;
}

bool CSphRwlock::Unlock ()
{
	return pthread_rwlock_unlock ( &m_tLock )==0;
}

#endif

//
// $Id$
//
