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

#include "queuecreator.h"
#include "sortbound.h"
#include "sortfilter.h"
#include "sphinxint.h"
#include "sortertraits.h"

#include <algorithm>
#include <limits>
#include <vector>


class SortBoundTest_c : public testing::Test
{
protected:
	CSphSchema	m_tSchema;
	CSphQuery	m_tQuery;

	void SetUp() override
	{
		m_tSchema.AddAttr ( CSphColumnInfo ( "id", SPH_ATTR_BIGINT ), true );
		m_tSchema.AddAttr ( CSphColumnInfo ( "ts", SPH_ATTR_BIGINT ), true );
		m_tSchema.AddAttr ( CSphColumnInfo ( "f", SPH_ATTR_FLOAT ), true );
		m_tQuery.m_eSort = SPH_SORT_EXTENDED;
		m_tQuery.m_sSortBy = "ts desc";
		m_tQuery.m_iMaxMatches = 3;
		m_tQuery.m_iLimit = 3;
	}


	std::unique_ptr<ISphMatchSorter> Create()
	{
		SphQueueSettings_t tSettings(m_tSchema);
		tSettings.m_bComputeItems = true;
		tSettings.m_iMaxMatches = m_tQuery.m_iMaxMatches;

		SphQueueRes_t tRes;
		CSphString sError;
		std::unique_ptr<ISphMatchSorter> pSorter ( sphCreateQueue ( tSettings, m_tQuery, sError, tRes ) );
		EXPECT_TRUE ( pSorter ) << sError.cstr();
		return pSorter;
	}


	void Push ( ISphMatchSorter & tSorter, RowID_t tRow, SphAttr_t tValue )
	{
		CSphMatch tMatch;
		tMatch.Reset ( tSorter.GetSchema()->GetDynamicSize() );
		tMatch.m_tRowID = tRow;
		tMatch.SetAttr ( tSorter.GetSchema()->GetAttr("id")->m_tLocator, tRow+1 );
		tMatch.SetAttr ( tSorter.GetState().m_tLocator[0], tValue );
		tSorter.Push(tMatch);
	}
};


TEST_F ( SortBoundTest_c, FullnessAndConservativeThreshold )
{
	for ( bool bKbuffer : { false, true } )
		for ( bool bDescending : { false, true } )
		{
			SCOPED_TRACE ( testing::Message() << "kbuffer=" << bKbuffer << " desc=" << bDescending );
			m_tQuery.m_bSortKbuffer = bKbuffer;
			m_tQuery.m_sSortBy = bDescending ? "ts desc" : "ts asc";
			auto pSorter = Create();
			ASSERT_TRUE(pSorter);

			SortBound_c tBound;
			ASSERT_TRUE ( tBound.Setup ( m_tQuery, m_tSchema, *pSorter ) );
			EXPECT_STREQ ( tBound.GetAttr().cstr(), "ts" );
			EXPECT_EQ ( tBound.IsDescending(), bDescending );
			EXPECT_FALSE ( tBound.Update() );
			EXPECT_FALSE ( tBound.IsSet() );

			std::vector<SphAttr_t> dValues;
			for ( int i=0; i<100; ++i )
			{
				SphAttr_t tValue = i<3 ? 0 : ( bDescending ? i : -i );
				if ( i==98 )
					tValue = std::numeric_limits<SphAttr_t>::min();
				else if ( i==99 )
					tValue = std::numeric_limits<SphAttr_t>::max();

				Push ( *pSorter, i, tValue );
				dValues.push_back(tValue);
				std::sort ( dValues.begin(), dValues.end() );

				bool bFull = true;
				const auto * pWorst = pSorter->GetWorst(&bFull);
				EXPECT_EQ ( bFull, i>=2 );
				if ( bKbuffer )
					EXPECT_EQ ( pSorter->GetWorst(), nullptr );
				else
					EXPECT_EQ ( pSorter->GetWorst(), pWorst );

				tBound.Update();
				EXPECT_EQ ( tBound.IsSet(), i>=2 );
				if ( i<2 )
					continue;

				ASSERT_NE ( pWorst, nullptr );
				SphAttr_t tExact = bDescending ? dValues[dValues.size()-3] : dValues[2];
				if ( bDescending )
					EXPECT_LE ( tBound.GetValue(), tExact );
				else
					EXPECT_GE ( tBound.GetValue(), tExact );

				EXPECT_FALSE ( tBound.Update() );
			}

			auto pClone = std::unique_ptr<ISphMatchSorter> ( pSorter->Clone() );
			ASSERT_TRUE ( tBound.Setup ( m_tQuery, m_tSchema, *pClone ) );
			EXPECT_FALSE ( tBound.Update() );
			EXPECT_FALSE ( tBound.IsSet() );

			CSphMatch dMatches[3];
			ASSERT_EQ ( pSorter->Flatten(dMatches), 3 );
			for ( int i=0; i<3; ++i )
				EXPECT_EQ ( dMatches[i].GetAttr ( pSorter->GetState().m_tLocator[0] ), bDescending ? dValues[dValues.size()-1-i] : dValues[i] );

			bool bFull = true;
			EXPECT_EQ ( pSorter->GetWorst(&bFull), nullptr );
			EXPECT_FALSE(bFull);
		}
}


