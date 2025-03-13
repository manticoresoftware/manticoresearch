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


template<typename MTX>
CSphScopedLock<MTX>::CSphScopedLock ( MTX& tMutex, CSphScopedLock<MTX>::ADOPT_LOCK_E )
	: m_tMutexRef ( tMutex )
	, m_bLocked ( true )
{}


template<typename MTX>
CSphScopedLock<MTX>::CSphScopedLock ( MTX& tMutex ) ACQUIRE ( tMutex )
	: m_tMutexRef ( tMutex )
	, m_bLocked ( true )
{
	m_tMutexRef.Lock();
	m_bLocked = true;
}

template<typename MTX>
CSphScopedLock<MTX>::~CSphScopedLock() RELEASE()
{
	if ( m_bLocked )
		m_tMutexRef.Unlock();
}

template<typename MTX>
void CSphScopedLock<MTX>::Lock()
{
	if ( !m_bLocked )
	{
		m_tMutexRef.Lock();
		m_bLocked = true;
	}
}

template<typename MTX>
void CSphScopedLock<MTX>::Unlock()
{
	if ( m_bLocked )
	{
		m_tMutexRef.Unlock();
		m_bLocked = false;
	}
}
