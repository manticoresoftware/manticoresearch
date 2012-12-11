//
// $Id$
//

//
// Copyright (c) 2011, Andrew Aksyonoff
// Copyright (c) 2011, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxjson_
#define _sphinxjson_

#include "sphinx.h"

/// supported JSON value types
enum ESphJsonType
{
	JSON_EOF			= 0,
	JSON_INT32			= 1,
	JSON_INT64			= 2,
	JSON_DOUBLE			= 3,
	JSON_STRING			= 4,
	JSON_STRING_VECTOR	= 5,
};


/// get stored value from SphinxBSON blob
inline int sphJsonLoadInt ( const BYTE ** pp )
{
	const BYTE * p = *pp;
	DWORD uRes = p[0] + ( p[1]<<8 ) + ( p[2]<<16 ) + ( p[3]<<24 );
	*pp += 4;
	return int(uRes);
}


/// get stored value from SphinxBSON blob
inline int64_t sphJsonLoadBigint ( const BYTE ** pp )
{
	uint64_t uRes = (DWORD)sphJsonLoadInt ( pp );
	uRes += ( uint64_t ( sphJsonLoadInt ( pp ) )<<32 );
	return uRes;
}


/// unpack value from SphinxBSON blob
/// 0..251 == value itself
/// 252 == 2 more bytes
/// 253 == 3 more bytes
/// 254 == 4 more bytes
/// 255 == reserved
inline DWORD sphJsonUnpackInt ( const BYTE ** pp )
{
	const BYTE * p = *pp;
	DWORD uRes = p[0];
	switch ( p[0] )
	{
		default:
			(*pp) += 1;
			break;
		case 252:
			uRes = p[1] + ( p[2]<<8 );
			(*pp) += 3;
			break;
		case 253:
			uRes = p[1] + ( p[2]<<8 ) + ( p[3]<<16 );
			(*pp) += 4;
			break;
		case 254:
			uRes = p[1] + ( p[2]<<8 ) + ( p[3]<<16 ) + ( p[4]<<24 );
			(*pp) += 5;
			break;
		case 255:
			assert ( 0 && "unexpected reserved length code 255" );
			(*pp) += 1;
			break;
	}
	return uRes;
}

/// parse JSON, convert it into SphinxBSON blob
bool sphJsonParse ( CSphVector<BYTE> & dData, char * sData, bool bAutoconv, bool bToLowercase, CSphString & sError );

/// convert SphinxBSON blob back to JSON document
void sphJsonFormat ( CSphVector<BYTE> & dOut, const BYTE * pData );

/// convert SphinxBSON blob back to JSON document
const BYTE * sphJsonFieldFormat ( CSphVector<BYTE> & dOut, const BYTE * pData, ESphJsonType eType );

/// compute key mask (for Bloom filtering) from the key name
DWORD sphJsonKeyMask ( const char * sKey );

/// find value by key in SphinxBSON blob, return associated type
ESphJsonType sphJsonFindKey ( const BYTE ** ppValue, const BYTE * pData, const JsonKey_t & tKey );

/// split name to object and key parts, return false if not JSON name
bool sphJsonNameSplit ( const char * sName, CSphString * sColumn, CSphString * sKey );

#endif // _sphinxjson_

//
// $Id$
//
