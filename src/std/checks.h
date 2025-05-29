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

#pragma once

#include "attrstub.h"
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// COMPILE-TIME CHECKS
/////////////////////////////////////////////////////////////////////////////

#define VARIABLE_IS_NOT_USED __attribute__((unused))

#define STATIC_SIZE_ASSERT(_type,_size)	static_assert ( sizeof(_type)==_size, "sizeof " #_type " must be " #_size " bytes" )

#ifndef __analysis_assume
#define __analysis_assume(_arg)
#endif


/// some function arguments only need to have a name in debug builds
#ifndef NDEBUG
#define DEBUGARG(_arg) _arg
#else
#define DEBUGARG(_arg)
#endif

/////////////////////////////////////////////////////////////////////////////
// DEBUGGING
/////////////////////////////////////////////////////////////////////////////

#if _WIN32
#ifndef NDEBUG

void sphAssert ( const char * sExpr, const char * sFile, int iLine );

#undef assert
#define assert(_expr) (void)( (_expr) || ( sphAssert ( #_expr, __FILE__, __LINE__ ), 0 ) )

#endif // !NDEBUG
#endif // _WIN32


// to avoid disappearing of _expr in release builds
#ifndef NDEBUG
#define Verify(_expr) assert(_expr)
#else
#define Verify(_expr) _expr
#endif

#ifndef NDEBUG
#define Debug( _expr ) _expr
#else
#define Debug(_expr)
#endif