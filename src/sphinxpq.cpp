//
// Copyright (c) 2017-2018, Manticore Software LTD (http://manticoresearch.com)
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
#include "sphinxrlp.h"

#if USE_WINDOWS
#include <io.h> // for open(), close()
#endif

/// protection from concurrent changes during binlog replay
static auto&	g_bRTChangesAllowed		= RTChangesAllowed ();

//////////////////////////////////////////////////////////////////////////
// percolate index

struct StoredQuery_t : public StoredQuery_i
{
	XQQuery_t *						m_pXQ = nullptr;

	CSphVector<uint64_t>			m_dRejectTerms;
	CSphFixedVector<uint64_t>		m_dRejectWilds {0};
	bool							m_bOnlyTerms = false; // flag of simple query, ie only words and no operators
	CSphVector<uint64_t>			m_dTags;
	DictMap_t						m_hDict;
	CSphVector<CSphString>			m_dSuffixes;

	bool							IsFullscan() const { return m_pXQ->m_bEmpty; }

	virtual ~StoredQuery_t() override { SafeDelete ( m_pXQ ); }
};

static bool NotImplementedError ( CSphString * pError )
{
	if ( pError )
		*pError = "not implemented";

	return false;
}

struct StoredQueryKey_t
{
	uint64_t m_uQUID = 0;
	StoredQuery_t * m_pQuery = nullptr;
};

// FindSpan vector operators
static bool operator < ( const StoredQueryKey_t & tKey, uint64_t uQUID )
{
	return tKey.m_uQUID<uQUID;
}

static bool operator == ( const StoredQueryKey_t & tKey, uint64_t uQUID )
{
	return tKey.m_uQUID==uQUID;
}

static bool operator < ( uint64_t uQUID, const StoredQueryKey_t & tKey )
{
	return uQUID<tKey.m_uQUID;
}

static int g_iPercolateThreads = 1;

class PercolateIndex_c : public PercolateIndex_i
{
public:
	explicit PercolateIndex_c ( const CSphSchema & tSchema, const char * sIndexName, const char * sPath );
	~PercolateIndex_c () override;

	bool AddDocument ( const VecTraits_T<const char *> &dFields, const CSphMatch & tDoc,
		bool bReplace, const CSphString & sTokenFilterOptions, const char ** ppStr, const VecTraits_T<DWORD> & dMvas,
		CSphString & sError, CSphString & sWarning, ISphRtAccum * pAccExt ) override;
	bool MatchDocuments ( ISphRtAccum * pAccExt, PercolateMatchResult_t &tRes ) override;
	void RollBack ( ISphRtAccum * pAccExt ) override;
	int DeleteQueries ( const uint64_t * pQueries, int iCount ) override;
	int DeleteQueries ( const char * sTags ) override;

	StoredQuery_i * AddQuery ( const PercolateQueryArgs_t & tArgs, const ISphTokenizer * pTokenizer, CSphDict * pDict, CSphString & sError )
		REQUIRES (!m_tLock);
	StoredQuery_i * Query ( const PercolateQueryArgs_t & tArgs, CSphString & sError ) override;
	bool Commit ( StoredQuery_i * pQuery, CSphString & sError ) override;

	bool Prealloc ( bool bStripPath ) override;
	void Dealloc () override {}
	void Preread () override {}
	void PostSetup() override;
	ISphRtAccum * CreateAccum ( CSphString & sError ) override;
	ISphTokenizer * CloneIndexingTokenizer() const override { return m_pTokenizerIndexing->Clone ( SPH_CLONE_INDEX ); }
	void SaveMeta ( bool bShutdown );
	void GetQueries ( const char * sFilterTags, bool bTagsEq, const CSphFilterSettings * pUID, int iOffset, int iLimit, CSphVector<PercolateQueryDesc> & dQueries ) override
		REQUIRES (!m_tLock);
	bool Truncate ( CSphString & ) override EXCLUDES (m_tLock);

	// RT index stub
	bool MultiQuery ( const CSphQuery *, CSphQueryResult *, int, ISphMatchSorter **, const CSphMultiQueryArgs & ) const override;
	bool MultiQueryEx ( int, const CSphQuery *, CSphQueryResult **, ISphMatchSorter **, const CSphMultiQueryArgs & ) const override;
	virtual bool AddDocument ( ISphHits * , const CSphMatch & , const char ** , const CSphVector<DWORD> & , CSphString & , CSphString & ) { return true; }
	void Commit ( int * , ISphRtAccum * pAccExt ) override { RollBack ( pAccExt ); }
	bool DeleteDocument ( const SphDocID_t * , int , CSphString & , ISphRtAccum * pAccExt ) override { RollBack ( pAccExt ); return true; }
	void CheckRamFlush () override;
	void ForceRamFlush ( bool bPeriodic ) override;
	void ForceDiskChunk () override;
	bool AttachDiskIndex ( CSphIndex * , CSphString & ) override { return true; }
	void Optimize () override {}
	bool IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, CSphString & sError ) const override;
	void Reconfigure ( CSphReconfigureSetup & tSetup ) override REQUIRES ( !m_tLock );
	CSphIndex * GetDiskChunk ( int ) override { return NULL; } // NOLINT
	int64_t GetFlushAge() const override { return 0; }

	// plain index stub
	SphDocID_t *		GetKillList () const override { return NULL; }
	int					GetKillListSize () const override { return 0 ; }
	bool				HasDocid ( SphDocID_t ) const override { return false; }
	int					Build ( const CSphVector<CSphSource*> & , int , int ) override { return 0; }
	bool				Merge ( CSphIndex * , const CSphVector<CSphFilterSettings> & , bool ) override {return false; }
	void				SetBase ( const char * ) override {}
	bool				Rename ( const char * ) override { return false; }
	bool				Lock () override { return true; }
	void				Unlock () override {}
//	virtual bool				Mlock () { return false; }
	bool				EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const override;
	const CSphSourceStats &	GetStats () const override { return m_tStat; }
	void				GetStatus ( CSphIndexStatus* pRes ) const override { assert (pRes); if ( pRes ) { pRes->m_iDiskUse = 0; pRes->m_iRamUse = 0;}}
	bool				GetKeywords ( CSphVector <CSphKeywordInfo> & , const char * , const GetKeywordsSettings_t & , CSphString * pError ) const override { return NotImplementedError(pError); }
	bool				FillKeywords ( CSphVector <CSphKeywordInfo> & ) const override { return false; }
	int					UpdateAttributes ( const CSphAttrUpdate & , int , CSphString & sError, CSphString & ) override { NotImplementedError ( &sError ); return -1; }
	bool				SaveAttributes ( CSphString & ) const override { return true; }
	DWORD				GetAttributeStatus () const override { return 0; }
	virtual bool		CreateModifiedFiles ( bool , const CSphString & , ESphAttr , int , CSphString & ) { return true; }
	bool				AddRemoveAttribute ( bool , const CSphString & , ESphAttr , CSphString & sError ) override { return NotImplementedError ( &sError ); }
	void				DebugDumpHeader ( FILE *, const char *, bool ) override {}
	void				DebugDumpDocids ( FILE * ) override {}
	void				DebugDumpHitlist ( FILE * , const char * , bool ) override {}
	int					DebugCheck ( FILE * ) override { return 0; } // NOLINT
	void				DebugDumpDict ( FILE * ) override {}
	void				SetProgressCallback ( CSphIndexProgress::IndexingProgress_fn ) override {}
	void				SetMemorySettings ( bool , bool , bool ) override {}
	void				ProhibitSave() override { m_bSaveDisabled = true; }

	const CSphSchema &GetMatchSchema () const override { return m_tMatchSchema; }

private:
	static const DWORD				META_HEADER_MAGIC = 0x50535451;	///< magic 'PSTQ' header
	static const DWORD				META_VERSION = 7;				///< current version, added expression filter

	int								m_iLockFD = -1;
	int								m_iDeleted = 0; // set in DeleteDocument, reset and return in Commit
	CSphSourceStats					m_tStat;
	ISphTokenizerRefPtr_c			m_pTokenizerIndexing;
	int								m_iMaxCodepointLength = 0;
	int64_t							m_iSavedTID = 0;
	int64_t							m_tmSaved = 0;
	bool							m_bSaveDisabled = false;

	CSphVector<StoredQueryKey_t>	m_dStored GUARDED_BY ( m_tLock );
	RwLock_t						m_tLock;

	CSphFixedVector<StoredQueryDesc_t>	m_dLoadedQueries { 0 };
	CSphSchema						m_tMatchSchema;

	void DoMatchDocuments ( const RtSegment_t * pSeg, PercolateMatchResult_t & tRes ) REQUIRES ( !m_tLock );
	bool MultiScan ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters,
		ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs &tArgs ) const;

	PercolateMatchContext_t * CreateMatchContext ( const RtSegment_t * pSeg, const SegmentReject_t &tReject );

	void RamFlush ( bool bPeriodic );
};

//////////////////////////////////////////////////////////////////////////
// percolate functions

#define PERCOLATE_BLOOM_WILD_COUNT 32
#define PERCOLATE_BLOOM_SIZE PERCOLATE_BLOOM_WILD_COUNT * 2
#define PERCOLATE_WORDS_PER_CP 128

/// percolate query index factory
PercolateIndex_i * CreateIndexPercolate ( const CSphSchema & tSchema, const char * sIndexName, const char * sPath )
{
	MEMORY ( MEM_INDEX_RT );
	return new PercolateIndex_c ( tSchema, sIndexName, sPath );
}

