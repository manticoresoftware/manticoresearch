// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)

#include "manticorecli.h"

#include "fileutils.h"
#include "manticoreclient.h"
#include "searchdaemon.h"
#include "sphinxjson.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#if !_WIN32
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#else
#include <io.h>
#include <process.h>
#endif

namespace manticorecli
{
namespace
{
constexpr const char * LOCAL_DATA_DIR = "manticore_data";
constexpr const char * LOCAL_LOG = "searchd.log";
constexpr const char * LOCAL_PID = "searchd.pid";
constexpr const char * LOCAL_SOCKET = "searchd.sock";
constexpr const char * SERVICE_NAME = "manticore.service";
constexpr int DEFAULT_START_TIMEOUT_SEC = 300;

#if !_WIN32
volatile sig_atomic_t g_bInterrupted = 0;

void HandleInterrupt ( int )
{
	g_bInterrupted = 1;
}

class InterruptGuard_c
{
	struct sigaction m_tOld {};
	bool m_bInstalled = false;

public:
	InterruptGuard_c()
	{
		g_bInterrupted = 0;
		struct sigaction tAction {};
		tAction.sa_handler = HandleInterrupt;
		sigemptyset ( &tAction.sa_mask );
		m_bInstalled = sigaction ( SIGINT, &tAction, &m_tOld )==0;
	}

