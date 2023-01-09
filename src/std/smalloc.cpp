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

#include "smalloc.h"

#include <cassert>
#include "mutex.h"

//////////////////////////////////////////////////////////////////////////


#if WITH_SMALLALLOC

////////////////////////////////////////////////////////////////////////////////
/// FixedAllocator_c, PtrAttrAllocator_c
/// Provides fast alloc/dealloc for small objects
/// (large objects will be redirected to standard new/delete)
////////////////////////////////////////////////////////////////////////////////
static constexpr size_t DEFAULT_CHUNK_SIZE = 4096;


////////////////////////////////////////////////////////////////////////////////
/// FixedAllocator_c
/// Represents storage for any num of objects of fixed size
////////////////////////////////////////////////////////////////////////////////
class FixedAllocator_c: ISphNoncopyable
{
	struct Chunk_t
	{
		void Init ( int iBlockSize, BYTE uBlocks );
		void Release() const;

		BYTE* Allocate ( int iBlockSize );
		void Deallocate ( BYTE* pBlob, int iBlockSize );

		BYTE* m_pData;
		BYTE m_uFirstAvailable;
		BYTE m_uAvailable;
	};

	int m_iBlockSize;					// size of blobs I can serve
	Chunk_t* m_pAllocChunk = nullptr;	// shortcut to last alloc
	Chunk_t* m_pDeallocChunk = nullptr; // shortcut to last dealloc
	Chunk_t* m_pLastFree = nullptr;		//
	CSphVector<Chunk_t> m_dChunks;		// my chunks
	BYTE m_uNumBlocks;					// # of blocks per chunk

private:
	void Swap ( FixedAllocator_c& rhs );
	void DoDeallocate ( BYTE* pBlob );
	Chunk_t* VicinityFind ( const BYTE* pBlob );

public:
	explicit FixedAllocator_c ( int iBlockSize = 0 );
	FixedAllocator_c ( FixedAllocator_c&& ) noexcept;
	FixedAllocator_c& operator= ( FixedAllocator_c&& ) noexcept;
	~FixedAllocator_c();

	// allocate block of my m_iBlockSize
	BYTE* Allocate();

	// Deallocate a memory block previously allocated with Allocate()
	// (if that's not the case, the behavior is undefined)
	void Deallocate ( BYTE* pBlob );

	inline int BlockSize() const
	{
		return m_iBlockSize;
	}

	// debug/diagnostic
	size_t GetAllocatedSize() const; // how many allocated right now
	size_t GetReservedSize() const;	 // how many pooled from the sys right now
};

////////////////////////////////////////////////////////////////////////////////
/// FixedAllocator_c::Chunk_t
/// Represents a blob of memory for few objects of fixed size
////////////////////////////////////////////////////////////////////////////////
void FixedAllocator_c::Chunk_t::Init ( int iBlockSize, BYTE uBlocks )
{
	assert ( iBlockSize > 0 );
	assert ( uBlocks > 0 );

	// Overflow check
	assert ( ( iBlockSize * uBlocks ) / iBlockSize == uBlocks );

	m_pData = new BYTE[iBlockSize * uBlocks];
	m_uAvailable = uBlocks;
	m_uFirstAvailable = 0;

	auto p = m_pData;
	for ( BYTE i = 0; i < uBlocks; p += iBlockSize )
		*p = ++i;
}

void FixedAllocator_c::Chunk_t::Release() const
{
	delete[] m_pData;
}

BYTE* FixedAllocator_c::Chunk_t::Allocate ( int iBlockSize )
{
	if ( !m_uAvailable )
		return nullptr;

	assert ( ( m_uFirstAvailable * iBlockSize ) / iBlockSize == m_uFirstAvailable );

	auto* pResult = m_pData + ( m_uFirstAvailable * iBlockSize );
	m_uFirstAvailable = *pResult;
	--m_uAvailable;

	return pResult;
}

void FixedAllocator_c::Chunk_t::Deallocate ( BYTE* pBlob, int iBlockSize )
{
	assert ( pBlob >= m_pData );

	// Alignment check
	assert ( ( pBlob - m_pData ) % iBlockSize == 0 );

	*pBlob = m_uFirstAvailable;
	m_uFirstAvailable = (BYTE)( ( pBlob - m_pData ) / iBlockSize );

	// Truncation check
	assert ( m_uFirstAvailable == ( pBlob - m_pData ) / iBlockSize );
	++m_uAvailable;
}


FixedAllocator_c::FixedAllocator_c ( int iBlockSize )
	: m_iBlockSize ( iBlockSize )
{
	if ( !iBlockSize )
		return;

	assert ( m_iBlockSize > 0 );
	m_uNumBlocks = (BYTE)Min ( DEFAULT_CHUNK_SIZE / iBlockSize, BYTE ( 0xFF ) );
	assert ( m_uNumBlocks && "Too large iBlocksize passed" );
}

FixedAllocator_c::FixedAllocator_c ( FixedAllocator_c&& rhs ) noexcept
{
	Swap ( rhs );
}

FixedAllocator_c& FixedAllocator_c::operator= ( FixedAllocator_c&& rhs ) noexcept
{
	Swap ( rhs );
	return *this;
}

