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

// API
enum class ApiAuth_e : BYTE
{
	NO_AUTH = 1,
	SHA1,
	SHA256
};
int GetApiTokenSize ( ApiAuth_e eType );
bool CheckAuth ( ApiAuth_e eType, const VecTraits_T<BYTE> & dToken, CSphString & sUser, CSphString & sError );

struct ApiAuthToken_t
{
	ApiAuth_e m_eType = ApiAuth_e::NO_AUTH;
	CSphFixedVector<BYTE> m_dToken  { 0 };
};

bool ApiCheckPerms ( const CSphString & sUser, AuthAction_e eAction, const CSphString & sTarget, ISphOutputBuffer & tOut );
// FIXME!!! add cluster name to all api replication command and check proper perms
bool ApiCheckClusterPerms ( const CSphString & sUser, ISphOutputBuffer & tOut ); // !COMMIT