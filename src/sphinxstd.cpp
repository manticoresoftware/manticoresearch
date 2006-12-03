//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"

/////////////////////////////////////////////////////////////////////////////
// DEBUG MEMORY MANAGER
/////////////////////////////////////////////////////////////////////////////

#if SPH_DEBUG_LEAKS

#undef new

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


static int				g_iAllocsCount	= 0;
static int				g_iAllocsId		= 0;
static CSphMemHeader *	g_pAllocs		= NULL;


void * sphDebugNew ( size_t iSize, const char * sFile, int iLine, bool bArray )
{
	BYTE * pBlock = (BYTE*) ::malloc ( iSize+sizeof(CSphMemHeader)+sizeof(DWORD) );
	if ( !pBlock )
		sphDie ( "FATAL: out of memory (unable to allocate %d bytes).\n", iSize ); // FIXME! this may fail with malloc error too

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

	g_iAllocsCount++;
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
				sphDie ( "FATAL: delete [] on non-array block %d allocated at %s(%d)\n",
					pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );
			break;

		case MEMORY_MAGIC_PLAIN:
			if ( bArray )
				sphDie ( "FATAL: delete on array block %d allocated at %s(%d)\n",
					pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );
			break;

		case MEMORY_MAGIC_DELETED:
			sphDie ( "FATAL: double delete on block %d allocated at %s(%d)\n",
				pHeader->m_iAllocId, pHeader->m_sFile, pHeader->m_iLine );
			break;

		default:
			sphDie ( "FATAL: delete on unmanaged block at 0x%08x\n", pPtr );
			return;
	}

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
	g_iAllocsCount--;
	::free ( pHeader );
}


int sphAllocsCount ()
{
	return g_iAllocsCount;
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
		sphDie ( "FATAL: out of memory (unable to allocate %d bytes).\n", iSize ); // FIXME! this may fail with malloc error too
	return pResult;
}


void * operator new [] ( size_t iSize )
{
	void * pResult = ::malloc ( iSize );
	if ( !pResult )
		sphDie ( "FATAL: out of memory (unable to allocate %d bytes).\n", iSize ); // FIXME! this may fail with malloc error too
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

//
// $Id$
//
