//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "tokenizerbase2_impl.h"

#include "lowercaser_impl.h"
#include "exceptions_trie.h"
#include "sphinxint.h"

static inline void CopySubstring ( BYTE* pDst, const BYTE* pSrc, int iLen )
{
	while ( iLen-- > 0 && *pSrc )
		*pDst++ = *pSrc++;
	*pDst++ = '\0';
}


static FORCE_INLINE bool IsSeparator ( int iFolded, bool bFirst )
{
	// eternal separator
	if ( iFolded < 0 || ( iFolded & MASK_CODEPOINT ) == 0 )
		return true;

	// just a codepoint
	if ( !( iFolded & MASK_FLAGS ) )
		return false;

	// any magic flag, besides dual
	if ( !( iFolded & FLAG_CODEPOINT_DUAL ) )
		return true;

	// FIXME? n-grams currently also set dual
	if ( iFolded & FLAG_CODEPOINT_NGRAM )
		return true;

	// dual depends on position
	return bFirst;
}

int CSphTokenizerBase2::SkipBlended()
{
	if ( !m_pBlendEnd )
		return 0;

	const BYTE* pMax = m_pBufferMax;
	m_pBufferMax = m_pBlendEnd;

	// loop until the blended token end
	int iBlended = 0; // how many blended subtokens we have seen so far
	int iAccum = 0;	  // how many non-blended chars in a row we have seen so far
	while ( m_pCur < m_pBufferMax )
	{
		int iCode = GetCodepoint();
		if ( iCode == '\\' )
			iCode = GetCodepoint();				   // no boundary check, GetCP does it
		iCode = GetLowercaser().ToLower ( iCode ); // no -1 check, ToLower does it
		if ( iCode < 0 )
			iCode = 0;
		if ( iCode & FLAG_CODEPOINT_BLEND )
			iCode = 0;
		if ( iCode & MASK_CODEPOINT )
		{
			iAccum++;
			continue;
		}
		if ( iAccum >= m_tSettings.m_iMinWordLen )
			iBlended++;
		iAccum = 0;
	}
	if ( iAccum >= m_tSettings.m_iMinWordLen )
		iBlended++;

	m_pBufferMax = pMax;
	return iBlended;
}

