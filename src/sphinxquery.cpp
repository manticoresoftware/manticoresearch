//
// $Id$
//

#include "sphinx.h"
#include "sphinxquery.h"
#include "sphinxutils.h"
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
// EXTENEDED PARSER RELOADED
//////////////////////////////////////////////////////////////////////////

typedef CSphExtendedQueryNode XQNode_t;
#include "sphinxqueryyy.hpp"

//////////////////////////////////////////////////////////////////////////

class XQParser_t
{
public:
					XQParser_t ();
					~XQParser_t () {}

public:
	bool			Parse ( CSphExtendedQuery & tQuery, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict );

	bool			Error ( const char * sTemplate, ... );
	void			Warning ( const char * sTemplate, ... );

	bool			AddField ( DWORD & uFields, const char * szField, int iLen );
	bool			ParseFields ( DWORD & uFields, int & iMaxFieldPos );

	bool			IsSpecial ( char c );
	int				GetToken ( YYSTYPE * lvalp );

	void			AddQuery ( XQNode_t * pNode );
	XQNode_t *		AddKeyword ( const char * sKeyword );
	XQNode_t *		AddKeywordFromInt ( int iValue, bool bKeyword );
	XQNode_t *		AddKeyword ( XQNode_t * pLeft, XQNode_t * pRight );
	XQNode_t *		AddOp ( ESphExtendedQueryOperator eOp, XQNode_t * pLeft, XQNode_t * pRight );

	void			Cleanup ();
	XQNode_t *		SweepNulls ( XQNode_t * pNode );
	bool			FixupNots ( XQNode_t * pNode );

public:
	CSphExtendedQuery *		m_pParsed;

	BYTE *					m_sQuery;
	int						m_iQueryLen;
	const char *			m_pLastTokenStart;

	const CSphSchema *		m_pSchema;
	ISphTokenizer *			m_pTokenizer;
	CSphDict *				m_pDict;

	const char *			m_pCur;

	CSphVector<XQNode_t*>	m_dSpawned;
	XQNode_t *				m_pRoot;

	bool					m_bStopOnInvalid;
	int						m_iAtomPos;

	int						m_iPendingNulls;
	int						m_iPendingType;
	YYSTYPE					m_tPendingToken;

	int						m_iGotTokens;
};

//////////////////////////////////////////////////////////////////////////

int yylex ( YYSTYPE * lvalp, XQParser_t * pParser )
{
	return pParser->GetToken ( lvalp );
}

void yyerror ( XQParser_t * pParser, const char * sMessage )
{
	pParser->m_pParsed->m_sParseError.SetSprintf ( "%s near '%s'", sMessage, pParser->m_pLastTokenStart );
}

#include "sphinxqueryyy.cpp"

//////////////////////////////////////////////////////////////////////////

void CSphExtendedQueryNode::SetFieldSpec ( DWORD uMask, int iMaxPos )
{
	// set it, if we do not yet have one
	if ( !m_bFieldSpec )
	{
		m_bFieldSpec = true;
		m_uFieldMask = uMask;
		m_iFieldMaxPos = iMaxPos;

		if ( IsPlain() )
		{
			m_tAtom.m_uFields = uMask;
			m_tAtom.m_iMaxFieldPos = iMaxPos;
		}
	}

	// some of the children might not yet have a spec, even if the node itself has
	// eg. 'hello @title world' (whole node has '@title' spec but 'hello' node does not have any!)
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->SetFieldSpec ( uMask, iMaxPos );
}

//////////////////////////////////////////////////////////////////////////

XQParser_t::XQParser_t ()
	: m_pParsed ( NULL )
	, m_pLastTokenStart ( NULL )
	, m_bStopOnInvalid ( true )
	, m_pRoot ( NULL )
{
}


/// cleanup spawned nodes (for bailing out on errors)
void XQParser_t::Cleanup ()
{
	ARRAY_FOREACH ( i, m_dSpawned )
	{
		m_dSpawned[i]->m_dChildren.Reset ();
		SafeDelete ( m_dSpawned[i] );
	}
	m_dSpawned.Reset ();
}



