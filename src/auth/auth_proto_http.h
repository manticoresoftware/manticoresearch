//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2011-2016, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxstd.h"
#include "auth_perms.h"

// HTTP
struct HttpProcessResult_t;

bool CheckAuth ( const SmallStringHash_T<CSphString> & hOptions, HttpProcessResult_t & tRes, CSphVector<BYTE> & dReply, CSphString & sUser );
CSphString GetBuddyToken();
const CSphString CreateSessionToken();

bool HttpCheckPerms ( const CSphString & sUser, AuthAction_e eAction, const CSphString & sTarget, EHTTP_STATUS & eReplyHttpCode, CSphString & sError, CSphVector<BYTE> & dReply );
