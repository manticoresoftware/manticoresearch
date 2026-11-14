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
#include "termproducer.h"

namespace
{

class ProducerBlock_c final : public PostingBlock_i
{
public:
	ProducerBlock_c ( const std::vector<QwordDoc_t> & dDocs, DWORD uFields, int & iReads ) : m_dDocs(dDocs), m_uFields(uFields), m_iReads(iReads) {}


	bool Prepare ( RowID_t tMin, PostingLeaf_t & tLeaf ) final
	{
		if ( tMin>=m_dDocs.size() )
			return false;

		tLeaf = { tMin, m_dDocs.size(), true };
		return true;
	}


	int ReadRows ( const PostingWindow_t & tWindow, RowID_t * pRows ) final
	{
		int iCount = 0;
		for ( const auto & tDoc : m_dDocs )
			if ( tDoc.m_tRowID>=tWindow.m_tMin && tDoc.m_tRowID<tWindow.m_uEnd && (tDoc.m_uFields & m_uFields) )
				pRows[iCount++] = tDoc.m_tRowID;

		return iCount;
	}


	bool ReadPayloads ( const RowID_t * pRows, int iCount, QwordDoc_t * pDocs, bool bTF, bool bHits ) final
	{
		++m_iReads;
		for ( int i=0; i<iCount; ++i )
		{
			pDocs[i] = m_dDocs[pRows[i]];
			if ( !bTF )
				pDocs[i].m_uMatchHits = 0;

			if ( !bHits )
				pDocs[i].m_iHitlistPos = 0;
		}

		return true;
	}


	int Intersect ( const PostingWindow_t &, DWORD * ) final { ADD_FAILURE() << "decoded source"; return 0; }


	int FilterRows ( RowID_t *, int ) final { ADD_FAILURE() << "unexpected probe"; return 0; }

private:
	const std::vector<QwordDoc_t> & m_dDocs;
	DWORD	m_uFields;
	int &	m_iReads;
};


class ProducerQword_c final : public ISphQword
{
public:
	ProducerQword_c()
	{
		for ( RowID_t i=0; i<12; ++i )
			m_dDocs.push_back ( { i, i%2 ? 1U : 2U, i+1, 100+i } );

		m_iDocs = int(m_dDocs.size());
	}


	int GetNextDocs ( QwordDoc_t * pDocs, int iMax ) final
	{
		++m_iBatches;
		int iCount = 0;
		while ( iCount<iMax && m_iPos<int(m_dDocs.size()) )
			pDocs[iCount++] = m_dDocs[m_iPos++];

		return iCount;
	}


	const CSphMatch & GetNextDoc() final
	{
		++m_iSingles;
		m_tMatch.m_tRowID = m_iPos<int(m_dDocs.size()) ? m_dDocs[m_iPos].m_tRowID : INVALID_ROWID;
		if ( m_tMatch.m_tRowID!=INVALID_ROWID )
		{
			m_uMatchHits = m_dDocs[m_iPos].m_uMatchHits;
			m_iHitlistPos = m_dDocs[m_iPos].m_iHitlistPos;
			++m_iPos;
		}

		return m_tMatch;
	}


	void CollectHitMask() final
	{
		++m_iMasks;
		m_dQwordFields.UnsetAll();
		m_dQwordFields.Set ( m_tMatch.m_tRowID%2 ? 33 : 0 );
	}


	bool HintRowID ( RowID_t tMin ) final
	{
		while ( m_iPos<int(m_dDocs.size()) && m_dDocs[m_iPos].m_tRowID<tMin )
			++m_iPos;

		return true;
	}


	void SetPayloadRequirements ( bool bTF, bool bHits ) final
	{
		m_bTF = bTF;
		m_bHits = bHits;
	}


	std::unique_ptr<PostingBlock_i> CreatePostingBlock ( DWORD uFields, const PostingWindowContext_t & ) final
	{
		if ( !m_bBlock )
			return nullptr;

		return std::make_unique<ProducerBlock_c> ( m_dDocs, uFields, m_iPayloadReads );
	}


	void SeekHitlist ( SphOffset_t ) final { ADD_FAILURE() << "unexpected hits"; }


	Hitpos_t GetNextHit() final { ADD_FAILURE() << "unexpected hits"; return 0; }

	std::vector<QwordDoc_t> m_dDocs;
	CSphMatch	m_tMatch;
	int			m_iPos = 0;
	int			m_iBatches = 0;
	int			m_iSingles = 0;
	int			m_iMasks = 0;
	int			m_iPayloadReads = 0;
	bool		m_bBlock = false;
	bool		m_bTF = false;
	bool		m_bHits = false;
};


class ProducerFilter_c final : public PostingCandidateFilter_i
{
public:
	RowID_t	AdvanceTo ( RowID_t tMin ) final { return tMin; }


