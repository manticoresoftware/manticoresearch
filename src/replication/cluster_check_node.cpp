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

#include "cluster_check_node.h"

#include "nodes.h"
#include "configuration.h"
#include "coroutine.h"
#include "searchdaemon.h"
#include "searchdreplication.h"

void operator<< ( ISphOutputBuffer & tOut, const CheckNodeRequest_t & tReq )
{
	tOut << (const ClusterRequest_t &)tReq;
	tOut.SendString ( tReq.m_sNode.cstr() );
}

void operator>> ( InputBuffer_c & tIn, CheckNodeRequest_t & tReq )
{
	tIn >> (ClusterRequest_t &)tReq;
	tReq.m_sNode = tIn.GetString();
}

StringBuilder_c & operator<< ( StringBuilder_c & tOut, const CheckNodeRequest_t & tReq )
{
	tOut << (const ClusterRequest_t &)tReq << "node:" << tReq.m_sNode;
	return tOut;
}

void operator<< ( ISphOutputBuffer & tOut, const CheckNodeReply_t & tReply )
{
	tOut.SendString ( tReply.m_sError.cstr() );
	tOut.SendByte ( tReply.m_bFinal ? 1 : 0 );
	tOut.SendInt ( tReply.m_iServerId );
}

void operator>> ( InputBuffer_c & tIn, CheckNodeReply_t & tReply )
{
	tReply.m_sError = tIn.GetString();
	tReply.m_bFinal = !!tIn.GetByte();
	tReply.m_iServerId = tIn.GetInt();
}

StringBuilder_c & operator<< ( StringBuilder_c & tOut, const CheckNodeReply_t & tReply )
{
	tOut << "error:" << tReply.m_sError << "final:" << ( tReply.m_bFinal ? 1 : 0 ) << "server_id:" << tReply.m_iServerId;
	return tOut;
}

// SEARCHD_COMMAND_PING in the same wire format as taskping.cpp, without the HA dashboard part
class PingNode_c final : public RequestBuilder_i, public ReplyParser_i
{
public:
	void BuildRequest ( const AgentConn_t &, ISphOutputBuffer & tOut ) const final
	{
		auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_PING, VER_COMMAND_PING );
		tOut.SendInt ( 0 ); // cookie, echoed back
	}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final
	{
		tReq.GetInt();
		return !tReq.GetError();
	}
};

// empty when this node can reach sNode: the same resolve as the state transfer, then a ping to its API port
static CSphString CheckNodeReachable ( const CSphString & sNode, bool & bFinal )
{
	bFinal = true;
	if ( sNode.IsEmpty() )
		return "empty node address";

	// the state transfer skips its own address the same way, so it would never reach the asking node
	if ( MyIncomingApiAddrBeginsWith ( sNode.cstr() ) )
		return SphSprintf ( "'%s' is also the API address of this node, node_address must be unique", sNode.cstr() );

	bFinal = false;

	StrVec_t dNodes;
	dNodes.Add ( sNode );
	VecAgentDesc_t dDesc = GetDescAPINodes ( dNodes, Resolve_e::QUICK );
	if ( dDesc.IsEmpty() )
		return TlsMsg::HasErr() ? TlsMsg::MoveToString() : SphSprintf ( "'%s' is not an API address", sNode.cstr() );
	TlsMsg::ResetErr();

	VecRefPtrs_t<AgentConn_t *> dAgents;
	dAgents.Add ( CreateAgentBase ( dDesc[0], ReplicationSstQueryTimeout() ) ); // replication_query_timeout
	SetAuth ( session::GetUser(), dAgents );
	PingNode_c tPing;
	PerformRemoteTasks ( dAgents, &tPing, &tPing );
	TlsMsg::ResetErr();

	if ( dAgents[0]->m_bSuccess )
		return {};

	return SphSprintf ( "ping '%s' failed: %s", sNode.cstr(), dAgents[0]->m_sFailure.scstr() );
}

void ReceiveClusterCheckNode ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString & sCluster )
{
	CheckNodeRequest_t tReq;
	ClusterCheckNode_c::ParseRequest ( tBuf, tReq );
	sCluster = tReq.m_sCluster;

	// the result goes into the reply, so the asking node can tell "cannot reach" from "did not answer"
	CheckNodeReply_t tReply;
	tReply.m_sError = CheckNodeReachable ( tReq.m_sNode, tReply.m_bFinal );
	tReply.m_iServerId = GetUidShortServerId();
	TlsMsg::ResetErr();

	sphLogDebugRpl ( "cluster '%s': check node '%s': %s", sCluster.cstr(), tReq.m_sNode.cstr(), tReply.m_sError.IsEmpty() ? "ok" : tReply.m_sError.cstr() );
	ClusterCheckNode_c::BuildReply ( tOut, tReply );
}

