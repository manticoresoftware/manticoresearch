// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)
//
// Manticore SQL client target selection and entry point.

#include "manticoreclient.h"
#include "searchdlocalinternal.h"

#include "fileutils.h"
#include "searchdaemon.h"

#include <cerrno>
#include <climits>
#include <clocale>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#if !_WIN32
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace
{
constexpr const char * LOCAL_DATA_DIR = "manticore_data";
constexpr const char * LOCAL_PID = "searchd.pid";
constexpr const char * LOCAL_SOCKET = "searchd.sock";
CSphString g_sLocalDataDir;

CSphString LocalPath ( const char * szName )
{
	CSphString sPath;
	sPath.SetSprintf ( "%s/%s", g_sLocalDataDir.cstr(), szName );
	return sPath;
}

#if !_WIN32
void SetTcpEndpoint ( localmode::SqlEndpoint_t & tEndpoint, DWORD uIP, int iPort )
{
	tEndpoint.m_uIP = uIP==htonl(INADDR_ANY) ? htonl(INADDR_LOOPBACK) : uIP;
	tEndpoint.m_iPort = iPort;
	char szAddress[INET_ADDRSTRLEN] {};
	in_addr tAddress { tEndpoint.m_uIP };
	const char * szText = inet_ntop ( AF_INET, &tAddress, szAddress, sizeof(szAddress) );
	tEndpoint.m_sDescription.SetSprintf ( "%s:%d", szText ? szText : "unknown", iPort );
}

bool ResolveConfiguredEndpoint ( const char * szConfigFile, localmode::SqlEndpoint_t & tEndpoint, CSphString & sError )
{
	CSphString sConfigFile = sphGetConfigFile ( szConfigFile );
	auto dConfig = FetchAndCheckIfChanged ( sConfigFile ).second;
	CSphConfig hConf;
	if ( !ParseConfig ( &hConf, sConfigFile, dConfig ) )
	{
		sError.SetSprintf ( "failed to parse config file '%s': %s", sConfigFile.cstr(), TlsMsg::szError() );
		return false;
	}

	if ( !hConf.Exists ( "searchd" ) || !hConf["searchd"].Exists ( "searchd" ) )
	{
		sError.SetSprintf ( "'searchd' config section not found in '%s'", sConfigFile.cstr() );
		return false;
	}

	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];
	CSphVariant * pListener = hSearchd ( "listen" );
	if ( !pListener )
	{
		SetTcpEndpoint ( tEndpoint, htonl(INADDR_LOOPBACK), SPHINXAPI_PORT );
		return true;
	}

	bool bFound = false;
	CSphString sRelativeUnix;
	CSphString sPortRange;
	for ( ; pListener; pListener=pListener->m_pNext )
	{
		CSphString sListenerError;
		ListenerDesc_t tListener = ParseListener ( pListener->cstr(), &sListenerError );
		if ( !sListenerError.IsEmpty() )
		{
			sError.SetSprintf ( "invalid listener '%s' in '%s': %s", pListener->cstr(), sConfigFile.cstr(), sListenerError.cstr() );
			return false;
		}

		if ( bFound || ( tListener.m_eProto!=Proto_e::SPHINX && tListener.m_eProto!=Proto_e::HTTP ) )
			continue;

		if ( !tListener.m_sUnix.IsEmpty() )
		{
			if ( !IsPathAbsolute ( tListener.m_sUnix ) )
			{
				if ( sRelativeUnix.IsEmpty() )
					sRelativeUnix = tListener.m_sUnix;
				continue;
			}
			tEndpoint.m_sUnix = tListener.m_sUnix;
			tEndpoint.m_sDescription = tListener.m_sUnix;
		}
		else
		{
			if ( tListener.m_iPortsCount>1 )
			{
				if ( sPortRange.IsEmpty() )
					sPortRange = pListener->cstr();
				continue;
			}
			SetTcpEndpoint ( tEndpoint, tListener.m_uIP, tListener.m_iPort );
		}
		bFound = true;
	}

	if ( !bFound )
	{
		if ( !sRelativeUnix.IsEmpty() )
			sError.SetSprintf ( "relative Unix listener '%s' in '%s' cannot be resolved safely; use an absolute socket path or a TCP HTTP/binary listener", sRelativeUnix.cstr(), sConfigFile.cstr() );
		else if ( !sPortRange.IsEmpty() )
			sError.SetSprintf ( "listener port range '%s' in '%s' cannot be resolved safely; use a fixed HTTP/binary listener", sPortRange.cstr(), sConfigFile.cstr() );
		else
			sError.SetSprintf ( "no HTTP or binary listener found in config file '%s'", sConfigFile.cstr() );
		return false;
	}

	return true;
}

