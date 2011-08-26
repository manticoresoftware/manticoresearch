%{
#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif
%}

%lex-param	{ SqlParser_c * pParser }
%parse-param	{ SqlParser_c * pParser }
%pure-parser
%error-verbose

%token	TOK_IDENT
%token	TOK_ATIDENT
%token	TOK_CONST_INT
%token	TOK_CONST_FLOAT
%token	TOK_CONST_MVA
%token	TOK_QUOTED_STRING
%token	TOK_USERVAR
%token	TOK_SYSVAR

%token	TOK_AS
%token	TOK_ASC
%token	TOK_AVG
%token	TOK_BEGIN
%token	TOK_BETWEEN
%token	TOK_BY
%token	TOK_CALL
%token	TOK_COLLATION
%token	TOK_COMMIT
%token	TOK_COUNT
%token	TOK_CREATE
%token	TOK_DELETE
%token	TOK_DESC
%token	TOK_DESCRIBE
%token	TOK_DISTINCT
%token	TOK_DIV
%token	TOK_DROP
%token	TOK_FALSE
%token	TOK_FLOAT
%token	TOK_FROM
%token	TOK_FUNCTION
%token	TOK_GLOBAL
%token	TOK_GROUP
%token	TOK_ID
%token	TOK_IN
%token	TOK_INSERT
%token	TOK_INT
%token	TOK_INTO
%token	TOK_LIMIT
%token	TOK_MATCH
%token	TOK_MAX
%token	TOK_META
%token	TOK_MIN
%token	TOK_MOD
%token	TOK_NAMES
%token	TOK_NULL
%token	TOK_OPTION
%token	TOK_ORDER
%token	TOK_RAND
%token	TOK_REPLACE
%token	TOK_RETURNS
%token	TOK_ROLLBACK
%token	TOK_SELECT
%token	TOK_SET
%token	TOK_SESSION
%token	TOK_SHOW
%token	TOK_SONAME
%token	TOK_START
%token	TOK_STATUS
%token	TOK_SUM
%token	TOK_TABLES
%token	TOK_TRANSACTION
%token	TOK_TRUE
%token	TOK_UPDATE
%token	TOK_VALUES
%token	TOK_VARIABLES
%token	TOK_WARNINGS
%token	TOK_WEIGHT
%token	TOK_WHERE
%token	TOK_WITHIN

%type	<m_iValue>		named_const_list
%type	<m_iValue>		udf_type

%left TOK_OR
%left TOK_AND
%left '|'
%left '&'
%left '=' TOK_NE
%left '<' '>' TOK_LTE TOK_GTE
%left '+' '-'
%left '*' '/' '%' TOK_DIV TOK_MOD
%nonassoc TOK_NOT
%nonassoc TOK_NEG

%{
// some helpers
#include <float.h> // for FLT_MAX

%}

%%

request:
   statement							{ pParser->PushQuery(); }
   | multi_stmt_list
   | multi_stmt_list ';'
   ;

statement:
	insert_into
	| delete_from
	| set_clause
	| set_global_clause
	| transact_op
	| call_proc
	| describe
	| show_tables
	| update
	| show_variables
	| show_collation
	| create_function
	| drop_function
	;

//////////////////////////////////////////////////////////////////////////

multi_stmt_list:
	multi_stmt							{ pParser->PushQuery(); }
	| multi_stmt_list ';' multi_stmt	{ pParser->PushQuery(); }
	;

multi_stmt:
	select_from
	| show_clause
	;

select_from:
	TOK_SELECT select_items_list
	TOK_FROM ident_list
	opt_where_clause
	opt_group_clause
	opt_group_order_clause
	opt_order_clause
	opt_limit_clause
	opt_option_clause
		{
			pParser->m_pStmt->m_eStmt = STMT_SELECT;
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+$4.m_iStart, $4.m_iEnd-$4.m_iStart );
		}
	;
	
select_items_list:
	select_item
	| select_items_list ',' select_item
	;

select_item:
	'*'						{ pParser->AddItem ( &$1 ); }
	| select_expr opt_alias
	;

opt_alias:
	// empty				
	| TOK_IDENT					{ pParser->AliasLastItem ( &$1 ); }
	| TOK_AS TOK_IDENT				{ pParser->AliasLastItem ( &$2 ); }
	;

