//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxstd.h"
#include "sphinxutils.h"
#include "memio.h"
#include "sphinxpq.h"
#include "searchdreplication.h"
#include "replication/configuration.h"
#include "accumulator.h"
#include "fileutils.h"
#include "coroutine.h"
#include "digest_sha1.h"
#include "tracer.h"

#include "replication/wsrep_cxx.h"
#include "replication/common.h"
#include "replication/portrange.h"
#include "replication/nodes.h"
#include "replication/commit_monitor.h"
#include "replication/receiver_ctx.h"
#include "replication/serialize.h"
#include "replication/cluster_delete.h"
#include "replication/cluster_get_nodes.h"
#include "replication/cluster_update_nodes.h"
#include "replication/cluster_synced.h"
#include "replication/replicate_index.h"
#include "replication/grastate.h"

#if !_WIN32
// MAC-specific header
#include <netinet/in.h>
#endif

// global application context for wsrep callbacks

// debug options passed into Galera for our logreplication command line key
static const char * const g_sDebugOptions = "debug=on;cert.log_conflicts=yes";
// prefix added for Galera nodes
static const char * const g_sGcommPrefix = "gcomm://";

// verbose logging of replcating transactions, ruled by this env variable
static const bool LOG_LEVEL_RPL_TNX = val_from_env ( "MANTICORE_LOG_RPL_TNX", false );
#define LOG_COMPONENT_RPL_TNX ""
#define RPL_TNX LOGMSG ( RPL_DEBUG, RPL_TNX, RPL_TNX )

inline static bool IsSyncedOrDonor ( ClusterState_e eState ) noexcept
{
	return ( eState == ClusterState_e::DONOR || eState == ClusterState_e::SYNCED );
}

enum class BOOTSTRAP_E : bool {NO,YES};

// cluster related data
struct ReplicationCluster_t final : public ClusterDesc_t, Wsrep::Cluster_i
{
public:
	// replicator
	Wsrep::Provider_i* m_pProvider = nullptr;

	// serializer for replicator - guards for only one replication Op a time
	Threads::Coro::Mutex_c m_tReplicationMutex;

	// receiver thread
	Threads::Coro::Waitable_T<bool> m_bWorkerActive { false };

	// Galera port got from global ports list on cluster created
	ScopedPort_c	m_tPort;

	bool m_bUserRequest { false }; // indicates, if cluster is joining by user request (i.e. stmt 'join ...')

	// state variables cached from Galera
	Wsrep::UUID_t			m_dUUID {};
	int64_t					m_iConfID = 0;
	Wsrep::ViewStatus_e		m_eStatus = Wsrep::ViewStatus_e::DISCONNECTED;
	int						m_iSize = 0;
	int						m_iIdx = 0;

	// error that got reported to main thread
	CSphMutex				m_tErrorLock;
	StringBuilder_c			m_sError GUARDED_BY ( m_tErrorLock ) { ";" };

private:
	~ReplicationCluster_t() final; // private since ref-counted

	void AbortSST();
public:
	explicit ReplicationCluster_t ( ClusterDesc_t&& tDesc )
	{
		m_sName = std::move ( tDesc.m_sName );
		m_sPath = std::move ( tDesc.m_sPath );
		m_dClusterNodes = std::move ( tDesc.m_dClusterNodes );
		m_tOptions = std::move ( tDesc.m_tOptions );
	}

	// state of node
	void SetState ( ClusterState_e eNodeState );
	ClusterState_e GetState() const noexcept;
	const char* szState() const noexcept;
	ClusterState_e WaitReady();

	bool IsHealthy() const;
	bool Init();
	bool Connect ( BOOTSTRAP_E eBootStrap );
	void StartListen();
	void DisconnectAndDeleteProvider();

	void UpdateGroupView ( const Wsrep::ViewInfo_t* pView ) EXCLUDES ( m_tViewNodesLock );
	StrVec_t FilterViewNodesByProto ( Proto_e eProto = Proto_e::SPHINX, bool bResolve = true ) const EXCLUDES ( m_tViewNodesLock );
	StrVec_t GetViewNodes() const EXCLUDES ( m_tViewNodesLock );
	void SetViewNodes ( StrVec_t&& dNodes );
	StrVec_t GetIndexes() const noexcept EXCLUDES ( m_tIndexLock );
	const CSphString & GetNodeName () const { return m_sNodeName; }

	template<typename ACTION>
	void FilterViewNodes ( ACTION&& Verb ) const
	{
		Threads::SccRL_t tNodesRLock ( m_tViewNodesLock );
		m_dViewNodes.Apply ( std::forward<ACTION> ( Verb ) );
	}

	void ShowStatus ( VectorLike& dOut );

	void HeartBeat ()
	{
		m_tHeardBeat.NotifyAll();
	}

	template<typename PRED>
	void WaitHeartBeat ( PRED&& fnPred )
	{
		m_tHeardBeat.WaitVoid ( std::forward<PRED> ( fnPred ) );
	}

	template<typename PRED>
	bool WaitHeartBeatForMs ( PRED&& fnPred, int64_t iPeriodMs )
	{
		return m_tHeardBeat.WaitVoidForMs ( std::forward<PRED> ( fnPred ), iPeriodMs );
	}

	template<typename PRED>
	ClusterState_e			WaitAny (PRED&& fnPred)
	{
		return m_tNodeState.Wait ( std::forward<PRED> ( fnPred ) );
	}

	template<typename PRED>
	ClusterState_e			WaitAnyForMs (PRED&& fnPred, int64_t iPeriodMs )
	{
		return m_tNodeState.WaitForMs ( std::forward<PRED> ( fnPred ), iPeriodMs );
	}

	/// Cluster_i implementation
	///////////////////////////

	void ChangeView ( const Wsrep::ViewInfo_t* pView, const char* pState, uint64_t iStateLen, void** ppSstReq, uint64_t* pSstReqLen ) final;

	// callback for Galera synced_cb that cluster fully synced and could accept transactions
	void SetSynced() final;

	bool DonateSST ( CSphString sJoiner, const Wsrep::GlobalTid_t* pStateID, bool bBypass ) final;

	void OnRecvStarted() final;

	void OnRecvFinished ( bool bSuccess ) final;

	bool IsPrimary() const noexcept { return ( m_eStatus == Wsrep::ViewStatus_e::PRIMARY ); }

	template<typename VISITOR>
	auto WithRlockedIndexes ( VISITOR fnVisitor ) const noexcept EXCLUDES ( m_tIndexLock )
	{
		Threads::SccRL_t tIndexRLock ( m_tIndexLock );
		return fnVisitor ( m_hIndexes );
	}

	template<typename VISITOR>
	auto WithRlockedAllIndexes ( VISITOR fnVisitor ) const noexcept EXCLUDES ( m_tIndexLock )
	{
		Threads::SccRL_t tIndexRLock ( m_tIndexLock );
		return fnVisitor ( m_hIndexes, m_hIndexesLoaded );
	}

	template<typename VISITOR>
	auto WithWlockedIndexes ( VISITOR fnVisitor ) EXCLUDES ( m_tIndexLock )
	{
		Threads::SccWL_t tIndexWLock ( m_tIndexLock );
		return fnVisitor ( m_hIndexes, m_hIndexesLoaded );
	}

	template<typename VISITOR>
	auto WithRlockedOptions ( VISITOR fnVisitor ) const noexcept EXCLUDES ( m_tOptsLock )
	{
		Threads::SccRL_t tOptionsRLock ( m_tOptsLock );
		return fnVisitor ( m_tOptions );
	}

	template<typename VISITOR>
	auto WithWlockedOptions ( VISITOR fnVisitor ) EXCLUDES ( m_tOptsLock )
	{
		Threads::SccWL_t tOptionsWLock ( m_tOptsLock );
		return fnVisitor ( m_tOptions );
	}

private:
	Threads::Coro::Waitable_T<ClusterState_e> m_tNodeState { ClusterState_e::CLOSED };
	Threads::Coro::Waitable_T<bool> m_tHeardBeat { false };

	// nodes at cluster
	// raw nodes addresses (API and replication) from whole cluster
	mutable Threads::Coro::RWLock_c m_tViewNodesLock;
	StrVec_t m_dViewNodes GUARDED_BY ( m_tViewNodesLock );

	mutable Threads::Coro::RWLock_c m_tOptsLock;
	mutable Threads::Coro::RWLock_c m_tIndexLock;
	sph::StringSet m_hIndexesLoaded;	// list of index name loaded into daemon but not yet in cluster used for donor to send indexes into joiner

#ifndef NDEBUG
	// it is impossible to attach 'GUARDED_BY to 'using ClusterDesc_t::m_tOptions', so use reference for it debug build
	ClusterOptions_t& m_tOptions GUARDED_BY ( m_tOptsLock ) { ClusterDesc_t::m_tOptions };
	sph::StringSet& m_hIndexes GUARDED_BY ( m_tIndexLock ) { ClusterDesc_t::m_hIndexes }; // to quickly validate query to cluster:index
#else
	using ClusterDesc_t::m_tOptions;
	using ClusterDesc_t::m_hIndexes;
#endif

	CSphString m_sNodeName;
};

using ReplicationClusterRefPtr_c = CSphRefcountedPtr<ReplicationCluster_t>;

// serializer for cluster management operations - only one cluster operation a time
static Threads::Coro::Mutex_c g_tClusterOpsLock;

// cluster list
static Threads::Coro::RWLock_c g_tClustersLock;
static SmallStringHash_T<ReplicationClusterRefPtr_c> g_hClusters GUARDED_BY ( g_tClustersLock );

ReplicationClusterRefPtr_c ClusterByName ( const CSphString& sCluster, const char* szErrTmpl = "unknown cluster '%s'" ) EXCLUDES ( g_tClustersLock )
{
	ReplicationClusterRefPtr_c pCluster;
	Threads::SccRL_t tLock ( g_tClustersLock );
	if ( !g_hClusters.Exists ( sCluster ) )
	{
		if ( szErrTmpl )
			TlsMsg::Err ( szErrTmpl, sCluster.cstr() );
	} else
		pCluster = g_hClusters[sCluster];
	return pCluster;
}

static bool CheckClusterIndexes ( const VecTraits_T<CSphString> & dIndexes, ReplicationClusterRefPtr_c pCluster )
{
	auto fnCmdValidate =  [&]( const auto & hIndexes )
	{
		for ( const CSphString & sIndex : dIndexes )
		{
			if ( !hIndexes[sIndex] )
				return TlsMsg::Err ( "table '%s' doesn't belong to cluster '%s'", sIndex.cstr(), pCluster->m_sName.cstr() );
		}

		return true;
	};

	return pCluster->WithRlockedIndexes ( fnCmdValidate );
}

static bool CheckClusterIndex ( const CSphString & sIndex, ReplicationClusterRefPtr_c pCluster )
{
	return pCluster->WithRlockedIndexes ( [&]( const auto & hIndexes )
	{
		if ( !hIndexes[sIndex] )
			return TlsMsg::Err ( "table '%s' doesn't belong to cluster '%s'", sIndex.cstr(), pCluster->m_sName.cstr() );

		return true;
	});
}

/////////////////////////////////////////////////////////////////////////////
/// forward declarations
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/// remote commands for cluster and index managements
/////////////////////////////////////////////////////////////////////////////

// send local indexes to remote nodes via API
static bool SendClusterIndexes ( const ReplicationCluster_t * pCluster, const CSphString & sNode, bool bBypass, const Wsrep::GlobalTid_t & tStateID );

static bool ValidateUpdate ( const ReplicationCommand_t & tCmd );

static bool DoClusterAlterUpdate ( const CSphString & sCluster, const CSphString & sUpdate, NODES_E eUpdate );
static bool IsSameVector ( StrVec_t & dSrc, StrVec_t & dDst );


static bool g_bReplicationEnabled = false;
static CSphString g_sReplicationStartError;
static bool g_bReplicationStarted = false;

bool ReplicationEnabled()
{
	return g_bReplicationEnabled;
}

static constexpr const char * szNodeState ( ClusterState_e eState )
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

const char* ReplicationCluster_t::szState() const noexcept
{
	return ::szNodeState ( GetState() );
}

void ReplicationCluster_t::SetState ( ClusterState_e eNodeState )
{
	m_tNodeState.SetValue ( eNodeState );
	m_tNodeState.NotifyAll();
}
ClusterState_e ReplicationCluster_t::GetState() const noexcept
{
	return m_tNodeState.GetValue();
}

