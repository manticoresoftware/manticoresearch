//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

/// @file searchdssl.h
/// SSL 

#pragma once

#include "networking_daemon.h"

#if WITH_SSL
	// set SSL key, certificate and ca-certificate to be used in SSL, if required.
	// does NOT anyway initialize SSL library or call any of it's funcitons.
	void SetServerSSLKeys ( CSphVariant * pSslCert, CSphVariant * pSslKey, CSphVariant * pSslCa );

	// Try to initialize SSL, if not yet done. Returns whether it is usable or not (i.e. - no lib, no keys, any error).
	// used to set 'switch-to-ssl' bit in mysql handshake depending from whether we can do it, or not.
    bool CheckWeCanUseSSL ( CSphString * pError=nullptr );

	// Replace pSource with it's SSL version.
	// any data not consumed from original source will be considered as part of ssl handshake.
	bool MakeSecureLayer ( std::unique_ptr<AsyncNetBuffer_c> & pSource );

#else
	// these stubs work together with NOT including searchdsll.cpp into the final build
	inline void SetServerSSLKeys ( CSphVariant *,  CSphVariant *,  CSphVariant * ) {}
	inline bool CheckWeCanUseSSL ( CSphString * pError=nullptr )
	{
		if ( pError )
			*pError="daemon built without SSL support";
		return false;
	}
	inline bool MakeSecureLayer ( std::unique_ptr<AsyncNetBuffer_c> & ) { return false; }
#endif
