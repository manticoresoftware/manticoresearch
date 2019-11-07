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


static bool ParseSnippetOption ( const CSphNamedVariant & tVariant, SnippetQuerySettings_t & tOpt, CSphString & sError )
{
	CSphString sName = tVariant.m_sKey;
	sName.ToLower();

	const CSphString & sVal = tVariant.m_sValue;
	int iVal = tVariant.m_iValue;
	bool bVal = tVariant.m_iValue!=0;

	if ( sName=="before_match" )			tOpt.m_sBeforeMatch = sVal;
	else if ( sName=="after_match" ) 		tOpt.m_sAfterMatch = sVal;
	else if ( sName=="chunk_separator" )	tOpt.m_sChunkSeparator = sVal;
	else if ( sName=="field_separator" )	tOpt.m_sFieldSeparator = sVal;
	else if ( sName=="limit" )				tOpt.m_iLimit = iVal;
	else if ( sName=="around" ) 			tOpt.m_iAround = iVal;
	else if ( sName=="use_boundaries" )		tOpt.m_bUseBoundaries = bVal;
	else if ( sName=="weight_order" )		tOpt.m_bWeightOrder = bVal;
	else if ( sName=="force_all_words" ) 	tOpt.m_bForceAllWords = bVal;
	else if ( sName=="limit_passages" ) 	tOpt.m_iLimitPassages = iVal;
	else if ( sName=="limit_words" ) 		tOpt.m_iLimitWords = iVal;
	else if ( sName=="start_passage_id" )	tOpt.m_iPassageId = iVal;
	else if ( sName=="load_files" ) 		tOpt.m_uFilesMode |= bVal ? 1 : 0;
	else if ( sName=="load_files_scattered" ) tOpt.m_uFilesMode |= bVal ? 2 : 0;
	else if ( sName=="html_strip_mode" )	tOpt.m_sStripMode = sVal;
	else if ( sName=="allow_empty" ) 		tOpt.m_bAllowEmpty = bVal;
	else if ( sName=="emit_zones" )			tOpt.m_bEmitZones = bVal;
	else if ( sName=="force_passages" )		tOpt.m_bForcePassages = bVal;
	else if ( sName=="passage_boundary" )	tOpt.m_ePassageSPZ = GetPassageBoundary(sVal);
	else if ( sName=="json_query" )			tOpt.m_bJsonQuery = bVal;
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
		CSphString sQuery = *(CSphString*)pArg;			
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

/// searchd-level expression function
class Expr_Snippet_c : public ISphStringExpr, public QueryExprTraits_c
{
public:
				Expr_Snippet_c ( ISphExpr * pArglist, CSphIndex * pIndex, CSphQueryProfile * pProfiler, CSphString & sError );

	int			StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const override;
	bool		IsDataPtrAttr () const final { return true; }
	void		FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override;
	void		Command ( ESphExprCommand eCmd, void * pArg ) override;
	uint64_t	GetHash ( const ISphSchema &, uint64_t, bool & ) override;

protected:
	CSphRefcountedPtr<ISphExpr>		m_pArgs;
	CSphRefcountedPtr<ISphExpr>		m_pText;
	CSphIndex *						m_pIndex;
	SnippetQuerySettings_t			m_tSnippetQuery;
	CSphQueryProfile *				m_pProfiler;
	CSphScopedPtr<SnippetBuilder_i>	m_pSnippetBuilder;
};


Expr_Snippet_c::Expr_Snippet_c ( ISphExpr * pArglist, CSphIndex * pIndex, CSphQueryProfile * pProfiler, CSphString & sError )
	: QueryExprTraits_c ( pArglist->GetArg(1) )
	, m_pArgs ( pArglist )
	, m_pIndex ( pIndex )
	, m_pProfiler ( pProfiler )
	, m_pSnippetBuilder ( CreateSnippetBuilder() )
{
	SafeAddRef ( m_pArgs );
	assert ( m_pArgs->IsArglist() );

	m_pText = pArglist->GetArg(0);
	SafeAddRef ( m_pText );

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
		char * pWords = const_cast<char *> ( sArgs.cstr() );

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

	m_tSnippetQuery.m_bHasBeforePassageMacro = SnippetTransformPassageMacros ( m_tSnippetQuery.m_sBeforeMatch, m_tSnippetQuery.m_sBeforeMatchPassage );
	m_tSnippetQuery.m_bHasAfterPassageMacro = SnippetTransformPassageMacros ( m_tSnippetQuery.m_sAfterMatch, m_tSnippetQuery.m_sAfterMatchPassage );

	if ( !m_pSnippetBuilder->Setup ( m_pIndex, m_tSnippetQuery, sError ) )
		return;
}


int Expr_Snippet_c::StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
{
	CSphScopedProfile ( m_pProfiler, SPH_QSTATE_SNIPPET );

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

	CSphScopedPtr<TextSource_i> pSource ( CreateSnippetSource ( m_tSnippetQuery.m_uFilesMode, szSource, iLen ) );

	// FIXME! fill in all the missing options; use consthash?
	SnippetResult_t tRes;
	if ( !m_pSnippetBuilder->Build ( pSource.Ptr(), tRes ) )
		return 0;

	if ( m_tSnippetQuery.m_bJsonQuery )
		return PackSnippets ( tRes.m_dResult, tRes.m_dSeparators, m_tSnippetQuery.m_sChunkSeparator.Length(), ppStr );

	assert ( tRes.m_dResult.IsEmpty() || tRes.m_dResult.Last()=='\0' );
	int iResultLength = tRes.m_dResult.GetLength();
	*ppStr = tRes.m_dResult.LeakData();
	// skip trailing zero
	return ( iResultLength ? iResultLength-1 : 0 );
}


void Expr_Snippet_c::FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	if ( m_pText )
		m_pText->FixupLocator ( pOldSchema, pNewSchema );
}


