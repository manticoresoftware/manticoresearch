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
#include "postingsproducers/termproducer.h"
#include "postingsproducers/andproducer.h"

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


	bool ReadPayloads ( const RowID_t * pRows, int iCount, QwordDoc_t * pDocs, bool bTF, bool bHits, const uint16_t * ) final
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


	int FilterRows ( RowID_t * pRows, int iCount ) final
	{
		m_iFilterRows += iCount;
		int iOut = 0;
		for ( int i=0; i<iCount; ++i )
			if ( m_dDocs[pRows[i]].m_uFields & m_uFields )
				pRows[iOut++] = pRows[i];

		return iOut;
	}

	int		m_iFilterRows = 0;

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
			m_dQwordFields.Assign32(m_dDocs[m_iPos].m_uFields);
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

		auto pBlock = std::make_unique<ProducerBlock_c> ( m_dDocs, uFields, m_iPayloadReads );
		m_pLastBlock = pBlock.get();
		return pBlock;
	}


	void SeekHitlist ( SphOffset_t ) final { ADD_FAILURE() << "unexpected hits"; }


	Hitpos_t GetNextHit() final { ADD_FAILURE() << "unexpected hits"; return 0; }

	ProducerBlock_c * m_pLastBlock = nullptr;
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


class ProducerFilter_c final : public PostingCandidateFilter_c
{
public:
	RowID_t	AdvanceTo ( RowID_t tMin ) final { return tMin; }


	int		SelectRows ( RowID_t *, int iCount ) final { return iCount; }
};


class SelectiveProducerFilter_c final : public PostingCandidateFilter_c
{
public:
	RowID_t	AdvanceTo ( RowID_t tMin ) final { return tMin; }


	int		SelectRows ( RowID_t * pRows, int iCount ) final
	{
		m_dSizes.push_back(iCount);
		int iOut = 0;
		for ( int i=0; i<iCount; ++i )
			if ( pRows[i]>=8 )
				pRows[iOut++] = pRows[i];

		return iOut;
	}

	std::vector<int> m_dSizes;
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

	auto tBatch = pProducer->NextCandidates(tSetup.m_iCapacity);
	ASSERT_EQ ( tBatch.m_iCount, 3 );
	EXPECT_EQ ( tBatch.GetRowID(0), 3U );
	EXPECT_EQ ( tBatch.GetRowID(2), 7U );
	RowID_t dSelection[] = { 3, 7 };
	const auto * pDocs = pProducer->ReadPayloads ( 0, dSelection, 2 );
	ASSERT_NE ( pDocs, nullptr );
	EXPECT_EQ ( pDocs[1].m_tRowID, 7U );
	EXPECT_EQ ( pDocs[1].m_uMatchHits, 8U );
	EXPECT_EQ ( pDocs[1].m_iHitlistPos, 107 );

	tBatch = pProducer->NextCandidates(tSetup.m_iCapacity);
	ASSERT_EQ ( tBatch.m_iCount, 1 );
	EXPECT_EQ ( tBatch.GetRowID(0), 9U );
	EXPECT_EQ ( pProducer->ReadPayloads()[0].m_tRowID, 9U );
	EXPECT_EQ ( pProducer->NextCandidates(tSetup.m_iCapacity).m_iCount, 0 );
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
	EXPECT_EQ ( PlanTermProducer(tSetup), TermReadMode_e::QWORD_WIDE_FIELDS );

	auto pProducer = CreateTermProducer ( tWord, tSetup );
	pProducer->HintRowID(4);
	pProducer->SetPayloadRequirements ( { false, true } );
	EXPECT_TRUE(tWord.m_bTF);
	EXPECT_TRUE(tWord.m_bHits);

	auto tBatch = pProducer->NextCandidates(tSetup.m_iCapacity);
	ASSERT_EQ ( tBatch.m_iCount, 3 );
	EXPECT_EQ ( tBatch.GetRowID(0), 5U );
	EXPECT_EQ ( tBatch.GetRowID(2), 9U );
	const auto * pDocs = pProducer->ReadPayloads();
	EXPECT_EQ ( pDocs[0].m_uFields, 0U );
	EXPECT_EQ ( pDocs[2].m_uMatchHits, 10U );
	EXPECT_EQ ( pProducer->NextCandidates(tSetup.m_iCapacity).m_iCount, 0 );
	EXPECT_GT ( tWord.m_iMasks, 0 );
	EXPECT_EQ ( tWord.m_iBatches, 0 );
}


