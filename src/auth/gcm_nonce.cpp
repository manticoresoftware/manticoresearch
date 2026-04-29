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

class NonceGenerator_c : public NonceGenerator_i
{
public:
	NonceGenerator_c();
	~NonceGenerator_c() override = default;
	void Generate ( DWORD uServerId, bool bIsMaster, BYTE * pNonce ) override;

private:
	DWORD m_uBootId; // random ID generated on startup
	std::atomic<DWORD> m_uCounter;
};


class NonceValidator_c : public NonceValidator_i
{
public:
	~NonceValidator_c() override = default;
	bool Validate ( const BYTE * pNonce, bool bIsMaster, CSphString & sError ) override;

private:
	static constexpr int REPLAY_WINDOW_SIZE = 256;

	struct AgentState_t
	{
		DWORD m_uLastBootId = 0;
		DWORD m_uLastCounter = 0;
		BitVec_T<DWORD, REPLAY_WINDOW_SIZE> m_tReplayWindow { REPLAY_WINDOW_SIZE };

		bool ValidateCounter ( DWORD uCounter, DWORD uAgentId, CSphString & sError ) const;
		void SetCounterInWindow ( DWORD uCounter);
		void SetCounterNewWindow ( DWORD uCounter );
		void ShiftBitmapLeft ( DWORD iShift );

		AgentState_t() = default;
		AgentState_t ( DWORD uBootId, DWORD uLastCounter );
	};

	CSphMutex m_tLock;
	CSphOrderedHash<AgentState_t, DWORD, IdentityHash_fn, 1024> m_hStates;
};

static NonceGenerator_c g_tNonceGenerator;
static NonceValidator_c g_tNonceValidator;

NonceGenerator_i &  GetNonceGen()
{
	return g_tNonceGenerator;
}

NonceValidator_i & GetNonceValidator()
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

	DWORD uAgentId = tNonce.GetAgentId(); // this is the sender ID
	DWORD uBootId = tNonce.m_uBootId;
	DWORD uCounter = tNonce.m_uCounter;

	CSphScopedLock tLock ( m_tLock );
	AgentState_t * pState = m_hStates ( uAgentId );
	if ( !pState )
	{
		m_hStates.Add ( AgentState_t ( uBootId, uCounter ), uAgentId );
		return true;
	}

	if ( uBootId>pState->m_uLastBootId )
	{
		*pState = AgentState_t ( uBootId, uCounter );
		return true;
	}

	if ( uBootId<pState->m_uLastBootId )
	{
		sError.SetSprintf ( "nonce validation failed for agent %u, received stale boot_id %u (expected %u)", uAgentId, uBootId, pState->m_uLastBootId );
		return false;
	}

	// if highest counter - update state and accept
	if ( uCounter>pState->m_uLastCounter )
	{
		pState->SetCounterNewWindow ( uCounter );
		return true;
	}

	if ( !pState->ValidateCounter ( uCounter, uAgentId, sError ) )
		return false;

	// valid bit out-of-order counter within the window, mark and accept
	pState->SetCounterInWindow ( uCounter );

	return true;
}

bool NonceValidator_c::AgentState_t::ValidateCounter ( DWORD uCounter, DWORD uAgentId, CSphString & sError ) const
{
	assert ( uCounter<=m_uLastCounter );

	// counter too old \ outside the window
	if ( ( m_uLastCounter-uCounter )>=REPLAY_WINDOW_SIZE )
	{
		sError.SetSprintf ( "nonce validation failed for agent %u: counter %u is too old ( last seen %u )", uAgentId, uCounter, m_uLastCounter );
		return false;
	}

	// counter replay detected \ already seen within the window
	DWORD uPos = m_uLastCounter - uCounter;
	if ( m_tReplayWindow.BitGet ( uPos ) )
	{
		sError.SetSprintf ( "nonce validation failed for agent %u: replay detected, counter %u already seen", uAgentId, uCounter );
		return false;
	}

	return true;
}

void NonceValidator_c::AgentState_t::SetCounterInWindow ( DWORD uCounter)
{
	assert ( uCounter<=m_uLastCounter && ( m_uLastCounter-uCounter)<REPLAY_WINDOW_SIZE );

	DWORD uPos = m_uLastCounter - uCounter;
	m_tReplayWindow.BitSet ( uPos );
}

void NonceValidator_c::AgentState_t::ShiftBitmapLeft ( DWORD iShift )
{
	assert ( iShift>0 );

	if ( iShift>=REPLAY_WINDOW_SIZE )
	{
		m_tReplayWindow.Clear();
		return;
	}

	DWORD * pData = m_tReplayWindow.Begin();
	constexpr int BITS_PER_DWORD = sizeof(DWORD) * 8;

	int iWordShift = iShift / BITS_PER_DWORD;
	int iBitShift = iShift % BITS_PER_DWORD;

	int iStorageSize = ( REPLAY_WINDOW_SIZE + BITS_PER_DWORD - 1 ) / BITS_PER_DWORD;

	if ( iBitShift==0 )
	{
		memmove( pData+iWordShift, pData, ( iStorageSize - iWordShift) * sizeof(DWORD) );
	}
	else
	{
		for (int i = iStorageSize-1; i>=iWordShift; --i )
		{
			DWORD uWord = ( i>=iWordShift ) ? pData[i-iWordShift] : 0;
			DWORD uCarry = ( i>iWordShift ) ? pData[i-iWordShift-1] : 0;
			
			pData[i] = ( uWord<<iBitShift ) | ( uCarry>>( BITS_PER_DWORD-iBitShift ) );
		}
	}

	// clear the lower words were shifted away
	if ( iWordShift>0 )
		memset ( pData, 0, iWordShift * sizeof(DWORD) );
}

void NonceValidator_c::AgentState_t::SetCounterNewWindow ( DWORD uCounter )
{
	assert ( uCounter>m_uLastCounter );

	DWORD uShift = uCounter - m_uLastCounter;
	ShiftBitmapLeft ( uShift );
	
	// mark the new highest counter
	m_tReplayWindow.BitSet ( 0 );
	m_uLastCounter = uCounter;
}

NonceValidator_c::AgentState_t::AgentState_t ( DWORD uBootId, DWORD uLastCounter )
{
	m_uLastBootId = uBootId;
	m_uLastCounter = uLastCounter;
	m_tReplayWindow.Clear();
	m_tReplayWindow.BitSet ( 0 );
}