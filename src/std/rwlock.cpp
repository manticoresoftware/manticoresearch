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

#include "rwlock.h"

//////////////////////////////////////////////////////////////////////////
// RWLOCK
//////////////////////////////////////////////////////////////////////////

#if _WIN32

// Windows rwlock implementation

RwLock_t::RwLock_t ( bool bPreferWriter )
{
	Verify ( Init ( bPreferWriter ) );
}


bool RwLock_t::Init ( bool )
{
	assert ( !m_bInitialized );
	assert ( !m_hWriteMutex && !m_hReadEvent && !m_iReaders );

	m_hReadEvent = CreateEvent ( NULL, TRUE, FALSE, NULL );
	if ( !m_hReadEvent )
		return false;

	m_hWriteMutex = CreateMutex ( NULL, FALSE, NULL );
	if ( !m_hWriteMutex )
	{
		CloseHandle ( m_hReadEvent );
		m_hReadEvent = NULL;
		return false;
	}
	m_bInitialized = true;
	return true;
}


bool RwLock_t::Done()
{
	if ( !m_bInitialized )
		return true;

	if ( !CloseHandle ( m_hReadEvent ) )
		return false;
	m_hReadEvent = NULL;

	if ( !CloseHandle ( m_hWriteMutex ) )
		return false;
	m_hWriteMutex = NULL;

	m_iReaders = 0;
	m_bInitialized = false;
	return true;
}


bool RwLock_t::ReadLock()
{
	assert ( m_bInitialized );

	DWORD uWait = WaitForSingleObject ( m_hWriteMutex, INFINITE );
	if ( uWait == WAIT_FAILED || uWait == WAIT_TIMEOUT )
		return false;

	// got the writer mutex, can't be locked for write
	// so it's OK to add the reader lock, then free the writer mutex
	// writer mutex also protects readers counter
	InterlockedIncrement ( &m_iReaders );

	// reset writer lock event, we just got ourselves a reader
	if ( !ResetEvent ( m_hReadEvent ) )
		return false;

	// release writer lock
	return ReleaseMutex ( m_hWriteMutex ) == TRUE;
}


bool RwLock_t::WriteLock()
{
	assert ( m_bInitialized );

	// try to acquire writer mutex
	DWORD uWait = WaitForSingleObject ( m_hWriteMutex, INFINITE );
	if ( uWait == WAIT_FAILED || uWait == WAIT_TIMEOUT )
		return false;

	// got the writer mutex, no pending readers, rock'n'roll
	if ( !m_iReaders )
		return true;

	// got the writer mutex, but still have to wait for all readers to complete
	uWait = WaitForSingleObject ( m_hReadEvent, INFINITE );
	if ( uWait == WAIT_FAILED || uWait == WAIT_TIMEOUT )
	{
		// wait failed, well then, release writer mutex
		ReleaseMutex ( m_hWriteMutex );
		return false;
	}
	return true;
}


bool RwLock_t::Unlock()
{
	assert ( m_bInitialized );

	// are we unlocking a writer?
	if ( ReleaseMutex ( m_hWriteMutex ) )
		return true; // yes we are

	if ( GetLastError() != ERROR_NOT_OWNER )
		return false; // some unexpected error

	// writer mutex wasn't mine; we must have a read lock
	if ( !m_iReaders )
		return true; // could this ever happen?

	// atomically decrement reader counter
	if ( InterlockedDecrement ( &m_iReaders ) )
		return true; // there still are pending readers

	// no pending readers, fire the event for write lock
	return SetEvent ( m_hReadEvent ) == TRUE;
}

#else

// UNIX rwlock implementation (pthreads wrapper)

RwLock_t::RwLock_t ( bool bPreferWriter )
{
	m_pLock = new pthread_rwlock_t;
	Verify ( Init ( bPreferWriter ) );
}

bool RwLock_t::Init ( bool bPreferWriter )
{
	assert ( !m_bInitialized );
	assert ( m_pLock );

	pthread_rwlockattr_t tAttr;
	pthread_rwlockattr_t* pAttr = nullptr;

	if ( bPreferWriter )
	{
		bool bOk = ( pthread_rwlockattr_init ( &tAttr ) == 0 );
		assert ( bOk );

		if ( bOk )
		{

#if HAVE_RWLOCK_PREFER_WRITER
			bOk = ( pthread_rwlockattr_setkind_np ( &tAttr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP ) == 0 );
#else
			// Mac OS X knows nothing about PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP
			m_pWritePreferHelper = new CSphMutex();
#endif
			assert ( bOk );

			if ( !bOk )
				pthread_rwlockattr_destroy ( &tAttr );
			else
				pAttr = &tAttr;
		}
	}
	m_bInitialized = ( pthread_rwlock_init ( m_pLock, pAttr ) == 0 );

	if ( pAttr )
		pthread_rwlockattr_destroy ( &tAttr );

	return m_bInitialized;
}

bool RwLock_t::Done()
{
	assert ( m_pLock );
	if ( !m_bInitialized )
		return true;

	m_bInitialized = pthread_rwlock_destroy ( m_pLock ) != 0;
	return !m_bInitialized;
}

bool RwLock_t::ReadLock()
{
	assert ( m_bInitialized );
	assert ( m_pLock );

	if ( !m_pWritePreferHelper )
		return pthread_rwlock_rdlock ( m_pLock ) == 0;

	ScopedMutex_t tScopedLock ( *m_pWritePreferHelper );
	return pthread_rwlock_rdlock ( m_pLock ) == 0;
}

bool RwLock_t::WriteLock()
{
	assert ( m_bInitialized );
	assert ( m_pLock );

	if ( !m_pWritePreferHelper )
		return pthread_rwlock_wrlock ( m_pLock ) == 0;

	ScopedMutex_t tScopedLock ( *m_pWritePreferHelper );
	return pthread_rwlock_wrlock ( m_pLock ) == 0;
}

bool RwLock_t::Unlock()
{
	assert ( m_bInitialized );
	assert ( m_pLock );

	return pthread_rwlock_unlock ( m_pLock ) == 0;
}

#endif