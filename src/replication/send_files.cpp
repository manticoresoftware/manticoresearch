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

#include "send_files.h"

#include "searchdha.h"
#include "searchdaemon.h"

#include <limits>
#include <optional>

// count of chunks for file size
int FileChunks_t::GetChunksCount () const noexcept
{
	if ( m_iFileSize )
		return int ( 1 + ( m_iFileSize - 1 ) / m_iChunkBytes );
	return 0;
}


int64_t FileChunks_t::GetChunkFileLength ( int iChunk ) const noexcept
{
	int64_t iSize = m_iChunkBytes;
	if ( iChunk == GetChunksCount() - 1 ) // calculate file tail size for last chunk
		iSize = m_iFileSize - (int64_t)m_iChunkBytes * iChunk;

	return iSize;
}


int64_t FileChunks_t::GetChunkFileOffset ( int iChunk ) const noexcept
{
	return (int64_t)m_iChunkBytes * iChunk;
}


SyncSrc_t::SyncSrc_t ( StrVec_t&& dIndexFiles )
{
	m_dIndexFiles.SwapData ( dIndexFiles );
}


HASH20_t& SyncSrc_t::GetFileHash ( int iFile ) const noexcept
{
	assert ( iFile >= 0 && iFile < m_dBaseNames.GetLength() );
	return m_dHashes[iFile];
}


HASH20_t& SyncSrc_t::GetChunkHash ( int iFile, int iChunk ) const noexcept
{
	assert ( iFile >= 0 && iFile < m_dBaseNames.GetLength() );
	assert ( iChunk >= 0 && iChunk < m_dChunks[iFile].GetChunksCount() );
	return m_dHashes[m_dChunks[iFile].m_iHashStartItem + iChunk];
}

// rsync uses sqrt ( iSize ) but that make too small buffers
constexpr int iBlockMin = 2048;

static int64_t GetLayoutChunkBytes ( const FileSyncLayout_t & tFile, int64_t iChunkFloor ) noexcept
{
	if ( !tFile.m_iFileSize )
		return 1;

	return Min ( tFile.m_iFileSize, Max ( tFile.m_iPreferredChunkBytes, iChunkFloor ) );
}


int64_t GetFileSyncMaxHashes ( const VecTraits_T<CSphString> & dBaseNames, int iFiles, CSphString & sError )
{
	constexpr int64_t iFileReserveArraysBudget = int64_t ( SPH_MAX_PACKET_SIZE ) * 3 / 4;
	int64_t iNonHashArrayBytes = 3 * sizeof ( int ) + int64_t ( iFiles ) * sizeof ( FileChunks_t );
	for ( const CSphString & sBaseName : dBaseNames )
	{
		const int64_t iSerializedNameBytes = sizeof ( int ) + sBaseName.Length();
		if ( iNonHashArrayBytes > iFileReserveArraysBudget - iSerializedNameBytes )
		{
			sError = "SST metadata is too large: FILE_RESERVE file layout exceeds the safe 96 MiB metadata budget";
			return -1;
		}
		iNonHashArrayBytes += iSerializedNameBytes;
	}
	return ( iFileReserveArraysBudget - iNonHashArrayBytes ) / sizeof ( HASH20_t );
}


int64_t CountFileSyncHashes ( const VecTraits_T<FileSyncLayout_t> & dFiles, int64_t iChunkFloor ) noexcept
{
	int64_t iHashes = dFiles.GetLength();
	for ( const FileSyncLayout_t & tFile : dFiles )
	{
		if ( tFile.m_iFileSize<0 || tFile.m_iPreferredChunkBytes<=0 )
			return std::numeric_limits<int64_t>::max();

		if ( !tFile.m_iFileSize )
			continue;

		const int64_t iChunkBytes = GetLayoutChunkBytes ( tFile, iChunkFloor );
		if ( iChunkBytes<=0 )
			return std::numeric_limits<int64_t>::max();

		const int64_t iChunks = 1 + ( tFile.m_iFileSize - 1 ) / iChunkBytes;
		if ( iHashes > std::numeric_limits<int64_t>::max() - iChunks )
			return std::numeric_limits<int64_t>::max();
		iHashes += iChunks;
	}
	return iHashes;
}


