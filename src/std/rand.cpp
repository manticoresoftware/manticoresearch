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

#include "rand.h"

#if !_WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

//////////////////////////////////////////////////////////////////////////
// RANDOM NUMBERS GENERATOR
//////////////////////////////////////////////////////////////////////////

/// MWC (Multiply-With-Carry) RNG, invented by George Marsaglia
static DWORD g_dRngState[5] = { 0x95d3474bUL, 0x035cf1f7UL, 0xfd43995fUL, 0x5dfc55fbUL, 0x334a9229UL };


/// seed
void sphSrand ( DWORD uSeed )
{
	for ( int i = 0; i < 5; ++i )
	{
		uSeed = uSeed * 29943829 - 1;
		g_dRngState[i] = uSeed;
	}
	for ( int i = 0; i < 19; ++i )
		sphRand();
}


/// auto-seed RNG based on time and PID
void sphAutoSrand()
{
	// get timestamp
	DWORD uPID;
#if !_WIN32
	struct timeval tv;
	gettimeofday ( &tv, NULL );
	uPID = getpid();
#else
	struct
	{
		time_t tv_sec;
		DWORD tv_usec;
	} tv;

	FILETIME ft;
	GetSystemTimeAsFileTime ( &ft );

	uint64_t ts = ( uint64_t ( ft.dwHighDateTime ) << 32 ) + uint64_t ( ft.dwLowDateTime ) - 116444736000000000ULL; // Jan 1, 1970 magic
	ts /= 10;																										// to microseconds
	tv.tv_sec = (DWORD)( ts / 1000000 );
	tv.tv_usec = (DWORD)( ts % 1000000 );
	uPID = GetCurrentProcessId();
#endif

	// twist and shout
	sphSrand ( sphRand() ^ DWORD ( tv.tv_sec ) ^ ( DWORD ( tv.tv_usec ) + uPID ) );
}

/// generate another dword
DWORD sphRand ()
{
	uint64_t uSum;
	uSum =
		(uint64_t)g_dRngState[0] * U64C(5115) +
		(uint64_t)g_dRngState[1] * U64C(1776) +
		(uint64_t)g_dRngState[2] * U64C(1492) +
		(uint64_t)g_dRngState[3] * U64C(2111111111) +
		(uint64_t)g_dRngState[4];
	g_dRngState[3] = g_dRngState[2];
	g_dRngState[2] = g_dRngState[1];
	g_dRngState[1] = g_dRngState[0];
	g_dRngState[4] = (DWORD)( uSum>>32 );
	g_dRngState[0] = (DWORD)uSum;
	return g_dRngState[0];
}