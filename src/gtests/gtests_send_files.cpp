//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//

#include <gtest/gtest.h>

#include "replication/send_files.h"

#include <cstring>
#include <limits>

namespace
{
FileSyncLayout_t Layout ( int64_t iFileSize, int64_t iPreferredChunk )
{
	FileSyncLayout_t tResult;
	tResult.m_iFileSize = iFileSize;
	tResult.m_iPreferredChunkBytes = iPreferredChunk;
	return tResult;
}

void CheckContiguous ( const CSphVector<FileSyncLayout_t> & dFiles, int64_t iHashes )
{
	int64_t iNext = dFiles.GetLength();
	for ( const FileSyncLayout_t & tFile : dFiles )
	{
		EXPECT_EQ ( tFile.m_iHashStartItem, iNext );
		if ( tFile.m_iFileSize )
			iNext += 1 + ( tFile.m_iFileSize - 1 ) / tFile.m_iChunkBytes;
	}
	EXPECT_EQ ( iNext, iHashes );
}
}


TEST ( SstHashLayout, KeepsNaturalLayoutBelowBudget )
{
	CSphVector<FileSyncLayout_t> dFiles;
	dFiles.Add ( Layout ( 10000, 2048 ) );
	dFiles.Add ( Layout ( 100, 100 ) );

	const int64_t iBefore = CountFileSyncHashes ( dFiles );
	int64_t iHashes = 0;
	int64_t iFloor = -1;
	CSphString sError;
	ASSERT_TRUE ( AdjustFileSyncLayout ( dFiles, iBefore, 8192, iHashes, iFloor, sError ) ) << sError.cstr();
	EXPECT_EQ ( iHashes, iBefore );
	EXPECT_EQ ( iFloor, 0 );
	EXPECT_EQ ( dFiles[0].m_iChunkBytes, 2048 );
	EXPECT_EQ ( dFiles[1].m_iChunkBytes, 100 );
	CheckContiguous ( dFiles, iHashes );
}


TEST ( SstHashLayout, AdjustsOneHashAboveBudget )
{
	CSphVector<FileSyncLayout_t> dFiles;
	dFiles.Add ( Layout ( 10000, 2000 ) );
	dFiles.Add ( Layout ( 10000, 2000 ) );
	ASSERT_EQ ( CountFileSyncHashes ( dFiles ), 12 );

	int64_t iHashes = 0;
	int64_t iFloor = 0;
	CSphString sError;
	ASSERT_TRUE ( AdjustFileSyncLayout ( dFiles, 11, 10000, iHashes, iFloor, sError ) ) << sError.cstr();
	EXPECT_LE ( iHashes, 11 );
	EXPECT_GT ( iFloor, 2000 );
	for ( const FileSyncLayout_t & tFile : dFiles )
	{
		EXPECT_GE ( tFile.m_iChunkBytes, tFile.m_iPreferredChunkBytes );
		EXPECT_LE ( tFile.m_iChunkBytes, 10000 );
		EXPECT_LE ( tFile.m_iChunkBytes, tFile.m_iFileSize );
	}
	CheckContiguous ( dFiles, iHashes );
}


TEST ( SstHashLayout, HandlesMixedTinyEmptyAndHugeFiles )
{
	CSphVector<FileSyncLayout_t> dFiles;
	dFiles.Add ( Layout ( 0, 1 ) );
	dFiles.Add ( Layout ( 7, 7 ) );
	dFiles.Add ( Layout ( 1000000, 2048 ) );

	int64_t iHashes = 0;
	int64_t iFloor = 0;
	CSphString sError;
	ASSERT_TRUE ( AdjustFileSyncLayout ( dFiles, 20, 100000, iHashes, iFloor, sError ) ) << sError.cstr();
	EXPECT_EQ ( dFiles[0].m_iChunkBytes, 1 );
	EXPECT_EQ ( dFiles[1].m_iChunkBytes, 7 );
	EXPECT_GT ( dFiles[2].m_iChunkBytes, 2048 );
	EXPECT_LE ( iHashes, 20 );
	CheckContiguous ( dFiles, iHashes );
}


