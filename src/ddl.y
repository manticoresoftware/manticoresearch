%{
#if _WIN32
#pragma warning(push,1)
#pragma warning(disable:4702) // unreachable code
#endif
%}

%lex-param		{ DdlParser_c * pParser }
%parse-param	{ DdlParser_c * pParser }
%pure-parser
%error-verbose

%token	TOK_IDENT
%token	TOK_CONST_FLOAT
%token	TOK_CONST_INT
%token	TOK_QUOTED_STRING
%token	TOK_BAD_NUMERIC

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
%token	TOK_FLOAT
%token	TOK_FROM
%token	TOK_FUNCTION
%token	TOK_IF
%token	TOK_IMPORT
%token	TOK_INDEXED
%token	TOK_INT
%token	TOK_INTEGER
%token	TOK_JOIN
%token	TOK_JSON
%token	TOK_KILLLIST_TARGET
%token	TOK_LIKE
%token	TOK_MULTI
%token	TOK_MULTI64
%token	TOK_NOT
%token	TOK_PLUGIN
%token	TOK_RECONFIGURE
%token	TOK_RETURNS
%token	TOK_RTINDEX
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
	| drop_table
	| create_function
	| drop_function
	| create_plugin
	| drop_plugin
	| create_cluster
	| join_cluster
	| import_table
	;

ident:
	TOK_IDENT
	| TOK_TYPE
	;

text_or_string:
	TOK_TEXT		{ $$.m_iType = ( DdlParser_c::FLAG_INDEXED | DdlParser_c::FLAG_STORED ); }
	| TOK_STRING	{ $$.m_iType = DdlParser_c::FLAG_ATTRIBUTE; }
	;
//////////////////////////////////////////////////////////////////////////

alter_col_type:
	TOK_INTEGER 	{ $$.m_iValue = SPH_ATTR_INTEGER; }
	| TOK_BIGINT	{ $$.m_iValue = SPH_ATTR_BIGINT; }
	| TOK_FLOAT		{ $$.m_iValue = SPH_ATTR_FLOAT; }
	| TOK_BOOL		{ $$.m_iValue = SPH_ATTR_BOOL; }
	| TOK_MULTI		{ $$.m_iValue = SPH_ATTR_UINT32SET; }
	| TOK_MULTI64	{ $$.m_iValue = SPH_ATTR_INT64SET; }
	| TOK_JSON		{ $$.m_iValue = SPH_ATTR_JSON; }
	| TOK_INT		{ $$.m_iValue = SPH_ATTR_INTEGER; }
	| TOK_UINT		{ $$.m_iValue = SPH_ATTR_INTEGER; }
	| TOK_TIMESTAMP	{ $$.m_iValue = SPH_ATTR_TIMESTAMP; }
	| text_or_string 		{ $$.m_iValue = SPH_ATTR_STRING; }
	| text_or_string field_flag_list { $$.m_iValue = SPH_ATTR_STRING; $$.m_iType = $2.m_iType; }
	;

columnar_flag:
	// empty
	| TOK_COLUMNAR	{ pParser->m_pStmt->m_bColumnar = true; }
	;

alter:
	TOK_ALTER TOK_TABLE ident TOK_ADD TOK_COLUMN ident alter_col_type columnar_flag
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_ADD;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			pParser->ToString ( tStmt.m_sAlterAttr, $6 );
			tStmt.m_eAlterColType = (ESphAttr)$7.m_iValue;
			tStmt.m_uFieldFlags = ConvertFlags($7.m_iType);
		}
	| TOK_ALTER TOK_TABLE ident TOK_DROP TOK_COLUMN ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_DROP;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			pParser->ToString ( tStmt.m_sAlterAttr, $6 );
		}
	| TOK_ALTER TOK_RTINDEX ident TOK_RECONFIGURE
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	| TOK_ALTER TOK_TABLE ident TOK_RECONFIGURE
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_RECONFIGURE;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	| TOK_ALTER TOK_TABLE ident TOK_KILLLIST_TARGET '=' TOK_QUOTED_STRING
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_KLIST_TARGET;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			pParser->ToString ( tStmt.m_sAlterOption, $6 ).Unquote();
		}
	| TOK_ALTER TOK_TABLE ident create_table_option_list
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ALTER_INDEX_SETTINGS;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	| TOK_ALTER TOK_CLUSTER ident TOK_ADD ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CLUSTER_ALTER_ADD;
			pParser->ToString ( tStmt.m_sCluster, $3 );
			pParser->ToString ( tStmt.m_sIndex, $5 );
		}
	| TOK_ALTER TOK_CLUSTER ident TOK_DROP ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CLUSTER_ALTER_DROP;
			pParser->ToString ( tStmt.m_sCluster, $3 );
			pParser->ToString ( tStmt.m_sIndex, $5 );
		}
	| TOK_ALTER TOK_CLUSTER ident TOK_UPDATE ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CLUSTER_ALTER_UPDATE;
			pParser->ToString ( tStmt.m_sCluster, $3 );
			pParser->ToString ( tStmt.m_sSetName, $5 );
		}
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

