%{

#if _WIN32
#define YY_NO_UNISTD_H 1
#endif

#include "bisddl.h"

#ifdef __GNUC__
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wpragmas"
	#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
#endif

#if _WIN32
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
ID					[a-zA-Z_@][a-zA-Z_0-9]*
SPACE				[ \t\n\r]
EXPONENT			[eE](\+|-)?{DIGIT}+
ALPHA				[a-zA-Z_]+

%option prefix="yy3"
%option noyywrap
%option nounput
%option reentrant
%option case-insensitive
%option batch
%option align

%x ccomment

%%

"/*"         		{ BEGIN(ccomment); }
<ccomment>.			{ }
<ccomment>"*/"		{ BEGIN(INITIAL); }

"ADD"				{ YYSTOREBOUNDS; return TOK_ADD; }
"ALTER"				{ YYSTOREBOUNDS; return TOK_ALTER; }
"AS"				{ YYSTOREBOUNDS; return TOK_AS; }
"AT"				{ YYSTOREBOUNDS; return TOK_AT; }
"ATTRIBUTE"			{ YYSTOREBOUNDS; return TOK_ATTRIBUTE; }
"BIGINT"			{ YYSTOREBOUNDS; return TOK_BIGINT; }
"BIT"				{ YYSTOREBOUNDS; return TOK_BIT; }
"BOOL"				{ YYSTOREBOUNDS; return TOK_BOOL; }
"CLUSTER"			{ YYSTOREBOUNDS; return TOK_CLUSTER; }
"COLUMN"			{ YYSTOREBOUNDS; return TOK_COLUMN; }
"COLUMNAR"			{ YYSTOREBOUNDS; return TOK_COLUMNAR; }
"CREATE"			{ YYSTOREBOUNDS; return TOK_CREATE; }
"DOUBLE"			{ YYSTOREBOUNDS; return TOK_DOUBLE; }
"DROP"				{ YYSTOREBOUNDS; return TOK_DROP; }
"ENGINE"			{ YYSTOREBOUNDS; return TOK_ENGINE; }
"EXISTS"			{ YYSTOREBOUNDS; return TOK_EXISTS; }
"FAST_FETCH"		{ YYSTOREBOUNDS; return TOK_FAST_FETCH; }
"FLOAT"				{ YYSTOREBOUNDS; return TOK_FLOAT; }
"FROM"				{ YYSTOREBOUNDS; return TOK_FROM; }
"FUNCTION"			{ YYSTOREBOUNDS; return TOK_FUNCTION; }
"HASH"				{ YYSTOREBOUNDS; return TOK_HASH; }
"IMPORT"			{ YYSTOREBOUNDS; return TOK_IMPORT; }
"INDEXED"			{ YYSTOREBOUNDS; return TOK_INDEXED; }
"INTEGER"			{ YYSTOREBOUNDS; return TOK_INTEGER; }
"INT"				{ YYSTOREBOUNDS; return TOK_INT; }
"IF"				{ YYSTOREBOUNDS; return TOK_IF; }
"JOIN"				{ YYSTOREBOUNDS; return TOK_JOIN; }
"JSON"				{ YYSTOREBOUNDS; return TOK_JSON; }
"KILLLIST_TARGET"	{ YYSTOREBOUNDS; return TOK_KILLLIST_TARGET; }
"LIKE"				{ YYSTOREBOUNDS; return TOK_LIKE; }
"MULTI"				{ YYSTOREBOUNDS; return TOK_MULTI; }
"MULTI64"			{ YYSTOREBOUNDS; return TOK_MULTI64; }
"NOT"				{ YYSTOREBOUNDS; return TOK_NOT; }
"PLUGIN"			{ YYSTOREBOUNDS; return TOK_PLUGIN; }
"REBUILD"			{ YYSTOREBOUNDS; return TOK_REBUILD; }
"RECONFIGURE"		{ YYSTOREBOUNDS; return TOK_RECONFIGURE; }
"RETURNS"			{ YYSTOREBOUNDS; return TOK_RETURNS; }
"RTINDEX"			{ YYSTOREBOUNDS; return TOK_RTINDEX; }
"SECONDARY"			{ YYSTOREBOUNDS; return TOK_SECONDARY; }
"SONAME"			{ YYSTOREBOUNDS; return TOK_SONAME; }
"STORED"			{ YYSTOREBOUNDS; return TOK_STORED; }
"STRING"			{ YYSTOREBOUNDS; return TOK_STRING; }
"TABLE"				{ YYSTOREBOUNDS; return TOK_TABLE; }
"TEXT"				{ YYSTOREBOUNDS; return TOK_TEXT; }
"TIMESTAMP"			{ YYSTOREBOUNDS; return TOK_TIMESTAMP; }
"TYPE"				{ YYSTOREBOUNDS; return TOK_TYPE; }
"UINT"				{ YYSTOREBOUNDS; return TOK_UINT; }
"UPDATE"			{ YYSTOREBOUNDS; return TOK_UPDATE; }

'([^'\\]|\\.|\\\\)*' { YYSTOREBOUNDS; pParser->m_pLastTokenStart = yytext; lvalp->SetValueInt ( ( (SphAttr_t)lvalp->m_iStart<<32 ) | ( lvalp->m_iEnd-lvalp->m_iStart ) ); return TOK_QUOTED_STRING; }

{DIGIT}+\.{DIGIT}*{EXPONENT}?	{ YYSTOREBOUNDS; lvalp->m_fValue = (float)strtod ( yytext, NULL ); return TOK_CONST_FLOAT; }
{DIGIT}+			{ YYSTOREBOUNDS; lvalp->SetValueInt ( strtoull ( yytext, NULL, 10 ), false ); return TOK_CONST_INT; }

{ID}				{ YYSTOREBOUNDS; return TOK_IDENT; }
`{ID}`				{ YYSTOREBOUNDS; ++lvalp->m_iStart; --lvalp->m_iEnd; return TOK_IDENT; }
`{DIGIT}+{ID}`		{ YYSTOREBOUNDS; ++lvalp->m_iStart; --lvalp->m_iEnd; return TOK_IDENT; } // allow `123xyz` column names

{SPACE}+			{ ; }
.					{ YYSTOREBOUNDS; return yytext[0]; }

%%

// warning, lexer generator dependent!
// flex inserts trailing zero as needed into the buffer when lexing
// but we need that rolled back when doing error reporting from yyerror
static void yy3lex_unhold ( yyscan_t yyscanner )
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	if ( YY_CURRENT_BUFFER )
	{
		*yyg->yy_c_buf_p = yyg->yy_hold_char;
		YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
		YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
	}
}

#if _WIN32
#pragma warning(pop)
#endif

#ifdef __GNUC__
	#pragma GCC diagnostic pop
#endif