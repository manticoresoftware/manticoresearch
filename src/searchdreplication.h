//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

bool ReplicationIsEnabled();
void ReplicationSetIncoming ( CSphString sIncoming );

// collect all available into an array
void ReplicationCollectClusters ( CSphVector<ClusterDesc_t> & dClusters );

// set Galera option for cluster
bool ReplicateSetOption ( const CSphString & sCluster, const CSphString & sName, const CSphString & sVal, CSphString & sError );

// single point there all commands passed these might be replicated, even if no cluster
bool HandleCmdReplicate ( RtAccum_t & tAcc, CSphString & sError );
bool HandleCmdReplicate ( RtAccum_t & tAcc, CSphString & sError, int & iDeletedCount );
bool HandleCmdReplicate ( RtAccum_t & tAcc, CSphString & sError, CSphString & sWarning, int & iUpdated );

// delete all clusters on daemon shutdown
void ReplicateClustersDelete();

// start clusters on daemon start
void ReplicationStart ( const VecTraits_T<ListenerDesc_t> & dListeners, bool bNewCluster, bool bForce );

// cluster joins to existed nodes
bool ClusterJoin ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, bool bUpdateNodes, CSphString & sError );

// cluster creates master node
bool ClusterCreate ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, CSphString & sError );

// cluster deletes
bool ClusterDelete ( const CSphString & sCluster, CSphString & sError, CSphString & sWarning );

// handler of all remote commands via API parsed at daemon as SEARCHD_COMMAND_CLUSTERPQ
void HandleCommandClusterPq ( ISphOutputBuffer & tOut, WORD uCommandVer, InputBuffer_c & tBuf, const char * sClient );

// cluster ALTER statement
bool ClusterAlter ( const CSphString & sCluster, const CSphString & sIndex, bool bAdd, CSphString & sError, CSphString & sWarning );

// cluster ALTER statement that updates nodes option from view nodes at all nodes at cluster
bool ClusterAlterUpdate ( const CSphString & sCluster, const CSphString & sUpdate, bool bRemoteError, CSphString & sError );

// dump all clusters statuses
void ReplicateClustersStatus ( VectorLike & dStatus );

// validate that SphinxQL statement could be run for this cluster:index
bool CheckIndexCluster ( const CSphString & sIndexName, const ServedDesc_t & tDesc, const CSphString & sStmtCluster, bool bHTTP, CSphString & sError );
std::optional<CSphString> IsPartOfCluster ( const ServedDesc_t* pDesc );

// set cluster name into index desc for fast rejects
bool SetIndexCluster ( const CSphString& sIndex, const CSphString& sCluster, CSphString * pError=nullptr );

CSphString WaitClusterReady ( const CSphString& sCluster, int64_t iTimeoutS );
std::pair<int,CSphString> WaitClusterCommit ( const CSphString& sCluster, int iTxn, int64_t iTimeoutS );

#endif // _searchdreplication_
