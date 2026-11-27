//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "scopedlock.h"

template <typename KEY, typename VALUE, typename HELPER>
LRUCache_T<KEY,VALUE,HELPER>::LRUCache_T ( int64_t iCacheSize, int64_t iMaxBlockSize, int64_t iAdaptiveFloor )
	: m_iCacheSize ( iCacheSize )
	, m_uMaxBlockSize ( DWORD(iMaxBlockSize<0 ? iCacheSize/64 : iMaxBlockSize) )
	, m_tHash ( 1024 )
	, m_iAdaptiveFloor ( Min ( iCacheSize, iAdaptiveFloor ) )
	, m_iAdaptiveTarget ( iCacheSize )
{}

template <typename KEY, typename VALUE, typename HELPER>
LRUCache_T<KEY,VALUE,HELPER>::~LRUCache_T()
{
	LinkedEntry_t * pEntry = m_pHead;
	while ( pEntry )
	{
		LinkedEntry_t * pToDelete = pEntry;
		pEntry = pEntry->m_pNext;
		Delete(pToDelete);
	}
}

template <typename KEY, typename VALUE, typename HELPER>
bool LRUCache_T<KEY,VALUE,HELPER>::Find ( KEY tKey, VALUE & tData, Ref_t * ppRef )
{
	ScopedMutex_t tLock(m_tLock);

	LinkedEntry_t ** ppEntry = m_tHash.Find(tKey);
	if ( !ppEntry )
	{
		if ( m_iAdaptiveFloor ) Observe(nullptr);
		return false;
	}

	MoveToHead ( *ppEntry );

	(*ppEntry)->m_iRefcount.fetch_add ( 1, std::memory_order_relaxed );
	if ( ppRef ) *ppRef = &(*ppEntry)->m_iRefcount;
	tData = (*ppEntry)->m_tValue;
	if ( m_iAdaptiveFloor ) Observe(*ppEntry);
	return true;
}

template <typename KEY, typename VALUE, typename HELPER>
bool LRUCache_T<KEY,VALUE,HELPER>::Add ( KEY tKey, const VALUE & tData, Ref_t * ppRef )
{
	ScopedMutex_t tLock(m_tLock);

	// if another thread managed to add a similar block while we were uncompressing ours, let it be
	LinkedEntry_t ** ppEntry = m_tHash.Find(tKey);
	if ( ppEntry )
		return false;

	DWORD uSize = HELPER::GetSize(tData);
	DWORD uSpaceNeeded = uSize + sizeof(LinkedEntry_t);
	if ( !HaveSpaceFor ( uSpaceNeeded ) )
	{
		if ( uSpaceNeeded>m_uMaxBlockSize )
			return false;

		SweepUnused ( uSpaceNeeded );
		if ( !HaveSpaceFor ( uSpaceNeeded ) )
			return false;
	}

	LinkedEntry_t * pEntry = new LinkedEntry_t;
	*(VALUE*)pEntry = tData;
	pEntry->m_iRefcount.store ( 1, std::memory_order_relaxed );
	pEntry->m_tKey = tKey;
	pEntry->m_uSize = uSize;

	Add ( pEntry );
	if ( m_iAdaptiveFloor ) Touch(pEntry);
	if ( ppRef ) *ppRef = &pEntry->m_iRefcount;
	return true;
}

