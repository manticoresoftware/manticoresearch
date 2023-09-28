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

#include "geodist.h"

#include <cmath>

// conversions between degrees and radians
static const double PI = 3.14159265358979323846;
static const double TO_RAD = PI / 180.0;
static const double TO_RAD2 = PI / 360.0;
static const double TO_DEG = 180.0 / PI;
static const float TO_RADF = (float)( PI / 180.0 );
static const float TO_RADF2 = (float)( PI / 360.0 );
static const float TO_DEGF = (float)( 180.0 / PI );

const int GEODIST_TABLE_COS		= 1024; // maxerr 0.00063%
const int GEODIST_TABLE_ASIN	= 512;
const int GEODIST_TABLE_K		= 1024;

static float g_GeoCos[GEODIST_TABLE_COS+1];		///< cos(x) table
static float g_GeoAsin[GEODIST_TABLE_ASIN+1];	///< asin(sqrt(x)) table
static float g_GeoFlatK[GEODIST_TABLE_K+1][2];	///< GeodistAdaptive() flat ellipsoid method k1,k2 coeffs table

/// double argument squared
static FORCE_INLINE double sqr ( double v )
{
	return v * v;
}

void GeodistInit()
{
	for ( int i=0; i<=GEODIST_TABLE_COS; i++ )
		g_GeoCos[i] = (float)cos ( 2*PI*i/GEODIST_TABLE_COS ); // [0, 2pi] -> [0, COSTABLE]

	for ( int i=0; i<=GEODIST_TABLE_ASIN; i++ )
		g_GeoAsin[i] = (float)asin ( sqrt ( double(i)/GEODIST_TABLE_ASIN ) ); // [0, 1] -> [0, ASINTABLE]

	for ( int i=0; i<=GEODIST_TABLE_K; i++ )
	{
		double x = PI*i/GEODIST_TABLE_K - PI*0.5; // [-pi/2, pi/2] -> [0, KTABLE]
		g_GeoFlatK[i][0] = (float) sqr ( 111132.09 - 566.05*cos ( 2*x ) + 1.20*cos ( 4*x ) );
		g_GeoFlatK[i][1] = (float) sqr ( 111415.13*cos(x) - 94.55*cos ( 3*x ) + 0.12*cos ( 5*x ) );
	}
}


static const double HAVERSINE_EARTH_RADIUS = 6384000.0;

