//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "searchdsql.h"
#include "sphinxint.h"
#include "sphinxplugin.h"
#include "searchdaemon.h"
#include "searchdddl.h"

extern int g_iAgentQueryTimeoutMs;	// global (default). May be override by index-scope values, if one specified

SqlStmt_t::SqlStmt_t()
{
	m_tQuery.m_eMode = SPH_MATCH_EXTENDED2; // only new and shiny matching and sorting
	m_tQuery.m_eSort = SPH_SORT_EXTENDED;
	m_tQuery.m_sSortBy = "@weight desc"; // default order
	m_tQuery.m_sOrderBy = "@weight desc";
	m_tQuery.m_iAgentQueryTimeoutMs = g_iAgentQueryTimeoutMs;
	m_tQuery.m_iRetryCount = -1;
	m_tQuery.m_iRetryDelay = -1;
}


SqlStmt_t::~SqlStmt_t()
{
	SafeDelete ( m_pTableFunc );
}


bool SqlStmt_t::AddSchemaItem ( const char * psName )
{
	m_dInsertSchema.Add ( psName );
	CSphString & sAttr = m_dInsertSchema.Last();
	sAttr.ToLower();
	int iLen = sAttr.Length();
	if ( iLen>1 && sAttr.cstr()[0] == '`' && sAttr.cstr()[iLen-1]=='`' )
		sAttr = sAttr.SubString ( 1, iLen-2 );

	m_iSchemaSz = m_dInsertSchema.GetLength();
	return true; // stub; check if the given field actually exists in the schema
}

// check if the number of fields which would be inserted is in accordance to the given schema
bool SqlStmt_t::CheckInsertIntegrity()
{
	// cheat: if no schema assigned, assume the size of schema as the size of the first row.
	// (if it is wrong, it will be revealed later)
	if ( !m_iSchemaSz )
		m_iSchemaSz = m_dInsertValues.GetLength();

	m_iRowsAffected++;
	return m_dInsertValues.GetLength()==m_iRowsAffected*m_iSchemaSz;
}

//////////////////////////////////////////////////////////////////////////

SqlParserTraits_c::SqlParserTraits_c ( CSphVector<SqlStmt_t> & dStmt, const char* szQuery, CSphString* pError )
	: m_pBuf ( szQuery )
	, m_pParseError ( pError )
	, m_dStmt ( dStmt )
{}


void SqlParserTraits_c::PushQuery()
{
	assert ( m_dStmt.GetLength() || ( !m_pQuery && !m_pStmt ) );

	// add new
	m_dStmt.Add ( SqlStmt_t() );
	m_pStmt = &m_dStmt.Last();
}


CSphString & SqlParserTraits_c::ToString ( CSphString & sRes, const SqlNode_t & tNode ) const
{
	if ( tNode.m_iType>=0 )
		sRes.SetBinary ( m_pBuf + tNode.m_iStart, tNode.m_iEnd - tNode.m_iStart );
	else switch ( tNode.m_iType )
	{
	case SPHINXQL_TOK_COUNT:	sRes = "@count"; break;
	case SPHINXQL_TOK_GROUPBY:	sRes = "@groupby"; break;
	case SPHINXQL_TOK_WEIGHT:	sRes = "@weight"; break;
	default:					assert ( 0 && "internal error: unknown parser ident code" );
	}
	return sRes;
}


CSphString SqlParserTraits_c::ToStringUnescape ( const SqlNode_t & tNode ) const
{
	assert ( tNode.m_iType>=0 );
	return SqlUnescape ( m_pBuf + tNode.m_iStart, tNode.m_iEnd - tNode.m_iStart );
}

//////////////////////////////////////////////////////////////////////////

enum class Option_e : BYTE;

class SqlParser_c : public SqlParserTraits_c
{
public:
	ESphCollation	m_eCollation;

	CSphVector<FilterTreeItem_t> m_dFilterTree;
	CSphVector<int>	m_dFiltersPerStmt;
	bool			m_bGotFilterOr = false;
	bool 			m_bGotDDLClause = false;

public:
					SqlParser_c ( CSphVector<SqlStmt_t> & dStmt, ESphCollation eCollation, const char* szQuery, CSphString* pError );

	void			PushQuery ();

	bool			AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue );
	bool			AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue, const SqlNode_t & sArg );
	bool			AddOption ( const SqlNode_t & tIdent, CSphVector<CSphNamedInt> & dNamed );
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
	bool			AddFloatFilterGreater ( const SqlNode_t & tAttr, float fVal, bool bHasEqual );
	bool			AddFloatFilterLesser ( const SqlNode_t & tAttr, float fVal, bool bHasEqual );
	bool			AddIntRangeFilter ( const SqlNode_t & tAttr, int64_t iMin, int64_t iMax, bool bExclude );
	bool			AddIntFilterGreater ( const SqlNode_t & tAttr, int64_t iVal, bool bHasEqual );
	bool			AddIntFilterLesser ( const SqlNode_t & tAttr, int64_t iVal, bool bHasEqual );
	bool			AddUservarFilter ( const SqlNode_t & tCol, const SqlNode_t & tVar, bool bExclude );
	void			AddGroupBy ( const SqlNode_t & tGroupBy );
	CSphFilterSettings * AddFilter ( const SqlNode_t & tCol, ESphFilter eType );
	bool			AddStringFilter ( const SqlNode_t & tCol, const SqlNode_t & tVal, bool bExclude );
	CSphFilterSettings * AddValuesFilter ( const SqlNode_t & tCol ) { return AddFilter ( tCol, SPH_FILTER_VALUES ); }
	bool			AddStringListFilter ( const SqlNode_t & tCol, SqlNode_t & tVal, StrList_e eType, bool bInverse=false );
	bool			AddNullFilter ( const SqlNode_t & tCol, bool bEqualsNull );
	void			AddHaving ();

	bool			AddDistinct ( SqlNode_t * pNewExpr, SqlNode_t * pStart, SqlNode_t * pEnd );
	void			AddDistinct ( SqlNode_t * pNewExpr );
	bool			MaybeAddFacetDistinct();

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
	void			GenericStatement ( SqlNode_t * pNode );
	void 			SwapSubkeys();

	void			AddUpdatedAttr ( const SqlNode_t & tName, ESphAttr eType ) const;
	void			UpdateMVAAttr ( const SqlNode_t & tName, const SqlNode_t& dValues );
	void			UpdateStringAttr ( const SqlNode_t & tCol, const SqlNode_t & tStr );
	void			SetGroupbyLimit ( int iLimit );
	void			SetLimit ( int iOffset, int iLimit );

	float			ToFloat ( const SqlNode_t & tNode ) const;
	int64_t			DotGetInt ( const SqlNode_t & tNode ) const;

	void 			AddStringSubkey ( const SqlNode_t & tNode ) const;
	void 			AddIntSubkey ( const SqlNode_t & tNode ) const;
	void			AddDotIntSubkey ( const SqlNode_t & tNode ) const;

private:
	bool						m_bGotQuery = false;
	BYTE						m_uSyntaxFlags = 0;
	bool						m_bNamedVecBusy = false;
	CSphVector<CSphNamedInt>	m_dNamedVec;

	void			AutoAlias ( CSphQueryItem & tItem, SqlNode_t * pStart, SqlNode_t * pEnd );
	bool			CheckInteger ( const CSphString & sOpt, const CSphString & sVal ) const;
	bool			CheckOption ( Option_e eOption ) const;
	SqlStmt_e		GetSecondaryStmt () const;
};



#define YYSTYPE SqlNode_t

									// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_DECL static int my_lex ( YYSTYPE * lvalp, void * yyscanner, SqlParser_c * pParser )

#if _WIN32
#define YY_NO_UNISTD_H 1
#endif

#include "flexsphinxql.c"

void yyerror ( SqlParser_c * pParser, const char * sMessage )
{
	// flex put a zero at last token boundary; make it undo that
	yylex_unhold ( pParser->m_pScanner );

	// create our error message
	pParser->m_pParseError->SetSprintf ( "%s %s near '%s'", pParser->m_sErrorHeader.cstr(), sMessage,
		pParser->m_pLastTokenStart ? pParser->m_pLastTokenStart : "(null)" );

	// fixup TOK_xxx thingies
	char * s = const_cast<char*> ( pParser->m_pParseError->cstr() );
	char * d = s;
	while ( *s )
	{
		if ( strncmp ( s, "TOK_", 4 )==0 )
			s += 4;
		else
			*d++ = *s++;
	}
	*d = '\0';
}


#ifndef NDEBUG
// using a proxy to be possible to debug inside yylex
static int yylex ( YYSTYPE * lvalp, SqlParser_c * pParser )
{
	int res = my_lex ( lvalp, pParser->m_pScanner, pParser );
	return res;
}
#else
static int yylex ( YYSTYPE * lvalp, SqlParser_c * pParser )
{
	return my_lex ( lvalp, pParser->m_pScanner, pParser );
}
#endif

#include "bissphinxql.c"

//////////////////////////////////////////////////////////////////////////

