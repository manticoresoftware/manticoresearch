//
// Copyright (c) 2026, Manticore Software LTD
//
// Indexer-assisted RT bulk loader.
//

#include "client_session.h"
#include "conversion.h"
#include "sphinxexpr.h"

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <ostream>
#include <stdlib.h>
#include <type_traits>

#include "indexer_rt_bulk.h"
#include "coroutine.h"

#include <boost/version.hpp>
#if BOOST_VERSION >= 108800
#define BOOST_PROCESS_VERSION 1
#include <boost/process/v1/args.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/error.hpp>
#include <boost/process/v1/extend.hpp>
#include <boost/process/v1/handles.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/pipe.hpp>
#else
#include <boost/process.hpp>
#include <boost/process/extend.hpp>
#endif

#if !_WIN32
#if BOOST_VERSION >= 108800
#include <boost/process/v1/posix.hpp>
#else
#include <boost/process/posix.hpp>
#endif
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace fs = boost::filesystem;

static const char * GetIndexerRtBulkCsvpipeCommand ()
{
	if ( env_exists ( "INDEXER_RT_BULK_CSV_PIPE_COMMAND" ) )
		return getenv ( "INDEXER_RT_BULK_CSV_PIPE_COMMAND" );
#if !_WIN32
	return "/bin/cat";
#else
	return "-";
#endif
}

DiskAttachRes_e AttachIndexerRtBulkChunk ( const CSphString & sTable, const ServedIndexWriteReservation_c & tReservation, int64_t iIndexId, int iAlterGeneration, const CSphString & sPath, bool & bTargetStale, CSphString & sError );


#if _WIN32
static Threads::Coro::Mutex_c g_tIndexerRtBulkLaunchMutex;
#endif


static bool ValidateFloatVectorValue ( const CSphColumnInfo & tAttr, const SqlInsert_t & tValue, int iRow, CSphString & sError )
{
	if ( tValue.m_iType==SqlInsert_t::TOK_NULL )
		return true;

	if ( tValue.m_iType!=SqlInsert_t::CONST_MVA || !tValue.m_pVals )
	{
		sError.SetSprintf ( "row %d, attribute '%s': float_vector requires a tuple value", iRow+1, tAttr.m_sName.cstr() );
		return false;
	}

	const auto & dValues = *tValue.m_pVals;
	if ( tAttr.IsIndexedKNN() && dValues.GetLength()!=tAttr.m_tKNN.m_iDims )
	{
		sError.SetSprintf ( "row %d, attribute '%s': KNN index requires %d vector entries; %d specified", iRow+1, tAttr.m_sName.cstr(), tAttr.m_tKNN.m_iDims, dValues.GetLength() );
		return false;
	}

	for ( const auto & tItem : dValues )
		if ( !std::isfinite ( tItem.m_fValue ) )
		{
			sError.SetSprintf ( "row %d, attribute '%s': float_vector entries must be finite", iRow+1, tAttr.m_sName.cstr() );
			return false;
		}

	return true;
}

static bool GetIndexerPath ( CSphString & sIndexer, CSphString & sError )
{
	CSphString sExecutable = GetExecutablePath();
	if ( sExecutable.IsEmpty() )
	{
		sError = "failed to locate the running searchd executable for bulk_import";
		return false;
	}

	#if _WIN32
	const char * szIndexer = "indexer.exe";
	#else
	const char * szIndexer = "indexer";
	#endif
	sIndexer.SetSprintf ( "%s%s", GetPathOnly ( sExecutable ).cstr(), szIndexer );
	return true;
}


CSphString GetIndexerRtBulkRoot ( const RtIndex_i & tRt )
{
	return SphSprintf ( "%s.indexer-rt-bulk", tRt.GetFilebase() );
}


static CSphString EscapeIndexerRtBulkConfigPath ( const CSphString & sPath )
{
	std::string sResult;
	sResult.reserve ( sPath.Length() );
	for ( const char * p=sPath.cstr(); *p; ++p )
	{
		if ( *p=='#' )
			sResult.push_back ( '\\' );
		sResult.push_back ( *p );
	}
	return sResult.c_str();
}


static bool ListStagingDir ( const CSphString & sDir, CSphVector<IndexerRtBulkFile_t> & dFiles, CSphString & sError )
{
	boost::system::error_code tError;
	fs::recursive_directory_iterator tIt ( sDir.cstr(), tError );
	for ( const fs::recursive_directory_iterator tEnd; !tError && tIt!=tEnd; tIt.increment ( tError ) )
	{
		auto & tFile = dFiles.Add();
		const auto & tPath = tIt->path();
		tFile.m_sPath = tPath.string().c_str();
		if ( fs::is_regular_file ( tPath, tError ) )
		{
			auto iSize = fs::file_size ( tPath, tError );
			if ( !tError )
				tFile.m_iSize = (int64_t)iSize;
		}
	}
	if ( tError )
	{
		sError.SetSprintf ( "failed to list bulk staging directory '%s': %s", sDir.cstr(), tError.message().c_str() );
		return false;
	}
	return true;
}


static bool PrepareStagingRoot ( const CSphString & sRoot, CSphString & sError )
{
	boost::system::error_code tError;
	if ( fs::create_directory ( sRoot.cstr(), tError ) || !tError )
		return true;

	sError.SetSprintf ( "failed to create bulk staging root '%s': %s", sRoot.cstr(), tError.message().c_str() );
	return false;
}


static bool CreateStagingDir ( const CSphString & sRoot, CSphString & sDir, CSphString & sError )
{
	static const int64_t iDaemonId = GetIndexUid();
	static std::atomic<int64_t> iBulkId { 0 };

	for ( ;; )
	{
		sDir.SetSprintf ( "%s/" INT64_FMT "-" INT64_FMT, sRoot.cstr(), iDaemonId, iBulkId.fetch_add ( 1, std::memory_order_relaxed ) + 1 );
		boost::system::error_code tError;
		if ( fs::create_directory ( sDir.cstr(), tError ) )
			return true;
		if ( !tError )
			continue;

		sError.SetSprintf ( "failed to create bulk staging directory '%s': %s", sDir.cstr(), tError.message().c_str() );
		return false;
	}
}

enum class IndexerRtBulkWait_e
{
	OK,
	FAILED,
	CANCELLED
};

constexpr int INDEXER_RT_BULK_WAIT_INTERVAL_MS = 25;


static bool CheckIndexerRtBulkCancelled ( CSphString & sError )
{
	if ( sphInterrupted() )
	{
		sError = "Server shutdown in progress";
		return true;
	}

	if ( session::GetKilled() )
	{
		sError = "query was killed";
		return true;
	}

	return false;
}

#if !_WIN32
struct ResetSignalMask_t : boost::process::extend::handler
{
	template <typename EXECUTOR>
	void on_exec_setup ( EXECUTOR & tExecutor ) const
	{
		sigset_t tSignals;
		if ( sigemptyset ( &tSignals )!=0 )
		{
			tExecutor.set_error ( std::error_code ( errno, std::generic_category() ), "sigemptyset() failed" );
			return;
		}

		if ( sigprocmask ( SIG_SETMASK, &tSignals, nullptr )!=0 )
			tExecutor.set_error ( std::error_code ( errno, std::generic_category() ), "sigprocmask() failed" );
	}
};
#endif


static void AppendIndexerRtBulkError ( CSphString & sError, const CSphString & sExtra )
{
	if ( sExtra.IsEmpty() )
		return;
	if ( sError.IsEmpty() )
	{
		sError = sExtra;
		return;
	}

	CSphString sPrevious = sError;
	sError.SetSprintf ( "%s; %s", sPrevious.cstr(), sExtra.cstr() );
}


bool ListIndexerRtBulkFiles ( const CSphString & sRoot, CSphVector<IndexerRtBulkFile_t> & dFiles, CSphString & sError )
{
	dFiles.Reset();
	boost::system::error_code tError;
	auto tStatus = fs::status ( sRoot.cstr(), tError );
	if ( tError==boost::system::errc::no_such_file_or_directory )
		return true;
	if ( tError )
	{
		sError.SetSprintf ( "failed to inspect bulk staging root '%s': %s", sRoot.cstr(), tError.message().c_str() );
		return false;
	}
	if ( !fs::is_directory ( tStatus ) )
	{
		sError.SetSprintf ( "bulk staging root '%s' is not a directory", sRoot.cstr() );
		return false;
	}

	return ListStagingDir ( sRoot, dFiles, sError );
}


static void DropSharedBulkImport ( const CSphString & sTable );

// For PURGE and DROP: cancel the shared load into sTable, if any, so that its children stop writing, then remove the
// staging root.
bool RemoveIndexerRtBulkRoot ( const CSphString & sTable, const CSphString & sRoot, CSphString & sError )
{
	DropSharedBulkImport ( sTable );
	sError = "";
	boost::system::error_code tError;
	fs::remove_all ( sRoot.cstr(), tError );
	if ( !tError )
		return true;

	sError.SetSprintf ( "failed to remove bulk staging root '%s': %s", sRoot.cstr(), tError.message().c_str() );
	return false;
}


static bool CheckSchemaSupported ( const CSphSchema & tSchema, CSphString & sError );
static bool WriteIndexerRtBulkConfig ( const RtIndex_i & tRt, const CSphString & sDir, CSphVector<int> & dCsvAttrOrder, CSphString & sError );

// One indexer child: its staging directory, generated config, CSV input stream and chunk.
class RtBulkIndexer_c
{
public:
	~RtBulkIndexer_c()
	{
		Abort();
		RemoveDir();
	}

	bool Start ( const RtIndex_i & tRt, CSphString & sError )
	{
		if ( !CheckSchemaSupported ( tRt.GetMatchSchema(), sError ) )
			return false;

		CSphString sRoot = GetIndexerRtBulkRoot ( tRt );
		if ( !PrepareStagingRoot ( sRoot, sError ) )
			return false;

		if ( !CreateStagingDir ( sRoot, m_sDir, sError ) )
			return false;

		if ( !WriteIndexerRtBulkConfig ( tRt, m_sDir, m_dCsvAttrOrder, sError ) )
			return false;

		CSphString sIndexer;
		if ( !GetIndexerPath ( sIndexer, sError ) )
			return false;

		return Launch ( sIndexer, GetFile ( "indexer.conf" ), GetFile ( "indexer.log" ), sError );
	}