select_expr:
	expr						{ pParser->AddItem ( &$1 ); }
	| TOK_AVG '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_AVG, &$1, &$4 ); }
	| TOK_MAX '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_MAX, &$1, &$4 ); }
	| TOK_MIN '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_MIN, &$1, &$4 ); }
	| TOK_SUM '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_SUM, &$1, &$4 ); }
	| TOK_COUNT '(' '*' ')'				{ if ( !pParser->AddItem ( "count(*)", &$1, &$4 ) ) YYERROR; }
	| TOK_WEIGHT '(' ')'				{ if ( !pParser->AddItem ( "weight()", &$1, &$3 ) ) YYERROR; }
	| TOK_COUNT '(' TOK_DISTINCT TOK_IDENT ')' 	{ if ( !pParser->AddDistinct ( &$4, &$1, &$5 ) ) YYERROR; }
	;

ident_list:
	TOK_IDENT
	| ident_list ',' TOK_IDENT			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	;

opt_where_clause:
	// empty
	| where_clause
	;

where_clause:
	TOK_WHERE where_expr
	;

where_expr:
	where_item
	| where_expr TOK_AND where_item 
	;

where_item:
	TOK_MATCH '(' TOK_QUOTED_STRING ')'
		{
			if ( !pParser->SetMatch($3) )
				YYERROR;
		}
	| expr_ident '=' const_int
		{
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( $1 );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( $3.m_iValue );
		}
	| expr_ident TOK_NE const_int
		{
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( $1 );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( $3.m_iValue );
			pFilter->m_bExclude = true;
		}
	| expr_ident TOK_IN '(' const_list ')'
		{
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( $1 );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *$4.m_pValues.Ptr();
			pFilter->m_dValues.Sort();
		}
	| expr_ident TOK_NOT TOK_IN '(' const_list ')'
		{
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( $1 );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues = *$5.m_pValues.Ptr();
			pFilter->m_bExclude = true;
			pFilter->m_dValues.Sort();
		}
	| expr_ident TOK_IN TOK_USERVAR
		{
			if ( !pParser->AddUservarFilter ( $1.m_sValue, $3.m_sValue, false ) )
				YYERROR;
		}
	| expr_ident TOK_NOT TOK_IN TOK_USERVAR
		{
			if ( !pParser->AddUservarFilter ( $1.m_sValue, $3.m_sValue, true ) )
				YYERROR;
		}
	| expr_ident TOK_BETWEEN const_int TOK_AND const_int
		{
			if ( !pParser->AddUintRangeFilter ( $1.m_sValue, $3.m_iValue, $5.m_iValue ) )
				YYERROR;
		}
	| expr_ident '>' const_int
		{
			if ( !pParser->AddUintRangeFilter ( $1.m_sValue, $3.m_iValue+1, UINT_MAX ) )
				YYERROR;
		}
	| expr_ident '<' const_int
		{
			if ( !pParser->AddUintRangeFilter ( $1.m_sValue, 0, $3.m_iValue-1 ) )
				YYERROR;
		}
	| expr_ident TOK_GTE const_int
		{
			if ( !pParser->AddUintRangeFilter ( $1.m_sValue, $3.m_iValue, UINT_MAX ) )
				YYERROR;
		}
	| expr_ident TOK_LTE const_int
		{
			if ( !pParser->AddUintRangeFilter ( $1.m_sValue, 0, $3.m_iValue ) )
				YYERROR;
		}
	| expr_ident '=' const_float
	| expr_ident TOK_NE const_float
	| expr_ident '>' const_float
	| expr_ident '<' const_float
		{
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		}
	| expr_ident TOK_BETWEEN const_float TOK_AND const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1.m_sValue, $3.m_fValue, $5.m_fValue ) )
				YYERROR;
		}
	| expr_ident TOK_GTE const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1.m_sValue, $3.m_fValue, FLT_MAX ) )
				YYERROR;
		}
	| expr_ident TOK_LTE const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1.m_sValue, -FLT_MAX, $3.m_fValue ) )
				YYERROR;
		}
	;

expr_ident:
	TOK_IDENT
	| TOK_ATIDENT
		{
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		}
	| TOK_COUNT '(' '*' ')'
		{
			$$.m_sValue = "@count";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}
	| TOK_WEIGHT '(' ')'
		{
			$$.m_sValue = "@weight";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}
	| TOK_ID
		{
			$$.m_sValue = "@id";
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}
	;

const_int:
	TOK_CONST_INT			{ $$.m_iInstype = TOK_CONST_INT; $$.m_iValue = $1.m_iValue; }
	| '-' TOK_CONST_INT		{ $$.m_iInstype = TOK_CONST_INT; $$.m_iValue = -$2.m_iValue; }
	;

