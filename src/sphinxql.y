%{
#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif
%}

%lex-param		{ SqlParser_c * pParser }
%parse-param	{ SqlParser_c * pParser }
%pure-parser
%error-verbose

%token	TOK_IDENT
%token	TOK_CONST_INT
%token	TOK_CONST_FLOAT
%token	TOK_QUOTED_STRING

%token	TOK_AS
%token	TOK_ASC
%token	TOK_AVG
%token	TOK_BEGIN
%token	TOK_BETWEEN
%token	TOK_BY
%token	TOK_CALL
%token	TOK_COMMIT
%token	TOK_COUNT
%token	TOK_DELETE
%token	TOK_DESC
%token	TOK_DISTINCT
%token	TOK_FALSE
%token	TOK_FROM
%token	TOK_GROUP
%token	TOK_ID
%token	TOK_IN
%token	TOK_INSERT
%token	TOK_INTO
%token	TOK_LIMIT
%token	TOK_MATCH
%token	TOK_MAX
%token	TOK_META
%token	TOK_MIN
%token	TOK_OPTION
%token	TOK_ORDER
%token	TOK_REPLACE
%token	TOK_ROLLBACK
%token	TOK_SELECT
%token	TOK_SET
%token	TOK_SHOW
%token	TOK_START
%token	TOK_STATUS
%token	TOK_SUM
%token	TOK_TRANSACTION
%token	TOK_TRUE
%token	TOK_VALUES
%token	TOK_WARNINGS
%token	TOK_WEIGHT
%token	TOK_WHERE
%token	TOK_WITHIN

%type	<m_iValue>		named_const_list

%left TOK_OR
%left TOK_AND
%left '|'
%left '&'
%left '=' TOK_NE
%left '<' '>' TOK_LTE TOK_GTE
%left '+' '-'
%left '*' '/'
%nonassoc TOK_NOT
%nonassoc TOK_NEG

%{
// some helpers
#include <float.h> // for FLT_MAX

static void AddFloatRangeFilter ( SqlParser_c * pParser, const CSphString & sAttr, float fMin, float fMax )
{
	CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
	tFilter.m_sAttrName = sAttr;
	tFilter.m_eType = SPH_FILTER_FLOATRANGE;
	tFilter.m_fMinValue = fMin;
	tFilter.m_fMaxValue = fMax;
}

static void AddUintRangeFilter ( SqlParser_c * pParser, const CSphString & sAttr, DWORD uMin, DWORD uMax )
{
	CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
	tFilter.m_sAttrName = sAttr;
	tFilter.m_eType = SPH_FILTER_RANGE;
	tFilter.m_uMinValue = uMin;
	tFilter.m_uMaxValue = uMax;
}

static void AddInsval ( CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode )
{
	SqlInsert_t & tIns = dVec.Add();
	tIns.m_iType = tNode.m_iInstype;
	tIns.m_iVal = tNode.m_iValue; // OPTIMIZE? copy conditionally based on type?
	tIns.m_fVal = tNode.m_fValue;
	tIns.m_sVal = tNode.m_sValue;
}
%}

%%

statement:
	select_from
	| show_clause
	| insert_into
	| delete_from
	| set_clause
	| transact_op
	| call_proc
	;

