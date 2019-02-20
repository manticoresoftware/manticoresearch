
#include "sphinx.h"
#include "sphinxstd.h"
#include "sphinxutils.h"
#include "sphinxint.h"
#include "sphinxpq.h"
#include "searchdreplication.h"
#include "sphinxjson.h"
#include "replication/wsrep_api.h"

#define USE_PSI_INTERFACE 1

#if USE_WINDOWS
#include <io.h> // for setmode(). open() on windows
#define sphSeek		_lseeki64
#else
#define sphSeek		lseek
#endif

const char * GetReplicationDL()
{
#ifndef REPLICATION_LIB
	return nullptr;
#else
	return REPLICATION_LIB;
#endif
}

// global application context for wsrep callbacks
static CSphString	g_sLogFile;

static CSphString	g_sDataDir;
static CSphString	g_sConfigPath;
static bool			g_bCfgHasData = false;
// FIXME!!! take from config
static int			g_iRemoteTimeout = 120 * 1000; // 2 minutes in msec
static const char * g_sDebugOptions = "debug=on;cert.log_conflicts=yes";

typedef wsrep_member_status_t ClusterState_e;

struct ClusterDesc_t
{
	CSphString	m_sName;
	CSphString	m_sURI;
	CSphString	m_sListen; // +1 after listen is IST port
	CSphString	m_sPath; // path relative to data_dir
	CSphString	m_sOptions;
	CSphVector<CSphString> m_dIndexes;
};

struct ReplicationCluster_t : public ClusterDesc_t
{
	// replicator
	wsrep_t *	m_pProvider = nullptr;
	SphThread_t	m_tRecvThd;
	CSphString	m_sNodes; // raw nodes addresses (API and replication) from whole cluster
	RwLock_t m_tNodesLock;
	CSphVector<uint64_t> m_dIndexHashes;
	CSphAtomicL m_eState { WSREP_MEMBER_UNDEFINED };

	// state
	CSphFixedVector<BYTE>	m_dUUID { 0 };
	int						m_iConfID = 0;
	int						m_iStatus = 0;
	int						m_iSize = 0;
	int						m_iIdx = 0;

	void					UpdateIndexHashes();
};

struct IndexDesc_t
{
	CSphString	m_sName;
	CSphString	m_sPath;
	IndexType_e		m_eType = IndexType_e::ERROR_;
};

struct ReplicationArgs_t
{
	ReplicationCluster_t *	m_pCluster = nullptr;
	bool					m_bNewCluster = false;
	bool					m_bJoin = false;
	CSphString				m_sIncomingAdresses;
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
	bool CommitTOI ( CSphString & sError );

private:
	ReplicationCommand_t & m_tCmd;
	int * m_pDeletedCount = nullptr;
};

static SmallStringHash_T<ReplicationCluster_t *> g_hClusters;
static CSphString g_sIncomingAddr;
static RwLock_t g_tClustersLock;

// description of clusters and indexes loaded from JSON config
static CSphVector<ClusterDesc_t> g_dCfgClusters;
static CSphVector<IndexDesc_t> g_dCfgIndexes;

static void ReplicationAbort();

static bool JsonConfigWrite ( const CSphString & sConfigPath, const SmallStringHash_T<ReplicationCluster_t *> & hClusters, const CSphVector<IndexDesc_t> & dIndexes, CSphString & sError );
static bool JsonConfigRead ( const CSphString & sConfigPath, CSphVector<ClusterDesc_t> & hClusters, CSphVector<IndexDesc_t> & dIndexes, CSphString & sError );
static void GetLocalIndexes ( CSphVector<IndexDesc_t> & dIndexes );
static void JsonConfigSetIndex ( const IndexDesc_t & tIndex, JsonObj_c & tIndexes );
static bool JsonLoadIndexDesc ( const JsonObj_c & tJson, CSphString & sError, IndexDesc_t & tIndex );
static void JsonConfigDumpClusters ( const SmallStringHash_T<ReplicationCluster_t *> & hClusters, StringBuilder_c & tOut );
static void JsonConfigDumpIndexes ( const CSphVector<IndexDesc_t> & dIndexes, StringBuilder_c & tOut );

static bool CheckPath ( const CSphString & sPath, bool bCheckWrite, CSphString & sError, const char * sCheckFileName="tmp" );
static CSphString GetClusterPath ( const CSphString & sPath );
static void GetNodeAddress ( const ClusterDesc_t & tCluster, CSphString & sAddr );

struct PQRemoteReply_t;
struct PQRemoteData_t;

static bool RemoteClusterDelete ( const CSphString & sCluster, CSphString & sError );
static bool RemoteFileReserve ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError );
static bool RemoteFileStore ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError );
static bool RemoteLoadIndex ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError );
static bool SendClusterIndexes ( const ReplicationCluster_t * pCluster, const CSphString & sNode, bool bBypass, const wsrep_gtid_t & tStateID, CSphString & sError );
static bool RemoteClusterSynced ( const PQRemoteData_t & tCmd, CSphString & sError );

static bool ParseCmdReplicated ( const BYTE * pData, int iLen, bool bIsolated, const CSphString & sCluster, ReplicationCommand_t & tCmd );
static bool HandleCmdReplicated ( ReplicationCommand_t & tCmd );

// receiving thread context for wsrep callbacks
struct ReceiverCtx_t : ISphRefcounted
{
	ReplicationCluster_t *	m_pCluster = nullptr;
	bool					m_bJoin = false;
	CSphAutoEvent			m_tStarted;

	ReplicationCommand_t	m_tCmd;

