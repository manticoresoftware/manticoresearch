//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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
#include <cassert>
#include <cstdio>
#include <cctype>
#include <cstdarg>

#include "generics.h"

inline bool StrEq ( const char* l, const char* r )
{
	if ( !l || !r )
		return ( ( !r && !l ) || ( !r && !*l ) || ( !l && !*r ) );
	return strcmp ( l, r ) == 0;
}

inline bool StrEqN ( const char* l, const char* r )
{
	if ( !l || !r )
		return ( ( !r && !l ) || ( !r && !*l ) || ( !l && !*r ) );
	return strcasecmp ( l, r ) == 0;
}

inline bool StrEq ( Str_t l, const char * r )
{
	if ( IsEmpty ( l ) || !r )
		return ( ( !r && IsEmpty ( l ) ) || ( IsEmpty ( l ) && !*r ) );
	return strncmp ( l.first, r, l.second ) == 0;
}

inline bool StrEqN ( Str_t l, const char* r )
{
	if ( IsEmpty ( l ) || !r )
		return ( ( !r && IsEmpty ( l ) ) || ( IsEmpty ( l ) && !*r ) );
	return strncasecmp ( l.first, r, l.second ) == 0;
}

inline Str_t FromStr ( const CSphString& sString ) noexcept
{
	return { sString.cstr(), (int)sString.Length() };
}

inline void CSphString::SafeFree()
{
	if ( m_sValue != EMPTY )
		SafeDeleteArray ( m_sValue );
}


// take a note this is not an explicit constructor
// so a lot of silent constructing and deleting of strings is possible
// Example:
// SmallStringHash_T<int> hHash;
// ...
// hHash.Exists ( "asdf" ); // implicit CSphString construction and deletion here
inline CSphString::CSphString ( const CSphString& rhs )
{
	if ( !rhs.m_sValue )
		return;
	else if ( rhs.m_sValue[0] == '\0' )
	{
		m_sValue = EMPTY;
	} else
	{
		auto iLen = 1 + (int)strlen ( rhs.m_sValue ) + 1;
		m_sValue = new char[iLen + SAFETY_GAP];

		memcpy ( m_sValue, rhs.m_sValue, iLen ); // NOLINT
		memset ( m_sValue + iLen, 0, SAFETY_GAP );
	}
}

inline CSphString::CSphString ( const char* szString ) // NOLINT
{
	if ( szString )
	{
		if ( szString[0] == '\0' )
		{
			m_sValue = EMPTY;
		} else
		{
			auto iLen = (int)strlen ( szString );
			m_sValue = new char[iLen + SAFETY_GAP + 1];
			memcpy ( m_sValue, szString, iLen ); // NOLINT
			memset ( m_sValue + iLen, 0, SAFETY_GAP + 1 );
		}
	}
}

inline CSphString::CSphString ( const char* szString, guarded_e ) // NOLINT
{
	int iLen = 0;
	if ( szString && szString[0] != '\0')
		iLen = (int)strlen ( szString );
	m_sValue = new char[iLen + SAFETY_GAP + 1];

	if ( iLen )
		memcpy ( m_sValue, szString, iLen ); // NOLINT
	memset ( m_sValue + iLen, 0, SAFETY_GAP + 1 );
}

inline CSphString CSphString::SubString ( int iStart, int iCount ) const
{
#ifndef NDEBUG
	auto iLen = (int)strlen ( m_sValue );
	iCount = Min ( iLen - iStart, iCount );
#endif
	assert ( iStart >= 0 && iStart < iLen );
	assert ( iCount > 0 );
	assert ( ( iStart + iCount ) <= iLen );

	CSphString sRes;
	sRes.m_sValue = new char[1 + SAFETY_GAP + iCount];
	strncpy ( sRes.m_sValue, m_sValue + iStart, iCount );
	memset ( sRes.m_sValue + iCount, 0, 1 + SAFETY_GAP );
	return sRes;
}

// tries to reuse memory buffer, but calls Length() every time
// hope this won't kill performance on a huge strings
inline void CSphString::SetBinary ( const char* sValue, int iLen )
{
	assert ( iLen >= 0 );
	auto iLen_ = size_t ( iLen );
	if ( Length() < ( iLen + SAFETY_GAP + 1 ) )
	{
		SafeFree();
		if ( !sValue )
			m_sValue = EMPTY;
		else
		{
			m_sValue = new char[1 + SAFETY_GAP + iLen];
			memcpy ( m_sValue, sValue, iLen_ );
			memset ( m_sValue + iLen, 0, 1 + SAFETY_GAP );
		}
		return;
	}

	if ( sValue && iLen )
	{
		memcpy ( m_sValue, sValue, iLen_ );
		memset ( m_sValue + iLen, 0, 1 + SAFETY_GAP );
	} else
	{
		SafeFree();
		m_sValue = EMPTY;
	}
}

inline void CSphString::Reserve ( int iLen )
{
	SafeFree();
	m_sValue = new char[1 + SAFETY_GAP + iLen];
	memset ( m_sValue, 0, 1 + SAFETY_GAP + iLen );
}

inline const CSphString& CSphString::SetSprintf ( const char* sTemplate, ... )
{
	char sBuf[1024];
	va_list ap;

	va_start ( ap, sTemplate );
	vsnprintf ( sBuf, sizeof ( sBuf ), sTemplate, ap );
	va_end ( ap );

	( *this ) = sBuf;
	return ( *this );
}

