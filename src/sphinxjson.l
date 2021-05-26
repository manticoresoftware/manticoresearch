%{
#include "bissphinxjson.h"

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
		lvalp->m_sValue.m_iStart = yytext - pParser->m_pSource; \
		lvalp->m_sValue.m_iLen = yyleng; \
		pParser->m_pLastToken = yytext; \
	}

#define STRING_BEGIN \
	{ \
		lvalp->m_sValue.m_iStart = yytext - pParser->m_pSource; \
	}

#define STRING_END \
	{ \
		lvalp->m_sValue.m_iLen = yytext - pParser->m_pSource + yyleng - lvalp->m_sValue.m_iStart ; \
		pParser->m_pLastToken = pParser->m_pSource + lvalp->m_sValue.m_iStart; \
	}

%}

ID					[a-zA-Z_][a-zA-Z_0-9]*
SPACE				[ \t\n\r]
DIGIT				[0-9]
SIGN                [+-]?
EXPONENT			[eE]{SIGN}{DIGIT}+
I                   {DIGIT}+
FLOAT_CONSTANT      ({I}\.{I}?|{I}?\.{I}){EXPONENT}?
EXP_CONSTANT        {I}{EXPONENT}

%option prefix="yy2"
%option noyywrap
%option nounput
%option reentrant
%option case-insensitive
%option batch
%option align
%option 8bit
%option full
%option ecs
%option warn
%option perf-report

%x ccomment
%x QSTR
%x DQSTR

%%

"//"         		{ BEGIN(ccomment); }
"#"         		{ BEGIN(ccomment); }
<ccomment>[^\n]+
<ccomment>\n		{ BEGIN(INITIAL); }


'	                { STRING_BEGIN; BEGIN (QSTR); }
<QSTR>[^'\n\\]+
<QSTR>'             { STRING_END; BEGIN(INITIAL); lvalp->m_eType = JSON_STRING; return TOK_STRING; }

					/* string in double quotes */
\"	                { STRING_BEGIN; BEGIN (DQSTR); }
<DQSTR>[^\"\n\\]+
<DQSTR>\"			{ STRING_END; BEGIN(INITIAL); lvalp->m_eType = JSON_STRING; return TOK_STRING; }

					/* string process escaped char and unexpected EOF */
<QSTR,DQSTR>\\.
<QSTR,DQSTR><<EOF>>

{SIGN}{I}						{ lvalp->m_eType = JSON_INT64; lvalp->m_iValue = strtoll ( yytext, NULL, 10 ); return TOK_INT; }

{SIGN}{I}\.{EXPONENT}?			|
{SIGN}{I}\.{I}					|
{SIGN}{I}{EXPONENT}				|
{SIGN}{I}\.{I}{EXPONENT}?		{ lvalp->m_eType = JSON_DOUBLE; lvalp->m_fValue = strtod ( yytext, NULL ); return TOK_FLOAT; }

{SIGN}
{SIGN}{I}\.[eE]
{SIGN}{I}\.[eE]{SIGN}
{SIGN}{I}(\.{I})?[eE]
{SIGN}{I}(\.{I})?[eE]{SIGN}

"true"					{ lvalp->m_eType = JSON_TRUE; return TOK_TRUE; }
"false"					{ lvalp->m_eType = JSON_FALSE; return TOK_FALSE; }
"null"					{ lvalp->m_eType = JSON_NULL; return TOK_NULL; }
{ID}					{ STORE_BOUNDS; lvalp->m_eType = JSON_STRING; return TOK_IDENT; }

{SPACE}+				{ ; }
.			            { return yytext[0]; }

%%

// additional options instead of 'fast':
//%option full
//%option ecs
//%option warn
//%option perf-report


// warning, lexer generator dependent!
// flex inserts trailing zero as needed into the buffer when lexing
// but we need that rolled back when doing error reporting from yyerror
void yy2lex_unhold ( yyscan_t yyscanner )
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