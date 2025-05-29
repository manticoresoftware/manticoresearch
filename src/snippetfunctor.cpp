//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "snippetfunctor.h"

#include "sphinxexcerpt.h"
#include "sphinxsearch.h"

#include "snippetindex.h"
#include "snippetstream.h"
#include "snippetpassage.h"
#include "tokenizer/tokenizer.h"

//////////////////////////////////////////////////////////////////////////

#define UINT32_MASK 0xffffffffUL


struct SpanToken_t
{
	DWORD				m_uQwordMask;		///< query words mask
	int					m_iLengthCP;		///< token length in codepoints
	BYTE				m_iWordFlag;		///< token type
};

struct Space_t
{
	int					m_iStartBytes;		///< offset from doc start
	int					m_iLengthBytes;		///< length in bytes
	int					m_iLengthCP;		///< length in codepoints
};

//////////////////////////////////////////////////////////////////////////

class TokenSpan_c
{
public:
	int		m_iStart;	///< starting token
	int		m_iWords;	///< number of TOK_WORDS tokens
	int		m_iQwords;	///< number of words matching query
	int		m_iCodes;	///< total length in codepoints (cached)

	void				Init ( int nStoredTokens );
	void				Reset();
	void				ScaleUp();
	void				Add ( int iToken, bool bWordFlag, bool bQWord, int iTermIndex, int iLengthCP );
	int					GetNumTokens () const { return m_iNumTokens; }
	const SpanToken_t &	GetToken ( int iToken ) const;
	void				RemoveStartingTokens ( int nTokens );

private:
	int		m_iStartIndex;
	int		m_iNumTokens;
	int		m_iBufferBits;	///< log2(m_dTokens.GetLength())
	DWORD	m_uBufferMask;

	CSphTightVector<SpanToken_t> m_dTokens;
};


void TokenSpan_c::Init ( int nStoredTokens )
{
	m_iBufferBits = sphLog2 ( nStoredTokens-1 );
	DWORD uBufferSize = 1<<m_iBufferBits;
	m_uBufferMask = uBufferSize-1;
	m_dTokens.Resize(uBufferSize);
	Reset ();
}


void TokenSpan_c::Reset()
{
	m_iStart = -1;
	m_iWords = 0;
	m_iQwords = 0;
	m_iCodes = 0;
	m_iStartIndex = 0;
	m_iNumTokens = 0;
}


void TokenSpan_c::ScaleUp()
{
	// not enough preallocated tokens, realloc
	CSphTightVector<SpanToken_t> dNewTokens;
	dNewTokens.Resize ( m_dTokens.GetLength()*2 );

	for ( int i=0; i<m_iNumTokens; i++ )
		dNewTokens[i] = GetToken(i);
	dNewTokens.SwapData ( m_dTokens );

	m_iBufferBits++;
	m_uBufferMask = ( 1<<m_iBufferBits )-1;

	m_iStartIndex = 0;
}


void TokenSpan_c::Add ( int iToken, bool bWordFlag, bool bQWord, int iTermIndex, int iLengthCP )
{
	bQWord &= bWordFlag;

	if ( m_iNumTokens==m_dTokens.GetLength() )
		ScaleUp();

	int iEndIndex = ( m_iStartIndex + m_iNumTokens ) & m_uBufferMask;
	m_iNumTokens++;

	SpanToken_t & tToken = m_dTokens [ iEndIndex ];
	tToken.m_uQwordMask = ( bQWord && ( iTermIndex>=0 ) ) ? ( 1<<iTermIndex ) : 0;
	tToken.m_iLengthCP = iLengthCP;
	tToken.m_iWordFlag = bWordFlag;

	m_iCodes += iLengthCP;
	m_iWords += bWordFlag;
	m_iQwords += bQWord;

	if ( m_iStart<0 )
		m_iStart = iToken;
}


const SpanToken_t & TokenSpan_c::GetToken ( int iToken ) const
{
	assert ( iToken<=m_iNumTokens );
	int iIndex = ( m_iStartIndex + iToken ) & m_uBufferMask;
	return m_dTokens[iIndex];
}


void TokenSpan_c::RemoveStartingTokens ( int nTokens )
{
	assert ( nTokens<m_iNumTokens );
	m_iStartIndex = ( m_iStartIndex + nTokens ) & m_uBufferMask;
	m_iNumTokens -= nTokens;
}

//////////////////////////////////////////////////////////////////////////

static bool IsTokenHit ( const TokenInfo_t & tTok, DWORD uHitPos, int iHitSpan, int iField )
{
	DWORD uTokenPos = HITMAN::Create ( iField, tTok.m_uPosition );
	if ( !tTok.m_iMultiPosLen )
		return ( uTokenPos>=uHitPos && uTokenPos<=uHitPos + iHitSpan );

	// 1d segments intersection
	// token.pos + token.len vs hit.pos + hit.spanLen
	return ( ( (int)(Min ( uTokenPos + tTok.m_iMultiPosLen, uHitPos + iHitSpan )) - (int)(Max ( uTokenPos, uHitPos )) )>=0 );
}


static void SplitSpaceIntoTokens ( CSphVector<Space_t> & dSpaces, const char * pDoc, int iStart, int iLen, int iBoundary = -1 )
{
	// most frequent case
	if ( sphIsSpace ( pDoc[iStart] ) && iLen==1 )
	{
		dSpaces.Resize(1);
		dSpaces[0].m_iStartBytes = iStart;
		dSpaces[0].m_iLengthBytes = 1;
		dSpaces[0].m_iLengthCP = 1;
		return;
	}

	dSpaces.Resize(0);

	int iGapStart = iStart;
	bool bWasSpace = sphIsSpace ( *(pDoc+iStart) );
	bool bWasBoundary = iBoundary==iStart;

	for ( int i=iStart; i<iStart+iLen; i++ )
	{
		bool bSpace = sphIsSpace ( *(pDoc+i) );
		bool bBoundary = i==iBoundary;
		if ( bSpace!=bWasSpace || bBoundary!=bWasBoundary )
		{
			Space_t & tLastSpace = dSpaces.Add();
			tLastSpace.m_iStartBytes = iGapStart;
			tLastSpace.m_iLengthBytes = i-iGapStart;
			tLastSpace.m_iLengthCP = sphUTF8Len ( pDoc+tLastSpace.m_iStartBytes, tLastSpace.m_iLengthBytes );

			bWasSpace = bSpace;
			bWasBoundary = bBoundary;
			iGapStart = i;
		}
	}

	if ( iGapStart < iStart+iLen )
	{
		Space_t & tLastSpace = dSpaces.Add();
		tLastSpace.m_iStartBytes = iGapStart;
		tLastSpace.m_iLengthBytes = iStart+iLen-iGapStart;
		tLastSpace.m_iLengthCP = sphUTF8Len ( pDoc+tLastSpace.m_iStartBytes, tLastSpace.m_iLengthBytes );
	}
}

//////////////////////////////////////////////////////////////////////////
/// document token processor functor traits
class TokenFunctorTraits_c : public TokenFunctor_i
{
protected:
	SnippetResult_t	&				m_tResult;
	const SnippetQuerySettings_t &	m_tQuery;
	const CSphIndexSettings	&		m_tIndexSettings;
	CSphVector<BYTE> &				m_dResult;

	TokenizerRefPtr_c			m_pTokenizer;

	const char *				m_szDocBuffer = nullptr;
	const char *				m_pDoc = nullptr;
	const char *				m_pDocMax = nullptr;

	int							m_iDocLen = 0;
	int							m_iSeparatorLen;
	int							m_iField = 0;

			TokenFunctorTraits_c ( TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, int iField, SnippetResult_t & tRes );

