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
#include "sphinxexcerpt.h"
#include "sphinxutils.h"
#include "sphinxsearch.h"
#include "sphinxquery.h"
#include "sphinxint.h"

#include <math.h>

/////////////////////////////////////////////////////////////////////////////
// THE EXCERPTS GENERATOR
/////////////////////////////////////////////////////////////////////////////

static const int MAX_HIGHLIGHT_WORDS = 256;

#define UINT32_MASK 0xffffffffUL
#define UINT16_MASK 0xffff
typedef uint64_t ZonePacked_t;

class ExcerptGen_c
{
	friend class SnippetsQwordSetup;

public:
	explicit				ExcerptGen_c ( bool bUtf8 );
							~ExcerptGen_c () {}

	void	BuildExcerpt ( ExcerptQuery_t & tQuery );

	void	TokenizeQuery ( const ExcerptQuery_t &, CSphDict * pDict, ISphTokenizer * pTokenizer, const CSphIndexSettings & tSettings );
	void	TokenizeDocument ( char * pData, int iDataLen, CSphDict * pDict, ISphTokenizer * pTokenizer, bool bFillMasks, const ExcerptQuery_t & q, const CSphIndexSettings & tSettings, int iSPZ );

	void	SetMarker ( CSphHitMarker * pMarker ) { m_pMarker = pMarker; }
	void	SetExactPhrase ( const ExcerptQuery_t & tQuery );

public:
	enum Token_e
	{
		TOK_NONE = 0,		///< unspecified type, also used as the end marker
		TOK_WORD,			///< just a word
		TOK_SPACE,			///< whitespace chars seq
		TOK_BREAK,			///< non-word chars seq which delimit a phrase part or boundary
		TOK_SPZ				///< SENTENCE, PARAGRAPH, ZONE
	};

	struct Token_t
	{
		Token_e				m_eType;		///< token type
		int					m_iStart;		///< token start (index in codepoints array)
		int					m_iLengthCP;	///< token length (in codepoints)
		int					m_iLengthBytes;	///< token length (in bytes)
		int					m_iWeight;		///< token weight
		DWORD				m_uWords;		///< matching query words mask
		SphWordID_t			m_iWordID;		///< token word ID from dictionary
		SphWordID_t			m_iBlendID;		///< blended word ID (eg. "T-mobile" would not tokenize itself, but still shadow "T" and "mobile")
		DWORD				m_uPosition;	///< hit position in document

		void Reset ()
		{
			m_eType = TOK_NONE;
			m_iStart = 0;
			m_iLengthCP = 0;
			m_iLengthBytes = 0;
			m_iWeight = 0;
			m_uWords = 0;
			m_iWordID = 0;
			m_iBlendID = 0;
			m_uPosition = 0;
		}
	};

	struct TokenSpan_t
	{
		int		m_iStart;	///< start index, inclusive
		int		m_iEnd;		///< end index, inclusive
		int		m_iWords;	///< number of TOK_WORDS tokens
		int		m_iQwords;	///< number of words matching query

		void Reset ()
		{
			m_iStart = -1;
			m_iEnd = -2;
			m_iWords = 0;
			m_iQwords = 0;
		}

		void Add ( int i, bool bQword )
		{
			assert ( m_iStart<i && m_iEnd<i );
			if ( m_iStart<0 )
				m_iStart = i;
			m_iEnd = i;
			m_iWords++;
			m_iQwords += bQword;
		}
	};

	struct Passage_t
	{
		int					m_iStart;			///< start token index
		int					m_iTokens;			///< token count
		int					m_iCodes;			///< codepoints count
		int					m_iWords;			///< words count
		DWORD				m_uQwords;			///< matching query words mask
		int					m_iQwordsWeight;	///< passage weight factor
		int					m_iQwordCount;		///< passage weight factor
		int			m_iMaxLCS;					///< passage weight factor
		int			m_iMinGap;					///< passage weight factor
		int			m_iStartLimit;				///< start of match in passage
		int			m_iEndLimit;				///< end of match in passage
		int			m_iAroundBefore;			///< how many words before first matched words
		int			m_iAroundAfter;				///< how many words after last matched words

		void Reset ()
		{
			m_iStart = 0;
			m_iTokens = 0;
			m_iCodes = 0;
			m_uQwords = 0;
			m_iQwordsWeight = 0;
			m_iQwordCount = 0;
			m_iMaxLCS = 0;
			m_iMinGap = 0;
			m_iAroundBefore = 0;
			m_iAroundAfter = 0;
		}

		inline int GetWeight () const
		{
			return m_iQwordCount + m_iQwordsWeight*m_iMaxLCS + m_iMinGap;
		}
	};

	enum KeywordStar_e
	{
		STAR_NONE	= 0,
		STAR_FRONT	= 1 << 0,
		STAR_BACK	= 1 << 1,
		STAR_BOTH	= STAR_FRONT | STAR_BACK
	};

	struct Keyword_t
	{
		int		m_uStar;
		int		m_iWord;
		int		m_iLength;
	};

	// ZonePacked_t bits:
	// 64-32	(32) - uint position ( in document words )
	// 32-16	(16) - uint sibling ( index of sibling (open/closed) zone instance )
	// 16-0		(0) - zone type for this instance
	const CSphVector<ZonePacked_t> & GetZones () const { return m_dZones; }
	const SmallStringHash_T<int> & GetZonesName () const { return m_hZones; }

protected:
	CSphVector<Token_t>		m_dTokens;		///< source text tokens
	CSphVector<Token_t>		m_dWords;		///< query words tokens
	int						m_iDocumentWords;
	int						m_iPassageId;

	CSphString				m_sBuffer; // FIXME!!! REMOVE!!! ME!!!

	CSphVector<BYTE>		m_dResult;		///< result holder
	int						m_iResultLen;	///< result codepoints count

	CSphVector<Passage_t>	m_dPassages;	///< extracted passages

	bool					m_bExactPhrase;

	DWORD					m_uFoundWords;	///< found words mask
	int						m_iQwordCount;
	int						m_iLastWord;
	CSphHitMarker *			m_pMarker;

	CSphVector<char>		m_dKeywordsBuffer;
	CSphVector<Keyword_t>	m_dKeywords;

	CSphVector<ZonePacked_t>	m_dZones;		///< zones for current document
	SmallStringHash_T<int>		m_hZones;	///< zones names
	CSphVector<int>				m_dZonePos;	///< zones positions (in characters)
	CSphVector<int>				m_dZoneParent;	///< zones parent type

	bool					m_bUtf8;
	int						m_iTotalCP;

protected:
	void					CalcPassageWeight ( Passage_t & tPass, const TokenSpan_t & tSpan, int iMaxWords, int iWordCountCoeff );
	void					UpdateGaps ( Passage_t & tPass, const TokenSpan_t & tSpan, int iMaxWords );
	bool					ExtractPassages ( const ExcerptQuery_t & q );
	bool					ExtractPhrases ( const ExcerptQuery_t & q );

	void					HighlightPhrase ( const ExcerptQuery_t & q, int iTok, int iEnd );
	void					HighlightAll ( const ExcerptQuery_t & q );
	void					HighlightStart ( const ExcerptQuery_t & q );
	bool					HighlightBestPassages ( const ExcerptQuery_t & q );

	void					ResultEmit ( const char * sLine, bool bHasMacro=false, int iPassageId=0, const char * sPostPassage=NULL );
	void					ResultEmit ( const Token_t & sTok );

	void					AddJunk ( int iStart, int iLength, int iBoundary );
	void					AddBoundary ();

	void					MarkHits ();

	bool					SetupWindow ( TokenSpan_t & tSpan, Passage_t & tPass, int iFrom, int iCpLimit, const ExcerptQuery_t & q );
	bool					FlushPassage ( const Passage_t & tPass, int iLCSThresh );
};

// find string sFind in first iLimit characters of sBuffer
static BYTE * FindString ( BYTE * sBuffer, BYTE * sFind, int iLimit )
{
	assert ( iLimit > 0 );
	assert ( sBuffer );
	assert ( sFind );

	iLimit++;
	do
	{
		while ( *sBuffer!=*sFind )
			if ( !*++sBuffer || !--iLimit ) return NULL;

		int iSubLimit = iLimit;
		BYTE * sSubFind = sFind;
		BYTE * sSubBuffer = sBuffer;
		while ( *sSubFind && *sSubBuffer && *sSubFind==*sSubBuffer++ )
		{
			sSubFind++;
			if ( !--iSubLimit ) return NULL;
		}
		if ( !*sSubFind )
			return sBuffer;
	}
	while ( *++sBuffer );

	return NULL;
}

/// hitman used here in snippets
typedef Hitman_c<8> HITMAN;

/// snippets query words for different cases
class ISnippetsQword : public ISphQword
{
public:
	CSphString *							m_sBuffer;
	CSphVector<ExcerptGen_c::Token_t> *		m_dTokens;
	ISphTokenizer *							m_pTokenizer;
	DWORD *									m_uFoundWords;

	// word information, filled during query word setup
	int			m_iWordLength;
	int			m_iLastIndex;
	DWORD		m_uWordMask;

	// iterator state
	CSphMatch	m_tMatch;
	int			m_iToken;
	int			m_iChunk;

	typedef ExcerptGen_c::Token_t Token_t;

	ISnippetsQword()
		: m_iToken ( 0 )
		, m_iChunk ( 0 )
	{}

	virtual void SeekHitlist ( SphOffset_t ) {}

	virtual const CSphMatch & GetNextDoc ( DWORD * )
	{
		m_dQwordFields.Set();
		if ( ( m_iChunk++ )==0 )
		{
			if ( GetNextHit()!=EMPTY_HIT )
			{
				m_tMatch.m_iDocID = 1;
				m_iToken--;
			} else
				m_tMatch.m_iDocID = 0;
		} else
			m_tMatch.m_iDocID = 0;
		return m_tMatch;
	}

	virtual const char * OnSetup ( CSphDict * )
	{
		m_iWordLength = strlen ( m_sDictWord.cstr() );
		return m_sDictWord.cstr();
	}
};

/// simple keyword match on id
struct SnippetsQword_Exact_c: public ISnippetsQword
{
	virtual Hitpos_t GetNextHit ()
	{
		while ( m_iToken < m_dTokens->GetLength() )
		{
			Token_t & tToken = (*m_dTokens)[m_iToken++];
			if ( !( tToken.m_eType==ExcerptGen_c::TOK_WORD || tToken.m_eType==ExcerptGen_c::TOK_SPZ ) )
				continue;

			if ( tToken.m_iWordID==m_iWordID || tToken.m_iBlendID==m_iWordID )
			{
				tToken.m_uWords |= m_uWordMask;
				*m_uFoundWords |= m_uWordMask;
				return HITMAN::Create ( 0, tToken.m_uPosition, ( m_iToken-1 )==m_iLastIndex );
			}
		}
		return EMPTY_HIT;
	}
};

/// partial matches
template < typename COMPARE > struct SnippetsQword_c: public ISnippetsQword
{
	virtual Hitpos_t GetNextHit ()
	{
		while ( m_iToken < m_dTokens->GetLength() )
		{
			Token_t & tToken = (*m_dTokens)[m_iToken++];
			if ( tToken.m_eType!=ExcerptGen_c::TOK_WORD )
				continue;

			m_pTokenizer->SetBuffer ( (BYTE *) &m_sBuffer->cstr() [ tToken.m_iStart ], tToken.m_iLengthBytes );
			BYTE * sToken = m_pTokenizer->GetToken(); // OPTIMIZE? token can be memorized and shared between qwords
			if ( (*(COMPARE *)this).Match ( tToken, sToken ) )
			{
				tToken.m_uWords |= m_uWordMask;
				*m_uFoundWords |= m_uWordMask;
				return HITMAN::Create ( 0, tToken.m_uPosition, ( m_iToken-1 )==m_iLastIndex );
			}
		}
		return EMPTY_HIT;
	}
};

template < typename COMPARE >
struct SnippetQword_Star_t : public SnippetsQword_c<COMPARE>
{
	virtual const char * OnSetup ( CSphDict * )
	{
		this->m_iWordLength = strlen ( this->m_sWord.cstr() );
		return this->m_sWord.cstr();
	}
};

struct SnippetsQword_StarFront_c : public SnippetQword_Star_t<SnippetsQword_StarFront_c>
{
	inline bool Match ( const Token_t & tToken, BYTE * sToken )
	{
		int iOffset = tToken.m_iLengthBytes - m_iWordLength;
		return iOffset>=0 &&
			memcmp ( m_sWord.cstr(), sToken + iOffset, m_iWordLength )==0;
	}
};

struct SnippetsQword_StarBack_c : public SnippetQword_Star_t<SnippetsQword_StarBack_c>
{
	inline bool Match ( const Token_t & tToken, BYTE * sToken )
	{
		return ( tToken.m_iLengthBytes>=m_iWordLength ) &&
			memcmp ( m_sWord.cstr(), sToken, m_iWordLength )==0;
	}
};

struct SnippetsQword_StarBoth_c : public SnippetQword_Star_t<SnippetsQword_StarBoth_c>
{
	inline bool Match ( const Token_t & tToken, BYTE * sToken )
	{
		return FindString ( sToken, (BYTE *)m_sWord.cstr(), tToken.m_iLengthBytes )!=NULL;
	}
};


struct SnippetsQword_ExactForm_c : public SnippetsQword_c<SnippetsQword_ExactForm_c>
{
	inline bool Match ( const Token_t & tToken, BYTE * sToken )
	{
		return tToken.m_iBlendID==m_iWordID || ( memcmp ( sToken, m_sDictWord.cstr()+1, m_iWordLength )==0 );
	}

	virtual const char* OnSetup ( CSphDict * pDict )
	{
		// FIXME!!! to match with blended parts it recalculates wordID for word without head '=' part

		int iLen = m_sWord.Length()-1;
		BYTE sTmp [ 3*SPH_MAX_WORD_LEN + 16 ];

		assert ( iLen>0 && iLen<(int)sizeof(sTmp) );
		assert ( m_sWord.Begins ( "=" ) );
		assert ( pDict && m_pTokenizer );

		memcpy ( sTmp, m_sWord.cstr()+1, iLen );
		sTmp[iLen] = '\0';

		m_pTokenizer->SetBuffer ( sTmp, iLen );
		while ( m_pTokenizer->GetToken()!=NULL )
		{
			if ( m_pTokenizer->TokenIsBlended() )
			{
				m_pTokenizer->SkipBlended();
				m_iWordID = pDict->GetWordID ( sTmp );
				break;
			}
		}

		return SnippetsQword_c<SnippetsQword_ExactForm_c>::OnSetup ( pDict );
	}
};

/// snippets query word setup
/// FIXME! throw these away in favor of fastpath ones
class SnippetsQwordSetup: public ISphQwordSetup
{
	ExcerptGen_c *		m_pGenerator;
	ISphTokenizer *		m_pTokenizer;

public:
	SnippetsQwordSetup ( ExcerptGen_c * pGenerator, ISphTokenizer * pTokenizer )
		: m_pGenerator ( pGenerator )
		, m_pTokenizer ( pTokenizer )
	{}

	virtual ISphQword *		QwordSpawn ( const XQKeyword_t & tWord ) const;
	virtual bool			QwordSetup ( ISphQword * pQword ) const;
};

ISphQword * SnippetsQwordSetup::QwordSpawn ( const XQKeyword_t & tWord ) const
{
	if ( tWord.m_sWord.cstr()[0]=='=' )
		return new SnippetsQword_ExactForm_c;
	switch ( tWord.m_uStarPosition )
	{
		case STAR_NONE:		return new SnippetsQword_Exact_c;
		case STAR_FRONT:	return new SnippetsQword_StarFront_c;
		case STAR_BACK:		return new SnippetsQword_StarBack_c;
		case STAR_BOTH:		return new SnippetsQword_StarBoth_c;
		default:			assert ( "impossible star position" && 0 ); return NULL;
	}
}

bool SnippetsQwordSetup::QwordSetup ( ISphQword * pQword ) const
{
	ISnippetsQword * pWord = dynamic_cast<ISnippetsQword *> ( pQword );
	if ( !pWord )
		assert ( "query word setup failed" && 0 );

	pWord->m_iLastIndex = m_pGenerator->m_iLastWord;
	pWord->m_uWordMask = 1 << (m_pGenerator->m_iQwordCount++);
	pWord->m_dTokens = &(m_pGenerator->m_dTokens);
	pWord->m_sBuffer = &(m_pGenerator->m_sBuffer);
	pWord->m_pTokenizer = m_pTokenizer;
	pWord->m_uFoundWords = &m_pGenerator->m_uFoundWords;

	pWord->m_iDocs = 1;
	pWord->m_iHits = 1;
	pWord->m_bHasHitlist = true;

	const char * sWord = pWord->OnSetup ( m_pDict );

	// add dummy word, used for passage weighting
	const int iLength = m_pTokenizer->IsUtf8() ? sphUTF8Len ( sWord ) : strlen ( sWord );
	m_pGenerator->m_dWords.Add().m_iLengthCP = iLength;
	m_pGenerator->m_dKeywords.Add().m_iLength = iLength;

	return true;
}

/////////////////////////////////////////////////////////////////////////////

inline bool operator < ( const ExcerptGen_c::Token_t & a, const ExcerptGen_c::Token_t & b )
{
	if ( a.m_iLengthCP==b.m_iLengthCP )
		return a.m_iStart > b.m_iStart;
	return a.m_iLengthCP < b.m_iLengthCP;
}


inline bool operator < ( const ExcerptGen_c::Passage_t & a, const ExcerptGen_c::Passage_t & b )
{
	if ( a.GetWeight()==b.GetWeight() )
		return a.m_iCodes < b.m_iCodes;
	return a.GetWeight() < b.GetWeight();
}

ExcerptGen_c::ExcerptGen_c ( bool bUtf8 )
{
	m_iQwordCount = 0;
	m_bExactPhrase = false;
	m_pMarker = NULL;
	m_uFoundWords = 0;
	m_bUtf8 = bUtf8;
	m_iTotalCP = 0;
}

void ExcerptGen_c::AddBoundary()
{
	Token_t & tLast = m_dTokens.Add();
	tLast.Reset();
	tLast.m_eType = TOK_BREAK;
}

void ExcerptGen_c::AddJunk ( int iStart, int iLength, int iBoundary )
{
	assert ( iLength>0 );
#ifdef PARANOID
	assert ( iLength<=m_sBuffer.Length() );
	assert ( iStart+iLength<=m_sBuffer.Length() );
#endif

	int iChunkStart = iStart;
	int iSaved = 0;

	for ( int i = iStart; i < iStart+iLength; i++ )
		if ( sphIsSpace ( m_sBuffer.cstr () [i] )!=sphIsSpace ( m_sBuffer.cstr () [iChunkStart] ) )
		{
			Token_t & tLast = m_dTokens.Add();
			tLast.Reset();
			tLast.m_eType = TOK_SPACE;
			tLast.m_iStart = iChunkStart;
			tLast.m_iLengthBytes = tLast.m_iLengthCP = i - iChunkStart;
			if ( m_bUtf8 )
				tLast.m_iLengthCP = sphUTF8Len ( m_sBuffer.cstr() + tLast.m_iStart, tLast.m_iLengthBytes );
			m_iTotalCP += tLast.m_iLengthCP;

			iChunkStart = i;
			iSaved += tLast.m_iLengthBytes;

			if ( iBoundary!=-1 && iSaved > ( iBoundary-iStart ) )
			{
				AddBoundary();
				iBoundary = -1;
			}
		}

	Token_t & tLast = m_dTokens.Add();
	tLast.Reset();
	tLast.m_eType = TOK_SPACE;
	tLast.m_iStart = iChunkStart;
	tLast.m_iLengthBytes = tLast.m_iLengthCP = iStart + iLength - iChunkStart;
	if ( m_bUtf8 )
		tLast.m_iLengthCP = sphUTF8Len ( m_sBuffer.cstr() + tLast.m_iStart, tLast.m_iLengthBytes );
	m_iTotalCP += tLast.m_iLengthCP;

	if ( iBoundary!=-1 )
		AddBoundary();
}


