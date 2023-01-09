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

#include "sphinxint.h"
#include "icu.h"
#include "tokenizer/charset_definition_parser.h"
#include "tokenizer/tokenizer.h"

#if WITH_ICU

#ifndef U_STATIC_IMPLEMENTATION
#define U_STATIC_IMPLEMENTATION
#endif

#ifndef U_CHARSET_IS_UTF8
#define U_CHARSET_IS_UTF8 1
#endif

#ifndef U_NO_DEFAULT_INCLUDE_UTF_HEADERS
#define U_NO_DEFAULT_INCLUDE_UTF_HEADERS 1
#endif


#include <unicode/brkiter.h>
#include <unicode/udata.h>
#include <unicode/ustring.h>


static CSphString g_sICUDir;

//////////////////////////////////////////////////////////////////////////

static bool g_bICUInitialized = false;

static void ConfigureICU()
{
	if ( g_bICUInitialized )
		return;

	g_sICUDir = GET_ICU_DATA_DIR();

	u_setDataDirectory ( g_sICUDir.cstr() );

	g_bICUInitialized = true;
}

//////////////////////////////////////////////////////////////////////////

class ICUPreprocessor_c
{
public:
	bool					Init ( CSphString & sError );
	bool					Process ( const BYTE * pBuffer, int iLength, CSphVector<BYTE> & dOut, bool bQuery );
	bool					SetBlendChars ( const char * szBlendChars, CSphString & sError );

protected:
	CSphString				m_sBlendChars;

private:
	std::unique_ptr<icu::BreakIterator>	m_pBreakIterator;
	const BYTE *			m_pBuffer {nullptr};
	int						m_iBoundaryIndex {0};
	int						m_iPrevBoundary {0};

	CSphVector<CSphRemapRange> m_dBlendChars;


	void					AddTextChunk ( const BYTE * pStart, int iLen, CSphVector<BYTE> & dOut, bool bChinese, bool bQuery );
	bool					NeedAddSpace ( const BYTE * pToken, const CSphVector<BYTE> & dOut, bool bQuery ) const;

	void					ProcessBufferICU ( const BYTE * pBuffer, int iLength );
	const BYTE *			GetNextTokenICU ( int & iTokenLen );

	bool					IsChineseSeparator ( int iCode ) const;
	bool					IsSpecialQueryCode ( int iCode ) const;
	bool					IsBlendChar ( int iCode ) const;
};


bool ICUPreprocessor_c::Init ( CSphString & sError )
{
	ConfigureICU();

	assert ( !m_pBreakIterator );

	UErrorCode tStatus = U_ZERO_ERROR;
	m_pBreakIterator = std::unique_ptr<icu::BreakIterator> { icu::BreakIterator::createWordInstance ( icu::Locale::getChinese(), tStatus ) };
	if ( U_FAILURE(tStatus) )
	{
		sError.SetSprintf( "Unable to initialize ICU break iterator: %s", u_errorName(tStatus) );
		if ( tStatus==U_MISSING_RESOURCE_ERROR )
			sError.SetSprintf ( "%s. Make sure ICU data file is accessible (using '%s' folder)", sError.cstr(), g_sICUDir.cstr() );

		return false;			
	}

	if ( !m_pBreakIterator )
	{
		sError = "Unable to initialize ICU break iterator";
		return false;
	}

	return true;
}


bool ICUPreprocessor_c::Process ( const BYTE * pBuffer, int iLength, CSphVector<BYTE> & dOut, bool bQuery )
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


bool ICUPreprocessor_c::SetBlendChars ( const char * szBlendChars, CSphString & sError )
{
	m_sBlendChars = szBlendChars;
	return sphParseCharset ( szBlendChars, m_dBlendChars, &sError );
}


bool ICUPreprocessor_c::NeedAddSpace ( const BYTE * pToken, const CSphVector<BYTE> & dOut, bool bQuery ) const
{
	int iResLen = dOut.GetLength();
	if ( !iResLen )
		return false;

	if ( bQuery && ( IsSpecialQueryCode ( dOut[iResLen - 1] ) || IsSpecialQueryCode ( *pToken ) ) )
		return false;

	if ( IsBlendChar( dOut[iResLen - 1] ) || IsBlendChar ( *pToken ) )
		return false;

	return !sphIsSpace ( dOut[iResLen-1] ) && !sphIsSpace ( *pToken );
}


