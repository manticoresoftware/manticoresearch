%{
#if _WIN32
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif
%}

%lex-param		{ SqlParser_c * pParser }
%parse-param	{ SqlParser_c * pParser }
%pure-parser
%error-verbose

%token	TOK_IDENT "identifier"
%token	TOK_BACKIDENT "`identifier`"
%token	TOK_ATIDENT
%token	TOK_CONST_INT 260 "integer"
%token	TOK_CONST_FLOAT 261 "float"
%token	TOK_CONST_MVA 262	// not a real token, only placeholder
%token	TOK_QUOTED_STRING 263 "string"
%token	TOK_NULL 473 "null"
%token	TOK_USERVAR "@uservar"
%token	TOK_SYSVAR "@@sysvar"
%token	TOK_CONST_STRINGS 269	// not a real token, only placeholder
%token	TOK_BAD_NUMERIC
%token	TOK_SUBKEY
%token	TOK_BACKTICKED_SUBKEY
%token	TOK_DOT_NUMBER ".number"
%token	TOK_MANTICORE_DOT "Manticore."
%token	TOK_SYSTEM "system"

%token	TOK_AGENT
%token	TOK_ALL
%token	TOK_ANY
%token	TOK_AS
%token	TOK_ASC
%token	TOK_AVG
%token	TOK_BEGIN
%token	TOK_BETWEEN
%token	TOK_BIGINT
%token	TOK_BY
%token	TOK_CALL
%token	TOK_CHARACTER
%token	TOK_CHUNK
%token	TOK_CLUSTER
%token  TOK_COLLATE
%token	TOK_COLLATION
%token	TOK_COLUMN
%token	TOK_COMMIT
%token	TOK_COUNT
%token	TOK_CREATE
%token	TOK_DATABASES
%token	TOK_DAY
%token	TOK_DATE_ADD
%token	TOK_DATE_SUB
%token	TOK_DELETE
%token	TOK_DESC
%token	TOK_DESCRIBE
%token	TOK_DISTINCT
%token	TOK_DIV
%token	TOK_DOUBLE
%token	TOK_EXPLAIN
%token	TOK_FACET
%token	TOK_FALSE
%token	TOK_FLOAT
%token	TOK_FOR
%token	TOK_FORCE
%token	TOK_FROM
%token	TOK_FREEZE
%token	TOK_GLOBAL
%token	TOK_GROUP
%token	TOK_GROUPBY
%token	TOK_GROUP_CONCAT
%token	TOK_HAVING
%token	TOK_HINT_SECONDARY
%token	TOK_HINT_NO_SECONDARY
%token	TOK_HINT_DOCID
%token	TOK_HINT_NO_DOCID
%token	TOK_HINT_CLOSE
%token	TOK_HINT_COLUMNAR
%token	TOK_HINT_NO_COLUMNAR
%token	TOK_HINT_OPEN
%token	TOK_HOSTNAMES
%token	TOK_HOUR
%token	TOK_IGNORE
%token	TOK_IN
%token	TOK_INDEX
%token	TOK_INDEXES
%token	TOK_INDEXOF
%token	TOK_INNER
%token	TOK_INSERT
%token	TOK_INT
%token	TOK_INTERVAL
%token	TOK_INTEGER
%token	TOK_INTO
%token	TOK_IS
%token	TOK_JOIN
%token	TOK_KILL
%token	TOK_KNN
%token	TOK_LEFT
%token	TOK_LIKE
%token	TOK_LIMIT
%token	TOK_LOGS
%token	TOK_LOCKS
%token	TOK_MATCH
%token	TOK_MAX
%token	TOK_META
%token	TOK_MIN
%token	TOK_MINUTE
%token	TOK_MOD
%token	TOK_MONTH
%token	TOK_MULTI
%token	TOK_MULTI64
%token	TOK_NAMES
%token	TOK_NOT
%token	TOK_OFFSET
%token	TOK_ON
%token	TOK_OPTION
%token	TOK_ORDER
%token	TOK_OPTIMIZE
%token	TOK_PLAN
%token	TOK_PLUGINS
%token	TOK_PROFILE
%token	TOK_QUARTER
%token	TOK_QUERY
%token	TOK_RAND
%token	TOK_REBUILD
%token	TOK_REGEX
%token	TOK_RELOAD
%token	TOK_REPLACE
%token	TOK_REMAP
%token	TOK_ROLLBACK
%token	TOK_SCROLL
%token	TOK_SECOND
%token	TOK_SECONDARY
%token	TOK_SELECT
%token	TOK_SET
%token	TOK_SETTINGS
%token	TOK_SESSION
%token	TOK_SHOW
%token	TOK_SONAME
%token	TOK_START
%token	TOK_STATUS
%token	TOK_STRING
%token	TOK_SYSFILTERS
%token	TOK_SUM
%token	TOK_TABLE 378
%token	TOK_TABLES
%token	TOK_THREADS
%token	TOK_TO
%token	TOK_TRANSACTION
%token	TOK_TRUE
%token	TOK_UNFREEZE
%token	TOK_UPDATE
%token	TOK_VALUES
%token	TOK_VARIABLES
%token	TOK_WARNINGS
%token	TOK_WEEK
%token	TOK_WEIGHT
%token	TOK_WHERE
%token	TOK_WITHIN
%token	TOK_YEAR

%token	TOK_LTE "<="
%token	TOK_GTE ">="
%token	TOK_NE "!="

%right TOK_TABLE TOK_STATUS

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

static CSphFilterSettings * AddMvaRange ( SqlParser_c * pParser, const SqlNode_t & tNode, int64_t iMin, int64_t iMax )
{
	CSphFilterSettings * f = pParser->AddFilter ( tNode, SPH_FILTER_RANGE );
	f->m_eMvaFunc = ( tNode.m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
	f->m_iMinValue = iMin;
	f->m_iMaxValue = iMax;
	return f;
}

#define TRACK_BOUNDS(_res,_left,_right) \
	_res = _left; \
	if ( _res.m_iStart>0 && pParser->m_pBuf[_res.m_iStart-1]=='`' ) \
		_res.m_iStart--; \
	_res.m_iEnd = _right.m_iEnd; \
	_res.m_iType = 0;

%}

%%

request:
	statement					{ pParser->PushQuery(); }
	| statement ';'				{ pParser->PushQuery(); }
	| multi_stmt_list
	| multi_stmt_list ';'
	;

multi_stmt_list:
	multi_stmt							{ pParser->PushQuery(); }
	| multi_stmt_list ';' multi_stmt	{ pParser->PushQuery(); }
	| multi_stmt_list facet_stmt		{ pParser->PushQuery(); }
	;

statement:
	insert_into
	| delete_from
	| transact_op
	| call_proc
	| describe
	| update
	| select_without_from
	| optimize_index
	| sysfilters
	| explain_query
	;

multi_stmt:
	select
	| show_stmt
	| set_stmt
	;

//////////////////////////////////////////////////////////////////////////

// many known keywords are also allowed as column or index names
//
// FROM conflicts with select_expr opt_alias
// NAMES, TRANSACTION conflicts with SET
//
// more known conflicts (but rather crappy ideas for a name anyway) are:
// AND, AS, BY, DIV, FACET, FALSE, ID, IN, IS, LIMIT, MOD, NOT, NULL,
// OR, ORDER, SELECT, TRUE

/// All used _reserved_ keywords looking as TOK_XXX should be located here. That is mandatory
/// and used in regexp in 'reserved.py' script to check reserved words consistency between
/// grammar and documentation.
///
/// Line below starts the list, that is MANDATORY, don't remove/change it!
/// *** ALL_IDENT_LIST ***

