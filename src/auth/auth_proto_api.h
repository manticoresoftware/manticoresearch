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
#include "auth_perms.h"

// API
struct ApiKey_t
{
	CSphString m_sUser;
	CSphFixedVector<BYTE> m_dEncKey  { 0 };
};

bool ApiCheckPerms ( const CSphString & sUser, AuthAction_e eAction, const CSphString & sTarget, ISphOutputBuffer & tOut );
// FIXME!!! add cluster name to all api replication command and check proper perms
bool ApiCheckClusterPerms ( const CSphString & sUser, ISphOutputBuffer & tOut ); // !COMMIT

bool ApiEncrypt ( const ApiKey_t & tApiKey, SmartOutputBuffer_t & dChunks, CSphString & sError );

class AsyncNetInputBuffer_c;
bool ApiDecrypt ( SearchdCommand_e eCmd, DWORD uVer, AsyncNetInputBuffer_c & tIn, int & iReplySize, CSphString & sUser, CSphString & sError );

bool ApiEncryptReply ( const CSphString & sUser, GenericOutputBuffer_c & tOut, int iPacketOff, CSphString & sError );
bool ApiDecryptReply ( const ApiKey_t & tApiKey, CSphFixedVector<BYTE> & dReplyBuf, CSphString & sError );
