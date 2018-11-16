
#include "sphinx.h"
#include "sphinxstd.h"
#include "sphinxutils.h"
#include "sphinxint.h"
#include "searchdaemon.h"
#include "json/cJSON.h"
#include "replication/wsrep_api.h"

#define USE_PSI_INTERFACE 1

// global application context for wsrep callbacks
struct AppCtx_t
{
	CSphAutoEvent * m_pSync = nullptr;
	Abort_fn m_fnAbort = nullptr;
};
static AppCtx_t g_tCtx;

// receiving thread context for wsrep callbacks
struct ReceiverCtx_t
{
	ReplicationCluster_t * m_pCluster = nullptr;

	CSphVector<ReplicationCommand_t> m_dCommands;

	ReceiverCtx_t() = default;
	~ReceiverCtx_t()
	{
		ResetCommands();
	}

	void ResetCommands ()
	{
		ARRAY_FOREACH ( i, m_dCommands )
			SafeDelete ( m_dCommands[i].m_pStored );

		m_dCommands.Reset();
	}
};

// Logging stuff for the loader
static const char * g_sClusterStatus[WSREP_VIEW_MAX] = { "primary", "non-primary", "disconnected" };
static const char * g_sStatusDesc[] = {
	"success",
    "warning",
    "transaction is not known",
    "transaction aborted, server can continue",
    "transaction was victim of brute force abort",
    "data exceeded maximum supported size",
    "error in client connection, must abort",
	"error in node state, wsrep must reinit",
    "fatal error, server must abort",
    "transaction was aborted before commencing pre-commit",
    "feature not implemented"
};

static const char * GetStatus ( wsrep_status_t tStatus )
{
	if ( tStatus>=WSREP_OK && tStatus<=WSREP_NOT_IMPLEMENTED )
		return g_sStatusDesc[tStatus];
	else
		return strerror ( tStatus );
}

static void LoggerWrapper ( wsrep_log_level_t eLevel, const char * sFmt, ... )
{
	ESphLogLevel eLevelDst = SPH_LOG_INFO;
	switch ( eLevel )
	{
		case WSREP_LOG_FATAL: eLevelDst = SPH_LOG_FATAL; break;
		case WSREP_LOG_ERROR: eLevelDst = SPH_LOG_FATAL; break;
		case WSREP_LOG_WARN: eLevelDst = SPH_LOG_WARNING; break;
		case WSREP_LOG_INFO: eLevelDst = SPH_LOG_RPL_DEBUG; break;
		case WSREP_LOG_DEBUG: eLevelDst = SPH_LOG_RPL_DEBUG; break;
		default: eLevelDst = SPH_LOG_RPL_DEBUG;
	}

	va_list ap;
	va_start ( ap, sFmt );
	sphLogVa ( sFmt, ap, eLevelDst );
	va_end ( ap );
}

static void Logger_fn ( wsrep_log_level_t eLevel, const char * sMsg )
{
	LoggerWrapper ( eLevel, sMsg );
}

#define SST_OPT_ROLE "--role"
#define SST_OPT_DATA "--datadir"
#define SST_OPT_PARENT "--parent"
#define SST_OPT_STATE_IN "--state-in"
#define SST_OPT_STATE_OUT "--state-out"
#define SST_OPT_ADDR "--address"
#define SST_OPT_LISTEN "--listen"

struct SstArgs_t
{
	CSphAutoEvent m_tEv;
	CSphString m_sError;

	bool m_bBypass = false;
	CSphString m_sAddr;
	wsrep_gtid m_tGtid = WSREP_GTID_UNDEFINED;

	ReplicationCluster_t * m_pCluster = nullptr;

	explicit SstArgs_t ( ReplicationCluster_t * pCluster )
		: m_pCluster ( pCluster )
	{
	}
};

static CSphString GetSstStateFileName ( const ReplicationCluster_t * pCluster, int iSalt, bool bFileOnly )
{
	CSphString sName;
	assert ( pCluster );
	if ( iSalt )
		sName.SetSprintf ( "%s/sst_state_%d.json", pCluster->m_sPath.cstr(), iSalt );
	else if ( bFileOnly )
		sName = "sst_state.json";
	else
		sName.SetSprintf ( "%s/sst_state.json", pCluster->m_sPath.cstr() );
	return sName;
}

