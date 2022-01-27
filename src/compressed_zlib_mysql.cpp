//
// Copyright (c) 2021-2022, Manticore Software LTD (https://manticoresearch.com)
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
	inline static uLong Common_compressBound ( uLong uSize )
	{
		return (uLong)compressBound (uSize);
	}

	inline static int Common_compress ( BYTE* pDest, uLong* pDestLen, const BYTE* pSource, uLong uSourceLen )
	{
		return compress2 ( pDest, pDestLen, pSource, uSourceLen, LEVEL_COMPRESSION );
	}

	inline static bool Common_uncompress ( BYTE* pDest, uLong* pDestLen, const BYTE* pSource, uLong uSourceLen )
	{
		auto iZResult = uncompress ( pDest, pDestLen, pSource, uSourceLen );
		return iZResult == Z_OK;
	}
};

bool IsZlibCompressionAvailable()
{
	return true;
}

void MakeZlibMysqlCompressedLayer ( AsyncNetBufferPtr_c & pSource )
{
	pSource = new MysqlCompressedSocket_T<ZlibCompressor> ( pSource );
}
