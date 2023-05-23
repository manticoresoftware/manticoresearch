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

#include "ints.h"
#include "datamove.h"
#include "generics.h"
#include "log2.h"

#include <cassert>
#include <climits>

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::OpenHashTraits_T ( const OpenHashTraits_T & rhs )
{
	m_iSize = rhs.m_iSize;
	m_iUsed = rhs.m_iUsed;
	m_iMaxUsed = rhs.m_iMaxUsed;

	// as we anyway copy raw data, don't need to call ctrs with this allocation
	using Entry_Storage = typename std::aligned_storage<sizeof ( ENTRY ), alignof ( ENTRY )>::type;
	m_pHash = (ENTRY*)new Entry_Storage[m_iSize];
	sph::DefaultCopy_T<ENTRY>::CopyVoid ( m_pHash, rhs.m_pHash, m_iSize );
}

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
ENTRY * OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::FindEntry ( KEY k ) const
{
	if ( !m_iSize )
		return nullptr;

	int64_t iIndex = HASHFUNC::GetHash(k) & ( m_iSize-1 );

	while ( m_pHash[iIndex].IsUsed(*this) )
	{
		ENTRY & tEntry = m_pHash[iIndex];
		if ( tEntry.m_Key==k )
			return &tEntry;

		iIndex = ( iIndex+1 ) & ( m_iSize-1 );
	}

	return nullptr;
}

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
ENTRY & OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::AcquireEntry ( KEY k )
{
	if ( !m_iSize )
		Grow();

	DWORD uHash = HASHFUNC::GetHash(k);
	int64_t iIndex = uHash & ( m_iSize-1 );

	while (true)
	{
		// found matching key? great, return the value
		ENTRY * p = m_pHash + iIndex;
		if ( p->IsUsed(*this) )
		{
			if ( p->m_Key==k )
				return *p;
		}
		else // no matching keys? add it
		{
			// not enough space? grow the hash and force rescan
			if ( m_iUsed>=m_iMaxUsed )
			{
				Grow();
				iIndex = uHash & ( m_iSize-1 );
				continue;
			}

			// store the newly added key
			p->m_Key = k;
			p->SetUsed ( *this, true );
			m_iUsed++;
			return *p;
		}

		// no match so far, keep probing
		iIndex = ( iIndex+1 ) & ( m_iSize-1 );
	}
}

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
void OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::Grow()
{
	int64_t iNewSize = 2*Max(m_iSize,8);
	assert ( iNewSize<=UINT_MAX ); 		// sanity check

	ENTRY * pNew = new ENTRY[iNewSize];

	MoveToNewStorage ( *this, pNew, iNewSize );

	SafeDeleteArray ( m_pHash );
	m_pHash = pNew;
	m_iSize = iNewSize;
	m_iMaxUsed = GetMaxLoad ( m_iSize );
}

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
void OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::DeleteEntry ( ENTRY * pEntry )
{
	int64_t iIndex = pEntry-m_pHash;
	int64_t iNext = iIndex;
	while ( true )
	{
		iNext = ( iNext+1 ) & ( m_iSize-1 );
		ENTRY & tEntry = m_pHash[iNext];
		if ( !tEntry.IsUsed(*this) )
			break;

		int64_t iDesired = HASHFUNC::GetHash ( tEntry.m_Key ) & ( m_iSize-1 );
		if ( ( iNext>iIndex && ( iDesired<=iIndex || iDesired>iNext ) ) ||
			( iNext<iIndex && ( iDesired<=iIndex && iDesired>iNext ) ) )
		{
			m_pHash[iIndex] = m_pHash[iNext];
			iIndex = iNext;
		}
	}

	m_pHash[iIndex].SetUsed ( *this, false );
	m_iUsed--;
}

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
void OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::MoveToNewStorage ( MYTYPE & rhs, ENTRY * pNew, int64_t iNewSize ) const
{
	for ( int64_t i=0; i<m_iSize; i++ )
		if ( m_pHash[i].IsUsed(*this) )
		{
			int64_t j = HASHFUNC::GetHash ( m_pHash[i].m_Key ) & ( iNewSize-1 );
			while ( pNew[j].IsUsed(rhs) )
				j = ( j+1 ) & ( iNewSize-1 );

			pNew[j] = m_pHash[i];
			pNew[j].SetUsed ( rhs, true );
		}
}

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
void OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::Swap ( MYTYPE & rhs ) noexcept
{
	STATE::Swap ( (STATE &)rhs );

	std::swap ( m_iSize, rhs.m_iSize );
	std::swap ( m_iUsed, rhs.m_iUsed );
	std::swap ( m_iMaxUsed, rhs.m_iMaxUsed );
	std::swap ( m_pHash, rhs.m_pHash );
}

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
void OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::MoveTo ( MYTYPE & rhs ) const
{
	assert ( rhs.m_iSize>=m_iSize );
	MoveToNewStorage ( rhs, rhs.m_pHash, rhs.m_iSize );
	rhs.m_iUsed = m_iUsed;
}

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
bool OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::Delete ( KEY k )
{
	ENTRY * pEntry = FindEntry(k);
	if ( !pEntry )
		return false;

	DeleteEntry(pEntry);
	return true;
}

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
void OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::Reset ( int64_t iSize )
{
	assert ( iSize<=UINT_MAX ); 		// sanity check
	SafeDeleteArray ( m_pHash );
	if ( iSize<=0 )
	{
		m_iSize = m_iUsed = m_iMaxUsed = 0;
		return;
	}

	iSize = ( 1ULL<<sphLog2 ( iSize-1 ) );
	assert ( iSize<=UINT_MAX ); 		// sanity check
	m_pHash = new ENTRY[iSize];
	m_iSize = iSize;
	m_iUsed = 0;
	m_iMaxUsed = GetMaxLoad ( iSize );
}

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
void OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>::Clear()
{
	for ( int i=0; i<m_iSize; i++ )
		m_pHash[i] = ENTRY();

	m_iUsed = 0;
}

