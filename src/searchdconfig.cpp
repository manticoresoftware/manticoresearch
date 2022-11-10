//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "searchdconfig.h"
#include "sphinxjson.h"
#include "searchdaemon.h"
#include "searchdha.h"
#include "searchdreplication.h"
#include "fileutils.h"
#include "sphinxint.h"
#include "coroutine.h"
#include "sphinxpq.h"

using namespace Threads;

static Coro::Mutex_c g_tSaveInProgress;
static Coro::RWLock_c g_tCfgIndexesLock;

// description of clusters and indexes loaded from internal config
static CSphVector<ClusterDesc_t> g_dCfgClusters;
static CSphVector<IndexDesc_t> g_dCfgIndexes GUARDED_BY ( g_tCfgIndexesLock );

static CSphString	g_sLogFile;
static CSphString	g_sDataDir;
static CSphString	g_sConfigPath;
static bool			g_bConfigless = false;

static CSphString GetPathForNewIndex ( const CSphString & sIndexName )
{
	CSphString sRes;
	if ( g_sDataDir.Length() && !g_sDataDir.Ends("/") && !g_sDataDir.Ends("\\") )
		sRes.SetSprintf ( "%s/%s", g_sDataDir.cstr(), sIndexName.cstr() );
	else
		sRes.SetSprintf ( "%s%s", g_sDataDir.cstr(), sIndexName.cstr() );

	return sRes;
}


CSphString GetDataDirInt()
{
	return g_sDataDir;
}


bool IsConfigless()
{
	return g_bConfigless;
}


const CSphVector<ClusterDesc_t> & GetClustersInt()
{
	return g_dCfgClusters;
}


void ModifyDaemonPaths ( CSphConfigSection & hSearchd )
{
	if ( !IsConfigless() )
		return;

	const char * szBinlogKey = "binlog_path";
	if ( !hSearchd.Exists(szBinlogKey) )
	{
		CSphString sBinlogDir;
		sBinlogDir.SetSprintf ( "%s/binlog", GetDataDirInt().cstr() );
		if ( !sphDirExists ( sBinlogDir.cstr() ) )
		{
			if ( !MkDir ( sBinlogDir.cstr() ) )
			{
				sphWarning ( "Unable to create binlog dir '%s'", sBinlogDir.cstr() );
				return;
			}
		}

		hSearchd.AddEntry ( szBinlogKey, sBinlogDir.cstr() );
	}
}


// support for old-style absolute paths
static void MakeRelativePath ( CSphString & sPath )
{
	bool bAbsolute = strchr ( sPath.cstr(), '/' ) || strchr ( sPath.cstr(), '\\' );
	if ( !bAbsolute )
		sPath.SetSprintf ( "%s/%s/%s", GetDataDirInt().cstr(), sPath.cstr(), sPath.cstr() );
}

//////////////////////////////////////////////////////////////////////////

class FilenameBuilder_c : public FilenameBuilder_i
{
public:
					explicit FilenameBuilder_c ( const char * szIndex );

	CSphString		GetFullPath ( const CSphString & sName ) const final;

private:
	const CSphString		m_sIndex;
};


FilenameBuilder_c::FilenameBuilder_c ( const char * szIndex )
	: m_sIndex ( szIndex )
{}


CSphString FilenameBuilder_c::GetFullPath ( const CSphString & sName ) const
{
	if ( !IsConfigless() || !sName.Length() )
		return sName;

	CSphString sPath = GetPathForNewIndex ( m_sIndex );

	StrVec_t dFiles;
	StringBuilder_c sNewValue {" "};

	// we assume that path has been stripped before
	StrVec_t dValues = sphSplit ( sName.cstr(), sName.Length(), " \t," );
	for ( auto & i : dValues )
	{
		if ( !i.Length() )
			continue;

		CSphString & sNew = dFiles.Add();
		sNew.SetSprintf ( "%s/%s", sPath.cstr(), i.Trim().cstr() );
		sNewValue << sNew;
	}

	return sNewValue.cstr();
}


//////////////////////////////////////////////////////////////////////////

// parse and set cluster options into hash from single string
void ClusterOptions_t::Parse ( const CSphString & sOptions )
{
	if ( sOptions.IsEmpty() )
		return;

	const char * sCur = sOptions.cstr();
	while ( *sCur )
	{
		// skip leading spaces
		while ( *sCur && sphIsSpace ( *sCur ) )
			sCur++;

		if ( !*sCur )
			break;

		// skip all name characters
		const char * sNameStart = sCur;
		while ( *sCur && !sphIsSpace ( *sCur ) && *sCur!='=' )
			sCur++;

		if ( !*sCur )
			break;

		// set name
		CSphString sName;
		sName.SetBinary ( sNameStart, int ( sCur - sNameStart ) );

		// skip set char and space prior to values
		while ( *sCur && ( sphIsSpace ( *sCur ) || *sCur=='=' ) )
			sCur++;

		// skip all value characters and delimiter
		const char * sValStart = sCur;
		while ( *sCur && !sphIsSpace ( *sCur ) && *sCur!=';' )
			sCur++;

		CSphString sVal;
		sVal.SetBinary ( sValStart, int ( sCur - sValStart ) );

		m_hOptions.Add ( sVal, sName );

		// skip delimiter
		if ( *sCur )
			sCur++;
	}
}


// get string of cluster options with semicolon delimiter
CSphString ClusterOptions_t::AsStr ( bool bSave ) const
{
	StringBuilder_c tBuf ( ";" );
	for ( const auto& tOpt : m_hOptions )
	{
		// skip one time options on save
		if ( bSave && tOpt.first == "pc.bootstrap" )
			continue;

		tBuf.Appendf ( "%s=%s", tOpt.first.cstr(), tOpt.second.cstr() );
	}
	return (CSphString)tBuf;
}

//////////////////////////////////////////////////////////////////////////

