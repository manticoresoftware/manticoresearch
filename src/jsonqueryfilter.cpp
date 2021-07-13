//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "jsonqueryfilter.h"

#include "sphinxint.h"

static const char * g_szFilter = "_@filter_";

static bool ParseLatLon ( const JsonObj_c & tLat, const JsonObj_c & tLon, LocationField_t * pField, LocationSource_t * pSource, CSphString & sError )
{
	if ( !tLat || !tLon )
	{
		if ( !tLat && !tLon )
			sError = R"("lat" and "lon" properties missing)";
		else
			sError.SetSprintf ( R"("%s" property missing)", ( !tLat ? "lat" : "lon" ) );

		return false;
	}

	bool bParseField = !!pField;
	bool bLatChecked = bParseField ? tLat.IsNum() : tLat.IsStr();
	bool bLonChecked = bParseField ? tLon.IsNum() : tLon.IsStr();
	if ( !bLatChecked || !bLonChecked )
	{
		if ( !bLatChecked && !bLonChecked )
			sError.SetSprintf ( R"("lat" and "lon" property values should be %s)", ( bParseField ? "numeric" : "string" ) );
		else
			sError.SetSprintf ( R"("%s" property value should be %s)", ( !bLatChecked ? "lat" : "lon" ), ( bParseField ? "numeric" : "string" ) );

		return false;
	}

	if ( bParseField )
	{
		pField->m_fLat = tLat.FltVal();
		pField->m_fLon = tLon.FltVal();
	} else
	{
		pSource->m_sLat = tLat.StrVal();
		pSource->m_sLon = tLon.StrVal();
	}

	return true;
}


static bool ParseLocation ( const char * sName, const JsonObj_c & tLoc, LocationField_t * pField, LocationSource_t * pSource, CSphString & sError )
{
	bool bParseField = !!pField;
	assert ( ( bParseField && pField ) || pSource );

	bool bObj = tLoc.IsObj();
	bool bString = tLoc.IsStr();
	bool bArray = tLoc.IsArray();

	if ( !bObj && !bString && !bArray )
	{
		sError.SetSprintf ( "\"%s\" property value should be either an object or a string or an array", sName );
		return false;
	}

	if ( bObj )
		return ParseLatLon ( tLoc.GetItem("lat"), tLoc.GetItem("lon"), pField, pSource, sError );

	if ( bString )
	{
		StrVec_t dVals;
		sphSplit ( dVals, tLoc.SzVal() );

		if ( dVals.GetLength()!=2 )
		{
			sError.SetSprintf ( "\"%s\" property values should be sting with lat,lon items, got %d items", sName, dVals.GetLength() );
			return false;
		}

		// string and array order differs
		// string - lat, lon
		// array - lon, lat
		int iLatLen = dVals[0].Length();
		int iLonLen = dVals[1].Length();
		if ( !iLatLen || !iLonLen )
		{
			if ( !iLatLen && !iLonLen )
				sError.SetSprintf ( R"("lat" and "lon" values should be %s)", ( bParseField ? "numeric" : "string" ) );
			else
				sError.SetSprintf ( "\"%s\" value should be %s", ( !iLatLen ? "lat" : "lon" ), ( bParseField ? "numeric" : "string" ) );

			return false;
		}

		if ( bParseField )
		{
			pField->m_fLat = (float)atof ( dVals[0].cstr() );
			pField->m_fLon = (float)atof ( dVals[1].cstr() );
		} else
		{
			pSource->m_sLat = dVals[0];
			pSource->m_sLon = dVals[1];
		}

		return true;
	}

	assert ( bArray );
	int iCount = tLoc.Size();
	if ( iCount!=2 )
	{
		sError.SetSprintf ( "\"%s\" property values should be an array with lat,lon items, got %d items", sName, iCount );
		return false;
	}

	// string and array order differs
	// array - lon, lat
	// string - lat, lon
	return ParseLatLon ( tLoc[1], tLoc[0], pField, pSource, sError );
}


