//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "cluster_get_nodes.h"
#include "cluster_commands.h"

#include "api_command_cluster.h"
#include "nodes.h"
#include "searchdreplication.h"
#include "serialize.h"

// API command to remote node to get nodes it sees
using ClusterGetNodes_c = ClusterCommand_T<E_CLUSTER::GET_NODES, ClusterRequest_t, StrVec_t>;

StrVec_t RemoteClusterGetNodes ( VectorAgentConn_t & dAgents )
{
	sph::StringSet hAllNodes;
	// submit initial jobs
	CSphRefcountedPtr<RemoteAgentsObserver_i> tReporter ( GetObserver ());
	ClusterGetNodes_c tReq;
	ScheduleDistrJobs ( dAgents, &tReq, &tReq, tReporter, ReplicationRetryCount(), ReplicationRetryDelay() );

	bool bDone = false;
	while (!bDone)
	{
		// don't forget to check incoming replies after send was over
		bDone = tReporter->IsDone ();
		// wait one or more remote queries to complete
		if ( !bDone )
			tReporter->WaitChanges ();

		for ( AgentConn_t * pAgent : dAgents )
		{
			if ( !pAgent->m_bSuccess )
				continue;

			pAgent->m_bSuccess = false; // re-use !m_bSuccess as 'visited' flag

			// need to wait all replies in case any node get nodes list and merge these lists
			// also need a way for distributed loop to finish as it can not break early
			StrVec_t & dNodes = ClusterGetNodes_c::GetRes ( *pAgent );
			for_each ( dNodes, [&hAllNodes] ( const CSphString& sNode ) { hAllNodes.Add ( sNode ); } );
		}
	}

	StrVec_t dRes;
	for_each ( hAllNodes, [&dRes] ( const auto& sNode ) { dRes.Add ( sNode.first ); } );
	return dRes;
}

// command to all remote nodes at cluster to get actual nodes list
StrVec_t QueryNodeListFromRemotes ( const VecTraits_T<CSphString>& dClusterNodes, const CSphString& sCluster )
{
	StrVec_t dNodes;
	TlsMsg::ResetErr();
	VecAgentDesc_t dDesc = GetDescAPINodes ( dClusterNodes, Resolve_e::QUICK );
	if ( dDesc.IsEmpty() )
	{
		if ( TlsMsg::HasErr() )
			TlsMsg::Err ( "%s invalid node, error: %s", StrVec2Str ( dClusterNodes ).cstr(), TlsMsg::szError() );
		else
			TlsMsg::Err ( "%s invalid node", StrVec2Str ( dClusterNodes ).cstr() );

		return dNodes;
	}

	ClusterRequest_t dRequest;
	dRequest.m_sCluster = sCluster;

	VecRefPtrs_t<AgentConn_t*> dAgents = ClusterGetNodes_c::MakeAgents ( dDesc, ReplicationTimeoutAnyNode(), dRequest );
	dNodes = RemoteClusterGetNodes ( dAgents );

	ScopedComma_c tColon ( TlsMsg::Err(), ";" );
	for ( const AgentConn_t* pAgent : dAgents )
	{
		if ( !pAgent->m_sFailure.IsEmpty() )
		{
			StringBuilder_c sOneError;
			sOneError.Sprintf ( "'%s:%d': %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, pAgent->m_sFailure.cstr() );
			sphWarning ( "%s", sOneError.cstr() );
			TlsMsg::Err() << sOneError;
		}
	}

	return dNodes;
}


void ReceiveClusterGetNodes ( ISphOutputBuffer& tOut, InputBuffer_c& tBuf, CSphString& sCluster )
{
	ClusterRequest_t tCmd;
	ClusterGetNodes_c::ParseRequest ( tBuf, tCmd );
	sCluster = tCmd.m_sCluster;
	auto dNodes = ClusterGetAllNodes ( tCmd.m_sCluster );
	if ( !TlsMsg::HasErr ())
		ClusterGetNodes_c::BuildReply ( tOut, dNodes );
}

StrVec_t GetNodeListFromRemotes ( const ClusterDesc_t& tDesc )
{
	auto dNodes = QueryNodeListFromRemotes ( tDesc.m_dClusterNodes, tDesc.m_sName );
	if ( dNodes.IsEmpty() )
		TlsMsg::Err ( "cluster '%s', no nodes available(%s), error: %s", tDesc.m_sName.cstr(), StrVec2Str( tDesc.m_dClusterNodes ).cstr(),  TlsMsg::szError() );
	else
		TlsMsg::Err ( "cluster '%s', invalid nodes '%s'(%s), error: %s", tDesc.m_sName.cstr(), StrVec2Str ( dNodes ).cstr(), StrVec2Str ( tDesc.m_dClusterNodes ).cstr(), TlsMsg::szError() );
	return dNodes;
}

/////////////////////////////////////////////////////////////////////////////
// cluster get nodes state
/////////////////////////////////////////////////////////////////////////////

struct ClusterNodesStatesReply_t : public ClusterRequest_t
{
	ClusterNodesStatesVec_t m_dStates;
	RemoteNodeClusterState_t m_tState;
};

// API command to remote node to get node state
using ClusterNodeState_c = ClusterCommand_T<E_CLUSTER::GET_NODE_STATE, ClusterRequest_t, ClusterNodesStatesReply_t>;

void operator<< ( ISphOutputBuffer & tOut, const ClusterNodesStatesReply_t & tReq )
{
	tOut << (const ClusterRequest_t&)tReq;
	tOut.SendByte ( (BYTE)tReq.m_tState.m_eState );
	tOut.SendString ( tReq.m_tState.m_sNode.cstr() );
	tOut.SendString ( tReq.m_tState.m_sHash.cstr() );
}

void operator>> ( InputBuffer_c & tIn, ClusterNodesStatesReply_t & tReq )
{
	tIn >> (ClusterRequest_t&)tReq;

	RemoteNodeClusterState_t & tState = tReq.m_dStates.Add();
	tState.m_eState = (ClusterState_e)tIn.GetByte();
	tState.m_sNode = tIn.GetString();
	tState.m_sHash = tIn.GetString();
}

static bool SendClusterNodesStates ( const CSphString & sCluster, const VecTraits_T<CSphString> & dNodes, ClusterNodesStatesVec_t & dStates )
{
	ClusterNodeState_c::REQUEST_T tReq;
	tReq.m_sCluster = sCluster;

	auto dAgents = ClusterNodeState_c::MakeAgents ( GetDescAPINodes ( dNodes, Resolve_e::SLOW ), ReplicationTimeoutQuery(), tReq );
	// no nodes left seems a valid case
	if ( dAgents.IsEmpty() )
		return true;

	ClusterNodeState_c tReply;
	// FIXME!!! handle errors
	PerformRemoteTasksWrap ( dAgents, tReply, tReply, true );

	for ( const AgentConn_t * pAgent : dAgents )
	{
		if ( pAgent->m_bSuccess )
			dStates.Append ( ClusterNodeState_c::GetRes ( *pAgent ).m_dStates );
	}

	return true;
}

ClusterNodesStatesVec_t GetStatesFromRemotes ( const ClusterDesc_t & tDesc )
{
	ClusterNodesStatesVec_t dStates;
	SendClusterNodesStates ( tDesc.m_sName, tDesc.m_dClusterNodes, dStates );
	return dStates;
}

void ReceiveClusterGetState ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString & sCluster )
{
	ClusterNodesStatesReply_t tRequest;
	ClusterNodeState_c::ParseRequest ( tBuf, tRequest );
	
	ClusterGetState ( tRequest.m_sCluster, tRequest.m_tState );
	ClusterNodeState_c::BuildReply ( tOut, tRequest );
}

