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

#pragma once

#include "api_command_cluster.h"

struct ClusterDesc_t;

struct CheckNodeRequest_t : ClusterRequest_t
{
	CSphString m_sNode; // API address of the node to check
};

struct CheckNodeReply_t
{
	CSphString m_sError;	// empty when the node is reachable
	bool m_bFinal = false;	// a config error, retries will not help
	int m_iServerId = 0;	// of the answering node, to skip answers from the asking node itself
};

void operator<< ( ISphOutputBuffer & tOut, const CheckNodeRequest_t & tReq );
void operator>> ( InputBuffer_c & tIn, CheckNodeRequest_t & tReq );
StringBuilder_c & operator<< ( StringBuilder_c & tOut, const CheckNodeRequest_t & tReq );

void operator<< ( ISphOutputBuffer & tOut, const CheckNodeReply_t & tReply );
void operator>> ( InputBuffer_c & tIn, CheckNodeReply_t & tReply );
StringBuilder_c & operator<< ( StringBuilder_c & tOut, const CheckNodeReply_t & tReply );

using ClusterCheckNode_c = ClusterCommand_T<E_CLUSTER::CHECK_NODE, CheckNodeRequest_t, CheckNodeReply_t>;

// API handler: can this node reach the node from the request
void ReceiveClusterCheckNode ( ISphOutputBuffer & tOut, InputBuffer_c & tBuf, CSphString & sCluster );

// before joining: every node of the view must be able to reach this node at its API address,
// as the donor connects to it for the state transfer; fails when a node answers it cannot
bool CheckRemotesReachThisNode ( const ClusterDesc_t & tDesc, const VecTraits_T<CSphString> & dViewNodes );
