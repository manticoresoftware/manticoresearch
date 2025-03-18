//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "compressed_http.h"

#if WITH_ZLIB
#include <zlib.h>
#endif

bool GzipDecompress ( const ByteBlob_t sIn, CSphVector<BYTE> & dRes, CSphString & sError )
{
#if WITH_ZLIB
	const int iWindowBits =  MAX_WBITS | 32;
	z_stream tInflate;
	tInflate.zalloc = Z_NULL;
	tInflate.zfree = Z_NULL;
	tInflate.opaque = Z_NULL;
	tInflate.avail_in = 0;
	tInflate.next_in = Z_NULL;

	if ( inflateInit2 ( &tInflate, iWindowBits )!=Z_OK )
	{
		sError = "gzip error: init failed";
		return false;
	}
	tInflate.next_in = (z_const Bytef *)sIn.first;
	tInflate.avail_in = sIn.second;
	tInflate.avail_out = 0;

	int iDecompressed = 0;
	int iBufSize = Max ( 4096, sIn.second * 2 );

	while ( tInflate.avail_out==0 )
	{
		dRes.Resize ( iDecompressed + iBufSize + 1 );

		tInflate.next_out = dRes.Begin() + iDecompressed;
		tInflate.avail_out = iBufSize;
		
		int iRes = inflate ( &tInflate, Z_FINISH );
		if ( iRes!=Z_STREAM_END && iRes!=Z_OK && iRes!=Z_BUF_ERROR )
		{
			sError.SetSprintf ( "gzip error: %s", tInflate.msg );
			inflateEnd ( &tInflate );
			return false;
		}

		iDecompressed += ( iBufSize - tInflate.avail_out );
	}

	inflateEnd ( &tInflate );
	dRes.Resize ( iDecompressed + 1);
	dRes[iDecompressed] = 0; // zero-terminate, as it may be processed by cjson
	return true;

#else
	sError = "gzip error: unpack is not supported, rebuild with zlib";
	return false;
#endif
}


bool HasGzip()
{
#if WITH_ZLIB
	return true;
#else
	return false;
#endif
}