void ExcerptGen_c::TokenizeQuery ( const ExcerptQuery_t & tQuery, CSphDict * pDict,
	ISphTokenizer * pTokenizer, const CSphIndexSettings & tSettings )
{
	// tokenize query words
	int iWordsLength = strlen ( tQuery.m_sWords.cstr() );

	m_dKeywords.Reserve ( MAX_HIGHLIGHT_WORDS );

	BYTE * sWord;
	int iKwIndex = 0;
	int uPosition = 0;

	pTokenizer->SetBuffer ( (BYTE *)tQuery.m_sWords.cstr(), iWordsLength );
	while ( ( sWord = pTokenizer->GetToken() )!=NULL )
	{
		SphWordID_t iWord = pDict->GetWordID ( sWord );

		bool bIsStopWord = false;
		if ( !iWord )
			bIsStopWord = pDict->IsStopWord ( sWord );

		if ( !pTokenizer->TokenIsBlended() )
		{
			uPosition += pTokenizer->GetOvershortCount();

			if ( pTokenizer->GetBoundary() )
				uPosition += tSettings.m_iBoundaryStep;

			if ( iWord || bIsStopWord )
				uPosition = bIsStopWord ? uPosition+tSettings.m_iStopwordStep : uPosition+1;
		}

		if ( iWord )
		{
			Token_t & tLast = m_dWords.Add();
			tLast.m_eType = TOK_WORD;
			tLast.m_iWordID = iWord;
			tLast.m_iLengthBytes = tLast.m_iLengthCP = strlen ( (const char *)sWord );
			if ( m_bUtf8 )
				tLast.m_iLengthCP = sphUTF8Len ( (const char *)sWord );
			m_iTotalCP += tLast.m_iLengthCP;
			tLast.m_uPosition = uPosition;

			// store keyword
			Keyword_t & kwLast = m_dKeywords.Add();
			kwLast.m_iLength = tLast.m_iLengthCP;

			// find stars
			bool bStarBack = ( *pTokenizer->GetTokenEnd()=='*' );
			bool bStarFront = ( pTokenizer->GetTokenStart()!=pTokenizer->GetBufferPtr() ) &&
				( pTokenizer->GetTokenStart()[-1]=='*' );
			kwLast.m_uStar = ( bStarFront ? STAR_FRONT : 0 ) | ( bStarBack ? STAR_BACK : 0 );

			// store token
			const int iEndIndex = iKwIndex + tLast.m_iLengthBytes + 1;
			m_dKeywordsBuffer.Resize ( iEndIndex );
			kwLast.m_iWord = iKwIndex;
			strcpy ( &m_dKeywordsBuffer [ iKwIndex ], (const char *)sWord ); // NOLINT
			iKwIndex = iEndIndex;

			if ( m_dWords.GetLength()==MAX_HIGHLIGHT_WORDS )
				break;
		}
	}
}

static int FindTagEnd ( const char * sData )
{
	assert ( *sData=='<' );
	const char * s = sData+1;

	// we just scan until EOLN or tag end
	while ( *s && *s!='>' )
	{
		// exit on duplicate
		if ( *s=='<' )
			return -1;

		if ( *s=='\'' || *s=='"' )
			s = (const char *)SkipQuoted ( (const BYTE *)s );
		else
			s++;
	}

	if ( !*s )
		return -1;

	return s-sData;
}

uint64_t sphPackZone ( DWORD uPosition, int iSiblingIndex, int iZoneType )
{
	assert ( iSiblingIndex>=0 && iSiblingIndex<UINT16_MASK );
	assert ( iZoneType>=0 && iZoneType<UINT16_MASK );

	return ( ( (uint64_t)uPosition<<32 )
		| ( ( iSiblingIndex & UINT16_MASK )<<16 )
		| ( iZoneType & UINT16_MASK ) );
}

int FindAddZone ( const char * sZoneName, int iZoneNameLen, SmallStringHash_T<int> & hZones )
{
	CSphString sZone;
	sZone.SetBinary ( sZoneName, iZoneNameLen );

	int * pZoneIndex = hZones ( sZone );
	if ( pZoneIndex )
		return *pZoneIndex;

	int iZone = hZones.GetLength();
	hZones.Add ( iZone, sZone );
	return iZone;
}

// FIXME! unify with global static void TokenizeDocument somehow, lots of common code
void ExcerptGen_c::TokenizeDocument ( char * pData, int iDataLen, CSphDict * pDict,
	ISphTokenizer * pTokenizer, bool bFillMasks, const ExcerptQuery_t & q,
	const CSphIndexSettings & tSettings, int iSPZ )
{
	assert ( q.m_sStripMode!="retain" );
	bool bQueryMode = q.m_bHighlightQuery;

	m_iTotalCP = 0;
	m_iDocumentWords = 0;
	m_dTokens.Reserve ( Max ( iDataLen/4, 256 ) ); // len/tok ratio ranged 2.8 to 3.2 on my testing data
	m_sBuffer = pData;

	pTokenizer->SetBuffer ( (BYTE*)pData, iDataLen );

	const char * pStartPtr = pTokenizer->GetBufferPtr ();
	const char * pLastTokenEnd = pStartPtr;

	assert ( pStartPtr && pLastTokenEnd );

	CSphVector<int> dZoneStack;
	CSphVector<int> dExactPhrase;
	if ( m_bExactPhrase )
		dExactPhrase.Reserve ( m_dWords.GetLength() );

	BYTE * sWord;
	DWORD uPosition = 0; // hit position in document
	SphWordID_t iBlendID = 0;
	const char * pBlendedEnd = NULL;
	while ( ( sWord = pTokenizer->GetToken() )!=NULL )
	{
		if ( pTokenizer->TokenIsBlended() )
		{
			if ( pBlendedEnd<pTokenizer->GetTokenEnd() )
			{
				iBlendID = pDict->GetWordID ( sWord );
				pBlendedEnd = pTokenizer->GetTokenEnd();
			}
			continue;
		}

		uPosition += pTokenizer->GetOvershortCount();

		const char * pTokenStart = pTokenizer->GetTokenStart ();

		if ( pTokenStart!=pStartPtr && pTokenStart>pLastTokenEnd )
		{
			AddJunk ( pLastTokenEnd - pStartPtr,
				pTokenStart - pLastTokenEnd,
				pTokenizer->GetBoundary() ? pTokenizer->GetBoundaryOffset() : -1 );
			pLastTokenEnd = pTokenStart;
		}

		// handle SPZ tokens GE then needed
		// add SENTENCE, PARAGRAPH, ZONE token, do junks and tokenizer and pLastTokenEnd fix up
		// FIXME!!! it heavily depends on such this attitude MAGIC_CODE_SENTENCE < MAGIC_CODE_PARAGRAPH < MAGIC_CODE_ZONE
		if ( *sWord==MAGIC_CODE_SENTENCE || *sWord==MAGIC_CODE_PARAGRAPH || *sWord==MAGIC_CODE_ZONE )
		{
			// SPZ token has position and could be last token too
			uPosition += ( iSPZ && *sWord>=iSPZ );

			if ( iSPZ && *sWord>=iSPZ && ( m_dTokens.GetLength()==0 || m_dTokens.Last().m_eType!=TOK_SPZ ) )
			{
				BYTE * sWordSPZ = sWord;
				if ( (*sWord)==MAGIC_CODE_SENTENCE )
					sWordSPZ = (BYTE *)MAGIC_WORD_SENTENCE;
				else if ( (*sWord)==MAGIC_CODE_PARAGRAPH )
					sWordSPZ = (BYTE *)MAGIC_WORD_PARAGRAPH;

				Token_t & tLast = m_dTokens.Add();
				tLast.Reset();
				tLast.m_eType = TOK_SPZ;
				tLast.m_iWordID = pDict->GetWordID ( sWordSPZ );
				tLast.m_uPosition = uPosition;

				if ( *sWord==MAGIC_CODE_SENTENCE )
				{
					tLast.m_iStart = pTokenStart-pStartPtr;
					tLast.m_iLengthBytes = tLast.m_iLengthCP = 1;
					m_iTotalCP++;
				}

				// SPZ token has position and could be last token too
				m_iLastWord = m_dTokens.GetLength();
				pLastTokenEnd = pTokenizer->GetTokenEnd(); // fix it up to prevent adding last chunk on exit
			}

			if ( *sWord==MAGIC_CODE_ZONE )
			{
				const char * pEnd = pTokenizer->GetBufferPtr();
				const char * pTagStart = pEnd;
				while ( *pEnd && *pEnd!=MAGIC_CODE_ZONE )
					pEnd++;
				pEnd++; // skip zone token too
				pTokenizer->SetBufferPtr ( pEnd );
				pLastTokenEnd = pEnd; // fix it up to prevent adding last chunk on exit

				// span's management
				if ( *pTagStart!='/' )	// open zone
				{
					// zone stack management
					int iSelf = m_dZones.GetLength();
					dZoneStack.Add ( iSelf );

					// add zone itself
					int iZone = FindAddZone ( pTagStart, pEnd-pTagStart-1, m_hZones );
					m_dZones.Add ( sphPackZone ( uPosition, iSelf, iZone ) );

					// zone position in characters
					m_dZonePos.Add ( pTagStart-pStartPtr );

					// for open zone the parent is the zone itself
					m_dZoneParent.Add ( iZone );
				} else					// close zone
				{
#ifndef NDEBUG
					// lets check open - close tags match
					assert ( dZoneStack.GetLength() && dZoneStack.Last()<m_dZones.GetLength() );
					int iOpening = m_dZonePos [ dZoneStack.Last() ];
					assert ( iOpening<pEnd-pStartPtr && strncmp ( pStartPtr+iOpening, pTagStart+1, pEnd-pTagStart-2 )==0 );
#endif
					int iZone = FindAddZone ( pTagStart+1, pEnd-pTagStart-2, m_hZones );
					int iOpen = dZoneStack.Last();
					int iClose = m_dZones.GetLength();
					uint64_t uOpenPacked = m_dZones[ iOpen ];
					DWORD uOpenPos = (DWORD)( ( uOpenPacked>>32 ) & UINT32_MASK );
					assert ( iZone==(int)( uOpenPacked & UINT16_MASK ) ); // check for zone's types match;

					m_dZones[iOpen] = sphPackZone ( uOpenPos, iClose, iZone );
					m_dZones.Add ( sphPackZone ( uPosition, iOpen, iZone ) );

					// zone position in characters
					m_dZonePos.Add ( pTagStart-pStartPtr );

					// for close zone the parent is the previous zone on stack
					int iParentZone = dZoneStack.GetLength()>2 ? dZoneStack[dZoneStack.GetLength()-2] : 0;
					uint64_t uParentPacked = m_dZones.GetLength() && iParentZone<m_dZones.GetLength() ? m_dZones[iParentZone] : 0;
					m_dZoneParent.Add ( (int)( uParentPacked & UINT16_MASK ) );

					// pop up current zone from zone's stack
					dZoneStack.Resize ( dZoneStack.GetLength()-1 );
				}
			}

			continue;
		}

		pLastTokenEnd = pTokenizer->GetTokenEnd ();

		SphWordID_t iExactID = 0;
		if ( bQueryMode && tSettings.m_bIndexExactWords )
		{
			BYTE sBuf [ 3*SPH_MAX_WORD_LEN+4 ];
			int iBytes = pLastTokenEnd - pTokenStart;
			if ( iBytes+2>(int)sizeof(sBuf) )
				iBytes = (int)sizeof(sBuf)-2;
			memcpy ( sBuf + 1, sWord, iBytes );
			sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			sBuf[iBytes+1] = '\0';

			iExactID = pDict->GetWordIDNonStemmed ( sBuf );
		}

		SphWordID_t iWord = pDict->GetWordID ( sWord );

		if ( pTokenizer->GetBoundary() )
			uPosition += tSettings.m_iBoundaryStep;

		bool bIsStopWord = false;
		if ( !iWord )
			bIsStopWord = pDict->IsStopWord ( sWord );

		if ( iWord || bIsStopWord )
			uPosition = bIsStopWord ? uPosition+tSettings.m_iStopwordStep : uPosition+1;

		if ( !pTokenizer->TokenIsBlendedPart() )
			iBlendID = 0;

		Token_t & tLast = m_dTokens.Add();
		tLast.m_eType = ( iWord || bIsStopWord ) ? TOK_WORD : TOK_SPACE;
		tLast.m_uPosition = ( iWord || bIsStopWord ) ? uPosition : 0;
		tLast.m_iStart = pTokenStart - pStartPtr;
		tLast.m_iLengthBytes = tLast.m_iLengthCP = pLastTokenEnd - pTokenStart;
		if ( m_bUtf8 && ( iWord || bIsStopWord ) )
			tLast.m_iLengthCP = sphUTF8Len ( pTokenStart, tLast.m_iLengthBytes );
		m_iTotalCP += tLast.m_iLengthCP;
		tLast.m_iWordID = iWord;
		tLast.m_iBlendID = iBlendID;
		tLast.m_uWords = 0;
		if ( iWord || bIsStopWord )
			m_iDocumentWords++;

		m_iLastWord = iWord ? m_dTokens.GetLength() - 1 : m_iLastWord;

		// fill word mask
		tLast.m_uWords = 0;

		if ( bFillMasks && iWord )
		{
			bool bMatch = false;
			int iOffset;

			ARRAY_FOREACH ( nWord, m_dWords )
			{
				const char * sKeyword = &m_dKeywordsBuffer [ m_dKeywords[nWord].m_iWord ];
				const Token_t & tToken = m_dWords[nWord];

				switch ( m_dKeywords[nWord].m_uStar )
				{
				case STAR_NONE:
					bMatch = ( iWord==tToken.m_iWordID || iExactID==tToken.m_iWordID );
					break;

				case STAR_FRONT:
					iOffset = tLast.m_iLengthBytes - tToken.m_iLengthBytes;
					bMatch = ( iOffset>=0 ) &&
						( memcmp ( sKeyword, sWord + iOffset, tToken.m_iLengthBytes )==0 );
					break;

				case STAR_BACK:
					bMatch = ( tLast.m_iLengthBytes>=tToken.m_iLengthBytes ) &&
						( memcmp ( sKeyword, sWord, tToken.m_iLengthBytes )==0 );
					break;

				case STAR_BOTH:
					bMatch = strstr ( (const char *)sWord, sKeyword )!=NULL;
					break;
				}

				if ( bMatch )
				{
					tLast.m_uWords |= 1UL<<nWord;
					m_uFoundWords |= 1UL<<nWord;
				}
			}

			// check that bag of words in case of exact_phrase
			if ( m_bExactPhrase )
			{
				// FIXME! maybe should keep and roll-back m_uFoundWords state
				DWORD uExactWord = 1UL<<dExactPhrase.GetLength();
				bool bExactFound = ( ( tLast.m_uWords & uExactWord )!=0 );
				if ( bExactFound )
				{
					if ( dExactPhrase.GetLength()+1==m_dWords.GetLength() ) // phrase found, no need to track these QWords
						dExactPhrase.Resize ( 0 );
					else
						dExactPhrase.Add ( m_dTokens.GetLength()-1 ); // maybe phrase, keep collecting QWords into phrase list
				} else // clean up QWords from phrase and current one
				{
					ARRAY_FOREACH ( i, dExactPhrase )
						m_dTokens[dExactPhrase[i]].m_uWords = 0;
					dExactPhrase.Resize ( 0 );

					if ( (tLast.m_uWords & 1 )!=0 ) // it could be beginning of another exact phrase
						dExactPhrase.Add ( m_dTokens.GetLength()-1 );
					else
						tLast.m_uWords = 0;
				}
			}
		}
	}

	// clean up tail QWords from phrase list
	if ( m_bExactPhrase )
	{
		ARRAY_FOREACH ( i, dExactPhrase )
			m_dTokens[dExactPhrase[i]].m_uWords = 0;
	}

	// last space if any
	if ( pLastTokenEnd!=pTokenizer->GetBufferEnd() )
	{
		int iOffset = pTokenizer->GetBoundary() ? pTokenizer->GetBoundaryOffset() : -1;
		AddJunk ( pLastTokenEnd - pStartPtr, pTokenizer->GetBufferEnd () - pLastTokenEnd, iOffset );
	}

	Token_t & tLast = m_dTokens.Add();
	tLast.Reset();
}

void ExcerptGen_c::MarkHits ()
{
	assert ( m_pMarker );

	// mark
	CSphVector<SphHitMark_t> dMarked;
	dMarked.Reserve ( m_dTokens.GetLength() );
	m_pMarker->Mark ( dMarked );

	// fix-up word masks
	int iMarked = dMarked.GetLength();
	int iTokens = m_dTokens.GetLength();
	int i = 0, k = 0;
	while ( i < iTokens )
	{
		// sync
		while ( k < iMarked && m_dTokens[i].m_uPosition > dMarked[k].m_uPosition )
			k++;

		if ( k==iMarked ) // no more marked hits, clear tail
		{
			for ( ; i < iTokens; i++ )
				m_dTokens[i].m_uWords = 0;
			break;
		}

		// clear false matches
		while ( dMarked[k].m_uPosition > m_dTokens[i].m_uPosition )
		{
			m_dTokens[i++].m_uWords = 0;
			assert ( i<m_dTokens.GetLength() );
		}

		// skip tokens covered by hit's span
		assert ( dMarked[k].m_uPosition==m_dTokens[i].m_uPosition );
		assert ( dMarked[k].m_uSpan>=1 );
		while ( dMarked[k].m_uSpan-- )
		{
			i++;
			while ( i < iTokens && !m_dTokens[i].m_uPosition ) i++;
		}
	}
}


void ExcerptGen_c::SetExactPhrase ( const ExcerptQuery_t & tQuery )
{
	m_bExactPhrase = tQuery.m_bExactPhrase && ( m_dWords.GetLength()>1 );
}


void ExcerptGen_c::BuildExcerpt ( ExcerptQuery_t & tQuery )
{
	m_iPassageId = tQuery.m_iPassageId;

	if ( tQuery.m_bHighlightQuery )
		MarkHits();

	// assign word weights
	ARRAY_FOREACH ( i, m_dWords )
		m_dWords[i].m_iWeight = m_dWords[i].m_iLengthCP; // FIXME! should obtain freqs from dict

	// reset result
	m_dResult.Reserve ( 16384 );
	m_dResult.Resize ( 0 );
	m_iResultLen = 0;

	// do highlighting
	if ( ( tQuery.m_iLimit<=0 || tQuery.m_iLimit>m_iTotalCP )
		&& ( tQuery.m_iLimitWords<=0 || tQuery.m_iLimitWords>m_iDocumentWords ) && tQuery.m_ePassageSPZ==SPH_SPZ_NONE )
	{
		HighlightAll ( tQuery );

	} else
	{
		if ( !( ExtractPassages ( tQuery ) && HighlightBestPassages ( tQuery ) ) )
		{
			if ( !tQuery.m_bAllowEmpty )
				HighlightStart ( tQuery );
		}
	}

	// alloc, fill and return the result
	m_dResult.Add ( 0 );
	// if was stripped
	if ( tQuery.m_sStripMode=="strip" || tQuery.m_sStripMode=="index" )
	{
		// replacing sphinx magic (refer to sphinxint.h) characters in output
		tQuery.m_dRes.Resize ( m_dResult.GetLength() );
		const BYTE * s = m_dResult.Begin();
		const BYTE * pEnd = m_dResult.Begin() + m_dResult.GetLength();
		BYTE * d = tQuery.m_dRes.Begin();
		while ( s<pEnd )
		{
			BYTE c = *s++;
			*d++ = ( c>0 && c<=5 )?( ' ' ):( c );
		}
	} else
		tQuery.m_dRes.SwapData ( m_dResult );
	return;
}


void ExcerptGen_c::HighlightPhrase ( const ExcerptQuery_t & q, int iTok, int iEnd )
{
	int iPhrase = 0;
	for ( ; iTok<=iEnd; iTok++ )
	{
		bool bQWord = m_dTokens[iTok].m_uWords!=0;

		if ( bQWord && iPhrase==0 )
			ResultEmit ( q.m_sBeforeMatch.cstr(), q.m_bHasBeforePassageMacro, m_iPassageId, q.m_sBeforeMatchPassage.cstr() );

		ResultEmit ( m_dTokens[iTok] );
		iPhrase += bQWord ? 1 : 0;

		if ( bQWord && iPhrase==m_dWords.GetLength() )
		{
			ResultEmit ( q.m_sAfterMatch.cstr(), q.m_bHasAfterPassageMacro, m_iPassageId++, q.m_sAfterMatchPassage.cstr() );
			iPhrase = 0;
		}
	}
}


void ExcerptGen_c::HighlightAll ( const ExcerptQuery_t & q )
{
	bool bOpen = false;
	const int iMaxTok = m_dTokens.GetLength()-1; // skip last one, it's TOK_NONE

	if ( m_bExactPhrase )
	{
		HighlightPhrase ( q, 0, iMaxTok );
	} else
	{
		// bag of words
		for ( int iTok=0; iTok<iMaxTok; iTok++ )
		{
			if ( ( m_dTokens[iTok].m_uWords!=0 ) ^ bOpen )
			{
				if ( bOpen )
					ResultEmit ( q.m_sAfterMatch.cstr(), q.m_bHasAfterPassageMacro, m_iPassageId++, q.m_sAfterMatchPassage.cstr() );
				else
					ResultEmit ( q.m_sBeforeMatch.cstr(), q.m_bHasBeforePassageMacro, m_iPassageId, q.m_sBeforeMatchPassage.cstr() );
				bOpen = !bOpen;
			}
			ResultEmit ( m_dTokens[iTok] );
		}
		if ( bOpen )
			ResultEmit ( q.m_sAfterMatch.cstr(), q.m_bHasAfterPassageMacro, m_iPassageId++, q.m_sAfterMatchPassage.cstr() );
	}
}