	void	ResultEmit ( CSphVector<BYTE> & dBuf, const char * pSrc, int iLen, bool bHasPassageMacro=false, int iPassageId=0, const char * pPost=nullptr, int iPostLen=0 ) const;
	void	EmitPassageSeparator ( CSphVector<BYTE> & dBuf );
};


TokenFunctorTraits_c::TokenFunctorTraits_c ( TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, int iField, SnippetResult_t & tRes )
	: m_tResult ( tRes )
	, m_tQuery ( tQuery )
	, m_tIndexSettings ( tIndexSettings )
	, m_dResult ( m_tResult.m_dFields[iField].m_dPassages[0].m_dText )
	, m_pTokenizer ( std::move (pTokenizer) )
	, m_szDocBuffer ( szDoc )
	, m_iDocLen ( iDocLen )
	, m_iField ( iField )

{
	assert(m_pTokenizer);
	m_pTokenizer->SetBuffer ( (BYTE*)const_cast<char*>(szDoc), m_iDocLen );
	m_pDoc = m_pTokenizer->GetBufferPtr();
	m_pDocMax = m_pTokenizer->GetBufferEnd();

	m_iSeparatorLen = m_tQuery.m_sChunkSeparator.Length();
}


void TokenFunctorTraits_c::ResultEmit ( CSphVector<BYTE> & dBuf, const char * pSrc, int iLen, bool bHasPassageMacro, int iPassageId, const char * pPost, int iPostLen ) const
{
	dBuf.Append ( pSrc, iLen );

	if ( !bHasPassageMacro )
		return;

	char sBuf[16];
	int iPassLen = snprintf ( sBuf, sizeof(sBuf), "%d", iPassageId );

	dBuf.Append ( sBuf, iPassLen );
	dBuf.Append ( pPost, iPostLen );
}


void TokenFunctorTraits_c::EmitPassageSeparator ( CSphVector<BYTE> & dBuf )
{
	ResultEmit ( dBuf, m_tQuery.m_sChunkSeparator.cstr(), m_iSeparatorLen );
}


//////////////////////////////////////////////////////////////////////////
class HitTraits_c
{
protected:
	const SphHitMark_t * m_pHit = nullptr;
	const SphHitMark_t * m_pHitEnd = nullptr;

			HitTraits_c ( const CSphVector<SphHitMark_t> & dHits );
	void	RewindHits ( DWORD uTokPos, int iField );
};


HitTraits_c::HitTraits_c ( const CSphVector<SphHitMark_t> & dHits )
	: m_pHit ( dHits.Begin() )
	, m_pHitEnd ( dHits.Begin() + dHits.GetLength() )
{}


void HitTraits_c::RewindHits ( DWORD uTokPos, int iField )
{
	while ( m_pHit<m_pHitEnd && m_pHit->m_uPosition+m_pHit->m_uSpan<=HITMAN::Create ( iField, uTokPos ) )
		m_pHit++;
}


//////////////////////////////////////////////////////////////////////////
/// functor that highlights the start of the document
class DocStartHighlighter_c : public TokenFunctorTraits_c
{
public:
			DocStartHighlighter_c ( TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const SnippetLimits_t & tLimits, const CSphIndexSettings & tIndexSettings, const char * szDoc,
				int iDocLen, int iField, int & iResultCP, SnippetResult_t & tRes );

	bool	OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> &, const CSphVector<int> * ) final;
	bool	OnOverlap ( int iStart, int iLen, int ) final;
	void	OnTail ( int iStart, int iLen, int ) final;
	void	OnSkipHtml ( int, int ) final;
	void	OnSPZ ( BYTE, DWORD, const char *, int ) final {}
	void	OnFinish () final {}

private:
	const SnippetLimits_t &	m_tLimits;
	bool					m_bCollectionStopped = false;
	int &					m_iResultLenCP;
	CSphVector<Space_t>		m_dSpaces;

	void	CollectStartTokens ( int iStart, int iLen, int iLenghCP = -1 );
	void	CollectStartSpaces();
};


DocStartHighlighter_c::DocStartHighlighter_c ( TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const SnippetLimits_t & tLimits, const CSphIndexSettings & tIndexSettings, const char * szDoc,
	int iDocLen, int iField, int & iResultCP, SnippetResult_t & tRes )
	: TokenFunctorTraits_c ( std::move ( pTokenizer ), tQuery, tIndexSettings, szDoc, iDocLen, iField, tRes )
	, m_tLimits ( tLimits )
	, m_iResultLenCP ( iResultCP )
{
	assert ( !m_tQuery.m_bAllowEmpty );
}


bool DocStartHighlighter_c::OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> &, const CSphVector<int> * )
{
	CollectStartTokens ( tTok.m_iStart, tTok.m_iLen );
	return !m_bCollectionStopped;
}


bool DocStartHighlighter_c::OnOverlap ( int iStart, int iLen, int )
{
	SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, iStart, iLen );
	CollectStartSpaces();
	return !m_bCollectionStopped;
}


void DocStartHighlighter_c::OnTail ( int iStart, int iLen, int )
{
	SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, iStart, iLen );
	CollectStartSpaces();
}


void DocStartHighlighter_c::OnSkipHtml ( int iStart, int iLen )
{
	assert ( m_pDoc );
	assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
	CollectStartTokens ( iStart, iLen, -1 );
}


void DocStartHighlighter_c::CollectStartTokens ( int iStart, int iLen, int iLenghCP )
{
	if ( m_tQuery.m_bAllowEmpty || m_bCollectionStopped )
		return;

	bool bLengthOk = true;
	int iCalcLengthCP = 0;
	if ( m_tLimits.m_iLimit>0 )
	{
		iCalcLengthCP = iLenghCP;
		if ( iCalcLengthCP==-1 )
			iCalcLengthCP = sphUTF8Len ( m_pDoc+iStart, iLen );

		bLengthOk = m_iResultLenCP+iCalcLengthCP<=m_tLimits.m_iLimit;
	}

	if ( bLengthOk || !m_dResult.GetLength() )
	{
		ResultEmit ( m_dResult, m_pDoc+iStart, iLen );
		m_iResultLenCP += iCalcLengthCP;
	}

	if ( !bLengthOk )
	{
		EmitPassageSeparator ( m_dResult );
		m_bCollectionStopped = true;
	}
}


void DocStartHighlighter_c::CollectStartSpaces()
{
	if ( m_tQuery.m_bAllowEmpty || m_bCollectionStopped )
		return;

	ARRAY_FOREACH_COND ( i, m_dSpaces, !m_bCollectionStopped )
	{
		Space_t & tSpace = m_dSpaces[i];
		CollectStartTokens ( tSpace.m_iStartBytes, tSpace.m_iLengthBytes, tSpace.m_iLengthCP );
	}
}


//////////////////////////////////////////////////////////////////////////
/// functor that extracts passages for further highlighting
class PassageExtractor_c : public TokenFunctorTraits_c, public HitTraits_c
{
public:
				PassageExtractor_c ( const SnippetsDocIndex_c & tContainer, PassageContext_t & tPassageContext, TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const SnippetLimits_t & tLimits,
					const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes );

protected:
	bool		OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> & dTokens, const CSphVector<int> * ) final;
	bool		OnOverlap ( int iStart, int iLen, int iBoundary ) final;
	void		OnSkipHtml ( int iStart, int iLen ) final {}
	void		OnSPZ ( BYTE iSPZ, DWORD uPosition, const char * szZone, int iZone ) final;
	void		OnTail ( int iStart, int iLen, int iBoundary ) final;
	void		OnFinish() final;