static void SegmentGetRejects ( const RtSegment_t * pSeg, bool bBuildInfix, bool bUtf8, SegmentReject_t & tReject )
{
	tReject.m_iRows = pSeg->m_iRows;
	const bool bMultiDocs = ( pSeg->m_iRows>1 );
	if ( bMultiDocs )
	{
		tReject.m_dPerDocTerms.Reset ( pSeg->m_iRows );
		if ( bBuildInfix )
		{
			tReject.m_dPerDocWilds.Reset ( pSeg->m_iRows * PERCOLATE_BLOOM_SIZE );
			tReject.m_dPerDocWilds.Fill ( 0 );
		}
	}
	if ( bBuildInfix )
	{
		tReject.m_dWilds.Reset ( PERCOLATE_BLOOM_SIZE );
		tReject.m_dWilds.Fill ( 0 );
	}

	RtWordReader_t tDict ( pSeg, true, PERCOLATE_WORDS_PER_CP );
	const RtWord_t * pWord = nullptr;
	BloomGenTraits_t tBloom0 ( tReject.m_dWilds.Begin() );
	BloomGenTraits_t tBloom1 ( tReject.m_dWilds.Begin() + PERCOLATE_BLOOM_WILD_COUNT );

	while ( ( pWord = tDict.UnzipWord() )!=nullptr )
	{
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
				RtDocReader_t tDoc ( pSeg, *pWord );
				while ( true )
				{
					const RtDoc_t * pDoc = tDoc.UnzipDoc();
					if ( !pDoc )
						break;

					// Docid - should be row-based started from 1
					assert ( pDoc->m_uDocID>=1 && (int)pDoc->m_uDocID<pSeg->m_iRows+1 );
					int iDoc = (int)pDoc->m_uDocID - 1;
					tReject.m_dPerDocTerms[iDoc].Add ( uHash );

					if ( bBuildInfix )
					{
						uint64_t * pBloom = tReject.m_dPerDocWilds.Begin() + iDoc * PERCOLATE_BLOOM_SIZE;
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
}

static void DoQueryGetRejects ( const XQNode_t * pNode, CSphDict * pDict, CSphVector<uint64_t> & dRejectTerms, CSphFixedVector<uint64_t> & dRejectBloom, CSphVector<CSphString> & dSuffixes, bool & bOnlyTerms, bool bUtf8 )
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
		const char * sInfix = NULL;
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

static void QueryGetRejects ( const XQNode_t * pNode, CSphDict * pDict, CSphVector<uint64_t> & dRejectTerms, CSphFixedVector<uint64_t> & dRejectBloom, CSphVector<CSphString> & dSuffixes, bool & bOnlyTerms, bool bUtf8 )
{
	DoQueryGetRejects ( pNode, pDict, dRejectTerms, dRejectBloom, dSuffixes, bOnlyTerms, bUtf8 );
	dRejectTerms.Uniq();
}

static void QueryGetTerms ( const XQNode_t * pNode, CSphDict * pDict, DictMap_t & hDict )
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

		strncpy ( (char *)sTmp, tWord.m_sWord.cstr(), iLen );
		sTmp[iLen] = '\0';

		SphWordID_t uWord = 0;
		if ( tWord.m_bMorphed )
			uWord = pDict->GetWordIDNonStemmed ( sTmp );
		else
			uWord = pDict->GetWordID ( sTmp );

		if ( !uWord )
			continue;

		iLen = strnlen ( (const char *)sTmp, sizeof(sTmp) );
		DictTerm_t & tTerm = hDict.m_hTerms.Acquire ( uHash );
		tTerm.m_uWordID = uWord;
		tTerm.m_iWordOff = hDict.m_dKeywords.GetLength();
		tTerm.m_iWordLen = iLen;

		hDict.m_dKeywords.Append ( sTmp, iLen );
	}

	for ( const XQNode_t * pChild : pNode->m_dChildren )
		QueryGetTerms ( pChild, pDict, hDict );
}

static bool TermsReject ( const CSphVector<uint64_t> & dDocs, const CSphVector<uint64_t> & dQueries )
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
		const RtWordCheckpoint_t * pCheckpoint = sphSearchCheckpoint ( sWord, iLen, 0, false, true, pSeg->m_dWordCheckpoints.Begin(), &pSeg->m_dWordCheckpoints.Last() );
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
	if ( pSeg->m_dWordCheckpoints.GetLength() )
	{
		const RtWordCheckpoint_t * pLast = &pSeg->m_dWordCheckpoints.Last();
		const RtWordCheckpoint_t * pCheckpoint = sphSearchCheckpoint ( sPrefix, iPrefix, 0, true, true, pSeg->m_dWordCheckpoints.Begin(), pLast );

		if ( pCheckpoint )
		{
			// there could be valid data prior 1st checkpoint that should be unpacked and checked
			int iNameLen = strnlen ( pCheckpoint->m_sWord, SPH_MAX_KEYWORD_LEN );
			if ( pCheckpoint!=pSeg->m_dWordCheckpoints.Begin() || (sphDictCmp ( sPrefix, iPrefix, pCheckpoint->m_sWord, iNameLen )==0 && iPrefix==iNameLen) )
				tChPoint.m_uOff = pCheckpoint->m_iOffset;

			// find the last checkpoint that meets prefix condition ( ie might be a span of terms that splat to a couple of checkpoints )
			pCheckpoint++;
			while ( pCheckpoint<=pLast )
			{
				iNameLen = strnlen ( pCheckpoint->m_sWord, SPH_MAX_KEYWORD_LEN );
				int iCmp = sphDictCmp ( sPrefix, iPrefix, pCheckpoint->m_sWord, iNameLen );
				if ( iCmp==0 && iPrefix==iNameLen )
					tChPoint.m_uOff = pCheckpoint->m_iOffset;
				if ( iCmp<0 )
					break;
				pCheckpoint++;
			}
		}
	}

	return tChPoint;
}

static void GetSuffixLocators ( const char * sWord, int iMaxCodepointLength, const RtSegment_t * pSeg, SubstringInfo_t & tSubInfo, CSphVector<Slice_t> & dPoints )
{
	assert ( sphIsWild ( *sWord ) );

	// find the longest substring of non-wild-cards
	const char * sMaxInfix = NULL;
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

		if ( iCur > 0 )
			tChPoint.m_uOff = pSeg->m_dWordCheckpoints[iCur].m_iOffset;
		if ( iNext < pSeg->m_dWordCheckpoints.GetLength() )
			tChPoint.m_uLen = pSeg->m_dWordCheckpoints[iNext].m_iOffset;
	}
}

static void PercolateTags ( const char * sTags, CSphVector<uint64_t> & dTags )
{
	if ( !sTags || !*sTags )
		return;

	StrVec_t dTagStrings;
	sphSplit ( dTagStrings, sTags );
	if ( dTagStrings.IsEmpty() )
		return;

	dTags.Resize ( dTagStrings.GetLength() );
	ARRAY_FOREACH ( i, dTagStrings )
		dTags[i] = sphFNV64 ( dTagStrings[i].cstr() );
	dTags.Uniq();
}

static bool TagsMatched ( const VecTraits_T<uint64_t>& dFilter, const VecTraits_T<uint64_t>& dQueryTags, bool bTagsEq=true )
{
	auto pFilter = dFilter.begin();
	auto pQueryTags = dQueryTags.begin();
	auto pFilterEnd = dFilter.end();
	auto pTagsEnd = dQueryTags.end();

	while ( pFilter<pFilterEnd && pQueryTags<pTagsEnd )
	{
		if ( *pQueryTags<*pFilter )
			++pQueryTags;
		else if ( *pFilter<*pQueryTags )
			++pFilter;
		else if ( *pQueryTags==*pFilter )
			return bTagsEq;
	}
	return !bTagsEq;
}

//////////////////////////////////////////////////////////////////////////
// percolate index definition

PercolateIndex_c::PercolateIndex_c ( const CSphSchema & tSchema, const char * sIndexName, const char * sPath )
	: PercolateIndex_i ( sIndexName, sPath )
{
	m_tSchema = tSchema;

	// fill match schema
	m_tMatchSchema.AddAttr ( CSphColumnInfo ( "uid", SPH_ATTR_BIGINT ), true );
	m_tMatchSchema.AddAttr ( CSphColumnInfo ( "query", SPH_ATTR_STRINGPTR ), true );
	m_tMatchSchema.AddAttr ( CSphColumnInfo ( "tags", SPH_ATTR_STRINGPTR ), true );
	m_tMatchSchema.AddAttr ( CSphColumnInfo ( "filters", SPH_ATTR_STRINGPTR ), true );
}

PercolateIndex_c::~PercolateIndex_c ()
{
	bool bValid = m_pTokenizer && m_pDict;
	if ( bValid )
		SaveMeta ( true );

	{ // coverity complains about accessing m_dStored without locking tLock
		ScWL_t wLock { m_tLock };
		for ( auto& dStored : m_dStored )
			SafeDelete ( dStored.m_pQuery );
	}
	SafeClose ( m_iLockFD );
}

ISphRtAccum * PercolateIndex_c::CreateAccum ( CSphString & sError )
{
	return AcquireAccum ( m_pDict, nullptr, true, false, &sError );
}


