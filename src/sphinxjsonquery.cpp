//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxquery.h"
#include "sphinxsearch.h"
#include "sphinxplugin.h"
#include "sphinxutils.h"
#include "searchdaemon.h"
#include "jsonqueryfilter.h"
#include "attribute.h"
#include "searchdsql.h"

#include "json/cJSON.h"

static const char * g_szAll = "_all";
static const char * g_szHighlight = "_@highlight_";
static const char * g_szOrder = "_@order_";


class QueryTreeBuilder_c : public XQParseHelper_c
{
public:
					QueryTreeBuilder_c ( const CSphQuery * pQuery, TokenizerRefPtr_c pQueryTokenizerQL, const CSphIndexSettings & tSettings );

	void			CollectKeywords ( const char * szStr, XQNode_t * pNode, const XQLimitSpec_t & tLimitSpec );

	bool			HandleFieldBlockStart ( const char * & /*pPtr*/ ) override { return true; }
	bool			HandleSpecialFields ( const char * & pPtr, FieldMask_t & dFields ) override;
	bool			NeedTrailingSeparator() override { return false; }

	XQNode_t *		CreateNode ( XQLimitSpec_t & tLimitSpec );

	const TokenizerRefPtr_c &	GetQLTokenizer() { return m_pQueryTokenizerQL; }
	const CSphIndexSettings &	GetIndexSettings() { return m_tSettings; }
	const CSphQuery *			GetQuery() { return m_pQuery; }

private:
	const CSphQuery *			m_pQuery {nullptr};
	const TokenizerRefPtr_c		m_pQueryTokenizerQL;
	const CSphIndexSettings &	m_tSettings;

	XQNode_t *		AddChildKeyword ( XQNode_t * pParent, const char * szKeyword, int iSkippedPosBeforeToken, const XQLimitSpec_t & tLimitSpec );
};


QueryTreeBuilder_c::QueryTreeBuilder_c ( const CSphQuery * pQuery, TokenizerRefPtr_c pQueryTokenizerQL, const CSphIndexSettings & tSettings )
	: m_pQuery ( pQuery )
	, m_pQueryTokenizerQL ( std::move (pQueryTokenizerQL) )
	, m_tSettings ( tSettings )
{}


void QueryTreeBuilder_c::CollectKeywords ( const char * szStr, XQNode_t * pNode, const XQLimitSpec_t & tLimitSpec )
{
	m_pTokenizer->SetBuffer ( (const BYTE*)szStr, (int) strlen ( szStr ) );

	while (true)
	{
		int iSkippedPosBeforeToken = 0;
		if ( m_bWasBlended )
		{
			iSkippedPosBeforeToken = m_pTokenizer->SkipBlended();
			// just add all skipped blended parts except blended head (already added to atomPos)
			if ( iSkippedPosBeforeToken>1 )
				m_iAtomPos += iSkippedPosBeforeToken - 1;
		}

		// FIXME!!! only wildcard node need tokes with wildcard symbols
		const char * sToken = (const char *) m_pTokenizer->GetToken ();
		if ( !sToken )
		{
			AddChildKeyword ( pNode, nullptr, iSkippedPosBeforeToken, tLimitSpec );
			break;
		}

		// now let's do some token post-processing
		m_bWasBlended = m_pTokenizer->TokenIsBlended();

		int iPrevDeltaPos = 0;
		if ( m_pPlugin && m_pPlugin->m_fnPushToken )
			sToken = m_pPlugin->m_fnPushToken ( m_pPluginData, const_cast<char*>(sToken), &iPrevDeltaPos, m_pTokenizer->GetTokenStart(), int ( m_pTokenizer->GetTokenEnd() - m_pTokenizer->GetTokenStart() ) );

		m_iAtomPos += 1 + iPrevDeltaPos;

		bool bMultiDestHead = false;
		bool bMultiDest = false;
		int iDestCount = 0;
		// do nothing inside phrase
		if ( !m_pTokenizer->m_bPhrase )
			bMultiDest = m_pTokenizer->WasTokenMultiformDestination ( bMultiDestHead, iDestCount );

		// check for stopword, and create that node
		// temp buffer is required, because GetWordID() might expand (!) the keyword in-place
		BYTE sTmp [ MAX_TOKEN_BYTES ];

		strncpy ( (char*)sTmp, sToken, MAX_TOKEN_BYTES );
		sTmp[MAX_TOKEN_BYTES-1] = '\0';

		int iStopWord = 0;
		if ( m_pPlugin && m_pPlugin->m_fnPreMorph )
			m_pPlugin->m_fnPreMorph ( m_pPluginData, (char*)sTmp, &iStopWord );

		SphWordID_t uWordId = iStopWord ? 0 : m_pDict->GetWordID ( sTmp );

		if ( uWordId && m_pPlugin && m_pPlugin->m_fnPostMorph )
		{
			int iRes = m_pPlugin->m_fnPostMorph ( m_pPluginData, (char*)sTmp, &iStopWord );
			if ( iStopWord )
				uWordId = 0;
			else if ( iRes )
				uWordId = m_pDict->GetWordIDNonStemmed ( sTmp );
		}

		if ( !uWordId )
		{
			sToken = nullptr;
			// stopwords with step=0 must not affect pos
			if ( m_bEmptyStopword )
				m_iAtomPos--;
		}

		XQNode_t * pChildNode = nullptr;
		if ( bMultiDest && !bMultiDestHead )
		{
			assert ( m_dMultiforms.GetLength() );
			m_dMultiforms.Last().m_iDestCount++;
			m_dDestForms.Add ( sToken );
		} else
			pChildNode = AddChildKeyword ( pNode, sToken, iSkippedPosBeforeToken, tLimitSpec );

		if ( bMultiDestHead )
		{
			MultiformNode_t & tMulti = m_dMultiforms.Add();
			tMulti.m_pNode = pChildNode;
			tMulti.m_iDestStart = m_dDestForms.GetLength();
			tMulti.m_iDestCount = 0;
		}
	}
}


bool QueryTreeBuilder_c::HandleSpecialFields ( const char * & pPtr, FieldMask_t & dFields )
{
	if ( *pPtr=='_' )
	{
		auto iLen = (int) strlen(g_szAll);
		if ( !strncmp ( pPtr, g_szAll, iLen ) )
		{
			pPtr += iLen;
			dFields.SetAll();
			return true;
		}
	}

	return false;
}


XQNode_t * QueryTreeBuilder_c::CreateNode ( XQLimitSpec_t & tLimitSpec )
{
	auto * pNode = new XQNode_t(tLimitSpec);
	m_dSpawned.Add ( pNode );
	return pNode;
}


XQNode_t * QueryTreeBuilder_c::AddChildKeyword ( XQNode_t * pParent, const char * szKeyword, int iSkippedPosBeforeToken, const XQLimitSpec_t & tLimitSpec )
{
	XQKeyword_t tKeyword ( szKeyword, m_iAtomPos );
	tKeyword.m_iSkippedBefore = iSkippedPosBeforeToken;
	auto * pNode = new XQNode_t ( tLimitSpec );
	pNode->m_pParent = pParent;
	pNode->m_dWords.Add ( tKeyword );
	pParent->m_dChildren.Add ( pNode );
	m_dSpawned.Add ( pNode );

	return pNode;
}

//////////////////////////////////////////////////////////////////////////

class QueryParserJson_c : public QueryParser_i
{
public:
	bool	IsFullscan ( const XQQuery_t & tQuery ) const final;
	bool	ParseQuery ( XQQuery_t & tParsed, const char * sQuery, const CSphQuery * pQuery,
		TokenizerRefPtr_c pQueryTokenizer, TokenizerRefPtr_c pQueryTokenizerJson,
		const CSphSchema * pSchema, const DictRefPtr_c& pDict, const CSphIndexSettings & tSettings ) const final;

private:
	XQNode_t *		ConstructMatchNode ( const JsonObj_c & tJson, bool bPhrase, bool bTerms, bool bSingleTerm, QueryTreeBuilder_c & tBuilder ) const;
	XQNode_t *		ConstructBoolNode ( const JsonObj_c & tJson, QueryTreeBuilder_c & tBuilder ) const;
	XQNode_t *		ConstructQLNode ( const JsonObj_c & tJson, QueryTreeBuilder_c & tBuilder ) const;
	XQNode_t *		ConstructMatchAllNode ( QueryTreeBuilder_c & tBuilder ) const;

	bool			ConstructBoolNodeItems ( const JsonObj_c & tClause, CSphVector<XQNode_t *> & dItems, QueryTreeBuilder_c & tBuilder ) const;
	bool			ConstructNodeOrFilter ( const JsonObj_c & tItem, CSphVector<XQNode_t *> & dNodes, QueryTreeBuilder_c & tBuilder ) const;

	XQNode_t *		ConstructNode ( const JsonObj_c & tJson, QueryTreeBuilder_c & tBuilder ) const;
};


bool QueryParserJson_c::IsFullscan ( const XQQuery_t & tQuery ) const
{
	return !( tQuery.m_pRoot && ( tQuery.m_pRoot->m_dChildren.GetLength () || tQuery.m_pRoot->m_dWords.GetLength () ) );
}


bool QueryParserJson_c::ParseQuery ( XQQuery_t & tParsed, const char * szQuery, const CSphQuery * pQuery, TokenizerRefPtr_c pQueryTokenizerQL, TokenizerRefPtr_c pQueryTokenizerJson, const CSphSchema * pSchema, const DictRefPtr_c& pDict, const CSphIndexSettings & tSettings ) const
{
	JsonObj_c tRoot ( szQuery );

	// take only the first item of the query; ignore the rest
	int iNumIndexes = ( tRoot.Empty() ? 0 : tRoot.Size() );
	if ( !iNumIndexes )
	{
		tParsed.m_sParseError = "\"query\" property is empty";
		return false;
	}

	assert ( pQueryTokenizerJson->IsQueryTok() );
	DictRefPtr_c pMyDict = GetStatelessDict ( pDict );

	QueryTreeBuilder_c tBuilder ( pQuery, std::move ( pQueryTokenizerQL ), tSettings );
	tBuilder.Setup ( pSchema, pQueryTokenizerJson->Clone ( SPH_CLONE ), pMyDict, &tParsed, tSettings );

	tParsed.m_pRoot = ConstructNode ( tRoot[0], tBuilder );
	if ( tBuilder.IsError() )
	{
		tBuilder.Cleanup();
		return false;
	}

	XQLimitSpec_t tLimitSpec;
	tParsed.m_pRoot = tBuilder.FixupTree ( tParsed.m_pRoot, tLimitSpec, IsAllowOnlyNot() );
	if ( tBuilder.IsError() )
	{
		tBuilder.Cleanup();
		return false;
	}

	return true;
}


static const char * g_szOperatorNames[]=
{
	"and",
	"or"
};


static XQOperator_e StrToNodeOp ( const char * szStr )
{
	if ( !szStr )
		return SPH_QUERY_TOTAL;

	int iOp=0;
	for ( auto i : g_szOperatorNames )
	{
		if ( !strcmp ( szStr, i ) )
			return XQOperator_e(iOp);

		iOp++;
	}

	return SPH_QUERY_TOTAL;
}


static bool IsBoolNode ( const JsonObj_c & tJson )
{
	if ( !tJson )
		return false;

	return CSphString ( tJson.Name() )=="bool";
}


