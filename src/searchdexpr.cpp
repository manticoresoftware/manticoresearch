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

#include "exprtraits.h"
#include "searchdexpr.h"
#include "sphinxexcerpt.h"
#include "sphinxutils.h"
#include "sphinxint.h"
#include "attribute.h"
#include "docstore.h"


enum HookType_e
{
	HOOK_SNIPPET,
	HOOK_HIGHLIGHT
};


static int StringBinary2Number ( const char * sStr, int iLen )
{
	if ( !sStr || !iLen )
		return 0;

	char sBuf[64];
	if ( (int)(sizeof ( sBuf )-1 )<iLen )
		iLen = sizeof ( sBuf )-1;
	memcpy ( sBuf, sStr, iLen );
	sBuf[iLen] = '\0';

	return atoi ( sBuf );
}


static bool ParseSnippetLimit ( const CSphString & sName, int iVal, SnippetLimits_t & tOpt )
{
	if ( sName=="limit" )				tOpt.m_iLimit = iVal;
	else if ( sName=="limit_passages" || sName=="limit_snippets" ) tOpt.m_iLimitPassages = iVal;
	else if ( sName=="limit_words" ) 	tOpt.m_iLimitWords = iVal;
	else
		return false;

	return true;
}


static bool ParseSnippetOption ( const CSphNamedVariant & tVariant, SnippetQuerySettings_t & tOpt, CSphString & sError )
{
	CSphString sName = tVariant.m_sKey;
	sName.ToLower();

	const CSphString & sVal = tVariant.m_sValue;
	int iVal = tVariant.m_iValue;
	bool bVal = tVariant.m_iValue!=0;

	if ( ParseSnippetLimit ( sName, iVal, tOpt ) )
		return true;

	const char * szBegins = "__";
	if ( sName.Begins(szBegins) )
	{
		auto iStartLen = (int)strlen(szBegins);
		const char * szTmp = sName.cstr()+iStartLen;
		while ( *szTmp && *szTmp!='_' )
			szTmp++;

		if ( *szTmp )
			szTmp++;

		CSphString sField = sName.SubString ( iStartLen, szTmp-sName.cstr()-iStartLen-1 );
		CSphString sOption = szTmp;
		SnippetLimits_t tLimits;
		if ( !ParseSnippetLimit ( sOption, iVal, tLimits ) )
		{
			sError.SetSprintf ( "unknown option %s in %s", sName.cstr(), sOption.cstr() );
			return false;
		}

		tOpt.m_hPerFieldLimits.AddUnique(sField) = tLimits;
		return true;
	}

	if ( sName=="before_match" )			tOpt.m_sBeforeMatch = sVal;
	else if ( sName=="after_match" ) 		tOpt.m_sAfterMatch = sVal;
	else if ( sName=="chunk_separator" || sName=="snippet_separator")	tOpt.m_sChunkSeparator = sVal;
	else if ( sName=="field_separator" )	tOpt.m_sFieldSeparator = sVal;
	else if ( sName=="around" ) 			tOpt.m_iAround = iVal;
	else if ( sName=="use_boundaries" )		tOpt.m_bUseBoundaries = bVal;
	else if ( sName=="weight_order" )		tOpt.m_bWeightOrder = bVal;
	else if ( sName=="force_all_words" ) 	tOpt.m_bForceAllWords = bVal;
	else if ( sName=="start_passage_id" || sName=="start_snippet_id") tOpt.m_iPassageId = iVal;
	else if ( sName=="load_files" ) 		tOpt.m_uFilesMode |= bVal ? 1 : 0;
	else if ( sName=="load_files_scattered" ) tOpt.m_uFilesMode |= bVal ? 2 : 0;
	else if ( sName=="html_strip_mode" )	tOpt.m_sStripMode = sVal;
	else if ( sName=="allow_empty" ) 		tOpt.m_bAllowEmpty = bVal;
	else if ( sName=="emit_zones" )			tOpt.m_bEmitZones = bVal;
	else if ( sName=="force_passages" || sName=="force_snippets" )		tOpt.m_bForcePassages = bVal;
	else if ( sName=="passage_boundary" || sName=="snippet_boundary" )	tOpt.m_ePassageSPZ = GetPassageBoundary(sVal);
	else if ( sName=="json_query" )			tOpt.m_bJsonQuery = bVal;
	else if ( sName=="pack_fields" )		tOpt.m_bPackFields = bVal;
	else if ( sName=="limits_per_field" )	tOpt.m_bLimitsPerField = bVal;
	else if ( sName=="exact_phrase" )
	{
		sError.SetSprintf ( "exact_phrase option is deprecated" );
		return false;
	}
	else if ( sName=="query_mode" )
	{
		if ( !bVal )
		{
			sError.SetSprintf ( "query_mode=0 is deprecated" );
			return false;
		}
	}
	else
	{
		sError.SetSprintf ( "Unknown option: %s", sName.cstr() );
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

class QueryExprTraits_c
{
public:
								QueryExprTraits_c ( ISphExpr * pQuery );
								QueryExprTraits_c ( const QueryExprTraits_c & rhs );

	bool						UpdateQuery ( const CSphMatch & tMatch ) const;
	const CSphString &			GetQuery() const { return m_sQuery; }
	bool						Command ( ESphExprCommand eCmd, void * pArg );

private:
	CSphRefcountedPtr<ISphExpr>	m_pQuery;
	mutable bool				m_bFirstQuery = true;
	mutable CSphString			m_sQuery;

	CSphString					FetchQuery ( const CSphMatch & tMatch ) const;
};


QueryExprTraits_c::QueryExprTraits_c ( ISphExpr * pQuery )
	: m_pQuery ( pQuery )
{
	if ( m_pQuery )
		SafeAddRef(m_pQuery);
}


QueryExprTraits_c::QueryExprTraits_c ( const QueryExprTraits_c & rhs )
	: m_pQuery ( SafeClone ( rhs.m_pQuery ) )
{}


bool QueryExprTraits_c::UpdateQuery ( const CSphMatch & tMatch ) const
{
	CSphString sQuery = FetchQuery(tMatch);
	if ( m_bFirstQuery || m_sQuery!=sQuery )
	{
		m_bFirstQuery = false;
		m_sQuery = sQuery;
		return true;
	}

	return false;
}


CSphString QueryExprTraits_c::FetchQuery ( const CSphMatch & tMatch ) const
{
	if ( !m_pQuery )
		return m_sQuery;

	CSphString sQuery;
	char * pWords;
	int iQueryLen = m_pQuery->StringEval ( tMatch, (const BYTE**)&pWords );
	if ( m_pQuery->IsDataPtrAttr() )
		sQuery.Adopt ( &pWords );
	else
		sQuery.SetBinary ( pWords, iQueryLen );

	return sQuery;
}


bool QueryExprTraits_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( m_pQuery )
		m_pQuery->Command ( eCmd, pArg );

	if ( eCmd==SPH_EXPR_SET_QUERY && !m_pQuery ) // don't do this if we have a query expression specified
	{
		CSphString sQuery ( (const char*)pArg );
		if ( m_bFirstQuery || m_sQuery!=sQuery )
		{
			m_sQuery = sQuery;
			m_bFirstQuery = false;
			return true;
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
class Expr_HighlightTraits_c : public ISphStringExpr, public QueryExprTraits_c
{
public:
				Expr_HighlightTraits_c ( const CSphIndex * pIndex, QueryProfile_c * pProfiler, ISphExpr * pQuery );

	void		Command ( ESphExprCommand eCmd, void * pArg ) override;
	void		FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override;
	uint64_t	GetHash ( const ISphSchema &, uint64_t, bool & ) override;
	bool		IsDataPtrAttr () const final { return true; }

protected:
	CSphRefcountedPtr<ISphExpr>		m_pArgs;
	CSphRefcountedPtr<ISphExpr>		m_pText;
	int								m_iTextLocator = -1;
	CSphVector<int>					m_dRequestedFieldIds;
	const CSphIndex *				m_pIndex = nullptr;
	QueryProfile_c *				m_pProfiler = nullptr;
	SnippetQuerySettings_t			m_tSnippetQuery;
	std::unique_ptr<SnippetBuilder_c>	m_pSnippetBuilder;


				Expr_HighlightTraits_c ( const Expr_HighlightTraits_c & rhs );

	void		SetTextExpr ( ISphExpr * pExpr, const ISphSchema * pRsetSchema );
};


Expr_HighlightTraits_c::Expr_HighlightTraits_c ( const CSphIndex * pIndex, QueryProfile_c * pProfiler, ISphExpr * pQuery )
	: QueryExprTraits_c ( pQuery )
	, m_pIndex ( pIndex )
	, m_pProfiler ( pProfiler )
	, m_pSnippetBuilder { std::make_unique<SnippetBuilder_c>() }
{}


Expr_HighlightTraits_c::Expr_HighlightTraits_c ( const Expr_HighlightTraits_c & rhs )
	: QueryExprTraits_c ( rhs )
	, m_pArgs ( SafeClone ( rhs.m_pArgs ) )
	, m_pText ( SafeClone ( rhs.m_pText ) )
	, m_dRequestedFieldIds ( rhs.m_dRequestedFieldIds )
	, m_pIndex ( rhs.m_pIndex )
	, m_pProfiler ( rhs.m_pProfiler )
	, m_tSnippetQuery ( rhs.m_tSnippetQuery )
	, m_pSnippetBuilder { std::make_unique<SnippetBuilder_c>() }
{
	m_pSnippetBuilder->Setup ( m_pIndex, m_tSnippetQuery );
}


void Expr_HighlightTraits_c::FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	if ( !m_pText )
		return;

	// when highlight expression was created, the m_pText was a GetColumnarString expression
	// but when the result is converted to dynamic schema, m_pText becomes a plain GetString expression
	// we need to update m_pText pointer
	if ( !m_pText->IsColumnar() )
	{
		m_pText->FixupLocator ( pOldSchema, pNewSchema );
		return;
	}

	CSphString sColumnarCol;
	m_pText->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sColumnarCol );
	m_iTextLocator = pNewSchema->GetAttrIndex ( sColumnarCol.cstr() );
	assert ( m_iTextLocator!=0 );

	m_pText = pNewSchema->GetAttr(m_iTextLocator).m_pExpr;
}


void Expr_HighlightTraits_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( m_pArgs )
		m_pArgs->Command ( eCmd, pArg );

	if ( m_pText )
		m_pText->Command ( eCmd, pArg );

	if ( eCmd==SPH_EXPR_GET_DEPENDENT_COLS && m_pText && m_pText->IsColumnar() )
	{
		assert ( m_iTextLocator>=0 );
		static_cast < CSphVector<int>* >(pArg)->Add ( m_iTextLocator );
	}

	if ( eCmd==SPH_EXPR_UPDATE_DEPENDENT_COLS && m_iTextLocator>=*static_cast<int*>(pArg) )
		m_iTextLocator--;

	if ( QueryExprTraits_c::Command ( eCmd, pArg ) )
	{
		// fixme! handle errors
		CSphString sError;
		m_pSnippetBuilder->SetQuery ( GetQuery(), false, sError );
	}
}


uint64_t Expr_HighlightTraits_c::GetHash ( const ISphSchema &, uint64_t, bool & )
{
	assert ( 0 && "no highlighting in filters" );
	return 0;
}


void Expr_HighlightTraits_c::SetTextExpr ( ISphExpr * pExpr, const ISphSchema * pRsetSchema )
{
	m_pText = pExpr;
	SafeAddRef(m_pText);

	if ( m_pText && m_pText->IsColumnar() )
	{
		CSphString sColumnarCol;
		m_pText->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sColumnarCol );
		m_iTextLocator = pRsetSchema->GetAttrIndex ( sColumnarCol.cstr() ); 
	}
}

