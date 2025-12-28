//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "minimize_aggr_result.h"

#include "searchdaemon.h"
#include "queuecreator.h"
#include "frontendschema.h"
#include "schematransform.h"
#include "match_iterator.h"
#include "searchdha.h"

// returns true if incoming schema (src) is compatible with existing (dst); false otherwise
static bool MinimizeSchema ( CSphSchema & tDst, const ISphSchema & tSrc )
{
	// if dst is empty, result is also empty
	if ( tDst.GetAttrsCount()==0 )
		return tSrc.GetAttrsCount()==0;

	// check for equality, and remove all dst attributes that are not present in src
	CSphVector<CSphColumnInfo> dDst;
	for ( int i = 0, iAttrsCount = tDst.GetAttrsCount (); i<iAttrsCount; ++i )
		dDst.Add ( tDst.GetAttr(i) );

	bool bEqual = ( tDst.GetAttrsCount()==tSrc.GetAttrsCount() );
	ARRAY_FOREACH ( i, dDst )
	{
		auto& tDstAttr = dDst[i];
		int iSrcIdx = tSrc.GetAttrIndex ( tDstAttr.m_sName.cstr() );

		// check for index mismatch
		if ( iSrcIdx!=i )
			bEqual = false;

		// check for type/size mismatch (and fixup if needed)
		if ( iSrcIdx>=0 )
		{
			const CSphColumnInfo & tSrcAttr = tSrc.GetAttr ( iSrcIdx );

			// should seamlessly convert ( bool > float ) | ( bool > int > bigint )
			ESphAttr eDst = tDstAttr.m_eAttrType;
			ESphAttr eSrc = tSrcAttr.m_eAttrType;
			bool bSame = ( eDst==eSrc )
				|| ( ( eDst==SPH_ATTR_FLOAT && eSrc==SPH_ATTR_BOOL ) || ( eDst==SPH_ATTR_BOOL && eSrc==SPH_ATTR_FLOAT ) )
				|| ( ( eDst==SPH_ATTR_BOOL || eDst==SPH_ATTR_INTEGER || eDst==SPH_ATTR_BIGINT )
					&& ( eSrc==SPH_ATTR_BOOL || eSrc==SPH_ATTR_INTEGER || eSrc==SPH_ATTR_BIGINT ) );

			int iDstBitCount = tDstAttr.m_tLocator.m_iBitCount;
			int iSrcBitCount = tSrcAttr.m_tLocator.m_iBitCount;

			if ( !bSame )
			{
				// different types? remove the attr
				iSrcIdx = -1;
				bEqual = false;

			} else if ( iDstBitCount!=iSrcBitCount )
			{
				// different bit sizes? choose the max one
				tDstAttr.m_tLocator.m_iBitCount = Max ( iDstBitCount, iSrcBitCount );
				bEqual = false;
				if ( iDstBitCount<iSrcBitCount )
					tDstAttr.m_eAttrType = tSrcAttr.m_eAttrType;
			}

			if ( tSrcAttr.m_tLocator.m_iBitOffset!=tDstAttr.m_tLocator.m_iBitOffset )
			{
				// different offsets? have to force target dynamic then, since we can't use one locator for all matches
				bEqual = false;
			}

			if ( tSrcAttr.m_tLocator.m_bDynamic!=tDstAttr.m_tLocator.m_bDynamic )
			{
				// different location? have to force target dynamic then
				bEqual = false;
			}
		}

		// check for presence
		if ( iSrcIdx<0 )
		{
			dDst.Remove ( i );
			--i;
		}
	}

	if ( !bEqual )
	{
		CSphVector<CSphColumnInfo> dFields { tDst.GetFieldsCount() };
		for ( int i = 0, iFieldsCount = tDst.GetFieldsCount (); i<iFieldsCount; ++i )
			dFields[i] = tDst.GetField(i);

		tDst.Reset();

		for ( auto& dAttr : dDst )
			tDst.AddAttr ( dAttr, true );

		for ( auto& dField: dFields )
			tDst.AddField ( dField );

	} else
		tDst.SwapAttrs ( dDst );

	return bEqual;
}