XQNode_t * QueryParserJson_c::ConstructMatchNode ( const JsonObj_c & tJson, bool bPhrase, bool bTerms, bool bSingleTerm, QueryTreeBuilder_c & tBuilder ) const
{
	if ( !tJson.IsObj() )
	{
		tBuilder.Error ( "\"match\" value should be an object" );
		return nullptr;
	}

	if ( tJson.Size()!=1 )
	{
		tBuilder.Error ( "ill-formed \"match\" property" );
		return nullptr;
	}

	JsonObj_c tFields = tJson[0];
	tBuilder.SetString ( tFields.Name() );

	XQLimitSpec_t tLimitSpec;
	const char * szQuery = nullptr;
	XQOperator_e eNodeOp = bPhrase ? SPH_QUERY_PHRASE : SPH_QUERY_OR;
	bool bIgnore = false;
	StringBuilder_c tTermsBuf ( " " );

	if ( !tBuilder.ParseFields ( tLimitSpec.m_dFieldMask, tLimitSpec.m_iFieldMaxPos, bIgnore ) )
		return nullptr;

	if ( bIgnore )
	{
		tBuilder.Warning ( R"(ignoring fields in "%s", using "_all")", tFields.Name() );
		tLimitSpec.Reset();
	}

	tLimitSpec.m_bFieldSpec = true;

	if ( bTerms )
	{
		if ( !tFields.IsArray() )
		{
			tBuilder.Warning ( "values of properties in \"terms\" should be an array" );
			return nullptr;
		}

		for ( const auto & tTerm : tFields )
		{
			if ( !tTerm.IsStr() )
			{
				tBuilder.Error ( "\"terms\" value should be a string" );
				return nullptr;
			}

			tTermsBuf += tTerm.SzVal();
		}

		szQuery = tTermsBuf.cstr();

	} else if ( tFields.IsObj() )
	{
		// matching with flags
		CSphString sError;
		JsonObj_c tQuery = ( bSingleTerm ? tFields.GetStrItem ( "value", sError ) : tFields.GetStrItem ( "query", sError ) );
		if ( !tQuery )
		{
			tBuilder.Error ( "%s", sError.cstr() );
			return nullptr;
		}

		szQuery = tQuery.SzVal();

		if ( !bPhrase )
		{
			JsonObj_c tOp = tFields.GetItem ( "operator" );
			if ( tOp ) // "and", "or"
			{
				eNodeOp = StrToNodeOp ( tOp.SzVal() );
				if ( eNodeOp==SPH_QUERY_TOTAL )
				{
					tBuilder.Error ( "unknown operator: \"%s\"", tOp.SzVal() );
					return nullptr;
				}
			}
		}
	} else
	{
		// simple list of keywords
		if ( !tFields.IsStr() )
		{
			tBuilder.Warning ( "values of properties in \"match\" should be strings or objects" );
			return nullptr;
		}

		szQuery = tFields.SzVal();
	}

	assert ( szQuery );

	XQNode_t * pNewNode = tBuilder.CreateNode ( tLimitSpec );
	pNewNode->SetOp ( eNodeOp );
	tBuilder.CollectKeywords ( szQuery, pNewNode, tLimitSpec );

	return pNewNode;
}


bool QueryParserJson_c::ConstructNodeOrFilter ( const JsonObj_c & tItem, CSphVector<XQNode_t *> & dNodes, QueryTreeBuilder_c & tBuilder ) const
{
	if ( !tItem )
		return true;

	// we created filters before, no need to process them again
	if ( IsFilter(tItem) )
		return true;

	XQNode_t * pNode = ConstructNode ( tItem, tBuilder );
	if ( !pNode )
		return IsBoolNode ( tItem ); // need walk down the tree for compart mode

	dNodes.Add ( pNode );
	return true;
}


bool QueryParserJson_c::ConstructBoolNodeItems ( const JsonObj_c & tClause, CSphVector<XQNode_t *> & dItems, QueryTreeBuilder_c & tBuilder ) const
{
	if ( tClause.IsArray() )
	{
		for ( const auto & tObject : tClause )
		{
			if ( !tObject.IsObj() )
			{
				tBuilder.Error ( "\"%s\" array value should be an object", tClause.Name() );
				return false;
			}

			if ( !ConstructNodeOrFilter ( tObject[0], dItems, tBuilder ) )
				return false;
		}
	} else if ( tClause.IsObj() )
	{
		if ( !ConstructNodeOrFilter ( tClause[0], dItems, tBuilder ) )
			return false;
	} else
	{
		tBuilder.Error ( "\"%s\" value should be an object or an array", tClause.Name() );
		return false;
	}

	return true;
}


XQNode_t * QueryParserJson_c::ConstructBoolNode ( const JsonObj_c & tJson, QueryTreeBuilder_c & tBuilder ) const
{
	if ( !tJson.IsObj() )
	{
		tBuilder.Error ( "\"bool\" value should be an object" );
		return nullptr;
	}

	CSphVector<XQNode_t *> dMust, dShould, dMustNot;

	for ( const auto & tClause : tJson )
	{
		CSphString sName = tClause.Name();
		if ( sName=="must" )
		{
			if ( !ConstructBoolNodeItems ( tClause, dMust, tBuilder ) )
				return nullptr;
		} else if ( sName=="should" )
		{
			if ( !ConstructBoolNodeItems ( tClause, dShould, tBuilder ) )
				return nullptr;
		} else if ( sName=="must_not" )
		{
			if ( !ConstructBoolNodeItems ( tClause, dMustNot, tBuilder ) )
				return nullptr;
		} else if ( sName=="filter" )
		{
			if ( !ConstructBoolNodeItems ( tClause, dMust, tBuilder ) )
				return nullptr;
		} else if ( sName=="minimum_should_match" ) // FIXME!!! add to should as option
		{
			continue;
		} else
		{
			tBuilder.Error ( "unknown bool query type: \"%s\"", sName.cstr() );
			return nullptr;
		}
	}

	XQNode_t * pMustNode = nullptr;
	XQNode_t * pShouldNode = nullptr;
	XQNode_t * pMustNotNode = nullptr;

	XQLimitSpec_t tLimitSpec;

	if ( dMust.GetLength() )
	{
		// no need to construct AND node for a single child
		if ( dMust.GetLength()==1 )
			pMustNode = dMust[0];
		else
		{
			XQNode_t * pAndNode = tBuilder.CreateNode ( tLimitSpec );
			pAndNode->SetOp ( SPH_QUERY_AND );

			for ( auto & i : dMust )
			{
				pAndNode->m_dChildren.Add(i);
				i->m_pParent = pAndNode;
			}

			pMustNode = pAndNode;
		}
	}

	if ( dShould.GetLength() )
	{
		if ( dShould.GetLength()==1 )
			pShouldNode = dShould[0];
		else
		{
			XQNode_t * pOrNode = tBuilder.CreateNode ( tLimitSpec );
			pOrNode->SetOp ( SPH_QUERY_OR );

			for ( auto & i : dShould )
			{
				pOrNode->m_dChildren.Add(i);
				i->m_pParent = pOrNode;
			}

			pShouldNode = pOrNode;
		}
	}

	// slightly different case - we need to construct the NOT node anyway
	if ( dMustNot.GetLength() )
	{
		XQNode_t * pNotNode = tBuilder.CreateNode ( tLimitSpec );
		pNotNode->SetOp ( SPH_QUERY_NOT );

		if ( dMustNot.GetLength()==1 )
		{
			pNotNode->m_dChildren.Add ( dMustNot[0] );
			dMustNot[0]->m_pParent = pNotNode;
		} else
		{
			XQNode_t * pOrNode = tBuilder.CreateNode ( tLimitSpec );
			pOrNode->SetOp ( SPH_QUERY_OR );

			for ( auto & i : dMustNot )
			{
				pOrNode->m_dChildren.Add ( i );
				i->m_pParent = pOrNode;
			}

			pNotNode->m_dChildren.Add ( pOrNode );
			pOrNode->m_pParent = pNotNode;
		}

		pMustNotNode = pNotNode;
	}

	int iTotalNodes = 0;
	iTotalNodes += pMustNode ? 1 : 0;
	iTotalNodes += pShouldNode ? 1 : 0;
	iTotalNodes += pMustNotNode ? 1 : 0;

	XQNode_t * pResultNode = nullptr;

	if ( !iTotalNodes )
		return nullptr;
	else if ( iTotalNodes==1 )
	{
		if ( pMustNode )
			pResultNode = pMustNode;
		else if ( pShouldNode )
			pResultNode = pShouldNode;
		else
			pResultNode = pMustNotNode;

		assert ( pResultNode );
	} else
	{
		pResultNode = pMustNode ? pMustNode : pMustNotNode;
		assert ( pResultNode );
		
		// combine 'must' and 'must_not' with AND
		if ( pMustNode && pMustNotNode )
		{
			XQNode_t * pAndNode = tBuilder.CreateNode(tLimitSpec);
			pAndNode->SetOp(SPH_QUERY_AND);
			pAndNode->m_dChildren.Add ( pMustNode );
			pAndNode->m_dChildren.Add ( pMustNotNode );
			pMustNode->m_pParent = pAndNode;
			pMustNotNode->m_pParent = pAndNode;

			pResultNode = pAndNode;
		}

		// combine 'result' node and 'should' node with MAYBE
		if ( pShouldNode )
		{
			XQNode_t * pMaybeNode = tBuilder.CreateNode ( tLimitSpec );
			pMaybeNode->SetOp ( SPH_QUERY_MAYBE );
			pMaybeNode->m_dChildren.Add ( pResultNode );
			pMaybeNode->m_dChildren.Add ( pShouldNode );
			pShouldNode->m_pParent = pMaybeNode;
			pResultNode->m_pParent = pMaybeNode;

			pResultNode = pMaybeNode;
		}
	}

	return pResultNode;
}


XQNode_t * QueryParserJson_c::ConstructQLNode ( const JsonObj_c & tJson, QueryTreeBuilder_c & tBuilder ) const
{
	if ( !tJson.IsStr() )
	{
		tBuilder.Error ( "\"query_string\" value should be an string" );
		return nullptr;
	}

	XQQuery_t tParsed;
	tParsed.m_dZones = tBuilder.GetZone(); // should keep the same zone list for whole tree
	if ( !sphParseExtendedQuery ( tParsed, tJson.StrVal().cstr(), tBuilder.GetQuery(), tBuilder.GetQLTokenizer(), tBuilder.GetSchema(), tBuilder.GetDict(), tBuilder.GetIndexSettings() ) )
	{
		tBuilder.Error ( "%s", tParsed.m_sParseError.cstr() );
		return nullptr;
	}

	if ( !tParsed.m_sParseWarning.IsEmpty() )
		tBuilder.Warning ( "%s", tParsed.m_sParseWarning.cstr() );

	XQNode_t * pRoot = tParsed.m_pRoot;
	tParsed.m_pRoot = nullptr;
	tBuilder.SetZone ( tParsed.m_dZones );
	return pRoot;
}


XQNode_t * QueryParserJson_c::ConstructMatchAllNode ( QueryTreeBuilder_c & tBuilder ) const
{
	XQLimitSpec_t tLimitSpec;
	XQNode_t * pNewNode = tBuilder.CreateNode ( tLimitSpec );
	pNewNode->SetOp ( SPH_QUERY_NULL );
	return pNewNode;
}


XQNode_t * QueryParserJson_c::ConstructNode ( const JsonObj_c & tJson, QueryTreeBuilder_c & tBuilder ) const
{
	CSphString sName = tJson.Name();
	if ( !tJson || sName.IsEmpty() )
	{
		tBuilder.Error ( "empty json found" );
		return nullptr;
	}

	bool bMatch = ( sName=="match" );
	bool bTerms = ( sName=="terms" );
	bool bPhrase = sName=="match_phrase";
	bool bSingleTerm = ( sName=="term" );
	if ( bMatch || bPhrase || bTerms || bSingleTerm )
		return ConstructMatchNode ( tJson, bPhrase, bTerms, bSingleTerm, tBuilder );

	if ( sName=="match_all" )
		return ConstructMatchAllNode ( tBuilder );

	if ( sName=="bool" )
		return ConstructBoolNode ( tJson, tBuilder );

	if ( sName=="query_string" )
		return ConstructQLNode ( tJson, tBuilder );

	if ( sName=="simple_query_string" && tJson.IsObj() )
		return ConstructQLNode ( tJson.GetItem ( "query" ), tBuilder );

	return nullptr;
}

bool NonEmptyQuery ( const JsonObj_c & tQuery )
{
	return ( tQuery.HasItem("match")
	|| tQuery.HasItem("match_phrase")
	|| tQuery.HasItem("bool") )
	|| tQuery.HasItem("query_string");
}


//////////////////////////////////////////////////////////////////////////

static bool ParseSnippet ( const JsonObj_c & tSnip, CSphQuery & tQuery, CSphString & sError );
static bool ParseSort ( const JsonObj_c & tSort, JsonQuery_c & tQuery, bool & bGotWeight, CSphString & sError, CSphString & sWarning );
static bool ParseSelect ( const JsonObj_c & tSelect, CSphQuery & tQuery, CSphString & sError );
static bool ParseScriptFields ( const JsonObj_c & tExpr, CSphQuery & tQuery, CSphString & sError );
static bool ParseExpressions ( const JsonObj_c & tExpr, CSphQuery & tQuery, CSphString & sError );
static bool ParseDocFields ( const JsonObj_c & tDocFields, JsonQuery_c & tQuery, CSphString & sError );
static bool ParseAggregates ( const JsonObj_c & tAggs, JsonQuery_c & tQuery, CSphString & sError );

