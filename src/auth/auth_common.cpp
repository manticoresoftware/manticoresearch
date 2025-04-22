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

static const CSphString g_sBuddyName ( "system.buddy" );
static CSphString g_sPrefixAuth ( "system.auth_" );
static CSphString g_sIndexNameAuthUsers ( "system.auth_users" );
static CSphString g_sIndexNameAuthPerms ( "system.auth_permissions" );

const CSphString & GetPrefixAuth()
{
	return g_sPrefixAuth;
}

const CSphString & GetIndexNameAuthUsers()
{
	return g_sIndexNameAuthUsers;
}

const CSphString & GetIndexNameAuthPerms()
{
	return g_sIndexNameAuthPerms;
}

const CSphString & GetAuthBuddyName()
{
	return g_sBuddyName;
}

static void FromString ( Str_t sSrc, CSphString & sRes )
{
	// expects hash in form FIPS-180-1, that is:
	// 45f44fd2db02b08b4189abf21e90edd712c9616d
	// i.e. 40 symbols hex hash in small letters, space

	ByteBlob_t dBuf ( sRes );
	for ( int i=0; i<sSrc.second/2; ++i )
	{
		BYTE & uCode = (BYTE &)dBuf.first[i];
		uCode = HexChar ( sSrc.first[i * 2] );
		uCode = BYTE ( ( uCode << 4 ) + HexChar ( sSrc.first[i * 2 + 1] ) );
	}
}

CSphString ReadHex ( Str_t sRaw, int iHashLen, CSphString & sError )
{
	CSphString sRes;

	if ( sRaw.second/2!=iHashLen )
	{
		sError.SetSprintf ( "wrong hash size %d, expected %d", sRaw.second/2, iHashLen );
		return sRes;
	}

	sRes.Reserve ( sRaw.second/2 );
	FromString ( sRaw, sRes );

	return sRes;
}

CSphString ReadHex ( const char * sName, int iHashLen, const bson::Bson_c & tNode, CSphString & sError )
{
	CSphString sRes;

	Str_t sRaw = bson::ToStr ( tNode.ChildByName ( sName ) );
	if ( sRaw.second/2!=iHashLen )
	{
		sError.SetSprintf ( "wrong hash size %d, expected %d", sRaw.second/2, iHashLen );
		return sRes;
	}

	sRes.Reserve ( sRaw.second/2 );
	FromString ( sRaw, sRes );
	return sRes;
}

static bool ReadUsers ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersMutablePtr_t & tAuth, CSphString & sError );
static bool ReadPerms ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersMutablePtr_t & tAuth, CSphString & sError );

