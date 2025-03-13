%{
#if _WIN32
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#endif

#define TRACK_BOUNDS(_res,_left,_right) \
	_res = _left; \
	if ( _res.m_iStart>0 && pParser->m_pBuf[_res.m_iStart-1]=='`' ) \
		_res.m_iStart--; \
	_res.m_iEnd = _right.m_iEnd; \
	_res.m_iType = 0;
%}

%lex-param		{ DdlParser_c * pParser }
%parse-param	{ DdlParser_c * pParser }
%pure-parser
%error-verbose

%token	TOK_IDENT "identifier"
%token	TOK_TABLEIDENT "tablename"
%token	TOK_CONST_FLOAT "float"
%token	TOK_CONST_INT "integer"
%token	TOK_QUOTED_STRING "string"

%token	TOK_ADD
%token	TOK_ALTER
%token	TOK_AS
%token	TOK_AT
%token	TOK_ATTRIBUTE
%token	TOK_BIGINT
%token	TOK_BIT
%token	TOK_BOOL
%token	TOK_CLUSTER
%token	TOK_COLUMN
%token	TOK_COLUMNAR
%token	TOK_CREATE
%token	TOK_DOUBLE
%token	TOK_DROP
%token	TOK_ENGINE
%token	TOK_EXISTS
%token	TOK_FAST_FETCH
%token	TOK_FLOAT
%token	TOK_FLOAT_VECTOR
%token	TOK_FROM
%token	TOK_FUNCTION
%token	TOK_HASH
%token	TOK_HNSW_EF_CONSTRUCTION
%token	TOK_HNSW_M
%token	TOK_HNSW_SIMILARITY
%token	TOK_IF
%token	TOK_IMPORT
%token	TOK_INDEXED
%token	TOK_INT
%token	TOK_INTEGER
%token	TOK_JOIN
%token	TOK_JSON
%token	TOK_KILLLIST_TARGET
%token	TOK_KNN_DIMS
%token	TOK_KNN_TYPE
%token	TOK_LIKE
%token	TOK_OPTION
%token	TOK_MULTI
%token	TOK_MULTI64
%token	TOK_MODIFY
%token	TOK_NOT
%token	TOK_PLUGIN
%token	TOK_QUANTIZATION
%token	TOK_REBUILD
%token	TOK_RECONFIGURE
%token	TOK_RETURNS
%token	TOK_RTINDEX
%token	TOK_SECONDARY
%token	TOK_SECONDARY_INDEX
%token	TOK_SONAME
%token	TOK_STORED
%token	TOK_STRING
%token	TOK_TABLE
%token	TOK_TEXT
%token	TOK_TIMESTAMP
%token	TOK_TYPE
%token	TOK_UINT
%token	TOK_UPDATE

%%

request:
	statement							{ pParser->PushQuery(); }
	;

statement:
	alter
	| create_table
	| create_table_like
	| drop_table_with_opt
	| create_function
	| drop_function
	| create_plugin
	| drop_plugin
	| create_cluster
	| join_cluster
	| import_table
	;

tablename:
	TOK_TABLEIDENT
	| TOK_MODIFY
	;

ident_without_modify:
	TOK_TABLEIDENT
	| TOK_IDENT
    | TOK_TYPE
    ;

ident:
	ident_without_modify
	| TOK_MODIFY
	;

text_or_string:
	TOK_TEXT		{ $$.m_iType = ( DdlParser_c::FLAG_INDEXED | DdlParser_c::FLAG_STORED ); }
	| TOK_STRING	{ $$.m_iType = DdlParser_c::FLAG_ATTRIBUTE; }
	;

attribute_type:
	TOK_INTEGER 	{ $$.SetValueInt ( SPH_ATTR_INTEGER ); }
	| TOK_BIGINT	{ $$.SetValueInt ( SPH_ATTR_BIGINT ); }
	| TOK_FLOAT		{ $$.SetValueInt ( SPH_ATTR_FLOAT ); }
	| TOK_BOOL		{ $$.SetValueInt ( SPH_ATTR_BOOL ); }
	| TOK_MULTI		{ $$.SetValueInt ( SPH_ATTR_UINT32SET ); }
	| TOK_MULTI64	{ $$.SetValueInt ( SPH_ATTR_INT64SET ); }
	| TOK_JSON		{ $$.SetValueInt ( SPH_ATTR_JSON ); }
	| TOK_INT		{ $$.SetValueInt ( SPH_ATTR_INTEGER ); }
	| TOK_UINT		{ $$.SetValueInt ( SPH_ATTR_INTEGER ); }
	| TOK_TIMESTAMP	{ $$.SetValueInt ( SPH_ATTR_TIMESTAMP ); }
	| TOK_FLOAT_VECTOR { $$.SetValueInt ( SPH_ATTR_FLOAT_VECTOR ); }
	;

