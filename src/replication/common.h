//
// Copyright (c) 2019-2023, Manticore Software LTD (https://manticoresearch.com)
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

// check whether local indexes with given name exists and it is mutable (pq or rt)
bool CheckMutableIndexExists ( const CSphString & sIndex );

// Update nodes list at cluster from actual nodes list
enum class NODES_E : bool { VIEW, BOTH };
bool ClusterUpdateNodes ( const CSphString& sCluster, NODES_E eNodes, StrVec_t* pNodes = nullptr );

void ReportClusterError ( const CSphString& sCluster, const CSphString& sError, const char* szClient, int iCmd );

// command at remote node for CLUSTER_DELETE to delete cluster
bool ClusterDelete ( const CSphString& sCluster );

// command at remote node for CLUSTER_SYNCED to pick up received indexes and notify Galera that sst received
struct ClusterSyncedRequest_t;
bool ClusterSynced ( const ClusterSyncedRequest_t& tCmd );