	ReceiverCtx_t() = default;
	~ReceiverCtx_t()
	{
		SafeDelete ( m_tCmd.m_pStored );
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


static const WORD g_iReplicateCommandVer = 0x101;

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

static void UpdateGroupView ( const wsrep_view_info_t * pView, ReplicationCluster_t * pCluster )
{
	StringBuilder_c sBuf ( ";" );
	const wsrep_member_info_t * pBoxes = pView->members;
	for ( int i=0; i<pView->memb_num; i++ )
	{
		if ( i!=pView->my_idx )
			sBuf += pBoxes[i].incoming;
	}

	// change of nodes happens only here with single thread
	// its safe to compare nodes string here without lock
	if ( pCluster->m_sNodes!=sBuf.cstr() )
	{
		ScWL_t tLock ( pCluster->m_tNodesLock );
		pCluster->m_sNodes = sBuf.cstr();
	}
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
	UpdateGroupView ( pView, pCluster );

	*ppSstReq = nullptr;
	*pSstReqLen = 0;

	if ( pView->state_gap )
	{
		CSphString sAddr;
		GetNodeAddress ( *pCluster, sAddr );
		sphLogDebugRpl ( "join %s to %s", sAddr.cstr(), pCluster->m_sName.cstr() );

		*pSstReqLen = sAddr.Length() + 1;
		*ppSstReq = sAddr.Leak();
		pCluster->m_eState = WSREP_MEMBER_JOINER;
	}

	return WSREP_CB_SUCCESS;
}

static wsrep_cb_status_t SstDonate_fn ( void * pAppCtx, void * pRecvCtx, const void * sMsg, size_t iMsgLen, const wsrep_gtid_t * pStateID, const char * pState, size_t iStateLen, wsrep_bool_t bBypass )
{
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pRecvCtx;
	
	CSphString sNode;
	sNode.SetBinary ( (const char *)sMsg, iMsgLen );

	wsrep_gtid_t tGtid = *pStateID;
	char sGtid[WSREP_GTID_STR_LEN];
	Verify ( wsrep_gtid_print ( &tGtid, sGtid, sizeof(sGtid) )>0 );

	ReplicationCluster_t * pCluster = pLocalCtx->m_pCluster;
	sphLogDebugRpl ( "donate %s to %s, gtid %s, bypass %d", pCluster->m_sName.cstr(), sNode.cstr(), sGtid, (int)bBypass );

	CSphString sError;

	pCluster->m_eState = WSREP_MEMBER_DONOR;
	const bool bOk = SendClusterIndexes ( pCluster, sNode, bBypass, tGtid, sError );
	pCluster->m_eState = WSREP_MEMBER_SYNCED;

	if ( !bOk )
	{
		sphWarning ( "%s", sError.cstr() );
		tGtid.seqno = WSREP_SEQNO_UNDEFINED;
	}

	pCluster->m_pProvider->sst_sent( pCluster->m_pProvider, &tGtid, ( bOk ? 0 : -ECANCELED ) );

	sphLogDebugRpl ( "donate cluster %s to %s, gtid %s, bypass %d, done %d", pCluster->m_sName.cstr(), sNode.cstr(), sGtid, (int)bBypass, (int)bOk );

	return ( bOk ? WSREP_CB_SUCCESS : WSREP_CB_FAILURE );
}

static void Synced_fn ( void * pAppCtx )
{
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pAppCtx;

	assert ( pLocalCtx );
	ReplicationCluster_t * pCluster = pLocalCtx->m_pCluster;
	pCluster->m_eState = WSREP_MEMBER_SYNCED;

	sphLogDebugRpl ( "synced cluster %s", pCluster->m_sName.cstr() );
}

// This is called to "apply" writeset.
// If writesets don't conflict on keys, it may be called concurrently to
// utilize several CPU cores.
static wsrep_cb_status_t Apply_fn ( void * pCtx, const void * pData, size_t uSize, uint32_t uFlags, const wsrep_trx_meta_t * pMeta )
{
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pCtx;

	bool bCommit = ( ( uFlags & WSREP_FLAG_COMMIT )!=0 );
	bool bIsolated = ( ( uFlags & WSREP_FLAG_ISOLATION )!=0 );
	sphLogDebugRpl ( "writeset at apply, seq " INT64_FMT ", size %d, flags %u, on %s", (int64_t)pMeta->gtid.seqno, (int)uSize, uFlags, ( bCommit ? "commit" : "rollback" ) );

	if ( !ParseCmdReplicated ( (const BYTE *)pData, uSize, bIsolated, pLocalCtx->m_pCluster->m_sName, pLocalCtx->m_tCmd ) )
		return WSREP_CB_FAILURE;

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
		bool bOk = false;
		if ( pLocalCtx->m_tCmd.m_bIsolated )
		{
			bOk = HandleCmdReplicated ( pLocalCtx->m_tCmd );
		} else
		{
			pWsrep->applier_pre_commit ( pWsrep, const_cast<void *>( hndTrx ) );

			bOk = HandleCmdReplicated ( pLocalCtx->m_tCmd );

			pWsrep->applier_interim_commit ( pWsrep, const_cast<void *>( hndTrx ) );
			pWsrep->applier_post_commit ( pWsrep, const_cast<void *>( hndTrx ) );
		}

		sphLogDebugRpl ( "seq " INT64_FMT ", committed %d, isolated %d", (int64_t)pMeta->gtid.seqno, (int)bOk, (int)pLocalCtx->m_tCmd.m_bIsolated );

		eRes = ( bOk ? WSREP_CB_SUCCESS : WSREP_CB_FAILURE );
	}

	return eRes;
}

static wsrep_cb_status_t Unordered_fn ( void * pCtx, const void * pData, size_t uSize )
{
	//ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pCtx;
	sphLogDebugRpl ( "unordered" );
	return WSREP_CB_SUCCESS;
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

	// join thread shut with event set but start up sequence should shutdown daemon
	if ( !pCtx->m_bJoin && ( eState==WSREP_CONN_FAIL || eState==WSREP_NODE_FAIL || eState==WSREP_FATAL ) )
	{
		pCtx->Release();
		ReplicationAbort();
		return;
	}

	pCtx->Release();
}

void Instr_fn ( wsrep_pfs_instr_type_t , wsrep_pfs_instr_ops_t , wsrep_pfs_instr_tag_t , void ** , void ** , const void * );

bool ReplicateClusterInit ( ReplicationArgs_t & tArgs, CSphString & sError )
{
	assert ( !g_sDataDir.IsEmpty() );
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

	sphLogDebugRpl ( "listen IP '%s', '%s'", tArgs.m_pCluster->m_sListen.cstr(), sMyName.cstr() );

	ReceiverCtx_t * pRecvArgs = new ReceiverCtx_t();
	pRecvArgs->m_pCluster = tArgs.m_pCluster;
	pRecvArgs->m_bJoin = tArgs.m_bJoin;
	CSphString sFullClusterPath = GetClusterPath ( tArgs.m_pCluster->m_sPath );
	CSphString sOptions = tArgs.m_pCluster->m_sOptions;
	if ( g_eLogLevel>=SPH_LOG_RPL_DEBUG )
	{
		if ( sOptions.IsEmpty() )
			sOptions = g_sDebugOptions;
		else
			sOptions.SetSprintf ( "%s;%s", sOptions.cstr(), g_sDebugOptions );
	}

	// wsrep provider initialization arguments
	wsrep_init_args wsrep_args;
	wsrep_args.app_ctx		= pRecvArgs,

	wsrep_args.node_name	= sMyName.cstr();
	wsrep_args.node_address	= tArgs.m_pCluster->m_sListen.cstr();
	// must to be set otherwise node works as GARB - does not affect FC and might hung 
	wsrep_args.node_incoming = tArgs.m_sIncomingAdresses.cstr();
	wsrep_args.data_dir		= sFullClusterPath.cstr(); // working directory
	wsrep_args.options		= sOptions.scstr();
	wsrep_args.proto_ver	= 127; // maximum supported application event protocol

	wsrep_args.state_id		= &tStateID;
	wsrep_args.state		= "";
	wsrep_args.state_len	= 0;

	wsrep_args.view_handler_cb = ViewChanged_fn;
	wsrep_args.synced_cb	= Synced_fn;
	wsrep_args.sst_donate_cb = SstDonate_fn;
	wsrep_args.apply_cb		= Apply_fn;
	wsrep_args.commit_cb	= Commit_fn;
	wsrep_args.unordered_cb	= Unordered_fn;
	wsrep_args.logger_cb	= Logger_fn;

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

static bool Replicate ( uint64_t uQueryHash, const CSphVector<BYTE> & dBuf, wsrep_t * pProvider, CommitMonitor_c & tMonitor, CSphString & sError )
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
		if ( bOk && !tMonitor.Commit( sError ) )
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

static bool ReplicateTOI ( uint64_t uQueryHash, const CSphVector<BYTE> & dBuf, wsrep_t * pProvider, CommitMonitor_c & tMonitor, CSphString & sError )
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

	wsrep_status_t tRes = pProvider->to_execute_start ( pProvider, iConnId, &tKey, 1, &tQuery, 1, &tLogMeta );
	bool bOk = CheckResult ( tRes, tLogMeta, "to_execute_start", sError );

	sphLogDebugRpl ( "replicating TOI %d, seq " INT64_FMT ", hash " UINT64_FMT, (int)bOk, (int64_t)tLogMeta.gtid.seqno, uQueryHash );

	// FXIME!!! can not fail TOI transaction
	if ( bOk )
		tMonitor.CommitTOI( sError );

	if ( bOk )
	{

		tRes = pProvider->to_execute_end ( pProvider, iConnId );
		CheckResult ( tRes, tLogMeta, "to_execute_end", sError );

		sphLogDebugRpl ( "%s seq " INT64_FMT ", hash " UINT64_FMT, ( bOk ? "committed" : "rolled-back" ), (int64_t)tLogMeta.gtid.seqno, uQueryHash );
	}

	// FIXME!!! move ThreadID and to net loop handler exit
	pProvider->free_connection ( pProvider, iConnId );

	return bOk;
}

static void ReplicateClusterStats ( ReplicationCluster_t * pCluster, VectorLike & dOut )
{
	if ( !pCluster || !pCluster->m_pProvider )
		return;

	assert ( pCluster->m_iStatus>=0 && pCluster->m_iStatus<WSREP_VIEW_MAX );

	const char * sName = pCluster->m_sName.cstr();

	// cluster vars
	if ( dOut.MatchAdd ( "cluster_name" ) )
		dOut.Add( pCluster->m_sName );
	if ( dOut.MatchAddVa ( "cluster_%s_state_uuid", sName ) )
	{
		wsrep_uuid_t tUUID;
		memcpy ( tUUID.data, pCluster->m_dUUID.Begin(), pCluster->m_dUUID.GetLengthBytes() );
		char sUUID[WSREP_UUID_STR_LEN+1] = { '\0' };
		wsrep_uuid_print ( &tUUID, sUUID, sizeof(sUUID) );
		dOut.Add() = sUUID;
	}
	if ( dOut.MatchAddVa ( "cluster_%s_conf_id", sName ) )
		dOut.Add().SetSprintf ( "%d", pCluster->m_iConfID );
	if ( dOut.MatchAddVa ( "cluster_%s_status", sName ) )
		dOut.Add() = g_sClusterStatus[pCluster->m_iStatus];
	if ( dOut.MatchAddVa ( "cluster_%s_size", sName ) )
		dOut.Add().SetSprintf ( "%d", pCluster->m_iSize );
	if ( dOut.MatchAddVa ( "cluster_%s_local_index", sName ) )
		dOut.Add().SetSprintf ( "%d", pCluster->m_iIdx );

	// cluster indexes
	if ( dOut.MatchAddVa ( "cluster_%s_indexes_count", sName ) )
		dOut.Add().SetSprintf ( "%d", pCluster->m_dIndexes.GetLength() );
	if ( dOut.MatchAddVa ( "cluster_%s_indexes", sName ) )
	{
		StringBuilder_c tBuf ( "," );
		for ( const CSphString & sIndex : pCluster->m_dIndexes )
			tBuf += sIndex.cstr();

		dOut.Add( tBuf.cstr() );
	}

	// cluster status
	wsrep_stats_var * pVarsStart = pCluster->m_pProvider->stats_get ( pCluster->m_pProvider );
	if ( !pVarsStart )
		return;

	wsrep_stats_var * pVars = pVarsStart;
	while ( pVars->name )
	{
		if ( dOut.MatchAddVa ( "cluster_%s_%s", sName, pVars->name ) )
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

bool ReplicateSetOption ( const CSphString & sCluster, const CSphString & sOpt, CSphString & sError )
{
	ScRL_t tClusterGuard ( g_tClustersLock );		
	ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
	if ( !ppCluster )
	{
		sError.SetSprintf ( "unknown cluster '%s' in SET statement", sCluster.cstr() );
		return false;
	}

	const ReplicationCluster_t * pCluster = *ppCluster;
	wsrep_status_t tOk = pCluster->m_pProvider->options_set ( pCluster->m_pProvider, sOpt.cstr() );
	return ( tOk==WSREP_OK );
}

void ReplicationAbort()
{
	sphWarning ( "shutting down due to abort" );
	// no need to shutdown cluster properly in case of cluster signaled abort
	g_hClusters.Reset();
	Shutdown();
}

void ReplicateClustersDelete()
{
	wsrep_t * pProvider = nullptr;
	void * pIt = nullptr;
	while ( g_hClusters.IterateNext ( &pIt ) )
	{
		pProvider = g_hClusters.IterateGet ( &pIt )->m_pProvider;
		ReplicateClusterDone ( g_hClusters.IterateGet ( &pIt ) );
	}
	g_hClusters.Reset();

	if ( pProvider )
		wsrep_unload ( pProvider );
}

void JsonLoadConfig ( const CSphConfigSection & hSearchd )
{
	g_sLogFile = hSearchd.GetStr ( "log", "" );

	StringBuilder_c sIncomingAddrs ( "," );
	int iCountEmpty = 0;
	int iCountApi = 0;
	
	// node with empty incoming addresses works as GARB - does not affect FC
	// might hung on pushing 1500 transactions
	for ( const CSphVariant * pOpt = hSearchd("listen"); pOpt; pOpt = pOpt->m_pNext )
	{
		const CSphString & sListen = pOpt->strval();
		// check for valid address of API protocol but not local or port only
		ListenerDesc_t tListen = ParseListener ( sListen.cstr() );
		if ( tListen.m_eProto!=PROTO_SPHINX )
			continue;

		iCountApi++;
		if ( tListen.m_uIP==0 )
		{
			iCountEmpty++;
			continue;
		}

		sIncomingAddrs += sListen.cstr();
	}

	if ( sIncomingAddrs.IsEmpty() )
	{
		if ( iCountApi && iCountApi==iCountEmpty )
			sphWarning ( "all listen have empty address, can not set incoming addresses, replication disabled" );
		else
			sphWarning ( "no listen found, can not set incoming addresses, replication disabled" );
		return;
	}
	g_sIncomingAddr = sIncomingAddrs.cstr();

	g_sDataDir = hSearchd.GetStr ( "data_dir", "" );
	if ( g_sDataDir.IsEmpty() )
		return;

	CSphString sError;
	// check data_dir exists and available
	if ( !CheckPath ( g_sDataDir, true, sError ) )
	{
		sphWarning ( "%s, replication disabled", sError.cstr() );
		g_sDataDir = "";
		return;
	}

	g_sConfigPath.SetSprintf ( "%s/manticoresearch.json", g_sDataDir.cstr() );
	if ( !JsonConfigRead ( g_sConfigPath, g_dCfgClusters, g_dCfgIndexes, sError ) )
		sphDie ( "failed to use JSON config, %s", sError.cstr() );
}

void JsonDoneConfig()
{
	CSphString sError;
	if ( !g_sDataDir.IsEmpty() && ( g_bCfgHasData || g_hClusters.GetLength()>0 ) )
	{
		CSphVector<IndexDesc_t> dIndexes;
		GetLocalIndexes ( dIndexes );
		if ( !JsonConfigWrite ( g_sConfigPath, g_hClusters, dIndexes, sError ) )
			sphLogFatal ( "%s", sError.cstr() );
	}
}

void JsonConfigConfigureAndPreload ( int & iValidIndexes, int & iCounter )
{
	for ( const IndexDesc_t & tIndex : g_dCfgIndexes )
	{
		CSphConfigSection hIndex;
		hIndex.Add ( CSphVariant ( tIndex.m_sPath.cstr(), 0 ), "path" );
		hIndex.Add ( CSphVariant ( GetTypeName ( tIndex.m_eType ).cstr(), 0 ), "type" );

		ESphAddIndex eAdd = ConfigureAndPreload ( hIndex, tIndex.m_sName.cstr(), true );
		iValidIndexes += ( eAdd!=ADD_ERROR ? 1 : 0 );
		iCounter += ( eAdd==ADD_DSBLED ? 1 : 0 );
		if ( eAdd==ADD_ERROR )
			sphWarning ( "index '%s': removed from JSON config", tIndex.m_sName.cstr() );
		else
			g_bCfgHasData = true;
	}
}

void ReplicateClustersStatus ( VectorLike & dStatus )
{
	ScRL_t tLock ( g_tClustersLock );
	void * pIt = nullptr;
	while ( g_hClusters.IterateNext ( &pIt ) )
		ReplicateClusterStats ( g_hClusters.IterateGet ( &pIt ), dStatus );
}

static bool CheckLocalIndex ( const CSphString & sIndex, CSphString & sError )
{
	ServedIndexRefPtr_c pServed = GetServed ( sIndex );
	if ( !pServed )
	{
		sError.SetSprintf ( "unknown index '%s'", sIndex.cstr() );
		return false;
	}

	ServedDescRPtr_c pDesc ( pServed );
	if ( pDesc->m_eType!=IndexType_e::PERCOLATE )
	{
		sError.SetSprintf ( "wrong type of index '%s'", sIndex.cstr() );
		return false;
	}

	return true;
}

static bool SetIndexCluster ( const CSphString & sIndex, const CSphString & sCluster, CSphString & sError )
{
	ServedIndexRefPtr_c pServed = GetServed ( sIndex );
	if ( !pServed )
	{
		sError.SetSprintf ( "unknown index '%s'", sIndex.cstr() );
		return false;
	}

	bool bSetCluster = false;
	{
		ServedDescRPtr_c pDesc ( pServed );
		if ( pDesc->m_eType!=IndexType_e::PERCOLATE )
		{
			sError.SetSprintf ( "wrong type of index '%s'", sIndex.cstr() );
			return false;
		}
		bSetCluster = ( pDesc->m_sCluster!=sCluster );
	}

	if ( bSetCluster )
	{
		ServedDescWPtr_c pDesc ( pServed );
		pDesc->m_sCluster = sCluster;
	}

	return true;
}

bool ParseCmdReplicated ( const BYTE * pData, int iLen, bool bIsolated, const CSphString & sCluster, ReplicationCommand_t & tCmd )
{
	MemoryReader_c tReader ( pData, iLen );

	ReplicationCommand_e eCommand = (ReplicationCommand_e)tReader.GetWord();
	if ( eCommand<0 || eCommand>RCOMMAND_TOTAL )
	{
		sphWarning ( "replication bad command %d", (int)eCommand );
		return false;
	}

	WORD uVer = tReader.GetWord();
	if ( uVer!=g_iReplicateCommandVer )
	{
		sphWarning ( "replication command %d, version mismatch %d, got %d", (int)eCommand, g_iReplicateCommandVer, (int)uVer );
		return false;
	}

	CSphString sIndex = tReader.GetString();
	const BYTE * pRequest = pData + tReader.GetPos();
	int iRequestLen = iLen - tReader.GetPos();

	tCmd.m_eCommand = eCommand;
	tCmd.m_sCluster = sCluster;
	tCmd.m_sIndex = sIndex;
	tCmd.m_bIsolated = bIsolated;

	switch ( eCommand )
	{
	case RCOMMAND_PQUERY_ADD:
	{
		ServedIndexRefPtr_c pServed = GetServed ( sIndex );
		if ( !pServed )
		{
			sphWarning ( "replicate unknown index '%s', command %d", sIndex.cstr(), (int)eCommand );
			return false;
		}

		ServedDescRPtr_c pDesc ( pServed );
		if ( pDesc->m_eType!=IndexType_e::PERCOLATE )
		{
			sphWarning ( "replicate wrong type of index '%s', command %d", sIndex.cstr(), (int)eCommand );
			return false;
		}

		PercolateIndex_i * pIndex = (PercolateIndex_i * )pDesc->m_pIndex;

		LoadStoredQuery ( pRequest, iRequestLen, tCmd.m_tPQ );
		sphLogDebugRpl ( "pq-add, index '%s', uid " UINT64_FMT " query %s", tCmd.m_sIndex.cstr(), tCmd.m_tPQ.m_uQUID, tCmd.m_tPQ.m_sQuery.cstr() );

		CSphString sError;
		PercolateQueryArgs_t tArgs ( tCmd.m_tPQ );
		tArgs.m_bReplace = true;
		tCmd.m_pStored = pIndex->Query ( tArgs, sError );

		if ( !tCmd.m_pStored )
		{
			sphWarning ( "replicate pq-add error '%s', index '%s'", sError.cstr(), tCmd.m_sIndex.cstr() );
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

	case RCOMMAND_CLUSTER_ALTER_ADD:
		tCmd.m_bCheckIndex = false;
		sphLogDebugRpl ( "pq-cluster-alter-add, index '%s'", tCmd.m_sIndex.cstr() );
		break;

	case RCOMMAND_CLUSTER_ALTER_DROP:
		sphLogDebugRpl ( "pq-cluster-alter-drop, index '%s'", tCmd.m_sIndex.cstr() );
		break;

	default:
		sphWarning ( "replicate unsupported command %d", (int)eCommand );
		return false;
	}

	return true;
}


bool HandleCmdReplicated ( ReplicationCommand_t & tCmd )
{
	CSphString sError;

	bool bCmdCluster = ( tCmd.m_eCommand==RCOMMAND_CLUSTER_ALTER_ADD || tCmd.m_eCommand==RCOMMAND_CLUSTER_ALTER_DROP );
	if ( bCmdCluster )
	{
		if ( tCmd.m_eCommand==RCOMMAND_CLUSTER_ALTER_ADD && !CheckLocalIndex ( tCmd.m_sIndex, sError ) )
		{
			sphWarning ( "replicate %s, command %d", sError.cstr(), (int)tCmd.m_eCommand );
			return false;
		}

		CommitMonitor_c tCommit ( tCmd, nullptr );
		bool bOk = tCommit.CommitTOI ( sError );
		if ( !bOk )
			sphWarning ( "%s", sError.cstr() );
		return bOk;
	}

	ServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sIndex );
	if ( !pServed )
	{
		sphWarning ( "replicate unknown index '%s', command %d", tCmd.m_sIndex.cstr(), (int)tCmd.m_eCommand );
		return false;
	}

	ServedDescPtr_c pDesc ( pServed, ( tCmd.m_eCommand==RCOMMAND_TRUNCATE ) );
	if ( pDesc->m_eType!=IndexType_e::PERCOLATE )
	{
		sphWarning ( "replicate wrong type of index '%s', command %d", tCmd.m_sIndex.cstr(), (int)tCmd.m_eCommand );
		return false;
	}

	PercolateIndex_i * pIndex = (PercolateIndex_i * )pDesc->m_pIndex;

	switch ( tCmd.m_eCommand )
	{
	case RCOMMAND_PQUERY_ADD:
	{
		sphLogDebugRpl ( "pq-add-commit, index '%s', uid " INT64_FMT, tCmd.m_sIndex.cstr(), tCmd.m_tPQ.m_uQUID );

		bool bOk = pIndex->CommitPercolate ( tCmd.m_pStored, sError );
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
		if ( !pIndex->Truncate ( sError ) )
			sphWarning ( "%s", sError.cstr() );
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

	if ( tCmd.m_sCluster.IsEmpty() )
		return tMonitor.Commit ( sError );

	g_tClustersLock.ReadLock();
	ReplicationCluster_t ** ppCluster = g_hClusters ( tCmd.m_sCluster );
	g_tClustersLock.Unlock();
	if ( !ppCluster )
	{
		sError.SetSprintf ( "unknown cluster %s", tCmd.m_sCluster.cstr() );
		return false;
	}

	if ( tCmd.m_eCommand==RCOMMAND_TRUNCATE && tCmd.m_bReconfigure )
	{
		sError.SetSprintf ( "RECONFIGURE not supported in cluster index" );
		return false;
	}

	const uint64_t uIndexHash = sphFNV64 ( tCmd.m_sIndex.cstr() );
	if ( tCmd.m_bCheckIndex && !(*ppCluster)->m_dIndexHashes.BinarySearch ( uIndexHash ) )
	{
		sError.SetSprintf ( "index '%s' not belongs to cluster '%s'", tCmd.m_sIndex.cstr(), tCmd.m_sCluster.cstr() );
		return false;
	}

	// alter add index should be valid
	if ( tCmd.m_eCommand==RCOMMAND_CLUSTER_ALTER_ADD && !CheckLocalIndex ( tCmd.m_sIndex, sError ) )
		return false;

	assert ( (*ppCluster)->m_pProvider );

	// replicator check CRC of data - no need to check that at our side
	CSphVector<BYTE> dBuf;
	uint64_t uQueryHash = uIndexHash;
	uQueryHash = sphFNV64 ( &tCmd.m_eCommand, sizeof(tCmd.m_eCommand), uQueryHash );

	// scope for writer
	{
		MemoryWriter_c tWriter ( dBuf );
		tWriter.PutWord ( tCmd.m_eCommand );
		tWriter.PutWord ( g_iReplicateCommandVer );
		tWriter.PutString ( tCmd.m_sIndex );
	}

	bool bTOI = false;
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

	case RCOMMAND_CLUSTER_ALTER_ADD:
	case RCOMMAND_CLUSTER_ALTER_DROP:
		bTOI = true;
		break;

	default:
		sError.SetSprintf ( "unknown command %d", tCmd.m_eCommand );
		return false;
	}

	if ( !bTOI )
		return Replicate ( uQueryHash, dBuf, (*ppCluster)->m_pProvider, tMonitor, sError );
	else
		return ReplicateTOI ( uQueryHash, dBuf, (*ppCluster)->m_pProvider, tMonitor, sError );
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
	if ( m_tCmd.m_eCommand!=RCOMMAND_TRUNCATE && pDesc->m_eType!=IndexType_e::PERCOLATE )
	{
		sError = "requires an existing Percolate index";
		return false;
	}

	bool bOk = false;
	PercolateIndex_i * pIndex = (PercolateIndex_i * )pDesc->m_pIndex;

	switch ( m_tCmd.m_eCommand )
	{
	case RCOMMAND_PQUERY_ADD:
		bOk = pIndex->CommitPercolate ( m_tCmd.m_pStored, sError );
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

bool CommitMonitor_c::CommitTOI ( CSphString & sError )
{
	ScWL_t tLock (g_tClustersLock ); // FIXME!!! no need to lock as all cluster operation serialized with TOI mode
	ReplicationCluster_t ** ppCluster = g_hClusters ( m_tCmd.m_sCluster );
	if ( !ppCluster )
	{
		sError.SetSprintf ( "unknown cluster %s", m_tCmd.m_sCluster.cstr() );
		return false;
	}

	ReplicationCluster_t * pCluster = *ppCluster;
	const uint64_t uIndexHash = sphFNV64 ( m_tCmd.m_sIndex.cstr() );
	if ( m_tCmd.m_bCheckIndex && !pCluster->m_dIndexHashes.BinarySearch ( uIndexHash ) )
	{
		sError.SetSprintf ( "index '%s' not belongs to cluster '%s'", m_tCmd.m_sIndex.cstr(), m_tCmd.m_sCluster.cstr() );
		return false;
	}

	switch ( m_tCmd.m_eCommand )
	{
	case RCOMMAND_CLUSTER_ALTER_ADD:
		if ( !SetIndexCluster ( m_tCmd.m_sIndex, pCluster->m_sName, sError ) )
			return false;
		pCluster->m_dIndexes.Add ( m_tCmd.m_sIndex );
		pCluster->m_dIndexes.Uniq();
		pCluster->UpdateIndexHashes();
		break;

	case RCOMMAND_CLUSTER_ALTER_DROP:
		if ( !SetIndexCluster ( m_tCmd.m_sIndex, CSphString(), sError ) )
			return false;
		pCluster->m_dIndexes.RemoveValue ( m_tCmd.m_sIndex );
		pCluster->UpdateIndexHashes();
		break;

	default:
		sError.SetSprintf ( "unknown command %d", m_tCmd.m_eCommand );
		return false;
	}

	return true;
}


bool JsonLoadIndexDesc ( const JsonObj_c & tJson, CSphString & sError, IndexDesc_t & tIndex )
{
	CSphString sName = tJson.Name();
	if ( sName.IsEmpty() )
	{
		sError = "empty index name";
		return false;
	}

	if ( !tJson.FetchStrItem ( tIndex.m_sPath, "path", sError ) )
		return false;

	CSphString sType;
	if ( !tJson.FetchStrItem ( sType, "type", sError ) )
		return false;

	tIndex.m_eType = TypeOfIndexConfig ( sType );
	if ( tIndex.m_eType==IndexType_e::ERROR_ )
	{
		sError.SetSprintf ( "type '%s' is invalid", sType.cstr() );
		return false;
	}

	tIndex.m_sName = sName;

	return true;
}

static const CSphString g_sDefaultReplicationURI = "gcomm://";

bool JsonConfigRead ( const CSphString & sConfigPath, CSphVector<ClusterDesc_t> & dClusters, CSphVector<IndexDesc_t> & dIndexes, CSphString & sError )
{
	if ( !sphIsReadable ( sConfigPath, nullptr ) )
		return true;

	CSphAutoreader tConfigFile;
	if ( !tConfigFile.Open ( sConfigPath, sError ) )
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
	JsonObj_c tRoot ( (const char*)dData.Begin() );
	if ( tRoot.GetError ( (const char *)dData.Begin(), dData.GetLength(), sError ) )
		return false;

	// FIXME!!! check for path duplicates
	// check indexes
	JsonObj_c tIndexes = tRoot.GetItem("indexes");
	for ( const auto & i : tIndexes )
	{
		IndexDesc_t tIndex;
		if ( !JsonLoadIndexDesc ( i, sError, tIndex ) )
		{
			sphWarning ( "index '%s'(%d) error: %s", i.Name(), dIndexes.GetLength(), sError.cstr() );
			return false;
		}

		dIndexes.Add ( tIndex );
	}

	// check clusters
	JsonObj_c tClusters = tRoot.GetItem("clusters");
	int iCluster = 0;
	for ( const auto & i : tClusters )
	{
		ClusterDesc_t tCluster;

		bool bGood = true;
		tCluster.m_sName = i.Name();
		if ( tCluster.m_sName.IsEmpty() )
		{
			sError = "empty cluster name";
			bGood = false;
		}

		// optional items such as: options and skipSst
		if ( !i.FetchStrItem ( tCluster.m_sOptions, "options", sError, true ) )
			sphWarning ( "cluster '%s'(%d): %s", tCluster.m_sName.cstr(), iCluster, sError.cstr() );

		tCluster.m_sURI = g_sDefaultReplicationURI;
		bGood &= i.FetchStrItem ( tCluster.m_sURI, "uri", sError, true );
		bGood &= i.FetchStrItem ( tCluster.m_sPath, "path", sError, true );
		// must have cluster desc
		bGood &= i.FetchStrItem ( tCluster.m_sListen, "listen", sError, false );

		// set indexes prior replication init
		JsonObj_c tIndexes = i.GetItem("indexes");
		int iItem = 0;
		for ( const auto & j : tIndexes )
		{
			if ( j.IsStr() )
				tCluster.m_dIndexes.Add ( j.StrVal() );
			else
				sphWarning ( "cluster '%s' index %d name should be a string, skipped", tCluster.m_sName.cstr(), iItem );

			iItem++;
		}

		if ( !bGood )
		{
			sphWarning ( "cluster '%s'(%d): removed from JSON config, %s", tCluster.m_sName.cstr(), iCluster, sError.cstr() );
			sError = "";
		} else
			dClusters.Add ( tCluster );

		iCluster++;
	}

	return true;
}


bool JsonConfigWrite ( const CSphString & sConfigPath, const SmallStringHash_T<ReplicationCluster_t *> & hClusters, const CSphVector<IndexDesc_t> & dIndexes, CSphString & sError )
{
	StringBuilder_c sConfigData;
	sConfigData += "{\n\"clusters\":\n";
	JsonConfigDumpClusters ( hClusters, sConfigData );
	sConfigData += ",\n\"indexes\":\n";
	JsonConfigDumpIndexes ( dIndexes, sConfigData );
	sConfigData += "\n}\n";

	CSphString sNew, sOld;
	CSphString sCur = sConfigPath;
	sNew.SetSprintf ( "%s.new", sCur.cstr() );
	sOld.SetSprintf ( "%s.old", sCur.cstr() );

	CSphWriter tConfigFile;
	if ( !tConfigFile.OpenFile ( sNew, sError ) )
		return false;

	tConfigFile.PutBytes ( sConfigData.cstr(), sConfigData.GetLength() );
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

static bool SaveConfig()
{
	CSphVector<IndexDesc_t> dJsonIndexes;
	CSphString sError;
	GetLocalIndexes ( dJsonIndexes );
	{
		ScRL_t tLock ( g_tClustersLock );
		if ( !JsonConfigWrite ( g_sConfigPath, g_hClusters, dJsonIndexes, sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			return false;
		}
	}

	return true;
}

void JsonConfigSetIndex ( const IndexDesc_t & tIndex, JsonObj_c & tIndexes )
{
	JsonObj_c tIdx;
	tIdx.AddStr ( "path", tIndex.m_sPath );
	tIdx.AddStr ( "type", GetTypeName ( tIndex.m_eType ) );

	tIndexes.AddItem ( tIndex.m_sName.cstr(), tIdx );
}

void JsonConfigDumpClusters ( const SmallStringHash_T<ReplicationCluster_t *> & hClusters, StringBuilder_c & tOut )
{
	JsonObj_c tClusters;

	void * pIt = nullptr;
	while ( hClusters.IterateNext ( &pIt ) )
	{
		const ReplicationCluster_t * pCluster = hClusters.IterateGet ( &pIt );

		JsonObj_c tItem;
		tItem.AddStr ( "path", pCluster->m_sPath );
		tItem.AddStr ( "listen", pCluster->m_sListen );
		tItem.AddStr ( "uri", pCluster->m_sURI );
		tItem.AddStr ( "options", pCluster->m_sOptions );

		// index array
		JsonObj_c tIndexes ( true );
		for ( const CSphString & sIndex : pCluster->m_dIndexes )
		{
			JsonObj_c tStrItem = JsonObj_c::CreateStr ( sIndex );
			tIndexes.AddItem ( tStrItem );
		}
		tItem.AddItem ( "indexes", tIndexes );
		tClusters.AddItem ( pCluster->m_sName.cstr(), tItem );
	}

	tOut += tClusters.AsString(true).cstr();
}

void JsonConfigDumpIndexes ( const CSphVector<IndexDesc_t> & dIndexes, StringBuilder_c & tOut )
{
	JsonObj_c tIndexes;
	for ( const auto & i : dIndexes )
		JsonConfigSetIndex ( i, tIndexes );

	tOut += tIndexes.AsString(true).cstr();
}


void GetLocalIndexes ( CSphVector<IndexDesc_t> & dIndexes )
{
	for ( RLockedServedIt_c tIt ( g_pLocalIndexes ); tIt.Next (); )
	{
		auto pServed = tIt.Get();
		if ( !pServed )
			continue;

		ServedDescRPtr_c tDesc ( pServed );
		if ( !tDesc->m_bJson )
			continue;

		IndexDesc_t & tIndex = dIndexes.Add();
		tIndex.m_sName = tIt.GetName();
		tIndex.m_sPath = tDesc->m_sIndexPath;
		tIndex.m_eType = tDesc->m_eType;
	}
}

bool LoadIndex ( const CSphConfigSection & hIndex, const CSphString & sIndexName, const CSphString & sCluster )
{
	// delete existed index first, does not allow to save it's data that breaks sync'ed index files
	// need a scope for RefRtr's to work
	bool bJson = true;
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
				bJson = pDesc->m_bJson;
			}

			g_pLocalIndexes->Delete ( sIndexName );
		}
	}

	ESphAddIndex eAdd = AddIndex ( sIndexName.cstr(), hIndex );
	if ( eAdd!=ADD_DSBLED )
		return false;

	ServedIndexRefPtr_c pServed = GetDisabled ( sIndexName );
	ServedDescWPtr_c pDesc ( pServed );
	pDesc->m_bJson = bJson;
	pDesc->m_sCluster = sCluster;

	bool bPreload = PreallocNewIndex ( *pDesc, &hIndex, sIndexName.cstr() );
	if ( !bPreload )
		return false;

	// finally add the index to the hash of enabled.
	g_pLocalIndexes->AddOrReplace ( pServed, sIndexName );
	pServed->AddRef ();
	g_pDisabledIndexes->Delete ( sIndexName );

	return true;
}

static bool ReplicatedIndexes ( const CSphFixedVector<CSphString> & dIndexes, const CSphString & sCluster, CSphString & sError )
{
	assert ( !g_sDataDir.IsEmpty() );

	if ( !g_hClusters.GetLength() )
	{
		sError = "no clusters found";
		return false;
	}

	// scope for check of cluster data
	{
		SmallStringHash_T<int> hIndexes;
		for ( const CSphString & sIndex : dIndexes )
		{
			if ( !CheckLocalIndex ( sIndex, sError ) )
				return false;

			hIndexes.Add ( 1, sIndex );
		}

		ScRL_t tLock ( g_tClustersLock );
		ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "unknown cluster %s", sCluster.cstr() );
			return false;
		}

		const ReplicationCluster_t * pCluster = *ppCluster;
		// indexes should be new or from same cluster
		void * pIt = nullptr;
		while ( g_hClusters.IterateNext ( &pIt ) )
		{
			const ReplicationCluster_t * pOrigCluster = g_hClusters.IterateGet ( &pIt );
			if ( pOrigCluster==pCluster )
				continue;

			for ( const CSphString & sIndex : pOrigCluster->m_dIndexes )
			{
				if ( hIndexes.Exists ( sIndex ) )
				{
					sError.SetSprintf ( "index '%s' is already in another cluster '%s', replicated cluster '%s'", sIndex.cstr(), pOrigCluster->m_sName.cstr(), sCluster.cstr() );
					return false;
				}
			}
		}
	}

	for ( const CSphString & sIndex : dIndexes )
	{
		if ( !SetIndexCluster ( sIndex, sCluster, sError ) )
			return false;
	}

	// scope for modify cluster data
	{
		ScWL_t tLock ( g_tClustersLock );
		// should be already in cluster list
		ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "unknown cluster %s", sCluster.cstr() );
			return false;
		}

		ReplicationCluster_t * pCluster = *ppCluster;
		pCluster->m_dIndexes.Resize ( dIndexes.GetLength() );
		ARRAY_FOREACH ( i, dIndexes )
			pCluster->m_dIndexes[i] = dIndexes[i];
		pCluster->UpdateIndexHashes();
	}

	SaveConfig();

	return true;
}

CSphString GetClusterPath ( const CSphString & sPath )
{
	if ( sPath.IsEmpty() )
		return g_sDataDir;

	CSphString sFullPath;
	sFullPath.SetSprintf ( "%s/%s", g_sDataDir.cstr(), sPath.cstr() );

	return sFullPath;
}

static bool GetClusterPath ( const CSphString & sCluster, CSphString & sClusterPath, CSphString & sError )
{
	ScRL_t tLock ( g_tClustersLock );
	ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
	if ( !ppCluster )
	{
		sError.SetSprintf ( "unknown cluster '%s'", sCluster.cstr() );
		return false;
	}

	sClusterPath = GetClusterPath ( (*ppCluster)->m_sPath );
	return true;
}

bool CheckPath ( const CSphString & sPath, bool bCheckWrite, CSphString & sError, const char * sCheckFileName )
{
	if ( !sphIsReadable ( sPath, &sError ) )
	{
		sError.SetSprintf ( "can not access directory %s, %s", sPath.cstr(), sError.cstr() );
		return false;
	}

	if ( bCheckWrite )
	{
		CSphString sTmp;
		sTmp.SetSprintf ( "%s/%s", sPath.cstr(), sCheckFileName );
		CSphAutofile tFile ( sTmp, SPH_O_NEW, sError, true );
		if ( tFile.GetFD()<0 )
		{
			sError.SetSprintf ( "directory %s write error: %s", sPath.cstr(), sError.cstr() );
			return false;
		}
	}

	return true;
}

static bool ClusterCheckPath ( const CSphString & sPath, const CSphString & sName, bool bCheckWrite, CSphString & sError )
{
	if ( g_sDataDir.IsEmpty() )
	{
		sError.SetSprintf ( "data_dir option is missed, replication disabled" );
		return false;
	}

	CSphString sFullPath = GetClusterPath ( sPath );
	if ( !CheckPath ( sFullPath, bCheckWrite, sError ) )
	{
		sError.SetSprintf ( "cluster '%s', %s", sName.cstr(), sError.cstr() );
		return false;
	}

	ScRL_t tClusterRLock ( g_tClustersLock );
	void * pIt = nullptr;
	while ( g_hClusters.IterateNext ( &pIt ) )
	{
		const ReplicationCluster_t * pCluster = g_hClusters.IterateGet ( &pIt );
		if ( sPath==pCluster->m_sPath )
		{
			sError.SetSprintf ( "duplicate paths, cluster '%s' has the same path as '%s'", sName.cstr(), pCluster->m_sName.cstr() );
			return false;
		}
	}

	return true;
}

static bool NewClusterForce ( const CSphString & sPath, CSphString & sError )
{
	CSphString sClusterState;
	sClusterState.SetSprintf ( "%s/grastate.dat", sPath.cstr() );
	CSphString sNewState;
	sNewState.SetSprintf ( "%s/grastate.dat.new", sPath.cstr() );
	CSphString sOldState;
	sOldState.SetSprintf ( "%s/grastate.dat.old", sPath.cstr() );
	const char sPattern[] = "safe_to_bootstrap";
	const int iPatternLen = sizeof(sPattern)-1;

	// cluster starts well without grastate.dat file
	if ( !sphIsReadable ( sClusterState ) )
		return true;

	CSphAutoreader tReader;
	if ( !tReader.Open ( sClusterState, sError ) )
		return false;

	CSphWriter tWriter;
	if ( !tWriter.OpenFile ( sNewState, sError ) )
		return false;

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
		sError = tReader.GetErrorMessage();
		return false;
	}
	if ( tWriter.IsError() )
		return false;

	tReader.Close();
	tWriter.CloseFile();

	if ( sph::rename ( sClusterState.cstr (), sOldState.cstr () )!=0 )
	{
		sError.SetSprintf ( "failed to rename %s to %s", sClusterState.cstr(), sOldState.cstr() );
		return false;
	}

	if ( sph::rename ( sNewState.cstr (), sClusterState.cstr () )!=0 )
	{
		sError.SetSprintf ( "failed to rename %s to %s", sNewState.cstr(), sClusterState.cstr() );
		return false;
	}

	::unlink ( sOldState.cstr() );
	return true;
}

const char * g_sLibFiles[] = { "grastate.dat", "galera.cache" };

static void NewClusterClean ( const CSphString & sPath )
{
	for ( const char * sFile : g_sLibFiles )
	{
		CSphString sName;
		sName.SetSprintf ( "%s/%s", sPath.cstr(), sFile );

		::unlink ( sName.cstr() );
	}
}

void GetNodeAddress ( const ClusterDesc_t & tCluster, CSphString & sAddr )
{
	sAddr.SetSprintf ( "%s,%s:replication", g_sIncomingAddr.cstr(), tCluster.m_sListen.cstr() );
}

bool ReplicationStart ( const CSphConfigSection & hSearchd, bool bNewCluster, bool bForce )
{
	if ( !GetReplicationDL() )
	{
		// warning only in case clusters declared
		if ( g_dCfgClusters.GetLength() )
			sphWarning ( "got cluster but provider not set, replication disabled" );
		return false;
	}

	if ( g_sDataDir.IsEmpty() )
	{
		sphWarning ( "data_dir option is missed, replication disabled" );
		return false;
	}

	if ( g_sIncomingAddr.IsEmpty() )
	{
		sphWarning ( "incoming addresses not set, replication disabled" );
		return false;
	}

	CSphString sError;

	for ( const ClusterDesc_t & tDesc : g_dCfgClusters )
	{
		ReplicationArgs_t tArgs;
		// global options
		tArgs.m_bNewCluster = ( bNewCluster || bForce );
		GetNodeAddress ( tDesc, tArgs.m_sIncomingAdresses );

		CSphScopedPtr<ReplicationCluster_t> pElem ( new ReplicationCluster_t );
		pElem->m_sURI = tDesc.m_sURI;
		pElem->m_sName = tDesc.m_sName;
		pElem->m_sListen = tDesc.m_sListen;
		pElem->m_sPath = tDesc.m_sPath;
		pElem->m_sOptions = tDesc.m_sOptions;

		// check listen IP address
		ListenerDesc_t tListen = ParseListener ( pElem->m_sListen.cstr() );
		char sListenBuf [ SPH_ADDRESS_SIZE ];
		sphFormatIP ( sListenBuf, sizeof(sListenBuf), tListen.m_uIP );
		if ( tListen.m_uIP==0 )
		{
			sphWarning ( "cluster '%s' listen empty address (%s), skipped", pElem->m_sName.cstr(), sListenBuf );
			continue;
		}

		bool bUriEmpty = ( pElem->m_sURI==g_sDefaultReplicationURI );
		if ( bUriEmpty )
		{
			if ( !tArgs.m_bNewCluster )
				sphWarning ( "uri property is empty, force new cluster '%s'", pElem->m_sName.cstr() );
			tArgs.m_bNewCluster = true;
		}

		// check cluster path is unique
		if ( !ClusterCheckPath ( pElem->m_sPath, pElem->m_sName, false, sError ) )
		{
			sphWarning ( "%s, skipped", sError.cstr() );
			continue;
		}

		CSphString sClusterPath = GetClusterPath ( pElem->m_sPath );

		// set safe_to_bootstrap to 1 into grastate.dat file at cluster path
		if ( ( bForce || bUriEmpty ) && !NewClusterForce ( sClusterPath, sError ) )
		{
			sphWarning ( "%s, cluster '%s' skipped", sError.cstr(), pElem->m_sName.cstr() );
			continue;
		}

		// check indexes valid
		for ( const CSphString & sIndexName : tDesc.m_dIndexes )
		{
			if ( !SetIndexCluster ( sIndexName, pElem->m_sName, sError ) )
			{
				sphWarning ( "%s, removed from cluster %s", sError.cstr(), pElem->m_sName.cstr() );
				continue;
			}
			pElem->m_dIndexes.Add ( sIndexName );
		}
		pElem->UpdateIndexHashes();

		tArgs.m_pCluster = pElem.Ptr();

		CSphString sError;
		if ( !ReplicateClusterInit ( tArgs, sError ) )
			sphDie ( "%s", sError.cstr() );

		assert ( pElem->m_pProvider );
		g_hClusters.Add ( pElem.LeakPtr(), tDesc.m_sName );
	}
	return true;
}

static bool CheckClusterOption ( const SmallStringHash_T<SqlInsert_t *> & hValues, const char * sName, bool bOptional, CSphString & sVal, CSphString & sError )
{
	SqlInsert_t ** ppVal = hValues ( sName );
	if ( !ppVal )
	{
		if ( !bOptional )
			sError.SetSprintf ( "'%s' not set", sName );
		return bOptional;
	}

	if ( (*ppVal)->m_sVal.IsEmpty() )
	{
		sError.SetSprintf ( "'%s' should have string value", sName );
		return false;
	}

	sVal = (*ppVal)->m_sVal;

	return true;
}

static bool CheckClusterStatement ( const CSphString & sCluster, bool bCheckCluster, CSphString & sError )
{
	if ( !GetReplicationDL() )
	{
		sError = "provider not set, recompile with cmake option WITH_REPLICATION=1";
		return false;
	}

	if ( g_sIncomingAddr.IsEmpty() )
	{
		sError = "incoming addresses not set";
		return false;
	}

	if ( !bCheckCluster )
		return true;

	g_tClustersLock.ReadLock();
	const bool bClusterExists = ( g_hClusters ( sCluster )!=nullptr );
	g_tClustersLock.Unlock();
	if ( bClusterExists )
	{
		sError.SetSprintf ( "cluster '%s' already exists", sCluster.cstr() );
		return false;
	}

	return true;
}

static bool CheckClusterStatement ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, CSphString & sError, CSphScopedPtr<ReplicationCluster_t> & pElem )
{
	if ( !CheckClusterStatement ( sCluster, true, sError ) )
		return false;

	SmallStringHash_T<SqlInsert_t *> hValues;
	assert ( dNames.GetLength()==dValues.GetLength() );
	ARRAY_FOREACH ( i, dNames )
		hValues.Add ( dValues.Begin() + i, dNames[i] );

	CSphString sListen;
	if ( !CheckClusterOption ( hValues, "listen", false, sListen, sError ) )
		return false;

	pElem = new ReplicationCluster_t;
	pElem->m_sName = sCluster;
	pElem->m_sListen = sListen;

	// optional items
	if ( !CheckClusterOption ( hValues, "uri", true, pElem->m_sURI, sError ) )
		return false;
	if ( !CheckClusterOption ( hValues, "path", true, pElem->m_sPath, sError ) )
		return false;
	if ( !CheckClusterOption ( hValues, "options", true, pElem->m_sOptions, sError ) )
		return false;

	// check cluster path is unique
	bool bValidPath = ClusterCheckPath ( pElem->m_sPath, pElem->m_sName, true, sError );
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

	return true;
}

