//
// $Id$
//

#include "sphinx.h"
#include "sphinxquery.h"
#include "sphinxutils.h"
#include <stdarg.h>

/////////////////////////////////////////////////////////////////////////////
// CSphBooleanQueryExpr
/////////////////////////////////////////////////////////////////////////////

CSphBooleanQueryExpr::CSphBooleanQueryExpr ()
	: m_eType ( NODE_UNDEF )
	, m_pExpr ( NULL )
	, m_pPrev ( NULL )
	, m_pNext ( NULL )
	, m_pParent ( NULL )
	, m_bInvert ( false )
	, m_bEvaluable ( false )
{
}


CSphBooleanQueryExpr::~CSphBooleanQueryExpr ()
{
	SafeDelete ( m_pExpr );
	SafeDelete ( m_pNext );
}

void CSphBooleanQueryExpr::Detach ()
{
	// fixup parent
	if ( m_pParent )
		if ( m_pParent->m_pExpr==this )
	{
		assert ( !m_pPrev );
		m_pParent->m_pExpr = m_pNext;
	}

	// fixup siblings
	if ( m_pPrev )
	{
		assert ( m_pPrev->m_pNext==this );
		m_pPrev->m_pNext = m_pNext;
	}
	if ( m_pNext )
	{
		assert ( m_pNext->m_pPrev==this );
		m_pNext->m_pPrev = m_pPrev;
	}

	// fixup subexpressions
	assert ( !m_pExpr );

	// i'm all detached now
	m_pParent = m_pPrev = m_pNext = m_pExpr = NULL;
}


CSphBooleanQueryExpr * CSphBooleanQueryExpr::NewTail ()
{
	CSphBooleanQueryExpr * pNew = new CSphBooleanQueryExpr ();
	pNew->m_pPrev = this;
	pNew->m_pParent = m_pParent;

	assert ( !m_pNext );
	m_pNext = pNew;

	return pNew;
}


bool CSphBooleanQueryExpr::IsNull ()
{
	bool bRes = ( m_pExpr==NULL && m_sWord.cstr()==NULL );
	assert ( !( bRes && m_pNext ) );
	return bRes;
}


bool CSphBooleanQueryExpr::IsAlone ()
{
	return ( !m_pNext && !m_pPrev );
}

/////////////////////////////////////////////////////////////////////////////
// CSphBooleanQueryParser implementation
/////////////////////////////////////////////////////////////////////////////

class CSphBooleanQueryParser
{
public:
	CSphBooleanQueryExpr *	Parse ( const char * sQuery, const ISphTokenizer * pTokenizer );

protected:
	CSphBooleanQueryExpr *	m_pRoot;
	CSphBooleanQueryExpr *	m_pCur;

protected:
	void					HandleOperator ( int iCh );
};


void CSphBooleanQueryParser::HandleOperator ( int iCh )
{
	assert ( iCh=='|' || iCh=='&' );
	assert ( m_pCur->IsNull() );

	// there were no words preceding the operator, so ignore it
	if ( !m_pCur->m_pPrev )
		return;

	// what type is it?
	ESphBooleanQueryExpr eNewType = ( iCh=='|' ) ? NODE_OR : NODE_AND;

	// there's only one preceding word, so we can change the type
	if ( !m_pCur->m_pPrev->m_pPrev )
	{
		assert ( !m_pCur->m_pPrev->IsNull() );
		m_pCur->m_pPrev->m_eType = eNewType;
		m_pCur->m_eType = eNewType;
		return;
	}

	// current type matches, just propagate it to the placeholder
	if ( m_pCur->m_pPrev->m_eType==eNewType )
	{
		m_pCur->m_eType = eNewType;
		return;
	}

	// current type does not match, spawn another tree level
	CSphBooleanQueryExpr * pNew = new CSphBooleanQueryExpr ();
	pNew->m_eType = eNewType;

	if ( eNewType==NODE_OR )
	{
		// OR priority is higher, so we detach the last word and make a new child
		assert ( m_pCur->IsNull() );
		assert ( m_pCur->m_pPrev );
		assert ( m_pCur->m_pPrev->m_pPrev );

		// make a new child
		pNew->m_pExpr = m_pCur->m_pPrev;

		// detach last word, and reattach it as a new subexpression instead
		pNew->m_pPrev = m_pCur->m_pPrev->m_pPrev;
		pNew->m_pParent = m_pCur->m_pParent;
		m_pCur->m_pPrev->m_pPrev->m_pNext = pNew;
		m_pCur->m_pPrev->m_pPrev = NULL;

		m_pCur->m_pPrev->m_eType = NODE_OR;
		m_pCur->m_eType = NODE_OR;
		pNew->m_eType = NODE_AND;

		m_pCur->m_pPrev->m_pParent = pNew;
		m_pCur->m_pParent = pNew;

	} else
	{
		if ( m_pCur->m_pParent )
		{
			// got parent? its type must be AND (or UNDEF yet), so plug the current holder there
			assert ( m_pCur->m_pPrev );
			if ( m_pCur->m_pParent->m_eType==NODE_UNDEF )
				m_pCur->m_pParent->m_eType = NODE_AND;
			assert ( m_pCur->m_pParent->m_eType==NODE_AND );

			CSphBooleanQueryExpr * pPar = m_pCur->m_pParent;
			while ( pPar->m_pNext )
				pPar = pPar->m_pNext;
			pPar->m_pNext = m_pCur;

			m_pCur->m_pPrev->m_pNext = NULL;
			m_pCur->m_pPrev = pPar;

			m_pCur->m_eType = NODE_AND;
			m_pCur->m_pParent = m_pCur->m_pParent->m_pParent;

		} else
		{
			// no parent? change the root
			CSphBooleanQueryExpr * pNew = new CSphBooleanQueryExpr ();
			pNew->m_eType = NODE_AND;

			pNew->m_pExpr = m_pRoot;
			m_pRoot = pNew;

			// close old expr, make a new placeholder
			bool bInv = m_pCur->m_bInvert;
			m_pCur->Detach ();
			SafeDelete ( m_pCur );
			m_pCur = pNew->NewTail ();
			m_pCur->m_bInvert = bInv;

			// fixup old expr parents
			for ( CSphBooleanQueryExpr * pTmp = pNew->m_pExpr; pTmp; pTmp = pTmp->m_pNext )
				pTmp->m_pParent = pNew;
		}
	}

	// whatever happens, my new type must be this
	m_pCur->m_eType = eNewType;
}


