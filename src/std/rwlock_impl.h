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

#include "checks.h"
#include "ints.h"

inline RwLock_t::~RwLock_t ()
{
	Verify ( Done() );
#if !_WIN32
	SafeDelete ( m_pLock );
	SafeDelete ( m_pWritePreferHelper );
#endif
}


template<class LOCKED>
CSphScopedRLock_T<LOCKED>::CSphScopedRLock_T ( LOCKED & tLock )
	: m_tLock ( tLock )
{
	m_tLock.ReadLock();
}

template<class LOCKED>
CSphScopedRLock_T<LOCKED>::~CSphScopedRLock_T ()
{
	m_tLock.Unlock();
}


template<class LOCKED>
CSphScopedWLock_T<LOCKED>::CSphScopedWLock_T ( LOCKED & tLock )
	: m_tLock ( tLock )
{
	m_tLock.WriteLock();
}


template<class LOCKED>
CSphScopedWLock_T<LOCKED>::~CSphScopedWLock_T ()
{
	m_tLock.Unlock();
}


template<class LOCKED>
ScopedUnlock_T<LOCKED>::ScopedUnlock_T ( LOCKED &tLock )
	: m_pLock ( &tLock )
{}


template<class LOCKED>
ScopedUnlock_T<LOCKED>::ScopedUnlock_T ( ScopedUnlock_T && tLock ) noexcept
	: m_pLock ( tLock.m_pLock )
{
	tLock.m_pLock = nullptr;
}


template<class LOCKED>
ScopedUnlock_T<LOCKED>& ScopedUnlock_T<LOCKED>::operator= ( ScopedUnlock_T<LOCKED> &&rhs ) noexcept
{
	if ( this==&rhs )
		return *this;
	if ( m_pLock )
		m_pLock->Unlock();
	m_pLock = rhs.m_pLock;
	rhs.m_pLock = nullptr;
	return *this;
}


template<class LOCKED>
ScopedUnlock_T<LOCKED>::~ScopedUnlock_T ()
{
	if ( m_pLock )
		m_pLock->Unlock ();
}