bool AdjustFileSyncLayout ( CSphVector<FileSyncLayout_t> & dFiles, int64_t iMaxHashes, int64_t iMaxChunkBytes,
	int64_t & iHashes, int64_t & iChunkFloor, CSphString & sError )
{
	iChunkFloor = 0;
	iHashes = CountFileSyncHashes ( dFiles );
	if ( iMaxHashes<=0 || iMaxChunkBytes<=0 || iHashes==std::numeric_limits<int64_t>::max() )
	{
		sError = "invalid SST file hash layout";
		return false;
	}
	for ( const FileSyncLayout_t & tFile : dFiles )
		if ( tFile.m_iPreferredChunkBytes > iMaxChunkBytes )
		{
			sError = "invalid SST file hash layout: preferred chunk exceeds the send buffer";
			return false;
		}

	if ( iHashes > iMaxHashes )
	{
		const int64_t iMinHashes = CountFileSyncHashes ( dFiles, iMaxChunkBytes );
		if ( iMinHashes > iMaxHashes )
		{
			sError.SetSprintf ( "SST metadata is too large: FILE_RESERVE requires at least " INT64_FMT
				" hashes and exceeds the safe hash budget of " INT64_FMT " hashes for the 128 MiB cluster packet limit",
				iMinHashes, iMaxHashes );
			return false;
		}

		int64_t iLeft = 1;
		int64_t iRight = iMaxChunkBytes;
		while ( iLeft < iRight )
		{
			const int64_t iMiddle = iLeft + ( iRight - iLeft ) / 2;
			if ( CountFileSyncHashes ( dFiles, iMiddle ) <= iMaxHashes )
				iRight = iMiddle;
			else
				iLeft = iMiddle + 1;
		}
		iChunkFloor = iLeft;
		iHashes = CountFileSyncHashes ( dFiles, iChunkFloor );
	}

	int64_t iHashStart = dFiles.GetLength();
	for ( FileSyncLayout_t & tFile : dFiles )
	{
		tFile.m_iChunkBytes = GetLayoutChunkBytes ( tFile, iChunkFloor );
		tFile.m_iHashStartItem = iHashStart;
		if ( tFile.m_iFileSize )
			iHashStart += 1 + ( tFile.m_iFileSize - 1 ) / tFile.m_iChunkBytes;
	}

	assert ( iHashStart==iHashes );
	return true;
}

std::optional<int> SyncSrc_t::InitSyncSrc ()
{
	TLS_MSG_STRING ( sError );

	const int iFiles = m_dIndexFiles.GetLength();
	m_dBaseNames.Reset ( iFiles );
	m_dChunks.Reset ( iFiles );

	int iMaxChunkBytes = 0;
	m_iBufferSize = (int64_t)g_iMaxPacketSize * 3 / 4;
	CSphVector<FileSyncLayout_t> dLayout;
	dLayout.Resize ( iFiles );

	for ( int i = 0; i < iFiles; ++i )
	{
		const CSphString& sFile = m_dIndexFiles[i];
		CSphAutofile tIndexFile;
		if ( tIndexFile.Open ( sFile, SPH_O_READ, sError ) < 0 )
			return std::nullopt;

		m_dBaseNames[i] = GetBaseName ( sFile );
		int64_t iFileSize = tIndexFile.GetSize();

		// int iChunkBytes = int ( iFileSize / iBlockMin ); // FIXME!!! sqrt ( iFileSize )
		// no need too small chunks
		int64_t iChunkBytes = iFileSize
			? Min ( Min ( Max ( iBlockMin, int64_t ( sqrt ( iFileSize ) ) ), iFileSize ), m_iBufferSize )
			: 1;
		assert ( iChunkBytes>0 && iChunkBytes<INT_MAX );

		FileSyncLayout_t & tLayout = dLayout[i];
		tLayout.m_iFileSize = iFileSize;
		tLayout.m_iPreferredChunkBytes = iChunkBytes;
	}

	CSphString sLayoutError;
	const int64_t iMaxHashes = GetFileSyncMaxHashes ( m_dBaseNames, iFiles, sLayoutError );
	if ( iMaxHashes<0 )
	{
		TlsMsg::Err ( sLayoutError );
		return std::nullopt;
	}
	const int64_t iOriginalHashes = CountFileSyncHashes ( dLayout );
	int64_t iHashes = 0;
	int64_t iChunkFloor = 0;
	if ( !AdjustFileSyncLayout ( dLayout, iMaxHashes, m_iBufferSize, iHashes, iChunkFloor, sLayoutError ) )
	{
		TlsMsg::Err ( sLayoutError );
		return std::nullopt;
	}

	if ( iChunkFloor )
		sphLogDebugRpl ( "SST hash metadata too large, increasing chunk size: files=%d, hashes=" INT64_FMT
			" -> " INT64_FMT ", chunk_floor=" INT64_FMT, iFiles, iOriginalHashes, iHashes, iChunkFloor );

	assert ( iHashes<=INT_MAX );
	for ( int i = 0; i < iFiles; ++i )
	{
		const FileSyncLayout_t & tLayout = dLayout[i];
		FileChunks_t & tChunk = m_dChunks[i];
		tChunk.m_iFileSize = tLayout.m_iFileSize;
		tChunk.m_iChunkBytes = (int)tLayout.m_iChunkBytes;
		tChunk.m_iHashStartItem = (int)tLayout.m_iHashStartItem;
		iMaxChunkBytes = Max ( tChunk.m_iChunkBytes, iMaxChunkBytes );
	}
	m_dHashes.Reset ( (int)iHashes );
	m_dHashes.Fill ( {} );
	return iMaxChunkBytes;
}

