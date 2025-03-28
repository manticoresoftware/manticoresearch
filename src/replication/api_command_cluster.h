//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "std/ints.h"
#include "std/string.h"
#include "std/fixedvector.h"
#include "std/bitvec.h"
#include "searchdconfig.h"
#include "searchdha.h"
#include "wsrep_cxx.h"
#include "send_files.h"

#include <memory>

// API commands that not get replicated via Galera, cluster management
enum class E_CLUSTER : WORD
{
	DELETE_				= 0, // named this way because of macro conflict with winnt.h
	FILE_RESERVE		= 1,
	FILE_SEND			= 3,
	INDEX_ADD_LOCAL		= 4,
	SYNCED				= 5,
	GET_NODES			= 6,
	UPDATE_NODES		= 7,
	INDEX_ADD_DIST		= 8,
	GET_NODE_STATE		= 9,
	GET_NODE_VER		= 10,
	GET_NODE_VER_ID		= 11,
};

inline constexpr const char* szClusterCmd ( E_CLUSTER eCmd )
{
	switch ( eCmd )
	{
	case E_CLUSTER::DELETE_ : return "delete";
	case E_CLUSTER::FILE_RESERVE: return "file_reserve";
	case E_CLUSTER::FILE_SEND: return "file_send";
	case E_CLUSTER::INDEX_ADD_LOCAL: return "index_add_local";
	case E_CLUSTER::SYNCED: return "synced";
	case E_CLUSTER::GET_NODES: return "get_nodes";
	case E_CLUSTER::UPDATE_NODES: return "update_nodes";
	case E_CLUSTER::INDEX_ADD_DIST: return "index_add_distributed";
	case E_CLUSTER::GET_NODE_STATE: return "get_node_state";
	case E_CLUSTER::GET_NODE_VER: return "get_node_ver";
	case E_CLUSTER::GET_NODE_VER_ID: return "get_node_ver_id";
	default: return "unknown";
	}
}

struct ClusterRequest_t
{
	CSphString m_sCluster;
};

struct EmptyReply_t { };

// use void as result type, as we never chain this requests.
void operator<< ( ISphOutputBuffer&, const ClusterRequest_t& );
void operator>> ( InputBuffer_c&, ClusterRequest_t& );
StringBuilder_c& operator<< ( StringBuilder_c&, const ClusterRequest_t& );

void operator<< ( ISphOutputBuffer&, const StrVec_t& );
void operator>> ( InputBuffer_c&, StrVec_t& );
StringBuilder_c& operator<< ( StringBuilder_c&, const StrVec_t& );

inline void operator<< ( ISphOutputBuffer& tOut, const EmptyReply_t& ) { tOut.SendByte ( 1 ); }
inline void operator>> ( InputBuffer_c& tIn, EmptyReply_t& ) { tIn.GetByte(); }
inline StringBuilder_c& operator<< ( StringBuilder_c& tOut, const EmptyReply_t& ) { tOut << "byte 1 (fake)"; return tOut; }


template <typename REQUEST, typename REPLY>
struct CustomAgentData_T final: public DefaultQueryResult_t
{
	const REQUEST& m_tRequest;
	REPLY m_tReply;

	explicit CustomAgentData_T ( const REQUEST& tReq )
		: m_tRequest ( tReq )
	{}
};


// base of API commands request and reply builders
AgentConn_t* CreateAgentBase ( const AgentDesc_t& tDesc, int64_t iTimeoutMs );

// set to true to see all proto exchanging in the log
constexpr bool VERBOSE_LOG = false;

template<E_CLUSTER CMD, typename REQUEST = ClusterRequest_t, typename REPLY = EmptyReply_t >
class ClusterCommand_T: public RequestBuilder_i, public ReplyParser_i
{
public:
	using REQUEST_T = REQUEST;
	using REPLY_T = REPLY;

	static REPLY& GetRes ( const AgentConn_t& tAgent )
	{
		auto* pResult = static_cast<CustomAgentData_T<REQUEST, REPLY>*> ( tAgent.m_pResult.get() );
		assert ( pResult );
		return pResult->m_tReply;
	}

