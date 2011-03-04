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
%token SEL_AVG
%token SEL_MAX
%token SEL_MIN
%token SEL_SUM
%token SEL_COUNT
%token SEL_WEIGHT
%token SEL_DISTINCT
%token SEL_MATCH_WEIGHT

%token TOK_NEG
%token TOK_LTE
%token TOK_GTE
%token TOK_EQ
%token TOK_NE

%left TOK_AND TOK_OR
%left TOK_EQ TOK_NE
%left '<' '>' TOK_LTE TOK_GTE
%left '+' '-'
%left '*' '/'
%nonassoc TOK_NEG
%nonassoc TOK_NOT

%%

select_list:
	select_item
	| select_list ',' select_item
	;

select_item:
	expr						{ pParser->AddItem ( &$1, NULL ); }
	| expr opt_as SEL_TOKEN		{ pParser->AddItem ( &$1, &$3 ); }
	| '*'						{ pParser->AddItem ( &$1, NULL ); }
	| SEL_AVG '(' expr ')' opt_as SEL_TOKEN		{ pParser->AddItem ( &$3, &$6, SPH_AGGR_AVG ); }
	| SEL_MAX '(' expr ')' opt_as SEL_TOKEN		{ pParser->AddItem ( &$3, &$6, SPH_AGGR_MAX ); }
	| SEL_MIN '(' expr ')' opt_as SEL_TOKEN		{ pParser->AddItem ( &$3, &$6, SPH_AGGR_MIN ); }
	| SEL_SUM '(' expr ')' opt_as SEL_TOKEN		{ pParser->AddItem ( &$3, &$6, SPH_AGGR_SUM ); }
	;

expr:
	SEL_TOKEN
	| SEL_COUNT '(' '*' ')'			{ $$ = $1; $$.m_iEnd = $4.m_iEnd; }
	| SEL_COUNT '(' SEL_DISTINCT SEL_TOKEN ')' { $$ = $1; $$.m_iEnd = $5.m_iEnd; }
	| SEL_WEIGHT '(' ')'			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| SEL_MATCH_WEIGHT '(' ')'		{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| '-' expr %prec TOK_NEG		{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	| TOK_NOT expr				{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
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
	| expr TOK_AND expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_OR expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| '(' expr ')'				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| function
	;

function:
	SEL_TOKEN '(' arglist ')'	{ $$ = $1; $$.m_iEnd = $4.m_iEnd; }
	| SEL_TOKEN '(' ')'			{ $$ = $1; $$.m_iEnd = $3.m_iEnd }
	| SEL_MIN '(' expr ',' expr ')'		{ $$ = $1; $$.m_iEnd = $6.m_iEnd }	// handle clash with aggregate functions
	| SEL_MAX '(' expr ',' expr ')'		{ $$ = $1; $$.m_iEnd = $6.m_iEnd }
	;

arglist:
	expr
	| arglist ',' expr
	;

opt_as:
	// empty
	| SEL_AS
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
