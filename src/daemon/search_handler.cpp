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

#include "search_handler.h"

#include "std/string.h"

#include "sphinxdefs.h"
#include "queuecreator.h"
#include "joinsorter.h"
#include "pseudosharding.h"
#include "threadutils.h"
#include "dynamic_idx.h"
#include "api_search.h"
#include "logger.h"
#include "searchdsql.h"
#include "debug_cmds.h"
#include "schematransform.h"
#include "frontendschema.h"

static bool LOG_LEVEL_LOCAL_SEARCH = val_from_env ( "MANTICORE_LOG_LOCAL_SEARCH", false ); // verbose logging local search events, ruled by this env variable
#define LOG_COMPONENT_LOCSEARCHINFO __LINE__ << " "
#define LOCSEARCHINFO LOGINFO ( LOCAL_SEARCH, LOCSEARCHINFO )

constexpr int DAEMON_MAX_RETRY_COUNT = 8;
constexpr int DAEMON_MAX_RETRY_DELAY = 1000;

extern int g_iQueryLogMinMs; // log 'slow' threshold for query, defined in searchd.cpp

using namespace Threads;

void CheckQuery ( const CSphQuery & tQuery, CSphString & sError, bool bCanLimitless )
{
	#define LOC_ERROR( ... ) do { sError.SetSprintf (__VA_ARGS__); return; } while(0)

	sError = nullptr;

	if ( (int)tQuery.m_eMode<0 || tQuery.m_eMode>SPH_MATCH_TOTAL )
		LOC_ERROR ( "invalid match mode %d", tQuery.m_eMode );

	if ( (int)tQuery.m_eRanker<0 || tQuery.m_eRanker>SPH_RANK_TOTAL )
		LOC_ERROR ( "invalid ranking mode %d", tQuery.m_eRanker );

	if ( tQuery.m_iMaxMatches<1 )
		LOC_ERROR ( "max_matches can not be less than one" );

	if ( tQuery.m_iOffset<0 || tQuery.m_iOffset>=tQuery.m_iMaxMatches )
		LOC_ERROR ( "offset out of bounds (offset=%d, max_matches=%d)", tQuery.m_iOffset, tQuery.m_iMaxMatches );

	if ( tQuery.m_iLimit < ( bCanLimitless ? -1 : 0 ) ) // -1 is magic for 'limitless select'
		LOC_ERROR ( "limit out of bounds (limit=%d)", tQuery.m_iLimit );

	if ( tQuery.m_iCutoff<-1 )
		LOC_ERROR ( "cutoff out of bounds (cutoff=%d)", tQuery.m_iCutoff );

	if ( ( tQuery.m_iRetryCount!=-1 ) && ( tQuery.m_iRetryCount>DAEMON_MAX_RETRY_COUNT ) )
		LOC_ERROR ( "retry count out of bounds (count=%d)", tQuery.m_iRetryCount );

	if ( ( tQuery.m_iRetryDelay!=-1 ) && ( tQuery.m_iRetryDelay>DAEMON_MAX_RETRY_DELAY ) )
		LOC_ERROR ( "retry delay out of bounds (delay=%d)", tQuery.m_iRetryDelay );

	if ( tQuery.m_iOffset>0 && tQuery.m_bHasOuter )
		LOC_ERROR ( "inner offset must be 0 when using outer order by (offset=%d)", tQuery.m_iOffset );

	#undef LOC_ERROR
}

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
const CSphVector<CSphQueryItem> & ExpandAsterisk ( const ISphSchema & tSchema, const CSphVector<CSphQueryItem> & dItems, CSphVector<CSphQueryItem> & tExpanded, const CSphQuery & tQuery, bool & bHaveExprs )
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

// in MatchIterator_c we need matches sorted assending by DocID.
// also we don't want to sort matches themselves; sorted vec of indexes quite enough
// also we wont to avoid allocating vec for the matches as it may be huge.
// There are several possible solutions to have vec of indexes:
// 1. Use matches tags, as they're not used in this part of code. With intensive working it is however not a good in
// terms of cache misses (i.e. 'min' match is match[N] where N is match[0].tag, then match[M] where M is match[1] tag.
// So each time we make about random jumps.
// 2. Use space between last match and end of the vector (assuming reserved space > used space). If it is enough space,
// we can use it either as vec or WORDS, or as vec or DWORDS, depending from N of matches. First case need at most 128K
// of RAM, second needs more, but that RAM is compact.
// So, let's try with tail space first, but if it is not available (no, or not enough space), use tags.


// That is to sort tags in matches without moving rest of them.
class MatchTagSortAccessor_c
{
	const VecTraits_T<CSphMatch> & m_dTagOrder;
public:
	explicit MatchTagSortAccessor_c ( const VecTraits_T<CSphMatch> & dTagOrder) : m_dTagOrder ( dTagOrder ) {}
	using T = CSphMatch;
	using MEDIAN_TYPE = int;
	static MEDIAN_TYPE Key ( T * a ) { return a->m_iTag; }
	static void Swap ( T * a, T * b ) { ::Swap ( a->m_iTag, b->m_iTag ); }
	static T * Add ( T * p, int i ) { return p+i; }
	static int Sub ( T * b, T * a ) { return (int)(b-a); }
	static void CopyKey ( MEDIAN_TYPE * pMed, CSphMatch * pVal ) { *pMed = Key ( pVal ); }

	bool IsLess ( int a, int b ) const
	{
		return sphGetDocID ( m_dTagOrder[a].m_pDynamic )<sphGetDocID ( m_dTagOrder[b].m_pDynamic );
	}
};


class MatchIterator_c
{
	int m_iRawIdx;    // raw iteration index (internal)
	int m_iLimit;
	std::function<int(int)> m_fnOrder;	// use to access matches by accending docid order
	bool m_bTailClean = false;

	// use space after end of matches to store indexes, WORD per match
	bool MaybeUseWordOrder ( const CSphSwapVector<CSphMatch>& dMatches ) const
	{
		if ( dMatches.GetLength()>0x10000 )
			return false;

		int64_t iTail = dMatches.AllocatedBytes ()-dMatches.GetLengthBytes64 ();
		if ( iTail<(int64_t) ( dMatches.GetLength () * sizeof ( WORD ) ) )
			return false;

		// will use tail of the vec as blob of WORDs
		VecTraits_T<WORD> dOrder = { (WORD *) dMatches.end (), m_iLimit };
		ARRAY_CONSTFOREACH( i, dOrder )
			dOrder[i] = i;
		dOrder.Sort ( Lesser ( [&dMatches] ( WORD a, WORD b ) {
			return sphGetDocID ( dMatches[a].m_pDynamic )<sphGetDocID ( dMatches[b].m_pDynamic );
		} ) );
		return true;
	}

	// use space after end of matches to store indexes, DWORD per match
	bool MaybeUseDwordOrder ( const CSphSwapVector<CSphMatch>& dMatches ) const
	{
		if ( dMatches.GetLength64()>0x100000000 )
			return false;

		int64_t iTail = dMatches.AllocatedBytes ()-dMatches.GetLengthBytes64 ();
		if ( iTail<(int64_t) ( dMatches.GetLength () * sizeof ( DWORD ) ) )
			return false;

		// will use tail of the vec as blob of WORDs
		VecTraits_T<DWORD> dOrder = { (DWORD *) dMatches.end (), m_iLimit };
		for( DWORD i=0, uLen=dOrder.GetLength(); i<uLen; ++i )
			dOrder[i] = i;
		dOrder.Sort ( Lesser ( [&dMatches] ( DWORD a, DWORD b ) {
			return sphGetDocID ( dMatches[a].m_pDynamic )<sphGetDocID ( dMatches[b].m_pDynamic );
		} ) );
		return true;
	}

	// use tags to store indexes. No extra space, but random access order, many cash misses expected
	void UseTags ( VecTraits_T<CSphMatch> & dOrder )
	{
		ARRAY_CONSTFOREACH( i, dOrder )
			dOrder[i].m_iTag = i;

		MatchTagSortAccessor_c tOrder ( dOrder );
		sphSort ( dOrder.Begin (), dOrder.GetLength (), tOrder, tOrder );
		m_bTailClean = true;
	}

public:
	OneResultset_t&			m_tResult;
	DocID_t					m_tDocID;
	int						m_iIdx;		// ordering index (each step gives matches in sorted by Docid order)

	explicit MatchIterator_c ( OneResultset_t & tResult )
		: m_tResult ( tResult )
	{
		auto& dMatches = tResult.m_dMatches;
		m_iLimit = dMatches.GetLength();

		if ( MaybeUseWordOrder ( dMatches ) )
			m_fnOrder = [pData = (WORD *) m_tResult.m_dMatches.end ()] ( int i ) { return pData[i]; };
		else if ( MaybeUseDwordOrder ( dMatches ) )
			m_fnOrder = [pData = (DWORD *) m_tResult.m_dMatches.end ()] ( int i ) { return pData[i]; };
		else
		{
			UseTags ( dMatches );
			m_fnOrder = [this] ( int i ) { return m_tResult.m_dMatches[m_iRawIdx].m_iTag; };
		}

		m_iRawIdx = 0;
		m_iIdx = m_fnOrder(0);

		assert ( m_tResult.m_tSchema.GetAttr ( sphGetDocidName() ) );
		m_tDocID = sphGetDocID ( m_tResult.m_dMatches[m_iIdx].m_pDynamic );
	}

	~MatchIterator_c()
	{
		if ( m_bTailClean )
			return;

		// need to reset state of some tail matches in order to avoid issues when deleting the vec of them
		// (since we used that memory region for own purposes)
		int iDirtyMatches = m_iLimit>0x10000 ? m_iLimit * sizeof ( DWORD ) : m_iLimit * sizeof ( WORD );
		iDirtyMatches = ( iDirtyMatches+sizeof ( CSphMatch )-1 ) / sizeof ( CSphMatch );
		for ( int i = 0; i<iDirtyMatches; ++i )
			( m_tResult.m_dMatches.end ()+i )->CleanGarbage();
	}

	bool Step()
	{
		++m_iRawIdx;
		if ( m_iRawIdx>=m_iLimit )
			return false;
		m_iIdx = m_fnOrder ( m_iRawIdx );
		m_tDocID = sphGetDocID ( m_tResult.m_dMatches[m_iIdx].m_pDynamic );
		return true;
	}

	static bool IsLess ( MatchIterator_c *a, MatchIterator_c *b )
	{
		if ( a->m_tDocID!=b->m_tDocID )
			return a->m_tDocID<b->m_tDocID;

		// that mean local matches always preffered over remote, but it seems that is not necessary
//		if ( !a->m_dResult.m_bTag && b->m_dResult.m_bTag )
//			return true;

		return a->m_tResult.m_iTag>b->m_tResult.m_iTag;
	}
};

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
		for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
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

	default:
		tMatch.SetAttrFloat ( pCol->m_tLocator, pCol->m_pExpr->Eval ( tMatch ) );
		break;
	}
}

// single resultset cunk, but has many tags
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
	for ( int i = 0, iAttrsCount = tRes.m_tSchema.GetAttrsCount (); i<iAttrsCount && !bGotPostlimit; ++i )
	{
		const CSphColumnInfo & tCol = tRes.m_tSchema.GetAttr(i);
		bGotPostlimit = ( tCol.m_eStage==SPH_EVAL_POSTLIMIT && ( bMaster || tCol.m_uFieldFlags==CSphColumnInfo::FIELD_NONE ) );
	}

	if ( !bGotPostlimit )
		return;

	int iLimit = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );
	iLimit += Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
	CSphVector<const CSphColumnInfo *> dPostlimit;
	for ( auto & tResult : tRes.m_dResults )
	{
		dPostlimit.Resize ( 0 );
		ExtractPostlimit ( tResult.m_tSchema, bMaster, dPostlimit );
		if ( dPostlimit.IsEmpty () )
			continue;

		iLimit = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );

		// we can't estimate limit.offset per result set
		// as matches got merged and sort next step
		if ( !tResult.m_bTag )
			ProcessSinglePostlimit ( tResult, dPostlimit, tQuery.m_sQuery.cstr (), 0, iLimit );
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
	if ( eRes==SORT_CLAUSE_RANDOM )
		tRes.m_sError = "order by rand() not supported in outer select";

	if ( eRes!=SORT_CLAUSE_OK )
		return false;

	assert ( eFunc==FUNC_GENERIC1 ||eFunc==FUNC_GENERIC2 || eFunc==FUNC_GENERIC3 || eFunc==FUNC_GENERIC4 || eFunc==FUNC_GENERIC5 );
	auto& dMatches = tRes.m_dResults.First().m_dMatches;
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
	int iLimit = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );

	if ( tRes.m_bTagsAssigned )
		ProcessMultiPostlimit ( tRes, dPostlimit, tQuery.m_sQuery.cstr (), iOff, iLimit );
	else
		ProcessSinglePostlimit ( tRes.m_dResults.First(), dPostlimit, tQuery.m_sQuery.cstr(), iOff, iLimit );
}

/// merges multiple result sets, remaps columns, does reorder for outer selects
static bool MinimizeAggrResult ( AggrResult_t & tRes, const CSphQuery & tQuery, bool bHaveLocals, const sph::StringSet & hExtraColumns, QueryProfile_c * pProfiler, const CSphFilterSettings * pAggrFilter, bool bForceRefItems, bool bMaster )
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

	if ( tRes.m_iSuccesses==1 )
		RemapNullMask ( tRes.m_dResults[0].m_dMatches, tOldSchema, tRes.m_tSchema );

	return true;
}

/////////////////////////////////////////////////////////////////////////////

PubSearchHandler_c::PubSearchHandler_c ( int iQueries, std::unique_ptr<QueryParser_i> pQueryParser, QueryType_e eQueryType, bool bMaster )
	: m_pImpl { std::make_unique<SearchHandler_c> ( iQueries, std::move ( pQueryParser ), eQueryType, bMaster ) }
{
	assert ( m_pImpl );
}

PubSearchHandler_c::~PubSearchHandler_c () = default;

void PubSearchHandler_c::RunQueries ()
{
	m_pImpl->RunQueries();
}

