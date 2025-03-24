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

/// scoped mutex lock
///  may adopt, lock and unlock explicitly
template<typename MTX>
class CAPABILITY ( "mutex" ) SCOPED_CAPABILITY CSphScopedLock: public ISphNoncopyable
{
public:
	// Tag type used to distinguish constructors.
	enum ADOPT_LOCK_E { adopt_lock };

	/// adopt already held lock
	CSphScopedLock ( MTX& tMutex, ADOPT_LOCK_E ) REQUIRES ( tMutex ) ACQUIRE ( tMutex );

	/// constructor acquires the lock
	explicit CSphScopedLock ( MTX& tMutex ) ACQUIRE ( tMutex );

	/// unlock on going out of scope
	~CSphScopedLock() RELEASE();

	/// Explicitly acquire the lock.
	/// to be used ONLY from the same thread! (call from another is obviously wrong)
	void Lock() ACQUIRE();

	/// Explicitly release the lock.
	void Unlock() RELEASE();

	bool Locked() const
	{
		return m_bLocked;
	}

	typename MTX::mtx& mutex() RETURN_CAPABILITY ( m_tMutexRef )
	{
		return m_tMutexRef.mutex();
	}

private:
	MTX& m_tMutexRef;
	bool m_bLocked; // whether the mutex is currently locked or unlocked
};

#include "scopedlock_impl.h"