SqlParser_c::SqlParser_c ( CSphVector<SqlStmt_t> & dStmt, ESphCollation eCollation, const char* szQuery, CSphString* pError )
	: SqlParserTraits_c ( dStmt, szQuery, pError )
	, m_eCollation ( eCollation )
{
	assert ( m_dStmt.IsEmpty() );
	PushQuery ();
}

void SqlParser_c::PushQuery ()
{
	assert ( m_dStmt.GetLength() || ( !m_pQuery && !m_pStmt ) );

	// post set proper result-set order
	if ( m_dStmt.GetLength() && m_pQuery )
	{
		if ( m_pQuery->m_sGroupBy.IsEmpty() )
			m_pQuery->m_sSortBy = m_pQuery->m_sOrderBy;
		else
			m_pQuery->m_sGroupSortBy = m_pQuery->m_sOrderBy;

		m_dFiltersPerStmt.Add ( m_dFilterTree.GetLength() );
	}

	SqlParserTraits_c::PushQuery();

	m_pQuery = &m_pStmt->m_tQuery;
	m_pQuery->m_eCollation = m_eCollation;

	m_bGotQuery = false;
}


static bool CheckInteger ( const CSphString & sOpt, const CSphString & sVal, CSphString & sError )
{
	const char * p = sVal.cstr();
	while ( sphIsInteger ( *p++ ) )
		p++;

	if ( *p )
	{
		sError.SetSprintf ( "%s value should be a number: '%s'", sOpt.cstr(), sVal.cstr() );
		return false;
	}

	return true;
}


bool SqlParser_c::CheckInteger ( const CSphString & sOpt, const CSphString & sVal ) const
{
	return ::CheckInteger ( sOpt, sVal, *m_pParseError );
}

float SqlParser_c::ToFloat ( const SqlNode_t & tNode ) const
{
	return (float) strtod ( m_pBuf+tNode.m_iStart, nullptr );
}

int64_t SqlParser_c::DotGetInt ( const SqlNode_t & tNode ) const
{
	return (int64_t) strtoull ( m_pBuf+tNode.m_iStart+1, nullptr, 10 );
}

void SqlParser_c::AddStringSubkey ( const SqlNode_t & tNode ) const
{
	auto& sKey = m_pStmt->m_dStringSubkeys.Add();
	ToString ( sKey, tNode );
}

void SqlParser_c::AddIntSubkey ( const SqlNode_t & tNode ) const
{
	m_pStmt->m_dIntSubkeys.Add ( tNode.m_iValue );
}

void SqlParser_c::AddDotIntSubkey ( const SqlNode_t & tNode ) const
{
	m_pStmt->m_dIntSubkeys.Add ( DotGetInt ( tNode ) );
}


/// hashes for all options
enum class Option_e : BYTE
{
	AGENT_QUERY_TIMEOUT = 0,
	BOOLEAN_SIMPLIFY,
	COLUMNS,
	COMMENT,
	CUTOFF,
	DEBUG_NO_PAYLOAD, // fixme! document
	EXPAND_KEYWORDS,
	FIELD_WEIGHTS,
	FORMAT,
	GLOBAL_IDF,
	IDF,
	IGNORE_NONEXISTENT_COLUMNS,
	IGNORE_NONEXISTENT_INDEXES, // fixme! document!
	INDEX_WEIGHTS,
	LOCAL_DF,
	LOW_PRIORITY,
	MAX_MATCHES,
	MAX_PREDICTED_TIME,
	MAX_QUERY_TIME,
	MORPHOLOGY,
	RAND_SEED,
	RANKER,
	RETRY_COUNT,
	RETRY_DELAY,
	REVERSE_SCAN,
	SORT_METHOD,
	STRICT_, 		// dash added because of windows
	SYNC,
	THREADS,
	TOKEN_FILTER,
	TOKEN_FILTER_OPTIONS,
	NOT_ONLY_ALLOWED,
	STORE,
	PSEUDO_THRESH,
	MAXMATCH_THRESH,

	INVALID_OPTION
};

static SmallStringHash_T<Option_e, (BYTE) Option_e::INVALID_OPTION * 2> g_hParseOption;

void InitParserOption()
{
	static const char * dOptions[(BYTE) Option_e::INVALID_OPTION] = { "agent_query_timeout", "boolean_simplify",
		"columns", "comment", "cutoff", "debug_no_payload", "expand_keywords", "field_weights", "format", "global_idf",
		"idf", "ignore_nonexistent_columns", "ignore_nonexistent_indexes", "index_weights", "local_df", "low_priority",
		"max_matches", "max_predicted_time", "max_query_time", "morphology", "rand_seed", "ranker", "retry_count",
		"retry_delay", "reverse_scan", "sort_method", "strict", "sync", "threads", "token_filter", "token_filter_options",
		"not_terms_only_allowed", "store", "pseudo_thresh", "maxmatch_thresh" };

	for ( BYTE i = 0u; i<(BYTE) Option_e::INVALID_OPTION; ++i )
		g_hParseOption.Add ( (Option_e) i, dOptions[i] );
}

static Option_e ParseOption ( const CSphString& sOpt )
{
	auto * pCol = g_hParseOption ( sOpt );
	return ( pCol ? *pCol : Option_e::INVALID_OPTION );
}

static bool CheckOption ( SqlStmt_e eStmt, Option_e eOption )
{
	// trick! following vectors must be sorted, as BinarySearch used to determine presence of a value.
	static Option_e dDeleteOptions[] = { Option_e::STORE };

	static Option_e dUpdateOptions[] = { Option_e::AGENT_QUERY_TIMEOUT, Option_e::BOOLEAN_SIMPLIFY, Option_e::COMMENT,
			Option_e::CUTOFF, Option_e::DEBUG_NO_PAYLOAD, Option_e::EXPAND_KEYWORDS, Option_e::FIELD_WEIGHTS,
			Option_e::GLOBAL_IDF, Option_e::IDF, Option_e::IGNORE_NONEXISTENT_COLUMNS,
			Option_e::IGNORE_NONEXISTENT_INDEXES, Option_e::INDEX_WEIGHTS, Option_e::LOCAL_DF, Option_e::LOW_PRIORITY,
			Option_e::MAX_MATCHES, Option_e::MAX_PREDICTED_TIME, Option_e::MAX_QUERY_TIME, Option_e::MORPHOLOGY,
			Option_e::RAND_SEED, Option_e::RANKER, Option_e::RETRY_COUNT, Option_e::RETRY_DELAY, Option_e::REVERSE_SCAN,
			Option_e::SORT_METHOD, Option_e::STRICT_, Option_e::THREADS, Option_e::TOKEN_FILTER,
			Option_e::NOT_ONLY_ALLOWED };

	static Option_e dSelectOptions[] = { Option_e::AGENT_QUERY_TIMEOUT, Option_e::BOOLEAN_SIMPLIFY, Option_e::COMMENT,
			Option_e::CUTOFF, Option_e::DEBUG_NO_PAYLOAD, Option_e::EXPAND_KEYWORDS, Option_e::FIELD_WEIGHTS,
			Option_e::GLOBAL_IDF, Option_e::IDF, Option_e::IGNORE_NONEXISTENT_INDEXES, Option_e::INDEX_WEIGHTS,
			Option_e::LOCAL_DF, Option_e::LOW_PRIORITY, Option_e::MAX_MATCHES, Option_e::MAX_PREDICTED_TIME,
			Option_e::MAX_QUERY_TIME, Option_e::MORPHOLOGY, Option_e::RAND_SEED, Option_e::RANKER,
			Option_e::RETRY_COUNT, Option_e::RETRY_DELAY, Option_e::REVERSE_SCAN, Option_e::SORT_METHOD,
			Option_e::THREADS, Option_e::TOKEN_FILTER, Option_e::NOT_ONLY_ALLOWED, Option_e::PSEUDO_THRESH, Option_e::MAXMATCH_THRESH };

	static Option_e dInsertOptions[] = { Option_e::TOKEN_FILTER_OPTIONS };

	static Option_e dOptimizeOptions[] = { Option_e::CUTOFF, Option_e::SYNC };

	static Option_e dShowOptions[] = { Option_e::COLUMNS, Option_e::FORMAT };

#define CHKOPT( _set, _val ) VecTraits_T<Option_e> (_set, sizeof(_set)).BinarySearch (_val)!=nullptr

	switch ( eStmt )
	{
	case STMT_DELETE:
		return CHKOPT( dDeleteOptions, eOption );
	case STMT_UPDATE:
		return CHKOPT( dUpdateOptions, eOption );
	case STMT_SELECT:
		return CHKOPT( dSelectOptions, eOption );
	case STMT_INSERT:
	case STMT_REPLACE:
		return CHKOPT( dInsertOptions, eOption );
	case STMT_OPTIMIZE_INDEX:
		return CHKOPT( dOptimizeOptions, eOption );
	case STMT_EXPLAIN:
	case STMT_SHOW_PLAN:
	case STMT_SHOW_THREADS:
		return CHKOPT( dShowOptions, eOption );
	default:
		return false;
	}
#undef CHKOPT
}