bool PercolateIndex_c::AddDocument ( const VecTraits_T<const char *> &dFields,
	const CSphMatch & tDoc, bool , const CSphString & , const char ** ppStr, const VecTraits_T<DWORD> & dMvas,
	CSphString & sError, CSphString & sWarning, ISphRtAccum * pAccExt )
{
	auto pAcc = ( RtAccum_t * ) AcquireAccum ( m_pDict, pAccExt, true, true, &sError );
	if ( !pAcc )
		return false;

	ISphTokenizerRefPtr_c tTokenizer { CloneIndexingTokenizer () };
	if ( !tTokenizer )
	{
		sError = GetLastError ();
		return false;
	}

	CSphSource_StringVector tSrc ( dFields, m_tSchema );
	if ( m_tSettings.m_bHtmlStrip &&
		!tSrc.SetStripHTML ( m_tSettings.m_sHtmlIndexAttrs.cstr(), m_tSettings.m_sHtmlRemoveElements.cstr(),
			m_tSettings.m_bIndexSP, m_tSettings.m_sZones.cstr(), sError ) )
		return false;

	ISphFieldFilterRefPtr_c pFieldFilter;
	if ( m_pFieldFilter )
		pFieldFilter = m_pFieldFilter->Clone();

	// TODO: field filter \ token filter?
	tSrc.Setup ( m_tSettings );
	tSrc.SetTokenizer ( tTokenizer );
	tSrc.SetDict ( pAcc->m_pDict );
	tSrc.SetFieldFilter ( pFieldFilter );
	if ( !tSrc.Connect ( m_sLastError ) )
		return false;

	m_tSchema.CloneWholeMatch ( &tSrc.m_tDocInfo, tDoc );

	if ( !tSrc.IterateStart ( sError ) || !tSrc.IterateDocument ( sError ) )
		return false;

	ISphHits * pHits = tSrc.IterateHits ( sError );
	pAcc->GrabLastWarning ( sWarning );

	pAcc->AddDocument ( pHits, tDoc, true, m_tSchema.GetRowSize(), ppStr, dMvas );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// percolate Qword

struct PercolateQword_t : public ISphQword
{
public:
	PercolateQword_t () = default;
	virtual ~PercolateQword_t ()
	{
	}

	const CSphMatch & GetNextDoc ( DWORD * ) final
	{
		m_iHits = 0;
		while (true)
		{
			const RtDoc_t * pDoc = m_tDocReader.UnzipDoc();
			if ( !pDoc && m_iDoc>=m_dDoclist.GetLength() )
			{
				m_tMatch.m_uDocID = 0;
				return m_tMatch;
			}

			if ( !pDoc )
			{
				SetupReader();
				pDoc = m_tDocReader.UnzipDoc();
				assert ( pDoc );
			}

			m_tMatch.m_uDocID = pDoc->m_uDocID;
			m_dQwordFields.Assign32 ( pDoc->m_uDocFields );
			m_uMatchHits = pDoc->m_uHits;
			m_iHitlistPos = (uint64_t(pDoc->m_uHits)<<32) + pDoc->m_uHit;
			m_bAllFieldsKnown = false;

			return m_tMatch;
		}
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
			m_tHitReader.Seek ( DWORD(uOff), iHits );
		}
	}

	Hitpos_t GetNextHit () final
	{
		if ( m_uNextHit==0 )
			return Hitpos_t ( m_tHitReader.UnzipHit() );
		else if ( m_uNextHit==0xffffffffUL )
			return EMPTY_HIT;
		else
		{
			Hitpos_t tHit ( m_uNextHit );
			m_uNextHit = 0xffffffffUL;
			return tHit;
		}
	}

	bool Setup ( const RtSegment_t * pSeg, CSphVector<Slice_t> & dDoclist )
	{
		m_iDoc = 0;
		m_tDocReader = RtDocReader_t();
		m_pSeg = pSeg;
		m_tHitReader.m_pBase = pSeg->m_dHits.Begin();

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
		m_tDocReader = RtDocReader_t ( m_pSeg, tWord );
		m_iDoc++;
	}

	const RtSegment_t *			m_pSeg = nullptr;
	CSphFixedVector<Slice_t>	m_dDoclist { 0 };
	CSphMatch					m_tMatch;
	RtDocReader_t				m_tDocReader;
	RtHitReader2_t				m_tHitReader;

	int							m_iDoc = 0;
	DWORD						m_uNextHit = 0;
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

	// cases:
	// empty - check all words
	// no matches - check only words prior to 1st checkpoint
	// checkpoint found - check words at that checkpoint
	const BYTE * pWordBase = m_pSeg->m_dWords.Begin();
	CSphVector<Slice_t> dDictWords;
	ARRAY_FOREACH ( i, dDictLoc )
	{
		RtWordReader_t tReader ( m_pSeg, true, PERCOLATE_WORDS_PER_CP );
		// locator
		// m_uOff - Start
		// m_uLen - End
		tReader.m_pCur = pWordBase + dDictLoc[i].m_uOff;
		tReader.m_pMax = pWordBase + dDictLoc[i].m_uLen;

		const RtWord_t * pWord = NULL;
		while ( ( pWord = tReader.UnzipWord() )!=NULL )
		{
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
					if ( !( tSubInfo.m_iSubLen<=pWord->m_sWord[0] && sphWildcardMatch ( (const char *)pWord->m_sWord + 1 + iSkipMagic, tSubInfo.m_sWildcard ) ) )
						iCmp = -1;
				}
				break;

			case PERCOLATE::INFIX:
				if ( sphWildcardMatch ( (const char *)pWord->m_sWord + 1 + iSkipMagic, tSubInfo.m_sWildcard ) )
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


SphWordID_t DictMap_t::GetTerm ( BYTE * sWord ) const
{
	const DictTerm_t * pTerm = m_hTerms.Find ( sphFNV64 ( sWord ) );
	if ( !pTerm )
		return 0;

	memcpy ( sWord, m_dKeywords.Begin() + pTerm->m_iWordOff, pTerm->m_iWordLen );
	return pTerm->m_uWordID;
}

PercolateMatchContext_t * PercolateIndex_c::CreateMatchContext ( const RtSegment_t * pSeg, const SegmentReject_t &tReject )
{
	return new PercolateMatchContext_t ( pSeg, m_iMaxCodepointLength, m_pDict->HasMorphology (), this
												   , m_tSchema, tReject );
}

// percolate matching
static void MatchingWork ( const StoredQuery_t * pStored, PercolateMatchContext_t & tMatchCtx )
{
	int64_t tmQueryStart = ( tMatchCtx.m_bVerbose ? sphMicroTimer() : 0 );
	tMatchCtx.m_iOnlyTerms += ( pStored->m_bOnlyTerms ? 1 : 0 );

	if ( !pStored->IsFullscan() && tMatchCtx.m_tReject.Filter ( pStored, tMatchCtx.m_bUtf8 ) )
		return;

	const RtSegment_t * pSeg = (RtSegment_t *)tMatchCtx.m_pCtx->m_pIndexData;
	const BYTE * pStrings = pSeg->m_dStrings.Begin();
	const DWORD * pMva = pSeg->m_dMvas.Begin();

	++tMatchCtx.m_iEarlyPassed;
	tMatchCtx.m_pCtx->ResetFilters();

	CSphString sError;
	CSphString sWarning;

	// setup filters
	CreateFilterContext_t tFlx;
	tFlx.m_pFilters = &pStored->m_dFilters;
	tFlx.m_pFilterTree = &pStored->m_dFilterTree;
	tFlx.m_pSchema = &tMatchCtx.m_tSchema;
	tFlx.m_pMvaPool = pMva;
	tFlx.m_pStrings = pStrings;
	tFlx.m_bArenaProhibit = true;

	bool bRes = tMatchCtx.m_pCtx->CreateFilters ( tFlx, sError, sWarning );
	tMatchCtx.m_dMsg.Err ( sError );
	tMatchCtx.m_dMsg.Warn ( sWarning );

	if (!bRes )
	{
		++tMatchCtx.m_iQueriesFailed;
		return;
	}

	const bool bCollectDocs = tMatchCtx.m_bGetDocs;
	int iDocsOff = tMatchCtx.m_dDocsMatched.GetLength();
	int iMatchCount = 0;
	// reserve space for matched docs counter
	if ( bCollectDocs )
		tMatchCtx.m_dDocsMatched.Add ( 0 );


	if ( !pStored->IsFullscan() ) // matching path
	{
		// set terms dictionary
		tMatchCtx.m_tDictMap.SetMap ( pStored->m_hDict );
		CSphQueryResult tTmpResult;
		CSphScopedPtr<ISphRanker> pRanker ( sphCreateRanker ( *pStored->m_pXQ, &tMatchCtx.m_tDummyQuery, &tTmpResult,
			*tMatchCtx.m_pTermSetup.Ptr(), *tMatchCtx.m_pCtx.Ptr(), tMatchCtx.m_tSchema ) );

		if ( !pRanker )
			return;

		const CSphMatch * pMatch = pRanker->GetMatchesBuffer();
		while ( true )
		{
			int iMatches = pRanker->GetMatches();
			if ( !iMatches )
				break;

			if ( bCollectDocs )
			{
				// docs encoding: docs-count (reserved above); docs matched
				int * pDocids = tMatchCtx.m_dDocsMatched.AddN ( iMatches );
				for ( int iMatch=0; iMatch<iMatches; ++iMatch )
					pDocids[iMatch] = ( int ) pMatch[iMatch].m_uDocID;
			}

			iMatchCount += iMatches;
		}
	} else // full-scan path
	{
		CSphMatch tDoc;
		int iStride = DOCINFO_IDSIZE + tMatchCtx.m_tSchema.GetRowSize();
		const CSphIndex * pIndex = tMatchCtx.m_pTermSetup->m_pIndex;
		const CSphRowitem * pRow = pSeg->m_dRows.Begin();
		for ( int i = 0; i<pSeg->m_iRows; ++i )
		{
			tDoc.m_uDocID = DOCINFO2ID ( pRow );
			pRow += iStride;
			if ( pIndex->EarlyReject ( tMatchCtx.m_pCtx.Ptr(), tDoc ) )
				continue;

			++iMatchCount;
			if ( bCollectDocs ) // keep matched docs
				tMatchCtx.m_dDocsMatched.Add ( tDoc.m_uDocID );
		}
	}

	// collect matched pq, if any
	if ( iMatchCount )
	{
		tMatchCtx.m_iDocsMatched += iMatchCount;

		PercolateQueryDesc & tDesc = tMatchCtx.m_dQueryMatched.Add();
		tDesc.m_uQID = pStored->m_uQUID;
		if ( bCollectDocs )
			tMatchCtx.m_dDocsMatched[iDocsOff] = iMatchCount;
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

	} else if ( bCollectDocs ) // pop's up reserved but not used matched counter
		tMatchCtx.m_dDocsMatched.Resize ( iDocsOff );
}


struct PercolateMatchJob_t : public ISphJob
{
	const CSphVector<StoredQueryKey_t> & m_dStored;
	CSphAtomic & m_tQueryCounter;
	PercolateMatchContext_t & m_tMatchCtx;

	const CrashQuery_t * m_pCrashQuery = nullptr;

	PercolateMatchJob_t ( const CSphVector<StoredQueryKey_t> & dStored, CSphAtomic & tQueryCounter,
		PercolateMatchContext_t & tMatchCtx, const CrashQuery_t * pCrashQuery=nullptr )
		: m_dStored ( dStored )
		, m_tQueryCounter ( tQueryCounter )
		, m_tMatchCtx ( tMatchCtx )
		, m_pCrashQuery ( pCrashQuery )
	{}

	void Call () final
	{
		CrashQuery_t tQueryTLS;
		if ( m_pCrashQuery )
		{
			CrashQuerySetTop ( &tQueryTLS ); // set crash info container
			CrashQuerySet ( *m_pCrashQuery ); // transfer crash info into container
		}
		m_tMatchCtx.m_dMsg.Clear();
		for ( long iQuery = m_tQueryCounter++; iQuery<m_dStored.GetLength (); iQuery = m_tQueryCounter++ )
			MatchingWork ( m_dStored[iQuery].m_pQuery, m_tMatchCtx );
	}
};

void PercolateQueryDesc::Swap ( PercolateQueryDesc & tOther )
{
	::Swap ( m_uQID, tOther.m_uQID );
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
	int m_iDocOff = 0;

	PQMergeIterator_t ( PQMatchContextResult_t * pMatch = nullptr )
		: m_pMatch ( pMatch )
	{
		if ( pMatch )
			m_iElems = pMatch->m_dQueryMatched.GetLength();
	}

	inline PercolateQueryDesc& CurDesc() const { return m_pMatch->m_dQueryMatched[m_iIdx];}
	inline int CurDt () const { return m_pMatch->m_dDt[m_iIdx]; }

	static inline bool IsLess ( const PQMergeIterator_t &a, const PQMergeIterator_t &b )
	{
		return a.CurDesc().m_uQID<b.CurDesc().m_uQID;
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
	CSphQueue<PQMergeIterator_t, PQMergeIterator_t> qMatches ( dMatches.GetLength ());
	for ( PQMatchContextResult_t * pMatch : dMatches )
	{
		if ( pMatch->m_dQueryMatched.IsEmpty() )
			continue;

		qMatches.Push ( pMatch );

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

	if ( qMatches.GetLength ()==1 ) // fastpath, only 1 essential result set
	{
		auto & tIt = qMatches.Root();
		assert ( tIt.m_pMatch->m_dQueryMatched.GetLength ()==iGotQueries );
		tRes.m_dQueryDesc.Set ( tIt.m_pMatch->m_dQueryMatched.LeakData (), iGotQueries );
		if ( tRes.m_bVerbose )
		{
			assert ( tIt.m_pMatch->m_dDt.GetLength ()==iGotQueries );
			tRes.m_dQueryDT.Set ( tIt.m_pMatch->m_dDt.LeakData (), iGotQueries );
		}
		if ( tRes.m_bGetDocs )
		{
			assert ( tIt.m_pMatch->m_dDocsMatched.GetLength()==iGotDocs );
			tRes.m_dDocs.Set ( tIt.m_pMatch->m_dDocsMatched.LeakData (), iGotDocs );
		}
		return;
	}

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

	PQMergeIterator_t tMin = qMatches.Root ();
	qMatches.Pop ();

	assert ( qMatches.GetLength()>=0 ); // since case of only 1 resultset we already processed.

	while (true) {
		pDst->Swap ( tMin.CurDesc () );
		++pDst;

		if ( tRes.m_bVerbose )
			*pDt++ = tMin.CurDt();

		// docs copy
		if ( tRes.m_bGetDocs )
		{
			int iDocOff = tMin.m_iDocOff;
			int iDocCount = tMin.m_pMatch->m_dDocsMatched[iDocOff];

			memcpy ( pDocs, tMin.m_pMatch->m_dDocsMatched.begin() + iDocOff,
				sizeof(int) * ( iDocCount + 1 ) );

			tMin.m_iDocOff += iDocCount + 1;
			pDocs += iDocCount + 1;
		}

		++tMin.m_iIdx;
		if ( tMin.m_iIdx<tMin.m_iElems )
		{
			// if current root is better - change the head.
			if ( qMatches.GetLength () && !PQMergeIterator_t::IsLess ( tMin, qMatches.Root() ) )
				qMatches.Push ( tMin );
			else
				continue;
		}

		if ( !qMatches.GetLength () )
			break;

		tMin = qMatches.Root();
		qMatches.Pop();
	}
}

// adaptor from vec of PercolateMatchContext_t* to PQMatchContextResult_t*
inline void PercolateMergeResults ( const VecTraits_T<PercolateMatchContext_t *> &dMatches, PercolateMatchResult_t &tRes )
{
	auto pMatches = ( PQMatchContextResult_t ** ) dMatches.begin ();
	auto iMatches = dMatches.GetLength ();
	PercolateMergeResults ( VecTraits_T<PQMatchContextResult_t *> ( pMatches, iMatches ), tRes );
}

void PercolateIndex_c::DoMatchDocuments ( const RtSegment_t * pSeg, PercolateMatchResult_t & tRes )
{
	SegmentReject_t tReject;
	// reject need bloom filter for either infix or prefix
	SegmentGetRejects ( pSeg, ( m_tSettings.m_iMinInfixLen>0 || m_tSettings.m_iMinPrefixLen>0 ),
		( m_iMaxCodepointLength>1 ), tReject );

	CSphAtomic iCurQuery;
	CSphFixedVector<PercolateMatchContext_t *> dResults ( 1 );
	ISphThdPool * pPool = nullptr;
	CrashQuery_t tCrashQuery;

	{
		ScRL_t rLock ( m_tLock );
		tRes.m_iTotalQueries = m_dStored.GetLength ();
	}

	// pool jobs only for decent amount of queries
	if ( g_iPercolateThreads>1 && tRes.m_iTotalQueries>4 )
	{
		int iThreads = Min ( g_iPercolateThreads, tRes.m_iTotalQueries );
		dResults.Reset ( iThreads );
		// one job always goes at current thread
		CSphString sError;
		pPool = sphThreadPoolCreate ( iThreads-1, "percolate", sError );
		if ( !pPool )
			sphWarning( "failed to create thread_pool, single thread matching used: %s", sError.cstr() );
	}

	for ( auto &dCtx: dResults )
	{
		dCtx = CreateMatchContext ( pSeg, tReject );
		dCtx->m_bGetDocs = tRes.m_bGetDocs;
		dCtx->m_bGetQuery = tRes.m_bGetQuery;
		dCtx->m_bGetFilters = tRes.m_bGetFilters;
		dCtx->m_bVerbose = tRes.m_bVerbose;
	}

	if ( tRes.m_bVerbose )
		tRes.m_tmSetup = sphMicroTimer() + tRes.m_tmSetup;

	// queries should be locked for reading now
	{
		ScRL_t rLock ( m_tLock );
		PercolateMatchJob_t tJobMain ( m_dStored, iCurQuery, *dResults[0] ); // still got crash info no need to set it again

		// work loop
		if ( pPool )
		{
			tCrashQuery = CrashQueryGet();
			for ( int i=1; i<dResults.GetLength(); ++i )
			{
				auto * pJob = new PercolateMatchJob_t ( m_dStored, iCurQuery, *dResults[i], &tCrashQuery );
				pPool->AddJob ( pJob );
			}
		}
		tJobMain.Call();
		SafeDelete ( pPool );
	}

	// merge result set
	PercolateMergeResults ( dResults, tRes );

	for ( auto & dMatch: dResults )
		SafeDelete ( dMatch );
}


bool PercolateIndex_c::MatchDocuments ( ISphRtAccum * pAccExt, PercolateMatchResult_t & tRes )
{
	MEMORY ( MEM_INDEX_RT );

	int64_t tmStart = sphMicroTimer();
	if ( tRes.m_bVerbose )
		tRes.m_tmSetup = -tmStart;
	m_sLastWarning = "";

	auto pAcc = ( RtAccum_t * ) AcquireAccum ( m_pDict, pAccExt );
	if ( !pAcc )
		return false;

	// empty txn or no queries just ignore
	{
		ScRL_t rLock ( m_tLock );
		if ( !pAcc->m_iAccumDocs || m_dStored.IsEmpty() )
		{
			pAcc->Cleanup ();
			return true;
		}
	}

	pAcc->Sort();

	RtSegment_t * pSeg = pAcc->CreateSegment ( m_tSchema.GetRowSize(), PERCOLATE_WORDS_PER_CP );
	assert ( !pSeg || pSeg->m_iRows>0 );
	assert ( !pSeg || pSeg->m_iAliveRows>0 );
	assert ( !pSeg || !pSeg->m_bTlsKlist );
	BuildSegmentInfixes ( pSeg, m_pDict->HasMorphology(), true, m_tSettings.m_iMinInfixLen,
		PERCOLATE_WORDS_PER_CP, ( m_iMaxCodepointLength>1 ) );

	DoMatchDocuments ( pSeg, tRes );
	SafeDelete ( pSeg );

	// done; cleanup accum
	pAcc->Cleanup ();

	int64_t tmEnd = sphMicroTimer();
	tRes.m_tmTotal = tmEnd - tmStart;
	if ( tRes.m_iQueriesFailed )
		tRes.m_sMessages.Warn ( "%d queries failed", tRes.m_iQueriesFailed );
	return true;
}

void PercolateIndex_c::RollBack ( ISphRtAccum * pAccExt )
{
	assert ( g_bRTChangesAllowed );

	auto pAcc = ( RtAccum_t * ) AcquireAccum ( m_pDict, pAccExt );
	if ( !pAcc )
		return;

	pAcc->Cleanup ();
}

bool PercolateIndex_c::EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const
{
	if ( !pCtx->m_pFilter )
		return false;

	int iStride = DOCINFO_IDSIZE + m_tSchema.GetRowSize();
	const CSphRowitem * pRow = FindDocinfo ( (RtSegment_t*)pCtx->m_pIndexData, tMatch.m_uDocID, iStride );
	if ( !pRow )
		return true;
	CopyDocinfo ( tMatch, pRow );

	return !pCtx->m_pFilter->Eval ( tMatch );
}


StoredQuery_i * PercolateIndex_c::Query ( const PercolateQueryArgs_t & tArgs, CSphString & sError )
{
	ISphTokenizerRefPtr_c pTokenizer ( m_pTokenizer->Clone ( SPH_CLONE_QUERY ) );
	sphSetupQueryTokenizer ( pTokenizer, IsStarDict(), m_tSettings.m_bIndexExactWords, false );

	CSphDictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };

	if ( IsStarDict () )
		SetupStarDict ( pDict, pTokenizer );

	if ( m_tSettings.m_bIndexExactWords )
		SetupExactDict ( pDict, pTokenizer );

	const ISphTokenizer * pTok = pTokenizer;
	ISphTokenizerRefPtr_c pTokenizerJson;
	if ( !tArgs.m_bQL )
	{
		pTokenizerJson = m_pTokenizer->Clone ( SPH_CLONE_QUERY );
		sphSetupQueryTokenizer ( pTokenizerJson, IsStarDict (), m_tSettings.m_bIndexExactWords, true );
		pTok = pTokenizerJson;
	}
	return AddQuery ( tArgs, pTok, pDict, sError );
}


static const QueryParser_i * CreatePlainQueryparser ( bool )
{
	return sphCreatePlainQueryParser();
}

static CreateQueryParser * g_pCreateQueryParser = CreatePlainQueryparser;
void SetPercolateQueryParserFactory ( CreateQueryParser * pCall )
{
	g_pCreateQueryParser = pCall;
}

static void FixExpanded ( XQNode_t * pNode )
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
		FixExpanded ( pNode->m_dChildren[i] );
}

StoredQuery_i * PercolateIndex_c::AddQuery ( const PercolateQueryArgs_t & tArgs, const ISphTokenizer * pTokenizer, CSphDict * pDict, CSphString & sError )
{
	const char * sQuery = tArgs.m_sQuery;
	CSphVector<BYTE> dFiltered;
	if ( m_pFieldFilter && sQuery )
	{
		ISphFieldFilterRefPtr_c pFieldFilter { m_pFieldFilter->Clone() };
		if ( pFieldFilter && pFieldFilter->Apply ( (const BYTE *)sQuery, strlen ( sQuery ), dFiltered, true ) )
			sQuery = (const char *)dFiltered.Begin();
	}

	CSphScopedPtr<XQQuery_t> tParsed ( new XQQuery_t() );
	CSphScopedPtr<const QueryParser_i> tParser ( g_pCreateQueryParser ( !tArgs.m_bQL ) );

	// right tokenizer created at upper level
	bool bParsed = tParser->ParseQuery ( *tParsed.Ptr(), sQuery, nullptr, pTokenizer, pTokenizer, &m_tSchema, pDict, m_tSettings );
	if ( !bParsed )
	{
		sError = tParsed->m_sParseError;
		return nullptr;
	}

	// FIXME!!! provide segments list instead index
	sphTransformExtendedQuery ( &tParsed->m_pRoot, m_tSettings, false, NULL );

	if ( m_iExpandKeywords!=KWE_DISABLED )
	{
		tParsed->m_pRoot = sphQueryExpandKeywords ( tParsed->m_pRoot, m_tSettings, m_iExpandKeywords );
		tParsed->m_pRoot->Check ( true );
	}

	// this should be after keyword expansion
	if ( m_tSettings.m_uAotFilterMask )
		TransformAotFilter ( tParsed->m_pRoot, pDict->GetWordforms(), m_tSettings );

	if ( m_tSettings.m_iMinPrefixLen>0 || m_tSettings.m_iMinInfixLen>0 )
		FixExpanded ( tParsed->m_pRoot );

	auto pStored = new StoredQuery_t();
	pStored->m_pXQ = tParsed.LeakPtr();
	pStored->m_bOnlyTerms = true;
	pStored->m_sQuery = sQuery;
	QueryGetRejects ( pStored->m_pXQ->m_pRoot, pDict, pStored->m_dRejectTerms, pStored->m_dRejectWilds, pStored->m_dSuffixes, pStored->m_bOnlyTerms, ( m_iMaxCodepointLength>1 ) );
	QueryGetTerms ( pStored->m_pXQ->m_pRoot, pDict, pStored->m_hDict );
	pStored->m_sTags = tArgs.m_sTags;
	PercolateTags ( tArgs.m_sTags, pStored->m_dTags );
	pStored->m_uQUID = tArgs.m_uQUID;
	pStored->m_dFilters = tArgs.m_dFilters;
	pStored->m_dFilterTree = tArgs.m_dFilterTree;
	pStored->m_bQL = tArgs.m_bQL;

	bool bAutoID = ( pStored->m_uQUID==0 );
	if ( bAutoID )
	{
		m_tLock.ReadLock();
		if ( m_dStored.GetLength() )
			pStored->m_uQUID = m_dStored.Last().m_uQUID + 1;
		else
			pStored->m_uQUID = 1;
		m_tLock.Unlock();

	} else
	{
		m_tLock.ReadLock();
		const StoredQueryKey_t * pGotStored = m_dStored.BinarySearch ( bind ( &StoredQueryKey_t::m_uQUID ), tArgs.m_uQUID );
		m_tLock.Unlock();

		if ( pGotStored && !tArgs.m_bReplace )
		{
			sError.SetSprintf ( "duplicate id '" UINT64_FMT "'", tArgs.m_uQUID );
			SafeDelete ( pStored );
		}
	}

	return pStored;
}

bool PercolateIndex_c::Commit ( StoredQuery_i * pQuery, CSphString & )
{
	StoredQuery_t * pStored = (StoredQuery_t *)pQuery;

	if ( GetBinlog() )
		GetBinlog()->BinlogPqAdd ( &m_iTID, m_sIndexName.cstr(), *pStored );

	StoredQueryKey_t tItem;
	tItem.m_pQuery = pStored;
	tItem.m_uQUID = pStored->m_uQUID;

	m_tLock.WriteLock();
	int iPos = FindSpan ( m_dStored, pStored->m_uQUID );
	if ( iPos==-1 )
	{
		m_dStored.Add ( tItem );
	} else if ( m_dStored[iPos].m_uQUID==tItem.m_uQUID )
	{
		SafeDelete ( m_dStored[iPos].m_pQuery );
		m_dStored[iPos].m_pQuery = tItem.m_pQuery;
	} else
	{
		m_dStored.Insert ( iPos+1, tItem );
	}
	m_tLock.Unlock();

	return true;
}

int PercolateIndex_c::DeleteQueries ( const uint64_t * pQueries, int iCount )
{
	assert ( !iCount || pQueries!=NULL );

	int iDeleted = 0;
	ScWL_t wLock (m_tLock);

	for ( int i=0; i<iCount; i++ )
	{
		const StoredQueryKey_t * ppElem = m_dStored.BinarySearch ( bind ( &StoredQueryKey_t::m_uQUID ), pQueries[i] );
		if ( ppElem )
		{
			int iElem = ppElem - m_dStored.Begin();
			SafeDelete ( m_dStored[iElem].m_pQuery );
			m_dStored.Remove ( iElem );
			iDeleted++;
		}
	}
	if ( iDeleted && GetBinlog() )
		GetBinlog()->BinlogPqDelete ( &m_iTID, m_sIndexName.cstr(), pQueries, iCount, nullptr );

	return iDeleted;
}

int PercolateIndex_c::DeleteQueries ( const char * sTags )
{
	CSphVector<uint64_t> dTags;
	PercolateTags ( sTags, dTags );

	if ( !dTags.GetLength() )
		return 0;

	int iDeleted = 0;
	ScWL_t wLock ( m_tLock );

	ARRAY_FOREACH ( i, m_dStored )
	{
		const StoredQuery_t * pQuery = m_dStored[i].m_pQuery;
		if ( !pQuery->m_dTags.GetLength() )
			continue;

		if ( TagsMatched ( dTags, pQuery->m_dTags ) )
		{
			SafeDelete ( m_dStored[i].m_pQuery );
			m_dStored.Remove ( i );
			i--;
			iDeleted++;
		}
	}
	if ( iDeleted && GetBinlog() )
		GetBinlog()->BinlogPqDelete ( &m_iTID, m_sIndexName.cstr(), nullptr, 0, sTags );

	return iDeleted;
}

struct PqMatchProcessor_t : ISphMatchProcessor, ISphNoncopyable
{
	const CSphQueryContext &m_tCtx;
	int m_iTag;

	PqMatchProcessor_t ( int iTag, const CSphQueryContext &tCtx )
		: m_tCtx ( tCtx )
		, m_iTag ( iTag )
	{}

	void Process ( CSphMatch * pMatch ) final
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

bool PercolateIndex_c::MultiScan ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters,
	ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs &tArgs ) const
{
	assert ( tArgs.m_iTag>=0 );

	CSphQueryProfile * pProfiler = pResult->m_pProfile;
	ESphQueryState eOldState = SPH_QSTATE_UNKNOWN;

	// we count documents only (before filters)
	if ( pQuery->m_iMaxPredictedMsec )
		pResult->m_bHasPrediction = true;

	if ( tArgs.m_uPackedFactorFlags & SPH_FACTOR_ENABLE )
		pResult->m_sWarning.SetSprintf ( "packedfactors() will not work with a fullscan; you need to specify a query" );

	// start counting
	int64_t tmQueryStart = sphMicroTimer ();
	int64_t tmMaxTimer = 0;
	if ( pQuery->m_uMaxQueryMsec>0 )
		tmMaxTimer = sphMicroTimer () + pQuery->m_uMaxQueryMsec * 1000; // max_query_time

	// select the sorter with max schema
	// uses GetAttrsCount to get working facets (was GetRowSize)
	int iMaxSchemaSize = -1;
	int iMaxSchemaIndex = -1;
	for ( int i = 0; i<iSorters; ++i )
		if ( ppSorters[i]->GetSchema ()->GetAttrsCount ()>iMaxSchemaSize )
		{
			iMaxSchemaSize = ppSorters[i]->GetSchema ()->GetAttrsCount ();
			iMaxSchemaIndex = i;
		}

	const ISphSchema &tMaxSorterSchema = *( ppSorters[iMaxSchemaIndex]->GetSchema () );

	CSphVector<const ISphSchema *> dSorterSchemas;
	SorterSchemas ( ppSorters, iSorters, iMaxSchemaIndex, dSorterSchemas );

	// setup calculations and result schema
	CSphQueryContext tCtx ( *pQuery );
	if ( !tCtx.SetupCalc ( pResult, tMaxSorterSchema, m_tMatchSchema, nullptr, false, dSorterSchemas ) )
		return false;

	// setup filters
	CreateFilterContext_t tFlx;
	tFlx.m_pFilters = &pQuery->m_dFilters;
	tFlx.m_pFilterTree = &pQuery->m_dFilterTree;
	tFlx.m_pSchema = &tMaxSorterSchema;
	tFlx.m_eCollation = pQuery->m_eCollation;
	tFlx.m_bScan = true;

	if ( !tCtx.CreateFilters ( tFlx, pResult->m_sError, pResult->m_sWarning ) )
		return false;

	// setup lookup
	tCtx.m_bLookupFilter = false;
	tCtx.m_bLookupSort = true;


	// setup overrides
	if ( !tCtx.SetupOverrides ( pQuery, pResult, m_tMatchSchema, tMaxSorterSchema ) )
		return false;

	// get all locators
	const CSphColumnInfo & dUID = m_tMatchSchema.GetAttr ( 0 );
	const CSphColumnInfo & dColQuery = m_tMatchSchema.GetAttr ( 1 );
	const CSphColumnInfo & dColTags = m_tMatchSchema.GetAttr ( 2 );
	const CSphColumnInfo & dColFilters = m_tMatchSchema.GetAttr ( 3 );
	StringBuilder_c sFilters;

	// prepare to work them rows
	bool bRandomize = ppSorters[0]->m_bRandomize;

	CSphMatch tMatch;
	tMatch.Reset ( tMaxSorterSchema.GetDynamicSize () );
	tMatch.m_iWeight = tArgs.m_iIndexWeight;
	// fixme! tag also used over bitmask | 0x80000000,
	// which marks that match comes from remote.
	// using -1 might be also interpreted as 0xFFFFFFFF in such context!
	// Does it intended?
	tMatch.m_iTag = tCtx.m_dCalcFinal.GetLength () ? -1 : tArgs.m_iTag;

	if ( pProfiler )
		eOldState = pProfiler->Switch ( SPH_QSTATE_FULLSCAN );

	int iCutoff = ( pQuery->m_iCutoff<=0 ) ? -1 : pQuery->m_iCutoff;
	BYTE * pData = nullptr;

	CSphVector<PercolateQueryDesc> dQueries;

	for ( const StoredQueryKey_t &dQuery : m_dStored )
	{
		auto * pQuery = dQuery.m_pQuery;

		tMatch.m_uDocID = dQuery.m_uQUID;
		tMatch.SetAttr ( dUID.m_tLocator, dQuery.m_uQUID );

		int iLen = pQuery->m_sQuery.Length ();
		tMatch.SetAttr ( dColQuery.m_tLocator, (SphAttr_t) sphPackPtrAttr ( iLen, pData ) );
		memcpy ( pData, pQuery->m_sQuery.cstr (), iLen );

		if ( pQuery->m_sTags.IsEmpty () )
			tMatch.SetAttr ( dColTags.m_tLocator, ( SphAttr_t ) 0 );
		else {
			iLen = pQuery->m_sTags.Length();
			tMatch.SetAttr ( dColTags.m_tLocator, ( SphAttr_t ) sphPackPtrAttr ( iLen, pData ) );
			memcpy ( pData, pQuery->m_sTags.cstr (), iLen );
		}

		sFilters.Clear ();
		if ( pQuery->m_dFilters.GetLength () )
			FormatFiltersQL ( pQuery->m_dFilters, pQuery->m_dFilterTree, sFilters );
		iLen = sFilters.GetLength ();
		tMatch.SetAttr ( dColFilters.m_tLocator, ( SphAttr_t ) sphPackPtrAttr ( iLen, pData ) );
		memcpy ( pData, sFilters.cstr (), iLen );


		++pResult->m_tStats.m_iFetchedDocs;

		tCtx.CalcFilter ( tMatch );
		if ( tCtx.m_pFilter && !tCtx.m_pFilter->Eval ( tMatch ) )
		{
			tCtx.FreeDataFilter ( tMatch );
			continue;
		}

		if ( bRandomize )
			tMatch.m_iWeight = ( sphRand () & 0xffff ) * tArgs.m_iIndexWeight;

		// submit match to sorters
		tCtx.CalcSort ( tMatch );

		bool bNewMatch = false;
		for ( int iSorter = 0; iSorter<iSorters; ++iSorter )
			bNewMatch |= ppSorters[iSorter]->Push ( tMatch );

		// stringptr expressions should be duplicated (or taken over) at this point
		tCtx.FreeDataFilter ( tMatch );
		tCtx.FreeDataSort ( tMatch );

		// handle cutoff
		if ( bNewMatch && --iCutoff==0 )
			break;

		// handle timer
		if ( tmMaxTimer && sphMicroTimer ()>=tmMaxTimer )
		{
			pResult->m_sWarning = "query time exceeded max_query_time";
			break;
		}
	}

	if ( pProfiler )
		pProfiler->Switch ( SPH_QSTATE_FINALIZE );

	// do final expression calculations
	if ( tCtx.m_dCalcFinal.GetLength () )
	{
		PqMatchProcessor_t tFinal ( tArgs.m_iTag, tCtx );
		for ( int iSorter = 0; iSorter<iSorters; iSorter++ )
		{
			ISphMatchSorter * pTop = ppSorters[iSorter];
			if ( pTop )
				pTop->Finalize ( tFinal, false );
		}
	}

	if ( pProfiler )
		pProfiler->Switch ( eOldState );

	pResult->m_iQueryTime += ( int ) ( ( sphMicroTimer () - tmQueryStart ) / 1000 );
	pResult->m_iBadRows += tCtx.m_iBadRows;

	return true; // fixme! */
}

bool PercolateIndex_c::MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters,
									ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs &tArgs ) const
{
	assert ( pQuery );

	MEMORY ( MEM_DISK_QUERY );

	// to avoid the checking of a ppSorters's element for NULL on every next step, just filter out all nulls right here
	CSphVector<ISphMatchSorter *> dSorters;
	dSorters.Reserve ( iSorters );
	for ( int i = 0; i<iSorters; ++i )
		if ( ppSorters[i] )
			dSorters.Add ( ppSorters[i] );

	iSorters = dSorters.GetLength ();

	// if we have anything to work with
	if ( iSorters==0 )
		return false;

	// non-random at the start, random at the end
	dSorters.Sort ( CmpPSortersByRandom_fn () );

	const QueryParser_i * pQueryParser = pQuery->m_pQueryParser;
	assert ( pQueryParser );

	// fast path for scans
	if ( pQueryParser->IsFullscan ( *pQuery ) )
		return MultiScan ( pQuery, pResult, iSorters, &dSorters[0], tArgs );

	return false;
}

bool PercolateIndex_c::MultiQueryEx ( int iQueries, const CSphQuery * ppQueries, CSphQueryResult ** ppResults,
										ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs &tArgs) const
{
	bool bResult = false;
	for ( int i = 0; i<iQueries; ++i )
		if ( MultiQuery ( &ppQueries[i], ppResults[i], 1, &ppSorters[i], tArgs ) )
			bResult = true;
		else
			ppResults[i]->m_iMultiplier = -1;

	return bResult;
}

void PercolateIndex_c::PostSetup()
{
	PercolateIndex_i::PostSetup();
	m_iMaxCodepointLength = m_pTokenizer->GetMaxCodepointLength();

	// bigram filter
	if ( m_tSettings.m_eBigramIndex!=SPH_BIGRAM_NONE && m_tSettings.m_eBigramIndex!=SPH_BIGRAM_ALL )
	{
		m_pTokenizer->SetBuffer ( (BYTE*)m_tSettings.m_sBigramWords.cstr(), m_tSettings.m_sBigramWords.Length() );

		BYTE * pTok = NULL;
		while ( ( pTok = m_pTokenizer->GetToken() )!=NULL )
			m_tSettings.m_dBigramWords.Add() = (const char*)pTok;

		m_tSettings.m_dBigramWords.Sort();
	}

	// FIXME!!! handle error
	m_pTokenizerIndexing = m_pTokenizer->Clone ( SPH_CLONE_INDEX );
	ISphTokenizerRefPtr_c pIndexing { ISphTokenizer::CreateBigramFilter ( m_pTokenizerIndexing, m_tSettings.m_eBigramIndex, m_tSettings.m_sBigramWords, m_sLastError ) };
	if ( pIndexing )
		m_pTokenizerIndexing = pIndexing;

	if ( m_tSettings.m_uAotFilterMask )
		m_pTokenizerIndexing = sphAotCreateFilter ( m_pTokenizerIndexing, m_pDict, m_tSettings.m_bIndexExactWords
										  , m_tSettings.m_uAotFilterMask );

	// SPZ and zones setup
	if ( m_tSettings.m_bIndexSP && !m_pTokenizerIndexing->EnableSentenceIndexing ( m_sLastError ) )
		m_pTokenizerIndexing = nullptr;
	else if ( !m_tSettings.m_sZones.IsEmpty () && !m_pTokenizerIndexing->EnableZoneIndexing ( m_sLastError ) )
		m_pTokenizerIndexing = nullptr;

	// create queries
	ISphTokenizerRefPtr_c pTokenizer { m_pTokenizer->Clone ( SPH_CLONE_QUERY ) };
	sphSetupQueryTokenizer ( pTokenizer, IsStarDict(), m_tSettings.m_bIndexExactWords, false );

	ISphTokenizerRefPtr_c pTokenizerJson { m_pTokenizer->Clone ( SPH_CLONE_QUERY ) };
	sphSetupQueryTokenizer ( pTokenizerJson, IsStarDict(), m_tSettings.m_bIndexExactWords, true );

	CSphDictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };

	if ( IsStarDict () )
		SetupStarDict ( pDict, pTokenizer );

	if ( m_tSettings.m_bIndexExactWords )
		SetupExactDict ( pDict, pTokenizer );

	CSphString sError;
	ARRAY_FOREACH ( i, m_dLoadedQueries )
	{
		const StoredQueryDesc_t & tQuery = m_dLoadedQueries[i];
		const ISphTokenizer * pTok = tQuery.m_bQL ? pTokenizer : pTokenizerJson;
		PercolateQueryArgs_t tArgs ( tQuery );
		StoredQuery_i * pQuery = AddQuery ( tArgs, pTok, pDict, sError );
		if ( !pQuery || !Commit ( pQuery, sError ) )
			sphWarning ( "index '%s': %d (id=" UINT64_FMT ") query failed to load, ignoring", m_sIndexName.cstr(), i, tQuery.m_uQUID );
	}

	m_dLoadedQueries.Reset ( 0 );
}

