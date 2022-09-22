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

#pragma once

#include <utility>
#include "ints.h"

/// generic bytes of chars array
using ByteBlob_t = std::pair<const BYTE*, int>;

inline bool IsEmpty ( const ByteBlob_t& dBlob )		{ return !dBlob.second; }
inline bool IsFilled ( const ByteBlob_t& dBlob )	{ return dBlob.first && dBlob.second > 0; }
inline bool IsValid ( const ByteBlob_t& dBlob )	{ return IsEmpty ( dBlob ) || IsFilled ( dBlob ); }

/// blob of chars with length (for zero-copy string processing)
using Str_t = std::pair<const char*, int>;
const Str_t dEmptyStr { "", 0 };

// Str_t stuff
inline bool IsEmpty ( const Str_t& dBlob ) { return !dBlob.second; }
inline bool IsFilled ( const Str_t& dBlob ) { return dBlob.first && dBlob.second > 0; }
Str_t FromSz ( const char* szString ); // { return { szString, szString ? (int)strlen ( szString ) : 0 }; }

struct CSphString;
Str_t FromStr ( const CSphString& sString ); // { return { sString.cstr(), (int)sString.Length() }; }
#define FROMS( STR ) Str_t { STR, sizeof ( STR ) - 1 }

/// mutual conversion Str_t <-> ByteBlob_t
inline Str_t B2S ( const ByteBlob_t& sData ) { return { (const char*)sData.first, sData.second }; }
inline ByteBlob_t S2B ( const Str_t& sStr ) { return { (const BYTE*)sStr.first, sStr.second }; };

/// name+int pair
using CSphNamedInt = std::pair<CSphString, int>;

/// period of time in us
struct timespan_t
{
	int64_t m_iVal;
	explicit timespan_t ( int64_t iVal ) noexcept
		: m_iVal ( iVal ) {};
};

/// timestamp in us
struct timestamp_t
{
	int64_t m_iVal;
	explicit timestamp_t ( int64_t iVal ) noexcept
		: m_iVal ( iVal ) {};
};

/// fixed prec. number
template<typename INT, int iPrec>
struct FixedFrac_T
{
	INT m_tVal;
	explicit FixedFrac_T ( INT iVal ) noexcept
		: m_tVal ( iVal ) {};
};

template<int prec=3> FixedFrac_T<int, prec> FixedFrac ( int iVal );
template<int prec=3> FixedFrac_T<int64_t, prec> FixedFrac ( int64_t iVal );

/// num with predefined output traits (to be used in << instead of formatted)
template<typename INT, int iBase, int iWidth, int iPrec, char cFill>
struct FixedNum_T
{
	INT m_tVal;
	explicit FixedNum_T ( INT iVal ) noexcept
		: m_tVal ( iVal ) {};
};

template<int iBase=10, int iWidth=0, int iPrec=0, char cFill=' '> FixedNum_T<int64_t, iBase, iWidth, iPrec, cFill> FixedNum ( int64_t iVal );
template<int iBase=10, int iWidth=0, int iPrec=0, char cFill=' '> FixedNum_T<int, iBase, iWidth, iPrec, cFill> FixedNum ( int iVal );
template<int iWidth> FixedNum_T<int64_t, 10, iWidth, 0, '0'> Digits ( int64_t iVal );
template<int iWidth> FixedNum_T<int, 10, iWidth, 0, '0'> Digits ( int iVal );

#include "blobs_impl.h"