void ExcerptGen_c::HighlightStart ( const ExcerptQuery_t & q )
{
	// no matches found. just show the starting tokens
	int i = 0;
	while ( m_dTokens.GetLength() && ( m_iResultLen+m_dTokens[i].m_iLengthCP < q.m_iLimit || i==0 ) )
	{
		ResultEmit ( m_dTokens[i++] );
		if ( i>=m_dTokens.GetLength() )
			break;
	}
	ResultEmit ( q.m_sChunkSeparator.cstr() );
}


void ExcerptGen_c::ResultEmit ( const char * sLine, bool bHasMacro, int iPassageId, const char * sPostPassage )
{
	// plain old emit
	while ( sLine && *sLine )
	{
		assert ( (*(BYTE*)sLine)<128 );
		m_dResult.Add ( *sLine++ );
		m_iResultLen++;
	}

	if ( !bHasMacro )
		return;

	char sBuf[16];
	int iPassLen = snprintf ( sBuf, sizeof(sBuf), "%d", iPassageId );
	for ( int i=0; i<iPassLen; i++ )
	{
		m_dResult.Add ( sBuf[i] );
		m_iResultLen++;
	}

	while ( sPostPassage && *sPostPassage )
	{
		m_dResult.Add ( *sPostPassage++ );
		m_iResultLen++;
	}
}


void ExcerptGen_c::ResultEmit ( const Token_t & sTok )
{
	for ( int i=0; i<sTok.m_iLengthBytes; i++ )
		m_dResult.Add ( m_sBuffer.cstr () [ i+sTok.m_iStart ] );

	m_iResultLen += sTok.m_iLengthCP;
}

/////////////////////////////////////////////////////////////////////////////

void ExcerptGen_c::CalcPassageWeight ( Passage_t & tPass, const TokenSpan_t & tSpan, int iMaxWords, int iWordCountCoeff )
{
	DWORD uLast = 0;
	int iLCS = 1;
	tPass.m_iMaxLCS = 1;

	// calc everything
	tPass.m_uQwords = 0;
	tPass.m_iMinGap = iMaxWords-1;
	tPass.m_iStartLimit = INT_MAX;
	tPass.m_iEndLimit = INT_MIN;
	tPass.m_iAroundBefore = tPass.m_iAroundAfter = 0;

	int iWord = -1;
	for ( int iTok=tSpan.m_iStart; iTok<=tSpan.m_iEnd; iTok++ )
	{
		Token_t & tTok = m_dTokens[iTok];
		if ( tTok.m_eType!=TOK_WORD )
			continue;
		iWord++;

		// update mask
		tPass.m_uQwords |= tTok.m_uWords;

		// update match boundary
		if ( tTok.m_uWords )
		{
			tPass.m_iStartLimit = Min ( tPass.m_iStartLimit, iTok );
			tPass.m_iEndLimit = Max ( tPass.m_iEndLimit, iTok );
		}

		// update LCS
		uLast = tTok.m_uWords & ( uLast<<1 );
		if ( uLast )
		{
			iLCS++;
			tPass.m_iMaxLCS = Max ( iLCS, tPass.m_iMaxLCS );
		} else
		{
			iLCS = 1;
			uLast = tTok.m_uWords;
		}

		// update min gap
		if ( tTok.m_uWords )
		{
			tPass.m_iMinGap = Min ( tPass.m_iMinGap, iWord );
			tPass.m_iMinGap = Min ( tPass.m_iMinGap, tSpan.m_iWords-1-iWord );
		}

		tPass.m_iAroundBefore += ( tPass.m_uQwords==0 );
		tPass.m_iAroundAfter = ( tTok.m_uWords ? 0 : tPass.m_iAroundAfter+1 );
	}
	assert ( tPass.m_iMinGap>=0 );
	assert ( tSpan.m_iWords==iWord+1 );

	// calc final weight
	tPass.m_iQwordsWeight = 0;
	tPass.m_iQwordCount = 0;

	DWORD uWords = tPass.m_uQwords;
	for ( int iWord=0; uWords; uWords >>= 1, iWord++ )
		if ( uWords & 1 )
	{
		tPass.m_iQwordsWeight += m_dWords[iWord].m_iWeight;
		tPass.m_iQwordCount++;
	}

	tPass.m_iMaxLCS *= iMaxWords;
	tPass.m_iQwordCount *= iWordCountCoeff;
}

void ExcerptGen_c::UpdateGaps ( Passage_t & tPass, const TokenSpan_t & tSpan, int iMaxWords )
{
	tPass.m_iMinGap = iMaxWords-1;
	tPass.m_iAroundBefore = tPass.m_iAroundAfter = 0;
	DWORD uQwords = 0;
	int iWord = -1;
	for ( int iTok=tSpan.m_iStart; iTok<=tSpan.m_iEnd; iTok++ )
	{
		Token_t & tTok = m_dTokens[iTok];
		if ( tTok.m_eType!=TOK_WORD )
			continue;

		iWord++;
		if ( tTok.m_uWords )
		{
			tPass.m_iMinGap = Min ( tPass.m_iMinGap, iWord );
			tPass.m_iMinGap = Min ( tPass.m_iMinGap, tSpan.m_iWords-1-iWord );
		}

		uQwords |= tTok.m_uWords;
		tPass.m_iAroundBefore += ( uQwords==0 );
		tPass.m_iAroundAfter = ( tTok.m_uWords ? 0 : tPass.m_iAroundAfter+1 );
	}
	assert ( tPass.m_iMinGap>=0 );
}


static int GetWordsLimit ( const ExcerptQuery_t & q, int iQwords )
{
	int iSoftLimit = 2*q.m_iAround + iQwords;
	if ( q.m_iLimitWords )
		return Min ( iSoftLimit, q.m_iLimitWords );

	return iSoftLimit;
}


bool ExcerptGen_c::SetupWindow ( TokenSpan_t & tSpan, Passage_t & tPass, int i, int iCpLimit, const ExcerptQuery_t & q )
{
	assert ( i>=0 && i<m_dTokens.GetLength() );

	tPass.Reset();
	tPass.m_iStart = i;

	for ( ; i<m_dTokens.GetLength(); i++ )
	{
		const Token_t & tToken = m_dTokens[i];

		// skip starting whitespace
		if ( tPass.m_iTokens==0 && tToken.m_eType!=TOK_WORD )
		{
			tPass.m_iStart++;
			continue;
		}

		// stop when the window is large enough or meet SPZ
		if ( ( tPass.m_iCodes + tToken.m_iLengthCP > iCpLimit )
			|| tSpan.m_iWords>=GetWordsLimit ( q, tSpan.m_iQwords )
			|| tToken.m_eType==TOK_SPZ )
		{
			tPass.m_iTokens += ( tToken.m_eType==TOK_SPZ && tToken.m_iLengthBytes>0 ); // only MAGIC_CODE_SENTENCE has length
			return ( tToken.m_eType==TOK_SPZ );
		}

		// got token, update passage
		tPass.m_iTokens++;
		tPass.m_iCodes += tToken.m_iLengthCP;

		if ( tToken.m_eType==TOK_WORD )
			tSpan.Add ( i, m_dTokens[i].m_uWords!=0 );
	}

	return false;
}

bool ExcerptGen_c::FlushPassage ( const Passage_t & tPass, int iLCSThresh )
{
	if (!( tPass.m_uQwords && tPass.m_iMaxLCS>=iLCSThresh ))
		return false;

	// if it's the very first one, do add
	if ( !m_dPassages.GetLength() )
	{
		m_dPassages.Add ( tPass );
		return true;
	}

	// check if it's new or better
	Passage_t & tLast = m_dPassages.Last();
	if ( ( tPass.m_iStartLimit<=tLast.m_iStartLimit && tLast.m_iEndLimit<=tPass.m_iEndLimit )
		|| ( tLast.m_iStartLimit<=tPass.m_iStartLimit && tPass.m_iEndLimit<=tLast.m_iEndLimit ) )
	{
		int iPassPre = tPass.m_iStartLimit - tPass.m_iStart + 1;
		int iPassPost = tPass.m_iStart + tPass.m_iTokens - tPass.m_iEndLimit + 1;
		float fPassGap = (float)Max ( iPassPre, iPassPost ) / (float)Min ( iPassPre, iPassPost );

		int iLastPre = tLast.m_iStartLimit - tLast.m_iStart + 1;
		int iLastPost = tLast.m_iStart + tLast.m_iTokens - tLast.m_iEndLimit + 1;
		float fLastGap = (float)Max ( iLastPre, iLastPost ) / (float)Min ( iLastPre, iLastPost );

		// centered snippet wins last passage
		if ( tLast.GetWeight()<tPass.GetWeight() || ( tLast.GetWeight()==tPass.GetWeight() && fPassGap<fLastGap ) )
			tLast = tPass;
	} else
		m_dPassages.Add ( tPass );

	return true;
}


bool ExcerptGen_c::ExtractPassages ( const ExcerptQuery_t & q )
{
	m_dPassages.Reserve ( 256 );
	m_dPassages.Resize ( 0 );

	if ( q.m_bUseBoundaries )
		return ExtractPhrases ( q );

	// my current passage
	TokenSpan_t tSpan;
	Passage_t tPass;
	tSpan.Reset ();
	tPass.Reset ();

	int iLCSThresh = 0;
	if ( m_bExactPhrase )
		iLCSThresh = m_dWords.GetLength() * GetWordsLimit ( q, m_dWords.GetLength() );
	int iCpLimit = q.m_iLimit ? q.m_iLimit : INT_MAX;

	// setup initial window
	bool bSPZ = SetupWindow ( tSpan, tPass, 0, iCpLimit, q );

	// move our window until the end of document
	const int iCount = m_dTokens.GetLength();
	bool bQwordsChanged = true;
	for ( ;; )
	{
		// does current token span has any matching words at all?
		if ( tSpan.m_iQwords )
		{
			// re-weight current passage, and submit it
			// if there weren't any keyword changes, use a fast path that only updates gap values
			if ( bQwordsChanged )
				CalcPassageWeight ( tPass, tSpan, GetWordsLimit ( q, tSpan.m_iQwords ), 0 );
			else
				UpdateGaps ( tPass, tSpan, GetWordsLimit ( q, tSpan.m_iQwords ) );
			tPass.m_iWords = tSpan.m_iWords;
			if ( FlushPassage ( tPass, iLCSThresh ) )
				bQwordsChanged = false;
		}

		int iToken = tPass.m_iStart + tPass.m_iTokens;
		assert ( iToken<=iCount );
		if ( iToken==iCount )
			break;

		if ( bSPZ )
		{
			tSpan.Reset();
			bSPZ = SetupWindow ( tSpan, tPass, iToken, iCpLimit, q );
			bQwordsChanged = true;
			continue;
		}

		// add another word
		for ( ; iToken < iCount; iToken++ )
		{
			tPass.m_iTokens++;
			tPass.m_iCodes += m_dTokens[iToken].m_iLengthCP;
			if ( m_dTokens[iToken].m_eType==TOK_WORD )
			{
				bool bQword = ( m_dTokens[iToken].m_uWords!=0 );
				tSpan.Add ( iToken, bQword );
				if ( bQword )
					bQwordsChanged = true;
				break;
			}

			if ( m_dTokens[iToken].m_eType==TOK_SPZ )
			{
				bSPZ = true;
				bQwordsChanged = true;
				break;
			}
		}
		if ( iToken==iCount || bSPZ )
			continue;

		// drop front tokens until the window fits into both word and CP limits
		while ( ( tPass.m_iCodes > iCpLimit || tSpan.m_iWords > GetWordsLimit ( q, tSpan.m_iQwords ) ) && tPass.m_iTokens!=1 )
		{
			if ( m_dTokens[tPass.m_iStart].m_eType==TOK_WORD )
			{
				// remove heading word from wordspan
				assert ( m_dTokens[tSpan.m_iStart].m_eType==TOK_WORD );
				if ( m_dTokens[tSpan.m_iStart].m_uWords )
				{
					tSpan.m_iQwords--;
					bQwordsChanged = true;
				}
				tSpan.m_iStart++;
				if ( tSpan.m_iStart > tSpan.m_iEnd )
				{
					tSpan.Reset();
				} else
				{
					tSpan.m_iWords--;
					while ( m_dTokens[tSpan.m_iStart].m_eType!=TOK_WORD )
						tSpan.m_iStart++;
				}
			}

			tPass.m_iCodes -= m_dTokens[tPass.m_iStart].m_iLengthCP;
			tPass.m_iTokens--;
			tPass.m_iStart++;
		}
	}

	return m_dPassages.GetLength()!=0;
}


bool ExcerptGen_c::ExtractPhrases ( const ExcerptQuery_t & )
{
	int iMaxWords = 100;
	int iLCSThresh = m_bExactPhrase ? m_dWords.GetLength()*iMaxWords : 0;

	int iStart = 0;
	DWORD uWords = 0;

	ARRAY_FOREACH ( iTok, m_dTokens )
	{
		// phrase boundary found, go flush
		if ( m_dTokens[iTok].m_eType==TOK_BREAK || m_dTokens[iTok].m_eType==TOK_NONE )
		{
			int iEnd = iTok - 1;

			// emit non-empty phrases with matching words as passages
			if ( iStart<iEnd && uWords!=0 )
			{
				Passage_t tPass;
				tPass.Reset ();

				tPass.m_iStart = iStart;
				tPass.m_iTokens = iEnd-iStart+1;

				TokenSpan_t tSpan;
				tSpan.Reset ();

				for ( int i=iStart; i<=iEnd; i++ )
				{
					tPass.m_iCodes += m_dTokens[i].m_iLengthCP;
					if ( m_dTokens[i].m_eType==TOK_WORD )
						tSpan.Add ( i, m_dTokens[i].m_uWords!=0 );
				}

				CalcPassageWeight ( tPass, tSpan, iMaxWords, 10000 );
				if ( tPass.m_iMaxLCS>=iLCSThresh )
				{
					tPass.m_iWords = tSpan.m_iWords;
					m_dPassages.Add ( tPass );
				}
			}

			if ( m_dTokens[iTok].m_eType==TOK_NONE )
				break;

			iStart = iTok + 1;
			uWords = 0;
		}

		// just an incoming token
		if ( m_dTokens[iTok].m_eType==TOK_WORD )
			uWords |= m_dTokens[iTok].m_uWords;
	}

	return m_dPassages.GetLength()!=0;
}


struct PassageOrder_fn
{
	inline bool IsLess ( const ExcerptGen_c::Passage_t & a, const ExcerptGen_c::Passage_t & b ) const
	{
		return a.m_iStart < b.m_iStart;
	}
};


