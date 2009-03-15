%{
#if USE_WINDOWS
#pragma warning(push,1)
#endif
%}

%lex-param		{ SqlParser_t * pParser }
%parse-param	{ SqlParser_t * pParser }
%pure-parser
%error-verbose

%token	TOK_IDENT
%token	TOK_CONST
%token	TOK_QUOTED_STRING

%token	TOK_AS
%token	TOK_ASC
%token	TOK_AVG
%token	TOK_BETWEEN
%token	TOK_BY
%token	TOK_COUNT
%token	TOK_DESC
%token	TOK_DISTINCT
%token	TOK_FROM
%token	TOK_GROUP
%token	TOK_LIMIT
%token	TOK_IN
%token	TOK_ID
%token	TOK_MATCH
%token	TOK_MAX
%token	TOK_META
%token	TOK_MIN
%token	TOK_OPTION
%token	TOK_ORDER
%token	TOK_SELECT
%token	TOK_SHOW
%token	TOK_STATUS
%token	TOK_SUM
%token	TOK_WARNINGS
%token	TOK_WEIGHT
%token	TOK_WITHIN
%token	TOK_WHERE

%left TOK_AND TOK_OR
%nonassoc TOK_NOT
%left '=' TOK_NE
%left '<' '>' TOK_LTE TOK_GTE
%left '+' '-'
%left '*' '/'
%nonassoc TOK_NEG

%%

statement:
	select_from
	| show_warnings
	| show_status
	| show_meta
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
			pParser->m_eStmt = STMT_SELECT;
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
	| TOK_IDENT '=' TOK_CONST
		{
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( $3.m_iValue );
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		}
	| TOK_IDENT TOK_NE TOK_CONST
		{
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( $3.m_iValue );
			tFilter.m_bExclude = true;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		}
	| TOK_IDENT TOK_IN '(' const_list ')'
		{
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = $4.m_dValues;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		}
	| TOK_IDENT TOK_NOT TOK_IN '(' const_list ')'
		{
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues = $4.m_dValues;
			tFilter.m_bExclude = true;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		}
	| TOK_IDENT TOK_BETWEEN TOK_CONST TOK_AND TOK_CONST
		{
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = $3.m_iValue;
			tFilter.m_uMaxValue = $5.m_iValue;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		}
	| TOK_IDENT '>' TOK_CONST
		{
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = $3.m_iValue + 1;
			tFilter.m_uMaxValue = UINT_MAX;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		}
	| TOK_IDENT '<' TOK_CONST
		{
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = 0;
			tFilter.m_uMaxValue = $3.m_iValue - 1;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		}
	| TOK_IDENT TOK_GTE TOK_CONST
		{
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = $3.m_iValue;
			tFilter.m_uMaxValue = UINT_MAX;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		}
	| TOK_IDENT TOK_LTE TOK_CONST
		{
			CSphFilterSettings tFilter;
			tFilter.m_sAttrName = $1.m_sValue;
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_uMinValue = 0;
			tFilter.m_uMaxValue = $3.m_iValue;
			pParser->m_pQuery->m_dFilters.Add ( tFilter );
		}
	;

const_list:
	TOK_CONST							{ $$.m_dValues.Add ( $1.m_iValue ); }
	| const_list ',' TOK_CONST			{ $$.m_dValues.Add ( $3.m_iValue ); }
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
	TOK_IDENT
	| TOK_IDENT TOK_ASC					{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	| TOK_IDENT TOK_DESC				{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	;

opt_limit_clause:
	// empty
	| limit_clause
	;

limit_clause:
	TOK_LIMIT TOK_CONST
		{
			pParser->m_pQuery->m_iOffset = 0;
			pParser->m_pQuery->m_iLimit = $2.m_iValue;
		}
	| TOK_LIMIT TOK_CONST ',' TOK_CONST
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
	TOK_IDENT '=' TOK_IDENT		{ if ( !pParser->AddOption ( $1, $3 ) ) YYERROR; }
	| TOK_IDENT '=' TOK_CONST	{ if ( !pParser->AddOption ( $1, $3 ) ) YYERROR; }
	;

//////////////////////////////////////////////////////////////////////////

expr:
	TOK_IDENT
	| TOK_CONST
	| '-' expr %prec TOK_NEG	{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	| TOK_NOT expr				{ $$ = $1; $$.m_iEnd = $2.m_iEnd; }
	| expr '+' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '-' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '*' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '/' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '<' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
	| expr '>' expr				{ $$ = $1; $$.m_iEnd = $3.m_iEnd; }
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

show_warnings:
	TOK_SHOW TOK_WARNINGS		{ pParser->m_eStmt = STMT_SHOW_WARNINGS; }
	;

show_status:
	TOK_SHOW TOK_STATUS			{ pParser->m_eStmt = STMT_SHOW_STATUS; }
	;

show_meta:
	TOK_SHOW TOK_META			{ pParser->m_eStmt = STMT_SHOW_META; }
	;

%%

#if USE_WINDOWS
#pragma warning(pop)
#endif
