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

#include "sphinxutils.h"

#include "searchdsql.h"
#include "client_session.h"
#include "auth_common.h"
#include "auth_proto_mysql.h"
#include "auth_log.h"
#include "auth.h"

static void Crypt ( BYTE * pBuf, const BYTE * pS1, const BYTE * pS2, int iLen )
{
	const BYTE * pEnd = pS1 + iLen;
	while ( pS1<pEnd )
	{
		*pBuf++= *pS1++ ^ *pS2++;
	}
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
	const HASH20_t & tSha1 = tEntry.m_tPwdSha1;
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

	if ( !IsAuthEnabled() )
	{
		sphLogDebug ( "no users found in config, user '%s' access granted", sUser.cstr() );
		return true;
	}

	AuthUsersPtr_t pUsers = GetAuth();
	const AuthUserCred_t * pUser = pUsers->m_hUserToken ( sUser );
	if ( !pUser || dClientHash.IsEmpty() )
	{
		sError.SetSprintf ( "Access denied for user '%s' (using password: %s)", sUser.cstr(), ( dClientHash.IsEmpty() ? "NO" : "YES" ) );
		if ( !pUser )
			AuthLog().AuthFailure ( sUser, AccessMethod_e::SQL, session::szClientName(), "user does not exist" );
		else
			AuthLog().AuthFailure ( sUser, AccessMethod_e::SQL, session::szClientName(), "no password provided" );
		return false;
	}

	if ( !CheckPwd ( tAuth, *pUser, dClientHash ) )
	{
		sError.SetSprintf ( "Access denied for user '%s' (using password: YES)", sUser.cstr() );
		AuthLog().AuthFailure ( sUser, AccessMethod_e::SQL, session::szClientName(), "invalid password" );
		return false;
	} else
	{
		AuthLog().AuthSuccess ( sUser, AccessMethod_e::SQL, session::szClientName() );
		return true;
	}
}

