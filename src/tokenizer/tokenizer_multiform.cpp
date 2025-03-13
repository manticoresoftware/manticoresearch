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

#include "charset_definition_parser.h"
#include "sphinxdefs.h"
#include "multiform_container.h"

struct StoredToken_t
{
	BYTE m_sToken[3 * SPH_MAX_WORD_LEN + 4];
	// tokenized state
	const char* m_szTokenStart;
	const char* m_szTokenEnd;
	const char* m_pBufferPtr;
	const char* m_pBufferEnd;
	int m_iTokenLen;
	int m_iOvershortCount;
	bool m_bBoundary;
	bool m_bSpecial;
	bool m_bBlended;
	bool m_bBlendedPart;
};

void FillStoredTokenInfo ( StoredToken_t& tToken, const BYTE* sToken, const TokenizerRefPtr_c& pTokenizer )
{
	assert ( sToken && pTokenizer );
	strncpy ( (char*)tToken.m_sToken, (const char*)sToken, sizeof ( tToken.m_sToken ) - 1 );
	tToken.m_szTokenStart = pTokenizer->GetTokenStart();
	tToken.m_szTokenEnd = pTokenizer->GetTokenEnd();
	tToken.m_iOvershortCount = pTokenizer->GetOvershortCount();
	tToken.m_iTokenLen = pTokenizer->GetLastTokenLen();
	tToken.m_pBufferPtr = pTokenizer->GetBufferPtr();
	tToken.m_pBufferEnd = pTokenizer->GetBufferEnd();
	tToken.m_bBoundary = pTokenizer->GetBoundary();
	tToken.m_bSpecial = pTokenizer->WasTokenSpecial();
	tToken.m_bBlended = pTokenizer->TokenIsBlended();
	tToken.m_bBlendedPart = pTokenizer->TokenIsBlendedPart();
}

/// token filter for multiforms support
class MultiformTokenizer: public CSphTokenFilter
{
	using Base = CSphTokenFilter;

public:
	MultiformTokenizer ( TokenizerRefPtr_c pTokenizer, const CSphMultiformContainer* pContainer );


public:
	void SetBuffer ( const BYTE* sBuffer, int iLength ) final;
	BYTE* GetToken() final;
	void EnableTokenizedMultiformTracking() final
	{
		m_bBuildMultiform = true;
	}
	int GetLastTokenLen() const noexcept final
	{
		return m_iStart < m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_iTokenLen : Base::GetLastTokenLen();
	}
	bool GetBoundary() const noexcept final
	{
		return m_iStart < m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bBoundary : Base::GetBoundary();
	}
	bool WasTokenSpecial() const noexcept final
	{
		return m_iStart < m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bSpecial : Base::WasTokenSpecial();
	}
	int GetOvershortCount() const noexcept final
	{
		return m_iStart < m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_iOvershortCount : Base::GetOvershortCount();
	}
	BYTE* GetTokenizedMultiform() noexcept final
	{
		return m_sTokenizedMultiform[0] ? m_sTokenizedMultiform : nullptr;
	}
	bool TokenIsBlended() const noexcept final
	{
		return m_iStart < m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bBlended : Base::TokenIsBlended();
	}
	bool TokenIsBlendedPart() const noexcept final
	{
		return m_iStart < m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bBlendedPart : Base::TokenIsBlendedPart();
	}
	int SkipBlended() final;

public:
	TokenizerRefPtr_c Clone ( ESphTokenizerClone eMode ) const noexcept final;
	const char* GetTokenStart() const noexcept final
	{
		return m_iStart < m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_szTokenStart : Base::GetTokenStart();
	}
	const char* GetTokenEnd() const noexcept final
	{
		return m_iStart < m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_szTokenEnd : Base::GetTokenEnd();
	}
	const char* GetBufferPtr() const noexcept final
	{
		return m_iStart < m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_pBufferPtr : Base::GetBufferPtr();
	}
	void SetBufferPtr ( const char* sNewPtr ) final;
	uint64_t GetSettingsFNV() const noexcept final;
	bool WasTokenMultiformDestination ( bool& bHead, int& iDestCount ) const noexcept final;

private:
	const CSphMultiformContainer* m_pMultiWordforms;
	int m_iStart = 0;
	int m_iOutputPending = -1;
	const CSphMultiform* m_pCurrentForm = nullptr;

	bool m_bBuildMultiform = false;
	BYTE m_sTokenizedMultiform[3 * SPH_MAX_WORD_LEN + 4];

	CSphVector<StoredToken_t> m_dStoredTokens;
};


//////////////////////////////////////////////////////////////////////////

MultiformTokenizer::MultiformTokenizer ( TokenizerRefPtr_c pTokenizer, const CSphMultiformContainer* pContainer )
	: CSphTokenFilter ( std::move (pTokenizer) )
	, m_pMultiWordforms ( pContainer )
{
	assert ( pContainer );
	m_dStoredTokens.Reserve ( pContainer->m_iMaxTokens + 6 ); // max form tokens + some blended tokens
	m_sTokenizedMultiform[0] = '\0';
}

