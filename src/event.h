//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
/// @file event.h - declares and implements generic low-level event, mutex-type agnostic.

#pragma once

#include "config.h"

#include "sphinxstd.h"
#include <mutex>

// exception on event create instead of ec (usually we invoke sphDie, let's try exceptions in some places)
class SystemError_c : public std::exception
{
	int m_iErrorCode ;
	mutable CSphString m_sWhat;

public:
	explicit SystemError_c ( int iEc ) noexcept
		: m_iErrorCode ( iEc ) {}

	explicit SystemError_c ( const char * szWhat )
		: m_iErrorCode ( 0 )
	{
		m_sWhat.SetSprintf ( "%s", szWhat );
	}

	SystemError_c ( int iEc, const char* szWhat )
		: m_iErrorCode ( iEc )
	{
		m_sWhat.SetSprintf ( "%s, error %d", szWhat, iEc );
	}

	~SystemError_c () noexcept override {}

	int Errno () const noexcept { return m_iErrorCode; }
	const char* sWhat() const noexcept { return m_sWhat.scstr(); }
};

namespace sph {

#if _WIN32

class Event_c : public ISphNoncopyable
{
	HANDLE m_Events[2]; // 0-th with manual reset, to signal all. 1-st with auto reset, to signal one.
	DWORD m_uState;

public:
	// Constructor.
	Event_c ()
		: m_uState ( 0 )
	{
		m_Events[0] = CreateEvent ( NULL, TRUE, FALSE, NULL );
		if ( !m_Events[0] )
			throw SystemError_c ( GetLastError(), "event creation failed" );

		m_Events[1] = CreateEvent ( NULL, FALSE, FALSE, NULL );
		if ( !m_Events[1] )
		{
			CloseHandle ( m_Events[0]);
			throw SystemError_c ( GetLastError (), "event creation failed" );
		}
	}

	// Destructor.
	~Event_c ()
	{
		CloseHandle ( m_Events[0] );
		CloseHandle ( m_Events[1] );
	}

	// Signal all waiters.
	template<typename Lock>
	void SignalAll ( Lock & dLock )
	{
		assert ( dLock.Locked ());
		m_uState |= 1;
		SetEvent ( m_Events[0] );
	}

	// Unlock the mutex and signal one waiter.
	template<typename Lock>
	void UnlockAndSignalOne ( Lock & dLock )
	{
		assert ( dLock.Locked ());
		m_uState |= 1;
		bool bHaveWaiters = (m_uState>1);
		dLock.Unlock ();
		if ( bHaveWaiters )
			SetEvent ( m_Events[1] );
	}

	// If there's a waiter, unlock the mutex and signal it.
	template<typename Lock>
	bool MaybeUnlockAndSignalOne ( Lock & dLock )
	{
		assert ( dLock.Locked ());
		m_uState |= 1;
		if ( m_uState>1 )
		{
			dLock.Unlock ();
			SetEvent ( m_Events[1] );
			return true;
		}
		return false;
	}

	// Reset the event.
	template<typename Lock>
	void Clear ( Lock & dLock )
	{
		assert ( dLock.Locked ());
		ResetEvent ( m_Events[0] );
		m_uState &= ~size_t ( 1 );
	}

	// Wait for the event to become signalled.
	template<typename Lock>
	void Wait ( Lock & dLock )
	{
		assert ( dLock.Locked ());
		while ((m_uState & 1)==0)
		{
			m_uState += 2;
			dLock.Unlock ();
			WaitForMultipleObjects ( 2, m_Events, FALSE, INFINITE );
			dLock.Lock ();
			m_uState -= 2;
		}
	}