reserved_tokens_without_option:
	TOK_AGENT | TOK_ALL | TOK_ANY | TOK_ASC
	| TOK_AVG | TOK_BEGIN | TOK_BETWEEN | TOK_BIGINT | TOK_CALL
	| TOK_CHARACTER | TOK_CHUNK | TOK_CLUSTER | TOK_COLLATION | TOK_COLUMN | TOK_COMMIT
	| TOK_COUNT | TOK_CREATE | TOK_DATABASES | TOK_DELETE
	| TOK_DESC | TOK_DESCRIBE  | TOK_DOUBLE
	| TOK_FLOAT | TOK_FOR | TOK_FREEZE | TOK_GLOBAL | TOK_GROUP
	| TOK_GROUP_CONCAT | TOK_GROUPBY | TOK_HAVING | TOK_HOSTNAMES | TOK_INDEX | TOK_INDEXOF | TOK_INSERT
	| TOK_INT | TOK_INTEGER | TOK_INTO
	| TOK_LIKE | TOK_LOGS | TOK_MATCH | TOK_MAX | TOK_META | TOK_MIN | TOK_MULTI
	| TOK_MULTI64 | TOK_OPTIMIZE | TOK_PLAN
	| TOK_PLUGINS | TOK_PROFILE | TOK_RAND | TOK_REBUILD
	| TOK_REMAP | TOK_REPLACE
	| TOK_ROLLBACK | TOK_SECONDARY | TOK_SESSION | TOK_SET
	| TOK_SETTINGS | TOK_SHOW | TOK_SONAME | TOK_START | TOK_STATUS | TOK_STRING
	| TOK_SUM | TOK_TABLE | TOK_TABLES | TOK_THREADS | TOK_TO
	| TOK_UNFREEZE | TOK_UPDATE | TOK_VALUES | TOK_VARIABLES
	| TOK_WARNINGS | TOK_WEIGHT | TOK_WHERE | TOK_WITHIN | TOK_KILL | TOK_QUERY
	| TOK_INTERVAL | TOK_REGEX
	| TOK_DATE_ADD | TOK_DATE_SUB | TOK_DAY | TOK_HOUR | TOK_MINUTE | TOK_MONTH | TOK_QUARTER | TOK_SECOND | TOK_WEEK | TOK_YEAR
	| TOK_LOCKS | TOK_SCROLL
	;

names_transaction_collate:
    TOK_NAMES | TOK_TRANSACTION | TOK_COLLATE
    ;

ident_without_option:
	TOK_IDENT | reserved_tokens_without_option
	;

ident_for_set_stmt:
	TOK_OPTION | ident_without_option
	;

/// *** ALL_IDENT_LIST_END ***
// WARNING! line above is MANDATORY for consistency checking!
//////////////////////////////////////////////////////////////////////////

non_reserved_tokens:
	TOK_SYSTEM
	;

all_set_tail:
	names_transaction_collate | non_reserved_tokens
	;

ident:
	ident_for_set_stmt | all_set_tail | TOK_BACKIDENT
	;

option_name:
	ident_without_option | all_set_tail | TOK_FORCE 
	;


/// id of columns
identcol:
	ident
	| identcol ':' ident {TRACK_BOUNDS ( $$, $1, $3 );}
	;

/// indexes
single_tablename:
	TOK_BACKIDENT
	{
		$$ = $1;
		++$$.m_iStart;
		--$$.m_iEnd;
	}
	| ident_for_set_stmt | all_set_tail
	;

// table or Manticore.table
single_manticore_tablename:
	 single_tablename
	 | TOK_MANTICORE_DOT single_tablename {$$ = $2;}
	;

tablename_with_maybecluster:
	single_manticore_tablename
	| ident ':' single_manticore_tablename
		{
			pParser->ToString (pParser->m_pStmt->m_sCluster, $1);
			$$ = $3;
		}
	;

// .1 OR chunk 3
chunk:
	TOK_DOT_NUMBER
		{
			pParser->AddDotIntSubkey ($1);
			pParser->m_pStmt->m_iIntParam = $1.GetValueInt();
		}
	| TOK_CHUNK TOK_CONST_INT
		{
			pParser->AddIntSubkey ($2);
			pParser->m_pStmt->m_iIntParam = $2.GetValueInt();
		}
	;

// .1.2 chunk 4
chunks:
	chunk
	| chunks chunk
	;

// .subkey
string_key:
	TOK_SUBKEY
		{
    		pParser->AddStringSubkey ($1);
    	}
    ;

// .subkey1 .subkey2 ...
string_keys:
	string_key
	| string_keys string_key
	;

// .subkey.subkey or .1.2.subkey.4
maybechunk_then_subkeys:
	string_keys
	| chunk string_keys
	;

// .2
maybesubkey_then_chunks:
	chunks
	| string_key chunks
	;

subkeys_for_target_in_select_from:
	// empty
	| maybechunk_then_subkeys
	| maybesubkey_then_chunks
	;

single_tablename_with_maybecluster:
	tablename_with_maybecluster
		{
			pParser->SetIndex ($1);
		}
	;

one_index_opt_subindex:				// used in describe and insert (meta m.b. num or name)
	single_tablename_with_maybecluster
	| single_tablename_with_maybecluster maybechunk_then_subkeys
	;

one_index_opt_chunk:				// used in show settings, show status, update
	single_tablename_with_maybecluster
	| single_tablename_with_maybecluster chunks
	| list_of_indexes
    	{
    		pParser->ToString (pParser->m_pQuery->m_sIndexes, $1);
    	}
	;

target_in_update:
	one_index_opt_chunk
	| single_tablename_with_maybecluster string_keys
	| single_tablename_with_maybecluster string_keys chunks
	;

target_in_delete_from:
	single_tablename_with_maybecluster
	| single_tablename_with_maybecluster chunks
	| single_tablename_with_maybecluster string_keys
	| single_tablename_with_maybecluster string_keys chunks
	;

target_in_select_from:		// used in select ... from
	tablename_with_maybecluster subkeys_for_target_in_select_from
		{
			pParser->SetIndex ($1);
			pParser->m_pQuery->m_sIndexes = pParser->m_pStmt->m_sIndex;
			pParser->SwapSubkeys();
		}
	| list_of_indexes
		{
			pParser->ToString (pParser->m_pQuery->m_sIndexes, $1);
		}
	| sysvar_ext
		{
    		pParser->ToString (pParser->m_pQuery->m_sIndexes, $1);
    		pParser->SwapSubkeys();
    	}
	;

list_of_indexes:
	tablename_with_maybecluster ',' tablename_with_maybecluster
		{
    		TRACK_BOUNDS ( $$, $1, $3 );
    	}
	| list_of_indexes ',' tablename_with_maybecluster
		{
			TRACK_BOUNDS ( $$, $1, $3 );
		}
	;

// enhanced sysvars
//////////////////////////////////////////////////////////////////////////

string_nokeys:
	TOK_SUBKEY
	| string_keys TOK_SUBKEY
		{
			$$ = $2;
		}
	;

multi_strings_and_maybe_chunk_nokey:
	string_nokeys
	| TOK_DOT_NUMBER
	| TOK_DOT_NUMBER string_nokeys
		{
    		$$ = $2;
    	}
    | string_nokeys TOK_DOT_NUMBER
    	{
    		$$ = $2;
    	}
    | string_nokeys TOK_DOT_NUMBER string_nokeys
    	{
    		$$ = $3;
    	}
	;

sysvar:					// full name in token, like var '@@session.last_insert_id', no subkeys parsed
	TOK_SYSVAR
	| TOK_SYSVAR multi_strings_and_maybe_chunk_nokey
		{
			TRACK_BOUNDS ( $$, $1, $2 );
		}
    ;

sysvar_ext:				// name in token + subkeys, like var '@@session' and 1 subkey '.last_insert_id'
	TOK_SYSVAR
	| TOK_SYSVAR maybechunk_then_subkeys
	| TOK_SYSVAR chunk
	;

//////////////////////////////////////////////////////////////////////////

select_without_from:
	TOK_SELECT select_items_list opt_limit_clause
		{
			pParser->m_pStmt->m_eStmt = STMT_SELECT_COLUMNS;
		}
	;

