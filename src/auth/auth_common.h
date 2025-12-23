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

	CSphFixedVector<BYTE> m_dSalt { 0 };
	HASH20_t	m_tPwdSha1;
	CSphFixedVector<BYTE> m_dPwdSha256 { 0 };
	CSphFixedVector<BYTE> m_dBearerSha256 { 0 };

	CSphFixedVector<BYTE> m_dApiKey { m_iApiKeyLen };

	AuthUserCred_t() = default;
	AuthUserCred_t ( const AuthUserCred_t & tOther );
	AuthUserCred_t & operator= ( const AuthUserCred_t & tOther );
	void Copy ( const AuthUserCred_t & tOther );
	bool MakeApiKey ( CSphString & sError );

	static constexpr int m_iSourceTokenLen { 32 };
	static constexpr int m_iApiKeyLen { 32 };
};

class AuthUsers_t
{
public:
	SmallStringHash_T<AuthUserCred_t> m_hUserToken;

	SmallStringHash_T<UserPerms_t> m_hUserPerms;
	bool m_bEnabled = false;

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
bool ReadAuthFile ( const CSphString & sFile, CSphFixedVector<char> & dRawJson, CSphString & sError );

const CSphString & GetPrefixAuth();
const CSphString & GetIndexNameAuthUsers();
const CSphString & GetIndexNameAuthPerms();
const CSphString & GetAuthBuddyName();

void AddUser ( const AuthUserCred_t & tEntry, AuthUsersMutablePtr_t & tAuth );
CSphFixedVector<BYTE> ReadHexVec ( const char * sName, Str_t sRaw, int iHashLen, bool bAllowEmpty, CSphString & sError );
CSphFixedVector<BYTE> ReadHexVec (  const char * sName, const bson::Bson_c & tRoot, int iHashLen, bool bAllowEmpty, CSphString & sError );
void SortUserPerms ( UserPerms_t & dPerms );
CSphString WriteJson ( const AuthUsers_t & tAuth );
AuthUsersMutablePtr_t ReadAuth ( char * sSrc, const CSphString & sSrcName, CSphString & sError );
bool CreateAuthFile ( const CSphString & sFile, CSphString & sError );
void CopyVec ( const BYTE * pSrc, int ilen, CSphFixedVector<BYTE> & dDst );
bool Validate ( const AuthUsersMutablePtr_t & tAuth, CSphString & sError );
CSphString AuthGetPath ( const CSphConfigSection & hSearchd );
HASH256_t GetPwdHash256 ( const AuthUserCred_t & tEntry, const CSphString & sPwd );

class BearerCache_i
{
public:
	virtual ~BearerCache_i() = default;
	virtual void AddUser ( const HASH256_t & tHash, const CSphString & sUser ) = 0;
	virtual void Invalidate () = 0;
	virtual CSphString FindUser ( const HASH256_t & tHash ) const = 0;
};

BearerCache_i & GetBearerCache();
