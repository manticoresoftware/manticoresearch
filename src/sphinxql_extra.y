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

%lex-param		{ class SqlExtraParser_c * pParser }
%parse-param	{ class SqlExtraParser_c * pParser }
%pure-parser
%error-verbose

%token	TOK_CONST_FLOAT
%token	TOK_CONST_INT
%token	TOK_IDENT "identifier"
%token	TOK_TABLE_IDENT "tablename"

%token	TOK_QUOTED_STRING "string"

%token	TOK_CREATE
%token	TOK_DATABASE
%token	TOK_FIELDS
%token	TOK_FLUSH
%token	TOK_FROM
%token	TOK_GLOBAL
%token	TOK_LIKE
%token	TOK_LOCK
%token	TOK_READ
%token	TOK_RELOAD
%token	TOK_SAVEPOINT
%token	TOK_SESSION
%token	TOK_SET
%token	TOK_SHOW
%token	TOK_TABLE
%token	TOK_TABLES
%token	TOK_TRIGGERS
%token	TOK_UNLOCK
%token	TOK_USE
%token	TOK_WITH
%token	TOK_WRITE
%token	TOK_COMMENT

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
	| flush_tables
	| unlock_tables
	| lock_tables
	| use_database
	| savepoint_sp
	| show_fields
	| show_triggers
	| create_database
	| comments
	;

//////////////////////////////////////////////////////////////////////////

// generic ident - like name of the variable, column, etc. but NOT name of the table.
ident:
	TOK_FIELDS | TOK_FLUSH | TOK_FROM | TOK_LOCK | TOK_READ | TOK_RELOAD | TOK_SAVEPOINT
	| TOK_SET | TOK_SHOW | TOK_TABLE | TOK_TABLES | TOK_UNLOCK | TOK_USE | TOK_WITH | TOK_IDENT
	| TOK_TRIGGERS | TOK_LIKE | TOK_CREATE | TOK_DATABASE
	; // no TOK_SESSION, no TOK_GLOBAL

//////////////////////////////////////////////////////////////////////////
param_ident: ident;
savepoint_ident: ident;
database_ident: ident;
table_db_ident:
	ident
	| ident '.' ident { TRACK_BOUNDS ( $$, $1, $3 ); }
	;

table_ident:
	table_db_ident
	| TOK_TABLE_IDENT // covers all like `cluster:table` and `cluster:db.table`
	| ident ':' table_db_ident { TRACK_BOUNDS ( $$, $1, $3 ); }
	;

anysetvalue:
	ident
		{
    		pParser->AddStrval ( pParser->m_pStmt->m_dInsertValues, $1 );
    	}
    | TOK_QUOTED_STRING
		{
			pParser->AddStrval ( pParser->m_pStmt->m_dInsertValues, $1 );
		}
	| const_int
		{
			pParser->AddIntval ( pParser->m_pStmt->m_dInsertValues, $1 );
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

ident_value:
	param_ident '=' anysetvalue
		{
			pParser->AddSetName ( pParser->m_pStmt->m_dInsertSchema, $1 );
		}
	;

global_or_session:
	// empty
	| TOK_GLOBAL
		{
			pParser->SetGlobalScope();
		}
	| TOK_SESSION
		{
			pParser->SetSessionScope();
		}
	;

set_clause:
	global_or_session ident_value
	;

set_clause_list:
	set_clause
	| set_clause_list ',' set_clause
	;

set_global_stmt:
	TOK_SET set_clause_list
		{
			pParser->SetStatement ();
		}
	;

//////////////////////////////////////////////////////////////////////////

flush_tables:
	TOK_FLUSH TOK_TABLES optional_with_read_lock
		{
			pParser->DefaultOk();
		}
	;

optional_with_read_lock:
	// empty
	| TOK_WITH TOK_READ TOK_LOCK
	;

//////////////////////////////////////////////////////////////////////////

unlock_tables:
	TOK_UNLOCK TOK_TABLES
		{
			pParser->DefaultOk();
		}
	;

//////////////////////////////////////////////////////////////////////////

lock_tables:
	TOK_LOCK TOK_TABLES list_locked
		{
			pParser->DefaultOk();
		}
	;

list_locked:
	lock_table
	| list_locked ',' lock_table

lock_table:
	table_ident read_or_write opt_comment
	;

read_or_write:
	TOK_READ
	| TOK_WRITE
	;

opt_comment:
	// empty
	| comment
	;

//////////////////////////////////////////////////////////////////////////

use_database:
	TOK_USE database_ident
		{
			pParser->DefaultOk();
		}
	;

//////////////////////////////////////////////////////////////////////////

savepoint_sp:
	TOK_SAVEPOINT savepoint_ident
		{
			pParser->DefaultOk();
		}
	;

//////////////////////////////////////////////////////////////////////////

show_fields:
	TOK_SHOW TOK_FIELDS TOK_FROM table_ident like_filter
		{
			pParser->SetIndex ($4);
			pParser->m_pStmt->m_eStmt = STMT_DESCRIBE;
			pParser->m_pStmt->m_iIntParam = -2; // indicates we want show fields from
		}
	;

//////////////////////////////////////////////////////////////////////////

show_triggers:
	TOK_SHOW TOK_TRIGGERS like_filter
		{
			pParser->DefaultOk({"Trigger","Event","Table","Statement","Timing","Created"});
		}
	;

like_filter:
	// empty
	| TOK_LIKE TOK_QUOTED_STRING		{ pParser->m_pStmt->m_sStringParam = pParser->ToStringUnescape ($2 ); }
	;


create_database:
	TOK_CREATE TOK_DATABASE database_ident
		{
    		pParser->DefaultOk();
    	}
    ;

comment:
	TOK_COMMENT
		{
    		pParser->Comment($1);
    	}
    ;

comments:
	comments comment
	| comment
	;
%%

#if _WIN32
#pragma warning(pop)
#endif
