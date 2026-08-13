#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif

//
// Copyright (c) 2017-2026, Manticore Software LTD
//
// Experimental indexer-assisted RT bulk loader (dev#2761).
//

#include "indexer_rt_bulk.h"

#include "client_session.h"
#include "fileutils.h"
#include "indexfiles.h"
#include "indexsettings.h"
#include "knnmisc.h"
#include "searchdaemon.h"
#include "searchdsql.h"
#include "sphinxrt.h"
#include "threadutils.h"

#include <atomic>
#include <cmath>

#if !_WIN32
#include <fcntl.h>
#include <signal.h>
#include <spawn.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

extern char ** environ;
#endif

bool AttachIndexerRtBulkChunk ( const CSphString & sTable, int64_t iIndexId, const CSphString & sPath, CSphString & sError );


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


static CSphString GetIndexerPath()
{
	const char * szIndexer = getenv ( "MANTICORE_INDEXER_RT_INDEXER" );
	if ( szIndexer )
		return szIndexer;

	CSphString sExecutable = GetExecutablePath();
	if ( sExecutable.IsEmpty() )
		return "indexer";

	CSphString sIndexer;
	sIndexer.SetSprintf ( "%s/indexer", GetPathOnly ( sExecutable ).cstr() );
	return sIndexer;
}


#if !_WIN32
static bool WaitIndexerRtBulk ( ClientSession_c & tSession, CSphString & sError )
{
	if ( tSession.m_iIndexerRtBulkPid<0 )
		return true;

	int iStatus = 0;
	pid_t iResult;
	do
	{
		iResult = waitpid ( tSession.m_iIndexerRtBulkPid, &iStatus, 0 );
	} while ( iResult<0 && errno==EINTR );
	tSession.m_iIndexerRtBulkPid = -1;

	if ( iResult<0 )
	{
		sError.SetSprintf ( "failed waiting for indexer RT bulk process: %s", strerrorm ( errno ) );
		return false;
	}
	if ( WIFEXITED ( iStatus ) && WEXITSTATUS ( iStatus )==0 )
		return true;
	if ( WIFEXITED ( iStatus ) )
		sError.SetSprintf ( "indexer RT bulk build failed with status %d", WEXITSTATUS ( iStatus ) );
	else if ( WIFSIGNALED ( iStatus ) )
		sError.SetSprintf ( "indexer RT bulk build was killed by signal %d", WTERMSIG ( iStatus ) );
	else
		sError = "indexer RT bulk build failed";
	return false;
}


static void StopIndexerRtBulk ( ClientSession_c & tSession )
{
	if ( tSession.m_iIndexerRtBulkPid<0 )
		return;

	// The indexer's csvpipe command is its child, so terminate the entire process group.
	const pid_t iPid = tSession.m_iIndexerRtBulkPid;
	kill ( -iPid, SIGTERM );
	kill ( iPid, SIGTERM );
	int iStatus = 0;
	for ( int i=0; i<100; ++i )
	{
		pid_t iResult = waitpid ( iPid, &iStatus, WNOHANG );
		if ( iResult==iPid || ( iResult<0 && errno==ECHILD ) )
		{
			tSession.m_iIndexerRtBulkPid = -1;
			return;
		}
		if ( iResult<0 && errno!=EINTR )
			break;
		usleep ( 10000 );
	}

	kill ( -iPid, SIGKILL );
	kill ( iPid, SIGKILL );
	while ( waitpid ( iPid, &iStatus, 0 )<0 && errno==EINTR ) {}
	tSession.m_iIndexerRtBulkPid = -1;
}


