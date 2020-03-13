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

struct SslClient_i;
struct NetStateHttps_t final : public NetStateAPI_t
{
	SslClient_i * m_pSession = nullptr;
	CSphVector<BYTE> m_dDecrypted;

	~NetStateHttps_t() final;
};

class NetReceiveDataHttps_c final : public ISphNetAction
{
	class Impl_c;
	Impl_c * m_pImpl = nullptr;

public:
	explicit NetReceiveDataHttps_c ( NetStateHttps_t * pState );
	~NetReceiveDataHttps_c () final;

	NetEvent_e		Loop ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop ) final;
	NetEvent_e		Setup ( int64_t tmNow ) final;
	void			CloseSocket () final;
};

NetSendData_t * MakeReplyHttps_t ( NetStateAPI_t * pState );