static bool ParseIndex ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, CSphString & sError )
{
	if ( !tRoot )
	{
		sError.SetSprintf ( "unable to parse: %s", tRoot.GetErrorPtr() );
		return false;
	}

	JsonObj_c tIndex = tRoot.GetStrItem ( "index", sError );
	if ( !tIndex )
		return false;

	tStmt.m_sIndex = tIndex.StrVal();
	tStmt.m_tQuery.m_sIndexes = tStmt.m_sIndex;

	const char * sIndexStart = strchr ( tStmt.m_sIndex.cstr(), ':' );
	if ( sIndexStart!=nullptr )
	{
		const char * sIndex = tStmt.m_sIndex.cstr();
		sError.SetSprintf ( "wrong table at cluster syntax, use \"cluster\": \"%.*s\" and \"index\": \"%s\" properties, instead of '%s'",
			(int)(sIndexStart-sIndex), sIndex, sIndexStart+1, sIndex );
		return false;
	}

	return true;
}


static bool ParseIndexId ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError )
{
	if ( !ParseIndex ( tRoot, tStmt, sError ) )
		return false;

	JsonObj_c tId = tRoot.GetIntItem ( "id", sError );

	if ( tId )
		tDocId = tId.IntVal();
	else
		tDocId = 0; 	// enable auto-id

	return true;
}

static bool ParseCluster ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, CSphString & sError )
{
	if ( !tRoot )
	{
		sError.SetSprintf ( "unable to parse: %s", tRoot.GetErrorPtr() );
		return false;
	}

	// cluster is optional
	JsonObj_c tCluster = tRoot.GetStrItem ( "cluster", sError, true );
	if ( tCluster )
		tStmt.m_sCluster = tCluster.StrVal();

	return true;
}


std::unique_ptr<QueryParser_i> sphCreateJsonQueryParser()
{
	return std::make_unique<QueryParserJson_c>();
}


static bool ParseLimits ( const JsonObj_c & tRoot, CSphQuery & tQuery, CSphString & sError )
{
	JsonObj_c tLimit = tRoot.GetIntItem ( "limit", "size", sError );
	if ( !sError.IsEmpty() )
		return false;

	if ( tLimit )
		tQuery.m_iLimit = (int)tLimit.IntVal();

	JsonObj_c tOffset = tRoot.GetIntItem ( "offset", "from", sError );
	if ( !sError.IsEmpty() )
		return false;

	if ( tOffset )
		tQuery.m_iOffset = (int)tOffset.IntVal();

	JsonObj_c tMaxMatches = tRoot.GetIntItem ( "max_matches", sError, true );
	if ( !sError.IsEmpty() )
		return false;

	if ( tMaxMatches )
	{
		tQuery.m_iMaxMatches = (int)tMaxMatches.IntVal();
		tQuery.m_bExplicitMaxMatches = true;
	}

	return true;
}


static bool ParseOptions ( const JsonObj_c & tRoot, CSphQuery & tQuery, CSphString & sError )
{
	if ( !tRoot.IsObj() )
	{	
		sError = "\"options\" property value should be an object";
		return false;
	}

	for ( const auto & i : tRoot )
	{
		AddOption_e eAdd = AddOption_e::NOT_FOUND;
		CSphString sOpt = i.Name();
		if ( i.IsInt() )
			eAdd = AddOption ( tQuery, sOpt, i.StrVal(), i.IntVal(), STMT_SELECT, sError );
		else if ( i.IsStr() )
		{
			CSphString sRanker = i.StrVal();
			const char * szRanker = sRanker.cstr();
			while ( sphIsAlpha(*szRanker) )
				szRanker++;

			if ( *szRanker=='(' && sRanker.Ends(")")  )
			{
				int iRankerNameLen = szRanker-sRanker.cstr();
				CSphString sExpr = sRanker.SubString (iRankerNameLen+1, sRanker.Length()-iRankerNameLen-2 );
				sExpr.Unquote();

				sRanker = sRanker.SubString ( 0, iRankerNameLen );
				eAdd = ::AddOptionRanker ( tQuery, sOpt, sRanker, [sExpr]{ return sExpr; }, STMT_SELECT, sError );
			}

			if ( eAdd==AddOption_e::NOT_FOUND )
				eAdd = AddOption ( tQuery, sOpt, i.StrVal(), [&i]{ return i.StrVal(); }, STMT_SELECT, sError );
		}
		else if ( i.IsObj() )
		{
			CSphVector<CSphNamedInt> dNamed;
			for ( const auto & tNamed : i )
			{
				if ( !tNamed.IsInt() )
				{
					sError.SetSprintf ( "\"%s\" property of \"%s\"' option should be integer", sOpt.cstr(), tNamed.Name() );
					return false;
				}

				dNamed.Add ( { tNamed.Name(), tNamed.IntVal() } );
			}

			eAdd = ::AddOption ( tQuery, sOpt, dNamed, STMT_SELECT, sError );
		}

		if ( eAdd==AddOption_e::NOT_FOUND )
		{
			sError.SetSprintf ( "unknown option '%s'", sOpt.cstr () );
			return false;
		}
		else if ( eAdd==AddOption_e::FAILED )
			return false;
	}

	return true;
}


bool sphParseJsonQuery ( Str_t sQuery, JsonQuery_c & tQuery, bool & bProfile, CSphString & sError, CSphString & sWarning )
{
	JsonObj_c tRoot ( sQuery );
	tQuery.m_sRawQuery = sQuery;

	return sphParseJsonQuery ( tRoot, tQuery, bProfile, sError, sWarning );
}

bool sphParseJsonQuery ( const JsonObj_c & tRoot, JsonQuery_c & tQuery, bool & bProfile, CSphString & sError, CSphString & sWarning )
{
	if ( !tRoot )
	{
		sError.SetSprintf ( "unable to parse: %s", tRoot.GetErrorPtr() );
		return false;
	}

	JsonObj_c tIndex = tRoot.GetStrItem ( "index", sError );
	if ( !tIndex )
		return false;

	tQuery.m_sIndexes = tIndex.StrVal();
	tQuery.m_sIndexes.ToLower();

	if ( tQuery.m_sIndexes==g_szAll )
		tQuery.m_sIndexes = "*";

	if ( !ParseLimits ( tRoot, tQuery, sError ) )
		return false;

	JsonObj_c tJsonQuery = tRoot.GetItem("query");

	// common code used by search queries and update/delete by query
	if ( !ParseJsonQueryFilters ( tJsonQuery, tQuery, sError, sWarning ) )
		return false;

	JsonObj_c tOptions = tRoot.GetItem("options");
	if ( tOptions && !ParseOptions ( tOptions, tQuery, sError ) )
		return false;

	bProfile = false;
	if ( !tRoot.FetchBoolItem ( bProfile, "profile", sError, true ) )
		return false;

	// expression columns go first to select list
	JsonObj_c tScriptFields = tRoot.GetItem ( "script_fields" );
	if ( tScriptFields && !ParseScriptFields ( tScriptFields, tQuery, sError ) )
		return false;

	// a synonym to "script_fields"
	JsonObj_c tExpressions = tRoot.GetItem ( "expressions" );
	if ( tExpressions && !ParseExpressions ( tExpressions, tQuery, sError ) )
		return false;

	JsonObj_c tSnip = tRoot.GetObjItem ( "highlight", sError, true );
	if ( tSnip )
	{
		if ( !ParseSnippet ( tSnip, tQuery, sError ) )
			return false;
	}
	else if ( !sError.IsEmpty() )
		return false;

	JsonObj_c tSort = tRoot.GetItem("sort");
	if ( tSort && !( tSort.IsArray() || tSort.IsObj() ) )
	{
		sError = "\"sort\" property value should be an array or an object";
		return false;
	}

	if ( tSort )
	{
		bool bGotWeight = false;
		if ( !ParseSort ( tSort, tQuery, bGotWeight, sError, sWarning ) )
			return false;

		JsonObj_c tTrackScore = tRoot.GetBoolItem ( "track_scores", sError, true );
		if ( !sError.IsEmpty() )
			return false;

		bool bTrackScore = tTrackScore && tTrackScore.BoolVal();
		if ( !bGotWeight && !bTrackScore )
			tQuery.m_eRanker = SPH_RANK_NONE;
	}

	// source \ select filter
	JsonObj_c tSelect = tRoot.GetItem("_source");
	bool bParsedSelect = ( !tSelect || ParseSelect ( tSelect, tQuery, sError ) );
	if ( !bParsedSelect )
		return false;

	// docvalue_fields
	JsonObj_c tDocFields = tRoot.GetItem ( "docvalue_fields" );
	if ( tDocFields && !ParseDocFields ( tDocFields, tQuery, sError ) )
		return false;

	// aggs
	JsonObj_c tAggs = tRoot.GetItem ( "aggs" );
	if ( tAggs && !ParseAggregates ( tAggs, tQuery, sError ) )
		return false;

	return true;
}


bool ParseJsonInsert ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, DocID_t & tDocId, bool bReplace, bool bCompat, CSphString & sError )
{
	if ( !ParseIndexId ( tRoot, tStmt, tDocId, sError ) )
		return false;

	if ( !ParseCluster ( tRoot, tStmt, sError ) )
		return false;

	tStmt.m_dInsertSchema.Add ( sphGetDocidName() );
	SqlInsert_t & tId = tStmt.m_dInsertValues.Add();
	tId.m_iType = SqlInsert_t::CONST_INT;
	tId.m_iVal = tDocId;

	// "doc" is optional
	JsonObj_c tSource = tRoot.GetItem("doc");

	return ParseJsonInsertSource ( tSource, tStmt, bReplace, bCompat, sError );
}

bool ParseJsonInsertSource ( const JsonObj_c & tSource, SqlStmt_t & tStmt, bool bReplace, bool bCompat, CSphString & sError )
{
	tStmt.m_eStmt = bReplace ? STMT_REPLACE : STMT_INSERT;

	const char * sColName = nullptr;
	StringBuilder_c tTmpName;

	if ( tSource )
	{
		for ( const auto & tItem : tSource )
		{
			sColName = tItem.Name();

			// replace internal of elastic and does not supported fields with underscore
			// any '@name' fields
			// 'ecs' or 'ecs.version' fields
			if ( bCompat )
			{
				if ( sColName[0]=='@' )
				{
					tTmpName.Clear();
					tTmpName.Sprintf ( "_%s", sColName + 1 );
					sColName = tTmpName.cstr();
				}
			}

			tStmt.m_dInsertSchema.Add ( sColName );
			tStmt.m_dInsertSchema.Last().ToLower();

			SqlInsert_t & tNewValue = tStmt.m_dInsertValues.Add();
			if ( tItem.IsStr() )
			{
				tNewValue.m_iType = SqlInsert_t::QUOTED_STRING;
				tNewValue.m_sVal = tItem.StrVal();
			} else if ( tItem.IsDbl() )
			{
				tNewValue.m_iType = SqlInsert_t::CONST_FLOAT;
				tNewValue.m_fVal = tItem.FltVal();
			} else if ( tItem.IsInt() || tItem.IsBool() )
			{
				tNewValue.m_iType = SqlInsert_t::CONST_INT;
				tNewValue.m_iVal = tItem.IntVal();
			} else if ( tItem.IsArray() && !bCompat )
			{
				tNewValue.m_iType = SqlInsert_t::CONST_MVA;
				tNewValue.m_pVals = new RefcountedVector_c<SphAttr_t>;

				for ( const auto & tArrayItem : tItem )
				{
					if ( !tArrayItem.IsInt() )
					{
						sError = "MVA elements should be integers";
						return false;
					}

					tNewValue.m_pVals->Add ( tArrayItem.IntVal() );
				}
			} else if ( tItem.IsObj() || ( bCompat && tItem.IsArray() ) )
			{
				tNewValue.m_iType = SqlInsert_t::QUOTED_STRING;
				tNewValue.m_sVal = tItem.AsString();
			} else
			{
				sError = "unsupported value type";
				return false;
			}
		}
	}

	if ( !tStmt.CheckInsertIntegrity() )
	{
		sError = "wrong number of values";
		return false;
	}

	return true;
}


