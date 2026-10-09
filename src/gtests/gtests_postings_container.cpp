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

#include "postings_container_reader.h"
#include "postings_container_writer.h"
#include "norm_store.h"
#include "rt_field_norms.h"
#include "threadutils.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

namespace
{

using ByteVec_t = std::vector<uint8_t>;

class VectorPublicIDReader_c final : public e1::PublicIDReader_i
{
public:
	explicit VectorPublicIDReader_c ( const std::vector<uint64_t> & dIDs ) : m_dIDs ( dIDs ) {}
	uint32_t Rows() const override { return uint32_t(m_dIDs.size()); }
	bool Get ( uint32_t uRow, uint64_t & uValue ) const override
	{
		if ( uRow>=m_dIDs.size() ) return false;
		uValue = m_dIDs[uRow];
		return true;
	}
private:
	const std::vector<uint64_t> & m_dIDs;
};

void PutValue ( ByteVec_t & dData, size_t uOffset, uint64_t uValue, unsigned uBytes )
{
	for ( unsigned i=0; i<uBytes; ++i )
		dData[uOffset+i] = uint8_t ( uValue >> ( 8*i ) );
}

void UpdateChecksum ( ByteVec_t & dData )
{
	PutValue ( dData, 16, dData.size(), 8 );
	const uint32_t uHeaderSize = e1::U32 ( dData.data()+12 );
	PutValue ( dData, 32, e1::CRC(dData.data()+uHeaderSize,dData.size()-uHeaderSize), 4 );
}

ByteVec_t ReadFile ( const std::string & sPath )
{
	std::ifstream tFile ( sPath, std::ios::binary );
	return { std::istreambuf_iterator<char>(tFile), {} };
}

class PostingsContainerTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		const testing::TestInfo * pInfo = testing::UnitTest::GetInstance()->current_test_info();
		m_sBase = "__postings_container_" + std::to_string ( GetOsProcessId() ) + "_" + pInfo->name();
	}

	void TearDown() override
	{
		for ( const char * szExt : { ".spd", ".spi", ".spp", ".spn", ".sph" } )
			std::remove ( (m_sBase+szExt).c_str() );
	}

	void WriteContainer ( const std::vector<e1::Posting> & dPostings, bool bHasHitlist=false )
	{
		{ std::ofstream(m_sBase+".spi",std::ios::binary).write("dict",4); }
		{ std::ofstream(m_sBase+".spp",std::ios::binary).put('\1'); }
		uint64_t uHits = 0;
		for ( const auto & tPosting : dPostings )
			uHits += tPosting.m_uTF;
		std::string sError;
		e1::Writer tWriter;
		ASSERT_TRUE ( tWriter.Open(m_sBase+".spd",sError) ) << sError;
		ASSERT_TRUE ( tWriter.FinishTerm(1,dPostings,uHits,bHasHitlist,sError) ) << sError;
		ASSERT_TRUE ( tWriter.Finalize(m_sBase+".spi",m_sBase+".spp",sError) ) << sError;
	}

	std::string m_sBase;
};

TEST ( PostingsContainer, CRC32KnownVectorAndIncrementalUpdate )
{
	const char * szValue = "123456789";
	EXPECT_EQ ( e1::CRC ( reinterpret_cast<const uint8_t *>(szValue), 9 ), 0xcbf43926u );
	uint32_t uCRC = ~0u;
	uCRC = e1::CRCUpdate ( uCRC, reinterpret_cast<const uint8_t *>(szValue), 4 );
	uCRC = e1::CRCUpdate ( uCRC, reinterpret_cast<const uint8_t *>(szValue+4), 5 );
	EXPECT_EQ ( ~uCRC, 0xcbf43926u );
}

TEST_F ( PostingsContainerTest, TrustedGenerationIsOneShotAndRejectsModifiedComponents )
{
	const std::string sPostings = m_sBase+".spd";
	const std::string sDict = m_sBase+".spi";
	const std::string sHits = m_sBase+".spp";
	{ std::ofstream(sPostings,std::ios::binary).write("postings",8); }
	{ std::ofstream(sDict,std::ios::binary).write("dict",4); }
	{ std::ofstream(sHits,std::ios::binary).write("hits",4); }

	e1::MarkTrustedGeneration ( sPostings, sDict, sHits, 8, 1, 2, 3 );
#if !defined(_WIN32)
	EXPECT_TRUE ( e1::ConsumeTrustedGeneration(sPostings,sDict,sHits,8,1,2,3) );
#else
	EXPECT_FALSE ( e1::ConsumeTrustedGeneration(sPostings,sDict,sHits,8,1,2,3) );
#endif
	EXPECT_FALSE ( e1::ConsumeTrustedGeneration(sPostings,sDict,sHits,8,1,2,3) );

	e1::MarkTrustedGeneration ( sPostings, sDict, sHits, 8, 1, 2, 3 );
	{ std::ofstream(sDict,std::ios::binary|std::ios::app).put('!'); }
	EXPECT_FALSE ( e1::ConsumeTrustedGeneration(sPostings,sDict,sHits,8,1,2,3) );
}

TEST_F ( PostingsContainerTest, V10PublicIDProofIsContentBoundAndDeepValidated )
{
	std::vector<uint64_t> dPublicIDs { 400, 300, 200, 100 };
	std::vector<e1::Posting> dPostings;
	for ( uint32_t uRow=0; uRow<dPublicIDs.size(); ++uRow )
		dPostings.emplace_back ( uRow, 1, 1, 0, 0, dPublicIDs[uRow] );
	const std::string sPostings = m_sBase+".spd";
	const std::string sDict = m_sBase+".spi";
	const std::string sHits = m_sBase+".spp";
	auto fnWrite = [&] ()
	{
		{ std::ofstream(sDict,std::ios::binary).write("dict",4); }
		{ std::ofstream(sHits,std::ios::binary).put('\1'); }
		std::string sError;
		e1::Writer tWriter;
		EXPECT_TRUE ( tWriter.Open(sPostings,sError) ) << sError;
		tWriter.BindPublicIDs ( dPublicIDs.data(), uint32_t(dPublicIDs.size()), true );
		EXPECT_TRUE ( tWriter.FinishTerm(1,static_cast<const std::vector<e1::Posting> &>(dPostings),dPostings.size(),false,sError) ) << sError;
		EXPECT_TRUE ( tWriter.Finalize(sDict,sHits,sError) ) << sError;
	};
	auto fnConsume = [&] ( const std::vector<uint64_t> & dIDs )
	{
		auto dRaw = ReadFile(sPostings);
		e1::PublicIDDigest_t dDigest {};
		const bool bDigest = e1::PublicIDContentDigest ( uint32_t(dIDs.size()), [&dIDs] ( uint32_t uRow, uint64_t & uValue ) { uValue=dIDs[uRow]; return true; }, dDigest );
		EXPECT_TRUE ( bDigest );
		return e1::ConsumeTrustedGeneration ( sPostings, sDict, sHits, e1::U64(dRaw.data()+16), e1::U32(dRaw.data()+32), e1::U32(dRaw.data()+36), e1::U32(dRaw.data()+40), uint32_t(dIDs.size()), &dDigest );
	};

	fnWrite();
	auto tProof = fnConsume(dPublicIDs);
#if !defined(_WIN32)
	ASSERT_TRUE ( tProof.m_bGeneration );
	ASSERT_TRUE ( tProof.m_bPublicIDs );
#else
	ASSERT_FALSE ( tProof.m_bGeneration );
#endif
	auto dRaw=ReadFile(sPostings), dDict=ReadFile(sDict), dHits=ReadFile(sHits);
	VectorPublicIDReader_c tPublicIDs ( dPublicIDs );
	std::string sError;
	e1::Store tStore;
	e1::OpenValidationPath_e ePath = e1::OpenValidationPath_e::NONE;
	ASSERT_TRUE ( tStore.Open(dRaw.data(),dRaw.size(),uint32_t(dPublicIDs.size()),dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,tProof.m_bGeneration,nullptr,nullptr,&tPublicIDs,&ePath,tProof.m_bPublicIDs) ) << sError;
#if !defined(_WIN32)
	EXPECT_EQ ( ePath, e1::OpenValidationPath_e::DEEP );
	EXPECT_TRUE ( tStore.PublicIDMinValidated() );
#endif
	EXPECT_FALSE ( fnConsume(dPublicIDs).m_bGeneration );

	fnWrite();
	auto dWrongIDs = dPublicIDs;
	dWrongIDs[0] = 1;
	auto tWrongProof = fnConsume(dWrongIDs);
#if !defined(_WIN32)
	ASSERT_TRUE ( tWrongProof.m_bGeneration );
	EXPECT_FALSE ( tWrongProof.m_bPublicIDs );
#endif
	VectorPublicIDReader_c tWrongIDs ( dWrongIDs );
	e1::Store tWrongStore;
	EXPECT_FALSE ( tWrongStore.Open(dRaw.data(),dRaw.size(),uint32_t(dWrongIDs.size()),dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,tWrongProof.m_bGeneration,nullptr,nullptr,&tWrongIDs,&ePath,tWrongProof.m_bPublicIDs) );
	EXPECT_NE ( sError.find("unsafe public ID minimum"), std::string::npos );
	EXPECT_FALSE ( fnConsume(dPublicIDs).m_bGeneration );

	fnWrite();
	dRaw=ReadFile(sPostings);
	{ std::ofstream(sPostings,std::ios::binary|std::ios::app).put('!'); }
	e1::PublicIDDigest_t dDigest {};
	ASSERT_TRUE ( e1::PublicIDContentDigest ( uint32_t(dPublicIDs.size()), [&dPublicIDs] ( uint32_t uRow, uint64_t & uValue ) { uValue=dPublicIDs[uRow]; return true; }, dDigest ) );
	EXPECT_FALSE ( e1::ConsumeTrustedGeneration(sPostings,sDict,sHits,e1::U64(dRaw.data()+16),e1::U32(dRaw.data()+32),e1::U32(dRaw.data()+36),e1::U32(dRaw.data()+40),uint32_t(dPublicIDs.size()),&dDigest).m_bGeneration );
}

