//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>
#include "sphinxint.h"
#include "sphinxsearch.h"
#include "sphinxfilter.h"
#include "tokenizer/tokenizer.h"
#include "dict/dict_base.h"
#include "searchnode.h"
#include "querycontext.h"
#include "client_task_info.h"
#include "killlist.h"
#include "secondaryindex.h"
#include "coroutine.h"
#include "mini_timer.h"
#include "diskqword.h"
#include <algorithm>
#include <set>
#include <vector>

namespace
{
using Docs_t = std::vector<QwordDoc_t>;

class WindowTestQword_c final : public ISphQword
{
public:
	explicit WindowTestQword_c ( const Docs_t & dDocs, int * pLiveQwords=nullptr )
		: m_dDocs(dDocs)
		, m_pLiveQwords(pLiveQwords)
	{
		if ( m_pLiveQwords )
			++*m_pLiveQwords;
	}

	~WindowTestQword_c() override
	{
		if ( m_pLiveQwords )
			--*m_pLiveQwords;
	}
	const CSphMatch & GetNextDoc() override { ADD_FAILURE() << "expected batched decode"; return m_tMatch; }
	void SeekHitlist ( SphOffset_t ) override { ADD_FAILURE() << "count must not read hits"; }
	Hitpos_t GetNextHit() override { ADD_FAILURE() << "count must not read hits"; return 0; }
	int GetNextDocs ( QwordDoc_t * pDocs, int iMax ) override
	{
		++m_iBatchCalls;
		int iGot = 0;
		while ( iGot<iMax && m_iPos<m_dDocs.size() )
			pDocs[iGot++] = m_dDocs[m_iPos++];
		return iGot;
	}
	bool HintRowID ( RowID_t tRow ) override
	{
		++m_iHintCalls;
		while ( m_iPos<m_dDocs.size() && m_dDocs[m_iPos].m_tRowID<tRow )
			++m_iPos;
		return true;
	}
	void Reset() override { ISphQword::Reset(); m_iPos = 0; }
	const Docs_t & m_dDocs;
	int m_iBatchCalls = 0;
	int m_iHintCalls = 0;
private:
	size_t m_iPos = 0;
	CSphMatch m_tMatch;
	int * m_pLiveQwords;
};

class WindowTestSetup_c final : public ISphQwordSetup
{
public:
	Docs_t m_dAlpha, m_dBeta, m_dEmpty;
	mutable int m_iLiveQwords = 0;
	ISphQword * QwordSpawn ( const XQKeyword_t & tWord ) const override
	{
		return new WindowTestQword_c ( tWord.m_sWord=="alpha" ? m_dAlpha : tWord.m_sWord=="beta" ? m_dBeta : m_dEmpty, &m_iLiveQwords );
	}
	bool QwordSetup ( ISphQword * pWord ) const override
	{
		pWord->m_iDocs = int ( static_cast<WindowTestQword_c*>(pWord)->m_dDocs.size() );
		pWord->m_iHits = pWord->m_iDocs;
		return pWord->m_iDocs!=0;
	}
	ISphQword * ScanSpawn ( int ) const override { return nullptr; }
};

class WindowTestIterator_c final : public RowidIterator_i
{
public:
	std::vector<RowID_t> m_dRows;
	bool HintRowID ( RowID_t tRow ) override
	{
		while ( m_iPos<m_dRows.size() && m_dRows[m_iPos]<tRow ) ++m_iPos;
		return m_iPos<m_dRows.size();
	}
	bool GetNextRowIdBlock ( RowIdBlock_t & dRows ) override
	{
		if ( m_iPos==m_dRows.size() ) return false;
		const auto iCount = std::min ( size_t(47), m_dRows.size()-m_iPos );
		dRows = RowIdBlock_t ( m_dRows.data()+m_iPos, iCount );
		m_iPos += iCount;
		return true;
	}
	int64_t GetNumProcessed() const override { return m_iPos; }
	void SetCutoff ( int ) override {}
	bool WasCutoffHit() const override { return false; }
	void AddDesc ( CSphVector<IteratorDesc_t> & ) const override {}
private:
	size_t m_iPos = 0;
};

// A window producer independent of OR/posting cursors, used to verify adaptation.
class AdapterTestWindow_c final : public WindowNode_i
{
public:
	AdapterTestWindow_c ( bool & bContinue, int & iDestroyed )
		: m_bContinue(bContinue)
		, m_iDestroyed(iDestroyed)
	{
		for ( int i=0; i<256; ++i )
			m_dRows[i] = i;
	}

	~AdapterTestWindow_c() override { ++m_iDestroyed; }
	bool CheckContinue() override { return m_bContinue; }
	CountWindow_t * NextWindow() override
	{
		if ( !CheckContinue() || m_iWindow==2 )
			return nullptr;

		m_tWindow = {};
		if ( m_iWindow++==0 )
		{
			m_tWindow.m_pRows = m_dRows;
			m_tWindow.m_iRows = 256;
		}
		else
		{
			m_tWindow.m_tBase = 4096;
			for ( int i=0; i<8; ++i )
				m_tWindow.m_dBits[i] = ~DWORD(0);
		}

		return &m_tWindow;
	}

	void Reset ( const ISphQwordSetup & ) override { m_iWindow = 0; }
	void HintRowID ( RowID_t ) override {}
	void SetRowidBoundaries ( const RowIdBoundaries_t & ) override {}
	int GetQwords ( ExtQwordsHash_t & ) override { return 0; }
	void SetQwordsIDF ( const ExtQwordsHash_t & ) override {}
	void GetTerms ( const ExtQwordsHash_t &, CSphVector<TermPos_t> & ) const override {}
	bool GotHitless() override { return false; }
	uint64_t GetWordID() const override { return 0; }
	void SetAtomPos ( int iPos ) override { m_iAtomPos = iPos; }
	int GetAtomPos() const override { return m_iAtomPos; }
	NodeEstimate_t Estimate ( int64_t ) const override { return { 1.0f, 512, 1 }; }
	void DebugDump ( int ) override {}
	bool TimeExceeded() const override { return !m_bContinue; }
	int64_t GetMaxTimeout() const override { return 0; }

private:
	bool & m_bContinue;
	int & m_iDestroyed;
	int m_iWindow = 0;
	int m_iAtomPos = 0;
	CountWindow_t m_tWindow;
	RowID_t m_dRows[256];
};

std::vector<RowID_t> ReadWindows ( QueryNode_i & tNode )
{
	std::vector<RowID_t> dResult;
	while ( auto * pWindow = tNode.GetCountWindows()->NextWindow() )
	{
		if ( pWindow->m_pRows )
			dResult.insert ( dResult.end(), pWindow->m_pRows, pWindow->m_pRows+pWindow->m_iRows );
		else
			for ( int i=0; i<CountWindow_t::ROWS; ++i )
				if ( pWindow->m_dBits[i>>5] & (DWORD(1)<<(i&31)) )
					dResult.push_back ( pWindow->m_tBase+i );
	}
	return dResult;
}

std::vector<RowID_t> ReadChunks ( ExtNode_i & tNode )
{
	std::vector<RowID_t> dResult;
	while ( const auto * pDocs = tNode.GetDocsChunk() )
		for ( ; pDocs->m_tRowID!=INVALID_ROWID; ++pDocs )
			dResult.push_back ( pDocs->m_tRowID );
	return dResult;
}
}

