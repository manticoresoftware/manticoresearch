//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _searchdconfig_
#define _searchdconfig_

#include "sphinxutils.h"
#include "indexsettings.h"

enum class IndexType_e
{
	PLAIN = 0,
	TEMPLATE,
	RT,
	PERCOLATE,
	DISTR,
	ERROR_, // simple "ERROR" doesn't work on win due to '#define ERROR 0' somewhere.
};

const char * szIndexType ( IndexType_e eType );

struct ClusterOptions_t
{
	SmallStringHash_T<CSphString> m_hOptions;

	void			Parse ( const CSphString & sOptions );
	CSphString		AsStr ( bool bSave ) const;
};


class JsonObj_c;

// cluster data that gets stored and loaded
class ClusterDesc_t
{
public:
	ClusterDesc_t() = default;
	virtual ~ClusterDesc_t() {}
	CSphString				m_sName;			// cluster name
	CSphString				m_sPath;			// path relative to data_dir
	CSphVector<CSphString>	m_dIndexes;			// list of index name in cluster
	CSphString				m_sClusterNodes;	// string list of comma separated nodes (node - address:API_port)
	ClusterOptions_t		m_tOptions;			// options for Galera

	bool					Parse ( const JsonObj_c & tJson, CSphString & sWarning, CSphString & sError );
	void					Save ( JsonObj_c & tClusters ) const;
};


struct AgentConfigDesc_t
{
	CSphString		m_sConfig;
	bool			m_bPersistent = false;
	bool			m_bBlackhole = false;
};


struct IndexDescDistr_t
{
	StrVec_t		m_dLocals;
	CSphVector<AgentConfigDesc_t> m_dAgents;
	int				m_iAgentConnectTimeout = 0;
	int				m_iAgentQueryTimeout = 0;
	int				m_iAgentRetryCount = 0;
	bool			m_bDivideRemoteRanges = false;
	CSphString		m_sHaStrategy;

	bool			Parse ( const JsonObj_c & tJson, CSphString & sWarning, CSphString & sError );
	void			Save ( JsonObj_c & tIndexes ) const;
	void			Save ( CSphConfigSection & hIndex ) const;
};


struct IndexDesc_t
{
	CSphString	m_sName;
	CSphString	m_sPath;
	IndexType_e	m_eType = IndexType_e::ERROR_;

	IndexDescDistr_t m_tDistr;

	bool		Parse ( const JsonObj_c & tJson, CSphString & sWarning, CSphString & sError );
	void		Save ( JsonObj_c & tIndexes ) const;
	void		Save ( CSphConfigSection & hIndex ) const;
};


// load data from internal config on daemon start
bool		LoadConfigInt ( const CSphConfig & hConf, const CSphString & sConfigFile, CSphString & sError );
bool		SaveConfigInt ( CSphString & sError );

// load indexes got from internal config on daemon indexes preload (part of ConfigureAndPreload work done here)
void		ConfigureAndPreloadConfiglessIndexes ( int & iValidIndexes, int & iCounter );

std::unique_ptr<FilenameBuilder_i> CreateFilenameBuilder ( const char * szIndex );

void		ModifyDaemonPaths ( CSphConfigSection & hSearchd );
CSphString	GetDataDirInt();
bool		IsConfigless();
const CSphVector<ClusterDesc_t> & GetClustersInt();

struct DistributedIndex_t;
CSphString	BuildCreateTableDistr ( const CSphString & sName, const DistributedIndex_t & tDistr );

bool		CreateNewIndexConfigless ( const CSphString & sIndex, const CreateTableSettings_t & tCreateTable, StrVec_t & dWarnings, CSphString & sError );
bool		AddExistingIndexConfigless ( const CSphString & sIndex, IndexType_e eType, StrVec_t & dWarnings, CSphString & sError );
bool		DropIndexInt ( const CSphString & sIndex, bool bIfExists, CSphString & sError );
bool		CopyExternalIndexFiles ( const StrVec_t & dFiles, const CSphString & sDestPath, StrVec_t & dCopied, CSphString & sError );
bool		CopyIndexFiles ( const CSphString & sIndex, const CSphString & sPathToIndex, bool & bPQ, StrVec_t & dWarnings, CSphString & sError );

enum RunIdx_e : int {
	NOTSERVED = 0,
	LOCAL,
	DISTR,
};
RunIdx_e		IndexIsServed ( const CSphString& sName );

#endif // _searchdconfig_
