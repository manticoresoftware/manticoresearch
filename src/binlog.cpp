//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
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
#include "sphinxsearch.h"
#include "sphinxpq.h"
#include "accumulator.h"

#define BINLOG_WRITE_BUFFER		(256*1024)
#define BINLOG_AUTO_FLUSH		1000000 // 1 sec

static const DWORD		BINLOG_HEADER_MAGIC = 0x4c425053;	/// magic 'SPBL' header that marks binlog file
static const DWORD		BLOP_MAGIC = 0x214e5854;			/// magic 'TXN!' header that marks binlog entry
static const DWORD		BINLOG_META_MAGIC = 0x494c5053;		/// magic 'SPLI' header that marks binlog meta

//////////////////////////////////////////////////////////////////////////
// BINLOG
//////////////////////////////////////////////////////////////////////////

/// binlog file view of the index
/// everything that a given log file needs to know about an index
struct BinlogIndexInfo_t
{
	CSphString	m_sName;				///< index name
	int64_t		m_iMinTID = INT64_MAX;	///< min TID logged by this file
	int64_t		m_iMaxTID = 0;			///< max TID logged by this file
	int64_t		m_iFlushedTID = 0;		///< last flushed TID
	int64_t		m_tmMin = INT64_MAX;	///< min TID timestamp
	int64_t		m_tmMax = 0;			///< max TID timestamp

	CSphIndex *	m_pIndex = nullptr;		///< replay only; associated index (might be NULL if we don't serve it anymore!)
//	RtIndex_c *	m_pRT = nullptr;		///< replay only; RT index handle (might be NULL if N/A or non-RT)
//	PercolateIndex_i *	m_pPQ = nullptr;		///< replay only; PQ index handle (might be NULL if N/A or non-PQ)
	int64_t		m_iPreReplayTID = 0;	///< replay only; index TID at the beginning of this file replay
};

/// binlog file descriptor
/// file id (aka extension), plus a list of associated index infos
struct BinlogFileDesc_t
{
	int								m_iExt = 0;
	CSphVector<BinlogIndexInfo_t>	m_dIndexInfos;
};


class BinlogWriter_c : public MemoryWriter2_c
{
public:
					BinlogWriter_c() : MemoryWriter2_c(m_dBuf) {}
					~BinlogWriter_c() { CloseFile(); }

	bool			Write ( bool bRemoveUnsuccessful = true );
	bool			Fsync();
	int64_t			GetPos() const				{ return m_iFilePos; }

	bool			OpenFile ( const CSphString & sFile, CSphString & sError );
	void			CloseFile();
	const CSphString & GetError() const			{ return m_sError; }

	bool			HasUnwrittenData () const	{ return m_dBuf.GetLength()>0; }
	bool			HasUnsyncedData () const	{ return m_iLastFsyncPos!=m_iFilePos; }

	void			StartTransaction();
	void			EndTransaction ( bool bWriteOnOverflow );

private:
	CSphAutofile	m_tFile;
	CSphVector<BYTE> m_dBuf;
	CSphString		m_sError;

	int64_t			m_iFilePos = 0;
	int64_t			m_iLastFsyncPos = 0;
	int				m_iLastCrcPos = 0;
	int				m_iTransactionStartPos = 0;
	DWORD			m_uCRC = 0;

	void			CalculateHash();
};


class BinlogReader_c final : public CSphAutoreader
{
public:
					BinlogReader_c ();

	void			ResetCrc ();
	bool			CheckCrc ( const char * sOp, const char * sIndexName, int64_t iTid, int64_t iTxnPos );

private:
	DWORD			m_uCRC;
	int				m_iLastCrcPos;
	void			UpdateCache () final;
	void			HashCollected ();
};

using namespace Binlog;

class Binlog_c : public ISphNoncopyable
{
public:
			~Binlog_c();

	void	NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown );
	bool	BinlogCommit ( Blop_e eOp, int64_t * pTID, const char * szIndexName, bool bIncTID, FnWriteCommit && fnSaver, CSphString & sError );

	void	Configure ( const CSphConfigSection & hSearchd, bool bTestMode, DWORD uReplayFlags );
	void	Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, ProgressCallbackSimple_t * pfnProgressCallback );

	bool	IsActive () const { return !m_bDisabled; }
	void	CheckPath ( const CSphConfigSection & hSearchd, bool bTestMode );

	bool	IsFlushingEnabled() const;
	void	DoFlush ();
	int64_t	NextFlushingTime() const;

	CSphString GetLogPath() const;

private:
	volatile int64_t		m_iLastFlushed = 0;
	volatile int64_t		m_iFlushPeriod = BINLOG_AUTO_FLUSH;

	enum OnCommitAction_e
	{
		ACTION_NONE,
		ACTION_FSYNC,
		ACTION_WRITE
	};
	OnCommitAction_e		m_eOnCommit { ACTION_NONE };

	CSphMutex				m_tWriteLock; // lock on operation

	int						m_iLockFD = -1;
	CSphString				m_sWriterError;
	BinlogWriter_c			m_tWriter;

	mutable CSphVector<BinlogFileDesc_t>	m_dLogFiles; // active log files

	CSphString				m_sLogPath;

	bool					m_bReplayMode = false; // replay mode indicator
	bool					m_bDisabled = true;
	DWORD					m_uReplayFlags = 0;

	int						m_iRestartSize = 268435456; // binlog size restart threshold, 256M