TEST ( TermProducer, UnsupportedRestrictionDoesNotConsume )
{
	ProducerQword_c tWord;
	ProducerFilter_c tFilter;
	auto tSetup = MakeTermSetup();
	tSetup.m_pFilter = &tFilter;
	EXPECT_EQ ( PlanTermProducer(tSetup), TermReadMode_e::BLOCK );
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
	EXPECT_EQ ( pProducer->NextCandidates(tSetup.m_iCapacity).GetRowID(0), 1U );
}


TEST ( TermProducer, InterruptionAndHint )
{
	ProducerQword_c tWord;
	auto tSetup = MakeTermSetup();
	bool bContinue = false;
	tSetup.m_tContext = { &bContinue, [] ( void * p ) { return *static_cast<bool *>(p); }, nullptr };
	auto pProducer = CreateTermProducer ( tWord, tSetup );
	EXPECT_EQ ( pProducer->NextCandidates(tSetup.m_iCapacity).m_iCount, 0 );
	EXPECT_EQ ( tWord.m_iPos, 0 );
	bContinue = true;
	pProducer->HintRowID(8);
	auto tBatch = pProducer->NextCandidates(tSetup.m_iCapacity);
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
	auto tBatch = pProducer->NextCandidates(tSetup.m_iCapacity);
	ASSERT_EQ ( tBatch.m_iCount, 3 );
	EXPECT_EQ ( tBatch.GetRowID(0), 5U );
	EXPECT_EQ ( tWord.m_iPayloadReads, 0 );
	RowID_t dRows[] = { 7, 9 };
	const auto * pDocs = pProducer->ReadPayloads ( 0, dRows, 2 );
	ASSERT_NE ( pDocs, nullptr );
	EXPECT_EQ ( tWord.m_iPayloadReads, 1 );
	EXPECT_EQ ( pDocs[0].m_tRowID, 7U );
	EXPECT_EQ ( pDocs[0].m_uMatchHits, 8U );
	EXPECT_EQ ( pDocs[0].m_iHitlistPos, 0 );

	pProducer->SetPayloadRequirements ( { false, true } );
	tBatch = pProducer->NextCandidates(tSetup.m_iCapacity);
	ASSERT_EQ ( tBatch.m_iCount, 1 );
	EXPECT_EQ ( tBatch.GetRowID(0), 11U );
	pDocs = pProducer->ReadPayloads();
	EXPECT_EQ ( pDocs[0].m_iHitlistPos, 111 );
	EXPECT_EQ ( pProducer->NextCandidates(tSetup.m_iCapacity).m_iCount, 0 );
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

				auto tBatch = pProducer->NextCandidates(tSetup.m_iCapacity);
				ASSERT_EQ ( tBatch.m_iCount, dExpected.size() );
				for ( int i=0; i<tBatch.m_iCount; ++i )
					EXPECT_EQ ( tBatch.GetRowID(i), dExpected[i] );

				EXPECT_EQ ( pProducer->NextCandidates(tSetup.m_iCapacity).m_iCount, 0 );
			}
}


TEST ( TermProducer, EarlyFilterRequiresCheapComplexity )
{
	for ( float fComplexity : { 1.0f, 1.01f, 3.0f, std::numeric_limits<float>::infinity() } )
	{
		SCOPED_TRACE(fComplexity);
		ProducerQword_c tWord;
		SelectiveProducerFilter_c tFilter;
		if ( fComplexity!=std::numeric_limits<float>::infinity() )
			tFilter.SetFilterComplexity(fComplexity);

		ProducerQword_c tOther;
		tWord.m_bBlock = tOther.m_bBlock = true;
		FieldMask_t tAllFields, tFields;
		tAllFields.Assign32(3);
		tFields.Assign32(1);

		AndProducerSetup_t tSetup;
		tSetup.m_dTerms = { { &tWord, tAllFields }, { &tOther, tFields } };
		tSetup.m_pFilter = &tFilter;
		tSetup.m_uEnd = 12;
		tSetup.m_iCapacity = 12;
		auto pProducer = CreateAndProducer(tSetup);
		ASSERT_TRUE(pProducer);

		auto tRows = pProducer->NextCandidates(12);
		ASSERT_EQ ( tRows.m_iCount, 2 );
		EXPECT_EQ ( tRows.m_pRows[0], 9U );
		EXPECT_EQ ( tRows.m_pRows[1], 11U );
		EXPECT_EQ ( tOther.m_pLastBlock->m_iFilterRows, fComplexity==1.0f ? 4 : 12 );
		EXPECT_EQ ( tFilter.m_dSizes, fComplexity==1.0f ? (std::vector<int>{12,2}) : (std::vector<int>{6}) );
		EXPECT_EQ ( pProducer->NextCandidates(12).m_iCount, 0 );
	}
}