	~InterruptGuard_c()
	{
		if ( m_bInstalled )
			sigaction ( SIGINT, &m_tOld, nullptr );
	}
};
#endif

ParseResult_t Error ( const char * szMessage )
{
	ParseResult_t tResult;
	tResult.m_sError = szMessage;
	return tResult;
}

bool IsOption ( const char * szArg, const char * szShort, const char * szLong )
{
	return strcmp ( szArg, szShort )==0 || strcmp ( szArg, szLong )==0;
}

std::string Trim ( std::string sValue )
{
	while ( !sValue.empty() && ( sValue.back()=='\n' || sValue.back()=='\r' || sValue.back()==' ' || sValue.back()=='\t' ) )
		sValue.pop_back();
	size_t iStart = 0;
	while ( iStart<sValue.size() && ( sValue[iStart]==' ' || sValue[iStart]=='\t' || sValue[iStart]=='\n' || sValue[iStart]=='\r' ) )
		++iStart;
	return sValue.substr ( iStart );
}

#if !_WIN32
std::string JoinPath ( const std::string & sDir, const char * szName )
{
	if ( sDir.empty() || sDir=="." )
		return std::string("./") + szName;
	return sDir.back()=='/' ? sDir+szName : sDir+'/'+szName;
}

bool IsExecutable ( const std::string & sPath )
{
	return !sPath.empty() && access ( sPath.c_str(), X_OK )==0;
}

std::string FindOnPath ( const char * szName )
{
	if ( !szName || !*szName )
		return {};
	if ( strchr ( szName, '/' ) )
		return IsExecutable(szName) ? szName : std::string();
	const char * szPath = getenv ( "PATH" );
	if ( !szPath )
		return {};
	std::string sPath = szPath;
	size_t iStart = 0;
	while ( iStart<=sPath.size() )
	{
		size_t iEnd = sPath.find ( ':', iStart );
		std::string sDir = sPath.substr ( iStart, iEnd==std::string::npos ? std::string::npos : iEnd-iStart );
		std::string sCandidate = JoinPath ( sDir.empty() ? "." : sDir, szName );
		if ( IsExecutable(sCandidate) )
			return sCandidate;
		if ( iEnd==std::string::npos )
			break;
		iStart = iEnd+1;
	}
	return {};
}

std::string DirectoryName ( const std::string & sPath )
{
	size_t iSlash = sPath.rfind ( '/' );
	return iSlash==std::string::npos ? "." : iSlash ? sPath.substr ( 0, iSlash ) : "/";
}

std::string ResolveExecutable ( const char * szArgv0 )
{
	std::string sPath = FindOnPath ( szArgv0 );
	if ( sPath.empty() && szArgv0 && strchr(szArgv0,'/') )
		sPath = szArgv0;
	char szResolved[PATH_MAX];
	return !sPath.empty() && realpath ( sPath.c_str(), szResolved ) ? szResolved : sPath;
}

std::string FindSearchd ( const char * szArgv0 )
{
	std::string sSelf = ResolveExecutable ( szArgv0 );
	if ( !sSelf.empty() )
	{
		std::string sSibling = JoinPath ( DirectoryName(sSelf), "searchd" );
		if ( IsExecutable(sSibling) )
			return sSibling;
	}
#ifdef MANTICORE_INSTALL_BINDIR
	if ( IsExecutable(MANTICORE_INSTALL_BINDIR "/searchd") )
		return MANTICORE_INSTALL_BINDIR "/searchd";
#endif
	return FindOnPath ( "searchd" );
}

int DecodeStatus ( int iStatus )
{
	if ( WIFEXITED(iStatus) )
		return WEXITSTATUS(iStatus);
	if ( WIFSIGNALED(iStatus) )
		return 128+WTERMSIG(iStatus);
	return 1;
}

struct ChildResult_t
{
	int m_iExit = 1;
	std::string m_sOutput;
};

ChildResult_t RunCapture ( const std::vector<std::string> & dArgs, bool bMergeStderr=true, int64_t iDeadlineUS=0 )
{
	ChildResult_t tResult;
	if ( dArgs.empty() )
		return tResult;
	if ( !iDeadlineUS )
		iDeadlineUS = sphMicroTimer()+5000000;
	int dPipe[2];
	if ( pipe(dPipe)<0 )
	{
		tResult.m_sOutput = strerror(errno);
		return tResult;
	}
	pid_t iPid = fork();
	if ( iPid<0 )
	{
		close ( dPipe[0] );
		close ( dPipe[1] );
		tResult.m_sOutput = strerror(errno);
		return tResult;
	}
	if ( !iPid )
	{
		setpgid ( 0, 0 );
		close ( dPipe[0] );
		dup2 ( dPipe[1], STDOUT_FILENO );
		if ( bMergeStderr )
			dup2 ( dPipe[1], STDERR_FILENO );
		else
		{
			int iNull = open ( "/dev/null", O_WRONLY );
			if ( iNull>=0 )
			{
				dup2 ( iNull, STDERR_FILENO );
				close ( iNull );
			}
		}
		close ( dPipe[1] );
		std::vector<char *> dArgv;
		for ( const std::string & sArg : dArgs )
			dArgv.push_back ( const_cast<char *>(sArg.c_str()) );
		dArgv.push_back ( nullptr );
		if ( strchr(dArgv[0],'/') ) execv ( dArgv[0], dArgv.data() ); else execvp ( dArgv[0], dArgv.data() );
		dprintf ( STDERR_FILENO, "cannot execute %s: %s\n", dArgv[0], strerror(errno) );
		_Exit ( errno==ENOENT ? 127 : 126 );
	}

	setpgid ( iPid, iPid );
	close ( dPipe[1] );
	int iFlags = fcntl ( dPipe[0], F_GETFL, 0 );
	if ( iFlags>=0 ) fcntl ( dPipe[0], F_SETFL, iFlags|O_NONBLOCK );
	bool bPipeOpen = true;
	bool bChildDone = false;
	bool bWaitFailed = false;
	int iStatus = 0;
	char szBuffer[4096];
	auto CheckChildDone = [&]()
	{
		siginfo_t tInfo {};
		if ( waitid(P_PID,(id_t)iPid,&tInfo,WEXITED|WNOHANG|WNOWAIT)==0 )
		{
			if ( tInfo.si_pid==iPid )
				bChildDone = true;
			return;
		}
		if ( errno==EINTR )
			return;
		bWaitFailed = true;
		tResult.m_sOutput += "waitid failed: ";
		tResult.m_sOutput += strerror(errno);
		tResult.m_sOutput += '\n';
	};
	while ( bPipeOpen || !bChildDone )
	{
		while ( bPipeOpen )
		{
			ssize_t iRead = read ( dPipe[0], szBuffer, sizeof(szBuffer) );
			if ( iRead>0 )
				tResult.m_sOutput.append ( szBuffer, (size_t)iRead );
			else if ( iRead==0 )
				bPipeOpen = false;
			else if ( errno==EINTR )
				continue;
			else if ( errno==EAGAIN || errno==EWOULDBLOCK )
				break;
			else
			{
				bPipeOpen = false;
				tResult.m_sOutput += "read failed: ";
				tResult.m_sOutput += strerror(errno);
				tResult.m_sOutput += '\n';
			}
		}
		if ( !bChildDone )
			CheckChildDone();
		if ( !bPipeOpen && bChildDone ) break;
		int64_t iRemaining = iDeadlineUS-sphMicroTimer();
		if ( iRemaining<=0 )
		{
			kill ( -iPid, SIGTERM );
			kill ( iPid, SIGTERM );
			for ( int i=0; i<10 && !bChildDone; ++i )
			{
				CheckChildDone();
				if ( !bChildDone ) usleep ( 10000 );
			}
			// The leader may exit after SIGTERM while a TERM-ignoring
			// descendant still owns the capture pipe. Always clean the whole
			// operation-owned process group before reaping the leader.
			kill ( -iPid, SIGKILL );
			if ( !bChildDone )
				kill ( iPid, SIGKILL );
			pid_t iWait = -1;
			do { iWait = waitpid ( iPid, &iStatus, 0 ); } while ( iWait<0 && errno==EINTR );
			tResult.m_sOutput += "helper command timed out\n";
			tResult.m_iExit = 124;
			close ( dPipe[0] );
			return tResult;
		}
		pollfd tPoll { dPipe[0], POLLIN|POLLHUP, 0 };
		poll ( &tPoll, 1, (int)std::min<int64_t>((iRemaining+999)/1000,50) );
	}
	close ( dPipe[0] );
	pid_t iWait = -1;
	do { iWait = waitpid ( iPid, &iStatus, 0 ); } while ( iWait<0 && errno==EINTR );
	if ( iWait!=iPid )
		bWaitFailed = true;
	tResult.m_iExit = !bWaitFailed ? DecodeStatus(iStatus) : 1;
	return tResult;
}

int RunPassthrough ( const std::vector<std::string> & dArgs )
{
	if ( dArgs.empty() )
		return 1;
	pid_t iPid = fork();
	if ( iPid<0 )
	{
		fprintf ( stderr, "manticore: cannot start '%s': %s\n", dArgs[0].c_str(), strerror(errno) );
		return 1;
	}
	if ( !iPid )
	{
		std::vector<char *> dArgv;
		for ( const std::string & sArg : dArgs )
			dArgv.push_back ( const_cast<char *>(sArg.c_str()) );
		dArgv.push_back ( nullptr );
		if ( strchr(dArgv[0],'/') ) execv ( dArgv[0], dArgv.data() ); else execvp ( dArgv[0], dArgv.data() );
		dprintf ( STDERR_FILENO, "manticore: cannot execute %s: %s\n", dArgv[0], strerror(errno) );
		_Exit ( errno==ENOENT ? 127 : 126 );
	}
	int iStatus = 0;
	pid_t iWait = -1;
	do { iWait = waitpid ( iPid, &iStatus, 0 ); } while ( iWait<0 && errno==EINTR );
	if ( iWait<0 )
	{
		fprintf ( stderr, "manticore: waitpid failed for '%s': %s\n", dArgs[0].c_str(), strerror(errno) );
		return 1;
	}
	return DecodeStatus ( iStatus );
}

class Progress_c
{
	bool m_bTty = isatty(STDOUT_FILENO);
	std::string m_sAction;
	std::string m_sLog;
	off_t m_iLogOffset = 0;
	size_t m_iFrame = 0;
	std::string m_sChildStdoutPending;
	std::string m_sChildStderrPending;
	std::string m_sJournalctl;
	std::string m_sJournalCursor;
	size_t m_iJournalPoll = 0;

	void ClearSpinner()
	{
		if ( m_bTty )
			fputs ( "\r\033[2K", stdout );
	}

