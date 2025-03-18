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


template<typename T>
inline T* sph::CustomStorage_T<T>::Allocate ( int64_t iLimit )
{
	return sphAllocateSmall ( int ( iLimit * sizeof ( T ) ) );
}

#if WITH_SMALLALLOC

template<typename T>
inline void sph::CustomStorage_T<T>::Deallocate ( T* pData, int64_t iLimit )
{
	sphDeallocateSmall ( (BYTE*)pData, int ( iLimit * sizeof ( T ) ) );
}

#else

template<typename T>
FORCE_INLINE void sph::CustomStorage_T<T>::Deallocate ( T * pData )
{
	sphDeallocateSmall ( (BYTE*)pData );
}

FORCE_INLINE BYTE* sphAllocateSmall ( int iBytes )
{
	return new BYTE[iBytes];
};

FORCE_INLINE void sphDeallocateSmall ( const BYTE* pBlob, int )
{
	delete[] pBlob;
};

FORCE_INLINE void sphDeallocateSmall ( const BYTE* pBlob )
{
	delete[] pBlob;
};

// how many allocated right now
inline size_t sphGetSmallAllocatedSize()
{
	return 0;
};

// how many pooled from the sys right now
inline size_t sphGetSmallReservedSize()
{
	return 0;
};

#endif // WITH_SMALLALLOC
