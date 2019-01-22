
#include "sphinx.h"
#include "sphinxstd.h"
#include "sphinxutils.h"
#include "sphinxint.h"
#include "sphinxpq.h"
#include "searchdreplication.h"
#include "json/cJSON.h"
#include "replication/wsrep_api.h"

#define USE_PSI_INTERFACE 1

// global application context for wsrep callbacks
static CSphString	g_sLogFile;

static CSphString	g_sJsonConfigName;
static cJSON *		g_pCfgJson = nullptr;
static bool			g_bSkipCfgJson = false;
static bool			g_bCfgJsonValid = false;

static bool JsonConfigWrite ( const cJSON * pConf, CSphString & sError );
static void JsonConfigSetLocal ( cJSON * pConf );
static void JsonConfigDefault ( cJSON ** ppConf );
static bool JsonConfigRead ( cJSON ** ppConf, CSphString & sError );
static void JsonConfigSetIndex ( const CSphString & sIndexName, const CSphString & sPath, eITYPE eIndexType, cJSON * pIndexes );
static bool JsonGetError ( const char * sBuf, int iBufLen, CSphString & sError );
static bool JsonConfigCheckProperty ( const cJSON * pNode, const char * sPropertyName, CSphString & sError );
static bool JsonCheckIndex ( const cJSON * pIndex, CSphString & sError );

struct ReplicationCluster_t
{
	CSphString	m_sName;
	CSphString	m_sURI;
	CSphString	m_sListen;
	// +1 after listen is IST port
	// +1 after listen IST is SST port
	CSphString	m_sListenSST; 
	CSphString	m_sPath;
	CSphString	m_sOptions;
	bool		m_bSkipSST = false;
	CSphVector<CSphString> m_dIndexes;

	// replicator
	wsrep_t *	m_pProvider = nullptr;
	SphThread_t	m_tRecvThd;
	CSphAutoEvent m_tSync;

	// state
	CSphFixedVector<BYTE>	m_dUUID { 0 };
	int						m_iConfID = 0;
	int						m_iStatus = 0;
	int						m_iSize = 0;
	int						m_iIdx = 0;
};

struct ReplicationArgs_t
{
	ReplicationCluster_t *	m_pCluster = nullptr;
	bool					m_bNewCluster = false;
	bool					m_bJoin = false;
	const char *			m_sIncomingAdresses = nullptr;
};

class CommitMonitor_c
{
public:
	CommitMonitor_c ( ReplicationCommand_t & tCmd, int * pDeletedCount )
		: m_tCmd ( tCmd )
		, m_pDeletedCount ( pDeletedCount )
	{}
	~CommitMonitor_c() = default;

	bool Commit ( CSphString & sError );

private:
	ReplicationCommand_t & m_tCmd;
	int * m_pDeletedCount = nullptr;
};

static CSphVector<ReplicationCluster_t *> g_dClusters;
static SmallStringHash_T<ReplicationCluster_t *> g_hIndex2Cluster;
static CSphString g_sIncomingAddr;
static RwLock_t g_tClustersLock;

static void ReplicationAbort();

const char * GetReplicationDL()
{
#ifndef REPLICATION_LIB
	return nullptr;
#else
	return REPLICATION_LIB;
#endif
}

static bool ReplicatedIndexes ( const cJSON * pIndexes, bool bBypass, ReplicationCluster_t * pCluster );
static bool ParseCmdReplicated ( const BYTE * pData, int iLen, CSphVector<ReplicationCommand_t> & dCommands );
static bool HandleCmdReplicated ( ReplicationCommand_t & tCmd );

// receiving thread context for wsrep callbacks
struct ReceiverCtx_t : ISphRefcounted
{
	ReplicationCluster_t *	m_pCluster = nullptr;
	bool					m_bJoin = false;
	CSphAutoEvent			m_tStarted;

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


class CJsonScopedPtr_c : public CSphScopedPtr<cJSON>
{
public:
	CJsonScopedPtr_c ( cJSON * pPtr  )
		: CSphScopedPtr<cJSON> ( pPtr )
	{}

	~CJsonScopedPtr_c()
	{
		cJSON_Delete(m_pPtr);
		m_pPtr = nullptr;
	}
};


#define SST_OPT_ROLE "--role"
#define SST_OPT_DATA "--datadir"
#define SST_OPT_PARENT "--parent"
#define SST_OPT_NAME "--cluster-name"
#define SST_OPT_STATE_IN "--state-in"
#define SST_OPT_STATE_OUT "--state-out"
#define SST_OPT_ADDR "--address"
#define SST_OPT_LISTEN "--listen"
#define SST_OPT_LOG "--log-file"

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
		SST_OPT_NAME" %s "
		SST_OPT_STATE_IN" %s "
		SST_OPT_STATE_OUT" %s",
		sAddr.cstr(), pCluster->m_sPath.cstr(), GetOsThreadId(), pCluster->m_sName.cstr(), sStateFilename.cstr(), sStateOut.cstr() );

	// SST script logs info into daemon log
	if ( g_eLogLevel>=SPH_LOG_RPL_DEBUG && !g_sLogFile.IsEmpty() )
		sCmd.Appendf ( " " SST_OPT_LOG" %s", g_sLogFile.cstr() );

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
		SST_OPT_NAME" %s "
		SST_OPT_STATE_IN" %s "
		SST_OPT_LISTEN" %s",
		pCluster->m_sPath.cstr(), GetOsThreadId(), pCluster->m_sName.cstr(), sStateFilename.cstr(), pCluster->m_sListenSST.cstr() );

	// SST script logs info into daemon log
	if ( g_eLogLevel>=SPH_LOG_RPL_DEBUG && !g_sLogFile.IsEmpty() )
		sCmd.Appendf ( " " SST_OPT_LOG" %s", g_sLogFile.cstr() );

	bool bExecPassed = TryToExec ( nullptr, nullptr, dOut, sError, sizeof(sError), sCmd.cstr() );
	if ( !bExecPassed )
		sphLogFatal ( "joiner failed to start sync\n%s\n%s", sError, ( dOut.GetLength() ? dOut.Begin() : "" ) );

	wsrep_gtid tGtid = WSREP_GTID_UNDEFINED;
	bool bBypass = false;
	CJsonScopedPtr_c pIndexes ( bExecPassed ? ReadJoinerState ( pCluster, sStateFilename, tGtid, bBypass ) : nullptr );
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
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pAppCtx;
	sphLogDebugRpl ( "synced" );

	assert ( pLocalCtx );
	pLocalCtx->m_pCluster->m_tSync.SetEvent();
}

// This is the listening thread. It blocks in wsrep::recv() call until
// disconnect from cluster. It will apply and commit writesets through the
// callbacks defined above.
static void ReplicationRecv_fn ( void * pArgs )
{
	ReceiverCtx_t * pCtx = (ReceiverCtx_t *)pArgs;
	assert ( pCtx );

	// grab ownership and release master thread
	pCtx->AddRef();
	pCtx->m_tStarted.SetEvent();
	sphLogDebugRpl ( "receiver thread started" );

	wsrep_t * pWsrep = pCtx->m_pCluster->m_pProvider;
	wsrep_status_t eState = pWsrep->recv ( pWsrep, pCtx );

	sphLogDebugRpl ( "receiver done, code %d, %s", eState, GetStatus ( eState ) );

	// FIXME!!! release of main thread waiting sync
	pCtx->m_pCluster->m_tSync.SetEvent();

	// join thread shut with event set but start up sequence should shutdown daemon
	if ( !pCtx->m_bJoin && ( eState==WSREP_CONN_FAIL || eState==WSREP_NODE_FAIL || eState==WSREP_FATAL ) )
	{
		pCtx->Release();
		ReplicationAbort();
		return;
	}

	pCtx->Release();
}

