//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//


#include "sphinx.h"
#include "sphinxstd.h"
#include "sphinxutils.h"
#include "memio.h"
#include "sphinxpq.h"
#include "searchdreplication.h"
#include "accumulator.h"
#include "fileutils.h"
#include <math.h>

#if !HAVE_WSREP
#include "replication/wsrep_api_stub.h"
// it also populates header guard, so next including of 'normal' wsrep_api will break nothing
#endif

#include "replication/wsrep_api.h"
#include "coroutine.h"

#if !_WIN32
// MAC-specific header
#include <netinet/in.h>
#endif

// global application context for wsrep callbacks

static int64_t GetQueryTimeout ( int64_t iTimeout=0 ); // 2 minutes in msec
// 200 msec is ok as we do not need to any missed nodes in cluster node list
static int			g_iAnyNodesTimeout = 200;
static int			g_iNodeRetry = 3;
static int			g_iNodeRetryWait = 500;


// debug options passed into Galera for our logreplication command line key
static const char * g_sDebugOptions = "debug=on;cert.log_conflicts=yes";
// prefix added for Galera nodes
static const char * g_sDefaultReplicationNodes = "gcomm://";

// cluster state this node sees
enum class ClusterState_e
{
	// stop terminal states
	CLOSED,				// node shut well or not started
	DESTROYED,			// node closed with error
	// transaction states
	JOINING,			// node joining cluster
	DONOR,				// node is donor for another node joining cluster
	// ready terminal state 
	SYNCED				// node works as usual
};


// managed port got back into global ports list
class ScopedPort_c : public ISphNoncopyable
{
	int m_iPort = -1;

public:
	explicit ScopedPort_c ( int iPort ) : m_iPort ( iPort ) {}
	~ScopedPort_c();
	int Get() const { return m_iPort; }
	void Set ( int iPort );
	int Leak();
	void Free();
};


// cluster related data
struct ReplicationCluster_t : public ClusterDesc_t
{
	// replicator
	wsrep_t *	m_pProvider = nullptr;

	// receiver thread
	CSphAutoEvent	m_tWorkerFinished;
	bool			m_bHasWorker = false;

	// nodes at cluster
	CSphString	m_sViewNodes; // raw nodes addresses (API and replication) from whole cluster
	// lock protects access to m_sViewNodes and m_sClusterNodes
	CSphMutex	m_tViewNodesLock;
	// representation of m_dIndexes for binarySearch
	// to quickly validate query to cluster:index
	CSphVector<uint64_t> m_dIndexHashes;
	// Galera port got from global ports list on cluster created
	ScopedPort_c	m_tPort { -1 };

	// lock protects access to m_hOptions
	CSphMutex m_tOptsLock;

	// state variables cached from Galera
	CSphFixedVector<char>	m_dUUID { 0 };
	int						m_iConfID = 0;
	int						m_iStatus = WSREP_VIEW_DISCONNECTED;
	int						m_iSize = 0;
	int						m_iIdx = 0;

	// error that got reported to main thread
	CSphMutex				m_tErrorLock;
	CSphString				m_sError;

	void					UpdateIndexHashes();

	// state of node
	void					SetNodeState ( ClusterState_e eNodeState )
	{
		m_eNodeState = eNodeState;
		m_tStateChanged.SetEvent();
	}
	ClusterState_e			GetNodeState() const { return m_eNodeState; }
	ClusterState_e			WaitReady()
	{
		while ( m_eNodeState==ClusterState_e::CLOSED || m_eNodeState==ClusterState_e::JOINING || m_eNodeState==ClusterState_e::DONOR )
			m_tStateChanged.WaitEvent();

		return m_eNodeState;
	}
	void					SetPrimary ( wsrep_view_status_t eStatus )
	{
		m_iStatus = eStatus;
	}
	bool					IsPrimary() const { return ( m_iStatus==WSREP_VIEW_PRIMARY ); }

private:
	CoroEvent_c m_tStateChanged;
	ClusterState_e m_eNodeState { ClusterState_e::CLOSED };
};


// arguments to replication static functions 
struct ReplicationArgs_t
{
	// cluster description
	ReplicationCluster_t *	m_pCluster = nullptr;
	// to create new cluster or join existed
	bool					m_bNewCluster = false;
	// node address to listen by Galera
	CSphString				m_sListenAddr;
	// node incoming address passed into cluster, IP used from listen API or node_address
	int						m_iListenPort = -1;
	// nodes list to join by Galera
	CSphString				m_sNodes;
};

// interface to pass into static Replicate to issue commit for specific command
class CommitMonitor_c
{
public:
	CommitMonitor_c ( RtAccum_t & tAcc, int * pDeletedCount )
		: m_tAcc ( tAcc )
		, m_pDeletedCount ( pDeletedCount )
	{}
	CommitMonitor_c ( RtAccum_t & tAcc, CSphString * pWarning, int * pUpdated )
		: m_tAcc ( tAcc )
		, m_pWarning ( pWarning )
		, m_pUpdated ( pUpdated )
	{}
	CommitMonitor_c ( RtAccum_t & tAcc, int * pDeletedCount, CSphString * pWarning, int * pUpdated )
		: m_tAcc ( tAcc )
		, m_pDeletedCount ( pDeletedCount )
		, m_pWarning ( pWarning )
		, m_pUpdated ( pUpdated )
	{}
	~CommitMonitor_c();

	// commit for common commands
	bool Commit ( CSphString & sError );
	// commit for Total Order Isolation commands
	bool CommitTOI ( ServedDesc_t * pDesc, CSphString & sError );

	// update with Total Order Isolation
	bool Update ( bool bCluster, CSphString & sError );

private:
	RtAccum_t & m_tAcc;
	int * m_pDeletedCount = nullptr;
	CSphString * m_pWarning = nullptr;
	int * m_pUpdated = nullptr;

	bool CommitNonEmptyCmds ( RtIndex_i* pIndex, const ReplicationCommand_t& tCmd, bool bOnlyTruncate,
		CSphString& sError ) const;
};

// lock protects operations at g_hClusters
static RwLock_t g_tClustersLock;
// cluster list
static SmallStringHash_T<ReplicationCluster_t *> g_hClusters GUARDED_BY ( g_tClustersLock );

// hack for abort callback to invalidate only specific cluster
thread_local ReplicationCluster_t* g_pTlsCluster;

/////////////////////////////////////////////////////////////////////////////
// forward declarations
/////////////////////////////////////////////////////////////////////////////

// abort callback that invalidates specific cluster
static void ReplicationAbort();

/////////////////////////////////////////////////////////////////////////////
// JSON config related functions
/////////////////////////////////////////////////////////////////////////////

// create string by join global data_dir and cluster path 
static CSphString GetClusterPath ( const CSphString & sPath );

/////////////////////////////////////////////////////////////////////////////
// remote commands for cluster and index managements
/////////////////////////////////////////////////////////////////////////////

struct PQRemoteReply_t;
struct PQRemoteData_t;

// command at remote node for CLUSTER_DELETE to delete cluster
static bool RemoteClusterDelete ( const CSphString & sCluster, CSphString & sError );

// command at remote node for CLUSTER_FILE_RESERVE to check
// - file could be allocated on disk at cluster path and reserve disk space for a file
// - or make sure that index has exact same index file, ie sha1 matched
static bool RemoteFileReserve ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError );

// command at remote node for CLUSTER_FILE_SEND to store data into file, data size and file offset defined by sender
static bool RemoteFileStore ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError );

// command at remote node for CLUSTER_INDEX_ADD_LOCAL to check sha1 of index file matched and load index into daemon
static bool RemoteLoadIndex ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError );

// send local indexes to remote nodes via API
static bool SendClusterIndexes ( const ReplicationCluster_t * pCluster, const CSphString & sNode, bool bBypass, const wsrep_gtid_t & tStateID, CSphString & sError );

// callback at remote node for CLUSTER_SYNCED to pick up received indexes then call Galera sst_received
static bool RemoteClusterSynced ( const PQRemoteData_t & tCmd, CSphString & sError );

// callback for Galera apply_cb to parse replicated command
static bool ParseCmdReplicated ( const BYTE * pData, int iLen, bool bIsolated, const CSphString & sCluster, RtAccum_t & tAcc, CSphAttrUpdate& tUpd, CSphQuery & tQuery );

// callback for Galera commit_cb to commit replicated command
static bool HandleCmdReplicated ( RtAccum_t & tAcc );

// command to all remote nodes at cluster to get actual nodes list
static bool ClusterGetNodes ( const CSphString & sClusterNodes, const CSphString & sCluster, const CSphString & sGTID, CSphString & sError, CSphString & sNodes );

// callback at remote node for CLUSTER_GET_NODES to return actual nodes list at cluster
static bool RemoteClusterGetNodes ( const CSphString & sCluster, const CSphString & sGTID, CSphString & sError, CSphString & sNodes );

// utility function to filter nodes list provided at string by specific protocol
static void ClusterFilterNodes ( const CSphString & sSrcNodes, Proto_e eProto, CSphString & sDstNodes );

// callback at remote node for CLUSTER_UPDATE_NODES to update nodes list at cluster from actual nodes list
static bool RemoteClusterUpdateNodes ( const CSphString & sCluster, CSphString * pNodes, CSphString & sError );

static bool IsClusterCommand ( const RtAccum_t & tAcc );

static bool IsUpdateCommand ( const RtAccum_t & tAcc );

static void SaveUpdate ( const CSphAttrUpdate & tUpd, CSphVector<BYTE> & dOut );

static int LoadUpdate ( const BYTE * pBuf, int iLen, CSphAttrUpdate & tUpd );

static void SaveUpdate ( const CSphQuery & tQuery, CSphVector<BYTE> & dOut );

static int LoadUpdate ( const BYTE * pBuf, int iLen, CSphQuery & tQuery );


static bool IsInetAddrFree ( DWORD uAddr, int iPort )
{
	static struct sockaddr_in iaddr;
	memset ( &iaddr, 0, sizeof(iaddr) );
	iaddr.sin_family = AF_INET;
	iaddr.sin_addr.s_addr = uAddr;
	iaddr.sin_port = htons ( (short)iPort );

	int iSock = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( iSock==-1 )
	{
		sphWarning ( "failed to create TCP socket: %s", sphSockError() );
		return false;
	}

	int iRes = bind ( iSock, (struct sockaddr *)&iaddr, sizeof(iaddr) );
	SafeClose ( iSock );

	return ( iRes==0 );
}

static const int g_iDefaultPortBias = 10;
static const int g_iDefaultPortRange = 200;

struct PortsRange_t
{
	int m_iPort = 0;
	int m_iCount = 0;
};

// manage ports pairs for clusters set as Galera replication listener ports range
class FreePortList_c
{
private:
	CSphVector<int>	m_dFree;
	CSphTightVector<PortsRange_t> m_dPorts;
	CSphMutex m_tLock;
	DWORD m_uAddr = 0;

public:
	// set range of ports there is could generate ports pairs
	void AddRange ( const PortsRange_t & tPorts )
	{
		assert ( tPorts.m_iPort && tPorts.m_iCount && ( tPorts.m_iCount%2 )==0 );
		ScopedMutex_t tLock ( m_tLock );
		m_dPorts.Add ( tPorts );
	}

	void AddAddr ( const CSphString & sAddr )
	{
		m_uAddr = sphGetAddress ( sAddr.cstr (), false, false );
	}

	// get next available range of ports for Galera listener
	// first reuse ports pair that was recently released
	// or pair from range set
	int Get ()
	{
		int iPortMin = -1;
		ScopedMutex_t tLock ( m_tLock );
		while ( iPortMin==-1 && ( m_dPorts.GetLength() || m_dFree.GetLength () ) )
		{
			if ( m_dFree.GetLength() )
			{
				iPortMin = m_dFree.Pop();
			} else if ( m_dPorts.GetLength() )
			{
				assert ( m_dPorts.Last().m_iCount>=2 );
				PortsRange_t & tPorts = m_dPorts.Last();
				iPortMin = tPorts.m_iPort;

				tPorts.m_iPort += 2;
				tPorts.m_iCount -= 2;
				if ( !tPorts.m_iCount )
					m_dPorts.Pop();
			}

			if ( IsInetAddrFree ( m_uAddr, iPortMin ) && IsInetAddrFree ( m_uAddr, iPortMin+1 ) )
				break;

			iPortMin = -1;
		}
		return iPortMin;
	}

	// free ports pair and add it to free list
	void Free ( int iPort )
	{
		ScopedMutex_t tLock ( m_tLock );
		m_dFree.Add ( iPort );
	}
};

static bool g_bReplicationEnabled = false;
// incoming address guessed (false) or set via searchd.node_address
static bool g_bHasIncoming = false;
// incoming IP part of address set by searchd.node_address or took from listener
static CSphString g_sIncomingIP;
// incoming address (IP:port from API listener) used for request to this node from other daemons
static CSphString g_sIncomingProto;
// listen IP part of address for Galera
static CSphString g_sListenReplicationIP;
// ports pairs manager
static FreePortList_c g_tPorts;


bool ReplicationIsEnabled()
{
	return g_bReplicationEnabled;
}


void ReplicationSetIncoming ( CSphString sIncoming )
{
	g_sIncomingIP = sIncoming;
	g_bHasIncoming = !g_sIncomingIP.IsEmpty();
}


ScopedPort_c::~ScopedPort_c()
{
	Free();
}

void ScopedPort_c::Free()
{
	if ( m_iPort!=-1 )
		g_tPorts.Free ( m_iPort );
	m_iPort = -1;
}

void ScopedPort_c::Set ( int iPort )
{
	Free();
	m_iPort = iPort;
}

int ScopedPort_c::Leak()
{
	int iPort = m_iPort;
	m_iPort = -1;
	return iPort;
}


// data passed to Galera and used at callbacks
struct ReceiverCtx_t
{
	ReplicationCluster_t *	m_pCluster = nullptr;

	// share of remote commands received between apply and commit callbacks
	RtAccum_t m_tAcc { false };
	CSphAttrUpdate m_tUpdAPI;
	CSphQuery m_tQuery;

	~ReceiverCtx_t() { Cleanup(); }
	void Cleanup();
};

// log strings enum -> strings
static const char * g_sClusterStatus[WSREP_VIEW_MAX] = { "primary", "non-primary", "disconnected" };
static const char * g_sStatusDesc[] = {
 "success",
 "warning",
 "transaction is not known",
 "transaction aborted, server can continue",
 "transaction was victim of brute force abort",
 "data exceeded maximum supported size",
 "error in client connection, must abort",
 "error in node state, must reinit",
 "fatal error, server must abort",
 "transaction was aborted before commencing pre-commit",
 "feature not implemented"
};

static const char * GetNodeState ( ClusterState_e eState )
{
	switch ( eState )
	{
		case ClusterState_e::CLOSED: return "closed";
		case ClusterState_e::DESTROYED: return "destroyed";
		case ClusterState_e::JOINING: return "joining";
		case ClusterState_e::DONOR: return "donor";
		case ClusterState_e::SYNCED: return "synced";

		default: return "undefined";
	}
};

static const char * GetStatus ( wsrep_status_t tStatus )
{
	if ( tStatus>=WSREP_OK && tStatus<=WSREP_NOT_IMPLEMENTED )
		return g_sStatusDesc[tStatus];
	else
		return strerror ( tStatus );
}

// var args wrapper for log callback
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

static bool CheckNoWarning ( const char * sMsg );

// callback for Galera logger_cb to log messages and errors
static void Logger_fn ( wsrep_log_level_t eLevel, const char * sMsg )
{
	// in normal flow need to skip certain messages from Galera but keep messagea in debug replication verbosity level
	// dont want to patch Galera source code
	if ( g_eLogLevel<SPH_LOG_RPL_DEBUG && eLevel==WSREP_LOG_WARN && CheckNoWarning ( sMsg ) )
		return;

	LoggerWrapper ( eLevel, sMsg );
}

// commands version (commands these got replicated via Galera)
// ver 0x104 added docstore from RT index
// ver 0x105 fixed CSphWordHit serialization - instead of direct raw blob copy only fields sent (16 bytes vs 24)
static const WORD g_iReplicateCommandVer = 0x105;

// log debug info about cluster nodes as current nodes views that
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

// check cluster state prior passing write transaction \ commands into cluster
static bool CheckClasterState ( ClusterState_e eState, bool bPrimary, const CSphString & sCluster, CSphString & sError )
{
	if ( !bPrimary )
	{
		sError.SetSprintf ( "cluster '%s' is not ready, not primary state (%s)", sCluster.cstr(), GetNodeState ( eState ) );
		return false;
	}
	if ( eState!=ClusterState_e::SYNCED && eState!=ClusterState_e::DONOR )
	{
		sError.SetSprintf ( "cluster '%s' is not ready, current state is %s", sCluster.cstr(), GetNodeState ( eState ) );
		return false;
	}

	return true;
}

// check cluster state wrapper
static bool CheckClasterState ( const ReplicationCluster_t * pCluster, CSphString & sError )
{
	assert ( pCluster );
	const ClusterState_e eState = pCluster->GetNodeState();
	const bool bPrimary = pCluster->IsPrimary();
	return CheckClasterState ( eState, bPrimary, pCluster->m_sName, sError );
}


// update cluster state nodes from Galera callback on cluster view changes
static void UpdateGroupView ( const wsrep_view_info_t * pView, ReplicationCluster_t * pCluster )
{
	StringBuilder_c sBuf ( "," );
	const wsrep_member_info_t * pBoxes = pView->members;
	for ( int i=0; i<pView->memb_num; i++ )
		sBuf += pBoxes[i].incoming;

	// change of nodes happens only here with single thread
	// its safe to compare nodes string here without lock
	if ( pCluster->m_sViewNodes!=sBuf.cstr() )
	{
		ScopedMutex_t tLock ( pCluster->m_tViewNodesLock );
		pCluster->m_sViewNodes = sBuf.cstr();
	}
}

// callback for Galera view_handler_cb that cluster view got changed, ie node either added or removed from cluster
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
	pCluster->m_iSize = pView->memb_num;
	pCluster->m_iIdx = pView->my_idx;
	pCluster->SetPrimary ( pView->status );
	UpdateGroupView ( pView, pCluster );

	*ppSstReq = nullptr;
	*pSstReqLen = 0;

	if ( pView->state_gap )
	{
		auto sAddr = ::strdup( g_sIncomingProto.scstr() );
		sphLogDebugRpl ( "join %s to %s", sAddr, pCluster->m_sName.cstr() );
		*pSstReqLen = strlen(sAddr) + 1;
		*ppSstReq = sAddr;
		pCluster->SetNodeState ( ClusterState_e::JOINING );
	}

	return WSREP_CB_SUCCESS;
}

// callback for Galera sst_donate_cb to become of donor and start sending SST (all cluster indexes) to joiner
static wsrep_cb_status_t SstDonate_fn ( void * pAppCtx, void * pRecvCtx, const void * sMsg, size_t iMsgLen, const wsrep_gtid_t * pStateID, const char * pState, size_t iStateLen, wsrep_bool_t bBypass )
{
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pRecvCtx;
	
	CSphString sNode;
	sNode.SetBinary ( (const char *)sMsg, (int) iMsgLen );

	wsrep_gtid_t tGtid = *pStateID;
	char sGtid[WSREP_GTID_STR_LEN];
	wsrep_gtid_print ( &tGtid, sGtid, sizeof(sGtid) );

	ReplicationCluster_t * pCluster = pLocalCtx->m_pCluster;
	sphLogDebugRpl ( "donate %s to %s, gtid %s, bypass %d", pCluster->m_sName.cstr(), sNode.cstr(), sGtid, (int)bBypass );

	CSphString sError;

	pCluster->SetNodeState ( ClusterState_e::DONOR );
	const bool bOk = SendClusterIndexes ( pCluster, sNode, bBypass, tGtid, sError );
	pCluster->SetNodeState ( ClusterState_e::SYNCED );

	if ( !bOk )
	{
		sphWarning ( "%s", sError.cstr() );
		tGtid.seqno = WSREP_SEQNO_UNDEFINED;
	}

	pCluster->m_pProvider->sst_sent( pCluster->m_pProvider, &tGtid, ( bOk ? 0 : -ECANCELED ) );

	sphLogDebugRpl ( "donate cluster %s to %s, gtid %s, bypass %d, done %d", pCluster->m_sName.cstr(), sNode.cstr(), sGtid, (int)bBypass, (int)bOk );

	return ( bOk ? WSREP_CB_SUCCESS : WSREP_CB_FAILURE );
}

// callback for Galera synced_cb that cluster fully synced and could accept transactions
static void Synced_fn ( void * pAppCtx )
{
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pAppCtx;

	assert ( pLocalCtx );
	ReplicationCluster_t * pCluster = pLocalCtx->m_pCluster;
	pCluster->SetNodeState ( ClusterState_e::SYNCED );

	sphLogDebugRpl ( "synced cluster %s", pCluster->m_sName.cstr() );
}

void ReceiverCtx_t::Cleanup()
{
	m_tAcc.Cleanup();

	m_tUpdAPI = CSphAttrUpdate();

	for ( CSphFilterSettings & tItem : m_tQuery.m_dFilters )
	{
		// fixme! What a legacy WTF? you take val from tItem.GetValueArray, compare with tItem.m_dValues,
		// then first invoke delete[] and then(!) call SetExternalValues. Why it is here? Why not put it as tItem member,
		// and let it do 'cleanup' action, instead this out-of-encapsulation strange actions upon totally other object
		// with implicitly unknown structure?
		// If will surely lead to UB (most probably - crash), if you delete uservars placed to external values,
		// since they're shared, not owned!
		const SphAttr_t * pVals = tItem.GetValueArray();
		if ( pVals && pVals!=tItem.m_dValues.Begin() )
		{
			delete[] pVals;
			tItem.SetExternalValues ( nullptr, 0 );
		}
	}
	m_tQuery.m_dFilters.Reset();
	m_tQuery.m_dFilterTree.Reset();
}

// callback for Galera apply_cb that transaction received from cluster by node
// This is called to "apply" writeset.
// If writesets don't conflict on keys, it may be called concurrently to
// utilize several CPU cores.
static wsrep_cb_status_t Apply_fn ( void * pCtx, const void * pData, size_t uSize, uint32_t uFlags, const wsrep_trx_meta_t * pMeta )
{
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pCtx;

	bool bCommit = ( ( uFlags & WSREP_FLAG_COMMIT )!=0 );
	bool bIsolated = ( ( uFlags & WSREP_FLAG_ISOLATION )!=0 );
	sphLogDebugRpl ( "writeset at apply, seq " INT64_FMT ", size %d, flags %u, on %s", (int64_t)pMeta->gtid.seqno, (int)uSize, uFlags, ( bCommit ? "commit" : "rollback" ) );

	if ( !ParseCmdReplicated ( (const BYTE *)pData, (int) uSize, bIsolated, pLocalCtx->m_pCluster->m_sName, pLocalCtx->m_tAcc, pLocalCtx->m_tUpdAPI, pLocalCtx->m_tQuery ) )
		return WSREP_CB_FAILURE;

	return WSREP_CB_SUCCESS;
}

