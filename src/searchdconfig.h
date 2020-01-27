//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
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


struct ClusterOptions_t
{
	SmallStringHash_T<CSphString> m_hOptions;

	void			Parse ( const CSphString & sOptions );
	CSphString		AsStr ( bool bSave ) const;
};


class JsonObj_c;

// cluster data that gets stored and loaded
struct ClusterDesc_t
{
	CSphString				m_sName;			// cluster name
	CSphString				m_sPath;			// path relative to data_dir
	CSphVector<CSphString>	m_dIndexes;			// list of index name in cluster
	CSphString				m_sClusterNodes;	// string list of comma separated nodes (node - address:API_port)
	ClusterOptions_t		m_tOptions;			// options for Galera

	bool					Parse ( const JsonObj_c & tJson, CSphString & sWarning, CSphString & sError );
	void					Save ( JsonObj_c & tClusters ) const;
};


struct IndexDesc_t
{
	CSphString	m_sName;
	CSphString	m_sPath;
	IndexType_e	m_eType = IndexType_e::ERROR_;
	bool		m_bFromReplication = false;

	bool		Parse ( const JsonObj_c & tJson, CSphString & sError );
	void		Save ( JsonObj_c & tIndexes ) const;
};


// load data from internal config on daemon start
void		LoadConfigInt ( const CSphConfig & hConf, const CSphString & sConfigFile );
void		SaveConfigInt();

// read info about cluster and indexes from manticore.json and validate data
bool		ConfigRead ( const CSphString & sConfigPath, CSphVector<ClusterDesc_t> & dClusters, CSphVector<IndexDesc_t> & dIndexes, CSphString & sError );

// load indexes got from internal config on daemon indexes preload (part of ConfigureAndPreload work done here)
void		ConfigureAndPreloadInt ( int & iValidIndexes, int & iCounter );

CSphString	GetDataDirInt();
bool		IsConfigless();
const CSphVector<ClusterDesc_t> & GetClustersInt();

CSphIndex *	CreateNewIndexInt ( const CSphString & sIndex, const CreateTableSettings_t & tCreateTable, bool bBinlogReplay, CSphString & sError );
bool		DropIndexInt ( const CSphString & sIndex, bool bIfExists, bool bBinlogReplay, CSphString & sError );

#endif // _searchdconfig_