bool ClusterDesc_t::Parse ( const JsonObj_c & tJson, CSphString & sWarning, CSphString & sError )
{
	m_sName = tJson.Name();
	if ( m_sName.IsEmpty() )
	{
		sError = "empty cluster name";
		return false;
	}

	// optional items such as: options and skipSst
	CSphString sOptions;
	if ( tJson.FetchStrItem ( sOptions, "options", sError, true ) )
		m_tOptions.Parse(sOptions);
	else
		sWarning = sError;

	if ( !tJson.FetchStrItem ( m_sClusterNodes, "nodes", sError, true ) )
		return false;

	if ( !tJson.FetchStrItem ( m_sPath, "path", sError, true ) )
		return false;

	// set indexes prior replication init
	JsonObj_c tIndexes = tJson.GetItem("indexes");
	int iItem = 0;
	for ( const auto & j : tIndexes )
	{
		if ( j.IsStr() )
			m_dIndexes.Add ( j.StrVal() );
		else
			sWarning.SetSprintf ( "index %d: name '%s' should be a string, skipped", iItem, m_sName.cstr() );

		iItem++;
	}

	return true;
}


void ClusterDesc_t::Save ( JsonObj_c & tClusters ) const
{
	JsonObj_c tItem;
	tItem.AddStr ( "path", m_sPath );
	tItem.AddStr ( "nodes", m_sClusterNodes );
	tItem.AddStr ( "options", m_tOptions.AsStr(true) );

	// index array
	JsonObj_c tIndexes(true);
	for ( const auto & i : m_dIndexes )
	{
		JsonObj_c tStr = JsonObj_c::CreateStr(i);
		tIndexes.AddItem(tStr);
	}

	tItem.AddItem ( "indexes", tIndexes );

	tClusters.AddItem ( m_sName.cstr(), tItem );
}

//////////////////////////////////////////////////////////////////////////

bool IndexDescDistr_t::Parse ( const JsonObj_c & tJson, CSphString & sWarning, CSphString & sError )
{
	JsonObj_c tLocals = tJson.GetItem("locals");
	for ( const auto & i : tLocals )
	{
		if ( !i.IsStr() )
		{
			sWarning = "lists of local indexes must only contain strings, skipped";
			continue;
		}

		m_dLocals.Add ( i.StrVal() );
	}

	JsonObj_c tAgents = tJson.GetItem("agents");
	for ( const auto & i : tAgents )
	{
		AgentConfigDesc_t & tNew = m_dAgents.Add();
		if ( !i.FetchStrItem ( tNew.m_sConfig, "config", sError, false ) )
			return false;

		if ( !i.FetchBoolItem ( tNew.m_bBlackhole, "blackhole", sError, true ) )
			return false;

		if ( !i.FetchBoolItem ( tNew.m_bPersistent, "persistent", sError, true ) )
			return false;
	}

	if ( !tJson.FetchIntItem ( m_iAgentConnectTimeout, "agent_connect_timeout", sError, true ) )
		return false;

	if ( !tJson.FetchIntItem ( m_iAgentQueryTimeout, "agent_query_timeout", sError, true ) )
		return false;

	if ( !tJson.FetchIntItem ( m_iAgentRetryCount, "agent_retry_count", sError, true ) )
		return false;

	if ( !tJson.FetchBoolItem ( m_bDivideRemoteRanges, "divide_remote_ranges", sError, true ) )
		return false;

	CSphString sHaStrategy;
	if ( !tJson.FetchStrItem ( sHaStrategy, "ha_strategy", sError, true ) )
		return false;

	return true;
}


void IndexDescDistr_t::Save ( JsonObj_c & tIdx ) const
{
	if ( !m_dLocals.IsEmpty() )
	{
		JsonObj_c tLocals(true);
		for ( const auto & i : m_dLocals )
		{
			JsonObj_c tStr = JsonObj_c::CreateStr(i);
			tLocals.AddItem(tStr);
		}

		tIdx.AddItem ( "locals", tLocals );
	}

	if ( !m_dAgents.IsEmpty() )
	{
		JsonObj_c tAgents(true);
		for ( const auto & i : m_dAgents )
		{
			JsonObj_c tNew;
			tNew.AddStr ( "config", i.m_sConfig );
			tNew.AddBool ( "blackhole", i.m_bBlackhole );
			tNew.AddBool ( "persistent", i.m_bPersistent );
			tAgents.AddItem(tNew);
		}

		tIdx.AddItem ( "agents", tAgents );
	}

	tIdx.AddInt ( "agent_connect_timeout",	m_iAgentConnectTimeout);
	tIdx.AddInt ( "agent_query_timeout",	m_iAgentQueryTimeout );
	if ( m_iAgentRetryCount>0 )
		tIdx.AddInt ( "agent_retry_count",	m_iAgentRetryCount );

	tIdx.AddBool( "divide_remote_ranges",	m_bDivideRemoteRanges );

	if ( !m_sHaStrategy.IsEmpty() )
		tIdx.AddStr ( "ha_strategy",		m_sHaStrategy );
}


void IndexDescDistr_t::Save ( CSphConfigSection & hIndex ) const
{
	for ( const auto & i : m_dLocals )
		hIndex.AddEntry ( "local", i.cstr() );

	for ( const auto & i : m_dAgents )
	{
		const char * szConf = i.m_sConfig.cstr();

		if ( i.m_bBlackhole )
			hIndex.AddEntry ( "agent_blackhole", szConf );
		else if ( i.m_bPersistent )
			hIndex.AddEntry ( "agent_persistent", szConf );
		else
			hIndex.AddEntry ( "agent", szConf );
	}

	CSphString sTmp;
	hIndex.AddEntry ( "agent_connect_timeout",	sTmp.SetSprintf ( "%d", m_iAgentConnectTimeout ).cstr() );
	hIndex.AddEntry ( "agent_query_timeout",	sTmp.SetSprintf ( "%d", m_iAgentQueryTimeout ).cstr() );
	if ( m_iAgentRetryCount > 0 )
		hIndex.AddEntry ( "agent_retry_count",		sTmp.SetSprintf ( "%d", m_iAgentRetryCount ).cstr() );

	hIndex.AddEntry ( "divide_remote_ranges",	m_bDivideRemoteRanges ? "1" : "0" );

	if ( !m_sHaStrategy.IsEmpty() )
		hIndex.AddEntry ( "ha_strategy",		m_sHaStrategy.cstr() );
}

