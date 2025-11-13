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

#include "winservice.h"
#include "daemon_ipc.h"

static bool				g_bService		= false;
bool WinService () noexcept
{
	return g_bService;
}

#if _WIN32

#include "std/ints.h"
#include "std/fatal.h"
#include "sphinxutils.h"
#include "searchdaemon.h"

#include "winsvc.h"


static bool				g_bServiceStop 	= false;
static const char *		g_sServiceName	= "searchd";
static HANDLE			g_hPipe			= INVALID_HANDLE_VALUE;
constexpr int			WIN32_PIPE_BUFSIZE = 32;

SERVICE_STATUS g_ss;
SERVICE_STATUS_HANDLE g_ssHandle;

bool StopWinService () noexcept
{
	return g_bServiceStop;
}


void MySetServiceStatus ( DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint )
{
	static DWORD dwCheckPoint = 1;

	if ( dwCurrentState==SERVICE_START_PENDING )
		g_ss.dwControlsAccepted = 0;
	else
		g_ss.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;

	g_ss.dwCurrentState = dwCurrentState;
	g_ss.dwWin32ExitCode = dwWin32ExitCode;
	g_ss.dwWaitHint = dwWaitHint;

	if ( dwCurrentState==SERVICE_RUNNING || dwCurrentState==SERVICE_STOPPED )
		g_ss.dwCheckPoint = 0;
	else
		g_ss.dwCheckPoint = dwCheckPoint++;

	SetServiceStatus ( g_ssHandle, &g_ss );
}


void WINAPI ServiceControl ( DWORD dwControlCode )
{
	switch ( dwControlCode )
	{
		case SERVICE_CONTROL_STOP:
		case SERVICE_CONTROL_SHUTDOWN:
			MySetServiceStatus ( SERVICE_STOP_PENDING, NO_ERROR, 0 );
			g_bServiceStop = true;
			break;

		default:
			MySetServiceStatus ( g_ss.dwCurrentState, NO_ERROR, 0 );
			break;
	}
}


// warning! static buffer, non-reentrable
const char * WinErrorInfo ()
{
	static char sBuf[1024];

	DWORD uErr = ::GetLastError ();
	snprintf ( sBuf, sizeof(sBuf), "code=%d, error=", uErr );

	auto iLen = (int) strlen(sBuf);
	if ( !FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM, NULL, uErr, 0, sBuf+iLen, sizeof(sBuf)-iLen, NULL ) ) // FIXME? force US-english langid?
		snprintf ( sBuf+iLen, sizeof(sBuf)-iLen, "(no message)" );

	return sBuf;
}


SC_HANDLE ServiceOpenManager ()
{
	SC_HANDLE hSCM = OpenSCManager (
		NULL,						// local computer
		NULL,						// ServicesActive database
		SC_MANAGER_ALL_ACCESS );	// full access rights

	if ( hSCM==NULL )
		sphFatal ( "OpenSCManager() failed: %s", WinErrorInfo() );

	return hSCM;
}


void AppendArg ( char * sBuf, int iBufLimit, const char * sArg )
{
	char * sBufMax = sBuf + iBufLimit - 2; // reserve place for opening space and trailing zero
	sBuf += strlen(sBuf);

	if ( sBuf>=sBufMax )
		return;

	auto iArgLen = (int) strlen(sArg);
	bool bQuote = false;
	for ( int i=0; i<iArgLen && !bQuote; i++ )
		if ( sArg[i]==' ' || sArg[i]=='"' )
			bQuote = true;

	*sBuf++ = ' ';
	if ( !bQuote )
	{
		// just copy
		int iToCopy = Min ( sBufMax-sBuf, iArgLen );
		memcpy ( sBuf, sArg, iToCopy );
		sBuf[iToCopy] = '\0';

	} else
	{
		// quote
		sBufMax -= 2; // reserve place for quotes
		if ( sBuf>=sBufMax )
			return;

		*sBuf++ = '"';
		while ( sBuf<sBufMax && *sArg )
		{
			if ( *sArg=='"' )
			{
				// quote
				if ( sBuf<sBufMax-1 )
				{
					*sBuf++ = '\\';
					*sBuf++ = *sArg++;
				}
			} else
			{
				// copy
				*sBuf++ = *sArg++;
			}
		}
		*sBuf++ = '"';
		*sBuf++ = '\0';
	}
}


