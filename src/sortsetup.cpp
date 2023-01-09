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

#include "sortsetup.h"

#include "sphinxjson.h"
#include "sphinxsort.h"


CSphMatchComparatorState::CSphMatchComparatorState()
{
	for ( int i=0; i<MAX_ATTRS; ++i )
	{
		m_eKeypart[i] = SPH_KEYPART_ROWID;
		m_dAttrs[i] = -1;
	}
}


bool CSphMatchComparatorState::UsesBitfields() const
{
	for ( int i=0; i<MAX_ATTRS; ++i )
		if ( m_eKeypart[i]==SPH_KEYPART_INT && m_tLocator[i].IsBitfield() )
			return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////

class SortClauseTokenizer_c
{
public:
	explicit SortClauseTokenizer_c ( const char * sBuffer )
	{
		auto iLen = (int) strlen(sBuffer);
		m_pBuf = new char [ iLen+1 ];
		m_pMax = m_pBuf+iLen;
		m_pCur = m_pBuf;

		// make string lowercase but keep case of JSON.field
		bool bJson = false;
		for ( int i=0; i<=iLen; i++ )
		{
			char cSrc = sBuffer[i];
			char cDst = ToLower ( cSrc );
			bJson = ( cSrc=='.' || cSrc=='[' || ( bJson && cDst>0 ) ); // keep case of valid char sequence after '.' and '[' symbols
			m_pBuf[i] = bJson ? cSrc : cDst;
		}
	}

	~SortClauseTokenizer_c()
	{
		SafeDeleteArray ( m_pBuf );
	}

	const char * GetToken ()
	{
		// skip spaces
		while ( m_pCur<m_pMax && !*m_pCur )
			m_pCur++;
		if ( m_pCur>=m_pMax )
			return nullptr;

		// memorize token start, and move pointer forward
		const char * sRes = m_pCur;
		while ( *m_pCur )
			m_pCur++;
		return sRes;
	}

	bool IsSparseCount ( const char * sTok )
	{
		const char * sSeq = "(*)";
		for ( ; sTok<m_pMax && *sSeq; sTok++ )
		{
			bool bGotSeq = ( *sSeq==*sTok );
			if ( bGotSeq )
				sSeq++;

			// stop checking on any non space char outside sequence or sequence end
			if ( ( !bGotSeq && !sphIsSpace ( *sTok ) && *sTok!='\0' ) || !*sSeq )
				break;
		}

		if ( !*sSeq && sTok+1<m_pMax && !sTok[1] )
		{
			// advance token iterator after composite count(*) token
			m_pCur = sTok+1;
			return true;
		}
		
		return false;
	}

protected:
	const char *	m_pCur = nullptr;
	const char *	m_pMax = nullptr;
	char *			m_pBuf = nullptr;

	char ToLower ( char c )
	{
		// 0..9, A..Z->a..z, _, a..z, @, .
		if ( ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || c=='_' || c=='@' || c=='.' || c=='[' || c==']' || c=='\'' || c=='\"' || c=='(' || c==')' || c=='*' )
			return c;
		if ( c>='A' && c<='Z' )
			return c-'A'+'a';
		return 0;
	}
};


static inline ESphSortKeyPart Attr2Keypart ( ESphAttr eType )
{
	switch ( eType )
	{
		case SPH_ATTR_FLOAT:
			return SPH_KEYPART_FLOAT;

		case SPH_ATTR_DOUBLE:
			return SPH_KEYPART_DOUBLE;

		case SPH_ATTR_STRING:
			return SPH_KEYPART_STRING;

		case SPH_ATTR_JSON:
		case SPH_ATTR_JSON_PTR:
		case SPH_ATTR_JSON_FIELD:
		case SPH_ATTR_JSON_FIELD_PTR:
		case SPH_ATTR_STRINGPTR:
			return SPH_KEYPART_STRINGPTR;

		default:
			return SPH_KEYPART_INT;
	}
}

//////////////////////////////////////////////////////////////////////////

class SortStateSetup_c
{
public:
			SortStateSetup_c ( const char * szTok, SortClauseTokenizer_c & tTok, CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs, int iField, const ISphSchema & tSchema, const CSphQuery & tQuery );

	bool	Setup ( CSphString & sError );

private:
	const char *				m_szTok = nullptr;
	SortClauseTokenizer_c &		m_tTok;
	CSphMatchComparatorState &	m_tState;
	ExtraSortExpr_t &			m_tExtraExpr;
	const int					m_iField;
	const ISphSchema &			m_tSchema;
	const CSphQuery &			m_tQuery;

	int							m_iAttr = -1;
	ESphAttr					m_eAttrType = SPH_ATTR_NONE;

	bool	SetupSortByRelevance();
	void	UnifyInternalAttrNames();
	bool	CheckOrderByMva ( CSphString & sError ) const;
	void	FindAliasedGroupby();
	void	FindAliasedSortby();
	bool	IsJsonAttr() const;
	void	SetupJsonAttr();
	bool	SetupJsonField ( CSphString & sError );
	bool	SetupColumnar ( CSphString & sError );
	bool	SetupJson ( CSphString & sError );
	void	SetupJsonConversions();
	void	SetupPrecalculatedJson();
};


SortStateSetup_c::SortStateSetup_c ( const char * szTok, SortClauseTokenizer_c & tTok, CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dJsonExprs, int iField, const ISphSchema & tSchema, const CSphQuery & tQuery )
	: m_szTok ( szTok )
	, m_tTok ( tTok )
	, m_tState ( tState )
	, m_tExtraExpr ( dJsonExprs[iField] )
	, m_iField ( iField )
	, m_tSchema ( tSchema )
	, m_tQuery ( tQuery )
{}


bool SortStateSetup_c::SetupSortByRelevance()
{
	if ( !strcasecmp ( m_szTok, "@relevance" )
		|| !strcasecmp ( m_szTok, "@rank" )
		|| !strcasecmp ( m_szTok, "@weight" )
		|| !strcasecmp ( m_szTok, "weight()" ) )
	{
		m_tState.m_eKeypart[m_iField] = SPH_KEYPART_WEIGHT;
		return true;
	}

	return false;
}


void SortStateSetup_c::UnifyInternalAttrNames()
{
	if ( !strcasecmp ( m_szTok, "@group" ) )
		m_szTok = "@groupby";
	else if ( !strcasecmp ( m_szTok, "count(*)" ) )
		m_szTok = "@count";
	else if ( !strcasecmp ( m_szTok, "facet()" ) )
		m_szTok = "@groupby"; // facet() is essentially a @groupby alias
	else if ( strcasecmp ( m_szTok, "count" )>=0 && m_tTok.IsSparseCount ( m_szTok + sizeof ( "count" ) - 1 ) ) // epression count(*) with various spaces
		m_szTok = "@count";
}


bool SortStateSetup_c::CheckOrderByMva ( CSphString & sError ) const
{
	int iAttr = m_tSchema.GetAttrIndex(m_szTok);

	if ( iAttr<0 )
		return true;

	ESphAttr eAttrType = m_tSchema.GetAttr(iAttr).m_eAttrType;
	if ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET || eAttrType==SPH_ATTR_UINT32SET_PTR || eAttrType==SPH_ATTR_INT64SET_PTR )
	{
		sError.SetSprintf ( "order by MVA is undefined" );
		return false;
	}

	return true;
}