//////////////////////////////////////////////////////////////////////////

bool IndexDesc_t::Parse ( const JsonObj_c & tJson, CSphString & sWarning, CSphString & sError )
{
	m_sName = tJson.Name();
	if ( m_sName.IsEmpty() )
	{
		sError = "empty index name";
		return false;
	}

	CSphString sType;
	if ( !tJson.FetchStrItem ( sType, "type", sError ) )
		return false;

	m_eType = TypeOfIndexConfig ( sType );
	if ( m_eType==IndexType_e::ERROR_ )
	{
		sError.SetSprintf ( "type '%s' is invalid", sType.cstr() );
		return false;
	}

	if ( m_eType==IndexType_e::DISTR )
	{
		bool bParseOk = m_tDistr.Parse ( tJson, sWarning, sError );
		if ( !sError.IsEmpty() )
			sError.SetSprintf ( "index %s: %s", m_sName.cstr(), sError.cstr() );

		if ( !sWarning.IsEmpty() )
			sWarning.SetSprintf ( "index %s: %s", m_sName.cstr(), sWarning.cstr() );

		if ( !bParseOk )
			return false;
	}
	else
	{
		if ( !tJson.FetchStrItem ( m_sPath, "path", sError ) )
			return false;

		MakeRelativePath(m_sPath);
	}

	return true;
}


void IndexDesc_t::Save ( JsonObj_c & tIndexes ) const
{
	JsonObj_c tIdx;
	tIdx.AddStr ( "type", GetTypeName ( m_eType ) );

	if ( m_eType==IndexType_e::DISTR )
		m_tDistr.Save(tIdx);
	else
	{
		CSphString sPath = m_sPath;
		tIdx.AddStr ( "path", StripPath(sPath) );
	}

	tIndexes.AddItem ( m_sName.cstr(), tIdx );
}


void IndexDesc_t::Save ( CSphConfigSection & hIndex ) const
{
	hIndex.Add ( CSphVariant ( GetTypeName ( m_eType ).cstr() ), "type" );

	if ( m_eType==IndexType_e::DISTR )
		m_tDistr.Save (hIndex);
	else
	{
		hIndex.Add ( CSphVariant ( m_sPath.cstr() ), "path" );

		// dummy
		hIndex.Add ( CSphVariant ( "text" ), "rt_field" );
		hIndex.Add ( CSphVariant ( "gid" ), "rt_attr_uint" );
	}
}

//////////////////////////////////////////////////////////////////////////

// read info about cluster and indexes from manticore.json and validate data
static bool ConfigRead ( const CSphString & sConfigPath, CSphVector<ClusterDesc_t> & dClusters, CSphVector<IndexDesc_t> & dIndexes, CSphString & sError )
{
	if ( !sphIsReadable ( sConfigPath, nullptr ) )
		return true;

	CSphAutoreader tConfigFile;
	if ( !tConfigFile.Open ( sConfigPath, sError ) )
		return false;

	int iSize = (int)tConfigFile.GetFilesize();
	if ( !iSize )
		return true;

	CSphFixedVector<BYTE> dData ( iSize+1 );
	tConfigFile.GetBytes ( dData.Begin(), iSize );

	if ( tConfigFile.GetErrorFlag() )
	{
		sError = tConfigFile.GetErrorMessage();
		return false;
	}

	dData[iSize] = 0; // safe gap
	JsonObj_c tRoot ( (const char*)dData.Begin() );
	if ( tRoot.GetError ( (const char *)dData.Begin(), dData.GetLength(), sError ) )
		return false;

	// FIXME!!! check for path duplicates
	// check indexes
	JsonObj_c tIndexes = tRoot.GetItem("indexes");
	for ( const auto & i : tIndexes )
	{
		IndexDesc_t tIndex;
		CSphString sWarning;
		if ( !tIndex.Parse ( i, sWarning, sError ) )
		{
			sphWarning ( "index '%s'(%d) error: %s", i.Name(), dIndexes.GetLength(), sError.cstr() );
			return false;
		}

		if ( !sWarning.IsEmpty() )
			sphWarning ( "index '%s'(%d) warning: %s", i.Name(), dIndexes.GetLength(), sWarning.cstr() );

		dIndexes.Add(tIndex);
	}

	// check clusters
	JsonObj_c tClusters = tRoot.GetItem("clusters");
	int iCluster = 0;
	for ( const auto & i : tClusters )
	{
		ClusterDesc_t tCluster;
	
		CSphString sClusterError, sClusterWarning;
		if ( !tCluster.Parse ( i, sClusterWarning, sClusterError ) )
			sphWarning ( "cluster '%s'(%d): removed from JSON config, %s", tCluster.m_sName.cstr(), iCluster, sClusterError.cstr() );
		else
			dClusters.Add(tCluster);

		if ( !sClusterWarning.IsEmpty() )
			sphWarning ( "cluster '%s'(%d): %s", tCluster.m_sName.cstr(), iCluster, sClusterWarning.cstr() );

		iCluster++;
	}

	sphLogDebug ( "config loaded, indexes %d, clusters %d", dIndexes.GetLength(), dClusters.GetLength() );

	return true;
}


