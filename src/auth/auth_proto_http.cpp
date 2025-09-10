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

#include "std/base64.h"
#include "searchdhttp.h"

#include "auth_common.h"
#include "auth_log.h"
#include "auth.h"

/////////////////////////////////////////////////////////////////////////////
/// HTTP

HASH256_t GetPwdHash256 ( const AuthUserCred_t & tEntry, const CSphString & sPwd )
{
	std::unique_ptr<SHA256_i> pHasher { CreateSHA256() };
	pHasher->Init();
	pHasher->Update ( tEntry.m_dSalt.Begin(), tEntry.m_dSalt.GetLength() );
	pHasher->Update ( (const BYTE*)sPwd.cstr(), sPwd.Length() );
	return pHasher->FinalHash();
}

static bool CheckPwd ( const AuthUserCred_t & tEntry, const CSphString & sPwd )
{
	HASH256_t tPwdHash = GetPwdHash256 ( tEntry, sPwd );
	int iCmp = memcmp ( tEntry.m_dPwdSha256.Begin(), tPwdHash.data(), tPwdHash.size() );
	return ( iCmp==0 );
}

static bool FailAuth ( HttpProcessResult_t & tRes, CSphVector<BYTE> & dReply )
{
	tRes.m_eReplyHttpCode = EHTTP_STATUS::_401;
	tRes.m_bSkipBuddy = true;
	sphHttpErrorReply ( dReply, tRes.m_eReplyHttpCode, tRes.m_sError.cstr() );
	return false;
}

static Str_t GetTokenFromAuth ( const CSphString & sSrcAuth, int iPrefixLen )
{
	int iSrcAuthLen = sSrcAuth.Length();
	const char * sCur = sSrcAuth.cstr() + iPrefixLen;
	const char * sEnd = sSrcAuth.cstr() + iSrcAuthLen;
	while ( sCur<sEnd && isspace ( *sCur ) )
		sCur++;

	return Str_t ( sCur, iSrcAuthLen - ( sCur - sSrcAuth.cstr() ) );
}

static bool CheckAuthBasic ( const Str_t & sSrcUserPwd, HttpProcessResult_t & tRes, CSphVector<BYTE> & dReply, CSphString & sUser )
{
	CSphVector<BYTE> dSrcUserPwd;
	DecodeBinBase64 ( sSrcUserPwd, dSrcUserPwd );

	int iDel = dSrcUserPwd.GetFirst ( []( BYTE c ) { return c==':'; } );
	if ( iDel==-1 )
	{
		tRes.m_sError = "Failed to find user:password delimiter";
		AuthLog().AuthFailure ( sUser, AccessMethod_e::HTTP_BASIC, session::szClientName(), "malformed authentication string - missing delimiter" );
		return FailAuth ( tRes, dReply );
	}

	sUser.SetBinary ( (const char *)dSrcUserPwd.Begin(), iDel );
	CSphString sPwd;
	sPwd.SetBinary ( (const char *)dSrcUserPwd.Begin() + iDel + 1, dSrcUserPwd.GetLength()-iDel-1 );

	AuthUsersPtr_t pUsers = GetAuth();
	const AuthUserCred_t * pUser = pUsers->m_hUserToken ( sUser );
	if ( !pUser || sPwd.IsEmpty() )
	{
		tRes.m_sError.SetSprintf ( "Access denied for user '%s' (using password: NO)", sUser.cstr() );
		if ( !pUser )
			AuthLog().AuthFailure ( sUser, AccessMethod_e::HTTP_BASIC, session::szClientName(), "user does not exist" );
		else
			AuthLog().AuthFailure ( sUser, AccessMethod_e::HTTP_BASIC, session::szClientName(), "no password provided" );
		return FailAuth ( tRes, dReply );
	}

	if ( !CheckPwd ( *pUser, sPwd ) )
	{
		tRes.m_sError.SetSprintf ( "Access denied for user '%s' (using password: YES)", sUser.cstr() );
		AuthLog().AuthFailure ( sUser, AccessMethod_e::HTTP_BASIC, session::szClientName(), "invalid password" );
		return FailAuth ( tRes, dReply );
	} else
	{
		AuthLog().AuthSuccess ( sUser, AccessMethod_e::HTTP_BASIC, session::szClientName() );
		return true;
	}
}

static bool CheckBearerMatched ( const HASH256_t & tHashTokenData, const AuthUserCred_t & tUser, CSphString & sUser )
{
	std::unique_ptr<SHA256_i> pHashBearer256 { CreateSHA256() };
	pHashBearer256->Init();
	pHashBearer256->Update ( tUser.m_dSalt.Begin(), tUser.m_dSalt.GetLength() );
	pHashBearer256->Update ( tHashTokenData.data(), tHashTokenData.size() );
	auto tBearer256 = pHashBearer256->FinalHash();

	if ( memcmp ( tUser.m_dBearerSha256.Begin(), tBearer256.data(), tBearer256.size() )==0 )
	{
		GetBearerCache().AddUser ( tHashTokenData, tUser.m_sUser );
		sUser = tUser.m_sUser;

		return true;
	} else
		return false;
}

