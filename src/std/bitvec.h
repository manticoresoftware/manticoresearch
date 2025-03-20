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

#pragma once

#include "ints.h"
#include "log2.h"

//////////////////////////////////////////////////////////////////////////

/// generic dynamic bitvector
/// with a preallocated part for small-size cases, and a dynamic route for big-size ones
template<typename T=DWORD, int STATICBITS=128>
class BitVec_T
{
public:
	BitVec_T () = default;

	explicit BitVec_T ( int iElements );
	explicit BitVec_T ( T * pData, int iElements );
	~BitVec_T ();

	void Swap ( BitVec_T & rhs ) noexcept;
	BitVec_T ( BitVec_T && rhs ) noexcept;
	BitVec_T ( const BitVec_T& rhs );
	BitVec_T & operator= ( BitVec_T rhs );

	void Init ( int iElements );
	void Clear();
	void Set ();
	bool BitGet ( int iIndex ) const noexcept;
	bool BitGetOr ( int iIndex, bool bAlternative=false ) const noexcept;
	void BitSet ( int iIndex );
	void BitClear ( int iIndex );
	const T * Begin() const noexcept;
	T * Begin();
	int GetSizeBytes () const noexcept;
	int GetSize() const noexcept;

	bool IsEmpty() const noexcept;
	int BitCount () const noexcept;
	int Scan ( int iStart ) const;
	void Negate ();

protected:
	static constexpr int	SIZEBITS = sizeof ( T ) * CHAR_BIT;
	static constexpr int	STATICSIZE = STATICBITS / SIZEBITS;
	static constexpr T		MASK = T ( sizeof ( T ) * CHAR_BIT - 1 );
	static constexpr int	SHIFT = sphLog2const ( SIZEBITS ) - 1;

	T *		m_pData = nullptr;
	T		m_dStatic[STATICSIZE] {0};
	int		m_iElements = 0;
	bool	m_bOwnStorage = true;

private:
	int ScanBit ( int iIndex, int iStart ) const;
	int CalcStorage() const noexcept;
};

using CSphBitvec = BitVec_T<>;

#include "bitvec_impl.h"
