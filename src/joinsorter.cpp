//
//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "joinsorter.h"

#include "std/hash.h"
#include "std/openhash.h"
#include "sphinxquery.h"
#include "sphinxsort.h"
#include "sphinxjson.h"
#include "querycontext.h"
#include "docstore.h"

static int64_t g_iJoinCacheSize = 20971520;
static int g_iJoinBatchSize = 1000;

void SetJoinCacheSize ( int64_t iSize )
{
	g_iJoinCacheSize = iSize;
}


int64_t GetJoinCacheSize()
{
	return g_iJoinCacheSize;
}


void SetJoinBatchSize ( int iSize )
{
	g_iJoinBatchSize = iSize;
}


int	GetJoinBatchSize()
{
	return g_iJoinBatchSize;
}


bool ExprHasLeftTableAttrs ( const CSphString & sAttr, const ISphSchema & tLeftSchema )
{
	const char * szAttr = sAttr.cstr();
	const int MAX_ATTR_NAME_LEN = 512;
	char dAttrName[MAX_ATTR_NAME_LEN+1];
	int iPos = 0;
	while ( *szAttr )
	{
		if ( sphIsAlphaOnly(*szAttr) || *szAttr=='.' || *szAttr=='_' )
		{
			if ( iPos==MAX_ATTR_NAME_LEN-1 )
				break;

            dAttrName[iPos++] = *szAttr;
        }
		else
		{
            if (iPos)
			{
                dAttrName[iPos] = '\0';
				if ( !strchr ( dAttrName, '.' ) && tLeftSchema.GetAttrIndex(dAttrName)!=-1 )
					return true;

                iPos = 0;
            }
        }

        szAttr++;
    }

    if (iPos)
	{
        dAttrName[iPos] = '\0';
		if ( !strchr ( dAttrName, '.' ) && tLeftSchema.GetAttrIndex(dAttrName)!=-1 )
			return true;
    }

	return false;
}


static bool GetJoinAttrName ( const CSphString & sAttr, const CSphString & sJoinedIndex, const ISphSchema & tLeftSchema, CSphString * pModified = nullptr )
{
	CSphString sPrefix;
	sPrefix.SetSprintf ( "%s.", sJoinedIndex.cstr() );
	int iPrefixLen = sPrefix.Length();

	if ( ExprHasLeftTableAttrs ( sAttr, tLeftSchema ) )
		return false;

	bool bRightTable = false;
	CSphString sMod = sAttr;
	const char * szStart = sMod.cstr();

	while ( true )
	{
		const char * szFound = strstr ( sMod.cstr(), sPrefix.cstr() );
		if ( !szFound )
			break;

		if ( szFound > szStart )
		{
			char c = *(szFound-1);
			if ( ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='_' )
				continue;		
		}

		bRightTable = true;
		int iStart = szFound-sMod.cstr();
		CSphString sNewExprPre = iStart > 0 ? sMod.SubString ( 0, iStart ) : "";

		int iPostLen = sMod.Length()-iStart-iPrefixLen;
		CSphString sNewExprPost = iPostLen > 0 ? sMod.SubString ( iStart + iPrefixLen, sMod.Length()-iStart-iPrefixLen ) : "";
		sMod.SetSprintf ( "%s%s", sNewExprPre.cstr(), sNewExprPost.cstr() );
	}

	if ( bRightTable )
	{
		if ( pModified )
			*pModified = sMod;

		return true;
	}

	return false;
}


static StrVec_t ParseGroupBy ( const CSphString & sGroupBy )
{
	StrVec_t dRes;
	sphSplit ( dRes, sGroupBy.cstr(), ", \t\n" );
	return dRes;
}


static const char * GetBatchedItemPrefix()
{
	return "@batched_join_";
}


CSphVector<std::pair<int,bool>> FetchJoinRightTableFilters ( const CSphVector<CSphFilterSettings> & dFilters, const ISphSchema & tSchema, const char * szJoinedIndex )
{
	CSphString sPrefix;
	sPrefix.SetSprintf ( "%s.", szJoinedIndex );

	CSphVector<std::pair<int,bool>> dRightFilters;
	ARRAY_FOREACH ( i, dFilters )
	{
		const auto & tFilter = dFilters[i];
		bool bHasPrefix = tFilter.m_sAttrName.Begins ( sPrefix.cstr() );
		const CSphColumnInfo * pFilterAttr = tSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
		if ( pFilterAttr )
		{
			if ( !pFilterAttr->IsJoined() )
				continue;
		}
		else
		{
			if ( !bHasPrefix )
				continue;
		}

		dRightFilters.Add ( {i,bHasPrefix} );
	}

	return dRightFilters;
}


