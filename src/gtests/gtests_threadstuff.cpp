//
// Copyright (c) 2019-2021, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>

#include "threadutils.h"
#include "coroutine.h"
#include <atomic>

void SetStderrLogger ();


// here intentionally added initialized lambda capture
// it will always show compiler warning on ancient compilers
TEST ( ThreadPool, movelambda )
{
	CSphString test = "hello";
	CSphString to;
	const char* sOrigHello = test.cstr();
	auto x = [&, line = std::move ( test )] () mutable { to = std::move(line); };
	ASSERT_STREQ ( test.cstr (), NULL ) << "Line is already captured";
	ASSERT_STREQ ( to.cstr (), NULL ) << "line is not yet finally moved";
	x ();
	ASSERT_STREQ ( to.cstr (), "hello" ) << "lambda moved captured line to here";
	ASSERT_EQ ( sOrigHello, to.cstr() ) << "moved value is exactly one from the start";
}

TEST ( ThreadPool, Counter100 )
{
//	SetStderrLogger ();
	auto pPool = Threads::MakeThreadPool ( 4, "tp" );
	auto & tPool = *pPool;
	std::atomic<int> v {0};
	for ( int i=0; i<100; ++i)
		tPool.Schedule ([&] { ++v; }, false);
	tPool.StopAll ();
	ASSERT_EQ ( v, 100 );
}

TEST ( ThreadPool, Counter100c )
{
	SetMaxChildrenThreads ( 300 );
	StartGlobalWorkPool();
	int v = 0;
	for ( int i = 0; i<100000; ++i )
		Threads::CallCoroutine ( [&] { ++v; } );
	ASSERT_EQ ( v, 100000 );
}
