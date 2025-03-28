//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _geodist_
#define _geodist_

#include "sphinxstd.h"

enum GeoFunc_e
{
	GEO_HAVERSINE,
	GEO_ADAPTIVE
};

typedef float (*Geofunc_fn)( float, float, float, float );

struct GeoDistSettings_t
{
	Geofunc_fn	m_pFunc = nullptr;
	float		m_fScale = 1.0f;
	float		m_fAnchorLat = 0.0f;
	float		m_fAnchorLon = 0.0f;
	CSphString	m_sAttrLat;
	CSphString	m_sAttrLon;
};

struct Poly2dBBox_t
{
	float		m_fMinX = 0.0f;
	float		m_fMinY = 0.0f;
	float		m_fMaxX = 0.0f;
	float		m_fMaxY = 0.0f;
	int			m_iNumPoints = 0;
	CSphString	m_sAttrLat;
	CSphString	m_sAttrLon;
};

Geofunc_fn	GetGeodistFn ( GeoFunc_e eFunc, bool bDeg );
float		CalcGeodist ( GeoFunc_e eFunc, bool bDeg, float lat1, float lon1, float lat2, float lon2 );
bool		GeodistGetSphereBBox ( Geofunc_fn fnFunc, float fLat, float fLon, float fDist, float & fLatMin, float & fLatMax, float & fLonMin, float & fLonMax );

/// init tables used by our geodistance functions
void		GeodistInit();
void		GeoTesselate ( CSphVector<float> & dIn );

/// get geodist conversion coeff
bool		GeoDistanceUnit ( const char * szUnit, float & fCoeff );

#endif // _geodist_
