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

inline void ISphRefcountedMT::AddRef() const noexcept
{
	m_iRefCount.fetch_add ( 1, std::memory_order_relaxed );
}

inline void ISphRefcountedMT::Release() const noexcept
{
	if ( m_iRefCount.fetch_sub ( 1, std::memory_order_release ) == 1 )
	{
		std::atomic_thread_fence ( std::memory_order_acquire );
		assert ( m_iRefCount.load ( std::memory_order_seq_cst ) == 0 );
		delete this;
	}
}

inline long ISphRefcountedMT::GetRefcount() const
{
	return m_iRefCount.load ( std::memory_order_acquire );
}

inline bool ISphRefcountedMT::IsLast() const
{
	return 1 == m_iRefCount.load ( std::memory_order_acquire );
}


template<class T>
VecRefPtrs_t<T>::VecRefPtrs_t ( VecRefPtrs_t<T>&& rhs ) noexcept
{
	SwapData ( rhs );
}

template<class T>
VecRefPtrs_t<T>& VecRefPtrs_t<T>::operator= ( VecRefPtrs_t<T>&& rhs ) noexcept
{
	SwapData ( rhs );
	return *this;
}

template<class T>
VecRefPtrs_t<T>::~VecRefPtrs_t()
{
	CSphVector<T>::Apply ( [] ( T& ptr ) { SafeRelease ( ptr ); } );
}
