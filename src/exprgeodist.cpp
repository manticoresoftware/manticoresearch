//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "exprgeodist.h"

#include "sphinx.h"
#include "exprtraits.h"


class ExprGeodist_c : public ISphExpr
{
public:
						ExprGeodist_c() = default;

	bool				Setup ( const CSphQuery & tQuery, const ISphSchema & tSchema, CSphString & sError );
	float				Eval ( const CSphMatch & tMatch ) const final;
	void				FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final;
	void				Command ( ESphExprCommand eCmd, void * pArg ) final;
	uint64_t			GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final;
	ISphExpr *			Clone() const final;

protected:
	CSphAttrLocator		m_tGeoLatLoc;
	CSphAttrLocator		m_tGeoLongLoc;
	float				m_fGeoAnchorLat;
	float				m_fGeoAnchorLong;
	int					m_iLat;
	int					m_iLon;
};


bool ExprGeodist_c::Setup ( const CSphQuery & tQuery, const ISphSchema & tSchema, CSphString & sError )
{
	if ( !tQuery.m_bGeoAnchor )
	{
		sError.SetSprintf ( "INTERNAL ERROR: no geoanchor, can not create geodist evaluator" );
		return false;
	}

	int iLat = tSchema.GetAttrIndex ( tQuery.m_sGeoLatAttr.cstr() );
	if ( iLat<0 )
	{
		sError.SetSprintf ( "unknown latitude attribute '%s'", tQuery.m_sGeoLatAttr.cstr() );
		return false;
	}

	int iLong = tSchema.GetAttrIndex ( tQuery.m_sGeoLongAttr.cstr() );
	if ( iLong<0 )
	{
		sError.SetSprintf ( "unknown latitude attribute '%s'", tQuery.m_sGeoLongAttr.cstr() );
		return false;
	}

	m_tGeoLatLoc = tSchema.GetAttr(iLat).m_tLocator;
	m_tGeoLongLoc = tSchema.GetAttr(iLong).m_tLocator;
	m_fGeoAnchorLat = tQuery.m_fGeoLatitude;
	m_fGeoAnchorLong = tQuery.m_fGeoLongitude;
	m_iLat = iLat;
	m_iLon = iLong;

	return true;
}


static inline double sphSqr ( double v )
{
	return v*v;
}


float ExprGeodist_c::Eval ( const CSphMatch & tMatch ) const
{
	const double R = 6384000;
	float plat = tMatch.GetAttrFloat ( m_tGeoLatLoc );
	float plon = tMatch.GetAttrFloat ( m_tGeoLongLoc );
	double dlat = plat - m_fGeoAnchorLat;
	double dlon = plon - m_fGeoAnchorLong;
	double a = sphSqr ( sin ( dlat/2 ) ) + cos(plat)*cos(m_fGeoAnchorLat)*sphSqr(sin(dlon/2));
	double c = 2*asin ( Min ( 1.0, sqrt(a) ) );
	return (float)(R*c);
}


void ExprGeodist_c::FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	sphFixupLocator ( m_tGeoLatLoc, pOldSchema, pNewSchema );
	sphFixupLocator ( m_tGeoLongLoc, pOldSchema, pNewSchema );
}


void ExprGeodist_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( eCmd==SPH_EXPR_GET_DEPENDENT_COLS )
	{
		static_cast < CSphVector<int>* >(pArg)->Add ( m_iLat );
		static_cast < CSphVector<int>* >(pArg)->Add ( m_iLon );
	}

	if ( eCmd==SPH_EXPR_UPDATE_DEPENDENT_COLS )
	{
		int iRef = *static_cast<int*>(pArg);
		if ( m_iLat>=iRef )	m_iLat--;
		if ( m_iLon>=iRef )	m_iLon--;
	}
}


uint64_t ExprGeodist_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	uint64_t uHash = sphCalcExprDepHash ( this, tSorterSchema, uPrevHash, bDisable );

	static const char * EXPR_TAG = "ExprGeodist_c";
	uHash = sphFNV64 ( EXPR_TAG, (int) strlen(EXPR_TAG), uHash );
	uHash = sphFNV64 ( &m_fGeoAnchorLat, sizeof(m_fGeoAnchorLat), uHash );
	uHash = sphFNV64 ( &m_fGeoAnchorLong, sizeof(m_fGeoAnchorLong), uHash );

	return uHash;
}


ISphExpr * ExprGeodist_c::Clone() const
{
	auto * pClone = new ExprGeodist_c;
	pClone->m_tGeoLatLoc = m_tGeoLatLoc;
	pClone->m_tGeoLongLoc = m_tGeoLongLoc;
	pClone->m_fGeoAnchorLat = m_fGeoAnchorLat;
	pClone->m_fGeoAnchorLong = m_fGeoAnchorLong;
	pClone->m_iLat = m_iLat;
	pClone->m_iLon = m_iLon;

	return pClone;
}

///////////////////////////////////////////////////////////////////////////////

ISphExpr * CreateExprGeodist ( const CSphQuery & tQuery, const ISphSchema & tSchema, CSphString & sError )
{
	auto * pExpr = new ExprGeodist_c;
	if ( !pExpr->Setup ( tQuery, tSchema, sError ) )
	{
		pExpr->Release();
		return nullptr;
	}

	return pExpr;
}
