%{
#include "yysphinxql.h"

#pragma warning(disable:4127) // conditional expression is constant
#pragma warning(disable:4244) // conversion from int to short

// warning, lexer generator dependent!
// this macros relies on that in flex yytext points to the actual location in the buffer
#define YYSTOREBOUNDS \
	{ \
		lvalp->m_iStart = yytext - pParser->m_pBuf; \
		lvalp->m_iEnd = lvalp->m_iStart + yyleng; \
		pParser->m_pLastTokenStart = yytext; \
	}

%}

DIGIT				[0-9]
ID					[a-zA-Z_][a-zA-Z_0-9]*
SPACE				[ \t\n]

%option noyywrap
%option nounput

%%

"AS"				{ YYSTOREBOUNDS; return TOK_AS; }
"ASC"				{ YYSTOREBOUNDS; return TOK_ASC; }
"BETWEEN"			{ YYSTOREBOUNDS; return TOK_BETWEEN; }
"BY"				{ YYSTOREBOUNDS; return TOK_BY; }
"DESC"				{ YYSTOREBOUNDS; return TOK_DESC; }
"FROM"				{ YYSTOREBOUNDS; return TOK_FROM; }
"GROUP"				{ YYSTOREBOUNDS; return TOK_GROUP; }
"LIMIT"				{ YYSTOREBOUNDS; return TOK_LIMIT; }
"IN"				{ YYSTOREBOUNDS; return TOK_IN; }
"ID"				{ YYSTOREBOUNDS; return TOK_ID; }
"MATCH"				{ YYSTOREBOUNDS; return TOK_MATCH; }
"ORDER"				{ YYSTOREBOUNDS; return TOK_ORDER; }
"SELECT"			{ YYSTOREBOUNDS; return TOK_SELECT; }
"SHOW"				{ YYSTOREBOUNDS; return TOK_SHOW; }
"WARNINGS"			{ YYSTOREBOUNDS; return TOK_WARNINGS; }
"WEIGHT"			{ YYSTOREBOUNDS; return TOK_WEIGHT; }
"WITHIN"			{ YYSTOREBOUNDS; return TOK_WITHIN; }
"WHERE"				{ YYSTOREBOUNDS; return TOK_WHERE; }

"AND"				{ YYSTOREBOUNDS; return TOK_AND; }
"OR"				{ YYSTOREBOUNDS; return TOK_OR; }
"NOT"				{ YYSTOREBOUNDS; return TOK_NOT; }

"!="				{ YYSTOREBOUNDS; return TOK_NE; }
"<>"				{ YYSTOREBOUNDS; return TOK_NE; }
"<="				{ YYSTOREBOUNDS; return TOK_LTE; }
">="				{ YYSTOREBOUNDS; return TOK_GTE; }

'([^']|\\')*'		{ YYSTOREBOUNDS; SqlUnescape ( lvalp->m_sValue, yytext, yyleng ); return TOK_QUOTED_STRING; }

{DIGIT}+			{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; lvalp->m_iValue = strtoll ( yytext, NULL, 10 ); return TOK_CONST; }

"@id"				{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_IDENT; }
"@weight"			{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_IDENT; }
"@count"			{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_IDENT; }
{ID}				{ YYSTOREBOUNDS; lvalp->m_sValue = yytext; return TOK_IDENT; }

{SPACE}+			{ ; }
.					{ YYSTOREBOUNDS; return yytext[0]; }

%%

#pragma warning(default:4127) // conditional expression is constant
#pragma warning(default:4244) // conversion from int to short
