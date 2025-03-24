//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "binlog.h"
#include "sphinxrt.h"
#include "memio.h"
#include "coroutine.h"

static constexpr int BINLOG_WRITE_BUFFER = 256*1024;
static constexpr int64_t BINLOG_AUTO_FLUSH = 1000000; // 1 sec

static constexpr DWORD		BINLOG_HEADER_MAGIC_SPBL = 0x4c425053;	/// magic 'SPBL' header that marks binlog file
static constexpr DWORD		BLOP_MAGIC_TXN_ = 0x214e5854;			/// magic 'TXN!' header that marks binlog entry
static constexpr DWORD		BINLOG_META_MAGIC_SPLI = 0x494c5053;	/// magic 'SPLI' header that marks binlog meta

// up to 12: PQ_ADD_DELETE added
// 13 : changed txn format; now stores total documents also
// 14 : ??
// 15 : big refactor: remove external ops; ops is 1 byte (unzipped); + internal ops, + size for ADD_TXN, - index ID

constexpr unsigned int BINLOG_VERSION = 15;

/// Bin Log Operation
enum Blop_e : BYTE
{
	ADD_INDEX = 1,
	ADD_TXN = 2,
	ADD_CACHE = 3,

	TOTAL
};

//////////////////////////////////////////////////////////////////////////
// BINLOG
//////////////////////////////////////////////////////////////////////////

/// binlog file view of the index
/// everything that a given log file needs to know about an index
struct BinlogIndexInfo_t
{
	CSphString	m_sName;
	int64_t		m_iMinTID = INT64_MAX;	///< min TID logged by this file
	int64_t		m_iMaxTID = 0;			///< max TID logged by this file
	int64_t		m_iFlushedTID = 0;		///< last flushed TID
};

struct BinlogIndexReplayInfo_t : BinlogIndexInfo_t
{
	CSphIndex * m_pIndex = nullptr;  ///< associated index (might be NULL if we don't serve it anymore!)
	int64_t m_iPreReplayTID = 0;     ///< index TID at the beginning of this file replay
};

/// binlog file descriptor
/// file id (aka extension), plus a list of associated index infos
struct BinlogFileDesc_t
{
	int								m_iExt = 0;
	CSphVector<BinlogIndexInfo_t>	m_dIndexInfos;
};

struct BinlogReplayFileDesc_t
{
	int									m_iExt;
	CSphVector<BinlogIndexReplayInfo_t> m_dIndexInfos;
};


class BinlogWriter_c final : public MemoryWriter2_c
{
public:
					BinlogWriter_c() : MemoryWriter2_c(m_dBuf) {}
					~BinlogWriter_c() final { CloseFile(); }

	bool			Write ( bool bRemoveUnsuccessful = true );
	bool			WriteAndFsync();
	bool			Fsync ( int iFD = -1 );

	int64_t			GetFilePos() const noexcept			{ return m_iLastFilePos; }

	bool			OpenFile ( const CSphString & sFile, CSphString & sError );
	void			CloseFile();
	int				LeakFD();
	int				GetFD () const noexcept;
	CSphString		GetFilename() const noexcept;
	const CSphString & GetError() const			{ return m_sError; }

	bool			HasUnwrittenData () const	{ return !m_dBuf.IsEmpty(); }
	bool			HasUnsyncedData () const	{ return m_iLastFsyncPos.load(std::memory_order_relaxed)!=m_iLastFilePos; }

	bool			IsOpen() const noexcept { return ( m_tFile.GetFD()!=-1 ); }


private:
	friend class BinlogTransactionGuard_c;
	friend class TransactionSizeGuard_c;

	void StartTransaction ();
	void EndTransaction ( int64_t iStartTransaction, bool bWriteOnOverflow );
	inline int64_t GetBuffPos () const noexcept { return m_dBuf.GetLengthBytes64 (); }

private:
	CSphAutofile	m_tFile;
	CSphVector<BYTE> m_dBuf;
	CSphString		m_sError;

	int64_t			m_iLastFilePos = 0;
	std::atomic<int64_t> m_iLastFsyncPos { 0 };
	int				m_iLastTransactionStartPos = 0;
};


class BinlogReader_c final : public CSphAutoreader
{
public:
	void			ResetCrc ();
	bool			CheckCrc ( const char * sOp, const char * sIndexName, int64_t iTid, int64_t iTxnPos );

private:
	DWORD			m_uCRC = 0;
	int				m_iLastCrcPos = m_iBuffPos;
	void			UpdateCache () final;
	void			HashCollected ();
};

using namespace Binlog;
class Binlog_c;

enum class FlushAction_e : BYTE
{
	ACTION_NONE = 0,
	ACTION_FSYNC = 1,
	ACTION_WRITE = 2,
	ACTION_FSYNC_ON_CLOSE = 3,
};

enum class BinlogFileState_e
{
	OK,
	ERROR_NON_READABLE, // if can't open binlog file
	ERROR_EMPTY_0,		// if file is empty (0 bytes)
	ERROR_WRONG_FILE,	// file is broken
	ERROR_EMPTY_8,		// file is valid, but no txns
	ERROR_ABANDONED		// file is valid, but only cache, no txns
};

using BinlogMutex_t = Threads::Coro::Mutex_c;
using ScopedBinlogMutex_t = Threads::ScopedCoroMutex_t;

class SingleBinlog_c final : public ISphRefcountedMT
{
	Binlog_c * m_pOwner;

	mutable BinlogMutex_t m_tWriteAccess;      // serialize ops
	BinlogWriter_c m_tWriter GUARDED_BY ( m_tWriteAccess );
	mutable Threads::Coro::RWLock_c m_tLogFilesAccess;
	CSphVector<BinlogFileDesc_t> m_dLogFiles GUARDED_BY ( m_tLogFilesAccess ); // active log files
	Threads::Coro::Waitable_T<int> m_tFlushRunning { 0 };

	~SingleBinlog_c () final;
public:
	NONCOPYMOVABLE( SingleBinlog_c );

	explicit SingleBinlog_c ( Binlog_c* pOwner ) noexcept
		: m_pOwner { pOwner } {}

	void Deinit();

	std::pair<int, CSphRefcountedPtr<SingleBinlog_c>> DoFlush ( FlushAction_e eAction ) EXCLUDES ( m_tWriteAccess );
	void DoFsync ( int iFd );
	void CollectBinlogFiles ( CSphVector<int>& tOutput ) const noexcept EXCLUDES ( m_tLogFilesAccess );
	bool BinlogCommit ( int64_t * pTID, const char* szIndexName, FnWriteCommit fnSaver, CSphString & sError ) EXCLUDES ( m_tWriteAccess );
	bool NotifyIndexFlush ( int64_t iFlushedTID, const char * szIndexName, Shutdown_e eShutdown, ForceSave_e eForceSave ) EXCLUDES ( m_tWriteAccess );
	void AdoptIndex ( int iExt, const BinlogIndexInfo_t & tIdx ) REQUIRES ( m_tLogFilesAccess );
	void AdoptFile ( int iExt ) REQUIRES ( m_tLogFilesAccess );
	void OpenNewLog ( BinlogFileState_e eState = BinlogFileState_e::OK ) REQUIRES ( m_tWriteAccess );
	int64_t LastTidFor ( const CSphString & sIndex ) const noexcept EXCLUDES ( m_tLogFilesAccess );

	void LockWriter () ACQUIRE ( m_tWriteAccess );
	void UnlockWriter () RELEASE ( m_tWriteAccess );

private:
	void RemoveLastEmptyLog () EXCLUDES ( m_tLogFilesAccess );
	int GetWriteIndexID ( const char * szIndexName, int64_t iTID ) EXCLUDES ( m_tLogFilesAccess ) REQUIRES ( m_tWriteAccess );
	void DoCacheWrite () EXCLUDES ( m_tLogFilesAccess ) REQUIRES ( m_tWriteAccess );
	void CheckDoRestart () REQUIRES ( m_tWriteAccess );
	bool CheckDoFlush () REQUIRES ( m_tWriteAccess );
	void SaveMeta () EXCLUDES ( m_tLogFilesAccess ) ;
	void FixNofFiles ( int iFiles ) const;
};

using SingleBinlogPtr = CSphRefcountedPtr<SingleBinlog_c>;

class Binlog_c : public ISphNoncopyable
{
	friend class SingleBinlog_c;
public:
			~Binlog_c();

	void	NotifyIndexFlush ( int64_t iTID, const char * szIndexName, Shutdown_e eShutdown, ForceSave_e eForceSave );
	bool	BinlogCommit ( int64_t * pTID, const char * szIndexName, FnWriteCommit fnSaver, CSphString & sError );

	void	Configure ( const CSphConfigSection & hSearchd, DWORD uReplayFlags );
	void	SetCommon ( bool bCommonBinlog );
	void	Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, ProgressCallbackSimple_t * pfnProgressCallback );

	bool	IsActive () const { return !m_bDisabled; }
	bool 	MockDisabled ( bool bNewVal );
	void	CheckAndSetPath ( CSphString sBinlogPath );

	bool	IsFlushingEnabled() const;
	void	DoFlush (); // invoked by task binlog flush, every BINLOG_AUTO_FLUSH (1 sec)
	int64_t	NextFlushingTime() const noexcept;

	inline CSphString GetLogPath() const noexcept { return m_sLogPath; }
	int64_t LastTidFor ( const CSphString & sIndex ) const noexcept EXCLUDES ( m_tHashAccess );

