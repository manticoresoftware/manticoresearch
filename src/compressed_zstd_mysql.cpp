//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "compressed_zstd_mysql.h"
#include "compressed_mysql_layer.h"
#include <zstd.h>

#if DL_ZSTD

static decltype ( &ZSTD_createCCtx ) sph_ZSTD_createCCtx = nullptr;
static decltype ( &ZSTD_createDCtx ) sph_ZSTD_createDCtx = nullptr;
static decltype ( &ZSTD_freeDCtx ) sph_ZSTD_freeDCtx = nullptr;
static decltype ( &ZSTD_freeCCtx ) sph_ZSTD_freeCCtx = nullptr;
static decltype ( &ZSTD_compressBound ) sph_ZSTD_compressBound = nullptr;
static decltype ( &ZSTD_compressCCtx ) sph_ZSTD_compressCCtx = nullptr;
static decltype ( &ZSTD_decompressDCtx ) sph_ZSTD_decompressDCtx = nullptr;
static decltype ( &ZSTD_isError ) sph_ZSTD_isError = nullptr;

static bool InitDynamicZstd()
{
	const char* sFuncs[] = { "ZSTD_createCCtx", "ZSTD_createDCtx", "ZSTD_freeDCtx", "ZSTD_freeCCtx", "ZSTD_compressBound", "ZSTD_compressCCtx", "ZSTD_decompressDCtx", "ZSTD_isError" };
	void** pFuncs[] = { (void**)&sph_ZSTD_createCCtx, (void**)&sph_ZSTD_createDCtx, (void**)&sph_ZSTD_freeDCtx, (void**)&sph_ZSTD_freeCCtx, (void**)&sph_ZSTD_compressBound, (void**)&sph_ZSTD_compressCCtx, (void**)&sph_ZSTD_decompressDCtx, (void**)&sph_ZSTD_isError };

	static CSphDynamicLibrary dLib ( ZSTD_LIB );
	return dLib.LoadSymbols ( sFuncs, pFuncs, sizeof ( pFuncs ) / sizeof ( void** ) );
}

#else

#define sph_ZSTD_createCCtx ZSTD_createCCtx
#define sph_ZSTD_createDCtx ZSTD_createDCtx
#define sph_ZSTD_freeDCtx ZSTD_freeDCtx
#define sph_ZSTD_freeCCtx ZSTD_freeCCtx
#define sph_ZSTD_compressBound ZSTD_compressBound
#define sph_ZSTD_compressCCtx ZSTD_compressCCtx
#define sph_ZSTD_decompressDCtx ZSTD_decompressDCtx
#define sph_ZSTD_isError ZSTD_isError
#define InitDynamicZstd() ( true )

#endif

class ZstdCompressor
{
	int m_iLevel =
#ifdef ZSTD_CLEVEL_DEFAULT
	ZSTD_CLEVEL_DEFAULT
#else
	3
#endif
	;
	ZSTD_CCtx *m_pCtxCompress = nullptr;
	ZSTD_DCtx *m_pCtxDecompress = nullptr;

protected:
	using csize_t = size_t;

	ZstdCompressor()
	{
		m_pCtxCompress = sph_ZSTD_createCCtx();
		m_pCtxDecompress = sph_ZSTD_createDCtx();
	}

	~ZstdCompressor()
	{
		sph_ZSTD_freeDCtx ( m_pCtxDecompress );
		sph_ZSTD_freeCCtx ( m_pCtxCompress );
	}

	inline csize_t Common_compressBound ( csize_t uSize )
	{
		return (size_t)sph_ZSTD_compressBound ( uSize );
	}

	inline int Common_compress ( BYTE* pDest, csize_t* pDestLen, const BYTE* pSource, csize_t uSourceLen ) const
	{
		auto uSize = sph_ZSTD_compressCCtx ( m_pCtxCompress, pDest, *pDestLen, pSource, uSourceLen, m_iLevel );
		*pDestLen = uSize;
		return 0;
	}

	inline bool Common_uncompress ( BYTE* pDest, csize_t* pDestLen, const BYTE* pSource, csize_t uSourceLen )
	{
		auto iZResult = sph_ZSTD_decompressDCtx ( m_pCtxDecompress, pDest, *pDestLen, pSource, uSourceLen );
		if ( sph_ZSTD_isError ( iZResult ) )
			return false;
		*pDestLen = iZResult;
		return true;
	}

public:
	inline void SetLevel ( int iLevel )
	{
		m_iLevel = iLevel;
	}
};

bool IsZstdCompressionAvailable()
{
	static bool bZstdLoaded = false;
	if ( !bZstdLoaded )
		bZstdLoaded = InitDynamicZstd();

	return bZstdLoaded;
}

void MakeZstdMysqlCompressedLayer ( std::unique_ptr<AsyncNetBuffer_c>& pSource, int iLevel )
{
	auto pCompressed = std::make_unique<MysqlCompressedSocket_T<ZstdCompressor>> ( std::move ( pSource ) );
	pCompressed->SetLevel ( iLevel );
	pSource = std::move ( pCompressed );
}