// if query is special, like 'select .. from @@system.threads', it can adopt options for 'show threads' also,
// so, provide stmt for extended validation of the option in this case.
SqlStmt_e SqlParser_c::GetSecondaryStmt () const
{
	if ( m_pQuery->m_dStringSubkeys.any_of ([] (const CSphString& s) { return s==".threads"; }))
		return STMT_SHOW_THREADS;

	return STMT_PARSE_ERROR;
}

bool SqlParser_c::CheckOption ( Option_e eOption ) const
{
	assert ( m_pStmt );
	auto bRes = ::CheckOption ( m_pStmt->m_eStmt, eOption );

	if ( bRes )
		return true;

	if ( m_pStmt->m_eStmt!=STMT_SELECT )
		return false;

	return ::CheckOption ( GetSecondaryStmt(), eOption );
}

static auto fnFailer ( CSphString& sError )
{
	return [&sError] ( const char* sTemplate, ... ) {
		va_list ap;
		va_start ( ap, sTemplate );
		sError.SetSprintfVa ( sTemplate, ap );
		va_end ( ap );
		return AddOption_e::FAILED;
	};
}

#ifdef FAILED
#undef FAILED
#endif


AddOption_e AddOption ( CSphQuery & tQuery, const CSphString & sOpt, const CSphString & sValue, int64_t iValue, SqlStmt_e eStmt, CSphString & sError )
{
	auto FAILED = fnFailer (sError);

	auto eOpt = ParseOption ( sOpt );
	if ( !CheckOption ( eStmt, eOpt ) )
		return FAILED ( "unknown option '%s'", sOpt.cstr () );

	const Option_e dIntegerOptions[] =
	{	
		Option_e::MAX_MATCHES, Option_e::CUTOFF, Option_e::MAX_QUERY_TIME, Option_e::RETRY_COUNT,
		Option_e::RETRY_DELAY, Option_e::IGNORE_NONEXISTENT_COLUMNS, Option_e::AGENT_QUERY_TIMEOUT, Option_e::MAX_PREDICTED_TIME,
		Option_e::BOOLEAN_SIMPLIFY, Option_e::GLOBAL_IDF, Option_e::LOCAL_DF, Option_e::IGNORE_NONEXISTENT_INDEXES,
		Option_e::STRICT_, Option_e::COLUMNS, Option_e::RAND_SEED, Option_e::SYNC, Option_e::EXPAND_KEYWORDS,
		Option_e::THREADS, Option_e::NOT_ONLY_ALLOWED, Option_e::LOW_PRIORITY, Option_e::DEBUG_NO_PAYLOAD,
		Option_e::PSEUDO_THRESH, Option_e::MAXMATCH_THRESH
	};

	bool bFound = ::any_of ( dIntegerOptions, [eOpt] ( auto i ) { return i == eOpt; } );

	if ( !bFound )
		return AddOption_e::NOT_FOUND;

	if ( sValue.cstr() && !CheckInteger ( sOpt, sValue, sError ) )
		return AddOption_e::FAILED;

	switch ( eOpt )
	{
	case Option_e::MAX_MATCHES: // else if ( sOpt=="max_matches" )
		tQuery.m_iMaxMatches = (int)iValue;
		tQuery.m_bExplicitMaxMatches = true;
		break;

	case Option_e::DEBUG_NO_PAYLOAD:
		if ( iValue )
			tQuery.m_uDebugFlags |= QUERY_DEBUG_NO_PAYLOAD;
		else
			tQuery.m_uDebugFlags &= ~QUERY_DEBUG_NO_PAYLOAD;
		break;

	case Option_e::CUTOFF:						tQuery.m_iCutoff = (int)iValue;	break;
	case Option_e::MAX_QUERY_TIME: 				tQuery.m_uMaxQueryMsec = (int)iValue; break;
	case Option_e::RETRY_COUNT:					tQuery.m_iRetryCount = (int)iValue; break;
	case Option_e::RETRY_DELAY:					tQuery.m_iRetryDelay = (int)iValue; break;
	case Option_e::IGNORE_NONEXISTENT_COLUMNS:	tQuery.m_bIgnoreNonexistent = iValue!=0; break;
	case Option_e::AGENT_QUERY_TIMEOUT:			tQuery.m_iAgentQueryTimeoutMs = (int)iValue; break;
	case Option_e::MAX_PREDICTED_TIME:			tQuery.m_iMaxPredictedMsec = int ( iValue > INT_MAX ? INT_MAX : iValue ); break;
	case Option_e::BOOLEAN_SIMPLIFY:			tQuery.m_bSimplify = iValue!=0; break;
	case Option_e::GLOBAL_IDF:					tQuery.m_bGlobalIDF = iValue!=0; break;
	case Option_e::LOCAL_DF: 					tQuery.m_bLocalDF = iValue!=0; break;
	case Option_e::IGNORE_NONEXISTENT_INDEXES:	tQuery.m_bIgnoreNonexistentIndexes = iValue!=0; break;
	case Option_e::STRICT_:						tQuery.m_bStrict = iValue!=0; break;
	case Option_e::SYNC: 						tQuery.m_bSync = iValue!=0; break;
	case Option_e::EXPAND_KEYWORDS:				tQuery.m_eExpandKeywords = ( iValue!=0 ? QUERY_OPT_ENABLED : QUERY_OPT_DISABLED ); break;
	case Option_e::THREADS:						tQuery.m_iCouncurrency = (int)iValue; break;
	case Option_e::NOT_ONLY_ALLOWED:			tQuery.m_bNotOnlyAllowed = iValue!=0; break;
	case Option_e::RAND_SEED:					tQuery.m_iRandSeed = int64_t(DWORD(iValue)); break;
	case Option_e::LOW_PRIORITY:				tQuery.m_bLowPriority = iValue!=0; break;
	case Option_e::PSEUDO_THRESH:				tQuery.m_iPseudoThresh = iValue; break;
	case Option_e::MAXMATCH_THRESH:				tQuery.m_iMaxMatchThresh = iValue; break;
	default:
		return AddOption_e::NOT_FOUND;
	}

	return AddOption_e::ADDED;
}


AddOption_e AddOption ( CSphQuery & tQuery, const CSphString & sOpt, const CSphString & sVal, const std::function<CSphString ()> & fnGetUnescaped, SqlStmt_e eStmt, CSphString & sError )
{
	auto FAILED = fnFailer ( sError );

	auto eOpt = ParseOption ( sOpt );
	if ( !::CheckOption ( eStmt, eOpt ) )
		return FAILED ( "unknown option '%s'", sOpt.cstr () );

	// OPTIMIZE? hash possible sOpt choices?
	switch ( eOpt )
	{
	case Option_e::RANKER:
		tQuery.m_eRanker = SPH_RANK_TOTAL;
		for ( int iRanker = SPH_RANK_PROXIMITY_BM25; iRanker<=SPH_RANK_SPH04; iRanker++ )
			if ( sVal==sphGetRankerName ( ESphRankMode ( iRanker ) ) )
			{
				tQuery.m_eRanker = ESphRankMode ( iRanker );
				break;
			}

		if ( tQuery.m_eRanker==SPH_RANK_TOTAL )
		{
			if ( sVal==sphGetRankerName ( SPH_RANK_EXPR ) || sVal==sphGetRankerName ( SPH_RANK_EXPORT ) )
				return FAILED ( "missing ranker expression (use OPTION ranker=expr('1+2') for example)" );
			else if ( sphPluginExists ( PLUGIN_RANKER, sVal.cstr() ) )
			{
				tQuery.m_eRanker = SPH_RANK_PLUGIN;
				tQuery.m_sUDRanker = sVal;
			}

			return FAILED ( "unknown ranker '%s'", sVal.cstr() );
		}
		break;

	case Option_e::TOKEN_FILTER:    // tokfilter = hello.dll:hello:some_opts
	{
		StrVec_t dParams;
		if ( !sphPluginParseSpec ( sVal, dParams, sError ) )
			return AddOption_e::FAILED;

		if ( !dParams.GetLength() )
			return FAILED ( "missing token filter spec string" );

		tQuery.m_sQueryTokenFilterLib = dParams[0];
		tQuery.m_sQueryTokenFilterName = dParams[1];
		tQuery.m_sQueryTokenFilterOpts = dParams[2];
	}
	break;

	case Option_e::REVERSE_SCAN: //} else if ( sOpt=="reverse_scan" )
		return FAILED ( "reverse_scan is deprecated" );

	case Option_e::COMMENT: //} else if ( sOpt=="comment" )
		tQuery.m_sComment = fnGetUnescaped();
		break;

	case Option_e::SORT_METHOD: //} else if ( sOpt=="sort_method" )
		if ( sVal=="pq" )			tQuery.m_bSortKbuffer = false;
		else if ( sVal=="kbuffer" )	tQuery.m_bSortKbuffer = true;
		else
			return FAILED ( "unknown sort_method=%s (known values are pq, kbuffer)", sVal.cstr() );
		break;

	case Option_e::IDF: //} else if ( sOpt=="idf" )
	{
		StrVec_t dOpts;
		sphSplit ( dOpts, sVal.cstr() );

		ARRAY_FOREACH ( i, dOpts )
		{
			if ( dOpts[i]=="normalized" )
				tQuery.m_bPlainIDF = false;
			else if ( dOpts[i]=="plain" )
				tQuery.m_bPlainIDF = true;
			else if ( dOpts[i]=="tfidf_normalized" )
				tQuery.m_bNormalizedTFIDF = true;
			else if ( dOpts[i]=="tfidf_unnormalized" )
				tQuery.m_bNormalizedTFIDF = false;
			else
				return FAILED ( "unknown flag %s in idf=%s (known values are plain, normalized, tfidf_normalized, tfidf_unnormalized)", dOpts[i].cstr(), sVal.cstr() );
		}
	}
	break;


	case Option_e::MORPHOLOGY: //} else if ( sOpt=="morphology" )
		if ( sVal=="none" )
			tQuery.m_eExpandKeywords = QUERY_OPT_MORPH_NONE;
		else
			return FAILED ( "morphology could be only disabled with option none, got %s", sVal.cstr() );
		break;

	case Option_e::STORE: //} else if ( sOpt=="store" )
		tQuery.m_sStore = sVal;
		break;

	default:
		return AddOption_e::NOT_FOUND;
	}

	return AddOption_e::ADDED;
}