private:
	std::atomic<int64_t>	m_iLastFlushed {0};
	int64_t					m_iFlushPeriod = BINLOG_AUTO_FLUSH;

	FlushAction_e			m_eFlushFlavour = FlushAction_e::ACTION_NONE;

	mutable Threads::Coro::RWLock_c m_tHashAccess;
	SmallStringHash_T<SingleBinlogPtr> m_hBinlogs GUARDED_BY ( m_tHashAccess );

	int						m_iLockFD = -1;

	CSphString				m_sLogPath;

	IntVec_t		 		m_dSavedFiles;

	mutable Threads::Coro::RWLock_c m_tCurrentFilesAccess;
	IntVec_t				m_dCurrentFiles GUARDED_BY ( m_tCurrentFilesAccess );

	bool					m_bReplayMode = false; // replay mode indicator
	bool					m_bDisabled = true;
	DWORD					m_uReplayFlags = 0;
	bool					m_bWrongVersion = false;
	bool					m_bCommonBinlog = false; // per-index binlog(false), or old-way common binlog(true)

	int						m_iRestartSize = 268435456; // binlog size restart threshold, 256M; searchd.binlog_max_log_size
	int 					m_iBinlogFileDigits = 4;	// how many digits use for naming binlog files
	CSphString				m_sBinlogFileNameTemplate;

	std::atomic<int>		m_iNextBinlog { 0 };
	std::atomic<int>		m_iNumFiles;

private:

	SingleBinlog_c *		GetWriteIndexBinlog ( const char* szIndexName, bool bOpenNewLog=true ) EXCLUDES ( m_tHashAccess );
	SingleBinlog_c * 		GetSingleWriteIndexBinlog ( bool bOpenNewLog ) EXCLUDES ( m_tHashAccess );

	SingleBinlog_c *		GetFlushIndexBinlog ( const char * szIndexName ) const REQUIRES ( m_tHashAccess );

	void					LoadMeta ();
	enum SaveMeta_e : bool { eNoForce, eForce };
	void					DoSaveMeta ( IntVec_t dFiles, SaveMeta_e eForce ) EXCLUDES ( m_tCurrentFilesAccess );
	void					SaveMeta () EXCLUDES ( m_tHashAccess ) EXCLUDES ( m_tCurrentFilesAccess );
	void					SaveMetaUnlock ( SaveMeta_e eForce = eNoForce ) REQUIRES_SHARED ( m_tHashAccess ) EXCLUDES ( m_tCurrentFilesAccess );
	IntVec_t				CollectBinlogFiles() EXCLUDES ( m_tHashAccess );
	IntVec_t				CollectBinlogFilesUnlock() REQUIRES_SHARED ( m_tHashAccess );
	bool					CompareCurrentFiles ( const IntVec_t & dFiles ) EXCLUDES ( m_tCurrentFilesAccess );

	void					LockBinlog ();
	void					UnlockBinlog ();
	void					RemoveFile ( int iExt ) const;

	BinlogFileState_e		ReplayBinlog ( BinlogReplayFileDesc_t & tLog, const SmallStringHash_T<CSphIndex*> & hIndexes );
	bool					ReplayTxn ( const BinlogReplayFileDesc_t & tLog, BinlogReader_c & tReader ) const;
	bool					ReplayIndexAdd ( BinlogReplayFileDesc_t & tLog, const SmallStringHash_T<CSphIndex*> & hIndexes, BinlogReader_c & tReader ) const;
	bool					ReplayCacheAdd ( const BinlogReplayFileDesc_t & tLog, DWORD uVersion, BinlogReader_c & tReader ) const;
	bool 					IsBinlogWritable () const noexcept;

	bool	PerformChecks ( const char * szOp, BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t iTxnPos, BinlogReader_c & tReader ) const;

	void	Log ( DWORD uFlag, const char * sTemplate, ... ) const;
	int		ReplayIndexID ( CSphReader & tReader, const BinlogReplayFileDesc_t & tLog ) const;
	CSphString MakeBinlogName ( int iExt ) const noexcept;
	void MakeBinlogFilenameTemplate () noexcept;

	int NextBinlogExt();
	void FixNofFiles ( int iFiles );
};

std::unique_ptr<Binlog_c>		g_pRtBinlog;


inline const char * SzTxnName ( Txn_e eTxn )
{
	switch ( eTxn )
	{
	case UPDATE_ATTRS: return "update";
	case COMMIT: return "commit";
	case PQ_ADD_DELETE: return "pq_add_delete";
	}
}

//////////////////////////////////////////////////////////////////////////
// BinlogTransactionGuard_c
//////////////////////////////////////////////////////////////////////////

class BinlogTransactionGuard_c final
{
public:
	NONCOPYMOVABLE ( BinlogTransactionGuard_c );

	explicit BinlogTransactionGuard_c ( BinlogWriter_c & tWriter, bool bWriteOnOverflow = true )
		: m_tWriter ( tWriter )
		, m_bWriteOnOverflow ( bWriteOnOverflow )
		, m_iStartTransaction ( tWriter.GetBuffPos() )
	{
		m_tWriter.StartTransaction();
	}

	~BinlogTransactionGuard_c()
	{
		m_tWriter.EndTransaction ( m_iStartTransaction+sizeof ( BLOP_MAGIC_TXN_ ), m_bWriteOnOverflow );
	}

private:
	BinlogWriter_c &	m_tWriter;
	bool				m_bWriteOnOverflow;
	int64_t				m_iStartTransaction;
};

class TransactionSizeGuard_c final
{
public:
	NONCOPYMOVABLE ( TransactionSizeGuard_c );


	explicit TransactionSizeGuard_c ( BinlogWriter_c & tWriter )
			: m_tWriter ( tWriter )
			, m_iStartTransaction ( tWriter.GetBuffPos () )
	{
		m_tWriter.PutDword ( 0 ); // placeholder for size
	}


	~TransactionSizeGuard_c ()
	{
		DWORD uSize = m_tWriter.GetBuffPos ()-m_iStartTransaction-sizeof ( DWORD );
		memcpy ( &m_tWriter.m_dBuf[m_iStartTransaction], &uSize, sizeof ( DWORD ) );
	}


private:
	BinlogWriter_c & m_tWriter;
	int64_t m_iStartTransaction;
};

//////////////////////////////////////////////////////////////////////////
// BinlogWriter_c
//////////////////////////////////////////////////////////////////////////

bool BinlogWriter_c::Write ( bool bRemoveUnsuccessful )
{
	if ( m_dBuf.IsEmpty() )
		return true;

	if ( !WriteNonThrottled ( m_tFile.GetFD(), m_dBuf.Begin(), m_dBuf.GetLength(), m_tFile.GetFilename(), m_sError ) )
	{
		// if we got a partial write, clamp the file at the end of last written transaction
		sphSeek ( m_tFile.GetFD(), m_iLastFilePos, SEEK_SET );
		sphTruncate ( m_tFile.GetFD() );

		if ( bRemoveUnsuccessful )
		{
			// remove last transaction from memory buffer
			// other unwritten transactions may still be in the buffer, but we can't tell the daemon that they failed at this point,
			// so we remove only the last one
			m_dBuf.Resize ( m_iLastTransactionStartPos );
		}

		return false;
	}

	m_iLastFilePos += m_dBuf.GetLength ();
	m_iLastTransactionStartPos = 0;
	m_dBuf.Resize(0);
	return true;
}

#if _WIN32
int fsync ( int iFD )
{
	// map fd to handle
	HANDLE h = (HANDLE) _get_osfhandle ( iFD );
	if ( h==INVALID_HANDLE_VALUE )
	{
		errno = EBADF;
		return -1;
	}

	// do flush
	if ( FlushFileBuffers(h) )
		return 0;

	// error handling
	errno = EIO;
	if ( GetLastError()==ERROR_INVALID_HANDLE )
		errno = EINVAL;
	return -1;
}
#endif


bool BinlogWriter_c::Fsync (int iFD)
{
	if ( iFD==-1 )
		iFD = m_tFile.GetFD ();

	if ( fsync ( iFD )!=0 )
	{
		m_sError.SetSprintf ( "failed to sync %s: %s", m_tFile.GetFilename (), strerrorm ( errno ) );
		return false;
	}

	if ( m_iLastFsyncPos<m_iLastFilePos )
		m_iLastFsyncPos.store ( m_iLastFilePos, std::memory_order_relaxed );
	return true;
}

bool BinlogWriter_c::WriteAndFsync()
{
	if ( HasUnwrittenData() && !Write() )
		return false;

	if ( !HasUnsyncedData() )
		return true;

	return Fsync();
}


bool BinlogWriter_c::OpenFile ( const CSphString & sFile, CSphString & sError )
{
	m_iLastFilePos = 0;
	return m_tFile.Open ( sFile, SPH_O_NEW, sError )>=0;
}


void BinlogWriter_c::CloseFile()
{
	if ( HasUnwrittenData() )
		Write();

	m_tFile.Close();
	m_iLastFsyncPos.store ( m_iLastFilePos = 0, std::memory_order_relaxed ); // that is =, not ==
}

int BinlogWriter_c::LeakFD()
{
	if ( HasUnwrittenData () )
		Write ();

	m_iLastFsyncPos.store ( m_iLastFilePos = 0, std::memory_order_relaxed ); // that is =, not ==
	return m_tFile.LeakID ();
}