bool PercolateIndex_c::Prealloc ( bool bStripPath )
{
	CSphString sLock;
	sLock.SetSprintf ( "%s.lock", m_sFilename.cstr() );
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

	/////////////
	// load meta
	/////////////

	CSphString sMeta;
	sMeta.SetSprintf ( "%s.meta", m_sFilename.cstr() );

	// no readable meta? no disk part yet
	if ( !sphIsReadable ( sMeta.cstr() ) )
		return true;

	// opened and locked, lets read
	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMeta, m_sLastError ) )
		return false;

	if ( rdMeta.GetDword()!=META_HEADER_MAGIC )
	{
		m_sLastError.SetSprintf ( "invalid meta file %s", sMeta.cstr() );
		return false;
	}
	DWORD uVersion = rdMeta.GetDword();
	if ( uVersion==0 || uVersion>META_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%d, binary is v.%d", sMeta.cstr(), uVersion, META_VERSION );
		return false;
	}

	DWORD uIndexVersion = rdMeta.GetDword();

	CSphTokenizerSettings tTokenizerSettings;
	CSphDictSettings tDictSettings;
	CSphEmbeddedFiles tEmbeddedFiles;

	// load settings
	ReadSchema ( rdMeta, m_tSchema, uIndexVersion, false );
	LoadIndexSettings ( m_tSettings, rdMeta, uIndexVersion );
	if ( !LoadTokenizerSettings ( rdMeta, tTokenizerSettings, tEmbeddedFiles, uIndexVersion, m_sLastError ) )
		return false;
	LoadDictionarySettings ( rdMeta, tDictSettings, tEmbeddedFiles, uIndexVersion, m_sLastWarning );

	// initialize AOT if needed
	DWORD uPrevAot = m_tSettings.m_uAotFilterMask;
	m_tSettings.m_uAotFilterMask = sphParseMorphAot ( tDictSettings.m_sMorphology.cstr() );
	if ( m_tSettings.m_uAotFilterMask!=uPrevAot )
		sphWarning ( "index '%s': morphology option changed from config has no effect, ignoring", m_sIndexName.cstr() );

	if ( bStripPath )
	{
		StripPath ( tTokenizerSettings.m_sSynonymsFile );
		StripPath ( tDictSettings.m_sStopwords );
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
			StripPath ( tDictSettings.m_dWordforms[i] );
	}

	// recreate tokenizer
	m_pTokenizer = ISphTokenizer::Create ( tTokenizerSettings, &tEmbeddedFiles, m_sLastError );
	if ( !m_pTokenizer )
		return false;

	// recreate dictionary
	m_pDict = sphCreateDictionaryCRC ( tDictSettings, &tEmbeddedFiles, m_pTokenizer, m_sIndexName.cstr(), m_sLastError );
	if ( !m_pDict )
	{
		m_sLastError.SetSprintf ( "index '%s': %s", m_sIndexName.cstr(), m_sLastError.cstr() );
		return false;
	}

	m_pTokenizer = ISphTokenizer::CreateMultiformFilter ( m_pTokenizer, m_pDict->GetMultiWordforms () );

	// regexp and RLP
	if ( uVersion>=6 )
	{
		ISphFieldFilterRefPtr_c pFieldFilter;
		CSphFieldFilterSettings tFieldFilterSettings;
		LoadFieldFilterSettings ( rdMeta, tFieldFilterSettings );
		if ( tFieldFilterSettings.m_dRegexps.GetLength() )
			pFieldFilter = sphCreateRegexpFilter ( tFieldFilterSettings, m_sLastError );

		if ( !sphSpawnRLPFilter ( pFieldFilter, m_tSettings, tTokenizerSettings, sMeta.cstr(), m_sLastError ) )
			return false;

		SetFieldFilter ( pFieldFilter );
	}

	// queries
	DWORD uQueries = rdMeta.GetDword();
	m_dLoadedQueries.Reset ( uQueries );
	ARRAY_FOREACH ( i, m_dLoadedQueries )
	{
		StoredQueryDesc_t & tQuery = m_dLoadedQueries[i];
		if ( uVersion<7 )
			LoadStoredQueryV6 ( uVersion, tQuery, rdMeta );
		else
			LoadStoredQuery ( uVersion, tQuery, rdMeta );
	}
	if ( uVersion>=7 )
		m_iTID = rdMeta.GetOffset();

	m_iSavedTID = m_iTID;
	m_tmSaved = sphMicroTimer();

	return true;
}

