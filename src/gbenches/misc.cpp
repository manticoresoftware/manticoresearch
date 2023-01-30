//
// Copyright (c) 2022-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "std/string.h"
#include "std/rand.h"
#include "std/timers.h"
#include "threadutils.h"

#include <benchmark/benchmark.h>

class Misc: public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State& state )
	{
		sphSrand ( 0 );
		iRes = 0;
	}

	void TearDown ( const ::benchmark::State& state )
	{
	}

	static void BenchTimerThread ( void* pEndtime )
	{
		volatile int iRes = 0;
		for ( int i = 0; i < 100; ++i )
			iRes += (int)sphMicroTimer();
		*(int64_t*)pEndtime = sphMicroTimer();
	}

	volatile int iRes;
	int64_t t;
	int64_t tmStart;
};

BENCHMARK_DEFINE_F ( Misc, rand )
( benchmark::State& st )
{
	auto NRUNS = st.range(0);
	for ( auto _ : st )
		for ( int i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( sphRand() );
	st.SetItemsProcessed ( st.iterations() * NRUNS );
}

BENCHMARK_DEFINE_F ( Misc, randlog2 )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( int i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( sphLog2 ( sphRand() ) );
	st.SetItemsProcessed ( st.iterations() * NRUNS );
}

BENCHMARK_DEFINE_F ( Misc, timer )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( int i = 0; i < NRUNS; ++i )
			benchmark::DoNotOptimize ( sphMicroTimer() );
	st.SetItemsProcessed ( st.iterations() * NRUNS );
}

BENCHMARK_DEFINE_F ( Misc, timer_threaded )
( benchmark::State& st )
{
	Threads::Init();
	tmStart = sphMicroTimer();
	auto THREADS = st.range ( 0 );
	CSphFixedVector<SphThread_t> dThd { THREADS };
	CSphFixedVector<int64_t> tmEnd { THREADS };
	for ( auto _ : st )
	{
		for ( int i = 0; i < THREADS; ++i )
			Threads::Create ( &dThd[i], [this, i, THREADS, &tmEnd] { BenchTimerThread ( &tmEnd[i] ); } );
		for ( int i = 0; i < THREADS; ++i )
			if ( !Threads::Join ( &dThd[i] ) )
				sphDie ( "thread_join failed" );
	}
	st.SetItemsProcessed ( st.iterations()*THREADS );
}

BENCHMARK_REGISTER_F ( Misc, rand )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Misc, randlog2 )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Misc, timer )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Misc, timer_threaded )->RangeMultiplier ( 2 )->Range ( 1, 16 );