static void RemapResult ( AggrResult_t & dResult )
{
	const ISphSchema & tSchema = dResult.m_tSchema;
	int iAttrsCount = tSchema.GetAttrsCount();
	CSphVector<int> dMapFrom(iAttrsCount);
	CSphVector<int> dRowItems(iAttrsCount);
	static const int SIZE_OF_ROW = 8 * sizeof ( CSphRowitem );

	for ( auto & tRes : dResult.m_dResults )
	{
		if ( tRes.m_dMatches.IsEmpty() )
			continue;

		dMapFrom.Resize ( 0 );
		dRowItems.Resize ( 0 );
		CSphSchema & dSchema = tRes.m_tSchema;
		for ( int i = 0; i<iAttrsCount; ++i )
		{
			auto iSrcCol = dSchema.GetAttrIndex ( tSchema.GetAttr ( i ).m_sName.cstr () );
			dMapFrom.Add ( iSrcCol );
			dRowItems.Add ( dSchema.GetAttr ( iSrcCol ).m_tLocator.m_iBitOffset / SIZE_OF_ROW );
			assert ( dMapFrom[i]>=0
				|| IsSortStringInternal ( tSchema.GetAttr(i).m_sName )
				|| IsSortJsonInternal ( tSchema.GetAttr(i).m_sName )
				);
		}

		// inverse dRowItems - we'll free only those NOT enumerated yet
		dRowItems = dSchema.SubsetPtrs ( dRowItems );
		for ( auto& tMatch : tRes.m_dMatches )
		{
			// create new and shiny (and properly sized) match
			CSphMatch tNewMatch;
			tNewMatch.Reset ( tSchema.GetDynamicSize () );
			tNewMatch.m_tRowID = tMatch.m_tRowID;
			tNewMatch.m_iWeight = tMatch.m_iWeight;

			// remap attrs
			for ( int j = 0; j<iAttrsCount; ++j )
			{
				const CSphColumnInfo & tDst = tSchema.GetAttr ( j );
				// we could keep some of the rows static
				// and so, avoid the duplication of the data.
				int iMapFrom = dMapFrom[j];
				const CSphColumnInfo & tSrc = dSchema.GetAttr ( iMapFrom );
				if ( !tDst.m_tLocator.m_bDynamic )
				{
					assert ( iMapFrom<0 || !dSchema.GetAttr ( iMapFrom ).m_tLocator.m_bDynamic );
					tNewMatch.m_pStatic = tMatch.m_pStatic;
				} else if ( iMapFrom>=0 )
				{
					if ( tDst.m_eAttrType==SPH_ATTR_FLOAT && tSrc.m_eAttrType==SPH_ATTR_BOOL )
					{
						tNewMatch.SetAttrFloat ( tDst.m_tLocator, ( tMatch.GetAttr ( tSrc.m_tLocator )>0 ? 1.0f : 0.0f ) );
					} else
					{
						tNewMatch.SetAttr ( tDst.m_tLocator, tMatch.GetAttr ( tSrc.m_tLocator ) );
					}
				}
			}
			// swap out old (most likely wrong sized) match
			Swap ( tMatch, tNewMatch );
			CSphSchemaHelper::FreeDataSpecial ( tNewMatch, dRowItems );
		}
	}
}

static bool GetIndexSchemaItems ( const ISphSchema & tSchema, const CSphVector<CSphQueryItem> & dItems, CSphVector<int> & dAttrs )
{
	bool bHaveAsterisk = false;
	for ( const auto & i : dItems )
	{
		if ( i.m_sAlias.cstr() )
		{
			int iAttr = tSchema.GetAttrIndex ( i.m_sAlias.cstr() );
			if ( iAttr>=0 )
				dAttrs.Add(iAttr);
		}

		bHaveAsterisk |= i.m_sExpr=="*";
	}

	dAttrs.Sort();
	return bHaveAsterisk;
}

static bool IsJoinedWeight ( const CSphString & sAttr, const CSphQuery & tQuery )
{
	// don't skip this attribute in json queries as it will be used as _score
	if ( tQuery.m_eQueryType==QUERY_JSON )
		return false;

	if ( tQuery.m_sJoinIdx.IsEmpty() )
		return false;

	CSphString sWeight;
	sWeight.SetSprintf ( "%s.weight()", tQuery.m_sJoinIdx.cstr() );

	return sAttr==sWeight;
}

static bool GetItemsLeftInSchema ( const ISphSchema & tSchema, const CSphQuery & tQuery, const CSphVector<int> & dAttrs, CSphVector<int> & dAttrsInSchema )
{
	bool bHaveExprs = false;

	for ( int i = 0, iAttrsCount = tSchema.GetAttrsCount (); i<iAttrsCount; ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);

		if ( tAttr.m_pExpr )
		{
			bHaveExprs = true;

			// need to keep post-limit expression (stored field) for multi-query \ facet
			// also keep columnar attributes (with expressions)
			if ( tQuery.m_bFacetHead && !tAttr.m_pExpr->IsColumnar() && tAttr.m_eStage!=SPH_EVAL_POSTLIMIT )
				continue;
		}

		if ( !IsGroupbyMagic ( tAttr.m_sName ) && !IsSortStringInternal ( tAttr.m_sName ) && !IsJoinedWeight ( tAttr.m_sName, tQuery ) && !dAttrs.BinarySearch(i) )
			dAttrsInSchema.Add(i);
	}

	return bHaveExprs;
}

struct AttrSort_fn
{
	const ISphSchema & m_tSchema;

	explicit AttrSort_fn ( const ISphSchema & tSchema )
		: m_tSchema ( tSchema )
	{}

