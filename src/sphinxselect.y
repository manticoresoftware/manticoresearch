%{
#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif
%}

%lex-param		{ SelectParser_t * pParser }
%parse-param	{ SelectParser_t * pParser }
%pure-parser
%error-verbose

%token SEL_TOKEN
%token SEL_AS

%token TOK_NEG
%token TOK_LTE
%token TOK_GTE
%token TOK_EQ
%token TOK_NE

%left TOK_EQ TOK_NE
%left '<' '>' TOK_LTE TOK_GTE
%left '+' '-'
%left '*' '/'
%nonassoc TOK_NEG

%%

select_list:
	select_item
	| select_list ',' select_item
	;

select_item:
	expr						{ pParser->AddItem ( &$1, NULL ); }
	| expr SEL_AS SEL_TOKEN		{ pParser->AddItem ( &$1, &$3 ); }
	| '*'						{ pParser->AddItem ( &$1, NULL ); }
	;

expr:
	SEL_TOKEN
	| '-' expr %prec TOK_NEG	{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	| expr '+' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '-' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '*' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '/' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '<' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '>' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_LTE expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_GTE expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_EQ expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_NE expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| '(' expr ')'				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| function
	;

function:
	SEL_TOKEN '(' arglist ')'	{ $$ = $1; $$.m_iEnd = $4.m_iEnd; }
	| SEL_TOKEN '(' ')'			{ $$ = $1; $$.m_iEnd = $3.m_iEnd }
	;

arglist:
	expr
	| arglist ',' expr
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