//////////////////////////////////////////////////////////////////////////
class Expr_Snippet_c : public Expr_HighlightTraits_c
{
public:
				Expr_Snippet_c ( ISphExpr * pArglist, const CSphIndex * pIndex, const ISphSchema * pRsetSchema, QueryProfile_c * pProfiler, QueryType_e eQueryType, CSphString & sError );

	int			StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const override;
	ISphExpr *	Clone() const override;
	uint64_t	GetHash ( const ISphSchema & tSchema, uint64_t uPrevHash, bool & bDisable ) override;
};


Expr_Snippet_c::Expr_Snippet_c ( ISphExpr * pArglist, const CSphIndex * pIndex, const ISphSchema * pRsetSchema, QueryProfile_c * pProfiler, QueryType_e eQueryType, CSphString & sError )
	: Expr_HighlightTraits_c ( pIndex, pProfiler, pArglist->GetArg(1) )
{
	m_pArgs = pArglist;
	SafeAddRef ( m_pArgs );
	assert ( m_pArgs->IsArglist() );

	SetTextExpr ( pArglist->GetArg(0), pRsetSchema );

	CSphMatch tDummy;
	char * pWords;
	for ( int i = 2; i < pArglist->GetNumArgs(); i++ )
	{
		assert ( !pArglist->GetArg(i)->IsDataPtrAttr() ); // aware of memleaks potentially caused by StringEval()
		int iLen = pArglist->GetArg(i)->StringEval ( tDummy, (const BYTE**)&pWords );
		if ( !pWords || !iLen )
			continue;

		CSphString sArgs;
		sArgs.SetBinary ( pWords, iLen );
		pWords = const_cast<char *> ( sArgs.cstr() );

		const char * sEnd = pWords + iLen;
		while ( pWords<sEnd && *pWords && sphIsSpace ( *pWords ) )	pWords++;
		char * szOption = pWords;
		while ( pWords<sEnd && *pWords && sphIsAlpha ( *pWords ) )	pWords++;
		char * szOptEnd = pWords;
		while ( pWords<sEnd && *pWords && sphIsSpace ( *pWords ) )	pWords++;

		if ( *pWords++!='=' )
		{
			sError.SetSprintf ( "Error parsing SNIPPET options: %s", pWords );
			return;
		}

		*szOptEnd = '\0';
		while ( pWords<sEnd && *pWords && sphIsSpace ( *pWords ) )	pWords++;
		char * sValue = pWords;

		if ( !*sValue )
		{
			sError.SetSprintf ( "Error parsing SNIPPET options" );
			return;
		}

		while ( pWords<sEnd && *pWords ) pWords++;
		int iStrValLen = pWords - sValue;

		CSphNamedVariant tVariant;
		tVariant.m_sKey = szOption;
		tVariant.m_sValue.SetBinary ( sValue, iStrValLen );
		tVariant.m_iValue = StringBinary2Number ( sValue, iStrValLen );
		if ( !ParseSnippetOption ( tVariant, m_tSnippetQuery, sError ) )
			return;
	}

	m_tSnippetQuery.m_bJsonQuery = eQueryType==QUERY_JSON;
	m_tSnippetQuery.Setup();
	m_pSnippetBuilder->Setup ( m_pIndex, m_tSnippetQuery );
	m_dRequestedFieldIds.Add(0);
}


