//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
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

#include <ctype.h>

/////////////////////////////////////////////////////////////////////////////
// THE EXCERPTS GENERATOR
/////////////////////////////////////////////////////////////////////////////

static const int MAX_HIGHLIGHT_WORDS = 256;

class ExcerptGen_c
{
	friend class SnippetsQwordSetup;
	
public:
							ExcerptGen_c ();
							~ExcerptGen_c () {}

	char *	BuildExcerpt ( const ExcerptQuery_t &, CSphDict * pDict, ISphTokenizer * pTokenizer );
	
	void	TokenizeQuery ( const ExcerptQuery_t &, CSphDict * pDict, ISphTokenizer * pTokenizer );
	void	TokenizeDocument ( const ExcerptQuery_t &, CSphDict * pDict, ISphTokenizer * pTokenizer, bool bFillMasks );
	
	void	SetMarker ( CSphHitMarker * pMarker ) { m_pMarker = pMarker; }
	
public:
	enum Token_e
	{
		TOK_NONE = 0,		///< unspecified type, also used as the end marker
		TOK_WORD,			///< just a word
		TOK_SPACE,			///< whitespace chars seq
		TOK_BREAK			///< non-word chars seq which delimit a phrase part or boundary
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
		DWORD				m_uPosition;	///< hit position in document
	};

	struct Passage_t
	{
		int					m_iStart;		///< start token index
		int					m_iTokens;		///< token count
		int					m_iCodes;		///< codepoints count
		DWORD				m_uWords;		///< matching query words mask
		int					m_iWordsWeight;	///< passage weight factor
		int					m_iWordCount;	///< passage weight factor
		int					m_iMaxLCS;		///< passage weight factor
		int					m_iMinGap;		///< passage weight factor

		void Reset ()
		{
			m_iStart = 0;
			m_iTokens = 0;
			m_iCodes = 0;
			m_uWords = 0;
			m_iWordsWeight = 0;
			m_iWordCount = 0;
			m_iMaxLCS = 0;
			m_iMinGap = 0;
		}

		inline int GetWeight () const
		{
			return m_iWordCount + m_iWordsWeight*m_iMaxLCS + m_iMinGap;
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
	};

protected:
	CSphVector<Token_t>		m_dTokens;		///< source text tokens
	CSphVector<Token_t>		m_dWords;		///< query words tokens

	CSphString				m_sBuffer;

	CSphVector<BYTE>		m_dResult;		///< result holder
	int						m_iResultLen;	///< result codepoints count

	CSphVector<Passage_t>	m_dPassages;	///< extracted passages

	bool					m_bExactPhrase;

	int						m_iWordCount; 
	int						m_iLastWord;
	CSphHitMarker *			m_pMarker;

	CSphVector<char>		m_dKeywordsBuffer;
	CSphVector<Keyword_t>	m_dKeywords;

protected:
	void					CalcPassageWeight ( const CSphVector<int> & dPassage, Passage_t & tPass, int iMaxWords, int iWordCountCoeff );
	bool					ExtractPassages ( const ExcerptQuery_t & q );
	bool					ExtractPhrases ( const ExcerptQuery_t & q );

	void					HighlightPhrase ( const ExcerptQuery_t & q, int iStart, int iEnd );
	void					HighlightAll ( const ExcerptQuery_t & q );
	void					HighlightStart ( const ExcerptQuery_t & q );
	bool					HighlightBestPassages ( const ExcerptQuery_t & q );

	void					ResultEmit ( const char * sLine );
	void					ResultEmit ( const Token_t & sTok );

	void					AddJunk ( int iStart, int iLength, int iBoundary );
	void					AddBoundary ();