bool VerifyFileHash ( int iFile, const CSphString& sName, const SyncSrc_t& tSrc, CSphBitvec& tDst, CSphVector<BYTE>& dBuf, CSphString& sError )
{
	const FileChunks_t& tChunk = tSrc.m_dChunks[iFile];
	SHA1_c tHashFile;
	SHA1_c tHashChunk;
	tHashFile.Init();

	dBuf.Resize ( tChunk.m_iChunkBytes );
	BYTE* pReadData = dBuf.Begin();
	HASH20_t tFileHash {};
	HASH20_t tChunkHash {};

	CSphAutofile tIndexFile;

	if ( tIndexFile.Open ( sName, SPH_O_READ, sError ) < 0 )
		return false;

	int iChunk = 0;
	int64_t iReadTotal = 0;
	while ( iReadTotal < tChunk.m_iFileSize )
	{
		int64_t iLeft = tChunk.m_iFileSize - iReadTotal;
		iLeft = Min ( iLeft, tChunk.m_iChunkBytes );
		iReadTotal += iLeft;

		if ( !tIndexFile.Read ( pReadData, iLeft, sError ) )
			return false;

		// update whole file hash
		tHashFile.Update ( pReadData, iLeft );

		// update and flush chunk hash
		tHashChunk.Init();
		tHashChunk.Update ( pReadData, iLeft );
		tHashChunk.Final ( tChunkHash );

		if ( tChunkHash == tSrc.GetChunkHash ( iFile, iChunk ) )
			tDst.BitSet ( tChunk.m_iHashStartItem + iChunk );
		++iChunk;
	}

	tHashFile.Final ( tFileHash );

	if ( tFileHash == tSrc.GetFileHash ( iFile ) )
		tDst.BitSet ( iFile );

	return true;
}

bool SyncSigVerify ( const CSphString& sFile, const HASH20_t& dHash )
{
	CSphAutoreader tIndexFile;
	{
		TLS_MSG_STRING ( sError );
		if ( !tIndexFile.Open ( sFile, sError ) )
			return false;
	}

	SHA1_c tHashFile;
	tHashFile.Init();
	const int64_t iFileSize = tIndexFile.GetFilesize();
	int64_t iReadTotal = 0;
	while ( iReadTotal < iFileSize )
	{
		const int64_t iLeft = iFileSize - iReadTotal;

		const BYTE * pData = nullptr;
		const int64_t iGot = tIndexFile.GetBytesZerocopy ( &pData, iLeft );
		iReadTotal += iGot;

		// update whole file hash
		tHashFile.Update ( pData, iGot );
	}

	auto dNewHash = tHashFile.FinalHash();

	return dNewHash == dHash
		|| TlsMsg::Err ( "%s sha1 does not matched, expected %s, got %s", sFile.cstr(), BinToHex ( dHash ).cstr(), BinToHex ( dNewHash ).cstr() );
}

bool SyncSigVerify ( const VecTraits_T<CSphString>& dFiles, const VecTraits_T<HASH20_t>& dHashes )
{
	ARRAY_FOREACH ( iFile, dFiles )
	{
		if ( !SyncSigVerify ( dFiles[iFile], dHashes[iFile] ) )
			return false;
	}
	return true;
}

uint64_t SyncSrc_t::CalculateNeededBytes ( const CSphBitvec & dNeededChunks ) const
{
	uint64_t uTotalBytes = 0;
	if ( m_dHashes.IsEmpty() )
		return 0;

	for ( int iFile=0; iFile<m_dBaseNames.GetLength(); ++iFile )
	{
		const FileChunks_t& tFileChunks = m_dChunks[iFile];
		const int iNumChunksInFile = tFileChunks.GetChunksCount();

		for ( int iChunk=0; iChunk<iNumChunksInFile; ++iChunk )
		{
			int iCurChunk = tFileChunks.m_iHashStartItem + iChunk;
			if ( !dNeededChunks.BitGet ( iCurChunk ) )
				uTotalBytes += tFileChunks.GetChunkFileLength ( iChunk );
		}
	}
	
	return uTotalBytes;
}