	bool IsLess ( int iA, int iB ) const
	{
		const auto & sNameA = m_tSchema.GetAttr(iA).m_sName;
		const auto & sNameB = m_tSchema.GetAttr(iB).m_sName;
		bool bDocIdA = sNameA==sphGetDocidName();
		bool bDocIdB = sNameB==sphGetDocidName();
		if ( bDocIdA || bDocIdB )
			return bDocIdA || !bDocIdB;

		bool bBlobLocA = sNameA==sphGetBlobLocatorName();
		bool bBlobLocB = sNameB==sphGetBlobLocatorName();
		if ( bBlobLocA ||bBlobLocB )
			return bBlobLocA || !bBlobLocB;

		bool bFieldA = !!m_tSchema.GetField ( sNameA.cstr() );
		bool bFieldB = !!m_tSchema.GetField ( sNameB.cstr() );
		if ( bFieldA || bFieldB )
		{
			if ( bFieldA && bFieldB )
			{
				int iFieldIdA = m_tSchema.GetFieldIndex ( sNameA.cstr() );
				int iFieldIdB = m_tSchema.GetFieldIndex ( sNameB.cstr() );
				return iFieldIdA < iFieldIdB;
			}

			return bFieldA || !bFieldB;
		}

		int iIndexA = m_tSchema.GetAttr(iA).m_iIndex;
		int iIndexB = m_tSchema.GetAttr(iB).m_iIndex;

		if ( iIndexA == -1 && iIndexB == -1 )
			return iA < iB;

		return iIndexA != -1 && ( iIndexB == -1 || iIndexA < iIndexB );
	}
};


static void DoExpansion ( const ISphSchema & tSchema, const CSphVector<int> & dAttrsInSchema, const CSphVector<CSphQueryItem> & dItems, CSphVector<CSphQueryItem> & dExpanded )
{
	bool bExpandedAsterisk = false;
	for ( const auto & i : dItems )
	{
		if ( i.m_sExpr=="*" )
		{
			if ( bExpandedAsterisk )
				continue;

			bExpandedAsterisk = true;

			IntVec_t dSortedAttrsInSchema = dAttrsInSchema;
			dSortedAttrsInSchema.Sort ( AttrSort_fn(tSchema) );

			for ( auto iAttr : dSortedAttrsInSchema )
			{
				const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
				CSphQueryItem & tExpanded = dExpanded.Add();
				tExpanded.m_sExpr = tCol.m_sName;
				if ( tCol.m_pExpr )	// stored fields
					tExpanded.m_sAlias = tCol.m_sName;
			}
		}
		else
			dExpanded.Add(i);
	}
}

// rebuild the results itemlist expanding stars
static const CSphVector<CSphQueryItem> & ExpandAsterisk ( const ISphSchema & tSchema, const CSphVector<CSphQueryItem> & dItems, CSphVector<CSphQueryItem> & tExpanded, const CSphQuery & tQuery, bool & bHaveExprs )
{
	// the result schema usually is the index schema + calculated items + @-items
	// we need to extract the index schema only
	CSphVector<int> dIndexSchemaAttrs;
	bool bHaveAsterisk = GetIndexSchemaItems ( tSchema, dItems, dIndexSchemaAttrs );

	// no stars? Nothing to do.
	if ( !bHaveAsterisk )
		return dItems;

	// find items that are in index schema but not in our requested item list
	// not do not include @-items
	CSphVector<int> dAttrsLeftInSchema;
	bHaveExprs = GetItemsLeftInSchema ( tSchema, tQuery, dIndexSchemaAttrs, dAttrsLeftInSchema );

	DoExpansion ( tSchema, dAttrsLeftInSchema, dItems, tExpanded );

	return tExpanded;
}


static int KillPlainDupes ( ISphMatchSorter * pSorter, AggrResult_t & tRes )
{
	int iDupes = 0;

	auto& dResults = tRes.m_dResults;

	// normal sorter needs massage
	// queue by docid and then ascending by tag to guarantee the replacement order
	RawVector_T <MatchIterator_c> dIterators;
	dIterators.Reserve_static ( dResults.GetLength () );
	CSphQueue<MatchIterator_c *, MatchIterator_c> qMatches ( dResults.GetLength () );

	for ( auto & tResult : dResults )
		if ( !tResult.m_dMatches.IsEmpty() )
		{
			dIterators.Emplace_back(tResult);
			qMatches.Push ( &dIterators.Last() );
		}

	DocID_t tPrevDocID = DOCID_MIN;
	while ( qMatches.GetLength() )
	{
		auto * pMin = qMatches.Root();
		DocID_t tDocID = pMin->m_tDocID;
		if ( tDocID!=tPrevDocID ) // by default, simply remove dupes (select first by tag)
		{
			CSphMatch & tMatch = pMin->m_tResult.m_dMatches[pMin->m_iIdx];
			auto iTag = tMatch.m_iTag;	// as we may use tag for ordering
			if ( !pMin->m_tResult.m_bTagsAssigned )
				tMatch.m_iTag = pMin->m_tResult.m_iTag; // that will link us back to docstore

			pSorter->Push ( tMatch );
			tMatch.m_iTag = iTag;	// restore tag
			tPrevDocID = tDocID;
		}
		else
			++iDupes;

		qMatches.Pop ();
		if ( pMin->Step() )
			qMatches.Push ( pMin );
	}
	tRes.m_bTagsAssigned = true;
	return iDupes;
}

static int KillGroupbyDupes ( ISphMatchSorter * pSorter, AggrResult_t & tRes, const VecTraits_T<int>& dOrd )
{
	int iDupes = 0;
	pSorter->SetBlobPool ( nullptr );
	for ( int iOrd : dOrd )
	{
		auto & tResult = tRes.m_dResults[iOrd];
		ARRAY_CONSTFOREACH( i, tResult.m_dMatches )
		{
			CSphMatch & tMatch = tResult.m_dMatches[i];
			if ( !tResult.m_bTagsAssigned )
				tMatch.m_iTag = tResult.m_iTag; // that will link us back to docstore

			if ( !pSorter->PushGrouped ( tMatch, i==0 ) )  // groupby sorter does that automagically
				++iDupes;
		}
	}
	tRes.m_bTagsAssigned = true;
	return iDupes;
}

