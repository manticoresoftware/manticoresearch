//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "networking_daemon.h"

enum class NetloopState_e : BYTE;
struct ListenTaskInfo_t : public TaskInfo_t
{
	DECLARE_RENDER( ListenTaskInfo_t );
	NetloopState_e m_eThdState;
	DWORD	m_uWorks = 0;
	DWORD	m_uTick = 0;
};

class NetActionAccept_c final : public ISphNetAction
{
	class Impl_c;
	std::unique_ptr<Impl_c> m_pImpl;

protected:
	~NetActionAccept_c () final;

public:
	NetActionAccept_c ( const Listener_t & tListener, CSphNetLoop* pNetLoop );
	void Process () final;
	void NetLoopDestroying() final;
};