bool sphParseJsonInsert ( const char * szInsert, SqlStmt_t & tStmt, DocID_t & tDocId, bool bReplace, bool bCompat, CSphString & sError )
{
	JsonObj_c tRoot ( szInsert );
	return ParseJsonInsert ( tRoot, tStmt, tDocId, bReplace, bCompat, sError );
}


static bool ParseUpdateDeleteQueries ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError )
{	
	tStmt.m_tQuery.m_sSelect = "id";
	if ( !ParseIndex ( tRoot, tStmt, sError ) )
		return false;

	if ( !ParseCluster ( tRoot, tStmt, sError ) )
		return false;

	JsonObj_c tId = tRoot.GetIntItem ( "id", sError );
	if ( tId )
	{
		CSphFilterSettings & tFilter = tStmt.m_tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_VALUES;
		tFilter.m_dValues.Add ( tId.IntVal() );
		tFilter.m_sAttrName = "id";

		tDocId = tId.IntVal();
	}

	// "query" is optional
	JsonObj_c tQuery = tRoot.GetItem("query");
	if ( tQuery && tId )
	{
		sError = R"(both "id" and "query" specified)";
		return false;
	}

	CSphString sWarning; // fixme: add to results
	return ParseJsonQueryFilters ( tQuery, tStmt.m_tQuery, sError, sWarning );
}


static bool ParseJsonUpdate ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError )
{
	CSphAttrUpdate & tUpd = tStmt.AttrUpdate();

	tStmt.m_eStmt = STMT_UPDATE;

	if ( !ParseUpdateDeleteQueries ( tRoot, tStmt, tDocId, sError ) )
		return false;

	JsonObj_c tSource = tRoot.GetObjItem ( "doc", sError );
	if ( !tSource )
		return false;

	CSphVector<int64_t> dMVA;

	for ( const auto & tItem : tSource )
	{
		bool bFloat = tItem.IsNum();
		bool bInt = tItem.IsInt();
		bool bBool = tItem.IsBool();
		bool bString = tItem.IsStr();
		bool bArray = tItem.IsArray();
		bool bObject = tItem.IsObj();

		if ( !bFloat && !bInt && !bBool && !bString && !bArray && !bObject )
		{
			sError = "unsupported value type";
			return false;
		}

		CSphString sAttr = tItem.Name();
		TypedAttribute_t & tTypedAttr = tUpd.m_dAttributes.Add();
		tTypedAttr.m_sName = sAttr.ToLower();

		if ( bInt || bBool )
		{
			int64_t iValue = tItem.IntVal();

			tUpd.m_dPool.Add ( (DWORD)iValue );
			auto uHi = (DWORD)( iValue>>32 );

			if ( uHi )
			{
				tUpd.m_dPool.Add ( uHi );
				tTypedAttr.m_eType = SPH_ATTR_BIGINT;
			} else
				tTypedAttr.m_eType = SPH_ATTR_INTEGER;
		}
		else if ( bFloat )
		{
			auto fValue = tItem.FltVal();
			tUpd.m_dPool.Add ( sphF2DW ( fValue ) );
			tTypedAttr.m_eType = SPH_ATTR_FLOAT;
		}
		else if ( bString || bObject )
		{
			CSphString sEncoded;
			const char * szValue = tItem.SzVal();
			if ( bObject )
			{
				sEncoded = tItem.AsString();
				szValue = sEncoded.cstr();
			}

			auto iLength = (int) strlen ( szValue );
			tUpd.m_dPool.Add ( tUpd.m_dBlobs.GetLength() );
			tUpd.m_dPool.Add ( iLength );

			if ( iLength )
			{
				BYTE * pBlob = tUpd.m_dBlobs.AddN ( iLength+2 );	// a couple of extra \0 for json parser to be happy
				memcpy ( pBlob, szValue, iLength );
				pBlob[iLength] = 0;
				pBlob[iLength+1] = 0;
			}

			tTypedAttr.m_eType = SPH_ATTR_STRING;
		} else if ( bArray )
		{
			dMVA.Resize ( 0 );
			for ( const auto & tArrayItem : tItem )
			{
				if ( !tArrayItem.IsInt() )
				{
					sError = "MVA elements should be integers";
					return false;
				}

				dMVA.Add ( tArrayItem.IntVal() );
			}
			dMVA.Uniq();

			tUpd.m_dPool.Add ( dMVA.GetLength()*2 ); // as 64 bit stored into DWORD vector
			tTypedAttr.m_eType = SPH_ATTR_UINT32SET;

			for ( int64_t uVal : dMVA )
			{
				if ( uVal>UINT_MAX )
					tTypedAttr.m_eType = SPH_ATTR_INT64SET;
				*(( int64_t* ) tUpd.m_dPool.AddN ( 2 )) = uVal;
			}
		}
	}

	return true;
}


bool sphParseJsonUpdate ( Str_t sUpdate, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError )
{
	JsonObj_c tRoot ( sUpdate );
	return ParseJsonUpdate ( tRoot, tStmt, tDocId, sError );
}


static bool ParseJsonDelete ( const JsonObj_c & tRoot, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError )
{
	tStmt.m_eStmt = STMT_DELETE;
	return ParseUpdateDeleteQueries ( tRoot, tStmt, tDocId, sError );
}


bool sphParseJsonDelete ( Str_t sDelete, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError )
{
	JsonObj_c tRoot ( sDelete );
	return ParseJsonDelete ( tRoot, tStmt, tDocId, sError );
}


bool sphParseJsonStatement ( const char * szStmt, SqlStmt_t & tStmt, CSphString & sStmt, CSphString & sQuery, DocID_t & tDocId, CSphString & sError )
{
	JsonObj_c tRoot ( szStmt );
	if ( !tRoot )
	{
		sError.SetSprintf ( "unable to parse: %s", tRoot.GetErrorPtr() );
		return false;
	}

	JsonObj_c tJsonStmt = tRoot[0];
	if ( !tJsonStmt )
	{
		sError = "no statement found";
		return false;
	}

	sStmt = tJsonStmt.Name();

	if ( !tJsonStmt.IsObj() )
	{
		sError.SetSprintf ( "statement %s should be an object", sStmt.cstr() );
		return false;
	}

	if ( sStmt=="index" || sStmt=="replace" )
	{
		if ( !ParseJsonInsert ( tJsonStmt, tStmt, tDocId, true, false, sError ) )
			return false;
	}  else if ( sStmt=="create" || sStmt=="insert" )
	{
		if ( !ParseJsonInsert ( tJsonStmt, tStmt, tDocId, false, false, sError ) )
			return false;
	} else if ( sStmt=="update" )
	{
		if ( !ParseJsonUpdate ( tJsonStmt, tStmt, tDocId, sError ) )
			return false;
	} else if ( sStmt=="delete" )
	{
		if ( !ParseJsonDelete ( tJsonStmt, tStmt, tDocId, sError ) )
			return false;
	} else
	{
		sError.SetSprintf ( "unknown bulk operation: %s", sStmt.cstr() );
		return false;
	}

	sQuery = tJsonStmt.AsString();

	return true;
}


//////////////////////////////////////////////////////////////////////////
static void PackedShortMVA2Json ( StringBuilder_c & tOut, const BYTE * pMVA )
{
	auto dMVA = sphUnpackPtrAttr ( pMVA );
	auto nValues = dMVA.second / sizeof ( DWORD );
	auto pValues = ( const DWORD * ) dMVA.first;
	for ( int i = 0; i<(int) nValues; ++i )
		tOut.NtoA(pValues[i]);
}

static void PackedWideMVA2Json ( StringBuilder_c & tOut, const BYTE * pMVA )
{
	auto dMVA = sphUnpackPtrAttr ( pMVA );
	auto nValues = dMVA.second / sizeof ( int64_t );
	auto pValues = ( const int64_t * ) dMVA.first;
	for ( int i = 0; i<(int) nValues; ++i )
		tOut.NtoA(pValues[i]);
}

static void JsonObjAddAttr ( JsonEscapedBuilder & tOut, ESphAttr eAttrType, const CSphMatch & tMatch, const CSphAttrLocator & tLoc )
{
	switch ( eAttrType )
	{
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_TIMESTAMP:
	case SPH_ATTR_TOKENCOUNT:
	case SPH_ATTR_BIGINT:
		tOut.NtoA ( tMatch.GetAttr(tLoc) );
		break;

	case SPH_ATTR_FLOAT:
		tOut.FtoA ( tMatch.GetAttrFloat(tLoc) );
		break;

	case SPH_ATTR_DOUBLE:
		tOut.DtoA ( tMatch.GetAttrDouble(tLoc) );
		break;

	case SPH_ATTR_BOOL:
		tOut << ( tMatch.GetAttr ( tLoc ) ? "true" : "false" );
		break;

	case SPH_ATTR_UINT32SET_PTR:
	case SPH_ATTR_INT64SET_PTR:
	{
		auto _ = tOut.Array ();
		const auto * pMVA = ( const BYTE * ) tMatch.GetAttr ( tLoc );
		if ( eAttrType==SPH_ATTR_UINT32SET_PTR )
			PackedShortMVA2Json ( tOut, pMVA );
		else
			PackedWideMVA2Json ( tOut, pMVA );
	}
	break;

	case SPH_ATTR_STRINGPTR:
	{
		const auto * pString = ( const BYTE * ) tMatch.GetAttr ( tLoc );
		auto dString = sphUnpackPtrAttr ( pString );

		// special process for legacy typed strings
		if ( dString.second>1 && dString.first[dString.second-2]=='\0')
		{
			auto uSubtype = dString.first[dString.second-1];
			dString.second -= 2;
			switch ( uSubtype)
			{
				case 1: // ql
				{
					ScopedComma_c sBrackets ( tOut, nullptr, R"({"ql":)", "}" );
					tOut.AppendEscapedWithComma (( const char* ) dString.first, dString.second);
					break;
				}
				case 0: // json
					tOut << ( const char* ) dString.first;
					break;

				default:
					tOut.Sprintf ("\"internal error! wrong subtype of stringptr %d\"", uSubtype );
			}
			break;
		}
		tOut.AppendEscapedWithComma ( ( const char * ) dString.first, dString.second );
	}
	break;

	case SPH_ATTR_JSON_PTR:
	{
		const auto * pJSON = ( const BYTE * ) tMatch.GetAttr ( tLoc );
		auto dJson = sphUnpackPtrAttr ( pJSON );

		// no object at all? return NULL
		if ( IsEmpty ( dJson ) )
			tOut << "null";
		else
			sphJsonFormat ( tOut, dJson.first );
	}
	break;

	case SPH_ATTR_FACTORS:
	case SPH_ATTR_FACTORS_JSON:
	{
		const auto * pFactors = ( const BYTE * ) tMatch.GetAttr ( tLoc );
		auto dFactors = sphUnpackPtrAttr ( pFactors );
		if ( IsEmpty ( dFactors ))
			tOut << "null";
		else
			sphFormatFactors ( tOut, (const unsigned int *) dFactors.first, true );
	}
	break;

	case SPH_ATTR_JSON_FIELD_PTR:
	{
		const auto * pField = ( const BYTE * ) tMatch.GetAttr ( tLoc );
		auto dField = sphUnpackPtrAttr ( pField );
		if ( IsEmpty ( dField ))
		{
			tOut << "null";
			break;
		}

		auto eJson = ESphJsonType ( *dField.first++ );
		if ( eJson==JSON_NULL )
			tOut << "null";
		else
			sphJsonFieldFormat ( tOut, dField.first, eJson, true );
	}
	break;

	default:
		assert ( 0 && "Unknown attribute" );
		break;
	}
}


static void JsonObjAddAttr ( JsonEscapedBuilder & tOut, ESphAttr eAttrType, const char * szCol, const CSphMatch & tMatch, const CSphAttrLocator & tLoc )
{
	assert ( sphPlainAttrToPtrAttr ( eAttrType )==eAttrType );
	tOut.AppendName ( szCol );
	JsonObjAddAttr ( tOut, eAttrType, tMatch, tLoc );
}


static bool IsHighlightAttr ( const CSphString & sName )
{
	return sName.Begins ( g_szHighlight );
}


