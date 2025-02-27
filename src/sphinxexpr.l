%{

#define YY_NO_INPUT

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wpragmas"
#endif


#if _WIN32
#define YY_NO_UNISTD_H 1
#pragma warning(push,1)
#endif

#define DOTBOUNDS \
	{ \
		auto iBeg = yytext - pParser->m_sExpr.first; \
		lvalp->iConst = ( int64_t(iBeg)<<32 ) + yyleng; \
	}

#define STRING_BEGIN \
	{ \
	    lvalp->iConst = yytext - pParser->m_sExpr.first; \
	}

#define STRING_END \
	{ \
		auto iBeg = (int)( lvalp->iConst ); \
		int iLen = (int)( yytext - pParser->m_sExpr.first ) - iBeg + yyleng; \
		lvalp->iConst = ( int64_t(iBeg)<<32 ) + iLen; \
	}

 /* %option backup - will add lex.backup to the sources aside sphinxexpr.l with states slowing lexer */

%}

%option noyywrap
%option nounput
%option reentrant
%option case-insensitive
%option batch
%option align
%option prefix="yy1"
%option 8bit
%option full
%option ecs
%option warn
%option perf-report

DIGIT				[0-9]
ALPHA				[a-zA-Z_@]
ALNUM				[a-zA-Z_0-9]
ID					{ALPHA}{ALNUM}*
SPACE				[ \t\n\r]
SIGN                [+-]?
INT                 {DIGIT}+
EXP     			[eE]{SIGN}{INT}

%x QSTR
%x DQSTR
%x DOT
%x BACK
%x DOTBACK

%%

					/* string in single quotes */
'	                { STRING_BEGIN; BEGIN (QSTR); }
<QSTR>[^'\n\\]+
<QSTR>'             { STRING_END; BEGIN(INITIAL); return TOK_CONST_STRING; }

					/* string in double quotes */
\"	                { STRING_BEGIN; BEGIN (DQSTR); }
<DQSTR>[^\"\n\\]+
<DQSTR>\"			{ STRING_END; BEGIN(INITIAL); return TOK_CONST_STRING; }

					/* string process escaped char and unexpected EOF */
<QSTR,DQSTR>\\.
<QSTR,DQSTR><<EOF>>		{ return pParser->ErrLex ( "unterminated string constant near '%s'", pParser->m_sExpr.first + lvalp->iConst ); }


					/* expression started with dot(.). Note! dot itself is NOT part of final token */
\.							{ BEGIN (DOT); }
<DOT>{INT}					{ DOTBOUNDS; BEGIN(INITIAL); return TOK_DOT_NUMBER; }
<DOT>{INT}{EXP}				{ BEGIN(INITIAL); lvalp->fConst = (float)strtod ( yytext-1, NULL ); return TOK_CONST_FLOAT; }
<DOT>{INT}[eE]{SIGN}		{ DOTBOUNDS; BEGIN(INITIAL); return TOK_SUBKEY; }
<DOT>{SPACE}*`				{ BEGIN (DOTBACK); STRING_BEGIN; lvalp->iConst+=yyleng; } // + to skip backtick
<DOT>{SPACE}+
<DOT>{ALNUM}+				{ DOTBOUNDS; BEGIN(INITIAL); return TOK_SUBKEY; }




					/* predefined nums, columns and ops */
{INT}               { lvalp->iConst = (int64_t)strtoull ( yytext, NULL, 10 ); return TOK_CONST_INT; }

{INT}\.{INT}?{EXP}?	|
{INT}{EXP}			{ lvalp->fConst = (float)strtod ( yytext, NULL ); return TOK_CONST_FLOAT; }

					/* backing up rules for skip interrupted semi-float number */
{INT}[eE]{SIGN}			|
{INT}\.{INT}?[eE]{SIGN}	{ return pParser->ErrLex ( "unterminated float number '%s'", yytext ); }

\<\>|\!\=	{ return TOK_NE; }
\<=			{ return TOK_LTE; }
\>=			{ return TOK_GTE; }
==?			{ return TOK_EQ; }

{ALPHA} |
{ALPHA}{ALNUM}+	{ lvalp->iTrailingBr = 0; return pParser->ProcessRawToken ( yytext,yyleng,lvalp ); }

{ALPHA}{ALNUM}*/"("	{ lvalp->iTrailingBr = 1; return pParser->ProcessRawToken ( yytext,yyleng,lvalp ); }
{ALPHA}{ALNUM}*{SPACE}+/"(" { lvalp->iTrailingBr = 2; return pParser->ProcessRawToken ( yytext,yyleng,lvalp ); }


`	                { BEGIN (BACK); STRING_BEGIN; lvalp->iConst+=yyleng; } // ++ to skip backtick
<BACK,DOTBACK>[^\n\\`]+
<BACK,DOTBACK>\\[^\n]
<DOTBACK>`			{
							STRING_END; BEGIN(INITIAL);
							--lvalp->iConst; // -- to skip trailing backtick
							return TOK_SUBKEY;
					}

<BACK>`				{
							BEGIN(INITIAL);
							auto pText = pParser->m_sExpr.first + lvalp->iConst;
                            int iLen =  yytext + yyleng - pText - 1;
                            lvalp->iTrailingBr = 0;
							return pParser->ProcessRawToken (pText,iLen,lvalp);
					}

<BACK,DOTBACK>\n			|	/* terminate on \n or EOF inside unclosed backtick */
<BACK,DOTBACK><<EOF>>	{
							BEGIN(INITIAL);
							return pParser->ErrLex ( "unterminated ID near '%s'", pParser->m_sExpr.first + lvalp->iConst );
						}

{SPACE}+

.			{ return yytext[0]; }

%%

// warning, lexer generator dependent!
// flex inserts trailing zero as needed into the buffer when lexing
// but we need that rolled back when doing error reporting from yyerror
const char* yy1lex_unhold ( yyscan_t yyscanner )
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	if ( YY_CURRENT_BUFFER )
	{
		*yyg->yy_c_buf_p = yyg->yy_hold_char;
		YY_CURRENT_BUFFER_LVALUE->yy_buf_pos = yyg->yy_c_buf_p;
		YY_CURRENT_BUFFER_LVALUE->yy_n_chars = yyg->yy_n_chars;
		return yyg->yy_c_buf_p;
	}
	return nullptr;
}

#if _WIN32
#pragma warning(pop)
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

// %option backup
// %option debug
// %option verbose