bool ExcerptGen_c::HighlightBestPassages ( const ExcerptQuery_t & tQuery )
{
	assert ( m_dPassages.GetLength() );

	// needed for "slightly outta limit" check below
	int iKeywordsLength = 0;
	ARRAY_FOREACH ( i, m_dKeywords )
		iKeywordsLength += m_dKeywords[i].m_iLength;

	// our limits
	int iMaxPassages = tQuery.m_iLimitPassages
		? Min ( m_dPassages.GetLength(), tQuery.m_iLimitPassages )
		: m_dPassages.GetLength();
	int iMaxWords = tQuery.m_iLimitWords ? tQuery.m_iLimitWords : INT_MAX;
	int iMaxCp = tQuery.m_iLimit ? tQuery.m_iLimit : INT_MAX;

	// our best passages
	CSphVector<Passage_t> dShow;
	DWORD uWords = 0; // mask of words in dShow so far
	int iTotalCodes = 0;
	int iTotalWords = 0;
	bool bAroundComply = true;

	CSphVector<int> dWeights ( m_dPassages.GetLength() );
	ARRAY_FOREACH ( i, m_dPassages )
		dWeights[i] = m_dPassages[i].m_iQwordsWeight;

	// collect enough best passages to show all keywords and max out the limits
	// don't care much if we're going over limits in this loop, it will be tightened below
	bool bAll = false;
	while ( dShow.GetLength() < iMaxPassages )
	{
		// get next best passage
		int iBest = -1;
		ARRAY_FOREACH ( i, m_dPassages )
		{
			if ( m_dPassages[i].m_iCodes && ( iBest==-1 || m_dPassages[iBest] < m_dPassages[i] ) )
				iBest = i;
		}
		if ( iBest<0 )
			break;
		Passage_t & tBest = m_dPassages[iBest];

		// does this passage fit the limits?
		bool bFits = ( iTotalCodes + tBest.m_iCodes<=iMaxCp ) && ( iTotalWords + tBest.m_iWords<=iMaxWords );
		bAroundComply &= ( Max ( tBest.m_iAroundBefore, tBest.m_iAroundAfter )<=tQuery.m_iAround );

		// all words will be shown and we're outta limit
		if ( uWords==m_uFoundWords && !bFits )
		{
			// there might be just enough space to partially display this passage
			if ( ( iTotalCodes + iKeywordsLength )<=tQuery.m_iLimit )
				dShow.Add ( tBest );
			break;
		}

		// save it, despite limits or whatever, we'll tighten everything in the loop below
		dShow.Add ( tBest );
		uWords |= tBest.m_uQwords;
		iTotalWords += tBest.m_iWords;
		iTotalCodes += tBest.m_iCodes;
		tBest.m_iCodes = 0; // no longer needed here, abusing to mark displayed passages

		// we just managed to show all words? do one final re-weighting run
		if ( !bAll && uWords==m_uFoundWords )
		{
			bAll = true;
			ARRAY_FOREACH ( i, m_dPassages )
				m_dPassages[i].m_iQwordsWeight = dWeights[i];
		}

		// if we're already showing all words, re-weighting is not needed any more
		if ( bAll )
			continue;

		// re-weight passages, adjust for new mask of shown words
		ARRAY_FOREACH ( i, m_dPassages )
		{
			if ( !m_dPassages[i].m_iCodes )
				continue;
			DWORD uMask = tBest.m_uQwords;
			for ( int iWord=0; uMask; iWord++, uMask >>= 1 )
				if ( ( uMask & 1 ) && ( m_dPassages[i].m_uQwords & ( 1UL<<iWord ) ) )
					m_dPassages[i].m_iQwordsWeight -= m_dWords[iWord].m_iWeight;
			m_dPassages[i].m_uQwords &= ~uWords;
		}
	}

	// fix-up around constraint
	if ( !bAroundComply )
	{
		ARRAY_FOREACH ( i, dShow )
		{
			Passage_t & tPass = dShow[i];
			while ( tQuery.m_iAround<tPass.m_iAroundBefore )
			{
				assert ( tPass.m_iStart<tPass.m_iStartLimit );
				const Token_t & tTok = m_dTokens[tPass.m_iStart];
				assert ( tTok.m_uWords==0 );

				tPass.m_iCodes -= tTok.m_iLengthCP;
				iTotalCodes -= tTok.m_iLengthCP;

				tPass.m_iAroundBefore -= ( tTok.m_eType==TOK_WORD );
				iTotalWords -= ( tTok.m_eType==TOK_WORD );

				tPass.m_iStart++;
				tPass.m_iTokens--;
			}
			while ( tQuery.m_iAround<tPass.m_iAroundAfter )
			{
				assert ( tPass.m_iEndLimit<( tPass.m_iStart + tPass.m_iTokens - 1 ) );
				const Token_t & tTok = m_dTokens[tPass.m_iStart+tPass.m_iTokens-1];
				assert ( tTok.m_uWords==0 );

				tPass.m_iCodes -= tTok.m_iLengthCP;
				iTotalCodes -= tTok.m_iLengthCP;

				tPass.m_iAroundAfter -= ( tTok.m_eType==TOK_WORD );
				iTotalWords -= ( tTok.m_eType==TOK_WORD );

				tPass.m_iTokens--;
			}
		}
	}

	// if all passages won't fit into the limit, try to trim them a bit
	//
	// this step is skipped when use_boundaries is enabled, because
	// each passage must be a separate sentence (delimited by
	// boundaries) and we don't want to split them
	if ( ( iTotalCodes > iMaxCp || iTotalWords > iMaxWords ) && !tQuery.m_bUseBoundaries )
	{
		// trim passages
		bool bFirst = true;
		bool bDone = false;
		int iCodes = iTotalCodes;
		while ( !bDone )
		{
			// drop one token from each passage starting from the least relevant
			for ( int i=dShow.GetLength(); i > 0; i-- )
			{
				Passage_t & tPassage = dShow[i-1];
				int iFirst = tPassage.m_iStart;
				int iLast = tPassage.m_iStart + tPassage.m_iTokens - 1;
				if ( iFirst!=tPassage.m_iStartLimit && ( bFirst || iLast==tPassage.m_iEndLimit ) )
				{
					// drop first
					if ( ( tQuery.m_bForceAllWords && m_dTokens[tPassage.m_iStart].m_uWords==0 )
						|| !tQuery.m_bForceAllWords )
						tPassage.m_iStart++;
					tPassage.m_iTokens--;
					tPassage.m_iCodes -= m_dTokens[iFirst].m_iLengthCP;
					iTotalCodes -= m_dTokens[iFirst].m_iLengthCP;
					iTotalWords -= ( m_dTokens[iFirst].m_eType==TOK_WORD );

				} else if ( iLast!=tPassage.m_iEndLimit )
				{
					// drop last
					if ( ( tQuery.m_bForceAllWords && m_dTokens[tPassage.m_iStart+tPassage.m_iTokens-1].m_uWords==0 )
						|| !tQuery.m_bForceAllWords )
						tPassage.m_iTokens--;
					tPassage.m_iCodes -= m_dTokens[iLast].m_iLengthCP;
					iTotalCodes -= m_dTokens[iLast].m_iLengthCP;
					iTotalWords -= ( m_dTokens[iLast].m_eType==TOK_WORD );
				}
				if ( iTotalCodes<=iMaxCp && iTotalWords<=iMaxWords )
				{
					bDone = true;
					break;
				}
			}
			if ( iTotalCodes==iCodes )
				break; // couldn't reduce anything
			iCodes = iTotalCodes;
			bFirst = !bFirst;
		}
	}

	// if passages still don't fit start dropping least significant ones, limit is sacred.
	while ( ( iTotalCodes > iMaxCp || iTotalWords > iMaxWords ) && !tQuery.m_bForceAllWords )
	{
		iTotalCodes -= dShow.Last().m_iCodes;
		iTotalWords -= dShow.Last().m_iWords;
		dShow.RemoveFast ( dShow.GetLength()-1 );
	}

	if ( !dShow.GetLength() )
		return false;

	// sort passages in the document order
	if ( !tQuery.m_bWeightOrder )
		dShow.Sort ( PassageOrder_fn() );

	/// show
	int iLast = -1;
	bool bEmitZones = tQuery.m_bEmitZones && m_dZones.GetLength();
	ARRAY_FOREACH ( i, dShow )
	{
		int iTok = dShow[i].m_iStart;
		int iEnd = iTok + dShow[i].m_iTokens - 1;

		if ( ( iLast>=0 && iLast<m_dTokens.GetLength() && m_dTokens[iLast].m_eType==TOK_SPZ ) || iTok>1+iLast || tQuery.m_bWeightOrder )
		{
			ResultEmit ( tQuery.m_sChunkSeparator.cstr() );
			// find and emit most enclosing zone
			if ( bEmitZones )
			{
				int iHighlightStart = m_dTokens[iTok].m_iStart;
				int iZone = FindSpan ( m_dZonePos, iHighlightStart );
				if ( iZone!=-1 )
				{
					int iParent = m_dZoneParent[iZone];
					m_hZones.IterateStart();
					while ( m_hZones.IterateNext() )
					{
						if ( m_hZones.IterateGet()!=iParent )
							continue;

						ResultEmit ( "<" );
						ResultEmit ( m_hZones.IterateGetKey().cstr() );
						ResultEmit ( ">" );
						break;
					}
				}
			}
		}

		if ( m_bExactPhrase )
			HighlightPhrase ( tQuery, iTok, iEnd );
		else
		{
			while ( iTok<=iEnd )
			{
				if ( iTok>iLast || tQuery.m_bWeightOrder )
				{
					if ( m_dTokens[iTok].m_uWords )
					{
						ResultEmit ( tQuery.m_sBeforeMatch.cstr(), tQuery.m_bHasBeforePassageMacro, m_iPassageId, tQuery.m_sBeforeMatchPassage.cstr() );
						ResultEmit ( m_dTokens[iTok] );
						ResultEmit ( tQuery.m_sAfterMatch.cstr(), tQuery.m_bHasAfterPassageMacro, m_iPassageId++, tQuery.m_sAfterMatchPassage.cstr() );
					} else
						ResultEmit ( m_dTokens[iTok] );
				}
				iTok++;
			}
		}

		iLast = tQuery.m_bWeightOrder ? iEnd : Max ( iLast, iEnd );
	}
	if ( m_dTokens[iLast].m_eType!=TOK_NONE && m_dTokens[iLast+1].m_eType!=TOK_NONE )
		ResultEmit ( tQuery.m_sChunkSeparator.cstr() );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// FAST PATH FOR FULL DOCUMENT HIGHLIGHTING
//////////////////////////////////////////////////////////////////////////

struct DocQueryZonePair_t
{
	int m_iDoc;
	int m_iQuery;
	bool operator<( const DocQueryZonePair_t & b ) const { return m_iDoc<b.m_iDoc; }
	bool operator>( const DocQueryZonePair_t & b ) const { return m_iDoc>b.m_iDoc; }
	bool operator==( const DocQueryZonePair_t & b ) const { return m_iDoc==b.m_iDoc; }
};

/// hit-in-zone check implementation for the matching engine
class SnippetZoneChecker_c : public ISphZoneCheck
{
private:
	struct ZoneHits_t
	{
		CSphVector<Hitpos_t> m_dOpen;
		CSphVector<Hitpos_t> m_dClose;
	};

	CSphVector<ZoneHits_t> m_dZones;

public:
	SnippetZoneChecker_c ( const CSphVector<ZonePacked_t> & dDocZones,
		const SmallStringHash_T<int> & hDocNames, const CSphVector<CSphString> & dQueryZones )
	{
		if ( !dQueryZones.GetLength() )
			return;

		CSphVector<DocQueryZonePair_t> dCheckedZones;
		ARRAY_FOREACH ( i, dQueryZones )
		{
			int * pZone = hDocNames ( dQueryZones[i] );
			if ( pZone )
			{
				DocQueryZonePair_t & tPair = dCheckedZones.Add ();
				tPair.m_iDoc = *pZone;
				tPair.m_iQuery = i;
			}
		}

		dCheckedZones.Sort();
		m_dZones.Resize ( dQueryZones.GetLength() );

		ARRAY_FOREACH ( i, dDocZones )
		{
			uint64_t uZonePacked = dDocZones[i];
			DWORD uPos = (DWORD)( ( uZonePacked >>32 ) & UINT32_MASK );
			int iSibling = (int)( ( uZonePacked>>16 ) & UINT16_MASK );
			int iZone = (int)( uZonePacked & UINT16_MASK );
			assert ( iSibling>=0 && iSibling<dDocZones.GetLength() );
			assert ( iZone==(int)( dDocZones[iSibling] & UINT16_MASK ) );

			// skip cases:
			// + close zone (tSpan.m_iSibling<i) - skipped
			// + open without close zone (tSpan.m_iSibling==i) - skipped
			// + open zone position > close zone position
			// + zone type not in query zones
			if ( iSibling<=i || uPos>=( ( dDocZones[iSibling]>>32 ) & UINT32_MASK ) )
				continue;

			DocQueryZonePair_t tRefZone;
			tRefZone.m_iDoc = iZone;
			const DocQueryZonePair_t * pPair = dCheckedZones.BinarySearch ( tRefZone );
			if ( !pPair )
				continue;

			uint64_t uClosePacked = dDocZones[iSibling];
			DWORD uClosePos = ( (int)( uClosePacked>>32 ) & UINT32_MASK );

			ZoneHits_t & tZone = m_dZones[pPair->m_iQuery];
			tZone.m_dOpen.Add ( uPos );
			tZone.m_dClose.Add ( uClosePos );
		}

#ifndef NDEBUG
		ARRAY_FOREACH ( i, m_dZones )
		{
			const ZoneHits_t & tZone = m_dZones[i];
			assert ( tZone.m_dOpen.GetLength()==tZone.m_dClose.GetLength() );
			const Hitpos_t * pHit = tZone.m_dOpen.Begin()+1;
			const Hitpos_t * pMax = tZone.m_dOpen.Begin()+tZone.m_dOpen.GetLength();
			for ( ; pHit<pMax; pHit++ )
				assert ( pHit[-1]<pHit[0] );
			pHit = tZone.m_dClose.Begin()+1;
			pMax = tZone.m_dClose.Begin()+tZone.m_dClose.GetLength();
			for ( ; pHit<pMax; pHit++ )
				assert ( pHit[-1]<pHit[0] );
		}
#endif
	}

	virtual SphZoneHit_e IsInZone ( int iZone, const ExtHit_t * pHit, int * pLastSpan )
	{
		Hitpos_t uPos = HITMAN::GetLCS ( pHit->m_uHitpos );
		int iOpen = FindSpan ( m_dZones[iZone].m_dOpen, uPos );
		if ( pLastSpan )
			* pLastSpan = iOpen;
		return ( iOpen>=0 && uPos<=m_dZones[iZone].m_dClose[iOpen] ) ? SPH_ZONE_FOUND : SPH_ZONE_NO_SPAN;
	}
};

//////////////////////////////////////////////////////////////////////////

/// mini-index for a single document
/// keeps query words
/// keeps hit lists for every query keyword
class SnippetsDocIndex_c : public ISphNoncopyable
{
public:
	// document related
	DWORD									m_uLastPos;
	CSphVector< CSphVector<int> >			m_dDocHits;

	// query parsing result
	const XQQuery_t	&						m_tQuery;

protected:
	// query keywords and parsing stuff
	struct TermLength_t
	{
		SphWordID_t		m_iWordId;
		int				m_iLengthCP;
	};

	CSphVector<SphWordID_t>					m_dTerms;
	CSphVector<TermLength_t>				m_dTermLengths;
	CSphVector<SphWordID_t>					m_dStarred;
	CSphVector<ExcerptGen_c::Keyword_t>		m_dStars;
	CSphVector<BYTE>						m_dStarBuffer;
	CSphVector<int>							m_dTermWeights;
	CSphVector<CSphString>					m_dAllKeywords;
	int										m_iTotalKeywordLen;

	bool									m_bQueryMode;
	BYTE									m_sTmpWord [ 3*SPH_MAX_WORD_LEN + 16 ];

public:
	SnippetsDocIndex_c ( bool bQueryMode, const XQQuery_t & tQuery );
	void		SetupHits ();
	int			FindWord ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen ) const;
	int			FindStarred ( const char * sWord, int uStarPosition ) const;
	void		AddHits ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen, DWORD uPosition );
	void		ParseQuery ( const char * sQuery, ISphTokenizer * pTokenizer, CSphDict * pDict, DWORD eExtQuerySPZ );
	int			GetNumTerms () const;
	int			GetTermWeight ( int iTerm ) const;
	int			GetTotalKeywordLen() const;

protected:
	bool		MatchStar ( const ExcerptGen_c::Keyword_t & tTok, const BYTE * sWord, int iWordLen ) const;
	void		AddWord ( SphWordID_t iWordID, int iLengthCP );
	void		AddWord ( const char * sWord, int iStarPosition );
	void		ExtractWords ( XQNode_t * pNode, ISphTokenizer * pTokenizer, CSphDict * pDict );
};


SnippetsDocIndex_c::SnippetsDocIndex_c ( bool bQueryMode, const XQQuery_t & tQuery )
	: m_uLastPos ( 0 )
	, m_tQuery ( tQuery )
	, m_iTotalKeywordLen ( 0 )
	, m_bQueryMode ( bQueryMode )
{}


void SnippetsDocIndex_c::SetupHits ()
{
	m_dDocHits.Resize ( m_dTerms.GetLength() + m_dStars.GetLength() );
	m_uLastPos = 0;
}


bool SnippetsDocIndex_c::MatchStar ( const ExcerptGen_c::Keyword_t & tTok, const BYTE * sWord, int iWordLen ) const
{
	assert ( tTok.m_uStar!=STAR_NONE );
	const BYTE * sKeyword = m_dStarBuffer.Begin() + tTok.m_iWord;

	switch ( tTok.m_uStar )
	{
	case STAR_FRONT:
		{
			int iOffset = iWordLen - tTok.m_iLength;
			return ( iOffset>=0 ) && ( memcmp ( sKeyword, sWord + iOffset, tTok.m_iLength )==0 );
		}

	case STAR_BACK:
		return ( iWordLen>=tTok.m_iLength ) && ( memcmp ( sKeyword, sWord, tTok.m_iLength )==0 );

	case STAR_BOTH:
		return strstr ( (const char *)sWord, (const char *)sKeyword )!=NULL;
	}
	return false;
}


int SnippetsDocIndex_c::FindWord ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen ) const
{
	const SphWordID_t * pQueryID = iWordID ? m_dTerms.BinarySearch ( iWordID ) : NULL;
	if ( pQueryID )
		return pQueryID - m_dTerms.Begin();

	if ( sWord && iWordLen )
		ARRAY_FOREACH ( i, m_dStars )
			if ( MatchStar ( m_dStars[i], sWord, iWordLen ) )
				return i + m_dTerms.GetLength();

	return -1;
}

int SnippetsDocIndex_c::FindStarred ( const char * sWord, int uStarPosition ) const
{
	if ( !sWord || !uStarPosition )
		return -1;

	const BYTE * pBuf = m_dStarBuffer.Begin();
	int iLen = strlen ( sWord );
	ARRAY_FOREACH ( i, m_dStars )
	{
		const ExcerptGen_c::Keyword_t & tTok = m_dStars[i];
		if ( tTok.m_iLength==iLen && tTok.m_uStar==uStarPosition && memcmp ( pBuf+tTok.m_iWord, sWord, iLen )==0 )
			return i + m_dTerms.GetLength();
	}

	return -1;
}


void SnippetsDocIndex_c::AddHits ( SphWordID_t iWordID, const BYTE * sWord, int iWordLen, DWORD uPosition )
{
	assert ( m_dDocHits.GetLength()==m_dTerms.GetLength()+m_dStars.GetLength() );

	// FIXME!!! replace to 6well formed full-blown infix keyword dict
	const SphWordID_t * pQueryWord = ( iWordID ? m_dTerms.BinarySearch ( iWordID ) : NULL );
	if ( pQueryWord )
	{
		m_dDocHits [ pQueryWord - m_dTerms.Begin() ].Add ( uPosition );

		// might add hit to star hit-list too
		if ( !m_dStarred.BinarySearch ( iWordID ) )
			return;
	}

	if ( sWord && iWordLen )
		ARRAY_FOREACH ( i, m_dStars )
			if ( MatchStar ( m_dStars[i], sWord, iWordLen ) )
				m_dDocHits [ m_dTerms.GetLength() + i ].Add ( uPosition );
}


void SnippetsDocIndex_c::ParseQuery ( const char * sQuery, ISphTokenizer * pTokenizer, CSphDict * pDict, DWORD eExtQuerySPZ )
{
	m_iTotalKeywordLen = 0;
	m_dAllKeywords.Resize(0);

	int iQueryLen = 0;
	if ( !m_bQueryMode )
	{
		// parse bag-of-words query
		iQueryLen = strlen ( sQuery ); // FIXME!!! get length as argument
		pTokenizer->SetBuffer ( (BYTE *)sQuery, iQueryLen );

		BYTE * sWord = NULL;
		// FIXME!!! add warning on query words overflow
		while ( ( sWord = pTokenizer->GetToken() )!=NULL && ( m_dTerms.GetLength() + m_dStars.GetLength() )<MAX_HIGHLIGHT_WORDS )
		{
			SphWordID_t uWordID = pDict->GetWordID ( sWord );
			if ( !uWordID )
				continue;

			m_dAllKeywords.Add ( (const char*)sWord );

			bool bStarBack = ( *pTokenizer->GetTokenEnd()=='*' );
			bool bStarFront = ( pTokenizer->GetTokenStart()!=pTokenizer->GetBufferPtr() ) &&
				( pTokenizer->GetTokenStart()[-1]=='*' );
			int uStar = ( bStarFront ? STAR_FRONT : 0 ) | ( bStarBack ? STAR_BACK : 0 );

			if ( uStar )
				AddWord ( (const char *)sWord, uStar );
			else
				AddWord ( uWordID, pTokenizer->GetLastTokenLen() );
		}

		m_dAllKeywords.Uniq();

		ARRAY_FOREACH ( i, m_dAllKeywords )
			m_iTotalKeywordLen += pTokenizer->IsUtf8() ? sphUTF8Len ( m_dAllKeywords[i].cstr() ) : m_dAllKeywords[i].Length();
	} else
	{
		ExtractWords ( m_tQuery.m_pRoot, pTokenizer, pDict );
		m_dAllKeywords.Uniq();

		ARRAY_FOREACH ( i, m_dAllKeywords )
			m_iTotalKeywordLen += pTokenizer->IsUtf8() ? sphUTF8Len ( m_dAllKeywords[i].cstr() ) : m_dAllKeywords[i].Length();

		if ( eExtQuerySPZ & SPH_SPZ_SENTENCE )
		{
			strncpy ( (char *)m_sTmpWord, MAGIC_WORD_SENTENCE, sizeof(m_sTmpWord) );
			AddWord ( pDict->GetWordID ( m_sTmpWord ), strlen ( (char*)m_sTmpWord ) );
		}
		if ( eExtQuerySPZ & SPH_SPZ_PARAGRAPH )
		{
			strncpy ( (char *)m_sTmpWord, MAGIC_WORD_PARAGRAPH, sizeof(m_sTmpWord) );
			AddWord ( pDict->GetWordID ( m_sTmpWord ), strlen ( (char*)m_sTmpWord ) );
		}

		// should be in sync with ExtRanker_c constructor
		ARRAY_FOREACH ( i, m_tQuery.m_dZones )
		{
			snprintf ( (char *)m_sTmpWord, sizeof(m_sTmpWord), "%c%s", MAGIC_CODE_ZONE, m_tQuery.m_dZones[i].cstr() );
			AddWord ( pDict->GetWordID ( m_sTmpWord ), strlen ( (char*)m_sTmpWord ) );
		}
	}

	// all ok, remove dupes
	m_dTerms.Uniq();
	assert ( !m_dStars.GetLength() || m_dStarBuffer.GetLength() );

	// plain terms could also match as starred terms
	if ( m_dStars.GetLength() && m_dTerms.GetLength() && m_bQueryMode )
	{
		CSphVector<const XQNode_t *> dChildren;
		dChildren.Add ( m_tQuery.m_pRoot );
		ARRAY_FOREACH ( i, dChildren )
		{
			const XQNode_t * pChild = dChildren[i];
			if ( !pChild )
				continue;

			ARRAY_FOREACH ( j, pChild->m_dChildren )
				dChildren.Add ( pChild->m_dChildren[j] );

			ARRAY_FOREACH ( j, pChild->m_dWords )
			{
				if ( pChild->m_dWords[j].m_uStarPosition )
					continue;

				const BYTE * sWord = (const BYTE *)pChild->m_dWords[j].m_sWord.cstr();
				int iLen = pChild->m_dWords[j].m_sWord.Length();
				ARRAY_FOREACH ( k, m_dStars )
				{
					if ( MatchStar ( m_dStars[k], sWord, iLen ) )
					{
						memcpy ( m_sTmpWord, sWord, iLen );
						m_dStarred.Add ( pDict->GetWordID ( m_sTmpWord ) );
						break;
					}
				}
			}
		}

		m_dStarred.Uniq();
	}

	m_dTermWeights.Reserve ( m_dTerms.GetLength()+m_dStars.GetLength() );

	// FIXME! should obtain freqs from dict
	ARRAY_FOREACH ( i, m_dTerms )
		ARRAY_FOREACH ( j, m_dTermLengths )
			if ( m_dTerms[i]==m_dTermLengths[j].m_iWordId )
			{
				m_dTermWeights.Add ( m_dTermLengths[j].m_iLengthCP );
				break;
			}

	ARRAY_FOREACH ( i, m_dStars )
		m_dTermWeights.Add ( sphUTF8Len ( (const char *)&( m_dStarBuffer[m_dStars[i].m_iWord] ) ) );
}


int SnippetsDocIndex_c::GetNumTerms () const
{
	return m_dTerms.GetLength();
}


int	SnippetsDocIndex_c::GetTermWeight ( int iTerm ) const
{
	return m_dTermWeights[iTerm];
}