// rearrange results so thet the're placed by accending tags order
// dOrd contains indexes to access results in descending tag order
static void SortTagsAndDocstores ( AggrResult_t & tRes, const VecTraits_T<int>& dOrd )
{
	auto iTags = dOrd.GetLength ();
	CSphFixedVector<DocstoreAndTag_t> dTmp { iTags };
	auto & dResults = tRes.m_dResults;

	for ( int i=0; i<iTags; ++i )
		dTmp[iTags-i-1].Assign ( dResults[dOrd[i]] );

	for ( int i = 0; i<iTags; ++i )
		dResults[i].Assign ( dTmp[i] );

	Debug ( tRes.m_bIdxByTag = true; )
}

static int KillDupesAndFlatten ( ISphMatchSorter * pSorter, AggrResult_t & tRes )
{
	assert ( pSorter );

	int iTags = tRes.m_dResults.GetLength();
	CSphFixedVector<int> dOrd ( iTags );
	ARRAY_CONSTFOREACH( i, dOrd )
		dOrd[i] = i;

	// sort resultsets in descending tag order
	dOrd.Sort ( Lesser ( [&tRes] ( int l, int r ) { return tRes.m_dResults[r].m_iTag<tRes.m_dResults[l].m_iTag; } ) );

	// remap to compact (non-fragmented) range of tags
	for ( int iRes : dOrd )
		tRes.m_dResults[iRes].m_iTag = --iTags;
	Debug ( tRes.m_bTagsCompacted = true );

	// do actual deduplication
	int iDup = pSorter->IsGroupby() ? KillGroupbyDupes ( pSorter, tRes, dOrd ) : KillPlainDupes ( pSorter, tRes );

	// ALL matches have same schema, as KillAllDupes called after RemapResults(), or already having identical schemas.
	for ( auto& dResult : tRes.m_dResults )
	{
		for ( auto& dMatch : dResult.m_dMatches )
			tRes.m_tSchema.FreeDataPtrs ( dMatch );
		dResult.m_dMatches.Reset();
	}

	// don't issue tRes.m_dResults.reset since each result still has a docstore by tag

	// flatten all results into single chunk
	auto & tFinalMatches = tRes.m_dResults.First ();
	tFinalMatches.FillFromSorter ( pSorter );
	Debug ( tRes.m_bSingle = true; )
	Debug ( tRes.m_bOneSchema = true; )

	// now all matches properly tagged located in tRes.m_dResults.First()
	// each tRes.m_dResults has proper tag and corresponding docstore pointer in random order
	// and we have dOrd wich enumerates them in descending tag order
	SortTagsAndDocstores ( tRes, dOrd );
	return iDup;
}

static void RecoverAggregateFunctions ( const CSphQuery & tQuery, const AggrResult_t & tRes )
{
	for ( const auto& tItem : tQuery.m_dItems )
	{
		if ( tItem.m_eAggrFunc==SPH_AGGR_NONE )
			continue;

		for ( int j = 0, iAttrsCount = tRes.m_tSchema.GetAttrsCount (); j<iAttrsCount; ++j )
		{
			auto & tCol = const_cast<CSphColumnInfo&> ( tRes.m_tSchema.GetAttr(j) );
			if ( tCol.m_sName==tItem.m_sAlias )
			{
				assert ( tCol.m_eAggrFunc==SPH_AGGR_NONE );
				tCol.m_eAggrFunc = tItem.m_eAggrFunc;
			}
		}
	}
}

struct GenericMatchSort_fn : CSphMatchComparatorState
{
	bool IsLess ( const CSphMatch * a, const CSphMatch * b ) const
	{
		for ( int i=0; i<MAX_ATTRS; i++ )
			switch ( m_eKeypart[i] )
		{
			case SPH_KEYPART_ROWID:
				if ( a->m_tRowID==b->m_tRowID )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( a->m_tRowID < b->m_tRowID );

			case SPH_KEYPART_WEIGHT:
				if ( a->m_iWeight==b->m_iWeight )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( a->m_iWeight < b->m_iWeight );

			case SPH_KEYPART_INT:
			{
				SphAttr_t aa = a->GetAttr ( m_tLocator[i] );
				SphAttr_t bb = b->GetAttr ( m_tLocator[i] );
				if ( aa==bb )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( aa < bb );
			}
			case SPH_KEYPART_FLOAT:
			{
				float aa = a->GetAttrFloat ( m_tLocator[i] );
				float bb = b->GetAttrFloat ( m_tLocator[i] );
				if ( aa==bb )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( aa < bb );
			}
			case SPH_KEYPART_DOUBLE:
			{
				double aa = a->GetAttrDouble ( m_tLocator[i] );
				double bb = b->GetAttrDouble ( m_tLocator[i] );
				if ( aa==bb )
					continue;
				return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( aa < bb );
			}
			case SPH_KEYPART_STRINGPTR:
			case SPH_KEYPART_STRING:
			{
				int iCmp = CmpStrings ( *a, *b, i );
				if ( iCmp!=0 )
					return ( ( m_uAttrDesc>>i ) & 1 ) ^ ( iCmp < 0 );
				break;
			}
		}

		return a->m_tRowID<b->m_tRowID;
	}
};

