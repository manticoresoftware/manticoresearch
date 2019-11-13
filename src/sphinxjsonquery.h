//
// $Id$
//

//
// Copyright (c) 2017, Manticore Software LTD (http://manticoresearch.com)
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
struct cJSON;
struct XQNode_t;
struct SqlStmt_t;

QueryParser_i *	sphCreateJsonQueryParser();
bool			sphParseJsonQuery ( const char * szQuery, CSphQuery & tQuery, bool & bProfile, CSphString & sError, CSphString & sWarning );
bool			sphParseJsonInsert ( const char * szInsert, SqlStmt_t & tStmt, DocID_t & tDocId, bool bReplace, CSphString & sError );
bool			sphParseJsonUpdate ( const char * szUpdate, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError );
bool			sphParseJsonDelete ( const char * szDelete, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError );
bool			sphParseJsonStatement ( const char * szStmt, SqlStmt_t & tStmt, CSphString & sStmt, CSphString & sQuery, DocID_t & tDocId, CSphString & sError );

CSphString		sphEncodeResultJson ( const AggrResult_t & tRes, const CSphQuery & tQuery, CSphQueryProfile * pProfile );
JsonObj_c		sphEncodeInsertResultJson ( const char * szIndex, bool bReplace, DocID_t tDocId );
JsonObj_c		sphEncodeUpdateResultJson ( const char * szIndex, DocID_t tDocId, int iAffected );
JsonObj_c 		sphEncodeDeleteResultJson ( const char * szIndex, DocID_t tDocId, int iAffected );
JsonObj_c		sphEncodeInsertErrorJson ( const char * szIndex, const char * szError );

bool			sphGetResultStats ( const char * szResult, int & iAffected, int & iWarnings, bool bUpdate );

void			sphBuildProfileJson ( JsonEscapedBuilder &tOut, const XQNode_t * pNode, const CSphSchema &tSchema, const StrVec_t &dZones );

bool			ParseJsonQueryFilters ( const JsonObj_c & tJson, CSphQuery & tQuery, CSphString & sError, CSphString & sWarning );
bool			NonEmptyQuery ( const JsonObj_c & tQuery );

#endif

//
// $Id$
//