ClusterState_e ReplicationCluster_t::WaitReady()
{
	return WaitAny ( [] ( ClusterState_e i ) { return i == ClusterState_e::SYNCED || i == ClusterState_e::DESTROYED; } );
}

bool ReplicationCluster_t::IsHealthy() const
{
	if ( !IsPrimary() )
		return TlsMsg::Err ( "cluster '%s' is not ready, not primary state (%s)", m_sName.cstr(), szState() );

	auto eState = GetState();
	return eState == ClusterState_e::SYNCED
		|| eState == ClusterState_e::DONOR
		|| TlsMsg::Err ( "cluster '%s' is not ready, current state is %s", m_sName.cstr(), szState() );
}

static int GetClusterMemLimitMB ( int iMemLimit, int iIndexes )
{
	const int CACHE_PER_INDEX = 16;
	const int MIN_CACHE_SIZE = 128;

	// change default cache size to 16Mb per added index or size of largest rt_mem_limit of RT index but at least 128Mb
	int iSize = iMemLimit / 1024 / 1024;
	iIndexes = Max ( 1, iIndexes );
	return Max ( Max ( iIndexes * CACHE_PER_INDEX, iSize ), MIN_CACHE_SIZE );
}

bool ReplicationCluster_t::Init()
{
	assert ( ReplicationEnabled() );
	CSphString sListenAddr, sIncoming, sFullClusterPath;
	m_sNodeName.SetSprintf ( "node_%s_%s_%d", szIncomingIP(), m_sName.cstr(), GetOsThreadId() );
	sListenAddr.SetSprintf ( "%s:%d", szListenReplicationIP(), int ( m_tPort ) );
	sIncoming.SetSprintf ( "%s,%s:%d:replication", szIncomingProto(), szIncomingIP(), int ( m_tPort ) );
	sFullClusterPath = GetDatadirPath ( m_sPath );

	sphLogDebugRpl ( "node incoming '%s', listen '%s', name '%s'", sIncoming.cstr(), sListenAddr.cstr(), m_sNodeName.cstr() );
	StringBuilder_c sOptions ( ";" );
	WithRlockedOptions ( [&sOptions] ( const auto& tOptions ) { sOptions << tOptions.AsStr(); } );

	// all replication options below have not stored into cluster config
	// set incoming address
	if ( HasIncoming() )
	{
		sOptions.Sprintf ( "ist.recv_addr=%s", szIncomingIP() );
		sOptions << "ist.recv_bind=0.0.0.0";
	}

	// change default cache size to 16Mb per added index but not more than default value
	bool bHaveGcache = false;
	WithRlockedOptions ( [&bHaveGcache] ( const auto& tOptions ) { bHaveGcache = tOptions.m_hOptions.Exists ( "gcache.size" ); } );
	if ( !bHaveGcache )
	{
		int iMaxMemLimit = 0;
		int iIndexes = 0;
		WithRlockedIndexes ( [&iMaxMemLimit, &iIndexes] ( const auto& hIndexes ) {
			iIndexes = hIndexes.GetLength();
			for_each ( hIndexes, [&iMaxMemLimit] ( const auto& hIndex ) {
				cServedIndexRefPtr_c pServed = GetServed ( hIndex.first );
				if ( pServed && pServed->m_tSettings.m_iMemLimit > iMaxMemLimit )
					iMaxMemLimit = pServed->m_tSettings.m_iMemLimit;
			} );
		} );

		int iSizeMB = GetClusterMemLimitMB ( iMaxMemLimit, iIndexes );
		sOptions.Sprintf ( "gcache.size=%dM", iSizeMB );
	}

	// set debug log option
	if ( g_eLogLevel >= SPH_LOG_RPL_DEBUG )
		sOptions += g_sDebugOptions;

	m_pProvider = Wsrep::MakeProvider ( this, m_sNodeName, sListenAddr.cstr(), sIncoming.cstr(), sFullClusterPath.cstr(), sOptions.cstr() );
	return bool ( m_pProvider );
}

bool ReplicationCluster_t::Connect ( BOOTSTRAP_E eBootStrap )
{
	StringBuilder_c sNodes;
	sNodes << g_sGcommPrefix;
	sNodes.StartBlock ( "," );
	FilterViewNodesByProto ( Proto_e::REPLICATION ).Apply ( [&sNodes] ( const CSphString& sNode ) { sNodes << sNode; } );

	sphLogDebugRpl ( "nodes '%s'", sNodes.cstr() );
	if ( g_eLogLevel >= SPH_LOG_RPL_DEBUG ) {
		StringBuilder_c sIndexes ( "," );
		WithRlockedIndexes ( [&sIndexes] ( const auto& hIndexes ) { for_each ( hIndexes, [&] ( const auto& tIndex ) { sIndexes << tIndex.first; } ); } );
		sphLogDebugRpl ( "cluster '%s', indexes '%s', nodes '%s'", m_sName.cstr(), sIndexes.cstr(), sNodes.cstr() );
	}

	// Connect to cluster
	return m_pProvider->Connect ( m_sName.cstr(), sNodes.cstr(), ( eBootStrap == BOOTSTRAP_E::YES ) );
}

void ReplicationCluster_t::StartListen()
{
	CSphRefcountedPtr<Wsrep::Receiver_i> pReceiver { MakeReceiverCtx ( m_sName, m_pProvider, [this]() { HeartBeat(); } ) };
	m_pProvider->StartListen ( pReceiver.Ptr() );
	sphLogDebugRpl ( "replicator is created for cluster '%s'", m_sName.cstr() );
}

// update cluster state nodes from Galera callback on cluster view changes
void ReplicationCluster_t::UpdateGroupView ( const Wsrep::ViewInfo_t* pView )
{
	const auto* pBoxes = &pView->m_tMemInfo;
	StrVec_t dNodes;
	for ( int i = 0; i < pView->m_iNMembers; ++i )
		dNodes.Append ( ParseNodesFromString ( pBoxes[i].m_sIncoming ) );

	sphLogDebugRpl ( "cluster '%s' view nodes changed: %s > %s", m_sName.cstr(), StrVec2Str ( GetViewNodes() ).cstr(), StrVec2Str ( dNodes ).cstr() );
	SetViewNodes ( std::move ( dNodes ) );
}

CSphString WaitClusterReady ( const CSphString& sCluster, int64_t iTimeoutS )
{
	auto pCluster = ClusterByName ( sCluster );
	if ( !pCluster )
		return TlsMsg::MoveToString();

	ClusterState_e eState;
	if ( iTimeoutS<=0 )
		eState = pCluster->WaitAny ( [] ( ClusterState_e i ) { return i == ClusterState_e::SYNCED || i == ClusterState_e::DONOR; } );
	else
		eState = pCluster->WaitAnyForMs ( [] ( ClusterState_e i ) { return i == ClusterState_e::SYNCED || i == ClusterState_e::DONOR; }, iTimeoutS*1000 );
	return szNodeState ( eState );
}

std::pair<int, CSphString> WaitClusterCommit ( const CSphString& sCluster, int iTxn, int64_t iTimeoutS )
{
	auto pCluster = ClusterByName ( sCluster );
	if (!pCluster)
		return {-1,TlsMsg::MoveToString()};

	int64_t iVal = -1;
	auto fnPred = [iTxn, &iVal, pProvider=pCluster->m_pProvider]()
	{
		if ( !pProvider )
			return false;
		assert ( pProvider );
		pProvider->EnumFindStatsVar ( [&iVal] ( const Wsrep::StatsVars_t& tVar ) -> bool {
			if ( 0 != strcmp ( tVar.m_szName, "last_committed" ) )
				return false;
			iVal = tVar.m_tValue.iVal64;
			return true;
		} );
		return iVal >= iTxn;
	};
	bool bSuccess = true;
	if ( iTimeoutS <= 0 )
		pCluster->WaitHeartBeat ( std::move ( fnPred ) );
	else
		bSuccess = pCluster->WaitHeartBeatForMs ( std::move ( fnPred ), iTimeoutS * 1000 );
	if ( !bSuccess )
		return { iVal, "timeout" };
	return { iVal, "" };
}

// callback for Galera that cluster view got changed, ie node either added or removed from cluster.
// It is guaranteed that no other callbacks are called concurrently with it.
void ReplicationCluster_t::ChangeView ( const Wsrep::ViewInfo_t* pView, const char* pState, uint64_t uStateLen, void** ppSstReq, uint64_t* pSstReqLen )
{
	m_dUUID = pView->m_tStateId.m_tUuid;
	m_iConfID = pView->m_ViewSeqNo;
	m_iSize = pView->m_iNMembers;
	m_iIdx = pView->m_iIdx;
	m_eStatus = pView->m_eStatus;
	if ( IsPrimary() )
		UpdateGroupView ( pView );

	*ppSstReq = nullptr;
	*pSstReqLen = 0;

	if ( !pView->m_bGap )
		return;

	auto sAddr = FromSz ( szIncomingProto() );
	sphLogDebugRpl ( "join %s to %s", sAddr.first, m_sName.cstr() );
	*ppSstReq = memcpy ( malloc ( sAddr.second ), sAddr.first, sAddr.second ); // mem will be freed by Galera
	*pSstReqLen = sAddr.second;
	SetState ( ClusterState_e::JOINING );
}

// callback for Galera synced_cb that cluster fully synced and could accept transactions
void ReplicationCluster_t::SetSynced()
{
	SetState ( ClusterState_e::SYNCED );
	sphLogDebugRpl ( "synced cluster %s", m_sName.cstr() );
}

bool ReplicationCluster_t::DonateSST ( CSphString sJoiner, const Wsrep::GlobalTid_t* pStateID, bool bBypass )
{
	auto tGtid = *pStateID;
	sphLogDebugRpl ( "donate %s to %s, gtid %s, bypass %d", m_sName.cstr(), sJoiner.cstr(), Wsrep::Gtid2Str ( tGtid ).cstr(), (int)bBypass );

	SetState ( ClusterState_e::DONOR );
	const bool bOk = SendClusterIndexes ( this, sJoiner, bBypass, tGtid );
	SetState ( ClusterState_e::SYNCED );

	if ( !bOk ) {
		sphWarning ( "%s", TlsMsg::szError() );
		tGtid.m_iSeqNo = Wsrep::WRONG_SEQNO;
	}

	m_pProvider->SstSent ( tGtid, bOk );

	sphLogDebugRpl ( "donate cluster %s to %s, gtid %s, bypass %d, done %d", m_sName.cstr(), sJoiner.cstr(), Wsrep::Gtid2Str ( *pStateID ).cstr(), (int)bBypass, (int)bOk );
	return bOk;
}

void ReplicationCluster_t::OnRecvStarted()
{
	m_bWorkerActive.SetValue ( true );
	SetState ( ClusterState_e::JOINING );
}

void ReplicationCluster_t::OnRecvFinished ( bool bSuccess )
{
	SetState ( bSuccess ? ClusterState_e::CLOSED : ClusterState_e::DESTROYED );
	m_bWorkerActive.SetValueAndNotifyAll ( false );
}

void ReplicationCluster_t::AbortSST()
{
	if ( GetState() != ClusterState_e::JOINING )
		return;

	assert ( m_pProvider );
	sphLogDebugRpl ( "aborting SST" );
	Wsrep::GlobalTid_t tGtid {};
	m_pProvider->SstReceived ( tGtid, -ECANCELED );
}

// shutdown and delete cluster, also join cluster recv thread
void ReplicationCluster_t::DisconnectAndDeleteProvider()
{
	if ( !m_pProvider )
		return;

	AbortSST ();
	sphLogDebugRpl ( "disconnecting from cluster %s", m_sName.cstr() );
	m_pProvider->Disconnect();
	sphLogDebugRpl ( "disconnected from cluster %s", m_sName.cstr() );
}

ReplicationCluster_t::~ReplicationCluster_t()
{
	sphLogDebugRpl ( "cluster '%s' wait to finish", m_sName.scstr() );
	// Listening thread are now running and receiving writesets. Wait for them
	// to join. Thread will join after signal handler closes wsrep connection
	m_bWorkerActive.Wait ( [] ( bool bWorking ) { return !bWorking; } );

	HeartBeat();

	sphLogDebugRpl ( "deleting provider of cluster %s", m_sName.cstr() );
	SafeDelete ( m_pProvider );
	sphLogDebugRpl ( "cluster '%s' finished, cluster deleted", m_sName.scstr() );
}