inline float GeodistSphereRad ( float lat1, float lon1, float lat2, float lon2 )
{
	static const double D = 2*HAVERSINE_EARTH_RADIUS;
	double dlat2 = 0.5*( lat1 - lat2 );
	double dlon2 = 0.5*( lon1 - lon2 );
	double a = sqr ( sin(dlat2) ) + cos(lat1)*cos(lat2)*sqr ( sin(dlon2) );
	double c = asin ( Min ( 1.0, sqrt(a) ) );
	return (float)(D*c);
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


float GeodistFlatDeg ( float fLat1, float fLon1, float fLat2, float fLon2 )
{
	double c1 = cos ( TO_RAD2*( fLat1+fLat2 ) );
	double c2 = 2*c1*c1-1; // cos(2*t)
	double c3 = c1*(2*c2-1); // cos(3*t)
	double k1 = 111132.09 - 566.05*c2;
	double k2 = 111415.13*c1 - 94.55*c3;
	float dlat = GeodistDegDiff ( fLat1-fLat2 );
	float dlon = GeodistDegDiff ( fLon1-fLon2 );
	return (float)sqrt ( k1*k1*dlat*dlat + k2*k2*dlon*dlon );
}


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

/// float argument squared
inline float fsqr ( float v )
{
	return v * v;
}

const float ADAPTIVE_EARTH_RADIUS = 6371000.0f;

inline float GeodistAdaptiveDeg ( float lat1, float lon1, float lat2, float lon2 )
{
	float dlat = GeodistDegDiff ( lat1-lat2 );
	float dlon = GeodistDegDiff ( lon1-lon2 );

	if ( dlon<13 )
	{
		// points are close enough; use flat ellipsoid model
		// interpolate sqr(k1), sqr(k2) coefficients using latitudes midpoint
		float m = ( lat1+lat2+180 )*GEODIST_TABLE_K/360; // [-90, 90] degrees -> [0, KTABLE] indexes
		auto i = int(m);
		i &= ( GEODIST_TABLE_K-1 );
		float kk1 = g_GeoFlatK[i][0] + ( g_GeoFlatK[i+1][0] - g_GeoFlatK[i][0] )*( m-i );
		float kk2 = g_GeoFlatK[i][1] + ( g_GeoFlatK[i+1][1] - g_GeoFlatK[i][1] )*( m-i );
		return (float)sqrt ( kk1*dlat*dlat + kk2*dlon*dlon );
	} else
	{
		// points too far away; use haversine
		static const float D = 2.0f*ADAPTIVE_EARTH_RADIUS;
		float a = fsqr ( GeodistFastSin ( dlat*TO_RADF2 ) ) + GeodistFastCos ( lat1*TO_RADF ) * GeodistFastCos ( lat2*TO_RADF ) * fsqr ( GeodistFastSin ( dlon*TO_RADF2 ) );
		return (float)( D*GeodistFastAsinSqrt(a) );
	}
}


inline float GeodistAdaptiveRad ( float lat1, float lon1, float lat2, float lon2 )
{
	// cut-paste-optimize, maybe?
	return GeodistAdaptiveDeg ( lat1*TO_DEGF, lon1*TO_DEGF, lat2*TO_DEGF, lon2*TO_DEGF );
}


void GeoTesselate ( CSphVector<float> & dIn )
{
	// 1 minute of latitude, max
	// (it varies from 1842.9 to 1861.57 at 0 to 90 respectively)
	static const float LAT_MINUTE = 1861.57f;

	// 1 minute of longitude in metres, at different latitudes
	static const float LON_MINUTE[] =
	{
		1855.32f, 1848.31f, 1827.32f, 1792.51f, // 0, 5, 10, 15
		1744.12f, 1682.50f, 1608.10f, 1521.47f, // 20, 25, 30, 35
		1423.23f, 1314.11f, 1194.93f, 1066.57f, // 40, 45, 50, 55
		930.00f, 786.26f, 636.44f, 481.70f, // 60, 65 70, 75
		323.22f, 162.24f, 0.0f // 80, 85, 90
	};

	// tesselation threshold
	// FIXME! make this configurable?
	static const float TESSELATE_TRESH = 500000.0f; // 500 km, error under 150m or 0.03%

	CSphVector<float> dOut;
	for ( int i=0; i<dIn.GetLength(); i+=2 )
	{
		// add the current vertex in any event
		dOut.Add ( dIn[i] );
		dOut.Add ( dIn[i+1] );

		// get edge lat/lon, convert to radians
		bool bLast = ( i==dIn.GetLength()-2 );
		float fLat1 = dIn[i];
		float fLon1 = dIn[i+1];
		float fLat2 = dIn [ bLast ? 0 : (i+2) ];
		float fLon2 = dIn [ bLast ? 1 : (i+3) ];

		// quick rough geodistance estimation
		float fMinLat = Min ( fLat1, fLat2 );
		auto iLatBand = (int) floor ( fabs ( fMinLat ) / 5.0f );
		iLatBand = iLatBand % 18;

		auto d = (float) (60.0f*( LAT_MINUTE*fabs ( fLat1-fLat2 ) + LON_MINUTE [ iLatBand ]*fabs ( fLon1-fLon2 ) ) );
		if ( d<=TESSELATE_TRESH )
			continue;

		// convert to radians
		// FIXME! make units configurable
		fLat1 *= TO_RADF;
		fLon1 *= TO_RADF;
		fLat2 *= TO_RADF;
		fLon2 *= TO_RADF;

		// compute precise geodistance
		d = GeodistSphereRad ( fLat1, fLon1, fLat2, fLon2 );
		if ( d<=TESSELATE_TRESH )
			continue;
		int iSegments = (int) ceil ( d / TESSELATE_TRESH );

		// compute arc distance
		// OPTIMIZE! maybe combine with CalcGeodist?
		d = (float)acos ( sin(fLat1)*sin(fLat2) + cos(fLat1)*cos(fLat2)*cos(fLon1-fLon2) );
		const auto isd = (float)(1.0f / sin(d));
		const auto clat1 = (float)cos(fLat1);
		const auto slat1 = (float)sin(fLat1);
		const auto clon1 = (float)cos(fLon1);
		const auto slon1 = (float)sin(fLon1);
		const auto clat2 = (float)cos(fLat2);
		const auto slat2 = (float)sin(fLat2);
		const auto clon2 = (float)cos(fLon2);
		const auto slon2 = (float)sin(fLon2);

		for ( int j=1; j<iSegments; j++ )
		{
			float f = float(j) / float(iSegments); // needed distance fraction
			float a = (float)sin ( (1-f)*d ) * isd;
			float b = (float)sin ( f*d ) * isd;
			float x = a*clat1*clon1 + b*clat2*clon2;
			float y = a*clat1*slon1 + b*clat2*slon2;
			float z = a*slat1 + b*slat2;
			dOut.Add ( (float)( TO_DEG * atan2 ( z, sqrt ( x*x+y*y ) ) ) );
			dOut.Add ( (float)( TO_DEG * atan2 ( y, x ) ) );
		}
	}

	// swap 'em results
	dIn.SwapData ( dOut );
}


bool GeodistGetSphereBBox ( Geofunc_fn fnFunc, float fLat, float fLon, float fDist, float & fLatMin, float & fLatMax, float & fLonMin, float & fLonMax )
{
	bool bAdaptive = fnFunc==GetGeodistFn ( GEO_ADAPTIVE, true ) || fnFunc==GetGeodistFn ( GEO_ADAPTIVE, false );
	bool bDeg = fnFunc==GetGeodistFn ( GEO_ADAPTIVE, true ) || fnFunc==GetGeodistFn ( GEO_HAVERSINE, true );

	double fLatRad = fLat;
	double fLonRad = fLon;
	if ( bDeg )
	{
		fLatRad *= TO_RAD;
		fLonRad *= TO_RAD;
	}

	double fRadius = HAVERSINE_EARTH_RADIUS;
	if ( bAdaptive )
		fRadius = ADAPTIVE_EARTH_RADIUS;

	const double CORRECTION_LAT = 1.003;
	const double CORRECTION_LON = 1.10;
	double fAngularDist = (double)fDist / fRadius*CORRECTION_LAT;
	fLatMin = fLatRad - fAngularDist;
	fLatMax = fLatRad + fAngularDist;

	// pole is near
	if ( fLatMin <= -PI/2.0 || fLatMax >= PI/2.0 )
		return false;

	// about 100m
	const double EPS = 1.570795e-5;
	if ( fabs ( fLatRad - PI/2.0 ) <= EPS || fabs ( fLatRad + PI/2.0 ) <= EPS )
		return false;

	double fDeltaLon = fAngularDist / cos(fLatRad) * CORRECTION_LON;
	fLonMin = float(fLonRad - fDeltaLon);
	fLonMax = float(fLonRad + fDeltaLon);

	// near prime meridian?
	if ( fLonMin < -PI )
		fLonMin += 2*PI;

	if ( fLonMax > PI )
		fLonMax -= 2*PI;

	if ( fLonMin > fLonMax )
		Swap ( fLonMin, fLonMax );

	// too near to the poles
	if ( fLonMin < -PI || fLonMax > PI )
		return false;

	if ( bDeg )
	{
		fLatMin *= TO_DEGF;
		fLatMax *= TO_DEGF;
		fLonMin *= TO_DEGF;
		fLonMax *= TO_DEGF;
	}

	return true;
}


Geofunc_fn GetGeodistFn ( GeoFunc_e eFunc, bool bDeg )
{
	switch ( 2*eFunc+bDeg )
	{
	case 2*GEO_HAVERSINE:		return &GeodistSphereRad;
	case 2*GEO_HAVERSINE+1:		return &GeodistSphereDeg;
	case 2*GEO_ADAPTIVE:		return &GeodistAdaptiveRad;
	case 2*GEO_ADAPTIVE+1:		return &GeodistAdaptiveDeg;
	default:;
	}
	return nullptr;
}


float CalcGeodist ( GeoFunc_e eFunc, bool bDeg, float lat1, float lon1, float lat2, float lon2 )
{
	return GetGeodistFn ( eFunc, bDeg ) ( lat1, lon1, lat2, lon2 );
}


bool GeoDistanceUnit ( const char * szUnit, float & fCoeff )
{
	struct DistanceUnit_t
	{
		CSphString	m_dNames[3];
		float		m_fConversion;
	};

	static DistanceUnit_t dUnits[] = 
	{
		{ { "mi", "miles" },				1609.34f },
		{ { "yd", "yards" },				0.9144f },
		{ { "ft", "feet" },					0.3048f },
		{ { "in", "inch" },					0.0254f },
		{ { "km", "kilometers" },			1000.0f },
		{ { "m", "meters" },				1.0f },
		{ { "cm", "centimeters" },			0.01f },
		{ { "mm", "millimeters" },			0.001f },
		{ { "NM", "nmi", "nauticalmiles" },	1852.0f }
	};

	if ( !szUnit || !*szUnit )
	{
		fCoeff = 1.0f;
		return true;
	}

	for ( const auto & i : dUnits )
		for ( const auto & j : i.m_dNames )
			if ( j==szUnit )
			{
				fCoeff = i.m_fConversion;
				return true;
			}

	return false;
}
