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

#include "digest_sha1.h"

#include "sphinxstd.h"
#include "fileio.h"
#include "fileutils.h"
#include "indexsettings.h"


#if WITH_SSL
#define USE_SHA1_FROM_OPENSSL
#include <openssl/evp.h>
#endif

//////////////////////////////////////////////////////////////////////////
// SHA1 digests
//////////////////////////////////////////////////////////////////////////

// SHA1 from https://github.com/shodanium/nanomysql/blob/master/nanomysql.cpp

// nanomysql, a tiny MySQL client
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

static constexpr int SHA1_SIZE = HASH20_SIZE;

class SHA1_c::Impl_c
{
public:
#ifdef USE_SHA1_FROM_OPENSSL
	void Init()
	{
		m_dCtx = EVP_MD_CTX_create();
		EVP_DigestInit_ex ( m_dCtx, EVP_sha1(), nullptr );
	}

	void Update ( const BYTE* data, int len )
	{
		EVP_DigestUpdate ( m_dCtx, data, len );
	}

	void Final ( BYTE digest[SHA1_SIZE] )
	{
		unsigned int uLen = SHA1_SIZE;
		EVP_DigestFinal_ex ( m_dCtx, digest, &uLen );
		EVP_MD_CTX_destroy ( m_dCtx );
	}

private:
	EVP_MD_CTX* m_dCtx;

#else
	void Init()
	{
		const DWORD dInit[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
		memcpy ( state, dInit, sizeof ( state ) );
		count[0] = count[1] = 0;
	}

	void Update ( const BYTE* data, int len )
	{
		int i, j = ( count[0] >> 3 ) & 63;
		count[0] += ( len << 3 );
		if ( count[0] < (DWORD)( len << 3 ) )
			count[1]++;
		count[1] += ( len >> 29 );
		if ( ( j + len ) > 63 )
		{
			i = 64 - j;
			memcpy ( &buffer[j], data, i );
			Transform ( buffer );
			for ( ; i + 63 < len; i += 64 )
				Transform ( data + i );
			j = 0;
		} else
			i = 0;
		memcpy ( &buffer[j], &data[i], len - i );
	}

	void Final ( BYTE digest[SHA1_SIZE] )
	{
		BYTE finalcount[8];
		for ( auto i = 0; i < 8; i++ )
			finalcount[i] = (BYTE)( ( count[( i >= 4 ) ? 0 : 1] >> ( ( 3 - ( i & 3 ) ) * 8 ) )
									& 255 ); // endian independent
		Update ( (const BYTE*)"\200", 1 );	 // add padding
		while ( ( count[0] & 504 ) != 448 )
			Update ( (const BYTE*)"\0", 1 );
		Update ( finalcount, 8 ); // should cause a SHA1_Transform()
		for ( auto i = 0; i < SHA1_SIZE; i++ )
			digest[i] = (BYTE)( ( state[i >> 2] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 255 );
	}

private:
	static constexpr int SHA1_BUF_SIZE = 64;

	DWORD state[5], count[2];
	BYTE buffer[SHA1_BUF_SIZE];

	void Transform ( const BYTE buf[SHA1_BUF_SIZE] )
	{
		DWORD a = state[0], b = state[1], c = state[2], d = state[3], e = state[4], block[16];
		memset ( block, 0, sizeof ( block ) ); // initial conversion to big-endian units
		for ( int i = 0; i < 64; i++ )
			block[i >> 2] += buf[i] << ( ( 3 - ( i & 3 ) ) * 8 );
		for ( int i = 0; i < 80; i++ ) // do hashing rounds
		{
#define LROT( value, bits ) ( ( ( value ) << ( bits ) ) | ( ( value ) >> ( 32 - ( bits ) ) ) )
			if ( i >= 16 )
				block[i & 15] = LROT (
						block[( i + 13 ) & 15] ^ block[( i + 8 ) & 15] ^ block[( i + 2 ) & 15] ^ block[i & 15], 1 );

			if ( i < 20 )
				e += ( ( b & ( c ^ d ) ) ^ d ) + 0x5A827999;
			else if ( i < 40 )
				e += ( b ^ c ^ d ) + 0x6ED9EBA1;
			else if ( i < 60 )
				e += ( ( ( b | c ) & d ) | ( b & c ) ) + 0x8F1BBCDC;
			else
				e += ( b ^ c ^ d ) + 0xCA62C1D6;

			e += block[i & 15] + LROT ( a, 5 );
			DWORD t = e;
			e = d;
			d = c;
			c = LROT ( b, 30 );
			b = a;
			a = t;
		}
		state[0] += a; // save state
		state[1] += b;
		state[2] += c;
		state[3] += d;
		state[4] += e;
	}
#endif
};


SHA1_c::SHA1_c()
	: m_pImpl { new Impl_c }
{}

SHA1_c::~SHA1_c() = default;

void SHA1_c::Init()
{
	m_pImpl->Init();
}

void SHA1_c::Update ( const BYTE* pData, int iLen )
{
	m_pImpl->Update ( pData, iLen );
}

void SHA1_c::Final ( BYTE digest[HASH20_SIZE] )
{
	m_pImpl->Final ( digest );
}

CSphString BinToHex ( const BYTE * pHash, int iLen )
{
	const char * sDigits = "0123456789abcdef";
	if ( !iLen )
		return "";

	CSphString sRes;
	int iStrLen = 2*iLen+2;
	sRes.Reserve ( iStrLen );
	char * sHash = const_cast<char *> (sRes.cstr ());

	for ( int i=0; i<iLen; i++ )
	{
		*sHash++ = sDigits[pHash[i] >> 4];
		*sHash++ = sDigits[pHash[i] & 0x0f];
	}
	*sHash = '\0';
	return sRes;
}

CSphString BinToHex ( const VecTraits_T<BYTE> & dHash )
{
	return BinToHex ( dHash.Begin(), dHash.GetLength() );
}

CSphString CalcSHA1 ( const void * pData, int iLen )
{
	CSphFixedVector<BYTE> dHashValue ( HASH20_SIZE );
	SHA1_c dHasher;
	dHasher.Init();
	dHasher.Update ( (const BYTE*)pData, iLen );
	dHasher.Final ( dHashValue.begin() );
	return BinToHex ( dHashValue );
}

bool CalcSHA1 ( const CSphString & sFileName, CSphString & sRes, CSphString & sError )
{
	CSphAutofile tFile ( sFileName, SPH_O_READ, sError, false );
	if ( tFile.GetFD()<0 )
		return false;

	CSphFixedVector<BYTE> dHashValue ( HASH20_SIZE );
	SHA1_c dHasher;
	dHasher.Init();

	const int64_t iFileSize = tFile.GetSize();
	const int iBufSize = (int)Min ( iFileSize, DEFAULT_READ_BUFFER );
	int64_t iOff = 0;
	CSphFixedVector<BYTE> dFileData ( iBufSize );
	while ( iOff<iFileSize )
	{
		const int iLen = (int)Min ( iBufSize, iFileSize - iOff );
		if ( !tFile.Read ( dFileData.Begin(), iLen, sError ) )
			return false;

		dHasher.Update ( dFileData.Begin(), iLen );
		iOff += iLen;
	}

	dHasher.Final ( dHashValue.Begin() );
	sRes = BinToHex ( dHashValue );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// WriterWithHash_c - CSphWriter which also calc SHA1 on-the-fly
//////////////////////////////////////////////////////////////////////////

WriterWithHash_c::WriterWithHash_c()
{
	m_pHasher = new SHA1_c;
	m_pHasher->Init();
}

WriterWithHash_c::~WriterWithHash_c ()
{
	SafeDelete ( m_pHasher );
}

void WriterWithHash_c::Flush()
{
	assert ( !m_bHashDone ); // can't do anything with already finished hash
	if ( m_iPoolUsed>0 )
	{
		m_pHasher->Update ( m_pBuffer.get(), m_iPoolUsed );
		CSphWriter::Flush();
	}
}

const BYTE * WriterWithHash_c::GetHASHBlob () const
{
	assert ( m_bHashDone );
	return m_dHashValue;
}

void WriterWithHash_c::CloseFile ()
{
	assert ( !m_bHashDone );
	CSphWriter::CloseFile ();
	m_pHasher->Final ( m_dHashValue );
	m_bHashDone = true;
}


//////////////////////////////////////////////////////////////////////////
// TaggedHash20_t - string tag (filename) with 20-bytes binary hash
//////////////////////////////////////////////////////////////////////////

const BYTE TaggedHash20_t::m_dZeroHash[HASH20_SIZE] = { 0 };

// by tag + hash
TaggedHash20_t::TaggedHash20_t ( const char* sTagName, const BYTE* pHashValue )
	: m_sTagName ( sTagName )
{
	if ( !pHashValue )
		pHashValue = m_dZeroHash;
	memcpy ( m_dHashValue, pHashValue, HASH20_SIZE );
}

// serialize to FIPS form
CSphString TaggedHash20_t::ToFIPS () const
{
	const char * sDigits = "0123456789abcdef";
	char sHash [41];
	StringBuilder_c sResult;

	if ( Empty() )
		return "";

	for ( auto i = 0; i<HASH20_SIZE; ++i )
	{
		sHash[i << 1] = sDigits[m_dHashValue[i] >> 4];
		sHash[1 + (i << 1)] = sDigits[m_dHashValue[i] & 0x0f];
	}
	sHash[40] = '\0';

	// FIPS-180-1 - checksum, space, "*" (indicator of binary mode), tag
	sResult.Appendf ("%s *%s\n", sHash, m_sTagName.cstr());
	return sResult.cstr();
}

inline BYTE hex_char ( unsigned char c )
{
	if ( c>=0x30 && c<=0x39 )
		return c - '0';
	else if ( c>=0x61 && c<=0x66 )
		return c - 'a' + 10;
	assert ( false && "broken hex num - expected digits and a..f letters in the num" );
	return 0;
}

// de-serialize from FIPS, returns len of parsed chunk of sFIPS or -1 on error
int TaggedHash20_t::FromFIPS ( const char * sFIPS )
{
	// expects hash in form FIPS-180-1, that is:
	// 45f44fd2db02b08b4189abf21e90edd712c9616d *rt_full.ram\n
	// i.e. 40 symbols hex hash in small letters, space, '*' and tag, finished by '\n'

	assert ( sFIPS[HASH20_SIZE * 2]==' ' && "broken FIPS - space expected after hash" );
	assert ( sFIPS[HASH20_SIZE * 2 + 1]=='*' && "broken FIPS - * expected after hash and space" );

	for ( auto i = 0; i<HASH20_SIZE; ++i )
	{
		BYTE & uCode = m_dHashValue[i];
		uCode = hex_char ( sFIPS[i * 2] );
		uCode = BYTE ( ( uCode << 4 ) + hex_char ( sFIPS[i * 2 + 1] ) );
	}

	sFIPS += 2 + HASH20_SIZE * 2;
	auto len = (int) strlen ( sFIPS );

	if ( sFIPS[len - 1]!='\n' )
		return -1;

	m_sTagName.SetBinary ( sFIPS, len - 1 );
	return len;
}

bool TaggedHash20_t::operator== ( const BYTE * pRef ) const
{
	assert ( pRef );
	return !memcmp ( m_dHashValue, pRef, HASH20_SIZE );
}