// add 'RPL' flag - i.e., that we're working in replication
struct RPLRep_t: public MiniTaskInfo_t
{
	DECLARE_RENDER ( RPLRep_t );
};

DEFINE_RENDER ( RPLRep_t )
{
	auto& tInfo = *(RPLRep_t*)const_cast<void*> ( pSrc );
	dDst.m_sDescription.Sprintf ( "(RPL %.2T)", tInfo.m_tmStart );
	dDst.m_sChain << "RPL ";
}

// repl version
// replicate serialized data into cluster and call commit monitor along
static bool Replicate ( const VecTraits_T<uint64_t>& dKeys, const VecTraits_T<BYTE>& tQueries, Wsrep::Writeset_i& tWriteSet, CommitMonitor_c&& tMonitor, bool bUpdate, bool bSharedKeys )
{
	TRACE_CONN ( "conn", "Replicate" );

	// just displays 'RPL' flag.
	auto RPL = PublishTaskInfo ( new RPLRep_t );

	bool bOk = tWriteSet.AppendKeys ( dKeys, bSharedKeys ) && tWriteSet.AppendData ( tQueries );

	if ( !bOk )
		return false;

	auto dFinalReport = AtScopeExit ( [&tWriteSet] {
		sphLogDebugRpl ( "%s seq " INT64_FMT, ( tWriteSet.LastOk() ? "committed" : "rolled-back" ), tWriteSet.LastSeqno() );
	} );

	TRACE_CONN ( "conn", "pProvider->replicate" );
	if ( !tWriteSet.Replicate() )
	{
		RPL_TNX << "replicating " << (int)bOk << ", seq " << tWriteSet.LastSeqno() << ", commands " << dKeys.GetLength();
		return false;
	}

	if ( !tWriteSet.PreCommit() )
		return false;

	// in case only commit failed
	// need to abort running transaction prior to rollback
	if ( !bUpdate )
		bOk = tMonitor.Commit ();
	else
		bOk = tMonitor.UpdateTOI ();

	if ( !bOk )
	{
		tWriteSet.AbortPreCommit();
		return false;
	}

	tWriteSet.InterimCommit();
	tWriteSet.PostCommit();
	return true;
}

// replicate serialized data into cluster in TotalOrderIsolation mode and call commit monitor along
static bool ReplicateTOI ( const VecTraits_T<uint64_t> & dKeys, const VecTraits_T<BYTE> & tQueries, Wsrep::Writeset_i & tWriteSet, CommitMonitor_c && tMonitor )
{
	bool bOk = tWriteSet.ToExecuteStart ( dKeys, tQueries );
	sphLogDebugRpl ( "replicating TOI %d, seq " INT64_FMT, (int)bOk, tWriteSet.LastSeqno() );
	if ( !bOk )
		return false;

	// FIXME!!! can not fail TOI transaction
	tMonitor.CommitTOI();
	tWriteSet.ToExecuteEnd();

	sphLogDebugRpl ( "%s seq " INT64_FMT, tWriteSet.LastOk() ? "commited" : "rolled-back", tWriteSet.LastSeqno() );
	return tWriteSet.LastOk();
}

// get cluster status variables (our and Galera)
void ReplicationCluster_t::ShowStatus ( VectorLike& dOut )
{
	if ( !m_pProvider )
		return;

	assert ( m_eStatus >= Wsrep::ViewStatus_e::PRIMARY && m_eStatus < Wsrep::ViewStatus_e::MAX );

	const char* sName = m_sName.cstr();

	// cluster vars
	if ( dOut.MatchAdd ( "cluster_name" ) )
		dOut.Add ( m_sName );
	if ( dOut.MatchAddf ( "cluster_%s_state_uuid", sName ) )
		dOut.Add ( Wsrep::Uuid2Str ( m_dUUID ) );
	if ( dOut.MatchAddf ( "cluster_%s_conf_id", sName ) )
		dOut.Add().SetSprintf ( INT64_FMT, m_iConfID );
	if ( dOut.MatchAddf ( "cluster_%s_status", sName ) )
		dOut.Add() = Wsrep::GetViewStatus ( m_eStatus );
	if ( dOut.MatchAddf ( "cluster_%s_size", sName ) )
		dOut.Add().SetSprintf ( "%d", m_iSize );
	if ( dOut.MatchAddf ( "cluster_%s_local_index", sName ) )
		dOut.Add().SetSprintf ( "%d", m_iIdx );
	if ( dOut.MatchAddf ( "cluster_%s_node_state", sName ) )
		dOut.Add ( szState() );
	// nodes of cluster defined and view
	if ( dOut.MatchAddf ( "cluster_%s_nodes_set", sName ) )
		dOut.Add ( StrVec2Str ( m_dClusterNodes ).cstr() );

	if ( dOut.MatchAddf ( "cluster_%s_nodes_view", sName ) )
	{
		StringBuilder_c sNodes ( "," );
		FilterViewNodes ( [&sNodes] ( const CSphString& sNode ) { sNodes << sNode; } );
		dOut.Add ( sNodes.cstr() );
	}

	// cluster indexes
	if ( dOut.MatchAddf ( "cluster_%s_indexes_count", sName ) )
		WithRlockedIndexes ( [&dOut] ( const auto& hIndexes ) { dOut.Addf ( "%d", hIndexes.GetLength() ); } );

	if ( dOut.MatchAddf ( "cluster_%s_indexes", sName ) )
	{
		StringBuilder_c tBuf ( "," );
		WithRlockedIndexes ( [&tBuf] ( const auto& hIndexes ) { for_each ( hIndexes, [&] ( const auto& tIndex ) { tBuf << tIndex.first; } ); } );
		dOut.Add ( tBuf.cstr() );
	}

	// show last cluster error if any
	if ( dOut.Matchf ( "cluster_%s_last_error", sName ) )
	{
		ScopedMutex_t tErrorLock ( m_tErrorLock );
		if ( !m_sError.IsEmpty() ) {
			dOut.Addf ( "cluster_%s_last_error", sName );
			dOut.Add ( m_sError.cstr() );
		}
	}

	// cluster status
	m_pProvider->EnumStatsVars ( [&dOut,&sName] ( const Wsrep::StatsVars_t& tVar ) {
		if ( !dOut.MatchAddf ( "cluster_%s_%s", sName, tVar.m_szName ) )
			return;

		switch ( tVar.m_eType )
		{
		case Wsrep::StatsVars_t::STRING:
			dOut.Add ( tVar.m_tValue.szString );
			return;

		case Wsrep::StatsVars_t::DOUBLE:
			dOut.Addf ( "%f", tVar.m_tValue.fDouble );
			return;

		case Wsrep::StatsVars_t::INT64:
			dOut.Addf ( "%l", tVar.m_tValue.iVal64 );
			return;

		default:
			assert ( 0 && "Internal error" );
		}
	} );
}

// set Galera option for cluster
bool ReplicateSetOption ( const CSphString& sCluster, const CSphString& sName, const CSphString& sVal )
{
	auto pCluster = ClusterByName ( sCluster, "unknown cluster '%s' in SET statement" );
	if ( !pCluster )
		return false;

	if ( !pCluster->m_pProvider->OptionsSet ( sName, sVal ) )
		return false;

	pCluster->WithWlockedOptions ( [&] ( ClusterOptions_t& hOptions ) { hOptions.m_hOptions.Add ( sVal, sName ); } );
	return true;
}

// delete all clusters on daemon shutdown
void ReplicationServiceShutdown() EXCLUDES ( g_tClustersLock )
{
	Threads::CallCoroutine ( [] {
		Threads::SccWL_t wLock ( g_tClustersLock );
		if ( g_hClusters.IsEmpty() )
			return;

		sphLogDebugRpl ( "clusters (%d) delete invoked", g_hClusters.GetLength() );
		for ( auto & tCluster : g_hClusters )
		{
			sphLogDebugRpl ( "cluster '%s' delete", tCluster.first.cstr() );
			tCluster.second->DisconnectAndDeleteProvider();
		}

		sphLogDebugRpl ( "clusters delete done" );
		g_hClusters.Reset();
	} );
}


// collect clusters and their indexes
CSphVector<ClusterDesc_t> ReplicationCollectClusters ()  EXCLUDES ( g_tClustersLock )
{
	CSphVector<ClusterDesc_t> dClusters;

	if ( !ReplicationEnabled() )
		return dClusters;

	Threads::SccRL_t tLock ( g_tClustersLock );
	for ( const auto& tCluster : g_hClusters )
	{
		// should save all clusters on start
		// but skip cluster that just joining from user request
		if ( tCluster.second->GetState() != ClusterState_e::JOINING || !tCluster.second->m_bUserRequest )
			dClusters.Add ( *tCluster.second );
	}
	return dClusters;
}


// dump all clusters statuses
void ReplicateClustersStatus ( VectorLike & dStatus ) EXCLUDES ( g_tClustersLock )
{
	Threads::SccRL_t tLock ( g_tClustersLock );
	for_each ( g_hClusters, [&dStatus] (const auto& tCluster) { tCluster.second->ShowStatus ( dStatus ); });
}

// check whether index with given name exists and it is mutable (pq or rt) or distributed
static bool CheckIndexExists ( const CSphString & sIndex )
{
	cServedIndexRefPtr_c pServed = GetServed ( sIndex );
	bool bMutable = ServedDesc_t::IsMutable ( pServed );
	cDistributedIndexRefPtr_t pDist ( !bMutable ? GetDistr ( sIndex ) : nullptr );
	if ( !bMutable && !pDist )
		return TlsMsg::Err ( "unknown table '%s'", sIndex.cstr() );

	if ( ServedDesc_t::IsMutable ( pServed ) || pDist )
		return true;
	else
		return TlsMsg::Err ( "wrong type of table '%s'", sIndex.cstr() );
}

static StrVec_t SplitIndexes ( const CSphString & sIndexes )
{
	const char * sIndexesNameDel = ",` ";
	StrVec_t dRes;

	sphSplitApply ( sIndexes.cstr(), sIndexes.Length(), sIndexesNameDel, [&dRes] ( const char * sTok, int iLen )
	{
		if ( !iLen )
			return;
		dRes.Add().SetBinary ( sTok, iLen );
	});

	return dRes;
}

static bool CheckIndexesExists ( const CSphString & sIndex )
{
	StrVec_t dIndexes = SplitIndexes ( sIndex );
	for ( const CSphString & sIndex : dIndexes )
	{
		if ( !CheckIndexExists ( sIndex ) )
			return false;
	}

	return true;
}

// set cluster name into index desc for fast rejects
bool AssignClusterToIndex ( const CSphString & sIndex, const CSphString & sCluster )
{
	TlsMsg::ResetErr();
	cServedIndexRefPtr_c pServed = GetServed ( sIndex );
	bool bMutable = ServedDesc_t::IsMutable ( pServed );
	cDistributedIndexRefPtr_t pDist ( !bMutable ? GetDistr ( sIndex ) : nullptr );
	if ( !bMutable && !pDist )
		return TlsMsg::Err ( "unknown table, or wrong type of table '%s'", sIndex.cstr() );

	if ( bMutable )
	{
		if ( pServed->m_sCluster==sCluster )
			return true;

		ServedIndexRefPtr_c pClone = MakeFullClone ( pServed );
		pClone->m_sCluster = sCluster;
		g_pLocalIndexes->Replace ( pClone, sIndex );
	} else
	{
		if ( pDist->m_sCluster==sCluster )
			return true;

		DistributedIndexRefPtr_t pNewDist ( pDist->Clone() );
		pNewDist->m_sCluster = sCluster;
		g_pDistIndexes->Replace ( pNewDist, sIndex );
	}

	return true;
}

bool AssignClusterToIndexes ( const VecTraits_T<CSphString> & dIndexes, const CSphString & sCluster )
{
	for ( const CSphString & sIndex : dIndexes )
	{
		if ( !AssignClusterToIndex ( sIndex, sCluster ) )
			return false;
	}

	return true;
}

// lock or unlock write operations to disk chunks of index
static bool EnableIndexWrite ( const CSphString & sIndex )
{
	cServedIndexRefPtr_c pServed = GetServed ( sIndex );
	bool bMutable = ServedDesc_t::IsMutable ( pServed );
	cDistributedIndexRefPtr_t pDist ( !bMutable ? GetDistr ( sIndex ) : nullptr );
	if ( !bMutable && !pDist )
		return TlsMsg::Err ( "unknown or wrong-typed table '%s'", sIndex.cstr() );

	if ( pDist )
		return true;

	RIdx_T<RtIndex_i*> ( pServed )->EnableSave();
	return true;
}