TEST_F ( PostingsContainerTest, StreamedDirectoryUsesImplicitContiguousKeys )
{
	{ std::ofstream(m_sBase+".spi",std::ios::binary).write("dict",4); }
	{ std::ofstream(m_sBase+".spp",std::ios::binary).put('\1'); }
	const std::vector<e1::Posting> dFirst { { 1,1,1,0 } };
	const std::vector<e1::Posting> dSecond { { 2,1,1,( uint64_t(1)<<63 ) | 1 } };
	std::string sError;
	e1::Writer tWriter;
	ASSERT_TRUE ( tWriter.Open(m_sBase+".spd",sError) ) << sError;
	ASSERT_TRUE ( tWriter.FinishTerm(1,dFirst,1,false,sError) ) << sError;
	EXPECT_FALSE ( tWriter.FinishTerm(3,dSecond,1,true,sError) );
	ASSERT_TRUE ( tWriter.FinishTerm(2,dSecond,1,true,sError) ) << sError;
	ASSERT_TRUE ( tWriter.Finalize(m_sBase+".spi",m_sBase+".spp",sError) ) << sError;

	auto dRaw=ReadFile(m_sBase+".spd"), dDict=ReadFile(m_sBase+".spi"), dHits=ReadFile(m_sBase+".spp");
	ASSERT_GE ( dRaw.size(), 88u );
	const uint64_t uDirectory = e1::U64 ( dRaw.data()+48 );
	EXPECT_EQ ( dRaw.size()-uDirectory, 32u );
	e1::Store tStore;
	ASSERT_TRUE ( tStore.Open(dRaw.data(),dRaw.size(),4,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError) ) << sError;
	EXPECT_NE ( tStore.Find(1), nullptr );
	EXPECT_NE ( tStore.Find(2), nullptr );
	EXPECT_EQ ( tStore.Find(3), nullptr );
	EXPECT_EQ ( tStore.View(1).DF(), 1u );
	EXPECT_EQ ( tStore.View(2).DF(), 1u );
	EXPECT_FALSE ( tStore.View(1).HasHitlist() );
	EXPECT_TRUE ( tStore.View(2).HasHitlist() );
}


TEST_F ( PostingsContainerTest, CompactMasksAndMixedReferences )
{
	std::vector<e1::Posting> dPostings;
	for ( uint32_t i=0; i<4096; ++i )
	{
		const uint32_t uTF = 1u+(i&1u);
		const uint32_t uMask = uTF==1 ? uint32_t(1u<<((i/2u)%2u)) : 3u;
		const uint64_t uRef = uTF==1 ? ( uint64_t(1)<<63 ) | ( uint64_t(__builtin_ctz(uMask))<<24 ) | ((i&4u) ? (uint64_t(1)<<23) : 0u) | uint64_t(1u+(i%127u)) : 1000000u+uint64_t(i)*4u;
		dPostings.push_back ( { i, uTF, uMask, uRef } );
	}
	WriteContainer ( dPostings, true );
	auto dRaw=ReadFile(m_sBase+".spd"), dDict=ReadFile(m_sBase+".spi"), dHits=ReadFile(m_sBase+".spp");
	dHits.resize ( 1000000u+dPostings.size()*4u+3u );
	for ( const auto & tPosting : dPostings )
		if ( tPosting.m_uTF>1 )
		{
			const size_t uOffset = size_t(tPosting.m_uRef);
			dHits[uOffset] = 1;
			dHits[uOffset+1] = 1;
			dHits[uOffset+2] = 0;
		}
	PutValue ( dRaw, 40, e1::CRC(dHits.data(),dHits.size()), 4 );
	EXPECT_LT ( dRaw.size(), 12000u );

	std::string sError;
	e1::Store tStore;
	ASSERT_TRUE ( tStore.Open(dRaw.data(),dRaw.size(),4096,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError) ) << sError;
	e1::Cursor tCursor;
	tCursor.Bind ( tStore, 1 );
	for ( const auto & tPosting : dPostings )
	{
		uint32_t uRow=UINT32_MAX,uTF=0,uMask=0;
		uint64_t uRef=0;
		ASSERT_TRUE ( tCursor.Next(uRow,&uTF,&uMask,&uRef) );
		EXPECT_EQ ( uRow, tPosting.m_uRow );
		EXPECT_EQ ( uTF, tPosting.m_uTF );
		EXPECT_EQ ( uMask, tPosting.m_uMask );
		EXPECT_EQ ( uRef, tPosting.m_uRef );
	}

	// A one-bit 32-bit mask cannot identify a wider encoded field.  One such
	// reference must make the whole metadata block use the lossless fallback.
	const std::vector<e1::Posting> dWideField {
		{ 0, 1, 1, (uint64_t(1)<<63) | (uint64_t(37)<<24) | 7 },
		{ 1, 1, 1, (uint64_t(1)<<63) | 9 }
	};
	WriteContainer ( dWideField, true );
	dRaw=ReadFile(m_sBase+".spd"); dDict=ReadFile(m_sBase+".spi"); dHits=ReadFile(m_sBase+".spp");
	e1::Store tWideStore;
	ASSERT_TRUE ( tWideStore.Open(dRaw.data(),dRaw.size(),2,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError) ) << sError;
	e1::Cursor tWideCursor;
	tWideCursor.Bind ( tWideStore, 1 );
	for ( const auto & tPosting : dWideField )
	{
		uint32_t uRow=UINT32_MAX,uTF=0,uMask=0;
		uint64_t uRef=0;
		ASSERT_TRUE ( tWideCursor.Next(uRow,&uTF,&uMask,&uRef) );
		EXPECT_EQ ( uRow, tPosting.m_uRow );
		EXPECT_EQ ( uRef, tPosting.m_uRef );
	}
}

