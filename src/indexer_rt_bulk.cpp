//
// Copyright (c) 2026, Manticore Software LTD
//
// Indexer-assisted RT bulk loader.
//

#include "client_session.h"

#include <filesystem>

#include "indexer_rt_bulk.h"

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
#include <signal.h>
#endif

namespace fs = std::filesystem;

DiskAttachRes_e AttachIndexerRtBulkChunk ( const CSphString & sTable, const ServedIndexWriteReservation_c & tReservation, int64_t iIndexId, int iAlterGeneration, const CSphString & sPath, bool & bTargetStale, CSphString & sError );


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

static void AppendCsvEscaped ( std::ostream & tOut, const char * szValue )
{
	const char * pStart = szValue ? szValue : "";
	for ( const char * p = pStart; ; ++p )
	{
		if ( *p!='"' && *p!='\0' )
			continue;
		tOut.write ( pStart, p-pStart );
		if ( !*p )
			break;
		tOut.write ( "\"\"", 2 );
		pStart = p+1;
	}
}


static void AppendCsvQuoted ( std::ostream & tOut, const char * szValue )
{
	tOut.put ( '"' );
	AppendCsvEscaped ( tOut, szValue );
	tOut.put ( '"' );
}


static bool GetIndexerPath ( CSphString & sIndexer, CSphString & sError )
{
#if STATIC_BINARY
	sError = "indexer RT bulk is unavailable in static builds";
	return false;
#else
	CSphString sExecutable = GetExecutablePath();
	if ( sExecutable.IsEmpty() )
	{
		sError = "failed to locate the running searchd executable for indexer RT bulk";
		return false;
	}

	#if _WIN32
	const char * szIndexer = "indexer.exe";
	#else
	const char * szIndexer = "indexer";
	#endif
	sIndexer.SetSprintf ( "%s%s", GetPathOnly ( sExecutable ).cstr(), szIndexer );
	return true;
#endif
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
	std::error_code tError;
	fs::recursive_directory_iterator tIt ( sDir.cstr(), tError );
	for ( const fs::recursive_directory_iterator tEnd; !tError && tIt!=tEnd; tIt.increment ( tError ) )
	{
		auto & tFile = dFiles.Add();
		tFile.m_sPath = tIt->path().string().c_str();
		if ( tIt->is_regular_file ( tError ) )
		{
			auto iSize = tIt->file_size ( tError );
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
	std::error_code tError;
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
		std::error_code tError;
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
	std::error_code tError;
	auto tStatus = fs::status ( sRoot.cstr(), tError );
	if ( tError==std::errc::no_such_file_or_directory )
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
	std::error_code tError;
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
		std::error_code tError;
		try
		{
			m_tChild = boost::process::child
			(
				sIndexer.cstr(),
				boost::process::args ( dArgs ),
				boost::process::std_in < m_tInput,
				( boost::process::std_out & boost::process::std_err ) > sOutput.cstr(),
				boost::process::limit_handles,
				#if !_WIN32
				ResetSignalMask_t {},
				#endif
				boost::process::error ( tError ),
				tEnvironment
			);
		}
		catch ( const std::exception & tException )
		{
			sError.SetSprintf ( "failed to start indexer RT bulk process '%s': %s", sIndexer.cstr(), tException.what() );
			Abort();
			return false;
		}
		catch ( ... )
		{
			sError.SetSprintf ( "failed to start indexer RT bulk process '%s'", sIndexer.cstr() );
			Abort();
			return false;
		}

		if ( tError || !m_tChild.valid() )
		{
			if ( tError )
				sError.SetSprintf ( "failed to start indexer RT bulk process '%s': %s", sIndexer.cstr(), tError.message().c_str() );
			else
				sError.SetSprintf ( "failed to start indexer RT bulk process '%s'", sIndexer.cstr() );
			Abort();
			return false;
		}

		return true;
	}

	std::ostream & Input()
	{
		return m_tInput;
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
			sChildError = "indexer RT bulk process is unavailable";
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
						sChildError.SetSprintf ( "failed checking indexer RT bulk process: %s", tRunningError.message().c_str() );
					break;
				}

				Threads::Coro::SleepMsec ( INDEXER_RT_BULK_WAIT_INTERVAL_MS );
			}

			if ( sChildError.IsEmpty() )
			{
				std::error_code tWaitError;
				m_tChild.wait ( tWaitError );
				if ( tWaitError )
					sChildError.SetSprintf ( "failed waiting for indexer RT bulk process: %s", tWaitError.message().c_str() );
				else if ( m_tChild.exit_code()!=0 )
					sChildError.SetSprintf ( "indexer RT bulk build process '%s' failed with status %d", m_sExecutable.cstr(), m_tChild.exit_code() );
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
		try
		{
			m_tInput.flush();
			if ( !m_tInput )
			{
				sError = "failed flushing indexer RT bulk stream";
				bOk = false;
			}
		}
		catch ( const std::exception & tException )
		{
			sError.SetSprintf ( "failed flushing indexer RT bulk stream: %s", tException.what() );
			bOk = false;
		}
		catch ( ... )
		{
			sError = "failed flushing indexer RT bulk stream";
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
			sCloseError.SetSprintf ( "failed closing indexer RT bulk stream: %s", tException.what() );
			AppendIndexerRtBulkError ( sError, sCloseError );
			bOk = false;
		}
		catch ( ... )
		{
			AppendIndexerRtBulkError ( sError, CSphString ( "failed closing indexer RT bulk stream" ) );
			bOk = false;
		}

		return bOk;
	}

	void DiscardInput() noexcept
	{
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


template <typename VALUE>
static void AppendCsvInteger ( std::ostream & tOut, VALUE tValue )
{
	char sValue[32];
	auto tResult = std::to_chars ( sValue, sValue+sizeof(sValue), tValue );
	if ( tResult.ec!=std::errc() )
	{
		tOut.setstate ( std::ios_base::badbit );
		return;
	}
	tOut.write ( sValue, tResult.ptr-sValue );
}


static void AppendCsvFloat ( std::ostream & tOut, double fValue )
{
	char sValue[64];
	int iLength = snprintf ( sValue, sizeof(sValue), "%.9g", fValue );
	if ( iLength<0 || iLength>=int(sizeof(sValue)) )
	{
		tOut.setstate ( std::ios_base::badbit );
		return;
	}
	tOut.write ( sValue, iLength );
}


static void AppendInsertValueToCsv ( std::ostream & tOut, const SqlInsert_t & tValue, ESphAttr eType, bool bDocid )
{
	tOut.put ( '"' );
	switch ( tValue.m_iType )
	{
	case SqlInsert_t::QUOTED_STRING:
		AppendCsvEscaped ( tOut, tValue.m_sVal.cstr() );
		break;

	case SqlInsert_t::CONST_FLOAT:
		AppendCsvFloat ( tOut, tValue.m_fVal );
		break;

	case SqlInsert_t::CONST_INT:
		if ( bDocid )
			AppendCsvInteger ( tOut, tValue.GetValueUint() );
		else
			AppendCsvInteger ( tOut, tValue.GetValueInt() );
		break;

	case SqlInsert_t::CONST_MVA:
		if ( tValue.m_pVals )
		{
			bool bFirst = true;
			for ( const auto & tItem : *tValue.m_pVals )
			{
				if ( !bFirst )
					tOut.put ( ' ' );
				bFirst = false;
				if ( eType==SPH_ATTR_FLOAT_VECTOR )
					AppendCsvFloat ( tOut, tItem.m_fValue );
				else
					AppendCsvInteger ( tOut, tItem.m_iValue );
			}
		}
		break;

	case SqlInsert_t::TOK_NULL:
		break;

	default:
		AppendCsvEscaped ( tOut, tValue.m_sVal.cstr() );
		break;
	}
	tOut.put ( '"' );
}


static int FindInsertColumn ( const SqlStmt_t & tStmt, const CSphString & sName )
{
	return tStmt.m_dInsertSchema.GetFirst ( [&sName] ( const CSphString & sColumn ) { return sColumn==sName; } );
}


static const char * CsvAttrDirective ( ESphAttr eType )
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


static constexpr ESphAttr g_dCsvAttrOrder[] =
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
		std::error_code tError;
		fs::remove_all ( tDir, tError );
		if ( tError )
			sphWarning ( "indexer RT bulk cleanup failed for '%s': %s", tState.m_sDir.cstr(), tError.message().c_str() );
		else
		{
			tError.clear();
			fs::remove ( tDir.parent_path(), tError );
			if ( tError && tError!=std::errc::directory_not_empty )
				sphWarning ( "indexer RT bulk cleanup failed for '%s': %s", tDir.parent_path().string().c_str(), tError.message().c_str() );
		}
	}

	tState.m_sDir = "";
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
		sError = "indexer RT bulk supports numeric document ids only";
		return false;
	}

	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( sphIsInternalAttr ( tAttr ) )
			continue;
		ESphAttr eType = tAttr.m_eAttrType;
		if ( eType!=SPH_ATTR_TOKENCOUNT && !CsvAttrDirective ( eType ) )
		{
			sError.SetSprintf ( "indexer RT bulk does not support attribute '%s'", tAttr.m_sName.cstr() );
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

	sError.SetSprintf ( "indexer RT bulk executable '%s' is unavailable: %s", sIndexer.cstr(), sFileError.cstr() );
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
	fprintf ( fpConfig, "source indexer_rt_bulk_source {\n  type = csvpipe\n  csvpipe_command = -\n" );
	for ( int i=0; i<tSchema.GetFieldsCount(); ++i )
	{
		const CSphColumnInfo & tField = tSchema.GetField(i);
		const CSphColumnInfo * pSameAttr = tSchema.GetAttr ( tField.m_sName.cstr() );
		fprintf ( fpConfig, "  %s = %s\n", pSameAttr && pSameAttr->m_eAttrType==SPH_ATTR_STRING ? "csvpipe_field_string" : "csvpipe_field", tField.m_sName.cstr() );
	}
	for ( ESphAttr eType : g_dCsvAttrOrder )
		for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
		{
			const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
			if ( tAttr.m_eAttrType!=eType || tAttr.m_sName==sphGetDocidName() || sphIsInternalAttr ( tAttr ) )
				continue;
			const CSphColumnInfo * pSameField = tSchema.GetField ( tAttr.m_sName.cstr() );
			if ( pSameField && tAttr.m_eAttrType==SPH_ATTR_STRING )
				continue;
			fprintf ( fpConfig, "  %s = %s", CsvAttrDirective ( tAttr.m_eAttrType ), tAttr.m_sName.cstr() );
			if ( tAttr.m_eAttrType==SPH_ATTR_INTEGER && tAttr.m_tLocator.IsBitfield() )
				fprintf ( fpConfig, ":%d", tAttr.m_tLocator.m_iBitCount );
			fprintf ( fpConfig, "\n" );
		}
	fprintf ( fpConfig, "  csvpipe_attr_order = " );
	bool bFirstAttr = true;
	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_sName==sphGetDocidName() || sphIsInternalAttr ( tAttr ) || !CsvAttrDirective ( tAttr.m_eAttrType ) )
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
	return true;
}