TEST ( WindowCount, BufferedUnionHintsResetAndBounds )
{
	Threads::CallCoroutine ( [] {
		WindowTestSetup_c tSetup;
		CSphString sError;
		CSphDictSettings tDictSettings;
		tDictSettings.SetDictFormat ( DictFormat_e::CRC );
		auto pTok = Tokenizer::Detail::CreateUTF8Tokenizer();
		auto pDict = sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "window-test", false, 32, nullptr, sError );
		ASSERT_TRUE ( pDict ) << sError.cstr();
		tSetup.SetDict ( pDict );
		auto pIndex = sphCreateIndexPhrase ( "window-test", "unused-window-index" );
		tSetup.m_pIndex = pIndex.get();

		std::set<RowID_t> dExpected;
		for ( RowID_t i=0; i<10000; ++i )
		{
			if ( i%3==0 ) tSetup.m_dAlpha.push_back ( { i, i%5 ? DWORD(1) : DWORD(2), 1, 0 } );
			if ( i%7==0 ) tSetup.m_dBeta.push_back ( { i, 1, 1, 0 } );
			if ( (i%3==0 && i%5!=0) || i%7==0 ) dExpected.insert(i);
		}
		for ( RowID_t i : { RowID_t(65536), INVALID_ROWID-2, INVALID_ROWID-1 } )
		{
			tSetup.m_dAlpha.push_back ( { i, 1, 1, 0 } );
			dExpected.insert(i);
		}
		XQLimitSpec_t tSpec;
		tSpec.m_bFieldSpec = true;
		tSpec.m_dFieldMask.UnsetAll();
		tSpec.m_dFieldMask.Set(0);
		XQNode_t tRoot ( tSpec );
		tRoot.SetOp ( SPH_QUERY_OR );
		tRoot.AddDirtyWord ( { "alpha", 1 } );
		tRoot.AddDirtyWord ( { "beta", 2 } );
		tRoot.AddDirtyWord ( { "alpha", 3 } );
		tRoot.AddDirtyWord ( { "absent", 4 } );
		std::unique_ptr<QueryNode_i> pNode ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
		ASSERT_TRUE ( pNode );
		ASSERT_TRUE ( pNode->GetCountWindows() );
		const std::vector<RowID_t> dAll ( dExpected.begin(), dExpected.end() );
		EXPECT_EQ ( ReadWindows ( *pNode ), dAll );

		// EOF must not destroy dictionary statistics, including repeated terms.
		ExtQwordsHash_t hWords;
		pNode->GetQwords ( hWords );
		ASSERT_TRUE ( hWords("alpha") );
		EXPECT_EQ ( hWords["alpha"].m_iDocs, tSetup.m_dAlpha.size() );
		EXPECT_EQ ( hWords["alpha"].m_iQueryPos, 1 );

		auto pAdapter = CreateWindowDocsAdapter ( std::move(pNode) );
		EXPECT_EQ ( pAdapter->GetCountWindows(), nullptr );
		pAdapter->Reset ( tSetup );
		EXPECT_EQ ( ReadChunks ( *pAdapter ), dAll );
		for ( RowID_t tHint : { RowID_t(31), RowID_t(127), RowID_t(4095), RowID_t(4096), RowID_t(8193), INVALID_ROWID-1, INVALID_ROWID } )
		{
			pAdapter->Reset ( tSetup );
			ASSERT_TRUE ( pAdapter->GetDocsChunk() ); // retained window and lookahead batches
			pAdapter->HintRowID ( tHint );
			const RowID_t tFloor = Max ( tHint, dAll[MAX_BLOCK_DOCS-2]+1 );
			EXPECT_EQ ( ReadChunks ( *pAdapter ), std::vector<RowID_t> ( dExpected.lower_bound(tFloor), dExpected.end() ) );
		}

		pNode = pAdapter->ReleaseSource();

		pNode->Reset ( tSetup );
		RowIdBoundaries_t tBounds { 127, 4096 };
		pNode->SetRowidBoundaries ( tBounds );
		const std::vector<RowID_t> dBounded ( dExpected.lower_bound(127), dExpected.upper_bound(4096) );
		EXPECT_EQ ( ReadWindows ( *pNode ), dBounded );
		pNode->Reset ( tSetup );
		EXPECT_EQ ( ReadWindows ( *pNode ), dBounded );

		// A real pseudo-FT wrapper must intersect through the chunk adapter,
		// not expose the unfiltered root's bitmap to CountDocs.
		WindowTestIterator_c tIterator;
		std::vector<RowID_t> dIntersection;
		for ( RowID_t i=0; i<10000; i+=13 )
		{
			tIterator.m_dRows.push_back(i);
			if ( dExpected.count(i) ) dIntersection.push_back(i);
		}
		std::unique_ptr<QueryNode_i> pWrappedNode ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
		auto pWrapper = CreatePseudoFTNode ( CreateWindowDocsAdapter ( std::move(pWrappedNode) ).release(), &tIterator );
		EXPECT_EQ ( pWrapper->GetCountWindows(), nullptr );
		EXPECT_EQ ( ReadChunks ( *pWrapper ), dIntersection );

		// Unsupported operators must fall back before creating/consuming readers.
		tRoot.SetOp ( SPH_QUERY_AND );
		EXPECT_EQ ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ), nullptr );
		tRoot.SetOp ( SPH_QUERY_OR );
		tRoot.WithWord ( 0, [] ( XQKeyword_t & tWord ) { tWord.m_bFieldStart = true; } );
		EXPECT_EQ ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ), nullptr );
		tRoot.ResetWords();
		for ( int i=0; i<32; ++i ) tRoot.AddDirtyWord ( { "absent", i+1 } );
		std::unique_ptr<QueryNode_i> pEmpty ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
		ASSERT_TRUE ( pEmpty );
		EXPECT_TRUE ( ReadWindows ( *pEmpty ).empty() );
		tRoot.AddDirtyWord ( { "absent", 33 } );
		EXPECT_EQ ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ), nullptr );

		tRoot.ResetWords();
		tRoot.AddDirtyWord ( { "alpha", 1 } );
		tRoot.AddDirtyWord ( { "beta", 2 } );
		tSetup.m_pWarning = &sError;
		tSetup.m_iMaxTimer = sph::MicroTimer()-10000;
		std::unique_ptr<QueryNode_i> pExpired ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
		ASSERT_TRUE ( pExpired );
		EXPECT_TRUE ( ReadWindows ( *pExpired ).empty() );
		EXPECT_STREQ ( sError.cstr(), "query time exceeded max_query_time" );
	} );
}

