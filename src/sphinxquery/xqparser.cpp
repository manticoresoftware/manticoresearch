//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "xqparser.h"
#include "parse_helper.h"
#include "sphinxplugin.h"
#include "tokenizer/tokenizer.h"
#include "dict/dict_base.h"
#include "xqdebug.h"

//////////////////////////////////////////////////////////////////////////
// EXTENDED PARSER RELOADED
//////////////////////////////////////////////////////////////////////////
class XQParser_t;
#include "bissphinxquery.h"

static bool g_bOnlyNotAllowed = false;

void AllowOnlyNot ( bool bAllowed )
{
	g_bOnlyNotAllowed = bAllowed;
}

bool IsAllowOnlyNot ()
{
	return g_bOnlyNotAllowed;
}

namespace {
/// helper find-or-add (make it generic and move to sphinxstd?)
int GetZoneIndex ( XQQuery_t * pQuery, const CSphString & sZone )
{
	ARRAY_FOREACH ( i, pQuery->m_dZones )
		if ( pQuery->m_dZones[i] == sZone )
			return i;

	pQuery->m_dZones.Add ( sZone );
	return pQuery->m_dZones.GetLength() - 1;
}

bool HasMissedField ( const XQLimitSpec_t & tSpec )
{
	return (tSpec.m_bFieldSpec
		&& tSpec.m_dFieldMask.TestAll ( false )
		);
}
}

//////////////////////////////////////////////////////////////////////////

class XQParser_t : public XQParseHelper_c
{
	friend void yyerror ( XQParser_t * pParser, const char * sMessage );

	friend int yyparse (XQParser_t * pParser);

public:
					XQParser_t();
					~XQParser_t() override;

public:
	bool			Parse ( XQQuery_t & tQuery, const char * sQuery, const CSphQuery * pQuery, const TokenizerRefPtr_c & pTokenizer, const CSphSchema * pSchema, const DictRefPtr_c & pDict, const CSphIndexSettings & tSettings, const CSphBitvec * pMorphFields );
	int				ParseZone ( const char * pZone );

	bool			IsSpecial ( char c );
	bool			GetNumber ( const char * p, const char * sRestart );
	int				GetToken ( YYSTYPE * lvalp );

	void			HandleModifiers ( XQKeyword_t & tKeyword ) const noexcept;

	void			AddQuery ( XQNode_t * pNode );
	XQNode_t *		AddKeyword ( const char * sKeyword, int iSkippedPosBeforeToken=0 );
	XQNode_t *		AddKeyword ( XQNode_t * pLeft, XQNode_t * pRight );
	XQNode_t *		AddOp ( XQOperator_e eOp, XQNode_t * pLeft, XQNode_t * pRight, int iOpArg=0 );
	void			SetPhrase ( XQNode_t * pNode, bool bSetExact, XQOperator_e eOp );
	void			PhraseShiftQpos ( XQNode_t * pNode );
	XQNode_t *		AddPhraseKeyword ( XQNode_t * pLeft, XQNode_t * pRight );

	void	Cleanup () override;

	void SetFieldSpec ( const FieldMask_t& uMask, int iMaxPos )
	{
		FixRefSpec();
		m_dStateSpec.Last()->SetFieldSpec ( uMask, iMaxPos );
	}
	void SetZoneVec ( int iZoneVec, bool bZoneSpan = false )
	{
		FixRefSpec();
		m_dStateSpec.Last()->SetZoneSpec ( m_dZoneVecs[iZoneVec], bZoneSpan );
	}

	void FixRefSpec ()
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
	BYTE *					m_sQuery = nullptr;
	int						m_iQueryLen = 0;
	const char *			m_pErrorAt = nullptr;

	XQNode_t *				m_pRoot = nullptr;

	int						m_iPendingNulls = 0;
	int						m_iPendingType = 0;
	YYSTYPE					m_tPendingToken;
	bool					m_bWasKeyword = false;

	bool					m_bEmpty = false;
	bool					m_bWasFullText = false;
	bool					m_bQuoted = false;
	int						m_iOvershortStep = 0;

	int						m_iQuorumQuote = -1;
	int						m_iQuorumFSlash = -1;
	bool					m_bCheckNumber = false;

	StrVec_t				m_dIntTokens;

	CSphVector < CSphVector<int> >	m_dZoneVecs;
	CSphVector<XQLimitSpec_t *>		m_dStateSpec;
	CSphVector<XQLimitSpec_t *>		m_dSpecPool;
	IntVec_t						m_dPhraseStar;

protected:
	bool			HandleFieldBlockStart ( const char * & pPtr ) override;

private:
	XQNode_t *		ParseRegex ( const char * pStart );
};

