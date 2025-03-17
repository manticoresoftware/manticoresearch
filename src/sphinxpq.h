//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

// This header shares some structs and methods related to Percolate indexes
// in order to acces them in unit-tests


#ifndef MANTICORE_SPHINXPQ_H
#define MANTICORE_SPHINXPQ_H

#include "sphinxsearch.h"
#include "sphinxrt.h"
#include "searchdha.h"
#include "accumulator.h"
#include "querycontext.h"

// stuff moved here from sphinxrt.h

struct PercolateQueryDesc
{
	int64_t m_iQUID;
	CSphString m_sQuery;
	CSphString m_sTags;
	CSphString m_sFilters;
	bool m_bQL;

	void Swap ( PercolateQueryDesc & tOther );
};

struct PercolateMatchResult_t : ISphNoncopyable
{
	bool m_bGetDocs = false;
	bool m_bGetQuery = false;
	bool m_bGetFilters = true;

	int m_iQueriesMatched = 0;
	int m_iQueriesFailed = 0;
	int m_iDocsMatched = 0;
	int64_t m_tmTotal = 0;

	// verbose data
	bool m_bVerbose = false;

	int	m_iEarlyOutQueries = 0;
	int	m_iTotalQueries = 0;
	int m_iOnlyTerms = 0;
	int64_t m_tmSetup = 0;

	Warner_c m_sMessages;
	CSphFixedVector<PercolateQueryDesc> m_dQueryDesc { 0 };
	CSphFixedVector<int> m_dDocs { 0 };
	CSphFixedVector<int> m_dQueryDT { 0 }; // microsecond time per query

	PercolateMatchResult_t() = default;
	PercolateMatchResult_t ( PercolateMatchResult_t&& rhs ) noexcept;
	PercolateMatchResult_t& operator = ( PercolateMatchResult_t rhs ) noexcept;
	void Swap ( PercolateMatchResult_t& rhs ) noexcept;

	void Reset ();
};

struct PercolateQueryArgs_t
{
	const char * m_sQuery = nullptr;
	const char * m_sTags = nullptr;
	const VecTraits_T<CSphFilterSettings> & m_dFilters;
	const VecTraits_T<FilterTreeItem_t> & m_dFilterTree;
	int64_t m_iQUID = 0;
	bool m_bQL = true;

	bool m_bReplace = false;

	PercolateQueryArgs_t ( const VecTraits_T<CSphFilterSettings> & dFilters, const VecTraits_T<FilterTreeItem_t> & dFilterTree );
	explicit PercolateQueryArgs_t ( const StoredQueryDesc_t & tDesc );
};

class StoredQuery_i;

class PercolateIndex_i : public RtIndex_i
{
public:
	PercolateIndex_i ( CSphString sIndexName, CSphString sPath ) : RtIndex_i { std::move ( sIndexName ), std::move ( sPath ) } {}
	virtual bool	MatchDocuments ( RtAccum_t * pAccExt, PercolateMatchResult_t & tResult ) = 0;

	virtual std::unique_ptr<StoredQuery_i> CreateQuery ( PercolateQueryArgs_t & tArgs, CSphString & sError ) = 0;

	bool	IsPQ() const override { return true; }

};

/// percolate query index factory
std::unique_ptr<PercolateIndex_i> CreateIndexPercolate ( CSphString sIndexName, CSphString sPath, CSphSchema tSchema );
void FixPercolateSchema ( CSphSchema & tSchema );

using CreateQueryParser_fn = std::unique_ptr<QueryParser_i> ( bool bJson );
void SetPercolateQueryParserFactory ( CreateQueryParser_fn * pCall );

static const int PQ_META_VERSION_MAX = 255;

void LoadStoredQuery ( ByteBlob_t tData, StoredQueryDesc_t& tQuery );
void LoadStoredQuery ( DWORD uVersion, StoredQueryDesc_t & tQuery, CSphReader & tReader );
void LoadStoredQueryV6 ( DWORD uVersion, StoredQueryDesc_t & tQuery, CSphReader & tReader );
void SaveStoredQuery ( const StoredQueryDesc_t & tQuery, MemoryWriter_c& tWriter );
void SaveStoredQuery ( const StoredQueryDesc_t & tQuery, CSphWriter & tWriter );
void LoadDeleteQuery ( ByteBlob_t tData, CSphVector<int64_t>& dQueries, CSphString& sTags );
void LoadDeleteQuery ( CSphVector<int64_t> & dQueries, CSphString & sTags, CSphReader & tReader );
void SaveDeleteQuery ( const VecTraits_T<int64_t>& dQueries, const char * sTags, MemoryWriter_c& tWriter );
void SaveDeleteQuery ( const VecTraits_T<int64_t>& dQueries, const char * sTags, CSphWriter & tWriter );

//////////////////////////////////////////////////////////////////////////

struct DictTerm_t
{
	SphWordID_t m_uWordID = 0;
	int m_iWordOff = 0;
	int m_iWordLen = 0;
};

struct DictMap_t
{
	OpenHashTable_T<int64_t, DictTerm_t> m_hTerms { 0 };
	CSphFixedVector<BYTE> m_dKeywords { 0 };

	SphWordID_t GetTerm ( BYTE * pWord ) const;
};


struct StoredQuery_t;


struct SegmentReject_t
{
	CSphVector<uint64_t> m_dTerms;
	CSphFixedVector<uint64_t> m_dWilds { 0 };
	CSphFixedVector<CSphVector<uint64_t> > m_dPerDocTerms { 0 };
	CSphFixedVector<uint64_t> m_dPerDocWilds { 0 };
	int m_iRows = 0;

