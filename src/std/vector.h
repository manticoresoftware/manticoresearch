//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

#include "type_traits.h"
#include "ints.h"
#include "datamove.h"
#include "relimit.h"
#include "storage.h"
#include "vectraits.h"

namespace sph
{

/// generic vector
/// uses storage, mover and relimit backends
/// (don't even ask why it's not std::vector)
template<typename T, class POLICY = DefaultCopy_T<T>, class LIMIT = DefaultRelimit, class STORE = DefaultStorage_T<T>>
class Vector_T: public VecTraits_T<T>, protected STORE, LIMIT
{
	template<typename S = STORE>
	std::enable_if_t< S::is_sized> Deallocate ( typename STORE::TYPE* pData, int64_t );

	template<typename S = STORE>
	std::enable_if_t<!S::is_sized> Deallocate ( typename STORE::TYPE* pData, int64_t );
protected:
	using BASE = VecTraits_T<T>;
	using BASE::m_pData;
	using BASE::m_iCount;
	using STORE::Allocate;
	using STORE::Deallocate;

public:
	using BASE::Begin;
	using BASE::Sort;
	using BASE::GetLength; // these are for IDE helpers to work
	using BASE::GetLength64;
	using BASE::GetLengthBytes;
	using BASE::Slice;
	using LIMIT::Relimit;
	using LIMIT::SANE_SIZE;

	/// ctor
	Vector_T() = default;

	/// ctor with initial size
	explicit Vector_T ( int64_t iCount );

	/// copy ctor
	Vector_T ( const Vector_T<T>& rhs );

	/// move ctr
	Vector_T ( Vector_T<T>&& rhs ) noexcept;

	/// dtor
	~Vector_T();

	/// add entry
	T& Add();

	/// add entry
	template<typename S = STORE>
	typename std::enable_if<S::is_constructed>::type Add ( T tValue );

	template<typename S = STORE>
	typename std::enable_if<!S::is_constructed>::type Add ( T tValue );

	template<typename S = STORE, class... Args>
	typename std::enable_if<!S::is_constructed>::type Emplace_back ( Args&&... args );

	/// add N more entries, and return a pointer to that buffer
	T* AddN ( int iCount );

	/// add unique entry (ie. do not add if equal to last one)
	void AddUnique ( const T& tValue );

	/// remove several elements by index
	void Remove ( int iIndex, int iCount = 1 );

	/// remove element by index, swapping it with the tail
	void RemoveFast ( int iIndex );

	/// remove element by value (warning, linear O(n) search)
	bool RemoveValue ( T tValue );

	/// remove element by value, asuming vec is sorted/uniq
	bool RemoveValueFromSorted ( T tValue );

	/// pop last value by ref (for constructed storage)
	template<typename S = STORE>
	typename std::enable_if<S::is_constructed, T&>::type Pop();

	/// pop last value
	template<typename S = STORE>
	typename std::enable_if<!S::is_constructed, T>::type Pop();

	/// grow enough to hold iNewLimit-iDiscard entries, if needed.
	/// returns updated index of elem iDiscard.
	int DiscardAndReserve ( int64_t iDiscard, int64_t iNewLimit );

	/// grow enough to hold that much entries, if needed, but do *not* change the length
	void Reserve ( int64_t iNewLimit );

	/// for non-copyable types - work like Reset() + Reserve()
	/// destroys previous dataset, allocate new one and set size to 0.
	template<typename S = STORE>
	typename std::enable_if<!S::is_constructed>::type Reserve_static ( int64_t iNewLimit );

	/// ensure we have space for iGap more items (reserve more if necessary)
	inline void ReserveGap ( int iGap );

	/// resize
	template<typename S = STORE>
	typename std::enable_if<S::is_constructed>::type Resize ( int64_t iNewLength );

	/// for non-constructed imply destroy when shrinking, of construct when widening
	template<typename S = STORE>
	typename std::enable_if<!S::is_constructed>::type Resize ( int64_t iNewLength );

	// doesn't need default c-tr
	void Shrink ( int64_t iNewLength );

	/// reset
	void Reset();