BYTE* CSphTokenizerBase2::GetBlendedVariant()
{
	// we can get called on several occasions
	// case 1, a new blended token was just accumulated
	if ( m_bBlended && !m_bBlendAdd )
	{
		// fast path for the default case (trim_none)
		if ( m_uBlendVariants == BLEND_TRIM_NONE )
			return m_sAccum;

		// analyze the full token, find non-blended bounds
		m_iBlendNormalStart = -1;
		m_iBlendNormalEnd = -1;

		// OPTIMIZE? we can skip this based on non-blended flag from adjust
		const BYTE* p = m_sAccum;
		while ( *p )
		{
			int iLast = (int)( p - m_sAccum );
			int iCode = sphUTF8Decode ( p );
			if ( !( GetLowercaser().ToLower ( iCode ) & FLAG_CODEPOINT_BLEND ) )
			{
				m_iBlendNormalEnd = (int)( p - m_sAccum );
				if ( m_iBlendNormalStart < 0 )
					m_iBlendNormalStart = iLast;
			}
		}

		// build todo mask
		// check and revert a few degenerate cases
		m_uBlendVariantsPending = m_uBlendVariants;
		if ( m_uBlendVariantsPending & BLEND_TRIM_BOTH )
		{
			if ( m_iBlendNormalStart < 0 )
			{
				// no heading blended; revert BOTH to TAIL
				m_uBlendVariantsPending &= ~BLEND_TRIM_BOTH;
				m_uBlendVariantsPending |= BLEND_TRIM_TAIL;
			} else if ( m_iBlendNormalEnd < 0 )
			{
				// no trailing blended; revert BOTH to HEAD
				m_uBlendVariantsPending &= ~BLEND_TRIM_BOTH;
				m_uBlendVariantsPending |= BLEND_TRIM_HEAD;
			}
		}
		if ( m_uBlendVariantsPending & BLEND_TRIM_HEAD )
		{
			// either no heading blended, or pure blended; revert HEAD to NONE
			if ( m_iBlendNormalStart <= 0 )
			{
				m_uBlendVariantsPending &= ~BLEND_TRIM_HEAD;
				m_uBlendVariantsPending |= BLEND_TRIM_NONE;
			}
		}
		if ( m_uBlendVariantsPending & BLEND_TRIM_TAIL )
		{
			// either no trailing blended, or pure blended; revert TAIL to NONE
			if ( m_iBlendNormalEnd <= 0 || m_sAccum[m_iBlendNormalEnd] == 0 )
			{
				m_uBlendVariantsPending &= ~BLEND_TRIM_TAIL;
				m_uBlendVariantsPending |= BLEND_TRIM_NONE;
			}
		}

		// ok, we are going to return a few variants after all, flag that
		// OPTIMIZE? add fast path for "single" variants?
		m_bBlendAdd = true;
		assert ( m_uBlendVariantsPending );

		// we also have to stash the original blended token
		// because accumulator contents may get trashed by caller (say, when stemming)
		strncpy ( (char*)m_sAccumBlend, (char*)m_sAccum, sizeof ( m_sAccumBlend ) - 1 );
	}

	// case 2, caller is checking for pending variants, have we even got any?
	if ( !m_bBlendAdd )
		return nullptr;

	// handle trim_none
	// this MUST be the first handler, so that we could avoid copying below, and just return the original accumulator
	if ( m_uBlendVariantsPending & BLEND_TRIM_NONE )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_NONE;
		m_bBlended = true;
		return m_sAccum;
	}

	// handle trim_all
	if ( m_uBlendVariantsPending & BLEND_TRIM_ALL )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_ALL;
		m_bBlended = true;
		const BYTE* pSrc = m_sAccumBlend;
		BYTE* pDst = m_sAccum;
		while ( *pSrc )
		{
			int iCode = sphUTF8Decode ( pSrc );
			if ( !( GetLowercaser().ToLower ( iCode ) & FLAG_CODEPOINT_BLEND ) )
				pDst += sphUTF8Encode ( pDst, ( iCode & MASK_CODEPOINT ) );
		}
		*pDst = '\0';

		return m_sAccum;
	}

	// handle trim_both
	if ( m_uBlendVariantsPending & BLEND_TRIM_BOTH )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_BOTH;
		if ( m_iBlendNormalStart < 0 )
			m_uBlendVariantsPending |= BLEND_TRIM_TAIL; // no heading blended; revert BOTH to TAIL
		else if ( m_iBlendNormalEnd < 0 )
			m_uBlendVariantsPending |= BLEND_TRIM_HEAD; // no trailing blended; revert BOTH to HEAD
		else
		{
			assert ( m_iBlendNormalStart < m_iBlendNormalEnd );
			CopySubstring ( m_sAccum, m_sAccumBlend + m_iBlendNormalStart, m_iBlendNormalEnd - m_iBlendNormalStart );
			m_bBlended = true;
			return m_sAccum;
		}
	}

	// handle TRIM_HEAD
	if ( m_uBlendVariantsPending & BLEND_TRIM_HEAD )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_HEAD;
		if ( m_iBlendNormalStart >= 0 )
		{
			// FIXME! need we check for overshorts?
			CopySubstring ( m_sAccum, m_sAccumBlend + m_iBlendNormalStart, sizeof ( m_sAccum ) );
			m_bBlended = true;
			return m_sAccum;
		}
	}

	// handle TRIM_TAIL
	if ( m_uBlendVariantsPending & BLEND_TRIM_TAIL )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_TAIL;
		if ( m_iBlendNormalEnd > 0 )
		{
			// FIXME! need we check for overshorts?
			CopySubstring ( m_sAccum, m_sAccumBlend, m_iBlendNormalEnd );
			m_bBlended = true;
			return m_sAccum;
		}
	}

	// all clear, no more variants to go
	m_bBlendAdd = false;
	return nullptr;
}

