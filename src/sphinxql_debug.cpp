//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//


#include "sphinxql_debug.h"

struct BlobLocator_t
{
	int m_iStart;
	int m_iLen;
};

using namespace DebugCmd;

class SqlDebugParser_c : public SqlParserTraits_c
{
	DebugCommand_t& m_tCmd;

public:
	SqlDebugParser_c ( DebugCommand_t& tCmd, Str_t sQuery, CSphVector<SqlStmt_t>& dStmt, CSphString& sError )
		: SqlParserTraits_c ( dStmt, sQuery.first, &sError )
		, m_tCmd ( tCmd )
	{
		m_sErrorHeader = "sphinxql-debug:";
	}

	CSphString StrFromBlob ( BlobLocator_t tStr ) const
	{
		CSphString sResult;
		sResult.SetBinary(m_pBuf+tStr.m_iStart, tStr.m_iLen);
		return sResult;
	}

	ParsedOption_t& OptByName ( BlobLocator_t tStr )
	{
		auto sOption = StrFromBlob ( tStr );
		sOption.ToLower ();
		return m_tCmd.m_hOptions.AddUnique ( sOption );
	}

	void AddBoolOption ( BlobLocator_t tStr, bool bValue = true )
	{
		auto& tOption = OptByName ( tStr );
		if ( bValue )
		{
			tOption.m_bValue = true;
			tOption.m_iValue = 1;
			tOption.m_fValue = 1.0f;
			tOption.m_sValue = "true";
		} else {
			tOption.m_bValue = false;
			tOption.m_iValue = 0;
			tOption.m_fValue = 0.0f;
			tOption.m_sValue = "false";
		}
	}

	void AddStrOption ( BlobLocator_t tStr, BlobLocator_t tValue )
	{
		auto & tOption = OptByName ( tStr );
		tOption.m_sValue = StrFromBlob ( tValue );
		tOption.m_bValue = !tOption.m_sValue.IsEmpty ();
	}

	void AddStrOption ( const char* szStr, BlobLocator_t tValue )
	{
		auto& tOption = m_tCmd.m_hOptions.AddUnique ( szStr );
		tOption.m_sValue = StrFromBlob ( tValue );
		tOption.m_bValue = !tOption.m_sValue.IsEmpty();
	}

	void AddIntOption ( BlobLocator_t tStr, int64_t iValue )
	{
		auto & tOption = OptByName ( tStr );
		tOption.m_iValue = iValue;
		tOption.m_fValue = float(iValue);
		tOption.m_bValue = iValue!=0;
	}

	void AddFloatOption ( BlobLocator_t tStr, float fValue )
	{
		auto & tOption = OptByName ( tStr );
		tOption.m_fValue = fValue;
		tOption.m_iValue = int ( fValue );
		tOption.m_bValue = tOption.m_iValue!=0;
	}

	void SetWrongDebugCmd ()
	{
		SetCommand ( Cmd_e::PARSE_SYNTAX_ERROR );
	}

	void SetCommand ( Cmd_e eCmd )
	{
		m_tCmd.m_eCommand = eCmd;
	}

	bool CommandIs ( Cmd_e eCmd ) const noexcept
	{
		return m_tCmd.m_eCommand == eCmd;
	}

	void SetPar1 ( int64_t iPar )
	{
		m_tCmd.m_iPar1 = iPar;
	}

	void SetPar2 ( int64_t iPar )
	{
		m_tCmd.m_iPar2 = iPar;
	}

	void SetSParam ( BlobLocator_t tStr )
	{
		m_tCmd.m_sParam = StrFromBlob ( tStr );
	}

	void SetSParam2 ( BlobLocator_t tStr )
	{
		m_tCmd.m_sParam2 = StrFromBlob ( tStr );
	}
};

bool DebugCommand_t::bOpt ( const char * szName, bool bDefault ) const
{
	if ( !m_hOptions.Exists ( szName ) )
		return bDefault;
	return m_hOptions[szName].m_bValue;
}

int64_t DebugCommand_t::iOpt ( const char * szName, int64_t iDefault ) const
{
	if ( !m_hOptions.Exists ( szName ) )
		return iDefault;
	return m_hOptions[szName].m_iValue;
}

float DebugCommand_t::fOpt ( const char * szName, float fDefault ) const
{
	if ( !m_hOptions.Exists ( szName ) )
		return fDefault;
	return m_hOptions[szName].m_fValue;
}

CSphString DebugCommand_t::sOpt ( const char * szName, const char * szDefault ) const
{
	CSphString sResult;
	if ( !m_hOptions.Exists ( szName ) )
		sResult = szDefault;
	else
		sResult = m_hOptions[szName].m_sValue;
	return sResult;
}

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_DECL inline int flex_debugparser ( YYSTYPE * lvalp, void * yyscanner, SqlDebugParser_c * pParser )

#include "flexsphinxqldebug.c"

static void yyerror ( SqlDebugParser_c* pParser, const char* szMessage )
{
	// flex put a zero at last token boundary; make it undo that
	yy4lex_unhold ( pParser->m_pScanner );

	pParser->ProcessParsingError ( szMessage );
	if ( pParser->IsWrongSyntaxError() )
		pParser->SetWrongDebugCmd();
}