int Expr_Snippet_c::StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
{
	CSphScopedProfile tProf ( m_pProfiler, SPH_QSTATE_SNIPPET );

	*ppStr = nullptr;

	const BYTE * szSource = nullptr;
	int iLen = m_pText->StringEval ( tMatch, &szSource );

	// kinda like a scoped ptr, but for an array
	CSphFixedVector<BYTE> tScoped {0};
	if ( m_pText->IsDataPtrAttr() )
		tScoped.Set ( (BYTE *)szSource, iLen );

	if ( !iLen )
		return 0;

	if ( UpdateQuery(tMatch) )
	{
		CSphString sError;
		if ( !m_pSnippetBuilder->SetQuery ( GetQuery(), true, sError ) )
			return 0;
	}

	std::unique_ptr<TextSource_i> pSource = CreateSnippetSource ( m_tSnippetQuery.m_uFilesMode, szSource, iLen );

	// FIXME! fill in all the missing options; use consthash?
	SnippetResult_t tRes;
	if ( !m_pSnippetBuilder->Build ( pSource, tRes ) )
		return 0;

	CSphVector<BYTE> dRes = m_pSnippetBuilder->PackResult ( tRes, m_dRequestedFieldIds );

	int iResultLength = dRes.GetLength();
	*ppStr = dRes.LeakData();
	return iResultLength;
}


