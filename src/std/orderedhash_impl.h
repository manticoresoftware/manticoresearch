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

#include <cassert>
#include "generics.h"


template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
unsigned int CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::HashPos ( const KEY& tKey ) const
{
	return ( (unsigned int)HASHFUNC::Hash ( tKey ) ) % LENGTH;
}

template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
typename CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::HashEntry_t* CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::FindByKey ( const KEY& tKey ) const
{
	HashEntry_t* pEntry = m_dHash[HashPos ( tKey )];

	while ( pEntry )
	{
		if ( pEntry->first == tKey )
			return pEntry;
		pEntry = pEntry->m_pNextByHash;
	}
	return nullptr;
}

template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
typename CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::HashEntry_t* CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::AddImpl ( const KEY& tKey )
{
	// check if this key is already hashed
	HashEntry_t** ppEntry = &m_dHash[HashPos ( tKey )];
	HashEntry_t* pEntry = *ppEntry;
	while ( pEntry )
	{
		if ( pEntry->first == tKey )
			return nullptr;

		ppEntry = &pEntry->m_pNextByHash;
		pEntry = pEntry->m_pNextByHash;
	}

	// it's not; let's add the entry
	assert ( !pEntry );
	assert ( !*ppEntry );

	pEntry = new HashEntry_t;
	pEntry->first = tKey;

	*ppEntry = pEntry;

	if ( !m_pFirstByOrder )
		m_pFirstByOrder = pEntry;

	if ( m_pLastByOrder )
	{
		assert ( !m_pLastByOrder->m_pNextByOrder );
		assert ( !pEntry->m_pNextByOrder );
		m_pLastByOrder->m_pNextByOrder = pEntry;
		pEntry->m_pPrevByOrder = m_pLastByOrder;
	}
	m_pLastByOrder = pEntry;

	++m_iLength;
	return pEntry;
}


/// ctor
template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::CSphOrderedHash()
{
	for ( auto& pHash : m_dHash )
		pHash = nullptr;
}

/// dtor
template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::~CSphOrderedHash()
{
	Reset();
}

/// reset
template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
void CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::Reset()
{
	assert ( ( m_pFirstByOrder && m_iLength ) || ( !m_pFirstByOrder && !m_iLength ) );
	HashEntry_t* pKill = m_pFirstByOrder;
	while ( pKill )
	{
		HashEntry_t* pNext = pKill->m_pNextByOrder;
		SafeDelete ( pKill );
		pKill = pNext;
	}

	for ( auto& pHash : m_dHash )
		pHash = nullptr;

	m_pFirstByOrder = nullptr;
	m_pLastByOrder = nullptr;
	m_iLength = 0;
}

/// add new entry
/// returns true on success
/// returns false if this key is already hashed
template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
bool CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::Add ( T&& tValue, const KEY& tKey )
{
	// check if this key is already hashed
	HashEntry_t* pEntry = AddImpl ( tKey );
	if ( !pEntry )
		return false;
	pEntry->second = std::move ( tValue );
	return true;
}

template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
bool CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::Add ( const T& tValue, const KEY& tKey )
{
	// check if this key is already hashed
	HashEntry_t* pEntry = AddImpl ( tKey );
	if ( !pEntry )
		return false;
	pEntry->second = tValue;
	return true;
}

/// add new entry
/// returns ref to just inserted or previously existed value
template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
T& CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::AddUnique ( const KEY& tKey )
{
	// check if this key is already hashed
	HashEntry_t** ppEntry = &m_dHash[HashPos ( tKey )];
	HashEntry_t* pEntry = *ppEntry;

	while ( pEntry )
	{
		if ( pEntry->first == tKey )
			return pEntry->second;

		ppEntry = &pEntry->m_pNextByHash;
		pEntry = *ppEntry;
	}

	// it's not; let's add the entry
	assert ( !pEntry );

	pEntry = new HashEntry_t;
	pEntry->first = tKey;

	*ppEntry = pEntry;

	if ( !m_pFirstByOrder )
		m_pFirstByOrder = pEntry;

	if ( m_pLastByOrder )
	{
		assert ( !m_pLastByOrder->m_pNextByOrder );
		assert ( !pEntry->m_pNextByOrder );
		m_pLastByOrder->m_pNextByOrder = pEntry;
		pEntry->m_pPrevByOrder = m_pLastByOrder;
	}
	m_pLastByOrder = pEntry;

	++m_iLength;
	return pEntry->second;
}

/// delete an entry
template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
bool CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::Delete ( const KEY& tKey )
{
	auto uHash = HashPos ( tKey );
	HashEntry_t* pEntry = m_dHash[uHash];

	HashEntry_t* pPrevEntry = nullptr;
	HashEntry_t* pToDelete = nullptr;
	while ( pEntry )
	{
		if ( pEntry->first == tKey )
		{
			pToDelete = pEntry;
			if ( pPrevEntry )
				pPrevEntry->m_pNextByHash = pEntry->m_pNextByHash;
			else
				m_dHash[uHash] = pEntry->m_pNextByHash;

			break;
		}

		pPrevEntry = pEntry;
		pEntry = pEntry->m_pNextByHash;
	}

	if ( !pToDelete )
		return false;

	if ( pToDelete->m_pPrevByOrder )
		pToDelete->m_pPrevByOrder->m_pNextByOrder = pToDelete->m_pNextByOrder;
	else
		m_pFirstByOrder = pToDelete->m_pNextByOrder;

	if ( pToDelete->m_pNextByOrder )
		pToDelete->m_pNextByOrder->m_pPrevByOrder = pToDelete->m_pPrevByOrder;
	else
		m_pLastByOrder = pToDelete->m_pPrevByOrder;

	SafeDelete ( pToDelete );
	--m_iLength;

	return true;
}


/// get value reference by key, asserting that the key exists in hash
template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
T& CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::operator[] ( const KEY& tKey ) const
{
	HashEntry_t* pEntry = FindByKey ( tKey );
	assert ( pEntry && "hash missing value in operator []" );

	return pEntry->second;
}

template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
void CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>::Swap ( CSphOrderedHash<T, KEY, HASHFUNC, LENGTH>& rhs ) noexcept
{
	HashEntry_t* dFoo[LENGTH];
	memcpy ( dFoo, m_dHash, LENGTH * sizeof ( HashEntry_t* ) );
	memcpy ( m_dHash, rhs.m_dHash, LENGTH * sizeof ( HashEntry_t* ) );
	memcpy ( rhs.m_dHash, dFoo, LENGTH * sizeof ( HashEntry_t* ) );
	::Swap ( m_pFirstByOrder, rhs.m_pFirstByOrder );
	::Swap ( m_pLastByOrder, rhs.m_pLastByOrder );
	::Swap ( m_iLength, rhs.m_iLength );
}

