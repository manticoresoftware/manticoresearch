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


#include "sphinxql_second.h"

struct BlobLocator_t
{
	int m_iStart;
	int m_iLen;
};


class SqlSecondParser_c : public SqlParserTraits_c
{
public:
	SqlSecondParser_c ( CSphVector<SqlStmt_t>& dStmt, const char* szQuery, CSphString* pError)
		: SqlParserTraits_c ( dStmt, szQuery, pError )
	{
		if ( m_dStmt.IsEmpty() )
			PushQuery();
		else
			m_pStmt = &m_dStmt.Last();
		m_sErrorHeader = "P02:";
	}

	CSphString StrFromBlob ( BlobLocator_t tStr ) const
	{
		CSphString sResult;
		sResult.SetBinary(m_pBuf+tStr.m_iStart, tStr.m_iLen);
		return sResult;
	}

	void SetStatement ( const SqlNode_t & tName, SqlSet_e eSet );
	void SetStatement ( const SqlNode_t & tName, SqlSet_e eSet, int iValuesIdx );
};

void SqlSecondParser_c::SetStatement ( const SqlNode_t& tName, SqlSet_e eSet )
{
	m_pStmt->m_eStmt = STMT_SET;
	m_pStmt->m_eSet = eSet;
	ToString ( m_pStmt->m_sSetName, tName );
}


void SqlSecondParser_c::SetStatement ( const SqlNode_t & tName, SqlSet_e eSet, int iValuesIdx )
{
	SetStatement ( tName, eSet );
	
	auto & dSV = m_pStmt->m_dSetValues;
	const auto& dValues = GetMvaVec ( iValuesIdx );
	dSV.Resize ( dValues.GetLength() );
	ARRAY_FOREACH ( i, dValues )
		dSV[i] = dValues[i].m_iValue;
}

using YYSTYPE = SqlNode_t;
STATIC_ASSERT ( IS_TRIVIALLY_COPYABLE ( SqlNode_t ), YYSTYPE_MUST_BE_TRIVIAL_FOR_RESIZABLE_PARSER_STACK );
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_DECL inline int flex_secondparser ( YYSTYPE* lvalp, void* yyscanner, SqlSecondParser_c* pParser )

#include "flexsphinxql_second.c"

static void yyerror ( SqlParserTraits_c* pParser, const char* szMessage )
{
	// flex put a zero at last token boundary; make it undo that
	yy5lex_unhold ( pParser->m_pScanner );

	pParser->ProcessParsingError ( szMessage );
}

#ifndef NDEBUG
// using a proxy to be possible to debug inside yylex
inline int yylex ( YYSTYPE * lvalp, SqlSecondParser_c * pParser )
{
	int res = flex_secondparser ( lvalp, pParser->m_pScanner, pParser );
	return res;
}
#else
inline int yylex ( YYSTYPE * lvalp, SqlSecondParser_c * pParser )
{
	return flex_secondparser ( lvalp, pParser->m_pScanner, pParser );
}
#endif

#include "bissphinxql_second.c"

ParseResult_e ParseSecond ( Str_t sQuery, CSphVector<SqlStmt_t>& dStmt, CSphString& sError )
{
	assert ( IsFilled ( sQuery ) );
	SqlSecondParser_c tParser ( dStmt, sQuery.first, &sError );
	tParser.m_pBuf = sQuery.first;

	char * sEnd = const_cast<char *>( sQuery.first+sQuery.second );
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	yy5lex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy5_scan_buffer ( const_cast<char *>( sQuery.first ), sQuery.second+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: yy5_scan_buffer() failed";
		return ParseResult_e::PARSE_ERROR;
	}

	int iRes = yyparse ( &tParser );

	yy5_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yy5lex_destroy ( tParser.m_pScanner );

	return ( iRes || dStmt.IsEmpty() ) ? ParseResult_e::PARSE_ERROR : ParseResult_e::PARSE_OK;
}