int	SnippetsDocIndex_c::GetTotalKeywordLen() const
{
	return m_iTotalKeywordLen;
}


void SnippetsDocIndex_c::AddWord ( SphWordID_t iWordID, int iLengthCP )
{
	assert ( iWordID );
	m_dTerms.Add ( iWordID );
	TermLength_t & tTermLength = m_dTermLengths.Add();
	tTermLength.m_iWordId = iWordID;
	tTermLength.m_iLengthCP = iLengthCP;
}


void SnippetsDocIndex_c::AddWord ( const char * sWord, int iStarPosition )
{
	int iLen = strlen ( sWord );
	int iOff = m_dStarBuffer.GetLength();

	m_dStarBuffer.Resize ( iOff+iLen+1 ); // reserve space for word + trailing zero
	memcpy ( &m_dStarBuffer[iOff], sWord, iLen );
	m_dStarBuffer[iOff+iLen] = 0;

	ExcerptGen_c::Keyword_t & tTok = m_dStars.Add();
	tTok.m_iWord = iOff;
	tTok.m_iLength = iLen;
	tTok.m_uStar = iStarPosition;
}


void SnippetsDocIndex_c::ExtractWords ( XQNode_t * pNode, ISphTokenizer * pTokenizer, CSphDict * pDict )
{
	if ( !pNode )
		return;

	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		const XQKeyword_t & tWord = pNode->m_dWords[i];
		m_dAllKeywords.Add ( tWord.m_sWord );

		if ( tWord.m_uStarPosition )
		{
			AddWord ( tWord.m_sWord.cstr(), tWord.m_uStarPosition );
		} else
		{
			strncpy ( (char *)m_sTmpWord, tWord.m_sWord.cstr(), sizeof(m_sTmpWord) );
			SphWordID_t iWordID = pDict->GetWordID ( m_sTmpWord );
			if ( iWordID )
				AddWord ( iWordID, pTokenizer->IsUtf8() ? sphUTF8Len ( tWord.m_sWord.cstr() ) : tWord.m_sWord.Length() );
		}
	}

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		ExtractWords ( pNode->m_dChildren[i], pTokenizer, pDict );
}

//////////////////////////////////////////////////////////////////////////

/// document token processor functor traits
class TokenFunctorTraits_c : public ISphNoncopyable, public ExcerptQuery_t
{
public:
	CSphVector<ZonePacked_t>	m_dZones;
	SmallStringHash_T<int>		m_hZones;
	CSphVector<BYTE>					m_dResult;

	SnippetsDocIndex_c &	m_tContainer;
	ISphTokenizer *			m_pTokenizer;
	CSphDict *				m_pDict;
	const char *			m_pDoc;

	int		m_iBoundaryStep;
	int		m_iStopwordStep;
	bool	m_bIndexExactWords;
	int		m_iDocLen;
	int		m_iMatchesCount;

	explicit TokenFunctorTraits_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer,
		CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex,
		const char * sDoc, int iDocLen )
		: m_tContainer ( tContainer )
		, m_pTokenizer ( pTokenizer )
		, m_pDict ( pDict )
		, m_pDoc ( NULL )
		, m_iBoundaryStep ( tSettingsIndex.m_iBoundaryStep )
		, m_iStopwordStep ( tSettingsIndex.m_iStopwordStep )
		, m_bIndexExactWords ( tSettingsIndex.m_bIndexExactWords )
		, m_iDocLen ( iDocLen )
		, m_iMatchesCount ( 0 )
	{
		assert ( m_pTokenizer && m_pDict );
		ExcerptQuery_t::operator = ( tQuery );
		m_pTokenizer->SetBuffer ( (BYTE*)sDoc, m_iDocLen );
		m_pDoc = m_pTokenizer->GetBufferPtr();
	}

	virtual ~TokenFunctorTraits_c () {}

	void ResultEmit ( const char * pSrc, int iLen, bool bHasPassageMacro=false, int iPassageId=0,
		const char * pPost=NULL, int iPostLen=0 )
	{
		ResultEmit ( m_dResult, pSrc, iLen, bHasPassageMacro, iPassageId, pPost, iPostLen );
	}

	void ResultEmit ( CSphVector<BYTE> & dResult, const char * pSrc, int iLen, bool bHasPassageMacro=false, int iPassageId=0,
		const char * pPost=NULL, int iPostLen=0 )
	{
		if ( iLen>0 )
		{
			int iOutLen = dResult.GetLength();
			dResult.Resize ( iOutLen+iLen );
			memcpy ( &dResult[iOutLen], pSrc, iLen );
		}

		if ( !bHasPassageMacro )
			return;

		char sBuf[16];
		int iPassLen = snprintf ( sBuf, sizeof(sBuf), "%d", iPassageId );
		int iOutLen = dResult.GetLength();
		dResult.Resize ( iOutLen + iPassLen + iPostLen );

		if ( iPassLen )
			memcpy ( dResult.Begin()+iOutLen, sBuf, iPassLen );
		if ( iPostLen )
			memcpy ( dResult.Begin()+iOutLen+iPassLen, pPost, iPostLen );
	}

	virtual void OnOverlap ( int iStart, int iLen ) = 0;
	virtual void OnSkipHtml ( int iStart, int iLen ) = 0;
	virtual void OnToken ( int iStart, int iLen, const BYTE * sWord, DWORD uPosition, const CSphVector<SphWordID_t> & dWordids ) = 0;
	virtual void OnSPZ ( BYTE iSPZ, DWORD uPosition, char * sZoneName ) = 0;
	virtual void OnTail ( int iStart, int iLen ) = 0;
	virtual void OnFinish () = 0;
	virtual const CSphVector<int> * GetHitlist ( const XQKeyword_t & tWord ) const = 0;
};


/// functor that processes tokens and collects matching keyword hits into mini-index
class HitCollector_c : public TokenFunctorTraits_c
{
public:
	mutable BYTE	m_sTmpWord [ 3*SPH_MAX_WORD_LEN + 16 ];
	SphWordID_t		m_uSentenceID;
	SphWordID_t		m_uParagraphID;

public:
	explicit HitCollector_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer,
		CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex,
		const char * sDoc, int iDocLen )
		: TokenFunctorTraits_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen )
	{
		strncpy ( (char *)m_sTmpWord, MAGIC_WORD_SENTENCE, sizeof(m_sTmpWord) );
		m_uSentenceID = pDict->GetWordID ( m_sTmpWord );
		strncpy ( (char *)m_sTmpWord, MAGIC_WORD_PARAGRAPH, sizeof(m_sTmpWord) );
		m_uParagraphID = pDict->GetWordID ( m_sTmpWord );
		m_tContainer.SetupHits();
	}

	virtual ~HitCollector_c () {}

	virtual void OnToken ( int, int iLen, const BYTE * sWord, DWORD uPosition, const CSphVector<SphWordID_t> & dWordids )
	{
		bool bReal = false;
		ARRAY_FOREACH ( i, dWordids )
			if ( dWordids[i] )
		{
			m_tContainer.AddHits ( dWordids[i], sWord, iLen, uPosition );
			bReal = true;
		}
		m_tContainer.m_uLastPos = bReal ? uPosition : m_tContainer.m_uLastPos;
	}

	virtual void OnSPZ ( BYTE iSPZ, DWORD uPosition, char * sZoneName )
	{
		switch ( iSPZ )
		{
		case MAGIC_CODE_SENTENCE:
			m_tContainer.AddHits ( m_uSentenceID, NULL, 0, uPosition );
			break;
		case MAGIC_CODE_PARAGRAPH:
			m_tContainer.AddHits ( m_uParagraphID, NULL, 0, uPosition );
			break;
		case MAGIC_CODE_ZONE:
			assert ( m_dZones.GetLength() );
			assert ( ( ( m_dZones.Last()>>32 ) & UINT32_MASK )==uPosition );
			assert ( sZoneName );

			m_tContainer.AddHits ( m_pDict->GetWordID ( (BYTE *)sZoneName ), NULL, 0, uPosition );
			break;
		default: assert ( 0 && "impossible SPZ" );
		}

		m_tContainer.m_uLastPos = uPosition;
	}

	virtual const CSphVector<int> * GetHitlist ( const XQKeyword_t & tWord ) const
	{
		int iWord = -1;
		if ( tWord.m_uStarPosition )
		{
			iWord = m_tContainer.FindStarred ( tWord.m_sWord.cstr(), tWord.m_uStarPosition );
		} else
		{
			strncpy ( (char *)m_sTmpWord, tWord.m_sWord.cstr(), sizeof(m_sTmpWord) );
			SphWordID_t iWordID = m_pDict->GetWordID ( m_sTmpWord );
			if ( iWordID )
				iWord = m_tContainer.FindWord ( iWordID, NULL, 0 );
		}

		if ( iWord!=-1 )
			return m_tContainer.m_dDocHits.Begin()+iWord;
		else
			return NULL;
	}

	virtual void OnOverlap ( int, int ) {}
	virtual void OnSkipHtml ( int, int ) {}
	virtual void OnTail ( int, int ) {}
	virtual void OnFinish () {}
};


/// functor that matches tokens against query words from mini-index and highlights them
class HighlightPlain_c : public TokenFunctorTraits_c
{
protected:
	int		m_iBeforeLen;
	int		m_iAfterLen;
	int		m_iBeforePostLen;
	int		m_iAfterPostLen;

public:
	HighlightPlain_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer,
		CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex,
		const char * sDoc, int iDocLen )
		: TokenFunctorTraits_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen )
		, m_iBeforeLen ( tQuery.m_sBeforeMatch.Length() )
		, m_iAfterLen ( tQuery.m_sAfterMatch.Length() )
		, m_iBeforePostLen ( tQuery.m_sBeforeMatchPassage.Length() )
		, m_iAfterPostLen ( tQuery.m_sAfterMatchPassage.Length() )
	{
		m_dResult.Reserve ( m_iDocLen );
	}

	virtual ~HighlightPlain_c () {}

	virtual void OnOverlap ( int iStart, int iLen )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pTokenizer->GetBufferEnd() );
		ResultEmit ( m_pDoc+iStart, iLen );
	}

	virtual void OnSkipHtml ( int iStart, int iLen )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pTokenizer->GetBufferEnd() );
		ResultEmit ( m_pDoc+iStart, iLen );
	}

	virtual void OnToken ( int iStart, int iLen, const BYTE * sWord, DWORD, const CSphVector<SphWordID_t> & dWordids )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pTokenizer->GetBufferEnd() );

		bool bMatch = m_tContainer.FindWord ( dWordids[0], sWord , iLen )!=-1; // the primary one; need this for star matching
		for ( int i=1; i<dWordids.GetLength() && !bMatch; i++ )
			if ( m_tContainer.FindWord ( dWordids[i], NULL, 0 )!=-1 )
				bMatch = true;

		if ( bMatch )
		{
			ResultEmit ( m_sBeforeMatch.cstr(), m_iBeforeLen, m_bHasBeforePassageMacro,
				m_iPassageId, m_sBeforeMatchPassage.cstr(), m_iBeforePostLen );
			m_iMatchesCount++;
		}

		ResultEmit ( m_pDoc+iStart, iLen );

		if ( bMatch )
			ResultEmit ( m_sAfterMatch.cstr(), m_iAfterLen, m_bHasAfterPassageMacro,
				m_iPassageId++, m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
	}

	virtual void OnSPZ ( BYTE , DWORD, char * ) {}

	virtual void OnTail ( int iStart, int iLen )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pTokenizer->GetBufferEnd() );
		ResultEmit ( m_pDoc+iStart, iLen );
	}

	virtual void OnFinish () {}

	virtual const CSphVector<int> * GetHitlist ( const XQKeyword_t & ) const
	{
		return NULL;
	}
};


/// functor that matches tokens against hit positions from mini-index and highlights them
class HighlightQuery_c : public HighlightPlain_c
{
public:
	const SphHitMark_t * m_pHit;
	const SphHitMark_t * m_pHitEnd;

public:
	HighlightQuery_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict,
		const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex,
		const char * sDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits )
		: HighlightPlain_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen )
		, m_pHit ( dHits.Begin() )
		, m_pHitEnd ( dHits.Begin()+dHits.GetLength() )
	{}

	virtual ~HighlightQuery_c () {}

	virtual void OnToken ( int iStart, int iLen, const BYTE *, DWORD uPosition, const CSphVector<SphWordID_t> & )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pTokenizer->GetBufferEnd() );

		// fast forward until next potentially matching hit (hits are sorted by position)
		while ( m_pHit<m_pHitEnd && m_pHit->m_uPosition+m_pHit->m_uSpan<=uPosition )
			m_pHit++;

		// marker folding, emit "before" marker at span start only
		if ( m_pHit<m_pHitEnd && uPosition==m_pHit->m_uPosition )
		{
			ResultEmit ( m_sBeforeMatch.cstr(), m_iBeforeLen, m_bHasBeforePassageMacro,
				m_iPassageId, m_sBeforeMatchPassage.cstr(), m_iBeforePostLen );
			m_iMatchesCount++;
		}

		// emit token itself
		ResultEmit ( m_pDoc+iStart, iLen );

		// marker folding, emit "after" marker at span end only
		if ( m_pHit<m_pHitEnd && uPosition==m_pHit->m_uPosition+m_pHit->m_uSpan-1 )
			ResultEmit ( m_sAfterMatch.cstr(), m_iAfterLen, m_bHasAfterPassageMacro,
				m_iPassageId++, m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
	}
};


enum SpanTokenType_e
{
	TOK_SPACE,
	TOK_WORD
};

struct StoredToken_t
{
	int				m_iLengthCP;
	SpanTokenType_e	m_eType;
};

struct Passage_t
{
	int					m_iStart;			///< start token index
	int					m_iTokens;			///< token count
	int					m_iCodes;			///< codepoints count
	int					m_iWords;			///< words count
	DWORD				m_uQwords;			///< matching query words mask
	int					m_iQwordsWeight;	///< passage weight factor
	int					m_iQwordCount;		///< passage weight factor
	int					m_iMaxLCS;			///< passage weight factor
	int					m_iMinGap;			///< passage weight factor
	int					m_iStartLimit;		///< start of match in passage
	int					m_iEndLimit;		///< end of match in passage
	int					m_iAroundBefore;	///< words before first matched words
	int					m_iAroundAfter;		///< words after last matched words
	CSphVector<StoredToken_t> m_dBeforeTokens;	///< stored tokens before match
	CSphVector<StoredToken_t> m_dAfterTokens;	///< stored tokens after match

	void Reset ()
	{
		m_iStart = 0;
		m_iTokens = 0;
		m_iCodes = 0;
		m_uQwords = 0;
		m_iQwordsWeight = 0;
		m_iQwordCount = 0;
		m_iMaxLCS = 0;
		m_iMinGap = 0;
		m_iAroundBefore = 0;
		m_iAroundAfter = 0;
		m_dBeforeTokens.Resize(0);
		m_dAfterTokens.Resize(0);
	}

	void CopyData ( Passage_t & tPassage )
	{
		m_iStart = tPassage.m_iStart;
		m_iTokens = tPassage.m_iTokens;
		m_iCodes = tPassage.m_iCodes;
		m_iWords = tPassage.m_iWords;
		m_uQwords = tPassage.m_uQwords;
		m_iQwordsWeight = tPassage.m_iQwordsWeight;
		m_iQwordCount = tPassage.m_iQwordCount;
		m_iMaxLCS = tPassage.m_iMaxLCS;
		m_iMinGap = tPassage.m_iMinGap;
		m_iStartLimit = tPassage.m_iStartLimit;
		m_iEndLimit = tPassage.m_iEndLimit;
		m_iAroundBefore = tPassage.m_iAroundBefore;
		m_iAroundAfter = tPassage.m_iAroundAfter;
		m_dBeforeTokens.SwapData ( tPassage.m_dBeforeTokens );
		m_dAfterTokens.SwapData ( tPassage.m_dAfterTokens );
	}

	inline int GetWeight () const
	{
		return m_iQwordCount + m_iQwordsWeight*m_iMaxLCS + m_iMinGap;
	}
};

inline bool operator < ( const Passage_t & a, const Passage_t & b )
{
	if ( a.GetWeight()==b.GetWeight() )
		return a.m_iCodes < b.m_iCodes;
	return a.GetWeight() < b.GetWeight();
}


struct PassagePositionOrder_fn
{
	inline bool IsLess ( const Passage_t & a, const Passage_t & b ) const
	{
		return a.m_iStart < b.m_iStart;
	}
};


struct SpanToken_t
{
	SpanTokenType_e		m_eType;			///< token type
	DWORD				m_uQwordMask;		///< query words mask
	int					m_iLengthCP;		///< token length in codepoints
};


struct Space_t
{
	int					m_iStartBytes;		///< offset from doc start
	int					m_iLengthBytes;		///< length in bytes
	int					m_iLengthCP;		///< length in codepoints
};


struct TokenSpan_t
{
	int		m_iStart;	///< starting token
	int		m_iWords;	///< number of TOK_WORDS tokens
	int		m_iQwords;	///< number of words matching query
	int		m_iCodes;	///< total length in codepoints (cached)

	void Init ( int nStoredTokens )
	{
		m_dTokens.Resize ( nStoredTokens );
		Reset ();
	}

	void Reset ()
	{
		m_iStart = -1;
		m_iWords = 0;
		m_iQwords = 0;
		m_iCodes = 0;
		m_iStartIndex = 0;
		m_iNumTokens = 0;
	}

	void Add ( int iToken, SphWordID_t iWord, bool bStopWord, bool bQWord, int iTermIndex, int iLengthCP )
	{
		int iEndIndex = (m_iStartIndex+m_iNumTokens) % m_dTokens.GetLength();
		if ( m_iNumTokens==m_dTokens.GetLength() )
		{
			// not enough preallocated tokens - realloc
			CSphTightVector<SpanToken_t> dNewTokens;
			dNewTokens.Resize ( m_dTokens.GetLength()*2 );
			for ( int i = 0; i < m_iNumTokens; i++ )
				dNewTokens[i]=GetToken(i);

			dNewTokens.SwapData ( m_dTokens );

			m_iStartIndex = 0;
			iEndIndex = m_iNumTokens;
		}

		m_iNumTokens++;

		SpanToken_t & tToken = m_dTokens[iEndIndex];
		tToken.m_eType = ( iWord || bStopWord ) ? TOK_WORD : TOK_SPACE;
		tToken.m_iLengthCP = iLengthCP;
		tToken.m_uQwordMask = 0;

		m_iCodes += iLengthCP;

		if ( tToken.m_eType==TOK_WORD )
		{
			m_iWords++;
			if ( bQWord )
			{
				tToken.m_uQwordMask = iTermIndex==-1 ? 0 : 1 << iTermIndex;
				m_iQwords += bQWord;
			}

			assert ( m_iStart<iToken );
			if ( m_iStart<0 )
				m_iStart = iToken;
		}
	}

	int GetNumTokens () const
	{
		return m_iNumTokens;
	}

	const SpanToken_t & GetToken ( int iToken ) const
	{
		assert ( iToken<=m_iNumTokens );
		int iIndex = (m_iStartIndex+iToken) % m_dTokens.GetLength();
		return m_dTokens[iIndex];
	}

	void RemoveStartingTokens ( int nTokens )
	{
		assert ( nTokens<m_iNumTokens );
		m_iStartIndex = (m_iStartIndex+nTokens) % m_dTokens.GetLength();
		m_iNumTokens -= nTokens;
	}

private:
	int		m_iStartIndex;
	int		m_iNumTokens;
	CSphTightVector<SpanToken_t> m_dTokens;
};


static void SplitSpaceIntoTokens ( CSphVector<Space_t> & dSpaces, const char * pDoc, bool bUTF8, int iStart, int iLen )
{
	dSpaces.Resize(0);

	int iGapStart = iStart;
	bool bWasSpace = sphIsSpace ( *(pDoc+iStart) );

	for ( int i=iStart; i<iStart+iLen; i++ )
	{
		bool bSpace = sphIsSpace ( *(pDoc+i) );
		if ( bSpace!=bWasSpace )
		{
			Space_t & tLastSpace = dSpaces.Add();
			tLastSpace.m_iStartBytes = iGapStart;
			tLastSpace.m_iLengthBytes = i-iGapStart;
			tLastSpace.m_iLengthCP = bUTF8 ? sphUTF8Len ( pDoc+tLastSpace.m_iStartBytes, tLastSpace.m_iLengthBytes ) : tLastSpace.m_iLengthBytes;

			bWasSpace = bSpace;
			iGapStart = i;
		}
	}

	if ( iGapStart < iStart+iLen )
	{
		Space_t & tLastSpace = dSpaces.Add();
		tLastSpace.m_iStartBytes = iGapStart;
		tLastSpace.m_iLengthBytes = iStart+iLen-iGapStart;
		tLastSpace.m_iLengthCP = bUTF8 ? sphUTF8Len ( pDoc+tLastSpace.m_iStartBytes, tLastSpace.m_iLengthBytes ) : tLastSpace.m_iLengthBytes;
	}
}


