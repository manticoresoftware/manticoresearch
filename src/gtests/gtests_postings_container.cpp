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
#include "threadutils.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{

using ByteVec_t = std::vector<uint8_t>;

void PutValue ( ByteVec_t & dData, size_t uOffset, uint64_t uValue, unsigned uBytes )
{
	for ( unsigned i=0; i<uBytes; ++i )
		dData[uOffset+i] = uint8_t ( uValue >> ( 8*i ) );
}

void AppendValue ( ByteVec_t & dData, uint64_t uValue, unsigned uBytes )
{
	const size_t uOffset = dData.size();
	dData.resize ( uOffset+uBytes );
	PutValue ( dData, uOffset, uValue, uBytes );
}

unsigned BitWidth ( uint64_t uValue )
{
	unsigned uWidth = 0;
	while ( uValue )
	{
		++uWidth;
		uValue >>= 1;
	}
	return uWidth;
}

void AppendPacked ( ByteVec_t & dData, unsigned uWidth, const std::vector<uint64_t> & dValues )
{
	const size_t uOffset = dData.size();
	dData.resize ( uOffset+( uint64_t(dValues.size())*uWidth+7 )/8 );
	for ( size_t i=0; i<dValues.size(); ++i )
		for ( unsigned j=0; j<uWidth; ++j )
			if ( ( dValues[i] >> j ) & 1 )
				dData[uOffset+(i*uWidth+j)/8] |= uint8_t ( 1u << ( (i*uWidth+j)%8 ) );
}

void UpdateChecksum ( ByteVec_t & dData )
{
	PutValue ( dData, 16, dData.size(), 8 );
	const uint32_t uHeaderSize = e1::U32 ( dData.data()+12 );
	PutValue ( dData, 32, e1::CRC(dData.data()+uHeaderSize,dData.size()-uHeaderSize), 4 );
}