int BinlogWriter_c::GetFD () const noexcept
{
	return m_tFile.GetFD();
}


CSphString BinlogWriter_c::GetFilename () const noexcept
{
	return m_tFile.GetFilename();
}


void BinlogWriter_c::StartTransaction ()
{
	m_iLastTransactionStartPos = m_dBuf.GetLength ();
	PutDword ( BLOP_MAGIC_TXN_ );
}


void BinlogWriter_c::EndTransaction ( int64_t iStartTransaction, bool bWriteOnOverflow )
{
	auto uCRC = sphCRC32 ( m_dBuf.Slice ( iStartTransaction ) );
	PutDword ( uCRC );

	// try to write if buffer gets too large but don't handle write errors just yet
	// also, don't remove unsuccessful transactions from the buffer
	if ( bWriteOnOverflow && m_dBuf.GetLength ()>BINLOG_WRITE_BUFFER )
		Write ( false );
}


//////////////////////////////////////////////////////////////////////////
// BinlogReader_c
//////////////////////////////////////////////////////////////////////////

void BinlogReader_c::ResetCrc ()
{
	m_uCRC = 0;
	m_iLastCrcPos = m_iBuffPos;
}


bool BinlogReader_c::CheckCrc ( const char * sOp, const char * sIndexName, int64_t iTid, int64_t iTxnPos )
{
	HashCollected ();
	DWORD uCRC = m_uCRC;
	DWORD uRefCRC = CSphAutoreader::GetDword();
	ResetCrc();
	bool bPassed = ( uRefCRC==uCRC );
	if ( !bPassed )
		sphWarning ( "binlog: %s: CRC mismatch (table=%s, tid=" INT64_FMT ", pos=" INT64_FMT ")", sOp, sIndexName ? sIndexName : "", iTid, iTxnPos );
	return bPassed;
}

void BinlogReader_c::UpdateCache ()
{
	HashCollected();
	CSphAutoreader::UpdateCache();
	m_iLastCrcPos = m_iBuffPos;
}

void BinlogReader_c::HashCollected ()
{
	assert ( m_iLastCrcPos<=m_iBuffPos );
	m_uCRC = sphCRC32 ( m_pBuff + m_iLastCrcPos, m_iBuffPos - m_iLastCrcPos, m_uCRC );
	m_iLastCrcPos = m_iBuffPos;
}

//////////////////////////////////////////////////////////////////////////
SingleBinlog_c::~SingleBinlog_c ()
{
	Deinit ();
}


void SingleBinlog_c::Deinit () NO_THREAD_SAFETY_ANALYSIS
{
	if ( !m_pOwner )
		return;

	bool bLastLogEmpty = false;
	if ( !m_dLogFiles.IsEmpty () && m_tWriter.IsOpen () )
		bLastLogEmpty = m_dLogFiles.Last ().m_dIndexInfos.IsEmpty ();

	// could be already closed and meta saved on shutdown
	if ( m_tWriter.IsOpen () )
	{
		DoCacheWrite ();
		m_tWriter.WriteAndFsync ();
	}
	m_tWriter.CloseFile ();

	// should remove last binlog if no tnx was writen
	if ( bLastLogEmpty )
		RemoveLastEmptyLog ();

	m_pOwner = nullptr;
}


std::pair<int, CSphRefcountedPtr<SingleBinlog_c>> SingleBinlog_c::DoFlush ( FlushAction_e eAction )
{
	MEMORY ( MEM_BINLOG );

	ScopedBinlogMutex_t tLock ( m_tWriteAccess );
	std::pair<int, CSphRefcountedPtr<SingleBinlog_c>> tRes { -1, this };
	AddRef();
	if ( eAction==FlushAction_e::ACTION_NONE && m_tWriter.HasUnwrittenData () )
	{
		if ( !m_tWriter.Write () )
			return tRes;
	}
	if ( !m_tWriter.HasUnsyncedData () )
		return tRes;

	tRes.first = m_tWriter.GetFD ();
	if ( tRes.first != -1 )
		m_tFlushRunning.ModifyValue ( [] ( int & iVal ) { ++iVal; } );
	return tRes;
}


void SingleBinlog_c::DoFsync ( int iSyncFd ) NO_THREAD_SAFETY_ANALYSIS
{
	if ( iSyncFd==-1 )
		return;
	m_tWriter.Fsync ( iSyncFd );
	m_tFlushRunning.ModifyValueAndNotifyAll ( [] ( int & iVal ) { --iVal; } );
}


void SingleBinlog_c::CollectBinlogFiles ( CSphVector<int> & tOutput ) const noexcept
{
	Threads::SccRL_t tLock ( m_tLogFilesAccess );
	for ( const auto & tLog: m_dLogFiles )
		tOutput.Add ( tLog.m_iExt );
}


bool SingleBinlog_c::BinlogCommit ( int64_t * pTID, const char * szIndexName, FnWriteCommit fnSaver, CSphString & sError )
{
	MEMORY ( MEM_BINLOG );
	ScopedBinlogMutex_t tLock ( m_tWriteAccess );

	int64_t iTID = ++( *pTID );
	const int uIndex = GetWriteIndexID ( szIndexName, iTID );

	{
		BinlogTransactionGuard_c tGuard ( m_tWriter, m_pOwner->m_eFlushFlavour==FlushAction_e::ACTION_NONE );

		// header
		m_tWriter.PutByte ( Blop_e::ADD_TXN );
		m_tWriter.ZipOffset ( uIndex );
		m_tWriter.ZipOffset ( iTID );
		TransactionSizeGuard_c tPutSize ( m_tWriter );

		// save txn data
		fnSaver ( m_tWriter );
	}

	// finalize
	if ( !CheckDoFlush () )
	{
		sError.SetSprintf ( "unable to write to binlog: %s", m_tWriter.GetError ().cstr () );
		return false;
	}

	CheckDoRestart ();
	return true;
}


bool SingleBinlog_c::NotifyIndexFlush ( int64_t iFlushedTID, const char * szFlushedIndexName, Shutdown_e eShutdown, ForceSave_e eAction )
{
	MEMORY ( MEM_BINLOG );
	ScopedBinlogMutex_t tLock ( m_tWriteAccess );

	bool bCurrentLogAbandoned = false;
	int iPreflushFiles = 0;
	int iFinalFiles = 0;

	{
		Threads::SccWL_t tLogLock ( m_tLogFilesAccess );
		assert ( eShutdown || m_dLogFiles.GetLength () );

		iPreflushFiles = m_dLogFiles.GetLength ();

		// loop through all log files, and check if we can unlink any
		ARRAY_FOREACH ( iLog, m_dLogFiles )
		{
			BinlogFileDesc_t & tLog = m_dLogFiles[iLog];
			bool bUsed = false;

			// update index info for this log file
			for ( auto & tIndex: tLog.m_dIndexInfos )
			{
				// this index was just flushed, update flushed TID
				if ( tIndex.m_sName==szFlushedIndexName )
				{
					assert ( iFlushedTID>=tIndex.m_iFlushedTID );
					tIndex.m_iFlushedTID = Max ( tIndex.m_iFlushedTID, iFlushedTID );
				}

				// if max logged TID is greater than last flushed TID, log file still has needed recovery data
				if ( tIndex.m_iFlushedTID<tIndex.m_iMaxTID )
					bUsed = true;
			}

			// it's needed, keep looking
			if ( bUsed )
				continue;

			// hooray, we can remove this log!
			// if this is our current log, we have to close it first
			if ( iLog==m_dLogFiles.GetLength ()-1 )
			{
				m_tWriter.CloseFile ();
				bCurrentLogAbandoned = true;
			}

			// do unlink
			CSphString sLog = m_pOwner->MakeBinlogName ( tLog.m_iExt );
			if ( ::unlink ( sLog.cstr () ) )
				sphWarning ( "binlog: failed to unlink %s: %s", sLog.cstr (), strerrorm ( errno ) );

			// we need to reset it, otherwise there might be leftover data after last Remove()
			m_dLogFiles[iLog] = {};
			// quit tracking it
			m_dLogFiles.Remove ( iLog-- );
		}
		iFinalFiles = m_dLogFiles.GetLength();
	} // release m_tLogFilesAccess

	if ( bCurrentLogAbandoned && !eShutdown )
	{
		// if all logs were closed, we can rewind binlog file ext back to 0
		if ( !iFinalFiles )
			FixNofFiles ( iPreflushFiles );

		// if current log was closed, we need a new one (it will automatically save meta, too)
		if ( eAction!=DropTable )
			OpenNewLog ();
	} else if ( iPreflushFiles!=iFinalFiles || eAction!=NoSave )
	{
		// if we unlinked any logs, we need to save meta, too
		SaveMeta();
	}
	return iFinalFiles==0;
}


void SingleBinlog_c::AdoptIndex ( int iExt, const BinlogIndexInfo_t & tIdx )
{
	if ( m_dLogFiles.IsEmpty () )
	{
		BinlogFileDesc_t & tLog = m_dLogFiles.Add ();
		tLog.m_iExt = iExt;
		tLog.m_dIndexInfos.Add ( tIdx );
		return;
	}

	if ( m_dLogFiles.Last ().m_iExt!=iExt )
	{
		BinlogFileDesc_t & tLog = m_dLogFiles.Add ();
		tLog.m_iExt = iExt;
		tLog.m_dIndexInfos.Add ( tIdx );
		return;
	}

	BinlogFileDesc_t & tLog = m_dLogFiles.Last ();
	assert ( tLog.m_iExt==iExt );
	tLog.m_dIndexInfos.Add ( tIdx );
}