void PubSearchHandler_c::SetQuery ( int iQuery, const CSphQuery & tQuery, std::unique_ptr<ISphTableFunc> pTableFunc )
{
	m_pImpl->SetQuery ( iQuery, tQuery, std::move(pTableFunc) );
}

void PubSearchHandler_c::SetJoinQueryOptions ( int iQuery, const CSphQuery & tJoinQueryOptions )
{
	m_pImpl->SetJoinQueryOptions ( iQuery, tJoinQueryOptions );
}

void PubSearchHandler_c::SetProfile ( QueryProfile_c * pProfile )
{
	m_pImpl->SetProfile ( pProfile );
}

void PubSearchHandler_c::SetStmt ( SqlStmt_t & tStmt )
{
	m_pImpl->m_pStmt = &tStmt;
}

AggrResult_t * PubSearchHandler_c::GetResult ( int iResult )
{
	return m_pImpl->GetResult (iResult);
}

void PubSearchHandler_c::PushIndex ( const CSphString& sIndex, const cServedIndexRefPtr_c& pDesc )
{
	m_pImpl->m_dAcquired.AddIndex ( sIndex, pDesc );
}

void PubSearchHandler_c::RunCollect ( const CSphQuery& tQuery, const CSphString& sIndex, CSphString* pErrors, CSphVector<BYTE>* pCollectedDocs )
{
	m_pImpl->RunCollect ( tQuery, sIndex, pErrors, pCollectedDocs );
}


SearchHandler_c::SearchHandler_c ( int iQueries, std::unique_ptr<QueryParser_i> pQueryParser, QueryType_e eQueryType, bool bMaster )
	: m_dTables ( iQueries )
{
	m_dQueries.Resize ( iQueries );
	m_dJoinQueryOptions.Resize ( iQueries );
	m_dAggrResults.Resize ( iQueries );
	m_dFailuresSet.Resize ( iQueries );
	m_dAgentTimes.Resize ( iQueries );
	m_bMaster = bMaster;
	m_bFederatedUser = false;

	SetQueryParser ( std::move ( pQueryParser ), eQueryType );
	m_dResults.Resize ( iQueries );
	for ( int i=0; i<iQueries; ++i )
		m_dResults[i].m_pMeta = &m_dAggrResults[i];

	// initial slices (when nothing explicitly asked)
	m_dNQueries = m_dQueries;
	m_dNJoinQueryOptions = m_dJoinQueryOptions;
	m_dNAggrResults = m_dAggrResults;
	m_dNResults = m_dResults;
	m_dNFailuresSet = m_dFailuresSet;
}

//////////////////
/* Smart gc retire of vec of queries.
 * We have CSphVector<CSphQuery> which is over, but some threads may still use separate queries from it, so we can't just
 * delete it, since they will loose the objects and it will cause crash.
 *
 * So, if some queries are still in use, we retire them with custom deleter, which will decrease counter,
 * and finally delete whole vec.
*/
class RetireQueriesVec_c
{
	CSphVector<CSphQuery>	m_dQueries;	// given queries I'll finally remove
	std::atomic<int>		m_iInUse;	// how many of them still reffered

	void OneQueryDeleted()
	{
		if ( m_iInUse.fetch_sub ( 1, std::memory_order_release )==1 )
		{
			assert( m_iInUse.load ( std::memory_order_acquire )==0 );
			delete this;
		}
	}

	static void Delete ( void * pArg )
	{
		if ( pArg )
		{
			auto pMe = (RetireQueriesVec_c *) ( (CSphQuery *) pArg )->m_pCookie;
			assert ( pMe && "Each retiring query from vec must have address of RetireQueriesVec_c in cookie");
			if ( pMe )
				pMe->OneQueryDeleted ();
		}
	}

public:
	void EngageRetiring ( CSphVector<CSphQuery> dQueries, CSphVector<int> dRetired )
	{
		assert ( !dRetired.IsEmpty () );
		m_iInUse.store ( dRetired.GetLength (), std::memory_order_release );
		m_dQueries = std::move ( dQueries );
		for ( auto iRetired: dRetired )
		{
			m_dQueries[iRetired].m_pCookie = this;
			hazard::Retire ( (void*) &m_dQueries[iRetired], Delete );
		}
	}
};

SearchHandler_c::~SearchHandler_c ()
{
	auto dPointed = hazard::GetListOfPointed ( m_dQueries );
	if ( !dPointed.IsEmpty () )
	{
		// pQueryHolder will be self-removed when all used queries retired
		auto pQueryHolder = new RetireQueriesVec_c;
		pQueryHolder->EngageRetiring ( std::move ( m_dQueries ), std::move ( dPointed ) );
	}
}

void SearchHandler_c::SetQueryParser ( std::unique_ptr<QueryParser_i> pParser, QueryType_e eQueryType )
{
	m_pQueryParser = std::move ( pParser );
	m_eQueryType = eQueryType;
	for ( auto & dQuery : m_dQueries )
	{
		dQuery.m_pQueryParser = m_pQueryParser.get();
		dQuery.m_eQueryType = eQueryType;
	}
}

bool KeepCollection_c::AddUniqIndex ( const CSphString & sName )
{
	if ( m_hIndexes.Exists ( sName ) )
		return true;

	auto pIdx = GetServed ( sName );
	if ( !pIdx )
		return false;

	m_hIndexes.Add ( std::move ( pIdx ), sName );
	return true;
}

void KeepCollection_c::AddIndex ( const CSphString & sName, cServedIndexRefPtr_c pIdx )
{
	if ( m_hIndexes.Exists ( sName ) )
		return;

	m_hIndexes.Add ( std::move ( pIdx ), sName );
}

cServedIndexRefPtr_c KeepCollection_c::Get ( const CSphString & sName ) const
{
	auto * ppIndex = m_hIndexes ( sName );
	assert ( ppIndex && "KeepCollection_c::Get called with absent key");
	return *ppIndex;
}

void SearchHandler_c::RunCollect ( const CSphQuery &tQuery, const CSphString &sIndex, CSphString * pErrors, CSphVector<BYTE> * pCollectedDocs )
{
	m_bQueryLog = false;
	m_pCollectedDocs = pCollectedDocs;
	RunActionQuery ( tQuery, sIndex, pErrors );
}

void SearchHandler_c::RunActionQuery ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors )
{
	SetQuery ( 0, tQuery, nullptr );
	m_dQueries[0].m_sIndexes = sIndex;
	m_dLocal.Add ().m_sName = sIndex;

	CheckQuery ( tQuery, *pErrors );
	if ( !pErrors->IsEmpty() )
		return;

	int64_t tmLocal = -sphMicroTimer();
	int64_t tmCPU = -sphTaskCpuTimer ();

	RunLocalSearches();
	tmLocal += sphMicroTimer();
	tmCPU += sphTaskCpuTimer();

	OnRunFinished();

	auto & tRes = m_dAggrResults[0];

	tRes.m_iOffset = tQuery.m_iOffset;
	tRes.m_iCount = Max ( Min ( tQuery.m_iLimit, tRes.GetLength()-tQuery.m_iOffset ), 0 );
	// actualy tRes.m_iCount=0 since delete/update produces no matches

	tRes.m_iQueryTime += (int)(tmLocal/1000);
	tRes.m_iCpuTime += tmCPU;

	if ( !tRes.m_iSuccesses )
	{
		StringBuilder_c sFailures;
		m_dFailuresSet[0].BuildReport ( sFailures );
		sFailures.MoveTo ( *pErrors );

	} else if ( !tRes.m_sError.IsEmpty() )
	{
		StringBuilder_c sFailures;
		m_dFailuresSet[0].BuildReport ( sFailures );
		sFailures.MoveTo ( tRes.m_sWarning ); // FIXME!!! commit warnings too
	}

	const CSphIOStats & tIO = tRes.m_tIOStats;

	auto & g_tStats = gStats ();
	g_tStats.m_iQueries.fetch_add ( 1, std::memory_order_relaxed );
	g_tStats.m_iQueryTime.fetch_add ( tmLocal, std::memory_order_relaxed );
	g_tStats.m_iQueryCpuTime.fetch_add ( tmLocal, std::memory_order_relaxed );
	g_tStats.m_iDiskReads.fetch_add ( tIO.m_iReadOps, std::memory_order_relaxed );
	g_tStats.m_iDiskReadTime.fetch_add ( tIO.m_iReadTime, std::memory_order_relaxed );
	g_tStats.m_iDiskReadBytes.fetch_add ( tIO.m_iReadBytes, std::memory_order_relaxed );

	if ( m_bQueryLog )
		LogQuery ( m_dQueries[0], m_dJoinQueryOptions[0], m_dAggrResults[0], m_dAgentTimes[0] );
}

void SearchHandler_c::SetQuery ( int iQuery, const CSphQuery & tQuery, std::unique_ptr<ISphTableFunc> pTableFunc )
{
	m_dQueries[iQuery] = tQuery;
	m_dQueries[iQuery].m_pQueryParser = m_pQueryParser.get();
	m_dQueries[iQuery].m_eQueryType = m_eQueryType;
	m_dTables[iQuery] = std::move ( pTableFunc );
}


void SearchHandler_c::SetProfile ( QueryProfile_c * pProfile )
{
	assert ( pProfile );
	m_pProfile = pProfile;
}


void SearchHandler_c::RunQueries()
{
	// batch queries to same index(es)
	// or work each query separately if indexes are different

	int iStart = 0;
	ARRAY_FOREACH ( i, m_dQueries )
	{
		if ( m_dQueries[i].m_sIndexes!=m_dQueries[iStart].m_sIndexes )
		{
			RunSubset ( iStart, i );
			iStart = i;
		}
	}
	RunSubset ( iStart, m_dQueries.GetLength() );
	if ( m_bQueryLog )
	{
		ARRAY_FOREACH ( i, m_dQueries )
			LogQuery ( m_dQueries[i], m_dJoinQueryOptions[i], m_dAggrResults[i], m_dAgentTimes[i] );
	}
	// no need to call OnRunFinished() as meta.matches already calculated at search
}


// final fixup
void SearchHandler_c::OnRunFinished()
{
	for ( auto & tResult : m_dAggrResults )
		tResult.m_iMatches = tResult.GetLength();
}

// return sequence of columns as 'show create table', or 'describe' reveal
static StrVec_t GetDefaultSchema ( const CSphIndex* pIndex )
{
	StrVec_t dRes;
	auto& tSchema = pIndex->GetMatchSchema();
	if ( tSchema.GetAttrsCount()==0 )
		return dRes;
	assert ( tSchema.GetAttr ( 0 ).m_sName == sphGetDocidName() );
	const auto& tId = tSchema.GetAttr ( 0 );
	dRes.Add ( tId.m_sName );

	for ( int i = 0; i < tSchema.GetFieldsCount(); ++i )
	{
		const auto& tField = tSchema.GetField ( i );
		dRes.Add ( tField.m_sName );
	}

	for ( int i = 1; i < tSchema.GetAttrsCount(); ++i ) // from 1, as 0 is docID and already emerged
	{
		const auto& tAttr = tSchema.GetAttr ( i );
		if ( sphIsInternalAttr ( tAttr ) )
			continue;

		if ( tSchema.GetField ( tAttr.m_sName.cstr() ) )
			continue; // already described it as a field property

		dRes.Add ( tAttr.m_sName );
	}
	return dRes;
}

static int GetMaxMatches ( int iQueryMaxMatches, const CSphIndex * pIndex )
{
	if ( iQueryMaxMatches<=DEFAULT_MAX_MATCHES )
		return iQueryMaxMatches;

	int64_t iDocs = Min ( (int)INT_MAX, pIndex->GetStats().m_iTotalDocuments ); // clamp to int max
	return Min ( iQueryMaxMatches, Max ( iDocs, DEFAULT_MAX_MATCHES ) ); // do not want 0 sorter and sorter longer than query.max_matches
}

SphQueueSettings_t SearchHandler_c::MakeQueueSettings ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, int iMaxMatches, bool bForceSingleThread, ISphExprHook * pHook ) const
{
	auto& tSess = session::Info();

	SphQueueSettings_t tQS ( pIndex->GetMatchSchema (), m_pProfile, tSess.m_pSqlRowBuffer, &tSess.m_pSessionOpaque1, &tSess.m_pSessionOpaque2 );
	tQS.m_bComputeItems = true;
	tQS.m_pCollection = m_pCollectedDocs;
	tQS.m_pHook = pHook;
	tQS.m_iMaxMatches = GetMaxMatches ( iMaxMatches, pIndex );
	tQS.m_bNeedDocids = m_bNeedDocIDs;	// need docids to merge results from indexes
	tQS.m_fnGetCountDistinct	= [pIndex]( const CSphString & sAttr, CSphString & sModifiedAttr ){ return pIndex->GetCountDistinct ( sAttr, sModifiedAttr ); };
	tQS.m_fnGetCountFilter		= [pIndex]( const CSphFilterSettings & tFilter, CSphString & sModifiedAttr ){ return pIndex->GetCountFilter ( tFilter, sModifiedAttr ); };
	tQS.m_fnGetCount			= [pIndex](){ return pIndex->GetCount(); };
	tQS.m_bEnableFastDistinct = m_dLocal.GetLength()<=1;
	tQS.m_bForceSingleThread = bForceSingleThread;
	tQS.m_dCreateSchema = GetDefaultSchema ( pIndex );
	if ( pJoinedIndex )
		tQS.m_pJoinArgs = std::make_unique<JoinArgs_t> ( pJoinedIndex->GetMatchSchema(), pIndex->GetName(), pJoinedIndex->GetName() );

	return tQS;
}


