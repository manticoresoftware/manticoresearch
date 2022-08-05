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

// big-endian (most significant septets first)
template<typename T>
inline int sphCalcZippedLen ( T tValue )
{
	int nBytes = 1;
	tValue >>= 7;
	while ( tValue )
	{
		tValue >>= 7;
		++nBytes;
	}

	return nBytes;
}

// big-endian (most significant septets first)
template<typename T, typename WRITER>
inline int sphZipValue ( WRITER fnPut, T tValue )
{
	int nBytes = sphCalcZippedLen ( tValue );
	for ( int i = nBytes - 1; i >= 0; --i )
		fnPut ( ( 0x7f & ( tValue >> ( 7 * i ) ) ) | ( i ? 0x80 : 0 ) );

	return nBytes;
}

// big-endian (most significant septets first)
template<typename T>
inline int sphZipToPtr ( BYTE* pData, T tValue )
{
	return sphZipValue ( [pData] ( BYTE b ) mutable { *pData++ = b; }, tValue );
}