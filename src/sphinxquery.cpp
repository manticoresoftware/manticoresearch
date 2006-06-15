//
// $Id$
//

#include "sphinx.h"
#include "sphinxquery.h"

/////////////////////////////////////////////////////////////////////////////
// Local classes
/////////////////////////////////////////////////////////////////////////////

class SphQueryParser_c
{
public:
	SphQueryExpr_t *	Parse ( const char * sQuery, const ISphTokenizer * pTokenizer );

protected:
	SphQueryExpr_t *	m_pRoot;
	SphQueryExpr_t *	m_pCur;

protected:
	int					IsSpecial ( int iCh );
	void				HandleOperator ( int iCh );
};

/////////////////////////////////////////////////////////////////////////////
// SphQueryExpr_t implementation
/////////////////////////////////////////////////////////////////////////////

SphQueryExpr_t::SphQueryExpr_t ()
	: m_eType ( NODE_UNDEF )
	, m_pExpr ( NULL )
	, m_pPrev ( NULL )
	, m_pNext ( NULL )
	, m_pParent ( NULL )
	, m_bInvert ( false )
{
}


SphQueryExpr_t::~SphQueryExpr_t ()
{
	SafeDelete ( m_pExpr );
	SafeDelete ( m_pNext );
}

void SphQueryExpr_t::Detach ()
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


SphQueryExpr_t * SphQueryExpr_t::NewTail ()
{
	SphQueryExpr_t * pNew = new SphQueryExpr_t ();
	pNew->m_pPrev = this;
	pNew->m_pParent = m_pParent;

	assert ( !m_pNext );
	m_pNext = pNew;

	return pNew;
}


bool SphQueryExpr_t::IsNull ()
{
	bool bRes = ( m_pExpr==NULL && m_sWord.cstr()==NULL );
	assert ( !( bRes && m_pNext ) );
	return bRes;
}


bool SphQueryExpr_t::IsAlone ()
{
	return ( !m_pNext && !m_pPrev );
}

/////////////////////////////////////////////////////////////////////////////
// SphQueryParser_c implementation
/////////////////////////////////////////////////////////////////////////////

int SphQueryParser_c::IsSpecial ( int iCh )
{
	return ( iCh=='(' || iCh==')' || iCh=='&' || iCh=='|' || iCh=='-' || iCh=='!' );
}


void SphQueryParser_c::HandleOperator ( int iCh )
{
	assert ( iCh=='|' || iCh=='&' );
	assert ( m_pCur->IsNull() );

	// there were no words preceding the operator, so ignore it
	if ( !m_pCur->m_pPrev )
		return;

	// what type is it?
	SphQueryExpr_e eNewType = ( iCh=='|' ) ? NODE_OR : NODE_AND;

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
	SphQueryExpr_t * pNew = new SphQueryExpr_t ();
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

			SphQueryExpr_t * pPar = m_pCur->m_pParent;
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
			SphQueryExpr_t * pNew = new SphQueryExpr_t ();
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
			for ( SphQueryExpr_t * pTmp = pNew->m_pExpr; pTmp; pTmp = pTmp->m_pNext )
				pTmp->m_pParent = pNew;
		}
	}

	// whatever happens, my new type must be this
	m_pCur->m_eType = eNewType;
}


