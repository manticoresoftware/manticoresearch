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

#include "cluster_synced.h"
#include "cluster_commands.h"

#include "api_command_cluster.h"
#include "common.h"

void operator<< ( ISphOutputBuffer& tOut, const ClusterSyncedRequest_t& tReq )
{
	tOut << (const ClusterRequest_t&)tReq;
	tOut.SendBytes ( tReq.m_tGtid.m_tUuid.data(), 16 );
	tOut.SendUint64 ( tReq.m_tGtid.m_iSeqNo );
	tOut.SendUint64 ( tReq.m_iFilesSeqNo );
	tOut.SendByte ( tReq.m_bSendFilesSuccess );
	tOut.SendString ( tReq.m_sMsg.cstr() );
	tOut << tReq.m_dIndexes;
	tOut.SendByte ( tReq.m_bIst );
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const ClusterSyncedRequest_t& tReq )
{
	tOut << (const ClusterRequest_t&)tReq;
	tOut << "gtid:" << Wsrep::Gtid2Str ( tReq.m_tGtid );
	tOut << "filesseqno:" << tReq.m_iFilesSeqNo;
	tOut << "filesendsuccess:" << (BYTE)tReq.m_bSendFilesSuccess;
	tOut << "msg:" << tReq.m_sMsg;
	tOut << "indexes:" << tReq.m_dIndexes;
	tOut << "ist:" << (BYTE)tReq.m_bIst;
	return tOut;
}

void operator>> ( InputBuffer_c& tIn, ClusterSyncedRequest_t& tReq )
{
	tIn >> (ClusterRequest_t&)tReq;
	tIn.GetBytes ( tReq.m_tGtid.m_tUuid.data(), 16 );
	tReq.m_tGtid.m_iSeqNo = tIn.GetUint64();
	tReq.m_iFilesSeqNo = (int64_t)tIn.GetUint64();
	tReq.m_bSendFilesSuccess = !!tIn.GetByte();
	tReq.m_sMsg = tIn.GetString();
	tIn >> tReq.m_dIndexes;
	tReq.m_bIst = !!tIn.GetByte();
}

void operator<< ( ISphOutputBuffer& tOut, const ClusterSyncedReply_t& tReply )
{
	tOut.SendByte ( tReply.m_bAccepted );
	tOut.SendString ( tReply.m_sReason.cstr() );
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const ClusterSyncedReply_t& tReply )
{
	tOut << "accepted:" << (BYTE)tReply.m_bAccepted;
	tOut << "reason:" << tReply.m_sReason;
	return tOut;
}

void operator>> ( InputBuffer_c& tIn, ClusterSyncedReply_t& tReply )
{
	tReply.m_bAccepted = !!tIn.GetByte();
	tReply.m_sReason = tIn.GetString();
}

// API command to remote node to issue cluster synced callback
using ClusterSynced_c = ClusterCommand_T<E_CLUSTER::SYNCED, ClusterSyncedRequest_t, ClusterSyncedReply_t>;

// API command to remote node to issue cluster synced callback
bool SendClusterSynced ( const VecAgentDesc_t & dDesc, const ClusterSyncedRequest_t & tRequest, const CSphString & sUser )
{
	ClusterSynced_c tReq;
	auto dNodes = tReq.MakeAgents ( dDesc, sUser, ReplicationTimeoutQuery(), tRequest );
	if ( !PerformRemoteTasksWrap ( dNodes, tReq, tReq, true ) )
		return false;

	for ( const AgentConn_t * pAgent : dNodes )
	{
		const ClusterSyncedReply_t & tReply = ClusterSynced_c::GetRes ( *pAgent );
		if ( !tReply.m_bAccepted )
			return TlsMsg::Err ( "'%s:%d' refused the state transfer: %s", pAgent->m_tDesc.m_sAddr.cstr(), pAgent->m_tDesc.m_iPort, tReply.m_sReason.cstr() );
	}
	return true;
}

void ReceiveClusterSynced ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString& sCluster, const char * szClient )
{
	ClusterSyncedRequest_t tCmd;
	ClusterSynced_c::ParseRequest ( tBuf, tCmd );
	sCluster = tCmd.m_sCluster;
	ClusterSyncedReply_t tReply;
	if ( !ClusterSynced ( tCmd, tReply, szClient ) )
		return;

	// a refusal is a reply: the donor must not retry it as a transport error
	TlsMsg::ResetErr();
	ClusterSynced_c::BuildReply ( tOut, tReply );
}
