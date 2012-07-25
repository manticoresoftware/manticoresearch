//
// $Id$
//

//
// Copyright (c) 2001-2012, Andrew Aksyonoff
// Copyright (c) 2008-2012, Sphinx Technologies Inc
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
					~XQParser_t ();

public:
	bool			Parse ( XQQuery_t & tQuery, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict, const CSphIndexSettings & tSettings );

	bool			Error ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
	void			Warning ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );

	bool			AddField ( CSphSmallBitvec & dFields, const char * szField, int iLen );
	bool			ParseFields ( CSphSmallBitvec & uFields, int & iMaxFieldPos );
	int				ParseZone ( const char * pZone );

	bool			IsSpecial ( char c );
	int				GetToken ( YYSTYPE * lvalp );

	void			AddQuery ( XQNode_t * pNode );
	XQNode_t *		AddKeyword ( const char * sKeyword, DWORD uStar = STAR_NONE );
	XQNode_t *		AddKeyword ( XQNode_t * pLeft, XQNode_t * pRight );
	XQNode_t *		AddOp ( XQOperator_e eOp, XQNode_t * pLeft, XQNode_t * pRight, int iOpArg=0 );

	void			Cleanup ();
	XQNode_t *		SweepNulls ( XQNode_t * pNode );
	bool			FixupNots ( XQNode_t * pNode );
	void			DeleteNodesWOFields ( XQNode_t * pNode );

	inline void SetFieldSpec ( const CSphSmallBitvec& uMask, int iMaxPos )
	{
		FixRefSpec();
		m_dStateSpec.Last()->SetFieldSpec ( uMask, iMaxPos );
	}
	inline void SetZoneVec ( int iZoneVec, bool bZoneSpan = false )
	{
		FixRefSpec();
		m_dStateSpec.Last()->SetZoneSpec ( m_dZoneVecs[iZoneVec], bZoneSpan );
	}

	inline void FixRefSpec ()
	{
		bool bRef = ( m_dStateSpec.GetLength()>1 && ( m_dStateSpec[m_dStateSpec.GetLength()-1]==m_dStateSpec[m_dStateSpec.GetLength()-2] ) );
		if ( !bRef )
			return;

		XQLimitSpec_t * pSpec = m_dStateSpec.Pop();
		m_dSpecPool.Add ( new XQLimitSpec_t ( *pSpec ) );
		m_dStateSpec.Add ( m_dSpecPool.Last() );
	}

public:
	const CSphVector<int> & GetZoneVec ( int iZoneVec ) const
	{
		return m_dZoneVecs[iZoneVec];
	}

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
	bool					m_bEmptyStopword;
	int						m_iOvershortStep;

	CSphVector<CSphString>	m_dIntTokens;

	CSphVector < CSphVector<int> >	m_dZoneVecs;
	CSphVector<XQLimitSpec_t *>		m_dStateSpec;
	CSphVector<XQLimitSpec_t *>		m_dSpecPool;
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

void XQLimitSpec_t::SetFieldSpec ( const CSphSmallBitvec& uMask, int iMaxPos )
{
	m_bFieldSpec = true;
	m_dFieldMask = uMask;
	m_iFieldMaxPos = iMaxPos;
}

void XQNode_t::SetFieldSpec ( const CSphSmallBitvec& uMask, int iMaxPos )
{
	// set it, if we do not yet have one
	if ( !m_dSpec.m_bFieldSpec )
		m_dSpec.SetFieldSpec ( uMask, iMaxPos );

	// some of the children might not yet have a spec, even if the node itself has
	// eg. 'hello @title world' (whole node has '@title' spec but 'hello' node does not have any!)
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->SetFieldSpec ( uMask, iMaxPos );
}

void XQLimitSpec_t::SetZoneSpec ( const CSphVector<int> & dZones, bool bZoneSpan )
{
	m_dZones = dZones;
	m_bZoneSpan = bZoneSpan;
}


void XQNode_t::SetZoneSpec ( const CSphVector<int> & dZones, bool bZoneSpan )
{
	// set it, if we do not yet have one
	if ( !m_dSpec.m_dZones.GetLength() )
		m_dSpec.SetZoneSpec ( dZones, bZoneSpan );

	// some of the children might not yet have a spec, even if the node itself has
	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->SetZoneSpec ( dZones, bZoneSpan );
}

