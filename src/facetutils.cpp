//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "searchdaemon.h"
#include "queuecreator.h"
#include "facetutils.h"
#include "std/fnv64.h"

namespace facet
{
void FacetBucketSet_t::Reset()
{
	m_sAttr = "";
	m_dHashes.Resize ( 0 );
}

bool FacetStatusSources_t::HasStatus() const
{
	return m_pSelectedFilters || m_pSelectedBuckets || m_pAvailableBuckets;
}

bool AttrNameInList ( const StrVec_t & dAttrs, const CSphString & sAttr )
{
	return dAttrs.any_of ( [&sAttr] ( const auto & sCandidate ) { return sCandidate==sAttr; } );
}

bool IsSelectedFilterType ( const CSphFilterSettings & tFilter )
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

bool IsJsonFacetStatusBucket ( Aggr_e eAggr )
{
	switch ( eAggr )
	{
	case Aggr_e::NONE:
	case Aggr_e::HISTOGRAM:
	case Aggr_e::DATE_HISTOGRAM:
	case Aggr_e::RANGE:
	case Aggr_e::DATE_RANGE:
		return true;
	default:
		return false;
	}
}

bool HasExcludedFilter ( const CSphVector<CSphFilterSettings> & dFilters )
{
	return dFilters.any_of ( [] ( const auto & tFilter ) { return tFilter.m_bExclude; } );
}

bool HasLocalFilterMode ( const FacetFilterTrait_t & tTrait )
{
	return tTrait.m_tMode.has_value();
}

FacetFilterMode_e GetDefaultFilterMode ( const FacetFilterTrait_t & tTrait )
{
	return tTrait.m_tMode.value_or ( FacetFilterMode_e::Strict );
}

FacetFilterMode_e GetFilterMode ( const CSphQuery & tHeadQuery, const FacetFilterTrait_t & tLocalTrait )
{
	return tLocalTrait.m_tMode.value_or ( GetDefaultFilterMode ( tHeadQuery.m_tFacetFilter ) );
}

FacetFilterMode_e GetFilterMode ( const CSphQuery & tHeadQuery, const CSphQuery & tFacetQuery )
{
	return GetFilterMode ( tHeadQuery, tFacetQuery.m_tFacetFilter );
}

bool ShouldRewriteFilters ( const CSphQuery & tHeadQuery, const CSphQuery & tFacetQuery )
{
	if ( tFacetQuery.m_tFacetFilter.m_eClause==FacetFilterClause_e::Include || tFacetQuery.m_tFacetFilter.m_eClause==FacetFilterClause_e::Exclude )
		return true;

	if ( HasLocalFilterMode ( tFacetQuery.m_tFacetFilter ) )
		return GetFilterMode ( tHeadQuery, tFacetQuery )!=FacetFilterMode_e::Strict;

	return GetDefaultFilterMode ( tHeadQuery.m_tFacetFilter )!=FacetFilterMode_e::Strict;
}

bool CopyFilters ( const CSphQuery & tHeadQuery, CSphQuery & tFacetQuery, CSphString & sError, bool bUseOwnExclusionRuleInNoneClause )
{
	if ( tFacetQuery.m_tFacetFilter.m_eClause==FacetFilterClause_e::All || !ShouldRewriteFilters ( tHeadQuery, tFacetQuery ) )
	{
		tFacetQuery.m_dFilters.Append ( tHeadQuery.m_dFilters );
		tFacetQuery.m_dFilterTree.Append ( tHeadQuery.m_dFilterTree );
		return true;
	}

	if ( !tHeadQuery.m_dFilterTree.IsEmpty() )
	{
		sError = "facet-local filter scope supports conjunction-only attribute filters";
		return false;
	}

	if ( HasExcludedFilter ( tHeadQuery.m_dFilters ) )
	{
		sError = "facet-local filter scope supports conjunction-only attribute filters";
		return false;
	}

	for ( const auto & tFilter : tHeadQuery.m_dFilters )
	{
		bool bKeep = true;
		switch ( tFacetQuery.m_tFacetFilter.m_eClause )
		{
		case FacetFilterClause_e::Include:
			bKeep = AttrNameInList ( tFacetQuery.m_tFacetFilter.m_dAttrs, tFilter.m_sAttrName );
			break;
		case FacetFilterClause_e::Exclude:
			bKeep = !AttrNameInList ( tFacetQuery.m_tFacetFilter.m_dAttrs, tFilter.m_sAttrName );
			break;
		case FacetFilterClause_e::None:
			bKeep = bUseOwnExclusionRuleInNoneClause ? !AttrNameInList ( tFacetQuery.m_dFacetOwnFilterAttrs, tFilter.m_sAttrName ) : false;
			break;
		case FacetFilterClause_e::All:
			bKeep = true;
			break;
		}

		if ( bKeep )
			tFacetQuery.m_dFilters.Add ( tFilter );
	}

	return true;
}

static bool MatchStringFilterValue ( const CSphMatch & tMatch, const CSphColumnInfo & tAttr, const CSphString & sValue )
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

static bool MatchBucketFilter ( const CSphMatch & tMatch, const ISphSchema & tSchema, const CSphFilterSettings & tFilter )
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

const CSphColumnInfo * GetGroupbyOnlyMagicAttr ( const ISphSchema & tSchema )
{
	for ( int i = 0; i<tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr ( i );
		if ( IsGroupbyOnlyMagic ( tAttr.m_sName ) )
			return &tAttr;
	}

	return nullptr;
}

static bool GetFacetStatusAttr ( const CSphQuery & tFacetQuery, const ISphSchema * pSchema, const ISphSchema & tBucketSchema, CSphString & sAttr )
{
	auto fnHasAttr = [pSchema,&tBucketSchema] ( const char * szAttr )
	{
		return ( !pSchema || pSchema->GetAttr ( szAttr ) ) && tBucketSchema.GetAttr ( szAttr );
	};

	auto fnUseAttr = [&fnHasAttr,&sAttr] ( const CSphString & sCandidate )
	{
		if ( sCandidate.IsEmpty() || !fnHasAttr ( sCandidate.cstr() ) )
			return false;

		sAttr = sCandidate;
		return true;
	};

	auto fnUseGroupbyMagicAttr = [pSchema,&tBucketSchema,&sAttr] ()
	{
		const CSphColumnInfo * pAttr = GetGroupbyOnlyMagicAttr ( tBucketSchema );
		if ( !pAttr )
			return false;

		if ( pSchema && !pSchema->GetAttr ( pAttr->m_sName.cstr() ) )
			return false;

		sAttr = pAttr->m_sName;
		return true;
	};

	if ( tFacetQuery.m_dFacetOwnFilterAttrs.GetLength()>1 && !tFacetQuery.m_sGroupBy.IsEmpty() )
		if ( fnUseGroupbyMagicAttr() )
			return true;

	if ( tFacetQuery.m_dFacetOwnFilterAttrs.GetLength()==1 && fnUseAttr ( tFacetQuery.m_dFacetOwnFilterAttrs[0] ) )
		return true;

	if ( fnUseAttr ( tFacetQuery.m_sFacetBy ) )
		return true;

	if ( fnUseAttr ( tFacetQuery.m_sGroupBy ) )
		return true;

	if ( !tFacetQuery.m_sGroupBy.IsEmpty() && fnUseGroupbyMagicAttr() )
		return true;

	return false;
}

static uint64_t CalcBucketHash ( const CSphMatch & tMatch, const CSphColumnInfo & tAttr )
{
	if ( tAttr.m_eAttrType==SPH_ATTR_STRINGPTR )
	{
		auto * pString = (const BYTE*) tMatch.GetAttr ( tAttr.m_tLocator );
		auto dString = sphUnpackPtrAttr ( pString );
		int iLen = dString.second;
		if ( iLen>1 && dString.first[iLen-2]=='\0' )
			iLen -= 2;

		return sphFNV64 ( dString.first, iLen );
	}

	return sphFNV64 ( tMatch.GetAttr ( tAttr.m_tLocator ) );
}

static const FacetBucketSet_t * CollectBucketSet ( const VecTraits_T<CSphMatch> & dMatches, const ISphSchema & tSchema, const CSphString & sAttr, FacetBucketSet_t & tBuckets )
{
	tBuckets.Reset();
	const CSphColumnInfo * pAttr = tSchema.GetAttr ( sAttr.cstr() );
	if ( !pAttr )
		return nullptr;

	tBuckets.m_sAttr = sAttr;
	for ( const auto & tMatch : dMatches )
		tBuckets.m_dHashes.Add ( CalcBucketHash ( tMatch, *pAttr ) );

	tBuckets.m_dHashes.Uniq();

	return &tBuckets;
}

static bool MatchBucketSet ( const CSphMatch & tMatch, const ISphSchema & tSchema, const FacetBucketSet_t & tBuckets )
{
	const CSphColumnInfo * pAttr = tSchema.GetAttr ( tBuckets.m_sAttr.cstr() );
	if ( !pAttr )
		return false;

	return tBuckets.m_dHashes.BinarySearch ( CalcBucketHash ( tMatch, *pAttr ) )!=nullptr;
}

const FacetBucketSet_t * CollectFacetStatusValuesFilter ( const CSphQuery & tFacetQuery, const ISphSchema & tBucketSchema, const AggrResult_t & tRes, FacetBucketSet_t & tBuckets )
{
	if ( tRes.m_dResults.IsEmpty() )
		return nullptr;

	CSphString sAttr;
	if ( !GetFacetStatusAttr ( tFacetQuery, &tRes.m_tSchema, tBucketSchema, sAttr ) )
		return nullptr;

	auto dMatches = tRes.m_dResults.First().m_dMatches.Slice ( tRes.m_iOffset, tRes.m_iCount );
	if ( dMatches.IsEmpty() )
		return nullptr;

	return CollectBucketSet ( dMatches, tRes.m_tSchema, sAttr, tBuckets );
}

const FacetBucketSet_t * CollectFacetAvailableFilters ( const AggrResult_t & tRes, const CSphString & sAttr, const VecTraits_T<CSphMatch> & dMatches, FacetBucketSet_t & tAvailable )
{
	if ( tRes.m_dResults.IsEmpty() )
	{
		tAvailable.Reset();
		tAvailable.m_sAttr = sAttr;
		return &tAvailable;
	}

	return CollectBucketSet ( dMatches, tRes.m_tSchema, sAttr, tAvailable );
}

const FacetBucketSet_t * CollectFacetAvailableFilters ( const CSphQuery & tFacetQuery, const ISphSchema & tBucketSchema, const AggrResult_t & tRes, FacetBucketSet_t & tAvailable )
{
	CSphString sAttr;
	if ( tRes.m_dResults.IsEmpty() )
	{
		if ( !GetFacetStatusAttr ( tFacetQuery, nullptr, tBucketSchema, sAttr ) )
			return nullptr;

		tAvailable.Reset();
		tAvailable.m_sAttr = sAttr;
		return &tAvailable;
	}

	if ( !GetFacetStatusAttr ( tFacetQuery, &tRes.m_tSchema, tBucketSchema, sAttr ) )
		return nullptr;

	auto dMatches = tRes.m_dResults.First().m_dMatches.Slice ( tRes.m_iOffset, tRes.m_iCount );
	return CollectFacetAvailableFilters ( tRes, sAttr, dMatches, tAvailable );
}

const char * GetBucketStatus ( const CSphMatch & tMatch, const ISphSchema & tSchema, const FacetStatusSources_t & tStatus )
{
	bool bSelected = tStatus.m_pSelectedFilters && !tStatus.m_pSelectedFilters->IsEmpty();
	if ( bSelected )
	{
		for ( const auto & tFilter : *tStatus.m_pSelectedFilters )
		{
			if ( !MatchBucketFilter ( tMatch, tSchema, tFilter ) )
			{
				bSelected = false;
				break;
			}
		}
	}

	if ( bSelected )
		return "selected";

	if ( tStatus.m_pSelectedBuckets && MatchBucketSet ( tMatch, tSchema, *tStatus.m_pSelectedBuckets ) )
		return "selected";

	if ( tStatus.m_pAvailableBuckets && MatchBucketSet ( tMatch, tSchema, *tStatus.m_pAvailableBuckets ) )
		return "available";

	return tStatus.m_pAvailableBuckets ? "unavailable" : "available";
}

const CSphVector<CSphFilterSettings> * CollectSelectedFiltersForAttr ( const CSphVector<CSphFilterSettings> & dFilters, const CSphString & sAttr, CSphVector<CSphFilterSettings> & dSelected )
{
	for ( const auto & tFilter : dFilters )
		if ( IsSelectedFilterType ( tFilter ) && sAttr==tFilter.m_sAttrName )
			dSelected.Add ( tFilter );

	return dSelected.IsEmpty() ? nullptr : &dSelected;
}
}