private:

	int 					GetWriteIndexID ( const char * sName, int64_t iTID, int64_t tmNow );
	void					LoadMeta ();
	void					SaveMeta ();
	void					LockFile ( bool bLock );
	void					DoCacheWrite ();
	void					CheckDoRestart ();
	bool					CheckDoFlush();
	void					OpenNewLog ( int iLastState=0 );

	int						ReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, int iBinlog );
	bool					ReplayTxn ( Binlog::Blop_e eOp, int iBinlog, BinlogReader_c & tReader ) const;
	bool					ReplayUpdateAttributes ( int iBinlog, BinlogReader_c & tReader ) const;
	bool					ReplayIndexAdd ( int iBinlog, const SmallStringHash_T<CSphIndex*> & hIndexes, BinlogReader_c & tReader ) const;
	bool					ReplayCacheAdd ( int iBinlog, BinlogReader_c & tReader ) const;
	bool 					IsBinlogWritable ( int64_t * pTID = nullptr );

	static bool	CheckCrc ( const char * sOp, const CSphString & sIndex, int64_t iTID, int64_t iTxnPos, BinlogReader_c & tReader ) ;
	bool		CheckTid ( const char * sOp, const BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t iTxnPos ) const;

	void	CheckTidSeq ( const char * sOp, const BinlogIndexInfo_t & tIndex, int64_t iTID, CSphIndex * pIndexTID, int64_t iTxnPos ) const;
	bool	CheckTime ( BinlogIndexInfo_t & tIndex, const char * sOp, int64_t tmStamp, int64_t iTID, int64_t iTxnPos ) const;
	bool	PerformChecks ( const char * szOp, BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t iTxnPos, int64_t tmStamp, BinlogReader_c & tReader ) const;
	static void	UpdateIndexInfo ( BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t tmStamp ) ;

	void	Log ( DWORD uFlag, const char * sTemplate, ... ) const;
	int		ReplayIndexID ( BinlogReader_c & tReader, const BinlogFileDesc_t & tLog, const char * sPlace ) const;
};

static Binlog_c *		g_pRtBinlog				= nullptr;

//////////////////////////////////////////////////////////////////////////
// BINLOG
//////////////////////////////////////////////////////////////////////////

static CSphString MakeBinlogName ( const char * sPath, int iExt )
{
	CSphString sName;
	sName.SetSprintf ( "%s/binlog.%03d", sPath, iExt );
	return sName;
}

//////////////////////////////////////////////////////////////////////////
class BinlogTransactionGuard_c
{
public:
	BinlogTransactionGuard_c ( BinlogWriter_c & tWriter, bool bWriteOnOverflow = true )
		: m_tWriter ( tWriter )
		, m_bWriteOnOverflow ( bWriteOnOverflow )
	{
		m_tWriter.StartTransaction();
	}

	~BinlogTransactionGuard_c()
	{
		m_tWriter.EndTransaction ( m_bWriteOnOverflow );
	}

private:
	BinlogWriter_c &	m_tWriter;
	bool				m_bWriteOnOverflow = true;
};

//////////////////////////////////////////////////////////////////////////

void BinlogWriter_c::StartTransaction()
{
	m_iTransactionStartPos = m_dBuf.GetLength();

	PutDword ( BLOP_MAGIC ); 

	m_uCRC = 0;
	m_iLastCrcPos = m_dBuf.GetLength();
}


void BinlogWriter_c::EndTransaction ( bool bWriteOnOverflow )
{
	CalculateHash();
	PutDword(m_uCRC);
	m_uCRC = 0;
	m_iLastCrcPos = m_dBuf.GetLength();

	// try to write if buffer gets too large but don't handle write errors just yet
	// also, don't remove unsuccessful transactions from the buffer
	if ( bWriteOnOverflow && m_dBuf.GetLength()>BINLOG_WRITE_BUFFER )
		Write(false);
}


void BinlogWriter_c::CalculateHash()
{
	assert ( m_iLastCrcPos<=m_dBuf.GetLength() );
	m_uCRC = sphCRC32 ( m_dBuf.Begin() + m_iLastCrcPos, m_dBuf.GetLength() - m_iLastCrcPos, m_uCRC );
	m_iLastCrcPos = m_dBuf.GetLength();
}