/////////////////////////////////////////////////////////////////////

struct HashFunc_Int64_t
{
	static FORCE_INLINE DWORD GetHash ( int64_t k )
	{
		return ( DWORD(k) * 0x607cbb77UL ) ^ ( k>>32 );
	}
};

template<typename KEY, typename VALUE> OpenHashEntry_T<KEY,VALUE>::OpenHashEntry_T() = default;
template<> inline OpenHashEntry_T<int64_t,int>::OpenHashEntry_T() : m_Key{0}, m_Value{0} {}
template<> inline OpenHashEntry_T<int64_t, DWORD>::OpenHashEntry_T() : m_Key{0}, m_Value{0} {}
template<> inline OpenHashEntry_T<int64_t,float>::OpenHashEntry_T() : m_Key{0}, m_Value{0.0f} {}
template<> inline OpenHashEntry_T<int64_t,int64_t>::OpenHashEntry_T() : m_Key{0}, m_Value{0} {}
template<> inline OpenHashEntry_T<int64_t,uint64_t>::OpenHashEntry_T() : m_Key{0}, m_Value{0} {}


template <typename KEY, typename VALUE, typename HASHFUNC, typename ENTRY, typename STATE>
OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE>::OpenHashTable_T ( const OpenHashTable_T & rhs )
	: BASE(rhs)
{}

template <typename KEY, typename VALUE, typename HASHFUNC, typename ENTRY, typename STATE>
OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE> & OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE>::operator = ( OpenHashTable_T && rhs ) noexcept
{
	BASE::Swap ( rhs );
	return *this;
}

template <typename KEY, typename VALUE, typename HASHFUNC, typename ENTRY, typename STATE>
OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE> & OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE>::operator = ( const OpenHashTable_T & rhs ) noexcept
{
	OpenHashTable_T tNew(rhs);
	*this = std::move(tNew);
	return *this;
}