void ServiceInstall ( int argc, char ** argv )
{
	if ( g_bService )
		return;

	sphInfo ( "Installing service..." );

	char szBinary[MAX_PATH];
	if ( !GetModuleFileName ( NULL, szBinary, MAX_PATH ) )
		sphFatal ( "GetModuleFileName() failed: %s", WinErrorInfo() );

	char szPath[MAX_PATH];
	szPath[0] = '\0';

	AppendArg ( szPath, sizeof(szPath), szBinary );
	AppendArg ( szPath, sizeof(szPath), "--ntservice" );
	for ( int i=1; i<argc; i++ )
		if ( strcmp ( argv[i], "--install" ) )
			AppendArg ( szPath, sizeof(szPath), argv[i] );

	SC_HANDLE hSCM = ServiceOpenManager ();
	SC_HANDLE hService = CreateService (
		hSCM,							// SCM database
		g_sServiceName,					// name of service
		g_sServiceName,					// service name to display
		SERVICE_ALL_ACCESS,				// desired access
		SERVICE_WIN32_OWN_PROCESS,		// service type
		SERVICE_AUTO_START,				// start type
		SERVICE_ERROR_NORMAL,			// error control type
		szPath+1,						// path to service's binary
		NULL,							// no load ordering group
		NULL,							// no tag identifier
		NULL,							// no dependencies
		NULL,							// LocalSystem account
		NULL );							// no password

	if ( !hService )
	{
		CloseServiceHandle ( hSCM );
		sphFatal ( "CreateService() failed: %s", WinErrorInfo() );
	}
	sphInfo ( "Service '%s' installed successfully.", g_sServiceName );

	CSphString sDesc;
	sDesc.SetSprintf ( "%s-%s", g_sServiceName, szStatusVersion() );

	SERVICE_DESCRIPTION tDesc;
	tDesc.lpDescription = (LPSTR) sDesc.cstr();
	if ( !ChangeServiceConfig2 ( hService, SERVICE_CONFIG_DESCRIPTION, &tDesc ) )
		sphWarning ( "failed to set service description" );

	CloseServiceHandle ( hService );
	CloseServiceHandle ( hSCM );
}


void ServiceDelete ()
{
	if ( g_bService )
		return;

	sphInfo ( "Deleting service..." );

	// open manager
	SC_HANDLE hSCM = ServiceOpenManager ();

	// open service
	SC_HANDLE hService = OpenService ( hSCM, g_sServiceName, DELETE );
	if ( !hService )
	{
		CloseServiceHandle ( hSCM );
		sphFatal ( "OpenService() failed: %s", WinErrorInfo() );
	}

	// do delete
	bool bRes = !!DeleteService ( hService );
	CloseServiceHandle ( hService );
	CloseServiceHandle ( hSCM );

	if ( !bRes )
		sphFatal ( "DeleteService() failed: %s", WinErrorInfo() );
	else
		sphInfo ( "Service '%s' deleted successfully.", g_sServiceName );
}

void EventLogEntry ( ESphLogLevel eLevel, char * sBuf, char * sTtyBuf )
{
	HANDLE hEventSource;
	LPCTSTR lpszStrings[2];

	hEventSource = RegisterEventSource ( NULL, g_sServiceName );
	if ( hEventSource )
	{
		lpszStrings[0] = g_sServiceName;
		lpszStrings[1] = sBuf;

		WORD eType;
		switch ( eLevel )
		{
		case SPH_LOG_FATAL: eType = EVENTLOG_ERROR_TYPE;
			break;
		case SPH_LOG_WARNING: eType = EVENTLOG_WARNING_TYPE;
			break;
		case SPH_LOG_INFO: eType = EVENTLOG_INFORMATION_TYPE;
			break;
		default: eType = EVENTLOG_INFORMATION_TYPE;
			break;
		}

		ReportEvent ( hEventSource, // event log handle
					  eType, // event type
					  0, // event category
					  0, // event identifier
					  NULL, // no security identifier
					  2, // size of lpszStrings array
					  0, // no binary data
					  lpszStrings, // array of strings
					  NULL ); // no binary data

		DeregisterEventSource ( hEventSource );
	}
}

BOOL WINAPI CtrlHandler ( DWORD )
{
	if ( !WinService() )
		sphInterruptNow();
	return TRUE;
}

void WinSetConsoleCtrlHandler () noexcept
{
	SetConsoleCtrlHandler ( CtrlHandler, TRUE );
}

static StrVec_t g_dArgs;

bool ParseArgsAndStartWinService ( int argc, char ** argv, void * ServiceFunction )
{
	int iNameIndex = -1;
	for ( int i = 1; i < argc; ++i )
	{
		if ( strcmp ( argv[i], "--ntservice" ) == 0 )
			g_bService = true;

		if ( strcmp ( argv[i], "--servicename" ) == 0 && (i + 1) < argc )
		{
			iNameIndex = i + 1;
			g_sServiceName = argv[iNameIndex];
		}
	}

	if ( !g_bService )
		return false;

	for ( int i = 0; i < argc; i++ )
		g_dArgs.Add ( argv[i] );

	if ( iNameIndex >= 0 )
		g_sServiceName = g_dArgs[iNameIndex].cstr();

	SERVICE_TABLE_ENTRY dDispatcherTable[] =
	{
		{ (LPSTR) g_sServiceName, (LPSERVICE_MAIN_FUNCTION) ServiceFunction },
		{ NULL, NULL }
	};
	if ( !StartServiceCtrlDispatcher ( dDispatcherTable ) )
		sphFatal ( "StartServiceCtrlDispatcher() failed: %s", WinErrorInfo() );

	return true;
}