static void ExtractPostlimit ( const ISphSchema & tSchema, bool bMaster, CSphVector<const CSphColumnInfo *> & dPostlimit )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr ( i );
		if ( tCol.m_eStage==SPH_EVAL_POSTLIMIT && ( bMaster || tCol.m_uFieldFlags==CSphColumnInfo::FIELD_NONE ) )
			dPostlimit.Add ( &tCol );
	}
}

// for single chunk of matches return list of tags with docstores
static CSphVector<int> GetUniqueTagsWithDocstores ( const AggrResult_t & tRes, int iOff, int iLim )
{
	assert ( tRes.m_bTagsCompacted );
	assert ( tRes.m_bSingle );

	CSphVector<bool> dBoolTags;
	dBoolTags.Resize ( tRes.m_dResults.GetLength() );
	dBoolTags.ZeroVec();

	auto dMatches = tRes.m_dResults.First ().m_dMatches.Slice ( iOff, iLim );
	for ( const auto& dMatch : dMatches )
	{
		assert ( dMatch.m_iTag < tRes.m_dResults.GetLength() );
		if ( tRes.m_dResults[dMatch.m_iTag].Docstore() )
			dBoolTags[dMatch.m_iTag] = true;
	}

	CSphVector<int> dTags;
	ARRAY_CONSTFOREACH( i, dBoolTags )
		if ( dBoolTags[i] )
			dTags.Add(i);

	return dTags;
}

static void SetupPostlimitExprs ( const DocstoreReader_i * pDocstore, const CSphColumnInfo * pCol, const char * sQuery, int64_t iDocstoreSessionId )
{
	DocstoreSession_c::InfoDocID_t tSessionInfo;
	tSessionInfo.m_pDocstore = pDocstore;
	tSessionInfo.m_iSessionId = iDocstoreSessionId;

	assert ( pCol && pCol->m_pExpr );
	pCol->m_pExpr->Command ( SPH_EXPR_SET_DOCSTORE_DOCID, &tSessionInfo ); // value is copied; no leak of pointer to local here.
	pCol->m_pExpr->Command ( SPH_EXPR_SET_QUERY, (void *)sQuery);
}

static void EvalPostlimitExprs ( CSphMatch & tMatch, const CSphColumnInfo * pCol )
{
	assert ( pCol && pCol->m_pExpr );

	switch ( pCol->m_eAttrType )
	{
	case SPH_ATTR_TIMESTAMP:
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_BOOL:
		tMatch.SetAttr ( pCol->m_tLocator, pCol->m_pExpr->IntEval ( tMatch ) );
		break;

	case SPH_ATTR_BIGINT:
		tMatch.SetAttr ( pCol->m_tLocator, pCol->m_pExpr->Int64Eval ( tMatch ) );
		break;

	case SPH_ATTR_STRINGPTR:
		// FIXME! a potential leak of *previous* value?
		tMatch.SetAttr ( pCol->m_tLocator, (SphAttr_t) pCol->m_pExpr->StringEvalPacked ( tMatch ) );
		break;

	case SPH_ATTR_UINT32SET_PTR:
	case SPH_ATTR_INT64SET_PTR:
	case SPH_ATTR_FLOAT_VECTOR_PTR:
		tMatch.SetAttr ( pCol->m_tLocator, (SphAttr_t)pCol->m_pExpr->Int64Eval(tMatch) );
		break;

	default:
		tMatch.SetAttrFloat ( pCol->m_tLocator, pCol->m_pExpr->Eval ( tMatch ) );
		break;
	}
}


// single resultset chunk, but has many tags
static void ProcessMultiPostlimit ( AggrResult_t & tRes, VecTraits_T<const CSphColumnInfo *> & dPostlimit, const char * sQuery, int iOff, int iLim )
{
	if ( dPostlimit.IsEmpty() )
		return;

	assert ( tRes.m_bSingle );
	assert ( tRes.m_bOneSchema );
	assert ( tRes.m_bTagsAssigned );
	assert ( tRes.m_bTagsCompacted );
	assert ( tRes.m_bIdxByTag );

	// collect unique tags from matches
	CSphVector<int> dDocstoreTags = GetUniqueTagsWithDocstores ( tRes, iOff, iLim );

	// generates docstore session id
	DocstoreSession_c tSession;
	auto iSessionUID = tSession.GetUID();

	// spawn buffered readers for the current session
	// put them to a global hash
	for ( int iTag : dDocstoreTags )
		tRes.m_dResults[iTag].m_pDocstore->CreateReader ( iSessionUID );

	int iLastTag = -1;
	auto dMatches = tRes.m_dResults.First ().m_dMatches.Slice ( iOff, iLim );
	for ( auto & dMatch : dMatches )
	{
		int iTag = dMatch.m_iTag;
		if ( tRes.m_dResults[iTag].m_bTag )
			continue; // remote match; everything should be precalculated

		auto * pDocstore = tRes.m_dResults[iTag].Docstore ();
		assert ( iTag<tRes.m_dResults.GetLength () );

		if ( iTag!=iLastTag )
		{
			for ( const auto & pCol : dPostlimit )
				SetupPostlimitExprs ( pDocstore, pCol, sQuery, iSessionUID );
			iLastTag = iTag;
		}

		for ( const auto & pCol : dPostlimit )
			EvalPostlimitExprs ( dMatch, pCol );
	}
}


