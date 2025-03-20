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

struct MySQLAuth_t
{
	static constexpr BYTE SCRAMBLE_LENGTH = 20;
	CSphFixedVector<char> m_dScramble { SCRAMBLE_LENGTH + 1 };
};

// SphixnQL
MySQLAuth_t GetMySQLAuth();
bool CheckAuth ( const MySQLAuth_t & tAuth, const CSphString & sUser, const VecTraits_T<BYTE> & dClientHash, CSphString & sError );

struct SqlStmt_t;
bool SqlCheckPerms ( const CSphString & sUser, const CSphVector<SqlStmt_t> & dStmt, CSphString & sError );

void HandleMysqlShowPerms ( RowBuffer_i & tOut );
void HandleMysqlShowUsers ( RowBuffer_i & tOut );
void HandleMysqlShowToken ( const CSphString & sUser, RowBuffer_i & tOut );