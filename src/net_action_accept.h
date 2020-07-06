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

#include "networking_daemon.h"
#include "netstate_api.h"

class NetActionAccept_c final : public ISphNetAction
{
	class Impl_c;
	Impl_c * m_pImpl = nullptr;

public:
	explicit NetActionAccept_c ( const Listener_t & tListener );
	~NetActionAccept_c() final;

	void Process ( DWORD uGotEvents, CSphNetLoop * pLoop ) final;
};

// helper to be used from plain workers
void FillNetState ( NetConnection_t * pState, int iClientSock, int iConnID, bool bVIP, bool bSSL,
		const sockaddr_storage & saStorage );