static bool WaitClusterState ( const CSphString & sCluster, ClusterState_e eState, CSphString & sError )
{
	while ( true )
	{
		ScRL_t tLock ( g_tClustersLock );

		ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "no cluster '%s' found", sCluster.cstr() );
			return false;
		}

		if ( (*ppCluster)->m_eState==WSREP_MEMBER_SYNCED )
			break;

		sphSleepMsec ( 100 );
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// cluster join to existed
/////////////////////////////////////////////////////////////////////////////

bool ClusterJoin ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, CSphString & sError )
{
	CSphScopedPtr<ReplicationCluster_t> pElem ( nullptr );
	if ( !CheckClusterStatement ( sCluster, dNames, dValues, sError, pElem ) )
		return false;

	// uri should start with gcomm://
	if ( !pElem->m_sURI.Begins ( "gcomm://" ) )
		pElem->m_sURI.SetSprintf ( "gcomm://%s", pElem->m_sURI.cstr() );

	ReplicationArgs_t tArgs;
	// global options
	tArgs.m_bNewCluster = false;
	tArgs.m_bJoin = true;
	GetNodeAddress ( *pElem.Ptr(), tArgs.m_sIncomingAdresses );
	tArgs.m_pCluster = pElem.Ptr();

	// need to clean up Galera system files left from previous cluster
	CSphString sClusterPath = GetClusterPath ( pElem->m_sPath );
	NewClusterClean ( sClusterPath );

	{
		ScWL_t tLock ( g_tClustersLock );
		g_hClusters.Add ( pElem.LeakPtr(), sCluster );
	}

	if ( !ReplicateClusterInit ( tArgs, sError ) )
	{
		ScWL_t tLock ( g_tClustersLock );
		g_hClusters.Delete ( sCluster );
		return false;
	}

	return WaitClusterState ( sCluster, WSREP_MEMBER_SYNCED, sError );
}

