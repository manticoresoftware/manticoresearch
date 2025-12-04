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

#pragma once

#if _WIN32
#include <std/ints.h>

bool StopWinService () noexcept;

void WinSetConsoleCtrlHandler () noexcept;

void ServiceInstall ( int argc, char ** argv );

void ServiceDelete ();

enum ESphLogLevel : BYTE;

void EventLogEntry ( ESphLogLevel eLevel, char * sBuf, char * sTtyBuf );

bool ParseArgsAndStartWinService ( int argc, char ** argv, void * ServiceFunction );

void SetupWinService ( int& argc, char **& argv );

void SetWinServiceStopped ();

void SetWinServiceRunning ();

void CloseWinPipe ();

void CheckWinSignals ();

int WinStopOrWaitAnother(int iPid, int iWaitTimeout);

#endif

bool WinService () noexcept;