	CSphString GetFile ( const char * szFile ) const
	{
		CSphString sPath;
		sPath.SetSprintf ( "%s/%s", m_sDir.cstr(), szFile );
		return sPath;
	}

	const CSphVector<int> & GetCsvAttrOrder() const { return m_dCsvAttrOrder; }
	int GetPid() const { return (int)m_tChild.id(); }

	CSphString GetOutput() const
	{
		CSphString sPath = GetFile ( "indexer.log" );
		FILE * fp = fopen ( sPath.cstr(), "rb" );
		if ( !fp )
			return {};

		char sBuffer[4097];
		size_t iLength = fread ( sBuffer, 1, sizeof(sBuffer)-1, fp );
		fclose ( fp );
		while ( iLength && sphIsSpace ( sBuffer[iLength-1] ) )
			--iLength;
		sBuffer[iLength] = '\0';
		return CSphString ( sBuffer );
	}

private:
	void RemoveDir() noexcept
	{
		if ( m_sDir.IsEmpty() )
			return;

		fs::path tDir ( m_sDir.cstr() );
		boost::system::error_code tError;
		fs::remove_all ( tDir, tError );
		if ( tError )
			sphWarning ( "bulk_import cleanup failed for '%s': %s", m_sDir.cstr(), tError.message().c_str() );
		else
		{
			tError.clear();
			fs::remove ( tDir.parent_path(), tError );
			if ( tError && tError!=boost::system::errc::directory_not_empty )
				sphWarning ( "bulk_import cleanup failed for '%s': %s", tDir.parent_path().string().c_str(), tError.message().c_str() );
		}
		m_sDir = "";
	}

	bool Launch ( const CSphString & sIndexer, const CSphString & sConfig, const CSphString & sOutput, CSphString & sError )
	{
		assert ( !m_tChild.valid() );
		m_sExecutable = sIndexer;

		std::vector<std::string> dArgs { "--config", sConfig.cstr(), "--remove_dups", "indexer_rt_bulk_chunk" };
		// limit_handles keeps per-launch handles in mutable state.
		// Do not share Boost's global instance across concurrent requests.
		std::decay_t<decltype(boost::process::limit_handles)> tLimitHandles;
		std::error_code tError;
		#if !_WIN32
		{
			try
			{
				if ( m_tInput.pipe().is_open() )
					m_tInput.pipe().close();
			}
			catch ( const std::exception & tException )
			{
				sError.SetSprintf ( "failed closing unused bulk_import opstream: %s", tException.what() );
				return false;
			}
			catch ( ... )
			{
				sError = "failed closing unused bulk_import opstream";
				return false;
			}

			int dSockets[2] = { -1, -1 };
			int iSocketType = SOCK_STREAM;
			#ifdef SOCK_CLOEXEC
			iSocketType |= SOCK_CLOEXEC;
			#endif
			if ( socketpair ( AF_UNIX, iSocketType, 0, dSockets )<0 )
			{
				sError.SetSprintf ( "failed to create bulk_import socketpair: %s", strerrorm ( errno ) );
				return false;
			}
			#ifndef SOCK_CLOEXEC
			if ( fcntl ( dSockets[0], F_SETFD, FD_CLOEXEC )<0 || fcntl ( dSockets[1], F_SETFD, FD_CLOEXEC )<0 )
			{
				const int iError = errno;
				close ( dSockets[0] );
				close ( dSockets[1] );
				sError.SetSprintf ( "failed protecting bulk_import socketpair descriptors: %s", strerrorm ( iError ) );
				return false;
			}
			#endif

			boost::process::pipe tSocketInput ( dSockets[0], dSockets[1] );
			// Keep socket ownership here; Boost may return an invalid child before its error callbacks run.
			errno = 0;
			m_tChild = boost::process::child
			(
				sIndexer.cstr(),
				boost::process::args ( dArgs ),
				boost::process::posix::fd.bind ( STDIN_FILENO, tSocketInput.native_source() ),
				( boost::process::std_out & boost::process::std_err ) > sOutput.cstr(),
				tLimitHandles,
				ResetSignalMask_t {},
				boost::process::error ( tError )
			);
			const int iStartError = errno;

			if ( tError || !m_tChild.valid() )
			{
				if ( tError )
					sError.SetSprintf ( "failed to start bulk_import process '%s': %s", sIndexer.cstr(), tError.message().c_str() );
				else if ( iStartError )
					sError.SetSprintf ( "failed to start bulk_import process '%s': %s", sIndexer.cstr(), strerrorm ( iStartError ) );
				else
					sError.SetSprintf ( "failed to start bulk_import process '%s'", sIndexer.cstr() );
				Abort();
				return false;
			}

			const int iInput = tSocketInput.native_sink();
			tSocketInput.assign_sink ( -1 );
			m_pCsvSocketInput = fdopen ( iInput, "wb" );
			if ( !m_pCsvSocketInput )
			{
				const int iError = errno;
				close ( iInput );
				Abort();
				sError.SetSprintf ( "failed opening bulk_import socket stream: %s", strerrorm ( iError ) );
				return false;
			}

			constexpr size_t iBufferSize = 256*1024;
			m_dCsvSocketBuffer.Reset ( iBufferSize );
			if ( setvbuf ( m_pCsvSocketInput, m_dCsvSocketBuffer.Begin(), _IOFBF, iBufferSize ) )
			{
				Abort();
				sError = "failed configuring bulk_import socket buffering";
				return false;
			}
			return true;
		}
		#endif
		#if _WIN32
		// Windows limit_handles temporarily changes process-wide inheritance flags.
		// This path runs from a request coroutine; use a coroutine mutex so
		// contenders yield instead of blocking scheduler worker threads.
		Threads::Coro::ScopedMutex_t tLaunchLock ( g_tIndexerRtBulkLaunchMutex );
		#endif
		m_tChild = boost::process::child
		(
			sIndexer.cstr(),
			boost::process::args ( dArgs ),
			boost::process::std_in < m_tInput,
			( boost::process::std_out & boost::process::std_err ) > sOutput.cstr(),
			tLimitHandles,
			#if !_WIN32
			ResetSignalMask_t {},
			#endif
			boost::process::error ( tError )
		);

		if ( tError || !m_tChild.valid() )
		{
			if ( tError )
				sError.SetSprintf ( "failed to start bulk_import process '%s': %s", sIndexer.cstr(), tError.message().c_str() );
			else
				sError.SetSprintf ( "failed to start bulk_import process '%s'", sIndexer.cstr() );
			Abort();
			return false;
		}

		return true;
	}

public:
	bool WriteCsvBatch ( const CSphString & sCsv )
	{
		#if !_WIN32
		return fwrite ( sCsv.cstr(), 1, sCsv.Length(), m_pCsvSocketInput )==size_t ( sCsv.Length() );
		#else
		m_tInput.write ( sCsv.cstr(), sCsv.Length() );
		return bool ( m_tInput );
		#endif
	}

	// Kill the child at once, without waiting and without touching the input a writer may be blocked in: the writer
	// fails as soon as the child's end of the input is closed. Abort() or the destructor reaps the child.
	void Kill() const noexcept
	{
		if ( !m_tChild.valid() )
			return;
		#if _WIN32
		::TerminateProcess ( m_tChild.native_handle(), 1 );
		#else
		::kill ( m_tChild.id(), SIGKILL );
		#endif
	}

	// Flush and close the CSV input; the child sees EOF.
	bool EndInput ( CSphString & sError )
	{
		CSphString sInputError;
		const bool bInputOk = CloseInput ( sInputError );
		AppendIndexerRtBulkError ( sError, sInputError );
		return bInputOk;
	}

	IndexerRtBulkWait_e WaitChild ( CSphString & sError )
	{
		CSphString sChildError;
		if ( !m_tChild.valid() )
			sChildError = "bulk_import process is unavailable";
		else
		{
			while ( true )
			{
				if ( CheckIndexerRtBulkCancelled ( sChildError ) )
				{
					Abort();
					AppendIndexerRtBulkError ( sError, sChildError );
					return IndexerRtBulkWait_e::CANCELLED;
				}

				std::error_code tRunningError;
				if ( !m_tChild.running ( tRunningError ) )
				{
					if ( tRunningError )
						sChildError.SetSprintf ( "failed checking bulk_import process: %s", tRunningError.message().c_str() );
					break;
				}

				Threads::Coro::SleepMsec ( INDEXER_RT_BULK_WAIT_INTERVAL_MS );
			}

			if ( sChildError.IsEmpty() )
			{
				std::error_code tWaitError;
				m_tChild.wait ( tWaitError );
				if ( tWaitError )
					sChildError.SetSprintf ( "failed waiting for bulk_import process: %s", tWaitError.message().c_str() );
				else if ( m_tChild.exit_code()!=0 )
					sChildError.SetSprintf ( "bulk_import build process '%s' failed with status %d", m_sExecutable.cstr(), m_tChild.exit_code() );
			}
		}

		if ( !sChildError.IsEmpty() )
			Abort();

		AppendIndexerRtBulkError ( sError, sChildError );
		return sChildError.IsEmpty() ? IndexerRtBulkWait_e::OK : IndexerRtBulkWait_e::FAILED;
	}

