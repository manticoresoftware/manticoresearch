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
#include "fileutils.h"

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

static int64_t GetLayoutChunkBytes ( const FileChunks_t & tFile, int64_t iChunkFloor ) noexcept
{
	if ( !tFile.m_iFileSize )
		return 1;

	return Min ( tFile.m_iFileSize, Max ( int64_t ( tFile.m_iChunkBytes ), iChunkFloor ) );
}


// Returns the hash capacity after accounting for serialized file-layout arrays.
static int64_t GetFileSyncMaxHashes ( const VecTraits_T<CSphString> & dBaseNames, int iFiles, CSphString & sError )
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


// Includes one full-file hash per file and all per-chunk hashes.
static int64_t CountFileSyncHashes ( const VecTraits_T<FileChunks_t> & dFiles, int64_t iChunkFloor = 0 ) noexcept
{
	int64_t iHashes = dFiles.GetLength();
	for ( const FileChunks_t & tFile : dFiles )
	{
		if ( tFile.m_iFileSize<0 || tFile.m_iChunkBytes<=0 )
			return INT64_MAX;

		if ( !tFile.m_iFileSize )
			continue;

		const int64_t iChunkBytes = GetLayoutChunkBytes ( tFile, iChunkFloor );
		if ( iChunkBytes<=0 )
			return INT64_MAX;

		const int64_t iChunks = 1 + ( tFile.m_iFileSize - 1 ) / iChunkBytes;
		if ( iHashes > INT64_MAX - iChunks )
			return INT64_MAX;
		iHashes += iChunks;
	}
	return iHashes;
}


// Keeps preferred chunk sizes intact until the common chunk floor is chosen.
bool AdjustFileSyncLayout ( FileSyncLayout_t & tLayout )
{
	int64_t iChunkFloor = 0;
	int64_t iHashes = CountFileSyncHashes ( tLayout.m_dChunks );
	const int64_t iPreferredHashes = iHashes;
	if ( tLayout.m_iMaxHashes<=0 || tLayout.m_iBufferSize<=0 || iHashes==INT64_MAX )
	{
		tLayout.m_sError = "invalid SST file hash layout";
		return false;
	}
	if ( iHashes > tLayout.m_iMaxHashes )
	{
		int64_t iLastHashes = iHashes;
		int64_t iLeft = 1;
		int64_t iRight = tLayout.m_iBufferSize;
		while ( iLeft <= iRight )
		{
			const int64_t iMiddle = iLeft + ( iRight - iLeft ) / 2;
			const int64_t iCandidateHashes = CountFileSyncHashes ( tLayout.m_dChunks, iMiddle );
			iLastHashes = iCandidateHashes;
			if ( iCandidateHashes <= tLayout.m_iMaxHashes )
			{
				iChunkFloor = iMiddle;
				iHashes = iCandidateHashes;
				iRight = iMiddle - 1;
			}
			else if ( iMiddle < iRight )
				iLeft = iMiddle + 1;
			else
				break; // Last remaining candidate failed; avoid advancing past the upper limit.
		}
		if ( !iChunkFloor )
		{
			// With no fitting candidate, the last probe was at the buffer limit.
			tLayout.m_sError.SetSprintf ( "SST metadata is too large: FILE_RESERVE requires at least " INT64_FMT
				" hashes and exceeds the safe hash budget of " INT64_FMT " hashes for the 128 MiB cluster packet limit",
				iLastHashes, tLayout.m_iMaxHashes );
			return false;
		}
	}

	if ( iChunkFloor )
		sphLogDebugRpl ( "SST hash metadata too large, increasing chunk size: files=%d, hashes=" INT64_FMT
			" -> " INT64_FMT ", chunk_floor=" INT64_FMT, tLayout.m_dChunks.GetLength(), iPreferredHashes, iHashes, iChunkFloor );

	assert ( iHashes<=INT_MAX );
	int iReadChunkBytes = 0;
	int64_t iHashStart = tLayout.m_dChunks.GetLength();
	for ( FileChunks_t & tFile : tLayout.m_dChunks )
	{
		const int64_t iChunkBytes = GetLayoutChunkBytes ( tFile, iChunkFloor );
		assert ( iChunkBytes>0 && iChunkBytes<INT_MAX );
		assert ( iHashStart<=INT_MAX );
		tFile.m_iChunkBytes = (int)iChunkBytes;
		tFile.m_iHashStartItem = (int)iHashStart;
		if ( tFile.m_iFileSize )
			iHashStart += 1 + ( tFile.m_iFileSize - 1 ) / iChunkBytes;
		iReadChunkBytes = Max ( tFile.m_iChunkBytes, iReadChunkBytes );
	}

	assert ( iHashStart==iHashes );
	tLayout.m_iHashes = iHashes;
	tLayout.m_iMaxChunkBytes = iReadChunkBytes;
	return true;
}