private:
	enum State_e
	{
		STATE_WINDOW_SETUP,
		STATE_ADD_WORD
	};

	State_e					m_eState = STATE_WINDOW_SETUP;
	bool					m_bQwordsChanged = true;
	bool					m_bAppendSentenceEnd = false;

	const SnippetsDocIndex_c & m_tContainer;
	SnippetLimits_t			m_tLimits;

	TokenSpan_c				m_tSpan;
	Passage_t				m_tPass;
	int						m_iCurToken = 0;
	CSphVector<Space_t>		m_dSpaces;
	CSphVector<BYTE>		m_dStartResult;

	int						m_iThresh = 0;
	PassageContext_t &		m_tContext;

	void		AddSpaces ( int iBoundary );
	void		WeightAndSubmit();
	void		UpdateGaps ( int iMaxWords );
	void		FlushPassage();
	void		UpdateTopPassages ( int iQword, int iWeight );
	void		ShrinkSpanHead();
	void		CalcPassageWeight ( int iMaxWords );
	void		AppendBeforeAfterTokens ( Passage_t & tPassage, const TokenSpan_c & tSpan );
	int			GetSpanWordsLimit() const;
};


PassageExtractor_c::PassageExtractor_c ( const SnippetsDocIndex_c & tContainer, PassageContext_t & tPassageContext, TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const SnippetLimits_t & tLimits,
	const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes )
	: TokenFunctorTraits_c ( std::move (pTokenizer), tQuery, tIndexSettings, szDoc, iDocLen, iField, tRes )
	, HitTraits_c ( dHits )
	, m_tContainer ( tContainer )
	, m_tLimits ( tLimits )
	, m_tContext ( tPassageContext )
{
	const int AVG_WORD_LEN = 5;
	int iSpanSize = m_tLimits.m_iLimit ? 8*m_tLimits.m_iLimit/AVG_WORD_LEN : 2*m_tLimits.m_iLimitWords;
	if ( !iSpanSize )
		iSpanSize = 128;

	m_tSpan.Init ( iSpanSize );
	m_tPass.Reset();

	if ( m_tLimits.m_iLimitPassages>0 )
		m_iThresh = m_tLimits.m_iLimitPassages;
	else if ( m_tLimits.m_iLimitWords>0 )
		m_iThresh = m_tLimits.m_iLimitWords / 2;
	else if ( m_tLimits.m_iLimit>0 )
		m_iThresh = m_tLimits.m_iLimit / 4;

	m_iThresh = 1 << sphLog2 ( m_iThresh );
}


bool PassageExtractor_c::OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> & dTokens, const CSphVector<int> * )
{
	assert ( m_pDoc );
	assert ( tTok.m_iStart>=0 && m_pDoc+tTok.m_iStart+tTok.m_iLen<=m_pDocMax );

	bool bQWord = false;
	int iTermIndex = -1;

	RewindHits ( tTok.m_uPosition, m_iField );

	if ( m_pHit<m_pHitEnd && IsTokenHit ( tTok, m_pHit->m_uPosition, m_pHit->m_uSpan, m_iField ) )
		bQWord = true;

	iTermIndex = tTok.m_iTermIndex;

	int iLengthCP = sphUTF8Len ( m_pDoc+tTok.m_iStart, tTok.m_iLen );

	switch ( m_eState )
	{
	case STATE_WINDOW_SETUP:
	{
		DWORD uAllQwords = ( 1 << m_tContainer.GetNumTerms() )-1;
		const int iCpLimit = m_tLimits.m_iLimit ? m_tLimits.m_iLimit : INT_MAX;
		bool bLimitsOk = m_tSpan.m_iCodes+iLengthCP<=iCpLimit && m_tSpan.m_iWords<=GetSpanWordsLimit();

		if ( ( m_tQuery.m_bForceAllWords && m_tPass.m_uQwords==uAllQwords && !bLimitsOk ) ||
			( !m_tQuery.m_bForceAllWords && !bLimitsOk ) )
		{
			m_bQwordsChanged = true;
			WeightAndSubmit ();
			m_eState = STATE_ADD_WORD;
		}

		m_tSpan.Add ( m_iCurToken, tTok.m_bWord || tTok.m_bStopWord, bQWord, iTermIndex, iLengthCP );
		m_bQwordsChanged |= bQWord;

		if ( m_eState==STATE_ADD_WORD && (tTok.m_bWord || tTok.m_bStopWord) )
		{
			m_bQwordsChanged |= bQWord;
			ShrinkSpanHead();
			WeightAndSubmit();
		}
	}
	break;

	case STATE_ADD_WORD:
		m_tSpan.Add ( m_iCurToken, tTok.m_bWord || tTok.m_bStopWord, bQWord, iTermIndex, iLengthCP );

		if ( tTok.m_bWord || tTok.m_bStopWord )
		{
			m_bQwordsChanged |= bQWord;
			ShrinkSpanHead();
			WeightAndSubmit();
		}
		break;
	}

	m_iCurToken++;

	return true;
}


void PassageExtractor_c::OnFinish ()
{
	switch ( m_eState )
	{
	case STATE_WINDOW_SETUP:
		WeightAndSubmit();
		break;
	case STATE_ADD_WORD:
		break;
	}

	m_tPass.Reset();
}


bool PassageExtractor_c::OnOverlap ( int iStart, int iLen, int iBoundary )
{
	// most frequent case
	if ( sphIsSpace ( m_pDoc[iStart] ) && iLen==1 && iBoundary<0 && m_eState==STATE_ADD_WORD )
	{
		m_tSpan.Add ( m_iCurToken++, false, false, -1, 1 );
		if ( m_bAppendSentenceEnd )
			OnSPZ ( MAGIC_CODE_SENTENCE, 0, NULL, -1 );
	} else
	{
		SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, iStart, iLen, iBoundary );
		AddSpaces ( iBoundary );
	}

	return true;
}


void PassageExtractor_c::OnSPZ ( BYTE iSPZ, DWORD uPosition, const char * szZone, int iZone )
{
	if ( m_bAppendSentenceEnd )
		m_bAppendSentenceEnd = false;
	else if ( iSPZ==MAGIC_CODE_SENTENCE )
	{
		m_bAppendSentenceEnd = true;
		return;
	}

	switch ( m_eState )
	{
	case STATE_WINDOW_SETUP:
		WeightAndSubmit();
		break;

	case STATE_ADD_WORD:
		m_bQwordsChanged = true;
		WeightAndSubmit();
		m_eState = STATE_WINDOW_SETUP;
		break;
	}

	m_bQwordsChanged = true;
	m_tSpan.Reset();
}

void PassageExtractor_c::OnTail ( int iStart, int iLen, int iBoundary )
{
	SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, iStart, iLen, iBoundary );
	AddSpaces ( iBoundary );
	ShrinkSpanHead();
	WeightAndSubmit();
}


