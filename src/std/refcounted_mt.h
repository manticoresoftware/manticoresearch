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

#include "generics.h"
#include <atomic>
#include "refptr.h"
#include "vector.h"

//////////////////////////////////////////////////////////////////////////


/// MT-aware refcounted base (uses atomics that sometimes m.b. slow because of inter-cpu sync)
struct ISphRefcountedMT: public ISphNoncopyable
{
protected:
	virtual ~ISphRefcountedMT() = default;

public:
	inline void AddRef() const noexcept;
	inline void Release() const noexcept;
	inline long GetRefcount() const;
	inline bool IsLast() const;

private:
	mutable std::atomic<long> m_iRefCount { 1 };
};

using cRefCountedRefPtrGeneric_t = cRefCountedRefPtr_T<ISphRefcountedMT>;
using RefCountedRefPtrGeneric_t = RefCountedRefPtr_T<ISphRefcountedMT>;

/// vector of refcounted pointers (safe-release each ptr in dtr)
template<class T>
struct VecRefPtrs_t: public ISphNoncopyable, public CSphVector<T>
{
	using CSphVector<T>::SwapData;

	VecRefPtrs_t() = default;
	VecRefPtrs_t ( VecRefPtrs_t<T>&& rhs ) noexcept;
	VecRefPtrs_t& operator= ( VecRefPtrs_t<T>&& rhs ) noexcept;
	~VecRefPtrs_t();
};

#include "refcounted_mt_impl.h"