static bool ConfigWrite ( const CSphString & sConfigPath, const CSphVector<ClusterDesc_t> & dClusters, const CSphVector<IndexDesc_t> & dIndexes, CSphString & sError )
{
	JsonObj_c tRoot;
	JsonObj_c tClusters;
	for ( const auto & i : dClusters )
		i.Save(tClusters);

	tRoot.AddItem ( "clusters", tClusters );

	JsonObj_c tIndexes;
	for ( const auto & i : dIndexes )
		i.Save(tIndexes);

	tRoot.AddItem ( "indexes", tIndexes );

	CSphString sNew, sOld;
	auto & sCur = sConfigPath;
	sNew.SetSprintf ( "%s.new", sCur.cstr() );
	sOld.SetSprintf ( "%s.old", sCur.cstr() );

	CSphWriter tConfigFile;
	if ( !tConfigFile.OpenFile ( sNew, sError ) )
		return false;

	CSphString sConfigData = tRoot.AsString(true);
	tConfigFile.PutBytes ( sConfigData.cstr(), sConfigData.Length() );
	tConfigFile.CloseFile();
	if ( tConfigFile.IsError() )
		return false;

	if ( sphIsReadable ( sCur, nullptr ) && rename ( sCur.cstr(), sOld.cstr() ) )
	{
		sError.SetSprintf ( "failed to rename current to old, '%s'->'%s', error '%s'", sCur.cstr(), sOld.cstr(), strerror(errno) );
		return false;
	}

	if ( rename ( sNew.cstr(), sCur.cstr() ) )
	{
		sError.SetSprintf ( "failed to rename new to current, '%s'->'%s', error '%s'", sNew.cstr(), sCur.cstr(), strerror(errno) );
		if ( sphIsReadable ( sOld, nullptr ) && rename ( sOld.cstr(), sCur.cstr() ) )
			sError.SetSprintf ( "%s, rollback failed too", sError.cstr() );
		return false;
	}

	unlink ( sOld.cstr() );

	sphLogDebug ( "config saved, indexes %d, clusters %d", dIndexes.GetLength(), dClusters.GetLength() );

	return true;
}

// ClientSession_c::Execute -> HandleMysqlImportTable -> AddExistingIndexConfigless -> ConfiglessPreloadIndex
// ServiceMain -> ConfigureAndPreload -> ConfigureAndPreloadConfiglessIndexes -> ConfiglessPreloadIndex
static ESphAddIndex ConfiglessPreloadIndex ( const IndexDesc_t & tIndex, StrVec_t & dWarnings, CSphString & sError )
{
	assert ( IsConfigless() );
	CSphConfigSection hIndex;
	tIndex.Save(hIndex);

	ESphAddIndex eAdd = ConfigureAndPreloadIndex ( hIndex, tIndex.m_sName.cstr(), dWarnings, sError );
	if ( eAdd==ADD_ERROR )
		dWarnings.Add ( "disabled at the JSON config" );

	return eAdd;
}


// load indexes got from JSON config on daemon indexes preload (part of ConfigureAndPreload work done here)
// ServiceMain -> ConfigureAndPreload -> ConfigureAndPreloadConfiglessIndexes
void ConfigureAndPreloadConfiglessIndexes ( int & iValidIndexes, int & iCounter ) REQUIRES ( MainThread )
{
	// assume g_dCfgIndexes has all locals, then all distributed. Otherwise, distr with yet invisible local agents will fail to load!
	assert ( IsConfigless() );
	SccRL_t tCfgRLock { g_tCfgIndexesLock };
	for ( const IndexDesc_t & tIndex : g_dCfgIndexes )
	{
		CSphString sError;
		StrVec_t dWarnings;
		ESphAddIndex eAdd = ConfiglessPreloadIndex ( tIndex, dWarnings, sError );
		iValidIndexes += ( eAdd==ADD_ERROR ? 0 : 1 );
		iCounter += ( eAdd== ADD_NEEDLOAD ? 1 : 0 );

		for ( const auto & i : dWarnings )
			sphWarning ( "index '%s': %s", tIndex.m_sName.cstr(), i.cstr() );

		if ( eAdd==ADD_ERROR )
			sphWarning ( "index '%s': %s - NOT SERVING", tIndex.m_sName.cstr(), sError.cstr() );
	}
}

static bool HasConfigLocal ( const CSphString & sName )
{
	SccRL_t tCfgRLock { g_tCfgIndexesLock };
	return g_dCfgIndexes.Contains ( bind ( &IndexDesc_t::m_sName ), sName );
}

// collect local indexes that should be saved into JSON config
static void CollectLocalIndexesInt ( CSphVector<IndexDesc_t> & dIndexes )
{
	if ( !IsConfigless() )
		return;

	assert ( g_pLocalIndexes );
	ServedSnap_t hLocals = g_pLocalIndexes->GetHash();
	for ( const auto & tIt : *hLocals )
	{
		assert ( tIt.second );
		IndexDesc_t & tIndex = dIndexes.Add();
		tIndex.m_sName = tIt.first;
		tIndex.m_sPath = tIt.second->m_sIndexPath;
		tIndex.m_eType = tIt.second->m_eType;
	}

	SmallStringHash_T<IndexDesc_t*> hConfigLocal;
	SccRL_t tCfgRLock { g_tCfgIndexesLock };
	for_each ( g_dCfgIndexes, [&hConfigLocal] ( IndexDesc_t& tDesc ) { hConfigLocal.Add ( &tDesc, tDesc.m_sName ); } );
	for_each ( *hLocals, [&hConfigLocal] ( auto& tIt ) { hConfigLocal.Delete ( tIt.first ); } );

	// keep indexes loaded from JSON but disabled due to errors
	for_each ( hConfigLocal, [&dIndexes] ( auto& tIt ) { assert ( tIt.second); dIndexes.Add ( *tIt.second ); } );
}