/// format value using provided va_list
inline const CSphString& CSphString::SetSprintfVa ( const char* sTemplate, va_list ap )
{
	char sBuf[1024];
	vsnprintf ( sBuf, sizeof ( sBuf ), sTemplate, ap );

	( *this ) = sBuf;
	return ( *this );
}
/// \return true if internal char* ptr is null, of value is empty.
inline bool CSphString::IsEmpty() const
{
	if ( !m_sValue )
		return true;
	return ( ( *m_sValue ) == '\0' );
}

inline CSphString& CSphString::ToLower()
{
	if ( m_sValue )
		for ( char* s = m_sValue; *s; ++s )
			*s = (char)tolower ( *s );
	return *this;
}

inline CSphString& CSphString::ToUpper()
{
	if ( m_sValue )
		for ( char* s = m_sValue; *s; s++ )
			*s = (char)toupper ( *s );
	return *this;
}

inline void CSphString::Swap ( CSphString& rhs )
{
	::Swap ( m_sValue, rhs.m_sValue );
}

/// \return true if the string begins with sPrefix
inline bool CSphString::Begins ( const char* sPrefix ) const
{
	if ( !m_sValue || !sPrefix )
		return false;
	return strncmp ( m_sValue, sPrefix, strlen ( sPrefix ) ) == 0;
}

/// \return true if the string ends with sSuffix
inline bool CSphString::Ends ( const char* sSuffix ) const
{
	if ( !m_sValue || !sSuffix )
		return false;

	auto iVal = (int)strlen ( m_sValue );
	auto iSuffix = (int)strlen ( sSuffix );
	if ( iVal < iSuffix )
		return false;
	return strncmp ( m_sValue + iVal - iSuffix, sSuffix, iSuffix ) == 0;
}

/// trim leading and trailing spaces
inline CSphString& CSphString::Trim()
{
	if ( m_sValue )
	{
		const char* sStart = m_sValue;
		const char* sEnd = m_sValue + strlen ( m_sValue ) - 1;
		while ( sStart <= sEnd && isspace ( (unsigned char)*sStart ) )
			sStart++;
		while ( sStart <= sEnd && isspace ( (unsigned char)*sEnd ) )
			sEnd--;
		memmove ( m_sValue, sStart, sEnd - sStart + 1 );
		m_sValue[sEnd - sStart + 1] = '\0';
	}

	return *this;
}

inline int CSphString::Length() const
{
	return m_sValue ? (int)strlen ( m_sValue ) : 0;
}

/// \return internal string and releases it from being destroyed in d-tr
inline char* CSphString::Leak()
{
	if ( m_sValue == EMPTY )
	{
		m_sValue = nullptr;
		auto* pBuf = new char[1];
		pBuf[0] = '\0';
		return pBuf;
	}
	char* pBuf = m_sValue;
	m_sValue = nullptr;
	return pBuf;
}

/// internal string and releases it from being destroyed in d-tr
inline void CSphString::LeakToVec ( CSphVector<BYTE>& dVec )
{
	if ( m_sValue == EMPTY )
	{
		m_sValue = nullptr;
		auto* pBuf = new char[1];
		pBuf[0] = '\0';
		dVec.AdoptData ( (BYTE*)pBuf, 0, 1 );
		return;
	}
	int iLen = Length();
	dVec.AdoptData ( (BYTE*)m_sValue, iLen, iLen + 1 + SAFETY_GAP );
	m_sValue = nullptr;
}

/// take string from outside and 'adopt' it as own child.
inline void CSphString::Adopt ( char** sValue )
{
	SafeFree();
	m_sValue = *sValue;
	*sValue = nullptr;
}

inline void CSphString::Adopt ( char*&& sValue )
{
	SafeFree();
	m_sValue = sValue;
	sValue = nullptr;
}

/// compares using strcmp
inline bool CSphString::operator<( const CSphString& b ) const
{
	if ( !m_sValue && !b.m_sValue )
		return false;
	if ( !m_sValue || !b.m_sValue )
		return !m_sValue;
	return strcmp ( m_sValue, b.m_sValue ) < 0;
}

inline void CSphString::Unquote()
{
	int l = Length();
	if ( l && m_sValue[0] == '\'' && m_sValue[l - 1] == '\'' )
	{
		memmove ( m_sValue, m_sValue + 1, l - 2 );
		m_sValue[l - 2] = '\0';
	}
}

inline CSphString::operator ByteBlob_t() const
{
	return { (const BYTE*)m_sValue, Length() };
}


/// string swapper
inline void Swap ( CSphString& v1, CSphString& v2 )
{
	v1.Swap ( v2 );
}

/// directly make formatted string
inline CSphString SphSprintfVa ( const char* sTemplate, va_list ap )
{
	CSphString sResult;
	sResult.SetSprintfVa ( sTemplate, ap );
	return sResult;
}

inline CSphString SphSprintf ( const char* sTemplate, ... )
{
	CSphString sResult;

	va_list ap;
	va_start ( ap, sTemplate );
	sResult.SetSprintfVa ( sTemplate, ap );
	va_end ( ap );

	return sResult;
}