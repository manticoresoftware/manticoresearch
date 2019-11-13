//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
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


void Passage_t::Reset()
{
	m_iStart = 0;
	m_iTokens = 0;
	m_iCodes = 0;
	m_uQwords = 0;
	m_iQwordsWeight = 0;
	m_iQwordCount = 0;
	m_iUniqQwords = 0;
	m_iMaxLCS = 0;
	m_iMinGap = 0;
	m_iAroundBefore = 0;
	m_iAroundAfter = 0;
	m_iCodesBetweenKeywords = 0;
	m_iWordsBetweenKeywords = 0;
	m_iField = 0;
	m_dBeforeTokens.Resize(0);
	m_dAfterTokens.Resize(0);
}

void Passage_t::CopyData ( Passage_t & tPassage )
{
	m_iStart = tPassage.m_iStart;
	m_iTokens = tPassage.m_iTokens;
	m_iCodes = tPassage.m_iCodes;
	m_iWords = tPassage.m_iWords;
	m_uQwords = tPassage.m_uQwords;
	m_iQwordsWeight = tPassage.m_iQwordsWeight;
	m_iQwordCount = tPassage.m_iQwordCount;
	m_iUniqQwords = tPassage.m_iUniqQwords;
	m_iMaxLCS = tPassage.m_iMaxLCS;
	m_iMinGap = tPassage.m_iMinGap;
	m_iStartLimit = tPassage.m_iStartLimit;
	m_iEndLimit = tPassage.m_iEndLimit;
	m_iAroundBefore = tPassage.m_iAroundBefore;
	m_iAroundAfter = tPassage.m_iAroundAfter;
	m_iCodesBetweenKeywords = tPassage.m_iCodesBetweenKeywords;
	m_iWordsBetweenKeywords = tPassage.m_iWordsBetweenKeywords;
	m_iField = tPassage.m_iField;
	m_dBeforeTokens.SwapData ( tPassage.m_dBeforeTokens );
	m_dAfterTokens.SwapData ( tPassage.m_dAfterTokens );
}


inline int Passage_t::GetWeight () const
{
	return m_iQwordCount + m_iQwordsWeight*m_iMaxLCS + m_iMinGap;
}


inline bool operator < ( const Passage_t & a, const Passage_t & b )
{
	if ( a.m_iUniqQwords==b.m_iUniqQwords )
	{
		int iWeightA = a.GetWeight();
		int iWeightB = b.GetWeight();

		return iWeightA==iWeightB ? a.m_iCodes < b.m_iCodes : iWeightA < iWeightB;
	} else
		return a.m_iUniqQwords < b.m_iUniqQwords;
}


