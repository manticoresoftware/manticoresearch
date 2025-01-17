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

#include "digest_sha1.h"
#include "std/base64.h"
#include "fileutils.h"
#include <filesystem>
#include "searchdaemon.h"
#include "searchdhttp.h"
#include "searchdbuddy.h"
#include "searchdha.h"

#include "auth.h"

struct AuthUserCred_t
{
	CSphString	m_sUser;
	HASH20_t	m_tPwdHash;
	HASH20_t	m_tApiToken;
};

struct ApiUserCred_t
{
	CSphString	m_sUser;
	HASH20_t	m_tApiToken;
};

struct AuthUsers_t
{
	SmallStringHash_T<AuthUserCred_t> m_hUserToken;
	OpenHashTable_T<int64_t, ApiUserCred_t> m_hApiToken2User;
	SmallStringHash_T<AuthUserCred_t> m_hHttpToken;
};
static AuthUsers_t g_tAuth;

static const CSphString g_sBuddyName ( "system.buddy" );
static CSphString g_sBuddyToken;
static bool g_bBuddyAuthCheck = val_from_env ( "MANTICORE_BUDDY_AUTH_CHECK", false ); // max start timeout 3 sec

VecTraits_T<BYTE> Hash20Vec ( HASH20_t & dData )
{
	return VecTraits_T<BYTE> ( dData.data(), dData.size() );
}

/////////////////////////////////////////////////////////////////////////////
/// auth load

static void Crypt ( BYTE * pBuf, const BYTE * pS1, const BYTE * pS2, int iLen )
{
	const BYTE * pEnd = pS1 + iLen;
	while ( pS1<pEnd )
	{
		*pBuf++= *pS1++ ^ *pS2++;
	}
}

static void AddUserEntry ( const Str_t & sUser, const VecTraits_T<BYTE> & dHashBuf )
{
	AuthUserCred_t tEntry;

	tEntry.m_sUser.SetBinary ( sUser );
	memcpy ( tEntry.m_tPwdHash.data(), dHashBuf.Begin(), HASH20_SIZE );

	SHA1_c tTokenHash;
	tTokenHash.Init();
	tTokenHash.Update ( (const BYTE *)tEntry.m_sUser.cstr(), tEntry.m_sUser.Length() );
	tTokenHash.Update ( tEntry.m_tPwdHash.data(), tEntry.m_tPwdHash.size() );
	tEntry.m_tApiToken = tTokenHash.FinalHash();

	ApiUserCred_t tApiEntry;
	tApiEntry.m_sUser = tEntry.m_sUser;
	tApiEntry.m_tApiToken = tEntry.m_tApiToken;

	g_tAuth.m_hUserToken.Add ( tEntry, tEntry.m_sUser );
	g_tAuth.m_hApiToken2User.Add ( sphFNV64 ( tApiEntry.m_tApiToken.data(), tApiEntry.m_tApiToken.size() ), tApiEntry );
}

