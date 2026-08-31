//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "checks.h"
#include "std/generics.h"
#include "std/string.h"
#include "std/vector.h"

struct BlobLocator_t
{
	int m_iStart;
	int m_iLen;
};

/// parser view on a generic node
/// CAUTION, nodes get copied in the parser all the time, must keep assignment slim
struct SqlToken_t final
{
	int						m_iStart = 0;	///< first byte relative to m_pBuf, inclusive
	int						m_iEnd = 0;		///< last byte relative to m_pBuf, exclusive! thus length = end - start
	EPARAM_TYPE				m_eType = EPARAM_TYPE::SINGLE;
};

class SqlParameterParser_c
{
public:
	const char *	m_pBuf;
	CSphString *	m_pParseError;
	CSphVector<tScanParam>& m_dParams;
	void *			m_pScanner = nullptr;

	NONCOPYMOVABLE ( SqlParameterParser_c );
	SqlParameterParser_c ( const char* szQuery, CSphString* pError, CSphVector<tScanParam>& dParams )
		: m_pBuf { szQuery }
		, m_pParseError { pError }
		, m_dParams {dParams}
	{}

	void AddParam (const SqlToken_t& tParam)
	{
//		sphWarning ("Add param");
		m_dParams.Add ({tParam.m_iStart, tParam.m_iEnd-tParam.m_iStart, tParam.m_eType });
	}

	void UnexpectedDelim (const char* tPos)
	{
		assert ( m_pParseError );
		m_pParseError->SetSprintf ("PreparedScan: unexpected ; at pos %d (prepared statements allows only one single statement)", (int)(tPos-m_pBuf));
	}
};

using YYSTYPE = SqlToken_t;
static_assert ( IS_TRIVIALLY_COPYABLE ( SqlToken_t ), "YYSTYPE must be trivial for resizable parser stack" );
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_DECL inline int flex_scanparams ( YYSTYPE* lvalp, void* yyscanner, SqlParameterParser_c* pParser )

#include "flexscanparams.c"

#ifndef NDEBUG
// using a proxy to be possible to debug inside yylex
inline int yylex ( YYSTYPE * lvalp, SqlParameterParser_c * pParser )
{
	int res = flex_scanparams ( lvalp, pParser->m_pScanner, pParser );
	return res;
}
#else
inline int yylex ( YYSTYPE * lvalp, SqlParameterParser_c * pParser )
{
	return flex_scanparams ( lvalp, pParser->m_pScanner, pParser );
}
#endif

ParseResult_e ScanParameters ( Str_t sQuery, CSphVector<tScanParam>& dParams, CSphString& sError )
{
	assert ( IsFilled ( sQuery ) );
	SqlParameterParser_c tParser { sQuery.first, &sError, dParams };

	char * sEnd = const_cast<char *>( sQuery.first+sQuery.second );
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	yy7lex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy7_scan_buffer ( const_cast<char *>( sQuery.first ), sQuery.second+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: yy7_scan_buffer() failed";
		return ParseResult_e::PARSE_ERROR;
	}

	YYSTYPE tToken;
	while ( yylex (&tToken, &tParser) )
		;

	int iRes = tParser.m_pParseError->IsEmpty()?0:1;

	yy7_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yy7lex_destroy ( tParser.m_pScanner );

	return ( iRes ) ? ParseResult_e::PARSE_ERROR : ParseResult_e::PARSE_OK;
}