	int		SelectRows ( RowID_t *, int iCount ) final { return iCount; }
};


TermProducerSetup_t MakeTermSetup()
{
	TermProducerSetup_t tSetup;
	tSetup.m_tFields.UnsetAll();
	tSetup.m_tFields.Set(0);
	tSetup.m_iCapacity = 3;
	return tSetup;
}

}


TEST ( TermProducer, BatchedSelectionAndBounds )
{
	ProducerQword_c tWord;
	auto tSetup = MakeTermSetup();
	tSetup.m_tMin = 2;
	tSetup.m_uEnd = 10;
	auto pProducer = CreateTermProducer ( tWord, tSetup );
	ASSERT_TRUE(pProducer);
	EXPECT_EQ ( tWord.m_iBatches, 0 );

	auto tBatch = pProducer->NextCandidates();
	ASSERT_EQ ( tBatch.m_iCount, 3 );
	EXPECT_EQ ( tBatch.GetRowID(0), 3U );
	EXPECT_EQ ( tBatch.GetRowID(2), 7U );
	RowID_t dSelection[] = { 3, 7 };
	const auto * pDocs = pProducer->ReadPayloads ( dSelection, 2 );
	ASSERT_NE ( pDocs, nullptr );
	EXPECT_EQ ( pDocs[1].m_tRowID, 7U );
	EXPECT_EQ ( pDocs[1].m_uMatchHits, 8U );
	EXPECT_EQ ( pDocs[1].m_iHitlistPos, 107 );

	tBatch = pProducer->NextCandidates();
	ASSERT_EQ ( tBatch.m_iCount, 1 );
	EXPECT_EQ ( tBatch.GetRowID(0), 9U );
	EXPECT_EQ ( pProducer->ReadPayloads()[0].m_tRowID, 9U );
	EXPECT_EQ ( pProducer->NextCandidates().m_iCount, 0 );
	EXPECT_EQ ( tWord.m_iSingles, 0 );
}


TEST ( TermProducer, WideFieldsAndBounds )
{
	ProducerQword_c tWord;
	auto tSetup = MakeTermSetup();
	tSetup.m_tFields.UnsetAll();
	tSetup.m_tFields.Set(33);
	tSetup.m_bWideFields = true;
	tSetup.m_tMin = 4;
	tSetup.m_uEnd = 10;
	EXPECT_EQ ( PlanTermProducer(tSetup), TermReadMode_e::WIDE_FIELDS );

	auto pProducer = CreateTermProducer ( tWord, tSetup );
	pProducer->HintRowID(4);
	pProducer->SetPayloadRequirements ( { false, true } );
	EXPECT_TRUE(tWord.m_bTF);
	EXPECT_TRUE(tWord.m_bHits);

	auto tBatch = pProducer->NextCandidates();
	ASSERT_EQ ( tBatch.m_iCount, 3 );
	EXPECT_EQ ( tBatch.GetRowID(0), 5U );
	EXPECT_EQ ( tBatch.GetRowID(2), 9U );
	const auto * pDocs = pProducer->ReadPayloads();
	EXPECT_EQ ( pDocs[0].m_uFields, 0U );
	EXPECT_EQ ( pDocs[2].m_uMatchHits, 10U );
	EXPECT_EQ ( pProducer->NextCandidates().m_iCount, 0 );
	EXPECT_GT ( tWord.m_iMasks, 0 );
	EXPECT_EQ ( tWord.m_iBatches, 0 );
}


TEST ( TermProducer, UnsupportedRestrictionDoesNotConsume )
{
	ProducerQword_c tWord;
	ProducerFilter_c tFilter;
	auto tSetup = MakeTermSetup();
	tSetup.m_pFilter = &tFilter;
	EXPECT_EQ ( PlanTermProducer(tSetup), TermReadMode_e::RESTRICTED_BLOCK );
	EXPECT_FALSE ( CreateTermProducer ( tWord, tSetup ) );
	tSetup.m_bWideFields = true;
	EXPECT_EQ ( PlanTermProducer(tSetup), TermReadMode_e::UNSUPPORTED );
	EXPECT_FALSE ( CreateTermProducer ( tWord, tSetup ) );
	EXPECT_EQ ( tWord.m_iPos, 0 );
	EXPECT_FALSE(tWord.m_bTF);
	EXPECT_FALSE(tWord.m_bHits);

	tSetup.m_pFilter = nullptr;
	tSetup.m_bWideFields = false;
	auto pProducer = CreateTermProducer ( tWord, tSetup );
	EXPECT_EQ ( pProducer->NextCandidates().GetRowID(0), 1U );
}