TEST ( AndProducer, NativeAndFallbackPayloadsBoundsAndHints )
{
	for ( bool bBlock : { false, true } )
	{
		SCOPED_TRACE(bBlock);
		ProducerQword_c tFirst, tSecond;
		tFirst.m_bBlock = tSecond.m_bBlock = bBlock;
		for ( auto & tDoc : tSecond.m_dDocs )
		{
			tDoc.m_uMatchHits += 20;
			tDoc.m_iHitlistPos += 1000;
			if ( tDoc.m_tRowID==5 )
				tDoc.m_uFields = 2;
		}

		AndProducerSetup_t tSetup;
		FieldMask_t tFields;
		tFields.Assign32(1);
		tSetup.m_dTerms = { { &tFirst, tFields }, { &tSecond, tFields } };
		tSetup.m_iCapacity = 3;
		tSetup.m_tMin = 2;
		tSetup.m_uEnd = 11;
		tSetup.m_tPayloads = { true, true };

		auto pProducer = CreateAndProducer(tSetup);
		ASSERT_TRUE(pProducer);
		EXPECT_EQ ( tFirst.m_iPos, 0 );
		auto tRows = pProducer->NextCandidates(2);
		ASSERT_EQ ( tRows.m_iCount, 2 );
		EXPECT_EQ ( tRows.m_pRows[0], 3U );
		EXPECT_EQ ( tRows.m_pRows[1], 7U );
		EXPECT_EQ ( tFirst.m_iPayloadReads, 0 );

		const auto * pDocs = pProducer->ReadPayloads(1);
		ASSERT_NE ( pDocs, nullptr );
		EXPECT_EQ ( pDocs[0].m_uMatchHits, 24U );
		EXPECT_EQ ( pDocs[1].m_iHitlistPos, 1107 );
		pDocs = pProducer->ReadPayloads(0);
		EXPECT_EQ ( pDocs[0].m_uMatchHits, 4U );
		EXPECT_EQ ( pDocs[1].m_iHitlistPos, 107 );
		pProducer->HintRowID(9);
		pProducer->HintRowID(1);
		tRows = pProducer->NextCandidates(3);
		ASSERT_EQ ( tRows.m_iCount, 1 );
		EXPECT_EQ ( tRows.m_pRows[0], 9U );
		EXPECT_EQ ( pProducer->NextCandidates(3).m_iCount, 0 );
		EXPECT_EQ ( pProducer->NextCandidates(3).m_iCount, 0 );
	}
}


TEST ( AndProducer, WideFieldsAndFailedFilterAdmission )
{
	ProducerQword_c tFirst, tSecond;
	tFirst.m_bBlock = true;
	FieldMask_t tFields;
	tFields.Assign32(1);

	AndProducerSetup_t tSetup;
	tSetup.m_dTerms = { { &tFirst, tFields }, { &tSecond, tFields } };
	tSetup.m_iCapacity = 3;
	ProducerFilter_c tFilter;
	tSetup.m_pFilter = &tFilter;
	EXPECT_FALSE(CreateAndProducer(tSetup));
	EXPECT_EQ ( tFirst.m_iPos, 0 );
	EXPECT_EQ ( tSecond.m_iPos, 0 );
	EXPECT_FALSE(tFirst.m_bTF);
	tSetup.m_dTerms[1].m_tFields.UnsetAll();
	tSetup.m_dTerms[1].m_tFields.Set(33);
	tSetup.m_dTerms[1].m_bWideFields = true;
	EXPECT_EQ ( PlanAndProducer(tSetup), AndReadMode_e::UNSUPPORTED );
	EXPECT_FALSE(CreateAndProducer(tSetup));
	tSetup.m_pFilter = nullptr;
	EXPECT_EQ ( PlanAndProducer(tSetup), AndReadMode_e::QWORD );

	auto pProducer = CreateAndProducer(tSetup);
	ASSERT_TRUE(pProducer);
	pProducer->SetPayloadRequirements ( { false, true } );
	EXPECT_TRUE(tFirst.m_bHits);
	EXPECT_TRUE(tSecond.m_bTF);
	pProducer->HintRowID(6);
	auto tRows = pProducer->NextCandidates(3);
	ASSERT_EQ ( tRows.m_iCount, 3 );
	EXPECT_EQ ( tRows.m_pRows[0], 7U );
	EXPECT_EQ ( tRows.m_pRows[2], 11U );
	EXPECT_EQ ( pProducer->ReadPayloads(1)[0].m_uFields, 0U );
	EXPECT_GT ( tSecond.m_iMasks, 0 );
}


