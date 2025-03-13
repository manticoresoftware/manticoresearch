//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

struct SyncDst_t;
struct RemoteFileState_t
{
	const AgentDesc_t* m_pAgentDesc = nullptr;
	const SyncSrc_t * m_pSyncSrc = nullptr;
	const SyncDst_t * m_pSyncDst = nullptr;
};

// send file to multiple nodes by chunks as API command CLUSTER_FILE_SEND
bool RemoteClusterFileSend ( const SyncSrc_t & tSigSrc, const CSphVector<RemoteFileState_t> & dDesc, const CSphString & sCluster, const CSphString & sIndex );