bool BinlogWriter_c::Write ( bool bRemoveUnsuccessful )
{
	if ( !m_dBuf.GetLength() )
		return true;

	if ( !sphWriteThrottled ( m_tFile.GetFD(), m_dBuf.Begin(), m_dBuf.GetLength(), m_tFile.GetFilename(), m_sError ) )
	{
		// if we got a partial write, clamp the file at the end of last written transaction
		sphSeek ( m_tFile.GetFD(), m_iFilePos, SEEK_SET );
		sphTruncate ( m_tFile.GetFD() );

		if ( bRemoveUnsuccessful )
		{
			// remove last transaction from memory buffer
			// other unwritten transactions may still be in the buffer, but we can't tell the daemon that they failed at this point
			// so we remove only the last one
			m_dBuf.Resize(m_iTransactionStartPos);
		}

		return false;
	}
	
	m_iFilePos += m_dBuf.GetLength();
	m_iLastCrcPos = 0;
	m_iTransactionStartPos = 0;
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


bool BinlogWriter_c::Fsync()
{
	if ( HasUnwrittenData() && !Write() )
		return false;

	if ( !HasUnsyncedData() )
		return true;

	if ( fsync ( m_tFile.GetFD() )!=0 )
	{
		m_sError.SetSprintf ( "failed to sync %s: %s" , m_tFile.GetFilename(), strerrorm(errno) );
		return false;
	}

	m_iLastFsyncPos = m_iFilePos;
	return true;
}


bool BinlogWriter_c::OpenFile ( const CSphString & sFile, CSphString & sError )
{
	m_iFilePos = 0;
	return m_tFile.Open ( sFile, SPH_O_NEW, sError )>=0;
}


void BinlogWriter_c::CloseFile()
{
	if ( HasUnwrittenData() )
		Write();

	m_tFile.Close();

	m_iLastFsyncPos = m_iFilePos = 0;
}

//////////////////////////////////////////////////////////////////////////

BinlogReader_c::BinlogReader_c()
{
	ResetCrc ();
}

void BinlogReader_c::ResetCrc ()
{
	m_uCRC = 0;
	m_iLastCrcPos = m_iBuffPos;
}


bool BinlogReader_c::CheckCrc ( const char * sOp, const char * sIndexName, int64_t iTid, int64_t iTxnPos )
{
	HashCollected ();
	DWORD uCRC = m_uCRC;
	DWORD uRef = CSphAutoreader::GetDword();
	ResetCrc();
	bool bPassed = ( uRef==uCRC );
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

Binlog_c::~Binlog_c ()
{
	if ( !m_bDisabled )
	{
		DoCacheWrite();
		m_tWriter.CloseFile();
		LockFile ( false );
	}
}

// here's been going binlogs with ALL closed indices removing
void Binlog_c::NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown )
{
	if ( m_bReplayMode )
		sphInfo ( "table '%s': ramchunk saved. TID=" INT64_FMT "", sIndexName, iTID );

	if (!IsBinlogWritable ())
		return;

	MEMORY ( MEM_BINLOG );
	assert ( bShutdown || m_dLogFiles.GetLength() );

	ScopedMutex_t tWriteLock ( m_tWriteLock );

	bool bCurrentLogAbandoned = false;
	const int iPreflushFiles = m_dLogFiles.GetLength();

	// loop through all log files, and check if we can unlink any
	ARRAY_FOREACH ( iLog, m_dLogFiles )
	{
		BinlogFileDesc_t & tLog = m_dLogFiles[iLog];
		bool bUsed = false;

		// update index info for this log file
		for ( auto& tIndex : tLog.m_dIndexInfos )
		{
			// this index was just flushed, update flushed TID
			if ( tIndex.m_sName==sIndexName )
			{
				assert ( iTID>=tIndex.m_iFlushedTID );
				tIndex.m_iFlushedTID = Max ( tIndex.m_iFlushedTID, iTID );
			}

			// if max logged TID is greater than last flushed TID, log file still has needed recovery data
			if ( tIndex.m_iFlushedTID < tIndex.m_iMaxTID )
				bUsed = true;
		}

		// it's needed, keep looking
		if ( bUsed )
			continue;

		// hooray, we can remove this log!
		// if this is our current log, we have to close it first
		if ( iLog==m_dLogFiles.GetLength()-1 )
		{
			m_tWriter.CloseFile ();
			bCurrentLogAbandoned = true;
		}

		// do unlink
		CSphString sLog = MakeBinlogName ( m_sLogPath.cstr(), tLog.m_iExt );
		if ( ::unlink ( sLog.cstr() ) )
			sphWarning ( "binlog: failed to unlink %s: %s", sLog.cstr(), strerrorm(errno) );

		// we need to reset it, otherwise there might be leftover data after last Remove()
		m_dLogFiles[iLog] = BinlogFileDesc_t();
		// quit tracking it
		m_dLogFiles.Remove ( iLog-- );
	}

	if ( bCurrentLogAbandoned && !bShutdown )
	{
		// if current log was closed, we need a new one (it will automatically save meta, too)
		OpenNewLog ();

	} else if ( iPreflushFiles!=m_dLogFiles.GetLength() )
	{
		// if we unlinked any logs, we need to save meta, too
		SaveMeta ();
	}
}

#ifndef LOCALDATADIR
#define LOCALDATADIR "."
#endif

void Binlog_c::Configure ( const CSphConfigSection & hSearchd, bool bTestMode, DWORD uReplayFlags )
{
	MEMORY ( MEM_BINLOG );

	const int iMode = hSearchd.GetInt ( "binlog_flush", 2 );
	switch ( iMode )
	{
		case 0:		m_eOnCommit = ACTION_NONE; break;
		case 1:		m_eOnCommit = ACTION_FSYNC; break;
		case 2:		m_eOnCommit = ACTION_WRITE; break;
		default:	sphDie ( "unknown binlog flush mode %d (must be 0, 1, or 2)\n", iMode );
	}

	m_sLogPath = hSearchd.GetStr ( "binlog_path", bTestMode ? "" : LOCALDATADIR );
	m_bDisabled = m_sLogPath.IsEmpty();

	m_iRestartSize = hSearchd.GetSize ( "binlog_max_log_size", m_iRestartSize );
	m_uReplayFlags = uReplayFlags;

	if ( !m_bDisabled )
	{
		LockFile ( true );
		LoadMeta();
	}
}

void Binlog_c::Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, ProgressCallbackSimple_t * pfnProgressCallback )
{
	if ( m_bDisabled )
		return;

	// on replay started
	if ( pfnProgressCallback )
		pfnProgressCallback();

	int64_t tmReplay = sphMicroTimer();

	// do replay
	m_bReplayMode = true;
	int iLastLogState = 0;
	ARRAY_FOREACH ( i, m_dLogFiles )
	{
		iLastLogState = ReplayBinlog ( hIndexes, i );
		if ( pfnProgressCallback ) // on each replayed binlog
			pfnProgressCallback();
	}

	if ( m_dLogFiles.GetLength()>0 )
	{
		tmReplay = sphMicroTimer() - tmReplay;
		sphInfo ( "binlog: finished replaying total %d in %d.%03d sec",
			m_dLogFiles.GetLength(),
			(int)(tmReplay/1000000), (int)((tmReplay/1000)%1000) );
	}

	// FIXME?
	// in some cases, indexes might had been flushed during replay
	// and we might therefore want to update m_iFlushedTID everywhere
	// but for now, let's just wait until next flush for simplicity

	// resume normal operation
	m_bReplayMode = false;
	OpenNewLog ( iLastLogState );
}


bool Binlog_c::IsFlushingEnabled () const
{
	return ( !m_bDisabled && m_eOnCommit!=ACTION_FSYNC );
}


void Binlog_c::DoFlush ()
{
	assert ( !m_bDisabled );
	MEMORY ( MEM_BINLOG );

	if ( m_eOnCommit==ACTION_NONE || m_tWriter.HasUnwrittenData() )
	{
		ScopedMutex_t LockWriter( m_tWriteLock);
		if ( !m_tWriter.Write() )
			return;
	}

	if ( m_tWriter.HasUnsyncedData() && !m_tWriter.Fsync() )
		return;

	m_iLastFlushed = sphMicroTimer ();
}

int64_t Binlog_c::NextFlushingTime () const
{
	if ( !m_iLastFlushed )
		return sphMicroTimer () + m_iFlushPeriod;
	return m_iLastFlushed + m_iFlushPeriod;
}

