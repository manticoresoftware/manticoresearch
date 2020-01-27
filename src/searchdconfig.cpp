//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
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
#include "searchdreplication.h"
#include "fileutils.h"
#include "sphinxint.h"

// description of clusters and indexes loaded from internal config
static CSphVector<ClusterDesc_t> g_dCfgClusters;
static CSphVector<IndexDesc_t> g_dCfgIndexes;

static CSphString	g_sLogFile;
static CSphString	g_sDataDir;
static CSphString	g_sConfigPath;
static bool			g_bConfigless = false;

static CSphMutex	g_tSaveInProgress;

extern ISphBinlog * g_pBinlog;


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
		sName.SetBinary ( sNameStart, sCur - sNameStart );

		// skip set char and space prior to values
		while ( *sCur && ( sphIsSpace ( *sCur ) || *sCur=='=' ) )
			sCur++;

		// skip all value characters and delimiter
		const char * sValStart = sCur;
		while ( *sCur && !sphIsSpace ( *sCur ) && *sCur!=';' )
			sCur++;

		CSphString sVal;
		sVal.SetBinary ( sValStart, sCur - sValStart );

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
	void * pIt = nullptr;
	while ( m_hOptions.IterateNext ( &pIt ) )
	{
		// skip one time options on save
		if ( bSave && m_hOptions.IterateGetKey ( &pIt )=="pc.bootstrap" )
			continue;

		tBuf.Appendf ( "%s=%s", m_hOptions.IterateGetKey ( &pIt ).cstr(), m_hOptions.IterateGet ( &pIt ).cstr() );
	}

	return tBuf.cstr();
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

bool IndexDesc_t::Parse ( const JsonObj_c & tJson, CSphString & sError )
{
	m_sName = tJson.Name();
	if ( m_sName.IsEmpty() )
	{
		sError = "empty index name";
		return false;
	}

	if ( !tJson.FetchStrItem ( m_sPath, "path", sError ) )
		return false;

	CSphString sType;
	if ( !tJson.FetchStrItem ( sType, "type", sError ) )
		return false;

	m_eType = TypeOfIndexConfig ( sType );
	if ( m_eType==IndexType_e::ERROR_ )
	{
		sError.SetSprintf ( "type '%s' is invalid", sType.cstr() );
		return false;
	}

	if ( !tJson.FetchBoolItem ( m_bFromReplication, "from_replication", sError, true ) )
		return false;

	return true;
}


void IndexDesc_t::Save ( JsonObj_c & tIndexes ) const
{
	JsonObj_c tIdx;
	tIdx.AddStr ( "path", m_sPath );
	tIdx.AddStr ( "type", GetTypeName ( m_eType ) );
	tIdx.AddBool( "from_replication", m_bFromReplication );

	tIndexes.AddItem ( m_sName.cstr(), tIdx );
}

//////////////////////////////////////////////////////////////////////////

// read info about cluster and indexes from manticore.json and validate data
bool ConfigRead ( const CSphString & sConfigPath, CSphVector<ClusterDesc_t> & dClusters, CSphVector<IndexDesc_t> & dIndexes, CSphString & sError )
{
	if ( !sphIsReadable ( sConfigPath, nullptr ) )
		return true;

	CSphAutoreader tConfigFile;
	if ( !tConfigFile.Open ( sConfigPath, sError ) )
		return false;

	int iSize = tConfigFile.GetFilesize();
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
		if ( !tIndex.Parse ( i, sError ) )
		{
			sphWarning ( "index '%s'(%d) error: %s", i.Name(), dIndexes.GetLength(), sError.cstr() );
			return false;
		}

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

	return true;
}


// load indexes got from JSON config on daemon indexes preload (part of ConfigureAndPreload work done here)
void ConfigureAndPreloadInt ( int & iValidIndexes, int & iCounter ) REQUIRES ( MainThread )
{
	for ( const IndexDesc_t & tIndex : g_dCfgIndexes )
	{
		CSphConfigSection hIndex;
		hIndex.Add ( CSphVariant ( tIndex.m_sPath.cstr() ), "path" );
		hIndex.Add ( CSphVariant ( GetTypeName ( tIndex.m_eType ).cstr() ), "type" );
		// dummy
		hIndex.Add ( CSphVariant ( "text" ), "rt_field" );
		hIndex.Add ( CSphVariant ( "gid" ), "rt_attr_uint" );

		ESphAddIndex eAdd = ConfigureAndPreloadIndex ( hIndex, tIndex.m_sName.cstr (), tIndex.m_bFromReplication );
		iValidIndexes += ( eAdd!=ADD_ERROR ? 1 : 0 );
		iCounter += ( eAdd==ADD_DSBLED ? 1 : 0 );
		if ( eAdd==ADD_ERROR )
			sphWarning ( "index '%s': removed from JSON config", tIndex.m_sName.cstr() );
	}
}