static bool WriteDonateState ( const ReplicationCluster_t * pCluster, const CSphString & sFilename, const wsrep_gtid & tGtid, bool bBypass )
{
	CSphString sError;

	CJsonScopedPtr_c pIndexes ( cJSON_CreateObject() );
	if ( !bBypass ) // in bypass mode informs only joiner by sending gtid
	{
		ARRAY_FOREACH ( i, pCluster->m_dIndexes )
		{
			CSphString sName = pCluster->m_dIndexes[i];
			CSphString sPath;
			eITYPE eType = eITYPE::PERCOLATE;
			// need scope for RLock to free after operations
			{
				ServedDescRPtr_c pServed ( GetServed ( pCluster->m_dIndexes[i] ) );
				if ( !pServed || !pServed->m_pIndex || !pServed->IsMutable() )
					continue;

				eType = pServed->m_eType;
				sPath = pServed->m_sIndexPath;

				// save data to disk to sync it
				((ISphRtIndex *)pServed->m_pIndex)->ForceRamFlush ( false );
			}

			JsonConfigSetIndex ( sName, sPath, eType, pIndexes.Ptr() );
		}
		if ( !cJSON_GetArraySize ( pIndexes.Ptr() ) )
		{
			sphLogFatal ( "no indexes to sync" );
			return false;
		}
	}

	CJsonScopedPtr_c pSyncSrc ( cJSON_CreateObject() );
	cJSON_AddItemToObject ( pSyncSrc.Ptr(), "indexes", pIndexes.LeakPtr() );
	
	char sGtid[WSREP_GTID_STR_LEN];
	Verify ( wsrep_gtid_print ( &tGtid, sGtid, sizeof(sGtid) )>0 );
	cJSON_AddStringToObject ( pSyncSrc.Ptr(), "gtid", sGtid );

	cJSON_AddBoolToObject ( pSyncSrc.Ptr(), "bypass", bBypass );

	CSphWriter tFile;
	if ( !tFile.OpenFile ( sFilename, sError ) )
	{
		sphLogFatal ( "%s", sError.cstr() );
		return false;
	}

	CSphString sData;
	char * sRaw = cJSON_Print ( pSyncSrc.Ptr() );
	sData.Adopt ( &sRaw );

	tFile.PutBytes ( sData.cstr(), sData.Length() );
	tFile.CloseFile();
	if ( tFile.IsError() )
	{
		sphLogFatal ( "%s", sError.cstr() );
		return false;
	}

	return true;
}

static cJSON * ReadJoinerState ( const ReplicationCluster_t * pCluster, const CSphString & sFilename, wsrep_gtid & tGtid, bool & bBypass )
{
	CSphString sError;

	if ( !sphIsReadable ( sFilename, nullptr ) )
	{
		sphLogFatal ( "no readable SST file found %s", sFilename.cstr() );
		return nullptr;
	}

	CSphAutoreader tReader;
	if ( !tReader.Open ( sFilename, sError ) )
	{
		sphLogFatal ( "%s", sError.cstr() );
		return nullptr;
	}

	int iSize = tReader.GetFilesize();
	if ( !iSize )
	{
		sphLogFatal ( "empty SST file %s", sFilename.cstr() );
		return nullptr;
	}
	
	CSphFixedVector<BYTE> dData ( iSize+1 );
	tReader.GetBytes ( dData.Begin(), iSize );

	if ( tReader.GetErrorFlag() )
	{
		sphLogFatal ( "error reading SST file %s, %s", sFilename.cstr(), tReader.GetErrorMessage().cstr() );
		return nullptr;
	}
	
	dData[iSize] = 0; // safe gap
	CJsonScopedPtr_c pRoot ( cJSON_Parse ( (const char*)dData.Begin() ) );
	if ( JsonGetError ( (const char *)dData.Begin(), dData.GetLength(), sError ) )
	{
		sphLogFatal ( "error parsing SST JSON, file %s, %s", sFilename.cstr(), sError.cstr() );
		return nullptr;
	}

	cJSON * pRes = nullptr;
	while ( true )
	{
		if ( !JsonConfigCheckProperty ( pRoot.Ptr(), "gtid", sError ) )
			break;

		cJSON * pGtid = cJSON_GetObjectItem ( pRoot.Ptr(), "gtid" );
		int iGtidLen = wsrep_gtid_scan ( pGtid->valuestring, strlen ( pGtid->valuestring ), &tGtid );
		if ( iGtidLen<0 )
		{
			sError.SetSprintf ( "invalid GTID property value, %s", pGtid->valuestring );
			break;
		}

		cJSON * pBypass = cJSON_GetObjectItem ( pRoot.Ptr(), "bypass" );
		if ( !pBypass || !cJSON_IsBool ( pBypass ) )
		{
			sError = "property 'bypass' should be a bool";
			break;
		}
		bBypass = ( !!cJSON_IsTrue ( pBypass ) );

		cJSON * pIndexes = cJSON_GetObjectItem ( pRoot.Ptr(), "indexes" );
		cJSON * pIndex = ( pIndexes && pIndexes->child ? pIndexes->child : nullptr );
		if ( !bBypass && !pIndex )
		{
			sError.SetSprintf ( "no indexes found" );
			break;
		}

		int iElem = 0;
		while ( pIndex!=nullptr )
		{
			if ( !JsonCheckIndex ( pIndex, sError ) )
			{
				// remove bad index from config
				sphWarning ( "index '%s'(%d): removed from JSON SST, %s", pIndex->string, iElem, sError.cstr() );
				cJSON * pDelete = pIndex;
				// advance to next element prior delete
				pIndex = pIndex->next;
				cJSON_DetachItemViaPointer ( pIndexes, pDelete );
				cJSON_Delete ( pDelete );
			} else
			{
				pIndex = pIndex->next;


			}
			iElem++;
		}

		if ( !bBypass && !cJSON_GetArraySize ( pIndexes ) )
		{
			sError.SetSprintf ( "no indexes found" );
			break;
		}

		cJSON_DetachItemViaPointer ( pRoot.Ptr(), pIndexes );
		pRes = pIndexes;
		break;
	}

	if ( !pRes )
		sphLogFatal (  "%s", sError.cstr() );

	return pRes;
}

