//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "jsonqueryfilter.h"

#include "sphinxint.h"
#include <time.h>

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

	if ( sName=="exists" )
		return true;

	return false;
}


static void AddToSelectList ( CSphQuery & tQuery, const CSphVector<CSphQueryItem> & dItems, int iFirstItem = 0 )
{
	for ( int i = iFirstItem; i < dItems.GetLength(); i++ )
		tQuery.m_sSelect.SetSprintf ( "%s, %s as %s", tQuery.m_sSelect.cstr(), dItems[i].m_sExpr.cstr(), dItems[i].m_sAlias.cstr() );
}


static ESphAttr Json2AttrType ( const JsonObj_c & tJson )
{
	if ( tJson.IsInt() )	return SPH_ATTR_BIGINT;
	if ( tJson.IsDbl() )	return SPH_ATTR_FLOAT;
	if ( tJson.IsBool() )	return SPH_ATTR_BOOL;
	if ( tJson.IsStr() )	return SPH_ATTR_STRING;

	return SPH_ATTR_NONE;
}


struct FilterTreeNode_t
{
	std::unique_ptr<FilterTreeNode_t>	m_pLeft;
	std::unique_ptr<FilterTreeNode_t>	m_pRight;
	std::unique_ptr<CSphFilterSettings> m_pFilter;
	bool	m_bOr = false;
};


class FilterTreeConstructor_c
{
public:
			FilterTreeConstructor_c ( CSphQuery & tQuery, CSphString & sError, CSphString & sWarning );

	bool	Parse ( const JsonObj_c & tObj );

private:
	CSphQuery &		m_tQuery;
	CSphString &	m_sError;
	CSphString &	m_sWarning;
	int				m_iQueryItemId = 0;

	std::pair<bool, std::unique_ptr<FilterTreeNode_t>> ConstructBoolFilters ( const JsonObj_c & tBool );
	std::pair<bool, std::unique_ptr<FilterTreeNode_t>> ConstructPlainFilters ( const JsonObj_c & tObj );
	std::pair<bool, std::unique_ptr<FilterTreeNode_t>> ConstructBoolNodeFilters ( const JsonObj_c & tClause, CSphVector<CSphQueryItem> & dQueryItems, bool bOr );

	std::unique_ptr<FilterTreeNode_t>	ConstructInFilter ( const JsonObj_c & tJson );
	std::unique_ptr<FilterTreeNode_t>	ConstructGeoFilter ( const JsonObj_c & tJson, CSphVector<CSphQueryItem> & dQueryItems );
	std::unique_ptr<FilterTreeNode_t>	ConstructFilter ( const JsonObj_c & tJson, CSphVector<CSphQueryItem> & dQueryItems );
	std::unique_ptr<FilterTreeNode_t>	ConstructEqualsFilter ( const JsonObj_c & tJson );
	std::unique_ptr<FilterTreeNode_t>	ConstructRangeFilter ( const JsonObj_c & tJson );
	std::unique_ptr<FilterTreeNode_t>	ConstructExistsFilter ( const JsonObj_c & tJson );

	JsonObj_c GetFilterColumn ( const JsonObj_c & tJson );
};


template <typename T>
static void WalkTree ( std::unique_ptr<FilterTreeNode_t> & pRoot, T && fnAction )
{
	if ( !pRoot )
		return;

	fnAction(pRoot);
	WalkTree ( pRoot->m_pLeft, fnAction );
	WalkTree ( pRoot->m_pRight, fnAction );
}


static int CreateFilterTree ( std::unique_ptr<FilterTreeNode_t> & pRoot, CSphVector<CSphFilterSettings> & dFilters, CSphVector<FilterTreeItem_t> & dFilterTree )
{
	if ( !pRoot )
		return -1;

	int iLeft = CreateFilterTree ( pRoot->m_pLeft, dFilters, dFilterTree );
	int iRight = CreateFilterTree ( pRoot->m_pRight, dFilters, dFilterTree );

	FilterTreeItem_t & tNew = dFilterTree.Add();
	tNew.m_bOr = pRoot->m_bOr;
	if ( pRoot->m_pFilter )
	{
		tNew.m_iFilterItem = dFilters.GetLength();
		dFilters.Add ( std::move ( *pRoot->m_pFilter ) );
	}
	else
	{
		tNew.m_iLeft = iLeft;
		tNew.m_iRight = iRight;
	}

	return dFilterTree.GetLength()-1;
}


