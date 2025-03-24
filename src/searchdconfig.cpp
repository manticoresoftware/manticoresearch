//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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
#include "replication/configuration.h"
#include "fileutils.h"
#include "sphinxint.h"
#include "coroutine.h"
#include "sphinxpq.h"
#include "binlog.h"
#include "global_idf.h"

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

CSphString GetDatadirPath ( const CSphString& sPath )
{
	if ( sPath.IsEmpty() )
		return GetDataDirInt();

	return SphSprintf ( "%s/%s", GetDataDirInt().cstr(), sPath.cstr() );
}


bool IsConfigless()
{
	return g_bConfigless;
}


const CSphVector<ClusterDesc_t> & GetClustersInt()
{
	return g_dCfgClusters;
}


void ModifyDaemonPaths ( CSphConfigSection & hSearchd, FixPathAbsolute_fn && fnPathFix )
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

	if ( !fnPathFix )
		return;

	CSphString sBinlogPath ( hSearchd.GetStr( szBinlogKey ) );
	fnPathFix ( sBinlogPath );
	hSearchd.Delete ( szBinlogKey );
	hSearchd.AddEntry ( szBinlogKey, sBinlogPath.cstr() );
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
					explicit FilenameBuilder_c ( CSphString sIndex );

	CSphString		GetFullPath ( const CSphString & sName ) const final;

private:
	const CSphString		m_sIndex;
};


FilenameBuilder_c::FilenameBuilder_c ( CSphString sIndex )
	: m_sIndex ( std::move ( sIndex ) )
{}


CSphString FilenameBuilder_c::GetFullPath ( const CSphString & sName ) const
{
	if ( !IsConfigless() || !sName.Length() )
		return sName;

	CSphString sPath = GetPathForNewIndex ( m_sIndex );

	StringBuilder_c sNewValue {" "};
	StringBuilder_c sTmp;

	// we assume that path has been stripped before
	StrVec_t dValues = sphSplit ( sName.cstr(), sName.Length(), " \t," );
	for ( auto & i : dValues )
	{
		if ( !i.Length() )
			continue;

		sTmp.Clear();
		sTmp.Appendf ( "%s/%s", sPath.cstr(), i.Trim().cstr() );
		sNewValue << RealPath ( sTmp.cstr() );
	}

	return (CSphString)sNewValue;
}


//////////////////////////////////////////////////////////////////////////
// parse and set cluster options into hash from single string
void ClusterOptions_t::Parse ( const CSphString & sOptions )
{
	if ( !sOptions.IsEmpty() )
		sph::ParseKeyValues ( sOptions.cstr(), [this] ( CSphString&& sIdent, const CSphString& sValue ) {	m_hOptions.Add ( sValue, std::move(sIdent) ); }, ";" );
}

// get string of cluster options with semicolon delimiter
CSphString ClusterOptions_t::AsStr () const
{
	StringBuilder_c tBuf ( ";" );
	for ( const auto& tOpt : m_hOptions )
		tBuf.Sprintf ( "%s=%s", tOpt.first.cstr(), tOpt.second.cstr() );
	return (CSphString)tBuf;
}

bool ClusterOptions_t::IsEmpty() const noexcept
{
	return m_hOptions.IsEmpty();
}

//////////////////////////////////////////////////////////////////////////

bool ClusterDesc_t::Parse ( const bson::Bson_c& tBson, const CSphString& sName, CSphString& sWarning )
{
	using namespace bson;
	if ( sName.IsEmpty() )
		return TlsMsg::Err ( "empty cluster name" );

	m_sName = sName;

	m_dClusterNodes.Reset();
	Bson_c tNodes { tBson.ChildByName ( "nodes" ) };
	if ( tNodes.IsString() ) // old config - all nodes in one line, ','-separated
		sphSplit ( m_dClusterNodes, String ( tNodes ).cstr(), "," );
	else
		tNodes.ForEach ( [this] ( const NodeHandle_t& tNode ) { m_dClusterNodes.Add ( String ( tNode ) ); } );

	m_tOptions.m_hOptions.Reset();
	Bson_c tOptions { tBson.ChildByName ( "options" ) };
	if ( tOptions.IsString() ) // old config - all options in one line, need to be parsed
		m_tOptions.Parse ( String ( tOptions ) );
	else
		tOptions.ForEach ( [this] ( CSphString&& sName, const NodeHandle_t& tNode ) { m_tOptions.m_hOptions.Add ( String ( tNode ), sName ); } );

	int iItem = 0;
	Bson_c ( tBson.ChildByName ( "indexes" ) ).ForEach ( [this,&iItem,&sWarning] ( const NodeHandle_t& tNode ) {
		if ( IsString(tNode ) )
			m_hIndexes.Add ( String ( tNode ));
		else
			sWarning.SetSprintf ( "table %d: name '%s' should be a string, skipped", iItem, m_sName.cstr() );
		++iItem;
	} );

	m_sPath = String ( tBson.ChildByName ( "path" ) );
	return true;
}

void operator<< ( JsonEscapedBuilder& tOut, const ClusterOptions_t& tOptions )
{
	auto _ = tOut.ObjectW();
	for ( const auto& tOpt : tOptions.m_hOptions )
	{
		if ( tOpt.first == "pc.bootstrap" ) // we always skip pc.bootstrap when store value into json
			continue;

		tOut.NamedString ( tOpt.first.cstr(), tOpt.second );
	}
}

void ClusterDesc_t::Save ( JsonEscapedBuilder& tOut ) const
{
	tOut.Named ( m_sName.cstr() );
	auto _0 = tOut.ObjectW();
	if ( !m_dClusterNodes.IsEmpty() )
	{
		tOut.Named ( "nodes" );
		auto _1 = tOut.Array();
		for_each ( m_dClusterNodes, [&tOut] ( const auto& sNode ) { tOut.String ( sNode ); } );
	}
	if ( !m_tOptions.IsEmpty() )
		tOut.NamedVal ( "options", m_tOptions );
	if ( !m_hIndexes.IsEmpty() )
	{
		tOut.Named ( "indexes" );
		auto _1 = tOut.Array();
		for_each ( m_hIndexes, [&tOut] ( const auto& tIndex ) { tOut.String ( tIndex.first ); } );
	}
	tOut.NamedStringNonEmpty ( "path", m_sPath );
}

