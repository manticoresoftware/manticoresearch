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

#pragma once

#include "tokenizerbase_impl.h"

#include "sphinxdefs.h"
#include "sphinxint.h"

#include "lowercaser_impl.h"
#include "exceptions_trie.h"


inline bool IsWhitespace ( int c )
{
	return ( c == '\0' || c == ' ' || c == '\t' || c == '\r' || c == '\n' );
}

inline bool IsPunctuation ( int c )
{
	return ( c >= 33 && c <= 47 ) || ( c >= 58 && c <= 64 ) || ( c >= 91 && c <= 96 ) || ( c >= 123 && c <= 126 );
}

inline bool ShortTokenFilter ( const BYTE* pToken, int iLen )
{
	return pToken[0] == '*' || ( iLen > 0 && pToken[iLen - 1] == '*' );
}

// handles escaped specials that are not in the character set
// returns true if the codepoint should be processed as a simple codepoint,
// returns false if it should be processed as a whitespace
// for example: aaa\!bbb => aaa bbb
inline bool Special2Simple ( int& iCodepoint )
{
	if ( ( iCodepoint & FLAG_CODEPOINT_DUAL ) || !( iCodepoint & FLAG_CODEPOINT_SPECIAL ) )
	{
		iCodepoint &= ~( FLAG_CODEPOINT_SPECIAL | FLAG_CODEPOINT_DUAL );
		return true;
	}

	return false;
}

/// methods that get specialized with regards to charset type
/// aka GetCodepoint() decoder and everything that depends on it
class CSphTokenizerBase2: public CSphTokenizerBase
{
protected:
	/// get codepoint
	inline int GetCodepoint()
	{
		while ( m_pCur < m_pBufferMax )
		{
			int iCode = sphUTF8Decode ( m_pCur );
			if ( iCode >= 0 )
				return iCode; // successful decode
		}
		return -1; // eof
	}

	/// accum codepoint
	inline void AccumCodepoint ( int iCode )
	{
		assert ( iCode > 0 );
		assert ( m_iAccum >= 0 );

		// throw away everything which is over the token size
		bool bFit = ( m_iAccum < SPH_MAX_WORD_LEN );
		bFit &= ( m_pAccum - m_sAccum + SPH_MAX_UTF8_BYTES <= (int)sizeof ( m_sAccum ) );

		if ( bFit )
		{
			m_pAccum += sphUTF8Encode ( m_pAccum, iCode );
			assert ( m_pAccum >= m_sAccum && m_pAccum < m_sAccum + sizeof ( m_sAccum ) );
			m_iAccum++;
		}
	}

protected:
	BYTE* GetBlendedVariant();
	bool CheckException ( const BYTE* pStart, const BYTE* pCur, bool bQueryMode );

