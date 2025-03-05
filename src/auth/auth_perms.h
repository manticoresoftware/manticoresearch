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

enum class AuthAction_e
{
	READ,
	WRITE,
	SCHEMA,
	REPLICATION,
	ADMIN,

	UNKNOWN
};

struct UserPerm_t
{
	AuthAction_e m_eAction;
	CSphString m_sTarget;
	bool m_bAllow = false;

	bool m_bTargetWildcard = false;
};

// FIME!!! add offset for each action inside perms vector
using UserPerms_t = CSphVector<UserPerm_t>;

bool CheckPerms ( const CSphString & sUser, AuthAction_e eAction, const CSphString & sTarget, CSphString & sError );
