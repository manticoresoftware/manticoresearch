//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"

#if !USE_WINDOWS
#include <sys/time.h> // for gettimeofday
#endif

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
#include <sys/time.h>
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


static int				g_iCurAllocs	= 0;
static int				g_iAllocsId		= 0;
static CSphMemHeader *	g_pAllocs		= NULL;
static int				g_iCurBytes		= 0;
static int				g_iTotalAllocs	= 0;
static int				g_iPeakAllocs	= 0;
static int				g_iPeakBytes	= 0;

void * sphDebugNew ( size_t iSize, const char * sFile, int iLine, bool bArray )
{
	BYTE * pBlock = (BYTE*) ::malloc ( iSize+sizeof(CSphMemHeader)+sizeof(DWORD) );
	if ( !pBlock )
		sphDie ( "out of memory (unable to allocate %"PRIu64" bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too

	*(DWORD*)( pBlock+iSize+sizeof(CSphMemHeader) ) = MEMORY_MAGIC_END;

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

	return pHeader+1;
}


void sphDebugDelete ( void * pPtr, bool bArray )
{
	if ( !pPtr )
		return;

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
	if ( *(DWORD*)( pBlock+pHeader->m_iSize+sizeof(CSphMemHeader) ) != MEMORY_MAGIC_END )
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
}


int	sphAllocBytes ()
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
}


void sphAllocsStats ()
{
	fprintf ( stdout, "--- total-allocs=%d, peak-allocs=%d, peak-bytes=%d\n",
		g_iTotalAllocs, g_iPeakAllocs, g_iPeakBytes );
}


void sphAllocsCheck ()
{
	for ( CSphMemHeader * pHeader=g_pAllocs; pHeader; pHeader=pHeader->m_pNext )
	{
		BYTE * pBlock = (BYTE*) pHeader;

		if (!( pHeader->m_uMagic==MEMORY_MAGIC_ARRAY || pHeader->m_uMagic==MEMORY_MAGIC_PLAIN ))
			sphDie ( "corrupted header in block %d allocated at %s(%d)",
				pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );

		if ( *(DWORD*)( pBlock+pHeader->m_iSize+sizeof(CSphMemHeader) ) != MEMORY_MAGIC_END )
			sphDie ( "out-of-bounds write beyond block %d allocated at %s(%d)",
				pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );
	}
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
// PRODUCTION MEMORY MANAGER
//////////////////////////////////////////////////////////////////////////////

#else

void * operator new ( size_t iSize )
{
	void * pResult = ::malloc ( iSize );
	if ( !pResult )
		sphDie ( "out of memory (unable to allocate %"PRIu64" bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too
	return pResult;
}


void * operator new [] ( size_t iSize )
{
	void * pResult = ::malloc ( iSize );
	if ( !pResult )
		sphDie ( "out of memory (unable to allocate %"PRIu64" bytes)", (uint64_t)iSize ); // FIXME! this may fail with malloc error too
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

#endif // SPH_DEBUG_LEAKS

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
	for ( int i=0; i<5; i++)
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
	tv.tv_sec  = (DWORD)(ts/1000000);
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

//
// $Id$
//