void PercolateIndex_c::SaveMeta ( bool bShutdown )
{
	// sanity check
	if ( m_iLockFD<0 || m_bSaveDisabled )
		return;

	// write new meta
	CSphString sMeta, sMetaNew;
	sMeta.SetSprintf ( "%s.meta", m_sFilename.cstr() );
	sMetaNew.SetSprintf ( "%s.meta.new", m_sFilename.cstr() );

	CSphString sError;
	CSphWriter wrMeta;
	if ( !wrMeta.OpenFile ( sMetaNew, sError ) )
	{
		sphWarning ( "failed to serialize meta: %s", sError.cstr() );
		return;
	}

	wrMeta.PutDword ( META_HEADER_MAGIC );
	wrMeta.PutDword ( META_VERSION );
	wrMeta.PutDword ( INDEX_FORMAT_VERSION );

	WriteSchema ( wrMeta, m_tSchema );
	SaveIndexSettings ( wrMeta, m_tSettings );
	SaveTokenizerSettings ( wrMeta, m_pTokenizer, m_tSettings.m_iEmbeddedLimit );
	SaveDictionarySettings ( wrMeta, m_pDict, false, m_tSettings.m_iEmbeddedLimit );

	// meta v.6
	SaveFieldFilterSettings ( wrMeta, m_pFieldFilter );

	Verify ( m_tLock.ReadLock() );
	wrMeta.PutDword ( m_dStored.GetLength() );

	ARRAY_FOREACH ( i, m_dStored )
	{
		const StoredQuery_t * pQuery = m_dStored[i].m_pQuery;
		SaveStoredQuery ( *pQuery, wrMeta );
	}

	wrMeta.PutOffset ( m_iTID );

	if ( GetBinlog() )
		GetBinlog()->NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, bShutdown );

	m_iSavedTID = m_iTID;
	m_tmSaved = sphMicroTimer();

	m_tLock.Unlock();

	wrMeta.CloseFile();

	// rename
	if ( sph::rename ( sMetaNew.cstr(), sMeta.cstr() ) )
		sphWarning ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)", sMetaNew.cstr(), sMeta.cstr(), errno, strerrorm( errno ) );
}

