%{

#if _WIN32
#define YY_NO_UNISTD_H 1
#endif

#include "bissphinxql_extra.h"

#define YY_NO_INPUT

#ifdef __GNUC__
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wpragmas"
	#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
#endif

#if _WIN32
#pragma warning(push,1)
#endif

#define STORE_BOUNDS \
	{ \
		lvalp->m_iStart = yytext - pParser->m_pBuf; \
		lvalp->m_iEnd = lvalp->m_iStart + yyleng; \
		pParser->m_pLastTokenStart = yytext; \
	}

#define Q_START \
	{ \
		pParser->m_pLastTokenStart = yytext  + yyleng - 1; \
	    lvalp->m_iStart = yytext  + yyleng - pParser->m_pBuf - 1; \
	}

#define Q_END \
	{ \
		lvalp->m_iEnd = yytext + yyleng - pParser->m_pBuf; \
	}

%}

%option noyywrap
%option nounput
%option reentrant
%option case-insensitive
%option batch
%option prefix="yy6"
%option align

ID					[a-zA-Z_][a-zA-Z_0-9]*
ALPHA				[a-zA-Z_]+
SPACE				[ \t\n\r]
DIGIT				[0-9]
SIGN                [+-]?
EXPONENT			[eE](\+|-)?{DIGIT}+
INT                 {DIGIT}+
FLOAT_CONSTANT      ({INT}\.{INT}?|{INT}?\.{INT}){EXPONENT}?

%x ccomment
%x acomment
%x QSTR

%%

"/*"         		{ BEGIN(ccomment); }
<ccomment>"*/"		{ BEGIN(INITIAL); return TOK_COMMENT; }
<ccomment>[^*\n]+
<ccomment>"*"[^/]
<ccomment>"\n"

^"--"         		{ BEGIN(acomment); }
<acomment>"\n"		{ BEGIN(INITIAL); return TOK_COMMENT; }
<acomment><<EOF>>   { BEGIN(INITIAL); return TOK_COMMENT; }
<acomment>[^*\n]+

'	                { Q_START; BEGIN (QSTR); }
<QSTR>[^'\n\\]+
<QSTR>'             {  Q_END; BEGIN(INITIAL); return TOK_QUOTED_STRING; }

"CREATE"			{ STORE_BOUNDS; return TOK_CREATE; }
"DATABASE"			{ STORE_BOUNDS; return TOK_DATABASE; }
"FIELDS"			{ STORE_BOUNDS; return TOK_FIELDS; }
"FLUSH" 			{ STORE_BOUNDS; return TOK_FLUSH; }
"FROM"				{ STORE_BOUNDS; return TOK_FROM; }
"GLOBAL"			{ STORE_BOUNDS; return TOK_GLOBAL; }
"LIKE"				{ STORE_BOUNDS; return TOK_LIKE; }
"LOCK"				{ STORE_BOUNDS; return TOK_LOCK; }
"READ"				{ STORE_BOUNDS; return TOK_READ; }
"RELOAD"			{ STORE_BOUNDS; return TOK_RELOAD; }
"SAVEPOINT"			{ STORE_BOUNDS; return TOK_SAVEPOINT; }
"SESSION"			{ STORE_BOUNDS; return TOK_SESSION; }
"SET"				{ STORE_BOUNDS; return TOK_SET; }
"SHOW"				{ STORE_BOUNDS; return TOK_SHOW; }
"TABLE"				{ STORE_BOUNDS; return TOK_TABLE; }
"TABLES"			{ STORE_BOUNDS; return TOK_TABLES; }
"TRIGGERS"			{ STORE_BOUNDS; return TOK_TRIGGERS; }
"UNLOCK"			{ STORE_BOUNDS; return TOK_UNLOCK; }
"USE"				{ STORE_BOUNDS; return TOK_USE; }
"WITH"				{ STORE_BOUNDS; return TOK_WITH; }
"WRITE"				{ STORE_BOUNDS; return TOK_WRITE; }


{FLOAT_CONSTANT}	{  lvalp->m_fValue = (float)strtod ( yytext, NULL ); return TOK_CONST_FLOAT; }
{INT}   			{  lvalp->SetValueInt ( strtoull ( yytext, NULL, 10 ), false); return TOK_CONST_INT; }

{ID}				{ STORE_BOUNDS; return TOK_IDENT; }
`{ID}`				{ STORE_BOUNDS; ++lvalp->m_iStart; --lvalp->m_iEnd; return TOK_IDENT; }
`{DIGIT}+{ID}`		{ STORE_BOUNDS; ++lvalp->m_iStart; --lvalp->m_iEnd; return TOK_IDENT; } // allow `123xyz` column names
`{ID}:{ID}`			{ STORE_BOUNDS; ++lvalp->m_iStart; --lvalp->m_iEnd; return TOK_TABLE_IDENT; } // specific for table names
`{ID}.{ID}`			{ STORE_BOUNDS; ++lvalp->m_iStart; --lvalp->m_iEnd; return TOK_TABLE_IDENT; }
`{ID}:{ID}.{ID}`	{ STORE_BOUNDS; ++lvalp->m_iStart; --lvalp->m_iEnd; return TOK_TABLE_IDENT; }

{SPACE}+
.					{  return yytext[0]; }

%%

// @{ID}				{ STORE_BOUNDS; return TOK_USERVAR; }
// @@{ID}				{ STORE_BOUNDS; return TOK_SYSVAR; }

static void yy6lex_unhold ( yyscan_t yyscanner )
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