/////////////////////////////////////////////////////////////////////////////
// cluster create new without nodes
/////////////////////////////////////////////////////////////////////////////

bool ClusterCreate ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, CSphString & sError )
{
	CSphScopedPtr<ReplicationCluster_t> pElem ( nullptr );
	if ( !CheckClusterStatement ( sCluster, dNames, dValues, sError, pElem ) )
		return false;

	// uri should start with gcomm://
	pElem->m_sURI.SetSprintf ( "gcomm://" );

	ReplicationArgs_t tArgs;
	// global options
	tArgs.m_bNewCluster = true;
	tArgs.m_bJoin = false;
	GetNodeAddress ( *pElem.Ptr(), tArgs.m_sIncomingAdresses );
	tArgs.m_pCluster = pElem.Ptr();

	// need to clean up Galera system files left from previous cluster
	CSphString sClusterPath = GetClusterPath ( pElem->m_sPath );
	NewClusterClean ( sClusterPath );

	{
		ScWL_t tLock ( g_tClustersLock );
		g_hClusters.Add ( pElem.LeakPtr(), sCluster );
	}
	
	if ( !ReplicateClusterInit ( tArgs, sError ) )
	{
		ScWL_t tLock ( g_tClustersLock );
		g_hClusters.Delete ( sCluster );
		return false;
	}


	return WaitClusterState ( sCluster, WSREP_MEMBER_SYNCED, sError );
}

