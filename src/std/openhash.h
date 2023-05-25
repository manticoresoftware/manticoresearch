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

#pragma pack(push,4)
template < typename KEY, typename VALUE >
class OpenHashEntry_T
{
public:
	KEY		m_Key;
	VALUE	m_Value;

	FORCE_INLINE		OpenHashEntry_T();

	template <typename STATE> FORCE_INLINE bool	IsUsed ( const STATE & tState ) const			{ return m_bUsed; }
	template <typename STATE> FORCE_INLINE void SetUsed ( const STATE & tState, bool bUsed )	{ m_bUsed = bUsed; }

private:
	bool	m_bUsed = false;
};
#pragma pack(pop)

#pragma pack(push,4)
template < typename KEY, typename VALUE >
class OpenHashVersionEntry_T
{
public:
	KEY		m_Key;
	VALUE	m_Value;

	template <typename STATE> FORCE_INLINE bool IsUsed ( const STATE & tState ) const			{ return m_uVersion==tState.m_uVersion; }
	template <typename STATE> FORCE_INLINE void SetUsed ( const STATE & tState, bool bUsed )	{ m_uVersion = tState.m_uVersion - !bUsed; }

private:
	uint32_t	m_uVersion = 0;
};
#pragma pack(pop)

#pragma pack(push,4)
template < typename KEY >
class OpenHashSetEntry_T
{
public:
	KEY		m_Key = (KEY)0;

	template <typename STATE> FORCE_INLINE bool IsUsed ( const STATE & tState ) const			{ return m_bUsed; }
	template <typename STATE> FORCE_INLINE void SetUsed ( const STATE & tState, bool bUsed )	{ m_bUsed = bUsed; }

private:
	bool	m_bUsed = false;
};
#pragma pack(pop)

#pragma pack(push,4)
template < typename KEY >
class OpenHashSetVersionEntry_T
{
public:
	KEY			m_Key;

	template <typename STATE> FORCE_INLINE bool	IsUsed ( const STATE & tState ) const			{ return m_uVersion==tState.m_uVersion; }
	template <typename STATE> FORCE_INLINE void	SetUsed ( const STATE & tState, bool bUsed )	{ m_uVersion = tState.m_uVersion - !bUsed; }

private:
	uint32_t	m_uVersion = 0;
};
#pragma pack(pop)


struct HashNoState_t
{
	void Swap ( HashNoState_t & rhs ) {}
};


struct HashStateVersion_t
{
	uint32_t m_uVersion = 1;

	void Swap ( HashStateVersion_t & rhs ) { std::swap ( m_uVersion, rhs.m_uVersion ); }
};

struct HashFunc_Int64_t;

template <typename KEY, typename ENTRY, typename HASHFUNC, typename STATE>
class OpenHashTraits_T : public STATE
{
	using MYTYPE = OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>;

public:
	explicit			OpenHashTraits_T ( int64_t iSize=256 ) { Reset(iSize); }
						OpenHashTraits_T ( const OpenHashTraits_T & rhs );
						~OpenHashTraits_T() { SafeDeleteArray ( m_pHash ); }

	void				Reset ( int64_t iSize );
	void				Clear();
	FORCE_INLINE bool	Delete ( KEY k );

	/// get number of inserted key-value pairs
	FORCE_INLINE int64_t GetLength() const			{ return m_iUsed; }

	/// get total size (not accounting for load factor)
	FORCE_INLINE int64_t GetSize() const				{ return m_iSize; }

	FORCE_INLINE bool	IsFull() const				{ return m_iUsed==m_iMaxUsed; }
	FORCE_INLINE int64_t GetLengthBytes() const		{ return m_iSize*sizeof(ENTRY); }
	FORCE_INLINE int64_t GetUsedLengthBytes() const	{ return m_iUsed*sizeof(ENTRY); }
	static FORCE_INLINE float GetLoadFactor()		{ return LOAD_FACTOR; }

	// move contents to another hash (possibly of larger size)
	void				MoveTo ( MYTYPE & rhs ) const;

protected:
	static constexpr float LOAD_FACTOR = 0.95f;

	int64_t				m_iSize {0};					// total hash size
	int64_t				m_iUsed {0};					// how many entries are actually used
	int64_t				m_iMaxUsed {0};					// resize threshold

