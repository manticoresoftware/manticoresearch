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

#include "std/string.h"
#include "searchdaemon.h"
#include "common.h"

// command to all remote nodes at cluster to get actual nodes list
StrVec_t GetNodeListFromRemotes ( const ClusterDesc_t& tDesc );

struct RemoteNodeClusterState_t
{
	CSphString m_sNode;
	ClusterState_e m_eState;
	CSphString m_sHash;
};

using ClusterNodesStatesVec_t = CSphVector<RemoteNodeClusterState_t>;

// command to all remote nodes at cluster to get actual nodes states
ClusterNodesStatesVec_t GetStatesFromRemotes ( const ClusterDesc_t & tDesc );
bool ClusterGetState ( const CSphString & sCluster, RemoteNodeClusterState_t & tState );

bool CheckRemotesVersions ( const ClusterDesc_t & tDesc, bool bWithServerId );