list_of_tables:
	| tablename ',' tablename
		{
    		TRACK_BOUNDS ( $$, $1, $3 );
    	}
	| list_of_tables ',' tablename
		{
			TRACK_BOUNDS ( $$, $1, $3 );
		}
	;

table_or_tables:
	tablename
	| list_of_tables
	;
	
//////////////////////////////////////////////////////////////////////////

alter_col_type:
	attribute_type
	| text_or_string 					{ $$.SetValueInt ( SPH_ATTR_STRING ); }
	| text_or_string field_flag_list 	{ $$.SetValueInt ( SPH_ATTR_STRING ); $$.m_iType = $2.m_iType; }
	;

alter:
	TOK_ALTER TOK_TABLE tablename TOK_ADD TOK_COLUMN ident alter_col_type item_option_list
		{
			if ( !pParser->SetupAlterTable ( $3, $6, $7 ) )
			{
			 	yyerror ( pParser, pParser->GetLastError() );
	            YYERROR;
			}
		}
	| TOK_ALTER TOK_TABLE tablename TOK_MODIFY TOK_COLUMN ident alter_col_type item_option_list
		{
			if ( !pParser->SetupAlterTable ( $3, $6, $7, true ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
				YYERROR;
			}
		}
	| TOK_ALTER TOK_TABLE tablename TOK_ADD TOK_COLUMN ident TOK_BIT '(' TOK_CONST_INT ')' item_option_list
		{
			if ( !pParser->SetupAlterTable ( $3, $6, SPH_ATTR_INTEGER, 0, $9.GetValueInt() ) )
			{
			 	yyerror ( pParser, pParser->GetLastError() );
	            YYERROR;
			}
		}
	| TOK_ALTER TOK_TABLE tablename TOK_DROP TOK_COLUMN ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			pParser->ToString ( tStmt.m_sAlterAttr, $6 );
		}
	| TOK_ALTER TOK_RTINDEX tablename TOK_RECONFIGURE
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	| TOK_ALTER TOK_TABLE tablename TOK_RECONFIGURE
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	| TOK_ALTER TOK_TABLE tablename TOK_KILLLIST_TARGET '=' TOK_QUOTED_STRING
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_KLIST_TARGET;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			pParser->ToString ( tStmt.m_sAlterOption, $6 ).Unquote();
		}
	| alter_index_settings_with_options
	| TOK_ALTER TOK_CLUSTER ident TOK_ADD table_or_tables
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CLUSTER_ALTER_ADD;
			pParser->ToString ( tStmt.m_sCluster, $3 );
			pParser->ToString ( tStmt.m_sIndex, $5 );
		}
	| TOK_ALTER TOK_CLUSTER ident TOK_DROP table_or_tables
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CLUSTER_ALTER_DROP;
			pParser->ToString ( tStmt.m_sCluster, $3 );
			pParser->ToString ( tStmt.m_sIndex, $5 );
		}
	| TOK_ALTER TOK_CLUSTER ident TOK_UPDATE tablename
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CLUSTER_ALTER_UPDATE;
			pParser->ToString ( tStmt.m_sCluster, $3 );
			pParser->ToString ( tStmt.m_sSetName, $5 );
		}
	| TOK_ALTER TOK_TABLE tablename TOK_REBUILD TOK_SECONDARY
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_REBUILD_SI;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	;


alter_index_settings:
	TOK_ALTER TOK_TABLE tablename create_table_option_list
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_INDEX_SETTINGS;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	;

alter_index_settings_with_options:
	alter_index_settings opt_option_clause
	;

//////////////////////////////////////////////////////////////////////////

field_flag:
	TOK_INDEXED			{ $$.m_iType = DdlParser_c::FLAG_INDEXED; }
	| TOK_STORED		{ $$.m_iType = DdlParser_c::FLAG_STORED; }
	| TOK_ATTRIBUTE		{ $$.m_iType = DdlParser_c::FLAG_ATTRIBUTE; }
	;

