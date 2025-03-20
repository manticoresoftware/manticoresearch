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
#include "api_command_cluster.h"

enum class NODES_E : bool;

struct UpdateNodesRequest_t : public ClusterRequest_t
{
	NODES_E m_eKindNodes;
};

bool SendClusterUpdateNodes ( const CSphString& sCluster, NODES_E eNodes, const VecTraits_T<CSphString>& dNodes );