//////////////////////////////////////////////////////////////////////////

bool GeoDistInfo_c::Parse ( const JsonObj_c & tRoot, bool bNeedDistance, CSphString & sError, CSphString & sWarning )
{
	JsonObj_c tLocAnchor = tRoot.GetItem("location_anchor");
	JsonObj_c tLocSource = tRoot.GetItem("location_source");

	if ( !tLocAnchor || !tLocSource )
	{
		if ( !tLocAnchor && !tLocSource )
			sError = R"("location_anchor" and "location_source" properties missing)";
		else
			sError.SetSprintf ( "\"%s\" property missing", ( !tLocAnchor ? "location_anchor" : "location_source" ) );
		return false;
	}

	if ( !ParseLocation ( "location_anchor", tLocAnchor, &m_tLocAnchor, nullptr, sError )
		|| !ParseLocation ( "location_source", tLocSource, nullptr, &m_tLocSource, sError ) )
		return false;

	JsonObj_c tType = tRoot.GetStrItem ( "distance_type", sError, true );
	if ( tType )
	{
		CSphString sType = tType.StrVal();
		if ( sType!="adaptive" && sType!="haversine" )
		{
			sWarning.SetSprintf ( R"("distance_type" property type is invalid: "%s", defaulting to "adaptive")", sType.cstr() );
			m_bGeodistAdaptive = true;
		} else
			m_bGeodistAdaptive = sType=="adaptive";

	} else if ( !sError.IsEmpty() )
		return false;

	JsonObj_c tDistance = tRoot.GetItem("distance");
	if ( tDistance )
	{
		if ( !ParseDistance ( tDistance, sError ) )
			return false;
	} else if ( bNeedDistance )
	{
		sError = "\"distance\" not specified";
		return false;
	}

	m_bGeodist = true;

	return true;
}


bool GeoDistInfo_c::ParseDistance ( const JsonObj_c & tDistance, CSphString & sError )
{
	if ( tDistance.IsNum() )
	{
		// no units specified, meters assumed
		m_fDistance = tDistance.FltVal();
		return true;
	}

	if ( !tDistance.IsStr() )
	{
		sError = "\"distance\" property should be a number or a string";
		return false;
	}

	const char * p = tDistance.SzVal();
	assert ( p );
	while ( *p && sphIsSpace(*p) )
		p++;

	const char * szNumber = p;
	while ( *p && ( *p=='.' || ( *p>='0' && *p<='9' ) ) )
		p++;

	CSphString sNumber;
	sNumber.SetBinary ( szNumber, int ( p-szNumber ) );

	while ( *p && sphIsSpace(*p) )
		p++;

	const char * szUnit = p;
	while ( *p && sphIsAlpha(*p) )
		p++;

	CSphString sUnit;
	sUnit.SetBinary ( szUnit, int ( p-szUnit ) );

	m_fDistance = (float)atof ( sNumber.cstr() );

	float fCoeff = 1.0f;	
	if ( !sphGeoDistanceUnit ( sUnit.cstr(), fCoeff ) )
	{
		sError.SetSprintf ( "unknown distance unit: %s", sUnit.cstr() );
		return false;
	}

	m_fDistance *= fCoeff;

	return true;
}


CSphString GeoDistInfo_c::BuildExprString() const
{
	CSphString sResult;
	sResult.SetSprintf ( "GEODIST(%f, %f, %s, %s, {in=deg, out=m, method=%s})", m_tLocAnchor.m_fLat, m_tLocAnchor.m_fLon, m_tLocSource.m_sLat.cstr(), m_tLocSource.m_sLon.cstr(), m_bGeodistAdaptive ? "adaptive" : "haversine" );
	return sResult;
}


//////////////////////////////////////////////////////////////////////////

const char * GetFilterAttrPrefix()
{
	return g_szFilter;
}