static void ConcatTrees ( std::unique_ptr<FilterTreeNode_t> & pLeft, std::unique_ptr<FilterTreeNode_t> & pRight )
{
	auto pRoot = std::make_unique<FilterTreeNode_t>();
	pRoot->m_pLeft  = std::move(pLeft);
	pRoot->m_pRight = std::move(pRight);
	pLeft = std::move(pRoot);
}


FilterTreeConstructor_c::FilterTreeConstructor_c ( CSphQuery & tQuery, CSphString & sError, CSphString & sWarning )
	: m_tQuery ( tQuery )
	, m_sError ( sError )
	, m_sWarning ( sWarning )
{}


bool FilterTreeConstructor_c::Parse ( const JsonObj_c & tObj )
{
	bool bOk;
	std::unique_ptr<FilterTreeNode_t> pRoot;

	JsonObj_c tBool = tObj.GetItem("bool");
	if ( tBool )
		std::tie ( bOk, pRoot ) = ConstructBoolFilters(tBool);
	else
		std::tie ( bOk, pRoot ) = ConstructPlainFilters(tObj);

	if ( !bOk )
		return false;

	if ( !pRoot )
		return true;

	bool bAllAnd = true;
	WalkTree ( pRoot, [&bAllAnd]( auto & pNode ){ bAllAnd &= !pNode->m_bOr; } );

	if ( bAllAnd )
	{
		// no tree; collect filters from the tree and add them to the query
		WalkTree ( pRoot, [this]( auto & pNode ){ if ( pNode->m_pFilter ) m_tQuery.m_dFilters.Add ( std::move ( *pNode->m_pFilter ) ); } );
	}
	else
		CreateFilterTree ( pRoot, m_tQuery.m_dFilters, m_tQuery.m_dFilterTree );

	return true;
}


std::pair<bool, std::unique_ptr<FilterTreeNode_t>> FilterTreeConstructor_c::ConstructPlainFilters ( const JsonObj_c & tObj )
{
	for ( const auto & tChild : tObj )
		if ( IsFilter(tChild) )
		{
			int iFirstNewItem = m_tQuery.m_dItems.GetLength();

			auto pFilter = ConstructFilter ( tChild, m_tQuery.m_dItems );
			if ( !pFilter )
				return { false, nullptr };

			AddToSelectList ( m_tQuery, m_tQuery.m_dItems, iFirstNewItem );

			// handle only the first filter in this case	
			return { true, std::move(pFilter) };
		}

	return { true, nullptr };
}


std::pair<bool, std::unique_ptr<FilterTreeNode_t>> FilterTreeConstructor_c::ConstructBoolFilters ( const JsonObj_c & tBool )
{
	if ( !tBool.IsObj() )
	{
		m_sError = "\"bool\" value should be an object";
		return { false, nullptr };
	}

	bool bOk = false;
	std::unique_ptr<FilterTreeNode_t> pMustTreeRoot, pShouldTreeRoot, pMustNotTreeRoot, pFilterTreeRoot;
	CSphVector<CSphQueryItem> dMustQI, dShouldQI, dMustNotQI;

	for ( const auto & tClause : tBool )
	{
		CSphString sName = tClause.Name();

		if ( sName=="must" )
		{
			std::tie ( bOk, pMustTreeRoot ) = ConstructBoolNodeFilters ( tClause, dMustQI, false );
			if ( !bOk )
				return { false, nullptr };
		} 
		else if ( sName=="should" )
		{
			std::tie ( bOk, pShouldTreeRoot ) = ConstructBoolNodeFilters ( tClause, dShouldQI, true );
			if ( !bOk )
				return { false, nullptr };
		}
		else if ( sName=="must_not" )
		{
			std::tie ( bOk, pMustNotTreeRoot ) = ConstructBoolNodeFilters ( tClause, dMustNotQI, false );
			if ( !bOk )
				return { false, nullptr };
		} else if ( sName=="filter" )
		{
			std::tie ( bOk, pFilterTreeRoot ) = ConstructBoolNodeFilters ( tClause, dMustQI, false );
			if ( !bOk )
				return { false, nullptr };
		} else if ( sName=="minimum_should_match" ) // FIXME!!! add to should as option
		{
			continue;
		} else
		{
			m_sError.SetSprintf ( "unknown bool query type: \"%s\"", sName.cstr() );
			return { false, nullptr };
		}
	}

	if ( pFilterTreeRoot )
	{
		if ( pMustTreeRoot )
			ConcatTrees ( pMustTreeRoot, pFilterTreeRoot );
		else
			pMustTreeRoot = std::move ( pFilterTreeRoot );
	}

	if ( pMustNotTreeRoot )
	{
		// fixme! this may not work as expected; better add a NOT node
		WalkTree ( pMustNotTreeRoot, []( auto & pNode ) { if ( pNode->m_pFilter ) pNode->m_pFilter->m_bExclude = !pNode->m_pFilter->m_bExclude; } );

		for ( auto & i : dMustNotQI )
			dMustQI.Add(i);

		if ( pMustTreeRoot )
			ConcatTrees ( pMustTreeRoot, pMustNotTreeRoot );
		else
			pMustTreeRoot = std::move ( pMustNotTreeRoot );
	}

	if ( pMustTreeRoot )
	{
		AddToSelectList ( m_tQuery, dMustQI );

		for ( const auto & i : dMustQI )
			m_tQuery.m_dItems.Add(i);

		return { true, std::move(pMustTreeRoot) };
	}

	if ( pShouldTreeRoot )
	{
		AddToSelectList ( m_tQuery, dShouldQI );

		for ( const auto & i : dShouldQI )
			m_tQuery.m_dItems.Add(i);

		return { true, std::move(pShouldTreeRoot) };
	}

	return { true, nullptr };
}


