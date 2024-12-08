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
#include "auth.h"

struct AuthUserCred_t
{
	CSphString				m_sUser;
	CSphString				m_sPwd;
};

static SmallStringHash_T<AuthUserCred_t> g_hUsers;

void AuthConfigure ( const CSphConfigSection & hSearchd )
{
	AuthUserCred_t tEntry;
	tEntry.m_sUser = hSearchd.GetStr ( "auth_user" );
	tEntry.m_sPwd = hSearchd.GetStr ( "auth_pass" );

	g_hUsers.Add ( tEntry, tEntry.m_sUser );
}

MySQLAuth_t GetMySQLAuth()
{
	MySQLAuth_t tAuth;

	//tAuth.m_dScramble.Fill ( 0 );

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


static void printHex ( const BYTE * pBuffer, int iLen )
{
	printf ( "0x" );
    for ( int i=0; i<iLen; i++ )
        printf ( "%02x", (BYTE)(pBuffer[i]) );
	printf ( "\n" );
}

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
	HASH20_t tSha1 = CalcBinarySHA1 ( tEntry.m_sPwd.cstr(), tEntry.m_sPwd.Length() );
	sphLogDebug ( "sha1:(%d)", (int)tSha1.size() );
	printHex ( tSha1.data(), tSha1.size() );
	sphLogDebug ( "sha1:%s", CalcSHA1 ( tEntry.m_sPwd.cstr(), tEntry.m_sPwd.Length() ).cstr() );

	HASH20_t tSha2 = CalcBinarySHA1 ( tSha1.data(), tSha1.size() );
	sphLogDebug ( "sha2:(%d)", (int)tSha2.size() );
	printHex ( tSha2.data(), tSha2.size() );

	SHA1_c tSaltSha2Calc;
	tSaltSha2Calc.Init();
	tSaltSha2Calc.Update ( (const BYTE *)tSalt.m_dScramble.Begin(), tSalt.m_dScramble.GetLength()-1 );
	tSaltSha2Calc.Update ( tSha2.data(), tSha2.size() );
	HASH20_t tSha3 = tSaltSha2Calc.FinalHash();

	sphLogDebug ( "sha3:(%d)", (int)tSha3.size() );
	printHex ( tSha3.data(), tSha3.size() );

	CSphFixedVector<BYTE> dRes ( tSha1.size() );
	Crypt ( dRes.Begin(), tSha3.data(), tSha1.data(), dRes.GetLength() );

	sphLogDebug ( "buf:(%d)", (int)dRes.GetLength() );
	printHex ( dRes.Begin(), dRes.GetLength() );

	int iCmp = memcmp ( dRes.Begin(), dClientHash.Begin(), Min ( dClientHash.GetLength(), dRes.GetLength() ) );

	sphLogDebug ( "pwd:(%d) matched(%d):", dClientHash.GetLength(), iCmp );
	printHex ( dClientHash.Begin(), dClientHash.GetLength() );

	return ( iCmp==0 );
}

bool CheckAuth ( const MySQLAuth_t & tAuth, const CSphString & sUser, const VecTraits_T<BYTE> & dClientHash, CSphString & sError )
{
	sphLogDebug ( "auth raw:(%d)0x", dClientHash.GetLength() );
	printHex ( dClientHash.Begin(), dClientHash.GetLength() );

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
		return true;
}