TEST ( WindowCount, SingleClauseRowsMixedWithBitmaps )
{
	Threads::CallCoroutine ( [] {
		WindowTestSetup_c tSetup;
		CSphString sError;
		CSphDictSettings tDictSettings;
		tDictSettings.SetDictFormat ( DictFormat_e::CRC );
		auto pTok = Tokenizer::Detail::CreateUTF8Tokenizer();
		auto pDict = sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "window-single-test", false, 32, nullptr, sError );
		ASSERT_TRUE(pDict) << sError.cstr();
		tSetup.SetDict(pDict);
		auto pIndex = sphCreateIndexPhrase ( "window-single-test", "unused-window-index" );
		tSetup.m_pIndex = pIndex.get();
		std::set<RowID_t> dExpected;
		auto Add = [&] ( Docs_t & dDocs, RowID_t tRow, DWORD uFields ) {
			dDocs.push_back ( { tRow, uFields, 1, 0 } );
			if ( uFields&1 ) dExpected.insert(tRow);
		};
		for ( RowID_t i=0; i<600; ++i )
		{
			Add ( tSetup.m_dAlpha, i, i%5 ? DWORD(1) : DWORD(2) );
			if ( i==127 ) Add ( tSetup.m_dAlpha, i, 1 ); // duplicate across a decode boundary
		}
		for ( RowID_t i=4096; i<4200; ++i )
		{
			Add ( tSetup.m_dAlpha, i, 1 );
			Add ( tSetup.m_dBeta, i+32, 1 );
		}
		for ( RowID_t i=8192; i<8400; ++i ) Add ( tSetup.m_dBeta, i, 1 );
		for ( RowID_t i=12288; i<12500; ++i ) Add ( tSetup.m_dAlpha, i, 2 ); // filtered-empty window
		for ( RowID_t i : { RowID_t(65536), INVALID_ROWID-2, INVALID_ROWID-1 } ) Add ( tSetup.m_dAlpha, i, 1 );

		XQLimitSpec_t tSpec;
		tSpec.m_bFieldSpec = true;
		tSpec.m_dFieldMask.UnsetAll();
		tSpec.m_dFieldMask.Set(0);
		XQNode_t tRoot(tSpec);
		tRoot.SetOp(SPH_QUERY_OR);
		tRoot.AddDirtyWord ( { "alpha", 1 } );
		tRoot.AddDirtyWord ( { "beta", 2 } );
		tRoot.AddDirtyWord ( { "absent", 3 } );
		std::unique_ptr<QueryNode_i> pNode ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
		ASSERT_TRUE(pNode);
		const std::vector<RowID_t> dAll ( dExpected.begin(), dExpected.end() );
		EXPECT_EQ ( ReadWindows(*pNode), dAll );
		pNode->Reset(tSetup);
		auto * pWindow = pNode->GetCountWindows()->NextWindow();
		ASSERT_TRUE(pWindow);
		ASSERT_NE ( pWindow->m_pRows, nullptr );
		EXPECT_EQ ( pWindow->m_iRows, 128 );
		std::fill ( std::begin(pWindow->m_dBits), std::end(pWindow->m_dBits), 0xa5a5a5a5 );
		pWindow = pNode->GetCountWindows()->NextWindow();
		ASSERT_TRUE(pWindow);
		ASSERT_NE ( pWindow->m_pRows, nullptr );
		for ( auto u : pWindow->m_dBits ) EXPECT_EQ ( u, 0xa5a5a5a5u ); // no bitmap work
		int iBitmaps = 0;
		while ( ( pWindow = pNode->GetCountWindows()->NextWindow() ) )
			if ( !pWindow->m_pRows )
			{
				++iBitmaps;
				EXPECT_EQ ( pWindow->m_tBase, 4096u );
			}
		EXPECT_EQ ( iBitmaps, 1 );

		auto pAdapter = CreateWindowDocsAdapter ( std::move(pNode) );
		pAdapter->Reset(tSetup);
		EXPECT_EQ ( ReadChunks(*pAdapter), dAll );
		for ( RowID_t tHint : { RowID_t(31), RowID_t(127), RowID_t(128), RowID_t(511), RowID_t(4096), RowID_t(8193), INVALID_ROWID-1, INVALID_ROWID } )
		{
			pAdapter->Reset(tSetup);
			ASSERT_TRUE ( pAdapter->GetDocsChunk() );
			pAdapter->HintRowID(tHint);
			const RowID_t tFloor = Max ( tHint, dAll[MAX_BLOCK_DOCS-2]+1 );
			EXPECT_EQ ( ReadChunks(*pAdapter), std::vector<RowID_t> ( dExpected.lower_bound(tFloor), dExpected.end() ) );
		}
		pNode = pAdapter->ReleaseSource();

		pNode->Reset(tSetup);
		pNode->SetRowidBoundaries ( { 127, 8300 } );
		const std::vector<RowID_t> dBounded ( dExpected.lower_bound(127), dExpected.upper_bound(8300) );
		EXPECT_EQ ( ReadWindows(*pNode), dBounded );
		pNode->Reset(tSetup);
		pAdapter = CreateWindowDocsAdapter ( std::move(pNode) );
		EXPECT_EQ ( ReadChunks(*pAdapter), dBounded );

		WindowTestIterator_c tIterator;
		std::vector<RowID_t> dIntersection;
		for ( RowID_t i=0; i<13000; i+=13 )
		{
			tIterator.m_dRows.push_back(i);
			if ( dExpected.count(i) ) dIntersection.push_back(i);
		}
		std::unique_ptr<QueryNode_i> pUnwrapped ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
		auto pWrapper = CreatePseudoFTNode ( CreateWindowDocsAdapter ( std::move(pUnwrapped) ).release(), &tIterator );
		EXPECT_EQ ( pWrapper->GetCountWindows(), nullptr );
		EXPECT_EQ ( ReadChunks(*pWrapper), dIntersection );
	} );
}

TEST ( PostingWindow, SharedCursorProbeRowsAndBitmap )
{
	Docs_t dDocs;
	for ( RowID_t i=0; i<300; ++i )
	{
		dDocs.push_back ( { i, i%5 ? DWORD(1) : DWORD(2), 1, 0 } );
		if ( i==127 ) dDocs.push_back ( { i, 1, 1, 0 } );
	}
	dDocs.push_back ( { 4096, 1, 1, 0 } );
	dDocs.push_back ( { 9000, 1, 1, 0 } );
	WindowTestQword_c tWord(dDocs);
	tWord.m_iDocs = int(dDocs.size());
	CSphQueryStats tStats;
	auto p = CreateBufferedPostingWindow ( tWord, 1, { nullptr, nullptr, &tStats } );
	EXPECT_EQ ( p->AdvanceTo(0).m_tNext, 0u ); // conservative candidate, wrong field
	auto tProbe = p->Probe(0);
	EXPECT_FALSE ( tProbe.m_bMatch );
	EXPECT_EQ ( tProbe.m_tState.m_tNext, 1u );
	EXPECT_TRUE ( p->Probe(1).m_bMatch );
	EXPECT_TRUE ( p->Probe(1).m_bMatch ); // successful probes retain their row
	EXPECT_EQ ( tWord.m_iBatchCalls, 1 );
	EXPECT_EQ ( tWord.m_iHintCalls, 0 );

	RowID_t dRows[17];
	auto tRows = p->ReadRows ( { 0, 1, 4096 }, dRows, 17 );
	ASSERT_EQ ( tRows.m_iRows, 17 );
	EXPECT_EQ ( dRows[0], 1u );
	EXPECT_EQ ( dRows[16], 21u );
	EXPECT_EQ ( tRows.m_tNext, 22u );
	EXPECT_TRUE ( p->Probe(22).m_bMatch );
	EXPECT_EQ ( p->AdvanceTo(127).m_tNext, 127u );
	EXPECT_EQ ( tWord.m_iHintCalls, 0 ); // retained batch; never seek underlying qword
	tRows = p->ReadRows ( { 0, 127, 4096 }, dRows, 1 );
	ASSERT_EQ ( tRows.m_iRows, 1 );
	EXPECT_EQ ( dRows[0], 127u );
	EXPECT_EQ ( tRows.m_tNext, 128u ); // duplicate straddles the refill boundary

	DWORD dBits[PostingWindow_t::WORDS] = {};
	dBits[0] = 1; // FillOr preserves existing bits
	auto tFill = p->FillOr ( { 0, 128, 4096 }, dBits );
	EXPECT_FALSE ( tFill.m_bStopped );
	EXPECT_EQ ( tFill.m_tNext, 4096u );
	int iExpected = 0;
	for ( int i=0; i<PostingWindow_t::ROWS; ++i )
	{
		const bool bExpected = i==0 || (i>=128 && i<300 && i%5!=0);
		EXPECT_EQ ( (dBits[i>>5] & (DWORD(1)<<(i&31)))!=0, bExpected ) << i;
		if ( i>=128 && bExpected ) ++iExpected;
	}
	EXPECT_EQ ( tFill.m_iRows, iExpected );
	EXPECT_TRUE ( p->Probe(4096).m_bMatch ); // FillOr did not consume the boundary
	tRows = p->ReadRows ( { 4096, 4096, 8192 }, dRows, 17 );
	EXPECT_EQ ( tRows.m_iRows, 1 );
	EXPECT_EQ ( dRows[0], 4096u );
	EXPECT_EQ ( tRows.m_tNext, 9000u );
	std::fill ( std::begin(dBits), std::end(dBits), 0 );
	tFill = p->FillOr ( { 8192, 8192, 12288 }, dBits );
	EXPECT_EQ ( tFill.m_iRows, 1 );
	EXPECT_EQ ( tFill.m_tNext, INVALID_ROWID );
	EXPECT_FALSE ( p->Probe(10000).m_bMatch );
	EXPECT_EQ ( tStats.m_iFetchedDocs, dDocs.size() );
	EXPECT_EQ ( tWord.m_iDocs, dDocs.size() ); // EOF does not overwrite metadata

	tWord.Reset();
	tWord.m_iDocs = int(dDocs.size());
	p->Reset ( true, {} );
	EXPECT_EQ ( p->AdvanceTo(200).m_tNext, 200u );
	EXPECT_EQ ( tWord.m_iHintCalls, 1 ); // new target beyond any retained postings
	EXPECT_FALSE ( p->Probe(200).m_bMatch );
	EXPECT_TRUE ( p->Probe(201).m_bMatch );
}

