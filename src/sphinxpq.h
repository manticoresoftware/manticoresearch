//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
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
//#include "binlog.h"
#include "accumulator.h"

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
	const CSphVector<CSphFilterSettings> & m_dFilters;
	const CSphVector<FilterTreeItem_t> & m_dFilterTree;
	int64_t m_iQUID = 0;
	bool m_bQL = true;

	bool m_bReplace = false;

	PercolateQueryArgs_t ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> & dFilterTree );
	explicit PercolateQueryArgs_t ( const StoredQueryDesc_t & tDesc );
};

class StoredQuery_i;

class PercolateIndex_i : public RtIndex_i
{
public:
	PercolateIndex_i ( const char * sIndexName, const char * sFileName ) : RtIndex_i ( sIndexName, sFileName ) {}
	virtual bool	MatchDocuments ( RtAccum_t * pAccExt, PercolateMatchResult_t & tResult ) = 0;

	virtual StoredQuery_i * CreateQuery ( PercolateQueryArgs_t & tArgs, CSphString & sError ) = 0;

	bool	IsPQ() const override { return true; }

	virtual int ReplayDeleteQueries ( const VecTraits_T<int64_t>& dQueries ) = 0;
	virtual int ReplayDeleteQueries ( const char * sTags ) = 0;
	virtual void ReplayCommit ( StoredQuery_i * pQuery ) = 0;
};

/// percolate query index factory
PercolateIndex_i * CreateIndexPercolate ( const CSphSchema & tSchema, const char * sIndexName, const char * sPath );
void FixPercolateSchema ( CSphSchema & tSchema );

typedef const QueryParser_i * CreateQueryParser_fn ( bool bJson );
void SetPercolateQueryParserFactory ( CreateQueryParser_fn * pCall );

static const int PQ_META_VERSION_MAX = 255;

void LoadStoredQuery ( const BYTE * pData, int iLen, StoredQueryDesc_t & tQuery );
void LoadStoredQuery ( DWORD uVersion, StoredQueryDesc_t & tQuery, CSphReader & tReader );
void LoadStoredQueryV6 ( DWORD uVersion, StoredQueryDesc_t & tQuery, CSphReader & tReader );
void SaveStoredQuery ( const StoredQueryDesc_t & tQuery, CSphVector<BYTE> & dOut );
void SaveStoredQuery ( const StoredQueryDesc_t & tQuery, CSphWriter & tWriter );
void LoadDeleteQuery ( const BYTE * pData, int iLen, CSphVector<int64_t> & dQueries, CSphString & sTags );
void LoadDeleteQuery ( CSphVector<int64_t> & dQueries, CSphString & sTags, CSphReader & tReader );
void SaveDeleteQuery ( const VecTraits_T<int64_t>& dQueries, const char * sTags, CSphVector<BYTE> & dOut );
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
	OpenHash_T<DictTerm_t, int64_t, HashFunc_Int64_t> m_hTerms;
	CSphVector<BYTE> m_dKeywords;

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
	ISphQword * ScanSpawn() const final;

private:
	const RtSegment_t * m_pSeg;
	int m_iMaxCodepointLength;
	ESphHitless m_eHitless = SPH_HITLESS_NONE;
};

class PercolateDictProxy_c : public CSphDict
{
	const DictMap_t * m_pDict = nullptr;
	const bool m_bHasMorph = false;
	DictRefPtr_c m_pDictMorph { nullptr };

public:
	explicit PercolateDictProxy_c ( bool bHasMorph, CSphDict * pDictMorph )
		: m_bHasMorph ( bHasMorph )
		, m_pDictMorph ( pDictMorph )
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

	// not implemented
	CSphDictSettings m_tDummySettings;
	CSphVector<CSphSavedFile> m_tDummySF;

	SphWordID_t GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) final { return 0; }
	void LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords ) final {}
	void LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer, bool bStripFile ) final {}
	void WriteStopwords ( CSphWriter &tWriter ) const final {}
	bool LoadWordforms ( const StrVec_t &, const CSphEmbeddedFiles * pEmbedded,
		const ISphTokenizer * pTokenizer, const char * sIndex ) final { return false; }
	void WriteWordforms ( CSphWriter &tWriter ) const final {}
	int SetMorphology ( const char * szMorph, CSphString &sMessage ) final { return 0; }
	void Setup ( const CSphDictSettings &tSettings ) final {}
	const CSphDictSettings &GetSettings () const final { return m_tDummySettings; }
	const CSphVector<CSphSavedFile> &GetStopwordsFileInfos () const final { return m_tDummySF; }
	const CSphVector<CSphSavedFile> &GetWordformsFileInfos () const final { return m_tDummySF; }
	const CSphMultiformContainer * GetMultiWordforms () const final { return nullptr; }
	bool IsStopWord ( const BYTE * pWord ) const final { return false; }
	uint64_t GetSettingsFNV () const final { return 0; }
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
};

struct PercolateMatchContext_t : public PQMatchContextResult_t
{
	bool m_bGetDocs = false;
	bool m_bGetQuery = false;
	bool m_bGetFilters = false;
	bool m_bVerbose = false;

	PercolateDictProxy_c m_tDictMap;
	CSphQuery m_tDummyQuery;
	CSphScopedPtr <CSphQueryContext> m_pCtx { nullptr };
	CSphScopedPtr <PercolateQwordSetup_c> m_pTermSetup { nullptr };

	// const actually shared between all workers
	const ISphSchema &m_tSchema;
	const SegmentReject_t &m_tReject;
	const bool m_bUtf8 = false;
	Warner_c m_dMsg;

	PercolateMatchContext_t ( const RtSegment_t * pSeg, int iMaxCodepointLength, bool bHasMorph,
			CSphDict * pDictMorph, const PercolateIndex_i * pIndex, const ISphSchema & tSchema,
			const SegmentReject_t & tReject, ESphHitless eHitless, bool bHasWideFields )
		: m_tDictMap ( bHasMorph, pDictMorph )
		, m_tSchema ( tSchema )
		, m_tReject ( tReject )
		, m_bUtf8 ( iMaxCodepointLength>1 )
	{
		m_tDummyQuery.m_eRanker = SPH_RANK_NONE;
		m_pCtx = new CSphQueryContext ( m_tDummyQuery );
		m_pCtx->m_bSkipQCache = true;
		// for lookups to work
		m_pCtx->m_pIndexData = pSeg;

		// setup search terms
		m_pTermSetup = new PercolateQwordSetup_c ( pSeg, iMaxCodepointLength, eHitless );
		m_pTermSetup->SetDict ( &m_tDictMap );
		m_pTermSetup->m_pIndex = pIndex;
		m_pTermSetup->m_pCtx = m_pCtx.Ptr ();
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