TEST_F ( PostingsContainerTest, CurrentWriterPersistsExactSafeRatioBoundsByOrdinal )
{
	std::vector<e1::Posting> dPostings;
	std::vector<uint32_t> dTotalDL ( 130 );
	for ( uint32_t i=0; i<130; ++i )
	{
		dTotalDL[i] = i==70 ? 11554u : 20u+i;
		dPostings.push_back ( { i, i==70 ? 89u : 1u+(i%11), 1, 0 } );
	}
	{ std::ofstream(m_sBase+".spi",std::ios::binary).write("dict",4); }
	{ std::ofstream(m_sBase+".spp",std::ios::binary).put('\1'); }
	std::string sError;
	e1::Writer tWriter;
	ASSERT_TRUE ( tWriter.Open(m_sBase+".spd",sError) ) << sError;
	tWriter.BindTotalDL ( dTotalDL.data(), uint32_t(dTotalDL.size()), true );
	uint64_t uHits = 0;
	for ( const auto & tPosting : dPostings ) uHits += tPosting.m_uTF;
	ASSERT_TRUE ( tWriter.FinishTerm(1,dPostings,uHits,false,sError) ) << sError;
	ASSERT_TRUE ( tWriter.Finalize(m_sBase+".spi",m_sBase+".spp",sError) ) << sError;

	auto dRaw=ReadFile(m_sBase+".spd"), dDict=ReadFile(m_sBase+".spi"), dHits=ReadFile(m_sBase+".spp");
	ASSERT_GE ( dRaw.size(), 56u );
	EXPECT_EQ ( std::string(reinterpret_cast<const char *>(dRaw.data()),8), "E1POST10" );
	EXPECT_EQ ( e1::U32(dRaw.data()+8), 10u );
	e1::norms::Builder tNormBuilder ( 1 );
	for ( uint32_t uDL : dTotalDL )
		ASSERT_TRUE ( tNormBuilder.AddRow(&uDL,1,sError) ) << sError;
	std::vector<uint8_t> dNormBytes;
	ASSERT_TRUE ( tNormBuilder.Build(dNormBytes,sError) ) << sError;
	e1::norms::Store tNormStore;
	ASSERT_TRUE ( tNormStore.Open(dNormBytes.data(),dNormBytes.size(),sError) ) << sError;
	e1::Store tStore;
	// The current format always takes the deep validation path.
	ASSERT_TRUE ( tStore.Open(dRaw.data(),dRaw.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,true,nullptr,&tNormStore) ) << sError;
	e1::Cursor tCursor;
	tCursor.Bind ( tStore, 1 );
	ASSERT_TRUE ( tCursor.HasBM25ARatioBounds() );
	EXPECT_FALSE ( tCursor.HasPublicIdMinBounds() );
	for ( uint32_t uBlock=0; uBlock<3; ++uBlock )
	{
		uint8_t uActual = 0;
		ASSERT_TRUE ( tCursor.BM25ARatioBound(uBlock,uActual) );
		uint8_t uExpected = 0;
		const uint32_t uEnd = std::min<uint32_t> ( (uBlock+1)*64, dTotalDL.size() );
		for ( uint32_t i=uBlock*64; i<uEnd; ++i )
			uExpected = std::max ( uExpected, E1EncodeBM25A12_075_256(i==70 ? 89u : 1u+(i%11),dTotalDL[i]) );
		EXPECT_EQ ( uActual, uExpected );
	}

	// Deep reopen validates every finite code against the authoritative norms.
	e1::Store tDeepStore;
	ASSERT_TRUE ( tDeepStore.Open(dRaw.data(),dRaw.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,false,nullptr,&tNormStore) ) << sError;

	for ( uint32_t uObsoleteVersion : { 6u, 7u, 9u } )
	{
		auto dObsolete = dRaw;
		const std::string sMagic = "E1POST0"+std::to_string(uObsoleteVersion);
		memcpy ( dObsolete.data(), sMagic.data(), 8 );
		PutValue ( dObsolete, 8, uObsoleteVersion, 4 );
		e1::Store tObsolete;
		EXPECT_FALSE ( tObsolete.Open(dObsolete.data(),dObsolete.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,false,nullptr,&tNormStore) );
		EXPECT_NE ( sError.find("format/length"), std::string::npos );
	}

	auto dUnderestimate = dRaw;
	const auto * pTerm = dUnderestimate.data()+56;
	const uint32_t uMetaCount = e1::U32(pTerm+8);
	const auto * pLastMeta = dUnderestimate.data()+e1::U64(pTerm+16)+uint64_t(uMetaCount-1)*16;
	const size_t uBoundsOffset = e1::U64(pLastMeta)+e1::Meta4Bytes(e1::U32(pLastMeta+8),e1::U32(pLastMeta+12));
	ASSERT_GT ( dUnderestimate[uBoundsOffset], 0u );
	--dUnderestimate[uBoundsOffset];
	UpdateChecksum ( dUnderestimate );
	e1::Store tRejected;
	EXPECT_FALSE ( tRejected.Open(dUnderestimate.data(),dUnderestimate.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,false,nullptr,&tNormStore) );
	EXPECT_NE ( sError.find("unsafe BM25A ratio bound"), std::string::npos );
}

TEST_F ( PostingsContainerTest, CurrentPersistsConservativePublicIdMinimumPerOrdinalBlock )
{
	std::vector<e1::Posting> dPostings;
	std::vector<uint32_t> dTotalDL ( 130, 32 );
	std::vector<uint64_t> dPublicIDs;
	for ( uint32_t i=0; i<130; ++i )
	{
		dPostings.push_back ( { i, 1, 1, 0, 0, 10000u-i*13u } );
		dPublicIDs.push_back ( dPostings.back().m_uPublicID );
	}
	{ std::ofstream(m_sBase+".spi",std::ios::binary).write("dict",4); }
	{ std::ofstream(m_sBase+".spp",std::ios::binary).put('\1'); }
	std::string sError;
	e1::Writer tWriter;
	ASSERT_TRUE ( tWriter.Open(m_sBase+".spd",sError) ) << sError;
	tWriter.BindTotalDL ( dTotalDL.data(), uint32_t(dTotalDL.size()), true );
	uint64_t uHits = 0;
	std::array<uint64_t,3> dExpectedMin { UINT64_MAX, UINT64_MAX, UINT64_MAX };
	for ( uint32_t i=0; i<dPostings.size(); ++i )
	{
		uHits += dPostings[i].m_uTF;
		dExpectedMin[i/64] = std::min ( dExpectedMin[i/64], dPostings[i].m_uPublicID );
	}
	ASSERT_TRUE ( tWriter.FinishTerm(1,dPostings,uHits,false,sError) ) << sError;
	ASSERT_TRUE ( tWriter.Finalize(m_sBase+".spi",m_sBase+".spp",sError) ) << sError;

	auto dRaw=ReadFile(m_sBase+".spd"), dDict=ReadFile(m_sBase+".spi"), dHits=ReadFile(m_sBase+".spp");
	ASSERT_GE ( dRaw.size(), 56u );
	EXPECT_EQ ( std::string(reinterpret_cast<const char *>(dRaw.data()),8), "E1POST10" );
	EXPECT_EQ ( e1::U32(dRaw.data()+8), 10u );
	e1::norms::Builder tNormBuilder ( 1 );
	for ( uint32_t uDL : dTotalDL ) ASSERT_TRUE ( tNormBuilder.AddRow(&uDL,1,sError) ) << sError;
	std::vector<uint8_t> dNormBytes;
	ASSERT_TRUE ( tNormBuilder.Build(dNormBytes,sError) ) << sError;
	e1::norms::Store tNormStore;
	ASSERT_TRUE ( tNormStore.Open(dNormBytes.data(),dNormBytes.size(),sError) ) << sError;
	e1::Store tStore;
	e1::OpenValidationPath_e eValidationPath = e1::OpenValidationPath_e::NONE;
	ASSERT_TRUE ( tStore.Open(dRaw.data(),dRaw.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,true,nullptr,&tNormStore,nullptr,&eValidationPath) ) << sError;
	EXPECT_EQ ( eValidationPath, e1::OpenValidationPath_e::DEEP );
	e1::Cursor tCursor;
	tCursor.Bind ( tStore, 1 );
	// A checksum or trusted-generation token is not a validation proof.
	EXPECT_FALSE ( tCursor.HasPublicIdMinBounds() );

	VectorPublicIDReader_c tPublicIDs ( dPublicIDs );
	// The current format always deep-validates. Missing or mismatched proof
	// must reject every corrupt input.
	auto dBadPostings = dRaw;
	dBadPostings.back() ^= 1;
	e1::Store tBadPostingsStore;
	EXPECT_FALSE ( tBadPostingsStore.Open(dBadPostings.data(),dBadPostings.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,true,nullptr,&tNormStore,nullptr,&eValidationPath) );
	EXPECT_EQ ( eValidationPath, e1::OpenValidationPath_e::DEEP );
	EXPECT_NE ( sError.find("checksums"), std::string::npos );

	auto dBadDict = dDict;
	dBadDict[0] ^= 1;
	e1::Store tBadDictStore;
	EXPECT_FALSE ( tBadDictStore.Open(dRaw.data(),dRaw.size(),130,dBadDict.data(),dBadDict.size(),dHits.data(),dHits.size(),sError,true,nullptr,&tNormStore,&tPublicIDs,&eValidationPath,false) );
	EXPECT_EQ ( eValidationPath, e1::OpenValidationPath_e::DEEP );
	EXPECT_NE ( sError.find("checksums"), std::string::npos );

	auto dBadHits = dHits;
	dBadHits[0] ^= 1;
	e1::Store tBadHitsStore;
	EXPECT_FALSE ( tBadHitsStore.Open(dRaw.data(),dRaw.size(),130,dDict.data(),dDict.size(),dBadHits.data(),dBadHits.size(),sError,true,nullptr,&tNormStore,nullptr,&eValidationPath) );
	EXPECT_EQ ( eValidationPath, e1::OpenValidationPath_e::DEEP );
	EXPECT_NE ( sError.find("checksums"), std::string::npos );

	e1::Store tValidatedStore;
	ASSERT_TRUE ( tValidatedStore.Open(dRaw.data(),dRaw.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,true,nullptr,&tNormStore,&tPublicIDs,&eValidationPath) ) << sError;
	EXPECT_EQ ( eValidationPath, e1::OpenValidationPath_e::DEEP );
	tCursor.Bind ( tValidatedStore, 1 );
	ASSERT_TRUE ( tCursor.HasPublicIdMinBounds() );
	for ( uint32_t uBlock=0; uBlock<3; ++uBlock )
	{
		uint64_t uActual = 0;
		ASSERT_TRUE ( tCursor.PublicIdMinBound(uBlock,uActual) );
		EXPECT_EQ ( uActual, dExpectedMin[uBlock] );
	}
	// A short authoritative binding is not a proof and must leave equality
	// pruning unavailable while retaining the safe score-bound path.
	std::vector<uint64_t> dShortIDs ( dPublicIDs.begin(), dPublicIDs.end()-1 );
	VectorPublicIDReader_c tShortIDs ( dShortIDs );
	e1::Store tShortStore;
	ASSERT_TRUE ( tShortStore.Open(dRaw.data(),dRaw.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,true,nullptr,&tNormStore,&tShortIDs,&eValidationPath) ) << sError;
	EXPECT_EQ ( eValidationPath, e1::OpenValidationPath_e::DEEP );
	e1::Cursor tShortCursor;
	tShortCursor.Bind ( tShortStore, 1 );
	EXPECT_TRUE ( tShortCursor.HasBM25ARatioBounds() );
	EXPECT_FALSE ( tShortCursor.HasPublicIdMinBounds() );

	// The generic comparator is strict on id: min<worst admits, while
	// min==worst cannot beat ORDER BY weight DESC,id ASC.
	uint8_t uCode = 0;
	ASSERT_TRUE ( tCursor.BM25ARatioBound(0,uCode) );
	const float fIDF = 0.1f;
	const int iThreshold = E1SafeUpperRatioWeight ( uCode, fIDF );
	uint32_t uRow=0,uTF=0,uMask=0;
	uint64_t uRef=0,uEntries=0,uBuckets=0,uSelected=0,uSkipped=0,uSkippedDocs=0,uDecoded=0,uEqualitySkipped=0;
	EXPECT_TRUE ( tCursor.NextRanked(uRow,uTF,uMask,uRef,uEntries,uBuckets,uSelected,uSkipped,uSkippedDocs,uDecoded,nullptr,0,nullptr,dExpectedMin[0]+1,fIDF,iThreshold,&uEqualitySkipped) );
	EXPECT_EQ ( uRow, 0u );
	tCursor.Reset();
	uEntries=uBuckets=uSelected=uSkipped=uSkippedDocs=uDecoded=uEqualitySkipped=0;
	EXPECT_FALSE ( tCursor.NextRanked(uRow,uTF,uMask,uRef,uEntries,uBuckets,uSelected,uSkipped,uSkippedDocs,uDecoded,nullptr,0,nullptr,dExpectedMin[2],fIDF,iThreshold,&uEqualitySkipped) );
	EXPECT_EQ ( uEqualitySkipped, 3u );
	EXPECT_EQ ( uSkippedDocs, 130u );

	// A row/public-ID binding mutation must fail even with intact container
	// checksums; otherwise a valid minimum could be applied to the wrong rows.
	auto dWrongRows = dPublicIDs;
	dWrongRows[0] = 1;
	VectorPublicIDReader_c tWrongRows ( dWrongRows );
	e1::Store tWrongRowStore;
	EXPECT_FALSE ( tWrongRowStore.Open(dRaw.data(),dRaw.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,true,nullptr,&tNormStore,&tWrongRows) );
	EXPECT_NE ( sError.find("unsafe public ID minimum"), std::string::npos );

	// Checksum-valid inflation is unsafe metadata, not a benchmarkable mode.
	auto dInflated = dRaw;
	const auto * pTerm = dInflated.data()+56;
	const uint32_t uMetaCount = e1::U32(pTerm+8);
	const auto * pLastMeta = dInflated.data()+e1::U64(pTerm+16)+uint64_t(uMetaCount-1)*16;
	const size_t uBoundsOffset = e1::U64(pLastMeta)+e1::Meta4Bytes(e1::U32(pLastMeta+8),e1::U32(pLastMeta+12));
	const size_t uMinIDsOffset = uBoundsOffset+dExpectedMin.size();
	PutValue ( dInflated, uMinIDsOffset, dExpectedMin[0]+1, 8 );
	UpdateChecksum ( dInflated );
	e1::Store tInflatedStore;
	EXPECT_FALSE ( tInflatedStore.Open(dInflated.data(),dInflated.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,false,nullptr,&tNormStore,&tPublicIDs) );
	EXPECT_NE ( sError.find("unsafe public ID minimum"), std::string::npos );

	// Reopening the same Store must recompute capability from this open's
	// bindings. Neither a successful prior proof nor a failed changed binding
	// may leave equality pruning enabled.
	ASSERT_TRUE ( tValidatedStore.PublicIDMinValidated() );
	ASSERT_TRUE ( tValidatedStore.Open(dRaw.data(),dRaw.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,true,nullptr,&tNormStore) ) << sError;
	EXPECT_FALSE ( tValidatedStore.PublicIDMinValidated() );
	e1::Cursor tReopenedCursor;
	tReopenedCursor.Bind ( tValidatedStore, 1 );
	EXPECT_FALSE ( tReopenedCursor.HasPublicIdMinBounds() );
	EXPECT_FALSE ( tValidatedStore.Open(dRaw.data(),dRaw.size(),130,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,true,nullptr,&tNormStore,&tWrongRows) );
	EXPECT_FALSE ( tValidatedStore.PublicIDMinValidated() );
}