bool NeedToMoveMixedJoinFilters ( const CSphQuery & tQuery, const ISphSchema & tSchema )
{
	CSphVector<std::pair<int,bool>> dRightFilters = FetchJoinRightTableFilters ( tQuery.m_dFilters, tSchema, tQuery.m_sJoinIdx.cstr() );
	if ( !dRightFilters.GetLength() )
		return false;

	// move all filters to the left query in case of LEFT JOIN
	// otherwise we can't distinguish between 'no match' and 'match with null part from right table'
	if ( tQuery.m_eJoinType==JoinType_e::LEFT )
		return true;

	if ( !tQuery.m_dFilterTree.GetLength() )
		return false;

	return dRightFilters.GetLength()!=tQuery.m_dFilters.GetLength();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MatchPtrIterator_c
{
public:
	MatchPtrIterator_c ( CSphMatch ** ppMatch ) : m_ppMatch ( ppMatch ) {}

	FORCE_INLINE CSphMatch & operator*() const { return **m_ppMatch; }
	FORCE_INLINE bool operator!= ( const MatchPtrIterator_c & tRhs ) const { return m_ppMatch != tRhs.m_ppMatch; }
    FORCE_INLINE MatchPtrIterator_c & operator++()
    {
        ++m_ppMatch;
        return *this;
    }

private:
	CSphMatch ** m_ppMatch = nullptr;
};


class MatchPtrVec_c
{
public:
	MatchPtrVec_c ( CSphVector<CSphMatch *>	& dMatchPtrs ) : m_dMatchPtrs ( dMatchPtrs ) {}

	FORCE_INLINE MatchPtrIterator_c begin() const	{ return MatchPtrIterator_c ( m_dMatchPtrs.begin() ); }
	FORCE_INLINE MatchPtrIterator_c end() const		{ return MatchPtrIterator_c ( m_dMatchPtrs.end() ); }

	int GetLength() const { return m_dMatchPtrs.GetLength(); }

private:
	CSphVector<CSphMatch *>	& m_dMatchPtrs;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// FIXME! maybe replace it with a LRU cache
class MatchCache_c
{
public:
						MatchCache_c ( uint64_t uCacheSize, int iBatchSize );
						~MatchCache_c();

	void				SetSchema ( const ISphSchema * pSchema );

	template <typename MATCHES>
	bool				Add ( uint64_t uHash, const MATCHES & dMatches );
	FORCE_INLINE bool	Fetch ( uint64_t uHash, CSphSwapVector<CSphMatch> & dMatches );
	FORCE_INLINE bool	IsFull() const { return m_uMaxSize && m_uCurSize>=m_uMaxSize; }

private:
	// a simplified match (incoming matches don't have a static part)
	struct StoredMatch_t
	{
		CSphRowitem *	m_pDynamic = nullptr;
	};

	static const int INITIAL_HASH_SIZE = 4096;

	using StoredMatches_t = CSphVector<StoredMatch_t>;
	OpenHashTable_T<uint64_t, StoredMatches_t> m_hCache;

	uint64_t			m_uMaxSize = 0;
	uint64_t			m_uCurSize = 0;
	uint64_t			m_uFetched = 0;
	int					m_iBatchSize = 0;

	std::unique_ptr<ISphSchema> m_pSchema;
	uint64_t			CalcMatchMem ( const CSphMatch & tMatch );
};


MatchCache_c::MatchCache_c ( uint64_t uCacheSize, int iBatchSize )
	: m_hCache ( INITIAL_HASH_SIZE )
	, m_uMaxSize ( uCacheSize )
	, m_iBatchSize ( iBatchSize )
{}


MatchCache_c::~MatchCache_c()
{
	int64_t iIterator = 0;
	std::pair<SphGroupKey_t, StoredMatches_t*> tRes;
	while ( ( tRes = m_hCache.Iterate ( iIterator ) ).second )
	{
		StoredMatches_t & dMatches = *tRes.second;
		for ( auto & i : dMatches )
		{
			CSphMatch tStub;
			tStub.m_pDynamic = i.m_pDynamic;
			m_pSchema->FreeDataPtrs(tStub);
			tStub.ResetDynamic();
		}
	}
}


void MatchCache_c::SetSchema ( const ISphSchema * pSchema )
{
	if ( m_pSchema )
		return;

	// keep a clone of the schema
	// we assume that the schema won't change during the lifetime of the cache
	m_pSchema = std::unique_ptr<ISphSchema> ( pSchema->CloneMe() );
}


uint64_t MatchCache_c::CalcMatchMem ( const CSphMatch & tMatch )
{
	uint64_t uMem = 0;
	for ( int i = 0; i < m_pSchema->GetAttrsCount(); i++ )
	{
		const auto & tAttr = m_pSchema->GetAttr(i);
		if ( !tAttr.IsDataPtr() )
			continue;

		const BYTE * pBlob = (const BYTE *)sphGetRowAttr ( tMatch.m_pDynamic, tAttr.m_tLocator );
		uMem += sphUnpackPtrAttr(pBlob).second;
	}

	uMem += m_pSchema->GetDynamicSize()*sizeof(CSphRowitem);
	uMem += sizeof(StoredMatch_t);
	return uMem;
}

template <typename MATCHES>
bool MatchCache_c::Add ( uint64_t uHash, const MATCHES & dMatches )
{
	if ( !m_pSchema )
		return false;

	if ( IsFull() )
		return false;

	// if we're inserting but not fetching anything back, there's high probability that we are dealing with unique JOIN ON conditions
	// there's no point in caching those
	int64_t iThresh = Max ( int64_t(8)*m_iBatchSize, 32768 );
	if ( m_hCache.GetLength() >= iThresh )
	{
		float fRatio = float(m_uFetched)/iThresh;
		if ( fRatio<=0.0001f )
			return false;
	}

	uint64_t uEntrySize = 0;
	StoredMatches_t dStoredMatches;
	for ( const auto & i : dMatches )
	{
		dStoredMatches.Add ( { i.m_pDynamic } );
		uEntrySize += CalcMatchMem(i);
	}

	uEntrySize += m_hCache.GetEntrySize();

	if ( m_hCache.Add ( uHash, dStoredMatches ) )
	{
		m_uCurSize += uEntrySize;
		return true;
	}

	return false;
}


bool MatchCache_c::Fetch ( uint64_t uHash, CSphSwapVector<CSphMatch> & dMatches )
{
	StoredMatches_t * pMatches = m_hCache.Find(uHash);
	if ( !pMatches )
		return false;

	dMatches.Resize ( pMatches->GetLength() );
	for ( int i = 0; i < pMatches->GetLength(); i++ )
		dMatches[i].m_pDynamic = (*pMatches)[i].m_pDynamic;

	m_uFetched++;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MatchCalc_c : public MatchProcessor_i
{
public:
	MatchCalc_c ( ISphMatchSorter * pSorter ) : m_pSorter ( pSorter ) {}

	void Process ( CSphMatch * pMatch ) override	{ m_pSorter->Push ( *pMatch ); }
	bool ProcessInRowIdOrder() const override		{ return false; }
	void Process ( VecTraits_T<CSphMatch *> & dMatches ) override
	{
		for ( auto & i : dMatches )
			Process(i);
	}

protected:
	ISphMatchSorter * m_pSorter = nullptr;
};


class MatchCalcGrouped_c : public MatchCalc_c
{
public:
	MatchCalcGrouped_c ( ISphMatchSorter * pSorter ) : MatchCalc_c ( pSorter ) {}

	void Process ( CSphMatch * pMatch ) final
	{
		m_pSorter->PushGrouped ( *pMatch, m_bFirst );
		m_bFirst = false;
	}

private:
	bool m_bFirst = true;
};


class StoredFetch_c : public MatchProcessor_i
{
public:
			StoredFetch_c ( const ISphSchema & tSorterSchema, const CSphIndex & tRightIndex, uint64_t uNullMask );

	void	Process ( CSphMatch * pMatch ) override;
	bool	ProcessInRowIdOrder() const override	{ return false; }
	void	Process ( VecTraits_T<CSphMatch *> & dMatches ) override;

	bool	HasFieldToFetch() const					{ return !m_dFieldToFetch.IsEmpty(); }

private:
	const ISphSchema &					m_tSorterSchema;
	const CSphIndex &					m_tRightIndex;
	const CSphColumnInfo *				m_pId = nullptr;
	const CSphColumnInfo *				m_pNullMaskAttr = nullptr;
	std::unique_ptr<DocstoreSession_c>	m_pSession;
	int64_t								m_iSessionUID = 0;
	IntVec_t							m_dFieldToFetch;
	CSphVector<CSphAttrLocator>			m_dAttrRemap;
	uint64_t							m_uNullMask = 0;

	void	CreateDocstoreSession();
	void	SetupAttrRemap();
};


StoredFetch_c::StoredFetch_c ( const ISphSchema & tSorterSchema, const CSphIndex & tRightIndex, uint64_t uNullMask )
	: m_tSorterSchema ( tSorterSchema )
	, m_tRightIndex ( tRightIndex )
	, m_uNullMask ( uNullMask )
{
	CreateDocstoreSession();
	SetupAttrRemap();
}


void StoredFetch_c::Process ( CSphMatch * pMatch )
{
	assert ( !m_dFieldToFetch.IsEmpty() );
	assert(m_pId);

	SphAttr_t tDocId = pMatch->GetAttr ( m_pId->m_tLocator );

	// compare against a preset null mask that the join sorter uses
	bool bNull = m_pNullMaskAttr && pMatch->GetAttr ( m_pNullMaskAttr->m_tLocator )==m_uNullMask;

	DocstoreDoc_t tDoc;
	if ( !bNull && m_tRightIndex.GetDoc ( tDoc, tDocId, &m_dFieldToFetch, m_iSessionUID, true ) )
	{
		ARRAY_FOREACH ( i, tDoc.m_dFields )
		{
			const CSphAttrLocator & tLoc = m_dAttrRemap[i];
			auto pPrev = (BYTE*)pMatch->GetAttr(tLoc);
			SafeDeleteArray(pPrev);

			pMatch->SetAttr ( tLoc, (SphAttr_t)tDoc.m_dFields[i].LeakData() );
		}
	}
	else
	{
		ARRAY_FOREACH ( i, tDoc.m_dFields )
		{
			auto pPrev = (BYTE*)pMatch->GetAttr ( m_dAttrRemap[i] );
			SafeDeleteArray(pPrev);
		}
	}
}


void StoredFetch_c::Process ( VecTraits_T<CSphMatch *> & dMatches )
{
	for ( auto & i : dMatches )
		Process(i);
}


void StoredFetch_c::CreateDocstoreSession()
{
	m_pSession = std::make_unique<DocstoreSession_c>();
	m_iSessionUID = m_pSession->GetUID();

	// spawn buffered readers for the current session
	m_tRightIndex.CreateReader(m_iSessionUID);
}


void StoredFetch_c::SetupAttrRemap()
{
	CSphString sAttrName;
	sAttrName.SetSprintf ( "%s.%s", m_tRightIndex.GetName(), sphGetDocidName() );
	m_pId = m_tSorterSchema.GetAttr ( sAttrName.cstr() );
	assert(m_pId);

	const ISphSchema & tRightSchema = m_tRightIndex.GetMatchSchema();
	for ( int i = 0; i < tRightSchema.GetFieldsCount(); i++ )
	{
		const CSphColumnInfo & tField = tRightSchema.GetField(i);
		if ( !( tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED ) )
			continue;

		CSphString sAttrName;
		sAttrName.SetSprintf ( "%s.%s", m_tRightIndex.GetName(), tField.m_sName.cstr() );

		int iDocStoreFieldId = m_tRightIndex.GetFieldId ( tField.m_sName, DOCSTORE_TEXT );
		int iAttrId = m_tSorterSchema.GetAttrIndex ( sAttrName.cstr() );
		assert ( iDocStoreFieldId!=-1 && iAttrId!=-1 );

		m_dFieldToFetch.Add(iDocStoreFieldId);
		m_dAttrRemap.Add ( m_tSorterSchema.GetAttr(iAttrId).m_tLocator );
	}

	m_pNullMaskAttr = m_tSorterSchema.GetAttr ( GetNullMaskAttrName() );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class FilterEval_c
{
public:
	void				SetFilter ( std::unique_ptr<ISphFilter> & pFilter )	{ m_pFilter = std::move ( pFilter ); }
	void				SetBlobPool ( const BYTE * pBlobPool )				{ if ( m_pFilter ) m_pFilter->SetBlobStorage(pBlobPool); }
	void				SetColumnar ( columnar::Columnar_i * pColumnar )	{ if ( m_pFilter ) m_pFilter->SetColumnar(pColumnar); }
	FORCE_INLINE bool	Eval ( const CSphMatch & tMatch ) const				{ return m_pFilter ? m_pFilter->Eval(tMatch) : true; }

private:
	std::unique_ptr<ISphFilter>	m_pFilter;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class JoinSorter_c : public ISphMatchSorter
{
public:
				JoinSorter_c ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const CSphQuery & tQuery, const CSphQuery & tJoinQueryOptions, ISphMatchSorter * pSorter, bool bJoinedGroupSort, int iBatchSize );
				JoinSorter_c ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const VecTraits_T<const CSphQuery> & dQueries, const VecTraits_T<const CSphQuery> & dJoinQueryOptions, ISphMatchSorter * pSorter, bool bJoinedGroupSort, int iBatchSize );

	bool		IsGroupby() const override											{ return m_pSorter->IsGroupby(); }
	void		SetState ( const CSphMatchComparatorState & tState ) override		{ m_pSorter->SetState(tState); }
	const		CSphMatchComparatorState & GetState() const override				{ return m_pSorter->GetState(); }
	void		SetGroupState ( const CSphMatchComparatorState & tState ) override	{ m_pSorter->SetGroupState(tState); }
	void		SetBlobPool ( const BYTE * pBlobPool ) override;
	void		SetColumnar ( columnar::Columnar_i * pColumnar ) override;
	void		SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) override;
	const ISphSchema *	GetSchema() const override									{ return m_pSorter->GetSchema(); }
	bool		Push ( const CSphMatch & tEntry ) override							{ return Push_T ( tEntry, [this]( const CSphMatch & tMatch ){ return m_pSorter->Push(tMatch); }, false ); }
	void		Push ( const VecTraits_T<const CSphMatch> & dMatches ) override;
	bool		PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override		{ return Push_T ( tEntry, [this,bNewSet]( const CSphMatch & tMatch ){ return m_pSorter->PushGrouped ( tMatch, bNewSet ); }, true ); }
	int			GetLength() override												{ return m_pSorter->GetLength(); }
	int64_t		GetTotalCount() const override;
	void		Finalize ( MatchProcessor_i & tProcessor, bool bCallProcessInResultSetOrder, bool bFinalizeMatches ) override { m_pSorter->Finalize ( tProcessor, bCallProcessInResultSetOrder, bFinalizeMatches ); }
	int			Flatten ( CSphMatch * pTo ) override								{ return m_pSorter->Flatten(pTo); }
	const CSphMatch * GetWorst() const override										{ return m_pSorter->GetWorst(); }
	bool		CanBeCloned() const override										{ return m_pSorter->CanBeCloned(); }
	ISphMatchSorter * Clone() const override;
	void		MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) override			{ m_pSorter->MoveTo ( ((JoinSorter_c *)pRhs)->m_pSorter.get(), bCopyMeta ); }
	void		CloneTo ( ISphMatchSorter * pTrg ) const override					{ m_pSorter->CloneTo(pTrg); }
	void		SetFilteredAttrs ( const sph::StringSet & hAttrs, bool bAddDocid ) override	{ m_pSorter->SetFilteredAttrs(hAttrs, bAddDocid); }
	void		TransformPooled2StandalonePtrs ( GetBlobPoolFromMatch_fn fnBlobPoolFromMatch, GetColumnarFromMatch_fn fnGetColumnarFromMatch, bool bFinalizeSorters ) override { m_pSorter->TransformPooled2StandalonePtrs(fnBlobPoolFromMatch, fnGetColumnarFromMatch, bFinalizeSorters); }
	void		SetRandom ( bool bRandom ) override									{ m_pSorter->SetRandom(bRandom); }
	bool		IsRandom() const override											{ return m_pSorter->IsRandom(); }
	int			GetMatchCapacity() const override									{ return m_pSorter->GetMatchCapacity(); }
	RowTagged_t	GetJustPushed() const override										{ return m_pSorter->GetJustPushed(); }
	VecTraits_T<RowTagged_t> GetJustPopped() const override							{ return m_pSorter->GetJustPopped(); }
	bool		IsCutoffDisabled() const override									{ return m_pSorter->IsCutoffDisabled(); }
	void		SetMerge ( bool bMerge ) override									{ m_pSorter->SetMerge(bMerge); }
	bool		IsPrecalc() const override											{ return false; }
	bool		IsJoin() const override												{ return true; }
	bool		FinalizeJoin ( CSphString & sError, CSphString & sWarning ) override;

	bool		GetErrorFlag() const												{ return m_bErrorFlag; }
	const CSphString & GetErrorMessage() const										{ return m_sErrorMessage; }

protected:
	bool							m_bCanBatch = true;
	template <typename PUSH> bool	RunBatch ( PUSH && fnPush );

	template <typename PUSH> FORCE_INLINE bool Push_T ( const CSphMatch & tMatch, PUSH && fnPush, bool bGrouped );
	template <typename PUSH, typename MATCHES> FORCE_INLINE bool PushJoinedMatches ( const CSphMatch & tEntry, const MATCHES & dMatches, PUSH && fnPush, const BYTE * pBlobPool, columnar::Columnar_i *	pColumnar );
	template <typename PUSH> FORCE_INLINE bool PushLeftMatch ( const CSphMatch & tEntry, PUSH && fnPush, const BYTE * pBlobPool, columnar::Columnar_i *	pColumnar );

	virtual	bool RunFinalBatch();

private:
	struct JoinAttrNameRemap_t
	{
		CSphString	m_sFrom;
		StrVec_t	m_dTo;
	};

	struct JoinAttrRemap_t
	{
		CSphAttrLocator		m_tLocSrc;
		CSphAttrLocator		m_tLocDst;
		bool				m_bJsonRepack = false;
	};

	struct FilterRemap_t
	{
		int				m_iFilterId = -1;
		CSphAttrLocator	m_tLocator;
		CSphAttrLocator	m_tRightStandaloneLocator;
		bool			m_bBlob = false;
	};

	CSphQuery						m_tJoinQuerySettings;
	CSphQuery						m_tJoinQuery;
	std::unique_ptr<QueryParser_i>	m_pJoinQueryParser;
	const CSphIndex *				m_pIndex = nullptr;
	const CSphIndex *				m_pJoinedIndex = nullptr;
	const CSphQuery &				m_tQuery;
	VecTraits_T<const CSphQuery> 	m_dQueries;
	VecTraits_T<const CSphQuery> 	m_dJoinQueryOptions;
	FilterEval_c					m_tMixedFilter;
	CSphMatch						m_tMatch;
	std::unique_ptr<ISphMatchSorter> m_pSorter;
	std::unique_ptr<ISphMatchSorter> m_pOriginalSorter;
	std::unique_ptr<ISphMatchSorter> m_pRightSorter;
	std::unique_ptr<ISphSchema>		m_pRightSorterRsetSchema;
	const BYTE *					m_pBlobPool = nullptr;
	columnar::Columnar_i *			m_pColumnar = nullptr;
	const CSphColumnInfo *			m_pAttrNullBitmask = nullptr;
	CSphSwapVector<CSphMatch>		m_dMatches;
	CSphVector<JoinAttrNameRemap_t>	m_dAttrRemap;
	CSphVector<JoinAttrRemap_t>		m_dJoinRemap;
	bool							m_bNeedToSetupRemap = true;
	CSphVector<FilterRemap_t>		m_dFilterRemap;
	int								m_iDynamicSize = 0;
	bool							m_bFinalCalcOnly = false;
	const ISphSchema *				m_pSorterSchema = nullptr;
	CSphVector<ContextCalcItem_t>	m_dCalcPrefilter;
	CSphVector<ContextCalcItem_t>	m_dCalcPresort;
	CSphVector<ContextCalcItem_t>	m_dAggregates;
	IntVec_t						m_dCalcPrefilterPtrAttrs;
	IntVec_t						m_dCalcPresortPtrAttrs;
	bool							m_bSorterSchemaHasDataPtrs = false;

	MatchCache_c					m_tCache;

	std::unique_ptr<BYTE[]>			m_pNullMask;
	uint64_t						m_uNullMask = 0;

	bool							m_bErrorFlag = false;
	CSphString						m_sErrorMessage;

	int								m_iBatchSize = 0;
	int								m_iBatched = 0;
	CSphVector<SphAttr_t>			m_dJoinOnFilterValues;
	CSphVector<CSphString>			m_dJoinOnFilterStrings;
	CSphVector<SphAttr_t>			m_dBatchedFilterValues;
	CSphVector<CSphString>			m_dBatchedFilterStrings;
	CSphVector<CSphMatch *>			m_dMatchPtrs;
	IntVec_t						m_dRightMatchUseCount;
	OpenHashTableFastClear_T<uint64_t, IntVec_t> m_hRightMatches;

	IntVec_t						m_dIntFilters;
	IntVec_t						m_dStrFilters;

	struct BatchedMatches_t
	{
		CSphMatch				m_tMatch;
		uint64_t				m_uFilterHash = 0;
		const BYTE *			m_pBlobPool = nullptr; // each match potentially comes from a different chunk with different pools
		columnar::Columnar_i *	m_pColumnar = nullptr;
	};

	CSphFixedVector<BatchedMatches_t> m_dBatchedMatches;

	bool		SetupJoinQuery ( int iDynamicSize, CSphString & sError );
	bool		SetupJoinSorter ( CSphString & sError );
	void		SetupJoinAttrRemap();
	void		SetupDependentAttrCalc ( const IntVec_t & dJoinedAttrs );
	void		SetupSorterSchema();
	void		SetupNullMask();
	void		SetupAggregates();
	FORCE_INLINE uint64_t SetupJoinFilters ( const CSphMatch & tEntry );
	bool		SetupRightFilters ( CSphString & sError );
	bool		SetupOnFilters ( CSphString & sError );
	void		SetupRightStandaloneLocators();
	void		AddToAttrRemap ( const CSphString & sFrom, const CSphString & sTo );
	void		AddToJoinSelectList ( const CSphString & sExpr );
	void		AddToJoinSelectList ( const CSphString & sExpr, const CSphString & sAlias );
	void		AddToJoinSelectList ( const CSphString & sExpr, const CSphString & sAlias, const char * szRemapPrefix );
	void		AddToJoinSelectList ( const CSphString & sExpr, const CSphString & sAlias, int iSorterAttrId, bool bConvertJsonType=false );
	void		AddToJoinSelectListForced ( const CSphString & sJoinExpr, const CSphString & sJoinAlias );
	void		AddOnFilterToFilterTree ( int iFilterId );

	void		AddStarItemsToJoinSelectList();
	void		AddQueryItemsToJoinSelectList();
	void		AddGroupbyItemsToJoinSelectList();
	void		AddRemappedStringItemsToJoinSelectList();
	void		AddExpressionItemsToJoinSelectList();
	void		AddDocidToJoinSelectList();
	void		AddWeightToJoinSelectList();
	void		AddBatchedFilterItemsToJoinSelectList();

	void		SetupJoinSelectList();
	void		IncreaseJoinedMaxMatches ( int iTotalCount );

	void		RepackJsonFieldAsStr ( const CSphMatch & tSrcMatch, const CSphAttrLocator & tLocSrc, const CSphAttrLocator & tLocDst );
	void		ProduceCacheSizeWarning ( CSphString & sWarning );
	void		PopulateStoredFields();

	FORCE_INLINE void AddToBatch ( const CSphMatch & tEntry, uint64_t uFilterHash );
	FORCE_INLINE bool IsBatchFull() const;
	void		SetupJoinFiltersBatch();
	void		ClearBatch();
	uint64_t	CalcRightFilterHash ( const CSphMatch & tEntry );
	int			DistributeRightMatches();
	void		CleanupUnusedRightMatches();

	template <typename MATCHES> void CleanupRightMatches ( MATCHES & dMatches );
	template <typename PUSH> void PushBatch ( PUSH && fnPush );
	bool		RunJoinedQuery ( int & iTotalCount );
	bool		RunJoinedQueryAndAdjustMaxMatches();

	template <typename PUSH, typename MATCHES>
	FORCE_INLINE bool AddToCacheAndPush ( const CSphMatch & tEntry, uint64_t uJoinOnFilterHash, PUSH && fnPush, MATCHES & dMatches, const BYTE * pBlobPool, columnar::Columnar_i *	pColumnar, bool bAddToCache );
};


JoinSorter_c::JoinSorter_c ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const CSphQuery & tQuery, const CSphQuery & tJoinQueryOptions, ISphMatchSorter * pSorter, bool bJoinedGroupSort, int iBatchSize )
	: JoinSorter_c ( pIndex, pJoinedIndex, { &tQuery, 1 }, { &tJoinQueryOptions, 1 }, pSorter, bJoinedGroupSort, iBatchSize )
{}