#ifndef NDEBUG
// using a proxy to be possible to debug inside yylex
inline int yylex ( YYSTYPE * lvalp, SqlDebugParser_c * pParser )
{
	int res = flex_debugparser ( lvalp, pParser->m_pScanner, pParser );
	return res;
}
#else
inline int yylex ( YYSTYPE * lvalp, SqlDebugParser_c * pParser )
{
	return flex_debugparser ( lvalp, pParser->m_pScanner, pParser );
}
#endif

#include "bissphinxql_debug.c"

static std::unique_ptr<DebugCmd::DebugCommand_t> ParseDebugCmdImpl ( Str_t sQuery, CSphVector<SqlStmt_t>& dStmt, CSphString& sError )
{
	auto pResult = std::make_unique<DebugCmd::DebugCommand_t>();
	auto& tResult = *pResult;
	if ( !IsFilled ( sQuery ) )
	{
		tResult.m_sParam = "query was empty";
		return pResult;
	}

	SqlDebugParser_c tParser ( tResult, sQuery, dStmt, sError );
	tParser.m_pBuf = sQuery.first;
	tResult.m_szStmt = sQuery.first;

	char * sEnd = const_cast<char *>( sQuery.first+sQuery.second );
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	yy4lex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy4_scan_buffer ( const_cast<char *>( sQuery.first ), sQuery.second+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		tResult.m_sParam = "internal error: yy4_scan_buffer() failed";
		return pResult;
	}

	yyparse ( &tParser );

	yy4_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yy4lex_destroy ( tParser.m_pScanner );

	return pResult;
}

ParseResult_e ParseDebugCmd ( Str_t sQuery, CSphVector<SqlStmt_t>& dStmt, CSphString& sError )
{
	// parse debug statements
	auto pCmd = ParseDebugCmdImpl ( sQuery, dStmt, sError );
	if ( !pCmd->Valid() )
		return ParseResult_e::PARSE_SYNTAX_ERROR;

	auto& tStmt = dStmt.Add();
	tStmt.m_pDebugCmd = std::move ( pCmd );
	tStmt.m_eStmt = STMT_DEBUG;
	return ParseResult_e::PARSE_OK;
}


CmdNotice_t DebugCmd::dCommands[(BYTE) Cmd_e::INVALID_CMD] = {
	{ NEED_VIP, "debug shutdown <password>", "emulate TERM signal" },
	{ NEED_VIP, "debug crash <password>", "crash daemon (make SIGSEGV action)" },
	{ NONE, "debug token <password>", "calculate token for password" },
	{ MALLOC_STATS, "debug malloc_stats", "perform 'malloc_stats', result in searchd.log" },
	{ MALLOC_TRIM, "debug malloc_trim", "pefrorm 'malloc_trim' call" },
	{ NEED_VIP | NO_WIN, "debug procdump", "ask watchdog to dump us" },
	{ NEED_VIP | NO_WIN, "debug setgdb on|off", "enable or disable potentially dangerous crash dumping with gdb" },
	{ NEED_VIP | NO_WIN, "debug setgdb status", "show current mode of gdb dumping" },
	{ NONE, "debug sleep <N>", "sleep for <N> seconds" },
	{ NONE, "debug tasks", "display global tasks stat (use select from @@system.tasks instead)" },
	{ NONE, "debug sched", "display task manager schedule (use select from @@system.sched instead)" },
	{ NONE, "debug merge <TBL> [chunk] <X> [into] [chunk] <Y> [option sync=1,byid=0]",
			"For RT table <TBL> merge disk chunk X into disk chunk Y" },
	{ NONE, "debug drop [chunk] <X> [from] <TBL> [option sync=1]",
			"For RT table <TBL> drop disk chunk X" },
	{ NONE, "debug files <TBL> [option format=all|external]",
			"list files belonging to <TBL>. 'all' - including external (wordforms, stopwords, etc.)" },
	{ NONE, "debug close", "ask server to close connection from it's side" },
	{ NONE, "debug compress <TBL> [chunk] <X> [option sync=1]",
			"Compress disk chunk X of RT table <TBL> (wipe out deleted documents)" },
	{ NONE, "debug split <TBL> [chunk] <X> on @<uservar> [option sync=1]",
			"Split disk chunk X of RT table <TBL> using set of DocIDs from @uservar" },
	{ NO_WIN, "debug wait <cluster> [like 'xx'] [option timeout=3]", "wait <cluster> ready, but no more than 3 secs." },
	{ NO_WIN, "debug wait <cluster> status <N> [like 'xx'] [option timeout=13]", "wait <cluster> commit achieve <N>, but no more than 13 secs" },
	{ NONE, "debug meta", "Show max_matches/pseudo_shards. Needs set profiling=1" },
	{ NONE, "debug trace OFF|'path/to/file' [<N>]", "trace flow to file until N bytes written, or 'trace OFF'" },
	{ NONE, "debug curl <URL>", "request given url via libcurl" },
};
