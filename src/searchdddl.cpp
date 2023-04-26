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

#include "searchdddl.h"

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

#define YYSTYPE SqlNode_t

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_DECL static int my_lex ( YYSTYPE * lvalp, void * yyscanner, DdlParser_c * pParser )

#if _WIN32
	#define YY_NO_UNISTD_H 1
#endif

#include "flexddl.c"

static void yyerror ( DdlParser_c * pParser, const char * sMessage )
{
	// flex put a zero at last token boundary; make it undo that
	yy3lex_unhold ( pParser->m_pScanner );

	// 'wrong parser' is quite empiric - we fire it when from very beginning parser sees syntax error
	pParser->ProcessParsingError ( sMessage );
}


#ifndef NDEBUG
// using a proxy to be possible to debug inside yylex
static int yylex ( YYSTYPE * lvalp, DdlParser_c * pParser )
{
	int res = my_lex ( lvalp, pParser->m_pScanner, pParser );
	return res;
}
#else
static int yylex ( YYSTYPE * lvalp, DdlParser_c * pParser )
{
	return my_lex ( lvalp, pParser->m_pScanner, pParser );
}
#endif

#include "bisddl.c"

//////////////////////////////////////////////////////////////////////////

void DdlParser_c::ItemOptions_t::Reset()
{
	m_eEngine = AttrEngine_e::DEFAULT;
	m_bStringHash = true;
	m_bFastFetch = true;

	m_bHashOptionSet = false;
}


DWORD DdlParser_c::ItemOptions_t::ToFlags() const
{
	DWORD uFlags = 0;
	uFlags |= m_bStringHash ? CSphColumnInfo::ATTR_COLUMNAR_HASHES : 0;
	uFlags |= m_bFastFetch ? CSphColumnInfo::ATTR_STORED : 0;
	return uFlags;
}

//////////////////////////////////////////////////////////////////////////

DdlParser_c::DdlParser_c ( CSphVector<SqlStmt_t> & dStmt, const char* szQuery, CSphString* pError )
	: SqlParserTraits_c ( dStmt, szQuery, pError )
{
	if ( m_dStmt.IsEmpty() )
		PushQuery ();
	else
		m_pStmt = &m_dStmt.Last();
	assert ( m_dStmt.GetLength()==1 );
}


void DdlParser_c::AddCreateTableBitCol ( const SqlNode_t & tCol, int iBits )
{
	assert(m_pStmt);
	CreateTableAttr_t & tAttr = m_pStmt->m_tCreateTable.m_dAttrs.Add();
	ToString ( tAttr.m_tAttr.m_sName, tCol );
	tAttr.m_tAttr.m_sName.ToLower();
	tAttr.m_tAttr.m_eAttrType = SPH_ATTR_INTEGER;
	tAttr.m_tAttr.m_tLocator.m_iBitCount = iBits;
	tAttr.m_tAttr.m_eEngine = m_tItemOptions.m_eEngine;
	tAttr.m_bFastFetch = m_tItemOptions.m_bFastFetch;
	tAttr.m_bStringHash	= m_tItemOptions.m_bStringHash;

	m_tItemOptions.Reset();
}


void DdlParser_c::AddField ( const CSphString & sName, DWORD uFlags )
{
	assert(m_pStmt);
	auto & tField = m_pStmt->m_tCreateTable.m_dFields.Add();
	tField.m_sName = sName;
	tField.m_uFieldFlags = uFlags;
}


static DWORD ConvertFlags ( int iFlags )
{
	// convert flags;
	DWORD uFieldFlags = 0;
	uFieldFlags |= ( iFlags & DdlParser_c::FLAG_INDEXED ) ? CSphColumnInfo::FIELD_INDEXED : 0;
	uFieldFlags |= ( iFlags & DdlParser_c::FLAG_STORED ) ? CSphColumnInfo::FIELD_STORED : 0;
	uFieldFlags |= ( iFlags & DdlParser_c::FLAG_ATTRIBUTE ) ? CSphColumnInfo::FIELD_IS_ATTRIBUTE : 0;

	return uFieldFlags;
}


bool DdlParser_c::CheckFieldFlags ( ESphAttr eAttrType, int iFlags, const CSphString & sName, const ItemOptions_t & tOpts, CSphString & sError )
{
	if ( eAttrType==SPH_ATTR_STRING )
	{
		if ( ( iFlags & FLAG_ATTRIBUTE ) && ( iFlags & FLAG_STORED ) )
		{
			sError.SetSprintf ( "unable to create a stored attribute '%s'", sName.cstr() );
			return false;
		}
	}
	else
	{
		if ( iFlags )
		{
			sError.SetSprintf ( "options 'attribute', 'stored', 'indexed' are not applicable to non-string column '%s'", sName.cstr() );
			return false;
		}

		if ( tOpts.m_bHashOptionSet )
		{
			sError.SetSprintf ( "'hash' is applicable to columnar strings only (attempted to set for '%s')", sName.cstr() );
			return false;

		}
	}

	return true;
}