int Binlog_c::GetWriteIndexID ( const char * sName, int64_t iTID, int64_t tmNow )
{
	MEMORY ( MEM_BINLOG );
	assert ( m_dLogFiles.GetLength() );

	// OPTIMIZE? maybe hash them?
	BinlogFileDesc_t & tLog = m_dLogFiles.Last();
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
	{
		BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];
		if ( tIndex.m_sName==sName )
		{
			tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
			tIndex.m_tmMax = Max ( tIndex.m_tmMax, tmNow );
			return i;
		}
	}

	// create a new entry
	int iID = tLog.m_dIndexInfos.GetLength();
	BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos.Add(); // caller must hold a wlock
	tIndex.m_sName = sName;
	tIndex.m_iMinTID = iTID;
	tIndex.m_iMaxTID = iTID;
	tIndex.m_iFlushedTID = 0;
	tIndex.m_tmMin = tmNow;
	tIndex.m_tmMax = tmNow;

	// log this new entry
	BinlogTransactionGuard_c tGuard ( m_tWriter, false );

	m_tWriter.ZipOffset ( ADD_INDEX );
	m_tWriter.ZipOffset ( iID );
	m_tWriter.PutString ( sName );
	m_tWriter.ZipOffset ( iTID );
	m_tWriter.ZipOffset ( tmNow );

	// return the index
	return iID;
}

void Binlog_c::LoadMeta ()
{
	MEMORY ( MEM_BINLOG );

	CSphString sMeta;
	sMeta.SetSprintf ( "%s/binlog.meta", m_sLogPath.cstr() );
	if ( !sphIsReadable ( sMeta.cstr() ) )
		return;

	CSphString sError;

	// opened and locked, lets read
	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMeta, sError ) )
		sphDie ( "%s error: %s", sMeta.cstr(), sError.cstr() );

	if ( rdMeta.GetDword()!=BINLOG_META_MAGIC )
		sphDie ( "invalid meta file %s", sMeta.cstr() );

	// binlog meta v1 was dev only, crippled, and we don't like it anymore
	// binlog metas v2 upto current v4 (and likely up) share the same simplistic format
	// so let's support empty (!) binlogs w/ known versions and compatible metas
	DWORD uVersion = rdMeta.GetDword();
	if ( uVersion==1 || uVersion>BINLOG_VERSION )
		sphDie ( "binlog meta file %s is v.%d, binary is v.%d; recovery requires previous binary version",
			sMeta.cstr(), uVersion, BINLOG_VERSION );

	const bool bLoaded64bit = ( rdMeta.GetByte()==1 );
	m_dLogFiles.Resize ( rdMeta.UnzipInt() ); // FIXME! sanity check

	if ( !m_dLogFiles.GetLength() )
		return;

	// ok, so there is actual recovery data
	// let's require that exact version and bitness, then
	if ( uVersion!=BINLOG_VERSION )
		sphDie ( "binlog meta file %s is v.%d, binary is v.%d; recovery requires previous binary version",
			sMeta.cstr(), uVersion, BINLOG_VERSION );

	if ( !bLoaded64bit )
		sphDie ( "tables with 32-bit docids are no longer supported; recovery requires previous binary version" );

	// load list of active log files
	ARRAY_FOREACH ( i, m_dLogFiles )
		m_dLogFiles[i].m_iExt = rdMeta.UnzipInt(); // everything else is saved in logs themselves
}

void Binlog_c::SaveMeta ()
{
	MEMORY ( MEM_BINLOG );

	CSphString sMeta, sMetaOld;
	sMeta.SetSprintf ( "%s/binlog.meta.new", m_sLogPath.cstr() );
	sMetaOld.SetSprintf ( "%s/binlog.meta", m_sLogPath.cstr() );

	CSphString sError;

	// opened and locked, lets write
	CSphWriter wrMeta;
	if ( !wrMeta.OpenFile ( sMeta, sError ) )
		sphDie ( "failed to open '%s': '%s'", sMeta.cstr(), sError.cstr() );

	wrMeta.PutDword ( BINLOG_META_MAGIC );
	wrMeta.PutDword ( BINLOG_VERSION );
	wrMeta.PutByte ( 1 ); // was USE_64BIT

	// save list of active log files
	wrMeta.ZipInt ( m_dLogFiles.GetLength() );
	ARRAY_FOREACH ( i, m_dLogFiles )
		wrMeta.ZipInt ( m_dLogFiles[i].m_iExt ); // everything else is saved in logs themselves

	wrMeta.CloseFile();

	if ( wrMeta.IsError() )
	{
		sphWarning ( "%s", sError.cstr() );
		return;
	}

	if ( sph::rename ( sMeta.cstr(), sMetaOld.cstr() ) )
		sphDie ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)",
			sMeta.cstr(), sMetaOld.cstr(), errno, strerrorm(errno) ); // !COMMIT handle this gracefully
	sphLogDebug ( "SaveMeta: Done (%s)", sMeta.cstr() );
}

void Binlog_c::LockFile ( bool bLock )
{
	CSphString sName;
	sName.SetSprintf ( "%s/binlog.lock", m_sLogPath.cstr() );

	if ( bLock )
	{
		assert ( m_iLockFD==-1 );
		const int iLockFD = ::open ( sName.cstr(), SPH_O_NEW, 0644 );

		if ( iLockFD<0 )
			sphDie ( "failed to open '%s': %u '%s'", sName.cstr(), errno, strerrorm(errno) );

		if ( !sphLockEx ( iLockFD, false ) )
			sphDie ( "failed to lock '%s': %u '%s'", sName.cstr(), errno, strerrorm(errno) );

		m_iLockFD = iLockFD;
	} else
	{
		if ( m_iLockFD>=0 )
			sphLockUn ( m_iLockFD );
		SafeClose ( m_iLockFD );
		::unlink ( sName.cstr()	);
	}
}

