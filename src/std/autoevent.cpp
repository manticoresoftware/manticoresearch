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

#include "autoevent.h"

#include "fatal.h"
#include "timers.h"
#include "config.h"
#include <cerrno>


#if _WIN32

EventWrapper_c::EventWrapper_c()
{
	InitializeConditionVariable ( &m_tCond );
	InitializeCriticalSection ( &m_tMutex );
	m_bInitialized = true;
}

EventWrapper_c::~EventWrapper_c()
{
	if ( !m_bInitialized )
		return;

	m_bInitialized = false;
	DeleteCriticalSection ( &m_tMutex );
	// no need to deinit m_tCond as it is local
}

template<>
void AutoEvent_T<true>::SetEvent()
{
	EnterCriticalSection ( &m_tMutex );
	m_iSent = 1;
	LeaveCriticalSection ( &m_tMutex );
	WakeConditionVariable ( &m_tCond );
}

template<>
void AutoEvent_T<false>::SetEvent()
{
	EnterCriticalSection ( &m_tMutex );
	++m_iSent;
	LeaveCriticalSection ( &m_tMutex );
	WakeConditionVariable ( &m_tCond );
}

template<>
bool AutoEvent_T<true>::WaitEvent ( int iMsec )
{
	if ( !m_bInitialized )
		return false;

	DWORD iTime = ( iMsec == -1 ) ? INFINITE : iMsec;
	EnterCriticalSection ( &m_tMutex );
	while ( !m_iSent )
		if ( !SleepConditionVariableCS ( &m_tCond, &m_tMutex, iTime ) && GetLastError() == ERROR_TIMEOUT )
		{
			LeaveCriticalSection ( &m_tMutex );
			return false;
		}

	m_iSent = 0;
	LeaveCriticalSection ( &m_tMutex );
	return true;
}

template<>
bool AutoEvent_T<false>::WaitEvent ( int iMsec )
{
	if ( !m_bInitialized )
		return false;

	DWORD iTime = ( iMsec == -1 ) ? INFINITE : iMsec;
	EnterCriticalSection ( &m_tMutex );
	while ( !m_iSent )
		if ( !SleepConditionVariableCS ( &m_tCond, &m_tMutex, iTime ) && GetLastError() == ERROR_TIMEOUT )
		{
			LeaveCriticalSection ( &m_tMutex );
			return false;
		}

	--m_iSent;
	LeaveCriticalSection ( &m_tMutex );
	return true;
}

#else

// UNIX implementation

EventWrapper_c::EventWrapper_c()
{
	m_bInitialized = ( pthread_mutex_init ( &m_tMutex, nullptr ) == 0 );
	m_bInitialized &= ( pthread_cond_init ( &m_tCond, nullptr ) == 0 );
}

EventWrapper_c::~EventWrapper_c()
{
	if ( !m_bInitialized )
		return;

	pthread_cond_destroy ( &m_tCond );
	pthread_mutex_destroy ( &m_tMutex );
}

template<>
void AutoEvent_T<false>::SetEvent()
{
	if ( !m_bInitialized )
		return;

	pthread_mutex_lock ( &m_tMutex );
	++m_iSent;
	pthread_cond_signal ( &m_tCond );
	pthread_mutex_unlock ( &m_tMutex );
}

template<>
void AutoEvent_T<true>::SetEvent()
{
	if ( !m_bInitialized )
		return;

	pthread_mutex_lock ( &m_tMutex );
	m_iSent = 1;
	pthread_cond_signal ( &m_tCond );
	pthread_mutex_unlock ( &m_tMutex );
}

template<>
bool AutoEvent_T<false>::WaitEvent ( int iMsec )
{
	if ( !m_bInitialized )
		return false;

	if ( iMsec == -1 )
	{
		pthread_mutex_lock ( &m_tMutex );
		while ( !m_iSent )
			pthread_cond_wait ( &m_tCond, &m_tMutex );

		--m_iSent;
		pthread_mutex_unlock ( &m_tMutex );
		return true;
	}

#ifdef HAVE_PTHREAD_COND_TIMEDWAIT
	struct timespec ts;
	clock_gettime ( CLOCK_REALTIME, &ts );

	int ns = ts.tv_nsec + ( iMsec % 1000 ) * 1000000;
	ts.tv_sec += ( ns / 1000000000 ) + ( iMsec / 1000 );
	ts.tv_nsec = ( ns % 1000000000 );

	int iRes = 0;
	pthread_mutex_lock ( &m_tMutex );
	while ( !m_iSent && !iRes )
		iRes = pthread_cond_timedwait ( &m_tCond, &m_tMutex, &ts );

	bool bEventHappened = iRes != ETIMEDOUT;
	if ( bEventHappened )
		--m_iSent;
	pthread_mutex_unlock ( &m_tMutex );
	return bEventHappened;
#endif
}

template<>
bool AutoEvent_T<true>::WaitEvent ( int iMsec )
{
	if ( !m_bInitialized )
		return false;

	if ( iMsec == -1 )
	{
		pthread_mutex_lock ( &m_tMutex );
		while ( !m_iSent )
			pthread_cond_wait ( &m_tCond, &m_tMutex );

		m_iSent = 0;
		pthread_mutex_unlock ( &m_tMutex );
		return true;
	}

#ifdef HAVE_PTHREAD_COND_TIMEDWAIT
	struct timespec ts;
	clock_gettime ( CLOCK_REALTIME, &ts );

	int ns = ts.tv_nsec + ( iMsec % 1000 ) * 1000000;
	ts.tv_sec += ( ns / 1000000000 ) + ( iMsec / 1000 );
	ts.tv_nsec = ( ns % 1000000000 );

	int iRes = 0;
	pthread_mutex_lock ( &m_tMutex );
	while ( !m_iSent && !iRes )
		iRes = pthread_cond_timedwait ( &m_tCond, &m_tMutex, &ts );

	bool bEventHappened = iRes != ETIMEDOUT;
	if ( bEventHappened )
		m_iSent = 0;
	pthread_mutex_unlock ( &m_tMutex );
	return bEventHappened;
#endif
}

#endif