const_float:
	TOK_CONST_FLOAT			{ $$.m_iInstype = TOK_CONST_FLOAT; $$.m_fValue = $1.m_fValue; }
	| '-' TOK_CONST_FLOAT	{ $$.m_iInstype = TOK_CONST_FLOAT; $$.m_fValue = -$2.m_fValue; }
	;

const_list:
	const_int
		{
			assert ( !$$.m_pValues.Ptr() );
			$$.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			$$.m_pValues->Add ( $1.m_iValue ); 
		}
	| const_list ',' const_int
		{
			$$.m_pValues->Add ( $3.m_iValue );
		}
	;

opt_group_clause:
	// empty
	| group_clause
	;

group_clause:
	TOK_GROUP TOK_BY expr_ident
		{
			pParser->m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
			pParser->m_pQuery->m_sGroupBy = $3.m_sValue;
		}
	;

opt_group_order_clause:
	// empty
	| group_order_clause
	;

group_order_clause:
	TOK_WITHIN TOK_GROUP TOK_ORDER TOK_BY order_items_list
		{
			pParser->m_pQuery->m_sSortBy.SetBinary ( pParser->m_pBuf+$5.m_iStart, $5.m_iEnd-$5.m_iStart );
		}
	;

opt_order_clause:
	// empty
	| order_clause
	;

order_clause:
	TOK_ORDER TOK_BY order_items_list
		{
			pParser->m_pQuery->m_sOrderBy.SetBinary ( pParser->m_pBuf+$3.m_iStart, $3.m_iEnd-$3.m_iStart );
		}
	| TOK_ORDER TOK_BY TOK_RAND '(' ')'
		{
			pParser->m_pQuery->m_sOrderBy = "@random";
		}
	;

