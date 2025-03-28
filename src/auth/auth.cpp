//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2011-2016, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "std/base64.h"
#include "searchdha.h"

#include "auth_common.h"
#include "auth_proto_mysql.h"
#include "auth_proto_http.h"
#include "auth_proto_api.h"
#include "auth.h"

static CSphMutex g_tAuthLock;
static AuthUsersPtr_t g_tAuth;

static CSphString g_sAuthFile;
static const CSphString g_sBuddyName ( "system.buddy" );

/////////////////////////////////////////////////////////////////////////////
/// auth load

static void AddBuddyToken ( const AuthUserCred_t * pSrcBuddy, AuthUsers_t & tDst );

void AuthConfigure ( const CSphConfigSection & hSearchd )
{
	CSphString sFile = hSearchd.GetStr ( "auth_user_file" );
	if ( sFile.IsEmpty() )
		return;

	g_sAuthFile = RealPath ( sFile );
	CSphString sError;
	AuthUsersPtr_t tAuth = ReadAuthFile ( g_sAuthFile, sError );
	if ( !tAuth )
		sphFatal ( "%s", sError.cstr() );

	AddBuddyToken ( nullptr, *tAuth );
	
	ScopedMutex_t tLock ( g_tAuthLock );
	g_tAuth = tAuth;
}

bool IsAuthEnabled()
{
	if ( !g_tAuth )
		return false;

	ScopedMutex_t tLock ( g_tAuthLock );
	return g_tAuth->m_hUserToken.GetLength();
}

CSphString GetBuddyToken()
{
	AuthUsersPtr_t pUsers = GetAuth();
	const AuthUserCred_t * pBuddy = pUsers->m_hUserToken ( g_sBuddyName );
	if ( !pBuddy )
		return CSphString();
	return pBuddy->m_sRawBearerSha256;
}

const CSphString CreateSessionToken()
{
	CSphString sToken;

	if ( !IsAuthEnabled() )
		return sToken;

	AuthUsersPtr_t pUsers = GetAuth();
	AuthUserCred_t * pAuth = pUsers->m_hUserToken ( session::GetUser() );
	if ( !pAuth )
		return sToken;

	sToken = pAuth->m_sRawBearerSha256;
	return sToken;
}

const AuthUsersPtr_t GetAuth()
{
	ScopedMutex_t tLock ( g_tAuthLock );
	return g_tAuth;
}

bool AuthReload ( CSphString & sError )
{
	if ( !IsAuthEnabled() )
	{
		sError = "authorization disabled, can not reload";
		return false;
	}

	AuthUsersPtr_t tAuth = ReadAuthFile ( g_sAuthFile, sError );
	if ( !tAuth )
		return false;

	ScopedMutex_t tLock ( g_tAuthLock );
	const AuthUserCred_t * pSrcBuddy = g_tAuth->m_hUserToken ( g_sBuddyName );
	AddBuddyToken ( pSrcBuddy, *tAuth );
	g_tAuth = tAuth;

	return true;
}

void AddBuddyToken ( const AuthUserCred_t * pSrcBuddy, AuthUsers_t & tAuth )
{
	AuthUserCred_t tBuddy;
	if ( !pSrcBuddy )
	{
		tBuddy.m_sUser = g_sBuddyName;

		SHA1_c tSaltHash;
		tSaltHash.Init();
		DWORD uScramble[4] = { sphRand(), sphRand(), sphRand(), sphRand() };
		tSaltHash.Update ( (const BYTE *)&uScramble, sizeof ( uScramble ) );
		int64_t iDT = sphMicroTimer();
		tSaltHash.Update ( (const BYTE *)&iDT, sizeof ( iDT ) );
		tSaltHash.Update ( (const BYTE *)tBuddy.m_sUser.cstr(), tBuddy.m_sUser.Length() );
		tBuddy.m_sSalt = BinToHex ( tSaltHash.FinalHash() );

		SHA1_c tSaltPwd1;
		tSaltPwd1.Init();
		tSaltPwd1.Update ( (const BYTE *)tBuddy.m_sSalt.cstr(), tBuddy.m_sSalt.Length() );
		tSaltPwd1.Update ( (const BYTE *)tBuddy.m_sUser.cstr(), tBuddy.m_sUser.Length() );
		tBuddy.m_tPwdSha1 = tSaltPwd1.FinalHash();

		std::unique_ptr<SHA256_i> pPwd256 { CreateSHA256() };
		pPwd256->Init();
		pPwd256->Update ( (const BYTE *)tBuddy.m_sSalt.cstr(), tBuddy.m_sSalt.Length() );
		pPwd256->Update ( (const BYTE *)tBuddy.m_sUser.cstr(), tBuddy.m_sUser.Length() );
		tBuddy.m_sPwdSha256 = BinToHex ( pPwd256->FinalHash() );

		std::unique_ptr<SHA256_i> pToken { CreateSHA256() };
		pToken->Init();
		pToken->Update ( (const BYTE *)tBuddy.m_sSalt.cstr(), tBuddy.m_sSalt.Length() );
		pToken->Update ( (const BYTE *)tBuddy.m_sPwdSha256.cstr(), tBuddy.m_sPwdSha256.Length() );
		tBuddy.m_sRawBearerSha256 = tBuddy.m_sBearerSha256 = BinToHex ( pToken->FinalHash() );

		pSrcBuddy = &tBuddy;
	}

	tAuth.m_hUserToken.Add ( *pSrcBuddy, pSrcBuddy->m_sUser );
	tAuth.m_hHttpToken2User.Add ( pSrcBuddy->m_sUser, pSrcBuddy->m_sRawBearerSha256 );
	tAuth.m_hApiToken2User.Add ( pSrcBuddy->m_sUser, pSrcBuddy->m_sBearerSha256 );
}