TEST ( AndProducer, RestrictionAndPayloadRequirements )
{
	ProducerQword_c tFirst, tSecond;
	tFirst.m_bBlock = tSecond.m_bBlock = true;
	FieldMask_t tFields;
	tFields.Assign32(1);
	SelectiveProducerFilter_c tFilter;
	AndProducerSetup_t tSetup;
	tSetup.m_dTerms = { { &tFirst, tFields }, { &tSecond, tFields } };
	tSetup.m_iCapacity = 3;
	tSetup.m_pFilter = &tFilter;

	auto pProducer = CreateAndProducer(tSetup);
	ASSERT_TRUE(pProducer);
	auto tRows = pProducer->NextCandidates(3);
	ASSERT_EQ ( tRows.m_iCount, 2 );
	EXPECT_EQ ( tRows.m_pRows[0], 9U );
	EXPECT_EQ ( tRows.m_pRows[1], 11U );
	EXPECT_EQ ( tFirst.m_iPayloadReads, 0 );
	EXPECT_EQ ( pProducer->ReadPayloads(0)[0].m_uMatchHits, 0U );
	EXPECT_EQ ( pProducer->ReadPayloads(1)[0].m_iHitlistPos, 0 );
	EXPECT_EQ ( pProducer->NextCandidates(3).m_iCount, 0 );
}


TEST ( AndProducer, EmptyBoundsAbsentTermsAndInterruption )
{
	for ( bool bBlock : { false, true } )
	{
		ProducerQword_c tFirst, tSecond;
		tFirst.m_bBlock = tSecond.m_bBlock = bBlock;
		FieldMask_t tFields;
		tFields.SetAll();
		AndProducerSetup_t tSetup;
		tSetup.m_dTerms = { { &tFirst, tFields }, { &tSecond, tFields } };
		tSetup.m_iCapacity = 3;
		tSetup.m_tMin = tSetup.m_uEnd = INVALID_ROWID;

		auto pProducer = CreateAndProducer(tSetup);
		ASSERT_TRUE(pProducer);
		EXPECT_EQ ( pProducer->NextCandidates(3).m_iCount, 0 );
		EXPECT_EQ ( tFirst.m_iPos, 0 );
		tSetup.m_tMin = 0;
		tSetup.m_tContext = { nullptr, [] ( void * ) { return false; }, nullptr };
		pProducer = CreateAndProducer(tSetup);
		EXPECT_EQ ( pProducer->NextCandidates(3).m_iCount, 0 );
		EXPECT_EQ ( tFirst.m_iPos, 0 );
		tSecond.m_iDocs = 0;
		EXPECT_FALSE(CreateAndProducer(tSetup));
	}
}