void SingleBinlog_c::AdoptFile ( int iExt )
{
	if ( !m_dLogFiles.IsEmpty () && m_dLogFiles.Last ().m_iExt==iExt )
		return;

	BinlogFileDesc_t & tLog = m_dLogFiles.Add ();
	tLog.m_iExt = iExt;
}


void SingleBinlog_c::OpenNewLog ( BinlogFileState_e eState )
{
	MEMORY ( MEM_BINLOG );

	// calc new ext
	int iExt = 0;
	{
		Threads::SccWL_t tLogLock ( m_tLogFilesAccess );
		if ( !m_dLogFiles.IsEmpty () )
			iExt = m_dLogFiles.Last ().m_iExt;

		if ( eState==BinlogFileState_e::OK || iExt<0 )
		{
			iExt = m_pOwner->NextBinlogExt ();
			BinlogFileDesc_t tLog;
			tLog.m_iExt = iExt;
			m_dLogFiles.Add ( std::move ( tLog ) );
		}
	}

	// update meta first then only remove binlog file
	SaveMeta ();

	// create file
	CSphString sLog = m_pOwner->MakeBinlogName ( iExt );

	if ( eState!=BinlogFileState_e::OK && eState!=BinlogFileState_e::ERROR_NON_READABLE ) // reuse the last binlog since it is empty or useless.
		::unlink ( sLog.cstr () );

	CSphString sError;
	if ( !m_tWriter.OpenFile ( sLog, sError ) )
		sphDie ( "failed to create %s: %s errno=%d, error=%s", sLog.cstr (), sError.cstr (), errno, strerrorm ( errno ) );

	// emit header
	m_tWriter.PutDword ( BINLOG_HEADER_MAGIC_SPBL );
	m_tWriter.PutDword ( BINLOG_VERSION );
}


int64_t SingleBinlog_c::LastTidFor ( const CSphString & sIndex ) const noexcept
{
	int64_t iTID = 0;
	Threads::SccRL_t tLogLock ( m_tLogFilesAccess );
	if ( m_dLogFiles.IsEmpty () )
		return iTID;

	for ( const auto& tLog : m_dLogFiles )
	{
		for ( const auto & tIndex: tLog.m_dIndexInfos )
		{
			if ( tIndex.m_sName == sIndex )
				iTID = Max ( tIndex.m_iFlushedTID, iTID );
		}
	}
	return iTID;
}


void SingleBinlog_c::LockWriter ()
{
	m_tWriteAccess.Lock();
}

void SingleBinlog_c::UnlockWriter ()
{
	m_tWriteAccess.Unlock();
}


void SingleBinlog_c::RemoveLastEmptyLog ()
{
	{
		Threads::SccWL_t tLock { m_tLogFilesAccess };
		assert ( !m_dLogFiles.IsEmpty () && m_dLogFiles.Last ().m_dIndexInfos.IsEmpty () );
		// do unlink
		CSphString sLog = m_pOwner->MakeBinlogName ( m_dLogFiles.Last ().m_iExt );
		if ( ::unlink ( sLog.cstr () ) )
			sphWarning ( "binlog: failed to unlink abandoned %s: %s", sLog.cstr (), strerrorm ( errno ) );

		// quit tracking it
		m_dLogFiles.Pop ();
	}

	// if we unlinked any logs, we need to save meta, too
	SaveMeta ();
}


int SingleBinlog_c::GetWriteIndexID ( const char * szIndexName, int64_t iTID )
{
	MEMORY ( MEM_BINLOG );
	Threads::SccRL_t tLogLock { m_tLogFilesAccess };
	assert ( !m_dLogFiles.IsEmpty () );

	// OPTIMIZE? maybe hash them?
	BinlogFileDesc_t & tLog = m_dLogFiles.Last ();
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
	{
		BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];
		if ( tIndex.m_sName==szIndexName )
		{
			tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
			return i;
		}
	}

	// create a new entry
	int iID = tLog.m_dIndexInfos.GetLength ();
	BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos.Add (); // caller must hold a wlock
	tIndex.m_iMinTID = iTID;
	tIndex.m_iMaxTID = iTID;
	tIndex.m_sName = szIndexName;
	tIndex.m_iFlushedTID = 0;

	// log this new entry
	BinlogTransactionGuard_c tGuard ( m_tWriter, false );

	m_tWriter.PutByte ( ADD_INDEX );
	m_tWriter.ZipOffset ( iID );
	m_tWriter.PutZString ( szIndexName );

	// return the index
	return iID;
}


// cache is a small summary of affected indexes, it is written at the very end of binlog file when it exceeded size limit,
// before opening new file.
void SingleBinlog_c::DoCacheWrite ()
{
	Threads::SccRL_t tLogLock { m_tLogFilesAccess };
	if ( m_dLogFiles.IsEmpty () )
		return;

	assert ( m_tWriter.IsOpen () );
	const CSphVector<BinlogIndexInfo_t> & dIndexes = m_dLogFiles.Last ().m_dIndexInfos;

	BinlogTransactionGuard_c tGuard ( m_tWriter );

	m_tWriter.PutByte ( ADD_CACHE );
	m_tWriter.ZipOffset ( dIndexes.GetLength () );
	for ( const auto & tIndex: dIndexes )
	{
		m_tWriter.PutZString ( tIndex.m_sName.cstr () );
		m_tWriter.ZipOffset ( tIndex.m_iMinTID );
		m_tWriter.ZipOffset ( tIndex.m_iMaxTID );
		m_tWriter.ZipOffset ( tIndex.m_iFlushedTID );
	}
	m_tWriter.Write ();
}


void SingleBinlog_c::CheckDoRestart ()
{
	// restart on exceed file size limit
	if ( !m_pOwner->m_iRestartSize || m_tWriter.GetFilePos ()<=m_pOwner->m_iRestartSize )
		return;

	MEMORY ( MEM_BINLOG );
#ifndef NDEBUG
	{ Threads::SccRL_t tLock { m_tLogFilesAccess };
	assert ( !m_dLogFiles.IsEmpty () ); }
#endif
	DoCacheWrite ();

	auto sName = m_tWriter.GetFilename();
	int iFD = m_tWriter.LeakFD();
	bool bFsyncOnClose = m_pOwner->m_eFlushFlavour==FlushAction_e::ACTION_FSYNC_ON_CLOSE;
	OpenNewLog ();

	Threads::Coro::Go ( [this, iFD, bFsyncOnClose, sName] {
		m_tFlushRunning.Wait ( [] ( int iVal ) { return iVal<1; } );
		if ( bFsyncOnClose && fsync ( iFD )!=0 )
			sphWarning ( "failed to sync %s: %d (%s)", sName.cstr(), errno, strerrorm ( errno ) );
		::close ( iFD );
	}, Threads::Coro::CurrentScheduler () );
}


bool SingleBinlog_c::CheckDoFlush ()
{
	switch ( m_pOwner->m_eFlushFlavour )
	{
	case FlushAction_e::ACTION_NONE:
		break;

	case FlushAction_e::ACTION_WRITE:
	case FlushAction_e::ACTION_FSYNC_ON_CLOSE:
		if ( !m_tWriter.Write () )
			return false;
		break;

	case FlushAction_e::ACTION_FSYNC:
		if ( !m_tWriter.WriteAndFsync () )
			return false;
		break;

	default:
		assert( false && "wrong binlog flush action flag" );
		break;
	}

	return true;
}


void SingleBinlog_c::SaveMeta ()
{
	m_pOwner->SaveMeta ();
}


void SingleBinlog_c::FixNofFiles ( int iFiles ) const
{
	m_pOwner->FixNofFiles (iFiles);
}

//////////////////////////////////////////////////////////////////////////

SingleBinlog_c* Binlog_c::GetWriteIndexBinlog ( const char* szIndexName, bool bOpenNewLog ) NO_THREAD_SAFETY_ANALYSIS
{
	if ( m_bCommonBinlog )
		return GetSingleWriteIndexBinlog ( bOpenNewLog );

	SingleBinlogPtr * pVal;
	{
		Threads::SccRL_t tLock ( m_tHashAccess );
		pVal = m_hBinlogs ( szIndexName );
	}
	if ( pVal )
		return pVal->Ptr();

	bool bLocked = false;
	{
		Threads::SccWL_t tLock ( m_tHashAccess );
		pVal = m_hBinlogs ( szIndexName );
		if ( pVal ) // the value arrived while we acquired w-lock
			return pVal->Ptr ();

		m_hBinlogs.Add ( SingleBinlogPtr { new SingleBinlog_c (this) }, szIndexName );
		pVal = m_hBinlogs ( szIndexName );
		if ( bOpenNewLog )
		{
			pVal->Ptr ()->LockWriter ();
			bLocked = true;
		}
	}
	assert ( pVal );
	// OpenNewLog invokes SaveMeta, which excludes m_tHashAccess
	if ( bOpenNewLog )
		pVal->Ptr ()->OpenNewLog ();

	if ( bLocked )
		pVal->Ptr ()->UnlockWriter ();

	return pVal->Ptr ();
}