	ENTRY *				m_pHash {nullptr};	///< hash entries

	/// find (and do not touch!) entry by key
	FORCE_INLINE ENTRY * FindEntry ( KEY k ) const;

	/// acquire value by key (ie. get existing hashed value, or add a new default value)
	FORCE_INLINE ENTRY & AcquireEntry ( KEY k );

	/// we are overloaded, lets grow 2x and rehash
	void				Grow();

	/// get max load, ie. max number of actually used entries at given size
	FORCE_INLINE int64_t GetMaxLoad ( int64_t iSize ) const	{ return (int64_t)( iSize*LOAD_FACTOR ); }
	FORCE_INLINE void	DeleteEntry ( ENTRY * pEntry );

	void				MoveToNewStorage ( MYTYPE & rhs, ENTRY * pNew, int64_t iNewSize ) const;

	void				Swap ( MYTYPE & rhs ) noexcept;
};

/// simple open-addressing hash
template <typename KEY, typename VALUE, typename HASHFUNC=HashFunc_Int64_t, typename ENTRY=OpenHashEntry_T<KEY,VALUE>, typename STATE=HashNoState_t>
class OpenHashTable_T : public OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>
{
	using BASE = OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>;
	using MYTYPE = OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,STATE>;
	using BASE::BASE;

public:
					OpenHashTable_T ( const OpenHashTable_T & rhs );
					OpenHashTable_T ( OpenHashTable_T && rhs ) noexcept { BASE::Swap(rhs); }

	OpenHashTable_T & operator = ( OpenHashTable_T && rhs ) noexcept;
	OpenHashTable_T & operator = ( const OpenHashTable_T & rhs ) noexcept;

	/// acquire value by key (ie. get existing hashed value, or add a new default value)
	FORCE_INLINE VALUE &	Acquire ( KEY k ) { return BASE::AcquireEntry(k).m_Value; }

	/// find an existing value by key
	FORCE_INLINE VALUE *	Find ( KEY k ) const;

	/// find and delete; assumes that key exists
	FORCE_INLINE VALUE		FindAndDelete ( KEY k );

	/// add or fail (if key already exists)
	FORCE_INLINE bool		Add ( KEY k, const VALUE & v );

	/// find existing value, or add a new value
	FORCE_INLINE VALUE &	FindOrAdd ( KEY k, const VALUE & v );

	/// iterate the hash by entry index, starting from 0
	/// finds the next alive key-value pair starting from the given index
	/// returns that pair and updates the index on success
	/// returns NULL when the hash is over
	VALUE *					Iterate ( int64_t * pIndex, KEY * pKey ) const;

	// same as above, but without messing of return value/return param
	std::pair<KEY,VALUE*>	Iterate ( int64_t & iIndex ) const;
};

template <typename KEY, typename VALUE, typename HASHFUNC=HashFunc_Int64_t, typename ENTRY=OpenHashVersionEntry_T<KEY,VALUE>>
class OpenHashTableFastClear_T : public OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,HashStateVersion_t>
{
	using BASE = OpenHashTable_T<KEY,VALUE,HASHFUNC,ENTRY,HashStateVersion_t>;
	using BASE::BASE;

public:
	FORCE_INLINE void	Clear();
};

template <typename KEY, typename HASHFUNC=HashFunc_Int64_t, typename ENTRY=OpenHashSetEntry_T<KEY>, typename STATE=HashNoState_t>
class OpenHashSet_T : public OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>
{
	using BASE = OpenHashTraits_T<KEY,ENTRY,HASHFUNC,STATE>;
	using BASE::BASE;

public:
	FORCE_INLINE bool	Find ( KEY k ) const { return !!BASE::FindEntry(k); }
	FORCE_INLINE bool	Add ( KEY k );

	KEY *				Iterate ( int64_t & iIndex ) const;
};

template <typename KEY, typename HASHFUNC=HashFunc_Int64_t, typename ENTRY=OpenHashSetVersionEntry_T<KEY>>
class OpenHashSetFastClear_T : public OpenHashSet_T<KEY,HASHFUNC,ENTRY,HashStateVersion_t>
{
	using BASE = OpenHashSet_T<KEY,HASHFUNC,ENTRY,HashStateVersion_t>;
	using BASE::BASE;

public:
	FORCE_INLINE void	Clear();
};


#include "openhash_impl.h"