	void Abort() noexcept
	{
		DiscardInput();

		if ( m_tChild.valid() )
		{
			std::error_code tRunningError;
			if ( m_tChild.running ( tRunningError ) )
				Kill(); // not child::terminate(): on Windows it closes the process handle, and the wait below would not wait

			std::error_code tFinalWaitError;
			m_tChild.wait ( tFinalWaitError );
		}
	}

private:
	bool CloseInput ( CSphString & sError )
	{
		bool bOk = true;
		#if !_WIN32
		if ( m_pCsvSocketInput )
		{
			FILE * pInput = m_pCsvSocketInput;
			m_pCsvSocketInput = nullptr;
			bOk = fclose ( pInput )==0;
			const int iError = errno;
			m_dCsvSocketBuffer.Reset ( 0 );
			if ( !bOk )
				sError.SetSprintf ( "failed closing bulk_import socket stream: %s", strerrorm ( iError ) );
			return bOk;
		}
		#endif
		try
		{
			m_tInput.flush();
			if ( !m_tInput )
			{
				sError = "failed flushing bulk_import stream";
				bOk = false;
			}
		}
		catch ( const std::exception & tException )
		{
			sError.SetSprintf ( "failed flushing bulk_import stream: %s", tException.what() );
			bOk = false;
		}
		catch ( ... )
		{
			sError = "failed flushing bulk_import stream";
			bOk = false;
		}

		try
		{
			// Boost 1.71-1.87 opstream::close() flushes but does not close the pipe.
			// Close the underlying pipe explicitly after the checked flush to deliver EOF.
			if ( m_tInput.pipe().is_open() )
				m_tInput.pipe().close();
		}
		catch ( const std::exception & tException )
		{
			CSphString sCloseError;
			sCloseError.SetSprintf ( "failed closing bulk_import stream: %s", tException.what() );
			AppendIndexerRtBulkError ( sError, sCloseError );
			bOk = false;
		}
		catch ( ... )
		{
			AppendIndexerRtBulkError ( sError, CSphString ( "failed closing bulk_import stream" ) );
			bOk = false;
		}

		return bOk;
	}

	void DiscardInput() noexcept
	{
		#if !_WIN32
		if ( m_pCsvSocketInput )
		{
			FILE * pInput = m_pCsvSocketInput;
			m_pCsvSocketInput = nullptr;
			const int iInput = fileno ( pInput );
			const int iNull = open ( "/dev/null", O_WRONLY );
			if ( iNull>=0 )
			{
				if ( iInput>=0 && dup2 ( iNull, iInput )>=0 )
				{
					close ( iNull );
					fclose ( pInput );
					m_dCsvSocketBuffer.Reset ( 0 );
					return;
				}
				close ( iNull );
			}
			fclose ( pInput );
			m_dCsvSocketBuffer.Reset ( 0 );
			return;
		}
		#endif
		try
		{
			// Do not flush a partially staged statement on abort.
			if ( m_tInput.pipe().is_open() )
				m_tInput.pipe().close();
		}
		catch ( ... )
		{}
	}

	CSphString m_sDir;
	CSphVector<int> m_dCsvAttrOrder;
	boost::process::opstream m_tInput;
	boost::process::child m_tChild;
	CSphString m_sExecutable;
#if !_WIN32
	FILE * m_pCsvSocketInput = nullptr;
	CSphFixedVector<char> m_dCsvSocketBuffer { 0 };
#endif
};


static IndexerRtBulkWait_e WaitIndexerRtBulk ( RtBulkIndexer_c & tIndexer, CSphString & sError )
{
	auto eResult = IndexerRtBulkWait_e::CANCELLED;
	if ( CheckIndexerRtBulkCancelled ( sError ) )
		tIndexer.Abort();
	else
	{
		const bool bInputOk = tIndexer.EndInput ( sError );
		eResult = tIndexer.WaitChild ( sError );
		if ( eResult==IndexerRtBulkWait_e::OK && !bInputOk )
			eResult = IndexerRtBulkWait_e::FAILED;
		if ( eResult==IndexerRtBulkWait_e::OK )
			return eResult;
	}

	CSphString sOutput = tIndexer.GetOutput();
	if ( !sOutput.IsEmpty() )
		AppendIndexerRtBulkError ( sError, sOutput );
	return eResult;
}


IndexerRtBulkState_t::IndexerRtBulkState_t() = default;
IndexerRtBulkState_t::~IndexerRtBulkState_t() = default;


static void AppendCsvEscaped ( StringBuilder_c & sOut, const char * szValue )
{
	const char * pStart = szValue ? szValue : "";
	for ( const char * p = pStart; ; ++p )
	{
		if ( *p!='"' && *p!='\0' )
			continue;
		sOut.AppendRawChunk ( Str_t { pStart, int ( p-pStart ) } );
		if ( !*p )
			break;
		sOut.AppendRawChunk ( Str_t { "\"\"", 2 } );
		pStart = p+1;
	}
}


static void AppendCsvQuoted ( StringBuilder_c & sOut, const char * szValue )
{
	sOut << '"';
	AppendCsvEscaped ( sOut, szValue );
	sOut << '"';
}


// The id as an ordinary insert takes it, never the raw value (the indexer would read a string loosely or drop its
// row): a missing id or one that converts to 0 gets an auto-id, as there. ValidateIndexerRtBulkInputValues has
// refused a failed conversion.
static void AppendIdToCsv ( StringBuilder_c & sOut, const SqlInsert_t * pValue )
{
	uint64_t uId = 0;
	CSphString sError;
	if ( pValue )
		ConvertInsertDocId ( *pValue, 0, 0, uId, sError );

	sOut << '"' << ( uId ? uId : (uint64_t)UidShort() ) << '"';
}


static void AppendInsertValueToCsv ( StringBuilder_c & sOut, const SqlInsert_t & tValue, ESphAttr eType )
{
	sOut << '"';
	switch ( tValue.m_iType )
	{
	case SqlInsert_t::QUOTED_STRING:
		AppendCsvEscaped ( sOut, tValue.m_sVal.cstr() );
		break;

	case SqlInsert_t::CONST_FLOAT:
		sOut.Appendf ( "%.9g", tValue.m_fVal );
		break;

	case SqlInsert_t::CONST_INT:
		sOut << tValue.GetValueInt();
		break;

	case SqlInsert_t::CONST_MVA:
		if ( eType==SPH_ATTR_JSON )
		{
			AppendCsvEscaped ( sOut, tValue.m_sVal.cstr() );
			break;
		}
		if ( tValue.m_pVals )
		{
			bool bFirst = true;
			for ( const auto & tItem : *tValue.m_pVals )
			{
				if ( !bFirst )
					sOut.RawC ( ' ' );
				bFirst = false;
				if ( eType==SPH_ATTR_FLOAT_VECTOR )
					sOut.Appendf ( "%.9g", tItem.m_fValue );
				else
					sOut << tItem.m_iValue;
			}
		}
		break;

	case SqlInsert_t::TOK_NULL:
		break;

	default:
		AppendCsvEscaped ( sOut, tValue.m_sVal.cstr() );
		break;
	}
	sOut << '"';
}


static const char * SVpipeAttrDirective ( ESphAttr eType )
{
	switch ( eType )
	{
	case SPH_ATTR_INTEGER:		return "csvpipe_attr_uint";
	case SPH_ATTR_TIMESTAMP:	return "csvpipe_attr_timestamp";
	case SPH_ATTR_BOOL:			return "csvpipe_attr_bool";
	case SPH_ATTR_FLOAT:		return "csvpipe_attr_float";
	case SPH_ATTR_BIGINT:		return "csvpipe_attr_bigint";
	case SPH_ATTR_STRING:		return "csvpipe_attr_string";
	case SPH_ATTR_JSON:			return "csvpipe_attr_json";
	case SPH_ATTR_UINT32SET:	return "csvpipe_attr_multi";
	case SPH_ATTR_INT64SET:		return "csvpipe_attr_multi_64";
	case SPH_ATTR_FLOAT_VECTOR:	return "csvpipe_attr_float_vector";
	default:					return nullptr;
	}
}


struct IndexerRtBulkColumn_t
{
	int m_iColumn;
	ESphAttr m_eType;
	bool m_bDocid;
};


struct IndexerRtBulkInputMap_t
{
	CSphFixedVector<int> m_dAttrs { 0 };
	CSphFixedVector<int> m_dFields { 0 };
	int m_iId = -1;
};


static constexpr ESphAttr g_dSVpipeAttrOrder[] =
{
	SPH_ATTR_INTEGER, SPH_ATTR_TIMESTAMP, SPH_ATTR_BOOL, SPH_ATTR_FLOAT, SPH_ATTR_BIGINT,
	SPH_ATTR_UINT32SET, SPH_ATTR_INT64SET, SPH_ATTR_FLOAT_VECTOR, SPH_ATTR_STRING, SPH_ATTR_JSON
};


void AbortIndexerRtBulkBatch ( ClientSession_c & tSession )
{
	tSession.m_tIndexerRtBulk.m_pIndexer.reset();
}


void CleanupIndexerRtBulk ( ClientSession_c & tSession )
{
	auto & tState = tSession.m_tIndexerRtBulk;
	AbortIndexerRtBulkBatch ( tSession );
	tState.m_sTable = "";
	tState.m_iIndexId = -1;
	tState.m_iAlterGeneration = -1;
	tState.m_tReservation.Reset();
}


static bool CheckSchemaSupported ( const CSphSchema & tSchema, CSphString & sError )
{
	const CSphColumnInfo * pDocid = tSchema.GetAttr ( sphGetDocidName() );
	if ( !pDocid || pDocid->IsUuidLinkedDocid() )
	{
		sError = "bulk_import supports numeric document ids only";
		return false;
	}

	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( sphIsInternalAttr ( tAttr ) )
			continue;
		ESphAttr eType = tAttr.m_eAttrType;
		if ( eType!=SPH_ATTR_TOKENCOUNT && !SVpipeAttrDirective ( eType ) )
		{
			sError.SetSprintf ( "bulk_import does not support attribute '%s'", tAttr.m_sName.cstr() );
			return false;
		}
	}
	return true;
}


static bool CheckIndexerRtBulkExecutable ( CSphString & sError )
{
	CSphString sIndexer;
	if ( !GetIndexerPath ( sIndexer, sError ) )
		return false;

	CSphString sFileError;
	if ( sphIsReadable ( sIndexer, &sFileError ) )
		return true;

	sError.SetSprintf ( "bulk_import executable '%s' is unavailable: %s", sIndexer.cstr(), sFileError.cstr() );
	return false;
}


struct IndexerRtBulkTarget_t
{
	cServedIndexRefPtr_c	m_pServed;
	int64_t				m_iIndexId = -1;
	int					m_iAlterGeneration = -1;
};