	void					MarkHits ();
};

// find string sFind in first iLimit characters of sBuffer
static const char * FindString ( const char * sBuffer, const char * sFind, int iLimit )
{
	assert ( iLimit > 0 );
	assert ( sBuffer );
	assert ( sFind );
	
	iLimit++;
	do
	{
		while ( *sBuffer != *sFind )
			if ( !*++sBuffer || !--iLimit ) return NULL;

		int iSubLimit = iLimit;
		const char * sSubFind = sFind;
		const char * sSubBuffer = sBuffer;
		while ( *sSubFind && *sSubBuffer && *sSubFind == *sSubBuffer++ )
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

/// snippets query words for different cases

class ISnippetsQword: public ISphQword
{
public:
	CSphString *							m_sBuffer;
	CSphVector<ExcerptGen_c::Token_t> *		m_dTokens;

	// word information, filled by setup
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
		m_uFields = 0xFFFFFFFFUL;
		if ( m_iChunk++ == 0 )
		{
			if ( GetNextHit() )
			{
				m_tMatch.m_iDocID = 1;
				m_iToken--;
			}
			else
				m_tMatch.m_iDocID = 0;
		}
		else
			m_tMatch.m_iDocID = 0;
		return m_tMatch;
	}
};

template < typename COMPARE > struct SnippetsQword_c: public ISnippetsQword
{
	virtual DWORD GetNextHit ()
	{
		while ( m_iToken < m_dTokens->GetLength() )
		{
			Token_t & tToken = (*m_dTokens)[m_iToken++];
			if ( tToken.m_eType != ExcerptGen_c::TOK_WORD )
				continue;

			const char * sToken =  &m_sBuffer->cstr() [ tToken.m_iStart ];
			if ( (*(COMPARE *)this).Match ( tToken, sToken ) )
			{
				tToken.m_uWords  |= m_uWordMask;
				return HIT_PACK(0, tToken.m_uPosition) | ( m_iToken - 1 == m_iLastIndex ? HIT_FIELD_END : 0 );
			}
		}
		return 0;
	}
};

///

struct SnippetsQword_Exact_c: public SnippetsQword_c<SnippetsQword_Exact_c>
{
	inline bool Match ( const Token_t & tToken, const char * )
	{
		return tToken.m_iWordID == m_iWordID;
	}
};

struct SnippetsQword_StarFront_c: public SnippetsQword_c<SnippetsQword_StarFront_c>
{
	inline bool Match ( const Token_t & tToken, const char * sToken )
	{
		int iOffset = tToken.m_iLengthBytes - m_iWordLength;
		return iOffset >= 0 &&
			memcmp ( m_sDictWord.cstr(), sToken + iOffset, m_iWordLength ) == 0;
	}
};

struct SnippetsQword_StarBack_c: public SnippetsQword_c<SnippetsQword_StarBack_c>
{
	inline bool Match ( const Token_t & tToken, const char * sToken )
	{
		return ( tToken.m_iLengthBytes >= m_iWordLength ) &&
			memcmp( m_sDictWord.cstr(), sToken, m_iWordLength ) == 0;
	}
};

struct SnippetsQword_StarBoth_c: public SnippetsQword_c<SnippetsQword_StarBoth_c>
{
	inline bool Match ( const Token_t & tToken, const char * sToken )
	{
		return FindString ( sToken, m_sDictWord.cstr(), tToken.m_iLengthBytes ) != NULL;
	}
};

/// snippets query word setup

class SnippetsQwordSetup: public ISphQwordSetup
{
	ExcerptGen_c *	m_pGenerator;
	bool			m_bUtf8;

public:
	SnippetsQwordSetup ( ExcerptGen_c * pGenerator, bool bUtf8 )
		: m_pGenerator ( pGenerator )
		, m_bUtf8 ( bUtf8 )
	{}

	virtual ISphQword *		QwordSpawn ( const XQKeyword_t & tWord ) const;
	virtual bool			QwordSetup ( ISphQword * pQword ) const;
};

ISphQword * SnippetsQwordSetup::QwordSpawn ( const XQKeyword_t & tWord ) const
{
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
	pWord->m_uWordMask = 1 << (m_pGenerator->m_iWordCount++);
	pWord->m_iWordLength = strlen ( pWord->m_sDictWord.cstr() );
	pWord->m_dTokens = &(m_pGenerator->m_dTokens);
	pWord->m_sBuffer = &(m_pGenerator->m_sBuffer);
	
	pWord->m_iDocs = 1;
	pWord->m_iHits = 1;
	pWord->m_bHasHitlist = true;

	// add dummy word, used for passage weighting
	const char * sWord = pWord->m_sDictWord.cstr();
	const int iLength = m_bUtf8 ? sphUTF8Len ( sWord ) : strlen ( sWord );
	m_pGenerator->m_dWords.Add().m_iLengthCP = iLength;

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
		return a.m_iCodes > b.m_iCodes;
	return a.GetWeight() > b.GetWeight();
}

ExcerptGen_c::ExcerptGen_c ()
{
	m_iWordCount = 0;
	m_bExactPhrase = false;
	m_pMarker = NULL;
}

void ExcerptGen_c::AddBoundary()
{
	m_dTokens.Resize ( m_dTokens.GetLength () + 1 );
	Token_t & tLast = m_dTokens.Last ();
	tLast.m_eType = TOK_BREAK;
	tLast.m_iStart = 0;
	tLast.m_iLengthBytes = 0;
	tLast.m_iWordID = 0;
	tLast.m_uWords = 0;
	tLast.m_uPosition = 0;
}

void ExcerptGen_c::AddJunk ( int iStart, int iLength, int iBoundary )
{
	int iChunkStart = iStart;
	int iSaved = 0;

	for ( int i = iStart; i < iStart+iLength; i++ )
		if ( sphIsSpace ( m_sBuffer.cstr () [i] ) != sphIsSpace ( m_sBuffer.cstr () [iChunkStart] ) )
		{
			m_dTokens.Resize ( m_dTokens.GetLength () + 1 );
			Token_t & tLast = m_dTokens.Last ();
			tLast.m_eType   = TOK_SPACE;
			tLast.m_iStart	= iChunkStart;
			tLast.m_iLengthBytes = i - iChunkStart;
			tLast.m_iWordID = 0;
			tLast.m_uWords = 0;
			tLast.m_uPosition = 0;

			iChunkStart = i;
			iSaved += tLast.m_iLengthBytes;

			if ( iBoundary != -1 && iSaved > iBoundary - iStart )
			{
				AddBoundary();
				iBoundary = -1;
			}
		}

	m_dTokens.Resize ( m_dTokens.GetLength () + 1 );
	Token_t & tLast = m_dTokens.Last ();
	tLast.m_eType   = TOK_SPACE;
	tLast.m_iStart	= iChunkStart;
	tLast.m_iLengthBytes = iStart + iLength - iChunkStart;
	tLast.m_iWordID = 0;
	tLast.m_uWords = 0;
	tLast.m_uPosition = 0;

	if ( iBoundary != -1 ) AddBoundary();
}


void ExcerptGen_c::TokenizeQuery ( const ExcerptQuery_t & tQuery, CSphDict * pDict, ISphTokenizer * pTokenizer )
{
	const bool bUtf8 = pTokenizer->IsUtf8();

	// tokenize query words
	int iWordsLength = strlen ( tQuery.m_sWords.cstr() );

	m_dKeywords.Reserve ( MAX_HIGHLIGHT_WORDS );

	BYTE * sWord;
	int iKwIndex = 0;

	pTokenizer->SetBuffer ( (BYTE *)tQuery.m_sWords.cstr(), iWordsLength );
	while ( ( sWord = pTokenizer->GetToken() ) != NULL )
	{
		SphWordID_t iWord = pDict->GetWordID ( sWord );
		if ( iWord )
		{
			m_dWords.Resize ( m_dWords.GetLength () + 1 );
			Token_t & tLast = m_dWords.Last ();
			tLast.m_eType = TOK_WORD;
			tLast.m_iWordID = iWord;
			tLast.m_iLengthBytes = strlen ( (const char *)sWord );
			tLast.m_iLengthCP = bUtf8 ? sphUTF8Len ( (const char *)sWord ) : tLast.m_iLengthBytes;

			// store keyword
			m_dKeywords.Resize( m_dKeywords.GetLength() + 1 );
			Keyword_t & kwLast = m_dKeywords.Last();

			// find stars
			bool bStarBack = *pTokenizer->GetTokenEnd() == '*';
			bool bStarFront = ( pTokenizer->GetTokenStart() != pTokenizer->GetBufferPtr() ) &&
				pTokenizer->GetTokenStart()[-1] == '*';
			kwLast.m_uStar = ( bStarFront ? STAR_FRONT : 0 ) | ( bStarBack ? STAR_BACK : 0 );

			// store token
			const int iEndIndex = iKwIndex + tLast.m_iLengthBytes + 1;
			m_dKeywordsBuffer.Resize ( iEndIndex );
			kwLast.m_iWord = iKwIndex;
			strcpy ( &m_dKeywordsBuffer [ iKwIndex ], (const char *)sWord );
			iKwIndex = iEndIndex;

			if ( m_dWords.GetLength() == MAX_HIGHLIGHT_WORDS )
				break;
		}
	}
}

void ExcerptGen_c::TokenizeDocument ( const ExcerptQuery_t & tQuery, CSphDict * pDict, ISphTokenizer * pTokenizer, bool bFillMasks )
{
	m_dTokens.Reserve ( 1024 );
	m_sBuffer = tQuery.m_sSource;

	pTokenizer->SetBuffer ( (BYTE *)tQuery.m_sSource.cstr (), strlen ( tQuery.m_sSource.cstr () ) );

	const char * pStartPtr = pTokenizer->GetBufferPtr ();
	const char * pLastTokenEnd = pStartPtr;

	BYTE * sWord;
	DWORD uPosition = 0; // hit position in document
	while ( ( sWord = pTokenizer->GetToken() ) != NULL )
	{
		const char * pTokenStart = pTokenizer->GetTokenStart ();

		if ( pTokenStart != pStartPtr )
			AddJunk ( pLastTokenEnd - pStartPtr,
					  pTokenStart - pLastTokenEnd,
					  pTokenizer->GetBoundary() ? pTokenizer->GetBoundaryOffset() : -1 );

		SphWordID_t iWord = pDict->GetWordID ( sWord );

		pLastTokenEnd = pTokenizer->GetTokenEnd ();

		if ( pTokenizer->GetBoundary() )
			uPosition += 100; // FIXME: this should be taken from index settings

		m_dTokens.Resize ( m_dTokens.GetLength () + 1 );
		Token_t & tLast = m_dTokens.Last ();
		tLast.m_eType	= iWord ? TOK_WORD : TOK_SPACE;
		tLast.m_uPosition = iWord ? ++uPosition : 0 ;
		tLast.m_iStart  = pTokenStart - pStartPtr;
		tLast.m_iLengthBytes = pLastTokenEnd - pTokenStart;
		tLast.m_iWordID = iWord;
		tLast.m_uWords = 0;

		m_iLastWord = iWord ? m_dTokens.GetLength() - 1 : m_iLastWord;

		// fill word mask
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
					bMatch = iWord == tToken.m_iWordID;
					break;

				case STAR_FRONT:
					iOffset = tLast.m_iLengthBytes - tToken.m_iLengthBytes;
					bMatch = (iOffset >= 0) &&
						( memcmp( sKeyword, sWord + iOffset, tToken.m_iLengthBytes ) == 0 );
					break;

				case STAR_BACK:
					bMatch = ( tLast.m_iLengthBytes >= tToken.m_iLengthBytes ) &&
						( memcmp( sKeyword, sWord, tToken.m_iLengthBytes ) == 0 );
					break;

				case STAR_BOTH:
					bMatch = strstr( (const char *)sWord, sKeyword ) != NULL;
					break;
				}

				if ( bMatch )
					tLast.m_uWords |= (1UL << nWord);
			}
		}
	}

	// last space if any
	if ( pLastTokenEnd != pTokenizer->GetBufferEnd () )
	{
		int iOffset = pTokenizer->GetBoundary() ? pTokenizer->GetBoundaryOffset() : -1;
		AddJunk ( pLastTokenEnd - pStartPtr, pTokenizer->GetBufferEnd () - pLastTokenEnd, iOffset );
	}

	m_dTokens.Resize ( m_dTokens.GetLength () + 1 );
	Token_t & tLast = m_dTokens.Last ();
	tLast.m_eType   = TOK_NONE;
	tLast.m_iStart  = 0;
	tLast.m_iLengthBytes = 0;
	tLast.m_iWordID = 0;
	tLast.m_uWords = 0;
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

		if ( k == iMarked ) // no more marked hits, clear tail
		{
			for ( ; i < iTokens; i++ )
				m_dTokens[i].m_uWords = 0;
			break;
		}

		// clear false matches
		while ( dMarked[k].m_uPosition > m_dTokens[i].m_uPosition )
			m_dTokens[i++].m_uWords = 0;
		
		// skip tokens covered by hit's span
		assert ( dMarked[k].m_uPosition == m_dTokens[i].m_uPosition );
		assert ( dMarked[k].m_uSpan >= 1 );
		while ( dMarked[k].m_uSpan-- )
		{
			i++; 
			while ( i < iTokens && !m_dTokens[i].m_uPosition ) i++;
		}
	}
}