SingleBinlog_c * Binlog_c::GetSingleWriteIndexBinlog ( bool bOpenNewLog ) NO_THREAD_SAFETY_ANALYSIS
{
	assert ( m_bCommonBinlog );

	SingleBinlogPtr * pVal = nullptr;
	{
		Threads::SccRL_t tLock ( m_tHashAccess );
		auto Iter = m_hBinlogs.begin ();
		if ( Iter != m_hBinlogs.end() )
			pVal = &Iter->second;
	}
	if ( pVal )
		return pVal->Ptr ();

	bool bLocked = false;
	{
		Threads::SccWL_t tLock ( m_tHashAccess );
		auto Iter = m_hBinlogs.begin ();
		if ( Iter!=m_hBinlogs.end () )
			pVal = &Iter->second;

		if ( pVal ) // the value arrived while we acquired w-lock
			return pVal->Ptr ();

		m_hBinlogs.Add ( SingleBinlogPtr{new SingleBinlog_c ( this )}, "common" );
		pVal = &m_hBinlogs.begin ()->second;
		if ( bOpenNewLog )
		{
			pVal->Ptr()->LockWriter();
			bLocked = true;
		}
	}
	assert ( pVal );
	// OpenNewLog invokes SaveMeta, which excludes m_tHashAccess
	if ( bOpenNewLog )
		pVal->Ptr ()->OpenNewLog ();

	if ( bLocked )
		pVal->Ptr ()->UnlockWriter();

	return pVal->Ptr ();
}


SingleBinlog_c * Binlog_c::GetFlushIndexBinlog ( const char * szIndexName ) const REQUIRES ( m_tHashAccess )
{
	SingleBinlogPtr * pVal = nullptr;
	if ( !m_bCommonBinlog )
		pVal = m_hBinlogs ( szIndexName );
	else
	{
		auto Iter = m_hBinlogs.begin ();
		if ( Iter!=m_hBinlogs.end () )
			pVal = &Iter->second;
	}

	if ( !pVal )
		return nullptr;
	return pVal->Ptr ();
}


Binlog_c::~Binlog_c ()
{
	if ( m_bDisabled )
		return;

	for ( auto & tBinlog: m_hBinlogs )
		tBinlog.second->Deinit();

	UnlockBinlog ();
}

void Binlog_c::MakeBinlogFilenameTemplate() noexcept
{
	m_sBinlogFileNameTemplate = SphSprintf ( "%s/binlog.%%0%dd", m_sLogPath.cstr (), m_iBinlogFileDigits );
}

CSphString Binlog_c::MakeBinlogName ( int iExt ) const noexcept
{
	assert ( !m_sBinlogFileNameTemplate.IsEmpty() );
	return SphSprintf ( m_sBinlogFileNameTemplate.scstr(), iExt );
}

void Binlog_c::RemoveFile ( int iExt ) const NO_THREAD_SAFETY_ANALYSIS
{
	auto sLog = MakeBinlogName ( iExt );
	::unlink ( sLog.cstr () );
}

// here's been going binlogs with ALL closed indices removing // notify per-index-binlog?
void Binlog_c::NotifyIndexFlush ( int64_t iFlushedTID, const char* szFlushedIndexName, Shutdown_e eShutdown, ForceSave_e eAction )
{
	if ( m_bReplayMode )
		sphInfo ( "table '%s': ramchunk saved. TID=" INT64_FMT, szFlushedIndexName, iFlushedTID );

	if ( !IsBinlogWritable () )
		return;

	if ( m_bCommonBinlog || eAction!=DropTable )
	{
		if ( eAction == DropTable )
			eAction = ForceSave;
		auto pSingleBinlog = GetWriteIndexBinlog ( szFlushedIndexName );
		pSingleBinlog->NotifyIndexFlush ( iFlushedTID, szFlushedIndexName, eShutdown, eAction );
		return;
	}

	Threads::SccWL_t tLock ( m_tHashAccess );
	auto pSingleBinlog = GetFlushIndexBinlog ( szFlushedIndexName );
	if ( !pSingleBinlog )
		return;

	bool bAbandoned = pSingleBinlog->NotifyIndexFlush ( iFlushedTID, szFlushedIndexName, eShutdown, eAction );
	if ( !bAbandoned )
		return;

	m_hBinlogs.Delete ( szFlushedIndexName );
	SaveMetaUnlock();
}


int Binlog_c::NextBinlogExt ()
{
	return m_iNextBinlog.fetch_add ( 1, std::memory_order_relaxed );
}


void Binlog_c::FixNofFiles ( int iFiles )
{
	m_iNumFiles.fetch_sub ( iFiles );
	if ( !m_iNumFiles )
		m_iNextBinlog.store ( 0, std::memory_order_release );
}


// run once on startup
void Binlog_c::CheckAndSetPath ( CSphString sBinlogPath )
{
	m_sLogPath = std::move ( sBinlogPath );
	MakeBinlogFilenameTemplate();
	m_bDisabled = m_sLogPath.IsEmpty ();

	if ( m_bDisabled )
		return;

	// pair lock/unlock ensures binlog path is available and writable
	LockBinlog ();
	UnlockBinlog ();
}

// run once on startup
void Binlog_c::Configure ( const CSphConfigSection & hSearchd, DWORD uReplayFlags )
{
	MEMORY ( MEM_BINLOG );

	const int iMode = hSearchd.GetInt ( "binlog_flush", 2 );
	switch ( iMode )
	{
	case 0: m_eFlushFlavour = FlushAction_e::ACTION_NONE; break;
	case 1: m_eFlushFlavour = FlushAction_e::ACTION_FSYNC; break;
	case 2: m_eFlushFlavour = FlushAction_e::ACTION_WRITE; break;
	case 3: m_eFlushFlavour = FlushAction_e::ACTION_FSYNC_ON_CLOSE; break;
		default:	sphDie ( "unknown binlog flush mode %d (must be 0, 1, 2, or 3)\n", iMode );
	}

	m_iRestartSize = hSearchd.GetSize ( "binlog_max_log_size", m_iRestartSize );
	m_uReplayFlags = uReplayFlags;
	m_iBinlogFileDigits = hSearchd.GetInt ( "binlog_filename_digits", 4 );

	if ( m_bDisabled )
		return;

	LockBinlog ();
	LoadMeta();
	MakeBinlogFilenameTemplate ();
}

void Binlog_c::SetCommon ( bool bCommonBinlog )
{
	if ( m_bCommonBinlog == bCommonBinlog )
		return;

	m_bCommonBinlog = bCommonBinlog;
	// fixme! add cleanup to avoid 'mixed' binlog
}

bool Binlog_c::IsFlushingEnabled () const
{
	return !m_bDisabled && m_eFlushFlavour!=FlushAction_e::ACTION_FSYNC;
}


// executed externally by task binlog flush, every BINLOG_AUTO_FLUSH (1 sec)
void Binlog_c::DoFlush ()
{
	assert ( !m_bDisabled );
	MEMORY ( MEM_BINLOG );

	if ( sphInterrupted () )
		return;

	m_iLastFlushed.store ( sphMicroTimer (), std::memory_order_relaxed );

	CSphVector<std::pair<int, SingleBinlogPtr>> dToFsync;
	{
		Threads::SccRL_t tLock ( m_tHashAccess );
		for ( auto& tBinlog : m_hBinlogs )
			dToFsync.Add ( tBinlog.second->DoFlush ( m_eFlushFlavour ) );
	}
	for ( auto& tSync : dToFsync)
	{
		if ( sphInterrupted () )
			return;
		tSync.second->DoFsync ( tSync.first );
	}
}

int64_t Binlog_c::NextFlushingTime () const noexcept
{
	auto iLastFlushed = m_iLastFlushed.load ( std::memory_order_relaxed );
	if ( !iLastFlushed )
		return sphMicroTimer () + m_iFlushPeriod;
	return iLastFlushed + m_iFlushPeriod;
}


int64_t Binlog_c::LastTidFor ( const CSphString & sIndex ) const noexcept
{
	int64_t iTID = 0;
	Threads::SccRL_t tLock ( m_tHashAccess );
	for ( auto & tBinlog: m_hBinlogs )
		iTID = Max ( iTID, tBinlog.second->LastTidFor ( sIndex ) );
	return iTID;
}

static constexpr int SAVE_TRIES = 4;
static constexpr int SAVE_TRIE_DELAY = 50;