// callback for Galera commit_cb that transaction received and parsed before should be committed or rolled back
static wsrep_cb_status_t Commit_fn ( void * pCtx, const void * hndTrx, uint32_t uFlags, const wsrep_trx_meta_t * pMeta, wsrep_bool_t * pExit, wsrep_bool_t bCommit )
{
	ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pCtx;
	wsrep_t * pWsrep = pLocalCtx->m_pCluster->m_pProvider;
	sphLogDebugRpl ( "writeset at %s, seq " INT64_FMT ", flags %u", ( bCommit ? "commit" : "rollback" ), (int64_t)pMeta->gtid.seqno, uFlags );

	wsrep_cb_status_t eRes = WSREP_CB_SUCCESS;
	if ( bCommit && pLocalCtx->m_tAcc.m_dCmd.GetLength() )
	{
		bool bOk = false;
		bool bIsolated = ( pLocalCtx->m_tAcc.m_dCmd[0]->m_bIsolated );
		if ( bIsolated )
		{
			bOk = HandleCmdReplicated ( pLocalCtx->m_tAcc );
		} else
		{
			pWsrep->applier_pre_commit ( pWsrep, const_cast<void *>( hndTrx ) );

			bOk = HandleCmdReplicated ( pLocalCtx->m_tAcc );

			pWsrep->applier_interim_commit ( pWsrep, const_cast<void *>( hndTrx ) );
			pWsrep->applier_post_commit ( pWsrep, const_cast<void *>( hndTrx ) );
		}

		sphLogDebugRpl ( "seq " INT64_FMT ", committed %d, isolated %d", (int64_t)pMeta->gtid.seqno, (int)bOk, (int)bIsolated );

		eRes = ( bOk ? WSREP_CB_SUCCESS : WSREP_CB_FAILURE );
	}
	pLocalCtx->Cleanup();

	return eRes;
}

// callback for Galera unordered_cb that unordered transaction received
static wsrep_cb_status_t Unordered_fn ( void * pCtx, const void * pData, size_t uSize )
{
	//ReceiverCtx_t * pLocalCtx = (ReceiverCtx_t *)pCtx;
	sphLogDebugRpl ( "unordered" );
	return WSREP_CB_SUCCESS;
}

// main recv thread of Galera that handles cluster
// This is the listening thread. It blocks in wsrep::recv() call until
// disconnect from cluster. It will apply and commit writesets through the
// callbacks defined above.
static void ReplicationRecv_fn ( SharedPtr_t<ReceiverCtx_t *> pCtx )
{
	g_pTlsCluster = pCtx->m_pCluster;
	pCtx->m_pCluster->m_bHasWorker = true;
	pCtx->m_pCluster->SetNodeState ( ClusterState_e::JOINING );

	sphLogDebugRpl ( "receiver thread started" );

	wsrep_t * pWsrep = pCtx->m_pCluster->m_pProvider;
	wsrep_status_t eState = pWsrep->recv ( pWsrep, pCtx );

	sphLogDebugRpl ( "receiver done, code %d, %s", eState, GetStatus ( eState ) );
	if ( eState==WSREP_CONN_FAIL || eState==WSREP_NODE_FAIL || eState==WSREP_FATAL )
	{
		pCtx->m_pCluster->SetNodeState ( ClusterState_e::DESTROYED );
	} else
	{
		pCtx->m_pCluster->SetNodeState ( ClusterState_e::CLOSED );
	}
	auto* pCluster = pCtx->m_pCluster;
	pCtx = nullptr;
	pCluster->m_tWorkerFinished.SetEvent ();
}

// callback for Galera pfs_instr_cb there all mutex \ threads \ events should be implemented, could also count these operations for extended stats
// @brief a callback to create PFS instrumented mutex/condition variables
//
// @param type			mutex or condition variable
// @param ops			add/init or remove/destory mutex/condition variable
// @param tag			tag/name of instrument to monitor
// @param value			created mutex or condition variable
// @param alliedvalue	allied value for supporting operation.
//						for example: while waiting for cond-var corresponding
//						mutex is passes through this variable.
// @param ts			time to wait for condition.

void Instr_fn ( wsrep_pfs_instr_type_t type, wsrep_pfs_instr_ops_t ops, wsrep_pfs_instr_tag_t tag, void** value,
	void** alliedvalue, const void* ts )
{
	if ( type==WSREP_PFS_INSTR_TYPE_THREAD || type==WSREP_PFS_INSTR_TYPE_FILE )
		return;

#if !_WIN32
	if ( type==WSREP_PFS_INSTR_TYPE_MUTEX )
	{
		switch ( ops )
		{
			case WSREP_PFS_INSTR_OPS_INIT:
			{
				pthread_mutex_t* pMutex = new pthread_mutex_t;
				pthread_mutex_init ( pMutex, nullptr );
				*value = pMutex;
			}
				break;

			case WSREP_PFS_INSTR_OPS_DESTROY:
			{
				pthread_mutex_t* pMutex = ( pthread_mutex_t* ) ( *value );
				assert ( pMutex );
				pthread_mutex_destroy ( pMutex );
				delete ( pMutex );
				*value = nullptr;
			}
				break;

			case WSREP_PFS_INSTR_OPS_LOCK:
			{
				pthread_mutex_t* pMutex = ( pthread_mutex_t* ) ( *value );
				assert ( pMutex );
				pthread_mutex_lock ( pMutex );
			}
				break;

			case WSREP_PFS_INSTR_OPS_UNLOCK:
			{
				pthread_mutex_t* pMutex = ( pthread_mutex_t* ) ( *value );
				assert ( pMutex );
				pthread_mutex_unlock ( pMutex );
			}
				break;

			default: assert( 0 );
				break;
		}
	} else if ( type==WSREP_PFS_INSTR_TYPE_CONDVAR )
	{
		switch ( ops )
		{
			case WSREP_PFS_INSTR_OPS_INIT:
			{
				pthread_cond_t* pCond = new pthread_cond_t;
				pthread_cond_init ( pCond, nullptr );
				*value = pCond;
			}
				break;

			case WSREP_PFS_INSTR_OPS_DESTROY:
			{
				pthread_cond_t* pCond = ( pthread_cond_t* ) ( *value );
				assert ( pCond );
				pthread_cond_destroy ( pCond );
				delete ( pCond );
				*value = nullptr;
			}
				break;

			case WSREP_PFS_INSTR_OPS_WAIT:
			{
				pthread_cond_t* pCond = ( pthread_cond_t* ) ( *value );
				pthread_mutex_t* pMutex = ( pthread_mutex_t* ) ( *alliedvalue );
				assert ( pCond && pMutex );
				pthread_cond_wait ( pCond, pMutex );
			}
				break;

			case WSREP_PFS_INSTR_OPS_TIMEDWAIT:
			{
				pthread_cond_t* pCond = ( pthread_cond_t* ) ( *value );
				pthread_mutex_t* pMutex = ( pthread_mutex_t* ) ( *alliedvalue );
				const timespec* wtime = ( const timespec* ) ts;
				assert ( pCond && pMutex );
				pthread_cond_timedwait ( pCond, pMutex, wtime );
			}
				break;

			case WSREP_PFS_INSTR_OPS_SIGNAL:
			{
				pthread_cond_t* pCond = ( pthread_cond_t* ) ( *value );
				assert ( pCond );
				pthread_cond_signal ( pCond );
			}
				break;

			case WSREP_PFS_INSTR_OPS_BROADCAST:
			{
				pthread_cond_t* pCond = ( pthread_cond_t* ) ( *value );
				assert ( pCond );
				pthread_cond_broadcast ( pCond );
			}
				break;

			default: assert( 0 );
				break;
		}
	}
#endif
}

static int GetClusterMemLimit ( const StrVec_t & dIndexes )
{
	int iMemLimit = 0;
	int iIndexes = 0;
	for ( const CSphString & sIndex : dIndexes )
	{
		ServedIndexRefPtr_c pServed = GetServed ( sIndex );
		if ( !pServed )
			continue;

		ServedDescRPtr_c pDesc ( pServed );
		iMemLimit = Max ( iMemLimit, pDesc->m_tSettings.m_iMemLimit );
		iIndexes++;
	}

	const int CACHE_PER_INDEX = 16;

	// change default cache size to 16Mb per added index or size of largest rt_mem_limit of RT index
	int iSize = iMemLimit / 1024 / 1024;
	int iCount = Max ( 1, iIndexes );
	iSize = Max ( iCount * CACHE_PER_INDEX, iSize );

	return iSize;
}

// display incoming string as client name in show threads
struct ReplInfo_t : public TaskInfo_t
{
	DECLARE_RENDER( ReplInfo_t );
	CSphString m_sIncoming;	// set once on create, so is th-safe
};

DEFINE_RENDER( ReplInfo_t )
{
	auto & tInfo = *(ReplInfo_t *) pSrc;
	dDst.m_sChain << (int) tInfo.m_eType << ":Repl ";
	dDst.m_sClientName << "wsrep" << tInfo.m_sIncoming.cstr();
}

// create cluster from desc and become master node or join existed cluster
bool ReplicateClusterInit ( ReplicationArgs_t & tArgs, CSphString & sError )
{
	assert ( g_bReplicationEnabled );
	wsrep_t * pWsrep = nullptr;
	// let's load and initialize provider
	CSphString sGaleraFullPath = GET_GALERA_FULLPATH ();
	auto eRes = (wsrep_status_t)wsrep_load ( sGaleraFullPath.cstr(), &pWsrep, Logger_fn );
	if ( eRes!=WSREP_OK )
	{
		sError.SetSprintf ( "provider '%s' - failed to load, %d '%s'", sGaleraFullPath.cstr(), (int)eRes, GetStatus ( eRes ) );
		return false;
	}
	assert ( pWsrep );
	tArgs.m_pCluster->m_pProvider = pWsrep;
	tArgs.m_pCluster->m_dUUID.Reset ( sizeof(wsrep_uuid_t) );
	memcpy ( tArgs.m_pCluster->m_dUUID.Begin(), WSREP_UUID_UNDEFINED.data, tArgs.m_pCluster->m_dUUID.GetLengthBytes() );

	wsrep_gtid_t tStateID = { WSREP_UUID_UNDEFINED, WSREP_SEQNO_UNDEFINED };
	CSphString sMyName;
	sMyName.SetSprintf ( "daemon_%d_%s", GetOsThreadId(), tArgs.m_pCluster->m_sName.cstr() );

	CSphString sConnectNodes;
	if ( tArgs.m_sNodes.IsEmpty() )
	{
		sConnectNodes = g_sDefaultReplicationNodes;
	} else
	{
		sConnectNodes.SetSprintf ( "%s%s", g_sDefaultReplicationNodes, tArgs.m_sNodes.cstr() );
	}

	CSphString sIncoming;
	sIncoming.SetSprintf ( "%s,%s:%d:replication", g_sIncomingProto.cstr(), g_sIncomingIP.cstr(), tArgs.m_iListenPort );
	sphLogDebugRpl ( "node incoming '%s', listen '%s', nodes '%s', name '%s'", sIncoming.cstr(), tArgs.m_sListenAddr.cstr(), sConnectNodes.cstr(), sMyName.cstr() );
	if ( g_eLogLevel>=SPH_LOG_RPL_DEBUG )
	{
		StringBuilder_c sIndexes ( "," );
		for ( const CSphString & sIndex : tArgs.m_pCluster->m_dIndexes )
			sIndexes += sIndex.cstr();
		sphLogDebugRpl ( "cluster '%s', indexes '%s', nodes '%s'", tArgs.m_pCluster->m_sName.cstr(), sIndexes.cstr(), tArgs.m_pCluster->m_sClusterNodes.cstr() );
	}

	SharedPtr_t<ReceiverCtx_t *> pRecvArgs { new ReceiverCtx_t() };
	pRecvArgs->m_pCluster = tArgs.m_pCluster;
	CSphString sFullClusterPath = GetClusterPath ( tArgs.m_pCluster->m_sPath );

	StringBuilder_c sOptions ( ";" );
	sOptions += tArgs.m_pCluster->m_tOptions.AsStr(false).cstr();

	// all replication options below have not stored into cluster config

	// set incoming address
	if ( g_bHasIncoming )
		sOptions.Appendf ( "ist.recv_addr=%s", g_sIncomingIP.cstr() );

	// change default cache size 
	if ( !tArgs.m_pCluster->m_tOptions.m_hOptions.Exists ( "gcache.size" ) )
	{
		int iSize = GetClusterMemLimit ( tArgs.m_pCluster->m_dIndexes );
		sOptions.Appendf ( "gcache.size=%dM", iSize );
	}

	// set debug log option
	if ( g_eLogLevel>=SPH_LOG_RPL_DEBUG )
	{
		sOptions += g_sDebugOptions;
	}

	// wsrep provider initialization arguments
	wsrep_init_args wsrep_args;
	wsrep_args.app_ctx		= pRecvArgs,

	wsrep_args.node_name	= sMyName.cstr();
	wsrep_args.node_address	= tArgs.m_sListenAddr.cstr();
	// must to be set otherwise node works as GARB - does not affect FC and might hung 
	wsrep_args.node_incoming = sIncoming.cstr();
	wsrep_args.data_dir		= sFullClusterPath.cstr(); // working directory
	wsrep_args.options		= sOptions.cstr();
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
	eRes = pWsrep->connect ( pWsrep, tArgs.m_pCluster->m_sName.cstr(), sConnectNodes.cstr(), "", tArgs.m_bNewCluster );
	if ( eRes!=WSREP_OK )
	{
		sError.SetSprintf ( "replication connection failed: %d '%s'", (int)eRes, GetStatus ( eRes ) );
		// might try to join existed cluster however that took too long to timeout in case no cluster started
		return false;
	}

	// let's start listening thread with proper provider set
	auto pScheduler = GetAloneScheduler(-1, sMyName.cstr ());
	Threads::CoGo ( [pRecvArgs,sIncoming] () mutable
	{
		// publish stuff in 'show threads'
		auto pDisplayIncoming = new ReplInfo_t;
		pDisplayIncoming->m_sIncoming = sIncoming;
		ScopedInfo_T<ReplInfo_t> _ { pDisplayIncoming };

		ReplicationRecv_fn ( std::move ( pRecvArgs ) );
	}, pScheduler );
	sphLogDebugRpl ( "replicator is created for cluster '%s'", tArgs.m_pCluster->m_sName.cstr() );
	return true;
}

// shutdown and delete cluster, also join cluster recv thread
void ReplicateClusterDone ( ReplicationCluster_t * pCluster )
{
	if ( !pCluster )
		return;

	auto pProvider = pCluster->m_pProvider;
	pCluster->m_pProvider = nullptr;

	if ( pProvider )
		pProvider->disconnect ( pProvider );

	sphLogDebug ( "disconnect from cluster invoked" );
	// Listening thread are now running and receiving writesets. Wait for them
	// to join. Thread will join after signal handler closes wsrep connection
	pCluster->m_tWorkerFinished.WaitEvent ();

	delete pCluster;
	wsrep_unload ( pProvider );
	sphLogDebug ( "ReplicateClusterDone finished, cluster deleted lib %p unloaded", pProvider );
}

// check return code from Galera calls
// FIXME!!! handle more status codes properly
static bool CheckResult ( wsrep_status_t tRes, const wsrep_trx_meta_t & tMeta, const char * sAt, CSphString & sError )
{
	if ( tRes==WSREP_OK )
		return true;

	sError.SetSprintf ( "error at %s, code %d, seqno " INT64_FMT, sAt, tRes, tMeta.gtid.seqno );
	sphWarning ( "%s", sError.cstr() );
	return false;
}

static std::atomic<int64_t> g_iConnID { 1 };

// replicate serialized data into cluster and call commit monitor along
static bool Replicate ( int iKeysCount, const wsrep_key_t * pKeys, const wsrep_buf_t & tQueries, wsrep_t * pProvider, CommitMonitor_c & tMonitor, bool bUpdate, CSphString & sError )
{
	assert ( pProvider );

	int64_t iConnId = g_iConnID.fetch_add ( 1, std::memory_order_relaxed );

	wsrep_trx_meta_t tLogMeta;
	tLogMeta.gtid = WSREP_GTID_UNDEFINED;

	wsrep_ws_handle_t tHnd;
	tHnd.trx_id = iConnId;
	tHnd.opaque = nullptr;

	wsrep_status_t tRes = pProvider->append_key ( pProvider, &tHnd, pKeys, iKeysCount, WSREP_KEY_EXCLUSIVE, false );
	bool bOk = CheckResult ( tRes, tLogMeta, "append_key", sError );

	if ( bOk )
	{
		tRes = pProvider->append_data ( pProvider, &tHnd, &tQueries, 1, WSREP_DATA_ORDERED, false );
		bOk = CheckResult ( tRes, tLogMeta, "append_data", sError );
	}

	if ( bOk )
	{
		tRes = pProvider->replicate ( pProvider, iConnId, &tHnd, WSREP_FLAG_COMMIT, &tLogMeta );
		bOk = CheckResult ( tRes, tLogMeta, "replicate", sError );
	}
	if ( tRes==WSREP_TRX_FAIL )
	{
		wsrep_status_t tRoll = pProvider->post_rollback ( pProvider, &tHnd );
		CheckResult ( tRoll, tLogMeta, "post_rollback", sError );
	}

	sphLogDebugRpl ( "replicating %d, seq " INT64_FMT ", commands %d", (int)bOk, (int64_t)tLogMeta.gtid.seqno, iKeysCount );

	if ( bOk )
	{
		tRes = pProvider->pre_commit ( pProvider, iConnId, &tHnd, WSREP_FLAG_COMMIT, &tLogMeta );
		bOk = CheckResult ( tRes, tLogMeta, "pre_commit", sError );

		// in case only commit failed
		// need to abort running transaction prior to rollback
		if ( bOk )
		{
			if ( !bUpdate )
				bOk = tMonitor.Commit( sError );
			else
				bOk = tMonitor.Update( true, sError );

			if ( !bOk )
				pProvider->abort_pre_commit ( pProvider, WSREP_SEQNO_UNDEFINED, tHnd.trx_id );
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

		sphLogDebugRpl ( "%s seq " INT64_FMT, ( bOk ? "committed" : "rolled-back" ), (int64_t)tLogMeta.gtid.seqno );
	}

	// FIXME!!! move ThreadID and to net loop handler exit
	pProvider->free_connection ( pProvider, iConnId );

	return bOk;
}

// replicate serialized data into cluster in TotalOrderIsolation mode and call commit monitor along
static bool ReplicateTOI ( int iKeysCount, const wsrep_key_t * pKeys, const wsrep_buf_t & tQueries, wsrep_t * pProvider, CommitMonitor_c & tMonitor, ServedDesc_t * pDesc, CSphString & sError )
{
	assert ( pProvider );

	int64_t iConnId = g_iConnID.fetch_add ( 1, std::memory_order_relaxed );

	wsrep_trx_meta_t tLogMeta;
	tLogMeta.gtid = WSREP_GTID_UNDEFINED;

	wsrep_status_t tRes = pProvider->to_execute_start ( pProvider, iConnId, pKeys, iKeysCount, &tQueries, 1, &tLogMeta );
	bool bOk = CheckResult ( tRes, tLogMeta, "to_execute_start", sError );

	sphLogDebugRpl ( "replicating TOI %d, seq " INT64_FMT, (int)bOk, (int64_t)tLogMeta.gtid.seqno );

	// FXIME!!! can not fail TOI transaction
	if ( bOk )
		tMonitor.CommitTOI ( pDesc, sError );

	if ( bOk )
	{

		tRes = pProvider->to_execute_end ( pProvider, iConnId );
		CheckResult ( tRes, tLogMeta, "to_execute_end", sError );

		sphLogDebugRpl ( "%s seq " INT64_FMT, ( bOk ? "committed" : "rolled-back" ), (int64_t)tLogMeta.gtid.seqno );
	}

	// FIXME!!! move ThreadID and to net loop handler exit
	pProvider->free_connection ( pProvider, iConnId );

	return bOk;
}

// get cluster status variables (our and Galera)
static void ReplicateClusterStats ( ReplicationCluster_t * pCluster, VectorLike & dOut )
{
	if ( !pCluster || !pCluster->m_pProvider )
		return;

	assert ( pCluster->m_iStatus>=0 && pCluster->m_iStatus<WSREP_VIEW_MAX );

	const char * sName = pCluster->m_sName.cstr();

	// cluster vars
	dOut.MatchTuplet ( "cluster_name", sName );
	if ( dOut.MatchAddf ( "cluster_%s_state_uuid", sName ) )
	{
		wsrep_uuid_t tUUID;
		memcpy ( tUUID.data, pCluster->m_dUUID.Begin(), pCluster->m_dUUID.GetLengthBytes() );
		char sUUID[WSREP_UUID_STR_LEN+1] = { '\0' };
		wsrep_uuid_print ( &tUUID, sUUID, sizeof(sUUID) );
		dOut.Add ( sUUID );
	}
	if ( dOut.MatchAddf ( "cluster_%s_conf_id", sName ) )
		dOut.Addf( "%d", pCluster->m_iConfID );
	if ( dOut.MatchAddf ( "cluster_%s_status", sName ) )
		dOut.Add ( g_sClusterStatus[pCluster->m_iStatus] );
	if ( dOut.MatchAddf ( "cluster_%s_size", sName ) )
		dOut.Addf ( "%d", pCluster->m_iSize );
	if ( dOut.MatchAddf ( "cluster_%s_local_index", sName ) )
		dOut.Addf ( "%d", pCluster->m_iIdx );
	if ( dOut.MatchAddf ( "cluster_%s_node_state", sName ) )
		dOut.Add ( GetNodeState ( pCluster->GetNodeState () ) );
	// nodes of cluster defined and view
	{
		ScopedMutex_t tLock ( pCluster->m_tViewNodesLock );
		if ( dOut.MatchAddf ( "cluster_%s_nodes_set", sName ) )
			dOut.Add ( pCluster->m_sClusterNodes.scstr () );
		if ( dOut.MatchAddf ( "cluster_%s_nodes_view", sName ) )
			dOut.Add ( pCluster->m_sViewNodes.scstr () );
	}

	// cluster indexes
	if ( dOut.MatchAddf ( "cluster_%s_indexes_count", sName ) )
		dOut.Addf( "%d", pCluster->m_dIndexes.GetLength() );
	if ( dOut.MatchAddf ( "cluster_%s_indexes", sName ) )
	{
		StringBuilder_c tBuf ( "," );
		for ( const CSphString & sIndex : pCluster->m_dIndexes )
			tBuf += sIndex.cstr();

		dOut.Add( tBuf.cstr() );
	}

	// cluster status
	// trick
	if ( !pCluster->m_pProvider->ctx )
		return;
	wsrep_stats_var * pVarsStart = pCluster->m_pProvider->stats_get ( pCluster->m_pProvider );
	if ( !pVarsStart )
		return;

	wsrep_stats_var * pVars = pVarsStart;
	while ( pVars->name )
	{
		if ( dOut.MatchAddf ( "cluster_%s_%s", sName, pVars->name ) )
		{
			switch ( pVars->type )
			{
			case WSREP_VAR_STRING:
				dOut.Add ( pVars->value._string );
				break;

			case WSREP_VAR_DOUBLE:
				dOut.Addf ( "%f", pVars->value._double );
				break;

			default:
			case WSREP_VAR_INT64:
				dOut.Addf ( "%l", pVars->value._int64 );
				break;
			}
		}

		pVars++;
	}

	pCluster->m_pProvider->stats_free ( pCluster->m_pProvider, pVarsStart );
}

// set Galera option for cluster
bool ReplicateSetOption ( const CSphString & sCluster, const CSphString & sName,
	const CSphString & sVal, CSphString & sError ) EXCLUDES ( g_tClustersLock )
{
	ScRL_t rLock ( g_tClustersLock );
	ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
	if ( !ppCluster )
	{
		sError.SetSprintf ( "unknown cluster '%s' in SET statement", sCluster.cstr() );
		return false;
	}

	ReplicationCluster_t * pCluster = *ppCluster;
	CSphString sOpt;
	sOpt.SetSprintf ( "%s=%s", sName.cstr(), sVal.cstr() );

	wsrep_status_t tOk = pCluster->m_pProvider->options_set ( pCluster->m_pProvider, sOpt.cstr() );
	if ( tOk==WSREP_OK )
	{
		ScopedMutex_t tLock ( pCluster->m_tOptsLock );
		pCluster->m_tOptions.m_hOptions.Add ( sVal, sName );
	} else
	{
		sError = GetStatus ( tOk );
	}

	return ( tOk==WSREP_OK );
}

// abort callback that invalidates specific cluster
void ReplicationAbort()
{
	ReplicationCluster_t * pCluster = g_pTlsCluster;
	sphWarning ( "abort from cluster '%s'", ( pCluster ? pCluster->m_sName.cstr() : "" ) );
	if ( pCluster )
		pCluster->SetNodeState ( ClusterState_e::DESTROYED );
}

// delete all clusters on daemon shutdown
void ReplicateClustersDelete() EXCLUDES ( g_tClustersLock )
{
	sphLogDebug ( "ReplicateClustersDelete invoked" );
	ScWL_t wLock ( g_tClustersLock );
	if ( !g_hClusters.GetLength() )
		return;

	for ( auto& tCluster: g_hClusters )
	{
		sphLogDebug ( "ReplicateClustersDelete for %s", tCluster.first.cstr() );
		ReplicateClusterDone ( tCluster.second );
	}

	sphLogDebug ( "ReplicateClustersDelete done" );
	g_hClusters.Reset ();
}

// clean up cluster prior to start it again
void DeleteClusterByName ( const CSphString& sCluster ) EXCLUDES ( g_tClustersLock )
{
	ScWL_t wLock( g_tClustersLock );
	auto** ppCluster = g_hClusters ( sCluster );
	if ( ppCluster )
	{
		SafeDelete ( *ppCluster );
		g_hClusters.Delete ( sCluster );
	}
}


static void CollectClusterDesc ( CSphVector<ClusterDesc_t> & dClusters )
{
	dClusters.Reset();
	void * pIt = nullptr;
	ScRL_t tLock ( g_tClustersLock );
	while ( g_hClusters.IterateNext ( &pIt ))
	{
		const ReplicationCluster_t * pCluster = g_hClusters.IterateGet ( &pIt );
		dClusters.Add ( *pCluster );
	}
}


// collect clusters and their indexes
void ReplicationCollectClusters ( CSphVector<ClusterDesc_t> & dClusters )  EXCLUDES ( g_tClustersLock )
{
	CSphString sError;
	ScRL_t rLock ( g_tClustersLock );
	if ( !g_bReplicationEnabled && !g_hClusters.GetLength() )
		return;

	CollectClusterDesc ( dClusters );
}


// dump all clusters statuses
void ReplicateClustersStatus ( VectorLike & dStatus ) EXCLUDES ( g_tClustersLock )
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
	if ( pDesc->m_eType!=IndexType_e::PERCOLATE && pDesc->m_eType!=IndexType_e::RT )
	{
		sError.SetSprintf ( "wrong type of index '%s'", sIndex.cstr() );
		return false;
	}

	return true;
}