/// functor that extracts passages for further highlighting
class ExtractExcerpts_c : public TokenFunctorTraits_c
{
public:
	const SphHitMark_t *	m_pHit;
	const SphHitMark_t *	m_pHitEnd;
	CSphVector<Passage_t>	m_dPassages;
	CSphVector<BYTE>		m_dStartResult;

public:
	ExtractExcerpts_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex, const char * sDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits )
		: TokenFunctorTraits_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen )
		, m_pHit		( dHits.Begin() )
		, m_pHitEnd		( dHits.Begin()+dHits.GetLength() )
		, m_eState		( STATE_WINDOW_SETUP )
		, m_bQwordsChanged ( true )
		, m_iCurToken	( 0 )
		, m_uFoundWords	( 0 )
		, m_bCollectionStopped ( false )
	{
		const int AVG_WORD_LEN = 5;
		int iSpanSize = m_iLimit ? 2*m_iLimit/AVG_WORD_LEN : 2*m_iLimitWords;
		m_tSpan.Init ( iSpanSize );
		m_tPass.Reset();
		m_dPassages.Reserve(1024);
	}

	virtual void OnToken ( int iStart, int iLen, const BYTE * sWord, DWORD uPosition, const CSphVector<SphWordID_t> & dWordIds )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pTokenizer->GetBufferEnd() );

		CollectStartText ( iStart, iLen );

		while ( m_pHit<m_pHitEnd && m_pHit->m_uPosition+m_pHit->m_uSpan<=uPosition )
			m_pHit++;

		bool bQWord = false;
		if ( m_pHit<m_pHitEnd && uPosition>=m_pHit->m_uPosition && uPosition<=m_pHit->m_uPosition+m_pHit->m_uSpan )
			bQWord = true;

		SphWordID_t iWord = dWordIds[0];
		bool bIsStopWord = iWord ? false : m_pDict->IsStopWord ( sWord );

		int iLengthCP = sphUTF8Len ( (const char *)sWord );
		const int iCpLimit = m_iLimit ? m_iLimit : INT_MAX;

		if ( !iWord )
			bQWord = false;

		int iTermIndex = m_tContainer.FindWord ( dWordIds[0], sWord, iLen );
		for ( int i=1; i<dWordIds.GetLength() && iTermIndex==-1; i++ )
			iTermIndex = m_tContainer.FindWord ( dWordIds[i], NULL, 0 );

		m_uFoundWords |= iTermIndex==-1 ? 0 : 1 << iTermIndex;

		switch ( m_eState )
		{
		case STATE_WINDOW_SETUP:
			{
				// skip starting whitespace
				if ( !m_tSpan.GetNumTokens() && !iWord && !bIsStopWord )
					break;

				if ( ( m_tSpan.m_iCodes + iLengthCP > iCpLimit ) || m_tSpan.m_iWords>=GetWordsLimit ( *this, m_tSpan.m_iQwords ) )
				{
					m_bQwordsChanged = true;
					WeightAndSubmit ();
					m_eState = STATE_ADD_WORD;
				}

				m_tSpan.Add ( m_iCurToken, iWord, bIsStopWord, bQWord, iTermIndex, iLengthCP );
				m_bQwordsChanged |= bQWord;

				if ( m_eState==STATE_ADD_WORD && (iWord || bIsStopWord) )
				{
					m_bQwordsChanged |= bQWord;
					ShrinkSpanHead ( iCpLimit );
					WeightAndSubmit ();
				}
			}
			break;

		case STATE_ADD_WORD:
			m_tSpan.Add ( m_iCurToken, iWord, bIsStopWord, bQWord, iTermIndex, iLengthCP );

			if ( iWord || bIsStopWord )
			{
				m_bQwordsChanged |= bQWord;
				ShrinkSpanHead ( iCpLimit );
				WeightAndSubmit ();
			}
			break;
		}

		m_iCurToken++;
	}

	virtual void OnFinish ()
	{
		switch ( m_eState )
		{
		case STATE_WINDOW_SETUP:
			m_eState = STATE_ADD_WORD;
			m_bQwordsChanged = true;
			break;

		case STATE_ADD_WORD:
			if ( m_dPassages.GetLength() )
			{
				CSphVector<Passage_t> dPassagesToShow;
				if ( SelectBestPassages ( dPassagesToShow ) )
				{
					ARRAY_FOREACH ( i, dPassagesToShow )
					{
						dPassagesToShow[i].m_dBeforeTokens.Reset();
						dPassagesToShow[i].m_dAfterTokens.Reset();
					}

					m_dPassages = dPassagesToShow;
				} else
					m_dPassages.Resize(0);
			}
			break;
		}
	}

	virtual void OnOverlap ( int iStart, int iLen )
	{
		CollectStartText ( iStart, iLen );
		AddSpaces ( iStart, iLen );
	}

	virtual void OnSkipHtml ( int , int )	{}

	virtual void OnSPZ ( BYTE , DWORD , char * )
	{
		switch ( m_eState )
		{
		case STATE_WINDOW_SETUP:
			{
				WeightAndSubmit ();
				m_eState = STATE_ADD_WORD;
				break;
			}
			break;

		case STATE_ADD_WORD:
			m_bQwordsChanged = true;
			WeightAndSubmit();
			m_bQwordsChanged = true;
			m_tSpan.Reset();
			m_eState = STATE_WINDOW_SETUP;
			break;
		}
	}

	virtual void OnTail ( int iStart, int iLen )
	{
		const int iCpLimit = m_iLimit ? m_iLimit : INT_MAX;
		CollectStartText ( iStart, iLen );
		AddSpaces ( iStart, iLen );
		ShrinkSpanHead ( iCpLimit );
		WeightAndSubmit ();
	}

	virtual const CSphVector<int> * GetHitlist ( const XQKeyword_t & ) const
	{
		return NULL;
	}

private:
	enum State_e
	{
		STATE_WINDOW_SETUP,
		STATE_ADD_WORD
	};

	TokenSpan_t			m_tSpan;
	Passage_t			m_tPass;
	State_e				m_eState;
	bool				m_bQwordsChanged;
	int					m_iCurToken;
	DWORD				m_uFoundWords;
	bool				m_bCollectionStopped;

	CSphVector<Space_t> m_dSpaces;

	void CollectStartText ( int iStart, int iLen )
	{
		if ( m_bAllowEmpty || m_bCollectionStopped )
			return;

		int iLengthCP = m_pTokenizer->IsUtf8() ? sphUTF8Len ( m_pDoc+iStart, iLen ) : iLen;
		bool bLengthOk = m_dStartResult.GetLength()+iLengthCP < m_iLimit;
		if ( bLengthOk || !m_iCurToken )
			ResultEmit ( m_dStartResult, m_pDoc+iStart, iLen );

		if ( !bLengthOk )
		{
			ResultEmit ( m_dStartResult, m_sChunkSeparator.cstr(), m_sChunkSeparator.Length() );
			m_bCollectionStopped = true;
		}
	}


	void AddSpaces ( int iStart, int iLen )
	{
		switch ( m_eState )
		{
		case STATE_WINDOW_SETUP:
			{
				if ( !m_tSpan.GetNumTokens() )
				{
					m_iCurToken++;
					return;
				}

				SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, m_pTokenizer->IsUtf8(), iStart, iLen );

				const int iCpLimit = m_iLimit ? m_iLimit : INT_MAX;

				ARRAY_FOREACH ( i, m_dSpaces )
				{
					if ( m_tSpan.m_iCodes + m_dSpaces[i].m_iLengthCP > iCpLimit || m_tSpan.m_iWords>=GetWordsLimit ( *this, m_tSpan.m_iQwords ) )
					{
						WeightAndSubmit ();
						m_eState = STATE_ADD_WORD;
					}

					m_tSpan.Add ( m_iCurToken++, 0, false, false, -1, m_dSpaces[i].m_iLengthCP );
				}
			}
			break;

		case STATE_ADD_WORD:
			SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, m_pTokenizer->IsUtf8(), iStart, iLen );
			ARRAY_FOREACH ( i, m_dSpaces )
				m_tSpan.Add ( m_iCurToken++, 0, false, false, -1, m_dSpaces[i].m_iLengthCP );
			break;
		}
	}

	void WeightAndSubmit ()
	{
		if ( m_tSpan.m_iQwords )
		{
			if ( m_bQwordsChanged )
			{
				CalcPassageWeight ( GetWordsLimit ( *this, m_tSpan.m_iQwords ), 0 );
				m_bQwordsChanged = false;
			} else
				UpdateGaps ( GetWordsLimit ( *this, m_tSpan.m_iQwords ) );

			m_tPass.m_iWords = m_tSpan.m_iWords;
			if ( m_tPass.m_uQwords )
				FlushPassage();
		}
	}

	void CalcPassageWeight ( int iMaxWords, int iWordCountCoeff )
	{
		DWORD uLast = 0;
		int iLCS = 1;

		m_tPass.m_iMaxLCS = 1;
		m_tPass.m_uQwords = 0;
		m_tPass.m_iMinGap = iMaxWords-1;
		m_tPass.m_iStartLimit = INT_MAX;
		m_tPass.m_iEndLimit = INT_MIN;
		m_tPass.m_iAroundBefore = m_tPass.m_iAroundAfter = 0;

		int iWord = -1;
		for ( int i = 0; i < m_tSpan.GetNumTokens(); i++ )
		{
			const SpanToken_t & tTok = m_tSpan.GetToken(i);
			if ( tTok.m_eType!=TOK_WORD )
				continue;
			iWord++;

			// update mask
			m_tPass.m_uQwords |= tTok.m_uQwordMask;

			// update match boundary
			if ( tTok.m_uQwordMask )
			{
				int iTok = m_tSpan.m_iStart+i;
				m_tPass.m_iStartLimit = Min ( m_tPass.m_iStartLimit, iTok );
				m_tPass.m_iEndLimit = Max ( m_tPass.m_iEndLimit, iTok );
			}

			// update LCS
			uLast = tTok.m_uQwordMask & ( uLast<<1 );
			if ( uLast )
			{
				iLCS++;
				m_tPass.m_iMaxLCS = Max ( iLCS, m_tPass.m_iMaxLCS );
			} else
			{
				iLCS = 1;
				uLast = tTok.m_uQwordMask;
			}

			// update min gap
			if ( tTok.m_uQwordMask )
			{
				m_tPass.m_iMinGap = Min ( m_tPass.m_iMinGap, iWord );
				m_tPass.m_iMinGap = Min ( m_tPass.m_iMinGap, m_tSpan.m_iWords-1-iWord );
			}

			m_tPass.m_iAroundBefore += ( m_tPass.m_uQwords==0 );
			m_tPass.m_iAroundAfter = (tTok.m_uQwordMask ? 0 : m_tPass.m_iAroundAfter+1 );
		}
		assert ( m_tPass.m_iMinGap>=0 );
		assert ( m_tSpan.m_iWords==iWord+1 );

		// calc final weight
		m_tPass.m_iQwordsWeight = 0;
		m_tPass.m_iQwordCount = 0;

		DWORD uWords = m_tPass.m_uQwords;
		for ( int iWord=0; uWords; uWords >>= 1, iWord++ )
			if ( uWords & 1 )
			{
				m_tPass.m_iQwordsWeight += m_tContainer.GetTermWeight(iWord);
				m_tPass.m_iQwordCount++;
			}

		m_tPass.m_iMaxLCS *= iMaxWords;
		m_tPass.m_iQwordCount *= iWordCountCoeff;
	}

	void UpdateGaps ( int iMaxWords )
	{
		m_tPass.m_iMinGap = iMaxWords-1;
		m_tPass.m_iAroundBefore = m_tPass.m_iAroundAfter = 0;
		DWORD uQwords = 0;
		int iWord = -1;
		for ( int i = 0; i < m_tSpan.GetNumTokens(); i++ )
		{
			const SpanToken_t & tTok = m_tSpan.GetToken(i);
			if ( tTok.m_eType!=TOK_WORD )
				continue;

			iWord++;
			if ( tTok.m_uQwordMask )
			{
				m_tPass.m_iMinGap = Min ( m_tPass.m_iMinGap, iWord );
				m_tPass.m_iMinGap = Min ( m_tPass.m_iMinGap, m_tSpan.m_iWords-1-iWord );
			}

			uQwords |= tTok.m_uQwordMask;
			m_tPass.m_iAroundBefore += ( uQwords==0 );
			m_tPass.m_iAroundAfter = ( tTok.m_uQwordMask ? 0 : m_tPass.m_iAroundAfter+1 );
		}
		assert ( m_tPass.m_iMinGap>=0 );
	}

	void AppendBeforeAfterTokens ( Passage_t & tPassage, const TokenSpan_t & tSpan )
	{
		// maybe we don't need no extra token info
		if ( m_iLimit==0 || m_iLimit>=m_iDocLen || m_bUseBoundaries )
			return;

		tPassage.m_dBeforeTokens.Resize(0);
		tPassage.m_dAfterTokens.Resize(0);

		tPassage.m_dBeforeTokens.Reserve ( (m_iAround+1)*2 );
		tPassage.m_dAfterTokens.Reserve ( (m_iAround+1)*2 );

		int iBefore = 0;
		for ( int i = tPassage.m_iStartLimit-tSpan.m_iStart-1; i>=0; i-- )
		{
			const SpanToken_t & tTok = tSpan.GetToken(i);
			int iWord = tTok.m_eType==TOK_WORD ? 1 : 0;
			if ( iBefore+iWord<=tPassage.m_iAroundBefore )
			{
				StoredToken_t & tBeforeToken = tPassage.m_dBeforeTokens.Add();
				tBeforeToken.m_eType = tTok.m_eType;
				tBeforeToken.m_iLengthCP = tTok.m_iLengthCP;
				iBefore += iWord;
			} else
				break;
		}

		int iAfter = 0;
		for ( int i = tPassage.m_iEndLimit-tSpan.m_iStart+1; i < tSpan.GetNumTokens(); i++ )
		{
			const SpanToken_t & tTok = tSpan.GetToken(i);
			int iWord = tSpan.GetToken(i).m_eType==TOK_WORD ? 1 : 0;
			if ( iAfter+iWord<=tPassage.m_iAroundAfter )
			{
				StoredToken_t & tAfterToken = tPassage.m_dAfterTokens.Add();
				tAfterToken.m_eType = tTok.m_eType;
				tAfterToken.m_iLengthCP = tTok.m_iLengthCP;
				iAfter += iWord;
			} else
				break;
		}
	}

	void FlushPassage ()
	{
		m_tPass.m_iStart = m_tSpan.m_iStart;
		m_tPass.m_iTokens = m_tSpan.GetNumTokens();
		m_tPass.m_iCodes = m_tSpan.m_iCodes;

		int iBefore = 0;
		while ( m_tPass.m_iAroundBefore>m_iAround )
		{
			assert ( m_tPass.m_iStart<m_tPass.m_iStartLimit );
			const SpanToken_t & tTok = m_tSpan.GetToken ( iBefore );
			assert ( tTok.m_uQwordMask==0 );

			m_tPass.m_iCodes -= tTok.m_iLengthCP;
			m_tPass.m_iAroundBefore -= ( tTok.m_eType==TOK_WORD );
			m_tPass.m_iStart++;
			m_tPass.m_iTokens--;
			iBefore++;
		}

		int iAfter = m_tSpan.GetNumTokens()-1;
		while ( m_tPass.m_iAroundAfter>m_iAround )
		{
			assert ( m_tPass.m_iEndLimit<m_tPass.m_iStart+m_tPass.m_iTokens-1 );
			const SpanToken_t & tTok = m_tSpan.GetToken ( iAfter );
			assert ( tTok.m_uQwordMask==0 );

			m_tPass.m_iCodes -= tTok.m_iLengthCP;
			m_tPass.m_iAroundAfter -= ( tTok.m_eType==TOK_WORD );
			m_tPass.m_iTokens--;
			iAfter--;
		}

		// if it's the very first one, do add
		if ( !m_dPassages.GetLength() )
		{
			Passage_t & tPassage = m_dPassages.Add ();
			tPassage.CopyData ( m_tPass );
			AppendBeforeAfterTokens ( tPassage, m_tSpan );
			return;
		}

		// check if it's new or better
		Passage_t & tLast = m_dPassages.Last();
		if ( ( m_tPass.m_iStartLimit<=tLast.m_iStartLimit && tLast.m_iEndLimit<=m_tPass.m_iEndLimit )
			|| ( tLast.m_iStartLimit<=m_tPass.m_iStartLimit && m_tPass.m_iEndLimit<=tLast.m_iEndLimit ) )
		{
			int iPassPre = m_tPass.m_iStartLimit - m_tPass.m_iStart + 1;
			int iPassPost = m_tPass.m_iStart + m_tPass.m_iTokens - m_tPass.m_iEndLimit + 1;
			float fPassGap = (float)Max ( iPassPre, iPassPost ) / (float)Min ( iPassPre, iPassPost );

			int iLastPre = tLast.m_iStartLimit - tLast.m_iStart + 1;
			int iLastPost = tLast.m_iStart + tLast.m_iTokens - tLast.m_iEndLimit + 1;
			float fLastGap = (float)Max ( iLastPre, iLastPost ) / (float)Min ( iLastPre, iLastPost );

			// centered snippet wins last passage
			if ( tLast.GetWeight()<m_tPass.GetWeight() || ( tLast.GetWeight()==m_tPass.GetWeight() && fPassGap<fLastGap ) )
			{
				tLast.CopyData ( m_tPass );
				AppendBeforeAfterTokens ( tLast, m_tSpan );
			}
		} else
		{
			Passage_t & tPassage = m_dPassages.Add ();
			tPassage.CopyData ( m_tPass );
			AppendBeforeAfterTokens ( tPassage, m_tSpan );
		}
	}

	void ShrinkSpanHead ( int iCpLimit )
	{
		int iTokenStart = 0;

		// drop front tokens until the window fits into both word and CP limits
		while ( iTokenStart<m_tSpan.GetNumTokens() && ( m_tSpan.m_iCodes > iCpLimit || m_tSpan.m_iWords > GetWordsLimit ( *this, m_tSpan.m_iQwords ) ) &&
			m_tSpan.GetNumTokens()-iTokenStart!=1 )
		{
			const SpanToken_t & tTok = m_tSpan.GetToken ( iTokenStart );
			if ( tTok.m_eType==TOK_WORD )
			{
				if ( tTok.m_uQwordMask )
				{
					m_tSpan.m_iQwords--;
					m_bQwordsChanged = true;
				}

				m_tSpan.m_iWords--;
				m_tSpan.m_iCodes -= tTok.m_iLengthCP;
				m_tSpan.m_iStart++;
				iTokenStart++;
			} else
			{
				m_tSpan.m_iCodes -= tTok.m_iLengthCP;
				m_tSpan.m_iStart++;
				iTokenStart++;
			}
		}

		// remove extra tokens
		if ( iTokenStart>=m_tSpan.GetNumTokens() )
			m_tSpan.Reset();
		else if ( iTokenStart>0 )
			m_tSpan.RemoveStartingTokens ( iTokenStart );
	}

	bool SelectBestPassages ( CSphVector<Passage_t> & dShow )
	{
		assert ( m_dPassages.GetLength() );

		// our limits
		int iMaxPassages = m_iLimitPassages
			? Min ( m_dPassages.GetLength(), m_iLimitPassages )
			: m_dPassages.GetLength();
		int iMaxWords = m_iLimitWords ? m_iLimitWords : INT_MAX;
		int iMaxCp = m_iLimit ? m_iLimit : INT_MAX;

		DWORD uWords = 0; // mask of words in dShow so far
		int iTotalCodes = 0;
		int iTotalWords = 0;

		CSphVector<int> dWeights ( m_dPassages.GetLength() );
		ARRAY_FOREACH ( i, m_dPassages )
			dWeights[i] = m_dPassages[i].m_iQwordsWeight;

		// collect enough best passages to show all keywords and max out the limits
		// don't care much if we're going over limits in this loop, it will be tightened below
		bool bAll = false;
		while ( dShow.GetLength() < iMaxPassages )
		{
			// get next best passage
			int iBest = -1;
			ARRAY_FOREACH ( i, m_dPassages )
			{
				if ( m_dPassages[i].m_iCodes && ( iBest==-1 || m_dPassages[iBest] < m_dPassages[i] ) )
					iBest = i;
			}
			if ( iBest<0 )
				break;
			Passage_t & tBest = m_dPassages[iBest];

			// does this passage fit the limits?
			bool bFits = ( iTotalCodes + tBest.m_iCodes<=iMaxCp ) && ( iTotalWords + tBest.m_iWords<=iMaxWords );

			// all words will be shown and we're outta limit
			if ( uWords==m_uFoundWords && !bFits )
			{
				// there might be just enough space to partially display this passage
				if ( ( iTotalCodes + m_tContainer.GetTotalKeywordLen() )<=m_iLimit )
					dShow.Add ( tBest );
				break;
			}

			// save it, despite limits or whatever, we'll tighten everything in the loop below
			dShow.Add ( tBest );
			uWords |= tBest.m_uQwords;
			iTotalWords += tBest.m_iWords;
			iTotalCodes += tBest.m_iCodes;
			tBest.m_iCodes = 0; // no longer needed here, abusing to mark displayed passages

			// we just managed to show all words? do one final re-weighting run
			if ( !bAll && uWords==m_uFoundWords )
			{
				bAll = true;
				ARRAY_FOREACH ( i, m_dPassages )
					m_dPassages[i].m_iQwordsWeight = dWeights[i];
			}

			// if we're already showing all words, re-weighting is not needed any more
			if ( bAll )
				continue;

			// re-weight passages, adjust for new mask of shown words
			ARRAY_FOREACH ( i, m_dPassages )
			{
				if ( !m_dPassages[i].m_iCodes )
					continue;
				DWORD uMask = tBest.m_uQwords;
				for ( int iWord=0; uMask; iWord++, uMask >>= 1 )
					if ( ( uMask & 1 ) && ( m_dPassages[i].m_uQwords & ( 1UL<<iWord ) ) )
						m_dPassages[i].m_iQwordsWeight -= m_tContainer.GetTermWeight(iWord);
				m_dPassages[i].m_uQwords &= ~uWords;
			}
		}

		// if all passages won't fit into the limit, try to trim them a bit
		//
		// this step is skipped when use_boundaries is enabled, because
		// each passage must be a separate sentence (delimited by
		// boundaries) and we don't want to split them
		if ( ( iTotalCodes > iMaxCp || iTotalWords > iMaxWords ) && !m_bUseBoundaries )
		{
			// trim passages
			bool bFirst = true;
			bool bDone = false;
			int iCodes = iTotalCodes;
			while ( !bDone )
			{
				// drop one token from each passage starting from the least relevant
				for ( int i=dShow.GetLength(); i > 0; i-- )
				{
					Passage_t & tPassage = dShow[i-1];
					if ( tPassage.m_dBeforeTokens.GetLength() && ( bFirst || !tPassage.m_dAfterTokens.GetLength() ) )
					{
						// drop first
						const StoredToken_t & tTok = tPassage.m_dBeforeTokens.Pop();
						tPassage.m_iStart++;
						tPassage.m_iTokens--;
						tPassage.m_iCodes -= tTok.m_iLengthCP;
						iTotalCodes -= tTok.m_iLengthCP;
						iTotalWords -= tTok.m_eType==TOK_WORD ? 1 : 0;

					} else if ( tPassage.m_dAfterTokens.GetLength() )
					{
						// drop last
						const StoredToken_t & tTok = tPassage.m_dAfterTokens.Pop();
						tPassage.m_iTokens--;
						tPassage.m_iCodes -= tTok.m_iLengthCP;
						iTotalCodes -= tTok.m_iLengthCP;
						iTotalWords -= tTok.m_eType==TOK_WORD ? 1 : 0;
					}
					if ( iTotalCodes<=iMaxCp && iTotalWords<=iMaxWords )
					{
						bDone = true;
						break;
					}
				}
				if ( iTotalCodes==iCodes )
					break; // couldn't reduce anything
				iCodes = iTotalCodes;
				bFirst = !bFirst;
			}
		}

		// if passages still don't fit start dropping least significant ones, limit is sacred.
		while ( ( iTotalCodes > iMaxCp || iTotalWords > iMaxWords ) && !m_bForceAllWords )
		{
			iTotalCodes -= dShow.Last().m_iCodes;
			iTotalWords -= dShow.Last().m_iWords;
			dShow.RemoveFast ( dShow.GetLength()-1 );
		}

		if ( !dShow.GetLength() )
			return false;

		// sort passages in the document order
		if ( !m_bWeightOrder )
			dShow.Sort ( PassagePositionOrder_fn() );

		return true;
	}
};