int SearchHandler_c::CreateMultiQueryOrFacetSorters ( const CSphIndex * pIndex, CSphVector<const CSphIndex*> & dJoinedIndexes, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const
{
	int iValidSorters = 0;

	auto tQueueSettings = MakeQueueSettings ( pIndex, dJoinedIndexes[0], m_dNQueries.First ().m_iMaxMatches, m_dPSInfo.First().m_bForceSingleThread, pHook );
	sphCreateMultiQueue ( tQueueSettings, m_dNQueries, dSorters, dErrors, tQueueRes, pExtra, m_pProfile );

	m_dNQueries.First().m_bZSlist = tQueueRes.m_bZonespanlist;
	dSorters.Apply ( [&iValidSorters] ( const ISphMatchSorter * pSorter ) {
		if ( pSorter )
			++iValidSorters;
	} );
	if ( m_bFacetQueue && iValidSorters<dSorters.GetLength () )
	{
		dSorters.Apply ( [] ( ISphMatchSorter *& pSorter ) { SafeDelete (pSorter); } );
		return 0;
	}

	int iBatchSize = m_dNQueries[0].m_iJoinBatchSize==-1 ? GetJoinBatchSize() : m_dNQueries[0].m_iJoinBatchSize;
	if ( m_bFacetQueue && !CreateJoinMultiSorter ( pIndex, dJoinedIndexes[0], tQueueSettings, m_dNQueries, m_dNJoinQueryOptions, dSorters, iBatchSize, dErrors[0] ) )
	{
		dSorters.Apply ( [] ( ISphMatchSorter *& pSorter ) { SafeDelete (pSorter); } );
		return 0;
	}

	return iValidSorters;
}


int SearchHandler_c::CreateSingleSorters ( const CSphIndex * pIndex, CSphVector<const CSphIndex*> & dJoinedIndexes, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const
{
	int iValidSorters = 0;
	tQueueRes.m_bAlowMulti = false;
	const int iQueries = m_dNQueries.GetLength();
	for ( int iQuery = 0; iQuery<iQueries; ++iQuery )
	{
		CSphQuery & tQuery = m_dNQueries[iQuery];

		// create queue
		auto tQueueSettings = MakeQueueSettings ( pIndex, dJoinedIndexes[iQuery], tQuery.m_iMaxMatches, m_dPSInfo.First().m_bForceSingleThread, pHook );
		ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings, tQuery, dErrors[iQuery], tQueueRes, pExtra, m_pProfile );
		if ( !pSorter )
			continue;

		// possibly create a wrapper (if we have JOIN)
		int iBatchSize = tQuery.m_iJoinBatchSize==-1 ? GetJoinBatchSize() : tQuery.m_iJoinBatchSize;
		pSorter = CreateJoinSorter ( pIndex, dJoinedIndexes[iQuery], tQueueSettings, tQuery, pSorter, m_dNJoinQueryOptions[iQuery], tQueueRes.m_bJoinedGroupSort, iBatchSize, dErrors[iQuery] );
		if ( !pSorter )
			continue;

		tQuery.m_bZSlist = tQueueRes.m_bZonespanlist;
		dSorters[iQuery] = pSorter;
		++iValidSorters;
	}
	return iValidSorters;
}


int SearchHandler_c::CreateSorters ( const CSphIndex * pIndex, CSphVector<const CSphIndex*> & dJoinedIndexes, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t* pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const
{
	if ( m_bMultiQueue || m_bFacetQueue )
		return CreateMultiQueryOrFacetSorters ( pIndex, dJoinedIndexes, dSorters, dErrors, pExtra, tQueueRes, pHook );

	return CreateSingleSorters ( pIndex, dJoinedIndexes, dSorters, dErrors, pExtra, tQueueRes, pHook );
}

static int64_t CalcPredictedTimeMsec ( const CSphQueryResultMeta & tMeta )
{
	assert ( tMeta.m_bHasPrediction );

	int64_t iNanoResult = int64_t(g_iPredictorCostSkip)* tMeta.m_tStats.m_iSkips
		+ g_iPredictorCostDoc * tMeta.m_tStats.m_iFetchedDocs
		+ g_iPredictorCostHit * tMeta.m_tStats.m_iFetchedHits
		+ g_iPredictorCostMatch * tMeta.m_iTotalMatches;

	return iNanoResult/1000000;
}

struct LocalSearchRef_t
{
	ExprHook_c&	m_tHook;
	StrVec_t* m_pExtra;
	VecTraits_T<SearchFailuresLog_c>& m_dFailuresSet;
	VecTraits_T<AggrResult_t>& m_dAggrResults;
	VecTraits_T<CSphQueryResult>& m_dResults;

	LocalSearchRef_t ( ExprHook_c & tHook, StrVec_t* pExtra, VecTraits_T<SearchFailuresLog_c> & dFailures, VecTraits_T<AggrResult_t> & dAggrResults, VecTraits_T<CSphQueryResult> & dResults )
		: m_tHook ( tHook )
		, m_pExtra ( pExtra )
		, m_dFailuresSet ( dFailures )
		, m_dAggrResults ( dAggrResults )
		, m_dResults ( dResults )
	{}

	void MergeChild ( LocalSearchRef_t dChild ) const
	{
		if ( m_pExtra )
		{
			assert ( dChild.m_pExtra );
			m_pExtra->Append ( *dChild.m_pExtra );
		}

		auto & dChildAggrResults = dChild.m_dAggrResults;
		for ( int i = 0, iQueries = m_dAggrResults.GetLength (); i<iQueries; ++i )
		{
			auto & tResult = m_dAggrResults[i];
			auto & tChild = dChildAggrResults[i];

			tResult.m_dResults.Append ( tChild.m_dResults );

			// word statistics
			tResult.MergeWordStats ( tChild );

			// other data (warnings, errors, etc.)
			// errors
			if ( !tChild.m_sError.IsEmpty ())
				tResult.m_sError = tChild.m_sError;

			// warnings
			if ( !tChild.m_sWarning.IsEmpty ())
				tResult.m_sWarning = tChild.m_sWarning;

			// prediction counters
			tResult.m_bHasPrediction |= tChild.m_bHasPrediction;
			if ( tChild.m_bHasPrediction )
			{
				tResult.m_tStats.Add ( tChild.m_tStats );
				tResult.m_iPredictedTime = CalcPredictedTimeMsec ( tResult );
			}

			// profiling
			if ( tChild.m_pProfile )
				tResult.m_pProfile->AddMetric ( *tChild.m_pProfile );

			tResult.m_iCpuTime += tChild.m_iCpuTime;
			tResult.m_iTotalMatches += tChild.m_iTotalMatches;
			tResult.m_bTotalMatchesApprox |= tChild.m_bTotalMatchesApprox;
			tResult.m_iSuccesses += tChild.m_iSuccesses;
			tResult.m_tIOStats.Add ( tChild.m_tIOStats );

			tResult.m_tIteratorStats.Merge ( tChild.m_tIteratorStats );

			// failures
			m_dFailuresSet[i].Append ( dChild.m_dFailuresSet[i] );
		}
	}

	inline static bool IsClonable()
	{
		return true;
	}
};

struct LocalSearchClone_t
{
	ExprHook_c m_tHook;
	StrVec_t m_dExtra;
	StrVec_t* m_pExtra;
	CSphVector<SearchFailuresLog_c> m_dFailuresSet;
	CSphVector<AggrResult_t>	m_dAggrResults;
	CSphVector<CSphQueryResult> m_dResults;

	explicit LocalSearchClone_t ( const LocalSearchRef_t & dParent)
	{
		int iQueries = dParent.m_dFailuresSet.GetLength ();
		m_dFailuresSet.Resize ( iQueries );
		m_dAggrResults.Resize ( iQueries );
		m_dResults.Resize ( iQueries );
		for ( int i=0; i<iQueries; ++i )
			m_dResults[i].m_pMeta = &m_dAggrResults[i];
		m_pExtra = dParent.m_pExtra ? &m_dExtra : nullptr;

		// set profiler complementary to one in RunSubset (search by `if ( iQueries==1 && m_pProfile )` clause)
		if ( iQueries==1 && dParent.m_dAggrResults.First ().m_pProfile )
		{
			auto pProfile = new QueryProfile_c;
			m_dAggrResults.First().m_pProfile = pProfile;
			m_tHook.SetProfiler ( pProfile );
		}
	}
	explicit operator LocalSearchRef_t ()
	{
		return { m_tHook, m_pExtra, m_dFailuresSet, m_dAggrResults, m_dResults };
	}

	~LocalSearchClone_t()
	{
		if ( !m_dAggrResults.IsEmpty () )
			SafeDelete ( m_dAggrResults.First().m_pProfile );
	}
};


cServedIndexRefPtr_c SearchHandler_c::CheckIndexSelectable ( const CSphString & sLocal, const char * szParent, VecTraits_T<SearchFailuresLog_c> * pNFailuresSet ) const
{
	const auto& pServed = m_dAcquired.Get ( sLocal );
	assert ( pServed );

	if ( !ServedDesc_t::IsSelectable ( pServed ) )
	{
		if ( pNFailuresSet )
			for ( auto & dFailureSet : *pNFailuresSet )
				dFailureSet.SubmitEx ( sLocal, nullptr, "%s", "table is not suitable for select" );

		return cServedIndexRefPtr_c{};
	}

	return pServed;
}


bool SearchHandler_c::PopulateJoinedIndexes ( CSphVector<JoinedServedIndex_t> & dJoinedServed, VecTraits_T<SearchFailuresLog_c> & dFailuresSet ) const
{
	dJoinedServed.Resize ( m_dNQueries.GetLength() );
	ARRAY_FOREACH ( i, m_dNQueries )
	{
		const auto & tQuery = m_dNQueries[i];
		if ( tQuery.m_sJoinIdx.IsEmpty() )
			continue;

		const auto & pServed = m_dAcquired.Get ( tQuery.m_sJoinIdx );
		if ( !pServed )
		{
			for ( auto & dFailureSet : dFailuresSet )
				dFailureSet.SubmitEx ( tQuery.m_sJoinIdx, nullptr, "%s", "table not found" );

			return false;
		}

		if ( !ServedDesc_t::IsSelectable ( pServed ) )
		{
			for ( auto & dFailureSet : dFailuresSet )
				dFailureSet.SubmitEx ( tQuery.m_sJoinIdx, nullptr, "%s", "table is not suitable for select" );

			return false;
		}

		dJoinedServed[i] = { pServed, -1 };
	}

	ARRAY_FOREACH ( i, dJoinedServed )
	{
		for ( int j = i+1; j < dJoinedServed.GetLength(); j++ )
			if ( dJoinedServed[j].m_pServed == dJoinedServed[i].m_pServed )
			{
				dJoinedServed[j].m_iDupeId = i;
				break;
			}
	}

	return true;
}


bool SearchHandler_c::CreateValidSorters ( VecTraits_T<ISphMatchSorter *> & dSrt, SphQueueRes_t * pQueueRes, VecTraits_T<SearchFailuresLog_c> & dFlr, StrVec_t * pExtra, const CSphIndex * pIndex, CSphVector<const CSphIndex*> & dJoinedIndexes, const CSphString & sLocal, const char * szParent, ISphExprHook * pHook )
{
	auto iQueries = dSrt.GetLength();
	#if PARANOID
	for ( const auto* pSorter : dSrt)
		assert ( !pSorter );
	#endif

	CSphFixedVector<CSphString> dErrors ( iQueries );
	int iValidSorters = CreateSorters ( pIndex, dJoinedIndexes, dSrt, dErrors, pExtra, *pQueueRes, pHook );
	if ( iValidSorters<dSrt.GetLength() )
	{
		ARRAY_FOREACH ( i, dErrors )
		{
			if ( !dErrors[i].IsEmpty () )
				dFlr[i].Submit ( sLocal, szParent, dErrors[i].cstr () );
		}
	}

	m_bMultiQueue = pQueueRes->m_bAlowMulti;
	return !!iValidSorters;
}


void SearchHandler_c::PopulateCountDistinct ( CSphVector<CSphVector<int64_t>> & dCountDistinct ) const
{
	dCountDistinct.Resize ( m_dLocal.GetLength() );

	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		const LocalIndex_t & tLocal = m_dLocal[iLocal];
		auto pIndex = CheckIndexSelectable ( tLocal.m_sName, tLocal.m_sParentIndex.cstr(), nullptr );
		if ( !pIndex )
			continue;

		auto & dIndexCountDistinct = dCountDistinct[iLocal];
		dIndexCountDistinct.Resize ( m_dNQueries.GetLength() );
		dIndexCountDistinct.Fill(-1);
		ARRAY_FOREACH ( i, dIndexCountDistinct )
		{
			auto & tQuery = m_dNQueries[i];
			int iGroupby = GetAliasedAttrIndex ( tQuery.m_sGroupBy, tQuery, RIdx_c(pIndex)->GetMatchSchema() );
			if ( iGroupby<0 )
				continue;

			auto & sAttr = RIdx_c(pIndex)->GetMatchSchema().GetAttr(iGroupby).m_sName;
			CSphString sModifiedAttr;
			dIndexCountDistinct[i] = RIdx_c(pIndex)->GetCountDistinct ( sAttr, sModifiedAttr );
		}
	}
}


int SearchHandler_c::CalcMaxThreadsPerIndex ( int iConcurrency ) const
{
	int iNumValid = 0;
	ARRAY_FOREACH ( i, m_dLocal )
	{
		auto pIndex = CheckIndexSelectable ( m_dLocal[i].m_sName, m_dLocal[i].m_sParentIndex.cstr(), nullptr );
		if ( !pIndex )
			continue;

		iNumValid++;
	}

	return ::CalcMaxThreadsPerIndex ( iConcurrency, iNumValid );
}


void SearchHandler_c::CalcThreadsPerIndex ( int iConcurrency )
{
	if ( !iConcurrency )
		iConcurrency =  MaxChildrenThreads();

	int iBusyWorkers = Max ( GlobalWorkPool()->CurTasks() - 1, 0 ); // ignore current task
	int iAvailableWorkers = Max ( Coro::CurrentScheduler()->WorkingThreads() - iBusyWorkers, 1 );
	iAvailableWorkers = Min ( iAvailableWorkers, iConcurrency );

	// this is need to obey ps dispatcher template, if it defines concurrency
	// that will help to perform reproducable queries, see test 261
	auto tDispatch = GetEffectivePseudoShardingDispatcherTemplate ();
	Dispatcher::Unify ( tDispatch, m_dNQueries.First ().m_tPseudoShardingDispatcher );
	if ( tDispatch.concurrency )
	{
//		sphWarning ( "correct iAvailableWorkers %d to defined %d", iAvailableWorkers, tDispatch.concurrency );
		iAvailableWorkers = tDispatch.concurrency;
	}

	CSphVector<CSphVector<int64_t>> dCountDistinct;
	PopulateCountDistinct ( dCountDistinct );

	int iMaxThreadsPerIndex = CalcMaxThreadsPerIndex ( iAvailableWorkers );

	CSphVector<SplitData_t> dSplitData ( m_dLocal.GetLength() );

	int iEnabledIndexes = 0;
	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		const LocalIndex_t & tLocal = m_dLocal[iLocal];
		auto pIndex = CheckIndexSelectable ( tLocal.m_sName, tLocal.m_sParentIndex.cstr(), nullptr );
		if ( !pIndex )
			continue;

		iEnabledIndexes++;
		auto & tPSInfo = m_dPSInfo[iLocal];
		auto & tSplitData = dSplitData[iLocal];
		RIdx_c pIdx { pIndex };
		if ( GetPseudoSharding () || pIdx->IsRT() )
		{
			// do metric calcs
			tPSInfo.m_iMaxThreads = iMaxThreadsPerIndex;
			auto tMetric = pIdx->GetPseudoShardingMetric ( m_dNQueries, dCountDistinct[iLocal], tPSInfo.m_iMaxThreads, tPSInfo.m_bForceSingleThread );
			assert ( tMetric.first>=0 );

			tSplitData.m_iMetric = tMetric.first;

			bool bExplicitConcurrency = m_dNQueries.any_of ( []( auto & tQuery ){ return tQuery.m_iConcurrency>0; } );
			tSplitData.m_iThreadCap = bExplicitConcurrency ? 0 : tMetric.second;	// ignore thread cap if concurrency is explicitly specified
		}
		else
		{
			// don't do metric calcs; we are guaranteed to have one thread
			// set the 'force single thread' flag to make sure max_matches won't be increased when it is not necessary
			tPSInfo = { 1, 1, true };
			tSplitData.m_iThreadCap = 1;
		}
	}

	if ( iAvailableWorkers>iEnabledIndexes )
	{
		IntVec_t dThreads;
		DistributeThreadsOverIndexes ( dThreads, dSplitData, iAvailableWorkers );
		ARRAY_FOREACH ( i, dThreads )
			m_dPSInfo[i].m_iThreads = dThreads[i];
	}
}


