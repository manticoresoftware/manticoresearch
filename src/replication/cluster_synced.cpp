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

#include "cluster_synced.h"
#include "cluster_commands.h"

#include "api_command_cluster.h"
#include "common.h"

void operator<< ( ISphOutputBuffer& tOut, const ClusterSyncedRequest_t& tReq )
{
	tOut << (const ClusterRequest_t&)tReq;
	tOut.SendBytes ( tReq.m_tGtid.m_tUuid.data(), 16 );
	tOut.SendUint64 ( tReq.m_tGtid.m_iSeqNo );
	tOut.SendByte ( tReq.m_bSendFilesSuccess );
	tOut.SendString ( tReq.m_sMsg.cstr() );
	tOut << tReq.m_dIndexes;
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const ClusterSyncedRequest_t& tReq )
{
	tOut << (const ClusterRequest_t&)tReq;
	tOut << "gtid:" << Wsrep::Gtid2Str ( tReq.m_tGtid );
	tOut << "filesendsuccess:" << (BYTE)tReq.m_bSendFilesSuccess;
	tOut << "msg:" << tReq.m_sMsg;
	tOut << "indexes:" << tReq.m_dIndexes;
	return tOut;
}

void operator>> ( InputBuffer_c& tIn, ClusterSyncedRequest_t& tReq )
{
	tIn >> (ClusterRequest_t&)tReq;
	tIn.GetBytes ( tReq.m_tGtid.m_tUuid.data(), 16 );
	tReq.m_tGtid.m_iSeqNo = tIn.GetUint64();
	tReq.m_bSendFilesSuccess = !!tIn.GetByte();
	tReq.m_sMsg = tIn.GetString();
	tIn >> tReq.m_dIndexes;
}

// API command to remote node to issue cluster synced callback
using ClusterSynced_c = ClusterCommand_T<E_CLUSTER::SYNCED, ClusterSyncedRequest_t>;

// API command to remote node to issue cluster synced callback
bool SendClusterSynced ( const VecAgentDesc_t& dDesc, const ClusterSyncedRequest_t& tRequest )
{
	ClusterSynced_c tReq;
	auto dNodes = tReq.MakeAgents ( dDesc, ReplicationTimeoutQuery(), tRequest );
	return PerformRemoteTasksWrap ( dNodes, tReq, tReq, true );
}

void ReceiveClusterSynced ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString& sCluster )
{
	ClusterSyncedRequest_t tCmd;
	ClusterSynced_c::ParseRequest ( tBuf, tCmd );
	sCluster = tCmd.m_sCluster;
	if ( ClusterSynced ( tCmd ))
		ClusterSynced_c::BuildReply ( tOut );
}