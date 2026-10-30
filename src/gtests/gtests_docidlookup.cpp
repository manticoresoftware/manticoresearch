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
		EXPECT_TRUE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), UUID_INDEX_VERSION+1, sError ) ) << sError.cstr(); // v.72: same layout
		EXPECT_TRUE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), UUID_INDEX_VERSION-1, sError ) ); // v.70 is not suspect - never checked
	}

	DowngradeToV70();
	{
		CSphMappedBuffer<BYTE> tData;
		Map ( tData );
		CSphString sError;
		EXPECT_TRUE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), UUID_INDEX_VERSION-1, sError ) ) << sError.cstr();
		EXPECT_FALSE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), UUID_INDEX_VERSION, sError ) );
		EXPECT_FALSE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), DOCID_LOOKUP_SUSPECT_MAX, sError ) ); // v.72 is suspect too
		EXPECT_TRUE ( CheckDocidLookupFormat ( tData.GetReadPtr(), tData.GetLengthBytes(), DOCID_LOOKUP_SUSPECT_MAX+1, sError ) ) << sError.cstr(); // v.73+: fixed, never checked
		EXPECT_TRUE ( strstr ( sError.cstr(), "pre-v.71" ) ) << sError.cstr();
		EXPECT_EQ ( DetectDocidLookupVersion ( tData.GetReadPtr(), tData.GetLengthBytes() ), UUID_INDEX_VERSION-1 );

		// only suspect headers are validated: a legacy lookup (a v.54 file is 10 bytes) passes under any
		// non-suspect version and fails under a suspect one
		const BYTE dLegacy[10] = { 3, 0, 0, 0, 0, 1, 1, 0, 0, 0 };
		EXPECT_TRUE ( CheckDocidLookupFormat ( dLegacy, sizeof(dLegacy), DOCID_LOOKUP_SPLIT_VERSION-1, sError ) ) << sError.cstr();
		EXPECT_TRUE ( CheckDocidLookupFormat ( dLegacy, sizeof(dLegacy), DOCID_LOOKUP_SPLIT_VERSION, sError ) ) << sError.cstr();
		EXPECT_FALSE ( CheckDocidLookupFormat ( dLegacy, sizeof(dLegacy), DOCID_LOOKUP_SUSPECT_MIN, sError ) );
		EXPECT_TRUE ( UpgradeDocidLookupFile ( m_sFile, DOCID_LOOKUP_SPLIT_VERSION-1, sError ) ) << sError.cstr(); // no-op below the gate
		EXPECT_EQ ( DetectDocidLookupVersion ( tData.GetReadPtr(), tData.GetLengthBytes() ), UUID_INDEX_VERSION-1 ); // the file is untouched

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

	// the fixed twins: 71->73, 72->74; already-fixed versions keep their number
	EXPECT_EQ ( FixedIndexFormatVersion(70), 73U );
	EXPECT_EQ ( FixedIndexFormatVersion(71), 73U );
	EXPECT_EQ ( FixedIndexFormatVersion(72), 74U );
	EXPECT_EQ ( FixedIndexFormatVersion(73), 73U );
	EXPECT_EQ ( FixedIndexFormatVersion(74), 74U );
}

} // namespace

namespace
{
struct CacheBudgetGuard_t
{
	explicit CacheBudgetGuard_t ( size_t nBytes ) { DocidLookupCache_c::SetBudget(nBytes); }
	~CacheBudgetGuard_t() { DocidLookupCache_c::SetBudget(0); }
};

CSphVector<RowID_t> CollectLookupRows ( const BYTE * pData, DWORD uVersion, DWORD nDocs, const CSphVector<DocID_t> & dValues, const LookupReader_c * pCacheReader, int64_t & nProcessed, bool bBitmap = false, int iPartition = -1 )
{
	CSphVector<CSphFilterSettings> dFilters;
	auto & tFilter = dFilters.Add();
	tFilter.m_sAttrName = "id";
	tFilter.m_eType = SPH_FILTER_VALUES;
	tFilter.m_dValues = dValues;
	CSphVector<SecondaryIndexInfo_t> dInfo;
	auto & tInfo = dInfo.Add();
	tInfo.m_eType = SecondaryIndexType_e::LOOKUP;
	// Exercise both storage specializations regardless of actual selectivity.
	tInfo.m_iRsetEstimate = bBitmap ? nDocs : 0;
	if ( iPartition>=0 )
	{
		auto & tRowFilter = dFilters.Add();
		tRowFilter.m_sAttrName = "@rowid";
		tRowFilter.m_eType = SPH_FILTER_RANGE;
		tRowFilter.m_iMinValue = iPartition;
		tRowFilter.m_iMaxValue = 4;
		dInfo.Add();
	}
	auto dIterators = CreateLookupIterator ( dInfo, dFilters, pData, uVersion, nDocs, pCacheReader );
	CSphVector<RowID_t> dRows;
	if ( dIterators.GetLength()!=1 )
	{
		ADD_FAILURE() << "expected one ID iterator";
		return dRows;
	}
	std::unique_ptr<RowidIterator_i> pIterator(dIterators[0].first);
	RowIdBlock_t dBlock;
	while ( pIterator->GetNextRowIdBlock(dBlock) )
		for ( RowID_t tRow : dBlock )
			dRows.Add(tRow);
	nProcessed = pIterator->GetNumProcessed();
	return dRows;
}
}