void SortStateSetup_c::FindAliasedGroupby()
{
	if ( m_iAttr>=0 )
		return;

	int iAttr = m_tSchema.GetAttrIndex(m_szTok);
	if ( iAttr>=0 )
	{
		m_iAttr = iAttr;
		return;
	}

	// try to lookup aliased count(*) and aliased groupby() in select items
	for ( auto & i : m_tQuery.m_dItems )
	{
		if ( !i.m_sAlias.cstr() || strcasecmp ( i.m_sAlias.cstr(), m_szTok ) )
			continue;

		if ( i.m_sExpr.Begins("@") )
		{
			m_iAttr = m_tSchema.GetAttrIndex ( i.m_sExpr.cstr() );
			return;
		}

		if ( i.m_sExpr=="count(*)" )
		{
			m_iAttr = m_tSchema.GetAttrIndex ( "@count" );
			return;
		}

		if ( i.m_sExpr=="groupby()" )
		{
			CSphString sGroupJson = SortJsonInternalSet ( m_tQuery.m_sGroupBy );
			m_iAttr = m_tSchema.GetAttrIndex ( sGroupJson.cstr() );
			// try numeric group by
			if ( m_iAttr<0 )
				m_iAttr = m_tSchema.GetAttrIndex ( "@groupby" );

			return;
		}
	}
}