static bool CheckIndexerRtBulkTarget ( const CSphString & sTable, IndexerRtBulkTarget_t & tTarget, CSphString & sError )
{
	tTarget.m_pServed = GetServed ( sTable );
	if ( !tTarget.m_pServed )
	{
		sError.SetSprintf ( "bulk_import requires an existing local RT table; table '%s' is absent or distributed", sTable.cstr() );
		return false;
	}

	if ( tTarget.m_pServed->m_eType!=IndexType_e::RT )
	{
		sError.SetSprintf ( "bulk_import requires an RT table; table '%s' has type %s", sTable.cstr(), szIndexType ( tTarget.m_pServed->m_eType ) );
		return false;
	}

	if ( ServedDesc_t::IsCluster ( tTarget.m_pServed ) )
	{
		sError.SetSprintf ( "bulk_import does not support cluster table '%s'", sTable.cstr() );
		return false;
	}

	RIdx_T<RtIndex_i *> pRt { tTarget.m_pServed };
	if ( !pRt->IsSaveEnabled() )
	{
		sError.SetSprintf ( "table '%s' is frozen", sTable.cstr() );
		return false;
	}

	if ( !CheckSchemaSupported ( pRt->GetMatchSchema(), sError ) )
		return false;

	tTarget.m_iIndexId = pRt->GetIndexId();
	tTarget.m_iAlterGeneration = pRt->GetAlterGeneration();
	return true;
}


static bool CheckReservation ( const IndexerRtBulkState_t & tState, const cServedIndexRefPtr_c & pServed, CSphString & sError )
{
	if ( tState.m_tReservation.Matches ( pServed ) )
		return true;

	sError.SetSprintf ( "table '%s' was replaced while bulk_import was active", tState.m_sTable.cstr() );
	return false;
}


static bool CheckGeneration ( const IndexerRtBulkState_t & tState, const RtIndex_i & tRt, CSphString & sError )
{
	if ( tRt.GetIndexId()==tState.m_iIndexId && tRt.GetAlterGeneration()==tState.m_iAlterGeneration )
		return true;

	sError.SetSprintf ( "table '%s' was altered while bulk_import was active", tState.m_sTable.cstr() );
	return false;
}


static bool RecheckTarget ( ClientSession_c & tSession, CSphString & sError, const char * szAction=nullptr )
{
	auto & tState = tSession.m_tIndexerRtBulk;
	auto pServed = GetServed ( tState.m_sTable );
	if ( !CheckReservation ( tState, pServed, sError ) )
	{
		if ( szAction )
			sError.SetSprintf ( "table '%s' was replaced while %s", tState.m_sTable.cstr(), szAction );
		CleanupIndexerRtBulk ( tSession );
		return false;
	}

	RIdx_T<RtIndex_i *> pRt { pServed };
	if ( CheckGeneration ( tState, *pRt.Ptr(), sError ) )
		return true;

	if ( szAction )
		sError.SetSprintf ( "table '%s' was altered while %s", tState.m_sTable.cstr(), szAction );
	CleanupIndexerRtBulk ( tSession );
	return false;
}


// Take the write reservation of the target and make sure the table did not change meanwhile.
static bool ReserveIndexerRtBulkTarget ( const CSphString & sTable, ServedIndexWriteReservation_c & tReservation, IndexerRtBulkTarget_t & tCurrent, CSphString & sError )
{
	IndexerRtBulkTarget_t tTarget;
	if ( !CheckIndexerRtBulkTarget ( sTable, tTarget, sError ) )
		return false;

	if ( !CheckIndexerRtBulkExecutable ( sError ) )
		return false;

	ServedIndexWriteReservation_c tNew;
	if ( !tNew.TryAcquire ( tTarget.m_pServed ) )
	{
		sError.SetSprintf ( "table '%s' is locked", sTable.cstr() );
		return false;
	}

	if ( !CheckIndexerRtBulkTarget ( sTable, tCurrent, sError ) )
		return false;
	if ( !tNew.Matches ( tCurrent.m_pServed ) || tCurrent.m_iIndexId!=tTarget.m_iIndexId || tCurrent.m_iAlterGeneration!=tTarget.m_iAlterGeneration )
	{
		sError.SetSprintf ( "table '%s' changed while enabling bulk_import", sTable.cstr() );
		return false;
	}

	tReservation = std::move ( tNew );
	return true;
}


bool ActivateIndexerRtBulk ( ClientSession_c & tSession, const CSphString & sTable, CSphString & sError )
{
	auto & tState = tSession.m_tIndexerRtBulk;
	if ( sTable.IsEmpty() )
	{
		sError = "bulk_import requires a target table; use SET bulk_import=<table>";
		return false;
	}

	if ( tState.IsEnabled() )
	{
		if ( tState.m_sTable!=sTable )
		{
			sError.SetSprintf ( "bulk_import is already active for table '%s'", tState.m_sTable.cstr() );
			return false;
		}

		return RecheckTarget ( tSession, sError );
	}

	if ( tSession.m_bInTransaction || tSession.m_tAcc.GetIndex() || tSession.m_tShardTxn.HasPendingData() )
	{
		sError = "bulk_import requires a clean session with no active transaction or pending writes";
		return false;
	}

	IndexerRtBulkTarget_t tCurrent;
	ServedIndexWriteReservation_c tReservation;
	if ( !ReserveIndexerRtBulkTarget ( sTable, tReservation, tCurrent, sError ) )
		return false;

	tState.m_sTable = sTable;
	tState.m_iIndexId = tCurrent.m_iIndexId;
	tState.m_iAlterGeneration = tCurrent.m_iAlterGeneration;
	tState.m_tReservation = std::move ( tReservation );
	return true;
}


// Explicit indexer limits for the generated config; these are the indexer defaults.
static const char * g_szIndexerRtBulkMemLimit = "128M";
static const char * g_szIndexerRtBulkWriteBuffer = "1M";


static bool WriteIndexerRtBulkConfig ( const RtIndex_i & tRt, const CSphString & sDir, CSphVector<int> & dCsvAttrOrder, CSphString & sError )
{
	const CSphSchema & tSchema = tRt.GetMatchSchema();

	// Use the same type-grouped order for csvpipe directives and streamed values.
	for ( ESphAttr eType : g_dSVpipeAttrOrder )
		for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
		{
			const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
			if ( tAttr.m_eAttrType!=eType || tAttr.m_sName==sphGetDocidName() || sphIsInternalAttr ( tAttr ) )
				continue;
			if ( tSchema.GetField ( tAttr.m_sName.cstr() ) && tAttr.m_eAttrType==SPH_ATTR_STRING )
				continue;
			dCsvAttrOrder.Add(i);
		}

	CSphString sConfig = SphSprintf ( "%s/indexer.conf", sDir.cstr() );
	CSphString sIndex = SphSprintf ( "%s/chunk", sDir.cstr() );
	FILE * fpConfig = fopen ( sConfig.cstr(), "wb" );
	if ( !fpConfig )
	{
		sError.SetSprintf ( "failed to create bulk staging config in '%s': %s", sDir.cstr(), strerrorm ( errno ) );
		return false;
	}

	CSphString sConfigLemmatizerBase = EscapeIndexerRtBulkConfigPath ( g_sLemmatizerBase );
	fprintf ( fpConfig, "common {\n  lemmatizer_base = %s\n}\n\n", sConfigLemmatizerBase.cstr() );
	fprintf ( fpConfig, "indexer {\n  mem_limit = %s\n  write_buffer = %s\n}\n\n", g_szIndexerRtBulkMemLimit, g_szIndexerRtBulkWriteBuffer );
	const char * szPipeField = "csvpipe_field";
	const char * szPipeFieldString = "csvpipe_field_string";
	const char * szPipeAttrOrder = "csvpipe_attr_order";
	fprintf ( fpConfig, "source indexer_rt_bulk_source {\n  type = csvpipe\n" );
	fprintf ( fpConfig, "  csvpipe_command = %s\n", GetIndexerRtBulkCsvpipeCommand() );
	for ( int i=0; i<tSchema.GetFieldsCount(); ++i )
	{
		const CSphColumnInfo & tField = tSchema.GetField(i);
		const CSphColumnInfo * pSameAttr = tSchema.GetAttr ( tField.m_sName.cstr() );
		fprintf ( fpConfig, "  %s = %s\n", pSameAttr && pSameAttr->m_eAttrType==SPH_ATTR_STRING ? szPipeFieldString : szPipeField, tField.m_sName.cstr() );
	}
	for ( int iAttr : dCsvAttrOrder )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr ( iAttr );
		const char * szDirective = SVpipeAttrDirective ( tAttr.m_eAttrType );
		fprintf ( fpConfig, "  %s = %s", szDirective, tAttr.m_sName.cstr() );
		if ( tAttr.m_eAttrType==SPH_ATTR_INTEGER && tAttr.m_tLocator.IsBitfield() )
			fprintf ( fpConfig, ":%d", tAttr.m_tLocator.m_iBitCount );
		fprintf ( fpConfig, "\n" );
	}
	fprintf ( fpConfig, "  %s = ", szPipeAttrOrder );
	bool bFirstAttr = true;
	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_sName==sphGetDocidName() || sphIsInternalAttr ( tAttr ) || !SVpipeAttrDirective ( tAttr.m_eAttrType ) )
			continue;
		fprintf ( fpConfig, "%s%s", bFirstAttr ? "" : ",", tAttr.m_sName.cstr() );
		bFirstAttr = false;
	}
	fprintf ( fpConfig, "\n" );
	CSphString sConfigIndex = EscapeIndexerRtBulkConfigPath ( sIndex );
	fprintf ( fpConfig, "}\n\nindex indexer_rt_bulk_chunk {\n  type = plain\n  source = indexer_rt_bulk_source\n  path = %s\n", sConfigIndex.cstr() );
	DumpSettingsCfg ( fpConfig, tRt, nullptr );
	if ( tRt.GetSettings().m_dKNN.GetLength() )
		fprintf ( fpConfig, "\n	knn = %s", FormatKNNConfigStr ( tRt.GetSettings().m_dKNN ).cstr() );
	fprintf ( fpConfig, "\n}\n" );
	fclose ( fpConfig );
	return true;
}


static bool InitIndexerRtBulk ( ClientSession_c & tSession, const RtIndex_i & tRt, CSphString & sError )
{
	auto & tState = tSession.m_tIndexerRtBulk;
	if ( tState.HasPendingData() )
		return true;

	auto pIndexer = std::make_unique<RtBulkIndexer_c>();
	if ( !pIndexer->Start ( tRt, sError ) )
		return false;

	tState.m_pIndexer = std::move ( pIndexer );
	return true;
}


