//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include <benchmark/benchmark.h>

#include "fileutils.h"
#include "std/string.h"
#include "std/fnv64.h"
#include "stripper/html_stripper.h"

class BM_stripper: public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State& state )
	{
		auto sPath = GetPathOnly(__FILE__);
		auto sTestFile = SphSprintf("%s../../test/bench/sphinx.html", sPath.cstr());

		FILE* fp = fopen ( sTestFile.cstr(), "rb" );
		if ( !fp )
		{
			printf ( "benchmark failed: unable to read %s\n", sTestFile.cstr() );
			return;
		}

		sBuf = new char[MAX_SIZE];
		iLen = (int)fread ( sBuf, 1, MAX_SIZE - 1, fp );
		sBuf[iLen] = '\0';
		fclose ( fp );

		sRef = new char[MAX_SIZE];
		memcpy ( sRef, sBuf, iLen + 1 );
	}

	void TearDown ( const ::benchmark::State& state )
	{
		SafeDeleteArray ( sBuf );
		SafeDeleteArray ( sRef );
	}

	static constexpr int MAX_SIZE = 1048576;
	char* sBuf = nullptr;
	char* sRef = nullptr;
	int iLen=0;
};

BENCHMARK_F ( BM_stripper, bench_default )
( benchmark::State& st )
{
	CSphHTMLStripper tStripper ( true );
	int iBytes = 0;
	for ( auto _ : st )
	{
		tStripper.Strip ( (BYTE*)sBuf );
		st.PauseTiming();
		memcpy ( sBuf, sRef, iLen + 1 );
		iBytes += iLen;
		st.ResumeTiming();
	}
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_F ( BM_stripper, bench_nostyle_script )
( benchmark::State& st )
{
	CSphHTMLStripper tStripper ( true );
	CSphString sError;
	tStripper.SetRemovedElements ( "style, script", sError );
	int iBytes = 0;
	for ( auto _ : st )
	{
		tStripper.Strip ( (BYTE*)sBuf );
		st.PauseTiming();
		memcpy ( sBuf, sRef, iLen + 1 );
		iBytes += iLen;
		st.ResumeTiming();
	}
	st.SetLabel ( "Without tags 'style' and 'script'" );
	st.SetBytesProcessed ( iBytes );
}

class FNV_hasher: public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State& state )
	{
		auto sPath = GetPathOnly(__FILE__);
		auto sTestFile = SphSprintf("%s../../test/bench/sphinx.html", sPath.cstr());

		FILE* fp = fopen ( sTestFile.cstr(), "rb" );
		if ( !fp )
		{
			printf ( "benchmark failed: unable to read %s\n", sTestFile.cstr() );
			return;
		}

		sBuf = new char[MAX_SIZE];
		iLen = (int)fread ( sBuf, 1, MAX_SIZE - 1, fp );
		sBuf[iLen] = '\0';
		fclose ( fp );
	}

	void TearDown ( const ::benchmark::State& state )
	{
		SafeDeleteArray ( sBuf );
	}

	static constexpr int MAX_SIZE = 1048576;
	char* sBuf = nullptr;
	int iLen=0;
};

uint64_t sphFNV64_old_for_bench ( const void * s, int iLen, uint64_t uPrev=SPH_FNV64_SEED );

FORCE_INLINE uint64_t sphFNV64_old_uint64 ( uint64_t uVal, uint64_t uPrev = SPH_FNV64_SEED )
{
	const uint64_t FNV_64_PRIME = 0x100000001b3ULL;
	const BYTE * p = (const BYTE*)&uVal;
	uint64_t hval = uPrev;

	hval ^= (uint64_t)*(p+0);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+1);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+2);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+3);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+4);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+5);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+6);
	hval *= FNV_64_PRIME;
	hval ^= (uint64_t)*(p+7);
	hval *= FNV_64_PRIME;

	return hval;
}

