//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxstd.h"
#include <array>

/// SHA256 digests
static constexpr int HASH256_SIZE = 32;
using HASH256_t = std::array<BYTE, HASH256_SIZE>;

class SHA256_i
{
public:
	virtual ~SHA256_i() = default;
	virtual void Init() = 0;
	virtual void Update ( const BYTE * pData, int iLen ) = 0;
	virtual void Final ( HASH256_t & tDigest ) = 0;
	virtual HASH256_t FinalHash() = 0;
};

SHA256_i * CreateSHA256();
HASH256_t CalcBinarySHA2 ( const void * pData, int iLen );
CSphString BinToHex ( const HASH256_t & dHash );