//////////////////////////////////////////////////////////////////////////

bool IndexDescDistr_t::Parse ( const bson::Bson_c& tBson, CSphString& sWarning )
{
	using namespace bson;
	Bson_c ( tBson.ChildByName ( "locals" ) ).ForEach ( [this,&sWarning] ( const NodeHandle_t& tNode ) {
		if ( !IsString(tNode) )
		{
			sWarning = "lists of local tables must only contain strings, skipped";
			return;
		}
		m_dLocals.Add ( String ( tNode ) );
	} );
	Bson_c ( tBson.ChildByName ( "agents" ) ).ForEach ( [this, &sWarning] ( const NodeHandle_t& tNode ) {
		AgentConfigDesc_t& tNew = m_dAgents.Add();
		auto tBson = bson::Bson_c { tNode };
		tNew.m_sConfig = String ( tBson.ChildByName ( "config" ) );
		tNew.m_bBlackhole = Bool ( tBson.ChildByName ( "blackhole" ) );
		tNew.m_bPersistent = Bool ( tBson.ChildByName ( "persistent" ) );
	} );
	m_iAgentConnectTimeout = Int ( tBson.ChildByName( "agent_connect_timeout" ));
	m_iAgentQueryTimeout = Int ( tBson.ChildByName ( "agent_query_timeout" ) );
	m_iAgentRetryCount = Int ( tBson.ChildByName ( "agent_retry_count" ) );
	m_bDivideRemoteRanges = Bool ( tBson.ChildByName ( "divide_remote_ranges" ) );
	m_sHaStrategy = String ( tBson.ChildByName ( "ha_strategy" ), {} );
	return true;
}

void operator<< ( JsonEscapedBuilder& tOut, const AgentConfigDesc_t& tAgent )
{
	auto _ = tOut.ObjectW();
	tOut.NamedString ( "config", tAgent.m_sConfig );
	tOut.NamedVal ( "blackhole", tAgent.m_bBlackhole );
	tOut.NamedVal ( "persistent", tAgent.m_bPersistent );
}

void IndexDescDistr_t::Save ( JsonEscapedBuilder& tOut ) const
{
	auto _0 = tOut.ObjectW();
	tOut.NamedString ( "type", GetIndexTypeName ( IndexType_e::DISTR ) );
	if ( !m_dLocals.IsEmpty() )
	{
		tOut.Named ( "locals" );
		auto _ = tOut.Array();
		for_each ( m_dLocals, [&tOut] ( const auto& sNode ) { tOut.String ( sNode ); } );
	}
	if ( !m_dAgents.IsEmpty() )
	{
		tOut.Named ( "agents" );
		auto _ = tOut.Array();
		for_each ( m_dAgents, [&tOut] ( const auto& sNode ) { tOut << sNode; } );
	}

	tOut.NamedValNonDefault ( "agent_connect_timeout", m_iAgentConnectTimeout, 0 );
	tOut.NamedValNonDefault ( "agent_query_timeout", m_iAgentQueryTimeout, 0 );
	tOut.NamedValNonDefault ( "agent_retry_count", m_iAgentRetryCount, 0 );
	tOut.NamedVal ( "divide_remote_ranges", m_bDivideRemoteRanges );
	tOut.NamedStringNonDefault ( "ha_strategy", m_sHaStrategy, {} );
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
	if ( m_iAgentConnectTimeout>0 )
		hIndex.AddEntry ( "agent_connect_timeout",	sTmp.SetSprintf ( "%d", m_iAgentConnectTimeout ).cstr() );
	if ( m_iAgentQueryTimeout>0 )
		hIndex.AddEntry ( "agent_query_timeout",	sTmp.SetSprintf ( "%d", m_iAgentQueryTimeout ).cstr() );
	if ( m_iAgentRetryCount > 0 )
		hIndex.AddEntry ( "agent_retry_count",		sTmp.SetSprintf ( "%d", m_iAgentRetryCount ).cstr() );

	hIndex.AddEntry ( "divide_remote_ranges",	m_bDivideRemoteRanges ? "1" : "0" );

	if ( !m_sHaStrategy.IsEmpty() )
		hIndex.AddEntry ( "ha_strategy",		m_sHaStrategy.cstr() );
}

//////////////////////////////////////////////////////////////////////////

bool IndexDesc_t::Parse ( const bson::Bson_c& tBson, const CSphString& sName, CSphString& sWarning )
{
	using namespace bson;
	if ( sName.IsEmpty() )
		return TlsMsg::Err ( "empty table name" );

	m_sName = sName;

	CSphString sType = String ( tBson.ChildByName ( "type" ) );
	m_eType = TypeOfIndexConfig ( sType );
	if ( m_eType == IndexType_e::ERROR_ )
		return TlsMsg::Err ( "type '%s' is invalid", sType.cstr() );

	if ( m_eType != IndexType_e::DISTR )
	{
		m_sPath = String ( tBson.ChildByName ( "path" ) );
		MakeRelativePath ( m_sPath );
		return true;
	}

	bool bParseOk = m_tDistr.Parse ( tBson, sWarning );
	if ( TlsMsg::HasErr() )
		TlsMsg::Err ( "table %s: %s", m_sName.cstr(), TlsMsg::szError() );

	if ( !sWarning.IsEmpty() )
		sWarning.SetSprintf ( "table %s: %s", m_sName.cstr(), sWarning.cstr() );

	return bParseOk;
}

void IndexDesc_t::Save ( JsonEscapedBuilder& tOut ) const
{
	tOut.Named ( m_sName.cstr() );
	if ( m_eType == IndexType_e::DISTR )
		return m_tDistr.Save ( tOut );

	auto _ = tOut.ObjectW();
	tOut.NamedString ("type", GetIndexTypeName ( m_eType ) );
	CSphString sPath = m_sPath;
	tOut.NamedString ( "path", StripPath ( sPath ) );
}


