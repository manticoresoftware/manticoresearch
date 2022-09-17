//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

// conversion between degrees and radians
static const double MY_PI = 3.14159265358979323846;
static const double TO_RADD = MY_PI / 180.0;
static const double TO_DEGD = 180.0 / MY_PI;

void DestVincenty ( double lat1, double lon1, double brng, double dist, double * lat2, double * lon2 )
{
	double a = 6378137, b = 6356752.3142, f = 1 / 298.257223563; // WGS-84 ellipsiod
	double s = dist;
	double alpha1 = brng * TO_RADD;
	double sinAlpha1 = sin ( alpha1 );
	double cosAlpha1 = cos ( alpha1 );

	double tanU1 = ( 1 - f ) * tan ( lat1 * TO_RADD );
	double cosU1 = 1 / sqrt ( 1 + tanU1 * tanU1 ), sinU1 = tanU1 * cosU1;
	double sigma1 = atan2 ( tanU1, cosAlpha1 );
	double sinAlpha = cosU1 * sinAlpha1;
	double cosSqAlpha = 1 - sinAlpha * sinAlpha;
	double uSq = cosSqAlpha * ( a * a - b * b ) / ( b * b );
	double A = 1 + uSq / 16384 * ( 4096 + uSq * ( -768 + uSq * ( 320 - 175 * uSq ) ) );
	double B = uSq / 1024 * ( 256 + uSq * ( -128 + uSq * ( 74 - 47 * uSq ) ) );

	double sigma = s / ( b * A ), sigmaP = 2 * MY_PI;
	double cos2SigmaM = 0, sinSigma = 0, cosSigma = 0;
	while ( fabs ( sigma - sigmaP )>1e-12 )
	{
		cos2SigmaM = cos ( 2 * sigma1 + sigma );
		sinSigma = sin ( sigma );
		cosSigma = cos ( sigma );
		double deltaSigma = B * sinSigma * ( cos2SigmaM + B / 4 * ( cosSigma * ( -1 + 2 * cos2SigmaM * cos2SigmaM ) -
			B / 6 * cos2SigmaM * ( -3 + 4 * sinSigma * sinSigma ) * ( -3 + 4 * cos2SigmaM * cos2SigmaM ) ) );
		sigmaP = sigma;
		sigma = s / ( b * A ) + deltaSigma;
	}

	double tmp = sinU1 * sinSigma - cosU1 * cosSigma * cosAlpha1;
	*lat2 = atan2 ( sinU1 * cosSigma + cosU1 * sinSigma * cosAlpha1,
		( 1 - f ) * sqrt ( sinAlpha * sinAlpha + tmp * tmp ) );
	double lambda = atan2 ( sinSigma * sinAlpha1, cosU1 * cosSigma - sinU1 * sinSigma * cosAlpha1 );
	double C = f / 16 * cosSqAlpha * ( 4 + f * ( 4 - 3 * cosSqAlpha ) );
	double L = lambda - ( 1 - C ) * f * sinAlpha *
		( sigma + C * sinSigma * ( cos2SigmaM + C * cosSigma * ( -1 + 2 * cos2SigmaM * cos2SigmaM ) ) );
	*lon2 = ( lon1 * TO_RADD + L + 3 * MY_PI );
	while ( *lon2>2 * MY_PI )
		*lon2 -= 2 * MY_PI;
	*lon2 -= MY_PI;
	*lat2 *= TO_DEGD;
	*lon2 *= TO_DEGD;
}

static const int NFUNCS = 3;

float CalcGeofunc ( int iFunc, double * t )
{
	switch ( iFunc )
	{
		case 0: return GeodistSphereDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
		case 1: return GeodistAdaptiveDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
		case 2: return GeodistFlatDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
	}
	return 0;
}

class BM_Geodist : public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State & state )
	{
		for ( int adist = 10; adist<=10 * 1000 * 1000; adist *= 10 )
			for ( int dist = adist; dist<10 * adist && dist<20 * 1000 * 1000; dist += 2 * adist )
			{
				double avgerr[NFUNCS] = { 0 }, maxerr[NFUNCS] = { 0 };
				int n = 0;
				for ( int lat = -80; lat<=80; lat += 10 )
				{
					for ( int lon = -179; lon<180; lon += 3 )
					{
						for ( int b = 0; b<360; b += 3, n++ )
						{
							double t[4] = { double ( lat ), double ( lon ), 0, 0 };
							DestVincenty ( t[0], t[1], b, dist, t+2, t+3 );
							for ( int j = 0; j<4; j++ )
								dBench.Add ( t[j] );
							for ( int f = 0; f<NFUNCS; f++ )
							{
								float fDist = CalcGeofunc ( f, t );
								double err = fabs ( 100 * ( double ( fDist )-double ( dist ) )
															/ double ( dist ) ); // relative error, in percents
								avgerr[f] += err;
								maxerr[f] = Max ( err, maxerr[f] );
							}
						}
					}
				}
			}
		tmax = dBench.Begin ()+dBench.GetLength ();
	}

	//	void TearDown ( const ::benchmark::State & state ) { }

	CSphVector<double> dBench;
	float fDist = 0;
	double * tmax;
};