static CSphString NodeName ( const AgentDesc_t & tDesc )
{
	return SphSprintf ( "%s:%d", tDesc.m_sAddr.cstr(), tDesc.m_iPort );
}

bool CheckRemotesReachThisNode ( const ClusterDesc_t & tDesc, const VecTraits_T<CSphString> & dViewNodes )
{
	const CSphString sMe = szIncomingProto();

	// resolve one by one, so a name this node cannot resolve does not hide the other nodes (it is skipped with a warning);
	// keep entries equal to this node's own address: another node may use the same node_address;
	// an answer from this node itself (a stale view entry after a restart) is skipped by its server_id
	StrVec_t dApiNodes = FilterNodesByProto ( dViewNodes, Proto_e::SPHINX, false );
	VecAgentDesc_t dAsk = GetDescAPINodesEach ( dApiNodes, tDesc.m_sName, false );
	TlsMsg::ResetErr();

	if ( dAsk.IsEmpty() )
	{
		sphLogDebugRpl ( "cluster '%s': no other nodes to check that they reach this node at '%s'", tDesc.m_sName.cstr(), sMe.cstr() );
		return true;
	}

	CheckNodeRequest_t tReq;
	tReq.m_sCluster = tDesc.m_sName;
	tReq.m_sNode = sMe;

	int64_t tmStart = sphMicroTimer();
	int iOk = 0;
	for ( int iAttempt = 0; ; ++iAttempt )
	{
		auto dAgents = ClusterCheckNode_c::MakeAgents ( dAsk, tDesc.m_sUser, ReplicationTimeoutQuery(), tReq );
		ClusterCheckNode_c tCheck;
		PerformRemoteTasks ( dAgents, &tCheck, &tCheck );
		TlsMsg::ResetErr();

		// a node that does not answer is skipped: it is not in the primary component now and cannot be the donor
		VecAgentDesc_t dNegative;
		CSphString sLastNode, sLastError;
		bool bFinal = false;
		for ( const AgentConn_t * pAgent : dAgents )
		{
			if ( !pAgent->m_bSuccess )
			{
				sphWarning ( "cluster '%s': node '%s' did not answer the check that it can reach this node at '%s': %s", tDesc.m_sName.cstr(), NodeName ( pAgent->m_tDesc ).cstr(), sMe.cstr(), pAgent->m_sFailure.scstr() );
				continue;
			}

			const CheckNodeReply_t & tReply = ClusterCheckNode_c::GetRes ( *pAgent );
			if ( tReply.m_iServerId==GetUidShortServerId() )
				continue;

			if ( tReply.m_sError.IsEmpty() )
			{
				++iOk;
				continue;
			}

			// report a final error over a transient one
			if ( tReply.m_bFinal || !bFinal )
			{
				sLastNode = NodeName ( pAgent->m_tDesc );
				sLastError = tReply.m_sError;
			}
			bFinal |= tReply.m_bFinal;
			dNegative.Add().CloneFrom ( pAgent->m_tDesc );
		}

		if ( dNegative.IsEmpty() )
			break;

		if ( bFinal )
			return TlsMsg::Err ( "cluster '%s': node '%s' cannot reach this node at '%s': %s", tDesc.m_sName.cstr(), sLastNode.cstr(), sMe.cstr(), sLastError.cstr() );

		int iDelayMs = SlowRetryDelayMs ( iAttempt );
		if ( iDelayMs<0 )
			return TlsMsg::Err ( "cluster '%s': node '%s' cannot reach this node at '%s': %s; in %d attempts within %.3f sec", tDesc.m_sName.cstr(), sLastNode.cstr(), sMe.cstr(), sLastError.cstr(), iAttempt+1, ( sphMicroTimer() - tmStart ) / 1000000.0f );

		sphLogDebugRpl ( "cluster '%s': %d node(s) cannot reach this node at '%s' yet, retry in %.3f sec; '%s': %s", tDesc.m_sName.cstr(), dNegative.GetLength(), sMe.cstr(), iDelayMs / 1000.0f, sLastNode.cstr(), sLastError.cstr() );
		if ( sphInterrupted() )
			return TlsMsg::Err ( "daemon shutdown" );

		Threads::Coro::SleepMsec ( iDelayMs );
		dAsk = std::move ( dNegative );
	}

	if ( !iOk )
		return TlsMsg::Err ( "cluster '%s': no node answered the check that it can reach this node at '%s'", tDesc.m_sName.cstr(), sMe.cstr() );

	return true;
}
