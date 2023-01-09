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

#include "mutex.h"

#include "fatal.h"
#include "timers.h"
#include "config.h"
#include <cerrno>
#include <cstring>

//////////////////////////////////////////////////////////////////////////
// MUTEX
//////////////////////////////////////////////////////////////////////////

#if _WIN32

// Windows mutex implementation

CSphMutex::CSphMutex () noexcept
{
	m_tMutex = CreateMutex ( NULL, FALSE, NULL );
	if ( !m_tMutex )
		sphDie ( "CreateMutex() failed" );
}

CSphMutex::~CSphMutex () noexcept
{
	if ( CloseHandle ( m_tMutex )==FALSE )
		sphDie ( "CloseHandle() failed" );
}

bool CSphMutex::Lock ()
{
	DWORD uWait = WaitForSingleObject ( m_tMutex, INFINITE );
	return ( uWait!=WAIT_FAILED && uWait!=WAIT_TIMEOUT );
}

bool CSphMutex::TimedLock ( int iMsec )
{
	DWORD uWait = WaitForSingleObject ( m_tMutex, iMsec );
	return ( uWait!=WAIT_FAILED && uWait!=WAIT_TIMEOUT );
}

bool CSphMutex::Unlock ()
{
	return ReleaseMutex ( m_tMutex )==TRUE;
}

#else

// UNIX mutex implementation

CSphMutex::CSphMutex() noexcept
{
	if ( pthread_mutex_init ( &m_tMutex, nullptr ) )
		sphDie ( "pthread_mutex_init() failed %s", strerror ( errno ) );
}

CSphMutex::~CSphMutex() noexcept
{
	if ( pthread_mutex_destroy ( &m_tMutex ) )
		sphDie ( "pthread_mutex_destroy() failed %s", strerror ( errno ) );
}

bool CSphMutex::Lock ()
{
	return ( pthread_mutex_lock ( &m_tMutex )==0 );
}

bool CSphMutex::TimedLock ( int iMsec )
{
// pthread_mutex_timedlock is not available on Mac Os. Fallback to lock without a timer.
#if defined (HAVE_PTHREAD_MUTEX_TIMEDLOCK)
	struct timespec ts;
	clock_gettime ( CLOCK_REALTIME, &ts );

	int ns = ts.tv_nsec + ( iMsec % 1000 )*1000000;
	ts.tv_sec += ( ns / 1000000000 ) + ( iMsec / 1000 );
	ts.tv_nsec = ( ns % 1000000000 );

	int iRes = pthread_mutex_timedlock ( &m_tMutex, &ts );
	return iRes==0;

#else
	int iRes = EBUSY;
	int64_t tmTill = sphMicroTimer () + iMsec * 1000;
	do
	{
		iRes = pthread_mutex_trylock ( &m_tMutex );
		if ( iRes!=EBUSY )
			break;
		// below is inlined sphSleepMsec(1) - placed here to avoid dependency from libsphinx (for wordbreaker)
#if _WIN32
		Sleep ( 1 );
#else
		struct timeval tvTimeout;
		tvTimeout.tv_sec = 0;
		tvTimeout.tv_usec = 1000;
		select ( 0, nullptr, nullptr, nullptr, &tvTimeout );
#endif

	} while ( sphMicroTimer ()<tmTill );
	if ( iRes==EBUSY )
		iRes = pthread_mutex_trylock ( &m_tMutex );

	return iRes!=EBUSY;

#endif
}

bool CSphMutex::Unlock ()
{
	return ( pthread_mutex_unlock ( &m_tMutex )==0 );
}

#endif