TEST ( AndProducer, StaggeredListsAndSentinelBounds )
{
	for ( RowID_t tMin : { RowID_t(0), RowID_t(3), RowID_t(8), INVALID_ROWID-1, INVALID_ROWID } )
		for ( RowID_t tEnd : { RowID_t(0), RowID_t(7), RowID_t(8), INVALID_ROWID } )
		{
			SCOPED_TRACE ( ::testing::Message() << tMin << ".." << tEnd );
			ProducerQword_c tFirst, tSecond, tThird;
			tFirst.m_dDocs = { { 0,1,2,100 }, { 3,1,4,103 }, { 7,1,8,107 }, { INVALID_ROWID-1,1,9,109 } };
			tSecond.m_dDocs = { { 2,1,1,102 }, { 3,1,6,203 }, { 7,1,16,207 }, { INVALID_ROWID-1,1,19,209 } };
			tThird.m_dDocs = { { 3,1,7,303 }, { 6,1,9,306 }, { 7,1,18,307 }, { INVALID_ROWID-1,1,29,309 } };
			FieldMask_t tFields;
			tFields.Assign32(1);

			AndProducerSetup_t tSetup;
			tSetup.m_dTerms = { { &tFirst,tFields }, { &tSecond,tFields }, { &tThird,tFields } };
			tSetup.m_iCapacity = 1;
			tSetup.m_tMin = tMin;
			tSetup.m_uEnd = tEnd;
			auto pProducer = CreateAndProducer(tSetup);
			ASSERT_TRUE(pProducer);

			std::vector<RowID_t> dExpected;
			for ( RowID_t tRow : { RowID_t(3), RowID_t(7), INVALID_ROWID-1 } )
				if ( tRow>=tMin && tRow<tEnd )
					dExpected.push_back(tRow);

			for ( RowID_t tRow : dExpected )
			{
				auto tRows = pProducer->NextCandidates(1);
				ASSERT_EQ ( tRows.m_iCount, 1 );
				EXPECT_EQ ( tRows.m_pRows[0], tRow );
				for ( int i=0; i<3; ++i )
					EXPECT_EQ ( pProducer->ReadPayloads(i)[0].m_tRowID, tRow );
			}

			EXPECT_EQ ( pProducer->NextCandidates(1).m_iCount, 0 );
		}
}


TEST ( PostingProducer, TermBatchCapacity )
{
	for ( int iMode=0; iMode<3; ++iMode )
	{
		SCOPED_TRACE(iMode);
		ProducerQword_c tWord;
		ProducerFilter_c tFilter;
		auto tSetup = MakeTermSetup();
		tSetup.m_tPayloads = { true, true };
		if ( iMode==1 )
		{
			tSetup.m_tFields.UnsetAll();
			tSetup.m_tFields.Set(33);
			tSetup.m_bWideFields = true;
		} else if ( iMode==2 )
		{
			tWord.m_bBlock = true;
			tSetup.m_pFilter = &tFilter;
		}

		std::unique_ptr<PostingProducer_i> pProducer = CreateTermProducer ( tWord, tSetup );
		ASSERT_TRUE(pProducer);

		RowID_t tExpected = 1;
		for ( int iCapacity : { 1, 2, 3 } )
		{
			auto tBatch = pProducer->NextCandidates(iCapacity);
			ASSERT_EQ ( tBatch.m_iCount, iCapacity );
			for ( int i=0; i<iCapacity; ++i )
				EXPECT_EQ ( tBatch.GetRowID(i), tExpected+2*i );

			const auto * pDocs = pProducer->ReadPayloads(0);
			ASSERT_NE ( pDocs, nullptr );
			for ( int i=0; i<iCapacity; ++i )
			{
				EXPECT_EQ ( pDocs[i].m_tRowID, tExpected );
				EXPECT_EQ ( pDocs[i].m_iHitlistPos, 100+tExpected );
				tExpected += 2;
			}
		}

		EXPECT_EQ ( pProducer->NextCandidates(1).m_iCount, 0 );
	}
}


TEST ( PostingProducer, AndSelectedPayloads )
{
	for ( bool bBlock : { false, true } )
	{
		SCOPED_TRACE(bBlock);
		ProducerQword_c tFirst, tSecond;
		tFirst.m_bBlock = tSecond.m_bBlock = bBlock;
		FieldMask_t tFields;
		tFields.Assign32(1);

		AndProducerSetup_t tSetup;
		tSetup.m_dTerms = { { &tFirst, tFields }, { &tSecond, tFields } };
		tSetup.m_iCapacity = 3;
		tSetup.m_tPayloads = { true, true };
		std::unique_ptr<PostingProducer_i> pProducer = CreateAndProducer(tSetup);
		ASSERT_TRUE(pProducer);

		auto tBatch = pProducer->NextCandidates(3);
		ASSERT_EQ ( tBatch.m_iCount, 3 );
		EXPECT_EQ ( tBatch.GetRowID(0), 1U );
		EXPECT_EQ ( tBatch.GetRowID(2), 5U );

		RowID_t dRows[] = { 3, 5 };
		const auto * pDocs = pProducer->ReadPayloads ( 1, dRows, 2 );
		ASSERT_NE ( pDocs, nullptr );
		EXPECT_EQ ( pDocs[0].m_tRowID, 3U );
		EXPECT_EQ ( pDocs[1].m_iHitlistPos, 105 );
		pDocs = pProducer->ReadPayloads(0);
		ASSERT_NE ( pDocs, nullptr );
		EXPECT_EQ ( pDocs[0].m_tRowID, 1U );
		EXPECT_EQ ( pDocs[2].m_iHitlistPos, 105 );
		tBatch = pProducer->NextCandidates(1);
		ASSERT_EQ ( tBatch.m_iCount, 1 );
		EXPECT_EQ ( tBatch.GetRowID(0), 7U );
	}
}