static bool StartIndexerRtBulk ( ClientSession_c & tSession, CSphString & sError )
{
	int dSockets[2] = { -1, -1 };
	int iSocketType = SOCK_STREAM;
	#ifdef SOCK_CLOEXEC
	iSocketType |= SOCK_CLOEXEC;
	#endif
	if ( socketpair ( AF_UNIX, iSocketType, 0, dSockets )<0 )
	{
		sError.SetSprintf ( "failed to create indexer RT bulk stream: %s", strerrorm ( errno ) );
		return false;
	}
	#ifndef SOCK_CLOEXEC
	if ( fcntl ( dSockets[0], F_SETFD, FD_CLOEXEC )<0 || fcntl ( dSockets[1], F_SETFD, FD_CLOEXEC )<0 )
	{
		close ( dSockets[0] );
		close ( dSockets[1] );
		sError.SetSprintf ( "failed to protect indexer RT bulk stream descriptors: %s", strerrorm ( errno ) );
		return false;
	}
	#endif

	CSphString sIndexer = GetIndexerPath();
	const char * szIndexer = sIndexer.cstr();
	const char * szConfig = tSession.m_sIndexerRtBulkConfig.cstr();
	posix_spawn_file_actions_t tActions;
	posix_spawnattr_t tAttrs;
	bool bActionsInit = false;
	bool bAttrsInit = false;
	int iSpawnError = posix_spawn_file_actions_init ( &tActions );
	if ( !iSpawnError )
	{
		bActionsInit = true;
		iSpawnError = posix_spawn_file_actions_adddup2 ( &tActions, dSockets[1], STDIN_FILENO );
	}
	if ( !iSpawnError && dSockets[0]!=STDIN_FILENO )
		iSpawnError = posix_spawn_file_actions_addclose ( &tActions, dSockets[0] );
	if ( !iSpawnError && dSockets[1]!=STDIN_FILENO )
		iSpawnError = posix_spawn_file_actions_addclose ( &tActions, dSockets[1] );
	#if defined(__linux__)
	if ( !iSpawnError )
		iSpawnError = posix_spawn_file_actions_addclosefrom_np ( &tActions, STDERR_FILENO+1 );
	#endif
	if ( !iSpawnError )
	{
		iSpawnError = posix_spawnattr_init ( &tAttrs );
		bAttrsInit = !iSpawnError;
	}
	if ( !iSpawnError )
	{
		sigset_t tSignals;
		sigemptyset ( &tSignals );
		iSpawnError = posix_spawnattr_setsigmask ( &tAttrs, &tSignals );
		if ( !iSpawnError )
			iSpawnError = posix_spawnattr_setpgroup ( &tAttrs, 0 );
		if ( !iSpawnError )
		{
			short iFlags = POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_SETSIGMASK;
			#ifdef POSIX_SPAWN_CLOEXEC_DEFAULT
			iFlags |= POSIX_SPAWN_CLOEXEC_DEFAULT;
			#endif
			iSpawnError = posix_spawnattr_setflags ( &tAttrs, iFlags );
		}
	}

	pid_t iPid = -1;
	if ( !iSpawnError )
	{
		char sConfigArg[] = "--config";
		char sIndexArg[] = "indexer_rt_bulk_chunk";
		char sQuietArg[] = "--quiet";
		char * dArgv[] = { const_cast<char *>( szIndexer ), sConfigArg, const_cast<char *>( szConfig ), sIndexArg, sQuietArg, nullptr };
		iSpawnError = posix_spawnp ( &iPid, szIndexer, &tActions, &tAttrs, dArgv, environ );
	}
	if ( bAttrsInit )
		posix_spawnattr_destroy ( &tAttrs );
	if ( bActionsInit )
		posix_spawn_file_actions_destroy ( &tActions );
	close ( dSockets[1] );
	if ( iSpawnError )
	{
		close ( dSockets[0] );
		sError.SetSprintf ( "failed to start indexer RT bulk process: %s", strerrorm ( iSpawnError ) );
		return false;
	}

	tSession.m_iIndexerRtBulkPid = iPid;
	tSession.m_pIndexerRtBulkStream = fdopen ( dSockets[0], "wb" );
	if ( !tSession.m_pIndexerRtBulkStream )
	{
		int iFdopenErrno = errno;
		close ( dSockets[0] );
		StopIndexerRtBulk ( tSession );
		sError.SetSprintf ( "failed to open indexer RT bulk stream: %s", strerrorm ( iFdopenErrno ) );
		return false;
	}
	setvbuf ( tSession.m_pIndexerRtBulkStream, nullptr, _IONBF, 0 );
	return true;
}
#endif


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


void CleanupIndexerRtBulk ( ClientSession_c & tSession )
{
	if ( tSession.m_pIndexerRtBulkStream )
	{
		fclose ( tSession.m_pIndexerRtBulkStream );
		tSession.m_pIndexerRtBulkStream = nullptr;
	}

	#if !_WIN32
	StopIndexerRtBulk ( tSession );
	#endif

	if ( !tSession.m_sIndexerRtBulkIndex.IsEmpty() )
		IndexFiles_c ( tSession.m_sIndexerRtBulkIndex, "indexer RT bulk" ).UnlinkExisted();

	if ( !tSession.m_sIndexerRtBulkConfig.IsEmpty() )
		::unlink ( tSession.m_sIndexerRtBulkConfig.cstr() );

	if ( !tSession.m_sIndexerRtBulkDir.IsEmpty() )
		::rmdir ( tSession.m_sIndexerRtBulkDir.cstr() );

	tSession.m_sIndexerRtBulkTable = "";
	tSession.m_iIndexerRtBulkIndexId = -1;
	tSession.m_sIndexerRtBulkDir = "";
	tSession.m_sIndexerRtBulkConfig = "";
	tSession.m_sIndexerRtBulkIndex = "";
}