void SetupWinService ( int argc, char ** argv )
{
	CSphVector<char *> dArgs;
	if ( WinService() )
	{
		g_ssHandle = RegisterServiceCtrlHandler ( g_sServiceName, ServiceControl );
		if ( !g_ssHandle )
			sphFatal ( "failed to start service: RegisterServiceCtrlHandler() failed: %s", WinErrorInfo() );

		g_ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		MySetServiceStatus ( SERVICE_START_PENDING, NO_ERROR, 4000 );

		if ( argc <= 1 )
		{
			dArgs.Resize ( g_dArgs.GetLength() );
			ARRAY_FOREACH ( i, g_dArgs )
				dArgs[i] = (char *) g_dArgs[i].cstr();

			argc = g_dArgs.GetLength();
			argv = &dArgs[0];
		}
	}

	CSphString sPipeName = GetSignalPipeName ( getpid() );
	g_hPipe = CreateNamedPipe ( sPipeName.cstr(), PIPE_ACCESS_INBOUND,
								PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_NOWAIT,
								PIPE_UNLIMITED_INSTANCES, 0, WIN32_PIPE_BUFSIZE, NMPWAIT_NOWAIT, NULL );
	ConnectNamedPipe ( g_hPipe, NULL );
}

void SetWinServiceStopped ()
{
	MySetServiceStatus ( SERVICE_STOPPED, NO_ERROR, 0 );
}

void SetWinServiceRunning ()
{
	if ( g_bService )
		MySetServiceStatus ( SERVICE_RUNNING, NO_ERROR, 0 );
}


void CloseWinPipe ()
{
	CloseHandle ( g_hPipe );
}


void CheckWinSignals ()
{
	auto & g_bGotSighup = sphGetGotSighup();
	BYTE dPipeInBuf[WIN32_PIPE_BUFSIZE];
	DWORD nBytesRead = 0;
	BOOL bSuccess = ReadFile ( g_hPipe, dPipeInBuf, WIN32_PIPE_BUFSIZE, &nBytesRead, NULL );
	if ( nBytesRead > 0 && bSuccess )
	{
		for ( DWORD i=0; i<nBytesRead; i++ )
		{
			switch ( dPipeInBuf[i] )
			{
			case 0:
				g_bGotSighup = 1;
				break;

			case 1:
				sphInterruptNow();
				if ( g_bService )
					g_bServiceStop = true;
				break;

			case PIPE_CMD_CHECK_CMD:
				ProcessAuthReloadPipe ( getpid() );
				break;

			}
		}

		DisconnectNamedPipe ( g_hPipe );
		ConnectNamedPipe ( g_hPipe, NULL );
	}
}

int WinStopOrWaitAnother(int iPid, int iWaitTimeout)
{
	bool bTerminatedOk = false;
	CSphString sPipeName = GetSignalPipeName ( iPid );

	HANDLE hPipe = INVALID_HANDLE_VALUE;
	while ( hPipe==INVALID_HANDLE_VALUE )
	{
		hPipe = CreateFile ( sPipeName.cstr(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );

		if ( hPipe==INVALID_HANDLE_VALUE )
		{
			if ( GetLastError()!=ERROR_PIPE_BUSY )
			{
				fprintf ( stdout, "WARNING: could not open pipe (GetLastError()=%d)\n", GetLastError () );
				break;
			}

			if ( !WaitNamedPipe ( sPipeName.cstr(), iWaitTimeout/1000 ) )
			{
				fprintf ( stdout, "WARNING: could not open pipe (GetLastError()=%d)\n", GetLastError () );
				break;
			}
		}
	}

	if ( hPipe!=INVALID_HANDLE_VALUE )
	{
		DWORD uWritten = 0;
		BYTE uWrite = 1;
		BOOL bResult = WriteFile ( hPipe, &uWrite, 1, &uWritten, NULL );
		if ( !bResult )
			fprintf ( stdout, "WARNING: failed to send SIGHTERM to searchd (pid=%d, GetLastError()=%d)\n", iPid, GetLastError () );

		bTerminatedOk = !!bResult;

		CloseHandle ( hPipe );
	}

	if ( !bTerminatedOk )
		sphFatal ( "stop: error terminating pid %d", iPid );

	sphInfo ( "stop: successfully terminated pid %d", iPid );
	return 0;

}

#endif // _WIN32

CSphString GetSignalPipeName ( int iPid )
{
	CSphString sName;
	sName.SetSprintf ( "\\\\.\\pipe\\searchd_%d", iPid );
	return sName;
}
