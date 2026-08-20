//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "coroutine.h"
#include "sphinxstd.h"

#include <atomic>
#include <cstdint>
class GenericOutputBuffer_c;

DWORD CalcRemoteHeartbeatIntervalMs ( int64_t iQueryTimeoutMs );


class ApiReplyStream_c : public ISphNoncopyable
{
public:
	ApiReplyStream_c ( uint32_t uIntervalMs, WORD uReplyVersion, GenericOutputBuffer_c & tOut );
	~ApiReplyStream_c ();

	void Start ();
	bool StopAndHandoff ();

private:
	struct State_t;
	static void Run ( SharedPtr_t<State_t> pState );

	SharedPtr_t<State_t> m_pState;
	bool m_bStarted = false;
	bool m_bHandedOff = false;
	bool m_bCanSerializeTerminal = true;
};