static bool CheckSchemaSupported ( const CSphSchema & tSchema, CSphString & sError )
{
	const CSphColumnInfo * pDocid = tSchema.GetAttr ( sphGetDocidName() );
	if ( !pDocid || pDocid->IsUuidLinkedDocid() )
	{
		sError = "indexer RT bulk prototype supports numeric document ids only";
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
			sError.SetSprintf ( "indexer RT bulk prototype does not support attribute '%s'", tAttr.m_sName.cstr() );
			return false;
		}
	}
	return true;
}


static bool InitIndexerRtBulk ( ClientSession_c & tSession, const SqlStmt_t & tStmt, const cServedIndexRefPtr_c & pServed, CSphString & sError )
{
	if ( !tSession.m_sIndexerRtBulkTable.IsEmpty() )
	{
		if ( tSession.m_sIndexerRtBulkTable!=tStmt.m_sIndex )
		{
			sError = "indexer RT bulk transaction may target only one table";
			return false;
		}
		return true;
	}

	RIdx_T<RtIndex_i *> pRt { pServed };
	const CSphSchema & tSchema = pRt->GetMatchSchema();
	if ( !CheckSchemaSupported ( tSchema, sError ) )
		return false;

	static std::atomic<DWORD> uBulkId { 0 };
	CSphString sParent = GetPathOnly ( pRt->GetFilebase() );
	tSession.m_sIndexerRtBulkDir.SetSprintf ( "%s/indexer-rt-bulk-%d-%u", sParent.cstr(), GetOsProcessId(), ++uBulkId );
	if ( !MkDir ( tSession.m_sIndexerRtBulkDir.cstr() ) )
	{
		sError.SetSprintf ( "failed to create bulk staging directory '%s': %s", tSession.m_sIndexerRtBulkDir.cstr(), strerrorm ( errno ) );
		return false;
	}

	tSession.m_sIndexerRtBulkConfig.SetSprintf ( "%s/indexer.conf", tSession.m_sIndexerRtBulkDir.cstr() );
	tSession.m_sIndexerRtBulkIndex.SetSprintf ( "%s/chunk", tSession.m_sIndexerRtBulkDir.cstr() );
	FILE * fpConfig = fopen ( tSession.m_sIndexerRtBulkConfig.cstr(), "wb" );
	if ( !fpConfig )
	{
		sError.SetSprintf ( "failed to create bulk staging config in '%s': %s", tSession.m_sIndexerRtBulkDir.cstr(), strerrorm ( errno ) );
		CleanupIndexerRtBulk ( tSession );
		return false;
	}

	fprintf ( fpConfig, "source indexer_rt_bulk_source {\n  type = csvpipe\n  csvpipe_command = /bin/cat\n" );
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
			fprintf ( fpConfig, "  %s = %s\n", CsvAttrDirective ( tAttr.m_eAttrType ), tAttr.m_sName.cstr() );
		}
	fprintf ( fpConfig, "}\n\nindex indexer_rt_bulk_chunk {\n  type = plain\n  source = indexer_rt_bulk_source\n  path = %s\n", tSession.m_sIndexerRtBulkIndex.cstr() );
	DumpSettingsCfg ( fpConfig, *pRt, nullptr );
	if ( pRt->GetSettings().m_dKNN.GetLength() )
		fprintf ( fpConfig, "\n	knn = %s", FormatKNNConfigStr ( pRt->GetSettings().m_dKNN ).cstr() );
	fprintf ( fpConfig, "\n}\n" );
	fclose ( fpConfig );

	tSession.m_sIndexerRtBulkTable = tStmt.m_sIndex;
	tSession.m_iIndexerRtBulkIndexId = pRt->GetIndexId();
	#if _WIN32
	sError = "streaming indexer RT bulk is not implemented on Windows";
	CleanupIndexerRtBulk ( tSession );
	return false;
	#else
	if ( !StartIndexerRtBulk ( tSession, sError ) )
	{
		CleanupIndexerRtBulk ( tSession );
		return false;
	}
	#endif
	return true;
}