create_table_item:
	ident alter_col_type
	{
		if ( !pParser->AddCreateTableCol ( $1, $2 ) )
		 {
		 	yyerror ( pParser, pParser->GetLastError() );
            YYERROR;
		 }
	 }
	| ident alter_col_type TOK_ENGINE '=' TOK_QUOTED_STRING
	{
		if ( !pParser->AddCreateTableCol ( $1, $2, $5 ) )
		 {
		 	yyerror ( pParser, pParser->GetLastError() );
            YYERROR;
		 }
	}
	| ident TOK_BIT '(' TOK_CONST_INT ')'	{ pParser->AddCreateTableBitCol ( $1, $4.m_iValue ); }
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
	ident '=' TOK_QUOTED_STRING			{ pParser->AddCreateTableOption ( $1, $3 ); }
	| TOK_ENGINE '=' TOK_QUOTED_STRING	{ pParser->AddCreateTableOption ( $1, $3 ); }
	;

create_table_option_list:
	// empty
	| create_table_option_list create_table_option
	;

create_table:
	TOK_CREATE TOK_TABLE ident create_table_items create_table_option_list
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_TABLE;
			tStmt.m_tCreateTable.m_bIfNotExists = false;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			tStmt.m_sIndex.ToLower();
		}
	| TOK_CREATE TOK_TABLE TOK_IF TOK_NOT TOK_EXISTS ident create_table_items create_table_option_list
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_TABLE;
			tStmt.m_tCreateTable.m_bIfNotExists = true;
			pParser->ToString ( tStmt.m_sIndex, $6 );
			tStmt.m_sIndex.ToLower();
		}
	;

create_table_like:
	TOK_CREATE TOK_TABLE ident TOK_LIKE ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_TABLE_LIKE;
			tStmt.m_tCreateTable.m_bIfNotExists = false;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			pParser->ToString ( tStmt.m_tCreateTable.m_sLike, $5 );
			tStmt.m_sIndex.ToLower();
			tStmt.m_tCreateTable.m_sLike.ToLower();
		}
	| TOK_CREATE TOK_TABLE TOK_IF TOK_NOT TOK_EXISTS ident TOK_LIKE ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_TABLE_LIKE;
			tStmt.m_tCreateTable.m_bIfNotExists = true;
			pParser->ToString ( tStmt.m_sIndex, $6 );
			pParser->ToString ( tStmt.m_tCreateTable.m_sLike, $8 );
			tStmt.m_sIndex.ToLower();
			tStmt.m_tCreateTable.m_sLike.ToLower();
		}
	;

drop_table:
	TOK_DROP TOK_TABLE ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_TABLE;
			tStmt.m_bIfExists = false;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			tStmt.m_sIndex.ToLower();
		}
	| TOK_DROP TOK_TABLE TOK_IF TOK_EXISTS ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_DROP_TABLE;
			tStmt.m_bIfExists = true;
			pParser->ToString ( tStmt.m_sIndex, $5 );
			tStmt.m_sIndex.ToLower();
		}
	;

//////////////////////////////////////////////////////////////////////////

create_function:
	TOK_CREATE TOK_FUNCTION ident TOK_RETURNS udf_type TOK_SONAME TOK_QUOTED_STRING
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CREATE_FUNCTION;
			pParser->ToString ( tStmt.m_sUdfName, $3 );
			tStmt.m_sUdfLib = pParser->ToStringUnescape ( $7 );
			tStmt.m_eUdfType = (ESphAttr) $5.m_iValue;
		}
	;

udf_type:
	TOK_INT			{ $$.m_iValue = SPH_ATTR_INTEGER; }
	| TOK_BIGINT	{ $$.m_iValue = SPH_ATTR_BIGINT; }
	| TOK_FLOAT		{ $$.m_iValue = SPH_ATTR_FLOAT; }
	| TOK_STRING	{ $$.m_iValue = SPH_ATTR_STRINGPTR; }
	| TOK_INTEGER	{ $$.m_iValue = SPH_ATTR_INTEGER; }
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
	TOK_CONST_INT			{ $$.m_iType = TOK_CONST_INT; $$.m_iValue = $1.m_iValue; }
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
	TOK_IMPORT TOK_TABLE ident TOK_FROM TOK_QUOTED_STRING
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_IMPORT_TABLE;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			tStmt.m_sStringParam = pParser->ToStringUnescape ( $5 );
		}
	;

%%

#if _WIN32
#pragma warning(pop)
#endif
