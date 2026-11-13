// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// Distributed under the GNU General Public License, version 3 or later.

#include <gtest/gtest.h>
#include "docstore.h"
#include "indexsettings.h"
#include "threadutils.h"

#include <atomic>
#include <string>
#include <thread>
#include <vector>

namespace
{
class DocstoreBlockCacheTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		InitDocstore ( 2*1024*1024 );
		const auto * pInfo = testing::UnitTest::GetInstance()->current_test_info();
		for ( int i=0; i<2; ++i )
			m_dFiles[i].SetSprintf ( "__docstore_blocks_%d_%s_%d.spds", GetOsProcessId(), pInfo->name(), i );
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

	void Write ( int iFile, int iVersion, int iRows, int iSize )
	{
		CSphString sError;
		DocstoreSettings_t tSettings;
		tSettings.m_uBlockSize = 16384;
		tSettings.m_eCompression = Compression_e::LZ4;
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

TEST_F ( DocstoreBlockCacheTest, ClearAndSameKeyReplacement )
{
	Write ( 0, 1, 256, 128 );
	Write ( 1, 2, 256, 128 );
	auto pStore = Open ( 0, 801 );
	ASSERT_TRUE(pStore);
	DocstoreSession_c tSession;
	pStore->CreateReader(tSession.GetUID());
	for ( int i=0; i<256; ++i ) ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 1, i, 128 ) );
	ClearDocstoreCache();
	ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 1, 123, 128 ) );
	pStore.reset();
	pStore = Open ( 1, 801 );
	ASSERT_TRUE(pStore);
	pStore->CreateReader(tSession.GetUID());
	for ( int i=255; i>=0; --i ) ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 2, i, 128 ) );
}

TEST_F ( DocstoreBlockCacheTest, ConcurrentReadsUnderEvictionPressure )
{
	Write ( 0, 1, 2048, 4096 ); // 8 MiB decoded data exceeds the 2 MiB shared budget.
	auto pStore = Open ( 0, 802 );
	ASSERT_TRUE(pStore);
	std::atomic<int> iFailures { 0 };
	std::vector<std::thread> dThreads;
	for ( int i=0; i<8; ++i )
		dThreads.emplace_back ( [&,i] {
			DocstoreSession_c tSession;
			pStore->CreateReader(tSession.GetUID());
			for ( int j=0; j<2048; ++j )
				if ( !Matches ( *pStore, tSession.GetUID(), 1, (j*509+i*173)%2048, 4096 ) ) ++iFailures;
		} );
	for ( auto & tThread : dThreads ) tThread.join();
	ASSERT_EQ ( iFailures.load(), 0 );
	ClearDocstoreCache();
}

TEST_F ( DocstoreBlockCacheTest, LargeFieldsAndTinyBudgetFallback )
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
	InitDocstore(1);
	pStore = Open ( 0, 803 );
	ASSERT_TRUE(pStore);
	pStore->CreateReader(tSession.GetUID());
	for ( int i=0; i<40; ++i ) ASSERT_TRUE ( Matches ( *pStore, tSession.GetUID(), 1, i, 70000 ) );
}
}
