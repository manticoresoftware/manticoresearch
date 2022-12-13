//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxpq.h"
#include "sphinxsort.h"
#include "fileutils.h"
#include "icu.h"
#include "accumulator.h"
#include "indexsettings.h"
#include "coroutine.h"
#include "mini_timer.h"
#include "binlog.h"
#include "indexfiles.h"
#include "tokenizer/tokenizer.h"
#include "task_dispatcher.h"

#include <atomic>

using namespace Threads;

/// protection from concurrent changes during binlog replay
#ifndef NDEBUG
static auto &g_bRTChangesAllowed = RTChangesAllowed ();
#endif

//////////////////////////////////////////////////////////////////////////
// percolate index

struct StoredQuery_t : public StoredQuery_i, public ISphRefcountedMT
{
	CSphFixedVector<uint64_t>		m_dRejectTerms { 0 };
	CSphFixedVector<uint64_t>		m_dRejectWilds { 0 };
	CSphFixedVector<uint64_t>		m_dTags { 0 };
	CSphVector<CSphString>			m_dSuffixes;
	DictMap_t						m_hDict;
	std::unique_ptr<XQQuery_t>		m_pXQ;

	bool							m_bOnlyTerms = false; // flag of simple query, ie only words and no operators
	bool							IsFullscan() const { return m_pXQ->m_bEmpty; }
};

using StoredQuerySharedPtr_t = SharedPtr_t<StoredQuery_t>;
using StoredQuerySharedPtrVecSharedPtr_t = SharedPtr_t<CSphVector<StoredQuerySharedPtr_t>>;
class SharedPQSlice_t : public VecTraits_T<const StoredQuerySharedPtr_t>
{
	using BASE_t = VecTraits_T<const StoredQuerySharedPtr_t>;
	StoredQuerySharedPtrVecSharedPtr_t m_pBackend;
	int64_t m_iGeneration {0};

public:
	explicit SharedPQSlice_t ( StoredQuerySharedPtrVecSharedPtr_t pBackend, int64_t iGeneration=0 )
		: BASE_t { *pBackend }
		, m_pBackend { std::move(pBackend) }
		, m_iGeneration ( iGeneration )
	{}

	SharedPQSlice_t () = default;
	SharedPQSlice_t ( SharedPQSlice_t&& rhs ) = default;
	SharedPQSlice_t& operator= ( SharedPQSlice_t&& rhs ) = default;

	int64_t Generation() const { return m_iGeneration; };
};

static FileAccessSettings_t g_tDummyFASettings;

class PercolateIndex_c : public PercolateIndex_i
{
public:
	PercolateIndex_c ( CSphString sIndexName, CSphString sPath, CSphSchema tSchema );
	~PercolateIndex_c () override;

	bool AddDocument ( InsertDocData_t & tDoc, bool bReplace, const CSphString & sTokenFilterOptions, CSphString & sError, CSphString & sWarning, RtAccum_t * pAccExt ) override;
	bool MatchDocuments ( RtAccum_t * pAccExt, PercolateMatchResult_t &tRes ) override;
	bool Commit ( int * pDeleted, RtAccum_t * pAccExt, CSphString* pError = nullptr ) override;
	void RollBack ( RtAccum_t * pAccExt ) override;

	std::unique_ptr<StoredQuery_i> CreateQuery ( PercolateQueryArgs_t & tArgs, CSphString & sError ) final EXCLUDES ( m_tLock );

	bool Prealloc ( bool bStripPath, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings ) override;
	void PostSetup() override EXCLUDES ( m_tLock );
	bool BindAccum ( RtAccum_t * pAccExt, CSphString * pError = nullptr ) override;
	TokenizerRefPtr_c CloneIndexingTokenizer() const override { return m_pTokenizerIndexing->Clone ( SPH_CLONE_INDEX ); }
	void SaveMeta ( bool bShutdown = false ) EXCLUDES ( m_tLock );
	void SaveMeta ( const SharedPQSlice_t& dStored, bool bShutdown = false );

	enum class LOAD_E { ParseError_e, GeneralError_e, Ok_e };
	LOAD_E LoadMetaJson ( const CSphString& sMeta, bool bStripPath, FilenameBuilder_i* pFilenameBuilder, StrVec_t& dWarnings );
	LOAD_E LoadMetaLegacy ( const CSphString& sMeta, bool bStripPath, FilenameBuilder_i* pFilenameBuilder, StrVec_t& dWarnings );
	bool LoadMeta ( const CSphString& sMeta, bool bStripPath, FilenameBuilder_i* pFilenameBuilder, StrVec_t& dWarnings );
	bool Truncate ( CSphString & ) override EXCLUDES ( m_tLock );

	// RT index stub
	bool MultiQuery ( CSphQueryResult &, const CSphQuery &, const VecTraits_T<ISphMatchSorter *> &, const CSphMultiQueryArgs & ) const override;
	bool DeleteDocument ( const VecTraits_T<DocID_t> &, CSphString & , RtAccum_t * pAccExt ) override { RollBack ( pAccExt ); return true; }
	void ForceRamFlush ( const char* szReason ) EXCLUDES ( m_tLock ) final;
	bool IsFlushNeed() const override;
	bool ForceDiskChunk () override;
	bool IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, StrVec_t & dWarnings, CSphString & sError ) const override;

	bool Reconfigure ( CSphReconfigureSetup & tSetup ) override EXCLUDES ( m_tLock );
	void ProcessDiskChunk ( int, VisitChunk_fn&& ) const final {};
	int64_t GetLastFlushTimestamp() const override { return m_tmSaved; }

	// plain index stub
	bool				EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const override;
	const CSphSourceStats &	GetStats () const override { return m_tStat; }
	void				GetStatus ( CSphIndexStatus* pRes ) const override;

	void				IndexDeleted() override { m_bIndexDeleted = true; }
	void				ProhibitSave() override { m_bSaveDisabled = true; }
	void				EnableSave() override { m_bSaveDisabled = false; }
	void				LockFileState ( CSphVector<CSphString> & dFiles ) final;

	const CSphSchema &GetMatchSchema () const override { return m_tMatchSchema; }
	virtual uint64_t GetSchemaHash () const final { return 0; }

	int64_t				GetMemLimit() const final { return 0; }

	Binlog::CheckTnxResult_t ReplayTxn(Binlog::Blop_e eOp,CSphReader& tReader, CSphString & sError, Binlog::CheckTxn_fn&& fnCanContinue) override; // cb from binlog
	Binlog::CheckTnxResult_t ReplayAdd(CSphReader& tReader, CSphString & sError, Binlog::CheckTxn_fn&& fnCanContinue);
	Binlog::CheckTnxResult_t ReplayDelete(CSphReader& tReader, Binlog::CheckTxn_fn&& fnCanContinue);
	Binlog::CheckTnxResult_t ReplayInsertAndDelete ( CSphReader& tReader, CSphString& sError, Binlog::CheckTxn_fn&& fnCanContinue );

private:
	static const DWORD				META_HEADER_MAGIC = 0x50535451;	///< magic 'PSTQ' header
	static const DWORD				META_VERSION = 9;				///< META in json format

	int								m_iLockFD = -1;
	CSphSourceStats					m_tStat;
	TokenizerRefPtr_c				m_pTokenizerIndexing;
	int								m_iMaxCodepointLength = 0;
	int64_t							m_iSavedTID = 0;
	int64_t							m_tmSaved = 0;
	bool							m_bSaveDisabled = false;
	bool							m_bHasFiles = false;
	bool							m_bIndexDeleted = false;

	StoredQuerySharedPtrVecSharedPtr_t	m_pQueries GUARDED_BY ( m_tLock );
	OpenHash_T< int, int64_t, HashFunc_Int64_t> m_hQueries GUARDED_BY ( m_tLock ); // QUID -> query
	int64_t							m_iGeneration GUARDED_BY ( m_tLock ) { 0 }; // eliminate ABA race on insert/delete
	mutable RwLock_t				m_tLock;

	CSphFixedVector<StoredQueryDesc_t>	m_dLoadedQueries { 0 }; // temporary, just descriptions
	CSphSchema						m_tMatchSchema;
	CSphVector<SphWordID_t>			m_dHitlessWords;

	void DoMatchDocuments ( const RtSegment_t * pSeg, PercolateMatchResult_t & tRes );
	bool MultiScan ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter*>& dSorters,
			const CSphMultiQueryArgs &tArgs ) const;
	bool CanBeAdded ( PercolateQueryArgs_t& tArgs, CSphString& sError ) const REQUIRES_SHARED ( m_tLock );
	std::unique_ptr<StoredQuery_i> CreateQuery ( PercolateQueryArgs_t& tArgs, const TokenizerRefPtr_c& pTokenizer, const DictRefPtr_c& pDict, CSphString& sError );

public:
	PercolateMatchContext_t * CreateMatchContext ( const RtSegment_t * pSeg, const SegmentReject_t &tReject );

private:
	int ReplayInsertAndDeleteQueries ( const VecTraits_T<StoredQuery_i*>& dNewQueries, const VecTraits_T<int64_t>& dDeleteQueries, const VecTraits_T<uint64_t>& dDeleteTags ) EXCLUDES ( m_tLock );

	void GetIndexFiles ( StrVec_t& dFiles, StrVec_t& dExtra, const FilenameBuilder_i* = nullptr ) const override;
	Bson_t ExplainQuery ( const CSphString & sQuery ) const final;

	StoredQuerySharedPtrVecSharedPtr_t MakeClone () const REQUIRES_SHARED ( m_tLock );
	void AddToStoredUnl ( StoredQuerySharedPtr_t tNew ) REQUIRES ( m_tLock );
	void PostSetupUnl () REQUIRES ( m_tLock  );
	SharedPQSlice_t GetStored () const EXCLUDES ( m_tLock );
	SharedPQSlice_t GetStoredUnl () const REQUIRES_SHARED ( m_tLock );

	void BinlogReconfigure ( CSphReconfigureSetup & tSetup );

	bool NeedStoreWordID () const override { return ( m_tSettings.m_eHitless==SPH_HITLESS_SOME && m_dHitlessWords.GetLength() ); }
	bool LoadMetaImpl ( const CSphString& sMeta, bool bStripPath, FilenameBuilder_i* pFilenameBuilder, StrVec_t& dWarnings );
};

//////////////////////////////////////////////////////////////////////////
// percolate functions

#define PERCOLATE_BLOOM_WILD_COUNT 32
#define PERCOLATE_BLOOM_SIZE PERCOLATE_BLOOM_WILD_COUNT * 2
#define PERCOLATE_WORDS_PER_CP 128

/// percolate query index factory
std::unique_ptr<PercolateIndex_i> CreateIndexPercolate ( CSphString sIndexName, CSphString sPath, CSphSchema tSchema )
{
	MEMORY ( MEM_INDEX_RT );
	return std::make_unique<PercolateIndex_c> ( std::move ( sIndexName ), std::move ( sPath ), std::move ( tSchema ) );
}

static SegmentReject_t SegmentGetRejects ( const RtSegment_t * pSeg, bool bBuildInfix, bool bUtf8, ESphHitless eHitless )
{
	SegmentReject_t tReject;
	tReject.m_iRows = pSeg->m_uRows;
	const bool bMultiDocs = ( pSeg->m_uRows>1 );
	if ( bMultiDocs )
	{
		tReject.m_dPerDocTerms.Reset ( pSeg->m_uRows );
		if ( bBuildInfix )
		{
			tReject.m_dPerDocWilds.Reset ( pSeg->m_uRows * PERCOLATE_BLOOM_SIZE );
			tReject.m_dPerDocWilds.Fill ( 0 );
		}
	}
	if ( bBuildInfix )
	{
		tReject.m_dWilds.Reset ( PERCOLATE_BLOOM_SIZE );
		tReject.m_dWilds.Fill ( 0 );
	}

	RtWordReader_c tDict ( pSeg, true, PERCOLATE_WORDS_PER_CP, eHitless );
	BloomGenTraits_t tBloom0 ( tReject.m_dWilds.Begin() );
	BloomGenTraits_t tBloom1 ( tReject.m_dWilds.Begin() + PERCOLATE_BLOOM_WILD_COUNT );

	while ( tDict.UnzipWord() )
	{
		const auto* pWord = (const RtWord_t*)tDict;
		const BYTE * pDictWord = pWord->m_sWord + 1;
		int iLen = pWord->m_sWord[0];

		uint64_t uHash = sphFNV64 ( pDictWord, iLen );
		tReject.m_dTerms.Add ( uHash );

		if ( bBuildInfix )
		{
			BuildBloom ( pDictWord, iLen, BLOOM_NGRAM_0, bUtf8, PERCOLATE_BLOOM_WILD_COUNT, tBloom0 );
			BuildBloom ( pDictWord, iLen, BLOOM_NGRAM_1, bUtf8, PERCOLATE_BLOOM_WILD_COUNT, tBloom1 );
		}

		if ( bMultiDocs )
		{
			RtDocReader_c tDoc ( pSeg, *pWord );
			while ( tDoc.UnzipDoc() )
			{

				assert ( tDoc->m_tRowID<pSeg->m_uRows );
				tReject.m_dPerDocTerms[tDoc->m_tRowID].Add ( uHash );

				if ( bBuildInfix )
				{
					uint64_t * pBloom = tReject.m_dPerDocWilds.Begin() + tDoc->m_tRowID * PERCOLATE_BLOOM_SIZE;
					BloomGenTraits_t tBloom2Doc0 ( pBloom );
					BloomGenTraits_t tBloom2Doc1 ( pBloom + PERCOLATE_BLOOM_WILD_COUNT );
					BuildBloom ( pDictWord, iLen, BLOOM_NGRAM_0, bUtf8, PERCOLATE_BLOOM_WILD_COUNT, tBloom2Doc0 );
					BuildBloom ( pDictWord, iLen, BLOOM_NGRAM_1, bUtf8, PERCOLATE_BLOOM_WILD_COUNT, tBloom2Doc1 );
				}
			}
		}
	}

	tReject.m_dTerms.Uniq();
	if ( bMultiDocs )
	{
		for ( auto & dTerms : tReject.m_dPerDocTerms )
			dTerms.Uniq();
	}
	return tReject;
}

static void DoQueryGetRejects ( const XQNode_t * pNode, const DictRefPtr_c& pDict, CSphVector<uint64_t> & dRejectTerms, CSphFixedVector<uint64_t> & dRejectBloom, CSphVector<CSphString> & dSuffixes, bool & bOnlyTerms, bool bUtf8 )
{
	// FIXME!!! replace recursion to prevent stack overflow for large and complex queries
	if ( pNode && !( pNode->GetOp()==SPH_QUERY_AND || pNode->GetOp()==SPH_QUERY_ANDNOT ) )
		bOnlyTerms = false;

	if ( !pNode || pNode->GetOp()==SPH_QUERY_NOT )
		return;

	BYTE sTmp[3 * SPH_MAX_WORD_LEN + 16];
	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		const XQKeyword_t & tWord = pNode->m_dWords[i];
		int iLen = tWord.m_sWord.Length();
		assert ( iLen < (int)sizeof( sTmp ) );

		if ( !iLen )
			continue;

		bool bStarTerm = false;
		int iCur = 0;
		int iInfixLen = 0;
		const char * sInfix = nullptr;
		const char * s = tWord.m_sWord.cstr();
		BYTE * sDst = sTmp;

		while ( *s )
		{
			if ( sphIsWild ( *s ) )
			{
				iCur = 0;
				bStarTerm = true;
			} else if ( ++iCur>iInfixLen )
			{
				sInfix = s - iCur + 1;
				iInfixLen = iCur;
			}
			*sDst++ = *s++;
		}
		sTmp[iLen] = '\0';

		// term goes to bloom
		if ( bStarTerm )
		{
			// initialize bloom filter array
			if ( !dRejectBloom.GetLength() )
			{
				dRejectBloom.Reset ( PERCOLATE_BLOOM_SIZE );
				dRejectBloom.Fill ( 0 );
			}

			BloomGenTraits_t tBloom0 ( dRejectBloom.Begin() );
			BloomGenTraits_t tBloom1 ( dRejectBloom.Begin() + PERCOLATE_BLOOM_WILD_COUNT );
			BuildBloom ( (const BYTE *)sInfix, iInfixLen, BLOOM_NGRAM_0, bUtf8, PERCOLATE_BLOOM_WILD_COUNT, tBloom0 );
			BuildBloom ( (const BYTE *)sInfix, iInfixLen, BLOOM_NGRAM_1, bUtf8, PERCOLATE_BLOOM_WILD_COUNT, tBloom1 );
			dSuffixes.Add().SetBinary ( sInfix, iInfixLen );

			continue;
		}

		SphWordID_t uWord = 0;
		if ( tWord.m_bMorphed )
			uWord = pDict->GetWordIDNonStemmed ( sTmp );
		else
			uWord = pDict->GetWordID ( sTmp );

		if ( !uWord )
			continue;

		// term goes to regular array
		dRejectTerms.Add ( sphFNV64 ( sTmp ) );
	}


	// composite nodes children recursion
	// for AND-NOT node NOT children should be skipped
	int iCount = pNode->m_dChildren.GetLength();
	if ( pNode->GetOp()==SPH_QUERY_ANDNOT && iCount>1 )
		iCount = 1;
	for ( int i=0; i<iCount; i++ )
		DoQueryGetRejects ( pNode->m_dChildren[i], pDict, dRejectTerms, dRejectBloom, dSuffixes, bOnlyTerms, bUtf8 );
}

static void QueryGetRejects ( const XQNode_t * pNode, const DictRefPtr_c& pDict, CSphFixedVector<uint64_t> & dRejectTerms, CSphFixedVector<uint64_t> & dRejectBloom, CSphVector<CSphString> & dSuffixes, bool & bOnlyTerms, bool bUtf8 )
{
	CSphVector<uint64_t> dTmpTerms;
	DoQueryGetRejects ( pNode, pDict, dTmpTerms, dRejectBloom, dSuffixes, bOnlyTerms, bUtf8 );
	dTmpTerms.Uniq();

	dRejectTerms.CopyFrom ( dTmpTerms );
}

static void DoQueryGetTerms ( const XQNode_t * pNode, const DictRefPtr_c& pDict, DictMap_t & hDict, CSphVector<BYTE> & dKeywords )
{
	if ( !pNode )
		return;

	BYTE sTmp[3 * SPH_MAX_WORD_LEN + 16];
	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		const XQKeyword_t & tWord = pNode->m_dWords[i];
		uint64_t uHash = sphFNV64 ( tWord.m_sWord.cstr() );
		if ( hDict.m_hTerms.Find ( uHash ) )
			continue;

		int iLen = tWord.m_sWord.Length();
		assert ( iLen < (int)sizeof( sTmp ) );

		if ( !iLen )
			continue;

		memcpy ( (char *)sTmp, tWord.m_sWord.cstr(), iLen );
		sTmp[iLen] = '\0';

		SphWordID_t uWord = 0;
		if ( tWord.m_bMorphed )
			uWord = pDict->GetWordIDNonStemmed ( sTmp );
		else
			uWord = pDict->GetWordID ( sTmp );

		if ( !uWord )
			continue;

		iLen = (int) strnlen ( (const char *)sTmp, sizeof(sTmp) );
		DictTerm_t & tTerm = hDict.m_hTerms.Acquire ( uHash );
		tTerm.m_uWordID = uWord;
		tTerm.m_iWordOff = dKeywords.GetLength();
		tTerm.m_iWordLen = iLen;

		dKeywords.Append ( sTmp, iLen );
	}

	for ( const XQNode_t * pChild : pNode->m_dChildren )
		DoQueryGetTerms ( pChild, pDict, hDict, dKeywords );
}

static void QueryGetTerms ( const XQNode_t * pNode, const DictRefPtr_c& pDict, DictMap_t & hDict )
{
	CSphVector<BYTE> dKeywords;
	DoQueryGetTerms ( pNode, pDict, hDict, dKeywords );

	hDict.m_dKeywords.CopyFrom ( dKeywords );
}

