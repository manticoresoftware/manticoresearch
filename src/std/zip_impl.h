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

#include <type_traits>

// N of bytes need to store value
template<typename T>
inline int sphCalcZippedLen ( T tValue )
{
	auto uValue = static_cast<std::make_unsigned_t<T>> ( tValue );
	int nBytes = 1;
	uValue >>= 7;
	while ( uValue )
	{
		uValue >>= 7;
		++nBytes;
	}

	return nBytes;
	// (sphLog2(tValue)+6)/7 gives no branching, but stalled-cycles-frontend because of 'bsr' instruction. It works faster standalone, but slower in ZipValue
}

// big-endian (most significant septets first)
template<typename T, typename WRITER>
inline int ZipValueBE ( WRITER fnPut, T tValue )
{
	int nBytes = sphCalcZippedLen ( tValue );
	for ( int i = nBytes - 1; i >= 0; --i )
		fnPut ( ( 0x7f & ( tValue >> ( 7 * i ) ) ) | ( i ? 0x80 : 0 ) );

	return nBytes;
}

// big-endian (most significant septets first)
template<typename T>
inline int ZipToPtrBE ( BYTE* pData, T tValue )
{
	return ZipValueBE ( [pData] ( BYTE b ) mutable { *pData++ = b; }, tValue );
}

template<typename T, typename READER>
inline T UnzipValueBE_reference ( READER fnGet )
{
#if PARANOID
	DWORD b = 0;
	T v = 0;
	int it = 0;
	do {
		b = fnGet();
		v = ( v << 7 ) + ( b & 0x7f );
		it++;
	} while ( b & 0x80 );
	assert ( ( it - 1 ) * 7 <= sizeof ( _type ) * 8 );
	return v;
#else
	BYTE b = fnGet();
	T res = 0;
	while ( b & 0x80 )
	{
		res = ( res << 7 ) | ( b & 0x7f );
		b = fnGet();
	}
	res = ( res << 7 ) | b;
	return res;
#endif
}

template<typename T, typename READER>
inline std::enable_if_t<sizeof ( T ) == 4, T> UnzipValueBE ( READER fnGet )
{
	BYTE b = fnGet();
	DWORD res = b & 0x7f;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res = ( res << 7 ) | ( b & 0x7f );
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res = ( res << 7 ) | ( b & 0x7f );
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res = ( res << 7 ) | ( b & 0x7f );
	if ( ( b & 0x80 ) == 0 )
		return res;
	return ( res << 7 ) | fnGet();
}

// reference implementation
template<typename T, typename READER>
inline std::enable_if_t<sizeof ( T ) == 8, T> UnzipValueBE ( READER fnGet )
{
#if PARANOID
	DWORD b = 0;
	T v = 0;
	int it = 0;
	do {
		b = fnGet();
		v = ( v << 7 ) + ( b & 0x7f );
		it++;
	} while ( b & 0x80 );
	assert ( ( it - 1 ) * 7 <= sizeof ( _type ) * 8 );
	return v;
#else
	BYTE b = fnGet();
	T res = 0;
	while ( b & 0x80 )
	{
		res = ( res << 7 ) | ( b & 0x7f );
		b = fnGet();
	}
	res = ( res << 7 ) | b;
	return res;
#endif
}

inline DWORD UnzipIntBE ( const BYTE*& pBuf )
{
	return UnzipValueBE<DWORD> ( [&pBuf]() mutable { return *pBuf++; } );
}

inline SphOffset_t UnzipOffsetBE ( const BYTE*& pBuf )
{
	return UnzipValueBE<SphOffset_t> ( [&pBuf]() mutable { return *pBuf++; } );
}


template<typename T, typename WRITER>
inline int ZipValueLE ( WRITER fnPut, T tValue )
{
	auto uValue = static_cast<std::make_unsigned_t<T>> ( tValue );
	int nBytes = 0;
	do
	{
		BYTE bOut = (BYTE)( uValue & 0x7f );
		uValue >>= 7;
		if ( uValue )
			bOut |= 0x80;
		fnPut ( bOut );
		++nBytes;
	} while ( uValue );
	return nBytes;
}

template<typename T>
inline int ZipToPtrLE ( BYTE* pData, T tValue )
{
	return ZipValueLE ( [pData] ( BYTE b ) mutable { *pData++ = b; }, tValue );
}

template<typename T, typename READER>
inline T UnzipValueLE_reference ( READER fnGet )
{
	BYTE b;
	T res = 0;
	BYTE iOff = 0;

	do
	{
		b = fnGet();
		res += ( T ( b & 0x7FU ) ) << iOff;
		iOff += 7;
	} while ( b & 0x80U );

	return res;
}

template<typename T, typename READER>
inline std::enable_if_t<sizeof ( T ) == 4, T> UnzipValueLE ( READER fnGet )
{
	BYTE b = fnGet();
	DWORD res = b & 0x7f;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res |= ( b & 0x7f ) << 7;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res |= ( b & 0x7f ) << 14;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	if ( ( b & 0x80 ) == 0 )
		return res | ( b & 0x7f ) << 21;
	return res | ( b & 0x7f ) << 21 | fnGet() << 28;
}

template<typename T, typename READER>
inline std::enable_if_t<sizeof ( T ) == 8, T> UnzipValueLE ( READER fnGet )
{
	BYTE b = fnGet();
	T res = b & 0x7f;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res |= T ( b & 0x7f ) << 7;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res |= T ( b & 0x7f ) << 14;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res |= T ( b & 0x7f ) << 21;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res |= T ( b & 0x7f ) << 28;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res |= T ( b & 0x7f ) << 35;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res |= T ( b & 0x7f ) << 42;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	res |= T ( b & 0x7f ) << 49;
	if ( ( b & 0x80 ) == 0 )
		return res;
	b = fnGet();
	if ( ( b & 0x80 ) == 0 )
		return res | T ( b & 0x7f ) << 56;
	return res | T ( b & 0x7f ) << 56 | T ( fnGet() ) << 63;
}

inline DWORD UnzipIntLE ( const BYTE*& pBuf )
{
	return UnzipValueLE<DWORD> ( [&pBuf]() mutable { return *pBuf++; } );
}

inline SphOffset_t UnzipOffsetLE ( const BYTE*& pBuf )
{
	return UnzipValueLE<SphOffset_t> ( [&pBuf]() mutable { return *pBuf++; } );
}