//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "knnmisc.h"


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
		bool			m_bIndexed = false;

		bool			m_bHashOptionSet = false;

		CSphString		m_sKNNType;
		int				m_iKNNDims = 0;
		int				m_iHNSWM = 16;
		int				m_iHNSWEFConstruction = 200;
		knn::HNSWSimilarity_e m_eHNSWSimilarity = knn::HNSWSimilarity_e::L2;
		knn::Quantization_e m_eQuantization = knn::Quantization_e::NONE;
		bool			m_bKNNDimsSpecified = false;
		bool			m_bHNSWSimilaritySpecified = false;

		void			Reset()	{ *this = ItemOptions_t(); }
		DWORD			ToFlags() const;
		knn::IndexSettings_t ToKNN() const;
		void			CopyOptionsTo ( CreateTableAttr_t & tAttr ) const;
	};

	DdlParser_c ( CSphVector<SqlStmt_t>& dStmt, const char* szQuery, CSphString* pError );

	const char * GetLastError() const { return m_sError.scstr(); }

	bool	AddCreateTableCol ( const SqlNode_t & tName, const SqlNode_t & tCol );
	bool	AddCreateTableId ( const SqlNode_t & tName );
	void	AddCreateTableBitCol ( const SqlNode_t & tCol, int iBits );

	bool	AddItemOptionEngine ( const SqlNode_t & tOption );
	bool	AddItemOptionHash ( const SqlNode_t & tOption );
	bool	AddItemOptionFastFetch ( const SqlNode_t & tOption );
	bool	AddItemOptionIndexed ( const SqlNode_t & tOption );

	bool	AddItemOptionKNNType ( const SqlNode_t & tOption );
	bool	AddItemOptionKNNDims ( const SqlNode_t & tOption );
	bool	AddItemOptionHNSWSimilarity ( const SqlNode_t & tOption );
	bool	AddItemOptionHNSWM ( const SqlNode_t & tOption );
	bool	AddItemOptionHNSWEfConstruction ( const SqlNode_t & tOption );
	bool	AddItemOptionQuantization ( const SqlNode_t & tOption );

	void	AddCreateTableOption ( const SqlNode_t & tName, const SqlNode_t & tValue );
	bool	SetupAlterTable  ( const SqlNode_t & tIndex, const SqlNode_t & tAttr, const SqlNode_t & tType, bool bModify = false );
	bool	SetupAlterTable ( const SqlNode_t & tIndex, const SqlNode_t & tAttr, ESphAttr eAttr, int iFieldFlags, int iBits=-1, bool bModify = false );

	void	JoinClusterAt ( const SqlNode_t & tAt );

	void	AddInsval ( CSphVector<SqlInsert_t> & dVec, const SqlNode_t & tNode );

private:
	CSphString		m_sError;
	ItemOptions_t	m_tItemOptions;

	void	AddField ( const CSphString & sName, DWORD uFlags );
	bool	ConvertToAttrEngine ( const SqlNode_t & tEngine, AttrEngine_e & eEngine );
	static bool CheckFieldFlags ( ESphAttr eAttrType, int iFlags, const CSphString & sName, const ItemOptions_t & tOpts, CSphString & sError );
};

using YYSTYPE = SqlNode_t;
STATIC_ASSERT ( IS_TRIVIALLY_COPYABLE ( SqlNode_t ), YYSTYPE_MUST_BE_TRIVIAL_FOR_RESIZABLE_PARSER_STACK );
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1

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

DWORD DdlParser_c::ItemOptions_t::ToFlags() const
{
	DWORD uFlags = 0;
	uFlags |= m_bStringHash ? CSphColumnInfo::ATTR_COLUMNAR_HASHES : 0;
	uFlags |= m_bFastFetch ? CSphColumnInfo::ATTR_STORED : 0;
	uFlags |= m_bIndexed ? CSphColumnInfo::ATTR_INDEXED_SI : 0;
	uFlags |= m_sKNNType.IsEmpty() ? 0 : CSphColumnInfo::ATTR_INDEXED_KNN;
	return uFlags;
}


knn::IndexSettings_t DdlParser_c::ItemOptions_t::ToKNN() const
{
	knn::IndexSettings_t tKNN;

	tKNN.m_iDims			= m_iKNNDims;
	tKNN.m_eHNSWSimilarity	= m_eHNSWSimilarity;
	tKNN.m_eQuantization	= m_eQuantization;
	tKNN.m_iHNSWM			= m_iHNSWM;
	tKNN.m_iHNSWEFConstruction = m_iHNSWEFConstruction;

	return tKNN;
}


void DdlParser_c::ItemOptions_t::CopyOptionsTo ( CreateTableAttr_t & tAttr ) const
{
	tAttr.m_tAttr.m_eEngine	= m_eEngine;
	tAttr.m_bFastFetch		= m_bFastFetch;
	tAttr.m_bStringHash		= m_bStringHash;
	tAttr.m_bIndexed		= m_bIndexed;
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
	m_sErrorHeader = "P03:";
	SetDefaultTableForOptions();
}


