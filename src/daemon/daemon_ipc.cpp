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

#if !_WIN32
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <errno.h>
#include <cstdio>
#else
#include "winservice.h"
#endif

#include "searchdaemon.h"
#include "auth/auth.h"

#include "daemon_ipc.h"

struct PipeTrait_t : public ISphNonCopyMovable
{
	CSphString m_sName;
	int m_iCreated = -1;
	int m_iFD = -1;

	PipeTrait_t() = default;
	~PipeTrait_t();
};

static void CreatePipe ( const CSphString & sName, int iOpenFlags, PipeTrait_t & tPipe, CSphString & sError );
static int WaitPipe ( const PipeTrait_t & tPipe, int iWaitTimeout );

int StopDaemonAndWait ( bool bWait, int iPid, int iWaitTimeout )
{
	CSphString sError;
	int iExitCode = 0;
	PipeTrait_t tPipe;

	if ( bWait )
		CreatePipe ( GetNamedPipeName ( iPid ), O_RDONLY, tPipe, sError );
	if ( !sError.IsEmpty() )
		sphWarning ( "%s", sError.cstr() );

#if !_WIN32
	if ( kill ( iPid, SIGTERM ) )
		sphWarning ( "stop: kill() on pid %d failed: %s", iPid, strerrorm(errno) );
	sphInfo ( "stop: successfully sent SIGTERM to pid %d", iPid );
#endif

	iExitCode = ( bWait && ( tPipe.m_iCreated==-1 || tPipe.m_iFD<0 ) ) ? 1 : 0;
	if ( bWait )
		iExitCode = WaitPipe ( tPipe, iWaitTimeout );

	return iExitCode;
}

void CreatePipe ( const CSphString & sName, int iOpenFlags, PipeTrait_t & tPipe, CSphString & sError )
{
#if !_WIN32
	tPipe.m_sName = sName;
	::unlink ( tPipe.m_sName.cstr() ); // avoid garbage to pollute us
	int iMask = umask ( 0 );

	tPipe.m_iCreated = mkfifo ( tPipe.m_sName.cstr(), 0666 );
	umask ( iMask );
	if ( tPipe.m_iCreated!=-1 )
		tPipe.m_iFD = ::open ( tPipe.m_sName.cstr(), iOpenFlags | O_NONBLOCK );

	if ( tPipe.m_iCreated==-1 )
		sError.SetSprintf ( "mkfifo failed (path=%s, err=%d, msg=%s); will NOT wait", tPipe.m_sName.cstr(), errno, strerrorm(errno) );
	else if ( tPipe.m_iFD<0 )
		sError.SetSprintf ( "open failed (path=%s, err=%d, msg=%s); will NOT wait", tPipe.m_sName.cstr(), errno, strerrorm(errno) );
#endif
}

int WaitPipe ( const PipeTrait_t & tPipe, int iWaitTimeout )
{
	int iExitCode = 0;

	bool bHandshake = true;
	if ( tPipe.m_iFD>=0 )
	{
		while ( true )
		{
			int iReady = sphPoll ( tPipe.m_iFD, iWaitTimeout );
			// error on wait
			if ( iReady<0 )
			{
				iExitCode = 3;
				sphWarning ( "stopwait%s error '%s'", ( bHandshake ? " handshake" : " " ), strerrorm(errno) );
				break;
			}

			// timeout
			if ( iReady==0 )
			{
				if ( !bHandshake )
					continue;

				sphWarning ( "timeout waiting for daemon reply" );
				iExitCode = 1;
				break;
			}

			// reading data
			DWORD uStatus = 0;
			int iGot = ::read ( tPipe.m_iFD, &uStatus, sizeof(uStatus) );
			if ( iGot!=sizeof(uStatus) )
			{
				sphWarning ( "stopwait read fifo error '%s'", strerrorm(errno) );
				iExitCode = 3; // stopped demon crashed during stop
				break;
			}
			iExitCode = ( uStatus==1 ? 0 : 2 ); // uStatus == 1 - AttributeSave - ok, other values - error

			if ( !bHandshake )
				break;

			bHandshake = false;
		}
	}

	return iExitCode;
}

PipeTrait_t::~PipeTrait_t()
{
	if ( !m_sName.IsEmpty() )
		::unlink ( m_sName.cstr () ); // is ok on linux after it is opened.

	if ( m_iFD>=0 )
		::close ( m_iFD );
}

