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
EXPONENT			[eE](\+|-)?{DIGIT}+
ALPHA				[a-zA-Z_]+

%option noyywrap
%option nounput
%option reentrant

%x ccomment

%%

"/*"         		{ BEGIN(ccomment); }
<ccomment>.			{ }
<ccomment>"*/"		{ BEGIN(INITIAL); }

"AGENT"				{ YYSTOREBOUNDS; return TOK_AGENT; }
"ALTER"				{ YYSTOREBOUNDS; return TOK_ALTER; }
"ADD"				{ YYSTOREBOUNDS; return TOK_ADD; }
"AND"				{ YYSTOREBOUNDS; return TOK_AND; }
"AS"				{ YYSTOREBOUNDS; return TOK_AS; }
"ASC"				{ YYSTOREBOUNDS; return TOK_ASC; }
"ATTACH"			{ YYSTOREBOUNDS; return TOK_ATTACH; }
"AVG"				{ YYSTOREBOUNDS; return TOK_AVG; }
"BEGIN"				{ YYSTOREBOUNDS; return TOK_BEGIN; }
"BETWEEN"			{ YYSTOREBOUNDS; return TOK_BETWEEN; }
"BIGINT"			{ YYSTOREBOUNDS; return TOK_BIGINT; }
"BOOL"				{ YYSTOREBOUNDS; return TOK_BOOL; }
"BY"				{ YYSTOREBOUNDS; return TOK_BY; }
"CALL"				{ YYSTOREBOUNDS; return TOK_CALL; }
"CHARACTER"			{ YYSTOREBOUNDS; return TOK_CHARACTER; }
"COLLATION"			{ YYSTOREBOUNDS; return TOK_COLLATION; }
"COLUMN"			{ YYSTOREBOUNDS; return TOK_COLUMN; }
"COMMIT"			{ YYSTOREBOUNDS; return TOK_COMMIT; }
"COMMITTED"			{ YYSTOREBOUNDS; return TOK_COMMITTED; }
"COUNT"				{ YYSTOREBOUNDS; return TOK_COUNT; }
"CREATE"			{ YYSTOREBOUNDS; return TOK_CREATE; }
"DATABASES"			{ YYSTOREBOUNDS; return TOK_DATABASES; }
"DELETE"			{ YYSTOREBOUNDS; return TOK_DELETE; }
"DESC"				{ YYSTOREBOUNDS; return TOK_DESC; }
"DESCRIBE"			{ YYSTOREBOUNDS; return TOK_DESCRIBE; }
"DISTINCT"			{ YYSTOREBOUNDS; return TOK_DISTINCT; }
"DIV"				{ YYSTOREBOUNDS; return TOK_DIV; }
"DOUBLE"			{ YYSTOREBOUNDS; return TOK_DOUBLE; }
"DROP"				{ YYSTOREBOUNDS; return TOK_DROP; }
"FALSE"				{ YYSTOREBOUNDS; return TOK_FALSE; }
"FLOAT"				{ YYSTOREBOUNDS; return TOK_FLOAT; }
"FLUSH"				{ YYSTOREBOUNDS; return TOK_FLUSH; }
"FOR"				{ YYSTOREBOUNDS; return TOK_FOR; }
"FROM"				{ YYSTOREBOUNDS; return TOK_FROM; }
"FUNCTION"			{ YYSTOREBOUNDS; return TOK_FUNCTION; }
"GLOBAL"			{ YYSTOREBOUNDS; return TOK_GLOBAL; }
"GROUP"				{ YYSTOREBOUNDS; return TOK_GROUP; }
"GROUPBY"			{ YYSTOREBOUNDS; return TOK_GROUPBY; }
"GROUP_CONCAT"		{ YYSTOREBOUNDS; return TOK_GROUP_CONCAT; }
"HAVING"			{ YYSTOREBOUNDS; return TOK_HAVING; }
"ID"				{ YYSTOREBOUNDS; return TOK_ID; }
"IN"				{ YYSTOREBOUNDS; return TOK_IN; }
"INDEX"				{ YYSTOREBOUNDS; return TOK_INDEX; }
"INSERT"			{ YYSTOREBOUNDS; return TOK_INSERT; }
"INTEGER"			{ YYSTOREBOUNDS; return TOK_INTEGER; }
"INT"				{ YYSTOREBOUNDS; return TOK_INT; }
"INTO"				{ YYSTOREBOUNDS; return TOK_INTO; }
"IS"				{ YYSTOREBOUNDS; return TOK_IS; }
"ISOLATION"			{ YYSTOREBOUNDS; return TOK_ISOLATION; }
"LEVEL"				{ YYSTOREBOUNDS; return TOK_LEVEL; }
"LIKE"				{ YYSTOREBOUNDS; return TOK_LIKE; }
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
"OPTIMIZE"			{ YYSTOREBOUNDS; return TOK_OPTIMIZE; }
"OR"				{ YYSTOREBOUNDS; return TOK_OR; }
"ORDER"				{ YYSTOREBOUNDS; return TOK_ORDER; }
"PLAN"				{ YYSTOREBOUNDS; return TOK_PLAN; }
"PROFILE"			{ YYSTOREBOUNDS; return TOK_PROFILE; }
"RAMCHUNK"			{ YYSTOREBOUNDS; return TOK_RAMCHUNK; }
"RAND"				{ YYSTOREBOUNDS; return TOK_RAND; }
"READ"				{ YYSTOREBOUNDS; return TOK_READ; }
"REPEATABLE"		{ YYSTOREBOUNDS; return TOK_REPEATABLE; }
"REPLACE"			{ YYSTOREBOUNDS; return TOK_REPLACE; }
"RETURNS"			{ YYSTOREBOUNDS; return TOK_RETURNS; }
"ROLLBACK"			{ YYSTOREBOUNDS; return TOK_ROLLBACK; }
"RTINDEX"			{ YYSTOREBOUNDS; return TOK_RTINDEX; }
"SELECT"			{ YYSTOREBOUNDS; return TOK_SELECT; }
"SERIALIZABLE"		{ YYSTOREBOUNDS; return TOK_SERIALIZABLE; }
"SET"				{ YYSTOREBOUNDS; return TOK_SET; }
"SESSION"			{ YYSTOREBOUNDS; return TOK_SESSION; }
"SHOW"				{ YYSTOREBOUNDS; return TOK_SHOW; }
"SONAME"			{ YYSTOREBOUNDS; return TOK_SONAME; }
"START"				{ YYSTOREBOUNDS; return TOK_START; }
"STATUS"			{ YYSTOREBOUNDS; return TOK_STATUS; }
"STRING"			{ YYSTOREBOUNDS; return TOK_STRING; }
"SUM"				{ YYSTOREBOUNDS; return TOK_SUM; }
"TABLE"				{ YYSTOREBOUNDS; return TOK_TABLE; }
"TABLES"			{ YYSTOREBOUNDS; return TOK_TABLES; }
"TO"				{ YYSTOREBOUNDS; return TOK_TO; }
"TRANSACTION"		{ YYSTOREBOUNDS; return TOK_TRANSACTION; }
"TRUE"				{ YYSTOREBOUNDS; return TOK_TRUE; }
"TRUNCATE"			{ YYSTOREBOUNDS; return TOK_TRUNCATE; }
"UNCOMMITTED"		{ YYSTOREBOUNDS; return TOK_UNCOMMITTED; }
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

