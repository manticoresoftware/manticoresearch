//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you do
// not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>

#include "docidlookup.h"
#include "fileio.h"
#include "fileutils.h"
#include "threadutils.h"

#include <cstring>

namespace
{

constexpr DWORD UUID_INDEX_VERSION = 71;

UuidDocidKey_t UuidKey ( const char * szUuid )
{
	return sphGetUuidDocidKey ( { szUuid, (int)strlen(szUuid) } );
}


CSphVector<DocidRowidPair_t> MakeDocidLookup()
{
	CSphVector<DocidRowidPair_t> dLookup;
	dLookup.Add ( { 11, 0 } );
	dLookup.Add ( { 22, 1 } );
	dLookup.Add ( { 33, 2 } );
	return dLookup;
}


CSphVector<UuidDocidLookupPair_t> MakeUuidLookup()
{
	CSphVector<UuidDocidLookupPair_t> dLookup;
	dLookup.Add ( { UuidKey ( "00000000-0000-1000-8000-000000000001" ), 11 } );
	dLookup.Add ( { UuidKey ( "00000000-0000-4000-8000-000000000010" ), 22 } );
	dLookup.Add ( { UuidKey ( "ffffffff-ffff-8fff-bfff-ffffffffffff" ), 33 } );
	return dLookup;
}


class UuidDocidLookupTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		const testing::TestInfo * pInfo = testing::UnitTest::GetInstance()->current_test_info();
		m_sFile.SetSprintf ( "__uuid_lookup_%d_%s.spt", GetOsProcessId(), pInfo->name() );
	}

	void TearDown() override
	{
		unlink ( m_sFile.cstr() );
	}

	void WriteUuidLookup()
	{
		CSphString sError;
		ASSERT_TRUE ( WriteDocidLookup ( m_sFile, MakeDocidLookup(), MakeUuidLookup(), sError ) ) << sError.cstr();
	}

	void WriteNumericLookup()
	{
		CSphString sError;
		ASSERT_TRUE ( WriteDocidLookup ( m_sFile, MakeDocidLookup(), sError ) ) << sError.cstr();
	}

	void Map ( CSphMappedBuffer<BYTE> & tData )
	{
		CSphString sError;
		ASSERT_TRUE ( tData.Setup ( m_sFile, sError, false ) ) << sError.cstr();
	}

	CSphString m_sFile;

	// turns a current-format lookup file into the pre-v.71 layout: no UUID entries offset in the
	// header, checkpoint data offsets 8 bytes lower
	void DowngradeToV70()
	{
		CSphVector<BYTE> dOld;
		{
			CSphMappedBuffer<BYTE> tData;
			Map ( tData );
			dOld.Resize ( tData.GetLengthBytes() );
			memcpy ( dOld.Begin(), tData.GetReadPtr(), tData.GetLengthBytes() );
		}

		const int64_t iOldHeader = sizeof(DWORD)*2 + sizeof(DocID_t) + sizeof(SphOffset_t);
		const int64_t iNewHeader = sizeof(DWORD)*2 + sizeof(DocID_t);
		DWORD nDocs = *(const DWORD*)dOld.Begin();
		DWORD nPerCheckpoint = *(const DWORD*)( dOld.Begin()+sizeof(DWORD) );
		int64_t nCheckpoints = ( nDocs + nPerCheckpoint - 1 ) / nPerCheckpoint;
		int64_t iDataStart = iOldHeader + nCheckpoints*(int64_t)sizeof(DocidLookupCheckpoint_t);

		CSphString sError;
		CSphWriter tWriter;
		ASSERT_TRUE ( tWriter.OpenFile ( m_sFile, sError ) ) << sError.cstr();
		tWriter.PutBytes ( dOld.Begin(), iNewHeader );
		const auto * pCheckpoints = (const DocidLookupCheckpoint_t *)( dOld.Begin()+iOldHeader );
		for ( int64_t i = 0; i<nCheckpoints; ++i )
		{
			tWriter.PutOffset ( pCheckpoints[i].m_tBaseDocID );
			tWriter.PutOffset ( pCheckpoints[i].m_tOffset - ( iOldHeader-iNewHeader ) );
		}
		tWriter.PutBytes ( dOld.Begin()+iDataStart, dOld.GetLength()-iDataStart );
		tWriter.CloseFile();
		ASSERT_FALSE ( tWriter.IsError() );
	}
};

TEST_F ( UuidDocidLookupTest, NumericPrefixCompatibility )
{
	WriteNumericLookup();

	CSphMappedBuffer<BYTE> tData;
	Map ( tData );
	LookupReader_c tNumeric;
	auto [ tUuidOffset, nDocs ] = tNumeric.SetData ( tData.GetReadPtr(), UUID_INDEX_VERSION );
	EXPECT_EQ ( tNumeric.Find(11), 0U );
	EXPECT_EQ ( tNumeric.Find(22), 1U );
	EXPECT_EQ ( tNumeric.Find(33), 2U );
	EXPECT_EQ ( tUuidOffset, 0 );
	EXPECT_EQ ( nDocs, 3U );

	UuidLookupReader_c tReader;
	tReader.SetData ( tData.GetReadPtr(), tUuidOffset, nDocs );
	EXPECT_EQ ( tReader.Find ( UuidKey ( "00000000-0000-1000-8000-000000000001" ) ), 0U );
}


