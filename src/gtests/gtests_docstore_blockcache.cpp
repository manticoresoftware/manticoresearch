// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// Distributed under the GNU General Public License, version 3 or later.

#include <gtest/gtest.h>
#include "docstore.h"
#include "indexsettings.h"
#include "threadutils.h"
#include "std/lrucache.h"
#include "docstore_blockpool.h"
#include "docstore_admission.h"

#include <atomic>
#include <string>
#include <thread>
#include <vector>

namespace
{
TEST ( DocstoreBlockPool, ReuseAndReturnExcessMappings )
{
	DocstoreBlockPool_c tPool;
	std::vector<BYTE *> dBlocks;
	for ( int i=0; i<32; ++i )
	{
		auto * p = tPool.Allocate(16381);
		ASSERT_NE ( p, nullptr );
		memset ( p, i, 16381 );
		dBlocks.push_back(p);
	}
	int64_t iMapped = 0, iSpare = 0;
	tPool.GetStats ( iMapped, iSpare );
	ASSERT_GE ( iMapped, 32*DocstoreBlockPool_c::AllocationSize(16381) );
	ASSERT_EQ ( iSpare, 0 );
	for ( auto * p : dBlocks ) tPool.Free ( p, 16381 );
	iMapped = iSpare = 0;
	tPool.GetStats ( iMapped, iSpare );
	ASSERT_LE ( iMapped, 64*1024 );
	ASSERT_EQ ( iMapped, iSpare );
	auto * p = tPool.Allocate(16381);
	ASSERT_NE ( p, nullptr );
	int64_t iMappedAfter = 0, iSpareAfter = 0;
	tPool.GetStats ( iMappedAfter, iSpareAfter );
	ASSERT_EQ ( iMappedAfter, iMapped );
	ASSERT_LT ( iSpareAfter, iSpare );
	tPool.Free ( p, 16381 );
}

TEST ( DocstoreBlockPool, LargeMappingAndNoSpareBudget )
{
	DocstoreBlockPool_c tPool(0);
	ASSERT_EQ ( tPool.Allocate(0), nullptr );
	ASSERT_EQ ( tPool.Allocate(UINT32_MAX), nullptr );
	auto * p = tPool.Allocate(70001);
	ASSERT_NE ( p, nullptr );
	memset ( p, 0x67, 70001 );
	tPool.Free ( p, 70001 );
	int64_t iMapped = 0, iSpare = 0;
	tPool.GetStats ( iMapped, iSpare );
	ASSERT_EQ ( iMapped, 0 );
	ASSERT_EQ ( iSpare, 0 );
}

TEST ( DocstoreBlockPool, PacksBlocksAbovePageBoundary )
{
	DocstoreBlockPool_c tPool;
	const DWORD uSize = 16385;
	ASSERT_GE ( DocstoreBlockPool_c::AllocationSize(uSize), uSize );
	ASSERT_LT ( DocstoreBlockPool_c::AllocationSize(uSize), 2*16384 );
	std::vector<BYTE *> dBlocks;
	for ( int i=0; i<30; ++i )
	{
		auto * p = tPool.Allocate(uSize);
		ASSERT_NE ( p, nullptr );
		memset ( p, i, uSize );
		dBlocks.push_back(p);
	}
	int64_t iMapped = 0, iSpare = 0;
	tPool.GetStats ( iMapped, iSpare );
	ASSERT_LT ( iMapped, 30*2*16384 );
	for ( int i=0; i<30; ++i )
	{
		EXPECT_EQ ( dBlocks[i][0], i );
		EXPECT_EQ ( dBlocks[i][uSize-1], i );
		tPool.Free ( dBlocks[i], uSize );
	}
}

TEST ( DocstoreBlockPool, ConcurrentAllocateAndRelease )
{
	DocstoreBlockPool_c tPool;
	std::atomic<int> iFailures {0};
	std::vector<std::thread> dThreads;
	for ( int i=0; i<8; ++i )
		dThreads.emplace_back ( [&,i] {
			for ( int j=0; j<2000; ++j )
			{
				const DWORD uSize = j%2 ? 16381 : 16637;
				auto * p = tPool.Allocate(uSize);
				if ( !p ) { ++iFailures; continue; }
				memset ( p, i, uSize );
				for ( DWORD k=0; k<uSize; ++k ) if ( p[k]!=i ) { ++iFailures; break; }
				tPool.Free ( p, uSize );
			}
		} );
	for ( auto & t : dThreads ) t.join();
	ASSERT_EQ ( iFailures.load(), 0 );
	int64_t iMapped = 0, iSpare = 0;
	tPool.GetStats ( iMapped, iSpare );
	ASSERT_EQ ( iMapped, iSpare );
	ASSERT_LE ( iMapped, 64*1024 );
}

struct CacheTestData_t
{
	BYTE * m_pData;
	DWORD m_uSize;
};

struct CacheTestUtil_t
{
	static DWORD GetHash ( int iKey ) { return DWORD(iKey)*2654435761U; }
	static bool Equal ( int a, int b ) { return a==b; }
	static DWORD GetSize ( const CacheTestData_t & tData ) { return tData.m_uSize; }
	static void Reset ( CacheTestData_t & tData ) { delete[] tData.m_pData; }
};

class AdaptiveLruTest_c : public LRUCache_T<int, CacheTestData_t, CacheTestUtil_t>
{
	using BASE = LRUCache_T<int, CacheTestData_t, CacheTestUtil_t>;
public:
	explicit AdaptiveLruTest_c ( bool bAdaptive, int64_t iLimit = 512*1024 )
		: BASE ( iLimit, iLimit, bAdaptive ? 16*1024 : 0 ) {}
	int64_t Used() { return GetUsedBytes(); }
	void Put ( int iKey )
	{
		CacheTestData_t tData;
		if ( Find ( iKey, tData ) )
		{
			Release(iKey);
			return;
		}
		tData = { new BYTE[1] { BYTE(iKey%251) }, 1024 };
		if ( Add ( iKey, tData ) )
			Release(iKey);
		else
			CacheTestUtil_t::Reset(tData);
	}
	void Hot ( int iReads = 30000 )
	{
		for ( int i=0; i<iReads; ++i ) Put(i%4);
	}
};

TEST ( AdaptiveLru, LazyGrowthAndHardLimit )
{
	AdaptiveLruTest_c tCache(true);
	ASSERT_EQ ( tCache.Used(), 0 );
	for ( int i=0; i<1000; ++i )
	{
		tCache.Put(i);
		ASSERT_LE ( tCache.Used(), 512*1024 );
	}
	ASSERT_GT ( tCache.Used(), 16*1024 );
}

TEST ( AdaptiveLru, HotSetChangeReclaimsColdBlocks )
{
	AdaptiveLruTest_c tCache(true);
	for ( int i=0; i<256; ++i ) tCache.Put(i);
	ASSERT_GT ( tCache.Used(), 128*1024 );
	tCache.Hot();
	ASSERT_LE ( tCache.Used(), 16*1024 );
	for ( int i=0; i<4; ++i )
	{
		CacheTestData_t tData;
		ASSERT_TRUE ( tCache.Find ( i, tData ) );
		EXPECT_EQ ( *tData.m_pData, i );
		tCache.Release(i);
	}
}

TEST ( AdaptiveLru, SecondTouchMissesStillAllowReclamation )
{
	AdaptiveLruTest_c tCache(true);
	for ( int i=0; i<256; ++i ) tCache.Put(i);
	const auto iBefore = tCache.Used();
	CacheTestData_t tData;
	for ( int i=0; i<30000; ++i )
	{
		// Two cold misses per 128 accesses model first-visit plus admission decoding.
		if ( i%128<2 ) EXPECT_FALSE ( tCache.Find ( -1, tData ) );
		else tCache.Put(i%4);
	}
	EXPECT_LT ( tCache.Used(), iBefore );
	EXPECT_LE ( tCache.Used(), 16*1024 );
}

TEST ( AdaptiveLru, ManualModeRetainsExistingBehavior )
{
	AdaptiveLruTest_c tCache(false);
	for ( int i=0; i<256; ++i ) tCache.Put(i);
	const auto iUsed = tCache.Used();
	tCache.Hot();
	ASSERT_EQ ( tCache.Used(), iUsed );
}

TEST ( AdaptiveLru, ReferencedBlocksSurviveReclamation )
{
	AdaptiveLruTest_c tCache(true);
	std::vector<CacheTestData_t> dPinned(256);
	for ( int i=0; i<256; ++i )
	{
		tCache.Put(i);
		ASSERT_TRUE ( tCache.Find ( i, dPinned[i] ) );
	}
	const auto iUsed = tCache.Used();
	tCache.Hot();
	ASSERT_EQ ( tCache.Used(), iUsed );
	for ( int i=0; i<256; ++i )
	{
		EXPECT_EQ ( *dPinned[i].m_pData, i%251 );
		tCache.Release(i);
	}
	tCache.Hot(2048);
	ASSERT_LE ( tCache.Used(), 16*1024 );
}

TEST ( AdaptiveLru, PinSurvivesEvictionAndReleasesWithoutLookup )
{
	AdaptiveLruTest_c tCache(true);
	tCache.Put(17);
	CacheTestData_t tData;
	AdaptiveLruTest_c::Ref_t pRef = nullptr;
	ASSERT_TRUE ( tCache.Find ( 17, tData, &pRef ) );
	for ( int i=1000; i<3000; ++i ) tCache.Put(i);
	EXPECT_EQ ( *tData.m_pData, 17 );
	tCache.ReleaseRef(pRef);
	for ( int i=3000; i<4000; ++i ) tCache.Put(i);
	EXPECT_FALSE ( tCache.Find ( 17, tData ) );
}

TEST ( AdaptiveLru, AddedPinAndDuplicateKeepSeparateOwnership )
{
	AdaptiveLruTest_c tCache(true);
	CacheTestData_t tOriginal { new BYTE[1] { 23 }, 1024 };
	AdaptiveLruTest_c::Ref_t pRef = nullptr;
	ASSERT_TRUE ( tCache.Add ( 23, tOriginal, &pRef ) );
	ASSERT_NE ( pRef, nullptr );
	CacheTestData_t tDuplicate { new BYTE[1] { 42 }, 1024 };
	AdaptiveLruTest_c::Ref_t pDuplicate = nullptr;
	EXPECT_FALSE ( tCache.Add ( 23, tDuplicate, &pDuplicate ) );
	EXPECT_EQ ( pDuplicate, nullptr );
	CacheTestUtil_t::Reset(tDuplicate);
	EXPECT_EQ ( *tOriginal.m_pData, 23 );
	tCache.ReleaseRef(pRef);
	tCache.Hot();
	EXPECT_LE ( tCache.Used(), 16*1024 );
}

TEST ( AdaptiveLru, ConcurrentGrowthReadsAndReclamation )
{
	AdaptiveLruTest_c tCache(true, 128*1024);
	std::atomic<int> iFailures { 0 };
	std::vector<std::thread> dThreads;
	for ( int i=0; i<8; ++i )
		dThreads.emplace_back ( [&,i] {
			for ( int j=0; j<30000; ++j )
			{
				int iKey = j<5000 ? (j*31+i*173)%1024 : j%4;
				tCache.Put(iKey);
				CacheTestData_t tData;
				AdaptiveLruTest_c::Ref_t pRef = nullptr;
				if ( tCache.Find ( iKey, tData, &pRef ) )
				{
					if ( *tData.m_pData!=iKey%251 ) ++iFailures;
					tCache.ReleaseRef(pRef);
				}
			}
		} );
	for ( auto & tThread : dThreads ) tThread.join();
	ASSERT_EQ ( iFailures.load(), 0 );
	// Overlapping thread phases can stop trimming at the one-quarter activity threshold.
	ASSERT_LT ( tCache.Used(), 32*1024 );
}

class DocstoreBlockCacheTest : public ::testing::TestWithParam<bool>
{
protected:
	void SetUp() override
	{
		InitDocstore ( 2*1024*1024, GetParam() );
		ASSERT_EQ ( GetDocstoreCacheUsedBytes(), 0 );
		const auto * pInfo = testing::UnitTest::GetInstance()->current_test_info();
		std::string sName = pInfo->name();
		for ( auto & c : sName ) if ( c=='/' ) c='_';
		for ( int i=0; i<2; ++i )
			m_dFiles[i].SetSprintf ( "__docstore_blocks_%d_%s_%d.spds", GetOsProcessId(), sName.c_str(), i );
	}

