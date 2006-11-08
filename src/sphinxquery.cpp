//
// $Id$
//

#include "sphinx.h"
#include "sphinxquery.h"

/////////////////////////////////////////////////////////////////////////////
// Local classes
/////////////////////////////////////////////////////////////////////////////

class CSphBooleanQueryParser
{
public:
	CSphBooleanQueryExpr *	Parse ( const char * sQuery, const ISphTokenizer * pTokenizer );

protected:
	CSphBooleanQueryExpr *	m_pRoot;
	CSphBooleanQueryExpr *	m_pCur;

protected:
	int						IsSpecial ( int iCh );
	void					HandleOperator ( int iCh );
};

/////////////////////////////////////////////////////////////////////////////
// CSphBooleanQueryExpr implementation
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

int CSphBooleanQueryParser::IsSpecial ( int iCh )
{
	return ( iCh=='(' || iCh==')' || iCh=='&' || iCh=='|' || iCh=='-' || iCh=='!' );
}


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

	ISphTokenizer * pMyTokenizer = pTokenizer->Clone ();
	pMyTokenizer->AddSpecials ( "&|()-!" );
	pMyTokenizer->SetBuffer ( (BYTE*)sBuffer.cstr(), strlen ( sBuffer.cstr() ), true );

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
			? ( IsSpecial(pToken[0])
				? pToken[0]
				: 0 )
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
					m_pCur = m_pCur->m_pPrev;
					m_pCur->m_pNext->Detach ();
					SafeDelete ( m_pCur->m_pNext );
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
// Entry point
/////////////////////////////////////////////////////////////////////////////

CSphBooleanQueryExpr * sphParseBooleanQuery ( const char * sQuery, const ISphTokenizer * pTokenizer )
{
	CSphBooleanQueryParser qp;
	CSphBooleanQueryExpr * pTree = qp.Parse ( sQuery, pTokenizer );

	if ( pTree )
		pTree = RemoveRedundantNodes ( pTree );

	if ( pTree && IsEvaluable ( pTree ) )
	{
		pTree = ReorderLevel ( pTree );
	} else
	{
		// FIXME! warning here
		SafeDelete ( pTree );
	}

	return pTree;
}

//
// $Id$
//