CSphBooleanQueryExpr * CSphBooleanQueryParser::Parse ( const char * sQuery, const ISphTokenizer * pTokenizer )
{
	assert ( sQuery );
	assert ( pTokenizer );

	m_pRoot = new CSphBooleanQueryExpr ();
	m_pCur = m_pRoot;

	// a buffer of my own
	CSphString sBuffer ( sQuery );

	CSphScopedPtr<ISphTokenizer> pMyTokenizer ( pTokenizer->Clone ( true ) );
	pMyTokenizer->AddSpecials ( "&|()-!" );
	pMyTokenizer->SetBuffer ( (BYTE*)sBuffer.cstr(), strlen ( sBuffer.cstr() ) );
	pMyTokenizer->EnableQueryParserMode ( true );

	// iterate all tokens
	const int QUERY_END = -1;

	int iLastExplicit = 0;
	BYTE * pToken;

	for ( ;; )
	{
		pToken = pMyTokenizer->GetToken ();

		assert ( m_pCur );
		assert ( m_pCur->IsNull() );

		int iSpecial = pToken
			? ( pMyTokenizer->WasTokenSpecial () ? pToken[0] : 0 )
			: QUERY_END;
		assert ( !( iSpecial>0 && pToken[1]!=0 ) );

		// enforce explicit AND/OR; the last one in a row wins
		if ( iSpecial=='|' || iSpecial=='&' )
		{
			iLastExplicit = iSpecial;
			continue;
		}
		if ( iLastExplicit>0 )
		{
			HandleOperator ( iLastExplicit );
			iLastExplicit = 0;
		}

		// handle negation operator
		if ( iSpecial=='-' || iSpecial=='!' )
		{
			m_pCur->m_bInvert = true;
			continue;
		}

		// open subexpression
		if ( iSpecial=='(' )
		{
			if ( m_pCur->m_pPrev )
			{
				if ( m_pCur->m_pPrev->m_eType==NODE_UNDEF )
					HandleOperator ( '&' );
				m_pCur->m_eType = m_pCur->m_pPrev->m_eType;
			}

			m_pCur->m_pExpr = new CSphBooleanQueryExpr ();
			m_pCur->m_pExpr->m_pParent = m_pCur;
			m_pCur = m_pCur->m_pExpr;
			continue;
		}

		// close expression
		if ( iSpecial==QUERY_END || iSpecial==')' )
		{
			if ( iSpecial==')' && !m_pCur->m_pParent )
				continue;

			// kill current placeholder
			assert ( m_pCur->IsNull() );

			bool bDone = false;
			for ( ;; )
			{
				if ( !m_pCur->m_pPrev )
				{
					// empty root? let's return NULL
					if ( m_pCur==m_pRoot )
					{
						m_pCur->Detach ();
						SafeDelete ( m_pCur );
						m_pRoot = NULL;
						return NULL;
					}

					// it's empty subexpression, fix
					assert ( !m_pCur->m_pNext );
					assert ( m_pCur->m_pParent );

					m_pCur = m_pCur->m_pParent;
					m_pCur->m_eType = NODE_UNDEF;
					m_pCur->m_pExpr->Detach ();
					SafeDelete ( m_pCur->m_pExpr );
					assert ( m_pCur->IsNull() );

					if ( iSpecial!=QUERY_END )
						bDone = true;
				} else
				{
					// just chop it off the end
					CSphBooleanQueryExpr * pToKill = m_pCur;
					m_pCur = m_pCur->m_pPrev;
					pToKill->Detach ();
					SafeDelete ( pToKill );
				}

				if ( iSpecial!=QUERY_END || !m_pCur || !m_pCur->IsNull() )
					break;
			}
			if ( iSpecial==QUERY_END )
				break;
			if ( bDone )
				continue;

			// go up level and make a new placeholder
			assert ( iSpecial==')' );
			m_pCur = m_pCur->m_pParent->NewTail ();
			continue;
		}

		// enforce implicit AND
		if ( m_pCur->m_pPrev && m_pCur->m_eType==NODE_UNDEF )
			HandleOperator ( '&' );

		// it shoul be a word, set it
		assert ( iSpecial==0 );
		assert ( strlen((const char*)pToken) );
		assert ( m_pCur->IsNull() );
		m_pCur->m_sWord = (const char*)pToken;
		m_pCur = m_pCur->NewTail ();
	}

	// done
	return m_pRoot;
}

