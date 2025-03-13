//
// Copyright (c) 2023-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "recv_state.h"
#include "std/fnv64.h"
#include "std/orderedhash.h"
#include "coroutine.h"
#include "send_files.h"

uint64_t DoubleStringKey ( const CSphString& sFirst, const CSphString& sSecond )
{
	return sphFNV64 ( sFirst.cstr(), sFirst.Length(), sphFNV64 ( sSecond.cstr(), sSecond.Length() ) );
}

WriteResult_e RecvState_c::Write ( int iFile, const CSphVector<FileOp_t>& dOps, const VecTraits_T<BYTE>& dBuf )
{
	Threads::ScopedCoroMutex_t tLock ( m_tLock );

	assert ( m_pMerge );

	if ( !SetFile ( iFile, false ) )
		return WriteResult_e::WRITE_FAILED;

	for ( const auto& tOp : dOps )
	{
		switch ( tOp.m_eOp )
		{
		case FileOp_e::COPY_FILE:
			if ( !CmdCopyFile ( tOp.m_iOffFile, tOp.m_iSize ) )
				return WriteResult_e::WRITE_FAILED;
			break;

		case FileOp_e::COPY_BUFFER:
			if ( !CmdCopyBuffer ( tOp.m_iOffFile, dBuf, tOp.m_iOffBuf, tOp.m_iSize ) )
				return WriteResult_e::WRITE_FAILED;
			break;

		case FileOp_e::VERIFY_FILE:
			return CmdVerifyFile ( iFile );

		default:
			TlsMsg::Err ( "unknown file %d operation %d", iFile, (int)tOp.m_eOp );
			return WriteResult_e::WRITE_FAILED;
		}
	}

	return WriteResult_e::OK;
}

MergeState_t* RecvState_c::Flush ()
{
	Threads::ScopedCoroMutex_t tLock ( m_tLock );

	assert ( m_pMerge );
	return m_pMerge.release();
}

void RecvState_c::SetMerge ( const SyncSrc_t& tSync, const SyncDst_t& tRes, const CSphString& sIndexPath, const VecTraits_T<CSphString>& dFilesRef )
{
	assert ( dFilesRef.GetLength() == tRes.m_dRemotePaths.GetLength() );

	Threads::ScopedCoroMutex_t tLock ( m_tLock );

	m_pMerge = std::make_unique<MergeState_t>();
	m_pMerge->m_sIndexPath = sIndexPath;
	m_pMerge->m_dMergeMask = tRes.m_dNodeChunksMask;
	m_pMerge->m_dFilesNew.CopyFrom ( tRes.m_dRemotePaths );
	m_pMerge->m_dFilesRef.CopyFrom ( dFilesRef );
	m_pMerge->m_dChunks.CopyFrom ( tSync.m_dChunks );
	m_pMerge->m_dHashes.CopyFrom ( tSync.m_dHashes );
}

bool RecvState_c::SetFile ( int iFile, bool bRestart )
{
	if ( !bRestart && m_iFile == iFile )
		return true;

	if ( !m_pMerge )
		return TlsMsg::Err ( "missed write state" );

	if ( iFile >= m_pMerge->m_dFilesNew.GetLength() )
		return TlsMsg::Err ( "switching disk file %d outside of the bounds %d", iFile, m_pMerge->m_dFilesNew.GetLength() );

	if ( !bRestart && m_pWriter )
		return TlsMsg::Err ( "active writer %s (%d), next  %s (%d)", GetFilename(), m_iFile, m_pMerge->m_dFilesNew[iFile].cstr(), iFile );

	sphLogDebugRpl ( "switching disk file %s (%d>%d), restart %d", m_pMerge->m_dFilesNew[iFile].cstr(), m_iFile, iFile, (int)bRestart );
	Close();
	m_bFileRestarted = bRestart;
	m_iFile = iFile;

	int iOpenFlags = ( bRestart ? ( O_CREAT | O_RDWR | SPH_O_BINARY ) : SPH_O_NEW ); // need to keep already written data
	auto pWriter = std::make_unique<WriterWithHash_c>();
	if ( !pWriter->OpenFile ( m_pMerge->m_dFilesNew[iFile], iOpenFlags, m_sError ) )
		return TlsMsg::Err ( m_sError );

	std::unique_ptr<CSphAutoreader> pReader;
	if ( sphFileExists ( m_pMerge->m_dFilesRef[iFile].cstr(), nullptr ) )
	{
		CSphString sError;
		pReader = std::make_unique<CSphAutoreader>();
		if ( !pReader->Open ( m_pMerge->m_dFilesRef[iFile], sError ) )
			return TlsMsg::Err ( sError );
	}

	m_pWriter = std::move ( pWriter );
	m_pReader = std::move ( pReader );
	return true;
}

bool RecvState_c::CmdCopyFile ( int64_t iOff, int64_t iSize )
{
	if ( !CheckFiles ( true, iOff ) )
		return false;

	while ( iSize > 0 )
	{
		const BYTE* pData = nullptr;
		int64_t iRead = m_pReader->GetBytesZerocopy ( &pData, iSize );
		if ( !iRead || m_pReader->GetErrorFlag() )
			return TlsMsg::Err ( m_pReader->GetErrorMessage() );

		m_pWriter->PutBytes ( pData, iRead );
		if ( m_pWriter->IsError() )
			return false;

		iSize -= iRead;
	}

	return true;
}

