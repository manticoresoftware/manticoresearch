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

#include <gtest/gtest.h>

#include "std/ints.h"
#include "std/num_conv.h"
#include "sphinxutils.h"
#include "sphinxjson.h"
#include "aggrexpr.h"

TEST ( functions, NtoA )
{
	using namespace sph;

	char sBuf[50];
	memset ( sBuf, 255, 50 );

	int iLen = NtoA ( sBuf, (DWORD)50 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "50", sBuf );

	iLen = NtoA ( sBuf, 50, 10, 0, 4 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "0050", sBuf );

	iLen = NtoA ( sBuf, 50, 10, 4 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "  50", sBuf );

	iLen = NtoA ( sBuf, 50, 10, 6, 3 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "   050", sBuf );

	iLen = NtoA ( sBuf, 50, 10, 6, 3, '_' );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "___050", sBuf );

	iLen = NtoA<int64_t> ( sBuf, 0xFFFFFFFFFFFFFFFFll );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-1", sBuf );

	iLen = NtoA<int64_t> ( sBuf, 0x8000000000000000ll );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-9223372036854775808", sBuf );

	iLen = NtoA ( sBuf, 0x7FFFFFFFFFFFFFFFll );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "9223372036854775807", sBuf );

	iLen = NtoA ( sBuf, -9223372036854775807 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-9223372036854775807", sBuf );

	sBuf[NtoA ( sBuf, -9223372036854775807 )] = '\0';
	ASSERT_STREQ ( "-9223372036854775807", sBuf );

	iLen = NtoA ( sBuf, 9223372036854775807 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "9223372036854775807", sBuf );

	iLen = NtoA<int64_t> ( sBuf, 0xFFFFFFFFFFFFFFFFll, 16 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-1", sBuf );

	iLen = NtoA<int64_t> ( sBuf, 0x8000000000000000ll, 16 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-8000000000000000", sBuf );
}

TEST ( functions, IFtoA )
{
	using namespace sph;

	char sBuf[50];
	memset ( sBuf, 255, 50 );

	int iLen = IFtoA ( sBuf, 50000, 3 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "50.000", sBuf );

	iLen = IFtoA ( sBuf, -50000, 3 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-50.000", sBuf );

	iLen = IFtoA ( sBuf, -1, 3 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-0.001", sBuf );

	iLen = IFtoA ( sBuf, 1, 3 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "0.001", sBuf );
}


void test_mysprintf ( const char* sFmt, int64_t iNum, const char* sResult )
{
	using namespace sph;
	char sBuf[50];
	memset ( sBuf, 255, 50 );
	sph::Sprintf ( sBuf, sFmt, iNum );
	ASSERT_STREQ ( sBuf, sResult ) << " (on fmt " << sFmt << ")";
}

void test_sprintf ( const char* sFmt, int64_t iNum )
{
	char sBuf[50];
	snprintf ( sBuf, 49, sFmt, iNum );
	test_mysprintf ( sFmt, iNum, sBuf );
}

void test_sphintf_for ( int64_t iNum )
{
	test_sprintf ( "%d", iNum );
	test_sprintf ( "%0d", iNum );
	test_sprintf ( "%4d", iNum );
	test_sprintf ( "%04d", iNum );
	test_sprintf ( "%.4d", iNum );
	test_sprintf ( "%0.4d", iNum );
	test_sprintf ( "%9.3d", iNum );
	test_sprintf ( "%09.3d", iNum );
}

TEST ( functions, sph_Sprintf )
{
	test_sphintf_for ( 0 );
	test_sphintf_for ( 50 );
	test_sphintf_for ( -50 );
	test_sphintf_for ( 10000 );
	test_sphintf_for ( -10000 );

	int iNum = -10000;
	test_mysprintf ( "%l", iNum, "-10000" ); // %l is our specific for 64-bit signed
	test_mysprintf ( "%0l", iNum, "-10000" );
	test_mysprintf ( "%4l", iNum, "-10000" );
	test_mysprintf ( "%04l", iNum, "-10000" );
	test_mysprintf ( "%.4l", iNum, "-10000" );
	test_mysprintf ( "%0.4l", iNum, "-10000" );
	test_mysprintf ( "%9.3l", iNum, "   -10000" );
	test_mysprintf ( "%09.3l", iNum, "   -10000" );


	test_mysprintf ( "%l", -100000000000000, "-100000000000000" ); // %l is our specific for 64-bit signed

	// my own fixed-point nums
	test_mysprintf ( "%.3D", iNum, "-10.000" );
	test_mysprintf ( "%.9D", iNum, "-0.000010000" );

	test_mysprintf ( "%.3F", iNum, "-10.000" );
	test_mysprintf ( "%.5F", iNum, "-0.10000" );

	iNum = 10000;
	test_mysprintf ( "%U", iNum, "10000" ); // %U is our specific for 64-bit signed
	test_mysprintf ( "%0U", iNum, "10000" );
	test_mysprintf ( "%4U", iNum, "10000" );
	test_mysprintf ( "%04U", iNum, "10000" );
	test_mysprintf ( "%.4U", iNum, "10000" );
	test_mysprintf ( "%0.4U", iNum, "10000" );
	test_mysprintf ( "%9.3U", iNum, "    10000" );
	test_mysprintf ( "%09.3U", iNum, "    10000" );

	// fallback to stardard %f
	using namespace sph;
	char sBuf[50];
	memset ( sBuf, 255, 50 );
	sph::Sprintf ( sBuf, "%03.2f", 99.9911 );
	ASSERT_STREQ ( sBuf, "99.99" );

	// strings output
	sph::Sprintf ( sBuf, "%s", "hello" );
	ASSERT_STREQ ( sBuf, "hello" );
	sph::Sprintf ( sBuf, "%-s", "hello" );
	ASSERT_STREQ ( sBuf, "hello" );
	sph::Sprintf ( sBuf, "%10s", "hello" );
	ASSERT_STREQ ( sBuf, "     hello" );
	sph::Sprintf ( sBuf, "%-10s", "hello" );
	ASSERT_STREQ ( sBuf, "hello     " );
	sph::Sprintf ( sBuf, "%-10.3s", "hello" );
	ASSERT_STREQ ( sBuf, "hel       " );
	sph::Sprintf ( sBuf, "%10.3s", "hello" );
	ASSERT_STREQ ( sBuf, "       hel" );

	sph::Sprintf ( sBuf, "Hello %l, %d world!", -100000000000000, -2000000000 );
	ASSERT_STREQ ( sBuf, "Hello -100000000000000, -2000000000 world!" );

	// std 64 fmt modifiers
	iNum = 10000;
	test_mysprintf ( "test " INT64_FMT, iNum, "test 10000" );
	test_mysprintf ( "test " UINT64_FMT, iNum, "test 10000" );
}

// sph::Sprintf into StringBuilder_c
TEST ( functions, sph_Sprintf_to_builder )
{
	using namespace sph;
	StringBuilder_c sBuf;
	Sprintf ( sBuf, "%-10s", "hello" );
	EXPECT_STREQ ( sBuf.cstr(), "hello     " );
	sBuf.Clear();

	Sprintf ( sBuf, "%03.2f", 99.9911 );
	EXPECT_STREQ ( sBuf.cstr(), "99.99" );
	sBuf.Clear();

	sph::Sprintf ( sBuf, "Hello %d, %l world!", -2000000000, -100000000000000 );
	ASSERT_STREQ ( sBuf.cstr(), "Hello -2000000000, -100000000000000 world!" );

	sph::Sprintf ( sBuf, "Hi!" );
	ASSERT_STREQ ( sBuf.cstr(), "Hello -2000000000, -100000000000000 world!Hi!" );
	sBuf.Clear();

	Sprintf ( sBuf, "%09.3d", -10000 );
	EXPECT_STREQ ( sBuf.cstr(), "   -10000" );
	sBuf.Clear();

	Sprintf ( sBuf, "%.3D", (int64_t)-10000 );
	EXPECT_STREQ ( sBuf.cstr(), "-10.000" );
	sBuf.Clear();

	Sprintf ( sBuf, "%.9D", -10000ll );
	ASSERT_STREQ ( sBuf.cstr(), "-0.000010000" );
	sBuf.Clear();

	sBuf.StartBlock ( dJsonObj );
	sBuf.Sprintf ( "%d %d %d", 1, -1, 100 );
	sBuf.Sprintf ( "%d %d %d", 2, -2, 200 );
	sBuf.FinishBlock();
	ASSERT_STREQ ( sBuf.cstr(), "{1 -1 100,2 -2 200}" );

	sBuf.Sprintf ( " %.3F, %.6F", 999500, -1400932 );
	ASSERT_STREQ ( sBuf.cstr(), "{1 -1 100,2 -2 200} 999.500, -1.400932" );

	sBuf.Sprintf ( " %.3F", 999005 );
	ASSERT_STREQ ( sBuf.cstr(), "{1 -1 100,2 -2 200} 999.500, -1.400932 999.005" );
}

TEST ( functions, builder_sprintf_formatters )
{
	using namespace sph;
	StringBuilder_c sBuf;

	// %s - string
	Sprintf ( sBuf, "%s", "hello" );
	EXPECT_STREQ ( sBuf.cstr(), "hello" );

	// %p - pointer
	sBuf.Clear();
	sBuf.Sprintf ( "%p", nullptr );
	EXPECT_STREQ ( sBuf.cstr(), "0" );

	// %x - hex unsigned integer
	sBuf.Clear();
	sBuf.Sprintf ( "%x", 0xFFFF );
	EXPECT_STREQ ( sBuf.cstr(), "ffff" );

	// %u - unsigned integer
	sBuf.Clear();
	sBuf.Sprintf ( "%u", 0xFFFF );
	EXPECT_STREQ ( sBuf.cstr(), "65535" );

	// %d - decimal integer
	sBuf.Clear();
	sBuf.Sprintf ( "%d", -2 );
	EXPECT_STREQ ( sBuf.cstr(), "-2" );

	// %i - ignore value
	sBuf.Clear();
	sBuf.Sprintf ( "hello%i world", -2 );
	EXPECT_STREQ ( sBuf.cstr(), "hello world" );

	// %l - decimal int64
	sBuf.Clear();
	sBuf.Sprintf ( "%l", (int64_t)-1 );
	EXPECT_STREQ ( sBuf.cstr(), "-1" );

	// %U - decimal uint64
	sBuf.Clear();
	sBuf.Sprintf ( "%U", (uint64_t)4294967295UL );
	EXPECT_STREQ ( sBuf.cstr(), "4294967295" );

	// %D - fixed-point signed 64 bit
	sBuf.Clear();
	Sprintf ( sBuf, "%.3D", (int64_t)-10000 );
	EXPECT_STREQ ( sBuf.cstr(), "-10.000" );

	// %F - fixed-point signed 32 bit
	sBuf.Clear();
	sBuf.Sprintf ( "%.3F", 999005 );
	EXPECT_STREQ ( sBuf.cstr(), "999.005" );

	// %t - timespan in int64 useconds
	sBuf.Clear();
	sBuf.Sprintf ( "%t", (int64_t)1000065 );
	EXPECT_STREQ ( sBuf.cstr(), "1s" );

	sBuf.Clear();
	sBuf.Sprintf ( "char %c is here", 'a' );
	EXPECT_STREQ ( sBuf.cstr(), "char a is here" );
}

TEST ( functions, sph_Sprintf_regression_on_empty_buf )
{
	StringBuilder_c sBuf;
	sBuf.Sprintf ( "%.3F", 10 );
	ASSERT_STREQ ( sBuf.cstr(), "0.010" );
}

TEST ( functions, sph_Sprintf_inttimespans )
{
	StringBuilder_c sBuf;

	static const struct
	{
		int64_t tm;
		const char* res;
	} models[] =
		{
			{ 4, "4us" },
			{ 5000, "5ms" },
			{ 6000000, "6s" },
			{ 120000000, "2m" },
			{ 3600000000, "1h" },
			{ 3600000000ULL * 24 * 2, "2d" },
			{ 3600000000ULL * 24 * 7 * 2, "2w" },
		};

	for ( const auto& model : models )
	{
		sBuf.Sprintf ( "%t", model.tm );
		EXPECT_STREQ ( sBuf.cstr(), model.res ) << "for " << model.tm << " with %t";
		sBuf.Clear();
	}
}

TEST ( functions, sph_Sprintf_fractimespans_round )
{
	StringBuilder_c sBuf;
	static const struct
	{
		int64_t tm;
		const char* fmt;
		const char* res;
	} models[] =
		{
			// us rounding
			{ 999, "%t", "999us" },
			{ 999, "%.1t", "999us" },
			{ 999, "%.2t", "999us" },
			{ 999, "%.3t", "999us" },

			// ms rounding
			{ 1559, "%t", "2ms" },
			{ 1559, "%.1t", "1.6ms" },
			{ 1559, "%.2t", "1.56ms" },
			{ 1559, "%.3t", "1ms 559us" },

			// s rounding
			{ 1555555, "%t", "2s" },
			{ 1555555, "%.1t", "1.6s" },
			{ 1555555, "%.2t", "1.56s" },
			{ 1555555, "%.3t", "1s 556ms" },
			{ 1555555, "%.4t", "1s 555.6ms" },
			{ 1555555, "%.5t", "1s 555.56ms" },
			{ 1999995, "%.5t", "2s" },
			{ 1555555, "%.6t", "1s 555ms 555us" },

			// m rounding
			{ 71555555, "%t", "1m" },
			{ 71555555, "%.1t", "1.2m" },
			{ 71555555, "%.2t", "1m 12s" },
			{ 71555555, "%.3t", "1m 11.6s" },
			{ 71555555, "%.4t", "1m 11.56s" },
			{ 71555555, "%.5t", "1m 11s 556ms" },
			{ 71555555, "%.6t", "1m 11s 555.6ms" },
			{ 71555555, "%.7t", "1m 11s 555.56ms" },
			{ 71555555, "%.8t", "1m 11s 555ms 555us" },
			{ 89999999, "%.7t", "1m 30s" },
			{ 89999994, "%.7t", "1m 29s 999.99ms" },
			{ 89999995, "%.7t", "1m 30s" },
			{ 90999999, "%.7t", "1m 31s" },
		};

	for ( const auto& model : models )
	{
		sBuf.Sprintf ( model.fmt, model.tm );
		EXPECT_STREQ ( sBuf.cstr(), model.res ) << "for " << model.tm << " with " << model.fmt;
		sBuf.Clear();
	}
}

TEST ( functions, sph_Sprintf_fractimezero )
{
	StringBuilder_c sBuf;

	sBuf.Sprintf ( "%t", 0ULL );
	ASSERT_STREQ ( sBuf.cstr(), "0us" );
	sBuf.Clear();

	sBuf.Sprintf ( "%.3t", 0ULL );
	ASSERT_STREQ ( sBuf.cstr(), "0us" );
	sBuf.Clear();
}

TEST ( functions, date_parse )
{
	int64_t iDate = 0;
	StringBuilder_c sBuf;

	sBuf.Clear();
	iDate = GetUTC ( "2019-03" );
	FormatDate ( iDate, sBuf );
	EXPECT_STREQ ( sBuf.cstr(), "2019-03-01T00:00:00" );

	sBuf.Clear();
	iDate = GetUTC ( "2019-03-23" );
	FormatDate ( iDate, sBuf );
	EXPECT_STREQ ( sBuf.cstr(), "2019-03-23T00:00:00" );

	sBuf.Clear();
	iDate = GetUTC ( "2019-03-23T21" );
	FormatDate ( iDate, sBuf );
	EXPECT_STREQ ( sBuf.cstr(), "2019-03-23T21:00:00" );

	sBuf.Clear();
	iDate = GetUTC ( "2019-03-23T21:34" );
	FormatDate ( iDate, sBuf );
	EXPECT_STREQ ( sBuf.cstr(), "2019-03-23T21:34:00" );

	sBuf.Clear();
	iDate = GetUTC ( "2019-03-23T21:34:46" );
	FormatDate ( iDate, sBuf );
	EXPECT_STREQ ( sBuf.cstr(), "2019-03-23T21:34:46" );

	sBuf.Clear();
	iDate = GetUTC ( "2019-03-23T21:34:46.1234567" );
	FormatDate ( iDate, sBuf );
	EXPECT_STREQ ( sBuf.cstr(), "2019-03-23T21:34:46" );

	sBuf.Clear();
	iDate = GetUTC ( "2019-03-23T21:34:46-04:00" );
	FormatDate ( iDate, sBuf );
	EXPECT_STREQ ( sBuf.cstr(), "2019-03-23T21:34:46" );

	sBuf.Clear();
	iDate = GetUTC ( "2019-03-23T21:34:46.123-04:00" );
	FormatDate ( iDate, sBuf );
	EXPECT_STREQ ( sBuf.cstr(), "2019-03-23T21:34:46" );

	sBuf.Clear();
	iDate = GetUTC ( "1553371205" );
	FormatDate ( iDate, sBuf );
	EXPECT_STREQ ( sBuf.cstr(), "2019-03-23T20:00:05" );
}