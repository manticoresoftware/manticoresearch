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

#include "searchdha.h"

#include "auth_common.h"
#include "auth_proto_api.h"
#include "auth.h"

/////////////////////////////////////////////////////////////////////////////
/// API

int GetApiTokenSize ( ApiAuth_e eType )
{
	switch ( eType )
	{
	case ApiAuth_e::SHA1: return HASH20_SIZE; break;
	case ApiAuth_e::SHA256: return HASH256_SIZE; break;

	case ApiAuth_e::NO_AUTH:
	default:
		return 0;
	}
}

bool CheckAuth ( ApiAuth_e eType, const VecTraits_T<BYTE> & dToken, CSphString & sUser, CSphString & sError )
{
	sphLogDebug ( "%d, token(%d):0x%s", (int)eType, dToken.GetLength(), BinToHex ( dToken.Begin(), dToken.GetLength() ).cstr() );

	if ( !IsAuthEnabled() )
		return true;

	CSphString sToken;
	sToken.SetBinary ( (const char *)dToken.Begin(), dToken.GetLength() );
	AuthUsersPtr_t pUsers = GetAuth();
	const CSphString * pUser = pUsers->m_hApiToken2User ( sToken );
	if ( !pUser )
	{
		sError.SetSprintf ( "Access denied for token '%s'", BinToHex ( dToken.Begin(), dToken.GetLength() ).cstr() );
		return false;
	}

	sphLogDebug ( "user %s passed", pUser->cstr() );

	sUser = *pUser;
	return true;
}

static void SetAuth ( const char * sBearerSha256, int iLen, AgentConn_t * pAgent )
{
	assert ( sBearerSha256 && iLen>0 );

	pAgent->m_tAuthToken.m_eType = ApiAuth_e::SHA256;
	pAgent->m_tAuthToken.m_dToken.Reset ( iLen );
	memcpy ( pAgent->m_tAuthToken.m_dToken.Begin(), sBearerSha256, iLen );
}


void SetAuth ( const CSphString & sUser, AgentConn_t * pAgent )
{
	if ( sUser.IsEmpty() || !pAgent )
		return;

	if ( !IsAuthEnabled() )
		return;

	AuthUsersPtr_t pUsers = GetAuth();
	const AuthUserCred_t * pUser = pUsers->m_hUserToken ( sUser );
	if ( !pUser )
		return;

	SetAuth ( pUser->m_sBearerSha256.cstr(), pUser->m_sBearerSha256.Length(), pAgent );
}

void SetAuth ( const CSphString & sUser, CSphVector<AgentConn_t *> & dRemotes )
{
	if ( sUser.IsEmpty() || dRemotes.IsEmpty() )
		return;

	if ( !IsAuthEnabled() )
		return;

	AuthUsersPtr_t pUsers = GetAuth();
	const AuthUserCred_t * pUser = pUsers->m_hUserToken ( sUser );
	if ( !pUser )
		return;

	// FIXME!!! replace by FixedVector
	int iTokenLen = pUser->m_sBearerSha256.Length();
	const char * sBearerSha256 = pUser->m_sBearerSha256.cstr();
	for ( auto & tClient : dRemotes )
		SetAuth ( sBearerSha256, iTokenLen, tClient );
}

void SetSessionAuth ( CSphVector<AgentConn_t *> & dRemotes )
{
	SetAuth ( session::GetUser(), dRemotes );
}

bool ApiCheckPerms ( const CSphString & sUser, AuthAction_e eAction, const CSphString & sTarget, ISphOutputBuffer & tOut )
{
	if ( !IsAuthEnabled() )
	{
		sphLogDebug ( "no users found in config, permission granted" );
		return true;
	}

	CSphString sError;
	if ( CheckPerms ( sUser, eAction, sTarget, false, sError ) )
		return true;

	SendErrorReply ( tOut, "%s", sError.cstr() );
	return false;
}

bool ApiCheckClusterPerms ( const CSphString & sUser, ISphOutputBuffer & tOut )
{
	if ( !IsAuthEnabled() )
	{
		sphLogDebug ( "no users found in config, permission granted" );
		return true;
	}

	CSphString sError;
	if ( CheckPerms ( sUser, AuthAction_e::REPLICATION, CSphString(), true, sError ) )
		return true;

	SendErrorReply ( tOut, "%s", sError.cstr() );
	return false;
}