void Binlog_c::OpenNewLog ( int iLastState )
{
	MEMORY ( MEM_BINLOG );

	// calc new ext
	int iExt = 1;
	if ( m_dLogFiles.GetLength() )
	{
		iExt = m_dLogFiles.Last().m_iExt;
		if ( !iLastState )
			iExt++;
	}

	// create entry
	// we need to reset it, otherwise there might be leftover data after last Remove()
	BinlogFileDesc_t tLog;
	tLog.m_iExt = iExt;
	m_dLogFiles.Add ( tLog );

	// update meta first then only remove binlog file
	SaveMeta();

	// create file
	CSphString sLog = MakeBinlogName ( m_sLogPath.cstr(), tLog.m_iExt );

	if ( !iLastState ) // reuse the last binlog since it is empty or useless.
		::unlink ( sLog.cstr() );

	if ( !m_tWriter.OpenFile ( sLog, m_sWriterError ) )
		sphDie ( "failed to create %s: errno=%d, error=%s", sLog.cstr(), errno, strerrorm(errno) );

	// emit header
	m_tWriter.PutDword ( BINLOG_HEADER_MAGIC );
	m_tWriter.PutDword ( BINLOG_VERSION );
}

// cache is a small summary of affected indexes, it is written at the very end of binlog file when it exceeded size limit,
// before opening new file.
void Binlog_c::DoCacheWrite ()
{
	if ( !m_dLogFiles.GetLength() )
		return;
	const CSphVector<BinlogIndexInfo_t> & dIndexes = m_dLogFiles.Last().m_dIndexInfos;

	BinlogTransactionGuard_c tGuard(m_tWriter);

	m_tWriter.ZipOffset ( ADD_CACHE );
	m_tWriter.ZipOffset ( dIndexes.GetLength() );
	ARRAY_FOREACH ( i, dIndexes )
	{
		m_tWriter.PutString ( dIndexes[i].m_sName.cstr() );
		m_tWriter.ZipOffset ( dIndexes[i].m_iMinTID );
		m_tWriter.ZipOffset ( dIndexes[i].m_iMaxTID );
		m_tWriter.ZipOffset ( dIndexes[i].m_iFlushedTID );
		m_tWriter.ZipOffset ( dIndexes[i].m_tmMin );
		m_tWriter.ZipOffset ( dIndexes[i].m_tmMax );
	}
}

void Binlog_c::CheckDoRestart ()
{
	// restart on exceed file size limit
	if ( m_iRestartSize>0 && m_tWriter.GetPos()>m_iRestartSize )
	{
		MEMORY ( MEM_BINLOG );

		assert ( m_dLogFiles.GetLength() );

		DoCacheWrite();
		m_tWriter.CloseFile();
		OpenNewLog();
	}
}

bool Binlog_c::CheckDoFlush()
{
	switch ( m_eOnCommit )
	{
	case ACTION_NONE:
		break;

	case ACTION_WRITE:
		if ( !m_tWriter.Write() )
			return false;
		break;

	case ACTION_FSYNC:
		if ( !m_tWriter.Write() )
			return false;

		if ( !m_tWriter.Fsync() )
			return false;
		break;

	default:
		assert(false && "wrong binlog flush action flag");
		break;
	}

	return true;
}

int Binlog_c::ReplayBinlog ( const SmallStringHash_T<CSphIndex*> & hIndexes, int iBinlog )
{
	assert ( iBinlog>=0 && iBinlog<m_dLogFiles.GetLength() );
	CSphString sError;

	const CSphString sLog ( MakeBinlogName ( m_sLogPath.cstr(), m_dLogFiles[iBinlog].m_iExt ) );
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	// open, check, play
	sphInfo ( "binlog: replaying log %s", sLog.cstr() );

	BinlogReader_c tReader;
	if ( !tReader.Open ( sLog, sError ) )
	{
		Log ( REPLAY_IGNORE_OPEN_ERROR, "binlog: log open error: %s", sError.cstr() );
		return 0;
	}

	const SphOffset_t iFileSize = tReader.GetFilesize();

	if ( !iFileSize )
	{
		sphWarning ( "binlog: empty binlog %s detected, skipping", sLog.cstr() );
		return -1;
	}

	if ( tReader.GetDword()!=BINLOG_HEADER_MAGIC )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: log %s missing magic header (corrupted?)", sLog.cstr() );
		return -1;
	}

	DWORD uVersion = tReader.GetDword();
	if ( uVersion!=BINLOG_VERSION || tReader.GetErrorFlag() )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: log %s is v.%d, binary is v.%d; recovery requires previous binary version", sLog.cstr(), uVersion, BINLOG_VERSION );
		return -1;
	}

	/////////////
	// do replay
	/////////////

	int dTotal [ TOTAL+1 ] = {0};

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
		if ( tReader.GetDword()!=BLOP_MAGIC )
		{
			Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: log missing txn marker at pos=" INT64_FMT " (corrupted?)", iPos );
			bReplayOK = false;
			break;
		}

		tReader.ResetCrc ();
		const uint64_t uOp = tReader.UnzipOffset ();

		if ( uOp<=0 || uOp>=TOTAL )
		{
			Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: unexpected entry (blop=" UINT64_FMT ", pos=" INT64_FMT ")", uOp, iPos );
			bReplayOK = false;
			break;
		}

		// FIXME! blop might be OK but skipped (eg. index that is no longer)
		switch ( uOp )
		{
			case ADD_INDEX:
				bReplayOK = ReplayIndexAdd ( iBinlog, hIndexes, tReader );
				break;

			case ADD_CACHE:
				if ( bHaveCacheOp )
				{
					Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: internal error, second BLOP_ADD_CACHE detected (corruption?)" );
					bReplayOK = false;
					break;
				}
				bHaveCacheOp = true;
				bReplayOK = ReplayCacheAdd ( iBinlog, tReader );
				break;

			case UPDATE_ATTRS:
				bReplayOK = ReplayUpdateAttributes ( iBinlog, tReader );
				break;

			case COMMIT:
			case RECONFIGURE:
			case PQ_ADD:
			case PQ_DELETE:
			case PQ_ADD_DELETE:
				bReplayOK = ReplayTxn ( Binlog::Blop_e(uOp), iBinlog, tReader );
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

	sphInfo ( "binlog: replay stats: %d commits; %d updates, %d reconfigure; %d pq-add; %d pq-delete; %d pq-add-delete, %d tables",
		dTotal[COMMIT], dTotal[UPDATE_ATTRS], dTotal[RECONFIGURE], dTotal[PQ_ADD], dTotal[PQ_DELETE],
			dTotal[PQ_ADD_DELETE], dTotal[ADD_INDEX] );
	sphInfo ( "binlog: finished replaying %s; %d.%d MB in %d.%03d sec",
		sLog.cstr(),
		(int)(iFileSize/1048576), (int)((iFileSize*10/1048576)%10),
		(int)(tmReplay/1000000), (int)((tmReplay/1000)%1000) );

	// only one operation, that is Add Cache - by the fact, empty binlog
	return ( bHaveCacheOp && dTotal[TOTAL]==1 ) ? 1 : 0;
}