static bool TermsReject ( const VecTraits_T<uint64_t> & dDocs, const VecTraits_T<uint64_t> & dQueries )
{
	if ( !dDocs.GetLength() || !dQueries.GetLength() )
		return false;

	const uint64_t * pQTerm = dQueries.Begin();
	const uint64_t * pQEnd = dQueries.Begin() + dQueries.GetLength();
	const uint64_t * pTermDoc = dDocs.Begin();
	const uint64_t * pTermLast = dDocs.Begin() + dDocs.GetLength() - 1;

	for ( ; pQTerm<pQEnd && pTermDoc<=pTermLast; pQTerm++, pTermDoc++ )
	{
		pTermDoc = sphBinarySearch ( pTermDoc, pTermLast, *pQTerm );
		if ( !pTermDoc )
			return false;
	}

	return ( pQTerm==pQEnd );
}

static bool WildsReject ( const uint64_t * pFilter, const CSphFixedVector<uint64_t> & dQueries )
{
	if ( !dQueries.GetLength() )
		return false;

	const uint64_t * pQTerm = dQueries.Begin();
	const uint64_t * pQEnd = dQueries.Begin() + dQueries.GetLength();

	for ( ; pQTerm<pQEnd; pQTerm++, pFilter++ )
	{
		// check bloom passes
		if ( *pQTerm && ( (*pQTerm & *pFilter)!=*pQTerm ) )
			return false;
	}
	return true;
}

bool SegmentReject_t::Filter ( const StoredQuery_t * pStored, bool bUtf8 ) const
{
	// no early reject for complex queries
	if ( !pStored->m_bOnlyTerms )
		return false;

	// empty query rejects
	if ( !pStored->m_dRejectTerms.GetLength() && !pStored->m_dRejectWilds.GetLength() )
		return true;

	bool bTermsRejected = ( pStored->m_dRejectTerms.GetLength()==0 );
	if ( pStored->m_dRejectTerms.GetLength() )
		bTermsRejected = !TermsReject ( m_dTerms, pStored->m_dRejectTerms );

	if ( bTermsRejected && ( !m_dWilds.GetLength() || !pStored->m_dRejectWilds.GetLength() ) )
		return true;

	bool bWildRejected = ( m_dWilds.GetLength()==0 || pStored->m_dRejectWilds.GetLength()==0 );
	if ( m_dWilds.GetLength() && pStored->m_dRejectWilds.GetLength() )
		bWildRejected = !WildsReject ( m_dWilds.Begin(), pStored->m_dRejectWilds );

	if ( bTermsRejected && bWildRejected )
		return true;

	if ( !bTermsRejected && pStored->m_dRejectTerms.GetLength() && m_dPerDocTerms.GetLength() )
	{
		// in case no document matched - early reject triggers
		int iRejects = 0;
		ARRAY_FOREACH ( i, m_dPerDocTerms )
		{
			if ( TermsReject ( m_dPerDocTerms[i], pStored->m_dRejectTerms ) )
				break;

			iRejects++;
		}

		bTermsRejected = ( iRejects==m_dPerDocTerms.GetLength() );
	}

	if ( bTermsRejected && !bWildRejected && pStored->m_dRejectWilds.GetLength() && m_dPerDocWilds.GetLength() )
	{
		// in case no document matched - early reject triggers
		int iRowsPassed = 0;
		for ( int i=0; i<m_iRows && iRowsPassed==0; i++ )
		{
			BloomCheckTraits_t tBloom0 ( m_dPerDocWilds.Begin() + i * PERCOLATE_BLOOM_SIZE );
			BloomCheckTraits_t tBloom1 ( m_dPerDocWilds.Begin() + i * PERCOLATE_BLOOM_SIZE + PERCOLATE_BLOOM_WILD_COUNT );
			int iWordsPassed = 0;
			ARRAY_FOREACH ( iWord, pStored->m_dSuffixes )
			{
				const CSphString & sSuffix = pStored->m_dSuffixes[iWord];
				int iLen = sSuffix.Length();

				BuildBloom ( (const BYTE *)sSuffix.cstr(), iLen, BLOOM_NGRAM_0, bUtf8, PERCOLATE_BLOOM_WILD_COUNT, tBloom0 );
				if ( !tBloom0.IterateNext() )
					break;
				BuildBloom ( (const BYTE *)sSuffix.cstr(), iLen, BLOOM_NGRAM_1, bUtf8, PERCOLATE_BLOOM_WILD_COUNT, tBloom1 );
				if ( !tBloom1.IterateNext() )
					break;

				iWordsPassed++;
			}
			if ( iWordsPassed!=pStored->m_dSuffixes.GetLength() )
				continue;

			iRowsPassed++;
		}

		bWildRejected = ( iRowsPassed==0 );
	}

	return ( bTermsRejected && bWildRejected );
}

// FIXME!!! move to common RT code instead copy-paste it
struct SubstringInfo_t
{
	char m_sMorph[SPH_MAX_KEYWORD_LEN];
	const char * m_sSubstring = nullptr;
	const char * m_sWildcard = nullptr;
	int m_iSubLen = 0;
};

static Slice_t GetTermLocator ( const char * sWord, int iLen, const RtSegment_t * pSeg )
{
	Slice_t tChPoint;
	tChPoint.m_uLen = pSeg->m_dWords.GetLength();

	// tighten dictionary location
	if ( pSeg->m_dWordCheckpoints.GetLength() )
	{
		const RtWordCheckpoint_t* pCheckpoint = sphSearchCheckpointWrd ( sWord, iLen, false, pSeg->m_dWordCheckpoints );
		if ( !pCheckpoint )
		{
			tChPoint.m_uLen = pSeg->m_dWordCheckpoints.Begin()->m_iOffset;
		} else
		{
			tChPoint.m_uOff = pCheckpoint->m_iOffset;
			if ( ( pCheckpoint + 1 )<=( &pSeg->m_dWordCheckpoints.Last() ) )
				tChPoint.m_uLen = pCheckpoint[1].m_iOffset;
		}
	}

	return tChPoint;
}

static Slice_t GetPrefixLocator ( const char * sWord, bool bHasMorphology, const RtSegment_t * pSeg, SubstringInfo_t & tSubInfo )
{
	// do prefix expansion
	// remove exact form modifier, if any
	const char * sPrefix = sWord;
	if ( *sPrefix=='=' )
		sPrefix++;

	// skip leading wild-cards
	// (in case we got here on non-infix index path)
	const char * sWildcard = sPrefix;
	while ( sphIsWild ( *sPrefix ) )
	{
		sPrefix++;
		sWildcard++;
	}

	// compute non-wild-card prefix length
	int iPrefix = 0;
	for ( const char * s = sPrefix; *s && !sphIsWild ( *s ); s++ )
		iPrefix++;

	// prefix expansion should work on non-stemmed words only
	if ( bHasMorphology )
	{
		tSubInfo.m_sMorph[0] = MAGIC_WORD_HEAD_NONSTEMMED;
		memcpy ( tSubInfo.m_sMorph + 1, sPrefix, iPrefix );
		sPrefix = tSubInfo.m_sMorph;
		iPrefix++;
	}

	tSubInfo.m_sWildcard = sWildcard;
	tSubInfo.m_sSubstring = sPrefix;
	tSubInfo.m_iSubLen = iPrefix;

	Slice_t tChPoint;
	tChPoint.m_uLen = pSeg->m_dWords.GetLength();

	// find initial checkpoint or check words prior to 1st checkpoint
	if ( !pSeg->m_dWordCheckpoints.IsEmpty() )
	{
		const RtWordCheckpoint_t * pLast = &pSeg->m_dWordCheckpoints.Last();
		const RtWordCheckpoint_t * pCheckpoint = sphSearchCheckpointWrd( sPrefix, iPrefix, true, pSeg->m_dWordCheckpoints );

		if ( pCheckpoint )
		{
			// there could be valid data prior 1st checkpoint that should be unpacked and checked
			auto iNameLen = (int) strnlen ( pCheckpoint->m_sWord, SPH_MAX_KEYWORD_LEN );
			if ( pCheckpoint!=pSeg->m_dWordCheckpoints.Begin() || (sphDictCmp ( sPrefix, iPrefix, pCheckpoint->m_sWord, iNameLen )==0 && iPrefix==iNameLen) )
				tChPoint.m_uOff = pCheckpoint->m_iOffset;

			// find the last checkpoint that meets prefix condition ( ie might be a span of terms that splat to a couple of checkpoints )
			++pCheckpoint;
			while ( pCheckpoint<=pLast )
			{
				iNameLen = (int) strnlen ( pCheckpoint->m_sWord, SPH_MAX_KEYWORD_LEN );
				int iCmp = sphDictCmp ( sPrefix, iPrefix, pCheckpoint->m_sWord, iNameLen );
				if ( iCmp==0 && iPrefix==iNameLen )
					tChPoint.m_uOff = pCheckpoint->m_iOffset;
				if ( iCmp<0 )
					break;
				++pCheckpoint;
			}
		}
	}

	return tChPoint;
}

static void GetSuffixLocators ( const char * sWord, int iMaxCodepointLength, const RtSegment_t * pSeg, SubstringInfo_t & tSubInfo, CSphVector<Slice_t> & dPoints )
{
	assert ( sphIsWild ( *sWord ) );

	// find the longest substring of non-wild-cards
	const char * sMaxInfix = nullptr;
	int iMaxInfix = 0;
	int iCur = 0;

	for ( const char * s = sWord; *s; s++ )
	{
		if ( sphIsWild ( *s ) )
		{
			iCur = 0;
		} else if ( ++iCur>iMaxInfix )
		{
			sMaxInfix = s - iCur + 1;
			iMaxInfix = iCur;
		}
	}

	tSubInfo.m_sWildcard = sWord;
	tSubInfo.m_sSubstring = sMaxInfix;
	tSubInfo.m_iSubLen = iMaxInfix;

	CSphVector<DWORD> dInfixes;
	ExtractInfixCheckpoints ( sMaxInfix, iMaxInfix, iMaxCodepointLength, pSeg->m_dWordCheckpoints.GetLength(), pSeg->m_dInfixFilterCP, dInfixes );

	ARRAY_FOREACH ( i, dInfixes )
	{
		int iNext = dInfixes[i];
		iCur = iNext - 1;

		Slice_t & tChPoint = dPoints.Add();
		tChPoint.m_uOff = 0;
		tChPoint.m_uLen = pSeg->m_dWords.GetLength();

		if ( iCur>=0 )
			tChPoint.m_uOff = pSeg->m_dWordCheckpoints[iCur].m_iOffset;
		if ( iNext<pSeg->m_dWordCheckpoints.GetLength() )
			tChPoint.m_uLen = pSeg->m_dWordCheckpoints[iNext].m_iOffset;
	}
}

static void PercolateTags ( const char * szTags, CSphFixedVector<uint64_t> & dDstTags )
{
	if ( !szTags || !*szTags )
		return;

	StrVec_t dTagStrings;
	sphSplit ( dTagStrings, szTags );
	if ( dTagStrings.IsEmpty() )
		return;

	CSphFixedVector<uint64_t> dTmpTags ( dTagStrings.GetLength() );
	ARRAY_FOREACH ( i, dTagStrings )
		dTmpTags[i] = sphFNV64 ( dTagStrings[i].cstr() );
	
	dTmpTags.Sort();
	int iLen = sphUniq ( dTmpTags.Begin(), dTmpTags.GetLength() );
	dDstTags.CopyFrom ( dTmpTags.Slice ( 0, iLen ) );
}

static void PercolateAppendTags ( const CSphString& sTags, CSphVector<uint64_t>& dTags )
{
	if ( sTags.IsEmpty() )
		return;

	StrVec_t dTagStrings;
	sphSplit ( dTagStrings, sTags.cstr() );
	if ( dTagStrings.IsEmpty() )
		return;

	dTags.ReserveGap ( dTagStrings.GetLength() );
	for ( const auto& sTag : dTagStrings )
		dTags.Add ( sphFNV64 ( sTag.cstr() ) );
}