void PercolateIndex_c::GetQueries ( const char * sFilterTags, bool bTagsEq, const CSphFilterSettings * pUID, int iOffset, int iLimit, CSphVector<PercolateQueryDesc> & dQueries )
{
	// FIXME!!! move to filter, add them via join
	CSphVector<uint64_t> dTags;
	PercolateTags ( sFilterTags, dTags );

	// FIXME!!! add UID scan for UID IN (value list) queries
	CSphScopedPtr<PercolateFilter_i> tFilter ( CreatePercolateFilter ( pUID ) );

	// reserve size to store all queries
	if ( !dTags.GetLength() && !tFilter.Ptr() )
		dQueries.Reserve ( m_dStored.GetLength() );

	StringBuilder_c tBuf;
	ScRL_t rLock ( m_tLock );

	int iFrom = 0;
	if ( iLimit>0 && iOffset>0 )
		iFrom = Min ( iOffset, m_dStored.GetLength() );

	for ( int i=iFrom; i<m_dStored.GetLength(); ++i )
	{
		const StoredQuery_t * pQuery = m_dStored[i].m_pQuery;
		if ( dTags.GetLength() )
		{
			if ( !pQuery->m_dTags.GetLength() )
				continue;

			if ( !TagsMatched ( dTags, pQuery->m_dTags, bTagsEq ) )
				continue;
		}

		if ( tFilter.Ptr() && !tFilter->Eval ( pQuery->m_uQUID ) )
			continue;

		PercolateQueryDesc & tItem = dQueries.Add();
		tItem.m_uQID = pQuery->m_uQUID;
		tItem.m_sQuery = pQuery->m_sQuery;
		tItem.m_sTags = pQuery->m_sTags;
		tItem.m_bQL = pQuery->m_bQL;

		if ( pQuery->m_dFilters.GetLength() )
		{
			tBuf.Clear();
			FormatFiltersQL ( pQuery->m_dFilters, pQuery->m_dFilterTree, tBuf );
			tItem.m_sFilters = tBuf.cstr();
		}

		if ( iLimit>0 && dQueries.GetLength()==iLimit )
			break;
	}
}