char * ExcerptGen_c::BuildExcerpt ( const ExcerptQuery_t & tQuery, CSphDict *, ISphTokenizer * pTokenizer )
{
	if ( tQuery.m_bHighlightQuery )
		MarkHits();

	// sum token lengths
	const bool bUtf8 = pTokenizer->IsUtf8();
	int iSourceCodes = 0;
	ARRAY_FOREACH ( i, m_dTokens )
	{
		m_dTokens [i].m_iWeight = 0;

		if ( m_dTokens [i].m_iLengthBytes )
		{
			if ( bUtf8 )
			{
				int iLen = sphUTF8Len ( m_sBuffer.SubString ( m_dTokens[i].m_iStart, m_dTokens[i].m_iLengthBytes ).cstr() );
				m_dTokens[i].m_iLengthCP = iLen;
			}
			else
				m_dTokens[i].m_iLengthCP = m_dTokens[i].m_iLengthBytes;
			iSourceCodes += m_dTokens[i].m_iLengthCP;
		}
		else
			m_dTokens [i].m_iLengthCP = 0;
	}

	m_bExactPhrase = tQuery.m_bExactPhrase && ( m_dWords.GetLength()>1 );

	// assign word weights
	ARRAY_FOREACH ( i, m_dWords )
		m_dWords[i].m_iWeight = m_dWords[i].m_iLengthCP; // FIXME! should obtain freqs from dict

	// reset result
	m_dResult.Reserve ( 16384 );
	m_dResult.Resize ( 0 );
	m_iResultLen = 0;

	// do highlighting
	if ( tQuery.m_iLimit<=0 || tQuery.m_iLimit>iSourceCodes )
	{
		HighlightAll ( tQuery );

	} else
	{
		if ( !( ExtractPassages ( tQuery ) && HighlightBestPassages ( tQuery ) ) )
			HighlightStart ( tQuery );
	}

	// alloc, fill and return the result
	m_dResult.Add ( 0 );
	char * pRes = new char [ m_dResult.GetLength() ];
	memcpy ( pRes, &m_dResult[0], m_dResult.GetLength() );
	m_dResult.Reset ();

	return pRes;
}