void PassageExtractor_c::AddSpaces ( int iBoundary )
{
	switch ( m_eState )
	{
	case STATE_WINDOW_SETUP:
	{
		DWORD uAllQwords = ( 1 << m_tContainer.GetNumTerms() )-1;
		const int iCpLimit = m_tLimits.m_iLimit ? m_tLimits.m_iLimit : INT_MAX;
		ARRAY_FOREACH ( i, m_dSpaces )
		{
			bool bBoundary = iBoundary==m_dSpaces[i].m_iStartBytes;
			bool bLimitsOk = m_tSpan.m_iCodes+m_dSpaces[i].m_iLengthCP<=iCpLimit && m_tSpan.m_iWords<=GetSpanWordsLimit();

			if ( m_tQuery.m_bForceAllWords && m_tPass.m_uQwords!=uAllQwords )
				bLimitsOk = true;

			if ( ( bBoundary || !bLimitsOk ) && m_eState!=STATE_ADD_WORD )
			{
				if ( bBoundary )
				{
					m_tSpan.Add ( m_iCurToken++, false, false, -1, m_dSpaces[i].m_iLengthCP );
					WeightAndSubmit();
					m_tSpan.Reset();

				} else
				{
					WeightAndSubmit();
					m_eState = STATE_ADD_WORD;
				}
			}

			if ( !bBoundary )
				m_tSpan.Add ( m_iCurToken++, false, false, -1, m_dSpaces[i].m_iLengthCP );

			if ( m_bAppendSentenceEnd )
				OnSPZ ( MAGIC_CODE_SENTENCE, 0, NULL, -1 );
		}
	}
	break;

	case STATE_ADD_WORD:
		ARRAY_FOREACH ( i, m_dSpaces )
		{
			m_tSpan.Add ( m_iCurToken++, false, false, -1, m_dSpaces[i].m_iLengthCP );
			if ( iBoundary==m_dSpaces[i].m_iStartBytes )
			{
				WeightAndSubmit ();
				m_tSpan.Reset();
				m_eState = STATE_WINDOW_SETUP;
			}

			if ( !i && m_bAppendSentenceEnd )
				OnSPZ ( MAGIC_CODE_SENTENCE, 0, NULL, -1 );
		}
		break;
	}
}


void PassageExtractor_c::WeightAndSubmit()
{
	if ( m_tSpan.m_iQwords )
	{
		if ( m_bQwordsChanged )
		{
			CalcPassageWeight ( GetSpanWordsLimit() );
			m_bQwordsChanged = false;
		} else
			UpdateGaps ( GetSpanWordsLimit() );

		if ( m_tPass.m_uQwords )
			FlushPassage();
	}
}