bool PercolateIndex_c::Truncate ( CSphString & sError )
{
	{
		ScWL_t wLock ( m_tLock );
		for ( auto& dStored : m_dStored )
			SafeDelete ( dStored.m_pQuery );
		m_dStored.Reset();
	}

	// update and save meta
	// current TID will be saved, so replay will properly skip preceding txns
	// FIXME!!! however it should be replicated to cluster maybe with TOI
	SaveMeta ( false );

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
{
	m_bGetDocs = rhs.m_bGetDocs;
	m_bGetQuery = rhs.m_bGetQuery;
	m_bGetFilters = rhs.m_bGetFilters;

	m_iQueriesMatched = rhs.m_iQueriesMatched;
	m_iQueriesFailed = rhs.m_iQueriesFailed;
	m_iDocsMatched = rhs.m_iDocsMatched;
	m_tmTotal = rhs.m_tmTotal;

	m_bVerbose = rhs.m_bVerbose;
	m_iEarlyOutQueries = rhs.m_iEarlyOutQueries;
	m_iTotalQueries = rhs.m_iTotalQueries;
	m_iOnlyTerms = rhs.m_iOnlyTerms;
	m_tmSetup = rhs.m_tmSetup;
	m_sMessages = std::move ( rhs.m_sMessages );

	m_dQueryDesc = std::move ( rhs.m_dQueryDesc );
	m_dDocs = std::move ( rhs.m_dDocs );
	m_dQueryDT = std::move ( rhs.m_dQueryDT );
}

PercolateMatchResult_t&PercolateMatchResult_t::operator= ( PercolateMatchResult_t &&rhs ) noexcept
{
	if ( &rhs != this )
	{
		m_bGetDocs = rhs.m_bGetDocs;
		m_bGetQuery = rhs.m_bGetQuery;
		m_bGetFilters = rhs.m_bGetFilters;

		m_iQueriesMatched = rhs.m_iQueriesMatched;
		m_iQueriesFailed = rhs.m_iQueriesFailed;
		m_iDocsMatched = rhs.m_iDocsMatched;
		m_tmTotal = rhs.m_tmTotal;

		m_bVerbose = rhs.m_bVerbose;
		m_iEarlyOutQueries = rhs.m_iEarlyOutQueries;
		m_iTotalQueries = rhs.m_iTotalQueries;
		m_iOnlyTerms = rhs.m_iOnlyTerms;
		m_tmSetup = rhs.m_tmSetup;
		m_sMessages = std::move ( rhs.m_sMessages );

		m_dQueryDesc = std::move ( rhs.m_dQueryDesc );
		m_dDocs = std::move ( rhs.m_dDocs );
		m_dQueryDT = std::move ( rhs.m_dQueryDT );
	}
	return *this;
}


void FixPercolateSchema ( CSphSchema & tSchema )
{
	if ( !tSchema.GetFieldsCount() )
		tSchema.AddField ( CSphColumnInfo ( "text" ) );

	if ( !tSchema.GetAttrsCount() )
	{
		CSphColumnInfo tCol ( "gid", SPH_ATTR_INTEGER );
		tCol.m_tLocator = CSphAttrLocator();
		tSchema.AddAttr ( tCol, false );
	}
}

bool PercolateIndex_c::IsSameSettings ( CSphReconfigureSettings & tSettings, CSphReconfigureSetup & tSetup, CSphString & sError ) const
{
	tSetup.m_tSchema = tSettings.m_tSchema;
	FixPercolateSchema ( tSetup.m_tSchema );

	CSphString sTmp;
	bool bSameSchema = m_tSchema.CompareTo ( tSettings.m_tSchema, sTmp, false );

	return CreateReconfigure ( m_sIndexName, IsStarDict(), m_pFieldFilter, m_tSettings,
		m_pTokenizer->GetSettingsFNV(), m_pDict->GetSettingsFNV(), m_pTokenizer->GetMaxCodepointLength(), bSameSchema, tSettings, tSetup, sError );
}

void PercolateIndex_c::Reconfigure ( CSphReconfigureSetup & tSetup )
{
	if ( GetBinlog() )
		GetBinlog()->BinlogReconfigure ( &m_iTID, m_sIndexName.cstr(), tSetup );

	m_tSchema = tSetup.m_tSchema;

	Setup ( tSetup.m_tIndex );
	SetTokenizer ( tSetup.m_pTokenizer );
	SetDictionary ( tSetup.m_pDict );
	SetFieldFilter ( tSetup.m_pFieldFilter );

	m_iMaxCodepointLength = m_pTokenizer->GetMaxCodepointLength();
	SetupQueryTokenizer();

	m_dLoadedQueries.Reset ( m_dStored.GetLength() );
	ARRAY_FOREACH ( i, m_dLoadedQueries )
	{
		StoredQueryDesc_t & tQuery = m_dLoadedQueries[i];
		const StoredQuery_t * pStored = m_dStored[i].m_pQuery;

		tQuery.m_uQUID = pStored->m_uQUID;
		tQuery.m_sQuery = pStored->m_sQuery;
		tQuery.m_sTags = pStored->m_sTags;
		tQuery.m_dFilters = pStored->m_dFilters;
		tQuery.m_dFilterTree = pStored->m_dFilterTree;

		SafeDelete ( pStored );
	}
	m_dStored.Resize ( 0 );

	PostSetup();
}

void SetPercolateThreads ( int iThreads )
{
	g_iPercolateThreads = Max ( 1, iThreads );
}

void PercolateIndex_c::ForceRamFlush ( bool bPeriodic )
{
	if ( m_iTID<=m_iSavedTID )
		return;

	RamFlush ( bPeriodic );
}

void PercolateIndex_c::RamFlush ( bool bPeriodic )
{
	int64_t tmStart = sphMicroTimer();
	int64_t iWasTID = m_iSavedTID;
	int64_t tmWas = m_tmSaved;
	SaveMeta ( false );

	int64_t tmNow = sphMicroTimer();
	int64_t tmAge = tmNow - tmWas;
	int64_t tmSave = tmNow - tmStart;

	sphInfo ( "percolate: index %s: saved ok (mode=%s, last TID=" INT64_FMT ", current TID=" INT64_FMT ", "
		"time delta=%d sec, took=%d.%03d sec)"
		, m_sIndexName.cstr(), bPeriodic ? "periodic" : "forced"
		, iWasTID, m_iTID
		, (int) (tmAge/1000000), (int)(tmSave/1000000), (int)((tmSave/1000)%1000) );
}

void PercolateIndex_c::ForceDiskChunk ()
{
	// right after cluster started with new indexes we need its files on disk to issue sync command
	if ( !m_iSavedTID )
		RamFlush ( false );
	else
		ForceRamFlush ( false );
}

void PercolateIndex_c::CheckRamFlush ()
{
	if ( ( sphMicroTimer()-m_tmSaved )/1000000<GetRtFlushPeriod() )
		return;
	if ( GetBinlog()->IsActive() && m_iTID<=m_iSavedTID )
		return;

	ForceRamFlush ( true );
}


PercolateQueryArgs_t::PercolateQueryArgs_t ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> & dFilterTree )
	: m_dFilters ( dFilters )
	, m_dFilterTree ( dFilterTree )
{}

