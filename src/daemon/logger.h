//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "std/string.h"

enum ESphLogLevel : BYTE;

void sphLog ( ESphLogLevel eLevel, const char * sFmt, va_list ap );
int GetDaemonLogBufSize () noexcept;
void SetLogFilter ( const CSphString & szFilter ) noexcept;
int GetActiveLogFD () noexcept;
int GetDaemonLogFD () noexcept;
bool ForceLogStdout () noexcept;
void StopLogStdout () noexcept;
const CSphString & sphGetLogFile () noexcept;
void LogTimeZoneStartup ( const CSphString & sWarning );
void ReopenDaemonLog ();
void RefreshIsAtty () noexcept;
void ChangeLogFileMode ( int iFile ) noexcept;
void SetDaemonLog ( CSphString && sLog, bool bCloseIfOpened = false );
void SetLogFileMode ( int iMode ) noexcept;
void DisableLogSyslog () noexcept;
bool LogSyslogEnabled () noexcept;