	void TearDown() override
	{
		ShutdownDocstore();
		for ( const auto & sFile : m_dFiles ) unlink(sFile.cstr());
	}

	static std::string Value ( int iVersion, int iRow, int iSize )
	{
		auto sPrefix = std::to_string(iVersion)+":"+std::to_string(iRow)+":";
		return sPrefix+std::string ( iSize-sPrefix.size(), char('a'+iRow%26) );
	}

	void Write ( int iFile, int iVersion, int iRows, int iSize, Compression_e eCompression = Compression_e::LZ4 )
	{
		CSphString sError;
		DocstoreSettings_t tSettings;
		tSettings.m_uBlockSize = 16384;
		tSettings.m_eCompression = eCompression;
		auto pBuilder = CreateDocstoreBuilder ( m_dFiles[iFile], tSettings, 65536, sError );
		ASSERT_TRUE(pBuilder) << sError.cstr();
		ASSERT_EQ ( pBuilder->AddField ( "body", DOCSTORE_TEXT ), 0 );
		for ( int i=0; i<iRows; ++i )
		{
			auto sValue = Value ( iVersion, i, iSize );
			DocstoreBuilder_i::Doc_t tDoc;
			tDoc.m_dFields.Add ( { reinterpret_cast<BYTE *>(sValue.data()), int(sValue.size()) } );
			pBuilder->AddDoc ( i, tDoc );
		}
		pBuilder->Finalize();
	}

