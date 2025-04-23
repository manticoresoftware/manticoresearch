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
#include "std/openhash.h"
#include "digest_sha1.h"
#include "digest_sha256.h"
#include "sphinxjson.h"

#include "auth_perms.h"

struct AuthUserCred_t
{
	CSphString	m_sUser;

	CSphString	m_sSalt;
	HASH20_t	m_tPwdSha1;
	CSphString	m_sPwdSha256;
	CSphString	m_sBearerSha256;

	CSphString	m_sRawBearerSha256;
};

class AuthUsers_t
{
public:
	SmallStringHash_T<AuthUserCred_t> m_hUserToken;
	SmallStringHash_T<CSphString> m_hApiToken2User;
	SmallStringHash_T<CSphString> m_hHttpToken2User;

	SmallStringHash_T<UserPerms_t> m_hUserPerms;

	AuthUsers_t() = default;
	~AuthUsers_t() = default;
	AuthUsers_t ( AuthUsers_t && ) noexcept = default;
	AuthUsers_t & operator= ( AuthUsers_t && ) noexcept = default;
	AuthUsers_t ( const AuthUsers_t & ) noexcept = default;
	AuthUsers_t & operator= ( const AuthUsers_t & ) noexcept = default;
};

using AuthUsersPtr_t = std::shared_ptr<const AuthUsers_t>;
using AuthUsersMutablePtr_t = std::unique_ptr<AuthUsers_t>;

const AuthUsersPtr_t GetAuth();
AuthUsersMutablePtr_t CopyAuth();

AuthUsersMutablePtr_t ReadAuthFile ( const CSphString & sFile, CSphString & sError );
bool SaveAuthFile ( const AuthUsers_t & tAuth, const CSphString & sFile, CSphString & sError );

const CSphString & GetPrefixAuth();
const CSphString & GetIndexNameAuthUsers();
const CSphString & GetIndexNameAuthPerms();
const CSphString & GetAuthBuddyName();

void AddUser ( const AuthUserCred_t & tEntry, AuthUsersMutablePtr_t & tAuth );
CSphString ReadHex ( Str_t sRaw, int iHashLen, CSphString & sError );
CSphString ReadHex ( const char * sName, int iHashLen, const bson::Bson_c & tNode, CSphString & sError );
void SortUserPerms ( UserPerms_t & dPerms );