static uint64_t newHash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED )
{
	assert ( pStr && iLen );

	uint64_t uAcc = uPrev;
	while ( iLen-- )
	{
		int iChar = tolower ( *pStr++ );
		uAcc = sphFNV64_4x ( iChar, uAcc );
	}

	return uAcc;
}

static uint64_t oldHash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED )
{
	assert ( pStr && iLen );

	uint64_t uAcc = uPrev;
	while ( iLen-- )
	{
		int iChar = tolower ( *pStr++ );
		uAcc = sphFNV64 ( &iChar, 4, uAcc );
	}

	return uAcc;
}

static uint64_t oldHash_inlined ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED )
{
	assert ( pStr && iLen );

	uint64_t uAcc = uPrev;
	while ( iLen-- )
	{
		int iChar = tolower ( *pStr++ );
		uAcc = sphFNV64 (iChar, uAcc);
	}

	return uAcc;
}

BENCHMARK_F ( FNV_hasher, caseless_default )
( benchmark::State& st )
{
	int iBytes = 0;
	for ( auto _ : st )
	{
		oldHash ( (const BYTE*)sBuf, iLen );
		st.PauseTiming();
		iBytes += iLen;
		st.ResumeTiming();
	}
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_F ( FNV_hasher, caseless_new )
( benchmark::State& st )
{
	int iBytes = 0;
	for ( auto _ : st )
	{
		newHash ( (const BYTE*)sBuf, iLen );
		st.PauseTiming();
		iBytes += iLen;
		st.ResumeTiming();
	}
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_F ( FNV_hasher, caseless_inlined )
( benchmark::State& st )
{
	int iBytes = 0;
	for ( auto _ : st )
	{
		oldHash_inlined ( (const BYTE*)sBuf, iLen );
		st.PauseTiming();
		iBytes += iLen;
		st.ResumeTiming();
	}
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_F ( FNV_hasher, old_flavour_cpp )
( benchmark::State& st )
{
	int iBytes = 0;
	for ( auto _ : st )
	{
		sphFNV64_old_for_bench ( (const BYTE*)sBuf, iLen );
		st.PauseTiming();
		iBytes += iLen;
		st.ResumeTiming();
	}
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_F ( FNV_hasher, new_flavour_cpp )
( benchmark::State& st )
{
	int iBytes = 0;
	for ( auto _ : st )
	{
		sphFNV64 ( (const BYTE*)sBuf, iLen );
		st.PauseTiming();
		iBytes += iLen;
		st.ResumeTiming();
	}
	st.SetBytesProcessed ( iBytes );
}


BENCHMARK_F ( FNV_hasher, old_flavour_from_header )
( benchmark::State& st )
{
	int iBytes = 0;
	const auto iSize64c = iLen/sizeof(uint64_t);
	uint64_t uPrev = SPH_FNV64_SEED;
	for ( auto _ : st )
	{
		auto iSize64 = iSize64c;
		auto sBuf64 = (const uint64_t*)sBuf;
		while (iSize64--) {
			uPrev = sphFNV64_old_uint64 ( *sBuf64, uPrev );
			st.PauseTiming();
			iBytes += 8;
			++sBuf64;
			st.ResumeTiming();
		}
	}
	st.SetBytesProcessed ( iBytes );
}

BENCHMARK_F ( FNV_hasher, new_flavour_from_header )
( benchmark::State& st )
{
	int iBytes = 0;
	const auto iSize64c = iLen/sizeof(uint64_t);
	uint64_t uPrev = SPH_FNV64_SEED;
	for ( auto _ : st )
	{
		auto iSize64 = iSize64c;
		auto sBuf64 = (const uint64_t*)sBuf;
		while (iSize64--) {
			uPrev = sphFNV64 ( *sBuf64, uPrev );
			st.PauseTiming();
			iBytes += 8;
			++sBuf64;
			st.ResumeTiming();
		}
	}
	st.SetBytesProcessed ( iBytes );
}