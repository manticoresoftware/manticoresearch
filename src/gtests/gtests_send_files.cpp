//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include <gtest/gtest.h>
#include "replication/send_files.h"

TEST ( SstHashLayout, PreferredLayoutFits )
{
	for ( int64_t iMaxHashes : { 9, 10 } )
	{
		SCOPED_TRACE ( iMaxHashes );
		CSphFixedVector<FileChunks_t> dFiles { 3 };
		dFiles[0] = { 0, 0, 1 };
		dFiles[1] = { 7, 0, 7 };
		dFiles[2] = { 10000, 0, 2048 };
		FileSyncLayout_t tLayout { dFiles };
		tLayout.m_iMaxHashes = iMaxHashes;
		tLayout.m_iBufferSize = 8192;

		ASSERT_TRUE ( AdjustFileSyncLayout ( tLayout ) ) << tLayout.m_sError.cstr();
		EXPECT_EQ ( tLayout.m_iHashes, 9 );
		EXPECT_EQ ( tLayout.m_iMaxChunkBytes, 2048 );
		EXPECT_EQ ( dFiles[0].m_iChunkBytes, 1 );
		EXPECT_EQ ( dFiles[1].m_iChunkBytes, 7 );
		EXPECT_EQ ( dFiles[2].m_iChunkBytes, 2048 );
		EXPECT_EQ ( dFiles[0].m_iHashStartItem, 3 );
		EXPECT_EQ ( dFiles[1].m_iHashStartItem, 3 );
		EXPECT_EQ ( dFiles[2].m_iHashStartItem, 4 );
	}
}


TEST ( SstHashLayout, MinimumFloorRetainsFittingCount )
{
	CSphFixedVector<FileChunks_t> dFiles { 1 };
	dFiles[0] = { 10000, 0, 2048 };
	FileSyncLayout_t tLayout { dFiles };
	tLayout.m_iMaxHashes = 4;
	tLayout.m_iBufferSize = 8192;

	// 3334 fits; the final probe at 3333 needs one more hash.
	ASSERT_TRUE ( AdjustFileSyncLayout ( tLayout ) ) << tLayout.m_sError.cstr();
	EXPECT_EQ ( dFiles[0].m_iChunkBytes, 3334 );
	EXPECT_EQ ( dFiles[0].m_iHashStartItem, 1 );
	EXPECT_EQ ( tLayout.m_iHashes, 4 );
	EXPECT_EQ ( tLayout.m_iMaxChunkBytes, 3334 );
}


TEST ( SstHashLayout, OnlyMaximumFloorFits )
{
	CSphFixedVector<FileChunks_t> dFiles { 1 };
	dFiles[0] = { 8192, 0, 2048 };
	FileSyncLayout_t tLayout { dFiles };
	tLayout.m_iMaxHashes = 3;
	tLayout.m_iBufferSize = 4096;

	ASSERT_TRUE ( AdjustFileSyncLayout ( tLayout ) ) << tLayout.m_sError.cstr();
	EXPECT_EQ ( dFiles[0].m_iChunkBytes, 4096 );
	EXPECT_EQ ( dFiles[0].m_iHashStartItem, 1 );
	EXPECT_EQ ( tLayout.m_iHashes, 3 );
	EXPECT_EQ ( tLayout.m_iMaxChunkBytes, 4096 );
}


TEST ( SstHashLayout, ImpossibleLayoutPreservesRecords )
{
	CSphFixedVector<FileChunks_t> dFiles { 2 };
	dFiles[0] = { 8192, 42, 2048 };
	dFiles[1] = { 7, 43, 7 };
	FileSyncLayout_t tLayout { dFiles };
	tLayout.m_iMaxHashes = 4;
	tLayout.m_iBufferSize = 4096;

	ASSERT_FALSE ( AdjustFileSyncLayout ( tLayout ) );
	EXPECT_STREQ ( tLayout.m_sError.cstr(), "SST metadata is too large: FILE_RESERVE requires at least 5 hashes and exceeds the safe hash budget of 4 hashes for the 128 MiB cluster packet limit" );
	EXPECT_EQ ( dFiles[0].m_iFileSize, 8192 );
	EXPECT_EQ ( dFiles[0].m_iChunkBytes, 2048 );
	EXPECT_EQ ( dFiles[0].m_iHashStartItem, 42 );
	EXPECT_EQ ( dFiles[1].m_iFileSize, 7 );
	EXPECT_EQ ( dFiles[1].m_iChunkBytes, 7 );
	EXPECT_EQ ( dFiles[1].m_iHashStartItem, 43 );
	EXPECT_EQ ( tLayout.m_iHashes, 0 );
	EXPECT_EQ ( tLayout.m_iMaxChunkBytes, 0 );
}


TEST ( SstHashLayout, OneHashOverBudgetKeepsMixedOffsets )
{
	CSphFixedVector<FileChunks_t> dFiles { 3 };
	dFiles[0] = { 0, 0, 1 };
	dFiles[1] = { 7, 0, 7 };
	dFiles[2] = { 10000, 0, 2048 };
	FileSyncLayout_t tLayout { dFiles };
	tLayout.m_iMaxHashes = 8;
	tLayout.m_iBufferSize = 8192;

	// Preferred layout needs nine hashes; only the large file needs adjustment.
	ASSERT_TRUE ( AdjustFileSyncLayout ( tLayout ) ) << tLayout.m_sError.cstr();
	EXPECT_EQ ( tLayout.m_iHashes, 8 );
	EXPECT_EQ ( tLayout.m_iMaxChunkBytes, 2500 );
	EXPECT_EQ ( dFiles[0].m_iChunkBytes, 1 );
	EXPECT_EQ ( dFiles[1].m_iChunkBytes, 7 );
	EXPECT_EQ ( dFiles[2].m_iChunkBytes, 2500 );
	EXPECT_EQ ( dFiles[0].m_iHashStartItem, 3 );
	EXPECT_EQ ( dFiles[1].m_iHashStartItem, 3 );
	EXPECT_EQ ( dFiles[2].m_iHashStartItem, 4 );
}