	void PrintLine ( FILE * pStream, const char * szPrefix, const std::string & sLine )
	{
		if ( sLine.empty() )
			return;
		ClearSpinner();
		fprintf ( pStream, "%s%s\n", szPrefix, sLine.c_str() );
		fflush ( pStream );
	}

	void PollJournal ( bool bForce = false )
	{
		if ( m_sJournalctl.empty() || m_sJournalCursor.empty() || ( !bForce && ++m_iJournalPoll%8 ) )
			return;
		ChildResult_t tJournal = RunCapture ( { m_sJournalctl, "-u", SERVICE_NAME, "--after-cursor", m_sJournalCursor, "--show-cursor", "--no-pager", "-o", "cat" } );
		if ( tJournal.m_iExit )
			return;
		size_t iCursor = tJournal.m_sOutput.rfind ( "-- cursor: " );
		if ( iCursor==std::string::npos )
			return;
		std::string sLines = tJournal.m_sOutput.substr ( 0, iCursor );
		size_t iCursorEnd = tJournal.m_sOutput.find ( '\n', iCursor+11 );
		m_sJournalCursor = Trim ( tJournal.m_sOutput.substr ( iCursor+11, iCursorEnd==std::string::npos ? std::string::npos : iCursorEnd-(iCursor+11) ) );
		size_t iStart = 0;
		while ( iStart<sLines.size() )
		{
			size_t iEnd = sLines.find ( '\n', iStart );
			std::string sLine = sLines.substr ( iStart, iEnd==std::string::npos ? std::string::npos : iEnd-iStart );
			if ( !sLine.empty() && sLine.back()=='\r' ) sLine.pop_back();
			PrintLine ( stdout, "[systemd] ", sLine );
			if ( iEnd==std::string::npos ) break;
			iStart = iEnd+1;
		}
	}

public:
	Progress_c ( std::string sAction, std::string sLog )
		: m_sAction ( std::move(sAction) ), m_sLog ( std::move(sLog) )
	{
		struct stat tStat {};
		if ( !m_sLog.empty() && stat ( m_sLog.c_str(), &tStat )==0 )
			m_iLogOffset = tStat.st_size;
		if ( !m_bTty )
		{
			fprintf ( stdout, "%s...\n", m_sAction.c_str() );
			fflush ( stdout );
		}
	}

	void FollowSystemdJournal()
	{
		m_sJournalctl = FindOnPath ( "journalctl" );
		if ( m_sJournalctl.empty() )
			return;
		ChildResult_t tCursor = RunCapture ( { m_sJournalctl, "-u", SERVICE_NAME, "-n", "0", "--show-cursor", "--no-pager" } );
		if ( tCursor.m_iExit )
		{
			m_sJournalctl.clear();
			return;
		}
		size_t iCursor = tCursor.m_sOutput.rfind ( "-- cursor: " );
		if ( iCursor==std::string::npos )
		{
			m_sJournalctl.clear();
			return;
		}
		size_t iCursorEnd = tCursor.m_sOutput.find ( '\n', iCursor+11 );
		m_sJournalCursor = Trim ( tCursor.m_sOutput.substr ( iCursor+11, iCursorEnd==std::string::npos ? std::string::npos : iCursorEnd-(iCursor+11) ) );
	}

	void Tick()
	{
		PollLog();
		PollJournal();
		if ( !m_bTty )
			return;
		static constexpr const char * dFrames[] = { "⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏" };
		fprintf ( stdout, "\r\033[2K%s %s", dFrames[m_iFrame++ % 10], m_sAction.c_str() );
		fflush ( stdout );
	}

	void ChildData ( const char * pData, size_t iLength, const char * szPrefix, bool bError = false )
	{
		std::string & sPending = bError ? m_sChildStderrPending : m_sChildStdoutPending;
		sPending.append ( pData, iLength );
		while ( true )
		{
			size_t iEnd = sPending.find ( '\n' );
			if ( iEnd==std::string::npos )
				break;
			std::string sLine = sPending.substr ( 0, iEnd );
			if ( !sLine.empty() && sLine.back()=='\r' ) sLine.pop_back();
			PrintLine ( bError ? stderr : stdout, szPrefix, sLine );
			sPending.erase ( 0, iEnd+1 );
		}
	}

	void FlushChild ( const char * szPrefix, bool bError = false )
	{
		std::string & sPending = bError ? m_sChildStderrPending : m_sChildStdoutPending;
		if ( !sPending.empty() )
			PrintLine ( bError ? stderr : stdout, szPrefix, sPending );
		sPending.clear();
	}

	void PollLog()
	{
		if ( m_sLog.empty() )
			return;
		FILE * pFile = fopen ( m_sLog.c_str(), "r" );
		if ( !pFile )
			return;
		if ( fseeko ( pFile, m_iLogOffset, SEEK_SET )<0 )
		{
			fclose ( pFile );
			return;
		}
		char * pLine = nullptr;
		size_t iCapacity = 0;
		ssize_t iLength = 0;
		while ( ( iLength=getline(&pLine,&iCapacity,pFile) )>=0 )
		{
			std::string sLine ( pLine, (size_t)iLength );
			while ( !sLine.empty() && ( sLine.back()=='\n' || sLine.back()=='\r' ) ) sLine.pop_back();
			PrintLine ( stdout, "[searchd] ", sLine );
		}
		free ( pLine );
		m_iLogOffset = ftello ( pFile );
		fclose ( pFile );
	}

	void Success ( const std::string & sMessage )
	{
		PollLog();
		PollJournal ( true );
		ClearSpinner();
		fprintf ( stdout, "✓ %s\n", sMessage.c_str() );
	}

