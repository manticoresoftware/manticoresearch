//
// $Id$
//

//
// Copyright (c) 2001-2010, Andrew Aksyonoff
// Copyright (c) 2008-2010, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxquery.h"
#include "sphinxutils.h"
#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
// EXTENDED PARSER RELOADED
//////////////////////////////////////////////////////////////////////////

#include "yysphinxquery.h"

//////////////////////////////////////////////////////////////////////////

class XQParser_t
{
public:
					XQParser_t ();
					~XQParser_t () {}

public:
	bool			Parse ( XQQuery_t & tQuery, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict );

	bool			Error ( const char * sTemplate, ... );
	void			Warning ( const char * sTemplate, ... );

	bool			AddField ( DWORD & uFields, const char * szField, int iLen );
	bool			ParseFields ( DWORD & uFields, int & iMaxFieldPos );

	bool			IsSpecial ( char c );
	int				GetToken ( YYSTYPE * lvalp );

	void			AddQuery ( XQNode_t * pNode );
	XQNode_t *		AddKeyword ( const char * sKeyword, DWORD uStar = STAR_NONE );
	XQNode_t *		AddKeyword ( XQNode_t * pLeft, XQNode_t * pRight );
	XQNode_t *		AddOp ( XQOperator_e eOp, XQNode_t * pLeft, XQNode_t * pRight );

	void			Cleanup ();
	XQNode_t *		SweepNulls ( XQNode_t * pNode );
	bool			FixupNots ( XQNode_t * pNode );

public:
	XQQuery_t *				m_pParsed;

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
	bool					m_bWasBlended;

	bool					m_bEmpty;

	bool					m_bQuoted;

	CSphVector<CSphString>	m_dIntTokens;
};

//////////////////////////////////////////////////////////////////////////

int yylex ( YYSTYPE * lvalp, XQParser_t * pParser )
{
	return pParser->GetToken ( lvalp );
}

void yyerror ( XQParser_t * pParser, const char * sMessage )
{
	if ( pParser->m_pParsed->m_sParseError.IsEmpty() )
		pParser->m_pParsed->m_sParseError.SetSprintf ( "%s near '%s'", sMessage, pParser->m_pLastTokenStart );
}

#include "yysphinxquery.c"

//////////////////////////////////////////////////////////////////////////

void XQNode_t::SetFieldSpec ( DWORD uMask, int iMaxPos )
{
	// set it, if we do not yet have one
	if ( !m_bFieldSpec )
	{
		m_bFieldSpec = true;
		m_uFieldMask = uMask;
		m_iFieldMaxPos = iMaxPos;
	}

	// some of the children might not yet have a spec, even if the node itself has
	// eg. 'hello @title world' (whole node has '@title' spec but 'hello' node does not have any!)
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->SetFieldSpec ( uMask, iMaxPos );
}

void XQNode_t::ClearFieldMask ()
{
	m_uFieldMask = 0xFFFFFFFFUL;

	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->ClearFieldMask();
}


bool XQNode_t::IsEqualTo ( const XQNode_t * pNode )
{
	if ( !pNode || pNode->GetHash()!=GetHash() || pNode->GetOp()!=GetOp() || pNode->IsPlain()!=IsPlain() )
		return false;

	if ( IsPlain() )
	{
		// two plain nodes. let's compare the keywords
		if ( pNode->m_dWords.GetLength()!=m_dWords.GetLength() )
			return false;

		if ( !m_dWords.GetLength() )
			return true;

		SmallStringHash_T<int> hSortedWords;
		ARRAY_FOREACH ( i, pNode->m_dWords )
			hSortedWords.Add ( 0, pNode->m_dWords[i].m_sWord );

		ARRAY_FOREACH ( i, m_dWords )
			if ( !hSortedWords.Exists ( m_dWords[i].m_sWord ) )
				return false;

		return true;
	}

	// two non-plain nodes. let's compare the children
	if ( pNode->m_dChildren.GetLength()!=m_dChildren.GetLength() )
		return false;

	if ( !m_dChildren.GetLength() )
		return true;

	ARRAY_FOREACH ( i, m_dChildren )
		if ( !pNode->m_dChildren[i]->IsEqualTo ( m_dChildren[i] ) )
			return false;
	return true;
}