/////////////////////////////////////////////////////////////////////////////
// Query simplification
/////////////////////////////////////////////////////////////////////////////

static void InvertExprLevel ( CSphBooleanQueryExpr * pNode )
{
	assert ( pNode->m_eType!=NODE_UNDEF );
	assert ( pNode->m_pPrev==NULL );

#ifndef NDEBUG
	ESphBooleanQueryExpr eOldType = pNode->m_eType;
#endif
	ESphBooleanQueryExpr eNewType = ( pNode->m_eType==NODE_AND ) ? NODE_OR : NODE_AND;

	for ( CSphBooleanQueryExpr * pFix = pNode; pFix; pFix = pFix->m_pNext )
	{
		assert ( pFix->m_eType==eOldType );
		pFix->m_eType = eNewType;
		pFix->m_bInvert = !pFix->m_bInvert;
		pFix->m_bEvaluable = !pFix->m_bEvaluable;
	}
}


static CSphBooleanQueryExpr * RemoveRedundantNodes ( CSphBooleanQueryExpr * pNode )
{
	if ( !pNode )
		return NULL;

	// optimize my subexpression
	if ( pNode->m_pExpr )
	{
		pNode->m_pExpr = RemoveRedundantNodes ( pNode->m_pExpr );
		if ( !pNode->m_pExpr )
		{
			// if i'm optimized out, so be it
			pNode->Detach ();
			SafeDelete ( pNode );
			return NULL;
		}
		assert ( pNode->m_pExpr->m_pParent==pNode );
	}

	// update my type
	if ( pNode->m_eType==NODE_UNDEF )
	{
		assert ( pNode->IsAlone() );
		if ( pNode->m_pExpr )
			pNode->m_eType = pNode->m_pExpr->m_eType;
		else
			pNode->m_eType = NODE_AND;
	}
	assert ( pNode->m_eType!=NODE_UNDEF ); // all types must be defined now
	assert (!( pNode->m_pExpr && pNode->m_pExpr->m_eType==NODE_UNDEF )); // all types must be defined now
	assert (!( pNode->IsAlone() && !pNode->m_pExpr && pNode->m_eType!=NODE_AND )); // there must be no single words with type OR 

	// optimize redundant sublevels of matching type
	while ( pNode->m_pExpr
		&& !pNode->m_bInvert
		&& pNode->m_pExpr->m_eType==pNode->m_eType )
	{
		// for each sublevel node, pull it up
		CSphBooleanQueryExpr * pSubFirst = pNode->m_pExpr;
		CSphBooleanQueryExpr * pSubLast = NULL;
		for ( CSphBooleanQueryExpr * pSub = pNode->m_pExpr; pSub; pSub = pSub->m_pNext )
		{
			pSub->m_pParent = pNode->m_pParent;
			pSubLast = pSub;
		}
		assert ( pSubFirst );
		assert ( pSubLast );

		// chain sublevel start right after this node
		assert ( !pSubFirst->m_pPrev );
		pSubFirst->m_pPrev = pNode;

		assert ( !pSubLast->m_pNext );
		pSubLast->m_pNext = pNode->m_pNext;
		if ( pNode->m_pNext )
			pNode->m_pNext->m_pPrev = pSubLast;
		pNode->m_pNext = pSubFirst;

		// this node is no longer needed; sublevel start replaces it
		pNode->m_pExpr = NULL;
		pNode->Detach ();
		SafeDelete ( pNode );
		pNode = pSubFirst;
	}

	// optimze my siblings
	if ( pNode->m_pNext )
	{
		pNode->m_pNext = RemoveRedundantNodes ( pNode->m_pNext );
		assert ( pNode->m_pNext->m_pPrev==pNode );
	}

	return pNode;
}