TEST_F ( UuidDocidLookupTest, NativeCacheMatchesAndInvalidatesOnRemap )
{
	CacheBudgetGuard_t tBudget ( 1024*1024 );
	CSphVector<DocidRowidPair_t> dPairs;
	for ( RowID_t i=0; i<1025; ++i )
		dPairs.Add ( { (DocID_t)(i*2), i } );
	dPairs.Add ( { INT64_MIN, 2000 } );
	dPairs.Add ( { -1, 2001 } );
	CSphString sError;
	ASSERT_TRUE ( WriteDocidLookup ( m_sFile, dPairs, sError ) ) << sError.cstr();
	CSphMappedBuffer<BYTE> tData;
	Map(tData);
	LookupReader_c tReference ( tData.GetReadPtr(), UUID_INDEX_VERSION );
	LookupReader_c tCached ( tData.GetReadPtr(), UUID_INDEX_VERSION );
	ASSERT_TRUE ( tCached.BuildCache(UUID_INDEX_VERSION) );
	for ( const auto & tPair : dPairs )
		EXPECT_EQ ( tCached.Find(tPair.m_tDocID), tReference.Find(tPair.m_tDocID) );
	for ( DocID_t i=1; i<2050; i+=2 )
		EXPECT_EQ ( tCached.Find(i), INVALID_ROWID );
	tCached.SetData ( tData.GetReadPtr(), UUID_INDEX_VERSION );
	EXPECT_FALSE ( tCached.HasCache() );
	EXPECT_EQ ( DocidLookupCache_c::GetUsed(), 0U );
	EXPECT_EQ ( tCached.Find(-1), 2001U );
	ASSERT_TRUE ( tCached.BuildCache(UUID_INDEX_VERSION) );
	tCached.SetData ( nullptr, UUID_INDEX_VERSION );
	EXPECT_EQ ( tCached.Find(0), INVALID_ROWID );
	EXPECT_EQ ( DocidLookupCache_c::GetUsed(), 0U );
}

TEST_F ( UuidDocidLookupTest, NativeCacheBudgetAndDuplicateFallback )
{
	CacheBudgetGuard_t tBudget(1024);
	CSphVector<DocidRowidPair_t> dPairs;
	for ( RowID_t i=0; i<4096; ++i )
		dPairs.Add ( { (DocID_t)(i*2), i } );
	CSphString sError;
	ASSERT_TRUE ( WriteDocidLookup ( m_sFile, dPairs, sError ) );
	{
		CSphMappedBuffer<BYTE> tData;
		Map(tData);
		LookupReader_c tReader ( tData.GetReadPtr(), UUID_INDEX_VERSION );
		EXPECT_FALSE ( tReader.BuildCache(UUID_INDEX_VERSION) );
		EXPECT_EQ ( DocidLookupCache_c::GetUsed(), 0U );
		EXPECT_EQ ( tReader.Find(8000), 4000U );
	}
	DocidLookupCache_c::SetBudget ( 1024*1024 );
	dPairs.Reset();
	for ( RowID_t i=0; i<200; ++i )
		dPairs.Add ( { 42, i } );
	ASSERT_TRUE ( WriteDocidLookup ( m_sFile, dPairs, sError ) );
	CSphMappedBuffer<BYTE> tData;
	Map(tData);
	LookupReader_c tReader ( tData.GetReadPtr(), UUID_INDEX_VERSION );
	EXPECT_FALSE ( tReader.BuildCache(UUID_INDEX_VERSION) );
	EXPECT_EQ ( DocidLookupCache_c::GetUsed(), 0U );
	CSphVector<DocID_t> dValues;
	dValues.Add(42);
	int64_t nProcessed;
	auto dRows = CollectLookupRows ( tData.GetReadPtr(), UUID_INDEX_VERSION, 200, dValues, &tReader, nProcessed );
	EXPECT_EQ ( dRows.GetLength(), 200 ); // all duplicates must survive filtering
}

