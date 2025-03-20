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

#pragma once

#include "send_files.h"

#include "std/ints.h"
#include "std/string.h"
#include "coroutine.h"
#include "cluster_file_reserve.h"


enum class FileOp_e : BYTE {
	COPY_FILE,
	COPY_BUFFER,
	VERIFY_FILE
};

struct FileOp_t
{
	FileOp_e m_eOp { FileOp_e::COPY_FILE };
	int64_t m_iOffFile { 0 };
	int m_iOffBuf { 0 };
	int64_t m_iSize { 0 };
};

enum class WriteResult_e : BYTE {
	OK,
	WRITE_FAILED,
	VERIFY_FAILED
};

uint64_t DoubleStringKey ( const CSphString& sFirst, const CSphString& sSecond );

class RecvState_c
{
public:
	WriteResult_e Write ( int iFile, const CSphVector<FileOp_t>& dOps, const VecTraits_T<BYTE>& dBuf ) EXCLUDES ( m_tLock );
	MergeState_t* Flush () EXCLUDES ( m_tLock );
	void SetMerge ( const SyncSrc_t& tSync, const SyncDst_t& tRes, const CSphString& sIndexPath, const VecTraits_T<CSphString>& dFilesRef ) EXCLUDES ( m_tLock );

private:
	bool SetFile ( int iFile, bool bRestart ) REQUIRES ( m_tLock );
	bool CmdCopyFile ( int64_t iOff, int64_t iSize ) REQUIRES ( m_tLock );
	bool CmdCopyBuffer ( int64_t iOffFile, const VecTraits_T<BYTE>& dBuf, int iOffBuf, int64_t iSize ) REQUIRES ( m_tLock );
	WriteResult_e CmdVerifyFile ( int iFile ) REQUIRES ( m_tLock );
	bool VerifyHashWriter () REQUIRES ( m_tLock );
	void Close() REQUIRES ( m_tLock );
	bool CheckFiles ( bool bSeekReader, int64_t iOff ) REQUIRES ( m_tLock );
	[[nodiscard]] inline const char* GetFilename() const noexcept REQUIRES ( m_tLock )
	{
		return ( ( m_iFile == -1 || !m_pMerge ) ? "" : m_pMerge->m_dFilesNew[m_iFile].cstr() );
	}

private:
	Threads::Coro::Mutex_c m_tLock; // prevent writing to same file from multiple clients

	std::unique_ptr<WriterWithHash_c> m_pWriter GUARDED_BY ( m_tLock );
	std::unique_ptr<CSphAutoreader> m_pReader GUARDED_BY ( m_tLock );
	std::unique_ptr<MergeState_t> m_pMerge GUARDED_BY ( m_tLock );
	CSphString m_sError; // writer need a string to put error message there

	int m_iFile GUARDED_BY ( m_tLock ) { -1 };
	int m_bFileRestarted GUARDED_BY ( m_tLock ) { false };
};

namespace RecvState {

RecvState_c& GetState ( uint64_t tWriterKey );
void Free ( uint64_t tWriterKey );
bool HasState ( uint64_t tWriterKey );

} // RecvState