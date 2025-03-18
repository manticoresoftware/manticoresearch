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
#include "checks.h"
#include "uniq.h"

namespace sph
{

template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
inline std::enable_if_t<S::is_sized> Vector_T<T, POLICY, LIMIT, STORE>::Vector_T::Deallocate ( typename STORE::TYPE* pData, int64_t iLimit )
{
	STORE::Deallocate ( pData, iLimit );
}

template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
inline std::enable_if_t<!S::is_sized> Vector_T<T, POLICY, LIMIT, STORE>::Vector_T::Deallocate ( typename STORE::TYPE* pData, int64_t )
{
	STORE::Deallocate ( pData );
}

/* variant below needs C++17
template<class STORE>
inline void Deallocate ( typename STORE::TYPE* pData, int64_t iLimit )
{
	if constexpr ( STORE::is_sized )
		STORE::Deallocate ( pData, iLimit );
	else
		STORE::Deallocate ( pData );
}
 */

/// ctor with initial size
template<typename T, class POLICY, class LIMIT, class STORE>
Vector_T<T,POLICY,LIMIT,STORE>::Vector_T ( int64_t iCount )
{
	Resize ( iCount );
}

/// copy ctor
template<typename T, class POLICY, class LIMIT, class STORE>
Vector_T<T, POLICY, LIMIT, STORE>::Vector_T ( const Vector_T<T>& rhs )
{
	m_iCount = rhs.m_iCount;
	m_iLimit = rhs.m_iLimit;
	if ( m_iLimit )
		m_pData = STORE::Allocate ( m_iLimit );
	__analysis_assume ( m_iCount <= m_iLimit );
	POLICY::Copy ( m_pData, rhs.m_pData, m_iCount );
}

/// move ctr
template<typename T, class POLICY, class LIMIT, class STORE>
Vector_T<T, POLICY, LIMIT, STORE>::Vector_T ( Vector_T<T>&& rhs ) noexcept
		: Vector_T()
	{
		SwapData ( rhs );
	}

/// dtor
template<typename T, class POLICY, class LIMIT, class STORE>
Vector_T<T, POLICY, LIMIT, STORE>::~Vector_T()
{
	destroy_at ( 0, m_iCount );
	Deallocate ( m_pData, m_iLimit );
}

/// add entry
template<typename T, class POLICY, class LIMIT, class STORE>
T& Vector_T<T, POLICY, LIMIT, STORE>::Add()
{
	if ( m_iCount >= m_iLimit )
		Reserve ( 1 + m_iCount );
	construct_at ( m_iCount, 1 );
	return m_pData[m_iCount++];
}

/// add entry
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if_t<S::is_constructed> Vector_T<T, POLICY, LIMIT, STORE>::Add ( T tValue )
{
	assert ( ( &tValue < m_pData || &tValue >= ( m_pData + m_iCount ) ) && "inserting own value (like last()) by ref!" );
	if ( m_iCount >= m_iLimit )
		Reserve ( 1 + m_iCount );
	m_pData[m_iCount++] = std::move ( tValue );
}

template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if_t<!S::is_constructed> Vector_T<T, POLICY, LIMIT, STORE>::Add ( T tValue )
{
	assert ( ( &tValue < m_pData || &tValue >= ( m_pData + m_iCount ) ) && "inserting own value (like last()) by ref!" );
	if ( m_iCount >= m_iLimit )
		Reserve ( 1 + m_iCount );
	new ( m_pData + m_iCount++ ) T ( std::move ( tValue ) );
}

template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S, class... Args>
typename std::enable_if_t<!S::is_constructed> Vector_T<T, POLICY, LIMIT, STORE>::Emplace_back ( Args&&... args )
{
	assert ( m_iCount <= m_iLimit );
	new ( m_pData + m_iCount++ ) T ( std::forward<Args> ( args )... );
}

/// add N more entries, and return a pointer to that buffer
template<typename T, class POLICY, class LIMIT, class STORE>
T* Vector_T<T, POLICY, LIMIT, STORE>::AddN ( int iCount )
{
	if ( m_iCount + iCount > m_iLimit )
		Reserve ( m_iCount + iCount );
	construct_at ( m_iCount, iCount );
	m_iCount += iCount;
	return m_pData + m_iCount - iCount;
}

/// add unique entry (ie. do not add if equal to last one)
template<typename T, class POLICY, class LIMIT, class STORE>
void Vector_T<T, POLICY, LIMIT, STORE>::AddUnique ( const T& tValue )
{
	assert ( ( &tValue < m_pData || &tValue >= ( m_pData + m_iCount ) ) && "inserting own value (like last()) by ref!" );
	if ( m_iCount >= m_iLimit )
		Reserve ( 1 + m_iCount );

	if ( m_iCount == 0 || m_pData[m_iCount - 1] != tValue )
		m_pData[m_iCount++] = tValue;
}

/// remove several elements by index
template<typename T, class POLICY, class LIMIT, class STORE>
void Vector_T<T, POLICY, LIMIT, STORE>::Remove ( int iIndex, int iCount )
{
	if ( iCount <= 0 )
		return;

	assert ( iIndex >= 0 && iIndex < m_iCount );
	assert ( iIndex + iCount <= m_iCount );

	m_iCount -= iCount;
	if ( m_iCount > iIndex )
		POLICY::Move ( m_pData + iIndex, m_pData + iIndex + iCount, m_iCount - iIndex );
	destroy_at ( m_iCount, iCount );
}

/// remove element by index, swapping it with the tail
template<typename T, class POLICY, class LIMIT, class STORE>
void Vector_T<T, POLICY, LIMIT, STORE>::RemoveFast ( int iIndex )
{
	assert ( iIndex >= 0 && iIndex < m_iCount );
	if ( iIndex != --m_iCount )
		Swap ( m_pData[iIndex], m_pData[m_iCount] ); // fixme! What about POLICY::CopyOrSwap here?
	destroy_at ( m_iCount, 1 );
}

/// remove element by value (warning, linear O(n) search)
template<typename T, class POLICY, class LIMIT, class STORE>
bool Vector_T<T, POLICY, LIMIT, STORE>::RemoveValue ( T tValue )
{
	for ( int i = 0; i < m_iCount; ++i )
		if ( m_pData[i] == tValue )
		{
			Remove ( i );
			return true;
		}
	return false;
}

/// remove element by value, asuming vec is sorted/uniq
template<typename T, class POLICY, class LIMIT, class STORE>
bool Vector_T<T, POLICY, LIMIT, STORE>::RemoveValueFromSorted ( T tValue )
{
	T* pValue = VecTraits_T<T>::BinarySearch ( tValue );
	if ( !pValue )
		return false;

	Remove ( int ( pValue - Begin() ) );
	return true;
}

/// pop last value by ref (for constructed storage)
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<S::is_constructed, T&>::type Vector_T<T, POLICY, LIMIT, STORE>::Pop()
{
	assert ( m_iCount > 0 );
	return m_pData[--m_iCount];
}

/// pop last value
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<!S::is_constructed, T>::type Vector_T<T, POLICY, LIMIT, STORE>::Pop()
{
	assert ( m_iCount > 0 );
	auto res = m_pData[--m_iCount];
	destroy_at ( m_iCount );
	return res;
}

/// grow enough to hold iNewLimit-iDiscard entries, if needed.
/// returns updated index of elem iDiscard.
template<typename T, class POLICY, class LIMIT, class STORE>
int Vector_T<T, POLICY, LIMIT, STORE>::DiscardAndReserve ( int64_t iDiscard, int64_t iNewLimit )
{
	assert ( iNewLimit >= 0 );
	assert ( iDiscard >= 0 );

	// check that we really need to be called
	if ( iNewLimit <= m_iLimit )
		return (int)iDiscard;

	if ( iDiscard > 0 )
	{
		// align limit and size
		iNewLimit -= iDiscard;
		m_iCount = ( iDiscard < m_iCount ) ? ( m_iCount - iDiscard ) : 0;

		// check, if we still need to be called with aligned limit
		if ( iNewLimit <= m_iLimit )
		{
			if ( m_iCount ) // has something to move back
				POLICY::Move ( m_pData, m_pData + iDiscard, m_iCount );
			return 0;
		}
	}

	// calc new limit
	m_iLimit = LIMIT::Relimit ( m_iLimit, std::exchange ( iNewLimit, m_iLimit ) );

	// realloc
	T* pNew = nullptr;
	if ( m_iLimit )
	{
		pNew = STORE::Allocate ( m_iLimit );
		__analysis_assume ( m_iCount - iDiscard <= m_iLimit );
		if ( m_iCount ) // has something to copy from an old storage
			POLICY::Move ( pNew, m_pData + iDiscard, m_iCount );

		if ( pNew == m_pData )
			return 0;
	}
	Swap ( pNew, m_pData );
	Deallocate ( pNew, iNewLimit );
	return 0;
}

/// grow enough to hold that much entries, if needed, but do *not* change the length
template<typename T, class POLICY, class LIMIT, class STORE>
void Vector_T<T, POLICY, LIMIT, STORE>::Reserve ( int64_t iNewLimit )
{
	DiscardAndReserve ( 0, iNewLimit );
}

/// for non-copyable types - work like Reset() + Reserve()
/// destroys previous dataset, allocate new one and set size to 0.
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<!S::is_constructed>::type Vector_T<T, POLICY, LIMIT, STORE>::Reserve_static ( int64_t iNewLimit )
{
	// check that we really need to be called
	destroy_at ( 0, m_iCount );
	m_iCount = 0;

	if ( iNewLimit == m_iLimit )
		return;

	std::swap ( m_iLimit, iNewLimit );

	// realloc
	T* pNew = nullptr;
	pNew = STORE::Allocate ( m_iLimit );
	if ( pNew == m_pData )
		return;

	__analysis_assume ( m_iCount <= m_iLimit );
	std::swap ( pNew, m_pData );
	Deallocate ( pNew, iNewLimit );
}

/// ensure we have space for iGap more items (reserve more if necessary)
template<typename T, class POLICY, class LIMIT, class STORE>
inline void Vector_T<T, POLICY, LIMIT, STORE>::ReserveGap ( int iGap )
{
	Reserve ( m_iCount + iGap );
}

/// resize
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<S::is_constructed>::type Vector_T<T, POLICY, LIMIT, STORE>::Resize ( int64_t iNewLength )
{
	assert ( iNewLength >= 0 );
	if ( iNewLength > m_iCount )
		Reserve ( iNewLength );
	m_iCount = iNewLength;
}

/// for non-constructed imply destroy when shrinking, of construct when widening
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<!S::is_constructed>::type Vector_T<T, POLICY, LIMIT, STORE>::Resize ( int64_t iNewLength )
{
	assert ( iNewLength >= 0 );
	if ( iNewLength < m_iCount )
		destroy_at ( iNewLength, m_iCount - iNewLength );
	else
	{
		Reserve ( iNewLength );
		construct_at ( m_iCount, iNewLength - m_iCount );
	}
	m_iCount = iNewLength;
}

// doesn't need default c-tr
template<typename T, class POLICY, class LIMIT, class STORE>
void Vector_T<T, POLICY, LIMIT, STORE>::Shrink ( int64_t iNewLength )
{
	assert ( iNewLength <= m_iCount );
	destroy_at ( iNewLength, m_iCount - iNewLength );
	m_iCount = iNewLength;
}

/// reset
template<typename T, class POLICY, class LIMIT, class STORE>
void Vector_T<T, POLICY, LIMIT, STORE>::Reset()
{
	Shrink ( 0 );
	Deallocate ( m_pData, m_iLimit );
	m_pData = nullptr;
	m_iLimit = 0;
}

/// Set whole vec to 0. For trivially copyable memset will be used
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<S::is_constructed>::type Vector_T<T, POLICY, LIMIT, STORE>::ZeroVec()
{
	POLICY::Zero ( m_pData, m_iLimit );
}

/// set the tail [m_iCount..m_iLimit) to zero
template<typename T, class POLICY, class LIMIT, class STORE>
void Vector_T<T, POLICY, LIMIT, STORE>::ZeroTail()
{
	if ( !m_pData )
		return;

	POLICY::Zero ( &m_pData[m_iCount], m_iLimit - m_iCount );
}

/// query current reserved size, in elements
template<typename T, class POLICY, class LIMIT, class STORE>
int Vector_T<T, POLICY, LIMIT, STORE>::GetLimit() const
{
	return (int)m_iLimit;
}

/// query currently allocated RAM, in bytes
/// (could be > GetLengthBytes() since uses limit, not size)
template<typename T, class POLICY, class LIMIT, class STORE>
int64_t Vector_T<T, POLICY, LIMIT, STORE>::AllocatedBytes() const
{
	return (int)m_iLimit * sizeof ( T );
}

/// filter unique
template<typename T, class POLICY, class LIMIT, class STORE>
void Vector_T<T, POLICY, LIMIT, STORE>::Uniq ( bool bSort )
{
	if ( !m_iCount )
		return;

	if ( bSort )
		Sort();

	int64_t iLeft = sphUniq ( m_pData, m_iCount );
	Shrink ( iLeft );
}

template<typename T, class POLICY, class LIMIT, class STORE>
void Vector_T<T, POLICY, LIMIT, STORE>::MergeSorted ( const Vector_T<T>& dA, const Vector_T<T>& dB )
{
	// fixme: add comparators
	int iLenA = dA.GetLength();
	int iLenB = dB.GetLength();
	Resize ( iLenA + iLenB );

	T* pA = dA.Begin();
	T* pB = dB.Begin();
	T* pRes = Begin();
	const T* pMaxA = pA + iLenA;
	const T* pMaxB = pB + iLenB;

	while ( pA < pMaxA && pB < pMaxB )
		*pRes++ = *pA < *pB ? *pA++ : *pB++;

	while ( pA < pMaxA )
		*pRes++ = *pA++;

	while ( pB < pMaxB )
		*pRes++ = *pB++;
}

/// copy + move
// if provided lvalue, it will be copied into rhs via copy ctr, then swapped to *this
// if provided rvalue, it will just pass to SwapData immediately.
template<typename T, class POLICY, class LIMIT, class STORE>
Vector_T<T, POLICY, LIMIT, STORE>& Vector_T<T, POLICY, LIMIT, STORE>::operator= ( Vector_T<T> rhs ) noexcept
{
	SwapData ( rhs );
	return *this;
}

template<typename T, class POLICY, class LIMIT, class STORE>
bool Vector_T<T, POLICY, LIMIT, STORE>::operator== ( const Vector_T<T>& rhs ) noexcept
{
	if ( m_iCount != rhs.m_iCount )
		return false;

	for ( int i = 0; i < m_iCount; i++ )
		if ( m_pData[i] != rhs.m_pData[i] )
			return false;

	return true;
}

/// memmove N elements from raw pointer to the end
/// works ONLY if T is POD type (i.e. may be simple memmoved)
/// otherwize compile error will appear (if so, use typed version below).
template<typename T, class POLICY, class LIMIT, class STORE>
void Vector_T<T, POLICY, LIMIT, STORE>::Append ( const void* pData, int iN )
{
	if ( iN <= 0 )
		return;

	auto* pDst = AddN ( iN );
	POLICY::CopyVoid ( pDst, pData, iN );
}

/// append another vec to the end
/// will use memmove (POD case), or one-by-one copying.
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<S::is_constructed>::type Vector_T<T, POLICY, LIMIT, STORE>::Append ( const VecTraits_T<T>& rhs )
{
	if ( rhs.IsEmpty() )
		return;

	auto* pDst = AddN ( rhs.GetLength() );
	POLICY::Copy ( pDst, rhs.begin(), rhs.GetLength() );
}

/// append another vec to the end for non-constructed
/// will construct in-place with copy c-tr
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<!S::is_constructed>::type Vector_T<T, POLICY, LIMIT, STORE>::Append ( const VecTraits_T<T>& rhs )
{
	if ( rhs.IsEmpty() )
		return;

	auto iRhsLen = rhs.GetLength64();
	if ( m_iCount + iRhsLen > m_iLimit )
		Reserve ( m_iCount + iRhsLen );
	for ( int i = 0; i < iRhsLen; ++i )
		new ( m_pData + m_iCount + i ) T ( rhs[i] );

	m_iCount += iRhsLen;
}

/// swap
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename L, typename S>
typename std::enable_if<!S::is_owned>::type Vector_T<T, POLICY, LIMIT, STORE>::SwapData ( Vector_T<T, POLICY, L, STORE>& rhs ) noexcept
{
	Swap ( m_iCount, rhs.m_iCount );
	Swap ( m_iLimit, rhs.m_iLimit );
	Swap ( m_pData, rhs.m_pData );
}

/// leak
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<!S::is_owned, T*>::type Vector_T<T, POLICY, LIMIT, STORE>::LeakData()
{
	T* pData = m_pData;
	m_pData = nullptr;
	Reset();
	return pData;
}

/// adopt external buffer
/// note that caller must himself then nullify origin pData to avoid double-deletion
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<!S::is_owned>::type Vector_T<T, POLICY, LIMIT, STORE>::AdoptData ( T* pData, int64_t iLen, int64_t iLimit )
{
	assert ( iLen >= 0 );
	assert ( iLimit >= 0 );
	assert ( pData || iLimit == 0 );
	assert ( iLen <= iLimit );
	Reset();
	m_pData = pData;
	m_iLimit = iLimit;
	m_iCount = iLen;
}

/// insert into a middle by policy-defined copier
template<typename T, class POLICY, class LIMIT, class STORE>
template<typename TT>
void Vector_T<T, POLICY, LIMIT, STORE>::Insert ( int64_t iIndex, TT&& tValue )
{
	assert ( iIndex >= 0 && iIndex <= m_iCount );

	if ( this->m_iCount >= this->m_iLimit )
		Reserve ( this->m_iCount + 1 );

	for ( auto i = this->m_iCount - 1; i >= iIndex; --i )
		POLICY::CopyOrSwap ( this->m_pData[i + 1], this->m_pData[i] );

	POLICY::CopyOrSwap ( this->m_pData[iIndex], std::forward<TT> ( tValue ) );
	++this->m_iCount;
}


template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<!S::is_constructed>::type Vector_T<T, POLICY, LIMIT, STORE>::destroy_at ( int64_t iIndex, int64_t iCount )
{
	for ( int64_t i = 0; i < iCount; ++i )
		m_pData[iIndex + i].~T();
}

template<typename T, class POLICY, class LIMIT, class STORE>
template<typename S>
typename std::enable_if<!S::is_constructed>::type Vector_T<T, POLICY, LIMIT, STORE>::construct_at ( int64_t iIndex, int64_t iCount )
{
	assert ( m_pData );
	for ( int64_t i = 0; i < iCount; ++i )
		new ( m_pData + iIndex + i ) T();
}

} // namespace sph