// get index list that should be saved into JSON config
static void CollectLocalIndexesInt ( CSphVector<IndexDesc_t> & dIndexes )
{
	for ( RLockedServedIt_c tIt ( g_pLocalIndexes ); tIt.Next (); )
	{
		auto pServed = tIt.Get();
		if ( !pServed )
			continue;

		ServedDescRPtr_c tDesc ( pServed );

		bool bCollectIndex = ( ReplicationIsEnabled() && tDesc->m_bFromReplication ) || IsConfigless();
		if ( !bCollectIndex )
			continue;

		IndexDesc_t & tIndex = dIndexes.Add();
		tIndex.m_sName = tIt.GetName();
		tIndex.m_sPath = tDesc->m_sIndexPath;
		tIndex.m_eType = tDesc->m_eType;
		tIndex.m_bFromReplication = tDesc->m_bFromReplication;
	}
}


static bool SetupConfiglessMode ( const CSphConfig & hConf, const CSphString & sConfigFile, CSphString & sError )
{
	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];

	if ( !hSearchd.Exists("data_dir") )
		return false;

	g_sDataDir = hSearchd["data_dir"].strval();

	if ( !sphDirExists ( g_sDataDir.cstr(), &sError ) )
		return false;

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

	return true;
}


// load data from JSON config on daemon start
void LoadConfigInt ( const CSphConfig & hConf, const CSphString & sConfigFile )
{
	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];

	g_bConfigless = hSearchd.Exists("data_dir");
	if ( g_bConfigless )
	{
		CSphString sError;
		if ( !SetupConfiglessMode ( hConf, sConfigFile, sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			g_bConfigless = false;
		}
	}

	g_sLogFile = hSearchd.GetStr ( "log", "" );

	// node with empty incoming addresses works as GARB - does not affect FC
	// might hung on pushing 1500 transactions
	ReplicationSetIncoming ( hSearchd.GetStr ( "node_address" ) );

	if ( g_sDataDir.IsEmpty() )
		return;

	CSphString sError;

	// check data_dir exists and available
	if ( !CheckPath ( g_sDataDir, true, sError ) )
	{
		sphWarning ( "%s, replication is disabled", sError.cstr() );
		g_sDataDir = "";
		return;
	}

	g_sConfigPath.SetSprintf ( "%s/manticore.json", g_sDataDir.cstr() );
	if ( !ConfigRead ( g_sConfigPath, g_dCfgClusters, g_dCfgIndexes, sError ) )
		sphDie ( "failed to use JSON config %s: %s", g_sConfigPath.cstr(), sError.cstr() );
}


void SaveConfigInt() 
{
	ScopedMutex_t tSaving ( g_tSaveInProgress );

	if ( !ReplicationIsEnabled() && !IsConfigless() )
		return;
	
	CSphVector<ClusterDesc_t> dClusters;

	// save clusters and their indexes into JSON config on daemon shutdown
	if ( ReplicationIsEnabled() )
		ReplicationCollectClusters ( dClusters );

	CSphVector<IndexDesc_t> dIndexes;
	CollectLocalIndexesInt ( dIndexes );

	CSphString sError;
	if ( !ConfigWrite ( g_sConfigPath, dClusters, dIndexes, sError ) )
		sphWarning ( "%s", sError.cstr() );
}

//////////////////////////////////////////////////////////////////////////

static CSphString GetPathForNewIndex ( const CSphString & sIndexName )
{
	CSphString sRes;
	if ( g_sDataDir.Length() && !g_sDataDir.Ends("/") && !g_sDataDir.Ends("\\") )
		sRes.SetSprintf ( "%s/%s", g_sDataDir.cstr(), sIndexName.cstr() );
	else
		sRes.SetSprintf ( "%s%s", g_sDataDir.cstr(), sIndexName.cstr() );

	return sRes;
}