static std::unique_ptr<FilterTreeNode_t> ConcatFilterTreeItems ( std::vector<std::unique_ptr<FilterTreeNode_t>> & dAdded, int iStart, bool bOr )
{
	if ( dAdded.empty() )
		return nullptr;

	if ( iStart==dAdded.size()-1 )
		return std::move ( dAdded[iStart] );

	auto pRoot = std::make_unique<FilterTreeNode_t>();
	pRoot->m_pLeft = std::move ( dAdded[iStart] );
	pRoot->m_pRight = ConcatFilterTreeItems ( dAdded, iStart+1, bOr );
	pRoot->m_bOr = bOr;

	return pRoot;
}


std::pair<bool, std::unique_ptr<FilterTreeNode_t>> FilterTreeConstructor_c::ConstructBoolNodeFilters ( const JsonObj_c & tClause, CSphVector<CSphQueryItem> & dQueryItems, bool bOr )
{
	if ( tClause.IsArray() )
	{
		std::vector<std::unique_ptr<FilterTreeNode_t>> dAdded;

		for ( const auto & tObject : tClause )
		{
			if ( !tObject.IsObj() )
			{
				m_sError.SetSprintf ( "\"%s\" array value should be an object", tClause.Name() );
				return {false, nullptr};
			}

			JsonObj_c tItem = tObject[0];
			if ( !tItem )
				continue;

			CSphString sName = tItem.Name();
			if ( sName=="bool" )
			{
				auto tRes = ConstructBoolFilters(tItem);
				if ( !tRes.first )
					return {false, nullptr};

				if ( tRes.second )
					dAdded.push_back ( std::move(tRes.second) );

			} else if ( IsFilter(tItem) )
			{
				auto pFilter = ConstructFilter ( tItem, dQueryItems );
				if ( !pFilter )
					return {false, nullptr};

				dAdded.push_back ( std::move(pFilter) );
			}
		}

		if ( dAdded.empty() )
			return {true, nullptr};

		return { true, ConcatFilterTreeItems ( dAdded, 0, bOr ) };
	}
	else if ( tClause.IsObj() )
	{
		JsonObj_c tItem = tClause[0];
		if ( IsFilter(tItem) )
		{
			auto pFilter = ConstructFilter ( tItem, dQueryItems );
			if ( !pFilter )
				return {false, nullptr};

			return { true, std::move(pFilter) };
		}

		return {true, nullptr};
	}

	m_sError.SetSprintf ( "\"%s\" value should be an object or an array", tClause.Name() );
	return {false, nullptr};
}