// statements
//////////////////////////////////////////////////////////////////////////



select:
	select1
	| TOK_SELECT TOK_IDENT '(' '(' select1 ')' opt_tablefunc_args ')'
		{
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by table argument
			pParser->ToString ( pParser->m_pStmt->m_sTableFunc, $2 );
		}
	;

select1:
	select_from
	| TOK_SELECT select_items_list TOK_FROM '(' subselect_start select_from ')'
		opt_outer_order opt_outer_limit
		{
			assert ( pParser->m_pStmt->m_eStmt==STMT_SELECT ); // set by subselect
		}
	;

opt_tablefunc_args:
	// nothing
	| ',' tablefunc_args_list
	;

tablefunc_args_list:
	tablefunc_arg
		{
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), $1 );
		}
	| tablefunc_args_list ',' tablefunc_arg
		{
			pParser->ToString ( pParser->m_pStmt->m_dTableFuncArgs.Add(), $3 );
		}
	;

tablefunc_arg:
	identcol
	| TOK_CONST_INT
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
	TOK_ORDER TOK_BY order_items_list
		{
			pParser->ToString ( pParser->m_pQuery->m_sOuterOrderBy, $3 );
			pParser->m_pQuery->m_bHasOuter = true;
		}
	;

opt_outer_limit:
	// nothing
	| TOK_LIMIT TOK_CONST_INT
		{
			pParser->m_pQuery->m_iOuterLimit = $2.GetValueInt();
			pParser->m_pQuery->m_bHasOuter = true;
		}
	| TOK_LIMIT TOK_CONST_INT ',' TOK_CONST_INT
		{
			pParser->m_pQuery->m_iOuterOffset = $2.GetValueInt();
			pParser->m_pQuery->m_iOuterLimit = $4.GetValueInt();
			pParser->m_pQuery->m_bHasOuter = true;
		}
	| TOK_LIMIT TOK_CONST_INT TOK_OFFSET TOK_CONST_INT
		{
			pParser->m_pQuery->m_iOuterLimit = $2.GetValueInt();
			pParser->m_pQuery->m_iOuterOffset = $4.GetValueInt();
			pParser->m_pQuery->m_bHasOuter = true;
		}
	;

select_from:
	TOK_SELECT select_items_list
	TOK_FROM target_in_select_from { pParser->m_pStmt->m_eStmt = STMT_SELECT; } // set stmt here to check the option below
	opt_join_clause
	opt_where_clause
	opt_group_clause
	opt_group_order_clause
	opt_having_clause
	opt_order_clause
	opt_limit_clause
	opt_option_clause
	opt_hint_clause
	;

// common to ALL select statements
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
	| opt_as identcol						{ pParser->AliasLastItem ( &$2 ); }
	;

