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

// shrink, if necessary, to be able to fit at least iLen more chars
inline void StringBuilder_c::GrowEnough ( int iLen )
{
	if ( m_iUsed + iLen<m_iSize )
		return;

	Grow ( iLen );
}


inline void StringBuilder_c::InitAddPrefix()
{
	if ( !m_szBuffer )
		InitBuffer();

	assert ( m_iUsed==0 || m_iUsed<m_iSize );

	auto sPrefix = Delim();
	if ( sPrefix.second ) // prepend delimiter first...
	{
		GrowEnough ( sPrefix.second );
		memcpy ( m_szBuffer + m_iUsed, sPrefix.first, sPrefix.second );
		m_iUsed += sPrefix.second;
	}
}

inline const Str_t & StringBuilder_c::Delim ()
{
	if ( m_dDelimiters.IsEmpty ())
		return dEmptyStr;
	int iLast = m_dDelimiters.GetLength()-1;
	std::function<void()> fnApply = [this, &iLast, &fnApply]()
	{
		--iLast;
		if ( iLast>=0 )
			AppendRawChunk( m_dDelimiters[iLast].RawComma( fnApply ));
	};
	return m_dDelimiters.Last().RawComma( fnApply );
}

inline void StringBuilder_c::LazyComma_c::Swap ( LazyComma_c & rhs ) noexcept
{
	Comma_c::Swap ( rhs );
	m_sPrefix.swap ( rhs.m_sPrefix );
	m_sSuffix.swap ( rhs.m_sSuffix );
	::Swap ( m_bSkipNext, rhs.m_bSkipNext );
}


inline StringBuilder_c& operator<< ( StringBuilder_c& tOut, const CSphNamedInt& tValue )
{
	tOut.Sprintf ( "%s=%d", tValue.first.cstr(), tValue.second );
	return tOut;
}


inline StringBuilder_c& operator<< ( StringBuilder_c& tOut, timespan_t tVal )
{
	tOut.Sprintf ( "%t", tVal.m_iVal );
	return tOut;
}


inline StringBuilder_c& operator<< ( StringBuilder_c& tOut, timestamp_t tVal )
{
	tOut.Sprintf ( "%T", tVal.m_iVal );
	return tOut;
}