/////////////////////////////////////////////////////////////////////////////
// cluster delete
/////////////////////////////////////////////////////////////////////////////


enum PQRemoteCommand_e : WORD
{
	CLUSTER_DELETE				= 0,
	CLUSTER_FILE_RESERVE		= 1,
	CLUSTER_FILE_SIZE			= 2,
	CLUSTER_FILE_SEND			= 3,
	CLUSTER_INDEX_ADD_LOCAL		= 4,
	CLUSTER_SYNCED				= 5,

	PQR_COMMAND_TOTAL,
	PQR_COMMAND_WRONG = PQR_COMMAND_TOTAL,
};

struct PQRemoteReply_t
{
	CSphString	m_sIndexPath;		// index path without extension (with or without path due to bClusterIndex)

	CSphString	m_sFileHash;			// sha1 of index file
	int64_t		m_iIndexFileSize = 0;	// size of index file
};


struct PQRemoteData_t : public PQRemoteReply_t
{
	CSphString	m_sIndex;			// local name of index
	CSphString	m_sFileName;		// index file name with extension (with or without path due to bClusterIndex)

	// file send args
	const BYTE * m_pSendBuff = nullptr;
	int			m_iSendSize = 0;
	int64_t		m_iFileOff = 0;

	CSphString	m_sCluster;			// cluster name of index

	IndexType_e	m_eIndex = IndexType_e::ERROR_;


	// SST
	CSphString	m_sGTID;							// GTID received
	CSphFixedVector<CSphString> m_dIndexes { 0 };	// index list received
};

struct PQRemoteAgentData_t : public iQueryResult
{
	const PQRemoteData_t & m_tReq;
	PQRemoteReply_t m_tReply;

	explicit PQRemoteAgentData_t ( const PQRemoteData_t & tReq )
		: m_tReq ( tReq )
	{
	}

	void Reset() override {}
	bool HasWarnings() const override { return false; }
};

struct VecAgentDesc_t : public ISphNoncopyable, public CSphVector<AgentDesc_t *>
{
	~VecAgentDesc_t ()
	{
		for ( int i=0; i<m_iCount; i++ )
			SafeDelete ( m_pData[i] );
	}
};

static void GetNodes ( const CSphString & sNodes, VecAgentDesc_t & dNodes )
{
	if ( sNodes.IsEmpty () )
		return;

	CSphString sTmp = sNodes;
	const char * sCur = sTmp.cstr();
	while ( *sCur )
	{
		char * sAddrs = (char *)sCur;
		while ( *sCur && *sCur!=';' )
			++sCur;

		const char * sAddrsStart = sAddrs;
		const char * sAddrsEnd = sCur;

		// skip the delimiter
		if ( *sCur )
			++sCur;

		int iNodes = dNodes.GetLength();
		while ( sAddrs<sAddrsEnd )
		{
			const char * sListen = sAddrs;
			while ( sAddrs<sAddrsEnd && *sAddrs!=',' )
				++sAddrs;

			// replace delimiter with 0 for ParseListener and skip that delimiter
			*sAddrs = '\0';
			sAddrs++;

			ListenerDesc_t tListen = ParseListener ( sListen );

			if ( tListen.m_eProto!=PROTO_SPHINX )
				continue;

			if ( tListen.m_uIP==0 )
				continue;

			char sAddrBuf [ SPH_ADDRESS_SIZE ];
			sphFormatIP ( sAddrBuf, sizeof(sAddrBuf), tListen.m_uIP );

			AgentDesc_t * pDesc = new AgentDesc_t;
			dNodes.Add( pDesc );
			pDesc->m_sAddr = sAddrBuf;
			pDesc->m_uAddr = tListen.m_uIP;
			pDesc->m_iPort = tListen.m_iPort;
			pDesc->m_bNeedResolve = false;
			pDesc->m_bPersistent = false;
			pDesc->m_iFamily = AF_INET;

			// need only first address, for multiple address need implement MultiAgent interface
			break;
		}

		if ( iNodes==dNodes.GetLength() )
			sphWarning ( "node '%.*s' without API address, remote command skipped", (int)( sAddrsEnd-sAddrsStart ), sAddrsStart );
	}
}

static AgentConn_t * CreateAgent ( const AgentDesc_t & tDesc, const PQRemoteData_t & tReq )
{
	AgentConn_t * pAgent = new AgentConn_t;
	pAgent->m_tDesc.CloneFrom ( tDesc );

	HostDesc_t tHost;
	pAgent->m_tDesc.m_pDash = new HostDashboard_t ( tHost );

	pAgent->m_iMyConnectTimeout = g_iRemoteTimeout;
	pAgent->m_iMyQueryTimeout = g_iRemoteTimeout;

	pAgent->m_pResult = new PQRemoteAgentData_t ( tReq );

	return pAgent;
}

static void GetNodes ( const CSphString & sNodes, VecRefPtrs_t<AgentConn_t *> & dNodes, const PQRemoteData_t & tReq )
{
	VecAgentDesc_t dDesc;
	GetNodes ( sNodes, dDesc );

	dNodes.Resize ( dDesc.GetLength() );
	ARRAY_FOREACH ( i, dDesc )
		dNodes[i] = CreateAgent ( *dDesc[i], tReq );
}

static void GetNodes ( const VecAgentDesc_t & dDesc, VecRefPtrs_t<AgentConn_t *> & dNodes, const PQRemoteData_t & tReq )
{
	dNodes.Resize ( dDesc.GetLength() );
	ARRAY_FOREACH ( i, dDesc )
		dNodes[i] = CreateAgent ( *dDesc[i], tReq );
}


struct PQRemoteBase_t : public IRequestBuilder_t, public IReplyParser_t
{
	explicit PQRemoteBase_t ( PQRemoteCommand_e eCmd )
		: m_eCmd ( eCmd )
	{
	}