	template<bool IS_QUERY, bool IS_BLEND>
	BYTE* DoGetToken()
	{
		// return pending blending variants
		if_const ( IS_BLEND )
		{
			BYTE* pVar = GetBlendedVariant();
			if ( pVar )
				return pVar;
			m_bBlendedPart = ( m_pBlendEnd != nullptr );
		}

		// in query mode, lets capture (soft-whitespace hard-whitespace) sequences and adjust overshort counter
		// sample queries would be (one NEAR $$$) or (one | $$$ two) where $ is not a valid character
		bool bGotNonToken = ( !IS_QUERY || m_bPhrase ); // only do this in query mode, never in indexing mode, never within phrases
		bool bGotSoft = false;							// hey Beavis he said soft huh huhhuh

		m_pTokenStart = nullptr;
		for ( ;; )
		{
			// get next codepoint
			const BYTE* const pCur = m_pCur; // to redo special char, if there's a token already

			int iCodePoint = ( pCur < m_pBufferMax && *pCur < 128 ) ? *m_pCur++ : GetCodepoint();
			int iCode = GetLowercaser().ToLower ( iCodePoint );

			// handle escaping
			const bool bWasEscaped = ( IS_QUERY && iCodePoint == '\\' ); // whether current codepoint was escaped
			if ( bWasEscaped )
			{
				iCodePoint = GetCodepoint();
				iCode = GetLowercaser().ToLower ( iCodePoint );
				if ( !Special2Simple ( iCode ) )
					iCode = 0;
			}

			// handle eof
			if ( iCode < 0 )
			{
				FlushAccum();

				// suddenly, exceptions
				if ( m_pExc && m_pTokenStart && CheckException ( m_pTokenStart, pCur, IS_QUERY ) )
					return m_sAccum;

				// skip trailing short word
				if ( m_iLastTokenLen < m_tSettings.m_iMinWordLen )
				{
					if ( !m_bShortTokenFilter || !ShortTokenFilter ( m_sAccum, m_iLastTokenLen ) )
					{
						if ( m_iLastTokenLen )
							++m_iOvershortCount;
						m_iLastTokenLen = 0;
						if_const ( IS_BLEND )
							BlendAdjust ( pCur );
						return nullptr;
					}
				}

				// keep token end here as BlendAdjust might change m_pCur
				m_pTokenEnd = m_pCur;

				// return trailing word
				if_const ( IS_BLEND && !BlendAdjust ( pCur ) ) return nullptr;
				if_const ( IS_BLEND && m_bBlended ) return GetBlendedVariant();
				return m_sAccum;
			}

			// handle all the flags..
			if_const ( IS_QUERY )
					iCode = CodepointArbitrationQ ( iCode, bWasEscaped, *m_pCur );
			else if ( m_bDetectSentences )
				iCode = CodepointArbitrationI ( iCode );

			// handle ignored chars
			if ( iCode & FLAG_CODEPOINT_IGNORE )
				continue;

			// handle blended characters
			if_const ( IS_BLEND && ( iCode & FLAG_CODEPOINT_BLEND ) )
			{
				if ( m_pBlendEnd )
					iCode = 0;
				else
				{
					m_bBlended = true;
					m_pBlendStart = m_iAccum ? m_pTokenStart : pCur;
				}
			}

			// handle soft-whitespace-only tokens
			if ( !bGotNonToken && !m_iAccum )
			{
				if ( !bGotSoft )
				{
					// detect opening soft whitespace
					if ( ( iCode == 0 && !IsWhitespace ( iCodePoint ) && !IsPunctuation ( iCodePoint ) )
							|| ( ( iCode & FLAG_CODEPOINT_BLEND ) && !m_iAccum ) )
					{
						bGotSoft = true;
					}
				} else
				{
					// detect closing hard whitespace or special
					// (if there was anything meaningful in the meantime, we must never get past the outer if!)
					if ( IsWhitespace ( iCodePoint ) || ( iCode & FLAG_CODEPOINT_SPECIAL ) )
					{
						++m_iOvershortCount;
						bGotNonToken = true;
					}
				}
			}

			// handle whitespace and boundary
			if ( m_bBoundary && ( iCode == 0 ) )
			{
				m_bTokenBoundary = true;
				m_iBoundaryOffset = int ( pCur - m_pBuffer - 1 );
			}
			m_bBoundary = ( iCode & FLAG_CODEPOINT_BOUNDARY ) != 0;

			// handle separator (aka, most likely a token!)
			if ( iCode == 0 || m_bBoundary )
			{
				FlushAccum();

				// suddenly, exceptions
				if ( m_pExc && CheckException ( m_pTokenStart ? m_pTokenStart : pCur, pCur, IS_QUERY ) )
					return m_sAccum;

				if_const ( IS_BLEND && !BlendAdjust ( pCur ) ) continue;

				if ( m_iLastTokenLen < m_tSettings.m_iMinWordLen
						&& !( m_bShortTokenFilter && ShortTokenFilter ( m_sAccum, m_iLastTokenLen ) ) )
				{
					if ( m_iLastTokenLen )
						++m_iOvershortCount;
					continue;
				} else
				{
					m_pTokenEnd = pCur;
					if_const ( IS_BLEND && m_bBlended ) return GetBlendedVariant();
					return m_sAccum;
				}
			}

			// handle specials
			if ( iCode & FLAG_CODEPOINT_SPECIAL )
			{
				// skip short words preceding specials
				if ( m_iAccum < m_tSettings.m_iMinWordLen )
				{
					m_sAccum[m_iAccum] = '\0';

					if ( !m_bShortTokenFilter || !ShortTokenFilter ( m_sAccum, m_iAccum ) )
					{
						if ( m_iAccum )
							m_iOvershortCount++;

						FlushAccum();
					}
				}

				if ( m_iAccum == 0 )
				{
					m_bNonBlended = m_bNonBlended || ( !( iCode & FLAG_CODEPOINT_BLEND ) && !( iCode & FLAG_CODEPOINT_SPECIAL ) );
					m_bWasSpecial = !( iCode & FLAG_CODEPOINT_NGRAM );
					m_pTokenStart = pCur;
					m_pTokenEnd = m_pCur;
					AccumCodepoint ( iCode & MASK_CODEPOINT ); // handle special as a standalone token
				} else
				{
					m_pCur = pCur; // we need to flush current accum and then redo special char again
					m_pTokenEnd = pCur;
				}

				FlushAccum();

				// suddenly, exceptions
				if ( m_pExc && m_pTokenStart && CheckException ( m_pTokenStart, pCur, IS_QUERY ) )
					return m_sAccum;

				if_const ( IS_BLEND )
				{
					if ( !BlendAdjust ( pCur ) )
						continue;
					if ( m_bBlended )
						return GetBlendedVariant();
				}
				return m_sAccum;
			}

			if ( m_iAccum == 0 )
				m_pTokenStart = pCur;

			// tricky bit
			// heading modifiers must not (!) affected blended status
			// eg. we want stuff like '=-' (w/o apostrophes) thrown away when pure_blend is on
			if_const ( IS_BLEND )
				if_const ( !( IS_QUERY && !m_iAccum && sphIsModifier ( iCode & MASK_CODEPOINT ) ) )
					m_bNonBlended = m_bNonBlended || !( iCode & FLAG_CODEPOINT_BLEND );

			// just accumulate
			// manual inlining of utf8 encoder gives us a few extra percent
			// which is important here, this is a hotspot
			if ( m_iAccum < SPH_MAX_WORD_LEN && ( m_pAccum - m_sAccum + SPH_MAX_UTF8_BYTES <= (int)sizeof ( m_sAccum ) ) )
			{
				iCode &= MASK_CODEPOINT;
				m_iAccum++;
				SPH_UTF8_ENCODE ( m_pAccum, iCode );
			}
		}
	}

	void FlushAccum();

public:
	int SkipBlended() final;
};