TEST_F ( PostingsContainerTest, CurrentDuplicatePublicIDsValidateConservatively )
{
	std::vector<e1::Posting> dPostings;
	std::vector<uint32_t> dTotalDL ( 65, 8 );
	std::vector<uint64_t> dPublicIDs;
	for ( uint32_t i=0; i<65; ++i ) dPublicIDs.push_back ( i/2 ); // monotonic with duplicates
	for ( uint32_t i=0; i<65; ++i ) dPostings.push_back ( { i, 1, 1, 0, 0, dPublicIDs[i] } );
	{ std::ofstream(m_sBase+".spi",std::ios::binary).write("dict",4); }
	{ std::ofstream(m_sBase+".spp",std::ios::binary).put('\1'); }
	std::string sError;
	e1::Writer tWriter;
	ASSERT_TRUE ( tWriter.Open(m_sBase+".spd",sError) ) << sError;
	tWriter.BindTotalDL ( dTotalDL.data(), uint32_t(dTotalDL.size()), true );
	ASSERT_TRUE ( tWriter.FinishTerm(1,dPostings,dPostings.size(),false,sError) ) << sError;
	ASSERT_TRUE ( tWriter.Finalize(m_sBase+".spi",m_sBase+".spp",sError) ) << sError;
	auto dRaw=ReadFile(m_sBase+".spd"), dDict=ReadFile(m_sBase+".spi"), dHits=ReadFile(m_sBase+".spp");
	e1::norms::Builder tNormBuilder ( 1 );
	for ( uint32_t uDL : dTotalDL ) ASSERT_TRUE ( tNormBuilder.AddRow(&uDL,1,sError) ) << sError;
	std::vector<uint8_t> dNormBytes;
	ASSERT_TRUE ( tNormBuilder.Build(dNormBytes,sError) ) << sError;
	e1::norms::Store tNormStore;
	ASSERT_TRUE ( tNormStore.Open(dNormBytes.data(),dNormBytes.size(),sError) ) << sError;
	VectorPublicIDReader_c tPublicIDs ( dPublicIDs );
	e1::Store tStore;
	ASSERT_TRUE ( tStore.Open(dRaw.data(),dRaw.size(),65,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,false,nullptr,&tNormStore,&tPublicIDs) ) << sError;
	e1::Cursor tCursor;
	tCursor.Bind ( tStore, 1 );
	uint64_t uMin = 0;
	ASSERT_TRUE ( tCursor.PublicIdMinBound(0,uMin) );
	EXPECT_EQ ( uMin, 0u );
}