BYTE* MultiformTokenizer::GetToken()
{
	if ( m_iOutputPending > -1 && m_pCurrentForm )
	{
		if ( ++m_iOutputPending >= m_pCurrentForm->m_dNormalForm.GetLength() )
		{
			m_iOutputPending = -1;
			m_pCurrentForm = nullptr;
		} else
		{
			StoredToken_t& tStart = m_dStoredTokens[m_iStart];
			strncpy ( (char*)tStart.m_sToken, m_pCurrentForm->m_dNormalForm[m_iOutputPending].m_sForm.cstr(), sizeof ( tStart.m_sToken ) - 1 );

			tStart.m_iTokenLen = m_pCurrentForm->m_dNormalForm[m_iOutputPending].m_iLengthCP;
			tStart.m_bBoundary = false;
			tStart.m_bSpecial = false;
			tStart.m_bBlended = false;
			tStart.m_bBlendedPart = false;
			return tStart.m_sToken;
		}
	}

	m_sTokenizedMultiform[0] = '\0';
	m_iStart++;

	if ( m_iStart >= m_dStoredTokens.GetLength() )
	{
		m_iStart = 0;
		m_dStoredTokens.Resize ( 0 );
		const BYTE* pToken = CSphTokenFilter::GetToken();
		if ( !pToken )
			return nullptr;

		FillStoredTokenInfo ( m_dStoredTokens.Add(), pToken, m_pTokenizer );
		while ( m_dStoredTokens.Last().m_bBlended || m_dStoredTokens.Last().m_bBlendedPart )
		{
			pToken = CSphTokenFilter::GetToken();
			if ( !pToken )
				break;

			FillStoredTokenInfo ( m_dStoredTokens.Add(), pToken, m_pTokenizer );
		}
	}

	CSphMultiforms** pWordforms = nullptr;
	int iTokensGot = 1;
	bool bBlended = false;

	// check multi-form
	// only blended parts checked for multi-form with blended
	// in case ALL blended parts got transformed primary blended got replaced by normal form
	// otherwise blended tokens provided as is
	if ( m_dStoredTokens[m_iStart].m_bBlended || m_dStoredTokens[m_iStart].m_bBlendedPart )
	{
		if ( m_dStoredTokens[m_iStart].m_bBlended && m_iStart + 1 < m_dStoredTokens.GetLength() && m_dStoredTokens[m_iStart + 1].m_bBlendedPart )
		{
			pWordforms = m_pMultiWordforms->m_Hash ( (const char*)m_dStoredTokens[m_iStart + 1].m_sToken );
			if ( pWordforms )
			{
				bBlended = true;
				for ( int i = m_iStart + 2; i < m_dStoredTokens.GetLength(); i++ )
				{
					// break out on blended over or got completely different blended
					if ( m_dStoredTokens[i].m_bBlended || !m_dStoredTokens[i].m_bBlendedPart )
						break;

					iTokensGot++;
				}
			}
		}
	} else
	{
		pWordforms = m_pMultiWordforms->m_Hash ( (const char*)m_dStoredTokens[m_iStart].m_sToken );
		if ( pWordforms )
		{
			int iTokensNeed = ( *pWordforms )->m_iMaxTokens + 1;
			int iCur = m_iStart;
			bool bGotBlended = false;

			// collect up ahead to multi-form tokens or all blended tokens or phrase starts or phrase ends
			while ( ( iTokensGot<iTokensNeed || bGotBlended ) && m_dStoredTokens.Last().m_sToken[0]!='"' )
			{
				iCur++;
				if ( iCur >= m_dStoredTokens.GetLength() )
				{
					// fetch next token
					const BYTE* pToken = CSphTokenFilter::GetToken();
					if ( !pToken )
						break;

					FillStoredTokenInfo ( m_dStoredTokens.Add(), pToken, m_pTokenizer );
				}

				bool bCurBleneded = ( m_dStoredTokens[iCur].m_bBlended || m_dStoredTokens[iCur].m_bBlendedPart );
				if ( bGotBlended && !bCurBleneded )
					break;

				bGotBlended = bCurBleneded;
				// count only regular tokens; can not fold mixed (regular+blended) tokens to form
				iTokensGot += ( bGotBlended ? 0 : 1 );
			}
		}
	}

	if ( !pWordforms || iTokensGot < ( *pWordforms )->m_iMinTokens + 1 )
		return m_dStoredTokens[m_iStart].m_sToken;

	int iStartToken = m_iStart + ( bBlended ? 1 : 0 );
	for ( const auto& pCurForm : ( *pWordforms )->m_pForms )
	{
		int iFormTokCount = pCurForm->m_dTokens.GetLength();

		if ( iTokensGot < iFormTokCount + 1 || ( bBlended && iTokensGot != iFormTokCount + 1 ) )
			continue;

		int iForm = 0;
		for ( ; iForm < iFormTokCount; iForm++ )
		{
			const StoredToken_t& tTok = m_dStoredTokens[iStartToken + 1 + iForm];
			const char* szStored = (const char*)tTok.m_sToken;
			const char* szNormal = pCurForm->m_dTokens[iForm].cstr();

			if ( *szNormal != *szStored || strcasecmp ( szNormal, szStored ) )
				break;
		}

		// early out - no destination form detected
		if ( iForm != iFormTokCount )
			continue;

		// tokens after folded form are valid tail that should be processed next time
		if ( m_bBuildMultiform )
		{
			BYTE* pOut = m_sTokenizedMultiform;
			BYTE* pMax = pOut + sizeof ( m_sTokenizedMultiform );
			for ( int j = 0; j < iFormTokCount + 1 && pOut < pMax; j++ )
			{
				const StoredToken_t& tTok = m_dStoredTokens[iStartToken + j];
				const BYTE* sTok = tTok.m_sToken;
				if ( j && pOut < pMax )
					*pOut++ = ' ';
				while ( *sTok && pOut < pMax )
					*pOut++ = *sTok++;
			}
			*pOut = '\0';
			*( pMax - 1 ) = '\0';
		}

		if ( !bBlended )
		{
			// fold regular tokens to form
			const StoredToken_t& tStart = m_dStoredTokens[m_iStart];
			StoredToken_t& tEnd = m_dStoredTokens[m_iStart + iFormTokCount];
			m_iStart += iFormTokCount;

			strncpy ( (char*)tEnd.m_sToken, pCurForm->m_dNormalForm[0].m_sForm.cstr(), sizeof ( tEnd.m_sToken ) - 1 );
			tEnd.m_szTokenStart = tStart.m_szTokenStart;
			tEnd.m_iTokenLen = pCurForm->m_dNormalForm[0].m_iLengthCP;

			tEnd.m_bBoundary = false;
			tEnd.m_bSpecial = false;
			tEnd.m_bBlended = false;
			tEnd.m_bBlendedPart = false;

			if ( pCurForm->m_dNormalForm.GetLength() > 1 )
			{
				m_iOutputPending = 0;
				m_pCurrentForm = pCurForm;
			}
		} else
		{
			// replace blended by form
			// FIXME: add multiple destination token support here (if needed)
			assert ( pCurForm->m_dNormalForm.GetLength() == 1 );
			StoredToken_t& tDst = m_dStoredTokens[m_iStart];
			strncpy ( (char*)tDst.m_sToken, pCurForm->m_dNormalForm[0].m_sForm.cstr(), sizeof ( tDst.m_sToken ) - 1 );
			tDst.m_iTokenLen = pCurForm->m_dNormalForm[0].m_iLengthCP;
		}
		break;
	}

	return m_dStoredTokens[m_iStart].m_sToken;
}