// handle replicated command (came from outside)
bool HandleCmdReplicated ( RtAccum_t & tAcc )
{
	TRACE_SCHED ( "conn", "HandleCmdReplicated" );
	TlsMsg::ResetErr();

	if ( tAcc.m_dCmd.IsEmpty() )
		return TlsMsg::Err ( "empty accumulator" );

	const ReplicationCommand_t & tCmd = *tAcc.m_dCmd[0];
	bool bCmdCluster = ( tAcc.m_dCmd.GetLength()==1
		&& ( tCmd.m_eCommand==ReplCmd_e::CLUSTER_ALTER_ADD || tCmd.m_eCommand==ReplCmd_e::CLUSTER_ALTER_DROP ) );
	if ( bCmdCluster )
	{
		if ( tCmd.m_eCommand==ReplCmd_e::CLUSTER_ALTER_ADD && !CheckIndexesExists ( tCmd.m_sIndex ) )
			return TlsMsg::Err ( "replication error: %s, command %d, cluster %s", TlsMsg::szError(), (int)tCmd.m_eCommand, tCmd.m_sCluster.cstr() );

		CommitMonitor_c tCommit ( tAcc );
		return tCommit.CommitTOI();
	}

	if ( tAcc.m_dCmd.GetLength()==1 &&
		( tCmd.m_eCommand== ReplCmd_e::UPDATE_API
			 || tCmd.m_eCommand== ReplCmd_e::UPDATE_QL
			 || tCmd.m_eCommand== ReplCmd_e::UPDATE_JSON ) )
	{
		int iUpd = -1;
		CSphString sWarning;
		CommitMonitor_c tCommit ( tAcc, &sWarning, &iUpd );
		return tCommit.UpdateTOI() && ( sWarning.IsEmpty() || TlsMsg::Err ( "%s", sWarning.cstr() ) );

		// FIXME!!! make update trx
	}

	cServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sIndex );
	if ( !pServed || !ServedDesc_t::IsMutable ( pServed ) )
		return TlsMsg::Err ( "wrong type of table '%s' for replication, command %d", tCmd.m_sIndex.cstr(), (int)tCmd.m_eCommand );

	CSphString sError;
	// special path with wlocked index for truncate
	if ( tCmd.m_eCommand == ReplCmd_e::TRUNCATE )
	{
		RPL_TNX << "truncate-commit, table '" << tCmd.m_sIndex.cstr() << "'";
		return WIdx_T<RtIndex_i*> ( pServed )->Truncate ( sError, RtIndex_i::TRUNCATE )
			|| TlsMsg::Err ( "%s", sError.cstr() );
	}

	assert ( tCmd.m_eCommand != ReplCmd_e::TRUNCATE );
	RPL_TNX << "commit, table '" << tCmd.m_sIndex.cstr() << "', uid " << ( tCmd.m_pStored ? tCmd.m_pStored->m_iQUID : int64_t(0) ) << ", queries " << tCmd.m_dDeleteQueries.GetLength() << ", tags " << tCmd.m_sDeleteTags.scstr();

	RIdx_T<RtIndex_i*> pIndex { pServed };
	if ( !tAcc.SetupDocstore ( *pIndex, sError ) )
	{
		sphWarning ( "%s, table '%s', command %d", sError.cstr(), tCmd.m_sIndex.cstr(), (int)tCmd.m_eCommand );
		return false;
	}

	return pIndex->Commit ( nullptr, &tAcc );
}

// single point there all commands passed these might be replicated to cluster
static bool HandleRealCmdReplicate ( RtAccum_t & tAcc, CommitMonitor_c && tMonitor ) EXCLUDES ( g_tClustersLock )
{
	TRACE_CONN ( "conn", "HandleCmdReplicate" );

	// FIXME!!! for now only PQ add and PQ delete multiple commands supported
	const ReplicationCommand_t & tCmdCluster = *tAcc.m_dCmd[0];

	auto pCluster = ClusterByName ( tCmdCluster.m_sCluster, nullptr );
	if ( !pCluster )
	{
		if ( g_bReplicationStarted )
			return TlsMsg::Err ( "unknown cluster '%s'", tCmdCluster.m_sCluster.cstr() );

		return TlsMsg::Err ( "cluster '%s' can not replicate: %s", tCmdCluster.m_sCluster.cstr(), g_sReplicationStartError.cstr() );
	}
	if ( !pCluster->IsHealthy() )
		return false;

	if ( tCmdCluster.m_eCommand==ReplCmd_e::TRUNCATE && tCmdCluster.m_tReconfigure )
		return TlsMsg::Err ( "RECONFIGURE is not supported for a cluster table" );

	if ( tCmdCluster.m_bCheckIndex )
	{
		if ( tCmdCluster.m_eCommand==ReplCmd_e::CLUSTER_ALTER_ADD || tCmdCluster.m_eCommand==ReplCmd_e::CLUSTER_ALTER_DROP )
		{
			StrVec_t dIndexes = SplitIndexes ( tCmdCluster.m_sIndex );
			if ( !CheckClusterIndexes ( dIndexes, pCluster ) )
				return false;
		} else if ( !CheckClusterIndex ( tCmdCluster.m_sIndex, pCluster ) )
			return false;
	}

	bool bUpdate = ( tCmdCluster.m_eCommand==ReplCmd_e::UPDATE_API
		|| tCmdCluster.m_eCommand==ReplCmd_e::UPDATE_QL
		|| tCmdCluster.m_eCommand==ReplCmd_e::UPDATE_JSON );
	if ( bUpdate && !ValidateUpdate ( tCmdCluster ) )
		return false;

	assert ( pCluster->m_pProvider );
	bool bTOI = false;

	// replicator check CRC of data - no need to check that at our side
	int iKeysCount = tAcc.m_dCmd.GetLength() + tAcc.m_dAccumKlist.GetLength();
	CSphVector<BYTE> dBufQueries;
	CSphVector<uint64_t> dBufKeys;
	dBufKeys.Reserve ( iKeysCount );

	BEGIN_CONN ( "conn", "HandleCmdReplicate.serialize", "commands", tAcc.m_dCmd.GetLength() );
	for ( auto& pCmd : tAcc.m_dCmd )
	{
		const ReplicationCommand_t & tCmd = *pCmd;
		uint64_t uQueryHash = sphFNV64 ( tCmdCluster.m_sIndex.cstr() );
		uQueryHash = sphFNV64 ( &tCmd.m_eCommand, sizeof(tCmd.m_eCommand), uQueryHash );

		MemoryWriter_c tWriter ( dBufQueries );
		SaveCmdHeader ( tCmd, tWriter );

		auto iLenPos = dBufQueries.GetLength();
		dBufQueries.AddN ( sizeof(DWORD) );
		int iLenOff = dBufQueries.GetLength();

		switch ( tCmd.m_eCommand )
		{
		case ReplCmd_e::PQUERY_ADD:
			assert ( tCmd.m_pStored );
			SaveStoredQuery ( *tCmd.m_pStored, tWriter );

			uQueryHash = sphFNV64 ( &tCmd.m_pStored->m_iQUID, sizeof(tCmd.m_pStored->m_iQUID), uQueryHash );
			break;

		case ReplCmd_e::PQUERY_DELETE:
			assert ( tCmd.m_dDeleteQueries.GetLength() || !tCmd.m_sDeleteTags.IsEmpty() );
			SaveDeleteQuery ( tCmd.m_dDeleteQueries, tCmd.m_sDeleteTags.cstr(), tWriter );

			for ( const auto& dQuery : tCmd.m_dDeleteQueries )
				uQueryHash = sphFNV64 ( &dQuery, sizeof ( dQuery ), uQueryHash );

			uQueryHash = sphFNV64cont ( tCmd.m_sDeleteTags.cstr(), uQueryHash );
			break;

		case ReplCmd_e::TRUNCATE:
			// FIXME!!! add reconfigure option here
			break;

		case ReplCmd_e::CLUSTER_ALTER_ADD:
		case ReplCmd_e::CLUSTER_ALTER_DROP:
			bTOI = true;
			break;

		case ReplCmd_e::RT_TRX:
		{
			auto iStartPos = dBufQueries.GetLengthBytes();
			tAcc.SaveRtTrx ( tWriter );
			uQueryHash = sphFNV64cont( { dBufQueries.begin() + iStartPos, (int64_t)(dBufQueries.GetLengthBytes() - iStartPos) }, uQueryHash);
		}
		break;

		case ReplCmd_e::UPDATE_API:
		{
			assert ( tCmd.m_pUpdateAPI );
			const CSphAttrUpdate * pUpd = tCmd.m_pUpdateAPI;

			uQueryHash = sphFNV64 ( pUpd->m_dDocids.Begin(), (int) pUpd->m_dDocids.GetLengthBytes(), uQueryHash );
			uQueryHash = sphFNV64 ( pUpd->m_dPool.Begin(), (int) pUpd->m_dPool.GetLengthBytes(), uQueryHash );

			SaveAttrUpdate ( *pUpd, tWriter );
		}
		break;

		case ReplCmd_e::UPDATE_QL:
		case ReplCmd_e::UPDATE_JSON:
		{
			assert ( tCmd.m_pUpdateAPI );
			assert ( tCmd.m_pUpdateCond );
			const CSphAttrUpdate * pUpd = tCmd.m_pUpdateAPI;

			uQueryHash = sphFNV64 ( pUpd->m_dDocids.Begin(), (int) pUpd->m_dDocids.GetLengthBytes(), uQueryHash );
			uQueryHash = sphFNV64 ( pUpd->m_dPool.Begin(), (int) pUpd->m_dPool.GetLengthBytes(), uQueryHash );

			SaveAttrUpdate ( *pUpd, tWriter );
			SaveUpdate ( *tCmd.m_pUpdateCond, tWriter );
		}
		break;

		default:
			return TlsMsg::Err ( "unknown command '%d'", (int)tCmd.m_eCommand );
		}

		// store query hash as key
		dBufKeys.Add (uQueryHash);

		if ( tCmd.m_eCommand == ReplCmd_e::RT_TRX )
			dBufKeys.Append ( tAcc.m_dAccumKlist );
		// store request length
		sphUnalignedWrite ( &dBufQueries[iLenPos], dBufQueries.GetLength() - iLenOff );
	}
	END_CONN ( "conn" );

	auto pWriteSet = pCluster->m_pProvider->MakeWriteSet();

	BEGIN_CONN ( "conn", "HandleCmdReplicate.cluster_lock" );
	Threads::ScopedCoroMutex_t tClusterLock { pCluster->m_tReplicationMutex };
	END_CONN ( "conn" );

	if ( bTOI )
		return ReplicateTOI ( dBufKeys, dBufQueries, *pWriteSet, std::move ( tMonitor ) );
	return Replicate ( dBufKeys, dBufQueries, *pWriteSet, std::move ( tMonitor ), bUpdate, tAcc.IsReplace () );
}


// single point there all commands passed these might be replicated, even if no cluster
static bool HandleCmdReplicateImpl ( RtAccum_t & tAcc, int * pDeletedCount, CSphString * pWarning, int * pUpdated ) EXCLUDES ( g_tClustersLock )
{
	TRACE_CORO ( "sph", "HandleCmdReplicateImpl" );

	CommitMonitor_c tMonitor ( tAcc, pDeletedCount, pWarning, pUpdated );

	// with cluster path
	if ( tAcc.IsClusterCommand () )
		return HandleRealCmdReplicate ( tAcc, std::move ( tMonitor ) );

	if ( tAcc.IsUpdateCommand () )
		return tMonitor.UpdateTOI ();
	return tMonitor.Commit ();
}

bool HandleCmdReplicate ( RtAccum_t & tAcc )
{
	return HandleCmdReplicateImpl ( tAcc, nullptr, nullptr, nullptr );
}

bool HandleCmdReplicateDelete ( RtAccum_t & tAcc, int & iDeletedCount )
{
	return HandleCmdReplicateImpl ( tAcc, &iDeletedCount, nullptr, nullptr );
}

bool HandleCmdReplicateUpdate ( RtAccum_t & tAcc, CSphString & sWarning, int & iUpdated )
{
	return HandleCmdReplicateImpl ( tAcc, nullptr, &sWarning, &iUpdated );
}


