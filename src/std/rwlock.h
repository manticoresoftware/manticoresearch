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

#include "thread_annotations.h"
#include "generics.h"
#include "ints.h"
#include "mutex.h"


/// rwlock implementation
class CAPABILITY ( "mutex" ) RwLock_t : public ISphNoncopyable
{
public:
	explicit RwLock_t ( bool bPreferWriter = false );
	~RwLock_t ();

	bool ReadLock () ACQUIRE_SHARED();
	bool WriteLock () ACQUIRE();
	bool Unlock () UNLOCK_FUNCTION();

	// Just for clang negative capabilities.
	const RwLock_t &operator! () const { return *this; }

private:
	bool				m_bInitialized = false;
#if _WIN32
	HANDLE				m_hWriteMutex = 0;
	HANDLE				m_hReadEvent = 0;
	LONG				m_iReaders = 0;
#else
	pthread_rwlock_t	* m_pLock;
	CSphMutex			* m_pWritePreferHelper = nullptr;
#endif

private:
	bool Init ( bool bPreferWriter = false );
	bool Done();
};


/// scoped shared (read) lock
template<class LOCKED = RwLock_t>
class SCOPED_CAPABILITY CSphScopedRLock_T : ISphNoncopyable
{
public:
	/// lock on creation
	explicit CSphScopedRLock_T ( LOCKED & tLock ) ACQUIRE_SHARED ( tLock );

	/// unlock on going out of scope
	~CSphScopedRLock_T () RELEASE ();

protected:
	LOCKED & m_tLock;
};


/// scoped exclusive (write) lock
template<class LOCKED = RwLock_t>
class SCOPED_CAPABILITY CSphScopedWLock_T : ISphNoncopyable
{
public:
	/// lock on creation
	explicit CSphScopedWLock_T ( LOCKED & tLock ) ACQUIRE ( tLock ) EXCLUDES ( tLock );

	/// unlock on going out of scope
	~CSphScopedWLock_T () RELEASE ();

protected:
	LOCKED & m_tLock;
};


/// scoped shared (read) fake fake - do nothing, just mute warnings
template<class LOCKED = RwLock_t>
struct SCOPED_CAPABILITY FakeScopedRLock_T : ISphNoncopyable
{
	explicit FakeScopedRLock_T ( LOCKED & tLock ) ACQUIRE_SHARED ( tLock ) {}
	~FakeScopedRLock_T() RELEASE () {}
};


/// scoped exclusive (write) fake lock - does nothing, just mute warnings
template<class LOCKED = RwLock_t>
struct SCOPED_CAPABILITY FakeScopedWLock_T : ISphNoncopyable
{
	explicit FakeScopedWLock_T ( LOCKED & tLock ) ACQUIRE ( tLock ) EXCLUDES ( tLock ) {}
	~FakeScopedWLock_T() RELEASE () {}
};


/// scoped lock owner - unlock in dtr
template<class LOCKED = RwLock_t>
class SCOPED_CAPABILITY ScopedUnlock_T : ISphNoncopyable
{
public:
	/// lock on creation
	explicit ScopedUnlock_T ( LOCKED &tLock ) ACQUIRE ( tLock );
	ScopedUnlock_T ( ScopedUnlock_T && tLock ) noexcept;
	ScopedUnlock_T &operator= ( ScopedUnlock_T &&rhs ) noexcept;

	/// unlock on going out of scope
	~ScopedUnlock_T () RELEASE ();

protected:
	LOCKED * m_pLock;
};

// shortcuts (original names sometimes looks too long)
using ScRL_t = CSphScopedRLock_T<>;
using ScWL_t = CSphScopedWLock_T<>;

#include "rwlock_impl.h"
