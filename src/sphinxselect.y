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
%token SEL_ID
%token SEL_AS
%token SEL_AVG
%token SEL_MAX
%token SEL_MIN
%token SEL_SUM
%token SEL_COUNT
%token SEL_WEIGHT
%token SEL_DISTINCT
%token SEL_OPTION
%token SEL_COMMENT_OPEN
%token SEL_COMMENT_CLOSE

%token TOK_DIV
%token TOK_MOD
%token TOK_NEG
%token TOK_LTE
%token TOK_GTE
%token TOK_EQ
%token TOK_NE
%token TOK_CONST_STRING

%left TOK_OR
%left TOK_AND
%left '|'
%left '&'
%left TOK_EQ TOK_NE
%left '<' '>' TOK_LTE TOK_GTE
%left '+' '-'
%left '*' '/' TOK_DIV '%' TOK_MOD
%nonassoc TOK_NEG
%nonassoc TOK_NOT

%%

select:
	select_list
	| select_list comment
	| comment
	;

select_list:
	select_item
	| select_list ',' select_item
	;

select_item:
	'*'				{ pParser->AddItem ( &$1 ); }
	| select_expr opt_alias

opt_alias:
	// empty				
	| SEL_TOKEN			{ pParser->AliasLastItem ( &$1 ); }
	| SEL_AS SEL_TOKEN		{ pParser->AliasLastItem ( &$2 ); }
	;

select_expr:
	expr				{ pParser->AddItem ( &$1 ); }
	| SEL_AVG '(' expr ')' 		{ pParser->AddItem ( &$3, SPH_AGGR_AVG, &$1, &$4 ); }
	| SEL_MAX '(' expr ')' 		{ pParser->AddItem ( &$3, SPH_AGGR_MAX, &$1, &$4 ); }
	| SEL_MIN '(' expr ')' 		{ pParser->AddItem ( &$3, SPH_AGGR_MIN, &$1, &$4 ); }
	| SEL_SUM '(' expr ')' 		{ pParser->AddItem ( &$3, SPH_AGGR_SUM, &$1, &$4 ); }
	| SEL_COUNT '(' '*' ')' 	{ pParser->AddItem ( "count(*)", &$1, &$4 ); }
	| SEL_WEIGHT '(' ')' 		{ pParser->AddItem ( "weight()", &$1, &$3 ); }
	| SEL_COUNT '(' SEL_DISTINCT SEL_TOKEN ')' 
		// FIXME: may be check if $4 == this->m_sGroupDistinct and warn/error, if not?
					{ pParser->AddItem ( "@distinct", &$1, &$5 ); }
	;

expr:
	select_atom
	| '-' expr %prec TOK_NEG		{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	| TOK_NOT expr				{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	| expr '+' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '-' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '*' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '/' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '<' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '>' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '&' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '|' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '%' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_DIV expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_MOD expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_LTE expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_GTE expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_EQ expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_NE expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_AND expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_OR expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| '(' expr ')'				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| function
	| json_field
	;

select_atom:
	SEL_ID
	| SEL_TOKEN
	;

function:
	SEL_TOKEN '(' arglist ')'		{ $$ = $1; $$.m_iEnd = $4.m_iEnd; }
	| SEL_TOKEN '(' ')'			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| SEL_MIN '(' expr ',' expr ')'		{ $$ = $1; $$.m_iEnd = $6.m_iEnd; }	// handle clash with aggregate functions
	| SEL_MAX '(' expr ',' expr ')'		{ $$ = $1; $$.m_iEnd = $6.m_iEnd; }
	;

arglist:
	arg
	| arglist ',' arg
	;

consthash:
	SEL_TOKEN TOK_EQ SEL_TOKEN
	| consthash ',' SEL_TOKEN TOK_EQ SEL_TOKEN
	;	
	
arg:
	expr
	| '{' consthash '}'
	| TOK_CONST_STRING
	;

comment:
	SEL_COMMENT_OPEN SEL_OPTION SEL_TOKEN TOK_EQ SEL_TOKEN SEL_COMMENT_CLOSE
		{
			pParser->AddOption ( &$3, &$5 );
		}
	;

json_field:
	SEL_TOKEN subscript			{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }

subscript:
	subkey
	| subscript subkey			{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	;

subkey:
	'.' SEL_TOKEN				{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	| '[' expr ']'				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| '[' TOK_CONST_STRING ']'	{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