void ICUPreprocessor_c::AddTextChunk ( const BYTE * pStart, int iLen, CSphVector<BYTE> & dOut, bool bChinese, bool bQuery )
{
	if ( !iLen )
		return;

	if ( bChinese )
	{
		ProcessBufferICU ( pStart, iLen );

		const BYTE * pToken;
		int iTokenLen = 0;
		while ( (pToken = GetNextTokenICU(iTokenLen))!=nullptr )
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


void ICUPreprocessor_c::ProcessBufferICU ( const BYTE * pBuffer, int iLength )
{
	assert ( m_pBreakIterator );
	UErrorCode tStatus = U_ZERO_ERROR;
	UText * pUText = nullptr;
	pUText = utext_openUTF8 ( pUText, (const char*)pBuffer, iLength, &tStatus );
	if ( U_FAILURE(tStatus) )
		sphWarning ( "Error processing buffer (ICU): %s", u_errorName(tStatus) );

	assert ( pUText );
	m_pBreakIterator->setText ( pUText, tStatus );
	if ( U_FAILURE(tStatus) )
		sphWarning ( "Error processing buffer (ICU): %s", u_errorName(tStatus) );

	utext_close ( pUText );

	m_pBuffer = pBuffer;
	m_iPrevBoundary = m_iBoundaryIndex = m_pBreakIterator->first();
}


const BYTE * ICUPreprocessor_c::GetNextTokenICU ( int & iTokenLen )
{
	if ( !m_pBreakIterator || m_iBoundaryIndex==icu::BreakIterator::DONE )
		return nullptr;

	while ( ( m_iBoundaryIndex = m_pBreakIterator->next() )!=icu::BreakIterator::DONE )
	{
		int iLength = m_iBoundaryIndex-m_iPrevBoundary;

		// ltrim
		const BYTE * pStart = m_pBuffer+m_iPrevBoundary;
		const BYTE * pMax = pStart + iLength;
		while ( pStart<pMax && sphIsSpace(*pStart) )
			pStart++;

		// rtrim
		while ( pStart<pMax && sphIsSpace(*(pMax-1)) )
			pMax--;

		m_iPrevBoundary = m_iBoundaryIndex;

		if ( pStart!=pMax )
		{
			iTokenLen = int ( pMax-pStart );
			return pStart;
		}
	}

	return nullptr;
}


bool ICUPreprocessor_c::IsChineseSeparator ( int iCode ) const
{
	return ( iCode>=0x3000 && iCode<=0x303F ) || sphIsSpace(iCode);
}


bool ICUPreprocessor_c::IsSpecialQueryCode ( int iCode ) const
{
	return iCode=='!' || iCode=='^' || iCode=='$' || iCode=='*' || iCode=='=';
}


bool ICUPreprocessor_c::IsBlendChar ( int iCode ) const
{
	ARRAY_FOREACH ( i, m_dBlendChars )
		if ( iCode>=m_dBlendChars[i].m_iStart && iCode<=m_dBlendChars[i].m_iEnd )
			return true;

	return false;
}


//////////////////////////////////////////////////////////////////////////
class FieldFilterICU_c : public ISphFieldFilter, public ICUPreprocessor_c
{
	std::unique_ptr<ISphFieldFilter> m_pParent;

public:
	int				Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool bQuery ) final;
	void			GetSettings ( CSphFieldFilterSettings & tSettings ) const final;
	std::unique_ptr<ISphFieldFilter> Clone() const final;
	void SetParent ( std::unique_ptr<ISphFieldFilter> pParent );
};

void FieldFilterICU_c::SetParent ( std::unique_ptr<ISphFieldFilter> pParent )
{
	m_pParent = std::move ( pParent );
}

int FieldFilterICU_c::Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool bQuery )
{
	if ( m_pParent )
	{
		int iResultLength = m_pParent->Apply ( sField, iLength, dStorage, bQuery );
		if ( iResultLength ) // can't use dStorage.GetLength() because of the safety gap
		{
			CSphFixedVector<BYTE> dTmp ( iResultLength );
			memcpy ( dTmp.Begin(), dStorage.Begin(), dStorage.GetLength() );
			if ( !Process ( dTmp.Begin(), iLength, dStorage, bQuery ) )
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

	if ( !Process ( sField, iLength, dStorage, bQuery ) )
		return 0;

	int iStorageLength = dStorage.GetLength();
	*dStorage.AddN(4) = '\0';

	return iStorageLength;
}


void FieldFilterICU_c::GetSettings ( CSphFieldFilterSettings & tSettings ) const
{
	if ( m_pParent )
		m_pParent->GetSettings ( tSettings );
}


std::unique_ptr<ISphFieldFilter> FieldFilterICU_c::Clone() const
{
	std::unique_ptr<ISphFieldFilter> pClonedParent { m_pParent ? m_pParent->Clone () : nullptr };

	CSphString sError;
	auto pFilter = sphCreateFilterICU ( std::move (pClonedParent), m_sBlendChars.cstr(), sError );
	if ( !pFilter )
		sphWarning ( "ICU filter clone error '%s'", sError.cstr() );

	return pFilter;
}

std::unique_ptr<ISphFieldFilter> sphCreateFilterICU ( std::unique_ptr<ISphFieldFilter> pParent, const char * szBlendChars, CSphString & sError )
{
	auto pFilter = std::make_unique<FieldFilterICU_c>();
	if ( !pFilter->Init ( sError ) )
		return pParent;

	if ( szBlendChars && *szBlendChars && !pFilter->SetBlendChars ( szBlendChars, sError ) )
		return pParent;

	pFilter->SetParent ( std::move ( pParent ) );
	return pFilter;
}


bool sphCheckConfigICU ( CSphIndexSettings &, CSphString & )
{
	return true;
}


bool sphCheckTokenizerICU ( CSphIndexSettings & tSettings, const CSphTokenizerSettings & tTokSettings, CSphString & sError )
{
	if ( tSettings.m_ePreprocessor!=Preprocessor_e::ICU )
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
		sError = "ICU CJK segmentation turned on, check that you have CJK characters in charset_table";
		return false;
	}

	return true;
}


bool sphSpawnFilterICU ( std::unique_ptr<ISphFieldFilter> & pFieldFilter, const CSphIndexSettings & m_tSettings, const CSphTokenizerSettings & tTokSettings, const char * szIndex, CSphString & sError )
{
	if ( m_tSettings.m_ePreprocessor==Preprocessor_e::NONE )
		return true;

	auto pFilterICU = sphCreateFilterICU ( std::move ( pFieldFilter ), tTokSettings.m_sBlendChars.cstr(), sError );
	if ( !sError.IsEmpty() )
	{
		sError.SetSprintf ( "table '%s': Error initializing ICU: %s", szIndex, sError.cstr() );
		return false;
	}

	pFieldFilter = std::move ( pFilterICU );
	return true;
}

#else


std::unique_ptr<ISphFieldFilter> sphCreateFilterICU ( std::unique_ptr<ISphFieldFilter> pParent, CSphString & )
{
	return pParent;
}


bool sphCheckConfigICU ( CSphIndexSettings & tSettings, CSphString & sError )
{
	if ( tSettings.m_ePreprocessor!=Preprocessor_e::NONE )
	{
		tSettings.m_ePreprocessor = Preprocessor_e::NONE;
		sError.SetSprintf ( "ICU options specified, but no ICU support compiled; ignoring\n" );
		return false;
	}

	return true;
}


bool sphCheckTokenizerICU ( CSphIndexSettings &, const CSphTokenizerSettings &, CSphString & )
{
	return true;
}


bool sphSpawnFilterICU ( std::unique_ptr<ISphFieldFilter> &, const CSphIndexSettings &, const CSphTokenizerSettings &, const char *, CSphString & )
{
	return true;
}

#endif