// place filters at the tail
CSphBooleanQueryExpr * ReorderLevel ( CSphBooleanQueryExpr * pNode )
{
	assert ( !pNode->m_pPrev );

	// detach filters
	CSphBooleanQueryExpr * pFilters = NULL;
	for ( CSphBooleanQueryExpr * pCur = pNode; pCur; )
	{
		CSphBooleanQueryExpr * pNext = pCur->m_pNext;
		if ( !pCur->m_bEvaluable )
		{
			// unchain from main list, and fixup its head, too
			if ( pCur->m_pPrev )
				pCur->m_pPrev->m_pNext = pCur->m_pNext;
			else
				pNode = pCur->m_pNext;
			if ( pCur->m_pNext )
				pCur->m_pNext->m_pPrev = pCur->m_pPrev;

			// chain to the filters list head
			pCur->m_pPrev = NULL;
			pCur->m_pNext = pFilters;
			if ( pFilters )
			{
				assert ( !pFilters->m_pPrev );
				pFilters->m_pPrev = pCur;
			}
			pFilters = pCur;
		}
		pCur = pNext;
	}
	assert ( pNode );

	// reattach filters
	if ( pFilters )
	{
		// to the tail
		CSphBooleanQueryExpr * pTail = pNode;
		while ( pTail->m_pNext )
			pTail = pTail->m_pNext;

		// from the tail
		while ( pFilters->m_pNext )
			pFilters = pFilters->m_pNext;

		while ( pFilters )
		{
			assert ( !pTail->m_pNext );
			assert ( !pFilters->m_pNext );

			CSphBooleanQueryExpr * pPrev = pFilters->m_pPrev;
			if ( pPrev )
				pPrev->m_pNext = NULL;

			pTail->m_pNext = pFilters;
			pFilters->m_pPrev = pTail;
			pTail = pTail->m_pNext;
			pFilters = pPrev;
		}
	}

	return pNode;
}


static bool IsEvaluable ( CSphBooleanQueryExpr * pNode )
{
	// find out if this node is evaluable
	if ( !pNode->m_pExpr )
	{
		pNode->m_bEvaluable = !pNode->m_bInvert;
	} else
	{
		// return code might be different from pNode->m_pExpr->m_bEvaluable,
		// because the first node in a level might be evaluable,
		// while the whole level is not
		bool bRes = IsEvaluable ( pNode->m_pExpr );
		pNode->m_bEvaluable = bRes ^ pNode->m_bInvert;

		// if sublevel can be made directly evaluable, well, do it
		// if it can't, convert it to AND type
		if ( !bRes &&
			( pNode->m_bInvert || pNode->m_eType==NODE_OR ) )
		{
			InvertExprLevel ( pNode->m_pExpr );
			pNode->m_bInvert = !pNode->m_bInvert;
			bRes = !bRes;
		}

		// if sublevel is evaluable, reorder it's nodes
		if ( bRes )
			pNode->m_pExpr = ReorderLevel ( pNode->m_pExpr );
	}

	// if it's not level start, we're done
	if ( pNode->m_pPrev )
		return pNode->m_bEvaluable;

	// now, this node is a level start, so we need to check its type
	// and all the siblings to find out if the level is evaluable
#ifndef NDEBUG
	for ( CSphBooleanQueryExpr * pCur = pNode->m_pNext; pCur; pCur = pCur->m_pNext )
	{
		bool bRes = IsEvaluable ( pCur );
		assert ( bRes==pCur->m_bEvaluable );
	}
#else
	for ( CSphBooleanQueryExpr * pCur = pNode->m_pNext; pCur; pCur = pCur->m_pNext )
		IsEvaluable ( pCur );
#endif

	switch ( pNode->m_eType )
	{
		case NODE_AND:
			// there needs to be at least one directly evaluable node
			for ( CSphBooleanQueryExpr * pCur = pNode; pCur; pCur = pCur->m_pNext )
				if ( pCur->m_bEvaluable )
					return true;
			return false;

		case NODE_OR:
			// all the nodes need to be directly evaluable
			for ( CSphBooleanQueryExpr * pCur = pNode; pCur; pCur = pCur->m_pNext )
				if ( !pCur->m_bEvaluable )
					return false;
			return true;

		default:
			assert ( 0 && "INTERNAL ERROR: unhandled node type" );
			return false;
	}
}

/////////////////////////////////////////////////////////////////////////////
// Debugging stuff
/////////////////////////////////////////////////////////////////////////////

