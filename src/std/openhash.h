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

#include "ints.h"
#include <utility>

struct HashFunc_Int64_t;

/// simple open-addressing hash
template < typename VALUE, typename KEY, typename HASHFUNC=HashFunc_Int64_t >
class OpenHash_T
{
public:
	using MYTYPE = OpenHash_T<VALUE,KEY,HASHFUNC>;

	/// initialize hash of a given initial size
	explicit OpenHash_T ( int64_t iSize=256 );
	OpenHash_T ( const OpenHash_T& rhs );
	OpenHash_T ( OpenHash_T&& rhs ) noexcept;
	OpenHash_T& operator= ( OpenHash_T rhs ) noexcept;
	~OpenHash_T();

	/// reset to a given size
	void Reset ( int64_t iSize );
	void Clear();

	/// acquire value by key (ie. get existing hashed value, or add a new default value)
	VALUE & Acquire ( KEY k );

	/// find an existing value by key
	VALUE * Find ( KEY k ) const;

	/// add or fail (if key already exists)
	bool Add ( KEY k, const VALUE & v );

	/// find existing value, or add a new value
	VALUE & FindOrAdd ( KEY k, const VALUE & v );

	/// delete by key
	bool Delete ( KEY k );

	/// get number of inserted key-value pairs
	int64_t GetLength() const;
	int64_t GetLengthBytes() const;
	int64_t GetUsedLengthBytes() const;
	 
	/// iterate the hash by entry index, starting from 0
	/// finds the next alive key-value pair starting from the given index
	/// returns that pair and updates the index on success
	/// returns NULL when the hash is over
	VALUE * Iterate ( int64_t * pIndex, KEY * pKey ) const;

	// same as above, but without messing of return value/return param
	std::pair<KEY,VALUE*> Iterate ( int64_t * pIndex ) const;

	void Swap ( MYTYPE& rhs ) noexcept;
protected:

	struct Entry_t;

	int64_t		m_iSize {0};					// total hash size
	int64_t		m_iUsed {0};					// how many entries are actually used
	int64_t		m_iMaxUsed {0};					// resize threshold

	Entry_t *	m_pHash {nullptr};	///< hash entries

									/// get max load, ie. max number of actually used entries at given size
	int64_t GetMaxLoad ( int64_t iSize ) const;

	/// we are overloaded, lets grow 2x and rehash
	void Grow();

	/// find (and do not touch!) entry by key
	Entry_t * FindEntry ( KEY k ) const;

private:
	static constexpr float LOAD_FACTOR = 0.85f;
};

#include "openhash_impl.h"