JoinSorter_c::JoinSorter_c ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const VecTraits_T<const CSphQuery> & dQueries, const VecTraits_T<const CSphQuery> & dJoinQueryOptions, ISphMatchSorter * pSorter, bool bJoinedGroupSort, int iBatchSize )
	: m_tJoinQuerySettings ( dJoinQueryOptions[0] )
	, m_pIndex ( pIndex )
	, m_pJoinedIndex ( pJoinedIndex )
	, m_tQuery ( dQueries.First() )
	, m_dQueries ( dQueries )
	, m_dJoinQueryOptions ( dJoinQueryOptions )
	, m_pSorter ( pSorter )
	, m_tCache ( GetJoinCacheSize(), iBatchSize )
	, m_iBatchSize ( iBatchSize )
	, m_hRightMatches ( iBatchSize )
	, m_dBatchedMatches ( iBatchSize )
{
	assert ( pIndex && pJoinedIndex && pSorter );

	const ISphSchema & tSorterSchema = *m_pSorter->GetSchema();
	bool bHaveAggregates = false;
	for ( int i = 0; i < tSorterSchema.GetAttrsCount(); i++ )
		bHaveAggregates |= tSorterSchema.GetAttr(i).m_eAggrFunc!=SPH_AGGR_NONE;

	CSphVector<std::pair<int,bool>> dRightFilters = FetchJoinRightTableFilters ( m_tQuery.m_dFilters, tSorterSchema, m_tQuery.m_sJoinIdx.cstr() );
	bool bDisableByImplicitGrouping = HasImplicitGrouping(m_tQuery) && m_tQuery.m_eJoinType!=JoinType_e::LEFT;
	m_bFinalCalcOnly = !pIndex->IsRT() && !bJoinedGroupSort && !bHaveAggregates && !dRightFilters.GetLength() && !NeedToMoveMixedJoinFilters ( m_tQuery, tSorterSchema ) && !pSorter->IsPrecalc() && !bDisableByImplicitGrouping;
	m_bErrorFlag = !SetupJoinQuery ( m_pSorter->GetSchema()->GetDynamicSize(), m_sErrorMessage );
	if ( m_bFinalCalcOnly || !m_iBatchSize )
		m_bCanBatch = false;
}


void JoinSorter_c::SetBlobPool ( const BYTE * pBlobPool )
{
	m_pBlobPool = pBlobPool;
	m_pSorter->SetBlobPool(pBlobPool);
	m_tMixedFilter.SetBlobPool(pBlobPool);
}


void JoinSorter_c::SetColumnar ( columnar::Columnar_i * pColumnar )
{
	m_pColumnar = pColumnar;
	m_pSorter->SetColumnar(pColumnar);
	m_tMixedFilter.SetColumnar(pColumnar);
}


void JoinSorter_c::SetSchema ( ISphSchema * pSchema, bool bRemapCmp )
{
	m_pSorter->SetSchema ( pSchema, bRemapCmp );
	m_bErrorFlag = !SetupJoinQuery ( pSchema->GetDynamicSize(), m_sErrorMessage );
}


void JoinSorter_c::SetupSorterSchema()
{
	m_pSorterSchema = m_pSorter->GetSchema();
	assert ( m_pSorterSchema );
	m_pAttrNullBitmask = m_pSorterSchema->GetAttr ( GetNullMaskAttrName() );

	for ( int i = 0; i < m_pSorterSchema->GetAttrsCount(); i++ )
		m_bSorterSchemaHasDataPtrs |= m_pSorterSchema->GetAttr(i).IsDataPtr();
}


void JoinSorter_c::SetupNullMask()
{
	if ( !m_pAttrNullBitmask )
		return;

	if ( m_pAttrNullBitmask->m_eAttrType==SPH_ATTR_STRINGPTR )
	{
		int iNumJoinAttrs = 0;
		int iDynamic = 0;
		for ( int i = 0; i < m_pSorterSchema->GetAttrsCount(); i++ )
		{
			const auto & tAttr = m_pSorterSchema->GetAttr(i);
			if ( !tAttr.m_tLocator.m_bDynamic )
				continue;

			iDynamic++;
			if ( tAttr.IsJoined() )
				iNumJoinAttrs = Max ( iNumJoinAttrs, iDynamic );
		}

		BitVec_T<BYTE> tMask(iNumJoinAttrs);

		iDynamic = 0;
		for ( int i = 0; i < m_pSorterSchema->GetAttrsCount(); i++ )
		{
			const auto & tAttr = m_pSorterSchema->GetAttr(i);
			if ( !tAttr.m_tLocator.m_bDynamic )
				continue;

			if ( tAttr.IsJoined() )
				tMask.BitSet(iDynamic);

			iDynamic++;
		}

		m_pNullMask = std::unique_ptr<BYTE[]>( sphPackPtrAttr ( { tMask.Begin(), tMask.GetSizeBytes() } ) );
		m_uNullMask = (uint64_t)m_pNullMask.get();
		return;
	}

	// we keep null flags only for attributes with a dynamic locator
	// and these attributes need to be from the right table
	m_uNullMask = 0;
	int iDynamic = 0;
	for ( int i = 0; i < m_pSorterSchema->GetAttrsCount(); i++ )
	{
		const auto & tAttr = m_pSorterSchema->GetAttr(i);
		if ( !tAttr.m_tLocator.m_bDynamic )
			continue;

		if ( tAttr.IsJoined() )
			m_uNullMask |= 1ULL << iDynamic;

		iDynamic++;
	}
}


