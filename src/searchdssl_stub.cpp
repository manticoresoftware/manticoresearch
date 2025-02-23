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

#include "searchdssl.h"

#if WITH_SSL

// these stubs for non-daemon (i.e. for tests)
void SetServerSSLKeys ( const CSphString & ,  const CSphString & ,  const CSphString & ) {}
bool CheckWeCanUseSSL ( CSphString * pError )
{
	if ( pError )
		*pError="daemon built without SSL support";
	return false;
}
bool MakeSecureLayer ( std::unique_ptr<AsyncNetBuffer_c> & ) { return false; }
#endif