bool ResolveRemoteEndpoint ( const char * szHost, int iPort, localmode::SqlEndpoint_t & tEndpoint, CSphString & sError )
{
	const char * szEffectiveHost = szHost && *szHost ? szHost : "127.0.0.1";
	addrinfo tHints {};
	tHints.ai_family = AF_INET;
	tHints.ai_socktype = SOCK_STREAM;
	addrinfo * pAddresses = nullptr;
	int iResult = getaddrinfo ( szEffectiveHost, nullptr, &tHints, &pAddresses );
	if ( iResult || !pAddresses )
	{
		sError.SetSprintf ( "cannot resolve Manticore host '%s': %s", szEffectiveHost, gai_strerror(iResult) );
		return false;
	}
	AT_SCOPE_EXIT ( [&] { freeaddrinfo(pAddresses); } );

	const auto * pAddress = (const sockaddr_in*)pAddresses->ai_addr;
	tEndpoint.m_uIP = pAddress->sin_addr.s_addr;
	tEndpoint.m_iPort = iPort;
	tEndpoint.m_bMysql = true;
	tEndpoint.m_sDescription.SetSprintf ( "%s:%d", szEffectiveHost, iPort );
	return true;
}

bool ValidateLocalDaemonOwnership ( int & iPid, CSphString & sError )
{
	CSphString sPidPath = LocalPath ( LOCAL_PID );
	int iFile = -1;
	if ( !OpenPidFile(sPidPath,iFile,iPid,sError) )
	{
		CSphString sDetail = sError;
		sError.SetSprintf ( "local PID file '%s' is missing or invalid: %s", sPidPath.cstr(), sDetail.cstr() );
		return false;
	}
	AT_SCOPE_EXIT ( [&] { SafeClose(iFile); } );
	if ( !ValidatePidFileOwner(iFile,iPid,sError) || ( kill(iPid,0)<0 && errno==ESRCH ) )
	{
		if ( sError.IsEmpty() )
			sError.SetSprintf ( "local PID file '%s' is not owned by its recorded live daemon", sPidPath.cstr() );
		return false;
	}
	return true;
}
#endif
}