void ExcerptGen_c::HighlightPhrase ( const ExcerptQuery_t & q, int iTok, int iEnd )
{
	while ( iTok<=iEnd )
	{
		while ( iTok<=iEnd && !m_dTokens[iTok].m_uWords )
			ResultEmit ( m_dTokens[iTok++] );

		if ( iTok>iEnd )
			break;

		bool bMatch = true;
		int iWord = 0;
		int iStart = iTok;
		while ( iWord<m_dWords.GetLength() )
		{
			if ( ( iTok > iEnd ) ||
				 !( m_dTokens[iTok].m_eType==TOK_SPACE || m_dTokens[iTok].m_uWords == ( 1UL<<iWord++ ) ) )
			{
				bMatch = false;
				break;
			}
			iTok++;
		}

		if ( !bMatch )
		{
			ResultEmit ( m_dTokens[iStart] );
			iTok = iStart + 1;
			continue;
		}

		ResultEmit ( q.m_sBeforeMatch.cstr() );
		while ( iStart<iTok )
			ResultEmit ( m_dTokens [ iStart++ ] );
		ResultEmit ( q.m_sAfterMatch.cstr() );
	}
}


void ExcerptGen_c::HighlightAll ( const ExcerptQuery_t & q )
{
	bool bOpen = false;
	const int iMaxTok = m_dTokens.GetLength()-1; // skip last one, it's TOK_NONE

	if ( m_bExactPhrase )
		HighlightPhrase ( q, 0, iMaxTok-1 );
	else
	{
		// bag of words
		for ( int iTok=0; iTok<iMaxTok; iTok++ )
		{
			if ( ( m_dTokens[iTok].m_uWords!=0 ) ^ bOpen )
			{
				ResultEmit ( bOpen ? q.m_sAfterMatch.cstr() : q.m_sBeforeMatch.cstr() );
				bOpen = !bOpen;
			}
			ResultEmit ( m_dTokens[iTok] );
		}
		if ( bOpen )
			ResultEmit ( q.m_sAfterMatch.cstr() );
	}
}


