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

#include "std/stringbuilder.h"
#include "std/string.h"

#include <benchmark/benchmark.h>

static void builder_Appendf ( benchmark::State& st )
{
	auto NUM_TRIES = st.range ( 0 );
	StringBuilder_c sRes;
	for ( auto _ : st )
		for ( int i = 0; i < NUM_TRIES; ++i )
			sRes.Appendf ( "%d ", i );
	st.SetLabel ( "Use Appendf to builder" );
}

static void builder_Sprintf ( benchmark::State& st )
{
	auto NUM_TRIES = st.range ( 0 );
	StringBuilder_c sRes;
	for ( auto _ : st )
		for ( int i = 0; i < NUM_TRIES; ++i )
			sRes.Sprintf ( "%d ", i );
	st.SetLabel("Use sprintf to builder");
}

static void string_Sprintf ( benchmark::State& st )
{
	auto NUM_TRIES = st.range ( 0 );
	CSphString sRes;
	sRes.SetSprintf ( "%s", "0" );
	for ( auto _ : st )
		for ( int i = 0; i < NUM_TRIES; ++i )
			sRes.SetSprintf ( "%s %d ", sRes.cstr(), i );
	st.SetLabel ( "Use SetSprintf to string" );
}

BENCHMARK ( builder_Appendf )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK ( builder_Sprintf )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK ( string_Sprintf )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