static bool TagsMatched ( const VecTraits_T<uint64_t>& dFilter, const VecTraits_T<uint64_t>& dQueryTags )
{
	auto *pFilter = dFilter.begin();
	auto *pQueryTags = dQueryTags.begin();
	auto *pFilterEnd = dFilter.end();
	auto *pTagsEnd = dQueryTags.end();

	while ( pFilter<pFilterEnd && pQueryTags<pTagsEnd )
	{
		if ( *pQueryTags<*pFilter )
			++pQueryTags;
		else if ( *pFilter<*pQueryTags )
			++pFilter;
		else if ( *pQueryTags==*pFilter )
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// percolate index definition

PercolateIndex_c::PercolateIndex_c ( CSphString sIndexName, CSphString sPath, CSphSchema tSchema )
	: PercolateIndex_i { std::move ( sIndexName ), std::move ( sPath ) }
{
	m_tSchema = std::move ( tSchema );

	// add id column
	CSphColumnInfo tCol ( sphGetDocidName () );
	tCol.m_eAttrType = SPH_ATTR_BIGINT;
	m_tMatchSchema.AddAttr ( tCol, true );

	// fill match schema
	m_tMatchSchema.AddAttr ( CSphColumnInfo ( "query", SPH_ATTR_STRINGPTR ), true );
	m_tMatchSchema.AddAttr ( CSphColumnInfo ( "tags", SPH_ATTR_STRINGPTR ), true );
	m_tMatchSchema.AddAttr ( CSphColumnInfo ( "filters", SPH_ATTR_STRINGPTR ), true );

	m_pQueries = new CSphVector<StoredQuerySharedPtr_t>;
}

PercolateIndex_c::~PercolateIndex_c ()
{
	bool bValid = m_pTokenizer && m_pDict;
	if ( bValid )
		SaveMeta ( sphInterrupted() );
	SafeClose ( m_iLockFD );

	if ( m_bIndexDeleted )
	{
		CSphString sFile = GetFilename ( "meta" );
		::unlink ( sFile.cstr() );
		sFile = GetFilename ( SPH_EXT_SETTINGS );
		::unlink ( sFile.cstr() );
	}
}

bool PercolateIndex_c::BindAccum ( RtAccum_t * pAccExt, CSphString* pError )
{
	return PrepareAccum ( pAccExt, true, pError );
}


bool PercolateIndex_c::AddDocument ( InsertDocData_t & tDoc, bool bReplace, const CSphString & sTokenFilterOptions, CSphString & sError, CSphString & sWarning, RtAccum_t * pAcc )
{
	if ( !BindAccum ( pAcc, &sError ) )
		return false;

	TokenizerRefPtr_c tTokenizer = CloneIndexingTokenizer ();
	if ( !tTokenizer )
	{
		sError = GetLastError ();
		return false;
	}

	CSphSource_StringVector tSrc ( tDoc.m_dFields, m_tSchema );
	if ( m_tSettings.m_bHtmlStrip &&
		!tSrc.SetStripHTML ( m_tSettings.m_sHtmlIndexAttrs.cstr(), m_tSettings.m_sHtmlRemoveElements.cstr(),
			m_tSettings.m_bIndexSP, m_tSettings.m_sZones.cstr(), sError ) )
		return false;

	// TODO: field filter \ token filter?
	tSrc.Setup ( m_tSettings, nullptr );
	tSrc.SetTokenizer ( std::move ( tTokenizer ) );
	tSrc.SetDict ( pAcc->m_pDict );
	if ( m_pFieldFilter )
		tSrc.SetFieldFilter ( m_pFieldFilter->Clone() );
	if ( !tSrc.Connect ( m_sLastError ) )
		return false;

	m_tSchema.CloneWholeMatch ( tSrc.m_tDocInfo, tDoc.m_tDoc );

	bool bEOF = false;
	if ( !tSrc.IterateStart ( sError ) || !tSrc.IterateDocument ( bEOF, sError ) )
		return false;

	ISphHits * pHits = tSrc.IterateHits ( sError );
	pAcc->GrabLastWarning ( sWarning );

	pAcc->AddDocument ( pHits, tDoc, true, m_tSchema.GetRowSize(), nullptr );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// percolate Qword

struct PercolateQword_t : public ISphQword
{
public:
	PercolateQword_t () = default;

	const CSphMatch & GetNextDoc() final
	{
		m_iHits = 0;
		while ( !m_tDocReader.UnzipDoc() )
		{
			if ( m_iDoc >= m_dDoclist.GetLength() )
			{
				m_tMatch.m_tRowID = INVALID_ROWID;
				return m_tMatch;
			}
			SetupReader();
		}

		const auto& tDoc = *m_tDocReader;
		m_tMatch.m_tRowID = tDoc.m_tRowID;
		m_dQwordFields.Assign32 ( tDoc.m_uDocFields );
		m_uMatchHits = tDoc.m_uHits;
		m_iHitlistPos = (uint64_t( tDoc.m_uHits)<<32) + tDoc.m_uHit;
		m_bAllFieldsKnown = false;
		return m_tMatch;
	}

	void SeekHitlist ( SphOffset_t uOff ) final
	{
		int iHits = (int)(uOff>>32);
		if ( iHits==1 )
		{
			m_uNextHit = DWORD(uOff);
		} else
		{
			m_uNextHit = 0;
			m_tHitReader.Seek ( m_pHits + DWORD ( uOff ), iHits );
		}
	}

	Hitpos_t GetNextHit () final
	{
		if ( !m_uNextHit )
			return Hitpos_t ( m_tHitReader.UnzipHit() );
		else if ( m_uNextHit==0xffffffffUL )
			return EMPTY_HIT;
		else
			return Hitpos_t ( std::exchange ( m_uNextHit, 0xffffffffUL ) );
	}

	bool Setup ( const RtSegment_t * pSeg, CSphVector<Slice_t> & dDoclist )
	{
		m_iDoc = 0;
		m_tDocReader.Reset();
		m_pSeg = pSeg;
		SafeAddRef ( pSeg );
		m_pHits = pSeg->m_dHits.begin();

		m_dDoclist.Set ( dDoclist.Begin(), dDoclist.GetLength() );
		dDoclist.LeakData();

		if ( m_iDoc && m_iDoc>=m_dDoclist.GetLength() )
			return false;

		SetupReader();
		return true;
	}

private:
	void SetupReader ()
	{
		RtWord_t tWord;
		tWord.m_uDoc = m_dDoclist[m_iDoc].m_uOff;
		tWord.m_uDocs = m_dDoclist[m_iDoc].m_uLen;
		m_tDocReader.Init ( m_pSeg, tWord );
		++m_iDoc;
	}

	ConstRtSegmentRefPtf_t		m_pSeg;
	CSphFixedVector<Slice_t>	m_dDoclist { 0 };
	CSphMatch					m_tMatch;
	RtDocReader_c				m_tDocReader;
	RtHitReader_c				m_tHitReader;

	int							m_iDoc = 0;
	DWORD						m_uNextHit = 0;
	const BYTE*					m_pHits = nullptr;
};


enum class PERCOLATE
{
	EXACT,
	PREFIX,
	INFIX
};

ISphQword *	PercolateQwordSetup_c::QwordSpawn ( const XQKeyword_t & ) const
{
	return new PercolateQword_t();
}

bool PercolateQwordSetup_c::QwordSetup ( ISphQword * pQword ) const
{
	auto * pMyQword = (PercolateQword_t *)pQword;
	const char * sWord = pMyQword->m_sDictWord.cstr();
	int iWordLen = pMyQword->m_sDictWord.Length();
	if ( !iWordLen )
		return false;

	// fix for the case '=*term' that should count as infix
	if ( iWordLen>1 && sWord[0]==MAGIC_WORD_HEAD_NONSTEMMED && sWord[1]=='*' )
	{
		sWord++;
		iWordLen--;
	}

	SubstringInfo_t tSubInfo;
	CSphVector<Slice_t> dDictLoc;
	PERCOLATE eCmp = PERCOLATE::EXACT;
	if ( !sphHasExpandableWildcards ( sWord ) )
	{
		// no wild-cards, or just wild-cards? do not expand
		Slice_t tChPoint = GetTermLocator ( sWord, iWordLen, m_pSeg );
		dDictLoc.Add ( tChPoint );
	} else if ( !sphIsWild ( *sWord ) )
	{
		eCmp = PERCOLATE::PREFIX;
		Slice_t tChPoint = GetPrefixLocator ( sWord, m_pDict->HasMorphology(), m_pSeg, tSubInfo );
		dDictLoc.Add ( tChPoint );
	} else
	{
		eCmp = PERCOLATE::INFIX;
		GetSuffixLocators ( sWord, m_iMaxCodepointLength, m_pSeg, tSubInfo, dDictLoc );
	}

	// to skip heading magic chars ( NONSTEMMED ) in the prefix
	int iSkipMagic = 0;
	if ( eCmp==PERCOLATE::PREFIX || eCmp==PERCOLATE::INFIX )
		iSkipMagic = ( BYTE ( *tSubInfo.m_sSubstring )<0x20 );

	// utf8 conversion for sphWildcardMatch
	int dWildcard [ SPH_MAX_WORD_LEN + 1 ];
	int * pWildcard = nullptr;
	if ( ( eCmp==PERCOLATE::PREFIX || eCmp==PERCOLATE::INFIX ) && sphIsUTF8 ( tSubInfo.m_sWildcard ) && sphUTF8ToWideChar ( tSubInfo.m_sWildcard, dWildcard, SPH_MAX_WORD_LEN ) )
		pWildcard = dWildcard;

	// cases:
	// empty - check all words
	// no matches - check only words prior to 1st checkpoint
	// checkpoint found - check words at that checkpoint
	const BYTE * pWordBase = m_pSeg->m_dWords.Begin();
	CSphVector<Slice_t> dDictWords;
	ARRAY_FOREACH ( i, dDictLoc )
	{
		RtWordReader_c tReader ( m_pSeg, true, PERCOLATE_WORDS_PER_CP, m_eHitless );
		// locator
		// m_uOff - Start
		// m_uLen - End
		tReader.m_pCur = pWordBase + dDictLoc[i].m_uOff;
		tReader.m_pMax = pWordBase + dDictLoc[i].m_uLen;

		while ( tReader.UnzipWord() )
		{
			const auto* pWord = (const RtWord_t*)tReader;
			// stemmed terms do not match any kind of wild-cards
			if ( ( eCmp==PERCOLATE::PREFIX || eCmp==PERCOLATE::INFIX ) && m_pDict->HasMorphology() && pWord->m_sWord[1]!=MAGIC_WORD_HEAD_NONSTEMMED )
				continue;

			int iCmp = -1;
			switch ( eCmp )
			{
			case PERCOLATE::EXACT:
				iCmp = sphDictCmpStrictly ( (const char *)pWord->m_sWord + 1, pWord->m_sWord[0], sWord, iWordLen );
				break;

			case PERCOLATE::PREFIX:
				iCmp = sphDictCmp ( (const char *)pWord->m_sWord + 1, pWord->m_sWord[0], tSubInfo.m_sSubstring, tSubInfo.m_iSubLen );
				if ( iCmp==0 )
				{
					if ( !( tSubInfo.m_iSubLen<=pWord->m_sWord[0] && sphWildcardMatch ( (const char *)pWord->m_sWord + 1 + iSkipMagic, tSubInfo.m_sWildcard, pWildcard ) ) )
						iCmp = -1;
				}
				break;

			case PERCOLATE::INFIX:
				if ( sphWildcardMatch ( (const char *)pWord->m_sWord + 1 + iSkipMagic, tSubInfo.m_sWildcard, pWildcard ) )
					iCmp = 0;
				break;

			default: break;
			}

			if ( iCmp==0 )
			{
				pMyQword->m_iDocs += pWord->m_uDocs;
				pMyQword->m_iHits += pWord->m_uHits;

				Slice_t & tDictPoint = dDictWords.Add();
				tDictPoint.m_uOff = pWord->m_uDoc;
				tDictPoint.m_uLen = pWord->m_uDocs;
			}

			if ( iCmp>0 || ( iCmp==0 && eCmp==PERCOLATE::EXACT ) )
				break;
		}
	}

	bool bWordSet = false;
	if ( dDictWords.GetLength() )
	{
		dDictWords.Sort ( bind ( &Slice_t::m_uOff ) );
		bWordSet = pMyQword->Setup ( m_pSeg, dDictWords );
	}

	return bWordSet;
}

ISphQword * PercolateQwordSetup_c::ScanSpawn() const
{
	return new QwordScan_c ( m_pSeg->m_uRows );
}

SphWordID_t PercolateDictProxy_c::GetWordID ( BYTE * pWord )
{
	assert ( m_pDict );
	assert ( !m_bHasMorph || m_pDictMorph );

	// apply stemmers
	if ( m_bHasMorph )
		m_pDictMorph->GetWordID ( pWord );

	return const_cast<DictMap_t *>(m_pDict)->GetTerm ( pWord );
}


SphWordID_t DictMap_t::GetTerm ( BYTE * sWord ) const
{
	const DictTerm_t * pTerm = m_hTerms.Find ( sphFNV64 ( sWord ) );
	if ( !pTerm )
		return 0;

	memcpy ( sWord, m_dKeywords.Begin() + pTerm->m_iWordOff, pTerm->m_iWordLen );
	return pTerm->m_uWordID;
}

PercolateMatchContext_t * PercolateIndex_c::CreateMatchContext ( const RtSegment_t * pSeg, const SegmentReject_t & tReject )
{
	return new PercolateMatchContext_t ( pSeg, m_iMaxCodepointLength, m_pDict->HasMorphology(), GetStatelessDict ( m_pDict ), this
												   , m_tSchema, tReject, m_tSettings.m_eHitless, ( m_tSchema.GetFieldsCount()>32 ) );
}

namespace {

// full scan when no docs required
int FullscanWithoutDocs ( PercolateMatchContext_t & tMatchCtx )
{
	const CSphIndex * pIndex = tMatchCtx.m_pTermSetup->m_pIndex;
	auto uRows = ((const RtSegment_t *) tMatchCtx.m_pCtx->m_pIndexData)->m_uRows;

	CSphMatch tDoc;
	int iMatchesCount = 0;
	for ( DWORD i = 0; i<uRows; ++i )
	{
		tDoc.m_tRowID = i;
		if ( !pIndex->EarlyReject ( tMatchCtx.m_pCtx.get(), tDoc ))
			++iMatchesCount;
	}
	return iMatchesCount;
}

// full scan and collect docs
int FullScanCollectingDocs ( PercolateMatchContext_t & tMatchCtx )
{
	const CSphIndex * pIndex = tMatchCtx.m_pTermSetup->m_pIndex;
	const auto * pSeg = (const RtSegment_t *) tMatchCtx.m_pCtx->m_pIndexData;
	int iStride = tMatchCtx.m_tSchema.GetRowSize ();

	int iCountIdx = tMatchCtx.m_dDocsMatched.GetLength ();
	tMatchCtx.m_dDocsMatched.Add ( 0 ); // placeholder for counter

	FakeRL_t _ ( pSeg->m_tLock ); // that is s-t by design, don't need real lock
	const CSphRowitem * pRow = pSeg->m_dRows.Begin ();
	CSphMatch tDoc;
	int iMatchesCount = 0;
	for ( DWORD i = 0; i<pSeg->m_uRows; ++i )
	{
		tDoc.m_tRowID = i;
		if ( !pIndex->EarlyReject ( tMatchCtx.m_pCtx.get(), tDoc ) )
		{
			tMatchCtx.m_dDocsMatched.Add ( (int)sphGetDocID(pRow) );
			++iMatchesCount;
		}
		pRow += iStride;
	}

	if ( iMatchesCount ) // write counter of docs into placeholder
		tMatchCtx.m_dDocsMatched[iCountIdx] = iMatchesCount;
	else
		tMatchCtx.m_dDocsMatched.Resize ( iCountIdx ); // pop's up reserved but not used matched counter
	return iMatchesCount;
}

// full-text search when no docs required
int FtMatchingWithoutDocs ( const StoredQuery_t * pStored, PercolateMatchContext_t & tMatchCtx )
{
	tMatchCtx.m_pDictMap->SetMap ( pStored->m_hDict ); // set terms dictionary
	CSphQueryResultMeta tTmpMeta;
	std::unique_ptr<ISphRanker> pRanker = sphCreateRanker ( *pStored->m_pXQ, tMatchCtx.m_tDummyQuery,
			tTmpMeta, *tMatchCtx.m_pTermSetup, *tMatchCtx.m_pCtx, tMatchCtx.m_tSchema );

	if ( !pRanker )
		return 0;

	int iMatchesCount = 0;
	for ( auto iMatches = pRanker->GetMatches (); iMatches!=0; iMatches = pRanker->GetMatches ())
		iMatchesCount += iMatches;
	return iMatchesCount;
}

// full-text search and collect docs
int FtMatchingCollectingDocs ( const StoredQuery_t * pStored, PercolateMatchContext_t & tMatchCtx )
{
	tMatchCtx.m_pDictMap->SetMap ( pStored->m_hDict ); // set terms dictionary
	CSphQueryResultMeta tTmpMeta;
	std::unique_ptr<ISphRanker> pRanker = sphCreateRanker ( *pStored->m_pXQ, tMatchCtx.m_tDummyQuery,
			tTmpMeta, *tMatchCtx.m_pTermSetup, *tMatchCtx.m_pCtx, tMatchCtx.m_tSchema );

	if ( !pRanker )
		return 0;

	int iCountIdx = tMatchCtx.m_dDocsMatched.GetLength();
	int iMatchesCount = 0;
	// reserve space for matched docs counter
	tMatchCtx.m_dDocsMatched.Add ( iMatchesCount );

	const auto * pSeg = (const RtSegment_t *)tMatchCtx.m_pCtx->m_pIndexData;
	const CSphMatch * pMatch = pRanker->GetMatchesBuffer();
	for ( auto iMatches = pRanker->GetMatches (); iMatches!=0; iMatches = pRanker->GetMatches ())
	{
		int * pDocids = tMatchCtx.m_dDocsMatched.AddN ( iMatches );
		for ( int i = 0; i<iMatches; ++i )
			pDocids[i] = (int)sphGetDocID ( pSeg->GetDocinfoByRowID ( pMatch[i].m_tRowID ) );
		iMatchesCount += iMatches;
	}

	if ( iMatchesCount ) // write counter of docs into placeholder
		tMatchCtx.m_dDocsMatched[iCountIdx] = iMatchesCount;
	else
		tMatchCtx.m_dDocsMatched.Resize ( iCountIdx ); // pop's up reserved but not used matched counter
	return iMatchesCount;
}

// percolate matching
void MatchingWork ( const StoredQuery_t * pStored, PercolateMatchContext_t & tMatchCtx )
{
	int64_t tmQueryStart = ( tMatchCtx.m_bVerbose ? sphMicroTimer() : 0 );
	tMatchCtx.m_iOnlyTerms += ( pStored->m_bOnlyTerms ? 1 : 0 );

	if ( !pStored->IsFullscan() && tMatchCtx.m_tReject.Filter ( pStored, tMatchCtx.m_bUtf8 ) )
		return;

	const auto * pSeg = (const RtSegment_t *)tMatchCtx.m_pCtx->m_pIndexData;
	FakeRL_t _ ( pSeg->m_tLock ); // that is s-t by design, don't need real lock
	const BYTE * pBlobs = pSeg->m_dBlobs.Begin();

	++tMatchCtx.m_iEarlyPassed;
	tMatchCtx.m_pCtx->ResetFilters();

	CSphString sError;
	CSphString sWarning;

	// setup filters
	CreateFilterContext_t tFlx;
	tFlx.m_pFilters = &pStored->m_dFilters;
	tFlx.m_pFilterTree = &pStored->m_dFilterTree;
	tFlx.m_pSchema = &tMatchCtx.m_tSchema;
	tFlx.m_pBlobPool = pBlobs;

	bool bRes = tMatchCtx.m_pCtx->CreateFilters ( tFlx, sError, sWarning );
	tMatchCtx.m_dMsg.Err ( sError );
	tMatchCtx.m_dMsg.Warn ( sWarning );

	if (!bRes )
	{
		++tMatchCtx.m_iQueriesFailed;
		return;
	}

	int iMatchesCount;
	if ( tMatchCtx.m_bGetDocs )
		iMatchesCount = pStored->IsFullscan ()
				? FullScanCollectingDocs ( tMatchCtx )
				: FtMatchingCollectingDocs ( pStored, tMatchCtx );
	else
		iMatchesCount = pStored->IsFullscan ()
				? FullscanWithoutDocs ( tMatchCtx )
				: FtMatchingWithoutDocs ( pStored, tMatchCtx );

	if ( !iMatchesCount )
		return;

	// collect matched pq, if any
	tMatchCtx.m_iDocsMatched += iMatchesCount;
	PercolateQueryDesc & tDesc = tMatchCtx.m_dQueryMatched.Add();
	tDesc.m_iQUID = pStored->m_iQUID;
	if ( tMatchCtx.m_bGetQuery )
	{
		tDesc.m_sQuery = pStored->m_sQuery;
		tDesc.m_sTags = pStored->m_sTags;
		tDesc.m_bQL = pStored->m_bQL;

		if ( tMatchCtx.m_bGetFilters && pStored->m_dFilters.GetLength() )
		{
			StringBuilder_c sFilters;
			FormatFiltersQL ( pStored->m_dFilters, pStored->m_dFilterTree, sFilters );
			sFilters.MoveTo ( tDesc.m_sFilters );
		}
	}

	if ( tMatchCtx.m_bVerbose )
		tMatchCtx.m_dDt.Add ( (int)( sphMicroTimer() - tmQueryStart ) );
}
} // static namespace

void PercolateQueryDesc::Swap ( PercolateQueryDesc & tOther )
{
	::Swap ( m_iQUID, tOther.m_iQUID );
	::Swap ( m_bQL, tOther.m_bQL );

	m_sQuery.Swap ( tOther.m_sQuery );
	m_sTags.Swap ( tOther.m_sTags );
	m_sFilters.Swap ( tOther.m_sFilters );
}

struct PQMergeIterator_t
{
	PQMatchContextResult_t * m_pMatch = nullptr;
	int m_iIdx = 0;
	int m_iElems = 0;

	CSphFixedVector<int> m_dElems;
	CSphFixedVector<int> m_dDocOffsets {0};

	explicit PQMergeIterator_t ( PQMatchContextResult_t * pMatch )
		: m_pMatch ( pMatch )
		, m_iElems ( pMatch->m_dQueryMatched.GetLength () )
		, m_dElems ( pMatch->m_dQueryMatched.GetLength () )
	{

		for ( int i=0; i<m_iElems; ++i )
			m_dElems[i] = i;
		m_dElems.Sort ( Lesser ( [this] ( int a, int b )
			  { return m_pMatch->m_dQueryMatched[a].m_iQUID < m_pMatch->m_dQueryMatched[b].m_iQUID; } ) );

		if ( pMatch->m_dDocsMatched.IsEmpty() )
			return;

		m_dDocOffsets.Reset ( pMatch->m_dQueryMatched.GetLength () );
		int iOffset = 0;
		for ( int i = 0; i < m_iElems; ++i )
		{
			m_dDocOffsets[i] = iOffset;
			iOffset += 1 + pMatch->m_dDocsMatched[iOffset];
		}
	}

	inline int CurElem() const { return m_dElems[m_iIdx]; }
	inline PercolateQueryDesc& CurDesc() const { return m_pMatch->m_dQueryMatched[CurElem()];}
	inline int CurDt () const { return m_pMatch->m_dDt[CurElem ()]; }
	inline int* CurDocs () const { return &m_pMatch->m_dDocsMatched[m_dDocOffsets[CurElem ()]]; }

	static inline bool IsLess ( PQMergeIterator_t *a, PQMergeIterator_t *b )
	{
		return a->CurDesc().m_iQUID<b->CurDesc().m_iQUID;
	}
};

// merge matches from one or many contexts into one result
void PercolateMergeResults ( const VecTraits_T<PQMatchContextResult_t *> & dMatches, PercolateMatchResult_t & tRes )
{
	if ( dMatches.IsEmpty() )
		return;

	int iGotQueries = 0;
	int iGotDocs = 0;
	tRes.m_iEarlyOutQueries = tRes.m_iTotalQueries;

	RawVector_T<PQMergeIterator_t> dIterators;
	CSphQueue<PQMergeIterator_t*, PQMergeIterator_t> qMatches ( dMatches.GetLength () );
	dIterators.Reserve ( dMatches.GetLength() );

	for ( PQMatchContextResult_t * pMatch : dMatches )
	{
		if ( pMatch->m_dQueryMatched.IsEmpty() )
			continue;

		dIterators.Emplace_back ( pMatch );
		qMatches.Push ( &dIterators.Last () );

		iGotQueries += pMatch->m_dQueryMatched.GetLength();
		iGotDocs += pMatch->m_iDocsMatched;

		tRes.m_iEarlyOutQueries -= pMatch->m_iEarlyPassed;
		tRes.m_iOnlyTerms += pMatch->m_iOnlyTerms;
		tRes.m_iQueriesFailed += pMatch->m_iQueriesFailed;
	}

	tRes.m_iQueriesMatched = iGotQueries;
	tRes.m_iDocsMatched = iGotDocs;
	iGotDocs += iGotQueries; // in addition to docs, the num of them is written per every query

	if ( !iGotQueries )
		return;

	tRes.m_dQueryDesc.Reset ( iGotQueries );
	PercolateQueryDesc * pDst = tRes.m_dQueryDesc.Begin ();

	int * pDt = nullptr;
	if ( tRes.m_bVerbose )
	{
		tRes.m_dQueryDT.Reset ( iGotQueries );
		pDt = tRes.m_dQueryDT.begin ();
		assert ( pDt );
	}

	int * pDocs = nullptr;
	if ( tRes.m_bGetDocs )
	{
		tRes.m_dDocs.Reset ( iGotDocs );
		pDocs = tRes.m_dDocs.begin();
		assert ( pDocs );
	}

	if ( qMatches.GetLength ()==1 ) // fastpath, only 1 essential result set
	{
		const auto & tIt = *qMatches.Root();
		assert ( tIt.m_iElems==iGotQueries );
		auto* pMatch = tIt.m_pMatch;
		for ( int iIdx : tIt.m_dElems )
		{
			pDst->Swap ( pMatch->m_dQueryMatched[iIdx] );
			++pDst;
			if ( pDt )
				*pDt++ = pMatch->m_dDt[iIdx];
			if ( pDocs )
			{
				auto iDocOff = tIt.m_dDocOffsets[iIdx];
				int iDocBlobSize = pMatch->m_dDocsMatched[iDocOff]+1;
				memcpy ( pDocs, &pMatch->m_dDocsMatched[iDocOff], sizeof ( int ) * iDocBlobSize );
				pDocs += iDocBlobSize;
			}
		}
		return;
	}

	PQMergeIterator_t* pMin = qMatches.Root ();
	qMatches.Pop ();

	assert ( qMatches.GetLength()>=0 ); // since case of only 1 resultset we already processed.

	while (true) {
		auto& tMin = *pMin;
		pDst->Swap ( tMin.CurDesc () );
		++pDst;

		if ( tRes.m_bVerbose )
			*pDt++ = tMin.CurDt();

		// docs copy
		if ( tRes.m_bGetDocs )
		{
			auto *pMinDocs = tMin.CurDocs ();
			int	 iDocBlobSize = *pMinDocs + 1;
			memcpy ( pDocs, pMinDocs, sizeof ( int ) * iDocBlobSize );
			pDocs += iDocBlobSize;
		}

		++tMin.m_iIdx;
		if ( tMin.m_iIdx<tMin.m_iElems )
		{
			// if current root is better - change the head.
			if ( qMatches.GetLength () && !PQMergeIterator_t::IsLess ( pMin, qMatches.Root() ) )
				qMatches.Push ( pMin );
			else
				continue;
		}

		if ( !qMatches.GetLength () )
			break;

		pMin = qMatches.Root();
		qMatches.Pop();
	}
}

// adaptor from vec of PercolateMatchContext_t* to PQMatchContextResult_t*
inline void PercolateMergeResults ( const VecTraits_T<PercolateMatchContext_t *> &dMatches, PercolateMatchResult_t &tRes )
{
	auto * pMatches = (PQMatchContextResult_t **) dMatches.begin ();
	auto iMatches = dMatches.GetLength ();
	PercolateMergeResults ( VecTraits_T<PQMatchContextResult_t *> ( pMatches, iMatches ), tRes );
}


struct PqMatchContextRef_t
{
	PercolateMatchContext_t * m_pMatchCtx;

	PercolateIndex_c * m_pIndex;
	const RtSegment_t * m_pSeg;
	const SegmentReject_t & m_tReject;
	const PercolateMatchResult_t & m_tRes;

	PqMatchContextRef_t ( PercolateIndex_c * pIndex, const RtSegment_t * pSeg,
			const SegmentReject_t & tReject, const PercolateMatchResult_t& tRes )
		: m_pIndex ( pIndex ), m_pSeg ( pSeg ), m_tReject ( tReject ), m_tRes ( tRes )
	{
		m_pMatchCtx = pIndex->CreateMatchContext( m_pSeg, m_tReject );
		m_pMatchCtx->m_bGetDocs = tRes.m_bGetDocs;
		m_pMatchCtx->m_bGetQuery = tRes.m_bGetQuery;
		m_pMatchCtx->m_bGetFilters = tRes.m_bGetFilters;
		m_pMatchCtx->m_bVerbose = tRes.m_bVerbose;
	}

	inline static bool IsClonable ()
	{
		return true;
	}
};

struct PqMatchContextClone_t : public PqMatchContextRef_t, ISphNoncopyable
{
	explicit PqMatchContextClone_t ( const PqMatchContextRef_t& dParent )
		: PqMatchContextRef_t ( dParent.m_pIndex, dParent.m_pSeg, dParent.m_tReject, dParent.m_tRes )
	{}
};

// display progress of pq execution
struct PQInfo_t : public TaskInfo_t
{
	DECLARE_RENDER( PQInfo_t );
	int m_iTotal = 0;
	int m_iCurrent = 0;
};

DEFINE_RENDER( PQInfo_t )
{
	auto & tInfo = *(const PQInfo_t *) pSrc;
	dDst.m_sChain << "PQ ";
	if ( tInfo.m_iTotal )
		dDst.m_sDescription.Sprintf ( "%d%% of %d:", tInfo.m_iCurrent * 100 / tInfo.m_iTotal, tInfo.m_iTotal );
	else
		dDst.m_sDescription.Sprintf ( "100% of %d:",tInfo.m_iTotal);
}

void PercolateIndex_c::DoMatchDocuments ( const RtSegment_t * pSeg, PercolateMatchResult_t & tRes )
{
	// reject need bloom filter for either infix or prefix
	auto tReject = SegmentGetRejects (
		  pSeg, ( m_tSettings.m_iMinInfixLen>0 || m_tSettings.GetMinPrefixLen ( m_pDict->GetSettings().m_bWordDict )>0 ), m_iMaxCodepointLength>1, m_tSettings.m_eHitless );

	auto dStored = GetStored();
	auto iJobs = dStored.GetLength ();
	tRes.m_iTotalQueries = iJobs;
	if ( !iJobs )
		return;

	// the context
	ClonableCtx_T<PqMatchContextRef_t, PqMatchContextClone_t, Threads::ECONTEXT::UNORDERED> dCtx { this, pSeg, tReject, tRes };
	auto pDispatcher = Dispatcher::Make ( iJobs, 0, GetEffectiveBaseDispatcherTemplate(), dCtx.IsSingle() );
	dCtx.LimitConcurrency ( pDispatcher->GetConcurrency() );

	if ( tRes.m_bVerbose )
		tRes.m_tmSetup = sphMicroTimer ()+tRes.m_tmSetup;

	Coro::ExecuteN ( dCtx.Concurrency ( iJobs ), [&]
	{
		auto pSource = pDispatcher->MakeSource();
		int iJob = -1; // make it consumed

		if ( !pSource->FetchTask ( iJob ) )
		{
			sphLogDebug ( "Early finish parallel DoMatchDocuments because of empty queue" );
			return; // already nothing to do, early finish.
		}

		auto pInfo = PublishTaskInfo ( new PQInfo_t );
		pInfo->m_iTotal = iJobs;
		auto tJobContext = dCtx.CloneNewContext();
		sphLogDebug ( "DoMatchDocuments cloned context %d", tJobContext.second );
		auto& tCtx = tJobContext.first;
		Threads::Coro::SetThrottlingPeriod ( session::GetThrottlingPeriodMS() );
		while (true)
		{
			sphLogDebugv ( "DoMatchDocuments %d, iJob: %d", tJobContext.second, iJob );
			pInfo->m_iCurrent = iJob;
			MatchingWork ( dStored[iJob], *tCtx.m_pMatchCtx );
			iJob = -1; // mark it consumed

			if ( !pSource->FetchTask ( iJob ) )
				return; // all is done

			// yield and reschedule every quant of time. It gives work to other tasks
			Threads::Coro::ThrottleAndKeepCrashQuery ();
		}
	});
	sphLogDebug ( "DoMatchDocuments processed in %d thread(s)", dCtx.NumWorked() );
	// collect and merge result set
	CSphVector<PercolateMatchContext_t *> dResults;
	dCtx.ForAll ( [&dResults] ( const PqMatchContextRef_t& tCtx ) { dResults.Add ( tCtx.m_pMatchCtx ); }, true );

	// merge result set
	PercolateMergeResults ( dResults, tRes );
	dResults.Apply ( [] ( PercolateMatchContext_t *& pCtx ) { SafeDelete ( pCtx ); } );
}


bool PercolateIndex_c::MatchDocuments ( RtAccum_t * pAcc, PercolateMatchResult_t & tRes )
{
	MEMORY ( MEM_INDEX_RT );

	int64_t tmStart = sphMicroTimer();
	if ( tRes.m_bVerbose )
		tRes.m_tmSetup = -tmStart;
	m_sLastWarning = "";

	if ( !BindAccum ( pAcc ) )
		return false;

	// empty txn or no queries just ignore
	if ( !pAcc->m_uAccumDocs || GetStored ().IsEmpty() )
	{
		pAcc->Cleanup ();
		return true;
	}

	pAcc->Sort();

	RtSegment_t * pSeg = CreateSegment ( pAcc, PERCOLATE_WORDS_PER_CP, m_tSettings.m_eHitless, m_dHitlessWords );
	assert ( !pSeg || pSeg->m_uRows>0 );
	assert ( !pSeg || pSeg->m_tAliveRows>0 );
	BuildSegmentInfixes ( pSeg, m_pDict->HasMorphology(), true, m_tSettings.m_iMinInfixLen,
		PERCOLATE_WORDS_PER_CP, ( m_iMaxCodepointLength>1 ), m_tSettings.m_eHitless );

	DoMatchDocuments ( pSeg, tRes );
	SafeRelease ( pSeg );

	// done; cleanup accum
	pAcc->Cleanup ();

	int64_t tmEnd = sphMicroTimer();
	tRes.m_tmTotal = tmEnd - tmStart;
	if ( tRes.m_iQueriesFailed )
		tRes.m_sMessages.Warn ( "%d queries failed", tRes.m_iQueriesFailed );
	return true;
}

void PercolateIndex_c::RollBack ( RtAccum_t * pAcc )
{
	assert ( g_bRTChangesAllowed );

	if ( BindAccum ( pAcc ) )
		pAcc->Cleanup();
}


bool PercolateIndex_c::EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const
{
	if ( !pCtx->m_pFilter )
		return false;

	auto * pSegment = (const RtSegment_t*)pCtx->m_pIndexData;
	tMatch.m_pStatic = pSegment->GetDocinfoByRowID ( tMatch.m_tRowID );

	return !pCtx->m_pFilter->Eval ( tMatch );
}

void PercolateIndex_c::GetStatus ( CSphIndexStatus * pRes ) const
{
	assert ( pRes );
	if (!pRes)
		return;

	CSphString sError;
	for ( const char * szExt : { "meta", "ram" } )
	{
		CSphAutofile fdRT ( GetFilename ( szExt ), SPH_O_READ, sError );
		int64_t iFileSize = fdRT.GetSize ();
		if ( iFileSize>0 )
			pRes->m_iDiskUse += iFileSize; // that uses disk, but not occupies
	}
	pRes->m_iTID = m_iTID;
	pRes->m_iSavedTID = m_iSavedTID;

	int64_t iRamUse = 0;
	{
		ScRL_t rLock { m_tLock };
		iRamUse = m_hQueries.GetLengthBytes();
		iRamUse += m_dHitlessWords.GetLengthBytes64() + m_dLoadedQueries.GetLengthBytes64();
		iRamUse = m_pQueries->GetLengthBytes64 ();
		for ( auto & pItem : *m_pQueries )
		{
			iRamUse += sizeof ( StoredQuery_t ) + sizeof ( XQQuery_t )
					+ pItem->m_dRejectTerms.GetLengthBytes64()
					+ pItem->m_dRejectWilds.GetLengthBytes64()
					+ pItem->m_dTags.GetLengthBytes64 ()
					+ pItem->m_dFilterTree.GetLengthBytes64 ()
					+ pItem->m_dFilters.GetLengthBytes64()
					+ pItem->m_dSuffixes.GetLengthBytes()
					+ pItem->m_sTags.Length()
					+ pItem->m_sQuery.Length();
			for ( const auto & sSuffix : pItem->m_dSuffixes )
				iRamUse += sSuffix.Length();
		}
	}
	pRes->m_iRamUse = iRamUse;
}

class XQTreeCompressor_t
{
	CSphVector<XQNode_t *> m_dWords;
	CSphVector<XQNode_t *> m_dChildren;

	void WalkNodes ( XQNode_t * pNode )
	{
		if ( !pNode )
			return;

		if ( pNode->m_dWords.GetLength() && pNode->m_dWords.GetLength()!=pNode->m_dWords.GetLimit() )
			m_dWords.Add ( pNode );

		if ( pNode->m_dChildren.GetLength() && pNode->m_dChildren.GetLength()!=pNode->m_dChildren.GetLimit() )
			m_dChildren.Add ( pNode );

		for ( auto & tChild : pNode->m_dChildren )
			WalkNodes ( tChild );
	}

	void Copy ()
	{
		// collect all old vectors then free them at once
		CSphFixedVector< CSphVector<XQKeyword_t> > dWords2Free ( m_dWords.GetLength() );
		CSphFixedVector< CSphVector<XQNode_t *> > dChildren2Free ( m_dChildren.GetLength() );

		for ( int i=0; i<m_dWords.GetLength(); i++ )
		{
			auto & dSrcWords = m_dWords[i]->m_dWords;
			int iLen = dSrcWords.GetLength();
			
			CSphFixedVector<XQKeyword_t> dDstWords ( iLen );
			dDstWords.CopyFrom ( dSrcWords );

			dWords2Free[i].SwapData ( dSrcWords ); // remove all collected vectors m_pData on exit
			dSrcWords.AdoptData ( dDstWords.LeakData(), iLen, iLen );
		}

		for ( int i=0; i<m_dChildren.GetLength(); i++ )
		{
			auto & dSrcChild = m_dChildren[i]->m_dChildren;
			int iLen = dSrcChild.GetLength();
			
			CSphFixedVector<XQNode_t *> dDstChildren ( iLen );
			dDstChildren.CopyFrom ( dSrcChild );
			dSrcChild.Resize ( 0 ); // XQNode_t poionter moved into new fixed-vector

			dChildren2Free[i].SwapData ( dSrcChild ); // remove all collected vectors m_pData on exit
			dSrcChild.AdoptData ( dDstChildren.LeakData(), iLen, iLen );
		}
	}

public:
	void DoWork ( XQNode_t * pNode )
	{
		WalkNodes ( pNode );
		Copy();
	}
};

std::unique_ptr<StoredQuery_i> PercolateIndex_c::CreateQuery ( PercolateQueryArgs_t & tArgs, CSphString & sError )
{
	{
		ScRL_t tLockHash { m_tLock };
		if ( !CanBeAdded ( tArgs, sError ))
			return nullptr;
	}

	bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	TokenizerRefPtr_c pTokenizer = sphCloneAndSetupQueryTokenizer ( m_pTokenizer, IsStarDict ( bWordDict ), m_tSettings.m_bIndexExactWords, false );
	DictRefPtr_c pDict = GetStatelessDict ( m_pDict );

	if ( IsStarDict ( bWordDict ) )
		SetupStarDictV8 ( pDict );

	if ( m_tSettings.m_bIndexExactWords )
		SetupExactDict ( pDict );

	if ( tArgs.m_bQL )
		return CreateQuery ( tArgs, pTokenizer, pDict, sError );

	TokenizerRefPtr_c pTokenizerJson = sphCloneAndSetupQueryTokenizer ( m_pTokenizer, IsStarDict ( bWordDict ), m_tSettings.m_bIndexExactWords, true );
	return CreateQuery ( tArgs, pTokenizerJson, pDict, sError );
}


static std::unique_ptr<QueryParser_i> CreatePlainQueryparser ( bool )
{
	return sphCreatePlainQueryParser();
}

static CreateQueryParser_fn * g_pCreateQueryParser = CreatePlainQueryparser;
void SetPercolateQueryParserFactory ( CreateQueryParser_fn * pCall )
{
	g_pCreateQueryParser = pCall;
}

static void FixExpandedNode ( XQNode_t * pNode )
{
	assert ( pNode );
	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		XQKeyword_t & tKw = pNode->m_dWords[i];
		if ( sphHasExpandableWildcards ( tKw.m_sWord.cstr() ) )
		{
			tKw.m_bExpanded = true;
			// that pointer has not owned by XQKeyword_t and will NOT be deleted
			// however it should be !=nullptr to create ExtPayload_c at ranker
			tKw.m_pPayload = (void *)1;
		}
	}

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		FixExpandedNode ( pNode->m_dChildren[i] );
}

static XQNode_t * FixExpanded ( XQNode_t * pNode, int iMinPrefix, int iMinInfix, bool bExactForm )
{
	ExpansionContext_t tExpCtx;
	tExpCtx.m_iMinPrefixLen = iMinPrefix;
	tExpCtx.m_iMinInfixLen = iMinInfix;
	tExpCtx.m_bHasExactForms = bExactForm;
	tExpCtx.m_bOnlyTreeFix = true;

	pNode = sphExpandXQNode ( pNode, tExpCtx );
	FixExpandedNode ( pNode );

	return pNode;
}

bool PercolateIndex_c::CanBeAdded ( PercolateQueryArgs_t& tArgs, CSphString& sError ) const REQUIRES_SHARED ( m_tLock )
{
	if ( tArgs.m_iQUID )
	{
		if ( tArgs.m_bReplace || !m_hQueries.Find ( tArgs.m_iQUID ) )
			return true;

		sError.SetSprintf ( "duplicate id '" INT64_FMT "'", tArgs.m_iQUID );
		return false;
	}

	int64_t iQUID = 0;
	do
		iQUID = UidShort ();
	while ( m_hQueries.Find( iQUID ) );
	tArgs.m_iQUID = iQUID;
	return true;
}


std::unique_ptr<StoredQuery_i> PercolateIndex_c::CreateQuery ( PercolateQueryArgs_t & tArgs, const TokenizerRefPtr_c& pTokenizer, const DictRefPtr_c& pDict, CSphString & sError )
{
	const char * sQuery = tArgs.m_sQuery;
	CSphVector<BYTE> dFiltered;
	if ( m_pFieldFilter && sQuery && m_pFieldFilter->Clone()->Apply ( sQuery, dFiltered, true ) )
		sQuery = (const char *)dFiltered.Begin();

	auto tParsed = std::make_unique<XQQuery_t>();
	std::unique_ptr<QueryParser_i> tParser = g_pCreateQueryParser ( !tArgs.m_bQL );

	// right tokenizer created at upper level
	if ( !tParser->ParseQuery ( *tParsed, sQuery, nullptr, pTokenizer, pTokenizer, &m_tSchema, pDict, m_tSettings ) )
	{
		sError = tParsed->m_sParseError;
		return nullptr;
	}

	// FIXME!!! provide segments list instead index
	sphTransformExtendedQuery ( &tParsed->m_pRoot, m_tSettings, false, nullptr );

	bool bWordDict = m_pDict->GetSettings().m_bWordDict;
	if ( m_tMutableSettings.m_iExpandKeywords!=KWE_DISABLED )
	{
		sphQueryExpandKeywords ( &tParsed->m_pRoot, m_tSettings, m_tMutableSettings.m_iExpandKeywords, bWordDict );
		tParsed->m_pRoot->Check ( true );
	}

	// this should be after keyword expansion
	TransformAotFilter ( tParsed->m_pRoot, pDict->GetWordforms(), m_tSettings );

	if ( m_tSettings.GetMinPrefixLen ( bWordDict )>0 || m_tSettings.m_iMinInfixLen>0 )
		tParsed->m_pRoot = FixExpanded ( tParsed->m_pRoot, m_tSettings.GetMinPrefixLen ( bWordDict ), m_tSettings.m_iMinInfixLen, ( pDict->HasMorphology () || m_tSettings.m_bIndexExactWords ) );

	// FIXME!!! move whole m_pRoot/pStored->m_pXQ content into arena and use from there to reduce fragmentation
	{
		XQTreeCompressor_t tXQCompressor;
		tXQCompressor.DoWork( tParsed->m_pRoot );
	}

	auto pStored = std::make_unique<StoredQuery_t>();
	pStored->m_pXQ = std::move ( tParsed );
	pStored->m_bOnlyTerms = true;
	pStored->m_sQuery = sQuery;
	QueryGetRejects ( pStored->m_pXQ->m_pRoot, pDict, pStored->m_dRejectTerms, pStored->m_dRejectWilds, pStored->m_dSuffixes, pStored->m_bOnlyTerms, ( m_iMaxCodepointLength>1 ) );
	QueryGetTerms ( pStored->m_pXQ->m_pRoot, pDict, pStored->m_hDict );
	pStored->m_sTags = tArgs.m_sTags;
	PercolateTags ( tArgs.m_sTags, pStored->m_dTags );
	pStored->m_iQUID = tArgs.m_iQUID;
	pStored->m_dFilters.CopyFrom ( tArgs.m_dFilters );
	pStored->m_dFilterTree.CopyFrom ( tArgs.m_dFilterTree );
	pStored->m_bQL = tArgs.m_bQL;
	// need keep m_bEmpty only in case query string is really empty string
	// but use full-text matching path in case query has only out of charset_table chars
	if ( pStored->m_pXQ->m_bEmpty && sQuery )
		pStored->m_pXQ->m_bEmpty = IsEmpty ( FromSz ( sQuery ) );

	return pStored;
}

template<typename READER>
static void LoadInsertDeleteQueries_T ( CSphVector<StoredQueryDesc_t>& dNewQueries, CSphVector<int64_t>& dDeleteQueries, CSphVector<uint64_t>& dDeleteTags, READER& tReader )
{
	dDeleteTags.Resize ( tReader.UnzipInt() );
	ARRAY_FOREACH ( i, dDeleteTags )
		dDeleteTags[i] = tReader.UnzipOffset();

	dDeleteQueries.Resize ( tReader.UnzipInt() );
	ARRAY_FOREACH ( i, dDeleteQueries )
		dDeleteQueries[i] = tReader.UnzipOffset();

	dNewQueries.Resize ( tReader.UnzipInt() );
	ARRAY_FOREACH ( i, dNewQueries )
		LoadStoredQuery ( PQ_META_VERSION_MAX, dNewQueries[i], tReader );
}

static void LoadInsertDeleteQueries ( CSphVector<StoredQueryDesc_t>& dNewQueries, CSphVector<int64_t>& dDeleteQueries, CSphVector<uint64_t>& dDeleteTags, CSphReader& tReader )
{
	LoadInsertDeleteQueries_T ( dNewQueries, dDeleteQueries, dDeleteTags, tReader );
}

template<typename WRITER, typename QUERY>
static void SaveInsertDeleteQueries_T ( const VecTraits_T<QUERY> & dNewQueries, const VecTraits_T<int64_t> & dDeleteQueries, const VecTraits_T<uint64_t> & dDeleteTags, WRITER & tWriter )
{
	tWriter.ZipInt ( dDeleteTags.GetLength() );
	for ( uint64_t uTag : dDeleteTags )
		tWriter.ZipOffset ( uTag );

	tWriter.ZipInt ( dDeleteQueries.GetLength() );
	for ( int64_t iQuery : dDeleteQueries )
		tWriter.ZipOffset ( iQuery );

	tWriter.ZipInt ( dNewQueries.GetLength() );
	for ( StoredQuery_i* pQuery : dNewQueries )
		SaveStoredQuery ( *pQuery, tWriter );
}

template<typename QUERY>
static void SaveInsertDeleteQueries ( const VecTraits_T<QUERY> & dNewQueries, const VecTraits_T<int64_t> & dDeleteQueries, const VecTraits_T<uint64_t> & dDeleteTags, CSphVector<BYTE> & dOut )
{
	MemoryWriter_c tWriter ( dOut );
	SaveInsertDeleteQueries_T ( dNewQueries, dDeleteQueries, dDeleteTags, tWriter );
}

template<typename QUERY>
static void SaveInsertDeleteQueries ( const VecTraits_T<QUERY> & dNewQueries, const VecTraits_T<int64_t> & dDeleteQueries, const VecTraits_T<uint64_t> & dDeleteTags, CSphWriter & tWriter )
{
	SaveInsertDeleteQueries_T ( dNewQueries, dDeleteQueries, dDeleteTags, tWriter );
}

namespace {
// wrap original queries vec, since we might retry more than once.
// Also eliminate dupes, so that last one query with same quid win
CSphVector<StoredQuerySharedPtr_t> UniqAndWrapQueries ( const VecTraits_T<StoredQuery_i*>& dNewQueries )
{
	CSphVector<StoredQuerySharedPtr_t> dNewSharedQueries;
	dNewSharedQueries.Reserve ( dNewQueries.GetLength() );
	OpenHash_T<int, int64_t, HashFunc_Int64_t> hQueries;
	for ( StoredQuery_i* pQuery : dNewQueries )
	{
		int* pIdx = hQueries.Find ( pQuery->m_iQUID );
		StoredQuerySharedPtr_t tNew { (StoredQuery_t*)pQuery };
		if ( !pIdx )
		{
			hQueries.Add ( pQuery->m_iQUID, dNewSharedQueries.GetLength() );
			dNewSharedQueries.Add ( tNew );
		} else
			dNewSharedQueries[*pIdx] = tNew;
	}
	return dNewSharedQueries;
}
} // namespace


int PercolateIndex_c::ReplayInsertAndDeleteQueries ( const VecTraits_T<StoredQuery_i*>& dNewQueries, const VecTraits_T<int64_t>& dDeleteQueries, const VecTraits_T<uint64_t>& dDeleteTags ) EXCLUDES ( m_tLock )
{
	// wrap original queries vec, since we might retry more than once
	auto dNewSharedQueries = UniqAndWrapQueries ( dNewQueries );

	while ( true )
	{
		SharedPQSlice_t dElems;
		int64_t iLimit = -1;

		// will use this slice to actual deletion
		VecTraits_T<int64_t> dAllToDelete = dDeleteQueries;

		// collect deletes by tag
		CSphVector<int64_t> dDeleteIdsAndTags;
		if ( !dDeleteTags.IsEmpty() )
		{
			// for delete by tags we need snapshot of the current queries
			{
				ScRL_t rLock ( m_tLock );
				dElems = GetStoredUnl();
				iLimit = m_pQueries->GetLimit();
			}

			// collect all deletes from tags, to process them all uniform way then
			for ( const StoredQuery_t* pQuery : dElems )
				if ( !pQuery->m_dTags.IsEmpty() && TagsMatched ( dDeleteTags, pQuery->m_dTags ) )
					dDeleteIdsAndTags.Add ( pQuery->m_iQUID );

			if ( !dDeleteIdsAndTags.IsEmpty() )
			{
				dDeleteIdsAndTags.Append ( dDeleteQueries );
				dDeleteIdsAndTags.Uniq();
				dAllToDelete = dDeleteIdsAndTags;
			}
		}

		// for both deletion and addition we need hash and snapshot
		OpenHash_T<int, int64_t, HashFunc_Int64_t> hQueries { 0 };
		{
			ScRL_t rLock ( m_tLock );
			if ( iLimit<0 )
			{
				dElems = GetStoredUnl();
				iLimit = m_pQueries->GetLimit();
			} else if ( dElems.Generation() != m_iGeneration )
				continue;
			hQueries = m_hQueries;
		}

		StoredQuerySharedPtrVecSharedPtr_t pNewVec;
		bool bWithFullClone = false;
		int iDeleted = 0;

		// delete by id pass (deletes by tags are also collected here)
		for ( int64_t iQuery : dAllToDelete )
		{
			auto* pIdx = hQueries.Find ( iQuery );
			if ( !pIdx )
				continue;

			if ( !bWithFullClone ) // first virgin hit, need to make heavy full clone of the queries
			{
				bWithFullClone = true;
				pNewVec = new CSphVector<StoredQuerySharedPtr_t>;
				pNewVec->Reserve ( iLimit );
				for ( auto& iElem : dElems )
					pNewVec->Add ( iElem );
			}
			auto iIdx = *pIdx;
			hQueries.Delete ( iQuery );
			if ( iQuery != pNewVec->Last()->m_iQUID )
				*hQueries.Find ( pNewVec->Last()->m_iQUID ) = iIdx; // fixup to removeFast
			pNewVec->RemoveFast ( iIdx );
			++iDeleted;
		}

		// insert/replace pass

		// check whether we can insert fastest possible way, or need to modify snapshot and only then insert
		if ( !bWithFullClone )
		{
			for ( const auto& pQuery : dNewSharedQueries )
			{
				if ( hQueries.Find ( pQuery->m_iQUID ) && !bWithFullClone )
				{
					bWithFullClone = true;
					pNewVec = new CSphVector<StoredQuerySharedPtr_t>;
					pNewVec->Reserve ( iLimit );
					for ( auto& iElem : dElems )
						pNewVec->Add ( iElem );
					break;
				}
			}
		}

		// perform inserts into clone
		int64_t iNewInserted = 0;
		if ( bWithFullClone )
		{
			for ( auto& pQuery : dNewSharedQueries )
			{
				int* pIdx = hQueries.Find ( pQuery->m_iQUID );
				if ( !pIdx )
				{
					hQueries.Add ( pQuery->m_iQUID, pNewVec->GetLength() );
					pNewVec->Add ( pQuery );
					++iNewInserted;
				} else
					( *pNewVec )[*pIdx] = pQuery;
			}
		}

		ScWL_t wLock ( m_tLock );
		if ( dElems.Generation() != m_iGeneration )
			continue;

		if ( bWithFullClone )
		{
			m_pQueries = pNewVec;
			m_hQueries.Swap ( hQueries );
			++m_iGeneration;
		} else {
			for ( auto& pQuery : dNewSharedQueries )
			{
				assert ( !hQueries.Find ( pQuery->m_iQUID ) );
				AddToStoredUnl ( pQuery );
				++iNewInserted;
			}
		}

		m_tStat.m_iTotalDocuments += iNewInserted - iDeleted;
		Binlog::Commit ( Binlog::PQ_ADD_DELETE, &m_iTID, GetName(), true, [&dNewSharedQueries, dDeleteQueries, dDeleteTags] ( CSphWriter& tWriter ) {
			SaveInsertDeleteQueries ( dNewSharedQueries, dDeleteQueries, dDeleteTags, tWriter );
		} );

		return iDeleted;
	}
}

bool PercolateIndex_c::Commit ( int * pDeleted, RtAccum_t * pAcc, CSphString* )
{
	assert ( g_bRTChangesAllowed );

	if ( !BindAccum ( pAcc ) )
		return true;

	CSphVector<StoredQuery_i*> dNewQueries; // not owned
	CSphVector<int64_t> dDeleteQueries;
	CSphVector<uint64_t> dDeleteTags;

	for ( auto& pCmd : pAcc->m_dCmd )
	{
		switch ( pCmd->m_eCommand )
		{
		case ReplicationCommand_e::PQUERY_ADD:
			dNewQueries.Add ( pCmd->m_pStored.release() );
			break;

		case ReplicationCommand_e::PQUERY_DELETE:
			if ( pCmd->m_dDeleteQueries.GetLength() )
				dDeleteQueries.Append ( pCmd->m_dDeleteQueries );
			else
				PercolateAppendTags ( pCmd->m_sDeleteTags, dDeleteTags );
			break;

		default:
			sphWarning ( "table %s: unsupported command %d", GetName(), (int)pCmd->m_eCommand );
		}
	}

	dDeleteTags.Uniq();
	dDeleteQueries.Uniq();

	int iDeleted = ReplayInsertAndDeleteQueries ( dNewQueries, dDeleteQueries, dDeleteTags );

	pAcc->Cleanup();

	if ( pDeleted )
		*pDeleted = iDeleted;

	return true;
}

Binlog::CheckTnxResult_t PercolateIndex_c::ReplayTxn ( Binlog::Blop_e eOp,CSphReader& tReader, CSphString & sError, Binlog::CheckTxn_fn&& fnCanContinue )
{
	switch ( eOp )
	{
	case Binlog::PQ_ADD: return ReplayAdd(tReader, sError, std::move(fnCanContinue));
	case Binlog::PQ_DELETE: return ReplayDelete(tReader, std::move(fnCanContinue));
	case Binlog::PQ_ADD_DELETE: return ReplayInsertAndDelete(tReader, sError, std::move(fnCanContinue));
	default: assert (false && "unknown op provided to replay");
	}
	return {};
}

Binlog::CheckTnxResult_t PercolateIndex_c::ReplayAdd ( CSphReader& tReader, CSphString & sError, Binlog::CheckTxn_fn&& fnCanContinue )
{
	StoredQueryDesc_t tStored;
	LoadStoredQuery ( PQ_META_VERSION_MAX, tStored, tReader );

	Binlog::CheckTnxResult_t tRes = fnCanContinue();
	if ( tRes.m_bValid && tRes.m_bApply )
	{
		PercolateQueryArgs_t tArgs ( tStored );
		// at binlog query already passed replace checks
		tArgs.m_bReplace = true;

		// actually replay
		auto pQuery = CreateQuery ( tArgs, sError );
		if ( !pQuery )
		{
			sError.SetSprintf ( "apply error, %s", sError.cstr() );
			tRes = Binlog::CheckTnxResult_t();
			return tRes;
		}

		// actually replay
		CSphVector<int64_t> dDeleteQueries;
		CSphVector<uint64_t> dDeleteTags;
		CSphVector<StoredQuery_i*> dNewQueries;
		dNewQueries.Add ( pQuery.release() );
		ReplayInsertAndDeleteQueries ( dNewQueries, dDeleteQueries, dDeleteTags );
	}
	return tRes;
}

Binlog::CheckTnxResult_t PercolateIndex_c::ReplayDelete (CSphReader& tReader, Binlog::CheckTxn_fn&& fnCanContinue)
{
	CSphVector<int64_t> dQueries;
	CSphString sTags;
	LoadDeleteQuery ( dQueries, sTags, tReader );

	Binlog::CheckTnxResult_t tRes = fnCanContinue();
	if ( tRes.m_bValid && tRes.m_bApply )
	{
		CSphVector<StoredQuery_i*> dNewQueries;
		CSphVector<uint64_t> dDeleteTags;
		if ( dQueries.IsEmpty() )
			PercolateAppendTags ( sTags, dDeleteTags );

		// actually replay
		ReplayInsertAndDeleteQueries ( dNewQueries, dQueries, dDeleteTags );
	}
	return tRes;
}

Binlog::CheckTnxResult_t PercolateIndex_c::ReplayInsertAndDelete ( CSphReader& tReader, CSphString& sError, Binlog::CheckTxn_fn&& fnCanContinue )
{
	CSphVector<StoredQueryDesc_t> dNewQueriesDescs;
	CSphVector<int64_t> dDeleteQueries;
	CSphVector<uint64_t> dDeleteTags;

	LoadInsertDeleteQueries( dNewQueriesDescs, dDeleteQueries, dDeleteTags, tReader );

	Binlog::CheckTnxResult_t tRes = fnCanContinue();
	if ( tRes.m_bValid && tRes.m_bApply )
	{
		CSphVector<StoredQuery_i*> dNewQueries; // not owned
		dNewQueries.Reserve ( dNewQueries.GetLength() );

		for ( StoredQueryDesc_t& tDesc : dNewQueriesDescs )
		{
			PercolateQueryArgs_t tArgs ( tDesc );
			// at binlog query already passed replace checks
			tArgs.m_bReplace = true;

			// actually replay
			auto pQuery = CreateQuery ( tArgs, sError );
			if ( !pQuery )
			{
				sError.SetSprintf ( "apply error, %s", sError.cstr() );
				tRes = Binlog::CheckTnxResult_t();
				for ( StoredQuery_i* pDelQuery : dNewQueries )
					SafeDelete ( pDelQuery );
				return tRes;
			}
			dNewQueries.Add ( pQuery.release() );
		}

		// actually replay
		ReplayInsertAndDeleteQueries ( dNewQueries, dDeleteQueries, dDeleteTags );
	}
	return tRes;
}

class PqMatchProcessor_c : public MatchProcessor_i, ISphNoncopyable
{
public:
	PqMatchProcessor_c ( int iTag, const CSphQueryContext & tCtx )
		: m_iTag ( iTag )
		, m_tCtx ( tCtx )
	{}

	bool ProcessInRowIdOrder() const final				{ return false; }
	void Process ( CSphMatch * pMatch ) final			{ ProcessMatch(pMatch); }
	void Process ( VecTraits_T<CSphMatch *> & dMatches ) final { dMatches.for_each ( [this]( CSphMatch * pMatch ){ ProcessMatch(pMatch); } ); }

private:
	int							m_iTag;
	const CSphQueryContext &	m_tCtx;

	inline void ProcessMatch ( CSphMatch * pMatch )
	{
		// fixme! tag is signed int,
		// for distr. tags from remotes set with | 0x80000000,
		// i e in terms of signed int they're <0!
		// Is it intention, or bug?
		// If intention, lt us use uniformely either <0, either &0x80000000
		// conditions to avoid messing. If bug, shit already happened!
		if ( pMatch->m_iTag>=0 )
			return;

		m_tCtx.CalcFinal ( *pMatch );
		pMatch->m_iTag = m_iTag;
	}
};


bool PercolateIndex_c::MultiScan ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs ) const
{
	assert ( tArgs.m_iTag>=0 );
	auto & tMeta = *tResult.m_pMeta;

	QueryProfile_c * pProfiler = tMeta.m_pProfile;

	// we count documents only (before filters)
	if ( tQuery.m_iMaxPredictedMsec )
		tMeta.m_bHasPrediction = true;

	if ( tArgs.m_uPackedFactorFlags & SPH_FACTOR_ENABLE )
		tMeta.m_sWarning.SetSprintf ( "packedfactors() will not work with a fullscan; you need to specify a query" );

	// start counting
	int64_t tmQueryStart = sphMicroTimer ();
	MiniTimer_c dTimerGuard;
	int64_t tmMaxTimer = dTimerGuard.Engage ( tQuery.m_uMaxQueryMsec ); // max_query_time

	// select the sorter with max schema
	// uses GetAttrsCount to get working facets (was GetRowSize)
	int iMaxSchemaIndex = GetMaxSchemaIndexAndMatchCapacity ( dSorters ).first;
	const ISphSchema & tMaxSorterSchema = *( dSorters[iMaxSchemaIndex]->GetSchema ());
	auto dSorterSchemas = SorterSchemas ( dSorters, iMaxSchemaIndex );

	// setup calculations and result schema
	CSphQueryContext tCtx ( tQuery );
	if ( !tCtx.SetupCalc ( tMeta, tMaxSorterSchema, m_tMatchSchema, nullptr, nullptr, dSorterSchemas ) )
		return false;

	// setup filters
	CreateFilterContext_t tFlx;
	tFlx.m_pFilters = &tQuery.m_dFilters;
	tFlx.m_pFilterTree = &tQuery.m_dFilterTree;
	tFlx.m_pSchema = &tMaxSorterSchema;
	tFlx.m_eCollation = tQuery.m_eCollation;
	tFlx.m_bScan = true;

	if ( !tCtx.CreateFilters ( tFlx, tMeta.m_sError, tMeta.m_sWarning ) )
		return false;

	// get all locators
	auto iIDidx = m_tMatchSchema.GetAttrIndex ( sphGetDocidName () );
	const CSphColumnInfo & dID = m_tMatchSchema.GetAttr ( iIDidx );
	const CSphColumnInfo & dColQuery = m_tMatchSchema.GetAttr ( iIDidx+1 );
	const CSphColumnInfo & dColTags = m_tMatchSchema.GetAttr ( iIDidx+2 );
	const CSphColumnInfo & dColFilters = m_tMatchSchema.GetAttr ( iIDidx+3 );
#if PARANOID
	assert ( m_tMatchSchema.GetAttrIndex ( "query" )==iIDidx + 1 );
	assert ( m_tMatchSchema.GetAttrIndex ( "tags" )==iIDidx + 2 );
	assert ( m_tMatchSchema.GetAttrIndex ( "filters" )==iIDidx + 3 );
#endif
	StringBuilder_c sFilters;

	// prepare to work them rows
	bool bRandomize = dSorters[0]->IsRandom();

	CSphMatch tMatch;
	// note: we reserve dynamic area in match using max sorter schema, but then fill it by locators from index schema.
	// that works relying that sorter always includes all attrs from index, leaving final selection of cols
	// to result minimizer. Once we try to pre-optimize sorter schema by select list, it will cause crashes here.
	tMatch.Reset ( tMaxSorterSchema.GetDynamicSize () );
	tMatch.m_iWeight = tArgs.m_iIndexWeight;
	// fixme! tag also used over bitmask | 0x80000000,
	// which marks that match comes from remote.
	// using -1 might be also interpreted as 0xFFFFFFFF in such context!
	// Does it intended?
	tMatch.m_iTag = tCtx.m_dCalcFinal.GetLength () ? -1 : tArgs.m_iTag;

	CSphScopedProfile tProf ( pProfiler, SPH_QSTATE_FULLSCAN );

	int iCutoff = ( tQuery.m_iCutoff<=0 ) ? -1 : tQuery.m_iCutoff;
	BYTE * pData = nullptr;

	CSphVector<PercolateQueryDesc> dQueries;
	auto dStored = GetStored();
	const int64_t& iCheckTimePoint { Threads::Coro::GetNextTimePointUS() };
	Threads::Coro::HighFreqChecker_c fnHeavyCheck;

	for ( const StoredQuery_t * pQuery : dStored )
	{
		tMatch.SetAttr ( dID.m_tLocator, pQuery->m_iQUID );

		int iLen = pQuery->m_sQuery.Length ();
		tMatch.SetAttr ( dColQuery.m_tLocator, (SphAttr_t) sphPackPtrAttr ( iLen, &pData ) );
		memcpy ( pData, pQuery->m_sQuery.cstr (), iLen );

		if ( pQuery->m_sTags.IsEmpty () )
			tMatch.SetAttr ( dColTags.m_tLocator, ( SphAttr_t ) 0 );
		else {
			iLen = pQuery->m_sTags.Length();
			tMatch.SetAttr ( dColTags.m_tLocator, ( SphAttr_t ) sphPackPtrAttr ( iLen, &pData ) );
			memcpy ( pData, pQuery->m_sTags.cstr (), iLen );
		}

		sFilters.Clear ();
		if ( pQuery->m_dFilters.GetLength () )
			FormatFiltersQL ( pQuery->m_dFilters, pQuery->m_dFilterTree, sFilters );
		iLen = sFilters.GetLength ();
		tMatch.SetAttr ( dColFilters.m_tLocator, ( SphAttr_t ) sphPackPtrAttr ( iLen, &pData ) );
		memcpy ( pData, sFilters.cstr (), iLen );


		++tMeta.m_tStats.m_iFetchedDocs;

		tCtx.CalcFilter ( tMatch );
		if ( tCtx.m_pFilter && !tCtx.m_pFilter->Eval ( tMatch ) )
		{
			tCtx.FreeDataFilter ( tMatch );
			m_tMatchSchema.FreeDataPtrs ( tMatch );
			continue;
		}

		if ( bRandomize )
			tMatch.m_iWeight = ( sphRand () & 0xffff ) * tArgs.m_iIndexWeight;

		// submit match to sorters
		tCtx.CalcSort ( tMatch );

		bool bNewMatch = false;
		dSorters.Apply ( [&tMatch, &bNewMatch] ( ISphMatchSorter * p ) { bNewMatch |= p->Push ( tMatch ); } );

		// stringptr expressions should be duplicated (or taken over) at this point
		tCtx.FreeDataFilter ( tMatch );
		tCtx.FreeDataSort ( tMatch );
		m_tMatchSchema.FreeDataPtrs ( tMatch );

		// handle cutoff
		if ( bNewMatch && --iCutoff==0 )
			break;

		// handle timer
		if ( sph::TimeExceeded ( tmMaxTimer ) )
		{
			tMeta.m_sWarning = "query time exceeded max_query_time";
			break;
		}

		if ( fnHeavyCheck() && sph::TimeExceeded ( iCheckTimePoint ) )
		{
			if ( session::GetKilled() )
			{
				tMeta.m_sWarning = "query was killed";
				break;
			}
			Threads::Coro::RescheduleAndKeepCrashQuery();
		}
	}

	SwitchProfile ( pProfiler, SPH_QSTATE_FINALIZE );

	// do final expression calculations
	if ( tCtx.m_dCalcFinal.GetLength () )
	{
		PqMatchProcessor_c tFinal ( tArgs.m_iTag, tCtx );
		dSorters.Apply ( [&] ( ISphMatchSorter * p ) { p->Finalize ( tFinal, false, tArgs.m_bFinalizeSorters ); } );
	}

	tMeta.m_iQueryTime += ( int ) ( ( sphMicroTimer () - tmQueryStart ) / 1000 );

	return true; // fixme! */
}

bool PercolateIndex_c::MultiQuery ( CSphQueryResult & tResult, const CSphQuery & tQuery,
		const VecTraits_T<ISphMatchSorter *> & dAllSorters, const CSphMultiQueryArgs &tArgs ) const
{
	MEMORY ( MEM_DISK_QUERY );

	// to avoid the checking of a ppSorters's element for NULL on every next step, just filter out all nulls right here
	CSphVector<ISphMatchSorter *> dSorters;
	dSorters.Reserve ( dAllSorters.GetLength() );
	dAllSorters.Apply ([&dSorters] ( ISphMatchSorter* p) { if ( p ) dSorters.Add(p); });

	// if we have anything to work with
	if ( dSorters.IsEmpty() )
		return false;

	// non-random at the start, random at the end
	dSorters.Sort ( CmpPSortersByRandom_fn () );

	const QueryParser_i * pQueryParser = tQuery.m_pQueryParser;
	assert ( pQueryParser );

	// fast path for scans
	if ( pQueryParser->IsFullscan ( tQuery ) )
		return MultiScan ( tResult, tQuery, dSorters, tArgs );

	return false;
}

void PercolateIndex_c::PostSetupUnl()
{
	PercolateIndex_i::PostSetup();
	m_iMaxCodepointLength = m_pTokenizer->GetMaxCodepointLength();

	// bigram filter
	if ( m_tSettings.m_eBigramIndex!=SPH_BIGRAM_NONE && m_tSettings.m_eBigramIndex!=SPH_BIGRAM_ALL )
	{
		m_pTokenizer->SetBuffer ( (BYTE*)const_cast<char*> ( m_tSettings.m_sBigramWords.cstr() ), m_tSettings.m_sBigramWords.Length() );

		for ( auto * pTok = m_pTokenizer->GetToken (); pTok; pTok = m_pTokenizer->GetToken () )
			m_tSettings.m_dBigramWords.Add() = (const char*)pTok;

		m_tSettings.m_dBigramWords.Sort();
	}

	// FIXME!!! handle error
	m_pTokenizerIndexing = m_pTokenizer->Clone ( SPH_CLONE_INDEX );
	Tokenizer::AddBigramFilterTo ( m_pTokenizerIndexing, m_tSettings.m_eBigramIndex, m_tSettings.m_sBigramWords, m_sLastError );

	if ( m_tSettings.m_uAotFilterMask )
		sphAotTransformFilter ( m_pTokenizerIndexing, m_pDict, m_tSettings.m_bIndexExactWords, m_tSettings.m_uAotFilterMask );

	// SPZ and zones setup
	if ( ( m_tSettings.m_bIndexSP && !m_pTokenizerIndexing->EnableSentenceIndexing ( m_sLastError ) ) ||
		( !m_tSettings.m_sZones.IsEmpty () && !m_pTokenizerIndexing->EnableZoneIndexing ( m_sLastError )) )
		m_pTokenizerIndexing = nullptr;

	bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	// create queries
	TokenizerRefPtr_c pTokenizer = sphCloneAndSetupQueryTokenizer ( m_pTokenizer, IsStarDict ( bWordDict ), m_tSettings.m_bIndexExactWords, false );
	TokenizerRefPtr_c pTokenizerJson = sphCloneAndSetupQueryTokenizer ( m_pTokenizer, IsStarDict ( bWordDict ), m_tSettings.m_bIndexExactWords, true );

	DictRefPtr_c pDict = GetStatelessDict ( m_pDict );

	if ( IsStarDict ( bWordDict ) )
		SetupStarDictV8 ( pDict );

	if ( m_tSettings.m_bIndexExactWords )
		SetupExactDict ( pDict );

	CSphString sHitlessFiles = m_tSettings.m_sHitlessFiles;
	if ( GetIndexFilenameBuilder() )
	{
		std::unique_ptr<FilenameBuilder_i> pFilenameBuilder = GetIndexFilenameBuilder() ( GetName() );
		if ( pFilenameBuilder )
			sHitlessFiles = pFilenameBuilder->GetFullPath ( sHitlessFiles );
	}

	// hitless
	if ( !LoadHitlessWords ( sHitlessFiles, m_pTokenizerIndexing, m_pDict, m_dHitlessWords, m_sLastError ) )
		sphWarning ( "table '%s': %s", GetName(), m_sLastError.cstr() );

	m_pQueries->ReserveGap( m_dLoadedQueries.GetLength () );

	CSphString sError;
	ARRAY_FOREACH ( i, m_dLoadedQueries )
	{
		const StoredQueryDesc_t & tQuery = m_dLoadedQueries[i];
		const TokenizerRefPtr_c& pTok = tQuery.m_bQL ? pTokenizer : pTokenizerJson;
		PercolateQueryArgs_t tArgs ( tQuery );
		if ( CanBeAdded ( tArgs, sError ) )
		{
			auto pQuery = CreateQuery ( tArgs, pTok, pDict, sError );
			if ( pQuery )
			{
				// as a new (not replace), query it will be anyway added to the tail.
				// so, we may reserve ref in the hash, and then occupy it with the query.
				assert ( !tArgs.m_bReplace );
				AddToStoredUnl ( StoredQuerySharedPtr_t ((StoredQuery_t *) pQuery.release() ));
				continue;
			}
		}
		sphWarning ( "table '%s': %d (id=" INT64_FMT ") query failed to load, ignoring", GetName(), i, tQuery.m_iQUID );
	}
	m_dLoadedQueries.Reset ( 0 );

	// still need index files for index just created from config
	if ( !m_bHasFiles )
		SaveMeta ( SharedPQSlice_t ( m_pQueries ) );

}

void PercolateIndex_c::PostSetup () EXCLUDES ( m_tLock )
{
	ScWL_t wLock ( m_tLock );
	PostSetupUnl();
}

// load old-style (legacy) binary meta
PercolateIndex_c::LOAD_E PercolateIndex_c::LoadMetaLegacy ( const CSphString& sMeta, bool bStripPath, FilenameBuilder_i* pFilenameBuilder, StrVec_t& dWarnings )
{
	/////////////
	// load meta
	/////////////

	m_sLastError = "";

	// opened and locked, lets read
	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMeta, m_sLastError ) )
		return LOAD_E::GeneralError_e;

	if ( rdMeta.GetDword() != META_HEADER_MAGIC )
	{
		m_sLastError.SetSprintf ( "invalid meta file %s", sMeta.cstr() );
		return LOAD_E::ParseError_e;
	}
	DWORD uVersion = rdMeta.GetDword();
	if ( uVersion == 0 || uVersion > META_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%u, binary is v.%u", sMeta.cstr(), uVersion, META_VERSION );
		return LOAD_E::GeneralError_e;
	}

	// we don't support anything prior to v8
	DWORD uMinFormatVer = 8;
	if ( uVersion < uMinFormatVer )
	{
		m_sLastError.SetSprintf ( "tables prior to v.%u are no longer supported (use index_converter tool); %s is v.%u", uMinFormatVer, GetFilebase(), uVersion );
		return LOAD_E::GeneralError_e;
	}

	DWORD uIndexVersion = rdMeta.GetDword();

	CSphTokenizerSettings tTokenizerSettings;
	CSphDictSettings tDictSettings;
	CSphEmbeddedFiles tEmbeddedFiles;

	// load settings
	ReadSchema ( rdMeta, m_tSchema, uIndexVersion );
	LoadIndexSettings ( m_tSettings, rdMeta, uIndexVersion );
	if ( !tTokenizerSettings.Load ( pFilenameBuilder, rdMeta, tEmbeddedFiles, m_sLastError ) )
		return LOAD_E::GeneralError_e;

	tDictSettings.Load ( rdMeta, tEmbeddedFiles, m_sLastWarning );

	// initialize AOT if needed
	DWORD uPrevAot = m_tSettings.m_uAotFilterMask;
	m_tSettings.m_uAotFilterMask = sphParseMorphAot ( tDictSettings.m_sMorphology.cstr() );
	if ( m_tSettings.m_uAotFilterMask!=uPrevAot )
		sphWarning ( "table '%s': morphology option changed from config has no effect, ignoring", GetName() );

	if ( bStripPath )
	{
		StripPath ( tTokenizerSettings.m_sSynonymsFile );
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
			StripPath ( tDictSettings.m_dWordforms[i] );
	}

	// recreate tokenizer
	m_pTokenizer = Tokenizer::Create ( tTokenizerSettings, &tEmbeddedFiles, pFilenameBuilder, dWarnings, m_sLastError );
	if ( !m_pTokenizer )
		return LOAD_E::GeneralError_e;

	// recreate dictionary
	m_pDict = sphCreateDictionaryCRC ( tDictSettings, &tEmbeddedFiles, m_pTokenizer, GetName(), bStripPath, m_tSettings.m_iSkiplistBlockSize, pFilenameBuilder, m_sLastError );
	if ( !m_pDict )
	{
		m_sLastError.SetSprintf ( "table '%s': %s", GetName(), m_sLastError.cstr() );
		return LOAD_E::GeneralError_e;
	}

	Tokenizer::AddToMultiformFilterTo ( m_pTokenizer, m_pDict->GetMultiWordforms () );

	// regexp and ICU
	if ( uVersion>=6 )
	{
		std::unique_ptr<ISphFieldFilter> pFieldFilter;
		CSphFieldFilterSettings tFieldFilterSettings;
		tFieldFilterSettings.Load(rdMeta);
		if ( tFieldFilterSettings.m_dRegexps.GetLength() )
			pFieldFilter = sphCreateRegexpFilter ( tFieldFilterSettings, m_sLastError );

		if ( !sphSpawnFilterICU ( pFieldFilter, m_tSettings, tTokenizerSettings, sMeta.cstr(), m_sLastError ) )
			return LOAD_E::GeneralError_e;

		SetFieldFilter ( std::move ( pFieldFilter ) );
	}

	// queries
	DWORD uQueries = rdMeta.GetDword();
	m_dLoadedQueries.Reset ( uQueries );
	if ( uVersion<7)
	{
		for ( auto& tQuery : m_dLoadedQueries )
			LoadStoredQueryV6 ( uVersion, tQuery, rdMeta);
	} else
	{
		for ( auto & tQuery : m_dLoadedQueries )
			LoadStoredQuery ( uVersion, tQuery, rdMeta );
		m_iTID = rdMeta.GetOffset ();
	}

	m_tStat.m_iTotalDocuments = uQueries;
	m_iSavedTID = m_iTID;
	return LOAD_E::Ok_e;
}