TEST_F ( UuidDocidLookupTest, WriterReaderRoundTripAndSearch )
{
	WriteUuidLookup();

	CSphMappedBuffer<BYTE> tData;
	Map ( tData );
	LookupReader_c tNumeric;
	auto [ tUuidOffset, nDocs ] = tNumeric.SetData ( tData.GetReadPtr(), UUID_INDEX_VERSION );
	EXPECT_EQ ( tNumeric.Find(11), 0U );
	EXPECT_EQ ( tNumeric.Find(22), 1U );
	EXPECT_EQ ( tNumeric.Find(33), 2U );
	EXPECT_EQ ( nDocs, 3U );

	UuidLookupReader_c tReader;
	tReader.SetData ( tData.GetReadPtr(), tUuidOffset, nDocs );
	EXPECT_EQ ( tReader.Find ( UuidKey ( "00000000-0000-1000-8000-000000000001" ) ), 11U );
	EXPECT_EQ ( tReader.Find ( UuidKey ( "00000000-0000-4000-8000-000000000010" ) ), 22U );
	EXPECT_EQ ( tReader.Find ( UuidKey ( "ffffffff-ffff-8fff-bfff-ffffffffffff" ) ), 33U );
	EXPECT_EQ ( tReader.Find ( UuidKey ( "00000000-0000-1000-8000-000000000000" ) ), 0U );
	EXPECT_EQ ( tReader.Find ( UuidKey ( "00000000-0000-1000-8000-000000000002" ) ), 0U );
}

// manticoresearch#4852: an in-place header rewrite stamped v.71 on chunks whose .spt was still v.70,
// so every docid lookup on them returned garbage rowids; the layout check must catch it, the upgrade must fix it
TEST_F ( UuidDocidLookupTest, FormatCheckAndUpgrade )
{
	WriteNumericLookup();
	{
		CSphMappedBuffer<BYTE> tData;
		Map ( tData );
		CSphString sError;
		EXPECT_TRUE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), UUID_INDEX_VERSION, sError ) ) << sError.cstr();
		EXPECT_FALSE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), UUID_INDEX_VERSION-1, sError ) );
	}

	DowngradeToV70();
	{
		CSphMappedBuffer<BYTE> tData;
		Map ( tData );
		CSphString sError;
		EXPECT_TRUE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), UUID_INDEX_VERSION-1, sError ) ) << sError.cstr();
		EXPECT_FALSE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), UUID_INDEX_VERSION, sError ) );
		EXPECT_TRUE ( strstr ( sError.cstr(), "pre-v.71" ) ) << sError.cstr();
		EXPECT_EQ ( DetectDocidLookupVersion ( tData.GetReadPtr(), tData.GetLengthBytes() ), UUID_INDEX_VERSION-1 );

		LookupReader_c tOld;
		tOld.SetData ( tData.GetReadPtr(), UUID_INDEX_VERSION-1 );
		EXPECT_EQ ( tOld.Find(11), 0U );
		EXPECT_EQ ( tOld.Find(22), 1U );
		EXPECT_EQ ( tOld.Find(33), 2U );
	}

	CSphString sError;
	ASSERT_TRUE ( UpgradeDocidLookupFile ( m_sFile, UUID_INDEX_VERSION-1, sError ) ) << sError.cstr();
	{
		CSphMappedBuffer<BYTE> tData;
		Map ( tData );
		EXPECT_TRUE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), UUID_INDEX_VERSION, sError ) ) << sError.cstr();
		EXPECT_EQ ( DetectDocidLookupVersion ( tData.GetReadPtr(), tData.GetLengthBytes() ), UUID_INDEX_VERSION );

		LookupReader_c tNew;
		auto [ tUuidOffset, nDocs ] = tNew.SetData ( tData.GetReadPtr(), UUID_INDEX_VERSION );
		EXPECT_EQ ( tUuidOffset, 0 );
		EXPECT_EQ ( nDocs, 3U );
		EXPECT_EQ ( tNew.Find(11), 0U );
		EXPECT_EQ ( tNew.Find(22), 1U );
		EXPECT_EQ ( tNew.Find(33), 2U );
		EXPECT_EQ ( tNew.Find(44), INVALID_ROWID );
	}

	// upgrading a current-format file is a no-op
	ASSERT_TRUE ( UpgradeDocidLookupFile ( m_sFile, UUID_INDEX_VERSION, sError ) ) << sError.cstr();
}

} // namespace
