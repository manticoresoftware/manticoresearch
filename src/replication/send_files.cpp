//
// Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)
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

#include <optional>

// count of chunks for file size
int FileChunks_t::GetChunksCount () const noexcept
{
	if ( m_iFileSize )
		return int ( ( m_iFileSize + m_iChunkBytes - 1 ) / m_iChunkBytes );
	return 0;
}


int FileChunks_t::GetChunkFileLength ( int iChunk ) const noexcept
{
	int iSize = m_iChunkBytes;
	if ( iChunk == GetChunksCount() - 1 ) // calculate file tail size for last chunk
		iSize = m_iFileSize - m_iChunkBytes * iChunk;

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

std::optional<int> SyncSrc_t::InitSyncSrc ()
{
	TLS_MSG_STRING ( sError );

	const int iFiles = m_dIndexFiles.GetLength();
	m_dBaseNames.Reset ( iFiles );
	m_dChunks.Reset ( iFiles );

	int iMaxChunkBytes = 0;
	m_iBufferSize = g_iMaxPacketSize * 3 / 4;
	int iHashes = iFiles;

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
		auto iChunkBytes = Min (
			Min (
				Max ( iBlockMin, int ( sqrt ( iFileSize ) ) ),
				(int)iFileSize ),
			m_iBufferSize );

		FileChunks_t& tChunk = m_dChunks[i];
		tChunk.m_iFileSize = iFileSize;
		tChunk.m_iChunkBytes = iChunkBytes;
		tChunk.m_iHashStartItem = iHashes;

		iHashes += tChunk.GetChunksCount();
		iMaxChunkBytes = Max ( tChunk.m_iChunkBytes, iMaxChunkBytes );
	}
	m_dHashes.Reset ( iHashes );
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
		int iLeft = tChunk.m_iFileSize - iReadTotal;
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
		const int iLeft = iFileSize - iReadTotal;

		// iGot is always less INT_MAX as CSphReader allocates buffer of int size max
		const BYTE* pData = nullptr;
		const int iGot = tIndexFile.GetBytesZerocopy ( &pData, iLeft );
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