static bool NeedToSkipAttr ( const CSphString & sName, const CSphQuery & tQuery )
{
	const char * szName = sName.cstr();

	if ( szName[0]=='i' && szName[1]=='d' && szName[2]=='\0' ) return true;
	if ( sName.Begins ( g_szHighlight ) ) return true;
	if ( sName.Begins ( GetFilterAttrPrefix() ) ) return true;
	if ( sName.Begins ( g_szOrder ) ) return true;

	if ( !tQuery.m_dIncludeItems.GetLength() && !tQuery.m_dExcludeItems.GetLength () )
		return false;

	// empty include - shows all select list items
	// exclude with only "*" - skip all select list items
	bool bInclude = ( tQuery.m_dIncludeItems.GetLength()==0 );
	for ( const auto &iItem: tQuery.m_dIncludeItems )
	{
		if ( sphWildcardMatch ( szName, iItem.cstr() ) )
		{
			bInclude = true;
			break;
		}
	}
	if ( bInclude && tQuery.m_dExcludeItems.GetLength() )
	{
		for ( const auto& iItem: tQuery.m_dExcludeItems )
		{
			if ( sphWildcardMatch ( szName, iItem.cstr() ) )
			{
				bInclude = false;
				break;
			}
		}
	}

	return !bInclude;
}

namespace { // static
void EncodeHighlight ( const CSphMatch & tMatch, int iAttr, const ISphSchema & tSchema, JsonEscapedBuilder & tOut )
{
	const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);

	ScopedComma_c tHighlightComma ( tOut, ",", R"("highlight":{)", "}", false );
	auto dSnippet = sphUnpackPtrAttr ((const BYTE *) tMatch.GetAttr ( tCol.m_tLocator ));

	SnippetResult_t tRes = UnpackSnippetData ( dSnippet );

	for ( const auto & tField : tRes.m_dFields )
	{
		tOut.AppendName ( tField.m_sName.cstr() );
		ScopedComma_c tHighlight ( tOut, ",", "[", "]", false );

		// we might want to add passage separators to field text here
		for ( const auto & tPassage : tField.m_dPassages )
			tOut.AppendEscapedWithComma ( (const char *)tPassage.m_dText.Begin(), tPassage.m_dText.GetLength() );
	}
}

static void EncodeFields ( const StrVec_t & dFields, const AggrResult_t & tRes, const CSphMatch & tMatch, const ISphSchema & tSchema,
	bool bValArray, const char * sPrefix, const char * sEnd, JsonEscapedBuilder & tOut )
{
	JsonEscapedBuilder tDFVal;

	tOut.StartBlock ( ",", sPrefix, sEnd );
	for ( const CSphString & sDF : dFields )
	{
		const CSphColumnInfo * pCol = tSchema.GetAttr ( sDF.cstr() );
		if ( !pCol )
		{
			tOut += R"("Default")";
			continue;
		}

		// FIXME!!! add format support
		tDFVal.Clear();
		JsonObjAddAttr ( tDFVal, pCol->m_eAttrType, tMatch, pCol->m_tLocator );

		if ( bValArray )
			tOut.Sprintf ( "%s", tDFVal.cstr() );
		else
			tOut.Sprintf ( R"("%s":["%s"])", sDF.cstr(), tDFVal.cstr() );
	}
	tOut.FinishBlock ( false ); // close obj
}

static void EncodeAggr ( const JsonAggr_t & tAggr, const AggrResult_t & tRes, bool bCompat, JsonEscapedBuilder & tOut )
{

	const CSphColumnInfo * pKey = tRes.m_tSchema.GetAttr ( tAggr.m_sCol.cstr() );
	const CSphColumnInfo * pCount = tRes.m_tSchema.GetAttr ( "count(*)" );
	// might be null for empty result set

	CSphFixedVector<std::pair<const CSphColumnInfo *, const char *>> dNested ( tAggr.m_dNested.GetLength() );
	ARRAY_FOREACH ( i, tAggr.m_dNested )
	{
		dNested[i].second = tAggr.m_dNested[i].m_sBucketName.cstr();
		dNested[i].first = tRes.m_tSchema.GetAttr ( tAggr.m_dNested[i].GetAliasName().cstr() );
		assert ( dNested[i].first );
	}

	JsonEscapedBuilder tKeyStr;
	tKeyStr.Clear();
	tKeyStr.Appendf ( R"("%s":{"buckets":[)", tAggr.m_sBucketName.cstr() );
	tOut.StartBlock ( ",", tKeyStr.cstr(), "]}");

	// might be null for empty result set
	if ( pKey && pCount )
	{
		auto dMatches = tRes.m_dResults.First ().m_dMatches.Slice ( tRes.m_iOffset, tRes.m_iCount );
		for ( const CSphMatch & tMatch : dMatches )
		{
			ScopedComma_c sQueryComma ( tOut, ",", "{", "}" );

			JsonObjAddAttr ( tOut, pKey->m_eAttrType, "key", tMatch, pKey->m_tLocator );
		
			tKeyStr.Clear();
			JsonObjAddAttr ( tKeyStr, pKey->m_eAttrType, tMatch, pKey->m_tLocator );
			if ( bCompat )
			{
				if ( pKey->m_eAttrType==SPH_ATTR_STRINGPTR )
					tOut.Sprintf ( R"("key_as_string":%s)", tKeyStr.cstr() );
				else
					tOut.Sprintf ( R"("key_as_string":"%s")", tKeyStr.cstr() );
			}

			JsonObjAddAttr ( tOut, pCount->m_eAttrType, "doc_count", tMatch, pCount->m_tLocator );

			for ( const auto & tNested : dNested )
			{
				tKeyStr.Clear();
				tKeyStr.Appendf ( R"("%s":{"value":)",  tNested.second );
				tOut.StartBlock ( ",", tKeyStr.cstr(), "}");

				JsonObjAddAttr ( tOut, tNested.first->m_eAttrType, tMatch, tNested.first->m_tLocator );

				tOut.FinishBlock ( false ); // named bucket obj
			}
		}
	}

	tOut.FinishBlock ( false ); // named bucket obj
}

void JsonRenderAccessSpecs ( JsonEscapedBuilder & tRes, const bson::Bson_c & tBson, bool bWithZones )
{
	using namespace bson;
	{
		ScopedComma_c sFieldsArray ( tRes, ",", "\"fields\":[", "]" );
		Bson_c ( tBson.ChildByName ( SZ_FIELDS ) ).ForEach ( [&tRes] ( const NodeHandle_t & tNode ) {
			tRes.AppendEscapedWithComma ( String ( tNode ).cstr() );
		} );
	}
	int iPos = (int)Int ( tBson.ChildByName ( SZ_MAX_FIELD_POS ) );
	if ( iPos )
		tRes.Sprintf ( "\"max_field_pos\":%d", iPos );

	if ( !bWithZones )
		return;

	auto tZones = tBson.GetFirstOf ( { SZ_ZONES, SZ_ZONESPANS } );
	ScopedComma_c dZoneDelim ( tRes, ", ", ( tZones.first==1 ) ? "\"zonespans\":[" : "\"zones\":[", "]" );
	Bson_c ( tZones.second ).ForEach ( [&tRes] ( const NodeHandle_t & tNode ) {
		tRes << String ( tNode );
	} );
}

bool JsonRenderKeywordNode ( JsonEscapedBuilder & tRes, const bson::Bson_c& tBson )
{
	using namespace bson;
	auto tWord = tBson.ChildByName ( SZ_WORD );
	if ( IsNullNode ( tWord ) )
		return false;

	ScopedComma_c sRoot ( tRes.Object() );
	tRes << R"("type":"KEYWORD")";
	tRes << "\"word\":";
	tRes.AppendEscapedSkippingComma ( String ( tWord ).cstr () );
	tRes.Sprintf ( R"("querypos":%d)", Int ( tBson.ChildByName ( SZ_QUERYPOS ) ) );

	if ( Bool ( tBson.ChildByName ( SZ_EXCLUDED ) ) )
		tRes << R"("excluded":true)";
	if ( Bool ( tBson.ChildByName ( SZ_EXPANDED ) ) )
		tRes << R"("expanded":true)";
	if ( Bool ( tBson.ChildByName ( SZ_FIELD_START ) ) )
		tRes << R"("field_start":true)";
	if ( Bool ( tBson.ChildByName ( SZ_FIELD_END ) ) )
		tRes << R"("field_end":true)";
	if ( Bool ( tBson.ChildByName ( SZ_FIELD_END ) ) )
		tRes << R"("morphed":true)";
	auto tBoost = tBson.ChildByName ( SZ_BOOST );
	if ( !IsNullNode ( tBoost ) )
	{
		auto fBoost = Double ( tBoost );
		if ( fBoost!=1.0f ) // really comparing floats?
			tRes.Sprintf ( R"("boost":%f)", fBoost );
	}
	return true;
}

void FormatJsonPlanFromBson ( JsonEscapedBuilder& tOut, bson::NodeHandle_t dBson )
{
	using namespace bson;
	if ( dBson==nullnode )
		return;

	Bson_c tBson ( dBson );

	if ( JsonRenderKeywordNode ( tOut, tBson) )
		return;

	auto dRootBlock = tOut.ObjectBlock();

	tOut << "\"type\":";
	tOut.AppendEscapedSkippingComma ( String ( tBson.ChildByName ( SZ_TYPE ) ).cstr() );

	tOut << "\"description\":";
	tOut.AppendEscapedSkippingComma ( sph::RenderBsonPlanBrief ( dBson ).cstr () );

	Bson_c ( tBson.ChildByName ( SZ_OPTIONS ) ).ForEach ( [&tOut] ( CSphString&& sName, const NodeHandle_t & tNode ) {
		tOut.Sprintf ( R"("options":"%s=%d")", sName.cstr (), (int) Int ( tNode ) );
	} );

	JsonRenderAccessSpecs ( tOut, dBson, true );

	tOut.StartBlock ( ",", "\"children\":[", "]" );
	Bson_c ( tBson.ChildByName ( SZ_CHILDREN ) ).ForEach ( [&] ( const NodeHandle_t & tNode ) {
		FormatJsonPlanFromBson ( tOut, tNode );
	} );
	tOut.FinishBlocks ( dRootBlock );
}

} // static

static CSphString JsonEncodeResultError ( const CSphString & sError, const char * sErrorType=nullptr, int * pStatus=nullptr, const char * sIndex=nullptr )
{
	JsonEscapedBuilder tOut;
	CSphString sResult;

	tOut.StartBlock ( ",", "{", "}" );

	tOut.StartBlock ( ",", R"("error":{)", "}" );

	tOut.AppendName ( "type" );
	tOut.AppendEscaped ( ( sErrorType ? sErrorType : "Error" ), EscBld::eEscape );

	tOut.AppendName ( "reason" );
	tOut.AppendEscaped ( sError.cstr(), EscBld::eEscape );

	if ( sIndex )
	{
		tOut.AppendName ( "index" );
		tOut.AppendEscaped ( sIndex, EscBld::eEscape );
	}

	tOut.FinishBlock ( false );

	if ( pStatus )
	{
		tOut.AppendName ( "status" );
		tOut << *pStatus;
	}

	tOut.FinishBlock ( false );

	tOut.MoveTo ( sResult ); // since simple return tOut.cstr() will cause copy of string, then returning it.
	return sResult;
}

CSphString JsonEncodeResultError ( const CSphString & sError, const char * sErrorType, int iStatus )
{
	return JsonEncodeResultError ( sError, sErrorType, &iStatus );
}

CSphString JsonEncodeResultError ( const CSphString & sError, const char * sErrorType, int iStatus, const char * sIndex )
{
	return JsonEncodeResultError ( sError, sErrorType, &iStatus, sIndex );
}