class AssignTag_c : public MatchProcessor_i
{
public:
	AssignTag_c ( int iTag )
		: m_iTag ( iTag )
	{}

	void Process ( CSphMatch * pMatch ) final			{ ProcessMatch(pMatch); }
	bool ProcessInRowIdOrder() const final				{ return false;	}
	void Process ( VecTraits_T<CSphMatch *> & dMatches ) final { dMatches.for_each ( [this]( CSphMatch * pMatch ){ ProcessMatch(pMatch); } ); }

private:
	int	m_iTag = 0;

	inline void ProcessMatch ( CSphMatch * pMatch )		{ pMatch->m_iTag = m_iTag; }
};


class GlobalSorters_c
{
public:
	GlobalSorters_c ( const VecTraits_T<CSphQuery> & dQueries, const CSphVector<cServedIndexRefPtr_c> & dIndexes )
		: m_dQueries ( dQueries )
		, m_dSorters { dQueries.GetLength() }
	{
		auto iValidIndexes = (int)dIndexes.count_of ( [&] ( const auto& pIndex ) { return pIndex; } );

		m_bNeedGlobalSorters = iValidIndexes>1 && !dQueries.First().m_sGroupDistinct.IsEmpty();
		if ( m_bNeedGlobalSorters )
		{
			// check if schemas are same
			const CSphSchema * pFirstSchema = nullptr;
			for ( auto i : dIndexes )
			{
				if ( !i )
					continue;

				if ( !pFirstSchema )
				{
					pFirstSchema = &RIdx_c ( i )->GetMatchSchema();
					continue;
				}

				CSphString sCmpError;
				if ( !pFirstSchema->CompareTo ( RIdx_c ( i )->GetMatchSchema(), sCmpError ) )
				{
					m_bNeedGlobalSorters = false;
					break;
				}
			}
		}

		for ( auto & i : m_dSorters )
			i.Resize ( dIndexes.GetLength() );
	}

	~GlobalSorters_c()
	{
		for ( auto & i : m_dSorters )
			for ( auto & j : i )
				SafeDelete ( j.m_pSorter );
	}

	bool StoreSorter ( int iQuery, int iIndex, ISphMatchSorter * & pSorter, const DocstoreReader_i * pDocstore, int iTag )
	{
		// FACET head is the plain query wo group sorter and can not move all result set into single sorter
		// could be replaced with !pSorter->IspSorter->IsGroupby()
		if ( !NeedGlobalSorters() || m_dQueries[iQuery].m_bFacetHead )
			return false;

		// take ownership of the sorter
		m_dSorters[iQuery][iIndex] = { pSorter, pDocstore, iTag };
		pSorter = nullptr;

		return true;
	}

	bool NeedGlobalSorters() const
	{
		return m_bNeedGlobalSorters;
	}

	void MergeResults ( VecTraits_T<AggrResult_t> & dResults )
	{
		if ( !NeedGlobalSorters() )
			return;

		ARRAY_FOREACH ( iQuery, m_dSorters )
		{
			CSphVector<ISphMatchSorter *> dValidSorters;
			for ( auto i : m_dSorters[iQuery] )
			{
				if ( !i.m_pSorter )
					continue;

				dValidSorters.Add ( i.m_pSorter );

				// assign order tag here so we can link to docstore later
				AssignTag_c tAssign ( i.m_iTag );
				i.m_pSorter->Finalize ( tAssign, false, false );
			}

			int iNumIndexes = dValidSorters.GetLength();
			if ( !iNumIndexes )
				continue;

			ISphMatchSorter * pLastSorter = dValidSorters[iNumIndexes-1];

			// merge all results to the last sorter. this is done to try to keep some compatibility with no-global-sorters code branch
			for ( int iIndex = iNumIndexes-2; iIndex>=0; iIndex-- )
				dValidSorters[iIndex]->MoveTo ( pLastSorter, true );

			dResults[iQuery].m_iTotalMatches = pLastSorter->GetTotalCount();
			dResults[iQuery].AddResultset ( pLastSorter, m_dSorters[iQuery][0].m_pDocstore, m_dSorters[iQuery][0].m_iTag, m_dQueries[iQuery].m_iCutoff );

			// we already assigned index/docstore tags to all matches; no need to do it again
			if ( dResults[iQuery].m_dResults.GetLength() )
				dResults[iQuery].m_dResults[0].m_bTagsAssigned = true;

			// add fake empty result sets (for tag->docstore lookup)
			for ( int i = 1; i < m_dSorters[iQuery].GetLength(); i++ )
				dResults[iQuery].AddEmptyResultset ( m_dSorters[iQuery][i].m_pDocstore, m_dSorters[iQuery][i].m_iTag );
		}
	}

private:
	struct SorterData_t
	{
		ISphMatchSorter *			m_pSorter = nullptr;
		const DocstoreReader_i *	m_pDocstore = nullptr;
		int							m_iTag = 0;
	};

	const VecTraits_T<CSphQuery> &			m_dQueries;
	CSphVector<CSphVector<SorterData_t>>	m_dSorters;
	bool									m_bNeedGlobalSorters = false;
};


CSphVector<const CSphIndex*> SearchHandler_c::GetRlockedJoinedIndexes ( const CSphVector<JoinedServedIndex_t> & dJoinedServed, std::vector<RIdx_c> & dRLockedJoined ) const
{
	CSphVector<const CSphIndex*> dJoinedIndexes;
	for ( auto & i : dJoinedServed )
	{
		if ( !i.m_pServed )
		{
			dJoinedIndexes.Add(nullptr);
			continue;
		}

		if ( i.m_iDupeId!=-1 )
			dJoinedIndexes.Add ( dJoinedIndexes[i.m_iDupeId] );
		else
		{
			dRLockedJoined.emplace_back ( i.m_pServed );
			dJoinedIndexes.Add ( dRLockedJoined.back() );
		}
	}

	return dJoinedIndexes;
}


bool SearchHandler_c::SubmitSuccess ( CSphVector<ISphMatchSorter *> & dSorters, GlobalSorters_c & tGlobalSorters, LocalSearchRef_t & tCtx, int64_t & iCpuTime, int iQuery, int iLocal, const CSphQueryResultMeta & tMqMeta, const CSphQueryResult & tMqRes )
{
	auto & dNFailuresSet = tCtx.m_dFailuresSet;
	auto & dNAggrResults = tCtx.m_dAggrResults;
	auto & dNResults = tCtx.m_dResults;
	int iNumQueries = m_dNQueries.GetLength();
	const LocalIndex_t & tLocal = m_dLocal[iLocal];
	const CSphString & sLocal = tLocal.m_sName;
	const char * szParent = tLocal.m_sParentIndex.cstr();
	int iOrderTag = tLocal.m_iOrderTag;
	ISphMatchSorter * pSorter = dSorters[iQuery];

	AggrResult_t & tNRes = dNAggrResults[iQuery];
	int iQTimeForStats = tNRes.m_iQueryTime;
	auto pDocstore = m_bMultiQueue ? tMqRes.m_pDocstore : dNResults[iQuery].m_pDocstore;

	// multi-queue only returned one result set meta, so we need to replicate it
	if ( m_bMultiQueue )
	{
		// these times will be overridden below, but let's be clean
		iQTimeForStats = tMqMeta.m_iQueryTime / iNumQueries;
		tNRes.m_iQueryTime += iQTimeForStats;
		tNRes.MergeWordStats ( tMqMeta );
		tNRes.m_iMultiplier = iNumQueries;
		tNRes.m_iCpuTime += tMqMeta.m_iCpuTime / iNumQueries;
		tNRes.m_bTotalMatchesApprox |= tMqMeta.m_bTotalMatchesApprox;

		iCpuTime /= iNumQueries;
	}
	else if ( tNRes.m_iMultiplier==-1 ) // multiplier -1 means 'error'
	{
		dNFailuresSet[iQuery].Submit ( sLocal, szParent, tNRes.m_sError.cstr() );
		return false;
	}

	++tNRes.m_iSuccesses;
	tNRes.m_iCpuTime = iCpuTime;
	tNRes.m_iTotalMatches += pSorter->GetTotalCount();
	tNRes.m_iPredictedTime = tNRes.m_bHasPrediction ? CalcPredictedTimeMsec ( tNRes ) : 0;

	m_dQueryIndexStats[iLocal].m_dStats[iQuery].m_iSuccesses = 1;
	m_dQueryIndexStats[iLocal].m_dStats[iQuery].m_uQueryTime = iQTimeForStats;
	m_dQueryIndexStats[iLocal].m_dStats[iQuery].m_uFoundRows = pSorter->GetTotalCount();

	// extract matches from sorter
	if ( !tGlobalSorters.StoreSorter ( iQuery, iLocal, dSorters[iQuery], pDocstore, iOrderTag ) )
		tNRes.AddResultset ( pSorter, pDocstore, iOrderTag, m_dNQueries[iQuery].m_iCutoff );

	if ( !tNRes.m_sWarning.IsEmpty() )
		dNFailuresSet[iQuery].Submit ( sLocal, szParent, tNRes.m_sWarning.cstr() );

	return true;
}