bool SetIndexesClusterTOI ( const ReplicationCommand_t * pCmd )
{
	assert ( pCmd );
	const ReplicationCommand_t& tCmd = *pCmd;
	assert ( tCmd.m_eCommand==ReplCmd_e::CLUSTER_ALTER_DROP || tCmd.m_eCommand==ReplCmd_e::CLUSTER_ALTER_ADD );
	auto pCluster = ClusterByName ( tCmd.m_sCluster );
	if ( !pCluster )
		return false;

	StrVec_t dIndexes = SplitIndexes ( tCmd.m_sIndex );

	sphLogDebugRpl ( "SetIndexesClusterTOI '%s' for cluster '%s': indexes '%s' > '%s'", ( tCmd.m_eCommand==ReplCmd_e::CLUSTER_ALTER_ADD ? "add" : "drop" ), pCluster->m_sName.cstr(), tCmd.m_sIndex.cstr(), StrVec2Str ( pCluster->GetIndexes() ).cstr() );

	if ( tCmd.m_bCheckIndex && !CheckClusterIndexes ( dIndexes, pCluster ) )
		return false;

	if ( tCmd.m_eCommand==ReplCmd_e::CLUSTER_ALTER_ADD )
	{
		if ( !AssignClusterToIndexes ( dIndexes, tCmd.m_sCluster ) )
			return false;

		pCluster->WithWlockedIndexes ( [&] ( auto & hIndexes, auto & hIndexesLoaded )
		{
			for ( const CSphString & sIndex : dIndexes )
			{
				hIndexes.Add ( sIndex );
				hIndexesLoaded.Delete ( sIndex );
			}
		});

	} else
	{
		if ( !AssignClusterToIndexes ( dIndexes, "" ) )
			return false;

		pCluster->WithWlockedIndexes ( [&] ( auto & hIndexes, auto & hIndexesLoaded )
		{
			for ( const CSphString & sIndex : dIndexes )
				hIndexes.Delete ( sIndex );
		});
	}

	TLS_MSG_STRING ( sError );
	bool bSaved = SaveConfigInt ( sError );

	sphLogDebugRpl ( "SetIndexesClusterTOI finished '%s' for cluster '%s': indexes '%s' > '%s', error: %s", ( tCmd.m_eCommand==ReplCmd_e::CLUSTER_ALTER_ADD ? "add" : "drop" ), pCluster->m_sName.cstr(), tCmd.m_sIndex.cstr(), StrVec2Str ( pCluster->GetIndexes() ).cstr(), sError.scstr() );

	return bSaved;
}

static bool ValidateUpdate ( const ReplicationCommand_t & tCmd )
{
	cServedIndexRefPtr_c pServed = GetServed ( tCmd.m_sIndex );
	if ( !pServed )
		return TlsMsg::Err ( "requires an existing table, %s", tCmd.m_sIndex.cstr() );

	const ISphSchema& tSchema = RIdx_c ( pServed )->GetMatchSchema();

	assert ( tCmd.m_pUpdateAPI );
	CSphString sError;
	return Update_CheckAttributes ( *tCmd.m_pUpdateAPI, tSchema, sError ) || TlsMsg::Err ( sError );
}

// load indexes received from another node or existed already into daemon
static bool ReplicatedIndexes ( const VecTraits_T<CSphString> & dIndexes, const CSphString & sCluster ) EXCLUDES ( g_tClustersLock )
{
	assert ( ReplicationEnabled() );

	if ( !dIndexes.all_of ( [] ( const CSphString & sIndex ) { return CheckIndexExists ( sIndex ); } ) )
		return false;

	auto pCluster = ClusterByName( sCluster );
	if ( !pCluster )
		return false;

	sph::StringSet hIndexes ( dIndexes );

	// scope for check of cluster data
	{
		Threads::SccRL_t rLock( g_tClustersLock );

		// indexes should be new or from same cluster
		for ( const auto& tCluster : g_hClusters )
		{
			const ReplicationCluster_t * pOrigCluster = tCluster.second;
			if ( pOrigCluster==pCluster.CPtr() )
				continue;
			
			bool bHasCluster = pOrigCluster->WithRlockedIndexes([&hIndexes,pOrigCluster]( const auto & hOrigIndexes )
			{
				for ( const auto & tIndex : hOrigIndexes )
				{
					if ( hIndexes[tIndex.first] )
						return TlsMsg::Err ( "table '%s' is already a part of cluster '%s'", tIndex.first.cstr(), pOrigCluster->m_sName.cstr() );
				}

				return true;
			});
			if ( !bHasCluster )
				return false;
		}
	}

	bool bOk = AssignClusterToIndexes ( dIndexes, sCluster );

	// need to enable back local index write
	for ( const CSphString & sIndex : dIndexes )
		bOk &= EnableIndexWrite ( sIndex );

	if ( !bOk )
		return false;

	pCluster->WithWlockedIndexes([&dIndexes] ( auto & hIndexes, auto & hIndexesLoaded )
	{
		hIndexes.Reset();
		dIndexes.for_each ( [&hIndexes, &hIndexesLoaded] ( const auto & sIndex )
		{
			hIndexes.Add ( sIndex );
			hIndexesLoaded.Delete ( sIndex );
		});
	});

	TLS_MSG_STRING ( sError );
	return SaveConfigInt ( sError );
}

// create string by join cluster path and given path
std::optional<CSphString> GetClusterPath ( const CSphString & sCluster ) EXCLUDES ( g_tClustersLock )
{
	std::optional<CSphString> tRes;
	auto pCluster = ClusterByName ( sCluster );
	if ( pCluster )
		tRes = GetDatadirPath ( pCluster->m_sPath );
	return tRes;
}

// validate clusters paths
static bool ClusterCheckPath ( const CSphString& sPath, const char* szCluster, bool bNeedWrite = false ) EXCLUDES ( g_tClustersLock )
{
	if ( !ReplicationEnabled() )
		return TlsMsg::Err ( "data_dir option is missing in config or no replication listener is set, replication is disabled" );

	auto sFullPath = GetDatadirPath ( sPath );
	CSphString sError;
	if ( !CheckPath ( sFullPath, bNeedWrite, sError ) )
		return TlsMsg::Err ( "cluster '%s', %s", szCluster, sError.cstr() );

	Threads::SccRL_t tClusterRLock ( g_tClustersLock );
	for ( const auto& tCluster : g_hClusters )
	{
		if ( sPath == tCluster.second->m_sPath )
			return TlsMsg::Err ( "duplicate paths, cluster '%s' has the same path as '%s'", szCluster, tCluster.second->m_sName.cstr() );
	}
	return true;
}

static ReplicationCluster_t* MakeClusterOffline ( ClusterDesc_t tDesc )
{
	auto tPort = PortRange::AcquirePort();
	if ( !tPort.IsValid() )
	{
		TlsMsg::Err ( "cluster '%s', no replication ports available, add replication listener", tDesc.m_sName.cstr() );
		return nullptr;
	}

	CSphRefcountedPtr<ReplicationCluster_t> pCluster ( new ReplicationCluster_t ( std::move ( tDesc ) ) );
	pCluster->m_tPort = std::move ( tPort );

	return pCluster.Leak();
}

static bool PrepareNodesAndInitCluster ( ReplicationCluster_t& tCluster, BOOTSTRAP_E eBootStrap )
{
	sphLogDebugRpl ( "PrepareNodesAndInitCluster '%s', bootstrap %d, nodes: %d", tCluster.m_sName.cstr(), (int)eBootStrap, tCluster.m_dClusterNodes.GetLength() );

	if ( tCluster.m_dClusterNodes.IsEmpty() && eBootStrap!=BOOTSTRAP_E::YES )
	{
		sphWarning ( "no nodes found, created new cluster '%s'", tCluster.m_sName.cstr() );
		eBootStrap = BOOTSTRAP_E::YES;
	}

	StrVec_t dNodes;
	if ( eBootStrap == BOOTSTRAP_E::NO )
	{
		dNodes = GetNodeListFromRemotes ( tCluster );
		if ( dNodes.IsEmpty() )
			return false;
	}

	if ( !tCluster.Init () )
		return false;

	tCluster.SetViewNodes ( std::move ( dNodes ) );
	tCluster.m_dClusterNodes = tCluster.FilterViewNodesByProto ();
	return true;
}

static ReplicationCluster_t* MakeCluster ( ClusterDesc_t tDesc, BOOTSTRAP_E eBootStrap )
{
	CSphRefcountedPtr<ReplicationCluster_t> pCluster ( MakeClusterOffline ( std::move ( tDesc ) ) );
	if ( !pCluster )
		return nullptr;

	if ( !PrepareNodesAndInitCluster ( *pCluster, eBootStrap) )
		return nullptr;
	return pCluster.Leak();
}

static bool AddAndStartCluster ( ReplicationClusterRefPtr_c pCluster )
{
	{
		Threads::SccWL_t tLock ( g_tClustersLock );
		if ( !g_hClusters.Add ( pCluster, pCluster->m_sName ) )
			return false;
	}

	pCluster->StartListen();
	return true;
}

static bool ClusterDescOk ( const ClusterDesc_t& tDesc, bool bForce ) noexcept
{
	if ( tDesc.m_dClusterNodes.IsEmpty() )
		sphWarning ( "no nodes found, create new cluster '%s'", tDesc.m_sName.cstr() );

	// check cluster path is unique
	if ( !ClusterCheckPath ( tDesc.m_sPath, tDesc.m_sName.cstr() ) )
	{
		sphWarning ( "Cluster %s: %s, skipped", tDesc.m_sName.cstr(), TlsMsg::szError() );
		return false;
	}

	if ( !bForce )
		return true;

	auto sDataDirPath = GetDatadirPath ( tDesc.m_sPath );
	if ( !NewClusterForce ( sDataDirPath ) || !CheckClusterNew ( sDataDirPath ) )
	{
		sphWarning ( "Cluster %s: %s, skipped", tDesc.m_sName.cstr(), TlsMsg::szError() );
		return false;
	}

	return true;
}

// called from ReplicationServiceStart - see below
static void CoReplicationServiceStart ( bool bBootStrap ) EXCLUDES ( g_tClustersLock )
{
	const auto eBootStrap = (BOOTSTRAP_E)bBootStrap;
	assert ( Threads::IsInsideCoroutine() );
	StrVec_t dFailedClustersToRemove;
	auto fnRemoveFailedCluster = [&dFailedClustersToRemove] ( std::pair<CSphString, CSphRefcountedPtr<ReplicationCluster_t>>& hCluster ) {
		sphWarning ( "%s", TlsMsg::szError() );
		dFailedClustersToRemove.Add ( hCluster.first );
		auto& pCluster = hCluster.second;
		pCluster->WithRlockedIndexes ( [] ( const auto& hIndexes ) { for_each ( hIndexes, [] ( const auto& tIndex ) { AssignClusterToIndex ( tIndex.first, "" ); } ); } );
	};

	Threads::SccWL_t tLock ( g_tClustersLock );
	for ( auto& hCluster : g_hClusters )
	{
		TlsMsg::ResetErr();
		auto& sName = hCluster.first;
		auto& pCluster = hCluster.second;
		if ( !PrepareNodesAndInitCluster ( *pCluster, eBootStrap ) )
		{
			fnRemoveFailedCluster ( hCluster );
			continue;
		}

		// check indexes valid
		if ( !pCluster->Connect ( eBootStrap ) )
		{
			fnRemoveFailedCluster ( hCluster );
			continue;
		}

		pCluster->StartListen();
		sphLogDebugRpl ( "'%s' cluster started with %d tables", sName.cstr(), pCluster->WithRlockedIndexes ( [] ( const auto& hIndexes ) { return hIndexes.GetLength(); } ) );
	}
	if ( !g_hClusters.IsEmpty() && dFailedClustersToRemove.GetLength()==g_hClusters.GetLength() )
		sphWarning ( "no clusters to start" );

	for ( const auto& sCluster : dFailedClustersToRemove )
		g_hClusters.Delete ( sCluster );

	g_bReplicationStarted = true;
}

