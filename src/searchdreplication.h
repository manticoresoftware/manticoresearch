//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
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

enum ProtocolType_e
{
	PROTO_SPHINX = 0,
	PROTO_MYSQL41,
	PROTO_HTTP,
	PROTO_REPLICATION,

	PROTO_TOTAL
};

struct ListenerDesc_t
{
	Proto_e			m_eProto;
	CSphString		m_sUnix;
	DWORD			m_uIP;
	int				m_iPort;
	int				m_iPortsCount;
	bool			m_bVIP;
};

struct ReplicationCommand_t;

// load data from JSON config on daemon start
void JsonLoadConfig ( const CSphConfigSection & hSearchd );

// save clusters and their indexes into JSON config on daemon shutdown
void JsonDoneConfig();

// load indexes got from JSON config on daemon indexes preload (part of ConfigureAndPreload work done here)
void JsonConfigConfigureAndPreload ( int & iValidIndexes, int & iCounter  );

// set Galera option for cluster
bool ReplicateSetOption ( const CSphString & sCluster, const CSphString & sName, const CSphString & sVal, CSphString & sError );

// single point there all commands passed these might be replicated, even if no cluster
bool HandleCmdReplicate ( RtAccum_t & tAcc, CSphString & sError, int * pDeletedCount );

// delete all clusters on daemon shutdown
void ReplicateClustersDelete();

// start clusters on daemon start
void ReplicationStart ( const CSphConfigSection & hSearchd, const CSphVector<ListenerDesc_t> & dListeners, bool bNewCluster, bool bForce );

// cluster joins to existed nodes
bool ClusterJoin ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, bool bUpdateNodes, CSphString & sError );

// cluster creates master node
bool ClusterCreate ( const CSphString & sCluster, const StrVec_t & dNames, const CSphVector<SqlInsert_t> & dValues, CSphString & sError );

// cluster deletes
bool ClusterDelete ( const CSphString & sCluster, CSphString & sError, CSphString & sWarning );

// handler of all remote commands via API parsed at daemon as SEARCHD_COMMAND_CLUSTERPQ
void HandleCommandClusterPq ( CachedOutputBuffer_c & tOut, WORD uCommandVer, InputBuffer_c & tBuf, const char * sClient );

// cluster ALTER statement
bool ClusterAlter ( const CSphString & sCluster, const CSphString & sIndex, bool bAdd, CSphString & sError, CSphString & sWarning );

// cluster ALTER statement that updates nodes option from view nodes at all nodes at cluster
bool ClusterAlterUpdate ( const CSphString & sCluster, const CSphString & sUpdate, CSphString & sError );

// 'like' matcher
class CheckLike
{
private:
	CSphString m_sPattern;

public:
	explicit CheckLike ( const char * sPattern );
	bool Match ( const char * sValue );
};

// string vector with 'like' matcher
class VectorLike : public StrVec_t, public CheckLike
{
public:
	CSphString m_sColKey;
	CSphString m_sColValue;

public:

	VectorLike ();
	explicit VectorLike ( const CSphString& sPattern );

	const char * szColKey() const;
	const char * szColValue() const;
	bool MatchAdd ( const char* sValue );
	bool MatchAddVa ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
};

// dump all clusters statuses
void ReplicateClustersStatus ( VectorLike & dStatus );

// forwards from searchd
#define SPH_ADDRESS_SIZE		sizeof("000.000.000.000")
#define SPH_ADDRPORT_SIZE		sizeof("000.000.000.000:00000")

ListenerDesc_t ParseListener ( const char * sSpec );
ESphAddIndex ConfigureAndPreload ( const CSphConfigSection & hIndex, const char * sIndexName, bool bJson );
ESphAddIndex AddIndex ( const char * szIndexName, const CSphConfigSection & hIndex, bool bReplace=false );
bool PreallocNewIndex ( ServedDesc_t & tIdx, const CSphConfigSection * pConfig, const char * szIndexName );
bool CheckIndexCluster ( const CSphString & sIndexName, const ServedDesc_t & tDesc, const CSphString & sStmtCluster, CSphString & sError );

CSphString GetMacAddress ();

#endif // _searchdreplication_