	void BuildRequest ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const final
	{
		// API header
		APICommand_t tWr { tOut, SEARCHD_COMMAND_CLUSTERPQ, VER_COMMAND_CLUSTERPQ };
		tOut.SendWord ( m_eCmd );
		BuildCommand ( tAgent, tOut );
	}

	static PQRemoteReply_t & GetRes ( const AgentConn_t & tAgent )
	{
		PQRemoteAgentData_t * pData = (PQRemoteAgentData_t *)tAgent.m_pResult.Ptr();
		assert ( pData );
		return pData->m_tReply;
	}
	
	static const PQRemoteData_t & GetReq ( const AgentConn_t & tAgent )
	{
		const PQRemoteAgentData_t * pData = (PQRemoteAgentData_t *)tAgent.m_pResult.Ptr();
		assert ( pData );
		return pData->m_tReq;
	}

private:
	virtual void BuildCommand ( const AgentConn_t &, CachedOutputBuffer_c & tOut ) const = 0;
	const PQRemoteCommand_e m_eCmd = PQR_COMMAND_WRONG;
};

struct PQRemoteDelete_t : public PQRemoteBase_t
{
	PQRemoteDelete_t()
		: PQRemoteBase_t ( CLUSTER_DELETE )
	{
	}

	void BuildCommand ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const final
	{
		const PQRemoteData_t & tCmd = GetReq ( tAgent );
		tOut.SendString ( tCmd.m_sCluster.cstr() );
	}

	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
	{
		tCmd.m_sCluster = tBuf.GetString();
	}

	static void BuildReply ( const PQRemoteReply_t & tRes, CachedOutputBuffer_c & tOut )
	{
		APICommand_t tReply ( tOut, SEARCHD_OK );
		tOut.SendByte ( 1 );
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final
	{
		// just payload as can not recv reply with 0 size
		tReq.GetByte();
		return !tReq.GetError();
	}
};

struct PQRemoteFileReserve_t : public PQRemoteBase_t
{
	PQRemoteFileReserve_t ()
		: PQRemoteBase_t ( CLUSTER_FILE_RESERVE )
	{
	}

	void BuildCommand ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const final
	{
		const PQRemoteData_t & tCmd = GetReq ( tAgent );
		tOut.SendString ( tCmd.m_sCluster.cstr() );
		tOut.SendString ( tCmd.m_sIndex.cstr() );
		tOut.SendString ( tCmd.m_sFileName.cstr() );
		tOut.SendUint64 ( tCmd.m_iIndexFileSize );
		tOut.SendString ( tCmd.m_sFileHash.cstr() );
	}

	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
	{
		tCmd.m_sCluster = tBuf.GetString();
		tCmd.m_sIndex = tBuf.GetString();
		tCmd.m_sFileName = tBuf.GetString();
		tCmd.m_iIndexFileSize = tBuf.GetUint64();
		tCmd.m_sFileHash = tBuf.GetString();
	}

	static void BuildReply ( const PQRemoteReply_t & tRes, CachedOutputBuffer_c & tOut )
	{
		APICommand_t tReply ( tOut, SEARCHD_OK );
		tOut.SendUint64 ( tRes.m_iIndexFileSize );
		tOut.SendString ( tRes.m_sFileHash.cstr() );
		tOut.SendString ( tRes.m_sIndexPath.cstr() );
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		PQRemoteReply_t & tRes = GetRes ( tAgent );
		tRes.m_iIndexFileSize = (int64_t)tReq.GetUint64();
		tRes.m_sFileHash = tReq.GetString();
		tRes.m_sIndexPath = tReq.GetString();

		return !tReq.GetError();
	}
};


struct PQRemoteFileSend_t : public PQRemoteBase_t
{
	PQRemoteFileSend_t ()
		: PQRemoteBase_t ( CLUSTER_FILE_SEND )
	{
	}

	void BuildCommand ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const final
	{
		const PQRemoteData_t & tCmd = GetReq ( tAgent );
		tOut.SendString ( tCmd.m_sCluster.cstr() );
		tOut.SendString ( tCmd.m_sFileName.cstr() );
		tOut.SendUint64 ( tCmd.m_iFileOff );
		tOut.SendInt ( tCmd.m_iSendSize );
		tOut.SendBytes ( tCmd.m_pSendBuff, tCmd.m_iSendSize );
	}

	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
	{
		tCmd.m_sCluster = tBuf.GetString();
		tCmd.m_sFileName = tBuf.GetString();
		tCmd.m_iFileOff = tBuf.GetUint64();
		tCmd.m_iSendSize = tBuf.GetInt();
		tBuf.GetBytesZerocopy ( &tCmd.m_pSendBuff, tCmd.m_iSendSize );
	}

	static void BuildReply ( const PQRemoteReply_t & tRes, CachedOutputBuffer_c & tOut )
	{
		APICommand_t tReply ( tOut, SEARCHD_OK );
		tOut.SendUint64 ( tRes.m_iIndexFileSize );
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		PQRemoteReply_t & tRes = GetRes ( tAgent );
		tRes.m_iIndexFileSize = (int64_t)tReq.GetUint64();

		return !tReq.GetError();
	}
};

struct PQRemoteIndexAdd_t : public PQRemoteBase_t
{
	PQRemoteIndexAdd_t ()
		: PQRemoteBase_t ( CLUSTER_INDEX_ADD_LOCAL )
	{
	}

	void BuildCommand ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const final
	{
		const PQRemoteData_t & tCmd = GetReq ( tAgent );
		tOut.SendString ( tCmd.m_sCluster.cstr() );
		tOut.SendString ( tCmd.m_sIndex.cstr() );
		tOut.SendString ( tCmd.m_sIndexPath.cstr() );
		tOut.SendByte ( (BYTE)IndexType_e::PERCOLATE );
		tOut.SendUint64 ( tCmd.m_iIndexFileSize );
		tOut.SendString ( tCmd.m_sFileHash.cstr() );
	}

	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
	{
		tCmd.m_sCluster = tBuf.GetString();
		tCmd.m_sIndex = tBuf.GetString();
		tCmd.m_sIndexPath = tBuf.GetString();
		tCmd.m_eIndex = (IndexType_e)tBuf.GetByte();
		tCmd.m_iIndexFileSize = tBuf.GetUint64();
		tCmd.m_sFileHash = tBuf.GetString();
	}

	static void BuildReply ( const PQRemoteReply_t & tRes, CachedOutputBuffer_c & tOut )
	{
		APICommand_t tReply ( tOut, SEARCHD_OK );
		tOut.SendUint64 ( tRes.m_iIndexFileSize );
		tOut.SendString ( tRes.m_sFileHash.cstr() );
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		PQRemoteReply_t & tRes = GetRes ( tAgent );
		tRes.m_iIndexFileSize = (int64_t)tReq.GetUint64();
		tRes.m_sFileHash = tReq.GetString();

		return !tReq.GetError();
	}
};


struct PQRemoteSynced_t : public PQRemoteBase_t
{
	PQRemoteSynced_t();
	void BuildCommand ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const final;
	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd );
	static void BuildReply ( const PQRemoteReply_t & tRes, CachedOutputBuffer_c & tOut );
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final;
};


static bool PerformRemoteTasks ( VectorAgentConn_t & dNodes, IRequestBuilder_t & tReq, IReplyParser_t & tReply, CSphString & sError )
{
	int iNodes = dNodes.GetLength();
	int iFinished = PerformRemoteTasks ( dNodes, &tReq, &tReply );

	if ( iFinished!=iNodes )
	{
		sphLogDebugRpl ( "%d(%d) nodes finished well", iFinished, iNodes );

		for ( const AgentConn_t * pAgent : dNodes )
		{
			if ( !pAgent->m_sFailure.IsEmpty() )
			{
				sphWarning ( "'%s:%d': %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pAgent->m_sFailure.cstr() );
				sError.SetSprintf ( "%s'%s:%d': %s", ( sError.IsEmpty() ? "" : ";" ), pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pAgent->m_sFailure.cstr() );
			}
		}
	}

	return ( iFinished==iNodes );
}

void HandleCommandClusterPq ( CachedOutputBuffer_c & tOut, WORD uCommandVer, InputBuffer_c & tBuf, const char * sClient )
{
	if ( !CheckCommandVersion ( uCommandVer, VER_COMMAND_CLUSTERPQ, tOut ) )
		return;

	PQRemoteCommand_e eClusterCmd = (PQRemoteCommand_e)tBuf.GetWord();
	sphLogDebugRpl ( "remote cluster command %d, client %s", (int)eClusterCmd, sClient );

	CSphString sError;
	PQRemoteData_t tCmd;
	PQRemoteReply_t tRes;
	bool bOk = false;
	switch ( eClusterCmd )
	{
		case CLUSTER_DELETE:
		{
			PQRemoteDelete_t::ParseCommand ( tBuf, tCmd );
			bOk = RemoteClusterDelete ( tCmd.m_sCluster, sError );
			SaveConfig();
			if ( bOk )
				PQRemoteDelete_t::BuildReply ( tRes, tOut );
		}
		break;

		case CLUSTER_FILE_RESERVE:
		{
			PQRemoteFileReserve_t::ParseCommand ( tBuf, tCmd );
			bOk = RemoteFileReserve ( tCmd, tRes, sError );
			if ( bOk )
				PQRemoteFileReserve_t::BuildReply ( tRes, tOut );
		}
		break;

		case CLUSTER_FILE_SEND:
		{
			PQRemoteFileSend_t::ParseCommand ( tBuf, tCmd );
			bOk = RemoteFileStore ( tCmd, tRes, sError );
			if ( bOk )
				PQRemoteFileSend_t::BuildReply ( tRes, tOut );
		}
		break;

		case CLUSTER_INDEX_ADD_LOCAL:
		{
			PQRemoteIndexAdd_t::ParseCommand ( tBuf, tCmd );
			bOk = RemoteLoadIndex ( tCmd, tRes, sError );
			if ( bOk )
				PQRemoteIndexAdd_t::BuildReply ( tRes, tOut );
		}
		break;

		case CLUSTER_SYNCED:
		{
			PQRemoteSynced_t::ParseCommand ( tBuf, tCmd );
			bOk = RemoteClusterSynced ( tCmd, sError );
			if ( bOk )
				PQRemoteSynced_t::BuildReply ( tRes, tOut );
		}
		break;

		default: assert ( 0 && "INTERNAL ERROR: unhandled command" ); break;
	}

	if ( !bOk )
	{
		APICommand_t tReply ( tOut, SEARCHD_ERROR );
		tOut.SendString ( sError.cstr() );
		sphLogFatal ( "%s", sError.cstr() );
	}
}

bool RemoteClusterDelete ( const CSphString & sCluster, CSphString & sError )
{
	// erase cluster from all hashes
	ReplicationCluster_t * pCluster = nullptr;
	{
		ScWL_t tLock ( g_tClustersLock );

		ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "unknown cluster '%s' ", sCluster.cstr() );
			return false;
		}

		pCluster = *ppCluster;
		// remove cluster from cache without delete of cluster itself
		g_hClusters.AddUnique ( sCluster ) = nullptr;
		g_hClusters.Delete ( sCluster );

		for ( const CSphString & sIndex : pCluster->m_dIndexes )
		{
			if ( !SetIndexCluster ( sIndex, CSphString(), sError ) )
				return false;
		}
		pCluster->m_dIndexes.Reset();
		pCluster->m_dIndexHashes.Reset();
	}

	ReplicateClusterDone ( pCluster );
	SafeDelete ( pCluster );

	return true;
}

// cluster delete every node then itself
bool ClusterDelete ( const CSphString & sCluster, CSphString & sError, CSphString & sWarning )
{
	if ( !CheckClusterStatement ( sCluster, false, sError ) )
		return false;

	CSphString sNodes;
	{
		ScRL_t tLock ( g_tClustersLock );
		ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "unknown cluster '%s'", sCluster.cstr() );
			return false;
		}

		// copy nodes with lock as change of cluster view would change node list string
		{
			ScRL_t tNodesLock ( (*ppCluster)->m_tNodesLock );
			sNodes = (*ppCluster)->m_sNodes;
		}
	}

	if ( !sNodes.IsEmpty() )
	{
		PQRemoteData_t tCmd;
		tCmd.m_sCluster = sCluster;
		VecRefPtrs_t<AgentConn_t *> dNodes;
		GetNodes ( sNodes, dNodes, tCmd );
		PQRemoteDelete_t tReq;
		if ( !PerformRemoteTasks ( dNodes, tReq, tReq, sError ) )
			return false;
	}

	// after all nodes finished
	bool bOk = RemoteClusterDelete ( sCluster, sError );
	SaveConfig();

	return bOk;
}

void ReplicationCluster_t::UpdateIndexHashes()
{
	m_dIndexHashes.Resize ( 0 );
	for ( const CSphString & sIndex : m_dIndexes )
		m_dIndexHashes.Add ( sphFNV64 ( sIndex.cstr() ) );

	m_dIndexHashes.Uniq();
}

