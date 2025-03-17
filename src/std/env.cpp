//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "env.h"

#include "sphinxutils.h"

#ifdef _WIN32
#include "fileutils.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// HELPERS
/////////////////////////////////////////////////////////////////////////////

bool val_from_env ( const char* szEnvName, bool bDefault )
{
	if ( getenv ( szEnvName ) )
		return true;
	return bDefault;
}

int val_from_env ( const char* szEnvName, int iDefault )
{
	int iRes = iDefault;
	const char* szEnv = getenv ( szEnvName );
	if ( szEnv )
	{
		char* szEnd = nullptr;
		iRes = strtol ( szEnv, &szEnd, 10 );
		if ( *szEnd != '\0' )
			sphWarning ( "%s expects to be numeric. %s provided, failed to parse as numeric since %s", szEnvName, szEnv, szEnd );
	}
	return iRes;
}

DWORD dwval_from_env ( const char* szEnvName, DWORD uDefault )
{
	DWORD uRes = uDefault;
	const char* szEnv = getenv ( szEnvName );
	if ( szEnv )
	{
		char* szEnd = nullptr;
		uRes = strtoul ( szEnv, &szEnd, 10 );
		if ( *szEnd != '\0' )
			sphWarning ( "%s expects to be numeric. %s provided, failed to parse as numeric since %s", szEnvName, szEnv, szEnd );
	}
	return uRes;
}

// absolute path from outside, as /usr/share/manticore, when prefix / or /usr. Or /usr/local/share/manticore if prefix /usr/local/
const char* GET_FULL_SHARE_DIR()
{
	const char* szEnv = getenv ( "FULL_SHARE_DIR" );
	if ( szEnv )
		return szEnv;

#if _WIN32
	CSphString sInstallDir = GetWinInstallDir();
	if ( !sInstallDir.IsEmpty() )
	{
		static CSphString sShare;
		sShare.SetSprintf ( "%s/share", sInstallDir.cstr() );
		return sShare.cstr();
	}
#endif

	return FULL_SHARE_DIR;
}

// galera_soname must be provided by configuration.
CSphString GET_GALERA_FULLPATH()
{
	CSphString sResult;
	const char* szEnv = getenv ( "GALERA_SONAME" );
	if ( szEnv )
		sResult = szEnv;
	else
		sResult.SetSprintf ( "%s/%s", GET_MANTICORE_MODULES(), GALERA_SONAME );
	return sResult;
}

CSphString GetColumnarFullpath()
{
	CSphString sResult;
	const char* szEnv = getenv ( "LIB_MANTICORE_COLUMNAR" );
	if ( szEnv )
		sResult = szEnv;
	else
		sResult.SetSprintf ( "%s/" LIB_MANTICORE_COLUMNAR, GET_MANTICORE_MODULES() );

	return sResult;
}

CSphString GetSecondaryFullpath()
{
	CSphString sResult;
	const char* szEnv = getenv ( "LIB_MANTICORE_SECONDARY" );
	if ( szEnv )
		sResult = szEnv;
	else
		sResult.SetSprintf ( "%s/" LIB_MANTICORE_SECONDARY, GET_MANTICORE_MODULES() );

	return sResult;
}

CSphString GetKNNFullpath()
{
	CSphString sResult;
	const char* szEnv = getenv ( "LIB_MANTICORE_KNN" );
	if ( szEnv )
		sResult = szEnv;
	else
		sResult.SetSprintf ( "%s/" LIB_MANTICORE_KNN, GET_MANTICORE_MODULES() );

	return sResult;
}


static CSphString GetSegmentationDataPath ( const char * szEnvVar, const char * szDir, const char * szMask )
{
	const char* szEnv = getenv(szEnvVar);
	if ( szEnv )
		return szEnv;

#ifdef _WIN32
	CSphString sPathToExe = GetPathOnly ( GetExecutablePath() );
	CSphString sPathToData;
	sPathToData.SetSprintf ( "%s/../share/%s/", sPathToExe.cstr(), szDir );
	sPathToData = sphNormalizePath ( sPathToData );

	CSphString sSearch;
	sSearch.SetSprintf ( "%s/%s", sPathToData.cstr(), szMask );

	StrVec_t dFiles = FindFiles ( sSearch.cstr(), false );
	if ( dFiles.GetLength() )
		return sPathToData;
#endif

	CSphString sInitModulesPrefix;
	sInitModulesPrefix.SetSprintf ( "%s/%s", GET_FULL_SHARE_DIR(), szDir );
	return sInitModulesPrefix;
}


CSphString GetICUDataDir()
{
	return GetSegmentationDataPath ( "ICU_DATA_DIR", "icu", "icudt*.dat" );
}


CSphString GetJiebaDataDir()
{
	return GetSegmentationDataPath ( "JIEBA_DATA_DIR", "jieba", "*.utf8" );
}


const char* GET_MANTICORE_MODULES()
{
	const char* szEnv = getenv ( "MANTICORE_MODULES" );
	if ( szEnv )
		return szEnv;
	static CSphString sModulesPrefix;
	if ( sModulesPrefix.IsEmpty() )
	{
		CSphString sInitModulesPrefix;
		sInitModulesPrefix.SetSprintf ( "%s/modules", GET_FULL_SHARE_DIR() );
		sModulesPrefix.Swap ( sInitModulesPrefix );
	}
	return sModulesPrefix.cstr();
}
