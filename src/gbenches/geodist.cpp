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

#include "sphinxexpr.h"
#include <cmath>

static constexpr double HAVERSINE_EARTH_RADIUS = 6384000.0;
constexpr float ADAPTIVE_EARTH_RADIUS = 6371000.0f;

constexpr double PI = 3.14159265358979323846;
constexpr double TO_RAD = PI / 180.0;
constexpr double TO_RAD2 = PI / 360.0;
constexpr float TO_RADF = (float)( PI / 180.0 );
constexpr float TO_RADF2 = (float)( PI / 360.0 );

constexpr int GEODIST_TABLE_K		= 1024;
constexpr int GEODIST_TABLE_ASIN	= 512;
constexpr int GEODIST_TABLE_COS		= 1024; // maxerr 0.00063%

static float g_GeoCos[GEODIST_TABLE_COS+1];		///< cos(x) table
static float g_GeoAsin[GEODIST_TABLE_ASIN+1];	///< asin(sqrt(x)) table
static float g_GeoFlatK[GEODIST_TABLE_K+1][2];	///< GeodistAdaptive() flat ellipsoid method k1,k2 coeffs table

/// double argument squared
static FORCE_INLINE double sqr ( double v )
{
	return v * v;
}

inline float GeodistSphereDeg ( float lat1, float lon1, float lat2, float lon2 )
{
	static const double D = 2*HAVERSINE_EARTH_RADIUS;
	double dlat2 = TO_RAD2*( lat1 - lat2 );
	double dlon2 = TO_RAD2*( lon1 - lon2 );
	double a = sqr ( sin(dlat2) ) + cos ( TO_RAD*lat1 )*cos ( TO_RAD*lat2 )*sqr ( sin(dlon2) );
	double c = asin ( Min ( 1.0, sqrt(a) ) );
	return (float)(D*c);
}

static FORCE_INLINE float GeodistDegDiff ( float f )
{
	f = (float)fabs(f);
	while ( f>360 )
		f -= 360;
	if ( f>180 )
		f = 360-f;
	return f;
}

float GeodistFlatDeg ( float fLat1, float fLon1, float fLat2, float fLon2 );

static FORCE_INLINE float GeodistFastCos ( float x )
{
	auto y = (float)(fabs(x)*GEODIST_TABLE_COS/PI/2);
	auto i = int(y);
	y -= i;
	i &= ( GEODIST_TABLE_COS-1 );
	return g_GeoCos[i] + ( g_GeoCos[i+1]-g_GeoCos[i] )*y;
}

static FORCE_INLINE float GeodistFastSin ( float x )
{
	auto y = float(fabs(x)*GEODIST_TABLE_COS/PI/2);
	auto i = int(y);
	y -= i;
	i = ( i - GEODIST_TABLE_COS/4 ) & ( GEODIST_TABLE_COS-1 ); // cos(x-pi/2)=sin(x), costable/4=pi/2
	return g_GeoCos[i] + ( g_GeoCos[i+1]-g_GeoCos[i] )*y;
}

/// fast implementation of asin(sqrt(x))
/// max error in floats 0.00369%, in doubles 0.00072%
static inline float GeodistFastAsinSqrt ( float x )
{
	if ( x<0.122 )
	{
		// distance under 4546km, Taylor error under 0.00072%
		auto y = (float)sqrt(x);
		return y + x*y*0.166666666666666f + x*x*y*0.075f + x*x*x*y*0.044642857142857f;
	}
	if ( x<0.948 )
	{
		// distance under 17083km, 512-entry LUT error under 0.00072%
		x *= GEODIST_TABLE_ASIN;
		auto i = int(x);
		return g_GeoAsin[i] + ( g_GeoAsin[i+1] - g_GeoAsin[i] )*( x-i );
	}
	return (float)asin ( sqrt(x) ); // distance over 17083km, just compute honestly
}