void LoadStoredQueryJson ( StoredQueryDesc_t& tQuery, const bson::Bson_c& tNode );

// load new (json) meta
PercolateIndex_c::LOAD_E PercolateIndex_c::LoadMetaJson ( const CSphString& sMeta, bool bStripPath, FilenameBuilder_i* pFilenameBuilder, StrVec_t& dWarnings )
{
	using namespace bson;

	CSphVector<BYTE> dData;
	if ( !sphJsonParse ( dData, sMeta, m_sLastError ) )
		return LOAD_E::ParseError_e;

	Bson_c tBson ( dData );
	if ( tBson.IsEmpty() || !tBson.IsAssoc() )
	{
		m_sLastError = "Something wrong read from json meta - it is either empty, either not root object.";
		return LOAD_E::ParseError_e;
	}

	// version
	DWORD uVersion = (DWORD)Int ( tBson.ChildByName ( "meta_version" ), 9 );
	if ( uVersion == 0 || uVersion > META_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%u, binary is v.%u", sMeta.cstr(), uVersion, META_VERSION );
		return LOAD_E::GeneralError_e;
	}

	// we don't support anything prior to v8
	DWORD uMinFormatVer = 9;
	if ( uVersion < uMinFormatVer )
	{
		m_sLastError.SetSprintf ( "tables prior to v.%u are no longer supported (use index_converter tool); %s is v.%u", uMinFormatVer, GetFilebase(), uVersion );
		return LOAD_E::GeneralError_e;
	}

//	DWORD uIndexVersion = (DWORD)Int ( tBson.ChildByName ( "index_format_version" ) );

	CSphTokenizerSettings tTokenizerSettings;
	CSphDictSettings tDictSettings;
	CSphEmbeddedFiles tEmbeddedFiles;

	// load settings
	ReadSchemaJson ( tBson.ChildByName ( "schema" ), m_tSchema );
	LoadIndexSettingsJson ( tBson.ChildByName ( "index_settings" ), m_tSettings );

	if ( !tTokenizerSettings.Load ( pFilenameBuilder, tBson.ChildByName ( "tokenizer_settings"), tEmbeddedFiles, m_sLastError ) )
		return LOAD_E::GeneralError_e;

	tDictSettings.Load ( tBson.ChildByName ( "dictionary_settings" ), tEmbeddedFiles, m_sLastWarning );

	// initialize AOT if needed
	DWORD uPrevAot = m_tSettings.m_uAotFilterMask;
	m_tSettings.m_uAotFilterMask = sphParseMorphAot ( tDictSettings.m_sMorphology.cstr() );
	if ( m_tSettings.m_uAotFilterMask != uPrevAot )
		sphWarning ( "table '%s': morphology option changed from config has no effect, ignoring", GetName() );

	if ( bStripPath )
	{
		StripPath ( tTokenizerSettings.m_sSynonymsFile );
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
			StripPath ( tDictSettings.m_dWordforms[i] );
	}

	// recreate tokenizer
	m_pTokenizer = Tokenizer::Create ( tTokenizerSettings, &tEmbeddedFiles, pFilenameBuilder, dWarnings, m_sLastError );
	if ( !m_pTokenizer )
		return LOAD_E::GeneralError_e;

	// recreate dictionary
	m_pDict = sphCreateDictionaryCRC ( tDictSettings, &tEmbeddedFiles, m_pTokenizer, GetName(), bStripPath, m_tSettings.m_iSkiplistBlockSize, pFilenameBuilder, m_sLastError );
	if ( !m_pDict )
	{
		m_sLastError.SetSprintf ( "table '%s': %s", GetName(), m_sLastError.cstr() );
		return LOAD_E::GeneralError_e;
	}

	Tokenizer::AddToMultiformFilterTo ( m_pTokenizer, m_pDict->GetMultiWordforms() );

	// regexp and ICU
	std::unique_ptr<ISphFieldFilter> pFieldFilter;
	auto tFieldFilterSettingsNode = tBson.ChildByName ( "field_filter_settings" );
	if ( !IsNullNode ( tFieldFilterSettingsNode ) )
	{
		CSphFieldFilterSettings tFieldFilterSettings;
		Bson_c ( tFieldFilterSettingsNode ).ForEach ( [&tFieldFilterSettings] ( const NodeHandle_t& tNode ) {
			tFieldFilterSettings.m_dRegexps.Add ( String ( tNode ) );
		} );

		if ( !tFieldFilterSettings.m_dRegexps.IsEmpty() )
			pFieldFilter = sphCreateRegexpFilter ( tFieldFilterSettings, m_sLastError );
	}

	if ( !sphSpawnFilterICU ( pFieldFilter, m_tSettings, tTokenizerSettings, sMeta.cstr(), m_sLastError ) )
		return LOAD_E::GeneralError_e;

	SetFieldFilter ( std::move ( pFieldFilter ) );

	m_iTID = Int ( tBson.ChildByName ( "tid" ) );

	// queries
	auto tQueriesNode = tBson.ChildByName ( "pqs" );
	if ( !IsNullNode( tQueriesNode) )
	{
		Bson_c tQueriesVec { tQueriesNode };
		m_dLoadedQueries.Reset ( tQueriesVec.CountValues() );
		int iLastQ = 0;
		tQueriesVec.ForEach ( [&iLastQ,this] ( const NodeHandle_t& tNode ) {
			LoadStoredQueryJson ( m_dLoadedQueries[iLastQ++], tNode );
		} );
	}

	m_tStat.m_iTotalDocuments = m_dLoadedQueries.GetLength();
	m_iSavedTID = m_iTID;
	return LOAD_E::Ok_e;
}

