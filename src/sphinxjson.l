%{
#include "yysphinxjson.h"

#if USE_WINDOWS
#pragma warning(push,1)
#endif

#define STORE_BOUNDS \
	{ \
		lvalp->m_iStart = yytext - pParser->m_pBuf; \
		lvalp->m_iEnd = lvalp->m_iStart + yyleng; \
		pParser->m_pLastToken = yytext; \
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

"//"         		{ BEGIN(ccomment); }
"#"         		{ BEGIN(ccomment); }
<ccomment>.			{ }
<ccomment>"\n"		{ BEGIN(INITIAL); }

"true"					{ STORE_BOUNDS; lvalp->m_eType = JSON_TRUE; return TOK_TRUE; }
"false"					{ STORE_BOUNDS; lvalp->m_eType = JSON_FALSE; return TOK_FALSE; }
"null"					{ STORE_BOUNDS; lvalp->m_eType = JSON_NULL; return TOK_NULL; }

'([^'\\]|\\.|\\\\)*'	{ STORE_BOUNDS; lvalp->m_eType = JSON_STRING; return TOK_STRING; }
\"([^"\\]|\\.|\\\\)*\"	{ STORE_BOUNDS; lvalp->m_eType = JSON_STRING; return TOK_STRING; }

-*{DIGIT}*\.{DIGIT}*	{ STORE_BOUNDS; lvalp->m_eType = JSON_DOUBLE; lvalp->m_fValue = strtod ( yytext, NULL ); return TOK_FLOAT; }
-*{DIGIT}+				{ STORE_BOUNDS; lvalp->m_eType = JSON_INT64; lvalp->m_iValue = strtoll ( yytext, NULL, 10 ); return TOK_INT; }
{ID}					{ STORE_BOUNDS; lvalp->m_eType = JSON_STRING; return TOK_IDENT; }

{SPACE}+				{ ; }
.						{ STORE_BOUNDS; return yytext[0]; }

%%

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

#if USE_WINDOWS
#pragma warning(pop)
#endif