CSphString GetNamedPipeName ( int iPid )
{
	CSphString sRes;
	sRes.SetSprintf ( "/tmp/searchd_%d", iPid );
	return sRes;
}

enum class ReloadAuthCmd_e
{
	REQ,
	REPLY
};

static CSphString GetCommandPipeName ( int iPid, ReloadAuthCmd_e eCmd )
{
	CSphString sRes;
	if ( eCmd==ReloadAuthCmd_e::REQ )
		sRes.SetSprintf ( "/tmp/searchd_cmd_req_%d", iPid );
	else
		sRes.SetSprintf ( "/tmp/searchd_cmd_reply_%d", iPid );
	return sRes;
}

#if _WIN32
static CSphString GetCommandPipeNameWin ( int iPid, ReloadAuthCmd_e eCmd )
{
	CSphString sRes;
	if ( eCmd == ReloadAuthCmd_e::REQ )
		sRes.SetSprintf ( "\\\\.\\pipe\\searchd_cmd_req_%d", iPid );
	else
		sRes.SetSprintf ( "\\\\.\\pipe\\searchd_cmd_reply_%d", iPid );
	return sRes;
}
#endif

static ReloadAuthResult_e WaitSimpleReply ( const PipeTrait_t & tPipe, int iWaitTimeout, CSphString & sError )
{
	if ( tPipe.m_iFD<0 )
		return ReloadAuthResult_e::CLIENT_ERROR; // pipe not created - error

	int iReady = sphPoll ( tPipe.m_iFD, iWaitTimeout );
	if ( iReady<0 )
	{
		sError.SetSprintf ( "pipe poll error: %s", strerrorm(errno) );
		return ReloadAuthResult_e::IPC_ERROR;
	}

	if ( iReady==0 )
	{
		sError.SetSprintf ( "timeout waiting for daemon reply" );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}

	// pipe is ready - read the data
	DWORD uStatus = 0;
	int iGot = ::read ( tPipe.m_iFD, &uStatus, sizeof(uStatus) );
	if ( iGot==sizeof(uStatus) )
		return ( uStatus==1 ? ReloadAuthResult_e::SUCCESS : ReloadAuthResult_e::DAEMON_ERROR );
	
	if ( iGot==0 ) // EOF
		sError.SetSprintf ( "daemon disconnected before sending reply" );
	else // iGot<0 (error) or a short read
		sError.SetSprintf ( "pipe read error: %s", strerrorm(errno) );
	
	return ReloadAuthResult_e::IPC_ERROR; // error code
}

constexpr DWORD CMD_HEADER_RELOAD_AUTH = 0x524c4155  ; ///< 'RLAU ' header

static ReloadAuthResult_e CommandReloadAuthAndWaitReplyUnix ( int iPid, int iWaitTimeout, CSphString & sError )
{
	PipeTrait_t tPipeReq;
	PipeTrait_t tPipeReply;

	//  use O_RDWR for the request pipe to avoid ENXIO instead of the O_WRONLY
	CreatePipe ( GetCommandPipeName ( iPid, ReloadAuthCmd_e::REQ ), O_RDWR, tPipeReq, sError );
	CreatePipe ( GetCommandPipeName ( iPid, ReloadAuthCmd_e::REPLY ), O_RDONLY, tPipeReply, sError );

	if ( tPipeReq.m_iFD<0 || tPipeReply.m_iFD<0 )
	{
		sError.SetSprintf ( "failed to create %s pipe: %s; cannot proceed", ( tPipeReq.m_iFD<0 ? "command" : "reply" ), sError.cstr() );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}

	DWORD uCmd = CMD_HEADER_RELOAD_AUTH;
	int iGot = ::write ( tPipeReq.m_iFD, &uCmd, sizeof(uCmd) );
	if ( iGot!=sizeof(uCmd) )
	{
		sError.SetSprintf ( "failed to write command to pipe: %s", strerrorm(errno) );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}
	
	// SIGUSR1 to notify the daemon to issue reload and check pipes
#if !_WIN32
	if ( kill ( iPid, SIGUSR1 ) )
	{
		sError.SetSprintf ( "kill() on pid %d failed: %s", iPid, strerrorm(errno) );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}
	sphLogDebug ( "reload auth: successfully sent SIGUSR1 to pid %d", iPid );
#endif

	return WaitSimpleReply ( tPipeReply, iWaitTimeout, sError );
}

