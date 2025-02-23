%{

#if _WIN32
#define YY_NO_UNISTD_H 1
#endif

#include "bissphinxql_second.h"

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
%option prefix="yy5"
%option align

ID					[a-zA-Z_@][a-zA-Z_0-9]*
SYS_ID				[a-zA-Z_][a-zA-Z_0-9]*
ALPHA				[a-zA-Z_]+
SPACE				[ \t\n\r]
DIGIT				[0-9]
SIGN                [+-]?
EXPONENT			[eE](\+|-)?{DIGIT}+
INT                 {DIGIT}+
FLOAT_CONSTANT      ({INT}\.{INT}?|{INT}?\.{INT}){EXPONENT}?

%x ccomment
%x QSTR
%x BACKTICK

%%

"/*"         		{ BEGIN(ccomment); }
<ccomment>"*/"		{ BEGIN(INITIAL); }
<ccomment>[^*\n]+
<ccomment>"*"[^/]
<ccomment>"\n"

'	                { Q_START; BEGIN (QSTR); }
<QSTR>[^'\n\\]+
<QSTR>'             {  Q_END; BEGIN(INITIAL); return TOK_QUOTED_STRING; }

\.{SPACE}*`	           	{ Q_START; BEGIN (BACKTICK); }
<BACKTICK>[^\n\\`]+		/* eat everything except \n, \, ` */
<BACKTICK>\\[^\n]		/* eat one symbol after \ (aka escaping) except \n  */
<BACKTICK>`				{
							Q_END;
							BEGIN(INITIAL);
							return TOK_BACKTICKED_SUBKEY;
						}

<BACKTICK>\n			|	/* terminate on \n or EOF inside unclosed backtick */
<BACKTICK><<EOF>>		{
							BEGIN(INITIAL);
							return -1;
						}

"ATTACH"    		{ STORE_BOUNDS; return TOK_ATTACH; }
"ATTRIBUTES"   		{ STORE_BOUNDS; return TOK_ATTRIBUTES; }
"CLUSTER"			{ STORE_BOUNDS; return TOK_CLUSTER; }
"COMMITTED"         { STORE_BOUNDS; return TOK_COMMITTED; }
"COMPRESS"			{ STORE_BOUNDS; return TOK_COMPRESS; }
"DELETE"			{ STORE_BOUNDS; return TOK_DELETE; }
"FLUSH"             { STORE_BOUNDS; return TOK_FLUSH; }
"FREEZE"			{ STORE_BOUNDS; return TOK_FREEZE; }
"FROM"              { STORE_BOUNDS; return TOK_FROM; }
"GLOBAL"			{ STORE_BOUNDS; return TOK_GLOBAL; }
"HOSTNAMES"         { STORE_BOUNDS; return TOK_HOSTNAMES; }
"INDEX"				{ STORE_BOUNDS; return TOK_INDEX; }
"INDEXES"	    	{ STORE_BOUNDS; return TOK_INDEXES; }
"ISOLATION"			{ STORE_BOUNDS; return TOK_ISOLATION; }
"KILL"				{ STORE_BOUNDS; return TOK_KILL; }
"LEVEL"				{ STORE_BOUNDS; return TOK_LEVEL; }
"LIKE"				{ STORE_BOUNDS; return TOK_LIKE; }
"LOGS"              { STORE_BOUNDS; return TOK_LOGS; }
"OFF"				{ STORE_BOUNDS; return TOK_OFF; }
"ON"				{ STORE_BOUNDS; return TOK_ON; }
"OPTION"			{ STORE_BOUNDS; return TOK_OPTION; }
"PLUGINS"           { STORE_BOUNDS; return TOK_PLUGINS; }
"QUERY"				{ STORE_BOUNDS; return TOK_QUERY; }
"RAMCHUNK"          { STORE_BOUNDS; return TOK_RAMCHUNK; }
"READ"				{ STORE_BOUNDS; return TOK_READ; }
"RECONFIGURE"       { STORE_BOUNDS; return TOK_RECONFIGURE; }
"RELOAD"            { STORE_BOUNDS; return TOK_RELOAD; }
"REPEATABLE"        { STORE_BOUNDS; return TOK_REPEATABLE; }
"RTINDEX"           { STORE_BOUNDS; return TOK_RTINDEX; }
"SERIALIZABLE"      { STORE_BOUNDS; return TOK_SERIALIZABLE; }
"SESSION"			{ STORE_BOUNDS; return TOK_SESSION; }
"SET"	    		{ STORE_BOUNDS; return TOK_SET; }
"SONAME"            { STORE_BOUNDS; return TOK_SONAME; }
"TABLE"				{ STORE_BOUNDS; return TOK_TABLE; }
"TABLES"			{ STORE_BOUNDS; return TOK_TABLES; }
"TO"                { STORE_BOUNDS; return TOK_TO; }
"TRANSACTION"		{ STORE_BOUNDS; return TOK_TRANSACTION; }
"TRUNCATE"          { STORE_BOUNDS; return TOK_TRUNCATE; }
"UNCOMMITTED"       { STORE_BOUNDS; return TOK_UNCOMMITTED; }
"UNFREEZE"			{ STORE_BOUNDS; return TOK_UNFREEZE; }
"WAIT"				{ STORE_BOUNDS; return TOK_WAIT; }
"WITH"              { STORE_BOUNDS; return TOK_WITH; }

"SYSTEM."           { STORE_BOUNDS; return TOK_SYSTEM_DOT; }


{FLOAT_CONSTANT}	{ STORE_BOUNDS; lvalp->m_fValue = (float)strtod ( yytext, NULL ); return TOK_CONST_FLOAT; }
{INT}   			{ STORE_BOUNDS; lvalp->SetValueInt ( strtoull ( yytext, NULL, 10 ), false); return TOK_CONST_INT; }

{ID}				{ STORE_BOUNDS; return TOK_IDENT; }
`{ID}`				{ STORE_BOUNDS; ++lvalp->m_iStart; --lvalp->m_iEnd; return TOK_IDENT; }
`{DIGIT}+{ID}`		{ STORE_BOUNDS; ++lvalp->m_iStart; --lvalp->m_iEnd; return TOK_IDENT; } // allow `123xyz` column names
@@{SYS_ID}			{ STORE_BOUNDS; return TOK_SYSVAR; }

{SPACE}+
.					{  return yytext[0]; }

%%

static void yy5lex_unhold ( yyscan_t yyscanner )
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