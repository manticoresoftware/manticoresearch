%{

#if _WIN32
    #define YY_NO_UNISTD_H 1
    #pragma warning(push,1)
#endif

#ifdef __GNUC__
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wpragmas"
	#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
#endif

#define YY_NO_INPUT

%}

%option nodefault
%option noyywrap
%option nounput
%option reentrant
%option case-insensitive
%option batch
%option prefix="val"
%option align

SPACE				[ \t\n\r]
DIGIT				[0-9]
EXPONENT			[eE][+-]?{DIGIT}+
INT                 [+-]?{DIGIT}+
FLOAT               [+-]?({INT}\.{INT}?|{INT}?\.{INT}){EXPONENT}?

%%

,
{SPACE}+
{INT}
{FLOAT}
"nan"
"inf"
"-inf"
.   			{ pParser->UnexpectedSymbol(yytext); return 0; }

%%

#if _WIN32
    #pragma warning(pop)
#endif

#ifdef __GNUC__
	#pragma GCC diagnostic pop
#endif