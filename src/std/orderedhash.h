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

#pragma once

#include <utility>


#include <cassert>
#include "generics.h"

//////////////////////////////////////////////////////////////////////////

/// simple dynamic hash
/// implementation: fixed-size bucket + chaining
/// keeps the order, so Iterate() return the entries in the order they was inserted
/// WARNING: slow copy
template<typename T, typename KEY, typename HASHFUNC, int LENGTH>
class CSphOrderedHash
{
public:
	using KeyValue_t = std::pair<KEY, T>;
	using KEY_TYPE = KEY;
	using VAL_TYPE = T;

protected:
	struct HashEntry_t: public KeyValue_t // key, data, owned by the hash
	{
		HashEntry_t* m_pNextByHash = nullptr;  ///< next entry in hash list
		HashEntry_t* m_pPrevByOrder = nullptr; ///< prev entry in the insertion order
		HashEntry_t* m_pNextByOrder = nullptr; ///< next entry in the insertion order
	};


protected:
	HashEntry_t* m_dHash[LENGTH];			///< all the hash entries
	HashEntry_t* m_pFirstByOrder = nullptr; ///< first entry in the insertion order
	HashEntry_t* m_pLastByOrder = nullptr;	///< last entry in the insertion order
	int m_iLength = 0;						///< entries count

protected:
	unsigned int HashPos ( const KEY& tKey ) const;


	/// find entry by key
	HashEntry_t* FindByKey ( const KEY& tKey ) const;

	HashEntry_t* AddImpl ( const KEY& tKey );

public:
	/// ctor
	CSphOrderedHash();

	/// dtor
	~CSphOrderedHash();

	/// reset
	void Reset();

	/// add new entry
	/// returns true on success
	/// returns false if this key is already hashed
	bool Add ( T&& tValue, const KEY& tKey );
	bool Add ( const T& tValue, const KEY& tKey );

	/// add new entry
	/// returns ref to just inserted or previously existed value
	T& AddUnique ( const KEY& tKey );

	/// delete an entry
	bool Delete ( const KEY& tKey );

	/// check if key exists
	bool Exists ( const KEY& tKey ) const
	{
		return FindByKey ( tKey ) != nullptr;
	}

	/// get value pointer by key
	T* operator() ( const KEY& tKey ) const
	{
		HashEntry_t* pEntry = FindByKey ( tKey );
		return pEntry ? &pEntry->second : nullptr;
	}

	/// get value reference by key, asserting that the key exists in hash
	T& operator[] ( const KEY& tKey ) const;

	/// copying ctor
	CSphOrderedHash ( const CSphOrderedHash& rhs )
		: CSphOrderedHash()
	{
		for ( const auto& tData : rhs )
			Add ( tData.second, tData.first );
	}

	/// moving ctor
	CSphOrderedHash ( CSphOrderedHash&& rhs ) noexcept
		: CSphOrderedHash()
	{
		Swap ( rhs );
	}

	void Swap ( CSphOrderedHash& rhs ) noexcept;

	/// copying & moving
	CSphOrderedHash& operator= ( CSphOrderedHash rhs )
	{
		Swap ( rhs );
		return *this;
	}

	/// length query
	int GetLength() const
	{
		return m_iLength;
	}

	/// emptiness
	bool IsEmpty() const
	{
		return m_iLength == 0;
	}

public:
	class Iterator_c
	{
		HashEntry_t* m_pIterator = nullptr;

	public:
		explicit Iterator_c ( HashEntry_t* pIterator = nullptr )
			: m_pIterator ( pIterator )
		{}

		KeyValue_t& operator*() { return *m_pIterator; }
		KeyValue_t* operator->() const { return m_pIterator; };

		Iterator_c& operator++()
		{
			m_pIterator = m_pIterator->m_pNextByOrder;
			return *this;
		}

		Iterator_c& operator--()
		{
			m_pIterator = m_pIterator->m_pPrevByOrder;
			return *this;
		}

		bool operator== ( const Iterator_c& rhs ) const
		{
			return m_pIterator == rhs.m_pIterator;
		}

		bool operator!= ( const Iterator_c& rhs ) const
		{
			return !operator== ( rhs );
		}
	};

	// c++11 style iteration
	Iterator_c begin() const
	{
		return Iterator_c { m_pFirstByOrder };
	}

	static Iterator_c end()
	{
		return Iterator_c { nullptr };
	}
};


#include "orderedhash_impl.h"
