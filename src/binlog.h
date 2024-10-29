//
// Copyright (c) 2021-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "binlog_defs.h"
#include "sphinxint.h"

namespace Binlog {

	using ProgressCallbackSimple_t = void();

	enum Txn_e : BYTE
	{
		UPDATE_ATTRS,
		COMMIT,
		PQ_ADD_DELETE,
	};

	enum ReplayFlags_e
	{
		REPLAY_ACCEPT_DESC_TIMESTAMP = 1,
		REPLAY_IGNORE_OPEN_ERROR = 2,
		REPLAY_IGNORE_TRX_ERROR = 4,

		REPLAY_IGNORE_ALL_ERRORS = 0xFF
	};

	using FnWriteCommit = std::function<void (Writer_i&)>;

	template < typename T >
	static void SaveVector ( Writer_i & tWriter, const VecTraits_T<T> &tVector )
	{
		STATIC_ASSERT ( IS_TRIVIALLY_COPYABLE (T), NON_TRIVIAL_VECTORS_ARE_UNSERIALIZABLE );
		tWriter.ZipOffset ( tVector.GetLength() );
		if ( tVector.GetLength() )
			tWriter.PutBytes ( tVector.Begin(), tVector.GetLengthBytes() );
	}

	template < typename T, typename P >
	static bool LoadVector ( CSphReader & tReader, CSphVector <T,P> & tVector )
	{
		STATIC_ASSERT ( IS_TRIVIALLY_COPYABLE(T), NON_TRIVIAL_VECTORS_ARE_UNSERIALIZABLE );
		tVector.Resize ( (int) tReader.UnzipOffset() ); // FIXME? sanitize?
		if ( tVector.GetLength() )
			tReader.GetBytes ( tVector.Begin(), (int) tVector.GetLengthBytes() );
		return !tReader.GetErrorFlag();
	}

	void Init ( CSphString sBinlogPath );
	void Configure ( const CSphConfigSection & hSearchd, DWORD uReplayFlags );
	void SetCommon ( bool bCommonBinlog );
	void Deinit ();
	bool IsActive();
	bool MockDisabled ( bool bNewVal );

	bool IsFlushEnabled();
	void Flush();
	int64_t NextFlushTimestamp();

	bool Commit ( int64_t * pTID, const char* szIndexName, CSphString & sError, FnWriteCommit && fnSaver );

	/// replay stored binlog
	void Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, ProgressCallbackSimple_t * pfnProgressCallback = nullptr );

	enum Shutdown_e : bool { NoShutdown=false, Shutdown };
	enum ForceSave_e : BYTE { NoSave=0, ForceSave, DropTable };
	void NotifyIndexFlush ( int64_t iTID, const char* szIndexName, Shutdown_e eShutdown, ForceSave_e eForceSave );

	CSphString GetPath();

	int64_t LastTidFor ( const CSphString & sIndex );
}