AddOption_e AddOption ( CSphQuery & tQuery, const CSphString & sOpt, CSphVector<CSphNamedInt> & dNamed, SqlStmt_e eStmt, CSphString & sError )
{
	auto FAILED = fnFailer ( sError );
	auto eOpt = ParseOption ( sOpt );
	if ( !::CheckOption ( eStmt, eOpt ) )
		return FAILED ( "unknown option '%s'", sOpt.cstr () );

	switch ( eOpt )
	{
	case Option_e::FIELD_WEIGHTS:	tQuery.m_dFieldWeights.SwapData ( dNamed ); break;
	case Option_e::INDEX_WEIGHTS:	tQuery.m_dIndexWeights.SwapData ( dNamed ); break;
	default:
		return AddOption_e::NOT_FOUND;
	}

	return AddOption_e::ADDED;
}


AddOption_e AddOptionRanker ( CSphQuery & tQuery, const CSphString & sOpt, const CSphString & sVal, const std::function<CSphString ()> & fnGetUnescaped, SqlStmt_e eStmt, CSphString & sError )
{
	auto FAILED = fnFailer ( sError );
	auto eOpt = ParseOption ( sOpt );
	if ( !::CheckOption ( eStmt, eOpt ) )
		return FAILED ( "unknown option '%s'", sOpt.cstr () );

	if ( eOpt==Option_e::RANKER )
	{
		if ( sVal=="expr" || sVal=="export" )
		{
			tQuery.m_eRanker = sVal=="expr" ? SPH_RANK_EXPR : SPH_RANK_EXPORT;
			tQuery.m_sRankerExpr = fnGetUnescaped();
			return AddOption_e::ADDED;
		} else if ( sphPluginExists ( PLUGIN_RANKER, sVal.cstr() ) )
		{
			tQuery.m_eRanker = SPH_RANK_PLUGIN;
			tQuery.m_sUDRanker = sVal;
			tQuery.m_sUDRankerOpts = fnGetUnescaped();
			return AddOption_e::ADDED;
		}
	}

	return AddOption_e::NOT_FOUND;
}


bool SqlParser_c::AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue )
{
	CSphString sOpt, sVal;
	ToString ( sOpt, tIdent ).ToLower();
	ToString ( sVal, tValue ).ToLower().Unquote();

	auto eOpt = ParseOption ( sOpt );
	if ( !CheckOption ( eOpt ) )
	{
		m_pParseError->SetSprintf ( "unknown option '%s'", sOpt.cstr () );
		return false;
	}

	AddOption_e eAddRes;
	eAddRes = ::AddOption ( *m_pQuery, sOpt, sVal, [this,tValue]{ return ToStringUnescape(tValue); }, m_pStmt->m_eStmt, *m_pParseError );
	if ( eAddRes==AddOption_e::FAILED )
		return false;
	else if ( eAddRes==AddOption_e::ADDED )
		return true;
	
	eAddRes = ::AddOption ( *m_pQuery, sOpt, sVal, tValue.m_iValue, m_pStmt->m_eStmt, *m_pParseError );
	if ( eAddRes==AddOption_e::FAILED )
		return false;
	else if ( eAddRes==AddOption_e::ADDED )
		return true;

	// OPTIMIZE? hash possible sOpt choices?
	switch ( eOpt )
	{
	case Option_e::COLUMNS: //} else if ( sOpt=="columns" ) // for SHOW THREADS
		if ( !CheckInteger ( sOpt, sVal ) )
			return false;

		m_pStmt->m_iThreadsCols = Max ( (int)tValue.m_iValue, 0 );
		break;

	case Option_e::FORMAT: //} else if ( sOpt=="format" ) // for SHOW THREADS
		m_pStmt->m_sThreadFormat = sVal;
		break;

	case Option_e::TOKEN_FILTER_OPTIONS: //} else if ( sOpt=="token_filter_options" )
		m_pStmt->m_sStringParam = sVal;
		break;

	default: //} else
		m_pParseError->SetSprintf ( "unknown option '%s' (or bad argument type)", sOpt.cstr() );
		return false;
	}

	return true;
}


bool SqlParser_c::AddOption ( const SqlNode_t & tIdent, const SqlNode_t & tValue, const SqlNode_t & tArg )
{
	CSphString sOpt, sVal;
	ToString ( sOpt, tIdent ).ToLower();
	ToString ( sVal, tValue ).ToLower().Unquote();

	auto eOpt = ParseOption ( sOpt );
	if ( !CheckOption ( eOpt ) )
	{
		m_pParseError->SetSprintf ( "unknown option '%s'", sOpt.cstr () );
		return false;
	}

	AddOption_e eAdd = ::AddOptionRanker ( *m_pQuery, sOpt, sVal, [this,tArg]{ return ToStringUnescape(tArg); }, m_pStmt->m_eStmt, *m_pParseError );
	if ( eAdd==AddOption_e::NOT_FOUND )
		m_pParseError->SetSprintf ( "unknown option '%s' (or bad argument type)", sOpt.cstr() );

	return eAdd==AddOption_e::ADDED;
}


bool SqlParser_c::AddOption ( const SqlNode_t & tIdent, CSphVector<CSphNamedInt> & dNamed )
{
	CSphString sOpt;
	ToString ( sOpt, tIdent ).ToLower ();
	auto eOpt = ParseOption ( sOpt );
	if ( !CheckOption ( eOpt ) )
	{
		m_pParseError->SetSprintf ( "unknown option '%s'", sOpt.cstr () );
		return false;
	}

	AddOption_e eAdd = ::AddOption ( *m_pQuery, sOpt, dNamed, m_pStmt->m_eStmt, *m_pParseError );
	if ( eAdd==AddOption_e::NOT_FOUND )
		m_pParseError->SetSprintf ( "unknown option '%s' (or bad argument type)", sOpt.cstr() );

	return eAdd==AddOption_e::ADDED;
}


void SqlParser_c::AddIndexHint ( IndexHint_e eHint, const SqlNode_t & tValue )
{
	CSphString sIndexes;
	ToString ( sIndexes, tValue );
	StrVec_t dIndexes;
	sphSplit ( dIndexes, sIndexes.cstr() );

	for ( const auto & i : dIndexes )
	{
		IndexHint_t & tHint = m_pQuery->m_dIndexHints.Add();
		tHint.m_sIndex = i;
		tHint.m_dHints[int(SecondaryIndexType_e::INDEX)] = eHint;
	}
}


void SqlParser_c::AliasLastItem ( SqlNode_t * pAlias )
{
	if ( pAlias )
	{
		CSphQueryItem & tItem = m_pQuery->m_dItems.Last();
		tItem.m_sAlias.SetBinary ( m_pBuf + pAlias->m_iStart, pAlias->m_iEnd - pAlias->m_iStart );
		tItem.m_sAlias.ToLower();
		SetSelect ( pAlias );
	}
}


void SqlParser_c::AddInsval ( CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode )
{
	SqlInsert_t & tIns = dVec.Add();
	tIns.m_iType = tNode.m_iType;
	tIns.m_iVal = tNode.m_iValue; // OPTIMIZE? copy conditionally based on type?
	tIns.m_fVal = tNode.m_fValue;
	if ( tIns.m_iType==TOK_QUOTED_STRING )
		tIns.m_sVal = ToStringUnescape ( tNode );
	tIns.m_pVals = tNode.m_pValues;
}


void SqlParser_c::ResetSelect()
{
	if ( m_pQuery )
		m_pQuery->m_iSQLSelectStart = m_pQuery->m_iSQLSelectEnd = -1;
}

