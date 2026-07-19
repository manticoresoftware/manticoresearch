//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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
	if ( IsLimitReached() )
		return true;

	GrowEnough ( 1 );
	auto* pCur = end();
	*pCur = '\0';
	return true;
}

template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEmptyQuotes()
{
	char cQuote = Q::cQuote;
	AppendRawChunk ( { &cQuote, 1 } );
	AppendRawChunk ( { &cQuote, 1 } );
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
template<bool BQUOTE>
void EscapedStringBuilder_T<Q>::AppendEscapedSkippingCommaT ( const char* sText )
{
	if constexpr ( BQUOTE )
	{
		if ( AppendEmptyEscaped ( sText ) )
			return;
	} else
	{
		if ( !sText || !*sText )
			return;
	}

	GrowEnough ( BQUOTE ? 7 : 5 ); // 2 quotes, terminator, and possible 4 for long escaping
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	auto fnCanAppend = [this, &pCur] ( int iBytes ) { return GetLimit()<0 || pCur - (BYTE*)m_szBuffer + iBytes <= GetLimit(); };

	if constexpr ( BQUOTE )
		if ( fnCanAppend ( 1 ) )
			*pCur++ = Q::cQuote;
	for ( ; *pSrc; ++pSrc )
	{
		if ( !fnCanAppend ( 1 + Q::EscapingSpace ( *pSrc ) ) )
			break;
		Q::EscapeChar ( pCur, *pSrc );

		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = pCur - (BYTE*)m_szBuffer;
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	if constexpr ( BQUOTE )
		if ( GetLimit()<0 || pCur - (BYTE*)m_szBuffer + 1 <= GetLimit() )
			*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = pCur - (BYTE*)m_szBuffer;
}

template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedSkippingComma ( const char* sText )
{
	AppendEscapedSkippingCommaT ( sText );
}

template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedSkippingCommaNoQuotes ( const char* sText )
{
	AppendEscapedSkippingCommaT<false> ( sText );
}

template<typename Q>
template<bool BQUOTE>
void EscapedStringBuilder_T<Q>::AppendEscapedWithCommaT ( const char* sText )
{
	AppendRawChunk ( Delim() );

	if constexpr ( BQUOTE )
	{
		if ( AppendEmptyEscaped ( sText ) )
			return;
	} else
	{
		if ( !sText || !*sText )
			return;
	}

	GrowEnough ( BQUOTE ? 7 : 5 ); // 2 quotes and terminator
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	auto fnCanAppend = [this, &pCur] ( int iBytes ) { return GetLimit()<0 || pCur - (BYTE*)m_szBuffer + iBytes <= GetLimit(); };

	if constexpr ( BQUOTE )
		if ( fnCanAppend ( 1 ) )
			*pCur++ = Q::cQuote;
	for ( ; *pSrc; ++pSrc )
	{
		if ( !fnCanAppend ( 1 + Q::EscapingSpace ( *pSrc ) ) )
			break;
		Q::EscapeChar ( pCur, *pSrc );

		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = pCur - (BYTE*)m_szBuffer;
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	if constexpr ( BQUOTE )
		if ( GetLimit()<0 || pCur - (BYTE*)m_szBuffer + 1 <= GetLimit() )
			*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = pCur - (BYTE*)m_szBuffer;
}

template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedWithComma ( const char* sText )
{
	AppendEscapedWithCommaT ( sText );
}

template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedWithCommaNoQuotes ( const char* sText )
{
	AppendEscapedWithCommaT<false> ( sText );
}

// dedicated EscBld::eEscape with comma with external len
template<typename Q>
template<bool BQUOTE>
void EscapedStringBuilder_T<Q>::AppendEscapedWithCommaT ( const char* sText, int iLen )
{
	AppendRawChunk ( Delim() );

	if ( !iLen )
	{
		if constexpr ( BQUOTE )
			return AppendEmptyQuotes();
		return;
	}

	GrowEnough ( BQUOTE ? 7 : 5 ); // 2 quotes, terminator, and possible 4 for long escaping
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	auto fnCanAppend = [this, &pCur] ( int iBytes ) { return GetLimit()<0 || pCur - (BYTE*)m_szBuffer + iBytes <= GetLimit(); };

	if constexpr ( BQUOTE )
		if ( fnCanAppend ( 1 ) )
			*pCur++ = Q::cQuote;
	for ( ; iLen; ++pSrc, --iLen )
	{
		if ( !fnCanAppend ( 1 + Q::EscapingSpace ( *pSrc ) ) )
			break;
		Q::EscapeChar ( pCur, *pSrc );

		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = pCur - (BYTE*)m_szBuffer;
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	if constexpr ( BQUOTE )
		if ( GetLimit()<0 || pCur - (BYTE*)m_szBuffer + 1 <= GetLimit() )
			*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = pCur - (BYTE*)m_szBuffer;
}

template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedWithComma ( const char* sText, int iLength )
{
	AppendEscapedWithCommaT ( sText, iLength );
}

template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedWithCommaNoQuotes ( const char* sText, int iLength )
{
	AppendEscapedWithCommaT<false> ( sText, iLength );
}

// dedicated EscBld::eEscape with comma with external len
template<typename Q>
template<bool BQUOTE>
void EscapedStringBuilder_T<Q>::AppendEscapedSkippingCommaT ( const char* sText, int iLen )
{
	if ( !iLen )
	{
		if constexpr ( BQUOTE )
			return AppendEmptyQuotes();
		return;
	}

	GrowEnough ( BQUOTE ? 7 : 5 ); // 2 quotes, terminator, and possible 4 for long escaping
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	auto fnCanAppend = [this, &pCur] ( int iBytes ) { return GetLimit()<0 || pCur - (BYTE*)m_szBuffer + iBytes <= GetLimit(); };

	if constexpr ( BQUOTE )
		if ( fnCanAppend ( 1 ) )
			*pCur++ = Q::cQuote;
	for ( ; iLen; ++pSrc, --iLen )
	{
		if ( !fnCanAppend ( 1 + Q::EscapingSpace ( *pSrc ) ) )
			break;
		Q::EscapeChar ( pCur, *pSrc );

		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = pCur - (BYTE*)m_szBuffer;
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	if constexpr ( BQUOTE )
		if ( GetLimit()<0 || pCur - (BYTE*)m_szBuffer + 1 <= GetLimit() )
			*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = pCur - (BYTE*)m_szBuffer;
}

template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedSkippingComma ( const char* sText, int iLength )
{
	AppendEscapedSkippingCommaT ( sText, iLength );
}

template<typename Q>
void EscapedStringBuilder_T<Q>::AppendEscapedSkippingCommaNoQuotes ( const char* sText, int iLength )
{
	AppendEscapedSkippingCommaT<false> ( sText, iLength );
}

// dedicated EscBld::eFixupSpace
template<typename Q>
void EscapedStringBuilder_T<Q>::FixupSpacesAndAppend ( const char* sText )
{
	if ( AppendEmpty ( sText ) )
		return;

	AppendRawChunk ( Delim() );

	GrowEnough ( 6 ); // terminator
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	auto fnCanAppend = [this, &pCur] ( int iBytes ) { return GetLimit()<0 || pCur - (BYTE*)m_szBuffer + iBytes <= GetLimit(); };

	for ( ; *pSrc; ++pSrc )
	{
		if ( !fnCanAppend ( 1 ) )
			break;
		Q::FixupSpace ( pCur, *pSrc );

		if ( pCur > ( pEnd - 7 ) ) // need terminator
		{
			m_iUsed = pCur - (BYTE*)m_szBuffer;
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	*pCur = '\0';
	m_iUsed = pCur - (BYTE*)m_szBuffer;
}

// dedicated EscBld::eAll (=EscBld::eFixupSpace | EscBld::eEscape )
template<typename Q>
template<bool BQUOTE>
void EscapedStringBuilder_T<Q>::FixupSpacedAndAppendEscapedT ( const char* sText )
{
	AppendRawChunk ( Delim() );

	if constexpr ( BQUOTE )
	{
		if ( AppendEmptyEscaped ( sText ) )
			return;
	} else
	{
		if ( !sText || !*sText )
			return;
	}

	GrowEnough ( BQUOTE ? 7 : 5 ); // 2 quotes, terminator, and possible 4 for long escaping
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	auto fnCanAppend = [this, &pCur] ( int iBytes ) { return GetLimit()<0 || pCur - (BYTE*)m_szBuffer + iBytes <= GetLimit(); };

	if constexpr ( BQUOTE )
		if ( fnCanAppend ( 1 ) )
			*pCur++ = Q::cQuote;
	for ( ; *pSrc; ++pSrc )
	{
		if ( !fnCanAppend ( 1 + Q::EscapingSpace ( *pSrc ) ) )
			break;
		Q::EscapeCharWithSpaces ( pCur, *pSrc );

		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = pCur - (BYTE*)m_szBuffer;
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	if constexpr ( BQUOTE )
		if ( GetLimit()<0 || pCur - (BYTE*)m_szBuffer + 1 <= GetLimit() )
			*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = pCur - (BYTE*)m_szBuffer;
}

template<typename Q>
void EscapedStringBuilder_T<Q>::FixupSpacedAndAppendEscaped ( const char* sText )
{
	FixupSpacedAndAppendEscapedT ( sText );
}

template<typename Q>
void EscapedStringBuilder_T<Q>::FixupSpacedAndAppendEscapedNoQuotes ( const char* sText )
{
	FixupSpacedAndAppendEscapedT<false> ( sText );
}

// dedicated EscBld::eAll (=EscBld::eFixupSpace | EscBld::eEscape ) with external len
template<typename Q>
template<bool BQUOTE>
void EscapedStringBuilder_T<Q>::FixupSpacedAndAppendEscapedT ( const char* sText, int iLen )
{
	assert ( iLen >= 0 );

	AppendRawChunk ( Delim() );

	if ( !iLen )
	{
		if constexpr ( BQUOTE )
			return AppendEmptyQuotes();
		return;
	}

	GrowEnough ( BQUOTE ? 7 : 5 ); // 2 quotes, terminator, and possible 4 for long escaping
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	auto fnCanAppend = [this, &pCur] ( int iBytes ) { return GetLimit()<0 || pCur - (BYTE*)m_szBuffer + iBytes <= GetLimit(); };

	if constexpr ( BQUOTE )
		if ( fnCanAppend ( 1 ) )
			*pCur++ = Q::cQuote;
	for ( ; iLen; ++pSrc, --iLen )
	{
		if ( !fnCanAppend ( 1 + Q::EscapingSpace ( *pSrc ) ) )
			break;
		Q::EscapeCharWithSpaces ( pCur, *pSrc );
		if ( pCur > ( pEnd - 8 ) ) // need 1 ending quote, terminator, and m.b. long escaping
		{
			m_iUsed = pCur - (BYTE*)m_szBuffer;
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	}
	if constexpr ( BQUOTE )
		if ( GetLimit()<0 || pCur - (BYTE*)m_szBuffer + 1 <= GetLimit() )
			*pCur++ = Q::cQuote;
	*pCur = '\0';
	m_iUsed = pCur - (BYTE*)m_szBuffer;
}

template<typename Q>
void EscapedStringBuilder_T<Q>::FixupSpacedAndAppendEscaped ( const char* sText, int iLen )
{
	FixupSpacedAndAppendEscapedT ( sText, iLen );
}

template<typename Q>
void EscapedStringBuilder_T<Q>::FixupSpacedAndAppendEscapedNoQuotes ( const char* sText, int iLen )
{
	FixupSpacedAndAppendEscapedT<false> ( sText, iLen );
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
	case ( EscBld::eEscape | EscBld::eSkipComma | EscBld::eNoLimit | EscBld::eSkipQuotes ):
		AppendEscapedSkippingCommaNoQuotes ( sText );
		return;
	case ( EscBld::eEscape | EscBld::eSkipComma ):
		AppendEscapedSkippingComma ( sText, iLen );
		return;
	case ( EscBld::eEscape | EscBld::eSkipComma | EscBld::eSkipQuotes ):
		AppendEscapedSkippingCommaNoQuotes ( sText, iLen );
		return;
	case ( EscBld::eEscape | EscBld::eNoLimit ):
		AppendEscapedWithComma ( sText );
		return;
	case ( EscBld::eEscape | EscBld::eNoLimit | EscBld::eSkipQuotes ):
		AppendEscapedWithCommaNoQuotes ( sText );
		return;
	case ( EscBld::eEscape ):
		AppendEscapedWithComma ( sText, iLen );
		return;
	case ( EscBld::eEscape | EscBld::eSkipQuotes ):
		AppendEscapedWithCommaNoQuotes ( sText, iLen );
		return;
	case ( EscBld::eFixupSpace | EscBld::eNoLimit ):
	case ( EscBld::eFixupSpace | EscBld::eNoLimit | EscBld::eSkipQuotes ):
		FixupSpacesAndAppend ( sText );
		return;
	case ( EscBld::eAll | EscBld::eNoLimit ):
		FixupSpacedAndAppendEscaped ( sText );
		return;
	case ( EscBld::eAll | EscBld::eNoLimit | EscBld::eSkipQuotes ):
		FixupSpacedAndAppendEscapedNoQuotes ( sText );
		return;
	case ( EscBld::eAll ):
		FixupSpacedAndAppendEscaped ( sText, iLen );
		return;
	case ( EscBld::eAll | EscBld::eSkipQuotes ):
		FixupSpacedAndAppendEscapedNoQuotes ( sText, iLen );
		return;
	}

	if ( ( eWhat & EscBld::eEscape ) == 0 && AppendEmpty ( sText ) )
		return;

	// process comma
	if ( eWhat & EscBld::eSkipComma ) // assert no eEscape here, since it is hold separately already.
		eWhat -= EscBld::eSkipComma;
	else
	{
		AppendRawChunk ( Delim() );
	}

	if ( ( eWhat & EscBld::eEscape ) && AppendEmptyEscaped ( sText ) )
		return;

	const bool bNoLimit = !!( eWhat & EscBld::eNoLimit );
	const bool bEscape = !!( eWhat & EscBld::eEscape );
	const bool bFixupSpace = !!( eWhat & EscBld::eFixupSpace );
	const bool bSkipQuotes = !!( eWhat & EscBld::eSkipQuotes );

	GrowEnough ( bEscape ? 7 : 1 ); // quotes, terminator, and possible 4 for long escaping
	const char* pSrc = sText;
	auto* pCur = (BYTE*)end();
	auto pEnd = (BYTE*)m_szBuffer + m_iSize;

	auto fnCanAppend = [this, &pCur] ( int iBytes ) { return GetLimit()<0 || pCur - (BYTE*)m_szBuffer + iBytes <= GetLimit(); };
	auto fnGrowIfNeeded = [this, &pCur, &pEnd] ( int iTailBytes )
	{
		if ( pCur > pEnd - iTailBytes )
		{
			m_iUsed = pCur - (BYTE*)m_szBuffer;
			GrowEnough ( 32 );
			pEnd = (BYTE*)m_szBuffer + m_iSize;
			pCur = (BYTE*)m_szBuffer + m_iUsed;
		}
	};
	auto fnHasMore = [&pSrc, &iLen, bNoLimit] { return bNoLimit ? *pSrc : iLen; };
	auto fnNext = [&pSrc, &iLen, bNoLimit] { ++pSrc; if ( !bNoLimit ) --iLen; };

	if ( bEscape && !bSkipQuotes && fnCanAppend ( 1 ) )
		*pCur++ = Q::cQuote;

	for ( ; fnHasMore(); fnNext() )
	{
		int iBytes = 1;
		if ( bEscape )
			iBytes += Q::EscapingSpace ( *pSrc );
		if ( !fnCanAppend ( iBytes ) )
			break;

		if ( bEscape && bFixupSpace )
			Q::EscapeCharWithSpaces ( pCur, *pSrc );
		else if ( bEscape )
			Q::EscapeChar ( pCur, *pSrc );
		else if ( bFixupSpace )
			Q::FixupSpace ( pCur, *pSrc );
		else
			*pCur++ = *pSrc;

		fnGrowIfNeeded ( bEscape ? 8 : 7 );
	}

	if ( bEscape && !bSkipQuotes && fnCanAppend ( 1 ) )
		*pCur++ = Q::cQuote;

	*pCur = '\0';
	m_iUsed = pCur - (BYTE*)m_szBuffer;
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
