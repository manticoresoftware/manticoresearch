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

#include "num_conv.h"

inline void StringBuilder_c::AppendRawChunk ( Str_t sText ) // append without any commas
{
	if ( !sText.second )
		return;

	GrowEnough ( sText.second + 1 ); // +1 because we'll put trailing \0 also

	memcpy ( m_szBuffer + m_iUsed, sText.first, sText.second );
	m_iUsed += sText.second;
	m_szBuffer[m_iUsed] = '\0';
}

inline StringBuilder_c& StringBuilder_c::SkipNextComma()
{
	if ( !m_dDelimiters.IsEmpty() )
		m_dDelimiters.Last().SkipNext();
	return *this;
}

inline StringBuilder_c& StringBuilder_c::AppendName ( const Str_t& sName, bool bQuoted )
{
	if ( ::IsEmpty ( sName ) )
		return *this;

	AppendChunk ( sName, bQuoted ? '"' : '\0' );
	GrowEnough ( 2 );
	m_szBuffer[m_iUsed] = ':';
	m_szBuffer[m_iUsed + 1] = '\0';
	m_iUsed += 1;
	return SkipNextComma();
}

inline StringBuilder_c& StringBuilder_c::AppendName ( const char* szName, bool bQuoted )
{
	return AppendName ( FromSz ( szName ), bQuoted );
}

inline StringBuilder_c& StringBuilder_c::AppendChunk ( const Str_t& sChunk, char cQuote )
{
	if ( !sChunk.second )
		return *this;

	auto sComma = Delim();
	int iQuote = cQuote != 0;

	GrowEnough ( sChunk.second + sComma.second + iQuote + iQuote + 1 ); // +1 because we'll put trailing \0 also

	if ( sComma.second )
		memcpy ( m_szBuffer + m_iUsed, sComma.first, sComma.second );
	if ( iQuote )
		m_szBuffer[m_iUsed + sComma.second] = cQuote;
	memcpy ( m_szBuffer + m_iUsed + sComma.second + iQuote, sChunk.first, sChunk.second );
	m_iUsed += sChunk.second + sComma.second + iQuote + iQuote;
	if ( iQuote )
		m_szBuffer[m_iUsed - 1] = cQuote;
	m_szBuffer[m_iUsed] = '\0';
	return *this;
}

inline StringBuilder_c& StringBuilder_c::AppendString ( const CSphString& sText, char cQuote )
{
	return AppendChunk ( FromStr ( sText ), cQuote );
}

inline StringBuilder_c& StringBuilder_c::AppendString ( const char* szText, char cQuote )
{
	return AppendChunk ( FromSz ( szText ), cQuote );
}

inline StringBuilder_c& StringBuilder_c::AppendString ( Str_t sText, char cQuote )
{
	return AppendChunk ( sText, cQuote );
}

inline StringBuilder_c& StringBuilder_c::operator+= ( const char* sText )
{
	if ( !sText || *sText == '\0' )
		return *this;

	return AppendChunk ( { sText, (int)strlen ( sText ) } );
}

inline StringBuilder_c& StringBuilder_c::operator<< ( const Str_t& sChunk )
{
	return AppendChunk ( sChunk );
}

inline StringBuilder_c& StringBuilder_c::operator<< ( const VecTraits_T<char>& sText )
{
	if ( sText.IsEmpty() )
		return *this;

	return AppendChunk ( { sText.begin(), sText.GetLength() } );
}

inline StringBuilder_c& StringBuilder_c::operator<< ( int iVal )
{
	InitAddPrefix();
	GrowEnough ( 32 );
	m_iUsed += sph::NtoA ( end(), iVal );
	m_szBuffer[m_iUsed] = '\0';
	return *this;
}

inline StringBuilder_c& StringBuilder_c::operator<< ( long iVal )
{
	InitAddPrefix();
	GrowEnough ( 32 );
	m_iUsed += sph::NtoA ( end(), iVal );
	m_szBuffer[m_iUsed] = '\0';
	return *this;
}

inline StringBuilder_c& StringBuilder_c::operator<< ( long long iVal )
{
	InitAddPrefix();
	GrowEnough ( 32 );
	m_iUsed += sph::NtoA ( end(), iVal );
	m_szBuffer[m_iUsed] = '\0';
	return *this;
}

inline StringBuilder_c& StringBuilder_c::operator<< ( unsigned int uVal )
{
	InitAddPrefix();
	GrowEnough ( 32 );
	m_iUsed += sph::NtoA ( end(), uVal );
	m_szBuffer[m_iUsed] = '\0';
	return *this;
}

inline StringBuilder_c& StringBuilder_c::operator<< ( unsigned long uVal )
{
	InitAddPrefix();
	GrowEnough ( 32 );
	m_iUsed += sph::NtoA ( end(), uVal );
	m_szBuffer[m_iUsed] = '\0';
	return *this;
}

inline StringBuilder_c& StringBuilder_c::operator<< ( unsigned long long uVal )
{
	InitAddPrefix();
	GrowEnough ( 32 );
	m_iUsed += sph::NtoA ( end(), uVal );
	m_szBuffer[m_iUsed] = '\0';
	return *this;
}