template <typename KEY, typename VALUE, typename HELPER>
int64_t LRUCache_T<KEY,VALUE,HELPER>::GetUsedBytes()
{
	ScopedMutex_t tLock(m_tLock);
	return m_iMemUsed;
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::Touch ( LinkedEntry_t * pEntry )
{
	if ( pEntry->m_uAccessEpoch==m_uAccessEpoch )
		return;
	pEntry->m_uAccessEpoch = m_uAccessEpoch;
	m_iActiveBytes += pEntry->m_uSize + sizeof(LinkedEntry_t);
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::Observe ( LinkedEntry_t * pEntry )
{
	if ( pEntry )
	{
		Touch(pEntry);
		++m_iWindowHits;
	}
	if ( ++m_iWindowAccesses==4096 )
	{
		// Require four stable windows and retain twice the recently touched bytes.
		// A 98% hit threshold allows both decoding misses from second-touch admission.
		if ( m_iWindowHits>=4015 && m_iActiveBytes<m_iMemUsed/4 )
		{
			m_iStableWindows = Min ( m_iStableWindows+1, 4 );
			if ( m_iStableWindows==4 )
				m_iAdaptiveTarget = Min ( m_iCacheSize, Max ( m_iAdaptiveFloor, m_iActiveBytes*2 ) );
		}
		else
		{
			m_iStableWindows = 0;
			m_iAdaptiveTarget = m_iCacheSize;
		}
		m_iWindowAccesses = m_iWindowHits = 0;
		m_iActiveBytes = 0;
		if ( !++m_uAccessEpoch )
		{
			m_uAccessEpoch = 1;
			for ( auto * p = m_pHead; p; p=p->m_pNext ) p->m_uAccessEpoch = 0;
		}
	}
	// Bound each lock hold while a smaller hot set releases cold storage.
	if ( !(m_iWindowAccesses%64) && m_iMemUsed>m_iAdaptiveTarget ) TrimCold();
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::TrimCold()
{
	auto * pEntry = m_pTail;
	for ( int i=0; pEntry && i<32 && m_iMemUsed>m_iAdaptiveTarget; ++i )
	{
		auto * pPrevious = pEntry->m_pPrev;
		if ( !pEntry->m_iRefcount.load(std::memory_order_acquire) && pEntry->m_uAccessEpoch!=m_uAccessEpoch
			&& pEntry->m_uAccessEpoch!=m_uAccessEpoch-1 )
			Delete(pEntry);
		pEntry = pPrevious;
	}
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::Release ( KEY tKey )
{
	ScopedMutex_t tLock(m_tLock);

	LinkedEntry_t ** ppEntry = m_tHash.Find(tKey);
	assert(ppEntry);

	LinkedEntry_t * pEntry = *ppEntry;
	ReleaseRef ( &pEntry->m_iRefcount );
}

template <typename KEY, typename VALUE, typename HELPER>
template <typename COND>
void LRUCache_T<KEY,VALUE,HELPER>::Delete ( COND && fnCond )
{
	ScopedMutex_t tLock(m_tLock);

	LinkedEntry_t * pEntry = m_pHead;
	while ( pEntry )
	{
		if ( fnCond ( pEntry->m_tKey) )
		{
			assert ( !pEntry->m_iRefcount.load(std::memory_order_acquire) );
			LinkedEntry_t * pToDelete = pEntry;
			pEntry = pEntry->m_pNext;
			Delete(pToDelete);
		}
		else
			pEntry = pEntry->m_pNext;
	}
}


template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::MoveToHead ( LinkedEntry_t * pEntry )
{
	assert ( pEntry );

	if ( pEntry==m_pHead )
		return;

	if ( m_pTail==pEntry )
		m_pTail = pEntry->m_pPrev;

	if ( pEntry->m_pPrev )
		pEntry->m_pPrev->m_pNext = pEntry->m_pNext;

	if ( pEntry->m_pNext )
		pEntry->m_pNext->m_pPrev = pEntry->m_pPrev;

	pEntry->m_pPrev = nullptr;
	pEntry->m_pNext = m_pHead;
	m_pHead->m_pPrev = pEntry;
	m_pHead = pEntry;
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::Add ( LinkedEntry_t * pEntry )
{
	pEntry->m_pNext = m_pHead;
	if ( m_pHead )
		m_pHead->m_pPrev = pEntry;

	if ( !m_pTail )
		m_pTail = pEntry;

	m_pHead = pEntry;

	Verify ( m_tHash.Add ( pEntry->m_tKey, pEntry ) );

	m_iMemUsed += pEntry->m_uSize + sizeof(LinkedEntry_t);
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::Delete ( LinkedEntry_t * pEntry )
{
	Verify ( m_tHash.Delete ( pEntry->m_tKey ) );

	if ( m_pHead == pEntry )
		m_pHead = pEntry->m_pNext;

	if ( m_pTail==pEntry )
		m_pTail = pEntry->m_pPrev;

	if ( pEntry->m_pPrev )
		pEntry->m_pPrev->m_pNext = pEntry->m_pNext;

	if ( pEntry->m_pNext )
		pEntry->m_pNext->m_pPrev = pEntry->m_pPrev;

	m_iMemUsed -= pEntry->m_uSize + sizeof(LinkedEntry_t);
	assert ( m_iMemUsed>=0 );

	HELPER::Reset ( pEntry->m_tValue );
	SafeDelete(pEntry);
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::SweepUnused ( DWORD uSpaceNeeded )
{
	// least recently used blocks are the tail
	LinkedEntry_t * pEntry = m_pTail;
	while ( pEntry && !HaveSpaceFor ( uSpaceNeeded ) )
	{
		if ( !pEntry->m_iRefcount.load(std::memory_order_acquire) )
		{
			assert ( !pEntry->m_iRefcount.load(std::memory_order_acquire) );
			LinkedEntry_t * pToDelete = pEntry;
			pEntry = pEntry->m_pPrev;
			Delete(pToDelete);
		}
		else
			pEntry = pEntry->m_pPrev;
	}
}

template <typename KEY, typename VALUE, typename HELPER>
bool LRUCache_T<KEY,VALUE,HELPER>::HaveSpaceFor ( DWORD uSpaceNeeded ) const
{
	return m_iMemUsed+uSpaceNeeded <= m_iCacheSize;
}