/// functor that highlights selected passages
class HighlightPassages_c : public HighlightPlain_c
{
public:
	const SphHitMark_t * m_pHit;
	const SphHitMark_t * m_pHitEnd;
	const CSphVector<Passage_t> & m_dPassages;
	int m_iCurToken;
	int m_iCurPassage;
	int m_iSeparatorLen;
	bool m_bLastWasSeparator;

public:
	HighlightPassages_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict, const ExcerptQuery_t & tQuery, const CSphIndexSettings & tSettingsIndex, const char * sDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, const CSphVector<Passage_t> & dPassages )
		: HighlightPlain_c ( tContainer, pTokenizer, pDict, tQuery, tSettingsIndex, sDoc, iDocLen )
		, m_pHit ( dHits.Begin() )
		, m_pHitEnd ( dHits.Begin()+dHits.GetLength() )
		, m_dPassages ( dPassages )
		, m_iCurToken ( 0 )
		, m_iCurPassage	( -1 )
		, m_iSeparatorLen ( m_sChunkSeparator.Length() )
		, m_bLastWasSeparator ( false )
		, m_iNextPassage ( 0 )
	{}

	virtual void OnToken ( int iStart, int iLen, const BYTE *, DWORD uPosition, const CSphVector<SphWordID_t> & )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pTokenizer->GetBufferEnd() );

		UpdatePassage ();

		if ( m_iCurPassage!=-1 )
		{
			m_bLastWasSeparator = false;
			while ( m_pHit<m_pHitEnd && m_pHit->m_uPosition+m_pHit->m_uSpan<=uPosition )
				m_pHit++;

			if ( m_bExactPhrase )
			{
				// marker folding, emit "before" marker at span start only
				if ( m_pHit<m_pHitEnd && uPosition==m_pHit->m_uPosition && m_iCurToken )
				{
					ResultEmit ( m_sBeforeMatch.cstr(), m_iBeforeLen, m_bHasBeforePassageMacro, m_iPassageId, m_sBeforeMatchPassage.cstr(), m_iBeforePostLen );
					m_iMatchesCount++;
				}

				// emit token itself
				ResultEmit ( m_pDoc+iStart, iLen );

				// marker folding, emit "after" marker at span end only
				if ( m_pHit<m_pHitEnd && uPosition==m_pHit->m_uPosition+m_pHit->m_uSpan-1 )
					ResultEmit ( m_sAfterMatch.cstr(), m_iAfterLen, m_bHasAfterPassageMacro, m_iPassageId++, m_sAfterMatchPassage.cstr(), m_iAfterPostLen );

			} else
			{
				bool bHit = m_pHit<m_pHitEnd && uPosition>=m_pHit->m_uPosition && uPosition<=m_pHit->m_uPosition+m_pHit->m_uSpan-1;
				if ( bHit )
				{
					ResultEmit ( m_sBeforeMatch.cstr(), m_iBeforeLen, m_bHasBeforePassageMacro, m_iPassageId, m_sBeforeMatchPassage.cstr(), m_iBeforePostLen );
					m_iMatchesCount++;
				}

				// emit token itself
				ResultEmit ( m_pDoc+iStart, iLen );

				if ( bHit )
					ResultEmit ( m_sAfterMatch.cstr(), m_iAfterLen, m_bHasAfterPassageMacro, m_iPassageId++, m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
			}
		}

		m_iCurToken++;
	}

	virtual void OnOverlap ( int iStart, int iLen )
	{
		EmitSpaces ( iStart, iLen );
	}

	virtual void OnTail ( int iStart, int iLen )
	{
		EmitSpaces ( iStart, iLen );
	}

private:
	int					m_iNextPassage;
	TokenSpan_t			m_tTmpSpan;
	CSphVector<Space_t>	m_dSpaces;

	void EmitSpaces ( int iStart, int iLen )
	{
		assert ( m_pDoc );
		assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pTokenizer->GetBufferEnd() );

		SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, m_pTokenizer->IsUtf8(), iStart, iLen );
		ARRAY_FOREACH ( i, m_dSpaces )
		{
			UpdatePassage();

			if ( m_iCurPassage!=-1 )
				ResultEmit ( m_pDoc+m_dSpaces[i].m_iStartBytes, m_dSpaces[i].m_iLengthBytes );

			m_iCurToken++;
		}
	}

	void UpdatePassage ()
	{
		if ( m_iCurPassage!=-1 )
		{
			if ( !m_bLastWasSeparator && m_iCurToken==m_dPassages[m_iCurPassage].m_iStart+m_dPassages[m_iCurPassage].m_iTokens )
			{
				ResultEmit ( m_sChunkSeparator.cstr(), m_iSeparatorLen );
				m_bLastWasSeparator = true;
			}
		}

		if ( m_iCurPassage!=-1 && m_iCurToken>m_dPassages[m_iCurPassage].m_iStart+m_dPassages[m_iCurPassage].m_iTokens-1 )
			m_iNextPassage = m_iCurPassage+1;

		if ( m_iCurPassage==-1 || m_iCurToken<m_dPassages[m_iCurPassage].m_iStart || m_iCurToken>m_dPassages[m_iCurPassage].m_iStart+m_dPassages[m_iCurPassage].m_iTokens-1 )
		{
			m_iCurPassage = -1;
			for ( int i = m_iNextPassage; i<m_dPassages.GetLength() && m_iCurPassage==-1; i++ )
				if ( m_iCurToken>=m_dPassages[i].m_iStart && m_iCurToken<=m_dPassages[i].m_iStart+m_dPassages[i].m_iTokens-1 )
					m_iCurPassage = i;
		}

		if ( m_iCurPassage!=-1 )
		{
			if ( !m_bLastWasSeparator && m_iCurToken==m_dPassages[m_iCurPassage].m_iStart && m_iCurToken )
			{
				ResultEmit ( m_sChunkSeparator.cstr(), m_iSeparatorLen );
				m_bLastWasSeparator = true;
			}
		}
	}
};


// make zone name lowercase
static void CopyZoneName ( CSphVector<char> & dName, const char * sZone, int iLen )
{
	dName.Resize ( iLen+1 );
	char * pDst = dName.Begin();
	const char * pEnd = sZone + iLen;
	while ( sZone<pEnd )
		*pDst++ = (char)tolower ( *sZone++ );

	dName[iLen] = '\0';
}


static void AddZone ( const char * pStart, const char * pEnd, int uPosition, TokenFunctorTraits_c & tFunctor
						, CSphVector<int> & dZoneStack, CSphVector<char> & dZoneName
#ifndef NDEBUG
						, const char * pBuf
						, CSphVector<int> & dZonePos
#endif
					)
{
	CSphVector<ZonePacked_t> & dZones = tFunctor.m_dZones;
	SmallStringHash_T<int> & hZones = tFunctor.m_hZones;

	// span's management
	if ( *pStart!='/' )	// open zone
	{
#ifndef NDEBUG
		// zone position in characters
		dZonePos.Add ( pStart-pBuf );
#endif

		// zone stack management
		int iSelf = dZones.GetLength();
		dZoneStack.Add ( iSelf );

		// add zone itself
		int iZoneNameLen = pEnd-pStart-1;
		CopyZoneName ( dZoneName, pStart, iZoneNameLen );

		int iZone = FindAddZone ( dZoneName.Begin(), iZoneNameLen, hZones );
		dZones.Add ( sphPackZone ( uPosition, iSelf, iZone ) );
	} else					// close zone
	{
#ifndef NDEBUG
		// lets check open - close tags match
		assert ( dZoneStack.GetLength() && dZoneStack.Last()<dZones.GetLength() );
		int iOpening = dZonePos [ dZoneStack.Last() ];
		assert ( iOpening<pEnd-pBuf && strncmp ( pBuf+iOpening, pStart+1, pEnd-pStart-2 )==0 );
#endif

		int iZoneNameLen = pEnd-pStart-2;
		CopyZoneName ( dZoneName, pStart+1, iZoneNameLen );

		int iZone = FindAddZone ( dZoneName.Begin(), iZoneNameLen, hZones );
		int iOpen = dZoneStack.Last();
		int iClose = dZones.GetLength();
		uint64_t uOpenPacked = dZones[ iOpen ];
		DWORD uOpenPos = (DWORD)( ( uOpenPacked>>32 ) & UINT32_MASK );
		assert ( iZone==(int)( uOpenPacked & UINT16_MASK ) ); // check for zone's types match;

		dZones[iOpen] = sphPackZone ( uOpenPos, iClose, iZone );
		dZones.Add ( sphPackZone ( uPosition, iOpen, iZone ) );

#ifndef NDEBUG
		// zone position in characters
		dZonePos.Add ( pStart-pBuf );
#endif


		// pop up current zone from zone's stack
		dZoneStack.Resize ( dZoneStack.GetLength()-1 );
	}
}


/// tokenize document using a given functor
static void TokenizeDocument ( TokenFunctorTraits_c & tFunctor, const CSphHTMLStripper * pStripper, DWORD iSPZ )
{
	ISphTokenizer * pTokenizer = tFunctor.m_pTokenizer;
	CSphDict * pDict = tFunctor.m_pDict;

	const char * pStartPtr = pTokenizer->GetBufferPtr ();
	const char * pLastTokenEnd = pStartPtr;
	const char * pBufferEnd = pTokenizer->GetBufferEnd();
	assert ( pStartPtr && pLastTokenEnd );

	bool bRetainHtml = tFunctor.m_sStripMode=="retain";
	int uPosition = 0;
	BYTE * sWord = NULL;
	SphWordID_t iBlendID = 0;
	SphWordID_t iBlendedExactID = 0;
	const char * pBlendedEnd = NULL;

	CSphVector<int> dZoneStack;
	CSphVector<char> dZoneName ( 16+3*SPH_MAX_WORD_LEN );
	BYTE sExactBuf [ 3*SPH_MAX_WORD_LEN+4 ];
	BYTE sNonStemmed [ 3*SPH_MAX_WORD_LEN+3];

	// FIXME!!! replace by query SPZ extraction pass
	if ( !iSPZ && ( bRetainHtml && tFunctor.m_bHighlightQuery ) )
		iSPZ = MAGIC_CODE_ZONE;

#ifndef NDEBUG
	CSphVector<int> dZonePos;
#endif

	const bool bUtf8 = pTokenizer->IsUtf8();
	while ( ( sWord = pTokenizer->GetToken() )!=NULL )
	{
		if ( pTokenizer->TokenIsBlended() )
		{
			if ( pBlendedEnd<pTokenizer->GetTokenEnd() )
			{
				iBlendID = pDict->GetWordID ( sWord );
				iBlendedExactID = 0;
				pBlendedEnd = pTokenizer->GetTokenEnd();

				if ( tFunctor.m_bHighlightQuery && tFunctor.m_bIndexExactWords )
				{
					int iLen = strlen ( (const char *)sWord );
					if ( iLen+2>(int)sizeof(sExactBuf) )
						iLen = (int)sizeof(sExactBuf)-2;
					memcpy ( sExactBuf + 1, sWord, iLen );
					sExactBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
					sExactBuf[iLen+1] = '\0';
					iBlendedExactID = pDict->GetWordIDNonStemmed ( sExactBuf );
				}
			}
			continue;
		}

		uPosition += pTokenizer->GetOvershortCount();

		const char * pTokenStart = pTokenizer->GetTokenStart ();

		if ( pTokenStart>pLastTokenEnd )
		{
			tFunctor.OnOverlap ( pLastTokenEnd-pStartPtr, pTokenStart - pLastTokenEnd );
			pLastTokenEnd = pTokenStart;
		}

		if ( bRetainHtml && *pTokenStart=='<' )
		{
			const CSphHTMLStripper::StripperTag_t * pTag = NULL;
			const BYTE * sZoneName = NULL;
			const char * pEndSPZ = NULL;
			int iZoneNameLen = 0;
			if ( iSPZ && pStripper && pTokenStart+2<pBufferEnd && ( pStripper->IsValidTagStart ( *(pTokenStart+1) ) || pTokenStart[1]=='/') )
			{
				pEndSPZ = (const char *)pStripper->FindTag ( (const BYTE *)pTokenStart+1, &pTag, &sZoneName, &iZoneNameLen );
			}

			// regular HTML markup - keep it
			int iTagEnd = FindTagEnd ( pTokenStart );
			if ( iTagEnd!=-1 )
			{
				assert ( pTokenStart+iTagEnd<pTokenizer->GetBufferEnd() );
				tFunctor.OnSkipHtml ( pTokenStart-pStartPtr, iTagEnd+1 );
				pTokenizer->SetBufferPtr ( pTokenStart+iTagEnd+1 );
				pLastTokenEnd = pTokenStart+iTagEnd+1; // fix it up to prevent adding last chunk on exit
			}

			if ( pTag ) // (!S)PZ fix-up
			{
				pEndSPZ += ( pEndSPZ+1<=pBufferEnd && ( *pEndSPZ )!='\0' ); // skip closing angle bracket, if any

				assert ( pTag->m_bPara || pTag->m_bZone );
				assert ( pTag->m_bPara || pEndSPZ[0]=='\0' || pEndSPZ[-1]=='>' ); // should be at tag's end
				assert ( pEndSPZ && pEndSPZ<=pBufferEnd );

				uPosition++;

				// handle paragraph boundaries
				if ( pTag->m_bPara )
				{
					tFunctor.OnSPZ ( MAGIC_CODE_PARAGRAPH, uPosition, NULL );
				} else if ( pTag->m_bZone ) // handle zones
				{
#ifndef NDEBUG
					AddZone ( pTokenStart+1, pTokenStart+2+iZoneNameLen, uPosition, tFunctor, dZoneStack, dZoneName, pStartPtr, dZonePos );
#else
					AddZone ( pTokenStart+1, pTokenStart+2+iZoneNameLen, uPosition, tFunctor, dZoneStack, dZoneName );
#endif
					tFunctor.OnSPZ ( MAGIC_CODE_ZONE, uPosition, dZoneName.Begin() );
				}
			}

			if ( iTagEnd )
				continue;
		}

		// handle SPZ tokens GE then needed
		// add SENTENCE, PARAGRAPH, ZONE token, do junks and tokenizer and pLastTokenEnd fix up
		// FIXME!!! it heavily depends on such attitude MAGIC_CODE_SENTENCE < MAGIC_CODE_PARAGRAPH < MAGIC_CODE_ZONE
		if ( *sWord==MAGIC_CODE_SENTENCE || *sWord==MAGIC_CODE_PARAGRAPH || *sWord==MAGIC_CODE_ZONE )
		{
			// SPZ token has position and could be last token too
			uPosition += ( iSPZ && *sWord>=iSPZ );

			if ( *sWord==MAGIC_CODE_ZONE )
			{
				const char * pZoneEnd = pTokenizer->GetBufferPtr();
				const char * pZoneStart = pZoneEnd;
				while ( *pZoneEnd && *pZoneEnd!=MAGIC_CODE_ZONE )
					pZoneEnd++;
				pZoneEnd++; // skip zone token too
				pTokenizer->SetBufferPtr ( pZoneEnd );
				pLastTokenEnd = pZoneEnd; // fix it up to prevent adding last chunk on exit

#ifndef NDEBUG
				AddZone ( pZoneStart, pZoneEnd, uPosition, tFunctor, dZoneStack, dZoneName, pStartPtr, dZonePos );
#else
				AddZone ( pZoneStart, pZoneEnd, uPosition, tFunctor, dZoneStack, dZoneName );
#endif
			}

			if ( iSPZ && *sWord>=iSPZ )
			{
				tFunctor.OnSPZ ( *sWord, uPosition, dZoneName.Begin() );
			}
			continue;
		}

		pLastTokenEnd = pTokenizer->GetTokenEnd ();
		int iWordLen = pLastTokenEnd - pTokenStart;

		// build wordids vector
		// (exact form, blended, substrings all yield multiple ids)
		// TODO! only doing exact currently; add everything else (blended/star) here too
		CSphVector<SphWordID_t> dWordids;
		dWordids.Add ( 0 ); // will be fixed up later with "primary" wordid
		if ( tFunctor.m_bHighlightQuery && tFunctor.m_bIndexExactWords )
		{
			int iBytes = iWordLen;
			if ( iBytes+2>(int)sizeof(sExactBuf) )
				iBytes = (int)sizeof(sExactBuf)-2;
			memcpy ( sExactBuf + 1, sWord, iBytes );
			sExactBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			sExactBuf[iBytes+1] = '\0';
			dWordids.Add ( pDict->GetWordIDNonStemmed ( sExactBuf ) );
		}

		int iNonStemmedLen = iWordLen;
		if ( iNonStemmedLen+1>(int)sizeof(sNonStemmed) )
			iNonStemmedLen = sizeof(sNonStemmed)-1;

		memcpy ( sNonStemmed, sWord, iNonStemmedLen );
		sNonStemmed[iNonStemmedLen] = '\0';
		// must be last because it can change (stem) sWord
		SphWordID_t iWord = pDict->GetWordID ( sWord );
		dWordids[0] = iWord;

		// compute position
		if ( pTokenizer->GetBoundary() )
			uPosition += tFunctor.m_iBoundaryStep;

		bool bIsStopWord = false;
		if ( !iWord )
			bIsStopWord = pDict->IsStopWord ( sWord );

		if ( iWord || bIsStopWord )
			uPosition += bIsStopWord ? tFunctor.m_iStopwordStep : 1;

		ExcerptGen_c::Token_t tDocTok;
		tDocTok.m_eType = ( iWord || bIsStopWord ) ? ExcerptGen_c::TOK_WORD : ExcerptGen_c::TOK_SPACE;
		tDocTok.m_uPosition = ( iWord || bIsStopWord ) ? uPosition : 0;
		tDocTok.m_iStart = pTokenStart - pStartPtr;
		tDocTok.m_iLengthBytes = tDocTok.m_iLengthCP = iWordLen;
		if ( bUtf8 && ( iWord || bIsStopWord ) )
			tDocTok.m_iLengthCP = sphUTF8Len ( pTokenStart, tDocTok.m_iLengthBytes );

		if ( !pTokenizer->TokenIsBlendedPart() )
		{
			iBlendID = 0;
			iBlendedExactID = 0;
		} else
		{
			dWordids.Add ( iBlendID );
			if ( iBlendedExactID )
				dWordids.Add ( iBlendedExactID );
		}

		// match & emit
		// star match needs non-stemmed word
		tFunctor.OnToken ( tDocTok.m_iStart, tDocTok.m_iLengthBytes, sNonStemmed, tDocTok.m_uPosition, dWordids );
	}

	// last space if any
	if ( pLastTokenEnd!=pTokenizer->GetBufferEnd() )
		tFunctor.OnTail ( pLastTokenEnd-pStartPtr, pTokenizer->GetBufferEnd() - pLastTokenEnd );

	tFunctor.OnFinish();
}


