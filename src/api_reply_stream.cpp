//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "api_reply_stream.h"
#include "mini_timer.h"
#include "searchdaemon.h"
#include "searchdha.h"

#include <cassert>


DWORD CalcRemoteHeartbeatIntervalMs ( int64_t iQueryTimeoutMs )
{
	return DWORD ( Max<int64_t> ( 1, iQueryTimeoutMs/4 ) );
}


struct ApiReplyStream_c::State_t
{
	State_t ( uint32_t uIntervalMs, WORD uReplyVersion, GenericOutputBuffer_c & tOut )
		: m_uIntervalMs ( uIntervalMs )
		, m_uReplyVersion ( uReplyVersion )
		, m_pOut ( &tOut )
	{}

	const uint32_t m_uIntervalMs;
	const WORD m_uReplyVersion;
	GenericOutputBuffer_c * m_pOut;
	Threads::Coro::Mutex_c m_tOutputLock;
	std::atomic<bool> m_bStopRequested { false };
	bool m_bWireFailed = false;
};


ApiReplyStream_c::ApiReplyStream_c ( uint32_t uIntervalMs, WORD uReplyVersion, GenericOutputBuffer_c & tOut )
	: m_pState ( new State_t ( uIntervalMs, uReplyVersion, tOut ) )
{
	assert ( uIntervalMs );
}


ApiReplyStream_c::~ApiReplyStream_c ()
{
	StopAndHandoff ();
}


void ApiReplyStream_c::Start ()
{
	assert ( !m_bStarted );
	assert ( !m_bHandedOff );

	m_bStarted = true;
	const auto pState = m_pState;
	const int64_t iFirstHeartbeatUS = sph::MicroTimer() + int64_t ( pState->m_uIntervalMs ) * 1000;
	auto pTimer = new MiniTimer_c ( "remote-reply-heartbeat" );
	pTimer->EngageAt ( iFirstHeartbeatUS, [pTimer, pState]
	{
		if ( !pState->m_bStopRequested.load ( std::memory_order_acquire ) )
			Threads::Coro::Go ( [pState] { Run ( pState ); }, GlobalWorkPool () );
		delete pTimer;
	} );
}


bool ApiReplyStream_c::StopAndHandoff ()
{
	if ( m_bHandedOff )
		return m_bCanSerializeTerminal;

	m_bHandedOff = true;
	if ( !m_bStarted )
		return true;

	const auto pState = m_pState;
	pState->m_bStopRequested.store ( true, std::memory_order_release );
	{
		Threads::Coro::ScopedMutex_t tLock ( pState->m_tOutputLock );
		m_bCanSerializeTerminal = !pState->m_bWireFailed;
	}

	return m_bCanSerializeTerminal;
}


void ApiReplyStream_c::Run ( SharedPtr_t<State_t> pState )
{
	const auto fnStopRequested = [&pState] { return pState->m_bStopRequested.load ( std::memory_order_acquire ); };

	while ( !fnStopRequested () )
	{
		{
			Threads::Coro::ScopedMutex_t tLock ( pState->m_tOutputLock );
			if ( fnStopRequested () )
				break;

			{
				auto tReply = APIAnswer ( *pState->m_pOut, pState->m_uReplyVersion, SEARCHD_IN_PROGRESS );
			}
			if ( !pState->m_pOut->Flush () )
			{
				pState->m_bWireFailed = true;
				pState->m_bStopRequested.store ( true, std::memory_order_release );
				break;
			}
			sphLogDebugv ( "remote reply heartbeat sent, version=%u, interval=%u ms", (unsigned)pState->m_uReplyVersion, (unsigned)pState->m_uIntervalMs );
		}

		if ( fnStopRequested () )
			break;
		Threads::Coro::SleepMsec ( int ( pState->m_uIntervalMs ) );
	}
}
