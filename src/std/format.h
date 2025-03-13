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

#include "config.h"
#include <limits.h>
#include <cstring>

#if __has_include(<inttypes.h>)
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#ifndef PRIu64
#define PRIu64 "llu"
#endif

#ifndef PRIi64
#define PRIi64 "lld"
#endif

#define UINT64_FMT "%" PRIu64
#define INT64_FMT "%" PRIi64

#ifndef PRIx64
#define PRIx64 "llx"
#endif

#if __has_include( <charconv>)
#include <charconv>
#else
template<typename T>
inline static char* FormatInt ( char sBuf[32], T v )
{
	if ( sizeof ( T ) == 4 && v == INT_MIN )
		return strncpy ( sBuf, "-2147483648", 32 );
	if ( sizeof ( T ) == 8 && v == LLONG_MIN )
		return strncpy ( sBuf, "-9223372036854775808", 32 );

	bool s = ( v < 0 );
	if ( s )
		v = -v;

	char* p = sBuf + 31;
	*p = 0;
	do
	{
		*--p = '0' + char ( v % 10 );
		v /= 10;
	} while ( v );
	if ( s )
		*--p = '-';
	return p;
}
#endif