// set cluster name into index desc for fast rejects
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
		if ( pDesc->m_eType!=IndexType_e::PERCOLATE && pDesc->m_eType!=IndexType_e::RT )
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

// lock or unlock write operations to disk chunks of index
static bool ControlIndexWrite ( const CSphString & sIndex, bool bEnableWrite, CSphString & sError )
{
	ServedIndexRefPtr_c pServed = GetServed ( sIndex );
	if ( !pServed )
	{
		sError.SetSprintf ( "unknown index '%s'", sIndex.cstr() );
		return false;
	}

	ServedDescRPtr_c pDesc ( pServed );
	if ( !ServedDesc_t::IsMutable ( pDesc ) )
	{
		sError.SetSprintf ( "wrong type of index '%s'", sIndex.cstr() );
		return false;
	}

	auto * pIndex = (RtIndex_i*)pDesc->m_pIndex;
	if ( bEnableWrite )
		pIndex->EnableSave();
	else
		pIndex->ProhibitSave();

	return true;
}

// callback for Galera apply_cb to parse replicated commands
bool ParseCmdReplicated ( const BYTE * pData, int iLen, bool bIsolated, const CSphString & sCluster, RtAccum_t & tAcc, CSphAttrUpdate & tUpd, CSphQuery & tQuery )
{
	MemoryReader_c tReader ( pData, iLen );

	while ( tReader.GetPos()<iLen )
	{
		CSphScopedPtr<ReplicationCommand_t> pCmd ( new ReplicationCommand_t );

		ReplicationCommand_e eCommand = (ReplicationCommand_e)tReader.GetWord();
		if ( eCommand<ReplicationCommand_e::PQUERY_ADD || eCommand>ReplicationCommand_e::TOTAL )
		{
			sphWarning ( "bad replication command %d", (int)eCommand );
			return false;
		}

		WORD uVer = tReader.GetWord();
		if ( uVer!=g_iReplicateCommandVer )
		{
			sphWarning ( "replication command %d, version mismatch %d, got %d", (int)eCommand, g_iReplicateCommandVer, (int)uVer );
			return false;
		}

		CSphString sIndex = tReader.GetString();
		int iRequestLen = tReader.GetDword();
		if ( iRequestLen+tReader.GetPos()>iLen )
		{
			sphWarning ( "replication parse apply - out of buffer read %d+%d of %d", tReader.GetPos(), iRequestLen, iLen );
			return false;
		}

		const BYTE * pRequest = pData + tReader.GetPos();
		tReader.SetPos ( tReader.GetPos() + iRequestLen );

		pCmd->m_eCommand = eCommand;
		pCmd->m_sCluster = sCluster;
		pCmd->m_sIndex = sIndex;
		pCmd->m_bIsolated = bIsolated;

		switch ( eCommand )
		{
		case ReplicationCommand_e::PQUERY_ADD:
		{
			ServedIndexRefPtr_c pServed = GetServed ( sIndex );
			if ( !pServed )
			{
				sphWarning ( "unknown index '%s' for replication, command %d", sIndex.cstr(), (int)eCommand );
				return false;
			}

			ServedDescRPtr_c pDesc ( pServed );
			if ( pDesc->m_eType!=IndexType_e::PERCOLATE )
			{
				sphWarning ( "wrong type of index '%s' for replication, command %d", sIndex.cstr(), (int)eCommand );
				return false;
			}

			auto * pIndex = (PercolateIndex_i * )pDesc->m_pIndex;

			StoredQueryDesc_t tPQ;
			LoadStoredQuery ( pRequest, iRequestLen, tPQ );
			sphLogDebugRpl ( "pq-add, index '%s', uid " INT64_FMT " query %s", pCmd->m_sIndex.cstr(), tPQ.m_iQUID, tPQ.m_sQuery.cstr() );

			CSphString sError;
			PercolateQueryArgs_t tArgs ( tPQ );
			tArgs.m_bReplace = true;
			pCmd->m_pStored = pIndex->CreateQuery ( tArgs, sError );

			if ( !pCmd->m_pStored )
			{
				sphWarning ( "pq-add replication error '%s', index '%s'", sError.cstr(), pCmd->m_sIndex.cstr() );
				return false;
			}
		}
		break;

		case ReplicationCommand_e::PQUERY_DELETE:
			LoadDeleteQuery ( pRequest, iRequestLen, pCmd->m_dDeleteQueries, pCmd->m_sDeleteTags );
			sphLogDebugRpl ( "pq-delete, index '%s', queries %d, tags %s", pCmd->m_sIndex.cstr(), pCmd->m_dDeleteQueries.GetLength(), pCmd->m_sDeleteTags.scstr() );
			break;

		case ReplicationCommand_e::TRUNCATE:
			sphLogDebugRpl ( "pq-truncate, index '%s'", pCmd->m_sIndex.cstr() );
			break;

		case ReplicationCommand_e::CLUSTER_ALTER_ADD:
			pCmd->m_bCheckIndex = false;
			sphLogDebugRpl ( "pq-cluster-alter-add, index '%s'", pCmd->m_sIndex.cstr() );
			break;

		case ReplicationCommand_e::CLUSTER_ALTER_DROP:
			sphLogDebugRpl ( "pq-cluster-alter-drop, index '%s'", pCmd->m_sIndex.cstr() );
			break;

		case ReplicationCommand_e::RT_TRX:
			tAcc.LoadRtTrx ( pRequest, iRequestLen );
			sphLogDebugRpl ( "rt trx, index '%s'", pCmd->m_sIndex.cstr() );
			break;

		case ReplicationCommand_e::UPDATE_API:
			LoadUpdate ( pRequest, iRequestLen, tUpd );
			pCmd->m_pUpdateAPI = &tUpd;
			sphLogDebugRpl ( "update, index '%s'", pCmd->m_sIndex.cstr() );
			break;

		case ReplicationCommand_e::UPDATE_QL:
		case ReplicationCommand_e::UPDATE_JSON:
		{
			// can not handle multiple updates - only one update at time
			assert ( !tQuery.m_dFilters.GetLength() );

			int iGot = LoadUpdate ( pRequest, iRequestLen, tUpd );
			assert ( iGot<iRequestLen );
			LoadUpdate ( pRequest + iGot, iRequestLen - iGot, tQuery );
			pCmd->m_pUpdateAPI = &tUpd;
			pCmd->m_pUpdateCond = &tQuery;
			sphLogDebugRpl ( "update %s, index '%s'", ( eCommand==ReplicationCommand_e::UPDATE_QL ? "ql" : "json" ),  pCmd->m_sIndex.cstr() );
			break;
		}

		default:
			sphWarning ( "unsupported replication command %d", (int)eCommand );
			return false;
		}

		tAcc.m_dCmd.Add ( pCmd.LeakPtr() );
	}

	if ( tReader.GetPos()==iLen )
	{
		return true;
	} else
	{
		sphWarning ( "replication parse apply - out of buffer read %d of %d", tReader.GetPos(), iLen );
		return false;
	}
}

// callback for Galera commit_cb to commit replicated command
bool HandleCmdReplicated ( RtAccum_t & tAcc )
{
	if ( !tAcc.m_dCmd.GetLength() )
	{
		sphWarning ( "empty accumulator" );
		return false;
	}

	CSphString sError;
	const ReplicationCommand_t & tCmd = *tAcc.m_dCmd[0];

	bool bCmdCluster = ( tAcc.m_dCmd.GetLength()==1 &&
		( tCmd.m_eCommand==ReplicationCommand_e::CLUSTER_ALTER_ADD || tCmd.m_eCommand==ReplicationCommand_e::CLUSTER_ALTER_DROP ) );
	if ( bCmdCluster )
	{
		if ( tCmd.m_eCommand==ReplicationCommand_e::CLUSTER_ALTER_ADD && !CheckLocalIndex ( tCmd.m_sIndex, sError ) )
		{
			sphWarning ( "replication error %s, command %d", sError.cstr(), (int)tCmd.m_eCommand );
			return false;
		}

		CommitMonitor_c tCommit ( tAcc, nullptr );

		bool bOk = false;
		if ( tCmd.m_eCommand==ReplicationCommand_e::CLUSTER_ALTER_ADD )
		{
			ServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sIndex );
			if ( !pServed )
				sError.SetSprintf ( "unknown index '%s'", tCmd.m_sIndex.cstr() );

			ServedDescWPtr_c pDesc ( pServed );
			if ( pDesc.Ptr() && pDesc->m_eType!=IndexType_e::PERCOLATE && pDesc->m_eType!=IndexType_e::RT )
				sError.SetSprintf ( "wrong type of index '%s'", tCmd.m_sIndex.cstr() );

			if ( pDesc )
				bOk = tCommit.CommitTOI ( pDesc.Ptr(), sError );
		} else
		{
			bOk = tCommit.CommitTOI ( nullptr, sError );
		}

		if ( !bOk )
			sphWarning ( "%s", sError.cstr() );
		return bOk;
	}

	if ( tAcc.m_dCmd.GetLength()==1 &&
		( tCmd.m_eCommand==ReplicationCommand_e::UPDATE_API || tCmd.m_eCommand==ReplicationCommand_e::UPDATE_QL || tCmd.m_eCommand==ReplicationCommand_e::UPDATE_JSON ) )
	{
		int iUpd = -1;
		CSphString sWarning;
		CommitMonitor_c tCommit ( tAcc, &sWarning, &iUpd );
		bool bOk = tCommit.Update ( true, sError );
		if ( !bOk )
			sphWarning ( "%s", sError.cstr() );
		if ( !sWarning.IsEmpty() )
			sphWarning ( "%s", sWarning.cstr() );
		// FIXME!!! make update trx
		return true;

	}

	ServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sIndex );
	if ( !pServed )
	{
		sphWarning ( "unknown index '%s' for replication, command %d", tCmd.m_sIndex.cstr(), (int)tCmd.m_eCommand );
		return false;
	}

	// special path with wlocked index for truncate
	if ( tCmd.m_eCommand==ReplicationCommand_e::TRUNCATE )
	{
		ServedDescWPtr_c pWDesc ( pServed );
		if ( pWDesc->m_eType!=IndexType_e::PERCOLATE && pWDesc->m_eType!=IndexType_e::RT )
		{
			sphWarning ( "wrong type of index '%s' for replication, command %d", tCmd.m_sIndex.cstr (),
						 ( int ) tCmd.m_eCommand );
			return false;
		}
		auto* pIndex = ( RtIndex_i* ) pWDesc->m_pIndex;
		sphLogDebugRpl ( "truncate-commit, index '%s'", tCmd.m_sIndex.cstr ());
		if ( !pIndex->Truncate ( sError ))
			sphWarning ( "%s", sError.cstr ());
		return true;
	}

	ServedDescRPtr_c pRDesc ( pServed );
	if ( pRDesc->m_eType!=IndexType_e::PERCOLATE && pRDesc->m_eType!=IndexType_e::RT )
	{
		sphWarning ( "wrong type of index '%s' for replication, command %d", tCmd.m_sIndex.cstr(), (int)tCmd.m_eCommand );
		return false;
	}

	auto * pIndex = (RtIndex_i * ) pRDesc->m_pIndex;
	assert ( tCmd.m_eCommand!=ReplicationCommand_e::TRUNCATE );
	sphLogDebugRpl ( "commit, index '%s', uid " INT64_FMT ", queries %d, tags %s",
		tCmd.m_sIndex.cstr(), ( tCmd.m_pStored ? tCmd.m_pStored->m_iQUID : int64_t(0) ),
		tCmd.m_dDeleteQueries.GetLength(), tCmd.m_sDeleteTags.scstr() );

	if ( !tAcc.SetupDocstore ( *pIndex, sError ) )
	{
		sphWarning ( "%s, index '%s', command %d", sError.cstr(), tCmd.m_sIndex.cstr(), (int)tCmd.m_eCommand );
		return false;
	}

	if ( !pIndex->Commit ( nullptr, &tAcc ) )
		return false;

	return true;
}

// single point there all commands passed these might be replicated, even if no cluster
static bool HandleCmdReplicate ( RtAccum_t & tAcc, CSphString & sError, int * pDeletedCount, CSphString * pWarning, int * pUpdated, ServedDesc_t * pDesc ) EXCLUDES ( g_tClustersLock )
{
	CommitMonitor_c tMonitor ( tAcc, pDeletedCount, pWarning, pUpdated );

	// without cluster path
	if ( !IsClusterCommand ( tAcc ) )
	{
		if ( IsUpdateCommand ( tAcc ) )
			return tMonitor.Update ( false, sError );
		else
			return tMonitor.Commit ( sError );
	}

	// FIXME!!! for now only PQ add and PQ delete multiple commands supported
	const ReplicationCommand_t & tCmdCluster = *tAcc.m_dCmd[0];

	ClusterState_e eClusterState = ClusterState_e::CLOSED;
	bool bPrimary = false;
	ReplicationCluster_t ** ppCluster = nullptr;
	{
		ScRL_t rLock ( g_tClustersLock );
		ppCluster = g_hClusters ( tCmdCluster.m_sCluster );
		if ( ppCluster )
		{
			eClusterState = ( *ppCluster )->GetNodeState();
			bPrimary = ( *ppCluster )->IsPrimary();
		}
	}

	if ( !ppCluster )
	{
		sError.SetSprintf ( "unknown cluster '%s'", tCmdCluster.m_sCluster.cstr() );
		return false;
	}
	if ( !CheckClasterState ( eClusterState, bPrimary, tCmdCluster.m_sCluster, sError ) )
		return false;

	if ( tCmdCluster.m_eCommand==ReplicationCommand_e::TRUNCATE && tCmdCluster.m_tReconfigure.Ptr() )
	{
		sError.SetSprintf ( "RECONFIGURE is not supported for a cluster index" );
		return false;
	}

	const uint64_t uIndexHash = sphFNV64 ( tCmdCluster.m_sIndex.cstr() );
	if ( tCmdCluster.m_bCheckIndex && !(*ppCluster)->m_dIndexHashes.BinarySearch ( uIndexHash ) )
	{
		sError.SetSprintf ( "index '%s' doesn't belong to cluster '%s'", tCmdCluster.m_sIndex.cstr(), tCmdCluster.m_sCluster.cstr() );
		return false;
	}

	assert ( (*ppCluster)->m_pProvider );
	bool bTOI = false;
	bool bUpdate = false;

	// replicator check CRC of data - no need to check that at our side
	int iKeysCount = tAcc.m_dCmd.GetLength() + tAcc.m_dAccumKlist.GetLength();
	CSphVector<BYTE> dBufQueries;

	// FIXME!!! reduce buffers count
	CSphFixedVector<uint64_t> dBufKeys ( iKeysCount );
	CSphFixedVector<wsrep_buf_t> dBufProxy ( iKeysCount );
	CSphFixedVector<wsrep_key_t> dKeys ( iKeysCount );
//	dBufKeys.ZeroVec();
	int iKey = 0;

	ARRAY_FOREACH ( i, tAcc.m_dCmd )
	{
		const ReplicationCommand_t & tCmd = *tAcc.m_dCmd[i];
		uint64_t uQueryHash = uIndexHash;
		uQueryHash = sphFNV64 ( &tCmd.m_eCommand, sizeof(tCmd.m_eCommand), uQueryHash );

		// scope for writer
		{
			MemoryWriter_c tWriter ( dBufQueries );
			tWriter.PutWord ( (WORD)tCmd.m_eCommand );
			tWriter.PutWord ( g_iReplicateCommandVer );
			tWriter.PutString ( tCmd.m_sIndex );
		}
		dBufQueries.AddN ( sizeof(DWORD) );
		int iLenOff = dBufQueries.GetLength();

		switch ( tCmd.m_eCommand )
		{
		case ReplicationCommand_e::PQUERY_ADD:
			assert ( tCmd.m_pStored );
			SaveStoredQuery ( *tCmd.m_pStored.Ptr(), dBufQueries );

			uQueryHash = sphFNV64 ( &tCmd.m_pStored->m_iQUID, sizeof(tCmd.m_pStored->m_iQUID), uQueryHash );
			break;

		case ReplicationCommand_e::PQUERY_DELETE:
			assert ( tCmd.m_dDeleteQueries.GetLength() || !tCmd.m_sDeleteTags.IsEmpty() );
			SaveDeleteQuery ( tCmd.m_dDeleteQueries, tCmd.m_sDeleteTags.cstr(), dBufQueries );

			ARRAY_FOREACH ( i, tCmd.m_dDeleteQueries )
			{
				uQueryHash = sphFNV64 ( tCmd.m_dDeleteQueries.Begin()+i, sizeof(tCmd.m_dDeleteQueries[0]), uQueryHash );
			}
			uQueryHash = sphFNV64cont ( tCmd.m_sDeleteTags.cstr(), uQueryHash );
			break;

		case ReplicationCommand_e::TRUNCATE:
			// FIXME!!! add reconfigure option here
			break;

		case ReplicationCommand_e::CLUSTER_ALTER_ADD:
		case ReplicationCommand_e::CLUSTER_ALTER_DROP:
			bTOI = true;
			break;

		case ReplicationCommand_e::RT_TRX:
		{
			MemoryWriter_c tWriter ( dBufQueries );
			auto iStartPos = dBufQueries.GetLengthBytes();
			tAcc.SaveRtTrx ( tWriter );
			uQueryHash = sphFNV64cont( { dBufQueries.begin() + iStartPos, (int64_t)(dBufQueries.GetLengthBytes() - iStartPos) }, uQueryHash);
		}
		break;

		case ReplicationCommand_e::UPDATE_API:
		{
			assert ( tCmd.m_pUpdateAPI );
			const CSphAttrUpdate * pUpd = tCmd.m_pUpdateAPI;
			bUpdate = true;

			uQueryHash = sphFNV64 ( pUpd->m_dDocids.Begin(), (int) pUpd->m_dDocids.GetLengthBytes(), uQueryHash );
			uQueryHash = sphFNV64 ( pUpd->m_dPool.Begin(), (int) pUpd->m_dPool.GetLengthBytes(), uQueryHash );

			SaveUpdate ( *pUpd, dBufQueries );
		}
		break;

		case ReplicationCommand_e::UPDATE_QL:
		case ReplicationCommand_e::UPDATE_JSON:
		{
			assert ( tCmd.m_pUpdateAPI );
			assert ( tCmd.m_pUpdateCond );
			const CSphAttrUpdate * pUpd = tCmd.m_pUpdateAPI;
			bUpdate = true;

			uQueryHash = sphFNV64 ( pUpd->m_dDocids.Begin(), (int) pUpd->m_dDocids.GetLengthBytes(), uQueryHash );
			uQueryHash = sphFNV64 ( pUpd->m_dPool.Begin(), (int) pUpd->m_dPool.GetLengthBytes(), uQueryHash );

			SaveUpdate ( *pUpd, dBufQueries );
			SaveUpdate ( *tCmd.m_pUpdateCond, dBufQueries );
		}
		break;

		default:
			sError.SetSprintf ( "unknown command '%d'", (int)tCmd.m_eCommand );
			return false;
		}

		// store query hash as key
		dBufKeys[iKey] = uQueryHash;
		++iKey;

		if ( tCmd.m_eCommand==ReplicationCommand_e::RT_TRX )
		{
			// store ids as keys
			memcpy ( dBufKeys.Begin() + iKey, tAcc.m_dAccumKlist.Begin(), tAcc.m_dAccumKlist.GetLengthBytes() );
			iKey += tAcc.m_dAccumKlist.GetLength();
		}

		// store request length
		int iReqLen = dBufQueries.GetLength() - iLenOff;
		memcpy ( dBufQueries.Begin() + iLenOff - sizeof ( iReqLen ), &iReqLen, sizeof ( iReqLen ) );
	}

	// set keys wsrep_buf_t ptr and len
	for ( int i=0; i<iKeysCount; i++ )
	{
		dBufProxy[i].ptr = dBufKeys.Begin() + i;
		dBufProxy[i].len = sizeof ( dBufKeys[0] );

		dKeys[i].key_parts = dBufProxy.Begin() + i;
		dKeys[i].key_parts_num = 1;
	}

	wsrep_buf_t tQueries;
	tQueries.ptr = dBufQueries.Begin();
	tQueries.len = dBufQueries.GetLength();

	if ( !bTOI )
		return Replicate ( iKeysCount, dKeys.Begin(), tQueries, (*ppCluster)->m_pProvider, tMonitor, bUpdate, sError );
	else
		return ReplicateTOI ( iKeysCount, dKeys.Begin(), tQueries, (*ppCluster)->m_pProvider, tMonitor, pDesc, sError );
}

