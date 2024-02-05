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