static const WORD g_uClusterNodeVer = 1;

template<bool WITH_SERVER_ID>
struct ClusterNodeVerReply_T
{
	WORD m_uVerCommandCluster = 0;
	WORD m_uVerCommandReplicate = 0;
	int m_iServerId = 0;

	void Save ( ISphOutputBuffer & tOut ) const
	{
		tOut.SendWord ( g_uClusterNodeVer );
		tOut.SendWord ( m_uVerCommandCluster );
		tOut.SendWord ( m_uVerCommandReplicate );
		if_const ( WITH_SERVER_ID )
			tOut.SendInt ( m_iServerId );
	}

	void Load ( InputBuffer_c & tIn )
	{
		WORD uVer = tIn.GetWord();
		if ( uVer>=g_uClusterNodeVer )
		{
			m_uVerCommandCluster = tIn.GetWord();
			m_uVerCommandReplicate = tIn.GetWord();
			if_const ( WITH_SERVER_ID )
				m_iServerId = tIn.GetInt();

		}
	}
};

template<bool WITH_SERVER_ID>
void operator<< ( ISphOutputBuffer & tOut, const ClusterNodeVerReply_T<WITH_SERVER_ID> & tReq )
{
	tReq.Save ( tOut );
}

template<bool WITH_SERVER_ID>
void operator>> ( InputBuffer_c & tIn, ClusterNodeVerReply_T<WITH_SERVER_ID> & tReq )
{
	tReq.Load ( tIn );
}

using ClusterNodeVerReply_t = ClusterNodeVerReply_T<false>;
using ClusterNodeVerIdReply_t = ClusterNodeVerReply_T<true>;

// API command to remote node to get node versions
using ClusterNodeVer_c = ClusterCommand_T<E_CLUSTER::GET_NODE_VER, ClusterNodeVerReply_t, ClusterNodeVerReply_t>;