static void SstDonateThread ( void * pArgs )
{
	sphLogDebugRpl ( "SST donate thread started" );
	SstArgs_t * pDonateArgs = (SstArgs_t *)pArgs;
	assert ( pDonateArgs->m_pCluster && pDonateArgs->m_pCluster->m_pProvider );

	// args copy prior caller detached and vanished
	ReplicationCluster_t * pCluster = pDonateArgs->m_pCluster;
	wsrep_t * pWsrep = pDonateArgs->m_pCluster->m_pProvider;
	bool bBypass = pDonateArgs->m_bBypass;
	CSphString sAddr = pDonateArgs->m_sAddr;
	wsrep_gtid tGtid = pDonateArgs->m_tGtid;

	sphLogDebugRpl ( "SST donate address %s", sAddr.cstr() );

	// free caller thread
	// FIXME!!! move after exec and also return exec status as 
	// signal sst_prepare thread with ret code,
	// it will go on sending SST request
	pDonateArgs->m_tEv.SetEvent();

	// uniq output name for each invocation
	CSphString sStateFilename = GetSstStateFileName ( pCluster, GetOsThreadId(), false );
	if ( !WriteDonateState ( pCluster, sStateFilename, tGtid, bBypass ) )
	{
		tGtid = WSREP_GTID_UNDEFINED;
		pWsrep->sst_sent( pWsrep, &tGtid, -ECANCELED );
		sphLogFatal ( "SST donate thread exited " );
		return;
	}
	CSphString sStateOut = GetSstStateFileName ( pCluster, 0, true );

	// setup
	CSphVector<char> dOut;
	char sError [ 1024 ];
	StringBuilder_c sCmd;
	sCmd.Appendf (
		"python wsrep_sst_pysync.py "
		SST_OPT_ROLE" donor "
		SST_OPT_ADDR" %s "
		SST_OPT_DATA" %s "
		SST_OPT_PARENT" %d "
		SST_OPT_STATE_IN" %s "
		SST_OPT_STATE_OUT" %s",
		sAddr.cstr(), pCluster->m_sPath.cstr(), GetOsThreadId(), sStateFilename.cstr(), sStateOut.cstr() );

	bool bExecPassed = TryToExec ( nullptr, nullptr, dOut, sError, sizeof(sError), sCmd.cstr() );
	if ( !bExecPassed )
	{
		sphLogFatal ( "donate failed to sync\n%s\n%s", sError, ( dOut.GetLength() ? dOut.Begin() : "" ) );
		tGtid = WSREP_GTID_UNDEFINED;
	}

	char sGtid[WSREP_GTID_STR_LEN];
	Verify ( wsrep_gtid_print ( &tGtid, sGtid, sizeof(sGtid) )>0 );
	sphLogDebugRpl ( "SST donate sent %s UID %s", ( !bExecPassed ? "invalid" : "good" ), sGtid );

	// FIXME!!! replace with proper event
	sphSleepMsec ( 100 );
	pWsrep->sst_sent( pWsrep, &tGtid, ( !bExecPassed ? -ECANCELED : 0 ) );

	// SST finished
	sphLogDebugRpl ( "SST donate thread done" );
}

static SphThread_t g_tThdDonateSst;

bool PrepareDonateSst ( ReplicationCluster_t * pCluster, bool bBypass, const CSphString & sAddr, const wsrep_gtid & tGtid, CSphString & sError )
{
	SstArgs_t tArgs ( pCluster );
	tArgs.m_bBypass = bBypass;
	tArgs.m_sAddr = sAddr;
	tArgs.m_tGtid = tGtid;
	if ( !sphThreadCreate ( &g_tThdDonateSst, SstDonateThread, &tArgs, true ) )
	{
		sphLogDebugRpl ( "failed to start SST donate thread %d (%s)", errno, strerror(errno) );
		return false;
	}

	// wait SST thread to finish setup
	tArgs.m_tEv.WaitEvent();
	if ( !tArgs.m_sError.IsEmpty() )
	{
		sphWarning ( "failed to donate SST, '%s'", tArgs.m_sError.cstr() );
		return false;
	}
		
	// just sleep to donate rsync started
	// FIXME!!! wait proper process signal
	sphSleepMsec ( 100 );
	return true;
}