void SearchHandler_c::RunLocalSearches()
{
	int64_t tmLocal = sphMicroTimer ();

	// setup local searches
	const int iQueries = m_dNQueries.GetLength ();
	const int iNumLocals = m_dLocal.GetLength();

//	sphWarning ( "%s:%d", __FUNCTION__, __LINE__ );
//	sphWarning ("Locals: %d, queries %d", iNumLocals, iQueries );

	m_dQueryIndexStats.Resize ( iNumLocals );
	for ( auto & dQueryIndexStats : m_dQueryIndexStats )
		dQueryIndexStats.m_dStats.Resize ( iQueries );

	StrVec_t * pMainExtra = nullptr;
	if ( m_dNQueries.First ().m_bAgent )
	{
		m_dExtraSchema.Reset (); // cleanup from any possible previous usages
		pMainExtra = &m_dExtraSchema;
	}

	CSphVector<cServedIndexRefPtr_c> dLocalIndexes;
	for ( const auto& i : m_dLocal )
		dLocalIndexes.Add ( CheckIndexSelectable ( i.m_sName, nullptr ) );

	GlobalSorters_c tGlobalSorters ( m_dNQueries, dLocalIndexes );

	m_dPSInfo.Resize(iNumLocals);
	m_dPSInfo.Fill ( { 1, 1, false } );

	CSphFixedVector<int> dOrder { iNumLocals };
	for ( int i = 0; i<iNumLocals; ++i )
		dOrder[i] = i;

	auto tDispatch = GetEffectiveBaseDispatcherTemplate();
	Dispatcher::Unify ( tDispatch, m_dNQueries.First().m_tMainDispatcher );

	// the context
	ClonableCtx_T<LocalSearchRef_t, LocalSearchClone_t, Threads::ECONTEXT::UNORDERED> dCtx { m_tHook, pMainExtra, m_dNFailuresSet, m_dNAggrResults, m_dNResults };
	auto pDispatcher = Dispatcher::Make ( iNumLocals, m_dNQueries.First().m_iConcurrency, tDispatch, dCtx.IsSingle() );
	dCtx.LimitConcurrency ( pDispatcher->GetConcurrency() );

	bool bSingle = pDispatcher->GetConcurrency()==1;

//	sphWarning ( "iConcurrency: %d", iConcurrency );

	if ( !bSingle )
	{
//		sphWarning ( "Reordering..." );
		// if run parallel - start in mass order, if single - in natural order
		// set order by decreasing index mass (heaviest one comes first). That is why 'less' implemented by '>'
		dOrder.Sort ( Lesser ( [this] ( int a, int b ) {
			return m_dLocal[a].m_iMass>m_dLocal[b].m_iMass;
		} ) );

		CalcThreadsPerIndex ( pDispatcher->GetConcurrency() );
	}

//	for ( int iOrder : dOrder )
//		sphWarning ( "Sorted: %d, Order %d, mass %d", !!bSingle, iOrder, (int) m_dLocal[iOrder].m_iMass );

	std::atomic<int32_t> iTotalSuccesses { 0 };
	Coro::ExecuteN ( dCtx.Concurrency ( iNumLocals ), [&]
	{
		auto pSource = pDispatcher->MakeSource();
		int iJob = -1; // make it consumed

		if ( !pSource->FetchTask ( iJob ) )
		{
			LOCSEARCHINFO << "Early finish parallel RunLocalSearches because of empty queue";
			return; // already nothing to do, early finish.
		}

		// these two moved from inside the loop to avoid construction on every turn
		CSphVector<ISphMatchSorter *> dSorters ( iQueries );
		dSorters.ZeroVec ();

		auto tJobContext = dCtx.CloneNewContext();
		auto& tCtx = tJobContext.first;
		LOCSEARCHINFO << "RunLocalSearches cloned context " << tJobContext.second;
		Threads::Coro::SetThrottlingPeriodMS ( session::GetThrottlingPeriodMS() );
		while ( true )
		{
			if ( !pSource->FetchTask ( iJob ) )
				return; // all is done

			auto iLocal = dOrder[iJob];
			LOCSEARCHINFO << "RunLocalSearches " << tJobContext.second << ", iJob: " << iJob << ", iLocal: " << iLocal << ", mass " << ( (int)m_dLocal[iLocal].m_iMass );
			iJob = -1; // mark it consumed

			int64_t iCpuTime = -sphTaskCpuTimer ();

			// FIXME!!! handle different proto
			myinfo::SetTaskInfo( R"(api-search query="%s" comment="%s" index="%s")",
									 m_dNQueries.First().m_sQuery.scstr (),
									 m_dNQueries.First().m_sComment.scstr (),
									 m_dLocal[iLocal].m_sName.scstr ());

			const LocalIndex_t & dLocal = m_dLocal[iLocal];
			const CSphString& sLocal = dLocal.m_sName;
			const char * szParent = dLocal.m_sParentIndex.cstr ();
			int iIndexWeight = dLocal.m_iWeight;
			auto& dNFailuresSet = tCtx.m_dFailuresSet;
			auto& dNAggrResults = tCtx.m_dAggrResults;
			auto& dNResults = tCtx.m_dResults;
			auto* pExtra = tCtx.m_pExtra;

			// publish crash query index
			GlobalCrashQueryGetRef().m_dIndex = FromStr ( sLocal );

			// prepare and check the index
			cServedIndexRefPtr_c pServed = CheckIndexSelectable ( sLocal, szParent, &dNFailuresSet );
			if ( !pServed )
				continue;

			CSphVector<JoinedServedIndex_t> dJoinedServed;
			if ( !PopulateJoinedIndexes ( dJoinedServed, dNFailuresSet ) )
				continue;

			bool bResult = false;
			CSphQueryResultMeta tMqMeta;
			CSphQueryResult tMqRes;
			tMqRes.m_pMeta = &tMqMeta;

			{	// scope for r-locking the index
				RIdx_c pIndex { pServed };

				tCtx.m_tHook.SetIndex ( pIndex );
				tCtx.m_tHook.SetQueryType ( m_eQueryType );

				std::vector<RIdx_c> dRLockedJoined;
				CSphVector<const CSphIndex*> dJoinedIndexes = GetRlockedJoinedIndexes ( dJoinedServed, dRLockedJoined );

				// create sorters
				SphQueueRes_t tQueueRes;
				if ( !CreateValidSorters ( dSorters, &tQueueRes, dNFailuresSet, pExtra, pIndex, dJoinedIndexes, sLocal, szParent, &tCtx.m_tHook ) )
					continue;

				// do the query
				CSphMultiQueryArgs tMultiArgs ( iIndexWeight );
				tMultiArgs.m_uPackedFactorFlags = tQueueRes.m_uPackedFactorFlags;
				if ( m_bGotLocalDF )
				{
					tMultiArgs.m_bLocalDF = true;
					tMultiArgs.m_pLocalDocs = &m_hLocalDocs;
					tMultiArgs.m_iTotalDocs = m_iTotalDocs;
				}

				bool bCanBeCloned = dSorters.all_of ( []( auto * pSorter ){ return pSorter ? pSorter->CanBeCloned() : true; } );

				// fixme: previous calculations are wrong; we are not splitting the query if we are using non-clonable sorters
				tMultiArgs.m_iThreads = bCanBeCloned ? m_dPSInfo[iLocal].m_iThreads : 1;
				tMultiArgs.m_iTotalThreads = m_dPSInfo[iLocal].m_iMaxThreads;
				tMultiArgs.m_bFinalizeSorters = !tGlobalSorters.NeedGlobalSorters();

				LOCSEARCHINFO << "RunLocalSearches index:" << pIndex->GetName();

				dNAggrResults.First().m_tIOStats.Start ();
				if ( m_bMultiQueue )
					bResult = pIndex->MultiQuery ( tMqRes, m_dNQueries.First(), dSorters, tMultiArgs );
				else
					bResult = pIndex->MultiQueryEx ( iQueries, &m_dNQueries[0], &dNResults[0], &dSorters[0], tMultiArgs );
				dNAggrResults.First ().m_tIOStats.Stop ();
			}

			iCpuTime += sphTaskCpuTimer ();

			// handle results
			if ( bResult )
			{
				// multi-query succeeded
				for ( int i=0; i<iQueries; ++i )
				{
					// in mt here kind of tricky index calculation, up to the next lines with sorter
					// but some sorters could have failed at "create sorter" stage
					ISphMatchSorter * pSorter = dSorters[i];
					if ( !pSorter )
						continue;

					if ( SubmitSuccess ( dSorters, tGlobalSorters, tCtx, iCpuTime, i, iLocal, tMqMeta, tMqRes ) )
						iTotalSuccesses.fetch_add ( 1, std::memory_order_relaxed );
				}
			} else
				// failed, submit local (if not empty) or global error string
				for ( int i = 0; i<iQueries; ++i )
					dNFailuresSet[i].Submit ( sLocal, szParent, tMqMeta.m_sError.IsEmpty ()
							? dNAggrResults[m_bMultiQueue ? 0 : i].m_sError.cstr ()
							: tMqMeta.m_sError.cstr () );

			// cleanup sorters
			for ( auto &pSorter : dSorters )
				SafeDelete ( pSorter );

			LOCSEARCHINFO << "RunLocalSearches result " << bResult << " index " << sLocal;

			if ( !pSource->FetchTask ( iJob ) )
				return; // all is done

			Threads::Coro::ThrottleAndKeepCrashQuery (); // we set CrashQuery anyway at the start of the loop
		}
	});
	LOCSEARCHINFO << "RunLocalSearches processed in " << dCtx.NumWorked() << " thread(s)";
	dCtx.Finalize (); // merge mt results (if any)

	tGlobalSorters.MergeResults(m_dNAggrResults);

	// update our wall time for every result set
	tmLocal = sphMicroTimer ()-tmLocal;
	for ( int iQuery = 0; iQuery<iQueries; ++iQuery )
		m_dNAggrResults[iQuery].m_iQueryTime += (int) ( tmLocal / 1000 );

	auto iTotalSuccessesInt = iTotalSuccesses.load ( std::memory_order_relaxed );

	for ( auto iLocal = 0; iLocal<iNumLocals; ++iLocal )
		for ( int iQuery = 0; iQuery<iQueries; ++iQuery )
		{
			QueryStat_t & tStat = m_dQueryIndexStats[iLocal].m_dStats[iQuery];
			if ( tStat.m_iSuccesses )
				tStat.m_uQueryTime = (int) ( tmLocal / 1000 / iTotalSuccessesInt );
		}
}

// check expressions into a query to make sure that it's ready for multi query optimization
bool SearchHandler_c::AllowsMulti() const
{
	if ( m_bFacetQueue )
		return true;

	// in some cases the same select list allows queries to be multi query optimized
	// but we need to check dynamic parts size equality and we do it later in RunLocalSearches()
	const CSphVector<CSphQueryItem> & tFirstQueryItems = m_dNQueries.First().m_dItems;
	bool bItemsSameLen = true;
	for ( int i=1; i<m_dNQueries.GetLength() && bItemsSameLen; ++i )
		bItemsSameLen = ( tFirstQueryItems.GetLength()==m_dNQueries[i].m_dItems.GetLength() );
	if ( bItemsSameLen )
	{
		bool bSameItems = true;
		ARRAY_FOREACH_COND ( i, tFirstQueryItems, bSameItems )
		{
			const CSphQueryItem & tItem1 = tFirstQueryItems[i];
			for ( int j=1; j<m_dNQueries.GetLength () && bSameItems; ++j )
			{
				const CSphQueryItem & tItem2 = m_dNQueries[j].m_dItems[i];
				bSameItems = tItem1.m_sExpr==tItem2.m_sExpr && tItem1.m_eAggrFunc==tItem2.m_eAggrFunc;
			}
		}

		if ( bSameItems )
			return true;
	}

	// if select lists do not contain any expressions we can optimize queries too
	for ( const auto & dLocal : m_dLocal )
	{
		RIdx_c pServedIndex ( m_dAcquired.Get ( dLocal.m_sName ) );
		// FIXME!!! compare expressions as m_pExpr->GetHash
		const CSphSchema & tSchema = pServedIndex->GetMatchSchema();
		if ( m_dNQueries.any_of ( [&tSchema] ( const CSphQuery & tQ ) { return sphHasExpressions ( tQ, tSchema ); } ) )
			return false;
	}
	return true;
}


struct IndexSettings_t
{
	uint64_t	m_uHash;
	int			m_iLocal;
};

void SearchHandler_c::SetupLocalDF ()
{
	if ( m_dLocal.GetLength()<2 )
		return;

	SwitchProfile ( m_pProfile, SPH_QSTATE_LOCAL_DF );

	bool bGlobalIDF = true;
	ARRAY_FOREACH_COND ( i, m_dLocal, bGlobalIDF )
	{
		auto pDesc = GetServed( m_dLocal[i].m_sName );
		bGlobalIDF = ( pDesc && !pDesc->m_sGlobalIDFPath.IsEmpty () );
	}
	// bail out on all indexes with global idf set
	if ( bGlobalIDF )
		return;

	bool bOnlyNoneRanker = true;
	bool bOnlyFullScan = true;
	bool bHasLocalDF = false;
	for ( const CSphQuery & tQuery : m_dNQueries )
	{
		bOnlyFullScan &= tQuery.m_sQuery.IsEmpty();

		bHasLocalDF |= tQuery.m_bLocalDF.value_or ( false );
		if ( !tQuery.m_sQuery.IsEmpty() && tQuery.m_bLocalDF.value_or ( false ) )
			bOnlyNoneRanker &= ( tQuery.m_eRanker==SPH_RANK_NONE );
	}
	// bail out queries: full-scan, ranker=none, local_idf=0
	if ( bOnlyFullScan || bOnlyNoneRanker || !bHasLocalDF )
		return;

	CSphVector<char> dQuery ( 512 );
	dQuery.Resize ( 0 );
	for ( const CSphQuery & tQuery : m_dNQueries )
	{
		if ( tQuery.m_sQuery.IsEmpty() || !tQuery.m_bLocalDF.value_or ( false ) || tQuery.m_eRanker==SPH_RANK_NONE )
			continue;

		int iLen = tQuery.m_sQuery.Length();
		auto * pDst = dQuery.AddN ( iLen + 1 );
		memcpy ( pDst, tQuery.m_sQuery.cstr(), iLen );
		dQuery.Last() = ' '; // queries delimiter
	}
	// bail out on empty queries
	if ( !dQuery.GetLength() )
		return;

	dQuery.Add ( '\0' );

	// order indexes by settings
	CSphVector<IndexSettings_t> dLocal ( m_dLocal.GetLength() );
	dLocal.Resize ( 0 );
	ARRAY_FOREACH ( i, m_dLocal )
	{
		dLocal.Add();
		dLocal.Last().m_iLocal = i;
		// TODO: cache settingsFNV on index load
		// FIXME!!! no need to count dictionary hash
		RIdx_c pIndex ( m_dAcquired.Get ( m_dLocal[i].m_sName ) );
		dLocal.Last().m_uHash = pIndex->GetTokenizer()->GetSettingsFNV() ^ pIndex->GetDictionary()->GetSettingsFNV();
	}
	dLocal.Sort ( bind ( &IndexSettings_t::m_uHash ) );

	// gather per-term docs count
	CSphVector < CSphKeywordInfo > dKeywords;
	ARRAY_FOREACH ( i, dLocal )
	{
		int iLocalIndex = dLocal[i].m_iLocal;
		RIdx_c pIndex ( m_dAcquired.Get ( m_dLocal[iLocalIndex].m_sName ) );
		m_iTotalDocs += pIndex->GetStats().m_iTotalDocuments;

		if ( i && dLocal[i].m_uHash==dLocal[i-1].m_uHash )
		{
			dKeywords.Apply ( [] ( CSphKeywordInfo & tKw ) { tKw.m_iDocs = 0; } );

			// no need to tokenize query just fill docs count
			pIndex->FillKeywords ( dKeywords );
		} else
		{
			GetKeywordsSettings_t tSettings;
			tSettings.m_bStats = true;
			dKeywords.Resize ( 0 );
			pIndex->GetKeywords ( dKeywords, dQuery.Begin(), tSettings, NULL );

			// FIXME!!! move duplicate removal to GetKeywords to do less QWord setup and dict searching
			// custom uniq - got rid of word duplicates
			dKeywords.Sort ( bind ( &CSphKeywordInfo::m_sNormalized ) );
			if ( dKeywords.GetLength()>1 )
			{
				int iSrc = 1, iDst = 1;
				while ( iSrc<dKeywords.GetLength() )
				{
					if ( dKeywords[iDst-1].m_sNormalized==dKeywords[iSrc].m_sNormalized )
						iSrc++;
					else
					{
						Swap ( dKeywords[iDst], dKeywords[iSrc] );
						iDst++;
						iSrc++;
					}
				}
				dKeywords.Resize ( iDst );
			}
		}

		for ( auto& tKw: dKeywords )
		{
			int64_t * pDocs = m_hLocalDocs ( tKw.m_sNormalized );
			if ( pDocs )
				*pDocs += tKw.m_iDocs;
			else
				m_hLocalDocs.Add ( tKw.m_iDocs, tKw.m_sNormalized );
		}
	}

	m_bGotLocalDF = true;
}