static bool ValidateIndexerRtBulkInputValues ( const CSphSchema & tSchema, const SqlStmt_t & tStmt, const IndexerRtBulkInputMap_t & tMap, CSphString & sError );

static bool PrepareIndexerRtBulkInputMap ( const CSphSchema & tSchema, const SqlStmt_t & tStmt, IndexerRtBulkInputMap_t & tMap, EMYSQL_ERR & eError, CSphString & sError )
{
	const int iColumns = tStmt.m_iSchemaSz;
	const int iValues = tStmt.m_dInsertValues.GetLength();
	if ( iColumns<=0 || iValues%iColumns )
	{
		sError = "wrong number of values here";
		return false;
	}

	const int iDocidAttr = tSchema.GetAttrIndex ( sphGetDocidName() );
	const CSphColumnInfo * pDocid = iDocidAttr>=0 ? &tSchema.GetAttr ( iDocidAttr ) : nullptr;
	const bool bUuidDocid = pDocid && pDocid->IsUuidLinkedDocid();
	const int iAttrs = tSchema.GetAttrsCount();
	CSphBitvec dSeenColumns ( iAttrs + tSchema.GetFieldsCount() );
	tMap.m_dAttrs.Reset ( iAttrs );
	tMap.m_dFields.Reset ( tSchema.GetFieldsCount() );
	tMap.m_dAttrs.Fill ( -1 );
	tMap.m_dFields.Fill ( -1 );
	const CSphString * pDuplicate = nullptr;
	for ( int i=0; i<tStmt.m_dInsertSchema.GetLength(); ++i )
	{
		const CSphString & sColumn = tStmt.m_dInsertSchema[i];
		if ( bUuidDocid && sColumn=="@id" )
		{
			sError = "attribute '@id' is internal";
			return false;
		}

		const int iAttr = tSchema.GetAttrIndex ( sColumn.cstr() );
		const int iField = tSchema.GetFieldIndex ( sColumn.cstr() );
		if ( sColumn==sphGetUuidDocidName() || ( iAttr<0 && iField<0 ) )
		{
			sError.SetSprintf ( "unknown column: '%s'", sColumn.cstr() );
			return false;
		}

		bool bDuplicate = false;
		if ( iAttr>=0 )
		{
			bDuplicate = dSeenColumns.BitGet ( iAttr );
			dSeenColumns.BitSet ( iAttr );
			tMap.m_dAttrs[iAttr] = i;
		}
		if ( iField>=0 )
		{
			const int iFieldBit = iAttrs + iField;
			bDuplicate |= dSeenColumns.BitGet ( iFieldBit );
			dSeenColumns.BitSet ( iFieldBit );
			if ( sColumn==tSchema.GetField ( iField ).m_sName )
				tMap.m_dFields[iField] = i;
		}
		if ( bDuplicate && ( !pDuplicate || sColumn<*pDuplicate ) )
			pDuplicate = &sColumn;
	}

	if ( pDuplicate )
	{
		eError = EMYSQL_ERR::FIELD_SPECIFIED_TWICE;
		sError.SetSprintf ( "column '%s' specified twice", pDuplicate->cstr() );
		return false;
	}

	tMap.m_iId = iDocidAttr>=0 ? tMap.m_dAttrs[iDocidAttr] : -1; // without an id column every row gets an auto-id
	return ValidateIndexerRtBulkInputValues ( tSchema, tStmt, tMap, sError );
}


// Per-row checks of a statement whose columns already match tMap.
static bool ValidateIndexerRtBulkInputValues ( const CSphSchema & tSchema, const SqlStmt_t & tStmt, const IndexerRtBulkInputMap_t & tMap, CSphString & sError )
{
	const int iColumns = tStmt.m_iSchemaSz;
	const int iIdColumn = tMap.m_iId;
	for ( int iRow=0; iIdColumn>=0 && iRow<tStmt.m_iRowsAffected; ++iRow )
	{
		// the id is converted as by an ordinary insert, with its errors; its 0 gets an auto-id, as there
		uint64_t uId = 0;
		if ( !ConvertInsertDocId ( tStmt.m_dInsertValues[iRow*iColumns+iIdColumn], iRow, iIdColumn, uId, sError ) )
			return false;
	}

	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_eAttrType!=SPH_ATTR_FLOAT_VECTOR )
			continue;

		const int iColumn = tMap.m_dAttrs[i];
		if ( iColumn<0 )
			continue;

		for ( int iRow=0; iRow<tStmt.m_iRowsAffected; ++iRow )
			if ( !ValidateFloatVectorValue ( tAttr, tStmt.m_dInsertValues[iRow*iColumns+iColumn], iRow, sError ) )
				return false;
	}

	return true;
}


static void BuildIndexerRtBulkColumns ( const CSphSchema & tSchema, const IndexerRtBulkInputMap_t & tMap, const CSphVector<int> & dCsvAttrOrder, CSphVector<IndexerRtBulkColumn_t> & dColumns )
{
	dColumns.Add ( { tMap.m_iId, SPH_ATTR_BIGINT, true } );
	for ( int iColumn : tMap.m_dFields )
		dColumns.Add ( { iColumn, SPH_ATTR_STRING, false } );
	for ( int iAttr : dCsvAttrOrder )
		dColumns.Add ( { tMap.m_dAttrs[iAttr], tSchema.GetAttr ( iAttr ).m_eAttrType, false } );
}


static void AppendIndexerRtBulkCsv ( StringBuilder_c & sBatch, const CSphVector<IndexerRtBulkColumn_t> & dColumns, const SqlStmt_t & tStmt )
{
	const int iColumns = tStmt.m_iSchemaSz;
	for ( int iRow=0; iRow<tStmt.m_iRowsAffected; ++iRow )
	{
		bool bFirst = true;
		for ( const auto & tColumn : dColumns )
		{
			if ( !bFirst )
				sBatch << ',';
			bFirst = false;
			if ( tColumn.m_bDocid )
				AppendIdToCsv ( sBatch, tColumn.m_iColumn<0 ? nullptr : &tStmt.m_dInsertValues[iRow*iColumns+tColumn.m_iColumn] );
			else if ( tColumn.m_iColumn<0 )
				AppendCsvQuoted ( sBatch, "" );
			else
				AppendInsertValueToCsv ( sBatch, tStmt.m_dInsertValues[iRow*iColumns+tColumn.m_iColumn], tColumn.m_eType );
		}
		sBatch << '\n';
	}
}


bool StageIndexerRtBulk ( ClientSession_c & tSession, const SqlStmt_t & tStmt, EMYSQL_ERR & eError, CSphString & sError )
{
	eError = EMYSQL_ERR::PARSE_ERROR;
	auto & tState = tSession.m_tIndexerRtBulk;
	assert ( tState.IsEnabled() );
	if ( tStmt.m_eStmt!=STMT_INSERT )
	{
		sError = "bulk_import supports INSERT only";
		return false;
	}
	if ( tStmt.m_sIndex!=tState.m_sTable )
	{
		sError.SetSprintf ( "bulk_import is active for table '%s', not '%s'", tState.m_sTable.cstr(), tStmt.m_sIndex.cstr() );
		return false;
	}

	auto pServed = GetServed ( tState.m_sTable );
	if ( !CheckReservation ( tState, pServed, sError ) )
	{
		CleanupIndexerRtBulk ( tSession );
		return false;
	}
	RIdx_T<RtIndex_i *> pRt { pServed };
	if ( !CheckGeneration ( tState, *pRt.Ptr(), sError ) )
	{
		CleanupIndexerRtBulk ( tSession );
		return false;
	}
	const CSphSchema & tSchema = pRt->GetMatchSchema();
	IndexerRtBulkInputMap_t tMap;
	if ( !PrepareIndexerRtBulkInputMap ( tSchema, tStmt, tMap, eError, sError ) )
		return false;
	CSphVector<IndexerRtBulkColumn_t> dColumns;
	dColumns.Reserve ( 1 + tSchema.GetFieldsCount() + tSchema.GetAttrsCount() );
	if ( !InitIndexerRtBulk ( tSession, *pRt.Ptr(), sError ) )
		return false;
	BuildIndexerRtBulkColumns ( tSchema, tMap, tState.m_pIndexer->GetCsvAttrOrder(), dColumns );

	assert ( tState.m_pIndexer );
	StringBuilder_c sBatch;
	AppendIndexerRtBulkCsv ( sBatch, dColumns, tStmt );

	CSphString sCsv;
	sBatch.MoveTo ( sCsv );
	errno = 0;
	if ( !tState.m_pIndexer->WriteCsvBatch ( sCsv ) )
	{
		if ( errno )
			sError.SetSprintf ( "failed streaming bulk CSV to indexer: %s", strerrorm ( errno ) );
		else
			sError = "failed streaming bulk CSV to indexer";
		AbortIndexerRtBulkBatch ( tSession );
		return false;
	}
	return true;
}


bool FinalizeIndexerRtBulk ( ClientSession_c & tSession, CSphString & sError )
{
	auto & tState = tSession.m_tIndexerRtBulk;
	if ( !tState.HasPendingData() )
		return true;

	auto eWaitResult = WaitIndexerRtBulk ( *tState.m_pIndexer, sError );
	if ( eWaitResult!=IndexerRtBulkWait_e::OK )
	{
		if ( eWaitResult==IndexerRtBulkWait_e::CANCELLED )
			CleanupIndexerRtBulk ( tSession );
		else
		{
			CSphString sTargetError;
			if ( RecheckTarget ( tSession, sTargetError, "finalizing bulk_import" ) )
				AbortIndexerRtBulkBatch ( tSession );
			else
				sError = sTargetError;
		}
		return false;
	}
	if ( CheckIndexerRtBulkCancelled ( sError ) )
	{
		CleanupIndexerRtBulk ( tSession );
		return false;
	}

	bool bTargetStale = false;
	auto eResult = AttachIndexerRtBulkChunk ( tState.m_sTable, tState.m_tReservation, tState.m_iIndexId, tState.m_iAlterGeneration, tState.m_pIndexer->GetFile ( "chunk" ), bTargetStale, sError );
	if ( bTargetStale )
		CleanupIndexerRtBulk ( tSession );
	else
		AbortIndexerRtBulkBatch ( tSession );
	return eResult==DiskAttachRes_e::OK;
}


