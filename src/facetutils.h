//
// Shared facet helpers for filter-scope rewriting and bucket status metadata.
//
#ifndef _facetutils_
#define _facetutils_

#include "sphinx.h"

namespace facet
{
inline bool AttrNameInList ( const StrVec_t & dAttrs, const CSphString & sAttr )
{
	for ( const auto & sCandidate : dAttrs )
		if ( !strcasecmp ( sCandidate.cstr(), sAttr.cstr() ) )
			return true;

	return false;
}

inline bool IsSelectedFilterType ( const CSphFilterSettings & tFilter )
{
	if ( tFilter.m_bExclude )
		return false;

	switch ( tFilter.m_eType )
	{
	case SPH_FILTER_VALUES:
	case SPH_FILTER_STRING:
	case SPH_FILTER_STRING_LIST:
		return true;
	default:
		return false;
	}
}

inline bool ShouldRewriteFilters ( const CSphQuery & tHeadQuery, const CSphQuery & tFacetQuery )
{
	if ( tFacetQuery.m_eFacetFilterClause==FacetFilterClause_e::INCLUDE || tFacetQuery.m_eFacetFilterClause==FacetFilterClause_e::EXCLUDE )
		return true;

	if ( tFacetQuery.m_bFacetFilterModeExplicit )
		return tFacetQuery.m_eFacetFilterMode!=FacetFilterMode_e::FACET_FILTER_STRICT;

	return tHeadQuery.m_eFacetFilterMode!=FacetFilterMode_e::FACET_FILTER_STRICT;
}

inline bool CopyFilters ( const CSphQuery & tHeadQuery, CSphQuery & tFacetQuery, CSphString & sError, bool bUseOwnExclusionRuleInNoneClause )
{
	if ( tFacetQuery.m_eFacetFilterClause==FacetFilterClause_e::ALL || !ShouldRewriteFilters ( tHeadQuery, tFacetQuery ) )
	{
		ARRAY_FOREACH ( k, tHeadQuery.m_dFilters )
			tFacetQuery.m_dFilters.Add ( tHeadQuery.m_dFilters[k] );
		ARRAY_FOREACH ( k, tHeadQuery.m_dFilterTree )
			tFacetQuery.m_dFilterTree.Add ( tHeadQuery.m_dFilterTree[k] );
		return true;
	}

	if ( !tHeadQuery.m_dFilterTree.IsEmpty() )
	{
		sError = "facet-local filter scope supports conjunction-only attribute filters in V1";
		return false;
	}

	for ( const auto & tFilter : tHeadQuery.m_dFilters )
	{
		bool bKeep = true;
		switch ( tFacetQuery.m_eFacetFilterClause )
		{
		case FacetFilterClause_e::INCLUDE:
			bKeep = AttrNameInList ( tFacetQuery.m_dFacetFilterAttrs, tFilter.m_sAttrName );
			break;
		case FacetFilterClause_e::EXCLUDE:
			bKeep = !AttrNameInList ( tFacetQuery.m_dFacetFilterAttrs, tFilter.m_sAttrName );
			break;
		case FacetFilterClause_e::NONE:
			bKeep = bUseOwnExclusionRuleInNoneClause ? !AttrNameInList ( tFacetQuery.m_dFacetOwnFilterAttrs, tFilter.m_sAttrName ) : false;
			break;
		case FacetFilterClause_e::ALL:
			bKeep = true;
			break;
		}

		if ( bKeep )
			tFacetQuery.m_dFilters.Add ( tFilter );
	}

	return true;
}

inline bool MatchStringFilterValue ( const CSphMatch & tMatch, const CSphColumnInfo & tAttr, const CSphString & sValue )
{
	if ( tAttr.m_eAttrType!=SPH_ATTR_STRINGPTR )
		return false;

	auto * pString = (const BYTE*) tMatch.GetAttr ( tAttr.m_tLocator );
	auto dString = sphUnpackPtrAttr ( pString );
	int iLen = dString.second;
	if ( iLen>1 && dString.first[iLen-2]=='\0' )
		iLen -= 2;

	return iLen==(int)sValue.Length() && !memcmp ( dString.first, sValue.cstr(), iLen );
}

inline bool MatchBucketFilter ( const CSphMatch & tMatch, const ISphSchema & tSchema, const CSphFilterSettings & tFilter )
{
	if ( tFilter.m_bExclude )
		return false;

	const CSphColumnInfo * pAttr = tSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
	if ( !pAttr )
		return false;

	switch ( tFilter.m_eType )
	{
	case SPH_FILTER_VALUES:
	{
		SphAttr_t uValue = tMatch.GetAttr ( pAttr->m_tLocator );
		for ( auto uFilterValue : tFilter.GetValues() )
			if ( uValue==uFilterValue )
				return true;
		return false;
	}
	case SPH_FILTER_STRING:
		return tFilter.m_dStrings.GetLength()==1 && MatchStringFilterValue ( tMatch, *pAttr, tFilter.m_dStrings[0] );
	case SPH_FILTER_STRING_LIST:
		for ( const auto & sFilterValue : tFilter.m_dStrings )
			if ( MatchStringFilterValue ( tMatch, *pAttr, sFilterValue ) )
				return true;
		return false;
	default:
		return false;
	}
}

inline const char * GetBucketStatus ( const CSphMatch & tMatch, const ISphSchema & tSchema, const CSphVector<CSphFilterSettings> * pSelectedFilters, const CSphVector<CSphFilterSettings> * pAvailableFilters=nullptr )
{
	bool bSelected = pSelectedFilters && !pSelectedFilters->IsEmpty();
	if ( bSelected )
		for ( const auto & tFilter : *pSelectedFilters )
			if ( !MatchBucketFilter ( tMatch, tSchema, tFilter ) )
			{
				bSelected = false;
				break;
			}

	if ( bSelected )
		return "selected";

	if ( pAvailableFilters )
		for ( const auto & tFilter : *pAvailableFilters )
			if ( MatchBucketFilter ( tMatch, tSchema, tFilter ) )
				return "available";

	return pAvailableFilters ? "unavailable" : "available";
}

inline const CSphVector<CSphFilterSettings> * CollectSelectedFiltersForAttrs ( const CSphVector<CSphFilterSettings> & dFilters, const StrVec_t & dAttrs, CSphVector<CSphFilterSettings> & dSelected )
{
	for ( const auto & tFilter : dFilters )
		if ( IsSelectedFilterType ( tFilter ) && AttrNameInList ( dAttrs, tFilter.m_sAttrName ) )
			dSelected.Add ( tFilter );

	return dSelected.IsEmpty() ? nullptr : &dSelected;
}
}

#endif
