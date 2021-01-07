//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "networking_daemon.h"
#include "netstate_api.h"

// helper
bool CheckSocketError ( DWORD uGotEvents )
{
	const auto ERRHUP = NetPollEvent_t::ERR | NetPollEvent_t::HUP;
	bool bReadError = ( ( uGotEvents & NetPollEvent_t::READ ) && ( uGotEvents & ERRHUP ) );
	bool bWriteError = ( ( uGotEvents & NetPollEvent_t::WRITE ) && ( uGotEvents & NetPollEvent_t::ERR ) );

	if ( bReadError && (( uGotEvents & ERRHUP)==ERRHUP) )
		sphSockSetErrno ( ECONNRESET );

	return bReadError || bWriteError;
}