	/// Set whole vec to 0. For trivially copyable memset will be used
	template<typename S = STORE>
	typename std::enable_if<S::is_constructed>::type ZeroVec();

	/// set the tail [m_iCount..m_iLimit) to zero
	void ZeroTail();

	/// query current reserved size, in elements
	inline int GetLimit() const;

	/// query currently allocated RAM, in bytes
	/// (could be > GetLengthBytes() since uses limit, not size)
	inline int64_t AllocatedBytes() const;

	/// filter unique
	void Uniq();

	void MergeSorted ( const Vector_T<T>& dA, const Vector_T<T>& dB );

	/// copy + move
	// if provided lvalue, it will be copied into rhs via copy ctr, then swapped to *this
	// if provided rvalue, it will just pass to SwapData immediately.
	Vector_T& operator= ( Vector_T<T> rhs ) noexcept;

	bool operator== ( const Vector_T<T>& rhs ) noexcept;

	/// memmove N elements from raw pointer to the end
	/// works ONLY if T is POD type (i.e. may be simple memmoved)
	/// otherwize compile error will appear (if so, use typed version below).
	void Append ( const void* pData, int iN );

	/// append another vec to the end
	/// will use memmove (POD case), or one-by-one copying.
	template<typename S = STORE>
	typename std::enable_if<S::is_constructed>::type Append ( const VecTraits_T<T>& rhs );

	/// append another vec to the end for non-constructed
	/// will construct in-place with copy c-tr
	template<typename S = STORE>
	typename std::enable_if<!S::is_constructed>::type Append ( const VecTraits_T<T>& rhs );

	/// swap
	template<typename L = LIMIT, typename S = STORE>
	typename std::enable_if<!S::is_owned>::type SwapData ( Vector_T<T, POLICY, L, STORE>& rhs ) noexcept;

	/// leak
	template<typename S = STORE>
	typename std::enable_if<!S::is_owned, T*>::type LeakData();

	/// adopt external buffer
	/// note that caller must himself then nullify origin pData to avoid double-deletion
	template<typename S = STORE>
	typename std::enable_if<!S::is_owned>::type AdoptData ( T* pData, int64_t iLen, int64_t iLimit );

	/// insert into a middle (will fail to compile for swap vector)
	template<typename TT=T>
	void Insert ( int64_t iIndex, TT&& tValue );

protected:
	int64_t m_iLimit = 0; ///< entries allocated

	template<typename S = STORE>
	typename std::enable_if<S::is_constructed>::type destroy_at ( int64_t, int64_t ) {}

	template<typename S = STORE>
	typename std::enable_if<!S::is_constructed>::type destroy_at ( int64_t iIndex, int64_t iCount );

	template<typename S = STORE>
	typename std::enable_if<S::is_constructed>::type construct_at ( int64_t, int64_t ) {}

	template<typename S = STORE>
	typename std::enable_if<!S::is_constructed>::type construct_at ( int64_t iIndex, int64_t iCount );
};

} // namespace sph

//////////////////////////////////////////////////////////////////////////

/// old well-known vector
template<typename T, typename R = sph::DefaultRelimit>
using CSphVector = sph::Vector_T<T, sph::DefaultCopy_T<T>, R>;

template<typename T, typename R = sph::DefaultRelimit, int STATICSIZE = 4096 / sizeof ( T )>
using LazyVector_T = sph::Vector_T<T, sph::DefaultCopy_T<T>, R, sph::LazyStorage_T<T, STATICSIZE>>;

/// swap-vector
template<typename T>
using CSphSwapVector = sph::Vector_T<T, sph::SwapCopy_T<T>>;

/// tight-vector
template<typename T>
using CSphTightVector = CSphVector<T, sph::TightRelimit>;

/// raw vector for non-default-constructibles
template<typename T>
using RawVector_T = sph::Vector_T<T, sph::SwapCopy_T<T>, sph::DefaultRelimit, sph::RawStorage_T<T>>;

/// commonly used
using IntVec_t = CSphVector<int>;

/// vector of byte vectors
using BlobVec_t = CSphVector<CSphVector<BYTE>>;

#include "vector_impl.h"