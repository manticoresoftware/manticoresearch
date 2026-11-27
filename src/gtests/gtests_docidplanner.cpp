// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// Distributed under the GNU General Public License, version 3 or later.

#include <gtest/gtest.h>
#include "costestimate.h"
#include "histogram.h"
#include "secondaryindex.h"

class CachedDocidPlanner : public ::testing::Test
{
protected:
	static constexpr int DOCS = 131072;
	CSphSchema m_tSchema;
	HistogramContainer_c m_tHistograms;
	SIContainer_c m_tSI;
	CSphQuery m_tQuery;

	void SetUp() override
	{
		for ( const char * szName : { "id", "other" } )
		{
			CSphColumnInfo tAttr;
			tAttr.m_sName = szName;
			tAttr.m_eAttrType = SPH_ATTR_BIGINT;
			m_tSchema.AddAttr ( tAttr, false );
			auto pHistogram = CreateHistogram ( szName, SPH_ATTR_BIGINT, 32 );
			for ( int i=0; i<DOCS; ++i )
				pHistogram->Insert ( i*2 );
			pHistogram->Finalize();
			ASSERT_TRUE ( m_tHistograms.Add(std::move(pHistogram)) );
		}
		SetValues(1000);
	}

	void SetValues ( int nValues )
	{
		m_tQuery.m_dFilters.Resize(1);
		auto & tFilter = m_tQuery.m_dFilters[0];
		tFilter.m_sAttrName = "id";
		tFilter.m_dValues.Resize(nValues);
		for ( int i=0; i<nValues; ++i )
			tFilter.m_dValues[i] = int64_t(i+1)*DOCS/nValues;
	}

	SelectIteratorCtx_t Context ( bool bCache )
	{
		SelectIteratorCtx_t tCtx ( m_tQuery, m_tQuery.m_dFilters, m_tSchema, m_tSchema, &m_tHistograms, nullptr, m_tSI, -1, DOCS, 8 );
		tCtx.m_bHasDocidCache = bCache;
		return tCtx;
	}

	CSphVector<SecondaryIndexInfo_t> Plan ( bool bCache )
	{
		auto tCtx = Context(bCache);
		float fCost;
		StrVec_t dWarnings;
		return SelectIterators ( tCtx, fCost, dWarnings );
	}
};

TEST_F ( CachedDocidPlanner, SparseUniqueIDsBoundHistogramAndSelectLookup )
{
	auto dLegacy = Plan(false);
	ASSERT_GT ( dLegacy[0].m_iRsetEstimate, 1000 );
	auto dCached = Plan(true);
	EXPECT_LE ( dCached[0].m_iRsetEstimate, 1000 );
	EXPECT_EQ ( dCached[0].m_eType, SecondaryIndexType_e::LOOKUP );
}

TEST_F ( CachedDocidPlanner, ExternalValuesUseActualProbeCount )
{
	CSphVector<SphAttr_t> dValues;
	for ( int i=1; i<=32; ++i ) dValues.Add(i*4096);
	m_tQuery.m_dFilters[0].SetExternalValues(dValues);
	auto dCached = Plan(true);
	EXPECT_LE ( dCached[0].m_iRsetEstimate, 32 );
	EXPECT_EQ ( dCached[0].m_eType, SecondaryIndexType_e::LOOKUP );
}

TEST_F ( CachedDocidPlanner, DenseListsAndOtherAttributesRetainHistogram )
{
	SetValues(DOCS/64);
	EXPECT_LE ( Plan(true)[0].m_iRsetEstimate, DOCS/64 );
	SetValues(DOCS/64+1);
	EXPECT_EQ ( Plan(true)[0].m_iRsetEstimate, Plan(false)[0].m_iRsetEstimate );
	SetValues(1000);
	m_tQuery.m_dFilters[0].m_sAttrName = "other";
	EXPECT_EQ ( Plan(true)[0].m_iRsetEstimate, Plan(false)[0].m_iRsetEstimate );
}

TEST_F ( CachedDocidPlanner, OutOfRangeIDsBoundHistogram )
{
	SetValues(32);
	for ( auto & iValue : m_tQuery.m_dFilters[0].m_dValues ) iValue += int64_t(DOCS)*4;
	auto dInfo = Plan(true);
	EXPECT_LE ( dInfo[0].m_iRsetEstimate, 32 );
}

TEST_F ( CachedDocidPlanner, ExclusionsAndRangesRetainHistogram )
{
	auto & tFilter = m_tQuery.m_dFilters[0];
	tFilter.m_bExclude = true;
	EXPECT_EQ ( Plan(true)[0].m_iRsetEstimate, Plan(false)[0].m_iRsetEstimate );
	EXPECT_NE ( Plan(true)[0].m_eType, SecondaryIndexType_e::LOOKUP );
	tFilter.m_bExclude = false;
	tFilter.m_eType = SPH_FILTER_RANGE;
	tFilter.m_iMinValue = 200;
	tFilter.m_iMaxValue = 20000;
	EXPECT_EQ ( Plan(true)[0].m_iRsetEstimate, Plan(false)[0].m_iRsetEstimate );
}

TEST_F ( CachedDocidPlanner, ExplicitLookupHintsRemainAuthoritative )
{
	IndexHint_t tHint;
	tHint.m_sIndex = "id";
	tHint.m_eType = SecondaryIndexType_e::LOOKUP;
	tHint.m_bForce = false;
	m_tQuery.m_dIndexHints.Add(tHint);
	EXPECT_NE ( Plan(true)[0].m_eType, SecondaryIndexType_e::LOOKUP );
	m_tQuery.m_dIndexHints[0].m_bForce = true;
	EXPECT_EQ ( Plan(true)[0].m_eType, SecondaryIndexType_e::LOOKUP );
}

TEST_F ( CachedDocidPlanner, MissingIDsStillPayForEveryProbe )
{
	CSphVector<SecondaryIndexInfo_t> dInfo(1);
	dInfo[0].m_eType = SecondaryIndexType_e::LOOKUP;
	dInfo[0].m_iRsetEstimate = 0;
	auto tCtx = Context(true);
	tCtx.IgnorePushCost();
	std::unique_ptr<CostEstimate_i> pEstimate ( CreateCostEstimate ( dInfo, tCtx, -1 ) );
	float fMissCost = pEstimate->CalcQueryCost();
	EXPECT_GT ( fMissCost, 0.0f );
	dInfo[0].m_iRsetEstimate = 500;
	EXPECT_FLOAT_EQ ( fMissCost, pEstimate->CalcQueryCost() );
	tCtx.m_bHasDocidCache = false;
	dInfo[0].m_iRsetEstimate = 0;
	EXPECT_FLOAT_EQ ( pEstimate->CalcQueryCost(), 0.0f );
}