distinct_joined_col:
	TOK_SUBKEY
	| distinct_joined_col TOK_SUBKEY	{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	;

distinct_ident:
	identcol
	| identcol distinct_joined_col		{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	;

select_expr:
	expr								{ pParser->AddItem ( &$1 ); }
	| sysvar							{ pParser->AddItem ( &$1 ); }
	| TOK_AVG '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_AVG, &$1, &$4 ); }
	| TOK_MAX '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_MAX, &$1, &$4 ); }
	| TOK_MIN '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_MIN, &$1, &$4 ); }
	| TOK_SUM '(' expr ')'				{ pParser->AddItem ( &$3, SPH_AGGR_SUM, &$1, &$4 ); }
	| TOK_GROUP_CONCAT '(' expr ')'		{ pParser->AddItem ( &$3, SPH_AGGR_CAT, &$1, &$4 ); }
	| TOK_COUNT '(' '*' ')'				{ if ( !pParser->AddItem ( "count(*)", &$1, &$4 ) ) YYERROR; }
	| TOK_GROUPBY '(' ')'				{ if ( !pParser->AddItem ( "groupby()", &$1, &$3 ) ) YYERROR; }
	| TOK_COUNT '(' TOK_DISTINCT distinct_ident ')' { if ( !pParser->AddDistinct ( &$4, &$1, &$5 ) ) YYERROR; }
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
	| filter_expr
	| where_item TOK_AND filter_expr
	| where_item TOK_AND where_item
	| where_item TOK_AND filter_expr TOK_AND where_item
	| where_item TOK_AND where_item TOK_AND filter_expr
	| filter_expr TOK_AND where_item TOK_AND where_item
    | filter_expr TOK_AND where_item
	| filter_expr TOK_AND where_item TOK_AND filter_expr	{ pParser->FilterAnd ( $$, $1, $5 ); }
	;

where_item:
	TOK_MATCH '(' TOK_QUOTED_STRING ')'
		{
			if ( !pParser->SetMatch($3) )
				YYERROR;
		}
	| TOK_MATCH '(' TOK_QUOTED_STRING ',' single_tablename ')'
		{
			if ( !pParser->AddMatch($3,$5) )
				YYERROR;
		}
	| '(' TOK_MATCH '(' TOK_QUOTED_STRING ')' ')'
		{
			if ( !pParser->SetMatch($4) )
				YYERROR;
		}
	| '(' TOK_MATCH '(' TOK_QUOTED_STRING  ',' single_tablename ')' ')'
		{
			if ( !pParser->AddMatch($4,$6) )
				YYERROR;
		}
	;

opt_join_clause:
	// empty
	| join_clause
	;

join_type:
	// empty
	| TOK_INNER	{ pParser->SetJoinType ( JoinType_e::INNER ); }
	| TOK_LEFT	{ pParser->SetJoinType ( JoinType_e::LEFT ); }
	;

join_clause:
	join_type TOK_JOIN single_tablename TOK_ON on_clause
		{
			if ( !pParser->SetJoin($3) )
				YYERROR;
		}
	;

on_clause_attr:
	TOK_SUBKEY
	| on_clause_attr TOK_SUBKEY			{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	;

on_clause_type_cast:
	TOK_INT			{ pParser->SetJoinOnCast(SPH_ATTR_INTEGER); }
	| TOK_FLOAT		{ pParser->SetJoinOnCast(SPH_ATTR_FLOAT); }
	| TOK_STRING	{ pParser->SetJoinOnCast(SPH_ATTR_STRING); }
	;

on_clause_equality:
    single_tablename on_clause_attr '=' single_tablename on_clause_attr				{ pParser->AddOnFilter ( $1, $2, $4, $5, -1 ); }
	| on_clause_type_cast '(' single_tablename on_clause_attr ')' '=' single_tablename on_clause_attr	{ pParser->AddOnFilter ( $3, $4, $7, $8, 0 ); }
	| single_tablename on_clause_attr '=' on_clause_type_cast '(' single_tablename on_clause_attr	')' { pParser->AddOnFilter ( $1, $2, $6, $7, 1 ); }
	;

on_clause:
	on_clause_equality
	| on_clause TOK_AND on_clause
	;

knn_item:
	TOK_KNN '(' ident ',' const_int ',' '(' const_list ')' ')'
		{
			if ( !pParser->SetKNN ( $3, $5, $8, nullptr ) )
				YYERROR;
		}
	| TOK_KNN '(' ident ',' const_int ',' '(' const_list ')' ',' const_int ')'
		{
			if ( !pParser->SetKNN ( $3, $5, $8, &$11 ) )
				YYERROR;
		}
	;

filter_expr:
	filter_item							{ pParser->SetOp ( $$ ); }
	| filter_expr TOK_AND filter_expr	{ pParser->FilterAnd ( $$, $1, $3 ); }
	| filter_expr TOK_OR filter_expr	{ pParser->FilterOr ( $$, $1, $3 ); }
	| '(' filter_expr ')'				{ pParser->FilterGroup ( $$, $2 ); }
	| knn_item
	;
	
filter_item:	
	expr_ident '=' bool_or_integer_value

		{
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( $1 );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( $3.GetValueInt() );
		}
	| expr_ident TOK_NE bool_or_integer_value
		{
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( $1 );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( $3.GetValueInt() );
			pFilter->m_bExclude = true;
		}
	| expr_ident TOK_IN '(' const_list ')'
		{
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( $1, $4.m_iValues );
			if ( !pFilter )
				YYERROR;
		}
	| expr_ident TOK_NOT TOK_IN '(' const_list ')'
		{
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( $1, $5.m_iValues );
			if ( !pFilter )
				YYERROR;
			pFilter->m_bExclude = true;
		}
	| expr_ident TOK_IN '(' string_list ')'
		{
			if ( !pParser->AddStringListFilter ( $1, $4, StrList_e::STR_IN ) )
				YYERROR;
		}
	| expr_ident TOK_NOT TOK_IN '(' string_list ')'
		{
			if ( !pParser->AddStringListFilter ( $1, $5, StrList_e::STR_IN, true ) )
				YYERROR;
		}
	| expr_ident TOK_ANY '(' string_list ')'
		{
			if ( !pParser->AddStringListFilter ( $1, $4, StrList_e::STR_ANY ) )
				YYERROR;
		}
	| expr_ident TOK_NOT TOK_ANY '(' string_list ')'
		{
			if ( !pParser->AddStringListFilter ( $1, $5, StrList_e::STR_ANY, true ) )
				YYERROR;
		}
	| expr_ident TOK_ALL '(' string_list ')'
		{
			if ( !pParser->AddStringListFilter ( $1, $4, StrList_e::STR_ALL ) )
				YYERROR;
		}
	| expr_ident TOK_NOT TOK_ALL '(' string_list ')'
		{
			if ( !pParser->AddStringListFilter ( $1, $5, StrList_e::STR_ALL, true ) )
				YYERROR;
		}
	| expr_ident TOK_IN ident
		{
			if ( !pParser->AddUservarFilter ( $1, $3, false ) )
				YYERROR;
		}
	| expr_ident TOK_NOT TOK_IN ident
		{
			if ( !pParser->AddUservarFilter ( $1, $4, true ) )
				YYERROR;
		}
	| expr_ident TOK_BETWEEN const_int TOK_AND const_int
		{
			if ( !pParser->AddIntRangeFilter ( $1, $3.GetValueInt(), $5.GetValueInt(), false ) )
				YYERROR;
		}
	| expr_ident TOK_NOT TOK_BETWEEN const_int TOK_AND const_int
		{
			if ( !pParser->AddIntRangeFilter ( $1, $4.GetValueInt(), $6.GetValueInt(), true ) )
				YYERROR;
		}
	| expr_ident '>' const_int
		{
			if ( !pParser->AddIntFilterGreater ( $1, $3.GetValueInt(), false ) )
				YYERROR;
		}
	| expr_ident '<' const_int
		{
			if ( !pParser->AddIntFilterLesser ( $1, $3.GetValueInt(), false ) )
				YYERROR;
		}
	| expr_ident TOK_GTE const_int
		{
			if ( !pParser->AddIntFilterGreater ( $1, $3.GetValueInt(), true ) )
				YYERROR;
		}
	| expr_ident TOK_LTE const_int
		{
			if ( !pParser->AddIntFilterLesser ( $1, $3.GetValueInt(), true ) )
				YYERROR;
		}
	| expr_ident '=' const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1, $3.m_fValue, $3.m_fValue, true ) )
				YYERROR;
		}
	| expr_ident TOK_NE const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1, $3.m_fValue, $3.m_fValue, true, true ) )
				YYERROR;
		}
	| expr_ident TOK_BETWEEN const_float TOK_AND const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1, $3.m_fValue, $5.m_fValue, true ) )
				YYERROR;
		}
	| expr_ident TOK_BETWEEN const_int TOK_AND const_float
		{
			if ( !pParser->AddFloatRangeFilter ( $1, $3.GetValueInt(), $5.m_fValue, true ) )
				YYERROR;
		}
	| expr_ident TOK_BETWEEN const_float TOK_AND const_int
		{
			if ( !pParser->AddFloatRangeFilter ( $1, $3.m_fValue, $5.GetValueInt(), true ) )
				YYERROR;
		}
	| expr_ident '>' const_float
		{
			if ( !pParser->AddFloatFilterGreater ( $1, $3.m_fValue, false ) )
				YYERROR;
		}
	| expr_ident '<' const_float
		{
			if ( !pParser->AddFloatFilterLesser ( $1, $3.m_fValue, false ) )
				YYERROR;
		}
	| expr_ident TOK_GTE const_float
		{
			if ( !pParser->AddFloatFilterGreater ( $1, $3.m_fValue, true ) )
				YYERROR;
		}
	| expr_ident TOK_LTE const_float
		{
			if ( !pParser->AddFloatFilterLesser ( $1, $3.m_fValue, true ) )
				YYERROR;
		}
	| expr_ident '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddStringFilter ( $1, $3, false ) )
				YYERROR;
		}
	| expr_ident TOK_NE TOK_QUOTED_STRING
		{
			if ( !pParser->AddStringFilter ( $1, $3, true ) )
				YYERROR;
		}
	| expr_ident '>' TOK_QUOTED_STRING
		{
			if ( !pParser->AddStringCmpFilter ( $1, $3, false, EStrCmpDir::GT ) )
				YYERROR;
		}
	| expr_ident '<' TOK_QUOTED_STRING
		{
			if ( !pParser->AddStringCmpFilter ( $1, $3, false, EStrCmpDir::LT ) )
				YYERROR;
		}
	| expr_ident TOK_GTE TOK_QUOTED_STRING
		{
			if ( !pParser->AddStringCmpFilter ( $1, $3, true, EStrCmpDir::LT ) )
				YYERROR;
		}
	| expr_ident TOK_LTE TOK_QUOTED_STRING
		{
			if ( !pParser->AddStringCmpFilter ( $1, $3, true, EStrCmpDir::GT ) )
				YYERROR;
		}
	| expr_ident TOK_IS TOK_NULL
		{
			if ( !pParser->AddNullFilter ( $1, true ) )
				YYERROR;
		}
	| expr_ident TOK_IS TOK_NOT TOK_NULL
		{
			if ( !pParser->AddNullFilter ( $1, false ) )
				YYERROR;
		}
	| const_int '=' const_int
		{
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( $1 );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( $3.GetValueInt() );
		}
	| const_int TOK_NE const_int
		{
			CSphFilterSettings * pFilter = pParser->AddValuesFilter ( $1 );
			if ( !pFilter )
				YYERROR;
			pFilter->m_dValues.Add ( $3.GetValueInt() );
			pFilter->m_bExclude = true;
		}

	// filters on ANY(mva) or ALL(mva)
	| mva_aggr '=' const_int
		{
			CSphFilterSettings * f = pParser->AddFilter ( $1, SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( $1.m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
			f->m_dValues.Add ( $3.GetValueInt() );
		}
	| mva_aggr TOK_NE const_int
		{
			// tricky bit
			// any(tags!=val) is not equivalent to not(any(tags==val))
			// any(tags!=val) is instead equivalent to not(all(tags)==val)
			// thus, along with setting the exclude flag on, we also need to invert the function
			CSphFilterSettings * f = pParser->AddFilter ( $1, SPH_FILTER_VALUES );
			f->m_eMvaFunc = ( $1.m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;         
			f->m_dValues.Add ( $3.GetValueInt() );
		}
	| mva_aggr TOK_IN '(' const_list ')'
		{
			CSphFilterSettings * f = pParser->AddFilter ( $1, SPH_FILTER_VALUES, $4.m_iValues );
			f->m_eMvaFunc = ( $1.m_iType==TOK_ALL ) ? SPH_MVAFUNC_ALL : SPH_MVAFUNC_ANY;
		}
	| mva_aggr TOK_NOT TOK_IN '(' const_list ')'
		{
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( $1, SPH_FILTER_VALUES, $5.m_iValues );
			f->m_eMvaFunc = ( $1.m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
		}
	| mva_aggr TOK_BETWEEN const_int TOK_AND const_int
		{
			AddMvaRange ( pParser, $1, $3.GetValueInt(), $5.GetValueInt() );
		}
	| mva_aggr TOK_NOT TOK_BETWEEN const_int TOK_AND const_int
		{
			// tricky bit with inversion again
			CSphFilterSettings * f = pParser->AddFilter ( $1, SPH_FILTER_RANGE );
			f->m_eMvaFunc = ( $1.m_iType==TOK_ALL ) ? SPH_MVAFUNC_ANY : SPH_MVAFUNC_ALL;
			f->m_bExclude = true;
			f->m_iMinValue = $4.GetValueInt();
			f->m_iMaxValue = $6.GetValueInt();
		}
	| mva_aggr '<' const_int
		{
			AddMvaRange ( pParser, $1, INT64_MIN, $3.GetValueInt()-1 );
		}
	| mva_aggr '>' const_int
		{
			AddMvaRange ( pParser, $1, $3.GetValueInt()+1, INT64_MAX );
		}
	| mva_aggr TOK_LTE const_int
		{
			AddMvaRange ( pParser, $1, INT64_MIN, $3.GetValueInt() );
		}
	| mva_aggr TOK_GTE const_int
		{
			AddMvaRange ( pParser, $1, $3.GetValueInt(), INT64_MAX );
		}
	| TOK_REGEX '(' json_field ',' TOK_QUOTED_STRING ')'
		{
			TRACK_BOUNDS ( $$, $1, $6 );
			CSphFilterSettings * pFilter = pParser->AddFilter ( $$, SPH_FILTER_EXPRESSION );
			if ( !pFilter )
				YYERROR;
		}
	;

expr_ident:
	identcol
	| TOK_ATIDENT
		{
			if ( !pParser->SetOldSyntax() )
				YYERROR;
		}
	| TOK_COUNT '(' '*' ')'
		{
			$$.m_iType = SPHINXQL_TOK_COUNT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}
	| TOK_GROUPBY '(' ')'
		{
			$$.m_iType = SPHINXQL_TOK_GROUPBY;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}
	| TOK_WEIGHT '(' ')'
		{
			$$.m_iType = SPHINXQL_TOK_WEIGHT;
			if ( !pParser->SetNewSyntax() )
				YYERROR;
		}
	| json_expr
	| TOK_INTEGER '(' json_expr ')'	{ TRACK_BOUNDS ( $$, $1, $4 ); }
	| TOK_DOUBLE '(' json_expr ')'	{ TRACK_BOUNDS ( $$, $1, $4 ); }
	| TOK_BIGINT '(' json_expr ')'	{ TRACK_BOUNDS ( $$, $1, $4 ); }
	| TOK_FACET '(' ')'
	;

mva_aggr:
	TOK_ANY '(' identcol ')'	{ $$ = $3; $$.m_iType = TOK_ANY; }
	| TOK_ALL '(' identcol ')'	{ $$ = $3; $$.m_iType = TOK_ALL; }
	;

const_int:
	TOK_CONST_INT
		{
			$$.m_iType = TOK_CONST_INT;
			$$.SetValueInt ( $1.GetValueUint(), false );
		}
	| '-' TOK_CONST_INT
		{
			$$.m_iType = TOK_CONST_INT;
			$$.SetValueInt ( $2.GetValueUint(), true );
		}
	;

const_float:
	const_float_unsigned		{ $$.m_iType = TOK_CONST_FLOAT; $$.SetValueFloat ( $1.m_fValue ); }
	| '-' const_float_unsigned	{ $$.m_iType = TOK_CONST_FLOAT; $$.SetValueFloat ( -$2.m_fValue ); }
	;

// fixme! That is non-consequetive behaviour.
// in `select id*.1 a from indexrt where a>.4` - first .1 is part of expression and is passed
// to expr parser. Second .4 is part of the filter - and is parsed immediately here.
const_float_unsigned:
	TOK_CONST_FLOAT			{ $$.m_fValue = $1.m_fValue; }
	| TOK_DOT_NUMBER		{ $$.m_fValue = pParser->ToFloat ($1); }
	;

const_list:
	const_int
		{
			assert ( $$.m_iValues<0 );
        	$$.m_iValues = pParser->AddMvaVec ();
        	auto& dVec = pParser->GetMvaVec ( $$.m_iValues );
			dVec.Add ( { $1.GetValueInt(), $1.GetValueFloat(), false } );
		}
	| const_float
		{
			assert ( $$.m_iValues<0 );
        	$$.m_iValues = pParser->AddMvaVec ();
        	auto& dVec = pParser->GetMvaVec ( $$.m_iValues );
			dVec.Add ( { $1.GetValueInt(), $1.GetValueFloat(), true } );
		}
	| const_list ',' const_int
		{
			auto& dVec = pParser->GetMvaVec ( $$.m_iValues );
			dVec.Add ( { $3.GetValueInt(), $3.GetValueFloat(), false } );
		}
	| const_list ',' const_float
		{
			auto& dVec = pParser->GetMvaVec ( $$.m_iValues );
			dVec.Add ( { $3.GetValueInt(), $3.GetValueFloat(), true } );
		}
	;

string_list:
	TOK_QUOTED_STRING
		{
			assert ( $$.m_iValues<0 );
        	$$.m_iValues = pParser->AddMvaVec ();
        	auto& dVec = pParser->GetMvaVec ( $$.m_iValues );
			dVec.Add ( { $1.GetValueInt(), 0.0f } );
		}
	| string_list ',' TOK_QUOTED_STRING
		{
			auto& dVec = pParser->GetMvaVec ( $$.m_iValues );
			dVec.Add ( { $3.GetValueInt(), 0.0f } );
		}
	;

opt_group_clause:
	// empty
	| TOK_GROUP opt_int TOK_BY group_items_list
	;

opt_int:
	// empty
	| TOK_CONST_INT
		{
			pParser->SetGroupbyLimit ( $1.GetValueInt() );
		}
	;

group_items_list:
	expr_ident
		{
			pParser->AddGroupBy ( $1 );
		}
	| group_items_list ',' expr_ident
		{
			pParser->AddGroupBy ( $3 );
		}
	;

opt_having_clause:
	// empty
	| TOK_HAVING filter_item
		{
			pParser->AddHaving();
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
			pParser->ToString ( pParser->m_pQuery->m_sSortBy, $5 );
		}
	;

opt_order_clause:
	// empty
	| order_clause
	;

order_clause:
	TOK_ORDER TOK_BY order_items_list
		{
			pParser->ToString ( pParser->m_pQuery->m_sOrderBy, $3 );
		}
	| TOK_ORDER TOK_BY TOK_RAND '(' ')'
		{
			pParser->m_pQuery->m_sOrderBy = "@random";
		}
	| TOK_ORDER TOK_BY TOK_COUNT '(' TOK_DISTINCT distinct_ident ')' TOK_ASC
		{
			if ( !pParser->AddDistinctSort ( &$6, &$3, &$7, true ) )
				YYERROR;
		}
	| TOK_ORDER TOK_BY TOK_COUNT '(' TOK_DISTINCT distinct_ident ')' TOK_DESC
		{
			if ( !pParser->AddDistinctSort ( &$6, &$3, &$7, false ) )
				YYERROR;
		}
	;

order_items_list:
	order_item
	| order_items_list ',' order_item	{ TRACK_BOUNDS ( $$, $1, $3 ); }
	;

order_item:
	expr_ident
	| expr_ident TOK_ASC				{ TRACK_BOUNDS ( $$, $1, $2 ); }
	| expr_ident TOK_DESC				{ TRACK_BOUNDS ( $$, $1, $2 ); }
	;

opt_limit_clause:
	// empty
	| limit_clause
	;

limit_clause:
	TOK_LIMIT TOK_CONST_INT
		{
			pParser->SetLimit ( 0, $2.GetValueInt() );
		}
	| TOK_LIMIT '-' TOK_CONST_INT
      	{
      		pParser->SetLimit ( 0, -$3.GetValueInt() );
      	}
	| TOK_LIMIT TOK_CONST_INT ',' TOK_CONST_INT
		{
			pParser->SetLimit ( $2.GetValueInt(), $4.GetValueInt() );
		}
	| TOK_LIMIT TOK_CONST_INT TOK_OFFSET TOK_CONST_INT
		{
			pParser->SetLimit ( $4.GetValueInt(), $2.GetValueInt() );
		}
	;

opt_option_clause:
	// empty
	| option_clause
	;

option_clause:
	option_clause_item
	| option_clause option_clause_item
	;

option_clause_item:
	TOK_OPTION default_option_table_setup option_list
	| TOK_OPTION '(' single_manticore_tablename ')' option_table_setup option_list
	;

default_option_table_setup:
    {
        pParser->SetDefaultTableForOptions();
    }
    ;

option_table_setup:
    {
        if ( !pParser->SetTableForOptions($-1) )
			YYERROR;
    }
    ;

option_list:
	option_item
	| option_list ',' option_item
	;

option_item:
	option_name '=' identcol
		{
			if ( !pParser->AddOption ( $1, $3 ) )
				YYERROR;
		}
	| option_name '=' TOK_CONST_INT
		{
			if ( !pParser->AddOption ( $1, $3 ) )
				YYERROR;
		}
	| option_name '=' '(' named_const_list ')'
		{
			if ( !pParser->AddOption ( $1, pParser->GetNamedVec ( $4.GetValueInt() ) ) )
				YYERROR;
			pParser->FreeNamedVec ( $4.GetValueInt() );
		}
	| option_name '=' identcol '(' TOK_QUOTED_STRING ')'
		{
			if ( !pParser->AddOption ( $1, $3, $5 ) )
				YYERROR;
		}
	| option_name '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddOption ( $1, $3 ) )
				YYERROR;
		}
	;

named_const_list:
	named_const
		{
			$$.SetValueInt ( pParser->AllocNamedVec() );
			pParser->AddConst ( $$.GetValueInt(), $1 );
		}
	| named_const_list ',' named_const
		{
			pParser->AddConst( $$.GetValueInt(), $3 );
		}
	;

named_const:
	identcol '=' const_int
		{
			$$ = $1;
			$$.SetValueInt ( $3.GetValueInt() );
		}
	;

opt_hint_clause:
	// empty
	| TOK_HINT_OPEN hint_list TOK_HINT_CLOSE
	;

hint_list:
	hint_item
	| hint_list hint_item
	;

hint_attr_list:
	json_field
	| hint_attr_list ',' json_field {TRACK_BOUNDS ( $$, $1, $3 );}
	;

hint_item:           
	TOK_HINT_SECONDARY '(' hint_attr_list ')'
		{
			pParser->AddIndexHint ( SecondaryIndexType_e::INDEX, true, $3 );
		}
	| TOK_HINT_NO_SECONDARY '(' hint_attr_list ')'
		{
			pParser->AddIndexHint ( SecondaryIndexType_e::INDEX, false, $3 );
		}
	| TOK_HINT_DOCID '(' hint_attr_list ')'
		{
			pParser->AddIndexHint ( SecondaryIndexType_e::LOOKUP, true, $3 );
		}
	| TOK_HINT_NO_DOCID '(' hint_attr_list ')'
		{
			pParser->AddIndexHint ( SecondaryIndexType_e::LOOKUP, false, $3 );
		}
	| TOK_HINT_COLUMNAR '(' hint_attr_list ')'
		{
			pParser->AddIndexHint ( SecondaryIndexType_e::ANALYZER, true, $3 );
		}
	| TOK_HINT_NO_COLUMNAR '(' hint_attr_list ')'
		{
			pParser->AddIndexHint ( SecondaryIndexType_e::ANALYZER, false, $3 );
		}
	;

time_unit:
	TOK_SECOND
	| TOK_MINUTE
	| TOK_HOUR
	| TOK_DAY
	| TOK_WEEK
	| TOK_MONTH
	| TOK_QUARTER
	| TOK_YEAR
	;

//////////////////////////////////////////////////////////////////////////

expr:
	identcol
	| TOK_ATIDENT				{ if ( !pParser->SetOldSyntax() ) YYERROR; }
	| TOK_CONST_INT
	| TOK_CONST_FLOAT
	| TOK_DOT_NUMBER
	| '-' expr %prec TOK_NEG	{ TRACK_BOUNDS ( $$, $1, $2 ); }
	| TOK_NOT expr				{ TRACK_BOUNDS ( $$, $1, $2 ); }

	| expr '+' expr				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '-' expr				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '*' expr				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '/' expr				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '<' expr				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '>' expr				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '&' expr				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '|' expr				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '%' expr				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr TOK_DIV expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr TOK_MOD expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr TOK_LTE expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr TOK_GTE expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '=' expr				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr TOK_NE expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr TOK_AND expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr TOK_OR expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| '(' expr ')'				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| '{' consthash '}'			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| function
	| json_expr
	| streq
	| json_field TOK_IS TOK_NULL			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| json_field TOK_IS TOK_NOT TOK_NULL	{ TRACK_BOUNDS ( $$, $1, $4 ); }
	| ident TOK_SUBKEY '(' ')'				{ TRACK_BOUNDS ( $$, $1, $4 ); }
	;

accepted_funcs:
	TOK_INTEGER
	| TOK_BIGINT
	| TOK_FLOAT
	| TOK_DOUBLE
	| TOK_REGEX
	| TOK_IN
	| TOK_INTERVAL
	| TOK_SECOND
	| TOK_MINUTE
	| TOK_HOUR
	| TOK_DAY
	| TOK_WEEK
	| TOK_MONTH
	| TOK_QUARTER
	| TOK_YEAR
	;

function:
	TOK_IDENT '(' arglist ')'		{ TRACK_BOUNDS ( $$, $1, $4 ); }
	| json_field TOK_IN '(' arglist ')' { TRACK_BOUNDS ( $$, $1, $5 ); } // handle exception from 'ident' rule
	| TOK_IDENT '(' ')'				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| TOK_QUERY '(' ')'				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| TOK_MIN '(' expr ',' expr ')'	{ TRACK_BOUNDS ( $$, $1, $6 ); } // handle clash with aggregate functions
	| TOK_MAX '(' expr ',' expr ')'	{ TRACK_BOUNDS ( $$, $1, $6 ); }
	| TOK_WEIGHT '(' ')'			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| json_aggr '(' expr TOK_FOR identcol TOK_IN json_field ')' { TRACK_BOUNDS ( $$, $1, $8 ); }
	| TOK_REMAP '(' expr ',' expr ',' '(' arglist ')' ',' '(' arglist ')' ')' { TRACK_BOUNDS ( $$, $1, $14 ); }
	| TOK_RAND '(' ')'				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| TOK_RAND '(' arglist ')' 		{ TRACK_BOUNDS ( $$, $1, $4 ); }
	| TOK_DATE_ADD '(' expr ',' TOK_INTERVAL expr time_unit ')' { TRACK_BOUNDS ( $$, $1, $8 ); }
	| TOK_DATE_SUB '(' expr ',' TOK_INTERVAL expr time_unit ')' { TRACK_BOUNDS ( $$, $1, $8 ); }
	| accepted_funcs '(' arglist ')' { TRACK_BOUNDS ( $$, $1, $4 ); }
	;

arglist:
	arg
	| arglist ',' arg
	;

arg:
	expr
	| TOK_QUOTED_STRING
	;

json_aggr:
	TOK_ANY
	| TOK_ALL
	| TOK_INDEXOF
	;

consthash:
	// empty
	| hash_key '=' hash_val					{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| consthash ',' hash_key '=' hash_val	{ TRACK_BOUNDS ( $$, $1, $5 ); }
	;

hash_key:
	identcol
	| TOK_IN
	| TOK_LIMIT
	;

hash_val:
	const_int
	| identcol
	| TOK_QUOTED_STRING 
	;

//////////////////////////////////////////////////////////////////////////

show_stmt:
	TOK_SHOW show_what opt_option_clause
	;

like_filter:
	// empty
	| TOK_LIKE TOK_QUOTED_STRING		{ pParser->m_pStmt->m_sStringParam = pParser->ToStringUnescape ($2 ); }
	;

show_what:
	TOK_WARNINGS				{ pParser->m_pStmt->m_eStmt = STMT_SHOW_WARNINGS; }
	| TOK_STATUS like_filter	{ pParser->m_pStmt->m_eStmt = STMT_SHOW_STATUS; }
	| TOK_META like_filter		{ pParser->m_pStmt->m_eStmt = STMT_SHOW_META; }
	| TOK_AGENT TOK_STATUS like_filter	{ pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS; }
	| TOK_PROFILE				{ pParser->m_pStmt->m_eStmt = STMT_SHOW_PROFILE; }
	| TOK_PLAN					{ pParser->m_pStmt->m_eStmt = STMT_SHOW_PLAN; }
	| TOK_PLUGINS				{ pParser->m_pStmt->m_eStmt = STMT_SHOW_PLUGINS; }
	| TOK_THREADS				{ pParser->m_pStmt->m_eStmt = STMT_SHOW_THREADS; }
	| TOK_SCROLL				{ pParser->m_pStmt->m_eStmt = STMT_SHOW_SCROLL; }
	| TOK_CREATE TOK_TABLE single_manticore_tablename
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CREATE_TABLE;
			pParser->SetIndex ($3);
		}
	| TOK_AGENT TOK_QUOTED_STRING TOK_STATUS like_filter
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->SetIndex(pParser->ToStringUnescape ( $2 ));
		}
	| TOK_AGENT tablename_with_maybecluster TOK_STATUS like_filter
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_AGENT_STATUS;
			pParser->SetIndex( $2 );
		}
	| index_or_table one_index_opt_chunk TOK_STATUS like_filter
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_STATUS;
			pParser->SetIndex( $2 );
		}
	| index_or_table one_index_opt_chunk TOK_SETTINGS
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_INDEX_SETTINGS;
			pParser->SetIndex( $2 );
		}
	| index_or_table one_index_opt_chunk TOK_INDEXES like_filter 
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLE_INDEXES;
			pParser->SetIndex( $2 );
		}
	| TOK_TABLE TOK_STATUS like_filter
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_FEDERATED_INDEX_STATUS;
		}
	| TOK_COLLATION
		{
       		pParser->m_pStmt->m_eStmt = STMT_SHOW_COLLATION;
		}
	| TOK_CHARACTER TOK_SET
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_CHARACTER_SET;
		}
	| TOK_TABLES like_filter
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES;
		}
	| TOK_TABLES TOK_FROM TOK_SYSTEM like_filter
		{
			pParser->m_pStmt->m_iIntParam = 1;
			pParser->m_pStmt->m_eStmt = STMT_SHOW_TABLES;
		}
	| TOK_DATABASES like_filter
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_DATABASES;
		}
	| global_or_session TOK_VARIABLES like_filter
		{
      		pParser->m_pStmt->m_eStmt = STMT_SHOW_VARIABLES;
		}
	| TOK_SETTINGS
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_SETTINGS;
		}
	| TOK_LOCKS
		{
			pParser->m_pStmt->m_eStmt = STMT_SHOW_LOCKS;
		}
	;

