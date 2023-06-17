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

%lex-param		{ class SqlSecondParser_c * pParser }
%parse-param	{ class SqlSecondParser_c * pParser }
%pure-parser
%error-verbose

%token	TOK_CONST_FLOAT
%token	TOK_CONST_INT
%token	TOK_IDENT "identifier"
%token	TOK_OFF
%token	TOK_ON
%token	TOK_QUOTED_STRING "string"

%token	TOK_ATTACH
%token	TOK_ATTRIBUTES
%token	TOK_BACKTICKED_SUBKEY
%token	TOK_BAD_NUMERIC
%token	TOK_CLUSTER
%token	TOK_COMMITTED
%token	TOK_COMPRESS
%token	TOK_DELETE
%token	TOK_FLUSH
%token	TOK_FREEZE
%token	TOK_GLOBAL
%token	TOK_HOSTNAMES
%token	TOK_INDEX "index"
%token	TOK_INDEXES "indexes"
%token	TOK_ISOLATION
%token	TOK_KILL
%token	TOK_LEVEL
%token	TOK_LIKE
%token	TOK_LOGS
%token	TOK_OPTION
%token	TOK_QUERY
%token	TOK_RAMCHUNK
%token	TOK_READ
%token	TOK_RECONFIGURE
%token	TOK_REPEATABLE
%token	TOK_RTINDEX "rtindex"
%token	TOK_SERIALIZABLE
%token	TOK_SESSION
%token	TOK_SET
%token	TOK_SYSVAR "@@sysvar"
%token	TOK_TABLE "table"
%token	TOK_TABLES "tables"
%token	TOK_TO
%token	TOK_TRANSACTION
%token	TOK_UNCOMMITTED
%token	TOK_UNFREEZE
%token	TOK_USERVAR "@uservar"
%token	TOK_WAIT
%token	TOK_WITH
%token  TOK_FROM
%token  TOK_PLUGINS
%token  TOK_RELOAD
%token  TOK_SONAME
%token  TOK_TRUNCATE


%{

#define TRACK_BOUNDS(_res,_left,_right) \
	_res = _left; \
	if ( _res.m_iStart>0 && pParser->m_pBuf[_res.m_iStart-1]=='`' ) \
		_res.m_iStart--; \
	_res.m_iEnd = _right.m_iEnd; \
	_res.m_iType = 0;

%}

%%


statement:
	set_global_stmt
	| set_transaction
	| attach_index
	| truncate
	| flush_rtindex
	| flush_ramchunk
	| flush_index
	| flush_hostnames
	| flush_logs
	| reload_plugins
	| reload_index
    | reload_indexes
    | delete_cluster
    | freeze_indexes
    | unfreeze_indexes
  	| kill_connid
	;

//////////////////////////////////////////////////////////////////////////

ident_no_option:
	TOK_ATTACH | TOK_ATTRIBUTES | TOK_CLUSTER | TOK_COMMITTED | TOK_COMPRESS | TOK_FLUSH | TOK_FREEZE | TOK_GLOBAL
	| TOK_HOSTNAMES | TOK_INDEX | TOK_INDEXES | TOK_ISOLATION | TOK_KILL | TOK_LEVEL | TOK_LIKE | TOK_LOGS | TOK_OFF
	| TOK_ON | TOK_QUERY | TOK_RAMCHUNK | TOK_READ | TOK_RECONFIGURE | TOK_REPEATABLE | TOK_DELETE
	| TOK_RTINDEX | TOK_SERIALIZABLE | TOK_SESSION | TOK_SET | TOK_TABLE | TOK_TABLES | TOK_TO
	| TOK_UNCOMMITTED | TOK_UNFREEZE | TOK_WAIT | TOK_WITH | TOK_FROM | TOK_PLUGINS | TOK_RELOAD | TOK_SONAME
	| TOK_TRUNCATE | TOK_IDENT
	;

ident:
	ident_no_option
	| TOK_OPTION
	;

ident_all:
	ident
	| TOK_TRANSACTION
	;

/// id of columns
identcol:
	ident
	| identcol ':' ident {TRACK_BOUNDS ( $$, $1, $3 );}
	;