	static const REQUEST& GetReq ( const AgentConn_t& tAgent )
	{
		const auto* pResult = (CustomAgentData_T<REQUEST, REPLY>*)tAgent.m_pResult.get();
		assert ( pResult );
		return pResult->m_tRequest;
	}

	static AgentConn_t* CreateAgent ( const AgentDesc_t& tDesc, int64_t iTimeoutMs, const REQUEST& tReq )
	{
		auto* pAgent = CreateAgentBase ( tDesc, iTimeoutMs );
		pAgent->m_pResult = std::make_unique<CustomAgentData_T<REQUEST, REPLY>> ( tReq );
		return pAgent;
	}

	static VecRefPtrs_t<AgentConn_t*> MakeAgents ( const VecTraits_T<AgentDesc_t>& dDesc, int64_t iTimeout, const REQUEST& tReq )
	{
		VecRefPtrs_t<AgentConn_t*> dNodes;
		dNodes.Resize ( dDesc.GetLength() );
		ARRAY_FOREACH ( i, dDesc )
			dNodes[i] = CreateAgent ( dDesc[i], iTimeout, tReq );
		return dNodes;
	}

	template<typename REQ>
	static void VerboseProtoImpl ( const char* szCmd, const REQ& tData )
	{
		StringBuilder_c sRequest;
		sRequest.Sprintf ( "%s [%s]: ", szCmd, szClusterCmd ( CMD ) );
		ScopedComma_c space ( sRequest, " " );
		sRequest << tData;
		sphLogDebugRpl ( "%s", sRequest.cstr() );
	}

#define VerboseProto( ... ) do if constexpr ( VERBOSE_LOG ) VerboseProtoImpl ( __VA_ARGS__ ); while ( 0 )

	void BuildRequest ( const AgentConn_t& tAgent, ISphOutputBuffer& tOut ) const final
	{
		if ( CMD==E_CLUSTER::FILE_SEND )
		{
			{
				auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_PERSIST );
				tOut.SendInt ( 1 ); // set persistent to 1
			}
		}
		// API header
		auto tReply = APIHeader ( tOut, SEARCHD_COMMAND_CLUSTER, VER_COMMAND_CLUSTER );
		tOut.SendWord ( static_cast<WORD> ( CMD ) );
		tOut << GetReq ( tAgent );

		if ( CMD==E_CLUSTER::FILE_SEND )
		{
			{
				auto tHdr = APIHeader ( tOut, SEARCHD_COMMAND_PERSIST );
				tOut.SendInt ( 0 ); // set persistent to 0
			}
		}

		VerboseProto ( "BldRq", GetReq ( tAgent ) );
	}

	static void ParseRequest (  InputBuffer_c& tIn, REQUEST_T& tReq )
	{
		tIn >> tReq;
		VerboseProto ( "PrsRq", tReq );
	}

	static void BuildReply ( ISphOutputBuffer& tOut, const REPLY_T& tRes = {} )
	{
		auto tReply = APIAnswer ( tOut );
		tOut << tRes;
		VerboseProto ( "BldRpl", tRes );
	}

	bool ParseReply ( InputBuffer_c& tReq, AgentConn_t& tAgent ) const final
	{
		tReq >> GetRes ( tAgent );
		VerboseProto ( "PrsRpl", GetRes ( tAgent ) );
		return !tReq.GetError();
	}
};

bool PerformRemoteTasksWrap ( VectorAgentConn_t & dNodes, RequestBuilder_i & tReq, ReplyParser_i & tReply, bool bRetry );

// handle all API incoming.
void HandleAPICommandCluster ( ISphOutputBuffer& tOut, WORD uCommandVer, InputBuffer_c& tBuf, const char* szClient );

void ReplicationSetTimeouts ( int iConnectTimeoutMs, int iQueryTimeoutMs, int iRetryCount, int iRetryDelayMs );

int64_t ReplicationTimeoutQuery ( int64_t iTimeout = 0 ); // 2 minutes in msec
int ReplicationTimeoutConnect ();
int ReplicationRetryCount ();
int ReplicationRetryDelay ();
int ReplicationTimeoutAnyNode ();
int ReplicationFileRetryCount ();
int ReplicationFileRetryDelay ();
void ReportClusterError ( const CSphString& sCluster, const CSphString& sError, const char* szClient, E_CLUSTER eCmd );

