//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "timers.h"

#if !_WIN32
#include <sys/time.h> // for gettimeofday
#include <time.h>
#endif

//////////////////////////////////////////////////////////////////////////
#if _WIN32
inline static int64_t winMicroTimer()
{

	// Windows time query
	static int64_t iBase = 0;
	static int64_t iStart = 0;
	static int64_t iFreq = 0;

	LARGE_INTEGER iLarge;
	if ( !iBase )
	{
		// get start QPC value
		QueryPerformanceFrequency ( &iLarge );
		iFreq = iLarge.QuadPart;
		QueryPerformanceCounter ( &iLarge );
		iStart = iLarge.QuadPart;

		// get start UTC timestamp
		// assuming it's still approximately the same moment as iStart, give or take a msec or three
		FILETIME ft;
		GetSystemTimeAsFileTime ( &ft );

		iBase = ( int64_t ( ft.dwHighDateTime ) << 32 ) + int64_t ( ft.dwLowDateTime );
		iBase = ( iBase - 116444736000000000ULL ) / 10; // rebase from 01 Jan 1601 to 01 Jan 1970, and rescale to 1 usec from 100 ns
	}

	// we can't easily drag iBase into parens because iBase*iFreq/1000000 overflows 64bit int!
	QueryPerformanceCounter ( &iLarge );
	return iBase + ( iLarge.QuadPart - iStart ) * 1000000 / iFreq;
}
#endif

/// microsecond precision timestamp
int64_t sphMicroTimer()
{
#if _WIN32
	return winMicroTimer();
#else
	// UNIX time query
	struct timeval tv;
	gettimeofday ( &tv, NULL );
	return int64_t(tv.tv_sec)*int64_t(1000000) + int64_t(tv.tv_usec);
#endif // _WIN32
}

/// monotonic microsecond precision timestamp
int64_t MonoMicroTimer()
{
#if _WIN32
	return winMicroTimer();
#else

	// UNIX time query
	struct timespec tp;
	if ( clock_gettime ( CLOCK_MONOTONIC, &tp ) )
		return 0;

	return tp.tv_sec * 1000000 + tp.tv_nsec / 1000;
#endif
}


volatile bool& sphGetbCpuStat() noexcept;

/// return cpu time, in microseconds
int64_t sphThreadCpuTimer ()
{
	if ( !sphGetbCpuStat() )
		return 0;

#ifdef HAVE_CLOCK_GETTIME
#if defined (CLOCK_THREAD_CPUTIME_ID)
	// CPU time (user+sys), Linux style, current thread
	constexpr static clockid_t tClockLoc = CLOCK_THREAD_CPUTIME_ID;
#elif defined( CLOCK_PROF )
	// CPU time (user+sys), FreeBSD style
	constexpr static clockid_t tClockLoc = CLOCK_PROF;
#else
	// POSIX fallback (wall time)
	constexpr static clockid_t tClockLoc = CLOCK_REALTIME;
#endif

	struct timespec tp;
	if ( clock_gettime ( tClockLoc, &tp ) )
		return 0;

	return tp.tv_sec*1000000 + tp.tv_nsec/1000;
#else
	return sphMicroTimer();
#endif
}

/// return cpu time, in microseconds
int64_t sphProcessCpuTimer()
{
	if ( !sphGetbCpuStat() )
		return 0;

#ifdef HAVE_CLOCK_GETTIME
#if defined( CLOCK_PROCESS_CPUTIME_ID )
	// CPU time (user+sys), Linux style, current thread
	constexpr static clockid_t tClockLoc = CLOCK_PROCESS_CPUTIME_ID;
#elif defined( CLOCK_PROF )
	// CPU time (user+sys), FreeBSD style
	constexpr static clockid_t tClockLoc = CLOCK_PROF;
#else
	// POSIX fallback (wall time)
	constexpr static clockid_t tClockLoc = CLOCK_REALTIME;
#endif

	struct timespec tp;
	if ( clock_gettime ( tClockLoc, &tp ) )
		return 0;

	return tp.tv_sec * 1000000 + tp.tv_nsec / 1000;
#else
	return sphMicroTimer();
#endif
}