bool Binlog_c::ReplayIndexAdd ( int iBinlog, const SmallStringHash_T<CSphIndex*> & hIndexes, BinlogReader_c & tReader ) const
{
	// load and check index
	const int64_t iTxnPos = tReader.GetPos(); // that is purely for reporting anomalities
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	uint64_t uVal = tReader.UnzipOffset();
	if ( (int)uVal!=tLog.m_dIndexInfos.GetLength() )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: indexadd: unexpected table id (id=" UINT64_FMT ", expected=%d, pos=" INT64_FMT ")",
			uVal, tLog.m_dIndexInfos.GetLength(), iTxnPos );
		return false;
	}

	// load data
	CSphString sName = tReader.GetString();

	// FIXME? use this for double checking?
	tReader.UnzipOffset (); // TID
	tReader.UnzipOffset (); // time

	if ( !tReader.CheckCrc ( "indexadd", sName.cstr(), 0, iTxnPos ) )
		return false;

	// check for index name dupes
	ARRAY_FOREACH ( i, tLog.m_dIndexInfos )
		if ( tLog.m_dIndexInfos[i].m_sName==sName )
		{
			Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: duplicate table name (name=%s, dupeid=%d, pos=" INT64_FMT ")",
				sName.cstr(), i, iTxnPos );
			return false;
		}

	// not a dupe, lets add
	BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos.Add();
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

bool Binlog_c::ReplayCacheAdd ( int iBinlog, BinlogReader_c & tReader ) const
{
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	// load data
	CSphVector<BinlogIndexInfo_t> dCache;
	dCache.Resize ( (int) tReader.UnzipOffset() ); // FIXME! sanity check
	ARRAY_FOREACH ( i, dCache )
	{
		dCache[i].m_sName = tReader.GetString();
		dCache[i].m_iMinTID = tReader.UnzipOffset();
		dCache[i].m_iMaxTID = tReader.UnzipOffset();
		dCache[i].m_iFlushedTID = tReader.UnzipOffset();
		dCache[i].m_tmMin = tReader.UnzipOffset();
		dCache[i].m_tmMax = tReader.UnzipOffset();
	}
	if ( !tReader.CheckCrc ( "cache", "", 0, iTxnPos ) )
		return false;

	// if we arrived here by replay, let's verify everything
	// note that cached infos just passed checksumming, so the file is supposed to be clean!
	// in any case, broken log or not, we probably managed to replay something
	// so let's just report differences as warnings

	if ( dCache.GetLength()!=tLog.m_dIndexInfos.GetLength() )
	{
		sphWarning ( "binlog: cache mismatch: %d tables cached, %d replayed",
			dCache.GetLength(), tLog.m_dIndexInfos.GetLength() );
		return true;
	}

	ARRAY_FOREACH ( i, dCache )
	{
		BinlogIndexInfo_t & tCache = dCache[i];
		BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[i];

		if ( tCache.m_sName!=tIndex.m_sName )
		{
			sphWarning ( "binlog: cache mismatch: table %d name mismatch (%s cached, %s replayed)",
				i, tCache.m_sName.cstr(), tIndex.m_sName.cstr() );
			continue;
		}

		if ( tCache.m_iMinTID!=tIndex.m_iMinTID || tCache.m_iMaxTID!=tIndex.m_iMaxTID )
		{
			sphWarning ( "binlog: cache mismatch: table %s tid ranges mismatch "
				"(cached " INT64_FMT " to " INT64_FMT ", replayed " INT64_FMT " to " INT64_FMT ")",
				tCache.m_sName.cstr(),
				tCache.m_iMinTID, tCache.m_iMaxTID, tIndex.m_iMinTID, tIndex.m_iMaxTID );
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
/// custom replay stuff
//////////////////////////////////////////////////////////////////////////

// helper used in about all replay ops
int Binlog_c::ReplayIndexID ( BinlogReader_c & tReader, const BinlogFileDesc_t & tLog, const char * sPlace ) const
{
	const int64_t iTxnPos = tReader.GetPos();
	const int iVal = (int)tReader.UnzipOffset();

	if ( iVal<0 || iVal>=tLog.m_dIndexInfos.GetLength() )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: %s: unexpected table id (id=%d, max=%d, pos=" INT64_FMT ")",
			sPlace, iVal, tLog.m_dIndexInfos.GetLength(), iTxnPos );
		return -1;
	}

	return iVal;
}

CSphString Binlog_c::GetLogPath() const
{
	return m_sLogPath;
}

static const char* OpName ( Binlog::Blop_e eOp)
{
	switch (eOp)
	{
		case Binlog::COMMIT: return "commit";
		case Binlog::RECONFIGURE: return "reconfigure";
		case Binlog::PQ_ADD: return "pq-add";
		case Binlog::PQ_DELETE: return "pq-delete";
		case Binlog::PQ_ADD_DELETE: return "pq-add-delete";
		default: return "other";
	}
}

// dedicated function for replay attribute update
bool Binlog_c::ReplayUpdateAttributes ( int iBinlog, BinlogReader_c & tReader ) const
{
	// load and lookup index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];
	
	int iIdx = ReplayIndexID ( tReader, tLog, "update" );
	if ( iIdx==-1 )
		return false;
	BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[iIdx];

	auto iTID = (int64_t) tReader.UnzipOffset();
	auto tmStamp = (int64_t) tReader.UnzipOffset();

	// load transaction data
	AttrUpdateSharedPtr_t pUpd { new CSphAttrUpdate };
	auto& tUpd = *pUpd;
	tUpd.m_bIgnoreNonexistent = true;

	int iAttrs = (int)tReader.UnzipOffset();
	tUpd.m_dAttributes.Resize ( iAttrs ); // FIXME! sanity check
	for ( auto & i : tUpd.m_dAttributes )
	{
		i.m_sName = tReader.GetString();
		i.m_eType = (ESphAttr) tReader.UnzipOffset(); // safe, we'll crc check later
	}

	if ( tReader.GetErrorFlag() ) return false;

	if ( !Binlog::LoadVector ( tReader, tUpd.m_dPool ) ) return false;
	if ( !Binlog::LoadVector ( tReader, tUpd.m_dDocids ) ) return false;
	if ( !Binlog::LoadVector ( tReader, tUpd.m_dRowOffset ) ) return false;
	if ( !Binlog::LoadVector ( tReader, tUpd.m_dBlobs ) ) return false;

	if (!PerformChecks ( "update", tIndex, iTID, iTxnPos, tmStamp, tReader ))
		return false;

	if ( tIndex.m_pIndex && iTID > tIndex.m_pIndex->m_iTID )
	{
		// we normally expect per-index TIDs to be sequential
		// but let's be graceful about that
		CheckTidSeq ("update", tIndex, iTID, tIndex.m_pIndex, iTxnPos );

		CSphString sError, sWarning;
		bool bCritical = false;
		tIndex.m_pIndex->UpdateAttributes ( pUpd, bCritical, sError, sWarning ); // FIXME! check for errors
		assert ( !bCritical ); // fixme! handle this

		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pIndex->m_iTID = iTID;
	}

	// update info
	UpdateIndexInfo (tIndex, iTID, tmStamp);
	return true;
}

bool Binlog_c::ReplayTxn ( Binlog::Blop_e eOp, int iBinlog, BinlogReader_c & tReader ) const
{
	// load and lookup index
	const int64_t iTxnPos = tReader.GetPos();
	BinlogFileDesc_t & tLog = m_dLogFiles[iBinlog];

	int iIdx = ReplayIndexID ( tReader, tLog, OpName (eOp) );
	if ( iIdx==-1 )
		return false;

	BinlogIndexInfo_t & tIndex = tLog.m_dIndexInfos[iIdx];

	// load transaction data
	auto iTID = (int64_t) tReader.UnzipOffset();
	auto tmStamp = (int64_t) tReader.UnzipOffset();

	if ( !tIndex.m_pIndex )
		return false;

	CSphString sError;
	CheckTnxResult_t tReplayed = tIndex.m_pIndex->ReplayTxn ( eOp, tReader, sError, [ eOp, iTxnPos, iTID, tmStamp, this, &tReader, &tIndex ] () {
		
		CheckTnxResult_t tRes;

		tRes.m_bValid = PerformChecks ( OpName (eOp), tIndex, iTID, iTxnPos, tmStamp, tReader );
		if ( !tRes.m_bValid )
			return tRes;

		// only replay transaction when index exists and does not have it yet (based on TID)
		if ( tIndex.m_pIndex && iTID > tIndex.m_pIndex->m_iTID )
		{
			// we normally expect per-index TIDs to be sequential
			// but let's be graceful about that
			CheckTidSeq ( OpName (eOp), tIndex, iTID, tIndex.m_pIndex, iTxnPos );
			tRes.m_bApply = true;
		}
		return tRes;
	});

	// could be invalid TXN in binlog
	if ( !tReplayed.m_bValid )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: %s (table=%s, lasttid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ", error=%s)",
			OpName ( eOp ), tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos, sError.cstr() );
		return false;
	}

	// could be TXN in binlog that index already has should not apply again that TXN and should not change index TID by that TXN
	if ( tReplayed.m_bApply )
	{
		// update committed tid on replay in case of unexpected / mismatched tid
		tIndex.m_pIndex->m_iTID = iTID;
	} 

	UpdateIndexInfo ( tIndex, iTID, tmStamp );
	return true;

}

