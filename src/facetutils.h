//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _facetutils_
#define _facetutils_

#include "sphinx.h"
#include "aggrexpr.h"

namespace facet
{
struct FacetBucketSet_t
{
	CSphString m_sAttr;
	CSphVector<uint64_t> m_dHashes;

	void Reset();
};

struct FacetStatusSources_t
{
	const CSphVector<CSphFilterSettings> * m_pSelectedFilters = nullptr;
	const FacetBucketSet_t * m_pSelectedBuckets = nullptr;
	const FacetBucketSet_t * m_pAvailableBuckets = nullptr;

	bool HasStatus() const;
};

bool AttrNameInList ( const StrVec_t & dAttrs, const CSphString & sAttr );
bool IsSelectedFilterType ( const CSphFilterSettings & tFilter );
bool IsJsonFacetStatusBucket ( Aggr_e eAggr );
bool HasExcludedFilter ( const CSphVector<CSphFilterSettings> & dFilters );
bool HasLocalFilterMode ( const FacetFilterTrait_t & tTrait );
const CSphColumnInfo * GetGroupbyOnlyMagicAttr ( const ISphSchema & tSchema );

FacetFilterMode_e GetDefaultFilterMode ( const FacetFilterTrait_t & tTrait );
FacetFilterMode_e GetFilterMode ( const CSphQuery & tHeadQuery, const FacetFilterTrait_t & tLocalTrait );
FacetFilterMode_e GetFilterMode ( const CSphQuery & tHeadQuery, const CSphQuery & tFacetQuery );

bool ShouldRewriteFilters ( const CSphQuery & tHeadQuery, const CSphQuery & tFacetQuery );
bool CopyFilters ( const CSphQuery & tHeadQuery, CSphQuery & tFacetQuery, CSphString & sError, bool bUseOwnExclusionRuleInNoneClause );

const char * GetBucketStatus ( const CSphMatch & tMatch, const ISphSchema & tSchema, const FacetStatusSources_t & tStatus );
const CSphVector<CSphFilterSettings> * CollectSelectedFiltersForAttr ( const CSphVector<CSphFilterSettings> & dFilters, const CSphString & sAttr, CSphVector<CSphFilterSettings> & dSelected );

const FacetBucketSet_t * CollectFacetStatusValuesFilter ( const CSphQuery & tFacetQuery, const ISphSchema & tBucketSchema, const AggrResult_t & tRes, FacetBucketSet_t & tBuckets );
const FacetBucketSet_t * CollectFacetAvailableFilters ( const CSphQuery & tFacetQuery, const ISphSchema & tBucketSchema, const AggrResult_t & tRes, FacetBucketSet_t & tAvailable );
const FacetBucketSet_t * CollectFacetAvailableFilters ( const AggrResult_t & tRes, const CSphString & sAttr, const VecTraits_T<CSphMatch> & dMatches, FacetBucketSet_t & tAvailable );
}

#endif