void XQNode_t::CopySpecs ( const XQNode_t * pSpecs )
{
	if ( !pSpecs )
		return;

	if ( !m_dSpec.m_bFieldSpec )
		m_dSpec.SetFieldSpec ( pSpecs->m_dSpec.m_dFieldMask, pSpecs->m_dSpec.m_iFieldMaxPos );

	if ( !m_dSpec.m_dZones.GetLength() )
		m_dSpec.SetZoneSpec ( pSpecs->m_dSpec.m_dZones, pSpecs->m_dSpec.m_bZoneSpan );
}


void XQNode_t::ClearFieldMask ()
{
	m_dSpec.m_dFieldMask.Set();

	ARRAY_FOREACH ( i, m_dChildren )
		m_dChildren[i]->ClearFieldMask();
}


bool XQNode_t::IsEqualTo ( const XQNode_t * pNode )
{
	if ( !pNode || pNode->GetHash()!=GetHash() || pNode->GetOp()!=GetOp() )
		return false;

	if ( m_dWords.GetLength() )
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
	, m_bEmptyStopword ( false )
{
	m_dSpecPool.Add ( new XQLimitSpec_t() );
	m_dStateSpec.Add ( m_dSpecPool.Last() );
}

XQParser_t::~XQParser_t ()
{
	ARRAY_FOREACH ( i, m_dSpecPool )
		SafeDelete ( m_dSpecPool[i] );
}


/// cleanup spawned nodes (for bailing out on errors)
void XQParser_t::Cleanup ()
{
	m_dSpawned.Uniq(); // FIXME! should eliminate this by testing

	ARRAY_FOREACH ( i, m_dSpawned )
	{
		m_dSpawned[i]->m_dChildren.Reset ();
		SafeDelete ( m_dSpawned[i] );
	}
	m_dSpawned.Reset ();
	m_dStateSpec.Reset();
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
bool XQParser_t::AddField ( CSphSmallBitvec & dFields, const char * szField, int iLen )
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
		if ( iField>=SPH_MAX_FIELDS )
			return Error ( " max %d fields allowed", SPH_MAX_FIELDS );

		dFields.Set(iField);
	}

	return true;
}


