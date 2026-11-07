// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// Distributed under the GNU General Public License, version 3 or later.

#include <gtest/gtest.h>
#include "docidlookupcache.h"

#include <algorithm>
#include <atomic>
#include <random>
#include <thread>
#include <vector>

class NativeDocidCache : public ::testing::Test
{
	void SetUp() override
	{
		ASSERT_EQ ( DocidLookupCache_c::GetUsed(), 0U );
		DocidLookupCache_c::SetBudget ( 64*1024*1024 );
	}
	void TearDown() override
	{
		EXPECT_EQ ( DocidLookupCache_c::GetUsed(), 0U );
		DocidLookupCache_c::SetBudget(0);
	}
};

TEST_F ( NativeDocidCache, DisabledEmptyAndOversizedDoNotAllocate )
{
	EXPECT_FALSE ( DocidLookupCache_c::Create(0) );
	EXPECT_FALSE ( DocidLookupCache_c::Create(UINT32_MAX) );
	DocidLookupCache_c::SetBudget(0);
	EXPECT_FALSE ( DocidLookupCache_c::Create(1) );
	EXPECT_EQ ( DocidLookupCache_c::GetUsed(), 0U );
}

TEST_F ( NativeDocidCache, LeafAndDirectoryBoundaries )
{
	for ( uint32_t nDocs : { 1, 15, 16, 63, 64, 65, 511, 512, 513, 1023, 1024, 1025, 4095, 4096, 4097, 16384, 16385 } )
	{
		auto pCache = DocidLookupCache_c::Create(nDocs);
		ASSERT_TRUE(pCache);
		for ( uint32_t i=0; i<nDocs; ++i )
			ASSERT_TRUE ( pCache->Append ( uint64_t(i)*2, i ) );
		ASSERT_TRUE ( pCache->Seal() );
		for ( uint32_t i=0; i<nDocs; ++i )
		{
			ASSERT_EQ ( pCache->Find(uint64_t(i)*2), i );
			ASSERT_EQ ( pCache->Find(uint64_t(i)*2+1), UINT32_MAX );
		}
	}
}

TEST_F ( NativeDocidCache, UnsignedExtremesAndRowZero )
{
	auto pCache = DocidLookupCache_c::Create(4);
	ASSERT_TRUE(pCache);
	ASSERT_TRUE ( pCache->Append ( 0, 0 ) );
	ASSERT_TRUE ( pCache->Append ( 9, 7 ) );
	ASSERT_TRUE ( pCache->Append ( uint64_t(1)<<63, 3 ) );
	ASSERT_TRUE ( pCache->Append ( UINT64_MAX, 5 ) );
	ASSERT_TRUE ( pCache->Seal() );
	EXPECT_EQ ( pCache->Find(0), 0U );
	EXPECT_EQ ( pCache->Find(9), 7U );
	EXPECT_EQ ( pCache->Find(uint64_t(1)<<63), 3U );
	EXPECT_EQ ( pCache->Find(UINT64_MAX), 5U );
	EXPECT_EQ ( pCache->Find(UINT64_MAX-1), UINT32_MAX );
	EXPECT_FALSE ( pCache->Append ( 10, 10 ) );
}

TEST_F ( NativeDocidCache, IncompleteDuplicateAndUnsortedInputAreRefused )
{
	auto pCache = DocidLookupCache_c::Create(3);
	ASSERT_TRUE(pCache);
	ASSERT_TRUE ( pCache->Append ( 4, 1 ) );
	EXPECT_FALSE ( pCache->Append ( 4, 9 ) );
	EXPECT_FALSE ( pCache->Append ( 3, 9 ) );
	EXPECT_FALSE ( pCache->Seal() );
	EXPECT_EQ ( pCache->Find(4), UINT32_MAX );
}

TEST_F ( NativeDocidCache, MaxKeyInPartialLastLeaf )
{
	for ( uint32_t nDocs : { 65, 1025, 16385 } )
	{
		auto pCache = DocidLookupCache_c::Create(nDocs);
		ASSERT_TRUE(pCache);
		for ( uint32_t i=0; i<nDocs-1; ++i )
			ASSERT_TRUE ( pCache->Append ( i*2, i ) );
		ASSERT_TRUE ( pCache->Append ( UINT64_MAX, nDocs-1 ) );
		ASSERT_TRUE ( pCache->Seal() );
		EXPECT_EQ ( pCache->Find(UINT64_MAX), nDocs-1 );
		EXPECT_EQ ( pCache->Find(UINT64_MAX-1), UINT32_MAX );
		EXPECT_EQ ( pCache->Find(0), 0U );
	}
}