JsonObj_c FilterTreeConstructor_c::GetFilterColumn ( const JsonObj_c & tJson )
{
	if ( !tJson.IsObj() )
	{
		m_sError = "filter should be an object";
		return JsonNull;
	}

	if ( tJson.Size()!=1 )
	{
		m_sError = "filter should have only one element";
		return JsonNull;
	}

	JsonObj_c tColumn = tJson[0];
	if ( !tColumn )
	{
		m_sError = "empty filter found";
		return JsonNull;
	}

	return tColumn;
}

static void SetMvaFilterFunc ( CSphFilterSettings & tFilter );

std::unique_ptr<FilterTreeNode_t> FilterTreeConstructor_c::ConstructInFilter ( const JsonObj_c & tJson )
{
	JsonObj_c tColumn = GetFilterColumn(tJson);
	if ( !tColumn )
		return nullptr;

	if ( !tColumn.IsArray() )
	{
		m_sError = "\"in\" filter should contain an array of values";
		return nullptr;
	}

	auto pFilterNode = std::make_unique<FilterTreeNode_t>();
	pFilterNode->m_pFilter = std::make_unique<CSphFilterSettings>();
	auto & tFilter = *pFilterNode->m_pFilter;

	tFilter.m_sAttrName = tColumn.Name();
	sphColumnToLowercase ( const_cast<char *>( tFilter.m_sAttrName.cstr() ) );
	SetMvaFilterFunc ( tFilter );

	if ( tColumn.Size() )
	{
		ESphAttr eValueType = Json2AttrType ( tColumn[0] );
		switch ( eValueType )
		{
		case SPH_ATTR_STRING:
			tFilter.m_eType = SPH_FILTER_STRING_LIST;
			break;

		case SPH_ATTR_FLOAT:
		case SPH_ATTR_NONE:
			m_sError = "\"in\" supports only integer, bool and string values";
			return nullptr;

		default:
			tFilter.m_eType = SPH_FILTER_VALUES;
			break;
		}

		for ( const auto & i : tColumn )
		{
			ESphAttr eNewValueType = Json2AttrType(i);
			if ( eNewValueType!=eValueType )
			{
				m_sError = "all values in the \"in\" filter should have one type";
				return nullptr;
			}

			if ( eValueType==SPH_ATTR_STRING )
				tFilter.m_dStrings.Add ( i.StrVal() );
			else
				tFilter.m_dValues.Add ( i.IntVal() );
		}
	}

	tFilter.m_dStrings.Uniq();
	tFilter.m_dValues.Uniq();

	return pFilterNode;
}


std::unique_ptr<FilterTreeNode_t> FilterTreeConstructor_c::ConstructGeoFilter ( const JsonObj_c & tJson, CSphVector<CSphQueryItem> & dQueryItems )
{
	GeoDistInfo_c tGeoDist;
	if ( !tGeoDist.Parse ( tJson, true, m_sError, m_sWarning ) )
		return nullptr;

	CSphQueryItem & tQueryItem = dQueryItems.Add();
	tQueryItem.m_sExpr = tGeoDist.BuildExprString();
	tQueryItem.m_sAlias.SetSprintf ( "%s%d", g_szFilter, m_iQueryItemId++ );

	auto pFilterNode = std::make_unique<FilterTreeNode_t>();
	pFilterNode->m_pFilter = std::make_unique<CSphFilterSettings>();
	auto & tFilter = *pFilterNode->m_pFilter;

	tFilter.m_sAttrName = tQueryItem.m_sAlias;
	tFilter.m_bOpenLeft = true;
	tFilter.m_bHasEqualMax = true;
	tFilter.m_fMaxValue = tGeoDist.GetDistance();
	tFilter.m_eType = SPH_FILTER_FLOATRANGE;

	return pFilterNode;
}


