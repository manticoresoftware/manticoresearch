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

#include "generics.h"
#include <cstdlib>


/// ctor from C string
inline CSphVariant::CSphVariant ( const char* sString, int iTag )
	: m_sValue ( sString )
	, m_iValue ( sString ? atoi ( sString ) : 0 )
	, m_i64Value ( sString ? (int64_t)strtoull ( sString, nullptr, 10 ) : 0 )
	, m_fValue ( sString ? (float)atof ( sString ) : 0.0f )
	, m_iTag ( iTag )
{
}

/// copy ctor
inline CSphVariant::CSphVariant ( const CSphVariant& rhs )
{
	if ( rhs.m_pNext )
		m_pNext = new CSphVariant ( *rhs.m_pNext );

	m_sValue = rhs.m_sValue;
	m_iValue = rhs.m_iValue;
	m_i64Value = rhs.m_i64Value;
	m_fValue = rhs.m_fValue;
	m_bTag = rhs.m_bTag;
	m_iTag = rhs.m_iTag;
}

/// move ctor
inline CSphVariant::CSphVariant ( CSphVariant&& rhs ) noexcept
	: m_pNext ( nullptr ) // otherwise trash in uninitialized m_pNext causes crash in dtr
{
	Swap ( rhs );
}


/// default dtor
/// WARNING: automatically frees linked items!
inline CSphVariant::~CSphVariant()
{
	SafeDelete ( m_pNext );
}

/// default copy operator
inline CSphVariant& CSphVariant::operator= ( CSphVariant rhs )
{
	Swap ( rhs );
	return *this;
}

inline void CSphVariant::Swap ( CSphVariant& rhs ) noexcept
{
	::Swap ( m_pNext, rhs.m_pNext );
	::Swap ( m_sValue, rhs.m_sValue );
	::Swap ( m_iValue, rhs.m_iValue );
	::Swap ( m_i64Value, rhs.m_i64Value );
	::Swap ( m_fValue, rhs.m_fValue );
	::Swap ( m_bTag, rhs.m_bTag );
	::Swap ( m_iTag, rhs.m_iTag );
}