	void Failure()
	{
		PollLog();
		PollJournal ( true );
		ClearSpinner();
	}
};

int RunProgressProcess ( const std::vector<std::string> & dArgs, Progress_c & tProgress, const char * szPrefix )
{
	int dOut[2] { -1, -1 }, dErr[2] { -1, -1 };
	if ( pipe(dOut)<0 || pipe(dErr)<0 )
	{
		if ( dOut[0]>=0 ) { close(dOut[0]); close(dOut[1]); }
		fprintf ( stderr, "manticore: cannot create process pipe: %s\n", strerror(errno) );
		return 1;
	}
	pid_t iPid = fork();
	if ( iPid<0 )
	{
		close ( dOut[0] ); close ( dOut[1] ); close ( dErr[0] ); close ( dErr[1] );
		fprintf ( stderr, "manticore: cannot start '%s': %s\n", dArgs[0].c_str(), strerror(errno) );
		return 1;
	}
	if ( !iPid )
	{
		setpgid ( 0, 0 );
		signal ( SIGINT, SIG_DFL );
		close ( dOut[0] ); close ( dErr[0] );
		dup2 ( dOut[1], STDOUT_FILENO );
		dup2 ( dErr[1], STDERR_FILENO );
		close ( dOut[1] ); close ( dErr[1] );
		std::vector<char *> dArgv;
		for ( const std::string & sArg : dArgs ) dArgv.push_back ( const_cast<char *>(sArg.c_str()) );
		dArgv.push_back ( nullptr );
		if ( strchr(dArgv[0],'/') ) execv ( dArgv[0], dArgv.data() ); else execvp ( dArgv[0], dArgv.data() );
		dprintf ( STDERR_FILENO, "cannot execute %s: %s\n", dArgv[0], strerror(errno) );
		_Exit ( errno==ENOENT ? 127 : 126 );
	}

	setpgid ( iPid, iPid );
	close ( dOut[1] ); close ( dErr[1] );
	fcntl ( dOut[0], F_SETFL, fcntl(dOut[0],F_GETFL) | O_NONBLOCK );
	fcntl ( dErr[0], F_SETFL, fcntl(dErr[0],F_GETFL) | O_NONBLOCK );
	auto ReadAvailable = [&] ( int iFd, bool bError )
	{
		char szBuffer[4096];
		while ( true )
		{
			ssize_t iRead = read ( iFd, szBuffer, sizeof(szBuffer) );
			if ( iRead>0 ) tProgress.ChildData ( szBuffer, (size_t)iRead, szPrefix, bError ); else break;
		}
	};
	auto DetachDrainer = [] ( int iFd )
	{
		pid_t iDrainer = fork();
		if ( !iDrainer )
		{
			setpgid ( 0, 0 );
			signal ( SIGINT, SIG_IGN );
			int iFlags = fcntl ( iFd, F_GETFL );
			if ( iFlags>=0 ) fcntl ( iFd, F_SETFL, iFlags & ~O_NONBLOCK );
			char szDrain[4096];
			while ( true )
			{
				ssize_t iRead = read ( iFd, szDrain, sizeof(szDrain) );
				if ( iRead>0 || ( iRead<0 && errno==EINTR ) ) continue;
				break;
			}
			close ( iFd );
			_Exit ( 0 );
		}
		close ( iFd );
	};

	int iStatus = 0;
	while ( true )
	{
		ReadAvailable ( dOut[0], false );
		ReadAvailable ( dErr[0], true );
		pid_t iWait = waitpid ( iPid, &iStatus, WNOHANG );
		if ( iWait==iPid )
			break;
		if ( g_bInterrupted )
		{
			// Keep consuming detached backend output so closing this process does not
			// terminate a still-running systemctl/searchd through SIGPIPE.
			DetachDrainer ( dOut[0] );
			DetachDrainer ( dErr[0] );
			return 130;
		}
		if ( iWait<0 && errno!=EINTR )
		{
			iStatus = 1 << 8;
			break;
		}
		tProgress.Tick();
		usleep ( 80000 );
	}
	ReadAvailable ( dOut[0], false );
	ReadAvailable ( dErr[0], true );
	close ( dOut[0] ); close ( dErr[0] );
	tProgress.FlushChild ( szPrefix, false );
	tProgress.FlushChild ( szPrefix, true );
	tProgress.PollLog();
	return DecodeStatus ( iStatus );
}

bool ProcessAlive ( int iPid )
{
	if ( iPid<=0 || kill(iPid,0)<0 )
		return false;
#if __linux__
	std::string sStat = "/proc/" + std::to_string(iPid) + "/stat";
	FILE * pFile = fopen ( sStat.c_str(), "r" );
	if ( pFile )
	{
		int iReadPid = 0;
		char sName[512] {};
		char cState = 0;
		bool bRead = fscanf ( pFile, "%d %511s %c", &iReadPid, sName, &cState )==3;
		fclose ( pFile );
		if ( bRead && cState=='Z' )
			return false;
	}
#endif
	return kill(iPid,0)==0;
}

bool ReadPid ( const std::string & sPath, int & iPid, std::string & sError )
{
	iPid = 0;
	struct stat tStat {};
	if ( lstat(sPath.c_str(),&tStat)<0 )
	{
		if ( errno==ENOENT ) return false;
		sError = "cannot inspect PID file '" + sPath + "': " + strerror(errno);
		return false;
	}
	int iFile = -1;
	CSphString sPidError;
	if ( !OpenPidFile(CSphString(sPath.c_str()),iFile,iPid,sPidError) )
	{
		sError = sPidError.cstr();
		return false;
	}
	SafeClose ( iFile );
	return true;
}

bool PidFileOwnedBy ( const std::string & sPath, int iPid, std::string & sError )
{
	int iFile = -1;
	int iRecordedPid = 0;
	CSphString sPidError;
	if ( !OpenPidFile(CSphString(sPath.c_str()),iFile,iRecordedPid,sPidError) )
	{
		sError = sPidError.cstr();
		return false;
	}
	AT_SCOPE_EXIT ( [&] { SafeClose(iFile); } );
	if ( iRecordedPid!=iPid )
	{
		sError = "PID file '" + sPath + "' changed while it was being validated";
		return false;
	}
	if ( !ValidatePidFileOwner(iFile,iPid,sPidError) )
	{
		sError = sPidError.cstr();
		return false;
	}
	return true;
}

int StartTimeoutSec()
{
	const char * szValue = getenv ( "MANTICORE_START_TIMEOUT" );
	if ( !szValue || !*szValue )
		return DEFAULT_START_TIMEOUT_SEC;
	char * pEnd = nullptr;
	errno = 0;
	long iValue = strtol ( szValue, &pEnd, 10 );
	return !errno && pEnd && !*pEnd && iValue>0 && iValue<=INT_MAX ? (int)iValue : DEFAULT_START_TIMEOUT_SEC;
}

struct InstancePaths_t
{
	std::string m_sLog;
	std::string m_sPid;
};

InstancePaths_t LocalPaths()
{
	return { JoinPath(LOCAL_DATA_DIR,LOCAL_LOG), JoinPath(LOCAL_DATA_DIR,LOCAL_PID) };
}

InstancePaths_t GlobalPaths()
{
	InstancePaths_t tPaths;
	CSphString sConfig = sphGetConfigFile ( nullptr );
	auto dConfig = FetchAndCheckIfChanged ( sConfig ).second;
	CSphConfig hConf;
	if ( !ParseConfig ( &hConf, sConfig, dConfig ) || !hConf.Exists("searchd") || !hConf["searchd"].Exists("searchd") )
		return tPaths;
	const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];
	if ( CSphVariant * pLog = hSearchd("log") )
		tPaths.m_sLog = pLog->cstr();
	if ( CSphVariant * pPid = hSearchd("pid_file") )
		tPaths.m_sPid = pPid->cstr();
	return tPaths;
}

bool SystemdAvailable ( std::string & sSystemctl )
{
	sSystemctl = FindOnPath ( "systemctl" );
	if ( sSystemctl.empty() )
		return false;
	ChildResult_t tManager = RunCapture ( { sSystemctl, "show", "--property=Version", "--value" }, false );
	if ( tManager.m_iExit )
		return false;
	ChildResult_t tUnit = RunCapture ( { sSystemctl, "show", "--property=LoadState", "--value", SERVICE_NAME }, false );
	std::string sLoadState = Trim ( tUnit.m_sOutput );
	return !tUnit.m_iExit && !sLoadState.empty() && sLoadState!="not-found";
}

std::string SystemdStateDiagnostic ( const std::string & sSystemctl )
{
	ChildResult_t tState = RunCapture ( { sSystemctl, "show", "--property=ActiveState", "--property=SubState", "--property=Result", SERVICE_NAME } );
	std::string sOutput = Trim ( tState.m_sOutput );
	return sOutput.empty() ? "systemd unit state could not be read" : sOutput;
}

bool SystemdInactive ( const std::string & sSystemctl, int64_t iDeadlineUS=0 )
{
	ChildResult_t tState = RunCapture ( { sSystemctl, "show", "--property=ActiveState", "--value", SERVICE_NAME }, true, iDeadlineUS );
	return !tState.m_iExit && Trim(tState.m_sOutput)=="inactive";
}

ManticoreClientTarget_e ClientTarget ( Target_e eTarget )
{
	return eTarget==Target_e::LOCAL ? ManticoreClientTarget_e::LOCAL : eTarget==Target_e::GLOBAL ? ManticoreClientTarget_e::GLOBAL : ManticoreClientTarget_e::AUTO;
}

bool ValidateLocalMarker ( Marker_e eMarker, Command_e eCommand, Target_e eRequested, const std::string & sMarkerError )
{
	if ( eMarker==Marker_e::DIRECTORY )
		return true;
	if ( eMarker==Marker_e::ABSENT && eCommand==Command_e::START && eRequested==Target_e::LOCAL )
		return true;
	if ( eMarker==Marker_e::ABSENT )
		fprintf ( stderr, "manticore: local data directory './%s' does not exist\n", LOCAL_DATA_DIR );
	else
		fprintf ( stderr, "manticore: %s\n", sMarkerError.c_str() );
	return false;
}

bool WaitReadyWithProgress ( ManticoreClientTarget_e eTarget, int iPid, const std::string & sSystemctl, Progress_c & tProgress, std::string & sError )
{
	const int64_t iDeadline = sphMicroTimer() + (int64_t)StartTimeoutSec()*1000000;
	while ( !ProbeManticoreReady ( eTarget, nullptr, sError, iDeadline ) )
	{
		if ( g_bInterrupted )
		{
			sError = "waiting was interrupted; Manticore Search may still be starting";
			return false;
		}
		if ( iPid>0 && !ProcessAlive(iPid) )
		{
			sError = "daemon exited before SQL readiness";
			return false;
		}
		if ( !sSystemctl.empty() )
		{
			ChildResult_t tState = RunCapture ( { sSystemctl, "is-active", "--quiet", SERVICE_NAME }, true, iDeadline );
			if ( tState.m_iExit )
			{
				if ( tState.m_iExit==124 )
					sError = "startup readiness timed out: systemd readiness check timed out";
				else if ( SystemdInactive(sSystemctl,iDeadline) )
					sError = "manticore.service stopped before SQL readiness";
				else
					sError = "could not verify manticore.service state during SQL readiness";
				return false;
			}
		}
		if ( sphMicroTimer()>=iDeadline )
		{
			sError = "startup readiness timed out: " + sError;
			return false;
		}
		tProgress.Tick();
		usleep ( 80000 );
	}
	return true;
}

int RunLocalStatus()
{
	std::string sMarkerError;
	Marker_e eMarker = InspectLocalMarker ( ".", sMarkerError );
	if ( eMarker!=Marker_e::DIRECTORY )
	{
		fprintf ( stderr, "manticore: %s\n", eMarker==Marker_e::ABSENT ? "local data directory './manticore_data' does not exist" : sMarkerError.c_str() );
		return 1;
	}
	InstancePaths_t tPaths = LocalPaths();
	int iPid = 0;
	std::string sPidError;
	bool bPid = ReadPid ( tPaths.m_sPid, iPid, sPidError );
	bool bAlive = bPid && ProcessAlive(iPid);
	bool bOwned = bAlive && PidFileOwnedBy ( tPaths.m_sPid, iPid, sPidError );
	struct stat tSocket {};
	std::string sSocket = JoinPath ( LOCAL_DATA_DIR, LOCAL_SOCKET );
	bool bSocket = lstat(sSocket.c_str(),&tSocket)==0 && S_ISSOCK(tSocket.st_mode);
	std::string sReadyError;
	bool bReady = bOwned && bSocket && ProbeManticoreReady ( ManticoreClientTarget_e::LOCAL, nullptr, sReadyError );
	printf ( "Target: local (%s/%s)\n", sphGetCwd().cstr(), LOCAL_DATA_DIR );
	printf ( "State: %s\n", bReady ? "ready" : bAlive ? "starting" : "stopped" );
	if ( bPid ) printf ( "PID: %d\n", iPid );
	printf ( "Socket: %s (%s)\n", sSocket.c_str(), bSocket ? "present" : "absent" );
	if ( !bOwned && !sPidError.empty() ) fprintf ( stderr, "manticore: %s\n", sPidError.c_str() );
	return bReady ? 0 : 1;
}
#else
int RunWindowsVersion ( const char * szArgv0 )
{
	std::string sSearchd = szArgv0 ? szArgv0 : "";
	size_t iSlash = sSearchd.find_last_of ( "/\\" );
	if ( iSlash!=std::string::npos )
	{
		sSearchd.resize ( iSlash+1 );
		sSearchd += "searchd.exe";
		if ( _access(sSearchd.c_str(),0)==0 )
		{
			intptr_t iResult = _spawnl ( _P_WAIT, sSearchd.c_str(), sSearchd.c_str(), "--version", nullptr );
			return iResult<0 ? 126 : (int)iResult;
		}
	}
	intptr_t iResult = _spawnlp ( _P_WAIT, "searchd.exe", "searchd.exe", "--version", nullptr );
	if ( iResult<0 )
	{
		fprintf ( stderr, "manticore: matching searchd executable was not found\n" );
		return errno==ENOENT ? 127 : 126;
	}
	return (int)iResult;
}
#endif
}

