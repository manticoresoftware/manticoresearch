%{
#include "bissphinxql_debug.h"

#define YY_NO_INPUT

#if USE_WINDOWS
#define YY_NO_UNISTD_H 1
#pragma warning(push,1)
#endif

#define STORE_BOUNDS \
	{ \
		lvalp->sValue.m_iStart = yytext - pParser->m_pBuf; \
		lvalp->sValue.m_iLen = yyleng; \
	}

#define STORE_START \
	{ \
	    lvalp->sValue.m_iStart = yytext - pParser->m_pBuf + 1; \
	}

#define STORE_END \
	{ \
		lvalp->sValue.m_iLen = yyleng - lvalp->sValue.m_iStart + yytext - pParser->m_pBuf - 1; \
	}

%}

%option noyywrap
%option nounput
%option reentrant
%option case-insensitive
%option batch
%option prefix="yy4"
%option align

ID					[a-zA-Z_][a-zA-Z_0-9]*
ALPHA				[a-zA-Z_]+
SPACE				[ \t\n\r]
DIGIT				[0-9]
SIGN                [+-]?
EXPONENT			[eE]{SIGN}{DIGIT}+
I                   {DIGIT}+
FLOAT_CONSTANT      ({I}\.{I}?|{I}?\.{I}){EXPONENT}?

%x QSTR

%%

"DEBUG"				{ STORE_BOUNDS; return TOK_DEBUG; }
"SHUTDOWN"			{ STORE_BOUNDS; return TOK_SHUTDOWN; }
"CRASH"				{ STORE_BOUNDS; return TOK_CRASH; }
"TOKEN"				{ STORE_BOUNDS; return TOK_TOKEN; }
"MALLOC_STATS"		{ STORE_BOUNDS; return TOK_MALSTATS; }
"MALLOC_TRIM"		{ STORE_BOUNDS; return TOK_MALTRIM; }
"PROCDUMP"			{ STORE_BOUNDS; return TOK_PROCDUMP; }
"SETGDB"	    	{ STORE_BOUNDS; return TOK_SETGDB; }
"SLEEP"				{ STORE_BOUNDS; return TOK_SLEEP; }
"TASKS"				{ STORE_BOUNDS; return TOK_TASKS; }
"SYSTHREADS"		{ STORE_BOUNDS; return TOK_SYSTHREADS; }
"SCHED"			    { STORE_BOUNDS; return TOK_SCHED; }
"MERGE"				{ STORE_BOUNDS; return TOK_MERGE; }
"DROP"				{ STORE_BOUNDS; return TOK_DROP; }
"FILES"				{ STORE_BOUNDS; return TOK_FILES; }
"OPTION"			{ STORE_BOUNDS; return TOK_OPTION; }

"ON"				{ STORE_BOUNDS; return TOK_ON; }
"OFF"				{ STORE_BOUNDS; return TOK_OFF; }
"STATUS"			{ STORE_BOUNDS; return TOK_STATUS; }
"CHUNK"			    { STORE_BOUNDS; return TOK_CHUNK; }
"INTO"			    { STORE_BOUNDS; return TOK_INTO; }
"FROM"			    { STORE_BOUNDS; return TOK_FROM; }


'	                { STORE_START; BEGIN (QSTR); }
<QSTR>[^'\n\\]+
<QSTR>'             {  STORE_END; BEGIN(INITIAL); return TOK_QUOTED_STRING; }

{FLOAT_CONSTANT}	{  lvalp->fValue = (float)strtod ( yytext, NULL ); return TOK_CONST_FLOAT; }
{DIGIT}+			{  lvalp->iValue = (int64_t)strtoull ( yytext, NULL, 10 ); return TOK_CONST_INT; }
{DIGIT}+"US"		{  lvalp->iValue = (int64_t)strtoull ( yytext, NULL, 10 ); return TOK_USEC_INT; }
{DIGIT}+"MS"		{  lvalp->iValue = (int64_t)strtoull ( yytext, NULL, 10 )*1000; return TOK_USEC_INT; }
{DIGIT}+"S"		{  lvalp->iValue = (int64_t)strtoull ( yytext, NULL, 10 )*1000000; return TOK_USEC_INT; }
{DIGIT}+"M"		{  lvalp->iValue = (int64_t)strtoull ( yytext, NULL, 10 )*60000000; return TOK_USEC_INT; }
{DIGIT}+"H"		{  lvalp->iValue = (int64_t)strtoull ( yytext, NULL, 10 )*3600000000; return TOK_USEC_INT; }
{DIGIT}+"D"		{  lvalp->iValue = (int64_t)strtoull ( yytext, NULL, 10 )*3600000000*24; return TOK_USEC_INT; }
{DIGIT}+"W"		{  lvalp->iValue = (int64_t)strtoull ( yytext, NULL, 10 )*3600000000*24-7; return TOK_USEC_INT; }

{ID}				{ STORE_BOUNDS; return TOK_IDENT; }
@{ID}				{ STORE_BOUNDS; return TOK_USERVAR; }
@@{ID}				{ STORE_BOUNDS; return TOK_SYSVAR; }
@@{ID}\.{ID}		{ STORE_BOUNDS; return TOK_SYSVAR; }
`{ID}`				{  lvalp->sValue.m_iStart = yytext - pParser->m_pBuf + 1;
                       		lvalp->sValue.m_iLen = yyleng-2; return TOK_IDENT; }
{ID}:{ID}			{ STORE_BOUNDS; return TOK_IDENT; }


{SPACE}+
.					{  return yytext[0]; }

%%


#if USE_WINDOWS
#pragma warning(pop)
#endif
