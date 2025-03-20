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
#include "sphinxutils.h"
#include "fileutils.h"
#include "std/base64.h"
#include "std/fnv64.h"
#include "sphinxjson.h"
#include <filesystem>

#include "auth_perms.h"
#include "auth_common.h"

static void FromString ( Str_t sSrc, CSphVector<BYTE> & dBuf )
{
	// expects hash in form FIPS-180-1, that is:
	// 45f44fd2db02b08b4189abf21e90edd712c9616d
	// i.e. 40 symbols hex hash in small letters, space

	dBuf.Resize ( 0 );
	for ( int i=0; i<sSrc.second/2; ++i )
	{
		BYTE & uCode = dBuf.Add();
		uCode = HexChar ( sSrc.first[i * 2] );
		uCode = BYTE ( ( uCode << 4 ) + HexChar ( sSrc.first[i * 2 + 1] ) );
	}
}

static CSphString ReadHex ( const char * sName, int iHashLen, bson::Bson_c & tNode, CSphVector<BYTE> & dBuf, CSphString & sError )
{
	CSphString sRes;

	Str_t sRaw = bson::ToStr ( tNode.ChildByName ( sName ) );
	if ( sRaw.second!=iHashLen*2 )
	{
		sError.SetSprintf ( "wrong decoded hash size %d, expected %d", dBuf.GetLength(), iHashLen );
		return sRes;
	}

	dBuf.Resize ( 0 );
	FromString ( sRaw, dBuf );
	if ( dBuf.GetLength()!=iHashLen )
	{
		sError.SetSprintf ( "wrong decoded hash size %d, expected %d", dBuf.GetLength(), iHashLen );
		return sRes;
	}

	sRes.SetBinary ( (const char *)dBuf.Begin(), dBuf.GetLength() );
	return sRes;
}

static bool ReadUsers ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersPtr_t & tAuth, CSphString & sError );
static bool ReadPerms ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersPtr_t & tAuth, CSphString & sError );

AuthUsersPtr_t ReadAuthFile ( const CSphString & sFile, CSphString & sError )
{
	AuthUsersPtr_t tAuth { new AuthUsers_t() };

	// ok to skip auth init if the is no filename set
	if ( sFile.IsEmpty() )
		return tAuth;
	
	if ( !sphIsReadable ( sFile, &sError ) )
	{
		sError.SetSprintf ( "can not read users from the '%s': %s", sFile.cstr(), sError.cstr() );
		return nullptr;
	}

	// FIXME!!! fix file permission on windows
#if !_WIN32
	auto tPerms = std::filesystem::status ( sFile.cstr() ).permissions();
	if ( ( tPerms & ( std::filesystem::perms::group_all | std::filesystem::perms::others_all ) )!=std::filesystem::perms::none )
	{
		sError.SetSprintf ( "file '%s' has permissions to all", sFile.cstr() );
		return nullptr;
	}
#endif

	CSphAutoreader tReader;
	if ( !tReader.Open ( sFile, sError ) )
	{
		sError.SetSprintf ( "can not read users from the '%s', error: %s", sFile.cstr(), sError.cstr() );
		return nullptr;
	}

	int iSize = (int)tReader.GetFilesize();
	if ( !iSize )
		sError.SetSprintf ( "can not read users from the '%s', error: file empty", sFile.cstr() );

	CSphFixedVector<char> dRawJson ( iSize + 2 );
	auto iRead = (int64_t)sphReadThrottled ( tReader.GetFD(), dRawJson.begin(), iSize );
	if ( iRead!=iSize )
	{
		sError.SetSprintf ( "can not read users from the'%s', error: wrong size %d(%d)", sFile.cstr(), (int)iRead, iSize );
		return nullptr;
	}
	if ( tReader.GetErrorFlag() )
	{
		sError.SetSprintf ( "can not read users from the '%s', error: %s", sFile.cstr(), tReader.GetErrorMessage().cstr() );
		return nullptr;
	}

	dRawJson[iSize] = '\0'; // safe gap
	dRawJson[iSize+1] = '\0';

	CSphVector<BYTE> tRawBson;
	if ( !sphJsonParse ( tRawBson, dRawJson.begin(), false, false, false, sError ) )
	{
		sError.SetSprintf ( "can not read users from the '%s', error: %s", sFile.cstr(), sError.cstr() );
		return nullptr;
	}

	bson::Bson_c tBsonSrc ( tRawBson );
	if ( tBsonSrc.IsEmpty() || !tBsonSrc.IsAssoc() )
	{
		sError.SetSprintf ( "can not read users from the '%s', error: wrong json", sFile.cstr() );
		return nullptr;
	}

	if ( !ReadUsers ( sFile, tBsonSrc, tAuth, sError ) )
		return nullptr;
	if ( !ReadPerms ( sFile, tBsonSrc, tAuth, sError ) )
		return nullptr;

	return tAuth;
}

