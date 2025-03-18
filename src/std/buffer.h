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

#include "generics.h"
#include "vectraits.h"
#include "string.h"

/// buffer trait that neither own buffer nor clean-up it on destroy
template<typename T>
class CSphBufferTrait: public ISphNoncopyable, public VecTraits_T<T>
{
protected:
	using VecTraits_T<T>::m_pData;
	using VecTraits_T<T>::m_iCount;

public:
	using VecTraits_T<T>::GetLengthBytes;
	/// ctor
	CSphBufferTrait() = default;

	/// dtor
	virtual ~CSphBufferTrait();

	virtual void Reset() = 0;


	/// get write address
	T* GetWritePtr() const
	{
		return m_pData;
	}

	/// returns read address - same as write, but const pointer
	const T* GetReadPtr() const
	{
		return GetWritePtr();
	}

	void Set ( T* pData, int64_t iCount );

	bool MemLock ( CSphString& sWarning );

protected:
	bool m_bMemLocked = false;

	void MemUnlock();
};

#include "buffer_impl.h"