void JoinSorter_c::SetupAggregates()
{
	for ( int i = 0; i < m_pSorterSchema->GetAttrsCount(); i++ )
	{
		const auto & tAttr = m_pSorterSchema->GetAttr(i);
		if ( tAttr.m_eAggrFunc!=SPH_AGGR_NONE && tAttr.m_eStage==SPH_EVAL_SORTER && GetJoinAttrName ( tAttr.m_sName, CSphString ( m_pJoinedIndex->GetName() ), *m_pSorterSchema ) )
			m_dAggregates.Add ( { tAttr.m_tLocator, tAttr.m_eAttrType, tAttr.m_pExpr } );
	}
}


bool JoinSorter_c::SetupJoinQuery ( int iDynamicSize, CSphString & sError )
{
	m_pJoinQueryParser = std::unique_ptr<QueryParser_i>( m_tQuery.m_pQueryParser->Clone() );

	m_tJoinQuery = m_tJoinQuerySettings;
	m_tJoinQuery.m_pQueryParser = m_pJoinQueryParser.get();
	m_tJoinQuery.m_eQueryType	= m_tQuery.m_eQueryType;
	m_tJoinQuery.m_iLimit		= m_tJoinQuery.m_iMaxMatches;
	m_tJoinQuery.m_iCutoff		= 0;
	m_tJoinQuery.m_sQuery = m_tJoinQuery.m_sRawQuery = m_tQuery.m_sJoinQuery;

	m_tMatch.Reset ( iDynamicSize );
	SetupSorterSchema();
	SetupJoinSelectList();
	if ( !SetupRightFilters(sError) )	return false;
	if ( !SetupOnFilters(sError) )		return false;

	AddBatchedFilterItemsToJoinSelectList();
	if ( !SetupJoinSorter(sError) )		return false;

	SetupNullMask();
	SetupAggregates();
	m_iDynamicSize = iDynamicSize;

	return true;
}


bool JoinSorter_c::SetupJoinSorter ( CSphString & sError )
{
	SphQueueSettings_t tQueueSettings ( m_pJoinedIndex->GetMatchSchema() );
	tQueueSettings.m_bComputeItems = true;
	tQueueSettings.m_iMaxMatches = m_tJoinQuery.m_iMaxMatches;

	SphQueueRes_t tRes;
	m_pRightSorter = std::unique_ptr<ISphMatchSorter> ( sphCreateQueue ( tQueueSettings, m_tJoinQuery, sError, tRes ) );
	if ( !m_pRightSorter )
		return false;

	m_pRightSorterRsetSchema = std::unique_ptr<ISphSchema> ( m_pRightSorter->GetSchema()->CloneMe() );
	assert(m_pRightSorterRsetSchema);

	return true;
}


void JoinSorter_c::SetupDependentAttrCalc ( const IntVec_t & dJoinedAttrs )
{
	// find expressions that depend on joined attrs
	for ( int i = 0; i < m_pSorterSchema->GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_pSorterSchema->GetAttr(i);
		if ( !tAttr.m_pExpr )
			continue;

		StrVec_t dDeps;
		dDeps.Add ( tAttr.m_sName );
		FetchAttrDependencies ( dDeps, *m_pSorterSchema );

		bool bFound = false;
		for ( auto iJoinedAttr : dJoinedAttrs )
			for ( const auto & sDep : dDeps )
				bFound |= m_pSorterSchema->GetAttr(iJoinedAttr).m_sName==sDep;

		if ( !bFound )
			continue;

		switch ( m_pSorterSchema->GetAttr(i).m_eStage )
		{
		case SPH_EVAL_PREFILTER:
			m_dCalcPrefilter.Add ( { tAttr.m_tLocator, tAttr.m_eAttrType, tAttr.m_pExpr } );
			if ( sphIsDataPtrAttr ( tAttr.m_eAttrType ) )
				m_dCalcPrefilterPtrAttrs.Add ( m_dCalcPrefilter.GetLength()-1 );
			break;

		case SPH_EVAL_PRESORT:
			m_dCalcPresort.Add ( { tAttr.m_tLocator, tAttr.m_eAttrType, tAttr.m_pExpr } );
			if ( sphIsDataPtrAttr ( tAttr.m_eAttrType ) )
				m_dCalcPresortPtrAttrs.Add ( m_dCalcPresort.GetLength()-1 );
			break;

		default:
			break;
		}
	}
}


void JoinSorter_c::SetupRightStandaloneLocators()
{
	if ( !m_bCanBatch )
		return;

	const ISphSchema * pRightSchema = m_pRightSorter.get()->GetSchema();

	for ( auto & tRemap : m_dFilterRemap )
	{
		auto & tFilter = m_tJoinQuery.m_dFilters[tRemap.m_iFilterId];
		CSphString sBatchedAttrName;
		sBatchedAttrName.SetSprintf ( "%s%s", GetBatchedItemPrefix(), tFilter.m_sAttrName.cstr() );	
		const CSphColumnInfo * pRightJoinOnAttr = pRightSchema->GetAttr ( sBatchedAttrName.cstr() );
		assert ( pRightJoinOnAttr );
		tRemap.m_tRightStandaloneLocator = pRightJoinOnAttr->m_tLocator;
	}
}


void JoinSorter_c::SetupJoinAttrRemap()
{
	m_dJoinRemap.Resize(0);

	IntVec_t dJoinedAttrs;
	auto * pSorterSchema = m_pSorter->GetSchema();
	auto * pJoinSorterSchema = m_pRightSorter->GetSchema();
	for ( int i = 0; i < pJoinSorterSchema->GetAttrsCount(); i++ )
	{
		auto & tAttrSrc = pJoinSorterSchema->GetAttr(i);
		const JoinAttrNameRemap_t * pFound = nullptr;
		for ( const auto & tRemap : m_dAttrRemap )
			if ( tRemap.m_sFrom==tAttrSrc.m_sName )
			{
				pFound = &tRemap;
				break;
			}

		if ( !pFound )
			continue;

		for ( const auto & sDstAttr : pFound->m_dTo )
		{
			int iDstAttr = pSorterSchema->GetAttrIndex ( sDstAttr.cstr() );
			assert ( iDstAttr>=0 );
			dJoinedAttrs.Add(iDstAttr);
			const CSphColumnInfo & tAttrDst = pSorterSchema->GetAttr(iDstAttr);
			bool bJsonRepack = tAttrDst.m_sName.Begins ( GetInternalJsonPrefix() ) || tAttrDst.m_sName.Begins ( GetInternalAttrPrefix() );
			m_dJoinRemap.Add ( { tAttrSrc.m_tLocator, tAttrDst.m_tLocator, bJsonRepack } );
		}
	}

	SetupDependentAttrCalc(dJoinedAttrs);
	SetupRightStandaloneLocators();
	m_bNeedToSetupRemap = false;
}


FORCE_INLINE void SetExprBlobPool ( const CSphVector<ContextCalcItem_t> & dItems, const BYTE * pBlobPool )
{
	for ( const auto & i : dItems )
		i.m_pExpr->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
}

template <typename PUSH, typename MATCHES>
bool JoinSorter_c::PushJoinedMatches ( const CSphMatch & tEntry, const MATCHES & dMatches, PUSH && fnPush, const BYTE * pBlobPool, columnar::Columnar_i * pColumnar )
{
	SetExprBlobPool ( m_dCalcPrefilter, pBlobPool );
	SetExprBlobPool ( m_dCalcPresort, pBlobPool );
	SetExprBlobPool ( m_dAggregates, pBlobPool );

	bool bAnythingPushed = false;
	for ( auto & tMatchFromRset : dMatches )
	{
		memcpy ( m_tMatch.m_pDynamic, tEntry.m_pDynamic, m_iDynamicSize*sizeof(CSphRowitem) );
		for ( auto & i : m_dJoinRemap )
		{
			if ( i.m_bJsonRepack )
				RepackJsonFieldAsStr ( tMatchFromRset, i.m_tLocSrc, i.m_tLocDst );
			else
				m_tMatch.SetAttr ( i.m_tLocDst, tMatchFromRset.GetAttr ( i.m_tLocSrc ) );
		}

		CalcContextItems ( m_tMatch, m_dCalcPrefilter );
		if ( m_bCanBatch )
		{
			m_tMixedFilter.SetBlobPool(pBlobPool);
			m_tMixedFilter.SetColumnar(pColumnar);
		}

		if ( !m_tMixedFilter.Eval(m_tMatch) )
		{
			FreeDataPtrAttrs ( m_tMatch, m_dCalcPrefilter, m_dCalcPrefilterPtrAttrs );
			continue;
		}

		CalcContextItems ( m_tMatch, m_dCalcPresort );
		CalcContextItems ( m_tMatch, m_dAggregates );
		bAnythingPushed |= fnPush(m_tMatch);

		FreeDataPtrAttrs ( m_tMatch, m_dCalcPrefilter, m_dCalcPrefilterPtrAttrs );
		FreeDataPtrAttrs ( m_tMatch, m_dCalcPresort, m_dCalcPresortPtrAttrs );

		// clear repacked json
		for ( auto & i : m_dJoinRemap )
			if ( i.m_bJsonRepack )
			{
				auto pValue = (BYTE *)m_tMatch.GetAttr(i.m_tLocDst);
				SafeDeleteArray(pValue);
			}
	}

	return bAnythingPushed;
}

template <typename PUSH>
bool JoinSorter_c::PushLeftMatch ( const CSphMatch & tEntry, PUSH && fnPush, const BYTE * pBlobPool, columnar::Columnar_i *	pColumnar )
{
	// no matches with null values from right table if we have a MATCH() for the right table
	if ( !m_tQuery.m_sJoinQuery.IsEmpty() )
		return false;

	memcpy ( m_tMatch.m_pDynamic, tEntry.m_pDynamic, m_iDynamicSize*sizeof(CSphRowitem) );

	// set NULL bitmask
	assert(m_pAttrNullBitmask);
	m_tMatch.SetAttr ( m_pAttrNullBitmask->m_tLocator, m_uNullMask );

	if ( m_bCanBatch )
	{
		m_tMixedFilter.SetBlobPool(pBlobPool);
		m_tMixedFilter.SetColumnar(pColumnar);
	}

	if ( !m_tMixedFilter.Eval(m_tMatch) )
		return false;

	CalcContextItems ( m_tMatch, m_dAggregates );

	return fnPush(m_tMatch);
}


void JoinSorter_c::RepackJsonFieldAsStr ( const CSphMatch & tSrcMatch, const CSphAttrLocator & tLocSrc, const CSphAttrLocator & tLocDst )
{
	auto pValue = (BYTE *)m_tMatch.GetAttr(tLocDst);
	SafeDeleteArray(pValue);

	SphAttr_t tJsonFieldPtr = tSrcMatch.GetAttr(tLocSrc);
	if ( !tJsonFieldPtr )
		return;

	const BYTE * pVal = (BYTE *)tJsonFieldPtr;
	auto tBlob = sphUnpackPtrAttr(pVal);
	pVal = tBlob.first;
	ESphJsonType eJson = (ESphJsonType)*pVal++;
	CSphString sResult = FormatJsonAsSortStr ( pVal, eJson );
	int iStrLen = sResult.Length();
	BYTE * pData = nullptr;
	SphAttr_t uValue = (SphAttr_t) sphPackPtrAttr ( iStrLen+1, &pData );
	memcpy ( pData, sResult.cstr(), iStrLen+1 );

	m_tMatch.SetAttr ( tLocDst, uValue );
}