//////////////////////////////////////////////////////////////////////////

set_string_value:
	ident_all
	| TOK_QUOTED_STRING
	;

index_or_table:
	TOK_INDEX
	| TOK_TABLE
	;

indexes_or_tables:
	TOK_INDEXES
	| TOK_TABLES
	;

/// indexes
only_one_index:
	one_index
		{
			pParser->SetIndex ($1);
		}
	;

one_index:
	ident_all { sphWarning ("%s", "got ident_all"); }
	| ident_all ':' ident_all
		{
			sphWarning ("%s", "got ident_all : ident_all");
			pParser->ToString (pParser->m_pStmt->m_sCluster, $1);
			$$ = $3;
		}
	;

list_of_indexes:
	one_index ',' one_index
		{
    		TRACK_BOUNDS ( $$, $1, $3 );
    	}
	| list_of_indexes ',' one_index
		{
			TRACK_BOUNDS ( $$, $1, $3 );
		}
	;

one_or_more_indexes:
	one_index
		{
			pParser->ToString (pParser->m_pStmt->m_sIndex, $1);
		}
	| list_of_indexes
		{
			pParser->ToString (pParser->m_pStmt->m_sIndex, $1);
        }
	;


//////////////////////////////////////////////////////////////////////////

set_global_stmt:
	TOK_SET TOK_GLOBAL ident '=' '(' const_list ')'
		{
			pParser->SetStatement ( $3, SET_GLOBAL_UVAR );
			pParser->m_pStmt->m_dSetValues = *$6.m_pValues;
		}
	| TOK_SET TOK_GLOBAL ident '=' set_string_value
		{
			pParser->SetStatement ( $3, SET_GLOBAL_SVAR );
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, $5 ).Unquote();
		}
	| TOK_SET TOK_GLOBAL ident '=' const_int
		{
			pParser->SetStatement ( $3, SET_GLOBAL_SVAR );
			pParser->m_pStmt->m_iSetValue = $5.GetValueInt();
		}
	| TOK_SET index_or_table ident_all TOK_GLOBAL ident '=' '(' const_list ')'
		{
			pParser->SetStatement ( $5, SET_INDEX_UVAR );
			pParser->m_pStmt->m_dSetValues = *$8.m_pValues;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, $3 );
		}
	| TOK_SET TOK_CLUSTER ident_all TOK_GLOBAL TOK_QUOTED_STRING '=' set_string_value
		{
			pParser->SetStatement ( $5, SET_CLUSTER_UVAR );
			pParser->ToString ( pParser->m_pStmt->m_sIndex, $3 );
			pParser->ToString ( pParser->m_pStmt->m_sSetName, $5 ).Unquote();
			pParser->ToString ( pParser->m_pStmt->m_sSetValue, $7 ).Unquote();
		}
	| TOK_SET TOK_CLUSTER ident_all TOK_GLOBAL TOK_QUOTED_STRING '=' const_int
		{
			pParser->SetStatement ( $5, SET_CLUSTER_UVAR );
			pParser->ToString ( pParser->m_pStmt->m_sIndex, $3 );
			pParser->ToString ( pParser->m_pStmt->m_sSetName, $5 ).Unquote();
			pParser->m_pStmt->m_sSetValue.SetSprintf ( INT64_FMT, $7.GetValueInt() );
		}
	;

const_list:
	const_int
		{
			assert ( !$$.m_pValues );
			$$.m_pValues = new RefcountedVector_c<SphAttr_t> ();
			$$.m_pValues->Add ( $1.GetValueInt() );
		}
	| const_list ',' const_int
		{
			$$.m_pValues->Add ( $3.GetValueInt() );
		}
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

set_transaction:
	TOK_SET global_or_session TOK_TRANSACTION TOK_ISOLATION TOK_LEVEL isolation_level
		{
			pParser->m_pStmt->m_eStmt = STMT_DUMMY;
		}
	;

isolation_level:
	TOK_READ TOK_UNCOMMITTED
	| TOK_READ TOK_COMMITTED
	| TOK_REPEATABLE TOK_READ
	| TOK_SERIALIZABLE
	;

//////////////////////////////////////////////////////////////////////////