TEST ( PostingWindow, RowCapacityAtBatchBoundaryRetainsLookahead )
{
	for ( int iDocs : { 128, 129, 256, 300 } )
	{
		SCOPED_TRACE(iDocs);
		Docs_t dDocs;
		for ( RowID_t i=0; i<RowID_t(iDocs); ++i )
			dDocs.push_back ( { i, 1, 1, 0 } );
		WindowTestQword_c tWord(dDocs);
		tWord.m_iDocs = iDocs;
		auto p = CreateBufferedPostingWindow ( tWord, 1, {} );

		RowID_t dRows[128];
		int iConsumed = 0;
		while ( iConsumed<iDocs )
		{
			const auto tRows = p->ReadRows ( { 0, 0, 4096 }, dRows, 128 );
			const int iExpected = std::min ( 128, iDocs-iConsumed );
			ASSERT_EQ ( tRows.m_iRows, iExpected );
			EXPECT_FALSE ( tRows.m_bStopped );
			for ( int i=0; i<iExpected; ++i )
				EXPECT_EQ ( dRows[i], RowID_t(iConsumed+i) );
			iConsumed += iExpected;
			EXPECT_EQ ( tRows.m_tNext, iConsumed<iDocs ? RowID_t(iConsumed) : INVALID_ROWID );
		}
		EXPECT_EQ ( p->ReadRows ( { 0, 0, 4096 }, dRows, 128 ).m_iRows, 0 );
	}
}

TEST ( PostingWindow, BoundsEmptyAndInterruption )
{
	Docs_t dDocs;
	for ( RowID_t i=0; i<300; ++i ) dDocs.push_back ( { i, 1, 1, 0 } );
	dDocs.push_back ( { INVALID_ROWID-1, 1, 1, 0 } );
	WindowTestQword_c tWord(dDocs);
	tWord.m_iDocs = int(dDocs.size());
	int iChecks = 0;
	PostingWindowContext_t tContext { &iChecks, [] ( void * p ) { return ++*static_cast<int *>(p)<3; }, nullptr };
	auto p = CreateBufferedPostingWindow ( tWord, 1, tContext );
	DWORD dBits[PostingWindow_t::WORDS] = {};
	auto tFill = p->FillOr ( { 0, 0, 4096 }, dBits );
	EXPECT_TRUE ( tFill.m_bStopped ); // stop before the second decode batch
	EXPECT_EQ ( tFill.m_iRows, 128 );
	EXPECT_EQ ( tWord.m_iBatchCalls, 1 );
	EXPECT_TRUE ( p->AdvanceTo(4096).m_bStopped ); // sticky until reset
	EXPECT_TRUE ( p->Probe(4096).m_tState.m_bStopped );
	RowID_t tRow;
	EXPECT_TRUE ( p->ReadRows ( { 4096, 4096, 8192 }, &tRow, 1 ).m_bStopped );
	EXPECT_EQ ( tWord.m_iBatchCalls, 1 );

	tWord.Reset();
	tWord.m_iDocs = int(dDocs.size());
	p->Reset ( true, {} );
	std::fill ( std::begin(dBits), std::end(dBits), 0 );
	tFill = p->FillOr ( { 0, 127, 129 }, dBits );
	EXPECT_EQ ( tFill.m_iRows, 2 );
	EXPECT_EQ ( tFill.m_tNext, 129u );
	EXPECT_EQ ( dBits[3], 0x80000000u );
	EXPECT_EQ ( dBits[4], 1u );
	EXPECT_EQ ( p->ReadRows ( { 0, 129, 129 }, &tRow, 1 ).m_iRows, 0 );
	EXPECT_EQ ( p->AdvanceTo(INVALID_ROWID-1).m_tNext, INVALID_ROWID-1 );
	auto tRows = p->ReadRows ( { INVALID_ROWID-4095, INVALID_ROWID-1, uint64_t(INVALID_ROWID)+1 }, &tRow, 1 );
	EXPECT_EQ ( tRows.m_iRows, 1 );
	EXPECT_EQ ( tRow, INVALID_ROWID-1 );
	EXPECT_EQ ( tRows.m_tNext, INVALID_ROWID );
	EXPECT_FALSE ( tRows.m_bStopped );

	tWord.Reset();
	p->Reset ( false, {} );
	const int iCalls = tWord.m_iBatchCalls;
	EXPECT_EQ ( p->AdvanceTo(0).m_tNext, INVALID_ROWID );
	EXPECT_EQ ( p->FillOr ( { 0, 0, 4096 }, dBits ).m_iRows, 0 );
	EXPECT_FALSE ( p->Probe(0).m_bMatch );
	EXPECT_EQ ( tWord.m_iBatchCalls, iCalls );
}