// start clusters on daemon start
void ReplicationServiceStart ( bool bBootStrap ) EXCLUDES ( g_tClustersLock )
{
	// should be lined up with PrepareClustersOnStartup
	if ( !ReplicationEnabled() )
	{
		Threads::SccRL_t tLock ( g_tClustersLock );
		if ( !g_hClusters.IsEmpty() )
			sphWarning ( "loading %d cluster(s) but the replication disabled, %s", g_hClusters.GetLength(), g_sReplicationStartError.cstr() );
		return;
	}

	Threads::CallCoroutine ( [=]() EXCLUDES ( g_tClustersLock ) { CoReplicationServiceStart ( bBootStrap ); } );
}

// called from ReplicationServiceStart - see below
static void CoPrepareClustersOnStartup ( bool bForce ) EXCLUDES ( g_tClustersLock )
{
	SmallStringHash_T<ReplicationClusterRefPtr_c> hClusters;
	assert ( Threads::IsInsideCoroutine() );
	for ( const ClusterDesc_t& tDesc : GetClustersInt() )
	{
		if ( !ClusterDescOk ( tDesc, bForce ) )
			continue;

		if ( !CheckRemotesVersions ( tDesc, false ) )
		{
			sphWarning ( "%s", TlsMsg::szError() );
			continue;
		}

		CSphRefcountedPtr<ReplicationCluster_t> pNewCluster { MakeClusterOffline ( tDesc ) };
		if ( !pNewCluster ) {
			sphWarning ( "%s", TlsMsg::szError() );
			continue;
		}

		// check indexes valid
		for ( const auto & tIndex : tDesc.m_hIndexes )
		{
			const CSphString & sIndex = tIndex.first;
			if ( !AssignClusterToIndex ( sIndex, pNewCluster->m_sName ) )
			{
				sphWarning ( "%s, removed from cluster '%s'", TlsMsg::szError(), pNewCluster->m_sName.cstr() );
				continue;
			}
			pNewCluster->WithWlockedIndexes ( [&sIndex] ( auto & hIndexes, auto & hIndexesLoaded )
			{
				hIndexes.Add ( sIndex );
				hIndexesLoaded.Delete ( sIndex );
			});
		}

		if ( !hClusters.Add ( pNewCluster, pNewCluster->m_sName ) )
		{
			for ( const auto & tIndex : tDesc.m_hIndexes )
			{
				if ( !AssignClusterToIndex ( tIndex.first, "" ) )
					sphWarning ( "%s on removal table '%s' from a cluster", TlsMsg::szError(), tIndex.first.cstr() );
			}
			continue;
		}
	}

	// copy prepared clusters
	Threads::SccWL_t tLock ( g_tClustersLock );
	assert ( g_hClusters.IsEmpty() );
	for_each ( hClusters, [&] ( auto& hCluster ) REQUIRES ( g_tClustersLock ) { g_hClusters.Add ( hCluster.second, hCluster.first ); } );
}

void PrepareClustersOnStartup ( const VecTraits_T<ListenerDesc_t>& dListeners, bool bForce ) EXCLUDES ( g_tClustersLock )
{
	if ( !SetReplicationListener ( dListeners, g_sReplicationStartError ) )
	{
		if ( !GetClustersInt().IsEmpty() )
			sphWarning ( "%s", g_sReplicationStartError.cstr() );
		else
			sphLogDebugRpl ( "%s", g_sReplicationStartError.cstr() );
		return;
	}
	g_bReplicationEnabled = true;

	Threads::CallCoroutine ( [=]() EXCLUDES ( g_tClustersLock ) { CoPrepareClustersOnStartup ( bForce ); } );
}

// validate cluster option at SphinxQL statement
static std::optional <CSphString> CheckClusterOption ( const SmallStringHash_T<SqlInsert_t *> & hValues, const char * szName )
{
	SqlInsert_t ** ppVal = hValues ( szName );
	if (!ppVal)
		return "";

	if (( *ppVal )->m_sVal.IsEmpty())
	{
		TlsMsg::Err ( "'%s' should have a string value", szName );
		return {};
	}

	return ( *ppVal )->m_sVal;
}

// validate cluster SphinxQL statement
static bool CheckClusterExists ( const CSphString & sCluster ) EXCLUDES ( g_tClustersLock )
{
	Threads::SccRL_t rLock ( g_tClustersLock );
	return g_hClusters.Exists ( sCluster );
}

// validate cluster SphinxQL statement
enum class MAKE_E : bool { CREATE, JOIN };
static std::optional<ClusterDesc_t> ClusterDescFromSphinxqlStatement ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, MAKE_E eJoin ) EXCLUDES ( g_tClustersLock )
{
	std::optional<ClusterDesc_t> tDesc;
	if ( !ReplicationEnabled() )
	{
		TlsMsg::Err ( "data_dir option is missing or no replication provider configured" );
		return tDesc;
	}

	if ( CheckClusterExists ( sCluster ) )
	{
		TlsMsg::Err ( "cluster '%s' already exists", sCluster.cstr() );
		return tDesc;
	}

	SmallStringHash_T<SqlInsert_t*> hValues;
	assert ( dNames.GetLength() == dValues.GetLength() );
	ARRAY_FOREACH ( i, dNames )
		hValues.Add ( &dValues[i], dNames[i] );

	// optional items
	auto tNodes = CheckClusterOption ( hValues, "at_node" );
	if ( !tNodes )
		return tDesc;

	auto dClusterNodes = ParseNodesFromString ( tNodes.value() );
	if ( dClusterNodes.IsEmpty() )
	{
		tNodes = CheckClusterOption ( hValues, "nodes" );
		if ( !tNodes )
			return tDesc;
		dClusterNodes = ParseNodesFromString ( tNodes.value() );
	}

	if ( eJoin==MAKE_E::JOIN && dClusterNodes.IsEmpty() )
	{
		TlsMsg::Err ( "cannot join without either nodes list or AT node" );
		return tDesc;
	}

	auto tPath = CheckClusterOption ( hValues, "path" );
	if ( !tPath )
		return tDesc;

	auto tOptions = CheckClusterOption ( hValues, "options" );
	if ( !tOptions )
		return tDesc;

	// check cluster path is unique
	if ( !ClusterCheckPath ( tPath.value(), sCluster.cstr(), true ) )
		return tDesc;

	// all is ok, create cluster desc
	tDesc.emplace();
	tDesc->m_sName = sCluster;
	tDesc->m_sPath = tPath.value();
	tDesc->m_dClusterNodes = dClusterNodes;
	tDesc->m_tOptions.Parse ( tOptions.value() );
	return tDesc;
}

/////////////////////////////////////////////////////////////////////////////
// cluster joins to existed nodes
/////////////////////////////////////////////////////////////////////////////
bool ClusterJoin ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, bool bUpdateNodes ) EXCLUDES ( g_tClustersLock )
{
	Threads::ScopedCoroMutex_t tClusterLock { g_tClusterOpsLock };
	TlsMsg::ResetErr();
	auto tDesc = ClusterDescFromSphinxqlStatement ( sCluster, dNames, dValues, MAKE_E::JOIN );
	if ( !tDesc )
		return false;

	sphLogDebugRpl ( "joining cluster '%s', nodes: %s", sCluster.cstr(), StrVec2Str ( tDesc->m_dClusterNodes ).cstr() );

	// need to clean up Galera system files left from previous cluster
	CleanClusterFiles ( GetDatadirPath ( tDesc->m_sPath ) );

	if ( !CheckRemotesVersions ( tDesc.value(), true ) )
		return false;

	ReplicationClusterRefPtr_c pCluster { MakeCluster ( tDesc.value(), BOOTSTRAP_E::NO ) };
	if ( !pCluster )
		return false;

	pCluster->m_bUserRequest = true;

	if ( !pCluster->Connect ( BOOTSTRAP_E::NO ) )
		return false;

	if ( !AddAndStartCluster ( pCluster ) )
		return false;

	pCluster->m_bUserRequest = false;

	bool bOk = IsSyncedOrDonor ( pCluster->WaitReady() );
	if ( bOk && bUpdateNodes )
		bOk &= DoClusterAlterUpdate ( sCluster, "nodes", NODES_E::BOTH );

	if ( bOk )
		return true;

	if ( sphInterrupted() )
		return TlsMsg::Err ( "%s", "daemon shutdown" );

	{
		ScopedMutex_t tLock ( pCluster->m_tErrorLock );
		TlsMsg::Err ( pCluster->m_sError.cstr() );
	}

	sphWarning ( "'%s' cluster after join error: %s, nodes '%s'", sCluster.cstr(), TlsMsg::szError(), StrVec2Str ( pCluster->m_dClusterNodes ).cstr() );
	// need to wait recv thread to complete in case of error after worker started
	pCluster->m_bWorkerActive.Wait ( [] ( bool bWorking ) { return !bWorking; } );
	Threads::SccWL_t wLock ( g_tClustersLock );
	sphLogDebugRpl ( "deleting cluster %s", sCluster.cstr() );
	g_hClusters.Delete ( sCluster );
	return false;
}

 /////////////////////////////////////////////////////////////////////////////
// cluster creates master node
/////////////////////////////////////////////////////////////////////////////
bool ClusterCreate ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues ) EXCLUDES ( g_tClustersLock )
{
	Threads::ScopedCoroMutex_t tClusterLock { g_tClusterOpsLock };
	TlsMsg::ResetErr();
	if ( !g_bReplicationStarted )
		return TlsMsg::Err ( "can not create cluster '%s': %s", sCluster.cstr(), g_sReplicationStartError.cstr() );

	auto tDesc = ClusterDescFromSphinxqlStatement ( sCluster, dNames, dValues, MAKE_E::CREATE );
	if ( !tDesc )
		return TlsMsg::Err ( "failed to create desc: %s", TlsMsg::szError() );

	// need to clean up Galera system files left from previous cluster
	CleanClusterFiles ( GetDatadirPath ( tDesc->m_sPath ) );

	sphLogDebugRpl ( "creating cluster '%s', nodes: %s", sCluster.cstr(), StrVec2Str ( dNames ).cstr() );

	ReplicationClusterRefPtr_c pCluster { MakeCluster ( tDesc.value(), BOOTSTRAP_E::YES ) };
	if ( !pCluster )
		return TlsMsg::Err ( "failed to make cluster: %s", TlsMsg::szError() );

	if ( !pCluster->Connect ( BOOTSTRAP_E::YES ) )
		return TlsMsg::Err ( "Failed to connect %s", TlsMsg::szError() );

	if ( !AddAndStartCluster ( pCluster ) )
		return TlsMsg::Err ( "Failed to start and add %s", TlsMsg::szError() );

	auto eState = pCluster->WaitReady();
	TLS_MSG_STRING ( sError );
	return SaveConfigInt ( sError ) && ( IsSyncedOrDonor ( eState ) || TlsMsg::Err ( "Wrong state %s", szNodeState( eState ) ) );
}

// utility function to filter nodes list provided at string by specific protocol
StrVec_t ReplicationCluster_t::FilterViewNodesByProto ( Proto_e eProto, bool bResolve ) const
{
	Threads::SccRL_t tNodesRLock ( m_tViewNodesLock );
	return FilterNodesByProto ( m_dViewNodes, eProto, bResolve );
}

StrVec_t ReplicationCluster_t::GetViewNodes() const
{
	Threads::SccRL_t tNodesRLock ( m_tViewNodesLock );
	return m_dViewNodes;
}

void ReplicationCluster_t::SetViewNodes ( StrVec_t&& dNodes )
{
	Threads::SccWL_t tNodesWLock ( m_tViewNodesLock );
	m_dViewNodes = std::move ( dNodes );
}

StrVec_t ReplicationCluster_t::GetIndexes() const noexcept
{
	StrVec_t dIndexes;
	WithRlockedAllIndexes ( [&dIndexes] ( const auto & hIndexes, auto & hIndexesLoaded )
	{
		for ( const auto & tIndex : hIndexes )
			dIndexes.Add ( tIndex.first );
		for ( const auto & tIndex : hIndexesLoaded )
		{
			assert ( !hIndexes[tIndex.first] );
			dIndexes.Add ( tIndex.first );
		}
	});
	return dIndexes;
}

void ReportClusterError ( const CSphString & sCluster, const CSphString & sError, const char * szClient, E_CLUSTER eCmd )
{
	if ( sError.IsEmpty() )
		return;

	sphWarning ( "'%s' cluster [%s], cmd: %s(%d), error: %s", sCluster.cstr(), szClient, szClusterCmd ( eCmd ), (int)eCmd, sError.cstr() );

	auto pCluster = ClusterByName ( sCluster, nullptr );
	if ( !pCluster )
		return;

	ScopedMutex_t tErrorLock ( pCluster->m_tErrorLock );
	if ( pCluster->m_sError.GetLength()>1024 ) // collect up to 1024 chars
		pCluster->m_sError.Rewind();

	pCluster->m_sError += sError.cstr();
}

