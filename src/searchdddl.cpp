//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
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

#define YYSTYPE SqlNode_t

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_DECL static int my_lex ( YYSTYPE * lvalp, void * yyscanner, DdlParser_c * pParser )

#if USE_WINDOWS
	#define YY_NO_UNISTD_H 1
#endif

#ifdef CMAKE_GENERATED_LEXER
	#ifdef __GNUC__
		#pragma GCC diagnostic push 
		#pragma GCC diagnostic ignored "-Wsign-compare"
		#pragma GCC diagnostic ignored "-Wpragmas"
		#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
	#endif

	#include "flexddl.c"

	#ifdef __GNUC__
		#pragma GCC diagnostic pop
	#endif

#else
	#include "llddl.c"
#endif


void yyerror ( DdlParser_c * pParser, const char * sMessage )
{
	// flex put a zero at last token boundary; make it undo that
	yy3lex_unhold ( pParser->m_pScanner );

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

#ifdef CMAKE_GENERATED_GRAMMAR
	#include "bisddl.c"
#else
	#include "yyddl.c"
#endif

//////////////////////////////////////////////////////////////////////////

DdlParser_c::DdlParser_c ( CSphVector<SqlStmt_t> & dStmt )
	: SqlParserTraits_c ( dStmt )
{
	assert ( !m_dStmt.GetLength() );
	PushQuery ();
}


void DdlParser_c::AddCreateTableBitCol ( const SqlNode_t & tCol, int iBits )
{
	assert(m_pStmt);
	CSphColumnInfo & tAttr = m_pStmt->m_tCreateTable.m_dAttrs.Add();
	ToString ( tAttr.m_sName, tCol );
	tAttr.m_sName.ToLower();
	tAttr.m_eAttrType = SPH_ATTR_INTEGER;
	tAttr.m_tLocator.m_iBitCount = iBits;
}


void DdlParser_c::AddField ( const CSphString & sName, DWORD uFlags )
{
	assert(m_pStmt);
	auto & tField = m_pStmt->m_tCreateTable.m_dFields.Add();
	tField.m_sName = sName;
	tField.m_uFieldFlags = uFlags;
}


DWORD ConvertFlags (int iFlags)
{
	// convert flags;
	DWORD uFieldFlags = 0;
	uFieldFlags |= ( iFlags & DdlParser_c::FLAG_INDEXED ) ? CSphColumnInfo::FIELD_INDEXED : 0;
	uFieldFlags |= ( iFlags & DdlParser_c::FLAG_STORED ) ? CSphColumnInfo::FIELD_STORED : 0;
	uFieldFlags |= ( iFlags & DdlParser_c::FLAG_ATTRIBUTE ) ? CSphColumnInfo::FIELD_IS_ATTRIBUTE : 0;

	return uFieldFlags;
}


bool DdlParser_c::AddCreateTableCol ( const SqlNode_t & tName, const SqlNode_t & tCol, AttrEngine_e eEngine )
{
	assert( m_pStmt );

	CSphString sName;
	ToString ( sName, tName );
	sName.ToLower ();

	auto eAttrType = (ESphAttr) tCol.m_iValue;
	auto iType = tCol.m_iType;

	if ( eAttrType!=SPH_ATTR_STRING )
	{
		if ( iType )
		{
			m_sError.SetSprintf ( "options 'attribute', 'stored', 'indexed' are not appliable to non-string column '%s'", sName.cstr () );
			return false;
		}

		CSphColumnInfo & tAttr = m_pStmt->m_tCreateTable.m_dAttrs.Add();
		tAttr.m_sName = sName;
		tAttr.m_eAttrType = eAttrType;
		tAttr.m_eEngine = eEngine;
		return true;
	}

	// actually, this may or may not be a field
	// it all depends on the combination of flags provided
	assert ( eAttrType==SPH_ATTR_STRING );
	if ( iType & FLAG_ATTRIBUTE )
	{
		if ( iType & FLAG_STORED )
		{
			m_sError.SetSprintf ( "unable to create a stored attribute '%s'", sName.cstr() );
			return false;
		}

		// add attribute
		CSphColumnInfo & tAttr = m_pStmt->m_tCreateTable.m_dAttrs.Add();
		tAttr.m_sName = sName;
		tAttr.m_eAttrType = SPH_ATTR_STRING;
		tAttr.m_eEngine = eEngine;

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


bool DdlParser_c::AddCreateTableCol ( const SqlNode_t & tName, const SqlNode_t & tCol, const SqlNode_t & tEngine )
{
	AttrEngine_e eEngine = AttrEngine_e::DEFAULT;

	CSphString sEngine = ToStringUnescape(tEngine);
	CSphString sEngineLowerCase = sEngine;
	sEngineLowerCase.ToLower();

	if ( !StrToAttrEngine ( eEngine, sEngineLowerCase, m_sError ) )
		return false;

	return AddCreateTableCol ( tName, tCol, eEngine );
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
	tIns.m_iVal = tNode.m_iValue; // OPTIMIZE? copy conditionally based on type?
	tIns.m_fVal = tNode.m_fValue;
	if ( tIns.m_iType==TOK_QUOTED_STRING )
		tIns.m_sVal = ToStringUnescape ( tNode );
	tIns.m_pVals = tNode.m_pValues;
}


//////////////////////////////////////////////////////////////////////////

bool ParseDdl ( const char * sQuery, int iLen, CSphVector<SqlStmt_t> & dStmt, CSphString & sError )
{
	if ( !sQuery || !iLen )
	{
		sError = "query was empty";
		return false;
	}

	DdlParser_c tParser(dStmt);
	tParser.m_pBuf = sQuery;
	tParser.m_pLastTokenStart = NULL;
	tParser.m_pParseError = &sError;

	char * sEnd = const_cast<char *>( sQuery ) + iLen;
	sEnd[0] = 0; // prepare for yy_scan_buffer
	sEnd[1] = 0; // this is ok because string allocates a small gap

	yy3lex_init ( &tParser.m_pScanner );
	YY_BUFFER_STATE tLexerBuffer = yy3_scan_buffer ( const_cast<char *>( sQuery ), iLen+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: yy3_scan_buffer() failed";
		return false;
	}

	int iRes = yyparse ( &tParser );

	yy3_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yy3lex_destroy ( tParser.m_pScanner );

	dStmt.Pop(); // last query is always dummy

	if ( iRes!=0 || !dStmt.GetLength() )
		return false;

	return true;
}


const char * g_szDDL[] = { "alter", "create", "drop", "join", "import" };

bool IsDdlQuery ( const char * szQuery, int iLen )
{
	CSphString sQuery;
	sQuery.SetBinary ( szQuery, iLen );

	const char * p = sQuery.cstr();
	while ( *p && ( sphIsSpace(*p) || *p==';' ) )
		p++;

	const char * pStart = p;
	while ( *p && sphIsAlpha(*p) )
		p++;

	CSphString sStatement;
	sStatement.SetBinary ( pStart, p-pStart );
	sStatement.ToLower();

	for ( const auto & i : g_szDDL )
		if ( sStatement==i )
			return true;

	return false;
}