static void ReadAuthFile ( const CSphString & sFile )
{
	// ok to skip auth init if the is no filename set
	if ( sFile.IsEmpty() )
		return;
	
	CSphString sError;
	if ( !sphIsReadable ( sFile, &sError ) )
		sphFatal ( "can not read users from the '%s': %s", sFile.cstr(), sError.cstr() );

	// FIXME!!! fix file permission on windows
#if !_WIN32
	auto tPerms = std::filesystem::status ( sFile.cstr() ).permissions();
	if ( ( tPerms & ( std::filesystem::perms::group_all | std::filesystem::perms::others_all ) )!=std::filesystem::perms::none )
		sphFatal ( "file '%s' has permissions to all", sFile.cstr() );
#endif

	CSphAutoreader tReader;
	if ( !tReader.Open ( sFile, sError ) )
		sphFatal ( "can not read users from the '%s': %s", sFile.cstr(), sError.cstr() );

	CSphFixedVector<char> dBuf { 2048 };
	CSphVector<BYTE> dHashBuf { HASH20_SIZE };
	auto iSize = tReader.GetFilesize();
	for ( int iLine=0; tReader.GetPos()<iSize; iLine++ )
	{
		auto iLineLen = tReader.GetLine ( dBuf.Begin(), (int)dBuf.GetLengthBytes() );
		const char * pDel = (const char *)memchr ( dBuf.Begin(), ':', iLineLen );
		if ( !pDel )
		{
			sphWarning ( "no delimiter found at %d line", iLine );
			continue;
		}

		Str_t sUser ( dBuf.Begin(), pDel - dBuf.Begin() );
		Str_t sPwdHash ( pDel + 1, iLineLen - ( pDel + 1 - dBuf.Begin() ) );

		if ( IsEmpty ( sUser ) )
		{
			sphWarning ( "empty user name at line %d", iLine );
			continue;
		}

		const int iSha1PrefixLen = 5;
		if ( strncmp ( sPwdHash.first, "{SHA}", iSha1PrefixLen)!=0 )
		{
			sphWarning ( "only SHA-1 password hash allowed at line %d, got %.*s", iLine, iSha1PrefixLen, sPwdHash.first );
			continue;
		}
		if ( sPwdHash.second<=iSha1PrefixLen )
		{
			sphWarning ( "empty password hash at line %d", iLine );
			continue;
		}
		sPwdHash.first += iSha1PrefixLen;
		sPwdHash.second -= iSha1PrefixLen;

		dHashBuf.Resize ( 0 );
		DecodeBinBase64 ( sPwdHash, dHashBuf );
		if ( dHashBuf.GetLength()!=HASH20_SIZE )
		{
			sphWarning ( "wrong decoded hash size %d, expected %d at line %d", dHashBuf.GetLength(), HASH20_SIZE, iLine );
			continue;
		}

		AddUserEntry ( sUser, dHashBuf );

		const AuthUserCred_t & tEntry = g_tAuth.m_hUserToken[sUser];
		sphLogDebug ( "user:%s, sha1:(%d)0x%s, sha1 encoded:%s, api_token:0x%s", tEntry.m_sUser.cstr(), (int)tEntry.m_tPwdHash.size(), BinToHex ( tEntry.m_tPwdHash ).cstr(), sPwdHash.first, BinToHex ( tEntry.m_tApiToken ).cstr() );
	}

	// should fail daemon start if file provided but no users read
	if ( g_tAuth.m_hUserToken.IsEmpty() )
		sphFatal ( "no users read from the file '%s'", sFile.cstr() );
}

static void AddConfigEntry ( const CSphString & sUser, const CSphString & sPwd )
{
	if ( sUser.IsEmpty() )
		return;

	HASH20_t tBuf = CalcBinarySHA1 ( sPwd.cstr(), sPwd.Length() );

	AddUserEntry ( FromStr ( sUser ), Hash20Vec ( tBuf ) );
}

static CSphString AddToken( const AuthUserCred_t & tAuth )
{
	SHA1_c tTokenHash;
	tTokenHash.Init();

	DWORD uScramble[4] = { sphRand(), sphRand(), sphRand(), sphRand() };
	tTokenHash.Update ( (const BYTE *)&uScramble, sizeof ( uScramble ) );
	int64_t iDT = sphMicroTimer();
	tTokenHash.Update ( (const BYTE *)&iDT, sizeof ( iDT ) );
	tTokenHash.Update ( (const BYTE *)tAuth.m_sUser.cstr(), tAuth.m_sUser.Length() );

	HASH20_t tToken = tTokenHash.FinalHash();

	CSphString sToken = EncodeBinBase64 ( Hash20Vec ( tToken ) );
	Verify ( g_tAuth.m_hHttpToken.Add ( tAuth, sToken ) );

	return sToken;
}

static void AddBuddyToken ()
{
	if ( !IsAuthEnabled() )
		return;

	SHA1_c tPwdHash;
	tPwdHash.Init();

	DWORD uScramble[4] = { sphRand(), sphRand(), sphRand(), sphRand() };
	tPwdHash.Update ( (const BYTE *)&uScramble, sizeof ( uScramble ) );
	int64_t iDT = sphMicroTimer();
	tPwdHash.Update ( (const BYTE *)&iDT, sizeof ( iDT ) );

	HASH20_t tPwd = tPwdHash.FinalHash();

	AddUserEntry ( FromStr ( g_sBuddyName ), Hash20Vec ( tPwd ) );
	g_sBuddyToken = AddToken ( g_tAuth.m_hUserToken[g_sBuddyName] );
}