uint64_t Expr_Snippet_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_Snippet_c");
	return CALC_DEP_HASHES();
}

ISphExpr * Expr_Snippet_c::Clone () const
{
	return new Expr_Snippet_c ( *this );
}


//////////////////////////////////////////////////////////////////////////

class Expr_Highlight_c final : public Expr_HighlightTraits_c
{
public:
				Expr_Highlight_c ( ISphExpr * pArglist, const CSphIndex * pIndex, const ISphSchema * pRsetSchema, QueryProfile_c * pProfiler, QueryType_e eQueryType, CSphString & sError );

	int			StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final;
	void		Command ( ESphExprCommand eCmd, void * pArg ) final;
	ISphExpr *	Clone() const final;
	uint64_t	GetHash ( const ISphSchema & tSchema, uint64_t uPrevHash, bool & bDisable ) override;

private:
	DocstoreSession_c::InfoDocID_t	m_tSession;
	CSphVector<int>					m_dFieldsToFetch;
	bool							m_bFetchAllFields = false;

				Expr_Highlight_c ( const Expr_Highlight_c & rhs );

	bool		FetchFieldsFromDocstore ( DocstoreDoc_t & tFetchedDoc, DocID_t & tDocID ) const;
	CSphVector<FieldSource_t> RearrangeFetchedFields ( const DocstoreDoc_t & tFetchedDoc ) const;
	void		ParseFields ( ISphExpr * pExpr );
	bool		ParseOptions ( const VecTraits_T<CSphNamedVariant> & dMap, CSphString & sError );
	bool		MarkRequestedFields ( CSphString & sError );
	void		MarkAllFields();
};


