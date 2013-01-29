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
%token	TOK_CONST_STRINGS

%token	TOK_AGENT
%token	TOK_AS
%token	TOK_ASC
%token	TOK_ATTACH
%token	TOK_AVG
%token	TOK_BEGIN
%token	TOK_BETWEEN
%token	TOK_BY
%token	TOK_CALL
%token	TOK_CHARACTER
%token	TOK_COLLATION
%token	TOK_COMMIT
%token	TOK_COMMITTED
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
%token	TOK_FLUSH
%token	TOK_FROM
%token	TOK_FUNCTION
%token	TOK_GLOBAL
%token	TOK_GROUP
%token	TOK_GROUPBY
%token	TOK_GROUP_CONCAT
%token	TOK_ID
%token	TOK_IN
%token	TOK_INDEX
%token	TOK_INSERT
%token	TOK_INT
%token	TOK_INTO
%token	TOK_ISOLATION
%token	TOK_LEVEL
%token	TOK_LIKE
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
%token	TOK_OPTIMIZE
%token	TOK_RAND
%token	TOK_READ
%token	TOK_REPEATABLE
%token	TOK_REPLACE
%token	TOK_RETURNS
%token	TOK_ROLLBACK
%token	TOK_RTINDEX
%token	TOK_SELECT
%token	TOK_SERIALIZABLE
%token	TOK_SET
%token	TOK_SESSION
%token	TOK_SHOW
%token	TOK_SONAME
%token	TOK_START
%token	TOK_STATUS
%token	TOK_STRING
%token	TOK_SUM
%token	TOK_TABLES
%token	TOK_TO
%token	TOK_TRANSACTION
%token	TOK_TRUE
%token	TOK_TRUNCATE
%token	TOK_UNCOMMITTED
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
	| set_stmt
	| set_global_stmt
	| transact_op
	| call_proc
	| describe
	| show_tables
	| update
	| show_variables
	| show_collation
	| show_character_set
	| create_function
	| drop_function
	| attach_index
	| flush_rtindex
	| set_transaction
	| select_sysvar
	| truncate
	| optimize_index
	;

//////////////////////////////////////////////////////////////////////////

multi_stmt_list:
	multi_stmt							{ pParser->PushQuery(); }
	| multi_stmt_list ';' multi_stmt	{ pParser->PushQuery(); }
	;

multi_stmt:
	select
	| show_stmt
	;

select:
	select_from
	| TOK_SELECT select_items_list TOK_FROM '(' subselect_start select_from ')'
		opt_outer_order opt_outer_limit
		{
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		}
	;

subselect_start:
	{
		CSphVector<CSphQueryItem> & dItems = pParser->m_pQuery->m_dItems;
		if ( dItems.GetLength()!=1 || dItems[0].m_sExpr!="*" )
		{
			yyerror ( pParser, "outer select list must be a single star" );
			YYERROR;
		}
		dItems.Reset();
		pParser->ResetSelect();
	};


opt_outer_order:
	// nothing
		{
			pParser->m_pQuery->m_sOuterOrderBy = pParser->m_pQuery->m_sOrderBy;
		}
	| TOK_ORDER TOK_BY order_items_list
		{
			pParser->m_pQuery->m_sOuterOrderBy.SetBinary ( pParser->m_pBuf+$3.m_iStart,
				$3.m_iEnd-$3.m_iStart );
		}
	;

opt_outer_limit:
	// nothing
	| TOK_LIMIT TOK_CONST_INT
		{
			pParser->m_pQuery->m_iOuterLimit = $2.m_iValue;
		}
	| TOK_LIMIT TOK_CONST_INT ',' TOK_CONST_INT
		{
			pParser->m_pQuery->m_iOuterOffset = $2.m_iValue;
			pParser->m_pQuery->m_iOuterLimit = $4.m_iValue;
		}
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
			pParser->m_pQuery->m_sIndexes.SetBinary ( pParser->m_pBuf+$4.m_iStart,
				$4.m_iEnd-$4.m_iStart );
		}
	;

select_items_list:
	select_item
	| select_items_list ',' select_item
	;

select_item:
	'*'									{ pParser->AddItem ( &$1 ); }
	| select_expr opt_alias
	;