bool HandleCmdReplicate ( RtAccum_t & tAcc, CSphString & sError )
{
	return HandleCmdReplicate ( tAcc, sError, nullptr, nullptr, nullptr, nullptr );
}

bool HandleCmdReplicate ( RtAccum_t & tAcc, CSphString & sError, int & iDeletedCount )
{
	return HandleCmdReplicate ( tAcc, sError, &iDeletedCount, nullptr, nullptr, nullptr );
}

bool HandleCmdReplicate ( RtAccum_t & tAcc, CSphString & sError, CSphString & sWarning, int & iUpdated )
{
	return HandleCmdReplicate ( tAcc, sError, nullptr, &sWarning, &iUpdated, nullptr );
}

// commit for common commands
bool CommitMonitor_c::Commit ( CSphString& sError )
{
	RtIndex_i* pIndex = m_tAcc.GetIndex ();

	// short path for usual accum without commands
	if ( m_tAcc.m_dCmd.IsEmpty() )
	{
		if ( !pIndex )
			return false;

		if ( !pIndex->Commit ( m_pDeletedCount, &m_tAcc ) )
			return false;

		return true;
	}

	ReplicationCommand_t& tCmd = *m_tAcc.m_dCmd[0];
	bool bTruncate = tCmd.m_eCommand==ReplicationCommand_e::TRUNCATE;
	bool bOnlyTruncate = bTruncate && ( m_tAcc.m_dCmd.GetLength ()==1 );

	// process with index from accum (no need to lock/unlock it)
	if ( pIndex )
		return CommitNonEmptyCmds ( pIndex, tCmd, bOnlyTruncate, sError );

	auto pServed = GetServed ( tCmd.m_sIndex );
	if ( !pServed )
	{
		sError = "requires an existing index";
		return false;
	}

	// truncate needs wlocked index
	if ( bTruncate )
	{
		ServedDescWPtr_c wLocked ( pServed );
		if ( ServedDesc_t::IsMutable ( wLocked ))
			return CommitNonEmptyCmds (( RtIndex_i* ) wLocked->m_pIndex, tCmd, bOnlyTruncate, sError );

		sError = "requires an existing RT or percolate index";
		return false;
	}

	ServedDescRPtr_c rLocked ( pServed );
	if ( ServedDesc_t::IsMutable ( rLocked ))
		return CommitNonEmptyCmds (( RtIndex_i* ) rLocked->m_pIndex, tCmd, bOnlyTruncate, sError );

	sError = "requires an existing RT or percolate index";
	return false;
}

bool CommitMonitor_c::CommitNonEmptyCmds ( RtIndex_i* pIndex, const ReplicationCommand_t& tCmd, bool bOnlyTruncate, CSphString & sError ) const
{
	assert ( pIndex );
	if ( !bOnlyTruncate )
		return pIndex->Commit ( m_pDeletedCount, &m_tAcc );

	if ( !pIndex->Truncate ( sError ))
		return false;

	if ( !tCmd.m_tReconfigure.Ptr() )
		return true;

	CSphReconfigureSetup tSetup;
	StrVec_t dWarnings;
	bool bSame = pIndex->IsSameSettings ( *tCmd.m_tReconfigure.Ptr (), tSetup, dWarnings, sError );
	if ( !bSame && sError.IsEmpty() && !pIndex->Reconfigure ( tSetup ) )
		return false;

	return sError.IsEmpty ();
}

// commit for Total Order Isolation commands
bool CommitMonitor_c::CommitTOI ( ServedDesc_t * pDesc, CSphString & sError ) EXCLUDES ( g_tClustersLock )
{
	if ( m_tAcc.m_dCmd.IsEmpty() )
	{
		sError.SetSprintf ( "empty accumulator" );
		return false;
	}

	const ReplicationCommand_t & tCmd = *m_tAcc.m_dCmd[0];

	ScWL_t tLock ( g_tClustersLock ); // FIXME!!! no need to lock as all cluster operation serialized with TOI mode
	ReplicationCluster_t ** ppCluster = g_hClusters ( tCmd.m_sCluster );
	if ( !ppCluster )
	{
		sError.SetSprintf ( "unknown cluster '%s'", tCmd.m_sCluster.cstr() );
		return false;
	}

	ReplicationCluster_t * pCluster = *ppCluster;
	const uint64_t uIndexHash = sphFNV64 ( tCmd.m_sIndex.cstr() );
	if ( tCmd.m_bCheckIndex && !pCluster->m_dIndexHashes.BinarySearch ( uIndexHash ) )
	{
		sError.SetSprintf ( "index '%s' doesn't belong to cluster '%s'", tCmd.m_sIndex.cstr(), tCmd.m_sCluster.cstr() );
		return false;
	}

	switch ( tCmd.m_eCommand )
	{
	case ReplicationCommand_e::CLUSTER_ALTER_ADD:
		if ( pDesc->m_sCluster!=pCluster->m_sName )
			pDesc->m_sCluster = pCluster->m_sName;

		pCluster->m_dIndexes.Add ( tCmd.m_sIndex );
		pCluster->m_dIndexes.Uniq();
		pCluster->UpdateIndexHashes();
		break;

	case ReplicationCommand_e::CLUSTER_ALTER_DROP:
		if ( !SetIndexCluster ( tCmd.m_sIndex, CSphString(), sError ) )
			return false;

		pCluster->m_dIndexes.RemoveValue ( tCmd.m_sIndex );
		pCluster->UpdateIndexHashes();
		break;

	default:
		sError.SetSprintf ( "unknown command '%d'", (int)tCmd.m_eCommand );
		return false;
	}

	return true;
}

static bool UpdateAPI ( AttrUpdateArgs& tUpd, int & iUpdate )
{
	if ( !tUpd.m_pDesc )
	{
		*tUpd.m_pError = "index not available";
		return false;
	}

	bool bCritical = false;
	iUpdate = tUpd.m_pDesc->m_pIndex->UpdateAttributes ( *tUpd.m_pUpdate, -1, bCritical, tUpd.m_fnLocker, *tUpd.m_pError, *tUpd.m_pWarning );
	return ( iUpdate>=0 );
}

bool CommitMonitor_c::Update ( bool bCluster, CSphString & sError )
{
	if ( m_tAcc.m_dCmd.IsEmpty() )
	{
		sError.SetSprintf ( "empty accumulator" );
		return false;
	}

	const ReplicationCommand_t & tCmd = *m_tAcc.m_dCmd[0];

	ServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sIndex );
	if ( !pServed )
	{
		sError = "requires an existing index";
		return false;
	}

	assert ( m_pUpdated );
	assert ( m_pWarning );
	assert ( tCmd.m_pUpdateAPI );

	AttrUpdateArgs tUpd;
	tUpd.m_pUpdate = tCmd.m_pUpdateAPI;
	tUpd.m_pError = &sError;
	tUpd.m_pWarning = m_pWarning;
	tUpd.m_pQuery = tCmd.m_pUpdateCond;
	tUpd.m_pIndexName = &tCmd.m_sIndex;
	tUpd.m_bJson = ( tCmd.m_eCommand==ReplicationCommand_e::UPDATE_JSON );
	// can not reshedule into common quque for replication - could be dead-lock with incoming clients into the same index
	tUpd.m_bNoYeld = bCluster;
	ServedDescRPtr_c tDesc ( pServed );
	tUpd.m_pDesc = tDesc.Ptr ();

	// that might be called when blob updates necessary, however we don't need to call it actually more than once
	bool bWlocked = false;
	tUpd.m_fnLocker = [&bWlocked, &tDesc, bCluster] {
		if (!bWlocked)
		{
			tDesc.UpgradeLock ( bCluster );
			bWlocked = true;
		}
	};

	if ( tCmd.m_eCommand==ReplicationCommand_e::UPDATE_API )
		return UpdateAPI ( tUpd, *m_pUpdated );

	assert ( tCmd.m_pUpdateCond );
	HandleMySqlExtendedUpdate ( tUpd );
	if ( sError.IsEmpty() )
		*m_pUpdated += tUpd.m_iAffected;

	return ( sError.IsEmpty() );
}

CommitMonitor_c::~CommitMonitor_c()
{
	m_tAcc.Cleanup();
}


// load index into daemon from disk files for cluster use
// in case index already exists prohibit it to save on index delete as disk files has fresh data received from remote node
static bool LoadIndex ( const CSphConfigSection & hIndex, const CSphString & sIndexName, const CSphString & sCluster, CSphString & sError )
{
	// delete existed index first, does not allow to save it's data that breaks sync'ed index files
	// need a scope for RefRtr's to work
	{
		ServedIndexRefPtr_c pServedCur = GetServed ( sIndexName );
		if ( pServedCur )
		{
			// we are only owner of that index and will free it after delete from hash
			ServedDescWPtr_c pDesc ( pServedCur );
			if ( ServedDesc_t::IsMutable ( pDesc ) )
			{
				auto * pIndex = (RtIndex_i*)pDesc->m_pIndex;
				pIndex->ProhibitSave();
			}

			g_pLocalIndexes->Delete ( sIndexName );
		}
	}

	GuardedHash_c dNotLoadedIndexes;
	ESphAddIndex eAdd = AddIndexMT ( dNotLoadedIndexes, sIndexName.cstr(), hIndex, false, true, nullptr, sError );
	assert ( eAdd==ADD_DSBLED || eAdd==ADD_ERROR );

	if ( eAdd!=ADD_DSBLED )
		return false;

	ServedIndexRefPtr_c pServed = GetServed ( sIndexName, &dNotLoadedIndexes );
	ServedDescWPtr_c pDesc ( pServed );
	pDesc->m_sCluster = sCluster;

	StrVec_t dWarnings;
	bool bPreload = PreallocNewIndex ( *pDesc, &hIndex, sIndexName.cstr(), dWarnings, sError );
	if ( !bPreload )
		return false;

	for ( const auto & i : dWarnings )
		sphWarning ( "index '%s': %s", sIndexName.cstr(), i.cstr() );

	// finally add the index to the hash of enabled.
	g_pLocalIndexes->AddOrReplace ( pServed, sIndexName );
	return true;
}

// load indexes received from another node or existed already into daemon
static bool ReplicatedIndexes ( const CSphFixedVector<CSphString> & dIndexes, const CSphString & sCluster, CSphString & sError ) EXCLUDES ( g_tClustersLock )
{
	assert ( g_bReplicationEnabled );

	// scope for check of cluster data
	{
		ScRL_t rLock( g_tClustersLock );

		if ( !g_hClusters.GetLength())
		{
			sError = "no clusters found";
			return false;
		}

		SmallStringHash_T<int> hIndexes;
		for ( const CSphString & sIndex : dIndexes )
		{
			if ( !CheckLocalIndex ( sIndex, sError ) )
				return false;

			hIndexes.Add ( 1, sIndex );
		}

		ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "unknown cluster '%s'", sCluster.cstr() );
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
					sError.SetSprintf ( "index '%s' is already a part of cluster '%s'", sIndex.cstr(), pOrigCluster->m_sName.cstr() );
					return false;
				}
			}
		}
	}

	bool bOk = true;
	for ( const CSphString & sIndex : dIndexes )
		bOk &= SetIndexCluster ( sIndex, sCluster, sError );

	// need to enable back local index write
	for ( const CSphString & sIndex : dIndexes )
		bOk &= ControlIndexWrite ( sIndex, true, sError );

	if ( !bOk )
		return false;

	// scope for modify cluster data
	{
		ScWL_t tLock ( g_tClustersLock );
		// should be already in cluster list
		ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "unknown cluster '%s'", sCluster.cstr() );
			return false;
		}

		ReplicationCluster_t * pCluster = *ppCluster;
		pCluster->m_dIndexes.Resize ( dIndexes.GetLength() );
		ARRAY_FOREACH ( i, dIndexes )
			pCluster->m_dIndexes[i] = dIndexes[i];
		pCluster->UpdateIndexHashes();
	}

	return SaveConfigInt(sError);
}

// create string by join global data_dir and cluster path 
CSphString GetClusterPath ( const CSphString & sPath )
{
	if ( sPath.IsEmpty() )
		return GetDataDirInt();

	CSphString sFullPath;
	sFullPath.SetSprintf ( "%s/%s", GetDataDirInt().cstr(), sPath.cstr() );

	return sFullPath;
}

// create string by join global data_dir and cluster path 
static bool GetClusterPath ( const CSphString & sCluster, CSphString & sClusterPath, CSphString & sError ) EXCLUDES ( g_tClustersLock )
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


// validate clusters paths
static bool ClusterCheckPath ( const CSphString & sPath, const CSphString & sName, bool bCheckWrite, CSphString & sError ) EXCLUDES ( g_tClustersLock )
{
	if ( !g_bReplicationEnabled )
	{
		sError.SetSprintf ( "data_dir option is missing in config or no replication listener is set, replication is disabled" );
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

// set safe_to_bootstrap: 1 at cluster/grastate.dat for Galera to start properly
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
		int iLineLen = tReader.GetLine ( dBuf.Begin(), (int) dBuf.GetLengthBytes() );
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

// clean up Galera files at cluster path to start new and fresh cluster again
static void NewClusterClean ( const CSphString & sPath )
{
	for ( const char * sFile : g_sLibFiles )
	{
		CSphString sName;
		sName.SetSprintf ( "%s/%s", sPath.cstr(), sFile );

		::unlink ( sName.cstr() );
	}
}

// setup IP, ports and node incoming address
static void SetListener ( const CSphVector<ListenerDesc_t> & dListeners )
{
	bool bGotReplicationPorts = false;
	ARRAY_FOREACH ( i, dListeners )
	{
		const ListenerDesc_t & tListen = dListeners[i];
		if ( tListen.m_eProto!=Proto_e::REPLICATION )
			continue;

		const bool bBadCount = ( tListen.m_iPortsCount<2 ); 
		const bool bBadRange = ( ( tListen.m_iPortsCount%2 )!=0 && ( tListen.m_iPortsCount-1 )<2 );
		if ( bBadCount || bBadRange )
		{
			sphWarning ( "invalid replication ports count %d, should be at least 2", tListen.m_iPortsCount );
			continue;
		}

		PortsRange_t tPorts;
		tPorts.m_iPort = tListen.m_iPort;
		tPorts.m_iCount = tListen.m_iPortsCount;
		if ( ( tPorts.m_iCount%2 )!=0 )
			tPorts.m_iCount--;
		if ( tListen.m_uIP!=0 )
		{
			char sListenBuf [ SPH_ADDRESS_SIZE ];
			sphFormatIP ( sListenBuf, sizeof(sListenBuf), tListen.m_uIP );
			if ( g_sListenReplicationIP.IsEmpty() )
			{
				g_sListenReplicationIP = sListenBuf;
				g_tPorts.AddAddr ( g_sListenReplicationIP );
			} else if ( g_sListenReplicationIP!=sListenBuf )
			{
				sphWarning ( "multiple replication IP ('%s') found but only 1st IP '%s' used", sListenBuf, g_sListenReplicationIP.cstr() );
			}
		}

		bGotReplicationPorts = true;
		g_tPorts.AddRange ( tPorts );
	}

	int iPort = dListeners.GetFirst ( [&] ( const ListenerDesc_t & tListen ) { return tListen.m_eProto==Proto_e::SPHINX; } );
	if ( iPort==-1 )
	{
		if ( GetClustersInt().GetLength() )
			sphWarning ( "no 'listen' is found, cannot set incoming addresses, replication is disabled" );
		return;
	}

	if ( !bGotReplicationPorts )
	{
		const ListenerDesc_t & tListen = dListeners[iPort];

		if ( tListen.m_uIP!=0 )
		{
			char sListenBuf [ SPH_ADDRESS_SIZE ];
			sphFormatIP ( sListenBuf, sizeof(sListenBuf), tListen.m_uIP );
			g_sListenReplicationIP = sListenBuf;
		} else
		{
			g_sListenReplicationIP.SetSprintf ( "127.0.0.1" );
		}

		PortsRange_t tPorts;
		tPorts.m_iPort = tListen.m_iPort + g_iDefaultPortBias;
		tPorts.m_iCount = g_iDefaultPortRange;
		g_tPorts.AddRange ( tPorts );
		g_tPorts.AddAddr ( g_sListenReplicationIP );
	}

	if ( !g_bHasIncoming )
		g_sIncomingIP = g_sListenReplicationIP;

	g_sIncomingProto.SetSprintf ( "%s:%d", g_sIncomingIP.cstr(), dListeners[iPort].m_iPort );
	g_bReplicationEnabled = IsConfigless();
}

// start clusters on daemon start
void ReplicationStart ( const CSphConfigSection & hSearchd, const CSphVector<ListenerDesc_t> & dListeners,
	bool bNewCluster, bool bForce ) EXCLUDES ( g_tClustersLock )
{
	SetListener ( dListeners );

	if ( !g_bReplicationEnabled )
	{
		if ( GetClustersInt().GetLength() )
			sphWarning ( "data_dir option is missing in config or no replication listener is set, replication is disabled" );
		return;
	}

	Threads::CallCoroutine ( [bNewCluster,bForce] () EXCLUDES ( g_tClustersLock ) {
	CSphString sError;

	for ( const ClusterDesc_t & tDesc : GetClustersInt() )
	{
		ReplicationArgs_t tArgs;
		// global options
		tArgs.m_bNewCluster = ( bNewCluster || bForce );

		// cluster nodes
		if ( tArgs.m_bNewCluster || tDesc.m_sClusterNodes.IsEmpty() )
		{
			if ( tDesc.m_sClusterNodes.IsEmpty() )
			{
				sphWarning ( "no nodes found, created new cluster '%s'", tDesc.m_sName.cstr() );
				tArgs.m_bNewCluster = true;
			}
		} else
		{
			CSphString sNodes;
			if ( !ClusterGetNodes ( tDesc.m_sClusterNodes, tDesc.m_sName, "", sError, sNodes ) )
			{
				sphWarning ( "cluster '%s': no available nodes, replication is disabled, error: %s", tDesc.m_sName.cstr(), sError.cstr() );
				continue;
			}

			ClusterFilterNodes ( sNodes, Proto_e::REPLICATION, tArgs.m_sNodes );
			if ( sNodes.IsEmpty() )
			{
				sphWarning ( "cluster '%s': no available nodes, replication is disabled", tDesc.m_sName.cstr() );
				continue;
			}
		}

		ScopedPort_c tPort ( g_tPorts.Get() );
		if ( tPort.Get()==-1 )
		{
			sphWarning ( "cluster '%s', no available replication ports, replication is disabled, add replication listener", tDesc.m_sName.cstr() );
			continue;
		}
		tArgs.m_sListenAddr.SetSprintf ( "%s:%d", g_sListenReplicationIP.cstr(), tPort.Get() );
		tArgs.m_iListenPort = tPort.Get();

		CSphScopedPtr<ReplicationCluster_t> pElem ( new ReplicationCluster_t );
		pElem->m_sName = tDesc.m_sName;
		pElem->m_sPath = tDesc.m_sPath;
		pElem->m_tOptions = tDesc.m_tOptions;
		pElem->m_tPort.Set ( tPort.Leak() );
		pElem->m_sClusterNodes = tDesc.m_sClusterNodes;

		// check cluster path is unique
		if ( !ClusterCheckPath ( pElem->m_sPath, pElem->m_sName, false, sError ) )
		{
			sphWarning ( "%s, skipped", sError.cstr() );
			continue;
		}

		CSphString sClusterPath = GetClusterPath ( pElem->m_sPath );

		// set safe_to_bootstrap to 1 into grastate.dat file at cluster path
		if ( bForce && !NewClusterForce ( sClusterPath, sError ) )
		{
			sphWarning ( "%s, cluster '%s' skipped", sError.cstr(), pElem->m_sName.cstr() );
			continue;
		}

		// check indexes valid
		for ( const CSphString & sIndexName : tDesc.m_dIndexes )
		{
			if ( !SetIndexCluster ( sIndexName, pElem->m_sName, sError ) )
			{
				sphWarning ( "%s, removed from cluster '%s'", sError.cstr(), pElem->m_sName.cstr() );
				continue;
			}
			pElem->m_dIndexes.Add ( sIndexName );
		}
		pElem->UpdateIndexHashes();
		tArgs.m_pCluster = pElem.Ptr();

		{
			ScWL_t tLock ( g_tClustersLock );
			g_hClusters.Add ( pElem.LeakPtr(), tDesc.m_sName );
		}

		CSphString sError;
		if ( !ReplicateClusterInit ( tArgs, sError ) )
		{
			sphLogFatal ( "%s", sError.cstr() );
			DeleteClusterByName ( tDesc.m_sName );
		}
	}});
}

// validate cluster option at SphinxQL statement
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
		sError.SetSprintf ( "'%s' should have a string value", sName );
		return false;
	}

	sVal = (*ppVal)->m_sVal;

	return true;
}

