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

#include "datamove.h"
#include "storage.h"
#include "generics.h"
#include "vectraits.h"

#include <type_traits>

//////////////////////////////////////////////////////////////////////////

/// dynamically allocated fixed-size vector
template<typename T, class POLICY = sph::DefaultCopy_T<T>, class STORE = sph::DefaultStorage_T<T>>
class CSphFixedVector: public ISphNoncopyable
	, public VecTraits_T<T>
	, protected STORE
{
protected:
	using VecTraits_T<T>::m_pData;
	using VecTraits_T<T>::m_iCount;

public:
	using POLICY_T = POLICY;
	using STORE_T = STORE;

	explicit CSphFixedVector ( int64_t iSize ) noexcept;

	~CSphFixedVector();

	CSphFixedVector ( CSphFixedVector&& rhs ) noexcept;

	CSphFixedVector& operator= ( CSphFixedVector rhs ) noexcept;

	void Reset ( int64_t iSize );

	// overwrite content with copy of dOrigin
	void CopyFrom ( const VecTraits_T<T>& dOrigin );

	template<typename S = STORE>
	typename std::enable_if<!S::is_owned, T*>::type LeakData();

	/// swap
	template<typename S = STORE>
	typename std::enable_if<!S::is_owned>::type SwapData ( CSphFixedVector& rhs ) noexcept;

	template<typename S = STORE>
	typename std::enable_if<S::is_constructed && !S::is_owned>::type Set ( T* pData, int64_t iSize );

	/// Set whole vec to 0. For trivially copyable memset will be used
	void ZeroVec();
};

#include "fixedvector_impl.h"