truncate:
	TOK_TRUNCATE rtindex only_one_index opt_with_reconfigure
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_TRUNCATE_RTINDEX;
		}
	;

rtindex:
	TOK_RTINDEX
	| TOK_TABLE
	;

opt_with_reconfigure:
	// empty
	| TOK_WITH TOK_RECONFIGURE
		{
			pParser->m_pStmt->m_iIntParam = 1;
		}
	;

////////////////////////////////////////////////////////////

attach_index:
	TOK_ATTACH index_or_table ident_all TOK_TO rtindex ident_all opt_with_truncate
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_ATTACH_INDEX;
			pParser->ToString ( tStmt.m_sIndex, $3 );
			pParser->ToString ( tStmt.m_sStringParam, $6 );
		}
	;

opt_with_truncate:
	// empty
	| TOK_WITH TOK_TRUNCATE
		{
			pParser->m_pStmt->m_iIntParam = 1;
		}
	;
//////////////////////////////////////////////////////////////////////////

flush_rtindex:
	TOK_FLUSH rtindex ident_all
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RTINDEX;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	;

flush_ramchunk:
	TOK_FLUSH TOK_RAMCHUNK ident_all
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_RAMCHUNK;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	;

flush_index:
	TOK_FLUSH TOK_ATTRIBUTES
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_INDEX;
		}
	;

flush_hostnames:
	TOK_FLUSH TOK_HOSTNAMES
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_HOSTNAMES;
		}
	;

flush_logs:
	TOK_FLUSH TOK_LOGS
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_FLUSH_LOGS;
		}
	;

//////////////////////////////////////////////////////////////////////////

reload_plugins:
	TOK_RELOAD TOK_PLUGINS TOK_FROM TOK_SONAME TOK_QUOTED_STRING
		{
			SqlStmt_t & s = *pParser->m_pStmt;
			s.m_eStmt = STMT_RELOAD_PLUGINS;
			s.m_sUdfLib = pParser->ToStringUnescape ( $5 );
		}
	;

//////////////////////////////////////////////////////////////////////////

opt_reload_index_from:
	// empty
	| TOK_FROM TOK_QUOTED_STRING
		{
			pParser->m_pStmt->m_sStringParam = pParser->ToStringUnescape ( $2 );
		}
	;

reload_index:
	TOK_RELOAD index_or_table ident_all
		{
			pParser->m_pStmt->m_eStmt = STMT_RELOAD_INDEX;
			pParser->ToString ( pParser->m_pStmt->m_sIndex, $3);
		} opt_reload_index_from opt_option_clause
	;

reload_indexes:
	TOK_RELOAD indexes_or_tables
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_RELOAD_INDEXES;
		}
	;

delete_cluster:
	TOK_DELETE TOK_CLUSTER ident
		{
			SqlStmt_t & tStmt = *pParser->m_pStmt;
			tStmt.m_eStmt = STMT_CLUSTER_DELETE;
			pParser->ToString ( tStmt.m_sIndex, $3 );
		}
	;

freeze_indexes:
	TOK_FREEZE one_or_more_indexes
		{
			pParser->m_pStmt->m_eStmt = STMT_FREEZE;
		}
	;

unfreeze_indexes:
	TOK_UNFREEZE one_or_more_indexes
		{
			pParser->m_pStmt->m_eStmt = STMT_UNFREEZE;
		}
	;

opt_word_query:
	// empty
	| TOK_QUERY
	;

kill_connid:
	TOK_KILL opt_word_query TOK_CONST_INT
		{
			pParser->m_pStmt->m_eStmt = STMT_KILL;
			pParser->m_pStmt->m_iIntParam = $3.GetValueInt();
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
	ident_no_option '=' identcol
		{
			if ( !pParser->AddOption ( $1, $3 ) )
				YYERROR;
		}
	| ident_no_option '=' TOK_CONST_INT
		{
			if ( !pParser->AddOption ( $1, $3 ) )
				YYERROR;
		}
	| ident_no_option '=' TOK_QUOTED_STRING
		{
			if ( !pParser->AddOption ( $1, $3 ) )
				YYERROR;
		}
	;


%%

#if _WIN32
#pragma warning(pop)
#endif
