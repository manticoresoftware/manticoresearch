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

class SqlParameterValidator_c
{
public:
	const char *	m_pBuf;
	CSphString *	m_pParseError;
	void *			m_pScanner = nullptr;
	bool			m_bIsValid = true;

	NONCOPYMOVABLE ( SqlParameterValidator_c );
	SqlParameterValidator_c ( const char* szQuery, CSphString* pError )
		: m_pBuf { szQuery }
		, m_pParseError { pError }
	{}

	void UnexpectedSymbol (const char* tPos)
	{
		assert ( m_pParseError );
		m_pParseError->SetSprintf ("ValidateParam: unexpected symbol '%c' at pos %d (expected list of numbers)", *tPos, (int)(tPos-m_pBuf));
		m_bIsValid = false;
	}
};

using YYSTYPE = int;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_DECL inline int flex_validateparams ( YYSTYPE* lvalp, void* yyscanner, SqlParameterValidator_c* pParser )

#include "flexvalidatevec.c"

#ifndef NDEBUG
// using a proxy to be possible to debug inside yylex
inline int yylex ( YYSTYPE * lvalp, SqlParameterValidator_c * pParser )
{
	int res = flex_validateparams ( lvalp, pParser->m_pScanner, pParser );
	return res;
}
#else
inline int yylex ( YYSTYPE * lvalp, SqlParameterValidator_c * pParser )
{
	return flex_validateparams ( lvalp, pParser->m_pScanner, pParser );
}
#endif

bool ValidateVector ( Str_t sQuery, CSphString& sError )
{
	assert ( IsFilled ( sQuery ) );
	SqlParameterValidator_c tParser { sQuery.first, &sError };

	char * sEnd = const_cast<char *>( sQuery.first+sQuery.second );
	sEnd[0] = 0; // prepare for val_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	vallex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = val_scan_buffer ( const_cast<char *>( sQuery.first ), sQuery.second+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: val_scan_buffer() failed";
		return false;
	}

	YYSTYPE tToken;
	while ( yylex (&tToken, &tParser) )
		;

	val_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	vallex_destroy ( tParser.m_pScanner );

	return tParser.m_bIsValid;
}
