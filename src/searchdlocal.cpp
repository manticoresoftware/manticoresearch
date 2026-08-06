// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)
//
// Local zero-configuration searchd startup and query client.

#include "searchdlocal.h"
#include "searchdlocalinternal.h"

#include "fileutils.h"

#include <cerrno>
#include <clocale>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#if !_WIN32
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
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
	hSearchd.AddEntry ( "pid_file", LocalPath("searchd.pid").cstr() );
	hSearchd.AddEntry ( "log", LocalPath("searchd.log").cstr() );
	hSearchd.AddEntry ( "watchdog", "0" );

	sDataDir = g_sLocalDataDir;
	return true;
#endif
}

int ExecuteLocalSql ( const char * szSql )
{
#if _WIN32
	fprintf ( stderr, "searchd: -e local mode is not supported on Windows\n" );
	return 1;
#else
	g_sLocalDataDir.SetSprintf ( "%s/%s", sphGetCwd().cstr(), LOCAL_DATA_DIR );
	g_sLocalDataDir = sphNormalizePath ( g_sLocalDataDir );
	if ( !sphDirExists ( g_sLocalDataDir ) )
	{
		fprintf ( stderr, "searchd: no local Manticore instance found in %s\nRun 'searchd -l' in this directory first.\n", sphGetCwd().cstr() );
		return 1;
	}

	CSphString sError;
	int iSocket = localmode::ConnectSocket ( sError );
	if ( iSocket<0 )
	{
		CSphString sPidPath = LocalPath ( LOCAL_PID );
		FILE * pPid = fopen ( sPidPath.cstr(), "r" );
		int iPid = 0;
		if ( !pPid || fscanf ( pPid, "%d", &iPid )!=1 || iPid<=0 || ( kill ( iPid, 0 )<0 && errno==ESRCH ) )
		{
			if ( pPid )
				fclose ( pPid );
			fprintf ( stderr, "searchd: %s\n", sError.cstr() );
			return 1;
		}
		fclose ( pPid );

		// The daemon binds the socket before it finishes module loading and calls
		// listen(). Let an immediate `searchd -l; searchd -e ...` sequence wait
		// for that live process instead of failing spuriously.
		for ( int i=0; i<6000 && iSocket<0; ++i )
		{
			if ( kill ( iPid, 0 )<0 && errno==ESRCH )
				break;
			usleep ( 50000 );
			iSocket = localmode::ConnectSocket ( sError );
		}
		if ( iSocket<0 )
		{
			fprintf ( stderr, "searchd: %s\n", sError.cstr() );
			return 1;
		}
	}


	if ( szSql && *szSql )
	{
		localmode::QueryResult_e eResult = localmode::ExecuteSqlBatch ( iSocket, szSql, true, isatty(STDOUT_FILENO) );
		close ( iSocket );
		return eResult==localmode::QueryResult_e::OK ? 0 : 1;
	}

	const bool bShowPrompt = isatty ( STDIN_FILENO ) && isatty ( STDOUT_FILENO );
	std::unique_ptr<localmode::LineEditor_i> pEditor;
	if ( bShowPrompt )
	{
		setlocale ( LC_CTYPE, "" );
		CSphString sWarning;
		pEditor = localmode::CreateLineEditor ( g_sLocalDataDir, sWarning, localmode::CreateCompletionProvider() );
		if ( !sWarning.IsEmpty() )
			fprintf ( stderr, "Warning: %s\n", sWarning.cstr() );
		if ( !pEditor )
			fprintf ( stderr, "Warning: interactive history is unavailable; using basic input\n" );
	}
	int iResult = 0;
	while ( true )
	{
		std::string sLine;
		if ( pEditor )
		{
			if ( !pEditor->Read ( sLine ) )
				break;
		}
		else
		{
			if ( bShowPrompt )
			{
				fputs ( "manticore> ", stdout );
				fflush ( stdout );
			}
			if ( !std::getline ( std::cin, sLine ) )
				break;
		}
		sLine = localmode::TrimInput ( sLine );
		if ( sLine.empty() )
			continue;

		std::string sCommand = sLine;
		while ( !sCommand.empty() && sCommand.back()==';' )
		{
			sCommand.pop_back();
			sCommand = localmode::TrimInput ( sCommand );
		}
		if ( sCommand.empty() )
			continue;
		if ( strcasecmp ( sCommand.c_str(), "exit" )==0 || strcasecmp ( sCommand.c_str(), "quit" )==0 )
			break;
		if ( pEditor )
		{
			CSphString sHistoryError;
			if ( !pEditor->AddHistory ( sLine, sHistoryError ) )
				fprintf ( stderr, "Warning: %s\n", sHistoryError.cstr() );
		}

		localmode::QueryResult_e eResult = localmode::ExecuteSqlBatch ( iSocket, sLine.c_str(), false, bShowPrompt );
		if ( pEditor )
			pEditor->RefreshCompletions ( sLine, eResult==localmode::QueryResult_e::OK );
		if ( eResult!=localmode::QueryResult_e::OK )
			iResult = 1;
		if ( eResult==localmode::QueryResult_e::SQL_ERROR )
		{
			close ( iSocket );
			iSocket = localmode::ConnectSocket ( sError );
			if ( iSocket<0 )
			{
				fprintf ( stderr, "searchd: %s\n", sError.cstr() );
				break;
			}
		}
		else if ( eResult==localmode::QueryResult_e::CONNECTION_ERROR )
			break;
	}

	close ( iSocket );
	return iResult;
#endif
}
