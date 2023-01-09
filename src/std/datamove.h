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

#include "type_traits.h"
#include "ints.h"

#include <utility>
#include <cstring>

namespace sph
{

//////////////////////////////////////////////////////////////////////////
/// Copy backends for vector
/// Each backend provides Copy, Move and CopyOrSwap

/// Copy/move vec of a data item-by-item
template<typename T, bool C = IS_TRIVIALLY_COPYABLE ( T )>
class DataMover_T
{
public:
	static void Copy ( T* pNew, const T* pData, int64_t iLength );
	static void Move ( T* pNew, T* pData, int64_t iLength );
	static void Zero ( T* pData, int64_t iLength );
};

template<typename T> /// Copy/move blob of trivial data using memmove
class DataMover_T<T, true>
{
public:
	static void Copy ( T* pNew, const T* pData, int64_t iLength );
	static void Move ( T* pNew, T* pData, int64_t iLength );
	static void Zero ( T* pData, int64_t iLength );

	// append raw blob: defined ONLY in POD specialization.
	static void CopyVoid ( T* pNew, const void* pData, int64_t iLength );
};


/// default vector mover
template<typename T>
class DefaultCopy_T: public DataMover_T<T>
{
public:
	static void CopyOrSwap ( T& pLeft, const T& pRight );
};



/// swap-vector policy (for non-copyable classes)
/// use Swap() instead of assignment on resize
template<typename T>
class SwapCopy_T
{
public:
	static void Copy ( T* pNew, T* pData, int64_t iLength );
	static void Move ( T* pNew, T* pData, int64_t iLength );
	static void CopyOrSwap ( T& dLeft, T& dRight );
	static void CopyOrSwap ( T& dLeft, T&& dRight );
};

} // namespace sph

#include "datamove_impl.h"