bool IsFilter ( const JsonObj_c & tJson )
{
	if ( !tJson )
		return false;

	CSphString sName = tJson.Name();

	if ( sName=="equals" )
		return true;

	if ( sName=="range" )
		return true;

	if ( sName=="in" )
		return true;

	if ( sName=="geo_distance" )
		return true;

	return false;
}


static void AddToSelectList ( CSphQuery & tQuery, const CSphVector<CSphQueryItem> & dItems, int iFirstItem = 0 )
{
	for ( int i = iFirstItem; i < dItems.GetLength(); i++ )
		tQuery.m_sSelect.SetSprintf ( "%s, %s as %s", tQuery.m_sSelect.cstr(), dItems[i].m_sExpr.cstr(), dItems[i].m_sAlias.cstr() );
}


static JsonObj_c GetFilterColumn ( const JsonObj_c & tJson, CSphString & sError )
{
	if ( !tJson.IsObj() )
	{
		sError = "filter should be an object";
		return JsonNull;
	}

	if ( tJson.Size()!=1 )
	{
		sError = "filter should have only one element";
		return JsonNull;
	}

	JsonObj_c tColumn = tJson[0];
	if ( !tColumn )
	{
		sError = "empty filter found";
		return JsonNull;
	}

	return tColumn;
}


static bool ConstructEqualsFilter ( const JsonObj_c & tJson, CSphVector<CSphFilterSettings> & dFilters, CSphString & sError )
{
	JsonObj_c tColumn = GetFilterColumn ( tJson, sError );
	if ( !tColumn )
		return false;

	if ( !tColumn.IsNum() && !tColumn.IsStr() )
	{
		sError = "\"equals\" filter expects numeric or string values";
		return false;
	}

	CSphFilterSettings tFilter;
	tFilter.m_sAttrName = tColumn.Name();
	sphColumnToLowercase ( const_cast<char *>( tFilter.m_sAttrName.cstr() ) );

	if ( tColumn.IsInt() )
	{
		tFilter.m_eType = SPH_FILTER_VALUES;
		tFilter.m_dValues.Add ( tColumn.IntVal() );
	} else if ( tColumn.IsNum() )
	{
		tFilter.m_eType = SPH_FILTER_FLOATRANGE;
		tFilter.m_fMinValue = tColumn.FltVal();
		tFilter.m_fMaxValue = tColumn.FltVal();
		tFilter.m_bHasEqualMin = true;
		tFilter.m_bHasEqualMax = true;
		tFilter.m_bExclude = false;
	} else
	{
		tFilter.m_eType = SPH_FILTER_STRING;
		tFilter.m_dStrings.Add ( tColumn.StrVal() );
		tFilter.m_bExclude = false;
	}

	dFilters.Add ( tFilter );

	return true;
}