Expr_Highlight_c::Expr_Highlight_c ( ISphExpr * pArglist, const CSphIndex * pIndex, const ISphSchema * pRsetSchema, QueryProfile_c * pProfiler, QueryType_e eQueryType, CSphString & sError )
	: Expr_HighlightTraits_c ( pIndex, pProfiler, ( pArglist && pArglist->IsArglist() && pArglist->GetNumArgs()==3 ) ? pArglist->GetArg(2) : nullptr )
{
	assert ( m_pIndex );

	if ( pArglist && pArglist->IsArglist() )
	{
		m_pArgs = pArglist;
		SafeAddRef(m_pArgs);
	}

	int iNumArgs = pArglist ? ( pArglist->IsArglist() ? pArglist->GetNumArgs() : 1 ) : 0;

	if ( iNumArgs>=1 )
	{
		// this should be a map argument. at least we checked that in ExprHook_c::GetReturnType
		auto pMapArg = (Expr_MapArg_c *)(pArglist->IsArglist() ? pArglist->GetArg(0) : pArglist);
		assert(pMapArg);
		VecTraits_T<CSphNamedVariant> dOpts ( pMapArg->m_pValues, pMapArg->m_iCount );
		if ( !ParseOptions ( dOpts, sError ) )
			return;
	}

	if ( iNumArgs>=2 )
	{
		assert ( pArglist && pArglist->IsArglist() );
		ISphExpr * pArg2 = pArglist->GetArg(1);

		// mode 1: it is a list of stored fields
		// mode 2: it is an expression that needs to be evaluated
		if ( pArg2->IsConst() )
			ParseFields(pArg2);
		else
		{
			SetTextExpr ( pArg2, pRsetSchema );
			m_dRequestedFieldIds.Add(0);
		}
	}
	else
		MarkAllFields();

	m_tSnippetQuery.m_bJsonQuery = eQueryType==QUERY_JSON;
	m_tSnippetQuery.Setup();
	m_pSnippetBuilder->Setup ( m_pIndex, m_tSnippetQuery );
}