void PassageExtractor_c::UpdateGaps ( int iMaxWords )
{
	m_tPass.m_iMinGap = iMaxWords-1;
	m_tPass.m_iAroundBefore = m_tPass.m_iAroundAfter = 0;
	DWORD uQwords = 0;
	int iWord = -1;
	for ( int i = 0; i < m_tSpan.GetNumTokens(); i++ )
	{
		const SpanToken_t & tTok = m_tSpan.GetToken(i);
		if ( !tTok.m_iWordFlag )
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


void PassageExtractor_c::FlushPassage()
{
	m_tPass.m_iField = m_iField;
	m_tPass.m_iStart = m_tSpan.m_iStart;
	m_tPass.m_iTokens = m_tSpan.GetNumTokens();
	m_tPass.m_iCodes = m_tSpan.m_iCodes;
	m_tPass.m_iWords = m_tSpan.m_iWords;

	int iBefore = 0;
	while ( m_tPass.m_iAroundBefore>m_tQuery.m_iAround )
	{
		assert ( m_tPass.m_iStart<m_tPass.m_iStartLimit );
		const SpanToken_t & tTok = m_tSpan.GetToken ( iBefore );
		assert ( tTok.m_uQwordMask==0 );

		m_tPass.m_iCodes -= tTok.m_iLengthCP;
		m_tPass.m_iAroundBefore -= tTok.m_iWordFlag;
		m_tPass.m_iStart++;
		m_tPass.m_iTokens--;
		m_tPass.m_iWords -= tTok.m_iWordFlag;
		iBefore++;
	}

	int iAfter = m_tSpan.GetNumTokens()-1;
	while ( m_tPass.m_iAroundAfter>m_tQuery.m_iAround )
	{
		assert ( m_tPass.m_iEndLimit<m_tPass.m_iStart+m_tPass.m_iTokens-1 );
		const SpanToken_t & tTok = m_tSpan.GetToken ( iAfter );
		assert ( tTok.m_uQwordMask==0 );

		m_tPass.m_iCodes -= tTok.m_iLengthCP;
		m_tPass.m_iAroundAfter -= tTok.m_iWordFlag;
		m_tPass.m_iTokens--;
		m_tPass.m_iWords -= tTok.m_iWordFlag;
		iAfter--;
	}

	// if it's the very first one, do add
	if ( !m_tContext.m_dPassages.GetLength() )
	{
		Passage_t & tPassage = m_tContext.m_dPassages.Add();
		tPassage.CopyData ( m_tPass );
		AppendBeforeAfterTokens ( tPassage, m_tSpan );
		UpdateTopPassages ( m_tPass.m_iQwordCount==1 ? sphLog2 ( m_tPass.m_uQwords )-1 : -1, m_tPass.GetWeight() );
		return;
	}

	// check if it's new or better than the last one
	Passage_t & tLast = m_tContext.m_dPassages.Last();
	if ( ( m_tPass.m_iStartLimit<=tLast.m_iStartLimit && tLast.m_iEndLimit<=m_tPass.m_iEndLimit )
		|| ( tLast.m_iStartLimit<=m_tPass.m_iStartLimit && m_tPass.m_iEndLimit<=tLast.m_iEndLimit ) )
	{
		// overlapping passages, check which one is better centered
		int iPassPre = m_tPass.m_iStartLimit - m_tPass.m_iStart + 1;
		int iPassPost = m_tPass.m_iStart + m_tPass.m_iTokens - m_tPass.m_iEndLimit + 1;
		float fPassGap = (float)Max ( iPassPre, iPassPost ) / (float)Min ( iPassPre, iPassPost );

		int iLastPre = tLast.m_iStartLimit - tLast.m_iStart + 1;
		int iLastPost = tLast.m_iStart + tLast.m_iTokens - tLast.m_iEndLimit + 1;
		float fLastGap = (float)Max ( iLastPre, iLastPost ) / (float)Min ( iLastPre, iLastPost );

		int iWeightLast = tLast.GetWeight();
		int iWeightPass = m_tPass.GetWeight();
		// centered snippet wins last passage
		if ( tLast.m_iUniqQwords<=m_tPass.m_iUniqQwords &&
			( iWeightLast<iWeightPass || ( iWeightLast==iWeightPass && fPassGap<fLastGap ) ) )
		{
			tLast.CopyData ( m_tPass );
			AppendBeforeAfterTokens ( tLast, m_tSpan );
		}
		return;
	}

	// after a certain threshold, start being picky
	// only accept passages with new keywords, or big enough weight
	int iWeight = m_tPass.GetWeight();
	int iQword = -1;
	if ( m_tPass.m_iQwordCount==1 )
		iQword = sphLog2 ( m_tPass.m_uQwords )-1;

	while ( m_tContext.m_dPassages.GetLength()>m_iThresh )
	{
		// completely new keyword? accept
		if ( m_tPass.m_uQwords & ~m_tContext.m_uPassagesQwords )
			break;

		// single keyword passage? accept if better weight, otherwise reject
		if ( iQword>=0 )
		{
			if ( iWeight<=m_tContext.m_dQwordWeights[iQword] )
				return;
			break;
		}

		// multi-keyword passage? accept if weight within top-N
		assert ( iQword<0 );
		if ( iWeight<=m_tContext.m_dTopPassageWeights[m_iThresh] )
			return;
		break;
	}

	// kill them all, god will know his own
	Passage_t & tPassage = m_tContext.m_dPassages.Add();
	tPassage.CopyData ( m_tPass );
	AppendBeforeAfterTokens ( tPassage, m_tSpan );
	UpdateTopPassages ( iQword, iWeight );
}


void PassageExtractor_c::UpdateTopPassages ( int iQword, int iWeight )
{
	m_tContext.m_uPassagesQwords |= m_tPass.m_uQwords;
	if ( iQword>=0 )
		m_tContext.m_dQwordWeights[iQword] = Max ( m_tContext.m_dQwordWeights[iQword], iWeight );
	m_tContext.m_dTopPassageWeights.Add ( iWeight );
	if ( ( m_tContext.m_dTopPassageWeights.GetLength() & ( m_iThresh-1 ) )==0 )
		m_tContext.m_dTopPassageWeights.RSort();
}


void PassageExtractor_c::ShrinkSpanHead()
{
	const int iCpLimit = m_tLimits.m_iLimit ? m_tLimits.m_iLimit : INT_MAX;

	int iTokenStart = 0;
	const int iMaxToken = m_tSpan.GetNumTokens() - 1;

	// drop front tokens until the window fits into both word and CP limits
	while ( iTokenStart < iMaxToken
		&& ( m_tSpan.m_iCodes > iCpLimit || m_tSpan.m_iWords > GetSpanWordsLimit () ) )
	{
		const SpanToken_t & tTok = m_tSpan.GetToken ( iTokenStart );
		if ( tTok.m_uQwordMask )
		{
			m_tSpan.m_iQwords--; // FIXME? might not be true if we remove a duped keyword
			m_bQwordsChanged = true;
		}
		m_tSpan.m_iWords -= tTok.m_iWordFlag;
		m_tSpan.m_iCodes -= tTok.m_iLengthCP;
		iTokenStart++;
	}
	m_tSpan.m_iStart += iTokenStart;

	// remove extra tokens
	if ( iTokenStart>=m_tSpan.GetNumTokens() )
		m_tSpan.Reset();
	else if ( iTokenStart>0 )
		m_tSpan.RemoveStartingTokens ( iTokenStart );
}


void PassageExtractor_c::CalcPassageWeight ( int iMaxWords )
{
	DWORD uLast = 0;
	int iLCS = 1;

	m_tPass.m_iMaxLCS = 1;
	m_tPass.m_uQwords = 0;
	m_tPass.m_iMinGap = iMaxWords-1;
	m_tPass.m_iStartLimit = INT_MAX;
	m_tPass.m_iEndLimit = INT_MIN;
	m_tPass.m_iAroundBefore = m_tPass.m_iAroundAfter = 0;
	m_tPass.m_iQwordCount = 0;
	m_tPass.m_iUniqQwords = 0;
	m_tPass.m_iQwordsWeight = 0;

	int iWord = -1;
	for ( int i = 0; i < m_tSpan.GetNumTokens(); i++ )
	{
		const SpanToken_t & tTok = m_tSpan.GetToken(i);
		if ( !tTok.m_iWordFlag )
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
			m_tPass.m_iQwordCount++;
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

	// we do it only once because we don't need duplicate weights
	// but m_iQwordCount will still show the total amount of qwords w/dupes
	DWORD uWords = m_tPass.m_uQwords;
	for ( iWord=0; uWords; uWords >>= 1, iWord++ )
		if ( uWords & 1 )
		{
			m_tPass.m_iQwordsWeight += m_tContainer.GetTermWeight(iWord);
			m_tPass.m_iUniqQwords++;
		}

	// total number of words is important too, so lets boost it a bit
	m_tPass.m_iQwordCount *= 2;
}


void PassageExtractor_c::AppendBeforeAfterTokens ( Passage_t & tPassage, const TokenSpan_c & tSpan )
{
	// maybe we don't need no extra token info
	if ( ( ( m_tLimits.m_iLimit==0 || m_tLimits.m_iLimit>=m_iDocLen ) && !m_tLimits.m_iLimitWords && m_tQuery.m_ePassageSPZ==SPH_SPZ_NONE ) || m_tQuery.m_bUseBoundaries )
		return;

	tPassage.m_iCodesBetweenKeywords = tPassage.m_iCodes;
	tPassage.m_iWordsBetweenKeywords = tPassage.m_iWords;

	tPassage.m_dBeforeTokens.Resize(0);
	tPassage.m_dAfterTokens.Resize(0);

	tPassage.m_dBeforeTokens.Reserve ( (m_tQuery.m_iAround+1)*2 );
	tPassage.m_dAfterTokens.Reserve ( (m_tQuery.m_iAround+1)*2 );

	int iBefore = 0;
	for ( int i = tPassage.m_iStartLimit-tSpan.m_iStart-1; i>=tPassage.m_iStart-m_tSpan.m_iStart; i-- )
	{
		const SpanToken_t & tTok = tSpan.GetToken(i);

		tPassage.m_iCodesBetweenKeywords -= tTok.m_iLengthCP;
		tPassage.m_iWordsBetweenKeywords -= tTok.m_iWordFlag;

		if ( iBefore+tTok.m_iWordFlag<=tPassage.m_iAroundBefore )
		{
			StoredExcerptToken_t & tBeforeToken = tPassage.m_dBeforeTokens.Add();
			tBeforeToken.m_iWordFlag = tTok.m_iWordFlag;
			tBeforeToken.m_iLengthCP = tTok.m_iLengthCP;
			iBefore += tTok.m_iWordFlag;
		} else
			break;
	}

	int iAfter = 0;
	for ( int i = tPassage.m_iEndLimit-tSpan.m_iStart+1; i < tPassage.m_iTokens; i++ )
	{
		const SpanToken_t & tTok = tSpan.GetToken(i);
		BYTE iWordFlag = tSpan.GetToken(i).m_iWordFlag;

		tPassage.m_iCodesBetweenKeywords -= tTok.m_iLengthCP;
		tPassage.m_iWordsBetweenKeywords -= iWordFlag;

		if ( iAfter+iWordFlag<=tPassage.m_iAroundAfter )
		{
			StoredExcerptToken_t & tAfterToken = tPassage.m_dAfterTokens.Add();
			tAfterToken.m_iWordFlag = tTok.m_iWordFlag;
			tAfterToken.m_iLengthCP = tTok.m_iLengthCP;
			iAfter += iWordFlag;
		} else
			break;
	}

	assert ( tPassage.m_iWordsBetweenKeywords>0 && tPassage.m_iCodesBetweenKeywords>0 );
}


int PassageExtractor_c::GetSpanWordsLimit() const
{
	return m_tLimits.m_iLimitWords ? m_tLimits.m_iLimitWords : 2*m_tQuery.m_iAround + m_tSpan.m_iQwords;
}

//////////////////////////////////////////////////////////////////////////
class BeforeAfterTraits_c
{
protected:
	int		m_iBeforeLen;
	int		m_iAfterLen;
	int		m_iBeforePostLen;
	int		m_iAfterPostLen;
	int		m_iPassageId;

	BeforeAfterTraits_c ( const SnippetQuerySettings_t & tQuery )
		: m_iBeforeLen ( tQuery.m_sBeforeMatch.Length() )
		, m_iAfterLen ( tQuery.m_sAfterMatch.Length() )
		, m_iBeforePostLen ( tQuery.m_sBeforeMatchPassage.Length() )
		, m_iAfterPostLen ( tQuery.m_sAfterMatchPassage.Length() )
		, m_iPassageId ( tQuery.m_iPassageId )
	{}
};


/// functor that highlights selected passages
class PassageHighlighter_c : public TokenFunctorTraits_c, public BeforeAfterTraits_c, public HitTraits_c
{
public:
			PassageHighlighter_c ( CSphVector<Passage_t*> & dPassages, TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen,
				const CSphVector<SphHitMark_t> & dHits, const FunctorZoneInfo_t & tZoneInfo, int iField, SnippetResult_t & tRes );

protected:
	bool	OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> &, const CSphVector<int> * ) final;
	bool	OnOverlap ( int iStart, int iLen, int iBoundary ) final;
	void	OnSkipHtml ( int iStart, int iLen ) final;
	void	OnSPZ ( BYTE, DWORD, const char *, int ) final {}
	void	OnTail ( int iStart, int iLen, int iBoundary ) final;
	void	OnFinish() final;

private:
	CSphVector<Passage_t*> &		m_dPassages;
	CSphVector<PassageResult_t>	&	m_dPassageText;
	CSphVector<BYTE> *				m_pCurPassageText = nullptr;
	int								m_iCurToken = 0;
	int								m_iCurPassage = -1;
	int								m_iOpenUntilTokenPos = 0;
	int								m_iOpenUntilTokenNum = 0;
	int								m_iLastPos = 0;
	TokenSpan_c						m_tTmpSpan;
	CSphVector<Space_t>				m_dSpaces;
	const FunctorZoneInfo_t &		m_tZoneInfo;

	void	EmitZoneName ( int iStart ) const;
	void	EmitSpaces ( int iStart, int iLen, int iBoundary );
	void	UpdatePassage ( int iStart );
	void	CheckClose ( int iPos );
};


PassageHighlighter_c::PassageHighlighter_c ( CSphVector<Passage_t*> & dPassages, TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen,
	const CSphVector<SphHitMark_t> & dHits, const FunctorZoneInfo_t & tZoneInfo, int iField, SnippetResult_t & tRes )
	: TokenFunctorTraits_c ( std::move (pTokenizer), tQuery, tIndexSettings, szDoc, iDocLen, iField, tRes )
	, BeforeAfterTraits_c(tQuery)
	, HitTraits_c(dHits)
	, m_dPassages ( dPassages )
	, m_dPassageText ( tRes.m_dFields[iField].m_dPassages )
	, m_tZoneInfo ( tZoneInfo )
{
	m_dPassageText.Resize ( dPassages.GetLength() );

	// we don't want these separators in other functors
	for ( auto & i : m_dPassageText )
	{
		i.m_bStartSeparator = true;
		i.m_bEndSeparator = true;
	}
}


bool PassageHighlighter_c::OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> &, const CSphVector<int> * )
{
	assert ( m_pDoc );
	assert ( tTok.m_iStart>=0 && m_pDoc+tTok.m_iStart+tTok.m_iLen<=m_pDocMax );

	CheckClose ( tTok.m_uPosition );
	UpdatePassage ( tTok.m_iStart );

	if ( m_iCurPassage!=-1 )
	{
		assert ( m_pCurPassageText );
		RewindHits ( tTok.m_uPosition, m_iField );

		bool bHit = m_pHit<m_pHitEnd && IsTokenHit ( tTok, m_pHit->m_uPosition, m_pHit->m_uSpan - 1, m_iField );
		if ( bHit && !m_iOpenUntilTokenPos )
		{
			ResultEmit ( *m_pCurPassageText, m_tQuery.m_sBeforeMatch.cstr(), m_iBeforeLen, m_tQuery.m_bHasBeforePassageMacro, m_iPassageId, m_tQuery.m_sBeforeMatchPassage.cstr(), m_iBeforePostLen );
			m_iOpenUntilTokenPos = HITMAN::GetPos(m_pHit->m_uPosition)+m_pHit->m_uSpan;
			m_iOpenUntilTokenNum = m_dPassages[m_iCurPassage]->m_iStart+m_dPassages[m_iCurPassage]->m_iTokens;
		}

		// emit token itself
		ResultEmit ( *m_pCurPassageText, m_pDoc+tTok.m_iStart, tTok.m_iLen );
	}

	m_iLastPos = tTok.m_uPosition;
	m_iCurToken++;
	return true;
}


bool PassageHighlighter_c::OnOverlap ( int iStart, int iLen, int iBoundary )
{
	EmitSpaces ( iStart, iLen, iBoundary );
	return true;
}


void PassageHighlighter_c::OnSkipHtml ( int iStart, int iLen )
{
	assert ( m_pDoc );
	assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
	assert ( m_pCurPassageText );
	ResultEmit ( *m_pCurPassageText, m_pDoc+iStart, iLen );
}


void PassageHighlighter_c::OnTail ( int iStart, int iLen, int iBoundary )
{
	CheckClose ( m_iLastPos+1 );
	EmitSpaces ( iStart, iLen, iBoundary );
}


void PassageHighlighter_c::OnFinish()
{
	if ( m_iOpenUntilTokenPos )
	{
		assert ( m_pCurPassageText );
		ResultEmit ( *m_pCurPassageText, m_tQuery.m_sAfterMatch.cstr(), m_iAfterLen, m_tQuery.m_bHasAfterPassageMacro, m_iPassageId++, m_tQuery.m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
	}

	if ( m_iCurPassage!=-1 )
		m_dPassageText[m_iCurPassage].m_bEndSeparator = false;

	ARRAY_FOREACH ( i, m_dPassages )
		m_dPassageText[i].m_iWeight = m_dPassages[i]->GetWeight();
}


void PassageHighlighter_c::EmitZoneName ( int iStart ) const
{
	if ( !m_tQuery.m_bEmitZones || !m_tZoneInfo.m_dZonePos.GetLength() )
		return;

	int iZone = FindSpan ( m_tZoneInfo.m_dZonePos, iStart );
	if ( iZone!=-1 )
	{
		int iParent = m_tZoneInfo.m_dZoneParent[iZone];
		for ( const auto & tZone : m_tZoneInfo.m_hZones )
		{
			if ( tZone.second!=iParent )
				continue;

			assert ( m_pCurPassageText );
			ResultEmit ( *m_pCurPassageText, "<", 1 );
			ResultEmit ( *m_pCurPassageText, tZone.first.cstr(), tZone.first.Length() );
			ResultEmit ( *m_pCurPassageText, ">", 1 );
			break;
		}
	}
}


void PassageHighlighter_c::EmitSpaces ( int iStart, int iLen, int iBoundary )
{
	assert ( m_pDoc );
	assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );

	SplitSpaceIntoTokens ( m_dSpaces, m_pDoc, iStart, iLen, iBoundary );
	ARRAY_FOREACH ( i, m_dSpaces )
	{
		CheckClose ( m_iLastPos+1 );
		UpdatePassage ( iStart );

		if ( m_iCurPassage!=-1 )
		{
			assert ( m_pCurPassageText );
			ResultEmit ( *m_pCurPassageText, m_pDoc+m_dSpaces[i].m_iStartBytes, m_dSpaces[i].m_iLengthBytes );
		}

		m_iCurToken++;
	}
}


void PassageHighlighter_c::UpdatePassage ( int iStart )
{
	const Passage_t * pPassage = ( m_iCurPassage==-1 ) ? nullptr : *( m_dPassages.Begin ()+m_iCurPassage );

	int iPassage = m_iCurPassage;
	if ( m_iCurPassage==-1 || m_iCurToken<pPassage->m_iStart || m_iCurToken>( pPassage->m_iStart + pPassage->m_iTokens - 1 ) )
	{
		int iNextPassage = 0;
		if ( m_iCurPassage!=-1 && m_iCurToken>( pPassage->m_iStart + pPassage->m_iTokens - 1 ) )
			iNextPassage = m_iCurPassage+1;

		m_iCurPassage = -1;
		for ( int i=iNextPassage; i<m_dPassages.GetLength(); i++ )
			if ( m_iCurToken>=m_dPassages[i]->m_iStart && m_iCurToken<=( m_dPassages[i]->m_iStart + m_dPassages[i]->m_iTokens - 1 ) )
			{
				m_iCurPassage = i;
				break;
			}
	}

	if ( !m_iCurPassage && !m_iCurToken )
		m_dPassageText[m_iCurPassage].m_bStartSeparator = false;
	
	if ( m_iCurPassage!=-1 && iPassage!=m_iCurPassage )
	{
		m_pCurPassageText = &(m_dPassageText[m_iCurPassage].m_dText);
		EmitZoneName ( iStart );
	}
}


void PassageHighlighter_c::CheckClose ( int iPos )
{
	// marker folding, emit "after" marker at span end only
	if ( ( !m_iOpenUntilTokenPos || iPos<m_iOpenUntilTokenPos ) && ( !m_iOpenUntilTokenNum || m_iCurToken<m_iOpenUntilTokenNum ) )
		return;

	assert ( m_pCurPassageText );
	ResultEmit ( *m_pCurPassageText, m_tQuery.m_sAfterMatch.cstr(), m_iAfterLen, m_tQuery.m_bHasAfterPassageMacro, m_iPassageId++, m_tQuery.m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
	m_iOpenUntilTokenPos = m_iOpenUntilTokenNum = 0;
}

//////////////////////////////////////////////////////////////////////////
/// functor that matches tokens against hit positions from mini-index and highlights them
class QueryHighlighter_c : public TokenFunctorTraits_c, public BeforeAfterTraits_c, public HitTraits_c
{
public:
	QueryHighlighter_c ( TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
		const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes );

protected:
	bool	OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> &, const CSphVector<int> * ) final;
	bool	OnOverlap ( int iStart, int iLen, int ) final;
	void	OnSkipHtml ( int iStart, int iLen ) final;
	void	OnSPZ ( BYTE, DWORD, const char *, int ) final {}
	void	OnTail ( int iStart, int iLen, int ) final;
	void	OnFinish() final;

private:
	int		m_iOpenUntilTokenPos = 0; // blend-chars has same positions as blend-part tokens
	int		m_iLastPos = 0;
	int		m_iMatches = 0;

	void	CheckClose ( int iPos );
};


QueryHighlighter_c::QueryHighlighter_c ( TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes )
	: TokenFunctorTraits_c ( std::move (pTokenizer), tQuery, tIndexSettings, szDoc, iDocLen, iField, tRes )
	, BeforeAfterTraits_c(tQuery)
	, HitTraits_c(dHits)
{
	m_dResult.Reserve ( 1024 );
}


bool QueryHighlighter_c::OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> &, const CSphVector<int> * )
{
	assert ( m_pDoc );
	assert ( tTok.m_iStart>=0 && m_pDoc+tTok.m_iStart+tTok.m_iLen<=m_pDocMax );

	RewindHits ( tTok.m_uPosition, m_iField );
	CheckClose ( tTok.m_uPosition );

	// marker folding, emit "before" marker at span start only
	// tmg note: stopwords with step 0 resets m_iOpenUntilTokenPos and breaks highligh of spans of tokens
	if ( m_pHit<m_pHitEnd && IsTokenHit ( tTok, m_pHit->m_uPosition, m_pHit->m_uSpan, m_iField ) && !m_iOpenUntilTokenPos )
	{
		ResultEmit ( m_dResult, m_tQuery.m_sBeforeMatch.cstr(), m_iBeforeLen, m_tQuery.m_bHasBeforePassageMacro, m_iPassageId, m_tQuery.m_sBeforeMatchPassage.cstr(), m_iBeforePostLen );
		m_iMatches++;
		m_iOpenUntilTokenPos = HITMAN::GetPos(m_pHit->m_uPosition)+m_pHit->m_uSpan;
	}

	// emit token itself
	ResultEmit ( m_dResult, m_pDoc+tTok.m_iStart, tTok.m_iLen );
	m_iLastPos = tTok.m_uPosition + Max ( tTok.m_iMultiPosLen-1, 0 );
	return true;
}


