//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

/// @file _searchdreplication_.h
/// Declarations for the stuff specifically needed for replication


#ifndef _searchdreplication_
#define _searchdreplication_

#include "searchdsql.h"
#include <optional>

// return path to given cluster, or empty and error
std::optional<CSphString> GetClusterPath ( const CSphString& sCluster );

// collect all available into an array
CSphVector<ClusterDesc_t> ReplicationCollectClusters ();

// set Galera option for cluster
bool ReplicateSetOption ( const CSphString & sCluster, const CSphString & sName, const CSphString & sVal );

// single point there all commands passed these might be replicated, even if no cluster
bool HandleCmdReplicate ( RtAccum_t & tAcc );
bool HandleCmdReplicateDelete ( RtAccum_t & tAcc, int & iDeletedCount );
bool HandleCmdReplicateUpdate ( RtAccum_t & tAcc, CSphString & sWarning, int & iUpdated );

// Commit replicated commands came from cluster
bool HandleCmdReplicated ( RtAccum_t& m_tAcc );

// delete all clusters on daemon shutdown
void ReplicationServiceShutdown();

// prepare saved clusters (but not yet touch wsrep or even update nodes from remotes; only offline actions)
// bForce will cause 'safe_to_bootstrap: 1' added to grastate.data
void PrepareClustersOnStartup ( const VecTraits_T<ListenerDesc_t> & dListeners, bool bForce );

// start clusters on daemon start
void ReplicationServiceStart ( bool bBootStrap );

// cluster joins to existed nodes
bool ClusterJoin ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, bool bUpdateNodes );

// cluster creates master node
bool ClusterCreate ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues );

// cluster deletes
bool GloballyDeleteCluster ( const CSphString & sCluster, CSphString & sError );

// Return actual nodes list at cluster
StrVec_t ClusterGetAllNodes ( const CSphString& sCluster );

// cluster ALTER statement
bool ClusterAlter ( const CSphString & sCluster, const CSphString & sIndex, bool bAdd, CSphString & sError );

// cluster ALTER statement that updates nodes option from view nodes at all nodes at cluster
bool ClusterAlterUpdate ( const CSphString & sCluster, const CSphString & sUpdate, CSphString & sError );

// dump all clusters statuses
void ReplicateClustersStatus ( VectorLike & dStatus );

// validate that SphinxQL statement could be run for this cluster:index
bool ValidateClusterStatement ( const CSphString & sIndexName, const ServedDesc_t & tDesc, const CSphString & sStmtCluster, bool bHTTP );

std::optional<CSphString> IsPartOfCluster ( const ServedDesc_t* pDesc );

// set cluster name into index(es) desc for fast rejects
bool AssignClusterToIndex ( const CSphString & sIndex, const CSphString & sCluster );
bool AssignClusterToIndexes ( const VecTraits_T<CSphString> & dIndexes, const CSphString & sCluster );

bool SetIndexesClusterTOI ( const ReplicationCommand_t * pCmd );

CSphString WaitClusterReady ( const CSphString& sCluster, int64_t iTimeoutS );
std::pair<int,CSphString> WaitClusterCommit ( const CSphString& sCluster, int iTxn, int64_t iTimeoutS );

#endif // _searchdreplication_