static bool PrepareDirForNewIndex ( CSphString & sPath, const CSphString & sIndexName, bool bBinlogReplay, CSphString & sError )
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
			if ( !bLockFileLeft && !bBinlogReplay )
			{
				sError.SetSprintf ( "directory is not empty: %s", sRes.cstr() );
				return false;
			}
		}
	}
	else
		if ( !MkDir ( sRes.cstr() ) )
		{
			sError.SetSprintf ( "can't create directory: %s", sRes.cstr() );
			return false;
		}

	sRes << "/";
	sRes << sIndexName;
	sPath = sRes.cstr();

	return true;
}


static bool CheckCreateTableSettings ( const CreateTableSettings_t & tCreateTable, CSphString & sError )
{
	static const char * dForbidden[] = { "path", "type", "stored_fields", "rt_field" };

	for ( const auto & i : tCreateTable.m_dOpts )
	{
		for ( const auto & j : dForbidden )
			if ( i.m_sName==j )
			{
				sError.SetSprintf ( "setting not allowed: %s", i.m_sName.cstr() );
				return false;
			}

		for ( int j = 0; j < GetNumRtTypes(); j++ )
			if ( i.m_sName==GetRtType(j).m_szName )
			{
				sError.SetSprintf ( "setting not allowed: %s", i.m_sName.cstr() );
				return false;
			}
	}

	return true;
}


CSphIndex * CreateNewIndexInt ( const CSphString & sIndex, const CreateTableSettings_t & tCreateTable, bool bBinlogReplay, CSphString & sError )
{
	if ( tCreateTable.m_bIfNotExists && g_pLocalIndexes->Contains(sIndex) )
		return ServedDescRPtr_c ( GetServed(sIndex) )->m_pIndex;

	CSphString sPath;
	if ( !PrepareDirForNewIndex ( sPath, sIndex, bBinlogReplay, sError ) )
		return nullptr;

	if ( !CheckCreateTableSettings ( tCreateTable, sError ) )
		return nullptr;

	IndexSettingsContainer_c tSettingsContainer;
	tSettingsContainer.Populate ( tCreateTable );
	tSettingsContainer.Add ( "path", sPath );
	tSettingsContainer.Add ( "type", "rt" );

	const CSphConfigSection & hCfg = tSettingsContainer.AsCfg();

	GuardedHash_c dNotLoadedIndexes;
	ESphAddIndex eAdd = AddIndexMT ( dNotLoadedIndexes, sIndex.cstr(), hCfg );
	assert ( eAdd==ADD_DSBLED || eAdd==ADD_ERROR );
	if ( eAdd!=ADD_DSBLED )
		return nullptr;

	ServedIndexRefPtr_c pServed = GetServed ( sIndex, &dNotLoadedIndexes );
	ServedDescWPtr_c pDesc ( pServed );
	if ( !PreallocNewIndex ( *pDesc, &hCfg, sIndex.cstr() ) )
		return nullptr;

	if ( g_pBinlog && !bBinlogReplay )
		g_pBinlog->BinlogCreateTable ( &pDesc->m_pIndex->m_iTID, sIndex.cstr(), tCreateTable );

	g_pLocalIndexes->AddOrReplace ( pServed, sIndex );

	return pDesc->m_pIndex;
}


bool DropIndexInt ( const CSphString & sIndex, bool bIfExists, bool bBinlogReplay, CSphString & sError )
{
	auto pServed = GetServed(sIndex);
	if ( !pServed )
	{
		if ( bIfExists )
			return true;

		sError.SetSprintf ( "DROP TABLE failed: unknown local index '%s'", sIndex.cstr() );
		return false;
	}

	ServedDescWPtr_c pWptr(pServed); // write-lock

	if ( ServedDesc_t::IsCluster(pWptr) )
	{
		sError.SetSprintf ( "DROP TABLE failed: unable to drop a cluster index '%s'", sIndex.cstr() );
		return false;
	}

	auto * pRt = (RtIndex_i *)pWptr->m_pIndex;
	if ( !pRt )
	{
		sError.SetSprintf ( "DROP TABLE failed: unknown local index '%s'", sIndex.cstr() );
		return false;
	}

	if ( !pRt->Truncate ( bBinlogReplay, sError ) )
		return false;

	pRt->IndexDeleted();

	g_pLocalIndexes->Delete(sIndex);

	if ( g_pBinlog && !bBinlogReplay )
		g_pBinlog->BinlogDropTable ( &pRt->m_iTID, sIndex.cstr(), bIfExists );

	return true;
}