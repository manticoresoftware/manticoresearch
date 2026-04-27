%{

#if _WIN32
#define YY_NO_UNISTD_H 1
#endif

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
	}
%}

%option noyywrap
%option nounput
%option reentrant
%option case-insensitive
%option batch
%option prefix="yy7"
%option align

ID					@*[a-zA-Z_][a-zA-Z_0-9]*
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
<ccomment>"*/"		{ BEGIN(INITIAL); }
<ccomment>[^*\n]+
<ccomment>"*"[^/]
<ccomment>"\n"

^"--"         		{ BEGIN(acomment); }
<acomment>"\n"		{ BEGIN(INITIAL); }
<acomment><<EOF>>   { BEGIN(INITIAL); }
<acomment>[^*\n]+

'	                { BEGIN (QSTR); }
<QSTR>[^'\n\\]+
<QSTR>'             {  BEGIN(INITIAL); }



{FLOAT_CONSTANT}
{INT}

{ID}
{ID}\.{ID}
`{ID}`
`{DIGIT}+{ID}`
`{ID}:{ID}`
`{ID}\.{ID}`
`{ID}:{ID}\.{ID}`

"!="
"<>"
"<="
">="
[(){}+\-*/<>&|%=,]
;   			{ pParser->UnexpectedDelim(yytext); }


"?"                 { STORE_BOUNDS; lvalp->m_eType = EPARAM_TYPE::SINGLE; pParser->AddParam(*lvalp); }
"?VEC?"             { STORE_BOUNDS; lvalp->m_eType = EPARAM_TYPE::VECTOR; pParser->AddParam(*lvalp); }

{SPACE}+
.					{  return yytext[0]; }

%%

#if _WIN32
#pragma warning(pop)
#endif

#ifdef __GNUC__
	#pragma GCC diagnostic pop
#endif