// validate cluster SphinxQL statement
static bool CheckClusterStatement ( const CSphString & sCluster, bool bCheckCluster, CSphString & sError ) EXCLUDES ( g_tClustersLock )
{
	if ( !g_bReplicationEnabled )
	{
		sError = "data_dir option is missing or no replication provider configured";
		return false;
	}

	if ( !bCheckCluster )
		return true;

	ScRL_t rLock ( g_tClustersLock );
	const bool bClusterExists = ( g_hClusters ( sCluster )!=nullptr );
	if ( bClusterExists )
		sError.SetSprintf ( "cluster '%s' already exists", sCluster.cstr() );
	return !bClusterExists;
}

// validate cluster SphinxQL statement
static bool CheckClusterStatement ( const CSphString & sCluster, const StrVec_t & dNames,
	const CSphVector<SqlInsert_t> & dValues, bool bJoin, CSphString & sError, CSphScopedPtr<ReplicationCluster_t> & pElem )
{
	if ( !CheckClusterStatement ( sCluster, true, sError ) )
		return false;

	SmallStringHash_T<SqlInsert_t *> hValues;
	assert ( dNames.GetLength()==dValues.GetLength() );
	ARRAY_FOREACH ( i, dNames )
		hValues.Add ( dValues.Begin() + i, dNames[i] );

	pElem = new ReplicationCluster_t;
	pElem->m_sName = sCluster;

	// optional items
	if ( !CheckClusterOption ( hValues, "at_node", true, pElem->m_sClusterNodes, sError ) )
		return false;
	if ( pElem->m_sClusterNodes.IsEmpty() && !CheckClusterOption ( hValues, "nodes", true, pElem->m_sClusterNodes, sError ) )
		return false;

	if ( bJoin && pElem->m_sClusterNodes.IsEmpty() )
	{
		sError.SetSprintf ( "cannot join without either nodes list or AT node" );
		return false;
	}

	if ( !CheckClusterOption ( hValues, "path", true, pElem->m_sPath, sError ) )
		return false;

	CSphString sOptions;
	if ( !CheckClusterOption ( hValues, "options", true, sOptions, sError ) )
		return false;

	pElem->m_tOptions.Parse(sOptions);

	// check cluster path is unique
	bool bValidPath = ClusterCheckPath ( pElem->m_sPath, pElem->m_sName, true, sError );
	if ( !bValidPath )
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// cluster joins to existed nodes
/////////////////////////////////////////////////////////////////////////////

bool ClusterJoin ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues,
	bool bUpdateNodes, CSphString & sError ) EXCLUDES ( g_tClustersLock )
{
	ReplicationArgs_t tArgs;
	CSphScopedPtr<ReplicationCluster_t> pElem ( nullptr );
	if ( !CheckClusterStatement ( sCluster, dNames, dValues, true, sError, pElem ) )
		return false;

	CSphString sNodes;
	if ( !ClusterGetNodes ( pElem->m_sClusterNodes, pElem->m_sName, "", sError, sNodes ) )
	{
		sError.SetSprintf ( "cluster '%s', no nodes available, error '%s'", pElem->m_sName.cstr(), sError.cstr() );
		return false;
	}

	ClusterFilterNodes ( sNodes, Proto_e::REPLICATION, tArgs.m_sNodes );
	if ( tArgs.m_sNodes.IsEmpty() )
	{
		sError.SetSprintf ( "cluster '%s', no nodes available", pElem->m_sName.cstr() );
		return false;
	}

	ScopedPort_c tPort ( g_tPorts.Get() );
	if ( tPort.Get()==-1 )
	{
		sError.SetSprintf ( "cluster '%s', no replication ports available, add replication listener", pElem->m_sName.cstr() );
		return false;
	}
	tArgs.m_sListenAddr.SetSprintf ( "%s:%d", g_sListenReplicationIP.cstr(), tPort.Get() );
	tArgs.m_iListenPort = tPort.Get();
	pElem->m_tPort.Set ( tPort.Leak() );

	// global options
	tArgs.m_bNewCluster = false;
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
		DeleteClusterByName ( sCluster );
		return false;
	}

	ClusterState_e eState = tArgs.m_pCluster->WaitReady();
	bool bOk = ( eState==ClusterState_e::DONOR || eState==ClusterState_e::SYNCED );

	if ( bOk && bUpdateNodes )
	{
		sError = "";
		bOk &= ClusterAlterUpdate ( sCluster, "nodes", false, sError );
	}

	if ( !bOk )
	{
		{
			ScopedMutex_t tLock ( tArgs.m_pCluster->m_tErrorLock );
			sError = tArgs.m_pCluster->m_sError;
		}
		// need to wait recv thread to complete in case of error after worker started
		if ( tArgs.m_pCluster->m_bHasWorker )
			tArgs.m_pCluster->m_tWorkerFinished.WaitEvent ();
		DeleteClusterByName ( sCluster );
	}

	return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// cluster creates master node
/////////////////////////////////////////////////////////////////////////////

bool ClusterCreate ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, CSphString & sError ) EXCLUDES ( g_tClustersLock )
{
	ReplicationArgs_t tArgs;
	CSphScopedPtr<ReplicationCluster_t> pElem ( nullptr );
	if ( !CheckClusterStatement ( sCluster, dNames, dValues, false, sError, pElem ) )
		return false;

	ScopedPort_c tPort ( g_tPorts.Get() );
	if ( tPort.Get()==-1 )
	{
		sError.SetSprintf ( "cluster '%s', no replication ports available, add replication listener", pElem->m_sName.cstr() );
		return false;
	}
	tArgs.m_sListenAddr.SetSprintf ( "%s:%d", g_sListenReplicationIP.cstr(), tPort.Get() );
	tArgs.m_iListenPort = tPort.Get();
	pElem->m_tPort.Set ( tPort.Leak() );

	// global options
	tArgs.m_bNewCluster = true;
	tArgs.m_pCluster = pElem.Ptr();

	// need to clean up Galera system files left from previous cluster
	CSphString sClusterPath = GetClusterPath ( pElem->m_sPath );
	NewClusterClean ( sClusterPath );

	{
		ScWL_t tLock ( g_tClustersLock );
		g_hClusters.Add ( pElem.LeakPtr(), sCluster );
	}
	if ( !ReplicateClusterInit( tArgs, sError ))
	{
		DeleteClusterByName( sCluster );
		return false;
	}

	ClusterState_e eState = tArgs.m_pCluster->WaitReady();
	if ( !SaveConfigInt ( sError ) )
		return false;

	return ( eState==ClusterState_e::DONOR || eState==ClusterState_e::SYNCED );
}

// API commands that not get replicated via Galera, cluster management
enum PQRemoteCommand_e : WORD
{
	CLUSTER_DELETE				= 0,
	CLUSTER_FILE_RESERVE		= 1,
	CLUSTER_FILE_SIZE			= 2,
	CLUSTER_FILE_SEND			= 3,
	CLUSTER_INDEX_ADD_LOCAL		= 4,
	CLUSTER_SYNCED				= 5,
	CLUSTER_GET_NODES			= 6,
	CLUSTER_UPDATE_NODES		= 7,

	PQR_COMMAND_TOTAL,
	PQR_COMMAND_WRONG = PQR_COMMAND_TOTAL,
};


struct FileChunks_t
{
	int64_t m_iFileSize = 0; // size of whole file
	int m_iHashStartItem = 0; // offset in hashes vector in HASH20_SIZE items
	int m_iChunkBytes = 0; // length bytes of one hash chunk in file

	// count of chunks for file size
	int GetChunksCount() const { return int( ( m_iFileSize + m_iChunkBytes - 1) / m_iChunkBytes ); }
};


struct SyncSrc_t
{
	CSphVector<CSphString> m_dIndexFiles; // index file names (full path, file name, extension) at source node
	CSphFixedVector<CSphString> m_dBaseNames { 0 }; // index file names only (file name, extension) send from source node to destination

	CSphFixedVector<FileChunks_t> m_dChunks { 0 };
	CSphFixedVector<BYTE> m_dHashes { 0 }; // hashes of all index files
	int m_iMaxChunkBytes = 0; // max length bytes of file chunk hashed
	int64_t m_tmTook = 0; // millli-second it took to read and hash all files, used for calc agent query timeout later

	BYTE * GetFileHash ( int iFile ) const
	{
		assert ( iFile>=0 && iFile<m_dBaseNames.GetLength() );
		return m_dHashes.Begin() + iFile * HASH20_SIZE;
	}

	
	BYTE * GetChunkHash ( int iFile, int iChunk ) const
	{
		assert ( iFile>=0 && iFile<m_dBaseNames.GetLength() );
		assert ( iChunk>=0 && iChunk<m_dChunks[iFile].GetChunksCount() );
		return m_dHashes.Begin() + ( m_dChunks[iFile].m_iHashStartItem + iChunk ) * HASH20_SIZE;
	}
};

struct SyncDst_t
{
	CSphBitvec m_dNodeChunks;
	CSphFixedVector<CSphString> m_dRemotePaths { 0 };
};

// reply from remote node
struct PQRemoteReply_t
{
	CSphScopedPtr<SyncDst_t> m_pDst { nullptr };
	const SyncDst_t * m_pSharedDst = nullptr;
	CSphString m_sRemoteIndexPath;
	bool m_bIndexActive = false;
	CSphString m_sNodes;
	int64_t m_iReplyPayload = -1;
};

// query to remote node
struct PQRemoteData_t
{
	CSphString	m_sIndex;			// local name of index

	// file send args
	BYTE * m_pSendBuff = nullptr;
	int			m_iSendSize = 0;
	int64_t		m_iFileOff = 0;

	CSphString	m_sCluster;			// cluster name of index
	IndexType_e	m_eIndex = IndexType_e::ERROR_;

	// SST
	CSphString	m_sGTID;							// GTID received
	CSphFixedVector<CSphString> m_dIndexes { 0 };	// index list received
	SyncSrc_t * m_pChunks = nullptr;
	CSphString m_sIndexFileName;
	CSphString m_sRemoteFileName;
	CSphString m_sRemoteIndexPath;
};

// interface implementation to work with our agents
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

// get nodes of specific type from string
template <typename NODE_ITERATOR>
void GetNodes_T ( const CSphString & sNodes, NODE_ITERATOR & tIt )
{
	if ( sNodes.IsEmpty () )
		return;

	CSphString sTmp = sNodes;
	char * sCur = const_cast<char *>( sTmp.cstr() );
	while ( *sCur )
	{
		// skip spaces
		while ( *sCur && ( *sCur==';' || *sCur==',' || sphIsSpace ( *sCur ) )  )
			++sCur;

		const char * sAddrs = (char *)sCur;
		while ( *sCur && !( *sCur==';' || *sCur==',' || sphIsSpace ( *sCur ) )  )
			++sCur;

		// replace delimiter with 0 for ParseListener and skip delimiter itself
		if ( *sCur )
		{
			*sCur = '\0';
			sCur++;
		}

		if ( *sAddrs )
			tIt.SetNode ( sAddrs );
	}
}

// get nodes functor to collect listener API with external address
struct AgentDescIterator_t
{
	VecAgentDesc_t & m_dNodes;
	explicit AgentDescIterator_t ( VecAgentDesc_t & dNodes )
		: m_dNodes ( dNodes )
	{}

	void SetNode ( const char * sListen )
	{
		ListenerDesc_t tListen = ParseListener ( sListen );
		CHECK_LISTENER( tListen );

		if ( tListen.m_eProto!=Proto_e::SPHINX )
			return;

		if ( tListen.m_uIP==0 )
			return;

		// filter out own address to do not query itself
		if ( g_sIncomingProto.Begins ( sListen ) )
			return;

		char sAddrBuf [ SPH_ADDRESS_SIZE ];
		sphFormatIP ( sAddrBuf, sizeof(sAddrBuf), tListen.m_uIP );

		AgentDesc_t * pDesc = new AgentDesc_t;
		m_dNodes.Add( pDesc );
		pDesc->m_sAddr = sAddrBuf;
		pDesc->m_uAddr = tListen.m_uIP;
		pDesc->m_iPort = tListen.m_iPort;
		pDesc->m_bNeedResolve = false;
		pDesc->m_bPersistent = false;
		pDesc->m_iFamily = AF_INET;
	}
};

void GetNodes ( const CSphString & sNodes, VecAgentDesc_t & dNodes )
{
	AgentDescIterator_t tIt ( dNodes );
	GetNodes_T ( sNodes, tIt );
}

static AgentConn_t * CreateAgent ( const AgentDesc_t & tDesc, const PQRemoteData_t & tReq, int64_t iTimeoutMs )
{
	AgentConn_t * pAgent = new AgentConn_t;
	pAgent->m_tDesc.CloneFrom ( tDesc );

	HostDesc_t tHost;
	pAgent->m_tDesc.m_pDash = new HostDashboard_t ( tHost );

	pAgent->m_iMyConnectTimeoutMs = g_iAgentConnectTimeoutMs;
	pAgent->m_iMyQueryTimeoutMs = iTimeoutMs;

	pAgent->m_pResult = new PQRemoteAgentData_t ( tReq );

	return pAgent;
}

static void GetNodes ( const CSphString & sNodes, VecRefPtrs_t<AgentConn_t *> & dNodes, const PQRemoteData_t & tReq )
{
	VecAgentDesc_t dDesc;
	GetNodes ( sNodes, dDesc );

	dNodes.Resize ( dDesc.GetLength() );
	ARRAY_FOREACH ( i, dDesc )
		dNodes[i] = CreateAgent ( *dDesc[i], tReq, GetQueryTimeout() );
}

static void GetNodes ( const VecAgentDesc_t & dDesc, VecRefPtrs_t<AgentConn_t *> & dNodes, const PQRemoteData_t & tReq, int64_t iTimeout )
{
	dNodes.Resize ( dDesc.GetLength() );
	ARRAY_FOREACH ( i, dDesc )
		dNodes[i] = CreateAgent ( *dDesc[i], tReq, iTimeout );
}

// get nodes functor to collect listener with specific protocol
struct ListenerProtocolIterator_t
{
	StringBuilder_c m_sNodes { "," };
	Proto_e m_eProto;

	explicit ListenerProtocolIterator_t ( Proto_e eProto )
		: m_eProto ( eProto )
	{}

	void SetNode ( const char * sListen )
	{
		ListenerDesc_t tListen = ParseListener ( sListen );
		CHECK_LISTENER( tListen );

		// filter out wrong protocol
		if ( tListen.m_eProto!=m_eProto )
			return;

		char sAddrBuf [ SPH_ADDRESS_SIZE ];
		sphFormatIP ( sAddrBuf, sizeof(sAddrBuf), tListen.m_uIP );

		m_sNodes.Appendf ( "%s:%d", sAddrBuf, tListen.m_iPort );
	}
};

// utility function to filter nodes list provided at string by specific protocol
void ClusterFilterNodes ( const CSphString & sSrcNodes, Proto_e eProto, CSphString & sDstNodes )
{
	ListenerProtocolIterator_t tIt ( eProto );
	GetNodes_T ( sSrcNodes, tIt );
	sDstNodes = tIt.m_sNodes.cstr();
}

// base of API commands request and reply builders
class PQRemoteBase_c : public RequestBuilder_i, public ReplyParser_i
{
public:
	explicit PQRemoteBase_c ( PQRemoteCommand_e eCmd )
		: m_eCmd ( eCmd )
	{
	}

	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		// API header
		auto tReply = APIHeader ( tOut, SEARCHD_COMMAND_CLUSTERPQ, VER_COMMAND_CLUSTERPQ );
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
	virtual void BuildCommand ( const AgentConn_t &, ISphOutputBuffer & tOut ) const = 0;
	const PQRemoteCommand_e m_eCmd = PQR_COMMAND_WRONG;
};


// API command to remote node to delete cluster
class PQRemoteDelete_c : public PQRemoteBase_c
{
public:
	PQRemoteDelete_c()
		: PQRemoteBase_c ( CLUSTER_DELETE )
	{
	}

	void BuildCommand ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		const PQRemoteData_t & tCmd = GetReq ( tAgent );
		tOut.SendString ( tCmd.m_sCluster.cstr() );
	}

	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
	{
		tCmd.m_sCluster = tBuf.GetString();
	}

	static void BuildReply ( const PQRemoteReply_t & tRes, ISphOutputBuffer & tOut )
	{
		auto tReply = APIAnswer ( tOut );
		tOut.SendByte ( 1 );
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final
	{
		// just payload as can not recv reply with 0 size
		tReq.GetByte();
		return !tReq.GetError();
	}
};

// API command to remote node prior to file send
class PQRemoteFileReserve_c : public PQRemoteBase_c
{
public:
	PQRemoteFileReserve_c ()
		: PQRemoteBase_c ( CLUSTER_FILE_RESERVE )
	{
	}

	void BuildCommand ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		const PQRemoteData_t & tCmd = GetReq ( tAgent );
		assert ( tCmd.m_pChunks );
		tOut.SendString ( tCmd.m_sCluster.cstr() );
		tOut.SendString ( tCmd.m_sIndex.cstr() );
		tOut.SendString ( tCmd.m_sIndexFileName.cstr() );

		assert ( tCmd.m_pChunks );
		const SyncSrc_t * pSrc = tCmd.m_pChunks;
		SendArray ( pSrc->m_dBaseNames, tOut );
		SendArray ( pSrc->m_dChunks, tOut );
		SendArray ( pSrc->m_dHashes, tOut );
	}

	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
	{
		tCmd.m_sCluster = tBuf.GetString();
		tCmd.m_sIndex = tBuf.GetString();
		tCmd.m_sIndexFileName = tBuf.GetString();

		assert ( tCmd.m_pChunks );
		SyncSrc_t * pSrc = tCmd.m_pChunks;
		GetArray ( pSrc->m_dBaseNames, tBuf );
		GetArray ( pSrc->m_dChunks, tBuf );
		GetArray ( pSrc->m_dHashes, tBuf );
	}

	static void BuildReply ( const PQRemoteReply_t & tRes, ISphOutputBuffer & tOut )
	{
		assert ( tRes.m_pDst.Ptr() );
		const SyncDst_t * pDst = tRes.m_pDst.Ptr();

		auto tReply = APIAnswer ( tOut );
		tOut.SendString ( tRes.m_sRemoteIndexPath.cstr() );
		tOut.SendByte ( tRes.m_bIndexActive );
		SendArray ( pDst->m_dRemotePaths, tOut );
		tOut.SendInt ( pDst->m_dNodeChunks.GetBits() );
		tOut.SendBytes ( pDst->m_dNodeChunks.Begin(), sizeof(DWORD) * pDst->m_dNodeChunks.GetSize() );
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		PQRemoteReply_t & tRes = GetRes ( tAgent );
		assert ( tRes.m_pDst.Ptr() );
		SyncDst_t * pDst = tRes.m_pDst.Ptr();

		tRes.m_sRemoteIndexPath = tReq.GetString();
		tRes.m_bIndexActive = !!tReq.GetByte();
		GetArray ( pDst->m_dRemotePaths, tReq );
		int iBits = tReq.GetInt();
		pDst->m_dNodeChunks.Init ( iBits );
		tReq.GetBytes ( pDst->m_dNodeChunks.Begin(), sizeof(DWORD) * pDst->m_dNodeChunks.GetSize() );

		return !tReq.GetError();
	}
};


// API command to remote node of file send
class PQRemoteFileSend_c : public PQRemoteBase_c
{
public:
	PQRemoteFileSend_c ()
		: PQRemoteBase_c ( CLUSTER_FILE_SEND )
	{
	}

	void BuildCommand ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		const PQRemoteData_t & tCmd = GetReq ( tAgent );
		tOut.SendString ( tCmd.m_sCluster.cstr() );
		tOut.SendString ( tCmd.m_sRemoteFileName.cstr() );
		tOut.SendUint64 ( tCmd.m_iFileOff );
		tOut.SendInt ( tCmd.m_iSendSize );
		tOut.SendBytes ( tCmd.m_pSendBuff, tCmd.m_iSendSize );
	}

	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
	{
		tCmd.m_sCluster = tBuf.GetString();
		tCmd.m_sRemoteFileName = tBuf.GetString();
		tCmd.m_iFileOff = tBuf.GetUint64();
		tCmd.m_iSendSize = tBuf.GetInt();
		const BYTE * pData = nullptr;
		tBuf.GetBytesZerocopy ( &pData, tCmd.m_iSendSize );
		tCmd.m_pSendBuff = (BYTE *)pData;
	}

	static void BuildReply ( const PQRemoteReply_t & tRes, ISphOutputBuffer & tOut )
	{
		auto tReply = APIAnswer ( tOut );
		tOut.SendDword ( (int)tRes.m_iReplyPayload );
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		PQRemoteReply_t & tRes = GetRes ( tAgent );
		tRes.m_iReplyPayload = tReq.GetDword();
		return !tReq.GetError();
	}
};

// API command to remote node on add index into cluster
class PQRemoteIndexAdd_c : public PQRemoteBase_c
{
public:
	PQRemoteIndexAdd_c ()
		: PQRemoteBase_c ( CLUSTER_INDEX_ADD_LOCAL )
	{
	}

	void BuildCommand ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		const PQRemoteData_t & tCmd = GetReq ( tAgent );
		tOut.SendString ( tCmd.m_sCluster.cstr() );
		tOut.SendString ( tCmd.m_sIndex.cstr() );
		tOut.SendString ( tCmd.m_sRemoteIndexPath.cstr() );
		tOut.SendByte ( (BYTE)tCmd.m_eIndex );

		const SyncDst_t * pDst = GetRes ( tAgent ).m_pSharedDst;
		assert ( tCmd.m_pChunks );
		assert ( pDst );
		// remote file names at sync destination
		SendArray ( pDst->m_dRemotePaths, tOut );
		tOut.SendBytes ( tCmd.m_pChunks->m_dHashes.Begin(), pDst->m_dRemotePaths.GetLength() * HASH20_SIZE );
	}

	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
	{
		tCmd.m_sCluster = tBuf.GetString();
		tCmd.m_sIndex = tBuf.GetString();
		tCmd.m_sRemoteIndexPath = tBuf.GetString();
		tCmd.m_eIndex = (IndexType_e)tBuf.GetByte();

		assert ( tCmd.m_pChunks );
		GetArray ( tCmd.m_pChunks->m_dBaseNames, tBuf );
		tCmd.m_pChunks->m_dHashes.Reset ( tCmd.m_pChunks->m_dBaseNames.GetLength() * HASH20_SIZE );
		tBuf.GetBytes ( tCmd.m_pChunks->m_dHashes.Begin(), (int) tCmd.m_pChunks->m_dHashes.GetLengthBytes() );
	}

	static void BuildReply ( const PQRemoteReply_t & tRes, ISphOutputBuffer & tOut )
	{
		auto tReply = APIAnswer ( tOut );
		tOut.SendByte ( 1 );
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		tReq.GetByte();
		return !tReq.GetError();
	}
};

