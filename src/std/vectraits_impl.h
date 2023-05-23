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

#include <utility>
#include <cassert>
#include <type_traits>

#include "thread_annotations.h"
#include "checks.h"
#include "comp.h"
#include "sort.h"
#include "binarysearch.h"
#include "iterations.h"

// this ctr allows to regard any typed blob as VecTraits, and use its benefits.
template<typename T>
VecTraits_T<T>::VecTraits_T ( T* pData, int64_t iCount )
	: m_pData ( pData )
	, m_iCount ( iCount )
{}

template<typename T>
template<typename TT>
VecTraits_T<T>::VecTraits_T ( TT* pData, int64_t iCount )
	: m_pData ( pData )
	, m_iCount ( iCount * sizeof ( TT ) / sizeof ( T ) )
{}

template<typename T>
template<typename TT, typename INT>
VecTraits_T<T>::VecTraits_T ( const std::pair<TT*, INT>& dData )
	: m_pData ( (T*)dData.first )
	, m_iCount ( dData.second * sizeof ( TT ) / sizeof ( T ) )
{}

template<typename T>
template<typename TT, typename INT>
VecTraits_T<T>::VecTraits_T ( const std::pair<const TT*, INT>& dData )
	: m_pData ( (T*)const_cast<TT*> ( dData.first ) )
	, m_iCount ( dData.second * sizeof ( TT ) / sizeof ( T ) )
{}

template<typename T>
VecTraits_T<T> VecTraits_T<T>::Slice ( int64_t iBegin, int64_t iCount ) const
{
	// calculate starting bound
	if ( iBegin < 0 )
		iBegin = 0;
	else if ( iBegin > m_iCount )
		iBegin = m_iCount;

	iCount = ( iCount < 0 ) ? ( m_iCount - iBegin ) : Min ( iCount, m_iCount - iBegin );
	return VecTraits_T ( m_pData + iBegin, iCount );
}

/// accessor by forward index
template<typename T>
T& VecTraits_T<T>::operator[] ( int64_t iIndex ) const
{
	assert ( iIndex >= 0 && iIndex < m_iCount );
	return m_pData[iIndex];
}

template<typename T>
T& VecTraits_T<T>::At ( int64_t iIndex ) const
{
	return this->operator[] ( iIndex );
}

/// get first entry ptr
template<typename T>
T* VecTraits_T<T>::Begin() const
{
	return m_iCount ? m_pData : nullptr;
}

/// pointer to the item after the last
template<typename T>
T* VecTraits_T<T>::End() const
{
	return m_pData + m_iCount;
}

/// make happy C++11 ranged for loops
template<typename T>
T* VecTraits_T<T>::begin() const
{
	return Begin();
}

template<typename T>
T* VecTraits_T<T>::end() const
{
	return m_iCount ? m_pData + m_iCount : nullptr;
}

/// get first entry
template<typename T>
T& VecTraits_T<T>::First() const
{
	return ( *this )[0];
}

/// get last entry
template<typename T>
T& VecTraits_T<T>::Last() const
{
	return ( *this )[m_iCount - 1];
}

/// return idx of the item pointed by pBuf, or -1
template<typename T>
int VecTraits_T<T>::Idx ( const T* pBuf ) const
{
	if ( !pBuf )
		return -1;

	if ( pBuf < m_pData || pBuf >= m_pData + m_iCount )
		return -1;

	return int ( pBuf - m_pData );
}

/// make possible to pass VecTraits_T<T*> into funcs which need VecTraits_T<const T*>
/// fixme! M.b. add check and fire error if T is not a pointer?
template<typename T>
VecTraits_T<T>::operator VecTraits_T<const typename std::remove_pointer<T>::type*>&() const
{
	return *(VecTraits_T<const typename std::remove_pointer<T>::type*>*)( const_cast<VecTraits_T<T>*> ( this ) );
}

template<typename T>
template<typename TT>
VecTraits_T<T>::operator VecTraits_T<TT>&() const
{
	STATIC_ASSERT ( sizeof ( T ) == sizeof ( TT ), SIZE_OF_DERIVED_NOT_SAME_AS_ORIGIN );
	return *(VecTraits_T<TT>*)( const_cast<VecTraits_T<T>*> ( this ) );
}

template<typename T>
template<typename TT, typename INT>
VecTraits_T<T>::operator std::pair<TT*, INT>() const
{
	return { (TT*)m_pData, INT ( m_iCount * sizeof ( T ) / sizeof ( TT ) ) };
}

/// check if i'm empty
template<typename T>
bool VecTraits_T<T>::IsEmpty() const
{
	return ( m_pData == nullptr || m_iCount == 0 );
}

/// query current length, in elements
template<typename T>
int64_t VecTraits_T<T>::GetLength64() const
{
	return m_iCount;
}