void Binlog_c::CheckPath ( const CSphConfigSection & hSearchd, bool bTestMode )
{
	m_sLogPath = hSearchd.GetStr ( "binlog_path", bTestMode ? "" : LOCALDATADIR );
	m_bDisabled = m_sLogPath.IsEmpty();

	if ( !m_bDisabled )
	{
		LockFile ( true );
		LockFile ( false );
	}
}

bool Binlog_c::CheckCrc ( const char * sOp, const CSphString & sIndex, int64_t iTID, int64_t iTxnPos, BinlogReader_c & tReader )
{
	return !tReader.GetErrorFlag() && tReader.CheckCrc ( sOp, sIndex.cstr(), iTID, iTxnPos );
}

bool Binlog_c::CheckTid ( const char * sOp, const BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t iTxnPos ) const
{
	if ( iTID<tIndex.m_iMaxTID )
	{
		Log ( REPLAY_IGNORE_TRX_ERROR, "binlog: %s: descending tid (table=%s, lasttid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ")",
			sOp, tIndex.m_sName.cstr(), tIndex.m_iMaxTID, iTID, iTxnPos );
		return false;
	}

	return true;
}


void Binlog_c::CheckTidSeq ( const char * sOp, const BinlogIndexInfo_t & tIndex, int64_t iTID, CSphIndex * pIndexTID, int64_t iTxnPos ) const
{
	if ( pIndexTID && iTID!=pIndexTID->m_iTID+1 )
		sphWarning ( "binlog: %s: unexpected tid (table=%s, indextid=" INT64_FMT ", logtid=" INT64_FMT ", pos=" INT64_FMT ")",
			sOp, tIndex.m_sName.cstr(), pIndexTID->m_iTID, iTID, iTxnPos );
}

bool Binlog_c::CheckTime ( BinlogIndexInfo_t & tIndex, const char * sOp, int64_t tmStamp, int64_t iTID, int64_t iTxnPos ) const
{
	if ( tmStamp<tIndex.m_tmMax )
	{
		Log ( REPLAY_ACCEPT_DESC_TIMESTAMP, "binlog: %s: descending time (table=%s, lasttime=" INT64_FMT ", logtime=" INT64_FMT ", pos=" INT64_FMT ")",
				sOp, tIndex.m_sName.cstr(), tIndex.m_tmMax, tmStamp, iTxnPos );
		return false;
	}

	tIndex.m_tmMax = tmStamp;
	return true;
}


