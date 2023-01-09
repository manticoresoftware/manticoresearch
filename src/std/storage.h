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

//////////////////////////////////////////////////////////////////////////
/// Storage backends for vector
/// Each backend provides Allocate and Deallocate

namespace sph
{

/// Default backend - uses plain old new/delete
template<typename T>
class DefaultStorage_T
{
public:
	using TYPE = T;

	inline static TYPE* Allocate ( int64_t iLimit )
	{
		return new T[iLimit];
	}

	inline static void Deallocate ( TYPE* pData )
	{
		delete[] pData;
	}

	static constexpr bool is_sized = false;
	static constexpr bool is_constructed = true;
	static constexpr bool is_owned = false;
};



/// Static backend: small blobs stored localy,
/// bigger came to plain old new/delete
template<typename T, int STATICSIZE = 4096>
class LazyStorage_T
{
public:
	using TYPE = T;

	// don't allow moving (it has no sence with embedded buffer)
	inline LazyStorage_T ( LazyStorage_T&& rhs ) noexcept = delete;
	inline LazyStorage_T& operator= ( LazyStorage_T&& rhs ) noexcept = delete;

	LazyStorage_T() = default;
	static const int iSTATICSIZE = STATICSIZE;

public:
	inline TYPE* Allocate ( int64_t iLimit )
	{
		if ( iLimit <= STATICSIZE )
			return m_dData;
		return new T[iLimit];
	}

	inline void Deallocate ( TYPE* pData ) const
	{
		if ( pData != m_dData )
			delete[] pData;
	}

	static constexpr bool is_sized = false;
	static constexpr bool is_constructed = true;
	static constexpr bool is_owned = true;

private:
	T m_dData[iSTATICSIZE];
};



/// optional backend - allocates space but *not* calls ctrs and dtrs
/// bigger came to plain old new/delete
template<typename T>
class RawStorage_T
{
	using StorageType = typename std::aligned_storage<sizeof ( T ), alignof ( T )>::type;

public:
	using TYPE = T;

	inline static TYPE* Allocate ( int64_t iLimit )
	{
		return (TYPE*)new StorageType[iLimit];
	}

	inline static void Deallocate ( TYPE* pData )
	{
		delete[] reinterpret_cast<StorageType*> ( pData );
	}

	static constexpr bool is_sized = false;
	// static const bool is_constructed = IS_TRIVIALLY_COPYABLE( T );
	static constexpr bool is_constructed = IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE ( T );
	static constexpr bool is_owned = false;
};

} // namespace sph