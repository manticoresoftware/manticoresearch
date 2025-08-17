//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2011-2016, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "auth.h"

#include "gcm_nonce.h"

// FIXME!!! use InputBuffer_c \ ISphOutputBuffer htonl, ntohl
#if _WIN32
	#include <winsock2.h>
#else
#include <arpa/inet.h> 
#endif

static NonceGenerator_c g_tNonceGenerator;
static NonceValidator_c g_tNonceValidator;

NonceGenerator_c &  GetNonceGen()
{
	return g_tNonceGenerator;
}

NonceValidator_c & GetNonceValidator()
{
	return g_tNonceValidator;
}

/////////////////////////////////////////////////////////////////////////////
// Nonce_t

void Nonce_t::Pack ( BYTE * pNonce ) const
{
	DWORD uDirAgentId = htonl ( m_uDirAgentId );
	DWORD uNetBootId = htonl ( m_uBootId );
	DWORD uNetCounter = htonl ( m_uCounter );

	memcpy ( pNonce, &uDirAgentId, 4 );
	memcpy ( pNonce + 4, &uNetBootId, 4 );
	memcpy ( pNonce + 8, &uNetCounter, 4 );
}

bool Nonce_t::Unpack ( const BYTE * pNonce )
{
	if ( !pNonce )
		return false;

	memcpy ( &m_uDirAgentId, pNonce, 4 );
	memcpy ( &m_uBootId, pNonce + 4, 4 );
	memcpy ( &m_uCounter, pNonce + 8, 4 );

	m_uDirAgentId = ntohl ( m_uDirAgentId );
	m_uBootId = ntohl ( m_uBootId );
	m_uCounter = ntohl ( m_uCounter );
	
	return true;
}

static const DWORD DIR_MASK = 0x80000000;
static const DWORD AGENT_ID_MASK = 0x7FFFFFFF;

void Nonce_t::SetDirection ( bool bIsMaster )
{
    if ( bIsMaster )
        m_uDirAgentId |= DIR_MASK;
    else
        m_uDirAgentId &= ~DIR_MASK;
}

void Nonce_t::SetAgentId ( DWORD uAgentId )
{
    m_uDirAgentId = ( ( m_uDirAgentId & DIR_MASK ) | ( uAgentId & AGENT_ID_MASK ) );
}

bool Nonce_t::IsMaster() const
{
	return ( ( m_uDirAgentId & DIR_MASK )!=0 );
}
DWORD Nonce_t::GetAgentId() const
{
	return ( m_uDirAgentId & AGENT_ID_MASK );
}


/////////////////////////////////////////////////////////////////////////////
// NonceGenerator_c

NonceGenerator_c::NonceGenerator_c()
	: m_uCounter ( 1 )
{
	// generate monotonic unique boot_id using time-based ratchet and rand

	// 1) the time part
	uint64_t uStartedSec = sphMicroTimer() / 1000000;
	const uint64_t uBaseSec = 1556668800; // 01 May 2019 00:00:00 GMT
	if ( uStartedSec > uBaseSec )
		uStartedSec -= uBaseSec;

	// lower 24 bits for the time part
	DWORD uTimePart = ((DWORD) uStartedSec ) & 0x00FFFFFF;

	// 2) the random part
	DWORD uRandPart = rand() & 0xFF;

	// final 32 bit boot_id
	// boot_id = [ 24 bits TIME | 8 bits RANDOM ]
	m_uBootId = ( uTimePart << 8 ) | uRandPart;
}

void NonceGenerator_c::Generate ( DWORD uServerId, bool bIsMaster, BYTE * pNonce )
{
	DWORD uCounter = m_uCounter.fetch_add ( 1, std::memory_order_relaxed );

	// FIXME!!! generate new m_uBootId on counter overflow

	Nonce_t tNonce;
	tNonce.SetAgentId ( uServerId );
	tNonce.SetDirection ( bIsMaster );
	tNonce.m_uBootId = m_uBootId;
	tNonce.m_uCounter = uCounter;

	tNonce.Pack ( pNonce );
}


/////////////////////////////////////////////////////////////////////////////
// NonceValidator_c

bool NonceValidator_c::Validate ( const BYTE * pNonce, bool bIsMaster, CSphString & sError )
{
	Nonce_t tNonce;
	if ( !tNonce.Unpack ( pNonce ) )
	{
		sError = "failed to unpack nonce";
		return false;
	}
	
	if ( tNonce.IsMaster()!=bIsMaster )
	{
		sError.SetSprintf ( "nonce validation failed: invalid message direction" );
		return false;
	}

	uint32_t uAgentId = tNonce.GetAgentId(); // this is the sender ID
	uint32_t uBootId = tNonce.m_uBootId;
	uint32_t uCounter = tNonce.m_uCounter;

	CSphScopedLock tLock ( m_tLock );
	AgentState_t * pState = m_hStates ( uAgentId );
	if ( !pState )
	{
		m_hStates.Add ( { uBootId, uCounter }, uAgentId );
		return true;
	}

	if ( uBootId>pState->m_uLastBootId )
	{
		pState->m_uLastBootId = uBootId;
		pState->m_uLastCounter = uCounter;
		return true;
	}

	if ( uBootId<pState->m_uLastBootId )
	{
		sError.SetSprintf ( "nonce validation failed for agent %u, received stale boot_id %u (expected %u)", uAgentId, uBootId, pState->m_uLastBootId );
		return false;
	}

	if ( uCounter<=pState->m_uLastCounter )
	{
		sError.SetSprintf ( "nonce validation failed for agent %u: replay detected, received counter %u, expected > %u", uAgentId, uCounter, pState->m_uLastCounter );
		return false;
	}

	pState->m_uLastCounter = uCounter;
	return true;
}