void Binlog_c::DoSaveMeta ( IntVec_t dFiles, SaveMeta_e eForce )
{
	if ( eForce==eNoForce && CompareCurrentFiles ( dFiles ) )
		return; // files are same as stored; no need to rewrite meta

	CSphVector<BYTE> dMeta;
	MemoryWriter2_c wrMeta ( dMeta );
	wrMeta.PutDword ( BINLOG_META_MAGIC_SPLI );
	wrMeta.PutDword ( BINLOG_VERSION );
	wrMeta.PutByte ( m_iBinlogFileDigits );

//	StringBuilder_c sMetaLog;
//	sMetaLog << "SaveMeta: " << m_bReplayMode << " " << dFiles.GetLength () << ": ";
//	sMetaLog.StartBlock ();
	m_iNumFiles.store ( dFiles.GetLength (), std::memory_order_relaxed );
	wrMeta.ZipInt ( dFiles.GetLength () );
	for ( const auto & iExt: dFiles )
	{
		wrMeta.ZipInt ( iExt ); // everything else is saved in logs themselves
//		sMetaLog << iExt;
	}

	// sphWarning ( "%s", sMetaLog.cstr() );

	Threads::SccWL_t rLock { m_tCurrentFilesAccess };
	if ( eForce==eNoForce && m_dCurrentFiles==dFiles )
		return;

	if ( dFiles.IsEmpty() )
		m_iNextBinlog.store ( 0, std::memory_order_release );

	m_dCurrentFiles.SwapData ( dFiles );

	auto sMetaNew = SphSprintf ( "%s/binlog.meta.new", m_sLogPath.cstr () );
	auto sMeta = SphSprintf ( "%s/binlog.meta", m_sLogPath.cstr () );

	for ( int i=0; i<SAVE_TRIES; ++i )
	{
		CSphString sError;
		CSphWriterNonThrottled wrMetaFile;
		::unlink ( sMetaNew.cstr() );
		if ( !wrMetaFile.OpenFile ( sMetaNew, sError ) )
			sphDie ( "failed to open '%s': '%s'", sMetaNew.cstr(), sError.cstr() );

		wrMetaFile.PutBytes ( dMeta.begin(), dMeta.GetLength() );
		wrMetaFile.CloseFile();

		if ( wrMetaFile.IsError() )
		{
			sphWarning ( "Error when closing file %s, errno=%d, error=%s, try=%d", sError.cstr(), errno, strerrorm ( errno ), i+1 );
			continue;
		}

		if ( sph::rename ( sMetaNew.cstr(), sMeta.cstr() ) )
		{
			sphSleepMsec ( SAVE_TRIE_DELAY );
			if ( sph::rename ( sMetaNew.cstr (), sMeta.cstr () ) )
			{
				if ( i<SAVE_TRIES-1 )
				{
					sphWarning ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s), try %d",
								 sMetaNew.cstr (), sMeta.cstr (), errno, strerrorm ( errno ),
								 i+1 ); // !COMMIT handle this gracefully
					sphSleepMsec ( SAVE_TRIE_DELAY );
				} else
					sphDie ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)",
							 sMetaNew.cstr (), sMeta.cstr (), errno,
							 strerrorm ( errno ) ); // !COMMIT handle this gracefully
			} else break;
		} else break;
	}
	sphLogDebug ( "Binlog::SaveMeta: Done (%s)", sMetaNew.cstr() );
}


void Binlog_c::SaveMeta ()
{
	MEMORY ( MEM_BINLOG );
	DoSaveMeta ( CollectBinlogFiles (), eNoForce );
}


void Binlog_c::SaveMetaUnlock ( SaveMeta_e eForce )
{
	MEMORY ( MEM_BINLOG );
	DoSaveMeta ( CollectBinlogFilesUnlock (), eForce );
}


IntVec_t Binlog_c::CollectBinlogFiles ()
{
	Threads::SccRL_t rLock { m_tHashAccess };
	return CollectBinlogFilesUnlock();
}


IntVec_t Binlog_c::CollectBinlogFilesUnlock ()
{
	IntVec_t dFiles;
	for ( auto & tBinlog: m_hBinlogs )
		tBinlog.second->CollectBinlogFiles ( dFiles );

	// append not yet processed saved files
	for ( int i = m_dSavedFiles.GetLength ()-1; i>=0; --i )
		dFiles.Add ( m_dSavedFiles[i] );

	return dFiles;
}


bool Binlog_c::CompareCurrentFiles ( const IntVec_t & dFiles )
{
	Threads::SccRL_t rLock { m_tCurrentFilesAccess };
	return m_dCurrentFiles==dFiles;
}

void Binlog_c::LockBinlog ()
{
	assert ( m_iLockFD==-1 );
	CSphString sError;
	if ( !RawFileLock ( SphSprintf ( "%s/binlog.lock", m_sLogPath.cstr () ), m_iLockFD, sError ) )
		sphDie ( "%s", sError.scstr () );
}

void Binlog_c::UnlockBinlog ()
{
	RawFileUnLock ( SphSprintf ( "%s/binlog.lock", m_sLogPath.cstr () ), m_iLockFD );
}


bool Binlog_c::IsBinlogWritable () const noexcept
{
	if ( m_bReplayMode )
		return false;

	return !m_bDisabled;
}


// commit stuff. Indexes call this function with serialization cb; binlog is agnostic to alien data structures.
bool Binlog_c::BinlogCommit ( int64_t * pTID, const char* szIndexName, FnWriteCommit fnSaver, CSphString & sError )
{
	if ( !IsBinlogWritable () ) // m.b. need to advance TID as index flush according to it
		return true;

	auto pSingleBinlog = GetWriteIndexBinlog ( szIndexName );
	return pSingleBinlog->BinlogCommit ( pTID, szIndexName, std::move ( fnSaver ), sError );
}


// called once on startup from Configure()
void Binlog_c::LoadMeta ()
{
	MEMORY ( MEM_BINLOG );

	auto sMeta = SphSprintf ( "%s/binlog.meta", m_sLogPath.cstr () );
	if ( !sphIsReadable ( sMeta.cstr () ) )
		return;

	CSphString sError;

	// opened and locked, lets read
	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMeta, sError ) )
		sphDie ( "%s error: %s", sMeta.cstr (), sError.cstr () );

	if ( rdMeta.GetDword ()!=BINLOG_META_MAGIC_SPLI )
		sphDie ( "invalid meta file %s", sMeta.cstr () );

	// binlog meta v1 was dev only, crippled, and we don't like it anymore
	// binlog metas v2 upto current v4 (and likely up) share the same simplistic format
	// so let's support empty (!) binlogs w/ known versions and compatible metas
	DWORD uVersion = rdMeta.GetDword ();
	if ( uVersion==1 || uVersion>BINLOG_VERSION )
		sphDie ( "binlog meta file %s is v.%d, binary is v.%d; recovery requires previous binary version",
				 sMeta.cstr (), uVersion, BINLOG_VERSION );

	auto uByte = rdMeta.GetByte(); // id64 for v<15; num of digits in names for v>=15
	m_dSavedFiles.Resize ( rdMeta.UnzipInt () ); // FIXME! sanity check

	if ( m_dSavedFiles.IsEmpty () )
		return;

	// ok, so there is actual recovery data
	// could be wrong version of the empty binlog
	m_bWrongVersion = ( uVersion!=BINLOG_VERSION );

	// let's require that bitness
	if ( uVersion < 15 )
	{
		if ( uByte != 1 )
			sphDie ( "tables with 32-bit docids are no longer supported; recovery requires previous binary version" );
	} else
		m_iBinlogFileDigits = uByte;

	assert ( m_iBinlogFileDigits>0 );

	// load list of active log files
	int iMaxExt = 0;
	for ( int i=m_dSavedFiles.GetLength ()-1; i>=0; --i )
	{
		auto iExt = rdMeta.UnzipInt (); // everything else is saved in logs themselves
		if ( iExt>iMaxExt )
			iMaxExt = iExt;
		m_dSavedFiles[i] = iExt;
	}

	m_iNextBinlog.store ( iMaxExt+1, std::memory_order_release );
}

// primary call - invoked from daemon once on start
void Binlog_c::Replay ( const SmallStringHash_T<CSphIndex *> & hIndexes, ProgressCallbackSimple_t * pfnProgressCallback ) NO_THREAD_SAFETY_ANALYSIS
{
	if ( m_bDisabled )
		return;

	// on replay started
	if ( pfnProgressCallback )
		pfnProgressCallback ();

	int64_t tmReplay = sphMicroTimer ();

	// do replay
	m_bReplayMode = true;
	BinlogFileState_e ePrevLogState = BinlogFileState_e::OK, eLastLogState = BinlogFileState_e::OK;
	SingleBinlog_c* pPrevInfo = nullptr, *pInfo = nullptr;
	while ( !m_dSavedFiles.IsEmpty() )
	{
		auto iExt = m_dSavedFiles.Last();
		BinlogReplayFileDesc_t tLog;
		tLog.m_iExt = iExt;
		ePrevLogState = std::exchange ( eLastLogState, ReplayBinlog ( tLog, hIndexes ) );
		if ( pfnProgressCallback ) // on each replayed binlog
			pfnProgressCallback ();

		m_dSavedFiles.Pop();

		if ( tLog.m_dIndexInfos.IsEmpty() )
		{
			if ( pInfo )
				pInfo->AdoptFile ( iExt );
			else
				RemoveFile ( iExt );
		}


		for ( const auto& tInfo : tLog.m_dIndexInfos )
		{
			pInfo = GetWriteIndexBinlog ( tInfo.m_sName.cstr (), false );
			pInfo->AdoptIndex ( tLog.m_iExt, tInfo );
			if ( pInfo!=pPrevInfo )
			{
				if ( pPrevInfo )
					pPrevInfo->OpenNewLog ( ePrevLogState );
				pPrevInfo = pInfo;
			}
		}
	}

	if ( !pInfo && !pPrevInfo )
		SaveMetaUnlock ( eForce );

	if ( pPrevInfo )
		pPrevInfo->OpenNewLog ( eLastLogState );

	if ( m_dSavedFiles.GetLength ()>0 )
	{
		tmReplay = sphMicroTimer ()-tmReplay;
		sphInfo ( "binlog: finished replaying total %d in %d.%03d sec",
				  m_dSavedFiles.GetLength (),
				  (int) ( tmReplay/1000000 ), (int) ( ( tmReplay/1000 )%1000 ) );
	}

	// FIXME?
	// in some cases, indexes might had been flushed during replay
	// and we might therefore want to update m_iFlushedTID everywhere
	// but for now, let's just wait until next flush for simplicity

	// resume normal operation
	m_bReplayMode = false;
}


