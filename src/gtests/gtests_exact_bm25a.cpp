//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>

#include "exact_bm25a_utils.h"
#include "sphinx.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace
{

int ExactWeight ( const uint32_t * pTF, const float * pIDF, const int * pCanonical, int iTerms )
{
	float fSum = 0.0f;
	for ( int i=0; i<iTerms; ++i )
	{
		const int iNode = pCanonical[i];
		const float fTF = float ( pTF[iNode] );
		fSum += fTF ? fTF/(fTF+0.3f)*pIDF[iNode] : 0.0f;
	}
	return int ( 1000.0f*(fSum+0.5f) );
}

TEST ( ExactBM25A, RankerAdmissionIsStrict )
{
	EXPECT_TRUE ( E1ExactRankerAdmission(true,false,nullptr) );
	EXPECT_TRUE ( E1ExactRankerAdmission(false,true,"1000*bm25a(1.2,0.75,256)") );
	EXPECT_TRUE ( E1ExactRankerAdmission(false,true," 1000 * BM25A ( 1.2, 0.75, 256 ) ") );
	const std::array<const char *,8> dRejected {{
		"bm25a(1.2,0.75,256)",
		"1000*bm25a(1.2,0.75)",
		"1000*bm25a(1.2,0.75,0)",
		"1000*bm25a(1.3,0.75,256)",
		"1000*bm25a(1.2,0.70,256)",

		"1000*bm25a(1.2,0.75,256)+1",
		"sum(lcs)",
		""
	}};
	for ( const char * szExpression : dRejected )
		EXPECT_FALSE ( E1ExactRankerAdmission(false,true,szExpression) ) << szExpression;
	EXPECT_FALSE ( E1ExactRankerAdmission(false,false,"1000*bm25a(1.2,0.75,256)") );
}

TEST ( ExactBM25A, IntegerRatioBoundEncoderContract )
{
	EXPECT_EQ ( E1EncodeBM25A12_075_256(0,0), 0u );
	EXPECT_EQ ( E1EncodeBM25A12_075_256((1u<<24)+1,1), 255u );
	EXPECT_EQ ( E1EncodeBM25A12_075_256(1,(1u<<24)+1), 255u );
	EXPECT_EQ ( E1EncodeBM25A12_075_256(1,1,false), 255u );
	EXPECT_TRUE ( std::isinf(E1DecodeBM25A12_075_256(255)) );

	for ( uint32_t uTF=1; uTF<=512; ++uTF )
		for ( uint32_t uDL=0; uDL<=8192; ++uDL )
		{
			const uint8_t uCode = E1EncodeBM25A12_075_256 ( uTF, uDL );
			ASSERT_LT ( uCode, 255u );
			const float fTF = float(uTF);
			const float fDL = float(uDL);
			const float fActual = fTF / ( fTF + 1.2f*( 0.25f + 0.75f*fDL/256.0f ) );
			ASSERT_GE ( E1DecodeBM25A12_075_256(uCode), fActual ) << "tf=" << uTF << " dl=" << uDL;
		}

	const uint8_t uCounterexample = E1EncodeBM25A12_075_256 ( 89, 11554 );
	EXPECT_GE ( E1DecodeBM25A12_075_256(uCounterexample),
		89.0f/(89.0f+1.2f*(0.25f+0.75f*11554.0f/256.0f)) );
}

TEST ( ExactBM25A, RatioBoundWeightIsStrictAndSaturationAdmits )
{
	EXPECT_STREQ ( E1RankedBoundKindName(E1RankedBoundKind_e::BM25A_RATIO), "bm25a_ratio" );
	const float fIDF = 0.75f;
	const uint8_t uCode = E1EncodeBM25A12_075_256 ( 7, 300 );
	const int iUpper = E1SafeUpperRatioWeight ( uCode, fIDF );
	EXPECT_FALSE ( E1RatioBoundReject(uCode,fIDF,iUpper) );
	EXPECT_TRUE ( E1RatioBoundReject(uCode,fIDF,iUpper+1) );
	EXPECT_FALSE ( E1RatioBoundReject(255,fIDF,std::numeric_limits<int>::max()) );
	EXPECT_FALSE ( E1RatioBoundReject(uCode,0.0f,iUpper+1) );
	EXPECT_FALSE ( E1RatioBoundReject(uCode,-0.5f,iUpper+1) );
}

TEST ( ExactBM25A, GenericFallbackSpecializesOnlyNamedBM25AWithoutPackedFactors )
{
	EXPECT_TRUE ( E1FixedBM25AGenericFallback(true,false) );
	EXPECT_FALSE ( E1FixedBM25AGenericFallback(true,true) );
	EXPECT_FALSE ( E1FixedBM25AGenericFallback(false,false) );
	EXPECT_FALSE ( E1FixedBM25AGenericFallback(false,true) );
}

TEST ( ExactBM25A, DefaultContractUsesBM25AAndImplicitRelevanceSort )
{
	// Keep the historical wire value stable and resolve it only for implicit
	// queries at execution time.
	EXPECT_EQ ( SPH_RANK_DEFAULT, SPH_RANK_PROXIMITY_BM25 );
	CSphQuery tQuery;
	SetQueryDefaultsExt2(tQuery);
	EXPECT_EQ ( tQuery.m_eRanker, SPH_RANK_DEFAULT );
	EXPECT_EQ ( tQuery.m_eSort, SPH_SORT_EXTENDED );
	EXPECT_STREQ ( tQuery.m_sSortBy.cstr(), "@weight desc" );
	EXPECT_STREQ ( tQuery.m_sOrderBy.cstr(), "@weight desc" );

	const MutableIndexSettings_c tSettings;
	const QueryExecutionSettings_t tEffective = BuildQueryExecutionSettings ( tQuery, tSettings );
	EXPECT_EQ ( tEffective.m_eRanker, SPH_RANK_BM25A );
}

TEST ( ExactBM25A, ExplicitProximityBM25StillOverridesDefault )
{
	const MutableIndexSettings_c tSettings;
	CSphQuery tQuery;
	SetQueryDefaultsExt2 ( tQuery );
	tQuery.m_bExplicitRanker = true;
	tQuery.m_eRanker = SPH_RANK_PROXIMITY_BM25;
	const QueryExecutionSettings_t tEffective = BuildQueryExecutionSettings ( tQuery, tSettings );
	EXPECT_EQ ( tEffective.m_eRanker, SPH_RANK_PROXIMITY_BM25 );
}

TEST ( ExactBM25A, RankerDataFunctionsKeepCompatibleImplicitRanker )
{
	const MutableIndexSettings_c tSettings;
	for ( const char * sExpr : { "zonespanlist()", "rankfactors()", "packedfactors()", "factors()", "bm25f(1.2,0.75)" } )
	{
		CSphQuery tQuery;
		SetQueryDefaultsExt2 ( tQuery );
		tQuery.m_dItems.Add().m_sExpr = sExpr;
		const QueryExecutionSettings_t tEffective = BuildQueryExecutionSettings ( tQuery, tSettings );
		EXPECT_EQ ( tEffective.m_eRanker, SPH_RANK_PROXIMITY_BM25 ) << sExpr;
	}
}

TEST ( ExactBM25A, RankerDataCompatibilityDoesNotOverrideExplicitRanker )
{
	const MutableIndexSettings_c tSettings;
	CSphQuery tQuery;
	SetQueryDefaultsExt2 ( tQuery );
	tQuery.m_bExplicitRanker = true;
	tQuery.m_eRanker = SPH_RANK_WORDCOUNT;
	tQuery.m_dItems.Add().m_sExpr = "zonespanlist()";
	const QueryExecutionSettings_t tEffective = BuildQueryExecutionSettings ( tQuery, tSettings );
	EXPECT_EQ ( tEffective.m_eRanker, SPH_RANK_WORDCOUNT );
}

TEST ( ExactBM25A, SimilarFunctionNamesDoNotChangeImplicitRanker )
{
	const MutableIndexSettings_c tSettings;
	CSphQuery tQuery;
	SetQueryDefaultsExt2 ( tQuery );
	tQuery.m_bExplicitOrderBy = true;
	tQuery.m_sSortBy = "id asc";
	tQuery.m_sOrderBy = "id asc";
	tQuery.m_dItems.Add().m_sExpr = "my_packedfactors()";
	const QueryExecutionSettings_t tEffective = BuildQueryExecutionSettings ( tQuery, tSettings );
	EXPECT_EQ ( tEffective.m_eRanker, SPH_RANK_BM25A );
}

TEST ( ExactBM25A, DynamicTopKCoversDeepPage )
{
	EXPECT_EQ ( E1RankedTopKFromPage ( 0, 20 ), 20 );
	EXPECT_EQ ( E1RankedTopKFromPage ( 100, 20 ), 120 );
	EXPECT_EQ ( E1RankedTopKFromPage ( 2000, 20 ), 2020 );
	EXPECT_EQ ( E1RankedTopKCapacity ( 2020 ), 2020 );
	EXPECT_EQ ( E1RankedTopKFromPage ( E1_MAX_DYNAMIC_TOPK-20, 20 ), E1_MAX_DYNAMIC_TOPK );
	EXPECT_EQ ( E1RankedTopKFromPage ( E1_MAX_DYNAMIC_TOPK-19, 20 ), 0 );
	EXPECT_EQ ( E1RankedTopKFromPage ( -1, 20 ), 0 );
	EXPECT_EQ ( E1RankedTopKFromPage ( 0, 0 ), 0 );
}


TEST ( ExactBM25A, PartialUpperBoundIsConservative )
{
	const float dIDFSets[][4] {{ 1.0f,0.3f,0,0 }, { 0.7f,-0.4f,0,0 }, { -0.2f,-1.0f,0,0 }};
	const int dOrders[][4] {{0,1,2,3},{1,0,2,3}};
	for ( const auto & dIDF : dIDFSets )
		for ( const auto & dCanonical : dOrders )
			for ( uint32_t uA=0; uA<=16; ++uA )
				for ( uint32_t uB=0; uB<=16; ++uB )
				{
					uint32_t dTF[4] { uA,uB,0,0 };
					uint8_t dBounds[4] { uint8_t(std::min(254u,uA+3)),uint8_t(std::min(254u,uB+5)),0,0 };
					EXPECT_GE ( E1PartialUpperWeight(dTF,0,dBounds,dIDF,dCanonical,2), ExactWeight(dTF,dIDF,dCanonical,2) );
				}

	uint32_t dTF[4] { 300,2,0,0 };
	uint8_t dBounds[4] { 255,2,0,0 };
	float dIDF[4] { 1.2f,-0.5f,0,0 };
	int dCanonical[4] { 0,1,2,3 };
	EXPECT_GE ( E1PartialUpperWeight(dTF,0,dBounds,dIDF,dCanonical,2), ExactWeight(dTF,dIDF,dCanonical,2) );
}

TEST ( ExactBM25A, PruningPreservesTopKAndTieOrder )
{
	struct Doc_t
	{
		uint32_t m_uRow;
		uint32_t m_uID;
		uint32_t m_dTF[4];
		uint8_t m_dBounds[4];
		int m_iWeight = 0;
		int m_iUpper = 0;
	};

	std::vector<Doc_t> dDocs;
	for ( uint32_t i=0; i<48; ++i )
	{
		const uint32_t uTF0 = i==47 ? 300u : i%17;
		const uint32_t uTF1 = (i*7)%19;
		dDocs.push_back ( { i, uint32_t(1000-(i*37)%997), {uTF0,uTF1,0,0},
			{ uint8_t(uTF0>254 ? 255 : std::min(254u,uTF0+(i%4))), uint8_t(std::min(254u,uTF1+((i+1)%5))),0,0 } } );
	}

	const float dIDFSets[][4] {{ 1.0f,0.3f,0,0 }, { 0.7f,-0.4f,0,0 }, { -0.2f,-1.0f,0,0 }};
	const int dOrders[][4] {{0,1,2,3},{1,0,2,3}};
	for ( const auto & dIDF : dIDFSets )
		for ( const auto & dCanonical : dOrders )
		{
			auto dScored = dDocs;
			for ( auto & tDoc : dScored )
			{
				tDoc.m_iWeight = ExactWeight ( tDoc.m_dTF, dIDF, dCanonical, 2 );
				tDoc.m_iUpper = E1PartialUpperWeight ( tDoc.m_dTF, 0, tDoc.m_dBounds, dIDF, dCanonical, 2 );
				ASSERT_GE ( tDoc.m_iUpper, tDoc.m_iWeight );
			}
			auto fnBetter = [] ( const Doc_t & a, const Doc_t & b )
			{
				return a.m_iWeight!=b.m_iWeight ? a.m_iWeight>b.m_iWeight : a.m_uID<b.m_uID;
			};
			auto dExpected = dScored;
			std::sort ( dExpected.begin(), dExpected.end(), fnBetter );
			const size_t uTopK = 7;
			dExpected.resize ( uTopK );
			const int iThreshold = dExpected.back().m_iWeight;
			auto dPruned = dScored;
			dPruned.erase ( std::remove_if ( dPruned.begin(), dPruned.end(), [iThreshold] ( const Doc_t & tDoc ) { return tDoc.m_iUpper<iThreshold; } ), dPruned.end() );
			std::sort ( dPruned.begin(), dPruned.end(), fnBetter );
			dPruned.resize ( std::min(uTopK,dPruned.size()) );
			ASSERT_EQ ( dPruned.size(), dExpected.size() );
			for ( size_t i=0; i<uTopK; ++i )
			{
				EXPECT_EQ ( dPruned[i].m_uID, dExpected[i].m_uID );
				EXPECT_EQ ( dPruned[i].m_iWeight, dExpected[i].m_iWeight );
			}
		}

	const uint64_t uCandidateRow=9,uWorstRow=4,uCandidateID=10,uWorstID=20;
	EXPECT_LT ( uCandidateID, uWorstID );
	EXPECT_GT ( uCandidateRow, uWorstRow );
	EXPECT_FALSE ( E1TieAwareBoundReject(1234,uCandidateRow,1234,uint64_t(UINT32_MAX)) );
	EXPECT_TRUE ( E1TieAwareBoundReject(1234,uCandidateRow,1234,uWorstRow) );
}

} // namespace