ParseResult_t ParseArgs ( int iArgc, const char * const * dArgv )
{
	ParseResult_t tResult;
	if ( iArgc<1 || !dArgv )
		return Error ( "invalid argument vector" );

	Options_t & tOptions = tResult.m_tOptions;
	if ( iArgc==1 )
	{
		tResult.m_bOk = true;
		return tResult;
	}

	const char * szFirst = dArgv[1];
	if ( strcmp ( szFirst, "start" )==0 || strcmp ( szFirst, "stop" )==0 || strcmp ( szFirst, "status" )==0 )
	{
		tOptions.m_eCommand = strcmp ( szFirst, "start" )==0 ? Command_e::START : strcmp ( szFirst, "stop" )==0 ? Command_e::STOP : Command_e::STATUS;
		if ( iArgc>3 )
			return Error ( "lifecycle commands accept at most one target: local or global" );
		if ( iArgc==3 )
		{
			if ( strcmp ( dArgv[2], "local" )==0 )
				tOptions.m_eTarget = Target_e::LOCAL;
			else if ( strcmp ( dArgv[2], "global" )==0 )
				tOptions.m_eTarget = Target_e::GLOBAL;
			else
				return Error ( "lifecycle target must be 'local' or 'global'" );
		}
		tResult.m_bOk = true;
		return tResult;
	}

	if ( strcmp ( szFirst, "-h" )==0 || strcmp ( szFirst, "--help" )==0 )
	{
		if ( iArgc!=2 ) return Error ( "--help cannot be combined with other arguments" );
		tOptions.m_eCommand = Command_e::HELP;
		tResult.m_bOk = true;
		return tResult;
	}
	if ( strcmp ( szFirst, "-v" )==0 || strcmp ( szFirst, "--version" )==0 )
	{
		if ( iArgc!=2 ) return Error ( "--version cannot be combined with other arguments" );
		tOptions.m_eCommand = Command_e::VERSION;
		tResult.m_bOk = true;
		return tResult;
	}

	bool bTargetSelected = false;
	for ( int i=1; i<iArgc; ++i )
	{
		const char * szArg = dArgv[i];
		if ( strcmp ( szArg, "--local" )==0 || strcmp ( szArg, "--global" )==0 )
		{
			if ( bTargetSelected ) return Error ( "--local, --global, and --config are mutually exclusive" );
			bTargetSelected = true;
			tOptions.m_eTarget = strcmp ( szArg, "--local" )==0 ? Target_e::LOCAL : Target_e::GLOBAL;
		}
		else if ( IsOption ( szArg, "-c", "--config" ) )
		{
			if ( bTargetSelected ) return Error ( "--local, --global, and --config are mutually exclusive" );
			if ( ++i>=iArgc ) return Error ( "--config requires a file" );
			bTargetSelected = true;
			tOptions.m_eTarget = Target_e::GLOBAL;
			tOptions.m_sConfig = dArgv[i];
			if ( tOptions.m_sConfig.empty() ) return Error ( "--config requires a non-empty file" );
		}
		else if ( IsOption ( szArg, "-e", "--execute" ) )
		{
			if ( tOptions.m_bExecute ) return Error ( "--execute may only be specified once" );
			if ( ++i>=iArgc ) return Error ( "--execute requires SQL" );
			tOptions.m_bExecute = true;
			tOptions.m_sQuery = dArgv[i];
			if ( tOptions.m_sQuery.empty() ) return Error ( "--execute requires non-empty SQL" );
		}
		else
			return Error ( "unknown command or option" );
	}

	tResult.m_bOk = true;
	return tResult;
}

