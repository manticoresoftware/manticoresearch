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

#include <cassert>
#include <utility>

template<typename T>
LogMessage_t& LogMessage_t::operator<< ( T&& t )
{
	m_dLog << std::forward<T> ( t );
	return *this;
}

inline void LocMessage_c::Swap ( LocMessage_c& rhs ) noexcept
{
	std::swap ( m_dLog, rhs.m_dLog );
}

inline LocMessage_c::LocMessage_c ( const LocMessage_c& rhs )
{
	assert ( false && "NRVO failed" );
}

template<typename T>
LocMessage_c& LocMessage_c::operator<< ( T&& t )
{
	m_dLog << std::forward<T> ( t );
	return *this;
}

inline LocMessage_c LocMessages_c::GetLoc()
{
	return LocMessage_c ( this );
}