std::unique_ptr<FilterTreeNode_t> FilterTreeConstructor_c::ConstructExistsFilter ( const JsonObj_c & tJson )
{
	JsonObj_c tColumn = GetFilterColumn(tJson);
	if ( !tColumn )
		return nullptr;

	auto pFilterNode = std::make_unique<FilterTreeNode_t>();
	pFilterNode->m_pFilter = std::make_unique<CSphFilterSettings>();
	auto & tFilter = *pFilterNode->m_pFilter;

	const char sFieldName[] = "field";
	bool bFieldType = ( strncmp ( tColumn.Name(), sFieldName, sizeof(sFieldName) )==0 );
	
	// that is non standard extension from compart mode filter fixup
	const char sAttrName[] = "attr";
	bool bAttrType = false;
	if ( !bFieldType )
		bAttrType = ( strncmp ( tColumn.Name(), sAttrName, sizeof(sAttrName) )==0 );

	if ( !bFieldType && !bAttrType )
	{
		m_sError = "exists filter should have only one field element";
		return nullptr;
	}

	if ( !tColumn.IsStr() )
	{
		m_sError = "exists filter expects string value";
		return nullptr;
	}

	if ( bFieldType )
	{
		tFilter.m_sAttrName.SetSprintf ( "%s_len", tColumn.SzVal() );
		tFilter.m_eType = SPH_FILTER_VALUES;
		tFilter.m_dValues.Add ( 0 );
		tFilter.m_bExclude = true;
	} else
	{
		tFilter.m_sAttrName.SetSprintf ( "length(%s)!=0", tColumn.SzVal() );
		tFilter.m_eType = SPH_FILTER_EXPRESSION;
	}


	return pFilterNode;
}

std::unique_ptr<FilterTreeNode_t> FilterTreeConstructor_c::ConstructFilter ( const JsonObj_c & tJson, CSphVector<CSphQueryItem> & dQueryItems )
{
	CSphString sName = tJson.Name();
	if ( sName=="equals" )
		return ConstructEqualsFilter(tJson);

	if ( sName=="range" )
		return ConstructRangeFilter(tJson);

	if ( sName=="in" )
		return ConstructInFilter(tJson);

	if ( sName=="geo_distance" )
		return ConstructGeoFilter ( tJson, dQueryItems );

	if ( sName=="exists" )
		return ConstructExistsFilter ( tJson );

	m_sError.SetSprintf ( "unknown filter type: %s", sName.cstr() );
	return nullptr;
}

void SetMvaFilterFunc ( CSphFilterSettings & tFilter )
{
	bool bAll = tFilter.m_sAttrName.Begins ( "all(" );
	bool bAny = tFilter.m_sAttrName.Begins ( "any(" );
	if ( !bAll && !bAny )
		return;

	int iLen = tFilter.m_sAttrName.Length();
	if ( iLen<5 )
		return;

	CSphString sName;
	sName.SetBinary ( tFilter.m_sAttrName.cstr()+4, iLen-5 );
	tFilter.m_sAttrName.Swap ( sName );

	tFilter.m_eMvaFunc = ( bAll ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY );
}

std::unique_ptr<FilterTreeNode_t> FilterTreeConstructor_c::ConstructEqualsFilter ( const JsonObj_c & tJson )
{
	JsonObj_c tColumn = GetFilterColumn(tJson);
	if ( !tColumn )
		return nullptr;

	if ( !tColumn.IsNum() && !tColumn.IsStr() && !tColumn.IsArray() )
	{
		m_sError = "\"equals\" filter expects numeric or string values";
		return nullptr;
	}

	auto pFilterNode = std::make_unique<FilterTreeNode_t>();
	pFilterNode->m_pFilter = std::make_unique<CSphFilterSettings>();
	auto & tFilter = *pFilterNode->m_pFilter;

	tFilter.m_sAttrName = tColumn.Name();
	sphColumnToLowercase ( const_cast<char *>( tFilter.m_sAttrName.cstr() ) );
	SetMvaFilterFunc ( tFilter );

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
	} else if ( tColumn.IsArray() )
	{
		int iSize = tColumn.Size();
		if ( iSize )
		{
			if ( tColumn[0].IsStr() )
			{
				tFilter.m_eType = SPH_FILTER_STRING_LIST;
				tFilter.m_dStrings.Resize ( iSize );
			} else
			{
				tFilter.m_eType = SPH_FILTER_VALUES;
				tFilter.m_dValues.Resize ( iSize );
			}
		}

		for ( int i=0; i<iSize; i++ )
		{
			const JsonObj_c tVal = tColumn[i];
			if ( tFilter.m_eType==SPH_FILTER_STRING_LIST )
			{
				tFilter.m_dStrings[i] = tVal.StrVal();
			} else
			{
				tFilter.m_dValues[i] = tVal.IntVal();
			}
		}
	} else
	{
		tFilter.m_eType = SPH_FILTER_STRING;
		tFilter.m_dStrings.Add ( tColumn.StrVal() );
		tFilter.m_bExclude = false;
	}

	return pFilterNode;
}