int	Expr_Highlight_c::StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
{
	CSphScopedProfile tProf ( m_pProfiler, SPH_QSTATE_SNIPPET );

	DocID_t tDocID = sphGetDocID ( tMatch.m_pDynamic ? tMatch.m_pDynamic : tMatch.m_pStatic );

	if ( UpdateQuery(tMatch) )
	{
		CSphString sError;
		if ( !m_pSnippetBuilder->SetQuery ( GetQuery(), true, sError ) )
			return 0;
	}

	DocstoreDoc_t tFetchedDoc;
	CSphVector<FieldSource_t> dAllFields;
	std::unique_ptr<TextSource_i> pSource;
	CSphFixedVector<BYTE> tScoped {0}; // scoped array ptr

	if ( m_pText )
	{
		// highlight an expression
		const BYTE * szSource = nullptr;
		int iLen = m_pText->StringEval ( tMatch, &szSource );

		if ( m_pText->IsDataPtrAttr() )
			tScoped.Set ( (BYTE *)szSource, iLen );

		pSource = CreateSnippetSource ( m_tSnippetQuery.m_uFilesMode, szSource, iLen );
	}
	else
	{
		// fetch fields and highlight
		if ( !FetchFieldsFromDocstore ( tFetchedDoc, tDocID ) )
			return 0;

		dAllFields = RearrangeFetchedFields ( tFetchedDoc );
		pSource = CreateHighlightSource ( dAllFields );
	}

	SnippetResult_t tRes;
	if ( !m_pSnippetBuilder->Build ( pSource, tRes ) )
		return 0;

	CSphVector<BYTE> dPacked = m_pSnippetBuilder->PackResult ( tRes, m_dRequestedFieldIds );
	int iResultLength = dPacked.GetLength();
	*ppStr = dPacked.LeakData();
	return iResultLength;
}


void Expr_Highlight_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	Expr_HighlightTraits_c::Command ( eCmd, pArg );

	if ( eCmd==SPH_EXPR_SET_DOCSTORE_DOCID )
	{
		const DocstoreSession_c::InfoDocID_t & tSession = *(DocstoreSession_c::InfoDocID_t*)pArg;
		bool bMark = tSession.m_pDocstore!=m_tSession.m_pDocstore;
		m_tSession = tSession;

		if ( bMark )
		{
			// fixme! handle errors
			CSphString sError;
			MarkRequestedFields(sError);
		}
	}
}


ISphExpr * Expr_Highlight_c::Clone () const
{
	return new Expr_Highlight_c ( *this );
}


Expr_Highlight_c::Expr_Highlight_c ( const Expr_Highlight_c& rhs )
	: Expr_HighlightTraits_c ( rhs )
{}


bool Expr_Highlight_c::FetchFieldsFromDocstore ( DocstoreDoc_t & tFetchedDoc, DocID_t & tDocID ) const
{
	if ( !m_tSession.m_pDocstore )
		return false;

	const CSphVector<int> * pFieldsToFetch = m_bFetchAllFields ? nullptr : &m_dFieldsToFetch;
	return m_tSession.m_pDocstore->GetDoc ( tFetchedDoc, tDocID, pFieldsToFetch, m_tSession.m_iSessionId, false );
}


