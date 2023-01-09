//
// Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)
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