std::unique_ptr<FilterTreeNode_t> FilterTreeConstructor_c::ConstructRangeFilter ( const JsonObj_c & tJson )
{
	JsonObj_c tColumn = GetFilterColumn(tJson);
	if ( !tColumn )
		return nullptr;

	auto pFilterNode = std::make_unique<FilterTreeNode_t>();
	pFilterNode->m_pFilter = std::make_unique<CSphFilterSettings>();
	auto & tFilter = *pFilterNode->m_pFilter;

	tFilter.m_sAttrName = tColumn.Name();
	sphColumnToLowercase ( const_cast<char *>( tFilter.m_sAttrName.cstr() ) );

	tFilter.m_bHasEqualMin = false;
	tFilter.m_bHasEqualMax = false;

	JsonObj_c tLess = tColumn.GetItem("lt");
	if ( !tLess )
	{
		tLess = tColumn.GetItem("lte");
		tFilter.m_bHasEqualMax = tLess;
	}

	JsonObj_c tGreater = tColumn.GetItem("gt");
	if ( !tGreater )
	{
		tGreater = tColumn.GetItem("gte");
		tFilter.m_bHasEqualMin = tGreater;
	}

	bool bLess = tLess;
	bool bGreater = tGreater;

	if ( !bLess && !bGreater )
	{
		m_sError = "empty filter found";
		return nullptr;
	}

	int iLessVal = -1;
	int iGreaterVal = -1;
	if ( ( bLess && !tLess.IsNum() ) || ( bGreater && !tGreater.IsNum() ) )
	{
		JsonObj_c tDateFormat = tColumn.GetStrItem ( "format", m_sError, true );
		if ( tDateFormat && tDateFormat.StrVal()=="strict_date_optional_time" )
		{
			if ( bLess )
				iLessVal = GetUTC ( tLess.StrVal(), CompatDateFormat() );
			if ( bGreater )
				iGreaterVal = GetUTC ( tGreater.StrVal(), CompatDateFormat() );
		}

		if ( ( bLess && iLessVal==-1 && tLess.IsStr() && tLess.SzVal() && strcmp ( tLess.SzVal(), "now" )==0 )
			|| ( bGreater && iGreaterVal==-1  && tGreater.IsStr() && tGreater.SzVal() && strcmp ( tGreater.SzVal(), "now" )==0 ) )
		{
			if ( bLess && iLessVal==-1 )
				iLessVal = (int) time ( nullptr );
			if ( bGreater && iGreaterVal )
				iGreaterVal = (int) time ( nullptr );
		}

		if ( ( bLess && iLessVal==-1 ) || ( bGreater && iGreaterVal==-1) )
		{
			m_sError = "range filter expects numeric values";
			return nullptr;
		}
	} else
	{
		if ( bLess && tLess.IsInt() )
			iLessVal = tLess.IntVal();
		if ( bGreater && tGreater.IntVal() )
			iGreaterVal = tGreater.IntVal();
	}

	bool bIntFilter = ( ( !bLess || tLess.IsInt() || tLess.IsStr() ) && ( !bGreater || tGreater.IsInt() || tGreater.IsStr() ) );

	if ( bLess )
	{
		if ( bIntFilter )
			tFilter.m_iMaxValue = iLessVal;
		else
			tFilter.m_fMaxValue = tLess.FltVal();

		tFilter.m_bOpenLeft = !bGreater;
	}

	if ( bGreater )
	{
		if ( bIntFilter )
			tFilter.m_iMinValue = iGreaterVal;
		else
			tFilter.m_fMinValue = tGreater.FltVal();

		tFilter.m_bOpenRight = !bLess;
	}

	tFilter.m_eType = bIntFilter ? SPH_FILTER_RANGE : SPH_FILTER_FLOATRANGE;

	// float filters don't support open ranges
	if ( !bIntFilter )
	{
		if ( tFilter.m_bOpenRight )
		{
			tFilter.m_fMaxValue = FLT_MAX;
			tFilter.m_bHasEqualMax = true;
		}

		if ( tFilter.m_bOpenLeft )
		{
			tFilter.m_fMinValue = -FLT_MAX;
			tFilter.m_bHasEqualMin = true;
		}
	}

	return pFilterNode;
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

	FilterTreeConstructor_c tTreeConstructor ( tQuery, sError, sWarning );
	return tTreeConstructor.Parse(tJson);
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
