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


#include "sphinxql_debug.h"

struct BlobLocator_t
{
	int m_iStart;
	int m_iLen;
};

using namespace DebugCmd;

struct SqlDebugParser_c
{
	explicit SqlDebugParser_c ( DebugCommand_t& tCmd )
		: m_tCmd ( tCmd ) {}

	DebugCommand_t& m_tCmd;
	void * m_pScanner = nullptr;
	const char * m_pBuf = nullptr;
	CSphString StrFromBlob ( BlobLocator_t tStr ) const
	{
		CSphString sResult;
		sResult.SetBinary(m_pBuf+tStr.m_iStart, tStr.m_iLen);
		return sResult;
	}

	Option_t& OptByName ( BlobLocator_t tStr )
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

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wpragmas"
#endif

#include "flexsphinxqldebug.c"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif


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

static void yyerror ( SqlDebugParser_c * /*pParser*/, const char * /*sMessage*/ )
{
	// flex put a zero at last token boundary; make it undo that
}


#include "bissphinxql_debug.c"

DebugCmd::DebugCommand_t DebugCmd::ParseDebugCmd ( Str_t sQuery, CSphString & sError )
{
	DebugCmd::DebugCommand_t tResult;
	if ( !IsFilled ( sQuery ) )
	{
		sError = "query was empty";
		return tResult;
	}

	SqlDebugParser_c tParser (tResult);
	tParser.m_pBuf = sQuery.first;
	tParser.m_tCmd.m_szStmt = sQuery.first;

	char * sEnd = const_cast<char *>( sQuery.first+sQuery.second );
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	yy4lex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy4_scan_buffer ( const_cast<char *>( sQuery.first ), sQuery.second+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: yy4_scan_buffer() failed";
		return tResult;
	}

	yyparse ( &tParser );

	yy4_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yy4lex_destroy ( tParser.m_pScanner );

	return tResult;
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
	{ NONE, "debug merge <IDX> [chunk] <X> [into] [chunk] <Y> [option sync=1,byid=0]",
			"For RT index <IDX> merge disk chunk X into disk chunk Y" },
	{ NONE, "debug drop [chunk] <X> [from] <IDX> [option sync=1]",
			"For RT index <IDX> drop disk chunk X" },
	{ NONE, "debug files <IDX> [option format=all|external]",
			"list files belonging to <IDX>. 'all' - including external (wordforms, stopwords, etc.)" },
	{ NONE, "debug close", "ask server to close connection from it's side" },
	{ NONE, "debug compress <IDX> [chunk] <X> [option sync=1]",
			"Compress disk chunk X of RT index <IDX> (wipe out deleted documents)" },
	{ NONE, "debug split <IDX> [chunk] <X> on @<uservar> [option sync=1]",
			"Split disk chunk X of RT index <IDX> using set of DocIDs from @uservar" },
	{ NO_WIN, "debug wait <cluster> [like 'xx'] [option timeout=3]", "wait <cluster> ready, but no more than 3 secs." },
	{ NO_WIN, "debug wait <cluster> status <N> [like 'xx'] [option timeout=13]", "wait <cluster> commit achieve <N>, but no more than 13 secs" },
	{ NONE, "debug meta", "Show max_matches/pseudo_shards. Needs set profiling=1" },
	{ NONE, "debug trace OFF|'path/to/file' [<N>]", "trace flow to file until N bytes written, or 'trace OFF'" },
};
