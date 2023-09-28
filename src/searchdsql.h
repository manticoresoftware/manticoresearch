//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _searchdsql_
#define _searchdsql_

#include "sphinx.h"

/// refcounted vector
template < typename T >
class RefcountedVector_c : public CSphVector<T>, public ISphRefcountedMT
{};

using AttrValues_p = CSphRefcountedPtr < RefcountedVector_c<SphAttr_t> >;


/// parser view on a generic node
/// CAUTION, nodes get copied in the parser all the time, must keep assignment slim
struct SqlNode_t
{
	int						m_iStart = 0;	///< first byte relative to m_pBuf, inclusive
	int						m_iEnd = 0;		///< last byte relative to m_pBuf, exclusive! thus length = end - start
	int						m_iType = 0;	///< TOK_xxx type for insert values; SPHINXQL_TOK_xxx code for special idents
	float					m_fValue = 0.0;
	AttrValues_p			m_pValues { nullptr };	///< filter values vector (FIXME? replace with numeric handles into parser state?)
	uint64_t				m_uValue = 0;
	int						m_iParsedOp = -1;
	bool					m_bNegative = false;	// this flag means that '-' was explicitly specified before the integer const

							SqlNode_t() = default;

	void					SetValueInt ( int64_t iValue );
	void					SetValueInt ( uint64_t uValue, bool bNegative );
	int64_t					GetValueInt() const;
	uint64_t				GetValueUint() const;
	void					CopyValueInt ( const SqlNode_t & tRhs );
};


/// types of string-list filters.
enum class StrList_e
{
	// string matching: assume attr is a whole solid string
	// attr MUST match any of variants provided, assuming collation applied
	STR_IN,

	// tags matching: assume attr is string of space-separated tags, no collation
	// any separate tag of attr MUST match any of variants provided
	STR_ANY,	/// 'hello world' OP ('hello', 'foo') true, OP ('foo', 'fee' ) false

	// every separate tag of attr MUST match any of variants provided
	STR_ALL    /// 'hello world' OP ('world', 'hello') true, OP ('a','world','hello') false
};


/// magic codes passed via SqlNode_t::m_iStart to handle certain special tokens
/// for instance, to fixup "count(*)" as "@count" easily
enum
{
	SPHINXQL_TOK_COUNT		= -1,
	SPHINXQL_TOK_GROUPBY	= -2,
	SPHINXQL_TOK_WEIGHT		= -3
};


enum SqlStmt_e
{
	STMT_PARSE_ERROR = 0,
	STMT_DUMMY,

	STMT_SELECT,
	STMT_INSERT,
	STMT_REPLACE,
	STMT_DELETE,
	STMT_SHOW_WARNINGS,
	STMT_SHOW_STATUS,
	STMT_SHOW_META,
	STMT_SET,
	STMT_BEGIN,
	STMT_COMMIT,
	STMT_ROLLBACK,
	STMT_CALL, // check.pl STMT_CALL_SNIPPETS STMT_CALL_KEYWORDS
	STMT_DESCRIBE,
	STMT_SHOW_TABLES,
	STMT_CREATE_TABLE,
	STMT_CREATE_TABLE_LIKE,
	STMT_DROP_TABLE,
	STMT_SHOW_CREATE_TABLE,
	STMT_UPDATE,
	STMT_CREATE_FUNCTION,
	STMT_DROP_FUNCTION,
	STMT_ATTACH_INDEX,
	STMT_FLUSH_RTINDEX,
	STMT_FLUSH_RAMCHUNK,
	STMT_SHOW_VARIABLES,
	STMT_TRUNCATE_RTINDEX,
	STMT_SELECT_COLUMNS, // was STMT_SELECT_SYSVAR
	STMT_SHOW_COLLATION,
	STMT_SHOW_CHARACTER_SET,
	STMT_OPTIMIZE_INDEX,
	STMT_SHOW_AGENT_STATUS,
	STMT_SHOW_INDEX_STATUS,
	STMT_SHOW_FEDERATED_INDEX_STATUS,
	STMT_SHOW_PROFILE,
	STMT_ALTER_ADD,
	STMT_ALTER_DROP,
	STMT_ALTER_MODIFY,
	STMT_SHOW_PLAN,
//	STMT_SELECT_DUAL, // deprecated to STMT_SELECT_COLUMNS as more general
	STMT_SHOW_DATABASES,
	STMT_CREATE_PLUGIN,
	STMT_DROP_PLUGIN,
	STMT_SHOW_PLUGINS,
	STMT_SHOW_THREADS,
	STMT_FACET,
	STMT_ALTER_RECONFIGURE,
	STMT_SHOW_INDEX_SETTINGS,
	STMT_FLUSH_INDEX,
	STMT_RELOAD_PLUGINS,
	STMT_RELOAD_INDEX,
	STMT_FLUSH_HOSTNAMES,
	STMT_FLUSH_LOGS,
	STMT_RELOAD_INDEXES,
	STMT_SYSFILTERS,
	STMT_DEBUG,
	STMT_ALTER_KLIST_TARGET,
	STMT_ALTER_INDEX_SETTINGS,
	STMT_JOIN_CLUSTER,
	STMT_CLUSTER_CREATE,
	STMT_CLUSTER_DELETE,
	STMT_CLUSTER_ALTER_ADD,
	STMT_CLUSTER_ALTER_DROP,
	STMT_CLUSTER_ALTER_UPDATE,
	STMT_EXPLAIN,
	STMT_IMPORT_TABLE,
	STMT_FREEZE,
	STMT_UNFREEZE,
	STMT_SHOW_SETTINGS,
	STMT_ALTER_REBUILD_SI,
	STMT_KILL,