static void DumpTree ( CSphBooleanQueryExpr * pNode, int iLevel=0 )
{
	for ( CSphBooleanQueryExpr * pCur = pNode; pCur; pCur = pCur->m_pNext )
	{
		for ( int i=0; i<iLevel; i++ )
			printf ( "\t" );

		switch ( pCur->m_eType )
		{
			case NODE_AND:	printf ( "&& "); break;
			case NODE_OR:	printf ( "|| "); break;
			default:		printf ( "?? "); break;
		}
		printf ( "%c ", pCur->m_bEvaluable ? '+' : '-' );
		if ( pCur->m_bInvert )
			printf ( "!" );

		if ( !pCur->m_pExpr )
		{
			printf ( "%s\n", pCur->m_sWord.cstr() );
		} else
		{
			printf ( "\n" );
			DumpTree ( pCur->m_pExpr, iLevel+1 );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// BOOLEAN PARSER ENTRY POINT
/////////////////////////////////////////////////////////////////////////////

bool sphParseBooleanQuery ( CSphBooleanQuery & tParsed, const char * sQuery, const ISphTokenizer * pTokenizer )
{
	CSphBooleanQueryParser qp;

	tParsed.m_pTree = RemoveRedundantNodes ( qp.Parse ( sQuery, pTokenizer ) );
	if ( !tParsed.m_pTree )
		return true;

	if ( !IsEvaluable ( tParsed.m_pTree ) )
	{
		tParsed.m_sParseError = "non-evaluable boolean query (negation on top level)";
		SafeDelete ( tParsed.m_pTree );
		return false;
	}

	tParsed.m_pTree = ReorderLevel ( tParsed.m_pTree );
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// EXTENDED QUERY PARSER
/////////////////////////////////////////////////////////////////////////////

class CSphExtendedQueryParser
{
public:
						CSphExtendedQueryParser ();
	bool				Parse ( CSphExtendedQuery & tParsed, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict );

protected:
	struct CNodeStackEntry
	{
		CSphExtendedQueryNode *		m_pNode;
		bool						m_bAny;

		CNodeStackEntry ()
			: m_pNode ( NULL )
			, m_bAny ( false )
		{}

		CNodeStackEntry ( CSphExtendedQueryNode * pNode, bool bAny )
			: m_pNode ( pNode )
			, m_bAny ( bAny )
		{}
	};

protected:
	CSphExtendedQuery *				m_pRes;		///< result holder
	CSphVector<CNodeStackEntry>		m_dStack;	///< open nodes stack

protected:
	bool				m_bStopOnInvalid;		///< stop on invalid fields or skip them

	bool				Error ( const char * sTemplate, ... );
	void				Warning ( const char * sTemplate, ... );
	bool				ParseFields ( DWORD & uFields, ISphTokenizer * pTokenizer, const CSphSchema * pSchema );
	bool				AddField ( DWORD & uFields, const char * szField, int iLen, const CSphSchema * pSchema );

	void				PushNode ();					///< push new empty node onto stack
	void				PopNode ( bool bReject=false);	///< pop node off the stack to proper list
};


void CSphExtendedQueryNode::Sublevelize ( CSphExtendedQueryNode * & pNew, bool bAny )
{
	if ( m_dChildren.GetLength()==1 )
	{
		// degenerate case with only one child. can attach another one
		m_dChildren.Add ( pNew );
		m_bAny = bAny;

	} else
	{
		// a clone of my own
		CSphExtendedQueryNode * pClone = new CSphExtendedQueryNode ();
		pClone->m_pParent = this;
		pClone->m_tAtom = m_tAtom;
		pClone->m_bAny = m_bAny;
		pClone->m_dChildren = m_dChildren;

		// my data was just moved to clone
		m_tAtom.Reset ();
		m_dChildren.Reset ();

		// and i'm a sublevel parent now
		m_bAny = bAny;
		m_dChildren.Add ( pClone );
		m_dChildren.Add ( pNew );
	}
	pNew = NULL;
}


void CSphExtendedQueryNode::Submit ( CSphExtendedQueryNode * & pNew, bool bAny )
{
	assert ( pNew->m_pParent==NULL );

	// empty src node. do nothing
	if ( pNew->IsEmpty() )
	{
		SafeDelete ( pNew );
		return;
	}

	// if dst is empty or if logical op matches, add src node to dst children
	if ( IsEmpty() || ( !IsPlain() && m_bAny==bAny ) )
	{
		m_dChildren.Add ( pNew );
		pNew = NULL;
		return;
	}

	// incoming conjunction. merge plain dst and src nodes if we can
	if ( bAny==false
		&& IsPlain() && pNew->IsPlain() && m_tAtom.m_uFields==pNew->m_tAtom.m_uFields
		&& m_tAtom.m_iMaxDistance==-1 && pNew->m_tAtom.m_iMaxDistance==-1 )
	{
		ARRAY_FOREACH ( i, pNew->m_tAtom.m_dWords )
			m_tAtom.m_dWords.Add ( pNew->m_tAtom.m_dWords[i] );

		SafeDelete ( pNew );
		return;
	}

	// disjunction. detach last word/child if we can
	if ( bAny==true && (
		( IsPlain() && m_tAtom.m_dWords.GetLength()>1 && m_tAtom.m_iMaxDistance==-1 ) ||
		( !IsPlain() && m_dChildren.GetLength()>1 ) ) )
	{
		assert ( IsPlain() || m_bAny==false );

		// detach last word/child if we can, and build a new sublevel
		CSphExtendedQueryNode * pChop;
		if ( IsPlain() )
		{
			pChop = new CSphExtendedQueryNode ();
			pChop->m_pParent = this;
			pChop->m_tAtom.m_uFields = m_tAtom.m_uFields;
			pChop->m_tAtom.m_dWords.Add ( m_tAtom.m_dWords.Last() );
			m_tAtom.m_dWords.Resize ( m_tAtom.m_dWords.GetLength()-1 );
		} else
		{
			pChop = m_dChildren.Pop ();
		}

		pChop->Sublevelize ( pNew, true );
		assert ( !pChop->IsPlain() );

		Sublevelize ( pChop, false );
		return;
	}

	// in all the other cases, just make a new sublevel
	Sublevelize ( pNew, bAny );
}


CSphExtendedQueryParser::CSphExtendedQueryParser ()
	: m_bStopOnInvalid ( true )
{
}


bool CSphExtendedQueryParser::Error ( const char * sTemplate, ... )
{
	assert ( m_pRes );
	char sBuf[256];

	const char * sPrefix = "query error: ";
	int iPrefix = strlen(sPrefix);
	strcpy ( sBuf, sPrefix );

	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf+iPrefix, sizeof(sBuf)-iPrefix, sTemplate, ap );
	va_end ( ap );

	m_pRes->m_sParseError = sBuf;
	m_pRes = NULL;
	return false;
}


void CSphExtendedQueryParser::Warning ( const char * sTemplate, ... )
{
	assert ( m_pRes );
	char sBuf[256];

	const char * sPrefix = "query warning: ";
	int iPrefix = strlen(sPrefix);
	strcpy ( sBuf, sPrefix );

	va_list ap;
	va_start ( ap, sTemplate );
	vsnprintf ( sBuf+iPrefix, sizeof(sBuf)-iPrefix, sTemplate, ap );
	va_end ( ap );

	m_pRes->m_sParseWarning = sBuf;
}


void CSphExtendedQueryParser::PopNode ( bool bReject )
{
	assert ( m_dStack.GetLength()>0 );

	// fixup degenerate phrases
	CSphExtendedQueryAtom & tAtom = m_dStack.Last().m_pNode->m_tAtom;
	if ( tAtom.m_iMaxDistance>=0 && tAtom.m_dWords.GetLength()<=1 )
		tAtom.m_iMaxDistance = -1;

	// pop stack top
	if ( m_dStack.GetLength()>=2 )
	{
		// collapse last pair of nodes
		m_dStack[ m_dStack.GetLength()-2 ].m_pNode->Submit ( m_dStack.Last().m_pNode, m_dStack.Last().m_bAny );
	} else
	{
		// submit top-level expr to proper list
		assert ( m_pRes );
		if ( bReject )
		{
			assert ( m_dStack.Last().m_bAny==false );
			m_pRes->m_pReject->Submit ( m_dStack.Last().m_pNode, true );
		} else
		{
			m_pRes->m_pAccept->Submit ( m_dStack.Last().m_pNode, m_dStack.Last().m_bAny );
		}
	}

	m_dStack.Pop ();
}


void CSphExtendedQueryParser::PushNode ()
{
	m_dStack.Add ( CNodeStackEntry ( new CSphExtendedQueryNode(), false ) );
}


bool CSphExtendedQueryParser::AddField ( DWORD & uFields, const char * szField, int iLen, const CSphSchema * pSchema )
{
	CSphString sField;
	sField.SetBinary ( szField, iLen );

	int iField = pSchema->GetFieldIndex ( sField.cstr () );
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


bool CSphExtendedQueryParser::ParseFields ( DWORD & uFields, ISphTokenizer * pTokenizer, const CSphSchema * pSchema )
{
	uFields = 0;

	if ( m_dStack.GetLength() )
		return Error ( "field specification is only allowed at top level" );

	const char * pPtr = pTokenizer->GetBufferPtr ();
	const char * pLastPtr = pTokenizer->GetBufferEnd ();

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
		pTokenizer->SetBufferPtr ( pPtr+1 );
		return true;

	} else if ( *pPtr=='(' )
	{
		// handle @(
		bBlock = true; pPtr++;

	}

	// handle invalid chars
	if ( !sphIsAlpha(*pPtr) )
	{
		pTokenizer->SetBufferPtr ( pPtr ); // ignore and re-parse (FIXME! maybe warn?)
		return true;
	}
	assert ( sphIsAlpha(*pPtr) ); // i think i'm paranoid

	// handle standalone field specification
	if ( !bBlock )
	{
		const char * pFieldStart = pPtr;
		while ( sphIsAlpha(*pPtr) && pPtr<pLastPtr )
			pPtr++;

		assert ( pPtr-pFieldStart>0 );
		if ( !AddField ( uFields, pFieldStart, pPtr-pFieldStart, pSchema ) )
			return false;

		pTokenizer->SetBufferPtr ( pPtr );
		if ( bNegate && uFields )
			uFields = ~uFields;
		return true;
	}

	// handle fields block specification
	assert ( sphIsAlpha(*pPtr) && bBlock ); // and complicated

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
			if ( !AddField ( uFields, pFieldStart, pPtr-pFieldStart, pSchema ) )
				return false;

			pFieldStart = NULL;
			pPtr++;

		} else if ( *pPtr==')' )
		{
			if ( !AddField ( uFields, pFieldStart, pPtr-pFieldStart, pSchema ) )
				return false;

			pTokenizer->SetBufferPtr ( pPtr+1 );
			if ( bNegate && uFields )
				uFields = ~uFields;
			return true;

		} else
		{
			return Error ( "invalid character '%c' in field block operator", *pPtr );
		}
	}

	return Error ( "missing closing ')' in field block operator" );
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


bool CSphExtendedQueryParser::Parse ( CSphExtendedQuery & tParsed, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict )
{
	assert ( sQuery );
	assert ( pTokenizer );
	assert ( pSchema );

	// clean up
	assert ( m_dStack.GetLength()==0 );
	m_pRes = &tParsed;

	// check for relaxed syntax
	const char * OPTION_RELAXED = "@@relaxed";
	const int OPTION_RELAXED_LEN = strlen ( OPTION_RELAXED );

	m_bStopOnInvalid = true;
	if ( strncmp ( sQuery, OPTION_RELAXED, OPTION_RELAXED_LEN )==0 && !sphIsAlpha ( sQuery[OPTION_RELAXED_LEN]) )
	{
		sQuery += OPTION_RELAXED_LEN;
		m_bStopOnInvalid = false;
	}

	// a buffer of my own
	CSphString sBuffer ( sQuery );
	CSphScopedPtr<ISphTokenizer> pMyTokenizer ( pTokenizer->Clone ( true ) );
	pMyTokenizer->AddSpecials ( "()|-!@~\"/" );
	pMyTokenizer->EnableQueryParserMode ( true );
	pMyTokenizer->SetBuffer ( (BYTE*)sBuffer.cstr(), strlen ( sBuffer.cstr() ) );

	// iterate all tokens
	const int QUERY_END = -1;

	enum
	{
		XQS_TEXT		= 0,
		XQS_PHRASE		= 1,
		XQS_PHRASE_END	= 2,
		XQS_PROXIMITY	= 3,
		XQS_NEGATION	= 4,
		XQS_NEGTEXT		= 5,
		XQS_QUORUM		= 6
	};

	CSphVector<int> dState;
	dState.Add ( XQS_TEXT );

	bool bAny = false;
	DWORD uFields = 0xFFFFFFFF;
	int iAtomPos = 0;

	bool bRedo = false;
	const char * sToken = NULL;

	bool bSpecial = false;
	for ( ;; )
	{
		// get next token
		if ( !bRedo )
		{
			// tricky stuff
			// we need to manually check for numbers in "expect number" states
			// required because if 0-9 are not in charset_table, or min_word_len is too high,
			// the tokenizer will *not* return the number as a token!
			if ( dState.Last()==XQS_PROXIMITY || dState.Last()==XQS_QUORUM )
			{
				const char * sEnd = pMyTokenizer->GetBufferEnd ();

				const char * p = pMyTokenizer->GetBufferPtr ();
				while ( p<sEnd && isspace(*p) ) p++;

				sToken = p;
				while ( p<sEnd && isdigit(*p) ) p++;

				if ( p>sToken )
				{
					// got a number, skip it
					pMyTokenizer->SetBufferPtr ( p );
					bSpecial = false;

				} else
				{
					// not a number, fallback
					sToken = (const char *) pMyTokenizer->GetToken ();
					bSpecial = pMyTokenizer->WasTokenSpecial ();
				}

			} else
			{
				sToken = (const char *) pMyTokenizer->GetToken ();
				bSpecial = pMyTokenizer->WasTokenSpecial ();
			}
		}
		bRedo = false;

		int iSpecial = sToken
			? ( bSpecial ? sToken[0] : 0 )
			: QUERY_END;
		assert ( !( iSpecial>0 && sToken[1]!=0 ) );

		// ignore stopwords, update positions
		if ( !iSpecial )
		{
			// GetWordID() may modify the word in-place; so we alloc a tempbuffer
			CSphString sTmp ( sToken );
			SphWordID_t iWordID = pDict->GetWordID ( (BYTE*)sTmp.cstr() );

			if ( iWordID || iAtomPos ) // fully ignore starting stopwords; update positions for everything else
				iAtomPos += 1+pMyTokenizer->GetOvershortCount();

			if ( !iWordID ) // skip stopwords
				continue;
		}

		///////////////////////////
		// handle "fragile" states
		///////////////////////////

		// handle post-atom negation state. everything except '|' flushes it
		if ( dState.Last()==XQS_NEGATION && iSpecial!='|' )
		{
			assert ( m_dStack.GetLength()==1 );
			dState.Pop ();
			PopNode ( true );
		}

		// handle "phrase end" state. there can be proximity or quorum modifier. everything else flushes it
		if ( dState.Last()==XQS_PHRASE_END )
		{
			assert ( m_dStack.Last().m_pNode->m_tAtom.m_iMaxDistance==0 );
			dState.Pop ();

			if ( iSpecial=='~' )
			{
				dState.Add ( XQS_PROXIMITY );
			} else if ( iSpecial=='/' )
			{
				dState.Add ( XQS_QUORUM );
			} else
			{
				bRedo = true;
				PopNode ();
			}
			continue;
		}

		// handle two "expect number" states (proximity and quorum). everything except valid number flushes it
		if ( dState.Last()==XQS_PROXIMITY || dState.Last()==XQS_QUORUM )
		{
			int iProx = iSpecial ? 0 : atoi ( sToken );
			if ( iProx>0 )
			{
				m_dStack.Last().m_pNode->m_tAtom.m_iMaxDistance = iProx;
				if ( dState.Last()==XQS_QUORUM )
				{
					m_dStack.Last().m_pNode->m_tAtom.m_bQuorum = true;
					if ( m_dStack.Last().m_pNode->m_tAtom.m_dWords.GetLength()>=32 ) // FIXME! must be in sync with eval engine
						return Error ( "too many keywords in the quorum operator (max=32)" );
				}

			} else
				bRedo = true;

			dState.Pop ();
			PopNode ();
			continue;
		}

		///////////////////////
		// handle non-specials
		///////////////////////

		if ( !iSpecial )
		{
			// handle in-phrase terms
			if ( dState.Last()==XQS_PHRASE )
			{
				assert ( m_dStack.Last().m_pNode->m_tAtom.m_iMaxDistance==0 );
				CSphExtendedQueryAtomWord tAW ( sToken, iAtomPos );
				m_dStack.Last().m_pNode->m_tAtom.m_dWords.Add ( tAW );
				continue;
			}

			// by default, spawn a new node and submit it
			assert ( dState.Last()==XQS_TEXT || dState.Last()==XQS_NEGTEXT );
			if ( dState.Last()==XQS_NEGTEXT )
				dState.Pop ();

			CSphExtendedQueryAtomWord tAW ( sToken, -1 );
			PushNode ();
			m_dStack.Last().m_bAny = bAny;
			m_dStack.Last().m_pNode->m_tAtom.m_uFields = uFields;
			m_dStack.Last().m_pNode->m_tAtom.m_dWords.Add ( tAW );
			bAny = false;
			PopNode ();
			continue;
		}

		///////////////////
		// handle specials
		////////////////////

		assert ( iSpecial );

		if ( iSpecial=='@' )
		{
			if ( !ParseFields ( uFields, pMyTokenizer.Ptr (), pSchema ) )
				return false;

			if ( pSchema->m_dFields.GetLength () != 32 )
				uFields &=  ( 1UL << pSchema->m_dFields.GetLength () ) - 1;
			continue;
		}

		// block start
		if ( iSpecial=='(' )
		{
			if ( dState.Last()==XQS_PHRASE )
				continue; // ignore parens within quotes

			if ( dState.Last()!=XQS_TEXT && dState.Last()!=XQS_NEGTEXT )
				return Error ( "INTERNAL ERROR: '(' in unexpected state %d", dState.Last() );

			if ( dState.Last()==XQS_TEXT && m_dStack.GetLength() )
				return Error ( "nested '(' are not allowed" );

			if ( dState.Last()==XQS_NEGTEXT )
				dState.Pop ();

			dState.Add ( XQS_TEXT );
			PushNode ();
			m_dStack.Last().m_bAny = bAny;
			continue;
		}

		// block end
		if ( iSpecial==')' )
		{
			if ( dState.Last()==XQS_PHRASE )
				continue; // ignore parens within quotes

			if ( dState.Last()!=XQS_TEXT )
				return Error ( "INTERNAL ERROR: ')' in unexpected state %d", dState.Last() );

			if ( m_dStack.GetLength()<1 )
				return Error ( "')' without matching '('" );

			dState.Pop ();
			PopNode ();
			continue;
		}

		// negation
		if ( iSpecial=='-' || iSpecial=='!' )
		{
			// silently ignore in-phrase negation
			if ( dState.Last()==XQS_PHRASE )
				continue;

			if ( m_dStack.GetLength() )
				return Error ( "negation is only allowed at top level" );

			dState.Add ( XQS_NEGATION );
			dState.Add ( XQS_NEGTEXT );
			PushNode ();
			continue;
		}

		// disjunction (ie. logical OR)
		if ( iSpecial=='|' )
		{
			if ( dState.Last()==XQS_NEGATION )
				dState.Add ( XQS_NEGTEXT );

			bAny = true;
			continue;
		}

		// query end
		if ( iSpecial==QUERY_END )
		{
			while ( m_dStack.GetLength() )
				PopNode ();
			break;
		}

		// phrase start end
		if ( iSpecial=='"' )
		{
			if ( dState.Last()!=XQS_PHRASE )
			{
				// opening quote
				PushNode ();
				m_dStack.Last().m_bAny = bAny;
				m_dStack.Last().m_pNode->m_tAtom.m_uFields = uFields;
				m_dStack.Last().m_pNode->m_tAtom.m_iMaxDistance = 0;
				bAny = false;
				dState.Add ( XQS_PHRASE );
				iAtomPos = 0;

			} else
			{
				// closing quote
				dState.Pop ();

				// implicit negation flush
				if ( dState.Last()==XQS_NEGTEXT )
					dState.Pop ();

				// we don't pop the node yet, because we need to handle proximity
				dState.Add ( XQS_PHRASE_END );
			}
			continue;
		}

		// proximity or quorum operator out of its state. just ignore
		if ( iSpecial=='~' || iSpecial=='/' )
			continue;

		assert ( 0 && "INTERNAL ERROR: unhandled special token" );
	}

	DeleteNodesWOFields ( m_pRes->m_pAccept );
	DeleteNodesWOFields ( m_pRes->m_pReject );

	m_pRes = NULL;
	return true;
}

//////////////////////////////////////////////////////////////////////////////

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
		xqIndent ( iIndent ); printf ( pNode->m_bAny ? "OR:\n" : "AND:\n" );
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
	CSphExtendedQueryParser qp;
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