void SqlParser_c::SetSelect ( SqlNode_t * pStart, SqlNode_t * pEnd )
{
	if ( m_pQuery )
	{
		if ( pStart && ( m_pQuery->m_iSQLSelectStart<0 || m_pQuery->m_iSQLSelectStart>pStart->m_iStart ) )
			m_pQuery->m_iSQLSelectStart = pStart->m_iStart;
		if ( !pEnd )
			pEnd = pStart;
		if ( pEnd && ( m_pQuery->m_iSQLSelectEnd<0 || m_pQuery->m_iSQLSelectEnd<pEnd->m_iEnd ) )
			m_pQuery->m_iSQLSelectEnd = pEnd->m_iEnd;
	}
}

void SqlParser_c::AutoAlias ( CSphQueryItem & tItem, SqlNode_t * pStart, SqlNode_t * pEnd )
{
	if ( pStart && pEnd )
	{
		tItem.m_sAlias.SetBinary ( m_pBuf + pStart->m_iStart, pEnd->m_iEnd - pStart->m_iStart );
		sphColumnToLowercase ( const_cast<char *>( tItem.m_sAlias.cstr() ) );
	} else
	{
		tItem.m_sAlias = tItem.m_sExpr;
	}
	SetSelect ( pStart, pEnd );
}

void SqlParser_c::AddItem ( SqlNode_t * pExpr, ESphAggrFunc eAggrFunc, SqlNode_t * pStart, SqlNode_t * pEnd )
{
	CSphQueryItem & tItem = m_pQuery->m_dItems.Add();
	tItem.m_sExpr.SetBinary ( m_pBuf + pExpr->m_iStart, pExpr->m_iEnd - pExpr->m_iStart );
	sphColumnToLowercase ( const_cast<char *>( tItem.m_sExpr.cstr() ) );
	tItem.m_eAggrFunc = eAggrFunc;
	AutoAlias ( tItem, pStart?pStart:pExpr, pEnd?pEnd:pExpr );
}

bool SqlParser_c::AddItem ( const char * pToken, SqlNode_t * pStart, SqlNode_t * pEnd )
{
	CSphQueryItem & tItem = m_pQuery->m_dItems.Add();
	tItem.m_sExpr = pToken;
	tItem.m_eAggrFunc = SPH_AGGR_NONE;
	sphColumnToLowercase ( const_cast<char *>( tItem.m_sExpr.cstr() ) );
	AutoAlias ( tItem, pStart, pEnd );
	return SetNewSyntax();
}

bool SqlParser_c::AddCount ()
{
	CSphQueryItem & tItem = m_pQuery->m_dItems.Add();
	tItem.m_sExpr = tItem.m_sAlias = "count(*)";
	tItem.m_eAggrFunc = SPH_AGGR_NONE;
	return SetNewSyntax();
}

void SqlParser_c::AddGroupBy ( const SqlNode_t & tGroupBy )
{
	if ( m_pQuery->m_sGroupBy.IsEmpty() )
	{
		m_pQuery->m_eGroupFunc = SPH_GROUPBY_ATTR;
		m_pQuery->m_sGroupBy.SetBinary ( m_pBuf + tGroupBy.m_iStart, tGroupBy.m_iEnd - tGroupBy.m_iStart );
		sphColumnToLowercase ( const_cast<char *>( m_pQuery->m_sGroupBy.cstr() ) );
	} else
	{
		m_pQuery->m_eGroupFunc = SPH_GROUPBY_MULTIPLE;
		CSphString sTmp;
		sTmp.SetBinary ( m_pBuf + tGroupBy.m_iStart, tGroupBy.m_iEnd - tGroupBy.m_iStart );
		sphColumnToLowercase ( const_cast<char *>( sTmp.cstr() ) );
		m_pQuery->m_sGroupBy.SetSprintf ( "%s, %s", m_pQuery->m_sGroupBy.cstr(), sTmp.cstr() );
	}
}

void SqlParser_c::SetGroupbyLimit ( int iLimit )
{
	m_pQuery->m_iGroupbyLimit = iLimit;
}

bool SqlParser_c::AddDistinct ( SqlNode_t * pNewExpr, SqlNode_t * pStart, SqlNode_t * pEnd )
{
	if ( !m_pQuery->m_sGroupDistinct.IsEmpty() )
	{
		yyerror ( this, "too many COUNT(DISTINCT) clauses" );
		return false;
	}

	ToString ( m_pQuery->m_sGroupDistinct, *pNewExpr );
	return AddItem ( "@distinct", pStart, pEnd );
}

void SqlParser_c::AddDistinct ( SqlNode_t * pNewExpr )
{
	if ( !pNewExpr )
	{
		m_pQuery->m_sGroupDistinct = "id";
	}
	else
	{
		ToString ( m_pQuery->m_sGroupDistinct, *pNewExpr );
		sphColumnToLowercase ( const_cast<char *>( m_pQuery->m_sGroupDistinct.cstr() ) );
	}
}

bool SqlParser_c::MaybeAddFacetDistinct()
{
	if ( m_pQuery->m_sGroupDistinct.IsEmpty() )
		return true;

	CSphQueryItem tItem;
	tItem.m_sExpr = "@distinct";
	tItem.m_eAggrFunc = SPH_AGGR_NONE;
	tItem.m_sAlias.SetSprintf ( "count(distinct %s)", m_pQuery->m_sGroupDistinct.cstr() );

	int iCountPos = m_pQuery->m_dItems.GetFirst ( [] ( const CSphQueryItem & tElem ) { return ( tElem.m_sExpr=="count(*)" ); });
	if ( iCountPos==-1 )
	{
		yyerror ( this, "can not find COUNT clause" );
		return false;
	}

	m_pQuery->m_dItems.Insert ( iCountPos, tItem );

	return SetNewSyntax();
}

bool SqlParser_c::AddSchemaItem ( YYSTYPE * pNode )
{
	assert ( m_pStmt );
	CSphString sItem;
	sItem.SetBinary ( m_pBuf + pNode->m_iStart, pNode->m_iEnd - pNode->m_iStart );
	return m_pStmt->AddSchemaItem ( sItem.cstr() );
}

bool SqlParser_c::SetMatch ( const YYSTYPE& tValue )
{
	if ( m_bGotQuery )
	{
		yyerror ( this, "too many MATCH() clauses" );
		return false;
	};

	m_pQuery->m_sQuery = ToStringUnescape ( tValue );
	m_pQuery->m_sRawQuery = m_pQuery->m_sQuery;
	return m_bGotQuery = true;
}

void SqlParser_c::AddConst ( int iList, const YYSTYPE& tValue )
{
	CSphVector<CSphNamedInt> & dVec = GetNamedVec ( iList );

	dVec.Add();
	ToString ( dVec.Last().first, tValue ).ToLower();
	dVec.Last().second = (int) tValue.m_iValue;
}

void SqlParser_c::SetStatement ( const YYSTYPE & tName, SqlSet_e eSet )
{
	m_pStmt->m_eStmt = STMT_SET;
	m_pStmt->m_eSet = eSet;
	ToString ( m_pStmt->m_sSetName, tName );
}

void SqlParser_c::SwapSubkeys ()
{
	m_pQuery->m_dIntSubkeys.SwapData ( m_pStmt->m_dIntSubkeys );
	m_pQuery->m_dStringSubkeys.SwapData ( m_pStmt->m_dStringSubkeys );
}

void SqlParser_c::GenericStatement ( SqlNode_t * pNode )
{
	SwapSubkeys();
	m_pStmt->m_iListStart = pNode->m_iStart;
	m_pStmt->m_iListEnd = pNode->m_iEnd;
	ToString ( m_pStmt->m_sIndex, *pNode );
}

void SqlParser_c::AddUpdatedAttr ( const SqlNode_t & tName, ESphAttr eType ) const
{
	CSphAttrUpdate & tUpd = m_pStmt->AttrUpdate();
	CSphString sAttr;
	TypedAttribute_t & tNew = tUpd.m_dAttributes.Add();
	tNew.m_sName = ToString ( sAttr, tName ).ToLower();
	tNew.m_eType = eType;
}


void SqlParser_c::UpdateMVAAttr ( const SqlNode_t & tName, const SqlNode_t & dValues )
{
	CSphAttrUpdate & tUpd = m_pStmt->AttrUpdate();
	ESphAttr eType = SPH_ATTR_UINT32SET;

	if ( dValues.m_pValues && dValues.m_pValues->GetLength()>0 )
	{
		// got MVA values, let's process them
		dValues.m_pValues->Uniq(); // don't need dupes within MVA
		tUpd.m_dPool.Add ( dValues.m_pValues->GetLength()*2 );
		for ( auto uVal : *dValues.m_pValues )
		{
			if ( uVal>UINT_MAX )
				eType = SPH_ATTR_INT64SET;
			*(( int64_t* ) tUpd.m_dPool.AddN ( 2 )) = uVal;
		}
	} else
	{
		// no values, means we should delete the attribute
		// we signal that to the update code by putting a single zero
		// to the values pool (meaning a zero-length MVA values list)
		tUpd.m_dPool.Add ( 0 );
	}

	AddUpdatedAttr ( tName, eType );
}


