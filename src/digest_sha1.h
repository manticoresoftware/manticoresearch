//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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
#include <array>

/// SHA1 digests
static constexpr int HASH20_SIZE = 20;
using HASH20_t = std::array<BYTE, HASH20_SIZE>;

class SHA1_c
{
	class Impl_c;
	std::unique_ptr<Impl_c> m_pImpl;

public:
	SHA1_c();
	~SHA1_c();
	void Init();
	void Update ( const BYTE* pData, int iLen );
	void Final ( HASH20_t& tDigest );
	HASH20_t FinalHash();
};

// string and 20-bytes hash
struct TaggedHash20_t
{
	CSphString m_sTagName;
	HASH20_t m_dHashValue { 0 };

	// by tag + hash
	explicit TaggedHash20_t ( const char* sTag = nullptr, const BYTE* pHashValue = nullptr );

	TaggedHash20_t ( const char* sTag, const HASH20_t& dHashValue );

	// convert to FIPS-180-1
	[[nodiscard]] CSphString ToFIPS() const;

	// load from FIPS-180-1
	int FromFIPS ( const char* sFIPS );

	// compare with raw hash
	bool operator== ( const BYTE * pRef ) const;

	[[nodiscard]] inline bool Empty() const noexcept { return *this==m_dZeroHash.data(); }

	// helper zero hash
	inline static const HASH20_t m_dZeroHash {};
};


// file writer with hashing on-the-fly.
class WriterWithHash_c final: public CSphWriter
{
public:
	WriterWithHash_c ();

	void Flush () final;
	void CloseFile ();

	// get resulting BLOB
	[[nodiscard]] HASH20_t GetHASHBlob () const noexcept { return m_dHashValue; }

private:
	std::unique_ptr<SHA1_c> m_pHasher;
	bool m_bHashDone = false;
	HASH20_t m_dHashValue{};
};

//CSphString BinToHex ( const VecTraits_T<BYTE>& dHash );
CSphString BinToHex ( const HASH20_t& dHash );
CSphString BinToHexx ( const BYTE* pHash, int iLen );
CSphString CalcSHA1 ( const void* pData, int iLen );
bool CalcSHA1 ( const CSphString& sFileName, CSphString& sRes, CSphString& sError );