'([^'\\]|\\.|\\\\)*'			{ YYSTOREBOUNDS; pParser->m_pLastTokenStart = yytext; return TOK_QUOTED_STRING; }

{DIGIT}+\.{DIGIT}*{EXPONENT}?	{ YYSTOREBOUNDS; lvalp->m_fValue = (float)strtod ( yytext, NULL ); return TOK_CONST_FLOAT; }
\.{DIGIT}+{EXPONENT}?			{ YYSTOREBOUNDS; lvalp->m_fValue = (float)strtod ( yytext, NULL ); return TOK_DOT_NUMBER; }
\.{DIGIT}+{ID}					{ YYSTOREBOUNDS; return TOK_SUBKEY; }
{DIGIT}+{EXPONENT}				{ YYSTOREBOUNDS; lvalp->m_fValue = (float)strtod ( yytext, NULL ); return TOK_CONST_FLOAT; }
{DIGIT}+			{ YYSTOREBOUNDS; lvalp->m_iValue = (int64_t)strtoull ( yytext, NULL, 10 ); return TOK_CONST_INT; }

"@id"				{ YYSTOREBOUNDS; return TOK_ATIDENT; }
"@weight"			{ YYSTOREBOUNDS; return TOK_ATIDENT; }
"@count"			{ YYSTOREBOUNDS; return TOK_ATIDENT; }
{ID}				{ YYSTOREBOUNDS; return TOK_IDENT; }
[a-zA-Z_0-9]+		{ YYSTOREBOUNDS; return TOK_BAD_NUMERIC; }
@{ID}				{ YYSTOREBOUNDS; return TOK_USERVAR; }
@@{ID}				{ YYSTOREBOUNDS; return TOK_SYSVAR; }
@@{ID}\.{ID}		{ YYSTOREBOUNDS; return TOK_SYSVAR; }
"`ID`"				{ YYSTOREBOUNDS; lvalp->m_iStart++; lvalp->m_iEnd--; return TOK_ID; }
`{ID}`				{ YYSTOREBOUNDS; lvalp->m_iStart++; lvalp->m_iEnd--; return TOK_IDENT; }

{SPACE}+			{ ; }
\.{SPACE}*{ID}		{ YYSTOREBOUNDS; return TOK_SUBKEY; }
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