void JoinSorter_c::AddToBatch ( const CSphMatch & tEntry, uint64_t uFilterHash )
{
	if ( m_dJoinOnFilterValues.GetLength() )
	{
		SphAttr_t * pBatched = m_dBatchedFilterValues.AddN ( m_dJoinOnFilterValues.GetLength() );
		memcpy ( pBatched, m_dJoinOnFilterValues.Begin(), m_dJoinOnFilterValues.GetLengthBytes() );
	}

	if ( m_dJoinOnFilterStrings.GetLength() )
	{
		int iNumStrings = m_dJoinOnFilterStrings.GetLength();
		CSphString * pBatched = m_dBatchedFilterStrings.AddN(iNumStrings);
		for ( int i = 0; i < iNumStrings; i++ )
			pBatched[i] = m_dJoinOnFilterStrings[i];
	}

	auto & tBatch = m_dBatchedMatches[m_iBatched++];
	m_pSorterSchema->CloneMatch ( tBatch.m_tMatch, tEntry );
	tBatch.m_uFilterHash = uFilterHash;
	tBatch.m_pBlobPool = m_pBlobPool;
	tBatch.m_pColumnar = m_pColumnar;
}


bool JoinSorter_c::IsBatchFull() const
{
	return m_iBatched>=m_iBatchSize;
}


void JoinSorter_c::SetupJoinFiltersBatch()
{
	ARRAY_FOREACH ( i, m_dFilterRemap )
	{
		const auto & tRemap = m_dFilterRemap[i];
		if ( tRemap.m_bBlob )
		{
			auto & dStrings = m_tJoinQuery.m_dFilters[tRemap.m_iFilterId].m_dStrings;
			dStrings.Resize(0);
			dStrings.Reserve(m_iBatched);
		}
		else
		{
			auto & dValues = m_tJoinQuery.m_dFilters[tRemap.m_iFilterId].m_dValues;
			dValues.Resize(0);
			dValues.Reserve(m_iBatched);
		}
	}

	int iNumIntFilters = m_dIntFilters.GetLength();
	ARRAY_FOREACH ( i, m_dBatchedFilterValues )
	{
		int iFilter = i % iNumIntFilters;
		const auto & tRemap = m_dFilterRemap[m_dIntFilters[iFilter]];
		assert ( !tRemap.m_bBlob );
		m_tJoinQuery.m_dFilters[tRemap.m_iFilterId].m_dValues.Add ( m_dBatchedFilterValues[i] );
	}

	int iNumStrFilters = m_dStrFilters.GetLength();
	ARRAY_FOREACH ( i, m_dBatchedFilterStrings )
	{
		int iFilter = i % iNumStrFilters;
		const auto & tRemap = m_dFilterRemap[m_dStrFilters[iFilter]];
		assert ( tRemap.m_bBlob );
		m_tJoinQuery.m_dFilters[tRemap.m_iFilterId].m_dStrings.Add ( m_dBatchedFilterStrings[i] );
	}

	ARRAY_FOREACH ( i, m_dIntFilters )
	{
		int iIntFilterId = m_dIntFilters[i];
		m_tJoinQuery.m_dFilters[m_dFilterRemap[iIntFilterId].m_iFilterId].m_dValues.Uniq();
	}

	ARRAY_FOREACH ( i, m_dStrFilters )
	{
		int iStrFilterId = m_dStrFilters[i];
		m_tJoinQuery.m_dFilters[m_dFilterRemap[iStrFilterId].m_iFilterId].m_dStrings.Uniq();
	}
}


static int GetIndexWeight ( const CSphQuery & tQuery, const CSphString & sIdxName )
{
	for ( const auto & i : tQuery.m_dIndexWeights )
		if ( i.first==sIdxName )
			return i.second;

	return 1;
}


bool JoinSorter_c::RunJoinedQuery ( int & iTotalCount )
{
	CSphQueryResultMeta tMeta;
	CSphQueryResult tQueryResult;
	tQueryResult.m_pMeta = &tMeta;

	// restore non-standalone schema
	// FIXME!!!! make a SetSchema that does not take ownership of the schema
	m_pRightSorter->SetSchema ( m_pRightSorterRsetSchema->CloneMe(), true );

	CSphMultiQueryArgs tArgs ( GetIndexWeight ( m_tJoinQuery, m_tQuery.m_sJoinIdx ) );
	ISphMatchSorter * pSorter = m_pRightSorter.get();
	if ( !m_pJoinedIndex->MultiQuery ( tQueryResult, m_tJoinQuery, { &pSorter, 1 }, tArgs ) )
	{
		m_bErrorFlag = true;
		m_sErrorMessage.SetSprintf ( "joined table %s: %s", m_pJoinedIndex->GetName(), tMeta.m_sError.cstr() );
		return false;
	}

	m_dMatches.Resize(0);

	// setup join attr remap, but do it only once
	// we can't do that before because we need to remap from the standalone schema and we get it only after the first query
	if ( m_bNeedToSetupRemap )
		SetupJoinAttrRemap();

	iTotalCount = pSorter->GetTotalCount();

	if ( pSorter->GetLength() )
	{
		int iCopied = pSorter->Flatten ( m_dMatches.AddN ( pSorter->GetLength() ) );
		m_dMatches.Resize(iCopied);
	}

	return true;
}

template <typename MATCHES>
void JoinSorter_c::CleanupRightMatches ( MATCHES & dMatches )
{
	const ISphSchema * pTransformedRightSchema = m_pRightSorter->GetSchema();
	for ( auto & i : dMatches )
	{
		pTransformedRightSchema->FreeDataPtrs(i);
		i.ResetDynamic();
	}
}


bool JoinSorter_c::RunJoinedQueryAndAdjustMaxMatches()
{
	while ( true )
	{
		int iTotalCount = 0;
		if ( !RunJoinedQuery(iTotalCount) )
			return false;

		bool bNeedToIncrease = m_dMatches.GetLength()==m_tJoinQuery.m_iMaxMatches && iTotalCount>m_tJoinQuery.m_iMaxMatches;
		if ( !bNeedToIncrease )
			break;

		CleanupRightMatches(m_dMatches);
		IncreaseJoinedMaxMatches(iTotalCount);
		if ( !SetupJoinSorter(m_sErrorMessage) )
		{
			m_bErrorFlag = true;
			return false;
		}
	}

	return true;
}

template <typename PUSH, typename MATCHES>
bool JoinSorter_c::AddToCacheAndPush ( const CSphMatch & tEntry, uint64_t uJoinOnFilterHash, PUSH && fnPush, MATCHES & dMatches, const BYTE * pBlobPool, columnar::Columnar_i *	pColumnar, bool bAddToCache )
{
	ISphMatchSorter * pSorter = m_pRightSorter.get();

	bool bInCache = !bAddToCache;
	if ( bAddToCache )
	{
		m_tCache.SetSchema ( pSorter->GetSchema() );
		bInCache = m_tCache.Add ( uJoinOnFilterHash, dMatches );
	}

	CSphRowitem * pDynamic = m_tMatch.m_pDynamic;
	memcpy ( &m_tMatch, &tEntry, sizeof(m_tMatch) );
	m_tMatch.m_pDynamic = pDynamic;

	bool bAnythingPushed = PushJoinedMatches ( tEntry, dMatches, fnPush, pBlobPool, pColumnar );

	if ( !dMatches.GetLength() && m_tQuery.m_eJoinType==JoinType_e::LEFT )
		bAnythingPushed = PushLeftMatch ( tEntry, fnPush, pBlobPool, pColumnar );

	if ( bInCache )
	{
		for ( auto & i : dMatches )
			i.m_pDynamic = nullptr;
	}
	else
		CleanupRightMatches(dMatches);

	return bAnythingPushed;
}


void JoinSorter_c::ClearBatch()
{
	if ( m_bSorterSchemaHasDataPtrs )
	{
		for ( int i = 0; i < m_iBatched; i++ )
			m_pSorterSchema->FreeDataPtrs ( m_dBatchedMatches[i].m_tMatch );
	}

	m_iBatched = 0;
	m_dBatchedFilterValues.Resize(0);
	m_dBatchedFilterStrings.Resize(0);
}


int JoinSorter_c::DistributeRightMatches()
{
	m_dRightMatchUseCount.Resize ( m_dMatches.GetLength() );
	m_dRightMatchUseCount.ZeroVec();
	m_hRightMatches.Clear();

	ARRAY_FOREACH ( iRightMatch, m_dMatches )
	{
		uint64_t uRightHash = CalcRightFilterHash ( m_dMatches[iRightMatch] );
		IntVec_t * pFound = m_hRightMatches.Find(uRightHash);
		if ( pFound )
			pFound->Add(iRightMatch);
		else
		{
			IntVec_t & tFound = m_hRightMatches.Acquire(uRightHash);
			tFound.Resize(0);
			tFound.Add(iRightMatch);
		}
	}

	int iMaxRightMatches = 0;
	for ( int iMatch = 0; iMatch < m_iBatched; iMatch++ )
	{
		auto & tLeftMatch = m_dBatchedMatches[iMatch];

		IntVec_t * pRightMatchVec = m_hRightMatches.Find(tLeftMatch.m_uFilterHash);
		if ( pRightMatchVec )
		{
			for ( auto iRightMatch : *pRightMatchVec )
				m_dRightMatchUseCount[iRightMatch]++;
		}

		iMaxRightMatches = Max ( iMaxRightMatches, pRightMatchVec ? pRightMatchVec->GetLength() : 0 );
	}

	return iMaxRightMatches;
}


void JoinSorter_c::CleanupUnusedRightMatches()
{
	const ISphSchema * pRightSchema = m_pRightSorter->GetSchema();
	ARRAY_FOREACH ( i, m_dMatches )
		if ( !m_dRightMatchUseCount[i] )
		{
			pRightSchema->FreeDataPtrs ( m_dMatches[i] );
			m_dMatches[i].ResetDynamic();
		}
}

template <typename PUSH>
void JoinSorter_c::PushBatch ( PUSH && fnPush )
{
	int iMaxRightMatches = DistributeRightMatches();
	CleanupUnusedRightMatches();

	const ISphSchema * pRightSchema = m_pRightSorter->GetSchema();
	CSphFixedVector<CSphMatch> dTmpMatchStorage {iMaxRightMatches};
	for ( int iMatch = 0; iMatch < m_iBatched; iMatch++ )
	{
		auto & tLeftMatch = m_dBatchedMatches[iMatch];

		int iTmpMatch = 0;
		m_dMatchPtrs.Resize(0);

		IntVec_t * pRightMatchIds = m_hRightMatches.Find ( tLeftMatch.m_uFilterHash );
		if ( pRightMatchIds )
			for ( auto iRightMatchId : (*pRightMatchIds) )
			{
				int & iUseCount = m_dRightMatchUseCount[iRightMatchId];
				assert ( iUseCount>0 );

				CSphMatch & tRightMatch = m_dMatches[iRightMatchId];
				if ( iUseCount>1 )
				{
					// need to clone match to place it into cache
					CSphMatch & tTmpMatch = dTmpMatchStorage[iTmpMatch++];
					pRightSchema->CloneMatch ( tTmpMatch, tRightMatch );
					m_dMatchPtrs.Add ( &tTmpMatch );
				}
				else
					m_dMatchPtrs.Add ( &tRightMatch ); // no need to clone, cache just takes ownership

				iUseCount--;
			}

		MatchPtrVec_c dMatchesToPush(m_dMatchPtrs);
		AddToCacheAndPush ( tLeftMatch.m_tMatch, tLeftMatch.m_uFilterHash, fnPush, dMatchesToPush, tLeftMatch.m_pBlobPool, tLeftMatch.m_pColumnar, true );
	}

#ifndef NDEBUG
	for ( auto & i : m_dMatches )
		assert ( !i.m_pDynamic );
#endif // !NDEBUG
}