//////////////////////////////////////////////////////////////////////////

static int yylex ( YYSTYPE * lvalp, XQParser_t * pParser )
{
	return pParser->GetToken ( lvalp );
}

void yyerror ( XQParser_t * pParser, const char * sMessage )
{
	if ( pParser->m_pParsed->m_sParseError.IsEmpty() )
		pParser->m_pParsed->m_sParseError.SetSprintf ( "P08: %s near '%s'", sMessage, pParser->m_pErrorAt );
}

#include "bissphinxquery.c"

//////////////////////////////////////////////////////////////////////////

XQParser_t::XQParser_t ()
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
	XQParseHelper_c::Cleanup();
	m_dStateSpec.Reset();
}


/// my special chars
bool XQParser_t::IsSpecial ( char c )
{
	return c=='(' || c==')' || c=='|' || c=='-' || c=='!' || c=='@' || c=='~' || c=='"' || c=='/';
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
		sZone.SetBinary ( pZone, int(p-pZone) );
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
		while (true)
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
			sZone.SetBinary ( pZone, int(p-pZone) );
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

XQNode_t * XQParser_t::ParseRegex ( const char * sStart )
{
	assert ( sStart );
	int iDel = *sStart++;
	const char * sEnd = m_pTokenizer->GetBufferEnd ();
	const char * sToken = sStart;

	while ( sStart<sEnd )
	{
		const char * sNextDel = (const char *)memchr ( sStart, iDel, sEnd-sStart );
		if ( !sNextDel )
			break;

		if ( sNextDel+1<sEnd && sNextDel[1]==')' )
		{
			// spawn token node
			XQNode_t * pNode = AddKeyword ( nullptr, 0 );
			pNode->WithWords ( [sToken,sNextDel] ( auto& dWords) { // fresh!
				dWords.First().m_sWord.SetBinary ( sToken, sNextDel-sToken );
				dWords.First().m_bRegex = true;
			});

			// skip the whole expression
			m_pTokenizer->SetBufferPtr ( sNextDel+2 );
			return pNode;
		}
		sStart = sNextDel+1;
	}

	// not a complete REGEX(/term/)
	return nullptr;
}


bool XQParser_t::GetNumber ( const char * p, const char * sRestart )
{
	int iDots = 0;
	const char * sToken = p;
	const char * sEnd = m_pTokenizer->GetBufferEnd ();
	while ( p<sEnd && ( isdigit ( *(const BYTE*)p ) || *p=='.' ) )
	{
		iDots += ( *p=='.' );
		p++;
	}

	// must be float number but got many dots or only dot
	if ( iDots && ( iDots>1 || p-sToken==iDots ) )
		p = sToken;

	// float as number allowed only as quorum argument and regular keywords stream otherwise
	if ( iDots==1 && ( m_iQuorumQuote!=m_iQuorumFSlash || m_iQuorumQuote!=m_iAtomPos ) )
		p = sToken;

	static const int NUMBER_BUF_LEN = 10; // max strlen of int32
	if ( p>sToken && p-sToken<NUMBER_BUF_LEN
		&& !( *p=='-' && !( p-sToken==1 && sphIsModifier ( p[-1] ) ) ) // !bDashInside copied over from arbitration
		&& ( *p=='\0' || sphIsSpace(*p) || IsSpecial(*p) ) )
	{
		// float as quorum argument has higher precedence than blended
		bool bQuorum = ( m_iQuorumQuote==m_iQuorumFSlash && m_iQuorumFSlash==m_iAtomPos );
		bool bQuorumPercent = ( bQuorum && iDots==1 );

		bool bTok = ( m_pTokenizer->GetToken()!=NULL );
		if ( bTok && m_pTokenizer->TokenIsBlended() && !( bQuorum || bQuorumPercent ) ) // number with blended should be tokenized as usual
		{
			m_pTokenizer->SkipBlended();
			m_pTokenizer->SetBufferPtr ( sRestart );
		} else if ( bTok && m_pTokenizer->WasTokenSynonym() && !( bQuorum || bQuorumPercent ) )
		{
			m_pTokenizer->SetBufferPtr ( sRestart );
		} else
		{
			// got not a very long number followed by a whitespace or special, handle it
			char sNumberBuf[NUMBER_BUF_LEN];

			int iNumberLen = Min ( (int)sizeof(sNumberBuf)-1, int(p-sToken) );
			memcpy ( sNumberBuf, sToken, iNumberLen );
			sNumberBuf[iNumberLen] = '\0';
			if ( iDots )
				m_tPendingToken.tInt.fValue = (float)strtod ( sNumberBuf, NULL );
			else
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
				if ( m_pDict->GetWordID ( (BYTE*)const_cast<char*>(sToken) ) )
					m_tPendingToken.tInt.iStrIndex = m_dIntTokens.GetLength()-1;
				else
					m_dIntTokens.Pop();
				m_iAtomPos++;
			}

			m_iPendingNulls = 0;
			m_iPendingType = iDots ? TOK_FLOAT : TOK_INT;
			return true;
		}
	}
	return false;
}