bool XQParser_t::Error ( const char * sTemplate, ... )
{
	assert ( m_pParsed );
	char sBuf[256];

	const char * sPrefix = "query error: ";
	int iPrefix = strlen(sPrefix);
	strcpy ( sBuf, sPrefix );

	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf+iPrefix, sizeof(sBuf)-iPrefix, sTemplate, ap );
	va_end ( ap );

	m_pParsed->m_sParseError = sBuf;
	return false;
}


void XQParser_t::Warning ( const char * sTemplate, ... )
{
	assert ( m_pParsed );
	char sBuf[256];

	const char * sPrefix = "query warning: ";
	int iPrefix = strlen(sPrefix);
	strcpy ( sBuf, sPrefix );

	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf+iPrefix, sizeof(sBuf)-iPrefix, sTemplate, ap );
	va_end ( ap );

	m_pParsed->m_sParseWarning = sBuf;
}


/// my special chars
bool XQParser_t::IsSpecial ( char c )
{
	return c=='(' || c==')' || c=='|' || c=='-' || c=='!' || c=='@' || c=='~' || c=='"' || c=='/';
}


/// lookup field and add it into mask
bool XQParser_t::AddField ( DWORD & uFields, const char * szField, int iLen )
{
	CSphString sField;
	sField.SetBinary ( szField, iLen );

	int iField = m_pSchema->GetFieldIndex ( sField.cstr () );
	if ( iField < 0 )
	{
		if ( m_bStopOnInvalid )
			return Error ( "no field '%s' found in schema", sField.cstr () );
		else
			Warning ( "no field '%s' found in schema", sField.cstr () );
	}
	else
	{
		if ( iField >= 32 )
			return Error ( " max 32 fields allowed" );

		uFields |= 1 << iField;
	}

	return true;
}


/// parse fields block
bool XQParser_t::ParseFields ( DWORD & uFields, int & iMaxFieldPos )
{
	uFields = 0;
	iMaxFieldPos = 0;

	const char * pPtr = m_pTokenizer->GetBufferPtr ();
	const char * pLastPtr = m_pTokenizer->GetBufferEnd ();

	if ( pPtr==pLastPtr )
		return true; // silently ignore trailing field operator

	bool bNegate = false;
	bool bBlock = false;

	// handle special modifiers
	if ( *pPtr=='!' )
	{
		// handle @! and @!(
		bNegate = true; pPtr++;
		if ( *pPtr=='(' ) { bBlock = true; pPtr++; }

	} else if ( *pPtr=='*' )
	{
		// handle @*
		uFields = 0xFFFFFFFF;
		m_pTokenizer->SetBufferPtr ( pPtr+1 );
		return true;

	} else if ( *pPtr=='(' )
	{
		// handle @(
		bBlock = true; pPtr++;
	}

	// handle invalid chars
	if ( !sphIsAlpha(*pPtr) )
	{
		m_pTokenizer->SetBufferPtr ( pPtr ); // ignore and re-parse (FIXME! maybe warn?)
		return true;
	}
	assert ( sphIsAlpha(*pPtr) ); // i think i'm paranoid

	// handle field specification
	if ( !bBlock )
	{
		// handle standalone field specification
		const char * pFieldStart = pPtr;
		while ( sphIsAlpha(*pPtr) && pPtr<pLastPtr )
			pPtr++;

		assert ( pPtr-pFieldStart>0 );
		if ( !AddField ( uFields, pFieldStart, pPtr-pFieldStart ) )
			return false;

		m_pTokenizer->SetBufferPtr ( pPtr );
		if ( bNegate && uFields )
			uFields = ~uFields;

	} else
	{
		// handle fields block specification
		assert ( sphIsAlpha(*pPtr) && bBlock ); // and complicated

		bool bOK = false;
		const char * pFieldStart = NULL;
		while ( pPtr<pLastPtr )
		{
			// accumulate field name, while we can
			if ( sphIsAlpha(*pPtr) )
			{
				if ( !pFieldStart )
					pFieldStart = pPtr;
				pPtr++;
				continue;
			}

			// separator found
			if ( pFieldStart==NULL )
			{
				return Error ( "separator without preceding field name in field block operator", *pPtr );

			} if ( *pPtr==',' )
			{
				if ( !AddField ( uFields, pFieldStart, pPtr-pFieldStart ) )
					return false;

				pFieldStart = NULL;
				pPtr++;

			} else if ( *pPtr==')' )
			{
				if ( !AddField ( uFields, pFieldStart, pPtr-pFieldStart ) )
					return false;

				m_pTokenizer->SetBufferPtr ( ++pPtr );
				if ( bNegate && uFields )
					uFields = ~uFields;

				bOK = true;
				break;

			} else
			{
				return Error ( "invalid character '%c' in field block operator", *pPtr );
			}
		}
		if ( !bOK )
			return Error ( "missing closing ')' in field block operator" );
	}

	// handle optional position range modifier
	if ( pPtr[0]=='[' && isdigit(pPtr[1]) )
	{
		// skip '[' and digits
		const char * p = pPtr+1;
		while ( *p && isdigit(*p) ) p++;

		// check that the range ends with ']' (FIXME! maybe report an error if it does not?)
		if ( *p!=']' )
			return true;

		// fetch my value
		iMaxFieldPos = strtoul ( pPtr+1, NULL, 10 );
		m_pTokenizer->SetBufferPtr ( p+1 );
	}

	// well done
	return true;
}