//////////////////////////////////////////////////////////////////////////
// Shared bulk_import

constexpr int SHARED_BULK_BATCH_BYTES = 32*1024;
constexpr int SHARED_BULK_WAIT_MS = 1000; // only to notice KILL; failures and slot returns notify
static const long g_iSharedBulkChunks = env_long ( "INDEXER_RT_BULK_SHARED_POOL_SIZE" ).value_or ( 1 ); // N of a bare shared

static const bool LOG_LEVEL_BULK_IMPORT = env_exists ( "MANTICORE_LOG_BULK_IMPORT" ); // shared bulk_import pool and slot events, ruled by this env variable
#define BulkImportInfo( ... ) do if ( LOG_LEVEL_BULK_IMPORT ) sphInfo_impl (__VA_ARGS__); while(0)

// Times of the BULK_IMPORT_SHARED lines; nothing is measured while they are off.
static int64_t BulkImportTimer()
{
	return LOG_LEVEL_BULK_IMPORT ? sphMicroTimer() : 0;
}

struct SharedBulkSlot_t
{
	std::unique_ptr<RtBulkIndexer_c> m_pIndexer;
	int64_t m_iRows = 0;
	// batches, bytes and times are counted only for the BULK_IMPORT_SHARED lines
	int64_t m_iBatches = 0;
	int64_t m_iBytes = 0;
	int64_t m_iWaitUs = 0;
	int64_t m_iWriteUs = 0;
	int64_t m_iMaxWaitUs = 0;
	int64_t m_iMaxWriteUs = 0;
	bool m_bOk = false;
};


// One load into one table: N indexer children (slots) fed by every request admitted to the pool.
class SharedBulkPool_c : public ISphRefcountedMT
{
public:
	const CSphString m_sTable;
	const int m_iChunks;

	// written by Start() before the slots are published under the lock; read-only afterwards
	ServedIndexWriteReservation_c m_tReservation; // released by FailLocked() of a running load or after publishing
	int64_t m_iIndexId = -1;
	int m_iAlterGeneration = -1;
	CSphSchema m_tSchema;
	CSphVector<int> m_dCsvAttrOrder;

	SharedBulkPool_c ( const CSphString & sTable, int iChunks )
		: m_sTable ( sTable )
		, m_iChunks ( iChunks )
	{}

	SharedBulkResult_e Enter ( CSphString & sError );
	void Leave();
	SharedBulkResult_e Start ( CSphString & sError );
	SharedBulkResult_e WaitStarted ( CSphString & sError );
	SharedBulkResult_e Write ( const CSphString & sCsv, int64_t iRows, CSphString & sError );
	CSphString Fail ( const CSphString & sError );
	bool FailOpen ( const CSphString & sError );
	SharedBulkResult_e Publish ( bool bAdmitted, CSphString & sError );

private:
	CSphFixedVector<SharedBulkSlot_t> m_dSlots { 0 }; // destroyed first: children die, then the reservation goes
	Threads::Coro::Mutex_c m_tLock;
	Threads::Coro::ConditionVariable_c m_tChanged;
	CSphVector<int> m_dFree;	// slots without a writer; empty until the children run
	int m_iActive = 1;			// admitted requests; the creating request enters before the pool is visible
	bool m_bStarted = false;	// m_dSlots are complete
	bool m_bClosing = false;	// a commit owns the slots
	CSphString m_sError;		// first error; a failed pool stays until commit or PURGE

	void FailLocked ( const CSphString & sError );
	bool PublishSlots ( CSphString & sError );
};

using SharedBulkPoolRefPtr_c = RefCountedRefPtr_T<SharedBulkPool_c>;
static ReadOnlyHash_T<SharedBulkPool_c> g_hSharedBulk;


static SharedBulkPoolRefPtr_c GetSharedBulkPool ( const CSphString & sTable )
{
	return ConstCastPtr ( g_hSharedBulk.Get ( sTable ) ); // pools are created mutable; the hash hands out const pointers
}


// Add pNew as the pool of sTable where there is none (pOld is nullptr), or remove the pool pOld (pNew is nullptr);
// false when the current pool of the table is not pOld.
static bool ReplaceSharedBulkPool ( const CSphString & sTable, const SharedBulkPool_c * pOld, const SharedBulkPoolRefPtr_c & pNew )
{
	WriteableHash_T<SharedBulkPool_c> tChanger { g_hSharedBulk };
	do
	{
		tChanger.CopyOwnerHash();
		auto * pEntry = ( *tChanger.m_pNewHash ) ( sTable );
		if ( ( pEntry ? pEntry->Ptr() : nullptr )!=pOld )
		{
			tChanger.ResetChanges();
			return false;
		}
		if ( pNew )
			tChanger.Add ( pNew, sTable );
		else
			tChanger.m_pNewHash->Delete ( sTable );
	} while ( !tChanger.TryCommit() );
	return true;
}


SharedBulkResult_e SharedBulkPool_c::Enter ( CSphString & sError )
{
	Threads::Coro::ScopedMutex_t tLock { m_tLock };
	if ( !m_sError.IsEmpty() )
	{
		sError = m_sError;
		return SharedBulkResult_e::FAILED_ALREADY;
	}
	if ( m_bClosing )
	{
		sError.SetSprintf ( "shared bulk_import into table '%s' is being published", m_sTable.cstr() );
		return SharedBulkResult_e::CLOSING;
	}
	++m_iActive;
	return SharedBulkResult_e::STAGED;
}


void SharedBulkPool_c::Leave()
{
	Threads::Coro::ScopedMutex_t tLock { m_tLock };
	--m_iActive;
}


// Reserve the table and start the children; the creating request runs it once, outside the lock.
SharedBulkResult_e SharedBulkPool_c::Start ( CSphString & sError )
{
	IndexerRtBulkTarget_t tTarget;
	if ( !ReserveIndexerRtBulkTarget ( m_sTable, m_tReservation, tTarget, sError ) )
		return SharedBulkResult_e::PRECONDITION;

	m_iIndexId = tTarget.m_iIndexId;
	m_iAlterGeneration = tTarget.m_iAlterGeneration;
	RIdx_T<RtIndex_i *> pRt { tTarget.m_pServed };
	m_tSchema = pRt->GetMatchSchema();
	BulkImportInfo ( "BULK_IMPORT_SHARED created table=%s pool_size=%d", m_sTable.cstr(), m_iChunks );

	CSphFixedVector<SharedBulkSlot_t> dSlots ( m_iChunks );
	ARRAY_FOREACH ( i, dSlots )
	{
		auto & pIndexer = dSlots[i].m_pIndexer;
		pIndexer = std::make_unique<RtBulkIndexer_c>();
		if ( !pIndexer->Start ( *pRt.Ptr(), sError ) )
			return SharedBulkResult_e::FAILED;
		BulkImportInfo ( "BULK_IMPORT_SHARED slot_started table=%s slot=%d pid=%d", m_sTable.cstr(), i, pIndexer->GetPid() );
	}
	for ( int iAttr : dSlots[0].m_pIndexer->GetCsvAttrOrder() )
		m_dCsvAttrOrder.Add ( iAttr );

	Threads::Coro::ScopedMutex_t tLock { m_tLock };
	if ( !m_sError.IsEmpty() ) // PURGE, DROP, shutdown or a joiner's invalid document came first
	{
		sError = m_sError;
		return SharedBulkResult_e::FAILED_ALREADY;
	}
	m_dSlots = std::move ( dSlots );
	for ( int i=0; i<m_iChunks; ++i )
		m_dFree.Add ( i );
	m_bStarted = true;
	m_tChanged.NotifyAll();
	return SharedBulkResult_e::STAGED;
}


// The schema, CSV order and slots are usable once the creating request published them.
SharedBulkResult_e SharedBulkPool_c::WaitStarted ( CSphString & sError )
{
	Threads::Coro::ScopedMutex_t tLock { m_tLock };
	while ( m_sError.IsEmpty() && !m_bStarted )
	{
		if ( m_tChanged.WaitForMs ( tLock, SHARED_BULK_WAIT_MS ) )
			continue; // notified

		if ( CheckIndexerRtBulkCancelled ( sError ) )
			return SharedBulkResult_e::FAILED;
	}

	if ( !m_sError.IsEmpty() )
	{
		sError = m_sError;
		return SharedBulkResult_e::FAILED_ALREADY;
	}
	return SharedBulkResult_e::STAGED;
}


// Lease a slot for one batch; the leaseholder alone touches the slot until it is returned.
SharedBulkResult_e SharedBulkPool_c::Write ( const CSphString & sCsv, int64_t iRows, CSphString & sError )
{
	if ( CheckIndexerRtBulkCancelled ( sError ) )
		return SharedBulkResult_e::FAILED;

	const int64_t tmLease = BulkImportTimer();
	int iSlot = -1;
	{
		Threads::Coro::ScopedMutex_t tLock { m_tLock };
		while ( m_sError.IsEmpty() && m_dFree.IsEmpty() )
		{
			if ( m_tChanged.WaitForMs ( tLock, SHARED_BULK_WAIT_MS ) )
				continue; // notified

			if ( CheckIndexerRtBulkCancelled ( sError ) )
				return SharedBulkResult_e::FAILED;
		}

		if ( !m_sError.IsEmpty() )
		{
			sError = m_sError;
			return SharedBulkResult_e::FAILED_ALREADY;
		}
		iSlot = m_dFree[0]; // round robin keeps the chunks even when one writer at a time is active
		m_dFree.Remove ( 0 );
	}

	auto & tSlot = m_dSlots[iSlot];
	const int64_t tmWrite = BulkImportTimer();
	errno = 0;
	const bool bWritten = tSlot.m_pIndexer->WriteCsvBatch ( sCsv );
	const int iWriteError = errno;
	if ( LOG_LEVEL_BULK_IMPORT )
	{
		const int64_t tmDone = sphMicroTimer();
		tSlot.m_iWaitUs += tmWrite-tmLease;
		tSlot.m_iMaxWaitUs = Max ( tSlot.m_iMaxWaitUs, tmWrite-tmLease );
		tSlot.m_iWriteUs += tmDone-tmWrite;
		tSlot.m_iMaxWriteUs = Max ( tSlot.m_iMaxWriteUs, tmDone-tmWrite );
		if ( bWritten )
		{
			++tSlot.m_iBatches;
			tSlot.m_iBytes += sCsv.Length();
		}
	}
	if ( bWritten )
		tSlot.m_iRows += iRows;

	{
		Threads::Coro::ScopedMutex_t tLock { m_tLock };
		m_dFree.Add ( iSlot );
		m_tChanged.NotifyOne();
	}
	if ( bWritten )
		return SharedBulkResult_e::STAGED;

	if ( iWriteError )
		sError.SetSprintf ( "failed streaming bulk CSV to indexer: %s", strerrorm ( iWriteError ) );
	else
		sError = "failed streaming bulk CSV to indexer";
	AppendIndexerRtBulkError ( sError, tSlot.m_pIndexer->GetOutput() ); // a child that died explains why; its log goes with the pool
	sError = Fail ( sError );
	return SharedBulkResult_e::FAILED;
}