bool DdlParser_c::SetupAlterTable ( const SqlNode_t & tIndex, const SqlNode_t & tAttr, ESphAttr eAttr, int iFieldFlags, int iBits )
{
	assert( m_pStmt );
	ItemOptions_t tOpts = m_tItemOptions;

	m_pStmt->m_eStmt = STMT_ALTER_ADD;
	ToString ( m_pStmt->m_sIndex, tIndex );
	ToString ( m_pStmt->m_sAlterAttr, tAttr );
	m_pStmt->m_sIndex.ToLower();
	m_pStmt->m_sAlterAttr.ToLower();
	m_pStmt->m_eAlterColType = eAttr;
	m_pStmt->m_uFieldFlags = ConvertFlags(iFieldFlags);
	m_pStmt->m_uAttrFlags = m_tItemOptions.ToFlags();
	m_pStmt->m_eEngine = tOpts.m_eEngine;
	m_pStmt->m_iBits = iBits;
	m_tItemOptions.Reset();

	return CheckFieldFlags ( m_pStmt->m_eAlterColType, iFieldFlags, m_pStmt->m_sAlterAttr, tOpts, m_sError );
}


bool DdlParser_c::SetupAlterTable  ( const SqlNode_t & tIndex, const SqlNode_t & tAttr, const SqlNode_t & tType )
{
	return SetupAlterTable ( tIndex, tAttr, (ESphAttr)tType.GetValueInt(), tType.m_iType );
}


bool DdlParser_c::AddCreateTableCol ( const SqlNode_t & tName, const SqlNode_t & tCol )
{
	assert( m_pStmt );

	CSphString sName;
	ToString ( sName, tName );
	sName.ToLower ();

	auto eAttrType = (ESphAttr) tCol.GetValueInt();
	auto iType = tCol.m_iType;

	ItemOptions_t tOpts = m_tItemOptions;
	m_tItemOptions.Reset();

	if ( !CheckFieldFlags ( eAttrType, iType, sName, tOpts, m_sError ) )
		return false;

	if ( eAttrType!=SPH_ATTR_STRING )
	{
		CreateTableAttr_t & tAttr = m_pStmt->m_tCreateTable.m_dAttrs.Add();
		tAttr.m_tAttr.m_sName		= sName;
		tAttr.m_tAttr.m_eAttrType	= eAttrType;
		tAttr.m_tAttr.m_eEngine		= tOpts.m_eEngine;
		tAttr.m_bFastFetch			= tOpts.m_bFastFetch;
		tAttr.m_bStringHash			= tOpts.m_bStringHash;
		return true;
	}

	// actually, this may or may not be a field
	// it all depends on the combination of flags provided
	assert ( eAttrType==SPH_ATTR_STRING );
	if ( iType & FLAG_ATTRIBUTE )
	{
		// add attribute
		CreateTableAttr_t & tAttr = m_pStmt->m_tCreateTable.m_dAttrs.Add();
		tAttr.m_tAttr.m_sName		= sName;
		tAttr.m_tAttr.m_eAttrType	= SPH_ATTR_STRING;
		tAttr.m_tAttr.m_eEngine		= tOpts.m_eEngine;
		tAttr.m_bFastFetch			= tOpts.m_bFastFetch;
		tAttr.m_bStringHash			= tOpts.m_bStringHash;

		if ( iType & FLAG_INDEXED )
			AddField ( sName, CSphColumnInfo::FIELD_INDEXED );
	}
	else
	{
		// convert flags;
		DWORD uFieldFlags = 0;
		uFieldFlags |= ( iType & FLAG_INDEXED ) ? CSphColumnInfo::FIELD_INDEXED : 0;
		uFieldFlags |= ( iType & FLAG_STORED ) ? CSphColumnInfo::FIELD_STORED : 0;

		if ( !uFieldFlags )
			uFieldFlags = CSphColumnInfo::FIELD_INDEXED | CSphColumnInfo::FIELD_STORED;

		AddField ( sName, uFieldFlags );
	}

	return true;
}