Target_e ResolveTarget ( Target_e eRequested, Marker_e eMarker )
{
	if ( eRequested!=Target_e::AUTO )
		return eRequested;
	return eMarker==Marker_e::ABSENT ? Target_e::GLOBAL : Target_e::LOCAL;
}

Marker_e InspectLocalMarker ( const char * szDirectory, std::string & sError )
{
#if _WIN32
	(void)szDirectory;
	sError = "local mode is not supported on Windows";
	return Marker_e::ERROR_;
#else
	std::string sPath = szDirectory && *szDirectory ? szDirectory : ".";
	if ( sPath.back()!='/' ) sPath += '/';
	sPath += LOCAL_DATA_DIR;
	struct stat tStat {};
	if ( lstat ( sPath.c_str(), &tStat )<0 )
	{
		if ( errno==ENOENT ) return Marker_e::ABSENT;
		sError = "cannot inspect local data path '" + sPath + "': " + strerror(errno);
		return Marker_e::ERROR_;
	}
	if ( !S_ISDIR(tStat.st_mode) )
	{
		sError = "local data path '" + sPath + "' is not a directory";
		return Marker_e::INVALID;
	}
	return Marker_e::DIRECTORY;
#endif
}

void ShowHelp()
{
	fputs (
		"Usage:\n"
		"  manticore [CLIENT_OPTIONS]\n"
		"  manticore start [local|global]\n"
		"  manticore stop [local|global]\n"
		"  manticore status [local|global]\n\n"
		"Client options:\n"
		"  -c, --config FILE  ignore ./manticore_data and use FILE\n"
		"  -e, --execute SQL  execute SQL once and exit\n"
		"      --local        require the current-directory local instance\n"
		"      --global       ignore ./manticore_data and use normal config discovery\n"
		"  -h, --help         show this help\n"
		"  -v, --version      show the Manticore Search version\n\n"
		"Without an explicit target, manticore uses ./manticore_data when present\n"
		"and the globally configured instance otherwise. 'manticore stop' always waits.\n",
		stdout );
}