bool ReadUsers ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersPtr_t & tAuth, CSphString & sError )
{
	CSphVector<BYTE> dHashBuf { HASH256_SIZE };
	sph::StringSet hSalts;

	bson::Bson_c ( tBson.ChildByName ( "users" ) ).ForEach ( [&] ( const bson::NodeHandle_t & tHandle )
	{
		bson::Bson_c tNode { tHandle };

		AuthUserCred_t tEntry;
		tEntry.m_sUser = bson::String ( tNode.ChildByName ( "username" ) );
		tEntry.m_sSalt = ReadHex ( "salt", HASH20_SIZE, tNode, dHashBuf, sError );

		bson::Bson_c tHashes { tNode.ChildByName ( "hashes" ) };

		ReadHex ( "password_sha1_no_salt", HASH20_SIZE, tHashes, dHashBuf, sError );
		memcpy ( tEntry.m_tPwdSha1.data(), dHashBuf.Begin(), dHashBuf.GetLength() );
		tEntry.m_sPwdSha256 = ReadHex ( "password_sha256", HASH256_SIZE, tHashes, dHashBuf, sError );
		tEntry.m_sBearerSha256 = ReadHex ( "bearer_sha256", HASH256_SIZE, tHashes, dHashBuf, sError );
		tEntry.m_sRawBearerSha256 = bson::String ( tHashes.ChildByName ( "bearer_sha256" ) );

		if ( !sError.IsEmpty() )
			return;

		if ( tNode.HasError() )
		{
			sError.SetSprintf ( "can not read users from the '%s', error: %s", sFile.cstr(), tNode.Error() );
			return;
		}
		if ( tHashes.HasError() )
		{
			sError.SetSprintf ( "can not read users from the '%s', error: %s", sFile.cstr(), tHashes.Error() );
			return;
		}
		if ( tAuth->m_hUserToken ( tEntry.m_sUser ) )
		{
			sError.SetSprintf ( "can not read users from the '%s', error: multiple user %s entries", sFile.cstr(), tEntry.m_sUser.cstr() );
			return;
		}
		if ( hSalts [ tEntry.m_sSalt ] )
		{
			sError.SetSprintf ( "can not read users from the '%s', error: multiple salt entries '%s'", sFile.cstr(), tEntry.m_sSalt.cstr() );
			return;
		}
		if ( tAuth->m_hHttpToken2User ( tEntry.m_sRawBearerSha256 ) )
		{
			sError.SetSprintf ( "can not read users from the '%s', error: multiple bearer entries '%s'", sFile.cstr(), tEntry.m_sRawBearerSha256.cstr() );
			return;
		}

		tAuth->m_hUserToken.Add ( tEntry, tEntry.m_sUser );
		tAuth->m_hHttpToken2User.Add ( tEntry.m_sUser, tEntry.m_sRawBearerSha256 );
		tAuth->m_hApiToken2User.Add ( tEntry.m_sUser, tEntry.m_sBearerSha256 );
		hSalts.Add ( tEntry.m_sSalt );
	});
	if ( !sError.IsEmpty() )
		return false;

	// should fail daemon start if file provided but no users read
	if ( tAuth->m_hUserToken.IsEmpty() )
	{
		sError.SetSprintf ( "no users read from the file '%s'", sFile.cstr() );
		return false;
	}

	return true;
}

