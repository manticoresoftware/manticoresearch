//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "logger.h"
#include "winservice.h"
#include "sphinxdefs.h"
#include "std/generics.h"
#include "datetime.h"
#include "searchdaemon.h"

// for USE_SYSLOG
#include "config.h"
#if USE_SYSLOG
#include <syslog.h>
#endif
#include <cstring>
#include <sys/stat.h>

constexpr int LOG_BUF_SIZE = 1024;
static char g_sLogFilter[SPH_MAX_FILENAME_LEN + 1] = "\0";
static int g_iLogFilterLen = 0;
static int g_iLogFile = STDOUT_FILENO; // log file descriptor
static bool g_bLogStdout = true; // extra copy of startup log messages to stdout; true until around "accepting connections", then MUST be false
static bool g_bLogSyslog = false;
static bool g_bLogTty = false; // cached isatty(g_iLogFile)
static CSphString g_sLogFile; // log file name
static int g_iLogFileMode = 0;


void LogTimeZoneStartup ( const CSphString & sWarning )
{
	// avoid writing this to stdout
	bool bLogStdout = g_bLogStdout;
	g_bLogStdout = false;
	if ( !sWarning.IsEmpty() )
		sphWarning ( "Error initializing time zones: %s", sWarning.cstr() );

	sphInfo ( "Using local time zone '%s'", GetLocalTimeZoneName().cstr() );
	g_bLogStdout = bLogStdout;
}


int GetDaemonLogBufSize () noexcept
{
	return LOG_BUF_SIZE;
}


void SetLogFilter ( const CSphString & sFilter ) noexcept
{
	int iLen = sFilter.Length();
	iLen = Min ( iLen, SPH_MAX_FILENAME_LEN );
	memcpy ( g_sLogFilter, sFilter.cstr(), iLen );
	g_sLogFilter[iLen] = '\0';
	g_iLogFilterLen = iLen;
}


int GetActiveLogFD () noexcept
{
	return ForceLogStdout() ? STDOUT_FILENO : g_iLogFile;
}


int GetDaemonLogFD () noexcept
{
	return g_iLogFile;
}


bool ForceLogStdout () noexcept
{
	return g_bLogStdout && g_iLogFile != STDOUT_FILENO;
}


void StopLogStdout () noexcept
{
	g_bLogStdout = false;
}


void SetLogFileMode ( int iMode ) noexcept
{
	g_iLogFileMode = iMode;
}


const CSphString & sphGetLogFile () noexcept
{
	return g_sLogFile;
}


void RefreshIsAtty () noexcept
{
	g_bLogTty = (isatty ( g_iLogFile ) != 0);
}

bool LogIsStdout () noexcept
{
	return ForceLogStdout();
}

bool LogIsSyslog () noexcept
{
	return g_bLogSyslog;
}

/// physically emit log entry
/// buffer must have 1 extra byte for linefeed
static void sphLogEntry ( ESphLogLevel eLevel, char * sBuf, char * sTtyBuf )
{
#if _WIN32
	if ( g_iLogFile==STDOUT_FILENO && WinService() )
		EventLogEntry ( eLevel, sBuf, sTtyBuf );
	else
#endif
	{
		(void) eLevel;
		strcat ( sBuf, "\n" ); // NOLINT

		sphSeek ( g_iLogFile, 0, SEEK_END );
		if ( g_bLogTty )
		{
			memmove ( sBuf + 20, sBuf + 15, 9 );
			sTtyBuf = sBuf + 19;
			*sTtyBuf = '[';
			sphWrite ( g_iLogFile, sTtyBuf, strlen ( sTtyBuf ) );
		} else
			sphWrite ( g_iLogFile, sBuf, strlen ( sBuf ) );

		if ( ForceLogStdout() )
			sphWrite ( STDOUT_FILENO, sTtyBuf, strlen ( sTtyBuf ) );
	}
}