SphQueryExpr_t * SphQueryParser_c::Parse ( const char * sQuery, const ISphTokenizer * pTokenizer )
{
	assert ( sQuery );
	assert ( pTokenizer );

	m_pRoot = new SphQueryExpr_t ();
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

			m_pCur->m_pExpr = new SphQueryExpr_t ();
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

static void InvertExpr ( SphQueryExpr_t * pNode )
{
	SphQueryExpr_e eNewType = ( pNode->m_eType==NODE_AND ) ? NODE_OR : NODE_AND;
	for ( SphQueryExpr_t * pFix = pNode; pFix; pFix = pFix->m_pNext )
	{
		assert ( pFix->m_eType!=eNewType );
		pFix->m_eType = eNewType;
		pFix->m_bInvert = !pFix->m_bInvert;
	}
}


static SphQueryExpr_t * RemoveRedundantNodes ( SphQueryExpr_t * pNode )
{
	// if there's a single subexpr, and the types match, this is it
	while ( pNode->m_pExpr
		&& ( pNode->IsAlone() || pNode->m_pExpr->IsAlone() )
		&& ( pNode->m_eType==pNode->m_pExpr->m_eType
			|| pNode->m_eType==NODE_UNDEF
			|| pNode->m_pExpr->m_eType==NODE_UNDEF ) )
	{
		if ( pNode->m_eType==NODE_UNDEF )
			pNode->m_eType = pNode->m_pExpr->m_eType;

		if ( pNode->m_pExpr->m_pExpr )
		{
			SphQueryExpr_t * pKill = pNode->m_pExpr;

			pNode->m_pExpr = pKill->m_pExpr;
			pNode->m_pExpr->m_pParent = pNode;

			if ( pNode->IsAlone() )
			{
				// replace node in the list
				pNode->m_pNext = pKill->m_pNext;
				pNode->m_pPrev = pKill->m_pPrev;
				if ( pKill->m_pPrev )
					pKill->m_pPrev->m_pNext = pNode;
				if ( pKill->m_pNext )
					pKill->m_pNext->m_pPrev = pNode;

				// unchain node
				pKill->m_pNext = NULL;
				pKill->m_pPrev = NULL;

				// if this node is inverted, do invert all expr tokens
				if ( pNode->m_bInvert )
				{
					pNode->m_bInvert = false;
					InvertExpr ( pNode );
				}
			}

			pKill->m_pExpr = NULL;
			assert ( pKill->IsAlone() );
			pKill->Detach ();
			SafeDelete ( pKill );

		} else
		{
			assert ( strlen(pNode->m_pExpr->m_sWord.cstr()) > 0 );
			pNode->m_sWord = pNode->m_pExpr->m_sWord;

			if ( pNode->IsAlone() )
			{
				assert ( !pNode->m_pExpr->m_pPrev );

				// move expr level up to this node
				pNode->m_pNext = pNode->m_pExpr->m_pNext;
				pNode->m_pExpr->m_pNext = NULL;
				if ( pNode->m_pNext )
					pNode->m_pNext->m_pPrev = pNode;
				
				// fixup expr level parents
				for ( SphQueryExpr_t * pFix = pNode->m_pNext; pFix; pFix = pFix->m_pNext )
				{
					assert ( pFix->m_pParent==pNode );
					pFix->m_pParent = pNode->m_pParent;
				}

				// if this node is inverted, do invert all expr tokens
				if ( pNode->m_bInvert )
				{
					pNode->m_bInvert = false;
					InvertExpr ( pNode );
				}
			}

			assert ( pNode->m_pExpr->IsAlone() );
			pNode->m_pExpr->Detach ();
			SafeDelete ( pNode->m_pExpr );
		}
	}

	// work the siblings and children
	if ( pNode->m_pExpr )
	{
		pNode->m_pExpr = RemoveRedundantNodes ( pNode->m_pExpr );
		assert ( pNode->m_pExpr->m_pParent==pNode );
	}
	if ( pNode->m_pNext )
	{
		pNode->m_pNext = RemoveRedundantNodes ( pNode->m_pNext );
		assert ( pNode->m_pNext->m_pPrev==pNode );
	}
	
	return pNode;
}


static SphQueryExpr_t * SimplifyTree ( SphQueryExpr_t * pNode )
{
	if ( !pNode )
		return pNode;
	pNode = RemoveRedundantNodes ( pNode );

	// fixup special one-word case
	if ( !pNode->m_pExpr && pNode->IsAlone() && pNode->m_eType==NODE_UNDEF )
		pNode->m_eType = NODE_AND;

	return pNode;
}

/////////////////////////////////////////////////////////////////////////////
// Entry point
/////////////////////////////////////////////////////////////////////////////

SphQueryExpr_t * sphParseQuery ( const char * sQuery, const ISphTokenizer * pTokenizer )
{
	SphQueryParser_c qp;
	SphQueryExpr_t * pTree = qp.Parse ( sQuery, pTokenizer );
	pTree = SimplifyTree ( pTree );
	return pTree;
}

//
// $Id$
//