void SqlParser_c::UpdateStringAttr ( const SqlNode_t & tCol, const SqlNode_t & tStr )
{
	CSphAttrUpdate & tUpd = m_pStmt->AttrUpdate();

	auto sStr = ToStringUnescape ( tStr );

	int iLength = sStr.Length();
	tUpd.m_dPool.Add ( tUpd.m_dBlobs.GetLength() );
	tUpd.m_dPool.Add ( iLength );

	if ( iLength )
	{
		BYTE * pBlob = tUpd.m_dBlobs.AddN ( iLength+2 );	// a couple of extra \0 for json parser to be happy
		memcpy ( pBlob, sStr.cstr(), iLength );
		pBlob[iLength] = 0;
		pBlob[iLength+1] = 0;
	}

	AddUpdatedAttr ( tCol, SPH_ATTR_STRING );
}


CSphFilterSettings * SqlParser_c::AddFilter ( const SqlNode_t & tCol, ESphFilter eType )
{
	CSphString sCol;
	ToString ( sCol, tCol ); // do NOT lowercase just yet, might have to retain case for JSON cols

	FilterTreeItem_t & tElem = m_dFilterTree.Add();
	tElem.m_iFilterItem = m_pQuery->m_dFilters.GetLength();

	CSphFilterSettings * pFilter = &m_pQuery->m_dFilters.Add();
	pFilter->m_sAttrName = sCol;
	pFilter->m_eType = eType;
	sphColumnToLowercase ( const_cast<char *>( pFilter->m_sAttrName.cstr() ) );
	return pFilter;
}

bool SqlParser_c::AddFloatRangeFilter ( const SqlNode_t & sAttr, float fMin, float fMax, bool bHasEqual, bool bExclude )
{
	CSphFilterSettings * pFilter = AddFilter ( sAttr, SPH_FILTER_FLOATRANGE );
	if ( !pFilter )
		return false;
	pFilter->m_fMinValue = fMin;
	pFilter->m_fMaxValue = fMax;
	pFilter->m_bHasEqualMin = bHasEqual;
	pFilter->m_bHasEqualMax = bHasEqual;
	pFilter->m_bExclude = bExclude;
	return true;
}

bool SqlParser_c::AddFloatFilterGreater ( const SqlNode_t & tAttr, float fVal, bool bHasEqual )
{
	CSphFilterSettings * pFilter = AddFilter ( tAttr, SPH_FILTER_FLOATRANGE );
	if ( !pFilter )
		return false;

	pFilter->m_fMaxValue = FLT_MAX;
	pFilter->m_fMinValue = fVal;
	pFilter->m_bHasEqualMin = bHasEqual;
	pFilter->m_bOpenRight = true;

	return true;
}

bool SqlParser_c::AddFloatFilterLesser ( const SqlNode_t & tAttr, float fVal, bool bHasEqual )
{
	CSphFilterSettings * pFilter = AddFilter ( tAttr, SPH_FILTER_FLOATRANGE );
	if ( !pFilter )
		return false;

	pFilter->m_fMinValue = -FLT_MAX;
	pFilter->m_fMaxValue = fVal;
	pFilter->m_bHasEqualMax = bHasEqual;
	pFilter->m_bOpenLeft = true;

	return true;
}

bool SqlParser_c::AddIntRangeFilter ( const SqlNode_t & sAttr, int64_t iMin, int64_t iMax, bool bExclude )
{
	CSphFilterSettings * pFilter = AddFilter ( sAttr, SPH_FILTER_RANGE );
	if ( !pFilter )
		return false;
	pFilter->m_iMinValue = iMin;
	pFilter->m_iMaxValue = iMax;
	pFilter->m_bExclude = bExclude;
	return true;
}

bool SqlParser_c::AddIntFilterGreater ( const SqlNode_t & tAttr, int64_t iVal, bool bHasEqual )
{
	CSphFilterSettings * pFilter = AddFilter ( tAttr, SPH_FILTER_RANGE );
	if ( !pFilter )
		return false;

	pFilter->m_iMaxValue = LLONG_MAX;
	pFilter->m_iMinValue = iVal;
	pFilter->m_bHasEqualMin = bHasEqual;
	pFilter->m_bOpenRight = true;

	return true;
}

bool SqlParser_c::AddIntFilterLesser ( const SqlNode_t & tAttr, int64_t iVal, bool bHasEqual )
{
	CSphFilterSettings * pFilter = AddFilter ( tAttr, SPH_FILTER_RANGE );
	if ( !pFilter )
		return false;

	pFilter->m_iMinValue = LLONG_MIN;
	pFilter->m_iMaxValue = iVal;
	pFilter->m_bHasEqualMax = bHasEqual;
	pFilter->m_bOpenLeft = true;

	return true;
}

bool SqlParser_c::AddUservarFilter ( const SqlNode_t & tCol, const SqlNode_t & tVar, bool bExclude )
{
	CSphFilterSettings * pFilter = AddFilter ( tCol, SPH_FILTER_USERVAR );
	if ( !pFilter )
		return false;

	CSphString & sUserVar = pFilter->m_dStrings.Add();
	ToString ( sUserVar, tVar ).ToLower();
	pFilter->m_bExclude = bExclude;
	return true;
}


bool SqlParser_c::AddStringFilter ( const SqlNode_t & tCol, const SqlNode_t & tVal, bool bExclude )
{
	CSphFilterSettings * pFilter = AddFilter ( tCol, SPH_FILTER_STRING );
	if ( !pFilter )
		return false;
	CSphString & sFilterString = pFilter->m_dStrings.Add();
	sFilterString = ToStringUnescape ( tVal );
	pFilter->m_bExclude = bExclude;
	return true;
}


bool SqlParser_c::AddStringListFilter ( const SqlNode_t & tCol, SqlNode_t & tVal, StrList_e eType, bool bInverse )
{
	CSphFilterSettings * pFilter = AddFilter ( tCol, SPH_FILTER_STRING_LIST );
	if ( !pFilter || !tVal.m_pValues )
		return false;

	pFilter->m_dStrings.Resize ( tVal.m_pValues->GetLength() );
	ARRAY_FOREACH ( i, ( *tVal.m_pValues ) )
	{
		uint64_t uVal = ( *tVal.m_pValues )[i];
		int iOff = ( uVal>>32 );
		int iLen = ( uVal & 0xffffffff );
		pFilter->m_dStrings[i] = SqlUnescape ( m_pBuf + iOff, iLen );
	}
	tVal.m_pValues = nullptr;
	pFilter->m_bExclude = bInverse;
	assert ( pFilter->m_eMvaFunc == SPH_MVAFUNC_NONE ); // that is default for IN filter
	if ( eType==StrList_e::STR_ANY )
		pFilter->m_eMvaFunc = SPH_MVAFUNC_ANY;
	else if ( eType==StrList_e::STR_ALL )
		pFilter->m_eMvaFunc = SPH_MVAFUNC_ALL;
	return true;
}


bool SqlParser_c::AddNullFilter ( const SqlNode_t & tCol, bool bEqualsNull )
{
	CSphFilterSettings * pFilter = AddFilter ( tCol, SPH_FILTER_NULL );
	if ( !pFilter )
		return false;
	pFilter->m_bIsNull = bEqualsNull;
	return true;
}

void SqlParser_c::AddHaving ()
{
	assert ( m_pQuery->m_dFilters.GetLength() );
	m_pQuery->m_tHaving = m_pQuery->m_dFilters.Pop();
}


bool SqlParser_c::IsGoodSyntax()
{
	if ( ( m_uSyntaxFlags & 3 )!=3 )
		return true;
	yyerror ( this, "Mixing the old-fashion internal vars (@id, @count, @weight) with new acronyms like count(*), weight() is prohibited" );
	return false;
}


int SqlParser_c::AllocNamedVec ()
{
	// we only allow one such vector at a time, right now
	assert ( !m_bNamedVecBusy );
	m_bNamedVecBusy = true;
	m_dNamedVec.Resize ( 0 );
	return 0;
}

void SqlParser_c::SetLimit ( int iOffset, int iLimit )
{
	m_pQuery->m_iOffset = iOffset;
	m_pQuery->m_iLimit = iLimit;
}

#ifndef NDEBUG
CSphVector<CSphNamedInt> & SqlParser_c::GetNamedVec ( int iIndex )
#else
CSphVector<CSphNamedInt> & SqlParser_c::GetNamedVec ( int )
#endif
{
	assert ( m_bNamedVecBusy && iIndex==0 );
	return m_dNamedVec;
}

#ifndef NDEBUG
void SqlParser_c::FreeNamedVec ( int iIndex )
#else
void SqlParser_c::FreeNamedVec ( int )
#endif
{
	assert ( m_bNamedVecBusy && iIndex==0 );
	m_bNamedVecBusy = false;
	m_dNamedVec.Resize ( 0 );
}

void SqlParser_c::SetOp ( SqlNode_t & tNode )
{
	tNode.m_iParsedOp = m_dFilterTree.GetLength() - 1;
}


bool SqlParser_c::SetOldSyntax()
{
	m_uSyntaxFlags |= 1;
	return IsGoodSyntax ();
}


