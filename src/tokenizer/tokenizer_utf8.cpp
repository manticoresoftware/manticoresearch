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

const char* SPHINX_DEFAULT_UTF8_TABLE = "0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451";

/// UTF-8 tokenizer
class Tokenizer_UTF8_Base_c: public CSphTokenizerBase2
{
public:
	explicit Tokenizer_UTF8_Base_c ( bool bDefaultCharset );
	void SetBuffer ( const BYTE* sBuffer, int iLength ) final;
	int GetCodepointLength ( int iCode ) const final;
	int GetMaxCodepointLength() const final
	{
		return GetLowercaser().GetMaxCodepointLength();
	}
};

Tokenizer_UTF8_Base_c::Tokenizer_UTF8_Base_c ( bool bDefaultCharset )
{
	if ( bDefaultCharset )
	{
		CSphString sTmp;
		SetCaseFolding ( SPHINX_DEFAULT_UTF8_TABLE, sTmp );
	}
	m_bHasBlend = false;
}


void Tokenizer_UTF8_Base_c::SetBuffer ( const BYTE* sBuffer, int iLength )
{
	// check that old one is over and that new length is sane
	assert ( iLength >= 0 );

	// set buffer
	m_pBuffer = sBuffer;
	m_pBufferMax = sBuffer + iLength;
	m_pCur = sBuffer;
	m_pTokenStart = m_pTokenEnd = nullptr;
	m_pBlendStart = m_pBlendEnd = nullptr;

	m_iOvershortCount = 0;
	m_bBoundary = m_bTokenBoundary = false;
}

int Tokenizer_UTF8_Base_c::GetCodepointLength ( int iCode ) const
{
	if ( iCode < 128 )
		return 1;

	int iBytes = 0;
	while ( iCode & 0x80 )
	{
		iBytes++;
		iCode <<= 1;
	}

	assert ( iBytes >= 2 && iBytes <= 4 );
	return iBytes;
}

template<bool IS_QUERY>
class CSphTokenizer_UTF8: public Tokenizer_UTF8_Base_c
{
public:
	explicit CSphTokenizer_UTF8 ( bool bDefaultCharset )
		: Tokenizer_UTF8_Base_c ( bDefaultCharset )
	{}
	BYTE* GetToken() override;
	TokenizerRefPtr_c Clone ( ESphTokenizerClone eMode ) const final;
};


template<bool IS_QUERY>
BYTE* CSphTokenizer_UTF8<IS_QUERY>::GetToken()
{
	m_bWasSpecial = false;
	m_bBlended = false;
	m_iOvershortCount = 0;
	m_bTokenBoundary = false;
	m_bWasSynonym = false;

	return m_bHasBlend
				 ? DoGetToken<IS_QUERY, true>()
				 : DoGetToken<IS_QUERY, false>();
}

template<bool IS_QUERY>
TokenizerRefPtr_c CSphTokenizer_UTF8<IS_QUERY>::Clone ( ESphTokenizerClone eMode ) const
{
	CSphTokenizerBase* pClone;
	if ( eMode != SPH_CLONE_INDEX )
		pClone = new CSphTokenizer_UTF8<true> ( false );
	else
		pClone = new CSphTokenizer_UTF8<false> ( false );
	pClone->CloneBase ( this, eMode );
	return TokenizerRefPtr_c {pClone};
}

/// UTF-8 tokenizer with n-grams
/////////////////////////////////////////////////////////////////////////////

template<bool IS_QUERY>
class CSphTokenizer_UTF8Ngram: public CSphTokenizer_UTF8<IS_QUERY>
{
public:
	explicit CSphTokenizer_UTF8Ngram ( bool bDefaultCharset )
		: CSphTokenizer_UTF8<IS_QUERY> (bDefaultCharset) {}
	bool SetNgramChars ( const char* sConfig, CSphString& sError ) final;
	void SetNgramLen ( int iLen ) final;
	BYTE* GetToken() final;

protected:
	int m_iNgramLen = 1;
};

template<bool IS_QUERY>
bool CSphTokenizer_UTF8Ngram<IS_QUERY>::SetNgramChars ( const char* sConfig, CSphString& sError )
{
	return ISphTokenizer::RemapCharacters ( sConfig, FLAG_CODEPOINT_NGRAM | FLAG_CODEPOINT_SPECIAL, "ngram", true, sError ); // !COMMIT support other n-gram lengths than 1
}


template<bool IS_QUERY>
void CSphTokenizer_UTF8Ngram<IS_QUERY>::SetNgramLen ( int iLen )
{
	assert ( iLen > 0 );
	m_iNgramLen = iLen;
}


template<bool IS_QUERY>
BYTE* CSphTokenizer_UTF8Ngram<IS_QUERY>::GetToken()
{
	// !COMMIT support other n-gram lengths than 1
	assert ( m_iNgramLen == 1 );
	return CSphTokenizer_UTF8<IS_QUERY>::GetToken();
}

TokenizerRefPtr_c Tokenizer::Detail::CreateUTF8Tokenizer ( bool bDefaultCharset )
{
	return TokenizerRefPtr_c { new CSphTokenizer_UTF8<false> ( bDefaultCharset ) };
}

TokenizerRefPtr_c Tokenizer::Detail::CreateUTF8NgramTokenizer ( bool bDefaultCharset )
{
	return TokenizerRefPtr_c { new CSphTokenizer_UTF8Ngram<false> ( bDefaultCharset ) };
}