static void ProcessSinglePostlimit ( OneResultset_t & tRes, VecTraits_T<const CSphColumnInfo *> & dPostlimit, const char * sQuery, int iOff, int iLim )
{
	auto dMatches = tRes.m_dMatches.Slice ( iOff, iLim );
	if ( dMatches.IsEmpty() )
		return;

	// generates docstore session id
	DocstoreSession_c tSession;
	auto iSessionUID = tSession.GetUID();

	// spawn buffered reader for the current session
	// put it to a global hash
	if ( tRes.Docstore () )
		tRes.m_pDocstore->CreateReader ( iSessionUID );

	for ( const auto & pCol : dPostlimit )
		SetupPostlimitExprs ( tRes.Docstore (), pCol, sQuery, iSessionUID );

	for ( auto & tMatch : dMatches )
		for ( const auto & pCol : dPostlimit )
			EvalPostlimitExprs ( tMatch, pCol );
}

static void ProcessLocalPostlimit ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bMaster )
{
	assert ( !tRes.m_bOneSchema );
	assert ( !tRes.m_bSingle );

	bool bGotPostlimit = false;
	for ( int i = 0, iAttrsCount = tRes.m_tSchema.GetAttrsCount(); i < iAttrsCount && !bGotPostlimit; ++i )
	{
		const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr ( i );
		bGotPostlimit = (tCol.m_eStage == SPH_EVAL_POSTLIMIT && (bMaster || tCol.m_uFieldFlags == CSphColumnInfo::FIELD_NONE));
	}

	if ( !bGotPostlimit )
		return;

	int iLimit = (tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit);
	iLimit += Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
	CSphVector<const CSphColumnInfo *> dPostlimit;
	for ( auto & tResult: tRes.m_dResults )
	{
		dPostlimit.Resize ( 0 );
		ExtractPostlimit ( tResult.m_tSchema, bMaster, dPostlimit );
		if ( dPostlimit.IsEmpty() )
			continue;

		iLimit = (tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit);

		// we can't estimate limit.offset per result set
		// as matches got merged and sort next step
		if ( !tResult.m_bTag )
			ProcessSinglePostlimit ( tResult, dPostlimit, tQuery.m_sQuery.cstr(), 0, iLimit );
	}
}

static bool MinimizeSchemas ( AggrResult_t & tRes )
{
	bool bAllEqual = true;
	bool bSchemaBaseSet = false;

	auto iResults = tRes.m_dResults.GetLength();

	for ( int i=0; i<iResults; ++i )
	{
		// skip empty result set
		if ( !tRes.m_dResults[i].m_dMatches.GetLength() )
			continue;

		// set base schema only from non-empty result set
		if ( !bSchemaBaseSet )
		{
			bSchemaBaseSet = true;
			tRes.m_tSchema = tRes.m_dResults[i].m_tSchema;
			continue;
		}

		if ( !MinimizeSchema ( tRes.m_tSchema, tRes.m_dResults[i].m_tSchema ) )
			bAllEqual = false;
	}

	// still want to set base schema from one of the result set
	if ( !bSchemaBaseSet && bAllEqual && tRes.m_dResults.GetLength() )
		tRes.m_tSchema = tRes.m_dResults[0].m_tSchema;

	return bAllEqual;
}