ByteVec_t MakeContainer ( const std::vector<uint32_t> & dRows, unsigned uCodec, unsigned uWidth=0 )
{
	ByteVec_t dData ( 128 );
	memcpy ( dData.data(), "E1POST06", 8 );
	PutValue ( dData, 8, 6, 4 );
	PutValue ( dData, 12, 48, 4 );
	PutValue ( dData, 24, 1, 8 );
	PutValue ( dData, 48, 1, 8 );
	PutValue ( dData, 56, 80, 8 );
	PutValue ( dData, 64, dRows.size(), 4 );
	PutValue ( dData, 68, 1, 4 );
	PutValue ( dData, 72, dRows.size(), 8 );
	PutValue ( dData, 80, dRows.size(), 4 );
	PutValue ( dData, 84, 1, 4 );
	PutValue ( dData, 88, 1, 4 );
	PutValue ( dData, 104, dRows.front(), 4 );
	PutValue ( dData, 108, dRows.back(), 4 );
	PutValue ( dData, 112, dRows.size(), 4 );
	PutValue ( dData, 116, uCodec | (uWidth<<16), 4 );
	PutValue ( dData, 120, dData.size(), 8 );

	if ( uCodec==2 )
		for ( size_t i=1; i<dRows.size(); ++i )
			AppendValue ( dData, dRows[i], 4 );
	else if ( uCodec==3 )
	{
		const size_t uOffset = dData.size();
		dData.resize ( uOffset+( (dRows.size()-1)*uWidth+7 )/8 );
		for ( size_t i=1; i<dRows.size(); ++i )
		{
			const uint32_t uGap = dRows[i]-dRows[i-1];
			for ( unsigned j=0; j<uWidth; ++j )
				if ( (uGap>>j)&1 )
					dData[uOffset+( (i-1)*uWidth+j )/8] |= uint8_t ( 1u << ( ((i-1)*uWidth+j)%8 ) );
		}
	}
	else if ( uCodec==4 )
	{
		const size_t uOffset = dData.size();
		dData.resize ( uOffset+( uint64_t(dRows.back())-dRows.front()+8 )/8 );
		for ( uint32_t uRow : dRows )
			dData[uOffset+(uRow-dRows.front())/8] |= uint8_t ( 1u << ( (uRow-dRows.front())%8 ) );
	}

	PutValue ( dData, 96, dData.size(), 8 );
	const size_t uMetadata = dData.size();
	dData.resize ( uMetadata+16 );
	PutValue ( dData, uMetadata, dData.size(), 8 );
	const unsigned uRowWidth = BitWidth ( dRows.size()-1 );
	PutValue ( dData, uMetadata+8, (uRowWidth<<16) | (1u<<22), 4 );
	PutValue ( dData, uMetadata+12, dRows.size(), 4 );
	AppendValue ( dData, 0x800001u, 8 );
	std::vector<uint64_t> dOrdinals;
	for ( size_t i=0; i<dRows.size(); ++i )
		dOrdinals.push_back ( i );
	AppendPacked ( dData, uRowWidth, dOrdinals );
	for ( size_t i=0; i<(dRows.size()+63)/64; ++i )
		AppendValue ( dData, 1, 1 );
	UpdateChecksum ( dData );
	return dData;
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
		for ( const char * szExt : { ".spd", ".spi", ".spp" } )
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

TEST ( PostingsContainer, CodecsSeekBulkAndMalformedInput )
{
	const std::vector<std::vector<uint32_t>> dCases { {0}, {8,9,10}, {5,7,12,14}, {100,101,103,104,105,107}, {0,0x80000000u,0xfffffffeu} };
	const unsigned dCodecs[] { 1, 1, 3, 4, 2 };

	for ( size_t iCase=0; iCase<dCases.size(); ++iCase )
	{
		const auto & dRows = dCases[iCase];
		auto dData = MakeContainer ( dRows, dCodecs[iCase], iCase==2 ? 3 : 0 );
		std::string sError;
		e1::Store tStore;
		ASSERT_TRUE ( tStore.Open(dData.data(),dData.size(),UINT32_MAX,nullptr,0,nullptr,0,sError) ) << sError;
		e1::Cursor tCursor;
		tCursor.Bind ( tStore, 1 );
		for ( size_t i=0; i<dRows.size(); ++i )
		{
			uint32_t uRow=0,uTF=0,uMask=0;
			uint64_t uRef=0;
			ASSERT_TRUE ( tCursor.Next(uRow,&uTF,&uMask,&uRef) );
			EXPECT_EQ ( uRow, dRows[i] );
			EXPECT_EQ ( uTF, 1u );
			EXPECT_EQ ( uMask, 1u );
			EXPECT_EQ ( uRef, (uint64_t(1)<<63) | uint64_t(0x800001u+i) );
		}
		uint32_t uRow = 0;
		EXPECT_FALSE ( tCursor.Next(uRow) );
		EXPECT_EQ ( uRow, UINT32_MAX );

		for ( uint32_t uTarget : dRows )
		{
			tCursor.Reset();
			tCursor.Hint ( uTarget );
			ASSERT_TRUE ( tCursor.Next(uRow) );
			EXPECT_EQ ( uRow, uTarget );
		}

		for ( unsigned uCapacity : { 1u,2u,127u,128u,129u } )
		{
			tCursor.Reset();
			std::vector<uint32_t> dActual;
			uint32_t dBuffer[129];
			for ( ;; )
			{
				const unsigned uRead = tCursor.NextRows ( dBuffer, uCapacity, uRow );
				dActual.insert ( dActual.end(), dBuffer, dBuffer+uRead );
				if ( uRead<uCapacity )
					break;
			}
			EXPECT_EQ ( dActual, dRows );
			EXPECT_EQ ( uRow, UINT32_MAX );
		}

		auto fnRejected = [&] ( auto fnCorrupt )
		{
			auto dBad = dData;
			fnCorrupt ( dBad );
			UpdateChecksum ( dBad );
			e1::Store tBad;
			return !tBad.Open ( dBad.data(), dBad.size(), UINT32_MAX, nullptr, 0, nullptr, 0, sError );
		};
		EXPECT_TRUE ( fnRejected ( [] ( ByteVec_t & dBad ) { PutValue ( dBad, 116, e1::U32(dBad.data()+116) | 0x80000000u, 4 ); } ) );
		EXPECT_TRUE ( fnRejected ( [] ( ByteVec_t & dBad ) { PutValue ( dBad, 96, UINT64_MAX, 8 ); } ) );
		EXPECT_TRUE ( fnRejected ( [] ( ByteVec_t & dBad ) { PutValue ( dBad, 108, UINT32_MAX, 4 ); } ) );
		EXPECT_TRUE ( fnRejected ( [] ( ByteVec_t & dBad ) { PutValue ( dBad, 72, 999, 8 ); } ) );
	}
}

TEST_F ( PostingsContainerTest, StreamedDirectoryUsesImplicitContiguousKeys )
{
	{ std::ofstream(m_sBase+".spi",std::ios::binary).write("dict",4); }
	{ std::ofstream(m_sBase+".spp",std::ios::binary).put('\1'); }
	const std::vector<e1::Posting> dFirst { { 1,1,1,0 } };
	const std::vector<e1::Posting> dSecond { { 2,2,1,0 } };
	std::string sError;
	e1::Writer tWriter;
	ASSERT_TRUE ( tWriter.Open(m_sBase+".spd",sError) ) << sError;
	ASSERT_TRUE ( tWriter.FinishTerm(1,dFirst,1,false,sError) ) << sError;
	EXPECT_FALSE ( tWriter.FinishTerm(3,dSecond,2,false,sError) );
	ASSERT_TRUE ( tWriter.FinishTerm(2,dSecond,2,false,sError) ) << sError;
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
}

TEST_F ( PostingsContainerTest, WriterReaderMetadataAndDirectWindows )
{
	std::vector<e1::Posting> dPostings;
	for ( uint32_t uRow=0; uRow<8192; ++uRow )
		if ( uRow%3!=1 )
			dPostings.push_back ( { uRow, uRow==4098 ? 300u : 1u+(uRow%7), 1, 0 } );
	WriteContainer ( dPostings );

	auto dRaw=ReadFile(m_sBase+".spd"), dDict=ReadFile(m_sBase+".spi"), dHits=ReadFile(m_sBase+".spp");
	std::string sError;
	e1::Store tStore;
	ASSERT_TRUE ( tStore.Open(dRaw.data(),dRaw.size(),8192,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError) ) << sError;
	e1::Cursor tCursor;
	tCursor.Bind ( tStore, 1 );
	ASSERT_TRUE ( tCursor.DirectOrSupported() );
	uint32_t uLastWindow = UINT32_MAX;
	ASSERT_TRUE ( tCursor.DirectLastWindow(uLastWindow) );
	EXPECT_EQ ( uLastWindow, 1u );

	for ( uint32_t uWindow=0; uWindow<=uLastWindow; ++uWindow )
	{
		std::array<uint64_t,64> dMask {};
		std::array<uint8_t,4096> dBounds {};
		uint32_t uCardinality=0,uMaxTF=0;
		uint64_t uBoundReads=0;
		ASSERT_TRUE ( tCursor.DirectWindow(uWindow,dMask.data(),dBounds.data(),uCardinality,uMaxTF,uBoundReads) );
		uint32_t uExpectedCardinality = 0;
		for ( uint32_t uOffset=0; uOffset<4096; ++uOffset )
		{
			const uint32_t uRow = uWindow*4096+uOffset;
			const bool bExpected = uRow%3!=1;
			EXPECT_EQ ( bool(dMask[uOffset/64] & (uint64_t(1)<<(uOffset%64))), bExpected );
			if ( bExpected )
			{
				++uExpectedCardinality;
				const uint32_t uTF = uRow==4098 ? 300u : 1u+(uRow%7);
				EXPECT_TRUE ( dBounds[uOffset]==255 || dBounds[uOffset]>=uTF );
			}
		}
		EXPECT_EQ ( uCardinality, uExpectedCardinality );
		EXPECT_GT ( uBoundReads, 0u );
	}

	uint32_t uTF = 0;
	EXPECT_TRUE ( tCursor.ProbeTF(4098,uTF) );
	EXPECT_EQ ( uTF, 300u );
	EXPECT_FALSE ( tCursor.ProbeTF(4096,uTF) );
	EXPECT_TRUE ( tCursor.ProbeTF(4101,uTF) );
	EXPECT_EQ ( uTF, 1u+(4101u%7) );

	tCursor.Bind ( tStore, 1 );
	size_t iExpected = 0;
	for ( uint32_t uTarget=0; uTarget<8192; uTarget+=17 )
	{
		while ( iExpected<dPostings.size() && dPostings[iExpected].m_uRow<uTarget )
			++iExpected;
		tCursor.Hint ( uTarget );
		if ( iExpected==dPostings.size() )
		{
			uint32_t uRow = 0;
			EXPECT_FALSE ( tCursor.Next(uRow) );
			break;
		}
		uint32_t uRow = UINT32_MAX;
		ASSERT_TRUE ( tCursor.Next(uRow) );
		EXPECT_EQ ( uRow, dPostings[iExpected].m_uRow );
		++iExpected;
	}
}

TEST_F ( PostingsContainerTest, FieldTermFrequencies )
{
	const std::vector<e1::Posting> dPostings {
		{ 1,3,1u,0,0 }, { 2,7,3u,0,2 }, { 3,9,6u,0,4 }, { 4,5,7u,0,0 }
	};
	WriteContainer ( dPostings );
	auto dRaw=ReadFile(m_sBase+".spd"), dDict=ReadFile(m_sBase+".spi"), dHits=ReadFile(m_sBase+".spp");
	std::string sError;
	e1::Store tStore;
	ASSERT_TRUE ( tStore.Open(dRaw.data(),dRaw.size(),16,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError) ) << sError;
	e1::Cursor tCursor;
	tCursor.Bind ( tStore, 1 );
	uint32_t uTF = 0;
	EXPECT_TRUE ( tCursor.ExactFieldTF(0,0,1u,3,uTF) ); EXPECT_EQ ( uTF, 3u );
	EXPECT_TRUE ( tCursor.ExactFieldTF(1,0,3u,7,uTF) ); EXPECT_EQ ( uTF, 2u );
	EXPECT_TRUE ( tCursor.ExactFieldTF(1,1,3u,7,uTF) ); EXPECT_EQ ( uTF, 5u );
	EXPECT_TRUE ( tCursor.ExactFieldTF(2,1,6u,9,uTF) ); EXPECT_EQ ( uTF, 4u );
	EXPECT_TRUE ( tCursor.ExactFieldTF(2,2,6u,9,uTF) ); EXPECT_EQ ( uTF, 5u );
	EXPECT_FALSE ( tCursor.ExactFieldTF(3,0,7u,5,uTF) );
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

TEST_F ( PostingsContainerTest, RankedBoundsAndPersistedVersions )
{
	std::vector<e1::Posting> dPostings;
	for ( uint32_t i=0; i<129; ++i )
		dPostings.push_back ( { i, i==70 ? 300u : (i==128 ? 2u : 1u), 1, 0 } );
	WriteContainer ( dPostings );
	auto dRaw=ReadFile(m_sBase+".spd"), dDict=ReadFile(m_sBase+".spi"), dHits=ReadFile(m_sBase+".spp");
	std::string sError;
	e1::Store tStore;
	ASSERT_TRUE ( tStore.Open(dRaw.data(),dRaw.size(),129,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError) ) << sError;
	e1::Cursor tCursor;
	tCursor.Bind ( tStore, 1 );
	EXPECT_TRUE ( tCursor.HasRankedBounds() );

	uint32_t uRow=0,uTF=0,uMask=0;
	uint64_t uRef=0,uEntries=0,uBuckets=0,uSelected=0,uSkipped=0,uSkippedDocs=0,uDecoded=0;
	ASSERT_TRUE ( tCursor.NextRanked(uRow,uTF,uMask,uRef,3,uEntries,uBuckets,uSelected,uSkipped,uSkippedDocs,uDecoded) );
	EXPECT_EQ ( uRow, 64u );
	while ( tCursor.NextRanked(uRow,uTF,uMask,uRef,3,uEntries,uBuckets,uSelected,uSkipped,uSkippedDocs,uDecoded) ) {}
	EXPECT_EQ ( uSkippedDocs, 65u );
	EXPECT_EQ ( uSkipped, 2u );
	EXPECT_EQ ( uDecoded, 1u );

	auto dV5 = dRaw;
	memcpy ( dV5.data(), "E1POST05", 8 );
	PutValue ( dV5, 8, 5, 4 );
	UpdateChecksum ( dV5 );
	ASSERT_TRUE ( tStore.Open(dV5.data(),dV5.size(),129,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError) ) << sError;
	tCursor.Bind ( tStore, 1 );
	EXPECT_TRUE ( tCursor.HasRankedBounds() );

	const uint64_t uDirectory = e1::U64 ( dRaw.data()+48 );
	const auto * pTerm = dRaw.data()+56;
	const uint32_t uMetadataCount = e1::U32 ( pTerm+8 );
	const uint64_t uMetadataOffset = e1::U64 ( pTerm+16 );
	const auto * pGroup = dRaw.data()+uMetadataOffset+uint64_t(uMetadataCount-1)*16;
	const size_t uBoundsOffset = e1::U64(pGroup)+e1::Meta4Bytes(e1::U32(pGroup+8),e1::U32(pGroup+12));
	const size_t uBoundsLength = (dPostings.size()+63)/64;
	auto dV4 = dRaw;
	dV4.erase ( dV4.begin()+uBoundsOffset, dV4.begin()+uBoundsOffset+uBoundsLength );
	memcpy ( dV4.data(), "E1POST04", 8 );
	PutValue ( dV4, 8, 4, 4 );
	PutValue ( dV4, 48, uDirectory-uBoundsLength, 8 );
	UpdateChecksum ( dV4 );
	ASSERT_TRUE ( tStore.Open(dV4.data(),dV4.size(),129,dDict.data(),dDict.size(),dHits.data(),dHits.size(),sError) ) << sError;
	tCursor.Bind ( tStore, 1 );
	EXPECT_FALSE ( tCursor.HasRankedBounds() );
	for ( uint32_t i=0; i<dPostings.size(); ++i )
	{
		ASSERT_TRUE ( tCursor.Next(uRow,&uTF,&uMask,&uRef) );
		EXPECT_EQ ( uRow, i );
		EXPECT_EQ ( uTF, dPostings[i].m_uTF );
	}
	EXPECT_FALSE ( tCursor.Next(uRow) );
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
	EXPECT_EQ ( tStore.TotalCacheBytes(), 512u );
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
	e1::norms::StagedBuilder tStaged ( 3, 4 );
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

	std::vector<uint8_t> dExpected;
	ASSERT_TRUE ( tMemory.Build(dExpected,sError) ) << sError;
	const std::string sPath = "__norm_store_"+std::to_string(GetOsProcessId())+".tmp";
	ASSERT_TRUE ( tStaged.Finish(sPath.c_str(),sError) ) << sError;
	std::ifstream tIn ( sPath, std::ios::binary );
	std::vector<uint8_t> dActual ( (std::istreambuf_iterator<char>(tIn)), std::istreambuf_iterator<char>() );
	std::remove ( sPath.c_str() );
	EXPECT_EQ ( dActual, dExpected );
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

	e1::norms::Store tStore;
	ASSERT_TRUE ( tStore.Open(dData.data(),dData.size(),sError) ) << sError;
	EXPECT_EQ ( tStore.Rows(), 2u );
	EXPECT_EQ ( tStore.Fields(), 0u );
	EXPECT_EQ ( tStore.TotalCacheBytes(), 2u );
}

} // namespace
