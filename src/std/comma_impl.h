//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//


#include <utility>
#include "generics.h"

inline Comma_c::Comma_c ( const char* sDelim )
{
	m_sComma = sDelim ? Str_t { sDelim, (int)strlen ( sDelim ) } : dEmptyStr;
}

inline Comma_c::Comma_c ( Str_t sDelim )
	: m_sComma ( std::move ( sDelim ) )
{}

inline Comma_c& Comma_c::operator= ( Comma_c rhs )
{
	Swap ( rhs );
	return *this;
}

inline void Comma_c::Swap ( Comma_c& rhs ) noexcept
{
	m_sComma.swap ( rhs.m_sComma );
	::Swap ( m_bStarted, rhs.m_bStarted );
}

inline Comma_c::operator Str_t()
{
	if ( m_bStarted )
		return m_sComma;
	m_bStarted = true;
	return dEmptyStr;
}
