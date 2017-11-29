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

#include "sphinxjsonquery.h"
#include "sphinxquery.h"
#include "sphinxsearch.h"
#include "json/cJSON.h"
#include "sphinxplugin.h"
#include "sphinxint.h"
#include "sphinxutils.h"
#include "searchdaemon.h"
#include "sphinxjson.h"

static const char * g_szAll = "_all";
static const char * g_szFilter = "_@filter_";
static const char g_sHighlight[] = "_@highlight_";
static const char g_sOrder[] = "_@order_";


class CJsonScopedPtr_c : public CSphScopedPtr<cJSON>
{
public:
	CJsonScopedPtr_c ( cJSON * pPtr  )
		: CSphScopedPtr<cJSON> ( pPtr )
	{}

	~CJsonScopedPtr_c()
	{
		cJSON_Delete(m_pPtr);
		m_pPtr = NULL;
	}
};


static bool	IsFilter ( const cJSON * pJson )
{
	if ( !pJson || !pJson->string )
		return false;

	if ( !strcmp ( pJson->string, "equals" ) )
		return true;

	if ( !strcmp ( pJson->string, "range" ) )
		return true;

	if ( !strcmp ( pJson->string, "geo_distance" ) )
		return true;

	return false;
}


//////////////////////////////////////////////////////////////////////////
// Misc cJSON helpers
cJSON * GetJSONPropertyString ( const cJSON * pNode, const char * szName, CSphString & sError )
{
	if ( !pNode )
		return nullptr;

	cJSON * pChild = cJSON_GetObjectItem ( pNode, szName );
	if ( !pChild )
	{
		sError.SetSprintf ( "\"%s\" property missing", szName );
		return nullptr;
	}

	if ( !cJSON_IsString ( pChild ) )
	{
		sError.SetSprintf ( "\"%s\" property value should be a string", szName );
		return nullptr;
	}

	return pChild;
}


cJSON * GetJSONPropertyInt ( const cJSON * pNode, const char * szName, CSphString & sError )
{
	if ( !pNode )
		return nullptr;

	cJSON * pChild = cJSON_GetObjectItem ( pNode, szName );
	if ( !pChild )
	{
		sError.SetSprintf ( "\"%s\" property missing", szName );
		return nullptr;
	}

	if ( !cJSON_IsInteger ( pChild ) )
	{
		sError.SetSprintf ( "\"%s\" property value should be an integer", szName );
		return nullptr;
	}

	return pChild;
}


cJSON * GetJSONPropertyObject ( const cJSON * pNode, const char * szName, CSphString & sError )
{
	if ( !pNode )
		return nullptr;

	cJSON * pChild = cJSON_GetObjectItem ( pNode, szName );
	if ( !pChild )
	{
		sError.SetSprintf ( "\"%s\" property missing", szName );
		return nullptr;
	}

	if ( !cJSON_IsObject ( pChild ) )
	{
		sError.SetSprintf ( "\"%s\" property value should be an object", szName );
		return nullptr;
	}

	return pChild;
}


//////////////////////////////////////////////////////////////////////////
class QueryTreeBuilder_c : public XQParseHelper_c
{
public:
	void			CollectKeywords ( const char * szStr, XQNode_t * pNode, const XQLimitSpec_t & tLimitSpec );

	bool			HandleFieldBlockStart ( const char * & /*pPtr*/ ) override { return true; }
	virtual bool	HandleSpecialFields ( const char * & pPtr, FieldMask_t & dFields ) override;
	virtual bool	NeedTrailingSeparator() override { return false; }

	XQNode_t *		CreateNode ( XQLimitSpec_t & tLimitSpec );

private:
	void			AddChildKeyword ( XQNode_t * pParent, const char * szKeyword, int iSkippedPosBeforeToken, const XQLimitSpec_t & tLimitSpec );
};


void QueryTreeBuilder_c::CollectKeywords ( const char * szStr, XQNode_t * pNode, const XQLimitSpec_t & tLimitSpec )
{
	m_pTokenizer->SetBuffer ( (const BYTE*)szStr, strlen ( szStr ) );

	for ( ;; )
	{
		int iSkippedPosBeforeToken = 0;
		if ( m_bWasBlended )
		{
			iSkippedPosBeforeToken = m_pTokenizer->SkipBlended();
			// just add all skipped blended parts except blended head (already added to atomPos)
			if ( iSkippedPosBeforeToken>1 )
				m_iAtomPos += iSkippedPosBeforeToken - 1;
		}

		const char * sToken = (const char *) m_pTokenizer->GetToken ();
		if ( !sToken )
		{
			AddChildKeyword ( pNode, NULL, iSkippedPosBeforeToken, tLimitSpec );
			break;
		}

		// now let's do some token post-processing
		m_bWasBlended = m_pTokenizer->TokenIsBlended();

		int iPrevDeltaPos = 0;
		if ( m_pPlugin && m_pPlugin->m_fnPushToken )
			sToken = m_pPlugin->m_fnPushToken ( m_pPluginData, (char*)sToken, &iPrevDeltaPos, m_pTokenizer->GetTokenStart(), m_pTokenizer->GetTokenEnd() - m_pTokenizer->GetTokenStart() );

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
			sToken = NULL;
			// stopwords with step=0 must not affect pos
			if ( m_bEmptyStopword )
				m_iAtomPos--;
		}

		if ( bMultiDest && !bMultiDestHead )
		{
			assert ( m_dMultiforms.GetLength() );
			m_dMultiforms.Last().m_iDestCount++;
			m_dDestForms.Add ( sToken );
		} else
			AddChildKeyword ( pNode, sToken, iSkippedPosBeforeToken, tLimitSpec );

		if ( bMultiDestHead )
		{
			MultiformNode_t & tMulti = m_dMultiforms.Add();
			tMulti.m_pNode = pNode;
			tMulti.m_iDestStart = m_dDestForms.GetLength();
			tMulti.m_iDestCount = 0;
		}
	}
}