// command at remote node for CLUSTER_DELETE to delete cluster
bool ClusterDelete ( const CSphString & sCluster ) EXCLUDES ( g_tClustersLock )
{
	// erase cluster from all hashes
	ReplicationClusterRefPtr_c pCluster;
	{
		Threads::SccWL_t tLock ( g_tClustersLock );
		if ( !g_hClusters.Exists ( sCluster ) )
			return TlsMsg::Err ( "unknown cluster '%s'", sCluster.cstr() );

		pCluster = g_hClusters[sCluster];
		g_hClusters.Delete ( sCluster );
	}

	sphLogDebugRpl ( "remote delete cluster %s", sCluster.cstr() );
	// remove cluster from cache without delete of cluster itself
	pCluster->DisconnectAndDeleteProvider();
	pCluster->WithRlockedIndexes ( [] ( const auto& hIndexes ) { for_each ( hIndexes, [] ( const auto& tIndex ) { AssignClusterToIndex ( tIndex.first, "" ); } ); } );
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// cluster deletes
/////////////////////////////////////////////////////////////////////////////

// cluster delete every node, than itself
bool GloballyDeleteCluster ( const CSphString & sCluster, CSphString & sError ) EXCLUDES (g_tClustersLock)
{
	TlsMsg::Err();
	if ( !g_bReplicationStarted )
	{
		sError.SetSprintf ( "can not delete cluster '%s': %s", sCluster.cstr(), g_sReplicationStartError.cstr() );
		return false;
	}

	auto pCluster = ClusterByName ( sCluster );
	if ( !pCluster )
	{
		TlsMsg::MoveError ( sError );
		return false;
	}

	auto dNodes = pCluster->FilterViewNodesByProto();
	SendClusterDeleteToNodes ( dNodes, sCluster );
	bool bOk = ClusterDelete ( sCluster );
	bOk &= SaveConfigInt ( sError );

	TlsMsg::MoveError ( sError );
	return bOk;
}

bool ClusterGetState ( const CSphString & sCluster, RemoteNodeClusterState_t & tState ) EXCLUDES ( g_tClustersLock )
{
	auto pCluster = ClusterByName ( sCluster );
	if ( !pCluster )
		return false;

	tState.m_eState = pCluster->GetState();
	tState.m_sNode = pCluster->GetNodeName();
	return true;
}

static bool HasNotReadyNodes ( ReplicationClusterRefPtr_c pCluster )
{
	ClusterState_e eState = pCluster->GetState();
	if ( eState==ClusterState_e::DONOR || eState==ClusterState_e::JOINING )
		return true;

	const auto dStates = GetStatesFromRemotes ( *pCluster );
	return dStates.any_of ( []( auto & tState ) { return ( tState.m_eState==ClusterState_e::DONOR || tState.m_eState==ClusterState_e::JOINING ); });
}

// cluster ALTER statement that removes index from cluster but keep it at daemon
static bool ClusterAlterDrop ( const CSphString & sCluster, const VecTraits_T<CSphString> & dIndexes )
{
	RtAccum_t tAcc;
	tAcc.AddCommand ( ReplCmd_e::CLUSTER_ALTER_DROP, StrVec2Str ( dIndexes, "," ), sCluster );
	return HandleCmdReplicate ( tAcc );
}

// FIXME!!! refactor to use same code as SendClusterIndexes
static bool SendIndex ( const CSphString & sIndex, ReplicationClusterRefPtr_c pCluster )
{
	cServedIndexRefPtr_c pServed = GetServed ( sIndex );
	bool bMutable = ServedDesc_t::IsMutable ( pServed );
	bool bDist = ( !bMutable && g_pDistIndexes->Contains ( sIndex ) );

	if ( !bMutable && !bDist )
		return TlsMsg::Err ( "unknown or wrong table '%s'", sIndex.cstr() );

	int iAttempt = 0;
	// should wait a bit longer during join
	// FIXME!!! fetch progress delta time and continue to wait while there is a progress
	const int iRetryCount = ReplicationRetryCount() * 2;
	const int iRetryDelay = ReplicationRetryDelay() * 2;
	int64_t tmStart = sphMicroTimer();
	while ( true )
	{
		if ( HasNotReadyNodes ( pCluster ) )
		{
			iAttempt++;
			if ( iAttempt>=iRetryCount )
			{
				int64_t tmEnd = sphMicroTimer();
				return TlsMsg::Err ( "alter '%s' has some nodes not ready for %.3f seconds", pCluster->m_sName.cstr(), (float)(tmEnd - tmStart)/1000000.0f );
			}

			// FIXME!!! send index only into new node in case of next try happens
			sphLogDebugRpl ( "alter '%s' has some nodes not ready, will wait for %d seconds before retry %d", pCluster->m_sName.cstr(), iRetryDelay / 1000, iAttempt );
			Threads::Coro::SleepMsec ( iRetryDelay );
			continue;
		}

		auto dNodes = pCluster->FilterViewNodesByProto ( Proto_e::SPHINX, false );
		sphLogDebugRpl ( "alter '%s' SST index '%s' to nodes %d: '%s'", pCluster->m_sName.cstr(), sIndex.cstr(), dNodes.GetLength(), StrVec2Str ( dNodes ).cstr() );

		// ok for just created cluster (wo nodes) to add existed index
		if ( !dNodes.IsEmpty() )
		{
			VecAgentDesc_t dDesc = GetDescAPINodes ( dNodes, Resolve_e::SLOW );
			if ( TlsMsg::HasErr() )
				return false;

			sphLogDebugRpl ( "alter '%s' SST index '%s' to resolved nodes %d", pCluster->m_sName.cstr(), sIndex.cstr(), dDesc.GetLength() );

			if ( dDesc.GetLength() )
			{
				bool bReplicated = ( bMutable ? ReplicateIndexToNodes ( pCluster->m_sName, sIndex, dDesc, pServed ) : ReplicateDistIndexToNodes ( pCluster->m_sName, sIndex, dDesc ) );
				if ( !bReplicated )
				{
					if ( TlsMsg::HasErr() )
						sphWarning ( "%s", TlsMsg::szError() );
					return false;
				}
			}
		}

		// nodes list might change during alter at the other node
		auto dNewNodes = pCluster->FilterViewNodesByProto ( Proto_e::SPHINX, false );

		// passed fine no join during the alter and the cluster remains same
		if ( !HasNotReadyNodes ( pCluster ) && IsSameVector ( dNodes, dNewNodes ) )
			break;

		sphLogDebugRpl ( "nodes not ready during alter '%s', wait for %d sec before retry %d", pCluster->m_sName.cstr(), iRetryDelay / 1000, iAttempt );
		Threads::Coro::SleepMsec ( iRetryDelay );
		// no need to increase attempt count here as it will be checked on next try
	}

	return true;
}


struct LoadedIndexesClusterCleanup_t
{
	LoadedIndexesClusterCleanup_t ( bool & bAdded, const VecTraits_T<CSphString> & dIndexes, ReplicationClusterRefPtr_c pCluster )
		: m_bAdded ( bAdded )
		, m_dIndexes ( dIndexes )
		, m_pCluster ( pCluster )
	{}
	~LoadedIndexesClusterCleanup_t()
	{
		if ( !m_bAdded )
		{
			m_pCluster->WithWlockedIndexes ( [this] ( auto & hIndexes, auto & hIndexesLoaded )
			{
					for ( const CSphString & sIndex : m_dIndexes )
						hIndexesLoaded.Delete ( sIndex );
			});
		}
	}

	bool & m_bAdded;
	const VecTraits_T<CSphString> & m_dIndexes;
	ReplicationClusterRefPtr_c m_pCluster;
};

// cluster ALTER statement that adds index
static bool ClusterAlterAdd ( const CSphString & sCluster, const VecTraits_T<CSphString> & dIndexes )
	EXCLUDES ( g_tClustersLock )
{
	auto pCluster = ClusterByName ( sCluster );
	if ( !pCluster )
		return false;

	if ( !pCluster->IsHealthy() )
		return false;

	for ( const CSphString & sIndex : dIndexes )
	{
		if ( !SendIndex ( sIndex, pCluster ) )
			return false;
	}

	bool bAdded = false;
	pCluster->WithWlockedIndexes ( [&dIndexes] ( auto & hIndexes, auto & hIndexesLoaded )
	{
		for ( const CSphString & sIndex : dIndexes )
			hIndexesLoaded.Add ( sIndex );
	});
	LoadedIndexesClusterCleanup_t tCleanup ( bAdded, dIndexes, pCluster );

	sphLogDebugRpl ( "alter '%s' adding index '%s'", pCluster->m_sName.cstr(), StrVec2Str ( dIndexes, "," ).cstr() );
	RtAccum_t tAcc;
	ReplicationCommand_t * pAddCmd = tAcc.AddCommand ( ReplCmd_e::CLUSTER_ALTER_ADD, StrVec2Str ( dIndexes, "," ), sCluster );
	pAddCmd->m_bCheckIndex = false;
	
	bAdded = HandleCmdReplicate ( tAcc );
	sphLogDebugRpl ( "alter '%s' %s index '%s'", pCluster->m_sName.cstr(), ( bAdded ? "added" : "failed to add" ), StrVec2Str ( dIndexes, "," ).cstr() );
	return bAdded;
}

static bool ClusterAddCheckDistLocals ( const StrVec_t & dLocals, const CSphString & sCluster, const CSphString & sIndex, CSphString & sError )
{
	StringBuilder_c sMissed ( "," );
	StringBuilder_c sNonCluster ( "," );
	StringBuilder_c sWrongCluster ( "," );

	for ( const CSphString & sLocal : dLocals )
	{
		cServedIndexRefPtr_c pServed = GetServed ( sLocal );
		if ( !ServedDesc_t::IsMutable ( pServed ) )
			sMissed += sLocal.cstr();
		else if ( pServed->m_sCluster.IsEmpty() )
			sNonCluster += sLocal.cstr();
		else if ( pServed->m_sCluster!=sCluster )
			sWrongCluster += sLocal.cstr();
	}

	if ( sMissed.IsEmpty() && sNonCluster.IsEmpty() && sWrongCluster.IsEmpty() )
		return true;

	StringBuilder_c sMsg;
	sMsg.Appendf ( "can not add distributed table '%s' into cluster '%s';", sIndex.cstr(), sCluster.cstr() );
	sMsg.StartBlock ( "; " );
	if ( !sMissed.IsEmpty() )
		sMsg.Appendf ( "has unknown local tables: %s", sMissed.cstr() );
	if ( !sNonCluster.IsEmpty() )
		sMsg.Appendf ( "has tables not in the cluster: %s", sNonCluster.cstr() );
	if ( !sWrongCluster.IsEmpty() )
		sMsg.Appendf ( "has tables in the other cluster: %s", sNonCluster.cstr() );
	sMsg.FinishBlock();

	sError = sMsg.cstr();
	return false;
}

// cluster ALTER statement
bool ClusterAlter ( const CSphString & sCluster, const CSphString & sIndexes, bool bAdd, CSphString & sError )
{
	StrVec_t dIndexes = SplitIndexes ( sIndexes );
	dIndexes.Uniq();

	Threads::ScopedCoroMutex_t tClusterLock { g_tClusterOpsLock };
	{
		for ( const CSphString & sIndex : dIndexes )
		{
			cServedIndexRefPtr_c pServed = GetServed ( sIndex );
			bool bMutable = ServedDesc_t::IsMutable ( pServed );
			cDistributedIndexRefPtr_t pDist ( !bMutable ? GetDistr ( sIndex ) : nullptr );
			if ( !bMutable && !pDist )
			{
				sError.SetSprintf ( "unknown or wrong type of table '%s'", sIndex.cstr() );
				return false;
			}

			const CSphString & sIndexCluster = ( bMutable ? pServed->m_sCluster : pDist->m_sCluster );
			if ( bAdd )
			{
				if ( !sIndexCluster.IsEmpty() )
				{
					sError.SetSprintf ( "table '%s' is already part of cluster '%s'", sIndex.cstr(), sIndexCluster.cstr() );
					return false;
				}
				// all local indexes should be part of the cluster too
				if ( pDist && !ClusterAddCheckDistLocals ( pDist->m_dLocal, sCluster, sIndex, sError ) )
					return false;
			} else
			{
				if ( sIndexCluster.IsEmpty() )
				{
					sError.SetSprintf ( "table '%s' is not in cluster '%s'", sIndex.cstr(), sCluster.cstr() );
					return false;
				}
			}
		}
	}

	if ( !g_bReplicationStarted )
	{
		sError.SetSprintf ( "can not ALTER cluster '%s': %s", sCluster.cstr(), g_sReplicationStartError.cstr() );
		return false;
	}

	bool bOk = false;
	if ( bAdd )
		bOk = ClusterAlterAdd ( sCluster, dIndexes );
	else
		bOk = ClusterAlterDrop ( sCluster, dIndexes );

	TlsMsg::MoveError ( sError );
	bOk &= SaveConfigInt ( sError );

	return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// SST
/////////////////////////////////////////////////////////////////////////////

bool IsSameVector ( StrVec_t & dSrc, StrVec_t & dDst )
{
	if ( dSrc.GetLength()!=dDst.GetLength() )
		return false;

	dSrc.Sort();
	dDst.Sort();

	ARRAY_FOREACH ( i, dSrc )
	{
		if ( dSrc[i]!=dDst[i] )
			return false;
	}

	return true;
}

bool AddLoadedIndexIntoCluster ( const CSphString & sCluster, const CSphString & sIndex )
{
	auto pCluster = ClusterByName ( sCluster );
	if ( !pCluster )
		return false;

	pCluster->WithWlockedIndexes ( [&sIndex] ( auto & hIndexes, auto & hIndexesLoaded ) { hIndexesLoaded.Add ( sIndex ); });
	return true;
}

// send local indexes to remote nodes via API
bool SendClusterIndexes ( const ReplicationCluster_t * pCluster, const CSphString & sNode, bool bBypass, const Wsrep::GlobalTid_t & tStateID )
{
	auto dDesc = GetDescAPINodes ( ParseNodesFromString ( sNode ), Resolve_e::SLOW );
	if ( dDesc.IsEmpty() )
	{
		if ( TlsMsg::HasErr() )
			TlsMsg::Err ( "%s invalid node, error: %s", sNode.cstr(), TlsMsg::MoveToString().cstr() );
		else
			TlsMsg::Err ( "%s invalid node", sNode.cstr() );
		return false;
	}

	auto dIndexes = pCluster->GetIndexes();

	ClusterSyncedRequest_t tSyncedRequest;
	tSyncedRequest.m_sCluster = pCluster->m_sName;
	tSyncedRequest.m_tGtid = tStateID;
	tSyncedRequest.m_dIndexes = dIndexes;

	if ( bBypass )
		return SendClusterSynced ( dDesc, tSyncedRequest );

	bool bSentOk = true;
	while ( true )
	{
		sphLogDebugRpl ( "sending cluster '%s' indexes %d '%s'...'%s'", pCluster->m_sName.cstr(), dIndexes.GetLength(), ( dIndexes.GetLength() ? dIndexes.First().cstr() : "" ), ( dIndexes.GetLength() ? dIndexes.Last().cstr() : "" ) );

		for ( const CSphString & sIndex : dIndexes )
		{
			cServedIndexRefPtr_c pServed = GetServed ( sIndex );
			bool bMutable = ServedDesc_t::IsMutable ( pServed );
			cDistributedIndexRefPtr_t pDist ( !bMutable ? GetDistr ( sIndex ) : nullptr );
			if ( !bMutable && !pDist )
			{
				bSentOk = false;
				sphWarning ( "unknown or wrong table '%s'", sIndex.cstr() );
				continue;
			}

			bool bReplicated = ( bMutable ? ReplicateIndexToNodes ( pCluster->m_sName, sIndex, dDesc, pServed ) : ReplicateDistIndexToNodes ( pCluster->m_sName, sIndex, dDesc ) );
			if ( !bReplicated )
			{
				sphWarning ( "%s", TlsMsg::szError() );
				bSentOk = false;
				break;
			}

			if ( TlsMsg::HasErr() )
				sphWarning ( "%s", TlsMsg::szError() );
		}

		// index list could have changed due to alter at the other node
		auto dNewIndexes = pCluster->GetIndexes();
		if ( dNewIndexes.GetLength() )
			sphLogDebugRpl ( "sent cluster '%s' indexes %d '%s'...'%s'", pCluster->m_sName.cstr(), dNewIndexes.GetLength(), dNewIndexes.First().cstr(), dNewIndexes.Last().cstr() );

		if ( IsSameVector ( dIndexes, dNewIndexes ) )
			break;

		sphLogDebugRpl ( "index list changed during donate '%s' to '%s'", pCluster->m_sName.cstr(), sNode.cstr() );
		// FIXME!!! send only new indexes but loads all cluster indexes
		dIndexes = dNewIndexes;
	}

	tSyncedRequest.m_dIndexes = dIndexes;
	tSyncedRequest.m_bSendFilesSuccess = bSentOk;
	tSyncedRequest.m_sMsg = TlsMsg::MoveToString();
	bool bSyncOk = SendClusterSynced ( dDesc, tSyncedRequest );
	return bSentOk && bSyncOk;
}

// callback at remote node for CLUSTER_SYNCED to pick up received indexes then call Galera sst_received
bool ClusterSynced ( const ClusterSyncedRequest_t & tCmd ) EXCLUDES ( g_tClustersLock )
{
	sphLogDebugRpl ( "join sync %s, UID %s, sent %s, tables %d, %s", tCmd.m_sCluster.cstr(), Wsrep::Gtid2Str ( tCmd.m_tGtid ).cstr(), ( tCmd.m_bSendFilesSuccess ? "ok" : "failed" ), tCmd.m_dIndexes.GetLength(), tCmd.m_sMsg.scstr() );

	if ( !tCmd.m_bSendFilesSuccess )
	{
		if ( tCmd.m_sMsg.IsEmpty() )
			TlsMsg::Err ( "donor failed to send files" );
		else
			TlsMsg::Err ( tCmd.m_sMsg );
	}

	Wsrep::GlobalTid_t tGtid {}; // by default is 'UNDEFINED'
	bool bValid = ( tCmd.m_bSendFilesSuccess && tCmd.m_tGtid != tGtid );

	if ( bValid )
		bValid &= ReplicatedIndexes ( tCmd.m_dIndexes, tCmd.m_sCluster );

	auto pCluster = ClusterByName ( tCmd.m_sCluster );
	if ( !pCluster )
		return false;

	tGtid = tCmd.m_tGtid;
	int iRes = 0;
	if ( !bValid )
	{
		tGtid.m_iSeqNo = Wsrep::WRONG_SEQNO;
		iRes = -ECANCELED;
	}

	pCluster->m_pProvider->SstReceived ( tGtid, iRes );
	return bValid;
}

// validate that SphinxQL statement could be run for this cluster:index 
bool ValidateClusterStatement ( const CSphString & sIndexName, const ServedDesc_t & tDesc, const CSphString & sStmtCluster, bool bHTTP )
{
	if ( tDesc.m_sCluster==sStmtCluster )
		return true;

	if ( tDesc.m_sCluster.IsEmpty() )
		return TlsMsg::Err ( "table '%s' is not in any cluster, use just '%s'", sIndexName.cstr(), sIndexName.cstr() );

	if ( !bHTTP )
		return TlsMsg::Err ( "table '%s' is a part of cluster '%s', use '%s:%s'", sIndexName.cstr(), tDesc.m_sCluster.cstr(), tDesc.m_sCluster.cstr(), sIndexName.cstr() );

	return TlsMsg::Err( R"(table '%s' is a part of cluster '%s', use "cluster":"%s" and "table":"%s" properties)", sIndexName.cstr(), tDesc.m_sCluster.cstr(), tDesc.m_sCluster.cstr(), sIndexName.cstr() );
}

std::optional<CSphString> IsPartOfCluster ( const ServedDesc_t * pDesc )
{
	assert ( pDesc );
	if ( !pDesc->m_sCluster.IsEmpty() )
		return pDesc->m_sCluster;
	return {};
}

// return set of nodes for given cluster (both, clusternodes and viewnodes)
StrVec_t ClusterGetAllNodes ( const CSphString & sCluster ) EXCLUDES ( g_tClustersLock )
{
	TlsMsg::Err();
	auto pCluster = ClusterByName ( sCluster );
	if ( !pCluster )
		return {};

	// send back view nodes of cluster - as list of actual nodes
	sph::StringSet hNodes;
	for_each ( pCluster->GetViewNodes(), [&hNodes] ( const auto& sNode ) { hNodes.Add ( sNode ); } );
	for_each ( pCluster->m_dClusterNodes, [&hNodes] ( const auto& sNode ) { hNodes.Add ( sNode ); } );
	StrVec_t dNodes;
	for_each ( hNodes, [&dNodes] ( const auto& tNode ) { dNodes.Add ( tNode.first ); } );
	return dNodes;
//	return FilterNodesByProto ( dNodes, Proto_e::REPLICATION );
}

// cluster ALTER statement that updates nodes option from view nodes at all nodes at cluster
bool ClusterAlterUpdate ( const CSphString & sCluster, const CSphString & sUpdate, CSphString & sError )
{
	TlsMsg::ResetErr();
	auto bOk = DoClusterAlterUpdate ( sCluster, sUpdate, NODES_E::VIEW );
	TlsMsg::MoveError ( sError );
	return bOk;
}

bool DoClusterAlterUpdate ( const CSphString & sCluster, const CSphString & sUpdate, NODES_E eNodes ) EXCLUDES ( g_tClustersLock )
{
	TlsMsg::ResetErr();
	if ( sUpdate!="nodes" )
		return TlsMsg::Err ( "unhandled statement, only UPDATE nodes are supported, got '%s'", sUpdate.cstr() );

	auto pCluster = ClusterByName ( sCluster );
	if ( !pCluster || !pCluster->IsHealthy() )
		return false;

	// need to update all VIEW nodes - not cluster set nodes
	StrVec_t dNodes;

	// local nodes update
	bool bOk = ClusterUpdateNodes ( sCluster, eNodes, &dNodes );

	if ( dNodes.IsEmpty() )
		return false;

	// remote nodes update after locals updated
	if ( !SendClusterUpdateNodes ( sCluster, eNodes, dNodes ) )
	{
		sphWarning ( "cluster %s nodes update error %s", sCluster.cstr(), TlsMsg::szError() );
		TlsMsg::ResetErr();
	}

	return bOk;
}

// callback at remote node for CLUSTER_UPDATE_NODES to update nodes list at cluster from actual nodes list
bool ClusterUpdateNodes ( const CSphString & sCluster, NODES_E eNodes, StrVec_t * pNodes ) EXCLUDES ( g_tClustersLock )
{
	auto pCluster = ClusterByName ( sCluster );
	if ( !pCluster || !pCluster->IsHealthy() )
	{
		// node in the joining state should skip the command
		if ( pCluster && pCluster->GetState()==ClusterState_e::JOINING )
		{
			TlsMsg::ResetErr();
			return true;
		}
		return false;
	}

	auto fnNodesHash = [](const StrVec_t dNodes) {
		uint64_t uRes = SPH_FNV64_SEED;
		dNodes.for_each ( [&uRes] ( auto& sNode ) { uRes = sphFNV64cont ( sNode.cstr(), uRes ); } );
		return uRes;
	};

	TlsMsg::ResetErr();

	StrVec_t dNodes = pCluster->GetViewNodes();
	if ( eNodes==NODES_E::BOTH )
		dNodes.Append ( pCluster->m_dClusterNodes );

	uint64_t uWasNodes = fnNodesHash ( pCluster->m_dClusterNodes );
	pCluster->m_dClusterNodes = FilterNodesByProto ( dNodes, Proto_e::SPHINX );
	bool bSaveConf = uWasNodes != fnNodesHash ( pCluster->m_dClusterNodes );

	if ( TlsMsg::HasErr() )
		return TlsMsg::Err ( "cluster '%s', invalid nodes, error: %s", sCluster.cstr(), TlsMsg::szError() );

	if ( pNodes )
		*pNodes = pCluster->m_dClusterNodes;
	else
		bSaveConf = true;

	TLS_MSG_STRING ( sError );
	auto bOk = true;
	if ( bSaveConf )
		bOk = SaveConfigInt ( sError );

	return bOk;
}
