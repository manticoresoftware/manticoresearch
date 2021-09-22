//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _lrucache_
#define _lrucache_

#include "sphinxstd.h"

template <typename KEY, typename VALUE, typename HELPER>
class LRUCache_T
{
public:
			LRUCache_T ( int64_t iCacheSize );
			~LRUCache_T();

	bool	Find ( KEY tKey, VALUE & tData );
	bool	Add ( KEY tKey, const VALUE & tData );
	void	Release ( KEY tKey );

protected:
	struct LinkedBlock_t
	{
		VALUE			m_tValue;
		DWORD			m_uSize = 0;
		int				m_iRefcount = 0;
		LinkedBlock_t *	m_pPrev = nullptr;
		LinkedBlock_t *	m_pNext = nullptr;
		KEY				m_tKey;
	};

	LinkedBlock_t *		m_pHead = nullptr;
	LinkedBlock_t *		m_pTail = nullptr;
	int64_t				m_iCacheSize = 0;
	int64_t				m_iMemUsed = 0;
	CSphMutex			m_tLock;
	OpenHash_T<LinkedBlock_t *, KEY, HELPER> m_tHash;

	void	Delete ( LinkedBlock_t * pBlock );

private:
	void	MoveToHead ( LinkedBlock_t * pBlock );
	void	Add ( LinkedBlock_t * pBlock );
	void	SweepUnused ( DWORD uSpaceNeeded );
	bool	HaveSpaceFor ( DWORD uSpaceNeeded ) const;
};

template <typename KEY, typename VALUE, typename HELPER>
LRUCache_T<KEY,VALUE,HELPER>::LRUCache_T ( int64_t iCacheSize )
	: m_iCacheSize ( iCacheSize )
	, m_tHash ( 1024 )
{}

template <typename KEY, typename VALUE, typename HELPER>
LRUCache_T<KEY,VALUE,HELPER>::~LRUCache_T()
{
	LinkedBlock_t * pBlock = m_pHead;
	while ( pBlock )
	{
		LinkedBlock_t * pToDelete = pBlock;
		pBlock = pBlock->m_pNext;
		Delete(pToDelete);
	}
}

template <typename KEY, typename VALUE, typename HELPER>
bool LRUCache_T<KEY,VALUE,HELPER>::Find ( KEY tKey, VALUE & tData )
{
	ScopedMutex_t tLock(m_tLock);

	LinkedBlock_t ** ppBlock = m_tHash.Find(tKey);
	if ( !ppBlock )
		return false;

	MoveToHead ( *ppBlock );

	(*ppBlock)->m_iRefcount++;
	tData = (*ppBlock)->m_tValue;
	return true;
}

template <typename KEY, typename VALUE, typename HELPER>
bool LRUCache_T<KEY,VALUE,HELPER>::Add ( KEY tKey, const VALUE & tData )
{
	ScopedMutex_t tLock(m_tLock);

	// if another thread managed to add a similar block while we were uncompressing ours, let it be
	LinkedBlock_t ** ppBlock = m_tHash.Find(tKey);
	if ( ppBlock )
		return false;

	DWORD uSize = HELPER::GetSize(tData);
	DWORD uSpaceNeeded = uSize + sizeof(LinkedBlock_t);
	if ( !HaveSpaceFor ( uSpaceNeeded ) )
	{
		DWORD MAX_BLOCK_SIZE = m_iCacheSize/64;
		if ( uSpaceNeeded>MAX_BLOCK_SIZE )
			return false;

		SweepUnused ( uSpaceNeeded );
		if ( !HaveSpaceFor ( uSpaceNeeded ) )
			return false;
	}

	LinkedBlock_t * pBlock = new LinkedBlock_t;
	*(VALUE*)pBlock = tData;
	pBlock->m_iRefcount++;
	pBlock->m_tKey = tKey;
	pBlock->m_uSize = uSize;

	Add ( pBlock );
	return true;
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::Release ( KEY tKey )
{
	ScopedMutex_t tLock(m_tLock);

	LinkedBlock_t ** ppBlock = m_tHash.Find(tKey);
	assert(ppBlock);

	LinkedBlock_t * pBlock = *ppBlock;
	pBlock->m_iRefcount--;
	assert ( pBlock->m_iRefcount>=0 );
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::MoveToHead ( LinkedBlock_t * pBlock )
{
	assert ( pBlock );

	if ( pBlock==m_pHead )
		return;

	if ( m_pTail==pBlock )
		m_pTail = pBlock->m_pPrev;

	if ( pBlock->m_pPrev )
		pBlock->m_pPrev->m_pNext = pBlock->m_pNext;

	if ( pBlock->m_pNext )
		pBlock->m_pNext->m_pPrev = pBlock->m_pPrev;

	pBlock->m_pPrev = nullptr;
	pBlock->m_pNext = m_pHead;
	m_pHead->m_pPrev = pBlock;
	m_pHead = pBlock;
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::Add ( LinkedBlock_t * pBlock )
{
	pBlock->m_pNext = m_pHead;
	if ( m_pHead )
		m_pHead->m_pPrev = pBlock;

	if ( !m_pTail )
		m_pTail = pBlock;

	m_pHead = pBlock;

	Verify ( m_tHash.Add ( pBlock->m_tKey, pBlock ) );

	m_iMemUsed += pBlock->m_uSize + sizeof(LinkedBlock_t);
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::Delete ( LinkedBlock_t * pBlock )
{
	Verify ( m_tHash.Delete ( pBlock->m_tKey ) );

	if ( m_pHead == pBlock )
		m_pHead = pBlock->m_pNext;

	if ( m_pTail==pBlock )
		m_pTail = pBlock->m_pPrev;

	if ( pBlock->m_pPrev )
		pBlock->m_pPrev->m_pNext = pBlock->m_pNext;

	if ( pBlock->m_pNext )
		pBlock->m_pNext->m_pPrev = pBlock->m_pPrev;

	m_iMemUsed -= pBlock->m_uSize + sizeof(LinkedBlock_t);
	assert ( m_iMemUsed>=0 );

	HELPER::Reset ( pBlock->m_tValue );
	SafeDelete(pBlock);
}

template <typename KEY, typename VALUE, typename HELPER>
void LRUCache_T<KEY,VALUE,HELPER>::SweepUnused ( DWORD uSpaceNeeded )
{
	// least recently used blocks are the tail
	LinkedBlock_t * pBlock = m_pTail;
	while ( pBlock && !HaveSpaceFor ( uSpaceNeeded ) )
	{
		if ( !pBlock->m_iRefcount )
		{
			assert ( !pBlock->m_iRefcount );
			LinkedBlock_t * pToDelete = pBlock;
			pBlock = pBlock->m_pPrev;
			Delete(pToDelete);
		}
		else
			pBlock = pBlock->m_pPrev;
	}
}

template <typename KEY, typename VALUE, typename HELPER>
bool LRUCache_T<KEY,VALUE,HELPER>::HaveSpaceFor ( DWORD uSpaceNeeded ) const
{
	return m_iMemUsed+uSpaceNeeded <= m_iCacheSize;
}

#endif // _lrucache_