bool Binlog_c::PerformChecks ( const char * szOp, BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t iTxnPos, int64_t tmStamp,
		BinlogReader_c & tReader ) const
{
	// checksum
	if ( !CheckCrc ( szOp, tIndex.m_sName, iTID, iTxnPos, tReader ) )
		return false;

	// check TID
	if ( !CheckTid ( szOp, tIndex, iTID, iTxnPos ) )
		return false;

	// check timestamp
	if ( !CheckTime ( tIndex, szOp, tmStamp, iTID, iTxnPos ) )
		return false;

	return true;
}


void Binlog_c::UpdateIndexInfo ( BinlogIndexInfo_t & tIndex, int64_t iTID, int64_t tmStamp )
{
	tIndex.m_iMinTID = Min ( tIndex.m_iMinTID, iTID );
	tIndex.m_iMaxTID = Max ( tIndex.m_iMaxTID, iTID );
	tIndex.m_tmMin = Min ( tIndex.m_tmMin, tmStamp );
	tIndex.m_tmMax = Max ( tIndex.m_tmMax, tmStamp );
}

bool Binlog_c::IsBinlogWritable ( int64_t * pTID )
{
	if ( m_bReplayMode )
		return false;

	if ( !m_bDisabled )
		return true;

	if ( pTID ) // still need to advance TID as index flush according to it
	{
		ScopedMutex_t tLock ( m_tWriteLock );
		++(*pTID);
	}
	return false;
}

// commit stuff. Indexes call this function with serialization cb; binlog is agnostic to alien data structures.
bool Binlog_c::BinlogCommit ( Blop_e eOp, int64_t * pTID, const char * szIndexName, bool bIncTID, FnWriteCommit && fnSaver, CSphString & sError )
{
	if (!IsBinlogWritable ( bIncTID ? pTID : nullptr )) // m.b. need to advance TID as index flush according to it
		return true;

	MEMORY ( MEM_BINLOG );
	ScopedMutex_t tWriteLock ( m_tWriteLock );

	int64_t iTID = ++(*pTID);
	const int64_t tmNow = sphMicroTimer();
	const int uIndex = GetWriteIndexID ( szIndexName, iTID, tmNow );

	{
		BinlogTransactionGuard_c tGuard ( m_tWriter, m_eOnCommit==ACTION_NONE );

		// header
		m_tWriter.ZipOffset ( eOp );
		m_tWriter.ZipOffset ( uIndex );
		m_tWriter.ZipOffset ( iTID );
		m_tWriter.ZipOffset ( tmNow );

		// save txn data
		fnSaver ( m_tWriter );
	}

	// finalize
	if ( !CheckDoFlush() )
	{
		sError.SetSprintf ( "unable to write to binlog: %s", m_tWriter.GetError().cstr() );
		return false;
	}

	CheckDoRestart();
	return true;
}

static auto & g_bRTChangesAllowed = RTChangesAllowed();

void Binlog::Init ( const CSphConfigSection & hSearchd, bool bTestMode )
{
	MEMORY ( MEM_BINLOG );

	g_bRTChangesAllowed = false;
	g_pRtBinlog = new Binlog_c();
	if ( !g_pRtBinlog )
		sphDie ( "binlog: failed to create binlog" );

	// check binlog path before detaching from the console
	g_pRtBinlog->CheckPath ( hSearchd, bTestMode );
}

void Binlog::Configure ( const CSphConfigSection & hSearchd, bool bTestMode, DWORD uReplayFlags )
{
	assert ( g_pRtBinlog );
	g_pRtBinlog->Configure ( hSearchd, bTestMode, uReplayFlags );
}


void Binlog::Deinit ()
{
	// its valid for "searchd --stop" case
	SafeDelete ( g_pRtBinlog );
}


void Binlog::Replay ( const SmallStringHash_T<CSphIndex*> & hIndexes, ProgressCallbackSimple_t * pfnProgressCallback )
{
	MEMORY ( MEM_BINLOG );
	g_pRtBinlog->Replay ( hIndexes, pfnProgressCallback );
	g_bRTChangesAllowed = true;
}

// implemented here because used both in plain and rt indexes
void Binlog::CommitUpdateAttributes ( int64_t * pTID, const char * szIndexName, const CSphAttrUpdate & tUpd )
{
	CSphString sError;
	Binlog::Commit ( Binlog::UPDATE_ATTRS, pTID, szIndexName, false, sError, [&tUpd] ( Writer_i & tWriter ) {

		// update data
		tWriter.ZipOffset ( tUpd.m_dAttributes.GetLength() );
		for ( const auto & i : tUpd.m_dAttributes )
		{
			tWriter.PutString ( i.m_sName );
			tWriter.ZipOffset ( i.m_eType );
		}

		// POD vectors
		Binlog::SaveVector ( tWriter, tUpd.m_dPool );
		Binlog::SaveVector ( tWriter, tUpd.m_dDocids );
		Binlog::SaveVector ( tWriter, tUpd.m_dRowOffset );
		Binlog::SaveVector ( tWriter, tUpd.m_dBlobs );
	});
}

bool Binlog::IsActive()
{
	if (!g_pRtBinlog)
		return false;
	return g_pRtBinlog->IsActive();
}

bool Binlog::Commit ( Blop_e eOp, int64_t * pTID, const char * szIndexName, bool bIncTID, CSphString & sError, FnWriteCommit && fnSaver )
{
	if ( !g_pRtBinlog )
		return true;

	return g_pRtBinlog->BinlogCommit ( eOp, pTID, szIndexName, bIncTID, std::move (fnSaver), sError );
}

void Binlog::NotifyIndexFlush ( const char * sIndexName, int64_t iTID, bool bShutdown )
{
	if (!g_pRtBinlog)
		return;

	g_pRtBinlog->NotifyIndexFlush ( sIndexName, iTID, bShutdown );
}

CSphString Binlog::GetPath()
{
	if ( g_pRtBinlog )
		return g_pRtBinlog->GetLogPath();
	return "";
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