template <typename PUSH>
bool JoinSorter_c::RunBatch ( PUSH && fnPush )
{
	if ( !m_iBatched )
		return true;

	SetupJoinFiltersBatch();
	if ( !RunJoinedQueryAndAdjustMaxMatches() )
		return false;

	PushBatch(fnPush);
	ClearBatch();

	return true;
}

template <typename PUSH>
bool JoinSorter_c::Push_T ( const CSphMatch & tEntry, PUSH && fnPush, bool bGrouped )
{
	if ( m_bFinalCalcOnly )
		return fnPush(tEntry);

	if ( m_bErrorFlag )
		return false;

	uint64_t uJoinOnFilterHash = SetupJoinFilters(tEntry);
	if ( m_tCache.Fetch ( uJoinOnFilterHash, m_dMatches ) )
		return AddToCacheAndPush ( tEntry, uJoinOnFilterHash, fnPush, m_dMatches, m_pBlobPool, m_pColumnar, false );

	if ( m_bCanBatch && !bGrouped )
	{
		AddToBatch ( tEntry, uJoinOnFilterHash );
		if ( IsBatchFull() )
			RunBatch(fnPush);

		return false; // always return false so that cutoff/implicit cutoff won't work
	}

	if ( !RunJoinedQueryAndAdjustMaxMatches() )
		return false;

	return AddToCacheAndPush ( tEntry, uJoinOnFilterHash, fnPush, m_dMatches, m_pBlobPool, m_pColumnar, true );
}


void JoinSorter_c::Push ( const VecTraits_T<const CSphMatch> & dMatches )
{
	for ( auto & i : dMatches )
		Push(i);
}


int64_t	JoinSorter_c::GetTotalCount() const
{
	assert ( !m_bCanBatch || !m_iBatched );
	return m_pSorter->GetTotalCount();
}


void JoinSorter_c::ProduceCacheSizeWarning ( CSphString & sWarning )
{
	if ( m_tCache.IsFull() )
		sWarning.SetSprintf ( "Join cache overflow detected; increase join_cache_size to improve performance" );
}


void JoinSorter_c::PopulateStoredFields()
{
	StoredFetch_c tCalc ( *m_pSorterSchema, *m_pJoinedIndex, m_uNullMask );
	if ( tCalc.HasFieldToFetch() )
		m_pSorter->Finalize ( tCalc, false, false );
}


ISphMatchSorter * JoinSorter_c::Clone() const
{
	ISphMatchSorter * pSourceSorter = m_pOriginalSorter ? m_pOriginalSorter.get() : m_pSorter.get();
	return new JoinSorter_c ( m_pIndex, m_pJoinedIndex, m_dQueries, m_dJoinQueryOptions, pSourceSorter->Clone(), !m_bFinalCalcOnly, m_iBatchSize );
}


bool JoinSorter_c::RunFinalBatch()
{
	if ( !m_bCanBatch )
		return true;

	return RunBatch ( [this]( const CSphMatch & tMatch ) { return m_pSorter->Push(tMatch); } );
}


bool JoinSorter_c::FinalizeJoin ( CSphString & sError, CSphString & sWarning )
{
	if ( !RunFinalBatch() )
		return false;

	if ( !m_bFinalCalcOnly )
	{
		PopulateStoredFields();
		ProduceCacheSizeWarning(sWarning);
		if ( m_bErrorFlag )
		{
			sError = m_sErrorMessage;
			return false;
		}

		return true;
	}

	// keep the original underlying sorter in case other threads want to clone it
	assert ( !m_pOriginalSorter );
	m_pOriginalSorter = std::move(m_pSorter);

	// replace underlying sorter with a new one
	// and fill it with matches that we already have
	m_pSorter = std::unique_ptr<ISphMatchSorter> ( m_pOriginalSorter->Clone() );
	SetupSorterSchema();

	m_bFinalCalcOnly = false;
	if ( m_pOriginalSorter->IsGroupby() )
	{
		MatchCalcGrouped_c tCalc(this);
		m_pOriginalSorter->Finalize ( tCalc, false, false );
	}
	else
	{
		MatchCalc_c tCalc(this);
		m_pOriginalSorter->Finalize ( tCalc, false, false );
	}

	PopulateStoredFields();
	ProduceCacheSizeWarning(sWarning);

	if ( m_bErrorFlag )
	{
		sError = m_sErrorMessage;
		return false;
	}

	return true;
}


static void RemoveTableNamePrefix ( CSphString & sAttr, const CSphFilterSettings & tFilter, const CSphString & sPrefix )
{
	int iPrefixLen = sPrefix.Length();
	sAttr = tFilter.m_sAttrName.SubString ( iPrefixLen, tFilter.m_sAttrName.Length() - iPrefixLen );
}


bool JoinSorter_c::SetupRightFilters ( CSphString & sError )
{
	m_tJoinQuery.m_dFilters.Resize(0);

	CSphVector<std::pair<int,bool>> dRightFilters = FetchJoinRightTableFilters ( m_tQuery.m_dFilters, *m_pSorterSchema, m_pJoinedIndex->GetName() );
	bool bLeftJoin = m_tQuery.m_eJoinType==JoinType_e::LEFT;
	if ( bLeftJoin || m_tQuery.m_dFilterTree.GetLength() )
	{
		if ( !dRightFilters.GetLength() )
			return true;

		if ( bLeftJoin || dRightFilters.GetLength()!=m_tQuery.m_dFilters.GetLength() )
		{
			CreateFilterContext_t tCtx;
			tCtx.m_pFilters		= &m_tQuery.m_dFilters;
			tCtx.m_pFilterTree	= &m_tQuery.m_dFilterTree;
			tCtx.m_pMatchSchema	= m_pSorterSchema;
			tCtx.m_pIndexSchema	= &m_pIndex->GetMatchSchema();
			tCtx.m_bScan		= m_tQuery.m_sQuery.IsEmpty();
			tCtx.m_sJoinIdx		= m_pJoinedIndex->GetName();
			if ( !sphCreateFilters ( tCtx, sError, sError ) )
			{
				sError.SetSprintf ( "failed to create query filters: %s", sError.cstr() );
				return false;
			}

			m_tMixedFilter.SetFilter ( tCtx.m_pFilter );
			return true;
		}

		m_tJoinQuery.m_dFilterTree = m_tQuery.m_dFilterTree;
	}

	CSphString sPrefix;
	sPrefix.SetSprintf ( "%s.", m_pJoinedIndex->GetName() );

	ARRAY_FOREACH ( i, dRightFilters )
	{
		const auto & tFilter = m_tQuery.m_dFilters[dRightFilters[i].first];
		if ( tFilter.m_eType==SPH_FILTER_NULL )
			continue;

		m_tJoinQuery.m_dFilters.Add(tFilter);
		if ( dRightFilters[i].second )
			RemoveTableNamePrefix ( m_tJoinQuery.m_dFilters.Last().m_sAttrName, tFilter, sPrefix );
	}

	return true;
}


void JoinSorter_c::AddOnFilterToFilterTree ( int iFilterId )
{
	if ( !m_tJoinQuery.m_dFilterTree.GetLength() )
		return;

	int iRootNodeId = m_tJoinQuery.m_dFilterTree.GetLength()-1;
	FilterTreeItem_t & tFilter = m_tJoinQuery.m_dFilterTree.Add();
	tFilter.m_iFilterItem = iFilterId;

	int iFilterNodeId = m_tJoinQuery.m_dFilterTree.GetLength()-1;
	FilterTreeItem_t & tAnd = m_tJoinQuery.m_dFilterTree.Add();
	tAnd.m_iLeft = iRootNodeId;
	tAnd.m_iRight = iFilterNodeId;
}


bool JoinSorter_c::SetupOnFilters ( CSphString & sError )
{
	for ( auto & tOnFilter : m_tQuery.m_dOnFilters )
	{
		CSphFilterSettings & tFilter = m_tJoinQuery.m_dFilters.Add();
		tFilter.m_dValues.Resize(0);
		tFilter.m_dStrings.Resize(0);

		CSphString sAttrIdx1 = tOnFilter.m_sAttr1;
		CSphString sAttrIdx2 = tOnFilter.m_sAttr2;
		CSphString sIdx1 = tOnFilter.m_sIdx1;
		CSphString sIdx2 = tOnFilter.m_sIdx2;

		if ( tOnFilter.m_sIdx1==m_pJoinedIndex->GetName() )
		{
			assert ( tOnFilter.m_sIdx2==m_pIndex->GetName() );
			Swap ( sAttrIdx1, sAttrIdx2 );
			Swap ( sIdx1, sIdx2 );
		}

		// FIXME! handle compound names for left table (e.g. 'table1.id')
		const CSphColumnInfo * pAttr1 = m_pSorter->GetSchema()->GetAttr ( sAttrIdx1.cstr() );
		assert(pAttr1);

		// maybe it is a stored field?
		if ( pAttr1->m_eAttrType==SPH_ATTR_STRINGPTR && pAttr1->m_eStage==SPH_EVAL_POSTLIMIT )
		{
			sError.SetSprintf ( "Unable to perform join on a stored field '%s.%s'", sIdx1.cstr(), pAttr1->m_sName.cstr() );
			return false;
		}

		const CSphColumnInfo * pAttr2 = m_pJoinedIndex->GetMatchSchema().GetAttr ( sAttrIdx2.cstr() );
		if ( pAttr2 && pAttr2->m_eAttrType==SPH_ATTR_STRINGPTR && pAttr2->m_eStage==SPH_EVAL_POSTLIMIT )
		{
			sError.SetSprintf ( "Unable to perform join on a stored field '%s.%s'", sIdx2.cstr(), pAttr2->m_sName.cstr() );
			return false;
		}

		if ( !pAttr2 && !sphJsonNameSplit ( sAttrIdx2.cstr() ) )
		{
			sError.SetSprintf ( "joined table %s: unknown column: %s", sIdx2.cstr(), sAttrIdx2.cstr() );
			return false;
		}

		bool bStringFilter = pAttr1->m_eAttrType==SPH_ATTR_STRING;

		tFilter.m_sAttrName = sAttrIdx2;
		tFilter.m_eType		= bStringFilter ? SPH_FILTER_STRING : SPH_FILTER_VALUES;

		int iFilterId = m_tJoinQuery.m_dFilters.GetLength()-1;
		m_dFilterRemap.Add ( { iFilterId, pAttr1->m_tLocator, {}, bStringFilter } );

		if ( bStringFilter )
		{
			tFilter.m_dStrings.Resize(1);
			m_dStrFilters.Add ( m_dFilterRemap.GetLength()-1 );
		}
		else
		{
			tFilter.m_dValues.Resize(1);
			m_dIntFilters.Add ( m_dFilterRemap.GetLength()-1 );
		}

		AddOnFilterToFilterTree(iFilterId);
	}

	m_dJoinOnFilterValues.Resize ( m_dIntFilters.GetLength() );
	m_dJoinOnFilterStrings.Resize ( m_dStrFilters.GetLength() );

	return true;
}