bool SqlParser_c::SetNewSyntax()
{
	m_uSyntaxFlags |= 2;
	return IsGoodSyntax();
}


bool SqlParser_c::IsDeprecatedSyntax() const
{
	return m_uSyntaxFlags & 1;
}


void SqlParser_c::FilterGroup ( SqlNode_t & tNode, SqlNode_t & tExpr )
{
	tNode.m_iParsedOp = tExpr.m_iParsedOp;
}

void SqlParser_c::FilterAnd ( SqlNode_t & tNode, const SqlNode_t & tLeft, const SqlNode_t & tRight )
{
	tNode.m_iParsedOp = m_dFilterTree.GetLength();

	FilterTreeItem_t & tElem = m_dFilterTree.Add();
	tElem.m_iLeft = tLeft.m_iParsedOp;
	tElem.m_iRight = tRight.m_iParsedOp;
}

void SqlParser_c::FilterOr ( SqlNode_t & tNode, const SqlNode_t & tLeft, const SqlNode_t & tRight )
{
	tNode.m_iParsedOp = m_dFilterTree.GetLength();
	m_bGotFilterOr = true;

	FilterTreeItem_t & tElem = m_dFilterTree.Add();
	tElem.m_bOr = true;
	tElem.m_iLeft = tLeft.m_iParsedOp;
	tElem.m_iRight = tRight.m_iParsedOp;
}


struct QueryItemProxy_t
{
	DWORD m_uHash;
	int m_iIndex;
	CSphQueryItem * m_pItem;

	bool operator < ( const QueryItemProxy_t & tItem ) const
	{
		return ( ( m_uHash<tItem.m_uHash ) || ( m_uHash==tItem.m_uHash && m_iIndex<tItem.m_iIndex ) );
	}

	bool operator == ( const QueryItemProxy_t & tItem ) const
	{
		return ( m_uHash==tItem.m_uHash );
	}

	void QueryItemHash ()
	{
		assert ( m_pItem );
		m_uHash = sphCRC32 ( m_pItem->m_sAlias.cstr() );
		m_uHash = sphCRC32 ( m_pItem->m_sExpr.cstr(), m_pItem->m_sExpr.Length(), m_uHash );
		m_uHash = sphCRC32 ( (const void*)&m_pItem->m_eAggrFunc, sizeof(m_pItem->m_eAggrFunc), m_uHash );
	}
};

static void CreateFilterTree ( const CSphVector<FilterTreeItem_t> & dOps, int iStart, int iCount, CSphQuery & tQuery )
{
	bool bHasOr = false;
	int iTreeCount = iCount - iStart;
	CSphVector<FilterTreeItem_t> dTree ( iTreeCount );
	for ( int i = 0; i<iTreeCount; i++ )
	{
		FilterTreeItem_t tItem = dOps[iStart + i];
		tItem.m_iLeft = ( tItem.m_iLeft==-1 ? -1 : tItem.m_iLeft - iStart );
		tItem.m_iRight = ( tItem.m_iRight==-1 ? -1 : tItem.m_iRight - iStart );
		dTree[i] = tItem;
		bHasOr |= ( tItem.m_iFilterItem==-1 && tItem.m_bOr );
	}

	// query has only plain AND filters - no need for filter tree
	if ( !bHasOr )
		return;

	tQuery.m_dFilterTree.SwapData ( dTree );
}


struct HintComp_fn
{
	bool IsLess ( const IndexHint_t & tA, const IndexHint_t & tB ) const
	{
		return strcasecmp ( tA.m_sIndex.cstr(), tB.m_sIndex.cstr() ) < 0;
	}

	bool IsEq ( const IndexHint_t & tA, const IndexHint_t & tB ) const
	{
		if ( tA.m_sIndex!=tB.m_sIndex )
			return false;

		for ( int i = 0; i < int(SecondaryIndexType_e::TOTAL); i++ )
			if ( tA.m_dHints[i] != tB.m_dHints[i] )
				return false;
		
		return true;
	}
};


static bool CheckQueryHints ( CSphVector<IndexHint_t> & dHints, CSphString & sError )
{
	sphSort ( dHints.Begin(), dHints.GetLength(), HintComp_fn() );
	sphUniq ( dHints.Begin(), dHints.GetLength(), HintComp_fn() );

	for ( int i = 1; i < dHints.GetLength(); i++ )
		if ( dHints[i-1].m_sIndex==dHints[i].m_sIndex )
		{
			sError.SetSprintf ( "conflicting hints specified for index '%s'", dHints[i-1].m_sIndex.cstr() );
			return false;
		}

	return true;
}


bool sphParseSqlQuery ( const char * sQuery, int iLen, CSphVector<SqlStmt_t> & dStmt, CSphString & sError, ESphCollation eCollation )
{
	if ( !sQuery || !iLen )
	{
		sError = "query was empty";
		return false;
	}

	SqlParser_c tParser ( dStmt, eCollation, sQuery, &sError );

	char * sEnd = const_cast<char *>( sQuery ) + iLen;
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	yylex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy_scan_buffer ( const_cast<char *>( sQuery ), iLen+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: yy_scan_buffer() failed";
		return false;
	}

	int iRes = yyparse ( &tParser );

	yy_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yylex_destroy ( tParser.m_pScanner );

	dStmt.Pop(); // last query is always dummy

	if ( tParser.m_bGotDDLClause )
		return ParseDdl ( sQuery, iLen, dStmt, sError );

	int iFilterStart = 0;
	int iFilterCount = 0;
	ARRAY_FOREACH ( iStmt, dStmt )
	{
		// select expressions will be reparsed again, by an expression parser,
		// when we have an index to actually bind variables, and create a tree
		//
		// so at SQL parse stage, we only do quick validation, and at this point,
		// we just store the select list for later use by the expression parser
		CSphQuery & tQuery = dStmt[iStmt].m_tQuery;
		if ( tQuery.m_iSQLSelectStart>=0 )
		{
			if ( tQuery.m_iSQLSelectStart-1>=0 && tParser.m_pBuf[tQuery.m_iSQLSelectStart-1]=='`' )
				tQuery.m_iSQLSelectStart--;
			if ( tQuery.m_iSQLSelectEnd<iLen && tParser.m_pBuf[tQuery.m_iSQLSelectEnd]=='`' )
				tQuery.m_iSQLSelectEnd++;

			tQuery.m_sSelect.SetBinary ( tParser.m_pBuf + tQuery.m_iSQLSelectStart,
				tQuery.m_iSQLSelectEnd - tQuery.m_iSQLSelectStart );
		}

		// validate tablefuncs
		// tablefuncs are searchd-level builtins rather than common expression-level functions
		// so validation happens here, expression parser does not know tablefuncs (ignorance is bliss)
		if ( dStmt[iStmt].m_eStmt==STMT_SELECT && !dStmt[iStmt].m_sTableFunc.IsEmpty() )
		{
			CSphString & sFunc = dStmt[iStmt].m_sTableFunc;
			sFunc.ToUpper();

			ISphTableFunc * pFunc = nullptr;
			if ( sFunc=="REMOVE_REPEATS" )
				pFunc = CreateRemoveRepeats();

			if ( !pFunc )
			{
				sError.SetSprintf ( "unknown table function %s()", sFunc.cstr() );
				return false;
			}
			if ( !pFunc->ValidateArgs ( dStmt[iStmt].m_dTableFuncArgs, tQuery, sError ) )
			{
				SafeDelete ( pFunc );
				return false;
			}
			dStmt[iStmt].m_pTableFunc = pFunc;
		}

		// validate filters
		for ( const auto& tFilter : tQuery.m_dFilters )
		{
			const CSphString & sCol = tFilter.m_sAttrName;
			if ( !strcasecmp ( sCol.cstr(), "@count" ) || !strcasecmp ( sCol.cstr(), "count(*)" ) )
			{
				sError.SetSprintf ( "sphinxql: aggregates in 'where' clause prohibited, use 'HAVING'" );
				return false;
			}
		}

		iFilterCount = tParser.m_dFiltersPerStmt[iStmt];

		// all queries have only plain AND filters - no need for filter tree
		if ( iFilterCount && tParser.m_bGotFilterOr )
			CreateFilterTree ( tParser.m_dFilterTree, iFilterStart, iFilterCount, tQuery );
		else
			OptimizeFilters ( tQuery.m_dFilters );


		iFilterStart = iFilterCount;

		// fixup hints
		if ( !CheckQueryHints ( tQuery.m_dIndexHints, sError ) )
			return false;
	}

	if ( iRes!=0 || !dStmt.GetLength() )
		return false;

	if ( tParser.IsDeprecatedSyntax() )
	{
		sError = "Using the old-fashion @variables (@count, @weight, etc.) is deprecated";
		return false;
	}

	// facets
	bool bGotFacet = false;
	ARRAY_FOREACH ( i, dStmt )
	{
		const SqlStmt_t & tHeadStmt = dStmt[i];
		const CSphQuery & tHeadQuery = tHeadStmt.m_tQuery;
		if ( dStmt[i].m_eStmt==STMT_SELECT )
		{
			++i;
			if ( i<dStmt.GetLength() && dStmt[i].m_eStmt==STMT_FACET )
			{
				bGotFacet = true;
				const_cast<CSphQuery &>(tHeadQuery).m_bFacetHead = true;
			}

			for ( ; i<dStmt.GetLength() && dStmt[i].m_eStmt==STMT_FACET; ++i )
			{
				SqlStmt_t & tStmt = dStmt[i];
				tStmt.m_tQuery.m_bFacet = true;

				tStmt.m_eStmt = STMT_SELECT;
				tStmt.m_tQuery.m_sIndexes = tHeadQuery.m_sIndexes;
				tStmt.m_tQuery.m_sSelect = tStmt.m_tQuery.m_sFacetBy;
				tStmt.m_tQuery.m_sQuery = tHeadQuery.m_sQuery;
				tStmt.m_tQuery.m_iMaxMatches = tHeadQuery.m_iMaxMatches;

				// append filters
				ARRAY_FOREACH ( k, tHeadQuery.m_dFilters )
					tStmt.m_tQuery.m_dFilters.Add ( tHeadQuery.m_dFilters[k] );
				ARRAY_FOREACH ( k, tHeadQuery.m_dFilterTree )
					tStmt.m_tQuery.m_dFilterTree.Add ( tHeadQuery.m_dFilterTree[k] );
			}
		}
	}

	if ( bGotFacet )
	{
		CSphString sDistinct;

		// need to keep order of query items same as at select list however do not duplicate items
		// that is why raw Vector.Uniq does not work here
		CSphVector<QueryItemProxy_t> dSelectItems;
		ARRAY_FOREACH ( i, dStmt )
		{
			CSphQuery & tQuery = dStmt[i].m_tQuery;
			ARRAY_FOREACH ( k, tQuery.m_dItems )
			{
				QueryItemProxy_t & tItem = dSelectItems.Add();
				tItem.m_pItem = tQuery.m_dItems.Begin() + k;
				tItem.m_iIndex = dSelectItems.GetLength() - 1;
				tItem.QueryItemHash();
			}

			if ( !tQuery.m_sGroupDistinct.IsEmpty() )
			{
				if ( !sDistinct.IsEmpty() && sDistinct!=tQuery.m_sGroupDistinct )
				{
					sError.SetSprintf ( "distinct field for all FACET queries should be the same '%s', query %d got '%s'", sDistinct.cstr(), i, tQuery.m_sGroupDistinct.cstr() );
					return false;
				}
				if ( sDistinct.IsEmpty() )
					sDistinct = tQuery.m_sGroupDistinct;
			}
		}
		// got rid of duplicates
		dSelectItems.Uniq();
		// sort back to select list appearance order
		dSelectItems.Sort ( bind ( &QueryItemProxy_t::m_iIndex ) );
		// get merged select list
		CSphVector<CSphQueryItem> dItems ( dSelectItems.GetLength() );
		ARRAY_FOREACH ( i, dSelectItems )
		{
			dItems[i] = *dSelectItems[i].m_pItem;
		}

		for ( SqlStmt_t& tStmt : dStmt )
		{
			// keep original items
			tStmt.m_tQuery.m_dItems.SwapData ( tStmt.m_tQuery.m_dRefItems );
			tStmt.m_tQuery.m_dItems = dItems;

			// for FACET strip off group by expression items
			// these come after count(*)
			if ( tStmt.m_tQuery.m_bFacet )
			{
				ARRAY_FOREACH ( j, tStmt.m_tQuery.m_dRefItems )
				{
					if ( tStmt.m_tQuery.m_dRefItems[j].m_sAlias=="count(*)" )
					{
						tStmt.m_tQuery.m_dRefItems.Resize ( j+1 );
						break;
					}
				}
			}

			tStmt.m_tQuery.m_sGroupDistinct = sDistinct;
		}
	}

	// need to keep same wide result set schema
	if ( !bGotFacet && dStmt.GetLength()>1 )
	{
		const CSphString & sDistinct = dStmt[0].m_tQuery.m_sGroupDistinct;
		for ( int i=1; i<dStmt.GetLength(); i++ )
			dStmt[i].m_tQuery.m_sGroupDistinct = sDistinct;
	}

	return true;
}