BENCHMARK_F( BM_Geodist, SphereDeg ) ( benchmark::State & st )
{
	for ( auto _ : st )
		for ( double * t = dBench.Begin (); t<tmax; t += 4 )
			fDist += GeodistSphereDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
}

BENCHMARK_F( BM_Geodist, FlatDeg ) ( benchmark::State & st )
{
	for ( auto _ : st )
		for ( double * t = dBench.Begin (); t<tmax; t += 4 )
			fDist += GeodistFlatDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
}

BENCHMARK_F( BM_Geodist, AdaptiveDeg ) ( benchmark::State & st )
{
	for ( auto _ : st )
		for ( double * t = dBench.Begin (); t<tmax; t += 4 )
			fDist += GeodistAdaptiveDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
}

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

class bench_escape : public benchmark::Fixture
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

BENCHMARK_F( bench_escape, empty_pass ) ( benchmark::State & st )
{
	for ( auto _ : st )
		bRes |= !!dChars[++i & 0x7F];
}

BENCHMARK_F( bench_escape, test_memchr ) ( benchmark::State & st )
{
	for ( auto _ : st )
		bRes |= IsEscapeChar1 ( dChars[++i & 0x7F] );
}

BENCHMARK_F( bench_escape, test_strchr ) ( benchmark::State & st )
{
	for ( auto _ : st )
		bRes |= IsEscapeChar2 ( dChars[++i & 0x7F] );
}

BENCHMARK_F( bench_escape, test_switch ) ( benchmark::State & st )
{
	for ( auto _ : st )
		bRes |= IsEscapeChar3 ( dChars[++i & 0x7F] );
}

BENCHMARK_F( bench_escape, test_booltable ) ( benchmark::State & st )
{
	for ( auto _ : st )
		bRes |= IsEscapeChar4 ( dChars[++i & 0x7F] );
}

BENCHMARK_F( bench_escape, test_bytetable ) ( benchmark::State & st )
{
	for ( auto _ : st )
		bRes |= IsEscapeChar5 ( dChars[++i & 0x7F] );
}

BENCHMARK_F( bench_escape, transform_switch ) ( benchmark::State & st )
{
	for ( auto _ : st )
		bRes |= GetEscapedChar1 ( dChars[++i & 0x7F] );
}

BENCHMARK_F( bench_escape, transform_narrow_table ) ( benchmark::State & st )
{
	for ( auto _ : st )
		bRes |= GetEscapedChar2 ( dChars[++i & 0x7F] );
}

BENCHMARK_F( bench_escape, transform_wide_table ) ( benchmark::State & st )
{
	for ( auto _ : st )
		bRes |= GetEscapedChar3 ( dChars[++i & 0x7F] );
}

BENCHMARK_F( bench_escape, test_strchr_transform_switch ) ( benchmark::State & st )
{
	for ( auto _ : st )
		if ( IsEscapeChar2 ( dChars[++i & 0x7F] ) )    // combo usual
			bRes |= !!GetEscapedChar1 ( dChars[i & 0x7F] );
}

BENCHMARK_F( bench_escape, test_and_transform_by_one_table ) ( benchmark::State & st )
{
	for ( auto _ : st )
		if ( IsEscapeChar5 ( dChars[++i & 0x7F] ) )    // combo usual
			bRes |= !!GetEscapedChar3combo ( dChars[i & 0x7F] );
}


static void BM_sphSprintf ( benchmark::State & state )
{
	char sBuf[40];
	for ( auto _ : state )
		sph::Sprintf ( sBuf, "%d", 1000000 );
}

static void BM_stdSprintf ( benchmark::State & state )
{
	char sBuf[40];
	for ( auto _ : state )
		sprintf ( sBuf, "%d", 1000000 );
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

BENCHMARK_MAIN();