// collect distributed indexes that should be saved into JSON config
static void CollectDistIndexesInt ( CSphVector<IndexDesc_t> & dIndexes )
{
	if ( !IsConfigless() )
		return;

	assert ( g_pDistIndexes );
	auto pDistSnapshot = g_pDistIndexes->GetHash();
	for ( auto& tIt : *pDistSnapshot )
	{
		IndexDesc_t & tIndex = dIndexes.Add();
		tIndex.m_sName = tIt.first;
		tIndex.m_eType = IndexType_e::DISTR;
		const auto& tIdx = *tIt.second;

		tIndex.m_tDistr.m_dLocals				= tIdx.m_dLocal;
		tIndex.m_tDistr.m_iAgentConnectTimeout	= tIdx.m_iAgentConnectTimeoutMs;
		tIndex.m_tDistr.m_iAgentQueryTimeout	= tIdx.m_iAgentQueryTimeoutMs;
		tIndex.m_tDistr.m_iAgentRetryCount		= tIdx.m_iAgentRetryCount;
		tIndex.m_tDistr.m_bDivideRemoteRanges	= tIdx.m_bDivideRemoteRanges;
		tIndex.m_tDistr.m_sHaStrategy			= HAStrategyToStr ( tIdx.m_eHaStrategy );

		for ( const auto& i : tIdx.m_dAgents )
		{
			if ( !i || !i->GetLength() )
				continue;

			AgentConfigDesc_t & tAgent = tIndex.m_tDistr.m_dAgents.Add();
			tAgent.m_sConfig		= i->GetConfigStr();
			tAgent.m_bBlackhole		= (*i)[0].m_bBlackhole;
			tAgent.m_bPersistent	= (*i)[0].m_bPersistent;
		}
	}
}


std::unique_ptr<FilenameBuilder_i> CreateFilenameBuilder ( const char * szIndex )
{
	if ( IsConfigless() )
		return std::make_unique<FilenameBuilder_c>(szIndex);

	return nullptr;
}


static bool SetupConfiglessMode ( const CSphConfig & hConf, const CSphString & sConfigFile, CSphString & sError )
{
	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];

	if ( !hSearchd.Exists("data_dir") )
		return false;

	g_sDataDir = hSearchd["data_dir"].strval();

#if _WIN32
	g_sDataDir = AppendWinInstallDir(g_sDataDir);
#endif

	if ( !sphDirExists ( g_sDataDir.cstr(), &sError ) )
	{
		sError.SetSprintf ( "%s; make sure it is accessible or remove data_dir from the config file", sError.cstr() );
		return false;
	}

	if ( hConf.Exists("index") )
	{
		sError.SetSprintf ( "'data_dir' cannot be mixed with index declarations in '%s'", sConfigFile.cstr() );
		return false;
	}

	if ( hConf.Exists("source") )
	{
		sError.SetSprintf ( "'data_dir' cannot be mixed with source declarations in '%s'", sConfigFile.cstr() );
		return false;
	}

	SetIndexFilenameBuilder ( CreateFilenameBuilder );

	return true;
}

static const char * g_sJsonConfName = "manticore.json";

// load data from JSON config on daemon start
bool LoadConfigInt ( const CSphConfig & hConf, const CSphString & sConfigFile, CSphString & sError ) REQUIRES (MainThread)
{
	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];
	g_sLogFile = hSearchd.GetStr ( "log", "" );

	g_bConfigless = hSearchd.Exists("data_dir");
	if ( !g_bConfigless )
		return true;

	if ( !SetupConfiglessMode ( hConf, sConfigFile, sError ) )
		return false;

	// node with empty incoming addresses works as GARB - does not affect FC
	// might hung on pushing 1500 transactions
	ReplicationSetIncoming ( hSearchd.GetStr ( "node_address" ) );

	// check data_dir exists and could write there
	if ( !CheckPath ( g_sDataDir, true, sError ) )
		return false;

	g_sConfigPath.SetSprintf ( "%s/%s", g_sDataDir.cstr(), g_sJsonConfName );
	
	// check that file is readable in case it exists
	if ( sphFileExists ( g_sConfigPath.cstr(), nullptr ) && !sphIsReadable ( g_sConfigPath.cstr(), &sError ) )
	{
		sError.SetSprintf ( "failed to use JSON config %s: %s", g_sConfigPath.cstr(), sError.cstr() );
		return false;
	}

	SccWL_t tCfgWLock { g_tCfgIndexesLock };
	if ( !ConfigRead ( g_sConfigPath, g_dCfgClusters, g_dCfgIndexes, sError ) )
	{
		sError.SetSprintf ( "failed to use JSON config %s: %s", g_sConfigPath.cstr(), sError.cstr() );
		return false;
	}

	return true;
}