	STMT_TOTAL
};


enum SqlSet_e
{
	SET_LOCAL,
	SET_GLOBAL_UVAR,
	SET_GLOBAL_SVAR,
	SET_INDEX_UVAR,
	SET_CLUSTER_UVAR,
	SET_EXTRA
};


/// insert value
struct SqlInsert_t
{
	// some internal tokens for bison grammar parser.
	// originaly we fetched values from parser itself, but it is more convenient to push own values instead.
	// in order to make it most seamless way, let's follow this manual:
	// To add new value XXX (if necessary) look into generated bissphinxql.h for TOK_XXX value,
	// then add the number BOTH into sphinxql.y (to fix this value forever), and into this enum (without TOK_ prefix),
	// as: for TOK_SYSVAR which is now 268 - change '%token TOK_SYSVAR' to '%token TOK_SYSVAR 268' in bison file,
	// then add SYSVAR = 268 here.
	enum Tokens_e
	{
		CONST_INT = 260,
		CONST_FLOAT = 261,
		CONST_MVA = 262,
		QUOTED_STRING = 263,
		CONST_STRINGS = 269,
		TABLE = 378,
	};

	int						m_iType = 0;
	float					m_fVal = 0.0;
	CSphString				m_sVal;		// OPTIMIZE? use char* and point to node?
	AttrValues_p			m_pVals;

	void					SetValueInt ( uint64_t uValue, bool bNegative );
	void					SetValueInt ( int64_t iValue );
	int64_t					GetValueInt() const;
	uint64_t				GetValueUint() const;
	bool					IsNegativeInt() const { return m_bNegative; }
	void					CopyValueInt ( const SqlNode_t & tRhs );

private:
	uint64_t				m_uValue = 0;
	bool					m_bNegative = false;
};

namespace DebugCmd { struct DebugCommand_t; }

/// parsing result
/// one day, we will start subclassing this
struct SqlStmt_t
{
	SqlStmt_e				m_eStmt = STMT_PARSE_ERROR;
	int						m_iRowsAffected = 0;
	const char *			m_sStmt = nullptr; // for error reporting

											   // SELECT specific
	CSphQuery				m_tQuery;
	std::unique_ptr<ISphTableFunc>			m_pTableFunc;

	CSphString				m_sTableFunc;
	StrVec_t				m_dTableFuncArgs;

	// used by INSERT, DELETE, CALL, DESC, ATTACH, ALTER, RELOAD INDEX
	CSphString				m_sIndex;
	CSphString				m_sCluster;
	bool					m_bClusterUpdateNodes = false;

	// INSERT (and CALL) specific
	CSphVector<SqlInsert_t>	m_dInsertValues; // reused by CALL
	StrVec_t				m_dInsertSchema;
	int						m_iSchemaSz = 0;

	// SET specific
	CSphString				m_sSetName;		// reused by ATTACH
	SqlSet_e				m_eSet = SET_LOCAL;
	int64_t					m_iSetValue = 0;
	CSphString				m_sSetValue;
	CSphVector<SphAttr_t>	m_dSetValues;

	// CALL specific
	CSphString				m_sCallProc;
	StrVec_t				m_dCallOptNames;
	CSphVector<SqlInsert_t>	m_dCallOptValues;
	StrVec_t				m_dCallStrings;

	// UPDATE specific
private:
	mutable AttrUpdateSharedPtr_t	m_pUpdate { nullptr }; // made private for lazy initialization
public:
	int						m_iListStart = -1; // < the position of start and end of index's definition in original query.
	int						m_iListEnd = -1;

	// CREATE/DROP FUNCTION, INSTALL PLUGIN specific
	CSphString				m_sUdfName; // FIXME! move to arg1?
	CSphString				m_sUdfLib;
	ESphAttr				m_eUdfType = SPH_ATTR_NONE;

	// ALTER specific
	CSphString				m_sAlterAttr;
	CSphString				m_sAlterOption;
	ESphAttr				m_eAlterColType = SPH_ATTR_NONE;
	AttrEngine_e			m_eEngine = AttrEngine_e::DEFAULT;
	DWORD					m_uFieldFlags = 0;
	DWORD					m_uAttrFlags = 0;
	int						m_iBits = -1;

	// CREATE TABLE specific
	CreateTableSettings_t	m_tCreateTable;

	// DROP TABLE specific
	bool					m_bIfExists = false;

	// SHOW THREADS specific
	int						m_iThreadsCols = -1;
	CSphString				m_sThreadFormat;

