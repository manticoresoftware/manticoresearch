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

#include "sphinxstd.h"

#include "auth_common.h"
#include "auth.h"

#include "auth_perms.h"

static bool ProcessPerms ( const UserPerms_t * pPerms, AuthAction_e eAction, const CSphString & sTarget )
{
	for ( const auto & tPerm : *pPerms )
	{
		if ( tPerm.m_eAction!=eAction )
			continue;

		if ( !tPerm.m_bTargetWildcard && tPerm.m_sTarget==sTarget )
			return tPerm.m_bAllow;

		if ( tPerm.m_bTargetWildcard && ( sTarget.IsEmpty() || sphWildcardMatch ( sTarget.cstr(), tPerm.m_sTarget.cstr() ) ) )
			return tPerm.m_bAllow;
	}

	return false;
}

bool CheckPerms ( const CSphString & sUser, AuthAction_e eAction, const CSphString & sTarget, CSphString & sError )
{
	assert ( IsAuthEnabled() );

	AuthUsersPtr_t pUsers = GetAuth();
	const UserPerms_t * pPerms = pUsers->m_hUserPerms ( sUser );
	if ( pPerms && pPerms->GetLength() && ProcessPerms ( pPerms, eAction, sTarget ) )
		return true;

	sError.SetSprintf ( "Permission denied for user '%s'", sUser.cstr() );
	return false;
}