bool PercolateIndex_c::LoadMetaImpl ( const CSphString& sMeta, bool bStripPath, FilenameBuilder_i* pFilenameBuilder, StrVec_t& dWarnings )
{
	auto eRes = LoadMetaJson ( sMeta, bStripPath, pFilenameBuilder, dWarnings );
	if ( eRes == LOAD_E::ParseError_e )
	{
		sphInfo ( "Index meta format is not json, will try it as binary..." );
		eRes = LoadMetaLegacy ( sMeta, bStripPath, pFilenameBuilder, dWarnings );
		if ( eRes == LOAD_E::ParseError_e )
		{
			sphWarning ( "Unable to parse header... Error %s", m_sLastError.cstr() );
			return false;
		}
	}
	if ( eRes == LOAD_E::GeneralError_e )
	{
		sphWarning ( "Unable to load header... Error %s", m_sLastError.cstr() );
		return false;
	}

	assert ( eRes == LOAD_E::Ok_e );
	return true;
}

bool PercolateIndex_c::LoadMeta ( const CSphString& sMeta, bool bStripPath, FilenameBuilder_i* pFilenameBuilder, StrVec_t& dWarnings )
{
	if ( LoadMetaImpl ( sMeta, bStripPath, pFilenameBuilder, dWarnings ) )
		return true;

	const char* szDumpPath = getenv ( "dump_corrupt_meta" );
	if ( !szDumpPath )
		return false;

	CSphString sDestPath = SphSprintf("%s%s",szDumpPath,"index.meta");
	CSphString sError;
	if ( !CopyFile ( sMeta, sDestPath, sError ) )
		sphWarning ( "%s", sError.cstr() );
	return false;
}