inline float GeodistAdaptiveDeg ( float lat1, float lon1, float lat2, float lon2 )
{
	const float dlat = GeodistDegDiff ( lat1-lat2 );
	const float dlon = GeodistDegDiff ( lon1-lon2 );

	if ( dlon<13 )
	{
		// points are close enough; use flat ellipsoid model
		// interpolate sqr(k1), sqr(k2) coefficients using latitudes midpoint
		const float m = ( lat1+lat2+180 )*GEODIST_TABLE_K/360; // [-90, 90] degrees -> [0, KTABLE] indexes
		auto i = int(m);
		i &= ( GEODIST_TABLE_K-1 );
		const float kk1 = g_GeoFlatK[i][0] + ( g_GeoFlatK[i+1][0] - g_GeoFlatK[i][0] )*( m-i );
		const float kk2 = g_GeoFlatK[i][1] + ( g_GeoFlatK[i+1][1] - g_GeoFlatK[i][1] )*( m-i );
		return (float)sqrt ( ( dlat * dlat ) * kk1 + ( dlon * dlon ) * kk2 ); // note! order is important
	}

	// points too far away; use haversine
	constexpr float D = 2.0f*ADAPTIVE_EARTH_RADIUS;
	const float GeodistFastSindlat = GeodistFastSin ( dlat * TO_RADF2 );
	const float GeodistFastSindlon = GeodistFastSin ( dlon * TO_RADF2 );
	// notice sequence of calculation: that is important, since floating-point arithmetic is not transitive
	// just changing sequence of multipliers cause different result produced, in some cases (test 125) 1m far from reference on arm64 vs x64
	const float d = (GeodistFastSindlon * GeodistFastSindlon) * GeodistFastCos ( lat1 * TO_RADF ) * GeodistFastCos ( lat2 * TO_RADF );

	// select geodist(2.2,2.2,0.0,0.0) a;
	// gives 7754368.500000 on arm64
	// gives 7754367.500000 on x64
	//
	// const float a = GeodistFastSindlat * GeodistFastSindlat + d;
	// = 0.326833007591961793912669481665389525915 ( reference 128bit float with sollya - www.sollya.org )
	// = 0.32683298 on x64
	// = 0.32683301 on arm64
	//
	// split into following steps gives 0.32683298 on both x64 and arm64; let's use this as universal solution
	const float GeodistFastSindlatSqr = GeodistFastSindlat * GeodistFastSindlat;
	const float a = GeodistFastSindlatSqr + d;

	return D * GeodistFastAsinSqrt ( a );
}

// conversion between degrees and radians
static const double MY_PI = 3.14159265358979323846;
static const double TO_RADD = MY_PI / 180.0;
static const double TO_DEGD = 180.0 / MY_PI;

void DestVincenty ( double lat1, double lon1, double brng, double dist, double* lat2, double* lon2 )
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
	while ( fabs ( sigma - sigmaP ) > 1e-12 )
	{
		cos2SigmaM = cos ( 2 * sigma1 + sigma );
		sinSigma = sin ( sigma );
		cosSigma = cos ( sigma );
		double deltaSigma = B * sinSigma * ( cos2SigmaM + B / 4 * ( cosSigma * ( -1 + 2 * cos2SigmaM * cos2SigmaM ) - B / 6 * cos2SigmaM * ( -3 + 4 * sinSigma * sinSigma ) * ( -3 + 4 * cos2SigmaM * cos2SigmaM ) ) );
		sigmaP = sigma;
		sigma = s / ( b * A ) + deltaSigma;
	}

	double tmp = sinU1 * sinSigma - cosU1 * cosSigma * cosAlpha1;
	*lat2 = atan2 ( sinU1 * cosSigma + cosU1 * sinSigma * cosAlpha1,
		( 1 - f ) * sqrt ( sinAlpha * sinAlpha + tmp * tmp ) );
	double lambda = atan2 ( sinSigma * sinAlpha1, cosU1 * cosSigma - sinU1 * sinSigma * cosAlpha1 );
	double C = f / 16 * cosSqAlpha * ( 4 + f * ( 4 - 3 * cosSqAlpha ) );
	double L = lambda - ( 1 - C ) * f * sinAlpha * ( sigma + C * sinSigma * ( cos2SigmaM + C * cosSigma * ( -1 + 2 * cos2SigmaM * cos2SigmaM ) ) );
	*lon2 = ( lon1 * TO_RADD + L + 3 * MY_PI );
	while ( *lon2 > 2 * MY_PI )
		*lon2 -= 2 * MY_PI;
	*lon2 -= MY_PI;
	*lat2 *= TO_DEGD;
	*lon2 *= TO_DEGD;
}

