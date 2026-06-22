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
#include "auth_log.h"
#include "auth.h"

#include "auth_perms.h"

static bool ProcessPerms ( const UserPerms_t * pPerms, AuthAction_e eAction, const CSphString & sTarget, bool bAllowEmpty )
{
	bool bGotEmptyTarget = sTarget.IsEmpty();
	bool bHasAllow = false;

	for ( const auto & tPerm : *pPerms )
	{
		if ( tPerm.m_eAction!=eAction )
			continue;

		if ( !tPerm.m_bTargetWildcard && tPerm.m_sTarget==sTarget )
			return tPerm.m_bAllow;

		if ( tPerm.m_bTargetWildcard && !bGotEmptyTarget && ( tPerm.m_bTargetWildcardAll || sphWildcardMatch ( sTarget.cstr(), tPerm.m_sTarget.cstr() ) ) )
			return tPerm.m_bAllow;

		// actions without target denny only in case no allow rules found
		// the resolution goes at the next consequent statements
		if ( bAllowEmpty && bGotEmptyTarget )
		{
			if ( tPerm.m_bTargetWildcardAll )
				return tPerm.m_bAllow;

			bHasAllow |= tPerm.m_bAllow;
		}
	}

	return ( bAllowEmpty && bHasAllow );
}

bool CheckPerms ( const CSphString & sUser, AuthAction_e eAction, const CSphString & sTarget, bool bAllowEmpty, CSphString & sError )
{
	assert ( IsAuthEnabled() );

	AuthUsersPtr_t pUsers = GetAuth();
	const UserPerms_t * pPerms = pUsers->m_hUserPerms ( sUser );
	if ( pPerms && pPerms->GetLength() && ProcessPerms ( pPerms, eAction, sTarget, bAllowEmpty ) )
		return true;

	sError.SetSprintf ( "Permission denied for user '%s'", sUser.cstr() );
	AuthLog().AuthDenied ( sUser, session::szClientName(), eAction, sTarget );

	return false;
}

void UserPerm_t::SetTarget ( const CSphString & sTarget )
{
	m_sTarget = sTarget;
	m_sTarget.Trim();
	m_bTargetWildcard = HasWildcard ( m_sTarget.cstr() );
	m_bTargetWildcardAll = ( m_sTarget=="*" );
}