bool RecvState_c::CmdCopyBuffer ( int64_t iOffFile, const VecTraits_T<BYTE>& dBuf, int iOffBuf, int64_t iSize )
{
	if ( !CheckFiles ( false, iOffFile ) )
		return false;

	if ( iOffBuf + iSize > dBuf.GetLength() )
		return TlsMsg::Err ( "out of bounds buffer slice (offset %d, size %d, buffer size %d) on buffer copy %s (%d)", iOffBuf, (int)iSize, dBuf.GetLength(), GetFilename(), m_iFile );

	m_pWriter->PutBytes ( dBuf.Begin() + iOffBuf, iSize );
	return !m_pWriter->IsError();
}

WriteResult_e RecvState_c::CmdVerifyFile ( int iFile )
{
	if ( iFile != m_iFile )
	{
		TlsMsg::Err ( "file mismatch, active writer %s (%d), verify  %s (%d)", GetFilename(), m_iFile, m_pMerge->m_dFilesNew[iFile].cstr(), iFile );
		Close();
		return WriteResult_e::VERIFY_FAILED;
	}

	// writer writes whole file from the beginning - data will be verified from writer hash
	if ( m_pWriter && !m_bFileRestarted )
	{
		bool bVerifyOk = VerifyHashWriter ();
		Close();
		return ( bVerifyOk ? WriteResult_e::OK : WriteResult_e::VERIFY_FAILED );
	}

	// no writer or writer got restarted from offset
	// data should be verified from the disk file
	iFile = m_iFile; // Close will invalidate m_iFile
	sphLogDebugRpl ( "verify disk file %s (%d)", GetFilename(), iFile );
	Close();

	bool bVerifyOk = SyncSigVerify ( m_pMerge->m_dFilesNew[iFile], m_pMerge->m_dHashes[iFile] );
	return ( bVerifyOk ? WriteResult_e::OK : WriteResult_e::VERIFY_FAILED );
}

bool RecvState_c::VerifyHashWriter ()
{
	assert ( m_pWriter );
	assert ( m_iFile >= 0 && m_iFile < m_pMerge->m_dFilesNew.GetLength() );

	// flush data and finalize hash
	m_pWriter->CloseFile();
	if ( m_pWriter->IsError() )
		return TlsMsg::Err ( m_sError );

	auto tHashWritten = m_pWriter->GetHASHBlob();
	const auto& tHashDonor = m_pMerge->m_dHashes[m_iFile];

	return ( tHashWritten == tHashDonor ) || TlsMsg::Err ( "%s sha1 does not match, expected %s, got %s", m_pWriter->GetFilename().cstr(), BinToHex ( tHashDonor ).cstr(), BinToHex ( tHashWritten ).cstr() );
}

void RecvState_c::Close()
{
	m_pWriter = nullptr;
	m_pReader = nullptr;
	m_iFile = -1;
}

bool RecvState_c::CheckFiles ( bool bSeekReader, int64_t iOff )
{
	if ( !m_pWriter )
		return TlsMsg::Err ( "no active writer %p on data copy %s (%d)", m_pWriter.get(), GetFilename(), m_iFile );

	if ( bSeekReader && !m_pReader )
		return TlsMsg::Err ( "no reader %p on data copy %s (%d)", m_pReader.get(), GetFilename(), m_iFile );

	if ( m_pWriter->GetPos() != iOff )
	{
		sphLogDebugRpl ( "file %s (%d) restarted at offset: " INT64_FMT ", writer offset: " INT64_FMT ", reader offset:" INT64_FMT, GetFilename(), m_iFile, iOff, (int64_t)m_pWriter->GetPos(), (int64_t)( m_pReader.get() ? m_pReader->GetPos() : -1 ) );

		if ( !SetFile ( m_iFile, true ) )
			return false;

		if ( bSeekReader && !m_pReader )
			return TlsMsg::Err ( "no reader %p on data copy %s (%d)", m_pReader.get(), GetFilename(), m_iFile );

		m_pWriter->SeekTo ( iOff, false );
	}

	if ( bSeekReader && m_pReader->GetPos() != iOff )
		m_pReader->SeekTo ( iOff, 0 );

	return true;
}

class StatesCache_c
{
	CSphOrderedHash<RecvState_c, uint64_t, IdentityHash_fn, 64> m_hStates GUARDED_BY ( m_tLock );
	Threads::Coro::Mutex_c m_tLock;

public:
	RecvState_c& GetState ( uint64_t tWriterKey )
	{
		Threads::ScopedCoroMutex_t tLock ( m_tLock );
		RecvState_c* pState = m_hStates ( tWriterKey );
		if ( pState )
			return *pState;

		return m_hStates.AddUnique ( tWriterKey );
	}

	void Free ( uint64_t tWriterKey )
	{
		Threads::ScopedCoroMutex_t tLock ( m_tLock );
		m_hStates.Delete ( tWriterKey );
	}

	bool HasState ( uint64_t tWriterKey )
	{
		Threads::ScopedCoroMutex_t tLock ( m_tLock );
		return m_hStates.Exists ( tWriterKey );
	}
};

static StatesCache_c g_tRecvStates;

RecvState_c& RecvState::GetState ( uint64_t tWriterKey )
{
	return g_tRecvStates.GetState ( tWriterKey );
}

void RecvState::Free ( uint64_t tWriterKey )
{
	g_tRecvStates.Free ( tWriterKey );
}

bool RecvState::HasState ( uint64_t tWriterKey )
{
	return g_tRecvStates.HasState ( tWriterKey );
}