static int GetIndexWeight ( const CSphString& sName, const CSphVector<CSphNamedInt> & dIndexWeights, int iDefaultWeight )
{
	for ( auto& dWeight : dIndexWeights )
		if ( dWeight.first==sName )
			return dWeight.second;

	// distributed index adds {'*', weight} to all agents in case it got custom weight
	if ( dIndexWeights.GetLength() && dIndexWeights.Last().first=="*" )
		return dIndexWeights[0].second;

	return iDefaultWeight;
}

uint64_t CalculateMass ( const CSphIndexStatus & dStats )
{
	auto iOvermapped = dStats.m_iMapped-dStats.m_iMappedResident;
	if ( iOvermapped<0 ) // it could be negative since resident is rounded up to page edge
		iOvermapped = 0;
	return 1000000 * dStats.m_iNumChunks
			+ 10 * iOvermapped
			+ dStats.m_iRamUse;
}

static uint64_t GetIndexMass ( const CSphString & sName )
{
	return ServedIndex_c::GetIndexMass ( GetServed ( sName ) );
}


// process system.table
inline static void FixupSystemTableW ( StrVec_t & dNames, CSphQuery & tQuery ) noexcept
{
	if ( dNames.GetLength ()==1 && dNames.First ().EqN ( "system" ) && !tQuery.m_dStringSubkeys.IsEmpty () )
	{
		dNames[0] = SphSprintf ( "system%s", tQuery.m_dStringSubkeys[0].cstr () );
		tQuery.m_dStringSubkeys.Remove(0);
	}
}

// declared to be used in ParseSysVar
void HandleMysqlShowThreads ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleMysqlShowTables ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleShowSessions ( RowBuffer_i& tOut, const SqlStmt_t* pStmt );
void HandleMysqlDescribe ( RowBuffer_i & tOut, SqlStmt_t * pStmt );
void HandleSelectIndexStatus ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );
void HandleSelectFiles ( RowBuffer_i & tOut, const SqlStmt_t * pStmt );

bool SearchHandler_c::ParseSysVar ()
{
	const auto& sVar = m_dLocal.First().m_sName;
	const auto & dSubkeys = m_dNQueries.First ().m_dStringSubkeys;

	if ( sVar=="@@system" )
	{
		if ( !dSubkeys.IsEmpty () )
		{
			bool bSchema = ( dSubkeys.Last ()==".@table" );
			bool bValid = true;
			TableFeeder_fn fnFeed;
			if ( dSubkeys[0]==".threads" ) // select .. from @@system.threads
			{
				if ( m_pStmt->m_sThreadFormat.IsEmpty() ) // override format to show all columns by default
					m_pStmt->m_sThreadFormat="all";

				fnFeed = [this] ( RowBuffer_i * pBuf ) { HandleMysqlShowThreads ( *pBuf, m_pStmt ); };
			}
			else if ( dSubkeys[0]==".tables" ) // select .. from @@system.tables
			{
				fnFeed = [this] ( RowBuffer_i * pBuf ) { HandleMysqlShowTables ( *pBuf, m_pStmt ); };
			}
			else if ( dSubkeys[0]==".tasks" ) // select .. from @@system.tasks
			{
				fnFeed = [] ( RowBuffer_i * pBuf ) { HandleTasks ( *pBuf ); };
			}
			else if ( dSubkeys[0]==".sched" ) // select .. from @@system.sched
			{
				fnFeed = [] ( RowBuffer_i * pBuf ) { HandleSched ( *pBuf ); };
			} else if ( dSubkeys[0] == ".sessions" ) // select .. from @@system.sched
			{
				fnFeed = [this] ( RowBuffer_i* pBuf ) { HandleShowSessions ( *pBuf, m_pStmt ); };
			}
			else
				bValid = false;

			if ( bValid )
			{
				cServedIndexRefPtr_c pIndex;
				if ( bSchema )
				{
					m_dLocal.First ().m_sName.SetSprintf( "@@system.%s.@table", dSubkeys[0].cstr() );
					pIndex = MakeDynamicIndexSchema ( std::move ( fnFeed ) );

				} else {
					m_dLocal.First ().m_sName.SetSprintf ( "@@system.%s", dSubkeys[0].cstr () );
					pIndex = MakeDynamicIndex ( std::move ( fnFeed ) );
				}
				m_dAcquired.AddIndex ( m_dLocal.First ().m_sName, std::move (pIndex) );
				return true;
			}
		}
	}

	m_sError << "no such variable " << sVar;
	dSubkeys.for_each ( [this] ( const auto& s ) { m_sError << s; } );
	return false;
}

bool SearchHandler_c::ParseIdxSubkeys ()
{
	const auto & sVar = m_dLocal.First ().m_sName;
	const auto & dSubkeys = m_dNQueries.First ().m_dStringSubkeys;

	assert ( !dSubkeys.IsEmpty () );

	bool bSchema = ( dSubkeys.GetLength()>1 && dSubkeys.Last ()==".@table" );
	TableFeeder_fn fnFeed;
	if ( dSubkeys[0]==".@table" ) // select .. idx.table
		fnFeed = [this] ( RowBuffer_i * pBuf ) { HandleMysqlDescribe ( *pBuf, m_pStmt ); };
	else if ( dSubkeys[0]==".@status" ) // select .. idx.status
		fnFeed = [this] ( RowBuffer_i * pBuf ) { HandleSelectIndexStatus ( *pBuf, m_pStmt ); };
	else if ( dSubkeys[0]==".@files" ) // select .. from idx.files
		fnFeed = [this] ( RowBuffer_i * pBuf ) { HandleSelectFiles ( *pBuf, m_pStmt ); };
	else
	{
		m_sError << "No such table " << sVar;
		dSubkeys.for_each ([this] (const auto& s) { m_sError << s;});
		return false;
	}

	cServedIndexRefPtr_c pIndex;
	if ( bSchema )
	{
		m_dLocal.First ().m_sName.SetSprintf ( "%s%s.@table", sVar.cstr (), dSubkeys[0].cstr () );
		pIndex = MakeDynamicIndexSchema ( std::move ( fnFeed ) );
	} else
	{
		m_dLocal.First ().m_sName.SetSprintf ( "%s%s", sVar.cstr (), dSubkeys[0].cstr () );
		pIndex = MakeDynamicIndex ( std::move ( fnFeed ) );
	}

	m_dAcquired.AddIndex ( m_dLocal.First().m_sName, std::move ( pIndex ) );
	return true;
}

////////////////////////////////////////////////////////////////
// check for single-query, multi-queue optimization possibility
////////////////////////////////////////////////////////////////
bool SearchHandler_c::CheckMultiQuery() const
{
	const int iQueries = m_dNQueries.GetLength();
	if ( iQueries<=1 )
		return false;

	const CSphQuery & qFirst = m_dNQueries.First();
	auto dQueries = m_dNQueries.Slice ( 1 );

	// queries over special indexes as status/meta are not capable for multiquery
	if ( !qFirst.m_dStringSubkeys.IsEmpty() )
		return false;

	for ( const CSphQuery & qCheck : dQueries )
	{
		// these parameters must be the same
		if (
			( qCheck.m_sRawQuery!=qFirst.m_sRawQuery ) || // query string
				( qCheck.m_dWeights.GetLength ()!=qFirst.m_dWeights.GetLength () ) || // weights count
				( qCheck.m_dWeights.GetLength () && memcmp ( qCheck.m_dWeights.Begin (), qFirst.m_dWeights.Begin (),
					sizeof ( qCheck.m_dWeights[0] ) * qCheck.m_dWeights.GetLength () ) ) || // weights
				( qCheck.m_eMode!=qFirst.m_eMode ) || // search mode
				( qCheck.m_eRanker!=qFirst.m_eRanker ) || // ranking mode
				( qCheck.m_dFilters.GetLength ()!=qFirst.m_dFilters.GetLength () ) || // attr filters count
				( qCheck.m_dFilterTree.GetLength ()!=qFirst.m_dFilterTree.GetLength () ) ||
				( qCheck.m_iCutoff!=qFirst.m_iCutoff ) || // cutoff
				( qCheck.m_eSort==SPH_SORT_EXPR && qFirst.m_eSort==SPH_SORT_EXPR && qCheck.m_sSortBy!=qFirst.m_sSortBy )
				|| // sort expressions
					( qCheck.m_bGeoAnchor!=qFirst.m_bGeoAnchor ) || // geodist expression
				( qCheck.m_bGeoAnchor && qFirst.m_bGeoAnchor
					&& ( qCheck.m_fGeoLatitude!=qFirst.m_fGeoLatitude
						|| qCheck.m_fGeoLongitude!=qFirst.m_fGeoLongitude ) ) ) // some geodist cases

			return false;

		// filters must be the same too
		assert ( qCheck.m_dFilters.GetLength ()==qFirst.m_dFilters.GetLength () );
		assert ( qCheck.m_dFilterTree.GetLength ()==qFirst.m_dFilterTree.GetLength () );
		ARRAY_FOREACH ( i, qCheck.m_dFilters )
		{
			if ( qCheck.m_dFilters[i]!=qFirst.m_dFilters[i] )
				return false;
		}
		ARRAY_FOREACH ( i, qCheck.m_dFilterTree )
		{
			if ( qCheck.m_dFilterTree[i]!=qFirst.m_dFilterTree[i] )
				return false;
		}
	}
	return true;
}

// lock local indexes invoked in query
// Fails if an index is absent and this is not allowed
bool SearchHandler_c::AcquireInvokedIndexes()
{
	// add indexes required by JOIN
	// but don't try to acquire local indexes if query is issued only for remote distributed
	if ( m_dLocal.GetLength() )
	{
		StringBuilder_c sFailed (", ");
		for ( const auto & tQuery : m_dNQueries )
			if ( !tQuery.m_sJoinIdx.IsEmpty() && !m_dAcquired.AddUniqIndex ( tQuery.m_sJoinIdx ) )
				sFailed << tQuery.m_sJoinIdx;

		if ( !sFailed.IsEmpty() )
		{
			m_sError << "unknown local table(s) '" << sFailed << "' in search request";
			return false;
		}
	}

	// if unexistent allowed, short flow
	if ( m_dNQueries.First().m_bIgnoreNonexistentIndexes )
	{
		ARRAY_FOREACH ( i, m_dLocal )
			if ( !m_dAcquired.AddUniqIndex ( m_dLocal[i].m_sName ) )
				m_dLocal.Remove ( i-- );
		return true;
	}

	// _build the list of non-existent
	StringBuilder_c sFailed (", ");
	for ( const auto & dLocal : m_dLocal )
		if ( !m_dAcquired.AddUniqIndex ( dLocal.m_sName ) )
			sFailed << dLocal.m_sName;

	// no absent indexes, viola!
	if ( sFailed.IsEmpty ())
		return true;

	// report failed
	m_sError << "unknown local table(s) '" << sFailed << "' in search request";
	return false;
}

// uniq dLocals and copy into m_dLocal only uniq part.
void SearchHandler_c::UniqLocals ( VecTraits_T<LocalIndex_t> & dLocals )
{
	int iLen = dLocals.GetLength ();
	if ( !iLen )
		return;

	CSphVector<int> dOrder;
	dOrder.Resize ( dLocals.GetLength() );
	dOrder.FillSeq();

	dOrder.Sort ( Lesser ( [&dLocals] ( int a, int b )
	{
		return ( dLocals[a].m_sName<dLocals[b].m_sName )
			|| ( dLocals[a].m_sName==dLocals[b].m_sName && dLocals[a].m_iOrderTag>dLocals[b].m_iOrderTag );
	}));

	int iSrc = 1, iDst = 1;
	while ( iSrc<iLen )
	{
		if ( dLocals[dOrder[iDst-1]].m_sName==dLocals[dOrder[iSrc]].m_sName )
			++iSrc;
		else
			dOrder[iDst++] = dOrder[iSrc++];
	}

	dOrder.Resize ( iDst );
	m_dLocal.Resize ( iDst );
	ARRAY_FOREACH ( i, dOrder )
		m_dLocal[i] = std::move ( dLocals[dOrder[i]] );
}


void SearchHandler_c::CalcTimeStats ( int64_t tmCpu, int64_t tmSubset, const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent )
{
	// in multi-queue case (1 actual call per N queries), just divide overall query time evenly
	// otherwise (N calls per N queries), divide common query time overheads evenly
	const int iQueries = m_dNQueries.GetLength();
	if ( m_bMultiQueue )
	{
		for ( auto & dResult : m_dNAggrResults )
		{
			dResult.m_iQueryTime = (int)( tmSubset/1000/iQueries );
			dResult.m_iRealQueryTime = (int)( tmSubset/1000/iQueries );
			dResult.m_iCpuTime = tmCpu/iQueries;
		}
		return;
	}

	int64_t tmAccountedWall = 0;
	int64_t tmAccountedCpu = 0;
	for ( const auto & dResult : m_dNAggrResults )
	{
		tmAccountedWall += dResult.m_iQueryTime*1000;
		assert ( ( dResult.m_iCpuTime==0 && dResult.m_iAgentCpuTime==0 ) ||	// all work was done in this thread
			( dResult.m_iCpuTime>0 && dResult.m_iAgentCpuTime==0 ) ||		// children threads work
			( dResult.m_iAgentCpuTime>0 && dResult.m_iCpuTime==0 ) );		// agents work
		tmAccountedCpu += dResult.m_iCpuTime;
		tmAccountedCpu += dResult.m_iAgentCpuTime;
	}

	// whether we had work done in children threads (dist_threads>1) or in agents
	bool bExternalWork = tmAccountedCpu!=0;
	int64_t tmDeltaWall = ( tmSubset - tmAccountedWall ) / iQueries;

	for ( auto & dResult : m_dNAggrResults )
	{
		dResult.m_iQueryTime += (int)(tmDeltaWall/1000);
		dResult.m_iRealQueryTime = (int)( tmSubset/1000/iQueries );
		dResult.m_iCpuTime = tmCpu/iQueries;
		if ( bExternalWork )
			dResult.m_iCpuTime += tmAccountedCpu;
	}

	// don't forget to add this to stats
	if ( bExternalWork )
		tmCpu += tmAccountedCpu;

	// correct per-index stats from agents
	int iTotalSuccesses = 0;
	for ( const auto & dResult : m_dNAggrResults )
		iTotalSuccesses += dResult.m_iSuccesses;

	if ( !iTotalSuccesses )
		return;

	int64_t tmDelta = tmSubset - tmAccountedWall;

	auto nValidDistrIndexes = dDistrServedByAgent.count_of ( [] ( auto& t ) { return t.m_dStats.any_of ( [] ( auto& i ) { return i.m_iSuccesses; } ); } );
	int64_t nDistrDivider = iTotalSuccesses * nValidDistrIndexes * 1000;
	if ( nDistrDivider )
		for ( auto &tDistrStat : dDistrServedByAgent )
			for ( QueryStat_t& tStat : tDistrStat.m_dStats )
			{
				auto tmDeltaWallAgent = tmDelta * tStat.m_iSuccesses / nDistrDivider;
				tStat.m_uQueryTime += (int)tmDeltaWallAgent;
			}

	auto nValidLocalIndexes = m_dQueryIndexStats.count_of ( [] ( auto& t ) { return t.m_dStats.any_of ( [] ( auto& i ) { return i.m_iSuccesses; } ); } );
	int64_t nLocalDivider = iTotalSuccesses * nValidLocalIndexes * 1000;
	if ( nLocalDivider )
		for ( auto &dQueryIndexStat : m_dQueryIndexStats )
			for ( QueryStat_t& tStat : dQueryIndexStat.m_dStats )
			{
				int64_t tmDeltaWallLocal = tmDelta * tStat.m_iSuccesses / nLocalDivider;
				tStat.m_uQueryTime += (int)tmDeltaWallLocal;
			}
}


