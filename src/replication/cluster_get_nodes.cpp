//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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
#include "auth/auth.h"
#include "auth/auth_common.h"

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
StrVec_t QueryNodeListFromRemotes ( const VecTraits_T<CSphString>& dClusterNodes, const CSphString& sCluster, const CSphString & sUser )
{
	StrVec_t dNodes;
	TlsMsg::ResetErr();
	VecAgentDesc_t dDesc = GetDescAPINodes ( dClusterNodes, Resolve_e::QUICK );
	if ( dDesc.IsEmpty() )
	{
		if ( TlsMsg::HasErr() )
			TlsMsg::Err ( "%s invalid node, error: %s", Vec2Str ( dClusterNodes ).cstr(), TlsMsg::szError() );
		else
			TlsMsg::Err ( "%s invalid node", Vec2Str ( dClusterNodes ).cstr() );

		return dNodes;
	}

	ClusterRequest_t dRequest;
	dRequest.m_sCluster = sCluster;

	VecRefPtrs_t<AgentConn_t*> dAgents = ClusterGetNodes_c::MakeAgents ( dDesc, sUser, ReplicationTimeoutAnyNode(), dRequest );
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

StrVec_t GetNodeListFromRemotes ( const ClusterDesc_t & tDesc )
{
	auto dNodes = QueryNodeListFromRemotes ( tDesc.m_dClusterNodes, tDesc.m_sName, tDesc.m_sUser );
	if ( dNodes.IsEmpty() )
		TlsMsg::Err ( "cluster '%s', no nodes available(%s), error: %s", tDesc.m_sName.cstr(), Vec2Str( tDesc.m_dClusterNodes ).cstr(),  TlsMsg::szError() );
	else
		TlsMsg::Err ( "cluster '%s', invalid nodes '%s'(%s), error: %s", tDesc.m_sName.cstr(), Vec2Str ( dNodes ).cstr(), Vec2Str ( tDesc.m_dClusterNodes ).cstr(), TlsMsg::szError() );
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

static bool SendClusterNodesStates ( const CSphString & sCluster, const CSphString & sUser, const VecTraits_T<CSphString> & dNodes, ClusterNodesStatesVec_t & dStates )
{
	ClusterNodeState_c::REQUEST_T tReq;
	tReq.m_sCluster = sCluster;

	auto dAgents = ClusterNodeState_c::MakeAgents ( GetDescAPINodes ( dNodes, Resolve_e::SLOW ), sUser, ReplicationTimeoutQuery(), tReq );
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
	SendClusterNodesStates ( tDesc.m_sName, tDesc.m_sUser, tDesc.m_dClusterNodes, dStates );
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
	int m_iServerId = 0;
	int64_t m_iClusterEpoch = 0;
	CSphString m_sUser;

	void Save ( ISphOutputBuffer & tOut ) const
	{
		tOut.SendWord ( m_uVerCommandCluster );
		tOut.SendWord ( m_uVerCommandReplicate );
		tOut.SendInt ( m_iServerId );
		tOut.SendUint64 ( m_iClusterEpoch );
		tOut.SendString ( m_sUser.cstr() );		
	}

	void Load ( InputBuffer_c & tIn )
	{
		m_uVerCommandCluster = tIn.GetWord();
		if ( m_uVerCommandCluster!=VER_COMMAND_CLUSTER )
			return;

		m_uVerCommandReplicate = tIn.GetWord();
		m_iServerId = tIn.GetInt();
		m_iClusterEpoch = (int64_t)tIn.GetUint64();
		m_sUser = tIn.GetString();
	}
};

void operator<< ( ISphOutputBuffer & tOut, const ClusterNodeVerReply_t & tReq )
{
	tReq.Save ( tOut );
}

void operator>> ( InputBuffer_c & tIn, ClusterNodeVerReply_t & tReq )
{
	tReq.Load ( tIn );
}

// API command to remote node to get node versions and id
using ClusterNodeVerId_c = ClusterCommand_T<E_CLUSTER::GET_NODE_VER_ID, ClusterRequest_t, ClusterNodeVerReply_t>;

void ReceiveClusterGetVer ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf )
{
	ClusterRequest_t tReq;
	ClusterNodeVerId_c::ParseRequest ( tBuf, tReq );
	if ( tReq.m_sCluster.IsEmpty() )
	{
		TlsMsg::Err ( "cluster is required for GET_NODE_VER_ID" );
		return;
	}
		
	ClusterNodeVerReply_t tReply;
	tReply.m_uVerCommandCluster = VER_COMMAND_CLUSTER;
	tReply.m_uVerCommandReplicate = GetVerCommandReplicate();
	tReply.m_iServerId = GetUidShortServerId();
	tReply.m_iClusterEpoch = ClusterGetRemoteEpoch ( tReq.m_sCluster );
	
	if ( !ClusterGetDonorMeta ( tReq.m_sCluster, tReply.m_sUser ) )
		return;
		
	ClusterNodeVerId_c::BuildReply ( tOut, tReply );
}

static bool CheckNodeVersions ( const ClusterDesc_t & tDesc, const ClusterNodeVerReply_t  & tVer, CSphString & sUser );

bool CheckRemotesVersions ( const ClusterDesc_t & tDesc, CSphString & sUser, int64_t * pEpoch )
{
	ClusterNodeVerId_c::REQUEST_T tReqId;
	tReqId.m_sCluster = tDesc.m_sName;

	auto dAgents = ClusterNodeVerId_c::MakeAgents ( GetDescAPINodes ( tDesc.m_dClusterNodes, Resolve_e::QUICK ), tDesc.m_sUser, ReplicationTimeoutAnyNode(), tReqId );
	// no nodes left seems a valid case
	if ( dAgents.IsEmpty() )
		return true;

	if ( pEpoch )
		*pEpoch = 0;

	ClusterNodeVerId_c tReply;
	PerformRemoteTasksWrap ( dAgents, tReply, tReply, false );

	int iSuccess = 0;
	CSphString sDonorUser;

	for ( const AgentConn_t * pAgent : dAgents )
	{
		if ( !pAgent->m_bSuccess )
			continue;

		ClusterNodeVerReply_t tVer = ClusterNodeVerId_c::GetRes ( *pAgent );

		// failure if:
		// - fetched but versions are wrong either VER_COMMAND_CLUSTER or VER_COMMAND_REPLICATE
		// - server_id on this node is the same as on any other node
		CSphString sGotUser;
		if ( !CheckNodeVersions ( tDesc, tVer, sGotUser ) )
			return false;

		if ( tVer.m_iServerId==GetUidShortServerId() )
		{
			TlsMsg::Err ( "server_id conflict, duplicate server_id:%d detected in the cluster '%s'. Please set a unique server_id at the config.", tVer.m_iServerId, tDesc.m_sName.cstr() );
			return false;
		}

		iSuccess++;
		if ( iSuccess==1 )
			sDonorUser = sGotUser;
		else if ( sDonorUser!=sGotUser )
		{
			TlsMsg::Err ( "inconsistent cluster user from remotes for '%s'", tDesc.m_sName.cstr() );
			return false;
		}

		if ( pEpoch )
			*pEpoch = tVer.m_iClusterEpoch;
	}

	if ( !iSuccess )
	{
		TlsMsg::ResetErr();
		return TlsMsg::Err ( "cluster '%s', failed to fetch donor user from any node", tDesc.m_sName.cstr() );
	}

	sUser = sDonorUser;

	return true;
}

bool CheckNodeVersions ( const ClusterDesc_t & tDesc, const ClusterNodeVerReply_t  & tVer, CSphString & sUser )
{
	if ( tVer.m_uVerCommandCluster!=VER_COMMAND_CLUSTER || tVer.m_uVerCommandReplicate!=GetVerCommandReplicate() )
	{
		TlsMsg::Err ( "versions mismatch, node removed from the cluster '%s': %d(%d), replication: %d(%d)", tDesc.m_sName.cstr(), (int)tVer.m_uVerCommandCluster, (int)VER_COMMAND_CLUSTER, (int)tVer.m_uVerCommandReplicate, (int)GetVerCommandReplicate() );
		return false;
	} else
	{
		sUser = tVer.m_sUser;
		return true;
	}
}

/////////////////////////////////////////////////////////////////////////////
// cluster get auth

struct ClusterAuthReply_t
{
	bool m_bWithAuth = false;
	uint64_t m_uAuthHash = 0;
	CSphString m_sAuth;

	void Save ( ISphOutputBuffer & tOut ) const
	{
		tOut.SendByte ( m_bWithAuth );
		tOut.SendUint64 ( m_uAuthHash );
		tOut.SendString ( m_sAuth.cstr() );
	}

	void Load ( InputBuffer_c & tIn )
	{
		m_bWithAuth = tIn.GetByte();
		m_uAuthHash = tIn.GetUint64();
		m_sAuth = tIn.GetString();
		int iLen = m_sAuth.Length();
		// json parser needs double trail zero
		if ( iLen )
		{
			char * sAuth = const_cast<char *>( m_sAuth.cstr() );
			sAuth[iLen] = '\0'; // safe gap
			sAuth[iLen+1] = '\0';
		}
	}
};

void operator<< ( ISphOutputBuffer & tOut, const ClusterAuthReply_t & tReq )
{
	tReq.Save ( tOut );
}

void operator>> ( InputBuffer_c & tIn, ClusterAuthReply_t & tReq )
{
	tReq.Load ( tIn );
}

// API command to remote node to get auth
using ClusterAuth_c = ClusterCommand_T<E_CLUSTER::GET_NODE_AUTH, ClusterAuthReply_t, ClusterAuthReply_t>;

void ReceiveClusterGetAuth ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf )
{
	ClusterAuthReply_t tReq;
	tReq.Load ( tBuf );

	ClusterAuthReply_t tReply;
	if ( tReq.m_bWithAuth && IsAuthEnabled() )
	{
		tReply.m_bWithAuth = true;
		const AuthUsersPtr_t tAuth = GetAuth();
		tReply.m_sAuth = WriteJson ( *tAuth.get() );
		tReply.m_uAuthHash = sphFNV64 ( tReply.m_sAuth.cstr(), tReply.m_sAuth.Length() );
		// no need to send back the same auth data as joiner already has
		if ( tReq.m_uAuthHash==tReply.m_uAuthHash )
			tReply.m_sAuth = "";
			
	}
	ClusterAuth_c::BuildReply ( tOut, tReply );
}