static void Instr_fn ( wsrep_pfs_instr_type_t , wsrep_pfs_instr_ops_t , wsrep_pfs_instr_tag_t , void ** , void ** , const void * );

bool ReplicateClusterInit ( ReplicationArgs_t & tArgs, CSphString & sError )
{
	wsrep_t * pWsrep = nullptr;
	// let's load and initialize provider
	wsrep_status_t eRes = (wsrep_status_t)wsrep_load ( GetReplicationDL(), &pWsrep, Logger_fn );
	if ( eRes!=WSREP_OK )
	{
		sError.SetSprintf ( "load of provider '%s' failed, %d '%s'", GetReplicationDL(), (int)eRes, GetStatus ( eRes ) );
		return false;
	}
	assert ( pWsrep );
	tArgs.m_pCluster->m_pProvider = pWsrep;
	tArgs.m_pCluster->m_dUUID.Reset ( sizeof(wsrep_uuid_t) );
	memcpy ( tArgs.m_pCluster->m_dUUID.Begin(), WSREP_UUID_UNDEFINED.data, tArgs.m_pCluster->m_dUUID.GetLengthBytes() );

	wsrep_gtid_t tStateID = { WSREP_UUID_UNDEFINED, WSREP_SEQNO_UNDEFINED };
	CSphString sMyName;
	sMyName.SetSprintf ( "daemon_%d_%s", GetOsThreadId(), tArgs.m_pCluster->m_sName.cstr() );

	sphLogDebugRpl ( "listen IP '%s', SST '%s', '%s'", tArgs.m_pCluster->m_sListen.cstr(), tArgs.m_pCluster->m_sListenSST.cstr(), sMyName.cstr() );

	ReceiverCtx_t * pRecvArgs = new ReceiverCtx_t();
	pRecvArgs->m_pCluster = tArgs.m_pCluster;
	pRecvArgs->m_bJoin = tArgs.m_bJoin;

	// wsrep provider initialization arguments
	wsrep_init_args wsrep_args;
	wsrep_args.app_ctx       = pRecvArgs,

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

	wsrep_args.abort_cb = ReplicationAbort;
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
	if ( !sphThreadCreate ( &tArgs.m_pCluster->m_tRecvThd, ReplicationRecv_fn, pRecvArgs, false ) )
	{
		pRecvArgs->Release();
		sError.SetSprintf ( "failed to start thread %d (%s)", errno, strerror(errno) );
		return false;
	}

	// need to transfer ownership after thread started
	// freed at recv thread normaly
	pRecvArgs->m_tStarted.WaitEvent();
	pRecvArgs->Release();

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

void ReplicationInit ( const CSphString & sLogFile )
{
	g_sLogFile = sLogFile;
}

static void ReplicateClusterStats ( ReplicationCluster_t * pCluster, VectorLike & dOut )
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

bool ReplicateSetOption ( const CSphString & sCluster, const CSphString & sOpt, CSphString & sError )
{
	CSphScopedRLock tClusterGuard ( g_tClustersLock );		
	int iCluster = g_dClusters.GetFirst ( [&] ( const ReplicationCluster_t * tVal ) { return sCluster==tVal->m_sName; } );
	if ( iCluster==-1 )
	{
		sError.SetSprintf ( "unknown cluster '%s' in SET statement", sCluster.cstr() );
		return false;
	}

	ReplicationCluster_t * pCluster = g_dClusters[iCluster];
	if ( !pCluster || !pCluster->m_pProvider )
	{
		sError = "SET to bad cluster";
		return false;
	}

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


void ReplicationAbort()
{
	sphWarning ( "shutting down due to abort" );
	// no need to shutdown cluster properly in case of cluster signaled abort
	g_dClusters.Reset();
	Shutdown();
}

// unfreeze threads waiting of replication started
void ReplicateClustersWake()
{
	ARRAY_FOREACH ( iCluster, g_dClusters )
		g_dClusters[iCluster]->m_tSync.SetEvent();
}

void ReplicateClustersDelete()
{
	ARRAY_FOREACH ( iCluster, g_dClusters )
	{
		ReplicateClusterDone ( g_dClusters[iCluster] );
		SafeDelete ( g_dClusters[iCluster] );
	}
}

void JsonSkipConfig()
{
	g_bSkipCfgJson = true;
}

void JsonLoadConfig ( const CSphString & sConfigName )
{
	if ( g_bSkipCfgJson )
		return;

	g_sJsonConfigName.SetSprintf ( "%s.json", sConfigName.cstr() );
	CSphString sError;
	JsonConfigDefault ( &g_pCfgJson );
	if ( !JsonConfigRead ( &g_pCfgJson, sError ) )
		sphDie ( "failed to use JSON config, %s", sError.cstr() );
}

void JsonSaveConfig()
{
	CSphString sError;
	if ( !g_bSkipCfgJson && g_bCfgJsonValid )
	{
		JsonConfigSetLocal ( g_pCfgJson );
		if ( !JsonConfigWrite ( g_pCfgJson, sError ) )
			sphLogFatal ( "%s", sError.cstr() );
	}
	if ( g_pCfgJson )
	{
		cJSON_Delete( g_pCfgJson );
		g_pCfgJson = nullptr;
	}
}

void JsonConfigConfigureAndPreload ( int & iValidIndexes, int & iCounter )
{
	cJSON * pIndexes = cJSON_GetObjectItem ( g_pCfgJson, "indexes" );
	cJSON * pIndex = ( pIndexes ? pIndexes->child : nullptr );
	while ( pIndex!=nullptr )
	{
		const char * sIndexName = pIndex->string;
		CSphVariant tPath ( cJSON_GetObjectItem( pIndex, "path" )->valuestring, 0 );
		CSphVariant tType ( cJSON_GetObjectItem( pIndex, "type" )->valuestring, 0 );

		CSphConfigSection hIndex;
		hIndex.Add ( tPath, "path" );
		hIndex.Add ( tType, "type" );

		ESphAddIndex eAdd = ConfigureAndPreload ( hIndex, sIndexName, true );
		iValidIndexes += ( eAdd!=ADD_ERROR ? 1 : 0 );
		iCounter +=  ( eAdd==ADD_DSBLED ? 1 : 0 );
		if ( eAdd==ADD_ERROR )
		{
			// remove bad index from config
			sphWarning ( "index '%s': removed from JSON config", sIndexName );
			cJSON * pDelete = pIndex;
			// advance to next element prior delete
			pIndex = pIndex->next;
			cJSON_DetachItemViaPointer ( pIndexes, pDelete );
			cJSON_Delete ( pDelete );
		} else
		{
			pIndex = pIndex->next;
		}
	}
	g_bCfgJsonValid = !g_bSkipCfgJson;
}

void ReplicateClustersStatus ( VectorLike & dStatus )
{
	g_tClustersLock.ReadLock();
	ARRAY_FOREACH ( i, g_dClusters )
	{
		ReplicationCluster_t * pCluster = g_dClusters[i];
		ReplicateClusterStats ( pCluster, dStatus );
	}
	g_tClustersLock.Unlock();
}


static WORD g_iReplicateCommandVer[RCOMMAND_TOTAL] = { 0x101, 0x101, 0x101, 0x101 };

WORD GetReplicateCommandVer ( ReplicationCommand_e eCommand )
{
	assert ( eCommand>=0 && eCommand<RCOMMAND_TOTAL );
	return g_iReplicateCommandVer[(int)eCommand];
}

bool ParseCmdReplicated ( const BYTE * pData, int iLen, CSphVector<ReplicationCommand_t> & dCommands )
{
	MemoryReader_c tReader ( pData, iLen );

	ReplicationCommand_e eCommand = (ReplicationCommand_e)tReader.GetWord();
	if ( eCommand<0 || eCommand>RCOMMAND_TOTAL )
	{
		sphWarning ( "replication bad command %d", (int)eCommand );
		return false;
	}

	WORD uVer = tReader.GetWord();
	if ( uVer!=GetReplicateCommandVer ( eCommand ) )
	{
		sphWarning ( "replication command %d, version mismatch %d, got %d", (int)eCommand, (int)GetReplicateCommandVer ( eCommand ), (int)uVer );
		return false;
	}

	CSphString sIndex = tReader.GetString();
	ServedIndexRefPtr_c pServed = GetServed ( sIndex );
	if ( !pServed )
	{
		sphWarning ( "replicate unknown index '%s', command %d", sIndex.cstr(), (int)eCommand );
		return false;
	}

	ServedDescRPtr_c pDesc ( pServed );
	if ( pDesc->m_eType!=eITYPE::PERCOLATE )
	{
		sphWarning ( "replicate wrong type of index '%s', command %d", sIndex.cstr(), (int)eCommand );
		return false;
	}

	PercolateIndex_i * pIndex = (PercolateIndex_i * )pDesc->m_pIndex;
	const BYTE * pRequest = pData + tReader.GetPos();
	int iRequestLen = iLen - tReader.GetPos();

	ReplicationCommand_t & tCmd = dCommands.Add();
	tCmd.m_eCommand = eCommand;
	tCmd.m_sIndex = sIndex;

	switch ( eCommand )
	{
	case RCOMMAND_PQUERY_ADD:
	{
		LoadStoredQuery ( pRequest, iRequestLen, tCmd.m_tPQ );
		sphLogDebugRpl ( "pq-add, index '%s', uid " UINT64_FMT " query %s", tCmd.m_sIndex.cstr(), tCmd.m_tPQ.m_uQUID, tCmd.m_tPQ.m_sQuery.cstr() );

		CSphString sError;
		PercolateQueryArgs_t tArgs ( tCmd.m_tPQ );
		tArgs.m_bReplace = true;
		tCmd.m_pStored = pIndex->Query ( tArgs, sError );

		if ( !tCmd.m_pStored )
		{
			sphWarning ( "replicate pq-add error '%s', index '%s'", sError.cstr(), tCmd.m_sIndex.cstr() );
			dCommands.Pop();
			return false;
		}
	}
	break;

	case RCOMMAND_DELETE:
		LoadDeleteQuery ( pRequest, iRequestLen, tCmd.m_dDeleteQueries, tCmd.m_sDeleteTags );
		sphLogDebugRpl ( "pq-delete, index '%s', queries %d, tags %s", tCmd.m_sIndex.cstr(), tCmd.m_dDeleteQueries.GetLength(), tCmd.m_sDeleteTags.cstr() );
		break;

	case RCOMMAND_TRUNCATE:
		sphLogDebugRpl ( "pq-truncate, index '%s'", tCmd.m_sIndex.cstr() );
		break;

	default:
		sphWarning ( "replicate unsupported command %d", (int)eCommand );
		return false;
	}

	return true;
}

bool HandleCmdReplicated ( ReplicationCommand_t & tCmd )
{
	ServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sIndex );
	if ( !pServed )
	{
		sphWarning ( "replicate unknown index '%s', command %d", tCmd.m_sIndex.cstr(), (int)tCmd.m_eCommand );
		return false;
	}

	ServedDescPtr_c pDesc ( pServed, ( tCmd.m_eCommand==RCOMMAND_TRUNCATE ) );
	if ( pDesc->m_eType!=eITYPE::PERCOLATE )
	{
		sphWarning ( "replicate wrong type of index '%s', command %d", tCmd.m_sIndex.cstr(), (int)tCmd.m_eCommand );
		return false;
	}

	PercolateIndex_i * pIndex = (PercolateIndex_i * )pDesc->m_pIndex;
	CSphString sError;

	switch ( tCmd.m_eCommand )
	{
	case RCOMMAND_PQUERY_ADD:
	{
		sphLogDebugRpl ( "pq-add-commit, index '%s', uid " INT64_FMT, tCmd.m_sIndex.cstr(), tCmd.m_tPQ.m_uQUID );

		bool bOk = pIndex->Commit ( tCmd.m_pStored, sError );
		tCmd.m_pStored = nullptr;
		if ( !bOk )
		{
			sphWarning ( "replicate commit error '%s', index '%s'", sError.cstr(), tCmd.m_sIndex.cstr() );
			return false;
		}
	}
	break;

	case RCOMMAND_DELETE:
		sphLogDebugRpl ( "pq-delete-commit, index '%s'", tCmd.m_sIndex.cstr() );

		if ( tCmd.m_dDeleteQueries.GetLength() )
			pIndex->DeleteQueries ( tCmd.m_dDeleteQueries.Begin(), tCmd.m_dDeleteQueries.GetLength() );
		else
			pIndex->DeleteQueries ( tCmd.m_sDeleteTags.cstr() );
		break;

	case RCOMMAND_TRUNCATE:
		sphLogDebugRpl ( "pq-truncate-commit, index '%s'", tCmd.m_sIndex.cstr() );
		pIndex->Truncate ( sError );
		break;

	default:
		sphWarning ( "replicate unsupported command %d", tCmd.m_eCommand );
		return false;
	}

	return true;
}

bool HandleCmdReplicate ( ReplicationCommand_t & tCmd, CSphString & sError, int * pDeletedCount )
{
	assert ( tCmd.m_eCommand>=0 && tCmd.m_eCommand<RCOMMAND_TOTAL );
	CommitMonitor_c tMonitor ( tCmd, pDeletedCount );

	g_tClustersLock.ReadLock();
	ReplicationCluster_t ** ppCluster = g_hIndex2Cluster ( tCmd.m_sIndex );
	g_tClustersLock.Unlock();
	if ( !ppCluster )
		return tMonitor.Commit ( sError );

	if ( tCmd.m_eCommand==RCOMMAND_TRUNCATE && tCmd.m_bReconfigure )
	{
		sError.SetSprintf ( "RECONFIGURE not supported in cluster index" );
		return false;
	}

	assert ( (*ppCluster)->m_pProvider );

	// replicator check CRC of data - no need to check that at our side
	CSphVector<BYTE> dBuf;
	uint64_t uQueryHash = sphFNV64 ( tCmd.m_sIndex.cstr() );
	uQueryHash = sphFNV64 ( &tCmd.m_eCommand, sizeof(tCmd.m_eCommand), uQueryHash );

	// scope for writer
	{
		MemoryWriter_c tWriter ( dBuf );
		tWriter.PutWord ( tCmd.m_eCommand );
		tWriter.PutWord ( GetReplicateCommandVer ( tCmd.m_eCommand ) );
		tWriter.PutString ( tCmd.m_sIndex );
	}

	switch ( tCmd.m_eCommand )
	{
	case RCOMMAND_PQUERY_ADD:
		assert ( tCmd.m_pStored );
		SaveStoredQuery ( *tCmd.m_pStored, dBuf );

		uQueryHash = sphFNV64 ( &tCmd.m_pStored->m_uQUID, sizeof(tCmd.m_pStored->m_uQUID), uQueryHash );
		break;

	case RCOMMAND_DELETE:
		assert ( tCmd.m_dDeleteQueries.GetLength() || !tCmd.m_sDeleteTags.IsEmpty() );
		SaveDeleteQuery ( tCmd.m_dDeleteQueries.Begin(), tCmd.m_dDeleteQueries.GetLength(), tCmd.m_sDeleteTags.cstr(), dBuf );

		ARRAY_FOREACH ( i, tCmd.m_dDeleteQueries )
		{
			uQueryHash = sphFNV64 ( tCmd.m_dDeleteQueries.Begin()+i, sizeof(tCmd.m_dDeleteQueries[0]), uQueryHash );
		}
		uQueryHash = sphFNV64cont ( tCmd.m_sDeleteTags.cstr(), uQueryHash );
		break;

	case RCOMMAND_TRUNCATE:
		// FIXME!!! add reconfigure option here
		break;

	default:
		sError.SetSprintf ( "unknown command %d", tCmd.m_eCommand );
		return false;
	}

	return Replicate ( uQueryHash, dBuf, (*ppCluster)->m_pProvider, tMonitor, sError );
}

bool CommitMonitor_c::Commit ( CSphString & sError )
{
	ServedIndexRefPtr_c pServed = GetServed ( m_tCmd.m_sIndex );
	if ( !pServed )
	{
		sError = "requires an existing RT index";
		return false;
	}

	ServedDescPtr_c pDesc ( pServed, ( m_tCmd.m_eCommand==RCOMMAND_TRUNCATE ) );
	if ( m_tCmd.m_eCommand!=RCOMMAND_TRUNCATE && pDesc->m_eType!=eITYPE::PERCOLATE )
	{
		sError = "requires an existing Percolate index";
		return false;
	}

	bool bOk = false;
	PercolateIndex_i * pIndex = (PercolateIndex_i * )pDesc->m_pIndex;

	switch ( m_tCmd.m_eCommand )
	{
	case RCOMMAND_PQUERY_ADD:
		bOk = pIndex->Commit ( m_tCmd.m_pStored, sError );
		break;

	case RCOMMAND_DELETE:
		assert ( m_pDeletedCount );
		if ( !m_tCmd.m_dDeleteQueries.GetLength() )
			*m_pDeletedCount = pIndex->DeleteQueries ( m_tCmd.m_sDeleteTags.cstr() );
		else
			*m_pDeletedCount = pIndex->DeleteQueries ( m_tCmd.m_dDeleteQueries.Begin(), m_tCmd.m_dDeleteQueries.GetLength() );

		bOk = true;
		break;

	case RCOMMAND_TRUNCATE:
		bOk = pIndex->Truncate ( sError );
		if ( bOk && m_tCmd.m_bReconfigure )
		{
			CSphReconfigureSetup tSetup;
			bool bSame = pIndex->IsSameSettings ( m_tCmd.m_tReconfigureSettings, tSetup, sError );
			if ( !bSame && sError.IsEmpty() )
				pIndex->Reconfigure ( tSetup );
			bOk = sError.IsEmpty();
		}
		break;

	default:
		sError.SetSprintf ( "unknown command %d", m_tCmd.m_eCommand );
		return false;
	}

	return bOk;
}

static bool JsonConfigCheckProperty ( const cJSON * pNode, const char * sPropertyName, bool bOptional, CSphString & sError )
{
	const cJSON * pChild = cJSON_GetObjectItem( pNode, sPropertyName );
	if ( !pChild && bOptional )
		return true;

	if ( !pChild || !cJSON_IsString ( pChild ) )
	{
		sError.SetSprintf ( "property '%s' should be a string", sPropertyName );
		return false;
	}
	if ( strlen ( pChild->valuestring )==0 && !bOptional )
	{
		sError.SetSprintf ( "property '%s' empty", sPropertyName );
		return false;
	}

	return true;
}

bool JsonConfigCheckProperty ( const cJSON * pNode, const char * sPropertyName, CSphString & sError )
{
	return JsonConfigCheckProperty ( pNode, sPropertyName, false, sError );
}

bool JsonCheckIndex ( const cJSON * pIndex, CSphString & sError )
{
	if ( !pIndex->string || strlen ( pIndex->string )==0 )
	{
		sError = "empty index name";
		return false;
	}

	if ( !JsonConfigCheckProperty ( pIndex, "path", sError ) )
		return false;

	cJSON * pType = cJSON_GetObjectItem( pIndex, "type" );
	if ( !JsonConfigCheckProperty ( pIndex, "type", sError ) )
		return false;

	if ( TypeOfIndexConfig ( pType->valuestring )==eITYPE::ERROR_ )
	{
		sError.SetSprintf ( "index '%s' type '%s' is invalid", pIndex->string, pType->valuestring );
		return false;
	}

	return true;
}

void JsonConfigDefault ( cJSON ** ppConf )
{
	assert ( ppConf );
	if ( *ppConf )
	{
		cJSON_Delete( *ppConf );
		*ppConf = nullptr;
	}

	// default
	*ppConf = cJSON_CreateObject();
	cJSON * pDefaultIndexes = cJSON_CreateObject();
	cJSON_AddItemToObject( *ppConf, "indexes", pDefaultIndexes );
	cJSON * pDefaultSearchd = cJSON_CreateObject();
	cJSON_AddItemToObject( *ppConf, "searchd", pDefaultSearchd );
	cJSON * pDefaultClusters = cJSON_CreateObject();
	cJSON_AddItemToObject( *ppConf, "clusters", pDefaultClusters );
}

bool JsonGetError ( const char * sBuf, int iBufLen, CSphString & sError )
{
	if ( !cJSON_GetErrorPtr() )
		return false;

	const int iErrorWindowLen = 20;

	const char * sErrorStart = cJSON_GetErrorPtr() - iErrorWindowLen/2;
	if ( sErrorStart<sBuf )
		sErrorStart = sBuf;

	int iLen = iErrorWindowLen;
	if ( sErrorStart-sBuf+iLen>iBufLen )
		iLen = iBufLen - ( sErrorStart - sBuf );

	sError.SetSprintf ( "JSON parse error at: %.*s", iLen, sErrorStart );
	return true;
}

static const CSphString g_sDefaultReplicationURI = "gcomm://";

bool JsonConfigRead ( cJSON ** ppConf, CSphString & sError )
{
	assert ( ppConf );
	assert ( cJSON_GetObjectItem ( *ppConf, "indexes" ) );
	assert ( cJSON_GetObjectItem ( *ppConf, "searchd" ) );
	assert ( cJSON_GetObjectItem ( *ppConf, "clusters" ) );

	if ( !sphIsReadable ( g_sJsonConfigName, nullptr ) )
		return true;

	CSphAutoreader tConfigFile;
	if ( !tConfigFile.Open ( g_sJsonConfigName, sError ) )
		return false;

	int iSize = tConfigFile.GetFilesize();
	if ( !iSize )
		return true;
	
	CSphFixedVector<BYTE> dData ( iSize+1 );
	tConfigFile.GetBytes ( dData.Begin(), iSize );

	if ( tConfigFile.GetErrorFlag() )
	{
		sError = tConfigFile.GetErrorMessage();
		return false;
	}
	
	dData[iSize] = 0; // safe gap
	cJSON * pRoot = cJSON_Parse ( (const char*)dData.Begin() );
	if ( JsonGetError ( (const char *)dData.Begin(), dData.GetLength(), sError ) )
		return false;

	// FIXME!!! check for path duplicates
	// check indexes
	int iElem = 0;
	cJSON * pIndexes = cJSON_GetObjectItem ( pRoot, "indexes" );
	cJSON * pIndex = ( pIndexes ? pIndexes->child : nullptr );
	while ( pIndex!=nullptr )
	{
		if ( !JsonCheckIndex ( pIndex, sError ) )
		{
			// remove bad index from config
			sphWarning ( "index '%s'(%d): removed from JSON config, %s", pIndex->string, iElem, sError.cstr() );
			cJSON * pDelete = pIndex;
			// advance to next element prior delete
			pIndex = pIndex->next;
			cJSON_DetachItemViaPointer ( pIndexes, pDelete );
			cJSON_Delete ( pDelete );
			sError = "";
		} else
		{
			pIndex = pIndex->next;
		}
		iElem++;
	}

	// check clusters
	iElem = 0;
	cJSON * pClusters = cJSON_GetObjectItem ( pRoot, "clusters" );
	cJSON * pCluster = ( pClusters ? pClusters->child : nullptr );
	while ( pCluster!=nullptr )
	{
		bool bGood = true;
		if ( !pCluster->string || strlen ( pCluster->string )==0 )
		{
			sError = "empty cluster name";
			bGood = false;
		}

		// optional items such as: options and skipSst
		cJSON * pOpts = cJSON_GetObjectItem( pCluster, "options" );
		if ( pOpts && !JsonConfigCheckProperty ( pCluster, "options", true, sError ) )
		{
			sphWarning ( "cluster '%s'(%d): %s, set default", pCluster->string, iElem, sError.cstr() );
			cJSON_DetachItemViaPointer ( pCluster, pOpts );
			cJSON_Delete ( pOpts );
			pOpts = nullptr;
			sError = "";
		}
		if ( !pOpts )
		{
			cJSON_AddStringToObject ( pCluster, "options", "" );
		}

		cJSON * pSkipSst = cJSON_GetObjectItem( pCluster, "skipSst" );
		if ( pSkipSst && !cJSON_IsBool ( pSkipSst) )
		{
			sphWarning ( "cluster '%s'(%d): invalid skipSst value, should be boolean, set to false", pCluster->string, iElem );
			cJSON_DetachItemViaPointer ( pCluster, pSkipSst );
			cJSON_Delete ( pSkipSst );
			pSkipSst = nullptr;
		}
		if ( !pSkipSst )
		{
			cJSON_AddFalseToObject ( pCluster, "skipSst" );
		}

		cJSON * pUri = cJSON_GetObjectItem ( pCluster, "uri" );
		bGood &= ( !pUri || JsonConfigCheckProperty ( pCluster, "uri", true, sError ) );
		// set default uri
		if ( bGood && ( !pUri || strlen ( pUri->valuestring )==0 ) )
		{
			if ( pUri )
				cJSON_DeleteItemFromObject ( pCluster, "uri" );
			cJSON_AddStringToObject ( pCluster, "uri", g_sDefaultReplicationURI.cstr() );
		}

		// cluster desc
		bGood &= JsonConfigCheckProperty ( pCluster, "listen", sError );
		bGood &= JsonConfigCheckProperty ( pCluster, "path", sError );

		if ( !bGood )
		{
			// remove bad index from config
			sphWarning ( "cluster '%s'(%d): removed from JSON config, %s", pCluster->string, iElem, sError.cstr() );
			cJSON * pDelete = pCluster;
			// advance to next element prior delete
			pCluster = pCluster->next;
			cJSON_DetachItemViaPointer ( pClusters, pDelete );
			cJSON_Delete ( pDelete );
			sError = "";
		} else
		{
			pCluster = pCluster->next;
		}
		iElem++;
	}

	// move sections into config
	if ( *ppConf )
	{
		if ( pIndexes )
		{
			cJSON_DeleteItemFromObject ( *ppConf, "indexes" );
			cJSON_DetachItemViaPointer ( pRoot, pIndexes );
			cJSON_AddItemToObject ( *ppConf, "indexes", pIndexes );
		}

		if ( pClusters )
		{
			cJSON_DeleteItemFromObject ( *ppConf, "clusters" );
			cJSON_DetachItemViaPointer ( pRoot, pClusters );
			cJSON_AddItemToObject ( *ppConf, "clusters", pClusters );
		}

		cJSON * pDaemon = cJSON_GetObjectItem ( pRoot, "searchd" );
		if ( pDaemon )
		{
			cJSON_DeleteItemFromObject ( *ppConf, "searchd" );
			cJSON_DetachItemViaPointer ( pRoot, pDaemon );
			cJSON_AddItemToObject ( *ppConf, "searchd", pDaemon );
		}
	}

	cJSON_Delete ( pRoot );

	return true;
}


bool JsonConfigWrite ( const cJSON * pConf, CSphString & sError )
{
	if ( !pConf )
		return true;

	cJSON * pIndexes = cJSON_GetObjectItem ( pConf, "indexes" );
	cJSON * pSearchd = cJSON_GetObjectItem ( pConf, "searchd" );
	cJSON * pClusters = cJSON_GetObjectItem ( pConf, "clusters" );
	int iIndexesCount = pIndexes ? cJSON_GetArraySize ( pIndexes ) : 0;
	int iSearchdOpts = pSearchd ? cJSON_GetArraySize ( pSearchd ) : 0;
	int iClustersCount = pClusters ? cJSON_GetArraySize ( pClusters ) : 0;
	// no need to write empty JSON config
	if ( !iIndexesCount && !iSearchdOpts && !iClustersCount )
		return true;

	CSphString sConfigData;
	char * sRawData = cJSON_Print ( pConf );
	sConfigData.Adopt ( &sRawData );

	CSphString sNew, sOld;
	CSphString sCur = g_sJsonConfigName;
	sNew.SetSprintf ( "%s.new", sCur.cstr() );
	sOld.SetSprintf ( "%s.old", sCur.cstr() );

	CSphWriter tConfigFile;
	if ( !tConfigFile.OpenFile ( sNew, sError ) )
		return false;

	tConfigFile.PutBytes ( sConfigData.cstr(), sConfigData.Length() );
	tConfigFile.CloseFile();
	if ( tConfigFile.IsError() )
		return false;

	if ( sphIsReadable ( sCur, nullptr ) && rename ( sCur.cstr(), sOld.cstr() ) )
	{
		sError.SetSprintf ( "failed to rename current to old, '%s'->'%s', error '%s'", sCur.cstr(), sOld.cstr(), strerror(errno) );
		return false;
	}

	if ( rename ( sNew.cstr(), sCur.cstr() ) )
	{
		sError.SetSprintf ( "failed to rename new to current, '%s'->'%s', error '%s'", sNew.cstr(), sCur.cstr(), strerror(errno) );
		if ( sphIsReadable ( sOld, nullptr ) && rename ( sOld.cstr(), sCur.cstr() ) )
			sError.SetSprintf ( "%s, rollback failed too", sError.cstr() );
		return false;
	}

	unlink ( sOld.cstr() );

	return true;
}

void JsonConfigSetIndex ( const CSphString & sIndexName, const CSphString & sPath, eITYPE eIndexType, cJSON * pIndexes )
{
	const CSphString sType = GetTypeName ( eIndexType );

	cJSON * pIdx = cJSON_CreateObject();
	cJSON_AddStringToObject ( pIdx, "path", sPath.cstr() );
	cJSON_AddStringToObject ( pIdx, "type", sType.cstr() );
	cJSON_AddItemToObject( pIndexes, sIndexName.cstr(), pIdx );
}

void JsonConfigSetLocal ( cJSON * pConf )
{
	assert ( pConf );
	assert ( cJSON_GetObjectItem ( pConf, "indexes" ) );
	cJSON_DeleteItemFromObject ( pConf, "indexes" );
	cJSON * pIndexes = cJSON_CreateObject();
	cJSON_AddItemToObject( pConf, "indexes", pIndexes );

	for ( RLockedServedIt_c tIt ( g_pLocalIndexes ); tIt.Next (); )
	{
		auto pServed = tIt.Get();
		if ( !pServed )
			continue;

		ServedDescRPtr_c tDesc ( pServed );
		if ( !tDesc->m_bJson )
			continue;

		JsonConfigSetIndex ( tIt.GetName(), tDesc->m_sIndexPath, tDesc->m_eType, pIndexes );
	}
}

bool LoadIndex ( const CSphConfigSection & hIndex, const CSphString & sIndexName )
{
	// delete existed index first, does not allow to save it's data that breaks sync'ed index files
	// need a scope for RefRtr's to work
	{
		ServedIndexRefPtr_c pServedCur = GetServed ( sIndexName );
		if ( pServedCur )
		{
			// we are only owner of that index and will free it after delete from hash
			ServedDescWPtr_c pDesc ( pServedCur );
			if ( pDesc->IsMutable() )
			{
				ISphRtIndex * pIndex = (ISphRtIndex *)pDesc->m_pIndex;
				pIndex->ProhibitSave();
			}

			g_pLocalIndexes->Delete ( sIndexName );
		}
	}

	ESphAddIndex eAdd = AddIndex ( sIndexName.cstr(), hIndex );
	if ( eAdd!=ADD_DSBLED )
		return false;

	ServedIndexRefPtr_c pServed = GetDisabled ( sIndexName );
	ServedDescWPtr_c pDesc ( pServed );
	pDesc->m_bJson = true;

	bool bPreload = PreallocNewIndex ( *pDesc, &hIndex, sIndexName.cstr() );
	if ( !bPreload )
		return false;

	// finally add the index to the hash of enabled.
	g_pLocalIndexes->AddOrReplace ( pServed, sIndexName );
	pServed->AddRef ();
	g_pDisabledIndexes->Delete ( sIndexName );

	return true;
}

bool ReplicatedIndexes ( const cJSON * pIndexes, bool bBypass, ReplicationCluster_t * pCluster )
{
	assert ( !g_bSkipCfgJson && g_bCfgJsonValid );
	assert ( pCluster );
	assert ( bBypass || ( pIndexes && cJSON_GetArraySize ( pIndexes ) ) );

	// reset indexes list at cluster and JSON representation
	cJSON * pClusters = cJSON_GetObjectItem ( g_pCfgJson, "clusters" );
	if ( !pClusters )
	{
		sphWarning ( "no clusters found" );
		return false;
	}
	cJSON * pClusterJSON = cJSON_GetObjectItem ( pClusters, pCluster->m_sName.cstr() );
	if ( !pClusterJSON )
	{
		sphWarning ( "no cluster '%s' found", pCluster->m_sName.cstr() );
		return false;
	}

	// for bypass mode only seqno got replicated but not indexes
	if ( bBypass )
		return true;

	cJSON_DeleteItemFromObject ( pClusterJSON, "indexes" );
	cJSON * pClusterIndexes = cJSON_CreateArray();
	cJSON_AddItemToObject ( pClusterJSON, "indexes", pClusterIndexes );
	pCluster->m_dIndexes.Resize ( 0 );

	const cJSON * pIndex = ( pIndexes ? pIndexes->child : nullptr );
	int iIndexCount = ( pIndexes ? cJSON_GetArraySize ( pIndexes ) : 0 );
	int iLoaded = 0;
	while ( pIndex!=nullptr )
	{
		CSphString sIndexName = pIndex->string;

		g_tClustersLock.ReadLock();
		ReplicationCluster_t ** ppOrigCluster = g_hIndex2Cluster ( sIndexName );
		g_tClustersLock.Unlock();

		if ( ppOrigCluster && (*ppOrigCluster)!=pCluster )
		{
			sphWarning ( "index '%s' is already in another cluster '%s', replicated cluster '%s'", sIndexName.cstr(), (*ppOrigCluster)->m_sName.cstr(), pCluster->m_sName.cstr() );
			pIndex = pIndex->next;
			continue;
		}

		CSphVariant tPath ( cJSON_GetObjectItem( pIndex, "path" )->valuestring, 0 );
		CSphVariant tType ( cJSON_GetObjectItem( pIndex, "type" )->valuestring, 0 );

		CSphConfigSection hIndex;
		hIndex.Add ( tPath, "path" );
		hIndex.Add ( tType, "type" );

		bool bLoaded = LoadIndex ( hIndex, sIndexName );
		iLoaded += ( bLoaded ? 1 : 0 );
		pIndex = pIndex->next;

		if ( !bLoaded )
		{
			sphWarning ( "index '%s': removed from JSON config", sIndexName.cstr() );
			continue;
		} 

		// add index into hash, cluster indexes list, JSON representation
		if ( !ppOrigCluster )
		{
			g_tClustersLock.WriteLock();
			g_hIndex2Cluster.Add ( pCluster, sIndexName );
			g_tClustersLock.Unlock();
		}

		pCluster->m_dIndexes.Add ( sIndexName );
		cJSON_AddItemToArray ( pClusterIndexes, cJSON_CreateString ( sIndexName.cstr() ) );
	}

	CSphString sError;
	JsonConfigSetLocal ( g_pCfgJson );
	if ( !JsonConfigWrite ( g_pCfgJson, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return false;
	}

	return ( iLoaded==iIndexCount );
}

static bool ClusterCheckPath ( const CSphString & sPath, const CSphString & sName, const CSphVector<ReplicationCluster_t *> & dClusters, CSphString & sError )
{
	if ( !sphIsReadable ( sPath ) )
	{
		sError.SetSprintf ( "can not access cluster '%s' directory %s", sName.cstr(), sPath.cstr() );
		return false;
	}

	ARRAY_FOREACH ( i, dClusters )
	{
		const ReplicationCluster_t * pCluster = dClusters[i];
		if ( sPath==pCluster->m_sPath )
		{
			sError.SetSprintf ( "duplicate paths, cluster '%s' has the same path as '%s'", sName.cstr(), pCluster->m_sName.cstr() );
			return false;
		}
	}

	return true;
}

static bool NewClusterForce ( const CSphString & sPath, const CSphString & sName )
{
	CSphString sClusterState;
	sClusterState.SetSprintf ( "%s/grastate.dat", sPath.cstr() );
	CSphString sNewState;
	sNewState.SetSprintf ( "%s/grastate.dat.new", sPath.cstr() );
	CSphString sOldState;
	sOldState.SetSprintf ( "%s/grastate.dat.old", sPath.cstr() );
	const char sPattern[] = "safe_to_bootstrap";
	const int iPatternLen = sizeof(sPattern)-1;
	CSphString sError;

	// cluster starts well without grastate.dat file
	if ( !sphIsReadable ( sClusterState ) )
		return true;

	CSphAutoreader tReader;
	if ( !tReader.Open ( sClusterState, sError ) )
	{
		sphWarning ( "%s, cluster '%s' skipped", sError.cstr(), sName.cstr() );
		return false;
	}

	CSphWriter tWriter;
	if ( !tWriter.OpenFile ( sNewState, sError ) )
	{
		sphWarning ( "%s, cluster '%s' skipped", sError.cstr(), sName.cstr() );
		return false;
	}

	CSphFixedVector<char> dBuf { 2048 };
	SphOffset_t iStateSize = tReader.GetFilesize();
	while ( tReader.GetPos()<iStateSize )
	{
		int iLineLen = tReader.GetLine ( dBuf.Begin(), dBuf.GetLengthBytes() );
		// replace value of safe_to_bootstrap to 1
		if ( iLineLen>iPatternLen && strncmp ( sPattern, dBuf.Begin(), iPatternLen )==0 )
			iLineLen = snprintf ( dBuf.Begin(), dBuf.GetLengthBytes(), "%s: 1", sPattern );

		tWriter.PutBytes ( dBuf.Begin(), iLineLen );
		tWriter.PutByte ( '\n' );
	}

	if ( tReader.GetErrorFlag() )
	{
		sphWarning ( "%s, cluster '%s' skipped", tReader.GetErrorMessage().cstr(), sName.cstr() );
		return false;
	}
	if ( tWriter.IsError() )
	{
		sphWarning ( "%s, cluster '%s' skipped", sError.cstr(), sName.cstr() );
		return false;
	}

	tReader.Close();
	tWriter.CloseFile();

	if ( sph::rename ( sClusterState.cstr (), sOldState.cstr () )!=0 )
	{
		sphWarning ( "failed to rename %s to %s, cluster '%s' skipped", sClusterState.cstr(), sOldState.cstr(), sName.cstr() );
		return false;
	}

	if ( sph::rename ( sNewState.cstr (), sClusterState.cstr () )!=0 )
	{
		sphWarning ( "failed to rename %s to %s, cluster '%s' skipped", sNewState.cstr(), sClusterState.cstr(), sName.cstr() );
		return false;
	}

	::unlink ( sOldState.cstr() );
	return true;
}

bool ReplicationStart ( const CSphConfigSection & hSearchd, bool bNewCluster, bool bForce )
{
	if ( g_bSkipCfgJson )
		return false;

	if ( !GetReplicationDL() )
	{
		// warning only in case clusters declared
		cJSON * pClusters = cJSON_GetObjectItem ( g_pCfgJson, "clusters" );
		if ( pClusters && pClusters->child )
			sphWarning ( "got cluster but provider not set, replication disabled" );
		return false;
	}

	CSphString sSstLog = hSearchd.GetStr ( "log", "" );
	ReplicationInit ( sSstLog );

	// node with empty incoming addresses works as GARB - does not affect FC
	// might hung on pushing 1500 transactions
	StringBuilder_c sIncomingAddrs;
	for ( const CSphVariant * pOpt = hSearchd("listen"); pOpt; pOpt = pOpt->m_pNext )
	{
		if ( sIncomingAddrs.GetLength() )
			sIncomingAddrs += ", ";
		sIncomingAddrs += pOpt->strval().cstr();
	}
	if ( sIncomingAddrs.IsEmpty() )
	{
		sphWarning ( "no listen found, can not set incoming addresses, replication disabled" );
		return false;
	}

	CSphString sError;
	g_sIncomingAddr = sIncomingAddrs.cstr();

	cJSON * pClusters = cJSON_GetObjectItem ( g_pCfgJson, "clusters" );
	cJSON * pCluster = pClusters ? pClusters->child : nullptr;
	while ( pCluster )
	{
		ReplicationArgs_t tArgs;
		// global options
		tArgs.m_bNewCluster = ( bNewCluster || bForce );
		tArgs.m_sIncomingAdresses = sIncomingAddrs.cstr();

		CSphScopedPtr<ReplicationCluster_t> pElem ( new ReplicationCluster_t );
		pElem->m_sURI = cJSON_GetObjectItem ( pCluster, "uri" )->valuestring;
		pElem->m_sName = pCluster->string;
		pElem->m_sListen = cJSON_GetObjectItem ( pCluster, "listen" )->valuestring;
		pElem->m_sPath = cJSON_GetObjectItem ( pCluster, "path" )->valuestring;
		pElem->m_sOptions = ReplicateClusterOptions ( cJSON_GetObjectItem ( pCluster, "options" )->valuestring, g_eLogLevel>=SPH_LOG_RPL_DEBUG );
		pElem->m_bSkipSST = ( !!cJSON_IsTrue ( cJSON_GetObjectItem ( pCluster, "skipSst" ) ) );

		// check listen IP address
		ListenerDesc_t tListen = ParseListener ( pElem->m_sListen.cstr() );
		char sListenBuf [ SPH_ADDRESS_SIZE ];
		sphFormatIP ( sListenBuf, sizeof(sListenBuf), tListen.m_uIP );
		if ( tListen.m_uIP==0 )
		{
			sphWarning ( "cluster '%s' listen empty address (%s), skipped", pElem->m_sName.cstr(), sListenBuf );
			pCluster = pCluster->next;
			continue;
		}
		pElem->m_sListenSST.SetSprintf ( "%s:%d", sListenBuf, tListen.m_iPort+2 );

		bool bUriEmpty = ( pElem->m_sURI==g_sDefaultReplicationURI );
		if ( bUriEmpty )
		{
			if ( !tArgs.m_bNewCluster )
				sphWarning ( "uri property is empty, force new cluster '%s'", pElem->m_sName.cstr() );
			tArgs.m_bNewCluster = true;
		}

		// check cluster path is unique
		if ( !ClusterCheckPath ( pElem->m_sPath, pElem->m_sName, g_dClusters, sError ) )
		{
			sphWarning ( "%s, skipped", sError.cstr() );
			pCluster = pCluster->next;
			continue;
		}

		// set safe_to_bootstrap to 1 into grastate.dat file at cluster path
		if ( ( bForce || bUriEmpty ) && !NewClusterForce ( pElem->m_sPath, pElem->m_sName ) )
		{
			pCluster = pCluster->next;
			continue;
		}

		// set indexes prior replication init
		cJSON * pClusterIndexes = cJSON_GetObjectItem ( pCluster, "indexes" );
		cJSON * pClusterIndex = pClusterIndexes ? pClusterIndexes->child : nullptr;
		while ( pClusterIndex )
		{
			if ( cJSON_IsString ( pClusterIndex ) )
			{
				pElem->m_dIndexes.Add ( pClusterIndex->valuestring );
				g_hIndex2Cluster.Add ( pElem.Ptr(), pElem->m_dIndexes.Last() );
			}
			pClusterIndex = pClusterIndex->next;
		}

		// save data to disk to sync it
		// but only for just created indexes
		ARRAY_FOREACH ( i, pElem->m_dIndexes )
		{
			ServedDescRPtr_c pServed ( GetServed ( pElem->m_dIndexes[i] ) );
			if ( !pServed || !pServed->m_pIndex || !pServed->IsMutable() )
				continue;

			((ISphRtIndex *)pServed->m_pIndex)->ForceDiskChunk();
		}

		tArgs.m_pCluster = pElem.Ptr();

		CSphString sError;
		if ( !ReplicateClusterInit ( tArgs, sError ) )
			sphDie ( "%s", sError.cstr() );

		assert ( pElem->m_pProvider );
		g_dClusters.Add ( pElem.LeakPtr() );

		pCluster = pCluster->next;
	}
	return true;
}

void ReplicationWait()
{
	int iClusters = g_dClusters.GetLength();
	sphLogDebugRpl ( "waiting replication of %d cluster(s)", iClusters );
	ARRAY_FOREACH ( iCluster, g_dClusters )
		g_dClusters[iCluster]->m_tSync.WaitEvent();
	sphLogDebugRpl ( "wait over of replication" );

	// check of cluster indexes after possible sync
	g_hIndex2Cluster.IterateStart();
	while ( g_hIndex2Cluster.IterateNext() )
	{
		const CSphString & sIndexName = g_hIndex2Cluster.IterateGetKey();

		ServedDescRPtr_c pDesc ( GetServed ( sIndexName ) );
		if ( !pDesc )
		{
			sphWarning ( "missed cluster index %s", sIndexName.cstr() );
			continue;
		}

		if ( pDesc->m_eType!=eITYPE::PERCOLATE )
		{
			sphWarning ( "index %s is not percolate, no replicator set", sIndexName.cstr() );
			continue;
		}
	}
}

static bool CheckJoinClusterValue ( const SmallStringHash_T<SqlInsert_t *> & hValues, const char * sName, CSphString & sError )
{
	if ( !hValues.Exists ( sName ) )
	{
		sError.SetSprintf ( "'%s' not set", sName );
		return false;
	}
	if ( hValues[sName]->m_sVal.IsEmpty() )
	{
		sError.SetSprintf ( "'%s' should have string value", sName );
		return false;
	}

	return true;
}

bool ReplicationJoin ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, CSphString & sError )
{
	if ( !GetReplicationDL() )
	{
		sError.SetSprintf ( "provider not set" );
		return false;
	}

	if ( g_sIncomingAddr.IsEmpty() )
	{
		sError.SetSprintf ( "incoming addresses not set" );
		return false;
	}

	assert ( dNames.GetLength()==dValues.GetLength() );
	SmallStringHash_T<SqlInsert_t *> hValues;
	ARRAY_FOREACH ( i, dNames )
		hValues.Add ( dValues.Begin() + i, dNames[i] );

	g_tClustersLock.ReadLock();
	bool bClusterExists = g_dClusters.FindFirst ( [&] ( const ReplicationCluster_t * pCluster ) { return pCluster->m_sName==sCluster; } );
	g_tClustersLock.Unlock();
	if ( bClusterExists )
	{
		sError.SetSprintf ( "cluster '%s' already exists", sCluster.cstr() );
		return false;
	}
	if ( !CheckJoinClusterValue ( hValues, "uri", sError ) )
		return false;
	if ( !CheckJoinClusterValue ( hValues, "listen", sError ) )
		return false;
	if ( !CheckJoinClusterValue ( hValues, "path", sError ) )
		return false;

	ReplicationArgs_t tArgs;
	// global options
	tArgs.m_bNewCluster = false;
	tArgs.m_bJoin = true;
	tArgs.m_sIncomingAdresses = g_sIncomingAddr.cstr();

	CSphScopedPtr<ReplicationCluster_t> pElem ( new ReplicationCluster_t );
	pElem->m_sURI = hValues["uri"]->m_sVal;
	pElem->m_sName = sCluster;
	pElem->m_sListen = hValues["listen"]->m_sVal;
	pElem->m_sPath = hValues["path"]->m_sVal;
	CSphString sOpts;
	if ( hValues.Exists ( "options" ) )
		sOpts = hValues["options"]->m_sVal;
	pElem->m_sOptions = ReplicateClusterOptions ( sOpts.cstr(), g_eLogLevel>=SPH_LOG_RPL_DEBUG );
	// uri should start with gcomm://
	if ( !pElem->m_sURI.Begins ( "gcomm://" ) )
		pElem->m_sURI.SetSprintf ( "gcomm://%s", pElem->m_sURI.cstr() );

	// check cluster path is unique
	g_tClustersLock.ReadLock();
	bool bValidPath = ClusterCheckPath ( pElem->m_sPath, pElem->m_sName, g_dClusters, sError );
	g_tClustersLock.Unlock();
	if ( !bValidPath )
		return false;

	// check listen IP address
	ListenerDesc_t tListen = ParseListener ( pElem->m_sListen.cstr() );
	char sListenBuf [ SPH_ADDRESS_SIZE ];
	sphFormatIP ( sListenBuf, sizeof(sListenBuf), tListen.m_uIP );
	if ( tListen.m_uIP==0 )
	{
		sError.SetSprintf ( "cluster '%s' listen empty address (%s)", pElem->m_sName.cstr(), sListenBuf );
		return false;
	}
	pElem->m_sListenSST.SetSprintf ( "%s:%d", sListenBuf, tListen.m_iPort+2 );

	tArgs.m_pCluster = pElem.Ptr();

	// need valid cluster description at config
	cJSON * pClusters = cJSON_GetObjectItem ( g_pCfgJson, "clusters" );
	if ( !pClusters )
	{
		sphWarning ( "no clusters found" );
		return false;
	}
	cJSON * pCluster = cJSON_CreateObject();
	cJSON_AddItemToObject ( pCluster, "uri", cJSON_CreateString ( pElem->m_sURI.cstr() ) );
	cJSON_AddItemToObject ( pCluster, "listen", cJSON_CreateString ( pElem->m_sListen.cstr() ) );
	cJSON_AddItemToObject ( pCluster, "path", cJSON_CreateString ( pElem->m_sPath.cstr() ) );
	cJSON_AddItemToObject ( pCluster, "options", cJSON_CreateString ( pElem->m_sOptions.cstr() ) );
	cJSON_AddItemToObject ( pCluster, "indexes", cJSON_CreateArray() );
	cJSON_AddItemToObject ( pClusters, pElem->m_sName.cstr(), pCluster );

	if ( !ReplicateClusterInit ( tArgs, sError ) )
	{
		// need to remove cluster description from config
		cJSON_DeleteItemFromObject ( pClusters, pElem->m_sName.cstr() );
		return false;
	}

	CSphAutoEvent * pSync = &pElem->m_tSync;

	assert ( pElem->m_pProvider );
	g_tClustersLock.WriteLock();
	g_dClusters.Add ( pElem.LeakPtr() );
	g_tClustersLock.Unlock();

	// wait sync to finish
	pSync->WaitEvent();

	return true;
}
