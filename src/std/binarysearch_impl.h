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

#include <assert.h>
#include "comp.h"

namespace sphstd {

/// generic binary search
template<typename T, typename U, typename PRED>
T* BinarySearch ( T* pStart, T* pEnd, PRED&& tPred, U tRef )
{
	if ( !pStart || pEnd < pStart )
		return nullptr;

	if ( tPred ( *pStart ) == tRef )
		return pStart;

	if ( tPred ( *pEnd ) == tRef )
		return pEnd;

	while ( pEnd - pStart > 1 )
	{
		if ( tRef < tPred ( *pStart ) || tPred ( *pEnd ) < tRef )
			break;
		assert ( tPred ( *pStart ) < tRef );
		assert ( tRef < tPred ( *pEnd ) );

		T* pMid = pStart + ( pEnd - pStart ) / 2;
		if ( tRef == tPred ( *pMid ) )
			return pMid;

		if ( tRef < tPred ( *pMid ) )
			pEnd = pMid;
		else
			pStart = pMid;
	}
	return nullptr;
}


// returns first (leftmost) occurrence of the value
// returns -1 if not found
template<typename T, typename U, typename PRED>
int BinarySearchFirst ( T* pValues, int iStart, int iEnd, PRED&& tPred, U tRef )
{
	assert ( iStart <= iEnd );

	while ( iEnd - iStart > 1 )
	{
		if ( tRef < tPred ( pValues[iStart] ) || tRef > tPred ( pValues[iEnd] ) )
			return -1;

		int iMid = iStart + ( iEnd - iStart ) / 2;
		if ( tPred ( pValues[iMid] ) >= tRef )
			iEnd = iMid;
		else
			iStart = iMid;
	}

	return iEnd;
}



// find the first entry that is greater than tRef
template<typename T, typename U, typename PRED>
T* BinarySearchFirst ( T* pStart, T* pEnd, PRED&& tPred, U tRef )
{
	if ( !pStart || pEnd < pStart )
		return nullptr;

	while ( pStart != pEnd )
	{
		T* pMid = pStart + ( pEnd - pStart ) / 2;
		if ( tRef > tPred ( *pMid ) )
			pStart = pMid + 1;
		else
			pEnd = pMid;
	}

	return pStart;
}

} // namespace sphstd


template<typename T, typename U, typename PRED>
T* sphBinarySearch ( T* pStart, T* pEnd, PRED&& tPred, U tRef )
{
	return sphstd::BinarySearch ( pStart, pEnd, std::forward<PRED> (tPred), tRef );
}

template<typename T>
T* sphBinarySearch ( T* pStart, T* pEnd, T& tRef )
{
	return sphstd::BinarySearch ( pStart, pEnd, SphIdentityFunctor_T<T>(), tRef );
}

template<typename T, typename U, typename PRED>
int sphBinarySearchFirst ( T* pValues, int iStart, int iEnd, PRED&& tPred, U tRef )
{
	return sphstd::BinarySearchFirst ( pValues, iStart, iEnd, std::forward<PRED> ( tPred ), tRef );
}

template<typename T, typename U, typename PRED>
T* sphBinarySearchFirst ( T* pStart, T* pEnd, PRED&& tPred, U tRef )
{
	return sphstd::BinarySearchFirst ( pStart, pEnd, std::forward<PRED> ( tPred ), tRef );
}