TEST_F ( PostingsContainerTest, V10EmbeddedFieldProjectionExactAndFailsClosed )
{
	static constexpr uint32_t ROWS = 8192;
	static constexpr uint32_t TOTAL_ROWS = ROWS*2;
	std::vector<e1::Posting> dPostings; dPostings.reserve(ROWS);
	std::vector<uint32_t> dTotalDL(TOTAL_ROWS,7);
	std::vector<uint32_t> dExpectedRows, dExpectedTF;
	for ( uint32_t i=0; i<ROWS; ++i )
	{
		uint32_t uMask=0,uTF=0,uFirst=0;
		if ( i<2048 ) { uMask=1;uTF=3;dExpectedRows.push_back(i*2);dExpectedTF.push_back(3); }
		else if ( i<4096 ) { uMask=3;uTF=5;uFirst=2;dExpectedRows.push_back(i*2);dExpectedTF.push_back(2); }
		else { uMask=2;uTF=4; }
		dPostings.push_back({i*2,uTF,uMask,0,uFirst});
	}
	{ std::ofstream(m_sBase+".spi",std::ios::binary).write("dict",4); }
	{ std::ofstream(m_sBase+".spp",std::ios::binary).put('\1'); }
	std::string sError; e1::Writer tWriter; ASSERT_TRUE(tWriter.Open(m_sBase+".spd",sError))<<sError;
	tWriter.BindTotalDL(dTotalDL.data(),ROWS,true);tWriter.BindIndexedFields(2);
	uint64_t uHits=0;for(const auto&t:dPostings)uHits+=t.m_uTF;
	ASSERT_TRUE(tWriter.FinishTerm(1,dPostings,uHits,false,sError))<<sError;ASSERT_TRUE(tWriter.Finalize(m_sBase+".spi",m_sBase+".spp",sError))<<sError;
	auto dRaw=ReadFile(m_sBase+".spd"),dDict=ReadFile(m_sBase+".spi"),dHits=ReadFile(m_sBase+".spp");
	e1::norms::Builder tNormBuilder(2);const uint32_t dNormRow[2]={3,4};for(uint32_t i=0;i<TOTAL_ROWS;++i)ASSERT_TRUE(tNormBuilder.AddRow(dNormRow,2,sError));std::vector<uint8_t>dNormBytes;ASSERT_TRUE(tNormBuilder.Build(dNormBytes,sError));e1::norms::Store tNorms;ASSERT_TRUE(tNorms.Open(dNormBytes.data(),dNormBytes.size(),sError));
	e1::Store tStore;ASSERT_TRUE(tStore.Open(dRaw.data(),dRaw.size(),TOTAL_ROWS,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,false,nullptr,&tNorms))<<sError;
	e1::Cursor tCursor;tCursor.Bind(tStore,1);ASSERT_TRUE(tCursor.SelectFieldProjection(0));EXPECT_EQ(tCursor.FieldProjectionRows(),4096u);EXPECT_FALSE(tCursor.SelectFieldProjection(1));ASSERT_TRUE(tCursor.SelectFieldProjection(0));
	std::vector<std::pair<uint32_t,uint32_t>> dActual;uint32_t uRow=0,uTF=0;uint64_t e=0,b=0,sel=0,skip=0,skipDocs=0;while(tCursor.NextFieldProjectionRanked(uRow,uTF,1.0f,0,e,b,sel,skip,skipDocs))dActual.emplace_back(uRow,uTF);std::sort(dActual.begin(),dActual.end());ASSERT_EQ(dActual.size(),dExpectedRows.size());for(size_t i=0;i<dActual.size();++i){EXPECT_EQ(dActual[i].first,dExpectedRows[i]);EXPECT_EQ(dActual[i].second,dExpectedTF[i]);}
	EXPECT_EQ(e,64u);EXPECT_EQ(sel,64u);EXPECT_EQ(skip,0u);
	const uint8_t*term=dRaw.data()+56;const uint32_t nm=e1::U32(term+8);const uint8_t*g=dRaw.data()+e1::U64(term+16)+uint64_t(nm-1)*16;const uint64_t aggBlocks=(ROWS+63)/64;const uint32_t fieldWidth=(e1::U32(term+12)>>8)&63;const uint64_t tail=e1::U64(g)+e1::Meta4Bytes(e1::U32(g+8),e1::U32(g+12))+aggBlocks+aggBlocks*8+(uint64_t(ROWS)*fieldWidth+7)/8;ASSERT_EQ(e1::U32(dRaw.data()+tail),1u);const uint64_t payload=e1::U64(dRaw.data()+tail+4+16);
	auto fnRejected = [&] ( ByteVec_t dBad, const char * szError ) { UpdateChecksum(dBad);e1::Store tRejected;EXPECT_FALSE(tRejected.Open(dBad.data(),dBad.size(),TOTAL_ROWS,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,false,nullptr,&tNorms));EXPECT_NE(sError.find(szError),std::string::npos)<<sError; };
	auto dUnder=dRaw;ASSERT_GT(dUnder[payload+13],0u);--dUnder[payload+13];fnRejected(std::move(dUnder),"unsafe field projection bound");
	auto dMalformed=dRaw;PutValue(dMalformed,tail+4+12,63,4);fnRejected(std::move(dMalformed),"field projection identity");
	const uint64_t rowData=payload+e1::U32(dRaw.data()+payload);const uint32_t rowBytes=e1::U32(dRaw.data()+payload+4);
	auto dNonMember=dRaw;ASSERT_EQ(dNonMember[rowData+1],2u);dNonMember[rowData+1]=3;fnRejected(std::move(dNonMember),"field projection non-member row");
	auto dAbsent=dRaw;PutValue(dAbsent,tail+4,1,4);fnRejected(std::move(dAbsent),"field projection field membership");
	auto dWrongTF=dRaw;ASSERT_EQ(dWrongTF[rowData+rowBytes],3u);dWrongTF[rowData+rowBytes]=4;fnRejected(std::move(dWrongTF),"field projection local TF");
	// A claimed projection may not reinterpret an unsupported canonical local-TF
	// contract as if the canonical member were absent.
	auto dUnsupported=dRaw;const uint8_t*unsupportedGroup=dUnsupported.data()+e1::U64(term+16)+uint64_t(2048/128)*16;ASSERT_EQ((e1::U32(unsupportedGroup+8)>>4)&3u,1u);ASSERT_EQ((e1::U32(unsupportedGroup+8)>>6)&3u,1u);PutValue(dUnsupported,e1::U64(unsupportedGroup)+4,7,4);const uint64_t uFieldTF=tail-(uint64_t(ROWS)*fieldWidth+7)/8;for(uint32_t ordinal=2048;ordinal<2176;++ordinal){const uint64_t uFieldBit=uint64_t(ordinal)*fieldWidth;for(uint32_t bit=0;bit<fieldWidth;++bit)dUnsupported[uFieldTF+(uFieldBit+bit)/8]&=uint8_t(~(1u<<((uFieldBit+bit)%8)));}fnRejected(std::move(dUnsupported),"field projection unsupported mask");
	// Rebuild a structurally valid, CRC-consistent projection after omitting a
	// non-tail member. This keeps all descriptor counts, offsets, block tails,
	// and bounds self-consistent, so only exact-set validation can reject it.
	auto dOmitted=dRaw;
	std::vector<uint32_t> dOmittedRows=dExpectedRows,dOmittedTF=dExpectedTF;
	dOmittedRows.erase(dOmittedRows.begin()+1);dOmittedTF.erase(dOmittedTF.begin()+1);
	const uint32_t uOmittedCount=uint32_t(dOmittedRows.size()),uOmittedBlocks=(uOmittedCount+63)/64;
	ByteVec_t dProjection(uint64_t(uOmittedBlocks)*16);
	auto fnAppendVar = [] ( ByteVec_t & dOut, uint32_t uValue ) { while(uValue>=128){dOut.push_back(uint8_t(uValue)|0x80);uValue>>=7;}dOut.push_back(uint8_t(uValue)); };
	for(uint32_t uBlock=0,uAt=0;uBlock<uOmittedBlocks;++uBlock)
	{
		const uint32_t uCount=std::min(64u,uOmittedCount-uAt);ByteVec_t dRows,dTF;uint32_t uPrevious=0;uint8_t uBound=0;
		for(uint32_t i=0;i<uCount;++i){const uint32_t uRow=dOmittedRows[uAt+i];fnAppendVar(dRows,i?uRow-uPrevious:uRow);uPrevious=uRow;fnAppendVar(dTF,dOmittedTF[uAt+i]);uBound=std::max(uBound,E1EncodeBM25A12_075_256(dOmittedTF[uAt+i],7));}
		const uint32_t uDataOffset=uint32_t(dProjection.size()),uDescriptor=uBlock*16;PutValue(dProjection,uDescriptor,uDataOffset,4);PutValue(dProjection,uDescriptor+4,dRows.size(),4);PutValue(dProjection,uDescriptor+8,dTF.size(),4);dProjection[uDescriptor+12]=uint8_t(uCount);dProjection[uDescriptor+13]=uBound;dProjection.insert(dProjection.end(),dRows.begin(),dRows.end());dProjection.insert(dProjection.end(),dTF.begin(),dTF.end());uAt+=uCount;
	}
	const uint64_t uOldDirectory=e1::U64(dOmitted.data()+48);dOmitted.erase(dOmitted.begin()+payload,dOmitted.begin()+uOldDirectory);dOmitted.insert(dOmitted.begin()+payload,dProjection.begin(),dProjection.end());PutValue(dOmitted,tail+4+4,uOmittedCount,4);PutValue(dOmitted,tail+4+8,uOmittedBlocks,4);PutValue(dOmitted,48,payload+dProjection.size(),8);fnRejected(std::move(dOmitted),"field projection incomplete");
	e1::OpenValidationPath_e ePath=e1::OpenValidationPath_e::NONE;ASSERT_TRUE(tStore.Open(dRaw.data(),dRaw.size(),TOTAL_ROWS,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,true,nullptr,&tNorms,nullptr,&ePath));EXPECT_EQ(ePath,e1::OpenValidationPath_e::DEEP);
	EXPECT_FALSE(tStore.Open(dRaw.data(),dRaw.size()-1,TOTAL_ROWS,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,false,nullptr,&tNorms));
}