TEST_F ( NativeDocidCache, BudgetChargesEntireAllocationAndReclaimsIt )
{
	auto pCache = DocidLookupCache_c::Create(10000);
	ASSERT_TRUE(pCache);
	size_t nBytes = pCache->GetLengthBytes();
	EXPECT_EQ ( DocidLookupCache_c::GetUsed(), nBytes );
	DocidLookupCache_c::SetBudget(nBytes);
	EXPECT_FALSE ( DocidLookupCache_c::Create(1) );
	pCache.reset();
	EXPECT_EQ ( DocidLookupCache_c::GetUsed(), 0U );
	DocidLookupCache_c::SetBudget(nBytes-1);
	EXPECT_FALSE ( DocidLookupCache_c::Create(10000) );
	DocidLookupCache_c::SetBudget(nBytes);
	pCache = DocidLookupCache_c::Create(10000);
	EXPECT_TRUE(pCache);
}

TEST_F ( NativeDocidCache, RuntimeAlignmentAndPaddingRemainBudgeted )
{
	const size_t nAlignment = DocidLookupCache_c::GetAlignment();
	ASSERT_TRUE ( nAlignment==64 || nAlignment==128 || nAlignment==256 );
	for ( uint32_t nDocs : { 1, 65, 1025, 16385 } )
	{
		auto pCache = DocidLookupCache_c::Create(nDocs);
		ASSERT_TRUE(pCache);
		EXPECT_EQ ( reinterpret_cast<uintptr_t>(pCache.get())%nAlignment, 0U );
		const size_t nBytes = pCache->GetLengthBytes();
		EXPECT_EQ ( DocidLookupCache_c::GetUsed(), nBytes );
		for ( uint32_t i=0; i<nDocs; ++i ) ASSERT_TRUE ( pCache->Append ( uint64_t(i)*2, i ) );
		ASSERT_TRUE ( pCache->Seal() );
		EXPECT_EQ ( pCache->Find(uint64_t(nDocs-1)*2), nDocs-1 );
		pCache.reset();
		DocidLookupCache_c::SetBudget(nBytes-1);
		EXPECT_FALSE ( DocidLookupCache_c::Create(nDocs) );
		DocidLookupCache_c::SetBudget(nBytes);
		pCache = DocidLookupCache_c::Create(nDocs);
		EXPECT_TRUE(pCache);
		pCache.reset();
		DocidLookupCache_c::SetBudget(64*1024*1024);
	}
}

TEST_F ( NativeDocidCache, RandomKeysMatchSortedReference )
{
	std::mt19937_64 tRandom(42);
	std::vector<uint64_t> dKeys;
	for ( int i=0; i<100000; ++i )
		dKeys.push_back(tRandom() & ~uint64_t(1));
	std::sort ( dKeys.begin(), dKeys.end() );
	auto pCache = DocidLookupCache_c::Create((uint32_t)dKeys.size());
	ASSERT_TRUE(pCache);
	for ( uint32_t i=0; i<dKeys.size(); ++i )
		ASSERT_TRUE ( pCache->Append ( dKeys[i], i ) );
	ASSERT_TRUE ( pCache->Seal() );
	for ( int i=0; i<100000; ++i )
	{
		uint32_t uRow = tRandom()%dKeys.size();
		ASSERT_EQ ( pCache->Find(dKeys[uRow]), uRow );
		ASSERT_EQ ( pCache->Find(dKeys[uRow]+1), UINT32_MAX );
	}
	EXPECT_LT ( pCache->GetLengthBytes(), dKeys.size()*13 );
}

TEST_F ( NativeDocidCache, ConcurrentReadersAndBudgetReservations )
{
	auto pCache = DocidLookupCache_c::Create(1025);
	ASSERT_TRUE(pCache);
	for ( uint32_t i=0; i<1025; ++i )
		ASSERT_TRUE ( pCache->Append ( i*2, i ) );
	ASSERT_TRUE ( pCache->Seal() );
	const size_t nBudget = pCache->GetLengthBytes()*3;
	DocidLookupCache_c::SetBudget(nBudget);
	std::vector<std::thread> dThreads;
	for ( int i=0; i<8; ++i )
		dThreads.emplace_back ( [&] {
			for ( uint32_t j=0; j<1025; ++j )
			{
				EXPECT_EQ ( pCache->Find(j*2), j );
				auto pOther = DocidLookupCache_c::Create(1025);
				EXPECT_LE ( DocidLookupCache_c::GetUsed(), nBudget );
			}
		} );
	for ( auto & tThread : dThreads )
		tThread.join();
	EXPECT_EQ ( DocidLookupCache_c::GetUsed(), pCache->GetLengthBytes() );
}