BinlogFileState_e Binlog_c::ReplayBinlog ( BinlogReplayFileDesc_t & tLog, const SmallStringHash_T<CSphIndex*> & hIndexes ) NO_THREAD_SAFETY_ANALYSIS
{
	CSphString sError;
	const CSphString sLog ( MakeBinlogName ( tLog.m_iExt ) );

	// open, check, play
	sphInfo ( "binlog: replaying log %s", sLog.cstr() );

	BinlogReader_c tReader;
	if ( !tReader.Open ( sLog, sError ) )
	{
		Log ( REPLAY_IGNORE_OPEN_ERROR, "binlog: log open error: %s", sError.cstr() );
		return BinlogFileState_e::ERROR_NON_READABLE;
	}

	const SphOffset_t iFileSize = tReader.GetFilesize();
	if ( !iFileSize )
	{
		sphWarning ( "binlog: empty binlog %s detected, skipping", sLog.cstr() );
		return BinlogFileState_e::ERROR_EMPTY_0;
	}

	if ( tReader.GetDword()!=BINLOG_HEADER_MAGIC_SPBL )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: log %s missing magic header (corrupted?)", sLog.cstr() );
		return BinlogFileState_e::ERROR_WRONG_FILE;
	}

	DWORD uVersion = tReader.GetDword();
	if ( tReader.GetErrorFlag() )
		sphWarning ( "binlog: log io error at pos=" INT64_FMT ": %s", tReader.GetPos(), sError.cstr() );

	// could replay empty binlog of the old version
	m_bWrongVersion = ( uVersion!=BINLOG_VERSION );

	if ( iFileSize==8 ) // couple of DWORDs we just read - header and version
	{
		sphWarning ( "binlog: empty binlog %s detected, skipping", sLog.cstr () );
		return BinlogFileState_e::ERROR_EMPTY_8;
	}

	/////////////
	// do replay
	/////////////

	std::array<int, TOTAL+1> dTotal {0};

	// !COMMIT
	// instead of simply replaying everything, we should check whether this binlog is clean
	// by loading and checking the cache stored at its very end
	tLog.m_dIndexInfos.Reset();

	bool bReplayOK = true;
	bool bHaveCacheOp = false;
	int64_t iPos = -1;

	int64_t tmReplay = sphMicroTimer();

	while ( iFileSize!=tReader.GetPos() && !tReader.GetErrorFlag() && bReplayOK )
	{
		iPos = tReader.GetPos();
		if ( tReader.GetDword()!=BLOP_MAGIC_TXN_ )
		{
			Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: log missing txn marker at pos=" INT64_FMT " (corrupted?)", iPos );
			bReplayOK = false;
			break;
		}
		tReader.ResetCrc ();
		const auto uOp = (Blop_e) tReader.GetByte ();

		if ( uOp<=0 || uOp>=TOTAL )
		{
			Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: unexpected entry (blop=" UINT64_FMT ", pos=" INT64_FMT ")", uOp, iPos );
			bReplayOK = false;
			break;
		}
		if ( m_bWrongVersion && uOp!=ADD_CACHE )
		{
			Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: log %s is v.%d, binary is v.%d; recovery requires previous binary version", sLog.cstr(), uVersion, BINLOG_VERSION );
			bReplayOK = false;
			break;
		}

		// FIXME! blop might be OK but skipped (eg. index that is no longer)
		switch ( uOp )
		{
			case ADD_INDEX:
				bReplayOK = ReplayIndexAdd ( tLog, hIndexes, tReader );
				break;

			case ADD_CACHE:
				if ( bHaveCacheOp )
				{
					Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: internal error, second BLOP_ADD_CACHE detected (corruption?)" );
					bReplayOK = false;
					break;
				}
				bHaveCacheOp = true;
				bReplayOK = ReplayCacheAdd ( tLog, uVersion, tReader );
				break;

			case ADD_TXN:
				bReplayOK = ReplayTxn ( tLog, tReader );
				break;

			default:
				Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: internal error, unhandled entry (blop=%d)", (int)uOp );
				bReplayOK = false;
				break;
		}

		dTotal [ uOp ] += bReplayOK ? 1 : 0;
		++dTotal [ TOTAL ];
	}

	tmReplay = sphMicroTimer() - tmReplay;

	if ( tReader.GetErrorFlag() )
		sphWarning ( "binlog: log io error at pos=" INT64_FMT ": %s", iPos, sError.cstr() );

	if ( !bReplayOK )
		sphWarning ( "binlog: replay error at pos=" INT64_FMT , iPos );

	// show additional replay statistics
	for ( const auto& tIndex : tLog.m_dIndexInfos )
	{
		if ( tIndex.m_iPreReplayTID < tIndex.m_iMaxTID )
		{
			sphInfo ( "binlog: table %s: recovered from tid " INT64_FMT " to tid " INT64_FMT,
				tIndex.m_sName.cstr(), tIndex.m_iPreReplayTID, tIndex.m_iMaxTID );

		} else
		{
			sphInfo ( "binlog: table %s: skipped at tid " INT64_FMT " and max binlog tid " INT64_FMT,
				tIndex.m_sName.cstr(), tIndex.m_iPreReplayTID, tIndex.m_iMaxTID );
		}
	}

	sphInfo ( "binlog: replay stats: %d commits, %d tables", dTotal[ADD_TXN], dTotal[ADD_INDEX] );
	sphInfo ( "binlog: finished replaying %s; %d.%d MB in %d.%03d sec",
		sLog.cstr(),
		(int)(iFileSize/1048576), (int)((iFileSize*10/1048576)%10),
		(int)(tmReplay/1000000), (int)((tmReplay/1000)%1000) );

	// only one operation, that is Add Cache - by the fact, empty binlog
	return ( bHaveCacheOp && dTotal[TOTAL]==1 ) ? BinlogFileState_e::ERROR_ABANDONED : BinlogFileState_e::OK;
}

bool Binlog_c::ReplayIndexAdd ( BinlogReplayFileDesc_t & tLog, const SmallStringHash_T<CSphIndex*> & hIndexes, BinlogReader_c & tReader ) const NO_THREAD_SAFETY_ANALYSIS
{
	// load and check index
	const int64_t iTxnPos = tReader.GetPos(); // that is purely for reporting anomalies

	uint64_t uVal = tReader.UnzipOffset();

	// load data
	CSphString sName = tReader.GetZString();

	if ( !tReader.CheckCrc ( "indexadd", sName.cstr(), 0, iTxnPos ) )
		return false;

	if ( (int) uVal!=tLog.m_dIndexInfos.GetLength () )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR,
			  "binlog: indexadd: unexpected table id (id=" UINT64_FMT ", expected=%d, pos=" INT64_FMT ")",
			  uVal, tLog.m_dIndexInfos.GetLength (), iTxnPos );
		return false;
	}

	// check for index name dupes
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
	{
		if ( tLog.m_dIndexInfos[i].m_sName == sName )
		{
			Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: duplicate table name (name=%s, dupeid=%d, pos=" INT64_FMT ")",
				sName.cstr(), i, iTxnPos );
			return false;
		}
	}

	// not a dupe, lets add
	BinlogIndexReplayInfo_t & tIndex = tLog.m_dIndexInfos.Add();
	tIndex.m_sName = sName;

	// lookup index in the list of currently served ones
	CSphIndex ** ppIndex = hIndexes ( sName.cstr() );
	CSphIndex * pIndex = ppIndex ? (*ppIndex) : nullptr;
	if ( pIndex )
	{
		tIndex.m_pIndex = pIndex;
		tIndex.m_iPreReplayTID = pIndex->m_iTID;
		tIndex.m_iFlushedTID = pIndex->m_iTID;
	}

	// all ok
	// TID ranges will be now recomputed as we replay
	return true;
}

bool Binlog_c::ReplayCacheAdd ( const BinlogReplayFileDesc_t & tLog, DWORD uVersion, BinlogReader_c & tReader ) const NO_THREAD_SAFETY_ANALYSIS
{
	const int64_t iTxnPos = tReader.GetPos();

	// check data
	int iCache = tReader.UnzipOffset (); // FIXME! sanity check
	if ( m_bWrongVersion && iCache )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: log %s is v.%d, binary is v.%d; recovery requires previous binary version", tReader.GetFilename().cstr(), uVersion, BINLOG_VERSION );
		return false;
	}

	BinlogIndexInfo_t tCache;
	for ( int i = 0; i<iCache; ++i )
	{
		tCache.m_sName = tReader.GetZString ();
		tCache.m_iMinTID = tReader.UnzipOffset ();
		tCache.m_iMaxTID = tReader.UnzipOffset ();
		tCache.m_iFlushedTID = tReader.UnzipOffset ();

		const BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];

		if ( tCache.m_sName!=tIndex.m_sName )
		{
			sphWarning ( "binlog: cache mismatch: table %d name mismatch (%s cached, %s replayed)",
				i, tCache.m_sName.cstr (), tIndex.m_sName.cstr () );
			continue;
		}

		if ( tCache.m_iMinTID!=tIndex.m_iMinTID || tCache.m_iMaxTID!=tIndex.m_iMaxTID )
		{
			sphWarning ( "binlog: cache mismatch: table %s tid ranges mismatch (cached " INT64_FMT " to " INT64_FMT ", replayed " INT64_FMT " to " INT64_FMT ")",
				tCache.m_sName.cstr (), tCache.m_iMinTID, tCache.m_iMaxTID, tIndex.m_iMinTID, tIndex.m_iMaxTID );
		}
	}

	if ( !tReader.CheckCrc ( "cache", "", 0, iTxnPos ) )
		return false;

	// if we arrived here by replay, let's verify everything
	// note that cached infos just passed checksumming, so the file is supposed to be clean!
	// in any case, broken log or not, we probably managed to replay something
	// so let's just report differences as warnings

	if ( iCache!=tLog.m_dIndexInfos.GetLength() )
		sphWarning ( "binlog: cache mismatch: %d tables cached, %d replayed", iCache, tLog.m_dIndexInfos.GetLength() );

	return true;
}

