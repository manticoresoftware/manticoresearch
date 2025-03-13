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

#include "cjkpreprocessor.h"

#include "sphinxint.h"
#include "tokenizer/charset_definition_parser.h"
#include "tokenizer/tokenizer.h"


bool CJKPreprocessor_c::Process ( const BYTE * pBuffer, int iLength, CSphVector<BYTE> & dOut, bool bQuery )
{
	if ( !pBuffer || !iLength )
		return false;

	if ( !sphDetectChinese ( pBuffer, iLength ) )
		return false;

	dOut.Resize(0);

	const BYTE * pBufferMax = pBuffer+iLength;

	bool bWasChineseCode = false;
	const BYTE * pChunkStart = pBuffer;
	bool bFirstCode = true;
	while ( pBuffer<pBufferMax )
	{
		const BYTE * pTmp = pBuffer;
		int iCode = sphUTF8Decode ( pBuffer );
		bool bIsChineseCode = sphIsChineseCode(iCode);
		if ( !bFirstCode && bWasChineseCode!=bIsChineseCode )
		{
			AddTextChunk ( pChunkStart, int ( pTmp-pChunkStart ), dOut, bWasChineseCode, bQuery );
			pChunkStart = pTmp;
		}

		bWasChineseCode = bIsChineseCode;
		bFirstCode = false;
	}

	AddTextChunk ( pChunkStart, int ( pBuffer-pChunkStart ), dOut, bWasChineseCode, bQuery );

	return true;
}


bool CJKPreprocessor_c::SetBlendChars ( const char * szBlendChars, CSphString & sError )
{
	return sphParseCharset ( szBlendChars, m_dBlendChars, &sError );
}


void CJKPreprocessor_c::AddTextChunk ( const BYTE * pStart, int iLen, CSphVector<BYTE> & dOut, bool bChinese, bool bQuery )
{
	if ( !iLen )
		return;

	if ( bChinese )
	{
		ProcessBuffer ( pStart, iLen );

		const BYTE * pToken;
		int iTokenLen = 0;
		while ( (pToken = GetNextToken(iTokenLen))!=nullptr )
		{
			bool bAddSpace = NeedAddSpace ( pToken, dOut, bQuery );

			BYTE * pOut = dOut.AddN ( iTokenLen + ( bAddSpace ? 1 : 0 ) );
			if ( bAddSpace )
				*pOut++ = ' ';

			memcpy ( pOut, pToken, iTokenLen );
		}
	}
	else
	{
		bool bAddSpace = NeedAddSpace ( pStart, dOut, bQuery );
		BYTE * pOut = dOut.AddN ( iLen + ( bAddSpace ? 1 : 0 ) );
		if ( bAddSpace )
			*pOut++ = ' ';

		memcpy ( pOut, pStart, iLen );
	}
}

//////////////////////////////////////////////////////////////////////////

FieldFilterCJK_c::FieldFilterCJK_c ( std::unique_ptr<CJKPreprocessor_c> pPreprocessor )
	: m_pPreprocessor ( std::move(pPreprocessor) )
{}


bool FieldFilterCJK_c::Init ( CSphString & sError )
{
	return m_pPreprocessor->Init(sError);
}


int FieldFilterCJK_c::Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool bQuery )
{
	if ( m_pParent )
	{
		int iResultLength = m_pParent->Apply ( sField, iLength, dStorage, bQuery );
		if ( iResultLength ) // can't use dStorage.GetLength() because of the safety gap
		{
			CSphFixedVector<BYTE> dTmp ( iResultLength );
			memcpy ( dTmp.Begin(), dStorage.Begin(), dStorage.GetLength() );
			if ( !m_pPreprocessor->Process ( dTmp.Begin(), iLength, dStorage, bQuery ) )
				return iResultLength;

			// add safety gap
			int iStorageLength = dStorage.GetLength();
			if ( iStorageLength )
			{
				dStorage.Resize ( iStorageLength+4 );
				dStorage[iStorageLength]='\0';
			}

			return iStorageLength;
		}
	}

	if ( !m_pPreprocessor->Process ( sField, iLength, dStorage, bQuery ) )
		return 0;

	int iStorageLength = dStorage.GetLength();
	*dStorage.AddN(4) = '\0';

	return iStorageLength;
}


void FieldFilterCJK_c::GetSettings ( CSphFieldFilterSettings & tSettings ) const
{
	if ( m_pParent )
		m_pParent->GetSettings ( tSettings );
}


std::unique_ptr<ISphFieldFilter> CreateFilterCJK ( std::unique_ptr<ISphFieldFilter> pParent, std::unique_ptr<CJKPreprocessor_c> pPreprocessor, const char * szBlendChars, CSphString & sError )
{
	auto pFilter = std::make_unique<FieldFilterCJK_c>( std::move(pPreprocessor) );
	if ( !pFilter->Init(sError) )
		return pParent;

	if ( szBlendChars && *szBlendChars && !pFilter->SetBlendChars ( szBlendChars, sError ) )
		return pParent;

	pFilter->Setup ( std::move(pParent) );
	return pFilter;
}


std::unique_ptr<ISphFieldFilter> FieldFilterCJK_c::Clone ( const FieldFilterOptions_t * pOptions ) const
{
	std::unique_ptr<ISphFieldFilter> pClonedParent { m_pParent ? m_pParent->Clone(pOptions) : nullptr };
	std::unique_ptr<CJKPreprocessor_c> pClonedPreprocessor { m_pPreprocessor->Clone(pOptions) };

	CSphString sError;
	auto pFilter = CreateFilterCJK ( std::move(pClonedParent), std::move(pClonedPreprocessor), m_sBlendChars.cstr(), sError );
	if ( !pFilter )
		sphWarning ( "ICU filter clone error '%s'", sError.cstr() );

	return pFilter;
}


bool FieldFilterCJK_c::SetBlendChars ( const char * szBlendChars, CSphString & sError )
{
	m_sBlendChars = szBlendChars;
	return m_pPreprocessor->SetBlendChars ( szBlendChars, sError );
}


void FieldFilterCJK_c::Setup ( std::unique_ptr<ISphFieldFilter> pParent )
{
	m_pParent = std::move(pParent);
}

//////////////////////////////////////////////////////////////////////

bool CheckTokenizerCJK ( CSphIndexSettings & tSettings, const CSphTokenizerSettings & tTokSettings, CSphString & sError )
{
	if ( tSettings.m_ePreprocessor==Preprocessor_e::NONE )
		return true;

	StrVec_t dWarnings;
	TokenizerRefPtr_c pTokenizer = Tokenizer::Create ( tTokSettings, nullptr, nullptr, dWarnings, sError );
	if ( !pTokenizer.Ptr() )
		return false;

	const CSphLowercaser & tLC = pTokenizer->GetLowercaser();

	const CharsetAlias_t * pCJKAlias = nullptr;
	for ( const auto & i : GetCharsetAliases() )
		if ( i.m_sName=="cjk" )
			pCJKAlias = &i;

	if ( !pCJKAlias )
		return true;

	int iFound = 0;
	int iTotal = 0;

	for ( const auto & i : pCJKAlias->m_dRemaps )
		for ( int iCode = i.m_iStart; iCode<=i.m_iEnd; iCode++ )
		{
			if ( tLC.ToLower(iCode) )
				iFound++;

			iTotal++;
		}

	float fRatio = float(iFound)/iTotal;
	if ( fRatio < 0.5f )
	{
		sError = "CJK segmentation turned on, check that you have CJK characters in charset_table";
		return false;
	}

	return true;
}