void SearchHandler_c::CalcPerIndexStats ( const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent ) const
{
	const int iQueries = m_dNQueries.GetLength();
	// calculate per-index stats
	ARRAY_FOREACH ( iLocal, m_dLocal )
	{
		const auto& pServed = m_dAcquired.Get ( m_dLocal[iLocal].m_sName );
		for ( int iQuery=0; iQuery<iQueries; ++iQuery )
		{
			QueryStat_t & tStat = m_dQueryIndexStats[iLocal].m_dStats[iQuery];
			if ( !tStat.m_iSuccesses )
				continue;

			pServed->m_pStats->AddQueryStat ( tStat.m_uFoundRows, tStat.m_uQueryTime );
			for ( auto &tDistr : dDistrServedByAgent )
			{
				if ( tDistr.m_dLocalNames.Contains ( m_dLocal[iLocal].m_sName ) )
				{
					tDistr.m_dStats[iQuery].m_uQueryTime += tStat.m_uQueryTime;
					tDistr.m_dStats[iQuery].m_uFoundRows += tStat.m_uFoundRows;
					++tDistr.m_dStats[iQuery].m_iSuccesses;
				}
			}
		}
	}

	for ( auto &tDistr : dDistrServedByAgent )
	{
		auto pServedDistIndex = GetDistr ( tDistr.m_sIndex );
		if ( pServedDistIndex )
			for ( int iQuery=0; iQuery<iQueries; ++iQuery )
			{
				auto & tStat = tDistr.m_dStats[iQuery];
				if ( !tStat.m_iSuccesses )
					continue;

				pServedDistIndex->m_tStats.AddQueryStat ( tStat.m_uFoundRows, tStat.m_uQueryTime );
			}
	}
}


void SearchHandler_c::CalcGlobalStats ( int64_t tmCpu, int64_t tmSubset, int64_t tmLocal, const CSphIOStats & tIO, const VecRefPtrsAgentConn_t & dRemotes ) const
{
	auto & g_tStats = gStats ();
	g_tStats.m_iQueries.fetch_add ( m_dNQueries.GetLength (), std::memory_order_relaxed );
	g_tStats.m_iQueryTime.fetch_add ( tmSubset, std::memory_order_relaxed );
	g_tStats.m_iQueryCpuTime.fetch_add ( tmCpu, std::memory_order_relaxed );
	if ( dRemotes.GetLength() )
	{
		int64_t tmWait = 0;
		for ( const AgentConn_t * pAgent : dRemotes )
			tmWait += pAgent->m_iWaited;

		// do *not* count queries to dist indexes w/o actual remote agents
		g_tStats.m_iDistQueries.fetch_add ( 1, std::memory_order_relaxed );
		g_tStats.m_iDistWallTime.fetch_add ( tmSubset, std::memory_order_relaxed );
		g_tStats.m_iDistLocalTime.fetch_add ( tmLocal, std::memory_order_relaxed );
		g_tStats.m_iDistWaitTime.fetch_add ( tmWait, std::memory_order_relaxed );
	}
	g_tStats.m_iDiskReads.fetch_add ( tIO.m_iReadOps, std::memory_order_relaxed );
	g_tStats.m_iDiskReadTime.fetch_add ( tIO.m_iReadTime, std::memory_order_relaxed );
	g_tStats.m_iDiskReadBytes.fetch_add ( tIO.m_iReadBytes, std::memory_order_relaxed );
}

static CSphVector<LocalIndex_t> CollectAllLocalIndexes ( const CSphVector<CSphNamedInt> & dIndexWeights )
{
	CSphVector<LocalIndex_t> dIndexes;
	int iOrderTag = 0;
	// search through all local indexes
	ServedSnap_t hLocal = g_pLocalIndexes->GetHash();
	for ( auto& tIt : *hLocal )
	{
		if ( !tIt.second ) // fixme! should never be...
			continue;
		auto & dLocal = dIndexes.Add ();
		dLocal.m_sName = tIt.first;
		dLocal.m_iOrderTag = iOrderTag++;
		dLocal.m_iWeight = GetIndexWeight ( tIt.first, dIndexWeights, 1 );
		dLocal.m_iMass = ServedIndex_c::GetIndexMass ( tIt.second );
	}
	return dIndexes;
}

// returns true = real indexes, false = sysvar (i.e. only one 'index' named from @@)
bool SearchHandler_c::BuildIndexList ( int & iDivideLimits, VecRefPtrsAgentConn_t & dRemotes, CSphVector<DistrServedByAgent_t> & dDistrServedByAgent )
{
	CSphQuery & tQuery = m_dNQueries.First ();

	if ( tQuery.m_sIndexes=="*" )
	{
		// they're all local, build the list
		m_dLocal = CollectAllLocalIndexes ( tQuery.m_dIndexWeights );
		return true;
	}

	m_dLocal.Reset ();
	int iOrderTag = 0;
	bool bSysVar = tQuery.m_sIndexes.Begins ( "@@" );

	// search through specified local indexes
	StrVec_t dIdxNames;
	if ( bSysVar )
		dIdxNames.Add ( tQuery.m_sIndexes );
	else
	{
		ParseIndexList ( tQuery.m_sIndexes, dIdxNames );
		FixupSystemTableW ( dIdxNames, tQuery );
	}

	const int iQueries = m_dNQueries.GetLength ();
	CSphVector<LocalIndex_t> dLocals;

	int iDistCount = 0;
	bool bDivideRemote = false;
	bool bHasLocalsAgents = false;

	for ( const auto& sIndex : dIdxNames )
	{
		auto pDist = GetDistr ( sIndex );
		if ( !pDist )
		{
			auto &dLocal = dLocals.Add ();
			dLocal.m_sName = sIndex;
			dLocal.m_iOrderTag = iOrderTag++;
			dLocal.m_iWeight = GetIndexWeight ( sIndex, tQuery.m_dIndexWeights, 1 );
			dLocal.m_iMass = GetIndexMass ( sIndex );
		} else
		{
			++iDistCount;
			int iWeight = GetIndexWeight ( sIndex, tQuery.m_dIndexWeights, -1 );
			auto & tDistrStat = dDistrServedByAgent.Add();
			tDistrStat.m_sIndex = sIndex;
			tDistrStat.m_dStats.Resize ( iQueries );
			tDistrStat.m_dStats.ZeroVec();

			for ( const auto& pAgent : pDist->m_dAgents )
			{
				tDistrStat.m_dAgentIds.Add ( dRemotes.GetLength() );
				auto * pConn = new AgentConn_t;
				pConn->SetMultiAgent ( pAgent );
				pConn->m_iStoreTag = iOrderTag++;
				pConn->m_iWeight = iWeight;
				pConn->m_iMyConnectTimeoutMs = pDist->GetAgentConnectTimeoutMs();
				pConn->m_iMyQueryTimeoutMs = ( tQuery.m_iAgentQueryTimeoutMs!=DEFAULT_QUERY_TIMEOUT ? tQuery.m_iAgentQueryTimeoutMs : pDist->GetAgentQueryTimeoutMs() );
				dRemotes.Add ( pConn );
			}

			ARRAY_CONSTFOREACH ( j, pDist->m_dLocal )
			{
				const CSphString& sLocalAgent = pDist->m_dLocal[j];
				tDistrStat.m_dLocalNames.Add ( sLocalAgent );
				auto &dLocal = dLocals.Add ();
				dLocal.m_sName = sLocalAgent;
				dLocal.m_iOrderTag = iOrderTag++;
				if ( iWeight!=-1 )
					dLocal.m_iWeight = iWeight;
				dLocal.m_iMass = GetIndexMass ( sLocalAgent );
				dLocal.m_sParentIndex = sIndex;
				bHasLocalsAgents = true;
			}

			bDivideRemote |= pDist->m_bDivideRemoteRanges;
		}
	}

	// set remote divider
	if ( bDivideRemote )
	{
		if ( iDistCount==1 )
			iDivideLimits = dRemotes.GetLength();
		else
		{
			for ( auto& dResult : m_dNAggrResults )
				dResult.m_sWarning.SetSprintf ( "distributed multi-table query '%s' doesn't support divide_remote_ranges", tQuery.m_sIndexes.cstr() );
		}
	}

	// eliminate local dupes that come from distributed indexes
	if ( bHasLocalsAgents )
		UniqLocals ( dLocals );
	else
		m_dLocal.SwapData ( dLocals );

	return !bSysVar;
}

// generate warning about slow full text expansion for queries there
// merged terms is less then expanded terms
// slower then query_log_min_msec or slower 100ms
static void CheckExpansion ( CSphQueryResultMeta & tMeta )
{
	if ( tMeta.m_hWordStats.IsEmpty() || !tMeta.m_tExpansionStats.m_iTerms )
		return;

	if ( tMeta.m_tExpansionStats.m_iMerged>=tMeta.m_tExpansionStats.m_iTerms )
		return;

	if ( tMeta.m_iQueryTime<100 || ( g_iQueryLogMinMs>0 && tMeta.m_iQueryTime<g_iQueryLogMinMs ) )
		return;

	int iTotal = tMeta.m_tExpansionStats.m_iMerged + tMeta.m_tExpansionStats.m_iTerms;
	int iMerged = (int)( float(tMeta.m_tExpansionStats.m_iMerged) * 100.0f / iTotal );

	StringBuilder_c sBuf;
	// notice, msg should not be finished with dot, and start with capital. That is because several messages can be unified with '; ' delimiter.
	sBuf.Appendf ( "current merge of expanded terms is %d%%, with a total of %d. Read manual about 'expansion_merge_threshold_docs/hits'", iMerged, iTotal );
	if ( !tMeta.m_sWarning.IsEmpty() )
		sBuf.Appendf ( "; %s", tMeta.m_sWarning.cstr() );

	sBuf.MoveTo ( tMeta.m_sWarning );
}

// query info - render query into the view
struct QueryInfo_t : public TaskInfo_t
{
	DECLARE_RENDER( QueryInfo_t );

	// actually it is 'virtually hazard'. Don't care about query* itself, however later in dtr of Searchandler_t
	// will work with refs to members of it's m_dQueries and retire or whole vec.
	std::atomic<const CSphQuery *> m_pHazardQuery;
};

DEFINE_RENDER ( QueryInfo_t )
{
	auto & tInfo = *(QueryInfo_t *) pSrc;
	dDst.m_sChain << "Query ";
	hazard::Guard_c tGuard;
	auto pQuery = tGuard.Protect ( tInfo.m_pHazardQuery );
	if ( pQuery && session::GetProto()!=Proto_e::MYSQL41 ) // cheat: for mysql query not used, so will not copy it then
		dDst.m_pQuery = std::make_unique<CSphQuery> ( *pQuery );
}

static void FillupFacetError ( int iQueries, const CSphVector<CSphQuery> & dQueries, VecTraits_T<AggrResult_t> & dAggrResults )
{
	if ( iQueries>1 && !dAggrResults.Begin()->m_iSuccesses && dAggrResults.Begin()->m_sError.IsEmpty() && dQueries.Begin()->m_bFacetHead )
	{
		const CSphString * pError = nullptr;
		for ( int iRes=0; iRes<iQueries; ++iRes )
		{
			const AggrResult_t & tRes = dAggrResults[iRes];
			if ( !tRes.m_iSuccesses && !tRes.m_sError.IsEmpty() )
			{
				pError = &tRes.m_sError;
				break;
			}
		}

		if ( !pError )
			return;

		for ( int iRes=0; iRes<iQueries; ++iRes )
		{
			AggrResult_t & tRes = dAggrResults[iRes];
			if ( !tRes.m_sError.IsEmpty() )
				break;

			tRes.m_sError = *pError;
		}
	}
}