//////////////////////////////////////////////////////////////////////////
/// custom replay stuff
//////////////////////////////////////////////////////////////////////////

// helper used in about all replay ops
int Binlog_c::ReplayIndexID ( CSphReader & tReader, const BinlogReplayFileDesc_t & tLog ) const
{
	const int64_t iTxnPos = tReader.GetPos();
	const int iVal = (int)tReader.UnzipOffset();

	if ( iVal<0 || iVal>=tLog.m_dIndexInfos.GetLength() )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: unexpected table id (id=%d, max=%d, pos=" INT64_FMT ")", iVal, tLog.m_dIndexInfos.GetLength(), iTxnPos );
		return -1;
	}

	return iVal;
}

namespace {
void SkipBytes ( CSphAutoreader& tReader, DWORD uSize )
{
	auto uFakeBuf = Min ( uSize, 4096 );
	CSphFixedVector<BYTE> dFakeBuf { uFakeBuf };
	while ( uSize )
	{
		tReader.GetBytes ( dFakeBuf.begin (), uFakeBuf );
		uSize -= uFakeBuf;
		uFakeBuf = Min ( uSize, 4096 );
	}
}
}

bool Binlog_c::ReplayTxn ( const BinlogReplayFileDesc_t & tLog, BinlogReader_c & tReader ) const NO_THREAD_SAFETY_ANALYSIS
{
	// load and lookup index
	const int64_t iTxnPos = tReader.GetPos();
	int iIdx = ReplayIndexID ( tReader, tLog );
	if ( iIdx==-1 )
		return false;

	BinlogIndexReplayInfo_t & tIndex = tLog.m_dIndexInfos[iIdx];

	// load transaction data
	auto iTID = (int64_t) tReader.UnzipOffset();
	auto uSize = tReader.GetDword();

	// skip txns of non-existent (deleted) indexes (skip blobs by size)
	if ( !tIndex.m_pIndex || iTID<=tIndex.m_pIndex->m_iTID || tIndex.m_pIndex->m_iTID==-1 )
	{
		tIndex.m_iMinTID = Min ( tIndex.m_iMinTID, iTID );
		tIndex.m_iFlushedTID = tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );

		// just skip the blob
		SkipBytes ( tReader, uSize );

		// checksum
		return !tReader.GetErrorFlag () && tReader.CheckCrc ( "skip", tIndex.m_sName.cstr (), iTID, iTxnPos );
	}

	assert ( tIndex.m_pIndex );

	CSphString sError;
	BYTE uOp = tReader.GetByte();
	CSphString sOp = SzTxnName ( (Txn_e) uOp );
	CheckTnxResult_t tReplayed = tIndex.m_pIndex->ReplayTxn ( tReader, sError, uOp, [ iTxnPos, iTID, this, &tReader, &tIndex, &sOp ] {
		
		CheckTnxResult_t tRes;
		tRes.m_bValid = PerformChecks ( sOp.cstr (), tIndex, iTID, iTxnPos, tReader );
		if ( !tRes.m_bValid )
			return tRes;

		// only replay transaction when index exists and does not have it yet (based on TID)
		if ( tIndex.m_pIndex && iTID>tIndex.m_pIndex->m_iTID )
		{
			tRes.m_bApply = true;
			// we normally expect per-index TIDs to be sequential
			// but let's be graceful about that
			if ( iTID!=tIndex.m_pIndex->m_iTID+1 )
				sphWarning (
						"binlog: %s: unexpected tid (table=%s, indextid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ")",
						sOp.cstr (), tIndex.m_sName.cstr (), tIndex.m_pIndex->m_iTID, iTID, iTxnPos );
		}
		return tRes;
	});

	// could be invalid TXN in binlog
	if ( !tReplayed.m_bValid )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: %s (table=%s, lasttid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ", error=%s)",
			  sOp.cstr (), tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos, sError.cstr() );
		return false;
	}

	// could be TXN in binlog that index already has should not apply again that TXN and should not change index TID by that TXN
	if ( tReplayed.m_bApply )
	{
		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pIndex->m_iTID = iTID;
	} 

	tIndex.m_iMinTID = Min ( tIndex.m_iMinTID, iTID );
	tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
	return true;

}

bool Binlog_c::MockDisabled ( bool bNewVal )
{
	return std::exchange ( m_bDisabled, bNewVal );
}

bool Binlog_c::PerformChecks ( const char * szOp, BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t iTxnPos, BinlogReader_c & tReader ) const
{
	// checksum
	if ( tReader.GetErrorFlag () || !tReader.CheckCrc ( szOp, tIndex.m_sName.cstr (), iTID, iTxnPos ) )
		return false;

	// check TID
	if ( iTID<tIndex.m_iMaxTID )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR,  "binlog: %s: descending tid (table=%s, lasttid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ")",
			  szOp, tIndex.m_sName.cstr (), tIndex.m_iMaxTID, iTID, iTxnPos );
		return false;
	}
	return true;
}


static auto & g_bRTChangesAllowed = RTChangesAllowed();

void Binlog::Init ( CSphString sBinlogPath )
{
	MEMORY ( MEM_BINLOG );

	g_bRTChangesAllowed = false;
	g_pRtBinlog.reset ( new Binlog_c );
	if ( !g_pRtBinlog )
		sphDie ( "binlog: failed to create binlog" );

	// check binlog path before detaching from the console - since we call sphDie on failure, and it should be visible.
	g_pRtBinlog->CheckAndSetPath ( std::move ( sBinlogPath ) );
}

void Binlog::SetCommon ( bool bCommonBinlog )
{
	assert ( g_pRtBinlog );
	g_pRtBinlog->SetCommon ( bCommonBinlog );
}

void Binlog::Configure ( const CSphConfigSection & hSearchd, DWORD uReplayFlags )
{
	assert ( g_pRtBinlog );
	g_pRtBinlog->Configure ( hSearchd, uReplayFlags );
}


void Binlog::Deinit ()
{
	g_pRtBinlog.reset();
}


void Binlog::Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, ProgressCallbackSimple_t * pfnProgressCallback )
{
	MEMORY ( MEM_BINLOG );
	g_pRtBinlog->Replay ( hIndexes, pfnProgressCallback );
	g_bRTChangesAllowed = true;
}

bool Binlog::IsActive()
{
	if ( !g_pRtBinlog )
		return false;
	return g_pRtBinlog->IsActive();
}

bool Binlog::MockDisabled ( bool bNewVal )
{
	if ( g_pRtBinlog )
		return g_pRtBinlog->MockDisabled ( bNewVal );
	return bNewVal;
}

bool Binlog::Commit ( int64_t * pTID, const char* szIndexName, CSphString & sError, FnWriteCommit && fnSaver )
{
	if ( !g_pRtBinlog )
		return true;

	if ( *pTID==-1 )
		return true;

	return g_pRtBinlog->BinlogCommit ( pTID, szIndexName, std::move (fnSaver), sError );
}

void Binlog::NotifyIndexFlush ( int64_t iTID, const char * szIndexName, Shutdown_e eShutdown, ForceSave_e eAction )
{
	if ( !g_pRtBinlog )
		return;

	if ( iTID==-1 )
		return;

	g_pRtBinlog->NotifyIndexFlush ( iTID, szIndexName, eShutdown, eAction );
}

CSphString Binlog::GetPath()
{
	if ( g_pRtBinlog )
		return g_pRtBinlog->GetLogPath();
	return "";
}

int64_t Binlog::LastTidFor ( const CSphString & sIndex )
{
	if ( !g_pRtBinlog )
		return 0;
	return g_pRtBinlog->LastTidFor ( sIndex );
}


bool Binlog::IsFlushEnabled ()
{
	if ( !g_pRtBinlog )
		return false;

	return g_pRtBinlog->IsFlushingEnabled ();
}

void Binlog::Flush ()
{
	if ( !g_pRtBinlog )
		return;

	g_pRtBinlog->DoFlush ();
}

int64_t Binlog::NextFlushTimestamp ()
{
	if ( !g_pRtBinlog )
		return -1;

	return g_pRtBinlog->NextFlushingTime ();
}

void Binlog_c::Log ( DWORD uFlag, const char * sTemplate, ... ) const
{
	va_list ap;

	va_start ( ap, sTemplate );
	if ( ( m_uReplayFlags & uFlag )==0 )
	{
		sphDieVa ( sTemplate, ap );
		exit ( 1 );
	} else if ( g_eLogLevel>=SPH_LOG_WARNING )
	{
		sphLogVa ( sTemplate, ap, SPH_LOG_WARNING );
	}
	va_end ( ap );
}