TEST_F ( SortBoundTest_c, AliasAndUnsupportedSorts )
{
	auto & tItem = m_tQuery.m_dItems.Add();
	tItem.m_sExpr = "ts";
	tItem.m_sAlias = "stamp";
	m_tQuery.m_sSortBy = "stamp desc";
	auto pSorter = Create();
	ASSERT_TRUE(pSorter);

	SortBound_c tBound;
	ASSERT_TRUE ( tBound.Setup ( m_tQuery, m_tSchema, *pSorter ) );
	EXPECT_STREQ ( tBound.GetAttr().cstr(), "ts" );
	for ( int i=0; i<3; ++i )
		Push ( *pSorter, i, i+10 );

	ASSERT_TRUE ( tBound.Update() );
	EXPECT_EQ ( tBound.GetValue(), 10 );

	m_tQuery.m_sSortBy = "ts desc, id asc";
	pSorter = Create();
	ASSERT_TRUE ( tBound.Setup ( m_tQuery, m_tSchema, *pSorter ) );

	for ( const char * sSort : { "f desc", "@weight desc", "@random" } )
	{
		m_tQuery.m_sSortBy = sSort;
		pSorter = Create();
		ASSERT_TRUE(pSorter);
		EXPECT_FALSE ( tBound.Setup ( m_tQuery, m_tSchema, *pSorter ) );
		EXPECT_FALSE ( tBound.IsSet() );
		EXPECT_FALSE ( tBound.Update() );
	}

	tItem.m_sExpr = "ts+1";
	m_tQuery.m_sSortBy = "stamp desc";
	pSorter = Create();
	ASSERT_TRUE(pSorter);
	EXPECT_FALSE ( tBound.Setup ( m_tQuery, m_tSchema, *pSorter ) );
}


TEST_F ( SortBoundTest_c, OffsetCapacityAndUnsupportedCollector )
{
	m_tQuery.m_iMaxMatches = 10;
	m_tQuery.m_iOffset = 2;
	m_tQuery.m_iLimit = 3;
	for ( bool bKbuffer : { false, true } )
	{
		m_tQuery.m_bSortKbuffer = bKbuffer;
		auto pSorter = Create();
		ASSERT_TRUE(pSorter);
		for ( int i=0; i<5; ++i )
		{
			Push ( *pSorter, i, i );

			bool bFull = true;
			pSorter->GetWorst(&bFull);
			EXPECT_EQ ( bFull, i==4 );
		}
	}

	CSphVector<BYTE> dCollected;
	std::unique_ptr<ISphMatchSorter> pCollector ( CreateCollectQueue ( 3, dCollected ) );
	bool bFull = true;
	EXPECT_EQ ( pCollector->GetWorst(&bFull), nullptr );
	EXPECT_FALSE(bFull);
}


