//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
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
	int64_t					m_iValue = 0;
	int						m_iType = 0;	///< TOK_xxx type for insert values; SPHINXQL_TOK_xxx code for special idents
	float					m_fValue = 0.0;
	AttrValues_p			m_pValues { nullptr };	///< filter values vector (FIXME? replace with numeric handles into parser state?)
	int						m_iParsedOp = -1;

	SqlNode_t() = default;

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
	STMT_SELECT_SYSVAR,
	STMT_SHOW_COLLATION,
	STMT_SHOW_CHARACTER_SET,
	STMT_OPTIMIZE_INDEX,
	STMT_SHOW_AGENT_STATUS,
	STMT_SHOW_INDEX_STATUS,
	STMT_SHOW_PROFILE,
	STMT_ALTER_ADD,
	STMT_ALTER_DROP,
	STMT_SHOW_PLAN,
	STMT_SELECT_DUAL,
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

	STMT_TOTAL
};


enum SqlSet_e
{
	SET_LOCAL,
	SET_GLOBAL_UVAR,
	SET_GLOBAL_SVAR,
	SET_INDEX_UVAR,
	SET_CLUSTER_UVAR
};


/// insert value
struct SqlInsert_t
{
	int						m_iType = 0;
	CSphString				m_sVal;		// OPTIMIZE? use char* and point to node?
	int64_t					m_iVal = 0;
	float					m_fVal = 0.0;
	AttrValues_p			m_pVals;
};


/// parsing result
/// one day, we will start subclassing this
struct SqlStmt_t
{
	SqlStmt_e				m_eStmt = STMT_PARSE_ERROR;
	int						m_iRowsAffected = 0;
	const char *			m_sStmt = nullptr; // for error reporting

											   // SELECT specific
	CSphQuery				m_tQuery;
	ISphTableFunc *			m_pTableFunc = nullptr;

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
	//	bool					m_bSetNull = false; // not(yet) used

	// CALL specific
	CSphString				m_sCallProc;
	StrVec_t				m_dCallOptNames;
	CSphVector<SqlInsert_t>	m_dCallOptValues;
	StrVec_t				m_dCallStrings;

	// UPDATE specific
	CSphAttrUpdate			m_tUpdate;
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

	// CREATE TABLE specific
	CreateTableSettings_t	m_tCreateTable;

	// DROP TABLE specific
	bool					m_bIfExists = false;

	// SHOW THREADS specific
	int						m_iThreadsCols = 0;
	CSphString				m_sThreadFormat;

	// generic parameter, different meanings in different statements
	// filter pattern in DESCRIBE, SHOW TABLES / META / VARIABLES
	// target index name in ATTACH
	// token filter options in INSERT
	// plugin type in INSTALL PLUGIN
	// path in RELOAD INDEX
	CSphString				m_sStringParam;

	// generic integer parameter, used in SHOW SETTINGS, default value -1
	int						m_iIntParam = -1;

	bool					m_bJson = false;
	CSphString				m_sEndpoint;

	SqlStmt_t ();
	~SqlStmt_t();

	bool AddSchemaItem ( const char * psName );
	// check if the number of fields which would be inserted is in accordance to the given schema
	bool CheckInsertIntegrity();
};


class SqlParserTraits_c : ISphNoncopyable
{
public:
	void *			m_pScanner = nullptr;
	const char *	m_pBuf = nullptr;
	const char *	m_pLastTokenStart = nullptr;
	CSphString *	m_pParseError = nullptr;
	CSphQuery *		m_pQuery = nullptr;
	SqlStmt_t *		m_pStmt = nullptr;
	CSphString		m_sErrorHeader = "sphinxql:";

	void			PushQuery();
	CSphString &	ToString ( CSphString & sRes, const SqlNode_t & tNode ) const;
	CSphString		ToStringUnescape ( const SqlNode_t & tNode ) const;

protected:
					SqlParserTraits_c ( CSphVector<SqlStmt_t> &	dStmt );

	CSphVector<SqlStmt_t> &	m_dStmt;
};


class SqlParser_c : public SqlParserTraits_c
{
public:
	ESphCollation	m_eCollation;

	CSphVector<FilterTreeItem_t> m_dFilterTree;
	CSphVector<int>	m_dFiltersPerStmt;
	bool			m_bGotFilterOr = false;

public:
					SqlParser_c ( CSphVector<SqlStmt_t> & dStmt, ESphCollation eCollation );

	void			PushQuery ();