static bool CheckCacheMatched ( const HASH256_t & tHashTokenData, const AuthUsersPtr_t & pUsers, CSphString & sUser )
{
	CSphString sCachedUser = GetBearerCache().FindUser ( tHashTokenData );
	if ( sCachedUser.IsEmpty() )
		return false;

	const auto * pUser = pUsers->m_hUserToken ( sCachedUser );
	if ( pUser && CheckBearerMatched ( tHashTokenData, *pUser, sUser ) )
		return true;

	// if the cache returns user but the bearer does not match - need to invalidate the cache
	GetBearerCache().Invalidate();
	return false;
}

static bool CheckAuthBearer ( const Str_t & sToken, HttpProcessResult_t & tRes, CSphVector<BYTE> & dReply, CSphString & sUser )
{
	CSphFixedVector<BYTE> dHexToken = ReadHexVec ( "token", sToken, AuthUserCred_t::m_iSourceTokenLen, false, tRes.m_sError );
	if ( !tRes.m_sError.IsEmpty() )
	{
		tRes.m_sError.SetSprintf ( "Access denied for token '%s', error: %s", sToken.first, tRes.m_sError.cstr() );
		AuthLog().AuthFailure ( sToken, AccessMethod_e::HTTP_BEARER, session::szClientName(), "malformed token" );
		return FailAuth ( tRes, dReply );
	}

	auto tHashTokenData = CalcBinarySHA2 ( dHexToken.Begin(), dHexToken.GetLength() );
	AuthUsersPtr_t pUsers = GetAuth();
	
	if ( CheckCacheMatched ( tHashTokenData, pUsers, sUser ) )
	{
		AuthLog().AuthSuccess ( sUser, AccessMethod_e::HTTP_BEARER, session::szClientName() );
		return true;
	}

	for ( const auto & tItem : pUsers->m_hUserToken )
	{
		const auto & tUser = tItem.second;
		if ( CheckBearerMatched ( tHashTokenData, tUser, sUser ) )
		{
			AuthLog().AuthSuccess ( sUser, AccessMethod_e::HTTP_BEARER, session::szClientName() );
			return true;
		}
	}

	tRes.m_sError.SetSprintf ( "Access denied for token '%s'", sToken.first );
	AuthLog().AuthFailure ( sToken, AccessMethod_e::HTTP_BEARER, session::szClientName(), "invalid token" );
	return FailAuth ( tRes, dReply );
}


bool CheckAuth ( const SmallStringHash_T<CSphString> & hOptions, HttpProcessResult_t & tRes, CSphVector<BYTE> & dReply, CSphString & sUser )
{
	if ( !IsAuthEnabled() )
	{
		sphLogDebug ( "no users found in config, access granted" );
		return true;
	}

	const CSphString * pSrcAuth = hOptions ( "authorization" );
	if ( !pSrcAuth )
	{
		tRes.m_eReplyHttpCode = EHTTP_STATUS::_401;
		tRes.m_bSkipBuddy = true;
		tRes.m_sError = "Authorization header field missed";
		sphHttpErrorReply ( dReply, tRes.m_eReplyHttpCode, tRes.m_sError.cstr(), R"(WWW-Authenticate: Basic realm="Manticore daemon", charset="UTF-8")" );
		AuthLog().AuthFailure ( "", AccessMethod_e::HTTP_BASIC, session::szClientName(), "authorization header is missing" );
		return false;
	}

	const char sAuthPrefixBasic[] = "Basic";
	const char sAuthPrefixBearer[] = "Bearer";
	const bool bBasicToken = ( pSrcAuth->Begins ( sAuthPrefixBasic ) );
	const bool bBearerToken = ( !bBasicToken && pSrcAuth->Begins ( sAuthPrefixBearer ) );
	if ( !bBasicToken && !bBearerToken )
	{
		tRes.m_sError = "Only Basic and Bearer authorization supported";
		AuthLog().AuthFailure ( "", AccessMethod_e::HTTP_BASIC, session::szClientName(), "unsupported authorization type" );
		return FailAuth ( tRes, dReply );
	}

	Str_t sAuthTail = GetTokenFromAuth ( *pSrcAuth, ( bBasicToken ? sizeof( sAuthPrefixBasic )-1 : sizeof( sAuthPrefixBearer )-1 ) );
	if ( bBasicToken )
		return CheckAuthBasic ( sAuthTail, tRes, dReply, sUser );
	else
		return CheckAuthBearer ( sAuthTail, tRes, dReply, sUser );
}

bool HttpCheckPerms ( const CSphString & sUser, AuthAction_e eAction, const CSphString & sTarget, EHTTP_STATUS & eReplyHttpCode, CSphString & sError, CSphVector<BYTE> & dReply )
{
	if ( !IsAuthEnabled() )
	{
		sphLogDebug ( "no users found in config, permission granted" );
		return true;
	}

	if ( CheckPerms ( sUser, eAction, sTarget, ( eAction==AuthAction_e::ADMIN ), sError ) )
		return true;

	eReplyHttpCode = EHTTP_STATUS::_403;
	sphHttpErrorReply ( dReply, eReplyHttpCode, sError.cstr() );
	return false;
}

CSphString GetBearer ( const CSphString & sUser )
{
	if ( sUser.IsEmpty() || !IsAuthEnabled() )
		return CSphString();

	AuthUsersPtr_t pUsers = GetAuth();
	const AuthUserCred_t * pUser = pUsers->m_hUserToken ( sUser );
	if ( !pUser )
		return CSphString();

	return BinToHex ( pUser->m_dBearerSha256 );
}