template<typename T>
StringBuilder_c& StringBuilder_c::operator+= ( const T* pVal )
{
	InitAddPrefix();
	AppendRawChunk ( FROMS ( "0x" ) );
	GrowEnough ( sizeof ( void* ) * 2 );
	m_iUsed += sph::NtoA ( end(), reinterpret_cast<uintptr_t> ( pVal ), 16, sizeof ( void* ) * 2, 0, '0' );
	m_szBuffer[m_iUsed] = '\0';
	return *this;
}

inline StringBuilder_c& StringBuilder_c::operator<< ( bool bVal )
{
	if ( bVal )
		return *this << "true";
	return *this << "false";
}

inline void StringBuilder_c::Clear()
{
	Rewind();
	m_dDelimiters.Reset();
}

inline void StringBuilder_c::Rewind()
{
	if ( m_szBuffer )
		m_szBuffer[0] = '\0';
	m_iUsed = 0;
}


template<typename INT, int iBase, int iWidth, int iPrec, char cFill>
inline void StringBuilder_c::NtoA ( INT uVal )
{
	InitAddPrefix();

	const int MAX_NUMERIC_STR = 22;
	GrowEnough ( MAX_NUMERIC_STR + 1 );

	m_iUsed += sph::NtoA ( end(), uVal, iBase, iWidth, iPrec, cFill );
	m_szBuffer[m_iUsed] = '\0';
}

template<typename INT, int iPrec>
inline void StringBuilder_c::IFtoA ( FixedFrac_T<INT, iPrec> tVal )
{
	InitAddPrefix();

	const int MAX_NUMERIC_STR = 22;
	GrowEnough ( MAX_NUMERIC_STR + 1 );

	int iLen = sph::IFtoA ( end(), tVal.m_tVal, iPrec );
	m_iUsed += iLen;
	m_szBuffer[m_iUsed] = '\0';
}

// shrink, if necessary, to be able to fit at least iLen more chars
inline void StringBuilder_c::GrowEnough ( int iLen )
{
	if ( m_iUsed + iLen<m_iSize )
		return;

	Grow ( iLen );
}


inline void StringBuilder_c::InitAddPrefix()
{
	if ( !m_szBuffer )
		InitBuffer();

	assert ( m_iUsed==0 || m_iUsed<m_iSize );

	auto sPrefix = Delim();
	if ( sPrefix.second ) // prepend delimiter first...
	{
		GrowEnough ( sPrefix.second );
		memcpy ( m_szBuffer + m_iUsed, sPrefix.first, sPrefix.second );
		m_iUsed += sPrefix.second;
	}
}

inline const Str_t & StringBuilder_c::Delim ()
{
	if ( m_dDelimiters.IsEmpty ())
		return dEmptyStr;
	int iLast = m_dDelimiters.GetLength()-1;
	std::function<void()> fnApply = [this, &iLast, &fnApply]()
	{
		--iLast;
		if ( iLast>=0 )
			AppendRawChunk( m_dDelimiters[iLast].RawComma( fnApply ));
	};
	return m_dDelimiters.Last().RawComma( fnApply );
}

inline void StringBuilder_c::LazyComma_c::Swap ( LazyComma_c & rhs ) noexcept
{
	Comma_c::Swap ( rhs );
	m_sPrefix.swap ( rhs.m_sPrefix );
	m_sSuffix.swap ( rhs.m_sSuffix );
	::Swap ( m_bSkipNext, rhs.m_bSkipNext );
}

template<typename... Params>
StringBuilder_c& StringBuilder_c::Sprint ( const Params&... tValues )
{
	(void)std::initializer_list<int> { ( *this << tValues, 0 )... };
	return *this;
}

inline StringBuilder_c& operator<< ( StringBuilder_c& tOut, const CSphNamedInt& tValue )
{
	tOut.Sprintf ( "%s=%d", tValue.first.cstr(), tValue.second );
	return tOut;
}


inline StringBuilder_c& operator<< ( StringBuilder_c& tOut, timespan_t tVal )
{
	tOut.Sprintf ( "%t", tVal.m_iVal );
	return tOut;
}


inline StringBuilder_c& operator<< ( StringBuilder_c& tOut, timestamp_t tVal )
{
	tOut.Sprintf ( "%T", tVal.m_iVal );
	return tOut;
}

template<typename INT, int iPrec>
inline StringBuilder_c& operator<< ( StringBuilder_c& tOut, FixedFrac_T<INT, iPrec>&& tVal )
{
	tOut.template IFtoA<INT, iPrec>(tVal);
	return tOut;
}

template<typename INT, int iBase, int iWidth, int iPrec, char cFill>
StringBuilder_c& operator<< ( StringBuilder_c& tOut, FixedNum_T<INT, iBase, iWidth, iPrec, cFill>&& tVal )
{
	tOut.template NtoA<INT, iBase, iWidth, iPrec, cFill> ( tVal.m_tVal );
	return tOut;
}