bool StageIndexerRtBulk ( ClientSession_c & tSession, const SqlStmt_t & tStmt, CSphString & sError )
{
	if ( !tSession.m_bInTransaction )
	{
		sError = "indexer_rt_bulk requires an active transaction; use BEGIN before INSERT";
		return false;
	}
	if ( tStmt.m_eStmt!=STMT_INSERT )
	{
		sError = "indexer_rt_bulk prototype supports INSERT only";
		return false;
	}

	auto pServed = GetServed ( tStmt.m_sIndex );
	if ( !ServedDesc_t::IsMutable ( pServed ) )
	{
		sError.SetSprintf ( "table '%s' is absent, or not real-time", tStmt.m_sIndex.cstr() );
		return false;
	}
	if ( FindInsertColumn ( tStmt, sphGetDocidName() )<0 )
	{
		sError = "indexer RT bulk prototype requires an explicit id";
		return false;
	}
	if ( !InitIndexerRtBulk ( tSession, tStmt, pServed, sError ) )
		return false;

	RIdx_T<RtIndex_i *> pRt { pServed };
	const CSphSchema & tSchema = pRt->GetMatchSchema();
	const int iColumns = tStmt.m_iSchemaSz;
	for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_eAttrType!=SPH_ATTR_FLOAT_VECTOR )
			continue;

		int iColumn = FindInsertColumn ( tStmt, tAttr.m_sName );
		if ( iColumn<0 )
			continue;

		for ( int iRow=0; iRow<tStmt.m_iRowsAffected; ++iRow )
			if ( !ValidateFloatVectorValue ( tAttr, tStmt.m_dInsertValues[iRow*iColumns+iColumn], iRow, sError ) )
			{
				CleanupIndexerRtBulk ( tSession );
				return false;
			}
	}

	struct CsvColumn_t { int m_iColumn; ESphAttr m_eType; bool m_bDocid; };
	CSphVector<CsvColumn_t> dColumns;
	auto AddColumn = [&] ( const CSphString & sName, ESphAttr eType, bool bDocid ) {
		dColumns.Add ( { FindInsertColumn ( tStmt, sName ), eType, bDocid } );
	};
	AddColumn ( sphGetDocidName(), SPH_ATTR_BIGINT, true );
	for ( int i=0; i<tSchema.GetFieldsCount(); ++i )
		AddColumn ( tSchema.GetField(i).m_sName, SPH_ATTR_STRING, false );
	for ( ESphAttr eType : g_dCsvAttrOrder )
		for ( int i=0; i<tSchema.GetAttrsCount(); ++i )
		{
			const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
			if ( tAttr.m_eAttrType!=eType || tAttr.m_sName==sphGetDocidName() || sphIsInternalAttr ( tAttr ) )
				continue;
			if ( tSchema.GetField ( tAttr.m_sName.cstr() ) && tAttr.m_eAttrType==SPH_ATTR_STRING )
				continue;
			AddColumn ( tAttr.m_sName, tAttr.m_eAttrType, false );
		}

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
	if ( fwrite ( sCsv.cstr(), 1, sCsv.Length(), tSession.m_pIndexerRtBulkStream )!=size_t ( sCsv.Length() ) )
	{
		sError.SetSprintf ( "failed streaming bulk CSV to indexer: %s", strerrorm ( errno ) );
		CleanupIndexerRtBulk ( tSession );
		return false;
	}
	return true;
}


bool FinalizeIndexerRtBulk ( ClientSession_c & tSession, CSphString & sError )
{
	if ( tSession.m_sIndexerRtBulkTable.IsEmpty() )
		return true;

	bool bClosed = true;
	int iCloseErrno = 0;
	if ( tSession.m_pIndexerRtBulkStream )
	{
		bClosed = fclose ( tSession.m_pIndexerRtBulkStream )==0;
		if ( !bClosed )
			iCloseErrno = errno;
		tSession.m_pIndexerRtBulkStream = nullptr;
	}

	#if !_WIN32
	if ( !WaitIndexerRtBulk ( tSession, sError ) )
	{
		CleanupIndexerRtBulk ( tSession );
		return false;
	}
	#endif
	if ( !bClosed )
	{
		sError.SetSprintf ( "failed closing indexer RT bulk stream: %s", strerrorm ( iCloseErrno ) );
		CleanupIndexerRtBulk ( tSession );
		return false;
	}

	bool bAttached = AttachIndexerRtBulkChunk ( tSession.m_sIndexerRtBulkTable, tSession.m_iIndexerRtBulkIndexId, tSession.m_sIndexerRtBulkIndex, sError );
	CleanupIndexerRtBulk ( tSession );
	return bAttached;
}