class MapError_c final : public StmtErrorReporter_i
{
public:
	MapError_c ( EMYSQL_ERR & eError, CSphString & sError )
		: m_eError ( eError )
		, m_sError ( sError )
	{}

	void Ok ( int, const CSphString &, int64_t ) final { assert ( 0 ); }
	void Ok ( int, int ) final { assert ( 0 ); }
	void ErrorEx ( EMYSQL_ERR eError, const char * sError ) final
	{
		m_eError = eError;
		m_sError = sError;
	}
	RowBuffer_i * GetBuffer() final { return nullptr; }

private:
	EMYSQL_ERR &	m_eError;
	CSphString &	m_sError;
};


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
	CSphVector<int> dAttrSchema ( tSchema.GetAttrsCount() );
	CSphVector<int> dFieldSchema ( tSchema.GetFieldsCount() );
	CSphVector<bool> dFieldAttrs ( tSchema.GetFieldsCount() );
	MapError_c tError { eError, sError };
	if ( !CreateAttrMaps ( dAttrSchema, dFieldSchema, dFieldAttrs, tSchema, tStmt.m_dInsertSchema, tError ) )
		return false;

	const int iIdColumn = FindInsertColumn ( tStmt, sphGetDocidName() );
	if ( iIdColumn<0 )
	{
		sError = "indexer RT bulk requires an explicit id";
		return false;
	}
	const int iColumns = tStmt.m_iSchemaSz;
	for ( int iRow=0; iRow<tStmt.m_iRowsAffected; ++iRow )
	{
		const SqlInsert_t & tId = tStmt.m_dInsertValues[iRow*iColumns+iIdColumn];
		if ( tId.m_iType==SqlInsert_t::CONST_INT && !tId.IsNegativeInt() && !tId.GetValueUint() )
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

		const int iColumn = dAttrSchema[i];
		if ( iColumn<0 )
			continue;

		for ( int iRow=0; iRow<tStmt.m_iRowsAffected; ++iRow )
			if ( !ValidateFloatVectorValue ( tAttr, tStmt.m_dInsertValues[iRow*iColumns+iColumn], iRow, sError ) )
				return false;
	}
	if ( !InitIndexerRtBulk ( tSession, *pRt.Ptr(), sError ) )
		return false;

	struct CsvColumn_t { int m_iColumn; ESphAttr m_eType; bool m_bDocid; };
	CSphVector<CsvColumn_t> dColumns;
	dColumns.Add ( { iIdColumn, SPH_ATTR_BIGINT, true } );
	for ( int i=0; i<tSchema.GetFieldsCount(); ++i )
		dColumns.Add ( { dFieldSchema[i], SPH_ATTR_STRING, false } );
	for ( ESphAttr eType : g_dCsvAttrOrder )
		for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
		{
			const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
			if ( tAttr.m_eAttrType!=eType || tAttr.m_sName==sphGetDocidName() || sphIsInternalAttr ( tAttr ) )
				continue;
			if ( tSchema.GetField ( tAttr.m_sName.cstr() ) && tAttr.m_eAttrType==SPH_ATTR_STRING )
				continue;
			dColumns.Add ( { dAttrSchema[i], tAttr.m_eAttrType, false } );
		}

	assert ( tState.m_pImpl );
	std::ostream & tStream = tState.m_pImpl->Input();
	errno = 0;
	for ( int iRow=0; iRow<tStmt.m_iRowsAffected; ++iRow )
	{
		bool bFirst = true;
		for ( const auto & tColumn : dColumns )
		{
			if ( !bFirst )
				tStream.put ( ',' );
			bFirst = false;
			if ( tColumn.m_iColumn<0 )
				AppendCsvQuoted ( tStream, "" );
			else
				AppendInsertValueToCsv ( tStream, tStmt.m_dInsertValues[iRow*iColumns+tColumn.m_iColumn], tColumn.m_eType, tColumn.m_bDocid );
		}
		tStream.put ( '\n' );
	}

	if ( !tStream )
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
			if ( RecheckTarget ( tSession, sTargetError, "finalizing indexer RT bulk" ) )
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