bool DdlParser_c::AddCreateTableId ( const SqlNode_t & tName )
{
	assert( m_pStmt );

	CSphString sName;
	ToString ( sName, tName );
	sName.ToLower();

	ItemOptions_t tOpts = m_tItemOptions;
	m_tItemOptions.Reset();

	if ( sName!="id" )
	{
		m_sError.SetSprintf ( "expected 'id', got '%s'", sName.cstr() );
		return false;
	}

	if ( tOpts.m_bHashOptionSet )
	{
		m_sError = "cannot set 'hash' option for 'id'";
		return false;
	}

	CreateTableAttr_t & tAttr = m_pStmt->m_tCreateTable.m_dAttrs.Add();
	tAttr.m_tAttr.m_sName		= sName;
	tAttr.m_tAttr.m_eAttrType	= SPH_ATTR_BIGINT;
	tAttr.m_tAttr.m_eEngine		= tOpts.m_eEngine;
	tAttr.m_bFastFetch			= tOpts.m_bFastFetch;
	return true;
}


bool DdlParser_c::AddItemOptionEngine ( const SqlNode_t & tOption )
{
	return ConvertToAttrEngine ( tOption, m_tItemOptions.m_eEngine );
}


bool DdlParser_c::AddItemOptionHash ( const SqlNode_t & tOption )
{
	CSphString sValue = ToStringUnescape(tOption);
	m_tItemOptions.m_bStringHash = !!strtoull ( sValue.cstr(), NULL, 10 );
	m_tItemOptions.m_bHashOptionSet = true;
	return true;
}


bool DdlParser_c::AddItemOptionFastFetch ( const SqlNode_t & tOption )
{
	CSphString sValue = ToStringUnescape(tOption);
	m_tItemOptions.m_bFastFetch = !!strtoull ( sValue.cstr(), NULL, 10 );
	return true;
}


bool DdlParser_c::ConvertToAttrEngine ( const SqlNode_t & tEngine, AttrEngine_e & eEngine )
{
	CSphString sEngine = ToStringUnescape(tEngine);
	CSphString sEngineLowerCase = sEngine;
	sEngineLowerCase.ToLower();

	return StrToAttrEngine ( eEngine, AttrEngine_e::DEFAULT, sEngineLowerCase, m_sError );
}


void DdlParser_c::AddCreateTableOption ( const SqlNode_t & tName, const SqlNode_t & tValue )
{
	assert(m_pStmt);
	NameValueStr_t & tOpt = m_pStmt->m_tCreateTable.m_dOpts.Add();

	ToString ( tOpt.m_sName, tName );
	tOpt.m_sValue = ToStringUnescape(tValue);
	tOpt.m_sName.ToLower();
}


void DdlParser_c::JoinClusterAt ( const SqlNode_t & tAt )
{
	assert(m_pStmt);
	m_pStmt->m_bClusterUpdateNodes = true;

	m_pStmt->m_dCallOptNames.Add ( "at_node" );

	SqlInsert_t & tVal = m_pStmt->m_dCallOptValues.Add();
	tVal.m_iType = tAt.m_iType;
	tVal.m_sVal = ToStringUnescape ( tAt );
}


void DdlParser_c::AddInsval ( CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode )
{
	SqlInsert_t & tIns = dVec.Add();
	tIns.m_iType = tNode.m_iType;
	tIns.CopyValueInt(tNode);
	tIns.m_fVal = tNode.m_fValue;
	if ( tIns.m_iType==TOK_QUOTED_STRING )
		tIns.m_sVal = ToStringUnescape ( tNode );
	tIns.m_pVals = tNode.m_pValues;
}


//////////////////////////////////////////////////////////////////////////

bool ParseDdl ( Str_t sQuery, CSphVector<SqlStmt_t>& dStmt, CSphString& sError )
{
	if ( !IsFilled ( sQuery ) )
	{
		sError = "query was empty";
		return false;
	}

	auto* sEnd = const_cast<char*> ( end ( sQuery ) );
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	return ParseResult_e::PARSE_OK == ParseDdlEx ( sQuery, dStmt, sError );
}

ParseResult_e ParseDdlEx ( Str_t sQuery, CSphVector<SqlStmt_t> & dStmt, CSphString & sError )
{
	assert ( IsFilled ( sQuery ) );

	DdlParser_c tParser { dStmt, sQuery.first, &sError };

	yy3lex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy3_scan_buffer ( const_cast<char *>( sQuery.first ), sQuery.second+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: yy3_scan_buffer() failed";
		return ParseResult_e::PARSE_ERROR;
	}

	int iRes = yyparse ( &tParser );

	yy3_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yy3lex_destroy ( tParser.m_pScanner );

	dStmt.Pop(); // last query is always dummy
	if ( tParser.IsWrongSyntaxError() )
		return ParseResult_e::PARSE_SYNTAX_ERROR;

	return ( iRes || dStmt.IsEmpty() ) ? ParseResult_e::PARSE_ERROR : ParseResult_e::PARSE_OK;
}