CSphVector<FieldSource_t> Expr_Highlight_c::RearrangeFetchedFields ( const DocstoreDoc_t & tFetchedDoc ) const
{
	// we need to arrange fetched fields as in original index schema
	// so that field matching will work as expected
	CSphVector<FieldSource_t> dAllFields;
	const CSphSchema & tSchema = m_pIndex->GetMatchSchema();
	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
	{
		const CSphColumnInfo & tInfo = tSchema.GetField(i);
		FieldSource_t & tNewField = dAllFields.Add();
		tNewField.m_sName = tInfo.m_sName;

		if ( !( tInfo.m_uFieldFlags & CSphColumnInfo::FIELD_STORED ) )
			continue;

		int iFieldId = m_tSession.m_pDocstore->GetFieldId ( tInfo.m_sName, DOCSTORE_TEXT );
		assert ( iFieldId!=-1 );

		int iFetchedFieldId = -1;
		if ( m_bFetchAllFields )
			iFetchedFieldId = iFieldId;
		else
		{
			int * pFound = sphBinarySearch ( m_dFieldsToFetch.Begin(), m_dFieldsToFetch.Begin()+m_dFieldsToFetch.GetLength()-1, iFieldId );
			if ( pFound )
				iFetchedFieldId = pFound-m_dFieldsToFetch.Begin();
		}

		if ( iFetchedFieldId!=-1 )
			tNewField.m_dData = tFetchedDoc.m_dFields[iFetchedFieldId].Slice();
	}

	return dAllFields;
}
uint64_t Expr_Highlight_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_Highlight_c");
	return CALC_DEP_HASHES();
}


void Expr_Highlight_c::ParseFields ( ISphExpr * pExpr )
{
	assert ( pExpr && !pExpr->IsDataPtrAttr() );
	assert(m_pIndex);

	CSphString sFields;
	char * szFields;
	CSphMatch tDummy;
	int iLen = pExpr->StringEval ( tDummy, (const BYTE**)&szFields );
	sFields.SetBinary ( szFields, iLen );
	sFields.ToLower();
	sFields.Trim();

	StrVec_t dRequestedFieldNames;
	sphSplit ( dRequestedFieldNames, sFields.cstr() );

	if ( !dRequestedFieldNames.GetLength() && sFields.IsEmpty() )
		MarkAllFields();
	else
	{
		const CSphSchema & tSchema = m_pIndex->GetMatchSchema();
		for ( const auto & i : dRequestedFieldNames )
		{
			int iField = tSchema.GetFieldIndex ( i.cstr() );
			if ( iField!=-1 )
				m_dRequestedFieldIds.Add(iField);
		}
	}
}


void Expr_Highlight_c::MarkAllFields()
{
	m_bFetchAllFields = true;
	m_dFieldsToFetch.Resize(0);

	const CSphSchema & tSchema = m_pIndex->GetMatchSchema();
	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
		m_dRequestedFieldIds.Add(i);
}


bool Expr_Highlight_c::MarkRequestedFields ( CSphString & sError )
{
	m_dFieldsToFetch.Resize(0);

	bool bResult = true;

	if ( !m_bFetchAllFields )
	{
		assert ( m_tSession.m_pDocstore );
		const CSphSchema & tSchema = m_pIndex->GetMatchSchema();

		for ( auto iField : m_dRequestedFieldIds )
		{
			const char * szField = tSchema.GetFieldName(iField);
			int iDocstoreField = m_tSession.m_pDocstore->GetFieldId ( szField, DOCSTORE_TEXT );
			if ( iDocstoreField==-1 )
			{
				sError.SetSprintf ( "field %s not found", szField );
				bResult = false;
				continue;
			}

			m_dFieldsToFetch.Add(iDocstoreField);
		}

		m_dFieldsToFetch.Uniq();
	}

	return bResult;
}