	std::unique_ptr<Docstore_i> Open ( int iFile, int64_t iIndex )
	{
		CSphString sError;
		auto pStore = CreateDocstore ( iIndex, m_dFiles[iFile], sError );
		EXPECT_TRUE(pStore) << sError.cstr();
		return pStore;
	}

	static bool Matches ( const Docstore_i & tStore, int64_t iSession, int iVersion, int iRow, int iSize )
	{
		auto tDoc = tStore.GetDoc ( iRow, nullptr, iSession, false );
		if ( tDoc.m_dFields.GetLength()!=1 ) return false;
		const auto & dValue = tDoc.m_dFields[0];
		return std::string ( reinterpret_cast<const char *>(dValue.Begin()), dValue.GetLength() )==Value ( iVersion, iRow, iSize );
	}

	CSphString m_dFiles[2];
};

TEST_P ( DocstoreBlockCacheTest, ClearAndSameKeyReplacement )
{
	Write ( 0, 1, 256, 128 );
	Write ( 1, 2, 256, 128 );
	auto pStore = Open ( 0, 801 );
	ASSERT_TRUE(pStore);
	DocstoreSession_c tSession;
	pStore->CreateReader(tSession.GetUID());
	for ( int i=0; i<256; ++i ) ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 1, i, 128 ) );
	ClearDocstoreCache();
	ASSERT_EQ ( GetDocstoreCacheUsedBytes(), 0 );
	ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 1, 123, 128 ) );
	pStore.reset();
	pStore = Open ( 1, 801 );
	ASSERT_TRUE(pStore);
	pStore->CreateReader(tSession.GetUID());
	for ( int i=255; i>=0; --i ) ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 2, i, 128 ) );
}

