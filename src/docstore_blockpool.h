// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// Distributed under the GNU General Public License, version 3 or later.

#pragma once

#include "std/mm.h"
#include "std/mutex.h"
#include "std/scopedlock.h"
#include "std/sys.h"

#include <cstdlib>
#include <new>

// Pack small payloads into independently releasable mappings, with one empty spare per shard.
class DocstoreBlockPool_c
{
public:
	explicit DocstoreBlockPool_c ( int64_t iSpareLimit = SLAB_SIZE ) : m_bKeepSpare(iSpareLimit>=SLAB_SIZE) {}
	~DocstoreBlockPool_c()
	{
		if ( m_pSpare ) UnmapSlab(m_pSpare);
		assert ( !m_iMappedBytes );
	}

	static uint64_t AllocationSize ( DWORD uSize )
	{
		const uint64_t uAlign = uSize<=MAX_SMALL ? 256 : GetMemPageSize();
		return (uint64_t(uSize)+uAlign-1)/uAlign*uAlign;
	}

	BYTE * Allocate ( DWORD uSize )
	{
		const auto uCharge = AllocationSize(uSize);
		if ( !uCharge || uCharge>UINT32_MAX ) return nullptr;
		ScopedMutex_t tLock(m_tLock);
		if ( uSize>MAX_SMALL )
		{
			auto * pData = Map(uCharge);
			if ( pData ) m_iMappedBytes += uCharge;
			return pData;
		}

		auto * pSlab = m_dAvailable[uCharge/256];
		if ( pSlab==m_pSpare && pSlab && pSlab->m_pNext ) pSlab = pSlab->m_pNext;
		if ( !pSlab )
		{
			if ( m_pSpare )
			{
				pSlab = m_pSpare;
				Remove(pSlab);
				m_pSpare = nullptr;
			}
			else
			{
				pSlab = MapSlab();
				if ( !pSlab ) return nullptr;
			}
			new (pSlab) Slab_t{};
			pSlab->m_pOwner = this;
			pSlab->m_uSlotSize = DWORD(uCharge);
			pSlab->m_uSlots = (SLAB_SIZE-HEADER_SIZE)/uCharge;
			Add(pSlab);
		}
		if ( pSlab==m_pSpare ) m_pSpare = nullptr;
		BYTE * pData = pSlab->m_pFree;
		if ( pData ) pSlab->m_pFree = *(BYTE **)pData;
		else pData = (BYTE *)pSlab+HEADER_SIZE+pSlab->m_uNextSlot++*uCharge;
		if ( ++pSlab->m_uLive==pSlab->m_uSlots ) Remove(pSlab);
		return pData;
	}

	void Free ( BYTE * pData, DWORD uSize )
	{
		ScopedMutex_t tLock(m_tLock);
		if ( uSize>MAX_SMALL )
		{
			const auto uCharge = AllocationSize(uSize);
			Unmap ( pData, uCharge );
			m_iMappedBytes -= uCharge;
			return;
		}
		auto * pSlab = (Slab_t *)((uintptr_t)pData & ~(uintptr_t(SLAB_SIZE)-1));
		assert ( pSlab->m_pOwner==this && pSlab->m_uLive );
		if ( pSlab->m_uLive--==pSlab->m_uSlots ) Add(pSlab);
		if ( !pSlab->m_uLive && (m_pSpare || !m_bKeepSpare) )
		{
			Remove(pSlab);
			UnmapSlab(pSlab);
			return;
		}
		*(BYTE **)pData = pSlab->m_pFree;
		pSlab->m_pFree = pData;
		if ( !pSlab->m_uLive ) m_pSpare = pSlab;
	}

	void GetStats ( int64_t & iMapped, int64_t & iSpare )
	{
		ScopedMutex_t tLock(m_tLock);
		iMapped += m_iMappedBytes;
		iSpare += m_pSpare ? SLAB_SIZE : 0;
	}

private:
	static constexpr DWORD SLAB_SIZE = 64*1024;
	static constexpr DWORD HEADER_SIZE = 256;
	static constexpr DWORD MAX_SMALL = 31*1024;
	struct Slab_t
	{
		DocstoreBlockPool_c * m_pOwner = nullptr;
		Slab_t * m_pPrev = nullptr;
		Slab_t * m_pNext = nullptr;
		BYTE * m_pFree = nullptr;
		DWORD m_uSlotSize = 0;
		DWORD m_uSlots = 0;
		DWORD m_uLive = 0;
		DWORD m_uNextSlot = 0;
	};
	static_assert ( sizeof(Slab_t)<=HEADER_SIZE, "slab header exceeds reserved space" );
	CSphMutex m_tLock;
	Slab_t * m_dAvailable[MAX_SMALL/256+1] {};
	Slab_t * m_pSpare = nullptr;
	int64_t m_iMappedBytes = 0;
	bool m_bKeepSpare;

	static BYTE * Map ( size_t uSize )
	{
#if _WIN32
		return (BYTE *)VirtualAlloc ( nullptr, uSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE );
#else
		auto * pData = (BYTE *)mmalloc(uSize);
		return mmapvalid(pData) ? pData : nullptr;
#endif
	}

	static void Unmap ( void * pData, size_t uSize )
	{
#if _WIN32
		const bool bFreed = !!VirtualFree ( pData, 0, MEM_RELEASE );
#else
		const bool bFreed = mmfree ( pData, uSize )==0;
#endif
		if ( !bFreed ) std::abort();
	}

	Slab_t * MapSlab()
	{
#if _WIN32
		auto * pData = Map(SLAB_SIZE);
		if ( !pData ) return nullptr;
		assert ( !((uintptr_t)pData & (SLAB_SIZE-1)) );
#else
		auto * pRaw = Map(2*SLAB_SIZE);
		if ( !pRaw ) return nullptr;
		auto * pData = (BYTE *)(((uintptr_t)pRaw+SLAB_SIZE-1) & ~(uintptr_t(SLAB_SIZE)-1));
		const size_t uPrefix = pData-pRaw;
		if ( uPrefix ) Unmap ( pRaw, uPrefix );
		Unmap ( pData+SLAB_SIZE, SLAB_SIZE-uPrefix );
#endif
		m_iMappedBytes += SLAB_SIZE;
		return (Slab_t *)pData;
	}

	void UnmapSlab ( Slab_t * pSlab )
	{
		Unmap ( pSlab, SLAB_SIZE );
		m_iMappedBytes -= SLAB_SIZE;
	}

	void Add ( Slab_t * pSlab )
	{
		auto & pHead = m_dAvailable[pSlab->m_uSlotSize/256];
		pSlab->m_pPrev = nullptr;
		pSlab->m_pNext = pHead;
		if ( pHead ) pHead->m_pPrev = pSlab;
		pHead = pSlab;
	}

	void Remove ( Slab_t * pSlab )
	{
		if ( pSlab->m_pPrev ) pSlab->m_pPrev->m_pNext = pSlab->m_pNext;
		else m_dAvailable[pSlab->m_uSlotSize/256] = pSlab->m_pNext;
		if ( pSlab->m_pNext ) pSlab->m_pNext->m_pPrev = pSlab->m_pPrev;
		pSlab->m_pPrev = pSlab->m_pNext = nullptr;
	}
};
