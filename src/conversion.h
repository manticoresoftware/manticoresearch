//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxdefs.h"

DWORD 	sphToDword ( const char * s );
int64_t sphToInt64 ( const char * szNumber, CSphString * pError=nullptr );
float 	sphToFloat ( const char * s );

/// float vs dword conversion
FORCE_INLINE DWORD sphF2DW ( float f )	{ union { float f; DWORD d; } u; u.f = f; return u.d; }

/// dword vs float conversion
FORCE_INLINE float sphDW2F ( DWORD d )	{ union { float f; DWORD d; } u; u.d = d; return u.f; }

/// double to bigint conversion
FORCE_INLINE uint64_t sphD2QW ( double f )	{ union { double f; uint64_t d; } u; u.f = f; return u.d; }

/// bigint to double conversion
FORCE_INLINE double sphQW2D ( uint64_t d )	{ union { double f; uint64_t d; } u; u.d = d; return u.f; }

template <typename T>
FORCE_INLINE T ConvertType ( SphAttr_t tValue )
{
	return (T)tValue;
}

template <>
FORCE_INLINE float ConvertType<float> ( SphAttr_t tValue )
{
	return sphDW2F ( (DWORD)tValue );
}

template <>
FORCE_INLINE double ConvertType<double> ( SphAttr_t tValue )
{
	return sphQW2D ( (uint64_t)tValue );
}