/// a lexer of my own
int XQParser_t::GetToken ( YYSTYPE * lvalp )
{
	// what, noone's pending for a bending?!
	if ( !m_iPendingType )
		for ( ;; )
	{
		assert ( m_iPendingNulls==0 );

		// tricky stuff
		// we need to manually check for numbers in certain states (currently, just after proximity or quorum operator)
		// required because if 0-9 are not in charset_table, or min_word_len is too high,
		// the tokenizer will *not* return the number as a token!
		m_pLastTokenStart = m_pTokenizer->GetBufferPtr ();
		const char * sEnd = m_pTokenizer->GetBufferEnd ();

		const char * p = m_pLastTokenStart;
		while ( p<sEnd && isspace(*(BYTE*)p) ) p++; // to avoid CRT assertions on Windows

		const char * sToken = p;
		while ( p<sEnd && isdigit(*(BYTE*)p) ) p++;

		if ( p>sToken && ( *p=='\0' || isspace(*(BYTE*)p) || IsSpecial(*p) ) )
		{
			// got a number followed by a whitespace or special, handle it
			char sNumberBuf[16];

			int iNumberLen = Min ( sizeof(sNumberBuf)-1, p-sToken );
			memcpy ( sNumberBuf, sToken, iNumberLen );
			sNumberBuf[iNumberLen] = '\0';
			m_tPendingToken.tInt.iValue = atoi ( sNumberBuf );

			// check if it can be used as a keyword too
			m_pTokenizer->SetBuffer ( (BYTE*)sNumberBuf, iNumberLen );
			sToken = (const char*) m_pTokenizer->GetToken();
			m_pTokenizer->SetBuffer ( m_sQuery, m_iQueryLen );
			m_pTokenizer->SetBufferPtr ( p );

			m_tPendingToken.tInt.bKeyword = ( sToken && m_pDict->GetWordID((BYTE*)sToken) );
			if ( sToken )
				m_iAtomPos++;

			m_iPendingNulls = 0;
			m_iPendingType = TOK_INT;
			break;
		}

		// not a number, or not followed by a whitespace, so fallback
		sToken = (const char *) m_pTokenizer->GetToken ();
		if ( !sToken )
			return 0;

		m_iPendingNulls = m_pTokenizer->GetOvershortCount ();
		m_iAtomPos += 1+m_iPendingNulls;

		if ( m_pTokenizer->WasTokenSpecial() )
		{
			// specials must not affect pos
			m_iAtomPos--;

			// return special token
			if ( sToken[0]!='@' )
			{
				m_iPendingType = sToken[0];
				break;
			}

			// parse fields operator
			if ( !ParseFields ( m_tPendingToken.tFieldLimit.uMask, m_tPendingToken.tFieldLimit.iMaxPos ) )
				return -1;

			if ( m_pSchema->m_dFields.GetLength()!=32 )
				m_tPendingToken.tFieldLimit.uMask &= ( 1UL<<m_pSchema->m_dFields.GetLength() )-1;

			m_iPendingType = TOK_FIELDLIMIT;
			break;
		}

		// check for stopword, and create that node
		CSphString sTmp ( sToken );
		if ( !m_pDict->GetWordID ( (BYTE*)sTmp.cstr() ) ) sToken = NULL;
		m_tPendingToken.pNode = AddKeyword ( sToken );
		m_iPendingType = TOK_KEYWORD;
		break;
	}

	// someone must be pending now!
	assert ( m_iPendingType );
	m_iGotTokens++;

	// ladies first, though
	if ( m_iPendingNulls>0 )
	{
		m_iPendingNulls--;
		lvalp->pNode = AddKeyword ( NULL );
		return TOK_KEYWORD;
	}

	// pending the offending 
	int iRes = m_iPendingType;
	m_iPendingType = 0;

	*lvalp = m_tPendingToken;
	return iRes;
}