uint64_t XQNode_t::GetHash() const
{
	if ( m_iMagicHash )
		return m_iMagicHash;

	XQOperator_e dZeroOp[2];
	dZeroOp[0] = m_eOp;
	dZeroOp[1] = (XQOperator_e) 0;

	ARRAY_FOREACH ( i, m_dWords )
		m_iMagicHash = 100 + ( m_iMagicHash ^ sphFNV64 ( (const BYTE*)m_dWords[i].m_sWord.cstr() ) ); ///< +100 to make it non-transitive
	ARRAY_FOREACH ( j, m_dChildren )
		m_iMagicHash = 100 + ( m_iMagicHash ^ m_dChildren[j]->GetHash() ); ///< +100 to make it non-transitive
	m_iMagicHash += 1000000; ///< to immerse difference between parents and children
	m_iMagicHash ^= sphFNV64 ( (const BYTE*)dZeroOp );

	return m_iMagicHash;
}


void XQNode_t::SetOp ( XQOperator_e eOp, XQNode_t * pArg1, XQNode_t * pArg2 )
{
	m_eOp = eOp;
	m_dChildren.Reset();
	if ( pArg1 )
		m_dChildren.Add ( pArg1 );
	if ( pArg2 )
		m_dChildren.Add ( pArg2 );
}

//////////////////////////////////////////////////////////////////////////

XQParser_t::XQParser_t ()
	: m_pParsed ( NULL )
	, m_pLastTokenStart ( NULL )
	, m_pRoot ( NULL )
	, m_bStopOnInvalid ( true )
	, m_bWasBlended ( false )
	, m_bQuoted ( false )
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
	memcpy ( sBuf, sPrefix, iPrefix );

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
	memcpy ( sBuf, sPrefix, iPrefix );

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
	} else
	{
		if ( iField>=32 )
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

			} else if ( *pPtr==',' )
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
	if ( pPtr[0]=='[' && isdigit ( pPtr[1] ) )
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

		if ( m_bWasBlended )
			m_iAtomPos += m_pTokenizer->SkipBlended();

		// tricky stuff
		// we need to manually check for numbers in certain states (currently, just after proximity or quorum operator)
		// required because if 0-9 are not in charset_table, or min_word_len is too high,
		// the tokenizer will *not* return the number as a token!
		m_pLastTokenStart = m_pTokenizer->GetBufferPtr ();
		const char * sEnd = m_pTokenizer->GetBufferEnd ();

		const char * p = m_pLastTokenStart;
		while ( p<sEnd && isspace ( *(BYTE*)p ) ) p++; // to avoid CRT assertions on Windows

		const char * sToken = p;
		while ( p<sEnd && isdigit ( *(BYTE*)p ) ) p++;

		if ( p>sToken && ( *p=='\0' || isspace ( *(BYTE*)p ) || IsSpecial(*p) ) )
		{
			// got a number followed by a whitespace or special, handle it
			char sNumberBuf[16];

			int iNumberLen = Min ( (int)sizeof(sNumberBuf)-1, int(p-sToken) );
			memcpy ( sNumberBuf, sToken, iNumberLen );
			sNumberBuf[iNumberLen] = '\0';
			m_tPendingToken.tInt.iValue = atoi ( sNumberBuf );

			// check if it can be used as a keyword too
			m_pTokenizer->SetBuffer ( (BYTE*)sNumberBuf, iNumberLen );
			sToken = (const char*) m_pTokenizer->GetToken();
			m_pTokenizer->SetBuffer ( m_sQuery, m_iQueryLen );
			m_pTokenizer->SetBufferPtr ( p );

			m_tPendingToken.tInt.iStrIndex = -1;
			if ( sToken )
			{
				m_dIntTokens.Add ( sToken );
				if ( m_pDict->GetWordID ( (BYTE*)sToken ) )
					m_tPendingToken.tInt.iStrIndex = m_dIntTokens.GetLength()-1;
				else
					m_dIntTokens.Pop();
				m_iAtomPos++;
			}

			m_iPendingNulls = 0;
			m_iPendingType = TOK_INT;
			break;
		}

		// not a number, or not followed by a whitespace, so fallback
		sToken = (const char *) m_pTokenizer->GetToken ();
		if ( !sToken )
			return 0;
		m_bWasBlended = m_pTokenizer->TokenIsBlended();
		m_bEmpty = false;

		m_iPendingNulls = m_pTokenizer->GetOvershortCount ();
		m_iAtomPos += 1+m_iPendingNulls;

		if ( m_pTokenizer->WasTokenSpecial() )
		{
			// specials must not affect pos
			m_iAtomPos--;

			// some specials are especially special
			if ( sToken[0]=='@' )
			{
				// parse fields operator
				if ( !ParseFields ( m_tPendingToken.tFieldLimit.uMask, m_tPendingToken.tFieldLimit.iMaxPos ) )
					return -1;

				if ( m_pSchema->m_dFields.GetLength()!=32 )
					m_tPendingToken.tFieldLimit.uMask &= ( 1UL<<m_pSchema->m_dFields.GetLength() )-1;

				m_iPendingType = TOK_FIELDLIMIT;
				break;

			} else if ( sToken[0]=='<' )
			{
				if ( *m_pTokenizer->GetBufferPtr()=='<' )
				{
					// got "<<", aka operator BEFORE
					m_iPendingType = TOK_BEFORE;
					break;
				} else
				{
					// got stray '<', ignore
					continue;
				}
			} else
			{
				// all the other specials are passed to parser verbatim
				if ( sToken[0]=='"' )
					m_bQuoted = !m_bQuoted;
				m_iPendingType = sToken[0]=='!' ? '-' : sToken[0];
				m_pTokenizer->m_bPhrase = m_bQuoted;
				break;
			}
		}

		// check for stopword, and create that node
		// temp buffer is required, because GetWordID() might expand (!) the keyword in-place
		const int MAX_BYTES = 3*SPH_MAX_WORD_LEN + 16;
		BYTE sTmp [ MAX_BYTES ];

		strncpy ( (char*)sTmp, sToken, MAX_BYTES );
		sTmp[MAX_BYTES-1] = '\0';

		if ( !m_pDict->GetWordID ( sTmp ) )
			sToken = NULL;

		// information about stars is lost after this point, so was have to save it now
		DWORD uStarPosition = STAR_NONE;
		uStarPosition |= *m_pTokenizer->GetTokenEnd()=='*' ? STAR_BACK : 0;
		uStarPosition |= ( m_pTokenizer->GetTokenStart()!=m_pTokenizer->GetBufferPtr() ) &&
			m_pTokenizer->GetTokenStart()[-1]=='*' ? STAR_FRONT : 0;

		m_tPendingToken.pNode = AddKeyword ( sToken, uStarPosition );
		m_iPendingType = TOK_KEYWORD;

		if ( m_pTokenizer->TokenIsBlended() )
			m_iAtomPos--;
		break;
	}

	// someone must be pending now!
	assert ( m_iPendingType );
	m_bEmpty = false;

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