TEST_P ( DocstoreBlockCacheTest, ConcurrentReadsUnderEvictionPressure )
{
	const int iRows = GetParam() ? 6144 : 2048;
	const int64_t iBudget = GetParam() ? 32*1024*1024 : 2*1024*1024;
	if ( GetParam() )
	{
		ShutdownDocstore();
		InitDocstore ( iBudget, true );
	}
	Write ( 0, 1, iRows, 4096 );
	auto pStore = Open ( 0, 802 );
	ASSERT_TRUE(pStore);
	std::atomic<int> iFailures { 0 };
	std::vector<std::thread> dThreads;
	auto fnRead = [&] ( int iCount, int iRange ) {
		dThreads.clear();
		for ( int i=0; i<8; ++i )
			dThreads.emplace_back ( [&,i,iCount,iRange] {
				DocstoreSession_c tSession;
				pStore->CreateReader(tSession.GetUID());
				for ( int j=0; j<iCount; ++j )
					if ( !Matches ( *pStore, tSession.GetUID(), 1, (j*509+i*173)%iRange, 4096 ) ) ++iFailures;
			} );
		for ( auto & tThread : dThreads ) tThread.join();
	};
	fnRead ( iRows, iRows );
	if ( GetParam() )
	{
		const auto iBefore = GetDocstoreCacheUsedBytes();
		ASSERT_GT ( iBefore, 20*1024*1024 );
		fnRead ( 50000, 512 );
		ASSERT_LT ( GetDocstoreCacheUsedBytes(), iBefore );
	}
	ASSERT_EQ ( iFailures.load(), 0 );
	ASSERT_LE ( GetDocstoreCacheUsedBytes(), iBudget );
	ClearDocstoreCache();
}