struct PassagePositionOrder_fn
{
	inline bool IsLess ( const Passage_t & a, const Passage_t & b ) const
	{
		return a.m_iStart < b.m_iStart;
	}
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
	m_uBufferMask = ( 1<<m_iBufferBits )-1;
	m_dTokens.Resize ( 1<<m_iBufferBits );
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


static inline int GetWordsLimit ( const SnippetQuerySettings_t & q, int iQwords )
{
	return q.m_iLimitWords ? q.m_iLimitWords : 2*q.m_iAround + iQwords;
}


static int EstimateResult ( const SnippetQuerySettings_t & q, int iDocLen )
{
	if ( q.m_iLimit>0 )
		return 2*q.m_iLimit;
	if ( q.m_iLimitPassages )
		return 256*q.m_iLimitPassages;
	if ( q.m_iLimitWords )
		return 16*q.m_iLimitWords;
	return iDocLen;
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
PassageContext_t::PassageContext_t()
{
	m_dPassages.Reserve(1024);
	memset ( m_dQwordWeights, 0, sizeof(m_dQwordWeights) );
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

			TokenFunctorTraits_c ( ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, int iField, SnippetResult_t & tRes );

	void	ResultEmit ( CSphVector<BYTE> & dBuf, const char * pSrc, int iLen, bool bHasPassageMacro=false, int iPassageId=0, const char * pPost=nullptr, int iPostLen=0 ) const;
	void	EmitPassageSeparator ( CSphVector<BYTE> & dBuf );
};


TokenFunctorTraits_c::TokenFunctorTraits_c ( ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, int iField, SnippetResult_t & tRes )
	: m_tResult ( tRes )
	, m_tQuery ( tQuery )
	, m_tIndexSettings ( tIndexSettings )
	, m_dResult ( m_tResult.m_dFields[iField].m_dPassages[0].m_dText )
	, m_pTokenizer ( pTokenizer )
	, m_szDocBuffer ( szDoc )
	, m_iDocLen ( iDocLen )
	, m_iField ( iField )

{
	SafeAddRef(pTokenizer);
	assert(m_pTokenizer);
	m_pTokenizer->SetBuffer ( (BYTE*)szDoc, m_iDocLen );
	m_pDoc = m_pTokenizer->GetBufferPtr();
	m_pDocMax = m_pTokenizer->GetBufferEnd();

	m_iSeparatorLen = m_tQuery.m_sChunkSeparator.Length();
}


void TokenFunctorTraits_c::ResultEmit ( CSphVector<BYTE> & dBuf, const char * pSrc, int iLen, bool bHasPassageMacro, int iPassageId,	const char * pPost, int iPostLen ) const
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
class DocStartHighlighter_c : public TokenFunctorTraits_c, public virtual DocStartHighlighter_i
{
public:
			DocStartHighlighter_c ( ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, int & iResultLenCP,
				int iField, SnippetResult_t & tRes );

	bool	OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> &, const CSphVector<int> * ) final;
	bool	OnOverlap ( int iStart, int iLen, int ) final;
	void	OnTail ( int iStart, int iLen, int ) final;
	void	OnSkipHtml ( int, int ) final {}
	void	OnSPZ ( BYTE, DWORD, const char *, int ) final {}
	void	OnFinish () final {}

	bool	CollectionStopped() const final { return m_bCollectionStopped; };

private:
	bool					m_bCollectionStopped = false;
	int	&					m_iResultLenCP;
	CSphVector<Space_t>		m_dSpaces;

	void	CollectStartTokens ( int iStart, int iLen, int iLenghCP = -1 );
	void	CollectStartSpaces();
};


DocStartHighlighter_c::DocStartHighlighter_c ( ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen, int & iResultLenCP,
	int iField, SnippetResult_t & tRes )
	: TokenFunctorTraits_c ( pTokenizer, tQuery, tIndexSettings, szDoc, iDocLen, iField, tRes )
	, m_iResultLenCP ( iResultLenCP )
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


void DocStartHighlighter_c::CollectStartTokens ( int iStart, int iLen, int iLenghCP )
{
	if ( m_tQuery.m_bAllowEmpty || m_bCollectionStopped )
		return;

	int iCalcLengthCP = iLenghCP;
	if ( iCalcLengthCP==-1 )
		iCalcLengthCP = sphUTF8Len ( m_pDoc+iStart, iLen );

	bool bLengthOk = m_iResultLenCP+iCalcLengthCP<=m_tQuery.m_iLimit;
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
class PassageExtractor_c : public TokenFunctorTraits_c, public HitTraits_c, public virtual PassageExtractor_i
{
public:
				PassageExtractor_c ( const SnippetsDocIndex_c & tContainer, PassageContext_t & tPassageContext, ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
					const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, DWORD uFoundWords, int iField, SnippetResult_t & tRes );

protected:
	bool		OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> & dTokens, const CSphVector<int> * ) final;
	bool		OnOverlap ( int iStart, int iLen, int iBoundary ) final;
	void		OnSkipHtml ( int iStart, int iLen ) final {}
	void		OnSPZ ( BYTE iSPZ, DWORD uPosition, const char * szZone, int iZone ) final;
	void		OnTail ( int iStart, int iLen, int iBoundary ) final;
	void		OnFinish() final;

	bool		GetWeightOrder() const final { return m_bFixedWeightOrder; }
	void		Finalize() final;

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

	TokenSpan_c				m_tSpan;
	Passage_t				m_tPass;
	int						m_iCurToken = 0;
	bool					m_bFixedWeightOrder = false;
	DWORD					m_uFoundWords = 0;
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
	bool		SelectBestPassages ( CSphVector<Passage_t> & dShow );
};


PassageExtractor_c::PassageExtractor_c ( const SnippetsDocIndex_c & tContainer, PassageContext_t & tPassageContext, ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, DWORD uFoundWords, int iField, SnippetResult_t & tRes )
	: TokenFunctorTraits_c ( pTokenizer, tQuery, tIndexSettings, szDoc, iDocLen, iField, tRes )
	, HitTraits_c ( dHits )
	, m_tContainer ( tContainer )
	, m_bFixedWeightOrder ( tQuery.m_bWeightOrder )
	, m_uFoundWords ( uFoundWords )
	, m_tContext ( tPassageContext )
{
	const int AVG_WORD_LEN = 5;
	int iSpanSize = tQuery.m_iLimit ? 8*tQuery.m_iLimit/AVG_WORD_LEN : 2*tQuery.m_iLimitWords;
	if ( !iSpanSize )
		iSpanSize = 128;

	m_tSpan.Init ( iSpanSize );
	m_tPass.Reset();

	if ( tQuery.m_iLimitPassages>0 )
		m_iThresh = tQuery.m_iLimitPassages;
	else if ( tQuery.m_iLimitWords>0 )
		m_iThresh = tQuery.m_iLimitWords / 2;
	else if ( tQuery.m_iLimit>0 )
		m_iThresh = tQuery.m_iLimit / 4;

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
		const int iCpLimit = m_tQuery.m_iLimit ? m_tQuery.m_iLimit : INT_MAX;
		bool bLimitsOk = m_tSpan.m_iCodes+iLengthCP<=iCpLimit && m_tSpan.m_iWords<=GetWordsLimit ( m_tQuery, m_tSpan.m_iQwords );

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


void PassageExtractor_c::Finalize()
{
	if ( !m_tContext.m_dPassages.GetLength() )
		return;

	CSphVector<Passage_t> dPassagesToShow;
	if ( SelectBestPassages ( dPassagesToShow ) )
	{
		ARRAY_FOREACH ( i, dPassagesToShow )
		{
			dPassagesToShow[i].m_dBeforeTokens.Reset();
			dPassagesToShow[i].m_dAfterTokens.Reset();
		}
		m_tContext.m_dPassages = dPassagesToShow;

	} else
		m_tContext.m_dPassages.Resize(0);
}


void PassageExtractor_c::AddSpaces ( int iBoundary )
{
	switch ( m_eState )
	{
	case STATE_WINDOW_SETUP:
	{
		DWORD uAllQwords = ( 1 << m_tContainer.GetNumTerms() )-1;
		const int iCpLimit = m_tQuery.m_iLimit ? m_tQuery.m_iLimit : INT_MAX;
		ARRAY_FOREACH ( i, m_dSpaces )
		{
			bool bBoundary = iBoundary==m_dSpaces[i].m_iStartBytes;
			bool bLimitsOk = m_tSpan.m_iCodes+m_dSpaces[i].m_iLengthCP<=iCpLimit && m_tSpan.m_iWords<=GetWordsLimit ( m_tQuery, m_tSpan.m_iQwords );

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
			CalcPassageWeight ( GetWordsLimit ( m_tQuery, m_tSpan.m_iQwords ) );
			m_bQwordsChanged = false;
		} else
			UpdateGaps ( GetWordsLimit ( m_tQuery, m_tSpan.m_iQwords ) );

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
	const int iCpLimit = m_tQuery.m_iLimit ? m_tQuery.m_iLimit : INT_MAX;

	int iTokenStart = 0;
	const int iMaxToken = m_tSpan.GetNumTokens() - 1;

	// drop front tokens until the window fits into both word and CP limits
	while ( iTokenStart < iMaxToken
		&& ( m_tSpan.m_iCodes > iCpLimit || m_tSpan.m_iWords > GetWordsLimit ( m_tQuery, m_tSpan.m_iQwords ) ) )
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
	if ( ( ( m_tQuery.m_iLimit==0 || m_tQuery.m_iLimit>=m_iDocLen ) && !m_tQuery.m_iLimitWords && m_tQuery.m_ePassageSPZ==SPH_SPZ_NONE ) || m_tQuery.m_bUseBoundaries )
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


bool PassageExtractor_c::SelectBestPassages ( CSphVector<Passage_t> & dShow )
{
	assert ( m_tContext.m_dPassages.GetLength() );

	// our limits
	int iMaxPassages = m_tQuery.m_iLimitPassages
		? Min ( m_tContext.m_dPassages.GetLength(), m_tQuery.m_iLimitPassages )
		: m_tContext.m_dPassages.GetLength();
	int iMaxWords = m_tQuery.m_iLimitWords ? m_tQuery.m_iLimitWords : INT_MAX;
	int iMaxCp = m_tQuery.m_iLimit ? m_tQuery.m_iLimit : INT_MAX;

	DWORD uWords = 0; // mask of words in dShow so far
	int iTotalCodes = 0;
	int iTotalWords = 0;
	int iTotalKeywordCodes = 0;
	int iTotalKeywordWords = 0;

	CSphVector<int> dWeights ( m_tContext.m_dPassages.GetLength() );
	ARRAY_FOREACH ( i, m_tContext.m_dPassages )
		dWeights[i] = m_tContext.m_dPassages[i].m_iQwordsWeight;

	// collect enough best passages to show all keywords and max out the limits
	// don't care much if we're going over limits in this loop, it will be tightened below
	bool bAll = false;
	while ( dShow.GetLength() < iMaxPassages )
	{
		// get next best passage
		int iBest = -1;
		ARRAY_FOREACH ( i, m_tContext.m_dPassages )
		{
			Passage_t & tPass = m_tContext.m_dPassages[i];
			if ( tPass.m_iCodes && ( iBest==-1 || m_tContext.m_dPassages[iBest] < tPass ) )
				iBest = i;
		}
		if ( iBest<0 )
			break;

		Passage_t & tBest = m_tContext.m_dPassages[iBest];

		// force_all_keywords passage may be very big, so let's allow to show one of them
		if ( !m_tQuery.m_bForceAllWords || dShow.GetLength() )
			if ( iTotalKeywordCodes+tBest.m_iCodesBetweenKeywords>iMaxCp || iTotalKeywordWords+tBest.m_iWordsBetweenKeywords>iMaxWords )
				break;

		bool bFits = iTotalCodes+tBest.m_iCodes<=iMaxCp && iTotalWords+tBest.m_iWords<=iMaxWords;
		if ( uWords==m_uFoundWords && !bFits )
		{
			// there might be just enough space to partially display this passage
			if ( iTotalCodes+tBest.m_iCodesBetweenKeywords<=iMaxCp && iTotalWords+tBest.m_iWordsBetweenKeywords<=iMaxWords )
			{
				iTotalWords += tBest.m_iWords;
				iTotalCodes += tBest.m_iCodes;
				dShow.Add ( tBest );
			}
			break;
		}

		// save it, despite limits or whatever, we'll tighten everything in the loop below
		dShow.Add ( tBest );
		uWords |= tBest.m_uQwords;
		iTotalKeywordWords += tBest.m_iWordsBetweenKeywords;
		iTotalKeywordCodes += tBest.m_iCodesBetweenKeywords;
		iTotalWords += tBest.m_iWords;
		iTotalCodes += tBest.m_iCodes;
		tBest.m_iCodes = 0; // no longer needed here, abusing to mark displayed passages

							// we just managed to show all words? do one final re-weighting run
		if ( !bAll && uWords==m_uFoundWords )
		{
			bAll = true;
			ARRAY_FOREACH ( i, m_tContext.m_dPassages )
				m_tContext.m_dPassages[i].m_iQwordsWeight = dWeights[i];
		}

		// if we're already showing all words, re-weighting is not needed any more
		if ( bAll )
			continue;

		// re-weight passages, adjust for new mask of shown words
		// FIXME! re-weighting doesn't change m_iQwordCount (and qwords could possibly be duplicated) and LCS
		ARRAY_FOREACH ( i, m_tContext.m_dPassages )
		{
			Passage_t & tPass = m_tContext.m_dPassages[i];
			if ( !tPass.m_iCodes )
				continue;

			DWORD uMask = tBest.m_uQwords;
			for ( int iWord=0; uMask; iWord++, uMask >>= 1 )
				if ( ( uMask & 1 ) && ( tPass.m_uQwords & ( 1UL<<iWord ) ) )
				{
					tPass.m_iQwordsWeight -= m_tContainer.GetTermWeight(iWord);
					tPass.m_iQwordCount--; // doesn't account for dupes
					tPass.m_iUniqQwords--;
				}

			tPass.m_uQwords &= ~uWords;
		}
	}

	// if all passages won't fit into the limit, try to trim them a bit
	//
	// this step is skipped when use_boundaries is enabled, because
	// each passage must be a separate sentence (delimited by
	// boundaries) and we don't want to split them
	if ( ( iTotalCodes > iMaxCp || iTotalWords > iMaxWords ) && !m_tQuery.m_bUseBoundaries )
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

				if ( !tPassage.m_dBeforeTokens.GetLength() && !tPassage.m_dAfterTokens.GetLength() )
					continue;

				bool bDropFirst;
				if ( tPassage.m_dBeforeTokens.GetLength() > tPassage.m_dAfterTokens.GetLength() )
					bDropFirst = true;
				else if ( tPassage.m_dBeforeTokens.GetLength() < tPassage.m_dAfterTokens.GetLength() )
					bDropFirst = false;
				else if ( !tPassage.m_dBeforeTokens.Last().m_iWordFlag && tPassage.m_dAfterTokens.Last().m_iWordFlag )
					bDropFirst = true;
				else if ( tPassage.m_dBeforeTokens.Last().m_iWordFlag && !tPassage.m_dAfterTokens.Last().m_iWordFlag )
					bDropFirst = false;
				else
					bDropFirst = bFirst;

				if ( bDropFirst )
				{
					// drop first
					const StoredExcerptToken_t & tTok = tPassage.m_dBeforeTokens.Pop();
					tPassage.m_iStart++;
					tPassage.m_iTokens--;
					tPassage.m_iCodes -= tTok.m_iLengthCP;
					iTotalCodes -= tTok.m_iLengthCP;
					iTotalWords -= tTok.m_iWordFlag;

				} else
				{
					// drop last
					const StoredExcerptToken_t & tTok = tPassage.m_dAfterTokens.Pop();
					tPassage.m_iTokens--;
					tPassage.m_iCodes -= tTok.m_iLengthCP;
					iTotalCodes -= tTok.m_iLengthCP;
					iTotalWords -= tTok.m_iWordFlag;
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
	while ( ( iTotalCodes > iMaxCp || iTotalWords > iMaxWords ) && !m_tQuery.m_bForceAllWords )
	{
		iTotalCodes -= dShow.Last().m_iCodes;
		iTotalWords -= dShow.Last().m_iWords;
		dShow.RemoveFast ( dShow.GetLength()-1 );
	}

	if ( !dShow.GetLength() )
		return false;

	bool bPreSorted = false;
	if ( m_tQuery.m_bWeightOrder )
	{
		// maybe the weight order is the same as position order?
		bool bPositionOrder = true;
		for ( int i = 1; i<dShow.GetLength() && bPositionOrder; i++ )
			if ( dShow[i-1].m_iStart>dShow[i].m_iStart )
				bPositionOrder = false;

		bPreSorted = bPositionOrder;

		if ( bPositionOrder )
			m_bFixedWeightOrder = false;
	}

	// sort passages in the document order
	if ( !bPreSorted )
		dShow.Sort ( PassagePositionOrder_fn() );

	return true;
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
			PassageHighlighter_c ( CSphVector<Passage_t*> & dPassages, ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen,
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


PassageHighlighter_c::PassageHighlighter_c ( CSphVector<Passage_t*> & dPassages, ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen,
	const CSphVector<SphHitMark_t> & dHits, const FunctorZoneInfo_t & tZoneInfo, int iField, SnippetResult_t & tRes )
	: TokenFunctorTraits_c ( pTokenizer, tQuery, tIndexSettings, szDoc, iDocLen, iField, tRes )
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
		m_tZoneInfo.m_hZones.IterateStart();
		while ( m_tZoneInfo.m_hZones.IterateNext() )
		{
			if ( m_tZoneInfo.m_hZones.IterateGet()!=iParent )
				continue;

			assert ( m_pCurPassageText );
			ResultEmit ( *m_pCurPassageText, "<", 1 );
			ResultEmit ( *m_pCurPassageText, m_tZoneInfo.m_hZones.IterateGetKey().cstr(), m_tZoneInfo.m_hZones.IterateGetKey().Length() );
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
	const Passage_t * pPassage = *(m_dPassages.Begin()+m_iCurPassage);
	const Passage_t * pNext = *(m_dPassages.Begin()+m_iCurPassage+1);

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
	QueryHighlighter_c ( ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
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


QueryHighlighter_c::QueryHighlighter_c ( ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes )
	: TokenFunctorTraits_c ( pTokenizer, tQuery, tIndexSettings, szDoc, iDocLen, iField, tRes )
	, BeforeAfterTraits_c(tQuery)
	, HitTraits_c(dHits)
{
	m_dResult.Reserve ( EstimateResult ( tQuery, iDocLen ) );
}


bool QueryHighlighter_c::OnToken ( const TokenInfo_t & tTok, const CSphVector<SphWordID_t> &, const CSphVector<int> * )
{
	assert ( m_pDoc );
	assert ( tTok.m_iStart>=0 && m_pDoc+tTok.m_iStart+tTok.m_iLen<=m_pDocMax );

	RewindHits ( tTok.m_uPosition, m_iField );
	CheckClose ( tTok.m_uPosition );

	// marker folding, emit "before" marker at span start only
	if ( m_pHit<m_pHitEnd && IsTokenHit ( tTok, m_pHit->m_uPosition, 0, m_iField ) && !m_iOpenUntilTokenPos )
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
	HitCollector_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
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
	DictRefPtr_c			m_pDict;

	mutable BYTE				m_sTmpWord [ 3*SPH_MAX_WORD_LEN + 16 ];
	SphWordID_t					m_uSentenceID;
	SphWordID_t					m_uParagraphID;
	DWORD						m_uFoundWords = 0;
};


HitCollector_c::HitCollector_c ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, int iField, CacheStreamer_i & tTokenContainer, CSphVector<ZonePacked_t> & dZones, FunctorZoneInfo_t & tZoneInfo, SnippetResult_t & tRes )
	: TokenFunctorTraits_c ( pTokenizer, tQuery, tIndexSettings, szDoc, iDocLen, iField, tRes )
	, m_tContainer ( tContainer )
	, m_tTokenContainer ( tTokenContainer )
	, m_dZones ( dZones )
	, m_tZoneInfo ( tZoneInfo )
	, m_pDict ( pDict )
{
	SafeAddRef(pDict);
	assert(m_pDict);

	strncpy ( (char *)m_sTmpWord, MAGIC_WORD_SENTENCE, sizeof(m_sTmpWord)-1 );
	m_uSentenceID = pDict->GetWordID ( m_sTmpWord );
	strncpy ( (char *)m_sTmpWord, MAGIC_WORD_PARAGRAPH, sizeof(m_sTmpWord)-1 );
	m_uParagraphID = pDict->GetWordID ( m_sTmpWord );
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

		m_tContainer.AddHits ( m_pDict->GetWordID ( (BYTE *)sZoneName ), NULL, 0, HITMAN::Create ( m_iField, uPosition ) );
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

DocStartHighlighter_i * CreateDocStartHighlighter ( ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen,
	int & iResultLenCP,	int iField, SnippetResult_t & tRes )
{
	return new DocStartHighlighter_c ( pTokenizer, tQuery, tIndexSettings, szDoc, iDocLen, iResultLenCP, iField, tRes );
}


TokenFunctor_i * CreateQueryHighlighter ( ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings, const char * szDoc, int iDocLen,
	const CSphVector<SphHitMark_t> & dHits, int iField, SnippetResult_t & tRes )
{
	return new QueryHighlighter_c ( pTokenizer, tQuery, tIndexSettings, szDoc, iDocLen, dHits, iField, tRes );
}


PassageExtractor_i * CreatePassageExtractor ( const SnippetsDocIndex_c & tContainer, PassageContext_t & tContext, ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, DWORD uFoundWords, int iField, SnippetResult_t & tRes )
{
	return new PassageExtractor_c ( tContainer, tContext, pTokenizer, tQuery, tIndexSettings, szDoc, iDocLen, dHits, uFoundWords, iField, tRes );
}


TokenFunctor_i * CreatePassageHighlighter ( CSphVector<Passage_t*> & dPassages, ISphTokenizer * pTokenizer, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, const CSphVector<SphHitMark_t> & dHits, const FunctorZoneInfo_t & tZoneInfo, int iField, SnippetResult_t & tRes )
{
	return new PassageHighlighter_c ( dPassages, pTokenizer, tQuery, tIndexSettings, szDoc, iDocLen, dHits, tZoneInfo, iField, tRes );
}


HitCollector_i * CreateHitCollector ( SnippetsDocIndex_c & tContainer, ISphTokenizer * pTokenizer, CSphDict * pDict, const SnippetQuerySettings_t & tQuery, const CSphIndexSettings & tIndexSettings,
	const char * szDoc, int iDocLen, int iField, CacheStreamer_i & tTokenContainer, CSphVector<ZonePacked_t> & dZones, FunctorZoneInfo_t & tZoneInfo, SnippetResult_t & tRes )
{
	return new HitCollector_c ( tContainer, pTokenizer, pDict, tQuery, tIndexSettings, szDoc, iDocLen, iField, tTokenContainer, dZones, tZoneInfo, tRes );
}