// API command to remote node to issue cluster synced callback
class PQRemoteSynced_c : public PQRemoteBase_c
{
public:
	PQRemoteSynced_c();
	void BuildCommand ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final;
	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd );
	static void BuildReply ( const PQRemoteReply_t & tRes, ISphOutputBuffer & tOut );
	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final;
};

// wrapper of PerformRemoteTasks
static bool PerformRemoteTasks ( VectorAgentConn_t & dNodes, RequestBuilder_i & tReq, ReplyParser_i & tReply, CSphString & sError )
{
	int iNodes = dNodes.GetLength();
	int iFinished = PerformRemoteTasks ( dNodes, &tReq, &tReply, g_iNodeRetry, g_iNodeRetryWait );

	if ( iFinished!=iNodes )
		sphLogDebugRpl ( "%d(%d) nodes finished well", iFinished, iNodes );

	StringBuilder_c tTmp ( ";" );
	for ( const AgentConn_t * pAgent : dNodes )
	{
		if ( !pAgent->m_sFailure.IsEmpty() )
		{
			sphWarning ( "'%s:%d': %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pAgent->m_sFailure.cstr() );
			tTmp.Appendf ( "'%s:%d': %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pAgent->m_sFailure.cstr() );
		}
	}
	if ( !tTmp.IsEmpty() )
		sError = tTmp.cstr();

	return ( iFinished==iNodes && sError.IsEmpty() );
}

// API command to remote node to get nodes it sees
class PQRemoteClusterGetNodes_c : public PQRemoteBase_c
{
public:
	PQRemoteClusterGetNodes_c ()
		: PQRemoteBase_c ( CLUSTER_GET_NODES )
	{
	}

	void BuildCommand ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		const PQRemoteData_t & tCmd = GetReq ( tAgent );
		tOut.SendString ( tCmd.m_sCluster.cstr() );
		tOut.SendString ( tCmd.m_sGTID.cstr() );
	}

	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
	{
		tCmd.m_sCluster = tBuf.GetString();
		tCmd.m_sGTID = tBuf.GetString();
	}

	static void BuildReply ( const CSphString & sNodes, ISphOutputBuffer & tOut )
	{
		auto tReply = APIAnswer ( tOut );
		tOut.SendString ( sNodes.cstr() );
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & tAgent ) const final
	{
		PQRemoteReply_t & tRes = GetRes ( tAgent );
		tRes.m_sNodes = tReq.GetString();

		return !tReq.GetError();
	}
};

// API command to remote node to update nodes by nodes it sees
class PQRemoteClusterUpdateNodes_c : public PQRemoteBase_c
{
public:
	PQRemoteClusterUpdateNodes_c()
		: PQRemoteBase_c ( CLUSTER_UPDATE_NODES )
	{
	}

	void BuildCommand ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		const PQRemoteData_t & tCmd = GetReq ( tAgent );
		tOut.SendString ( tCmd.m_sCluster.cstr() );
	}

	static void ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
	{
		tCmd.m_sCluster = tBuf.GetString();
	}

	static void BuildReply ( ISphOutputBuffer & tOut )
	{
		auto tReply = APIAnswer ( tOut );
		tOut.SendByte ( 1 );
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final
	{
		// just payload as can not recv reply with 0 size
		tReq.GetByte();
		return !tReq.GetError();
	}
};

static void ReportClusterError ( const CSphString & sCluster, const CSphString & sError )
{
	ScRL_t tLock ( g_tClustersLock );
	ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
	if ( !ppCluster )
		return;

	ReplicationCluster_t * pCluster = *ppCluster;
	if ( !pCluster->m_sError.IsEmpty() )
		return;

	ScopedMutex_t tErrorLock ( pCluster->m_tErrorLock );
	pCluster->m_sError = sError;
}

// handler of all remote commands via API parsed at daemon as SEARCHD_COMMAND_CLUSTERPQ
void HandleCommandClusterPq ( ISphOutputBuffer & tOut, WORD uCommandVer, InputBuffer_c & tBuf, const char * sClient )
{
	if ( !CheckCommandVersion ( uCommandVer, VER_COMMAND_CLUSTERPQ, tOut ) )
		return;

	PQRemoteCommand_e eClusterCmd = (PQRemoteCommand_e)tBuf.GetWord();
	if ( eClusterCmd!=CLUSTER_FILE_SEND )
		sphLogDebugRpl ( "remote cluster command %d, client %s", (int)eClusterCmd, sClient );

	CSphString sError;
	PQRemoteData_t tCmd;
	PQRemoteReply_t tRes;
	bool bOk = false;
	switch ( eClusterCmd )
	{
		case CLUSTER_DELETE:
		{
			PQRemoteDelete_c::ParseCommand ( tBuf, tCmd );
			bOk = RemoteClusterDelete ( tCmd.m_sCluster, sError );
			bOk &= SaveConfigInt(sError);
			if ( bOk )
				PQRemoteDelete_c::BuildReply ( tRes, tOut );
		}
		break;

		case CLUSTER_FILE_RESERVE:
		{
			SyncSrc_t tSrc;
			tCmd.m_pChunks = &tSrc;
			tRes.m_pDst = new SyncDst_t();
			PQRemoteFileReserve_c::ParseCommand ( tBuf, tCmd );
			bOk = RemoteFileReserve ( tCmd, tRes, sError );
			if ( bOk )
				PQRemoteFileReserve_c::BuildReply ( tRes, tOut );
		}
		break;

		case CLUSTER_FILE_SEND:
		{
			PQRemoteFileSend_c::ParseCommand ( tBuf, tCmd );
			bOk = RemoteFileStore ( tCmd, tRes, sError );
			if ( bOk )
				PQRemoteFileSend_c::BuildReply ( tRes, tOut );
		}
		break;

		case CLUSTER_INDEX_ADD_LOCAL:
		{
			SyncSrc_t tSrc;
			tCmd.m_pChunks = &tSrc;
			PQRemoteIndexAdd_c::ParseCommand ( tBuf, tCmd );
			bOk = RemoteLoadIndex ( tCmd, tRes, sError );
			if ( bOk )
				PQRemoteIndexAdd_c::BuildReply ( tRes, tOut );
		}
		break;

		case CLUSTER_SYNCED:
		{
			PQRemoteSynced_c::ParseCommand ( tBuf, tCmd );
			bOk = RemoteClusterSynced ( tCmd, sError );
			if ( bOk )
				PQRemoteSynced_c::BuildReply ( tRes, tOut );
		}
		break;

		case CLUSTER_GET_NODES:
		{
			PQRemoteClusterGetNodes_c::ParseCommand ( tBuf, tCmd );
			bOk = RemoteClusterGetNodes ( tCmd.m_sCluster, tCmd.m_sGTID, sError, tRes.m_sNodes );
			if ( bOk )
				PQRemoteClusterGetNodes_c::BuildReply ( tRes.m_sNodes, tOut );
		}
		break;

		case CLUSTER_UPDATE_NODES:
		{
			PQRemoteClusterUpdateNodes_c::ParseCommand ( tBuf, tCmd );
			bOk = RemoteClusterUpdateNodes ( tCmd.m_sCluster, nullptr, sError );
			if ( bOk )
				PQRemoteClusterUpdateNodes_c::BuildReply ( tOut );
		}
		break;

		default:
			sError.SetSprintf ( "INTERNAL ERROR: unhandled command %d", (int)eClusterCmd );
			break;
	}

	if ( eClusterCmd!=CLUSTER_FILE_SEND || !bOk )
		sphLogDebugRpl ( "remote cluster command %d, client %s - %s", (int)eClusterCmd, sClient, ( bOk ? "ok" : "error" ) );

	if ( !bOk )
	{
		auto tReply = APIHeader ( tOut, SEARCHD_ERROR );
		tOut.SendString ( sError.cstr() );
		sphLogFatal ( "%s", sError.cstr() );
		ReportClusterError ( tCmd.m_sCluster, sError );
	}
}

// command at remote node for CLUSTER_DELETE to delete cluster
bool RemoteClusterDelete ( const CSphString & sCluster, CSphString & sError ) EXCLUDES ( g_tClustersLock )
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
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// cluster deletes
/////////////////////////////////////////////////////////////////////////////

// cluster delete every node then itself
bool ClusterDelete ( const CSphString & sCluster, CSphString & sError, CSphString & sWarning ) EXCLUDES (g_tClustersLock)
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
			ScopedMutex_t tNodesLock ( (*ppCluster)->m_tViewNodesLock );
			sNodes = (*ppCluster)->m_sViewNodes;
		}
	}

	if ( !sNodes.IsEmpty() )
	{
		PQRemoteData_t tCmd;
		tCmd.m_sCluster = sCluster;
		VecRefPtrs_t<AgentConn_t *> dNodes;
		GetNodes ( sNodes, dNodes, tCmd );
		PQRemoteDelete_c tReq;
		if ( dNodes.GetLength() && !PerformRemoteTasks ( dNodes, tReq, tReq, sError ) )
			return false;
	}

	// after all nodes finished
	bool bOk = RemoteClusterDelete ( sCluster, sError );
	bOk &= SaveConfigInt(sError);

	return bOk;
}

void ReplicationCluster_t::UpdateIndexHashes()
{
	m_dIndexHashes.Resize ( 0 );
	for ( const CSphString & sIndex : m_dIndexes )
		m_dIndexHashes.Add ( sphFNV64 ( sIndex.cstr() ) );

	m_dIndexHashes.Uniq();
}

// cluster ALTER statement that removes index from cluster but keep it at daemon
static bool ClusterAlterDrop ( const CSphString & sCluster, const CSphString & sIndex, CSphString & sError )
{
	RtAccum_t tAcc { false };
	tAcc.AddCommand ( ReplicationCommand_e::CLUSTER_ALTER_DROP, sCluster, sIndex );
	return HandleCmdReplicate ( tAcc, sError );
}

static bool SyncSigCompare ( int iFile, const CSphString & sName, const SyncSrc_t & tSrc, CSphBitvec & tDst, CSphVector<BYTE> & dBuf, CSphString & sError )
{
	const FileChunks_t & tChunk = tSrc.m_dChunks[iFile];
	SHA1_c tHashFile;
	SHA1_c tHashChunk;
	tHashFile.Init();

	dBuf.Resize ( HASH20_SIZE * 2 + tChunk.m_iChunkBytes );
	BYTE * pReadData = dBuf.Begin();
	BYTE * pHashFile = dBuf.Begin() + tChunk.m_iChunkBytes;
	BYTE * pHashChunk = dBuf.Begin() + tChunk.m_iChunkBytes + HASH20_SIZE;

	CSphAutofile tIndexFile;

	if ( tIndexFile.Open ( sName, SPH_O_READ, sError )<0 )
		return false;

	int iChunk = 0;
	int64_t iReadTotal = 0;
	while ( iReadTotal<tChunk.m_iFileSize )
	{
		int64_t iLeftTotal = tChunk.m_iFileSize - iReadTotal;
		int iLeft = Min ( iLeftTotal, tChunk.m_iChunkBytes );
		iReadTotal += iLeft;

		if ( !tIndexFile.Read ( pReadData, iLeft, sError ) )
			return false;

		// update whole file hash
		tHashFile.Update ( pReadData, iLeft );

		// update and flush chunk hash
		tHashChunk.Init();
		tHashChunk.Update ( pReadData, iLeft );
		tHashChunk.Final ( pHashChunk );

		if ( memcmp ( pHashChunk, tSrc.GetChunkHash ( iFile, iChunk ), HASH20_SIZE )==0 )
			tDst.BitSet ( tChunk.m_iHashStartItem + iChunk );
		iChunk++;
	}

	tHashFile.Final ( pHashFile );

	if ( memcmp ( pHashFile, tSrc.GetFileHash ( iFile ), HASH20_SIZE )==0 )
		tDst.BitSet ( iFile );

	return true;
}

static bool SyncSigVerify ( const SyncSrc_t & tSrc, CSphString & sError )
{
	CSphAutoreader tIndexFile;
	SHA1_c tHashFile;

	ARRAY_FOREACH ( iFile, tSrc.m_dBaseNames )
	{
		const CSphString & sFileName = tSrc.m_dBaseNames[iFile];
		if ( !tIndexFile.Open ( sFileName, sError ) )
			return false;

		tHashFile.Init();
		const int64_t iFileSize = tIndexFile.GetFilesize();
		int64_t iReadTotal = 0;
		while ( iReadTotal<iFileSize )
		{
			const int64_t iLeft = iFileSize - iReadTotal;

			// iGot is always less INT_MAX as CSphReader allocates buffer of int size max
			const BYTE * pData = nullptr;
			const int iGot = tIndexFile.GetBytesZerocopy ( &pData, iLeft );
			iReadTotal += iGot;

			// update whole file hash
			tHashFile.Update ( pData, iGot );
		}

		BYTE dHash [HASH20_SIZE];
		tHashFile.Final ( dHash );
		tIndexFile.Close();

		if ( memcmp ( dHash, tSrc.GetFileHash ( iFile ), HASH20_SIZE )!=0 )
		{
			sError.SetSprintf ( "%s sha1 does not match, expected %s, got %s", sFileName.cstr(),
				BinToHex ( dHash, HASH20_SIZE ).cstr(), BinToHex ( tSrc.GetFileHash ( iFile ), HASH20_SIZE ).cstr() );
			return false;
		}
	}
		
	return true;
}

// command at remote node for CLUSTER_FILE_RESERVE to check
// - file could be allocated on disk at cluster path and reserve disk space for a file
// - or make sure that index has exact same index file, ie sha1 matched
bool RemoteFileReserve ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError )
{
	sphLogDebugRpl ( "reserve index '%s'", tCmd.m_sIndex.cstr() );

	assert ( tCmd.m_pChunks );
	assert ( tRes.m_pDst.Ptr() );
	// use index path first
	{
		ServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sIndex );
		ServedDescRPtr_c pDesc ( pServed );
		if ( ServedDesc_t::IsMutable ( pDesc ) )
		{
			tRes.m_bIndexActive = true;
			tRes.m_sRemoteIndexPath = pDesc->m_sIndexPath;
			( (RtIndex_i *)pDesc->m_pIndex )->ProhibitSave();
		}
	}

	tRes.m_pDst->m_dRemotePaths.Reset ( tCmd.m_pChunks->m_dBaseNames.GetLength() );

	// use cluster path as head of index path or existed index path
	if ( tRes.m_bIndexActive )
	{
		// set index files names into existing index files
		ARRAY_FOREACH ( iFile, tCmd.m_pChunks->m_dBaseNames )
		{
			const CSphString & sFile = tCmd.m_pChunks->m_dBaseNames[iFile];
			tRes.m_pDst->m_dRemotePaths[iFile].SetSprintf ( "%s.%s", tRes.m_sRemoteIndexPath.cstr(), GetExtension ( sFile.cstr() ) );
		}
	} else
	{
		CSphString sIndexPath;
		if ( !GetClusterPath ( tCmd.m_sCluster, sIndexPath, sError ) )
			return false;
		// index in its own directory
		tRes.m_sRemoteIndexPath.SetSprintf ( "%s/%s", sIndexPath.cstr(), tCmd.m_sIndexFileName.cstr() );
		MkDir ( tRes.m_sRemoteIndexPath.cstr() );

		// set index files names into cluster folder
		ARRAY_FOREACH ( iFile, tCmd.m_pChunks->m_dBaseNames )
		{
			const CSphString & sFile = tCmd.m_pChunks->m_dBaseNames[iFile];
			tRes.m_pDst->m_dRemotePaths[iFile].SetSprintf ( "%s/%s", tRes.m_sRemoteIndexPath.cstr(), sFile.cstr() );
		}
		tRes.m_sRemoteIndexPath.SetSprintf ( "%s/%s/%s", sIndexPath.cstr(), tCmd.m_sIndexFileName.cstr(), tCmd.m_sIndexFileName.cstr() );
	}

	int iBits = tCmd.m_pChunks->m_dChunks.Last().m_iHashStartItem + tCmd.m_pChunks->m_dChunks.Last().GetChunksCount();
	tRes.m_pDst->m_dNodeChunks.Init ( iBits );

	CSphVector<BYTE> dReadBuf;

	// check file exists, same size and same hash
	ARRAY_FOREACH ( iFile, tRes.m_pDst->m_dRemotePaths )
	{
		const CSphString & sFile = tRes.m_pDst->m_dRemotePaths[iFile];
		const FileChunks_t & tFile = tCmd.m_pChunks->m_dChunks[iFile];
		
		if ( sphIsReadable ( sFile ) )
		{
			int64_t iLen = 0;
			{
				CSphAutofile tOut ( sFile, SPH_O_READ, sError, false );
				if ( tOut.GetFD()<0 )
					return false;

				iLen = tOut.GetSize();
			}

			// check only in case size matched
			if ( iLen==tFile.m_iFileSize )
			{
				if ( !SyncSigCompare ( iFile, sFile, *tCmd.m_pChunks, tRes.m_pDst->m_dNodeChunks, dReadBuf, sError ) )
					return false;
				else
					continue;
			}
		}


		// need to create file with specific size
		CSphAutofile tOut ( sFile, SPH_O_NEW, sError, false );
		if ( tOut.GetFD()<0 )
			return false;

		int64_t iLen = sphSeek ( tOut.GetFD(), tFile.m_iFileSize, SEEK_SET );
		if ( iLen!=tFile.m_iFileSize )
		{
			if ( iLen<0 )
			{
				sError.SetSprintf ( "error: %d '%s'", errno, strerrorm ( errno ) );
			} else
			{
				sError.SetSprintf ( "error, expected: " INT64_FMT ", got " INT64_FMT, tFile.m_iFileSize, iLen );
			}

			return false;
		}
	}

	return true;
}

// command at remote node for CLUSTER_FILE_SEND to store data into file, data size and file offset defined by sender
bool RemoteFileStore ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError )
{
	tRes.m_iReplyPayload = -1;
	CSphAutofile tOut ( tCmd.m_sRemoteFileName, O_RDWR, sError, false );
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
			sError.SetSprintf ( "error, expected: " INT64_FMT ", got " INT64_FMT, tCmd.m_iFileOff, iPos );
		}
		return false;
	}

	if ( !sphWrite ( tOut.GetFD(), tCmd.m_pSendBuff, tCmd.m_iSendSize ) )
	{
		sError.SetSprintf ( "write error: %s", strerrorm(errno) );
		return false;
	}

	tRes.m_iReplyPayload = tCmd.m_iSendSize;

	return true;
}

// command at remote node for CLUSTER_INDEX_ADD_LOCAL to check sha1 of index file matched and load index into daemon
bool RemoteLoadIndex ( const PQRemoteData_t & tCmd, PQRemoteReply_t & tRes, CSphString & sError )
{
	assert ( tCmd.m_pChunks );
	CSphString sType = GetTypeName ( tCmd.m_eIndex );
	if ( tCmd.m_eIndex!=IndexType_e::PERCOLATE && tCmd.m_eIndex!=IndexType_e::RT )
	{
		sError.SetSprintf ( "unsupported type '%s' in index '%s'", sType.cstr(), tCmd.m_sIndex.cstr() );
		return false;
	}

	sphLogDebugRpl ( "verify index '%s' from %s", tCmd.m_sIndex.cstr(), tCmd.m_sRemoteIndexPath.cstr() );

	// check that size matched and sha1 matched prior to loading index
	if ( !SyncSigVerify ( *tCmd.m_pChunks, sError ) )
		return false;

	sphLogDebugRpl ( "loading index '%s' into cluster '%s' from %s", tCmd.m_sIndex.cstr(), tCmd.m_sCluster.cstr(), tCmd.m_sRemoteIndexPath.cstr() );

	CSphConfigSection hIndex;
	hIndex.Add ( CSphVariant ( tCmd.m_sRemoteIndexPath.cstr() ), "path" );
	hIndex.Add ( CSphVariant ( sType.cstr() ), "type" );
	// dummy
	hIndex.Add ( CSphVariant ( "text" ), "rt_field" );
	hIndex.Add ( CSphVariant ( "gid" ), "rt_attr_uint" );
	if ( !LoadIndex ( hIndex, tCmd.m_sIndex, tCmd.m_sCluster, sError ) )
	{
		sError.SetSprintf ( "failed to load index '%s': %s", tCmd.m_sIndex.cstr(), sError.cstr() );
		return false;
	}

	return true;
}

struct RemoteFileState_t
{
	CSphString m_sRemoteIndexPath;
	const AgentDesc_t * m_pAgentDesc = nullptr;
	const SyncSrc_t * m_pSyncSrc = nullptr;
	const SyncDst_t * m_pSyncDst = nullptr;
};

struct FileReader_t
{
	CSphAutofile m_tFile;
	PQRemoteData_t m_tArgs;
	const AgentDesc_t * m_pAgentDesc = nullptr;

	const SyncSrc_t * m_pSyncSrc = nullptr;
	const SyncDst_t * m_pSyncDst = nullptr;

	int m_iFile = -1;
	int m_iChunk = -1;
	int m_bDone = false;

	int m_iPackets = 1;
};

static bool ReadChunk ( int iChunk, FileReader_t & tReader, StringBuilder_c & sErrors )
{
	assert ( tReader.m_pSyncSrc );
	const SyncSrc_t * pSrc = tReader.m_pSyncSrc;
	const FileChunks_t & tChunk = pSrc->m_dChunks[tReader.m_iFile];

	int iSendSize = tChunk.m_iChunkBytes;
	if ( iChunk==tChunk.GetChunksCount()-1 ) // calculate file tail size for last chunk
		iSendSize = tChunk.m_iFileSize - tChunk.m_iChunkBytes * iChunk;

	tReader.m_tArgs.m_iFileOff = (int64_t)tChunk.m_iChunkBytes * iChunk;
	tReader.m_tArgs.m_iSendSize = iSendSize;

	bool bSeek = ( iChunk!=tReader.m_iChunk+1 );
	tReader.m_iChunk = iChunk;

	// seek to new chunk
	if ( bSeek )
	{
		int64_t iNewPos = sphSeek ( tReader.m_tFile.GetFD(), tReader.m_tArgs.m_iFileOff, SEEK_SET );
		if ( iNewPos!=tReader.m_tArgs.m_iFileOff )
		{
			if ( iNewPos<0 )
				sErrors.Appendf ( "seek error %d '%s'", errno, strerrorm (errno) );
			else
				sErrors.Appendf ( "seek error, expected: " INT64_FMT ", got " INT64_FMT, tReader.m_tArgs.m_iFileOff, iNewPos );

			return false;
		}
	}

	CSphString sReaderError;
	if ( !tReader.m_tFile.Read ( tReader.m_tArgs.m_pSendBuff, iSendSize, sReaderError ) )
	{
		sErrors += sReaderError.cstr();
		return false;
	}

	return true;
}