static bool ClusterAlterDrop ( const CSphString & sCluster, const CSphString & sIndex, CSphString & sError )
{
	ReplicationCommand_t tDropCmd;
	tDropCmd.m_eCommand = RCOMMAND_CLUSTER_ALTER_DROP;
	tDropCmd.m_sCluster = sCluster;
	tDropCmd.m_sIndex = sIndex;

	return HandleCmdReplicate ( tDropCmd, sError, nullptr );
}

bool RemoteFileReserve ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError )
{
	CSphString sFilePath;
	bool bGotIndex = false;

	// use index path first
	{
		ServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sIndex );
		ServedDescRPtr_c pDesc ( pServed );
		if ( pDesc && pDesc->IsMutable() )
		{
			bGotIndex = true;
			sFilePath.SetSprintf ( "%s.meta", pDesc->m_sIndexPath.cstr() );
			tRes.m_sIndexPath = pDesc->m_sIndexPath;
		}
	}

	// use cluster path as head of index path
	if ( !bGotIndex )
	{
		CSphString sClusterPath;
		if ( !GetClusterPath ( tCmd.m_sCluster, sClusterPath, sError ) )
			return false;

		sFilePath.SetSprintf ( "%s/%s.meta", sClusterPath.cstr(), tCmd.m_sFileName.cstr() );
		tRes.m_sIndexPath.SetSprintf ( "%s/%s", sClusterPath.cstr(), tCmd.m_sFileName.cstr() );
	}

	// check that index file matched to reference file
	// but only in case index loaded into daemon
	// as there is no path to not sync file but load index afters
	while ( bGotIndex )
	{
		CSphAutofile tFile ( sFilePath, SPH_O_READ, sError, false );
		if ( tFile.GetFD()<0 )
			break;

		tRes.m_iIndexFileSize = tFile.GetSize();
		if ( tRes.m_iIndexFileSize!=tCmd.m_iIndexFileSize )
			break;

		if ( !CalcSHA1 ( sFilePath, tRes.m_sFileHash, sError ) )
		{
			tRes.m_sFileHash = "";
			sphWarning ( "%s", sError.cstr() );
			break;
		}

		// no need to send index file
		if ( tRes.m_sFileHash==tCmd.m_sFileHash )
			return true;

		break;
	}

	// check that could create file with specific size
	CSphAutofile tOut ( sFilePath, SPH_O_NEW, sError, false );
	if ( tOut.GetFD()<0 )
		return false;

	tRes.m_iIndexFileSize = sphSeek ( tOut.GetFD(), tCmd.m_iIndexFileSize, SEEK_SET );
	if ( tCmd.m_iIndexFileSize!=tRes.m_iIndexFileSize )
	{
		if ( tRes.m_iIndexFileSize<0 )
		{
			sError.SetSprintf ( "error: %d '%s'", errno, strerrorm ( errno ) );
		} else
		{
			sError.SetSprintf ( "error, expected: " INT64_FMT ", got " INT64_FMT, tCmd.m_iIndexFileSize, tRes.m_iIndexFileSize );
		}

		return false;
	}
	return true;
}

bool RemoteFileStore ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError )
{
	CSphAutofile tOut ( tCmd.m_sFileName, O_RDWR, sError, false );
	if ( tOut.GetFD()<0 )
		return false;

	int64_t iPos = sphSeek ( tOut.GetFD(), tCmd.m_iFileOff, SEEK_SET );
	if ( tCmd.m_iFileOff!=iPos )
	{
		if ( iPos<0 )
		{
			sError.SetSprintf ( "error %s", strerrorm ( errno ) );
		} else
		{
			tRes.m_iIndexFileSize = iPos; // reply what disk space available
			sError.SetSprintf ( "error, expected: " INT64_FMT ", got " INT64_FMT, tCmd.m_iFileOff, iPos );
		}
		return false;
	}

	if ( !sphWrite ( tOut.GetFD(), tCmd.m_pSendBuff, tCmd.m_iSendSize ) )
	{
		sError.SetSprintf ( "write error: %s", strerrorm(errno) );
		return false;
	}

	tRes.m_iIndexFileSize = iPos + tCmd.m_iSendSize;
	return true;
}

bool RemoteLoadIndex ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError )
{
	CSphString sType = GetTypeName ( tCmd.m_eIndex );
	if ( tCmd.m_eIndex!=IndexType_e::PERCOLATE )
	{
		sError.SetSprintf ( "unsupported index '%s' type %s", tCmd.m_sIndex.cstr(), sType.cstr() );
		return false;
	}

	// check that size matched and sha1 matched prior to loading index
	{
		CSphString sIndexFile;
		sIndexFile.SetSprintf ( "%s.meta", tCmd.m_sIndexPath.cstr() );
		CSphAutofile tFile ( sIndexFile, SPH_O_READ, sError, false );
		if ( tFile.GetFD()<0 )
			return false;

		tRes.m_iIndexFileSize = tFile.GetSize();
		if ( tRes.m_iIndexFileSize!=tCmd.m_iIndexFileSize )
		{
			sError.SetSprintf ( "size " INT64_FMT " not equal to stored size " INT64_FMT, tCmd.m_iIndexFileSize, tRes.m_iIndexFileSize );
			return false;
		}

		if ( !CalcSHA1 ( sIndexFile, tRes.m_sFileHash, sError ) )
			return false;

		if ( tRes.m_sFileHash!=tCmd.m_sFileHash )
		{
			sError.SetSprintf ( "sha1 %s not equal to stored sha1 %s", tCmd.m_sFileHash.cstr(), tRes.m_sFileHash.cstr() );
			return false;
		}
	}

	sphLogDebugRpl ( "loading index '%s' from %s", tCmd.m_sIndex.cstr(), tCmd.m_sIndexPath.cstr() );

	CSphConfigSection hIndex;
	hIndex.Add ( CSphVariant ( tCmd.m_sIndexPath.cstr(), 0 ), "path" );
	hIndex.Add ( CSphVariant ( sType.cstr(), 0 ), "type" );
	if ( !LoadIndex ( hIndex, tCmd.m_sIndex, tCmd.m_sCluster ) )
	{
		sError.SetSprintf ( "failed to load index '%s'", tCmd.m_sIndex.cstr() );
		return false;
	}

	return true;
}

struct RemoteFileState_t
{
	CSphString m_sRemoteIndexPath;
	CSphString m_sRemoteFilePath;
	const AgentDesc_t * m_pAgentDesc = nullptr;
};

struct FileReader_t
{
	CSphAutofile m_tFile;
	PQRemoteData_t m_tArgs;
	const AgentDesc_t * m_pAgentDesc = nullptr;
};

static bool SendFile ( const CSphVector<RemoteFileState_t> & dDesc, const CSphString & sFileIn, int64_t iFileSize, const CSphString & sCluster, CSphString & sError )
{
	// setup buffers
	const int64_t iReaderBufSize = Min ( iFileSize, g_iMaxPacketSize * 3 / 4 );
	CSphFixedVector<BYTE> dReadBuf ( iReaderBufSize * dDesc.GetLength() );

	CSphFixedVector<FileReader_t> dReaders  ( dDesc.GetLength() );
	ARRAY_FOREACH ( i, dReaders )
	{
		// setup file readers
		FileReader_t & tReader = dReaders[i];
		tReader.m_tArgs.m_sCluster = sCluster;
		tReader.m_tArgs.m_sFileName = dDesc[i].m_sRemoteFilePath;
		tReader.m_pAgentDesc = dDesc[i].m_pAgentDesc;

		BYTE * pBuf = dReadBuf.Begin() + iReaderBufSize * i;
		tReader.m_tArgs.m_pSendBuff = pBuf;
		tReader.m_tArgs.m_iSendSize = iReaderBufSize;

		if ( tReader.m_tFile.Open ( sFileIn, SPH_O_READ, sError, false )<0 )
			return false;

		// initial send is whole buffer or whole file whichever is less
		if ( !tReader.m_tFile.Read ( pBuf, iReaderBufSize, sError ) )
			return false;
	}

	// create agents
	VecRefPtrs_t<AgentConn_t *> dNodes;
	dNodes.Resize ( dReaders.GetLength() );
	ARRAY_FOREACH ( i, dReaders )
		dNodes[i] = CreateAgent ( *dReaders[i].m_pAgentDesc, dReaders[i].m_tArgs );

	// submit initial jobs
	CSphRefcountedPtr<IRemoteAgentsObserver> tReporter ( GetObserver() );
	PQRemoteFileSend_t tReq;
	ScheduleDistrJobs ( dNodes, &tReq, &tReq, tReporter );

	StringBuilder_c tErrors;
	bool bDone = false;
	VecRefPtrs_t<AgentConn_t *> dNewNode;
	dNewNode.Add ( nullptr );

	while ( !bDone )
	{
		// don't forget to check incoming replies after send was over
		bDone = tReporter->IsDone();
		// wait one or more remote queries to complete
		if ( !bDone )
			tReporter->WaitChanges();

		ARRAY_FOREACH ( iAgent, dNodes )
		{
			AgentConn_t * pAgent = dNodes[iAgent];
			if ( !pAgent->m_bSuccess )
				continue;

			FileReader_t & tReader = dReaders[iAgent];
			const PQRemoteReply_t & tRes = PQRemoteBase_t::GetRes ( *pAgent );

			// report errors first
			if ( tRes.m_iIndexFileSize!=tReader.m_tArgs.m_iFileOff+tReader.m_tArgs.m_iSendSize || !pAgent->m_sFailure.IsEmpty() )
			{
				if ( !tErrors.IsEmpty() )
					tErrors += "; ";

				tErrors.Appendf ( "'%s:%d'", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort );

				if ( tRes.m_iIndexFileSize!=tReader.m_tArgs.m_iFileOff+tReader.m_tArgs.m_iSendSize )
					tErrors.Appendf ( "wrong file part stored, expected " INT64_FMT " stored " INT64_FMT,
						tReader.m_tArgs.m_iFileOff+tReader.m_tArgs.m_iSendSize, tRes.m_iIndexFileSize );

				if ( !pAgent->m_sFailure.IsEmpty() )
					tErrors += pAgent->m_sFailure.cstr();

				pAgent->m_bSuccess = false;
				continue;
			}

			tReader.m_tArgs.m_iFileOff += tReader.m_tArgs.m_iSendSize;
			int64_t iSize = iFileSize - tReader.m_tArgs.m_iFileOff;

			// check that file fully sent 
			if ( !iSize )
			{
				pAgent->m_bSuccess = false;
				continue;
			}

			// clump file tail to buffer
			if ( iSize>iReaderBufSize )
				iSize = iReaderBufSize;
			tReader.m_tArgs.m_iSendSize = iSize;

			if ( !tReader.m_tFile.Read ( const_cast<BYTE* >( tReader.m_tArgs.m_pSendBuff ), iSize, sError ) )
			{
				if ( !tErrors.IsEmpty() )
					tErrors += "; ";
				tErrors.Appendf ( "'%s:%d' %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, sError.cstr() );
				pAgent->m_bSuccess = false;
				continue;
			}

			// remove agent from main vector
			pAgent->Release();

			AgentConn_t * pNextJob = CreateAgent ( *tReader.m_pAgentDesc, tReader.m_tArgs );
			dNodes[iAgent] = pNextJob;

			dNewNode[0] = pNextJob;
			ScheduleDistrJobs ( dNewNode, &tReq, &tReq, tReporter );
			// agent managed at main vector
			dNewNode[0] = nullptr;

			// reset done flag to process new item
			bDone = false;
		}
	}

	// result got shared and has not owned
	for ( AgentConn_t * pAgent : dNodes )
		pAgent->m_pResult.LeakPtr();

	return true;
}

// check that agents reply with proper index size
static bool CheckReplyIndexState ( const VecRefPtrs_t<AgentConn_t *> & dNodes, int64_t iFileSize, CSphString * pHash, CSphString & sError )
{
	StringBuilder_c sBuf ( "," );
	for ( const AgentConn_t * pAgent : dNodes )
	{
		const PQRemoteReply_t & tRes = PQRemoteBase_t::GetRes ( *pAgent );
		const bool bSameSize = ( tRes.m_iIndexFileSize==iFileSize );
		const bool bSameHash = ( !pHash || ( (*pHash)==tRes.m_sFileHash ) );

		if ( !bSameSize )
		{
			sBuf.Appendf ( "'%s:%d' size " INT64_FMT " not equal to stored size " INT64_FMT, pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, iFileSize, tRes.m_iIndexFileSize );
		} else if ( !bSameHash )
		{
			sBuf.Appendf ( "'%s:%d' sha1 %s not equal to stored sha1 %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pHash->cstr(), tRes.m_sFileHash.cstr() );
		}
	}
	if ( !sBuf.IsEmpty() )
	{
		sphWarning ( "nodes failed to create file: %s", sBuf.cstr() );
		sError.SetSprintf ( "nodes failed to create file: %s", sBuf.cstr() );
		return false;
	}

	return true;
}

static bool NodesReplicateIndex ( const CSphString & sCluster, const CSphString & sIndex, const VecAgentDesc_t & dDesc, CSphString & sError )
{
	assert ( dDesc.GetLength() );

	CSphString sIndexPath;
	CSphString sIndexFile;
	{
		ServedIndexRefPtr_c pServed = GetServed ( sIndex );
		if ( !pServed )
		{
			sError.SetSprintf ( "unknown index '%s'", sIndex.cstr() );
			return false;
		}

		ServedDescRPtr_c pDesc ( pServed );
		if ( pDesc->m_eType!=IndexType_e::PERCOLATE )
		{
			sError.SetSprintf ( "wrong type of index '%s'", sIndex.cstr() );
			return false;
		}

		PercolateIndex_i * pIndex = (PercolateIndex_i *)pDesc->m_pIndex;
		pIndex->ForceRamFlush ( false );
		// FIXME!!! handle other index types
		sIndexPath = pDesc->m_sIndexPath;
		sIndexFile.SetSprintf ( "%s.meta", sIndexPath.cstr() );
	}
	assert ( !sIndexPath.IsEmpty() );

	int64_t iFileSize = 0;
	CSphString sFileHash;
	{
		CSphAutoreader tIndexFile;
		if ( !tIndexFile.Open ( sIndexFile, sError ) )
			return false;
	
		iFileSize = tIndexFile.GetFilesize();
		if ( !CalcSHA1 ( sIndexFile, sFileHash, sError ) )
			return false;
	}

	CSphVector<RemoteFileState_t> dSendStates;
	{
		PQRemoteData_t tAgentData;
		tAgentData.m_sCluster = sCluster;
		tAgentData.m_sIndex = sIndex;
		tAgentData.m_sFileName = GetBaseName ( sIndexPath );
		tAgentData.m_iIndexFileSize = iFileSize;
		tAgentData.m_sFileHash = sFileHash;

		VecRefPtrs_t<AgentConn_t *> dNodes;
		GetNodes ( dDesc, dNodes, tAgentData );
		PQRemoteFileReserve_t tReq;
		bool bOk = PerformRemoteTasks ( dNodes, tReq, tReq, sError );
		if ( !CheckReplyIndexState ( dNodes, iFileSize, nullptr, sError ) || !bOk )
			return false;

		// collect remote file states and make list nodes and files to send
		assert ( dDesc.GetLength()==dNodes.GetLength() );
		ARRAY_FOREACH ( i, dNodes )
		{
			 const PQRemoteReply_t & tRes = PQRemoteBase_t::GetRes ( *dNodes[i] );

			 // no need to send index files to nodes there files matches exactly
			 if ( tRes.m_iIndexFileSize==iFileSize && tRes.m_sFileHash==sFileHash )
				 continue;

			 RemoteFileState_t & tRemoteState = dSendStates.Add();
			 tRemoteState.m_sRemoteIndexPath = tRes.m_sIndexPath;
			 tRemoteState.m_sRemoteFilePath.SetSprintf ( "%s.meta", tRes.m_sIndexPath.cstr() );
			 tRemoteState.m_pAgentDesc = dDesc[i];
		}
	}

	if ( !dSendStates.GetLength() )
		return true;

	if ( !SendFile ( dSendStates, sIndexFile, iFileSize, sCluster, sError ) )
		return false;

	{
		CSphFixedVector<PQRemoteData_t> dAgentData ( dSendStates.GetLength() );
		VecRefPtrs_t<AgentConn_t *> dNodes;
		dNodes.Resize ( dSendStates.GetLength() );
		ARRAY_FOREACH ( i, dSendStates )
		{
			const RemoteFileState_t & tState = dSendStates[i];

			PQRemoteData_t & tAgentData = dAgentData[i];
			tAgentData.m_sCluster = sCluster;
			tAgentData.m_sIndex = sIndex;
			tAgentData.m_sFileHash = sFileHash;
			tAgentData.m_iIndexFileSize = iFileSize;
			tAgentData.m_sIndexPath = tState.m_sRemoteIndexPath;

			dNodes[i] = CreateAgent ( *tState.m_pAgentDesc, tAgentData );
		}

		PQRemoteIndexAdd_t tReq;
		bool bOk = PerformRemoteTasks ( dNodes, tReq, tReq, sError );
		if ( !CheckReplyIndexState ( dNodes, iFileSize, &sFileHash, sError ) || !bOk )
			return false;
	}

	return true;
}

static bool ClusterAlterAdd ( const CSphString & sCluster, const CSphString & sIndex, CSphString & sError, CSphString & sWarning )
{
	CSphString sNodes;
	{
		ScRL_t tLock ( g_tClustersLock );
		ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "unknown cluster '%s'", sCluster.cstr() );
			return false;
		}

		// copy nodes with lock as change of cluster view would change node list string
		{
			ScRL_t tNodesLock ( (*ppCluster)->m_tNodesLock );
			sNodes = (*ppCluster)->m_sNodes;
		}
	}

	// ok for just created cluster (wo nodes) to add existed index
	if ( !sNodes.IsEmpty() )
	{
		VecAgentDesc_t dDesc;
		GetNodes ( sNodes, dDesc );

		if ( !NodesReplicateIndex ( sCluster, sIndex, dDesc, sError ) )
			return false;
	}

	ReplicationCommand_t tAddCmd;
	tAddCmd.m_eCommand = RCOMMAND_CLUSTER_ALTER_ADD;
	tAddCmd.m_sCluster = sCluster;
	tAddCmd.m_sIndex = sIndex;
	tAddCmd.m_bCheckIndex = false;

	return HandleCmdReplicate ( tAddCmd, sError, nullptr );
}