bool QueryTreeBuilder_c::HandleSpecialFields ( const char * & pPtr, FieldMask_t & dFields )
{
	if ( *pPtr=='_' )
	{
		int iLen = strlen(g_szAll);
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
	XQNode_t * pNode = new XQNode_t(tLimitSpec);
	m_dSpawned.Add ( pNode );
	return pNode;
}


void QueryTreeBuilder_c::AddChildKeyword ( XQNode_t * pParent, const char * szKeyword, int iSkippedPosBeforeToken, const XQLimitSpec_t & tLimitSpec )
{
	XQKeyword_t tKeyword ( szKeyword, m_iAtomPos );
	tKeyword.m_iSkippedBefore = iSkippedPosBeforeToken;
	XQNode_t * pNode = new XQNode_t ( tLimitSpec );
	pNode->m_pParent = pParent;
	pNode->m_dWords.Add ( tKeyword );
	pParent->m_dChildren.Add ( pNode );
	m_dSpawned.Add ( pNode );
}

//////////////////////////////////////////////////////////////////////////

class QueryParserJson_c : public QueryParser_i
{
public:
	virtual bool	IsFullscan ( const CSphQuery & tQuery ) const;
	virtual bool	IsFullscan ( const XQQuery_t & tQuery ) const;
	virtual bool	ParseQuery ( XQQuery_t & tParsed, const char * sQuery, const CSphQuery * pQuery, const ISphTokenizer * pQueryTokenizer, const ISphTokenizer * pQueryTokenizerJson, const CSphSchema * pSchema, CSphDict * pDict, const CSphIndexSettings & tSettings ) const;

private:
	XQNode_t *		ConstructMatchNode ( XQNode_t * pParent, const cJSON * pJson, bool bPhrase, QueryTreeBuilder_c & tBuilder ) const;
	XQNode_t *		ConstructBoolNode ( XQNode_t * pParent, const cJSON * pJson, QueryTreeBuilder_c & tBuilder ) const;
	XQNode_t *		ConstructMatchAllNode ( XQNode_t * pParent, QueryTreeBuilder_c & tBuilder ) const;

	bool			ConstructBoolNodeItems ( const cJSON * pClause, CSphVector<XQNode_t *> & dItems, QueryTreeBuilder_c & tBuilder ) const;
	bool			ConstructNodeOrFilter ( const cJSON * pItem, CSphVector<XQNode_t *> & dNodes, QueryTreeBuilder_c & tBuilder ) const;

	XQNode_t *		ConstructNode ( XQNode_t * pParent, const cJSON * pJson, QueryTreeBuilder_c & tBuilder ) const;
};


bool QueryParserJson_c::IsFullscan ( const CSphQuery & tQuery ) const
{
	// fixme: add more checks here
	return tQuery.m_sQuery.IsEmpty();
}


bool QueryParserJson_c::IsFullscan ( const XQQuery_t & tQuery ) const
{
	return !tQuery.m_pRoot || ( !tQuery.m_pRoot->m_dChildren.GetLength() && !tQuery.m_pRoot->m_dWords.GetLength() );
}


bool QueryParserJson_c::ParseQuery ( XQQuery_t & tParsed, const char * szQuery, const CSphQuery * /*pQuery*/, const ISphTokenizer *, const ISphTokenizer * pQueryTokenizerJson, const CSphSchema * pSchema, CSphDict * pDict, const CSphIndexSettings & tSettings ) const
{
	CJsonScopedPtr_c pJsonRoot ( cJSON_Parse ( szQuery ) );
	assert ( pJsonRoot.Ptr() );

	// take only the first item of the query; ignore the rest
	int iNumIndexes = cJSON_GetArraySize ( pJsonRoot.Ptr() );
	if ( !iNumIndexes )
	{
		tParsed.m_sParseError = "\"query\" property is empty";
		return false;
	}

	CSphScopedPtr<ISphTokenizer> pMyTokenizer ( pQueryTokenizerJson->Clone ( SPH_CLONE_QUERY_LIGHTWEIGHT ) );

	CSphDict * pMyDict = pDict;
	CSphScopedPtr<CSphDict> tDictCloned ( NULL );
	if ( pDict->HasState() )
		tDictCloned = pMyDict = pDict->Clone();

	QueryTreeBuilder_c tBuilder;
	tBuilder.Setup ( pSchema, pMyTokenizer.Ptr(), pMyDict, &tParsed, tSettings );

	tParsed.m_pRoot = ConstructNode ( NULL, cJSON_GetArrayItem ( pJsonRoot.Ptr(), 0 ), tBuilder );
	if ( tBuilder.IsError() )
	{
		tBuilder.Cleanup();
		return false;
	}

	XQLimitSpec_t tLimitSpec;
	tParsed.m_pRoot = tBuilder.FixupTree ( tParsed.m_pRoot, tLimitSpec );
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


XQNode_t * QueryParserJson_c::ConstructMatchNode ( XQNode_t * pParent, const cJSON * pJson, bool bPhrase, QueryTreeBuilder_c & tBuilder ) const
{
	if ( !cJSON_IsObject ( pJson ) )
	{
		tBuilder.Error ( "\"match\" value should be an object" );
		return nullptr;
	}

	if ( cJSON_GetArraySize(pJson)!=1 )
	{
		tBuilder.Error ( "ill-formed \"match\" property" );
		return nullptr;
	}

	cJSON * pFields = cJSON_GetArrayItem ( pJson, 0 );
	tBuilder.SetString ( pFields->string );

	XQLimitSpec_t tLimitSpec;
	const char * szQuery = NULL;
	XQOperator_e eNodeOp = bPhrase ? SPH_QUERY_PHRASE : SPH_QUERY_OR;
	bool bIgnore = false;

	if ( !tBuilder.ParseFields ( tLimitSpec.m_dFieldMask, tLimitSpec.m_iFieldMaxPos, bIgnore ) )
		return nullptr;

	if ( bIgnore )
	{
		tBuilder.Warning ( "ignoring fields in \"%s\", using \"_all\"", pFields->string );
		tLimitSpec.Reset();
	}

	tLimitSpec.m_bFieldSpec = true;

	if ( cJSON_IsObject ( pFields ) )
	{
		// matching with flags
		cJSON * pQuery = cJSON_GetObjectItem ( pFields, "query" );
		if ( !pQuery )
		{
			tBuilder.Error ( "no search string specified" );
			return nullptr;
		}

		if ( !cJSON_IsString ( pQuery ) )
		{
			tBuilder.Error ( "query text should be a string" );
			return nullptr;
		}

		szQuery = pQuery->valuestring;

		if ( !bPhrase )
		{
			cJSON * pOperator = cJSON_GetObjectItem ( pFields, "operator" );
			if ( pOperator ) // "and", "or"
			{
				eNodeOp = StrToNodeOp ( pOperator->valuestring );
				if ( eNodeOp==SPH_QUERY_TOTAL )
				{
					tBuilder.Error ( "unknown operator: \"%s\"", pOperator->valuestring );
					return nullptr;
				}
			}
		}
	} else
	{
		// simple list of keywords
		if ( !cJSON_IsString ( pFields ) )
		{
			tBuilder.Warning ( "values of properties in \"match\" should be strings or objects" );
			return nullptr;
		}

		szQuery = pFields->valuestring;
	}

	assert ( szQuery );

	XQNode_t * pNewNode = tBuilder.CreateNode ( tLimitSpec );
	pNewNode->SetOp ( eNodeOp );
	pNewNode->m_pParent = pParent;

	tBuilder.CollectKeywords ( szQuery, pNewNode, tLimitSpec );

	return pNewNode;
}


bool QueryParserJson_c::ConstructNodeOrFilter ( const cJSON * pItem, CSphVector<XQNode_t *> & dNodes, QueryTreeBuilder_c & tBuilder ) const
{
	assert ( pItem );

	// we created filters before, no need to process them again
	if ( !IsFilter ( pItem ) )
	{
		XQNode_t * pNode = ConstructNode ( NULL, pItem, tBuilder );
		if ( !pNode )
			return false;

		dNodes.Add ( pNode );
	}

	return true;
}


bool QueryParserJson_c::ConstructBoolNodeItems ( const cJSON * pClause, CSphVector<XQNode_t *> & dItems, QueryTreeBuilder_c & tBuilder ) const
{
	assert ( pClause );

	if ( cJSON_IsArray ( pClause ) )
	{
		for ( int iClause = 0; iClause < cJSON_GetArraySize ( pClause ); iClause++ )
		{
			cJSON * pObject = cJSON_GetArrayItem ( pClause, iClause );
			if ( !cJSON_IsObject ( pObject ) )
			{
				tBuilder.Error ( "\"%s\" array value should be an object", pClause->string );
				return false;
			}

			cJSON * pItem = cJSON_GetArrayItem ( pObject, 0 );
			assert ( pItem );

			if ( !ConstructNodeOrFilter ( pItem, dItems, tBuilder ) )
				return false;
		}
	} else if ( cJSON_IsObject ( pClause ) )
	{
		cJSON * pItem = cJSON_GetArrayItem ( pClause, 0 );
		if ( !ConstructNodeOrFilter ( pItem, dItems, tBuilder ) )
			return false;
	} else
	{
		tBuilder.Error ( "\"%s\" value should be an object or an array", pClause->string );
		return false;
	}

	return true;
}


XQNode_t * QueryParserJson_c::ConstructBoolNode ( XQNode_t * pParent, const cJSON * pJson, QueryTreeBuilder_c & tBuilder ) const
{
	if ( !cJSON_IsObject ( pJson ) )
	{
		tBuilder.Error ( "\"bool\" value should be an object" );
		return nullptr;
	}

	CSphVector<XQNode_t *> dMust, dShould, dMustNot;

	for ( int i = 0; i < cJSON_GetArraySize(pJson); i++ )
	{
		cJSON * pClause = cJSON_GetArrayItem ( pJson, i );
		assert ( pClause );

		if ( !strcmp ( pClause->string, "must" ) )
		{
			if ( !ConstructBoolNodeItems ( pClause, dMust, tBuilder ) )
				return nullptr;
		} else if ( !strcmp ( pClause->string, "should" ) )
		{
			if ( !ConstructBoolNodeItems ( pClause, dShould, tBuilder ) )
				return nullptr;
		} else if ( !strcmp ( pClause->string, "must_not" ) )
		{
			if ( !ConstructBoolNodeItems ( pClause, dMustNot, tBuilder ) )
				return nullptr;
		} else
		{
			tBuilder.Error ( "unknown bool query type: \"%s\"", pClause->string );
			return nullptr;
		}
	}

	XQNode_t * pMustNode = NULL;
	XQNode_t * pShouldNode = NULL;
	XQNode_t * pMustNotNode = NULL;

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
	
	if ( !iTotalNodes )
		return nullptr;
	else if ( iTotalNodes==1 )
	{
		XQNode_t * pResultNode = NULL;
		if ( pMustNode )
			pResultNode = pMustNode;
		else if ( pShouldNode )
			pResultNode = pShouldNode;
		else
			pResultNode = pMustNotNode;

		assert ( pResultNode );

		pResultNode->m_pParent = pParent;
		return pResultNode;
	} else
	{
		XQNode_t * pResultNode = pMustNode ? pMustNode : pMustNotNode;
		assert ( pResultNode );
		
		// combine 'must' and 'must_not' with AND
		if ( pMustNode && pMustNotNode )
		{
			XQNode_t * pAndNode = tBuilder.CreateNode(tLimitSpec);
			pAndNode->SetOp(SPH_QUERY_AND);
			pAndNode->m_dChildren.Add ( pMustNode );
			pAndNode->m_dChildren.Add ( pMustNotNode );
			pAndNode->m_pParent = pParent;		// may be modified later

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
			pMaybeNode->m_pParent = pParent;

			pShouldNode->m_pParent = pMaybeNode;
			pResultNode->m_pParent = pMaybeNode;

			pResultNode = pMaybeNode;
		}

		return pResultNode;
	}

	return nullptr;
}


XQNode_t * QueryParserJson_c::ConstructMatchAllNode ( XQNode_t * pParent, QueryTreeBuilder_c & tBuilder ) const
{
	XQLimitSpec_t tLimitSpec;
	XQNode_t * pNewNode = tBuilder.CreateNode ( tLimitSpec );
	pNewNode->SetOp ( SPH_QUERY_NULL );
	pNewNode->m_pParent = pParent;
	return pNewNode;
}


XQNode_t * QueryParserJson_c::ConstructNode ( XQNode_t * pParent, const cJSON * pJson, QueryTreeBuilder_c & tBuilder ) const
{
	if ( !pJson || !pJson->string )
	{
		tBuilder.Error ( "empty json found" );
		return nullptr;
	}

	bool bMatch = !strcmp ( pJson->string, "match" );
	bool bPhrase = !strcmp ( pJson->string, "match_phrase" );
	if ( bMatch || bPhrase )
		return ConstructMatchNode ( pParent, pJson, bPhrase, tBuilder );

	if (!strcmp ( pJson->string, "match_all" ))
		return ConstructMatchAllNode ( pParent, tBuilder );

		

	if ( !strcmp ( pJson->string, "bool" ) )
		return ConstructBoolNode ( pParent, pJson, tBuilder );

	return nullptr;
}


//////////////////////////////////////////////////////////////////////////
struct LocationField_t
{
	float m_fLat;
	float m_fLon;
	LocationField_t ()
		: m_fLat ( 0.0f )
		, m_fLon ( 0.0f )
	{}
};

struct LocationSource_t
{
	CSphString m_sLat;
	CSphString m_sLon;
};

static bool ParseLocation ( const char * sName, cJSON * pLoc, LocationField_t * pField, LocationSource_t * pSource, CSphString & sError );

class GeoDistInfo_c
{
public:
	bool				Parse ( const cJSON * pRoot, bool bNeedDistance, CSphString & sError, CSphString & sWarning );
	CSphString			BuildExprString() const;
	bool				IsGeoDist() const { return m_bGeodist; }
	float				GetDistance() const { return m_fDistance; }

private:
	bool				m_bGeodist {false};
	bool				m_bGeodistAdaptive {true};
	float				m_fDistance {0.0f};

	LocationField_t		m_tLocAnchor;
	LocationSource_t	m_tLocSource;

	bool				ParseDistance ( cJSON * pDistance, CSphString & sError );
};


bool GeoDistInfo_c::Parse ( const cJSON * pRoot, bool bNeedDistance, CSphString & sError, CSphString & sWarning )
{
	assert ( pRoot );

	cJSON * pLocAnchor = cJSON_GetObjectItem ( pRoot, "location_anchor" );
	cJSON * pLocSource = cJSON_GetObjectItem ( pRoot, "location_source" );

	if ( !pLocSource || !pLocAnchor )
	{
		if ( !pLocSource && !pLocAnchor )
			sError = "\"location_anchor\" and \"location_source\" properties missed";
		else
			sError.SetSprintf ( "\"%s\" property missed", ( !pLocAnchor ? "location_anchor" : "location_source" ) );
		return false;
	}

	if ( !ParseLocation ( "location_anchor", pLocAnchor, &m_tLocAnchor, NULL, sError )
		|| !ParseLocation ( "location_source", pLocSource, NULL, &m_tLocSource, sError ) )
		return false;

	cJSON * pType = cJSON_GetObjectItem ( pRoot, "distance_type" );
	if ( pType )
	{
		if ( !cJSON_IsString ( pType ) )
		{
			sError = "\"distance_type\" property should be a string";
			return false;
		}

		CSphString sType = pType->valuestring;
		if ( sType!="adaptive" && sType!="haversine" )
		{
			sWarning.SetSprintf ( "\"distance_type\" property type is invalid: \"%s\", defaulting to \"adaptive\"", sType.cstr() );
			m_bGeodistAdaptive = true;
		} else
			m_bGeodistAdaptive = ( sType=="adaptive" );
	}

	cJSON * pDistance = cJSON_GetObjectItem ( pRoot, "distance" );
	if ( pDistance )
	{
		if ( !ParseDistance ( pDistance, sError ) )
			return false;
	} else if ( bNeedDistance )
	{
		sError = "\"distance\" not specified";
		return false;
	}

	m_bGeodist = true;

	return true;
}


bool GeoDistInfo_c::ParseDistance ( cJSON * pDistance, CSphString & sError )
{
	if ( cJSON_IsNumeric ( pDistance ) )
	{
		// no units specified, meters assumed
		m_fDistance = (float)pDistance->valuedouble;
		return true;
	}

	if ( !cJSON_IsString ( pDistance ) )
	{
		sError = "\"distance\" property should be a number or a string";
		return false;
	}

	const char * p = pDistance->valuestring;
	assert ( p );
	while ( *p && sphIsSpace(*p) )
		p++;

	const char * szNumber = p;
	while ( *p && ( *p=='.' || ( *p>='0' && *p<='9' ) ) )
		p++;

	CSphString sNumber;
	sNumber.SetBinary ( szNumber, p-szNumber );

	while ( *p && sphIsSpace(*p) )
		p++;

	const char * szUnit = p;
	while ( *p && sphIsAlpha(*p) )
		p++;

	CSphString sUnit;
	sUnit.SetBinary ( szUnit, p-szUnit );

	m_fDistance = (float)atof ( sNumber.cstr() );

	float fCoeff = 1.0f;	
	if ( !sphGeoDistanceUnit ( sUnit.cstr(), fCoeff ) )
	{
		sError.SetSprintf ( "unknown distance unit: %s", sUnit.cstr() );
		return false;
	}

	m_fDistance *= fCoeff;

	return true;
}


CSphString GeoDistInfo_c::BuildExprString() const
{
	CSphString sResult;
	sResult.SetSprintf ( "GEODIST(%f, %f, %s, %s, {in=deg, out=m, method=%s})", m_tLocAnchor.m_fLat, m_tLocAnchor.m_fLon, m_tLocSource.m_sLat.cstr(), m_tLocSource.m_sLon.cstr(), m_bGeodistAdaptive ? "adaptive" : "haversine" );
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
static void AddToSelectList ( CSphQuery & tQuery, const CSphVector<CSphQueryItem> & dItems, int iFirstItem = 0 )
{
	for ( int i = iFirstItem; i < dItems.GetLength(); i++ )
		tQuery.m_sSelect.SetSprintf ( "%s, %s as %s", tQuery.m_sSelect.cstr(), dItems[i].m_sExpr.cstr(), dItems[i].m_sAlias.cstr() );
}


static cJSON * GetFilterColumn ( const cJSON * pJson, CSphString & sError )
{
	if ( !cJSON_IsObject ( pJson ) )
	{
		sError = "filter should be an object";
		return nullptr;
	}

	if ( cJSON_GetArraySize ( pJson )!=1 )
	{
		sError = "\"equals\" filter should have only one element";
		return nullptr;
	}

	cJSON * pColumn = cJSON_GetArrayItem ( pJson, 0 );
	if ( !pColumn )
	{
		sError = "empty filter found";
		return nullptr;
	}

	return pColumn;
}


static bool ConstructEqualsFilter ( const cJSON * pJson, CSphVector<CSphFilterSettings> & dFilters, CSphString & sError )
{
	cJSON * pColumn = GetFilterColumn ( pJson, sError );
	if ( !pColumn )
		return false;

	if ( !cJSON_IsNumeric ( pColumn ) && !cJSON_IsString ( pColumn ) )
	{
		sError = "\"equals\" filter expects numeric or string values";
		return false;
	}

	CSphFilterSettings tFilter;
	tFilter.m_sAttrName = ( !strcasecmp ( pColumn->string, "id" ) ) ? "@id" : pColumn->string;
	sphColumnToLowercase ( const_cast<char *>( tFilter.m_sAttrName.cstr() ) );

	if ( cJSON_IsInteger( pColumn ) )
	{
		tFilter.m_eType = SPH_FILTER_VALUES;
		tFilter.m_dValues.Add ( pColumn->valueint );
	} else if ( cJSON_IsNumber ( pColumn ) )
	{
		tFilter.m_eType = SPH_FILTER_FLOATRANGE;
		tFilter.m_fMinValue = (float)pColumn->valuedouble;
		tFilter.m_fMaxValue = (float)pColumn->valuedouble;
		tFilter.m_bHasEqualMin = true;
		tFilter.m_bHasEqualMax = true;
		tFilter.m_bExclude = false;
	} else
	{
		tFilter.m_eType = SPH_FILTER_STRING;
		tFilter.m_dStrings.Add ( pColumn->valuestring );
		tFilter.m_bExclude = false;
	}

	dFilters.Add ( tFilter );

	return true;
}


static bool ConstructRangeFilter ( const cJSON * pJson, CSphVector<CSphFilterSettings> & dFilters, CSphString & sError )
{
	cJSON * pColumn = GetFilterColumn ( pJson, sError );
	if ( !pColumn )
		return false;

	CSphFilterSettings tNewFilter;
	tNewFilter.m_sAttrName = ( !strcasecmp ( pColumn->string, "id" ) ) ? "@id" : pColumn->string;
	sphColumnToLowercase ( const_cast<char *>( tNewFilter.m_sAttrName.cstr() ) );

	tNewFilter.m_bHasEqualMin = false;
	tNewFilter.m_bHasEqualMax = false;
	cJSON * pLess = cJSON_GetObjectItem ( pColumn, "lt" );
	if ( !pLess )
	{
		pLess = cJSON_GetObjectItem ( pColumn, "lte" );
		tNewFilter.m_bHasEqualMax = !!pLess;
	}

	cJSON * pGreater = cJSON_GetObjectItem ( pColumn, "gt" );
	if ( !pGreater )
	{
		pGreater = cJSON_GetObjectItem ( pColumn, "gte" );
		tNewFilter.m_bHasEqualMin = !!pGreater;
	}

	if ( !pLess && !pGreater )
	{
		sError = "empty filter found";
		return false;
	}

	if ( ( pLess && !cJSON_IsNumeric ( pLess ) ) || ( pGreater && !cJSON_IsNumeric ( pGreater ) ) )
	{
		sError = "range filter expects numeric values";
		return false;
	}

	bool bIntFilter = ( pLess && cJSON_IsInteger ( pLess ) ) || ( pGreater && cJSON_IsInteger ( pGreater ) );

	if ( pLess )
	{
		if ( bIntFilter )
			tNewFilter.m_iMaxValue = pLess->valueint;
		else
			tNewFilter.m_fMaxValue = (float)pLess->valuedouble;

		tNewFilter.m_bOpenLeft = !pGreater;
	}

	if ( pGreater )
	{
		if ( bIntFilter )
			tNewFilter.m_iMinValue = pGreater->valueint;
		else
			tNewFilter.m_fMinValue = (float)pGreater->valuedouble;

		tNewFilter.m_bOpenRight = !pLess;
	}

	tNewFilter.m_eType = bIntFilter ? SPH_FILTER_RANGE : SPH_FILTER_FLOATRANGE;

	// float filters don't support open ranges
	if ( !bIntFilter )
	{
		if ( tNewFilter.m_bOpenRight )
			tNewFilter.m_fMaxValue = FLT_MAX;

		if ( tNewFilter.m_bOpenLeft )
			tNewFilter.m_fMinValue = FLT_MIN;
	}

	dFilters.Add ( tNewFilter );

	return true;
}


static bool ConstructGeoFilter ( const cJSON * pJson, CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphQueryItem> & dQueryItems, int & iQueryItemId, CSphString & sError, CSphString & sWarning )
{
	GeoDistInfo_c tGeoDist;
	if ( !tGeoDist.Parse ( pJson, true, sError, sWarning ) )
		return false;

	CSphQueryItem & tQueryItem = dQueryItems.Add();
	tQueryItem.m_sExpr = tGeoDist.BuildExprString();
	tQueryItem.m_sAlias.SetSprintf ( "%s%d", g_szFilter, iQueryItemId++ );

	CSphFilterSettings & tFilter = dFilters.Add();
	tFilter.m_sAttrName = tQueryItem.m_sAlias;
	tFilter.m_bOpenLeft = true;
	tFilter.m_bHasEqualMax = true;
	tFilter.m_fMaxValue = tGeoDist.GetDistance();
	tFilter.m_eType = SPH_FILTER_FLOATRANGE;

	return true;
}


static bool ConstructFilter ( const cJSON * pJson, CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphQueryItem> & dQueryItems, int & iQueryItemId, CSphString & sError, CSphString & sWarning )
{
	if ( !IsFilter ( pJson ) )
		return true;

	if ( !strcmp ( pJson->string, "equals" ) )
		return ConstructEqualsFilter ( pJson, dFilters, sError );

	if ( !strcmp ( pJson->string, "range" ) )
		return ConstructRangeFilter ( pJson, dFilters, sError );

	if ( !strcmp ( pJson->string, "geo_distance" ) )
		return ConstructGeoFilter ( pJson, dFilters, dQueryItems, iQueryItemId, sError, sWarning );

	sError.SetSprintf ( "unknown filter type: %s", pJson->string );
	return false;
}


static bool ConstructBoolNodeFilters ( const cJSON * pClause, CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphQueryItem> & dQueryItems, int & iQueryItemId, CSphString & sError, CSphString & sWarning )
{
	if ( cJSON_IsArray ( pClause ) )
	{
		for ( int iClause = 0; iClause < cJSON_GetArraySize ( pClause ); iClause++ )
		{
			cJSON * pObject = cJSON_GetArrayItem ( pClause, iClause );
			if ( !cJSON_IsObject ( pObject ) )
			{
				sError.SetSprintf ( "\"%s\" array value should be an object", pClause->string );
				return false;
			}

			cJSON * pItem = cJSON_GetArrayItem ( pObject, 0 );
			assert ( pItem );

			if ( !ConstructFilter ( pItem, dFilters, dQueryItems, iQueryItemId, sError, sWarning ) )
				return false;
		}
	} else if ( cJSON_IsObject ( pClause ) )
	{
		cJSON * pItem = cJSON_GetArrayItem ( pClause, 0 );
		if ( !ConstructFilter ( pItem, dFilters, dQueryItems, iQueryItemId, sError, sWarning ) )
			return false;
	} else
	{
		sError.SetSprintf ( "\"%s\" value should be an object or an array", pClause->string );
		return false;
	}

	return true;
}


static bool ConstructBoolFilters ( cJSON * pBool, CSphQuery & tQuery, int & iQueryItemId, CSphString & sError, CSphString & sWarning )
{
	// non-recursive for now, maybe we should fix this later
	if ( !cJSON_IsObject ( pBool ) )
	{
		sError = "\"bool\" value should be an object";
		return false;
	}

	CSphVector<CSphFilterSettings> dMust, dShould, dMustNot;
	CSphVector<CSphQueryItem> dMustQI, dShouldQI, dMustNotQI;

	for ( int i = 0; i < cJSON_GetArraySize(pBool); i++ )
	{
		cJSON * pClause = cJSON_GetArrayItem ( pBool, i );
		assert ( pClause );

		if ( !strcmp ( pClause->string, "must" ) )
		{
			if ( !ConstructBoolNodeFilters ( pClause, dMust, dMustQI, iQueryItemId, sError, sWarning ) )
				return false;
		} else if ( !strcmp ( pClause->string, "should" ) )
		{
			if ( !ConstructBoolNodeFilters ( pClause, dShould, dShouldQI, iQueryItemId, sError, sWarning ) )
				return false;
		} else if ( !strcmp ( pClause->string, "must_not" ) )
		{
			if ( !ConstructBoolNodeFilters ( pClause, dMustNot, dMustNotQI, iQueryItemId, sError, sWarning ) )
				return false;
		} else
		{
			sError.SetSprintf ( "unknown bool query type: \"%s\"", pClause->string );
			return false;
		}
	}

	if ( dMustNot.GetLength() )
	{
		int iFilter = dMust.GetLength();
		dMust.Resize ( dMust.GetLength()+dMustNot.GetLength() );
		for ( auto & i : dMustNot )
		{
			i.m_bExclude = true;
			dMust[iFilter++] = i;
		}

		int iQueryItem = dMustQI.GetLength();
		dMustQI.Resize ( dMustQI.GetLength()+dMustNotQI.GetLength() );
		for ( auto & i : dMustNotQI )
			dMustQI[iQueryItem++] = i;
	}

	if ( dMust.GetLength() )
	{
		AddToSelectList ( tQuery, dMustQI );
		tQuery.m_dFilters.SwapData ( dMust );
		tQuery.m_dItems.SwapData ( dMustQI );
		return true;
	}

	if ( dShould.GetLength() )
	{
		AddToSelectList ( tQuery, dShouldQI );
		tQuery.m_dFilters.SwapData ( dShould );
		tQuery.m_dItems.SwapData ( dShouldQI );

		// need a filter tree
		FilterTreeItem_t & tTreeItem = tQuery.m_dFilterTree.Add();
		tTreeItem.m_iFilterItem = 0;
		int iRootNode = 0;

		ARRAY_FOREACH ( i, tQuery.m_dFilters )
		{
			int iNewFilterNodeId = tQuery.m_dFilterTree.GetLength();
			FilterTreeItem_t & tNewFilterNode = tQuery.m_dFilterTree.Add();
			tNewFilterNode.m_iFilterItem = i;

			int iNewOrNodeId = tQuery.m_dFilterTree.GetLength();
			FilterTreeItem_t & tNewOrNode = tQuery.m_dFilterTree.Add();
			tNewOrNode.m_bOr = true;
			tNewOrNode.m_iLeft = iRootNode;
			tNewOrNode.m_iRight = iNewFilterNodeId;

			iRootNode = iNewOrNodeId;
		}
	}

	return true;
}


static bool ConstructFilters ( const cJSON * pJson, CSphQuery & tQuery, CSphString & sError, CSphString & sWarning )
{
	if ( !pJson || !pJson->string )
		return false;

	if ( strcmp ( pJson->string, "query" ) )
	{
		sError.SetSprintf ( "'query' expected, got %s", pJson->string );
		return false;
	}

	int iQueryItemId = 0;

	cJSON * pBool = cJSON_GetObjectItem ( pJson, "bool" );
	if ( pBool )
		return ConstructBoolFilters ( pBool, tQuery, iQueryItemId, sError, sWarning );

	for ( int i = 0; i < cJSON_GetArraySize ( pJson ); i++ )
	{
		cJSON * pChild = cJSON_GetArrayItem ( pJson, i );
		assert ( pChild );

		if ( IsFilter ( pChild ) )
		{
			int iFirstNewItem = tQuery.m_dItems.GetLength();
			if ( !ConstructFilter ( pChild, tQuery.m_dFilters, tQuery.m_dItems, iQueryItemId, sError, sWarning ) )
				return false;

			AddToSelectList ( tQuery, tQuery.m_dItems, iFirstNewItem );

			// handle only the first filter in this case
			break;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

static bool ParseSnippet ( cJSON * pSnip, CSphQuery & tQuery, CSphString & sError );
static bool ParseSort ( cJSON * pSort, CSphQuery & tQuery, bool & bGotWeight, CSphString & sError, CSphString & sWarning );
static bool ParseSelect ( cJSON * pSelect, CSphQuery & tQuery, CSphString & sError );
static bool ParseExpr ( cJSON * pExpr, CSphQuery & tQuery, CSphString & sError );


static bool ParseIndex ( cJSON * pRoot, SqlStmt_t & tStmt, CSphString & sError )
{
	if ( !pRoot )
	{
		sError.SetSprintf ( "unable to parse: %s", cJSON_GetErrorPtr() );
		return false;
	}

	cJSON * pIndex = GetJSONPropertyString ( pRoot, "_index", sError );
	if ( !pIndex )
		return false;

	tStmt.m_sIndex = pIndex->valuestring;
	tStmt.m_tQuery.m_sIndexes = tStmt.m_sIndex;

	return true;
}


static bool ParseIndexId ( cJSON * pRoot, SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError )
{
	if ( !ParseIndex ( pRoot, tStmt, sError ) )
		return false;

	cJSON * pId = GetJSONPropertyInt ( pRoot, "_id", sError );
	if ( !pId )
		return false;

	tDocId = pId->valueint;

	return true;
}


QueryParser_i * sphCreateJsonQueryParser()
{
	return new QueryParserJson_c;
}


static bool ParseJsonQueryFilters ( cJSON * pQuery, CSphQuery & tQuery, CSphString & sError, CSphString & sWarning )
{
	if ( pQuery && !cJSON_IsObject ( pQuery ) )
	{	
		sError = "\"query\" property value should be an object";
		return false;
	}

	CSphQueryItem & tItem = tQuery.m_dItems.Add();
	tItem.m_sExpr = "*";
	tItem.m_sAlias = "*";
	tQuery.m_sSelect = "*";

	// we need to know if the query is fullscan before re-parsing it to build AST tree
	// so we need to do some preprocessing here
	bool bFullscan = !pQuery || ( cJSON_GetArraySize(pQuery)==1 && cJSON_HasObjectItem ( pQuery, "match_all" ) );

	if ( !bFullscan )
		tQuery.m_sQuery.Adopt ( cJSON_Print ( pQuery ) );

	// because of the way sphinxql parsing was implemented
	// we need to parse our query and extract filters now
	// and parse the rest of the query later
	if ( pQuery )
	{
		if ( !ConstructFilters ( pQuery, tQuery, sError, sWarning ) )
			return false;
	}

	return true;
}


bool sphParseJsonQuery ( const char * szQuery, CSphQuery & tQuery, bool & bProfile, bool & bAttrsHighlight, CSphString & sError, CSphString & sWarning )
{
	CJsonScopedPtr_c pRoot ( cJSON_Parse ( szQuery ) );
	if ( !pRoot.Ptr() )
	{
		sError.SetSprintf ( "unable to parse: %s", cJSON_GetErrorPtr() );
		return false;
	}

	tQuery.m_sRawQuery = szQuery;

	cJSON * pIndex = GetJSONPropertyString ( pRoot.Ptr(), "index", sError );
	if ( !pIndex )
		return false;

	tQuery.m_sIndexes = pIndex->valuestring;
	tQuery.m_sIndexes.ToLower();

	if ( tQuery.m_sIndexes==g_szAll )
		tQuery.m_sIndexes = "*";

	cJSON * pQuery = cJSON_GetObjectItem ( pRoot.Ptr(), "query" );

	// common code used by search queries and update/delete by query
	if ( !ParseJsonQueryFilters ( pQuery, tQuery, sError, sWarning ) )
		return false;

	bProfile = false;
	cJSON * pProfile = cJSON_GetObjectItem ( pRoot.Ptr(), "profile" );
	if ( pProfile )
	{
		if ( !cJSON_IsBool ( pProfile ) )
		{
			sError = "\"profile\" property value should be boolean";
			return false;
		}

		bProfile = !!cJSON_IsTrue ( pProfile );
	}

	// expression columns go first to select list
	cJSON * pExpr = cJSON_GetObjectItem ( pRoot.Ptr(), "script_fields" );
	if ( pExpr && !ParseExpr ( pExpr, tQuery, sError ) )
		return false;

	cJSON * pSnip = cJSON_GetObjectItem ( pRoot.Ptr(), "highlight" );
	if ( pSnip && !cJSON_IsObject ( pSnip ) )
	{
		sError = "\"highlight\" property value should be an object";
		return false;
	}

	if ( pSnip )
	{
		if ( !ParseSnippet ( pSnip, tQuery, sError ) )
			return false;
		else
			bAttrsHighlight = true;
	}

	cJSON * pSort = cJSON_GetObjectItem ( pRoot.Ptr(), "sort" );
	if ( pSort && !( cJSON_IsArray ( pSort ) || cJSON_IsObject ( pSort ) ) )
	{
		sError = "\"sort\" property value should be an array or an object";
		return false;
	}

	if ( pSort )
	{
		bool bGotWeight = false;
		if ( !ParseSort ( pSort, tQuery, bGotWeight, sError, sWarning ) )
			return false;

		cJSON * pTrackScore = cJSON_GetObjectItem ( pRoot.Ptr(), "track_scores" );
		if ( pTrackScore && !cJSON_IsBool ( pTrackScore ) )
		{
			sError = "\"track_scores\" property value should be boolean";
			return false;
		}
		bool bTrackScore = pTrackScore && cJSON_IsTrue ( pTrackScore );

		if ( !bGotWeight && !bTrackScore )
			tQuery.m_eRanker = SPH_RANK_NONE;
	}

	// source \ select filter
	cJSON * pSelect = cJSON_GetObjectItem ( pRoot.Ptr(), "_source" );
	if ( pSelect && !ParseSelect ( pSelect, tQuery, sError ) )
		return false;

	return true;
}


bool ParseJsonInsert ( cJSON * pRoot, SqlStmt_t & tStmt, SphDocID_t & tDocId, bool bReplace, CSphString & sError )
{
	tStmt.m_eStmt = bReplace ? STMT_REPLACE : STMT_INSERT;

	if ( !ParseIndexId ( pRoot, tStmt, tDocId, sError ) )
		return false;

	tStmt.m_dInsertSchema.Add ( "id" );
	SqlInsert_t & tId = tStmt.m_dInsertValues.Add();
	tId.m_iType = sphGetTokTypeInt();
	tId.m_iVal = tDocId;

	// "doc" is optional
	cJSON * pSource = cJSON_GetObjectItem ( pRoot, "doc" );
	if ( pSource )
	{
		for ( int i = 0; i < cJSON_GetArraySize(pSource); i++ )
		{
			cJSON * pItem = cJSON_GetArrayItem ( pSource, i );
			assert ( pItem );

			tStmt.m_dInsertSchema.Add ( pItem->string );
			tStmt.m_dInsertSchema.Last().ToLower();

			SqlInsert_t & tNewValue = tStmt.m_dInsertValues.Add();
			if ( cJSON_IsString ( pItem ) )
			{
				tNewValue.m_iType = sphGetTokTypeStr();
				tNewValue.m_sVal = pItem->valuestring;
			} else if ( cJSON_IsNumber ( pItem ) )
			{
				tNewValue.m_iType = sphGetTokTypeFloat();
				tNewValue.m_fVal = float(pItem->valuedouble);
			} else if ( cJSON_IsInteger ( pItem ) || cJSON_IsBool ( pItem ) )
			{
				tNewValue.m_iType = sphGetTokTypeInt();
				tNewValue.m_iVal = pItem->valueint;
			} else if ( cJSON_IsArray ( pItem ) )
			{
				tNewValue.m_iType = sphGetTokTypeConstMVA();
				tNewValue.m_pVals = new RefcountedVector_c<SphAttr_t>;

				int iCount = cJSON_GetArraySize ( pItem );
				for ( int iItem=0; iItem<iCount; iItem++ )
				{
					cJSON * pArrayItem = cJSON_GetArrayItem ( pItem, iItem );
					assert ( pArrayItem );

					if ( !cJSON_IsInteger ( pArrayItem ) )
					{
						sError = "MVA elements should be integers";
						return false;
					}

					tNewValue.m_pVals->Add ( pArrayItem->valueint );
				}

			} else if ( cJSON_IsObject ( pItem ) )
			{
				tNewValue.m_iType = sphGetTokTypeStr();
				tNewValue.m_sVal = cJSON_Print ( pItem );
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


bool sphParseJsonInsert ( const char * szInsert, SqlStmt_t & tStmt, SphDocID_t & tDocId, bool bReplace, CSphString & sError )
{
	CJsonScopedPtr_c pRoot ( cJSON_Parse ( szInsert ) );
	return ParseJsonInsert ( pRoot.Ptr(), tStmt, tDocId, bReplace, sError );
}


static bool ParseUpdateDeleteQueries ( cJSON * pRoot, SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError )
{	
	tStmt.m_tQuery.m_sSelect = "*";
	if ( !ParseIndex ( pRoot, tStmt, sError ) )
		return false;

	cJSON * pId = GetJSONPropertyInt ( pRoot, "_id", sError );
	if ( pId )
	{
		CSphFilterSettings & tFilter = tStmt.m_tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_VALUES;
		tFilter.m_dValues.Add ( pId->valueint );
		tFilter.m_sAttrName = "@id";

		tDocId = pId->valueint;
	}

	// "query" is optional
	cJSON * pQuery = cJSON_GetObjectItem ( pRoot, "query" );
	if ( pQuery && pId )
	{
		sError = "both \"_id\" and \"query\" specified";
		return false;
	}

	CSphString sWarning; // fixme: add to results
	if ( !ParseJsonQueryFilters ( pQuery, tStmt.m_tQuery, sError, sWarning ) )
		return false;

	return true;
}


static bool ParseJsonUpdate ( cJSON * pRoot, SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError )
{
	tStmt.m_eStmt = STMT_UPDATE;
	tStmt.m_tUpdate.m_dRowOffset.Add ( 0 );

	if ( !ParseUpdateDeleteQueries ( pRoot, tStmt, tDocId, sError ) )
		return false;

	cJSON * pSource = GetJSONPropertyObject ( pRoot, "doc", sError );
	if ( !pSource )
		return false;

	for ( int i = 0; i < cJSON_GetArraySize(pSource); i++ )
	{
		cJSON * pItem = cJSON_GetArrayItem ( pSource, i );
		assert ( pItem );

		bool bFloat = !!cJSON_IsNumber ( pItem );
		bool bInt = !!cJSON_IsInteger ( pItem );
		bool bBool = !!cJSON_IsBool ( pItem );

		if ( bFloat || bInt || bBool )
		{
			CSphAttrUpdate & tUpd = tStmt.m_tUpdate;
			CSphString sAttr = pItem->string;
			tUpd.m_dAttrs.Add ( sAttr.ToLower().Leak() );

			if ( bInt || bBool )
			{
				int64_t iValue = pItem->valueint;

				tUpd.m_dPool.Add ( (DWORD)iValue );
				DWORD uHi = (DWORD)( iValue>>32 );

				if ( uHi )
				{
					tUpd.m_dPool.Add ( uHi );
					tUpd.m_dTypes.Add ( SPH_ATTR_BIGINT );
				} else
					tUpd.m_dTypes.Add ( SPH_ATTR_INTEGER );
			} else
			{
				float fValue = (float) pItem->valuedouble;

				tUpd.m_dPool.Add ( sphF2DW ( fValue ) );
				tUpd.m_dTypes.Add ( SPH_ATTR_FLOAT );
			}
		} else
		{
			sError = "unsupported value type";
			return false;
		}
	}

	return true;
}


bool sphParseJsonUpdate ( const char * szUpdate, SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError )
{
	CJsonScopedPtr_c pRoot ( cJSON_Parse ( szUpdate ) );
	return ParseJsonUpdate ( pRoot.Ptr(), tStmt, tDocId, sError );
}


static bool ParseJsonDelete ( cJSON * pRoot, SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError )
{
	tStmt.m_eStmt = STMT_DELETE;
	return ParseUpdateDeleteQueries ( pRoot, tStmt, tDocId, sError );
}


bool sphParseJsonDelete ( const char * szDelete, SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError )
{
	CJsonScopedPtr_c pRoot ( cJSON_Parse ( szDelete ) );
	return ParseJsonDelete ( pRoot.Ptr(), tStmt, tDocId, sError );
}


bool sphParseJsonStatement ( const char * szStmt, SqlStmt_t & tStmt, CSphString & sStmt, SphDocID_t & tDocId, CSphString & sError )
{
	CJsonScopedPtr_c pRoot ( cJSON_Parse ( szStmt ) );
	if ( !pRoot.Ptr() )
	{
		sError.SetSprintf ( "unable to parse: %s", cJSON_GetErrorPtr() );
		return false;
	}

	cJSON * pStmt = cJSON_GetArrayItem ( pRoot.Ptr(), 0 );
	if ( !pStmt )
	{
		sError = "no statement found";
		return false;
	}

	sStmt = pStmt->string;

	if ( !cJSON_IsObject(pStmt) )
	{
		sError.SetSprintf ( "statement %s should be an object", sStmt.cstr() );
		return false;
	}

	if ( sStmt=="index" || sStmt=="replace" )
	{
		if ( !ParseJsonInsert ( pStmt, tStmt, tDocId, true, sError ) )
			return false;
	} else if ( sStmt=="create" || sStmt=="insert" )
	{
		if ( !ParseJsonInsert ( pStmt, tStmt, tDocId, false, sError ) )
			return false;
	} else if ( sStmt=="update" )
	{
		if ( !ParseJsonUpdate ( pStmt, tStmt, tDocId, sError ) )
			return false;
	} else if ( sStmt=="delete" )
	{
		if ( !ParseJsonDelete ( pStmt, tStmt, tDocId, sError ) )
			return false;
	} else
	{
		sError.SetSprintf ( "unknown bulk operation: %s", pStmt->string );
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////

template <typename T>
static void PackedMVA2Json ( const BYTE * pMVA, cJSON * pArray )
{
	int iLengthBytes = sphUnpackPtrAttr ( pMVA, &pMVA );
	int nValues = iLengthBytes / sizeof(T);
	const T * pValues = (const T *)pMVA;
	for ( int i = 0; i < nValues; i++ )
		cJSON_AddItemToArray ( pArray, cJSON_CreateNumber ( pValues[i] ) );
}


static void JsonObjAddAttr ( const AggrResult_t & tRes, ESphAttr eAttrType, const char * szCol, const CSphMatch & tMatch, const CSphAttrLocator & tLoc, cJSON * pSource, CSphVector<BYTE> & dTmp )
{
	assert ( sphPlainAttrToPtrAttr(eAttrType)==eAttrType );

	switch ( eAttrType )
	{
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_TIMESTAMP:
	case SPH_ATTR_TOKENCOUNT:
	case SPH_ATTR_BIGINT:
		cJSON_AddNumberToObject ( pSource, szCol, tMatch.GetAttr(tLoc) );
		break;

	case SPH_ATTR_FLOAT:
		cJSON_AddNumberToObject ( pSource, szCol, tMatch.GetAttrFloat(tLoc) );
		break;

	case SPH_ATTR_BOOL:
		cJSON_AddBoolToObject ( pSource, szCol, tMatch.GetAttr(tLoc) );
		break;

	case SPH_ATTR_UINT32SET_PTR:
	case SPH_ATTR_INT64SET_PTR:
		{
			cJSON * pArray = cJSON_CreateArray();
			cJSON_AddItemToObject ( pSource, szCol, pArray );

			const BYTE * pMVA = (const BYTE*)tMatch.GetAttr(tLoc);
			if ( eAttrType==SPH_ATTR_UINT32SET_PTR )
				PackedMVA2Json<DWORD> ( pMVA, pArray );
			else
				PackedMVA2Json<int64_t> ( pMVA, pArray );
		}
		break;

	case SPH_ATTR_STRINGPTR:
		{
			const BYTE * pString = (const BYTE *)tMatch.GetAttr(tLoc);
			int iLen = sphUnpackPtrAttr ( pString, &pString );
			dTmp.Resize ( iLen+1 );
			memcpy ( dTmp.Begin(), pString, iLen );
			dTmp[iLen] = '\0';
			cJSON_AddStringToObject ( pSource, szCol, (const char *)dTmp.Begin() );
		}
		break;

	case SPH_ATTR_JSON_PTR:
		{
			const BYTE * pJSON = (const BYTE *)tMatch.GetAttr(tLoc);
			sphUnpackPtrAttr ( pJSON, &pJSON );

			// no object at all? return NULL
			if ( !pJSON )
			{
				cJSON_AddNullToObject ( pSource, szCol );
				break;
			}

			dTmp.Resize ( 0 );
			sphJsonFormat ( dTmp, pJSON );
			if ( !dTmp.GetLength() )
			{
				cJSON_AddNullToObject ( pSource, szCol );
				break;
			}

			dTmp.Add ( '\0' );
			cJSON_AddStringToObject ( pSource, szCol, (const char *)dTmp.Begin() );
		}
		break;

	case SPH_ATTR_FACTORS:
	case SPH_ATTR_FACTORS_JSON:
		{
			const BYTE * pFactors = (const BYTE *)tMatch.GetAttr(tLoc);
			sphUnpackPtrAttr ( pFactors, &pFactors );
			if ( pFactors )
			{
				bool bStr = ( eAttrType==SPH_ATTR_FACTORS );
				dTmp.Resize ( 0 );
				sphFormatFactors ( dTmp, (const unsigned int *)pFactors, !bStr );
				dTmp.Add ( '\0' );
				cJSON_AddStringToObject ( pSource, szCol, (const char *)dTmp.Begin() );
			} else
				cJSON_AddNullToObject ( pSource, szCol );
		}
		break;

	case SPH_ATTR_JSON_FIELD_PTR:
		{
			const BYTE * pField = (const BYTE *)tMatch.GetAttr(tLoc);
			sphUnpackPtrAttr ( pField, &pField );
			if ( !pField )
			{
				cJSON_AddNullToObject ( pSource, szCol );
				break;
			}

			ESphJsonType eJson = ESphJsonType ( *pField++ );
			if ( eJson==JSON_NULL )
				cJSON_AddNullToObject ( pSource, szCol );
			else
			{
				dTmp.Resize ( 0 );
				sphJsonFieldFormat ( dTmp, pField, eJson, true );
				dTmp.Add ( '\0' );
				cJSON_AddStringToObject ( pSource, szCol, (const char *)dTmp.Begin() );
			}
		}
		break;

	default:
		assert ( 0 && "Unknown attribute" );
		break;
	}
}

static void UnpackSnippets ( const CSphMatch & tMatch, const CSphAttrLocator & tLoc, cJSON * pVec );

static bool IsHighlightAttr ( const CSphString & sName )
{
	return sName.Begins ( g_sHighlight );
}


static bool NeedToSkipAttr ( const CSphString & sName, const CSphQuery & tQuery )
{
	const char * szName = sName.cstr();

	if ( szName[0]=='i' && szName[1]=='d' && szName[2]=='\0' ) return true;
	if ( sName.Begins ( g_sHighlight ) ) return true;
	if ( sName.Begins ( g_szFilter ) ) return true;
	if ( sName.Begins ( g_sOrder ) ) return true;

	if ( !tQuery.m_dIncludeItems.GetLength() && !tQuery.m_dExcludeItems.GetLength () )
		return false;

	// empty include - shows all select list items
	// exclude with only "*" - skip all select list items
	bool bInclude = ( tQuery.m_dIncludeItems.GetLength()==0 );
	ARRAY_FOREACH ( iItem, tQuery.m_dIncludeItems )
	{
		if ( sphWildcardMatch ( szName, tQuery.m_dIncludeItems[iItem].cstr() ) )
		{
			bInclude = true;
			break;
		}
	}
	if ( bInclude && tQuery.m_dExcludeItems.GetLength() )
	{
		ARRAY_FOREACH ( iItem, tQuery.m_dExcludeItems )
		{
			if ( sphWildcardMatch ( szName, tQuery.m_dExcludeItems[iItem].cstr() ) )
			{
				bInclude = false;
				break;
			}
		}
	}

	return !bInclude;
}


void sphEncodeResultJson ( const AggrResult_t & tRes, const CSphQuery & tQuery, CSphQueryProfile * pProfile, bool bAttrsHighlight, CSphString & sResult )
{
	CJsonScopedPtr_c pJsonRoot ( cJSON_CreateObject() );
	cJSON * pRoot = pJsonRoot.Ptr();
	assert ( pRoot );

	if ( !tRes.m_iSuccesses )
	{
		cJSON * pError = cJSON_CreateObject();
		cJSON_AddItemToObject ( pRoot, "error", pError );
		cJSON_AddStringToObject ( pError, "type", "Error" );
		cJSON_AddStringToObject ( pError, "reason", tRes.m_sError.cstr() );

		char * szResult = cJSON_Print ( pRoot );
		assert ( szResult );
		sResult.Adopt ( &szResult );

		return;
	}

	cJSON_AddNumberToObject ( pRoot, "took", tRes.m_iQueryTime );
	cJSON_AddFalseToObject ( pRoot, "timed_out" );

	if ( !tRes.m_sWarning.IsEmpty() )
	{
		cJSON * pWarning = cJSON_CreateObject();
		cJSON_AddItemToObject ( pRoot, "warning", pWarning );
		cJSON_AddStringToObject ( pWarning, "reason", tRes.m_sWarning.cstr() );
	}

	cJSON * pHitMeta = cJSON_CreateObject();
	cJSON_AddItemToObject ( pRoot, "hits", pHitMeta );
	cJSON_AddNumberToObject ( pHitMeta, "total", tRes.m_iTotalMatches );

	cJSON * pHits = cJSON_CreateArray();
	cJSON_AddItemToObject ( pHitMeta, "hits", pHits );

	const ISphSchema & tSchema = tRes.m_tSchema;
	CSphVector<BYTE> dTmp;
	int iAttrsCount = sphSendGetAttrCount ( tSchema );

	CSphBitvec dHiAttrs ( iAttrsCount );
	CSphBitvec dSkipAttrs ( iAttrsCount );
	for ( int iAttr=0; iAttr<iAttrsCount; iAttr++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
		const char * sName = tCol.m_sName.cstr();

		if ( bAttrsHighlight && IsHighlightAttr ( sName ) )
			dHiAttrs.BitSet ( iAttr );

		if ( NeedToSkipAttr ( sName, tQuery ) )
			dSkipAttrs.BitSet ( iAttr );
	}

	for ( int iMatch=tRes.m_iOffset; iMatch<tRes.m_iOffset+tRes.m_iCount; iMatch++ )
	{
		const CSphMatch & tMatch = tRes.m_dMatches[iMatch];

		cJSON * pHit = cJSON_CreateObject();
		cJSON_AddItemToArray ( pHits, pHit );
		CSphString sTmp;
		sTmp.SetSprintf ( DOCID_FMT, tMatch.m_uDocID );
		cJSON_AddStringToObject ( pHit, "_id", sTmp.cstr() );
		cJSON_AddNumberToObject ( pHit, "_score", tMatch.m_iWeight );

		cJSON * pSource = cJSON_CreateObject();
		cJSON_AddItemToObject ( pHit, "_source", pSource );

		for ( int iAttr=0; iAttr<iAttrsCount; iAttr++ )
		{
			if ( dSkipAttrs.BitGet ( iAttr ) )
				continue;

			const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
			const char * sName = tCol.m_sName.cstr();

			JsonObjAddAttr ( tRes, tCol.m_eAttrType, sName, tMatch, tCol.m_tLocator, pSource, dTmp );
		}

		if ( bAttrsHighlight )
		{
			cJSON * pHighlight = cJSON_CreateObject();
			cJSON_AddItemToObject ( pHit, "highlight", pHighlight );

			for ( int iAttr=0; iAttr<iAttrsCount; iAttr++ )
			{
				if ( !dHiAttrs.BitGet ( iAttr ) )
					continue;

				const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
				const char * sName = tCol.m_sName.cstr() + sizeof(g_sHighlight) - 1;
				assert ( tCol.m_eAttrType==SPH_ATTR_STRINGPTR );

				cJSON * pSnippets = cJSON_CreateArray();
				cJSON_AddItemToObject ( pHighlight, sName, pSnippets );

				UnpackSnippets ( tMatch, tCol.m_tLocator, pSnippets );
			}
		}
	}

	if ( pProfile )
	{
		cJSON * pProfileMeta = cJSON_CreateObject();
		assert ( pProfileMeta );

		cJSON * pProfileResult = pProfile->LeakResultAsJson();

		// FIXME: result can be empty if we run a fullscan
		if ( pProfileResult )
		{
			assert ( cJSON_IsObject ( pProfileResult ) );
			cJSON_AddItemToObject ( pProfileMeta, "query", pProfileResult );
			cJSON_AddItemToObject ( pRoot, "profile", pProfileMeta );
		} else
			cJSON_AddNullToObject ( pRoot, "profile" );
	}

	char * szResult = cJSON_Print ( pRoot );
	sResult.Adopt ( &szResult );
}


cJSON * sphEncodeInsertResultJson( const char * szIndex, bool bReplace, SphDocID_t tDocId )
{
	cJSON * pRoot = cJSON_CreateObject();
	assert ( pRoot );

	cJSON_AddStringToObject ( pRoot, "_index", szIndex );
	cJSON_AddNumberToObject ( pRoot, "_id", tDocId );
	cJSON_AddBoolToObject ( pRoot, "created", !bReplace );
	cJSON_AddStringToObject ( pRoot, "result", bReplace ? "updated" : "created" );

	return pRoot;
}


cJSON * sphEncodeUpdateResultJson( const char * szIndex, SphDocID_t tDocId, int iAffected )
{
	cJSON * pRoot = cJSON_CreateObject();
	assert ( pRoot );

	cJSON_AddStringToObject ( pRoot, "_index", szIndex );

	if ( tDocId==DOCID_MAX )
		cJSON_AddNumberToObject ( pRoot, "updated", iAffected );
	else
	{
		cJSON_AddNumberToObject ( pRoot, "_id", tDocId );
		cJSON_AddStringToObject ( pRoot, "result", iAffected ? "updated" : "noop" );
	}

	return pRoot;
}


cJSON * sphEncodeDeleteResultJson ( const char * szIndex, SphDocID_t tDocId, int iAffected )
{
	cJSON * pRoot = cJSON_CreateObject();
	assert ( pRoot );

	cJSON_AddStringToObject ( pRoot, "_index", szIndex );

	if ( tDocId==DOCID_MAX )
		cJSON_AddNumberToObject ( pRoot, "deleted", iAffected );
	else
	{
		cJSON_AddNumberToObject ( pRoot, "_id", tDocId );
		cJSON_AddBoolToObject ( pRoot, "found", iAffected ? 1 : 0 );
		cJSON_AddStringToObject ( pRoot, "result", iAffected ? "deleted" : "not found" );
	}

	return pRoot;
}


cJSON * sphEncodeInsertErrorJson ( const char * szIndex, const char * szError )
{
	cJSON * pRoot = cJSON_CreateObject();
	assert ( pRoot );

	cJSON * pError = cJSON_CreateObject();
	assert ( pError );

	cJSON_AddStringToObject ( pError, "type", szError );
	cJSON_AddStringToObject ( pError, "index", szIndex );

	cJSON_AddItemToObject ( pRoot, "error", pError );
	cJSON_AddNumberToObject ( pRoot, "status", 500 );

	return pRoot;
}


void AddAccessSpecs ( XQNode_t * pNode, cJSON * pJson, const CSphSchema & tSchema  )
{
	assert ( pNode && pJson );

	// dump spec for keyword nodes
	// FIXME? double check that spec does *not* affect non keyword nodes
	if ( pNode->m_dSpec.IsEmpty() || !pNode->m_dWords.GetLength() )
		return;

	const XQLimitSpec_t & tSpec = pNode->m_dSpec;
	if ( tSpec.m_bFieldSpec && !tSpec.m_dFieldMask.TestAll ( true ) )
	{
		cJSON * pFields = cJSON_CreateArray();
		cJSON_AddItemToObject ( pJson, "fields", pFields );

		for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
			if ( tSpec.m_dFieldMask.Test(i) )
				cJSON_AddItemToArray ( pFields, cJSON_CreateString ( tSchema.GetFieldName(i) ) );
	}

	cJSON_AddNumberToObject ( pJson, "max_field_pos", tSpec.m_iFieldMaxPos );
}


cJSON * CreateKeywordNode ( const XQKeyword_t & tKeyword )
{
	cJSON * pRoot = cJSON_CreateObject();
	assert ( pRoot );

	cJSON_AddStringToObject ( pRoot, "type", "KEYWORD" );
	cJSON_AddStringToObject ( pRoot, "word", tKeyword.m_sWord.cstr() );
	cJSON_AddNumberToObject ( pRoot, "querypos", tKeyword.m_iAtomPos );

	if ( tKeyword.m_bExcluded )
		cJSON_AddTrueToObject ( pRoot, "excluded" );

	if ( tKeyword.m_bExpanded )
		cJSON_AddTrueToObject ( pRoot, "expanded" );

	if ( tKeyword.m_bFieldStart )
		cJSON_AddTrueToObject ( pRoot, "field_start" );

	if ( tKeyword.m_bFieldEnd )
		cJSON_AddTrueToObject ( pRoot, "field_end" );

	if ( tKeyword.m_bMorphed )
		cJSON_AddTrueToObject ( pRoot, "morphed" );

	if ( tKeyword.m_fBoost!=1.0f ) // really comparing floats?
		cJSON_AddNumberToObject ( pRoot, "boost", tKeyword.m_fBoost );

	return pRoot;
}


cJSON * sphBuildProfileJson ( XQNode_t * pNode, const CSphSchema & tSchema )
{
	assert ( pNode );

	cJSON * pRoot = cJSON_CreateObject();
	assert ( pRoot );

	CSphString sNodeName ( sphXQNodeToStr ( pNode ) );
	cJSON_AddStringToObject ( pRoot, "type", sNodeName.cstr() );

	CSphString sDescription ( sphExplainQueryBrief ( pNode, tSchema ) );
	cJSON_AddStringToObject ( pRoot, "description", sDescription.cstr() );

	CSphString sNodeOptions ( sphXQNodeGetExtraStr ( pNode ) );
	if ( !sNodeOptions.IsEmpty() )
		cJSON_AddStringToObject ( pRoot, "options", sNodeOptions.cstr() );

	AddAccessSpecs ( pNode, pRoot, tSchema );

	cJSON * pChildren = cJSON_CreateArray();
	cJSON_AddItemToObject ( pRoot, "children", pChildren );

	if ( pNode->m_dChildren.GetLength() )
	{
		for ( auto i : pNode->m_dChildren )
		{
			cJSON * pChild = sphBuildProfileJson ( i, tSchema );
			assert ( pChild );
			cJSON_AddItemToArray ( pChildren, pChild );
		}
	} else
	{
		for ( auto i : pNode->m_dWords )
		{
			cJSON * pKeyword = CreateKeywordNode(i);
			assert ( pKeyword );
			cJSON_AddItemToArray ( pChildren, pKeyword );
		}
	}

	return pRoot;
}


static void * cJsonMalloc ( size_t uSize )
{
	return new BYTE[uSize];
}


static void cJsonFree ( void * pPtr )
{
	delete [] (BYTE *)pPtr;
}


void sphInitCJson()
{
	cJSON_Hooks tHooks { cJsonMalloc, cJsonFree };
	cJSON_InitHooks ( &tHooks );
}

//////////////////////////////////////////////////////////////////////////
// Highlight

struct HttpSnippetField_t
{
	int m_iFragmentSize;
	int m_iFragmentCount;
	CSphString m_sName;

	HttpSnippetField_t()
		: m_iFragmentSize ( -1 )
		, m_iFragmentCount ( -1 )
	{}
};

static bool CheckField ( HttpSnippetField_t & tParsed, CSphString & sError, const cJSON * pField )
{
	assert ( pField && cJSON_IsObject ( pField ) );
	if ( !cJSON_GetArraySize ( pField ) )
		return true;

	const cJSON * pType = cJSON_GetObjectItem ( pField, "type" );
	if ( pType && cJSON_IsString ( pType ) )
	{
		CSphString sType ( pType->valuestring );
		if ( !sType.IsEmpty() && sType!="unified" )
		{
			sError = "\"type\" property only \"unified\" supported";
			return false;
		}
	}

	if ( cJSON_GetObjectItem ( pField, "force_source" ) )
	{
		sError = "\"force_source\" property not supported";
		return false;
	}

	const cJSON * pFragmenter = cJSON_GetObjectItem ( pField, "fragmenter" );
	if ( pFragmenter && cJSON_IsString ( pFragmenter ) )
	{
		CSphString sFragmenter ( pFragmenter->valuestring );
		if ( !sFragmenter.IsEmpty() && sFragmenter!="span" )
		{
			sError = "\"fragmenter\" property only \"span\" supported";
			return false;
		}
	}

	const cJSON * pFragmentSize = cJSON_GetObjectItem ( pField, "fragment_size" );
	if ( pFragmentSize && !cJSON_IsInteger ( pFragmentSize ) )
	{
		sError = "\"fragment_size\" property value should be an integer";
		return false;
	}
	if ( pFragmentSize )
		tParsed.m_iFragmentSize = pFragmentSize->valueint;

	const cJSON * pFragmentCount = cJSON_GetObjectItem ( pField, "number_of_fragments" );
	if ( pFragmentCount && !cJSON_IsInteger ( pFragmentCount ) )
	{
		sError = "\"number_of_fragments\" property value should be an integer";
		return false;
	}
	if ( pFragmentCount )
		tParsed.m_iFragmentCount = pFragmentCount->valueint;

	return true;
}

bool ParseSnippet ( cJSON * pSnip, CSphQuery & tQuery, CSphString & sError )
{
	assert ( pSnip );

	if ( cJSON_GetObjectItem ( pSnip, "tags_schema" ) )
	{
		sError = "\"tags_schema\" property not supported";
		return false;
	}

	// FIXME!!! snippets expression made here however depends on local index fields
	// "require_field_match" : true - is not error
	if ( cJSON_GetObjectItem ( pSnip, "require_field_match" ) )
	{
		sError = "\"require_field_match\" property not supported";
		return false;
	}

	// FIXME!!! have only SPZ and phrase_boundary settings these not directly applicable
	if ( cJSON_GetObjectItem ( pSnip, "boundary_scanner" ) )
	{
		sError = "\"boundary_scanner\" property not supported";
		return false;
	}

	if ( cJSON_GetObjectItem ( pSnip, "max_fragment_length" ) )
	{
		sError = "\"max_fragment_length\" property not supported";
		return false;
	}

	const cJSON * pFileds = cJSON_GetObjectItem ( pSnip, "fields" );
	if ( pFileds && !cJSON_IsObject ( pFileds ) )
	{
		sError = "\"fields\" property value should be an object";
		return false;
	}

	bool bKeepHtml = false;
	const cJSON * pEncoder = cJSON_GetObjectItem ( pSnip, "encoder" );
	if ( pEncoder )
	{
		if ( !cJSON_IsString ( pEncoder ) )
		{
			sError = "\"encoder\" property value should be a string";
			return false;
		}
		int iLen = 0;
		if ( pEncoder->valuestring && *pEncoder->valuestring )
			iLen = strnlen ( pEncoder->valuestring, 32 );
		bKeepHtml = ( iLen>0 && strncmp ( pEncoder->valuestring, "html", iLen )==0 );
	}

	CSphString sQuery;
	const cJSON * pQuery = cJSON_GetObjectItem ( pSnip, "highlight_query" );
	if ( pQuery )
	{
		if ( !cJSON_IsObject ( pQuery ) )
		{
			sError = "\"highlight_query\" property value should be an object";
			return false;
		}
		sQuery = cJSON_Print ( pQuery );
	}

	CSphString sPreTag;
	const cJSON * pPreTags = cJSON_GetObjectItem ( pSnip, "pre_tags" );
	if ( pPreTags )
	{
		if ( !cJSON_IsString ( pPreTags ) || cJSON_IsObject ( pPreTags ) )
		{
			if ( !cJSON_IsString ( pPreTags ) )
				sError = "\"pre_tags\" property value should be a string";
			else
				sError = "\"pre_tags\" property value should be a string, object not supported";
			return false;
		}
		sPreTag = pPreTags->valuestring;
	}

	CSphString sPostTag;
	const cJSON * pPostTags = cJSON_GetObjectItem ( pSnip, "post_tags" );
	if ( pPostTags )
	{
		if ( !cJSON_IsString ( pPostTags ) || cJSON_IsObject ( pPostTags ) )
		{
			if ( !cJSON_IsString ( pPostTags ) )
				sError = "\"post_tags\" property value should be a string";
			else
				sError = "\"post_tags\" property value should be a string, object not supported";
			return false;
		}
		sPostTag = pPostTags->valuestring;
	}

	int iNoMatch = 0;
	const cJSON * pNoMatch = cJSON_GetObjectItem ( pSnip, "no_match_size" );
	if ( pNoMatch )
	{
		if ( !cJSON_IsInteger ( pNoMatch ) )
		{
			sError = "\"no_match_size\" property value should be an integer";
			return false;
		}
		iNoMatch = pNoMatch->valueint;
	}

	bool bWeightOrder = false;
	const cJSON * pOrder = cJSON_GetObjectItem ( pSnip, "order" );
	if ( pOrder )
	{
		if ( !cJSON_IsString ( pOrder ) )
		{
			sError = "\"order\" property value should be a string";
			return false;
		}
		int iLen = 0;
		if ( pOrder->valuestring && *pOrder->valuestring )
			iLen = strnlen ( pOrder->valuestring, 32 );
		bWeightOrder = ( iLen>0 && strncmp ( pOrder->valuestring, "score", iLen )==0 );
	}

	HttpSnippetField_t tGlobalOptions;
	if ( !CheckField ( tGlobalOptions, sError, pSnip ) )
		return false;

	int iFields = cJSON_GetArraySize ( pFileds );
	CSphVector<HttpSnippetField_t> dFields;
	dFields.Reserve ( iFields );

	for ( int iField=0; iField<iFields; iField++ )
	{
		const cJSON * pField = cJSON_GetArrayItem ( pFileds, iField );
		if ( !cJSON_IsObject ( pField ) )
		{
			sError.SetSprintf ( "\"%s\" field should be an object", pField->string );
			return false;
		}

		HttpSnippetField_t & tField = dFields.Add();
		tField.m_sName = pField->string;
		if ( !CheckField ( tField, sError, pField ) )
			return false;

		if ( tGlobalOptions.m_iFragmentSize!=-1 )
			tField.m_iFragmentSize = tGlobalOptions.m_iFragmentSize;
		if ( tGlobalOptions.m_iFragmentCount!=-1 )
			tField.m_iFragmentCount = tGlobalOptions.m_iFragmentCount;
	}

	// FIXME!!! get rid of print -> parse
	StringBuilder_c sSelect;
	sSelect += tQuery.m_sSelect.cstr();

	StringBuilder_c sItem;
	ARRAY_FOREACH ( iSnip, dFields )
	{
		const HttpSnippetField_t & tSnip = dFields[iSnip];
		sItem.Clear();
		const char * sHiQuery = ( sQuery.IsEmpty() ? tQuery.m_sQuery.cstr() : sQuery.cstr() );
		sItem.Appendf ( "SNIPPET(%s, '%s'", tSnip.m_sName.cstr(), sHiQuery );

		if ( !sPreTag.IsEmpty() )
			sItem.Appendf ( ", 'before_match=%s'", sPreTag.cstr() );
		if ( !sPostTag.IsEmpty() )
			sItem.Appendf ( ", 'after_match=%s'", sPostTag.cstr() );
		if ( tSnip.m_iFragmentSize!=-1 && !bKeepHtml )
			sItem.Appendf ( ", 'limit=%d'", tSnip.m_iFragmentSize );
		if ( tSnip.m_iFragmentCount!=-1 && !bKeepHtml )
			sItem.Appendf ( ", 'limit_passages=%d'", tSnip.m_iFragmentCount );
		if ( iNoMatch<1 )
			sItem.Appendf ( ", 'allow_empty=1'" );
		if ( bWeightOrder )
			sItem.Appendf ( ", 'weight_order=1'" );
		if ( bKeepHtml )
			sItem.Appendf ( ", 'html_strip_mode=retain', 'limit=0'" );

		sItem += ",'json_query=1')";

		sSelect += ", ";
		sSelect += sItem.cstr();

		CSphQueryItem & tItem = tQuery.m_dItems.Add();
		tItem.m_sExpr = sItem.cstr();
		tItem.m_sAlias.SetSprintf ( "%s%s", g_sHighlight, tSnip.m_sName.cstr() );
	}

	return true;
}


struct PassageLocator_t
{
	int m_iOff;
	int m_iSize;
};


int PackSnippets ( const CSphVector<BYTE> & dRes, CSphVector<int> & dSeparators, int iSepLen, const BYTE ** ppStr )
{
	if ( !dSeparators.GetLength() && !dRes.GetLength() )
		return 0;

	int iLast = 0;
	int iTextLen = 0;
	CSphVector<PassageLocator_t> dPassages;
	dPassages.Reserve ( dSeparators.GetLength() );
	ARRAY_FOREACH ( iPassage, dSeparators )
	{
		int iCur = dSeparators[iPassage];
		int iFrom = iLast;
		int iLen = iCur - iFrom;
		iLast = iCur + iSepLen;
		if ( iLen<=0 )
			continue;
		PassageLocator_t & tPass = dPassages.Add();
		tPass.m_iOff = iFrom;
		tPass.m_iSize = iLen;
		iTextLen += iLen + 1;
	}

	if ( !dPassages.GetLength() )
	{
		PassageLocator_t & tPass = dPassages.Add();
		tPass.m_iOff = 0;
		tPass.m_iSize = dRes.GetLength();
		iTextLen = dRes.GetLength() + 1;
	}

	int iPassageCount = dPassages.GetLength();

	CSphVector<BYTE> dOut;
	BYTE * pData;

	pData = dOut.AddN ( sizeof(iPassageCount) );
	sphUnalignedWrite ( pData, iPassageCount );

	ARRAY_FOREACH ( iPassage, dPassages )
	{
		int iSize = dPassages[iPassage].m_iSize + 1;

		pData = dOut.AddN ( sizeof(iSize) );
		sphUnalignedWrite ( pData, iSize );
	}

	const BYTE * sText = dRes.Begin();
	ARRAY_FOREACH ( iPassage, dPassages )
	{
		int iSize = dPassages[iPassage].m_iSize;

		pData = dOut.AddN ( iSize+1 );
		memcpy ( pData, sText + dPassages[iPassage].m_iOff, iSize );
		pData[iSize] = '\0'; // make sz-string from binary
	}

	int iTotalSize = dOut.GetLength();
	*ppStr = dOut.LeakData();
	return iTotalSize;
}


void UnpackSnippets ( const CSphMatch & tMatch, const CSphAttrLocator & tLoc, cJSON * pSnippets )
{
	const BYTE * pData = (const BYTE *)tMatch.GetAttr(tLoc);
	sphUnpackPtrAttr ( pData, &pData );
	if ( !pData )
		return;

	int iPassageCount = sphUnalignedRead( *(int *)pData );
	pData += sizeof(iPassageCount);

	const int * pSize = (const int *)pData;
	const char * pText = (const char *)( pData + sizeof(iPassageCount) * iPassageCount );
	int iTextOff = 0;
	for ( int i=0; i<iPassageCount; i++ )
	{
		const char * pPassage = pText + iTextOff;
		cJSON_AddItemToArray ( pSnippets, cJSON_CreateString ( pPassage ) );

		iTextOff += sphUnalignedRead ( *pSize );
		pSize++;
	}
}

//////////////////////////////////////////////////////////////////////////
// Sort
struct SortField_t : public GeoDistInfo_c
{
	CSphString m_sName;
	CSphString m_sMode;
	bool m_bAsc {true};
};


bool ParseSort ( cJSON * pSort, CSphQuery & tQuery, bool & bGotWeight, CSphString & sError, CSphString & sWarning )
{
	assert ( pSort );
	bGotWeight = false;

	// unsupported options
	cJSON * pScript = cJSON_GetObjectItem ( pSort, "_script" );
	if ( pScript )
	{
		sError = "\"_script\" property not supported";
		return false;
	}

	int iItemsCount = cJSON_GetArraySize ( pSort );
	CSphVector<SortField_t> dSort;
	dSort.Reserve ( iItemsCount );

	for ( int i=0; i<iItemsCount; i++ )
	{
		cJSON * pItem = cJSON_GetArrayItem ( pSort, i );
		if ( !pItem )
		{
			sError.SetSprintf ( "missed %d \"sort\" property item", i );
			return false;
		}

		bool bString = !!cJSON_IsString ( pItem );
		bool bObj = !!cJSON_IsObject ( pItem );
		if ( !bString && !bObj )
		{
			sError.SetSprintf ( "\"sort\" property %d(\"%s\") should be a string or an object", i, ( pItem->string ? pItem->string : "" ) );
			return false;
		}
		if ( bString && !pItem->valuestring )
		{
			sError.SetSprintf ( "\"sort\" property %d should be a string", i );
			return false;
		}
		if ( bObj && cJSON_GetArraySize ( pItem )!=1 )
		{
			sError.SetSprintf ( "\"sort\" property %d(\"%s\") should be object", i, ( pItem->string ? pItem->string : "" ) );
			return false;
		}


		// [ "attr_name" ]
		if ( bString )
		{
			SortField_t & tItem = dSort.Add();
			tItem.m_sName = pItem->valuestring;
			// order defaults to desc when sorting on the _score, and defaults to asc when sorting on anything else
			tItem.m_bAsc = ( tItem.m_sName!="_score" );
			continue;
		}

		cJSON * pSortItem = cJSON_GetArrayItem ( pItem, 0 );
		if ( !pSortItem )
		{
			sError.SetSprintf ( "invalid %d \"sort\" property item", i );
			return false;
		}

		bool bSortString = !!cJSON_IsString ( pSortItem );
		bool bSortObj = !!cJSON_IsObject ( pSortItem );
		if ( ( !bSortString && !bSortObj ) || !pSortItem->string
			|| ( bSortString && !pSortItem->valuestring ) )
		{
			sError.SetSprintf ( "\"sort\" property 0(\"%s\") should be %s", ( pItem->string ? pItem->string : "" ), ( bSortObj ? "a string" : "an object" ) );
			return false;
		}

		// [ { "attr_name" : "sort_mode" } ]
		if ( bSortString )
		{
			CSphString sOrder = pSortItem->valuestring;
			if ( sOrder!="asc" && sOrder!="desc" )
			{
				sError.SetSprintf ( "\"sort\" property \"%s\" order is invalid %s", pSortItem->string, sOrder.cstr() );
				return false;
			}

			SortField_t & tItem = dSort.Add();
			tItem.m_sName = pSortItem->string;
			tItem.m_bAsc = ( sOrder=="asc" );
			continue;
		}


		// [ { "attr_name" : { "order" : "sort_mode" } } ]
		SortField_t & tItem = dSort.Add();
		tItem.m_sName = pSortItem->string;

		cJSON * pAttrItems = cJSON_GetObjectItem ( pSortItem, "order" );
		if ( pAttrItems )
		{
			if ( !cJSON_IsString ( pAttrItems ) )
			{
				sError.SetSprintf ( "\"sort\" property \"%s\" order is invalid", pAttrItems->string );
				return false;
			}

			CSphString sOrder = pAttrItems->valuestring;
			tItem.m_bAsc = ( sOrder=="asc" );
		}

		cJSON * pMode = cJSON_GetObjectItem ( pSortItem, "mode" );
		if ( pMode )
		{
			if ( !cJSON_IsString ( pMode ) )
			{
				sError.SetSprintf ( "\"mode\" property \"%s\" order is invalid", pAttrItems->string );
				return false;
			}

			CSphString sMode = pMode->valuestring;
			if ( sMode!="min" && sMode!="max" )
			{
				sError.SetSprintf ( "\"mode\" supported are \"min\" and \"max\", got \"%s\", not supported", sMode.cstr() );
				return false;
			}

			tItem.m_sMode = sMode;
		}

		// geodist
		if ( tItem.m_sName=="_geo_distance" )
		{
			if ( pMode )
			{
				sError = "\"mode\" property not supported with \"_geo_distance\"";
				return false;
			}
			if ( cJSON_GetObjectItem ( pSortItem, "unit" ) )
			{
				sError = "\"unit\" property not supported with \"_geo_distance\"";
				return false;
			}

			if ( !tItem.Parse ( pSortItem, false, sError, sWarning ) )
				return false;
		}

		// unsupported options
		cJSON * pUnmapped = cJSON_GetObjectItem ( pSortItem, "unmapped_type" );
		if ( pUnmapped )
		{
			sError = "\"unmapped_type\" property not supported";
			return false;
		}
		cJSON * pMissing = cJSON_GetObjectItem ( pSortItem, "missing" );
		if ( pMissing )
		{
			sError = "\"missing\" property not supported";
			return false;
		}
		cJSON * pNestedPath = cJSON_GetObjectItem ( pSortItem, "nested_path" );
		if ( pNestedPath )
		{
			sError = "\"nested_path\" property not supported";
			return false;
		}
		cJSON * pNestedFilter = cJSON_GetObjectItem ( pSortItem, "nested_filter" );
		if ( pNestedFilter )
		{
			sError = "\"nested_filter\" property not supported";
			return false;
		}
	}

	const char * sSep = "";
	StringBuilder_c sSortBuf;
	StringBuilder_c sTmp;
	bGotWeight = false;

	ARRAY_FOREACH ( i, dSort )
	{
		const SortField_t & tItem = dSort[i];
		if ( tItem.IsGeoDist() )
		{
			// ORDER BY statement
			sSortBuf.Appendf ( "%s%s%s %s", sSep, g_sOrder, tItem.m_sName.cstr(), ( tItem.m_bAsc ? "asc" : "desc" ) );

			// query item
			CSphQueryItem & tQueryItem = tQuery.m_dItems.Add();
			tQueryItem.m_sExpr = tItem.BuildExprString();
			tQueryItem.m_sAlias.SetSprintf ( "%s%s", g_sOrder, tItem.m_sName.cstr() );

			// select list
			sTmp.Clear();
			sTmp.Appendf ( "%s, %s as %s", tQuery.m_sSelect.cstr(), tQueryItem.m_sExpr.cstr(), tQueryItem.m_sAlias.cstr() );
			tQuery.m_sSelect = sTmp.cstr();
		} else if ( tItem.m_sMode.IsEmpty() )
		{
			// sort by attribute or weight
			sSortBuf.Appendf ( "%s%s %s", sSep, ( tItem.m_sName=="_score" ? "@weight" : tItem.m_sName.cstr() ), ( tItem.m_bAsc ? "asc" : "desc" ) );
			bGotWeight |= ( tItem.m_sName=="_score" );
		} else
		{
			// sort by MVA

			// ORDER BY statement
			sSortBuf.Appendf ( "%s%s%s %s", sSep, g_sOrder, tItem.m_sName.cstr(), ( tItem.m_bAsc ? "asc" : "desc" ) );

			// query item
			sTmp.Clear();
			sTmp.Appendf ( "%s(%s)", tItem.m_sMode=="min" ? "least" : "greatest", tItem.m_sName.cstr() );
			CSphQueryItem & tQueryItem = tQuery.m_dItems.Add();
			tQueryItem.m_sExpr = sTmp.cstr();
			tQueryItem.m_sAlias.SetSprintf ( "%s%s", g_sOrder, tItem.m_sName.cstr() );

			// select list
			sTmp.Clear();
			sTmp.Appendf ( "%s, %s as %s", tQuery.m_sSelect.cstr(), tQueryItem.m_sExpr.cstr(), tQueryItem.m_sAlias.cstr() );
			tQuery.m_sSelect = sTmp.cstr();
		}
		sSep = ", ";
	}
	if ( !dSort.GetLength() )
	{
		sSortBuf += "@weight desc";
		bGotWeight = true;
	}

	tQuery.m_eSort = SPH_SORT_EXTENDED;
	tQuery.m_sSortBy = sSortBuf.cstr();

	return true;
}

bool ParseLocation ( const char * sName, cJSON * pLoc, LocationField_t * pField, LocationSource_t * pSource, CSphString & sError )
{
	assert ( pLoc );
	bool bParseField = ( pField!=NULL );
	assert ( ( bParseField && pField!=NULL ) || pSource!=NULL );

	bool bObj = !!cJSON_IsObject ( pLoc );
	bool bString = !!cJSON_IsString ( pLoc );
	bool bArray = !!cJSON_IsArray ( pLoc );


	if ( !bObj && !bString && !bArray )
	{
		sError.SetSprintf ( "\"%s\" property value should be either object or string or array", sName );
		return false;
	}

	if ( bObj )
	{
		cJSON * pLat = cJSON_GetObjectItem ( pLoc, "lat" );
		cJSON * pLon = cJSON_GetObjectItem ( pLoc, "lon" );
		if ( !pLat || !pLon )
		{
			if ( !pLat && !pLon )
				sError = "\"lat\" and \"lon\" properties missed";
			else
				sError.SetSprintf ( "\"%s\" property missed", ( !pLat ? "lat" : "lon" ) );
			return false;
		}

		bool bLatChecked = bParseField ? !!cJSON_IsNumeric ( pLat ) : !!cJSON_IsString ( pLat );
		bool bLonChecked = bParseField ? !!cJSON_IsNumeric ( pLon ) : !!cJSON_IsString ( pLon );
		if ( !bLatChecked || !bLonChecked )
		{
			if ( !bLatChecked && !bLonChecked )
				sError.SetSprintf ( "\"lat\" and \"lon\" property values should be %s", ( bParseField ? "numeric" : "string" ) );
			else
				sError.SetSprintf ( "\"%s\" property value should be %s", ( !bLatChecked ? "lat" : "lon" ), ( bParseField ? "numeric" : "string" ) );
			return false;
		}

		if ( bParseField )
		{
			pField->m_fLat = (float)pLat->valuedouble;
			pField->m_fLon = (float)pLon->valuedouble;
		} else
		{
			pSource->m_sLat = pLat->valuestring;
			pSource->m_sLon = pLon->valuestring;
		}
		return true;
	}

	if ( bString )
	{
		CSphVector<CSphString> dVals;
		sphSplit ( dVals, pLoc->valuestring );

		if ( dVals.GetLength()!=2 )
		{
			sError.SetSprintf ( "\"%s\" property values should be sting with lat,lon items, got %d items", sName, dVals.GetLength() );
			return false;
		}

		// string and array order differs
		// string - lat, lon
		// array - lon, lat
		int iLatLen = dVals[0].Length();
		int iLonLen = dVals[1].Length();
		if ( !iLatLen || !iLonLen )
		{
			if ( !iLatLen && !iLonLen )
				sError.SetSprintf ( "\"lat\" and \"lon\" values should be %s", ( bParseField ? "numeric" : "string" ) );
			else
				sError.SetSprintf ( "\"%s\" value should be %s", ( !iLatLen ? "lat" : "lon" ), ( bParseField ? "numeric" : "string" ) );
			return false;
		}

		if ( bParseField )
		{
			pField->m_fLat = (float)atof ( dVals[0].cstr() );
			pField->m_fLon = (float)atof ( dVals[1].cstr() );
		} else
		{
			pSource->m_sLat = dVals[0];
			pSource->m_sLon = dVals[1];
		}
		return true;
	}

	assert ( bArray );
	int iCount = cJSON_GetArraySize ( pLoc );
	if ( iCount!=2 )
	{
		sError.SetSprintf ( "\"%s\" property values should be array with lat,lon items, got %d items", sName, iCount );
		return false;
	}

	// string and array order differs
	// array - lon, lat
	// string - lat, lon
	cJSON * pLon = cJSON_GetArrayItem ( pLoc, 0 );
	cJSON * pLat = cJSON_GetArrayItem ( pLoc, 1 );
	if ( !pLat || !pLon )
	{
		if ( !pLat && !pLon )
			sError = "\"lat\" and \"lon\" properties missed";
		else
			sError.SetSprintf ( "\"%s\" property missed", !pLat ? "lat" : "lon" );
		return false;
	}

	bool bLatChecked = bParseField ? !!cJSON_IsNumeric ( pLat ) : !!cJSON_IsString ( pLat );
	bool bLonChecked = bParseField ? !!cJSON_IsNumeric ( pLon ) : !!cJSON_IsString ( pLon );
	if ( !bLatChecked || !bLonChecked )
	{
		if ( !bLatChecked && !bLonChecked )
			sError.SetSprintf ( "\"lat\" and \"lon\" property values should be %s", ( bParseField ? "numeric" : "string" ) );
		else
			sError.SetSprintf ( "\"%s\" property value should be %s", ( !bLatChecked ? "lat" : "lon" ), ( bParseField ? "numeric" : "string" ) );
		return false;
	}

	if ( bParseField )
	{
		pField->m_fLat = (float)pLat->valuedouble;
		pField->m_fLon = (float)pLon->valuedouble;
	} else
	{
		pSource->m_sLat = pLat->valuestring;
		pSource->m_sLon = pLon->valuestring;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// _source / select list

static bool ParseStringArray ( const cJSON * pArray, const char * sProp, CSphVector<CSphString> & dItems, CSphString & sError )
{
	int iCount = cJSON_GetArraySize ( pArray );
	for ( int iItem=0; iItem<iCount; iItem++ )
	{
		cJSON * pItem = cJSON_GetArrayItem ( pArray, iItem );
		if ( !pItem )
		{
			sError.SetSprintf ( "missed %d %s property item", iItem, sProp );
			return false;
		}
		if ( !cJSON_IsString ( pItem ) )
		{
			sError.SetSprintf ( "%s property %d should be a string", sProp, iItem );
			return false;
		}

		dItems.Add ( pItem->valuestring );
	}

	return true;
}

bool ParseSelect ( cJSON * pSelect, CSphQuery & tQuery, CSphString & sError )
{
	bool bString = !!cJSON_IsString ( pSelect );
	bool bArray = !!cJSON_IsArray ( pSelect );
	bool bObj = !!cJSON_IsObject ( pSelect );

	if ( !bString && !bArray && !bObj )
	{
		sError = "\"_source\" property should be a string or an array or an object";
		return false;
	}

	if ( bString )
	{
		tQuery.m_dIncludeItems.Add ( pSelect->valuestring );
		if ( tQuery.m_dIncludeItems[0]=="*" || tQuery.m_dIncludeItems[0].IsEmpty() )
			tQuery.m_dIncludeItems.Reset();

		return true;
	}

	if ( bArray )
		return ParseStringArray ( pSelect, "\"_source\"", tQuery.m_dIncludeItems, sError );

	assert ( bObj );

	// includes part of _source object
	cJSON * pInclude = cJSON_GetObjectItem ( pSelect, "includes" );
	if ( pInclude && !cJSON_IsArray ( pInclude ) )
	{
		sError = "\"_source\" \"includes\" property should be an array";
		return false;
	}
	if ( pInclude )
	{
		if ( !ParseStringArray ( pInclude, "\"_source\" \"includes\"", tQuery.m_dIncludeItems, sError ) )
			return false;

		if ( tQuery.m_dIncludeItems.GetLength()==1 && tQuery.m_dIncludeItems[0]=="*" )
			tQuery.m_dIncludeItems.Reset();
	}

	// excludes part of _source object
	cJSON * pExclude = cJSON_GetObjectItem ( pSelect, "excludes" );
	if ( pExclude && !cJSON_IsArray ( pExclude ) )
	{
		sError = "\"_source\" \"excludes\" property should be an array";
		return false;
	}
	if ( pExclude )
	{
		if ( !ParseStringArray ( pExclude, "\"_source\" \"excludes\"", tQuery.m_dExcludeItems, sError ) )
			return false;

		if ( !tQuery.m_dExcludeItems.GetLength() )
			tQuery.m_dExcludeItems.Add ( "*" );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// script_fields / expressions

static bool GetString ( const cJSON * pObj, const char * sProp, CSphString & sVal, CSphString & sError )
{
	assert ( pObj && sProp );
	cJSON * pProp = cJSON_GetObjectItem ( pObj, sProp );
	if ( !pProp )
	{
		sError.SetSprintf ( "\"%s\" property should have \"%s\" string", pObj->string, sProp );
		return false;

	}

	if ( !cJSON_IsString ( pProp ) )
	{
		sError.SetSprintf ( "\"%s\" property should be a string", sProp );
		return false;
	}

	sVal = pProp->valuestring;
	return true;
}

bool ParseExpr ( cJSON * pExpr, CSphQuery & tQuery, CSphString & sError )
{
	if ( !pExpr )
		return true;

	if ( !cJSON_IsObject ( pExpr ) )
	{
		sError = "\"script_fields\" property should be an object";
		return false;
	}

	StringBuilder_c sSelect;
	sSelect += tQuery.m_sSelect.cstr();

	int iCount = cJSON_GetArraySize ( pExpr );
	for ( int iExpr=0; iExpr<iCount; iExpr++ )
	{
		cJSON * pAlias = cJSON_GetArrayItem ( pExpr, iExpr );
		if ( !pAlias )
		{
			sError.SetSprintf ( "missed %d \"script_fields\" property item", iExpr );
			return false;
		}
		if ( !cJSON_IsObject ( pAlias ) )
		{
			sError.SetSprintf ( "\"script_fields\" property %d should be an object", iExpr );
			return false;
		}
		if ( !pAlias->string || !*pAlias->string )
		{
			sError.SetSprintf ( "\"script_fields\" property %d invalid name", iExpr );
			return false;
		}

		cJSON * pAliasScript = cJSON_GetObjectItem ( pAlias, "script" );
		if ( !pAliasScript )
		{
			sError.SetSprintf ( "\"script_fields\" property should have \"script\" object" );
			return false;
		}

		CSphString sExpr;
		if ( !GetString ( pAliasScript, "inline", sExpr, sError ) )
			return false;

		// unsupported options
		if ( cJSON_GetObjectItem ( pAliasScript, "lang" ) )
		{
			sError.SetSprintf ( "\"script_fields\" \"lang\" property unsupported, use manticore inline expression" );
			return false;
		}
		if ( cJSON_GetObjectItem ( pAliasScript, "params" ) )
		{
			sError.SetSprintf ( "\"script_fields\" \"params\" property unsupported" );
			return false;
		}
		if ( cJSON_GetObjectItem ( pAliasScript, "stored" ) )
		{
			sError.SetSprintf ( "\"script_fields\" \"stored\" property unsupported" );
			return false;
		}
		if ( cJSON_GetObjectItem ( pAliasScript, "file" ) )
		{
			sError.SetSprintf ( "\"script_fields\" \"file\" property unsupported" );
			return false;
		}

		// add to query
		CSphQueryItem & tQueryItem = tQuery.m_dItems.Add();
		tQueryItem.m_sExpr = sExpr;
		tQueryItem.m_sAlias = pAlias->string;

		// add to select list
		sSelect.Appendf ( ", %s as %s", tQueryItem.m_sExpr.cstr(), tQueryItem.m_sAlias.cstr() );
	}

	tQuery.m_sSelect = sSelect.cstr();
	return true;
}
