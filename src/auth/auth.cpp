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

#include "auth.h"

struct AuthUserCred_t
{
	CSphString	m_sUser;
	HASH20_t	m_tPwdHash;
};

static SmallStringHash_T<AuthUserCred_t> g_hUsers;

/////////////////////////////////////////////////////////////////////////////
/// auth load

static void ReadAuthFile ( const CSphString & sFile )
{
	// ok to skip auth init if the is no filename set
	if ( sFile.IsEmpty() )
		return;
	
	// FIXME!!! add the check of the file permissions for only this OS user but not 777

	CSphString sError;
	if ( !sphIsReadable ( sFile, &sError ) )
		sphFatal ( "can not read users from the '%s': %s", sFile.cstr(), sError.cstr() );

	// FIXME!!! fix file permission on windows
#if !_WIN32
	auto tPerms = std::filesystem::status ( sFile.cstr() ).permissions();
	if ( std::filesystem::perms::none!=( tPerms & std::filesystem::perms::all ) )
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

		AuthUserCred_t tEntry;
		tEntry.m_sUser.SetBinary ( sUser );
		memcpy ( tEntry.m_tPwdHash.data(), dHashBuf.Begin(), HASH20_SIZE );
		g_hUsers.Add ( tEntry, tEntry.m_sUser );
	}

	// should fail daemon start if file provided but no users read
	if ( g_hUsers.IsEmpty() )
		sphFatal ( "no users read from the file '%s'", sFile.cstr() );
}

static void AddConfigEntry ( const CSphString & sUser, const CSphString & sPwd )
{
	if ( sUser.IsEmpty() )
		return;

	AuthUserCred_t tEntry;
	tEntry.m_sUser = sUser;
	tEntry.m_tPwdHash = CalcBinarySHA1 ( sPwd.cstr(), sPwd.Length() );

	g_hUsers.Add ( tEntry, tEntry.m_sUser );
}

void AuthConfigure ( const CSphConfigSection & hSearchd )
{
	CSphString sUser = hSearchd.GetStr ( "auth_user" );
	CSphString sPwd = hSearchd.GetStr ( "auth_pass" );
	AddConfigEntry ( sUser, sPwd );

	ReadAuthFile ( hSearchd.GetStr ( "auth_user_file" ) );
}

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

/////////////////////////////////////////////////////////////////////////////
/// SphixnQL

static void Crypt ( BYTE * pBuf, const BYTE * pS1, const BYTE * pS2, int iLen )
{
	const BYTE * pEnd = pS1 + iLen;
	while ( pS1<pEnd )
	{
		*pBuf++= *pS1++ ^ *pS2++;
	}
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

	if ( g_hUsers.IsEmpty() )
	{
		sphLogDebug ( "no users found in config, user '%s' access granted", sUser.cstr() );
		return true;
	}

	const AuthUserCred_t * pUser = g_hUsers ( sUser );
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

bool CheckAuth ( const SmallStringHash_T<CSphString> & hOptions, HttpProcessResult_t & tRes, CSphVector<BYTE> & dReply )
{
	if ( g_hUsers.IsEmpty() )
	{
		sphLogDebug ( "no users found in config, access granted" );
		return true;
	}

	// FIXME!!! add buddy authorization
	if ( IsBuddyQuery ( hOptions ) )
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

	const char sAuthPrefix[] = "Basic";
	if ( !pSrcAuth->Begins ( sAuthPrefix ) )
	{
		tRes.m_sError = "Only basic authorization supported";
		return FailAuth ( tRes, dReply );
	}

	int iSrcAuthLen = pSrcAuth->Length();
	const char * sCur = pSrcAuth->cstr() + sizeof ( sAuthPrefix ) - 1;
	const char * sEnd = pSrcAuth->cstr() + iSrcAuthLen;
	while ( sCur<sEnd && isspace ( *sCur ) )
		sCur++;
	Str_t sSrcUserPwd ( sCur, iSrcAuthLen - ( sCur - pSrcAuth->cstr() ) );

	CSphVector<BYTE> dSrcUserPwd;
	DecodeBinBase64 ( sSrcUserPwd, dSrcUserPwd );

	int iDel = dSrcUserPwd.GetFirst ( []( BYTE c ) { return c==':'; } );
	if ( iDel==-1 )
	{
		tRes.m_sError = "Failed to find user:password delimiter";
		return FailAuth ( tRes, dReply );
	}

	CSphString sUser;
	sUser.SetBinary ( (const char *)dSrcUserPwd.Begin(), iDel );
	CSphString sPwd;
	sPwd.SetBinary ( (const char *)dSrcUserPwd.Begin() + iDel + 1, dSrcUserPwd.GetLength()-iDel-1 );

	const AuthUserCred_t * pUser = g_hUsers ( sUser );
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