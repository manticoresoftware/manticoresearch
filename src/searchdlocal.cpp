// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)
//
// Local zero-configuration searchd startup.

#include "searchdlocal.h"

#include "fileutils.h"

#include <cerrno>
#include <cstring>

#if !_WIN32
#include <sys/stat.h>
#endif

namespace
{
constexpr const char * LOCAL_DATA_DIR = "manticore_data";
constexpr const char * LOCAL_PID = "searchd.pid";
CSphString g_sLocalDataDir;

CSphString LocalPath ( const char * szName )
{
	CSphString sPath;
	sPath.SetSprintf ( "%s/%s", g_sLocalDataDir.cstr(), szName );
	return sPath;
}
}

bool BuildLocalSearchdConfig ( CSphConfig & hConf, CSphString & sDataDir, CSphString & sError )
{
#if _WIN32
	sError = "zero-configuration local mode is not supported on Windows";
	return false;
#else
	CSphString sCwd = sphGetCwd();
	if ( sCwd.IsEmpty() )
	{
		sError.SetSprintf ( "failed to determine current directory: %s", strerror(errno) );
		return false;
	}

	g_sLocalDataDir.SetSprintf ( "%s/%s", sCwd.cstr(), LOCAL_DATA_DIR );
	g_sLocalDataDir = sphNormalizePath ( g_sLocalDataDir );

	struct stat tStat {};
	if ( lstat ( g_sLocalDataDir.cstr(), &tStat )<0 )
	{
		if ( errno!=ENOENT || !MkDir ( g_sLocalDataDir.cstr() ) )
		{
			sError.SetSprintf ( "failed to create local data directory '%s': %s", g_sLocalDataDir.cstr(), strerror(errno) );
			return false;
		}
	}
	else if ( !S_ISDIR(tStat.st_mode) )
	{
		sError.SetSprintf ( "local data path '%s' is not a directory", g_sLocalDataDir.cstr() );
		return false;
	}

	if ( chmod ( g_sLocalDataDir.cstr(), S_IRWXU )<0 )
	{
		sError.SetSprintf ( "failed to secure local data directory '%s': %s", g_sLocalDataDir.cstr(), strerror(errno) );
		return false;
	}

	hConf.Add ( CSphConfigType(), "searchd" );
	hConf["searchd"].Add ( CSphConfigSection(), "searchd" );
	CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];
	hSearchd.AddEntry ( "data_dir", g_sLocalDataDir.cstr() );
	hSearchd.AddEntry ( "listen", "unix:searchd.sock:http" );
	hSearchd.AddEntry ( "pid_file", LocalPath(LOCAL_PID).cstr() );
	hSearchd.AddEntry ( "log", LocalPath("searchd.log").cstr() );
	hSearchd.AddEntry ( "watchdog", "0" );

	sDataDir = g_sLocalDataDir;
	return true;
#endif
}