static void SstJoinerThread ( void * pArgs )
{
	sphLogDebugRpl ( "SST joiner thread started" );
	SstArgs_t * pJonerArgs = (SstArgs_t *)pArgs;
	assert ( pJonerArgs->m_pCluster && pJonerArgs->m_pCluster->m_pProvider );

	// args copy prior caller detached and vanished
	ReplicationCluster_t * pCluster = pJonerArgs->m_pCluster;
	wsrep_t * pWsrep = pJonerArgs->m_pCluster->m_pProvider;

	// free caller thread
	// FIXME!!! move after exec and also return exec status as 
	// signal sst_prepare thread with ret code,
	// it will go on sending SST request
	pJonerArgs->m_tEv.SetEvent();

	// setup
	CSphString sStateFilename = GetSstStateFileName ( pCluster, 0, false );
	CSphVector<char> dOut;
	char sError [ 1024 ];
	StringBuilder_c sCmd;
	sCmd.Appendf (
		"python wsrep_sst_pysync.py "
		SST_OPT_ROLE" joiner "
		SST_OPT_DATA" %s "
		SST_OPT_PARENT" %d "
		SST_OPT_STATE_IN" %s "
		SST_OPT_LISTEN" %s",
		pCluster->m_sPath.cstr(), GetOsThreadId(), sStateFilename.cstr(), pCluster->m_sListenSST.cstr() );

	bool bExecPassed = TryToExec ( nullptr, nullptr, dOut, sError, sizeof(sError), sCmd.cstr() );
	if ( !bExecPassed )
		sphLogFatal ( "joiner failed to start sync\n%s\n%s", sError, ( dOut.GetLength() ? dOut.Begin() : "" ) );

	wsrep_gtid tGtid = WSREP_GTID_UNDEFINED;
	bool bBypass = false;
	CJsonScopedPtr_c pIndexes ( nullptr );
	if ( bExecPassed )
		pIndexes.ReplacePtr ( ReadJoinerState ( pCluster, sStateFilename, tGtid, bBypass ) );
	bool bValidState = ( bBypass || pIndexes.Ptr()!=nullptr );

	char sGtid[WSREP_GTID_STR_LEN];
	Verify ( wsrep_gtid_print ( &tGtid, sGtid, sizeof(sGtid) )>0 );

	sphLogDebugRpl ( "SST joiner got %s UID %s, indexes %d", ( !bValidState ? "invalid" : "good" ), sGtid, ( pIndexes.Ptr() ? cJSON_GetArraySize ( pIndexes.Ptr() ) : 0 ) );

	if ( bValidState )
		bValidState = ReplicatedIndexes ( pIndexes.Ptr(), bBypass, pCluster );

	if ( !bValidState )
		tGtid = WSREP_GTID_UNDEFINED;

	// just sleep to wait of exit sst_prepare_cb
	// FIXME!!! replace with proper event
	sphSleepMsec ( 100 );
	pWsrep->sst_received ( pWsrep, &tGtid, nullptr, 0, !bValidState ? -ECANCELED : 0 );

	// SST finished
	sphLogDebugRpl ( "SST joiner thread done" );
}

static SphThread_t g_tThdJoinSst;

void PrepareJoinSst ( ReplicationCluster_t * pCluster, void ** ppSstName, size_t * pSstNameLen )
{
	*ppSstName = strdup ( pCluster->m_bSkipSST ? WSREP_STATE_TRANSFER_TRIVIAL : pCluster->m_sListenSST.cstr() );

	if ( *ppSstName )
		*pSstNameLen = strlen ( (const char *)( *ppSstName ) ) + 1;
	else
		*pSstNameLen = -ENOMEM;

	sphLogDebugRpl ( "SST join '%s'", (const char *)(*ppSstName) );
	if ( pCluster->m_bSkipSST )
		return;

	while ( pSstNameLen )
	{
		SstArgs_t tArgs ( pCluster );
		if ( !sphThreadCreate ( &g_tThdJoinSst, SstJoinerThread, &tArgs, true ) )
		{
			sphLogDebugRpl ( "failed to start SST joiner thread %d (%s)", errno, strerror(errno) );
			break;
		}

		// wait SST thread to finish setup
		tArgs.m_tEv.WaitEvent();
		if ( !tArgs.m_sError.IsEmpty() )
			sphWarning ( "failed to join SST, '%s'", tArgs.m_sError.cstr() );
		
		// just sleep to joiner rsync started
		// FIXME!!! wait proper process signal
		sphSleepMsec ( 100 );

		break;
	}
}

static void LogGroupView ( const wsrep_view_info_t * pView )
{
	if ( g_eLogLevel<SPH_LOG_RPL_DEBUG )
		return;

	sphLogDebugRpl ( "new cluster membership: %d(%d), global seqno: " INT64_FMT ", status %s, gap %d",
		pView->my_idx, pView->memb_num, (int64_t)pView->state_id.seqno, g_sClusterStatus[pView->status], (int)pView->state_gap );

	StringBuilder_c sBuf;
	sBuf += "\n";
	const wsrep_member_info_t * pBoxes = pView->members;
	for ( int i=0; i<pView->memb_num; i++ )
		sBuf.Appendf ( "'%s', '%s' %s\n", pBoxes[i].name, pBoxes[i].incoming, ( i==pView->my_idx ? "*" : "" ) );

	sphLogDebugRpl ( "%s", sBuf.cstr() );
}

// This will be called on cluster view change (nodes joining, leaving, etc.).
// Each view change is the point where application may be pronounced out of
// sync with the current cluster view and need state transfer.
// It is guaranteed that no other callbacks are called concurrently with it.
static wsrep_cb_status_t ViewChanged_fn ( void * pAppCtx, void * pRecvCtx, const wsrep_view_info_t * pView, const char * pState, size_t iStateLen, void ** ppSstReq, size_t * pSstReqLen )
{
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pRecvCtx;

	LogGroupView ( pView );
	ReplicationCluster_t * pCluster = pLocalCtx->m_pCluster;
	memcpy ( pCluster->m_dUUID.Begin(), pView->state_id.uuid.data, pCluster->m_dUUID.GetLengthBytes() );
	pCluster->m_iConfID = pView->view;
	pCluster->m_iStatus = pView->status;
	pCluster->m_iSize = pView->memb_num;
	pCluster->m_iIdx = pView->my_idx;

	*ppSstReq = nullptr;
	*pSstReqLen = 0;

	if ( pView->state_gap )
		PrepareJoinSst ( pCluster, ppSstReq, pSstReqLen );

	return WSREP_CB_SUCCESS;
}