	bool			AddOption ( const SqlNode_t & tIdent );
	bool			AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue );
	bool			AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue, const SqlNode_t & sArg );
	bool			AddOption ( const SqlNode_t & tIdent, CSphVector<CSphNamedInt> & dNamed );
	bool			AddInsertOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue );
	void			AddIndexHint ( IndexHint_e eHint, const SqlNode_t & tValue );
	void			AddItem ( SqlNode_t * pExpr, ESphAggrFunc eFunc=SPH_AGGR_NONE, SqlNode_t * pStart=NULL, SqlNode_t * pEnd=NULL );
	bool			AddItem ( const char * pToken, SqlNode_t * pStart=NULL, SqlNode_t * pEnd=NULL );
	bool			AddCount ();
	void			AliasLastItem ( SqlNode_t * pAlias );
	void			AddInsval ( CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode );

	/// called on transition from an outer select to inner select
	void			ResetSelect();

	/// called every time we capture a select list item
	/// (i think there should be a simpler way to track these though)
	void			SetSelect ( SqlNode_t * pStart, SqlNode_t * pEnd=NULL );
	bool			AddSchemaItem ( SqlNode_t * pNode );
	bool			SetMatch ( const SqlNode_t & tValue );
	void			AddConst ( int iList, const SqlNode_t& tValue );
	void			SetStatement ( const SqlNode_t & tName, SqlSet_e eSet );
	bool			AddFloatRangeFilter ( const SqlNode_t & tAttr, float fMin, float fMax, bool bHasEqual, bool bExclude=false );
	bool			AddIntRangeFilter ( const SqlNode_t & tAttr, int64_t iMin, int64_t iMax, bool bExclude );
	bool			AddIntFilterGreater ( const SqlNode_t & tAttr, int64_t iVal, bool bHasEqual );
	bool			AddIntFilterLesser ( const SqlNode_t & tAttr, int64_t iVal, bool bHasEqual );
	bool			AddUservarFilter ( const SqlNode_t & tCol, const SqlNode_t & tVar, bool bExclude );
	void			AddGroupBy ( const SqlNode_t & tGroupBy );
	bool			AddDistinct ( SqlNode_t * pNewExpr, SqlNode_t * pStart, SqlNode_t * pEnd );
	CSphFilterSettings * AddFilter ( const SqlNode_t & tCol, ESphFilter eType );
	bool			AddStringFilter ( const SqlNode_t & tCol, const SqlNode_t & tVal, bool bExclude );
	CSphFilterSettings * AddValuesFilter ( const SqlNode_t & tCol ) { return AddFilter ( tCol, SPH_FILTER_VALUES ); }
	bool			AddStringListFilter ( const SqlNode_t & tCol, SqlNode_t & tVal, StrList_e eType, bool bInverse=false );
	bool			AddNullFilter ( const SqlNode_t & tCol, bool bEqualsNull );
	void			AddHaving ();

	void			FilterGroup ( SqlNode_t & tNode, SqlNode_t & tExpr );
	void			FilterOr ( SqlNode_t & tNode, const SqlNode_t & tLeft, const SqlNode_t & tRight );
	void			FilterAnd ( SqlNode_t & tNode, const SqlNode_t & tLeft, const SqlNode_t & tRight );
	void			SetOp ( SqlNode_t & tNode );

	bool			SetOldSyntax();
	bool			SetNewSyntax();
	bool			IsGoodSyntax();
	bool			IsDeprecatedSyntax() const;

	int				AllocNamedVec ();
	CSphVector<CSphNamedInt> & GetNamedVec ( int iIndex );
	void			FreeNamedVec ( int iIndex );
	bool			UpdateStatement ( SqlNode_t * pNode );
	bool			DeleteStatement ( SqlNode_t * pNode );

	void			AddUpdatedAttr ( const SqlNode_t & tName, ESphAttr eType ) const;
	void			UpdateMVAAttr ( const SqlNode_t & tName, const SqlNode_t& dValues );
	void			UpdateStringAttr ( const SqlNode_t & tCol, const SqlNode_t & tStr );
	void			SetGroupbyLimit ( int iLimit );
	void			SetLimit ( int iOffset, int iLimit );
	void			SetIndex ( const SqlNode_t & tIndex );
	void			SetIndex ( const SqlNode_t & tIndex, CSphString & sIndex ) const;
	// split ident ( cluster:index ) to parts
	static void		SplitClusterIndex ( CSphString & sIndex, CSphString * pCluster );

private:
	bool						m_bGotQuery = false;
	BYTE						m_uSyntaxFlags = 0;
	bool						m_bNamedVecBusy = false;
	CSphVector<CSphNamedInt>	m_dNamedVec;

	void			AutoAlias ( CSphQueryItem & tItem, SqlNode_t * pStart, SqlNode_t * pEnd );
	void			GenericStatement ( SqlNode_t * pNode, SqlStmt_e iStmt );

	bool			CheckInteger ( const CSphString & sOpt, const CSphString & sVal ) const;
};


bool	sphParseSqlQuery ( const char * sQuery, int iLen, CSphVector<SqlStmt_t> & dStmt, CSphString & sError, ESphCollation eCollation );
bool	PercolateParseFilters ( const char * sFilters, ESphCollation eCollation, const CSphSchema & tSchema, CSphVector<CSphFilterSettings> & dFilters, CSphVector<FilterTreeItem_t> & dFilterTree, CSphString & sError );

// get tokens from sphinxql
int		sphGetTokTypeInt();
int		sphGetTokTypeFloat();
int		sphGetTokTypeStr();
int		sphGetTokTypeConstMVA();

#endif // _searchdsql_
