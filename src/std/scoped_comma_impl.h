//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <cassert>

inline ScopedComma_c::ScopedComma_c ( StringBuilder_c& tOwner, const char* sDel, const char* sPref, const char* sTerm, bool bAllowEmpty )
	: m_pOwner ( &tOwner )
	, m_bAllowEmpty ( bAllowEmpty )
{
	m_iLevel = tOwner.StartBlock ( sDel, sPref, sTerm );
}

inline ScopedComma_c::ScopedComma_c ( StringBuilder_c& tOwner, const StrBlock_t& dBlock, bool bAllowEmpty )
	: m_pOwner ( &tOwner )
	, m_bAllowEmpty ( bAllowEmpty )
{
	m_iLevel = tOwner.StartBlock ( dBlock );
}

inline ScopedComma_c::~ScopedComma_c()
{
	if ( m_pOwner )
		m_pOwner->FinishBlocks ( m_iLevel, m_bAllowEmpty );
}

inline void ScopedComma_c::Swap ( ScopedComma_c& rhs ) noexcept
{
	::Swap ( m_pOwner, rhs.m_pOwner );
	::Swap ( m_iLevel, rhs.m_iLevel );
}

inline void ScopedComma_c::Init ( StringBuilder_c& tOwner, const char* sDel, const char* sPref, const char* sTerm )
{
	assert ( !m_pOwner );
	if ( m_pOwner )
		return;
	m_pOwner = &tOwner;
	m_iLevel = tOwner.StartBlock ( sDel, sPref, sTerm );
}

inline StringBuilder_c& ScopedComma_c::Sink() const
{
	assert ( m_pOwner );
	return *m_pOwner;
}