	bool Filter ( const StoredQuery_t * pStored, bool bUtf8 ) const;
};

class PercolateQwordSetup_c : public ISphQwordSetup
{
public:
	PercolateQwordSetup_c ( const RtSegment_t * pSeg, int iMaxCodepointLength, ESphHitless eHitless )
		: m_pSeg ( pSeg )
		, m_iMaxCodepointLength ( iMaxCodepointLength )
		, m_eHitless ( eHitless )
	{}

	ISphQword * QwordSpawn ( const XQKeyword_t & ) const final;
	bool QwordSetup ( ISphQword * pQword ) const final;
	ISphQword * ScanSpawn ( int iAtomPos ) const final;

private:
	const RtSegment_t * m_pSeg;
	int m_iMaxCodepointLength;
	ESphHitless m_eHitless = SPH_HITLESS_NONE;
};

class PercolateDictProxy_c : public DictStub_c
{
	const DictMap_t * m_pDict = nullptr;
	const bool m_bHasMorph = false;
	DictRefPtr_c m_pDictMorph;

public:
	explicit PercolateDictProxy_c ( bool bHasMorph, DictRefPtr_c pDictMorph )
		: m_bHasMorph ( bHasMorph )
		, m_pDictMorph ( std::move (pDictMorph) )
	{
	}

	void SetMap ( const DictMap_t &hDict )
	{
		m_pDict = &hDict;
	}

	// these only got called actually
	SphWordID_t GetWordID ( BYTE * pWord ) final;

	SphWordID_t GetWordIDNonStemmed ( BYTE * pWord ) final
	{
		assert ( m_pDict );
		return const_cast<DictMap_t *>(m_pDict)->GetTerm ( pWord );
	}

	bool HasMorphology () const final { return m_bHasMorph; }
	void Setup ( const CSphDictSettings &tSettings ) final {}
};

struct PQMatchContextResult_t
{
	CSphVector<PercolateQueryDesc> m_dQueryMatched;
	CSphVector<int> m_dDocsMatched;
	CSphVector<int> m_dDt;
	int m_iDocsMatched = 0;
	int m_iEarlyPassed = 0;
	int m_iOnlyTerms = 0;
	int m_iQueriesFailed = 0;
	Warner_c m_dMsg;
};

struct PercolateMatchContext_t : public PQMatchContextResult_t
{
	bool m_bGetDocs = false;
	bool m_bGetQuery = false;
	bool m_bGetFilters = false;
	bool m_bVerbose = false;

	CSphRefcountedPtr<PercolateDictProxy_c> m_pDictMap;
	CSphQuery m_tDummyQuery;
	std::unique_ptr<CSphQueryContext> m_pCtx;
	std::unique_ptr<PercolateQwordSetup_c> m_pTermSetup;

	// const actually shared between all workers
	const ISphSchema &m_tSchema;
	const SegmentReject_t &m_tReject;
	const bool m_bUtf8 = false;
	int64_t m_iMaxStackSize = session::GetMaxStackSize();

	PercolateMatchContext_t ( const RtSegment_t * pSeg, int iMaxCodepointLength, bool bHasMorph, DictRefPtr_c pDictMorph, const PercolateIndex_i * pIndex, const ISphSchema & tSchema,
			const SegmentReject_t & tReject, ESphHitless eHitless, bool bHasWideFields )
		: m_pDictMap { new PercolateDictProxy_c ( bHasMorph, std::move (pDictMorph) ) }
		, m_tSchema ( tSchema )
		, m_tReject ( tReject )
		, m_bUtf8 ( iMaxCodepointLength>1 )
	{
		m_tDummyQuery.m_eRanker = SPH_RANK_NONE;
		m_pCtx = std::make_unique<CSphQueryContext> ( m_tDummyQuery );
		m_pCtx->m_bSkipQCache = true;
		// for lookups to work
		m_pCtx->m_pIndexData = pSeg;

		// setup search terms
		m_pTermSetup = std::make_unique<PercolateQwordSetup_c> ( pSeg, iMaxCodepointLength, eHitless );
		m_pTermSetup->SetDict ( (DictRefPtr_c)m_pDictMap );
		m_pTermSetup->m_pIndex = pIndex;
		m_pTermSetup->m_pCtx = m_pCtx.get ();
		m_pTermSetup->m_bHasWideFields = bHasWideFields;
	}
};

void PercolateMergeResults ( const VecTraits_T<PQMatchContextResult_t *> &dMatches, PercolateMatchResult_t &tRes );

struct CPqResult : public iQueryResult
{
	PercolateMatchResult_t m_dResult;
	CSphFixedVector<DocID_t> m_dDocids { 0 }; // check whether it necessary at all or not

	CPqResult () = default;

	CPqResult ( CPqResult &&rhs ) noexcept
		: m_dResult { std::move ( rhs.m_dResult ) }
		, m_dDocids { std::move ( rhs.m_dDocids ) }
	{}

	CPqResult &operator= ( CPqResult &&rhs ) noexcept
	{
		m_dResult = std::move ( rhs.m_dResult );
		m_dDocids = std::move ( rhs.m_dDocids );
		return *this;
	}

	void Reset () final
	{
		m_dResult.Reset ();
		m_dDocids.Reset ( 0 );
	}

	bool HasWarnings () const final
	{
		return false; // Stub. fixme!
	}
};

void MergePqResults ( const VecTraits_T<CPqResult *> &dChunks, CPqResult &dRes, bool bSparsed );

#endif //MANTICORE_SPHINXPQ_H