static bool Next ( FileReader_t & tReader, StringBuilder_c & sErrors )
{
	assert ( tReader.m_pSyncSrc );
	assert ( tReader.m_pSyncDst );
	assert ( !tReader.m_bDone );

	const SyncSrc_t * pSrc = tReader.m_pSyncSrc;
	const SyncDst_t * pDst = tReader.m_pSyncDst;

	// search for next chunk in same file
	const FileChunks_t & tChunk = pSrc->m_dChunks[tReader.m_iFile];
	int iChunk = tReader.m_iChunk+1;
	const int iCount = tChunk.GetChunksCount();
	for ( ; iChunk<iCount; iChunk++ )
	{
		if ( !pDst->m_dNodeChunks.BitGet ( tChunk.m_iHashStartItem + iChunk ) )
			break;
	}
	if ( iChunk<iCount )
		return ReadChunk ( iChunk, tReader, sErrors );

	// search for next file
	const int iFiles = pSrc->m_dBaseNames.GetLength();
	int iFile = tReader.m_iFile+1;
	for ( ; iFile<iFiles; iFile++ )
	{
		if ( !pDst->m_dNodeChunks.BitGet ( iFile ) )
			break;
	}
	tReader.m_iFile = iFile;
	tReader.m_iChunk = -1;

	// no more files left
	if ( iFile==iFiles )
	{
		tReader.m_bDone = true;
		return true;
	}

	// look for chunk in next file
	tReader.m_tFile.Close();
	CSphString sReaderError;
	if ( tReader.m_tFile.Open ( pSrc->m_dIndexFiles[iFile], SPH_O_READ, sReaderError, false )<0 )
	{
		sErrors += sReaderError.cstr();
		return false;
	}
	tReader.m_tArgs.m_sRemoteFileName = pDst->m_dRemotePaths[iFile];

	const FileChunks_t & tNewChunk = pSrc->m_dChunks[iFile];
	int iNewChunk = 0;
	const int iNewCount = tNewChunk.GetChunksCount();
	for ( ; iNewChunk<iNewCount; iNewChunk++ )
	{
		if ( !pDst->m_dNodeChunks.BitGet ( tNewChunk.m_iHashStartItem + iNewChunk ) )
			break;
	}
	assert ( iNewChunk<iNewCount );
	assert ( !pDst->m_dNodeChunks.BitGet ( tNewChunk.m_iHashStartItem + iNewChunk ) );
	return ReadChunk ( iNewChunk, tReader, sErrors );
}

static bool InitReader ( FileReader_t & tReader, StringBuilder_c & sErrors )
{
	assert ( tReader.m_pSyncSrc );
	assert ( tReader.m_pSyncDst );

	const SyncSrc_t * pSrc = tReader.m_pSyncSrc;
	const SyncDst_t * pDst = tReader.m_pSyncDst;

	const int iFiles = pSrc->m_dBaseNames.GetLength();
	int iFile = 0;
	for ( ; iFile<iFiles; iFile++ )
	{
		if ( !pDst->m_dNodeChunks.BitGet ( iFile ) )
			break;
	}
	assert ( !pDst->m_dNodeChunks.BitGet ( iFile ) );
	tReader.m_iFile = iFile;

	const FileChunks_t & tChunk = pSrc->m_dChunks[iFile];
	int iChunk = 0;
	const int iCount = tChunk.GetChunksCount();

	for ( ; iChunk<iCount; iChunk++ )
	{
		if ( !pDst->m_dNodeChunks.BitGet ( tChunk.m_iHashStartItem + iChunk ) )
			break;
	}
	assert ( iChunk<iCount );
	assert ( !pDst->m_dNodeChunks.BitGet ( tChunk.m_iHashStartItem + iChunk ) );

	tReader.m_tArgs.m_sRemoteFileName = pDst->m_dRemotePaths[iFile];

	CSphString sError;
	if ( tReader.m_tFile.Open ( pSrc->m_dIndexFiles[iFile], SPH_O_READ, sError, false )<0 )
	{
		sErrors += sError.cstr();
		return false;
	}

	return ReadChunk ( iChunk, tReader, sErrors );
}

static void ReportSendStat ( const VecRefPtrs_t<AgentConn_t *> & dNodes, const CSphFixedVector<FileReader_t> & dReaders, const CSphString & sCluster, const CSphString & sIndex )
{
	if ( g_eLogLevel<SPH_LOG_RPL_DEBUG )
		return;

	int iTotal = 0;
	StringBuilder_c tTmp;
	tTmp.Appendf ( "file sync packets sent '%s:%s' to ", sCluster.cstr(), sIndex.cstr() );
	ARRAY_FOREACH ( iAgent, dNodes )
	{
		const AgentConn_t * pAgent = dNodes[iAgent];
		const FileReader_t & tReader = dReaders[iAgent];

		tTmp.Appendf ( "'%s:%d':%d,", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, tReader.m_iPackets );
		iTotal += tReader.m_iPackets;
	}
	tTmp.Appendf ( " total:%d", iTotal );

	sphLogDebugRpl ( "%s", tTmp.cstr() );
}

// send file to multiple nodes by chunks as API command CLUSTER_FILE_SEND
static bool SendFile ( const SyncSrc_t & tSigSrc, const CSphVector<RemoteFileState_t> & dDesc, const CSphString & sCluster, const CSphString & sIndex, CSphString & sError )
{
	StringBuilder_c tErrors ( ";" );

	// setup buffers
	CSphFixedVector<BYTE> dReadBuf ( tSigSrc.m_iMaxChunkBytes * dDesc.GetLength() );
	CSphFixedVector<FileReader_t> dReaders  ( dDesc.GetLength() );

	ARRAY_FOREACH ( iNode, dReaders )
	{
		// setup file readers
		FileReader_t & tReader = dReaders[iNode];
		tReader.m_tArgs.m_sCluster = sCluster;
		tReader.m_pAgentDesc = dDesc[iNode].m_pAgentDesc;
		tReader.m_tArgs.m_pSendBuff = dReadBuf.Begin() + tSigSrc.m_iMaxChunkBytes * iNode;
		tReader.m_pSyncSrc = dDesc[iNode].m_pSyncSrc;
		tReader.m_pSyncDst = dDesc[iNode].m_pSyncDst;

		if ( !InitReader ( tReader, tErrors ) )
		{
			sError = tErrors.cstr();
			return false;
		}

		assert ( !tReader.m_bDone );
	}

	// create agents
	VecRefPtrs_t<AgentConn_t *> dNodes;
	dNodes.Resize ( dReaders.GetLength() );
	ARRAY_FOREACH ( i, dReaders )
		dNodes[i] = CreateAgent ( *dReaders[i].m_pAgentDesc, dReaders[i].m_tArgs, GetQueryTimeout() );

	// submit initial jobs
	CSphRefcountedPtr<RemoteAgentsObserver_i> tReporter ( GetObserver() );
	PQRemoteFileSend_c tReq;
	ScheduleDistrJobs ( dNodes, &tReq, &tReq, tReporter, g_iNodeRetry, g_iNodeRetryWait );

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
			if ( tReader.m_bDone )
				continue;

			bool bSendOk = ( PQRemoteBase_c::GetRes ( *pAgent ).m_iReplyPayload==tReader.m_tArgs.m_iSendSize );

			// report errors first
			if ( !pAgent->m_sFailure.IsEmpty() && !bSendOk )
			{
				tErrors.Appendf ( "'%s:%d' %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pAgent->m_sFailure.cstr() );
				tReader.m_bDone = true;
				pAgent->m_sFailure = "";
				continue;
			}
			if ( !pAgent->m_sFailure.IsEmpty() && bSendOk )
			{
				sphWarning ( "'%s:%d' %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pAgent->m_sFailure.cstr() );
				pAgent->m_sFailure = "";
			}

			if ( !Next ( tReader, tErrors ) )
			{
				pAgent->m_bSuccess = false;
				tReader.m_bDone = true;
				continue;
			}

			if ( tReader.m_bDone )
				continue;

			// remove agent from main vector
			pAgent->Release();

			AgentConn_t * pNextJob = CreateAgent ( *tReader.m_pAgentDesc, tReader.m_tArgs, GetQueryTimeout() );
			dNodes[iAgent] = pNextJob;

			dNewNode[0] = pNextJob;
			ScheduleDistrJobs ( dNewNode, &tReq, &tReq, tReporter, g_iNodeRetry, g_iNodeRetryWait );
			// agent managed at main vector
			dNewNode[0] = nullptr;

			// reset done flag to process new item
			bDone = false;
			tReader.m_iPackets++;
		}
	}

	if ( !tErrors.IsEmpty() )
		sError = tErrors.cstr();

	ReportSendStat ( dNodes, dReaders, sCluster, sIndex );

	return true;
}

static bool SyncSigBegin ( SyncSrc_t & tSync, CSphString & sError )
{
	int64_t tmStart = sphMicroTimer();

	const int iFiles =  tSync.m_dIndexFiles.GetLength();
	tSync.m_dBaseNames.Reset ( iFiles );
	tSync.m_dChunks.Reset ( iFiles );

	const int iMinChunkBuf = Min ( g_iMaxPacketSize * 3 / 4, 8 * 1024 );
	int iHashes = iFiles;

	for ( int i=0; i<iFiles; i++ )
	{
		const CSphString & sFile = tSync.m_dIndexFiles[i];
		CSphAutofile tIndexFile;
		if ( tIndexFile.Open ( sFile, SPH_O_READ, sError )<0 )
			return false;

		tSync.m_dBaseNames[i] = GetBaseName ( sFile );

		int64_t iFileSize = tIndexFile.GetSize();

		// rsync uses sqrt ( iSize ) but that make too small buffers
		int iChunkBytes = iFileSize / 2000;
		// no need too small chunks
		iChunkBytes = Max ( iChunkBytes, iMinChunkBuf );
		if ( iFileSize<iChunkBytes )
			iChunkBytes = iFileSize;

		FileChunks_t & tSlice = tSync.m_dChunks[i];
		tSlice.m_iFileSize = iFileSize;
		tSlice.m_iChunkBytes = iChunkBytes;
		tSlice.m_iHashStartItem = iHashes;

		iHashes += tSlice.GetChunksCount();
		tSync.m_iMaxChunkBytes = Max ( tSlice.m_iChunkBytes, tSync.m_iMaxChunkBytes );
	}
	tSync.m_dHashes.Reset ( iHashes * HASH20_SIZE );

	SHA1_c tHashFile;
	SHA1_c tHashChunk;

	CSphFixedVector<BYTE> dReadBuf ( tSync.m_iMaxChunkBytes );
	CSphAutofile tIndexFile;

	for ( int iFile=0; iFile<iFiles; iFile++ )
	{
		const CSphString & sFile = tSync.m_dIndexFiles[iFile];
		if ( tIndexFile.Open ( sFile, SPH_O_READ, sError )<0 )
			return false;

		const FileChunks_t & tChunk = tSync.m_dChunks[iFile];
		tHashFile.Init();

		int iChunk = 0;
		int64_t iReadTotal = 0;
		while ( iReadTotal<tChunk.m_iFileSize )
		{
			int64_t iLeftTotal = tChunk.m_iFileSize - iReadTotal;
			int iLeft = Min ( iLeftTotal, tChunk.m_iChunkBytes );
			iReadTotal += iLeft;

			if ( !tIndexFile.Read ( dReadBuf.Begin(), iLeft, sError ) )
				return false;

			// update whole file hash
			tHashFile.Update ( dReadBuf.Begin(), iLeft );

			// update and flush chunk hash
			tHashChunk.Init();
			tHashChunk.Update ( dReadBuf.Begin(), iLeft );
			tHashChunk.Final ( tSync.GetChunkHash ( iFile, iChunk ) );
			iChunk++;
		}

		tIndexFile.Close();
		tHashFile.Final ( tSync.GetFileHash ( iFile ) );
	}

	tSync.m_tmTook = ( sphMicroTimer() - tmStart ) / 1000;

	return true;
}

struct SendStatesGuard_t : public ISphNoncopyable
{
	CSphVector<const SyncDst_t *> m_dFree;
	CSphVector<RemoteFileState_t> m_dSendFiles;
	CSphVector<RemoteFileState_t> m_dActivateIndexes;

	~SendStatesGuard_t()
	{
		ARRAY_FOREACH ( i, m_dFree )
			SafeDelete ( m_dFree[i] );
	}
};

class IndexSaveGuard_t : public ISphNoncopyable
{
public:
	explicit IndexSaveGuard_t ( const ServedDesc_t * pIndexDesc )
		: m_pIndexDesc ( pIndexDesc )
	{}

	void EnableSave()
	{
		if ( !m_pIndexDesc )
			return;

		RtIndex_i * pIndex = (RtIndex_i *)m_pIndexDesc->m_pIndex;
		pIndex->EnableSave();

		m_pIndexDesc = nullptr;
	}

	~IndexSaveGuard_t()
	{
		EnableSave();
	}

private:
	const ServedDesc_t * m_pIndexDesc = nullptr;
};

// send local index to remote nodes via API
static bool NodesReplicateIndex ( const CSphString & sCluster, const CSphString & sIndex, const VecAgentDesc_t & dDesc, const ServedDesc_t * pIndexDesc, CSphString & sError )
{
	assert ( dDesc.GetLength() );

	CSphVector<CSphString> dIndexFiles;
	IndexSaveGuard_t tIndexSaveGuard ( pIndexDesc );
	RtIndex_i * pIndex = (RtIndex_i *)pIndexDesc->m_pIndex;
	pIndex->LockFileState ( dIndexFiles );

	CSphString sIndexPath = pIndexDesc->m_sIndexPath;
	IndexType_e eType = pIndexDesc->m_eType;

	assert ( !sIndexPath.IsEmpty() );
	assert ( dIndexFiles.GetLength() );

	sphLogDebugRpl ( "calculate sha1 of index files chunks '%s'", sIndex.cstr() );

	SyncSrc_t tSigSrc;
	tSigSrc.m_dIndexFiles.SwapData ( dIndexFiles );
	if ( !SyncSigBegin ( tSigSrc, sError ) )
		return false;

	sphLogDebugRpl ( "calculated sha1 of index '%s', files %d, hashes %d", sIndex.cstr(), tSigSrc.m_dIndexFiles.GetLength(), tSigSrc.m_dHashes.GetLength() );

	int64_t tmLongOpTimeout = GetQueryTimeout ( tSigSrc.m_tmTook * 3 ); // timeout = sha verify (of all index files) + preload (of all index files) +1 (for slow io)

	SendStatesGuard_t tStatesGuard;
	CSphVector<RemoteFileState_t> & dSendStates = tStatesGuard.m_dSendFiles;
	CSphVector<RemoteFileState_t> & dActivateIndexes = tStatesGuard.m_dActivateIndexes;
	{
		PQRemoteData_t tAgentData;
		tAgentData.m_sCluster = sCluster;
		tAgentData.m_sIndex = sIndex;
		tAgentData.m_pChunks = &tSigSrc;
		tAgentData.m_sIndexFileName = GetBaseName ( sIndexPath );

		VecRefPtrs_t<AgentConn_t *> dNodes;
		GetNodes ( dDesc, dNodes, tAgentData, tmLongOpTimeout );
		for ( AgentConn_t * pAgent : dNodes )
			PQRemoteBase_c::GetRes ( *pAgent ).m_pDst = new SyncDst_t();

		sphLogDebugRpl ( "reserve index '%s' at %d nodes with timeout %d.%03d sec", sIndex.cstr(), dNodes.GetLength(), (int)( tmLongOpTimeout/1000 ), (int)( tmLongOpTimeout%1000 ) );

		PQRemoteFileReserve_c tReq;
		bool bOk = PerformRemoteTasks ( dNodes, tReq, tReq, sError );
		sphLogDebugRpl ( "reserved index '%s' - %s", sIndex.cstr(), ( bOk ? "ok" : "failed" ) );
		if ( !bOk )
			return false;

		// collect remote file states and make list nodes and files to send
		assert ( dDesc.GetLength()==dNodes.GetLength() );
		ARRAY_FOREACH ( iNode, dNodes )
		{
			 PQRemoteReply_t & tRes = PQRemoteBase_c::GetRes ( *dNodes[iNode] );
			 const CSphBitvec & tFilesDst = tRes.m_pDst->m_dNodeChunks;
			 bool bSameFiles = ( tSigSrc.m_dBaseNames.GetLength()==tRes.m_pDst->m_dRemotePaths.GetLength() );
			 bool bSameHashes = ( tSigSrc.m_dHashes.GetLength() / HASH20_SIZE==tRes.m_pDst->m_dNodeChunks.GetBits() );
			 if ( !bSameFiles || !bSameHashes )
			 {
				if ( !sError.IsEmpty() )
					sError.SetSprintf ( "%s; ", sError.cstr() );

				sError.SetSprintf ( "%s'%s:%d' wrong stored files %d(%d), hashes %d(%d)",
					sError.scstr(), dNodes[iNode]->m_tDesc.m_sAddr.cstr(), dNodes[iNode]->m_tDesc.m_iPort,
					tSigSrc.m_dBaseNames.GetLength(), tRes.m_pDst->m_dRemotePaths.GetLength(),
					tSigSrc.m_dHashes.GetLength() / HASH20_SIZE, tRes.m_pDst->m_dNodeChunks.GetBits() );
				continue;
			 }

			 const SyncDst_t * pDst = tRes.m_pDst.LeakPtr();
			 tStatesGuard.m_dFree.Add ( pDst );

			 bool bFilesMatched = true;
			 for ( int iFile=0; bFilesMatched && iFile<tSigSrc.m_dBaseNames.GetLength(); iFile++ )
				 bFilesMatched &= tFilesDst.BitGet ( iFile );

			 // no need to send index files to nodes there files matches exactly
			 if ( !bFilesMatched )
			 {
				 RemoteFileState_t & tRemoteState = dSendStates.Add();
				 tRemoteState.m_sRemoteIndexPath = tRes.m_sRemoteIndexPath;
				 tRemoteState.m_pAgentDesc = dDesc[iNode];
				 tRemoteState.m_pSyncSrc = &tSigSrc;
				 tRemoteState.m_pSyncDst = pDst;

				 // after file send need also to re-activate index with new files
				 dActivateIndexes.Add ( tRemoteState );

			 } else if ( !tRes.m_bIndexActive )
			 {
				 RemoteFileState_t & tRemoteState = dActivateIndexes.Add();
				 tRemoteState.m_sRemoteIndexPath = tRes.m_sRemoteIndexPath;
				 tRemoteState.m_pAgentDesc = dDesc[iNode];
				 tRemoteState.m_pSyncSrc = &tSigSrc;
				 tRemoteState.m_pSyncDst = pDst;
			 }
		}
	}

	if ( !dSendStates.GetLength() && !dActivateIndexes.GetLength() )
		return true;

	sphLogDebugRpl ( "sending index '%s'", sIndex.cstr() );

	if ( dSendStates.GetLength() && !SendFile ( tSigSrc, dSendStates, sCluster, sIndex, sError ) )
		return false;

	// allow index local write operations passed without replicator
	tIndexSaveGuard.EnableSave();

	{
		CSphFixedVector<PQRemoteData_t> dAgentData ( dActivateIndexes.GetLength() );
		VecRefPtrs_t<AgentConn_t *> dNodes;
		dNodes.Resize ( dActivateIndexes.GetLength() );
		ARRAY_FOREACH ( i, dActivateIndexes )
		{
			const RemoteFileState_t & tState = dActivateIndexes[i];

			PQRemoteData_t & tAgentData = dAgentData[i];
			tAgentData.m_sCluster = sCluster;
			tAgentData.m_sIndex = sIndex;
			tAgentData.m_sRemoteIndexPath = tState.m_sRemoteIndexPath;
			tAgentData.m_pChunks = &tSigSrc;
			tAgentData.m_eIndex = eType;

			dNodes[i] = CreateAgent ( *tState.m_pAgentDesc, tAgentData, tmLongOpTimeout );
			PQRemoteBase_c::GetRes ( *dNodes[i] ).m_pSharedDst = tState.m_pSyncDst;
		}

		sphLogDebugRpl ( "sent index '%s' loading to %d nodes with timeout %d.%03d sec", sIndex.cstr(), dNodes.GetLength(), (int)( tmLongOpTimeout/1000 ), (int)( tmLongOpTimeout%1000 ) );

		PQRemoteIndexAdd_c tReq;
		if ( !PerformRemoteTasks ( dNodes, tReq, tReq, sError ) )
			return false;

		sphLogDebugRpl ( "remote index '%s' added", sIndex.cstr() );
	}

	return true;
}

// cluster ALTER statement that adds index
static bool ClusterAlterAdd ( const CSphString & sCluster, const CSphString & sIndex, CSphString & sError, CSphString & sWarning )
	EXCLUDES ( g_tClustersLock )
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
		if ( !CheckClasterState ( *ppCluster, sError ) )
			return false;

		// copy nodes with lock as change of cluster view would change node list string
		{
			ScopedMutex_t tNodesLock ( (*ppCluster)->m_tViewNodesLock );
			sNodes = (*ppCluster)->m_sViewNodes;
		}
	}

	ServedIndexRefPtr_c pServed = GetServed ( sIndex );
	if ( !pServed )
	{
		sError.SetSprintf ( "unknown index '%s'", sIndex.cstr() );
		return false;
	}

	// alter add index should be valid
	ServedDescWPtr_c pIndexDesc ( pServed );
	if ( pIndexDesc->m_eType!=IndexType_e::PERCOLATE && pIndexDesc->m_eType!=IndexType_e::RT )
	{
		sError.SetSprintf ( "wrong type of index '%s'", sIndex.cstr() );
		return false;
	}

	// ok for just created cluster (wo nodes) to add existed index
	if ( !sNodes.IsEmpty() )
	{
		VecAgentDesc_t dDesc;
		GetNodes ( sNodes, dDesc );

		if ( dDesc.GetLength() && !NodesReplicateIndex ( sCluster, sIndex, dDesc, pIndexDesc, sError ) )
			return false;

		if ( !sError.IsEmpty() )
			sphWarning ( "%s", sError.cstr() );
	}

	RtAccum_t tAcc { false };
	ReplicationCommand_t * pAddCmd = tAcc.AddCommand ( ReplicationCommand_e::CLUSTER_ALTER_ADD, sCluster, sIndex );
	pAddCmd->m_bCheckIndex = false;

	return HandleCmdReplicate ( tAcc, sError, nullptr, nullptr, nullptr, pIndexDesc.Ptr() );
}