void IndexDesc_t::Save ( CSphConfigSection & hIndex ) const
{
	hIndex.Add ( CSphVariant ( GetIndexTypeName ( m_eType ) ), "type" );

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
static bool ConfigRead ( const CSphString& sConfigPath, CSphVector<ClusterDesc_t>& dClusters, CSphVector<IndexDesc_t>& dIndexes, CSphString& sError )
{
	if ( !sphIsReadable ( sConfigPath, nullptr ) )
		return true;

	CSphAutoreader tConfigFile;
	if ( !tConfigFile.Open ( sConfigPath, sError ) )
		return false;

	int iSize = (int)tConfigFile.GetFilesize();
	if ( !iSize )
		return true;

	CSphFixedVector<char> dJsonText ( iSize + 2 );
	auto iRead = (int64_t)sphReadThrottled ( tConfigFile.GetFD(), dJsonText.begin(), iSize );
	if ( iRead != iSize )
		return false;

	if ( tConfigFile.GetErrorFlag() )
	{
		sError = tConfigFile.GetErrorMessage();
		return false;
	}

	decltype ( dJsonText )::POLICY_T::Zero ( dJsonText.begin() + iSize, 2 ); // safe gap

	CSphVector<BYTE> dBson;
	if ( !sphJsonParse ( dBson, dJsonText.begin(), false, false, false, sError ) )
		return false;

	using namespace bson;
	Bson_c tBson ( dBson );
	if ( tBson.IsEmpty() || !tBson.IsAssoc() )
	{
		sError = "Something wrong read from json config - it is either empty, either not root object.";
		return false;
	}

	LoadCompatHttp ( (const char*)dJsonText.Begin() );

	Bson_c ( tBson.ChildByName ( "indexes" ) ).ForEach ( [&dIndexes] ( CSphString&& sName, const NodeHandle_t& tNode ) {

		Bson_c tNodeBson { tNode };

		IndexDesc_t tIndex;
		CSphString sWarning;
		if ( !tIndex.Parse ( tNodeBson, sName, sWarning ) )
		{
			sphWarning ( "table '%s'(%d) error: %s", sName.cstr(), dIndexes.GetLength(), TlsMsg::szError() );
			return;
		}

		if ( !sWarning.IsEmpty() )
			sphWarning ( "table '%s'(%d) warning: %s", sName.cstr(), dIndexes.GetLength(), sWarning.cstr() );

		int iExists = dIndexes.GetFirst ( [&] ( const IndexDesc_t& tItem ) {
			return ( tItem.m_sName == tIndex.m_sName || ( ( tItem.m_eType == IndexType_e::PLAIN || tItem.m_eType == IndexType_e::RT ) && tItem.m_sPath == tIndex.m_sPath ) );
		} );

		if ( iExists != -1 )
		{
			const IndexDesc_t& tItem = dIndexes[iExists];
			sphWarning ( "table with the same %s already exists: %s, %s, SKIPPED", ( tItem.m_sName == tIndex.m_sName ? "name" : "path" ), tIndex.m_sName.scstr(), tIndex.m_sPath.scstr() );
			return;
		}

		dIndexes.Add ( tIndex );
	} );


	// check clusters
	Bson_c ( tBson.ChildByName ( "clusters" ) ).ForEach ( [&dClusters] ( CSphString&& sName, const NodeHandle_t& tNode ) {

		Bson_c tNodeBson { tNode };
		ClusterDesc_t tCluster;
		CSphString sClusterWarning;
		bool bParsed = tCluster.Parse ( tNodeBson, sName, sClusterWarning );

		if ( !sClusterWarning.IsEmpty() )
			sphWarning ( "cluster '%s': %s", tCluster.m_sName.cstr(), sClusterWarning.cstr() );

		if ( !bParsed )
		{
			sphWarning ( "cluster '%s': disabled at JSON config, %s", tCluster.m_sName.cstr(), TlsMsg::szError() );
			return;
		}

		int iExists = dClusters.GetFirst ( [&] ( const ClusterDesc_t& tItem ) { return ( tItem.m_sName == tCluster.m_sName || tItem.m_sPath == tCluster.m_sPath ); } );
		if ( iExists != -1 )
		{
			const ClusterDesc_t& tItem = dClusters[iExists];
			sphWarning ( "cluster with the same %s already exists: %s, %s, SKIPPED", ( tItem.m_sName == tCluster.m_sName ? "name" : "path" ), tCluster.m_sName.scstr(), tCluster.m_sPath.scstr() );
			return;
		}

		dClusters.Add ( tCluster );
	} );

	sphLogDebug ( "config loaded, tables %d, clusters %d", dIndexes.GetLength(), dClusters.GetLength() );
	return true;
}

static bool ConfigWrite ( const CSphString & sConfigPath, const CSphVector<ClusterDesc_t> & dClusters, const CSphVector<IndexDesc_t> & dIndexes, CSphString & sError )
{
	JsonEscapedBuilder tOut;
	tOut.ObjectWBlock();
	tOut.Named ( "clusters" );
	{
		auto _ = tOut.Object();
		for_each ( dClusters, [&tOut] ( const auto& tCluster ) { tCluster.Save ( tOut ); } );
	}
	tOut.Named ( "indexes" );
	{
		auto _ = tOut.Object();
		for_each ( dIndexes, [&tOut] ( const auto& tIndex ) { tIndex.Save ( tOut ); } );
	}
	SaveCompatHttp ( tOut );

	tOut.FinishBlocks();

	CSphString sNew, sOld;
	auto & sCur = sConfigPath;
	sNew.SetSprintf ( "%s.new", sCur.cstr() );
	sOld.SetSprintf ( "%s.old", sCur.cstr() );

	CSphWriter tConfigFile;
	if ( !tConfigFile.OpenFile ( sNew, sError ) )
		return false;

	tConfigFile.PutString ( (Str_t)tOut );
	assert ( bson::ValidateJson ( tOut.cstr() ) );
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

	sphLogDebug ( "config saved, tables %d, clusters %d", dIndexes.GetLength(), dClusters.GetLength() );

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
			sphWarning ( "table '%s': %s", tIndex.m_sName.cstr(), i.cstr() );

		if ( eAdd==ADD_ERROR )
			sphWarning ( "table '%s': %s - NOT SERVING", tIndex.m_sName.cstr(), sError.cstr() );
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
	for_each ( g_dCfgIndexes, [&hConfigLocal] ( IndexDesc_t& tDesc ) { if (tDesc.m_eType!=IndexType_e::DISTR) hConfigLocal.Add ( &tDesc, tDesc.m_sName ); } );
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
		tIndex.m_tDistr = GetDistributedDesc ( *tIt.second );
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

	if ( !sphDirExists ( g_sDataDir.cstr(), &sError ) )
	{
		sError.SetSprintf ( "%s; make sure it is accessible or remove data_dir from the config file", sError.cstr() );
		return false;
	}

	if ( hConf.Exists("index") )
	{
		sError.SetSprintf ( "'data_dir' cannot be mixed with table declarations in '%s'", sConfigFile.cstr() );
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
	// reset index and cluster descs on restart (watchdog resurrect)
	g_dCfgClusters.Reset();
	g_dCfgIndexes.Reset();
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

	if ( !ReplicationEnabled() && !IsConfigless() )
		return true;


	// save clusters and their indexes into JSON config on daemon shutdown
	auto dClusters = ReplicationCollectClusters ();

	CSphVector<IndexDesc_t> dIndexes;
	CollectLocalIndexesInt ( dIndexes );
	CollectDistIndexesInt ( dIndexes );

	if ( !ConfigWrite ( g_sConfigPath, dClusters, dIndexes, sError ) )
	{
		sphWarning ( "%s", TlsMsg::szError() );
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

using OptInt_t = std::optional<int>;
using CopiedFiles = SmallStringHash_T<CSphSavedFile>;

static bool CopyExternalFile ( const CSphString & sSrcPath, const CSphString & sDstPath, const char * sDstName, OptInt_t iPostfix, OptInt_t iChunk, CSphSavedFile & tExternal, CopiedFiles & hCopied, CSphString & sError )
{
	CSphString sFromFile = tExternal.m_sFilename;
	const CSphSavedFile * pDstExternal = hCopied ( sFromFile );
	if ( pDstExternal )
	{
		tExternal = *pDstExternal;
		return true;
	}

	assert ( sDstName && !sSrcPath.IsEmpty() && !sDstPath.IsEmpty() && !tExternal.m_sFilename.IsEmpty() );

	StringBuilder_c sFile;
	sFile << sDstName;
	if ( iPostfix.has_value() )
		sFile.Appendf ( "_chunk%d", iPostfix.value() );
	if ( iChunk.has_value() )
		sFile.Appendf ( "_%d", iChunk.value() );
	sFile << ".txt";

	CSphString sDst;
	sDst.SetSprintf ( "%s%s", sDstPath.cstr(), sFile.cstr() );

	CSphString sSrc;
	if ( !IsPathAbsolute ( tExternal.m_sFilename ) )
		sSrc.SetSprintf ( "%s%s", sSrcPath.cstr(), tExternal.m_sFilename.cstr() );
	else
		sSrc = tExternal.m_sFilename;

	if ( !CopyFile ( sSrc.cstr(), sDst.cstr(), sError ) )
		return false;

	if ( !tExternal.Collect ( sDst.cstr(), &sError ) )
		return false;

	tExternal.m_sFilename = sFile.cstr();
	hCopied.Add ( tExternal, sFromFile );
	return true;
}

static JsonObj_c DumpFileInfoWoName ( const CSphSavedFile & tInfo )
{
	JsonObj_c tInfoDump;
	tInfoDump.AddUint ( "size", tInfo.m_uSize );
	tInfoDump.AddUint ( "ctime", tInfo.m_uCTime );
	tInfoDump.AddUint ( "mtime", tInfo.m_uMTime );
	tInfoDump.AddUint ( "crc32", tInfo.m_uCRC32 );
	return tInfoDump;
}


static JsonObj_c DumpFileInfo ( const CSphSavedFile & tInfo )
{
	JsonObj_c tFileDump;
	tFileDump.AddStr ( "name", tInfo.m_sFilename );
	JsonObj_c tInfoDump;
	tInfoDump.AddUint ( "size", tInfo.m_uSize );
	tInfoDump.AddUint ( "ctime", tInfo.m_uCTime );
	tInfoDump.AddUint ( "mtime", tInfo.m_uMTime );
	tInfoDump.AddUint ( "crc32", tInfo.m_uCRC32 );
	tFileDump.AddItem ( "info", tInfoDump );
	return tFileDump;
}

static bool CopyExceptions ( const CSphString & sSrcPath, const CSphString & sDstPath, OptInt_t iPostfix, JsonObj_c & tTokSettings, CopiedFiles & hCopied, CSphString & sError )
{
	assert ( tTokSettings && tTokSettings.HasItem( "synonyms_file" ) && tTokSettings.GetItem( "synonyms_file" ).IsStr() );

	CSphSavedFile tExceptions;
	tExceptions.m_sFilename = tTokSettings.GetItem( "synonyms_file" ).StrVal();
	if ( !CopyExternalFile ( sSrcPath, sDstPath, "exceptions", iPostfix, std::nullopt, tExceptions, hCopied, sError ) )
		return false;

	tTokSettings.DelItem ( "synonyms_file" );
	tTokSettings.DelItem ( "syn_file_info" );
	tTokSettings.AddStr ( "synonyms_file", tExceptions.m_sFilename );
	JsonObj_c tFileInfo = DumpFileInfoWoName ( tExceptions );
	tTokSettings.AddItem ( "syn_file_info", tFileInfo );
	return true;
}

// copy multiple externals from the array object
static std::optional<JsonObj_c> CopyExternalFilesArray ( const CSphString & sSrcPath, const CSphString & sDstPath, OptInt_t iPostfix, const JsonObj_c & tInfos, const char * sItemName, CopiedFiles & hCopied, CSphString & sError)
{
	JsonObj_c tResArray ( true );
	for ( int i=0; i<tInfos.Size(); i++ )
	{
		JsonObj_c tItemName = tInfos[i].GetItem ( "name" );
		if ( !tItemName || !tItemName.IsStr() )
		{
			sError.SetSprintf ( "invalid %s", sItemName );
			return std::nullopt;
		}

		CSphSavedFile tFileInfo;
		tFileInfo.m_sFilename = tItemName.StrVal();
		if ( !CopyExternalFile ( sSrcPath, sDstPath, sItemName, iPostfix, i, tFileInfo, hCopied, sError ) )
			return std::nullopt;

		JsonObj_c tFileItem = DumpFileInfo ( tFileInfo );
		tResArray.AddItem ( tFileItem );
	}

	return tResArray;
}

static bool CopyWordforms ( const CSphString & sSrcPath, const CSphString & sDstPath, OptInt_t iPostfix, JsonObj_c & tDictSettings, CopiedFiles & hCopied, CSphString & sError )
{
	assert ( tDictSettings && tDictSettings.HasItem( "wordforms_file_infos" ) && tDictSettings.GetItem( "wordforms_file_infos" ).IsArray() && tDictSettings.GetItem( "wordforms_file_infos" ).Size()>0 );

	auto tWfDst = CopyExternalFilesArray ( sSrcPath, sDstPath, iPostfix, tDictSettings.GetItem( "wordforms_file_infos" ), "wordforms", hCopied, sError );
	if ( !tWfDst.has_value() )
		return false;

	tDictSettings.DelItem ( "wordforms_file_infos" );
	tDictSettings.AddItem ( "wordforms_file_infos", tWfDst.value() );

	return true;
}

static bool CopyStopwords ( const CSphString & sSrcPath, const CSphString & sDstPath, OptInt_t iPostfix, JsonObj_c & tDictSettings, CopiedFiles & hCopied, CSphString & sError )
{
	assert ( tDictSettings );

	// could by just stopwords without stopwords_file_infos
	if ( tDictSettings && !tDictSettings.HasItem( "stopwords_file_infos" ) )
	{
		assert ( tDictSettings.HasItem( "stopwords" ) && tDictSettings.GetItem( "stopwords" ).IsStr() );
		CSphString sStopwords = tDictSettings.GetItem( "stopwords" ).StrVal();
		StrVec_t dStops = sphSplit ( sStopwords.cstr(), sStopwords.Length(), " \t," );

		JsonObj_c tStopwordsInfo ( true );
		for ( const CSphString & sFile : dStops )
		{
			JsonObj_c tInfo;
			tInfo.AddStr ( "name", sFile );
			tStopwordsInfo.AddItem ( tInfo );
		};
		tDictSettings.AddItem ( "stopwords_file_infos", tStopwordsInfo );
	}

	assert ( tDictSettings.HasItem( "stopwords_file_infos" ) && tDictSettings.GetItem( "stopwords_file_infos" ).IsArray() && tDictSettings.GetItem( "stopwords_file_infos" ).Size()>0 );

	auto tStopsDst = CopyExternalFilesArray ( sSrcPath, sDstPath, iPostfix, tDictSettings.GetItem( "stopwords_file_infos" ), "stopwords", hCopied, sError );
	if ( !tStopsDst.has_value() )
		return false;

	StringBuilder_c sStopwords ( " " );
	for ( const JsonObj_c tFile : tStopsDst.value() )
	{
		assert ( tFile.HasItem ( "name" ) && tFile.GetItem ( "name" ).IsStr() );
		sStopwords << tFile.GetItem ( "name" ).SzVal();
	}
	tDictSettings.DelItem ( "stopwords" );
	tDictSettings.AddStr ( "stopwords", sStopwords.cstr() );
	tDictSettings.DelItem ( "stopwords_file_infos" );
	tDictSettings.AddItem ( "stopwords_file_infos", tStopsDst.value() );

	return true;
}


static bool CopyJiebaDict ( const CSphString & sSrcPath, const CSphString & sDstPath, OptInt_t iPostfix, JsonObj_c & tHeader, CopiedFiles & hCopied, CSphString & sError )
{
	const char * szDict = "jieba_user_dict_path";
	assert ( tHeader && tHeader.HasItem(szDict) && tHeader.GetItem(szDict).IsStr() );

	CSphSavedFile tJiebaDict;
	tJiebaDict.m_sFilename = tHeader.GetItem(szDict).StrVal();
	if ( !CopyExternalFile ( sSrcPath, sDstPath, szDict, iPostfix, std::nullopt, tJiebaDict, hCopied, sError ) )
		return false;

	tHeader.DelItem(szDict);
	tHeader.AddStr ( szDict, tJiebaDict.m_sFilename );
	return true;
}


static std::optional<JsonObj_c> ReadJsonHeader ( const CSphString & sFilename, CSphString & sError )
{
	CSphAutofile tFile;
	if ( tFile.Open ( sFilename, SPH_O_READ, sError )<0 )
		return std::nullopt;

	int64_t iSize = tFile.GetSize();
	CSphFixedVector<char> sMeta { iSize + 2 }; // and zero-gap at the end
	if ( !tFile.Read ( sMeta.Begin(), iSize, sError ) )
		return std::nullopt;

	sMeta[iSize] = sMeta[iSize + 1] = '\0';
	JsonObj_c tMeta ( sMeta );
	if ( tMeta.GetError ( sMeta.Begin(), iSize, sError ) )
		return std::nullopt;

	return tMeta;
}

static bool WriteJsonHeader ( const CSphString & sFilename, const JsonObj_c & tMeta, CSphString & sError )
{
	CSphAutofile tFile;
	if ( tFile.Open ( sFilename, SPH_O_NEW, sError, true )<0 )
		return false;

	CSphString sDump = tMeta.AsString ( true );
	if ( !sphWriteThrottled ( tFile.GetFD(), sDump.cstr(), sDump.Length(), sFilename.cstr(), sError ) )
		return false;

	tFile.SetPersistent();
	return true;
}

// check for any stopwords, wordforms or exceptions and copy all avaliable
static bool CopyExternalsFromHeader ( const CSphString & sSrcPath, const CSphString & sDstIndex, OptInt_t iPostfix, JsonObj_c & tHeader, CopiedFiles & hCopied, CSphString & sError )
{
	JsonObj_c tTokSettings = tHeader.GetItem ( "tokenizer_settings" );
	JsonObj_c tDictSettings = tHeader.GetItem ( "dictionary_settings" );

	bool bHasExceptions = tTokSettings && tTokSettings.HasItem( "synonyms_file" ) && tTokSettings.GetItem( "synonyms_file" ).IsStr();

	bool bHasStopwords = false;
	if ( tDictSettings && tDictSettings.HasItem( "stopwords_file_infos" ) )
	{
		JsonObj_c tStopwords = tDictSettings.GetItem( "stopwords_file_infos" );
		bHasStopwords = ( tStopwords.IsArray() && tStopwords.Size()>0 );
	}

	if ( !bHasStopwords && tDictSettings.HasItem( "stopwords" ) )
		bHasStopwords = tDictSettings.GetItem( "stopwords" ).IsStr();

	bool bHasWordforms = false;
	if ( tDictSettings && tDictSettings.HasItem( "wordforms_file_infos" ) )
	{
		JsonObj_c tWordforms = tDictSettings.GetItem( "wordforms_file_infos" );
		bHasWordforms = ( tWordforms.IsArray() && tWordforms.Size()>0 );
	}

	bool bHasJiebaDict = tHeader.HasItem("jieba_user_dict_path");

	if ( !bHasExceptions && !bHasStopwords && !bHasWordforms && !bHasJiebaDict )
		return true;

	CSphString sDstPath = GetPathOnly ( sDstIndex );

	if ( bHasExceptions && !CopyExceptions ( GetPathOnly ( sSrcPath ), sDstPath, iPostfix, tTokSettings, hCopied, sError ) )
		return false;

	if ( bHasStopwords && !CopyStopwords ( GetPathOnly ( sSrcPath ), sDstPath, iPostfix, tDictSettings, hCopied, sError ) )
		return false;

	if ( bHasWordforms && !CopyWordforms ( GetPathOnly ( sSrcPath ), sDstPath, iPostfix, tDictSettings, hCopied, sError ) )
		return false;

	if ( bHasJiebaDict && !CopyJiebaDict ( GetPathOnly ( sSrcPath ), sDstPath, iPostfix, tHeader, hCopied, sError ) )
		return false;

	return true;
}

// copy external from the either .meta or .sph
static bool CopyExternal ( const CSphString & sSrcPath, const CSphString & sDstIndex, const CSphString & sHeaderName, OptInt_t iPostfix, CopiedFiles & hCopied, CSphString & sError )
{
	std::optional<JsonObj_c> tRes = ReadJsonHeader ( sHeaderName, sError );
	if ( !tRes )
		return false;

	JsonObj_c & tMeta = tRes.value();

	// copy external from the RAM part
	if ( !CopyExternalsFromHeader ( sSrcPath, sDstIndex, iPostfix, tMeta, hCopied, sError ) )
		return false;

	// copy external from the disk chunks
	if ( tMeta.HasItem ( "chunk_names" ) && tMeta.GetItem ( "chunk_names" ).IsArray() && tMeta.GetItem ( "chunk_names" ).Size()>0 )
	{
		JsonObj_c tChunks = tMeta.GetItem ( "chunk_names" );
		for ( const JsonObj_c & tChunk : tChunks )
		{
			if ( !tChunk.IsInt() )
			{
				sError.SetSprintf ( "invalid chunk: %s", tChunk.AsString().cstr() );
				return false;
			}

			int iChunk = (int)tChunk.IntVal();
			CSphString sChunkName;
			sChunkName.SetSprintf ( "%s.%d.sph", sDstIndex.cstr(), iChunk );
			if ( !CopyExternal ( sSrcPath, sDstIndex, sChunkName, iChunk, hCopied, sError ) )
				return false;
		}
	}

	if ( !WriteJsonHeader ( sHeaderName, tMeta, sError ) )
		return false;

	return true;
}

// remove index_id from the .meta to prevent duplicate of active indexes
static bool CleanupHeader ( const CSphString & sHeaderName, bool & bPQ, CSphString & sError )
{
	std::optional<JsonObj_c> tRes = ReadJsonHeader ( sHeaderName, sError );
	if ( !tRes )
		return false;

	JsonObj_c & tMeta = tRes.value();
	if ( tMeta.HasItem ( "index_id" ) )
		tMeta.DelItem ( "index_id" );

	bPQ = tMeta.HasItem ( "pqs" );

	if ( !WriteJsonHeader ( sHeaderName, tMeta, sError ) )
		return false;

	return true;
}

bool CopyIndexFiles ( const CSphString & sIndex, const CSphString & sPathToIndex, bool & bPQ, StrVec_t & dWarnings, CSphString & sError )
{
	CSphString sPath, sNewIndexPath;
	if ( !PrepareDirForNewIndex ( sPath, sNewIndexPath, sIndex, sError ) )
		return false;

	StrVec_t dWipe;
	CopiedFiles hCopied;
	auto tCleanup = AtScopeExit ( [&dWipe, &hCopied, &sNewIndexPath]
	{
			dWipe.for_each ( [] ( const auto& i ) { unlink ( i.cstr() ); } );
			for  ( const auto & tItem : hCopied )
			{
				CSphString sName;
				sName.SetSprintf ( "%s%s", sNewIndexPath.cstr(), tItem.second.m_sFilename.cstr() );
				unlink ( sName.cstr() );
			};
	});

	CSphString sFind;
	sFind.SetSprintf ( "%s.*", sPathToIndex.cstr() );
	StrVec_t dFoundFiles = FindFiles ( sFind.cstr(), false );

	// checks for source index
	if ( !dFoundFiles.GetLength() )
	{
		sError = "no table files found";
		return false;
	}

	if ( !dFoundFiles.any_of ( [] ( const CSphString & sFile ) { return sFile.Ends ( ".meta" ); } ) )
	{
		sError.SetSprintf ( "missing %s.meta table file", sPathToIndex.cstr() );
		return false;
	}

	for ( const auto & i : dFoundFiles )
	{
		CSphString sDest;
		const char * sExt = GetExtension(i);
		if ( !sExt || StrEq ( sExt, "spl" ) || StrEq ( sExt, "lock" ) )
			continue;

		sDest.SetSprintf ( "%s.%s", sNewIndexPath.cstr(), sExt );
		if ( !CopyFile ( i, sDest, sError ) )
			return false;

		dWipe.Add(sDest);
	}

	CSphString sMetaName;
	sMetaName.SetSprintf ( "%s.meta", sNewIndexPath.cstr() );
	if ( !CopyExternal ( sPathToIndex, sNewIndexPath, sMetaName, std::nullopt, hCopied, sError ) )
		return false;

	if ( !CleanupHeader ( sMetaName, bPQ, sError ) )
		return false;

	dWipe.Reset();
	hCopied.Reset();
	return true;
}


static bool CheckCreateTableSettings ( const CreateTableSettings_t & tCreateTable, CSphString & sError )
{
	static const char * dForbidden[] = { "path", "stored_fields", "stored_only_fields", "columnar_attrs", "columnar_no_fast_fetch", "rowwise_attrs", "rt_field", "embedded_limit", "knn", "json_secondary_indexes" };
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
	if ( tDistr.GetAgentConnectTimeoutMs ( true )!=pDefault->GetAgentConnectTimeoutMs ( true ) )
		sRes << sOpt.SetSprintf ( "agent_connect_timeout='%d'", tDistr.GetAgentConnectTimeoutMs ( true ) );

	if ( tDistr.GetAgentQueryTimeoutMs ( true )!=pDefault->GetAgentQueryTimeoutMs ( true ) )
		sRes << sOpt.SetSprintf ( "agent_query_timeout='%d'", tDistr.GetAgentQueryTimeoutMs ( true ) );

	if ( tDistr.m_iAgentRetryCount!=pDefault->m_iAgentRetryCount )
		sRes << sOpt.SetSprintf ( "agent_retry_count='%d'", tDistr.m_iAgentRetryCount );

	if ( tDistr.m_bDivideRemoteRanges!=pDefault->m_bDivideRemoteRanges )
		sRes << sOpt.SetSprintf ( "divide_remote_ranges='%d'", tDistr.m_bDivideRemoteRanges ? 1 : 0 );

	if ( tDistr.m_eHaStrategy!=pDefault->m_eHaStrategy )
		sRes << sOpt.SetSprintf ( "ha_strategy='%s'", HAStrategyToStr ( tDistr.m_eHaStrategy ).cstr() );

	return sRes.cstr();
}


static void DeleteExtraIndexFiles ( CSphIndex * pIndex, const StrVec_t * pExtFiles )
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

	// also remove external files from disk chunks (could be another list of files not same as RT index itself)
	if ( pExtFiles )
	{
		for ( const auto & sName : *pExtFiles )
		{
			if  ( sphIsReadable ( sName ) )
				::unlink ( sName.cstr() );
		}
	}
}


static void DeleteRtIndex ( CSphIndex * pIdx, const StrVec_t * pExtFiles, Handler fnOnDestroy )
{
	assert ( IsConfigless() );
	if ( !pIdx->IsRT() && !pIdx->IsPQ() )
		return;
	auto pRt = static_cast<RtIndex_i*> ( pIdx );
	pRt->IndexDeleted ( std::move ( fnOnDestroy ) );
	DeleteExtraIndexFiles ( pRt, pExtFiles );
}

static void FixupIndexTID ( CSphIndex * pIdx, int64_t iTID )
{
	assert ( IsConfigless () );
	if ( pIdx && ( pIdx->IsRT () || pIdx->IsPQ () ) && pIdx->m_iTID!=-1 )
		pIdx->m_iTID = iTID;
}

static void RemoveAndDeleteRtIndex ( const CSphString& sIndex )
{
	assert ( IsConfigless() );
	cServedIndexRefPtr_c pServed = GetServed ( sIndex );
	if ( !pServed )
		return;

	WIdx_c pIdx { pServed };
	DeleteRtIndex ( pIdx, nullptr, nullptr );
	g_pLocalIndexes->Delete ( sIndex );
}


bool CreateNewIndexConfigless ( const CSphString & sIndex, const CreateTableSettings_t & tCreateTable, StrVec_t & dWarnings, CSphString & sError )
{
	assert ( IsConfigless() );
	if ( tCreateTable.m_bIfNotExists && IndexIsServed ( sIndex ) )
		return true;

	if ( !CheckCreateTableSettings ( tCreateTable, sError ) )
		return false;

	std::unique_ptr<IndexSettingsContainer_i> pSettingsContainer { CreateIndexSettingsContainer() };
	if ( !pSettingsContainer->Populate ( tCreateTable, true ) )
	{
		sError = pSettingsContainer->GetError();
		return false;
	}

	if ( pSettingsContainer->Get ( "type" )!="distributed")
	{
		CSphString sPath, sIndexPath;
		if ( !PrepareDirForNewIndex ( sPath, sIndexPath, sIndex, sError ) )
		{
			if ( HasConfigLocal ( sIndex ) )
				sError.SetSprintf ( "%s (the table may be corrupted, refer to Manticore log)", sError.cstr() );
			return false;
		}

		pSettingsContainer->Add ( "path", sIndexPath );
		if ( !pSettingsContainer->CopyExternalFiles ( sPath, 0 ) )
		{
			sError = pSettingsContainer->GetError();
			return false;
		}
	}

	const CSphConfigSection & hCfg = pSettingsContainer->AsCfg();

	ESphAddIndex eAdd;
	ServedIndexRefPtr_c pDesc;
	std::tie ( eAdd, pDesc ) = AddIndex ( sIndex.cstr(), hCfg, true, true, &dWarnings, sError );
	switch ( eAdd )
	{
	case ADD_ERROR: return false;
	case ADD_NEEDLOAD:
		{
			assert ( pDesc );
			if ( !pDesc->m_sGlobalIDFPath.IsEmpty() && !sph::PrereadGlobalIDF ( pDesc->m_sGlobalIDFPath, sError ) )
				dWarnings.Add ( "global IDF unavailable - IGNORING" );

			FixupIndexTID ( UnlockedHazardIdxFromServed ( *pDesc ), Binlog::LastTidFor ( sIndex ) );
			if ( !PreallocNewIndex ( *pDesc, &hCfg, sIndex.cstr(), dWarnings, sError ) )
			{
				DeleteRtIndex ( UnlockedHazardIdxFromServed ( *pDesc ), nullptr, nullptr );
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
		pSettingsContainer->ResetCleanup();
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


static bool DropDistrIndex ( const CSphString & sIndex, bool bForce, CSphString & sError )
{
	assert ( IsConfigless() );
	auto pDistr = GetDistr(sIndex);
	if ( !pDistr )
	{
		sError.SetSprintf ( "DROP TABLE failed: unknown distributed table '%s'", sIndex.cstr() );
		return false;
	}

	if ( IsDistrTableHasSystem ( *pDistr, bForce ) )
	{
		sError.SetSprintf ( "can not drop table '%s' because it contains system table", sIndex.cstr() );
		return false;
	}


	if ( !pDistr->m_sCluster.IsEmpty() )
	{
		sError.SetSprintf ( "DROP TABLE failed: unable to drop a cluster table '%s'", sIndex.cstr() );
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

static sph::StringSet g_dAllowedExt = { "meta", "ram", "settings", "lock"};

static bool ReportEmptyDir ( const CSphString & sIndexName, CSphString * pMsg )
{
	if ( !pMsg )
		return true;

	CSphString sIndexPath = GetPathForNewIndex ( sIndexName );
	CSphString sSearchPath; 
	sSearchPath.SetSprintf ( "%s/*", sIndexPath.cstr() );

	StrVec_t dFiles = FindFiles ( sSearchPath.cstr(), false );

	// no files in the index dir is ok
	if ( dFiles.IsEmpty() )
		return true;
	
	// some files removed at RT index dtor are ok
	bool bAllAllowed = dFiles.all_of ( [&] ( const CSphString & sFile )
	{
		const char * sExt = GetExtension ( sFile );
		return g_dAllowedExt[sExt];
	});
	if ( bAllAllowed )
		return true;

	StringBuilder_c sFiles ( ", " );
	dFiles.for_each ( [&] ( CSphString & sFile)
	{
		const char * sExt = GetExtension ( sFile );
		if ( !g_dAllowedExt[sExt] )
			sFiles += StripPath ( sFile ).cstr();
	});

	sphWarning ( "index %s directory '%s' is not empty after table drop, clean up files manually: %s", sIndexName.cstr(), sIndexPath.cstr(), sFiles.cstr() );
	pMsg->SetSprintf ( "index %s directory '%s' is not empty after table drop, clean up files manually: %s", sIndexName.cstr(), sIndexPath.cstr(), sFiles.cstr() );

	return false;
}

static bool DropLocalIndexWithCb ( const CSphString & sIndex, CSphString & sError, CSphString * pWarning, Handler fnOnDestroy=nullptr )
{
	assert ( IsConfigless() );
	auto pServed = GetServed(sIndex);
	if ( !pServed )
	{
		sError.SetSprintf ( "DROP TABLE failed: unknown local table '%s'", sIndex.cstr() );
		return false;
	}

	if ( ServedDesc_t::IsCluster ( pServed ) )
	{
		sError.SetSprintf ( "DROP TABLE failed: unable to drop a cluster table '%s'", sIndex.cstr() );
		return false;
	}

	// need to stop all long time write operation at the index as it will be dropped anyway
	{
		RIdx_T<RtIndex_i*> pRt ( pServed );
		pRt->ProhibitSave();
	}

	// scope for index removal on exit
	{
		WIdx_T<RtIndex_i*> pRt { pServed };
		if ( !pRt )
		{
			sError.SetSprintf ( "DROP TABLE failed: unknown local table '%s'", sIndex.cstr() );
			return false;
		}

		// need to collect all external files prior to truncate as disk chunks could have different options for externals
		StrVec_t dIndexFiles;
		StrVec_t dExtFiles;
		pRt->GetIndexFiles ( dIndexFiles, dExtFiles );

		if ( !pRt->Truncate(sError, RtIndex_i::DROP ) )
			return false;

		DeleteRtIndex ( pRt, &dExtFiles, std::move ( fnOnDestroy ) );
		RemoveConfigIndex ( sIndex );
	}

	ReportEmptyDir ( sIndex, pWarning );
	return true;
}

static bool DropLocalIndex ( const CSphString& sIndex, CSphString& sError, CSphString* pWarning )
{
	if ( !IsInsideCoroutine() )
		return DropLocalIndexWithCb ( sIndex, sError, pWarning );

	Coro::Waitable_T<bool> bDestroyed { false };
	if ( !DropLocalIndexWithCb ( sIndex, sError, pWarning, [&bDestroyed] { bDestroyed.SetValueAndNotifyOne ( true ); } ) )
		return false;
	bDestroyed.Wait ( [] ( bool bVal ) { return bVal; } );
	return true;
}


bool DropIndexInt ( const CSphString & sIndex, bool bIfExists, bool bForce, CSphString & sError, CSphString * pWarning )
{
	assert ( IsConfigless() );
	bool bLocal = GetServed ( sIndex );
	bool bDistr = GetDistr ( sIndex );
	if ( bDistr )
	{
		if ( !DropDistrIndex ( sIndex, bForce, sError ) )
			return false;
	}
	else if ( bLocal )
	{
		if ( !DropLocalIndex ( sIndex, sError, pWarning ) )
			return false;
	}
	else
	{
		if ( bIfExists )
			return true;

		sError.SetSprintf ( "DROP TABLE failed: unknown table '%s'", sIndex.cstr() );
		return false;
	}

	// we are unable to roll back the drop at this point
	if ( !SaveConfigInt(sError) )
	{
		sError.SetSprintf ( "DROP TABLE failed for table '%s': %s", sIndex.cstr(), sError.cstr() );
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

IndexDescDistr_t GetDistributedDesc ( const DistributedIndex_t & tDist )
{
	IndexDescDistr_t tIndex;

	tIndex.m_dLocals				= tDist.m_dLocal;
	tIndex.m_iAgentConnectTimeout	= tDist.GetAgentConnectTimeoutMs ( true );
	tIndex.m_iAgentQueryTimeout		= tDist.GetAgentQueryTimeoutMs ( true );
	tIndex.m_iAgentRetryCount		= tDist.m_iAgentRetryCount;
	tIndex.m_bDivideRemoteRanges	= tDist.m_bDivideRemoteRanges;
	tIndex.m_sHaStrategy			= HAStrategyToStr ( tDist.m_eHaStrategy );

	for ( const auto & tAgentIt : tDist.m_dAgents )
	{
		if ( !tAgentIt || !tAgentIt->GetLength() )
			continue;

		AgentConfigDesc_t & tAgent = tIndex.m_dAgents.Add();
		tAgent.m_sConfig		= tAgentIt->GetConfigStr();
		tAgent.m_bBlackhole		= (*tAgentIt)[0].m_bBlackhole;
		tAgent.m_bPersistent	= (*tAgentIt)[0].m_bPersistent;
	}

	return tIndex;
}

static const char * g_sTableNameSystem = "system.";

bool IsDistrTableHasSystem ( const DistributedIndex_t & tDistr, bool bForce )
{
	if ( bForce )
		return false;

	for ( const auto & sName : tDistr.m_dLocal )
	{
		if ( sName.Begins ( g_sTableNameSystem ) )
			return true;
	}

	for ( const auto & pAgent : tDistr.m_dAgents )
	{
		for ( const AgentDesc_t & tDesc : *pAgent )
		{
			if ( strstr ( tDesc.m_sIndexes.cstr(), g_sTableNameSystem )!=nullptr )
				return true;
		}
	}

	return false;
}