field_flag_list:
	field_flag
	| field_flag_list field_flag { $$.m_iType |= $2.m_iType; }
	;

item_option:
	TOK_ENGINE '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddItemOptionEngine ( $3 ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
	        	YYERROR;
			}
		}
	| TOK_HASH '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddItemOptionHash ( $3 ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
	        	YYERROR;
			}
		}
	| TOK_FAST_FETCH '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddItemOptionFastFetch ( $3 ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
    	    	YYERROR;
			}
		}
	| TOK_KNN_TYPE '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddItemOptionKNNType ( $3 ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
    	    	YYERROR;
			}
		}
	| TOK_KNN_DIMS '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddItemOptionKNNDims ( $3 ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
    	    	YYERROR;
			}
		}
	| TOK_HNSW_SIMILARITY '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddItemOptionHNSWSimilarity ( $3 ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
    	    	YYERROR;
			}
		}
	| TOK_HNSW_M '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddItemOptionHNSWM ( $3 ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
    	    	YYERROR;
			}
		}
	| TOK_HNSW_EF_CONSTRUCTION '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddItemOptionHNSWEfConstruction ( $3 ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
    	    	YYERROR;
			}
		}
	| TOK_QUANTIZATION '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddItemOptionQuantization ( $3 ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
    	    	YYERROR;
			}
		}
	| TOK_SECONDARY_INDEX '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddItemOptionIndexed ( $3 ) )
			{
				yyerror ( pParser, pParser->GetLastError() );
    	    	YYERROR;
			}
		}
	;

item_option_list:
	// empty
	| item_option_list item_option
	;

create_table_item:
	ident alter_col_type item_option_list
	{
		if ( !pParser->AddCreateTableCol ( $1, $2 ) )
		 {
		 	yyerror ( pParser, pParser->GetLastError() );
            YYERROR;
		 }
	}
	| ident item_option_list
	{
		if ( !pParser->AddCreateTableId ( $1 ) )
		{
			yyerror ( pParser, pParser->GetLastError() );
			YYERROR;
		}
	}
	| ident TOK_BIT '(' TOK_CONST_INT ')' item_option_list	{ pParser->AddCreateTableBitCol ( $1, $4.GetValueInt() ); }
	;

create_table_item_list:
	create_table_item
	| create_table_item_list ',' create_table_item
	;

create_table_items:
	// empty
	| '(' create_table_item_list ')'
	;

create_table_option:
	ident_without_modify '=' TOK_QUOTED_STRING			{ pParser->AddCreateTableOption ( $1, $3 ); }
	| TOK_ENGINE '=' TOK_QUOTED_STRING	{ pParser->AddCreateTableOption ( $1, $3 ); }
	;

create_table_option_list:
	// empty
	| create_table_option_list create_table_option
	;

_if_not_exists:
	// empty
	| TOK_IF TOK_NOT TOK_EXISTS			{ pParser->m_pStmt->m_tCreateTable.m_bIfNotExists = true; }
	;

_if_exists:
	// empty
	| TOK_IF TOK_EXISTS			{ pParser->m_pStmt->m_bIfExists = true; }
	;

create_table:
	TOK_CREATE TOK_TABLE _if_not_exists tablename create_table_items create_table_option_list
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_TABLE;
			pParser->ToString ( tStmt.m_sIndex, $4 );
			tStmt.m_sIndex.ToLower();
		}
	;

create_table_like:
	TOK_CREATE TOK_TABLE _if_not_exists tablename TOK_LIKE tablename
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_TABLE_LIKE;
			pParser->ToString ( tStmt.m_sIndex, $4 );
			pParser->ToString ( tStmt.m_tCreateTable.m_sLike, $6 );
			tStmt.m_sIndex.ToLower();
			tStmt.m_tCreateTable.m_sLike.ToLower();
		}
	;

drop_table:
	TOK_DROP TOK_TABLE _if_exists tablename
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_TABLE;
			pParser->ToString ( tStmt.m_sIndex, $4 );
			tStmt.m_sIndex.ToLower();
		}
	;

drop_table_with_opt:
	drop_table opt_option_clause
	;

//////////////////////////////////////////////////////////////////////////