index_or_table:
	TOK_INDEX
	| TOK_TABLE
	;

//////////////////////////////////////////////////////////////////////////

set_stmt:
	TOK_SET ident_for_set_stmt '=' bool_or_integer_value
		{
			pParser->SetLocalStatement ( $2 );
			pParser->m_pStmt->m_iSetValue = $4.GetValueInt();
		}
	| TOK_SET ident_for_set_stmt '=' set_string_value
		{
			pParser->SetLocalStatement ( $2 );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, $4 );
		}
	| TOK_SET ident_for_set_stmt '=' TOK_NULL
		{
			pParser->SetLocalStatement ( $2 );
		}
	| TOK_SET TOK_NAMES ident_or_string_or_num_or_nulls opt_collate { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }
	| TOK_SET sysvar '=' ident_or_string_or_num_or_nulls	{ pParser->m_pStmt->m_eStmt = STMT_DUMMY; }
	| TOK_SET TOK_CHARACTER TOK_SET ident_or_string_or_num_or_nulls { pParser->m_pStmt->m_eStmt = STMT_DUMMY; }
	;

opt_collate:
	// empty
	| TOK_COLLATE ident_or_string_or_num_or_nulls
	;

set_string_value:
	ident
	| TOK_QUOTED_STRING
	;

bool_or_integer_value:
	TOK_TRUE			{ $$.SetValueInt(1); }
	| TOK_FALSE			{ $$.SetValueInt(0); }
	| const_int			{ $$.SetValueInt ( $1.GetValueInt() ); }
	;