bool PercolateIndex_c::Prealloc ( bool bStripPath, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings )
{
	CSphString sLock = GetFilename ( "lock" ); // notice: .lock vs .spl
	m_iLockFD = ::open ( sLock.cstr(), SPH_O_NEW, 0644 );
	if ( m_iLockFD < 0 )
	{
		m_sLastError.SetSprintf ( "failed to open %s: %s", sLock.cstr(), strerrorm( errno ) );
		return false;
	}
	if ( !sphLockEx ( m_iLockFD, false ) )
	{
		m_sLastError.SetSprintf ( "failed to lock %s: %s", sLock.cstr(), strerrorm( errno ) );
		::close ( m_iLockFD );
		return false;
	}

	CSphString sMeta = GetFilename ( "meta" );

	// no readable meta? no disk part yet
	if ( !sphIsReadable ( sMeta.cstr() ) )
		return true;

	m_bHasFiles = true;

	if ( !LoadMeta ( sMeta, bStripPath, pFilenameBuilder, dWarnings ) )
		return false;

	CSphString sMutableFile = GetFilename ( SPH_EXT_SETTINGS );
	if ( !m_tMutableSettings.Load ( sMutableFile.cstr(), GetName() ) )
		return false;

	m_tmSaved = sphMicroTimer();
	return true;
}

void operator<< ( JsonEscapedBuilder& tOut, const StoredQueryDesc_t& tQuery );