bool CSphTokenizerBase2::CheckException ( const BYTE* pStart, const BYTE* pCur, bool bQueryMode )
{
	assert ( m_pExc );
	assert ( pStart );

	// at this point [pStart,pCur) is our regular tokenization candidate,
	// and pCur is pointing at what normally is considered separtor
	//
	// however, it might be either a full exception (if we're really lucky)
	// or (more likely) an exception prefix, so lets check for that
	//
	// interestingly enough, note that our token might contain a full exception
	// as a prefix, for instance [USAF] token vs [USA] exception; but in that case
	// we still need to tokenize regularly, because even exceptions need to honor
	// word boundaries

	// lets begin with a special (hopefully fast) check for the 1st byte
	const BYTE* p = pStart;
	if ( m_pExc->GetFirst ( *p ) < 0 )
		return false;

	// consume all the (character data) bytes until the first separator
	int iNode = 0;
	while ( p < pCur )
	{
		if ( bQueryMode && *p == '\\' )
		{
			p++;
			continue;
		}
		iNode = m_pExc->GetNext ( iNode, *p++ );
		if ( iNode < 0 )
			return false;
	}

	const BYTE* pMapEnd = nullptr; // the longest exception found so far is [pStart,pMapEnd)
	const BYTE* pMapTo = nullptr;  // the destination mapping
	bool bHasQueryQuote = false;

	// now, we got ourselves a valid exception prefix, so lets keep consuming more bytes,
	// ie. until further separators, and keep looking for a full exception match
	while ( iNode >= 0 )
	{
		// in query mode, ignore quoting slashes
		if ( bQueryMode && *p == '\\' )
		{
			p++;
			bHasQueryQuote = true;
			continue;
		}

		// decode one more codepoint, check if it is a separator
		bool bSep = true;
		bool bSpace = sphIsSpace ( *p ); // okay despite utf-8, cause hard whitespace is all ascii-7

		const BYTE* q = p;
		if ( p < m_pBufferMax )
			bSep = IsSeparator ( GetLowercaser().ToLower ( sphUTF8Decode ( q ) ), false ); // FIXME? sometimes they ARE first

		// there is a separator ahead, so check if we have a full match
		if ( bSep && m_pExc->GetMapping ( iNode ) )
		{
			pMapEnd = p;
			pMapTo = m_pExc->GetMapping ( iNode );
		}

		// eof? bail
		if ( p >= m_pBufferMax )
			break;

		// not eof? consume those bytes
		if ( bSpace )
		{
			// and fold (hard) whitespace while we're at it!
			while ( sphIsSpace ( *p ) )
				p++;
			iNode = m_pExc->GetNext ( iNode, ' ' );
		} else
		{
			// just consume the codepoint, byte-by-byte
			while ( p < q && iNode >= 0 )
				iNode = m_pExc->GetNext ( iNode, *p++ );
		}

		// we just consumed a separator, so check for a full match again
		if ( iNode >= 0 && bSep && m_pExc->GetMapping ( iNode ) )
		{
			pMapEnd = p;
			pMapTo = m_pExc->GetMapping ( iNode );
		}
	}

	// found anything?
	if ( !pMapTo )
		return false;

	strncpy ( (char*)m_sAccum, (char*)const_cast<BYTE*> ( pMapTo ), sizeof ( m_sAccum ) - 1 );
	m_pTokenStart = pStart;
	m_iLastTokenLen = (int)strlen ( (char*)m_sAccum );
	if ( bHasQueryQuote )
	{
		// move backpointer to the head of the quoting sequence
		while ( pMapEnd-1>=pStart && *(pMapEnd-1)=='\\' )
			pMapEnd--;
	}
	m_pCur = pMapEnd;
	m_pTokenEnd = pMapEnd;

	m_bWasSynonym = true;
	return true;
}

void CSphTokenizerBase2::FlushAccum()
{
	assert ( m_pAccum - m_sAccum < (int)sizeof ( m_sAccum ) );
	m_iLastTokenLen = m_iAccum;
	*m_pAccum = 0;
	m_iAccum = 0;
	m_pAccum = m_sAccum;
}
