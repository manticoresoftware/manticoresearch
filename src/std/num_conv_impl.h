//
// Copyright (c) 2022-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include <cassert>
#include <cstring>
#include "generics.h"

inline static void Grow ( char*, int ) {};
inline static char* Tail ( char* p )
{
	return p;
}

template<typename Num, typename PCHAR>
static void NtoA_T ( PCHAR* ppOutput, Num uVal, int iBase = 10, int iWidth = 0, int iPrec = 0, char cFill = ' ' )
{
	assert ( ppOutput );
	//	assert ( tail ( *ppOutput ) );
	assert ( iWidth >= 0 );
	assert ( iPrec >= 0 );
	assert ( iBase > 0 && iBase <= 16 );

	const char cAllDigits[] = "fedcba9876543210123456789abcdef";
	// point to the '0'. This hack allows to process negative numbers,
	// since digit[x%10] for both x==2 and x==-2 is '2'.
	const char* cDigits = cAllDigits + sizeof ( cAllDigits ) / 2 - 1;
	const char cZero = '0';
	auto& pOutput = *ppOutput;

	if ( !uVal )
	{
		if ( !iPrec && !iWidth )
		{
			*Tail ( pOutput ) = cZero;
			++pOutput;
		} else
		{
			if ( !iPrec )
				++iPrec;
			else
				cFill = ' ';

			if ( iWidth )
			{
				if ( iWidth < iPrec )
					iWidth = iPrec;
				iWidth -= iPrec;
			}

			if ( iWidth >= 0 )
			{
				Grow ( pOutput, iWidth );
				memset ( Tail ( pOutput ), cFill, iWidth );
				pOutput += iWidth;
			}

			if ( iPrec >= 0 )
			{
				Grow ( pOutput, iPrec );
				memset ( Tail ( pOutput ), cZero, iPrec );
				pOutput += iPrec;
			}
		}
		return;
	}

	const BYTE uMaxIndex = 31; // 20 digits for MAX_INT64 in decimal; let it be 31 (32 digits max).
	char CBuf[uMaxIndex + 1];
	char* pRes = &CBuf[uMaxIndex];


	BYTE uNegative = 0;
	if ( uVal < 0 )
		++uNegative;

	while ( uVal )
	{
		*pRes-- = cDigits[uVal % iBase];
		uVal /= iBase;
	}

	auto uLen = (BYTE)( uMaxIndex - ( pRes - CBuf ) );
	if ( !uLen )
		uNegative = 0;

	if ( iPrec && iWidth && cFill == cZero )
		cFill = ' ';

	if ( iWidth )
		iWidth = iWidth - Max ( iPrec, uLen ) - uNegative;

	if ( uNegative && cFill == cZero )
	{
		*Tail ( pOutput ) = '-';
		++pOutput;
		uNegative = 0;
	}

	if ( iWidth >= 0 )
	{
		Grow ( pOutput, iWidth );
		memset ( Tail ( pOutput ), cFill, iWidth );
		pOutput += iWidth;
	}

	if ( uNegative )
	{
		*Tail ( pOutput ) = '-';
		++pOutput;
	}

	if ( iPrec )
		iPrec -= uLen;

	if ( iPrec >= 0 )
	{
		Grow ( pOutput, iPrec );
		memset ( Tail ( pOutput ), cZero, iPrec );
		pOutput += iPrec;
	}

	Grow ( pOutput, uLen );
	memcpy ( Tail ( pOutput ), pRes + 1, uLen );
	pOutput += uLen;
}

static const int nDividers = 10;
static const int Dividers[nDividers] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

template<typename Num, typename PCHAR>
void IFtoA_T ( PCHAR* pOutput, Num nVal, int iPrec )
{
	assert ( iPrec < nDividers );
	auto& pBegin = *pOutput;
	if ( nVal < 0 )
	{
		*Tail ( pBegin ) = '-';
		++pBegin;
		nVal = -nVal;
	}
	auto iInt = nVal / Dividers[iPrec];
	auto iFrac = nVal % Dividers[iPrec];
	::NtoA_T ( &pBegin, iInt );
	*Tail ( pBegin ) = '.';
	++pBegin;
	::NtoA_T ( &pBegin, iFrac, 10, 0, iPrec, '0' );
}

namespace sph {

template<typename Num>
int NtoA ( char* pOutput, Num nVal, int iBase, int iWidth, int iPrec, char cFill )
{
	auto pBegin = pOutput;
	NtoA_T ( &pBegin, nVal, iBase, iWidth, iPrec, cFill );
	return int ( pBegin - pOutput );
}

inline int IFtoA ( char* pOutput, int nVal, int iPrec )
{
	auto pBegin = pOutput;
	IFtoA_T ( &pBegin, nVal, iPrec );
	return int ( pBegin - pOutput );
}

inline int IFtoA ( char* pOutput, int64_t nVal, int iPrec )
{
	auto pBegin = pOutput;
	IFtoA_T ( &pBegin, nVal, iPrec );
	return int ( pBegin - pOutput );
}

}