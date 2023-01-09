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

#include "config.h"

#include <stdio.h>

// for 64-bit types
#if HAVE_STDINT_H
#include <stdint.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include "checks.h"

#if _WIN32
	#include <io.h>
	#define sphSeek		_lseeki64
	#define popen		_popen
	#define pclose		_pclose
	using SphOffset_t = int64_t;
#else
	#include <unistd.h>
	#define sphSeek		lseek
	using SphOffset_t = off_t;
#endif

/////////////////////////////////////////////////////////////////////////////
// PORTABILITY
/////////////////////////////////////////////////////////////////////////////

#if _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#define strcasecmp strcmpi
#define strncasecmp _strnicmp
#define snprintf _snprintf
#define strtoll _strtoi64
#define strtoull _strtoui64

#else

	using DWORD = unsigned int;
	using WORD = unsigned short;
	using BYTE = unsigned char;

#endif // _WIN32

/////////////////////////////////////////////////////////////////////////////
// 64-BIT INTEGER TYPES AND MACROS
/////////////////////////////////////////////////////////////////////////////

#if defined(U64C) || defined(I64C)
#error "Internal 64-bit integer macros already defined."
#endif

// if platform-specific macros were not supplied, use common defaults
#ifndef U64C
#define U64C(v) v ## ULL
#endif

#ifndef I64C
#define I64C(v) v ## LL
#endif

#ifndef UINT64_MAX
#define UINT64_MAX U64C(0xffffffffffffffff)
#endif

#ifndef INT64_MIN
#define INT64_MIN I64C(0x8000000000000000)
#endif

#ifndef INT64_MAX
#define INT64_MAX I64C(0x7fffffffffffffff)
#endif

STATIC_SIZE_ASSERT ( uint64_t, 8 );
STATIC_SIZE_ASSERT ( int64_t, 8 );

// conversion macros that suppress %lld format warnings vs printf
// problem is, on 64-bit Linux systems with gcc and stdint.h, int64_t is long int
// and despite sizeof(long int)==sizeof(long long int)==8, gcc bitches about that
// using PRIi64 instead of %lld is of course The Right Way, but ugly like fuck
// so lets wrap them args in INT64() instead
#define INT64(_v) ((long long int)(_v))
#define UINT64(_v) ((unsigned long long int)(_v))