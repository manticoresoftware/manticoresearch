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
#include "cluster_delete.h"
#include "cluster_commands.h"

#include "api_command_cluster.h"

#include "common.h"
#include "nodes.h"

// API command to remote node to delete cluster
using ClusterDelete_c = ClusterCommand_T<E_CLUSTER::DELETE_>;

void ReceiveClusterDelete ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString& sCluster )
{
	ClusterRequest_t tCmd;
	ClusterDelete_c::ParseRequest ( tBuf, tCmd );
	sCluster = tCmd.m_sCluster;

	TLS_MSG_STRING ( sError );
	if ( ClusterDelete ( sCluster ) && SaveConfigInt ( sError ) )
		ClusterDelete_c::BuildReply ( tOut );
}

void SendClusterDeleteToNodes ( const VecTraits_T<CSphString>& dNodes, const CSphString& sCluster )
{
	if ( dNodes.IsEmpty() )
		return;

	ClusterRequest_t tData { sCluster };
	ClusterDelete_c tReq;
	auto dAgents = tReq.MakeAgents ( GetDescAPINodes ( dNodes, Resolve_e::SLOW ), ReplicationTimeoutQuery(), tData );
	PerformRemoteTasksWrap ( dAgents, tReq, tReq, true );
}
