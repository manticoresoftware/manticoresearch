//
// Copyright (c) 2021, Manticore Software LTD (https://manticoresearch.com)
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

class ZstdCompressor
{
	int m_iLevel = ZSTD_CLEVEL_DEFAULT;
	ZSTD_CCtx *m_pCtxCompress = nullptr;
	ZSTD_DCtx *m_pCtxDecompress = nullptr;

protected:
	ZstdCompressor()
	{
		m_pCtxCompress = ZSTD_createCCtx();
		m_pCtxDecompress = ZSTD_createDCtx();
	}

	~ZstdCompressor()
	{
		ZSTD_freeDCtx ( m_pCtxDecompress );
		ZSTD_freeCCtx ( m_pCtxCompress );
	}

	inline size_t Common_compressBound ( size_t uSize )
	{
		return (size_t)ZSTD_compressBound ( uSize );
	}

	inline int Common_compress ( BYTE* pDest, size_t* pDestLen, const BYTE* pSource, size_t uSourceLen ) const
	{
		auto uSize = ZSTD_compressCCtx ( m_pCtxCompress, pDest, *pDestLen, pSource, uSourceLen, m_iLevel );
		*pDestLen = uSize;
		return 0;
	}

	inline bool Common_uncompress ( BYTE* pDest, size_t* pDestLen, const BYTE* pSource, size_t uSourceLen )
	{
		auto iZResult = ZSTD_decompressDCtx ( m_pCtxDecompress, pDest, *pDestLen, pSource, uSourceLen );
		if ( ZSTD_isError ( iZResult ) )
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
	return true;
}

void MakeZstdMysqlCompressedLayer ( AsyncNetBufferPtr_c& pSource, int iLevel )
{
	auto pCompressed = new MysqlCompressedSocket_T<ZstdCompressor> ( pSource );
	pCompressed->SetLevel ( iLevel );
	pSource = pCompressed;
}
