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

#ifndef _searchdddl_
#define _searchdddl_

#include "searchdsql.h"


class DdlParser_c : public SqlParserTraits_c
{
public:
	// this exists because we have separate field/attribute entities in the schema, but not in DDL
	enum
	{
		FLAG_NONE		= 0,
		FLAG_STORED		= 1<<0,
		FLAG_INDEXED	= 1<<1,
		FLAG_ATTRIBUTE	= 1<<2
	};

	struct ItemOptions_t
	{
		AttrEngine_e	m_eEngine = AttrEngine_e::DEFAULT;
		bool			m_bStringHash = true;
		bool			m_bFastFetch = true;

		bool			m_bHashOptionSet = false;

		void			Reset();
		DWORD			ToFlags() const;
	};

	DdlParser_c ( CSphVector<SqlStmt_t>& dStmt, const char* szQuery, CSphString* pError );

	const char * GetLastError() const { return m_sError.scstr(); }

	bool	AddCreateTableCol ( const SqlNode_t & tName, const SqlNode_t & tCol );
	bool	AddCreateTableId ( const SqlNode_t & tName );
	void	AddCreateTableBitCol ( const SqlNode_t & tCol, int iBits );

	bool	AddItemOptionEngine ( const SqlNode_t & tOption );
	bool	AddItemOptionHash ( const SqlNode_t & tOption );
	bool	AddItemOptionFastFetch ( const SqlNode_t & tOption );

	void	AddCreateTableOption ( const SqlNode_t & tName, const SqlNode_t & tValue );
	bool	SetupAlterTable  ( const SqlNode_t & tIndex, const SqlNode_t & tAttr, const SqlNode_t & tType );
	bool	SetupAlterTable ( const SqlNode_t & tIndex, const SqlNode_t & tAttr, ESphAttr eAttr, int iFieldFlags, int iBits=-1 );

	void	JoinClusterAt ( const SqlNode_t & tAt );

	void	AddInsval ( CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode );

private:
	CSphString		m_sError;
	ItemOptions_t	m_tItemOptions;

	void	AddField ( const CSphString & sName, DWORD uFlags );
	bool	ConvertToAttrEngine ( const SqlNode_t & tEngine, AttrEngine_e & eEngine );
	static bool CheckFieldFlags ( ESphAttr eAttrType, int iFlags, const CSphString & sName, const ItemOptions_t & tOpts, CSphString & sError );
};

bool ParseDdl ( const char * sQuery, int iLen, CSphVector<SqlStmt_t> & dStmt, CSphString & sError );

#endif // _searchdddl_
