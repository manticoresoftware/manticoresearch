//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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

#include "generics.h"
#include "ints.h"

/// vector traits - provides generic ops over a typed blob (vector).
/// just provide common operators; doesn't manage buffer anyway
template<typename T>
class VecTraits_T
{
public:
	VecTraits_T() = default;

	// this ctr allows to regard any typed blob as VecTraits, and use it's benefits.
	VecTraits_T ( T* pData, int64_t iCount );

	template<typename TT>
	VecTraits_T ( TT* pData, int64_t iCount );

	template<typename TT, typename INT>
	VecTraits_T ( const std::pair<TT*, INT>& dData );

	template<typename TT, typename INT>
	VecTraits_T ( const std::pair<const TT*, INT>& dData );

	FORCE_INLINE VecTraits_T Slice ( int64_t iBegin = 0, int64_t iCount = -1 ) const noexcept;

	/// accessor by forward index
	FORCE_INLINE T & operator[] ( int64_t iIndex ) const noexcept;
	T& At ( int64_t iIndex ) const noexcept;

	/// get first entry ptr
	FORCE_INLINE T * Begin() const noexcept;

	/// pointer to the item after the last
	FORCE_INLINE T * End() const noexcept;

	/// make happy C++11 ranged for loops
	FORCE_INLINE T * begin() const noexcept;
	FORCE_INLINE T * end() const noexcept;

	/// get first entry
	T& First() const noexcept;

	/// get last entry
	T& Last() const noexcept;

	/// return idx of the item pointed by pBuf, or -1
	FORCE_INLINE int Idx ( const T* pBuf ) const noexcept;

	/// make possible to pass VecTraits_T<T*> into funcs which need VecTraits_T<const T*>
	operator VecTraits_T<const typename std::remove_pointer<T>::type*>&() const noexcept;

	template<typename TT>
	operator VecTraits_T<TT>&() const noexcept;

	template<typename TT, typename INT>
	operator std::pair<TT*, INT>() const noexcept;

	/// check if i'm empty
	FORCE_INLINE bool IsEmpty() const noexcept;

	/// query current length, in elements
	FORCE_INLINE int64_t GetLength64() const noexcept;
	FORCE_INLINE int GetLength() const noexcept;
	FORCE_INLINE DWORD GetULength() const noexcept;

	/// get length in bytes
	size_t GetLengthBytes() const noexcept;
	int64_t GetLengthBytes64() const noexcept;

	/// default sort
	void Sort ( int64_t iStart = 0, int64_t iEnd = -1 );

	/// default reverse sort
	void RSort ( int64_t iStart = 0, int64_t iEnd = -1 );

	/// generic sort
	template<typename F>
	void Sort ( F&& COMP, int64_t iStart = 0, int64_t iEnd = -1 );

	/// generic binary search
	/// assumes that the array is sorted in ascending order
	template<typename U, typename PRED>
	T* BinarySearch ( const PRED& tPred, U tRef ) const noexcept;

	/// generic binary search
	/// assumes that the array is sorted in ascending order
	T* BinarySearch ( T tRef ) const noexcept;

	template<typename FILTER>
	FORCE_INLINE int64_t GetFirst ( FILTER&& cond ) const noexcept;

	/// generic 'ARRAY_ALL'
	template<typename FILTER>
	FORCE_INLINE bool all_of ( FILTER&& cond ) const noexcept;

	/// generic linear search - 'ARRAY_ANY' replace
	/// see 'Contains()' below for examlpe of usage.
	template<typename FILTER>
	FORCE_INLINE bool any_of ( FILTER&& cond ) const noexcept;

	template<typename FILTER>
	FORCE_INLINE bool none_of ( FILTER&& cond ) const noexcept;

	template<typename FILTER>
	FORCE_INLINE int64_t count_of ( FILTER&& cond ) const noexcept;

	template<typename INT=int64_t, typename FILTER>
	FORCE_INLINE INT sum_of ( FILTER&& cond ) const noexcept;

	/// Apply an action to every member
	/// Apply ( [] (T& item) {...} );
	template<typename ACTION>
	FORCE_INLINE void Apply ( ACTION&& Verb ) const noexcept;

	template<typename ACTION>
	FORCE_INLINE void for_each ( ACTION&& tAction ) const noexcept;

	/// generic linear search
	bool Contains ( T tRef ) const noexcept;

	/// generic linear search
	template<typename FUNCTOR, typename U>
	bool Contains ( FUNCTOR&& COMP, U tValue );

	/// fill with given value
	void Fill ( const T& rhs );

	/// fill with sequence (appliable only to integers)
	void FillSeq ( T iStart = 0, T iStep = 1 );

protected:
	T* m_pData = nullptr;
	int64_t m_iCount = 0;
};

#include "vectraits_impl.h"