int ExecuteManticoreSql ( const char * szSql, ManticoreClientTarget_e eTarget, const char * szConfigFile, const char * szHost, int iPort )
{
#if _WIN32
	(void)szHost;
	(void)iPort;
	fprintf ( stderr, "manticore: client mode is not supported on Windows\n" );
	return 1;
#else
	bool bLocal = eTarget==ManticoreClientTarget_e::LOCAL;
	bool bRemote = eTarget==ManticoreClientTarget_e::REMOTE;
	if ( eTarget==ManticoreClientTarget_e::AUTO || eTarget==ManticoreClientTarget_e::LOCAL )
	{
		CSphString sCwd = sphGetCwd();
		if ( sCwd.IsEmpty() )
		{
			fprintf ( stderr, "manticore: failed to determine current directory: %s\n", strerror(errno) );
			return 1;
		}

		g_sLocalDataDir.SetSprintf ( "%s/%s", sCwd.cstr(), LOCAL_DATA_DIR );
		g_sLocalDataDir = sphNormalizePath ( g_sLocalDataDir );
		struct stat tStat {};
		if ( lstat ( g_sLocalDataDir.cstr(), &tStat )<0 )
		{
			if ( errno==ENOENT && eTarget==ManticoreClientTarget_e::AUTO )
				bLocal = false;
			else if ( errno==ENOENT )
			{
				fprintf ( stderr, "manticore: local data path '%s' does not exist; run 'manticore start local' to create it\n", g_sLocalDataDir.cstr() );
				return 1;
			}
			else
			{
				fprintf ( stderr, "manticore: cannot inspect local data path '%s': %s\n", g_sLocalDataDir.cstr(), strerror(errno) );
				return 1;
			}
		}
		else if ( !S_ISDIR(tStat.st_mode) )
		{
			fprintf ( stderr, "manticore: local data path '%s' is not a directory\n", g_sLocalDataDir.cstr() );
			return 1;
		}
		else
			bLocal = true;
	}

	localmode::SqlEndpoint_t tEndpoint;
	CSphString sError;
	int iOwnedPid = 0;
	if ( bRemote )
	{
		if ( !ResolveRemoteEndpoint(szHost,iPort,tEndpoint,sError) )
		{
			fprintf ( stderr, "manticore: %s\n", sError.cstr() );
			return 1;
		}
	}
	else if ( bLocal )
	{
		tEndpoint.m_sUnix.SetSprintf ( "%s/%s", LOCAL_DATA_DIR, LOCAL_SOCKET );
		tEndpoint.m_sDescription = tEndpoint.m_sUnix;
		if ( !ValidateLocalDaemonOwnership(iOwnedPid,sError) )
		{
			fprintf ( stderr, "manticore: local instance is not running: %s\nRun 'manticore start local' to start it.\n", sError.cstr() );
			return 1;
		}
	}
	else if ( !ResolveConfiguredEndpoint ( szConfigFile, tEndpoint, sError ) )
	{
		fprintf ( stderr, "manticore: %s\n", sError.cstr() );
		return 1;
	}

	int iSocket = localmode::ConnectSocket ( sError, tEndpoint );
	if ( iSocket<0 )
	{
		if ( !bLocal )
		{
			fprintf ( stderr, "manticore: %s\n", sError.cstr() );
			return 1;
		}
		// The daemon binds the socket before it finishes module loading and calls
		// listen(). Let an immediate `searchd -l; manticore -e ...` sequence wait
		// for the already validated owned process instead of reopening mutable PID metadata.
		const int64_t iDeadline = sphMicroTimer() + 300000000;
		while ( iSocket<0 && sphMicroTimer()<iDeadline )
		{
			if ( kill ( iOwnedPid, 0 )<0 && errno==ESRCH )
				break;
			usleep ( 50000 );
			iSocket = localmode::ConnectSocket ( sError, tEndpoint, iDeadline );
		}
		if ( iSocket<0 )
		{
			fprintf ( stderr, "manticore: local instance is not running: %s\nRun 'manticore start local' to start it.\n", sError.cstr() );
			return 1;
		}
	}


	if ( szSql && *szSql )
	{
		const bool bAligned = isatty ( STDOUT_FILENO );
		localmode::QueryResult_e eResult = localmode::ExecuteSqlBatch ( iSocket, szSql, bAligned, bAligned, tEndpoint );
		close ( iSocket );
		return eResult==localmode::QueryResult_e::OK ? 0 : 1;
	}

	const bool bShowPrompt = isatty ( STDIN_FILENO ) && isatty ( STDOUT_FILENO );
	std::unique_ptr<localmode::LineEditor_i> pEditor;
	if ( bShowPrompt )
	{
		setlocale ( LC_CTYPE, "" );
		CSphString sWarning;
		pEditor = localmode::CreateLineEditor ( g_sLocalDataDir, sWarning, localmode::CreateCompletionProvider ( tEndpoint ), bLocal );
		if ( !sWarning.IsEmpty() )
			fprintf ( stderr, "Warning: %s\n", sWarning.cstr() );
		if ( !pEditor )
			fprintf ( stderr, "Warning: interactive history is unavailable; using basic input\n" );
	}
	int iResult = 0;
	std::string sPending;
	auto ReportIncompleteInput = [&] ( localmode::SqlInputState_e eState )
	{
		if ( eState==localmode::SqlInputState_e::UNTERMINATED_BLOCK_COMMENT )
			fputs ( "ERROR: unterminated block comment\n", stderr );
		else
			fputs ( "ERROR: unterminated quoted string\n", stderr );
		iResult = 1;
	};
	auto ExecutePending = [&] ()
	{
		std::string sSql = localmode::TrimInput ( sPending );
		sPending.clear();
		if ( sSql.empty() )
			return true;
		if ( pEditor )
		{
			std::string sHistory = sSql;
			for ( char & c : sHistory )
				if ( c=='\n' || c=='\r' )
					c = ' ';
			CSphString sHistoryError;
			if ( !pEditor->AddHistory(sHistory,sHistoryError) )
				fprintf ( stderr, "Warning: %s\n", sHistoryError.cstr() );
		}

		localmode::QueryResult_e eResult = localmode::ExecuteSqlBatch ( iSocket, sSql.c_str(), false, bShowPrompt, tEndpoint );
		if ( pEditor )
			pEditor->RefreshCompletions ( sSql, eResult==localmode::QueryResult_e::OK );
		if ( eResult!=localmode::QueryResult_e::OK )
			iResult = 1;
		if ( eResult==localmode::QueryResult_e::SQL_ERROR )
		{
			close ( iSocket );
			iSocket = localmode::ConnectSocket ( sError, tEndpoint );
			if ( iSocket<0 )
			{
				fprintf ( stderr, "manticore: %s\n", sError.cstr() );
				return false;
			}
		}
		return eResult!=localmode::QueryResult_e::CONNECTION_ERROR;
	};

	while ( true )
	{
		std::string sLine;
		bool bInterrupted = false;
		bool bRead = false;
		if ( pEditor )
			bRead = pEditor->Read ( sLine, sPending.empty() ? "manticore> " : "       -> ", bInterrupted );
		else
		{
			if ( bShowPrompt )
			{
				fputs ( sPending.empty() ? "manticore> " : "       -> ", stdout );
				fflush ( stdout );
			}
			bRead = (bool)std::getline ( std::cin, sLine );
		}

		if ( bInterrupted )
		{
			sPending.clear();
			continue;
		}
		if ( !bRead )
		{
			if ( sPending.empty() )
				break;
			localmode::SqlInputState_e eState = localmode::InspectSqlInput ( sPending.c_str() );
			if ( eState==localmode::SqlInputState_e::UNTERMINATED_QUOTE || eState==localmode::SqlInputState_e::UNTERMINATED_BLOCK_COMMENT )
				ReportIncompleteInput ( eState );
			else
				ExecutePending(); // EOF terminates a final un-delimited statement for compatibility.
			break;
		}

		if ( sPending.empty() )
		{
			std::string sCommand = localmode::TrimInput ( sLine );
			while ( !sCommand.empty() && sCommand.back()==';' )
			{
				sCommand.pop_back();
				sCommand = localmode::TrimInput ( sCommand );
			}
			if ( strcasecmp ( sCommand.c_str(), "exit" )==0 || strcasecmp ( sCommand.c_str(), "quit" )==0 )
				break;
		}

		if ( !sPending.empty() )
			sPending.push_back ( '\n' );
		sPending += sLine;
		localmode::SqlInputState_e eState = localmode::InspectSqlInput ( sPending.c_str() );
		if ( eState==localmode::SqlInputState_e::EMPTY )
		{
			sPending.clear();
			continue;
		}
		if ( eState!=localmode::SqlInputState_e::COMPLETE )
			continue;
		if ( !ExecutePending() )
			break;
	}

	close ( iSocket );
	return iResult;
#endif
}

bool ProbeManticoreReady ( ManticoreClientTarget_e eTarget, const char * szConfigFile, std::string & sErrorOut, int64_t iDeadlineUS )
{
#if _WIN32
	(void)eTarget;
	(void)szConfigFile;
	(void)iDeadlineUS;
	sErrorOut = "client mode is not supported on Windows";
	return false;
#else
	localmode::SqlEndpoint_t tEndpoint;
	CSphString sError;
	if ( eTarget==ManticoreClientTarget_e::LOCAL )
	{
		// searchd binds this relative name so local mode also works when the
		// absolute working-directory path exceeds sockaddr_un::sun_path.
		tEndpoint.m_sUnix.SetSprintf ( "%s/%s", LOCAL_DATA_DIR, LOCAL_SOCKET );
		tEndpoint.m_sDescription = tEndpoint.m_sUnix;
	}
	else if ( !ResolveConfiguredEndpoint ( szConfigFile, tEndpoint, sError ) )
	{
		sErrorOut = sError.cstr();
		return false;
	}

	bool bReady = localmode::ProbeEndpoint ( tEndpoint, sError, iDeadlineUS )==localmode::QueryResult_e::OK;
	if ( !bReady )
		sErrorOut = sError.cstr();
	return bReady;
#endif
}