namespace
{
template<bool INLINE_HITS, typename READER>
void CheckNativePostingWindow()
{
	CSphString sFile;
	sFile.SetSprintf ( "test_native_postings_%d_%d.spd", int(INLINE_HITS), int(READER::IS_RAW) );
	struct Cleanup_t
	{
		const CSphString & m_sFile;
		~Cleanup_t() { ::unlink ( m_sFile.cstr() ); }
	} tCleanup { sFile };
	CSphString sError;
	std::vector<RowID_t> dDocs;
	for ( RowID_t i=0; i<300; ++i ) dDocs.push_back(i);
	dDocs.insert ( dDocs.end(), { 4096, 9000, INVALID_ROWID-1 } );
	SphOffset_t iSkipOffset = 0, iSkipHitPosition = 0;
	{
		CSphWriter tWriter;
		ASSERT_TRUE ( tWriter.OpenFile ( sFile, sError ) ) << sError.cstr();
		RowID_t tPrevious = INVALID_ROWID;
		SphOffset_t iHitPosition = 0;
		for ( RowID_t tRow : dDocs )
		{
			if ( tRow==128 )
			{
				iSkipOffset = tWriter.GetPos();
				iSkipHitPosition = iHitPosition;
			}
			const DWORD uField = tRow%5 ? 0 : 1;
			const DWORD uHits = tRow%2 ? 1 : 3;
			const SphOffset_t iDelta = tRow%7 ? 17 : (SphOffset_t(1)<<35);
			tWriter.ZipInt ( tRow-tPrevious );
			if constexpr ( INLINE_HITS )
			{
				tWriter.ZipInt(uHits);
				tWriter.ZipInt ( uHits==1 ? 123 : DWORD(1)<<uField );
				if ( uHits==1 )
					tWriter.ZipInt ( uField<<1 );
				else
				{
					tWriter.ZipOffset(iDelta);
					iHitPosition += iDelta;
				}
			} else
			{
				tWriter.ZipOffset(iDelta);
				iHitPosition += iDelta;
				tWriter.ZipInt ( DWORD(1)<<uField );
				tWriter.ZipInt(uHits);
			}
			tPrevious = tRow;
		}
		tWriter.ZipInt(0); // no padding: the final posting exercises checked map-tail decoding
		tWriter.CloseFile();
		ASSERT_FALSE ( tWriter.IsError() ) << sError.cstr();
	}
	DataReaderFactoryPtr_c pFactory ( NewProxyReader ( sFile, sError, DataReaderFactory_c::DOCS, 64, READER::IS_RAW ? FileAccess_e::MMAP : FileAccess_e::FILE ) );
	ASSERT_TRUE(pFactory) << sError.cstr();
	DiskIndexQword_c<INLINE_HITS,false,READER> tWord ( false, false, 0 );
	tWord.SetDocReader(pFactory);
	tWord.ResetDecoderState();
	tWord.SeekDocReader ( 0, 0 );
	tWord.m_iDocs = int(dDocs.size());
	CSphQueryStats tStats;
	auto p = tWord.CreatePostingWindow ( 1, { nullptr, nullptr, &tStats } );
	EXPECT_EQ ( p->AdvanceTo(0).m_tNext, 0u );
	EXPECT_EQ ( tStats.m_iFetchedDocs, 1 ); // only the retained candidate, no decoded batch
	EXPECT_FALSE ( p->Probe(0).m_bMatch );
	EXPECT_TRUE ( p->Probe(1).m_bMatch );
	EXPECT_TRUE ( p->Probe(1).m_bMatch );
	RowID_t dRows[128];
	auto tRows = p->ReadRows ( { 0, 1, 4096 }, dRows, 17 );
	ASSERT_EQ ( tRows.m_iRows, 17 );
	EXPECT_EQ ( dRows[0], 1u );
	EXPECT_EQ ( dRows[16], 21u );
	EXPECT_EQ ( tRows.m_tNext, 22u );
	DWORD dBits[PostingWindow_t::WORDS] = {};
	dBits[0] = 1;
	auto tFill = p->FillOr ( { 0, 22, 4096 }, dBits );
	EXPECT_EQ ( tFill.m_tNext, 4096u );
	int iExpected = 0;
	for ( int i=0; i<PostingWindow_t::ROWS; ++i )
	{
		const bool bExpected = i>=22 && i<300 && i%5;
		iExpected += bExpected;
		EXPECT_EQ ( (dBits[i>>5] & (DWORD(1)<<(i&31)))!=0, i==0 || bExpected ) << i;
	}
	EXPECT_EQ ( tFill.m_iRows, iExpected );
	EXPECT_TRUE ( p->Probe(4096).m_bMatch );
	EXPECT_EQ ( p->ReadRows ( { 4096, 4096, 4096 }, dRows, 1 ).m_iRows, 0 );
	tRows = p->ReadRows ( { 4096, 4096, 8192 }, dRows, 1 );
	EXPECT_EQ ( tRows.m_iRows, 1 );
	EXPECT_EQ ( dRows[0], 4096u );
	EXPECT_EQ ( tRows.m_tNext, 9000u );
	EXPECT_FALSE ( p->Probe(9000).m_bMatch );
	tRows = p->ReadRows ( { INVALID_ROWID-4095, INVALID_ROWID-1, uint64_t(INVALID_ROWID)+1 }, dRows, 1 );
	EXPECT_EQ ( tRows.m_iRows, 1 );
	EXPECT_EQ ( dRows[0], INVALID_ROWID-1 );
	EXPECT_EQ ( tRows.m_tNext, INVALID_ROWID );
	EXPECT_FALSE ( tRows.m_bStopped );
	EXPECT_EQ ( tStats.m_iFetchedDocs, dDocs.size() );
	EXPECT_EQ ( p->AdvanceTo(0).m_tNext, INVALID_ROWID ); // no restart after EOF

	// Capacity exactly at the periodic-check boundary must still obtain lookahead.
	tWord.Reset();
	tWord.SetDocReader(pFactory);
	tWord.SeekDocReader ( 0, 0 );
	tWord.m_iDocs = int(dDocs.size());
	p = tWord.CreatePostingWindow ( 3, {} );
	tRows = p->ReadRows ( { 0, 0, 4096 }, dRows, 128 );
	EXPECT_EQ ( tRows.m_iRows, 128 );
	EXPECT_EQ ( tRows.m_tNext, 128u );
	for ( int i=0; i<128; ++i ) EXPECT_EQ ( dRows[i], RowID_t(i) );
	EXPECT_TRUE ( p->Probe(128).m_bMatch );

	// Cancellation limits decoded work even when every posting fails its field mask.
	for ( DWORD uFields : { 3u, 4u } )
	{
		tWord.Reset();
		tWord.SetDocReader(pFactory);
		tWord.SeekDocReader ( 0, 0 );
		tWord.m_iDocs = int(dDocs.size());
		int iChecks = 0;
		CSphQueryStats tPartialStats;
		p = tWord.CreatePostingWindow ( uFields, { &iChecks, [] ( void * pCheck ) { return ++*static_cast<int *>(pCheck)<3; }, &tPartialStats } );
		tFill = p->FillOr ( { 0, 0, 4096 }, dBits );
		EXPECT_TRUE ( tFill.m_bStopped );
		EXPECT_EQ ( tFill.m_iRows, uFields==3 ? 128 : 0 );
		EXPECT_EQ ( tPartialStats.m_iFetchedDocs, 128 );
		EXPECT_TRUE ( p->AdvanceTo(4096).m_bStopped );
		EXPECT_EQ ( tPartialStats.m_iFetchedDocs, 128 );
	}

	// A real skip restores row and hit-offset bases before direct decoding resumes.
	tWord.Reset();
	tWord.SetDocReader(pFactory);
	tWord.SeekDocReader ( 0, 0 );
	tWord.m_pSkipData = new SkipData_t;
	tWord.m_pSkipData->m_dSkiplist.Add ( { 0, 0, 0 } );
	tWord.m_pSkipData->m_dSkiplist.Add ( { 128, iSkipOffset, iSkipHitPosition } );
	p->Reset ( true, { nullptr, nullptr, &tStats } );
	EXPECT_EQ ( p->AdvanceTo(200).m_tNext, 200u );
	EXPECT_EQ ( tStats.m_iSkips, 1 );
	DiskIndexQword_c<INLINE_HITS,false,READER> tReference ( false, false, 0 );
	tReference.SetDocReader(pFactory);
	tReference.ResetDecoderState();
	tReference.SeekDocReader ( 0, 0 );
	for ( int i=0; i<=200; ++i ) tReference.GetNextDoc();
	EXPECT_EQ ( tWord.m_iHitlistPos, tReference.m_iHitlistPos );
	EXPECT_EQ ( tWord.GetDocReaderPos(), tReference.GetDocReaderPos() );
	p->Reset ( false, {} );
	EXPECT_EQ ( p->ReadRows ( { 0, 0, 4096 }, dRows, 128 ).m_iRows, 0 );
}
}