XQNode_t * XQParser_t::AddKeyword ( const char * sKeyword, DWORD uStarPosition )
{
	XQKeyword_t tAW ( sKeyword, m_iAtomPos );
	tAW.m_uStarPosition = uStarPosition;

	XQNode_t * pNode = new XQNode_t();
	pNode->m_dWords.Add ( tAW );

	m_dSpawned.Add ( pNode );
	return pNode;
}


XQNode_t * XQParser_t::AddKeyword ( XQNode_t * pLeft, XQNode_t * pRight )
{
	if ( !pLeft || !pRight )
		return pLeft ? pLeft : pRight;

	assert ( pLeft->IsPlain() );
	assert ( pRight->IsPlain() );
	assert ( pRight->m_dWords.GetLength()==1 );

	pLeft->m_dWords.Add ( pRight->m_dWords[0] );
	m_dSpawned.RemoveValue ( pRight );
	SafeDelete ( pRight );
	return pLeft;
}


XQNode_t * XQParser_t::AddOp ( XQOperator_e eOp, XQNode_t * pLeft, XQNode_t * pRight )
{
	/////////
	// unary
	/////////

	if ( eOp==SPH_QUERY_NOT )
	{
		XQNode_t * pNode = new XQNode_t();
		pNode->SetOp ( SPH_QUERY_NOT, pLeft );
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
	if ( !pLeft->IsPlain() && pLeft->GetOp()==eOp )
	{
		pLeft->m_dChildren.Add ( pRight );
		pResult = pLeft;
	} else
	{
		XQNode_t * pNode = new XQNode_t();
		pNode->SetOp ( eOp, pLeft, pRight );
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
		ARRAY_FOREACH ( i, pNode->m_dWords )
			if ( pNode->m_dWords[i].m_sWord.cstr()==NULL )
				pNode->m_dWords.Remove ( i-- );

		if ( pNode->m_dWords.GetLength()==0 )
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
	if ( pNode->GetOp()!=SPH_QUERY_NOT && pNode->m_dChildren.GetLength()==1 )
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
		if ( pNode->m_dChildren[i]->GetOp()==SPH_QUERY_NOT )
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
	if ( pNode->GetOp()==SPH_QUERY_OR )
	{
		m_pParsed->m_sParseError.SetSprintf ( "query is non-computable (NOT is not allowed within OR)" );
		return false;
	}

	// NOT used in before operator
	if ( pNode->GetOp()==SPH_QUERY_BEFORE )
	{
		m_pParsed->m_sParseError.SetSprintf ( "query is non-computable (NOT cannot be used as before operand)" );
		return false;
	}

	// must be some NOTs within AND at this point, convert this node to ANDNOT
	assert ( pNode->GetOp()==SPH_QUERY_AND && pNode->m_dChildren.GetLength() && dNots.GetLength() );

	XQNode_t * pAnd = new XQNode_t();
	pAnd->SetOp ( SPH_QUERY_AND, pNode->m_dChildren );
	m_dSpawned.Add ( pAnd );

	XQNode_t * pNot = NULL;
	if ( dNots.GetLength()==1 )
	{
		pNot = dNots[0];
	} else
	{
		pNot = new XQNode_t();
		pNot->SetOp ( SPH_QUERY_OR, dNots );
		m_dSpawned.Add ( pNot );
	}

	pNode->SetOp ( SPH_QUERY_ANDNOT, pAnd, pNot );
	return true;
}


static void DeleteNodesWOFields ( XQNode_t * pNode )
{
	if ( !pNode )
		return;

	for ( int i = 0; i < pNode->m_dChildren.GetLength (); )
	{
		if ( pNode->m_dChildren[i]->m_uFieldMask==0 )
		{
			// this should be a leaf node
			assert ( pNode->m_dChildren[i]->m_dChildren.GetLength()==0 );
			SafeDelete ( pNode->m_dChildren[i] );
			pNode->m_dChildren.RemoveFast ( i );

		} else
		{
			DeleteNodesWOFields ( pNode->m_dChildren[i] );
			i++;
		}
	}
}


bool XQParser_t::Parse ( XQQuery_t & tParsed, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict )
{
	CSphScopedPtr<ISphTokenizer> pMyTokenizer ( pTokenizer->Clone ( true ) );
	pMyTokenizer->AddSpecials ( "()|-!@~\"/^$<" );
	pMyTokenizer->EnableQueryParserMode ( true );

	// check for relaxed syntax
	const char * OPTION_RELAXED = "@@relaxed";
	const int OPTION_RELAXED_LEN = strlen ( OPTION_RELAXED );

	m_bStopOnInvalid = true;
	if ( strncmp ( sQuery, OPTION_RELAXED, OPTION_RELAXED_LEN )==0 && !sphIsAlpha ( sQuery[OPTION_RELAXED_LEN] ) )
	{
		sQuery += OPTION_RELAXED_LEN;
		m_bStopOnInvalid = false;
	}

	// setup parser
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
	m_bEmpty = true;

	m_pTokenizer->SetBuffer ( m_sQuery, m_iQueryLen );
	int iRes = yyparse ( this );

	if ( ( iRes || !m_pParsed->m_sParseError.IsEmpty() ) && !m_bEmpty )
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

	if ( m_pRoot && m_pRoot->GetOp()==SPH_QUERY_NOT )
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
	while ( iIndent-- )
		printf ( " " );
}


static void xqDump ( XQNode_t * pNode, const CSphSchema & tSch, int iIndent )
{
	if ( !pNode->IsPlain() )
	{
		xqIndent ( iIndent );
		switch ( pNode->GetOp() )
		{
			case SPH_QUERY_AND: printf ( "AND:\n" ); break;
			case SPH_QUERY_OR: printf ( "OR:\n" ); break;
			case SPH_QUERY_NOT: printf ( "NOT:\n" ); break;
			case SPH_QUERY_ANDNOT: printf ( "ANDNOT:\n" ); break;
			case SPH_QUERY_BEFORE: printf ( "BEFORE:\n" ); break;
			default: printf ( "unknown-op-%d:\n", pNode->GetOp() ); break;
		}
		ARRAY_FOREACH ( i, pNode->m_dChildren )
			xqDump ( pNode->m_dChildren[i], tSch, iIndent+1 );
	} else
	{
		xqIndent ( iIndent );
		printf ( "MATCH(%d,%d):", pNode->m_uFieldMask, pNode->m_iMaxDistance );

		ARRAY_FOREACH ( i, pNode->m_dWords )
		{
			const XQKeyword_t & tWord = pNode->m_dWords[i];

			const char * sLocTag = "";
			if ( tWord.m_bFieldStart ) sLocTag = ", start";
			if ( tWord.m_bFieldEnd ) sLocTag = ", end";

			printf ( " %s (qpos %d%s)", tWord.m_sWord.cstr(), tWord.m_iAtomPos, sLocTag );
		}
		printf ( "\n" );
	}
}
#endif


bool sphParseExtendedQuery ( XQQuery_t & tParsed, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict ) // NOLINT
{
	XQParser_t qp;
	bool bRes = qp.Parse ( tParsed, sQuery, pTokenizer, pSchema, pDict );

#if XQDEBUG
	if ( bRes )
	{
		printf ( "--- query ---\n" );
		xqDump ( tParsed.m_pRoot, *pSchema, 0 );
		printf ( "---\n" );
	}
#endif

	return bRes;
}

//////////////////////////////////////////////////////////////////////////
// COMMON SUBTREES DETECTION
//////////////////////////////////////////////////////////////////////////

/// Decides if given pTree is appropriate for caching or not. Currently we don't cache
/// the end values (leafs).
static bool IsAppropriate ( XQNode_t * pTree )
{
	if ( !pTree ) return false;

	// skip nodes that actually are leaves (eg. "AND smth" node instead of merely "smth")
	return !( pTree->IsPlain() && pTree->m_dWords.GetLength()==1 && pTree->GetOp()!=SPH_QUERY_NOT );
}

typedef CSphOrderedHash < DWORD, uint64_t, IdentityHash_fn, 128, 117 > CDwordHash;

// stores the pair of a tree, and the bitmask of common nodes
// which contains the tree.
class BitMask_t
{
	XQNode_t *		m_pTree;
	uint64_t		m_uMask;

public:
	BitMask_t ()
		: m_pTree ( NULL )
		, m_uMask ( 0ull )
	{}

	void Init ( XQNode_t * pTree, uint64_t uMask )
	{
		m_pTree = pTree;
		m_uMask = uMask;
	}

	inline uint64_t GetMask() const { return m_uMask; }
	inline XQNode_t * GetTree() const { return m_pTree; }
};

// a list of unique values.
class Associations_t : public CDwordHash
{
public:

	// returns true when add the second member.
	// The reason is that only one is not interesting for us,
	// but more than two will flood the caller.
	bool Associate2nd ( uint64_t uTree )
	{
		if ( Exists ( uTree ) )
			return false;
		Add ( 0, uTree );
		return GetLength()==2;
	}

	// merge with another similar
	void Merge ( const Associations_t& parents )
	{
		parents.IterateStart();
		while ( parents.IterateNext() )
			Associate2nd ( parents.IterateGetKey() );
	}
};

// associate set of nodes, common bitmask for these nodes,
// and gives the < to compare different pairs
class BitAssociation_t
{
private:
	const Associations_t *	m_pAssociations;
	mutable int				m_iBits;

	// The key method of subtree selection.
	// Most 'heavy' subtrees will be extracted first.
	inline int GetWeight() const
	{
		assert ( m_pAssociations );
		int iNodes = m_pAssociations->GetLength();
		if ( m_iBits==0 && m_uMask!=0 )
		{
			for ( uint64_t dMask = m_uMask; dMask; dMask >>=1 )
				m_iBits += (int)( dMask & 1 );
		}

		// current working formula is num_nodes^2 * num_hits
		return iNodes * iNodes * m_iBits;
	}

public:
	uint64_t			m_uMask;

	BitAssociation_t()
		: m_pAssociations ( NULL )
		, m_iBits ( 0 )
		, m_uMask ( 0 )
	{}

	void Init ( uint64_t uMask, const Associations_t* dNodes )
	{
		m_uMask = uMask;
		m_pAssociations = dNodes;
		m_iBits = 0;
	}

	bool operator< (const BitAssociation_t& second) const
	{
		return GetWeight() < second.GetWeight();
	}
};

// for pairs of values builds and stores the association "key -> list of values"
class CAssociations_t
	: public CSphOrderedHash < Associations_t, uint64_t, IdentityHash_fn, 128, 117 >
{
	int		m_iBits;			// number of non-unique associations
public:

	CAssociations_t() : m_iBits ( 0 ) {}

	// Add the given pTree into the list of pTrees, associated with given uHash
	int Associate ( XQNode_t * pTree, uint64_t uHash )
	{
		if ( !Exists ( uHash ) )
			Add ( Associations_t(), uHash );
		if ( operator[]( uHash ).Associate2nd ( pTree->GetHash() ) )
			m_iBits++;
		return m_iBits;
	}

	// merge the existing association of uHash with given chain
	void MergeAssociations ( const Associations_t & chain, uint64_t uHash )
	{
		if ( !Exists ( uHash ) )
			Add ( chain, uHash );
		else
			operator[]( uHash ).Merge ( chain );
	}

	inline int GetBits() const { return m_iBits; }
};

// The main class for working with common subtrees
class RevealCommon_t : ISphNoncopyable
{
private:
	static const int			MAX_MULTINODES = 64;
	CSphVector<BitMask_t>		m_dBitmasks;		// all bitmasks for all the nodes
	CSphVector<uint64_t>		m_dSubQueries;		// final vector with roadmap for tree division.
	CAssociations_t				m_hNodes;			// initial accumulator for nodes
	CAssociations_t				m_hInterSections;	// initial accumulator for nodes
	CDwordHash					m_hBitOrders;		// order numbers for found common subnodes
	XQOperator_e				m_eOp;				// my operator which I process

private:

	// returns the order for given uHash (if any).
	inline int GetBitOrder ( uint64_t uHash ) const
	{
		if ( !m_hBitOrders.Exists ( uHash ) )
			return -1;
		return m_hBitOrders[uHash];
	}

	// recursively scans the whole tree and builds the maps
	// where a list of parents associated with every "leaf" nodes (i.e. with children)
	bool BuildAssociations ( XQNode_t * pTree )
	{
		if ( IsAppropriate ( pTree ) )
		{
			ARRAY_FOREACH ( i, pTree->m_dChildren )
			if ( ( !BuildAssociations ( pTree->m_dChildren[i] ) )
				|| ( ( m_eOp==pTree->GetOp() )
				&& ( m_hNodes.Associate ( pTree, pTree->m_dChildren[i]->GetHash() )>=MAX_MULTINODES ) ) )
			{
				return false;
			}
		}
		return true;
	}

	// Find all leafs, non-unique across the tree,
	// and associate the order number with every of them
	bool CalcCommonNodes ()
	{
		if ( !m_hNodes.GetBits() )
			return false; // there is totally no non-unique leaves
		int iBit = 0;
		m_hNodes.IterateStart();
		while ( m_hNodes.IterateNext() )
			if ( m_hNodes.IterateGet().GetLength() > 1 )
				m_hBitOrders.Add ( iBit++, m_hNodes.IterateGetKey() );
		assert ( m_hNodes.GetBits()==m_hBitOrders.GetLength() );
		m_hNodes.Reset(); ///< since from now we don't need this data anymore
		return true;
	}

	// recursively builds for every node the bitmaks
	// of common nodes it has as children
	void BuildBitmasks ( XQNode_t * pTree )
	{
		if ( !IsAppropriate ( pTree ) )
			return;

		if ( m_eOp==pTree->GetOp() )
		{
			// calculate the bitmask
			int iOrder;
			uint64_t dMask = 0;
			ARRAY_FOREACH ( i, pTree->m_dChildren )
			{
				iOrder = GetBitOrder ( pTree->m_dChildren[i]->GetHash() );
				if ( iOrder>=0 )
					dMask |= 1ull << iOrder;
			}

			// add the bitmask into the array
			if ( dMask )
				m_dBitmasks.Add().Init( pTree, dMask );
		}

		// recursively process all the children
		ARRAY_FOREACH ( i, pTree->m_dChildren )
			BuildBitmasks ( pTree->m_dChildren[i] );
	}

	// Collect all possible intersections of Bitmasks.
	// For every non-zero intersection we collect the list of trees which contain it.
	void CalcIntersections ()
	{
		// Round 1. Intersect all content of bitmasks one-by-one.
		ARRAY_FOREACH ( i, m_dBitmasks )
			for ( int j = i+1; j<m_dBitmasks.GetLength(); j++ )
			{
				// intersect one-by-one and group (grouping is done by nature of a hash)
				uint64_t uMask = m_dBitmasks[i].GetMask() & m_dBitmasks[j].GetMask();
				if ( uMask )
				{
					m_hInterSections.Associate ( m_dBitmasks[i].GetTree(), uMask );
					m_hInterSections.Associate ( m_dBitmasks[j].GetTree(), uMask );
				}
			}

		// Round 2. Intersect again all collected intersection one-by-one - until zero.
		void *p1=NULL, *p2;
		uint64_t uMask1, uMask2;
		while ( m_hInterSections.IterateNext ( &p1 ) )
		{
			p2 = p1;
			while ( m_hInterSections.IterateNext ( &p2 ) )
			{
				uMask1 = CAssociations_t::IterateGetKey ( &p1 );
				uMask2 = CAssociations_t::IterateGetKey ( &p2 );
				assert ( uMask1!=uMask2 );
				uMask1 &= uMask2;
				if ( uMask1 )
				{
					m_hInterSections.MergeAssociations ( CAssociations_t::IterateGet ( &p1 ), uMask1 );
					m_hInterSections.MergeAssociations ( CAssociations_t::IterateGet ( &p2 ), uMask1 );
				}
			}
		}
	}

	// create the final kit of common-subsets
	// which we will actually reveal (extract) from original trees
	void MakeQueries()
	{
		CSphVector<BitAssociation_t> dSubnodes; // masks for our selected subnodes
		dSubnodes.Reserve ( m_hInterSections.GetLength() );
		m_hInterSections.IterateStart();
		while ( m_hInterSections.IterateNext() )
			dSubnodes.Add().Init( m_hInterSections.IterateGetKey(), &m_hInterSections.IterateGet() );

		// sort by weight descending (weight sorting is hold by operator <)
		dSubnodes.RSort();
		m_dSubQueries.Reset();

		// make the final subtrees vector: get one-by-one from the beginning,
		// intresect with all the next and throw out zeros.
		// The final subqueries will not be intersected between each other.
		int j;
		uint64_t uMask;
		ARRAY_FOREACH ( i, dSubnodes )
		{
			uMask = dSubnodes[i].m_uMask;
			m_dSubQueries.Add ( uMask );
			j = i+1;
			while ( j < dSubnodes.GetLength() )
			{
				if ( !( dSubnodes[j].m_uMask &= ~uMask ) )
					dSubnodes.Remove(j);
				else
					j++;
			}
		}
	}

	// Now we finally extract the common subtrees from original tree
	// and (recursively) from it's children
	void Reorganize ( XQNode_t * pTree )
	{
		if ( !IsAppropriate ( pTree ) )
			return;

		if ( m_eOp==pTree->GetOp() )
		{
			// pBranch is for common subset of children, pOtherChildren is for the rest.
			CSphOrderedHash < XQNode_t*, int, IdentityHash_fn, 64, 13 > hBranches;
			XQNode_t * pOtherChildren = NULL;
			int iBit;
			int iOptimizations = 0;
			ARRAY_FOREACH ( i, pTree->m_dChildren )
			{
				iBit = GetBitOrder ( pTree->m_dChildren[i]->GetHash() );

				// works only with children which are actually common with somebody else
				if ( iBit>=0 )
				{
					// since subqueries doesn't intersected between each other,
					// the first hit we found in this loop is exactly what we searched.
					ARRAY_FOREACH ( j, m_dSubQueries )
						if ( ( 1ull << iBit ) & m_dSubQueries[j] )
						{
							XQNode_t * pNode;
							if ( !hBranches.Exists(j) )
							{
								pNode = new XQNode_t;
								pNode->SetOp ( m_eOp, pTree->m_dChildren[i] );
								hBranches.Add ( pNode, j );
							} else
							{
								pNode = hBranches[j];
								pNode->m_dChildren.Add ( pTree->m_dChildren[i] );

								// Count essential subtrees (with at least 2 children)
								if ( pNode->m_dChildren.GetLength()==2 )
									iOptimizations++;
							}
							break;
						}
					// another nodes add to the set of "other" children
				} else
				{
					if ( !pOtherChildren )
					{
						pOtherChildren = new XQNode_t;
						pOtherChildren->SetOp ( m_eOp, pTree->m_dChildren[i] );
					} else
						pOtherChildren->m_dChildren.Add ( pTree->m_dChildren[i] );
				}
			}

			// we don't reorganize explicit simple case - as no "others" and only one common.
			// Also reject optimization if there is nothing to optimize.
			if ( ( iOptimizations==0 )
				| ( !pOtherChildren && ( hBranches.GetLength()==1 ) ) )
			{
				if ( pOtherChildren )
					pOtherChildren->m_dChildren.Reset();
				hBranches.IterateStart();
				while ( hBranches.IterateNext() )
				{
					assert ( hBranches.IterateGet() );
					hBranches.IterateGet()->m_dChildren.Reset();
					SafeDelete ( hBranches.IterateGet() );
				}
			} else
			{
				// reorganize the tree: replace the common subset to explicit node with
				// only common members inside. This will give the the possibility
				// to cache the node.
				pTree->m_dChildren.Reset();
				if ( pOtherChildren )
					pTree->m_dChildren.SwapData ( pOtherChildren->m_dChildren );

				hBranches.IterateStart();
				while ( hBranches.IterateNext() )
				{
					if ( hBranches.IterateGet()->m_dChildren.GetLength()==1 )
					{
						pTree->m_dChildren.Add ( hBranches.IterateGet()->m_dChildren[0] );
						hBranches.IterateGet()->m_dChildren.Reset();
						SafeDelete ( hBranches.IterateGet() );
					} else
						pTree->m_dChildren.Add ( hBranches.IterateGet() );
				}
			}
			SafeDelete ( pOtherChildren );
		}

		// recursively process all the children
		ARRAY_FOREACH ( i, pTree->m_dChildren )
			Reorganize ( pTree->m_dChildren[i] );
	}

public:
	explicit RevealCommon_t ( XQOperator_e eOp )
		: m_eOp ( eOp )
	{}

	// actual method for processing tree and reveal (extract) common subtrees
	void transform ( const CSphVector<XQNode_t*> & dTrees )
	{
		// collect all non-unique nodes
		ARRAY_FOREACH ( i, dTrees )
			if ( !BuildAssociations ( dTrees[i] ) )
				return;

		// count and order all non-unique nodes
		if ( !CalcCommonNodes() )
			return;

		// create and collect bitmask for every node
		ARRAY_FOREACH ( i, dTrees )
			BuildBitmasks ( dTrees[i] );

		// intersect all bitmasks one-by-one, and also intersect all intersections
		CalcIntersections();

		// the die-hard: actually select the set of subtrees which we'll process
		MakeQueries();

		// ... and finally - process all our trees.
		ARRAY_FOREACH ( i, dTrees )
			Reorganize ( dTrees[i] );
	}
};


struct MarkedNode_t
{
	int			m_iCounter;
	XQNode_t *	m_pTree;
	bool		m_bMarked;
	int			m_iOrder;

	explicit MarkedNode_t ( XQNode_t * pTree=NULL )
		: m_iCounter ( 1 )
		, m_pTree ( pTree )
		, m_bMarked ( false )
		, m_iOrder ( 0 )
	{}

	void MarkIt ( bool bMark=true )
	{
		// mark
		if ( bMark )
		{
			m_iCounter++;
			m_bMarked = true;
			return;
		}

		// unmark
		if ( m_bMarked && m_iCounter>1 )
			m_iCounter--;
		if ( m_iCounter<2 )
			m_bMarked = false;
	}
};

typedef CSphOrderedHash < MarkedNode_t, uint64_t, IdentityHash_fn, 128, 117 > CSubtreeHash;

/// check hashes, then check subtrees, then flag
static void FlagCommonSubtrees ( XQNode_t * pTree,
	CSubtreeHash & hSubTrees, bool bFlag=true, bool bMarkIt=true )
{
	if ( !IsAppropriate ( pTree ) )
		return;

	// we do not yet have any collisions stats,
	// but chances are we don't actually need IsEqualTo() at all
	uint64_t iHash = pTree->GetHash();
	if ( bFlag && hSubTrees.Exists ( iHash ) && hSubTrees [ iHash ].m_pTree->IsEqualTo ( pTree ) )
	{
		hSubTrees[iHash].MarkIt ();

		// we just add all the children but do NOT mark them as common
		// so that only the subtree root is marked.
		// also we unmark all the cases which were eaten by bigger trees
		ARRAY_FOREACH ( i, pTree->m_dChildren )
			if ( !hSubTrees.Exists ( pTree->m_dChildren[i]->GetHash() ) )
				FlagCommonSubtrees ( pTree->m_dChildren[i], hSubTrees, false, bMarkIt );
			else
				FlagCommonSubtrees ( pTree->m_dChildren[i], hSubTrees, false, false );
	} else
	{
		if ( !bMarkIt )
			hSubTrees[iHash].MarkIt(false);
		else
			hSubTrees.Add ( MarkedNode_t ( pTree ), iHash );

		ARRAY_FOREACH ( i, pTree->m_dChildren )
			FlagCommonSubtrees ( pTree->m_dChildren[i], hSubTrees, bFlag, bMarkIt );
	}
}


static void SignCommonSubtrees ( XQNode_t * pTree, CSubtreeHash & hSubTrees )
{
	if ( !pTree )
		return;

	uint64_t iHash = pTree->GetHash();
	if ( hSubTrees.Exists(iHash) && hSubTrees[iHash].m_bMarked )
		pTree->TagAsCommon ( hSubTrees[iHash].m_iOrder, hSubTrees[iHash].m_iCounter );

	ARRAY_FOREACH ( i, pTree->m_dChildren )
		SignCommonSubtrees ( pTree->m_dChildren[i], hSubTrees );
}


int sphMarkCommonSubtrees ( const CSphVector<XQNode_t*> & dTrees )
{
	if ( !dTrees.GetLength() )
		return 0;

	{ // Optional reorganize tree to extract common parts
		RevealCommon_t ( SPH_QUERY_AND ).transform ( dTrees );
		RevealCommon_t ( SPH_QUERY_OR ).transform ( dTrees );
	}

	// flag common subtrees and refcount them
	CSubtreeHash hSubtrees;
	ARRAY_FOREACH ( i, dTrees )
		FlagCommonSubtrees ( dTrees[i], hSubtrees );

	// number marked subtrees and assign them order numbers.
	int iOrder = 0;
	hSubtrees.IterateStart();
	while ( hSubtrees.IterateNext() )
		if ( hSubtrees.IterateGet().m_bMarked )
			hSubtrees.IterateGet().m_iOrder = iOrder++;

	// copy the flags and orders to original trees
	ARRAY_FOREACH ( i, dTrees )
		SignCommonSubtrees ( dTrees[i], hSubtrees );

	return iOrder;
}

//
// $Id$
//