TEST ( SstHashLayout, RejectsImpossibleLayout )
{
	CSphVector<FileSyncLayout_t> dFiles;
	dFiles.Add ( Layout ( 10000, 1000 ) );
	dFiles.Add ( Layout ( 10000, 1000 ) );

	int64_t iHashes = 0;
	int64_t iFloor = 0;
	CSphString sError;
	EXPECT_FALSE ( AdjustFileSyncLayout ( dFiles, 3, 10000, iHashes, iFloor, sError ) );
	EXPECT_NE ( strstr ( sError.cstr(), "requires at least 4 hashes" ), nullptr ) << sError.cstr();
}


TEST ( SstHashLayout, CountsInt64BoundaryWithoutOverflow )
{
	CSphVector<FileSyncLayout_t> dFiles;
	dFiles.Add ( Layout ( std::numeric_limits<int64_t>::max(), 1 ) );
	EXPECT_EQ ( CountFileSyncHashes ( dFiles ), std::numeric_limits<int64_t>::max() );

	int64_t iHashes = 0;
	int64_t iFloor = 0;
	CSphString sError;
	EXPECT_FALSE ( AdjustFileSyncLayout ( dFiles, 100, 1024, iHashes, iFloor, sError ) );
}


TEST ( SstHashLayout, ManySmallFilesIncludeFullFileHashes )
{
	CSphVector<FileSyncLayout_t> dFiles;
	for ( int i = 0; i < 1000; ++i )
		dFiles.Add ( Layout ( 1, 1 ) );
	EXPECT_EQ ( CountFileSyncHashes ( dFiles ), 2000 );

	int64_t iHashes = 0;
	int64_t iFloor = 0;
	CSphString sError;
	EXPECT_FALSE ( AdjustFileSyncLayout ( dFiles, 1999, 1024, iHashes, iFloor, sError ) );
}


TEST ( SstHashLayout, FileReserveBudgetAccountsForNamesAndChunks )
{
	FixedStrVec_t dNames ( 2 );
	dNames[0] = "a";
	dNames[1] = "longer-file-name";
	CSphString sError;
	const int64_t iMaxHashes = GetFileSyncMaxHashes ( dNames, dNames.GetLength(), sError );
	const int64_t iBudget = 96LL * 1024 * 1024;
	const int64_t iNonHashBytes = 3 * sizeof ( int ) + 2 * sizeof ( FileChunks_t )
		+ sizeof ( int ) + dNames[0].Length() + sizeof ( int ) + dNames[1].Length();
	EXPECT_EQ ( iMaxHashes, ( iBudget - iNonHashBytes ) / 20 );
}


TEST ( SstHashLayout, CustomerScaleSyntheticLayoutFitsBudget )
{
	CSphVector<FileSyncLayout_t> dFiles;
	constexpr int iFiles = 1707;
	constexpr int64_t iChunksPerFile = 4440;
	for ( int i = 0; i < iFiles; ++i )
		dFiles.Add ( Layout ( iChunksPerFile * 2048, 2048 ) );

	const int64_t iBefore = CountFileSyncHashes ( dFiles );
	EXPECT_EQ ( iBefore, int64_t ( iFiles ) * ( iChunksPerFile + 1 ) );
	FixedStrVec_t dNames ( iFiles );
	for ( CSphString & sName : dNames )
		sName = "rtindexalphanumericcn_6.0.spd";
	CSphString sError;
	const int64_t iMaxHashes = GetFileSyncMaxHashes ( dNames, iFiles, sError );
	ASSERT_GT ( iMaxHashes, 0 ) << sError.cstr();
	ASSERT_GT ( iBefore, iMaxHashes );

	int64_t iHashes = 0;
	int64_t iFloor = 0;
	ASSERT_TRUE ( AdjustFileSyncLayout ( dFiles, iMaxHashes, 96LL * 1024 * 1024, iHashes, iFloor, sError ) ) << sError.cstr();
	EXPECT_LE ( iHashes, iMaxHashes );
	EXPECT_GT ( iFloor, 2048 );
	CheckContiguous ( dFiles, iHashes );
}