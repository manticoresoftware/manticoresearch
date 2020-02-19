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

#ifndef _searchdddl_
#define _searchdddl_

#include "sphinx.h"
#include "searchdsql.h"


class DdlParser_c : public SqlParserTraits_c
{
public:
			DdlParser_c ( CSphVector<SqlStmt_t> & dStmt );

	void	AddFieldFlag ( DWORD uFlag );
	void	AddCreateTableCol ( const SqlNode_t & tCol, ESphAttr eAttrType );
	void	AddCreateTableBitCol ( const SqlNode_t & tCol, int iBits );
	void	AddCreateTableField ( const SqlNode_t & tCol );
	void	AddCreateTableOption ( const SqlNode_t & tName, const SqlNode_t & tValue );

	void	JoinClusterAt ( const SqlNode_t & tAt );

	void	AddInsval ( CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode );

private:
	DWORD	m_uFieldFlags = 0;
};


bool ParseDdl ( const char * sQuery, int iLen, CSphVector<SqlStmt_t> & dStmt, CSphString & sError );
bool IsDdlQuery ( const char * szQuery, int iLen );

#endif // _searchdddl_
