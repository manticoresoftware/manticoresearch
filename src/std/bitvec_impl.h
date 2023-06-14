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
#include <cstring>

template<typename T, int STATICBITS>
BitVec_T<T, STATICBITS>::BitVec_T ( int iElements )
{
	Init ( iElements );
}

template<typename T, int STATICBITS>
BitVec_T<T, STATICBITS>::~BitVec_T()
{
	if ( m_pData != m_dStatic )
		SafeDeleteArray ( m_pData );
}

template<typename T, int STATICBITS>
BitVec_T<T, STATICBITS>::BitVec_T ( const BitVec_T<T, STATICBITS>& rhs )
{
	m_pData = nullptr;
	m_iElements = rhs.m_iElements;
	auto iStorage = CalcStorage();
	m_pData = ( m_iElements > STATICBITS ) ? new T[iStorage] : m_dStatic;
	memcpy ( m_pData, rhs.m_pData, GetSizeBytes() );
}

template<typename T, int STATICBITS>
void BitVec_T<T, STATICBITS>::Swap ( BitVec_T<T, STATICBITS>& rhs ) noexcept
{
	if ( m_pData == m_dStatic && rhs.m_pData == rhs.m_dStatic )
	{
		// both static - just exchange values
		for ( auto i = 0; i < STATICSIZE; ++i )
			std::swap ( m_dStatic[i], rhs.m_dStatic[i] );
	} else if ( m_pData == m_dStatic )
	{
		// me static, rhs dynamic
		assert ( rhs.m_pData != rhs.m_dStatic );
		for ( auto i = 0; i < STATICSIZE; ++i )
			rhs.m_dStatic[i] = m_dStatic[i];

		m_pData = rhs.m_pData;
		rhs.m_pData = rhs.m_dStatic;
	} else if ( rhs.m_pData == rhs.m_dStatic )
	{
		// me dynamic, rhs static
		assert ( m_pData != m_dStatic );
		for ( auto i = 0; i < STATICSIZE; ++i )
			m_dStatic[i] = rhs.m_dStatic[i];

		rhs.m_pData = m_pData;
		m_pData = m_dStatic;
	} else
	{
		// both dynamic. No need to copy static at all
		assert ( rhs.m_pData != rhs.m_dStatic );
		assert ( m_pData != m_dStatic );
		std::swap ( m_pData, rhs.m_pData );
	}

	std::swap ( m_iElements, rhs.m_iElements );
}

template<typename T, int STATICBITS>
BitVec_T<T, STATICBITS>::BitVec_T ( BitVec_T<T, STATICBITS>&& rhs ) noexcept
{
	Swap ( rhs );
}

template<typename T, int STATICBITS>
BitVec_T<T, STATICBITS>& BitVec_T<T, STATICBITS>::operator= ( BitVec_T<T, STATICBITS> rhs )
{
	Swap ( rhs );
	return *this;
}

template<typename T, int STATICBITS>
void BitVec_T<T, STATICBITS>::Init ( int iElements )
{
	assert ( iElements >= 0 );

	if ( m_pData != m_dStatic )
		SafeDeleteArray ( m_pData );

	m_iElements = iElements;
	if ( iElements > STATICBITS )
		m_pData = new T[CalcStorage()];
	else
		m_pData = m_dStatic;

	Clear();
}

template<typename T, int STATICBITS>
void BitVec_T<T, STATICBITS>::Clear()
{
	memset ( m_pData, 0, GetSizeBytes() );
}

template<typename T, int STATICBITS>
void BitVec_T<T, STATICBITS>::Set()
{
	memset ( m_pData, 0xff, GetSizeBytes() );
}

template<typename T, int STATICBITS>
bool BitVec_T<T, STATICBITS>::BitGet ( int iIndex ) const noexcept
{
	assert ( m_pData );
	assert ( iIndex >= 0 );
	assert ( iIndex < m_iElements );
	return ( m_pData[iIndex >> SHIFT] & ( 1ULL << ( iIndex & MASK ) ) ) != 0; // NOLINT
}

template<typename T, int STATICBITS>
bool BitVec_T<T, STATICBITS>::BitGetOr ( int iIndex, bool bAlternative ) const noexcept
{
	if ( m_pData && iIndex >= 0 && iIndex < m_iElements )
		return ( m_pData[iIndex >> SHIFT] & ( 1ULL << ( iIndex & MASK ) ) ) != 0; // NOLINT
	return bAlternative;
}

template<typename T, int STATICBITS>
void BitVec_T<T, STATICBITS>::BitSet ( int iIndex )
{
	assert ( iIndex >= 0 );
	assert ( iIndex < m_iElements );
	m_pData[iIndex >> SHIFT] |= ( 1ULL << ( iIndex & MASK ) ); // NOLINT
}

template<typename T, int STATICBITS>
void BitVec_T<T, STATICBITS>::BitClear ( int iIndex )
{
	assert ( iIndex >= 0 );
	assert ( iIndex < m_iElements );
	m_pData[iIndex >> SHIFT] &= ~( 1ULL << ( iIndex & MASK ) ); // NOLINT
}

template<typename T, int STATICBITS>
const T* BitVec_T<T, STATICBITS>::Begin() const noexcept
{
	return m_pData;
}

template<typename T, int STATICBITS>
T* BitVec_T<T, STATICBITS>::Begin()
{
	return m_pData;
}

template<typename T, int STATICBITS>
int BitVec_T<T, STATICBITS>::GetSizeBytes() const noexcept
{
	return CalcStorage() * sizeof ( T );
}

template<typename T, int STATICBITS>
int BitVec_T<T, STATICBITS>::GetSize() const noexcept
{
	return m_iElements;
}


template<typename T, int STATICBITS>
bool BitVec_T<T, STATICBITS>::IsEmpty() const noexcept
{
	if ( !m_pData )
		return true;

	return GetSize() == 0;
}

template<typename T, int STATICBITS>
int BitVec_T<T, STATICBITS>::BitCount() const noexcept
{
	int iBitSet = 0;
	for ( int i = 0; i < CalcStorage(); i++ )
		iBitSet += sphBitCount ( m_pData[i] );

	return iBitSet;
}

template<typename T, int STATICBITS>
int BitVec_T<T, STATICBITS>::Scan ( int iStart )
{
	assert ( iStart < m_iElements );

	int iIndex = iStart >> SHIFT;
	T uMask = ~( ( (T)1 << ( iStart & MASK ) ) - 1 );
	if ( m_pData[iIndex] & uMask )
		return ( iIndex << SHIFT ) + ScanBit ( iIndex, iStart & MASK );

	int iSize = CalcStorage();
	iIndex++;
	while ( iIndex < iSize && !m_pData[iIndex] )
		iIndex++;

	if ( iIndex >= iSize )
		return m_iElements;

	return ( iIndex << SHIFT ) + ScanBit ( iIndex, 0 );
}


template<typename T, int STATICBITS>
int BitVec_T<T, STATICBITS>::ScanBit ( int iIndex, int iStart )
{
	T uData = m_pData[iIndex];
	for ( int i = iStart; i < SIZEBITS; i++ )
		if ( uData & ( (T)1 << i ) )
			return i;

	return m_iElements;
}

template<typename T, int STATICBITS>
int BitVec_T<T, STATICBITS>::CalcStorage() const noexcept
{
	return ( m_iElements + SIZEBITS - 1 ) / SIZEBITS;
}
