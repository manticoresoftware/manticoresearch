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

#include <cassert>
#include <cerrno>

#include "mm.h"

template<typename T>
CSphBufferTrait<T>::~CSphBufferTrait()
{
	assert ( !m_bMemLocked && !m_pData );
}

template<typename T>
void CSphBufferTrait<T>::Set ( T* pData, int64_t iCount )
{
	m_pData = pData;
	m_iCount = iCount;
}

template<typename T>
bool CSphBufferTrait<T>::MemLock ( CSphString& sWarning )
{
	m_bMemLocked = mmlock ( m_pData, GetLengthBytes() );
	if ( !m_bMemLocked )
		sWarning.SetSprintf ( "mlock() failed: %s", strerror ( errno ) );

	return m_bMemLocked;
}

template<typename T>
void CSphBufferTrait<T>::MemUnlock()
{
	if ( !m_bMemLocked )
		return;

	m_bMemLocked = false;
	Debug ( bool bOk = ) mmunlock ( m_pData, GetLengthBytes() );
	assert ( bOk && "munlock() failed: %s" );
}
