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
#if 0
struct NetStateAPI_t : NetConnection_t
{
	bool				m_bKeepSocket = false;
	int64_t				m_iLeft = 0;
	int64_t				m_iPos = 0;

	NetStateAPI_t ();
	virtual ~NetStateAPI_t ();

	int64_t SocketIO ( bool bWrite, bool bAfterWrite = false );
	void CloseSocket ();
};

struct NetSendData_t : public ISphNetAction
{
	CSphScopedPtr<NetStateAPI_t>		m_tState;
	Proto_e								m_eProto;
	bool								m_bContinue;

	NetSendData_t ( NetStateAPI_t * pState, Proto_e eProto );

	NetEvent_e		Loop ( DWORD uGotEvents, CSphVector<ISphNetAction *> & dNextTick, CSphNetLoop * pLoop ) override;
	NetEvent_e		Setup ( int64_t tmNow ) override;
	void			CloseSocket () override;

	void SetContinue () { m_bContinue = true; }
};
#endif

/// helpers
bool CheckSocketError ( DWORD uGotEvents );