bool SqlCheckPerms ( const CSphString & sUser, const CSphVector<SqlStmt_t> & dStmt, CSphString & sError )
{
	if ( !IsAuthEnabled() )
	{
		sphLogDebug ( "no users found in config, permission granted" );
		return true;
	}

	if ( !dStmt.GetLength() )
	{
		sphLogDebug ( "empty statements, permission granted" );
		return true;
	}

	const SqlStmt_t & tStmt = dStmt[0];

	// handle SQL query
	switch ( tStmt.m_eStmt )
	{
	case STMT_PARSE_ERROR: return true;

	case STMT_SELECT:
	case STMT_CALL:
	case STMT_DESCRIBE:
	case STMT_SHOW_CREATE_TABLE:
	case STMT_SHOW_INDEX_STATUS:
	case STMT_SHOW_AGENT_STATUS:
	case STMT_SHOW_FEDERATED_INDEX_STATUS:
	case STMT_FACET:
	case STMT_SHOW_INDEX_SETTINGS:
	case STMT_EXPLAIN:
	case STMT_SHOW_TABLE_INDEXES:
		return CheckPerms ( sUser, AuthAction_e::READ, ( tStmt.m_sIndex.IsEmpty() ? tStmt.m_tQuery.m_sIndexes : tStmt.m_sIndex ), false, sError );

	// special read actions without index
	case STMT_SHOW_WARNINGS:
	case STMT_SHOW_META:
	case STMT_SHOW_TABLES:
	case STMT_SELECT_COLUMNS:
	case STMT_SHOW_PROFILE:
	case STMT_SHOW_PLAN:
	case STMT_SHOW_SCROLL:
		return CheckPerms ( sUser, AuthAction_e::READ, tStmt.m_sIndex, true, sError );

	case STMT_INSERT:
	case STMT_REPLACE:
	case STMT_DELETE:
	case STMT_UPDATE:
	case STMT_ATTACH_INDEX:
	case STMT_FLUSH_RTINDEX:
	case STMT_FLUSH_RAMCHUNK:
	case STMT_TRUNCATE_RTINDEX:
	case STMT_OPTIMIZE_INDEX:
	case STMT_FLUSH_INDEX:
	case STMT_ALTER_KLIST_TARGET:
	case STMT_FREEZE:
	case STMT_UNFREEZE:
		if ( tStmt.m_sIndex.Begins ( GetPrefixAuth().cstr() ) )
		{
			sError.SetSprintf ( "Permission denied for user '%s'", sUser.cstr() );
			return false;
		}
		return CheckPerms ( sUser, AuthAction_e::WRITE, tStmt.m_sIndex, false, sError );

	// special write actions without index
	case STMT_SET:
	case STMT_BEGIN:
	case STMT_COMMIT:
	case STMT_ROLLBACK:
	case STMT_SYSFILTERS:
	case STMT_KILL:
	case STMT_FLUSH_HOSTNAMES:
	case STMT_FLUSH_LOGS:
		return CheckPerms ( sUser, AuthAction_e::WRITE, tStmt.m_sIndex, true, sError );

	case STMT_CREATE_TABLE:
	case STMT_CREATE_TABLE_LIKE:
	case STMT_DROP_TABLE:
	case STMT_CREATE_FUNCTION:
	case STMT_DROP_FUNCTION:
	case STMT_ALTER_ADD:
	case STMT_ALTER_DROP:
	case STMT_ALTER_MODIFY:
	case STMT_CREATE_PLUGIN:
	case STMT_DROP_PLUGIN:
	case STMT_ALTER_RECONFIGURE:
	case STMT_RELOAD_PLUGINS:
	case STMT_RELOAD_INDEX:
	case STMT_RELOAD_INDEXES:
	case STMT_ALTER_INDEX_SETTINGS:
	case STMT_IMPORT_TABLE:
	case STMT_ALTER_REBUILD_SI:
		return CheckPerms ( sUser, AuthAction_e::SCHEMA, tStmt.m_sIndex, false, sError );

	case STMT_JOIN_CLUSTER:
	case STMT_CLUSTER_CREATE:
	case STMT_CLUSTER_DELETE:
	case STMT_CLUSTER_ALTER_ADD:
	case STMT_CLUSTER_ALTER_DROP:
	case STMT_CLUSTER_ALTER_UPDATE:
		return CheckPerms ( sUser, AuthAction_e::SCHEMA, tStmt.m_sCluster, false, sError );


	case STMT_SHOW_STATUS:
	case STMT_SHOW_VARIABLES:
	case STMT_SHOW_COLLATION:
	case STMT_SHOW_CHARACTER_SET:
	case STMT_SHOW_DATABASES:
	case STMT_SHOW_PLUGINS:
	case STMT_SHOW_THREADS:
	case STMT_SHOW_SETTINGS:
	case STMT_SHOW_LOCKS:
		return CheckPerms ( sUser, AuthAction_e::SCHEMA, tStmt.m_sIndex, true, sError );

	case STMT_RELOAD_AUTH:
	case STMT_DEBUG:
	case STMT_SHOW_USERS:
		 return CheckPerms ( sUser, AuthAction_e::ADMIN, tStmt.m_sIndex, true, sError );

	case STMT_SHOW_PERMISSIONS:
	{
		if ( !tStmt.m_dCallStrings.GetLength() )
			return true;

		// SHOW PERMISSIONS FOR '<user>' is an admin-only path.
		if ( !CheckPerms ( sUser, AuthAction_e::ADMIN, tStmt.m_sIndex, true, sError ) )
			return false;

		const CSphString & sTargetUser = tStmt.m_dCallStrings[0];
		AuthUsersPtr_t pUsers = GetAuth();
		if ( !pUsers->m_hUserToken ( sTargetUser ) )
		{
			sError.SetSprintf ( "user '%s' not found", sTargetUser.cstr() );
			return false;
		}

		return true;
	}

	case STMT_SHOW_TOKEN:
	case STMT_SET_PASSWORD:
	case STMT_TOKEN:
	{
		const CSphString sTargetUser = ( tStmt.m_dCallStrings.GetLength() ? tStmt.m_dCallStrings[0] : sUser );
		if ( sTargetUser==sUser )
			return true;
		return CheckPerms ( sUser, AuthAction_e::ADMIN, tStmt.m_sIndex, true, sError );
	}

	case STMT_GRANT:
	case STMT_REVOKE:
	case STMT_CREATE_USER:
	case STMT_DROP_USER:
		return CheckPerms ( sUser, AuthAction_e::ADMIN, tStmt.m_sIndex, true, sError );

	default:
		break;
	}

	// FIXME!!! skip buddy for that failed query
	sError.SetSprintf ( "Permission denied for user '%s'", sUser.cstr() );
	return false;
}