// API command to remote node to get node versions and id
using ClusterNodeVerId_c = ClusterCommand_T<E_CLUSTER::GET_NODE_VER_ID, ClusterNodeVerIdReply_t, ClusterNodeVerIdReply_t>;

void ReceiveClusterGetVer ( bool bServerid, ISphOutputBuffer & tOut )
{
	if ( !bServerid )
	{
		ClusterNodeVerReply_t tReply;
		tReply.m_uVerCommandCluster = VER_COMMAND_CLUSTER;
		tReply.m_uVerCommandReplicate = GetVerCommandReplicate();
		ClusterNodeVer_c::BuildReply ( tOut, tReply );
	} else
	{
		ClusterNodeVerIdReply_t tReply;
		tReply.m_uVerCommandCluster = VER_COMMAND_CLUSTER;
		tReply.m_uVerCommandReplicate = GetVerCommandReplicate();
		tReply.m_iServerId = GetUidShortServerId();
		ClusterNodeVerId_c::BuildReply ( tOut, tReply );
	}
}

static bool CheckRemoteVersions ( const ClusterDesc_t & tDesc, const AgentConn_t * pAgent );
static bool CheckRemoteVersionsId ( const ClusterDesc_t & tDesc, const AgentConn_t * pAgent );

bool CheckRemotesVersions ( const ClusterDesc_t & tDesc, bool bWithServerId )
{
	ClusterNodeVer_c::REQUEST_T tReqVer;
	ClusterNodeVerId_c::REQUEST_T tReqId;

	VecRefPtrs_t<AgentConn_t*> dAgents;
	if ( !bWithServerId )
	{
		dAgents = ClusterNodeVer_c::MakeAgents ( GetDescAPINodes ( tDesc.m_dClusterNodes, Resolve_e::QUICK ), ReplicationTimeoutAnyNode(), tReqVer );
	} else
	{
		dAgents = ClusterNodeVerId_c::MakeAgents ( GetDescAPINodes ( tDesc.m_dClusterNodes, Resolve_e::QUICK ), ReplicationTimeoutAnyNode(), tReqId );
	}
	// no nodes left seems a valid case
	if ( dAgents.IsEmpty() )
		return true;

	ClusterNodeVer_c tReplyVer;
	ClusterNodeVerId_c tReplyId;
	if ( !bWithServerId )
		PerformRemoteTasksWrap ( dAgents, tReplyVer, tReplyVer, false );
	else
		PerformRemoteTasksWrap ( dAgents, tReplyId, tReplyId, false );

	for ( const AgentConn_t * pAgent : dAgents )
	{
		// failure if:
		// - fetched but versions are wrong either VER_COMMAND_CLUSTER or VER_COMMAND_REPLICATE
		// - get remote node error reply with the wrong version message
		// - server_id on this node is the same as on any other node
		if ( pAgent->m_bSuccess )
		{
			if ( ( !bWithServerId && !CheckRemoteVersions ( tDesc, pAgent ) ) || ( bWithServerId && !CheckRemoteVersionsId ( tDesc, pAgent ) ) )
				return false;

		} else if ( pAgent->m_sFailure.Begins ( "remote error: client version is" ) )
		{
			TlsMsg::ResetErr();
			TlsMsg::Err ( pAgent->m_sFailure );
			return false;
		}
	}

	return true;
}

template<typename T>
bool CheckNodeVersions ( const ClusterDesc_t & tDesc, const T & tVer )
{
	if ( tVer.m_uVerCommandCluster!=VER_COMMAND_CLUSTER || tVer.m_uVerCommandReplicate!=GetVerCommandReplicate() )
	{
		TlsMsg::Err ( "versions mismatch, node removed from the cluster '%s': %d(%d), replication: %d(%d)", tDesc.m_sName.cstr(), (int)tVer.m_uVerCommandCluster, (int)VER_COMMAND_CLUSTER, (int)tVer.m_uVerCommandReplicate, (int)GetVerCommandReplicate() );
		return false;
	} else
	{
		return true;
	}
}

bool CheckRemoteVersions ( const ClusterDesc_t & tDesc, const AgentConn_t * pAgent )
{
	ClusterNodeVerReply_t tVer = ClusterNodeVer_c::GetRes ( *pAgent );
	return CheckNodeVersions ( tDesc, tVer );
}

bool CheckRemoteVersionsId ( const ClusterDesc_t & tDesc, const AgentConn_t * pAgent )
{
	ClusterNodeVerIdReply_t tVer = ClusterNodeVerId_c::GetRes ( *pAgent );
	if ( !CheckNodeVersions ( tDesc, tVer ) )
		return false;

	if ( tVer.m_iServerId==GetUidShortServerId() )
	{
		TlsMsg::Err ( "server_id conflict, duplicate server_id:%d detected in the cluster '%s'. Please set a unique server_id at the config.", tVer.m_iServerId, tDesc.m_sName.cstr() );
		return false;
	} else
	{
		return true;
	}
}