PercolateQueryArgs_t::PercolateQueryArgs_t ( const StoredQueryDesc_t & tDesc )
	: PercolateQueryArgs_t ( tDesc.m_dFilters, tDesc.m_dFilterTree )
{
	m_sQuery = tDesc.m_sQuery.cstr();
	m_sTags = tDesc.m_sTags.cstr();
	m_uQUID = tDesc.m_uQUID;
	m_bQL = tDesc.m_bQL;
}

// stuff for merging several results into one
struct PQMergeResultsIterator_t
{
	CPqResult * m_pResult = nullptr;
	int m_iIdx = 0;
	int m_iElems = 0;
	int* m_pDocs = nullptr;

	PQMergeResultsIterator_t ( CPqResult * pMatch = nullptr )
		: m_pResult ( pMatch )
	{
		if ( !pMatch )
			return;
		m_iElems = pMatch->m_dResult.m_dQueryDesc.GetLength ();
		m_pDocs = pMatch->m_dResult.m_dDocs.begin();
	}

	inline PercolateQueryDesc &CurDesc () const
	{ return m_pResult->m_dResult.m_dQueryDesc[m_iIdx]; }

	inline int CurDt () const
	{ return m_pResult->m_dResult.m_dQueryDT[m_iIdx]; }

	static inline bool IsLess ( const PQMergeResultsIterator_t &a, const PQMergeResultsIterator_t &b )
	{
		return a.CurDesc ().m_uQID<b.CurDesc ().m_uQID;
	}
};

void MergePqResults ( const VecTraits_T<CPqResult *> &dChunks, CPqResult &dRes, bool bSharded )
{
	if ( dChunks.IsEmpty () )
		return;

	// check if we have exactly one non-null and non-empty result
	if ( dChunks.GetLength ()==1 ) // short path for only 1 result.
	{
		dRes = std::move ( *dChunks[0] );
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

		qMatches.Push ( pChunk );
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

	CSphHash<int> hDocids ( iGotDocids+1 );
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
					pDocs[i] = hDocids.FindOrAdd ( tMin.m_pResult->m_dDocids[tMin.m_pDocs[i]], hDocids.GetLength () );
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
		int i = 0;
		int64_t iDocid;
		int * pIndex;
		while ( nullptr != ( pIndex = hDocids.Iterate ( &i, &iDocid ) ) )
			dRes.m_dDocids[*pIndex] = iDocid;

	}
}