TEST_F ( SortBoundTest_c, CompetitiveRowsAndMinMax )
{
	for ( bool bDescending : { false, true } )
	{
		m_tQuery.m_sSortBy = bDescending ? "ts desc" : "ts asc";
		auto pSorter = Create();
		SortBound_c tBound;
		ASSERT_TRUE ( tBound.Setup ( m_tQuery, m_tSchema, *pSorter ) );

		CSphAttrLocator tLocator;
		tLocator.m_iBitCount = 64;
		tLocator.m_iBitOffset = 0;
		const int iRows = 2*DOCINFO_INDEX_FREQ+1;
		std::vector<DWORD> dRows(iRows*2);
		std::vector<DWORD> dMinMax(4*4);
		for ( int i=0; i<iRows; ++i )
			sphSetRowAttr ( dRows.data()+2*i, tLocator, i<DOCINFO_INDEX_FREQ ? 0 : 10 );

		for ( int i=0; i<3; ++i )
		{
			sphSetRowAttr ( dMinMax.data()+4*i, tLocator, i ? 10 : 0 );
			sphSetRowAttr ( dMinMax.data()+4*i+2, tLocator, i ? 10 : 0 );
		}

		sphSetRowAttr ( dMinMax.data()+12, tLocator, 0 );
		sphSetRowAttr ( dMinMax.data()+14, tLocator, 10 );

		SortBoundFilter_c tFilter ( tBound, tLocator, dRows.data(), dMinMax.data(), iRows, 2, nullptr );
		EXPECT_EQ ( tFilter.AdvanceTo(0), 0 );
		RowID_t dBefore[] = { 0, DOCINFO_INDEX_FREQ, iRows-1 };
		EXPECT_EQ ( tFilter.SelectRows ( dBefore, 3 ), 3 );
		EXPECT_FALSE ( tFilter.HasPruned() );

		for ( int i=0; i<3; ++i )
			Push ( *pSorter, i, bDescending ? 10 : 0 );

		ASSERT_TRUE ( tBound.Update() );
		EXPECT_EQ ( tFilter.AdvanceTo(0), bDescending ? DOCINFO_INDEX_FREQ : 0 );
		EXPECT_EQ ( tFilter.AdvanceTo(iRows-1), bDescending ? iRows-1 : INVALID_ROWID );
		RowID_t dAfter[] = { 0, DOCINFO_INDEX_FREQ, iRows-1 };
		EXPECT_EQ ( tFilter.SelectRows ( dAfter, 3 ), bDescending ? 2 : 1 );
		EXPECT_EQ ( dAfter[0], bDescending ? DOCINFO_INDEX_FREQ : 0 );
		EXPECT_TRUE ( tFilter.HasPruned() );

		// Strengthening also rejects rows already decoded before publication.
		for ( int i=3; i<6; ++i )
			Push ( *pSorter, i, bDescending ? std::numeric_limits<SphAttr_t>::max() : std::numeric_limits<SphAttr_t>::min() );

		ASSERT_TRUE ( tBound.Update() );
		RowID_t dBuffered[] = { 0, DOCINFO_INDEX_FREQ, iRows-1 };
		EXPECT_EQ ( tFilter.SelectRows ( dBuffered, 3 ), 0 );
		EXPECT_EQ ( tFilter.AdvanceTo(0), INVALID_ROWID );
	}
}


TEST_F ( SortBoundTest_c, IndependentFilterBranches )
{
	auto pSorter = Create();
	SortBound_c tBound;
	ASSERT_TRUE ( tBound.Setup ( m_tQuery, m_tSchema, *pSorter ) );

	constexpr int iRows = DOCINFO_INDEX_FREQ*2;
	DWORD dRows[iRows] {};
	DWORD dMinMax[] = { 0, 0, 100, 100, 0, 100 };
	std::fill_n ( dRows+DOCINFO_INDEX_FREQ, DOCINFO_INDEX_FREQ, 100 );

	CSphAttrLocator tLoc;
	tLoc.m_iBitCount = 32;
	tLoc.m_iBitOffset = 0;
	SortBoundFilter_c tFilter ( tBound, tLoc, dRows, dMinMax, iRows, 1, nullptr );
	auto pRight = tFilter.CloneCursor();
	auto pNested = pRight->CloneCursor();
	ASSERT_TRUE ( pRight && pNested );
	EXPECT_EQ ( tFilter.AdvanceTo(iRows-1), iRows-1 );
	EXPECT_EQ ( pRight->AdvanceTo(0), 0 );

	for ( int i=0; i<3; ++i )
		Push ( *pSorter, i, 100 );

	ASSERT_TRUE(tBound.Update());
	EXPECT_EQ ( pNested->AdvanceTo(0), DOCINFO_INDEX_FREQ );
	EXPECT_TRUE(tFilter.HasPruned());
	EXPECT_EQ ( pRight->AdvanceTo(1), DOCINFO_INDEX_FREQ );
	EXPECT_EQ ( tFilter.AdvanceTo(iRows-1), iRows-1 );

	RowID_t dCandidates[] = { 0, DOCINFO_INDEX_FREQ, iRows-1 };
	EXPECT_EQ ( pRight->SelectRows ( dCandidates, 3 ), 2 );
	EXPECT_EQ ( dCandidates[0], DOCINFO_INDEX_FREQ );

	for ( int i=3; i<6; ++i )
		Push ( *pSorter, i, 101 );

	ASSERT_TRUE(tBound.Update());
	EXPECT_EQ ( pNested->AdvanceTo(DOCINFO_INDEX_FREQ), INVALID_ROWID );
	EXPECT_EQ ( tFilter.AdvanceTo(iRows-1), INVALID_ROWID );
	EXPECT_EQ ( pRight->SelectRows ( dCandidates, 2 ), 0 );
}
