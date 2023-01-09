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

#include "generics.h"

template<typename T>
typename SphAccessor_T<T>::MEDIAN_TYPE& SphAccessor_T<T>::Key (T* a) const
{
	return *a;
}

template<typename T>
void SphAccessor_T<T>::CopyKey ( SphAccessor_T<T>::MEDIAN_TYPE* pMed, T* pVal ) const
{
	*pMed = Key ( pVal );
}

template<typename T>
void SphAccessor_T<T>::Swap ( T* a, T* b ) const
{
	::Swap ( *a, *b );
}

template<typename T>
T* SphAccessor_T<T>::Add ( T* p, int i ) const
{
	return p + i;
}

template<typename T>
int SphAccessor_T<T>::Sub ( T* b, T* a ) const
{
	return (int)( b - a );
}