void SortStateSetup_c::FindAliasedSortby()
{
	if ( m_iAttr>=0 )
		return;

	int iAttr = m_tSchema.GetAttrIndex(m_szTok);
	if ( iAttr>=0 )
	{
		m_iAttr = iAttr;
		return;
	}

	for ( auto & i : m_tQuery.m_dItems )
	{
		if ( !i.m_sAlias.cstr() || strcasecmp ( i.m_sAlias.cstr(), m_szTok ) )
			continue;

		m_iAttr = m_tSchema.GetAttrIndex ( i.m_sExpr.cstr() );
		return;
	}
}


bool SortStateSetup_c::IsJsonAttr() const
{
	if ( m_iAttr<0 )
		return false;

	ESphAttr eAttrType = m_tSchema.GetAttr(m_iAttr).m_eAttrType;
	if ( eAttrType==SPH_ATTR_JSON_FIELD || eAttrType==SPH_ATTR_JSON_FIELD_PTR || eAttrType==SPH_ATTR_JSON || eAttrType==SPH_ATTR_JSON_PTR )
		return true;

	return false;
}


void SortStateSetup_c::SetupJsonAttr()
{
	const CSphColumnInfo & tAttr = m_tSchema.GetAttr(m_iAttr);
	m_tExtraExpr.m_pExpr = tAttr.m_pExpr;
	m_tExtraExpr.m_tKey = JsonKey_t ( m_szTok, (int)strlen(m_szTok) );
}


bool SortStateSetup_c::SetupJsonField ( CSphString & sError )
{
	CSphString sJsonCol;
	if ( !sphJsonNameSplit ( m_szTok, &sJsonCol ) )
		return true;

	m_iAttr = m_tSchema.GetAttrIndex ( sJsonCol.cstr() );
	if ( m_iAttr>=0 )
	{
		ExprParseArgs_t tExprArgs;
		ISphExpr * pExpr = sphExprParse ( m_szTok, m_tSchema, sError, tExprArgs );
		if ( !pExpr )
			return false;

		m_tExtraExpr.m_pExpr = pExpr;
		m_tExtraExpr.m_tKey = JsonKey_t ( m_szTok, (int) strlen ( m_szTok ) );
	}

	return true;
}


bool SortStateSetup_c::SetupColumnar ( CSphString & sError )
{
	if ( m_iAttr<0 )
		return true;

	const CSphColumnInfo & tAttr = m_tSchema.GetAttr(m_iAttr);
	if ( !tAttr.IsColumnar() )
		return true;

	ExprParseArgs_t tExprArgs;
	tExprArgs.m_pAttrType = &m_eAttrType;
	ISphExpr * pExpr = sphExprParse ( m_szTok, m_tSchema, sError, tExprArgs );
	if ( !pExpr )
		return false;

	m_tExtraExpr.m_pExpr = pExpr;
	m_tExtraExpr.m_eType = m_eAttrType;
	return true;
}


bool SortStateSetup_c::SetupJson ( CSphString & sError )
{
	if ( IsJsonAttr() )
	{
		SetupJsonAttr();
		return true;
	}

	// try JSON attribute and use JSON attribute instead of JSON field
	if ( m_iAttr<0 )
		return SetupJsonField(sError);

	return true;
}


void SortStateSetup_c::SetupJsonConversions()
{
	if ( m_iAttr>=0 )
		return;

	// try json conversion functions (integer()/double()/bigint() in the order by clause)
	ESphAttr eAttrType = SPH_ATTR_NONE;
	ExprParseArgs_t tExprArgs;
	tExprArgs.m_pAttrType = &eAttrType;
	CSphString sError; // ignored
	ISphExpr * pExpr = sphExprParse ( m_szTok, m_tSchema, sError, tExprArgs );
	if ( !pExpr )
		return;

	m_eAttrType = eAttrType;
	m_tExtraExpr.m_pExpr = pExpr;
	m_tExtraExpr.m_tKey = JsonKey_t ( m_szTok, (int) strlen(m_szTok) );
	m_tExtraExpr.m_eType = m_eAttrType;
	m_tExtraExpr.m_tKey.m_uMask = 0;

	m_iAttr = 0; // will be remapped in SetupSortRemap
}