/// parse fields block
bool XQParser_t::ParseFields ( CSphSmallBitvec & dFields, int & iMaxFieldPos )
{
	dFields.Unset();
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
		dFields.Set();
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
		if ( !AddField ( dFields, pFieldStart, pPtr-pFieldStart ) )
			return false;

		m_pTokenizer->SetBufferPtr ( pPtr );
		if ( bNegate && ( !dFields.TestAll() ) )
			dFields.Negate();

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
				CSphString sContext;
				sContext.SetBinary ( pPtr, (int)( pLastPtr-pPtr ) );
				return Error ( "invalid field block operator syntax near '%s'", sContext.cstr() ? sContext.cstr() : "" );

			} else if ( *pPtr==',' )
			{
				if ( !AddField ( dFields, pFieldStart, pPtr-pFieldStart ) )
					return false;

				pFieldStart = NULL;
				pPtr++;

			} else if ( *pPtr==')' )
			{
				if ( !AddField ( dFields, pFieldStart, pPtr-pFieldStart ) )
					return false;

				m_pTokenizer->SetBufferPtr ( ++pPtr );
				if ( bNegate && ( !dFields.TestAll() ) )
					dFields.Negate();

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


/// helper find-or-add (make it generic and move to sphinxstd?)
static int GetZoneIndex ( XQQuery_t * pQuery, const CSphString & sZone )
{
	ARRAY_FOREACH ( i, pQuery->m_dZones )
		if ( pQuery->m_dZones[i]==sZone )
			return i;

	pQuery->m_dZones.Add ( sZone );
	return pQuery->m_dZones.GetLength()-1;
}


/// parse zone
int XQParser_t::ParseZone ( const char * pZone )
{
	const char * p = pZone;

	// case one, just a single zone name
	if ( sphIsAlpha ( *pZone ) )
	{
		// find zone name
		while ( sphIsAlpha(*p) )
			p++;
		m_pTokenizer->SetBufferPtr ( p );

		// extract and lowercase it
		CSphString sZone;
		sZone.SetBinary ( pZone, p-pZone );
		sZone.ToLower();

		// register it in zones list
		int iZone = GetZoneIndex ( m_pParsed, sZone );

		// create new 1-zone vector
		m_dZoneVecs.Add().Add ( iZone );
		return m_dZoneVecs.GetLength()-1;
	}

	// case two, zone block
	// it must follow strict (name1,name2,...) syntax
	if ( *pZone=='(' )
	{
		// create new zone vector
		CSphVector<int> & dZones = m_dZoneVecs.Add();
		p = ++pZone;

		// scan names
		for ( ;; )
		{
			// syntax error, name expected!
			if ( !sphIsAlpha(*p) )
			{
				Error ( "unexpected character '%c' in zone block operator", *p );
				return -1;
			}

			// scan next name
			while ( sphIsAlpha(*p) )
				p++;

			// extract and lowercase it
			CSphString sZone;
			sZone.SetBinary ( pZone, p-pZone );
			sZone.ToLower();

			// register it in zones list
			dZones.Add ( GetZoneIndex ( m_pParsed, sZone ) );

			// must be either followed by comma, or closing paren
			// everything else will cause syntax error
			if ( *p==')' )
			{
				m_pTokenizer->SetBufferPtr ( p+1 );
				break;
			}

			if ( *p==',' )
				pZone = ++p;
		}

		return m_dZoneVecs.GetLength()-1;
	}

	// unhandled case
	Error ( "internal error, unhandled case in ParseZone()" );
	return -1;
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

		static const int NUMBER_BUF_LEN = 10; // max strlen of int32
		if ( p>sToken && p-sToken<NUMBER_BUF_LEN
			&& !( *p=='-' && !( p-sToken==1 && sphIsModifier ( p[-1] ) ) ) // !bDashInside copied over from arbitration
			&& ( *p=='\0' || sphIsSpace(*p) || IsSpecial(*p) ) )
		{
			if ( m_pTokenizer->GetToken() && m_pTokenizer->TokenIsBlended() ) // number with blended should be tokenized as usual
			{
				m_pTokenizer->SkipBlended();
				m_pTokenizer->SetBufferPtr ( m_pLastTokenStart );
			} else
			{
				// got not a very long number followed by a whitespace or special, handle it
				char sNumberBuf[NUMBER_BUF_LEN];

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
		}

		// not a number, long number, or number not followed by a whitespace, so fallback to regular tokenizing
		sToken = (const char *) m_pTokenizer->GetToken ();
		if ( !sToken )
		{
			m_iPendingNulls = m_pTokenizer->GetOvershortCount() * m_iOvershortStep;
			if ( !m_iPendingNulls )
				return 0;
			m_iPendingNulls = 0;
			lvalp->pNode = AddKeyword ( NULL );
			return TOK_KEYWORD;
		}

		// now let's do some token post-processing
		m_bWasBlended = m_pTokenizer->TokenIsBlended();
		m_bEmpty = false;

		m_iPendingNulls = m_pTokenizer->GetOvershortCount() * m_iOvershortStep;
		m_iAtomPos += 1+m_iPendingNulls;

		// handle NEAR (must be case-sensitive, and immediately followed by slash and int)
		if ( sToken && p && !m_pTokenizer->m_bPhrase && strncmp ( p, "NEAR/", 5 )==0 && isdigit(p[5]) )
		{
			// extract that int
			int iVal = 0;
			for ( p=p+5; isdigit(*p); p++ )
				iVal = iVal*10 + (*p) - '0'; // FIXME! check for overflow?
			m_pTokenizer->SetBufferPtr ( p );

			// we just lexed our next token
			m_iPendingType = TOK_NEAR;
			m_tPendingToken.tInt.iValue = iVal;
			m_tPendingToken.tInt.iStrIndex = -1;
			m_iAtomPos -= 1; // skip NEAR
			break;
		}

		// handle SENTENCE
		if ( sToken && p && !m_pTokenizer->m_bPhrase && !strcasecmp ( sToken, "sentence" ) && !strncmp ( p, "SENTENCE", 8 ) )
		{
			// we just lexed our next token
			m_iPendingType = TOK_SENTENCE;
			m_iAtomPos -= 1;
			break;
		}

		// handle PARAGRAPH
		if ( sToken && p && !m_pTokenizer->m_bPhrase && !strcasecmp ( sToken, "paragraph" ) && !strncmp ( p, "PARAGRAPH", 9 ) )
		{
			// we just lexed our next token
			m_iPendingType = TOK_PARAGRAPH;
			m_iAtomPos -= 1;
			break;
		}

		// handle ZONE
		if ( sToken && p && !m_pTokenizer->m_bPhrase && !strncmp ( p, "ZONE:", 5 )
			&& ( sphIsAlpha(p[5]) || p[5]=='(' ) )
		{
			// ParseZone() will update tokenizer buffer ptr as needed
			int iVec = ParseZone ( p+5 );
			if ( iVec<0 )
				return -1;

			// we just lexed our next token
			m_iPendingType = TOK_ZONE;
			m_tPendingToken.iZoneVec = iVec;
			m_iAtomPos -= 1;
			break;
		}

		// handle ZONESPAN
		if ( sToken && p && !m_pTokenizer->m_bPhrase && !strncmp ( p, "ZONESPAN:", 9 )
			&& ( sphIsAlpha(p[9]) || p[9]=='(' ) )
		{
			// ParseZone() will update tokenizer buffer ptr as needed
			int iVec = ParseZone ( p+9 );
			if ( iVec<0 )
				return -1;

			// we just lexed our next token
			m_iPendingType = TOK_ZONESPAN;
			m_tPendingToken.iZoneVec = iVec;
			m_iAtomPos -= 1;
			break;
		}

		// handle specials
		if ( m_pTokenizer->WasTokenSpecial() )
		{
			// specials must not affect pos
			m_iAtomPos--;

			// some specials are especially special
			if ( sToken[0]=='@' )
			{
				// parse fields operator
				if ( !ParseFields ( m_tPendingToken.tFieldLimit.dMask, m_tPendingToken.tFieldLimit.iMaxPos ) )
					return -1;

				if ( m_pSchema->m_dFields.GetLength()!=SPH_MAX_FIELDS )
					m_tPendingToken.tFieldLimit.dMask.LimitBits ( m_pSchema->m_dFields.GetLength() );

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

				if ( sToken[0]=='(' )
				{
					m_dStateSpec.Add ( m_dStateSpec.Last() );
				} else if ( sToken[0]==')' )
				{
					m_dStateSpec.Pop();
				}
				assert ( m_dStateSpec.GetLength()>=1 );

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
		{
			sToken = NULL;
			// stopwords with step=0 must not affect pos
			if ( m_bEmptyStopword )
				m_iAtomPos--;
		}

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

	XQNode_t * pNode = new XQNode_t ( *m_dStateSpec.Last() );
	pNode->m_dWords.Add ( tAW );

	m_dSpawned.Add ( pNode );
	return pNode;
}


XQNode_t * XQParser_t::AddKeyword ( XQNode_t * pLeft, XQNode_t * pRight )
{
	if ( !pLeft || !pRight )
		return pLeft ? pLeft : pRight;

	assert ( pLeft->m_dWords.GetLength()>0 );
	assert ( pRight->m_dWords.GetLength()==1 );

	pLeft->m_dWords.Add ( pRight->m_dWords[0] );
	m_dSpawned.RemoveValue ( pRight );
	SafeDelete ( pRight );
	return pLeft;
}


XQNode_t * XQParser_t::AddOp ( XQOperator_e eOp, XQNode_t * pLeft, XQNode_t * pRight, int iOpArg )
{
	/////////
	// unary
	/////////

	if ( eOp==SPH_QUERY_NOT )
	{
		XQNode_t * pNode = new XQNode_t ( *m_dStateSpec.Last() );
		pNode->SetOp ( SPH_QUERY_NOT, pLeft );
		m_dSpawned.Add ( pNode );
		return pNode;
	}

	//////////
	// binary
	//////////

	if ( !pLeft || !pRight )
		return pLeft ? pLeft : pRight;

	// left spec always tries to infect the nodes to the right, only brackets can stop it
	// eg. '@title hello' vs 'world'
	pRight->CopySpecs ( pLeft );

	// build a new node
	XQNode_t * pResult = NULL;
	if ( pLeft->m_dChildren.GetLength() && pLeft->GetOp()==eOp && pLeft->m_iOpArg==iOpArg )
	{
		pLeft->m_dChildren.Add ( pRight );
		pResult = pLeft;
		if ( pRight->m_dSpec.m_bFieldSpec )
			pResult->m_dSpec.SetFieldSpec ( pRight->m_dSpec.m_dFieldMask, pRight->m_dSpec.m_iFieldMaxPos );

		if ( pRight->m_dSpec.m_dZones.GetLength() )
			pResult->m_dSpec.SetZoneSpec ( pRight->m_dSpec.m_dZones, pRight->m_dSpec.m_bZoneSpan );
	} else
	{
		// however, it's right (!) spec which is chosen for the resulting node,
		// eg. '@title hello' + 'world @body program'
		XQNode_t * pNode = new XQNode_t ( pRight->m_dSpec );
		pNode->SetOp ( eOp, pLeft, pRight );
		pNode->m_iOpArg = iOpArg;
		m_dSpawned.Add ( pNode );
		pResult = pNode;
	}
	return pResult;
}


XQNode_t * XQParser_t::SweepNulls ( XQNode_t * pNode )
{
	if ( !pNode )
		return NULL;

	// sweep plain node
	if ( pNode->m_dWords.GetLength() )
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
	if ( !pNode || pNode->m_dWords.GetLength() )
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

	XQNode_t * pAnd = new XQNode_t ( pNode->m_dSpec );
	pAnd->SetOp ( SPH_QUERY_AND, pNode->m_dChildren );
	m_dSpawned.Add ( pAnd );

	XQNode_t * pNot = NULL;
	if ( dNots.GetLength()==1 )
	{
		pNot = dNots[0];
	} else
	{
		pNot = new XQNode_t ( pNode->m_dSpec );
		pNot->SetOp ( SPH_QUERY_OR, dNots );
		m_dSpawned.Add ( pNot );
	}

	pNode->SetOp ( SPH_QUERY_ANDNOT, pAnd, pNot );
	return true;
}


void XQParser_t::DeleteNodesWOFields ( XQNode_t * pNode )
{
	if ( !pNode )
		return;

	for ( int i = 0; i < pNode->m_dChildren.GetLength (); )
	{
		if ( pNode->m_dChildren[i]->m_dSpec.m_dFieldMask.TestAll() )
		{
			XQNode_t * pChild = pNode->m_dChildren[i];
			assert ( pChild->m_dChildren.GetLength()==0 );
			m_dSpawned.RemoveValue ( pChild );

			// this should be a leaf node
			SafeDelete ( pNode->m_dChildren[i] );
			pNode->m_dChildren.RemoveFast ( i );

		} else
		{
			DeleteNodesWOFields ( pNode->m_dChildren[i] );
			i++;
		}
	}
}


static bool CheckQuorum ( XQNode_t * pNode, CSphString * pError )
{
	assert ( pError );
	if ( !pNode )
		return true;

	if ( pNode->GetOp()==SPH_QUERY_QUORUM && pNode->m_iOpArg<=0 )
	{
		pError->SetSprintf ( "quorum threshold too low (%d)", pNode->m_iOpArg );
		return false;
	}

	bool bValid = true;
	ARRAY_FOREACH_COND ( i, pNode->m_dChildren, bValid )
	{
		bValid &= CheckQuorum ( pNode->m_dChildren[i], pError );
	}

	return bValid;
}


static void FixupDegenerates ( XQNode_t * pNode )
{
	if ( !pNode )
		return;

	if ( pNode->m_dWords.GetLength()==1 &&
		( pNode->GetOp()==SPH_QUERY_PHRASE || pNode->GetOp()==SPH_QUERY_PROXIMITY || pNode->GetOp()==SPH_QUERY_QUORUM ) )
	{
		pNode->SetOp ( SPH_QUERY_AND );
		return;
	}

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		FixupDegenerates ( pNode->m_dChildren[i] );
}


bool XQParser_t::Parse ( XQQuery_t & tParsed, const char * sQuery, const ISphTokenizer * pTokenizer,
	const CSphSchema * pSchema, CSphDict * pDict, const CSphIndexSettings & tSettings )
{
	CSphScopedPtr<ISphTokenizer> pMyTokenizer ( pTokenizer->Clone ( true ) );
	pMyTokenizer->AddSpecials ( "()|-!@~\"/^$<" );
	pMyTokenizer->AddPlainChar ( '?' );
	pMyTokenizer->AddPlainChar ( '%' );
	pMyTokenizer->EnableQueryParserMode ( true );

	// most outcomes are errors
	SafeDelete ( tParsed.m_pRoot );

	// check for relaxed syntax
	const char * OPTION_RELAXED = "@@relaxed";
	const int OPTION_RELAXED_LEN = strlen ( OPTION_RELAXED );

	m_bStopOnInvalid = true;
	if ( sQuery && strncmp ( sQuery, OPTION_RELAXED, OPTION_RELAXED_LEN )==0 && !sphIsAlpha ( sQuery[OPTION_RELAXED_LEN] ) )
	{
		sQuery += OPTION_RELAXED_LEN;
		m_bStopOnInvalid = false;
	}

	// setup parser
	m_pParsed = &tParsed;
	m_sQuery = (BYTE*) sQuery;
	m_iQueryLen = sQuery ? strlen(sQuery) : 0;
	m_pTokenizer = pMyTokenizer.Ptr();
	m_pSchema = pSchema;
	m_pDict = pDict;
	m_pCur = sQuery;
	m_iAtomPos = 0;
	m_iPendingNulls = 0;
	m_iPendingType = 0;
	m_pRoot = NULL;
	m_bEmpty = true;
	m_bEmptyStopword = ( tSettings.m_iStopwordStep==0 );
	m_iOvershortStep = tSettings.m_iOvershortStep;

	m_pTokenizer->SetBuffer ( m_sQuery, m_iQueryLen );
	int iRes = yyparse ( this );

	if ( ( iRes || !m_pParsed->m_sParseError.IsEmpty() ) && !m_bEmpty )
	{
		Cleanup ();
		return false;
	}

	DeleteNodesWOFields ( m_pRoot );
	m_pRoot = SweepNulls ( m_pRoot );
	FixupDegenerates ( m_pRoot );

	if ( !FixupNots ( m_pRoot ) )
	{
		Cleanup ();
		return false;
	}

	if ( !CheckQuorum ( m_pRoot, &m_pParsed->m_sParseError ) )
	{
		Cleanup();
		return false;
	}

	if ( m_pRoot && m_pRoot->GetOp()==SPH_QUERY_NOT )
	{
		Cleanup ();
		m_pParsed->m_sParseError.SetSprintf ( "query is non-computable (single NOT operator)" );
		return false;
	}

	// all ok; might want to create a dummy node to indicate that
	m_dSpawned.Reset();
	tParsed.m_pRoot = m_pRoot ? m_pRoot : new XQNode_t ( *m_dStateSpec.Last() );
	return true;
}

//////////////////////////////////////////////////////////////////////////

#define XQDEBUG 0

#if XQDEBUG
static void xqIndent ( int iIndent )
{
	iIndent *= 2;
	while ( iIndent-- )
		printf ( "|-" );
}


static void xqDump ( XQNode_t * pNode, int iIndent )
{
	if ( pNode->m_dChildren.GetLength() )
	{
		xqIndent ( iIndent );
		switch ( pNode->GetOp() )
		{
			case SPH_QUERY_AND: printf ( "AND:\n" ); break;
			case SPH_QUERY_OR: printf ( "OR:\n" ); break;
			case SPH_QUERY_NOT: printf ( "NOT:\n" ); break;
			case SPH_QUERY_ANDNOT: printf ( "ANDNOT:\n" ); break;
			case SPH_QUERY_BEFORE: printf ( "BEFORE:\n" ); break;
			case SPH_QUERY_PHRASE: printf ( "PHRASE:\n" ); break;
			case SPH_QUERY_PROXIMITY: printf ( "PROXIMITY:\n" ); break;
			case SPH_QUERY_QUORUM: printf ( "QUORUM:\n" ); break;
			case SPH_QUERY_NEAR: printf ( "NEAR:\n" ); break;
			case SPH_QUERY_SENTENCE: printf ( "SENTENCE:\n" ); break;
			case SPH_QUERY_PARAGRAPH: printf ( "PARAGRAPH:\n" ); break;
			default: printf ( "unknown-op-%d:\n", pNode->GetOp() ); break;
		}
		ARRAY_FOREACH ( i, pNode->m_dChildren )
			xqDump ( pNode->m_dChildren[i], iIndent+1 );
	} else
	{
		xqIndent ( iIndent );
		printf ( "MATCH(%d,%d):", pNode->m_dSpec.m_dFieldMask.GetMask32(), pNode->m_iOpArg );

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


bool sphParseExtendedQuery ( XQQuery_t & tParsed, const char * sQuery, const ISphTokenizer * pTokenizer,
	const CSphSchema * pSchema, CSphDict * pDict, const CSphIndexSettings & tSettings )
{
	XQParser_t qp;
	bool bRes = qp.Parse ( tParsed, sQuery, pTokenizer, pSchema, pDict, tSettings );

#ifndef NDEBUG
	if ( bRes && tParsed.m_pRoot )
		tParsed.m_pRoot->Check ( true );
#endif

#if XQDEBUG
	if ( bRes )
	{
		printf ( "--- query ---\n" );
		printf ( "%s\n", sQuery );
		xqDump ( tParsed.m_pRoot, 0 );
		printf ( "---\n" );
	}
#endif

	// moved here from ranker creation
	// as at that point term expansion could produce many terms from expanded term and this condition got failed
	tParsed.m_bSingleWord = ( tParsed.m_pRoot && tParsed.m_pRoot->m_dChildren.GetLength()==0 && tParsed.m_pRoot->m_dWords.GetLength()==1 );

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
	return !( pTree->m_dWords.GetLength()==1 && pTree->GetOp()!=SPH_QUERY_NOT );
}

typedef CSphOrderedHash < DWORD, uint64_t, IdentityHash_fn, 128 > CDwordHash;

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
	: public CSphOrderedHash < Associations_t, uint64_t, IdentityHash_fn, 128 >
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
			CSphOrderedHash < XQNode_t*, int, IdentityHash_fn, 64 > hBranches;
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
								pNode = new XQNode_t ( pTree->m_dSpec );
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
						pOtherChildren = new XQNode_t ( pTree->m_dSpec );
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
	void Transform ( int iXQ, const XQQuery_t * pXQ )
	{
		// collect all non-unique nodes
		for ( int i=0; i<iXQ; i++ )
			if ( !BuildAssociations ( pXQ[i].m_pRoot ) )
				return;

		// count and order all non-unique nodes
		if ( !CalcCommonNodes() )
			return;

		// create and collect bitmask for every node
		for ( int i=0; i<iXQ; i++ )
			BuildBitmasks ( pXQ[i].m_pRoot );

		// intersect all bitmasks one-by-one, and also intersect all intersections
		CalcIntersections();

		// the die-hard: actually select the set of subtrees which we'll process
		MakeQueries();

		// ... and finally - process all our trees.
		for ( int i=0; i<iXQ; i++ )
			Reorganize ( pXQ[i].m_pRoot );
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

typedef CSphOrderedHash < MarkedNode_t, uint64_t, IdentityHash_fn, 128 > CSubtreeHash;

/// check hashes, then check subtrees, then flag
static void FlagCommonSubtrees ( XQNode_t * pTree, CSubtreeHash & hSubTrees, bool bFlag=true, bool bMarkIt=true )
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


int sphMarkCommonSubtrees ( int iXQ, const XQQuery_t * pXQ )
{
	if ( iXQ<=0 || !pXQ )
		return 0;

	{ // Optional reorganize tree to extract common parts
		RevealCommon_t ( SPH_QUERY_AND ).Transform ( iXQ, pXQ );
		RevealCommon_t ( SPH_QUERY_OR ).Transform ( iXQ, pXQ );
	}

	// flag common subtrees and refcount them
	CSubtreeHash hSubtrees;
	for ( int i=0; i<iXQ; i++ )
		FlagCommonSubtrees ( pXQ[i].m_pRoot, hSubtrees );

	// number marked subtrees and assign them order numbers.
	int iOrder = 0;
	hSubtrees.IterateStart();
	while ( hSubtrees.IterateNext() )
		if ( hSubtrees.IterateGet().m_bMarked )
			hSubtrees.IterateGet().m_iOrder = iOrder++;

	// copy the flags and orders to original trees
	for ( int i=0; i<iXQ; i++ )
		SignCommonSubtrees ( pXQ[i].m_pRoot, hSubtrees );

	return iOrder;
}

//
// $Id$
//