void SqlParser_SplitClusterIndex ( CSphString & sIndex, CSphString * pCluster )
{
	if ( sIndex.IsEmpty() )
		return;

	const char * sDelimiter = strchr ( sIndex.cstr(), ':' );
	if ( sDelimiter )
	{
		CSphString sTmp = sIndex; // m_sIndex.SetBinary can not accept this(m_sIndex) pointer

		int iPos = int ( sDelimiter - sIndex.cstr() );
		int iLen = sIndex.Length();
		sIndex.SetBinary ( sTmp.cstr() + iPos + 1, iLen - iPos - 1 );
		if ( pCluster )
			pCluster->SetBinary ( sTmp.cstr(), iPos );
	}
}


//////////////////////////////////////////////////////////////////////////

bool PercolateParseFilters ( const char * sFilters, ESphCollation eCollation, const CSphSchema & tSchema, CSphVector<CSphFilterSettings> & dFilters, CSphVector<FilterTreeItem_t> & dFilterTree, CSphString & sError )
{
	if ( !sFilters || !*sFilters )
		return true;

	StringBuilder_c sBuf;
	sBuf << "sysfilters " << sFilters;
	int iLen = sBuf.GetLength();

	CSphVector<SqlStmt_t> dStmt;
	SqlParser_c tParser ( dStmt, eCollation, sBuf.cstr(), &sError );
	tParser.m_sErrorHeader = "percolate filters:";

	char * sEnd = const_cast<char *>( sBuf.cstr() ) + iLen;
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	yylex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy_scan_buffer ( const_cast<char *>( sBuf.cstr() ), iLen+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: yy_scan_buffer() failed";
		return false;
	}

	int iRes = yyparse ( &tParser );
	yy_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yylex_destroy ( tParser.m_pScanner );

	dStmt.Pop(); // last query is always dummy

	if ( dStmt.GetLength()>1 )
	{
		sError.SetSprintf ( "internal error: too many filter statements, got %d", dStmt.GetLength() );
		return false;
	}

	if ( dStmt.GetLength() && dStmt[0].m_eStmt!=STMT_SYSFILTERS )
	{
		sError.SetSprintf ( "internal error: not filter statement parsed, got %d", dStmt[0].m_eStmt );
		return false;
	}

	if ( dStmt.GetLength() )
	{
		CSphQuery & tQuery = dStmt[0].m_tQuery;

		int iFilterCount = tParser.m_dFiltersPerStmt[0];
		CreateFilterTree ( tParser.m_dFilterTree, 0, iFilterCount, tQuery );

		dFilters.SwapData ( tQuery.m_dFilters );
		dFilterTree.SwapData ( tQuery.m_dFilterTree );
	}

	// maybe its better to create real filter instead of just checking column name
	if ( iRes==0 && dFilters.GetLength() )
	{
		ARRAY_FOREACH ( i, dFilters )
		{
			const CSphFilterSettings & tFilter = dFilters[i];
			if ( tFilter.m_sAttrName.IsEmpty() )
			{
				sError.SetSprintf ( "bad filter %d name", i );
				return false;
			}

			if ( tFilter.m_sAttrName.Begins ( "@" ) )
			{
				sError.SetSprintf ( "unsupported filter column '%s'", tFilter.m_sAttrName.cstr() );
				return false;
			}

			const char * sAttrName = tFilter.m_sAttrName.cstr();

			// might be a JSON.field
			CSphString sJsonField;
			const char * sJsonDot = strchr ( sAttrName, '.' );
			if ( sJsonDot )
			{
				assert ( sJsonDot>sAttrName );
				sJsonField.SetBinary ( sAttrName, int ( sJsonDot - sAttrName ) );
				sAttrName = sJsonField.cstr();
			}

			int iCol = tSchema.GetAttrIndex ( sAttrName );
			if ( iCol==-1 )
			{
				sError.SetSprintf ( "no such filter attribute '%s'", sAttrName );
				return false;
			}
		}
	}

	// TODO: change way of filter -> expression create: produce single error, share parser code
	// try expression
	if ( iRes!=0 && !dFilters.GetLength() && sError.Begins ( "percolate filters: syntax error" ) )
	{
		ESphAttr eAttrType = SPH_ATTR_NONE;
		ExprParseArgs_t tExprArgs;
		tExprArgs.m_pAttrType = &eAttrType;
		tExprArgs.m_eCollation = eCollation;
		ISphExprRefPtr_c pExpr { sphExprParse ( sFilters, tSchema, sError, tExprArgs ) };
		if ( pExpr )
		{
			sError = "";
			iRes = 0;
			CSphFilterSettings & tExpr = dFilters.Add();
			tExpr.m_eType = SPH_FILTER_EXPRESSION;
			tExpr.m_sAttrName = sFilters;
		} else
		{
			return false;
		}
	}

	return ( iRes==0 );
}