TokenizerRefPtr_c MultiformTokenizer::Clone ( ESphTokenizerClone eMode ) const noexcept
{
	auto pClone = m_pTokenizer->Clone ( eMode );
	Tokenizer::AddToMultiformFilterTo ( pClone, m_pMultiWordforms );
	return pClone;
}


void MultiformTokenizer::SetBufferPtr ( const char* sNewPtr )
{
	m_iStart = 0;
	m_iOutputPending = -1;
	m_pCurrentForm = nullptr;
	m_dStoredTokens.Resize ( 0 );
	CSphTokenFilter::SetBufferPtr ( sNewPtr );
}

void MultiformTokenizer::SetBuffer ( const BYTE* sBuffer, int iLength )
{
	CSphTokenFilter::SetBuffer ( sBuffer, iLength );
	SetBufferPtr ( (const char*)sBuffer );
}

uint64_t MultiformTokenizer::GetSettingsFNV() const noexcept
{
	uint64_t uHash = CSphTokenFilter::GetSettingsFNV();
	uHash ^= (uint64_t)m_pMultiWordforms;
	return uHash;
}

int MultiformTokenizer::SkipBlended()
{
	bool bGotBlended = ( m_iStart < m_dStoredTokens.GetLength() && ( m_dStoredTokens[m_iStart].m_bBlended || m_dStoredTokens[m_iStart].m_bBlendedPart ) );
	if ( !bGotBlended )
		return 0;

	int iWasStart = m_iStart;
	for ( int iTok = m_iStart + 1; iTok < m_dStoredTokens.GetLength() && m_dStoredTokens[iTok].m_bBlendedPart && !m_dStoredTokens[iTok].m_bBlended; iTok++ )
		m_iStart = iTok;

	return m_iStart - iWasStart;
}

bool MultiformTokenizer::WasTokenMultiformDestination ( bool& bHead, int& iDestCount ) const noexcept
{
	if ( m_iOutputPending > -1 && m_pCurrentForm && m_pCurrentForm->m_dNormalForm.GetLength() > 1 && m_iOutputPending < m_pCurrentForm->m_dNormalForm.GetLength() )
	{
		bHead = ( m_iOutputPending == 0 );
		iDestCount = m_pCurrentForm->m_dNormalForm.GetLength();
		return true;
	}
	return false;
}

void Tokenizer::AddToMultiformFilterTo ( TokenizerRefPtr_c& pTokenizer, const CSphMultiformContainer* pContainer )
{
	if ( pContainer )
		pTokenizer = new MultiformTokenizer ( std::move ( pTokenizer ), pContainer );
}