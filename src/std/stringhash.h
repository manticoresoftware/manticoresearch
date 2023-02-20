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

#include "string.h"
#include "orderedhash.h"

/////////////////////////////////////////////////////////////////////////////

/// string hash function
struct CSphStrHashFunc
{
	static int Hash ( const CSphString& sKey );
};

/// small hash with string keys
template<typename T, int LENGTH = 256>
using SmallStringHash_T = CSphOrderedHash<T, CSphString, CSphStrHashFunc, LENGTH>;


namespace sph
{

// used to simple add/delete strings and check if a string was added by [] op
class StringSet: private SmallStringHash_T<bool>
{
	using BASE = SmallStringHash_T<bool>;

public:
	inline bool Add ( const CSphString& sKey )
	{
		return BASE::Add ( true, sKey );
	}

	inline bool operator[] ( const CSphString& sKey ) const
	{
		HashEntry_t* pEntry = FindByKey ( sKey );
		return pEntry ? pEntry->second : false;
	}

	StringSet ( std::initializer_list<const char *> dVals );
	StringSet() = default;

	using BASE::Delete;
	using BASE::Reset;
	using BASE::GetLength;
	using BASE::IsEmpty;

	using BASE::begin;
	using BASE::end;
	using Iterator_c = BASE::Iterator_c;
};
}