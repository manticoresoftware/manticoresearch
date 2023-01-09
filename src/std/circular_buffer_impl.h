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

#include <utility>
#include <cassert>


// simple circular buffer
template<typename T>
CircularBuffer_T<T>::CircularBuffer_T ( int iInitialSize, float fGrowFactor )
	: m_dValues ( iInitialSize )
	, m_fGrowFactor ( fGrowFactor )
{}

template<typename T>
CircularBuffer_T<T>::CircularBuffer_T ( CircularBuffer_T<T>&& rhs ) noexcept
	: CircularBuffer_T ( 0, 1.5f )
{
	Swap ( rhs );
}

template<typename T>
void CircularBuffer_T<T>::Swap ( CircularBuffer_T& rhs ) noexcept
{
	m_dValues.SwapData ( rhs.m_dValues );
	std::swap ( m_fGrowFactor, rhs.m_fGrowFactor );
	std::swap ( m_iHead, rhs.m_iHead );
	std::swap ( m_iTail, rhs.m_iTail );
	std::swap ( m_iUsed, rhs.m_iUsed );
}

template<typename T>
CircularBuffer_T<T>& CircularBuffer_T<T>::operator= ( CircularBuffer_T<T> rhs )
{
	Swap ( rhs );
	return *this;
}


template<typename T>
void CircularBuffer_T<T>::Push ( const T& tValue )
{
	if ( m_iUsed == m_dValues.GetLength() )
		Resize ( int ( m_iUsed * m_fGrowFactor ) );

	m_dValues[m_iTail] = tValue;
	m_iTail = ( m_iTail + 1 ) % m_dValues.GetLength();
	m_iUsed++;
}

template<typename T>
T& CircularBuffer_T<T>::Push()
{
	if ( m_iUsed == m_dValues.GetLength() )
		Resize ( int ( m_iUsed * m_fGrowFactor ) );

	int iOldTail = m_iTail;
	m_iTail = ( m_iTail + 1 ) % m_dValues.GetLength();
	++m_iUsed;

	return m_dValues[iOldTail];
}

template<typename T>
T& CircularBuffer_T<T>::Pop()
{
	assert ( !IsEmpty() );
	int iOldHead = m_iHead;
	m_iHead = ( m_iHead + 1 ) % m_dValues.GetLength();
	--m_iUsed;

	return m_dValues[iOldHead];
}

template<typename T>
const T& CircularBuffer_T<T>::Last() const
{
	assert ( !IsEmpty() );
	return operator[] ( GetLength() - 1 );
}

template<typename T>
T& CircularBuffer_T<T>::Last()
{
	assert ( !IsEmpty() );
	int iIndex = GetLength() - 1;
	return m_dValues[( iIndex + m_iHead ) % m_dValues.GetLength()];
}

template<typename T>
const T& CircularBuffer_T<T>::operator[] ( int iIndex ) const
{
	assert ( iIndex < m_iUsed );
	return m_dValues[( iIndex + m_iHead ) % m_dValues.GetLength()];
}

template<typename T>
bool CircularBuffer_T<T>::IsEmpty() const
{
	return m_iUsed == 0;
}

template<typename T>
int CircularBuffer_T<T>::GetLength() const
{
	return m_iUsed;
}

template<typename T>
void CircularBuffer_T<T>::Resize ( int iNewLength )
{
	CSphFixedVector<T> dNew ( iNewLength );
	for ( int i = 0; i < GetLength(); i++ )
		dNew[i] = m_dValues[( i + m_iHead ) % m_dValues.GetLength()];

	m_dValues.SwapData ( dNew );

	m_iHead = 0;
	m_iTail = m_iUsed;
}