bool ClusterAlter ( const CSphString & sCluster, const CSphString & sIndex, bool bAdd, CSphString & sError, CSphString & sWarning )
{
	{
		ServedIndexRefPtr_c pServed = GetServed ( sIndex );
		if ( !pServed )
		{
			sError.SetSprintf ( "unknown index '%s'", sIndex.cstr() );
			return false;
		}

		ServedDescRPtr_c pDesc ( pServed );
		if ( pDesc->m_eType!=IndexType_e::PERCOLATE )
		{
			sError.SetSprintf ( "wrong type of index '%s'", sIndex.cstr() );
			return false;
		}

		if ( bAdd && !pDesc->m_sCluster.IsEmpty() )
		{
			sError.SetSprintf ( "index already '%s' is part of cluster '%s'", sIndex.cstr(), pDesc->m_sCluster.cstr() );
			return false;
		}
		if ( !bAdd && pDesc->m_sCluster.IsEmpty() )
		{
			sError.SetSprintf ( "index '%s' is not in cluster '%s'", sIndex.cstr(), sCluster.cstr() );
			return false;
		}
	}

	if ( bAdd )
		return ClusterAlterAdd ( sCluster, sIndex, sError, sWarning );
	else
		return ClusterAlterDrop ( sCluster, sIndex, sError );
}

/////////////////////////////////////////////////////////////////////////////
// SST
/////////////////////////////////////////////////////////////////////////////

PQRemoteSynced_t::PQRemoteSynced_t ()
	: PQRemoteBase_t ( CLUSTER_SYNCED )
{
}

void PQRemoteSynced_t::BuildCommand ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const
{
	const PQRemoteData_t & tCmd = GetReq ( tAgent );
	tOut.SendString ( tCmd.m_sCluster.cstr() );
	tOut.SendString ( tCmd.m_sGTID.cstr() );
	tOut.SendInt( tCmd.m_dIndexes.GetLength() );
	for ( const CSphString & sIndex : tCmd.m_dIndexes )
		tOut.SendString ( sIndex.cstr() );
}

void PQRemoteSynced_t::ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
{
	tCmd.m_sCluster = tBuf.GetString();
	tCmd.m_sGTID = tBuf.GetString();
	tCmd.m_dIndexes.Reset ( tBuf.GetInt() );
	ARRAY_FOREACH ( i, tCmd.m_dIndexes )
		tCmd.m_dIndexes[i] = tBuf.GetString();
}

void PQRemoteSynced_t::BuildReply ( const PQRemoteReply_t & tRes, CachedOutputBuffer_c & tOut )
{
	APICommand_t tReply ( tOut, SEARCHD_OK );
	tOut.SendByte ( 1 );
}

bool PQRemoteSynced_t::ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const
{
	// just payload as can not recv reply with 0 size
	tReq.GetByte();
	return !tReq.GetError();
}

static bool SendClusterSynced ( const CSphString & sCluster, const CSphVector<CSphString> & dIndexes, const VecAgentDesc_t & dDesc, const char * sGTID, CSphString & sError )
{
	PQRemoteData_t tAgentData;
	tAgentData.m_sCluster = sCluster;
	tAgentData.m_sGTID = sGTID;
	tAgentData.m_dIndexes.Reset ( dIndexes.GetLength() );
	ARRAY_FOREACH ( i, dIndexes )
		tAgentData.m_dIndexes[i] = dIndexes[i];

	VecRefPtrs_t<AgentConn_t *> dNodes;
	GetNodes ( dDesc, dNodes, tAgentData );
	PQRemoteSynced_t tReq;
	bool bOk = PerformRemoteTasks ( dNodes, tReq, tReq, sError );
	if ( !CheckReplyIndexState ( dNodes, 0, nullptr, sError ) || !bOk )
		return false;

	return true;
}

bool SendClusterIndexes ( const ReplicationCluster_t * pCluster, const CSphString & sNode, bool bBypass, const wsrep_gtid_t & tStateID, CSphString & sError )
{
	VecAgentDesc_t dDesc;
	GetNodes ( sNode, dDesc );
	if ( !dDesc.GetLength() )
	{
		sError.SetSprintf ( "%s invalid node", sNode.cstr() );
		return false;
	}

	char sGTID[WSREP_GTID_STR_LEN];
	Verify ( wsrep_gtid_print ( &tStateID, sGTID, sizeof(sGTID) )>0 );

	if ( bBypass )
		return SendClusterSynced ( pCluster->m_sName, pCluster->m_dIndexes, dDesc, sGTID, sError );

	// keep index list
	CSphFixedVector<CSphString> dIndexes ( 0 );
	{
		ScRL_t tLock ( g_tClustersLock );
		ReplicationCluster_t ** ppCluster = g_hClusters ( pCluster->m_sName );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "unknown cluster '%s'", pCluster->m_sName.cstr() );
			return false;
		}

		dIndexes.Reset ( (*ppCluster)->m_dIndexes.GetLength() );
		ARRAY_FOREACH ( i, dIndexes )
			dIndexes[i] = (*ppCluster)->m_dIndexes[i];
	}

	bool bOk = true;
	for ( const CSphString & sIndex : dIndexes )
	{
		if ( !NodesReplicateIndex ( pCluster->m_sName, sIndex, dDesc, sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			bOk = false;
			break;
		}
	}

	bOk &= SendClusterSynced ( pCluster->m_sName, pCluster->m_dIndexes, dDesc, bOk ? sGTID : "" , sError );
	
	return bOk;
}

bool RemoteClusterSynced ( const PQRemoteData_t & tCmd, CSphString & sError )
{
	sphLogDebugRpl ( "join sync %s, UID %s, indexes %d", tCmd.m_sCluster.cstr(), tCmd.m_sGTID.cstr(), tCmd.m_dIndexes.GetLength() );

	bool bValid = ( !tCmd.m_sGTID.IsEmpty() );
	wsrep_gtid tGtid = WSREP_GTID_UNDEFINED;
	
	int iLen = wsrep_gtid_scan ( tCmd.m_sGTID.cstr(), tCmd.m_sGTID.Length(), &tGtid );
	if ( iLen<0 )
	{
		sError.SetSprintf ( "invalid GTID, %s", tCmd.m_sGTID.cstr() );
		bValid	= false;
	}

	if ( bValid )
		bValid &= ReplicatedIndexes ( tCmd.m_dIndexes, tCmd.m_sCluster, sError );

	{
		ScRL_t tLock ( g_tClustersLock );
		ReplicationCluster_t ** ppCluster = g_hClusters ( tCmd.m_sCluster );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "unknown cluster '%s'", tCmd.m_sCluster.cstr() );
			bValid = false;
		}

		int iRes = 0;
		if ( !bValid )
		{
			tGtid.seqno = WSREP_SEQNO_UNDEFINED;
			iRes = -ECANCELED;
		}

		wsrep_t * pProvider = (*ppCluster)->m_pProvider;
		pProvider->sst_received ( pProvider, &tGtid, nullptr, 0, iRes );
	}

	return bValid;
}

bool CheckIndexCluster ( const CSphString & sIndexName, const ServedDesc_t & tDesc, const CSphString & sStmtCluster, CSphString & sError )
{
	if ( tDesc.m_sCluster==sStmtCluster )
		return true;

	if ( tDesc.m_sCluster.IsEmpty() )
		sError.SetSprintf ( "index '%s' is not in cluster, use %s as ident", sIndexName.cstr(), sIndexName.cstr() );
	else
		sError.SetSprintf ( "index '%s' already is part of cluster '%s', use %s:%s as ident", sIndexName.cstr(), tDesc.m_sCluster.cstr(), tDesc.m_sCluster.cstr(), sIndexName.cstr() );

	return false;
}