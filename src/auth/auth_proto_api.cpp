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

#include "auth_common.h"
#include "auth_proto_api.h"
#include "gcm_nonce.h"
#include "auth_log.h"
#include "auth.h"

/////////////////////////////////////////////////////////////////////////////
/// API

static void SetAuth ( const AuthUserCred_t * pUser, AgentConn_t * pAgent )
{
	pAgent->m_tApiKey.m_sUser = pUser->m_sUser;
	pAgent->m_tApiKey.m_dEncKey.CopyFrom ( pUser->m_dApiKey );
}


void SetAuth ( const CSphString & sUser, AgentConn_t * pAgent )
{
	if ( sUser.IsEmpty() || !pAgent )
		return;

	if ( !IsAuthEnabled() )
		return;

	AuthUsersPtr_t pUsers = GetAuth();
	const AuthUserCred_t * pUser = pUsers->m_hUserToken ( sUser );
	if ( !pUser )
		return;

	SetAuth ( pUser, pAgent );
}

void SetAuth ( const CSphString & sUser, CSphVector<AgentConn_t *> & dRemotes )
{
	if ( sUser.IsEmpty() || dRemotes.IsEmpty() )
		return;

	if ( !IsAuthEnabled() )
		return;

	AuthUsersPtr_t pUsers = GetAuth();
	const AuthUserCred_t * pUser = pUsers->m_hUserToken ( sUser );
	if ( !pUser )
		return;

	for ( auto & tClient : dRemotes )
		SetAuth ( pUser, tClient );
}

void SetSessionAuth ( CSphVector<AgentConn_t *> & dRemotes )
{
	SetAuth ( session::GetUser(), dRemotes );
}

bool ApiCheckPerms ( const CSphString & sUser, AuthAction_e eAction, const CSphString & sTarget, ISphOutputBuffer & tOut )
{
	if ( !IsAuthEnabled() )
	{
		sphLogDebug ( "no users found in config, permission granted" );
		return true;
	}

	CSphString sError;
	if ( CheckPerms ( sUser, eAction, sTarget, false, sError ) )
		return true;

	SendErrorReply ( tOut, "%s", sError.cstr() );
	return false;
}

bool ApiCheckClusterPerms ( const CSphString & sUser, ISphOutputBuffer & tOut )
{
	if ( !IsAuthEnabled() )
	{
		sphLogDebug ( "no users found in config, permission granted" );
		return true;
	}

	CSphString sError;
	if ( CheckPerms ( sUser, AuthAction_e::REPLICATION, CSphString(), true, sError ) )
		return true;

	SendErrorReply ( tOut, "%s", sError.cstr() );
	return false;
}

struct Request_t
{
	static constexpr  int m_iSizeCmdVer = sizeof ( WORD ) * 2;
	static constexpr  int m_iSizeHeader = m_iSizeCmdVer + sizeof ( int );
};

bool ApiEncrypt ( const ApiKey_t & tApiKey, SmartOutputBuffer_t & dChunks, CSphString & sError )
{
	if ( !tApiKey.m_dEncKey.GetLength() )
		return true;

	CSphVector<ISphOutputBuffer *> dSrcBuf;
	dChunks.LeakTo ( dSrcBuf );
	CSphVector<ISphOutputBuffer *> dDstBuf ( dSrcBuf.GetLength() );
	dDstBuf.Resize ( 0 );

	AT_SCOPE_EXIT([&dSrcBuf, &dDstBuf]
	{
		for ( const auto * pBuf : dSrcBuf ) SafeDelete ( pBuf );
		for ( const auto * pBuf : dDstBuf ) SafeDelete ( pBuf );
	});

	for ( const auto * pSrc : dSrcBuf )
	{
		dDstBuf.Add ( new ISphOutputBuffer() );

		// do not encrypt buffer with only command without data
		if ( pSrc->GetSentCount()<=Request_t::m_iSizeHeader )
		{
			dDstBuf.Last()->m_dBuf = pSrc->m_dBuf;
			continue;
		}

		CSphVector<BYTE> & dDst = dDstBuf.Last()->m_dBuf;

		BYTE dNonce[GCM_NONCE_LEN];
		GetNonceGen().Generate ( GetUidShortServerId(), true, dNonce );

		if ( !EncryptGCM ( pSrc->m_dBuf, tApiKey.m_sUser, tApiKey.m_dEncKey, dNonce, Request_t::m_iSizeHeader, dDst, sError ) )
			return false;

		int iBlobLen = dDst.GetLength() - Request_t::m_iSizeHeader;
		// copy cmd + cmd_ver
		memcpy ( dDst.Begin(), pSrc->m_dBuf.Begin(), Request_t::m_iSizeCmdVer );
		// set new packet size
		dDstBuf.Last()->WriteInt ( Request_t::m_iSizeCmdVer, (int)iBlobLen );
	}

	dChunks.SwapData ( dDstBuf );
	// to prevent at scope exit cleanup
	dDstBuf.Reset();
	return true;
}

class GcmUserKey_c : public GcmUserKey_i
{
public:
	explicit GcmUserKey_c ( bool bMaster )
		: m_bMaster ( bMaster )
	{
	}

	virtual ~GcmUserKey_c() = default;
	std::optional<CSphFixedVector<BYTE>> Get ( const CSphString & sUser, CSphString & sError ) const override
	{
		assert ( IsAuthEnabled() );

		AuthUsersPtr_t pUsers = GetAuth();
		const AuthUserCred_t * pUser = pUsers->m_hUserToken ( sUser );
		if ( !pUser )
		{
			sError.SetSprintf ( "unknow user '%s'", sUser.cstr() );
			return std::nullopt;
		}

		CSphFixedVector<BYTE> dKey { pUser->m_dApiKey.GetLength() };
		dKey.CopyFrom ( pUser->m_dApiKey );
		return dKey;
	}