int Run ( const Options_t & tOptions, const char * szArgv0 )
{
	if ( tOptions.m_eCommand==Command_e::HELP )
	{
		ShowHelp();
		return 0;
	}
	if ( tOptions.m_eCommand==Command_e::VERSION )
	{
#if _WIN32
		return RunWindowsVersion ( szArgv0 );
#else
		std::string sSearchd = FindSearchd ( szArgv0 );
		if ( sSearchd.empty() )
		{
			fprintf ( stderr, "manticore: matching searchd executable was not found\n" );
			return 127;
		}
		return RunPassthrough ( { sSearchd, "--version" } );
#endif
	}
#if _WIN32
	(void)szArgv0;
	fprintf ( stderr, "manticore: this command is not supported on Windows yet\n" );
	return 1;
#else
	std::string sSearchd = FindSearchd ( szArgv0 );
	std::string sMarkerError;
	Marker_e eMarker = InspectLocalMarker ( ".", sMarkerError );
	Target_e eTarget = ResolveTarget ( tOptions.m_eTarget, eMarker );
	if ( tOptions.m_eCommand==Command_e::CLIENT )
	{
		sphInitCJson();
		const char * szQuery = tOptions.m_bExecute ? tOptions.m_sQuery.c_str() : nullptr;
		const char * szConfig = tOptions.m_sConfig.empty() ? nullptr : tOptions.m_sConfig.c_str();
		return ExecuteManticoreSql ( szQuery, ClientTarget(eTarget), szConfig );
	}

	if ( eTarget==Target_e::LOCAL && !ValidateLocalMarker ( eMarker, tOptions.m_eCommand, tOptions.m_eTarget, sMarkerError ) )
		return 1;

	if ( tOptions.m_eCommand==Command_e::STATUS && eTarget==Target_e::LOCAL )
		return RunLocalStatus();

	if ( sSearchd.empty() )
	{
		fprintf ( stderr, "manticore: matching searchd executable was not found\n" );
		return 127;
	}

	if ( tOptions.m_eCommand==Command_e::STATUS )
		return RunPassthrough ( { sSearchd, "--status" } );

	bool bLocal = eTarget==Target_e::LOCAL;
	InstancePaths_t tPaths = bLocal ? LocalPaths() : GlobalPaths();
	std::string sSystemctl;
	bool bSystemd = !bLocal && SystemdAvailable ( sSystemctl );
	const bool bStart = tOptions.m_eCommand==Command_e::START;
	const std::string sBackend = bSystemd ? SERVICE_NAME : sSearchd;
	const std::string sTargetDescription = bLocal ? JoinPath(sphGetCwd().cstr(),LOCAL_DATA_DIR) : "configured/global instance";
	std::string sAction = bStart ? "Starting " : "Stopping ";
	sAction += bLocal ? "local Manticore Search" : bSystemd ? "Manticore Search (systemd)" : "Manticore Search";
	InterruptGuard_c tInterruptGuard;
	Progress_c tProgress ( sAction, tPaths.m_sLog );
	if ( bSystemd ) tProgress.FollowSystemdJournal();

	int iOwnedPid = 0;
	if ( bStart && !bSystemd && !tPaths.m_sPid.empty() )
	{
		std::string sPidError;
		bool bHasPid = ReadPid ( tPaths.m_sPid, iOwnedPid, sPidError );
		if ( bHasPid )
		{
			if ( !ProcessAlive(iOwnedPid) || !PidFileOwnedBy(tPaths.m_sPid,iOwnedPid,sPidError) )
			{
				tProgress.Failure();
				fprintf ( stderr, "manticore: stale or unowned PID metadata prevents startup: %s\n", sPidError.empty() ? "recorded process is not running" : sPidError.c_str() );
				return 1;
			}
			std::string sReadyError;
			if ( !WaitReadyWithProgress ( bLocal ? ManticoreClientTarget_e::LOCAL : ManticoreClientTarget_e::GLOBAL, iOwnedPid, {}, tProgress, sReadyError ) )
			{
				tProgress.Failure();
				fprintf ( stderr, "manticore: existing Manticore Search process did not become ready: %s\n", sReadyError.c_str() );
				return g_bInterrupted ? 130 : 1;
			}
			tProgress.Success ( "Manticore Search is already ready at " + sTargetDescription + " via " + sBackend );
			return 0;
		}
		if ( !sPidError.empty() )
		{
			tProgress.Failure();
			fprintf ( stderr, "manticore: %s\n", sPidError.c_str() );
			return 1;
		}
		if ( bLocal && eMarker==Marker_e::DIRECTORY )
		{
			struct stat tSocketStat {};
			std::string sSocketPath = JoinPath ( LOCAL_DATA_DIR, LOCAL_SOCKET );
			bool bSocketExists = lstat(sSocketPath.c_str(),&tSocketStat)==0;
			if ( bSocketExists )
			{
				tProgress.Failure();
				fprintf ( stderr, "manticore: existing local data directory contains stale socket metadata without an owned PID file\n" );
				return 1;
			}
		}
	}

	int iStopPid = 0;
	if ( !bStart && !bSystemd && !tPaths.m_sPid.empty() )
	{
		std::string sPidError;
		if ( !ReadPid(tPaths.m_sPid,iStopPid,sPidError) )
		{
			if ( sPidError.empty() )
			{
				if ( bLocal )
				{
					struct stat tSocketStat {};
					std::string sSocketPath = JoinPath ( LOCAL_DATA_DIR, LOCAL_SOCKET );
					if ( lstat(sSocketPath.c_str(),&tSocketStat)==0 )
						sPidError = "stale local socket exists without an owned PID file";
					else if ( errno!=ENOENT )
						sPidError = "cannot inspect local socket '" + sSocketPath + "': " + strerror(errno);
				}
				std::string sReadyError;
				if ( sPidError.empty() && !ProbeManticoreReady ( bLocal ? ManticoreClientTarget_e::LOCAL : ManticoreClientTarget_e::GLOBAL, nullptr, sReadyError ) )
				{
					tProgress.Success ( bLocal ? "Local Manticore Search is already stopped" : "Manticore Search is already stopped" );
					return 0;
				}
				if ( sPidError.empty() )
					sPidError = "SQL endpoint is ready but the configured PID file is missing";
			}
			tProgress.Failure();
			fprintf ( stderr, "manticore: %s\n", sPidError.c_str() );
			return 1;
		}
		if ( !ProcessAlive(iStopPid) )
		{
			tProgress.Failure();
			fprintf ( stderr, "manticore: stale PID file refers to a stopped process\n" );
			return 1;
		}
		if ( !PidFileOwnedBy(tPaths.m_sPid,iStopPid,sPidError) )
		{
			tProgress.Failure();
			fprintf ( stderr, "manticore: refusing to stop an unowned process: %s\n", sPidError.c_str() );
			return 1;
		}
	}

	std::vector<std::string> dArgs;
	const char * szPrefix = "[searchd] ";
	if ( bSystemd )
	{
		dArgs = { sSystemctl, bStart ? "start" : "stop", SERVICE_NAME };
		szPrefix = "[systemd] ";
	}
	else
	{
		dArgs = { sSearchd };
		if ( bLocal ) dArgs.push_back ( "--local" );
		if ( !bStart ) dArgs.push_back ( "--stopwait" );
	}

	int iResult = RunProgressProcess ( dArgs, tProgress, szPrefix );
	if ( iResult )
	{
		tProgress.Failure();
		if ( iResult==130 )
		{
			fprintf ( stderr, "manticore: waiting was interrupted; Manticore Search may still be changing state\n" );
			return 130;
		}
		fprintf ( stderr, "manticore: backend '%s' failed for %s with exit code %d\n", sBackend.c_str(), sTargetDescription.c_str(), iResult );
		if ( bSystemd ) fprintf ( stderr, "Direct searchd fallback was not attempted because the packaged service is authoritative.\n" );
		return iResult;
	}

	if ( bSystemd )
	{
		bool bExpected = bStart
			? RunCapture({sSystemctl,"is-active","--quiet",SERVICE_NAME}).m_iExit==0
			: SystemdInactive(sSystemctl);
		if ( !bExpected )
		{
			tProgress.Failure();
			fprintf ( stderr, "manticore: systemd did not reach the expected %s state\n%s\n", bStart ? "active" : "inactive", SystemdStateDiagnostic(sSystemctl).c_str() );
			return 1;
		}
	}

	if ( bStart )
	{
		int iPid = -1;
		std::string sPidError;
		if ( !bSystemd )
		{
			if ( tPaths.m_sPid.empty() || !ReadPid(tPaths.m_sPid,iPid,sPidError) || !ProcessAlive(iPid) || !PidFileOwnedBy(tPaths.m_sPid,iPid,sPidError) )
			{
				tProgress.Failure();
				fprintf ( stderr, "manticore: daemon did not establish owned, live PID metadata after startup%s%s\n", sPidError.empty() ? "" : ": ", sPidError.c_str() );
				return 1;
			}
		}

		std::string sReadyError;
		if ( !WaitReadyWithProgress ( bLocal ? ManticoreClientTarget_e::LOCAL : ManticoreClientTarget_e::GLOBAL, iPid, bSystemd ? sSystemctl : std::string(), tProgress, sReadyError ) )
		{
			tProgress.Failure();
			fprintf ( stderr, "manticore: Manticore Search did not become ready: %s\n", sReadyError.c_str() );
			return g_bInterrupted ? 130 : 1;
		}
		tProgress.Success ( "Manticore Search is ready at " + sTargetDescription + " via " + sBackend );
	}
	else
	{
		if ( !bSystemd )
		{
			struct stat tStat {};
			bool bPidGone = tPaths.m_sPid.empty() || ( lstat(tPaths.m_sPid.c_str(),&tStat)<0 && errno==ENOENT );
			bool bSocketGone = true;
			if ( bLocal )
			{
				std::string sSocketPath = JoinPath ( LOCAL_DATA_DIR, LOCAL_SOCKET );
				bSocketGone = lstat(sSocketPath.c_str(),&tStat)<0 && errno==ENOENT;
			}
			std::string sReadyError;
			bool bReady = ProbeManticoreReady ( bLocal ? ManticoreClientTarget_e::LOCAL : ManticoreClientTarget_e::GLOBAL, nullptr, sReadyError );
			if ( ProcessAlive(iStopPid) || !bPidGone || !bSocketGone || bReady )
			{
				tProgress.Failure();
				fprintf ( stderr, "manticore: stop completed but the selected process, metadata, or SQL endpoint remains active\n" );
				return 1;
			}
		}
		tProgress.Success ( std::string(bLocal ? "Local Manticore Search stopped at " : "Manticore Search stopped at ") + sTargetDescription + " via " + sBackend );
	}
	return 0;
#endif
}
}