// one or more queries against one and same set of indexes
void SearchHandler_c::RunSubset ( int iStart, int iEnd )
{
	int iQueries = iEnd - iStart;
	m_dNQueries = m_dQueries.Slice ( iStart, iQueries );
	m_dNJoinQueryOptions = m_dJoinQueryOptions.Slice ( iStart, iQueries );
	m_dNAggrResults = m_dAggrResults.Slice ( iStart, iQueries );
	m_dNResults = m_dResults.Slice ( iStart, iQueries );
	m_dNFailuresSet = m_dFailuresSet.Slice ( iStart, iQueries );

	// we've own scoped context here
	auto pQueryInfo = new QueryInfo_t;
	pQueryInfo->m_pHazardQuery.store ( m_dNQueries.begin(), std::memory_order_release );
	ScopedInfo_T<QueryInfo_t> pTlsQueryInfo ( pQueryInfo );

	// all my stats
	int64_t tmSubset = -sphMicroTimer();
	int64_t tmLocal = 0;
	int64_t tmCpu = -sphTaskCpuTimer ();

	CSphScopedProfile tProf ( m_pProfile, SPH_QSTATE_UNKNOWN );

	// prepare for descent
	const CSphQuery & tFirst = m_dNQueries.First();
	m_dNAggrResults.Apply ( [] ( AggrResult_t & r ) { r.m_iSuccesses = 0; } );

	if ( iQueries==1 && m_pProfile )
	{
		m_dNAggrResults.First().m_pProfile = m_pProfile;
		m_tHook.SetProfiler ( m_pProfile );
	}

	// check for facets
	m_bFacetQueue = iQueries>1;
	for ( int iCheck = 1; iCheck<m_dNQueries.GetLength () && m_bFacetQueue; ++iCheck )
		if ( !m_dNQueries[iCheck].m_bFacet )
			m_bFacetQueue = false;

	m_bMultiQueue = m_bFacetQueue || CheckMultiQuery();

	////////////////////////////
	// build local indexes list
	////////////////////////////
	VecRefPtrsAgentConn_t dRemotes;
	CSphVector<DistrServedByAgent_t> dDistrServedByAgent;
	int iDivideLimits = 1;

	auto fnError = AtScopeExit ( [this]()
	{
		if ( !m_sError.IsEmpty() )
			m_dNAggrResults.for_each ( [this] ( auto& r ) { r.m_sError = (CSphString) m_sError; } );
	});

	if ( BuildIndexList ( iDivideLimits, dRemotes, dDistrServedByAgent ) )
	{
		// process query to meta, as myindex.status, etc.
		if ( !tFirst.m_dStringSubkeys.IsEmpty () )
		{
			// if apply subkeys ... else return
			if ( !ParseIdxSubkeys () )
				return;
		} else if ( !AcquireInvokedIndexes () ) // usual query processing
			return;
	} else
	{
		// process query to @@*, as @@system.threads, etc.
		if ( !ParseSysVar () )
			return;
		// here we deal
	}

	// at this point m_dLocal contains list of valid local indexes (i.e., existing ones),
	// and these indexes are also rlocked and available by calling m_dAcquired.Get()

	// sanity check
	if ( dRemotes.IsEmpty() && m_dLocal.IsEmpty() )
	{
		m_sError << "no enabled tables to search";
		return;
	}

	if ( m_dNQueries[0].m_iLimit==-1 && ( !dRemotes.IsEmpty () || m_dLocal.GetLength ()>1 ) )
	{
		m_sError << "only one local table allowed in streaming select";
		return;
	}

	// select lists must have no expressions
	if ( m_bMultiQueue )
		m_bMultiQueue = AllowsMulti ();

	assert ( !m_bFacetQueue || AllowsMulti () );
	if ( !m_bMultiQueue )
		m_bFacetQueue = false;

	///////////////////////////////////////////////////////////
	// main query loop (with multiple retries for distributed)
	///////////////////////////////////////////////////////////

	// connect to remote agents and query them, if required
	std::unique_ptr<SearchRequestBuilder_c> tReqBuilder;
	CSphRefcountedPtr<RemoteAgentsObserver_i> tReporter { nullptr };
	std::unique_ptr<ReplyParser_i> tParser;
	if ( !dRemotes.IsEmpty() )
	{
		SwitchProfile(m_pProfile, SPH_QSTATE_DIST_CONNECT);
		tReqBuilder = std::make_unique<SearchRequestBuilder_c> ( m_dNQueries, iDivideLimits );
		tParser = std::make_unique<SearchReplyParser_c> ( iQueries );
		tReporter = GetObserver();

		// run remote queries. tReporter will tell us when they're finished.
		// also blackholes will be removed from this flow of remotes.
		ScheduleDistrJobs ( dRemotes, tReqBuilder.get (),
			tParser.get (),
			tReporter, tFirst.m_iRetryCount, tFirst.m_iRetryDelay );
	}

	/////////////////////
	// run local queries
	//////////////////////

	// while the remote queries are running, do local searches
	if ( m_dLocal.GetLength() )
	{
		SetupLocalDF();

		SwitchProfile ( m_pProfile, SPH_QSTATE_LOCAL_SEARCH );
		m_bNeedDocIDs = m_dLocal.GetLength()+dRemotes.GetLength()>1;
		tmLocal = -sphMicroTimer();
		tmCpu -= sphTaskCpuTimer ();
		RunLocalSearches();
		tmCpu += sphTaskCpuTimer ();
		tmLocal += sphMicroTimer();
	}

	///////////////////////
	// poll remote queries
	///////////////////////

	if ( !dRemotes.IsEmpty() )
	{
		SwitchProfile ( m_pProfile, SPH_QSTATE_DIST_WAIT );

		bool bDistDone = false;
		while ( !bDistDone )
		{
			// don't forget to check incoming replies after send was over
			bDistDone = tReporter->IsDone();
			if ( !bDistDone )
				tReporter->WaitChanges (); /// wait one or more remote queries to complete. Note! M.b. context switch!

			ARRAY_FOREACH ( iAgent, dRemotes )
			{
				AgentConn_t * pAgent = dRemotes[iAgent];
				assert ( !pAgent->IsBlackhole () ); // must not be any blacknole here.

				if ( !pAgent->m_bSuccess )
					continue;

				sphLogDebugv ( "agent %d, state %s, order %d, sock %d", iAgent, pAgent->StateName(), pAgent->m_iStoreTag, pAgent->m_iSock );

				DistrServedByAgent_t * pDistr = nullptr;
				for ( auto &tDistr : dDistrServedByAgent )
					if ( tDistr.m_dAgentIds.Contains ( iAgent ) )
					{
						pDistr = &tDistr;
						break;
					}
				assert ( pDistr );

				// merge this agent's results
				for ( int iRes = 0; iRes<iQueries; ++iRes )
				{
					auto pResult = ( cSearchResult * ) pAgent->m_pResult.get ();
					if ( !pResult )
						continue;

					auto &tRemoteResult = pResult->m_dResults[iRes];

					// copy errors or warnings
					if ( !tRemoteResult.m_sError.IsEmpty() )
						m_dNFailuresSet[iRes].SubmitEx ( tFirst.m_sIndexes, nullptr,
							"agent %s: remote query error: %s",
							pAgent->m_tDesc.GetMyUrl().cstr(), tRemoteResult.m_sError.cstr() );
					if ( !tRemoteResult.m_sWarning.IsEmpty() )
						m_dNFailuresSet[iRes].SubmitEx ( tFirst.m_sIndexes, nullptr,
							"agent %s: remote query warning: %s",
							pAgent->m_tDesc.GetMyUrl().cstr(), tRemoteResult.m_sWarning.cstr() );

					if ( tRemoteResult.m_iSuccesses<=0 )
						continue;

					AggrResult_t & tRes = m_dNAggrResults[iRes];
					++tRes.m_iSuccesses;

					assert ( tRemoteResult.m_dResults.GetLength() == 1 ); // by design remotes return one chunk
					auto & dRemoteChunk = tRes.m_dResults.Add ();
					::Swap ( dRemoteChunk, *tRemoteResult.m_dResults.begin () );

					// note how we do NOT add per-index weight here

					// merge this agent's stats
					tRes.m_iTotalMatches += tRemoteResult.m_iTotalMatches;
					tRes.m_bTotalMatchesApprox |= tRemoteResult.m_bTotalMatchesApprox;
					tRes.m_iQueryTime += tRemoteResult.m_iQueryTime;
					tRes.m_iAgentCpuTime += tRemoteResult.m_iCpuTime;
					tRes.m_tAgentIOStats.Add ( tRemoteResult.m_tIOStats );
					tRes.m_iAgentPredictedTime += tRemoteResult.m_iPredictedTime;
					tRes.m_iAgentFetchedDocs += tRemoteResult.m_iAgentFetchedDocs;
					tRes.m_iAgentFetchedHits += tRemoteResult.m_iAgentFetchedHits;
					tRes.m_iAgentFetchedSkips += tRemoteResult.m_iAgentFetchedSkips;
					tRes.m_bHasPrediction |= ( m_dNQueries[iRes].m_iMaxPredictedMsec>0 );

					if ( pDistr )
					{
						pDistr->m_dStats[iRes].m_uQueryTime += tRemoteResult.m_iQueryTime;
						pDistr->m_dStats[iRes].m_uFoundRows += tRemoteResult.m_iTotalMatches;
						++pDistr->m_dStats[iRes].m_iSuccesses;
					}

					// merge this agent's words
					tRes.MergeWordStats ( tRemoteResult );
				}

				// dismissed
				if ( pAgent->m_pResult )
					pAgent->m_pResult->Reset ();
				pAgent->m_bSuccess = false;
				pAgent->m_sFailure = "";
			}
		} // while ( !bDistDone )

		// submit failures from failed agents
		// copy timings from all agents

		for ( const AgentConn_t * pAgent : dRemotes )
		{
			assert ( !pAgent->IsBlackhole () ); // must not be any blacknole here.

			for ( int j=iStart; j<iEnd; ++j )
			{
				assert ( pAgent->m_iWall>=0 );
				m_dAgentTimes[j].Add ( ( pAgent->m_iWall ) / ( 1000 * iQueries ) );
			}

			if ( !pAgent->m_bSuccess && !pAgent->m_sFailure.IsEmpty() )
				for ( int j=0; j<iQueries; ++j )
					m_dNFailuresSet[j].SubmitEx ( tFirst.m_sIndexes, nullptr, "agent %s: %s",
						pAgent->m_tDesc.GetMyUrl().cstr(), pAgent->m_sFailure.cstr() );
		}
	}

	/////////////////////
	// merge all results
	/////////////////////

	SwitchProfile ( m_pProfile, SPH_QSTATE_AGGREGATE );

	CSphIOStats tIO;

	for ( int iRes=0; iRes<iQueries; ++iRes )
	{
		sph::StringSet hExtra;
		for ( const CSphString & sExtra : m_dExtraSchema )
			hExtra.Add ( sExtra );

		AggrResult_t & tRes = m_dNAggrResults[iRes];
		const CSphQuery & tQuery = m_dNQueries[iRes];

		// minimize sorters needs these pointers
		tIO.Add ( tRes.m_tIOStats );

		// if there were no successful searches at all, this is an error
		if ( !tRes.m_iSuccesses )
		{
			StringBuilder_c sFailures;
			m_dNFailuresSet[iRes].BuildReport ( sFailures );
			sFailures.MoveTo (tRes.m_sError);
			continue;
		}

		if ( tRes.m_dResults.IsEmpty () ) // fixup. It is easier to have single empty result, then check each time.
		{
			auto& tEmptyRes = tRes.m_dResults.Add ();
			tEmptyRes.m_tSchema = tRes.m_tSchema;
		}

		// minimize schema and remove dupes
		// assuming here ( tRes.m_tSchema==tRes.m_dSchemas[0] )
		const CSphFilterSettings * pAggrFilter = nullptr;
		if ( m_bMaster && !tQuery.m_tHaving.m_sAttrName.IsEmpty() )
			pAggrFilter = &tQuery.m_tHaving;

		const CSphVector<CSphQueryItem> & dItems = ( tQuery.m_dRefItems.GetLength() ? tQuery.m_dRefItems : tQuery.m_dItems );

		if ( tRes.m_iSuccesses>1 || dItems.GetLength() || pAggrFilter )
		{
			if ( m_bMaster && tRes.m_iSuccesses && dItems.GetLength() && tQuery.m_sGroupBy.IsEmpty() && tRes.GetLength()==0 )
			{
				for ( auto& dItem : dItems )
				{
					if ( dItem.m_sExpr=="count(*)" || ( dItem.m_sExpr=="@distinct" ) )
						tRes.m_dZeroCount.Add ( dItem.m_sAlias );
				}
			}

			bool bOk = MinimizeAggrResult ( tRes, tQuery, !m_dLocal.IsEmpty(), hExtra, m_pProfile, pAggrFilter, m_bFederatedUser, m_bMaster );

			if ( !bOk )
			{
				tRes.m_iSuccesses = 0;
				continue;
			}
		} else if ( !tRes.m_dResults.IsEmpty() )
		{
			tRes.m_tSchema = tRes.m_dResults.First ().m_tSchema;
			Debug ( tRes.m_bOneSchema = true; )
		}

		if ( !m_dNFailuresSet[iRes].IsEmpty() )
		{
			StringBuilder_c sFailures;
			m_dNFailuresSet[iRes].BuildReport ( sFailures );
			sFailures.MoveTo ( tRes.m_sWarning );
		}
		CheckExpansion ( tRes );

		////////////
		// finalize
		////////////

		tRes.m_iOffset = Max ( tQuery.m_iOffset, tQuery.m_iOuterOffset );
		auto iLimit = ( tQuery.m_iOuterLimit ? tQuery.m_iOuterLimit : tQuery.m_iLimit );
		tRes.m_iCount = Max ( Min ( iLimit, tRes.GetLength()-tRes.m_iOffset ), 0 );
		tRes.m_iMatches = tRes.m_iCount;
		for ( const auto & tLocal : m_dLocal )
			tRes.m_dIndexNames.Add ( tLocal.m_sName );
	}

	// pop up facet error from one of the query to the front
	FillupFacetError ( iQueries, m_dQueries, m_dNAggrResults );

	/////////////////////////////////
	// functions on a table argument
	/////////////////////////////////

	for ( int i=0; i<iQueries; ++i )
	{
		AggrResult_t & tRes = m_dNAggrResults[i];
		auto& pTableFunc = m_dTables[iStart+i];

		// FIXME! log such queries properly?
		if ( pTableFunc )
		{
			SwitchProfile ( m_pProfile, SPH_QSTATE_TABLE_FUNC );
			if ( !pTableFunc->Process ( &tRes, tRes.m_sError ) )
				tRes.m_iSuccesses = 0;
		}
	}

	/////////
	// stats
	/////////

	tmSubset += sphMicroTimer();
	tmCpu += sphTaskCpuTimer();

	CalcTimeStats ( tmCpu, tmSubset, dDistrServedByAgent );
	CalcPerIndexStats ( dDistrServedByAgent );
	CalcGlobalStats ( tmCpu, tmSubset, tmLocal, tIO, dRemotes );
}
