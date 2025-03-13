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

#include "token_filter.h"

#include "sphinxstd.h"
#include "sphinxdefs.h"
#include "sphinxint.h"


/// token filter for bigram indexing
///
/// passes tokens through until an eligible pair is found
/// then buffers and returns that pair as a blended token
/// then returns the first token as a regular one
/// then pops the first one and cycles again
///
/// pair (aka bigram) eligibility depends on bigram_index value
/// "all" means that all token pairs gets indexed
/// "first_freq" means that 1st token must be from bigram_freq_words
/// "both_freq" means that both tokens must be from bigram_freq_words
class BigramTokenizer: public CSphTokenFilter
{
protected:
	enum {
		BIGRAM_CLEAN, ///< clean slate, nothing accumulated
		BIGRAM_PAIR,  ///< just returned a pair from m_sBuf, and m_iFirst/m_pSecond are correct
		BIGRAM_FIRST  ///< just returned a first token from m_sBuf, so m_iFirst/m_pSecond are still good
	} m_eState = BIGRAM_CLEAN;
	BYTE m_sBuf[MAX_KEYWORD_BYTES];							///< pair buffer
	BYTE* m_pSecond = nullptr;								///< second token pointer
	int m_iFirst = 0;										///< first token length, bytes
	CSphRefcountedPtr<const BigramTokenizer> m_pFather; ///< used by clones to share state

	// unchanged state (not need to copy on clone)
	ESphBigram m_eMode;						 ///< bigram indexing mode
	BYTE m_uMaxLen = 0;						 ///< max bigram_freq_words length
	CSphFixedVector<int> m_dWordsHash { 0 }; ///< offsets into m_dWords hashed by 1st byte
	CSphVector<BYTE> m_dWords;				 ///< case-folded, sorted bigram_freq_words

public:
	BigramTokenizer ( TokenizerRefPtr_c pTok, ESphBigram eMode, StrVec_t& dWords )
		: CSphTokenFilter ( std::move (pTok) )
	{
		assert ( eMode != SPH_BIGRAM_NONE );
		assert ( eMode == SPH_BIGRAM_ALL || dWords.GetLength() );

		m_sBuf[0] = 0;
		m_dWordsHash.Reset ( 256 );
		m_dWordsHash.ZeroVec();

		m_eMode = eMode;

		// only keep unique, real, short enough words
		dWords.Uniq();
		for ( const auto& sWord : dWords )
		{
			int iLen = Min ( sWord.Length(), (int)255 );
			if ( !iLen )
				continue;

			m_uMaxLen = Max ( m_uMaxLen, (BYTE)iLen );

			// hash word blocks by the first letter
			BYTE uFirst = *(BYTE*)const_cast<char*> ( sWord.cstr() );
			if ( !m_dWordsHash[uFirst] )
			{
				m_dWords.Add ( 0 );							 // end marker for the previous block
				m_dWordsHash[uFirst] = m_dWords.GetLength(); // hash new block
			}

			// store that word
			m_dWords.ReserveGap ( iLen + 1 );
			m_dWords.Add ( BYTE ( iLen ) );
			m_dWords.Append ( sWord.cstr(), iLen );
		}
		m_dWords.Add ( 0 );
	}

	BigramTokenizer ( TokenizerRefPtr_c pTok, const BigramTokenizer* pBase )
		: CSphTokenFilter ( std::move (pTok) )
	{
		m_sBuf[0] = 0;
		m_eMode = pBase->m_eMode;
		m_uMaxLen = pBase->m_uMaxLen;
		if ( pBase->m_pFather )
			m_pFather = pBase->m_pFather;
		else
		{
			pBase->AddRef();
			m_pFather = pBase;
		}
	}

	TokenizerRefPtr_c Clone ( ESphTokenizerClone eMode ) const noexcept final
	{
		return TokenizerRefPtr_c { new BigramTokenizer ( m_pTokenizer->Clone ( eMode ), this ) };
	}


	bool TokenIsBlended() const noexcept final
	{
		if ( m_eState == BIGRAM_PAIR )
			return true;
		if ( m_eState == BIGRAM_FIRST )
			return false;
		return m_pTokenizer->TokenIsBlended();
	}

	bool IsFreq ( int iLen, BYTE* sWord ) const
	{
		// early check
		if ( iLen > m_uMaxLen )
			return false;

		if ( m_pFather )
			return m_pFather->IsFreq ( iLen, sWord );

		// hash lookup, then linear scan
		int iPos = m_dWordsHash[*sWord];
		if ( !iPos )
			return false;
		while ( m_dWords[iPos] )
		{
			if ( m_dWords[iPos] == iLen && !memcmp ( sWord, &m_dWords[iPos + 1], iLen ) )
				break;
			iPos += 1 + m_dWords[iPos];
		}
		return m_dWords[iPos] != 0;
	}