TEST_F ( PostingsContainerTest, V10FieldProjectionRejectsCRCConsistentMemberOmission )
{
	static constexpr uint32_t ROWS=8192,TOTAL_ROWS=ROWS*2;
	std::vector<e1::Posting>dPostings;dPostings.reserve(ROWS);std::vector<uint32_t>dTotalDL(TOTAL_ROWS,7),dRows,dTF;dRows.reserve(4096);dTF.reserve(4096);
	for(uint32_t i=0;i<ROWS;++i){uint32_t mask=2,tf=4,first=0;if(i<2048){mask=1;tf=3;dRows.push_back(i*2);dTF.push_back(3);}else if(i<4096){mask=3;tf=5;first=2;dRows.push_back(i*2);dTF.push_back(2);}dPostings.push_back({i*2,tf,mask,0,first});}
	{std::ofstream(m_sBase+".spi",std::ios::binary).write("dict",4);}{std::ofstream(m_sBase+".spp",std::ios::binary).put('\1');}
	std::string sError;e1::Writer tWriter;ASSERT_TRUE(tWriter.Open(m_sBase+".spd",sError));tWriter.BindTotalDL(dTotalDL.data(),ROWS,true);tWriter.BindIndexedFields(2);uint64_t hits=0;for(const auto&p:dPostings)hits+=p.m_uTF;ASSERT_TRUE(tWriter.FinishTerm(1,dPostings,hits,false,sError));ASSERT_TRUE(tWriter.Finalize(m_sBase+".spi",m_sBase+".spp",sError));
	auto dRaw=ReadFile(m_sBase+".spd"),dDict=ReadFile(m_sBase+".spi"),dHits=ReadFile(m_sBase+".spp");e1::norms::Builder tNormBuilder(2);const uint32_t dNorm[2]={3,4};for(uint32_t i=0;i<TOTAL_ROWS;++i)ASSERT_TRUE(tNormBuilder.AddRow(dNorm,2,sError));std::vector<uint8_t>dNormBytes;ASSERT_TRUE(tNormBuilder.Build(dNormBytes,sError));e1::norms::Store tNorms;ASSERT_TRUE(tNorms.Open(dNormBytes.data(),dNormBytes.size(),sError));
	const uint8_t*term=dRaw.data()+56;const uint32_t nm=e1::U32(term+8);const uint8_t*g=dRaw.data()+e1::U64(term+16)+uint64_t(nm-1)*16;const uint64_t blocks=(ROWS+63)/64;const uint32_t width=(e1::U32(term+12)>>8)&63;const uint64_t tail=e1::U64(g)+e1::Meta4Bytes(e1::U32(g+8),e1::U32(g+12))+blocks+blocks*8+(uint64_t(ROWS)*width+7)/8,payload=e1::U64(dRaw.data()+tail+4+16);
	dRows.erase(dRows.begin()+1);dTF.erase(dTF.begin()+1);const uint32_t count=uint32_t(dRows.size()),projectionBlocks=(count+63)/64;ByteVec_t projection(uint64_t(projectionBlocks)*16);auto appendVar=[](ByteVec_t&out,uint32_t value){while(value>=128){out.push_back(uint8_t(value)|0x80);value>>=7;}out.push_back(uint8_t(value));};
	for(uint32_t block=0,at=0;block<projectionBlocks;++block){const uint32_t n=std::min(64u,count-at);ByteVec_t rows,tf;uint32_t previous=0;uint8_t bound=0;for(uint32_t i=0;i<n;++i){appendVar(rows,i?dRows[at+i]-previous:dRows[at+i]);previous=dRows[at+i];appendVar(tf,dTF[at+i]);bound=std::max(bound,E1EncodeBM25A12_075_256(dTF[at+i],7));}const uint32_t dataOffset=uint32_t(projection.size()),descriptor=block*16;PutValue(projection,descriptor,dataOffset,4);PutValue(projection,descriptor+4,rows.size(),4);PutValue(projection,descriptor+8,tf.size(),4);projection[descriptor+12]=uint8_t(n);projection[descriptor+13]=bound;projection.insert(projection.end(),rows.begin(),rows.end());projection.insert(projection.end(),tf.begin(),tf.end());at+=n;}
	const uint64_t oldDirectory=e1::U64(dRaw.data()+48);dRaw.erase(dRaw.begin()+payload,dRaw.begin()+oldDirectory);dRaw.insert(dRaw.begin()+payload,projection.begin(),projection.end());PutValue(dRaw,tail+8,count,4);PutValue(dRaw,tail+12,projectionBlocks,4);PutValue(dRaw,48,payload+projection.size(),8);UpdateChecksum(dRaw);
	e1::Store tRejected;EXPECT_FALSE(tRejected.Open(dRaw.data(),dRaw.size(),TOTAL_ROWS,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,false,nullptr,&tNorms));EXPECT_NE(sError.find("field projection incomplete"),std::string::npos)<<sError;
}

TEST_F ( PostingsContainerTest, V10ProjectionBudgetRejectsIncrementally )
{
	static constexpr uint32_t ROWS=8192;
	std::vector<e1::Posting>dPostings;dPostings.reserve(ROWS);std::vector<uint32_t>dTotalDL(ROWS,7);
	for(uint32_t i=0;i<ROWS;++i)dPostings.push_back({i,i<4096?0x10000000u:1u,i<4096?1u:2u,0,0});
	{std::ofstream(m_sBase+".spi",std::ios::binary).write("dict",4);}{std::ofstream(m_sBase+".spp",std::ios::binary).put('\1');}
	std::string sError;e1::Writer tWriter;ASSERT_TRUE(tWriter.Open(m_sBase+".spd",sError));tWriter.BindTotalDL(dTotalDL.data(),ROWS,true);tWriter.BindIndexedFields(2);uint64_t uHits=0;for(const auto&t:dPostings)uHits+=t.m_uTF;ASSERT_TRUE(tWriter.FinishTerm(1,dPostings,uHits,false,sError));ASSERT_TRUE(tWriter.Finalize(m_sBase+".spi",m_sBase+".spp",sError));
	auto dRaw=ReadFile(m_sBase+".spd"),dDict=ReadFile(m_sBase+".spi"),dHits=ReadFile(m_sBase+".spp");ASSERT_EQ(std::string(reinterpret_cast<const char*>(dRaw.data()),8),"E1POST10");EXPECT_EQ(e1::U32(dRaw.data()+8),10u);EXPECT_EQ(e1::VERSION,e1::VERSION10);
	e1::norms::Builder tNormBuilder(2);const uint32_t dNorm[2]={3,4};for(uint32_t i=0;i<ROWS;++i)ASSERT_TRUE(tNormBuilder.AddRow(dNorm,2,sError));std::vector<uint8_t>dNormBytes;ASSERT_TRUE(tNormBuilder.Build(dNormBytes,sError));e1::norms::Store tNorms;ASSERT_TRUE(tNorms.Open(dNormBytes.data(),dNormBytes.size(),sError));e1::Store tStore;ASSERT_TRUE(tStore.Open(dRaw.data(),dRaw.size(),ROWS,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError,false,nullptr,&tNorms))<<sError;e1::Cursor tCursor;tCursor.Bind(tStore,1);EXPECT_FALSE(tCursor.SelectFieldProjection(0));EXPECT_TRUE(tCursor.SelectFieldProjection(1));
}

TEST ( NormStore, ExactWidthsRangesGatherAndTotals )
{
	e1::norms::Builder tBuilder ( 3, 4 );
	std::string sError;
	std::array<std::array<uint32_t,3>,128> dRows {};
	for ( uint32_t i=0; i<dRows.size(); ++i )
	{
		dRows[i] = { i, i+1, i+7 };
		if ( i==0 ) dRows[i][1] = 255;
		if ( i==1 ) dRows[i][1] = 256;
		if ( i==0 ) dRows[i][2] = 65535;
		if ( i==1 ) dRows[i][2] = 65536;
		ASSERT_TRUE ( tBuilder.AddRow(dRows[i].data(),dRows[i].size(),sError) ) << sError;
	}
	std::vector<uint8_t> dData;
	ASSERT_TRUE ( tBuilder.Build(dData,sError) ) << sError;

	e1::norms::Store tStore;
	ASSERT_TRUE ( tStore.Open(dData.data(),dData.size(),sError) ) << sError;
	EXPECT_EQ ( tStore.Rows(), 128u );
	EXPECT_EQ ( tStore.Fields(), 3u );
	EXPECT_GT ( tStore.TotalCacheBytes(), 0u );
	EXPECT_LE ( tStore.TotalCacheBytes(), 512u );
	EXPECT_EQ ( tStore.Nonzero(0), 127u );
	EXPECT_EQ ( tStore.Sum(0), 8128u );

	uint32_t uValue = 0;
	ASSERT_TRUE ( tStore.Get(1,1,uValue) );
	EXPECT_EQ ( uValue, 256u );
	ASSERT_TRUE ( tStore.Get(2,1,uValue) );
	EXPECT_EQ ( uValue, 65536u );

	std::array<uint32_t,128> dIDs {};
	std::array<uint32_t,128> dValues {};
	for ( uint32_t i=0; i<128; ++i ) dIDs[i] = i;
	ASSERT_TRUE ( tStore.Gather128(0,dIDs.data(),dValues.data()) );
	for ( uint32_t i=0; i<128; ++i ) EXPECT_EQ ( dValues[i], i );
	ASSERT_TRUE ( tStore.GatherTotal(dIDs.data(),dIDs.size(),dValues.data()) );
	for ( uint32_t i=0; i<128; ++i ) EXPECT_EQ ( dValues[i], dRows[i][0]+dRows[i][1]+dRows[i][2] );
	ASSERT_TRUE ( tStore.ReadRange(2,1,7,dValues.data()) );
	for ( uint32_t i=0; i<7; ++i ) EXPECT_EQ ( dValues[i], dRows[i+1][2] );
}