static bool ConstructRangeFilter ( const JsonObj_c & tJson, CSphVector<CSphFilterSettings> & dFilters, CSphString & sError )
{
	JsonObj_c tColumn = GetFilterColumn ( tJson, sError );
	if ( !tColumn )
		return false;

	CSphFilterSettings tNewFilter;
	tNewFilter.m_sAttrName = tColumn.Name();
	sphColumnToLowercase ( const_cast<char *>( tNewFilter.m_sAttrName.cstr() ) );

	tNewFilter.m_bHasEqualMin = false;
	tNewFilter.m_bHasEqualMax = false;

	JsonObj_c tLess = tColumn.GetItem("lt");
	if ( !tLess )
	{
		tLess = tColumn.GetItem("lte");
		tNewFilter.m_bHasEqualMax = tLess;
	}

	JsonObj_c tGreater = tColumn.GetItem("gt");
	if ( !tGreater )
	{
		tGreater = tColumn.GetItem("gte");
		tNewFilter.m_bHasEqualMin = tGreater;
	}

	bool bLess = tLess;
	bool bGreater = tGreater;

	if ( !bLess && !bGreater )
	{
		sError = "empty filter found";
		return false;
	}

	if ( ( bLess && !tLess.IsNum() ) || ( bGreater && !tGreater.IsNum() ) )
	{
		sError = "range filter expects numeric values";
		return false;
	}

	bool bIntFilter = ( bLess && tLess.IsInt() ) || ( bGreater && tGreater.IsInt() );

	if ( bLess )
	{
		if ( bIntFilter )
			tNewFilter.m_iMaxValue = tLess.IntVal();
		else
			tNewFilter.m_fMaxValue = tLess.FltVal();

		tNewFilter.m_bOpenLeft = !bGreater;
	}

	if ( bGreater )
	{
		if ( bIntFilter )
			tNewFilter.m_iMinValue = tGreater.IntVal();
		else
			tNewFilter.m_fMinValue = tGreater.FltVal();

		tNewFilter.m_bOpenRight = !bLess;
	}

	tNewFilter.m_eType = bIntFilter ? SPH_FILTER_RANGE : SPH_FILTER_FLOATRANGE;

	// float filters don't support open ranges
	if ( !bIntFilter )
	{
		if ( tNewFilter.m_bOpenRight )
			tNewFilter.m_fMaxValue = FLT_MAX;

		if ( tNewFilter.m_bOpenLeft )
			tNewFilter.m_fMinValue = FLT_MIN;
	}

	dFilters.Add ( tNewFilter );

	return true;
}


static ESphAttr Json2AttrType ( const JsonObj_c & tJson )
{
	if ( tJson.IsInt() )	return SPH_ATTR_BIGINT;
	if ( tJson.IsDbl() )	return SPH_ATTR_FLOAT;
	if ( tJson.IsBool() )	return SPH_ATTR_BOOL;
	if ( tJson.IsStr() )	return SPH_ATTR_STRING;

	return SPH_ATTR_NONE;
}


static bool ConstructInFilter ( const JsonObj_c & tJson, CSphVector<CSphFilterSettings> & dFilters, CSphString & sError )
{
	JsonObj_c tColumn = GetFilterColumn ( tJson, sError );
	if ( !tColumn )
		return false;

	CSphFilterSettings tNewFilter;
	tNewFilter.m_sAttrName = tColumn.Name();
	sphColumnToLowercase ( const_cast<char *>( tNewFilter.m_sAttrName.cstr() ) );

	if ( !tColumn.IsArray() )
	{
		sError = "\"in\" filter should contain an array of values";
		return false;
	}

	if ( tColumn.Size() )
	{
		ESphAttr eValueType = Json2AttrType ( tColumn[0] );
		switch ( eValueType )
		{
		case SPH_ATTR_STRING:
			tNewFilter.m_eType = SPH_FILTER_STRING_LIST;
			break;

		case SPH_ATTR_FLOAT:
		case SPH_ATTR_NONE:
			sError = "\"in\" supports only integer, bool and string values";
			return false;

		default:
			tNewFilter.m_eType = SPH_FILTER_VALUES;
			break;
		}

		for ( const auto & i : tColumn )
		{
			ESphAttr eNewValueType = Json2AttrType(i);
			if ( eNewValueType!=eValueType )
			{
				sError = "all values in the \"in\" filter should have one type";
				return false;
			}

			if ( eValueType==SPH_ATTR_STRING )
				tNewFilter.m_dStrings.Add ( i.StrVal() );
			else
				tNewFilter.m_dValues.Add ( i.IntVal() );
		}
	}

	tNewFilter.m_dStrings.Uniq();
	tNewFilter.m_dValues.Uniq();

	dFilters.Add ( tNewFilter );

	return true;
}