FixedAllocator_c::~FixedAllocator_c()
{
	for ( auto& dChunk : m_dChunks )
	{
		assert ( dChunk.m_uAvailable == m_uNumBlocks );
		dChunk.Release();
	}
}

void FixedAllocator_c::Swap ( FixedAllocator_c& rhs )
{
	::Swap ( m_iBlockSize, rhs.m_iBlockSize );
	::Swap ( m_uNumBlocks, rhs.m_uNumBlocks );
	m_dChunks.SwapData ( rhs.m_dChunks );
	::Swap ( m_pAllocChunk, rhs.m_pAllocChunk );
	::Swap ( m_pDeallocChunk, rhs.m_pDeallocChunk );
}

BYTE* FixedAllocator_c::Allocate()
{
	if ( !m_pAllocChunk || !m_pAllocChunk->m_uAvailable )
	{
		for ( auto& dChunk : m_dChunks )
		{
			if ( dChunk.m_uAvailable )
			{
				m_pAllocChunk = &dChunk;
				return m_pAllocChunk->Allocate ( m_iBlockSize );
			}
		}

		// loop finished, we're still here. Create new chunk then
		Chunk_t& dNewChunk = m_dChunks.Add();
		dNewChunk.Init ( m_iBlockSize, m_uNumBlocks );
		m_pDeallocChunk = m_dChunks.begin();
		m_pAllocChunk = &m_dChunks.Last();
	}

	assert ( m_pAllocChunk );
	assert ( m_pAllocChunk->m_uAvailable );

	return m_pAllocChunk->Allocate ( m_iBlockSize );
}

void FixedAllocator_c::Deallocate ( BYTE* pBlob )
{
	assert ( !m_dChunks.IsEmpty() );
	assert ( m_dChunks.begin() <= m_pDeallocChunk );
	assert ( m_dChunks.end() > m_pDeallocChunk );

	m_pDeallocChunk = VicinityFind ( pBlob );
	assert ( m_pDeallocChunk );

	DoDeallocate ( pBlob );
}

// Finds the chunk corresponding to a pointer, using an efficient search
FixedAllocator_c::Chunk_t* FixedAllocator_c::VicinityFind ( const BYTE* pBlob )
{
	assert ( !m_dChunks.IsEmpty() );
	assert ( m_pDeallocChunk );

	const int iChunkLength = m_uNumBlocks * m_iBlockSize;

	Chunk_t* pLo = m_pDeallocChunk;
	Chunk_t* pHi = m_pDeallocChunk + 1;
	Chunk_t* pLoBound = m_dChunks.begin();
	Chunk_t* pHiBound = m_dChunks.end();

	// Special case: deallocChunk_ is the last in the array
	if ( pHi == pHiBound )
		pHi = nullptr;

	while ( true )
	{
		if ( pLo )
		{
			if ( pBlob >= pLo->m_pData && pBlob < pLo->m_pData + iChunkLength )
				return pLo;

			if ( pLo == pLoBound )
				pLo = nullptr;
			else
				--pLo;
		}

		if ( pHi )
		{
			if ( pBlob >= pHi->m_pData && pBlob < pHi->m_pData + iChunkLength )
				return pHi;

			if ( ++pHi == pHiBound )
				pHi = nullptr;
		}
	}
}