//////////////////////////////////////////////////////////////////////////

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
	TOK_IDENT									{ pParser->AddItem ( &$1, NULL ); }
	| expr TOK_AS TOK_IDENT						{ pParser->AddItem ( &$1, &$3 ); }
	| TOK_AVG '(' expr ')' TOK_AS TOK_IDENT		{ pParser->AddItem ( &$3, &$6, SPH_AGGR_AVG ); }
	| TOK_MAX '(' expr ')' TOK_AS TOK_IDENT		{ pParser->AddItem ( &$3, &$6, SPH_AGGR_MAX ); }
	| TOK_MIN '(' expr ')' TOK_AS TOK_IDENT		{ pParser->AddItem ( &$3, &$6, SPH_AGGR_MIN ); }
	| TOK_SUM '(' expr ')' TOK_AS TOK_IDENT		{ pParser->AddItem ( &$3, &$6, SPH_AGGR_SUM ); }
	| '*'										{ pParser->AddItem ( &$1, NULL ); }
	| TOK_COUNT '(' TOK_DISTINCT TOK_IDENT ')'
		{
			if ( !pParser->m_pQuery->m_sGroupDistinct.IsEmpty() )
			{
				yyerror ( pParser, "too many COUNT(DISTINCT) clauses" );
				YYERROR;

			} else
			{
				pParser->m_pQuery->m_sGroupDistinct = $4.m_sValue;
			}
		}
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
			if ( pParser->m_bGotQuery )
			{
				yyerror ( pParser, "too many MATCH() clauses" );
				YYERROR;

			} else
			{
				pParser->m_pQuery->m_sQuery = $3.m_sValue;
				pParser->m_bGotQuery = true;
			}
		}
	| TOK_ID '=' const_int
		{
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = "@id";
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( $3.m_iValue );
		}
	| TOK_IDENT '=' const_int
		{
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( $3.m_iValue );
		}
	| TOK_IDENT TOK_NE const_int
		{
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( $3.m_iValue );
			tFilter.m_bExclude = true;
		}
	| TOK_IDENT TOK_IN '(' const_list ')'
		{
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = $4.m_dValues;
		}
	| TOK_IDENT TOK_NOT TOK_IN '(' const_list ')'
		{
			CSphFilterSettings & tFilter = pParser->m_pQuery->m_dFilters.Add();
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = $4.m_dValues;
			tFilter.m_bExclude = true;
		}
	| TOK_IDENT TOK_BETWEEN const_int TOK_AND const_int
		{
			AddUintRangeFilter ( pParser, $1.m_sValue, $3.m_iValue, $5.m_iValue );
		}
	| TOK_IDENT '>' const_int
		{
			AddUintRangeFilter ( pParser, $1.m_sValue, $3.m_iValue+1, UINT_MAX );
		}
	| TOK_IDENT '<' const_int
		{
			AddUintRangeFilter ( pParser, $1.m_sValue, 0, $3.m_iValue-1 );
		}
	| TOK_IDENT TOK_GTE const_int
		{
			AddUintRangeFilter ( pParser, $1.m_sValue, $3.m_iValue, UINT_MAX );
		}
	| TOK_IDENT TOK_LTE const_int
		{
			AddUintRangeFilter ( pParser, $1.m_sValue, 0, $3.m_iValue );
		}
	| TOK_IDENT '=' const_float
	| TOK_IDENT TOK_NE const_float
	| TOK_IDENT '>' const_float
	| TOK_IDENT '<' const_float
		{
			yyerror ( pParser, "only >=, <=, and BETWEEN floating-point filter types are supported in this version" );
			YYERROR;
		}
	| TOK_IDENT TOK_BETWEEN const_float TOK_AND const_float
		{
			AddFloatRangeFilter ( pParser, $1.m_sValue, $3.m_fValue, $5.m_fValue );
		}
	| TOK_IDENT TOK_GTE const_float
		{
			AddFloatRangeFilter ( pParser, $1.m_sValue, $3.m_fValue, FLT_MAX );
		}
	| TOK_IDENT TOK_LTE const_float
		{
			AddFloatRangeFilter ( pParser, $1.m_sValue, -FLT_MAX, $3.m_fValue );
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
	const_int					{ $$.m_dValues.Add ( $1.m_iValue ); }
	| const_list ',' const_int	{ $$.m_dValues.Add ( $3.m_iValue ); }
	;

opt_group_clause:
	// empty
	| group_clause
	;

group_clause:
	TOK_GROUP TOK_BY TOK_IDENT
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
	;

order_items_list:
	order_item
	| order_items_list ',' order_item	{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	;

order_item:
	ident_or_id
	| ident_or_id TOK_ASC				{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	| ident_or_id TOK_DESC				{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	;

ident_or_id:
	TOK_ID
	| TOK_IDENT
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
	;

named_const_list:
	named_const
		{
			$$ = pParser->AllocNamedVec ();
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( $$ );

			assert ( !dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = $1.m_sValue;
			dVec.Last().m_iValue = $1.m_iValue;
		}
	| named_const_list ',' named_const
		{
			CSphVector<CSphNamedInt> & dVec = pParser->GetNamedVec ( $$ );

			assert ( dVec.GetLength() );
			dVec.Add();
			dVec.Last().m_sName = $3.m_sValue;
			dVec.Last().m_iValue = $3.m_iValue;
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
	| TOK_CONST_INT
	| TOK_CONST_FLOAT
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
	expr
	| arglist ',' expr
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

set_clause:
	TOK_SET TOK_IDENT '=' boolean_value
		{
			pParser->m_pStmt->m_eStmt = STMT_SET;
			pParser->m_pStmt->m_sSetName = $2.m_sValue;
			pParser->m_pStmt->m_iSetValue = $4.m_iValue;
		}
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
	TOK_COMMIT				{ pParser->m_pStmt->m_eStmt = STMT_COMMIT; }
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
			pParser->m_pStmt->m_sInsertIndex = $3.m_sValue;
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
	ident_or_id							{ if ( !pParser->AddSchemaItem ( &$1 ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }
	| column_list ',' ident_or_id		{ if ( !pParser->AddSchemaItem ( &$3 ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }
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
	;

//////////////////////////////////////////////////////////////////////////

delete_from:
	TOK_DELETE TOK_FROM TOK_IDENT TOK_WHERE TOK_ID '=' const_int
		{
			pParser->m_pStmt->m_eStmt = STMT_DELETE;
			pParser->m_pStmt->m_sDeleteIndex = $3.m_sValue;
			pParser->m_pStmt->m_iDeleteID = $7.m_iValue;
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
	insert_val TOK_AS call_opt_name
		{
			pParser->m_pStmt->m_dCallOptNames.Add ( $3.m_sValue );
			AddInsval ( pParser->m_pStmt->m_dCallOptValues, $1 );
		}
	;

call_opt_name:
	TOK_IDENT
	| TOK_LIMIT		{ $$.m_sValue = "limit"; }
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