void AuthConfigure ( const CSphConfigSection & hSearchd )
{
	CSphString sUser = hSearchd.GetStr ( "auth_user" );
	CSphString sPwd = hSearchd.GetStr ( "auth_pass" );
	AddConfigEntry ( sUser, sPwd );

	ReadAuthFile ( hSearchd.GetStr ( "auth_user_file" ) );
	AddBuddyToken();
}

bool IsAuthEnabled()
{
	return g_tAuth.m_hUserToken.GetLength();
}

const CSphString & GetBuddyToken()
{
	return g_sBuddyToken;
}

const CSphString CreateSessionToken()
{
	CSphString sToken;

	if ( !IsAuthEnabled() )
		return sToken;

	const AuthUserCred_t * pAuth = g_tAuth.m_hUserToken ( session::GetUser() );
	if ( !pAuth )
		return sToken;


	sToken = AddToken ( *pAuth );
	return sToken;
}

/////////////////////////////////////////////////////////////////////////////
/// SphixnQL

MySQLAuth_t GetMySQLAuth()
{
	MySQLAuth_t tAuth;

	// fill scramble auth data (random)
	DWORD i = 0;
	DWORD uRand = sphRand() | 0x01010101;

	for ( ; i<( tAuth.m_dScramble.GetLength() - sizeof ( DWORD ) ); i+=sizeof ( DWORD ) )
	{
		memcpy ( tAuth.m_dScramble.Begin() + i, &uRand, sizeof ( DWORD ) );
		uRand = sphRand() | 0x01010101;
	}
	if ( i<tAuth.m_dScramble.GetLength() )
		memcpy ( tAuth.m_dScramble.Begin() + i, &uRand, tAuth.m_dScramble.GetLength() - i );
	tAuth.m_dScramble.Last()  = 0;

	return tAuth;
}

static bool CheckPwd ( const MySQLAuth_t & tSalt, const AuthUserCred_t & tEntry, const VecTraits_T<BYTE> & dClientHash )
{
	const HASH20_t & tSha1 = tEntry.m_tPwdHash;
	sphLogDebug ( "sha1:(%d)0x%s", (int)tSha1.size(), BinToHex ( tSha1 ).cstr() );

	HASH20_t tSha2 = CalcBinarySHA1 ( tSha1.data(), tSha1.size() );
	sphLogDebug ( "sha2:(%d)0x%s", (int)tSha2.size(), BinToHex ( tSha2 ).cstr() );

	SHA1_c tSaltSha2Calc;
	tSaltSha2Calc.Init();
	tSaltSha2Calc.Update ( (const BYTE *)tSalt.m_dScramble.Begin(), tSalt.m_dScramble.GetLength()-1 );
	tSaltSha2Calc.Update ( tSha2.data(), tSha2.size() );
	HASH20_t tSha3 = tSaltSha2Calc.FinalHash();

	sphLogDebug ( "sha3:(%d)0x%s", (int)tSha3.size(), BinToHex ( tSha3 ).cstr() );

	CSphFixedVector<BYTE> dRes ( tSha1.size() );
	Crypt ( dRes.Begin(), tSha3.data(), tSha1.data(), dRes.GetLength() );

	sphLogDebug ( "buf:(%d)0x%s", (int)dRes.GetLength(), BinToHex ( dRes.Begin(), dRes.GetLength() ).cstr() );

	int iCmp = memcmp ( dRes.Begin(), dClientHash.Begin(), Min ( dClientHash.GetLength(), dRes.GetLength() ) );
	sphLogDebug ( "pwd:(%d) matched(%d):0x%s", dClientHash.GetLength(), iCmp, BinToHex ( dClientHash.Begin(), dClientHash.GetLength() ).cstr() );
	return ( iCmp==0 );
}