void Expr_Snippet_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( m_pArgs )
		m_pArgs->Command ( eCmd, pArg );

	if ( m_pText )
		m_pText->Command ( eCmd, pArg );

	if ( QueryExprTraits_c::Command ( eCmd, pArg ) )
	{
		// fixme! handle errors
		CSphString sError;
		m_pSnippetBuilder->SetQuery ( GetQuery(), false, sError );
	}
}


uint64_t Expr_Snippet_c::GetHash ( const ISphSchema &, uint64_t, bool & )
{
	assert ( 0 && "no snippets in filters" );
	return 0;
}


//////////////////////////////////////////////////////////////////////////

class Expr_Highlight_c : public ISphStringExpr, public QueryExprTraits_c
{
public:
				Expr_Highlight_c ( ISphExpr * pArglist, CSphIndex * pIndex, CSphQueryProfile * pProfiler, CSphString & sError );

	int			StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final;
	void		Command ( ESphExprCommand eCmd, void * pArg ) final;
	void		FixupLocator ( const ISphSchema * /*pOldSchema*/, const ISphSchema * /*pNewSchema*/ ) final {}
	uint64_t	GetHash ( const ISphSchema &, uint64_t, bool & ) final;

private:
	CSphIndex *						m_pIndex = nullptr;
	CSphQueryProfile *				m_pProfiler = nullptr;
	CSphScopedPtr<SnippetBuilder_i>	m_pSnippetBuilder;
	DocstoreSession_c::Info_t		m_tSession;
	SnippetQuerySettings_t			m_tSnippetQuery;
	CSphBitvec						m_tRequestedFields;
	StrVec_t						m_dRequestedFields;
	CSphVector<int>					m_dFieldsToFetch;
	CSphRefcountedPtr<ISphExpr>		m_pArgs;

	bool		FetchFieldsFromDocstore ( DocstoreDoc_t & tFetchedDoc, DocID_t & tDocID ) const;
	void		ParseFields ( ISphExpr * pExpr );
	bool		ParseOptions ( const CSphVector<CSphNamedVariant> & dMap, CSphString & sError );
	bool		MarkRequestedFields ( CSphString & sError );
	void		MarkAllFields();
};


