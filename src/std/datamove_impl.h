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

#include <utility>
#include <cstring>
#include "datamove.h"


namespace sph {

template<typename T, bool _>
void DataMover_T<T, _>::Copy ( T* pNew, const T* pData, int64_t iLength )
{
	for ( int i = 0; i < iLength; ++i )
		pNew[i] = pData[i];
}


template<typename T, bool C>
void DataMover_T<T, C>::Move ( T* pNew, T* pData, int64_t iLength )
{
	for ( int i = 0; i < iLength; ++i )
		pNew[i] = std::move ( pData[i] );
}

template<typename T, bool _>
void DataMover_T<T, _>::Zero ( T* pData, int64_t iLength )
{
	for ( int i = 0; i < iLength; ++i )
		pData[i] = 0;
}

template<typename T>
void DataMover_T<T, true>::Copy ( T* pNew, const T* pData, int64_t iLength )
{
	if ( iLength ) // m.b. work without this check, but sanitize for paranoids.
		memmove ( (void*)pNew, (const void*)pData, iLength * sizeof ( T ) );
}

template<typename T>
void DataMover_T<T, true>::Move ( T* pNew, T* pData, int64_t iLength )
{
	Copy ( pNew, pData, iLength );
}

// append raw blob: defined ONLY in POD specialization.
template<typename T>
void DataMover_T<T, true>::CopyVoid ( T* pNew, const void* pData, int64_t iLength )
{
	Copy ( pNew, (T*)const_cast<void*> ( pData ), iLength );
}

template<typename T>
void DataMover_T<T, true>::Zero ( T* pData, int64_t iLength )
{
	memset ( (void*)pData, 0, iLength * sizeof ( T ) );
}


/// default vector mover
template<typename T>
void DefaultCopy_T<T>::CopyOrSwap ( T& pLeft, const T& pRight )
{
	pLeft = pRight;
}


/// swap-vector policy (for non-copyable classes)
/// use Swap() instead of assignment on resize
template<typename T>
void SwapCopy_T<T>::Copy ( T* pNew, T* pData, int64_t iLength )
{
	for ( int i = 0; i < iLength; ++i )
		Swap ( pNew[i], pData[i] );
}

template<typename T>
void SwapCopy_T<T>::Move ( T* pNew, T* pData, int64_t iLength )
{
	for ( int i = 0; i < iLength; ++i )
		Swap ( pNew[i], pData[i] );
}

template<typename T>
void SwapCopy_T<T>::CopyOrSwap ( T& dLeft, T& dRight )
{
	Swap ( dLeft, dRight );
}

template<typename T>
void SwapCopy_T<T>::CopyOrSwap ( T& dLeft, T&& dRight )
{
	dLeft = std::forward<T> (dRight);
}

} // namespace sph