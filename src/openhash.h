//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _openhash_
#define _openhash_

#include "sphinxstd.h"

struct HashFunc_Int64_t
{
	static DWORD GetHash ( int64_t k )
	{
		return ( DWORD(k) * 0x607cbb77UL ) ^ ( k>>32 );
	}
};


/// simple open-addressing hash
template < typename VALUE, typename KEY, typename HASHFUNC=HashFunc_Int64_t >
class OpenHash_T
{
public:
	using MYTYPE = OpenHash_T<VALUE,KEY,HASHFUNC>;

	/// initialize hash of a given initial size
	explicit OpenHash_T ( int64_t iSize=256 )
	{
		Reset ( iSize );
	}

	OpenHash_T ( const OpenHash_T& rhs )
	{
		m_iSize = rhs.m_iSize;
		m_iUsed = rhs.m_iUsed;
		m_iMaxUsed = rhs.m_iMaxUsed;

		// as we anyway copy raw data, don't need to call ctrs with this allocation
		using Entry_Storage = typename std::aligned_storage<sizeof ( Entry_t ), alignof ( Entry_t )>::type;
		m_pHash = (Entry_t*)new Entry_Storage[m_iSize];
		sph::DefaultCopy_T<Entry_t>::CopyVoid ( m_pHash, rhs.m_pHash, m_iSize );
	}

	OpenHash_T ( OpenHash_T&& rhs ) noexcept
	{
		Swap ( rhs );
	}

	OpenHash_T& operator= ( OpenHash_T rhs ) noexcept
	{
		Swap ( rhs );
		return *this;
	}

	~OpenHash_T()
	{
		SafeDeleteArray ( m_pHash );
	}

	/// reset to a given size
	void Reset ( int64_t iSize )
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
		m_pHash = new Entry_t[iSize];
		m_iSize = iSize;
		m_iUsed = 0;
		m_iMaxUsed = GetMaxLoad ( iSize );
	}

	void Clear()
	{
		for ( int i=0; i<m_iSize; i++ )
			m_pHash[i] = Entry_t();

		m_iUsed = 0;
	}

	/// acquire value by key (ie. get existing hashed value, or add a new default value)
	VALUE & Acquire ( KEY k )
	{
		DWORD uHash = HASHFUNC::GetHash(k);
		int64_t iIndex = uHash & ( m_iSize-1 );

		while (true)
		{
			// found matching key? great, return the value
			Entry_t * p = m_pHash + iIndex;
			if ( p->m_bUsed && p->m_Key==k )
				return p->m_Value;

			// no matching keys? add it
			if ( !p->m_bUsed )
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
				p->m_bUsed = true;
				m_iUsed++;
				return p->m_Value;
			}

			// no match so far, keep probing
			iIndex = ( iIndex+1 ) & ( m_iSize-1 );
		}
	}

	/// find an existing value by key
	VALUE * Find ( KEY k ) const
	{
		Entry_t * e = FindEntry(k);
		return e ? &e->m_Value : nullptr;
	}

	/// add or fail (if key already exists)
	bool Add ( KEY k, const VALUE & v )
	{
		int64_t u = m_iUsed;
		VALUE & x = Acquire(k);
		if ( u==m_iUsed )
			return false; // found an existing value by k, can not add v
		x = v;

		return true;
	}

	/// find existing value, or add a new value
	VALUE & FindOrAdd ( KEY k, const VALUE & v )
	{
		int64_t u = m_iUsed;
		VALUE & x = Acquire(k);
		if ( u!=m_iUsed )
			x = v; // did not find an existing value by k, so add v

		return x;
	}

	/// delete by key
	bool Delete ( KEY k )
	{
		Entry_t * pEntry = FindEntry(k);
		if ( !pEntry )
			return false;

		int64_t iIndex = pEntry-m_pHash;
		int64_t iNext = iIndex;
		while ( true )
		{
			iNext = ( iNext+1 ) & ( m_iSize-1 );
			Entry_t & tEntry = m_pHash[iNext];
			if ( !tEntry.m_bUsed )
				break;

			int64_t iDesired = HASHFUNC::GetHash ( tEntry.m_Key ) & ( m_iSize-1 );
			if ( ( iNext>iIndex && ( iDesired<=iIndex || iDesired>iNext ) ) ||
				 ( iNext<iIndex && ( iDesired<=iIndex && iDesired>iNext ) ) )
			{
				m_pHash[iIndex] = m_pHash[iNext];
				iIndex = iNext;
			}
		}

		m_pHash[iIndex].m_bUsed = false;
		m_iUsed--;

		return true;
	}

	/// get number of inserted key-value pairs
	int64_t GetLength() const		{ return m_iUsed; }
	int64_t GetLengthBytes() const	{ return m_iSize * sizeof ( Entry_t ); }
	 
	/// iterate the hash by entry index, starting from 0
	/// finds the next alive key-value pair starting from the given index
	/// returns that pair and updates the index on success
	/// returns NULL when the hash is over
	VALUE * Iterate ( int64_t * pIndex, KEY * pKey ) const
	{
		if ( !pIndex || *pIndex<0 )
			return nullptr;

		for ( int64_t i = *pIndex; i < m_iSize; ++i )
			if ( m_pHash[i].m_bUsed )
			{
				*pIndex = i+1;
				if ( pKey )
					*pKey = m_pHash[i].m_Key;

				return &m_pHash[i].m_Value;
			}

		return nullptr;
	}

	// same as above, but without messing of return value/return param
	std::pair<KEY,VALUE*> Iterate ( int64_t * pIndex ) const
	{
		if ( !pIndex || *pIndex<0 )
			return {0, nullptr};

		for ( int64_t i = *pIndex; i<m_iSize; ++i )
			if ( m_pHash[i].m_bUsed )
			{
				*pIndex = i+1;
				return {m_pHash[i].m_Key, &m_pHash[i].m_Value};
			}

		return {0,nullptr};
	}

	void Swap ( MYTYPE& rhs ) noexcept
	{
		::Swap ( m_iSize, rhs.m_iSize );
		::Swap ( m_iUsed, rhs.m_iUsed );
		::Swap ( m_iMaxUsed, rhs.m_iMaxUsed );
		::Swap ( m_pHash, rhs.m_pHash );
	}