Expr_Highlight_c::Expr_Highlight_c ( ISphExpr * pArglist, CSphIndex * pIndex, CSphQueryProfile * pProfiler, CSphString & sError )
	: QueryExprTraits_c ( ( pArglist && pArglist->IsArglist() && pArglist->GetNumArgs()==3 ) ? pArglist->GetArg(2) : nullptr )
	, m_pIndex ( pIndex )
	, m_pProfiler ( pProfiler )
	, m_pSnippetBuilder ( CreateSnippetBuilder() )
	, m_tRequestedFields ( m_pIndex->GetMatchSchema().GetFieldsCount() )
{
	assert ( m_pIndex );

	if ( pArglist && pArglist->IsArglist() )
	{
		m_pArgs = pArglist;
		SafeAddRef(m_pArgs);
	}

	if ( !m_pSnippetBuilder->Setup ( m_pIndex, m_tSnippetQuery, sError ) )
		return;

	int iNumArgs = pArglist ? ( pArglist->IsArglist() ? pArglist->GetNumArgs() : 1 ) : 0;

	if ( iNumArgs>=1 )
	{
		// this should be a map argument. at least we checked that in ExprHook_c::GetReturnType
		auto pMapArg = (Expr_MapArg_c *)(pArglist->IsArglist() ? pArglist->GetArg(0) : pArglist);
		assert(pMapArg);
		if ( !ParseOptions ( pMapArg->m_dValues, sError ) )
			return;
	}

	if ( iNumArgs>=2 )
	{
		assert ( pArglist && pArglist->IsArglist() );
		ISphExpr * pFields = pArglist->GetArg(1);
		ParseFields(pFields);
	}
	else
		MarkAllFields();
}


int	Expr_Highlight_c::StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
{
	CSphScopedProfile ( m_pProfiler, SPH_QSTATE_SNIPPET );

	DocID_t tDocID = sphGetDocID ( tMatch.m_pDynamic ? tMatch.m_pDynamic : tMatch.m_pStatic );

	DocstoreDoc_t tFetchedDoc;
	if ( !FetchFieldsFromDocstore ( tFetchedDoc, tDocID ) )
		return 0;

	// now we've fetched all stored fields
	// we need to arrange them as in original index schema
	// so that field matching will work as expected
	const CSphSchema & tSchema = m_pIndex->GetMatchSchema();

	CSphVector<VecTraits_T<BYTE>> dAllFields;

	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
	{
		const CSphColumnInfo & tInfo = tSchema.GetField(i);
		if ( !( tInfo.m_uFieldFlags & CSphColumnInfo::FIELD_STORED ) )
		{
			dAllFields.Add ( VecTraits_T<BYTE>(nullptr,0) );
			continue;
		}

		int iFieldId = m_tSession.m_pDocstore->GetFieldId ( tInfo.m_sName, DOCSTORE_TEXT );
		assert ( iFieldId!=-1 );

		int iFetchedFieldId = -1;
		if ( m_dFieldsToFetch.GetLength() )
		{
			int * pFound = sphBinarySearch ( m_dFieldsToFetch.Begin(), m_dFieldsToFetch.Begin()+m_dFieldsToFetch.GetLength()-1, iFieldId );
			if ( pFound )
				iFetchedFieldId = pFound-m_dFieldsToFetch.Begin();
		}
		else
			iFetchedFieldId = iFieldId;

		if ( iFetchedFieldId!=-1 )
			dAllFields.Add ( VecTraits_T<BYTE>( tFetchedDoc.m_dFields[iFetchedFieldId].Begin(), tFetchedDoc.m_dFields[iFetchedFieldId].GetLength() ) );
		else
			dAllFields.Add ( VecTraits_T<BYTE>(nullptr,0) );
	}

	if ( UpdateQuery(tMatch) )
	{
		CSphString sError;
		if ( !m_pSnippetBuilder->SetQuery ( GetQuery(), true, sError ) )
			return 0;
	}
	
	CSphScopedPtr<TextSource_i> pSource ( CreateHighlightSource(dAllFields) );

	SnippetResult_t tRes;
	if ( !m_pSnippetBuilder->Build ( pSource.Ptr(), tRes ) )
		return 0;

	assert ( tRes.m_dResult.IsEmpty() || tRes.m_dResult.Last()=='\0' );
	int iResultLength = tRes.m_dResult.GetLength();
	*ppStr = tRes.m_dResult.LeakData();
	// skip trailing zero
	return ( iResultLength ? iResultLength-1 : 0 );
}


void Expr_Highlight_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( QueryExprTraits_c::Command ( eCmd, pArg ) )
	{
		// fixme! handle errors
		CSphString sError;
		m_pSnippetBuilder->SetQuery ( GetQuery(), false, sError );
	}

	if ( eCmd==SPH_EXPR_SET_DOCSTORE )
	{
		const DocstoreSession_c::Info_t & tSession = *(DocstoreSession_c::Info_t*)pArg;
		bool bMark = tSession.m_pDocstore!=m_tSession.m_pDocstore;
		m_tSession = tSession;

		if ( bMark )
		{
			// fixme! handle errors
			CSphString sError;
			MarkRequestedFields(sError);
		}
	}

	if ( m_pArgs )
		m_pArgs->Command ( eCmd, pArg );
}


