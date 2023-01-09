//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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
#include "fileio.h"

/// SHA1 digests
static constexpr int HASH20_SIZE = 20;

class SHA1_c
{
	class Impl_c;
	std::unique_ptr<Impl_c> m_pImpl;

public:
	SHA1_c();
	~SHA1_c();
	void Init();
	void Update ( const BYTE* pData, int iLen );
	void Final ( BYTE digest[HASH20_SIZE] );
};

// string and 20-bytes hash
struct TaggedHash20_t
{
	CSphString m_sTagName;
	BYTE m_dHashValue[HASH20_SIZE] = { 0 };

	// by tag + hash
	explicit TaggedHash20_t ( const char* sTag = nullptr, const BYTE* pHashValue = nullptr );

	// convert to FIPS-180-1
	CSphString ToFIPS() const;

	// load from FIPS-180-1
	int FromFIPS ( const char* sFIPS );

	// compare with raw hash
	bool operator== ( const BYTE * pRef ) const;

	inline bool Empty() const { return *this==m_dZeroHash; }

	// helper zero hash
	static const BYTE m_dZeroHash[HASH20_SIZE];
};


// file writer with hashing on-the-fly.
class WriterWithHash_c final: public CSphWriter
{
public:
	WriterWithHash_c ();
	~WriterWithHash_c () final;

	void Flush () final;
	void CloseFile ();

	// get resulting BLOB
	const BYTE * GetHASHBlob () const;

private:
	SHA1_c * m_pHasher;
	bool m_bHashDone = false;
	BYTE m_dHashValue[HASH20_SIZE] = { 0 };
};

CSphString BinToHex ( const VecTraits_T<BYTE>& dHash );
CSphString BinToHex ( const BYTE* pHash, int iLen );
CSphString CalcSHA1 ( const void* pData, int iLen );
bool CalcSHA1 ( const CSphString& sFileName, CSphString& sRes, CSphString& sError );