	BYTE* GetToken() final
	{
		if ( m_eState == BIGRAM_FIRST || m_eState == BIGRAM_CLEAN )
		{
			BYTE* pFirst;
			if ( m_eState == BIGRAM_FIRST )
			{
				// first out, clean slate again, actually
				// and second will now become our next first
				assert ( m_pSecond );
				m_eState = BIGRAM_CLEAN;
				pFirst = m_pSecond;
				m_pSecond = nullptr;
			} else
			{
				// just clean slate
				// assure we're, well, clean
				assert ( !m_pSecond );
				pFirst = CSphTokenFilter::GetToken();
			}

			// clean slate
			// get first non-blended token
			if ( !pFirst )
				return nullptr;

			// pass through blended
			// could handle them as first too, but.. cumbersome
			if ( CSphTokenFilter::TokenIsBlended() )
				return pFirst;

			// check pair
			// in first_freq and both_freq modes, 1st token must be listed
			m_iFirst = (int)strlen ( (const char*)pFirst );
			if ( m_eMode != SPH_BIGRAM_ALL && !IsFreq ( m_iFirst, pFirst ) )
				return pFirst;

			// copy it
			// subsequent calls can and will override token accumulator
			memcpy ( m_sBuf, pFirst, m_iFirst + 1 );

			// grow a pair!
			// get a second one (lookahead, in a sense)
			BYTE* pSecond = CSphTokenFilter::GetToken();

			// eof? oi
			if ( !pSecond )
				return m_sBuf;

			// got a pair!
			// check combined length
			m_pSecond = pSecond;
			auto iSecond = (int)strlen ( (const char*)pSecond );
			if ( m_iFirst + iSecond + 1 > SPH_MAX_WORD_LEN )
			{
				// too long pair
				// return first token as is
				m_eState = BIGRAM_FIRST;
				return m_sBuf;
			}

			// check pair
			// in freq2 mode, both tokens must be listed
			if ( m_eMode == SPH_BIGRAM_BOTHFREQ && !IsFreq ( iSecond, m_pSecond ) )
			{
				m_eState = BIGRAM_FIRST;
				return m_sBuf;
			}

			// ok, this is a eligible pair
			// begin with returning first+second pair (as blended)
			m_eState = BIGRAM_PAIR;
			m_sBuf[m_iFirst] = MAGIC_WORD_BIGRAM;
			assert ( m_iFirst + strlen ( (const char*)pSecond ) < sizeof ( m_sBuf ) );
			strcpy ( (char*)m_sBuf + m_iFirst + 1, (const char*)pSecond ); // NOLINT
			return m_sBuf;

		} else if ( m_eState == BIGRAM_PAIR )
		{
			// pair (aka bigram) out, return first token as a regular token
			m_eState = BIGRAM_FIRST;
			m_sBuf[m_iFirst] = 0;
			return m_sBuf;
		}

		assert ( 0 && "unhandled bigram tokenizer internal state" );
		return nullptr;
	}

	uint64_t GetSettingsFNV() const noexcept final
	{
		if ( m_pFather )
			return m_pFather->GetSettingsFNV();
		uint64_t uHash = CSphTokenFilter::GetSettingsFNV();
		uHash = sphFNV64 ( m_dWords.Begin(), m_dWords.GetLength(), uHash );
		return uHash;
	}
};


void Tokenizer::AddBigramFilterTo ( TokenizerRefPtr_c& pTokenizer, ESphBigram eBigramIndex, const CSphString& sBigramWords, CSphString& sError )
{
	if ( !pTokenizer || eBigramIndex == SPH_BIGRAM_NONE )
		return;

	StrVec_t dFreq;
	if ( eBigramIndex != SPH_BIGRAM_ALL )
	{
		const BYTE* pTok;
		pTokenizer->SetBuffer ( (const BYTE*)sBigramWords.cstr(), sBigramWords.Length() );
		while ( ( pTok = pTokenizer->GetToken() ) )
			dFreq.Add ( (const char*)pTok );

		if ( dFreq.IsEmpty() )
		{
			sError.SetSprintf ( "bigram_freq_words does not contain any valid words" );
			return;
		}
	}
	pTokenizer = new BigramTokenizer ( std::move ( pTokenizer ), eBigramIndex, dFreq );
}