TEST ( NormStore, RejectsCorruptionAndTruncation )
{
	e1::norms::Builder tBuilder ( 1, 4 );
	std::string sError;
	for ( uint32_t uValue : { 1u, 255u, 256u, 65536u, 7u } )
		ASSERT_TRUE ( tBuilder.AddRow(&uValue,1,sError) ) << sError;
	std::vector<uint8_t> dData;
	ASSERT_TRUE ( tBuilder.Build(dData,sError) ) << sError;

	e1::norms::Store tStore;
	auto dCorrupt = dData;
	dCorrupt.back() ^= 1;
	EXPECT_FALSE ( tStore.Open(dCorrupt.data(),dCorrupt.size(),sError) );
	EXPECT_FALSE ( tStore.Open(dData.data(),dData.size()-1,sError) );

	dCorrupt = dData;
	dCorrupt.back() ^= 1;
	PutValue ( dCorrupt, 56, e1::CRC(dCorrupt.data()+e1::norms::HEADER_SIZE,dCorrupt.size()-e1::norms::HEADER_SIZE), 4 );
	EXPECT_FALSE ( tStore.Open(dCorrupt.data(),dCorrupt.size(),sError) );
	EXPECT_EQ ( sError, "norms: group metadata mismatch" );
}

TEST ( NormStore, StagedBuilderMatchesInMemoryBuilderAndSupportsUpdates )
{
	std::string sError;
	e1::norms::Builder tMemory ( 3, 4 );
	e1::norms::StagedBuilder tStaged ( 3, 4, true );
	for ( uint32_t i=0; i<11; ++i )
	{
		const std::array<uint32_t,3> dRow { i, i==3 ? 256u : i+20, i==7 ? 65536u : i+200 };
		ASSERT_TRUE ( tMemory.AddRow(dRow.data(),dRow.size(),sError) ) << sError;
		ASSERT_TRUE ( tStaged.AddRow(dRow.data(),dRow.size(),sError) ) << sError;
	}
	ASSERT_TRUE ( tMemory.Set(2,1,70000,sError) ) << sError;
	ASSERT_TRUE ( tStaged.Set(2,1,70000,sError) ) << sError;
	ASSERT_TRUE ( tMemory.Set(10,2,90000,sError) ) << sError;
	ASSERT_TRUE ( tStaged.Set(10,2,90000,sError) ) << sError;
	ASSERT_TRUE ( tStaged.HasAuthoritativeTotalDL() );
	ASSERT_EQ ( tStaged.Rows(), 11u );
	ASSERT_NE ( tStaged.TotalDLData(), nullptr );
	EXPECT_EQ ( tStaged.TotalDLData()[2], 70204u );
	EXPECT_EQ ( tStaged.TotalDLData()[10], 90040u );

	std::vector<uint8_t> dExpected;
	ASSERT_TRUE ( tMemory.Build(dExpected,sError) ) << sError;
	const std::string sPath = "__norm_store_"+std::to_string(GetOsProcessId())+".tmp";
	ASSERT_TRUE ( tStaged.Finish(sPath.c_str(),sError) ) << sError;
	std::ifstream tIn ( sPath, std::ios::binary );
	std::vector<uint8_t> dActual ( (std::istreambuf_iterator<char>(tIn)), std::istreambuf_iterator<char>() );
	std::remove ( sPath.c_str() );
	EXPECT_EQ ( dActual, dExpected );
}

TEST ( NormStore, PackedStagedBuilderMatchesInMemoryBuilder )
{
	std::string sError;
	e1::norms::Builder tMemory ( 3, 4 );
	e1::norms::StagedBuilder tPacked ( 3, 4 );
	for ( uint32_t i=0; i<11; ++i )
	{
		const std::array<uint32_t,3> dRow { i, i==3 ? 256u : i+20, i==7 ? 65536u : i+200 };
		ASSERT_TRUE ( tMemory.AddRow(dRow.data(),dRow.size(),sError) ) << sError;
		ASSERT_TRUE ( tPacked.AddRow(dRow.data(),dRow.size(),sError) ) << sError;
	}
	std::vector<uint8_t> dExpected;
	ASSERT_TRUE ( tMemory.Build(dExpected,sError) ) << sError;
	const std::string sPath = "__packed_norm_store_"+std::to_string(GetOsProcessId())+".tmp";
	ASSERT_TRUE ( tPacked.Finish(sPath.c_str(),sError) ) << sError;
	std::ifstream tIn ( sPath, std::ios::binary );
	std::vector<uint8_t> dActual ( (std::istreambuf_iterator<char>(tIn)), std::istreambuf_iterator<char>() );
	std::remove ( sPath.c_str() );
	EXPECT_EQ ( dActual, dExpected );
}

TEST ( NormStore, DirectBuilderMatchesStagedBuilderAtGroupBoundaries )
{
	struct SourceRow_t
	{
		std::array<uint32_t,3> m_dValues {};
		bool m_bLive = false;
	};

	for ( uint32_t uRows : { 0u, 1u, 4095u, 4096u, 4097u } )
	{
		std::array<std::vector<SourceRow_t>,3> dSegments;
		for ( uint32_t uRow=0; uRow<uRows; ++uRow )
		{
			auto & dSegment = dSegments[std::min ( 2u, uint32_t(uint64_t(uRow)*3/std::max(1u,uRows)) )];
			if ( !(uRow%997) )
				dSegment.push_back ( { { UINT32_MAX, UINT32_MAX, UINT32_MAX }, false } );
			dSegment.push_back ( { { (uRow*17)%251, 256+(uRow%60000), 65536+uRow }, true } );
		}

		std::string sError;
		e1::norms::StagedBuilder tStaged ( 3 );
		for ( const auto & dSegment : dSegments )
			for ( const auto & tRow : dSegment )
				if ( tRow.m_bLive )
					ASSERT_TRUE ( tStaged.AddRow(tRow.m_dValues.data(),tRow.m_dValues.size(),sError) ) << sError;

		const std::string sSuffix = std::to_string(GetOsProcessId())+"_"+std::to_string(uRows)+".tmp";
		const std::string sStagedPath = "__staged_norm_store_"+sSuffix;
		const std::string sDirectPath = "__direct_norm_store_"+sSuffix;
		ASSERT_TRUE ( tStaged.Finish(sStagedPath.c_str(),sError) ) << sError;

		e1::norms::DirectBuilder tDirect ( 3 );
		auto fnValues = [&] ( uint32_t uField, const auto & fnValue )
		{
			for ( const auto & dSegment : dSegments )
				for ( const auto & tRow : dSegment )
					if ( tRow.m_bLive && !fnValue(tRow.m_dValues[uField]) )
						return false;
			return true;
		};
		ASSERT_TRUE ( tDirect.Finish(sDirectPath.c_str(),uRows,fnValues,sError) ) << sError;

		std::ifstream tStagedIn ( sStagedPath, std::ios::binary );
		std::ifstream tDirectIn ( sDirectPath, std::ios::binary );
		std::vector<uint8_t> dStaged ( (std::istreambuf_iterator<char>(tStagedIn)), std::istreambuf_iterator<char>() );
		std::vector<uint8_t> dDirect ( (std::istreambuf_iterator<char>(tDirectIn)), std::istreambuf_iterator<char>() );
		std::remove ( sStagedPath.c_str() );
		std::remove ( sDirectPath.c_str() );
		EXPECT_EQ ( dDirect, dStaged ) << "rows=" << uRows;
	}
}

TEST ( NormStore, StagedBuilderReportsOutputPathAndSystemError )
{
	std::string sError;
	e1::norms::StagedBuilder tStaged ( 1, 4 );
	const uint32_t uValue = 7;
	ASSERT_TRUE ( tStaged.AddRow(&uValue,1,sError) ) << sError;
	const char * szPath = "__missing_norm_store_dir__/products.spn";
	EXPECT_FALSE ( tStaged.Finish(szPath,sError) );
	EXPECT_NE ( sError.find(szPath), std::string::npos ) << sError;
	EXPECT_NE ( sError.find(": "), std::string::npos ) << sError;

	e1::norms::DirectBuilder tDirect ( 1, 4 );
	auto fnValues = [&] ( uint32_t, const auto & fnValue ) { return fnValue(uValue); };
	EXPECT_FALSE ( tDirect.Finish(szPath,1,fnValues,sError) );
	EXPECT_NE ( sError.find(szPath), std::string::npos ) << sError;
	EXPECT_NE ( sError.find(": "), std::string::npos ) << sError;
}

