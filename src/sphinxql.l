%{
#include "yysphinxql.h"

#if USE_WINDOWS
#pragma warning(push,1)
#endif

// warning, lexer generator dependent!
// this macro relies on that in flex yytext points to the actual location in the buffer
#define YYSTOREBOUNDS \
	{ \
		lvalp->m_iStart = yytext - pParser->m_pBuf; \
		lvalp->m_iEnd = lvalp->m_iStart + yyleng; \
		pParser->m_pLastTokenStart = yytext; \
	}

%}

DIGIT				[0-9]
ID					[a-zA-Z_][a-zA-Z_0-9]*
SPACE				[ \t\n\r]

%option noyywrap
%option nounput
%option reentrant

%%

"AS"				{ YYSTOREBOUNDS; return TOK_AS; }
"ASC"				{ YYSTOREBOUNDS; return TOK_ASC; }
"AVG"				{ YYSTOREBOUNDS; return TOK_AVG; }
"BETWEEN"			{ YYSTOREBOUNDS; return TOK_BETWEEN; }
"BY"				{ YYSTOREBOUNDS; return TOK_BY; }
"COUNT"				{ YYSTOREBOUNDS; return TOK_COUNT; }
"DELETE"			{ YYSTOREBOUNDS; return TOK_DELETE; }
"DESC"				{ YYSTOREBOUNDS; return TOK_DESC; }
"DISTINCT"			{ YYSTOREBOUNDS; return TOK_DISTINCT; }
"FROM"				{ YYSTOREBOUNDS; return TOK_FROM; }
"GROUP"				{ YYSTOREBOUNDS; return TOK_GROUP; }
"LIMIT"				{ YYSTOREBOUNDS; return TOK_LIMIT; }
"IN"				{ YYSTOREBOUNDS; return TOK_IN; }
"INSERT"			{ YYSTOREBOUNDS; return TOK_INSERT; }
"INTO"				{ YYSTOREBOUNDS; return TOK_INTO; }
"ID"				{ YYSTOREBOUNDS; return TOK_ID; }
"MATCH"				{ YYSTOREBOUNDS; return TOK_MATCH; }
"MAX"				{ YYSTOREBOUNDS; return TOK_MAX; }
"META"				{ YYSTOREBOUNDS; return TOK_META; }
"MIN"				{ YYSTOREBOUNDS; return TOK_MIN; }
"OPTION"			{ YYSTOREBOUNDS; return TOK_OPTION; }
"ORDER"				{ YYSTOREBOUNDS; return TOK_ORDER; }
"REPLACE"			{ YYSTOREBOUNDS; return TOK_REPLACE; }
"SELECT"			{ YYSTOREBOUNDS; return TOK_SELECT; }
"SHOW"				{ YYSTOREBOUNDS; return TOK_SHOW; }
"STATUS"			{ YYSTOREBOUNDS; return TOK_STATUS; }
"SUM"				{ YYSTOREBOUNDS; return TOK_SUM; }
"VALUES"			{ YYSTOREBOUNDS; return TOK_VALUES; }
"WARNINGS"			{ YYSTOREBOUNDS; return TOK_WARNINGS; }
"WEIGHT"			{ YYSTOREBOUNDS; return TOK_WEIGHT; }
"WITHIN"			{ YYSTOREBOUNDS; return TOK_WITHIN; }
"WHERE"				{ YYSTOREBOUNDS; return TOK_WHERE; }
"SET"				{ YYSTOREBOUNDS; return TOK_SET; }
"COMMIT"			{ YYSTOREBOUNDS; return TOK_COMMIT; }
"ROLLBACK"			{ YYSTOREBOUNDS; return TOK_ROLLBACK; }
"START"				{ YYSTOREBOUNDS; return TOK_START; }
"TRANSACTION"			{ YYSTOREBOUNDS; return TOK_TRANSACTION; }
"BEGIN"				{ YYSTOREBOUNDS; return TOK_BEGIN; }
"TRUE"				{ YYSTOREBOUNDS; return TOK_TRUE; }
"FALSE"				{ YYSTOREBOUNDS; return TOK_FALSE; }

"AND"				{ YYSTOREBOUNDS; return TOK_AND; }
"OR"				{ YYSTOREBOUNDS; return TOK_OR; }
"NOT"				{ YYSTOREBOUNDS; return TOK_NOT; }

"!="				{ YYSTOREBOUNDS; return TOK_NE; }
"<>"				{ YYSTOREBOUNDS; return TOK_NE; }
"<="				{ YYSTOREBOUNDS; return TOK_LTE; }
">="				{ YYSTOREBOUNDS; return TOK_GTE; }

'([^'\\]|\\.|\\\\)*'	{ YYSTOREBOUNDS; SqlUnescape ( lvalp->m_sValue, yytext, yyleng ); return TOK_QUOTED_STRING; }

-*{DIGIT}*\.{DIGIT}*	{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; lvalp->m_fValue = (float)strtod ( yytext, NULL ); return TOK_CONST_FLOAT; }
-*{DIGIT}+				{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; lvalp->m_iValue = strtoll ( yytext, NULL, 10 ); return TOK_CONST_INT; }

"@id"				{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_IDENT; }
"@weight"			{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_IDENT; }
"@count"			{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_IDENT; }
{ID}				{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_IDENT; }

{SPACE}+			{ ; }
.					{ YYSTOREBOUNDS; return yytext[0]; }

%%

// warning, lexer generator dependent!
// flex inserts trailing zero as needed into the buffer when lexing
// but we need that rolled back when doing error reporting from yyerror
void yylex_unhold ( yyscan_t yyscanner )
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	if ( YY_CURRENT_BUFFER )
	{
		*yyg->yy_c_buf_p = yyg->yy_hold_char;
		YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
		YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
	}
}

#if USE_WINDOWS
#pragma warning(pop)
#endif
