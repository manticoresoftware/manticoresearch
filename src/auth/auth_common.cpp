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

#include "searchdssl.h"
#include <filesystem>

#ifdef _WIN32
#include <aclapi.h>
#endif

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

static void FromString ( Str_t sSrc, CSphFixedVector<BYTE> & dBuf )
{
	dBuf.Reset ( sSrc.second/2 );

	// expects hash in form FIPS-180-1, that is:
	// 45f44fd2db02b08b4189abf21e90edd712c9616d
	// i.e. 40 symbols hex hash in small letters, space
	for ( int i=0; i<sSrc.second/2; ++i )
	{
		BYTE & uCode = dBuf[i];
		uCode = HexChar ( sSrc.first[i * 2] );
		uCode = BYTE ( ( uCode << 4 ) + HexChar ( sSrc.first[i * 2 + 1] ) );
	}
}

CSphFixedVector<BYTE> ReadHexVec ( const char * sName, Str_t sRaw, int iHashLen, bool bAllowEmpty, CSphString & sError )
{
	CSphFixedVector<BYTE> dRes ( 0 );

	if ( IsEmpty ( sRaw ) )
	{
		if ( !bAllowEmpty )
			sError.SetSprintf ( "empty '%s'", sName );
		return dRes;
	}

	if ( iHashLen>0 && sRaw.second/2!=iHashLen )
	{
		sError.SetSprintf ( "argument '%s' has wrong hash size %d, expected %d", sName, sRaw.second/2, iHashLen );
		return dRes;
	}

	FromString ( sRaw, dRes );
	return dRes;
}

CSphFixedVector<BYTE> ReadHexVec ( const char * sName, const bson::Bson_c & tRoot, int iHashLen, bool bAllowEmpty, CSphString & sError )
{
	CSphFixedVector<BYTE> dRes ( 0 );

	auto tChild = tRoot.ChildByName ( sName );
	if ( tChild==bson::nullnode )
	{
		sError.SetSprintf ( "missed node '%s'", sName );
		return dRes;
	}

	Str_t sRaw = bson::ToStr ( tChild );
	if ( !sRaw.second && bAllowEmpty )
		return dRes;

	if ( sRaw.second/2!=iHashLen )
	{
		sError.SetSprintf ( "node '%s' has wrong hash size %d, expected %d", sName, sRaw.second/2, iHashLen );
		return dRes;
	}

	FromString ( sRaw, dRes );
	return dRes;
}

static bool ReadUsers ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersMutablePtr_t & tAuth, CSphString & sError );
static bool ReadPerms ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersMutablePtr_t & tAuth, CSphString & sError );
AuthUsersMutablePtr_t ReadAuth ( char * sSrc, const CSphString & sSrcName, CSphString & sError );
static bool CheckFileIsPrivate ( const CSphString & sFile, CSphString & sError );

bool ReadAuthFile ( const CSphString & sFile, CSphFixedVector<char> & dRawJson, CSphString & sError )
{
	if ( !sphIsReadable ( sFile, &sError ) )
	{
		sError.SetSprintf ( "can not read the '%s': %s", sFile.cstr(), sError.cstr() );
		return false;
	}

	if ( !CheckFileIsPrivate ( sFile, sError ) )
		return false;

	CSphAutoreader tReader;
	if ( !tReader.Open ( sFile, sError ) )
	{
		sError.SetSprintf ( "can not read the '%s', error: %s", sFile.cstr(), sError.cstr() );
		return false;
	}

	int iSize = (int)tReader.GetFilesize();
	// empty file - valid state, not error
	// means authentication enabled but no users created
	if ( iSize==0 )
	{
		dRawJson.Reset ( 0 );
		return true;
	}

	dRawJson.Reset ( iSize + 2 );
	auto iRead = (int64_t)sphReadThrottled ( tReader.GetFD(), dRawJson.Begin(), iSize );
	if ( iRead!=iSize )
	{
		sError.SetSprintf ( "can not read the'%s', error: wrong size %d(%d)", sFile.cstr(), (int)iRead, iSize );
		return false;
	}
	if ( tReader.GetErrorFlag() )
	{
		sError.SetSprintf ( "can not read the '%s', error: %s", sFile.cstr(), tReader.GetErrorMessage().cstr() );
		return false;
	}

	dRawJson[iSize] = '\0'; // safe gap
	dRawJson[iSize+1] = '\0';
	return true;
}