void PercolateIndex_c::SaveMeta ( const SharedPQSlice_t& dStored, bool bShutdown )
{
	// sanity check
	if ( m_iLockFD<0 || m_bSaveDisabled )
		return;

	// write new meta
	CSphString sMeta = GetFilename("meta");
	CSphString sMetaNew = GetFilename ( "meta.new" );

	CSphString sError;
	JsonEscapedBuilder sNewMeta;
	sNewMeta.ObjectWBlock();

	// human-readable sugar
	sNewMeta.NamedString ( "meta_created_time_utc", sphCurrentUtcTime() );
	sNewMeta.NamedVal ( "meta_version", META_VERSION );
	sNewMeta.NamedVal ( "index_format_version", INDEX_FORMAT_VERSION );

	sNewMeta.NamedVal ( "schema", m_tSchema );
	sNewMeta.NamedVal ( "index_settings", m_tSettings );
	sNewMeta.Named ( "tokenizer_settings" );
	SaveTokenizerSettings ( sNewMeta, m_pTokenizer, m_tSettings.m_iEmbeddedLimit );
	sNewMeta.Named ( "dictionary_settings" );
	SaveDictionarySettings ( sNewMeta, m_pDict, false, m_tSettings.m_iEmbeddedLimit );

	// meta v.6
	CSphFieldFilterSettings tFieldFilterSettings;
	if ( m_pFieldFilter )
		m_pFieldFilter->GetSettings(tFieldFilterSettings);
	sNewMeta.NamedVal ( "field_filter_settings", tFieldFilterSettings );
	sNewMeta.NamedVal ( "tid", m_iTID );

	{
		sNewMeta.Named ( "pqs" );
		auto _ = sNewMeta.ArrayW();
		for ( const StoredQuery_t * pQuery : dStored )
			sNewMeta << *pQuery;
	}

	Binlog::NotifyIndexFlush ( GetName(), m_iTID, bShutdown );

	m_iSavedTID = m_iTID;
	m_tmSaved = sphMicroTimer();

	sNewMeta.FinishBlocks();
	CSphWriter wrMetaJson;
	if ( wrMetaJson.OpenFile ( sMetaNew, sError ) )
	{
		wrMetaJson.PutString ( (Str_t)sNewMeta );
		wrMetaJson.CloseFile();
		assert ( bson::ValidateJson ( sNewMeta.cstr(), &sError ) );
	} else {
		sphWarning ( "failed to serialize meta: %s", sError.cstr() );
		return;
	}

	// rename
	if ( sph::rename ( sMetaNew.cstr(), sMeta.cstr() ) )
		sphWarning ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)", sMetaNew.cstr(), sMeta.cstr(), errno, strerrorm( errno ) );

	SaveMutableSettings ( m_tMutableSettings, GetFilename ( SPH_EXT_SETTINGS ) );
}


void PercolateIndex_c::SaveMeta ( bool bShutdown )
{
	SaveMeta ( GetStored(), bShutdown );
}

bool PercolateIndex_c::Truncate ( CSphString & sError )
{
	ScWL_t wLock ( m_tLock );

	m_hQueries.Reset ( 256 );
	m_pQueries = new CSphVector<StoredQuerySharedPtr_t>;

	// update and save meta
	// current TID will be saved, so replay will properly skip preceding txns
	// FIXME!!! however it should be replicated to cluster maybe with TOI
	SaveMeta ( SharedPQSlice_t ( m_pQueries ) );

	return true;
}

void PercolateMatchResult_t::Reset ()
{
	m_bGetDocs = false;
	m_bGetQuery = false;
	m_bGetFilters = true;

	m_iQueriesMatched = 0;
	m_iQueriesFailed = 0;
	m_iDocsMatched = 0;
	m_tmTotal = 0;

	m_bVerbose = false;
	m_iEarlyOutQueries = 0;
	m_iTotalQueries = 0;
	m_iOnlyTerms = 0;
	m_tmSetup = 0;
	m_sMessages.Clear ();

	m_dQueryDesc.Reset ( 0 );
	m_dDocs.Reset ( 0 );
	m_dQueryDT.Reset ( 0 );

}

PercolateMatchResult_t::PercolateMatchResult_t ( PercolateMatchResult_t&& rhs ) noexcept
	: PercolateMatchResult_t()
{
	Swap (rhs);
}

void PercolateMatchResult_t::Swap ( PercolateMatchResult_t& rhs ) noexcept
{
	::Swap ( m_bGetDocs, rhs.m_bGetDocs );
	::Swap ( m_bGetQuery, rhs.m_bGetQuery );
	::Swap ( m_bGetFilters, rhs.m_bGetFilters );

	::Swap ( m_iQueriesMatched, rhs.m_iQueriesMatched );
	::Swap ( m_iQueriesFailed, rhs.m_iQueriesFailed );
	::Swap ( m_iDocsMatched, rhs.m_iDocsMatched );
	::Swap ( m_tmTotal, rhs.m_tmTotal );

	::Swap ( m_bVerbose, rhs.m_bVerbose );
	::Swap ( m_iEarlyOutQueries, rhs.m_iEarlyOutQueries );
	::Swap ( m_iTotalQueries, rhs.m_iTotalQueries );
	::Swap ( m_iOnlyTerms, rhs.m_iOnlyTerms );
	::Swap ( m_tmSetup, rhs.m_tmSetup );
	::Swap ( m_sMessages, rhs.m_sMessages );

	::Swap ( m_dQueryDesc, rhs.m_dQueryDesc );
	::Swap ( m_dDocs, rhs.m_dDocs );
	::Swap ( m_dQueryDT, rhs.m_dQueryDT );
}

PercolateMatchResult_t&PercolateMatchResult_t::operator= ( PercolateMatchResult_t rhs ) noexcept
{
	Swap ( rhs );
	return *this;
}


void FixPercolateSchema ( CSphSchema & tSchema )
{
	if ( !tSchema.GetFieldsCount() )
		tSchema.AddField ( CSphColumnInfo ( "text" ) );
}

bool PercolateIndex_c::IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, StrVec_t & dWarnings, CSphString & sError ) const
{
	tSetup.m_tSchema = tSettings.m_tSchema;
	FixPercolateSchema ( tSetup.m_tSchema );

	CSphString sTmp;
	bool bSameSchema = m_tSchema.CompareTo ( tSettings.m_tSchema, sTmp, false );

	return CreateReconfigure ( GetName(), IsStarDict ( m_pDict->GetSettings().m_bWordDict ), m_pFieldFilter.get(), m_tSettings, m_pTokenizer->GetSettingsFNV(),
		  m_pDict->GetSettingsFNV(), m_pTokenizer->GetMaxCodepointLength(), GetMemLimit(),
		  bSameSchema, tSettings, tSetup, dWarnings, sError );
}

// fixme? Retire this, then SaveIndexSettings/SaveTokenizersettings/SaveDictionarySettings for plain CSphWriter, only json left
void PercolateIndex_c::BinlogReconfigure ( CSphReconfigureSetup & tSetup )
{
	Binlog::Commit(Binlog::RECONFIGURE,&m_iTID,GetName(),false,[&tSetup] (CSphWriter& tWriter) {
		// reconfigure data
		SaveIndexSettings ( tWriter, tSetup.m_tIndex );
		SaveTokenizerSettings ( tWriter, tSetup.m_pTokenizer, 0 );
		SaveDictionarySettings ( tWriter, tSetup.m_pDict, false, 0 );

		CSphFieldFilterSettings tFieldFilterSettings;
		if ( tSetup.m_pFieldFilter )
			tSetup.m_pFieldFilter->GetSettings(tFieldFilterSettings);
		tFieldFilterSettings.Save(tWriter);
	});
}

bool PercolateIndex_c::Reconfigure ( CSphReconfigureSetup & tSetup )
{
	BinlogReconfigure ( tSetup );

	m_tSchema = tSetup.m_tSchema;

	Setup ( tSetup.m_tIndex );
	SetTokenizer ( tSetup.m_pTokenizer );
	SetDictionary ( tSetup.m_pDict );
	SetFieldFilter ( std::move ( tSetup.m_pFieldFilter ) );

	m_iMaxCodepointLength = m_pTokenizer->GetMaxCodepointLength();
	SetupQueryTokenizer();

	ScWL_t wLock ( m_tLock ); // ensure nothing will be changed during reconfigure pass.

	m_dLoadedQueries.Reset ( m_pQueries->GetLength() );
	ARRAY_FOREACH ( i, m_dLoadedQueries )
	{
		StoredQueryDesc_t & tQuery = m_dLoadedQueries[i];
		const StoredQuery_t * pStored = (*m_pQueries) [i];

		tQuery.m_iQUID = pStored->m_iQUID;
		tQuery.m_sQuery = pStored->m_sQuery;
		tQuery.m_sTags = pStored->m_sTags;
		tQuery.m_dFilters.CopyFrom ( pStored->m_dFilters );
		tQuery.m_dFilterTree.CopyFrom ( pStored->m_dFilterTree );
	}

	m_pQueries = new CSphVector<StoredQuerySharedPtr_t>;
	m_hQueries.Clear();

	// note: m_tLockHash and m_tLock is still held here.
	PostSetupUnl();
	return true;
}

bool PercolateIndex_c::IsFlushNeed() const
{
	// m_iTID get managed by binlog that is why wo binlog there is no need to compare it
	if ( Binlog::IsActive() && m_iTID<=m_iSavedTID )
		return false;

	return !m_bSaveDisabled;

}

void PercolateIndex_c::ForceRamFlush ( const char * szReason )
{
	if ( !IsFlushNeed() )
		return;

	int64_t tmStart = sphMicroTimer();
	int64_t iWasTID = m_iSavedTID;
	int64_t tmWas = m_tmSaved;
	SaveMeta ();

	int64_t tmNow = sphMicroTimer();
	int64_t tmAge = tmNow - tmWas;
	int64_t tmSave = tmNow - tmStart;

	sphInfo ( "percolate: table %s: saved ok (mode=%s, last TID=" INT64_FMT ", current TID=" INT64_FMT ", "
		"time delta=%d sec, took=%d.%03d sec)", GetName(), szReason, iWasTID, m_iTID, (int) (tmAge/1000000), (int)(tmSave/1000000), (int)((tmSave/1000)%1000) );
}

bool PercolateIndex_c::ForceDiskChunk()
{
	ForceRamFlush ( "forced" );
	return true;
}

void PercolateIndex_c::LockFileState ( StrVec_t & dFiles )
{
	ForceRamFlush ( "forced" );
	m_bSaveDisabled = true;

	GetIndexFiles ( dFiles, dFiles );
}


PercolateQueryArgs_t::PercolateQueryArgs_t ( const VecTraits_T<CSphFilterSettings> & dFilters, const VecTraits_T<FilterTreeItem_t> & dFilterTree )
	: m_dFilters ( dFilters )
	, m_dFilterTree ( dFilterTree )
{}

PercolateQueryArgs_t::PercolateQueryArgs_t ( const StoredQueryDesc_t & tDesc )
	: PercolateQueryArgs_t ( tDesc.m_dFilters, tDesc.m_dFilterTree )
{
	m_sQuery = tDesc.m_sQuery.cstr();
	m_sTags = tDesc.m_sTags.cstr();
	m_iQUID = tDesc.m_iQUID;
	m_bQL = tDesc.m_bQL;
}

// stuff for merging several results into one
struct PQMergeResultsIterator_t
{
	CPqResult * m_pResult = nullptr;
	int m_iIdx = 0;
	int m_iElems = 0;
	int* m_pDocs = nullptr;

	explicit PQMergeResultsIterator_t ( CPqResult * pMatch = nullptr )
		: m_pResult ( pMatch )
	{
		if ( !pMatch )
			return;
		m_iElems = pMatch->m_dResult.m_dQueryDesc.GetLength ();
		m_pDocs = pMatch->m_dResult.m_dDocs.begin();
	}

	inline PercolateQueryDesc & CurDesc () const
	{ return m_pResult->m_dResult.m_dQueryDesc[m_iIdx]; }

	inline int CurDt () const
	{ return m_pResult->m_dResult.m_dQueryDT[m_iIdx]; }

	static inline bool IsLess ( const PQMergeResultsIterator_t &a, const PQMergeResultsIterator_t &b )
	{
		return a.CurDesc().m_iQUID<b.CurDesc().m_iQUID;
	}
};

void MergePqResults ( const VecTraits_T<CPqResult *> &dChunks, CPqResult &dRes, bool bSharded )
{
	if ( dChunks.IsEmpty () )
		return;

	// check if we have exactly one non-null and non-empty result
	if ( dChunks.GetLength ()==1 ) // short path for only 1 result.
	{
		auto dOldMsgs = std::move(dRes.m_dResult.m_sMessages);
		dRes = std::move ( *dChunks[0] );
		dRes.m_dResult.m_sMessages.AddStringsFrom ( dOldMsgs );
		return;
	}

	assert ( dChunks.GetLength ()>1 ); // simplest cases must be already processed;

	int iGotQueries = 0;
	int iGotDocids = 0;
	auto & dFinal = dRes.m_dResult; // shortcut

	CSphQueue<PQMergeResultsIterator_t, PQMergeResultsIterator_t> qMatches ( dChunks.GetLength () );
	for ( CPqResult * pChunk : dChunks )
	{
		assert ( pChunk ); // no nulls allowed

		auto &dResult = pChunk->m_dResult; /// shortcut

		// collect all warnings/errors/other things despite the num of collected results
		dFinal.m_sMessages.AddStringsFrom ( dResult.m_sMessages );
		dFinal.m_iTotalQueries = dResult.m_iTotalQueries + ( bSharded ? 0 : dFinal.m_iTotalQueries );
		dFinal.m_iEarlyOutQueries += dResult.m_iEarlyOutQueries;
		dFinal.m_iQueriesFailed += dResult.m_iQueriesFailed;
		dFinal.m_iDocsMatched += dResult.m_iDocsMatched;
		dFinal.m_tmTotal += dResult.m_tmTotal;
		dFinal.m_tmSetup += dResult.m_tmSetup;

		dFinal.m_bVerbose |= !dResult.m_dQueryDT.IsEmpty();
		dFinal.m_bGetDocs |= !dResult.m_dDocs.IsEmpty();
		dFinal.m_bGetQuery = dResult.m_bGetQuery;

		// we interest only in filled results
		if ( dResult.m_dQueryDesc.IsEmpty () )
			continue;

		qMatches.Push ( PQMergeResultsIterator_t ( pChunk ));
		iGotQueries += dResult.m_dQueryDesc.GetLength ();
		iGotDocids += pChunk->m_dDocids.GetLength();
	}

	if ( !iGotQueries )
		return;

	dFinal.m_iQueriesMatched = iGotQueries;
	int iDocSpace = iGotQueries + dFinal.m_iDocsMatched;

	dFinal.m_dQueryDesc.Reset ( iGotQueries );
	PercolateQueryDesc * pDst = dFinal.m_dQueryDesc.Begin ();

	int * pDt = nullptr;
	if ( dFinal.m_bVerbose )
	{
		dFinal.m_dQueryDT.Reset ( iGotQueries );
		pDt = dFinal.m_dQueryDT.begin ();
		assert ( pDt );
	}

	int * pDocs = nullptr;
	if ( dFinal.m_bGetDocs )
	{
		dFinal.m_dDocs.Reset ( iDocSpace );
		pDocs = dFinal.m_dDocs.begin ();
		assert ( pDocs );
	}

	OpenHash_T<int, int64_t, HashFunc_Int64_t> hDocids ( iGotDocids + 1 );
	bool bHasDocids = iGotDocids!=0;
	if ( bHasDocids )
		hDocids.Add ( iGotDocids, 0 );

	PQMergeResultsIterator_t tMin = qMatches.Root ();
	qMatches.Pop ();

	assert ( qMatches.GetLength ()>=0 ); // since case of only 1 resultset we already processed.

	while ( true )
	{
		pDst->Swap ( tMin.CurDesc () );
		++pDst;

		if ( dFinal.m_bVerbose )
			*pDt++ = tMin.CurDt ();

		// docs copy
		if ( dFinal.m_bGetDocs )
		{
			int iDocCount = *tMin.m_pDocs;

			// in case of docids we collect them into common hash and rewrite results with hashed values
			if ( bHasDocids )
			{
				*pDocs = iDocCount;
				for ( int i=1; i<iDocCount+1; ++i )
					pDocs[i] = hDocids.FindOrAdd ( tMin.m_pResult->m_dDocids[tMin.m_pDocs[i]], (int)hDocids.GetLength () );
			} else
				memcpy ( pDocs, tMin.m_pDocs, sizeof(int) * (iDocCount + 1) );

			tMin.m_pDocs += iDocCount + 1;
			pDocs += iDocCount + 1;
		}

		++tMin.m_iIdx;
		if ( tMin.m_iIdx<tMin.m_iElems )
		{
			// if current root is better - change the head.
			if ( qMatches.GetLength () && !PQMergeResultsIterator_t::IsLess ( tMin, qMatches.Root () ) )
				qMatches.Push ( tMin );
			else
				continue;
		}

		if ( !qMatches.GetLength () )
			break;

		tMin = qMatches.Root ();
		qMatches.Pop ();
	}

	// repack hash into vec (if necessary)
	if ( bHasDocids )
	{
		dRes.m_dDocids.Reset ( hDocids.GetLength() );
		int64_t i = 0;
		int64_t iDocid = 0;
		int * pIndex = nullptr;
		while ( nullptr != ( pIndex = hDocids.Iterate ( &i, &iDocid ) ) )
			dRes.m_dDocids[*pIndex] = iDocid;

	}
}

void PercolateIndex_c::GetIndexFiles ( StrVec_t& dFiles, StrVec_t& dExtra, const FilenameBuilder_i* pParentFilenamebuilder ) const
{
	CSphString sPath = GetFilename("meta");
	if ( sphIsReadable ( sPath ) )
		dFiles.Add ( sPath );

	if ( m_tMutableSettings.NeedSave() ) // should be file already after post-setup
	{
		sPath = GetFilename ( SPH_EXT_SETTINGS );
		if ( sphIsReadable ( sPath ) )
			dFiles.Add ( sPath );
	}

	std::unique_ptr<FilenameBuilder_i> pFilenameBuilder { nullptr };
	if ( !pParentFilenamebuilder && GetIndexFilenameBuilder() )
	{
		pFilenameBuilder = GetIndexFilenameBuilder() ( GetName() );
		pParentFilenamebuilder = pFilenameBuilder.get();
	}
	GetSettingsFiles ( m_pTokenizer, m_pDict, GetSettings(), pParentFilenamebuilder, dExtra );
}

Bson_t PercolateIndex_c::ExplainQuery ( const CSphString & sQuery ) const
{
	WordlistStub_c tWordlist;

	bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	TokenizerRefPtr_c pQueryTokenizer = sphCloneAndSetupQueryTokenizer ( m_pTokenizer, IsStarDict ( bWordDict ), m_tSettings.m_bIndexExactWords, false );
	SetupExactTokenizer ( pQueryTokenizer );
	SetupStarTokenizer( pQueryTokenizer );

	ExplainQueryArgs_t tArgs;
	tArgs.m_szQuery = sQuery.cstr();
	tArgs.m_pSchema = &GetInternalSchema();
	tArgs.m_pDict = GetStatelessDict ( m_pDict );
	SetupStarDictV8 ( tArgs.m_pDict );
	SetupExactDict ( tArgs.m_pDict );
	if ( m_pFieldFilter )
		tArgs.m_pFieldFilter = m_pFieldFilter->Clone();
	tArgs.m_pSettings = &m_tSettings;
	tArgs.m_pWordlist = &tWordlist;
	tArgs.m_pQueryTokenizer = pQueryTokenizer;
	tArgs.m_iExpandKeywords = m_tMutableSettings.m_iExpandKeywords;
	tArgs.m_iExpansionLimit = m_iExpansionLimit;
	tArgs.m_bExpandPrefix = ( bWordDict && IsStarDict ( bWordDict ) );

	return Explain ( tArgs );
}

StoredQuerySharedPtrVecSharedPtr_t PercolateIndex_c::MakeClone () const
{
	StoredQuerySharedPtrVecSharedPtr_t pNewVec {new CSphVector<StoredQuerySharedPtr_t>};
	pNewVec->Reserve ( m_pQueries->GetLimit () );
	for ( auto& tItem : *m_pQueries )
		pNewVec->Add ( tItem );
	return pNewVec;
}