opt_alias:
	// empty				
	| TOK_IDENT							{ pParser->AliasLastItem ( &$1 ); }
	| TOK_AS TOK_IDENT					{ pParser->AliasLastItem ( &$2 ); }
	;

select_expr:
	expr								{ pParser->AddItem ( &$1 ); }
	| TOK_AVG '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_AVG, &$1, &$4 ); }
	| TOK_MAX '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_MAX, &$1, &$4 ); }
	| TOK_MIN '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_MIN, &$1, &$4 ); }
	| TOK_SUM '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_SUM, &$1, &$4 ); }
	| TOK_GROUP_CONCAT '(' expr ')'		{ pParser->AddItem ( &$3, SPH_AGGR_CAT, &$1, &$4 ); }
	| TOK_COUNT '(' '*' ')'				{ if ( !pParser->AddItem ( "count(*)", &$1, &$4 ) ) YYERROR; }
	| TOK_WEIGHT '(' ')'				{ if ( !pParser->AddItem ( "weight()", &$1, &$3 ) ) YYERROR; }
	| TOK_GROUPBY '(' ')'				{ if ( !pParser->AddItem ( "groupby()", &$1, &$3 ) ) YYERROR; }
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
	| where_expr TOK_AND where_expr
	| '(' where_expr ')'
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
			if ( !pParser->AddUservarFilter ( $1.m_sValue, $4.m_sValue, true ) )
				YYERROR;
		}
	| expr_ident TOK_BETWEEN const_int TOK_AND const_int
		{
			if ( !pParser->AddIntRangeFilter ( $1.m_sValue, $3.m_iValue, $5.m_iValue ) )
				YYERROR;
		}
	| expr_ident '>' const_int
		{
			if ( !pParser->AddIntFilterGreater ( $1.m_sValue, $3.m_iValue, false ) )
				YYERROR;
		}
	| expr_ident '<' const_int
		{
			if ( !pParser->AddIntFilterLesser ( $1.m_sValue, $3.m_iValue, false ) )
				YYERROR;
		}
	| expr_ident TOK_GTE const_int
		{
			if ( !pParser->AddIntFilterGreater ( $1.m_sValue, $3.m_iValue, true ) )
				YYERROR;
		}
	| expr_ident TOK_LTE const_int
		{
			if ( !pParser->AddIntFilterLesser ( $1.m_sValue, $3.m_iValue, true ) )
				YYERROR;
		}
	| expr_float_unhandled
		{
			yyerror ( pParser, "EQ and NEQ filters on floats are not (yet?) supported" );
			YYERROR;
		}
	| expr_ident TOK_BETWEEN const_float TOK_AND const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1.m_sValue, $3.m_fValue, $5.m_fValue, true ) )
				YYERROR;
		}
	| expr_ident '>' const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1.m_sValue, $3.m_fValue, FLT_MAX, false ) )
				YYERROR;
		}
	| expr_ident '<' const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1.m_sValue, -FLT_MAX, $3.m_fValue, false ) )
				YYERROR;
		}
	| expr_ident TOK_GTE const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1.m_sValue, $3.m_fValue, FLT_MAX, true ) )
				YYERROR;
		}
	| expr_ident TOK_LTE const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1.m_sValue, -FLT_MAX, $3.m_fValue, true ) )
				YYERROR;
		}
	| expr_ident '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddStringFilter ( $1.m_sValue, $3.m_sValue, false ) )
				YYERROR;
		}
	| expr_ident TOK_NE TOK_QUOTED_STRING
		{
			if ( !pParser->AddStringFilter ( $1.m_sValue, $3.m_sValue, true ) )
				YYERROR;
		}
	;

expr_float_unhandled:
	expr_ident '=' const_float
	| expr_ident TOK_NE const_float
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
	| TOK_GROUPBY '(' ')'
		{
			$$.m_sValue = "@groupby";
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
	| '-' TOK_CONST_INT
		{
			$$.m_iInstype = TOK_CONST_INT;
			if ( (uint64_t)$2.m_iValue > (uint64_t)LLONG_MAX )
				$$.m_iValue = LLONG_MIN;
			else
				$$.m_iValue = -$2.m_iValue;
		}
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
			pParser->SetGroupBy ( $3.m_sValue );
		}
	;

opt_group_order_clause:
	// empty
	| group_order_clause
	;

