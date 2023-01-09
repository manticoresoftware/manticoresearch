//
// Copyright (c) 2021-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "compressed_zlib_mysql.h"
#include "compressed_mysql_layer.h"
#include <zlib.h>

static const int LEVEL_COMPRESSION = Z_DEFAULT_COMPRESSION;

class ZlibCompressor
{
protected:
	using csize_t = uLong;

	inline static csize_t Common_compressBound ( csize_t uSize )
	{
		return (csize_t)compressBound (uSize);
	}

	inline static int Common_compress ( BYTE* pDest, csize_t* pDestLen, const BYTE* pSource, csize_t uSourceLen )
	{
		return compress2 ( pDest, pDestLen, pSource, uSourceLen, LEVEL_COMPRESSION );
	}

	inline static bool Common_uncompress ( BYTE* pDest, csize_t* pDestLen, const BYTE* pSource, csize_t uSourceLen )
	{
		auto iZResult = uncompress ( pDest, (csize_t*) pDestLen, pSource, uSourceLen );
		return iZResult == Z_OK;
	}
};

bool IsZlibCompressionAvailable()
{
	return true;
}

void MakeZlibMysqlCompressedLayer ( std::unique_ptr<AsyncNetBuffer_c> & pSource )
{
	pSource = std::make_unique<MysqlCompressedSocket_T<ZlibCompressor>> ( std::move ( pSource ) );
}