TEST ( NormStore, StagedBuilderSupportsAttributeOnlyIndexes )
{
	std::string sError;
	e1::norms::StagedBuilder tStaged ( 0, 4 );
	const uint32_t * pNoFields = nullptr;
	ASSERT_TRUE ( tStaged.AddRow(pNoFields,0,sError) ) << sError;
	ASSERT_TRUE ( tStaged.AddRow(pNoFields,0,sError) ) << sError;

	const std::string sPath = "__empty_norm_store_"+std::to_string(GetOsProcessId())+".tmp";
	ASSERT_TRUE ( tStaged.Finish(sPath.c_str(),sError) ) << sError;
	std::ifstream tIn ( sPath, std::ios::binary );
	std::vector<uint8_t> dData ( (std::istreambuf_iterator<char>(tIn)), std::istreambuf_iterator<char>() );
	std::remove ( sPath.c_str() );
	e1::norms::DirectBuilder tDirect ( 0, 4 );
	const std::string sDirectPath = "__empty_direct_norm_store_"+std::to_string(GetOsProcessId())+".tmp";
	auto fnValues = [] ( uint32_t, const auto & ) { return false; };
	ASSERT_TRUE ( tDirect.Finish(sDirectPath.c_str(),2,fnValues,sError) ) << sError;
	std::ifstream tDirectIn ( sDirectPath, std::ios::binary );
	std::vector<uint8_t> dDirectData ( (std::istreambuf_iterator<char>(tDirectIn)), std::istreambuf_iterator<char>() );
	std::remove ( sDirectPath.c_str() );
	EXPECT_EQ ( dDirectData, dData );

	e1::norms::Store tStore;
	ASSERT_TRUE ( tStore.Open(dData.data(),dData.size(),sError) ) << sError;
	EXPECT_EQ ( tStore.Rows(), 2u );
	EXPECT_EQ ( tStore.Fields(), 0u );
	EXPECT_EQ ( tStore.TotalCacheBytes(), 2u );
}

TEST ( RtFieldNorms, MapsMixedIndexedAndStoredFieldsAndExpandsLegacyRows )
{
	CSphSchema tSchema;
	auto fnAddField = [&] ( const char * szName, DWORD uFlags )
	{
		CSphColumnInfo tField ( szName );
		tField.m_uFieldFlags = uFlags;
		tSchema.AddField ( tField );
	};
	fnAddField ( "stored0", CSphColumnInfo::FIELD_STORED );
	fnAddField ( "indexed0", CSphColumnInfo::FIELD_INDEXED );
	fnAddField ( "both", CSphColumnInfo::FIELD_INDEXED | CSphColumnInfo::FIELD_STORED );
	fnAddField ( "stored1", CSphColumnInfo::FIELD_STORED );

	RtFieldNorms_c tNorms ( tSchema );
	EXPECT_EQ ( tNorms.Fields(), 4 );
	EXPECT_EQ ( tNorms.DenseFields(), 2 );
	EXPECT_EQ ( tNorms.DenseIndex(0), -1 );
	EXPECT_EQ ( tNorms.DenseIndex(1), 0 );
	EXPECT_EQ ( tNorms.DenseIndex(2), 1 );
	EXPECT_EQ ( tNorms.DenseIndex(3), -1 );
	EXPECT_EQ ( tNorms.SchemaField(0), 1 );
	EXPECT_EQ ( tNorms.SchemaField(1), 2 );

	const DWORD dSchemaRow[] = { 0, 17, 29, 0 };
	DWORD dDenseRow[] = { 0, 0 };
	tNorms.Compact ( dSchemaRow, dDenseRow );
	EXPECT_EQ ( dDenseRow[0], 17u );
	EXPECT_EQ ( dDenseRow[1], 29u );
	EXPECT_EQ ( tNorms.Get(dDenseRow,0), 0u );
	EXPECT_EQ ( tNorms.Get(dDenseRow,1), 17u );
	EXPECT_EQ ( tNorms.Get(dDenseRow,2), 29u );
	EXPECT_EQ ( tNorms.Get(dDenseRow,3), 0u );

	DWORD dExpanded[] = { 99, 99, 99, 99 };
	tNorms.Expand ( dDenseRow, dExpanded );
	EXPECT_EQ ( std::vector<DWORD>(dExpanded,dExpanded+4), ( std::vector<DWORD>{ 0, 17, 29, 0 } ) );
}

TEST ( RtFieldNorms, SupportsZeroIndexedFieldsWithoutRowPointers )
{
	CSphSchema tSchema;
	CSphColumnInfo tStored ( "stored" );
	tStored.m_uFieldFlags = CSphColumnInfo::FIELD_STORED;
	tSchema.AddField ( tStored );

	RtFieldNorms_c tNorms ( tSchema );
	EXPECT_EQ ( tNorms.Fields(), 1 );
	EXPECT_EQ ( tNorms.DenseFields(), 0 );
	EXPECT_EQ ( tNorms.DenseIndex(0), -1 );
	EXPECT_EQ ( tNorms.SchemaField(0), -1 );
	EXPECT_EQ ( tNorms.Get(nullptr,0), 0u );
	DWORD uExpanded = 99;
	tNorms.Compact ( &uExpanded, nullptr );
	tNorms.Expand ( nullptr, &uExpanded );
	EXPECT_EQ ( uExpanded, 0u );
}

TEST ( RtFieldNorms, SchemaWideCompatibilityWriterBoundsAreAllocationFree )
{
	CSphString sError;
	EXPECT_TRUE ( ValidateRtSchemaWideNorms ( 6, 3, 4, 2, sError ) ) << sError.cstr();
	EXPECT_FALSE ( ValidateRtSchemaWideNorms ( 5, 3, 4, 2, sError ) );
	EXPECT_NE ( strstr ( sError.cstr(), "dense norms count mismatch" ), nullptr );
	EXPECT_FALSE ( ValidateRtSchemaWideNorms ( 0, std::numeric_limits<DWORD>::max(), 2, 0, sError ) );
	EXPECT_NE ( strstr ( sError.cstr(), "count overflow" ), nullptr );
	const DWORD uReaderLimitRows = DWORD(std::numeric_limits<int>::max()/sizeof(DWORD))+1;
	EXPECT_FALSE ( ValidateRtSchemaWideNorms ( 0, uReaderLimitRows, 1, 0, sError ) );
	EXPECT_NE ( strstr ( sError.cstr(), "reader limit" ), nullptr );
}

TEST ( NormStore, DenseMixedFieldsExpandToByteIdenticalSchemaWideSpn )
{
	CSphSchema tSchema;
	for ( const auto & tField : std::array<std::pair<const char *,DWORD>,4> {{
		{ "stored0", CSphColumnInfo::FIELD_STORED },
		{ "indexed0", CSphColumnInfo::FIELD_INDEXED },
		{ "stored1", CSphColumnInfo::FIELD_STORED },
		{ "indexed1", CSphColumnInfo::FIELD_INDEXED | CSphColumnInfo::FIELD_STORED }
	}} )
	{
		CSphColumnInfo tCol ( tField.first );
		tCol.m_uFieldFlags = tField.second;
		tSchema.AddField ( tCol );
	}
	RtFieldNorms_c tNorms ( tSchema );
	const std::array<std::array<DWORD,2>,3> dDense {{ { 3, 7 }, { 11, 13 }, { 17, 19 } }};

	std::string sError;
	e1::norms::StagedBuilder tSchemaWide ( 4 );
	for ( const auto & dDenseRow : dDense )
	{
		DWORD dExpanded[4];
		tNorms.Expand ( dDenseRow.data(), dExpanded );
		ASSERT_TRUE ( tSchemaWide.AddRow(dExpanded,4,sError) ) << sError;
	}
	const std::string sSuffix = std::to_string(GetOsProcessId())+".tmp";
	const std::string sExpectedPath = "__schema_norm_store_"+sSuffix;
	const std::string sDensePath = "__dense_norm_store_"+sSuffix;
	ASSERT_TRUE ( tSchemaWide.Finish(sExpectedPath.c_str(),sError) ) << sError;

	e1::norms::DirectBuilder tDirect ( 4 );
	auto fnValues = [&] ( uint32_t uSchemaField, const auto & fnValue )
	{
		for ( const auto & dDenseRow : dDense )
			if ( !fnValue(tNorms.Get(dDenseRow.data(),uSchemaField)) )
				return false;
		return true;
	};
	ASSERT_TRUE ( tDirect.Finish(sDensePath.c_str(),dDense.size(),fnValues,sError) ) << sError;

	std::ifstream tExpectedIn ( sExpectedPath, std::ios::binary );
	std::ifstream tDenseIn ( sDensePath, std::ios::binary );
	std::vector<uint8_t> dExpected ( (std::istreambuf_iterator<char>(tExpectedIn)), std::istreambuf_iterator<char>() );
	std::vector<uint8_t> dActual ( (std::istreambuf_iterator<char>(tDenseIn)), std::istreambuf_iterator<char>() );
	std::remove ( sExpectedPath.c_str() );
	std::remove ( sDensePath.c_str() );
	EXPECT_EQ ( dActual, dExpected );
}

} // namespace