	// generic parameter, different meanings in different statements
	// filter pattern in DESCRIBE, SHOW TABLES / META / VARIABLES
	// target index name in ATTACH
	// token filter options in INSERT
	// plugin type in INSTALL PLUGIN
	// path in RELOAD INDEX
	CSphString				m_sStringParam;

	// generic integer parameter, used in SHOW SETTINGS, default value -1
	// for opt_scope TOK_GLOBAL = 0, TOK_SESSION = 1.
	int						m_iIntParam = -1;

	bool					m_bJson = false;
	CSphString				m_sEndpoint;

	CSphVector<CSphString>	m_dStringSubkeys;
	CSphVector<int64_t>		m_dIntSubkeys;

	std::unique_ptr<DebugCmd::DebugCommand_t> m_pDebugCmd;

	SqlStmt_t ();
	~SqlStmt_t();

	SqlStmt_t ( SqlStmt_t&& ) = default;
	SqlStmt_t& operator= ( SqlStmt_t&& ) = default;

	bool AddSchemaItem ( const char * psName );
	// check if the number of fields which would be inserted is in accordance to the given schema
	bool CheckInsertIntegrity();

	CSphAttrUpdate& AttrUpdate() const
	{
		if ( !m_pUpdate )
			m_pUpdate =	new CSphAttrUpdate;
		return *m_pUpdate;
	}

	AttrUpdateSharedPtr_t AttrUpdatePtr() const
	{
		if ( !m_pUpdate )
			m_pUpdate = new CSphAttrUpdate;
		return m_pUpdate;
	}
};

enum class Option_e : BYTE;

class SqlParserTraits_c : ISphNoncopyable
{
	bool m_bWrongParserSyntaxError = false;

public:
	const char *	m_pBuf;
	CSphString *	m_pParseError;
	void *			m_pScanner = nullptr;
	const char *	m_pLastTokenStart = nullptr;
	CSphQuery *		m_pQuery = nullptr;
	SqlStmt_t *		m_pStmt = nullptr;
	CSphString		m_sErrorHeader = "PER:";

	void			PushQuery();
	CSphString &	ToString ( CSphString & sRes, const SqlNode_t & tNode ) const;
	CSphString		ToStringUnescape ( const SqlNode_t & tNode ) const;
	void			ProcessParsingError ( const char* szMessage );
	bool 			IsWrongSyntaxError() const noexcept;

	bool			AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue );
	bool			AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue, const SqlNode_t & sArg );
	bool			AddOption ( const SqlNode_t & tIdent, CSphVector<CSphNamedInt> & dNamed );
	void			DefaultOk ( std::initializer_list<const char*> sList = {} );
	void			SetIndex ( const SqlNode_t& tNode ) const;
	void			SetIndex ( const CSphString& sIndex ) const;
	void 			AddComment ( const SqlNode_t* tNode ) const;

protected:
					SqlParserTraits_c ( CSphVector<SqlStmt_t> &	dStmt, const char* szQuery, CSphString* pError );
	bool			CheckInteger ( const CSphString& sOpt, const CSphString& sVal ) const;
	virtual bool	CheckOption ( Option_e eOption ) const;

	CSphVector<SqlStmt_t> &	m_dStmt;
};


bool	sphParseSqlQuery ( Str_t sQuery, CSphVector<SqlStmt_t> & dStmt, CSphString & sError, ESphCollation eCollation );
bool	PercolateParseFilters ( const char * sFilters, ESphCollation eCollation, const CSphSchema & tSchema, CSphVector<CSphFilterSettings> & dFilters, CSphVector<FilterTreeItem_t> & dFilterTree, CSphString & sError );
void	SqlParser_SplitClusterIndex ( CSphString & sIndex, CSphString * pCluster );
void	InitParserOption();

enum class AddOption_e
{
	NOT_FOUND,
	ADDED,
	FAILED
};

AddOption_e AddOption ( CSphQuery & tQuery, const CSphString & sOpt, const CSphString & sVal, const std::function<CSphString ()> & fnGetUnescaped, SqlStmt_e eStmt, CSphString & sError );
AddOption_e AddOption ( CSphQuery & tQuery, const CSphString & sOpt, const CSphString & sValue, int64_t iValue, SqlStmt_e eStmt, CSphString & sError );
AddOption_e AddOption ( CSphQuery & tQuery, const CSphString & sOpt, CSphVector<CSphNamedInt> & dNamed, SqlStmt_e eStmt, CSphString & sError );
AddOption_e AddOptionRanker ( CSphQuery & tQuery, const CSphString & sOpt, const CSphString & sVal, const std::function<CSphString ()> & fnGetUnescaped, SqlStmt_e eStmt, CSphString & sError );

enum class ParseResult_e { PARSE_OK, PARSE_ERROR, PARSE_SYNTAX_ERROR };

struct ParsedOption_t
{
	int64_t m_iValue = 0;
	float m_fValue = 0.0;
	bool m_bValue = false;
	CSphString m_sValue;
};


#endif // _searchdsql_