TEST_P ( DocstoreBlockCacheTest, LargeFieldsAndTinyBudgetFallback )
{
	Write ( 0, 1, 40, 70000 );
	auto pStore = Open ( 0, 803 );
	ASSERT_TRUE(pStore);
	DocstoreSession_c tSession;
	pStore->CreateReader(tSession.GetUID());
	for ( int k=0; k<2; ++k )
		for ( int i=0; i<40; ++i ) ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 1, i, 70000 ) );
	pStore.reset();
	ShutdownDocstore();
	InitDocstore ( 1, GetParam() );
	pStore = Open ( 0, 803 );
	ASSERT_TRUE(pStore);
	pStore->CreateReader(tSession.GetUID());
	for ( int i=0; i<40; ++i ) ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 1, i, 70000 ) );
	ASSERT_EQ ( GetDocstoreCacheUsedBytes(), 0 );
}

TEST_P ( DocstoreBlockCacheTest, UncompressedFieldsAndMappedClear )
{
	for ( int iFile=0; iFile<2; ++iFile )
	{
		const int iSize = iFile ? 70000 : 128;
		Write ( iFile, 3, 40, iSize, Compression_e::NONE );
		auto pStore = Open ( iFile, 810+iFile );
		ASSERT_TRUE(pStore);
		DocstoreSession_c tSession;
		pStore->CreateReader(tSession.GetUID());
		for ( int k=0; k<2; ++k )
			for ( int i=0; i<40; ++i ) ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 3, i, iSize ) );
		const auto tStats = GetDocstoreCacheStats();
		if ( GetParam() ) ASSERT_GT ( tStats.m_iMappedBytes, 0 );
		else ASSERT_EQ ( tStats.m_iMappedBytes, 0 );
	}
	ClearDocstoreCache();
	const auto tStats = GetDocstoreCacheStats();
	ASSERT_EQ ( tStats.m_iUsedBytes, 0 );
	ASSERT_EQ ( tStats.m_iMappedBytes, tStats.m_iSpareBytes );
	ASSERT_LE ( tStats.m_iSpareBytes, 16*64*1024 );
}

TEST_P ( DocstoreBlockCacheTest, FullWorkingSetFitsDefaultBudget )
{
	ShutdownDocstore();
	InitDocstore ( 16*1024*1024, GetParam() );
	Write ( 0, 1, 100000, 128 );
	auto pStore = Open ( 0, 820 );
	ASSERT_TRUE(pStore);
	DocstoreSession_c tSession;
	pStore->CreateReader(tSession.GetUID());
	for ( int i=0; i<100000; ++i ) ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 1, i, 128 ) );
	const auto iUsed = GetDocstoreCacheUsedBytes();
	ASSERT_GT ( iUsed, 100000*128 );
	ASSERT_LT ( iUsed, 14*1024*1024 );
	ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 1, 0, 128 ) );
}

INSTANTIATE_TEST_SUITE_P ( Modes, DocstoreBlockCacheTest, ::testing::Values ( false, true ) );

TEST_P ( DocstoreBlockCacheTest, SecondTouchAdmissionAndIndexReplacement )
{
	ShutdownDocstore();
	InitDocstore ( 16*1024*1024, GetParam() );
	Write ( 0, 1, 256, 128 );
	auto pStore = Open ( 0, 840 );
	ASSERT_TRUE(pStore);
	DocstoreSession_c tFirst;
	pStore->CreateReader(tFirst.GetUID());
	ASSERT_TRUE ( Matches ( *pStore, tFirst.GetUID(), 1, 0, 128 ) );
	if ( GetParam() ) ASSERT_EQ ( GetDocstoreCacheUsedBytes(), 0 );
	else ASSERT_GT ( GetDocstoreCacheUsedBytes(), 0 );
	ASSERT_TRUE ( Matches ( *pStore, tFirst.GetUID(), 1, 0, 128 ) );
	ASSERT_GT ( GetDocstoreCacheUsedBytes(), 0 );
	ClearDocstoreCache();
	ASSERT_EQ ( GetDocstoreCacheUsedBytes(), 0 );
	ASSERT_TRUE ( Matches ( *pStore, tFirst.GetUID(), 1, 0, 128 ) );
	Write ( 1, 2, 256, 128 );
	pStore.reset();
	pStore = Open ( 1, 840 );
	ASSERT_TRUE(pStore);
	pStore->CreateReader(tFirst.GetUID());
	for ( int i=0; i<256; ++i ) ASSERT_TRUE ( Matches ( *pStore, tFirst.GetUID(), 2, i, 128 ) );
}