bool CheckAuth ( const MySQLAuth_t & tAuth, const CSphString & sUser, const VecTraits_T<BYTE> & dClientHash, CSphString & sError )
{
	sphLogDebug ( "auth raw:(%d)0x%s", dClientHash.GetLength(), BinToHex ( dClientHash.Begin(), dClientHash.GetLength() ).cstr() );

	if ( g_tAuth.m_hUserToken.IsEmpty() )
	{
		sphLogDebug ( "no users found in config, user '%s' access granted", sUser.cstr() );
		return true;
	}

	const AuthUserCred_t * pUser = g_tAuth.m_hUserToken ( sUser );
	if ( !pUser || dClientHash.IsEmpty() )
	{
		sError.SetSprintf ( "Access denied for user '%s' (using password: NO)", sUser.cstr() );
		return false;
	}

	if ( !CheckPwd ( tAuth, *pUser, dClientHash ) )
	{
		sError.SetSprintf ( "Access denied for user '%s' (using password: YES)", sUser.cstr() );
		return false;
	} else
	{
		return true;
	}
}

/////////////////////////////////////////////////////////////////////////////
/// HTTP

static bool CheckPwd ( const AuthUserCred_t & tEntry, const CSphString & sPwd )
{
	HASH20_t tPwdHash = CalcBinarySHA1 ( sPwd.cstr(), sPwd.Length() );

	int iCmp = memcmp ( tEntry.m_tPwdHash.data(), tPwdHash.data(), tPwdHash.size() );
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

	const AuthUserCred_t * pUser = g_tAuth.m_hUserToken ( sUser );
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
	const AuthUserCred_t * pUser = g_tAuth.m_hHttpToken ( sToken );
	if ( !pUser )
	{
		tRes.m_sError.SetSprintf ( "Access denied for token '%s'", sToken.first );
		return FailAuth ( tRes, dReply );
	}

	sUser = pUser->m_sUser;
	return true;
}


bool CheckAuth ( const SmallStringHash_T<CSphString> & hOptions, HttpProcessResult_t & tRes, CSphVector<BYTE> & dReply, CSphString & sUser )
{
	if ( g_tAuth.m_hUserToken.IsEmpty() )
	{
		sphLogDebug ( "no users found in config, access granted" );
		return true;
	}

	// FIXME!!! add buddy authorization
	if ( IsBuddyQuery ( hOptions ) && !g_bBuddyAuthCheck )
		return true;

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


/////////////////////////////////////////////////////////////////////////////
/// API

int GetApiTokenSize ( ApiAuth_e eType )
{
	switch ( eType )
	{
	case ApiAuth_e::SHA1: return HASH20_SIZE; break;
	case ApiAuth_e::SHA256: return HASH20_SIZE; break;

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

	const ApiUserCred_t * pUser = g_tAuth.m_hApiToken2User.Find ( sphFNV64 ( dToken.Begin(), dToken.GetLength() ) );
	if ( !pUser )
	{
		sError.SetSprintf ( "Access denied for token '%s'", BinToHex ( dToken.Begin(), dToken.GetLength() ).cstr() );
		return false;
	}
	if ( dToken.GetLength()!=pUser->m_tApiToken.size() || memcmp ( dToken.Begin(), pUser->m_tApiToken.data(), dToken.GetLength() )!=0 )
	{
		sError.SetSprintf ( "Wrong token '%s'", BinToHex ( dToken.Begin(), dToken.GetLength() ).cstr() );
		return false;
	}

	sphLogDebug ( "user %s passed", pUser->m_sUser.cstr() );

	sUser = pUser->m_sUser;
	return true;
}

ApiAuthToken_t GetApiAuth ( const CSphString & sUser )
{
	ApiAuthToken_t tToken;

	if ( !IsAuthEnabled() )
		return tToken;

	const AuthUserCred_t * pUser = g_tAuth.m_hUserToken ( sUser );
	if ( !pUser )
		return tToken;

	tToken.m_eType = ApiAuth_e::SHA1;
	tToken.m_dToken.Reset ( pUser->m_tApiToken.size() );
	memcpy ( tToken.m_dToken.Begin(), pUser->m_tApiToken.data(), tToken.m_dToken.GetLength() );

	return tToken;
}

void SetSessionAuth ( CSphVector<AgentConn_t *> & dRemotes )
{
	if ( !IsAuthEnabled() )
		return;

	ApiAuthToken_t tAuthToken = GetApiAuth ( session::GetUser() );
	for ( auto & tClient : dRemotes )
	{
		tClient->m_tAuthToken.m_eType = tAuthToken.m_eType;
		tClient->m_tAuthToken.m_dToken.CopyFrom ( tAuthToken.m_dToken );
	}
}