static constexpr int NFUNCS = 3;

float CalcGeofunc ( int iFunc, double* t )
{
	switch ( iFunc )
	{
	case 0: return GeodistSphereDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
	case 1: return GeodistAdaptiveDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
	case 2: return GeodistFlatDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
	default: return 0;
	}
	return 0;
}

CSphVector<double> callonce()
{
	CSphVector<double> dBench;
	for ( int adist = 10; adist <= 10 * 1000 * 1000; adist *= 10 )
		for ( int dist = adist; dist < 10 * adist && dist < 20 * 1000 * 1000; dist += 2 * adist )
		{
			double avgerr[NFUNCS] = { 0 }, maxerr[NFUNCS] = { 0 };
			int n = 0;
			for ( int lat = -80; lat <= 80; lat += 10 )
			{
				for ( int lon = -179; lon < 180; lon += 3 )
				{
					for ( int b = 0; b < 360; b += 3, ++n )
					{
						double t[4] = { double ( lat ), double ( lon ), 0, 0 };
						DestVincenty ( t[0], t[1], b, dist, t + 2, t + 3 );
						for ( int j = 0; j < 4; j++ )
							dBench.Add ( t[j] );
						for ( int f = 0; f < NFUNCS; ++f )
						{
							float fDist = CalcGeofunc ( f, t );
							double err = fabs ( 100 * ( double ( fDist ) - double ( dist ) )
												/ double ( dist ) ); // relative error, in percents
							avgerr[f] += err;
							maxerr[f] = Max ( err, maxerr[f] );
						}
					}
				}
			}
			//				for ( int f = 0; f < NFUNCS; ++f )
			//					avgerr[f]/=n;
			//				printf ( "Sphere err max %f, avg %f\n", maxerr[0], avgerr[0] );
			//				printf ( "Adaptive err max %f, avg %f\n", maxerr[1], avgerr[1] );
			//				printf ( "Flat err max %f, avg %f\n", maxerr[2], avgerr[2] );
		}
	return dBench;
}

CSphVector<double>& stbench()
{
	static CSphVector<double> dBench = callonce();
	return dBench;
}

class Geodist: public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State& state )
	{
		tmax = dBench.Begin() + dBench.GetLength();
		fDist = 0;
	}

	//	void TearDown ( const ::benchmark::State & state ) { }

	CSphVector<double>& dBench = stbench();
	float fDist = 0;
	double* tmax;
};

BENCHMARK_DEFINE_F ( Geodist, SphereDeg )
( benchmark::State& st )
{
	double* t = dBench.Begin();
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( int i = 0; i < NRUNS; ++i )
		{
			fDist += GeodistSphereDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
			t += 4;
			if ( t >= tmax )
				t = dBench.Begin();
		}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
}
BENCHMARK_REGISTER_F ( Geodist, SphereDeg )->RangeMultiplier ( 10 )->Range ( 10, 1000 );

BENCHMARK_DEFINE_F ( Geodist, FlatDeg )
( benchmark::State& st )
{
	double* t = dBench.Begin();
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( int i = 0; i < NRUNS; ++i )
		{
			fDist += GeodistFlatDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
			t += 4;
			if ( t >= tmax )
				t = dBench.Begin();
		}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
}
BENCHMARK_REGISTER_F ( Geodist, FlatDeg )->RangeMultiplier ( 10 )->Range ( 10, 1000 );

BENCHMARK_DEFINE_F ( Geodist, AdaptiveDeg )
( benchmark::State& st )
{
	double* t = dBench.Begin();
	auto NRUNS = st.range ( 0 );
	for ( auto _ : st )
		for ( int i = 0; i < NRUNS; ++i )
		{
			fDist += GeodistAdaptiveDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
			t += 4;
			if ( t >= tmax )
				t = dBench.Begin();
		}
	st.SetItemsProcessed ( st.iterations() * NRUNS );
}
BENCHMARK_REGISTER_F ( Geodist, AdaptiveDeg )->RangeMultiplier ( 10 )->Range ( 10, 1000 );