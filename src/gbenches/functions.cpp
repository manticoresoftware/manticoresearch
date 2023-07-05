//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <benchmark/benchmark.h>

#include "sphinxutils.h"
#include <cmath>
#include "histogram.h"

// Miscelaneous short functional tests: TDigest, SpanSearch,
// stringbuilder, CJson, TaggedHash, Log2

// simplest way to test searchd internals - include the source, supress main() function there.
#define SUPRESS_SEARCHD_MAIN 1

#include "searchd.cpp"

//////////////////////////////////////////////////////////////////////////



// benches for EscapeJsonString_t
inline static bool IsEscapeChar1 ( char c )
{
	return memchr ( "\"\\\b\f\n\r\t", c, 8 )!=nullptr; // \ is \x5C, " is \x22
}

inline static bool IsEscapeChar2 ( char c )
{
	return strchr ( "\"\\\b\f\n\r\t", c )!=nullptr; // \ is \x5C, " is \x22
}

inline static bool IsEscapeChar3 ( char c )
{
	switch ( c )
	{
	case '\b': case '\f': case '\n': case '\r':	case '\t': case '\"': case '\\' : return true;
	default: return false;
	}
}

inline static bool IsEscapeChar4 ( BYTE c ) // winner!
{
	alignas ( 128 ) static const bool lookup[] =
				   {0,0,0,0,0,0,0,0, 1,1,1,0,1,1,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
					0,0,1,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
					0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,1,0,0,0,
					0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
					};
	return ( c & 0x80 ) ? false : lookup[c];
}


inline static BYTE GetEscapedChar1 ( BYTE c )
{
	switch ( c )
	{
	case '\b': return 'b'; // \x08
	case '\t': return 't'; // \x09
	case '\n': return 'n'; // \x0A
	case '\f': return 'f'; // \x0C
	case '\r': return 'r'; // \x0D

	default: return c;
	}
}

inline static BYTE GetEscapedChar2 ( BYTE c ) // winner!
{
	alignas ( 16 ) static const BYTE dTransform[16] = {'\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07',
			'b', 't', 'n', '\x0B', 'f', 'r', '\x0E', '\x0F' };
	//return dTransform[(BYTE) c];
	return ( c & 0xF0 ) ? c : dTransform[c];
}

alignas ( 128 ) static const BYTE g_Transform[] =
				   {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, 'b'|0x80, 't'|0x80, 'n'|0x80, 0x0b, 'f'|0x80, 'r'|0x80, 0x0e, 0x0f,
					0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
					0x20,0x21,'\"'|0x80,0x23,0x24,0x25,0x26,0x27, 0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
					0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, 0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
					0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47, 0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
					0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57, 0x58,0x59,0x5a,0x5b,'\\'|0x80,0x5d,0x5e,0x5f,
					0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
					0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
					};

inline static bool IsEscapeChar5 ( BYTE c )
{
	return ( c & 0x80 ) ? false : g_Transform[c] & 0x80;
}

inline static char GetEscapedChar3 ( BYTE c )
{
	return ( c & 0x80 ) ? c : g_Transform[c]&0x7F;
}

// here IsEscapeChar5 already excluded bytes with high bit set, so even simpler!
inline static char GetEscapedChar3combo ( BYTE c )
{
	return g_Transform[c] & 0x7F;
}

class Escaping : public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State & state )
	{
		dChars.Resize ( 128 );
		for ( char & c: dChars )
			c = sphRand () & 0xFF;
	}

//	void TearDown ( const ::benchmark::State & state ) { }

	CSphVector<char> dChars;
	bool bRes = false;
	int i = 0;
};