void QueryHighlighter_c::OnTail ( int iStart, int iLen, int )
{
	assert ( m_pDoc );
	assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
	CheckClose ( m_iLastPos+1 );
	ResultEmit ( m_dResult, m_pDoc+iStart, iLen );
}


void QueryHighlighter_c::OnFinish()
{
	if ( !m_iMatches && m_tQuery.m_bAllowEmpty )
	{
		m_dResult.Reset();
		return;
	}

	if ( !m_iOpenUntilTokenPos )
		return;

	ResultEmit ( m_dResult, m_tQuery.m_sAfterMatch.cstr(), m_iAfterLen, m_tQuery.m_bHasAfterPassageMacro, m_iPassageId++, m_tQuery.m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
}


bool QueryHighlighter_c::OnOverlap ( int iStart, int iLen, int )
{
	assert ( m_pDoc );
	assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
	CheckClose ( m_iLastPos+1 );
	ResultEmit ( m_dResult, m_pDoc+iStart, iLen );
	return true;
}


void QueryHighlighter_c::OnSkipHtml ( int iStart, int iLen )
{
	assert ( m_pDoc );
	assert ( iStart>=0 && m_pDoc+iStart+iLen<=m_pDocMax );
	CheckClose ( m_iLastPos+1 );
	ResultEmit ( m_dResult, m_pDoc+iStart, iLen );
}


void QueryHighlighter_c::CheckClose ( int iPos )
{
	// marker folding, emit "after" marker at span end only
	if ( !m_iOpenUntilTokenPos || iPos<m_iOpenUntilTokenPos )
		return;

	ResultEmit ( m_dResult, m_tQuery.m_sAfterMatch.cstr(), m_iAfterLen, m_tQuery.m_bHasAfterPassageMacro, m_iPassageId++, m_tQuery.m_sAfterMatchPassage.cstr(), m_iAfterPostLen );
	m_iOpenUntilTokenPos = 0;
}

//////////////////////////////////////////////////////////////////////////
/// functor that processes tokens and collects matching keyword hits into mini-index
class HitCollector_c : public TokenFunctorTraits_c, public virtual HitCollector_i
{
public:
	HitCollector_c ( SnippetsDocIndex_c & tContainer, TokenizerRefPtr_c pTokenizer, DictRefPtr_c pDict, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
		const char * szDoc, int iDocLen, int iField, CacheStreamer_i & tTokenContainer, CSphVector<ZonePacked_t> & dZones, FunctorZoneInfo_t & tZoneInfo, SnippetResult_t & tRes );

protected:
	bool	OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> & dTokens, const CSphVector<int> * pMultiPosDelta ) final;
	bool	OnOverlap ( int iStart, int iLen, int iBoundary ) final;
	void	OnSPZ ( BYTE iSPZ, DWORD uPosition, const char * sZoneName, int iZone ) final;
	void	OnSkipHtml ( int iStart, int iLen ) final;
	void	OnTail ( int iStart, int iLen, int iBoundary ) final;
	void	OnFinish () final {}

	DictRefPtr_c &			GetDict() final { return m_pDict; }
	TokenizerRefPtr_c &		GetTokenizer() final { return m_pTokenizer; }
	const CSphIndexSettings &	GetIndexSettings() final { return m_tIndexSettings; }
	const SnippetQuerySettings_t &		GetSnippetQuery() final { return m_tQuery; }
	CSphVector<ZonePacked_t> &	GetZones() final { return m_dZones;	}
	FunctorZoneInfo_t &			GetZoneInfo() final { return m_tZoneInfo; }
	bool						NeedExtraZoneInfo() const final	{ return m_bCollectExtraZoneInfo; }
	DWORD						GetFoundWords() const final	{ return m_uFoundWords; }

private:
	SnippetsDocIndex_c &		m_tContainer;
	CacheStreamer_i &			m_tTokenContainer;

	CSphVector<ZonePacked_t> &	m_dZones;
	FunctorZoneInfo_t &			m_tZoneInfo;
	bool						m_bCollectExtraZoneInfo = false;
	DictRefPtr_c				m_pDict;

	mutable BYTE				m_sTmpWord [ 3*SPH_MAX_WORD_LEN + 16 ];
	SphWordID_t					m_uSentenceID;
	SphWordID_t					m_uParagraphID;
	DWORD						m_uFoundWords = 0;
};


