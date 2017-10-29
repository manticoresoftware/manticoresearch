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

class QueryParser_i;
class StmtErrorReporter_i;
struct cJSON;
struct XQNode_t;
struct SqlStmt_t;

QueryParser_i *	sphCreateJsonQueryParser();
bool			sphParseJsonQuery ( const char * szQuery, CSphQuery & tQuery, bool & bProfile, bool & bAttrsHighlight, CSphString & sError, CSphString & sWarning );
bool			sphParseJsonInsert ( const char * szInsert, SqlStmt_t & tStmt, SphDocID_t & tDocId, bool bReplace, CSphString & sError );
bool			sphParseJsonUpdate ( const char * szUpdate, SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError );
bool			sphParseJsonDelete ( const char * szDelete, SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError );
bool			sphParseJsonStatement ( const char * szStmt, SqlStmt_t & tStmt, CSphString & sStmt, SphDocID_t & tDocId, CSphString & sError );
void			sphEncodeResultJson ( const AggrResult_t & tRes, const CSphQuery & tQuery, CSphQueryProfile * pProfile, bool bAttrsHighlight, CSphString & sResult );
cJSON *			sphEncodeInsertResultJson ( const char * szIndex, bool bReplace, SphDocID_t tDocId );
cJSON *			sphEncodeUpdateResultJson ( const char * szIndex, SphDocID_t tDocId, int iAffected );
cJSON *			sphEncodeDeleteResultJson ( const char * szIndex, SphDocID_t tDocId, int iAffected );
cJSON *			sphEncodeInsertErrorJson ( const char * szIndex, const char * szError );
cJSON *			sphBuildProfileJson ( XQNode_t * pNode, const CSphSchema & tSchema );
void			sphInitCJson();

int				PackSnippets ( const CSphVector<BYTE> & dRes, CSphVector<int> & dSeparators, int iSepLen, const BYTE ** ppStr );

#endif

//
// $Id$
//
