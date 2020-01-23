//
// Copyright (c) 2020, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>

#include "threadutils.h"
#include <atomic>

void SetStderrLogger ();

TEST ( ThreadPool, Empty )
{
	Threads::ThreadPool_c tPool ( 4, "tp" );
	tPool.Wait();
}

TEST ( ThreadPool, EmptyWait )
{
//	SetStderrLogger ();
	Threads::ThreadPool_c tPool ( 4, "tp" );
	tPool.Wait ();
	tPool.Wait ();
	tPool.Wait ();
}

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
	Threads::ThreadPool_c tPool ( 4, "tp" );
	std::atomic<int> v {0};
	for ( int i=0; i<100; ++i)
		tPool.Schedule ([&] { ++v; });
	tPool.Wait ();
	ASSERT_EQ ( v, 100 );
}