std::optional<int> SyncSrc_t::InitSyncSrc ()
{
	TLS_MSG_STRING ( sError );

	const int iFiles = m_dIndexFiles.GetLength();
	m_dBaseNames.Reset ( iFiles );
	m_dChunks.Reset ( iFiles );

	m_iBufferSize = (int64_t)g_iMaxPacketSize * 3 / 4;

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
		if ( iChunkBytes > m_iBufferSize )
		{
			TlsMsg::Err ( "invalid SST file hash layout: preferred chunk exceeds the send buffer" );
			return std::nullopt;
		}
		assert ( iChunkBytes>0 && iChunkBytes<INT_MAX );

		FileChunks_t & tChunk = m_dChunks[i];
		tChunk.m_iFileSize = iFileSize;
		tChunk.m_iChunkBytes = (int)iChunkBytes;
	}

	FileSyncLayout_t tLayout { m_dChunks };
	tLayout.m_iBufferSize = m_iBufferSize;
	tLayout.m_iMaxHashes = GetFileSyncMaxHashes ( m_dBaseNames, iFiles, tLayout.m_sError );
	if ( tLayout.m_iMaxHashes<0 || !AdjustFileSyncLayout ( tLayout ) )
	{
		TlsMsg::Err ( tLayout.m_sError );
		return std::nullopt;
	}

	m_dHashes.Reset ( (int)tLayout.m_iHashes );
	m_dHashes.Fill ( {} );
	return tLayout.m_iMaxChunkBytes;
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

void RemoveFiles ( const VecTraits_T<CSphString> & dFiles )
{
	for ( const auto & sFile : dFiles )
	{
		if ( sFile.IsEmpty() || !sphFileExists ( sFile.cstr() ) )
			continue;
		if ( ::unlink ( sFile.cstr() ) )
			sphWarning ( "failed to unlink file %s, error %s (%d)", sFile.cstr(), strerrorm ( errno ), errno );
	}
}

struct ScopedFilesRemoval_t: public ISphNoncopyable
{
	explicit ScopedFilesRemoval_t ( VecTraits_T<CSphString>* pFiles )
		: m_pFiles ( pFiles )
	{}
	~ScopedFilesRemoval_t()
	{
		if ( m_pFiles )
			RemoveFiles ( *m_pFiles );
	}
	VecTraits_T<CSphString>* m_pFiles { nullptr };
};