template<typename T>
int VecTraits_T<T>::GetLength() const
{
	return (int)m_iCount;
}

/// get length in bytes
template<typename T>
size_t VecTraits_T<T>::GetLengthBytes() const
{
	return sizeof ( T ) * (size_t)m_iCount;
}

/// get length in bytes
template<typename T>
int64_t VecTraits_T<T>::GetLengthBytes64() const
{
	return m_iCount * sizeof ( T );
}

/// default sort
template<typename T>
void VecTraits_T<T>::Sort ( int iStart, int iEnd )
{
	Sort ( SphLess_T<T>(), iStart, iEnd );
}

/// default reverse sort
template<typename T>
void VecTraits_T<T>::RSort ( int iStart, int iEnd )
{
	Sort ( SphGreater_T<T>(), iStart, iEnd );
}

/// generic sort
template<typename T>
template<typename F>
void VecTraits_T<T>::Sort ( F&& COMP, int iStart, int iEnd ) NO_THREAD_SAFETY_ANALYSIS
{
	if ( m_iCount < 2 )
		return;
	if ( iStart < 0 )
		iStart += m_iCount;
	if ( iEnd < 0 )
		iEnd += m_iCount;
	assert ( iStart <= iEnd );

	sphSort ( m_pData + iStart, iEnd - iStart + 1, std::forward<F> ( COMP ) );
}

/// generic binary search
/// assumes that the array is sorted in ascending order
template<typename T>
template<typename U, typename PRED>
T* VecTraits_T<T>::BinarySearch ( const PRED& tPred, U tRef ) const NO_THREAD_SAFETY_ANALYSIS
{
	return sphBinarySearch ( m_pData, m_pData + m_iCount - 1, tPred, tRef );
}

/// generic binary search
/// assumes that the array is sorted in ascending order
template<typename T>
T* VecTraits_T<T>::BinarySearch ( T tRef ) const
{
	return sphBinarySearch ( m_pData, m_pData + m_iCount - 1, tRef );
}

template<typename T>
template<typename FILTER>
int VecTraits_T<T>::GetFirst ( FILTER&& cond ) const NO_THREAD_SAFETY_ANALYSIS
{
	for ( int i = 0; i < m_iCount; ++i )
		if ( cond ( m_pData[i] ) )
			return i;
	return -1;
}

/// generic 'ARRAY_ALL'
template<typename T>
template<typename FILTER>
bool VecTraits_T<T>::all_of ( FILTER&& cond ) const
{
	return ::all_of ( *this, std::forward<FILTER> ( cond ) );
}

/// generic linear search - 'ARRAY_ANY' replace
/// see 'Contains()' below for examlpe of usage.
template<typename T>
template<typename FILTER>
bool VecTraits_T<T>::any_of ( FILTER&& cond ) const
{
	return ::any_of ( *this, std::forward<FILTER> ( cond ) );
}


template<typename T>
template<typename FILTER>
bool VecTraits_T<T>::none_of ( FILTER&& cond ) const
{
	return !any_of ( cond );
}


template<typename T>
template<typename FILTER>
int64_t VecTraits_T<T>::count_of ( FILTER&& cond ) const
{
	return ::count_of ( *this, std::forward<FILTER> ( cond ) );
}

/// Apply an action to every member
/// Apply ( [] (T& item) {...} );
template<typename T>
template<typename ACTION>
void VecTraits_T<T>::Apply ( ACTION&& Verb ) const
{
	::for_each ( *this, std::forward<ACTION> ( Verb ) );
}

template<typename T>
template<typename ACTION>
void VecTraits_T<T>::for_each ( ACTION&& tAction ) const
{
	::for_each ( *this, std::forward<ACTION> ( tAction ) );
}

/// generic linear search
template<typename T>
bool VecTraits_T<T>::Contains ( T tRef ) const NO_THREAD_SAFETY_ANALYSIS
{
	return any_of ( [&] ( const T& v ) { return tRef == v; } );
}

/// generic linear search
template<typename T>
template<typename FUNCTOR, typename U>
bool VecTraits_T<T>::Contains ( FUNCTOR&& COMP, U tValue ) NO_THREAD_SAFETY_ANALYSIS
{
	return any_of ( [&] ( const T& v ) { return COMP.IsEq ( v, tValue ); } );
}

/// fill with given value
template<typename T>
void VecTraits_T<T>::Fill ( const T& rhs )
{
	for ( int i = 0; i < m_iCount; ++i )
		m_pData[i] = rhs;
}

/// fill with sequence (appliable only to integers)
template<typename T>
void VecTraits_T<T>::FillSeq ( T iStart, T iStep )
{
	for ( int i = 0; i < m_iCount; ++i, iStart += iStep )
		m_pData[i] = iStart;
}
