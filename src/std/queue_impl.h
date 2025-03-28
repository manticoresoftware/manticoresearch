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
#include <cassert>


template<typename T, typename COMP>
CSphQueue<T, COMP>::CSphQueue ( int iSize )
{
	Reset ( iSize );
}


template<typename T, typename COMP>
CSphQueue<T, COMP>::~CSphQueue()
{
	SafeDeleteArray ( m_pData );
}

template<typename T, typename COMP>
void CSphQueue<T, COMP>::Reset ( int iSize )
{
	SafeDeleteArray ( m_pData );
	assert ( iSize >= 0 );
	m_iSize = iSize;
	if ( iSize )
		m_pData = new T[iSize];
	assert ( !iSize || m_pData );
}

template<typename T, typename COMP>
void CSphQueue<T, COMP>::Clear()
{
	m_iUsed = 0;
}

/// add entry to the queue
template<typename T, typename COMP>
bool CSphQueue<T, COMP>::Push ( const T& tEntry )
{
	assert ( m_pData );
	if ( m_iUsed == m_iSize )
	{
		// if it's worse that current min, reject it, else pop off current min
		if ( COMP::IsLess ( tEntry, m_pData[0] ) )
			return false;
		else
			Pop();
	}

	// do add
	m_pData[m_iUsed] = tEntry;
	int iEntry = m_iUsed++;

	// shift up if needed, so that worst (lesser) ones float to the top
	while ( iEntry )
	{
		int iParent = ( iEntry - 1 ) >> 1;
		if ( !COMP::IsLess ( m_pData[iEntry], m_pData[iParent] ) )
			break;

		// entry is less than parent, should float to the top
		Swap ( m_pData[iEntry], m_pData[iParent] );
		iEntry = iParent;
	}

	return true;
}

/// remove root (ie. top priority) entry
template<typename T, typename COMP>
void CSphQueue<T, COMP>::Pop()
{
	assert ( m_iUsed && m_pData );
	if ( !( --m_iUsed ) ) // empty queue? just return
		return;

	// make the last entry my new root
	m_pData[0] = m_pData[m_iUsed];

	// shift down if needed
	int iEntry = 0;
	while ( true )
	{
		// select child
		int iChild = ( iEntry << 1 ) + 1;
		if ( iChild >= m_iUsed )
			break;

		// select smallest child
		if ( iChild + 1 < m_iUsed )
			if ( COMP::IsLess ( m_pData[iChild + 1], m_pData[iChild] ) )
				++iChild;

		// if smallest child is less than entry, do float it to the top
		if ( COMP::IsLess ( m_pData[iChild], m_pData[iEntry] ) )
		{
			Swap ( m_pData[iChild], m_pData[iEntry] );
			iEntry = iChild;
			continue;
		}

		break;
	}
}

/// get current root
template<typename T, typename COMP>
inline const T& CSphQueue<T, COMP>::Root() const
{
	assert ( m_iUsed && m_pData );
	return m_pData[0];
}

template<typename T, typename COMP>
inline const T & CSphQueue<T, COMP>::Last() const
{
	assert ( m_iUsed && m_pData );
	return m_pData[m_iUsed-1];
}