static AuthAction_e ReadAction ( Str_t sAction )
{
	if ( StrEq ( sAction, "read" ) )
		return AuthAction_e::READ;
	else if ( StrEq ( sAction, "write" ) )
		return AuthAction_e::WRITE;
	else if ( StrEq ( sAction, "schema" ) )
		return AuthAction_e::SCHEMA;
	else if ( StrEq ( sAction, "replication" ) )
		return AuthAction_e::REPLICATION;
	else if ( StrEq ( sAction, "admin" ) )
		return AuthAction_e::ADMIN;
	else
		return AuthAction_e::UNKNOWN;
}

const char * GetActionName (  AuthAction_e eAction )
{
	switch ( eAction )
	{
	case AuthAction_e::READ: return "read";
	case AuthAction_e::WRITE: return "write";
	case AuthAction_e::SCHEMA: return "schema";
	case AuthAction_e::REPLICATION: return "replication";
	case AuthAction_e::ADMIN: return "admin";
	default:
		return "unknown";
		break;
	}
}

struct CmpPerm_fn
{
	static inline bool IsLess ( const UserPerm_t & tA, const UserPerm_t & tB )
	{
		// 1nd - actions
		if ( tA.m_eAction!=tB.m_eAction )
			return ( tA.m_eAction<tB.m_eAction );

		// 2st - allow = false
		if ( tA.m_bAllow!=tB.m_bAllow )
			return !tA.m_bAllow;


		// 3d - targets non-wildcards prior to wildcards
		bool bWildTargetA = tA.m_bTargetWildcard;
		bool bWildTargetB = tB.m_bTargetWildcard;
		if ( bWildTargetA!=bWildTargetB )
			return ( !bWildTargetA );

		return false;
	}
};

bool ReadPerms ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersPtr_t & tAuth, CSphString & sError )
{
	bson::Bson_c ( tBson.ChildByName ( "permissions" ) ).ForEach ( [&] ( const bson::NodeHandle_t & tHandle )
	{
		bson::Bson_c tNode { tHandle };
		
		CSphString sUser = bson::String ( tNode.ChildByName ( "username" ) );

		UserPerm_t tPerm;
		tPerm.m_eAction = ReadAction ( bson::ToStr ( tNode.ChildByName ( "action" ) ) );
		tPerm.m_sTarget = bson::String ( tNode.ChildByName ( "target" ) );
		tPerm.m_bAllow = bson::Bool ( tNode.ChildByName ( "allow" ) );
		tPerm.m_bTargetWildcard = HasWildcard ( tPerm.m_sTarget.cstr() );

		if ( tNode.HasError() )
		{
			sError.SetSprintf ( "can not read users from the '%s', error: %s", sFile.cstr(), tNode.Error() );
			return;
		}
		if ( tPerm.m_eAction==AuthAction_e::UNKNOWN )
		{
			sError.SetSprintf ( "can not read users from the '%s', user: %s, uknown action", sFile.cstr(), sUser.cstr() );
			return;
		}
		AuthUserCred_t * pUser = tAuth->m_hUserToken ( sUser );
		if ( !pUser )
		{
			sError.SetSprintf ( "can not read users from the '%s', permission for unknown user: %s", sFile.cstr(), sUser.cstr() );
			return;
		}

		UserPerms_t & tPerms = tAuth->m_hUserPerms.AddUnique ( sUser );
		tPerms.Add ( tPerm );
	});

	if ( !sError.IsEmpty() )
		return false;

	for ( auto & dPerms : tAuth->m_hUserPerms )
		dPerms.second.Sort( CmpPerm_fn() );

	return true;
}