static bool ConstructGeoFilter ( const JsonObj_c & tJson, CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphQueryItem> & dQueryItems, int & iQueryItemId, CSphString & sError, CSphString & sWarning )
{
	GeoDistInfo_c tGeoDist;
	if ( !tGeoDist.Parse ( tJson, true, sError, sWarning ) )
		return false;

	CSphQueryItem & tQueryItem = dQueryItems.Add();
	tQueryItem.m_sExpr = tGeoDist.BuildExprString();
	tQueryItem.m_sAlias.SetSprintf ( "%s%d", g_szFilter, iQueryItemId++ );

	CSphFilterSettings & tFilter = dFilters.Add();
	tFilter.m_sAttrName = tQueryItem.m_sAlias;
	tFilter.m_bOpenLeft = true;
	tFilter.m_bHasEqualMax = true;
	tFilter.m_fMaxValue = tGeoDist.GetDistance();
	tFilter.m_eType = SPH_FILTER_FLOATRANGE;

	return true;
}


static bool ConstructFilter ( const JsonObj_c & tJson, CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphQueryItem> & dQueryItems, int & iQueryItemId, CSphString & sError, CSphString & sWarning )
{
	if ( !IsFilter ( tJson ) )
		return true;

	CSphString sName = tJson.Name();
	if ( sName=="equals" )
		return ConstructEqualsFilter ( tJson, dFilters, sError );

	if ( sName=="range" )
		return ConstructRangeFilter ( tJson, dFilters, sError );

	if ( sName=="in" )
		return ConstructInFilter ( tJson, dFilters, sError );

	if ( sName=="geo_distance" )
		return ConstructGeoFilter ( tJson, dFilters, dQueryItems, iQueryItemId, sError, sWarning );

	sError.SetSprintf ( "unknown filter type: %s", sName.cstr() );
	return false;
}


static bool ConstructBoolNodeFilters ( const JsonObj_c & tClause, CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphQueryItem> & dQueryItems, int & iQueryItemId, CSphString & sError, CSphString & sWarning )
{
	if ( tClause.IsArray() )
	{
		for ( const auto & tObject : tClause )
		{
			if ( !tObject.IsObj() )
			{
				sError.SetSprintf ( "\"%s\" array value should be an object", tClause.Name() );
				return false;
			}

			JsonObj_c tItem = tObject[0];
			if ( !ConstructFilter ( tItem, dFilters, dQueryItems, iQueryItemId, sError, sWarning ) )
				return false;
		}
	} else if ( tClause.IsObj() )
	{
		JsonObj_c tItem = tClause[0];
		if ( !ConstructFilter ( tItem, dFilters, dQueryItems, iQueryItemId, sError, sWarning ) )
			return false;
	} else
	{
		sError.SetSprintf ( "\"%s\" value should be an object or an array", tClause.Name() );
		return false;
	}

	return true;
}