// cluster ALTER statement
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
		if ( pDesc->m_eType!=IndexType_e::PERCOLATE && pDesc->m_eType!=IndexType_e::RT )
		{
			sError.SetSprintf ( "wrong type of index '%s'", sIndex.cstr() );
			return false;
		}

		if ( bAdd && !pDesc->m_sCluster.IsEmpty() )
		{
			sError.SetSprintf ( "index '%s' is a part of cluster '%s'", sIndex.cstr(), pDesc->m_sCluster.cstr() );
			return false;
		}
		if ( !bAdd && pDesc->m_sCluster.IsEmpty() )
		{
			sError.SetSprintf ( "index '%s' is not in cluster '%s'", sIndex.cstr(), sCluster.cstr() );
			return false;
		}
	}

	bool bOk = false;
	if ( bAdd )
		bOk = ClusterAlterAdd ( sCluster, sIndex, sError, sWarning );
	else
		bOk = ClusterAlterDrop ( sCluster, sIndex, sError );

	bOk &= SaveConfigInt(sError);

	return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// SST
/////////////////////////////////////////////////////////////////////////////

PQRemoteSynced_c::PQRemoteSynced_c ()
	: PQRemoteBase_c ( CLUSTER_SYNCED )
{
}

void PQRemoteSynced_c::BuildCommand ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const
{
	const PQRemoteData_t & tCmd = GetReq ( tAgent );
	tOut.SendString ( tCmd.m_sCluster.cstr() );
	tOut.SendString ( tCmd.m_sGTID.cstr() );
	tOut.SendInt( tCmd.m_dIndexes.GetLength() );
	for ( const CSphString & sIndex : tCmd.m_dIndexes )
		tOut.SendString ( sIndex.cstr() );
}

void PQRemoteSynced_c::ParseCommand ( InputBuffer_c & tBuf, PQRemoteData_t & tCmd )
{
	tCmd.m_sCluster = tBuf.GetString();
	tCmd.m_sGTID = tBuf.GetString();
	tCmd.m_dIndexes.Reset ( tBuf.GetInt() );
	ARRAY_FOREACH ( i, tCmd.m_dIndexes )
		tCmd.m_dIndexes[i] = tBuf.GetString();
}

void PQRemoteSynced_c::BuildReply ( const PQRemoteReply_t & tRes, ISphOutputBuffer & tOut )
{
	auto tReply = APIAnswer ( tOut );
	tOut.SendByte ( 1 );
}

bool PQRemoteSynced_c::ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const
{
	// just payload as can not recv reply with 0 size
	tReq.GetByte();
	return !tReq.GetError();
}

// API command to remote node to issue cluster synced callback
static bool SendClusterSynced ( const CSphString & sCluster, const CSphVector<CSphString> & dIndexes, const VecAgentDesc_t & dDesc, const char * sGTID, CSphString & sError )
{
	PQRemoteData_t tAgentData;
	tAgentData.m_sCluster = sCluster;
	tAgentData.m_sGTID = sGTID;
	tAgentData.m_dIndexes.Reset ( dIndexes.GetLength() );
	ARRAY_FOREACH ( i, dIndexes )
		tAgentData.m_dIndexes[i] = dIndexes[i];

	VecRefPtrs_t<AgentConn_t *> dNodes;
	GetNodes ( dDesc, dNodes, tAgentData, GetQueryTimeout()  );
	PQRemoteSynced_c tReq;
	if ( !PerformRemoteTasks ( dNodes, tReq, tReq, sError ) )
		return false;

	return true;
}

// send local indexes to remote nodes via API
bool SendClusterIndexes ( const ReplicationCluster_t * pCluster, const CSphString & sNode, bool bBypass,
	const wsrep_gtid_t & tStateID, CSphString & sError ) EXCLUDES ( g_tClustersLock )
{
	VecAgentDesc_t dDesc;
	GetNodes ( sNode, dDesc );
	if ( !dDesc.GetLength() )
	{
		sError.SetSprintf ( "%s invalid node", sNode.cstr() );
		return false;
	}

	char sGTID[WSREP_GTID_STR_LEN];
	wsrep_gtid_print ( &tStateID, sGTID, sizeof(sGTID) );

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
		ServedIndexRefPtr_c pServed = GetServed ( sIndex );
		if ( !pServed )
		{
			sphWarning ( "unknown index '%s'", sIndex.cstr() );
			continue;
		}

		ServedDescRPtr_c pIndexDesc ( pServed );
		if ( pIndexDesc->m_eType!=IndexType_e::PERCOLATE && pIndexDesc->m_eType!=IndexType_e::RT )
		{
			sphWarning ( "wrong type of index '%s'", sIndex.cstr() );
			continue;
		}

		if ( !NodesReplicateIndex ( pCluster->m_sName, sIndex, dDesc, pIndexDesc, sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			bOk = false;
			break;
		}

		if ( !sError.IsEmpty() )
			sphWarning ( "%s", sError.cstr() );
	}

	bOk &= SendClusterSynced ( pCluster->m_sName, pCluster->m_dIndexes, dDesc, bOk ? sGTID : "" , sError );
	
	return bOk;
}

// callback at remote node for CLUSTER_SYNCED to pick up received indexes then call Galera sst_received
bool RemoteClusterSynced ( const PQRemoteData_t & tCmd, CSphString & sError ) EXCLUDES ( g_tClustersLock )
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

	ReplicationCluster_t** ppCluster = nullptr;
	{
		ScRL_t tLock ( g_tClustersLock );
		ppCluster = g_hClusters ( tCmd.m_sCluster );
	}

	if ( !ppCluster )
	{
		sError.SetSprintf ( "unknown cluster '%s'", tCmd.m_sCluster.cstr() );
		return false;
	}

	int iRes = 0;
	if ( !bValid )
	{
		tGtid.seqno = WSREP_SEQNO_UNDEFINED;
		iRes = -ECANCELED;
	}

	wsrep_t * pProvider = (*ppCluster)->m_pProvider;
	pProvider->sst_received ( pProvider, &tGtid, nullptr, 0, iRes );

	return bValid;
}

// validate that SphinxQL statement could be run for this cluster:index 
bool CheckIndexCluster ( const CSphString & sIndexName, const ServedDesc_t & tDesc, const CSphString & sStmtCluster, bool bHTTP, CSphString & sError )
{
	if ( tDesc.m_sCluster==sStmtCluster )
		return true;

	if ( tDesc.m_sCluster.IsEmpty() )
	{
		sError.SetSprintf ( "index '%s' is not in any cluster, use just '%s'", sIndexName.cstr(), sIndexName.cstr() );
	} else
	{
		if ( !bHTTP )
			sError.SetSprintf ( "index '%s' is a part of cluster '%s', use '%s:%s'", sIndexName.cstr(), tDesc.m_sCluster.cstr(), tDesc.m_sCluster.cstr(), sIndexName.cstr() );
		else
			sError.SetSprintf ( "index '%s' is a part of cluster '%s', use \"cluster\":\"%s\" and \"index\":\"%s\" properties", sIndexName.cstr(), tDesc.m_sCluster.cstr(), tDesc.m_sCluster.cstr(), sIndexName.cstr() );
	}

	return false;
}

bool ClusterOperationProhibit ( const ServedDesc_t * pDesc, CSphString & sError, const char * sOp )
{
	if ( !pDesc || pDesc->m_sCluster.IsEmpty() )
		return false;

	sError.SetSprintf ( "is a part of cluster '%s', can not issue %s", pDesc->m_sCluster.cstr(), sOp );
	return true;
}

// command to all remote nodes at cluster to get actual nodes list
bool ClusterGetNodes ( const CSphString & sClusterNodes, const CSphString & sCluster, const CSphString & sGTID, CSphString & sError, CSphString & sNodes )
{
	VecAgentDesc_t dDesc;
	GetNodes ( sClusterNodes, dDesc );
	if ( !dDesc.GetLength() )
	{
		sError.SetSprintf ( "%s invalid node", sClusterNodes.cstr() );
		return false;
	}

	CSphFixedVector<PQRemoteData_t> dReplies ( dDesc.GetLength() );
	VecRefPtrs_t<AgentConn_t *> dAgents;
	dAgents.Resize ( dDesc.GetLength() );
	ARRAY_FOREACH ( i, dAgents )
	{
		dReplies[i].m_sCluster = sCluster;
		dReplies[i].m_sGTID = sGTID;
		dAgents[i] = CreateAgent ( *dDesc[i], dReplies[i], g_iAnyNodesTimeout );
	}

	// submit initial jobs
	CSphRefcountedPtr<RemoteAgentsObserver_i> tReporter ( GetObserver() );
	PQRemoteClusterGetNodes_c tReq;
	ScheduleDistrJobs ( dAgents, &tReq, &tReq, tReporter, g_iNodeRetry, g_iNodeRetryWait );

	bool bDone = false;
	while ( !bDone )
	{
		// don't forget to check incoming replies after send was over
		bDone = tReporter->IsDone();
		// wait one or more remote queries to complete
		if ( !bDone )
			tReporter->WaitChanges();

		for ( const AgentConn_t * pAgent : dAgents )
		{
			if ( !pAgent->m_bSuccess )
				continue;

			// FIXME!!! no need to wait all replies in case any node get nodes list
			// just break on 1st successful reply
			// however need a way for distributed loop to finish as it can not break early
			const PQRemoteReply_t & tRes = PQRemoteBase_c::GetRes ( *pAgent );
			sNodes = tRes.m_sNodes;
		}
	}


	bool bGotNodes = ( !sNodes.IsEmpty() );
	if ( !bGotNodes )
	{
		StringBuilder_c sBuf ( ";" );
		for ( const AgentConn_t * pAgent : dAgents )
		{
			if ( !pAgent->m_sFailure.IsEmpty() )
			{
				sphWarning ( "'%s:%d': %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pAgent->m_sFailure.cstr() );
				sBuf.Appendf ( "'%s:%d': %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pAgent->m_sFailure.cstr() );
			}
		}
		sError = sBuf.cstr();
	}

	return bGotNodes;
}

// callback at remote node for CLUSTER_GET_NODES to return actual nodes list at cluster
bool RemoteClusterGetNodes ( const CSphString & sCluster, const CSphString & sGTID,
	CSphString & sError, CSphString & sNodes )  EXCLUDES ( g_tClustersLock )
{
	ScRL_t tLock ( g_tClustersLock );
	ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
	if ( !ppCluster )
	{
		sError.SetSprintf ( "unknown cluster '%s'", sCluster.cstr() );
		return false;
	}

	ReplicationCluster_t * pCluster = (*ppCluster);
	if ( !sGTID.IsEmpty() && strncmp ( sGTID.cstr(), pCluster->m_dUUID.Begin(), pCluster->m_dUUID.GetLength() )!=0 )
	{
		sError.SetSprintf ( "cluster '%s' GTID %.*s does not match with incoming %s", sCluster.cstr(), pCluster->m_dUUID.GetLength(), pCluster->m_dUUID.Begin(), sGTID.cstr() );
		return false;
	}

	// send back view nodes of cluster - as list of actual nodes
	{
		ScopedMutex_t tNodesLock ( (*ppCluster)->m_tViewNodesLock );
		sNodes = (*ppCluster)->m_sViewNodes;
	}

	return true;
}

// cluster ALTER statement that updates nodes option from view nodes at all nodes at cluster
bool ClusterAlterUpdate ( const CSphString & sCluster, const CSphString & sUpdate, bool bRemoteError, CSphString & sError )
	EXCLUDES ( g_tClustersLock )
{
	if ( sUpdate!="nodes" )
	{
		sError.SetSprintf ( "unhandled statement, only UPDATE nodes are supported, got '%s'", sUpdate.cstr() );
		return false;
	}

	{
		ScRL_t tLock ( g_tClustersLock );
		ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
		if ( !ppCluster )
		{
			sError.SetSprintf ( "unknown cluster '%s'", sCluster.cstr() );
			return false;
		}
		if ( !CheckClasterState ( *ppCluster, sError ) )
			return false;
	}
	// need to update all VIEW nodes - not cluster set nodes
	CSphString sNodes;

	// local nodes update
	bool bOk = RemoteClusterUpdateNodes ( sCluster, &sNodes, sError );

	// remote nodes update after locals updated
	PQRemoteData_t tReqData;
	tReqData.m_sCluster = sCluster;

	VecRefPtrs_t<AgentConn_t *> dNodes;
	GetNodes ( sNodes, dNodes, tReqData );
	PQRemoteClusterUpdateNodes_c tReq;
	bool bRemoteOk = PerformRemoteTasks ( dNodes, tReq, tReq, sError );
	bOk &= SaveConfigInt(sError);

	if ( !bRemoteOk )
	{
		if ( bRemoteError )
			bOk = false;
		else
		{
			sphWarning ( "cluster %s nodes update error %s", sCluster.cstr(), sError.cstr() );
			sError = "";
		}
	}

	return bOk;
}

// callback at remote node for CLUSTER_UPDATE_NODES to update nodes list at cluster from actual nodes list
bool RemoteClusterUpdateNodes ( const CSphString & sCluster, CSphString * pNodes, CSphString & sError )
	EXCLUDES ( g_tClustersLock )
{
	ScRL_t tLock ( g_tClustersLock );
	ReplicationCluster_t ** ppCluster = g_hClusters ( sCluster );
	if ( !ppCluster )
	{
		sError.SetSprintf ( "unknown cluster '%s'", sCluster.cstr() );
		return false;
	}

	{
		ReplicationCluster_t * pCluster = *ppCluster;
		ScopedMutex_t tNodesLock ( (*ppCluster)->m_tViewNodesLock );
		ClusterFilterNodes ( pCluster->m_sViewNodes, Proto_e::SPHINX, pCluster->m_sClusterNodes );
		if ( pNodes )
			*pNodes = pCluster->m_sClusterNodes;
	}
	return true;
}

bool IsClusterCommand ( const RtAccum_t & tAcc )
{
	return ( tAcc.m_dCmd.GetLength() && !tAcc.m_dCmd[0]->m_sCluster.IsEmpty() );
}

bool IsUpdateCommand ( const RtAccum_t & tAcc )
{
	return ( tAcc.m_dCmd.GetLength() &&
		( tAcc.m_dCmd[0]->m_eCommand==ReplicationCommand_e::UPDATE_API || tAcc.m_dCmd[0]->m_eCommand==ReplicationCommand_e::UPDATE_QL || tAcc.m_dCmd[0]->m_eCommand==ReplicationCommand_e::UPDATE_JSON ) );
}

void SendArray ( const VecTraits_T<CSphString> & dBuf, ISphOutputBuffer & tOut )
{
	tOut.SendInt ( dBuf.GetLength() );
	for ( const CSphString & sVal : dBuf )
		tOut.SendString ( sVal.cstr() );
}

void GetArray ( CSphFixedVector<CSphString> & dBuf, InputBuffer_c & tIn )
{
	int iCount = tIn.GetInt();
	if ( !iCount )
		return;

	dBuf.Reset ( iCount );
	for ( CSphString & sVal : dBuf )
		sVal = tIn.GetString();
}

void SaveArray ( const VecTraits_T<CSphString> & dBuf, MemoryWriter_c & tOut )
{
	tOut.PutDword ( dBuf.GetLength() );
	for ( const CSphString & sVal : dBuf )
		tOut.PutString ( sVal );
}

void GetArray ( CSphVector<CSphString> & dBuf, MemoryReader_c & tIn )
{
	int iCount = tIn.GetDword();
	if ( !iCount )
		return;

	dBuf.Resize ( iCount );
	for ( CSphString & sVal : dBuf )
		sVal = tIn.GetString();
}


void SaveUpdate ( const CSphAttrUpdate & tUpd, CSphVector<BYTE> & dOut )
{
	MemoryWriter_c tWriter ( dOut );
	
	tWriter.PutByte ( tUpd.m_bIgnoreNonexistent );
	tWriter.PutByte ( tUpd.m_bStrict );

	tWriter.PutDword ( tUpd.m_dAttributes.GetLength() );
	for ( const TypedAttribute_t & tElem : tUpd.m_dAttributes )
	{
		tWriter.PutString ( tElem.m_sName );
		tWriter.PutDword ( tElem.m_eType );
	}

	SaveArray ( tUpd.m_dDocids, tWriter );
	SaveArray ( tUpd.m_dRowOffset, tWriter );
	SaveArray ( tUpd.m_dPool, tWriter );
}

int LoadUpdate ( const BYTE * pBuf, int iLen, CSphAttrUpdate & tUpd )
{
	MemoryReader_c tIn ( pBuf, iLen );
	
	tUpd.m_bIgnoreNonexistent = !!tIn.GetByte();
	tUpd.m_bStrict = !!tIn.GetByte();

	tUpd.m_dAttributes.Resize ( tIn.GetDword() );
	for ( TypedAttribute_t & tElem : tUpd.m_dAttributes )
	{
		tElem.m_sName = tIn.GetString();
		tElem.m_eType = (ESphAttr)tIn.GetDword();
	}

	GetArray ( tUpd.m_dDocids, tIn );
	GetArray ( tUpd.m_dRowOffset, tIn );
	GetArray ( tUpd.m_dPool, tIn );

	return tIn.GetPos();
}

enum
{
	FILTER_FLAG_EXCLUDE			= 1UL << 0,
	FILTER_FLAG_HAS_EQUAL_MIN	= 1UL << 1,
	FILTER_FLAG_HAS_EQUAL_MAX	= 1UL << 2,
	FILTER_FLAG_OPEN_LEFT		= 1UL << 3,
	FILTER_FLAG_OPEN_RIGHT		= 1UL << 4,
	FILTER_FLAG_IS_NULL			= 1UL << 5
};


static void SaveFilter ( const CSphFilterSettings & tItem, MemoryWriter_c & tWriter )
{
	tWriter.PutString ( tItem.m_sAttrName );

	DWORD uFlags = 0;
	uFlags |= FILTER_FLAG_EXCLUDE * tItem.m_bExclude;
	uFlags |= FILTER_FLAG_HAS_EQUAL_MIN * tItem.m_bHasEqualMin;
	uFlags |= FILTER_FLAG_HAS_EQUAL_MAX * tItem.m_bHasEqualMax;
	uFlags |= FILTER_FLAG_OPEN_LEFT * tItem.m_bOpenLeft;
	uFlags |= FILTER_FLAG_OPEN_RIGHT * tItem.m_bOpenRight;
	uFlags |= FILTER_FLAG_IS_NULL * tItem.m_bIsNull;
	tWriter.PutDword ( uFlags );

	tWriter.PutByte ( tItem.m_eType );
	tWriter.PutByte ( tItem.m_eMvaFunc );
	tWriter.PutUint64 ( tItem.m_iMinValue );
	tWriter.PutUint64 ( tItem.m_iMaxValue );
	SaveArray ( tItem.m_dValues, tWriter );
	SaveArray ( tItem.m_dStrings, tWriter );

	int iValues = tItem.GetNumValues();
	if ( tItem.GetValueArray()==tItem.m_dValues.Begin() )
		iValues = 0;
	tWriter.PutDword ( iValues );
	if ( iValues )
		tWriter.PutBytes ( tItem.GetValueArray(), sizeof(*tItem.GetValueArray()) * iValues );
}

static void LoadFilter ( CSphFilterSettings & tItem, MemoryReader_c & tReader )
{
	tItem.m_sAttrName = tReader.GetString();

	DWORD uFlags = tReader.GetDword();
	tItem.m_bExclude = !!( uFlags & FILTER_FLAG_EXCLUDE );
	tItem.m_bHasEqualMin = !!( uFlags & FILTER_FLAG_HAS_EQUAL_MIN );
	tItem.m_bHasEqualMax = !!( uFlags & FILTER_FLAG_HAS_EQUAL_MAX );
	tItem.m_bOpenLeft = !!( uFlags & FILTER_FLAG_OPEN_LEFT );
	tItem.m_bOpenRight = !!( uFlags & FILTER_FLAG_OPEN_RIGHT );
	tItem.m_bIsNull = !!( uFlags & FILTER_FLAG_IS_NULL );

	tItem.m_eType = (ESphFilter )tReader.GetByte();
	tItem.m_eMvaFunc = (ESphMvaFunc)tReader.GetByte();
	tItem.m_iMinValue = tReader.GetUint64();
	tItem.m_iMaxValue = tReader.GetUint64();
	GetArray ( tItem.m_dValues, tReader );
	GetArray ( tItem.m_dStrings, tReader );

	int iValues = tReader.GetDword();
	if ( iValues )
	{
		CSphFixedVector<SphAttr_t> dValues ( iValues );
		tReader.GetBytes ( dValues.Begin(), (int) dValues.GetLengthBytes() );

		tItem.SetExternalValues ( dValues.LeakData(), iValues );
	}
}

void SaveUpdate ( const CSphQuery & tQuery, CSphVector<BYTE> & dOut )
{
	MemoryWriter_c tWriter ( dOut );

	tWriter.PutString ( tQuery.m_sQuery );
	tWriter.PutDword ( tQuery.m_dFilters.GetLength() );
	for ( const CSphFilterSettings & tItem : tQuery.m_dFilters )
		SaveFilter ( tItem, tWriter );
	
	SaveArray ( tQuery.m_dFilterTree, tWriter );
}

int LoadUpdate ( const BYTE * pBuf, int iLen, CSphQuery & tQuery )
{
	MemoryReader_c tReader ( pBuf, iLen );

	tQuery.m_sQuery =  tReader.GetString();
	tQuery.m_dFilters.Resize ( tReader.GetDword () );
	for ( CSphFilterSettings & tItem : tQuery.m_dFilters )
		LoadFilter ( tItem, tReader );
	
	GetArray ( tQuery.m_dFilterTree, tReader );

	return tReader.GetPos();
}

int64_t GetQueryTimeout ( int64_t iTimeout )
{
	// need default of 2 minutes in msec for replication requests as they are mostly long running
	int64_t iTm = Max ( g_iAgentQueryTimeoutMs, 120 * 1000 );
	return Max ( iTm, Min ( iTimeout, INT_MAX ) );
}

static const char * g_dReplicatorPatterns[] = {
	"Could not open state file for reading:",
	"No persistent state found. Bootstraping with default state",
	"Fail to access the file ("
};

bool CheckNoWarning ( const char * sMsg )
{
	if ( !sMsg || !sMsg[0] )
		return false;

	for ( const char * sPattern : g_dReplicatorPatterns )
	{
		if ( strncmp ( sMsg, sPattern, strlen(sPattern) )==0 )
			return true;
	}
	return false;
}