// This is called to "apply" writeset.
// If writesets don't conflict on keys, it may be called concurrently to
// utilize several CPU cores.
static wsrep_cb_status_t Apply_fn ( void * pCtx, const void * pData, size_t uSize, uint32_t uFlags, const wsrep_trx_meta_t * pMeta )
{
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pCtx;

	bool bCommit = ( ( uFlags & WSREP_FLAG_COMMIT )!=0 );
	sphLogDebugRpl ( "writeset at apply, seq " INT64_FMT ", size %d, flags %u, on %s", (int64_t)pMeta->gtid.seqno, (int)uSize, uFlags, ( bCommit ? "commit" : "rollback" ) );

	// FIXME!!! add replicate_pre_commit \ pre_commit
	if ( bCommit )
	{
		if ( !ParseCmdReplicated ( (const BYTE *)pData, uSize, pLocalCtx->m_dCommands ) )
			return WSREP_CB_FAILURE;
	} else
	{
		pLocalCtx->ResetCommands();
	}

	return WSREP_CB_SUCCESS;
}

static wsrep_cb_status_t Commit_fn ( void * pCtx, const void * hndTrx, uint32_t uFlags, const wsrep_trx_meta_t * pMeta, wsrep_bool_t * pExit, wsrep_bool_t bCommit )
{
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pCtx;
	wsrep_t * pWsrep = pLocalCtx->m_pCluster->m_pProvider;
	sphLogDebugRpl ( "writeset at %s, seq " INT64_FMT ", flags %u", ( bCommit ? "commit" : "rollback" ), (int64_t)pMeta->gtid.seqno, uFlags );

	wsrep_cb_status_t eRes = WSREP_CB_SUCCESS;
	if ( bCommit )
	{
		pWsrep->applier_pre_commit ( pWsrep, const_cast<void *>( hndTrx ) );

		bool bCmdOk = true;
		ARRAY_FOREACH_COND ( i, pLocalCtx->m_dCommands, bCmdOk )
		{
			bCmdOk &= HandleCmdReplicated ( pLocalCtx->m_dCommands[i] );
		}
		pLocalCtx->ResetCommands();

		pWsrep->applier_interim_commit ( pWsrep, const_cast<void *>( hndTrx ) );
		pWsrep->applier_post_commit ( pWsrep, const_cast<void *>( hndTrx ) );

		sphLogDebugRpl ( "seq " INT64_FMT ", committed %d", (int64_t)pMeta->gtid.seqno, (int)bCmdOk );

		eRes = ( bCmdOk ? WSREP_CB_SUCCESS : WSREP_CB_FAILURE );
	} else
	{
		// rollback
		pLocalCtx->ResetCommands();
	}

	return eRes;
}

static wsrep_cb_status_t Unordered_fn ( void * pCtx, const void * pData, size_t uSize )
{
	//ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pCtx;
	sphLogDebugRpl ( "unordered" );
	return WSREP_CB_SUCCESS;
}