// if m_pQueries has a room - do fast lockfree add. Otherwise allocate new one and do slower add.
void PercolateIndex_c::AddToStoredUnl ( StoredQuerySharedPtr_t tNew ) REQUIRES ( m_tLock )
{
	m_hQueries.Add ( tNew->m_iQUID, m_pQueries->GetLength ());
	assert ( m_hQueries.Find ( tNew->m_iQUID ) && ( *m_hQueries.Find ( tNew->m_iQUID )==m_pQueries->GetLength ()));
	if ( m_pQueries->GetLength() < m_pQueries->GetLimit() ) // fast add possible
	{
		m_pQueries->Add ( std::move ( tNew ) );
		++m_iGeneration;
		return;
	}

	// no room - perform full relimit
	assert ( m_pQueries->GetLength () >= m_pQueries->GetLimit () );
	auto pNewVec = MakeClone ();
	pNewVec->Add ( std::move (tNew) );
	m_pQueries = pNewVec;
	++m_iGeneration;
}

// immutable, unchangeable
SharedPQSlice_t PercolateIndex_c::GetStoredUnl () const REQUIRES_SHARED ( m_tLock )
{
	return SharedPQSlice_t { m_pQueries, m_iGeneration };
}

SharedPQSlice_t PercolateIndex_c::GetStored () const EXCLUDES ( m_tLock )
{
	ScRL_t rLock ( m_tLock );
	return GetStoredUnl();
}

//////////////////////////////////////////////////////////////////////////

void LoadStoredQueryV6 ( DWORD uVersion, StoredQueryDesc_t & tQuery, CSphReader & tReader )
{
	if ( uVersion>=3 )
		tQuery.m_iQUID = tReader.GetOffset();
	if ( uVersion>=4 )
		tQuery.m_bQL = ( tReader.GetDword()!=0 );

	tQuery.m_sQuery = tReader.GetString();
	if ( uVersion==1 )
		return;

	tQuery.m_sTags = tReader.GetString();

	tQuery.m_dFilters.Reset ( tReader.GetDword() );
	tQuery.m_dFilterTree.Reset ( tReader.GetDword() );
	for ( auto& tFilter : tQuery.m_dFilters )
	{
		tFilter.m_sAttrName = tReader.GetString();
		tFilter.m_bExclude = ( tReader.GetDword()!=0 );
		tFilter.m_bHasEqualMin = ( tReader.GetDword()!=0 );
		tFilter.m_bHasEqualMax = ( tReader.GetDword()!=0 );
		tFilter.m_eType = (ESphFilter)tReader.GetDword();
		tFilter.m_eMvaFunc = (ESphMvaFunc)tReader.GetDword ();
		tReader.GetBytes ( &tFilter.m_iMinValue, sizeof(tFilter.m_iMinValue) );
		tReader.GetBytes ( &tFilter.m_iMaxValue, sizeof(tFilter.m_iMaxValue) );
		tFilter.m_dValues.Resize ( tReader.GetDword() );
		tFilter.m_dStrings.Resize ( tReader.GetDword() );
		for ( auto& dValue : tFilter.m_dValues )
			tReader.GetBytes ( &dValue, sizeof ( dValue ) );
		for ( auto& dString : tFilter.m_dStrings )
			dString = tReader.GetString ();
	}
	for ( auto & tItem : tQuery.m_dFilterTree )
	{
		tItem.m_iLeft = tReader.GetDword();
		tItem.m_iRight = tReader.GetDword();
		tItem.m_iFilterItem = tReader.GetDword();
		tItem.m_bOr = ( tReader.GetDword()!=0 );
	}
}

template<typename READER>
void LoadStoredQuery ( DWORD uVersion, StoredQueryDesc_t & tQuery, READER & tReader )
{
	assert ( uVersion>=7 );
	tQuery.m_iQUID = tReader.UnzipOffset();
	tQuery.m_bQL = ( tReader.UnzipInt()!=0 );
	tQuery.m_sQuery = tReader.GetString();
	tQuery.m_sTags = tReader.GetString();

	tQuery.m_dFilters.Reset ( tReader.UnzipInt() );
	tQuery.m_dFilterTree.Reset ( tReader.UnzipInt() );
	for ( auto& tFilter : tQuery.m_dFilters )
	{
		tFilter.m_sAttrName = tReader.GetString();
		tFilter.m_bExclude = ( tReader.UnzipInt()!=0 );
		tFilter.m_bHasEqualMin = ( tReader.UnzipInt()!=0 );
		tFilter.m_bHasEqualMax = ( tReader.UnzipInt()!=0 );
		tFilter.m_bOpenLeft = ( tReader.UnzipInt()!=0 );
		tFilter.m_bOpenRight = ( tReader.UnzipInt()!=0 );
		tFilter.m_bIsNull = ( tReader.UnzipInt()!=0 );
		tFilter.m_eType = (ESphFilter)tReader.UnzipInt();
		tFilter.m_eMvaFunc = (ESphMvaFunc)tReader.UnzipInt ();
		tFilter.m_iMinValue = tReader.UnzipOffset();
		tFilter.m_iMaxValue = tReader.UnzipOffset();

		int iValCount = tReader.UnzipInt();
		int iStrCount = tReader.UnzipInt();
		CSphFixedVector<SphAttr_t> dVals ( iValCount );
		CSphFixedVector<CSphString> dStrings ( iStrCount );

		for ( auto & dValue : dVals )
			dValue = tReader.UnzipOffset ();
		for ( auto & dString : dStrings )
			dString = tReader.GetString ();

		tFilter.m_dValues.AdoptData ( dVals.LeakData(), iValCount, iValCount );
		tFilter.m_dStrings.AdoptData ( dStrings.LeakData(), iStrCount, iStrCount );
	}
	for ( auto& tItem : tQuery.m_dFilterTree )
	{
		tItem.m_iLeft = tReader.UnzipInt();
		tItem.m_iRight = tReader.UnzipInt();
		tItem.m_iFilterItem = tReader.UnzipInt();
		tItem.m_bOr = ( tReader.UnzipInt()!=0 );
	}
}

template<typename WRITER>
void SaveStoredQuery ( const StoredQueryDesc_t & tQuery, WRITER & tWriter )
{
	tWriter.ZipOffset ( tQuery.m_iQUID );
	tWriter.ZipInt ( tQuery.m_bQL );
	tWriter.PutString ( tQuery.m_sQuery );
	tWriter.PutString ( tQuery.m_sTags );
	tWriter.ZipInt ( tQuery.m_dFilters.GetLength() );
	tWriter.ZipInt ( tQuery.m_dFilterTree.GetLength() );
	ARRAY_FOREACH ( iFilter, tQuery.m_dFilters )
	{
		const CSphFilterSettings & tFilter = tQuery.m_dFilters[iFilter];
		tWriter.PutString ( tFilter.m_sAttrName );
		tWriter.ZipInt ( tFilter.m_bExclude );
		tWriter.ZipInt ( tFilter.m_bHasEqualMin );
		tWriter.ZipInt ( tFilter.m_bHasEqualMax );
		tWriter.ZipInt ( tFilter.m_bOpenLeft );
		tWriter.ZipInt ( tFilter.m_bOpenRight );
		tWriter.ZipInt ( tFilter.m_bIsNull );
		tWriter.ZipInt ( tFilter.m_eType );
		tWriter.ZipInt ( tFilter.m_eMvaFunc );
		tWriter.ZipOffset ( tFilter.m_iMinValue );
		tWriter.ZipOffset ( tFilter.m_iMaxValue );
		tWriter.ZipInt ( tFilter.m_dValues.GetLength() );
		tWriter.ZipInt ( tFilter.m_dStrings.GetLength() );
		ARRAY_FOREACH ( j, tFilter.m_dValues )
			tWriter.ZipOffset ( tFilter.m_dValues[j] );
		ARRAY_FOREACH ( j, tFilter.m_dStrings )
			tWriter.PutString ( tFilter.m_dStrings[j] );
	}
	ARRAY_FOREACH ( iTree, tQuery.m_dFilterTree )
	{
		const FilterTreeItem_t & tItem = tQuery.m_dFilterTree[iTree];
		tWriter.ZipInt ( tItem.m_iLeft );
		tWriter.ZipInt ( tItem.m_iRight );
		tWriter.ZipInt ( tItem.m_iFilterItem );
		tWriter.ZipInt ( tItem.m_bOr );
	}
}

void operator<< ( JsonEscapedBuilder& tOut, const FilterTreeItem_t& tItem )
{
	auto _ = tOut.Object();
	tOut.NamedValNonDefault ( "left", tItem.m_iLeft, -1 );
	tOut.NamedValNonDefault ( "right", tItem.m_iRight, -1 );
	tOut.NamedValNonDefault ( "item", tItem.m_iFilterItem, -1 );
	tOut.NamedValNonDefault ( "or", tItem.m_bOr, false );
}

void operator<< ( JsonEscapedBuilder& tOut, const CSphFilterSettings& tFilter )
{
	auto _ = tOut.ObjectW();
	tOut.NamedValNonDefault ( "type", tFilter.m_eType, SPH_FILTER_VALUES );
	tOut.NamedStringNonEmpty ( "attr", tFilter.m_sAttrName );
	if ( tFilter.m_eType==SPH_FILTER_FLOATRANGE )
	{
		tOut.NamedVal( "fmin", tFilter.m_fMinValue );
		tOut.NamedVal ( "fmax", tFilter.m_fMaxValue );
	} else if ( tFilter.m_eType== SPH_FILTER_RANGE )
	{
		tOut.NamedValNonDefault ( "min", tFilter.m_iMinValue, (SphAttr_t)LLONG_MIN );
		tOut.NamedValNonDefault ( "max", tFilter.m_iMaxValue, (SphAttr_t)LLONG_MAX );
	}
	tOut.NamedValNonDefault ( "not", tFilter.m_bExclude, false );
	tOut.NamedValNonDefault ( "eq_min", tFilter.m_bHasEqualMin, true );
	tOut.NamedValNonDefault ( "eq_max", tFilter.m_bHasEqualMax, true );
	tOut.NamedValNonDefault ( "open_left", tFilter.m_bOpenLeft, false );
	tOut.NamedValNonDefault ( "open_right", tFilter.m_bOpenRight, false );
	tOut.NamedValNonDefault ( "is_null", tFilter.m_bIsNull, false );
	tOut.NamedValNonDefault ( "mva_func", tFilter.m_eMvaFunc, SPH_MVAFUNC_NONE );
	if ( !tFilter.m_dValues.IsEmpty() )
	{
		tOut.Named ( "values" );
		auto _ = tOut.ArrayW();
		for ( const auto& tValue : tFilter.m_dValues )
			tOut << tValue;
	}
	if ( !tFilter.m_dStrings.IsEmpty() )
	{
		tOut.Named ( "strings" );
		auto _ = tOut.ArrayW();
		for ( const auto& tValue : tFilter.m_dStrings )
			tOut.FixupSpacedAndAppendEscaped (tValue.cstr());
	}
}

void operator<< ( JsonEscapedBuilder& tOut, const StoredQueryDesc_t& tQuery )
{
	auto tRoot = tOut.ObjectW();
	tOut.NamedVal ( "quid", tQuery.m_iQUID );
	tOut.NamedValNonDefault ( "ql", tQuery.m_bQL, true );
	tOut.NamedStringNonEmpty ( "query", tQuery.m_sQuery );
	tOut.NamedStringNonEmpty ( "tags", tQuery.m_sTags );
	if ( !tQuery.m_dFilters.IsEmpty() )
	{
		tOut.Named ( "filters" );
		auto _ = tOut.ArrayW();
		for ( const auto& tFilter : tQuery.m_dFilters )
			tOut << tFilter;
	}

	if ( !tQuery.m_dFilterTree.IsEmpty() )
	{
		tOut.Named ( "filter_tree" );
		auto _ = tOut.ArrayW();
		for ( const auto& tItem : tQuery.m_dFilterTree )
			tOut << tItem;
	}
}

template<typename T>
class JsonLoaderData_T
{
	CSphFixedVector<T> m_dVals { 0 };
	int m_iItem { 0 };
	bson::Bson_c m_tParent;

public:
	explicit JsonLoaderData_T ( bson::NodeHandle_t tNode )
		: m_tParent ( tNode )
	{
		if ( !bson::IsNullNode ( m_tParent ) )
			m_dVals.Reset ( m_tParent.CountValues() );
	}

	T & GetNextItem ()
	{
		return m_dVals[m_iItem++];
	}

	void LoadItemJson ( bson::Action_f && fAction )
	{
		if ( bson::IsNullNode ( m_tParent ) )
			return;

		m_tParent.ForEach ( [&fAction] ( const bson::NodeHandle_t& tNode ) {
			fAction ( tNode );
		} );
	}

	void MoveTo ( CSphVector<T> & dDst )
	{
		if ( bson::IsNullNode ( m_tParent ) )
			return;

		int iCount = m_dVals.GetLength();
		dDst.AdoptData ( m_dVals.LeakData(), iCount, iCount );
	}

	void SwapData ( CSphFixedVector<T> & dDst )
	{
		if ( bson::IsNullNode ( m_tParent ) )
			return;

		dDst.SwapData ( m_dVals );
	}
};

void LoadStoredFilterTreeItemJson ( const bson::Bson_c & tNode, FilterTreeItem_t & tItem )
{
	using namespace bson;
	tItem.m_iLeft = (int)Int ( tNode.ChildByName ( "left" ), -1 );
	tItem.m_iRight = (int)Int ( tNode.ChildByName ( "right" ), -1 );
	tItem.m_iFilterItem = (int)Int ( tNode.ChildByName ( "item" ), -1 );
	tItem.m_bOr = Bool ( tNode.ChildByName ( "or" ), false );
}

void LoadStoredFilterItemJson ( const bson::Bson_c & tNode, CSphFilterSettings & tFilter )
{
	using namespace bson;
	tFilter.m_eType = (ESphFilter)Int ( tNode.ChildByName ( "type" ), SPH_FILTER_VALUES );
	tFilter.m_sAttrName = String ( tNode.ChildByName ( "attr" ) );
	if ( tFilter.m_eType == SPH_FILTER_FLOATRANGE )
	{
		tFilter.m_fMinValue = (float)Double ( tNode.ChildByName ( "fmin" ) );
		tFilter.m_fMaxValue = (float)Double ( tNode.ChildByName ( "fmax" ) );
	} else if ( tFilter.m_eType == SPH_FILTER_RANGE )
	{
		tFilter.m_iMinValue = Int ( tNode.ChildByName ( "min" ), (SphAttr_t)LLONG_MIN );
		tFilter.m_iMaxValue = Int ( tNode.ChildByName ( "max" ), (SphAttr_t)LLONG_MAX );
	}
	tFilter.m_bExclude = Bool ( tNode.ChildByName ( "not" ), false );
	tFilter.m_bHasEqualMin = Bool ( tNode.ChildByName ( "eq_min" ), true );
	tFilter.m_bHasEqualMax = Bool ( tNode.ChildByName ( "eq_max" ), true );
	tFilter.m_bOpenLeft = Bool ( tNode.ChildByName ( "open_left" ), false );
	tFilter.m_bOpenRight = Bool ( tNode.ChildByName ( "open_right" ), false );
	tFilter.m_bIsNull = Bool ( tNode.ChildByName ( "is_null" ), false );
	tFilter.m_eMvaFunc = (ESphMvaFunc)Int ( tNode.ChildByName ( "mva_func" ), SPH_MVAFUNC_NONE );
	{
		JsonLoaderData_T<SphAttr_t> tLoaderValues ( tNode.ChildByName ( "values" ) );
		tLoaderValues.LoadItemJson ( [&tLoaderValues] ( const NodeHandle_t& tNode ) { tLoaderValues.GetNextItem() = Int ( tNode ); } );
		tLoaderValues.MoveTo ( tFilter.m_dValues );
	}
	{
		JsonLoaderData_T<CSphString> tLoaderStrings ( tNode.ChildByName ( "strings" ) );
		tLoaderStrings.LoadItemJson ( [&tLoaderStrings] ( const NodeHandle_t& tNode ) { tLoaderStrings.GetNextItem() = String ( tNode ); } );
		tLoaderStrings.MoveTo ( tFilter.m_dStrings );
	}
}

void LoadStoredQueryJson ( StoredQueryDesc_t & tQuery, const bson::Bson_c & tNode )
{
	using namespace bson;
	assert ( tNode.IsAssoc() );

	tQuery.m_iQUID = Int ( tNode.ChildByName ( "quid" ) );
	tQuery.m_bQL = Bool ( tNode.ChildByName ( "ql" ), true );
	tQuery.m_sQuery = String ( tNode.ChildByName ( "query" ) );
	tQuery.m_sTags = String ( tNode.ChildByName ( "tags" ) );

	{
		JsonLoaderData_T<CSphFilterSettings> tLoaderFilters ( tNode.ChildByName ( "filters" ) );
		tLoaderFilters.LoadItemJson ( [&tLoaderFilters] ( const NodeHandle_t& tNode ) {
			CSphFilterSettings & tFilter = tLoaderFilters.GetNextItem();
			LoadStoredFilterItemJson ( tNode, tFilter );
			} );
		tLoaderFilters.SwapData ( tQuery.m_dFilters );
	}
	{
		JsonLoaderData_T<FilterTreeItem_t> tLoaderFilterTree ( tNode.ChildByName ( "filter_tree" ) );
		tLoaderFilterTree.LoadItemJson ( [&tLoaderFilterTree] ( const NodeHandle_t& tNode ) {
			auto & tFilterItem = tLoaderFilterTree.GetNextItem();
			LoadStoredFilterTreeItemJson ( tNode, tFilterItem );
			} );
		tLoaderFilterTree.SwapData ( tQuery.m_dFilterTree );
	}
}

void LoadStoredQuery ( const BYTE * pData, int iLen, StoredQueryDesc_t & tQuery )
{
	MemoryReader_c tReader ( pData, iLen );
	LoadStoredQuery ( PQ_META_VERSION_MAX, tQuery, tReader );
}

void LoadStoredQuery ( DWORD uVersion, StoredQueryDesc_t & tQuery, CSphReader & tReader )
{
	LoadStoredQuery<CSphReader> ( uVersion, tQuery, tReader );
}

void SaveStoredQuery ( const StoredQueryDesc_t & tQuery, CSphVector<BYTE> & dOut )
{
	MemoryWriter_c tWriter ( dOut );
	SaveStoredQuery ( tQuery, tWriter );
}

void SaveStoredQuery ( const StoredQueryDesc_t & tQuery, CSphWriter & tWriter )
{
	SaveStoredQuery<CSphWriter> ( tQuery, tWriter );
}

template<typename READER>
void LoadDeleteQuery_T ( CSphVector<int64_t> & dQueries, CSphString & sTags, READER & tReader )
{
	dQueries.Resize ( tReader.UnzipInt() );
	ARRAY_FOREACH ( i, dQueries )
		dQueries[i] = tReader.UnzipOffset();

	sTags = tReader.GetString();
}

void LoadDeleteQuery ( const BYTE * pData, int iLen, CSphVector<int64_t> & dQueries, CSphString & sTags )
{
	MemoryReader_c tReader ( pData, iLen );
	LoadDeleteQuery_T ( dQueries, sTags, tReader );
}

void LoadDeleteQuery ( CSphVector<int64_t> & dQueries, CSphString & sTags, CSphReader & tReader )
{
	LoadDeleteQuery_T ( dQueries, sTags, tReader );
}

template<typename WRITER>
void SaveDeleteQuery_T ( const VecTraits_T<int64_t>& dQueries, const char* sTags, WRITER& tWriter )
{
	tWriter.ZipInt ( dQueries.GetLength () );
	for ( int64_t iQuery : dQueries )
		tWriter.ZipOffset ( iQuery );

	tWriter.PutString ( sTags );
}

void SaveDeleteQuery ( const VecTraits_T<int64_t>& dQueries, const char* sTags, CSphVector<BYTE>& dOut )
{
	MemoryWriter_c tWriter ( dOut );
	SaveDeleteQuery_T ( dQueries, sTags, tWriter );
}

void SaveDeleteQuery ( const VecTraits_T<int64_t> & dQueries, const char * sTags, CSphWriter & tWriter )
{
	SaveDeleteQuery_T ( dQueries, sTags, tWriter );
}