template <typename KEY, typename VALUE, typename HASHFUNC, typename ENTRY, typename STATE>
VALUE * OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE>::Find ( KEY k ) const
{
	ENTRY * e = BASE::FindEntry(k);
	return e ? &e->m_Value : nullptr;
}

template <typename KEY, typename VALUE, typename HASHFUNC, typename ENTRY, typename STATE>
VALUE OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE>::FindAndDelete ( KEY k )
{
	ENTRY * pEntry = BASE::FindEntry(k);
	assert(pEntry);

	VALUE tRes = pEntry->m_Value;
	BASE::DeleteEntry(pEntry);

	return tRes;
}

template <typename KEY, typename VALUE, typename HASHFUNC, typename ENTRY, typename STATE>
bool OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE>::Add ( KEY k, const VALUE & v )
{
	int64_t u = BASE::GetLength();
	VALUE & x = Acquire(k);
	if ( u==BASE::GetLength() )
		return false; // found an existing value by k, can not add v
	x = v;

	return true;
}

template <typename KEY, typename VALUE, typename HASHFUNC, typename ENTRY, typename STATE>
VALUE & OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE>::FindOrAdd ( KEY k, const VALUE & v )
{
	int64_t u = BASE::GetLength();
	VALUE & x = Acquire(k);
	if ( u!=BASE::GetLength() )
		x = v; // did not find an existing value by k, so add v

	return x;
}

template <typename KEY, typename VALUE, typename HASHFUNC, typename ENTRY, typename STATE>
VALUE * OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE>::Iterate ( int64_t * pIndex, KEY * pKey ) const
{
	if ( !pIndex || *pIndex<0 )
		return nullptr;

	for ( int64_t i = *pIndex; i < BASE::m_iSize; ++i )
		if ( BASE::m_pHash[i].IsUsed(*this) )
		{
			*pIndex = i+1;
			if ( pKey )
				*pKey = BASE::m_pHash[i].m_Key;

			return &BASE::m_pHash[i].m_Value;
		}

	return nullptr;
}

template <typename KEY, typename VALUE, typename HASHFUNC, typename ENTRY, typename STATE>
std::pair<KEY,VALUE*> OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE>::Iterate ( int64_t & iIndex ) const
{
	assert ( iIndex>=0 );

	for ( int64_t i = iIndex; i<BASE::m_iSize; ++i )
		if ( BASE::m_pHash[i].IsUsed(*this) )
		{
			iIndex = i+1;
			return { BASE::m_pHash[i].m_Key, &BASE::m_pHash[i].m_Value};
		}

	return {0,nullptr};
}

/////////////////////////////////////////////////////////////////////

template <typename KEY, typename VALUE, typename HASHFUNC, typename ENTRY>
void OpenHashTableFastClear_T<KEY,VALUE,HASHFUNC,ENTRY>::Clear()
{
	BASE::m_uVersion++;
	BASE::m_iUsed = 0;
}

/////////////////////////////////////////////////////////////////////

template <typename KEY, typename HASHFUNC, typename ENTRY, typename STATE>
bool OpenHashSet_T<KEY,HASHFUNC,ENTRY,STATE>::Add ( KEY k )
{
	int64_t u = BASE::GetLength();
	BASE::AcquireEntry(k);
	return u!=BASE::GetLength();
}

template <typename KEY, typename HASHFUNC, typename ENTRY, typename STATE>
KEY * OpenHashSet_T<KEY,HASHFUNC,ENTRY,STATE>::Iterate ( int64_t & iIndex ) const
{
	assert ( iIndex>=0 );

	for ( int64_t i = iIndex; i<BASE::m_iSize; i++ )
		if ( BASE::m_pHash[i].IsUsed(*this) )
		{
			iIndex = i+1;
			return &BASE::m_pHash[i].m_Key;
		}

	return nullptr;
}

/////////////////////////////////////////////////////////////////////

template <typename KEY, typename HASHFUNC, typename ENTRY>
void OpenHashSetFastClear_T<KEY,HASHFUNC,ENTRY>::Clear()
{
	BASE::m_uVersion++;
	BASE::m_iUsed = 0;
}
