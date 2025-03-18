//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//


#include "sphinxql_extra.h"
#include "sphinxql_debug.h"

struct BlobLocator_t
{
	int m_iStart;
	int m_iLen;
};


class SqlExtraParser_c : public SqlParserTraits_c
{
public:
	SqlExtraParser_c ( CSphVector<SqlStmt_t>& dStmt, const char* szQuery, CSphString* pError)
		: SqlParserTraits_c ( dStmt, szQuery, pError )
	{
		if ( m_dStmt.IsEmpty() )
			PushQuery();
		else
			m_pStmt = &m_dStmt.Last();
		m_sErrorHeader = "P05:";
	}

	CSphString StrFromBlob ( BlobLocator_t tStr ) const
	{
		CSphString sResult;
		sResult.SetBinary(m_pBuf+tStr.m_iStart, tStr.m_iLen);
		return sResult;
	}

	void SetStatement ()
	{
		m_pStmt->m_eStmt = STMT_SET;
		m_pStmt->m_eSet = SET_EXTRA;
	}

	inline void SetGlobalScope()
	{
		m_pStmt->m_iIntParam = 0;
	}

	inline void SetSessionScope()
	{
		m_pStmt->m_iIntParam = 1;
	}

	void AddIntval ( CSphVector<SqlInsert_t>& dVec, const SqlNode_t& tNode )
	{
		SqlInsert_t& tIns = dVec.Add();
		tIns.CopyValueInt ( tNode );
		tIns.m_iType = m_pStmt->m_iIntParam; // 0 or 1 - global or session int
	}

	void AddStrval ( CSphVector<SqlInsert_t>& dVec, const SqlNode_t& tNode )
	{
		SqlInsert_t& tIns = dVec.Add();
		ToString ( tIns.m_sVal, tNode ).Unquote();
		tIns.m_iType = 2 + m_pStmt->m_iIntParam; // 2 or 3 - global or session string
	}

	void AddSetName ( StrVec_t& dVec, const SqlNode_t& tNode )
	{
		auto& sIns = dVec.Add();
		ToString ( sIns, tNode ).Unquote();
	}
};

using YYSTYPE = SqlNode_t;
STATIC_ASSERT ( IS_TRIVIALLY_COPYABLE ( SqlNode_t ), YYSTYPE_MUST_BE_TRIVIAL_FOR_RESIZABLE_PARSER_STACK );
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_DECL inline int flex_extraparser ( YYSTYPE* lvalp, void* yyscanner, SqlExtraParser_c* pParser )

#include "flexsphinxql_extra.c"

static void yyerror ( SqlParserTraits_c* pParser, const char* szMessage )
{
	// flex put a zero at last token boundary; make it undo that
	yy6lex_unhold ( pParser->m_pScanner );

	pParser->ProcessParsingError ( szMessage );
}

#ifndef NDEBUG
// using a proxy to be possible to debug inside yylex
inline int yylex ( YYSTYPE * lvalp, SqlExtraParser_c * pParser )
{
	int res = flex_extraparser ( lvalp, pParser->m_pScanner, pParser );
	return res;
}
#else
inline int yylex ( YYSTYPE * lvalp, SqlExtraParser_c * pParser )
{
	return flex_extraparser ( lvalp, pParser->m_pScanner, pParser );
}
#endif

#include "bissphinxql_extra.c"

ParseResult_e ParseExtra ( Str_t sQuery, CSphVector<SqlStmt_t>& dStmt, CSphString& sError )
{
	assert ( IsFilled ( sQuery ) );
	SqlExtraParser_c tParser ( dStmt, sQuery.first, &sError );
	tParser.m_pBuf = sQuery.first;

	char * sEnd = const_cast<char *>( sQuery.first+sQuery.second );
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	yy6lex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy6_scan_buffer ( const_cast<char *>( sQuery.first ), sQuery.second+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: yy6_scan_buffer() failed";
		return ParseResult_e::PARSE_ERROR;
	}

	int iRes = yyparse ( &tParser );

	yy6_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yy6lex_destroy ( tParser.m_pScanner );

	// special case - processing single comment directive
	if ( sQuery.second > 1
		 && sQuery.first[0] == '/'
		 && sQuery.first[1] == '*'
		 && (sError == "P05: syntax error, unexpected $end near '(null)'" // usual old bison
			  || sError == "P05: syntax error, unexpected end of file near '(null)'") ) // new bison from homebrew
	{
		tParser.DefaultOk();
		iRes = 0;
	}

	return ( iRes || dStmt.IsEmpty() ) ? ParseResult_e::PARSE_ERROR : ParseResult_e::PARSE_OK;
}