protected:
#pragma pack(push,4)
	struct Entry_t
	{
		KEY		m_Key;
		VALUE	m_Value;
		bool	m_bUsed = false;

		Entry_t();
	};
#pragma pack(pop)


	int64_t		m_iSize {0};					// total hash size
	int64_t		m_iUsed {0};					// how many entries are actually used
	int64_t		m_iMaxUsed {0};					// resize threshold

	Entry_t *	m_pHash {nullptr};	///< hash entries

									/// get max load, ie. max number of actually used entries at given size
	int64_t GetMaxLoad ( int64_t iSize ) const
	{
		return (int64_t)( iSize*LOAD_FACTOR );
	}

	/// we are overloaded, lets grow 2x and rehash
	void Grow()
	{
		int64_t iNewSize = 2*Max(m_iSize,8);
		assert ( iNewSize<=UINT_MAX ); 		// sanity check

		Entry_t * pNew = new Entry_t[iNewSize];

		for ( int64_t i=0; i<m_iSize; i++ )
			if ( m_pHash[i].m_bUsed )
			{
				int64_t j = HASHFUNC::GetHash ( m_pHash[i].m_Key ) & ( iNewSize-1 );
				while ( pNew[j].m_bUsed )
					j = ( j+1 ) & ( iNewSize-1 );

				pNew[j] = m_pHash[i];
			}

		SafeDeleteArray ( m_pHash );
		m_pHash = pNew;
		m_iSize = iNewSize;
		m_iMaxUsed = GetMaxLoad ( m_iSize );
	}

	/// find (and do not touch!) entry by key
	inline Entry_t * FindEntry ( KEY k ) const
	{
		int64_t iIndex = HASHFUNC::GetHash(k) & ( m_iSize-1 );

		while ( m_pHash[iIndex].m_bUsed )
		{
			Entry_t & tEntry = m_pHash[iIndex];
			if ( tEntry.m_Key==k )
				return &tEntry;

			iIndex = ( iIndex+1 ) & ( m_iSize-1 );
		}

		return nullptr;
	}

private:
	static constexpr float LOAD_FACTOR = 0.85f;
};

template<typename V, typename K, typename H> OpenHash_T<V,K,H>::Entry_t::Entry_t() = default;
template<> inline OpenHash_T<int,int64_t>::Entry_t::Entry_t() : m_Key{0}, m_Value{0} {}
template<> inline OpenHash_T<DWORD,int64_t>::Entry_t::Entry_t() : m_Key{0}, m_Value{0} {}
template<> inline OpenHash_T<float,int64_t>::Entry_t::Entry_t() : m_Key{0}, m_Value{0.0f} {}
template<> inline OpenHash_T<int64_t,int64_t>::Entry_t::Entry_t() : m_Key{0}, m_Value{0} {}
template<> inline OpenHash_T<uint64_t,int64_t>::Entry_t::Entry_t() : m_Key{0}, m_Value{0} {}

#endif // _openhash_