	bool ValidateNonceReplay ( const BYTE * pNonce, CSphString & sError ) const override
	{
		if ( !pNonce )
		{
			sError = "empty n once";
			return false;
		}
	
		return GetNonceValidator().Validate ( pNonce, m_bMaster, sError );
	}

	bool m_bMaster = false;

};

bool ApiDecrypt ( SearchdCommand_e eCmd, DWORD uVer, AsyncNetInputBuffer_c & tIn, int & iReplySize, CSphString & sUser, CSphString & sError )
{
	if ( iReplySize<=Request_t::m_iSizeHeader )
		return true;

	if ( !IsAuthEnabled() )
	{
		sphLogDebug ( "no users found in config, access granted" );
		return true;
	}

	const BYTE * pEncrypted = nullptr;
	if ( !tIn.GetBytesZerocopy ( &pEncrypted, iReplySize ) )
	{
		AuthLog().AuthFailure ( sUser, AccessMethod_e::API, session::szClientName(), "buffer read failed" );
		return false;
	}

	GcmUserKey_c tKey ( true );

	VecTraits_T<BYTE> dEncryptedData ( (BYTE *)pEncrypted, iReplySize );
	CSphVector<BYTE> dDecrypted ( iReplySize );
	dDecrypted.Resize ( 0 );

	if ( !DecryptGCM ( dEncryptedData, dDecrypted, sUser, tKey, sError ) )
	{
		AuthLog().AuthFailure ( sUser, AccessMethod_e::API, session::szClientName(), sError.cstr() );
		return false;
	}

	tIn.SetBuffer ( std::move ( dDecrypted ) );

	// rewind back to position it left at ApiServe
	// and check these matched
	auto eDecryptedCmd = (SearchdCommand_e)tIn.GetWord ();
	auto uDecryptedVer = tIn.GetWord ();
	iReplySize = tIn.GetInt ();

	if ( eDecryptedCmd!=eCmd || uDecryptedVer!=uVer )
	{
		sError.SetSprintf ( "invalid decryption (command=%d(%d), ver=%u(%u))", eCmd, eDecryptedCmd, uVer, uDecryptedVer );
		AuthLog().AuthFailure ( sUser, AccessMethod_e::API, session::szClientName(), "invalid decryption: command/version mismatch" );
		return false;
	}

	AuthLog().AuthSuccess ( sUser, AccessMethod_e::API, session::szClientName() );

	return true;
}

bool ApiEncryptReply ( const CSphString & sUser, GenericOutputBuffer_c & tOut, int iPacketOff, CSphString & sError )
{
	if ( tOut.m_dBuf.GetLength()<=( Request_t::m_iSizeHeader + iPacketOff ) )
		return true;

	if ( sUser.IsEmpty() )
		return true;

	InputBuffer_c tIn ( tOut.m_dBuf );
	// skip to the packet start
	if ( iPacketOff )
	{
		const BYTE * pTmp = nullptr;
		tIn.GetBytesZerocopy ( &pTmp, iPacketOff );
	}
	SearchdStatus_e eRes = (SearchdStatus_e)tIn.GetWord();

	// error and retry return without encryption
	if ( eRes!=SEARCHD_OK && eRes!=SEARCHD_WARNING )
		return true;

	tIn.GetWord(); // WORD uVer
	int64_t iLenOff = tIn.GetBufferPos();
	int iSrcDataLen = tIn.GetInt();
	VecTraits_T dSrc ( tIn.GetBufferPtr(), iSrcDataLen );
	int64_t iSrcHeaderLen = tIn.GetBufferPos();

	GcmUserKey_c tKey ( false );
	auto dEncKey = tKey.Get ( sUser, sError );
	if ( !dEncKey )
		return false;

	BYTE dNonce[GCM_NONCE_LEN];
	GetNonceGen().Generate ( GetUidShortServerId(), false, dNonce );
	
	CSphVector<BYTE> dDst;
	// copy cmd + cmd_ver
	BYTE * pHeader = dDst.AddN ( iSrcHeaderLen );
	memcpy ( pHeader, tOut.m_dBuf.Begin(), iSrcHeaderLen );

	if ( !EncryptGCM ( dSrc, sUser, *dEncKey, dNonce, iSrcHeaderLen, dDst, sError ) )
		return false;

	int iBlobLen = dDst.GetLength() - iSrcHeaderLen;
	// set new packet size
	// dst got stealed here
	ISphOutputBuffer tNewOut ( dDst );
	tNewOut.WriteInt ( iLenOff, iBlobLen );
	
	tOut.m_dBuf.SwapData ( tNewOut.m_dBuf );
	return true;
}

bool ApiDecryptReply ( const ApiKey_t & tApiKey, CSphFixedVector<BYTE> & dReplyBuf, CSphString & sError )
{
	if ( !tApiKey.m_dEncKey.GetLength() )
		return true;

	if ( dReplyBuf.GetLength()<=Request_t::m_iSizeHeader )
		return true;

	GcmUserKey_c tKey ( false );
	CSphString sUser;

	CSphVector<BYTE> dDecrypted ( dReplyBuf.GetLength() );
	dDecrypted.Resize ( 0 );

	if ( !DecryptGCM ( dReplyBuf, dDecrypted, sUser, tKey, sError ) )
		return false;

	dReplyBuf.Reset ( 0 );
	int64_t iLen = dDecrypted.GetLength64();
	BYTE * pData = dDecrypted.LeakData();
	dReplyBuf.Set ( pData, iLen );

	return true;
}
