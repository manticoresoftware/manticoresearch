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

#pragma once

#include "sphinxstd.h"
#include "sphinxutils.h"

void AuthConfigure ( const CSphConfigSection & hSearchd );

struct MySQLAuth_t
{
	static constexpr BYTE SCRAMBLE_LENGTH = 20;
	CSphFixedVector<char> m_dScramble { SCRAMBLE_LENGTH + 1 };
};

// SphixnQL
MySQLAuth_t GetMySQLAuth();
bool CheckAuth ( const MySQLAuth_t & tAuth, const CSphString & sUser, const VecTraits_T<BYTE> & dClientHash, CSphString & sError );

// HTTP
struct HttpProcessResult_t;

bool CheckAuth ( const SmallStringHash_T<CSphString> & hOptions, HttpProcessResult_t & tRes, CSphVector<BYTE> & dReply );