uint64_t JoinSorter_c::CalcRightFilterHash ( const CSphMatch & tEntry )
{
	uint64_t uHash = 0;
	for ( auto & tRemap : m_dFilterRemap )
	{
		if ( tRemap.m_bBlob )
		{
			ByteBlob_t tBlob = tEntry.FetchAttrData ( tRemap.m_tRightStandaloneLocator, nullptr );
			uHash = HashWithSeed ( tBlob.first, tBlob.second, uHash );
		}
		else
		{
			SphAttr_t tValue = tEntry.GetAttr ( tRemap.m_tRightStandaloneLocator );
			uHash = HashWithSeed ( &tValue, sizeof(tValue), uHash );
		}
	}

	return uHash;
}


uint64_t JoinSorter_c::SetupJoinFilters ( const CSphMatch & tEntry )
{
	uint64_t uHash = 0;
	int iIntFilter = 0;
	int iStrFilter = 0;
	ARRAY_FOREACH ( i, m_dFilterRemap )
	{
		const auto & tRemap = m_dFilterRemap[i];
		auto & tFilter = m_tJoinQuery.m_dFilters[tRemap.m_iFilterId];
		if ( tRemap.m_bBlob )
		{
			ByteBlob_t tBlob = tEntry.FetchAttrData ( tRemap.m_tLocator, m_pBlobPool );
			tFilter.m_dStrings[0] = CSphString ( (const char*)tBlob.first, tBlob.second );
			if ( m_bCanBatch )
				m_dJoinOnFilterStrings[iStrFilter++] = tFilter.m_dStrings[0];

			uHash = HashWithSeed ( tBlob.first, tBlob.second, uHash );
		}
		else
		{
			SphAttr_t tValue = tEntry.GetAttr ( tRemap.m_tLocator );
			tFilter.m_dValues[0] = tValue;
			if ( m_bCanBatch )
				m_dJoinOnFilterValues[iIntFilter++] = tValue;

			uHash = HashWithSeed ( &tValue, sizeof(tValue), uHash );
		}
	}

	return uHash;
}


void JoinSorter_c::AddToAttrRemap ( const CSphString & sFrom, const CSphString & sTo )
{
	for ( auto & i : m_dAttrRemap )
		if ( i.m_sFrom==sFrom )
		{
			for ( const auto & j : i.m_dTo )
				if ( j==sTo )
					return;

			i.m_dTo.Add(sTo);
			return;
		}

	JoinAttrNameRemap_t & tNew = m_dAttrRemap.Add();
	tNew.m_sFrom = sFrom;
	tNew.m_dTo.Add(sTo);
}


static CSphString AddJsonTypeConversion ( const CSphString & sExpr, ESphAttr eAttr )
{
	CSphString sRes;
	switch ( eAttr )
	{
	case SPH_ATTR_STRING:
	case SPH_ATTR_STRINGPTR:
		sRes.SetSprintf ( "to_string(%s)", sExpr.cstr() );
		break;

	case SPH_ATTR_FLOAT:
		sRes.SetSprintf ( "double(%s)", sExpr.cstr() );
		break;

	default:
		sRes.SetSprintf ( "bigint(%s)", sExpr.cstr() );
		break;
	}

	return sRes;
}


void JoinSorter_c::AddToJoinSelectListForced ( const CSphString & sJoinExpr, const CSphString & sJoinAlias )
{
	// don't add duplicates to select list items
	for ( const auto & i : m_tJoinQuery.m_dItems )
		if ( i.m_sExpr==sJoinExpr && i.m_sAlias==sJoinAlias )
			return;

	auto & tItem = m_tJoinQuery.m_dItems.Add();
	tItem.m_sExpr = sJoinExpr;
	tItem.m_sAlias = sJoinAlias;
}


void JoinSorter_c::AddToJoinSelectList ( const CSphString & sExpr, const CSphString & sAlias, int iSorterAttrId, bool bConvertJsonType )
{
	if ( iSorterAttrId==-1 )
		return;

	if ( sExpr=="*" || sAlias=="*" )
		return;

	CSphString sJoinExpr;
	if ( !GetJoinAttrName ( sExpr, CSphString ( m_pJoinedIndex->GetName() ), *m_pSorterSchema, &sJoinExpr ) )
		return;

	const CSphColumnInfo & tSorterAttr = m_pSorterSchema->GetAttr(iSorterAttrId);
	if ( bConvertJsonType )
		sJoinExpr = AddJsonTypeConversion ( sJoinExpr, tSorterAttr.m_eAttrType );

	CSphString sJoinAlias = sExpr==sAlias ? sJoinExpr : sAlias;
	AddToAttrRemap ( sJoinAlias, tSorterAttr.m_sName );
	AddToJoinSelectListForced ( sJoinExpr, sJoinAlias );
}


void JoinSorter_c::AddToJoinSelectList ( const CSphString & sExpr, const CSphString & sAlias )
{
	int iSorterAttrId = m_pSorterSchema->GetAttrIndex ( sExpr.cstr() );
	if ( iSorterAttrId==-1 )
		iSorterAttrId = m_pSorterSchema->GetAttrIndex ( sAlias.cstr() );

	AddToJoinSelectList ( sExpr, sAlias, iSorterAttrId );
}


void JoinSorter_c::AddToJoinSelectList ( const CSphString & sExpr )
{
	AddToJoinSelectList ( sExpr, sExpr, m_pSorterSchema->GetAttrIndex ( sExpr.cstr() ), true );
}


void JoinSorter_c::AddToJoinSelectList ( const CSphString & sExpr, const CSphString & sAlias, const char * szRemapPrefix )
{
	// maybe it's a JSON attr?
	if ( !sphJsonNameSplit ( sExpr.cstr(), m_pJoinedIndex->GetName() ) )
		return;

	// try remapped groupby json attr
	CSphString sRemapped;
	sRemapped.SetSprintf ( "%s%s", szRemapPrefix, sExpr.cstr() );
	AddToJoinSelectList ( sExpr, sAlias, m_pSorterSchema->GetAttrIndex ( sRemapped.cstr() ) );
}


void JoinSorter_c::AddStarItemsToJoinSelectList()
{
	const CSphSchema & tJoinedSchema = m_pJoinedIndex->GetMatchSchema();
	bool bHaveStar = m_tQuery.m_dItems.any_of ( []( const CSphQueryItem & tItem ) { return tItem.m_sExpr=="*" || tItem.m_sAlias=="*"; } );
	if ( !bHaveStar )
		return;

	for ( int i = 0; i < tJoinedSchema.GetAttrsCount(); i++ )
	{
		auto & tAttr = tJoinedSchema.GetAttr(i);
		if ( sphIsInternalAttr(tAttr) )
			continue;

		CSphString sAttrName;
		sAttrName.SetSprintf ( "%s.%s", m_pJoinedIndex->GetName(), tAttr.m_sName.cstr() );
		AddToJoinSelectList ( sAttrName, sAttrName );
	}
}


void JoinSorter_c::AddQueryItemsToJoinSelectList()
{
	for ( const auto & i : m_tQuery.m_dItems )
		AddToJoinSelectList ( i.m_sExpr, i.m_sAlias );
}


void JoinSorter_c::AddGroupbyItemsToJoinSelectList()
{
	for ( const auto & tQuery : m_dQueries )
	{
		if ( !tQuery.m_sGroupBy.IsEmpty() )
		{
			StrVec_t dGroupby = ParseGroupBy ( tQuery.m_sGroupBy );
			for ( const auto & i : dGroupby )
			{
				AddToJoinSelectList ( i, i );
				AddToJoinSelectList ( i, i, GetInternalJsonPrefix() );	// try to add as json attr
			}
		}

		if ( !tQuery.m_sGroupDistinct.IsEmpty() )
			AddToJoinSelectList ( tQuery.m_sGroupDistinct, tQuery.m_sGroupDistinct );
	}
}


void JoinSorter_c::AddRemappedStringItemsToJoinSelectList()
{
	auto * pSorterSchema = m_pSorter->GetSchema();
	assert(pSorterSchema);
	for ( int i = 0; i < pSorterSchema->GetAttrsCount(); i++ )
	{
		auto & tAttr = pSorterSchema->GetAttr(i);
		if ( sphIsInternalAttr(tAttr) )
			continue;

		CSphString sName = tAttr.m_sName;
		if ( IsSortStringInternal ( sName.cstr() ) )
		{
			int iPrefixLen = strlen ( GetInternalAttrPrefix() );
			CSphString sJoinedAttrName = sName.cstr()+iPrefixLen;
			AddToJoinSelectList ( sJoinedAttrName, sJoinedAttrName, GetInternalAttrPrefix() );
		}

		if ( IsSortJsonInternal ( sName.cstr() ) )
		{
			int iPrefixLen = strlen ( GetInternalJsonPrefix() );
			CSphString sJoinedAttrName = sName.cstr()+iPrefixLen;
			AddToJoinSelectList ( sJoinedAttrName, sJoinedAttrName, GetInternalJsonPrefix() );
		}
	}
}


void JoinSorter_c::AddExpressionItemsToJoinSelectList()
{
	// find JSON/columnar attrs present in filters and add them to select list (only when all filters are moved to the left query)
	if ( !NeedToMoveMixedJoinFilters ( m_tQuery, *m_pSorterSchema ) )
		return;

	const CSphSchema & tJoinedSchema = m_pJoinedIndex->GetMatchSchema();
	for ( const auto & i : m_tQuery.m_dFilters )
	{
		if ( sphJsonNameSplit ( i.m_sAttrName.cstr(), m_pJoinedIndex->GetName() ) )
		{
			AddToJoinSelectList ( i.m_sAttrName );
			continue;
		}

		CSphString sJoinedAttr;
		if ( GetJoinAttrName ( i.m_sAttrName, CSphString ( m_pJoinedIndex->GetName() ), *m_pSorterSchema, &sJoinedAttr ) )
		{
			const CSphColumnInfo * pAttr = tJoinedSchema.GetAttr ( sJoinedAttr.cstr() );
			if ( pAttr && pAttr->IsColumnar() )
				AddToJoinSelectList ( i.m_sAttrName, i.m_sAttrName );
		}		
	}
}


void JoinSorter_c::AddDocidToJoinSelectList()
{
	// fetch docid; we need it for docstore queries
	CSphString sId;
	sId.SetSprintf ( "%s.%s", m_pJoinedIndex->GetName(), sphGetDocidName() );
	AddToJoinSelectList ( sId, sId );
}


void JoinSorter_c::AddWeightToJoinSelectList()
{
	if ( m_tJoinQuery.m_sQuery.IsEmpty() )
		return;

	CSphString sId;
	sId.SetSprintf ( "%s.weight()", m_pJoinedIndex->GetName());
	AddToJoinSelectList ( sId, sId );
}


void JoinSorter_c::AddBatchedFilterItemsToJoinSelectList()
{
	if ( !m_bCanBatch )
		return;

	for ( auto & i : m_tJoinQuery.m_dFilters )
	{
		CSphString sAttr;
		sAttr.SetSprintf ( "%s.%s", m_pJoinedIndex->GetName(), i.m_sAttrName.cstr() );

		CSphString sJoinExpr;
		if ( !GetJoinAttrName ( sAttr, CSphString ( m_pJoinedIndex->GetName() ), *m_pSorterSchema, &sJoinExpr ) )
			continue;

		CSphString sJoinAlias;
		sJoinAlias.SetSprintf ( "%s%s", GetBatchedItemPrefix(), sJoinExpr.cstr() );
		if ( sphJsonNameSplit ( sJoinExpr.cstr() ) )
			sJoinExpr = AddJsonTypeConversion ( sJoinExpr, i.m_eType==SPH_FILTER_STRING ? SPH_ATTR_STRINGPTR : SPH_ATTR_BIGINT );

		AddToJoinSelectListForced ( sJoinExpr, sJoinAlias );
	}
}