class DaemonIpcPipe : private ISphNonCopyMovable
{
public:
	explicit DaemonIpcPipe ( int iPid, ReloadAuthCmd_e eCmd );
	~DaemonIpcPipe ();

	bool IsOpen () const { return m_iFD>=0; }
	int GetError () const { return m_iError; }
	bool WriteStatus ( DWORD uStatus );
	bool ReadCommand ( DWORD & uCmd );
	int GetFD () { return m_iFD; }

private:
	int m_iFD = -1;
	int m_iError = 0;
	CSphString m_sPipeName;
};

// hold the pipe objects, preventing their premature delete
struct IpcSession
{
	DaemonIpcPipe m_tPipeReq;
	DaemonIpcPipe m_tPipeReply;

	IpcSession ( int iPid )
		: m_tPipeReq ( iPid, ReloadAuthCmd_e::REQ )
		, m_tPipeReply ( iPid, ReloadAuthCmd_e::REPLY )
	{}
};

static std::unique_ptr<IpcSession> g_pIpcSession = nullptr;

DaemonIpcPipe::DaemonIpcPipe ( int iPid, ReloadAuthCmd_e eCmd )
{
	m_sPipeName = GetCommandPipeName ( iPid, eCmd );
#if !_WIN32
	int iFlags = ( eCmd==ReloadAuthCmd_e::REPLY ? O_WRONLY : O_RDONLY );
	iFlags |= O_NONBLOCK;

	m_iFD = ::open ( m_sPipeName.cstr(), iFlags );

	if ( m_iFD<0 )
		m_iError = errno;
#endif
}

DaemonIpcPipe::~DaemonIpcPipe ()
{
	if ( m_iFD>=0 )
		::close ( m_iFD );
}

bool DaemonIpcPipe::WriteStatus ( DWORD uStatus )
{
	if ( !IsOpen() )
		return false;
	int iGot = ( ::write ( m_iFD, &uStatus, sizeof(uStatus) )==sizeof(uStatus) );
	return ( iGot>0 );
}

bool DaemonIpcPipe::ReadCommand ( DWORD & uCmd )
{
	if ( !IsOpen() )
		return false;
	int iGot = ( ::read ( m_iFD, &uCmd, sizeof(uCmd) )==sizeof(uCmd) );
	return ( iGot>0 );
}

bool AuthReloadSignalled ( int iPid )
{
	// if session is already active - ignore the new signal
	if ( g_pIpcSession )
	{
		sphWarning ( "ignore auth reload signal; a session is already in progress" );
		return true;
	}

	sphLogDebug ( "SIGUSR1: auth reload pipe detected, processing command..." );

	// check existence of the reply pipe - without opening it
#if !_WIN32
	if ( access ( GetCommandPipeName ( iPid, ReloadAuthCmd_e::REPLY ).cstr(), F_OK ) != 0 )
		return false; // pipe does not exist - normal log rotation
#endif

	g_pIpcSession = std::make_unique<IpcSession> ( iPid );

	if ( !g_pIpcSession->m_tPipeReply.IsOpen() )
	{
		sphWarning ( "failed to open reply pipe: %s", strerrorm ( g_pIpcSession->m_tPipeReply.GetError() ) );
		g_pIpcSession.reset();
		return true; // consume signal
	}

	if ( !g_pIpcSession->m_tPipeReq.IsOpen() )
	{
		sphWarning ( "failed to open request pipe: %s", strerrorm ( g_pIpcSession->m_tPipeReq.GetError() ) );
		g_pIpcSession->m_tPipeReply.WriteStatus ( 2 ); // try to report failure
		g_pIpcSession.reset();
		return true; // consume signal
	}

	bool bSuccess = false;
	CSphString sError;

	DWORD uCmd = 0;
	if ( !g_pIpcSession->m_tPipeReq.ReadCommand ( uCmd ) )
	{
		sphWarning ( "failed to read full command from request pipe" );

	} else if ( uCmd!=CMD_HEADER_RELOAD_AUTH )
	{
		sphWarning ( "received unknown command '0x%X' via pipe", uCmd );

	} else
	{
		bSuccess = AuthReload ( sError );
		if ( !bSuccess )
			sphWarning ( "AUTH RELOAD failed: %s", sError.cstr() );
		else
			sphLogDebug ( "AUTH RELOAD successful" );
	}

	// send the reply and keep session alive until the client disconnects
	g_pIpcSession->m_tPipeReply.WriteStatus ( bSuccess ? 1 : 2 );
	
	return true; // consume signal
}

