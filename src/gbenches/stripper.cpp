//
// Copyright (c) 2022-2025, Manticore Software LTD (https://manticoresearch.com)
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
	st.SetLabel ( "Without tags 'style' and 'scrypt'" );
	st.SetBytesProcessed ( iBytes );
}