void XQParser_t::AddQuery ( XQNode_t * pNode )
{
	m_pRoot = pNode;
}


XQNode_t * XQParser_t::AddKeyword ( const char * sKeyword )
{
	CSphExtendedQueryAtomWord tAW ( sKeyword, m_iAtomPos );

	XQNode_t * pNode = new XQNode_t();
	pNode->m_tAtom.m_dWords.Add ( tAW );

	m_dSpawned.Add ( pNode );
	return pNode;
}


XQNode_t * XQParser_t::AddKeywordFromInt ( int iValue, bool bKeyword )
{
	if ( !bKeyword )
		return AddKeyword ( NULL );

	char sBuf[16];
	snprintf ( sBuf, sizeof(sBuf), "%d", iValue );
	return AddKeyword ( sBuf );
}

XQNode_t * XQParser_t::AddKeyword ( XQNode_t * pLeft, XQNode_t * pRight )
{
	if ( !pLeft || !pRight )
		return pLeft ? pLeft : pRight;

	assert ( pLeft->IsPlain() );
	assert ( pRight->IsPlain() );
	assert ( pRight->m_tAtom.m_dWords.GetLength()==1 );

	pLeft->m_tAtom.m_dWords.Add ( pRight->m_tAtom.m_dWords[0] );
	m_dSpawned.RemoveValue ( pRight );
	SafeDelete ( pRight );
	return pLeft;
}


XQNode_t * XQParser_t::AddOp ( ESphExtendedQueryOperator eOp, XQNode_t * pLeft, XQNode_t * pRight )
{
	/////////
	// unary
	/////////

	if ( eOp==SPH_QUERY_NOT )
	{
		XQNode_t * pNode = new XQNode_t();
		pNode->m_dChildren.Add ( pLeft );
		pNode->m_eOp = eOp;
		return pNode;
	}

	//////////
	// binary
	//////////

	if ( !pLeft || !pRight )
		return pLeft ? pLeft : pRight;	

	// left spec always tries to infect the nodes to the right, only brackets can stop it
	// eg. '@title hello' vs 'world'
	if ( pLeft->m_bFieldSpec )
		pRight->SetFieldSpec ( pLeft->m_uFieldMask, pLeft->m_iFieldMaxPos );

	// build a new node
	XQNode_t * pResult = NULL;
	if ( !pLeft->IsPlain() && pLeft->m_eOp==eOp )
	{
		pLeft->m_dChildren.Add ( pRight );
		pResult = pLeft;
	} else
	{
		XQNode_t * pNode = new XQNode_t();
		pNode->m_dChildren.Add ( pLeft );
		pNode->m_dChildren.Add ( pRight );
		pNode->m_eOp = eOp;
		pResult = pNode;
	}

	// however, it's right (!) spec which is chosen for the resulting node,
	// eg. '@title hello' + 'world @body program'
	if ( pRight->m_bFieldSpec )
	{
		pResult->m_bFieldSpec = true;
		pResult->m_uFieldMask = pRight->m_uFieldMask;
		pResult->m_iFieldMaxPos = pRight->m_iFieldMaxPos;
	}

	return pResult;
}