CSphString sphEncodeResultJson ( const VecTraits_T<const AggrResult_t *> & dRes, const JsonQuery_c & tQuery, QueryProfile_c * pProfile, bool bCompat )
{
	assert ( dRes.GetLength()>=1 );
	assert ( dRes[0]!=nullptr );
	const AggrResult_t & tRes = *dRes[0];

	if ( !tRes.m_iSuccesses )
		return JsonEncodeResultError ( tRes.m_sError );

	JsonEscapedBuilder tOut;
	CSphString sResult;

	tOut.ObjectBlock();

	tOut.Sprintf (R"("took":%d,"timed_out":false)", tRes.m_iQueryTime);
	if ( !tRes.m_sWarning.IsEmpty() )
	{
		tOut.StartBlock ( nullptr, R"("warning":{"reason":)", "}" );
		tOut.AppendEscapedWithComma ( tRes.m_sWarning.cstr () );
		tOut.FinishBlock ( false );
	}

	if ( bCompat )
		tOut += R"("_shards":{ "total": 1, "successful": 1, "skipped": 0, "failed": 0 })";

	auto sHitMeta = tOut.StartBlock ( ",", R"("hits":{)", "}" );

	tOut.Sprintf ( R"("total":%d)", tRes.m_iTotalMatches );
	tOut.Sprintf ( R"("total_relation":%s)", tRes.m_bTotalMatchesApprox ? R"("gte")" : R"("eq")" );
	if ( bCompat )
		tOut += R"("max_score": null)";

	const ISphSchema & tSchema = tRes.m_tSchema;
	CSphVector<BYTE> dTmp;

	CSphBitvec tAttrsToSend;
	sphGetAttrsToSend ( tSchema, false, true, tAttrsToSend );

	int iHighlightAttr = -1;
	int nSchemaAttrs = tSchema.GetAttrsCount();
	CSphBitvec dSkipAttrs ( nSchemaAttrs );
	for ( int iAttr=0; iAttr<nSchemaAttrs; iAttr++ )
	{
		if ( !tAttrsToSend.BitGet(iAttr) )
			continue;

		const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
		const CSphString & sName = tCol.m_sName;

		if ( IsHighlightAttr ( sName ) )
			iHighlightAttr = iAttr;

		if ( NeedToSkipAttr ( sName, tQuery ) )
			dSkipAttrs.BitSet ( iAttr );

		if ( bCompat && tCol.m_eAttrType==SPH_ATTR_TOKENCOUNT )
			dSkipAttrs.BitSet ( iAttr );
	}

	tOut.StartBlock ( ",", R"("hits":[)", "]" );

	const CSphColumnInfo * pId = tSchema.GetAttr ( sphGetDocidName() );

	bool bCompatId = false;
	const CSphColumnInfo * pCompatRaw = nullptr;
	const CSphColumnInfo * pCompatVer = nullptr;
	if ( bCompat )
	{
		const CSphColumnInfo * pCompatId = tSchema.GetAttr ( "_id" );
		if ( pCompatId )
		{
			bCompatId = true;
			pId = pCompatId;
		}

		pCompatRaw = tSchema.GetAttr ( "_raw" );
		pCompatVer = tSchema.GetAttr ( "_version" );
	}

	StringBuilder_c tTmpName;
	bool bTag = tRes.m_bTagsAssigned;
	int iTag = ( bTag ? 0 : tRes.m_dResults.First().m_iTag );
	auto dMatches = tRes.m_dResults.First ().m_dMatches.Slice ( tRes.m_iOffset, tRes.m_iCount );
	for ( const auto& tMatch : dMatches )
	{
		ScopedComma_c sQueryComma ( tOut, ",", "{", "}" );

		// note, that originally there is string UID, so we just output number in quotes for docid here
		if ( bCompatId )
		{
			JsonObjAddAttr ( tOut, pId->m_eAttrType, "_id", tMatch, pId->m_tLocator );
			tOut.Sprintf ( R"("_score":%d)", tMatch.m_iWeight );
		} else if ( pId )
		{
			DocID_t tDocID = tMatch.GetAttr ( pId->m_tLocator );
			tOut.Sprintf ( R"("_id":"%l","_score":%d)", tDocID, tMatch.m_iWeight );
		}
		else
			tOut.Sprintf ( R"("_score":%d)", tMatch.m_iWeight );

		if ( bCompat )
		{
			tOut.Sprintf ( R"("_index":"%s")", tRes.m_dIndexNames[bTag ? tMatch.m_iTag : iTag].scstr() ); // FIXME!!! breaks for multiple indexes
			tOut += R"("_type": "doc")";
			if ( pCompatVer )
				JsonObjAddAttr ( tOut, pCompatVer->m_eAttrType, "_version", tMatch, pCompatVer->m_tLocator );
			else
				tOut += R"("_version": 1)";
		}

		tOut.StartBlock ( ",", "\"_source\":{", "}");

		if ( pCompatRaw )
		{
			JsonObjAddAttr ( tOut, pCompatRaw->m_eAttrType, "_raw", tMatch, pCompatRaw->m_tLocator );
		} else
		{
			for ( int iAttr=0; iAttr<nSchemaAttrs; iAttr++ )
			{
				if ( !tAttrsToSend.BitGet(iAttr) )
					continue;

				if ( dSkipAttrs.BitGet ( iAttr ) )
					continue;

				const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
				const char * sName = tCol.m_sName.cstr();

				if ( bCompat && sName[0]=='_' ) // replace all hits _name to @name
				{
					tTmpName.Clear();
					tTmpName.Sprintf ( "@%s", sName+1 );
					sName = tTmpName.cstr();
				}

				JsonObjAddAttr ( tOut, tCol.m_eAttrType, sName, tMatch, tCol.m_tLocator );
			}
		}

		tOut.FinishBlock ( false ); // _source obj

		if ( iHighlightAttr!=-1 )
			EncodeHighlight ( tMatch, iHighlightAttr, tSchema, tOut );

		if ( bCompat )
		{
			if ( tQuery.m_dDocFields.GetLength() )
				EncodeFields ( tQuery.m_dDocFields, tRes, tMatch, tSchema, false, R"("fields":{)", "}", tOut );
			if ( tQuery.m_dSortFields.GetLength() )
				EncodeFields ( tQuery.m_dSortFields, tRes, tMatch, tSchema, true, R"("sort":[)", "]", tOut );
		}
	}

	tOut.FinishBlocks ( sHitMeta, false ); // hits array, hits meta

	if ( dRes.GetLength()>1 )
	{
		assert ( dRes.GetLength()==tQuery.m_dAggs.GetLength()+1 );
		tOut.StartBlock ( ",", R"("aggregations":{)", "}");
		ARRAY_FOREACH ( i, tQuery.m_dAggs )
			EncodeAggr ( tQuery.m_dAggs[i], *dRes[i+1], bCompat, tOut );
		tOut.FinishBlock ( false ); // aggregations obj
	}
	if ( bCompat )
		tOut += R"("status": 200)";

	if ( pProfile )
	{
		JsonEscapedBuilder sPlan;
		FormatJsonPlanFromBson ( sPlan, bson::MakeHandle ( pProfile->m_dPlan ) );
		if ( sPlan.IsEmpty() )
			tOut << R"("profile":null)";
		else
			tOut.Sprintf ( R"("profile":{"query":%s})", sPlan.cstr () );
	}

	tOut.FinishBlocks (); tOut.MoveTo ( sResult ); return sResult;
}


JsonObj_c sphEncodeInsertResultJson ( const char * szIndex, bool bReplace, DocID_t tDocId )
{
	JsonObj_c tObj;

	tObj.AddStr ( "_index", szIndex );
	tObj.AddInt ( "_id", tDocId );
	tObj.AddBool ( "created", !bReplace );
	tObj.AddStr ( "result", bReplace ? "updated" : "created" );
	tObj.AddInt ( "status", bReplace ? 200 : 201 );

	return tObj;
}

JsonObj_c sphEncodeTxnResultJson ( const char* szIndex, DocID_t tDocId, int iInserts, int iDeletes, int iUpdates )
{
	JsonObj_c tObj;

	tObj.AddStr ( "_index", szIndex );
	tObj.AddInt ( "_id", tDocId );
	tObj.AddInt ( "created", iInserts );
	tObj.AddInt ( "deleted", iDeletes );
	tObj.AddInt ( "updated", iUpdates );
	bool bReplaced = (iInserts!=0 && iDeletes!=0);
	tObj.AddStr ( "result", bReplaced ? "updated" : "created" );
	tObj.AddInt ( "status", bReplaced ? 200 : 201 );

	return tObj;
}


JsonObj_c sphEncodeUpdateResultJson ( const char * szIndex, DocID_t tDocId, int iAffected )
{
	JsonObj_c tObj;

	tObj.AddStr ( "_index", szIndex );

	if ( !tDocId )
		tObj.AddInt ( "updated", iAffected );
	else
	{
		tObj.AddInt ( "_id", tDocId );
		tObj.AddStr ( "result", iAffected ? "updated" : "noop" );
	}

	return tObj;
}


JsonObj_c sphEncodeDeleteResultJson ( const char * szIndex, DocID_t tDocId, int iAffected )
{
	JsonObj_c tObj;

	tObj.AddStr ( "_index", szIndex );

	if ( !tDocId )
		tObj.AddInt ( "deleted", iAffected );
	else
	{
		tObj.AddInt ( "_id", tDocId );
		tObj.AddBool ( "found", !!iAffected );
		tObj.AddStr ( "result", iAffected ? "deleted" : "not found" );
	}

	return tObj;
}


JsonObj_c sphEncodeInsertErrorJson ( const char * szIndex, const char * szError )
{
	JsonObj_c tObj, tErr;

	tErr.AddStr ( "type", szError );
	tErr.AddStr ( "index", szIndex );

	tObj.AddItem ( "error", tErr );
	tObj.AddInt ( "status", 500 );

	return tObj;
}