HitCollector_c::HitCollector_c ( SnippetsDocIndex_c & tContainer, TokenizerRefPtr_c pTokenizer, DictRefPtr_c pDict, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, int iField, CacheStreamer_i & tTokenContainer, CSphVector<ZonePacked_t> & dZones, FunctorZoneInfo_t & tZoneInfo, SnippetResult_t & tRes )
	: TokenFunctorTraits_c ( std::move (pTokenizer), tQuery, tIndexSettings, szDoc, iDocLen, iField, tRes )
	, m_tContainer ( tContainer )
	, m_tTokenContainer ( tTokenContainer )
	, m_dZones ( dZones )
	, m_tZoneInfo ( tZoneInfo )
	, m_pDict ( std::move (pDict) )
{
	assert ( m_pDict );

	strncpy ( (char *)m_sTmpWord, MAGIC_WORD_SENTENCE, sizeof(m_sTmpWord)-1 );
	m_uSentenceID = m_pDict->GetWordID ( m_sTmpWord );
	strncpy ( (char *)m_sTmpWord, MAGIC_WORD_PARAGRAPH, sizeof(m_sTmpWord)-1 );
	m_uParagraphID = m_pDict->GetWordID ( m_sTmpWord );
	m_tContainer.SetupHits();

	m_bCollectExtraZoneInfo = true;
}