group_order_clause:
	TOK_WITHIN TOK_GROUP TOK_ORDER TOK_BY order_items_list
		{
			if ( pParser->m_pQuery->m_sGroupBy.IsEmpty() )
			{
				yyerror ( pParser, "you must specify GROUP BY element in order to use WITHIN GROUP ORDER BY clause" );
				YYERROR;
			}
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
	| TOK_IDENT '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddOption ( $1, $3 ) )
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
	| TOK_ATIDENT				{ if ( !pParser->SetOldSyntax() ) YYERROR; }
	| TOK_ID					{ if ( !pParser->SetNewSyntax() ) YYERROR; }
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
	| '{' consthash '}'			{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
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

consthash:
	TOK_IDENT '=' const_int						{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| consthash ',' TOK_IDENT '=' const_int		{ $$ = $1; $$.m_iEnd = $5.m_iEnd; }
	;

//////////////////////////////////////////////////////////////////////////

show_stmt:
	TOK_SHOW show_what
	;

like_filter:
	// empty
	| TOK_LIKE TOK_QUOTED_STRING		{ pParser->m_pStmt->m_sStringParam = $2.m_sValue; }
	;

show_what:
	TOK_WARNINGS						{ pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; }
	| TOK_STATUS like_filter			{ pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; }
	| TOK_META like_filter				{ pParser->m_pStmt->m_eStmt = STMT_SHOW_META; }
	| TOK_AGENT TOK_STATUS like_filter	{ pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; }
	| TOK_AGENT TOK_QUOTED_STRING TOK_STATUS like_filter
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = $2.m_sValue;
		}
	| TOK_AGENT TOK_IDENT TOK_STATUS like_filter
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->m_pStmt->m_sIndex = $2.m_sValue;
		}
	| TOK_INDEX TOK_IDENT TOK_STATUS
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->m_pStmt->m_sIndex = $2.m_sValue;
		}
	;

//////////////////////////////////////////////////////////////////////////

simple_set_value:
	TOK_IDENT
	| TOK_NULL
	| TOK_QUOTED_STRING
	| TOK_CONST_INT
	| TOK_CONST_FLOAT
	;

set_value:
	simple_set_value
	| set_value '-' simple_set_value
	;

set_stmt:
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

set_global_stmt:
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
	| '(' const_list ')'	{ $$.m_iInstype = TOK_CONST_MVA; $$.m_pValues = $2.m_pValues; }
	| '(' ')'				{ $$.m_iInstype = TOK_CONST_MVA; }
	;

//////////////////////////////////////////////////////////////////////////

delete_from:
	TOK_DELETE TOK_FROM ident_list TOK_WHERE TOK_ID '=' const_int
		{
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = $3.m_sValue;
			pParser->m_pStmt->m_iListStart = $3.m_iStart;
			pParser->m_pStmt->m_iListEnd = $3.m_iEnd;
			pParser->m_pStmt->m_dDeleteIds.Add ( $7.m_iValue );
		}
	| TOK_DELETE TOK_FROM ident_list TOK_WHERE TOK_ID TOK_IN '(' const_list ')'
		{
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sIndex = $3.m_sValue;
			pParser->m_pStmt->m_iListStart = $3.m_iStart;
			pParser->m_pStmt->m_iListEnd = $3.m_iEnd;
			for ( int i=0; i<$8.m_pValues.Ptr()->GetLength(); i++ )
				pParser->m_pStmt->m_dDeleteIds.Add ( (*$8.m_pValues.Ptr())[i] );
		}
	;

//////////////////////////////////////////////////////////////////////////

call_proc:
	TOK_CALL TOK_IDENT '(' call_args_list opt_call_opts_list ')'
		{
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->m_pStmt->m_sCallProc = $2.m_sValue;
		}
	;

call_args_list:
	call_arg
		{
			AddInsval ( pParser->m_pStmt->m_dInsertValues, $1 );
		}
	| call_args_list ',' call_arg
		{
			AddInsval ( pParser->m_pStmt->m_dInsertValues, $3 );
		}
	;

call_arg:
	insert_val
	| '(' const_string_list ')'
		{
			$$.m_iInstype = TOK_CONST_STRINGS;
		}
	;