void JoinSorter_c::SetupJoinSelectList()
{
	m_tJoinQuery.m_dItems.Resize(0);
	m_dAttrRemap.Reset();

	AddStarItemsToJoinSelectList();
	AddQueryItemsToJoinSelectList();
	AddGroupbyItemsToJoinSelectList();
	AddRemappedStringItemsToJoinSelectList();
	AddExpressionItemsToJoinSelectList();
	AddDocidToJoinSelectList();
	AddWeightToJoinSelectList();
}


void JoinSorter_c::IncreaseJoinedMaxMatches ( int iTotalCount )
{
	int64_t iNewLimit = sph::DefaultRelimit::Relimit ( m_tJoinQuery.m_iMaxMatches, iTotalCount );
	if ( iNewLimit > INT_MAX )
		return;

	m_tJoinQuery.m_iMaxMatches = (int)iNewLimit;
	m_tJoinQuery.m_iLimit = m_tJoinQuery.m_iMaxMatches;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class JoinMultiSorter_c : public JoinSorter_c
{
public:
			JoinMultiSorter_c ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const VecTraits_T<CSphQuery> & dQueries, const VecTraits_T<CSphQuery> & dJoinQueryOptions, VecTraits_T<ISphMatchSorter *> dSorters, int iBatchSize );

	bool	Push ( const CSphMatch & tEntry ) override;
	bool	PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override;

	// cloning multi-sorter is hard with the current architecture
	// as it holds pointers to sorters that also exist in dSorters array in matching/fullscan
	// for cloning to work we would need to clone the sorters that we hold and also somehow sync them with dSorters
	bool	CanBeCloned() const override { return false; }

protected:
	bool	RunFinalBatch() override;

private:
	CSphVector<ISphMatchSorter *> m_dSorters;	// we don't own 1..N sorters (JoinSorter_c owns sorter #0)
};


JoinMultiSorter_c::JoinMultiSorter_c ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const VecTraits_T<CSphQuery> & dQueries, const VecTraits_T<CSphQuery> & dJoinQueryOptions, VecTraits_T<ISphMatchSorter *> dSorters, int iBatchSize )
	: JoinSorter_c ( pIndex, pJoinedIndex, dQueries, dJoinQueryOptions, dSorters[0], true, iBatchSize )
{
	m_dSorters.Resize ( dSorters.GetLength() );
	memcpy ( m_dSorters.Begin(), dSorters.Begin(), dSorters.GetLengthBytes() );
}


bool JoinMultiSorter_c::Push ( const CSphMatch & tEntry )
{
	return Push_T ( tEntry, [this]( const CSphMatch & tMatch )
		{
			bool bNew = false;
			for ( auto & i : m_dSorters )
				bNew |= i->Push(tMatch);

			return bNew;
		},
		false
	);
}


bool JoinMultiSorter_c::PushGrouped ( const CSphMatch & tEntry, bool bNewSet )
{
	return Push_T ( tEntry, [this,bNewSet]( const CSphMatch & tMatch )
		{
			bool bNew = false;
			for ( auto & i : m_dSorters )
				bNew |= i->PushGrouped ( tMatch, bNewSet );

			return bNew;
		},
		true
	);
}


bool JoinMultiSorter_c::RunFinalBatch()
{
	if ( !m_bCanBatch )
		return true;

	return RunBatch ( [this]( const CSphMatch & tMatch )
		{
			bool bNew = false;
			for ( auto & i : m_dSorters )
				bNew |= i->Push ( tMatch );

			return bNew;
		} );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class SorterWrapperNoPush_c : public ISphMatchSorter
{
public:
				SorterWrapperNoPush_c ( ISphMatchSorter * pSorter ) : m_pSorter ( pSorter ) {}

	bool		IsGroupby() const override											{ return m_pSorter->IsGroupby(); }
	void		SetState ( const CSphMatchComparatorState & tState ) override		{ m_pSorter->SetState(tState); }
	const		CSphMatchComparatorState & GetState() const override				{ return m_pSorter->GetState(); }
	void		SetGroupState ( const CSphMatchComparatorState & tState ) override	{ m_pSorter->SetGroupState(tState); }
	void		SetBlobPool ( const BYTE * pBlobPool ) override						{ m_pSorter->SetBlobPool(pBlobPool); }
	void		SetColumnar ( columnar::Columnar_i * pColumnar ) override			{ m_pSorter->SetColumnar(pColumnar); }
	void		SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) override			{ m_pSorter->SetSchema(pSchema, bRemapCmp); }
	const ISphSchema *	GetSchema() const override									{ return m_pSorter->GetSchema(); }
	bool		Push ( const CSphMatch & tEntry ) override							{ return false; }
	void		Push ( const VecTraits_T<const CSphMatch> & dMatches ) override		{}
	bool		PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override		{ return false; }
	int			GetLength() override												{ return m_pSorter->GetLength(); }
	int64_t		GetTotalCount() const override										{ return m_pSorter->GetTotalCount(); }
	void		Finalize ( MatchProcessor_i & tProcessor, bool bCallProcessInResultSetOrder, bool bFinalizeMatches ) override { m_pSorter->Finalize ( tProcessor, bCallProcessInResultSetOrder, bFinalizeMatches ); }
	int			Flatten ( CSphMatch * pTo ) override								{ return m_pSorter->Flatten(pTo); }
	const CSphMatch * GetWorst() const override										{ return m_pSorter->GetWorst(); }
	bool		CanBeCloned() const override										{ return m_pSorter->CanBeCloned(); }
	ISphMatchSorter * Clone() const override										{ return new SorterWrapperNoPush_c ( m_pSorter->Clone() ); }
	void		MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) override			{ m_pSorter->MoveTo ( ((SorterWrapperNoPush_c *)pRhs)->m_pSorter.get(), bCopyMeta ); }
	void		CloneTo ( ISphMatchSorter * pTrg ) const override					{ m_pSorter->CloneTo(pTrg); }
	void		SetFilteredAttrs ( const sph::StringSet & hAttrs, bool bAddDocid ) override	{ m_pSorter->SetFilteredAttrs(hAttrs, bAddDocid); }
	void		TransformPooled2StandalonePtrs ( GetBlobPoolFromMatch_fn fnBlobPoolFromMatch, GetColumnarFromMatch_fn fnGetColumnarFromMatch, bool bFinalizeSorters ) override { m_pSorter->TransformPooled2StandalonePtrs(fnBlobPoolFromMatch, fnGetColumnarFromMatch, bFinalizeSorters); }
	void		SetRandom ( bool bRandom ) override									{ m_pSorter->SetRandom(bRandom); }
	bool		IsRandom() const override											{ return m_pSorter->IsRandom(); }
	int			GetMatchCapacity() const override									{ return m_pSorter->GetMatchCapacity(); }
	RowTagged_t	GetJustPushed() const override										{ return m_pSorter->GetJustPushed(); }
	VecTraits_T<RowTagged_t> GetJustPopped() const override							{ return m_pSorter->GetJustPopped(); }
	bool		IsCutoffDisabled() const override									{ return m_pSorter->IsCutoffDisabled(); }
	void		SetMerge ( bool bMerge ) override									{ m_pSorter->SetMerge(bMerge); }
	bool		IsPrecalc() const override											{ return m_pSorter->IsPrecalc(); }

private:
	std::unique_ptr<ISphMatchSorter> m_pSorter;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CheckJoinOnFilters ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const CSphQuery & tQuery, CSphString & sError )
{
	if ( !tQuery.m_dOnFilters.GetLength() )
	{
		sError.SetSprintf ( "JOIN ON condition is empty" );
		return false;
	}

	for ( const auto & i : tQuery.m_dOnFilters )
	{
		if ( i.m_sIdx1!=pIndex->GetName() && i.m_sIdx1!=pJoinedIndex->GetName() )
		{
			sError.SetSprintf ( "JOIN ON table '%s' not found", i.m_sIdx1.cstr() );
			return false;
		}

		if ( i.m_sIdx2!=pIndex->GetName() && i.m_sIdx2!=pJoinedIndex->GetName() )
		{
			sError.SetSprintf ( "JOIN ON table '%s' not found", i.m_sIdx2.cstr() );
			return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class JoinNullFilter_c : public ISphFilter
{
public:
			JoinNullFilter_c ( bool bIsNull, const CSphAttrLocator & tNullMapLocator );

	bool	Eval ( const CSphMatch & tMatch ) const override { return (!!tMatch.GetAttr(m_tNullMapLocator)) ^ (!m_bIsNull); }

private:
	bool			m_bIsNull = false;
	CSphAttrLocator	m_tNullMapLocator;
};


JoinNullFilter_c::JoinNullFilter_c ( bool bIsNull, const CSphAttrLocator & tNullMapLocator )
	: m_bIsNull ( bIsNull )
	, m_tNullMapLocator ( tNullMapLocator )
{}



std::unique_ptr<ISphFilter> CreateJoinNullFilter ( const CSphFilterSettings & tSettings, const CSphAttrLocator & tNullMapLocator )
{
	return std::make_unique<JoinNullFilter_c> ( tSettings.m_bIsNull, tNullMapLocator );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ISphMatchSorter * CreateJoinSorter ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const SphQueueSettings_t & tSettings, const CSphQuery & tQuery, ISphMatchSorter * pSorter, const CSphQuery & tJoinQueryOptions, bool bJoinedGroupSort, int iBatchSize, CSphString & sError )
{
	if ( !tSettings.m_pJoinArgs )
		return pSorter;

	if ( !CheckJoinOnFilters ( pIndex, pJoinedIndex, tQuery, sError ) )
	{
		SafeDelete(pSorter);
		return nullptr;
	}

	std::unique_ptr<JoinSorter_c> pJoinSorter = std::make_unique<JoinSorter_c> ( pIndex, pJoinedIndex, tQuery, tJoinQueryOptions, pSorter, bJoinedGroupSort, iBatchSize );
	if ( pJoinSorter->GetErrorFlag() )
	{
		sError = pJoinSorter->GetErrorMessage();
		pJoinSorter.reset();
	}

	return pJoinSorter.release();
}


bool CreateJoinMultiSorter ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, const SphQueueSettings_t & tSettings, const VecTraits_T<CSphQuery> & dQueries, const VecTraits_T<CSphQuery> & dJoinQueryOptions, VecTraits_T<ISphMatchSorter *> & dSorters, int iBatchSize, CSphString & sError )
{
	if ( !tSettings.m_pJoinArgs )
		return true;

	if ( !CheckJoinOnFilters ( pIndex, pJoinedIndex, dQueries.First(), sError ) )
		return false;

	// the idea is that 1st sorter does the join AND it also pushes joined matches to all other sorters
	// to avoid double push to 1..N sorters they are wrapped in a class that prevents pushing matches
	std::unique_ptr<JoinMultiSorter_c> pJoinSorter = std::make_unique<JoinMultiSorter_c> ( pIndex, pJoinedIndex, dQueries, dJoinQueryOptions, dSorters, iBatchSize );
	if ( pJoinSorter->GetErrorFlag() )
	{
		sError = pJoinSorter->GetErrorMessage();
		pJoinSorter.reset();
	}

	dSorters[0] = pJoinSorter.release();

	for ( int i = 1; i < dSorters.GetLength(); i++ )
	{
		if ( !dSorters[i] )
			continue;

		dSorters[i] = new SorterWrapperNoPush_c ( dSorters[i] );
	}

	return true;
}