static wsrep_cb_status_t SstDonate_fn ( void * pAppCtx, void * pRecvCtx, const void * sMsg, size_t iMsgLen, const wsrep_gtid_t * pStateID, const char * pState, size_t iStateLen, wsrep_bool_t bBypass )
{
	//AppCtx_t * pLocalAppCtx = (AppCtx_t *)pAppCtx;
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pRecvCtx;
	
	CSphString sAddr;
	sAddr.SetBinary ( (const char *)sMsg, iMsgLen );

	char sGtid[WSREP_GTID_STR_LEN];
	Verify ( wsrep_gtid_print ( pStateID, sGtid, sizeof(sGtid) )>0 );

	sphLogDebugRpl ( "donate to %s, gtid %s, bypass %d", sAddr.cstr(), sGtid, (int)bBypass );

	CSphString sError;
	if ( !PrepareDonateSst ( pLocalCtx->m_pCluster, bBypass, sAddr, *pStateID, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return WSREP_CB_FAILURE;
	}

	return WSREP_CB_SUCCESS;
}

static void Synced_fn ( void * pAppCtx )
{
	AppCtx_t * pLocalAppCtx = (AppCtx_t *)pAppCtx;
	sphLogDebugRpl ( "synced" );

	assert ( pLocalAppCtx && pLocalAppCtx->m_pSync );
	pLocalAppCtx->m_pSync->SetEvent();
}

// This is the listening thread. It blocks in wsrep::recv() call until
// disconnect from cluster. It will apply and commit writesets through the
// callbacks defined above.
static void ReplicationRecv_fn ( void * pCluster )
{
	ReceiverCtx_t tCtx;
	tCtx.m_pCluster = (ReplicationCluster_t *)pCluster;
	sphLogDebugRpl ( "receiver thread started" );

	wsrep_t * pWsrep = tCtx.m_pCluster->m_pProvider;
	wsrep_status_t eState = pWsrep->recv ( pWsrep, &tCtx );

	sphLogDebugRpl ( "receiver done, code %d, %s", eState, GetStatus ( eState ) );
	if ( g_tCtx.m_fnAbort && ( eState==WSREP_CONN_FAIL || eState==WSREP_NODE_FAIL || eState==WSREP_FATAL ) )
		g_tCtx.m_fnAbort();
}

static void Instr_fn ( wsrep_pfs_instr_type_t , wsrep_pfs_instr_ops_t , wsrep_pfs_instr_tag_t , void ** , void ** , const void * );

bool ReplicateClusterInit ( ReplicationArgs_t & tArgs, CSphString & sError )
{
	wsrep_t * pWsrep = nullptr;
	// let's load and initialize provider
	wsrep_status_t eRes = (wsrep_status_t)wsrep_load ( tArgs.m_sProvider, &pWsrep, Logger_fn );
	if ( eRes!=WSREP_OK )
	{
		sError.SetSprintf ( "load of provider '%s' failed, %d '%s'", tArgs.m_sProvider, (int)eRes, GetStatus ( eRes ) );
		return false;
	}
	assert ( pWsrep );
	tArgs.m_pCluster->m_pProvider = pWsrep;
	tArgs.m_pCluster->m_dUUID.Reset ( sizeof(wsrep_uuid_t) );
	memcpy ( tArgs.m_pCluster->m_dUUID.Begin(), WSREP_UUID_UNDEFINED.data, tArgs.m_pCluster->m_dUUID.GetLengthBytes() );

	wsrep_gtid_t tStateID = { WSREP_UUID_UNDEFINED, WSREP_SEQNO_UNDEFINED };
	CSphString sMyName;
	sMyName.SetSprintf ( "daemon_%d", GetOsThreadId() );

	sphLogDebugRpl ( "listen IP '%s', SST '%s', '%s'", tArgs.m_pCluster->m_sListen.cstr(), tArgs.m_pCluster->m_sListenSST.cstr(), sMyName.cstr() );

	// wsrep provider initialization arguments
	wsrep_init_args wsrep_args;
	wsrep_args.app_ctx       = &g_tCtx,

	wsrep_args.node_name     = sMyName.cstr();
	wsrep_args.node_address  = tArgs.m_pCluster->m_sListen.cstr();
	wsrep_args.node_incoming = tArgs.m_sIncomingAdresses; // must to be set otherwise node works as GARB - does not affect FC and might hung 
	wsrep_args.data_dir      = tArgs.m_pCluster->m_sPath.cstr(); // working directory
	wsrep_args.options       = tArgs.m_pCluster->m_sOptions.scstr();
	wsrep_args.proto_ver     = 127; // maximum supported application event protocol

	wsrep_args.state_id      = &tStateID;
	wsrep_args.state         = "";
	wsrep_args.state_len     = 0;

	wsrep_args.view_handler_cb = ViewChanged_fn;
	wsrep_args.synced_cb      = Synced_fn;
	wsrep_args.sst_donate_cb  = SstDonate_fn;
	wsrep_args.apply_cb       = Apply_fn;
	wsrep_args.commit_cb      = Commit_fn;
	wsrep_args.unordered_cb   = Unordered_fn;
	wsrep_args.logger_cb      = Logger_fn;

	wsrep_args.abort_cb = g_tCtx.m_fnAbort;
	wsrep_args.pfs_instr_cb = Instr_fn;

	eRes = pWsrep->init ( pWsrep, &wsrep_args );
	if ( eRes!=WSREP_OK )
	{
		sError.SetSprintf ( "replication init failed: %d '%s'", (int)eRes, GetStatus ( eRes ) );
		return false;
	}

	// Connect to cluster
	eRes = pWsrep->connect ( pWsrep, tArgs.m_pCluster->m_sName.cstr(), tArgs.m_pCluster->m_sURI.cstr(), "", tArgs.m_bNewCluster );
	if ( eRes!=WSREP_OK )
	{
		sError.SetSprintf ( "replication connect failed: %d '%s'", (int)eRes, GetStatus ( eRes ) );
		// might try to join existed cluster however that took too long to timeout in case no cluster started
		return false;
	}

	// let's start listening thread with proper provider set
	if ( !sphThreadCreate ( &tArgs.m_pCluster->m_tRecvThd, ReplicationRecv_fn, tArgs.m_pCluster, false ) )
	{
		sError.SetSprintf ( "failed to start thread %d (%s)", errno, strerror(errno) );
		return false;
	}

	sphLogDebugRpl ( "replicator created for cluster '%s'", tArgs.m_pCluster->m_sName.cstr() );

	return true;
}

void ReplicateClusterDone ( ReplicationCluster_t * pCluster )
{
	if ( !pCluster )
		return;

	assert ( pCluster->m_pProvider );

	pCluster->m_pProvider->disconnect ( pCluster->m_pProvider );

	// Listening thread are now running and receiving writesets. Wait for them
	// to join. Thread will join after signal handler closes wsrep connection
	sphThreadJoin ( &pCluster->m_tRecvThd );

	// FIXME!!! unload provider ONLY after nobody uses it any more
	wsrep_unload ( pCluster->m_pProvider );
	pCluster->m_pProvider = nullptr;
}

// FIXME!!! handle more status codes properly
static bool CheckResult ( wsrep_status_t tRes, const wsrep_trx_meta_t & tMeta, const char * sAt, CSphString & sError )
{
	if ( tRes==WSREP_OK )
		return true;

	sError.SetSprintf ( "error at %s, code %d, seqno " INT64_FMT, sAt, tRes, tMeta.gtid.seqno );
	sphWarning ( "%s", sError.cstr() );
	return false;
}

bool Replicate ( uint64_t uQueryHash, const CSphVector<BYTE> & dBuf, wsrep_t * pProvider, CommitMonitor_c & tMonitor, CSphString & sError )
{
	assert ( pProvider );

	int iConnId = GetOsThreadId();

	wsrep_trx_meta_t tLogMeta;
	tLogMeta.gtid = WSREP_GTID_UNDEFINED;

	wsrep_buf_t tKeyPart;
	tKeyPart.ptr = &uQueryHash;
	tKeyPart.len = sizeof(uQueryHash);

	wsrep_key_t tKey;
	tKey.key_parts = &tKeyPart;
	tKey.key_parts_num = 1;

	wsrep_buf_t tQuery;
	tQuery.ptr = dBuf.Begin();
	tQuery.len = dBuf.GetLength();

	wsrep_ws_handle_t tHnd;
	tHnd.trx_id = UINT64_MAX;
	tHnd.opaque = nullptr;

	wsrep_status_t tRes = pProvider->append_key ( pProvider, &tHnd, &tKey, 1, WSREP_KEY_EXCLUSIVE, false );
	bool bOk = CheckResult ( tRes, tLogMeta, "append_key", sError );

	if ( bOk )
	{
		tRes = pProvider->append_data ( pProvider, &tHnd, &tQuery, 1, WSREP_DATA_ORDERED, false );
		bOk = CheckResult ( tRes, tLogMeta, "append_data", sError );
	}

	if ( bOk )
	{
		tRes = pProvider->replicate ( pProvider, iConnId, &tHnd, WSREP_FLAG_COMMIT, &tLogMeta );
		bOk = CheckResult ( tRes, tLogMeta, "replicate", sError );
	}

	sphLogDebugRpl ( "replicating %d, seq " INT64_FMT ", hash " UINT64_FMT, (int)bOk, (int64_t)tLogMeta.gtid.seqno, uQueryHash );

	if ( bOk )
	{
		tRes = pProvider->pre_commit ( pProvider, iConnId, &tHnd, WSREP_FLAG_COMMIT, &tLogMeta );
		bOk = CheckResult ( tRes, tLogMeta, "pre_commit", sError );

		// in case only commit failed
		// need to abort running transaction prior to rollback
		if ( bOk  && !tMonitor.Commit( sError ) )
		{
			pProvider->abort_pre_commit ( pProvider, WSREP_SEQNO_UNDEFINED, tHnd.trx_id );
			bOk = false;
		}

		if ( bOk )
		{
			tRes = pProvider->interim_commit( pProvider, &tHnd );
			CheckResult ( tRes, tLogMeta, "interim_commit", sError );

			tRes = pProvider->post_commit ( pProvider, &tHnd );
			CheckResult ( tRes, tLogMeta, "post_commit", sError );
		} else
		{
			tRes = pProvider->post_rollback ( pProvider, &tHnd );
			CheckResult ( tRes, tLogMeta, "post_rollback", sError );
		}

		sphLogDebugRpl ( "%s seq " INT64_FMT ", hash " UINT64_FMT, ( bOk ? "committed" : "rolled-back" ), (int64_t)tLogMeta.gtid.seqno, uQueryHash );
	}

	// FIXME!!! move ThreadID and to net loop handler exit
	pProvider->free_connection ( pProvider, iConnId );

	return bOk;
}

void ReplicationInit ( Abort_fn fnAbort, CSphAutoEvent * pSync )
{
	g_tCtx.m_fnAbort = fnAbort;
	g_tCtx.m_pSync = pSync;
}

void ReplicateClusterStats ( ReplicationCluster_t * pCluster, VectorLike & dOut )
{
	if ( !pCluster || !pCluster->m_pProvider )
		return;

	assert ( pCluster->m_iStatus>=0 && pCluster->m_iStatus<WSREP_VIEW_MAX );

	// cluster vars
	if ( dOut.MatchAdd ( "cluster_name" ) )
		dOut.Add( pCluster->m_sName );
	if ( dOut.MatchAdd ( "cluster_state_uuid" ) )
	{
		wsrep_uuid_t tUUID;
		memcpy ( tUUID.data, pCluster->m_dUUID.Begin(), pCluster->m_dUUID.GetLengthBytes() );
		char sUUID[WSREP_UUID_STR_LEN+1] = { '\0' };
		wsrep_uuid_print ( &tUUID, sUUID, sizeof(sUUID) );
		dOut.Add() = sUUID;
	}
	if ( dOut.MatchAdd ( "cluster_conf_id" ) )
		dOut.Add().SetSprintf ( "%d", pCluster->m_iConfID );
	if ( dOut.MatchAdd ( "cluster_status" ) )
		dOut.Add() = g_sClusterStatus[pCluster->m_iStatus];
	if ( dOut.MatchAdd ( "cluster_size" ) )
		dOut.Add().SetSprintf ( "%d", pCluster->m_iSize );
	if ( dOut.MatchAdd ( "cluster_local_index" ) )
		dOut.Add().SetSprintf ( "%d", pCluster->m_iIdx );

	// cluster status
	wsrep_stats_var * pVarsStart = pCluster->m_pProvider->stats_get ( pCluster->m_pProvider );
	if ( !pVarsStart )
		return;

	wsrep_stats_var * pVars = pVarsStart;
	while ( pVars->name )
	{
		if ( dOut.MatchAdd ( pVars->name ) )
		{
			switch ( pVars->type )
			{
			case WSREP_VAR_STRING:
				dOut.Add ( pVars->value._string );
				break;

			case WSREP_VAR_DOUBLE:
				dOut.Add().SetSprintf ( "%f", pVars->value._double );
				break;

			default:
			case WSREP_VAR_INT64:
				dOut.Add().SetSprintf ( INT64_FMT, pVars->value.i64 );
				break;
			}
		}

		pVars++;
	}

	pCluster->m_pProvider->stats_free ( pCluster->m_pProvider, pVarsStart );
}

CSphString ReplicateClusterOptions ( const char * sOptionsRaw, bool bLogReplication )
{
	CSphString sOptions ( sOptionsRaw );
	if ( bLogReplication )
	{
		if ( sOptions.IsEmpty() )
			sOptions = "debug=on;cert.log_conflicts=yes";
		else
			sOptions.SetSprintf ( "%s;debug=on;cert.log_conflicts=yes", sOptions.cstr() );
	}
	return sOptions;
}

bool ReplicateSetOption ( ReplicationCluster_t * pCluster, const CSphString & sOpt )
{
	if ( !pCluster || !pCluster->m_pProvider || sOpt.IsEmpty() )
		return false;

	wsrep_status_t tOk = pCluster->m_pProvider->options_set ( pCluster->m_pProvider, sOpt.cstr() );
	return ( tOk==WSREP_OK );
}

// @brief a callback to create PFS instrumented mutex/condition variables
// 
// @param type          mutex or condition variable
// @param ops           add/init or remove/destory mutex/condition variable
// @param tag           tag/name of instrument to monitor
// @param value         created mutex or condition variable
// @param alliedvalue   allied value for supporting operation.
//                      for example: while waiting for cond-var corresponding
//                      mutex is passes through this variable.
// @param ts      time to wait for condition.

void Instr_fn ( wsrep_pfs_instr_type_t type, wsrep_pfs_instr_ops_t ops, wsrep_pfs_instr_tag_t tag, void ** value, void ** alliedvalue, const void * ts )
{
	if ( type==WSREP_PFS_INSTR_TYPE_THREAD || type==WSREP_PFS_INSTR_TYPE_FILE )
		return;

#if !USE_WINDOWS
	if (type == WSREP_PFS_INSTR_TYPE_MUTEX)
	{
		switch (ops)
		{
			case WSREP_PFS_INSTR_OPS_INIT:
			{
				pthread_mutex_t * pMutex = new pthread_mutex_t;
				pthread_mutex_init ( pMutex, nullptr );
				*value = pMutex;
			}
			break;

		case WSREP_PFS_INSTR_OPS_DESTROY:
		{
			pthread_mutex_t * pMutex= (pthread_mutex_t *)(*value);
			assert ( pMutex );
			pthread_mutex_destroy ( pMutex );
			delete ( pMutex );
			*value = nullptr;
		}
		break;

		case WSREP_PFS_INSTR_OPS_LOCK:
		{
			pthread_mutex_t * pMutex= (pthread_mutex_t *)(*value);
			assert ( pMutex );
			pthread_mutex_lock ( pMutex );
		}
		break;

		case WSREP_PFS_INSTR_OPS_UNLOCK:
		{
			pthread_mutex_t * pMutex= (pthread_mutex_t *)(*value);
			assert ( pMutex );
			pthread_mutex_unlock ( pMutex );
		}
		break;

		default:
			assert(0);
		break;
		}
	}
	else if (type == WSREP_PFS_INSTR_TYPE_CONDVAR)
	{
		switch (ops)
		{
		case WSREP_PFS_INSTR_OPS_INIT:
		{
			pthread_cond_t * pCond = new pthread_cond_t;
			pthread_cond_init ( pCond, nullptr );
			*value = pCond;
		}
		break;

		case WSREP_PFS_INSTR_OPS_DESTROY:
		{
			pthread_cond_t * pCond = (pthread_cond_t *)(*value);
			assert ( pCond );
			pthread_cond_destroy ( pCond );
			delete ( pCond );
			*value = nullptr;
		}
		break;

		case WSREP_PFS_INSTR_OPS_WAIT:
		{
			pthread_cond_t * pCond = (pthread_cond_t *)(*value);
			pthread_mutex_t * pMutex= (pthread_mutex_t *)(*alliedvalue);
			assert ( pCond && pMutex );
			pthread_cond_wait ( pCond, pMutex );
		}
		break;

		case WSREP_PFS_INSTR_OPS_TIMEDWAIT:
		{
			pthread_cond_t * pCond = (pthread_cond_t *)(*value);
			pthread_mutex_t * pMutex= (pthread_mutex_t *)(*alliedvalue);
			const timespec * wtime = (const timespec *)ts;
			assert ( pCond && pMutex );
			pthread_cond_timedwait( pCond, pMutex, wtime );
		}
		break;

		case WSREP_PFS_INSTR_OPS_SIGNAL:
		{
			pthread_cond_t * pCond = (pthread_cond_t *)(*value);
			assert ( pCond );
			pthread_cond_signal ( pCond );
		}
		break;

		case WSREP_PFS_INSTR_OPS_BROADCAST:
		{
			pthread_cond_t * pCond = (pthread_cond_t *)(*value);
			assert ( pCond );
			pthread_cond_broadcast ( pCond );
		}
		break;

		default:
			assert(0);
		break;
		}
	}
#endif
}

