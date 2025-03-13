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

#include <utility>

#include "log2.h"
#include "accessor.h"
#include "comp.h"

namespace sphstd {

/// heap sort helper
template<typename T, typename U, typename V>
void SiftDown ( T* pData, int iStart, int iEnd, U&& COMP, V&& ACC )
{
	while ( true )
	{
		int iChild = iStart * 2 + 1;
		if ( iChild > iEnd )
			return;

		int iChild1 = iChild + 1;
		if ( iChild1 <= iEnd && COMP.IsLess ( ACC.Key ( ACC.Add ( pData, iChild ) ), ACC.Key ( ACC.Add ( pData, iChild1 ) ) ) )
			iChild = iChild1;

		if ( COMP.IsLess ( ACC.Key ( ACC.Add ( pData, iChild ) ), ACC.Key ( ACC.Add ( pData, iStart ) ) ) )
			return;
		ACC.Swap ( ACC.Add ( pData, iChild ), ACC.Add ( pData, iStart ) );
		iStart = iChild;
	}
}


/// heap sort
template<typename T, typename U, typename V>
void HeapSort ( T* pData, int iCount, U&& COMP, V&& ACC )
{
	if ( !pData || iCount <= 1 )
		return;

	// build a max-heap, so that the largest element is root
	for ( int iStart = ( iCount - 2 ) >> 1; iStart >= 0; --iStart )
		SiftDown ( pData, iStart, iCount - 1, std::forward<U> ( COMP ), std::forward<V> ( ACC ) );

	// now keep popping root into the end of array
	for ( int iEnd = iCount - 1; iEnd > 0; )
	{
		ACC.Swap ( pData, ACC.Add ( pData, iEnd ) );
		SiftDown ( pData, 0, --iEnd, std::forward<U> ( COMP ), std::forward<V> ( ACC ) );
	}
}


/// generic sort
template<typename T, typename U, typename V>
void Sort ( T* pData, int iCount, U&& COMP, V&& ACC )
{
	if ( iCount < 2 )
		return;

	typedef T* P;
	// st0 and st1 are stacks with left and right bounds of array-part.
	// They allow us to avoid recursion in quicksort implementation.
	P st0[32], st1[32], a, b, i, j;
	typename std::remove_reference<V>::type::MEDIAN_TYPE x;
	int k;

	const int SMALL_THRESH = 32;
	int iDepthLimit = sphLog2 ( iCount );
	iDepthLimit = ( ( iDepthLimit << 2 ) + iDepthLimit ) >> 1; // x2.5

	k = 1;
	st0[0] = pData;
	st1[0] = ACC.Add ( pData, iCount - 1 );
	while ( k )
	{
		k--;
		i = a = st0[k];
		j = b = st1[k];

		// if quicksort fails on this data; switch to heapsort
		if ( !k )
		{
			if ( !--iDepthLimit )
			{
				HeapSort ( a, ACC.Sub ( b, a ) + 1, std::forward<U> ( COMP ), ACC );
				return;
			}
		}

		// for tiny arrays, switch to insertion sort
		int iLen = ACC.Sub ( b, a );
		if ( iLen <= SMALL_THRESH )
		{
			for ( i = ACC.Add ( a, 1 ); i <= b; i = ACC.Add ( i, 1 ) )
			{
				for ( j = i; j > a; )
				{
					P j1 = ACC.Add ( j, -1 );
					if ( COMP.IsLess ( ACC.Key ( j1 ), ACC.Key ( j ) ) )
						break;
					ACC.Swap ( j, j1 );
					j = j1;
				}
			}
			continue;
		}

		// ATTENTION! This copy can lead to memleaks if your CopyKey
		// copies something which is not freed by objects destructor.
		ACC.CopyKey ( &x, ACC.Add ( a, iLen / 2 ) );
		while ( a < b )
		{
			while ( i <= j )
			{
				while ( COMP.IsLess ( ACC.Key ( i ), x ) )
					i = ACC.Add ( i, 1 );
				while ( COMP.IsLess ( x, ACC.Key ( j ) ) )
					j = ACC.Add ( j, -1 );
				if ( i <= j )
				{
					ACC.Swap ( i, j );
					i = ACC.Add ( i, 1 );
					j = ACC.Add ( j, -1 );
				}
			}

			// Not so obvious optimization. We put smaller array-parts
			// to the top of stack. That reduces peak stack size.
			if ( ACC.Sub ( j, a ) >= ACC.Sub ( b, i ) )
			{
				if ( a < j ) {
					st0[k] = a;
					st1[k] = j;
					k++;
				}
				a = i;
			} else
			{
				if ( i < b ) {
					st0[k] = i;
					st1[k] = b;
					k++;
				}
				b = j;
			}
		}
	}
}

} // namespace sphstd

template<typename T, typename U, typename V>
void sphSort ( T* pData, int iCount, U&& COMP, V&& ACC ) noexcept
{
	sphstd::Sort ( pData, iCount, std::forward<U> ( COMP ), std::forward<V> ( ACC ) );
}

template<typename T, typename U>
void sphSort ( T* pData, int iCount, U&& COMP ) noexcept
{
	sphSort ( pData, iCount, std::forward<U> ( COMP ), SphAccessor_T<T>() );
}

template<typename T>
void sphSort ( T* pData, int iCount ) noexcept
{
	sphSort ( pData, iCount, SphLess_T<T>() );
}