/// log entry (with log levels, dupe catching, etc)
/// call with NULL format for dupe flushing
void sphLog ( ESphLogLevel eLevel, const char * sFmt, va_list ap )
{
	// dupe catcher state
	static const int FLUSH_THRESH_TIME = 1000000; // in microseconds
	static const int FLUSH_THRESH_COUNT = 100;

	static ESphLogLevel eLastLevel = SPH_LOG_INFO;
	static DWORD uLastEntry = 0;
	static int64_t tmLastStamp = -1000000 - FLUSH_THRESH_TIME;
	static int iLastRepeats = 0;

	// only if we can
	if ( sFmt && eLevel > g_eLogLevel )
		return;

#if USE_SYSLOG
	if ( g_bLogSyslog && sFmt )
	{
		const int levels[SPH_LOG_MAX + 1] = { LOG_EMERG, LOG_WARNING, LOG_INFO, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG };
		vsyslog ( levels[eLevel], sFmt, ap );
		return;
	}
#endif

	if ( g_iLogFile < 0 && !WinService() )
		return;

	// format the banner
	char sTimeBuf[128];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	const char * sBanner = "";
	if ( sFmt == NULL ) eLevel = eLastLevel;
	if ( eLevel == SPH_LOG_FATAL ) sBanner = "FATAL: ";
	if ( eLevel == SPH_LOG_WARNING ) sBanner = "WARNING: ";
	if ( eLevel >= SPH_LOG_DEBUG ) sBanner = "DEBUG: ";
	if ( eLevel == SPH_LOG_RPL_DEBUG ) sBanner = "RPL: ";

	char sBuf[LOG_BUF_SIZE];
	snprintf ( sBuf, sizeof(sBuf) - 1, "[%s] [%d] ", sTimeBuf, GetOsThreadId() );

	char * sTtyBuf = sBuf + strlen ( sBuf );
	strncpy ( sTtyBuf, sBanner, 32 ); // 32 is arbitrary; just something that is enough and keeps lint happy

	auto iLen = (int) strlen ( sBuf );

	// format the message
	if ( sFmt )
	{
		// need more space for tail zero and "\n" that added at sphLogEntry
		int iSafeGap = 4;
		int iBufSize = sizeof(sBuf) - iLen - iSafeGap;
		vsnprintf ( sBuf + iLen, iBufSize, sFmt, ap );
		sBuf[sizeof(sBuf) - iSafeGap] = '\0';
	}

	if ( sFmt && eLevel > SPH_LOG_INFO && g_iLogFilterLen )
	{
		if ( strncmp ( sBuf + iLen, g_sLogFilter, g_iLogFilterLen ) != 0 )
			return;
	}

	// catch dupes
	DWORD uEntry = sFmt ? sphCRC32 ( sBuf + iLen ) : 0;
	int64_t tmNow = sphMicroTimer();

	// accumulate while possible
	if ( sFmt && eLevel == eLastLevel && uEntry == uLastEntry && iLastRepeats < FLUSH_THRESH_COUNT && tmNow < tmLastStamp + FLUSH_THRESH_TIME )
	{
		tmLastStamp = tmNow;
		iLastRepeats++;
		return;
	}

	// flush if needed
	if ( iLastRepeats != 0 && (sFmt || tmNow >= tmLastStamp + FLUSH_THRESH_TIME) )
	{
		// flush if we actually have something to flush, and
		// case 1: got a message we can't accumulate
		// case 2: got a periodic flush and been otherwise idle for a thresh period
		char sLast[256];
		iLen = Min ( iLen, 256 );
		strncpy ( sLast, sBuf, iLen );
		if ( iLen < 256 )
			snprintf ( sLast + iLen, sizeof(sLast) - iLen, "last message repeated %d times", iLastRepeats );
		sphLogEntry ( eLastLevel, sLast, sLast + (sTtyBuf - sBuf) );

		tmLastStamp = tmNow;
		iLastRepeats = 0;
		eLastLevel = SPH_LOG_INFO;
		uLastEntry = 0;
	}

	// was that a flush-only call?
	if ( !sFmt )
		return;

	tmLastStamp = tmNow;
	iLastRepeats = 0;
	eLastLevel = eLevel;
	uLastEntry = uEntry;


	/////////////////////////////////////////////////////////////////////////////
// LOGGING
/////////////////////////////////////////////////////////////////////////////
// do the logging
	sphLogEntry ( eLevel, sBuf, sTtyBuf );
}


void ReopenDaemonLog ()
{
	if ( g_iLogFile < 0 || g_bLogTty )
		return;

	int iFD = ::open ( g_sLogFile.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
	if ( iFD < 0 )
	{
		sphWarning ( "failed to reopen log file '%s': %s", g_sLogFile.cstr(), strerrorm(errno) );
	} else
	{
		::close ( g_iLogFile );
		g_iLogFile = iFD;
		RefreshIsAtty();
		ChangeLogFileMode ( g_iLogFile );
		sphInfo ( "log reopened" );
	}
}


void ChangeLogFileMode ( int iFile ) noexcept
{
	if ( iFile < 0 || g_iLogFileMode == 0 || iFile == STDOUT_FILENO || iFile == STDERR_FILENO )
		return;

#if !_WIN32
	fchmod ( iFile, g_iLogFileMode );
#endif
}


void SetDaemonLog ( CSphString && sLog, bool bCloseIfOpened )
{
	if ( sLog == "syslog" )
	{
#if !USE_SYSLOG
		if ( g_iLogFile<0 )
		{
			g_iLogFile = STDOUT_FILENO;
			sphWarning ( "failed to use syslog for logging. You have to reconfigure --with-syslog and rebuild the daemon!" );
			sphInfo ( "will use default file 'searchd.log' for logging." );
		}
#else
		g_bLogSyslog = true;
#endif
	}

	umask ( 066 );
	if ( bCloseIfOpened && g_iLogFile != STDOUT_FILENO )
	{
		close ( g_iLogFile );
		g_iLogFile = STDOUT_FILENO;
	}
	if ( !g_bLogSyslog )
	{
		FixPathAbsolute ( sLog );
		g_iLogFile = open ( sLog.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
		if ( g_iLogFile < 0 )
		{
			g_iLogFile = STDOUT_FILENO;
			sphFatal ( "failed to open log file '%s': %s", sLog.cstr(), strerrorm ( errno ) );
		}
		ChangeLogFileMode ( g_iLogFile );
	}

	g_sLogFile = sLog;
	RefreshIsAtty();
}

void DisableLogSyslog () noexcept
{
	g_bLogSyslog = false;
}

bool LogSyslogEnabled () noexcept
{
		return g_bLogSyslog;
}