order_items_list:
	order_item
	| order_items_list ',' order_item	{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	;

order_item:
	expr_ident
	| expr_ident TOK_ASC				{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	| expr_ident TOK_DESC				{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	;

opt_limit_clause:
	// empty
	| limit_clause
	;

limit_clause:
	TOK_LIMIT TOK_CONST_INT
		{
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = $2.m_iValue;
		}
	| TOK_LIMIT TOK_CONST_INT ',' TOK_CONST_INT
		{
			pParser->m_pQuery->m_iOffset = $2.m_iValue;
			pParser->m_pQuery->m_iLimit = $4.m_iValue;
		}
	;

opt_option_clause:
	// empty
	| option_clause
	;

option_clause:
	TOK_OPTION option_list
	;

option_list:
	option_item
	| option_list ',' option_item
	;

option_item:
	TOK_IDENT '=' TOK_IDENT
		{
			if ( !pParser->AddOption ( $1, $3 ) )
				YYERROR;
		}
	| TOK_IDENT '=' TOK_CONST_INT
		{
			if ( !pParser->AddOption ( $1, $3 ) )
				YYERROR;
		}
	| TOK_IDENT '=' '(' named_const_list ')'
		{
			if ( !pParser->AddOption ( $1, pParser->GetNamedVec ( $4 ) ) )
				YYERROR;
			pParser->FreeNamedVec ( $4 );
		}
	| TOK_IDENT '=' TOK_IDENT '(' TOK_QUOTED_STRING ')'
		{
			if ( !pParser->AddOption ( $1, $4, $5.m_sValue ) )
				YYERROR;
		}
	;

named_const_list:
	named_const
		{
			$$ = pParser->AllocNamedVec ();
			pParser->AddConst ( $$, $1 );
		}
	| named_const_list ',' named_const
		{
			pParser->AddConst( $$, $3 );
		}
	;

named_const:
	TOK_IDENT '=' const_int
		{
			$$.m_sValue = $1.m_sValue;
			$$.m_iValue = $3.m_iValue;
		}
	;

//////////////////////////////////////////////////////////////////////////

expr:
	TOK_IDENT
	| TOK_ATIDENT	{ if ( !pParser->SetOldSyntax() ) YYERROR; }
	| TOK_ID	{ if ( !pParser->SetNewSyntax() ) YYERROR; }
	| TOK_CONST_INT
	| TOK_CONST_FLOAT
	| TOK_USERVAR
	| '-' expr %prec TOK_NEG	{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
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
	| expr '=' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_NE expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_AND expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr TOK_OR expr			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| '(' expr ')'				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| function
	;

function:
	TOK_IDENT '(' arglist ')'	{ $$ = $1; $$.m_iEnd = $4.m_iEnd; }
	| TOK_IN '(' arglist ')'	{ $$ = $1; $$.m_iEnd = $4.m_iEnd; }			// handle exception from 'ident' rule
	| TOK_IDENT '(' ')'			{ $$ = $1; $$.m_iEnd = $3.m_iEnd }
	| TOK_MIN '(' expr ',' expr ')'		{ $$ = $1; $$.m_iEnd = $6.m_iEnd }	// handle clash with aggregate functions
	| TOK_MAX '(' expr ',' expr ')'		{ $$ = $1; $$.m_iEnd = $6.m_iEnd }
	;

arglist:
	arg
	| arglist ',' arg
	;

arg:
	expr
	| TOK_QUOTED_STRING
	;

//////////////////////////////////////////////////////////////////////////

show_clause:
	TOK_SHOW show_variable
	;

show_variable:
	TOK_WARNINGS		{ pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; }
	| TOK_STATUS		{ pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; }
	| TOK_META			{ pParser->m_pStmt->m_eStmt = STMT_SHOW_META; }
	;

//////////////////////////////////////////////////////////////////////////

set_value:
	TOK_IDENT
	| TOK_NULL
	| TOK_QUOTED_STRING
	| TOK_CONST_INT
	| TOK_CONST_FLOAT
	;

set_clause:
	TOK_SET TOK_IDENT '=' boolean_value
		{
			pParser->SetStatement ( $2, SET_LOCAL );
			pParser->m_pStmt->m_iSetValue = $4.m_iValue;
		}
	| TOK_SET TOK_IDENT '=' set_string_value
		{
			pParser->SetStatement ( $2, SET_LOCAL );
			pParser->m_pStmt->m_sSetValue = $4.m_sValue;
		}
	| TOK_SET TOK_IDENT '=' TOK_NULL
		{
			pParser->SetStatement ( $2, SET_LOCAL );
			pParser->m_pStmt->m_bSetNull = true;
		}
	| TOK_SET TOK_NAMES set_value		{ pParser->m_pStmt->m_eStmt = STMT_DUMMY; }
	| TOK_SET TOK_SYSVAR '=' set_value	{ pParser->m_pStmt->m_eStmt = STMT_DUMMY; }
	;

set_global_clause:
	TOK_SET TOK_GLOBAL TOK_USERVAR '=' '(' const_list ')'
		{
			pParser->SetStatement ( $3, SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *$6.m_pValues.Ptr();
		}
	| TOK_SET TOK_GLOBAL TOK_IDENT '=' set_string_value
		{
			pParser->SetStatement ( $3, SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_sSetValue = $5.m_sValue;
		}
	;

set_string_value:
	TOK_IDENT
	| TOK_QUOTED_STRING
	;

boolean_value:
	TOK_TRUE			{ $$.m_iValue = 1; }
	| TOK_FALSE			{ $$.m_iValue = 0; }
	| const_int
		{
			$$.m_iValue = $1.m_iValue;
			if ( $$.m_iValue!=0 && $$.m_iValue!=1 )
			{
				yyerror ( pParser, "only 0 and 1 could be used as boolean values" );
				YYERROR;
			}
		}
	;

//////////////////////////////////////////////////////////////////////////

transact_op:
	TOK_COMMIT			{ pParser->m_pStmt->m_eStmt = STMT_COMMIT; }
	| TOK_ROLLBACK			{ pParser->m_pStmt->m_eStmt = STMT_ROLLBACK; }
	| start_transaction		{ pParser->m_pStmt->m_eStmt = STMT_BEGIN; }
	;

start_transaction:
	TOK_BEGIN
	| TOK_START TOK_TRANSACTION
	;

//////////////////////////////////////////////////////////////////////////

insert_into:
	insert_or_replace TOK_INTO TOK_IDENT opt_column_list TOK_VALUES insert_rows_list
		{
			// everything else is pushed directly into parser within the rules
			pParser->m_pStmt->m_sIndex = $3.m_sValue;
		}
	;

insert_or_replace:
	TOK_INSERT		{ pParser->m_pStmt->m_eStmt = STMT_INSERT; }
	| TOK_REPLACE	{ pParser->m_pStmt->m_eStmt = STMT_REPLACE; }
	;

opt_column_list:
	// empty
	| '(' column_list ')'
	;

column_list:
	expr_ident							{ if ( !pParser->AddSchemaItem ( &$1 ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }
	| column_list ',' expr_ident		{ if ( !pParser->AddSchemaItem ( &$3 ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }
	;

insert_rows_list:
	insert_row
	| insert_rows_list ',' insert_row
	;

insert_row:
	'(' insert_vals_list ')'			{ if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } }
	;

insert_vals_list:
	insert_val							{ AddInsval ( pParser->m_pStmt->m_dInsertValues, $1 ); }
	| insert_vals_list ',' insert_val	{ AddInsval ( pParser->m_pStmt->m_dInsertValues, $3 ); }
	;

insert_val:
	const_int				{ $$.m_iInstype = TOK_CONST_INT; $$.m_iValue = $1.m_iValue; }
	| const_float			{ $$.m_iInstype = TOK_CONST_FLOAT; $$.m_fValue = $1.m_fValue; }
	| TOK_QUOTED_STRING		{ $$.m_iInstype = TOK_QUOTED_STRING; $$.m_sValue = $1.m_sValue; }
	| '(' const_list ')'	{ $$.m_iInstype = TOK_CONST_MVA; $$.m_iValue = $2.m_pValues->GetLength(); $$.m_pValues = $2.m_pValues; }
	;

//////////////////////////////////////////////////////////////////////////

delete_from:
	TOK_DELETE TOK_FROM TOK_IDENT TOK_WHERE TOK_ID '=' const_int
		{
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = $3.m_sValue;
			pParser->m_pStmt->m_dDeleteIds.Add ( $7.m_iValue );
		}
	| TOK_DELETE TOK_FROM TOK_IDENT TOK_WHERE TOK_ID TOK_IN '(' const_list ')'
		{
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = $3.m_sValue;
			for ( int i=0; i<$8.m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*$8.m_pValues.Ptr())[i] );
		}
	;

//////////////////////////////////////////////////////////////////////////

call_proc:
	TOK_CALL TOK_IDENT '(' insert_vals_list opt_call_opts_list ')'
		{
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = $2.m_sValue;
		}
	;

opt_call_opts_list:
	// empty
	| ',' call_opts_list
	;

call_opts_list:
	call_opt
		{
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		}
	| call_opts_list ',' call_opt
	;

call_opt:
	insert_val opt_as call_opt_name
		{
			pParser->m_pStmt->m_dCallOptNames.Add ( $3.m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, $1 );
		}
	;

opt_as:
	// empty
	| TOK_AS
	;

call_opt_name:
	TOK_IDENT
	| TOK_LIMIT		{ $$.m_sValue = "limit"; }
	;

//////////////////////////////////////////////////////////////////////////

describe:
	describe_tok TOK_IDENT
		{
			pParser->m_pStmt->m_eStmt = STMT_DESC;
			pParser->m_pStmt->m_sIndex = $2.m_sValue;
		}
	;

describe_tok:
	TOK_DESCRIBE
	| TOK_DESC
	;

//////////////////////////////////////////////////////////////////////////

show_tables:
	TOK_SHOW TOK_TABLES		{ pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; }
	;

//////////////////////////////////////////////////////////////////////////

update:
	TOK_UPDATE ident_list TOK_SET update_items_list where_clause
		{
			if ( !pParser->UpdateStatement ( &$2 ) )
				YYERROR;
		}
	;

update_items_list:
	update_item
	| update_items_list ',' update_item
	;

update_item:
	TOK_IDENT '=' const_int
		{
			pParser->UpdateAttr ( $1.m_sValue, (DWORD) $3.m_iValue );
		}
	| TOK_IDENT '='  '(' const_list ')'
		{
			pParser->UpdateMVAAttr ( $1.m_sValue, $4 );
		}
	| TOK_IDENT '='  '(' ')' // special case () means delete mva
		{
			pParser->UpdateAttr ( $1.m_sValue, NULL, SPH_ATTR_UINT32SET );
		}
	;

//////////////////////////////////////////////////////////////////////////

show_variables:
	TOK_SHOW opt_session TOK_VARIABLES		{ pParser->m_pStmt->m_eStmt = STMT_DUMMY; }
	;

opt_session:
	| TOK_SESSION
	;

show_collation:
	TOK_SHOW TOK_COLLATION		{ pParser->m_pStmt->m_eStmt = STMT_DUMMY; }
	;

//////////////////////////////////////////////////////////////////////////

create_function:
	TOK_CREATE TOK_FUNCTION TOK_IDENT TOK_RETURNS udf_type TOK_SONAME TOK_QUOTED_STRING
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNC;
			tStmt.m_sUdfName = $3.m_sValue;
			tStmt.m_sUdfLib = $7.m_sValue;
			tStmt.m_eUdfType = (ESphAttr) $5;
		}
	;

udf_type:
	TOK_INT			{ $$ = SPH_ATTR_INTEGER; }
	| TOK_FLOAT		{ $$ = SPH_ATTR_FLOAT; }
	;

drop_function:
	TOK_DROP TOK_FUNCTION TOK_IDENT
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = $3.m_sValue;
		}
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
