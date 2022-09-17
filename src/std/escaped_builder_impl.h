//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include <cstring>

template<typename Q>
bool EscapedStringBuilder_T<Q>::AppendEmpty ( const char* sText )
{
	if ( sText && *sText )
		return false;

	GrowEnough ( 1 );
	auto* pCur = end();
	*pCur = '\0';
	return true;
}

template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEmptyQuotes()
{
	GrowEnough ( 3 );
	auto* pCur = end();
	pCur[0] = Q::cQuote;
	pCur[1] = Q::cQuote;
	pCur[2] = '\0';
	m_iUsed += 2;
}

template<typename Q>
bool EscapedStringBuilder_T<Q>::AppendEmptyEscaped ( const char* sText )
{
	if ( sText && *sText )
		return false;
	AppendEmptyQuotes();
	return true;
}

// dedicated EscBld::eEscape | EscBld::eSkipComma
template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedSkippingComma ( const char* sText )
{
	if ( AppendEmptyEscaped ( sText ) )
		return;

	GrowEnough ( 7 ); // 2 quotes, terminator, and possible 4 for long escaping
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	*pCur++ = Q::cQuote;
	for ( ; *pSrc; ++pSrc )
	{
		Q::EscapeChar ( pCur, *pSrc );

		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
}

// dedicated EscBld::eEscape with comma
template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedWithComma ( const char* sText )
{
	auto& sComma = Delim();
	if ( sComma.second )
	{
		GrowEnough ( sComma.second );
		memcpy ( end(), sComma.first, sComma.second );
		m_iUsed += sComma.second;
	}

	if ( AppendEmptyEscaped ( sText ) )
		return;

	GrowEnough ( 7 ); // 2 quotes and terminator
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	*pCur++ = Q::cQuote;
	for ( ; *pSrc; ++pSrc )
	{
		Q::EscapeChar ( pCur, *pSrc );

		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
}

// dedicated EscBld::eEscape with comma with external len
template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedWithComma ( const char* sText, int iLen )
{
	auto& sComma = Delim();
	if ( sComma.second )
	{
		GrowEnough ( sComma.second );
		memcpy ( end(), sComma.first, sComma.second );
		m_iUsed += sComma.second;
	}

	if ( !iLen )
		return AppendEmptyQuotes();

	GrowEnough ( 7 ); // 2 quotes, terminator, and possible 4 for long escaping
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	*pCur++ = Q::cQuote;
	for ( ; iLen; ++pSrc, --iLen )
	{
		Q::EscapeChar ( pCur, *pSrc );

		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
}

// dedicated EscBld::eFixupSpace
template<typename Q>
void EscapedStringBuilder_T<Q>::FixupSpacesAndAppend ( const char* sText )
{
	if ( AppendEmpty ( sText ) )
		return;

	auto& sComma = Delim();
	if ( sComma.second )
	{
		GrowEnough ( sComma.second );
		memcpy ( end(), sComma.first, sComma.second );
		m_iUsed += sComma.second;
	}

	GrowEnough ( 6 ); // terminator
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	for ( ; *pSrc; ++pSrc )
	{
		Q::FixupSpace ( pCur, *pSrc );

		if ( pCur > ( pEnd - 7 ) ) // need terminator
		{
			m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	*pCur = '\0';
	m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
}

// dedicated EscBld::eAll (=EscBld::eFixupSpace | EscBld::eEscape )
template<typename Q>
void EscapedStringBuilder_T<Q>::FixupSpacedAndAppendEscaped ( const char* sText )
{
	auto& sComma = Delim();
	if ( sComma.second )
	{
		GrowEnough ( sComma.second );
		memcpy ( end(), sComma.first, sComma.second );
		m_iUsed += sComma.second;
	}

	if ( AppendEmptyEscaped ( sText ) )
		return;

	GrowEnough ( 7 ); // 2 quotes, terminator, and possible 4 for long escaping
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	*pCur++ = Q::cQuote;
	for ( ; *pSrc; ++pSrc )
	{
		Q::EscapeCharWithSpaces ( pCur, *pSrc );

		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
}

// dedicated EscBld::eAll (=EscBld::eFixupSpace | EscBld::eEscape ) with external len
template<typename Q>
void EscapedStringBuilder_T<Q>::FixupSpacedAndAppendEscaped ( const char* sText, int iLen )
{
	assert ( iLen >= 0 );

	auto& sComma = Delim();
	if ( sComma.second )
	{
		GrowEnough ( sComma.second );
		memcpy ( end(), sComma.first, sComma.second );
		m_iUsed += sComma.second;
	}

	if ( !iLen )
		return AppendEmptyQuotes();

	GrowEnough ( 7 ); // 2 quotes, terminator, and possible 4 for long escaping
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	*pCur++ = Q::cQuote;
	for ( ; iLen; ++pSrc, --iLen )
	{
		Q::EscapeCharWithSpaces ( pCur, *pSrc );
		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = int ( pCur - (BYTE*)m_szBuffer );
}

// generic implementation. Used this way in tests. For best performance consider to use specialized versions
// (see selector switch inside) directly.
template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscaped ( const char* sText, BYTE eWhat, int iLen )
{
	if ( iLen == -1 )
		eWhat |= EscBld::eNoLimit;
	else
		eWhat &= ~EscBld::eNoLimit;

	// shortcuts to dedicated separate cases
	switch ( eWhat )
	{
	case ( EscBld::eEscape | EscBld::eSkipComma | EscBld::eNoLimit ):
		AppendEscapedSkippingComma ( sText );
		return;
	case ( EscBld::eEscape | EscBld::eNoLimit ):
		AppendEscapedWithComma ( sText );
		return;
	case ( EscBld::eEscape ):
		AppendEscapedWithComma ( sText, iLen );
		return;
	case ( EscBld::eFixupSpace | EscBld::eNoLimit ):
		FixupSpacesAndAppend ( sText );
		return;
	case ( EscBld::eAll | EscBld::eNoLimit ):
		FixupSpacedAndAppendEscaped ( sText );
		return;
	case ( EscBld::eAll ):
		FixupSpacedAndAppendEscaped ( sText, iLen );
		return;
	}

	if ( ( eWhat & EscBld::eEscape ) == 0 && AppendEmpty ( sText ) )
		return;

	// process comma
	if ( eWhat & EscBld::eSkipComma ) // assert no eEscape here, since it is hold separately already.
		eWhat -= EscBld::eSkipComma;
	else
	{
		auto sComma = Delim();
		if ( sComma.second )
		{
			GrowEnough ( sComma.second );
			memcpy ( end(), sComma.first, sComma.second );
			m_iUsed += sComma.second;
		}
	}

	if ( ( eWhat & EscBld::eEscape ) && AppendEmptyEscaped ( sText ) )
		return;

	const char* pSrc = sText;
	int iFinalLen = 0;
	if ( eWhat & EscBld::eEscape )
	{
		if ( eWhat & EscBld::eNoLimit )
		{
			eWhat &= ~EscBld::eNoLimit;
			for ( ; *pSrc; ++pSrc )
				iFinalLen += Q::EscapingSpace ( *pSrc );
		} else
		{
			for ( auto iL = 0; iL < iLen; ++pSrc, ++iL )
				iFinalLen += Q::EscapingSpace ( *pSrc );
		}
		iLen = (int)( pSrc - sText );
		iFinalLen += iLen + 2; // 2 quotes: 1 prefix, 2 postfix.
	} else if ( eWhat & EscBld::eNoLimit )
	{
		eWhat &= ~EscBld::eNoLimit;
		iFinalLen = iLen = (int)strlen ( sText );
	} else
		iFinalLen = iLen;

	GrowEnough ( iFinalLen + 1 ); // + zero terminator

	auto* pCur = (BYTE*)end();
	switch ( eWhat )
	{
	case EscBld::eNone:
		memcpy ( pCur, sText, iFinalLen );
		pCur += iFinalLen;
		break;
	case EscBld::eFixupSpace: // EscBld::eNoLimit hold especially
		for ( ; iLen; --iLen )
			Q::FixupSpace ( pCur, *sText++ );
		break;
	case EscBld::eEscape:
		*pCur++ = Q::cQuote;
		for ( ; iLen; --iLen )
			Q::EscapeChar ( pCur, *sText++ );
		*pCur++ = Q::cQuote;
		break;
	case EscBld::eAll:
	default:
		*pCur++ = Q::cQuote;
		for ( ; iLen; --iLen )
			Q::EscapeCharWithSpaces ( pCur, *sText++ );
		*pCur++ = Q::cQuote;
	}
	*pCur = '\0';
	m_iUsed += iFinalLen;
}

template<typename Q>
EscapedStringBuilder_T<Q>& EscapedStringBuilder_T<Q>::SkipNextComma()
{
	StringBuilder_c::SkipNextComma();
	return *this;
}

template<typename Q>
EscapedStringBuilder_T<Q>& EscapedStringBuilder_T<Q>::AppendName ( const char* sName, bool bQuoted )
{
	StringBuilder_c::AppendName ( sName, bQuoted );
	return *this;
}