bool HitCollector_c::OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> & dTokens, const CSphVector<int> * pMultiPosDelta )
{
	bool bReal = false;

	assert ( tTok.m_iMultiPosLen==0 || ( pMultiPosDelta && pMultiPosDelta->GetLength()==dTokens.GetLength()+1 ) );
	bool bMultiform = ( tTok.m_iMultiPosLen!=0 );
	int iPos = tTok.m_uPosition;

	// different paths for leading token and tokens position generation
	// for blended and multi word-form with multiple destination word-forms
	if ( !bMultiform && tTok.m_uWordId )
	{
		m_tContainer.AddHits ( tTok.m_uWordId, tTok.m_sWord, tTok.m_iLen, HITMAN::Create ( m_iField, iPos ) );
		bReal = true;
	}
	ARRAY_FOREACH ( i, dTokens )
	{
		if ( dTokens[i] )
		{
			if ( bMultiform )
				iPos += ( *pMultiPosDelta )[i];

			m_tContainer.AddHits ( dTokens[i], tTok.m_sWord, tTok.m_iLen, HITMAN::Create ( m_iField, iPos ) );
			bReal = true;
		}
	}

	if ( bMultiform && tTok.m_uWordId )
	{
		if ( bMultiform )
			iPos += pMultiPosDelta->Last();

		m_tContainer.AddHits ( tTok.m_uWordId, tTok.m_sWord, tTok.m_iLen, HITMAN::Create ( m_iField, iPos ) );
		bReal = true;
	}

	m_tContainer.SetLastPos ( bReal ? iPos : m_tContainer.GetLastPos() );

	int iTermIndex = m_tContainer.FindWord ( tTok.m_uWordId, tTok.m_sWord, tTok.m_iLen );
	ARRAY_FOREACH_COND ( i, dTokens, iTermIndex==-1 )
		iTermIndex = m_tContainer.FindWord ( dTokens[i], NULL, 0 );

	m_uFoundWords |= iTermIndex==-1 ? 0 : 1 << iTermIndex;

	m_tTokenContainer.StoreToken ( tTok, iTermIndex );

	return true;
}


void HitCollector_c::OnSPZ ( BYTE iSPZ, DWORD uPosition, const char * sZoneName, int iZone )
{
	switch ( iSPZ )
	{
	case MAGIC_CODE_SENTENCE:
		m_tContainer.AddHits ( m_uSentenceID, NULL, 0, HITMAN::Create ( m_iField, uPosition ) );
		break;
	case MAGIC_CODE_PARAGRAPH:
		m_tContainer.AddHits ( m_uParagraphID, NULL, 0, HITMAN::Create ( m_iField, uPosition ) );
		break;
	case MAGIC_CODE_ZONE:
		assert ( m_dZones.GetLength() );
		assert ( ( ( m_dZones.Last()>>32 ) & UINT32_MASK )==uPosition );
		assert ( sZoneName );

		m_tContainer.AddHits ( m_pDict->GetWordID ( (BYTE *)const_cast<char*>(sZoneName) ), NULL, 0, HITMAN::Create ( m_iField, uPosition ) );
		break;
	default: assert ( 0 && "impossible SPZ" );
	}

	m_tContainer.SetLastPos(uPosition);

	m_tTokenContainer.StoreSPZ ( iSPZ, uPosition, sZoneName, iZone );
}


bool HitCollector_c::OnOverlap ( int iStart, int iLen, int iBoundary )
{
	m_tTokenContainer.StoreOverlap ( iStart, iLen, iBoundary );
	return true;
}


void HitCollector_c::OnSkipHtml ( int iStart, int iLen )
{
	m_tTokenContainer.StoreSkipHtml ( iStart, iLen );
}


void HitCollector_c::OnTail ( int iStart, int iLen, int iBoundary )
{
	m_tTokenContainer.StoreTail ( iStart, iLen, iBoundary );
}


//////////////////////////////////////////////////////////////////////////

std::unique_ptr<TokenFunctor_i> CreateDocStartHighlighter ( TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const SnippetLimits_t & tLimits, const CSphIndexSettings & tIndexSettings, const char * szDoc,
	int iDocLen, int iField, int & iResultCP, SnippetResult_t & tRes )
{
	return std::make_unique<DocStartHighlighter_c> ( std::move ( pTokenizer ), tQuery, tLimits, tIndexSettings, szDoc, iDocLen, iField, iResultCP, tRes );
}


std::unique_ptr<TokenFunctor_i> CreateQueryHighlighter ( TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen,
	const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes )
{
	return std::make_unique<QueryHighlighter_c> ( std::move ( pTokenizer ), tQuery, tIndexSettings, szDoc, iDocLen, dHits, iField, tRes );
}


std::unique_ptr<TokenFunctor_i> CreatePassageExtractor ( const SnippetsDocIndex_c & tContainer, PassageContext_t & tContext, TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const SnippetLimits_t & tLimits,
	const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes )
{
	return std::make_unique<PassageExtractor_c> ( tContainer, tContext, std::move ( pTokenizer ), tQuery, tLimits, tIndexSettings, szDoc, iDocLen, dHits, iField, tRes );
}


std::unique_ptr<TokenFunctor_i> CreatePassageHighlighter ( CSphVector<Passage_t*> & dPassages, TokenizerRefPtr_c pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, const FunctorZoneInfo_t & tZoneInfo, int iField, SnippetResult_t & tRes )
{
	return std::make_unique<PassageHighlighter_c> ( dPassages, std::move ( pTokenizer ), tQuery, tIndexSettings, szDoc, iDocLen, dHits, tZoneInfo, iField, tRes );
}


std::unique_ptr<HitCollector_i> CreateHitCollector ( SnippetsDocIndex_c & tContainer, TokenizerRefPtr_c pTokenizer, DictRefPtr_c pDict, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, int iField, CacheStreamer_i & tTokenContainer, CSphVector<ZonePacked_t> & dZones, FunctorZoneInfo_t & tZoneInfo, SnippetResult_t & tRes )
{
	return std::make_unique<HitCollector_c> ( tContainer, std::move ( pTokenizer ), std::move (pDict), tQuery, tIndexSettings, szDoc, iDocLen, iField, tTokenContainer, dZones, tZoneInfo, tRes );
}
