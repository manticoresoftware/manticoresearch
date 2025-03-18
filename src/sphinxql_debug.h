//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "std/ints.h"
#include "std/string.h"
#include "searchdsql.h"

namespace DebugCmd {

enum class Cmd_e : BYTE
{
	SHUTDOWN = 0,
	CRASH,
	TOKEN,
	MALLOC_STATS,
	MALLOC_TRIM,
	PROCDUMP,
	SETGDB,
	GDBSTATUS,
	SLEEP,
	TASKS,
	SCHED,
	MERGE,
	DROP,
	FILES,
	CLOSE,
	COMPRESS,
	DEDUP,
	SPLIT,
	WAIT,
	WAIT_STATUS,
	META,
	TRACE,
	CURL,
	PAUSE,

	INVALID_CMD,
	PARSE_SYNTAX_ERROR,
};

struct DebugCommand_t
{
	Cmd_e	m_eCommand {Cmd_e::PARSE_SYNTAX_ERROR};
	CSphString m_sParam;
	CSphString m_sParam2;
	int64_t m_iPar1 = -1;
	int64_t m_iPar2;
	const char * m_szStmt;
	SmallStringHash_T<ParsedOption_t> m_hOptions;

	bool bOpt ( const char * szName, bool bDefault = false ) const;
	int64_t iOpt ( const char * szName, int64_t iDefault = 0 ) const;
	float fOpt ( const char * szName, float fDefault = 0.0 ) const;
	CSphString sOpt ( const char * szName, const char* szDefault = nullptr ) const;
	[[nodiscard]] inline bool Valid() const noexcept { return m_eCommand != Cmd_e::PARSE_SYNTAX_ERROR; };
};

} // namespace DebugCmd;

ParseResult_e ParseDebugCmd ( Str_t sQuery, CSphVector<SqlStmt_t>& dStmt, CSphString& sError );