bool SaveConfigInt ( CSphString & sError )
{
	return Threads::CallCoroutineRes ( [&sError]
	{

	ScopedCoroMutex_t tSaving ( g_tSaveInProgress );

	if ( !ReplicationIsEnabled() && !IsConfigless() )
		return true;
	
	CSphVector<ClusterDesc_t> dClusters;

	// save clusters and their indexes into JSON config on daemon shutdown
	if ( ReplicationIsEnabled() )
		ReplicationCollectClusters ( dClusters );

	CSphVector<IndexDesc_t> dIndexes;
	CollectLocalIndexesInt ( dIndexes );
	CollectDistIndexesInt ( dIndexes );

	if ( !ConfigWrite ( g_sConfigPath, dClusters, dIndexes, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return false;
	}

	return true;
	});
}

//////////////////////////////////////////////////////////////////////////

static bool PrepareDirForNewIndex ( CSphString & sPath, CSphString & sIndexPath, const CSphString & sIndexName, CSphString & sError )
{
	CSphString sNewPath = GetPathForNewIndex(sIndexName);
	StringBuilder_c sRes;
	sRes << sNewPath;

	if ( sphDirExists ( sRes.cstr() ) )
	{
		StringBuilder_c sMask;
		sMask << sRes.cstr() << "/*";
		StrVec_t dFiles = FindFiles ( sMask.cstr() );
		if ( dFiles.GetLength() )
		{
			bool bLockFileLeft = dFiles.GetLength()==1 && dFiles[0].Ends (".lock");
			if ( !bLockFileLeft )
			{
				sError.SetSprintf ( "directory is not empty: %s", sRes.cstr() );
				return false;
			}
		}
	}
	else if ( !MkDir ( sRes.cstr() ) )
	{
		sError.SetSprintf ( "can't create directory: %s", sRes.cstr() );
		return false;
	}

	sRes << "/";
	sPath = sRes.cstr();
	sRes << sIndexName;
	sIndexPath = sRes.cstr();

	return true;
}


bool CopyExternalIndexFiles ( const StrVec_t & dFiles, const CSphString & sDestPath, StrVec_t & dCopied, CSphString & sError )
{
	for ( const auto & i : dFiles )
	{
		CSphString sDest = i;
		StripPath(sDest);
		sDest.SetSprintf ( "%s%s", sDestPath.cstr(), sDest.cstr() );
		if ( i==sDest )
			continue;

		if ( !CopyFile ( i, sDest, sError ) )
			return false;

		dCopied.Add(sDest);
	}

	return true;
}


static std::unique_ptr<CSphIndex> TryToPreallocRt ( const CSphString & sIndex, const CSphString & sNewIndexPath, StrVec_t & dWarnings, CSphString & sError )
{
	CSphSchema tSchemaStub;
	auto pRT = sphCreateIndexRT ( tSchemaStub, sIndex.cstr(), 32*1024*1024, sNewIndexPath.cstr(), true );
	if ( !pRT->Prealloc ( false, nullptr, dWarnings ) )
	{
		sError.SetSprintf ( "failed to prealloc: %s", pRT->GetLastError().cstr() );
		return nullptr;
	}

	return pRT;
}


static std::unique_ptr<CSphIndex> TryToPreallocPq ( const CSphString & sIndex, const CSphString & sNewIndexPath, StrVec_t & dWarnings, CSphString & sError )
{
	CSphSchema tSchemaStub;
	auto pPQ = CreateIndexPercolate ( tSchemaStub, sIndex.cstr(), sNewIndexPath.cstr() );
	if ( !pPQ->Prealloc ( false, nullptr, dWarnings ) )
	{
		sError.SetSprintf ( "failed to prealloc: %s", pPQ->GetLastError().cstr() );
		return nullptr;
	}

	// FIXME! just Prealloc is not enough for PQ index to properly save meta on deallocation
	pPQ->PostSetup();

	return pPQ;
}


static bool CopyExternalFiles ( const CSphString & sIndex, const CSphString & sNewIndexPath, StrVec_t & dCopied, bool & bPQ, StrVec_t & dWarnings, CSphString & sError )
{
	bPQ = false;

	CSphString sRtError, sPqError;
	auto pIndex = TryToPreallocRt ( sIndex, sNewIndexPath, dWarnings, sRtError );
	if ( !pIndex )
	{
		pIndex = TryToPreallocPq ( sIndex, sNewIndexPath, dWarnings, sPqError );
		if ( !pIndex )
		{
			sError = sRtError;
			return false;
		}

		bPQ = true;
	}

	if ( !pIndex->CopyExternalFiles ( 0, dCopied ) )
	{
		sError = pIndex->GetLastError();
		return false;
	}

	return true;
}


bool CopyIndexFiles ( const CSphString & sIndex, const CSphString & sPathToIndex, bool & bPQ, StrVec_t & dWarnings, CSphString & sError )
{
	CSphString sPath, sNewIndexPath;
	if ( !PrepareDirForNewIndex ( sPath, sNewIndexPath, sIndex, sError ) )
		return false;

	StrVec_t dWipe;
	auto tCleanup = AtScopeExit ( [&dWipe] { dWipe.for_each ( [] ( const auto& i ) { unlink ( i.cstr() ); } ); } );

	CSphString sFind;
	sFind.SetSprintf ( "%s.*", sPathToIndex.cstr() );
	StrVec_t dFoundFiles = FindFiles ( sFind.cstr(), false );

	// checks for source index
	if ( !dFoundFiles.GetLength() )
	{
		sError = "no index files found";
		return false;
	}

	if ( !dFoundFiles.any_of ( [] ( const CSphString & sFile ) { return sFile.Ends ( ".meta" ); } ) )
	{
		sError.SetSprintf ( "missing %s.meta index file", sPathToIndex.cstr() );
		return false;
	}

	for ( const auto & i : dFoundFiles )
	{
		CSphString sDest;
		const char * szExt = GetExtension(i);
		if ( !szExt )
			continue;

		sDest.SetSprintf ( "%s.%s", sNewIndexPath.cstr(), szExt );
		if ( !CopyFile ( i, sDest, sError ) )
			return false;

		dWipe.Add(sDest);
	}

	if ( !CopyExternalFiles ( sIndex, sNewIndexPath, dWipe, bPQ, dWarnings, sError ) )
		return false;

	dWipe.Reset();
	return true;
}


static bool CheckCreateTableSettings ( const CreateTableSettings_t & tCreateTable, CSphString & sError )
{
	static const char * dForbidden[] = { "path", "stored_fields", "stored_only_fields", "columnar_attrs", "columnar_no_fast_fetch", "rowwise_attrs", "rt_field", "embedded_limit" };
	static const char * dTypes[] = { "rt", "pq", "percolate", "distributed" };

	for ( const auto & i : tCreateTable.m_dOpts )
	{
		for ( const auto & j : dForbidden )
			if ( i.m_sName==j )
			{
				sError.SetSprintf ( "setting not allowed: %s='%s'", i.m_sName.cstr(), i.m_sValue.cstr() );
				return false;
			}

		for ( int j = 0; j < GetNumRtTypes(); j++ )
			if ( i.m_sName==GetRtType(j).m_szName )
			{
				sError.SetSprintf ( "setting not allowed: %s='%s'", i.m_sName.cstr(), i.m_sValue.cstr() );
				return false;
			}

		if ( i.m_sName=="type" )
		{
			bool bFound = false;
			for ( const auto & j : dTypes )
				bFound |= i.m_sValue==j;

			if ( !bFound )
			{
				sError.SetSprintf ( "setting not allowed: %s='%s'", i.m_sName.cstr(), i.m_sValue.cstr() );
				return false;
			}
		}
	}

	return true;
}


CSphString BuildCreateTableDistr ( const CSphString & sName, const DistributedIndex_t & tDistr )
{
	StringBuilder_c sRes(" ");
	sRes << "CREATE TABLE" << sName << "type='distributed'";

	for ( const auto & i : tDistr.m_dLocal )
	{
		CSphString sLocal;
		sRes << sLocal.SetSprintf ( "local='%s'", i.cstr() );
	}

	for ( const auto& i : tDistr.m_dAgents )
	{
		CSphString sAgent;

		if ( !i || !i->GetLength() )
			continue;

		if ( (*i)[0].m_bBlackhole )
			sAgent = "agent_blackhole";
		else if ( (*i)[0].m_bPersistent )
			sAgent = "agent_persistent";
		else
			sAgent = "agent";

		sRes << sAgent.SetSprintf ( "%s='%s'", sAgent.cstr(), i->GetConfigStr().cstr() );
	}

	DistributedIndexRefPtr_t pDefault ( new DistributedIndex_t );
	CSphString sOpt;
	if ( tDistr.m_iAgentConnectTimeoutMs!=pDefault->m_iAgentConnectTimeoutMs )
		sRes << sOpt.SetSprintf ( "agent_connect_timeout='%d'", tDistr.m_iAgentConnectTimeoutMs );

	if ( tDistr.m_iAgentQueryTimeoutMs!=pDefault->m_iAgentQueryTimeoutMs )
		sRes << sOpt.SetSprintf ( "agent_query_timeout='%d'", tDistr.m_iAgentQueryTimeoutMs );

	if ( tDistr.m_iAgentRetryCount!=pDefault->m_iAgentRetryCount )
		sRes << sOpt.SetSprintf ( "agent_retry_count='%d'", tDistr.m_iAgentRetryCount );

	if ( tDistr.m_bDivideRemoteRanges!=pDefault->m_bDivideRemoteRanges )
		sRes << sOpt.SetSprintf ( "divide_remote_ranges='%d'", tDistr.m_bDivideRemoteRanges ? 1 : 0 );

	if ( tDistr.m_eHaStrategy!=pDefault->m_eHaStrategy )
		sRes << sOpt.SetSprintf ( "ha_strategy='%s'", HAStrategyToStr ( tDistr.m_eHaStrategy ).cstr() );

	return sRes.cstr();
}


static void DeleteExtraIndexFiles ( CSphIndex * pIndex )
{
	assert(pIndex);

	CSphString sTmp;
	CSphString sPath = GetPathForNewIndex ( pIndex->GetName() );

	auto pTokenizer = pIndex->GetTokenizer();
	auto pDict = pIndex->GetDictionary();

	if ( pTokenizer )
	{
		// single file
		const CSphString & sExceptions = pTokenizer->GetSettings().m_sSynonymsFile;
		if ( sExceptions.Length() )
		{
			sTmp.SetSprintf ( "%s/%s", sPath.cstr(), sExceptions.cstr() );
			::unlink ( sTmp.cstr() );
		}
	}

	if ( pDict )
	{
		// space-separated string
		const CSphString & sStopwords =  pDict->GetSettings().m_sStopwords;
		StrVec_t dStops = sphSplit ( sStopwords.cstr(), sStopwords.Length(), " \t," );
		for ( const auto & i : dStops )
		{
			sTmp.SetSprintf ( "%s/%s", sPath.cstr(), i.cstr() );
			::unlink ( sTmp.cstr() );
		}

		// array
		for ( const auto & i : pDict->GetSettings().m_dWordforms )
		{
			sTmp.SetSprintf ( "%s/%s", sPath.cstr(), i.cstr() );
			::unlink ( sTmp.cstr() );
		}
	}
}


static void DeleteRtIndex ( CSphIndex* pIdx )
{
	assert ( IsConfigless() );
	if ( !pIdx->IsRT() && !pIdx->IsPQ() )
		return;
	auto pRt = static_cast<RtIndex_i*> ( pIdx );
	pRt->IndexDeleted();
	DeleteExtraIndexFiles ( pRt );
}

static void RemoveAndDeleteRtIndex ( const CSphString& sIndex )
{
	assert ( IsConfigless() );
	cServedIndexRefPtr_c pServed = GetServed ( sIndex );
	if ( !pServed )
		return;

	WIdx_c pIdx { pServed };
	DeleteRtIndex ( pIdx );
	g_pLocalIndexes->Delete ( sIndex );
}


bool CreateNewIndexConfigless ( const CSphString & sIndex, const CreateTableSettings_t & tCreateTable, StrVec_t & dWarnings, CSphString & sError )
{
	assert ( IsConfigless() );
	if ( tCreateTable.m_bIfNotExists && IndexIsServed ( sIndex ) )
		return true;

	if ( !CheckCreateTableSettings ( tCreateTable, sError ) )
		return false;

	IndexSettingsContainer_c tSettingsContainer;
	if ( !tSettingsContainer.Populate ( tCreateTable ) )
	{
		sError = tSettingsContainer.GetError();
		return false;
	}

	StrVec_t dWipe;
	auto tCleanup = AtScopeExit ( [&dWipe] { dWipe.for_each ( [] ( const auto& i ) { unlink ( i.cstr() ); } ); } );

	if ( tSettingsContainer.Get ( "type" ) != "distributed")
	{
		CSphString sPath, sIndexPath;
		if ( !PrepareDirForNewIndex ( sPath, sIndexPath, sIndex, sError ) )
		{
			if ( HasConfigLocal ( sIndex ) )
				sError.SetSprintf ( "%s (the index may be corrupted, refer to Manticore log)", sError.cstr() );
			return false;
		}

		tSettingsContainer.Add ( "path", sIndexPath );
		if ( !CopyExternalIndexFiles ( tSettingsContainer.GetFiles(), sPath, dWipe, sError ) )
			return false;
	}

	const CSphConfigSection & hCfg = tSettingsContainer.AsCfg();

	ESphAddIndex eAdd;
	ServedIndexRefPtr_c pDesc;
	std::tie ( eAdd, pDesc ) = AddIndex ( sIndex.cstr(), hCfg, true, true, &dWarnings, sError );
	switch ( eAdd )
	{
	case ADD_ERROR: return false;
	case ADD_NEEDLOAD:
		{
			assert ( pDesc );
			if ( !PreallocNewIndex ( *pDesc, &hCfg, sIndex.cstr(), dWarnings, sError ) )
			{
				DeleteRtIndex ( UnlockedHazardIdxFromServed ( *pDesc ) );
				return false;
			}
		}
		// no break
	case ADD_SERVED:
		g_pLocalIndexes->Add ( pDesc, sIndex );
	case ADD_DISTR:
	default:
		break;
	}

	if ( SaveConfigInt ( sError ) )
	{
		dWipe.Reset();
		return true;
	}

	cServedIndexRefPtr_c pServed = GetServed ( sIndex );
	if ( pServed )
		RemoveAndDeleteRtIndex ( sIndex );
	else
		g_pDistIndexes->Delete ( sIndex );
	return false;
}


class ScopedCleanup_c
{
public:
	explicit ScopedCleanup_c ( CSphString sIndex )
		: m_sIndex ( std::move ( sIndex ) )
	{}

	void Ok()
	{
		m_bOk = true;
	}

	~ScopedCleanup_c()
	{
		if ( m_bOk )
			return;

		RemoveAndDeleteRtIndex ( m_sIndex );
	}

private:
	CSphString	m_sIndex;
	bool		m_bOk = false;
};

// ClientSession_c::Execute -> HandleMysqlImportTable -> AddExistingIndexConfigless
bool AddExistingIndexConfigless ( const CSphString & sIndex, IndexType_e eType, StrVec_t & dWarnings, CSphString & sError )
{
	assert ( IsConfigless() );
	ScopedCleanup_c tCleanup ( sIndex );

	IndexDesc_t tNewIndex;
	tNewIndex.m_eType = eType;
	tNewIndex.m_sName = sIndex;
	tNewIndex.m_sPath.SetSprintf ( "%s/%s", GetPathForNewIndex(sIndex).cstr(), sIndex.cstr() );

	if ( ConfiglessPreloadIndex ( tNewIndex, dWarnings, sError )!= ADD_NEEDLOAD )
		return false;

	if ( !SaveConfigInt ( sError ) )
		return false;

	tCleanup.Ok();
	return true;
}


static bool DropDistrIndex ( const CSphString & sIndex, CSphString & sError )
{
	assert ( IsConfigless() );
	auto pDistr = GetDistr(sIndex);
	if ( !pDistr )
	{
		sError.SetSprintf ( "DROP TABLE failed: unknown distributed index '%s'", sIndex.cstr() );
		return false;
	}

	g_pDistIndexes->Delete(sIndex);

	return true;
}

static void RemoveConfigIndex ( const CSphString & sIndex )
{
	g_pLocalIndexes->Delete ( sIndex );

	// also remove index from list of indexes at the JSON config
	SccWL_t tCfgWLock { g_tCfgIndexesLock };
	int iIdx = g_dCfgIndexes.GetFirst ( [&] ( const IndexDesc_t & tIdx ) { return tIdx.m_sName==sIndex; } );

	if ( iIdx>=0 )
		g_dCfgIndexes.Remove ( iIdx );
}

static bool DropLocalIndex ( const CSphString & sIndex, CSphString & sError )
{
	assert ( IsConfigless() );
	auto pServed = GetServed(sIndex);
	if ( !pServed )
	{
		sError.SetSprintf ( "DROP TABLE failed: unknown local index '%s'", sIndex.cstr() );
		return false;
	}

	if ( ServedDesc_t::IsCluster ( pServed ) )
	{
		sError.SetSprintf ( "DROP TABLE failed: unable to drop a cluster index '%s'", sIndex.cstr() );
		return false;
	}

	WIdx_T<RtIndex_i*> pRt { pServed };
	if ( !pRt )
	{
		sError.SetSprintf ( "DROP TABLE failed: unknown local index '%s'", sIndex.cstr() );
		return false;
	}

	if ( !pRt->Truncate(sError) )
		return false;

	DeleteRtIndex ( pRt );
	RemoveConfigIndex ( sIndex );

	return true;
}


bool DropIndexInt ( const CSphString & sIndex, bool bIfExists, CSphString & sError )
{
	assert ( IsConfigless() );
	bool bLocal = !!GetServed(sIndex);
	bool bDistr = !!GetDistr(sIndex);
	if ( bDistr )
	{
		if ( !DropDistrIndex ( sIndex, sError ) )
			return false;
	}
	else if ( bLocal )
	{
		if ( !DropLocalIndex ( sIndex, sError ) )
			return false;
	}
	else
	{
		if ( bIfExists )
			return true;

		sError.SetSprintf ( "DROP TABLE failed: unknown index '%s'", sIndex.cstr() );
		return false;
	}

	// we are unable to roll back the drop at this point
	if ( !SaveConfigInt(sError) )
	{
		sError.SetSprintf ( "DROP TABLE failed for index '%s': %s", sIndex.cstr(), sError.cstr() );
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
RunIdx_e IndexIsServed ( const CSphString& sName )
{
	if ( g_pLocalIndexes && g_pLocalIndexes->Contains ( sName ) )
		return LOCAL;

	if ( g_pDistIndexes && g_pDistIndexes->Contains ( sName ) )
		return DISTR;
	return NOTSERVED;
}