static bool MergeAllMatches ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bHaveLocals, bool bAllEqual, bool bMaster, const CSphFilterSettings * pAggrFilter, QueryProfile_c * pProfiler )
{
	ESphSortOrder eQuerySort = ( tQuery.m_sOuterOrderBy.IsEmpty() ? SPH_SORT_RELEVANCE : SPH_SORT_EXTENDED );
	CSphQuery tQueryCopy = tQuery;

	// got outer order? gotta do a couple things
	if ( tQueryCopy.m_bHasOuter )
	{
		// first, temporarily patch up sorting clause and max_matches (we will restore them later)
		Swap ( tQueryCopy.m_sOuterOrderBy, tQueryCopy.m_sGroupBy.IsEmpty() ? tQueryCopy.m_sSortBy : tQueryCopy.m_sGroupSortBy );
		Swap ( eQuerySort, tQueryCopy.m_eSort );

		// second, apply inner limit now, before (!) reordering
		for ( auto & tResult : tRes.m_dResults )
			tResult.ClampMatches ( tQueryCopy.m_iLimit );
	}

	// so we need to bring matches to the schema that the *sorter* wants
	// so we need to create the sorter before conversion
	//
	// create queue
	// at this point, we do not need to compute anything; it all must be here
	SphQueueSettings_t tQueueSettings ( tRes.m_tSchema );
	tQueueSettings.m_pAggrFilter = pAggrFilter;

	// FIXME? probably not right; 20 shards with by 300 matches might be too much
	// but propagating too small inner max_matches to the outer is not right either
	if ( tQueryCopy.m_bHasOuter )
		tQueueSettings.m_iMaxMatches = Min ( tQuery.m_iMaxMatches * tRes.m_dResults.GetLength(), tRes.GetLength() );
	else
		tQueueSettings.m_iMaxMatches = Min ( tQuery.m_iMaxMatches, tRes.GetLength() );

	tQueueSettings.m_iMaxMatches = Max ( tQueueSettings.m_iMaxMatches, 1 );
	tQueueSettings.m_bGrouped = true;

	SphQueueRes_t tQueueRes;
	std::unique_ptr<ISphMatchSorter> pSorter ( sphCreateQueue ( tQueueSettings, tQueryCopy, tRes.m_sError, tQueueRes ) );

	// restore outer order related patches, or it screws up the query log
	if ( tQueryCopy.m_bHasOuter )
	{
		Swap ( tQueryCopy.m_sOuterOrderBy, tQueryCopy.m_sGroupBy.IsEmpty() ? tQueryCopy.m_sSortBy : tQueryCopy.m_sGroupSortBy );
		Swap ( eQuerySort, tQueryCopy.m_eSort );
	}

	if ( !pSorter )
		return false;

	pSorter->SetMerge(true);

	// reset bAllEqual flag if sorter makes new attributes
	if ( bAllEqual )
	{
		// at first we count already existed internal attributes
		// then check if sorter makes more
		int iRemapCount = GetStringRemapCount ( tRes.m_tSchema, tRes.m_tSchema );
		int iNewCount = GetStringRemapCount ( *pSorter->GetSchema(), tRes.m_tSchema );

		bAllEqual = ( iNewCount<=iRemapCount );
	}

	// sorter expects this

	// just doing tRes.m_tSchema = *pSorter->GetSchema() won't work here
	// because pSorter->GetSchema() may already contain a pointer to tRes.m_tSchema as m_pIndexSchema
	// that's why we explicitly copy a CSphRsetSchema to a plain CSphSchema and move it to tRes.m_tSchema
	{
		CSphSchema tSchemaCopy;
		tSchemaCopy = *pSorter->GetSchema();
		tRes.m_tSchema.Swap ( tSchemaCopy );
	}

	// convert all matches to sorter schema - at least to manage all static to dynamic
	if ( !bAllEqual )
	{
		// post-limit stuff first
		if ( bHaveLocals )
		{
			CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_EVAL_POST );
			ProcessLocalPostlimit ( tRes, tQueryCopy, bMaster );
		}

		RemapResult ( tRes );
	}

	// do the sort work!
	tRes.m_iTotalMatches -= KillDupesAndFlatten ( pSorter.get(), tRes );
	return true;
}

static bool ApplyOuterOrder ( AggrResult_t & tRes, const CSphQuery & tQuery )
{
	assert ( !tRes.m_dResults.IsEmpty() );
	// reorder (aka outer order)
	ESphSortFunc eFunc;
	GenericMatchSort_fn tReorder;
	tReorder.m_fnStrCmp = GetStringCmpFunc ( tQuery.m_eCollation );
	CSphVector<ExtraSortExpr_t> dExtraExprs;

	ESortClauseParseResult eRes = sphParseSortClause ( tQuery, tQuery.m_sOuterOrderBy.cstr(), tRes.m_tSchema, eFunc, tReorder, dExtraExprs, nullptr, tRes.m_sError );
	if ( eRes == SORT_CLAUSE_RANDOM )
		tRes.m_sError = "order by rand() not supported in outer select";

	if ( eRes != SORT_CLAUSE_OK )
		return false;

	assert ( eFunc==FUNC_GENERIC1 ||eFunc==FUNC_GENERIC2 || eFunc==FUNC_GENERIC3 || eFunc==FUNC_GENERIC4 || eFunc==FUNC_GENERIC5 );
	auto & dMatches = tRes.m_dResults.First().m_dMatches;
	sphSort ( dMatches.Begin(), dMatches.GetLength(), tReorder, MatchSortAccessor_t() );
	return true;
}

static void ComputePostlimit ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bMaster )
{
	assert ( tRes.m_bSingle );
	assert ( tRes.m_bOneSchema );
	assert ( !tRes.m_dResults.IsEmpty () );

	CSphVector<const CSphColumnInfo *> dPostlimit;
	ExtractPostlimit ( tRes.m_tSchema, bMaster, dPostlimit );

	// post compute matches only between offset..limit
	// however at agent we can't estimate limit.offset at master merged result set
	// but master don't provide offset to agents only offset+limit as limit
	// so computing all matches from 0 up to inner.limit/outer.limit
	assert ( tRes.GetLength ()==tRes.m_dResults.First().m_dMatches.GetLength() );
	int iOff = Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
	int iLimit = (tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit);

	if ( tRes.m_bTagsAssigned )
		ProcessMultiPostlimit ( tRes, dPostlimit, tQuery.m_sQuery.cstr(), iOff, iLimit );
	else
		ProcessSinglePostlimit ( tRes.m_dResults.First(), dPostlimit, tQuery.m_sQuery.cstr(), iOff, iLimit );
}