TEST ( PostingWindow, NativeInlineMapped ) { CheckNativePostingWindow<true,RawReader_t>(); }
TEST ( PostingWindow, NativeInlineFile ) { CheckNativePostingWindow<true,BlockReader_t>(); }
TEST ( PostingWindow, NativePlainMapped ) { CheckNativePostingWindow<false,RawReader_t>(); }
TEST ( PostingWindow, NativePlainFile ) { CheckNativePostingWindow<false,BlockReader_t>(); }

namespace
{
template<bool INLINE_HITS, typename READER>
void CheckMembershipDecoder()
{
	for ( bool bHasHitlist : { false, true } )
	{
		CSphString sFile;
		sFile.SetSprintf ( "test_membership_%d_%d_%d.spd", int(INLINE_HITS), int(READER::IS_RAW), int(bHasHitlist) );
		struct Cleanup_t
		{
			const CSphString & m_sFile;
			~Cleanup_t() { ::unlink ( m_sFile.cstr() ); }
		} tCleanup { sFile };
		CSphString sError;
		const DWORD dFields[] = { 0, 1, 31, 32, 255, 256, 511 };
		const int iDocs = 280;
		{
			CSphWriter tWriter;
			ASSERT_TRUE ( tWriter.OpenFile ( sFile, sError ) ) << sError.cstr();
			for ( int i=0; i<iDocs; ++i )
			{
				const DWORD uField = dFields[i%7];
				const DWORD uMask = uField<32 ? DWORD(1)<<uField : 0;
				const DWORD uHits = i%3 ? 1 : 3;
				tWriter.ZipInt(1);
				if constexpr ( INLINE_HITS )
				{
					tWriter.ZipInt(uHits);
					if ( uHits==1 && bHasHitlist )
					{
						tWriter.ZipInt(123);
						tWriter.ZipInt ( (uField<<1)|1 );
					} else
					{
						tWriter.ZipInt(uMask);
						tWriter.ZipOffset ( SphOffset_t(1)<<35 );
					}
				} else
				{
					tWriter.ZipOffset ( SphOffset_t(1)<<35 );
					tWriter.ZipInt(uMask);
					tWriter.ZipInt(uHits);
				}
			}
			tWriter.ZipInt(0); // exercise the checked map tail too
			tWriter.CloseFile();
			ASSERT_FALSE ( tWriter.IsError() );
		}
		DataReaderFactoryPtr_c pFactory ( NewProxyReader ( sFile, sError, DataReaderFactory_c::DOCS, 64, READER::IS_RAW ? FileAccess_e::MMAP : FileAccess_e::FILE ) );
		ASSERT_TRUE(pFactory) << sError.cstr();
		for ( DWORD uFields : { 1u, 2u, 0x80000000u, 0xffffffffu } )
		{
			DiskIndexQword_c<INLINE_HITS,false,READER> tWord ( false, false, 0 ), tFull ( false, false, 0 );
			for ( auto * pWord : { &tWord, &tFull } )
			{
				pWord->SetDocReader(pFactory);
				pWord->ResetDecoderState();
				pWord->SeekDocReader ( 0, 0 );
				pWord->m_bHasHitlist = bHasHitlist;
				pWord->m_iDocs = iDocs;
			}
			auto p = tWord.CreatePostingWindow ( uFields, {} );
			for ( int i=0; i<iDocs; ++i )
			{
				SCOPED_TRACE(i);
				ASSERT_EQ ( tFull.GetNextDoc().m_tRowID, RowID_t(i) );
				ASSERT_EQ ( p->AdvanceTo(i).m_tNext, RowID_t(i) );
				EXPECT_EQ ( tWord.GetDocReaderPos(), tFull.GetDocReaderPos() );
				const DWORD uHits = i%3 ? 1 : 3;
				const bool bInline = INLINE_HITS && bHasHitlist && uHits==1;
				const DWORD uField = bInline ? dFields[i%7] & (SPH_MAX_FIELDS-1) : dFields[i%7];
				const DWORD uMask = uField<32 ? DWORD(1)<<uField : 0;
				EXPECT_EQ ( tFull.m_dQwordFields.GetMask32(), uMask );
				EXPECT_EQ ( tFull.m_uMatchHits, uHits );
				if ( bInline )
					EXPECT_EQ ( tFull.m_iHitlistPos, SphOffset_t(123 | (((dFields[i%7]<<1)|1)<<23) | (U64C(1)<<63)) );
				else
					EXPECT_EQ ( tWord.m_iHitlistPos, tFull.m_iHitlistPos );
				const bool bMatch = (uMask & uFields)!=0;
				EXPECT_EQ ( p->Probe(i).m_bMatch, bMatch );
				if ( bMatch )
					EXPECT_TRUE ( p->Probe(i).m_bMatch ); // retained field mask survives repeated probes
			}
			EXPECT_EQ ( p->AdvanceTo(iDocs).m_tNext, INVALID_ROWID );
			EXPECT_EQ ( tFull.GetNextDoc().m_tRowID, INVALID_ROWID );
			EXPECT_EQ ( tWord.GetDocReaderPos(), tFull.GetDocReaderPos() );
		}
	}
}
}

TEST ( PostingWindow, MembershipInlineMapped ) { CheckMembershipDecoder<true,RawReader_t>(); }
TEST ( PostingWindow, MembershipInlineFile ) { CheckMembershipDecoder<true,BlockReader_t>(); }
TEST ( PostingWindow, MembershipPlainMapped ) { CheckMembershipDecoder<false,RawReader_t>(); }
TEST ( PostingWindow, MembershipPlainFile ) { CheckMembershipDecoder<false,BlockReader_t>(); }

