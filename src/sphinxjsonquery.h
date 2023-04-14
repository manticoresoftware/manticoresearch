//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

class QueryParser_i;
class StmtErrorReporter_i;
class QueryProfile_c;
struct cJSON;
struct XQNode_t;
struct SqlStmt_t;

struct JsonAggr_t
{
	CSphString	m_sBucketName;
	CSphString	m_sCol;
	int			m_iSize = 0;
	ESphAggrFunc m_eAggrFunc { SPH_AGGR_NONE };
	CSphVector<JsonAggr_t> m_dNested;
	CSphString	m_sSort;

	CSphString GetAliasName () const;
	CSphString GetExpr () const;
};

/// search query. Pure struct, no member functions
struct JsonQuery_c : public CSphQuery
{
	StrVec_t m_dSortFields;
	StrVec_t m_dDocFields;
	CSphVector<JsonAggr_t> m_dAggs;
};


std::unique_ptr<QueryParser_i>	sphCreateJsonQueryParser();
bool			sphParseJsonQuery ( Str_t sQuery, JsonQuery_c & tQuery, bool & bProfile, CSphString & sError, CSphString & sWarning );
bool			sphParseJsonQuery ( const JsonObj_c & tRoot, JsonQuery_c & tQuery, bool & bProfile, CSphString & sError, CSphString & sWarning );
bool			sphParseJsonInsert ( const char * szInsert, SqlStmt_t & tStmt, DocID_t & tDocId, bool bReplace, bool bCompat, CSphString & sError );
bool			sphParseJsonUpdate ( Str_t sUpdate, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError );
bool			sphParseJsonDelete ( Str_t sDelete, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError );
bool			sphParseJsonStatement ( const char * szStmt, SqlStmt_t & tStmt, CSphString & sStmt, CSphString & sQuery, DocID_t & tDocId, CSphString & sError );

CSphString		sphEncodeResultJson ( const VecTraits_T<const AggrResult_t *> & dRes, const JsonQuery_c & tQuery, QueryProfile_c * pProfile, bool bCompat );
JsonObj_c		sphEncodeInsertResultJson ( const char * szIndex, bool bReplace, DocID_t tDocId );
JsonObj_c		sphEncodeUpdateResultJson ( const char * szIndex, DocID_t tDocId, int iAffected );
JsonObj_c 		sphEncodeDeleteResultJson ( const char * szIndex, DocID_t tDocId, int iAffected );
JsonObj_c		sphEncodeInsertErrorJson ( const char * szIndex, const char * szError );
JsonObj_c		sphEncodeTxnResultJson ( const char* szIndex, DocID_t tDocId, int iInserts, int iDeletes, int iUpdates );

bool			sphGetResultStats ( const char * szResult, int & iAffected, int & iWarnings, bool bUpdate );

bool			NonEmptyQuery ( const JsonObj_c & tQuery );

const CSphString & CompatDateFormat();

CSphString JsonEncodeResultError ( const CSphString & sError, const char * sErrorType, int iStatus );
CSphString JsonEncodeResultError ( const CSphString & sError, const char * sErrorType, int iStatus, const char * sIndex );
CSphString JsonEncodeResultError ( const CSphString & sError, int iStatus );

bool ParseJsonInsertSource ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, bool bReplace, bool bCompat, CSphString & sError );
bool ParseJsonUpdate ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError );

#endif