TEST_F ( NativeDocidCache, BatchMatchesReferenceWithTilesPaddingAndUnsignedExtremes )
{
	for ( uint32_t nDocs : { 1, 64, 65, 1024, 1025, 16385 } )
	{
		auto pCache = DocidLookupCache_c::Create(nDocs);
		ASSERT_TRUE(pCache);
		std::vector<std::pair<uint64_t,uint32_t>> dPairs;
		for ( uint32_t i=0; i<nDocs; ++i )
		{
			uint64_t uID = i==nDocs-1 ? UINT64_MAX : uint64_t(i)*2;
			dPairs.emplace_back ( uID, nDocs-i-1 );
			ASSERT_TRUE ( pCache->Append ( uID, nDocs-i-1 ) );
		}
		ASSERT_TRUE ( pCache->Seal() );
		pCache->FindBatch ( nullptr, nullptr, 0 );
		std::mt19937_64 tRandom(42);
		for ( size_t nQueries : { 1, 15, 16, 17, 31, 32, 33, 63, 64, 65, 1025 } )
		{
			std::vector<uint64_t> dQueries;
			for ( size_t i=0; i<nQueries; ++i )
			{
				uint64_t uID = dPairs[tRandom()%nDocs].first;
				if ( i%5==0 ) uID = UINT64_MAX;
				if ( i%5==1 ) uID = UINT64_MAX-1;
				if ( i%5==2 ) uID = uint64_t(1)<<63;
				if ( i%5==3 ) uID |= 1;
				dQueries.push_back(uID);
			}
			std::vector<uint32_t> dRows(nQueries+2,42);
			pCache->FindBatch ( dQueries.data(), dRows.data()+1, nQueries );
			EXPECT_EQ ( dRows.front(), 42U );
			EXPECT_EQ ( dRows.back(), 42U );
			for ( size_t i=0; i<nQueries; ++i )
			{
				auto tFound = std::lower_bound ( dPairs.begin(), dPairs.end(), std::make_pair(dQueries[i],uint32_t(0)) );
				uint32_t uExpected = tFound!=dPairs.end() && tFound->first==dQueries[i] ? tFound->second : UINT32_MAX;
				ASSERT_EQ ( dRows[i+1], uExpected );
			}
		}
	}
}

TEST_F ( NativeDocidCache, BatchUnsealedAndConcurrentReadsDoNotChangeBudget )
{
	auto pCache = DocidLookupCache_c::Create(1025);
	ASSERT_TRUE(pCache);
	uint64_t dIDs[33];
	uint32_t dRows[33];
	for ( int i=0; i<33; ++i ) dIDs[i] = i;
	pCache->FindBatch ( dIDs, dRows, 33 );
	for ( uint32_t uRow : dRows ) EXPECT_EQ ( uRow, UINT32_MAX );
	for ( uint32_t i=0; i<1025; ++i ) ASSERT_TRUE ( pCache->Append ( i*2, i ) );
	ASSERT_TRUE ( pCache->Seal() );
	const size_t nUsed = DocidLookupCache_c::GetUsed();
	DocidLookupCache_c::SetBudget(nUsed);
	std::vector<std::thread> dThreads;
	for ( int i=0; i<4; ++i )
		dThreads.emplace_back ( [&] {
			uint32_t dOutput[33];
			for ( int j=0; j<100; ++j )
			{
				pCache->FindBatch ( dIDs, dOutput, 33 );
				for ( int k=0; k<33; ++k ) EXPECT_EQ ( dOutput[k], k%2 ? UINT32_MAX : uint32_t(k/2) );
			}
		} );
	for ( auto & tThread : dThreads ) tThread.join();
	EXPECT_EQ ( DocidLookupCache_c::GetUsed(), nUsed );
}

TEST_F ( NativeDocidCache, BatchPreservesWideLeafIndicesAndOutputPositions )
{
	// Cross a 16-bit leaf index, and output positions beyond a byte, while
	// mixing inactive lanes, in-range misses, full tiles and a scalar tail.
	constexpr uint32_t nDocs = (uint32_t(1)<<16)*64+1;
	auto pCache = DocidLookupCache_c::Create(nDocs);
	ASSERT_TRUE(pCache);
	for ( uint32_t i=0; i<nDocs; ++i )
		if ( !pCache->Append ( i==nDocs-1 ? UINT64_MAX : uint64_t(i)*2+4, nDocs-1-i ) )
			FAIL() << "append at " << i;
	ASSERT_TRUE ( pCache->Seal() );
	const uint32_t dBoundaryRows[] = { 0, 63, 64, (65535U*64)-1, 65535U*64, nDocs-2, nDocs-1 };
	for ( size_t nQueries : { 16, 17, 257, 529 } )
	{
		std::vector<uint64_t> dIDs(nQueries);
		std::vector<uint32_t> dExpected(nQueries), dRows(nQueries+2,0xabcdef);
		for ( size_t i=0; i<nQueries; ++i )
		{
			uint32_t uIndex = dBoundaryRows[i%7];
			uint64_t uID = uIndex==nDocs-1 ? UINT64_MAX : uint64_t(uIndex)*2+4;
			dExpected[i] = nDocs-1-uIndex;
			if ( i%5==0 )
			{
				uID = 0;
				dExpected[i] = UINT32_MAX;
			} else if ( i%5==1 )
			{
				uID = uID==UINT64_MAX ? UINT64_MAX-1 : uID+1;
				dExpected[i] = UINT32_MAX;
			}
			dIDs[i] = uID;
		}
		pCache->FindBatch ( dIDs.data(), dRows.data()+1, nQueries );
		EXPECT_EQ ( dRows.front(), 0xabcdefU );
		EXPECT_EQ ( dRows.back(), 0xabcdefU );
		for ( size_t i=0; i<nQueries; ++i ) ASSERT_EQ ( dRows[i+1], dExpected[i] ) << i;
	}
}

