//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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
/// "second_numeric" means that 2nd token must be ASCII digits only
/// "second_has_digit" means that 2nd token must contain at least one ASCII digit
class BigramTokenizer: public CSphTokenFilter
{
protected:
	enum {
		BIGRAM_CLEAN, ///< clean slate, nothing accumulated
		BIGRAM_PAIR_DELIMITED, ///< just returned a delimited pair from m_sBuf, and m_iFirst/m_pSecond are correct
		BIGRAM_PAIR_CONCAT, ///< just returned a concatenated pair from m_sBuf, and m_iFirst/m_pSecond are correct
		BIGRAM_FIRST  ///< just returned a first token from m_sBuf, so m_iFirst/m_pSecond are still good
	} m_eState = BIGRAM_CLEAN;
	BYTE m_sBuf[MAX_KEYWORD_BYTES];							///< pair buffer
	BYTE* m_pSecond = nullptr;								///< second token pointer
	int m_iLenFirst = 0;										///< first token length, bytes
	int m_iLenSecond = 0;										///< second token length, bytes
	bool m_bEmitConcat = false;								///< whether concatenated pair is pending after delimited pair
	CSphRefcountedPtr<const BigramTokenizer> m_pFather; ///< used by clones to share state

	// unchanged state (not need to copy on clone)
	ESphBigram m_eMode;						 ///< bigram indexing mode
	BigramDelimiter_e m_eDelimiter;		 ///< bigram delimiter mode
	BYTE m_uMaxLen = 0;						 ///< max bigram_freq_words length
	CSphFixedVector<int> m_dWordsHash { 0 }; ///< offsets into m_dWords hashed by 1st byte
	CSphVector<BYTE> m_dWords;				 ///< case-folded, sorted bigram_freq_words

public:
	BigramTokenizer ( TokenizerRefPtr_c pTok, ESphBigram eMode, BigramDelimiter_e eDelimiter, StrVec_t& dWords )
		: CSphTokenFilter ( std::move (pTok) )
	{
		assert ( eMode != SPH_BIGRAM_NONE );
		assert ( !BigramNeedsFreq ( eMode ) || dWords.GetLength() );

		m_sBuf[0] = 0;
		m_dWordsHash.Reset ( 256 );
		m_dWordsHash.ZeroVec();

		m_eMode = eMode;
		m_eDelimiter = eDelimiter;

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
		m_eDelimiter = pBase->m_eDelimiter;
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
		if ( m_eState == BIGRAM_PAIR_DELIMITED || m_eState == BIGRAM_PAIR_CONCAT )
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

	bool CanEmitDelimited() const noexcept
	{
		return m_iLenFirst + m_iLenSecond + 1 <= SPH_MAX_WORD_LEN;
	}

	bool CanEmitConcat() const noexcept
	{
		return m_iLenFirst + m_iLenSecond <= SPH_MAX_WORD_LEN;
	}

	BYTE * BuildDelimited()
	{
		m_sBuf[m_iLenFirst] = MAGIC_WORD_BIGRAM;
		assert ( m_iLenFirst + 1 + m_iLenSecond < (int)sizeof ( m_sBuf ) );
		memcpy ( m_sBuf + m_iLenFirst + 1, m_pSecond, m_iLenSecond + 1 );
		return m_sBuf;
	}

	BYTE * BuildConcat()
	{
		assert ( m_iLenFirst + m_iLenSecond < (int)sizeof ( m_sBuf ) );
		memcpy ( m_sBuf + m_iLenFirst, m_pSecond, m_iLenSecond + 1 );
		return m_sBuf;
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

			m_iLenFirst = (int)strlen ( (const char*)pFirst );
			bool bFirstFreq = BigramNeedsFreq ( m_eMode ) && IsFreq ( m_iLenFirst, pFirst );
			if ( m_eMode == SPH_BIGRAM_FIRSTFREQ && !bFirstFreq )
				return pFirst;
			if ( m_eMode == SPH_BIGRAM_BOTHFREQ && !bFirstFreq )
				return pFirst;

			// copy it
			// subsequent calls can and will override token accumulator
			memcpy ( m_sBuf, pFirst, m_iLenFirst + 1 );

			// grow a pair!
			// get a second one (lookahead, in a sense)
			BYTE* pSecond = CSphTokenFilter::GetToken();

			// eof? oi
			if ( !pSecond )
				return m_sBuf;

			// got a pair!
			// check combined length
			m_pSecond = pSecond;
			m_iLenSecond = (int)strlen ( (const char*)pSecond );

			bool bBigram = false;
			switch ( m_eMode )
			{
			case SPH_BIGRAM_ALL:
				bBigram = true;
				break;

			case SPH_BIGRAM_FIRSTFREQ:
				bBigram = bFirstFreq;
				break;

			case SPH_BIGRAM_BOTHFREQ:
				bBigram = bFirstFreq && IsFreq ( m_iLenSecond, m_pSecond );
				break;

			case SPH_BIGRAM_SECONDNUMERIC:
			case SPH_BIGRAM_SECONDHASDIGIT:
				bBigram = BigramIsSecondDigit ( m_eMode, m_pSecond, m_iLenSecond );
				break;

			case SPH_BIGRAM_NONE:
				break;
			}
			if ( !bBigram )
			{
				m_eState = BIGRAM_FIRST;
				return m_sBuf;
			}

			bool bEmitDelimited = m_eDelimiter!=BigramDelimiter_e::NONE && CanEmitDelimited();
			bool bEmitConcat = m_eDelimiter!=BigramDelimiter_e::DELIMITED && CanEmitConcat();
			if ( !bEmitDelimited && !bEmitConcat )
			{
				m_eState = BIGRAM_FIRST;
				return m_sBuf;
			}

			// ok, this is a eligible pair
			// begin with returning the generated pair(s) as blended
			if ( bEmitDelimited )
			{
				m_bEmitConcat = bEmitConcat;
				m_eState = BIGRAM_PAIR_DELIMITED;
				return BuildDelimited();
			}

			m_bEmitConcat = false;
			m_eState = BIGRAM_PAIR_CONCAT;
			return BuildConcat();

		} else if ( m_eState == BIGRAM_PAIR_DELIMITED )
		{
			if ( m_bEmitConcat )
			{
				m_bEmitConcat = false;
				m_eState = BIGRAM_PAIR_CONCAT;
				return BuildConcat();
			}

			// pair (aka bigram) out, return first token as a regular token
			m_eState = BIGRAM_FIRST;
			m_sBuf[m_iLenFirst] = 0;
			return m_sBuf;
		} else if ( m_eState == BIGRAM_PAIR_CONCAT )
		{
			// pair (aka bigram) out, return first token as a regular token
			m_eState = BIGRAM_FIRST;
			m_sBuf[m_iLenFirst] = 0;
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
		uHash = sphFNV64 ( &m_eMode, sizeof(m_eMode), uHash );
		uHash = sphFNV64 ( &m_eDelimiter, sizeof(m_eDelimiter), uHash );
		uHash = sphFNV64 ( m_dWords.Begin(), m_dWords.GetLength(), uHash );
		return uHash;
	}
};


void Tokenizer::AddBigramFilterTo ( TokenizerRefPtr_c& pTokenizer, ESphBigram eBigramIndex, BigramDelimiter_e eBigramDelimiter, const CSphString& sBigramWords, CSphString& sError )
{
	if ( !pTokenizer || eBigramIndex == SPH_BIGRAM_NONE )
		return;

	StrVec_t dFreq;
	if ( BigramNeedsFreq ( eBigramIndex ) )
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
	pTokenizer = new BigramTokenizer ( std::move ( pTokenizer ), eBigramIndex, eBigramDelimiter, dFreq );
}
