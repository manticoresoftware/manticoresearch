//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxstd.h"
#include "searchdha.h"

extern int g_tmWait;
extern int g_iThrottleAction;
extern ISphThdPool * g_pThdPool;
extern int g_iThrottleAccept;
extern int g_iConnectionID;        ///< global conn-id

struct Listener_t
{
	int					m_iSock;
	bool				m_bTcp;
	Proto_e				m_eProto;
	bool				m_bVIP;
};

enum NetEvent_e : DWORD
{
	NE_KEEP = 0,
	NE_IN = NetPollEvent_t::READ,
	NE_OUT = NetPollEvent_t::WRITE,
	NE_HUP = NetPollEvent_t::HUP,
	NE_ERR = NetPollEvent_t::ERR,
	NE_MASK = NE_IN | NE_OUT | NE_HUP | NE_ERR,
	NE_REMOVE = 1UL<<4,
	NE_REMOVED = 1UL<<5,
};

class CSphNetLoop;
struct ISphNetAction : ISphNoncopyable, NetPollEvent_t
{
	explicit ISphNetAction ( int iSock ) : NetPollEvent_t ( iSock ) {}
	virtual NetEvent_e		Loop ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop ) = 0;
	virtual NetEvent_e		Setup ( int64_t tmNow ) = 0;
	virtual bool			GetStats ( int & ) { return false; }
	virtual void			CloseSocket () = 0;
};

// event that wakes-up poll net loop from finished thread pool job
// declared here to make available for testing
class CSphWakeupEvent final : public PollableEvent_t, public ISphNetAction
{
public:
	CSphWakeupEvent ();
	~CSphWakeupEvent () final;
	void Wakeup ();
	NetEvent_e Loop ( DWORD uGotEvents, CSphVector<ISphNetAction *> &, CSphNetLoop * ) final;
	NetEvent_e Setup ( int64_t ) final;
	void CloseSocket () final;
};

/////////////////////////////////////////////////////////////////////////////
/// NETWORK THREAD
/////////////////////////////////////////////////////////////////////////////

class CSphNetLoop
{
	class Impl_c;
	Impl_c * m_pImpl = nullptr;

public:
	DWORD							m_uTick = 0;

public:
	explicit CSphNetLoop ( const VecTraits_T<Listener_t> & dListeners );
	~CSphNetLoop();
	void LoopNetPoll ();

	void Kick ();
	void AddAction ( ISphNetAction * pElem );
	void RemoveIterEvent ( NetPollEvent_t * pEvent );

};

// main entry point - creates netloop and loop it
void ServeNetLoop ( const VecTraits_T<Listener_t> & dListeners );