ident_or_string_or_num_or_nulls:
	ident_or_string_or_num_or_null
	| ident_or_string_or_num_or_nulls '-' ident_or_string_or_num_or_null
	;

ident_or_string_or_num_or_null:
	ident
	| TOK_NULL
	| TOK_QUOTED_STRING
	| TOK_CONST_INT
	| TOK_CONST_FLOAT
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
	insert_or_replace TOK_INTO one_index_opt_subindex opt_column_list TOK_VALUES insert_rows_list opt_option_clause
	;

insert_or_replace:
	TOK_INSERT		{ pParser->m_pStmt->m_eStmt = STMT_INSERT; }
	| TOK_REPLACE	{ pParser->m_pStmt->m_eStmt = STMT_REPLACE; }
	;

opt_column_list:
	// empty
	| '(' column_list ')'
	;

column_ident:
	identcol
	;

column_list:
	column_ident					{ if ( !pParser->AddSchemaItem ( &$1 ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }
	| column_list ',' column_ident	{ if ( !pParser->AddSchemaItem ( &$3 ) ) { yyerror ( pParser, "unknown field" ); YYERROR; } }
	;

insert_rows_list:
	insert_row
	| insert_rows_list ',' insert_row
	;

insert_row:
	'(' insert_vals_list ')'	{ if ( !pParser->m_pStmt->CheckInsertIntegrity() ) { yyerror ( pParser, "wrong number of values here" ); YYERROR; } }
	;

insert_vals_list:
	insert_val				{ pParser->AddInsval ( pParser->m_pStmt->m_dInsertValues, $1 ); }
	| insert_vals_list ',' insert_val	{ pParser->AddInsval ( pParser->m_pStmt->m_dInsertValues, $3 ); }
	;

insert_val:
	const_int				{ $$.m_iType = TOK_CONST_INT; $$.CopyValueInt ( $1 ); }
	| const_float			{ $$.m_iType = TOK_CONST_FLOAT; $$.m_fValue = $1.m_fValue; }
	| TOK_QUOTED_STRING		{ $$.m_iType = TOK_QUOTED_STRING; $$.m_iStart = $1.m_iStart; $$.m_iEnd = $1.m_iEnd; }
	| '(' const_list ')'	{ $$.m_iType = TOK_CONST_MVA; $$.m_iValues = $2.m_iValues; }
	| '(' ')'				{ $$.m_iType = TOK_CONST_MVA; }
	;

//////////////////////////////////////////////////////////////////////////

delete_from:
	TOK_DELETE { pParser->m_pStmt->m_eStmt = STMT_DELETE; }
		TOK_FROM target_in_delete_from where_clause opt_option_clause
			{
				pParser->GenericStatement ( &$4 );
			}
	;

//////////////////////////////////////////////////////////////////////////

call_proc:
	TOK_CALL ident '(' call_args_list opt_call_opts_list ')'
		{
			pParser->m_pStmt->m_eStmt = STMT_CALL;
			pParser->ToString ( pParser->m_pStmt->m_sCallProc, $2 );
		}
	;

call_args_list:
	call_arg
		{
			pParser->AddInsval ( pParser->m_pStmt->m_dInsertValues, $1 );
		}
	| call_args_list ',' call_arg
		{
			pParser->AddInsval ( pParser->m_pStmt->m_dInsertValues, $3 );
		}
	;

call_arg:
	insert_val
	| '(' const_string_list ')'
		{
			$$.m_iType = TOK_CONST_STRINGS;
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
			pParser->m_pStmt->m_dCallStrings.Add() = pParser->ToStringUnescape ( $1 );
		}
	| const_string_list ',' TOK_QUOTED_STRING
		{
			pParser->m_pStmt->m_dCallStrings.Add() = pParser->ToStringUnescape ( $3 );
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
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), $3 );
			pParser->AddInsval ( pParser->m_pStmt->m_dCallOptValues, $1 );
		}
	;