TEST ( TermProducer, InterruptionAndHint )
{
	ProducerQword_c tWord;
	auto tSetup = MakeTermSetup();
	bool bContinue = false;
	tSetup.m_tContext = { &bContinue, [] ( void * p ) { return *static_cast<bool *>(p); }, nullptr };
	auto pProducer = CreateTermProducer ( tWord, tSetup );
	EXPECT_EQ ( pProducer->NextCandidates().m_iCount, 0 );
	EXPECT_EQ ( tWord.m_iPos, 0 );
	bContinue = true;
	pProducer->HintRowID(8);
	auto tBatch = pProducer->NextCandidates();
	ASSERT_EQ ( tBatch.m_iCount, 2 );
	EXPECT_EQ ( tBatch.GetRowID(0), 9U );
	EXPECT_EQ ( tBatch.GetRowID(1), 11U );
}


TEST ( TermProducer, RestrictedPayloadsAreLazyAndSelectable )
{
	ProducerQword_c tWord;
	tWord.m_bBlock = true;
	ProducerFilter_c tFilter;
	auto tSetup = MakeTermSetup();
	tSetup.m_pFilter = &tFilter;
	tSetup.m_tMin = 4;
	tSetup.m_tPayloads = { true, false };
	auto pProducer = CreateTermProducer ( tWord, tSetup );
	ASSERT_TRUE(pProducer);
	auto tBatch = pProducer->NextCandidates();
	ASSERT_EQ ( tBatch.m_iCount, 3 );
	EXPECT_EQ ( tBatch.GetRowID(0), 5U );
	EXPECT_EQ ( tWord.m_iPayloadReads, 0 );
	RowID_t dRows[] = { 7, 9 };
	const auto * pDocs = pProducer->ReadPayloads ( dRows, 2 );
	ASSERT_NE ( pDocs, nullptr );
	EXPECT_EQ ( tWord.m_iPayloadReads, 1 );
	EXPECT_EQ ( pDocs[0].m_tRowID, 7U );
	EXPECT_EQ ( pDocs[0].m_uMatchHits, 8U );
	EXPECT_EQ ( pDocs[0].m_iHitlistPos, 0 );

	pProducer->SetPayloadRequirements ( { false, true } );
	tBatch = pProducer->NextCandidates();
	ASSERT_EQ ( tBatch.m_iCount, 1 );
	EXPECT_EQ ( tBatch.GetRowID(0), 11U );
	pDocs = pProducer->ReadPayloads();
	EXPECT_EQ ( pDocs[0].m_iHitlistPos, 111 );
	EXPECT_EQ ( pProducer->NextCandidates().m_iCount, 0 );
	EXPECT_EQ ( tWord.m_iBatches, 0 );
}


TEST ( TermProducer, EmptyAndSentinelBounds )
{
	for ( bool bWide : { false, true } )
		for ( RowID_t tMin : { RowID_t(0), RowID_t(7), INVALID_ROWID-1, INVALID_ROWID } )
			for ( RowID_t tEnd : { RowID_t(0), RowID_t(7), INVALID_ROWID-1, INVALID_ROWID } )
			{
				SCOPED_TRACE ( ::testing::Message() << bWide << ": " << tMin << ".." << tEnd );
				ProducerQword_c tWord;
				tWord.m_dDocs = { { 0, 1, 1, 100 }, { 7, 1, 1, 101 }, { INVALID_ROWID-1, 1, 1, 102 } };
				tWord.m_iDocs = int(tWord.m_dDocs.size());
				auto tSetup = MakeTermSetup();
				tSetup.m_tFields.SetAll();
				tSetup.m_bWideFields = bWide;
				tSetup.m_tMin = tMin;
				tSetup.m_uEnd = tEnd;
				auto pProducer = CreateTermProducer ( tWord, tSetup );
				ASSERT_TRUE(pProducer);

				std::vector<RowID_t> dExpected;
				for ( const auto & tDoc : tWord.m_dDocs )
					if ( tDoc.m_tRowID>=tMin && tDoc.m_tRowID<tEnd )
						dExpected.push_back(tDoc.m_tRowID);

				auto tBatch = pProducer->NextCandidates();
				ASSERT_EQ ( tBatch.m_iCount, dExpected.size() );
				for ( int i=0; i<tBatch.m_iCount; ++i )
					EXPECT_EQ ( tBatch.GetRowID(i), dExpected[i] );

				EXPECT_EQ ( pProducer->NextCandidates().m_iCount, 0 );
			}
}