	// Timed wait for the event to become signalled.
	template<typename Lock>
	bool WaitForUsec ( Lock & dLock, long iUsec )
	{
		assert ( dLock.Locked ());
		if ((m_uState & 1)==0 ) {
			m_uState += 2;
			dLock.Unlock ();
			DWORD uMsec = iUsec>0 ? (iUsec<1000 ? 1 : iUsec / 1000) : 0;
			WaitForMultipleObjects ( 2, m_Events, FALSE, uMsec );
			dLock.Lock ();
			m_uState -= 2;
		}
		return (m_uState & 1)!=0;
	}
};

#else

class Event_c : public ISphNoncopyable
{
	pthread_cond_t m_tCond;
	DWORD m_uState; // bit 0 - when signalled; bit 1 - in waiting.

public:
	Event_c ()
		: m_uState ( 0)
	{

#if (defined(__MACH__) && defined(__APPLE__))
		int iError = pthread_cond_init ( &m_tCond, nullptr );
#else
		pthread_condattr_t attr;
		pthread_condattr_init(&attr);
		int iError = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
		if (iError == 0)
			iError = pthread_cond_init(&m_tCond, &attr);
#endif
		if ( iError!=0 )
			throw SystemError_c (iError, "event creation failed");
	}

	// Destructor.
	~Event_c ()
	{
		pthread_cond_destroy ( &m_tCond );
	}

	// Signal all waiters.
	template<typename Lock>
	void SignalAll ( Lock & dLock ) REQUIRES (dLock)
	{
		assert ( dLock.Locked ());
		m_uState |= 1;
		pthread_cond_broadcast ( &m_tCond ); // Ignore EINVAL.
	}

	// Unlock the mutex and signal one waiter.
	template<typename Lock>
	void UnlockAndSignalOne ( Lock & dLock ) RELEASE ( dLock )
	{
		assert ( dLock.Locked ());
		m_uState |= 1;
		bool bHaveWaiters = (m_uState>1);
		dLock.Unlock ();
		if ( bHaveWaiters )
			pthread_cond_signal ( &m_tCond ); // Ignore EINVAL.
	}

	// If there's a waiter, unlock the mutex and signal it.
	template<typename Lock>
	bool MaybeUnlockAndSignalOne ( Lock & dLock )
			REQUIRES ( dLock ) RELEASE ( dLock ) TRY_ACQUIRE ( false, dLock )
	{
		assert ( dLock.Locked ());
		m_uState |= 1;
		if ( m_uState>1 )
		{
			dLock.Unlock ();
			pthread_cond_signal ( &m_tCond ); // Ignore EINVAL.
			return true;
		}
		return false;
	}

	// Reset the event.
	template<typename Lock>
	void Clear ( Lock & dLock ) REQUIRES ( dLock )
	{
		assert ( dLock.Locked ());
		m_uState &= ~size_t ( 1 );
	}

	// Wait for the event to become signalled.
	template<typename Lock>
	void Wait ( Lock & dLock ) REQUIRES ( dLock )
	{
		assert ( dLock.Locked ());
		while ((m_uState & 1)==0)
		{
			m_uState += 2;
			pthread_cond_wait ( &m_tCond, &dLock.mutex () ); // Ignore EINVAL.
			m_uState -= 2;
		}
	}

	// Timed wait for the event to become signalled.
	template<typename Lock>
	bool WaitForUsec ( Lock & dLock, long iUsec ) REQUIRES ( dLock )
	{
		assert ( dLock.Locked ());
		if ((m_uState & 1)==0 )
		{
			m_uState += 2;
			timespec ts;

#if (defined(__MACH__) && defined(__APPLE__))
			ts.tv_sec = iUsec / 1000000;
			ts.tv_nsec = (iUsec % 1000000) * 1000;
			pthread_cond_timedwait_relative_np ( &m_tCond, &dLock.mutex (), &ts ); // Ignore EINVAL.
#else
			if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
			{
				ts.tv_sec += iUsec / 1000000;
				ts.tv_nsec += (iUsec % 1000000) * 1000;
				ts.tv_sec += ts.tv_nsec / 1000000000;
				ts.tv_nsec = ts.tv_nsec % 1000000000;
				pthread_cond_timedwait ( &m_tCond, &dLock.mutex (), &ts ); // Ignore EINVAL.
			}
#endif
			m_uState -= 2;
		}
		return (m_uState & 1)!=0;
	}
};

#endif

} // namespace sph
