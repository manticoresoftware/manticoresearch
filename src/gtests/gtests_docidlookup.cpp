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
#include "fileutils.h"
#include "killlist.h"
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


class LookupReaderForTest_c
{
public:
	explicit LookupReaderForTest_c ( const CSphVector<DocidRowidPair_t> & dEntries )
		: m_dEntries ( dEntries )
	{}

	bool Read ( DocID_t & tDocID, RowID_t & tRowID )
	{
		if ( m_iEntry==m_dEntries.GetLength() )
			return false;

		const DocidRowidPair_t & tEntry = m_dEntries[m_iEntry++];
		tDocID = tEntry.m_tDocID;
		tRowID = tEntry.m_tRowID;
		return true;
	}

	void HintDocID ( DocID_t ) {}

private:
	const CSphVector<DocidRowidPair_t> & m_dEntries;
	int m_iEntry {0};
};


class KillerReaderForTest_c
{
public:
	explicit KillerReaderForTest_c ( const CSphVector<DocID_t> & dEntries )
		: m_dEntries ( dEntries )
	{}

	bool ReadDocID ( DocID_t & tDocID )
	{
		if ( m_iEntry==m_dEntries.GetLength() )
			return false;

		tDocID = m_dEntries[m_iEntry++];
		return true;
	}

	void HintDocID ( DocID_t ) {}

private:
	const CSphVector<DocID_t> & m_dEntries;
	int m_iEntry {0};
};


TEST ( KillByLookupTest, SkipsOutOfRangeRowids )
{
	CSphVector<DocidRowidPair_t> dTarget;
	dTarget.Add ( { 1, 0 } );
	dTarget.Add ( { 2, 4076938452 } );

	CSphVector<DocID_t> dKiller;
	dKiller.Add ( 1 );
	dKiller.Add ( 2 );

	LookupReaderForTest_c tTargetReader ( dTarget );
	KillerReaderForTest_c tKillerReader ( dKiller );
	DeadRowMap_Ram_c tDeadRows ( 2 );

	EXPECT_EQ ( KillByLookup ( tTargetReader, tKillerReader, tDeadRows ), 1 );
	EXPECT_TRUE ( tDeadRows.IsSet ( 0 ) );
	EXPECT_FALSE ( tDeadRows.IsSet ( 1 ) );
}

} // namespace