AuthUsersMutablePtr_t ReadAuthFile ( const CSphString & sFile, CSphString & sError )
{
	CSphFixedVector<char> dRawJson ( 0 );
	if ( !ReadAuthFile ( sFile, dRawJson, sError ) )
		return nullptr;

	return ReadAuth ( dRawJson.Begin(), sFile, sError );
}

AuthUsersMutablePtr_t ReadAuth ( char * sSrc, const CSphString & sSrcName, CSphString & sError )
{
	assert ( sSrc );

	CSphVector<BYTE> tRawBson;
	if ( !sphJsonParse ( tRawBson, sSrc, false, false, false, sError ) )
	{
		sError.SetSprintf ( "can not read the '%s', error: %s", sSrcName.cstr(), sError.cstr() );
		return nullptr;
	}

	bson::Bson_c tBsonSrc ( tRawBson );

	AuthUsersMutablePtr_t tAuth { new AuthUsers_t() };
	// empty auth is ok
	if ( tBsonSrc.IsEmpty() )
		return tAuth;

	if ( !tBsonSrc.IsAssoc() )
	{
		sError.SetSprintf ( "can not read the '%s', error: wrong json", sSrcName.cstr() );
		return nullptr;
	}

	if ( !ReadUsers ( sSrcName, tBsonSrc, tAuth, sError ) )
		return nullptr;
	if ( !ReadPerms ( sSrcName, tBsonSrc, tAuth, sError ) )
		return nullptr;

	return tAuth;
}

void AddUser ( const AuthUserCred_t & tEntry, AuthUsersMutablePtr_t & tAuth )
{
	tAuth->m_hUserToken.Add ( tEntry, tEntry.m_sUser );
}

