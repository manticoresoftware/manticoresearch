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

%x ccomment

%%

"/*"         		{ BEGIN(ccomment); }
<ccomment>.			{ }
<ccomment>"*/"		{ BEGIN(INITIAL); }

"AND"				{ YYSTOREBOUNDS; return TOK_AND; }
"AS"				{ YYSTOREBOUNDS; return TOK_AS; }
"ASC"				{ YYSTOREBOUNDS; return TOK_ASC; }
"AVG"				{ YYSTOREBOUNDS; return TOK_AVG; }
"BEGIN"				{ YYSTOREBOUNDS; return TOK_BEGIN; }
"BETWEEN"			{ YYSTOREBOUNDS; return TOK_BETWEEN; }
"BY"				{ YYSTOREBOUNDS; return TOK_BY; }
"CALL"				{ YYSTOREBOUNDS; return TOK_CALL; }
"COLLATION"			{ YYSTOREBOUNDS; return TOK_COLLATION; }
"COMMIT"			{ YYSTOREBOUNDS; return TOK_COMMIT; }
"COUNT"				{ YYSTOREBOUNDS; return TOK_COUNT; }
"CREATE"			{ YYSTOREBOUNDS; return TOK_CREATE; }
"DELETE"			{ YYSTOREBOUNDS; return TOK_DELETE; }
"DESC"				{ YYSTOREBOUNDS; return TOK_DESC; }
"DESCRIBE"			{ YYSTOREBOUNDS; return TOK_DESCRIBE; }
"DISTINCT"			{ YYSTOREBOUNDS; return TOK_DISTINCT; }
"DIV"				{ YYSTOREBOUNDS; return TOK_DIV; }
"DROP"				{ YYSTOREBOUNDS; return TOK_DROP; }
"FALSE"				{ YYSTOREBOUNDS; return TOK_FALSE; }
"FLOAT"				{ YYSTOREBOUNDS; return TOK_FLOAT; }
"FROM"				{ YYSTOREBOUNDS; return TOK_FROM; }
"FUNCTION"			{ YYSTOREBOUNDS; return TOK_FUNCTION; }
"GLOBAL"			{ YYSTOREBOUNDS; return TOK_GLOBAL; }
"GROUP"				{ YYSTOREBOUNDS; return TOK_GROUP; }
"ID"				{ YYSTOREBOUNDS; return TOK_ID; }
"IN"				{ YYSTOREBOUNDS; return TOK_IN; }
"INSERT"			{ YYSTOREBOUNDS; return TOK_INSERT; }
"INT"				{ YYSTOREBOUNDS; return TOK_INT; }
"INTO"				{ YYSTOREBOUNDS; return TOK_INTO; }
"LIMIT"				{ YYSTOREBOUNDS; return TOK_LIMIT; }
"MATCH"				{ YYSTOREBOUNDS; return TOK_MATCH; }
"MAX"				{ YYSTOREBOUNDS; return TOK_MAX; }
"META"				{ YYSTOREBOUNDS; return TOK_META; }
"MIN"				{ YYSTOREBOUNDS; return TOK_MIN; }
"MOD"				{ YYSTOREBOUNDS; return TOK_MOD; }
"NAMES"				{ YYSTOREBOUNDS; return TOK_NAMES; }
"NOT"				{ YYSTOREBOUNDS; return TOK_NOT; }
"NULL"				{ YYSTOREBOUNDS; return TOK_NULL; }
"OPTION"			{ YYSTOREBOUNDS; return TOK_OPTION; }
"OR"				{ YYSTOREBOUNDS; return TOK_OR; }
"ORDER"				{ YYSTOREBOUNDS; return TOK_ORDER; }
"REPLACE"			{ YYSTOREBOUNDS; return TOK_REPLACE; }
"RETURNS"			{ YYSTOREBOUNDS; return TOK_RETURNS; }
"ROLLBACK"			{ YYSTOREBOUNDS; return TOK_ROLLBACK; }
"SELECT"			{ YYSTOREBOUNDS; return TOK_SELECT; }
"SET"				{ YYSTOREBOUNDS; return TOK_SET; }
"SESSION"			{ YYSTOREBOUNDS; return TOK_SESSION; }
"SHOW"				{ YYSTOREBOUNDS; return TOK_SHOW; }
"SONAME"			{ YYSTOREBOUNDS; return TOK_SONAME; }
"START"				{ YYSTOREBOUNDS; return TOK_START; }
"STATUS"			{ YYSTOREBOUNDS; return TOK_STATUS; }
"SUM"				{ YYSTOREBOUNDS; return TOK_SUM; }
"TABLES"			{ YYSTOREBOUNDS; return TOK_TABLES; }
"TRANSACTION"		{ YYSTOREBOUNDS; return TOK_TRANSACTION; }
"TRUE"				{ YYSTOREBOUNDS; return TOK_TRUE; }
"UPDATE"			{ YYSTOREBOUNDS; return TOK_UPDATE; }
"VALUES"			{ YYSTOREBOUNDS; return TOK_VALUES; }
"VARIABLES"			{ YYSTOREBOUNDS; return TOK_VARIABLES; }
"WARNINGS"			{ YYSTOREBOUNDS; return TOK_WARNINGS; }
"WEIGHT"			{ YYSTOREBOUNDS; return TOK_WEIGHT; }
"WHERE"				{ YYSTOREBOUNDS; return TOK_WHERE; }
"WITHIN"			{ YYSTOREBOUNDS; return TOK_WITHIN; }

"!="				{ YYSTOREBOUNDS; return TOK_NE; }
"<>"				{ YYSTOREBOUNDS; return TOK_NE; }
"<="				{ YYSTOREBOUNDS; return TOK_LTE; }
">="				{ YYSTOREBOUNDS; return TOK_GTE; }
":="				{ YYSTOREBOUNDS; return '='; }

'([^'\\]|\\.|\\\\)*'	{ YYSTOREBOUNDS; SqlUnescape ( lvalp->m_sValue, yytext, yyleng ); return TOK_QUOTED_STRING; }

{DIGIT}*\.{DIGIT}*	{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; lvalp->m_fValue = (float)strtod ( yytext, NULL ); return TOK_CONST_FLOAT; }
{DIGIT}+			{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; lvalp->m_iValue = strtoll ( yytext, NULL, 10 ); return TOK_CONST_INT; }

"@id"				{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_ATIDENT; }
"@weight"			{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_ATIDENT; }
"@count"			{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_ATIDENT; }
{ID}				{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_IDENT; }
@{ID}				{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_USERVAR; }
@@{ID}				{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_SYSVAR; }
`{ID}`				{ YYSTOREBOUNDS; lvalp->m_iStart++; lvalp->m_iEnd--; lvalp->m_sValue.SetBinary ( yytext+1, strlen(yytext)-2 ); return TOK_IDENT; }

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
