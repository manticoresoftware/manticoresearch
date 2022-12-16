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

inline Str_t FromSz ( const char* szString ) noexcept { return { szString, szString ? (int)strlen ( szString ) : 0 }; }

template<int prec>
inline FixedFrac_T<int, prec> FixedFrac ( int iVal )
{
	return FixedFrac_T<int, prec> { iVal };
}

template<int prec>
inline FixedFrac_T<int64_t, prec> FixedFrac ( int64_t iVal )
{
	return FixedFrac_T<int64_t, prec> { iVal };
}

template<int iBase, int iWidth, int iPrec, char cFill>
inline FixedNum_T<int64_t, iBase, iWidth, iPrec, cFill> FixedNum ( int64_t iVal )
{
	return FixedNum_T<int64_t, iBase, iWidth, iPrec, cFill> { iVal };
}

template<int iBase, int iWidth, int iPrec, char cFill>
inline FixedNum_T<int, iBase, iWidth, iPrec, cFill> FixedNum ( int iVal )
{
	return FixedNum_T<int, iBase, iWidth, iPrec, cFill> { iVal };
}

template<int iWidth>
inline FixedNum_T<int64_t, 10, iWidth, 0, '0'> Digits ( int64_t iVal )
{
	return FixedNum<10, iWidth, 0, '0'> ( iVal );
}
template<int iWidth>
inline FixedNum_T<int, 10, iWidth, 0, '0'> Digits ( int iVal )
{
	return FixedNum<10, iWidth, 0, '0'> ( iVal );
}