bool sphGetResultStats ( const char * szResult, int & iAffected, int & iWarnings, bool bUpdate )
{
	JsonObj_c tJsonRoot ( szResult );
	if ( !tJsonRoot )
		return false;

	// no warnings in json results for now
	iWarnings = 0;

	if ( tJsonRoot.HasItem("error") )
	{
		iAffected = 0;
		return true;
	}

	// its either update or delete
	CSphString sError;
	JsonObj_c tAffected = tJsonRoot.GetIntItem ( bUpdate ? "updated" : "deleted", sError );
	if ( tAffected )
	{
		iAffected = (int)tAffected.IntVal();
		return true;
	}

	// it was probably a query with an "_id"
	JsonObj_c tId = tJsonRoot.GetIntItem ( "_id", sError );
	if ( tId )
	{
		iAffected = 1;
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
// Highlight

static void FormatSnippetOpts ( const CSphString & sQuery, const SnippetQuerySettings_t & tSnippetQuery, CSphQuery & tQuery )
{
	StringBuilder_c sItem;
	sItem << "HIGHLIGHT(";
	sItem << tSnippetQuery.AsString();
	sItem << ",";

	auto & hFieldHash = tSnippetQuery.m_hPerFieldLimits;
	if ( tSnippetQuery.m_hPerFieldLimits.GetLength() )
	{
		sItem.StartBlock ( ",", "'", "'" );

		for ( const auto& tField : hFieldHash )
			sItem << tField.first;

		sItem.FinishBlock(false);
	}
	else
		sItem << "''";

	if ( !sQuery.IsEmpty() )
		sItem.Appendf ( ",'%s'", sQuery.cstr() );

	sItem << ")";

	CSphQueryItem & tItem = tQuery.m_dItems.Add();
	tItem.m_sExpr = sItem.cstr ();
	tItem.m_sAlias.SetSprintf ( "%s", g_szHighlight );
}


static bool ParseFieldsArray ( const JsonObj_c & tFields, SnippetQuerySettings_t & tSettings, CSphString & sError )
{
	for ( const auto & tField : tFields )
	{
		if ( !tField.IsStr() )
		{
			sError.SetSprintf ( "\"%s\" field should be an string", tField.Name() );
			return false;
		}

		SnippetLimits_t tDefault;
		tSettings.m_hPerFieldLimits.Add( tDefault, tField.StrVal() );
	}

	return true;
}


static bool ParseSnippetLimitsElastic ( const JsonObj_c & tSnip, SnippetLimits_t & tLimits, CSphString & sError )
{
	if ( !tSnip.FetchIntItem ( tLimits.m_iLimit, "fragment_size", sError, true ) )					return false;
	if ( !tSnip.FetchIntItem ( tLimits.m_iLimitPassages, "number_of_fragments", sError, true ) )	return false;

	return true;
}


static bool ParseSnippetLimitsSphinx ( const JsonObj_c & tSnip, SnippetLimits_t & tLimits, CSphString & sError )
{
	if ( !tSnip.FetchIntItem ( tLimits.m_iLimit, "limit", sError, true ) )					return false;

	if ( !tSnip.FetchIntItem ( tLimits.m_iLimitPassages, "limit_passages", sError, true ) )	return false;
	if ( !tSnip.FetchIntItem ( tLimits.m_iLimitPassages, "limit_snippets", sError, true ) )	return false;

	if ( !tSnip.FetchIntItem ( tLimits.m_iLimitWords, "limit_words", sError, true ) )		return false;

	return true;
}


static bool ParseFieldsObject ( const JsonObj_c & tFields, SnippetQuerySettings_t & tSettings, CSphString & sError )
{
	for ( const auto & tField : tFields )
	{
		if ( !tField.IsObj() )
		{
			sError.SetSprintf ( "\"%s\" field should be an object", tField.Name() );
			return false;
		}

		SnippetLimits_t & tLimits = tSettings.m_hPerFieldLimits.AddUnique ( tField.Name() );

		if ( !ParseSnippetLimitsElastic ( tField, tLimits, sError ) )
			return false;

		if ( !ParseSnippetLimitsSphinx ( tField, tLimits, sError ) )
			return false;
	}

	return true;
}



static bool ParseSnippetFields ( const JsonObj_c & tSnip, SnippetQuerySettings_t & tSettings, CSphString & sError )
{
	JsonObj_c tFields = tSnip.GetItem("fields");
	if ( !tFields )
		return true;

	if ( tFields.IsArray() )
		return ParseFieldsArray ( tFields, tSettings, sError );

	if ( tFields.IsObj() )
		return ParseFieldsObject ( tFields, tSettings, sError );

	sError = R"("fields" property value should be an array or an object)";
	return false;
}


static bool FetchTags ( const char * sName, const JsonObj_c & tSnip, CSphString & sVal, CSphString & sError )
{
	JsonObj_c tTag = tSnip.GetItem ( sName );
	if ( !tTag )
		return true;

	if ( tTag.IsStr() )
	{
		sVal = tTag.StrVal();
		return true;
	}

	if ( tTag.IsArray() )
	{
		if ( tTag.Size() )
			sVal = tTag[0].StrVal();
		return true;
	}

	sError.SetSprintf ( R"("%s" property value should be an array or sting)", sName );
	return false;
}


static bool ParseSnippetOptsElastic ( const JsonObj_c & tSnip, CSphString & sQuery, SnippetQuerySettings_t & tQuery, CSphString & sError )
{
	JsonObj_c tEncoder = tSnip.GetStrItem ( "encoder", sError, true );
	if ( tEncoder )
	{
		if ( tEncoder.StrVal()=="html" )
			tQuery.m_sStripMode = "retain";
	}
	else if ( !sError.IsEmpty() )
		return false;

	JsonObj_c tHlQuery = tSnip.GetObjItem ( "highlight_query", sError, true );
	if ( tHlQuery )
		sQuery = tHlQuery.AsString();
	else if ( !sError.IsEmpty() )
		return false;

	if ( !FetchTags ( "pre_tags", tSnip, tQuery.m_sBeforeMatch, sError ) )		return false;
	if ( !FetchTags ( "post_tags", tSnip, tQuery.m_sAfterMatch, sError ) )		return false;

	JsonObj_c tNoMatchSize = tSnip.GetItem ( "no_match_size" );
	if ( tNoMatchSize )
	{
		int iNoMatch = 0;
		if ( !tSnip.FetchIntItem ( iNoMatch, "no_match_size", sError, true ) )
			return false;

		tQuery.m_bAllowEmpty = iNoMatch<1;
	}

	JsonObj_c tOrder = tSnip.GetStrItem ( "order", sError, true );
	if ( tOrder )
		tQuery.m_bWeightOrder = tOrder.StrVal()=="score";
	else if ( !sError.IsEmpty() )
		return false;

	if ( !ParseSnippetLimitsElastic ( tSnip, tQuery, sError ) )
		return false;

	return true;
}


static bool ParseSnippetOptsSphinx ( const JsonObj_c & tSnip, SnippetQuerySettings_t & tOpt, CSphString & sError )
{
	if ( !ParseSnippetLimitsSphinx ( tSnip, tOpt, sError ) )
		return false;

	if ( !tSnip.FetchStrItem ( tOpt.m_sBeforeMatch, "before_match", sError, true ) )		return false;
	if ( !tSnip.FetchStrItem ( tOpt.m_sAfterMatch, "after_match", sError, true ) )			return false;
	if ( !tSnip.FetchIntItem ( tOpt.m_iAround, "around", sError, true ) )					return false;
	if ( !tSnip.FetchBoolItem ( tOpt.m_bUseBoundaries, "use_boundaries", sError, true ) )	return false;
	if ( !tSnip.FetchBoolItem ( tOpt.m_bWeightOrder, "weight_order", sError, true ) )		return false;
	if ( !tSnip.FetchBoolItem ( tOpt.m_bForceAllWords, "force_all_words", sError, true ) )	return false;
	if ( !tSnip.FetchStrItem ( tOpt.m_sStripMode, "html_strip_mode", sError, true ) )		return false;
	if ( !tSnip.FetchBoolItem ( tOpt.m_bAllowEmpty, "allow_empty", sError, true ) )			return false;
	if ( !tSnip.FetchBoolItem ( tOpt.m_bEmitZones, "emit_zones", sError, true ) )			return false;

	if ( !tSnip.FetchBoolItem ( tOpt.m_bForcePassages, "force_passages", sError, true ) )	return false;
	if ( !tSnip.FetchBoolItem ( tOpt.m_bForcePassages, "force_snippets", sError, true ) )	return false;

	if ( !tSnip.FetchBoolItem ( tOpt.m_bPackFields, "pack_fields", sError, true ) )			return false;
	if ( !tSnip.FetchBoolItem ( tOpt.m_bLimitsPerField, "limits_per_field", sError, true ) )return false;

	JsonObj_c tBoundary = tSnip.GetStrItem ( "passage_boundary", "snippet_boundary", sError );
	if ( tBoundary )
		tOpt.m_ePassageSPZ = GetPassageBoundary ( tBoundary.StrVal() );
	else if ( !sError.IsEmpty() )
		return false;

	return true;
}


static bool ParseSnippet ( const JsonObj_c & tSnip, CSphQuery & tQuery, CSphString & sError )
{
	CSphString sQuery;
	SnippetQuerySettings_t tSettings;
	tSettings.m_bJsonQuery = true;
	tSettings.m_bPackFields = true;

	if ( !ParseSnippetFields ( tSnip, tSettings, sError ) )
		return false;

	// elastic-style options
	if ( !ParseSnippetOptsElastic ( tSnip, sQuery, tSettings, sError ) )
		return false;
	
	// sphinx-style options
	if ( !ParseSnippetOptsSphinx ( tSnip, tSettings, sError ) )
		return false;

	FormatSnippetOpts ( sQuery, tSettings, tQuery );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// Sort
struct SortField_t : public GeoDistInfo_c
{
	CSphString m_sName;
	CSphString m_sMode;
	bool m_bAsc {true};
};


static void FormatSortBy ( const CSphVector<SortField_t> & dSort, JsonQuery_c & tQuery, bool & bGotWeight )
{
	StringBuilder_c sSortBuf;
	Comma_c sComma ({", ",2});

	for ( const SortField_t &tItem : dSort )
	{
		const char * sSort = ( tItem.m_bAsc ? " asc" : " desc" );
		if ( tItem.IsGeoDist() )
		{
			// ORDER BY statement
			sSortBuf << sComma << g_szOrder << tItem.m_sName << sSort;

			// query item
			CSphQueryItem & tQueryItem = tQuery.m_dItems.Add();
			tQueryItem.m_sExpr = tItem.BuildExprString();
			tQueryItem.m_sAlias.SetSprintf ( "%s%s", g_szOrder, tItem.m_sName.cstr() );

			// select list
			StringBuilder_c sTmp;
			sTmp << tQuery.m_sSelect << ", " << tQueryItem.m_sExpr << " as " << tQueryItem.m_sAlias;
			sTmp.MoveTo ( tQuery.m_sSelect );
		} else if ( tItem.m_sMode.IsEmpty() )
		{
			// sort by attribute or weight
			sSortBuf << sComma << ( tItem.m_sName=="_score" ? "@weight" : tItem.m_sName ) << sSort;
			bGotWeight |= ( tItem.m_sName=="_score" );
		} else
		{
			// sort by MVA
			// ORDER BY statement
			sSortBuf << sComma << g_szOrder << tItem.m_sName << sSort;

			// query item
			StringBuilder_c sTmp;
			sTmp << ( tItem.m_sMode=="min" ? "least" : "greatest" ) << "(" << tItem.m_sName << ")";
			CSphQueryItem & tQueryItem = tQuery.m_dItems.Add();
			sTmp.MoveTo (tQueryItem.m_sExpr);
			tQueryItem.m_sAlias.SetSprintf ( "%s%s", g_szOrder, tItem.m_sName.cstr() );

			// select list
			sTmp << tQuery.m_sSelect << ", " << tQueryItem.m_sExpr << " as " << tQueryItem.m_sAlias;
			sTmp.MoveTo ( tQuery.m_sSelect );
		}

		tQuery.m_dSortFields.Add ( tItem.m_sName );
	}

	if ( !dSort.GetLength() )
	{
		sSortBuf += "@weight desc";
		bGotWeight = true;
	}

	tQuery.m_eSort = SPH_SORT_EXTENDED;
	sSortBuf.MoveTo ( tQuery.m_sSortBy );
}


static bool ParseSort ( const JsonObj_c & tSort, JsonQuery_c & tQuery, bool & bGotWeight, CSphString & sError, CSphString & sWarning )
{
	bGotWeight = false;

	// unsupported options
	if ( tSort.HasItem("_script") )
	{
		sError = "\"_script\" property not supported";
		return false;
	}

	CSphVector<SortField_t> dSort;
	dSort.Reserve ( tSort.Size() );

	for ( const auto & tItem : tSort )
	{
		CSphString sName = tItem.Name();

		bool bString = tItem.IsStr();
		bool bObj = tItem.IsObj();
		if ( !bString && !bObj )
		{
			sError.SetSprintf ( R"("sort" property "%s" should be a string or an object)", sName.scstr() );
			return false;
		}

		if ( bObj && tItem.Size()!=1 )
		{
			sError.SetSprintf ( R"("sort" property "%s" should be an object)", sName.scstr() );
			return false;
		}

		// [ "attr_name" ]
		if ( bString )
		{
			SortField_t & tSortField = dSort.Add();
			tSortField.m_sName = tItem.StrVal();
			// order defaults to desc when sorting on the _score, and defaults to asc when sorting on anything else
			tSortField.m_bAsc = ( tSortField.m_sName!="_score" );
			continue;
		}

		JsonObj_c tSortItem = tItem[0];
		if ( !tSortItem )
		{
			sError = R"(invalid "sort" property item)";
			return false;
		}

		bool bSortString = tSortItem.IsStr();
		bool bSortObj = tSortItem.IsObj();

		CSphString sSortName = tSortItem.Name();
		if ( ( !bSortString && !bSortObj ) || !tSortItem.Name() || ( bSortString && !tSortItem.SzVal() ) )
		{
			sError.SetSprintf ( R"("sort" property 0("%s") should be %s)", sSortName.scstr(), ( bSortObj ? "a string" : "an object" ) );
			return false;
		}

		// [ { "attr_name" : "sort_mode" } ]
		if ( bSortString )
		{
			CSphString sOrder = tSortItem.StrVal();
			if ( sOrder!="asc" && sOrder!="desc" )
			{
				sError.SetSprintf ( R"("sort" property "%s" order is invalid %s)", sSortName.scstr(), sOrder.cstr() );
				return false;
			}

			SortField_t & tAscItem = dSort.Add();
			tAscItem.m_sName = sSortName;
			tAscItem.m_bAsc = ( sOrder=="asc" );
			continue;
		}

		// [ { "attr_name" : { "order" : "sort_mode" } } ]
		SortField_t & tSortField = dSort.Add();
		tSortField.m_sName = sSortName;

		JsonObj_c tAttrItems = tSortItem.GetItem("order");
		if ( tAttrItems )
		{
			if ( !tAttrItems.IsStr() )
			{
				sError.SetSprintf ( R"("sort" property "%s" order is invalid)", tAttrItems.Name() );
				return false;
			}

			CSphString sOrder = tAttrItems.StrVal();
			tSortField.m_bAsc = ( sOrder=="asc" );
		}

		JsonObj_c tMode = tSortItem.GetItem("mode");
		if ( tMode )
		{
			if ( tAttrItems && !tMode.IsStr() )
			{
				sError.SetSprintf ( R"("mode" property "%s" order is invalid)", tAttrItems.Name() );
				return false;
			}

			CSphString sMode = tMode.StrVal();
			if ( sMode!="min" && sMode!="max" )
			{
				sError.SetSprintf ( R"("mode" supported are "min" and "max", got "%s", not supported)", sMode.cstr() );
				return false;
			}

			tSortField.m_sMode = sMode;
		}

		// geodist
		if ( tSortField.m_sName=="_geo_distance" )
		{
			if ( tMode )
			{
				sError = R"("mode" property not supported with "_geo_distance")";
				return false;
			}

			if ( tSortItem.HasItem("unit") )
			{
				sError = R"("unit" property not supported with "_geo_distance")";
				return false;
			}

			if ( !tSortField.Parse ( tSortItem, false, sError, sWarning ) )
				return false;
		}

		// FXIME!!! "unmapped_type" should be replaced with expression EXIST
		// unsupported options
		const char * dUnsupported[] = { "missing", "nested_path", "nested_filter"};
		for ( auto szOption : dUnsupported )
			if ( tSortItem.HasItem(szOption) )
			{
				sError.SetSprintf ( R"("%s" property not supported)", szOption );
				return false;
			}
	}

	FormatSortBy ( dSort, tQuery, bGotWeight );

	return true;
}


//////////////////////////////////////////////////////////////////////////
// _source / select list

static bool ParseStringArray ( const JsonObj_c & tArray, const char * szProp, StrVec_t & dItems, CSphString & sError )
{
	for ( const auto & tItem : tArray )
	{
		if ( !tItem.IsStr() )
		{
			sError.SetSprintf ( R"("%s" property should be a string)", szProp );
			return false;
		}

		dItems.Add ( tItem.StrVal() );
	}

	return true;
}


static bool ParseSelect ( const JsonObj_c & tSelect, CSphQuery & tQuery, CSphString & sError )
{
	bool bString = tSelect.IsStr();
	bool bArray = tSelect.IsArray();
	bool bObj = tSelect.IsObj();

	if ( !bString && !bArray && !bObj )
	{
		sError = R"("_source" property should be a string or an array or an object)";
		return false;
	}

	if ( bString )
	{
		tQuery.m_dIncludeItems.Add ( tSelect.StrVal() );
		if ( tQuery.m_dIncludeItems[0]=="*" || tQuery.m_dIncludeItems[0].IsEmpty() )
			tQuery.m_dIncludeItems.Reset();

		return true;
	}

	if ( bArray )
		return ParseStringArray ( tSelect, R"("_source")", tQuery.m_dIncludeItems, sError );

	assert ( bObj );

	// includes part of _source object
	JsonObj_c tInclude = tSelect.GetArrayItem ( "includes", sError, true );
	if ( tInclude )
	{
		if ( !ParseStringArray ( tInclude, R"("_source" "includes")", tQuery.m_dIncludeItems, sError ) )
			return false;

		if ( tQuery.m_dIncludeItems.GetLength()==1 && tQuery.m_dIncludeItems[0]=="*" )
			tQuery.m_dIncludeItems.Reset();
	} else if ( !sError.IsEmpty() )
		return false;

	// excludes part of _source object
	JsonObj_c tExclude = tSelect.GetArrayItem ( "excludes", sError, true );
	if ( tExclude )
	{
		if ( !ParseStringArray ( tExclude, R"("_source" "excludes")", tQuery.m_dExcludeItems, sError ) )
			return false;
	} else if ( !sError.IsEmpty() )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// script_fields / expressions

static bool ParseScriptFields ( const JsonObj_c & tExpr, CSphQuery & tQuery, CSphString & sError )
{
	if ( !tExpr )
		return true;

	if ( !tExpr.IsObj() )
	{
		sError = R"("script_fields" property should be an object)";
		return false;
	}

	StringBuilder_c sSelect;
	sSelect << tQuery.m_sSelect;

	for ( const auto & tAlias : tExpr )
	{
		if ( !tAlias.IsObj() )
		{
			sError = R"("script_fields" properties should be objects)";
			return false;
		}

		if ( CSphString ( tAlias.Name() ).IsEmpty() )
		{
			sError = R"("script_fields" empty property name)";
			return false;
		}

		JsonObj_c tAliasScript = tAlias.GetItem("script");
		if ( !tAliasScript )
		{
			sError = R"("script_fields" property should have "script" object)";
			return false;
		}

		CSphString sExpr;
		if ( !tAliasScript.FetchStrItem ( sExpr, "inline", sError ) )
			return false;

		const char * dUnsupported[] = { "lang", "params", "stored", "file" };
		for ( auto szOption : dUnsupported )
			if ( tAliasScript.HasItem(szOption) )
			{
				sError.SetSprintf ( R"("%s" property not supported in "script_fields")", szOption );
				return false;
			}

		// add to query
		CSphQueryItem & tQueryItem = tQuery.m_dItems.Add();
		tQueryItem.m_sExpr = sExpr;
		tQueryItem.m_sAlias = tAlias.Name();

		// add to select list
		sSelect.Appendf ( ", %s as %s", tQueryItem.m_sExpr.cstr(), tQueryItem.m_sAlias.cstr() );
	}

	sSelect.MoveTo ( tQuery.m_sSelect );
	return true;
}


static bool ParseExpressions ( const JsonObj_c & tExpr, CSphQuery & tQuery, CSphString & sError )
{
	if ( !tExpr )
		return true;

	if ( !tExpr.IsObj() )
	{
		sError = R"("expressions" property should be an object)";
		return false;
	}

	StringBuilder_c sSelect;
	sSelect << tQuery.m_sSelect;

	for ( const auto & tAlias : tExpr )
	{
		if ( !tAlias.IsStr() )
		{
			sError = R"("expressions" properties should be strings)";
			return false;
		}

		if ( CSphString ( tAlias.Name() ).IsEmpty() )
		{
			sError = R"("expressions" empty property name)";
			return false;
		}

		// add to query
		CSphQueryItem & tQueryItem = tQuery.m_dItems.Add();
		tQueryItem.m_sExpr = tAlias.StrVal();
		tQueryItem.m_sAlias = tAlias.Name();

		// add to select list
		sSelect.Appendf ( ", %s as %s", tQueryItem.m_sExpr.cstr(), tQueryItem.m_sAlias.cstr() );
	}

	sSelect.MoveTo ( tQuery.m_sSelect );
	return true;
}

static CSphString g_sCompatDateFormat ( "%Y-%m-%dT%H:%M:%S." ); // YYYY-mm-dd'T'HH:mm:ss.SSS'Z'

const CSphString & CompatDateFormat()
{
	return g_sCompatDateFormat;
}

//////////////////////////////////////////////////////////////////////////
// docvalue_fields

bool ParseDocFields ( const JsonObj_c & tDocFields, JsonQuery_c & tQuery, CSphString & sError )
{
	if ( !tDocFields || !tDocFields.IsArray() )
	{
		sError = R"("docvalue_fields" property should be an array or an object")";
		return false;
	}

	for ( const auto & tItem : tDocFields )
	{
		if ( !tItem.IsObj() )
		{
			sError = R"("docvalue_fields" property item should be an object)";
			return false;
		}

		CSphString sFieldName;
		if ( !tItem.FetchStrItem ( sFieldName, "field", sError, false ) )
			return false;

		if ( tQuery.m_dItems.GetFirst ( [&sFieldName] ( const CSphQueryItem & tVal ) { return ( tVal.m_sExpr=="*" || tVal.m_sExpr==sFieldName ); } )==-1 )
		{
			CSphQueryItem & tDFItem = tQuery.m_dItems.Add();
			tDFItem.m_sExpr = sFieldName;
			tDFItem.m_sAlias = sFieldName;
		}

		tQuery.m_dDocFields.Add ( sFieldName );
	}

	return true;
}

static ESphAggrFunc GetAggrFunc ( const JsonObj_c & tBucket, bool bCheckAggType )
{
	if ( StrEq ( tBucket.Name(), "avg" ) )
		return SPH_AGGR_AVG;
	if ( StrEq ( tBucket.Name(), "min" ) )
		return SPH_AGGR_MIN;
	if ( StrEq ( tBucket.Name(), "max" ) )
		return SPH_AGGR_MAX;
	if ( StrEq ( tBucket.Name(), "sum" ) )
		return SPH_AGGR_SUM;

	if ( bCheckAggType )
		sphWarning ( "unsupported aggregate type '%s'", tBucket.Name() );

	return SPH_AGGR_NONE;
}

static bool AddSubAggregate ( const JsonObj_c & tAggs, bool bRoot, CSphVector<JsonAggr_t> & dParentItems, CSphString & sError )
{
	for ( const auto & tJsonItem : tAggs )
	{
		if ( !tJsonItem.IsObj() )
		{
			sError = R"("aggs" property item should be an object)";
			return false;
		}

		JsonAggr_t tItem;
		tItem.m_sBucketName = tJsonItem.Name();

		JsonObj_c tBucket = tJsonItem.begin();

		if ( StrEq ( tBucket.Name(), "aggs" ) && tBucket!=tJsonItem.end() )
		{
			if ( !AddSubAggregate ( tBucket, false, tItem.m_dNested, sError ) )
				return false;
			++tBucket;
		}

		if ( tBucket==tJsonItem.end() )
		{
			sError.SetSprintf ( R"("aggs" bucket '%s' with only nested items)", tItem.m_sBucketName.cstr() );
			return false;
		}

		if ( !tBucket.IsObj() )
		{
			sError.SetSprintf ( R"("aggs" bucket '%s' should be an object)", tItem.m_sBucketName.cstr() );
			return false;
		}

		if ( !tBucket.FetchStrItem ( tItem.m_sCol, "field", sError, false ) )
			return false;

		//tBucket.FetchStrItem ( tItem.m_sExpr, "calendar_interval", sError, true );
		tBucket.FetchIntItem ( tItem.m_iSize, "size", sError, true );
		int iShardSize = 0;
		tBucket.FetchIntItem ( iShardSize, "shard_size", sError, true );
		tItem.m_iSize = Max ( tItem.m_iSize, iShardSize ); // FIXME!!! use (size * 1.5 + 10) for shard size
		tItem.m_eAggrFunc = GetAggrFunc ( tBucket, !bRoot );

		if ( tItem.m_eAggrFunc==SPH_AGGR_NONE && !bRoot )
		{
			sError.SetSprintf ( R"(bucket '%s' without aggregate items, item type is '%s')", tItem.m_sBucketName.cstr(), tBucket.Name() );
			return false;
		}

		dParentItems.Add ( tItem );
	}

	return true;
}

bool ParseAggregates ( const JsonObj_c & tAggs, JsonQuery_c & tQuery, CSphString & sError )
{
	if ( !tAggs || !tAggs.IsObj() )
	{
		sError = R"("aggs" property should be an object")";
		return false;
	}

	if ( !tAggs.Size() )
		return true;

	if ( !AddSubAggregate ( tAggs, true, tQuery.m_dAggs, sError ) )
		return false;

	return true;
}

CSphString JsonAggr_t::GetAliasName () const
{
	CSphString sName;
	sName.SetSprintf ( "%s_%s", m_sCol.cstr(), m_sBucketName.cstr() );

	return sName;
}

CSphString JsonAggr_t::GetExpr () const
{
	CSphString sName;
	switch ( m_eAggrFunc )
	{
	case SPH_AGGR_AVG: sName.SetSprintf ( "avg(%s)", m_sCol.cstr() ); return sName;
	case SPH_AGGR_MIN: sName.SetSprintf ( "min(%s)", m_sCol.cstr() ); return sName;
	case SPH_AGGR_MAX: sName.SetSprintf ( "max(%s)", m_sCol.cstr() ); return sName;
	case SPH_AGGR_SUM: sName.SetSprintf ( "sum(%s)", m_sCol.cstr() ); return sName;
	case SPH_AGGR_CAT: sName.SetSprintf ( "cat(%s)", m_sCol.cstr() ); return sName;

	case SPH_AGGR_NONE:
	default:
		return m_sCol;
	}
}