bool SetAuth ( const ClusterDesc_t & tDesc, const AgentConn_t * pAgent, const ClusterAuthReply_t & tReq, StringBuilder_c & sError )
{
	ClusterAuthReply_t tReply = ClusterAuth_c::GetRes ( *pAgent );

	if ( tReply.m_uAuthHash==tReq.m_uAuthHash )
		return true;

	CSphString sTmp;
	if ( !ChangeAuth ( const_cast<char *>( tReply.m_sAuth.cstr() ), tDesc.m_sName, pAgent->m_tDesc.GetMyUrl(), sTmp ) )
	{
		sError += sTmp.cstr();
		return false;
	}

	return true;
}


bool GetRemotesAuth ( const ClusterDesc_t & tDesc )
{
	if ( !IsAuthEnabled() )
		return true;

	ClusterAuth_c::REQUEST_T tReq;
	tReq.m_bWithAuth = true;
	const AuthUsersPtr_t tAuth = GetAuth();
	CSphString sAuth = WriteJson ( *tAuth.get() );
	tReq.m_uAuthHash = sphFNV64 ( sAuth.cstr(), sAuth.Length() );

	VecRefPtrs_t<AgentConn_t*> dAgents = ClusterAuth_c::MakeAgents ( GetDescAPINodes ( tDesc.m_dClusterNodes, Resolve_e::QUICK ), tDesc.m_sUser, ReplicationTimeoutAnyNode(), tReq );
	// no nodes is a valid case
	if ( dAgents.IsEmpty() )
		return true;

	ClusterAuth_c tReply;
	PerformRemoteTasksWrap ( dAgents, tReply, tReply, false );

	StringBuilder_c sError ( "; " );

	// loop for successful agents
	// returns true on the first successful auth received and set
	// faulure at the set auth reports the error
	for ( const AgentConn_t * pAgent : dAgents )
	{
		if ( pAgent->m_bSuccess && SetAuth ( tDesc, pAgent, tReq, sError ) )
			return true;
	}

	// all auth setup errors retred first
	if ( !sError.IsEmpty() )
	{
		TlsMsg::ResetErr();
		TlsMsg::Err ( sError.cstr() );
		return false;
	}

	// loop for failed agents and report its errros
	for ( const AgentConn_t * pAgent : dAgents )
	{
		if ( !pAgent->m_bSuccess )
			sError += pAgent->m_sFailure.cstr();
	}

	TlsMsg::ResetErr();
	TlsMsg::Err ( sError.cstr() );
	return false;
}
