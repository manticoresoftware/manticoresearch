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

#include "icu.h"

#include "sphinxint.h"
#include "cjkpreprocessor.h"

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

	g_sICUDir = GetICUDataDir();

	u_setDataDirectory ( g_sICUDir.cstr() );

	g_bICUInitialized = true;
}

//////////////////////////////////////////////////////////////////////////

class ICUPreprocessor_c : public CJKPreprocessor_c
{
public:
	bool			Init ( CSphString & sError ) override;
	CJKPreprocessor_c * Clone ( const FieldFilterOptions_t * pOptions ) override { return new ICUPreprocessor_c; }

protected:
	void			ProcessBuffer ( const BYTE * pBuffer, int iLength ) override;
	const BYTE *	GetNextToken ( int & iTokenLen ) override;

private:
	std::unique_ptr<icu::BreakIterator>	m_pBreakIterator;
	const BYTE *	m_pBuffer {nullptr};
	int				m_iBoundaryIndex {0};
	int				m_iPrevBoundary {0};
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


void ICUPreprocessor_c::ProcessBuffer ( const BYTE * pBuffer, int iLength )
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


const BYTE * ICUPreprocessor_c::GetNextToken ( int & iTokenLen )
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

//////////////////////////////////////////////////////////////////////////

bool sphCheckConfigICU ( CSphIndexSettings &, CSphString & )
{
	return true;
}


bool sphSpawnFilterICU ( std::unique_ptr<ISphFieldFilter> & pFieldFilter, const CSphIndexSettings & tSettings, const CSphTokenizerSettings & tTokSettings, const char * szIndex, CSphString & sError )
{
	if ( tSettings.m_ePreprocessor!=Preprocessor_e::ICU )
		return true;

	auto pFilterICU = CreateFilterCJK ( std::move ( pFieldFilter ), std::make_unique<ICUPreprocessor_c>(), tTokSettings.m_sBlendChars.cstr(), sError );
	if ( !sError.IsEmpty() )
	{
		sError.SetSprintf ( "table '%s': Error initializing ICU: %s", szIndex, sError.cstr() );
		return false;
	}

	pFieldFilter = std::move ( pFilterICU );
	return true;
}

#else

bool sphCheckConfigICU ( CSphIndexSettings & tSettings, CSphString & sError )
{
	if ( tSettings.m_ePreprocessor==Preprocessor_e::ICU )
	{
		tSettings.m_ePreprocessor = Preprocessor_e::NONE;
		sError.SetSprintf ( "ICU options specified, but no ICU support compiled; ignoring\n" );
		return false;
	}

	return true;
}


bool sphSpawnFilterICU ( std::unique_ptr<ISphFieldFilter> &, const CSphIndexSettings &, const CSphTokenizerSettings &, const char *, CSphString & )
{
	return true;
}

#endif
