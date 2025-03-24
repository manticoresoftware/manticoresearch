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

#include "cluster_update_nodes.h"

#include "api_command_cluster.h"
#include "common.h"
#include "nodes.h"

// API command to remote node to update nodes by nodes it sees
using ClusterUpdateNodes_c = ClusterCommand_T<E_CLUSTER::UPDATE_NODES, UpdateNodesRequest_t>;

void operator<< ( ISphOutputBuffer& tOut, const UpdateNodesRequest_t& tReq )
{
	tOut << (const ClusterRequest_t&)tReq;
	tOut.SendByte ( static_cast<bool> ( tReq.m_eKindNodes ) ? 1 : 0 );
}

void operator>> ( InputBuffer_c& tIn, UpdateNodesRequest_t& tReq )
{
	tIn >> (ClusterRequest_t&)tReq;
	tReq.m_eKindNodes = (NODES_E)( tIn.GetByte() ? 1 : 0 );
}

bool SendClusterUpdateNodes ( const CSphString& sCluster, NODES_E eNodes, const VecTraits_T<CSphString>& dNodes )
{
	ClusterUpdateNodes_c::REQUEST_T tRequest;
	tRequest.m_sCluster = sCluster;
	tRequest.m_eKindNodes = eNodes;

	auto dAgents = ClusterUpdateNodes_c::MakeAgents ( GetDescAPINodes ( dNodes, Resolve_e::SLOW ), ReplicationTimeoutQuery(), tRequest );
	// no nodes left seems a valid case
	if ( dAgents.IsEmpty() )
		return true;

	ClusterUpdateNodes_c tReq;
	return PerformRemoteTasksWrap ( dAgents, tReq, tReq, true );
}


void ReceiveClusterUpdateNodes ( ISphOutputBuffer& tOut, InputBuffer_c& tBuf, CSphString& sCluster )
{
	UpdateNodesRequest_t tRequest;
	ClusterUpdateNodes_c::ParseRequest ( tBuf, tRequest );
	sCluster = tRequest.m_sCluster;
	if ( ClusterUpdateNodes ( tRequest.m_sCluster, tRequest.m_eKindNodes ) )
		ClusterUpdateNodes_c::BuildReply ( tOut );
}