XQNode_t * XQParser_t::SweepNulls ( XQNode_t * pNode )
{
	if ( !pNode )
		return NULL;

	// sweep plain node
	if ( pNode->IsPlain() )
	{
		ARRAY_FOREACH ( i, pNode->m_tAtom.m_dWords )
			if ( pNode->m_tAtom.m_dWords[i].m_sWord.cstr()==NULL )
				pNode->m_tAtom.m_dWords.Remove ( i-- );

		if ( pNode->m_tAtom.m_dWords.GetLength()==0 )
		{
			m_dSpawned.RemoveValue ( pNode ); // OPTIMIZE!
			SafeDelete ( pNode );
			return NULL;
		}

		return pNode;
	}

	// sweep op node
	ARRAY_FOREACH ( i, pNode->m_dChildren )
	{	
		pNode->m_dChildren[i] = SweepNulls ( pNode->m_dChildren[i] );
		if ( pNode->m_dChildren[i]==NULL )
			pNode->m_dChildren.Remove ( i-- );
	}

	if ( pNode->m_dChildren.GetLength()==0 )
	{
		m_dSpawned.RemoveValue ( pNode ); // OPTIMIZE!
		SafeDelete ( pNode );
		return NULL;
	}

	// remove redundancies if needed
	if ( pNode->m_eOp!=SPH_QUERY_NOT && pNode->m_dChildren.GetLength()==1 )
	{
		XQNode_t * pRet = pNode->m_dChildren[0];
		pNode->m_dChildren.Reset ();

		m_dSpawned.RemoveValue ( pNode ); // OPTIMIZE!
		SafeDelete ( pNode );
		return pRet;
	}

	// done
	return pNode;
}


bool XQParser_t::FixupNots ( XQNode_t * pNode )
{
	// no processing for plain nodes
	if ( !pNode || pNode->IsPlain() )
		return true;

	// process 'em children
	ARRAY_FOREACH ( i, pNode->m_dChildren )
		if ( !FixupNots ( pNode->m_dChildren[i] ) )
			return false;

	// extract NOT subnodes
	CSphVector<XQNode_t*> dNots;
	ARRAY_FOREACH ( i, pNode->m_dChildren )
		if ( pNode->m_dChildren[i]->m_eOp==SPH_QUERY_NOT )
	{
		dNots.Add ( pNode->m_dChildren[i] );
		pNode->m_dChildren.RemoveFast ( i-- );
	}

	// no NOTs? we're square
	if ( !dNots.GetLength() )
		return true;

	// nothing but NOTs? we can't compute that
	if ( !pNode->m_dChildren.GetLength() )
	{
		m_pParsed->m_sParseError.SetSprintf ( "query is non-computable (node consists of NOT operators only)" );
		return false;
	}

	// NOT within OR? we can't compute that
	if ( pNode->m_eOp==SPH_QUERY_OR )
	{
		m_pParsed->m_sParseError.SetSprintf ( "query is non-computable (NOT is not allowed within OR)" );
		return false;
	}

	// must be some NOTs within AND at this point, convert this node to ANDNOT
	assert ( pNode->m_eOp==SPH_QUERY_AND && pNode->m_dChildren.GetLength() && dNots.GetLength() );

	XQNode_t * pAnd = new XQNode_t();
	m_dSpawned.Add ( pAnd );
	pAnd->m_eOp = SPH_QUERY_AND;
	pAnd->m_dChildren = pNode->m_dChildren;

	XQNode_t * pNot = NULL;
	if ( dNots.GetLength()==1 )
	{
		pNot = dNots[0];
	} else
	{
		pNot = new XQNode_t();
		m_dSpawned.Add ( pNot );
		pNot->m_eOp = SPH_QUERY_OR;
		pNot->m_dChildren = dNots;
	}

	pNode->m_eOp = SPH_QUERY_ANDNOT;
	pNode->m_dChildren.Reset ();
	pNode->m_dChildren.Add ( pAnd );
	pNode->m_dChildren.Add ( pNot );
	return true;
}


