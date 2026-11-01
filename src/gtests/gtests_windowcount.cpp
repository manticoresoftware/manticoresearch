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
#include "killlist.h"
#include "secondaryindex.h"
#include "coroutine.h"
#include "mini_timer.h"
#include <algorithm>
#include <set>
#include <vector>

namespace
{
using Docs_t = std::vector<QwordDoc_t>;

class WindowTestQword_c final : public ISphQword
{
public:
	explicit WindowTestQword_c ( const Docs_t & dDocs ) : m_dDocs ( dDocs ) {}
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
};

class WindowTestSetup_c final : public ISphQwordSetup
{
public:
	Docs_t m_dAlpha, m_dBeta, m_dEmpty;
	ISphQword * QwordSpawn ( const XQKeyword_t & tWord ) const override
	{
		return new WindowTestQword_c ( tWord.m_sWord=="alpha" ? m_dAlpha : tWord.m_sWord=="beta" ? m_dBeta : m_dEmpty );
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

std::vector<RowID_t> ReadWindows ( ExtNode_i & tNode )
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
		std::unique_ptr<ExtNode_i> pNode ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
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

		pNode->Reset ( tSetup );
		EXPECT_EQ ( ReadChunks ( *pNode ), dAll );
		for ( RowID_t tHint : { RowID_t(31), RowID_t(127), RowID_t(4095), RowID_t(4096), RowID_t(8193), INVALID_ROWID-1, INVALID_ROWID } )
		{
			pNode->Reset ( tSetup );
			ASSERT_TRUE ( pNode->GetDocsChunk() ); // retained window and lookahead batches
			pNode->HintRowID ( tHint );
			const RowID_t tFloor = Max ( tHint, dAll[MAX_BLOCK_DOCS-2]+1 );
			EXPECT_EQ ( ReadChunks ( *pNode ), std::vector<RowID_t> ( dExpected.lower_bound(tFloor), dExpected.end() ) );
		}

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
		std::unique_ptr<ExtNode_i> pWrappedNode ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
		auto pWrapper = CreatePseudoFTNode ( pWrappedNode.release(), &tIterator );
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
		std::unique_ptr<ExtNode_i> pEmpty ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
		ASSERT_TRUE ( pEmpty );
		EXPECT_TRUE ( ReadWindows ( *pEmpty ).empty() );
		tRoot.AddDirtyWord ( { "absent", 33 } );
		EXPECT_EQ ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ), nullptr );

		tRoot.ResetWords();
		tRoot.AddDirtyWord ( { "alpha", 1 } );
		tRoot.AddDirtyWord ( { "beta", 2 } );
		tSetup.m_pWarning = &sError;
		tSetup.m_iMaxTimer = sph::MicroTimer()-10000;
		std::unique_ptr<ExtNode_i> pExpired ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
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
		std::unique_ptr<ExtNode_i> pNode ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
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

		pNode->Reset(tSetup);
		EXPECT_EQ ( ReadChunks(*pNode), dAll );
		for ( RowID_t tHint : { RowID_t(31), RowID_t(127), RowID_t(128), RowID_t(511), RowID_t(4096), RowID_t(8193), INVALID_ROWID-1, INVALID_ROWID } )
		{
			pNode->Reset(tSetup);
			ASSERT_TRUE ( pNode->GetDocsChunk() );
			pNode->HintRowID(tHint);
			const RowID_t tFloor = Max ( tHint, dAll[MAX_BLOCK_DOCS-2]+1 );
			EXPECT_EQ ( ReadChunks(*pNode), std::vector<RowID_t> ( dExpected.lower_bound(tFloor), dExpected.end() ) );
		}
		pNode->Reset(tSetup);
		pNode->SetRowidBoundaries ( { 127, 8300 } );
		const std::vector<RowID_t> dBounded ( dExpected.lower_bound(127), dExpected.upper_bound(8300) );
		EXPECT_EQ ( ReadWindows(*pNode), dBounded );
		pNode->Reset(tSetup);
		EXPECT_EQ ( ReadChunks(*pNode), dBounded );

		WindowTestIterator_c tIterator;
		std::vector<RowID_t> dIntersection;
		for ( RowID_t i=0; i<13000; i+=13 )
		{
			tIterator.m_dRows.push_back(i);
			if ( dExpected.count(i) ) dIntersection.push_back(i);
		}
		std::unique_ptr<ExtNode_i> pUnwrapped ( CreateWindowOrCountNode ( &tRoot, tSetup, nullptr ) );
		auto pWrapper = CreatePseudoFTNode ( pUnwrapped.release(), &tIterator );
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