bool Expr_Highlight_c::ParseOptions ( const VecTraits_T<CSphNamedVariant> & dMap, CSphString & sError )
{
	for ( const auto & i : dMap )
	{
		if ( !ParseSnippetOption ( i, m_tSnippetQuery, sError ) )
			return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////

int ExprHook_c::IsKnownFunc ( const char * sFunc ) const
{
	if ( !strcasecmp ( sFunc, "SNIPPET" ) )
		return HOOK_SNIPPET;

	if ( !strcasecmp ( sFunc, "HIGHLIGHT" ) )
		return HOOK_HIGHLIGHT;

	return -1;
}


ISphExpr * ExprHook_c::CreateNode ( int iID, ISphExpr * pLeft, const ISphSchema * pRsetSchema, ESphEvalStage * pEvalStage, bool * pNeedDocIds, CSphString & sError )
{
	if ( pEvalStage )
		*pEvalStage = SPH_EVAL_POSTLIMIT;

	if ( pNeedDocIds )
		*pNeedDocIds = true;

	CSphRefcountedPtr<ISphExpr> pRes;

	switch ( iID )
	{
	case HOOK_SNIPPET:
		pRes = new Expr_Snippet_c ( pLeft, m_pIndex, pRsetSchema, m_pProfiler, m_eQueryType, sError );
		break;

	case HOOK_HIGHLIGHT:
		pRes = new Expr_Highlight_c ( pLeft, m_pIndex, pRsetSchema, m_pProfiler, m_eQueryType, sError );
		break;

	default:
		assert ( 0 && "Unknown node type" );
		return nullptr;
	}

	if ( !sError.IsEmpty() )
		pRes->Release();

	return pRes.Leak();
}


ESphAttr ExprHook_c::GetIdentType ( int ) const
{
	assert(0);
	return SPH_ATTR_NONE;
}


ESphAttr ExprHook_c::GetReturnType ( int iID, const CSphVector<ESphAttr> & dArgs, bool, CSphString & sError ) const
{
	switch ( iID )
	{
	case HOOK_SNIPPET:
		if ( dArgs.GetLength()<2 )
		{
			sError = "SNIPPET() requires 2 or more arguments";
			return SPH_ATTR_NONE;
		}

		if ( dArgs[0]!=SPH_ATTR_STRINGPTR && dArgs[0]!=SPH_ATTR_STRING )
		{
			sError = "1st argument to SNIPPET() must be a string expression";
			return SPH_ATTR_NONE;
		}

		for ( int i = 1; i < dArgs.GetLength(); i++ )
			if ( dArgs[i]!=SPH_ATTR_STRING && dArgs[i]!=SPH_ATTR_STRINGPTR )
			{
				sError.SetSprintf ( "%d argument to SNIPPET() must be a string", i );
				return SPH_ATTR_NONE;
			}
		break;

	case HOOK_HIGHLIGHT:
		if ( dArgs.GetLength()>3 )
		{
			sError = "HIGHLIGHT() requires 0-3 arguments";
			return SPH_ATTR_NONE;
		}

		if ( dArgs.GetLength()>0 && dArgs[0]!=SPH_ATTR_MAPARG )
		{
			sError = "1st argument to HIGHLIGHT() must be a map";
			return SPH_ATTR_NONE;
		}

		if ( dArgs.GetLength()>1 && dArgs[1]!=SPH_ATTR_STRING && dArgs[1]!=SPH_ATTR_STRINGPTR )
		{
			sError = "2nd argument to HIGHLIGHT() must be a string";
			return SPH_ATTR_NONE;
		}

		if ( dArgs.GetLength()>2 && dArgs[2]!=SPH_ATTR_STRING && dArgs[2]!=SPH_ATTR_STRINGPTR )
		{
			sError = "3rd argument to HIGHLIGHT() must be a string";
			return SPH_ATTR_NONE;
		}

		break;
	}

	return SPH_ATTR_STRINGPTR;
}
