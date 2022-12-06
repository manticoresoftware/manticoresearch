//
// Copyright (c) 2021-2022, Manticore Software LTD (https://manticoresearch.com)
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

#include "sphinxstd.h"

namespace DebugCmd {

struct Option_t
{
	int64_t m_iValue = 0;
	float m_fValue = 0.0;
	bool m_bValue = false;
	CSphString m_sValue;
};

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
	SPLIT,
	WAIT,
	WAIT_STATUS,
	META,
	TRACE,
	CURL,

	INVALID_CMD
};

struct DebugCommand_t
{
	Cmd_e	m_eCommand {Cmd_e::INVALID_CMD};
	CSphString m_sParam;
	CSphString m_sParam2;
	int64_t m_iPar1 = -1;
	int64_t m_iPar2;
	const char * m_szStmt;
	SmallStringHash_T<Option_t> m_hOptions;

	bool bOpt ( const char * szName, bool bDefault = false ) const;
	int64_t iOpt ( const char * szName, int64_t iDefault = 0 ) const;
	float fOpt ( const char * szName, float fDefault = 0.0 ) const;
	CSphString sOpt ( const char * szName, const char* szDefault = nullptr ) const;
};

enum Traits_e : BYTE
{
	NONE = 0,
	NEED_VIP = 1,
	NO_WIN = 2,
	MALLOC_STATS = 4,
	MALLOC_TRIM = 8,
};

struct CmdNotice_t
{
	BYTE m_uTraits;
	const char * m_szExample;
	const char * m_szExplanation;
};

extern CmdNotice_t dCommands[(BYTE) Cmd_e::INVALID_CMD];

DebugCommand_t ParseDebugCmd ( Str_t sQuery, CSphString & sError );

} // namespace DebugCmd;