bool SqlSkipBuddy()
{
	auto pSession = session::Info().GetClientSession();
	if ( !pSession )
		return false;

	switch ( pSession->m_eLastStmt )
	{
	case STMT_SHOW_USERS:
	case STMT_SHOW_PERMISSIONS:
	case STMT_SHOW_TOKEN:
	case STMT_SET_PASSWORD:
	case STMT_TOKEN:
	case STMT_CREATE_USER:
	case STMT_DROP_USER:
	case STMT_GRANT:
	case STMT_REVOKE:
		return true;
	default:
		return false;
	}
}

static bool HasAdminPerm ( const CSphString & sUser )
{
	AuthUsersPtr_t pUsers = GetAuth();
	const UserPerms_t * pPerms = pUsers->m_hUserPerms ( sUser );
	if ( !pPerms || !pPerms->GetLength() )
		return false;

	for ( const auto & tPerm : *pPerms )
	{
		if ( tPerm.m_eAction==AuthAction_e::ADMIN )
			return tPerm.m_bAllow;
	}

	return false;
}

static bool DumpUserPerms ( const CSphString & sUser, const UserPerms_t & dPerms, RowBuffer_i & tOut )
{
	for ( const auto & tPerm : dPerms )
	{
		tOut.PutString ( sUser );
		tOut.PutString ( GetActionName ( tPerm.m_eAction ) );
		tOut.PutString ( tPerm.m_sTarget );
		tOut.PutString ( tPerm.m_bAllow ? "true" : "false" );
		tOut.PutString ( tPerm.m_sBudget );
		if ( !tOut.Commit () )
			return false;
	}

	return true;
}

void HandleMysqlShowPerms ( RowBuffer_i & tOut, const CSphString * pTargetUser )
{
	tOut.HeadBegin ();
	tOut.HeadColumn ( "Username" );
	tOut.HeadColumn ( "action" );
	tOut.HeadColumn ( "Target" );
	tOut.HeadColumn ( "Allow" );
	tOut.HeadColumn ( "Budget" );
	if ( !tOut.HeadEnd () )
		return;

	if ( IsAuthEnabled() )
	{
		const CSphString & sSessionUser = session::GetUser();
		AuthUsersPtr_t pUsers = GetAuth();

		if ( pTargetUser )
		{
			assert ( HasAdminPerm ( sSessionUser ) );
			const UserPerms_t * pPerms = pUsers->m_hUserPerms ( *pTargetUser );
			if ( pPerms && !DumpUserPerms ( *pTargetUser, *pPerms, tOut ) )
				return;

		} else if ( !HasAdminPerm ( sSessionUser ) )
		{
			const UserPerms_t * pPerms = pUsers->m_hUserPerms ( sSessionUser );
			if ( pPerms && !DumpUserPerms ( sSessionUser, *pPerms, tOut ) )
				return;

		} else
		{
			for ( const auto & tUser : pUsers->m_hUserPerms )
			{
				if ( !DumpUserPerms ( tUser.first, tUser.second, tOut ) )
					return;
			}
		}
	}

	tOut.Eof ( false );
}

void HandleMysqlShowUsers ( RowBuffer_i & tOut )
{
	tOut.HeadBegin ();
	tOut.HeadColumn ( "Username" );
	if ( !tOut.HeadEnd () )
		return;

	if ( IsAuthEnabled() )
	{
		AuthUsersPtr_t pUsers = GetAuth();
		for ( const auto & tUser : pUsers->m_hUserToken )
		{
			tOut.PutString ( tUser.first );
			if ( !tOut.Commit () )
				return;
		}
	}

	tOut.Eof ( false );
}

void HandleMysqlShowToken ( const CSphString & sUser, RowBuffer_i & tOut )
{
	tOut.HeadBegin ();
	tOut.HeadColumn ( "Username" );
	tOut.HeadColumn ( "Token" );
	if ( !tOut.HeadEnd () )
		return;

	if ( IsAuthEnabled() )
	{
		AuthUsersPtr_t pUsers = GetAuth();
		const AuthUserCred_t * pUser = pUsers->m_hUserToken ( sUser );

		if ( pUser )
		{
			tOut.PutString ( sUser );
			tOut.PutString ( BinToHex ( pUser->m_dBearerSha256 ) );

			if ( !tOut.Commit () )
				return;
		}
	}

	tOut.Eof ( false );

}