uint64_t Expr_Highlight_c::GetHash ( const ISphSchema &, uint64_t, bool & )
{
	assert ( 0 && "no snippets in filters" );
	return 0;
}


bool Expr_Highlight_c::FetchFieldsFromDocstore ( DocstoreDoc_t & tFetchedDoc, DocID_t & tDocID ) const
{
	if ( !m_tSession.m_pDocstore )
		return false;

	const CSphVector<int> * pFieldsToFetch = m_tRequestedFields.BitCount()==m_tRequestedFields.GetBits() ? nullptr : &m_dFieldsToFetch;
	return m_tSession.m_pDocstore->GetDoc ( tFetchedDoc, tDocID, pFieldsToFetch, m_tSession.m_iSessionId, false );
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

	sphSplit ( m_dRequestedFields, sFields.cstr() );
}


void Expr_Highlight_c::MarkAllFields()
{
	m_tRequestedFields.Clear();
	m_dFieldsToFetch.Resize(0);

	const CSphSchema & tSchema = m_pIndex->GetMatchSchema();
	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
		m_tRequestedFields.BitSet(i);
}


bool Expr_Highlight_c::MarkRequestedFields ( CSphString & sError )
{
	m_tRequestedFields.Clear();
	m_dFieldsToFetch.Resize(0);

	bool bResult = true;

	if ( m_dRequestedFields.GetLength() )
	{
		assert ( m_tSession.m_pDocstore );
		const CSphSchema & tSchema = m_pIndex->GetMatchSchema();

		for ( const auto & i : m_dRequestedFields )
		{
			int iDocstoreField = m_tSession.m_pDocstore->GetFieldId ( i, DOCSTORE_TEXT );
			if ( iDocstoreField==-1 )
			{
				sError.SetSprintf ( "field %s not found", i.cstr() );
				bResult = false;
				continue;
			}

			for ( int iSchemaField = 0; iSchemaField<tSchema.GetFieldsCount(); iSchemaField++ )
				if ( tSchema.GetField(iSchemaField).m_sName==i )
				{
					m_tRequestedFields.BitSet(iSchemaField);
					break;
				}

			m_dFieldsToFetch.Add(iDocstoreField);
		}

		if ( !bResult && !m_tRequestedFields.BitCount() )
			MarkAllFields();
	}
	else
		MarkAllFields();

	return bResult;
}


bool Expr_Highlight_c::ParseOptions ( const CSphVector<CSphNamedVariant> & dMap, CSphString & sError )
{
	for ( const auto & i : dMap )
	{
		if ( !ParseSnippetOption ( i, m_tSnippetQuery, sError ) )
			return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////

int ExprHook_c::IsKnownFunc ( const char * sFunc )
{
	if ( !strcasecmp ( sFunc, "SNIPPET" ) )
		return HOOK_SNIPPET;

	if ( !strcasecmp ( sFunc, "HIGHLIGHT" ) )
		return HOOK_HIGHLIGHT;

	return -1;
}


ISphExpr * ExprHook_c::CreateNode ( int iID, ISphExpr * pLeft, ESphEvalStage * pEvalStage, CSphString & sError )
{
	if ( pEvalStage )
		*pEvalStage = SPH_EVAL_POSTLIMIT;

	ISphExpr * pRes = nullptr;

	switch ( iID )
	{
	case HOOK_SNIPPET:
		pRes = new Expr_Snippet_c ( pLeft, m_pIndex, m_pProfiler, sError );
		break;

	case HOOK_HIGHLIGHT:
		pRes = new Expr_Highlight_c ( pLeft, m_pIndex, m_pProfiler, sError );
		break;

	default:
		assert ( 0 && "Unknown node type" );
		return nullptr;
	}

	if ( !sError.IsEmpty() )
		SafeDelete(pRes);

	return pRes;
}


ESphAttr ExprHook_c::GetIdentType ( int )
{
	assert(0);
	return SPH_ATTR_NONE;
}


ESphAttr ExprHook_c::GetReturnType ( int iID, const CSphVector<ESphAttr> & dArgs, bool, CSphString & sError )
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

		if ( dArgs.GetLength()>1 && dArgs[1]!=SPH_ATTR_STRING)
		{
			sError = "2nd argument to HIGHLIGHT() must be a const string";
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