void DdlParser_c::AddCreateTableBitCol ( const SqlNode_t & tCol, int iBits )
{
	assert(m_pStmt);
	CreateTableAttr_t & tAttr = m_pStmt->m_tCreateTable.m_dAttrs.Add();
	ToString ( tAttr.m_tAttr.m_sName, tCol );
	tAttr.m_tAttr.m_sName.ToLower();
	tAttr.m_tAttr.m_eAttrType = SPH_ATTR_INTEGER;
	tAttr.m_tAttr.m_tLocator.m_iBitCount = iBits;
	m_tItemOptions.CopyOptionsTo(tAttr);
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
	if ( eAttrType!=SPH_ATTR_FLOAT_VECTOR && !tOpts.m_sKNNType.IsEmpty() )
	{
		sError = "knn_type='hnsw' can only be used with float_vector attributes";
		return false;
	}

	if ( eAttrType==SPH_ATTR_STRING )
	{
		if ( ( iFlags & FLAG_ATTRIBUTE ) && ( iFlags & FLAG_STORED ) )
		{
			sError.SetSprintf ( "unable to create a stored attribute '%s'", sName.cstr() );
			return false;
		}
	}
	else if ( eAttrType==SPH_ATTR_FLOAT_VECTOR )
	{
		if ( !tOpts.m_sKNNType.IsEmpty() && ( !tOpts.m_bKNNDimsSpecified || !tOpts.m_bHNSWSimilaritySpecified ) )
		{
			sError = "knn_dims and hnsw_similarity are required if knn_type='hnsw'";
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


bool DdlParser_c::SetupAlterTable ( const SqlNode_t & tIndex, const SqlNode_t & tAttr, ESphAttr eAttr, int iFieldFlags, int iBits, bool bModify )
{
	assert( m_pStmt );

	m_pStmt->m_eStmt = bModify ? STMT_ALTER_MODIFY : STMT_ALTER_ADD;
	ToString ( m_pStmt->m_sIndex, tIndex );
	ToString ( m_pStmt->m_sAlterAttr, tAttr );
	m_pStmt->m_sIndex.ToLower();
	m_pStmt->m_sAlterAttr.ToLower();
	m_pStmt->m_eAlterColType = eAttr;
	m_pStmt->m_uFieldFlags = ConvertFlags(iFieldFlags);
	m_pStmt->m_uAttrFlags = m_tItemOptions.ToFlags();
	m_pStmt->m_eEngine = m_tItemOptions.m_eEngine;
	m_pStmt->m_iBits = iBits;
	m_pStmt->m_tAlterKNN = m_tItemOptions.ToKNN();

	bool bOk = CheckFieldFlags ( m_pStmt->m_eAlterColType, iFieldFlags, m_pStmt->m_sAlterAttr, m_tItemOptions, m_sError );
	m_tItemOptions.Reset();

	return bOk;
}


bool DdlParser_c::SetupAlterTable  ( const SqlNode_t & tIndex, const SqlNode_t & tAttr, const SqlNode_t & tType, bool bModify )
{
	return SetupAlterTable ( tIndex, tAttr, (ESphAttr)tType.GetValueInt(), tType.m_iType, -1, bModify );
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
		tAttr.m_tAttr.m_sName			= sName;
		tAttr.m_tAttr.m_eAttrType		= eAttrType;
		tOpts.CopyOptionsTo(tAttr);
		tAttr.m_bKNN					= !tOpts.m_sKNNType.IsEmpty();
		tAttr.m_tKNN					= tOpts.ToKNN();

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
		tOpts.CopyOptionsTo(tAttr);

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
	tOpts.CopyOptionsTo(tAttr);
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


bool DdlParser_c::AddItemOptionIndexed ( const SqlNode_t & tOption )
{
	CSphString sValue = ToStringUnescape(tOption);
	m_tItemOptions.m_bIndexed = !!strtoull ( sValue.cstr(), NULL, 10 );
	return true;
}


bool DdlParser_c::AddItemOptionKNNType ( const SqlNode_t & tOption )
{
	m_tItemOptions.m_sKNNType = ToStringUnescape(tOption).ToUpper();
	if ( m_tItemOptions.m_sKNNType!="HNSW" )
	{
		m_sError.SetSprintf ( "Unknown KNN type '%s'", m_tItemOptions.m_sKNNType.cstr() );
		return false;
	}

	return true;
}


bool DdlParser_c::AddItemOptionKNNDims ( const SqlNode_t & tOption )
{
	CSphString sValue = ToStringUnescape(tOption);
	m_tItemOptions.m_iKNNDims = strtoull ( sValue.cstr(), NULL, 10 );
	m_tItemOptions.m_bKNNDimsSpecified = true;
	return true;
}


bool DdlParser_c::AddItemOptionHNSWSimilarity ( const SqlNode_t & tOption )
{
	CSphString sValue = ToStringUnescape(tOption).ToUpper();
	if ( !Str2HNSWSimilarity ( sValue, m_tItemOptions.m_eHNSWSimilarity, &m_sError ) )
		return false;

	m_tItemOptions.m_bHNSWSimilaritySpecified = true;
	return true;
}


bool DdlParser_c::AddItemOptionHNSWM ( const SqlNode_t & tOption )
{
	CSphString sValue = ToStringUnescape(tOption);
	m_tItemOptions.m_iHNSWM = strtoull ( sValue.cstr(), NULL, 10 );
	return true;
}


bool DdlParser_c::AddItemOptionHNSWEfConstruction ( const SqlNode_t & tOption )
{
	CSphString sValue = ToStringUnescape(tOption);
	m_tItemOptions.m_iHNSWEFConstruction = strtoull ( sValue.cstr(), NULL, 10 );
	return true;
}


bool DdlParser_c::AddItemOptionQuantization ( const SqlNode_t & tOption )
{
	return Str2Quantization ( ToStringUnescape(tOption), m_tItemOptions.m_eQuantization, &m_sError );
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
	tIns.m_pVals = CloneMvaVecPtr ( tNode.m_iValues );
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