static bool GetNearToken ( const char * sTok, int iTokLen, int iTokType, const char * sBuf, const TokenizerRefPtr_c& pTokenizer, int & iPendingType, YYSTYPE & tPendingToken )
{
	if ( strncmp ( sBuf, sTok, iTokLen )==0 && isdigit(sBuf[iTokLen]) )
	{
		// extract that int
		int iVal = 0;
		for ( sBuf=sBuf+iTokLen; isdigit ( *sBuf ); sBuf++ )
			iVal = iVal*10 + ( *sBuf ) - '0'; // FIXME! check for overflow?
		pTokenizer->SetBufferPtr ( sBuf );

		// we just lexed our next token
		iPendingType = iTokType;
		tPendingToken.tInt.iValue = iVal;
		tPendingToken.tInt.iStrIndex = -1;
		return true;
	}

	return false;
}

/// a lexer of my own
int XQParser_t::GetToken ( YYSTYPE * lvalp )
{
	bool bWasFrontModifier = false; // used to print warning

	// what, noone's pending for a bending?!
	if ( !m_iPendingType )
		while (true)
	{
//		assert ( m_iPendingNulls==0 );

		bool bWasKeyword = m_bWasKeyword;
		m_bWasKeyword = false;

		int iSkippedPosBeforeToken = 0;
		if ( m_bWasBlended )
		{
			iSkippedPosBeforeToken = m_pTokenizer->SkipBlended();
			// just add all skipped blended parts except blended head (already added to atomPos)
			if ( iSkippedPosBeforeToken>1 )
				m_iAtomPos += iSkippedPosBeforeToken - 1;
		}

		// tricky stuff
		// we need to manually check for numbers in certain states (currently, just after proximity or quorum operator)
		// required because if 0-9 are not in charset_table, or min_word_len is too high,
		// the tokenizer will *not* return the number as a token!
		const char * pTokenStart = m_pTokenizer->GetBufferPtr();
		const char * pLastTokenEnd = m_pTokenizer->GetTokenEnd();
		const char * sBufferEnd = m_pTokenizer->GetBufferEnd();
		m_pErrorAt = pTokenStart;

		const char * p = pTokenStart;
		while ( p<sBufferEnd && isspace ( *(const BYTE*)p ) ) p++; // to avoid CRT assertions on Windows

		if ( m_bCheckNumber )
		{
			m_bCheckNumber = false;
			if ( GetNumber ( p, pTokenStart ) )
				break;
		}

		// not a number, long number, or number not followed by a whitespace, so fallback to regular tokenizing
		const char * sToken = (const char *) m_pTokenizer->GetToken ();
		if ( !sToken )
		{
			m_iPendingNulls = m_pTokenizer->GetOvershortCount() * m_iOvershortStep;
			if ( !( m_iPendingNulls || m_pTokenizer->GetBufferPtr()-p>0 ) )
				return 0;
			m_iPendingNulls = 0;
			lvalp->pNode = AddKeyword ( nullptr, iSkippedPosBeforeToken );
			m_bWasKeyword = true;
			return TOK_KEYWORD;
		}

		// now let's do some token post-processing
		m_bWasBlended = m_pTokenizer->TokenIsBlended();
		m_bEmpty = false;

		int iPrevDeltaPos = 0;
		if ( m_pPlugin && m_pPlugin->m_fnPushToken )
			sToken = m_pPlugin->m_fnPushToken ( m_pPluginData, const_cast<char*>(sToken), &iPrevDeltaPos, m_pTokenizer->GetTokenStart(), int ( m_pTokenizer->GetTokenEnd() - m_pTokenizer->GetTokenStart() ) );

		if ( !sToken )
			return 0;

		m_iPendingNulls = m_pTokenizer->GetOvershortCount() * m_iOvershortStep;
		m_iAtomPos += 1 + m_iPendingNulls;
		if ( iPrevDeltaPos>1 ) // to match with condifion of m_bWasBlended above
			m_iAtomPos += ( iPrevDeltaPos - 1);

		bool bMultiDestHead = false;
		bool bMultiDest = false;
		int iDestCount = 0;
		// do nothing inside phrase
		if ( !m_pTokenizer->IsPhraseMode() )
			bMultiDest = m_pTokenizer->WasTokenMultiformDestination ( bMultiDestHead, iDestCount );

		// handle NEAR (must be case-sensitive, and immediately followed by slash and int)
		if ( !bMultiDest && p && !m_pTokenizer->IsPhraseMode() &&
			( GetNearToken ( "NEAR/", 5, TOK_NEAR, p, m_pTokenizer, m_iPendingType, m_tPendingToken )
				|| GetNearToken ( "NOTNEAR/", 8, TOK_NOTNEAR, p, m_pTokenizer, m_iPendingType, m_tPendingToken ) ) )
		{
			m_iAtomPos -= 1; // skip token
			break;
		}

		// handle SENTENCE
		if ( !bMultiDest && p && !m_pTokenizer->IsPhraseMode() && !strcasecmp ( sToken, "sentence" ) && !strncmp ( p, "SENTENCE", 8 ) )
		{
			// we just lexed our next token
			m_iPendingType = TOK_SENTENCE;
			m_iAtomPos -= 1;
			break;
		}

		// handle PARAGRAPH
		if ( !bMultiDest && p && !m_pTokenizer->IsPhraseMode() && !strcasecmp ( sToken, "paragraph" ) && !strncmp ( p, "PARAGRAPH", 9 ) )
		{
			// we just lexed our next token
			m_iPendingType = TOK_PARAGRAPH;
			m_iAtomPos -= 1;
			break;
		}

		// handle MAYBE
		if ( !bMultiDest && p && !m_pTokenizer->IsPhraseMode() && !strcasecmp ( sToken, "maybe" ) && !strncmp ( p, "MAYBE", 5 ) )
		{
			// we just lexed our next token
			m_iPendingType = TOK_MAYBE;
			m_iAtomPos -= 1;
			break;
		}

		// handle ZONE
		if ( !bMultiDest && p && !m_pTokenizer->IsPhraseMode() && !strncmp ( p, "ZONE:", 5 )
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
		if ( !bMultiDest && p && !m_pTokenizer->IsPhraseMode() && !strncmp ( p, "ZONESPAN:", 9 )
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

		// handle REGEX
		if ( !bMultiDest && p && !strncmp ( p, "REGEX(", 6 ) )
		{
			// we just lexed our REGEX token
			XQNode_t * pRegex = ParseRegex ( p+6 );
			if ( pRegex )
			{
				m_tPendingToken.pNode = pRegex;
				m_iPendingType = TOK_REGEX;
				break;
			}
		}

		// count [ * ] at phrase node for qpos shift
		if ( m_pTokenizer->IsPhraseMode() && pLastTokenEnd )
		{
			if ( sToken[0]=='*' && sToken[1]=='\0' ) // phrase star should be separate token
			{
				m_dPhraseStar.Add ( m_iAtomPos );
			} else
			{
				int iSpace = 0;
				int iStar = 0;
				const char * sCur = pLastTokenEnd;
				const char * sEnd = m_pTokenizer->GetTokenStart();
				for ( ; sCur<sEnd; sCur++ )
				{
					int iCur = int ( sCur - pLastTokenEnd );
					switch ( *sCur )
					{
					case '*':
						iStar = int ( sCur - pLastTokenEnd );
						break;
					case ' ':
						if ( iSpace+2==iCur && iStar+1==iCur ) // match only [ * ] (separate single star) as valid shift operator
							m_dPhraseStar.Add ( m_iAtomPos );
						iSpace = iCur;
						break;
					}
				}
			}
		}

		// handle specials
		if ( m_pTokenizer->WasTokenSpecial() )
		{
			// specials must not affect pos
			m_iAtomPos--;

			// some specials are especially special
			if ( sToken[0]=='@' )
			{
				bool bIgnore;

				// parse fields operator
				if ( !ParseFields ( m_tPendingToken.tFieldLimit.dMask, m_tPendingToken.tFieldLimit.iMaxPos, bIgnore ) )
					return -1;

				if ( bIgnore )
					continue;

				m_iPendingType = TOK_FIELDLIMIT;
				break;

			}
			if ( sToken[0]=='<' )
			{
				if ( *m_pTokenizer->GetBufferPtr()=='<' )
				{
					// got "<<", aka operator BEFORE
					m_iPendingType = TOK_BEFORE;
					break;
				}

				// got stray '<', ignore
				if ( m_iPendingNulls>0 )
				{
					m_iPendingNulls = 0;
					lvalp->pNode = AddKeyword ( nullptr, iSkippedPosBeforeToken );
					m_bWasKeyword = true;
					return TOK_KEYWORD;
				}
				continue;

			}
			if ( sToken[0]=='^' )
			{
				const char * pTokEnd = m_pTokenizer->GetTokenEnd();
				if ( pTokEnd<m_pTokenizer->GetBufferEnd() && !sphIsSpace ( pTokEnd[0] ) )
					bWasFrontModifier = true;

				// this special is handled in HandleModifiers()
				continue;
			}
			if ( sToken[0]=='$' )
			{
				if ( bWasKeyword )
					continue;
//				if ( sphIsSpace ( m_pTokenizer->GetTokenStart() [ -1 ] ) ) // crashes with fuzzy on single "$"
//					continue;

				// right after overshort
				if ( m_pTokenizer->GetOvershortCount()==1 )
				{
					m_iPendingNulls = 0;
					lvalp->pNode = AddKeyword ( nullptr, iSkippedPosBeforeToken );
					return TOK_KEYWORD;
				}

				Warning ( "modifiers must be applied to keywords, not operators" );

				// this special is handled in HandleModifiers()
				continue;
			}

			bool bWasQuoted = m_bQuoted;
			// all the other specials are passed to parser verbatim
			if ( sToken[0]=='"' )
			{
				m_bQuoted = !m_bQuoted;
				if ( m_bQuoted )
					m_dPhraseStar.Resize ( 0 );
			}
			m_iPendingType = sToken[0];
			m_pTokenizer->SetPhraseMode ( m_bQuoted );

			if ( sToken[0]=='(' )
			{
				XQLimitSpec_t * pLastField = m_dStateSpec.Last();
				m_dStateSpec.Add ( pLastField );
			} else if ( sToken[0]==')' && m_dStateSpec.GetLength()>1 )
			{
				m_dStateSpec.Pop();
			}

			if ( bWasQuoted && !m_bQuoted )
				m_iQuorumQuote = m_iAtomPos;
			else if ( sToken[0]=='/' )
				m_iQuorumFSlash = m_iAtomPos;

			if ( sToken[0]=='~' ||sToken[0]=='/' )
				m_bCheckNumber = true;
			break;
		}

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

		if ( bMultiDest && !bMultiDestHead )
		{
			assert ( m_dMultiforms.GetLength() );
			m_dMultiforms.Last().m_iDestCount++;
			m_dDestForms.Add ( sToken );
			m_bWasKeyword = true;
		} else
		{
			m_tPendingToken.pNode = AddKeyword ( sToken, iSkippedPosBeforeToken );
			m_iPendingType = TOK_KEYWORD;
		}

		if ( bMultiDestHead )
		{
			MultiformNode_t & tMulti = m_dMultiforms.Add();
			tMulti.m_pNode = m_tPendingToken.pNode;
			tMulti.m_iDestStart = m_dDestForms.GetLength();
			tMulti.m_iDestCount = 0;
		}

		if ( !bMultiDest || bMultiDestHead )
			break;
	}

	if ( bWasFrontModifier && m_iPendingType!=TOK_KEYWORD )
		Warning ( "modifiers must be applied to keywords, not operators" );

	// someone must be pending now!
	assert ( m_iPendingType );
	m_bEmpty = false;

	// ladies first, though
	if ( m_iPendingNulls>0 )
	{
		--m_iPendingNulls;
		lvalp->pNode = AddKeyword ( nullptr );
		m_bWasKeyword = true;
		return TOK_KEYWORD;
	}

	// pending the offending
	int iRes = m_iPendingType;
	m_iPendingType = 0;

	if ( iRes==TOK_KEYWORD )
		m_bWasKeyword = true;
	*lvalp = m_tPendingToken;
	return iRes;
}


void XQParser_t::AddQuery ( XQNode_t * pNode )
{
	m_pRoot = pNode;
}

// Handle modifiers:
// 1) ^ - field start
// 2) $ - field end
// 3) ^1.234 - keyword boost
// keyword$^1.234 - field end with boost are on
// keywords^1.234$ - only boost here, '$' it NOT modifier
void XQParser_t::HandleModifiers ( XQKeyword_t & tKeyword ) const noexcept
{
	const char * sTokStart = m_pTokenizer->GetTokenStart();
	const char * sTokEnd = m_pTokenizer->GetTokenEnd();
	if ( !sTokStart || !sTokEnd )
		return;

	const char * sQuery = reinterpret_cast<char *> ( m_sQuery );
	tKeyword.m_bFieldStart = ( sTokStart-sQuery )>0 && sTokStart [ -1 ]=='^' &&
		!( ( sTokStart-sQuery )>1 && sTokStart [ -2 ]=='\\' );

	if ( sTokEnd[0]=='$' )
	{
		tKeyword.m_bFieldEnd = true;
		++sTokEnd; // Skipping.
	}

	if ( sTokEnd[0]=='^' && ( sTokEnd[1]=='.' || sphIsDigital ( sTokEnd[1] ) ) )
	{
		// Probably we have a boost, lets check.
		char * pEnd;
		float fBoost = (float)strtod ( sTokEnd+1, &pEnd );
		if ( ( sTokEnd+1 )!=pEnd )
		{
			// We do have a boost.
			// FIXME Handle ERANGE errno here.
			tKeyword.m_fBoost = fBoost;
			m_pTokenizer->SetBufferPtr ( pEnd );
		}
	}
}


XQNode_t * XQParser_t::AddKeyword ( const char * sKeyword, int iSkippedPosBeforeToken )
{
	XQKeyword_t tAW ( sKeyword, m_iAtomPos );
	tAW.m_iSkippedBefore = iSkippedPosBeforeToken;
	HandleModifiers ( tAW );
	XQNode_t * pNode = SpawnNode ( *m_dStateSpec.Last() );
	pNode->AddDirtyWord ( std::move(tAW) );
	return pNode;
}


XQNode_t * XQParser_t::AddKeyword ( XQNode_t * pLeft, XQNode_t * pRight )
{
	if ( !pLeft || !pRight )
		return pLeft ? pLeft : pRight;

	assert ( pLeft->dWords().GetLength()>0 );
//	assert ( pRight->dWords().GetLength()==1 );

	pRight->WithWords ( [pLeft] ( auto& dWords ) { for ( const auto & dWord : dWords ) pLeft->AddDirtyWord ( dWord ); });
	pLeft->Rehash();
	DeleteSpawned ( pRight );
	return pLeft;
}


XQNode_t * XQParser_t::AddOp ( XQOperator_e eOp, XQNode_t * pLeft, XQNode_t * pRight, int iOpArg )
{
	/////////
	// unary
	/////////

	if ( eOp==SPH_QUERY_NOT )
	{
		XQNode_t * pNode = SpawnNode ( pLeft ? pLeft->m_dSpec : *m_dStateSpec.Last() );
		pNode->SetOp ( SPH_QUERY_NOT, pLeft );
		return pNode;
	}

	//////////
	// binary
	//////////

	if ( !pLeft || !pRight )
		return pLeft ? pLeft : pRight;

	// build a new node
	XQNode_t * pResult = NULL;
	if ( !pLeft->dChildren().IsEmpty() && pLeft->GetOp()==eOp && pLeft->m_iOpArg==iOpArg )
	{
		pLeft->AddNewChild ( pRight );
		pResult = pLeft;
		if ( HasMissedField ( pResult->m_dSpec ) )
			pResult->m_dSpec = pRight->m_dSpec;
	} else
	{
		// however-2, beside all however below, [@@relaxed ((@title hello) | (@missed world)) @body other terms]
		// we should use valid (left) field mask for complex (OR) node
		// as right node in this case has m_bFieldSpec==true but m_dFieldMask==0
		const bool bHasMissedField = HasMissedField ( pRight->m_dSpec );
		const XQLimitSpec_t & tSpec = bHasMissedField ? pLeft->m_dSpec : pRight->m_dSpec;

		// however, it's right (!) spec which is chosen for the resulting node,
		// eg. '@title hello' + 'world @body program'
		XQNode_t * pNode = SpawnNode ( tSpec );
		pNode->SetOp ( eOp, pLeft, pRight );
		pNode->m_iOpArg = iOpArg;
		pResult = pNode;
	}
	return pResult;
}


void XQParser_t::SetPhrase ( XQNode_t * pNode, bool bSetExact, XQOperator_e eOp )
{
	if ( !pNode )
		return;

	assert ( eOp==SPH_QUERY_PHRASE || eOp==SPH_QUERY_PROXIMITY || eOp==SPH_QUERY_QUORUM );
	assert ( !pNode->dWords().IsEmpty() || !pNode->dChildren().IsEmpty() );

	if ( bSetExact )
	{
		pNode->WithWords ( [] ( auto& dWords ) {
			dWords.for_each ([] ( XQKeyword_t & dWord ) {
				if ( !dWord.m_sWord.IsEmpty() )
					dWord.m_sWord.SetSprintf ( "=%s", dWord.m_sWord.cstr() );
			});
		});
	}
	pNode->SetOp ( eOp );
	if ( eOp==SPH_QUERY_PROXIMITY && !pNode->dWords().IsEmpty() )
		m_iAtomPos = pNode->FixupAtomPos();

	PhraseShiftQpos ( pNode );
	m_pParsed->m_bNeedPhraseTransform |= ( pNode->dChildren().GetLength()>0 );
}


void XQParser_t::PhraseShiftQpos ( XQNode_t * pNode )
{
	if ( m_dPhraseStar.IsEmpty() )
		return;

	pNode->WithWords ( [this] ( auto& dWords ) {

	const int * pLast = m_dPhraseStar.Begin();
	const int * pEnd = m_dPhraseStar.Begin() + m_dPhraseStar.GetLength();
	int iQposShiftStart = *pLast;
	int iQposShift = 0;
	int iLastStarPos = *pLast;

	ARRAY_FOREACH ( iWord, dWords )
	{
		XQKeyword_t & tWord = dWords[iWord];

		// fold stars in phrase till current term position
		while ( pLast<pEnd && *(pLast)<=tWord.m_iAtomPos )
		{
			iLastStarPos = *pLast;
			pLast++;
			iQposShift++;
		}

		// star dictionary passes raw star however regular dictionary suppress it
		// raw star also might be suppressed by min_word_len option
		// so remove qpos shift from duplicated raw star term
		// however not stopwords that is also term with empty word
		if ( tWord.m_sWord=="*" || ( tWord.m_sWord.IsEmpty() && tWord.m_iAtomPos==iLastStarPos ) )
		{
			dWords.Remove ( iWord-- );
			iQposShift--;
			continue;
		}

		if ( iQposShiftStart<=tWord.m_iAtomPos )
			tWord.m_iAtomPos += iQposShift;
	}
	});
}

XQNode_t * XQParser_t::AddPhraseKeyword ( XQNode_t * pLeft, XQNode_t * pRight )
{
	if ( ( pLeft && pLeft->dChildren().GetLength() ) || ( pRight && pRight->dChildren().GetLength() ) )
		return AddOp ( SPH_QUERY_PHRASE, pLeft, pRight );

	return AddKeyword ( pLeft, pRight );
}

bool XQParser_t::Parse ( XQQuery_t & tParsed, const char * sQuery, const CSphQuery * pQuery, const TokenizerRefPtr_c& pTokenizer, const CSphSchema * pSchema, const DictRefPtr_c& pDict, const CSphIndexSettings & tSettings, const CSphBitvec * pMorphFields )
{
	// FIXME? might wanna verify somehow that pTokenizer has all the specials etc from sphSetupQueryTokenizer
	assert ( pTokenizer->IsQueryTok() );

	// most outcomes are errors
	SafeDelete ( tParsed.m_pRoot );

	// check for relaxed syntax
	const char * OPTION_RELAXED = "@@relaxed";
	auto OPTION_RELAXED_LEN = (const int) strlen ( OPTION_RELAXED );

	m_bStopOnInvalid = true;
	if ( sQuery && strncmp ( sQuery, OPTION_RELAXED, OPTION_RELAXED_LEN )==0 && !sphIsAlpha ( sQuery[OPTION_RELAXED_LEN] ) )
	{
		sQuery += OPTION_RELAXED_LEN;
		m_bStopOnInvalid = false;
	}

	m_pPlugin = nullptr;
	m_pPluginData = nullptr;

	if ( pQuery && !pQuery->m_sQueryTokenFilterName.IsEmpty() )
	{
		CSphString sError;
		m_pPlugin = PluginAcquire<PluginQueryTokenFilter_c> ( pQuery->m_sQueryTokenFilterLib.cstr(), PLUGIN_QUERY_TOKEN_FILTER, pQuery->m_sQueryTokenFilterName.cstr(), tParsed.m_sParseError );
		if ( !m_pPlugin )
			return false;

		char szError [ SPH_UDF_ERROR_LEN ];
		if ( m_pPlugin->m_fnInit && m_pPlugin->m_fnInit ( &m_pPluginData, MAX_TOKEN_BYTES, pQuery->m_sQueryTokenFilterOpts.cstr(), szError )!=0 )
		{
			tParsed.m_sParseError = sError;
			m_pPlugin = nullptr;
			m_pPluginData = nullptr;
			return false;
		}
	}

	// setup parser
	DictRefPtr_c pMyDict = GetStatelessDict ( pDict );

	Setup ( pSchema, pTokenizer->Clone ( SPH_CLONE ), pMyDict, &tParsed, tSettings );
	m_sQuery = (BYTE*)const_cast<char*>(sQuery);
	m_iQueryLen = sQuery ? (int) strlen(sQuery) : 0;
	m_iPendingNulls = 0;
	m_iPendingType = 0;
	m_pRoot = nullptr;
	m_bEmpty = true;
	m_iOvershortStep = tSettings.m_iOvershortStep;

	m_pTokenizer->SetBuffer ( m_sQuery, m_iQueryLen );
	int iRes = yyparse ( this );

	if ( m_pPlugin )
	{
		if ( m_pPlugin->m_fnDeinit )
			m_pPlugin->m_fnDeinit ( m_pPluginData );

		m_pPlugin = nullptr;
		m_pPluginData = nullptr;
	}

	if ( ( iRes || !m_pParsed->m_sParseError.IsEmpty() ) && !m_bEmpty )
	{
		Cleanup ();
		return false;
	}

	bool bNotOnlyAllowed = g_bOnlyNotAllowed;
	if ( pQuery )
		bNotOnlyAllowed |= pQuery->m_bNotOnlyAllowed;

	m_bWasFullText = ( m_pRoot && ( m_pRoot->dChildren().GetLength () || m_pRoot->dWords().GetLength () ) );
	XQNode_t * pNewRoot = FixupTree ( m_pRoot, *m_dStateSpec.Last(), pMorphFields, bNotOnlyAllowed );
	if ( !pNewRoot )
	{
		Cleanup();
		return false;
	}

	tParsed.m_pRoot = pNewRoot;

	return true;
}


bool XQParser_t::HandleFieldBlockStart ( const char * & pPtr )
{
	if ( *pPtr=='(' )
	{
		// handle @(
		pPtr++;
		return true;
	}

	return false;
}

bool sphParseExtendedQuery ( XQQuery_t & tParsed, const char * sQuery, const CSphQuery * pQuery, const TokenizerRefPtr_c& pTokenizer, const CSphSchema * pSchema, const DictRefPtr_c& pDict, const CSphIndexSettings & tSettings, const CSphBitvec * pMorphFields )
{
	XQParser_t qp;
	bool bRes = qp.Parse ( tParsed, sQuery, pQuery, pTokenizer, pSchema, pDict, tSettings, pMorphFields );

#ifndef NDEBUG
	if ( bRes && tParsed.m_pRoot )
		tParsed.m_pRoot->Check ( true );
#endif

#if XQDEBUG
	if ( bRes )
	{
		printf ( "\n--- query ---\n" );
		printf ( "%s\n", sQuery );
		xqDump ( tParsed.m_pRoot, 0 );
//		DotDump ( tParsed.m_pRoot );
		printf ( "\n--- query reconstructed ---\n" );
		printf ( "%s\n", sphReconstructNode ( tParsed.m_pRoot ).cstr());
		printf ( "---\n" );
	}
#endif

	// moved here from ranker creation
	// as at that point term expansion could produce many terms from expanded term and this condition got failed
	tParsed.m_bSingleWord = ( tParsed.m_pRoot && tParsed.m_pRoot->dChildren().IsEmpty() && tParsed.m_pRoot->dWords().GetLength()==1 );
	tParsed.m_bEmpty = qp.m_bEmpty;
	tParsed.m_bWasFullText = ( qp.m_bWasFullText || !qp.m_bEmpty );

	return bRes;
}

class QueryParserPlain_c : public QueryParser_i
{
public:
	bool IsFullscan ( const XQQuery_t & tQuery ) const override { return false; }
	bool ParseQuery ( XQQuery_t & tParsed, const char * sQuery, const CSphQuery * pQuery, TokenizerRefPtr_c pQueryTokenizer, TokenizerRefPtr_c pQueryTokenizerJson, const CSphSchema * pSchema, const DictRefPtr_c& pDict, const CSphIndexSettings & tSettings, const CSphBitvec * pMorphFields ) const override;
	QueryParser_i * Clone() const final { return new QueryParserPlain_c; }
};


bool QueryParserPlain_c::ParseQuery ( XQQuery_t & tParsed, const char * sQuery, const CSphQuery * pQuery, TokenizerRefPtr_c pQueryTokenizer, TokenizerRefPtr_c, const CSphSchema * pSchema, const DictRefPtr_c& pDict, const CSphIndexSettings & tSettings, const CSphBitvec * pMorphFields ) const
{
	return sphParseExtendedQuery ( tParsed, sQuery, pQuery, pQueryTokenizer, pSchema, pDict, tSettings, pMorphFields );
}


std::unique_ptr<QueryParser_i> sphCreatePlainQueryParser()
{
	return std::make_unique<QueryParserPlain_c>();
}
