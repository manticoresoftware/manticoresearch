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
	JSON_INT32_VECTOR	= 6,
	JSON_INT64_VECTOR	= 7,
	JSON_DOUBLE_VECTOR	= 8,
	JSON_MIXED_VECTOR	= 9,
	JSON_OBJECT			= 10,
	JSON_TRUE			= 11,
	JSON_FALSE			= 12,
	JSON_NULL			= 13,
	JSON_ROOT			= 14,

	JSON_TOTAL
};


/// get stored value from SphinxBSON blob
inline int sphJsonLoadInt ( const BYTE ** pp )
{
	DWORD uRes = sphGetDword(*pp);
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
/// NOTE, bQuoteString==false is intended to format individual values only (and avoid quoting string values in that case)
const BYTE * sphJsonFieldFormat ( CSphVector<BYTE> & dOut, const BYTE * pData, ESphJsonType eType, bool bQuoteString=true );

/// compute key mask (for Bloom filtering) from the key name
DWORD sphJsonKeyMask ( const char * sKey, int iLen );

/// find first value in SphinxBSON blob, return associated type
ESphJsonType sphJsonFindFirst ( const BYTE ** ppData );

/// find value by key in SphinxBSON blob, return associated type
ESphJsonType sphJsonFindByKey ( ESphJsonType eType, const BYTE ** ppValue, const void * pKey, int iLen, DWORD uMask );

/// find value by index in SphinxBSON blob, return associated type
ESphJsonType sphJsonFindByIndex ( ESphJsonType eType, const BYTE ** ppValue, int iIndex );

/// split name to object and key parts, return false if not JSON name
bool sphJsonNameSplit ( const char * sName, CSphString * sColumn, CSphString * sKey );

/// compute node size, in bytes
/// returns -1 when data itself is required to compute the size, but pData is NULL
int sphJsonNodeSize ( ESphJsonType eType, const BYTE * pData );

/// skip the current node, and update the pointer
void sphJsonSkipNode ( ESphJsonType eType, const BYTE ** ppData );

/// return object length or array length, in elements
/// POD types return 1, empty objects return 0
int sphJsonFieldLength ( ESphJsonType eType, const BYTE * pData );

/// inplace JSON update, both for realtime and non-realtime indexes, returns true if update is possible
bool sphJsonInplaceUpdate ( ESphJsonType eValueType, int64_t iValue, ISphExpr * pExpr, BYTE * pStrings, const CSphRowitem * pRow, bool bUpdate );

#endif // _sphinxjson_

//
// $Id$
//