static void DeleteNodesWOFields ( CSphExtendedQueryNode * pNode )
{
	if ( !pNode )
		return;

	for ( int i = 0; i < pNode->m_dChildren.GetLength (); )
	{
		if ( pNode->m_dChildren [i]->m_tAtom.m_uFields == 0 )
		{
			// this should be a leaf node
			assert ( pNode->m_dChildren [i]->m_dChildren.GetLength () == 0 );
			SafeDelete ( pNode->m_dChildren [i] );
			pNode->m_dChildren.RemoveFast ( i );
		}
		else
		{
			DeleteNodesWOFields ( pNode->m_dChildren [i] );
			i++;
		}
	}
}


bool XQParser_t::Parse ( CSphExtendedQuery & tParsed, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict )
{
	CSphScopedPtr<ISphTokenizer> pMyTokenizer ( pTokenizer->Clone ( true ) );
	pMyTokenizer->AddSpecials ( "()|-!@~\"/" );
	pMyTokenizer->EnableQueryParserMode ( true );

	m_pParsed = &tParsed;
	m_sQuery = (BYTE*) sQuery;
	m_iQueryLen = strlen(sQuery);
	m_pTokenizer = pMyTokenizer.Ptr();
	m_pSchema = pSchema;
	m_pDict = pDict;
	m_pCur = sQuery;
	m_iAtomPos = 0;
	m_iPendingNulls = 0;
	m_iPendingType = 0;
	m_pRoot = NULL;
	m_iGotTokens = 0;

	m_pTokenizer->SetBuffer ( m_sQuery, m_iQueryLen );
	int iRes = yyparse ( this );

	if ( iRes && m_iGotTokens )
	{
		Cleanup ();
		return false;
	}

	DeleteNodesWOFields ( m_pRoot );
	m_pRoot = SweepNulls ( m_pRoot );

	if ( !FixupNots ( m_pRoot ) )
	{
		Cleanup ();
		return false;
	}

	if ( m_pRoot && m_pRoot->m_eOp==SPH_QUERY_NOT )
	{
		m_pParsed->m_sParseError.SetSprintf ( "query is non-computable (single NOT operator)" );
		return false;
	}

	m_dSpawned.Reset();
	if ( m_pRoot )
	{
		SafeDelete ( tParsed.m_pRoot );
		tParsed.m_pRoot = m_pRoot;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

#define XQDEBUG 0

#if XQDEBUG
static void xqIndent ( int iIndent )
{
	iIndent *= 2;
	while ( iIndent--)
		printf ( " " );
}


static void xqDump ( CSphExtendedQueryNode * pNode, const CSphSchema & tSch, int iIndent )
{
	if ( pNode->m_tAtom.IsEmpty() )
	{
		xqIndent ( iIndent );
		switch ( pNode->m_eOp )
		{
			case SPH_QUERY_AND: printf ( "AND:\n" ); break;
			case SPH_QUERY_OR: printf ( "OR:\n" ); break;
			case SPH_QUERY_NOT: printf ( "NOT:\n" ); break;
			case SPH_QUERY_ANDNOT: printf ( "ANDNOT:\n" ); break;
		}
		ARRAY_FOREACH ( i, pNode->m_dChildren )
			xqDump ( pNode->m_dChildren[i], tSch, iIndent+1 );
	} else
	{
		const CSphExtendedQueryAtom & tAtom = pNode->m_tAtom;
		xqIndent ( iIndent );
		printf ( "MATCH(%d,%d):",
			tAtom.m_uFields,
			tAtom.m_iMaxDistance );
		ARRAY_FOREACH ( i, tAtom.m_dWords )
			printf ( " %s (pos %d)", tAtom.m_dWords[i].m_sWord.cstr(), tAtom.m_dWords[i].m_iAtomPos );
		printf ( "\n" );
	}
}
#endif


bool sphParseExtendedQuery ( CSphExtendedQuery & tParsed, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict )
{
	XQParser_t qp;
	bool bRes = qp.Parse ( tParsed, sQuery, pTokenizer, pSchema, pDict );

#if XQDEBUG
	if ( bRes )
	{
		printf ( "--- accept ---\n" );
		xqDump ( tParsed.m_pAccept, *pSchema, 0 );
		printf ( "--- reject ---\n" );
		xqDump ( tParsed.m_pReject, *pSchema, 0 );
		printf ( "---\n" );
	}
#endif

	return bRes;
}

//
// $Id$
//