create_function:
	TOK_CREATE TOK_FUNCTION ident TOK_RETURNS udf_type TOK_SONAME TOK_QUOTED_STRING
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, $3 );
			tStmt.m_sUdfLib = pParser->ToStringUnescape ( $7 );
			tStmt.m_eUdfType = (ESphAttr) $5.GetValueInt();
		}
	;

udf_type:
	TOK_INT			{ $$.SetValueInt ( SPH_ATTR_INTEGER ); }
	| TOK_BIGINT	{ $$.SetValueInt ( SPH_ATTR_BIGINT ); }
	| TOK_FLOAT		{ $$.SetValueInt ( SPH_ATTR_FLOAT ); }
	| TOK_STRING	{ $$.SetValueInt ( SPH_ATTR_STRINGPTR ); }
	| TOK_INTEGER	{ $$.SetValueInt ( SPH_ATTR_INTEGER ); }
	;

drop_function:
	TOK_DROP TOK_FUNCTION ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, $3 );
		}
	;

//////////////////////////////////////////////////////////////////////////

create_plugin:
	TOK_CREATE TOK_PLUGIN ident TOK_TYPE TOK_QUOTED_STRING TOK_SONAME TOK_QUOTED_STRING
		{
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_CREATE_PLUGIN;
			pParser->ToString ( s.m_sUdfName, $3 );
			s.m_sStringParam = pParser->ToStringUnescape ( $5 );
			s.m_sUdfLib = pParser->ToStringUnescape ( $7 );
		}
	;

drop_plugin:
	TOK_DROP TOK_PLUGIN ident TOK_TYPE TOK_QUOTED_STRING
		{
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_DROP_PLUGIN;
			pParser->ToString ( s.m_sUdfName, $3 );
			s.m_sStringParam = pParser->ToStringUnescape ( $5 );
		}
	;

//////////////////////////////////////////////////////////////////////////

call_opts_list:
	call_opt
		{
			assert ( pParser->m_pStmt->m_dCallOptNames.GetLength()==1 );
			assert ( pParser->m_pStmt->m_dCallOptValues.GetLength()==1 );
		}
	| call_opts_list ',' call_opt
	;

cluster_opts_list:
	// empty
	| call_opts_list
	;

call_opt:
	insert_val opt_as ident
		{
			pParser->ToString ( pParser->m_pStmt->m_dCallOptNames.Add(), $3 );
			pParser->AddInsval ( pParser->m_pStmt->m_dCallOptValues, $1 );
		}
	;

opt_as:
	// empty
	| TOK_AS
	;

insert_val:
	TOK_CONST_INT			{ $$.m_iType = TOK_CONST_INT; $$.SetValueInt ( $1.GetValueInt() ); }
	| TOK_CONST_FLOAT		{ $$.m_iType = TOK_CONST_FLOAT; $$.m_fValue = $1.m_fValue; }
	| TOK_QUOTED_STRING		{ $$.m_iType = TOK_QUOTED_STRING; $$.m_iStart = $1.m_iStart; $$.m_iEnd = $1.m_iEnd; }
	;

create_cluster:
	TOK_CREATE TOK_CLUSTER ident cluster_opts_list
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CLUSTER_CREATE;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	;

join_cluster:
	TOK_JOIN TOK_CLUSTER ident cluster_opts_list
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_JOIN_CLUSTER;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	| TOK_JOIN TOK_CLUSTER ident TOK_AT TOK_QUOTED_STRING cluster_opts_list
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_JOIN_CLUSTER;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			pParser->JoinClusterAt ( $5 );
		}
	;

import_table:
	TOK_IMPORT TOK_TABLE tablename TOK_FROM TOK_QUOTED_STRING
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_IMPORT_TABLE;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			tStmt.m_sStringParam = pParser->ToStringUnescape ( $5 );
		}
	;

//////////////////////////////////////////////////////////////////////////
// common option clause

opt_option_clause:
	// empty
	| TOK_OPTION option_list
	;

option_list:
	option_item
	| option_list ',' option_item
	;

option_item:
	ident '=' TOK_CONST_INT
		{
			if ( !pParser->AddOption ( $1, $3 ) )
				YYERROR;
		}
	| ident '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddOption ( $1, $3 ) )
				YYERROR;
		}
	;


%%

#if _WIN32
#pragma warning(pop)
#endif