opt_as:
	// empty
	| TOK_AS
	;

call_opt_name:
	ident
	| TOK_LIMIT
	;

//////////////////////////////////////////////////////////////////////////

describe:
	describe_tok { pParser->m_pStmt->m_eStmt = STMT_DESCRIBE; } one_index_opt_subindex describe_opt like_filter opt_option_clause
	;

describe_opt:
	// empty
	| TOK_TABLE
		{
			pParser->m_pStmt->m_iIntParam = TOK_TABLE; // just a flag that 'TOK_TABLE' is in use
		}
	;

describe_tok:
	TOK_DESCRIBE
	| TOK_DESC
	;


//////////////////////////////////////////////////////////////////////////

update:
	TOK_UPDATE { pParser->m_pStmt->m_eStmt = STMT_UPDATE; }
		target_in_update TOK_SET update_items_list where_clause opt_option_clause opt_hint_clause
			{
				pParser->GenericStatement ( &$3 );
			}
	;

update_items_list:
	update_item
	| update_items_list ',' update_item
	;

update_item:
	identcol '=' const_int
		{
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->AttrUpdate().m_dPool.Add ( (DWORD)$3.GetValueInt() );
			DWORD uHi = (DWORD)( $3.GetValueInt()>>32 );
			if ( uHi )
			{
				pParser->m_pStmt->AttrUpdate().m_dPool.Add ( uHi );
				pParser->AddUpdatedAttr ( $1, SPH_ATTR_BIGINT );
			} else
			{
				pParser->AddUpdatedAttr ( $1, SPH_ATTR_INTEGER );
			}
		}
	| identcol '=' const_float
		{
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->AttrUpdate().m_dPool.Add ( sphF2DW ( $3.m_fValue ) );
			pParser->AddUpdatedAttr ( $1, SPH_ATTR_FLOAT );
		}
	| identcol '=' '(' const_list ')'
		{
			pParser->UpdateMVAAttr ( $1, $4 );
		}
	| identcol '=' '(' ')' // special case () means delete mva
		{
			SqlNode_t tNoValues;
			pParser->UpdateMVAAttr ( $1, tNoValues );
		}
	| json_expr '=' const_int // duplicate ident code (avoiding s/r conflict)
		{
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->AttrUpdate().m_dPool.Add ( (DWORD)$3.GetValueInt() );
			DWORD uHi = (DWORD)( $3.GetValueInt()>>32 );
			if ( uHi )
			{
				pParser->m_pStmt->AttrUpdate().m_dPool.Add ( uHi );
				pParser->AddUpdatedAttr ( $1, SPH_ATTR_BIGINT );
			} else
			{
				pParser->AddUpdatedAttr ( $1, SPH_ATTR_INTEGER );
			}
		}
	| json_expr '=' const_float
		{
			// it is performance-critical to forcibly inline this
			pParser->m_pStmt->AttrUpdate().m_dPool.Add ( sphF2DW ( $3.m_fValue ) );
			pParser->AddUpdatedAttr ( $1, SPH_ATTR_FLOAT );
		}
	| identcol '=' TOK_QUOTED_STRING
		{
			pParser->UpdateStringAttr ( $1, $3 );
		}
	;