TEST_P ( DocstoreBlockCacheTest, SecondTouchConcurrentReadsAndUncachedFallback )
{
	ShutdownDocstore();
	InitDocstore ( 16*1024*1024, GetParam() );
	Write ( 0, 1, 2048, 128 );
	auto pStore = Open ( 0, 841 );
	ASSERT_TRUE(pStore);
	std::atomic<int> iFailures {0};
	std::vector<std::thread> dThreads;
	for ( int i=0; i<8; ++i )
		dThreads.emplace_back ( [&,i] {
			for ( int q=0; q<20; ++q )
			{
				DocstoreSession_c tSession;
				pStore->CreateReader(tSession.GetUID());
				for ( int j=0; j<128; ++j )
					if ( !Matches ( *pStore, tSession.GetUID(), 1, (i*257+q*31+j)%2048, 128 ) ) ++iFailures;
			}
		} );
	for ( auto & tThread : dThreads ) tThread.join();
	ASSERT_EQ ( iFailures.load(), 0 );
	EXPECT_TRUE ( Matches ( *pStore, -1, 1, 42, 128 ) );
	EXPECT_LE ( GetDocstoreCacheUsedBytes(), 16*1024*1024 );
	pStore.reset();
	ShutdownDocstore();
	InitDocstore ( 0, GetParam() );
	pStore = Open ( 0, 841 );
	ASSERT_TRUE(pStore);
	EXPECT_TRUE ( Matches ( *pStore, -1, 1, 42, 128 ) );
	EXPECT_EQ ( GetDocstoreCacheUsedBytes(), 0 );
}

TEST_P ( DocstoreBlockCacheTest, SecondTouchLargeAndUncompressedFallback )
{
	ShutdownDocstore();
	InitDocstore ( 16*1024*1024, GetParam() );
	for ( auto eCompression : { Compression_e::NONE, Compression_e::LZ4 } )
	for ( int iFile=0; iFile<2; ++iFile )
	{
		const int iSize = iFile ? 70000 : 128;
		Write ( iFile, 3, 40, iSize, eCompression );
		auto pStore = Open ( iFile, 850+iFile );
		ASSERT_TRUE(pStore);
		for ( int query=0; query<3; ++query )
		{
			DocstoreSession_c tSession;
			pStore->CreateReader(tSession.GetUID());
			for ( int i=0; i<40; ++i ) ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 3, i, iSize ) );
		}
	}
	ClearDocstoreCache();
	ASSERT_EQ ( GetDocstoreCacheUsedBytes(), 0 );
}

TEST ( DocstoreAdmission, SecondTouchCollisionsAndZeroHash )
{
	DocstoreAdmission_c tHistory;
	for ( uint32_t uHash : { 0U, 1U, UINT32_MAX } )
	{
		EXPECT_FALSE ( tHistory.Admit(uHash) );
		EXPECT_TRUE ( tHistory.Admit(uHash) );
		EXPECT_FALSE ( tHistory.Admit(uHash^65536U) );
		EXPECT_FALSE ( tHistory.Admit(uHash) );
		EXPECT_TRUE ( tHistory.Admit(uHash) );
	}
}

TEST ( DocstoreAdmission, ConcurrentFirstTouch )
{
	DocstoreAdmission_c tHistory;
	std::atomic<int> iAdmitted { 0 };
	std::vector<std::thread> dThreads;
	for ( int i=0; i<8; ++i ) dThreads.emplace_back ( [&] {
		if ( tHistory.Admit(12345) ) ++iAdmitted;
	} );
	for ( auto & tThread : dThreads ) tThread.join();
	EXPECT_EQ ( iAdmitted.load(), 7 );
}

}