BENCHMARK_DEFINE_F ( Escaping, empty_pass )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			bRes |= !!dChars[++i & 0x7F];
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_DEFINE_F ( Escaping, memchr )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			bRes |= IsEscapeChar1 ( dChars[++i & 0x7F] );
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_DEFINE_F ( Escaping, strchr )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			bRes |= IsEscapeChar2 ( dChars[++i & 0x7F] );
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_DEFINE_F ( Escaping, _switch )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			bRes |= IsEscapeChar3 ( dChars[++i & 0x7F] );
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_DEFINE_F ( Escaping, booltable )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			bRes |= IsEscapeChar4 ( dChars[++i & 0x7F] );
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_DEFINE_F ( Escaping, bytetable )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			bRes |= IsEscapeChar5 ( dChars[++i & 0x7F] );
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_DEFINE_F ( Escaping, transform_switch )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			bRes |= GetEscapedChar1 ( dChars[++i & 0x7F] );
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_DEFINE_F ( Escaping, transform_narrow_table )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			bRes |= GetEscapedChar2 ( dChars[++i & 0x7F] );
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_DEFINE_F ( Escaping, transform_wide_table )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			bRes |= GetEscapedChar3 ( dChars[++i & 0x7F] );
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_DEFINE_F ( Escaping, strchr_transform_switch )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			if ( IsEscapeChar2 ( dChars[++i & 0x7F] ) ) // combo usual
				bRes |= !!GetEscapedChar1 ( dChars[i & 0x7F] );
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_DEFINE_F ( Escaping, and_transform_by_one_table )
( benchmark::State& st )
{
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( auto i = 0; i < NRUNS; ++i )
			if ( IsEscapeChar5 ( dChars[++i & 0x7F] ) ) // combo usual
				bRes |= !!GetEscapedChar3combo ( dChars[i & 0x7F] );
	st.SetBytesProcessed ( NRUNS * st.iterations() );
}

BENCHMARK_REGISTER_F ( Escaping, empty_pass )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Escaping, memchr )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Escaping, strchr )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Escaping, _switch )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Escaping, booltable )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Escaping, bytetable )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Escaping, transform_switch )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Escaping, transform_narrow_table )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Escaping, transform_wide_table )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Escaping, strchr_transform_switch )->RangeMultiplier ( 10 )->Range ( 10, 1000 );
BENCHMARK_REGISTER_F ( Escaping, and_transform_by_one_table )->RangeMultiplier ( 10 )->Range ( 10, 1000 );


static void BM_sphSprintf ( benchmark::State & st )
{
	char sBuf[40];
	for ( auto _ : st )
		sph::Sprintf ( sBuf, "%d", 1000000 );
	st.SetItemsProcessed ( st.iterations() );
}

static void BM_stdSprintf ( benchmark::State & st )
{
	char sBuf[40];
	for ( auto _ : st )
		snprintf ( sBuf, 39, "%d", 1000000 );
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK( BM_sphSprintf );
BENCHMARK( BM_stdSprintf );

class bench_builder : public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State& state )
	{
		sphSrand(0);
		a = sphRand();
		b = sphRand();
		c = sphRand();
		d = sphRand();
		f = sphRand();
		g = sphRand();
		h = sphRand();
		i = sphRand();
		j = sphRand();
		k = sphRand();
		l = sphRand();
		m = sphRand();
		n = sphRand();
		o = sphRand();
		p = sphRand();
		q = sphRand();
	}

	const char * sFieldFmt = R"({"field":%d, "lcs":%u, "hit_count":%u, "word_count":%u, "tf_idf":%d, "min_idf":%d, )"
							 R"("max_idf":%d, "sum_idf":%d, "min_hit_pos":%d, "min_best_span_pos":%d, "exact_hit":%u, )"
							 R"("max_window_hits":%d, "min_gaps":%d, "exact_order":%u, "lccs":%d, "wlccs":%d, "atc":%d})";

	int a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q;

};

BENCHMARK_F( bench_builder, Appendf_ints ) ( benchmark::State & st )
{
	StringBuilder_c sBuf;

	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( sBuf.Appendf ( sFieldFmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q ) );
		sBuf.Rewind ();
	}
	st.SetLabel ( "use standard sprintf() inside" );
	st.SetItemsProcessed ( st.iterations() );
}