TEST_F ( NativeDocidCache, ConsecutiveBatchesHandleDifferentCachesAndActiveLanes )
{
	auto pLarge = DocidLookupCache_c::Create(16385);
	auto pSmall = DocidLookupCache_c::Create(65);
	ASSERT_TRUE(pLarge);
	ASSERT_TRUE(pSmall);
	for ( uint32_t i=0; i<16385; ++i ) ASSERT_TRUE ( pLarge->Append ( i*2+4, i+7 ) );
	for ( uint32_t i=0; i<65; ++i ) ASSERT_TRUE ( pSmall->Append ( i*2+4, 64-i ) );
	ASSERT_TRUE ( pLarge->Seal() );
	ASSERT_TRUE ( pSmall->Seal() );
	uint64_t dIDs[33];
	uint32_t dRows[35];
	for ( int repeat=0; repeat<40; ++repeat )
		for ( const auto * pCache : { pLarge.get(), pSmall.get() } )
			for ( size_t nQueries : { 33, 32, 17, 16, 15 } )
			{
				for ( size_t i=0; i<nQueries; ++i )
					dIDs[i] = repeat%3==0 ? UINT64_MAX : (i%3 ? uint64_t(i*4+4+repeat%2) : 0);
				std::fill ( std::begin(dRows), std::end(dRows), 42 );
				pCache->FindBatch ( dIDs, dRows+1, nQueries );
				EXPECT_EQ ( dRows[0], 42U );
				EXPECT_EQ ( dRows[nQueries+1], 42U );
				for ( size_t i=0; i<nQueries; ++i ) EXPECT_EQ ( dRows[i+1], pCache->Find(dIDs[i]) );
			}
}

TEST_F ( NativeDocidCache, ConcurrentBatchesIsolateThreadsCachesAndQueries )
{
	auto pFirst = DocidLookupCache_c::Create(16385);
	auto pSecond = DocidLookupCache_c::Create(1025);
	ASSERT_TRUE(pFirst);
	ASSERT_TRUE(pSecond);
	for ( uint32_t i=0; i<16385; ++i ) ASSERT_TRUE ( pFirst->Append ( uint64_t(i)*2+4, 16384-i ) );
	for ( uint32_t i=0; i<1025; ++i ) ASSERT_TRUE ( pSecond->Append ( uint64_t(i)*2+4, i+99 ) );
	ASSERT_TRUE ( pFirst->Seal() );
	ASSERT_TRUE ( pSecond->Seal() );
	const size_t nUsed = DocidLookupCache_c::GetUsed();
	std::atomic<int> iReady {0};
	std::atomic<bool> bGo {false};
	std::vector<std::thread> dThreads;
	for ( uint32_t iThread=0; iThread<8; ++iThread )
		dThreads.emplace_back ( [&,iThread] {
			std::mt19937_64 tRandom(iThread+42);
			uint64_t dIDs[129];
			uint32_t dRows[129];
			iReady.fetch_add ( 1, std::memory_order_release );
			while ( !bGo.load(std::memory_order_acquire) ) std::this_thread::yield();
			for ( int repeat=0; repeat<200; ++repeat )
			{
				const auto * pCache = repeat%2 ? pFirst.get() : pSecond.get();
				const size_t nQueries = repeat%3 ? 129 : 33;
				for ( size_t i=0; i<nQueries; ++i ) dIDs[i] = tRandom()%40000;
				pCache->FindBatch ( dIDs, dRows, nQueries );
				for ( size_t i=0; i<nQueries; ++i ) EXPECT_EQ ( dRows[i], pCache->Find(dIDs[i]) );
			}
		} );
	while ( iReady.load(std::memory_order_acquire)!=8 ) std::this_thread::yield();
	bGo.store ( true, std::memory_order_release );
	for ( auto & tThread : dThreads ) tThread.join();
	EXPECT_EQ ( DocidLookupCache_c::GetUsed(), nUsed );
}