void ManageIpcSession()
{
	if ( !g_pIpcSession )
		return;

	int iFD = g_pIpcSession->m_tPipeReq.GetFD();
	// poll pipe with zero \ non-blocking timeout
	// POLLHUP event- client has closed the pipe
	struct pollfd pfd;
#if _WIN32
	pfd.fd = (SOCKET)iFD;
#else
	pfd.fd = iFD;
#endif
	pfd.events = 0;
	pfd.revents = 0;
	poll ( &pfd, 1, 0 );

	// if the client has hung up, the session is over
	if ( pfd.revents & POLLHUP )
	{
		sphLogDebug ( "client disconnected, ending IPC session" );
		g_pIpcSession.reset(); // close the pipes
	}
}

ReloadAuthResult_e CommandReloadAuthAndWaitReplyWin ( int iPid, int iWaitTimeout, CSphString & sError )
{
	ReloadAuthResult_e iExitCode = ReloadAuthResult_e::IPC_ERROR;

#if _WIN32
	HANDLE hPipeReq = INVALID_HANDLE_VALUE;
	HANDLE hPipeReply = INVALID_HANDLE_VALUE;
	HANDLE hPipeSignal = INVALID_HANDLE_VALUE;
	AT_SCOPE_EXIT( [&]
	{
		if ( hPipeReq!=INVALID_HANDLE_VALUE )
		{
			CloseHandle ( hPipeReq );
			hPipeReq = INVALID_HANDLE_VALUE;
		}
		if ( hPipeReply!=INVALID_HANDLE_VALUE )
		{
			CloseHandle ( hPipeReply );
			hPipeReply = INVALID_HANDLE_VALUE;
		}
		if ( hPipeSignal!=INVALID_HANDLE_VALUE )
		{
			CloseHandle ( hPipeSignal );
			hPipeSignal = INVALID_HANDLE_VALUE;
		}
	}); 

	hPipeReq = CreateNamedPipe ( GetCommandPipeNameWin ( iPid, ReloadAuthCmd_e::REQ ).cstr(), PIPE_ACCESS_OUTBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, 0, sizeof(DWORD), iWaitTimeout, NULL );
	if ( hPipeReq==INVALID_HANDLE_VALUE )
	{
		sError.SetSprintf ( "failed to create request pipe: %s", WinErrorInfo() );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}

	hPipeReply = CreateNamedPipe ( GetCommandPipeNameWin ( iPid, ReloadAuthCmd_e::REPLY ).cstr(), PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_WAIT, 1, sizeof(DWORD), 0, iWaitTimeout, NULL );
	if ( hPipeReply==INVALID_HANDLE_VALUE )
	{
		sError.SetSprintf ( "failed to create reply pipe: %s", WinErrorInfo() );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}

	// send wake-up to daemon main signal pipe
	CSphString sSignalPipeName = GetSignalPipeName ( iPid );
	hPipeSignal = CreateFile ( sSignalPipeName.cstr(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
	if ( hPipeSignal==INVALID_HANDLE_VALUE && GetLastError()==ERROR_PIPE_BUSY )
	{
		if ( WaitNamedPipe ( sSignalPipeName.cstr(), iWaitTimeout ) )
			hPipeSignal = CreateFile ( sSignalPipeName.cstr(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );
	}
	
	if ( hPipeSignal==INVALID_HANDLE_VALUE )
	{
		sError.SetSprintf ( "failed to open daemon signal pipe: %s", WinErrorInfo() );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}
	
	DWORD uWritten = 0;
	BYTE uSignal = PIPE_CMD_CHECK_CMD;
	if ( !WriteFile ( hPipeSignal, &uSignal, 1, &uWritten, NULL ) || uWritten!=1 )
	{
		sError.SetSprintf ( "failed to send wake-up signal: %s", WinErrorInfo() );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}
	CloseHandle ( hPipeSignal );
	hPipeSignal = INVALID_HANDLE_VALUE;

	// wait daemon to connect to our pipes
	if ( !ConnectNamedPipe ( hPipeReq, NULL ) && GetLastError()!=ERROR_PIPE_CONNECTED )
	{
		sError.SetSprintf ( "daemon failed to connect to request pipe: %s", WinErrorInfo() );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}

	if ( !ConnectNamedPipe ( hPipeReply, NULL ) && GetLastError()!=ERROR_PIPE_CONNECTED )
	{
		sError.SetSprintf ( "daemon failed to connect to reply pipe: %s", WinErrorInfo() );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}

	// write the actual command to the request pipe
	DWORD uCmd = CMD_HEADER_RELOAD_AUTH;
	if ( !WriteFile ( hPipeReq, &uCmd, sizeof(uCmd), &uWritten, NULL ) || uWritten!=sizeof (uCmd) )
	{
		sError.SetSprintf ( "failed to write command to pipe: %s", WinErrorInfo() );
		return ReloadAuthResult_e::CLIENT_ERROR;
	}
	
	// wait reply
	DWORD uStatus = 0;
	DWORD uRead = 0;
	if ( ReadFile ( hPipeReply, &uStatus, sizeof(uStatus), &uRead, NULL ) && uRead==sizeof(uStatus) )
	{
		iExitCode = ( uStatus==1 ? ReloadAuthResult_e::SUCCESS : ReloadAuthResult_e::DAEMON_ERROR ); // 0 on success, 2 on daemon-side failure
	} else
	{
		sError.SetSprintf ( "failed to read reply from daemon: %s", WinErrorInfo() );
	}
#endif

	return iExitCode;
}

void ProcessAuthReloadPipe ( int iPid )
{
#if _WIN32
	sphLogDebug ( "received wake-up signal for command pipe" );

	HANDLE hPipeReq = INVALID_HANDLE_VALUE;
	HANDLE hPipeReply = INVALID_HANDLE_VALUE;
	AT_SCOPE_EXIT( [&]
	{
		if ( hPipeReq!=INVALID_HANDLE_VALUE )
		{
			CloseHandle ( hPipeReq );
			hPipeReq = INVALID_HANDLE_VALUE;
		}
		if ( hPipeReply!=INVALID_HANDLE_VALUE )
		{
			CloseHandle ( hPipeReply );
			hPipeReply = INVALID_HANDLE_VALUE;
		}
	}); 

	// connect to the command pipes as client
	hPipeReq = CreateFile ( GetCommandPipeNameWin ( iPid, ReloadAuthCmd_e::REQ ).cstr(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL );
	hPipeReply = CreateFile ( GetCommandPipeNameWin ( iPid, ReloadAuthCmd_e::REPLY ).cstr(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );

	if ( hPipeReq==INVALID_HANDLE_VALUE || hPipeReply==INVALID_HANDLE_VALUE )
	{
		sphWarning ( "failed to open command pipes (req: %p, reply: %p): %s", hPipeReq, hPipeReply, WinErrorInfo() );
		return;
	}

	// connected - read the command \ execute \ reply
	DWORD uCmd = 0;
	DWORD uRead = 0;
	bool bSuccess = false;
	
	if ( !ReadFile ( hPipeReq, &uCmd, sizeof(uCmd), &uRead, NULL ) || uRead != sizeof(uCmd) )
	{
		sphWarning ( "failed to read full command from request pipe: %s", WinErrorInfo() );
	}
	else if ( uCmd != CMD_HEADER_RELOAD_AUTH )
	{
		sphWarning ( "received unknown command '0x%X' via pipe", uCmd );
	}
	else
	{
		CSphString sError;
		bSuccess = AuthReload ( sError );
		if ( !bSuccess )
			sphWarning ( "AUTH RELOAD failed: %s", sError.cstr() );
		else
			sphLogDebug ( "AUTH RELOAD successful" );
	}
	
	DWORD uStatus = bSuccess ? 1 : 2; // 1 for success, 2 for failure
	DWORD uWritten = 0;
	if ( !WriteFile ( hPipeReply, &uStatus, sizeof(uStatus), &uWritten, NULL ) )
		sphWarning ( "failed to send AUTH RELOAD reply: %s", WinErrorInfo() );
#endif
}

ReloadAuthResult_e CommandReloadAuthAndWaitReply ( int iPid, int iWaitTimeout, CSphString & sError )
{
#if _WIN32
	return CommandReloadAuthAndWaitReplyWin ( iPid, iWaitTimeout, sError );
#else
	return CommandReloadAuthAndWaitReplyUnix ( iPid, iWaitTimeout, sError );
#endif
}