TEST ( AndProducer, LegacyPayloadModesAndSelectedRows )
{
	for ( unsigned uMode=0; uMode<4; ++uMode )
	{
		SCOPED_TRACE(uMode);
		ProducerQword_c tFirst, tSecond;
		FieldMask_t tFields;
		tFields.Assign32(1);

		AndProducerSetup_t tSetup;
		tSetup.m_dTerms = { { &tFirst,tFields }, { &tSecond,tFields } };
		tSetup.m_iCapacity = 2;
		auto pProducer = CreateAndProducer(tSetup);
		ASSERT_TRUE(pProducer);

		for ( unsigned iBatch=0; iBatch<2; ++iBatch )
		{
			unsigned uPayloads = iBatch ? uMode^3 : uMode;
			pProducer->SetPayloadRequirements ( { bool(uPayloads & 1), bool(uPayloads & 2) } );

			auto tBatch = pProducer->NextCandidates(2);
			ASSERT_EQ ( tBatch.m_iCount, 2 );

			RowID_t tSelected = 3+iBatch*4;
			const auto * pDocs = pProducer->ReadPayloads ( 1, &tSelected, 1 );
			ASSERT_NE ( pDocs, nullptr );
			EXPECT_EQ ( pDocs[0].m_tRowID, tSelected );
			EXPECT_EQ ( pDocs[0].m_uFields, 1U );
			if ( uPayloads & 1 )
				EXPECT_EQ ( pDocs[0].m_uMatchHits, tSelected+1 );

			if ( uPayloads & 2 )
				EXPECT_EQ ( pDocs[0].m_iHitlistPos, 100+tSelected );

			pDocs = pProducer->ReadPayloads(0);
			ASSERT_NE ( pDocs, nullptr );
			for ( int i=0; i<2; ++i )
			{
				RowID_t tRow = 1+iBatch*4+i*2;
				EXPECT_EQ ( pDocs[i].m_tRowID, tRow );
				if ( uPayloads & 1 )
					EXPECT_EQ ( pDocs[i].m_uMatchHits, tRow+1 );

				if ( uPayloads & 2 )
					EXPECT_EQ ( pDocs[i].m_iHitlistPos, 100+tRow );
			}
		}
	}
}


TEST ( AndProducer, LegacyCancellationWithoutMatches )
{
	ProducerQword_c tFirst, tSecond;
	tFirst.m_dDocs.clear();
	for ( RowID_t i=0; i<200; ++i )
		tFirst.m_dDocs.push_back ( { i,2,1,100+i } );

	tFirst.m_iDocs = int(tFirst.m_dDocs.size());
	FieldMask_t tFields;
	tFields.Assign32(1);

	int iChecks = 0;
	AndProducerSetup_t tSetup;
	tSetup.m_dTerms = { { &tFirst,tFields }, { &tSecond,tFields } };
	tSetup.m_iCapacity = 31;
	tSetup.m_tContext = { &iChecks, [] ( void * p ) { return ++*static_cast<int *>(p)<2; }, nullptr };
	auto pProducer = CreateAndProducer(tSetup);
	ASSERT_TRUE(pProducer);
	EXPECT_EQ ( pProducer->NextCandidates(31).m_iCount, 0 );
	EXPECT_EQ ( iChecks, 2 );
	EXPECT_LE ( tFirst.m_iSingles, 32 );
	EXPECT_EQ ( tSecond.m_iSingles, 0 );
}
