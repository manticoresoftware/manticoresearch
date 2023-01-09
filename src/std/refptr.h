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

#include <type_traits>
#include <utility>
#include "generics.h"

//////////////////////////////////////////////////////////////////////////

/// automatic pointer wrapper for refcounted objects
/// construction from or assignment of a raw pointer takes over (!) the ownership
template<typename T>
class CSphRefcountedPtr
{
public:
	using ORIGTYPE = T;
	using RAWT = typename std::remove_const<T>::type;
	using CT = const T;
	using TYPE = CSphRefcountedPtr<T>;
	using CTYPE = CSphRefcountedPtr<const T>;
	using RAWTYPE = CSphRefcountedPtr<RAWT>;

public:
	CSphRefcountedPtr() noexcept = default; ///< default NULL wrapper construction (for vectors)
	explicit CSphRefcountedPtr ( T* pPtr ) noexcept
		: m_pPtr ( pPtr )
	{} ///< construction from raw pointer, takes over ownership!
	CSphRefcountedPtr ( std::nullptr_t ) noexcept {};

	CSphRefcountedPtr ( const CSphRefcountedPtr& rhs ) noexcept
		: m_pPtr ( rhs.m_pPtr )
	{
		SafeAddRef ( m_pPtr );
	}

	template<typename DERIVED>
	explicit CSphRefcountedPtr ( const CSphRefcountedPtr<DERIVED>& rhs ) noexcept
		: m_pPtr ( rhs.Ptr() )
	{
		SafeAddRef ( m_pPtr );
	}

	CSphRefcountedPtr ( CSphRefcountedPtr&& rhs ) noexcept
	{
		Swap ( rhs );
	}

	CSphRefcountedPtr& operator= ( CSphRefcountedPtr rhs ) noexcept
	{
		Swap ( rhs );
		return *this;
	}

	template<typename DERIVED>
	CSphRefcountedPtr& operator= ( const CSphRefcountedPtr<DERIVED>& rhs ) noexcept
	{
		SafeAddRef ( rhs.Ptr() );
		SafeRelease ( m_pPtr );
		m_pPtr = rhs.Ptr();
		return *this;
	}

	template<typename DERIVED>
	CSphRefcountedPtr& operator= ( CSphRefcountedPtr<DERIVED>&& rhs ) noexcept
	{
		SafeRelease ( m_pPtr );
		m_pPtr = rhs.Leak();
		return *this;
	}

	void Swap ( CSphRefcountedPtr& rhs ) noexcept
	{
		::Swap ( m_pPtr, rhs.m_pPtr );
	}

	~CSphRefcountedPtr() noexcept { SafeRelease ( m_pPtr ); }

	T* operator->() const noexcept { return m_pPtr; }
	explicit operator bool() const noexcept { return m_pPtr != nullptr; }
	operator T*() const noexcept { return m_pPtr; }

	// drop the ownership and reset pointer
	inline T* Leak() noexcept
	{
		return std::exchange ( m_pPtr, nullptr );
	}

	T* Ptr() const noexcept { return m_pPtr; }
	CT* CPtr() const noexcept { return m_pPtr; }

public:
	/// assignment of a raw pointer, takes over ownership!
	CSphRefcountedPtr& operator= ( T* pPtr ) noexcept
	{
		SafeRelease ( m_pPtr );
		m_pPtr = pPtr;
		return *this;
	}

protected:
	T* m_pPtr = nullptr;
};

template<typename T>
using RefCountedRefPtr_T = CSphRefcountedPtr<T>;

template<typename T>
using cRefCountedRefPtr_T = CSphRefcountedPtr<const T>;

template<typename T>
inline RefCountedRefPtr_T<T> ConstCastPtr ( cRefCountedRefPtr_T<T> rhs )
{
	auto* pRaw = const_cast<T*> ( rhs.Ptr() );
	if ( pRaw )
		pRaw->AddRef();
	return RefCountedRefPtr_T<T> { pRaw };
}