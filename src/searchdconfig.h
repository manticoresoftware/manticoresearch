//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

	void						Parse ( const CSphString & sOptions );
	[[nodiscard]] CSphString	AsStr () const;
	[[nodiscard]] bool			IsEmpty() const noexcept;
};

// cluster data that gets stored and loaded
struct ClusterDesc_t
{
	CSphString				m_sName;			// cluster name
	CSphString				m_sPath;			// path relative to data_dir
	sph::StringSet			m_hIndexes;			// list of index name in cluster
	StrVec_t				m_dClusterNodes;	// string list of nodes (node - address:API_port)
	ClusterOptions_t		m_tOptions;			// options for Galera

	bool					Parse ( const bson::Bson_c & tBson, const CSphString& sName, CSphString & sWarning );
	void					Save ( JsonEscapedBuilder& tOut ) const;
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

	bool			Parse ( const bson::Bson_c & tBson, CSphString & sWarning );
	void 			Save ( JsonEscapedBuilder& tOut ) const;
	void			Save ( CSphConfigSection & hIndex ) const;
};


struct IndexDesc_t
{
	CSphString	m_sName;
	CSphString	m_sPath;
	IndexType_e	m_eType = IndexType_e::ERROR_;

	IndexDescDistr_t m_tDistr;

	bool		Parse ( const bson::Bson_c & tBson, const CSphString& sName, CSphString & sWarning );
	void		Save ( JsonEscapedBuilder& tOut ) const;
	void		Save ( CSphConfigSection & hIndex ) const;
};


// load data from internal config on daemon start
bool		LoadConfigInt ( const CSphConfig & hConf, const CSphString & sConfigFile, CSphString & sError );
bool		SaveConfigInt ( CSphString & sError );
struct DistributedIndex_t;
IndexDescDistr_t GetDistributedDesc ( const DistributedIndex_t & tDist );

// load indexes got from internal config on daemon indexes preload (part of ConfigureAndPreload work done here)
void		ConfigureAndPreloadConfiglessIndexes ( int & iValidIndexes, int & iCounter );

std::unique_ptr<FilenameBuilder_i> CreateFilenameBuilder ( const char * szIndex );

void		ModifyDaemonPaths ( CSphConfigSection & hSearchd, FixPathAbsolute_fn && fnPathFix = nullptr );
CSphString	GetDataDirInt();

// create string by join global data_dir and given path
CSphString	GetDatadirPath ( const CSphString& sPath );

bool		IsConfigless();
const CSphVector<ClusterDesc_t> & GetClustersInt();

struct DistributedIndex_t;
CSphString	BuildCreateTableDistr ( const CSphString & sName, const DistributedIndex_t & tDistr );

bool		CreateNewIndexConfigless ( const CSphString & sIndex, const CreateTableSettings_t & tCreateTable, StrVec_t & dWarnings, CSphString & sError );
bool		AddExistingIndexConfigless ( const CSphString & sIndex, IndexType_e eType, StrVec_t & dWarnings, CSphString & sError );
bool		DropIndexInt ( const CSphString & sIndex, bool bIfExists, bool bForce, CSphString & sError, CSphString * pWarning=nullptr );
bool		CopyIndexFiles ( const CSphString & sIndex, const CSphString & sPathToIndex, bool & bPQ, StrVec_t & dWarnings, CSphString & sError );
bool		IsDistrTableHasSystem ( const DistributedIndex_t & tDistr, bool bForce );

enum RunIdx_e : int {
	NOTSERVED = 0,
	LOCAL,
	DISTR,
};
RunIdx_e		IndexIsServed ( const CSphString& sName );

#endif // _searchdconfig_
