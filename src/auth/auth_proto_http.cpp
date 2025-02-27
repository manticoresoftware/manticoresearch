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
#include "auth_proto_http.h"
#include "auth.h"

/////////////////////////////////////////////////////////////////////////////
/// HTTP

static bool CheckPwd ( const AuthUserCred_t & tEntry, const CSphString & sPwd )
{
	HASH256_t tPwdHash = CalcBinarySHA2 ( sPwd.cstr(), sPwd.Length() );

	int iCmp = memcmp ( tEntry.m_sPwdSha256.cstr(), tPwdHash.data(), tPwdHash.size() );
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
		return FailAuth ( tRes, dReply );
	}

	if ( !CheckPwd ( *pUser, sPwd ) )
	{
		tRes.m_sError.SetSprintf ( "Access denied for user '%s' (using password: YES)", sUser.cstr() );
		return FailAuth ( tRes, dReply );
	} else
	{
		return true;
	}
}

static bool CheckAuthBearer ( const Str_t & sToken, HttpProcessResult_t & tRes, CSphVector<BYTE> & dReply, CSphString & sUser )
{
	AuthUsersPtr_t pUsers = GetAuth();
	const CSphString * pUser = pUsers->m_hHttpToken2User ( sToken );
	if ( !pUser )
	{
		tRes.m_sError.SetSprintf ( "Access denied for token '%s'", sToken.first );
		return FailAuth ( tRes, dReply );
	}

	sUser = *pUser;
	return true;
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
		return false;
	}

	const char sAuthPrefixBasic[] = "Basic";
	const char sAuthPrefixBearer[] = "Bearer";
	const bool bBasicToken = ( pSrcAuth->Begins ( sAuthPrefixBasic ) );
	const bool bBearerToken = ( !bBasicToken && pSrcAuth->Begins ( sAuthPrefixBearer ) );
	if ( !bBasicToken && !bBearerToken )
	{
		tRes.m_sError = "Only Basic and Bearer authorization supported";
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

	if ( CheckPerms ( sUser, eAction, sTarget, sError ) )
		return true;

	eReplyHttpCode = EHTTP_STATUS::_403;
	sphHttpErrorReply ( dReply, eReplyHttpCode, sError.cstr() );
	return false;
}