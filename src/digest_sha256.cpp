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


#include "sphinxstd.h"
#include "fileio.h"
#include "fileutils.h"
#include "indexsettings.h"
#include "digest_sha1.h"

#include "digest_sha256.h"

#if WITH_SSL
#define USE_SHA1_FROM_OPENSSL
#include <openssl/evp.h>
#endif

//////////////////////////////////////////////////////////////////////////
// SHA256 digests
//////////////////////////////////////////////////////////////////////////

class SHA256_c : public SHA256_i
{
public:
	void Init() final
	{
		m_pCtx = EVP_MD_CTX_create();
		EVP_DigestInit_ex ( m_pCtx, EVP_sha256(), nullptr );
	}

	void Update ( const BYTE * data, int len ) final
	{
		EVP_DigestUpdate ( m_pCtx, data, len );
	}

	void Final ( HASH256_t & tDigest ) final
	{
		unsigned int uLen = HASH256_SIZE;
		EVP_DigestFinal_ex ( m_pCtx, tDigest.data(), &uLen );
		EVP_MD_CTX_destroy ( m_pCtx );
	}

	HASH256_t FinalHash() final
	{
		HASH256_t dDigest {};
		Final ( dDigest );
		return dDigest;
	}

private:
	EVP_MD_CTX * m_pCtx = nullptr;
};

SHA256_i * CreateSHA256()
{
	return new SHA256_c();
}

HASH256_t CalcBinarySHA2 ( const void * pData, int iLen )
{
	SHA256_c dHasher;
	dHasher.Init();
	dHasher.Update ( (const BYTE*)pData, iLen );
	return dHasher.FinalHash();
}

CSphString BinToHex ( const HASH256_t & dHash )
{
	return BinToHex ( dHash.data(), dHash.size() );
}