// Keep the first error and wake everybody. A running load never attaches after that, so its children are killed (a
// writer stalled in a full input fails at once) and the table is released; staging goes with the last reference.
void SharedBulkPool_c::FailLocked ( const CSphString & sError )
{
	if ( m_sError.IsEmpty() )
	{
		m_sError = sError.IsEmpty() ? CSphString ( "shared bulk_import failed" ) : sError;
		if ( LOG_LEVEL_BULK_IMPORT )
			sphWarning ( "shared bulk_import into table '%s' failed: %s", m_sTable.cstr(), m_sError.cstr() );
		if ( m_bStarted && !m_bClosing ) // while publishing, the commit owns the children and the reservation
		{
			for ( auto & tSlot : m_dSlots )
				tSlot.m_pIndexer->Kill();
			m_tReservation.Reset();
		}
	}
	m_tChanged.NotifyAll();
}


CSphString SharedBulkPool_c::Fail ( const CSphString & sError )
{
	Threads::Coro::ScopedMutex_t tLock { m_tLock };
	FailLocked ( sError );
	return m_sError;
}


// Fail the pool unless a commit is already publishing it; for PURGE and for requests outside the pool.
bool SharedBulkPool_c::FailOpen ( const CSphString & sError )
{
	Threads::Coro::ScopedMutex_t tLock { m_tLock };
	if ( m_bClosing )
		return false;
	FailLocked ( sError );
	return true;
}


// EOF to every slot with data, wait for those children, then attach their chunks in turn. A slot without rows never
// got a byte (a failed write fails the pool), so its child is killed instead of building from an empty input.
bool SharedBulkPool_c::PublishSlots ( CSphString & sError )
{
	bool bOk = true;
	ARRAY_FOREACH ( i, m_dSlots )
	{
		auto & tSlot = m_dSlots[i];
		const int64_t tmClose = BulkImportTimer();
		bool bClosed = true;
		if ( tSlot.m_iRows )
			bClosed = tSlot.m_pIndexer->EndInput ( sError );
		else
		{
			tSlot.m_pIndexer->Abort();
			tSlot.m_bOk = true;
		}
		BulkImportInfo ( "BULK_IMPORT_SHARED slot_eof table=%s slot=%d pid=%d rows=" INT64_FMT " bytes=" INT64_FMT " close_ms=%.3f ok=%d", m_sTable.cstr(), i, tSlot.m_pIndexer->GetPid(), tSlot.m_iRows, tSlot.m_iBytes, ( sphMicroTimer()-tmClose )/1000.0, (int)bClosed );
		bOk &= bClosed;
	}

	ARRAY_FOREACH ( i, m_dSlots )
	{
		auto & tSlot = m_dSlots[i];
		if ( !tSlot.m_iRows )
			continue;
		BulkImportInfo ( "BULK_IMPORT_SHARED slot_wait_begin table=%s slot=%d pid=%d", m_sTable.cstr(), i, tSlot.m_pIndexer->GetPid() );
		const int64_t tmWait = BulkImportTimer();
		CSphString sChildError;
		tSlot.m_bOk = tSlot.m_pIndexer->WaitChild ( sChildError )==IndexerRtBulkWait_e::OK;
		BulkImportInfo ( "BULK_IMPORT_SHARED slot_complete table=%s slot=%d pid=%d wait_ms=%.3f ok=%d", m_sTable.cstr(), i, tSlot.m_pIndexer->GetPid(), ( sphMicroTimer()-tmWait )/1000.0, (int)tSlot.m_bOk );
		if ( !tSlot.m_bOk )
		{
			AppendIndexerRtBulkError ( sChildError, tSlot.m_pIndexer->GetOutput() );
			AppendIndexerRtBulkError ( sError, sChildError );
			bOk = false;
		}
	}

	{
		Threads::Coro::ScopedMutex_t tLock { m_tLock };
		if ( !m_sError.IsEmpty() ) // PURGE, DROP or shutdown while publishing
		{
			AppendIndexerRtBulkError ( sError, m_sError );
			bOk = false;
		}
	}
	if ( !bOk )
		return false;

	ARRAY_FOREACH ( i, m_dSlots )
	{
		auto & tSlot = m_dSlots[i];
		if ( !tSlot.m_iRows )
			continue;
		const int64_t tmAttach = BulkImportTimer();
		bool bTargetStale = false; // not read: Publish() removes every slot after a failed attach
		auto eAttach = AttachIndexerRtBulkChunk ( m_sTable, m_tReservation, m_iIndexId, m_iAlterGeneration, tSlot.m_pIndexer->GetFile ( "chunk" ), bTargetStale, sError );
		tSlot.m_bOk = eAttach==DiskAttachRes_e::OK;
		BulkImportInfo ( "BULK_IMPORT_SHARED slot_attached table=%s slot=%d attach_ms=%.3f ok=%d", m_sTable.cstr(), i, ( sphMicroTimer()-tmAttach )/1000.0, (int)tSlot.m_bOk );
		if ( !tSlot.m_bOk )
			return false;
	}

	return true;
}


SharedBulkResult_e SharedBulkPool_c::Publish ( bool bAdmitted, CSphString & sError )
{
	bool bPublish;
	{
		Threads::Coro::ScopedMutex_t tLock { m_tLock };
		if ( m_bClosing )
		{
			sError.SetSprintf ( "shared bulk_import into table '%s' is being published", m_sTable.cstr() );
			return SharedBulkResult_e::CLOSING;
		}
		bPublish = m_sError.IsEmpty();
		if ( bPublish && m_iActive>( bAdmitted ? 1 : 0 ) )
		{
			sError.SetSprintf ( "shared bulk_import into table '%s' has other active requests", m_sTable.cstr() );
			return SharedBulkResult_e::BUSY;
		}
		if ( bPublish )
			m_bClosing = true;
		else
			sError = m_sError;
	}

	// a failed load only leaves the hash; its last request destroys the children and staging
	if ( !bPublish )
	{
		ReplaceSharedBulkPool ( m_sTable, this, nullptr );
		return SharedBulkResult_e::FAILED;
	}

	const int64_t tmStart = BulkImportTimer();
	const bool bOk = PublishSlots ( sError );
	if ( LOG_LEVEL_BULK_IMPORT )
	{
		int64_t iRows = 0;
		ARRAY_FOREACH ( i, m_dSlots )
		{
			const auto & tSlot = m_dSlots[i];
			iRows += tSlot.m_iRows;
			BulkImportInfo ( "BULK_IMPORT_SHARED slot_finished table=%s slot=%d rows=" INT64_FMT " batches=" INT64_FMT " bytes=" INT64_FMT " wait_ms=%.3f write_ms=%.3f max_wait_ms=%.3f max_write_ms=%.3f ok=%d",
				m_sTable.cstr(), i, tSlot.m_iRows, tSlot.m_iBatches, tSlot.m_iBytes, tSlot.m_iWaitUs/1000.0, tSlot.m_iWriteUs/1000.0, tSlot.m_iMaxWaitUs/1000.0, tSlot.m_iMaxWriteUs/1000.0, (int)tSlot.m_bOk );
		}
		BulkImportInfo ( "BULK_IMPORT_SHARED finished table=%s rows=" INT64_FMT " ok=%d finalize_ms=%.3f pool_size=%d", m_sTable.cstr(), iRows, (int)bOk, ( sphMicroTimer()-tmStart )/1000.0, m_iChunks );
	}

	// children, staging directories and the table reservation go before the pool leaves the hash and before the
	// reply; newcomers meanwhile get CLOSING rather than a failing reservation
	CSphFixedVector<SharedBulkSlot_t> dSlots { 0 };
	{
		Threads::Coro::ScopedMutex_t tLock { m_tLock };
		m_bStarted = false;
		m_dFree.Reset();
		dSlots = std::move ( m_dSlots );
	}
	dSlots.Reset ( 0 );
	m_tReservation.Reset();
	ReplaceSharedBulkPool ( m_sTable, this, nullptr );
	return bOk ? SharedBulkResult_e::PUBLISHED : SharedBulkResult_e::FAILED;
}


// Pool size of a new load: shared=N, else env INDEXER_RT_BULK_SHARED_POOL_SIZE, else 1; a request that joins
// an open pool ignores its N. Writers may block in a full socket, so at most searchd.threads-2 of them can hold
// daemon threads: N is read as strtol() reads it, and a value out of 1..threads-2 is clamped into that range.
static int ParseSharedBulkChunks ( const CSphString & sTable, const CSphString & sShared )
{
	const long iLimit = Max ( 1, Threads::NThreads()-2 );
	const long iValue = sShared.IsEmpty() ? g_iSharedBulkChunks : strtol ( sShared.cstr(), nullptr, 10 );
	const int iChunks = (int)Min ( Max ( iValue, 1L ), iLimit );
	if ( iChunks!=iValue )
		BulkImportInfo ( "BULK_IMPORT_SHARED pool_size_clamped table=%s requested=%ld pool_size=%d", sTable.cstr(), iValue, iChunks );
	return iChunks;
}


struct SharedBulkRequest_c::Impl_t
{
	CSphString m_sTable;
	CSphString m_sShared;			// N of shared=N; only a request that creates the pool reads it
	bool m_bJoin = false;			// shared_join: never create a pool, so a load whose pool is gone fails
	SharedBulkPoolRefPtr_c m_pPool;	// the pool this request is admitted to
	StringBuilder_c m_sCsv;			// unwritten tail, less than a batch
	int64_t m_iRows = 0;