void SortStateSetup_c::SetupPrecalculatedJson()
{
	if ( m_iAttr>=0 )
		return;

	// try precalculated json fields/columnar attrs received from agents (prefixed with @int_*)
	CSphString sName;
	sName.SetSprintf ( "%s%s", GetInternalAttrPrefix(), m_szTok );
	m_iAttr = m_tSchema.GetAttrIndex ( sName.cstr() );
}


bool SortStateSetup_c::Setup ( CSphString & sError )
{
	if ( SetupSortByRelevance() )
		return true;

	UnifyInternalAttrNames();

	if ( !CheckOrderByMva(sError) )
		return false;

	FindAliasedGroupby();
	FindAliasedSortby();

	if ( !SetupColumnar(sError) )
		return false;

	if ( !SetupJson(sError) )
		return false;

	SetupJsonConversions();
	SetupPrecalculatedJson();

	if ( m_iAttr<0 )
	{
		sError.SetSprintf ( "sort-by attribute '%s' not found", m_szTok );
		return false;
	}

	const CSphColumnInfo & tCol = m_tSchema.GetAttr(m_iAttr);
	m_tState.m_eKeypart[m_iField] = Attr2Keypart ( m_eAttrType!=SPH_ATTR_NONE ? m_eAttrType : tCol.m_eAttrType );
	m_tState.m_tLocator[m_iField] = tCol.m_tLocator;
	m_tState.m_dAttrs[m_iField] = m_iAttr;

	return true;
}

//////////////////////////////////////////////////////////////////////////

ESortClauseParseResult sphParseSortClause ( const CSphQuery & tQuery, const char * szClause, const ISphSchema & tSchema, ESphSortFunc & eFunc, CSphMatchComparatorState & tState,
	CSphVector<ExtraSortExpr_t> & dExtraExprs, bool bComputeItems, CSphString & sError )
{
	for ( auto & tAttr : tState.m_dAttrs )
		tAttr = -1;

	dExtraExprs.Resize ( tState.MAX_ATTRS );

	// mini parser
	SortClauseTokenizer_c tTok(szClause);

	bool bField = false; // whether i'm expecting field name or sort order
	int iField = 0;

	for ( const char * pTok=tTok.GetToken(); pTok; pTok=tTok.GetToken() )
	{
		bField = !bField;

		// special case, sort by random
		if ( iField==0 && bField && strcmp ( pTok, "@random" )==0 )
			return SORT_CLAUSE_RANDOM;

		// handle sort order
		if ( !bField )
		{
			// check
			if ( strcmp ( pTok, "desc" ) && strcmp ( pTok, "asc" ) )
			{
				sError.SetSprintf ( "invalid sorting order '%s'", pTok );
				return SORT_CLAUSE_ERROR;
			}

			// set
			if ( !strcmp ( pTok, "desc" ) )
				tState.m_uAttrDesc |= ( 1<<iField );

			iField++;
			continue;
		}

		// handle attribute name
		if ( iField==CSphMatchComparatorState::MAX_ATTRS )
		{
			sError.SetSprintf ( "too many sort-by attributes; maximum count is %d", CSphMatchComparatorState::MAX_ATTRS );
			return SORT_CLAUSE_ERROR;
		}
		
		SortStateSetup_c tSetup ( pTok, tTok, tState, dExtraExprs, iField, tSchema, tQuery );
		if ( !tSetup.Setup(sError) )
			return SORT_CLAUSE_ERROR;		
	}

	if ( iField==0 )
	{
		sError.SetSprintf ( "no sort order defined" );
		return SORT_CLAUSE_ERROR;
	}

	switch ( iField )
	{
		case 1:		eFunc = FUNC_GENERIC1; break;
		case 2:		eFunc = FUNC_GENERIC2; break;
		case 3:		eFunc = FUNC_GENERIC3; break;
		case 4:		eFunc = FUNC_GENERIC4; break;
		case 5:		eFunc = FUNC_GENERIC5; break;
		default:	sError.SetSprintf ( "INTERNAL ERROR: %d fields in sphParseSortClause()", iField ); return SORT_CLAUSE_ERROR;
	}

	return SORT_CLAUSE_OK;
}
