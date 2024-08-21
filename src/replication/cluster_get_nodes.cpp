//
// Copyright (c) 2019-2024, Manticore Software LTD (https://manticoresearch.com)
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

struct ClusterNodeVerReply_t
{
	WORD m_uVerCommandCluster = 0;
	WORD m_uVerCommandReplicate = 0;
};

static const WORD g_uClusterNodeVer = 1;

// API command to remote node to get node versions
using ClusterNodeVer_c = ClusterCommand_T<E_CLUSTER::GET_NODE_VER, ClusterNodeVerReply_t, ClusterNodeVerReply_t>;

void operator<< ( ISphOutputBuffer & tOut, const ClusterNodeVerReply_t & tReq )
{
	tOut.SendWord ( g_uClusterNodeVer );
	tOut.SendWord ( tReq.m_uVerCommandCluster );
	tOut.SendWord ( tReq.m_uVerCommandReplicate );
}

void operator>> ( InputBuffer_c & tIn, ClusterNodeVerReply_t & tReq )
{
	WORD uVer = tIn.GetWord();
	if ( uVer>=g_uClusterNodeVer )
	{
		tReq.m_uVerCommandCluster = tIn.GetWord();
		tReq.m_uVerCommandReplicate = tIn.GetWord();
	}
}

void ReceiveClusterGetVer ( ISphOutputBuffer & tOut )
{
	ClusterNodeVerReply_t tReply;
	tReply.m_uVerCommandCluster = VER_COMMAND_CLUSTER;
	tReply.m_uVerCommandReplicate = GetVerCommandReplicate();
	ClusterNodeVer_c::BuildReply ( tOut, tReply );
}

bool CheckRemotesVersions ( const ClusterDesc_t & tDesc )
{
	ClusterNodeVer_c::REQUEST_T tReq;

	auto dAgents = ClusterNodeVer_c::MakeAgents ( GetDescAPINodes ( tDesc.m_dClusterNodes, Resolve_e::QUICK ), ReplicationTimeoutAnyNode(), tReq );
	// no nodes left seems a valid case
	if ( dAgents.IsEmpty() )
		return true;

	ClusterNodeVer_c tReply;
	PerformRemoteTasksWrap ( dAgents, tReply, tReply, false );

	for ( const AgentConn_t * pAgent : dAgents )
	{
		// failure if:
		// - fetched but versions are wrong either VER_COMMAND_CLUSTER or VER_COMMAND_REPLICATE
		// - get remote node error reply with the wrong version message
		if ( pAgent->m_bSuccess )
		{
			ClusterNodeVerReply_t tVer = ClusterNodeVer_c::GetRes ( *pAgent );
			if ( tVer.m_uVerCommandCluster!=VER_COMMAND_CLUSTER || tVer.m_uVerCommandReplicate!=GetVerCommandReplicate() )
			{
				TlsMsg::Err ( "versions mismatch, node removed from the cluster '%s': %d(%d), replication: %d(%d)", tDesc.m_sName.cstr(), (int)tVer.m_uVerCommandCluster, (int)VER_COMMAND_CLUSTER, (int)tVer.m_uVerCommandReplicate, (int)GetVerCommandReplicate() );
				return false;
			}
		} else if ( pAgent->m_sFailure.Begins ( "remote error: client version is" ) )
		{
			TlsMsg::ResetErr();
			TlsMsg::Err ( pAgent->m_sFailure );
			return false;
		}
	}

	return true;
}