	// insert layout of the current pool
	bool m_bLayout = false;
	StrVec_t m_dInsertSchema;
	int m_iSchemaSz = 0;
	IndexerRtBulkInputMap_t m_tMap;
	CSphVector<IndexerRtBulkColumn_t> m_dColumns;

	void Leave()
	{
		if ( !m_pPool )
			return;
		m_sCsv.Rewind();
		m_iRows = 0;
		m_bLayout = false;
		m_pPool->Leave();
		m_pPool = nullptr;
	}

	// tStmt inserts the columns of the prepared layout with the same row width, so m_tMap and m_dColumns fit its rows
	bool IsSameLayout ( const SqlStmt_t & tStmt ) const
	{
		if ( !m_bLayout )
			return false;

		if ( tStmt.m_iSchemaSz<=0 || tStmt.m_iSchemaSz!=m_iSchemaSz )
			return false;

		if ( tStmt.m_dInsertValues.GetLength() % tStmt.m_iSchemaSz )
			return false;

		if ( tStmt.m_dInsertSchema.GetLength()!=m_dInsertSchema.GetLength() )
			return false;

		ARRAY_FOREACH ( i, m_dInsertSchema )
			if ( tStmt.m_dInsertSchema[i]!=m_dInsertSchema[i] )
				return false;

		return true;
	}

	// Join the open pool of the table or, unless shared_join, create it on the first data line.
	SharedBulkResult_e Join ( CSphString & sError )
	{
		while ( true )
		{
			if ( CheckIndexerRtBulkCancelled ( sError ) )
				return SharedBulkResult_e::FAILED;

			auto pPool = GetSharedBulkPool ( m_sTable );
			if ( pPool )
			{
				auto eResult = pPool->Enter ( sError );
				if ( eResult==SharedBulkResult_e::STAGED )
					m_pPool = std::move ( pPool );
				return eResult;
			}

			if ( m_bJoin )
			{
				sError.SetSprintf ( "no shared bulk_import into table '%s' to join", m_sTable.cstr() );
				return SharedBulkResult_e::NO_POOL;
			}

			pPool = new SharedBulkPool_c ( m_sTable, ParseSharedBulkChunks ( m_sTable, m_sShared ) ); // already counts this request as admitted
			if ( !ReplaceSharedBulkPool ( m_sTable, nullptr, pPool ) )
				continue;

			m_pPool = std::move ( pPool );
			auto eResult = m_pPool->Start ( sError );
			if ( eResult==SharedBulkResult_e::STAGED )
				return eResult;
			sError = m_pPool->Fail ( sError ); // wakes the joiners waiting for the start
			ReplaceSharedBulkPool ( m_sTable, m_pPool, nullptr );
			Leave();
			return eResult;
		}
	}

	SharedBulkResult_e Flush ( CSphString & sError )
	{
		if ( !m_sCsv.GetLength() )
			return SharedBulkResult_e::STAGED;
		CSphString sCsv;
		m_sCsv.MoveTo ( sCsv );
		return m_pPool->Write ( sCsv, std::exchange ( m_iRows, 0 ), sError );
	}
};


SharedBulkRequest_c::SharedBulkRequest_c ( const CSphString & sTable, const CSphString & sShared, bool bJoin )
	: m_pImpl { std::make_unique<Impl_t>() }
{
	m_pImpl->m_sTable = sTable;
	m_pImpl->m_sShared = sShared;
	m_pImpl->m_bJoin = bJoin;
}


SharedBulkRequest_c::~SharedBulkRequest_c()
{
	m_pImpl->Leave(); // a broken body drops only its unwritten tail; the pool stays open
}


// Admit before reading the body if the table has an open pool, so a commit elsewhere sees this request;
// a failed or closing pool answers the first data line or the commit.
void SharedBulkRequest_c::Begin()
{
	auto pPool = GetSharedBulkPool ( m_pImpl->m_sTable );
	CSphString sEnter;
	if ( pPool && pPool->Enter ( sEnter )==SharedBulkResult_e::STAGED )
		m_pImpl->m_pPool = std::move ( pPool );
}


SharedBulkResult_e SharedBulkRequest_c::StageLine ( Str_t tLine, CSphString & sError )
{
	GlobalCrashQueryGetRef().m_dQuery = { (const BYTE*) tLine.first, tLine.second };
	SqlStmt_t tStmt;
	tStmt.m_bJson = true;
	tStmt.m_tQuery.m_eQueryType = QUERY_JSON;
	CSphString sStmt, sQuery;
	DocID_t tDocId = 0;
	if ( sphParseJsonStatement ( tLine.first, tStmt, sStmt, sQuery, tDocId, sError, &m_pImpl->m_sTable ) )
		return Stage ( tStmt, sError );

	Fail ( sError );
	return SharedBulkResult_e::FAILED_INPUT;
}


SharedBulkResult_e SharedBulkRequest_c::Stage ( const SqlStmt_t & tStmt, CSphString & sError )
{
	auto & tImpl = *m_pImpl;
	if ( tStmt.m_eStmt!=STMT_INSERT || tStmt.m_sIndex!=tImpl.m_sTable )
	{
		sError.SetSprintf ( "shared bulk_import into table '%s' accepts only INSERT into that table", tImpl.m_sTable.cstr() );
		Fail ( sError );
		return SharedBulkResult_e::FAILED_INPUT;
	}

	if ( !tImpl.m_pPool )
	{
		auto eResult = tImpl.Join ( sError );
		if ( eResult!=SharedBulkResult_e::STAGED )
			return eResult;
	}

	const SharedBulkPool_c & tPool = *tImpl.m_pPool;
	bool bValid;
	if ( tImpl.IsSameLayout ( tStmt ) )
		bValid = ValidateIndexerRtBulkInputValues ( tPool.m_tSchema, tStmt, tImpl.m_tMap, sError );
	else
	{
		if ( !tImpl.m_bLayout )
		{
			auto eStarted = tImpl.m_pPool->WaitStarted ( sError );
			if ( eStarted!=SharedBulkResult_e::STAGED )
				return eStarted;
		}
		EMYSQL_ERR eError;
		tImpl.m_bLayout = false;
		bValid = PrepareIndexerRtBulkInputMap ( tPool.m_tSchema, tStmt, tImpl.m_tMap, eError, sError );
		if ( bValid )
		{
			tImpl.m_dColumns.Resize ( 0 );
			BuildIndexerRtBulkColumns ( tPool.m_tSchema, tImpl.m_tMap, tPool.m_dCsvAttrOrder, tImpl.m_dColumns );
			tImpl.m_dInsertSchema = tStmt.m_dInsertSchema;
			tImpl.m_iSchemaSz = tStmt.m_iSchemaSz;
			tImpl.m_bLayout = true;
		}
	}
	if ( !bValid )
	{
		Fail ( sError );
		return SharedBulkResult_e::FAILED_INPUT;
	}

	AppendIndexerRtBulkCsv ( tImpl.m_sCsv, tImpl.m_dColumns, tStmt );
	tImpl.m_iRows += tStmt.m_iRowsAffected;
	return tImpl.m_sCsv.GetLength()<SHARED_BULK_BATCH_BYTES ? SharedBulkResult_e::STAGED : tImpl.Flush ( sError );
}


// A commit request carries no documents; Begin() may have admitted it to the open pool already.
SharedBulkResult_e SharedBulkRequest_c::Commit ( CSphString & sError )
{
	auto & tImpl = *m_pImpl;
	SharedBulkPoolRefPtr_c pPool = tImpl.m_pPool ? tImpl.m_pPool : GetSharedBulkPool ( tImpl.m_sTable );
	if ( !pPool )
	{
		sError.SetSprintf ( "no shared bulk_import into table '%s' to commit", tImpl.m_sTable.cstr() );
		return SharedBulkResult_e::NO_POOL;
	}
	return pPool->Publish ( !!tImpl.m_pPool, sError );
}


SharedBulkResult_e SharedBulkRequest_c::End ( CSphString & sError )
{
	return m_pImpl->m_pPool ? m_pImpl->Flush ( sError ) : SharedBulkResult_e::STAGED;
}


// An invalid document fails the table's open pool, also when this request has not joined it yet; a pool that is
// already being published is left alone by requests outside it.
void SharedBulkRequest_c::Fail ( const CSphString & sError )
{
	if ( m_pImpl->m_pPool )
		m_pImpl->m_pPool->Fail ( sError );
	else if ( auto pPool = GetSharedBulkPool ( m_pImpl->m_sTable ) )
		pPool->FailOpen ( sError );
}


bool PurgeSharedBulkImport ( const CSphString & sTable, bool bClearSessions, CSphString & sError )
{
	auto pPool = GetSharedBulkPool ( sTable );
	if ( !pPool )
		return true;
	if ( !bClearSessions )
	{
		sError.SetSprintf ( "shared bulk_import into table '%s' is open or failed; commit it or use OPTION clear_sessions=1 to cancel it", sTable.cstr() );
		return false;
	}
	if ( !pPool->FailOpen ( "shared bulk_import was cancelled by PURGE BULK_IMPORT" ) )
	{
		sError.SetSprintf ( "shared bulk_import into table '%s' is being published; repeat PURGE after it finishes", sTable.cstr() );
		return false;
	}
	ReplaceSharedBulkPool ( sTable, pPool, nullptr );
	return true;
}


static void DropSharedBulkImport ( const CSphString & sTable )
{
	auto pPool = GetSharedBulkPool ( sTable );
	if ( !pPool )
		return;
	pPool->Fail ( "shared bulk_import was cancelled by DROP TABLE or PURGE BULK_IMPORT" );
	ReplaceSharedBulkPool ( sTable, pPool, nullptr );
}


void ShutdownSharedBulkImport()
{
	// shutdown callbacks run on the main thread; the pool mutex needs a coroutine
	Threads::CallCoroutine ( [] {
		auto pPools = g_hSharedBulk.GetHash();
		for ( const auto & tPool : *pPools )
			ConstCastPtr ( tPool.second )->Fail ( "Server shutdown in progress" );
		g_hSharedBulk.ReleaseAndClear();
	} );
}
