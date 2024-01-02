//
// Copyright (c) 2019-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "api_command_cluster.h"

#include "configuration.h"
#include "digest_sha1.h"
#include "common.h"
#include "searchdha.h"
#include "cluster_commands.h"

void operator<< ( ISphOutputBuffer& tOut, const ClusterRequest_t& tReq )
{
	tOut.SendString ( tReq.m_sCluster.cstr() );
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const ClusterRequest_t& tReq )
{
	tOut << "cluster:" << tReq.m_sCluster;
	return tOut;
}

void operator>> ( InputBuffer_c& tIn, ClusterRequest_t& tReq )
{
	tReq.m_sCluster = tIn.GetString();
}

void operator<< ( ISphOutputBuffer& tOut, const StrVec_t& dStrings )
{
	tOut.SendInt ( dStrings.GetLength() );
	for ( const auto& sVal : dStrings )
		tOut.SendString ( sVal.cstr() );
}

StringBuilder_c& operator<< ( StringBuilder_c& tOut, const StrVec_t& dStrings )
{
	tOut << "strings (" << dStrings.GetLength() << "):";
	int iStrings = Min ( 10, dStrings.GetLength() );
	for ( int i=0; i<iStrings; ++i )
		tOut << dStrings[i];
	if ( iStrings > dStrings.GetLength() )
		tOut << "...rest" << dStrings.GetLength() - iStrings;
	return tOut;
}

void operator>> ( InputBuffer_c& tIn, StrVec_t& dStrings )
{
	dStrings.Resize ( tIn.GetInt() );
	for ( auto& sVal : dStrings )
		sVal = tIn.GetString();
}

AgentConn_t* CreateAgentBase ( const AgentDesc_t& tDesc, int64_t iTimeoutMs )
{
	auto* pAgent = new AgentConn_t;
	pAgent->m_tDesc.CloneFrom ( tDesc );
	pAgent->SetNoLimitReplySize();
	pAgent->m_tDesc.m_pDash = new HostDashboard_t;
	pAgent->m_iMyConnectTimeoutMs = g_iAgentConnectTimeoutMs;
	pAgent->m_iMyQueryTimeoutMs = iTimeoutMs;
	return pAgent;
}

// wrapper of PerformRemoteTasks
bool PerformRemoteTasksWrap ( VectorAgentConn_t & dNodes, RequestBuilder_i & tReq, ReplyParser_i & tReply/*, int iQueryRetry, int iQueryDelay*/ )
{
	if ( dNodes.IsEmpty() )
		return true;

	int iNodes = dNodes.GetLength();
	int iFinished = PerformRemoteTasks ( dNodes, &tReq, &tReply/*, iQueryRetry, iQueryDelay*/ );

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
	if ( iFinished!=iNodes && !tTmp.IsEmpty() )
		TlsMsg::Err() << tTmp.cstr();
	if ( TlsMsg::HasErr() )
		sphWarning ( "nodes %d(%d), tls msg: %s", iFinished, iNodes, TlsMsg::szError() );

	return ( iFinished==iNodes && !TlsMsg::HasErr() );
}


// handler of all remote commands via API parsed at daemon as SEARCHD_COMMAND_CLUSTER
void HandleAPICommandCluster ( ISphOutputBuffer & tOut, WORD uCommandVer, InputBuffer_c & tBuf, const char * szClient )
{
	if ( !CheckCommandVersion ( uCommandVer, VER_COMMAND_CLUSTER, tOut ))
		return;

	auto eClusterCmd = (E_CLUSTER) tBuf.GetWord ();
	CSphString sCluster;

	if ( eClusterCmd!=E_CLUSTER::FILE_SEND )
		sphLogDebugRpl ( "remote cluster command %d(%s), client %s", (int) eClusterCmd, szClusterCmd (eClusterCmd), szClient );

	TlsMsg::ResetErr();
	switch (eClusterCmd) {
	case E_CLUSTER::DELETE_:
		ReceiveClusterDelete ( tOut, tBuf, sCluster );
		break;

	case E_CLUSTER::FILE_RESERVE:
		ReceiveClusterFileReserve ( tOut, tBuf, sCluster );
		break;

	case E_CLUSTER::FILE_SEND:
		ReceiveClusterFileSend ( tOut, tBuf );
		break;

	case E_CLUSTER::INDEX_ADD_LOCAL:
		ReceiveClusterIndexAddLocal ( tOut, tBuf, sCluster );
		break;

	case E_CLUSTER::SYNCED:
		ReceiveClusterSynced ( tOut, tBuf, sCluster );
		break;

	case E_CLUSTER::GET_NODES:
		ReceiveClusterGetNodes ( tOut, tBuf, sCluster );
		break;

	case E_CLUSTER::UPDATE_NODES:
		ReceiveClusterUpdateNodes ( tOut, tBuf, sCluster );
		break;

	case E_CLUSTER::INDEX_ADD_DIST:
		ReceiveDistIndex ( tOut, tBuf, sCluster );
		break;

	default:
		TlsMsg::Err ( "INTERNAL ERROR: unhandled command %d", (int) eClusterCmd );
		break;
	}

	if ( !TlsMsg::HasErr() )
		return;

	assert ( eClusterCmd != E_CLUSTER::FILE_SEND );

	auto szError = TlsMsg::szError();
	sphLogDebugRpl ( "remote cluster '%s' command %d, client %s - %s", sCluster.scstr(), (int)eClusterCmd, szClient, szError );

	auto tReply = APIHeader ( tOut, SEARCHD_ERROR );
	tOut.SendString ( SphSprintf ( "[%s] %s", szIncomingIP(), szError ).cstr() );

	ReportClusterError ( sCluster, szError, szClient, (int)eClusterCmd );
}