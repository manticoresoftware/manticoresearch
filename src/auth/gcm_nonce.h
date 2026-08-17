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

// AES-GCM nonce 96-bit \ 12-byte
struct Nonce_t
{
	// - 1 bit: direction (master>agent \ agent>master)
	// - 31 bits: agent ID
	// - 32 bits: boot ID (random per daemon start)
	// - 32 bits: counter (monotonic per agent \ per direction)

	DWORD m_uDirAgentId = 0;
	DWORD m_uBootId = 0;
	DWORD m_uCounter = 0;

	void SetDirection ( bool bIsMaster );
	void SetAgentId ( DWORD uAgentId );
	bool IsMaster() const;
	DWORD GetAgentId() const;
	void Pack ( BYTE * pNonce ) const;
	bool Unpack ( const BYTE * pNonce );
};


class NonceGenerator_i
{
public:
	NonceGenerator_i() = default;
	virtual ~NonceGenerator_i() {};
	virtual void Generate ( DWORD uServerId, bool bIsMaster, BYTE * pNonce ) = 0;
};


class NonceValidator_i
{
public:
	NonceValidator_i() = default;
	virtual ~NonceValidator_i() {};
	virtual bool Validate ( const BYTE * pNonce, bool bIsMaster, CSphString & sError ) = 0;
};

NonceGenerator_i &  GetNonceGen();
NonceValidator_i & GetNonceValidator();
