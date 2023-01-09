//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "crc32.h"
#include "ints.h"

//////////////////////////////////////////////////////////////////////////

DWORD sphCRC32 ( const void* s )
{
	DWORD uCRC = ~( (DWORD)0 );
	for ( auto p = (const BYTE*)s; *p; ++p )
		CRC32_step ( uCRC, *p );
	return ~uCRC;
}


DWORD sphCRC32 ( const void* s, int iLen, DWORD uPrevCRC )
{
	auto uCRC = ~uPrevCRC;
	auto p = (const BYTE*)s;
	auto pMax = p + iLen;
	while ( p < pMax )
		CRC32_step ( uCRC, *p++ );
	return ~uCRC;
}


DWORD sphCRC32 ( const void * s, int iLen )
{
	return sphCRC32 ( s, iLen, 0 );
}

