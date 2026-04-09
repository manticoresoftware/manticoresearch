//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "generics.h"

template<typename T, typename INT>
FORCE_INLINE typename SphAccessor_T<T,INT>::MEDIAN_TYPE& SphAccessor_T<T,INT>::Key (T* a) const
{
	return *a;
}

template<typename T, typename INT>
FORCE_INLINE void SphAccessor_T<T,INT>::CopyKey ( SphAccessor_T<T,INT>::MEDIAN_TYPE* pMed, T* pVal ) const
{
	*pMed = Key ( pVal );
}

template<typename T, typename INT>
FORCE_INLINE void SphAccessor_T<T,INT>::Swap ( T* a, T* b ) const
{
	::Swap ( *a, *b );
}

template<typename T, typename INT>
FORCE_INLINE T * SphAccessor_T<T, INT>::Add ( T* p, INT i ) const
{
	return p + i;
}

template<typename T, typename INT>
FORCE_INLINE INT SphAccessor_T<T,INT>::Sub ( T* b, T* a ) const
{
	return (INT)( b - a );
}