TEST ( WindowCount, DeadMapPartialWindow )
{
	const CSphString sFile ( "test_window_deadmap.spm" );
	CSphString sError;
	ASSERT_TRUE ( WriteDeadRowMap ( sFile, 4099, sError ) ) << sError.cstr();
	{
		DeadRowMap_Disk_c tMap;
		ASSERT_TRUE ( tMap.Prealloc ( 4099, sFile, sError ) ) << sError.cstr();
		tMap.Preread ( "test", "dead rows", false );
		DWORD dBits[128];
		std::fill ( std::begin(dBits), std::end(dBits), ~DWORD(0) );
		tMap.MaskAlive ( 4096, dBits, 128 );
		EXPECT_EQ ( dBits[0], 7u );
		for ( int i=1; i<128; ++i ) EXPECT_EQ ( dBits[i], 0u );
		for ( RowID_t i=0; i<32; ++i ) tMap.Set(i);
		tMap.Set(4096);
		tMap.Set(4098);
		std::fill ( std::begin(dBits), std::end(dBits), ~DWORD(0) );
		tMap.MaskAlive ( 0, dBits, 128 );
		EXPECT_EQ ( dBits[0], 0u );
		EXPECT_EQ ( dBits[1], ~DWORD(0) );
		std::fill ( std::begin(dBits), std::end(dBits), ~DWORD(0) );
		tMap.MaskAlive ( 4096, dBits, 128 );
		EXPECT_EQ ( dBits[0], 2u );
		// SIMD tails, unaligned map offsets, and bounded reads at the map's end.
		for ( RowID_t tBase : { 0u, 32u, 4064u, 4096u, 4128u } )
			for ( int iWords : { 0, 1, 3, 7, 15, 17, 128 } )
			{
				DWORD dActual[129], dExpected[129];
				std::fill ( std::begin(dActual), std::end(dActual), 0xa5a5a5a5 );
				std::copy ( std::begin(dActual), std::end(dActual), dExpected );
				for ( int i=0; i<iWords*32; ++i )
					if ( tBase+i>=4099 || tMap.IsSet(tBase+i) )
						dExpected[i>>5] &= ~(DWORD(1)<<(i&31));
				tMap.MaskAlive ( tBase, dActual, iWords );
				EXPECT_TRUE ( std::equal ( std::begin(dActual), std::end(dActual), dExpected ) );
			}
		for ( RowID_t i=32; i<4096; ++i ) tMap.Set(i);
		std::fill ( std::begin(dBits), std::end(dBits), ~DWORD(0) );
		tMap.MaskAlive ( 0, dBits, 128 );
		for ( auto uBits : dBits ) EXPECT_EQ ( uBits, 0u );
		tMap.MaskAlive ( INVALID_ROWID-31, dBits, 128 );
		for ( auto uBits : dBits ) EXPECT_EQ ( uBits, 0u );
	}
	::unlink ( sFile.cstr() );
}
TEST ( WindowCount, RankerRestoresWindowSourceAfterSI )
{
	Threads::CallCoroutine ( [] {
		WindowTestSetup_c tSetup;
		CSphString sError;
		CSphDictSettings tDictSettings;
		tDictSettings.SetDictFormat(DictFormat_e::CRC);
		auto pTok = Tokenizer::Detail::CreateUTF8Tokenizer();
		auto pDict = sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "window-ranker-test", false, 32, nullptr, sError );
		ASSERT_TRUE ( pDict ) << sError.cstr();
		tSetup.SetDict(pDict);
		auto pIndex = sphCreateIndexPhrase ( "window-ranker-test", "unused-window-index" );
		pIndex->SetDictionary(pDict);
		tSetup.m_pIndex = pIndex.get();
		for ( RowID_t i=0; i<300; ++i )
		{
			tSetup.m_dAlpha.push_back ( { i, 1, 1, 0 } );
			if ( i<100 )
				tSetup.m_dBeta.push_back ( { i, 1, 1, 0 } );
		}
		for ( RowID_t i=4096; i<4396; ++i )
			tSetup.m_dBeta.push_back ( { i, 1, 1, 0 } );

		XQLimitSpec_t tSpec;
		XQQuery_t tXQ;
		tXQ.m_pRoot = new XQNode_t(tSpec);
		tXQ.m_pRoot->SetOp(SPH_QUERY_OR);
		tXQ.m_pRoot->AddDirtyWord ( { "alpha", 1 } );
		tXQ.m_pRoot->AddDirtyWord ( { "beta", 2 } );
		CSphQuery tQuery;
		QueryExecutionSettings_t tSettings;
		tSettings.m_eRanker = SPH_RANK_NONE;
		CSphQueryContext tContext ( tQuery, tSettings );
		tContext.m_bSkipQCache = true;
		tContext.m_iTotalDocs = tContext.m_iIndexTotalDocs = 4500;
		tSetup.m_pCtx = &tContext;
		CSphQueryResultMeta tMeta;
		auto pRanker = sphCreateRanker ( tXQ, tQuery, tSettings, tMeta, tSetup, tContext, pIndex->GetMatchSchema(), true );
		ASSERT_TRUE ( pRanker ) << tMeta.m_sError.cstr();
		ASSERT_TRUE ( pRanker->UsedCountWindows() );
		EXPECT_EQ ( pRanker->CountDocs(nullptr), 600 );

		// Each SI wrapper owns the adapter. Reset must recover a window-only root,
		// ready for a different chunk to use either direct counting or another SI.
		for ( int iPass=0; iPass<3; ++iPass )
		{
			pRanker->Reset(tSetup);
			WindowTestIterator_c tIterator;
			int iExpected = 0;
			for ( RowID_t i=0; i<4500; i+=7 )
			{
				tIterator.m_dRows.push_back(i);
				iExpected += i<300 || (i>=4096 && i<4396);
			}
			RowidIterator_i * pIterator = &tIterator;
			ASSERT_TRUE ( pRanker->ExtraData ( EXTRA_SET_ITERATOR, (void**)&pIterator ) );
			EXPECT_FALSE ( pRanker->UsedCountWindows() );
			EXPECT_EQ ( pRanker->CountDocs(nullptr), iExpected );
			pRanker->Reset(tSetup);
			EXPECT_TRUE ( pRanker->UsedCountWindows() );
			EXPECT_EQ ( pRanker->CountDocs(nullptr), 600 );
			EXPECT_EQ ( tSetup.m_iLiveQwords, 2 );
		}

		pRanker->Reset(tSetup);
		RowIdBoundaries_t tBounds { 100, 4200 };
		ASSERT_TRUE ( pRanker->ExtraData ( EXTRA_SET_BOUNDARIES, (void**)&tBounds ) );
		EXPECT_EQ ( pRanker->CountDocs(nullptr), 305 );
		pRanker->Reset(tSetup);
		EXPECT_EQ ( pRanker->CountDocs(nullptr), 305 );

		// Destruction with an attached SI must also destroy the adapter and source.
		pRanker->Reset(tSetup);
		WindowTestIterator_c tIterator;
		tIterator.m_dRows = { 100, 4096, 4200 };
		RowidIterator_i * pIterator = &tIterator;
		ASSERT_TRUE ( pRanker->ExtraData ( EXTRA_SET_ITERATOR, (void**)&pIterator ) );
		tBounds = { 4096, 4199 };
		ASSERT_TRUE ( pRanker->ExtraData ( EXTRA_SET_BOUNDARIES, (void**)&tBounds ) );
		EXPECT_EQ ( pRanker->CountDocs(nullptr), 1 );
		pRanker->Reset(tSetup);
		EXPECT_TRUE ( pRanker->UsedCountWindows() );
		EXPECT_EQ ( pRanker->CountDocs(nullptr), 104 );

		pRanker->Reset(tSetup);
		WindowTestIterator_c tFinalIterator;
		tFinalIterator.m_dRows = { 4096 };
		pIterator = &tFinalIterator;
		ASSERT_TRUE ( pRanker->ExtraData ( EXTRA_SET_ITERATOR, (void**)&pIterator ) );
		EXPECT_EQ ( pRanker->CountDocs(nullptr), 1 );
		pRanker.reset();
		EXPECT_EQ ( tSetup.m_iLiveQwords, 0 );
	} );
}
TEST ( WindowCount, GenericAdapterRetainedRowsAndOwnership )
{
	WindowTestSetup_c tSetup;
	bool bContinue = true;
	int iDestroyed = 0;
	auto pSource = std::make_unique<AdapterTestWindow_c> ( bContinue, iDestroyed );
	auto * pIdentity = pSource.get();
	EXPECT_EQ ( pSource->GetDocsNode(), nullptr );
	auto pAdapter = CreateWindowDocsAdapter ( std::move(pSource) );
	EXPECT_EQ ( pAdapter->GetDocsNode(), pAdapter.get() );
	EXPECT_EQ ( pAdapter->GetCountWindows(), nullptr );
	pAdapter->SetAtomPos(7);
	EXPECT_EQ ( pIdentity->GetAtomPos(), 7 );
	EXPECT_EQ ( pAdapter->Estimate(1000).m_iDocs, 512 );

	ASSERT_TRUE ( pAdapter->GetDocsChunk() );
	// Interruption must be checked even though most of the first window remains.
	bContinue = false;
	EXPECT_EQ ( pAdapter->GetDocsChunk(), nullptr );
	bContinue = true;
	pAdapter->Reset(tSetup);
	ASSERT_TRUE ( pAdapter->GetDocsChunk() );
	pAdapter->SetRowidBoundaries ( { 100, 4200 } );
	std::vector<RowID_t> dExpected;
	for ( RowID_t i=100; i<256; ++i )
		dExpected.push_back(i);
	for ( RowID_t i=4096; i<=4200; ++i )
		dExpected.push_back(i);
	EXPECT_EQ ( ReadChunks(*pAdapter), dExpected );
	pAdapter->Reset(tSetup);
	EXPECT_EQ ( ReadChunks(*pAdapter), dExpected );

	pAdapter->Reset(tSetup);
	ASSERT_TRUE ( pAdapter->GetDocsChunk() );
	pAdapter->HintRowID(4190);
	dExpected.clear();
	for ( RowID_t i=4190; i<=4200; ++i )
		dExpected.push_back(i);
	EXPECT_EQ ( ReadChunks(*pAdapter), dExpected );

	pAdapter->Reset(tSetup);
	auto pRestored = pAdapter->ReleaseSource();
	EXPECT_EQ ( pRestored.get(), pIdentity );
	EXPECT_NE ( pRestored->GetCountWindows(), nullptr );
	EXPECT_EQ ( pRestored->GetDocsNode(), nullptr );
	pAdapter.reset();
	EXPECT_EQ ( iDestroyed, 0 );
	pRestored.reset();
	EXPECT_EQ ( iDestroyed, 1 );

	pAdapter = CreateWindowDocsAdapter ( std::make_unique<AdapterTestWindow_c> ( bContinue, iDestroyed ) );
	pAdapter.reset();
	EXPECT_EQ ( iDestroyed, 2 );
}