bool ReserveFiles ( const VecTraits_T<CSphString> & dFilesRef, const VecTraits_T<FileChunks_t> & dChunks, const CSphBitvec & dMatched, StrVec_t & dFilesNew, CSphString & sError )
{
	assert ( dFilesRef.GetLength()==dChunks.GetLength() );
	dFilesNew.Resize ( dFilesRef.GetLength() );
	ScopedFilesRemoval_t tCleanup ( &dFilesNew );
	ARRAY_FOREACH ( iFile, dFilesRef )
	{
		if ( dMatched.BitGet ( iFile ) )
			continue;

		dFilesNew[iFile].SetSprintf ( "%s.new", dFilesRef[iFile].cstr() );
		const CSphString & sFile = dFilesNew[iFile];
		const FileChunks_t & tFile = dChunks[iFile];
		CSphAutofile tOut ( sFile, SPH_O_NEW, sError, false );
		if ( tOut.GetFD()<0 )
			return false;

		int64_t iLen = sphSeek ( tOut.GetFD(), tFile.m_iFileSize, SEEK_SET );
		if ( iLen!=tFile.m_iFileSize )
		{
			if ( iLen<0 )
				sError.SetSprintf ( "error: %d '%s'", errno, strerrorm ( errno ) );
			else
				sError.SetSprintf ( "error, expected: " INT64_FMT ", got " INT64_FMT, tFile.m_iFileSize, iLen );
			return false;
		}
	}
	tCleanup.m_pFiles = nullptr;
	return true;
}

RollbackFilesGuard_t::~RollbackFilesGuard_t()
{
	assert ( m_tFiles.m_dOld.GetLength() == m_tFiles.m_dRef.GetLength() );
	ARRAY_FOREACH ( i, m_tFiles.m_dOld )
	{
		if ( sph::rename ( m_tFiles.m_dOld[i].cstr(), m_tFiles.m_dRef[i].cstr() ) != 0 )
			sphWarning ( "rollback rename %s to %s failed, error %s (%d)", m_tFiles.m_dOld[i].cstr(), m_tFiles.m_dRef[i].cstr(), strerrorm ( errno ), errno );
	}
}

void RemoveStagedFiles ( const MergeState_t & tMerge )
{
	StrVec_t dRemove;
	ARRAY_FOREACH ( iFile, tMerge.m_dFilesNew )
		if ( !tMerge.m_dMergeMask.BitGet ( iFile ) )
			dRemove.Add ( tMerge.m_dFilesNew[iFile] );
	RemoveFiles ( dRemove );
}

bool RotateFiles ( const MergeState_t & tMerge, FilesTrait_t & tFilesGuard, CSphString & sError )
{
	int iMatched = 0;
	for ( int i = 0; i < tMerge.m_dFilesNew.GetLength(); i++ )
		iMatched += ( tMerge.m_dMergeMask.BitGet ( i ) ? 1 : 0 );

	// all matched files - no .new files these should be rotated at joiner
	// using joiner files as is
	if ( iMatched == tMerge.m_dFilesNew.GetLength() )
		return true;

	// rename index files to old
	StrVec_t dFilesRef2Old;
	StrVec_t dFilesOld;

	// rename new to index files
	StrVec_t dFilesNew;
	StrVec_t dFilesNew2Ref;

	for ( int iFile = 0; iFile < tMerge.m_dFilesNew.GetLength(); iFile++ )
	{
		// keep joiner file as is if whole file matched
		if ( tMerge.m_dMergeMask.BitGet ( iFile ) )
			continue;

		const CSphString& sNameNew = tMerge.m_dFilesNew[iFile];
		const CSphString& sNameRef = tMerge.m_dFilesRef[iFile];

		assert ( sphFileExists ( sNameNew.cstr() ) );
		dFilesNew.Add ( sNameNew );
		dFilesNew2Ref.Add ( sNameRef );

		if ( sphFileExists ( sNameRef.cstr(), nullptr ) )
		{
			dFilesRef2Old.Add ( sNameRef );
			dFilesOld.Add().SetSprintf ( "%s.old", sNameRef.cstr() );
		}
	}

	if ( !RenameWithRollback ( dFilesRef2Old, dFilesOld, sError ) )
		return false;

	if ( !RenameWithRollback ( dFilesNew, dFilesNew2Ref, sError ) )
	{
		CSphString sRollbackError;
		RenameFiles ( dFilesOld, dFilesRef2Old, sRollbackError );
		return false;
	}

	tFilesGuard.m_dOld.SwapData ( dFilesOld );
	tFilesGuard.m_dRef.SwapData ( dFilesRef2Old );

	return true;
}