TEST_F ( UuidDocidLookupTest, NativeCacheLegacyV70 )
{
	CacheBudgetGuard_t tBudget ( 1024*1024 );
	WriteNumericLookup();
	DowngradeToV70();
	CSphMappedBuffer<BYTE> tData;
	Map(tData);
	LookupReader_c tReader ( tData.GetReadPtr(), UUID_INDEX_VERSION-1 );
	ASSERT_TRUE ( tReader.BuildCache(UUID_INDEX_VERSION-1) );
	EXPECT_EQ ( tReader.Find(11), 0U );
	EXPECT_EQ ( tReader.Find(22), 1U );
	EXPECT_EQ ( tReader.Find(33), 2U );
	EXPECT_EQ ( tReader.Find(44), INVALID_ROWID );
}

TEST_F ( UuidDocidLookupTest, NativeCacheSparseAndDenseIdFiltersMatchMerge )
{
	CacheBudgetGuard_t tBudget ( 1024*1024 );
	CSphVector<DocidRowidPair_t> dPairs;
	for ( RowID_t i=0; i<4094; ++i )
		dPairs.Add ( { (DocID_t)(i*2), (i*37)%4096 } );
	dPairs.Add ( { INT64_MIN, (4094*37)%4096 } );
	dPairs.Add ( { -1, (4095*37)%4096 } );
	CSphString sError;
	ASSERT_TRUE ( WriteDocidLookup ( m_sFile, dPairs, sError ) );
	CSphMappedBuffer<BYTE> tData;
	Map(tData);
	LookupReader_c tReader ( tData.GetReadPtr(), UUID_INDEX_VERSION );
	ASSERT_TRUE ( tReader.BuildCache(UUID_INDEX_VERSION) );
	for ( int nValues : { 8, 13, 14, 15, 16, 29, 30, 31, 32, 61, 62, 63, 64, 4096 } )
	{
		bool bDense = uint64_t(nValues+2)*64>4096;
		CSphVector<DocID_t> dValues;
		dValues.Add(-1);
		dValues.Add(INT64_MIN);
		for ( DocID_t i=0; i<nValues; ++i )
			dValues.Add(i*3); // a mixture of hits and misses
		dValues.Sort();
		for ( bool bBitmap : { false, true } )
			for ( int iPartition : { -1, 0, 1, 3 } )
			{
				int64_t nReference, nCached;
				auto dReference = CollectLookupRows ( tData.GetReadPtr(), UUID_INDEX_VERSION, 4096, dValues, nullptr, nReference, bBitmap, iPartition );
				auto dCached = CollectLookupRows ( tData.GetReadPtr(), UUID_INDEX_VERSION, 4096, dValues, &tReader, nCached, bBitmap, iPartition );
				ASSERT_EQ ( dReference.GetLength(), dCached.GetLength() );
				for ( int i=0; i<dReference.GetLength(); ++i )
					EXPECT_EQ ( dReference[i], dCached[i] );
				if ( bDense )
					EXPECT_EQ ( nCached, nReference ); // dense lists retain merge
				else
				{
					EXPECT_EQ ( nCached, dValues.GetLength() );
					EXPECT_LT ( nCached, nReference );
				}
			}
	}
}

TEST_F ( UuidDocidLookupTest, NativeBatchMatchesCompressedAndHandlesRemap )
{
	CacheBudgetGuard_t tBudget ( 1024*1024 );
	CSphVector<DocidRowidPair_t> dPairs;
	for ( RowID_t i=0; i<1025; ++i ) dPairs.Add ( { (DocID_t)(i*2), i } );
	dPairs.Add ( { INT64_MIN, 1025 } );
	dPairs.Add ( { -1, 1026 } );
	CSphString sError;
	ASSERT_TRUE ( WriteDocidLookup ( m_sFile, dPairs, sError ) );
	CSphMappedBuffer<BYTE> tData;
	Map(tData);
	LookupReader_c tReference ( tData.GetReadPtr(), UUID_INDEX_VERSION );
	LookupReader_c tCached ( tData.GetReadPtr(), UUID_INDEX_VERSION );
	ASSERT_TRUE ( tCached.BuildCache(UUID_INDEX_VERSION) );
	uint64_t dQueries[65];
	RowID_t dRows[65];
	for ( int i=0; i<65; ++i ) dQueries[i] = i;
	dQueries[15] = uint64_t(INT64_MIN);
	dQueries[16] = UINT64_MAX;
	dQueries[32] = UINT64_MAX-1;
	for ( auto * pReader : { &tReference, &tCached } )
	{
		pReader->FindBatch ( nullptr, nullptr, 0 );
		for ( size_t nQueries : { 1, 15, 16, 17, 31, 32, 33, 65 } )
		{
			pReader->FindBatch ( dQueries, dRows, nQueries );
			for ( size_t i=0; i<nQueries; ++i ) EXPECT_EQ ( dRows[i], tReference.Find((DocID_t)dQueries[i]) );
		}
	}
	tCached.SetData ( nullptr, UUID_INDEX_VERSION );
	tCached.FindBatch ( dQueries, dRows, 65 );
	for ( RowID_t uRow : dRows ) EXPECT_EQ ( uRow, INVALID_ROWID );
}