BENCHMARK_F( bench_builder, Sprintf_ints ) ( benchmark::State & st )
{
	StringBuilder_c sBuf;

	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( sBuf.Sprintf ( sFieldFmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q ) );
		sBuf.Rewind ();
	}
	st.SetLabel ( "own implementation sph::Sprintf" );
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK_F ( bench_builder, Sprint_ints )
( benchmark::State& st )
{
	StringBuilder_c sBuf;
	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( sBuf.Sprint( "{\"field\":", a, ", \"lcs\":", b, ", \"hit_count\":", c, ", \"word_count\":", d, ", \"tf_idf\":", e, ", \"min_idf\":", f,
					  ", \"max_idf\":", g, ", \"sum_idf\":", h, ", \"min_hit_pos\":", i, ", \"min_best_span_pos\":", j, ", \"exact_hit\":", k,
					  ", \"max_window_hits\":", l, ", \"min_gaps\":", m, ", \"exact_order\":", n, ", \"lccs\":", o, ", \"wlccs\":", p, ", \"atc\":", q, '}') );
		sBuf.Rewind();
	}
	st.SetLabel ( "own implementation using templated Sprint" );
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK_F ( bench_builder, Sprint_streaming )
( benchmark::State& st )
{
	StringBuilder_c sBuf;

	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( sBuf << "{\"field\":" << a << ", \"lcs\":" << b << ", \"hit_count\":" << c << ", \"word_count\":" << d << ", \"tf_idf\":" << e
		<< ", \"min_idf\":" << f << ", \"max_idf\":" << g << ", \"sum_idf\":" << h << ", \"min_hit_pos\":" << i << ", \"min_best_span_pos\":" << j
		<< ", \"exact_hit\":" << k << ", \"max_window_hits\":" << l << ", \"min_gaps\":" << m << ", \"exact_order\":" << n << ", \"lccs\":" << o
		<< ", \"wlccs\":" << p << ", \"atc\":" << q << '}' );
		sBuf.Rewind();
	}
	st.SetLabel ( "own implementation using << (mostly inlined)" );
	st.SetItemsProcessed ( st.iterations() );
}

class rand_numbers: public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State& state )
	{
		dNumbers.Resize(1000);
		sphSrand ( 0 );
		for ( auto& iNum: dNumbers )
		{
			int64_t iVal = sphRand();
			iVal = ( iVal << 32 ) | sphRand();
			iNum = iVal;
		}

	}
	CSphVector<int64_t> dNumbers;
	char sBuf[32];
};

#if __has_include( <charconv>)
#include <charconv>
#endif
template<typename T>
inline static char* FormatInt ( char sBuf[32], T v )
{
	if ( sizeof ( T ) == 4 && v == INT_MIN )
		return strncpy ( sBuf, "-2147483648", 32 );
	if ( sizeof ( T ) == 8 && v == LLONG_MIN )
		return strncpy ( sBuf, "-9223372036854775808", 32 );

	bool s = ( v < 0 );
	if ( s )
		v = -v;

	char* p = sBuf + 31;
	*p = 0;
	do
	{
		*--p = '0' + char ( v % 10 );
		v /= 10;
	} while ( v );
	if ( s )
		*--p = '-';
	return p;
}

BENCHMARK_F ( rand_numbers, formatint )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( const auto dNum : dNumbers )
			benchmark::DoNotOptimize ( FormatInt<int> ( sBuf, (int)dNum ) );
	}
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK_F ( rand_numbers, ntoaint )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( const auto dNum : dNumbers )
			benchmark::DoNotOptimize ( sph::NtoA ( sBuf, (int)dNum ) );
	}
	st.SetItemsProcessed ( st.iterations() );
}

#if __has_include( <charconv>)
BENCHMARK_F ( rand_numbers, std_tocharsint )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( const auto dNum : dNumbers )
			benchmark::DoNotOptimize ( std::to_chars ( sBuf, sBuf + 32, (int)dNum ) );
	}
	st.SetItemsProcessed ( st.iterations() );
}
#endif

BENCHMARK_F ( rand_numbers, formatint64 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( const auto dNum : dNumbers )
			benchmark::DoNotOptimize ( FormatInt<int64_t> ( sBuf, dNum ) );
	}
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK_F ( rand_numbers, ntoaint64 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( const auto dNum : dNumbers )
			benchmark::DoNotOptimize ( sph::NtoA ( sBuf, dNum ) );
	}
	st.SetItemsProcessed ( st.iterations() );
}

#if __has_include( <charconv>)
BENCHMARK_F ( rand_numbers, std_tocharsint64 )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		for ( const auto dNum : dNumbers )
			benchmark::DoNotOptimize ( std::to_chars ( sBuf, sBuf + 32, dNum ) );
	}
	st.SetItemsProcessed ( st.iterations() );
}
#endif

BENCHMARK_MAIN();

