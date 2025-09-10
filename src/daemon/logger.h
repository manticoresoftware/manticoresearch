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
#include "std/vectraits.h"
#include "sphinx.h"

enum ESphLogLevel : BYTE;
void OpenSyslogIfNecessary ();

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
bool LogIsStdout () noexcept;
bool LogIsSyslog () noexcept;

// HTTP log
void SetupHttpLog ( CSphString sHttpLog );
void ReopenHttpLog ();
bool HttpLogEnabled () noexcept;
void logHttpOutput ( VecTraits_T<BYTE> dData, int iReqID );
void logHttpInput ( VecTraits_T<BYTE> sData, int iReqID, int64_t iTimestamp );

// query log
enum class LOG_FORMAT : BYTE
{
	_PLAIN,
	SPHINXQL
};

enum SqlStmt_e : BYTE; // defined in searchdsql.h

void SetLogFormat ( LOG_FORMAT eFormat ) noexcept;
LOG_FORMAT LogFormat () noexcept;
void SetLogCompact ( bool bCompact ) noexcept;
void SetupQueryLogDrain ( CSphString sQueryLog ); // filename for query-log, or "syslog"
void ReopenQueryLog ();
void RedirectQueryLogToDaemonLog ();
void SetLogStatsFlags ( bool bIOStats, bool bCPUStats ) noexcept; // switch iostats/cpustats
void ConfigureQueryLogCommands ( const CSphString & sMode );

using QuotationEscapedBuilder = EscapedStringBuilder_T<BaseQuotation_T<EscapeQuotator_t>>;
void FormatSphinxql ( const CSphQuery & q, const CSphQuery & tJoinOptions, int iCompactIN, QuotationEscapedBuilder & tBuf );
void LogQuery ( const CSphQuery & q, const CSphQuery & tJoinOptions, const CSphQueryResultMeta & tMeta, const CSphVector<int64_t> & dAgentTimes );
void LogSphinxqlClause ( Str_t sQuery, int iRealTime );
void LogQueryToSphinxlLog (const CSphString& sQuery, const CSphString& sError );

class LogStmtGuard_c
{
	int64_t m_tmStarted;
	bool m_bLogDone;

public:
	NONCOPYMOVABLE( LogStmtGuard_c );
	LogStmtGuard_c ( const Str_t & sQuery, SqlStmt_e eStmt, bool bMulti );
	~LogStmtGuard_c ();
};

// used by Buddy
enum class BuddyQuery_e : BYTE { SQL, HTTP };
void LogSphinxqlError ( const char * sStmt, const Str_t & sError );
void LogSphinxqlError ( const Str_t & sStmt, const Str_t & sError );
void LogBuddyQuery ( Str_t sQuery, BuddyQuery_e tType );