/// snippets query words for different cases
class SnippetsFastQword_c : public ISphQword
{
public:
	const CSphVector<int> *		m_pHits;
	CSphMatch					m_tMatch;
	DWORD						m_uLastPos;

public:
	explicit SnippetsFastQword_c ( const CSphVector<int> * pHits )
		: m_pHits ( pHits )
		, m_uLastPos ( 0 )
	{}

	virtual ~SnippetsFastQword_c () {}

	void Setup ( DWORD uLastPos )
	{
		m_iDocs = 0;
		m_iHits = 0;
		m_uLastPos = uLastPos;
		if ( m_pHits && m_pHits->GetLength() )
		{
			m_iDocs = 1;
			m_iHits = m_pHits->GetLength();
			m_uMatchHits = 0;
			m_bHasHitlist = true;
		}
	}

	bool HasHits () const
	{
		return m_pHits && m_uMatchHits<(DWORD)m_pHits->GetLength();
	}

	virtual const CSphMatch & GetNextDoc ( DWORD * )
	{
		m_dQwordFields.Set();
		m_tMatch.m_iDocID = !m_tMatch.m_iDocID && HasHits() ? 1 : 0;
		return m_tMatch;
	}

	virtual Hitpos_t GetNextHit ()
	{
		if ( !HasHits() )
			return EMPTY_HIT;

		int iPosition = *( m_pHits->Begin() + m_uMatchHits++ );
		return HITMAN::Create ( 0, iPosition, (m_uLastPos==(DWORD)iPosition) );
	}

	virtual void SeekHitlist ( SphOffset_t ) {}
};


/// snippets query word setup
class SnippetsFastQwordSetup_c : public ISphQwordSetup
{
public:
	const HitCollector_c * m_pHiglighter;

public:
	explicit SnippetsFastQwordSetup_c ( const HitCollector_c * pHiglighter )
	{
		m_pHiglighter = pHiglighter;
	}

	virtual ~SnippetsFastQwordSetup_c () {}

	virtual ISphQword * QwordSpawn ( const XQKeyword_t & tWord ) const
	{
		return new SnippetsFastQword_c ( m_pHiglighter->GetHitlist ( tWord ) );
	}

	virtual bool QwordSetup ( ISphQword * pQword ) const
	{
		SnippetsFastQword_c * pWord = dynamic_cast<SnippetsFastQword_c *> ( pQword );
		assert ( pWord );

		pWord->Setup ( m_pHiglighter->m_tContainer.m_uLastPos );
		return true;
	}
};


inline bool operator < ( const SphHitMark_t & a, const SphHitMark_t & b )
{
	return a.m_uPosition < b.m_uPosition;
}


// with sentence in query we should consider SENTECE, PARAGRAPH, ZONE
// with paragraph in query we should consider PARAGRAPH, ZONE
// with zone in query we should consider ZONE
int ConvertSPZ ( DWORD eSPZ )
{
	if ( eSPZ & SPH_SPZ_SENTENCE )
		return MAGIC_CODE_SENTENCE;
	else if ( eSPZ & SPH_SPZ_PARAGRAPH )
		return MAGIC_CODE_PARAGRAPH;
	else if ( eSPZ & SPH_SPZ_ZONE )
		return MAGIC_CODE_ZONE;
	else
		return 0;
}


static void HighlightFastpath ( const ExcerptQuery_t & tQuerySettings,
	const CSphIndexSettings & tIndexSettings, const XQQuery_t & tExtQuery, DWORD eExtQuerySPZ,
	const char * sDoc, int iDocLen,
	CSphDict * pDict, ISphTokenizer * pTokenizer, const CSphHTMLStripper * pStripper,
	CSphString & sError, ISphTokenizer * pQueryTokenizer, CSphVector<BYTE> & dRes )
{
	ExcerptQuery_t tFixedSettings ( tQuerySettings );

	// exact_phrase emulation
	// bug of words is replaced by query_mode=1 + "query words"
	XQQuery_t tExactPhraseQuery;
	bool bPhraseEmulation = tQuerySettings.m_bExactPhrase;
	if ( bPhraseEmulation )
	{
		if ( !tQuerySettings.m_bHighlightQuery && tQuerySettings.m_sWords.Length() && strchr ( tQuerySettings.m_sWords.cstr(), 0x22 )==NULL )
			tFixedSettings.m_sWords.SetSprintf ( "\"%s\"", tQuerySettings.m_sWords.cstr() );

		tFixedSettings.m_bHighlightQuery = true;

		CSphSchema tSchema;
		if ( !sphParseExtendedQuery ( tExactPhraseQuery, tFixedSettings.m_sWords.cstr(), pQueryTokenizer, &tSchema, pDict, tIndexSettings ) )
		{
			sError = tExactPhraseQuery.m_sParseError;
			return;
		}
		if ( tExactPhraseQuery.m_pRoot )
			tExactPhraseQuery.m_pRoot->ClearFieldMask();
	}

	bool bRetainHtml = ( tFixedSettings.m_sStripMode=="retain" );

	// adjust tokenizer for markup-retaining mode
	if ( bRetainHtml )
		pTokenizer->AddSpecials ( "<" );

	// create query and hit lists container, parse query
	SnippetsDocIndex_c tContainer ( tFixedSettings.m_bHighlightQuery, ( bPhraseEmulation ? tExactPhraseQuery : tExtQuery ) );
	tContainer.ParseQuery ( tFixedSettings.m_sWords.cstr(), pQueryTokenizer, pDict, eExtQuerySPZ );

	// do highlighting
	if ( !tFixedSettings.m_bHighlightQuery )
	{
		// simple bag of words query
		// do just one tokenization pass over the document, matching and highlighting keywords
		HighlightPlain_c tHighlighter ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen );
		TokenizeDocument ( tHighlighter, NULL, 0 );

		if ( !tHighlighter.m_iMatchesCount && tFixedSettings.m_bAllowEmpty )
			tHighlighter.m_dResult.Reset();

		// add trailing zero, and return
		tHighlighter.m_dResult.Add ( 0 );
		dRes.SwapData ( tHighlighter.m_dResult );

	} else
	{
		// query with syntax
		// do two passes over document
		// 1st pass will tokenize document, match keywords, and store positions into docindex
		// 2nd pass will highlight matching positions only (with some matching engine aid)

		// do the 1st pass
		HitCollector_c tHitCollector ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen );
		TokenizeDocument ( tHitCollector, pStripper, ConvertSPZ ( eExtQuerySPZ ) );

		// prepare for the 2nd pass (that is, extract matching hits)
		SnippetZoneChecker_c tZoneChecker ( tHitCollector.m_dZones, tHitCollector.m_hZones, tContainer.m_tQuery.m_dZones );

		SnippetsFastQwordSetup_c tQwordSetup ( &tHitCollector );
		tQwordSetup.m_pDict = pDict;
		tQwordSetup.m_eDocinfo = SPH_DOCINFO_EXTERN;
		tQwordSetup.m_pWarning = &sError;
		tQwordSetup.m_pZoneChecker = &tZoneChecker;

		// got a lot of stack allocated variables (up to 30K)
		// check that query not overflow stack here
		if ( !sphCheckQueryHeight ( tContainer.m_tQuery.m_pRoot, sError ) )
			return;

		CSphScopedPtr<CSphHitMarker> pMarker ( CSphHitMarker::Create ( tContainer.m_tQuery.m_pRoot, tQwordSetup ) );
		if ( !pMarker.Ptr() )
			return;

		CSphVector<SphHitMark_t> dMarked;
		pMarker->Mark ( dMarked );

		// we just collected matching spans into dMarked, but!
		// certain spans might not match all words within the span
		// for instance, (one NEAR/3 two) could return a 5-word span
		// but we do have full matching keywords list in tContainer
		// so let's post-process and break down such spans
		// FIXME! what about phrase spans vs stopwords? they will be split now
		if ( !tQuerySettings.m_bExactPhrase )
			ARRAY_FOREACH ( i, dMarked )
		{
			if ( dMarked[i].m_uSpan==1 )
				continue;

			CSphVector<int> dMatched;
			for ( int j=0; j<(int)dMarked[i].m_uSpan; j++ )
			{
				// OPTIMZE? we can premerge all dochits vectors once
				const int iPos = dMarked[i].m_uPosition + j;
				ARRAY_FOREACH ( k, tContainer.m_dDocHits )
					if ( tContainer.m_dDocHits[k].BinarySearch ( iPos ) )
				{
					dMatched.Add ( iPos );
					break;
				}
			}

			// this is something that must never happen
			// we got a span out of the matching engine that does not match any keywords?!
			assert ( dMatched.GetLength() );
			if ( !dMatched.GetLength() )
			{
				dMarked.RemoveFast ( i-- ); // remove, rescan
				continue;
			}

			// append all matching keywords as 1-long spans
			ARRAY_FOREACH ( j, dMatched )
			{
				SphHitMark_t & tMarked = dMarked.Add();
				tMarked.m_uPosition = dMatched[j];
				tMarked.m_uSpan = 1;
			}

			// this swaps current span with the last 1-long span we added
			// which is by definition okay; so we need not rescan it
			dMarked.RemoveFast ( i );
		}
		dMarked.Uniq();

		// we just exploded spans into actual matching hits
		// now lets fold marked and matched hits back into contiguous spans
		// so that we could highlight such spans instead of every individual word
		SphHitMark_t * pOut = dMarked.Begin(); // last emitted folded token
		SphHitMark_t * pIn = dMarked.Begin() + 1; // next token to process
		SphHitMark_t * pMax = dMarked.Begin() + dMarked.GetLength();
		while ( pIn<pMax )
		{
			if ( pIn->m_uPosition==( pOut->m_uPosition + pOut->m_uSpan ) )
			{
				pOut->m_uSpan += pIn->m_uSpan;
				pIn++;
			} else
			{
				*++pOut = *pIn++;
			}
		}
		if ( dMarked.GetLength()>1 )
			dMarked.Resize ( pOut - dMarked.Begin() + 1 );

		bool bHighlightAll = ( tFixedSettings.m_iLimit==0 || tFixedSettings.m_iLimit>=iDocLen ) && ( tFixedSettings.m_iLimitWords==0 || tFixedSettings.m_iLimitWords>iDocLen/2 );
		if ( bHighlightAll )
		{
			// 2nd pass
			HighlightQuery_c tHighlighter ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen, dMarked );
			TokenizeDocument ( tHighlighter, pStripper, ConvertSPZ ( eExtQuerySPZ ) );

			if ( !tHighlighter.m_iMatchesCount && tFixedSettings.m_bAllowEmpty )
				tHighlighter.m_dResult.Reset();

			dRes.SwapData ( tHighlighter.m_dResult );

		} else
		{
			// 2nd pass: collect excerpts
			ExtractExcerpts_c tExtractor ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen, dMarked );
			TokenizeDocument ( tExtractor, pStripper, ConvertSPZ ( eExtQuerySPZ ) );

			if ( tExtractor.m_dPassages.GetLength() )
			{
				// 3rd pass: highlight passages
				HighlightPassages_c tHighlighter ( tContainer, pTokenizer, pDict, tFixedSettings, tIndexSettings, sDoc, iDocLen, dMarked, tExtractor.m_dPassages );
				TokenizeDocument ( tHighlighter, pStripper, ConvertSPZ ( eExtQuerySPZ ) );

				dRes.SwapData ( tHighlighter.m_dResult );

			} else if ( !tFixedSettings.m_bAllowEmpty )
				dRes.SwapData ( tExtractor.m_dStartResult );
		}

		dRes.Add(0);
	}
}

/////////////////////////////////////////////////////////////////////////////

ExcerptQuery_t::ExcerptQuery_t ()
	: m_sBeforeMatch ( "<b>" )
	, m_sAfterMatch ( "</b>" )
	, m_sChunkSeparator ( " ... " )
	, m_sStripMode ( "index" )
	, m_iLimit ( 256 )
	, m_iLimitWords ( 0 )
	, m_iLimitPassages ( 0 )
	, m_iAround ( 5 )
	, m_iPassageId ( 1 )
	, m_bRemoveSpaces ( false )
	, m_bExactPhrase ( false )
	, m_bUseBoundaries ( false )
	, m_bWeightOrder ( false )
	, m_bHighlightQuery ( false )
	, m_bForceAllWords ( false )
	, m_iLoadFiles ( 0 )
	, m_bAllowEmpty ( false )
	, m_bEmitZones ( false )
	, m_iRawFlags ( -1 )
	, m_sFilePrefix ( "" )
	, m_iSize ( 0 )
	, m_iSeq ( 0 )
	, m_iNext ( -2 )
	, m_bHasBeforePassageMacro ( false )
	, m_bHasAfterPassageMacro ( false )
	, m_ePassageSPZ ( SPH_SPZ_NONE )
{
}

/////////////////////////////////////////////////////////////////////////////


void sphBuildExcerpt ( ExcerptQuery_t & tOptions, const CSphIndex * pIndex, const CSphHTMLStripper * pStripper, const XQQuery_t & tExtQuery,
						DWORD eExtQuerySPZ, CSphString & sError, CSphDict * pDict, ISphTokenizer * pDocTokenizer, ISphTokenizer * pQueryTokenizer )
{
	if ( tOptions.m_sStripMode=="retain"
		&& !( tOptions.m_iLimit==0 && tOptions.m_iLimitPassages==0 && tOptions.m_iLimitWords==0 ) )
	{
		sError = "html_strip_mode=retain requires that all limits are zero";
		return;
	}

	char * pData = const_cast<char*> ( tOptions.m_sSource.cstr() );
	CSphFixedVector<char> pBuffer ( 0 );
	int iDataLen = 0;

	if ( tOptions.m_iLoadFiles )
	{
		CSphAutofile tFile;
		if ( tOptions.m_sFilePrefix!="" )
		{
			CSphString sFilename;
			sFilename.SetSprintf ( "%s%s", tOptions.m_sFilePrefix.cstr(), tOptions.m_sSource.cstr() );
			if ( tFile.Open ( sFilename.cstr(), SPH_O_READ, sError )<0 )
				return;
		} else if ( tOptions.m_sSource.IsEmpty() )
		{
			sError.SetSprintf ( "snippet file name is empty" );
			return;
		} else if ( tFile.Open ( tOptions.m_sSource.cstr(), SPH_O_READ, sError )<0 )
			return;

		// will this ever trigger? time will tell; email me if it does!
		if ( tFile.GetSize()+1>=(SphOffset_t)INT_MAX )
		{
			sError.SetSprintf ( "%s too big for snippet (over 2 GB)", pData );
			return;
		}

		int iFileSize = (int)tFile.GetSize();
		if ( iFileSize<0 )
			return;

		iDataLen = iFileSize+1;
		pBuffer.Reset ( iDataLen );
		if ( !tFile.Read ( pBuffer.Begin(), iFileSize, sError ) )
			return;

		pBuffer[iFileSize] = 0;
		pData = pBuffer.Begin();
	}

	// strip if we have to
	if ( pStripper && ( tOptions.m_sStripMode=="strip" || tOptions.m_sStripMode=="index" ) )
		pStripper->Strip ( (BYTE*)pData );

	if ( tOptions.m_sStripMode!="retain" )
		pStripper = NULL;

	// FIXME!!! check on real data (~100 Mb) as stripper changes len
	iDataLen = strlen ( pData );

	bool bCanFastPathed = ( !tOptions.m_bUseBoundaries && !tOptions.m_bWeightOrder && tOptions.m_ePassageSPZ==SPH_SPZ_NONE && !tOptions.m_bEmitZones );

	// this is bag-of-words legacy highlighting
	if ( ( ( tOptions.m_iLimit && tOptions.m_iLimit<iDataLen ) || tOptions.m_iLimitWords || tOptions.m_bForceAllWords ) && !tOptions.m_bHighlightQuery )
		bCanFastPathed = false;

	// the fast path
	if ( bCanFastPathed )
	{
		HighlightFastpath ( tOptions, pIndex->GetSettings(), tExtQuery, eExtQuerySPZ, pData, iDataLen, pDict, pDocTokenizer, pStripper,
			sError, pQueryTokenizer, tOptions.m_dRes );
		return;
	}

	if ( !tOptions.m_bHighlightQuery )
	{
		// legacy highlighting
		ExcerptGen_c tGenerator ( pDocTokenizer->IsUtf8() );
		tGenerator.TokenizeQuery ( tOptions, pDict, pDocTokenizer, pIndex->GetSettings() );
		tGenerator.SetExactPhrase ( tOptions );
		tGenerator.TokenizeDocument ( pData, iDataLen, pDict, pDocTokenizer, true, tOptions,
			pIndex->GetSettings(), ConvertSPZ ( tOptions.m_ePassageSPZ | eExtQuerySPZ ) );
		tGenerator.BuildExcerpt ( tOptions );
		return;
	}

	ExcerptGen_c tGenerator ( pDocTokenizer->IsUtf8() );
	tGenerator.TokenizeDocument ( pData, iDataLen, pDict, pDocTokenizer, false, tOptions,
		pIndex->GetSettings(), ConvertSPZ ( tOptions.m_ePassageSPZ | eExtQuerySPZ ) );

	CSphScopedPtr<SnippetZoneChecker_c> pZoneChecker ( new SnippetZoneChecker_c ( tGenerator.GetZones(),
		tGenerator.GetZonesName(), tExtQuery.m_dZones ) );
	SnippetsQwordSetup tSetup ( &tGenerator, pDocTokenizer );
	CSphString sWarning;

	tSetup.m_pDict = pDict;
	tSetup.m_pIndex = pIndex;
	tSetup.m_eDocinfo = SPH_DOCINFO_EXTERN;
	tSetup.m_pWarning = &sWarning;
	tSetup.m_pZoneChecker = pZoneChecker.Ptr();

	// check that query not overflow stack
	if ( !sphCheckQueryHeight ( tExtQuery.m_pRoot, sError ) )
		return;

	CSphScopedPtr<CSphHitMarker> pMarker ( CSphHitMarker::Create ( tExtQuery.m_pRoot, tSetup ) );
	if ( !pMarker.Ptr() )
	{
		sError = sWarning;
		return;
	}

	tGenerator.SetMarker ( pMarker.Ptr() );
	tGenerator.BuildExcerpt ( tOptions );
}

//
// $Id$
//
