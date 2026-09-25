//
// Copyright (c) 2026, Manticore Software LTD
//
// Indexer-assisted RT bulk loader.
//

#include "client_session.h"
#include "conversion.h"
#include "sphinxexpr.h"

#include <boost/filesystem.hpp>
#include <boost/system/errc.hpp>
#include <ostream>
#include <stdlib.h>
#include <type_traits>

#include "indexer_rt_bulk.h"
#include "std/bitvec.h"
#include "std/env.h"
#if _WIN32
#include "coroutine.h"
#endif

#include <boost/version.hpp>
#if BOOST_VERSION >= 108800
#define BOOST_PROCESS_VERSION 1
#include <boost/process/v1/args.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/process/v1/env.hpp>
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


static CSphString GetIndexerRtBulkFile ( const IndexerRtBulkState_t & tState, const char * szFile )
{
	CSphString sPath;
	sPath.SetSprintf ( "%s/%s", tState.m_sDir.cstr(), szFile );
	return sPath;
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


bool RemoveIndexerRtBulkRoot ( const CSphString & sRoot, CSphString & sError )
{
	sError = "";
	boost::system::error_code tError;
	fs::remove_all ( sRoot.cstr(), tError );
	if ( !tError )
		return true;

	sError.SetSprintf ( "failed to remove bulk staging root '%s': %s", sRoot.cstr(), tError.message().c_str() );
	return false;
}


class IndexerRtBulkState_t::Impl_c
{
public:
	~Impl_c()
	{
		Abort();
	}

	bool Start ( const CSphString & sIndexer, const CSphString & sConfig, const CSphString & sOutput, CSphString & sError )
	{
		assert ( !m_tChild.valid() );
		m_sExecutable = sIndexer;

		std::vector<std::string> dArgs { "--config", sConfig.cstr(), "--remove_dups", "indexer_rt_bulk_chunk" };
		boost::process::environment tEnvironment = boost::this_process::environment();
		tEnvironment.erase ( "INDEXER_RT_BULK_CSV_PIPE_COMMAND" );
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
			m_tChild = boost::process::child
			(
				sIndexer.cstr(),
				boost::process::args ( dArgs ),
				boost::process::std_in < tSocketInput,
				( boost::process::std_out & boost::process::std_err ) > sOutput.cstr(),
				tLimitHandles,
				ResetSignalMask_t {},
				boost::process::error ( tError ),
				tEnvironment
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
			boost::process::error ( tError ),
			tEnvironment
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

	bool WriteCsvBatch ( const CSphString & sCsv )
	{
		#if !_WIN32
		return fwrite ( sCsv.cstr(), 1, sCsv.Length(), m_pCsvSocketInput )==size_t ( sCsv.Length() );
		#else
		m_tInput.write ( sCsv.cstr(), sCsv.Length() );
		return bool ( m_tInput );
		#endif
	}

	IndexerRtBulkWait_e CloseInputAndWait ( CSphString & sError )
	{
		if ( CheckIndexerRtBulkCancelled ( sError ) )
		{
			Abort();
			return IndexerRtBulkWait_e::CANCELLED;
		}

		CSphString sInputError;
		bool bInputOk = CloseInput ( sInputError );

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
					AppendIndexerRtBulkError ( sError, sInputError );
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

		AppendIndexerRtBulkError ( sError, sInputError );
		AppendIndexerRtBulkError ( sError, sChildError );
		return bInputOk && sChildError.IsEmpty() ? IndexerRtBulkWait_e::OK : IndexerRtBulkWait_e::FAILED;
	}

	void Abort() noexcept
	{
		DiscardInput();

		if ( m_tChild.valid() )
		{
			std::error_code tRunningError;
			if ( m_tChild.running ( tRunningError ) )
			{
				std::error_code tTerminateError;
				m_tChild.terminate ( tTerminateError );
			}

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

	boost::process::opstream m_tInput;
	boost::process::child m_tChild;
	CSphString m_sExecutable;
#if !_WIN32
	FILE * m_pCsvSocketInput = nullptr;
	CSphFixedVector<char> m_dCsvSocketBuffer { 0 };
#endif
};


static CSphString GetIndexerRtBulkOutput ( const IndexerRtBulkState_t & tState )
{
	CSphString sPath = GetIndexerRtBulkFile ( tState, "indexer.log" );
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


static IndexerRtBulkWait_e WaitIndexerRtBulk ( IndexerRtBulkState_t & tState, CSphString & sError )
{
	if ( !tState.m_pImpl )
		return IndexerRtBulkWait_e::OK;

	auto eResult = tState.m_pImpl->CloseInputAndWait ( sError );
	if ( eResult==IndexerRtBulkWait_e::OK )
		return eResult;

	CSphString sOutput = GetIndexerRtBulkOutput ( tState );
	if ( !sOutput.IsEmpty() )
		AppendIndexerRtBulkError ( sError, sOutput );
	return eResult;
}


static bool StartIndexerRtBulk ( IndexerRtBulkState_t & tState, CSphString & sError )
{
	CSphString sIndexer;
	if ( !GetIndexerPath ( sIndexer, sError ) )
		return false;

	CSphString sConfig = GetIndexerRtBulkFile ( tState, "indexer.conf" );
	CSphString sOutput = GetIndexerRtBulkFile ( tState, "indexer.log" );
	tState.m_pImpl = std::make_unique<IndexerRtBulkState_t::Impl_c>();
	if ( tState.m_pImpl->Start ( sIndexer, sConfig, sOutput, sError ) )
		return true;

	tState.m_pImpl.reset();
	return false;
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


static void AppendInsertValueToCsv ( StringBuilder_c & sOut, const SqlInsert_t & tValue, ESphAttr eType, bool bDocid )
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
		if ( bDocid )
			sOut << tValue.GetValueUint();
		else
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
	auto & tState = tSession.m_tIndexerRtBulk;
	auto pImpl = std::move ( tState.m_pImpl );
	if ( pImpl )
		pImpl->Abort();

	if ( !tState.m_sDir.IsEmpty() )
	{
		fs::path tDir ( tState.m_sDir.cstr() );
		boost::system::error_code tError;
		fs::remove_all ( tDir, tError );
		if ( tError )
			sphWarning ( "bulk_import cleanup failed for '%s': %s", tState.m_sDir.cstr(), tError.message().c_str() );
		else
		{
			tError.clear();
			fs::remove ( tDir.parent_path(), tError );
			if ( tError && tError!=boost::system::errc::directory_not_empty )
				sphWarning ( "bulk_import cleanup failed for '%s': %s", tDir.parent_path().string().c_str(), tError.message().c_str() );
		}
	}

	tState.m_sDir = "";
	tState.m_dCsvAttrOrder.Reset();
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

	IndexerRtBulkTarget_t tTarget;
	if ( !CheckIndexerRtBulkTarget ( sTable, tTarget, sError ) )
		return false;

	if ( !CheckIndexerRtBulkExecutable ( sError ) )
		return false;

	ServedIndexWriteReservation_c tReservation;
	if ( !tReservation.TryAcquire ( tTarget.m_pServed ) )
	{
		sError.SetSprintf ( "table '%s' is locked", sTable.cstr() );
		return false;
	}

	IndexerRtBulkTarget_t tCurrent;
	if ( !CheckIndexerRtBulkTarget ( sTable, tCurrent, sError ) )
		return false;
	if ( !tReservation.Matches ( tCurrent.m_pServed ) || tCurrent.m_iIndexId!=tTarget.m_iIndexId || tCurrent.m_iAlterGeneration!=tTarget.m_iAlterGeneration )
	{
		sError.SetSprintf ( "table '%s' changed while enabling bulk_import", sTable.cstr() );
		return false;
	}

	tState.m_sTable = sTable;
	tState.m_iIndexId = tCurrent.m_iIndexId;
	tState.m_iAlterGeneration = tCurrent.m_iAlterGeneration;
	tState.m_tReservation = std::move ( tReservation );
	return true;
}


static bool InitIndexerRtBulk ( ClientSession_c & tSession, const RtIndex_i & tRt, CSphString & sError )
{
	auto & tState = tSession.m_tIndexerRtBulk;
	if ( tState.HasPendingData() )
		return true;

	const CSphSchema & tSchema = tRt.GetMatchSchema();
	if ( !CheckSchemaSupported ( tSchema, sError ) )
		return false;

	// Use the same type-grouped order for csvpipe directives and streamed values.
	CSphVector<int> dCsvAttrOrder;
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

	CSphString sRoot = GetIndexerRtBulkRoot ( tRt );
	if ( !PrepareStagingRoot ( sRoot, sError ) )
		return false;

	CSphString sDir;
	if ( !CreateStagingDir ( sRoot, sDir, sError ) )
		return false;
	tState.m_sDir = sDir;

	CSphString sConfig = GetIndexerRtBulkFile ( tState, "indexer.conf" );
	CSphString sIndex = GetIndexerRtBulkFile ( tState, "chunk" );
	FILE * fpConfig = fopen ( sConfig.cstr(), "wb" );
	if ( !fpConfig )
	{
		sError.SetSprintf ( "failed to create bulk staging config in '%s': %s", tState.m_sDir.cstr(), strerrorm ( errno ) );
		AbortIndexerRtBulkBatch ( tSession );
		return false;
	}

	CSphString sConfigLemmatizerBase = EscapeIndexerRtBulkConfigPath ( g_sLemmatizerBase );
	fprintf ( fpConfig, "common {\n  lemmatizer_base = %s\n}\n\n", sConfigLemmatizerBase.cstr() );
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

	if ( !StartIndexerRtBulk ( tState, sError ) )
	{
		AbortIndexerRtBulkBatch ( tSession );
		return false;
	}
	tState.m_dCsvAttrOrder = std::move ( dCsvAttrOrder );
	return true;
}


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

	tMap.m_iId = iDocidAttr>=0 ? tMap.m_dAttrs[iDocidAttr] : -1;
	const int iIdColumn = tMap.m_iId;
	if ( iIdColumn<0 )
	{
		sError = "bulk_import requires an explicit id";
		return false;
	}
	for ( int iRow=0; iRow<tStmt.m_iRowsAffected; ++iRow )
	{
		const SqlInsert_t & tId = tStmt.m_dInsertValues[iRow*iColumns+iIdColumn];
		if ( tId.m_iType==SqlInsert_t::CONST_INT && ( tId.IsNegativeInt() || !tId.GetValueUint() ) )
		{
			sError.SetSprintf ( "row %d: bulk_import requires an explicit non-zero id", iRow+1 );
			return false;
		}
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
	BuildIndexerRtBulkColumns ( tSchema, tMap, tState.m_dCsvAttrOrder, dColumns );

	const int iColumns = tStmt.m_iSchemaSz;

	assert ( tState.m_pImpl );
	StringBuilder_c sBatch;
	for ( int iRow=0; iRow<tStmt.m_iRowsAffected; ++iRow )
	{
		bool bFirst = true;
		for ( const auto & tColumn : dColumns )
		{
			if ( !bFirst )
				sBatch << ',';
			bFirst = false;
			if ( tColumn.m_iColumn<0 )
				AppendCsvQuoted ( sBatch, "" );
			else
				AppendInsertValueToCsv ( sBatch, tStmt.m_dInsertValues[iRow*iColumns+tColumn.m_iColumn], tColumn.m_eType, tColumn.m_bDocid );
		}
		sBatch << '\n';
	}

	CSphString sCsv;
	sBatch.MoveTo ( sCsv );
	errno = 0;
	if ( !tState.m_pImpl->WriteCsvBatch ( sCsv ) )
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

	auto eWaitResult = WaitIndexerRtBulk ( tState, sError );
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
	auto eResult = AttachIndexerRtBulkChunk ( tState.m_sTable, tState.m_tReservation, tState.m_iIndexId, tState.m_iAlterGeneration, GetIndexerRtBulkFile ( tState, "chunk" ), bTargetStale, sError );
	if ( bTargetStale )
		CleanupIndexerRtBulk ( tSession );
	else
		AbortIndexerRtBulkBatch ( tSession );
	return eResult==DiskAttachRes_e::OK;
}
