//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "fastcount.h"

#include "sphinxquery/sphinxquery.h"
#include "sphinxutils.h"

bool IsRanklessCountQuery ( const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphMultiQueryArgs & tArgs )
{
	if ( tQuery.m_sQuery.IsEmpty() )
		return false;

	if ( tQuery.m_dItems.GetLength()!=1 || tQuery.m_dItems[0].m_sExpr!="count(*)" || tQuery.m_dItems[0].m_eAggrFunc!=SPH_AGGR_NONE )
		return false;

	if ( !tQuery.m_sGroupBy.IsEmpty() || !tQuery.m_sGroupDistinct.IsEmpty() || !tQuery.m_tHaving.m_sAttrName.IsEmpty() )
		return false;

	// filters are evaluated by the node tree walk; only a weight filter needs ranking
	if ( tQuery.m_dFilters.any_of ( []( const CSphFilterSettings & tFilter ) { return tFilter.m_sAttrName=="@weight" || tFilter.m_sAttrName=="weight()"; } ) )
		return false;

	if ( tQuery.HasKnn() || tQuery.m_bHybridSearch || tQuery.m_eJoinType!=JoinType_e::NONE || !tQuery.m_sJoinIdx.IsEmpty() )
		return false;

	if ( tQuery.m_bFacet || tQuery.m_bFacetHead || tQuery.IsInternalQuery() || tQuery.m_bHasOuter || tQuery.m_bExplicitOrderBy || tQuery.m_bZSlist )
		return false;

	if ( tQuery.m_iCutoff>0 )
		return false;

	if ( !tQuery.m_sQueryTokenFilterLib.IsEmpty() || !tQuery.m_dIndexHints.IsEmpty() )
		return false;

	if ( tArgs.m_uPackedFactorFlags & SPH_FACTOR_ENABLE )
		return false;

	if ( dSorters.GetLength()!=1 )
		return false;

	const ISphMatchSorter * pSorter = dSorters[0];
	if ( !pSorter || !pSorter->IsGroupby() || pSorter->IsPrecalc() || pSorter->IsJoin() || !pSorter->CanBeCloned() )
		return false;

	const ISphSchema & tSchema = *pSorter->GetSchema();
	if ( !tSchema.GetAttr ( "@count" ) || !tSchema.GetAttr ( "@groupby" ) || tSchema.GetAttr ( "@distinct" ) )
		return false;

	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_eAggrFunc!=SPH_AGGR_NONE && tAttr.m_sName!="@count" )
			return false;
	}

	return true;
}


bool TreeAllowsRanklessCount ( const XQQuery_t & tParsed )
{
	// zones are matched with the ranker's help; everything else is decided inside the node tree
	return tParsed.m_pRoot && tParsed.m_dZones.IsEmpty() && !tParsed.m_bNeedSZlist;
}


bool IsFastTermCountQuery ( const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphMultiQueryArgs & tArgs )
{
	if ( !IsRanklessCountQuery ( tQuery, dSorters, tArgs ) )
		return false;

	// the dictionary count is exact only over the whole chunk
	if ( !tQuery.m_dFilters.IsEmpty() || !tQuery.m_dFilterTree.IsEmpty() )
		return false;

	// keep syntactically complex queries on the regular path even if they simplify to a single term
	if ( strpbrk ( tQuery.m_sQuery.cstr(), "\"~!/^$<>=-\\" ) )
		return false;

	return true;
}


const XQKeyword_t * GetFastTermCountKeyword ( const XQNode_t * pRoot, const ISphSchema & tSchema )
{
	if ( !pRoot || !pRoot->dChildren().IsEmpty() || pRoot->dWords().GetLength()!=1 || pRoot->m_bVirtuallyPlain )
		return nullptr;

	if ( pRoot->GetOp()!=SPH_QUERY_AND && pRoot->GetOp()!=SPH_QUERY_OR )
		return nullptr;

	const int iFields = tSchema.GetFieldsCount();
	if ( iFields>32 )
		return nullptr;

	const XQKeyword_t & tWord = pRoot->dWord(0);
	if ( tWord.m_sWord.IsEmpty() || tWord.m_bExpanded || tWord.m_bExcluded || tWord.m_bFieldStart || tWord.m_bFieldEnd || tWord.m_bRegex || tWord.m_pPayload || tWord.m_iBlendedGroup>=0 )
		return nullptr;

	for ( const char * p = tWord.m_sWord.cstr(); *p; p++ )
		if ( sphIsWild(*p) )
			return nullptr;

	const XQLimitSpec_t & tSpec = pRoot->m_dSpec;
	if ( tSpec.m_iFieldMaxPos || tSpec.m_bZoneSpan || !tSpec.m_dZones.IsEmpty() )
		return nullptr;

	// the dictionary count is per word over all fields; it is exact only when every indexed field is queried
	if ( tSpec.m_bFieldSpec )
		for ( int i=0; i<iFields; i++ )
			if ( ( tSchema.GetField(i).m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED ) && !tSpec.m_dFieldMask.Test(i) )
				return nullptr;

	return &tWord;
}


void SetupFastCountQword ( ISphQword & tQword, const XQKeyword_t & tWord, const DictRefPtr_c & pDict, KeywordBuf_t & tBuf )
{
	tQword.Reset();
	tQword.m_sWord = tWord.m_sWord;

	BYTE * pWordBuf = tBuf.Begin();
	int iWordBytes = Min ( tWord.m_sWord.Length(), tBuf.GetLength()-1 );
	memcpy ( pWordBuf, tWord.m_sWord.cstr(), iWordBytes );
	pWordBuf[iWordBytes] = '\0';

	tQword.m_uWordID = tWord.m_bMorphed ? pDict->GetWordIDNonStemmed ( pWordBuf ) : pDict->GetWordID ( pWordBuf );
	tQword.m_sDictWord.SetBinary ( (const char*)pWordBuf, (int)strlen ( (const char*)pWordBuf ) );
	tQword.m_bExpanded = false;
}


void PushFastCountMatch ( const VecTraits_T<ISphMatchSorter*> & dSorters, const CSphRowitem * pStatic, int iTag, int iIndexWeight, int64_t iCount )
{
	if ( iCount<=0 )
		return;

	for ( ISphMatchSorter * pSorter : dSorters )
	{
		const ISphSchema & tSchema = *pSorter->GetSchema();
		const CSphColumnInfo * pCount = tSchema.GetAttr ( "@count" );
		const CSphColumnInfo * pGroupby = tSchema.GetAttr ( "@groupby" );
		assert ( pCount && pGroupby );

		// one aggregated row per chunk/segment; implicit group sorters sum @count through PushGrouped() and MoveTo()
		CSphMatch tMatch;
		tMatch.Reset ( tSchema.GetDynamicSize() );
		tMatch.m_tRowID = 0;
		tMatch.m_pStatic = pStatic;
		tMatch.m_iTag = iTag;
		tMatch.m_iWeight = iIndexWeight;
		tMatch.SetAttr ( pGroupby->m_tLocator, 1 );
		tMatch.SetAttr ( pCount->m_tLocator, iCount );
		pSorter->PushGrouped ( tMatch, true );
	}
}


void RecordFastCount ( CSphQueryResultMeta & tMeta, const char * szType )
{
	IteratorStats_t tStats;
	tStats.m_dIterators.Add ( { "fulltext", szType } );
	tStats.m_iTotal = 1;
	tMeta.m_tIteratorStats.Merge ( tStats );
}
