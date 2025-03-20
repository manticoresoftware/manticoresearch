//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _jsonqueryfilter_
#define _jsonqueryfilter_

#include "sphinxjson.h"

struct CSphQuery;

struct LocationField_t
{
	float m_fLat = 0.0f;
	float m_fLon = 0.0f;
};


struct LocationSource_t
{
	CSphString m_sLat;
	CSphString m_sLon;
};


class GeoDistInfo_c
{
public:
	bool				Parse ( const JsonObj_c & tRoot, bool bNeedDistance, CSphString & sError, CSphString & sWarning );
	CSphString			BuildExprString() const;
	bool				IsGeoDist() const { return m_bGeodist; }
	float				GetDistance() const { return m_fDistance; }

private:
	bool				m_bGeodist {false};
	bool				m_bGeodistAdaptive {true};
	float				m_fDistance {0.0f};

	LocationField_t		m_tLocAnchor;
	LocationSource_t	m_tLocSource;

	bool				ParseDistance ( const JsonObj_c & tDistance, CSphString & sError );
};


const char *	GetFilterAttrPrefix();
bool			IsFilter ( const JsonObj_c & tJson );
bool			ParseJsonQueryFilters ( const JsonObj_c & tJson, CSphQuery & tQuery, CSphString & sError, CSphString & sWarning );

#endif