bool ReadUsers ( const CSphString & sFile, bson::Bson_c & tBson, AuthUsersMutablePtr_t & tAuth, CSphString & sError )
{
	sph::StringSet hSalts;

	bson::Bson_c ( tBson.ChildByName ( "users" ) ).ForEach ( [&] ( const bson::NodeHandle_t & tHandle )
	{
		bson::Bson_c tNode { tHandle };

		AuthUserCred_t tEntry;
		tEntry.m_sUser = bson::String ( tNode.ChildByName ( "username" ) );
		tEntry.m_dSalt = ReadHexVec ( "salt", tNode, HASH20_SIZE, false, sError );

		bson::Bson_c tHashes { tNode.ChildByName ( "hashes" ) };

		auto dSha1 = ReadHexVec ( "password_sha1_no_salt", tHashes, HASH20_SIZE, false, sError );
		if ( !sError.IsEmpty() )
			return;
		memcpy ( tEntry.m_tPwdSha1.data(), dSha1.Begin(), HASH20_SIZE );

		tEntry.m_dPwdSha256 = ReadHexVec ( "password_sha256", tHashes, HASH256_SIZE, false, sError );
		tEntry.m_dBearerSha256 = ReadHexVec ( "bearer_sha256", tHashes, HASH256_SIZE, true, sError );

		if ( !sError.IsEmpty() )
		{
			sError.SetSprintf ( "user '%s' error: %s", tEntry.m_sUser.cstr(), sError.cstr() );
			return;
		}

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

		if ( !tEntry.MakeApiKey ( sError ) )
			return;

		AddUser ( tEntry, tAuth );
	});
	if ( !sError.IsEmpty() )
		return false;

	// should NOT fail daemon start if file provided but no users read
	return Validate ( tAuth, sError );
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
		tPerm.SetTarget ( bson::String ( tNode.ChildByName ( "target" ) ) );
		tPerm.m_bAllow = bson::Bool ( tNode.ChildByName ( "allow" ) );
		if ( tNode.HasAnyOf ( { "budget" } ) )
			bson::Bson_c ( tNode.ChildByName ( "budget" ) ).BsonToJson ( tPerm.m_sBudget );

		if ( tNode.HasError() )
		{
			sError.SetSprintf ( "can not read perms from the '%s', error: %s", sFile.cstr(), tNode.Error() );
			return;
		}
		if ( tPerm.m_eAction==AuthAction_e::UNKNOWN )
		{
			sError.SetSprintf ( "can not read perms from the '%s', user: %s, uknown action", sFile.cstr(), sUser.cstr() );
			return;
		}
		if ( tPerm.m_sTarget.IsEmpty() )
		{
			sError.SetSprintf ( "can not read perms from the '%s', user: %s, empty target", sFile.cstr(), sUser.cstr() );
			return;
		}
		AuthUserCred_t * pUser = tAuth->m_hUserToken ( sUser );
		if ( !pUser )
		{
			sError.SetSprintf ( "can not read perms from the '%s', permission for unknown user: %s", sFile.cstr(), sUser.cstr() );
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

CSphString WriteJson ( const AuthUsers_t & tAuth )
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
			tWriter.NamedString ( "salt", BinToHex ( tUser.m_dSalt ) );
			{
				tWriter.StartBlock ( ",\n", "\"hashes\":{\n", "\n}" );
				tWriter.NamedString ( "password_sha1_no_salt", BinToHex ( tUser.m_tPwdSha1 ) );
				tWriter.NamedString ( "password_sha256", BinToHex ( tUser.m_dPwdSha256 ) );
				tWriter.NamedString ( "bearer_sha256", BinToHex ( tUser.m_dBearerSha256 ) );
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

	if ( !CreateAuthFile ( sNewFile, sError ) )
		return false;

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

static bool SaveEmptyAuth ( const CSphString & sFile, CSphString & sError )
{
	CSphWriter tWriter;
	if ( !tWriter.OpenFile ( sFile, sError ) )
		return false;
	char sBuf[] = "{}";
	tWriter.PutBytes ( sBuf, sizeof ( sBuf ) );
	tWriter.CloseFile();

	return !tWriter.IsError();
}

#ifdef _WIN32
static CSphFixedVector<BYTE> GetCurUserSid()
{
	auto dUserSid = CSphFixedVector<BYTE> ( 0 );

	HANDLE hToken = NULL;
	if ( !OpenProcessToken ( GetCurrentProcess(), TOKEN_QUERY, &hToken ) )
		return dUserSid;

	DWORD uTokenSize = 0;
	GetTokenInformation ( hToken, TokenUser, NULL, 0, &uTokenSize );
	if ( ::GetLastError()!=ERROR_INSUFFICIENT_BUFFER )
	{
		CloseHandle ( hToken );
		return dUserSid;
	}

	auto pTokenUser = CSphFixedVector<BYTE> ( uTokenSize );
	bool bGotToken = GetTokenInformation ( hToken, TokenUser, pTokenUser.Begin(), uTokenSize, &uTokenSize );

	CloseHandle ( hToken );
	if ( !bGotToken )
		return dUserSid;

	PTOKEN_USER pTU = (PTOKEN_USER)pTokenUser.Begin();
	DWORD uSidSize = GetLengthSid ( pTU->User.Sid );
	dUserSid.Reset ( uSidSize );
	if ( !CopySid ( uSidSize, (PSID)dUserSid.Begin(), pTU->User.Sid ) )
		return dUserSid;

	return dUserSid;
}

bool CheckFileIsPrivate ( const CSphString & sFile, CSphString & sError )
{
	PACL pDacl = NULL;
	PSECURITY_DESCRIPTOR pSecDesc = NULL;
	
	AT_SCOPE_EXIT ( [&pSecDesc] { if ( pSecDesc ) LocalFree ( pSecDesc ); } );

	DWORD uRes = GetNamedSecurityInfoA ( sFile.cstr(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pDacl, NULL, (PSECURITY_DESCRIPTOR*)&pSecDesc );
	if ( uRes!=ERROR_SUCCESS )
	{
		sError.SetSprintf ( "file '%s' failed to get permissions", sFile.cstr() );
		return false;
	}

	if ( pDacl==NULL )
	{
		// NULL DACL - unrestricted access to everyone - failure
		sError.SetSprintf ( "file '%s' has NULL DACL, granting unrestricted access", sFile.cstr() );
		return false;
	}
	
	// SID for well-known accounts considered safe
	auto dCurUserSid = GetCurUserSid();
	if ( dCurUserSid.IsEmpty() )
	{
		sError.SetSprintf ( "file '%s' could not get current user SID", sFile.cstr() );
		return false;
	}

	BYTE dSidSystem[SECURITY_MAX_SID_SIZE];
	DWORD uSidSystem = sizeof(dSidSystem);
	if ( !CreateWellKnownSid ( WinLocalSystemSid, NULL, dSidSystem, &uSidSystem ) )
	{
		sError.SetSprintf ( "file '%s' CreateWellKnownSid for system failed", sFile.cstr() );
		return false;
	}

	BYTE dSidAdmins[SECURITY_MAX_SID_SIZE];
	DWORD uSidAdmins = sizeof(dSidAdmins);
	if ( !CreateWellKnownSid ( WinBuiltinAdministratorsSid, NULL, dSidAdmins, &uSidAdmins ) )
	{
		sError.SetSprintf ( "file '%s' CreateWellKnownSid for admins failed", sFile.cstr() );
		return false;
	}

	// iterate over ACE in the DACL
	for ( DWORD i=0; i<pDacl->AceCount; i++ )
	{
		ACCESS_ALLOWED_ACE * pAce = NULL;

		// can not get ACE - skip
		if ( GetAce ( pDacl, i, (LPVOID*)&pAce )==0 )
			continue;

		// access allowed entries check, all deny entries are fine
		if ( pAce->Header.AceType!=ACCESS_ALLOWED_ACE_TYPE )
			continue;

		PSID pAceSid = (PSID)( &pAce->SidStart );

		// if ACE SID matches one of allowed SID
		// then SID is ok, continue to the next
		if ( EqualSid ( pAceSid, dCurUserSid.Begin() ) || EqualSid ( pAceSid, dSidSystem ) || EqualSid ( pAceSid, dSidAdmins ) )
			continue;
		
		// found an ACE for an unauthorized user/group
		// convert SID to name for good error
		sError.SetSprintf ( "file '%s' has permissions for an unauthorized principal", sFile.cstr() );
		char sName[256], sDomain[256];
		DWORD uNameSizs = sizeof ( sName );
		DWORD uDomainSize = sizeof ( sDomain );
		SID_NAME_USE tNameUse;
		if ( LookupAccountSidA ( NULL, pAceSid, sName, &uNameSizs, sDomain, &uDomainSize, &tNameUse ) )
			sError.SetSprintf ( "%s %s:%s", sError.cstr(), sDomain, sName );

		return false;
	}

	// all ACEs iteration found no problems - the file is private
	return true;
}

bool CreateAuthFile ( const CSphString & sFile, CSphString & sError )
{
	HANDLE hToken = NULL;
	if ( !OpenProcessToken ( GetCurrentProcess(), TOKEN_QUERY, &hToken ) )
	{
		sError.SetSprintf ( "failed to create file '%s', errno: %u", sFile.cstr(), ::GetLastError() );
		return false;
	}
	
	DWORD uTokenSize = 0;
	GetTokenInformation ( hToken, TokenUser, NULL, 0, &uTokenSize );
	if ( ::GetLastError()!=ERROR_INSUFFICIENT_BUFFER )
	{
		CloseHandle ( hToken );
		sError.SetSprintf ( "failed to create file '%s', errno: %u", sFile.cstr(), ::GetLastError() );
		return false;
	}

	CSphFixedVector<BYTE> dTokenUser = CSphFixedVector<BYTE> ( uTokenSize );
	bool bGotToken = GetTokenInformation ( hToken, TokenUser, dTokenUser.Begin(), uTokenSize, &uTokenSize );
	CloseHandle ( hToken );

	if ( !bGotToken )
	{
		sError.SetSprintf ( "failed to create file '%s', errno: %u", sFile.cstr(), ::GetLastError() );
		return false;
	}
	
	PSID pUserSid = ( ( PTOKEN_USER )dTokenUser.Begin() )->User.Sid;

	EXPLICIT_ACCESS_A tEA[3];
	ZeroMemory ( &tEA, sizeof ( tEA ) );

	// current user
	tEA[0].grfAccessPermissions = GENERIC_ALL;
	tEA[0].grfAccessMode = SET_ACCESS;
	tEA[0].grfInheritance = NO_INHERITANCE;
	tEA[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	tEA[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
	tEA[0].Trustee.ptstrName = (LPSTR)pUserSid;

	// system
	BYTE dSidSystem[SECURITY_MAX_SID_SIZE];
	DWORD uSidSystemSize = sizeof ( dSidSystem );
	if ( !CreateWellKnownSid ( WinLocalSystemSid, NULL, dSidSystem, &uSidSystemSize ) )
	{
		sError.SetSprintf ( "failed to create file '%s', errno: %u", sFile.cstr(), ::GetLastError() );
		return false;
	}
	tEA[1].grfAccessPermissions = GENERIC_ALL;
	tEA[1].grfAccessMode = SET_ACCESS;
	tEA[1].grfInheritance = NO_INHERITANCE;
	tEA[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	tEA[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	tEA[1].Trustee.ptstrName = (LPSTR)dSidSystem;

	// admins
	BYTE dSidAdmins[SECURITY_MAX_SID_SIZE];
	DWORD uSidAdminsSize = sizeof ( dSidAdmins );
	if ( !CreateWellKnownSid ( WinBuiltinAdministratorsSid, NULL, dSidAdmins, &uSidAdminsSize ) )
	{
		sError.SetSprintf ( "failed to create file '%s', errno: %u", sFile.cstr(), ::GetLastError() );
		return false;
	}
	tEA[2].grfAccessPermissions = GENERIC_ALL;
	tEA[2].grfAccessMode = SET_ACCESS;
	tEA[2].grfInheritance = NO_INHERITANCE;
	tEA[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	tEA[2].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	tEA[2].Trustee.ptstrName = (LPSTR)dSidAdmins;

	// ACL contains the new ACE
	PACL pAcl = NULL;
	if ( SetEntriesInAclA ( 3, tEA, NULL, &pAcl )!= ERROR_SUCCESS )
	{
		sError.SetSprintf ( "failed to create file '%s', errno: %u", sFile.cstr(), ::GetLastError() );
		return false;
	}

	AT_SCOPE_EXIT ( [&pAcl] { if ( pAcl ) LocalFree ( pAcl ); } );

	// init security desc
	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc ( LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH );
	if ( pSD==NULL )
	{
		sError.SetSprintf ( "failed to create file '%s', errno: %u", sFile.cstr(), ::GetLastError() );
		return false;
	}

	AT_SCOPE_EXIT ( [&pSD] { if ( pSD ) LocalFree ( pSD ); } );

	if ( !InitializeSecurityDescriptor ( pSD, SECURITY_DESCRIPTOR_REVISION ) )
	{
		sError.SetSprintf ( "failed to create file '%s', errno: %u", sFile.cstr(), ::GetLastError() );
		return false;
	}

	// фdd ACL to the security desc
	if ( !SetSecurityDescriptorDacl ( pSD, TRUE, pAcl, FALSE ) )
	{
		sError.SetSprintf ( "failed to create file '%s', errno: %u", sFile.cstr(), ::GetLastError() );
		return false;
	}

	// init security attr
	SECURITY_ATTRIBUTES tSA;
	tSA.nLength = sizeof ( SECURITY_ATTRIBUTES );
	tSA.lpSecurityDescriptor = pSD;
	tSA.bInheritHandle = FALSE;

	// create file with the specified security attr
	HANDLE hFile = CreateFileA ( sFile.cstr(), GENERIC_WRITE, 0, &tSA, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL );
	if ( hFile==INVALID_HANDLE_VALUE )
	{
		sError.SetSprintf ( "failed to create file '%s', errno: %u", sFile.cstr(), ::GetLastError() );
		return false;
	}

	CloseHandle ( hFile );

	return SaveEmptyAuth ( sFile, sError );
}

#else

bool CheckFileIsPrivate ( const CSphString & sFile, CSphString & sError )
{
	auto tPerms = std::filesystem::status ( sFile.cstr() ).permissions();
	if ( ( tPerms & ( std::filesystem::perms::group_all | std::filesystem::perms::others_all ) )!=std::filesystem::perms::none )
	{
		sError.SetSprintf ( "file '%s' has permissions to all", sFile.cstr() );
		return false;
	}

	return true;
}

bool CreateAuthFile ( const CSphString & sFile, CSphString & sError )
{
	int iFD = ::open ( sFile.cstr(), SPH_O_NEW, 0600 );
	if ( iFD==-1 )
	{
		sError.SetSprintf ( "failed to open %s: %s", sFile.cstr(), strerrorm(errno) );
		return false;
	}

	SafeClose ( iFD );
	return SaveEmptyAuth ( sFile, sError );;
}

#endif

void CopyVec ( const BYTE * pSrc, int iLen, CSphFixedVector<BYTE> & dDst )
{
	dDst.Reset ( iLen );
	if ( iLen )
		memcpy ( dDst.Begin(), pSrc, dDst.GetLengthBytes() );
}

AuthUserCred_t::AuthUserCred_t ( const AuthUserCred_t & tOther )
{
	Copy ( tOther );
}

AuthUserCred_t & AuthUserCred_t::operator= ( const AuthUserCred_t & tOther )
{
	if ( this!=&tOther )
		Copy ( tOther );
	return *this;
}

void AuthUserCred_t::Copy ( const AuthUserCred_t & tOther )
{
	m_sUser = tOther.m_sUser;
	m_tPwdSha1 = tOther.m_tPwdSha1;
	m_dPwdSha256.CopyFrom ( tOther.m_dPwdSha256 );
	m_dBearerSha256.CopyFrom ( tOther.m_dBearerSha256 );
	m_dSalt.CopyFrom ( tOther.m_dSalt );
	m_dApiKey.CopyFrom ( tOther.m_dApiKey );
}

bool AuthUserCred_t::MakeApiKey ( CSphString & sError )
{
	return MakeApiKdf ( m_dPwdSha256, m_dSalt, m_dApiKey, sError );
}

using Key_fn = std::function<CSphString ( const AuthUserCred_t & )>;

static bool Validate ( const AuthUsersMutablePtr_t & tAuth, Key_fn fnKey, const char * sFieldName, StringBuilder_c & sBuilder )
{
	SmallStringHash_T<const AuthUserCred_t *> hFields;
	StringBuilder_c sUsersDups ( ", " );

	for ( const auto & tIt : tAuth->m_hUserToken )
	{
		CSphString sKey = fnKey ( tIt.second );
		if ( sKey.IsEmpty() )
			continue;

		const AuthUserCred_t ** ppExisted = hFields ( sKey );
		if ( ppExisted )
			sUsersDups.Appendf ( "%s=%s", tIt.second.m_sUser.cstr(), ( *ppExisted )->m_sUser.cstr() );
		else
			hFields.Add ( &tIt.second, sKey );
	}

	if ( sUsersDups.IsEmpty() )
		return true;

	sBuilder.Appendf ( "duplicate '%s' found, shared by users: %s", sFieldName, sUsersDups.cstr() );
	return false;
}

bool Validate ( const AuthUsersMutablePtr_t & tAuth, CSphString & sError )
{
	StringBuilder_c sBuilder ( "; " );

	OpenHashTable_T<uint64_t, CSphString> hHash;
	bool bValid = true;

	Key_fn fnSalt = []( const AuthUserCred_t & tAuthUser ) { return BinToHex ( tAuthUser.m_dSalt ); };
	bValid &= Validate ( tAuth, fnSalt, "salt", sBuilder );

	Key_fn fnSha256 = []( const AuthUserCred_t & tAuthUser ) { return BinToHex ( tAuthUser.m_dPwdSha256 ); };
	bValid &= Validate ( tAuth, fnSha256, "password_sha256", sBuilder );

	Key_fn fnBearer = []( const AuthUserCred_t & tAuthUser ) { return BinToHex ( tAuthUser.m_dBearerSha256 ); };
	bValid &= Validate ( tAuth, fnBearer, "bearer_sha256", sBuilder );

	sError = sBuilder.cstr();
	return bValid;
}

static uint64_t FoldHash256 ( const HASH256_t & tKey )
{
	return sphFNV64 ( tKey.data(), tKey.size() );
}

class BearerCache_c : public BearerCache_i
{
public:
	BearerCache_c() = default;
	~BearerCache_c() override = default;

	void AddUser ( const HASH256_t & tHash, const CSphString & sUser ) override;
	void Invalidate () override;
	CSphString FindUser ( const HASH256_t & tHash ) const override;

private:
	mutable CSphMutex m_tLock;
	OpenHashTable_T<uint64_t, CSphString> m_hTokens;
};

static BearerCache_c g_tBearerCache;

BearerCache_i & GetBearerCache()
{
	return g_tBearerCache;
}

void BearerCache_c::AddUser ( const HASH256_t & tHash, const CSphString & sUser )
{
	CSphScopedLock tLock ( m_tLock );
	m_hTokens.Add ( FoldHash256 ( tHash ), sUser );
}

void BearerCache_c::Invalidate ()
{
	CSphScopedLock tLock ( m_tLock );
	m_hTokens.Clear();
}

CSphString BearerCache_c::FindUser ( const HASH256_t & tHash ) const
{
	if ( !m_hTokens.GetLength() )
		return {};

	CSphScopedLock tLock ( m_tLock );
	const CSphString * pUser = m_hTokens.Find ( FoldHash256 ( tHash ) );
	return CSphString ( pUser ? *pUser : "" );
}

CSphString AuthGetPath ( const CSphConfigSection & hSearchd )
{
	CSphString sFile = hSearchd.GetStr ( "auth" );
	if ( sFile.IsEmpty() || sFile=="0" )
		return {};

	if ( IsConfigless() )
	{
		if ( sFile!="1" )
			sphFatal ( "cat not set file name in RT mode, enable auth with the '1'" );
		sFile = GetDatadirPath ( "auth.json" );
	}

	return RealPath ( sFile );
}