AuthUsersMutablePtr_t ReadAuthFile ( const CSphString & sFile, CSphString & sError )
{
	AuthUsersMutablePtr_t tAuth { new AuthUsers_t() };

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

void AddUser ( const AuthUserCred_t & tEntry, AuthUsersMutablePtr_t & tAuth )
{
	tAuth->m_hUserToken.Add ( tEntry, tEntry.m_sUser );
	tAuth->m_hHttpToken2User.Add ( tEntry.m_sUser, tEntry.m_sRawBearerSha256 );
	tAuth->m_hApiToken2User.Add ( tEntry.m_sUser, tEntry.m_sBearerSha256 );
}

bool ReadUsers ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersMutablePtr_t & tAuth, CSphString & sError )
{
	sph::StringSet hSalts;

	bson::Bson_c ( tBson.ChildByName ( "users" ) ).ForEach ( [&] ( const bson::NodeHandle_t & tHandle )
	{
		bson::Bson_c tNode { tHandle };

		AuthUserCred_t tEntry;
		tEntry.m_sUser = bson::String ( tNode.ChildByName ( "username" ) );
		tEntry.m_sSalt = ReadHex ( "salt", HASH20_SIZE, tNode, sError );

		bson::Bson_c tHashes { tNode.ChildByName ( "hashes" ) };

		CSphString sSha1 = ReadHex ( "password_sha1_no_salt", HASH20_SIZE, tHashes, sError );
		memcpy ( tEntry.m_tPwdSha1.data(), sSha1.cstr(), HASH20_SIZE );
		tEntry.m_sPwdSha256 = ReadHex ( "password_sha256", HASH256_SIZE, tHashes, sError );
		tEntry.m_sBearerSha256 = ReadHex ( "bearer_sha256", HASH256_SIZE, tHashes, sError );
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
			sError.SetSprintf ( "can not read users from the '%s', error: multiple salt entries '%s'", sFile.cstr(), BinToHex ( (const BYTE *)tEntry.m_sSalt.cstr(), tEntry.m_sSalt.Length() ).cstr() );
			return;
		}
		if ( tAuth->m_hHttpToken2User ( tEntry.m_sRawBearerSha256 ) )
		{
			sError.SetSprintf ( "can not read users from the '%s', error: multiple bearer entries '%s'", sFile.cstr(), tEntry.m_sRawBearerSha256.cstr() );
			return;
		}

		AddUser ( tEntry, tAuth );
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

AuthAction_e ReadAction ( Str_t sAction )
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

void SortUserPerms ( UserPerms_t & dPerms )
{
	dPerms.Sort( CmpPerm_fn() );
}

bool ReadPerms ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersMutablePtr_t & tAuth, CSphString & sError )
{
	bson::Bson_c ( tBson.ChildByName ( "permissions" ) ).ForEach ( [&] ( const bson::NodeHandle_t & tHandle )
	{
		bson::Bson_c tNode { tHandle };
		
		CSphString sUser = bson::String ( tNode.ChildByName ( "username" ) );

		UserPerm_t tPerm;
		tPerm.m_eAction = ReadAction ( bson::ToStr ( tNode.ChildByName ( "action" ) ) );
		tPerm.m_sTarget = bson::String ( tNode.ChildByName ( "target" ) );
		tPerm.m_sTarget.Trim();
		tPerm.m_bAllow = bson::Bool ( tNode.ChildByName ( "allow" ) );
		tPerm.m_bTargetWildcard = HasWildcard ( tPerm.m_sTarget.cstr() );
		tPerm.m_bTargetWildcardAll = ( tPerm.m_sTarget=="*" );
		if ( tNode.HasAnyOf ( { "budget" } ) )
			bson::Bson_c ( tNode.ChildByName ( "budget" ) ).BsonToJson ( tPerm.m_sBudget );

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
		if ( tPerm.m_sTarget.IsEmpty() )
		{
			sError.SetSprintf ( "can not read users from the '%s', user: %s, empty target", sFile.cstr(), sUser.cstr() );
			return;
		}
		AuthUserCred_t * pUser = tAuth->m_hUserToken ( sUser );
		if ( !pUser )
		{
			sError.SetSprintf ( "can not read users from the '%s', permission for unknown user: %s", sFile.cstr(), sUser.cstr() );
			return;
		}

		UserPerms_t & dPerms = tAuth->m_hUserPerms.AddUnique ( sUser );
		dPerms.Add ( tPerm );
	});

	if ( !sError.IsEmpty() )
		return false;

	for ( auto & tPerms : tAuth->m_hUserPerms )
		SortUserPerms ( tPerms.second );

	return true;
}

static CSphString WriteJson ( const AuthUsers_t & tAuth )
{
	JsonEscapedBuilder tWriter;
	tWriter.StartBlock ( nullptr, "{", "\n}" );
	{
		tWriter.StartBlock ( ",\n", "\"users\":[\n", "\n]" );
		for ( const auto & tItUser : tAuth.m_hUserToken )
		{
			const AuthUserCred_t & tUser = tItUser.second;
			if ( tUser.m_sUser==GetAuthBuddyName() )
				continue;

			tWriter.StartBlock ( ",\n", "{\n", "\n}" );
			tWriter.NamedString ( "username", tUser.m_sUser );
			tWriter.NamedString ( "salt", BinToHex ( (const BYTE *)tUser.m_sSalt.scstr(), tUser.m_sSalt.Length() ) );
			{
				tWriter.StartBlock ( ",\n", "\"hashes\":{\n", "\n}" );
				tWriter.NamedString ( "password_sha1_no_salt", BinToHex ( tUser.m_tPwdSha1.data(), tUser.m_tPwdSha1.size() ) );
				tWriter.NamedString ( "password_sha256", BinToHex ( (const BYTE *)tUser.m_sPwdSha256.scstr(), tUser.m_sPwdSha256.Length() ) );
				tWriter.NamedString ( "bearer_sha256", BinToHex ( (const BYTE *)tUser.m_sBearerSha256.scstr(), tUser.m_sBearerSha256.Length() ) );
				tWriter.FinishBlock ( true );
			}
			tWriter.FinishBlock ( true );
		}
		tWriter.FinishBlock ( true );

		tWriter.StartBlock ( ",\n", ",\"permissions\":[\n", "\n]" );
		for ( const auto & dPerms : tAuth.m_hUserPerms )
		{
			if ( dPerms.first==GetAuthBuddyName() )
				continue;

			for ( const auto & tPerm : dPerms.second )
			{
				tWriter.StartBlock ( ",\n", "{\n", "\n}" );
				tWriter.NamedString ( "username", dPerms.first );
				tWriter.NamedString ( "action", GetActionName ( tPerm.m_eAction ) );
				tWriter.NamedString ( "target", tPerm.m_sTarget );
				tWriter.NamedVal ( "allow", tPerm.m_bAllow );
				if ( !tPerm.m_sBudget.IsEmpty() )
				{
					tWriter.Named ( "budget" );
					tWriter.AppendRawChunk ( FromStr ( tPerm.m_sBudget ) ); 
				}
				tWriter.FinishBlock ( true );
			}
		}
		tWriter.FinishBlock ( true );
	}
	tWriter.FinishBlock ( true );

	return CSphString ( tWriter );
}

bool SaveAuthFile ( const AuthUsers_t & tAuth, const CSphString & sFile, CSphString & sError )
{
	CSphString sAuth = WriteJson ( tAuth );

	CSphString sNewFile;
	sNewFile.SetSprintf ( "%s.new", sFile.cstr() );

	CSphWriter tWriter;
	if ( !tWriter.OpenFile ( sNewFile, sError ) )
		return false;

	tWriter.PutBytes ( sAuth.cstr(), sAuth.Length() );
	tWriter.CloseFile();
	if ( tWriter.IsError() )
		return false;

	if ( sph::rename ( sNewFile.cstr(), sFile.cstr() ) )
	{
		sError.SetSprintf ( "failed to rename (src=%s, dst=%s, errno=%d, error=%s)", sNewFile.cstr(), sFile.cstr(), errno, strerrorm(errno) );
		return false;
	}

	return true;
}