/// merges multiple result sets, remaps columns, does reorder for outer selects
bool MinimizeAggrResult ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bHaveLocals, const sph::StringSet & hExtraColumns, QueryProfile_c * pProfiler, const CSphFilterSettings * pAggrFilter, bool bForceRefItems, bool bMaster )
{
	bool bReturnZeroCount = !tRes.m_dZeroCount.IsEmpty();
	bool bQueryFromAPI = tQuery.m_eQueryType==QUERY_API;

	// 0 matches via SphinxAPI? no fiddling with schemes is necessary
	// (and via SphinxQL, we still need to return the right schema)
	// 0 result set schemes via SphinxQL? just bail
	if ( tRes.IsEmpty() && ( bQueryFromAPI || !bReturnZeroCount ) )
	{
		Debug ( tRes.m_bSingle = true; )
		if ( !tRes.m_dResults.IsEmpty () )
		{
			tRes.m_tSchema = tRes.m_dResults.First ().m_tSchema;
			Debug( tRes.m_bOneSchema = true; )
		}
		return true;
	}

	Debug ( tRes.m_bSingle = tRes.m_dResults.GetLength ()==1; )

	// build a minimal schema over all the (potentially different) schemes
	// that we have in our aggregated result set
	assert ( tRes.m_dResults.GetLength() || bReturnZeroCount );

	bool bAllEqual = MinimizeSchemas(tRes);

	Debug ( tRes.m_bOneSchema = tRes.m_bSingle; )

	const CSphVector<CSphQueryItem> & dQueryItems = ( tQuery.m_bFacet || tQuery.m_bFacetHead || bForceRefItems ) ? tQuery.m_dRefItems : tQuery.m_dItems;

	// build a list of select items that the query asked for
	bool bHaveExprs = false;
	CSphVector<CSphQueryItem> tExtItems;
	const CSphVector<CSphQueryItem> & dItems = ExpandAsterisk ( tRes.m_tSchema, dQueryItems, tExtItems, tQuery, bHaveExprs );

	// api + index without attributes + select * case
	// can not skip aggregate filtering
	if ( bQueryFromAPI && dItems.IsEmpty() && !pAggrFilter && !bHaveExprs )
	{
		tRes.ClampAllMatches();
		return true;
	}

	// build the final schemas!
	FrontendSchemaBuilder_c tFrontendBuilder ( tRes, tQuery, dItems, dQueryItems, hExtraColumns, bQueryFromAPI, bHaveLocals );
	if ( !tFrontendBuilder.Build ( bMaster, tRes.m_sError ) )
		return false;

	// tricky bit
	// in purely distributed case, all schemas are received from the wire, and miss aggregate functions info
	// thus, we need to re-assign that info
	if ( !bHaveLocals )
		RecoverAggregateFunctions ( tQuery, tRes );

	// if there's more than one result set,
	// we now have to merge and order all the matches
	// this is a good time to apply outer order clause, too
	if ( tRes.m_iSuccesses>1 || pAggrFilter )
	{
		if ( !MergeAllMatches ( tRes, tQuery, bHaveLocals, bAllEqual, bMaster, pAggrFilter, pProfiler ) )
			return false;
	} else
	{
		tRes.m_dResults.First().m_iTag = 0;
		Debug ( tRes.m_bTagsCompacted = true );
		Debug ( tRes.m_bIdxByTag = true; )
	}

	// apply outer order clause to single result set
	// (multiple combined sets just got reordered above)
	// apply inner limit first
	if ( tRes.m_iSuccesses==1 && tQuery.m_bHasOuter )
	{
		tRes.ClampMatches ( tQuery.m_iLimit );
		if ( !tQuery.m_sOuterOrderBy.IsEmpty() )
		{
			if ( !ApplyOuterOrder ( tRes, tQuery ) )
				return false;
		}
		Debug ( tRes.m_bSingle = true; )
		Debug ( tRes.m_bTagsCompacted = true );
		Debug ( tRes.m_bIdxByTag = true; )
	}

	if ( bAllEqual && bHaveLocals )
	{
		CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_EVAL_POST );
		ComputePostlimit ( tRes, tQuery, bMaster );
	}

	if ( bMaster )
	{
		CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_EVAL_GETFIELD );
		RemotesGetField ( tRes, tQuery );
	}

	// all the merging and sorting is now done
	// replace the minimized matches schema with its subset, the result set schema
	CSphSchema tOldSchema = tRes.m_tSchema;
	tFrontendBuilder.PopulateSchema ( tRes.m_tSchema );

        // FIX(#3428): Adjust SHOW META -> total_found only when HAVING is present.
        // After flattening/merging, m_dResults[0] holds the post-HAVING groups.
        // Cap total_found to the number of survivors to reflect post-HAVING reality.
        // Same HAVING presence condition as used in MergeAllMatches: pAggrFilter != nullptr.
        const bool bHasHaving = ( pAggrFilter != nullptr );
        if ( bHasHaving && !tRes.m_dResults.IsEmpty() )
        {
                const int iHavingSurvivors = tRes.m_dResults[0].m_dMatches.GetLength();
                if ( iHavingSurvivors < tRes.m_iTotalMatches )
                        tRes.m_iTotalMatches = iHavingSurvivors;
        }

	if ( tRes.m_iSuccesses==1 )
		RemapNullMask ( tRes.m_dResults[0].m_dMatches, tOldSchema, tRes.m_tSchema );

	return true;
}
