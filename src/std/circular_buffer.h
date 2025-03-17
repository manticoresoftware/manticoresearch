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

#include "fixedvector.h"


// simple circular buffer
template<typename T>
class CircularBuffer_T
{
public:
	explicit CircularBuffer_T ( int iInitialSize = 256, float fGrowFactor = 1.5f );

	CircularBuffer_T ( CircularBuffer_T&& rhs ) noexcept;
	void Swap ( CircularBuffer_T& rhs ) noexcept;
	CircularBuffer_T& operator= ( CircularBuffer_T rhs );
	void Push ( const T& tValue );
	T& Push();
	T& Pop();
	const T& Last() const;
	T& Last();
	const T& operator[] ( int iIndex ) const;
	bool IsEmpty() const;
	int GetLength() const;

private:
	CSphFixedVector<T> m_dValues;
	float m_fGrowFactor;
	int m_iHead = 0;
	int m_iTail = 0;
	int m_iUsed = 0;

	void Resize ( int iNewLength );
};

#include "circular_buffer_impl.h"