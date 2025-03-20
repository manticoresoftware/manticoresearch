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

#include "std/ints.h"
#include "std/string.h"
#include "std/fixedvector.h"
#include "digest_sha1.h"
#include <optional>

struct FileChunks_t
{
	int64_t m_iFileSize = 0;  // size of whole file
	int m_iHashStartItem = 0; // offset in hashes vector in HASH20_SIZE items
	int m_iChunkBytes = 0;	  // length bytes of one hash chunk in file

	// count of chunks for file size
	[[nodiscard]] int GetChunksCount () const noexcept;
	[[nodiscard]] int64_t GetChunkFileLength ( int iChunk ) const noexcept;
	[[nodiscard]] int64_t GetChunkFileOffset ( int iChunk ) const noexcept;
};

struct MergeState_t
{
	CSphBitvec m_dMergeMask;
	CSphString m_sIndexPath;
	FixedStrVec_t m_dFilesNew { 0 };
	FixedStrVec_t m_dFilesRef { 0 };
	CSphFixedVector<FileChunks_t> m_dChunks { 0 };
	CSphFixedVector<HASH20_t> m_dHashes { 0 };
};

struct SyncSrc_t
{
	// index file names (full path, file name, extension) at source node
	StrVec_t m_dIndexFiles;

	// index file names only (file name, extension) send from source node to destination
	FixedStrVec_t m_dBaseNames { 0 };
	CSphFixedVector<FileChunks_t> m_dChunks { 0 };

	// hashes of all index files
	CSphFixedVector<HASH20_t> m_dHashes { 0 };

	int64_t m_tmTimeout = 0;	 // millisecond it took to read and hash all files, used for calc agent query timeout later
	int64_t m_tmTimeoutFile = 0; // max millisecond to read or hash files, used for calc agent query timeout later

	int64_t m_iBufferSize = 0;

public:
	SyncSrc_t() = default;
	explicit SyncSrc_t ( StrVec_t&& dIndexFiles );
	[[nodiscard]] HASH20_t& GetFileHash ( int iFile ) const noexcept;
	[[nodiscard]] HASH20_t& GetChunkHash ( int iFile, int iChunk ) const noexcept;
	bool CalculateFilesSignatures ();

private:
	std::optional<int> InitSyncSrc ();
};

bool VerifyFileHash ( int iFile, const CSphString& sName, const SyncSrc_t& tSrc, CSphBitvec& tDst, CSphVector<BYTE>& dBuf, CSphString& sError );
bool SyncSigVerify ( const CSphString& sFile, const HASH20_t& dHash );
bool SyncSigVerify ( const VecTraits_T<CSphString>& dFiles, const VecTraits_T<HASH20_t>& dHashes );
