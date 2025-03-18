//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxjsonquery_
#define _sphinxjsonquery_

#include "sphinx.h"
#include "sphinxjson.h"
#include "aggrexpr.h"

class QueryParser_i;
class StmtErrorReporter_i;
class QueryProfile_c;
struct cJSON;
struct XQNode_t;
struct SqlStmt_t;

struct AggrComposite_t
{
	CSphString m_sAlias;
	CSphString m_sColumn;
};

struct JsonAggr_t : public AggrSettings_t
{
	CSphString	m_sBucketName;
	CSphString	m_sCol;
	int			m_iSize = 0;
	CSphString	m_sSort;
	CSphVector<AggrComposite_t> m_dComposite;
	CSphVector<CSphFilterSettings> m_dCompositeAfterKey;

	CSphString GetAliasName () const;
};

struct JsonDocField_t
{
	CSphString m_sName;
	bool m_bDateTime = false;
};

/// search query. Pure struct, no member functions
struct JsonQuery_c : public CSphQuery
{
	StrVec_t m_dSortFields;
	CSphVector<JsonDocField_t> m_dDocFields;
	CSphVector<JsonAggr_t> m_dAggs;
	bool m_bGroupEmulation = false;
};

struct ParsedJsonQuery_t
{
				ParsedJsonQuery_t();

	JsonQuery_c	m_tQuery;
	CSphQuery	m_tJoinQueryOptions;
	CSphString	m_sWarning;
	bool		m_bProfile = false;
	int			m_iPlan = 0; // 0 - no plan, 1 - description, 2 - object, 3 - both
};

std::unique_ptr<QueryParser_i> sphCreateJsonQueryParser();
bool			sphParseJsonQuery ( Str_t sQuery, ParsedJsonQuery_t & tPJQuery );
bool			sphParseJsonQuery ( const JsonObj_c & tRoot, ParsedJsonQuery_t & tPJQuery );
bool			sphParseJsonInsert ( Str_t sInsert, SqlStmt_t & tStmt, DocID_t & tDocId, bool bReplace, CSphString & sError );
bool			sphParseJsonUpdate ( Str_t sUpdate, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError );
bool			sphParseJsonDelete ( Str_t sDelete, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError );
bool			sphParseJsonStatement ( const char * szStmt, SqlStmt_t & tStmt, CSphString & sStmt, CSphString & sQuery, DocID_t & tDocId, CSphString & sError );

enum class ResultSetFormat_e : bool
{
	ES,
	MntSearch
};

CSphString		sphEncodeResultJson ( const VecTraits_T<const AggrResult_t *> & dRes, const JsonQuery_c & tQuery, QueryProfile_c * pProfile, ResultSetFormat_e eFormat );
JsonObj_c		sphEncodeInsertResultJson ( const char * szIndex, bool bReplace, DocID_t tDocId, ResultSetFormat_e eFormat );
JsonObj_c		sphEncodeUpdateResultJson ( const char * szIndex, DocID_t tDocId, int iAffected, ResultSetFormat_e eFormat );
JsonObj_c 		sphEncodeDeleteResultJson ( const char * szIndex, DocID_t tDocId, int iAffected, ResultSetFormat_e eFormat );
JsonObj_c		sphEncodeInsertErrorJson ( const char * szIndex, const char * szError, ResultSetFormat_e eFormat );
JsonObj_c		sphEncodeTxnResultJson ( const char* szIndex, DocID_t tDocId, int iInserts, int iDeletes, int iUpdates, ResultSetFormat_e eFormat );

bool			sphGetResultStats ( const char * szResult, int & iAffected, int & iWarnings, bool bUpdate );

bool			NonEmptyQuery ( const JsonObj_c & tQuery );
bool			CheckRootNode ( const JsonObj_c & tRoot, CSphString & sError );

CSphString JsonEncodeResultError ( const CSphString & sError, const char * sErrorType, int iStatus );
CSphString JsonEncodeResultError ( const CSphString & sError, const char * sErrorType, int iStatus, const char * sIndex );
CSphString JsonEncodeResultError ( const CSphString & sError, int iStatus );

bool ParseJsonInsertSource ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, bool bReplace, CSphString & sError );
bool ParseJsonUpdate ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError );

#endif