static bool ConstructBoolFilters ( const JsonObj_c & tBool, CSphQuery & tQuery, int & iQueryItemId, CSphString & sError, CSphString & sWarning )
{
	// non-recursive for now, maybe we should fix this later
	if ( !tBool.IsObj() )
	{
		sError = "\"bool\" value should be an object";
		return false;
	}

	CSphVector<CSphFilterSettings> dMust, dShould, dMustNot;
	CSphVector<CSphQueryItem> dMustQI, dShouldQI, dMustNotQI;

	for ( const auto & tClause : tBool )
	{
		CSphString sName = tClause.Name();

		if ( sName=="must" )
		{
			if ( !ConstructBoolNodeFilters ( tClause, dMust, dMustQI, iQueryItemId, sError, sWarning ) )
				return false;
		} else if ( sName=="should" )
		{
			if ( !ConstructBoolNodeFilters ( tClause, dShould, dShouldQI, iQueryItemId, sError, sWarning ) )
				return false;
		} else if ( sName=="must_not" )
		{
			if ( !ConstructBoolNodeFilters ( tClause, dMustNot, dMustNotQI, iQueryItemId, sError, sWarning ) )
				return false;
		} else
		{
			sError.SetSprintf ( "unknown bool query type: \"%s\"", sName.cstr() );
			return false;
		}
	}

	if ( dMustNot.GetLength() )
	{
		for ( auto & i : dMustNot )
		{
			i.m_bExclude = true;
			dMust.Add(i);
		}

		for ( auto & i : dMustNotQI )
			dMustQI.Add(i);
	}

	if ( dMust.GetLength() )
	{
		AddToSelectList ( tQuery, dMustQI );
		tQuery.m_dFilters.SwapData ( dMust );
		for ( const auto & i : dMustQI )
			tQuery.m_dItems.Add(i);

		return true;
	}

	if ( dShould.GetLength() )
	{
		AddToSelectList ( tQuery, dShouldQI );
		tQuery.m_dFilters.SwapData ( dShould );
		for ( const auto & i : dShouldQI )
			tQuery.m_dItems.Add(i);

		// need a filter tree
		FilterTreeItem_t & tTreeItem = tQuery.m_dFilterTree.Add();
		tTreeItem.m_iFilterItem = 0;
		int iRootNode = 0;

		ARRAY_FOREACH ( i, tQuery.m_dFilters )
		{
			int iNewFilterNodeId = tQuery.m_dFilterTree.GetLength();
			FilterTreeItem_t & tNewFilterNode = tQuery.m_dFilterTree.Add();
			tNewFilterNode.m_iFilterItem = i;

			int iNewOrNodeId = tQuery.m_dFilterTree.GetLength();
			FilterTreeItem_t & tNewOrNode = tQuery.m_dFilterTree.Add();
			tNewOrNode.m_bOr = true;
			tNewOrNode.m_iLeft = iRootNode;
			tNewOrNode.m_iRight = iNewFilterNodeId;

			iRootNode = iNewOrNodeId;
		}
	}

	return true;
}


static bool ConstructFilters ( const JsonObj_c & tJson, CSphQuery & tQuery, CSphString & sError, CSphString & sWarning )
{
	if ( !tJson )
		return false;

	CSphString sName = tJson.Name();
	if ( sName.IsEmpty() )
		return false;

	if ( sName!="query" )
	{
		sError.SetSprintf ( R"("query" expected, got %s)", sName.cstr() );
		return false;
	}

	int iQueryItemId = 0;

	JsonObj_c tBool = tJson.GetItem("bool");
	if ( tBool )
		return ConstructBoolFilters ( tBool, tQuery, iQueryItemId, sError, sWarning );

	for ( const auto & tChild : tJson )
		if ( IsFilter ( tChild ) )
		{
			int iFirstNewItem = tQuery.m_dItems.GetLength();
			if ( !ConstructFilter ( tChild, tQuery.m_dFilters, tQuery.m_dItems, iQueryItemId, sError, sWarning ) )
				return false;

			AddToSelectList ( tQuery, tQuery.m_dItems, iFirstNewItem );

			// handle only the first filter in this case
			break;
		}

	return true;
}


bool ParseJsonQueryFilters ( const JsonObj_c & tJson, CSphQuery & tQuery, CSphString & sError, CSphString & sWarning )
{
	if ( tJson && !tJson.IsObj() )
	{	
		sError = "\"query\" property value should be an object";
		return false;
	}

	CSphQueryItem & tItem = tQuery.m_dItems.Add();
	tItem.m_sExpr = "*";
	tItem.m_sAlias = "*";
	tQuery.m_sSelect = "*";

	// we need to know if the query is fullscan before re-parsing it to build AST tree
	// so we need to do some preprocessing here
	bool bFullscan = !tJson || ( tJson.Size()==1 && tJson.HasItem("match_all") );

	if ( !bFullscan )
		tQuery.m_sQuery = tJson.AsString();

	// because of the way sphinxql parsing was implemented
	// we need to parse our query and extract filters now
	// and parse the rest of the query later
	if ( tJson )
	{
		if ( !ConstructFilters ( tJson, tQuery, sError, sWarning ) )
			return false;
	}

	return true;
}