//////////////////////////////////////////////////////////////////////////

global_or_session:
	// empty
	| TOK_GLOBAL
		{
			pParser->m_pStmt->m_iIntParam = 0;
		}
	| TOK_SESSION
		{
    		pParser->m_pStmt->m_iIntParam = 1;
    	}
	;

//////////////////////////////////////////////////////////////////////////

optimize_index:
	TOK_OPTIMIZE  { pParser->m_pStmt->m_eStmt = STMT_OPTIMIZE_INDEX; }
		index_or_table single_manticore_tablename opt_option_clause
			{
				pParser->SetIndex( $4 );
			}
	;



//////////////////////////////////////////////////////////////////////////

json_field:
	json_expr
	| ident
	;

json_expr:
	ident subscript				{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }

subscript:
	subkey
	| subscript subkey			{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	;

subkey:
	TOK_SUBKEY
	| TOK_BACKTICKED_SUBKEY
	| TOK_DOT_NUMBER
	| '[' expr ']'				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| '[' TOK_QUOTED_STRING ']'		{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	;

streq:
	expr '=' strval				{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| strval '=' expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr TOK_NE strval			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| strval TOK_NE expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '<' strval			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| strval '<' expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr '>' strval			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| strval '>' expr			{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr TOK_LTE strval		{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| strval TOK_LTE expr		{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| expr TOK_GTE strval		{ TRACK_BOUNDS ( $$, $1, $3 ); }
	| strval TOK_GTE expr		{ TRACK_BOUNDS ( $$, $1, $3 ); }
	;

strval:
	TOK_QUOTED_STRING
	;

//////////////////////////////////////////////////////////////////////////

opt_distinct_item:
	// empty
	| TOK_DISTINCT							{ pParser->AddDistinct ( nullptr ); }
	| TOK_DISTINCT identcol					{ pParser->AddDistinct ( &$2 ); }
	;

opt_facet_by_items_list:
	// empty
	| facet_by facet_items_list
	;

facet_by:
	TOK_BY
		{
			pParser->m_pQuery->m_sFacetBy = pParser->m_pQuery->m_sGroupBy;
			pParser->m_pQuery->m_sGroupBy = "";
			pParser->AddCount ();
		}
	;

facet_item:
	facet_expr opt_alias
	;

facet_expr:
	expr
		{
			pParser->AddItem ( &$1 );
			pParser->AddGroupBy ( $1 );
		}
	;

facet_items_list:
	facet_item
	| facet_items_list ',' facet_item
	;

facet_stmt:
	TOK_FACET facet_items_list opt_facet_by_items_list opt_distinct_item opt_order_clause opt_limit_clause
		{
			if ( !pParser->SetupFacetStmt() )
				YYERROR;
		}
	;

sysfilters:
	TOK_SYSFILTERS filter_expr
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_SYSFILTERS;
		}
	;



explain_query:
	TOK_EXPLAIN   { pParser->m_pStmt->m_eStmt = STMT_EXPLAIN; }
		ident ident TOK_QUOTED_STRING opt_option_clause
			{
				SqlStmt_t & tStmt = *pParser->m_pStmt;
				pParser->ToString ( tStmt.m_sCallProc, $3 );
				pParser->SetIndex( $4 );
				pParser->m_pQuery->m_sQuery = pParser->ToStringUnescape ( $5 );
			}
	;

%%

#if _WIN32
#pragma warning(pop)
#endif