const_string_list:
	TOK_QUOTED_STRING
		{
			// FIXME? for now, one such array per CALL statement, tops
			if ( pParser->m_pStmt->m_dCallStrings.GetLength() )
			{
				yyerror ( pParser, "unexpected constant string list" );
				YYERROR;
			}
			pParser->m_pStmt->m_dCallStrings.Add ( $1.m_sValue );
		}
	| const_string_list ',' TOK_QUOTED_STRING
		{
			pParser->m_pStmt->m_dCallStrings.Add ( $3.m_sValue );
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
	describe_tok TOK_IDENT like_filter
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
	TOK_SHOW TOK_TABLES like_filter		{ pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES; }
	;

//////////////////////////////////////////////////////////////////////////

update:
	TOK_UPDATE ident_list TOK_SET update_items_list where_clause opt_option_clause
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
			pParser->UpdateAttr ( $1.m_sValue, &$3 );
		}
	| TOK_IDENT '=' const_float
		{
			pParser->UpdateAttr ( $1.m_sValue, &$3, SPH_ATTR_FLOAT);
		}
	| TOK_IDENT '='  '(' const_list ')'
		{
			pParser->UpdateMVAAttr ( $1.m_sValue, $4 );
		}
	| TOK_IDENT '='  '(' ')' // special case () means delete mva
		{
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( $1.m_sValue, tNoValues );
		}
	;

//////////////////////////////////////////////////////////////////////////

show_variables:
	TOK_SHOW opt_scope TOK_VARIABLES opt_show_variables_where
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		}
	;

opt_show_variables_where:
	| show_variables_where
	;

show_variables_where:
	TOK_WHERE show_variables_where_list
	;

show_variables_where_list:
	show_variables_where_entry
	| show_variables_where_list TOK_OR show_variables_where_entry
	;

show_variables_where_entry:
	TOK_IDENT '=' TOK_QUOTED_STRING // for example, Variable_name = 'character_set'
	;

show_collation:
	TOK_SHOW TOK_COLLATION
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		}
	;

show_character_set:
	TOK_SHOW TOK_CHARACTER TOK_SET
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		}
	;
	
set_transaction:
	TOK_SET opt_scope TOK_TRANSACTION TOK_ISOLATION TOK_LEVEL isolation_level
		{
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		}
	;

opt_scope:
	| TOK_GLOBAL
	| TOK_SESSION
	;

isolation_level:
	TOK_READ TOK_UNCOMMITTED
	| TOK_READ TOK_COMMITTED
	| TOK_REPEATABLE TOK_READ
	| TOK_SERIALIZABLE
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
	| TOK_STRING	{ $$ = SPH_ATTR_STRINGPTR; }
	;

drop_function:
	TOK_DROP TOK_FUNCTION TOK_IDENT
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNC;
			tStmt.m_sUdfName = $3.m_sValue;
		}
	;

////////////////////////////////////////////////////////////

attach_index:
	TOK_ATTACH TOK_INDEX TOK_IDENT TOK_TO TOK_RTINDEX TOK_IDENT
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			tStmt.m_sIndex = $3.m_sValue;
			tStmt.m_sStringParam = $6.m_sValue;
		}
	;

//////////////////////////////////////////////////////////////////////////

flush_rtindex:
	TOK_FLUSH TOK_RTINDEX TOK_IDENT
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			tStmt.m_sIndex = $3.m_sValue;
		}
	;

//////////////////////////////////////////////////////////////////////////

select_sysvar:
	TOK_SELECT sysvar_name opt_limit_clause
		{
			pParser->m_pStmt->m_eStmt = STMT_SELECT_SYSVAR;
			pParser->m_pStmt->m_tQuery.m_sQuery = $2.m_sValue;
		}
	;
	
sysvar_name:
	TOK_SYSVAR
	| TOK_SYSVAR '.' TOK_IDENT
		{
			$$.m_sValue.SetSprintf ( "%s.%s", $1.m_sValue.cstr(), $3.m_sValue.cstr() );
		}
	;

//////////////////////////////////////////////////////////////////////////

truncate:
	TOK_TRUNCATE TOK_RTINDEX TOK_IDENT
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
			tStmt.m_sIndex = $3.m_sValue;
		}
	;

//////////////////////////////////////////////////////////////////////////

optimize_index:
	TOK_OPTIMIZE TOK_INDEX TOK_IDENT
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_OPTIMIZE_INDEX;
			tStmt.m_sIndex = $3.m_sValue;
		}
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
