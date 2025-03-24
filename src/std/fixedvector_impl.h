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
#include <utility>

template<typename T, class POLICY, class STORE>
CSphFixedVector<T,POLICY,STORE>::CSphFixedVector ( int64_t iSize ) noexcept
{
	m_iCount = iSize;
	assert ( iSize >= 0 );
	m_pData = ( iSize > 0 ) ? STORE::Allocate ( iSize ) : nullptr;
}

template<typename T, class POLICY, class STORE>
CSphFixedVector<T, POLICY, STORE>::~CSphFixedVector()
{
	STORE::Deallocate ( m_pData );
}

template<typename T, class POLICY, class STORE>
CSphFixedVector<T, POLICY, STORE>::CSphFixedVector ( CSphFixedVector&& rhs ) noexcept
{
	SwapData ( rhs );
}

template<typename T, class POLICY, class STORE>
CSphFixedVector<T, POLICY, STORE>& CSphFixedVector<T, POLICY, STORE>::operator= ( CSphFixedVector rhs ) noexcept
{
	SwapData ( rhs );
	return *this;
}

template<typename T, class POLICY, class STORE>
void CSphFixedVector<T, POLICY, STORE>::Reset ( int64_t iSize )
{
	assert ( iSize >= 0 );
	if ( iSize == m_iCount )
		return;

	STORE::Deallocate ( m_pData );
	m_pData = ( iSize > 0 ) ? STORE::Allocate ( iSize ) : nullptr;
	m_iCount = iSize;
}

template<typename T, class POLICY, class STORE>
void CSphFixedVector<T, POLICY, STORE>::CopyFrom ( const VecTraits_T<T>& dOrigin )
{
	Reset ( dOrigin.GetLength() );
	POLICY::Copy ( m_pData, dOrigin.begin(), dOrigin.GetLength() );
}

template<typename T, class POLICY, class STORE>
template<typename S>
typename std::enable_if<!S::is_owned, T*>::type CSphFixedVector<T, POLICY, STORE>::LeakData()
{
	T* pData = m_pData;
	m_pData = nullptr;
	Reset ( 0 );
	return pData;
}

/// swap
template<typename T, class POLICY, class STORE>
template<typename S>
typename std::enable_if<!S::is_owned>::type CSphFixedVector<T, POLICY, STORE>::SwapData ( CSphFixedVector& rhs ) noexcept
{
	std::swap ( m_pData, rhs.m_pData );
	std::swap ( m_iCount, rhs.m_iCount );
}

template<typename T, class POLICY, class STORE>
template<typename S>
typename std::enable_if<S::is_constructed && !S::is_owned>::type CSphFixedVector<T, POLICY, STORE>::Set ( T* pData, int64_t iSize )
{
	m_pData = pData;
	m_iCount = iSize;
}

/// Set whole vec to 0. For trivially copyable memset will be used
template<typename T, class POLICY, class STORE>
void CSphFixedVector<T, POLICY, STORE>::ZeroVec()
{
	POLICY::Zero ( m_pData, m_iCount );
}