// Performs deallocation. Assumes m_pDeallocChunk points to the correct chunk
void FixedAllocator_c::DoDeallocate ( BYTE* pBlob )
{
	assert ( m_pDeallocChunk->m_pData <= pBlob );
	assert ( m_pDeallocChunk->m_pData + m_uNumBlocks * m_iBlockSize > pBlob );

	// call into the chunk, will adjust the inner list but won't release memory
	m_pDeallocChunk->Deallocate ( pBlob, m_iBlockSize );

	if ( m_pDeallocChunk->m_uAvailable == m_uNumBlocks )
	{
		// deallocChunk_ is completely free, should we release it?
		if ( m_pLastFree != m_pDeallocChunk )
		{
			if ( m_pLastFree && m_pLastFree->m_uAvailable == m_uNumBlocks )
			{
				// Two free blocks, discard one
				m_pLastFree->Release();
				auto iIdx = m_pLastFree - m_dChunks.begin();
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

size_t FixedAllocator_c::GetAllocatedSize() const
{
	size_t uAccum = 0;
	for ( const auto& dChunk : m_dChunks )
		uAccum += m_uNumBlocks - dChunk.m_uAvailable;
	return uAccum * m_iBlockSize;
}

size_t FixedAllocator_c::GetReservedSize() const
{
	auto uSize = Max ( m_iBlockSize * m_uNumBlocks, 4096 );
	return (size_t)m_dChunks.GetLength() * uSize;
}

////////////////////////////////////////////////////////////////////////////////
/// class PtrAttrAllocator_c
/// Offers fast allocations/deallocations
////////////////////////////////////////////////////////////////////////////////
class PtrAttrAllocator_c: public ISphNoncopyable
{
	CSphSwapVector<FixedAllocator_c> m_dPool;
	FixedAllocator_c* m_pLastAlloc;
	FixedAllocator_c* m_pLastDealloc;
	CSphMutex m_dAllocMutex;

private:
	int LowerBound ( int iBytes ) REQUIRES ( m_dAllocMutex );

public:
	PtrAttrAllocator_c()
		: m_pLastAlloc ( nullptr )
		, m_pLastDealloc ( nullptr )
	{
		m_dPool.Reserve ( MAX_SMALL_OBJECT_SIZE );
	}

	BYTE* Allocate ( int iBytes );
	void Deallocate ( BYTE* pBlob, int iBytes );

	// debug/diagnostic
	size_t GetAllocatedSize(); // how many allocated right now
	size_t GetReservedSize();  // how many pooled from the sys right now
};

// returns lower bound for fixed allocators sized 'iBytes'.
// i.e. idx of pool (asc sorted) where you can insert new not breaking the sequence.
int PtrAttrAllocator_c::LowerBound ( int iBytes )
{
	if ( m_dPool.IsEmpty() )
		return -1;

	auto pStart = m_dPool.begin();
	auto pLast = &m_dPool.Last();

	if ( pStart->BlockSize() >= iBytes )
		return 0;

	if ( pLast->BlockSize() < iBytes )
		return -1;

	while ( pLast - pStart > 1 )
	{
		auto pMid = pStart + ( pLast - pStart ) / 2;
		if ( pMid->BlockSize() < iBytes )
			pStart = pMid;
		else
			pLast = pMid;
	}
	return int ( pLast - m_dPool.begin() );
}

// Allocates 'iBytes' memory
// Uses an internal pool of FixedAllocator_c objects for small objects
BYTE* PtrAttrAllocator_c::Allocate ( int iBytes )
{
	if ( iBytes > MAX_SMALL_OBJECT_SIZE )
		return new BYTE[iBytes];

	ScopedMutex_t tScopedLock ( m_dAllocMutex );
	if ( m_pLastAlloc && m_pLastAlloc->BlockSize() == iBytes )
		return m_pLastAlloc->Allocate();

	auto i = LowerBound ( iBytes );
	if ( i < 0 ) // required size > any other in the pool
	{
		i = m_dPool.GetLength();
		m_dPool.Add ( FixedAllocator_c ( iBytes ) );
		m_pLastDealloc = m_dPool.begin();
	} else if ( m_dPool[i].BlockSize() != iBytes )
	{
		FixedAllocator_c dAlloc ( iBytes );
		m_dPool.Insert ( i, dAlloc );
		m_pLastDealloc = m_dPool.begin();
	}
	m_pLastAlloc = m_dPool.begin() + i;
	return m_pLastAlloc->Allocate();
}

// Deallocates memory previously allocated with Allocate
// (undefined behavior if you pass any other pointer)
void PtrAttrAllocator_c::Deallocate ( BYTE* pBlob, int iBytes )
{
	if ( iBytes > MAX_SMALL_OBJECT_SIZE )
	{
		SafeDeleteArray ( pBlob );
		return;
	}

	ScopedMutex_t tScopedLock ( m_dAllocMutex );
	if ( m_pLastDealloc && m_pLastDealloc->BlockSize() == iBytes )
	{
		m_pLastDealloc->Deallocate ( pBlob );
		return;
	}

	auto i = LowerBound ( iBytes );
	assert ( i >= 0 );
	assert ( m_dPool[i].BlockSize() == iBytes );
	m_pLastDealloc = m_dPool.begin() + i;
	m_pLastDealloc->Deallocate ( pBlob );
}

size_t PtrAttrAllocator_c::GetAllocatedSize()
{
	ScopedMutex_t tScopedLock ( m_dAllocMutex );
	size_t uAccum = 0;
	for ( const auto& dAlloc : m_dPool )
		uAccum += dAlloc.GetAllocatedSize();
	return uAccum;
}

size_t PtrAttrAllocator_c::GetReservedSize()
{
	ScopedMutex_t tScopedLock ( m_dAllocMutex );
	size_t uAccum = 0;
	for ( const auto& dAlloc : m_dPool )
		uAccum += dAlloc.GetReservedSize();
	return uAccum;
}

PtrAttrAllocator_c& SmallAllocator()
{
	static PtrAttrAllocator_c dAllocator;
	return dAllocator;
}


BYTE* sphAllocateSmall ( int iBytes )
{
	assert ( iBytes > 0 );
	if ( iBytes > MAX_SMALL_OBJECT_SIZE )
		return new BYTE[iBytes];

	return SmallAllocator().Allocate ( iBytes );
}

void sphDeallocateSmall ( const BYTE* pBlob, int iBytes )
{
	if ( iBytes > MAX_SMALL_OBJECT_SIZE )
	{
		SafeDeleteArray ( pBlob );
		return;
	}
	SmallAllocator().Deallocate ( const_cast<BYTE*> ( pBlob ), iBytes );
}

size_t sphGetSmallAllocatedSize()
{
	return SmallAllocator().GetAllocatedSize();
}

size_t sphGetSmallReservedSize()
{
	return SmallAllocator().GetReservedSize();
}

#endif