void ExcerptGen_c::HighlightStart ( const ExcerptQuery_t & q )
{
	// no matches found. just show the starting tokens
	int i = 0;
	while ( m_iResultLen+m_dTokens[i].m_iLengthCP < q.m_iLimit )
	{
		ResultEmit ( m_dTokens[i++] );
		if ( i>=m_dTokens.GetLength() )
			break;
	}
	ResultEmit ( q.m_sChunkSeparator.cstr() );
}

void ExcerptGen_c::ResultEmit ( const char * sLine )
{
	while ( *sLine )
	{
		assert ( (*(BYTE*)sLine)<128 );
		m_dResult.Add ( *sLine++ );
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

void ExcerptGen_c::CalcPassageWeight ( const CSphVector<int> & dPassage, Passage_t & tPass, int iMaxWords, int iWordCountCoeff )
{
	DWORD uLast = 0;
	int iLCS = 1;
	tPass.m_iMaxLCS = 1;

	// calc everything
	tPass.m_uWords = 0;
	tPass.m_iMinGap = iMaxWords-1;

	ARRAY_FOREACH ( i, dPassage )
	{
		Token_t & tTok = m_dTokens[dPassage[i]];
		assert ( tTok.m_eType==TOK_WORD );

		// update mask
		tPass.m_uWords |= tTok.m_uWords;

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
			tPass.m_iMinGap = Min ( tPass.m_iMinGap, i );
			tPass.m_iMinGap = Min ( tPass.m_iMinGap, dPassage.GetLength()-1-i );
		}
	}
	assert ( tPass.m_iMinGap>=0 );

	// calc final weight
	tPass.m_iWordsWeight = 0;
	tPass.m_iWordCount = 0;

	DWORD uWords = tPass.m_uWords;
	for ( int iWord=0; uWords; uWords>>=1, iWord++ )
		if ( uWords & 1 )
	{
		tPass.m_iWordsWeight += m_dWords[iWord].m_iWeight;
		tPass.m_iWordCount++;
	}

	tPass.m_iMaxLCS *= iMaxWords;
	tPass.m_iWordCount *= iWordCountCoeff;
}


bool ExcerptGen_c::ExtractPassages ( const ExcerptQuery_t & q )
{
	m_dPassages.Reserve ( 256 );
	m_dPassages.Resize ( 0 );

	if ( q.m_bUseBoundaries )
		return ExtractPhrases ( q );

	// my current passage
	CSphVector<int> dPass;
	Passage_t tPass;
	tPass.Reset ();

	int iMaxWords = 2*q.m_iAround+1;
	int iLCSThresh = m_bExactPhrase ? m_dWords.GetLength()*iMaxWords : 0;

	// setup initial window
	ARRAY_FOREACH ( i, m_dTokens )
	{
		const Token_t & tToken = m_dTokens[i];

		// skip starting whitespace
		if ( tPass.m_iTokens == 0 && tToken.m_eType != TOK_WORD )
		{
			tPass.m_iStart++;
			continue;
		}

		// stop when the window is large enough
		if ( tPass.m_iCodes + tToken.m_iLengthCP > q.m_iLimit || dPass.GetLength() == iMaxWords )
			break;

		// got token, update passage
		tPass.m_iTokens++;
		tPass.m_iCodes += tToken.m_iLengthCP;

		if ( tToken.m_eType == TOK_WORD )
		{
			dPass.Add(i);
			tPass.m_uWords |= tToken.m_uWords;
		}
	}

	// move our window until the end of document
	const int iCount = m_dTokens.GetLength();
	for ( ;; )
	{
		// re-weight current passage, and check if it matches
		CalcPassageWeight ( dPass, tPass, iMaxWords, 0 );
		if ( tPass.m_uWords && tPass.m_iMaxLCS >= iLCSThresh )
		{
			// if it's the very first one, do add
			if ( !m_dPassages.GetLength() )
				m_dPassages.Add ( tPass );
			else
			{
				// check if it's new or better
				Passage_t & tLast = m_dPassages.Last();
				if ( tLast.m_uWords!=tPass.m_uWords || tLast.m_iStart + tLast.m_iTokens - 1 < tPass.m_iStart )
					m_dPassages.Add ( tPass ); // new
				else if ( tLast.GetWeight() < tPass.GetWeight() )
					tLast = tPass;  // better
			}
		}

		int iToken = tPass.m_iStart + tPass.m_iTokens;
		assert ( iToken <= iCount );
		if ( iToken == iCount ) break;

		// add another word
		for ( ; iToken < iCount; iToken++ )
		{
			tPass.m_iTokens++;
			tPass.m_iCodes += m_dTokens[iToken].m_iLengthCP;
			if ( m_dTokens[iToken].m_eType == TOK_WORD )
			{
				dPass.Add ( iToken );
				break;
			}
		}
		if ( iToken == iCount ) continue;

		// drop front tokens until the window fits into both word and CP limits
		while ( ( tPass.m_iCodes > q.m_iLimit || dPass.GetLength() > iMaxWords ) && tPass.m_iTokens != 1 )
		{
			if ( m_dTokens[tPass.m_iStart].m_eType == TOK_WORD )
				dPass.Remove ( 0 );

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

				CSphVector<int> dPass;
				for ( int i=iStart; i<=iEnd; i++ )
				{
					tPass.m_iCodes += m_dTokens[i].m_iLengthCP;
					if ( m_dTokens[i].m_eType==TOK_WORD )
						dPass.Add ( i );
				}

				CalcPassageWeight ( dPass, tPass, iMaxWords, 10000 );
				if ( tPass.m_iMaxLCS >= iLCSThresh )
					m_dPassages.Add ( tPass );

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
	inline bool operator () ( const ExcerptGen_c::Passage_t & a, const ExcerptGen_c::Passage_t & b ) const
	{
		return a.m_iStart < b.m_iStart;
	}
};


bool ExcerptGen_c::HighlightBestPassages ( const ExcerptQuery_t & q )
{
	///////////////////////////
	// select the ones to show
	///////////////////////////

	CSphVector<Passage_t> dShow;
	int iLeft = q.m_iLimit;

	if ( ( q.m_bUseBoundaries || iLeft>0 ) && m_dPassages.GetLength() )
	{
		// initial heapify
		for ( int i=1; i<m_dPassages.GetLength(); i++ )
		{
			// everything upto i-th is heapified; sift up i-th element
			for ( int j=i; j!=0 && ( m_dPassages[j] < m_dPassages[j>>1] ); j=j>>1 )
				Swap ( m_dPassages[j>>1], m_dPassages[j] );
		}

		// best passage extraction loop
		DWORD uNotShown = 1UL << ( m_dWords.GetLength()-1 );
		while ( m_dPassages.GetLength() )
		{
			// this is our hero
			Passage_t & tPass = m_dPassages[0];

			// emit this passage, if we can
			DWORD uShownWords = 0;
			if ( tPass.m_iCodes<=iLeft || q.m_bUseBoundaries )
			{
				// add it to the show
				dShow.Add ( tPass );
				iLeft -= tPass.m_iCodes;
				uShownWords = tPass.m_uWords;

				// sometimes we need only one best one
				if ( q.m_bSinglePassage )
					break;
			}

			// promote tail, retire head
			m_dPassages.RemoveFast ( 0 );

			// sift down former tail
			int iEntry = 0;
			for ( ;; )
			{
				// select child
				int iChild = (iEntry<<1) + 1;
				if ( iChild>=m_dPassages.GetLength() )
					break;

				// select smallest child
				if ( iChild+1<m_dPassages.GetLength() )
					if ( m_dPassages[iChild+1] < m_dPassages[iChild] )
						iChild++;

				// if smallest child is less than entry, exchange and continue
				if (!( m_dPassages[iChild]<m_dPassages[iEntry] ))
					break;
				Swap ( m_dPassages[iChild], m_dPassages[iEntry] );
				iEntry = iChild;
			}

			// we now show some of the query words,
			// so displaying other passages containing those is less significant,
			// so let's update all the other weights (and word masks, to avoid updating twice)
			// and sift up
			if ( uNotShown )
				for ( int i=0; i<m_dPassages.GetLength(); i++ )
			{
				if ( m_dPassages[i].m_uWords & uShownWords )
				{
					// update this passage
					DWORD uWords = uShownWords;
					for ( int iWord=0; uWords; iWord++, uWords>>=1 )
						if ( ( uWords & 1 ) && ( m_dPassages[i].m_uWords & ( 1UL<<iWord ) ) )
							m_dPassages[i].m_iWordsWeight -= m_dWords[iWord].m_iWeight;

					m_dPassages[i].m_uWords &= ~uShownWords;
					assert ( m_dPassages[i].m_iWordsWeight>=0 );
				}

				// every entry above this is both already updated and properly heapified
				// we only need to sift up, but we need to sift up *every* entry
				// because its parent might had been updated this time, breaking heap property
				for ( int j=i; j!=0 && ( m_dPassages[j] < m_dPassages[j>>1] ); j=j>>1 )
					Swap ( m_dPassages[j>>1], m_dPassages[j] );
			}
			uNotShown &= ~uShownWords;
		}
	}

	if ( !dShow.GetLength() )
		return false;

	///////////
	// do show
	///////////

	// sort the passaged in the document order
	if ( !q.m_bWeightOrder )
		dShow.Sort ( PassageOrder_fn() );

	// estimate length, and grow it up to the limit
	int iLast = -1;
	int iLength = 0;
	ARRAY_FOREACH ( i, dShow )
	{
		int iEnd = dShow[i].m_iStart + dShow[i].m_iTokens - 1;
		for ( int iTok = dShow[i].m_iStart; iTok<=iEnd; iTok++ )
			if ( iTok>iLast )
				iLength += m_dTokens[iTok].m_iLengthCP;
		iLast = iEnd;
	}
	if ( iLength<q.m_iLimit && !q.m_bUseBoundaries )
	{
		// word id is no longer needed; we'll use it to store index into dShow
		ARRAY_FOREACH ( i, m_dTokens )
			m_dTokens[i].m_iWordID = 0;

		ARRAY_FOREACH ( i, dShow )
			for ( int iTok = dShow[i].m_iStart; iTok < dShow[i].m_iStart+dShow[i].m_iTokens; iTok++ )
				if ( m_dTokens[iTok].m_iWordID==0 )
					m_dTokens[iTok].m_iWordID = i;

		int iLeft = q.m_iLimit - iLength;
		int iLastLeft = 0;
		while ( iLeft>0 && iLeft!=iLastLeft )
		{
			iLastLeft = iLeft;
			for ( int iShow=0; iShow<dShow.GetLength() && iLeft>0; iShow++ )
			{
				Passage_t & tPass = dShow [ iShow ];

				// the first one
				int iTok = tPass.m_iStart - 1;
				if ( iTok>=0
					&& m_dTokens[iTok].m_iWordID==0
					&& iLeft>=m_dTokens[iTok].m_iLengthCP )
				{
					iLeft -= m_dTokens [ iTok ].m_iLengthCP;
					m_dTokens [ iTok ].m_iWordID = iShow;
					tPass.m_iStart--;
					tPass.m_iTokens++;
				}

				// the last one
				iTok = tPass.m_iStart + tPass.m_iTokens;
				if ( iTok<m_dTokens.GetLength()
					&& m_dTokens[iTok].m_iWordID==0
					&& iLeft>=m_dTokens[iTok].m_iLengthCP )
				{
					iLeft -= m_dTokens [ iTok ].m_iLengthCP;
					m_dTokens [ iTok ].m_iWordID = iShow;
					tPass.m_iTokens++;
				}
			}
		}
	}

	// show everything
	iLast = -1;
	ARRAY_FOREACH ( i, dShow )
	{
		int iTok = dShow[i].m_iStart;
		int iEnd = iTok + dShow[i].m_iTokens - 1;

		if ( iTok>1+iLast || q.m_bWeightOrder )
			ResultEmit ( q.m_sChunkSeparator.cstr() );

		if ( m_bExactPhrase )
		{
			HighlightPhrase ( q, iTok, iEnd );
		}
		else // !m_bExactPhrase
		{
			while ( iTok<=iEnd )
			{
				if ( iTok>iLast || q.m_bWeightOrder )
				{
					if ( m_dTokens[iTok].m_uWords )
					{
						ResultEmit ( q.m_sBeforeMatch.cstr() );
						ResultEmit ( m_dTokens[iTok] );
						ResultEmit ( q.m_sAfterMatch.cstr() );
					}
					else
						ResultEmit ( m_dTokens[iTok] );
				}
				iTok++;
			}
		}

		iLast = iEnd;
	}
	if ( iLast != m_dTokens.GetLength()-1 )
		ResultEmit ( q.m_sChunkSeparator.cstr() );

	return true;
}

/////////////////////////////////////////////////////////////////////////////

char * sphBuildExcerpt ( ExcerptQuery_t & tOptions, CSphDict * pDict, ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphIndex *pIndex, CSphString & sError )
{
	if ( !tOptions.m_sWords.cstr()[0] )
		tOptions.m_bHighlightQuery = false;
		
	if ( !tOptions.m_bHighlightQuery )
	{
		 // legacy highlighting
		ExcerptGen_c tGenerator;
		tGenerator.TokenizeQuery ( tOptions, pDict, pTokenizer );
		tGenerator.TokenizeDocument ( tOptions, pDict, pTokenizer, true );
		return tGenerator.BuildExcerpt ( tOptions, pDict, pTokenizer );
	}

	XQQuery_t tQuery;
	if ( !sphParseExtendedQuery ( tQuery, tOptions.m_sWords.cstr(), pTokenizer, pSchema, pDict ) )
	{
		sError = tQuery.m_sParseError;
		return NULL;
	}
	tQuery.m_pRoot->ClearFieldMask();
	
	ExcerptGen_c tGenerator;
	SnippetsQwordSetup tSetup ( &tGenerator, pTokenizer->IsUtf8() );
	CSphString sWarning;

	tGenerator.TokenizeDocument ( tOptions, pDict, pTokenizer, false );
	
	tSetup.m_pDict = pDict;
	tSetup.m_pIndex = pIndex;
	tSetup.m_eDocinfo = SPH_DOCINFO_EXTERN;
	tSetup.m_tMin.m_iRowitems = 0;
	tSetup.m_iToCalc = 0;
	tSetup.m_iMaxTimer = 0;
	tSetup.m_pWarning = &sWarning;
	
	CSphScopedPtr<CSphHitMarker> pMarker ( CSphHitMarker::Create ( tQuery.m_pRoot, tSetup ) );
	if ( !pMarker.Ptr() )
	{
		sError = sWarning;
		return NULL;
	}

	tGenerator.SetMarker ( pMarker.Ptr() );
	return tGenerator.BuildExcerpt ( tOptions, pDict, pTokenizer );
}

//
// $Id$
//