TEST ( WindowCount, FilteredCountingCancellationPreservesPartialCount )
{
	class InterruptingFilter_c final : public ISphFilter
	{
	public:
		explicit InterruptingFilter_c ( CSphQueryStats & tStats )
			: m_tStats(tStats)
		{}

		bool Eval ( const CSphMatch & tMatch ) const override
		{
			if ( ++m_iEvaluated==m_iStopAt )
			{
				session::SetKilled(true);
				Threads::Coro::SetThrottlingPeriodMS(1);
				Threads::Coro::RestartRuntime();
				// Model a slow filter without yielding: the next producer check
				// must observe cancellation once the coroutine time slice expires.
				while ( !Threads::Coro::RuntimeExceeded() )
					sphSleepMsec(1);

				m_iFetchedAtStop = m_tStats.m_iFetchedDocs;
			}

			return !(tMatch.m_tRowID&1);
		}

		int m_iStopAt = 4096+5;
		mutable int m_iEvaluated = 0;
		mutable int64_t m_iFetchedAtStop = 0;

	private:
		CSphQueryStats & m_tStats;
	};

	Threads::CallCoroutine ( [] {
		// Avoid unrelated rescheduling outside the deliberately slow filter.
		Threads::Coro::SetThrottlingPeriodMS(60000);
		Threads::Coro::RestartRuntime();
		for ( bool bRows : { false, true } )
		{
			SCOPED_TRACE ( bRows ? "single-contributor rows" : "bitmap" );
			ScopedInfo_T<ClientTaskInfo_t> tSession ( new ClientTaskInfo_t );
			WindowTestSetup_c tSetup;
			CSphQueryStats tStats;
			tSetup.m_pStats = &tStats;
			CSphString sWarning;
			tSetup.m_pWarning = &sWarning;
			CSphDictSettings tDictSettings;
			tDictSettings.SetDictFormat(DictFormat_e::CRC);
			auto pTok = Tokenizer::Detail::CreateUTF8Tokenizer();
			auto pDict = sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "window-cancel-test", false, 32, nullptr, sWarning );
			ASSERT_TRUE ( pDict ) << sWarning.cstr();
			tSetup.SetDict(pDict);
			auto pIndex = sphCreateIndexPhrase ( "window-cancel-test", "unused-window-index" );
			pIndex->SetDictionary(pDict);
			tSetup.m_pIndex = pIndex.get();
			for ( RowID_t i=0; i<8192; ++i )
			{
				tSetup.m_dAlpha.push_back ( { i, 1, 1, 0 } );
				if ( !bRows )
					tSetup.m_dBeta.push_back ( { i, 1, 1, 0 } );
			}

			XQQuery_t tXQ;
			tXQ.m_pRoot = new XQNode_t ( XQLimitSpec_t() );
			tXQ.m_pRoot->SetOp(SPH_QUERY_OR);
			tXQ.m_pRoot->AddDirtyWord ( { "alpha", 1 } );
			tXQ.m_pRoot->AddDirtyWord ( { "beta", 2 } );
			CSphQuery tQuery;
			QueryExecutionSettings_t tSettings;
			tSettings.m_eRanker = SPH_RANK_NONE;
			CSphQueryContext tContext ( tQuery, tSettings );
			tContext.m_bSkipQCache = true;
			tContext.m_iTotalDocs = tContext.m_iIndexTotalDocs = 8192;
			tContext.m_pFilter = std::make_unique<InterruptingFilter_c>(tStats);
			auto * pFilter = static_cast<InterruptingFilter_c*>(tContext.m_pFilter.get());
			tSetup.m_pCtx = &tContext;
			CSphQueryResultMeta tMeta;
			auto pRanker = sphCreateRanker ( tXQ, tQuery, tSettings, tMeta, tSetup, tContext, pIndex->GetMatchSchema(), true );
			ASSERT_TRUE ( pRanker ) << tMeta.m_sError.cstr();
			ASSERT_TRUE ( pRanker->UsedCountWindows() );

			// Preserve the first window and accepted rows of the interrupted window.
			EXPECT_EQ ( pRanker->CountDocs(nullptr), (4096+32)/2 );
			EXPECT_EQ ( pFilter->m_iEvaluated, 4096+32 );
			EXPECT_EQ ( tStats.m_iFetchedDocs, pFilter->m_iFetchedAtStop );
			EXPECT_STREQ ( sWarning.cstr(), "query was killed" );
			EXPECT_EQ ( pRanker->CountDocs(nullptr), 0 );
			EXPECT_EQ ( pFilter->m_iEvaluated, 4096+32 );

			session::SetKilled(false);
			Threads::Coro::SetThrottlingPeriodMS(60000);
			Threads::Coro::RestartRuntime();
			pFilter->m_iStopAt = 0;
			pFilter->m_iEvaluated = 0;
			pRanker->Reset(tSetup);
			EXPECT_EQ ( pRanker->CountDocs(nullptr), 4096 );
			EXPECT_EQ ( pFilter->m_iEvaluated, 8192 );

			tContext.m_pFilter.reset();
			pRanker->Reset(tSetup);
			EXPECT_EQ ( pRanker->CountDocs(nullptr), 8192 );
		}
	} );
}
