//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2011-2016, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxjson.h"
#include "sphinxint.h"
#include "conversion.h"
#include "json/cJSON.h"

#if _WIN32
#include <io.h> // for isatty() in sphinxjson.c
#endif


//////////////////////////////////////////////////////////////////////////
// helpers

// cache result of system ::tolower into table for fast access
struct LowerTable
{
	char m_cLowerTable[256];
	LowerTable() { for ( int i = 0; i<256; ++i ) m_cLowerTable[i] = ( char ) tolower (i); }
};

inline char Mytolower ( char c )
{
	static LowerTable tbl;
//	return (char) tolower (c);
	return tbl.m_cLowerTable [ (int) c ];
}

// how match bytes need to store packed v
inline int PackedLen ( DWORD v )
{
	if ( v<0x000000FC ) // 251
		return 1;
	else if ( v<0x00010000 ) // 65536
		return 3;
	else if ( v<0x1000000 ) // 16777216
		return 4;
	else
		return 5;
}

// parse hex word (4 digits)
inline bool parse_hex ( const char * s, WORD* pRes )
{
	assert (pRes);
	*pRes=0;
	for ( int i=0; i<4; ++i)
	{
		switch (s[i])
		{
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			*pRes += s[i] - '0'; break;
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
			*pRes += s[i] - 'a' + 10; break;
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			*pRes += s[i] - 'A' + 10; break;
		default:
			return false;
		}
		if ( i<3 )
			*pRes <<= 4;
	}
	return true;
}

// shortcut return or parseUTF16 below
inline int parse_fail ( char *&pTarget )
{
	*pTarget++ = 'u';
	return 0;
}

// parse UTF-16 sequence (one or two codes)
// push result to target; return num of bytes to advance source
inline int parseUTF16 ( char* &pTarget, const char * s, const char* sMax )
{
	WORD uCode1;
	if ( s + 6<=sMax && parse_hex ( s+2, &uCode1 ) )
	{

		if ( ( uCode1>=0xDC00 ) && ( uCode1<0xE000 ) )
			return parse_fail ( pTarget );

		if ( ( uCode1>=0xD800 ) && ( uCode1<0xDC00 ) )
		{
			// it is surrogate pair; expect second code;
			s+=6;
			WORD uCode2;
			if ( s[0]=='\\' && s[1]=='u' && s + 6<=sMax && parse_hex ( s + 2, &uCode2 ) )
			{
				if ( ( uCode2<0xDC00 ) || ( uCode2>0xDFFF ) )
					return parse_fail ( pTarget );
				int iCodepoint = 0x10000 + ( ( ( uCode1 & 0x3FF ) << 10 ) | ( uCode2 & 0x3FF ) );
				pTarget += sphUTF8Encode ( ( BYTE * ) pTarget, iCodepoint );
				return 10;
			} else
				return parse_fail ( pTarget );
		}
		pTarget += sphUTF8Encode ( ( BYTE * ) pTarget, uCode1 );
		return 4;
	}
	return parse_fail ( pTarget );
}

// unescaped pEscaped, modify pEscaped to unescaped chain, return the length of unescaped
int JsonUnescape ( char * pTarget, const char * pEscaped, int iLen )
{
	assert ( pEscaped );
	const char * s = pEscaped;

	// skip heading and trailing quotes
	if ( ( s[0]=='\'' && s[iLen - 1]=='\'' ) || ( s[0]=='\"' && s[iLen - 1]=='\"' ) )
	{
		++s;
		iLen -= 2;
	}

	auto * sMax = s + iLen;
	auto * d = pTarget;
	const char * sEscape = nullptr;

	while ( s<sMax )
	{
		if ( s[0]=='\\' )
		{
			++s;
			if ( s<sMax )
			{
				switch ( s[0] )
				{
				case 'b': *d++ = '\b';
					break;
				case 'n': *d++ = '\n';
					break;
				case 'r': *d++ = '\r';
					break;
				case 't': *d++ = '\t';
					break;
				case 'f': *d++ = '\f';
					break; // formfeed (rfc 4627)
				case 'u': s += parseUTF16 ( d, s-1, sMax ); break;
				default: *d++ = *s;
				}
				++s;
			}
			continue;
		}
		assert ( s<sMax );
		// this snippet replaces a line '*d++=*s++'. On typical string (10% escapes) it provides total time about
		// 50% better then byte-by-byte copying.
		sEscape = (const char *) memchr ( s, '\\', sMax - s );
		if ( !sEscape )
			sEscape = sMax;
		auto iChunk = sEscape - s;
		memmove ( d, s, iChunk ); // consider also if ((d+iChunk)<s) memcpy (d, s, iChunk); else memmove...
		d += iChunk;
		s += iChunk;
	}
	return int ( d - pTarget );
}

//////////////////////////////////////////////////////////////////////////
// just a pair of numbers (std::pair is not suitable since assumes non-trivial ctrs)
struct BlobLocator_t
{
	int		m_iStart;
	int 	m_iLen;
};

/// parser view on a generic node
struct JsonNode_t
{
	union {
		int64_t			m_iValue;	///< integer value (only JSON_INT32 and JSON_INT64 )
		double			m_fValue;	///< floating point (only JSON_DOUBLE )
		BlobLocator_t	m_sValue;	///< string locator (value of JSON_STRING)
		BlobLocator_t	m_dChildren; ///< children locator (first, len) for obj/mix arrays
		INIT_WITH_0 ( int64_t, double, BlobLocator_t );
	};
	union {
		BlobLocator_t m_sName;				///< node name locator
		INIT_WITH_0 ( int64_t, BlobLocator_t );
	};
	int 				m_iNext = -1;		///< idx of next sibling
	ESphJsonType m_eType { JSON_TOTAL };	///< node type

	inline int GetLength() const { return m_dChildren.m_iLen; }

	JsonNode_t() = default;
	explicit JsonNode_t ( ESphJsonType eType )
	{
		m_eType = eType;
		m_sName.m_iLen = 0;
		m_sValue.m_iLen = 0;
	}

};
#define YYSTYPE JsonNode_t

namespace // static unnamed
{
	void StoreInt ( CSphVector<BYTE> & dBsonBuffer, int v )
	{
		StoreNUM32LE ( dBsonBuffer.AddN ( sizeof ( DWORD ) ), v );
	}

	void StoreBigint ( CSphVector<BYTE> & dBsonBuffer, int64_t v )
	{
		StoreBigintLE ( dBsonBuffer.AddN ( sizeof ( int64_t ) ), v );
	}

	void PackInt ( CSphVector<BYTE> & dBsonBuffer, DWORD v )
	{
//		assert ( v<16777216 ); // strings over 16M bytes and arrays over 16M entries are not supported
		if ( v<0x000000FC )
		{
			dBsonBuffer.Add ( BYTE ( v ) );
		} else if ( v<0x00010000 )
		{
			dBsonBuffer.Add ( 252 );
			dBsonBuffer.Add ( BYTE ( v & 255 ) );
			dBsonBuffer.Add ( BYTE ( v >> 8 ) );
		} else if ( v<0x1000000 )
		{
			dBsonBuffer.Add ( 253 );
			dBsonBuffer.Add ( BYTE ( v & 255 ) );
			dBsonBuffer.Add ( BYTE ( ( v >> 8 ) & 255 ) );
			dBsonBuffer.Add ( BYTE ( v >> 16 ) );
		} else
		{
			dBsonBuffer.Add ( 254 );
			StoreInt ( dBsonBuffer, v );
		}
	}

	inline void PackStr ( CSphVector<BYTE> & dBsonBuffer, const char * s, int iLen )
	{
		assert ( iLen<=0x00FFFFFF );
		iLen = Min ( iLen, 0x00ffffff );
		PackInt ( dBsonBuffer, iLen );
		auto * pOut = dBsonBuffer.AddN ( iLen );
		if ( iLen )
			memmove ( pOut, s, iLen );
	}

	inline void PackStr ( CSphVector<BYTE> & dBsonBuffer, const char * s )
	{
		PackStr ( dBsonBuffer, s, (int) strlen(s));
	}

	BYTE * PackStrUnescaped ( CSphVector<BYTE> & dBsonBuffer, const char * s, int iLen )
	{
		iLen = Min ( iLen, 0x00ffffff );
		auto iLenSize = PackedLen ( iLen );
		int iOfs = dBsonBuffer.GetLength ();
		auto * pOut = dBsonBuffer.AddN ( iLen + iLenSize ) + iLenSize;
		if ( iLen )
			iLen = JsonUnescape ( ( char * ) pOut, s, iLen );

		iLenSize -= PackedLen ( iLen ); // difference between place for original and unescaped len
		if ( iLenSize )
		{
			assert ( iLenSize>0 && "Strange, unescaped json need more place than escaped" );
			memmove ( pOut - iLenSize, pOut, iLen );
		}
		dBsonBuffer.Resize ( iOfs );
		PackInt ( dBsonBuffer, iLen );
		return dBsonBuffer.AddN ( iLen );
	}

	inline DWORD JsonKeyMask ( CSphVector<BYTE> & dBsonBuffer, const BlobLocator_t &sKey )
	{
		return sphJsonKeyMask ( ( const char * ) &dBsonBuffer[sKey.m_iStart], sKey.m_iLen );
	}

	void StoreMask ( CSphVector<BYTE> & dBsonBuffer, int iOfs, DWORD uMask )
	{
#if UNALIGNED_RAM_ACCESS && USE_LITTLE_ENDIAN
		*( DWORD * ) ( dBsonBuffer.begin () + iOfs ) = uMask;
#else
		for ( int i=0; i<4; ++i )
		{
			dBsonBuffer[iOfs+i] = BYTE ( uMask & 0xff );
			uMask >>= 8;
		}
#endif

	}

	/// reserve a single byte for a yet-unknown length, to be written later with PackSize()
	/// returns its offset, to be used by PackSize() to both calculate and stored the length
	int ReserveSize ( CSphVector<BYTE> & dBsonBuffer )
	{
		int iOfs = dBsonBuffer.GetLength ();
		dBsonBuffer.Resize ( iOfs + 1 );
		return iOfs;
	}

	/// compute current length from the offset reserved with ReserveSize(), and pack the value back there
	/// in most cases that single byte is enough; if not, we make room by memmove()ing the data
	void PackSize ( CSphVector<BYTE> & dBsonBuffer, int iOfs )
	{
		int iSize = dBsonBuffer.GetLength () - iOfs - 1;
		int iPackLen = PackedLen ( iSize );

		if ( iPackLen!=1 )
		{
			dBsonBuffer.Resize ( iOfs + iPackLen + iSize );
			memmove ( dBsonBuffer.Begin () + iOfs + iPackLen, dBsonBuffer.Begin () + iOfs + 1, iSize );
		}

		dBsonBuffer.Resize ( iOfs );
		PackInt ( dBsonBuffer, iSize );
		dBsonBuffer.AddN ( iSize );
	}

	void Finalize ( CSphVector<BYTE> & dBsonBuffer )
	{
		dBsonBuffer.Add ( JSON_EOF );
	}

	void DebugIndent ( int iLevel )
	{
		for ( int i = 0; i<iLevel; ++i )
			printf ( "    " );
	}

	void DebugDump ( ESphJsonType eType, const BYTE ** ppData, int iLevel )
	{
		DebugIndent ( iLevel );

		const BYTE * p = *ppData;

		switch ( eType )
		{
		case JSON_INT32: printf ( "JSON_INT32 %d\n", sphJsonLoadInt ( &p ) );
			break;
		case JSON_INT64: printf ( "JSON_INT64 " INT64_FMT "\n", sphJsonLoadBigint ( &p ) );
			break;
		case JSON_DOUBLE: printf ( "JSON_DOUBLE %lf\n", sphQW2D ( sphJsonLoadBigint ( &p ) ) );
			break;
		case JSON_STRING:
		{
			int iLen = sphJsonUnpackInt ( &p );
			CSphString sVal;
			sVal.SetBinary ( ( const char * ) p, iLen );
			printf ( "JSON_STRING \"%s\"\n", sVal.cstr () );
			p += iLen;
			break;
		}

		case JSON_TRUE: printf ( "JSON_TRUE\n" );
			break;
		case JSON_FALSE: printf ( "JSON_FALSE\n" );
			break;
		case JSON_NULL: printf ( "JSON_NULL\n" );
			break;
		case JSON_EOF: printf ( "JSON_EOF\n" );
			break;

			// associative arrays
		case JSON_ROOT:
		case JSON_OBJECT:
		{
			if ( eType==JSON_OBJECT )
				sphJsonUnpackInt ( &p );

			DWORD uMask = sphGetDword ( p );
			printf ( "%s (bloom mask: 0x%08x)\n", eType==JSON_OBJECT ? "JSON_OBJECT" : "JSON_ROOT", (uint32_t)uMask );
			p += 4; // skip bloom table
			while ( true )
			{
				auto eInnerType = ( ESphJsonType ) *p++;
				if ( eInnerType==JSON_EOF )
					break;
				const int iStrLen = sphJsonUnpackInt ( &p );
				CSphString sVal;
				sVal.SetBinary ( ( const char * ) p, iStrLen );
				DebugIndent ( iLevel + 1 );
				printf ( "\"%s\"", sVal.cstr () );
				p += iStrLen;
				DebugDump ( eInnerType, &p, iLevel + 1 );
			}
			break;
		}

		case JSON_MIXED_VECTOR:
		{
			int iTotalLen = sphJsonUnpackInt ( &p );
			int iLen = sphJsonUnpackInt ( &p );
			printf ( "JSON_MIXED_VECTOR [%d] (%d bytes)\n", iLen, iTotalLen );
			for ( int i = 0; i<iLen; ++i )
			{
				auto eInnerType = ( ESphJsonType ) *p++;
				DebugDump ( eInnerType, &p, iLevel + 1 );
			}
			break;
		}

			// optimized arrays ( note they can't be empty )
		case JSON_STRING_VECTOR:
		{
			sphJsonUnpackInt ( &p );
			int iLen = sphJsonUnpackInt ( &p );
			printf ( "JSON_STRING_VECTOR (%d) [", iLen );
			for ( int i = 0; i<iLen; ++i )
			{
				int iStrLen = sphJsonUnpackInt ( &p );
				CSphString sVal;
				sVal.SetBinary ( ( const char * ) p, iStrLen );
				printf ( "\"%s\"%s", sVal.cstr (), i<iLen - 1 ? "," : "]\n" );
				p += iStrLen;
			}
			break;
		}
		case JSON_INT32_VECTOR:
		{
			int iLen = sphJsonUnpackInt ( &p );
			printf ( "JSON_INT32_VECTOR (%d) [", iLen );
			for ( int i = 0; i<iLen; ++i )
				printf ( "%d%s", sphJsonLoadInt ( &p ), i<iLen - 1 ? "," : "]\n" );
			break;
		}
		case JSON_INT64_VECTOR:
		{
			int iLen = sphJsonUnpackInt ( &p );
			printf ( "JSON_INT64_VECTOR (%d) [", iLen );
			for ( int i = 0; i<iLen; ++i )
				printf ( INT64_FMT"%s", sphJsonLoadBigint ( &p ), i<iLen - 1 ? "," : "]\n" );
			break;
		}
		case JSON_DOUBLE_VECTOR:
		{
			int iLen = sphJsonUnpackInt ( &p );
			printf ( "JSON_DOUBLE_VECTOR (%d) [", iLen );
			for ( int i = 0; i<iLen; ++i )
				printf ( "%lf%s", sphQW2D ( sphJsonLoadBigint ( &p ) ), i<iLen - 1 ? "," : "]\n" );
			break;
		}

		default: printf ( "UNKNOWN\n" );
			break;
		}
		*ppData = p;
	}

	void DebugDump ( CSphVector<BYTE> & dBsonBuffer, const BYTE * p )
	{
		JsonEscapedBuilder dOut;
		sphJsonFormat ( dOut, dBsonBuffer.Begin () );
		printf ( "sphJsonFormat: %s\n", dOut.cstr () );
		printf ( "Blob size: %d bytes\n", dBsonBuffer.GetLength () );
		ESphJsonType eType = sphJsonFindFirst ( &p );
		DebugDump ( eType, &p, 0 );
		printf ( "\n" );
	}

	void AddStringData ( CSphVector<BYTE> & dBson, const VecTraits_T<CSphString> & dData )
	{
		auto iStartPos = ReserveSize ( dBson );
		PackInt ( dBson, dData.GetLength () );
		dData.Apply ( [&dBson] ( const CSphString & s ) { PackStr ( dBson, s.cstr () ); } );
		PackSize ( dBson, iStartPos );
	}

	void AddStringData ( CSphVector<BYTE> & dBson, const VecTraits_T<const char *> & dData )
	{
		auto iStartPos = ReserveSize ( dBson );
		PackInt ( dBson, dData.GetLength () );
		dData.Apply ( [&dBson] ( const char * sz ) { PackStr ( dBson, sz ); } );
		PackSize ( dBson, iStartPos );
	}

	void AddIntData ( CSphVector<BYTE> & dBson, const VecTraits_T<int> & dData )
	{
		PackInt ( dBson, dData.GetLength () );
		dData.Apply ( [&dBson] ( int i ) { StoreInt ( dBson, i ); } );
	}

	void AddIntData ( CSphVector<BYTE> & dBson, const VecTraits_T<int64_t> & dData )
	{
		PackInt ( dBson, dData.GetLength () );
		dData.Apply ( [&dBson] ( int64_t i ) { StoreInt ( dBson, int ( i ) ); } );
	}

	void AddBigintData ( CSphVector<BYTE> & dBson, const VecTraits_T<int64_t> & dData )
	{
		PackInt ( dBson, dData.GetLength () );
		dData.Apply ( [&dBson] ( int64_t i ) { StoreBigint ( dBson, i ); } );
	}

	void AddDoubleData ( CSphVector<BYTE> & dBson, const VecTraits_T<double> & dData )
	{
		PackInt ( dBson, dData.GetLength () );
		dData.Apply ( [&dBson] ( double f ) { StoreBigint ( dBson, sphD2QW ( f ) ); } );
	}
}

// store, pack, unpack, bloom, etc.
class BsonHelper : ISphNoncopyable
{
public:
	CSphVector<BYTE> &m_dBsonBuffer;

	BsonHelper ( CSphVector<BYTE> & dBuffer )
	: m_dBsonBuffer ( dBuffer )
	{
		// reserve 4 bytes for Bloom mask
		StoreInt ( 0 );
	}

	inline void StoreInt ( int v )
	{
		::StoreInt ( m_dBsonBuffer, v );
	}

	inline void StoreBigint ( int64_t v )
	{
		::StoreBigint ( m_dBsonBuffer, v );
	}

	inline void Add ( BYTE b )
	{
		m_dBsonBuffer.Add ( b );
	}

	inline void PackStr ( const char * s, int iLen )
	{
		::PackStr ( m_dBsonBuffer, s, iLen );
	}

	inline void PackStr ( const char * s )
	{
		::PackStr ( m_dBsonBuffer, s );
	}

	inline void PackInt ( DWORD v )
	{
		::PackInt ( m_dBsonBuffer, v );
	}

	inline BYTE * PackStrUnescaped ( const char * s, int iLen )
	{
		return ::PackStrUnescaped ( m_dBsonBuffer, s, iLen );
	}

	inline DWORD JsonKeyMask ( const BlobLocator_t &sKey )
	{
		return ::JsonKeyMask ( m_dBsonBuffer, sKey );
	}

	inline void StoreMask ( int iOfs, DWORD uMask )
	{
		::StoreMask ( m_dBsonBuffer, iOfs, uMask );
	}

	inline int ReserveSize ()
	{
		return ::ReserveSize ( m_dBsonBuffer );
	}

	inline void PackSize ( int iOfs )
	{
		::PackSize ( m_dBsonBuffer, iOfs );
	}

	inline void Finalize ()
	{
		::Finalize ( m_dBsonBuffer );
	}

	inline void DebugDump ( const BYTE * p )
	{
		::DebugDump ( m_dBsonBuffer, p );
	}
};

/// actually, JSON-to-SphinxBSON converter helper, but who cares
class JsonParser_c : public BsonHelper
{
	bool m_bAutoconv;
	bool m_bToLowercase;
public:
	void * m_pScanner = nullptr;
	const char * m_pLastToken = nullptr;
	StringBuilder_c &m_sError;
	const char * m_pSource = nullptr;
	CSphVector<JsonNode_t> m_dNodes;

public:
	JsonParser_c ( CSphVector<BYTE> &dBuffer, bool bAutoconv, bool bToLowercase, StringBuilder_c &sError )
		: BsonHelper (dBuffer)
		, m_bAutoconv ( bAutoconv )
		, m_bToLowercase ( bToLowercase )
		, m_sError ( sError )
	{}

private:
	void PackNodeStr ( const JsonNode_t &tNode )
	{
		PackStrUnescaped ( m_pSource + tNode.m_sValue.m_iStart, tNode.m_sValue.m_iLen );
	}

	// take string from source locator, write packed/unescaped string to output and return it's locator
	void WriteKeyUnescaped ( BlobLocator_t &sKey )
	{
		auto sPacked = PackStrUnescaped ( m_pSource + sKey.m_iStart, sKey.m_iLen );
		sKey.m_iStart = m_dBsonBuffer.Idx ( sPacked );
		sKey.m_iLen = m_dBsonBuffer.GetLength () - sKey.m_iStart;
		if ( m_bToLowercase )
			for ( auto &c : VecTraits_T<char> ( ( char * ) sPacked, sKey.m_iLen ) )
				c = ( char ) Mytolower ( c ); // OPTIMIZE! not sure if significant, but known to be hell slow
	}

public:

#define JSON_FOREACH( _index, _node ) \
    for ( auto i=0, _index=_node.m_dChildren.m_iStart; i<_node.GetLength(); ++i, _index = m_dNodes[_index].m_iNext )

	void NumericFixup ( JsonNode_t &tNode )
	{
		// auto-convert string values, if necessary
		if ( m_bAutoconv && tNode.m_eType==JSON_STRING )
			if ( !sphJsonStringToNumber ( m_pSource + tNode.m_sValue.m_iStart + 1, tNode.m_sValue.m_iLen - 2
										  , tNode.m_eType, tNode.m_iValue, tNode.m_fValue ) )
				return;

		// parser and converter emits int64 values, fix them up to int32 if possible
		if ( tNode.m_eType==JSON_INT64 && tNode.m_iValue==int64_t ( int ( tNode.m_iValue ) ) )
			tNode.m_eType = JSON_INT32;
	}

	ESphJsonType OptimizeMixedVector ( JsonNode_t &tNode )
	{
		assert ( tNode.m_eType==JSON_MIXED_VECTOR );

		if ( !tNode.m_dChildren.m_iLen )
			return JSON_MIXED_VECTOR;

		JSON_FOREACH ( j, tNode )
			NumericFixup ( m_dNodes[j] );

		ESphJsonType eBase = m_dNodes[tNode.m_dChildren.m_iStart].m_eType;
		bool bAllSame = true;
		JSON_FOREACH ( j, tNode )
			if ( eBase!=m_dNodes[j].m_eType )
				bAllSame = false;
		if ( bAllSame )
			switch ( eBase )
			{
			case JSON_INT32: return JSON_INT32_VECTOR;
			case JSON_INT64: return JSON_INT64_VECTOR;
			case JSON_DOUBLE: return JSON_DOUBLE_VECTOR;
			case JSON_STRING: return JSON_STRING_VECTOR;
			default: break; // type matches across all entries, but we do not have a special format for that type
			}
		return JSON_MIXED_VECTOR;
	}

	bool WriteRoot ( JsonNode_t &tNode ) // called in case of tompost obj enclosed in {}.
	{
		DWORD uMask = 0;
		JSON_FOREACH ( j, tNode )
		{
			auto &dNode = m_dNodes[j];
			WriteNode ( dNode, true );
			uMask |= JsonKeyMask ( dNode.m_sName );
		}
		m_dBsonBuffer.Add ( JSON_EOF );
		StoreMask ( 0, uMask );
		return true;
	}

	// main proc which do whole magic over the topmost obj/array
	bool WriteNode ( JsonNode_t &tNode, bool bNamed=false )
	{
		// convert int64 to int32, strings to numbers if needed
		NumericFixup ( tNode );

		auto eType = tNode.m_eType;

		// process mixed vector, convert to generic vector if possible
		if ( eType==JSON_MIXED_VECTOR )
			eType = OptimizeMixedVector ( tNode );

		// write type
		m_dBsonBuffer.Add ( eType );

		// write key name if exists
		if ( bNamed && tNode.m_sName.m_iLen )
			WriteKeyUnescaped ( tNode.m_sName );

		// write nodes
		switch ( eType )
		{
			// basic types
		case JSON_INT32: StoreInt ( ( int ) tNode.m_iValue );
			break;
		case JSON_INT64: StoreBigint ( tNode.m_iValue );
			break;
		case JSON_DOUBLE: StoreBigint ( sphD2QW ( tNode.m_fValue ) );
			break;
		case JSON_STRING: PackNodeStr ( tNode );
			break;

			// literals
		case JSON_TRUE:
		case JSON_FALSE:
		case JSON_NULL:
			// no content
			break;

			// associative arrays
		case JSON_OBJECT:
		{
			DWORD uMask = 0;
			int iOfs = ReserveSize ();
			StoreInt ( 0 );
			JSON_FOREACH ( j, tNode )
			{
				auto &dNode = m_dNodes[j];
				WriteNode ( dNode, true );
				uMask |= JsonKeyMask ( dNode.m_sName );
			}
			m_dBsonBuffer.Add ( JSON_EOF );
			StoreMask ( iOfs + 1, uMask );
			PackSize ( iOfs ); // MUST be in this order, because PackSize() might move the data!
			break;
		}

			// mixed array
		case JSON_MIXED_VECTOR:
		{
			int iOfs = ReserveSize ();
			PackInt ( tNode.GetLength () );
			JSON_FOREACH ( j, tNode )
				WriteNode ( m_dNodes[j] );
			PackSize ( iOfs );
			break;
		}

			// optimized (generic) arrays
		case JSON_INT32_VECTOR: PackInt ( tNode.GetLength () );
			JSON_FOREACH ( j, tNode )
				StoreInt ( ( int ) m_dNodes[j].m_iValue );
			break;
		case JSON_INT64_VECTOR: PackInt ( tNode.GetLength () );
			JSON_FOREACH ( j, tNode )
				StoreBigint ( m_dNodes[j].m_iValue );
			break;
		case JSON_DOUBLE_VECTOR: PackInt ( tNode.GetLength () );
			JSON_FOREACH ( j, tNode )
				StoreBigint ( sphD2QW ( m_dNodes[j].m_fValue ) );
			break;
		case JSON_STRING_VECTOR:
		{
			int iOfs = ReserveSize ();
			PackInt ( tNode.GetLength () );
			JSON_FOREACH ( j, tNode )
				PackNodeStr ( m_dNodes[j] );
			PackSize ( iOfs );
			break;
		}
		default: assert ( 0 && "internal error: unhandled type" );
			return false;
		}
		return true;
	}

	void AddNode ( const JsonNode_t & tNode )
	{
		m_dNodes.Add ( tNode );
	}
};

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_NO_UNISTD_H 1
#define YY_DECL inline static int my_lex ( YYSTYPE * lvalp, void * yyscanner, JsonParser_c * pParser )

#include "flexsphinxjson.c"

static void yyerror ( JsonParser_c * pParser, const char * sMessage )
{
	yy2lex_unhold ( pParser->m_pScanner );
	pParser->m_sError.Sprintf ( "P10: %s near '%s'", sMessage, pParser->m_pLastToken );
}

#ifndef NDEBUG

// using a proxy to be possible to debug inside yylex
static int yylex ( YYSTYPE * lvalp, JsonParser_c * pParser )
{
	int res = my_lex ( lvalp, pParser->m_pScanner, pParser );
	return res;
}

#else
inline static int yylex ( YYSTYPE * lvalp, JsonParser_c * pParser )
{
	return my_lex ( lvalp, pParser->m_pScanner, pParser );
}
#endif

#include "bissphinxjson.c"
#include "sphinxutils.h"

bool sphJsonParse ( CSphVector<BYTE>& dData, const CSphString& sFileName, CSphString& sError )
{
	auto iFileSize = sphGetFileSize ( sFileName, &sError );
	if ( iFileSize<0 )
		return false;

	CSphAutofile tFile;
	if ( tFile.Open ( sFileName, SPH_O_READ, sError )<0 )
		return false;

	CSphFixedVector<char> dJsonText { iFileSize + 2 }; // +4 for zero-gap at the end
	auto iRead = (int64_t)sphReadThrottled ( tFile.GetFD (), dJsonText.begin (), iFileSize );
	if ( iRead!=iFileSize )
		return false;

	decltype ( dJsonText )::POLICY_T::Zero ( dJsonText.begin() + iFileSize, 2 );
	return sphJsonParse ( dData, dJsonText.begin(), false, false, false, sError );
}

bool sphJsonParse ( CSphVector<BYTE> & dData, char * sData, bool bAutoconv, bool bToLowercase, bool bCheckSize, CSphString & sError )
{
	StringBuilder_c sMsg;
	auto bResult = sphJsonParse ( dData, sData, bAutoconv, bToLowercase, bCheckSize, sMsg );
	sMsg.MoveTo ( sError );
	return bResult;
}

bool sphJsonParse ( CSphVector<BYTE> &dData, char * sData, bool bAutoconv, bool bToLowercase, bool bCheckSize, StringBuilder_c &sMsg )
{
	auto iLen = (int) strlen ( sData );
	if ( sData[iLen+1]!=0 )
	{
		sMsg << "internal error: input data passed to sphJsonParse() must be terminated with a double zero";
		return false;
	}

	JsonParser_c tParser ( dData, bAutoconv, bToLowercase, sMsg );
	yy2lex_init ( &tParser.m_pScanner );

	tParser.m_pSource = sData; // sphJsonParse() is intentionally destructive, no need to copy data here

	YY_BUFFER_STATE tLexerBuffer = yy2_scan_buffer ( sData, iLen+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sMsg << "internal error: yy_scan_buffer() failed";
		return false;
	}

	int iRes = yyparse ( &tParser );
	yy2_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yy2lex_destroy ( tParser.m_pScanner );

	tParser.Finalize();

	if ( bCheckSize && dData.AllocatedBytes()>=0x400000 )
	{
		sMsg << "data exceeds 0x400000 bytes";
		iRes = -1;
	}

	if ( iRes!=0 )
		dData.Reset();

	return ( iRes==0 );
}

//////////////////////////////////////////////////////////////////////////

DWORD sphJsonKeyMask ( const char * sKey, int iLen )
{
	DWORD uCrc = sphCRC32 ( sKey, iLen );

	DWORD uBloom1 = ( 1UL<<( uCrc & 31 ) );
	DWORD uBloom2 = ( 1UL<<( ( uCrc>>8 ) & 31 ) );

	// DWORD overflow protection as 0 at bloom mask means not the ROOT node
	return ( ( uBloom1 + uBloom2 ) ? ( uBloom1 + uBloom2 ) : ( uBloom1 | uBloom2 ) );
}

static inline DWORD KeyMask ( const char * s )
{
	return sphJsonKeyMask ( s, (int) strlen ( s ) );
}

// returns -1 if size is unreachable (for remote agents)
int sphJsonNodeSize ( ESphJsonType eType, const BYTE *pData )
{
	int iLen;
	const BYTE * p = pData;
	switch ( eType )
	{
	case JSON_INT32:
		return 4;
	case JSON_INT64:
	case JSON_DOUBLE:
		return 8;
	case JSON_INT32_VECTOR:
		if ( !p )
			return -1;
		iLen = sphJsonUnpackInt ( &p );
		return int ( p - pData ) + iLen * 4;
	case JSON_INT64_VECTOR:
	case JSON_DOUBLE_VECTOR:
		if ( !p )
			return -1;
		iLen = sphJsonUnpackInt ( &p );
		return int ( p - pData ) + iLen * 8;
	case JSON_STRING:
	case JSON_STRING_VECTOR:
	case JSON_MIXED_VECTOR:
	case JSON_OBJECT:
		if ( !p )
			return -1;
		iLen = sphJsonUnpackInt ( &p );
		return int ( p - pData ) + iLen;
	case JSON_ROOT:
		if ( !p )
			return -1;
		p += 4; // skip filter
		while (true)
		{
			auto eNode = (ESphJsonType) *p++;
			if ( eNode==JSON_EOF )
				break;
			// skip key and node
			iLen = sphJsonUnpackInt ( &p );
			p += iLen;
			sphJsonSkipNode ( eNode, &p );
		}
		return int ( p - pData );
	default:
		return 0;
	}
}


int sphJsonSingleNodeSize ( ESphJsonType eType )
{
	switch ( eType )
	{
	case JSON_INT32_VECTOR:
	case JSON_INT64_VECTOR:
	case JSON_DOUBLE_VECTOR:
	case JSON_STRING:
	case JSON_STRING_VECTOR:
	case JSON_MIXED_VECTOR:
	case JSON_OBJECT:
	case JSON_ROOT:
		return -1;
	case JSON_INT32:
		return 4;
	case JSON_INT64:
	case JSON_DOUBLE:
		return 8;
	default:
		return 0;
	}
}

void sphJsonSkipNode ( ESphJsonType eType, const BYTE ** ppData )
{
	int iSize = sphJsonNodeSize ( eType, *ppData );
	*ppData += iSize;
}


int sphJsonFieldLength ( ESphJsonType eType, const BYTE * pData )
{
	const BYTE * p = pData;
	int iCount = 0;
	switch ( eType )
	{
	case JSON_INT32:
	case JSON_INT64:
	case JSON_DOUBLE:
		return 1;
	case JSON_STRING_VECTOR:
	case JSON_MIXED_VECTOR:
		sphJsonUnpackInt ( &p );
		return sphJsonUnpackInt ( &p );
	case JSON_INT32_VECTOR:
	case JSON_INT64_VECTOR:
	case JSON_DOUBLE_VECTOR:
		return sphJsonUnpackInt ( &p );
	case JSON_OBJECT:
		sphJsonUnpackInt ( &p ); // skip size, then continue
		// [[clang::fallthrough]];
	case JSON_ROOT:
		p += 4; // skip filter
		while (true)
		{
			auto eNode = (ESphJsonType) *p++;
			if ( eNode==JSON_EOF )
				break;
			// skip name of node
			int iLen = sphJsonUnpackInt ( &p );
			p += iLen;
			sphJsonSkipNode ( eNode, &p );
			++iCount;
		}
		return iCount;
	default:
		return 0;
	}
}


ESphJsonType sphJsonGetCoreType ( ESphJsonType eType )
{
	switch ( eType )
	{
	case JSON_INT32_VECTOR: return JSON_INT32;
	case JSON_INT64_VECTOR: return JSON_INT64;
	case JSON_DOUBLE_VECTOR: return JSON_DOUBLE;
	case JSON_STRING_VECTOR: return JSON_STRING;
	default:
		return JSON_EOF;
	}
}


ESphJsonType sphJsonFindFirst ( const BYTE ** ppData )
{
	// non-zero bloom mask? that is JSON_ROOT (basically a JSON_OBJECT without node header)
	if ( sphGetDword(*ppData) )
		return JSON_ROOT;

	// zero mask? must be followed by the type byte (typically JSON_EOF)
	auto eType = (ESphJsonType)((*ppData)[4]);
	*ppData += 5;
	return eType;
}


ESphJsonType sphJsonFindByKey ( ESphJsonType eType, const BYTE ** ppValue, const void * pKey, int iLen, DWORD uMask )
{
	if ( eType!=JSON_OBJECT && eType!=JSON_ROOT )
		return JSON_EOF;

	const BYTE * p = *ppValue;
	if ( eType==JSON_OBJECT )
		sphJsonUnpackInt ( &p );

	if ( ( sphGetDword(p) & uMask )!=uMask )
		return JSON_EOF;

	p += 4;
	while (true)
	{
		eType = (ESphJsonType) *p++;
		if ( eType==JSON_EOF )
			break;
		int iStrLen = sphJsonUnpackInt ( &p );
		p += iStrLen;
		if ( iStrLen==iLen && !memcmp ( p-iStrLen, pKey, iStrLen ) )
		{
			*ppValue = p;
			return eType;
		}
		sphJsonSkipNode ( eType, &p );
	}

	return JSON_EOF;
}


ESphJsonType sphJsonFindByIndex ( ESphJsonType eType, const BYTE ** ppValue, int iIndex )
{
	if ( iIndex<0 )
		return JSON_EOF;

	const BYTE * p = *ppValue;
	switch ( eType )
	{
	case JSON_INT32_VECTOR:
	case JSON_INT64_VECTOR:
	case JSON_DOUBLE_VECTOR:
		{
			int iLen = sphJsonUnpackInt ( &p );
			if ( iIndex>=iLen )
				return JSON_EOF;
			p += iIndex * ( eType==JSON_INT32_VECTOR ? 4 : 8 );
			*ppValue = p;
			return eType==JSON_INT32_VECTOR ? JSON_INT32
				: eType==JSON_INT64_VECTOR ? JSON_INT64
				: JSON_DOUBLE;
		}
	case JSON_STRING_VECTOR:
		{
			sphJsonUnpackInt ( &p );
			int iLen = sphJsonUnpackInt ( &p );
			if ( iIndex>=iLen )
				return JSON_EOF;
			for ( int i=0; i<iIndex; ++i )
			{
				int iStrLen = sphJsonUnpackInt ( &p );
				p += iStrLen;
			}
			*ppValue = p;
			return JSON_STRING;
		}
	case JSON_MIXED_VECTOR:
		{
			sphJsonUnpackInt ( &p );
			int iLen = sphJsonUnpackInt ( &p );
			if ( iIndex>=iLen )
				return JSON_EOF;
			for ( int i=0; i<iIndex; ++i )
			{
				eType = (ESphJsonType)*p++;
				sphJsonSkipNode ( eType, &p );
			}
			eType = (ESphJsonType)*p;
			*ppValue = p+1;
			return eType;
		}
	default:
		return JSON_EOF;
	}
}

//////////////////////////////////////////////////////////////////////////

static const BYTE * JsonFormatStr ( JsonEscapedBuilder &sOut, const BYTE * p, bool bQuote=true )
{
	int iLen = sphJsonUnpackInt ( &p );
	if ( bQuote )
		sOut.AppendEscaped ( ( const char * ) p, EscBld::eEscape, iLen );
	else
		sOut.AppendChunk ( {(const char *) p, iLen} );
	return p+iLen;
}

void sphJsonFormat ( JsonEscapedBuilder & sOut, const BYTE * pData )
{
	if ( !pData )
		return;
	ESphJsonType eType = sphJsonFindFirst ( &pData );

	// check for the empty root
	if ( eType==JSON_EOF )
		sOut << "{}";
	else
		sphJsonFieldFormat ( sOut, pData, eType );
}


const BYTE * sphJsonFieldFormat ( JsonEscapedBuilder & sOut, const BYTE * pData, ESphJsonType eType, bool bQuoteString )
{
	const BYTE szDouble = 32;
	char sDouble[szDouble];

	// format value
	const BYTE * p = pData;
	switch ( eType )
	{
		case JSON_INT32:
			sOut.Sprintf ("%d", sphJsonLoadInt ( &p )); // NOLINT
			break;

		case JSON_INT64:
			sOut.Sprintf ( "%l", sphJsonLoadBigint ( &p ) ); // NOLINT
			break;

		case JSON_DOUBLE:
		{
			auto iLen = snprintf ( sDouble, szDouble, "%lf", sphQW2D ( sphJsonLoadBigint ( &p ) ) ); // NOLINT
			sOut.AppendChunk ( {sDouble, iLen} );
			break;
		}
		case JSON_STRING:
			p = JsonFormatStr ( sOut, p, bQuoteString );
			break;

		case JSON_STRING_VECTOR:
			sOut.GrowEnough ( sphJsonUnpackInt ( &p ) );
			{
				auto _ = sOut.Array();
				for ( int i = sphJsonUnpackInt ( &p ); i > 0; --i )
					p = JsonFormatStr ( sOut, p );
			}
			break;

		case JSON_INT32_VECTOR:
			{
				auto _ = sOut.Array();
				for ( int i = sphJsonUnpackInt ( &p ); i > 0; --i )
					sOut.Sprintf ( "%d", sphJsonLoadInt ( &p ) );
				break;
			}
		case JSON_INT64_VECTOR:
			{
				auto _ = sOut.Array();
				for ( int i = sphJsonUnpackInt ( &p ); i > 0; --i )
					sOut.Sprintf ( "%l", sphJsonLoadBigint ( &p ) );
				break;
			}
		case JSON_DOUBLE_VECTOR:
			{
				auto _ = sOut.Array ();
				for ( int i = sphJsonUnpackInt ( &p ); i>0; --i )
				{
					auto iLen = snprintf ( sDouble, szDouble, "%lf", sphQW2D ( sphJsonLoadBigint ( &p ) ) ); // NOLINT
					sOut.AppendChunk ( {sDouble, iLen} );
				}
				break;
			}
		case JSON_MIXED_VECTOR:
			{
				auto _ = sOut.Array ();
				sphJsonUnpackInt ( &p );
				for ( int i = sphJsonUnpackInt ( &p ); i>0; --i )
				{
					auto eNode = ( ESphJsonType ) *p++;
					p = sphJsonFieldFormat ( sOut, p, eNode, true );
				}
				break;
			}
		case JSON_ROOT:
		case JSON_OBJECT:
			{
				if ( eType==JSON_OBJECT )
					sphJsonUnpackInt ( &p );
				p += 4; // skip bloom table
				sOut.ObjectBlock();
				while (true)
				{
					ScopedComma_c sMuteCommas ( sOut, nullptr );
					auto eNode = (ESphJsonType) *p++;
					if ( eNode==JSON_EOF )
						break;
					p = JsonFormatStr ( sOut, p );
					sOut.RawC (':');
					p = sphJsonFieldFormat ( sOut, p, eNode, true );
				}
				sOut.FinishBlock (false);
				break;
			}
		case JSON_TRUE:		sOut << ( bQuoteString ? "true" : "1" ); break;
		case JSON_FALSE:	sOut << ( bQuoteString ? "false" : "0"); break;
		case JSON_NULL:		sOut << ( bQuoteString ? "null" : nullptr); break;
		case JSON_EOF:		break;
		case JSON_TOTAL:	break;
	}
	return p;
}


bool sphJsonNameSplit ( const char * sName, CSphString * sColumn )
{
	if ( !sName )
		return false;

	// find either '[' or '.', what comes first
	const char * pSep = sName;
	while ( *pSep && *pSep!='.' && *pSep!='[' )
	{
		// check for invalid characters
		if ( !sphIsAttr( *pSep ) && *pSep!=' ' )
			return false;
		++pSep;
	}

	if ( !*pSep )
		return false;

	int iSep = int ( pSep - sName );
	if ( sColumn )
	{
		sColumn->SetBinary ( sName, iSep );
		sColumn->Trim();
	}
	return true;
}

JsonKey_t::JsonKey_t ( const char * sKey, int iLen )
{
	m_iLen = iLen;
	m_uMask = sphJsonKeyMask ( sKey, m_iLen );
	m_sKey.SetBinary ( sKey, m_iLen );
}


uint64_t sphJsonUnpackOffset ( uint64_t uPacked )
{
	return uPacked & 0x00ffffffffffffffULL;
}


ESphJsonType sphJsonUnpackType ( uint64_t uPacked )
{
	return (ESphJsonType)( uPacked >> 56 );
}


uint64_t sphJsonPackTypeOffset ( ESphJsonType eType, uint64_t uOffset )
{
	assert ( uOffset<=0x00ffffffffffffffULL );
	return uOffset | ( ((uint64_t)eType)<<56 );
}


bool sphJsonInplaceUpdate ( ESphJsonType eValueType, int64_t iValue, ISphExpr * pExpr, BYTE * pBlobPool, const CSphRowitem * pRow, bool bUpdate )
{
	if ( !pExpr || !pBlobPool )
		return false;

	pExpr->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );

	CSphMatch tMatch;
	tMatch.m_pStatic = pRow;

	uint64_t uPacked = pExpr->Int64Eval ( tMatch );
	BYTE * pData = pBlobPool + sphJsonUnpackOffset ( uPacked );
	ESphJsonType eType = sphJsonUnpackType ( uPacked );

	switch ( eType )
	{
	case JSON_INT32:
		if ( eValueType==JSON_DOUBLE )
			iValue = (int64_t)sphQW2D ( iValue );
		if ( int64_t(int(iValue))!=iValue )
			return false;
		if ( bUpdate )
			StoreNUM32LE ( pData, (int)iValue );
		break;
	case JSON_INT64:
		if ( bUpdate )
			StoreBigintLE ( pData, eValueType==JSON_DOUBLE ? (int64_t)sphQW2D ( iValue ) : iValue );
		break;
	case JSON_DOUBLE:
		if ( bUpdate )
			StoreBigintLE ( pData, eValueType==JSON_DOUBLE ? iValue : sphD2QW ( (double)iValue ) );
		break;
	default:
		return false;
	}
	return true;
}

inline bool is_json_whitespace (char c)
{
	return c==' ' || c=='\n' || c=='\r' || c=='\t' || c=='\f';
}

// note that usual parsing of num is done by grammar parser;
// this function is used only when 'autoconversion' is in game
bool sphJsonStringToNumber ( const char * s, int iLen, ESphJsonType &eType, int64_t &iVal, double &fVal )
{
	auto sEnd = s + iLen;
	// skip whitespace
	while ( s<sEnd && is_json_whitespace(*s) )
		++s;

	if ( s>=sEnd )
		return false;

	char sVal[64] = {0};
	char * pVal = sVal;
	char sDecimal_point = '.'; // m.b. set it locale-dependent?
	bool bIsFloat = false;
	for (int i=0; i<63 && s<sEnd ; ++i, ++pVal, ++s)
	{
		switch ( *s )
		{
		case '.':
			*pVal = sDecimal_point;
			bIsFloat = true;
			break;
		case 'e':
		case 'E':
			*pVal = 'e';
			bIsFloat = true;
			break;
		case '-':
		case '+':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			*pVal = *s;
			break;
		default:
			if ( !is_json_whitespace(*s) )
				return false;
			i=64;
			--pVal;
		}
	}

	iLen = int ( pVal-sVal );
	if ( !iLen || iLen==64 )
		return false;

	const char * pCheck = s;
	// check string conversion is valid if only whitespaces left
	while ( pCheck<sEnd && is_json_whitespace(*pCheck) )
		++pCheck;
	if ( pCheck!=sEnd )
		return false;

	char * pTail;
	errno = 0;
	if ( !bIsFloat )
	{
		int64_t iRes = strtoll ( sVal, &pTail, 10 );
		if ( pTail==pVal && errno!=ERANGE )
		{
			eType = JSON_INT64;
			iVal = iRes;
			return true;
		}
	} else
	{
		double fRes = strtod ( sVal, &pTail );
		if ( pTail==pVal && errno!=ERANGE )
		{
			eType = JSON_DOUBLE;
			fVal = fRes;
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////

#if DISABLE_MEMROUTINES
static void * cJsonMalloc ( size_t uSize )
{
	return malloc (uSize);
}

static void cJsonFree ( void * pPtr )
{
	free ( pPtr );
}

#else

static void* cJsonMalloc ( size_t uSize )
{
	return new BYTE[uSize];
}

static void cJsonFree ( void* pPtr )
{
	delete[] (BYTE*)pPtr;
}

#endif


void sphInitCJson()
{
	cJSON_Hooks tHooks { cJsonMalloc, cJsonFree };
	cJSON_InitHooks ( &tHooks );
}

//////////////////////////////////////////////////////////////////////////

JsonObj_c::JsonObj_c ( bool bArray )
{
	if ( bArray )
		m_pRoot = cJSON_CreateArray();
	else
		m_pRoot = cJSON_CreateObject();

	assert ( m_pRoot );
}


JsonObj_c::JsonObj_c ( cJSON * pRoot, bool bOwner )
{
	m_pRoot = pRoot;
	m_bOwner = bOwner;
}


JsonObj_c::JsonObj_c ( const char * szJson )
{
	m_pRoot = cJSON_Parse ( szJson );
}

JsonObj_c::JsonObj_c ( Str_t sJson )
{
	m_pRoot = cJSON_Parse ( sJson.first );
}


JsonObj_c::JsonObj_c ( JsonObj_c && rhs ) noexcept
	: m_pRoot ( nullptr )
{
	Swap (rhs);
}

void JsonObj_c::Swap ( JsonObj_c& rhs ) noexcept
{
	::Swap ( m_pRoot, rhs.m_pRoot );
	::Swap ( m_bOwner, rhs.m_bOwner );
}


JsonObj_c::~JsonObj_c()
{
	if ( m_pRoot )
	{
		if ( m_bOwner )
			cJSON_Delete(m_pRoot);

		m_pRoot = nullptr;
	}
}


JsonObj_c::operator bool() const
{
	return !Empty();
}


JsonObj_c & JsonObj_c::operator = ( JsonObj_c rhs )
{
	Swap(rhs);
	return *this;
}


void JsonObj_c::AddStr ( const char * szName, const char * szValue )
{
	assert ( m_pRoot );
	cJSON_AddStringToObject ( m_pRoot, szName, szValue );
}


void JsonObj_c::AddStr ( const char * szName, const CSphString & sValue )
{
	AddStr ( szName, sValue.cstr() );
}


void JsonObj_c::AddInt ( const char * szName, int64_t iValue )
{
	assert ( m_pRoot );
	cJSON_AddItemToObject ( m_pRoot, szName, CreateInt(iValue).Leak() );
}


void JsonObj_c::AddUint ( const char * szName, uint64_t uValue )
{
	assert ( m_pRoot );
	cJSON_AddItemToObject ( m_pRoot, szName, CreateUint(uValue).Leak() );
}


void JsonObj_c::AddBool ( const char * szName, bool bValue )
{
	assert ( m_pRoot );
	cJSON_AddBoolToObject ( m_pRoot, szName, bValue ? 1 : 0 );
}


void JsonObj_c::AddItem ( const char * szName, JsonObj_c & tObj )
{
	assert ( m_pRoot );
	cJSON_AddItemToObject ( m_pRoot, szName, tObj.Leak() );
}


void JsonObj_c::AddItem ( JsonObj_c & tObj )
{
	assert ( m_pRoot );
	cJSON_AddItemToArray ( m_pRoot, tObj.Leak() );
}


void JsonObj_c::DelItem ( const char * szName )
{
	assert ( m_pRoot );
	cJSON_DeleteItemFromObject ( m_pRoot, szName );
}


int JsonObj_c::Size() const
{
	assert ( m_pRoot );
	return cJSON_GetArraySize ( m_pRoot );
}


bool JsonObj_c::IsInt() const
{
	assert ( m_pRoot );
	return cJSON_IsInteger(m_pRoot);
}


bool JsonObj_c::IsUint() const
{
	assert ( m_pRoot );
	return cJSON_IsUInteger(m_pRoot);
}


bool JsonObj_c::IsDbl() const
{
	assert ( m_pRoot );
	return !!cJSON_IsNumber ( m_pRoot );
}


bool JsonObj_c::IsNum() const
{
	assert ( m_pRoot );
	return !!cJSON_IsNumeric ( m_pRoot );
}


bool JsonObj_c::IsBool() const
{
	assert ( m_pRoot );
	return !!cJSON_IsBool ( m_pRoot );
}


bool JsonObj_c::IsObj () const
{
	assert ( m_pRoot );
	return !!cJSON_IsObject ( m_pRoot );
}


bool JsonObj_c::IsStr() const
{
	assert ( m_pRoot );
	return !!cJSON_IsString ( m_pRoot );
}


bool JsonObj_c::IsArray() const
{
	assert ( m_pRoot );
	return !!cJSON_IsArray ( m_pRoot );
}

bool JsonObj_c::IsNull() const
{
	return !!cJSON_IsNull ( m_pRoot );
}


JsonObj_c JsonObj_c::operator[] ( int iItem ) const
{
	assert ( m_pRoot );
	return JsonObj_c ( cJSON_GetArrayItem ( m_pRoot, iItem ), false );
}


JsonObj_c JsonObj_c::GetItem ( const char * szName ) const
{
	assert ( m_pRoot );
	return JsonObj_c ( cJSON_GetObjectItem ( m_pRoot, szName ), false );
}


JsonObj_c JsonObj_c::GetIntItem ( const char * szName, CSphString & sError, bool bIgnoreMissing ) const
{
	JsonObj_c tChild = GetChild ( szName, sError, bIgnoreMissing );
	if ( !tChild )
		return JsonNull;

	if ( !tChild.IsInt() )
	{
		sError.SetSprintf ( R"("%s" property value should be an integer)", szName );
		return JsonNull;
	}

	return tChild;
}


JsonObj_c JsonObj_c::GetIntItem ( const char * szName1, const char * szName2, CSphString & sError ) const
{
	JsonObj_c tResult = GetIntItem ( szName1, sError, true );
	if ( !tResult )
	{
		if ( !sError.IsEmpty() )
			return JsonNull;

		tResult = GetIntItem ( szName2, sError, true );
	}

	return tResult;
}


JsonObj_c JsonObj_c::GetBoolItem ( const char * szName, CSphString & sError, bool bIgnoreMissing ) const
{
	JsonObj_c tChild = GetChild ( szName, sError, bIgnoreMissing );
	if ( !tChild )
		return JsonNull;

	if ( !tChild.IsBool() )
	{
		sError.SetSprintf ( R"("%s" property value should be a boolean)", szName );
		return JsonNull;
	}

	return tChild;
}


JsonObj_c JsonObj_c::GetStrItem ( const char * szName, CSphString & sError, bool bIgnoreMissing ) const
{
	JsonObj_c tChild = GetChild ( szName, sError, bIgnoreMissing );
	if ( !tChild )
		return JsonNull;

	if ( !tChild.IsStr() )
	{
		sError.SetSprintf ( R"("%s" property value should be a string)", szName );
		return JsonNull;
	}

	if ( tChild.StrVal().IsEmpty() && !bIgnoreMissing )
	{
		sError.SetSprintf ( R"("%s" property empty)", szName );
		return JsonNull;
	}

	return tChild;
}


JsonObj_c JsonObj_c::GetStrItem ( const char * szName1, const char * szName2, CSphString & sError ) const
{
	JsonObj_c tResult = GetStrItem ( szName1, sError, true );
	if ( !tResult )
	{
		if ( !sError.IsEmpty() )
			return JsonNull;

		tResult = GetStrItem ( szName2, sError, true );
	}

	return tResult;
}


JsonObj_c JsonObj_c::GetObjItem ( const char * szName, CSphString & sError, bool bIgnoreMissing ) const
{
	JsonObj_c tChild = GetChild ( szName, sError, bIgnoreMissing );
	if ( !tChild )
		return JsonNull;

	if ( !tChild.IsObj() )
	{
		sError.SetSprintf ( R"("%s" property value should be an object)", szName );
		return JsonNull;
	}

	return tChild;
}


JsonObj_c JsonObj_c::GetArrayItem ( const char * szName, CSphString & sError, bool bIgnoreMissing ) const
{
	JsonObj_c tChild = GetChild ( szName, sError, bIgnoreMissing );
	if ( !tChild )
		return JsonNull;

	if ( !tChild.IsArray() )
	{
		sError.SetSprintf ( R"("%s" property value should be an array)", szName );
		return JsonNull;
	}

	return tChild;
}


bool JsonObj_c::FetchIntItem ( int & iValue, const char * szName, CSphString & sError, bool bIgnoreMissing ) const
{
	JsonObj_c tItem = GetIntItem ( szName, sError, bIgnoreMissing );
	if ( tItem )
		iValue = (int)tItem.IntVal();
	else if ( !sError.IsEmpty() )
		return false;

	return true;
}


bool JsonObj_c::FetchBoolItem ( bool & bValue, const char * szName, CSphString & sError, bool bIgnoreMissing ) const
{
	JsonObj_c tItem = GetBoolItem ( szName, sError, bIgnoreMissing );
	if ( tItem )
		bValue = tItem.BoolVal();
	else if ( !sError.IsEmpty() )
		return false;

	return true;
}


bool JsonObj_c::FetchStrItem ( CSphString & sValue, const char * szName, CSphString & sError, bool bIgnoreMissing ) const
{
	JsonObj_c tItem = GetStrItem ( szName, sError, bIgnoreMissing );
	if ( tItem )
		sValue = tItem.StrVal();
	else if ( !sError.IsEmpty() )
		return false;

	return true;
}


bool JsonObj_c::HasItem ( const char * szItem ) const
{
	assert ( m_pRoot );
	return !!cJSON_HasObjectItem ( m_pRoot, szItem );
}


JsonObj_c JsonObj_c::CreateInt ( int64_t iInt )
{
	return JsonObj_c ( cJSON_CreateInteger(iInt) );
}


JsonObj_c JsonObj_c::CreateUint ( uint64_t uInt )
{
	return JsonObj_c ( cJSON_CreateUInteger(uInt) );
}


JsonObj_c JsonObj_c::CreateStr ( const CSphString & sStr )
{
	return JsonObj_c ( cJSON_CreateString ( sStr.cstr() ) );
}


bool JsonObj_c::Empty() const
{
	return !m_pRoot;
}


const char * JsonObj_c::Name() const
{
	assert ( m_pRoot );
	return m_pRoot->string;
}


int64_t	JsonObj_c::IntVal() const
{
	assert ( m_pRoot );
	return m_pRoot->valueint;
}


bool JsonObj_c::BoolVal() const
{
	assert ( m_pRoot );
	return !!cJSON_IsTrue(m_pRoot);
}


float JsonObj_c::FltVal() const
{
	return (float)DblVal();
}


double JsonObj_c::DblVal() const
{
	assert ( m_pRoot );
	return m_pRoot->valuedouble;
}


const char * JsonObj_c::SzVal() const
{
	assert ( m_pRoot );
	return m_pRoot->valuestring;
}


CSphString JsonObj_c::StrVal() const
{
	assert ( m_pRoot );
	return m_pRoot->valuestring;
}


const char * JsonObj_c::GetErrorPtr() const
{
	return cJSON_GetErrorPtr();
}


bool JsonObj_c::GetError ( const char * szBuf, int iBufLen, CSphString & sError ) const
{
	if ( !GetErrorPtr() )
		return false;

	const int iErrorWindowLen = 20;

	const char * szErrorStart = GetErrorPtr() - iErrorWindowLen/2;
	if ( szErrorStart < szBuf )
		szErrorStart = szBuf;

	int iLen = iErrorWindowLen;
	if ( szErrorStart-szBuf+iLen>iBufLen )
		iLen = iBufLen - int ( szErrorStart - szBuf );

	sError.SetSprintf ( "JSON parse error at: %.*s", iLen, szErrorStart );
	return true;
}


bool JsonObj_c::GetError( const char* szBuf, int iBufLen ) const
{
	CSphString sError;
	if ( GetError ( szBuf, iBufLen, sError ) )
		return !TlsMsg::Err ( sError );
	return false;
}


cJSON * JsonObj_c::Leak()
{
	cJSON * pRoot = m_pRoot;
	m_pRoot = nullptr;
	return pRoot;
}


JsonObj_c JsonObj_c::GetChild ( const char * szName, CSphString & sError, bool bIgnoreMissing ) const
{
	JsonObj_c tChild = GetItem(szName);
	if ( !tChild )
	{
		if ( !bIgnoreMissing )
			sError.SetSprintf ( "\"%s\" property missing", szName );

		return JsonNull;
	}

	return tChild;
}


cJSON * JsonObj_c::GetRoot()
{
	return m_pRoot;
}


CSphString JsonObj_c::AsString ( bool bFormat ) const
{
	if ( m_pRoot )
	{
		// we can't take this string and just adopt it because we need extra 'gap' bytes at the end
		char * szResult;
		
		if ( bFormat )
			szResult = cJSON_Print ( m_pRoot );
		else
			szResult = cJSON_PrintUnformatted ( m_pRoot );

		CSphString sResult ( szResult );
		cJsonFree ( szResult );
		return sResult;
	}

	return "";
}


JsonObj_c & JsonObj_c::operator++()
{
	if ( m_pRoot )
		m_pRoot = m_pRoot->next;

	return *this;
}


JsonObj_c JsonObj_c::operator*()
{
	return JsonObj_c(m_pRoot,false);
}


JsonObj_c JsonObj_c::begin() const
{
	return JsonObj_c ( m_pRoot ? m_pRoot->child : nullptr, false );
}


JsonObj_c JsonObj_c::end() const
{
	return JsonNull;
}

void JsonObj_c::ReplaceItem ( int iIndex, JsonObj_c & tObj )
{
	assert ( m_pRoot );
	assert ( cJSON_IsArray ( m_pRoot ) );
	assert ( iIndex<cJSON_GetArraySize ( m_pRoot ) );
	cJSON_ReplaceItemInArray ( m_pRoot, iIndex, tObj.Leak() );
}

JsonObj_c JsonObj_c::Clone () const
{
	if ( !m_pRoot )
		return JsonNull;

	JsonObj_c tNew ( cJSON_Duplicate ( m_pRoot, true ) );
	return tNew;
}

//////////////////////////////////////////////////////////////////////////

using namespace bson;

bool bson::IsAssoc ( const NodeHandle_t & dNode )
{
	return dNode.second==JSON_ROOT || dNode.second==JSON_OBJECT;
}

bool bson::IsArray ( const NodeHandle_t &dNode )
{
	return dNode.second==JSON_STRING_VECTOR
		|| dNode.second==JSON_MIXED_VECTOR
		|| dNode.second==JSON_INT32_VECTOR
		|| dNode.second==JSON_INT64_VECTOR
		|| dNode.second==JSON_DOUBLE_VECTOR;
}

bool bson::IsPODBlob ( const NodeHandle_t &dNode )
{
	return dNode.second==JSON_STRING
		|| dNode.second==JSON_INT32_VECTOR
		|| dNode.second==JSON_INT64_VECTOR
		|| dNode.second==JSON_DOUBLE_VECTOR;
}

bool bson::IsString ( const NodeHandle_t &dNode )
{
	return dNode.second==JSON_STRING;
}

bool bson::IsInt ( const NodeHandle_t &dNode )
{
	return dNode.second==JSON_INT32
		|| dNode.second==JSON_INT64;
}

bool bson::IsDouble ( const NodeHandle_t &dNode )
{
	return dNode.second==JSON_DOUBLE;
}

bool bson::IsNumeric ( const NodeHandle_t &dNode )
{
	return IsInt ( dNode ) || IsDouble ( dNode );
}

bool bson::Bool ( const NodeHandle_t &tLocator, bool bDefault )
{
	switch ( tLocator.second )
	{
	case JSON_INT32:
	case JSON_INT64: return !!Int ( tLocator );
	case JSON_DOUBLE: return 0.0!=Double ( tLocator );
	case JSON_TRUE: return true;
	case JSON_FALSE: return false;
		// fixme! Do we need also process here values like "True" (the string)?
	default: break;
	}
	return bDefault;
}

int64_t bson::Int ( const NodeHandle_t &tLocator, int64_t iDefault )
{
	const BYTE * p = tLocator.first;
	switch ( tLocator.second )
	{
	case JSON_INT32: return GetNUM32LE<int> ( p );
	case JSON_INT64: return sphJsonLoadBigint ( &p );
	case JSON_DOUBLE: return int64_t ( sphQW2D ( sphJsonLoadBigint ( &p ) ) );
	case JSON_TRUE: return 1;
	case JSON_FALSE: return 0;
	case JSON_STRING:
	{
		auto dBlob = RawBlob ( tLocator );
		if ( !dBlob.second )
			return 0;
		ESphJsonType eType;
		double fValue;
		int64_t iValue;
		if ( !sphJsonStringToNumber ( dBlob.first, dBlob.second, eType, iValue, fValue ) )
			return 0;
		if ( eType==JSON_DOUBLE )
			return int64_t ( fValue );
		return iValue;
	}
	default: break;
	}
	return iDefault;
}

double bson::Double ( const NodeHandle_t &tLocator, double fDefault )
{
	const BYTE * p = tLocator.first;
	switch ( tLocator.second )
	{
	case JSON_INT32: return GetNUM32LE<int> ( tLocator.first );
	case JSON_INT64: return sphJsonLoadBigint ( &p );
	case JSON_DOUBLE: return sphQW2D ( sphJsonLoadBigint ( &p ) );
	case JSON_TRUE: return 1.0;
	case JSON_FALSE: return 0.0;
	case JSON_STRING:
	{
		auto dBlob = RawBlob ( tLocator );
		if ( !dBlob.second )
			return 0.0;
		ESphJsonType eType;
		double fValue;
		int64_t iValue;
		if ( !sphJsonStringToNumber ( dBlob.first, dBlob.second, eType, iValue, fValue ) )
			return 0.0;
		if ( eType==JSON_DOUBLE )
			return fValue;
		return iValue;
	}
	default: break;
	}
	return fDefault;
}

CSphString bson::String ( const NodeHandle_t &tLocator, CSphString sDefault )
{
	if ( tLocator.second!=JSON_STRING )
		return sDefault;

	auto dBlob = bson::RawBlob ( tLocator );
	CSphString sResult;
	sResult.SetBinary ( dBlob.first, dBlob.second );
	return sResult;
}

Str_t bson::ToStr ( const NodeHandle_t & tLocator )
{
	if ( tLocator.second!=JSON_STRING )
		return dEmptyStr;

	auto dBlob = bson::RawBlob ( tLocator );
	return dBlob;
}

void bson::ForEach ( const NodeHandle_t &tLocator, Action_f&& fAction )
{
	const BYTE * p = tLocator.first;
	switch ( tLocator.second )
	{
	case JSON_STRING_VECTOR:
	{
		sphJsonUnpackInt ( &p );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			fAction ( { p, JSON_STRING } );
			int iStrLen = sphJsonUnpackInt ( &p );
			p += iStrLen;
		}
		break;
	}
	case JSON_MIXED_VECTOR:
	{
		sphJsonUnpackInt ( &p );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			auto eType = ( ESphJsonType ) *p++;
			fAction ( { p, eType } );
			sphJsonSkipNode ( eType, &p );
		}
		break;
	}
	case JSON_OBJECT: sphJsonUnpackInt ( &p );
		// [[clang::fallthrough]];
	case JSON_ROOT:
	{
		p += 4; // skip bloom
		while ( true )
		{
			auto eType = ( ESphJsonType ) *p++;
			if ( eType==JSON_EOF )
				return;
			int iStrLen = sphJsonUnpackInt ( &p );
			p += iStrLen; // skip name
			fAction ( { p, eType } );
			sphJsonSkipNode ( eType, &p );
		}
	}
	case JSON_INT32_VECTOR:
	case JSON_INT64_VECTOR:
	case JSON_DOUBLE_VECTOR:
	{
		auto eType = sphJsonGetCoreType ( tLocator.second );
		auto iSize = sphJsonSingleNodeSize ( eType );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			fAction ( { p, eType } );
			p += iSize;
		}
		break;
	}
	default: break;
	}
}

void bson::ForEach ( const NodeHandle_t &tLocator, NamedAction_f&& fAction )
{
	const BYTE * p = tLocator.first;
	switch ( tLocator.second )
	{
	case JSON_STRING_VECTOR:
	{
		sphJsonUnpackInt ( &p );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			fAction ( "", { p, JSON_STRING } );
			int iStrLen = sphJsonUnpackInt ( &p );
			p += iStrLen;
		}
		break;
	}
	case JSON_MIXED_VECTOR:
	{
		sphJsonUnpackInt ( &p );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			auto eType = ( ESphJsonType ) *p++;
			fAction ( "", { p, eType } );
			sphJsonSkipNode ( eType, &p );
		}
		break;
	}

	case JSON_OBJECT: sphJsonUnpackInt ( &p );
		// [[clang::fallthrough]];
	case JSON_ROOT:
	{
		p += 4; // skip bloom
		while ( true )
		{
			auto eType = ( ESphJsonType ) *p++;
			if ( eType==JSON_EOF )
				return;
			int iStrLen = sphJsonUnpackInt ( &p );
			CSphString sName ( ( const char * ) p, iStrLen );
			p += iStrLen; // skip name
			fAction ( std::move (sName), { p, eType } );
			sphJsonSkipNode ( eType, &p );
		}
	}
	case JSON_INT32_VECTOR:
	case JSON_INT64_VECTOR:
	case JSON_DOUBLE_VECTOR:
	{
		auto eType = sphJsonGetCoreType ( tLocator.second );
		auto iSize = sphJsonSingleNodeSize ( eType );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			fAction ( "", { p, eType } );
			p += iSize;
		}
		break;
	}
	default: break;
	}
}

void bson::ForSome ( const NodeHandle_t &tLocator, CondAction_f&& fAction )
{
	const BYTE * p = tLocator.first;
	switch ( tLocator.second )
	{
	case JSON_STRING_VECTOR:
	{
		sphJsonUnpackInt ( &p );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			if ( !fAction ( { p, JSON_STRING } ) )
				return;
			int iStrLen = sphJsonUnpackInt ( &p );
			p += iStrLen;
		}
		break;
	}
	case JSON_MIXED_VECTOR:
	{
		sphJsonUnpackInt ( &p );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			auto eType = ( ESphJsonType ) *p++;
			if ( !fAction ( { p, eType } ) )
				return;
			sphJsonSkipNode ( eType, &p );
		}
		break;
	}
	case JSON_OBJECT: sphJsonUnpackInt ( &p );
		// [[clang::fallthrough]];
	case JSON_ROOT:
	{
		p += 4; // skip bloom
		while ( true )
		{
			auto eType = ( ESphJsonType ) *p++;
			if ( eType==JSON_EOF )
				return;
			int iStrLen = sphJsonUnpackInt ( &p );
			p += iStrLen; // skip name
			if ( !fAction ( { p, eType } ) )
				return;
			sphJsonSkipNode ( eType, &p );
		}
	}
	case JSON_INT32_VECTOR:
	case JSON_INT64_VECTOR:
	case JSON_DOUBLE_VECTOR:
	{
		auto eType = sphJsonGetCoreType ( tLocator.second );
		auto iSize = sphJsonSingleNodeSize ( eType );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			if ( !fAction ( { p, eType } ) )
				return;
			p += iSize;
		}
		break;
	}
	default: break;
	}
}

void bson::ForSome ( const NodeHandle_t &tLocator, CondNamedAction_f&& fAction )
{
	const BYTE * p = tLocator.first;
	switch ( tLocator.second )
	{
	case JSON_STRING_VECTOR:
	{
		sphJsonUnpackInt ( &p );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			if ( !fAction ( "", { p, JSON_STRING } ) )
				return;
			int iStrLen = sphJsonUnpackInt ( &p );
			p += iStrLen;
		}
		break;
	}
	case JSON_MIXED_VECTOR:
	{
		sphJsonUnpackInt ( &p );
		int iLen = sphJsonUnpackInt ( &p );
		for ( int i = 0; i<iLen; ++i )
		{
			auto eType = ( ESphJsonType ) *p++;
			if ( !fAction ( "", { p, eType } ) )
				return;
			sphJsonSkipNode ( eType, &p );
		}
		break;
	}

	case JSON_OBJECT: sphJsonUnpackInt ( &p );
		// [[clang::fallthrough]];
	case JSON_ROOT:
	{
		p += 4; // skip bloom
		while ( true )
		{
			auto eType = ( ESphJsonType ) *p++;
			if ( eType==JSON_EOF )
				return;
			int iStrLen = sphJsonUnpackInt ( &p );
			CSphString sName ( ( const char * ) p, iStrLen );
			p += iStrLen; // skip name
			if ( !fAction ( std::move (sName), { p, eType } ) )
				return;
			sphJsonSkipNode ( eType, &p );
		}
	}
	default: break;
	}
}

std::pair<const char *, int> bson::RawBlob ( const NodeHandle_t &tLocator )
{
	if ( ::IsPODBlob ( tLocator ) )
	{
		const BYTE * p = tLocator.first;
		int iLen = sphJsonUnpackInt ( &p );
		return { ( const char * ) p, iLen };
	}
	return { nullptr, 0 };
}

Bson_c::Bson_c ( const VecTraits_T<BYTE> &dBlob )
{
	if ( dBlob.IsEmpty () )
		return;
	const BYTE * pData = dBlob.begin ();
	m_dData.second = sphJsonFindFirst ( &pData );
	m_dData.first = pData;
}


// fast lookup (no need to iterate just to know if empty or not)
bool Bson_c::IsEmpty() const
{
	const BYTE * p = m_dData.first;
	switch ( m_dData.second )
	{
	case JSON_STRING_VECTOR: case JSON_MIXED_VECTOR:
		sphJsonUnpackInt ( &p ); return 0==sphJsonUnpackInt ( &p );
	case JSON_INT32_VECTOR: case JSON_INT64_VECTOR: case JSON_DOUBLE_VECTOR: case JSON_STRING:
		return 0==sphJsonUnpackInt ( &p );
	case JSON_OBJECT:
		sphJsonUnpackInt ( &p ); // skip size, no break then
		// [[clang::fallthrough]];
	case JSON_ROOT:
		return JSON_EOF == ( ESphJsonType ) p[4]; // skip bloom filter
	default:
		return true;
	}
}

int Bson_c::CountValues () const
{
	return sphJsonFieldLength ( m_dData.second, m_dData.first );
}

int Bson_c::StandaloneSize() const
{
	const BYTE * p = m_dData.first;
	switch ( m_dData.second )
	{
		case JSON_ROOT:
			return sphJsonNodeSize ( JSON_ROOT, p );
		case JSON_OBJECT:
			return sphJsonUnpackInt ( &p );
		case JSON_INT32_VECTOR:
		case JSON_INT64_VECTOR:
		case JSON_DOUBLE_VECTOR:
		case JSON_STRING_VECTOR:
		case JSON_MIXED_VECTOR:
		{
			int iSize = sphJsonNodeSize ( m_dData.second, p );
			if ( iSize<0 )
				break;
			return 5 + iSize;
		}
		case JSON_EOF:
			return 5; // eof is actually empty root.
		default:
			break;
	}
	// technically speaking we can easy return here any other type (as INT, TRUE, etc) in root,
	// but our original parser returns _only_ object or array in the root
	return -1;
}

bool Bson_c::StrEq ( const char * sValue ) const
{
	if ( !::IsString ( m_dData ) )
		return false;
	const BYTE * p = m_dData.first;
	int iStrLen = sphJsonUnpackInt ( &p );
	auto iLen = (int) strlen ( sValue );
	if ( iStrLen==iLen && !memcmp ( p, sValue, iStrLen ) )
		return true;
	return false;
}

NodeHandle_t Bson_c::ChildByName ( const char * sName ) const
{
	// access by key available only for 'root' or 'object' types
	if ( !IsAssoc () )
		return nullnode;

	const BYTE * p = m_dData.first;
	if ( m_dData.second==JSON_OBJECT )
		sphJsonUnpackInt ( &p );

	auto iLen = (int) strlen ( sName );

	// fast reject by bloom filter
	auto uMask = sphJsonKeyMask ( sName, iLen );
	if ( ( sphGetDword ( p ) & uMask )!=uMask )
	{
		m_sError << "No such member " << sName;
		return nullnode;
	}

	p += 4;
	while ( true )
	{
		auto eType = ( ESphJsonType ) *p++;
		if ( eType==JSON_EOF )
			return nullnode;
		int iStrLen = sphJsonUnpackInt ( &p );
		p += iStrLen;
		if ( iStrLen==iLen && !memcmp ( p - iStrLen, sName, iStrLen ) )
			return { p, eType };
		sphJsonSkipNode ( eType, &p );
	}
}

NodeHandle_t Bson_c::ChildByIndex ( int iIndex ) const
{
	if ( iIndex<0 )
		return nullnode;

	// access by index available only for vectors
	if ( !IsArray () )
		return nullnode;

	const BYTE * p = m_dData.first;
	switch ( m_dData.second )
	{
	case JSON_INT32_VECTOR:
	case JSON_INT64_VECTOR:
	case JSON_DOUBLE_VECTOR:
	{
		int iLen = sphJsonUnpackInt ( &p );
		if ( iIndex>=iLen )
		{
			m_sError.Sprintf ( "In POD vector index %d is ouf of bound (%d)", iIndex, iLen );
			return nullnode;
		}
		p += iIndex * ( m_dData.second==JSON_INT32_VECTOR ? 4 : 8 );

		ESphJsonType eType = m_dData.second==JSON_INT32_VECTOR ? JSON_INT32
															   : m_dData.second==JSON_INT64_VECTOR ? JSON_INT64
																								   : JSON_DOUBLE;
		return { p, eType };
	}
	case JSON_STRING_VECTOR:
	{
		sphJsonUnpackInt ( &p );
		int iLen = sphJsonUnpackInt ( &p );
		if ( iIndex>=iLen )
		{
			m_sError.Sprintf ( "In string vector index %d is ouf of bound (%d)", iIndex, iLen );
			return nullnode;
		}
		for ( int i = 0; i<iIndex; ++i )
		{
			int iStrLen = sphJsonUnpackInt ( &p );
			p += iStrLen;
		}
		return { p, JSON_STRING };
	}
	case JSON_MIXED_VECTOR:
	{
		sphJsonUnpackInt ( &p );
		int iLen = sphJsonUnpackInt ( &p );
		if ( iIndex>=iLen )
		{
			m_sError.Sprintf ( "In mixed vector index %d is ouf of bound (%d)", iIndex, iLen );
			return nullnode;
		}
		for ( int i = 0; i<iIndex; ++i )
		{
			auto eType = ( ESphJsonType ) *p++;
			sphJsonSkipNode ( eType, &p );
		}
		return { p + 1, ( ESphJsonType ) *p };
	}
	default: break;
	}
	m_sError << "Node is not indexable";
	return nullnode;
}

NodeHandle_t Bson_c::ChildByPath ( const char * sPath ) const
{
	auto p = nullnode;
	StrVec_t dMembers;

	// split sPath by '.', then iterate by the chunks
	sphSplit ( dMembers, sPath, "." );
	for ( const auto &sMember : dMembers )
	{
		// split a chunk by '[', then iterate by chunks
		StrVec_t dIndices;
		sphSplit ( dIndices, sMember.scstr (), "[" );
		if ( dIndices.GetLength ()<1 )
		{
			m_sError << "something wrong in parsing " << sMember << " member of " << sPath;
			return nullnode;
		}
		auto &sKey = dIndices[0];

		// check if we're start from scratch
		if ( p==nullnode )
			p = m_dData;

		// first chunk is name. Navigate there
		if ( !sKey.IsEmpty () )
			p = Bson_c ( p ).ChildByName ( sKey.scstr () );

		if ( p==nullnode )
			return nullnode; // error! can't get by name...

		// walk throw indices
		for ( int i = 1; i<dIndices.GetLength (); ++i )
		{
			auto &sIdx = dIndices[i];
			sIdx.Trim ();

			if ( *sIdx.scstr ()==']' ) // obvious error of empty array
			{
				m_sError << "Can't look by empty index! " << sIdx;
				return nullnode;
			}

			// convert str to int
			char * sEnd;
			auto iIdx = ( int ) strtoll ( sIdx.scstr (), &sEnd, 10 );
			if ( *sEnd!=']' )
			{
				m_sError << "Can't convert index from string! " << sIdx;
				return nullnode; // error! can't convert index from string
			}
			p = Bson_c ( p ).ChildByIndex ( iIdx );

			if ( p==nullnode )
				return nullnode; // error! can't get by idx...
		}
	}
	return p;
}

bool Bson_c::HasAnyOf ( int iNames, ... ) const
{
	// access by key available only for 'root' or 'object' types
	if ( !IsAssoc () )
		return false;

	va_list ap;
	va_start ( ap, iNames );
	CSphFixedVector<const char*> dNames (iNames);
	for ( auto& sName : dNames )
		sName = va_arg ( ap, const char * );
	va_end ( ap );

	const BYTE * p = m_dData.first;
	if ( m_dData.second==JSON_OBJECT )
		sphJsonUnpackInt ( &p );

	// fast reject by bloom filter
	DWORD uNodeMask = sphGetDword ( p );
	bool bMayBe = false;
	for ( const char * sName : dNames )
	{
		auto iLen = (int) strlen ( sName );
		auto uMask = sphJsonKeyMask ( sName, iLen );
		bMayBe |= ( ( uNodeMask & uMask )==uMask );
		if ( bMayBe )
			break;
	}

	if ( !bMayBe ) // all bloom tries rejected
		return false;

	p += 4; // skip the bloom
	while ( true )
	{
		auto eType = ( ESphJsonType ) *p++;
		if ( eType==JSON_EOF )
			return false;
		int iStrLen = sphJsonUnpackInt ( &p );
		for ( const char * sName : dNames )
		{
			auto iLen = (int) strlen ( sName );
			if ( iStrLen==iLen && !memcmp ( p, sName, iStrLen ) )
				return true;
		}
		p += iStrLen;
		sphJsonSkipNode ( eType, &p );
	}
}

bool Bson_c::HasAnyOf ( std::initializer_list<const char *> dNames ) const
{
	// access by key available only for 'root' or 'object' types
	if ( !IsAssoc () )
		return false;

	const BYTE * p = m_dData.first;
	if ( m_dData.second==JSON_OBJECT )
		sphJsonUnpackInt ( &p );

	// fast reject by bloom filter
	DWORD uNodeMask = sphGetDword ( p );
	bool bMayBe = false;
	for ( const char * sName : dNames )
	{
		auto iLen = (int) strlen ( sName );
		auto uMask = sphJsonKeyMask ( sName, iLen );
		bMayBe |= ( ( uNodeMask & uMask )==uMask );
		if ( bMayBe )
			break;
	}

	if ( !bMayBe ) // all bloom tries rejected
		return false;

	p += 4; // skip the bloom
	while ( true )
	{
		auto eType = (ESphJsonType) *p++;
		if ( eType==JSON_EOF )
			return false;
		int iStrLen = sphJsonUnpackInt ( &p );
		for ( const char * sName : dNames )
		{
			auto iLen = (int) strlen ( sName );
			if ( iStrLen==iLen && !memcmp ( p, sName, iStrLen ) )
				return true;
		}
		p += iStrLen;
		sphJsonSkipNode ( eType, &p );
	}
}

std::pair<int, NodeHandle_t> Bson_c::GetFirstOf ( std::initializer_list<const char *> dNames ) const
{
	// access by key available only for 'root' or 'object' types
	if ( !IsAssoc () )
		return { -1, nullnode };

	const BYTE * p = m_dData.first;
	if ( m_dData.second==JSON_OBJECT )
		sphJsonUnpackInt ( &p );

	// fast reject by bloom filter
	DWORD uNodeMask = sphGetDword ( p );
	bool bMayBe = false;
	for ( const char * sName : dNames )
	{
		auto iLen = (int) strlen ( sName );
		auto uMask = sphJsonKeyMask ( sName, iLen );
		bMayBe |= ( ( uNodeMask & uMask )==uMask );
		if ( bMayBe )
			break;
	}

	if ( !bMayBe ) // all bloom tries rejected
		return { -1, nullnode };

	p += 4; // skip the bloom
	while ( true )
	{
		auto eType = (ESphJsonType) *p++;
		if ( eType==JSON_EOF )
			return { -1, nullnode };
		int iStrLen = sphJsonUnpackInt ( &p );
		int iIdx = 0;
		for ( const char * sName : dNames )
		{
			auto iLen = (int) strlen ( sName );
			if ( iStrLen==iLen && !memcmp ( p, sName, iStrLen ) )
				return { iIdx, { p+iStrLen, eType } };
			++iIdx;
		}
		p += iStrLen;
		sphJsonSkipNode ( eType, &p );
	}
}

bool Bson_c::Bool () const
{
	return bson::Bool ( m_dData );
}

int64_t Bson_c::Int () const
{
	return bson::Int ( m_dData );
}

double Bson_c::Double () const
{
	if ( m_dData.second==JSON_STRING )
	{
		auto dBlob = RawBlob ( m_dData );
		if ( !dBlob.second )
		{
			m_sError << "empty string passed to float conversion";
			return 0.0;
		}
		ESphJsonType eType;
		double fValue;
		int64_t iValue;
		if ( !sphJsonStringToNumber ( dBlob.first, dBlob.second, eType, iValue, fValue ) )
		{
			m_sError << "Error in conversion " << CSphString ( dBlob.first, dBlob.second )
					 << " to double";
			return 0.0;
		}
		if ( eType==JSON_DOUBLE )
			return fValue;
		return iValue;
	}
	return bson::Double ( m_dData );
}

CSphString Bson_c::String () const
{
	return bson::String ( m_dData );
}

void Bson_c::ForEach ( Action_f&& fAction ) const
{
	bson::ForEach ( m_dData, std::move ( fAction ) );
}

void Bson_c::ForEach ( NamedAction_f&& fAction ) const
{
	bson::ForEach ( m_dData, std::move ( fAction ) );
}

void Bson_c::ForSome ( CondAction_f&& fAction ) const
{
	bson::ForSome ( m_dData, std::move ( fAction ) );
}

void Bson_c::ForSome ( CondNamedAction_f&& fAction ) const
{
	bson::ForSome ( m_dData, std::move ( fAction ) );
}

bool Bson_c::BsonToJson ( CSphString & sResult ) const
{
	return BsonToJson ( sResult, true );
}

bool Bson_c::BsonToJson ( CSphString & sResult, bool bQuot ) const
{
	JsonEscapedBuilder sBuilder;
	if ( !m_dData.first )
		return false;

	// check for the empty root
	if ( m_dData.second==JSON_EOF )
		sBuilder << "{}";
	else
		sphJsonFieldFormat ( sBuilder, m_dData.first, m_dData.second, bQuot );

	sBuilder.MoveTo ( sResult );
	return true;
}

bool Bson_c::BsonToBson ( BYTE* pDst ) const
{
	switch ( m_dData.second )
	{
		case JSON_ROOT: // root - just dump as is
			memcpy ( pDst, m_dData.first, sphJsonNodeSize ( JSON_ROOT, m_dData.first ) );
			break;
		case JSON_OBJECT: // object - throw out first packed int (len), then save as is (it became root then)
			{
				const BYTE * p = m_dData.first;
				int iLen = sphJsonUnpackInt ( &p );
				memcpy ( pDst, p, iLen );
			}
			break;
		case JSON_INT32_VECTOR:
		case JSON_INT64_VECTOR:
		case JSON_DOUBLE_VECTOR:
		case JSON_STRING_VECTOR:
		case JSON_MIXED_VECTOR: // arrays - write zero bloom, then type, then dump the node (artificial root)
			StoreNUM32LE ( pDst, 0 );
			pDst[4] = m_dData.second;
			memcpy ( pDst+5, m_dData.first, sphJsonNodeSize ( m_dData.second, m_dData.first ) );
			break;
		case JSON_EOF:
			StoreNUM32LE ( pDst, 0 );
			pDst[4] = JSON_EOF;
			break;
		default: return false;
	}
	return true;
}

bool Bson_c::BsonToBson ( CSphVector<BYTE> &dOutput ) const
{
	int iSize = StandaloneSize ();
	if ( iSize<0 )
		return false;
	dOutput.Resize ( iSize );
	return BsonToBson ( dOutput.begin() );
}


const char * Bson_c::sError () const
{
	return m_sError.cstr();
}

BsonIterator_c::BsonIterator_c ( const NodeHandle_t &dParent )
{
	if ( bson::IsNullNode ( dParent ) )
		return;

	m_pData = dParent.first;
	m_eType = dParent.second;
	switch ( m_eType )
	{
		case JSON_STRING_VECTOR:
			sphJsonUnpackInt ( &m_pData );
			m_iSize = sphJsonUnpackInt ( &m_pData );
			m_dData = { m_pData, JSON_STRING };
			break;

		case JSON_MIXED_VECTOR:
			sphJsonUnpackInt ( &m_pData );
			m_iSize = sphJsonUnpackInt ( &m_pData );
			m_dData.second = ESphJsonType ( *m_pData++ );
			m_dData.first = m_pData;
		break;

		case JSON_OBJECT: sphJsonUnpackInt ( &m_pData );
			// [[clang::fallthrough]];
		case JSON_ROOT:
			{
				m_pData += 4; // skip bloom
				m_dData.second = ESphJsonType ( *m_pData++ );
				if ( m_dData.second == JSON_EOF )
					Finish();
				else
				{
					int iStrLen = sphJsonUnpackInt ( &m_pData );
					m_sName.SetBinary ( ( const char * ) m_pData, iStrLen );
					m_pData += iStrLen;
					m_dData.first = m_pData;
				}
				break;
			}

		case JSON_INT32_VECTOR:
			m_iSize = sphJsonUnpackInt ( &m_pData );
			m_dData = { m_pData, JSON_INT32 };
			break;

		case JSON_INT64_VECTOR:
			m_iSize = sphJsonUnpackInt ( &m_pData );
			m_dData = { m_pData, JSON_INT64 };
			break;

		case JSON_DOUBLE_VECTOR:
			m_iSize = sphJsonUnpackInt ( &m_pData );
			m_dData = { m_pData, JSON_DOUBLE };
			break;

		case JSON_INT32:
		case JSON_INT64:
		case JSON_DOUBLE:
		case JSON_STRING:
		case JSON_TRUE:
		case JSON_FALSE:
		case JSON_NULL:
			m_iSize = 1;
			m_dData = dParent;
			break;

		case JSON_EOF:
			Finish();
			break;

		default: break;
	}
	if ( !m_iSize )
		Finish();
}

bool BsonIterator_c::Next()
{
	switch ( m_eType )
	{
		case JSON_INT32:
		case JSON_INT64:
		case JSON_DOUBLE:
		case JSON_STRING:
		case JSON_TRUE:
		case JSON_FALSE:
		case JSON_NULL:
		case JSON_EOF:
			// nowhere to step next
			return Finish();

		case JSON_OBJECT: // these two have EOF as marker; no size.
		case JSON_ROOT:
		{
			sphJsonSkipNode ( m_dData.second, &m_pData );
			m_dData.second = ESphJsonType ( *m_pData );
			if ( m_dData.second==JSON_EOF ) // in case of empty root/obj EOF is already here
				return Finish ();

			++m_pData;
			int iStrLen = sphJsonUnpackInt ( &m_pData );
			m_sName.SetBinary ( ( const char * ) m_pData, iStrLen );
			m_pData += iStrLen;
			m_dData.first = m_pData;
			break;
		}

		case JSON_STRING_VECTOR: // these have size
		case JSON_INT32_VECTOR:
		case JSON_INT64_VECTOR:
		case JSON_DOUBLE_VECTOR:
		case JSON_MIXED_VECTOR:
			--m_iSize;
			if (m_iSize<=0)
				return Finish ();

			sphJsonSkipNode ( m_dData.second, &m_pData );
			if ( m_eType==JSON_MIXED_VECTOR )
				m_dData.second = ESphJsonType ( *m_pData++ );
			m_dData.first = m_pData;
			break;

		default:
			break;
	}
	return true;
}

inline ESphJsonType cJson2BsonType ( int iCjsonType )
{
	const ESphJsonType BsonTypes[] = { JSON_TOTAL, JSON_FALSE, JSON_TRUE, JSON_NULL, JSON_DOUBLE, JSON_STRING
									   , JSON_MIXED_VECTOR, JSON_OBJECT, JSON_TOTAL, JSON_INT64 };
	assert ( ( iCjsonType & 0xFF )>=0 );
	assert ( ( iCjsonType & 0xFF )<sizeof ( BsonTypes ) );
	return BsonTypes[iCjsonType & 0xFF];
}

class CJsonHelper
{
	BsonHelper m_dStorage;
	bool m_bAutoconv;
	bool m_bToLowercase;
//	StringBuilder_c m_sError;

public:
	CJsonHelper ( CSphVector<BYTE> &dBuffer, bool bAutoconv, bool bToLowercase )
		: m_dStorage ( dBuffer )
		, m_bAutoconv ( bAutoconv )
		, m_bToLowercase ( bToLowercase )
	{}

	inline ESphJsonType NumericFixup ( cJSON * pNode )
	{
		auto eOrigType = cJson2BsonType ( pNode->type );

		// auto-convert string values, if necessary
		if ( m_bAutoconv && eOrigType==JSON_STRING )
		{
			int64_t iFoo;
			if ( !sphJsonStringToNumber ( pNode->valuestring, (int) strlen ( pNode->valuestring ), eOrigType, iFoo, pNode->valuedouble ) )
				return eOrigType;
			pNode->valueint = iFoo;
		}

		if ( eOrigType==JSON_INT64 && pNode->valueint==int64_t ( int ( pNode->valueint ) ) )
			return JSON_INT32;
		return eOrigType;
	}

	inline int MeasureAndOptimizeVector ( cJSON * pCJSON, ESphJsonType &eType )
	{
		int iSize = 0;
		ESphJsonType eOutType = JSON_TOTAL;
		cJSON * pNode;
		bool bAllSame = true;
		cJSON_ArrayForEach( pNode, pCJSON )
		{
			if ( !iSize )
				eOutType = NumericFixup ( pNode );
			else if ( bAllSame && ( eOutType!=NumericFixup ( pNode ) ) )
				bAllSame = false;
			++iSize;
		}

		if ( !iSize )
			return 0;

		if ( bAllSame )
			switch ( eOutType )
			{
			case JSON_INT32: eType = JSON_INT32_VECTOR; break;
			case JSON_INT64: eType = JSON_INT64_VECTOR; break;
			case JSON_DOUBLE: eType = JSON_DOUBLE_VECTOR; break;
			case JSON_STRING: eType = JSON_STRING_VECTOR; break;
			default: break;
			}

		return iSize;
	}

	// save cjson as bson
	bool cJsonToBsonNode ( cJSON * pCJSON, const char * sName = nullptr )
	{
		auto eType = NumericFixup ( pCJSON );

		int iSize = 0; // have sense only for vectors
		if ( eType==JSON_MIXED_VECTOR )
			iSize = MeasureAndOptimizeVector ( pCJSON, eType );

		m_dStorage.Add ( eType );

		if ( sName )
			m_dStorage.PackStr ( sName );

		cJSON * pNode; // used in cJSON_ArrayForEach iterations
		switch ( eType )
		{
			// basic types
		case JSON_INT32: m_dStorage.StoreInt ( ( int ) pCJSON->valueint );	break;
		case JSON_INT64: m_dStorage.StoreBigint ( pCJSON->valueint ); break;
		case JSON_DOUBLE: m_dStorage.StoreBigint ( sphD2QW ( pCJSON->valuedouble ) ); break;
		case JSON_STRING: m_dStorage.PackStr ( pCJSON->valuestring ); break;

			// literals
		case JSON_TRUE:
		case JSON_FALSE:
		case JSON_NULL:
			// no content
			break;

			// associative arrays
		case JSON_OBJECT:
		{
			DWORD uMask = 0;
			int iOfs = m_dStorage.ReserveSize ();
			m_dStorage.StoreInt ( 0 ); // place for mask
			if ( m_bToLowercase )
			{
				cJSON_ArrayForEach( pNode, pCJSON )
				{
					auto iLen = (int) strlen ( pNode->string );
					for ( auto i = 0; i<iLen; ++i )
						pNode->string[i] = Mytolower ( pNode->string[i] );
					uMask |= KeyMask ( pNode->string );
					cJsonToBsonNode ( pNode, pNode->string );
				}
			} else
			{
				cJSON_ArrayForEach( pNode, pCJSON )
				{
					uMask |= KeyMask ( pNode->string );
					cJsonToBsonNode ( pNode, pNode->string );
				}
			}
			m_dStorage.Add ( JSON_EOF );
			m_dStorage.StoreMask ( iOfs + 1, uMask );
			m_dStorage.PackSize ( iOfs ); // MUST be in this order, because PackSize() might move the data!
			break;
		}

			// mixed array
		case JSON_MIXED_VECTOR:
		{
			int iOfs = m_dStorage.ReserveSize ();
			m_dStorage.PackInt ( iSize );
			cJSON_ArrayForEach( pNode, pCJSON )
				cJsonToBsonNode ( pNode );
			m_dStorage.PackSize ( iOfs );
			break;
		}

			// optimized (generic) arrays
		case JSON_INT32_VECTOR: m_dStorage.PackInt ( iSize );
			cJSON_ArrayForEach( pNode, pCJSON )
				m_dStorage.StoreInt ( ( int ) pNode->valueint );
			break;
		case JSON_INT64_VECTOR: m_dStorage.PackInt ( iSize );
			cJSON_ArrayForEach( pNode, pCJSON )
				m_dStorage.StoreBigint ( pNode->valueint );
			break;
		case JSON_DOUBLE_VECTOR: m_dStorage.PackInt ( iSize );
			cJSON_ArrayForEach( pNode, pCJSON )
				m_dStorage.StoreBigint ( sphD2QW ( pNode->valuedouble ) );
			break;
		case JSON_STRING_VECTOR:
		{
			int iOfs = m_dStorage.ReserveSize ();
			m_dStorage.PackInt ( iSize );
			cJSON_ArrayForEach( pNode, pCJSON )
				m_dStorage.PackStr ( pNode->valuestring );
			m_dStorage.PackSize ( iOfs );
			break;
		}
		default: assert ( 0 && "internal error: unhandled type" );
			return false;
		}
		return true;
	}

	// save cjson as bson
	bool cJsonToBson ( cJSON * pCJSON )
	{
		assert ( pCJSON );
		if ( ( pCJSON->type & 0xFF )!=cJSON_Object ) // topmost obj write as 'root'
			return cJsonToBsonNode ( pCJSON );

		DWORD uMask = 0;
		cJSON * pNode;
		if ( m_bToLowercase )
		{
			cJSON_ArrayForEach( pNode, pCJSON )
			{
				auto iLen = (int) strlen ( pNode->string );
				for ( auto i = 0; i<iLen; ++i )
					pNode->string[i] = Mytolower ( pNode->string[i] );
				uMask |= KeyMask ( pNode->string );
				cJsonToBsonNode ( pNode, pNode->string );
			}
		} else
		{
			cJSON_ArrayForEach( pNode, pCJSON )
			{
				uMask |= KeyMask ( pNode->string );
				cJsonToBsonNode ( pNode, pNode->string );
			}
		}
		m_dStorage.Add ( JSON_EOF );
		m_dStorage.StoreMask ( 0, uMask );
		return true;
	}
};

// save cjson as bson
bool bson::JsonObjToBson ( JsonObj_c & tJSON, CSphVector<BYTE> &dData, bool bAutoconv, bool bToLowercase/*, StringBuilder_c &sMsg*/ )
{
	return bson::cJsonToBson ( tJSON.GetRoot(), dData, bAutoconv, bToLowercase );
}

bool bson::cJsonToBson ( cJSON * pCJSON, CSphVector<BYTE> &dData, bool bAutoconv, bool bToLowercase/*, StringBuilder_c &sMsg*/ )
{
	dData.Resize (0);
	CJsonHelper dOut ( dData, bAutoconv, bToLowercase ); // that will write bloom at the beginning
	return dOut.cJsonToBson ( pCJSON );
}


BsonContainer_c::BsonContainer_c ( char * sJson, bool bAutoconv, bool bToLowercase, bool bCheckSize )
{
	sphJsonParse ( m_Bson, sJson, bAutoconv, bToLowercase, bCheckSize, m_sError );
	if ( m_Bson.IsEmpty () )
		return;
	const BYTE * pData = m_Bson.begin ();
	m_dData.second = sphJsonFindFirst ( &pData );
	m_dData.first = pData;
}

BsonContainer2_c::BsonContainer2_c ( const char * sJson, bool bAutoconv, bool bToLowercase )
{
	auto pCjson = cJSON_Parse ( sJson );
	m_Bson.Reserve ( (int) strlen ( sJson ) );
	bson::cJsonToBson ( pCjson, m_Bson, bAutoconv, bToLowercase/*, m_sError*/ );
	if ( pCjson )
		cJSON_Delete ( pCjson );
	if ( m_Bson.IsEmpty () )
		return;
	const BYTE * pData = m_Bson.begin ();
	m_dData.second = sphJsonFindFirst ( &pData );
	m_dData.first = pData;
}

static bool ValidateAsBson ( const char* szJson, CSphString* pError )
{
	CSphString sError;
	if ( !pError )
		pError = &sError;

	CSphVector<BYTE> dBson;
	CSphString sJson ( szJson );
	return sphJsonParse ( dBson, (char*)sJson.cstr(), false, false, false, *pError );
}

static bool ValidateAsCjson ( const char* szJson )
{
	auto pCjson = cJSON_Parse ( szJson );
	if ( !pCjson )
		return false;

	CSphVector<BYTE> dBson;
	dBson.Reserve ( (int)strlen ( szJson ) );
	bson::cJsonToBson ( pCjson, dBson, false, false );
	cJSON_Delete ( pCjson );
	return !dBson.IsEmpty();
}

bool bson::ValidateJson ( const char * szJson, JsonParser_e eParse, CSphString * pError )
{
	switch ( eParse )
	{
	case JsonParser_e::BSON:
		return ValidateAsBson ( szJson, pError );

	case JsonParser_e::CJSON:
		return ValidateAsCjson ( szJson );
	default: break;
	}
	return false;
}

bool bson::ValidateJson ( const char* szJson, CSphString* pError )
{
	bool bRes = ValidateAsBson ( szJson, pError ) && ValidateAsCjson ( szJson );
	return bRes;
}

// unused for now
// bench, then m.b. throw out
/*
static bool sphJsonStringToNumberOld ( const char * s, int iLen, ESphJsonType & eType, int64_t & iVal, double & fVal )
{
	// skip whitespace
	while ( iLen>0 && ( *s==' ' || *s=='\n' || *s=='\r' || *s=='\t' || *s=='\f' ) )
	{
		++s;
		--iLen;
	}

	if ( iLen<=0 )
		return false;

	// check whether the string looks like a numeric
	const char * p = s;
	const char * pEnd = p+iLen-1;
	bool bNumeric = ( *p=='-' || *p=='.' || ( *p>='0' && *p<='9' ) );
	bool bDot = ( *p=='.' );
	bool bExp = false;
	bool bExpSign = false;
	while ( bNumeric && p<pEnd )
	{
		++p;
		switch ( *p )
		{
		case '.':
			if ( bDot )
				bNumeric = false;
			bDot = true;
			break;
		case 'e':
		case 'E':
			if ( bExp )
				bNumeric = false;
			bExp = true;
			break;
		case '-':
		case '+':
			if ( !bExp || bExpSign )
				bNumeric = false;
			bExpSign = true;
			break;
		default:
			if ( *p<'0' || *p >'9' )
				bNumeric = false;
		}
	}

	// convert string to number
	if ( bNumeric && iLen<32 )
	{
		char sVal[32];
		memcpy ( sVal, s, iLen );
		sVal[iLen] = '\0';
		char * pCur;

		// setting errno to zero is necessary because strtod/strtoll do not indicate
		// whether it was an overflow or a valid input for borderline values
		errno = 0;

		if ( bDot || bExp )
		{
			double fRes = strtod ( sVal, &pCur );
			if ( pCur==sVal+iLen && errno!=ERANGE )
			{
				eType = JSON_DOUBLE;
				fVal = fRes;
				return true;
			}

		} else
		{
			int64_t iRes = strtoll ( sVal, &pCur, 10 );
			if ( pCur==sVal+iLen && errno!=ERANGE )
			{
				eType = JSON_INT64;
				iVal = iRes;
				return true;
			}
		}
	}

	return false;
}*/

// stuff below used only for testing/benching

// this one makes unescaped version by memmove in-place
int sphJsonUnescape ( char ** pEscaped, int iLen )
{
	assert ( pEscaped );
	char * s = *pEscaped;

	// skip heading and trailing quotes
	if ( ( s[0]=='\'' && s[iLen - 1]=='\'' ) || ( s[0]=='\"' && s[iLen - 1]=='\"' ) )
	{
		++s;
		iLen -= 2;
	}

	char * sMax = s + iLen;
	char * d = s;
	char * pStart = d;
	char sBuf[8] = { 0 };
	char * sEscape = nullptr;

	while ( s<sMax )
	{
		if ( s[0]=='\\' )
		{
			switch ( s[1] )
			{
			case 'b': *d++ = '\b';
				break;
			case 'n': *d++ = '\n';
				break;
			case 'r': *d++ = '\r';
				break;
			case 't': *d++ = '\t';
				break;
			case 'f': *d++ = '\f';
				break; // formfeed (rfc 4627)
			case 'u':
				// convert 6-byte sequences \u four-hex-digits (rfc 4627) to UTF-8
				if ( s + 6<=sMax && isxdigit ( s[2] ) && isxdigit ( s[3] ) && isxdigit ( s[4] ) && isxdigit ( s[5] ) )
				{
					memcpy ( sBuf, s + 2, 4 );
					d += sphUTF8Encode ( ( BYTE * ) d, ( int ) strtol ( sBuf, NULL, 16 ) );
					s += 4;
				} else
					*d++ = s[1];
				break;
			default: *d++ = s[1];
			}
			s += 2;
			continue;
		}
		sEscape = ( char * ) memchr ( s, '\\', sMax - s );
		if ( !sEscape )
			sEscape = sMax;
		auto iChunk = sEscape - s;
		memmove ( d, s, iChunk );
		d += iChunk;
		s += iChunk;
	}

	*pEscaped = pStart;
	return int ( d - pStart );
}

// this one moves finally nothing, just set point inside quotes and returns correct len
int sphJsonUnescape1 ( char ** pEscaped, int iLen )
{
	assert ( pEscaped );
	char * s = *pEscaped;

	// skip heading and trailing quotes
	if ( ( s[0]=='\'' && s[iLen - 1]=='\'' ) || ( s[0]=='\"' && s[iLen - 1]=='\"' ) )
	{
		++s;
		iLen -= 2;
	}

	char * sMax = s + iLen;
	char * d = s;
	char * pStart = d;
	char sBuf[8] = { 0 };

	while ( s<sMax )
	{
		if ( s[0]=='\\' )
		{
			switch ( s[1] )
			{
			case 'b': *d++ = '\b';
				break;
			case 'n': *d++ = '\n';
				break;
			case 'r': *d++ = '\r';
				break;
			case 't': *d++ = '\t';
				break;
			case 'f': *d++ = '\f';
				break; // formfeed (rfc 4627)
			case 'u':
				// convert 6-byte sequences \u four-hex-digits (rfc 4627) to UTF-8
				if ( s + 6<=sMax && isxdigit ( s[2] ) && isxdigit ( s[3] ) && isxdigit ( s[4] ) && isxdigit ( s[5] ) )
				{
					memcpy ( sBuf, s + 2, 4 );
					d += sphUTF8Encode ( ( BYTE * ) d, ( int ) strtol ( sBuf, NULL, 16 ) );
					s += 4;
				} else
					*d++ = s[1];
				break;
			default: *d++ = s[1];
			}
			s += 2;
		} else
			*d++ = *s++;
	}

	*pEscaped = pStart;
	return int ( d - pStart );
}

// Code below is non-recursive cJsonToBson.
// It is commented out, since it achieve same speed as recursive, but recursive is more plain to understand.
// Moreover, linear in typical usecases need 4K m for lazy vector (always),
// then will acquire more mem from Heap (if necessary),
// but recursive _can_ eat more, but usually eats less (we just never use so deep-nested jsons).
// So, if some hacker pushes us very-very-nested jsons, we can uncomment and roll to this code.
/*
struct BsonContext_t {
	const cJSON * pNext;
	DWORD uMask;
	int iOfs;
	ESphJsonType eType;
};

bool bson::cJsonToBsonLinear ( const cJSON * pNode, CSphVector<BYTE> &dData, StringBuilder_c &sMsg )
{
	dData.Resize ( 0 );
	CJsonHelper dOut ( dData ); // that will write bloom at the beginning
	LazyVector_T<BsonContext_t> dStack;
	BsonContext_t tParent;

	if ( !pNode ) {
		dOut.Add ( JSON_EOF );
		return true;
	}

	if ( ( pNode->type & 0xFF )==cJSON_Object ) // topmost obj write as 'root'
	{
		tParent = { nullptr, 0, 0, JSON_ROOT };
		pNode = pNode->child;
	} else
		tParent.eType = JSON_EOF;

	while (true)
	{
		if ( !pNode ) // current elem is absent or finished
		{
			switch ( tParent.eType )
			{
			case JSON_ROOT: dOut.Add ( JSON_EOF );
				dOut.StoreMask ( 0, tParent.uMask );
				break;

			case JSON_OBJECT: dOut.Add ( JSON_EOF );
				dOut.StoreMask ( tParent.iOfs + 1, tParent.uMask );
				dOut.PackSize ( tParent.iOfs ); // MUST be in this order, because PackSize() might move the data!
				assert ( !dStack.IsEmpty () ); // because topmost object must be root!
				break;

			case JSON_MIXED_VECTOR: dOut.PackSize ( tParent.iOfs );
				break;
			default: assert ( 0 && "internal error: unhandled parent type" );
				return false;
			}

			if ( dStack.IsEmpty () )
				return true;

			tParent = dStack.Pop ();
			pNode = tParent.pNext;
			continue;
		}

		auto eType = NumericFixup ( pNode );

		int iSize = 0; // have sense only for vectors
		if ( eType==JSON_MIXED_VECTOR )
			iSize = MeasureAndOptimizeVector ( pNode, eType );

		dOut.Add ( eType );

		if ( tParent.eType==JSON_OBJECT || tParent.eType==JSON_ROOT )
		{
			auto iLen = (int) strlen ( pNode->string );
			tParent.uMask |= sphJsonKeyMask ( pNode->string, iLen );
			dOut.PackStr ( pNode->string, iLen );
		}

		switch ( eType )
		{
				// basic types
			case JSON_INT32: dOut.StoreInt ( ( int ) pNode->valueint );
				break;
			case JSON_INT64: dOut.StoreBigint ( pNode->valueint );
				break;
			case JSON_DOUBLE: dOut.StoreBigint ( sphD2QW ( pNode->valuedouble ) );
				break;
			case JSON_STRING: dOut.PackStr ( pNode->valuestring );
				break;

				// literals
			case JSON_TRUE:
			case JSON_FALSE:
			case JSON_NULL:
				// no content
				break;

				// associative arrays
			case JSON_OBJECT:
			{
				dStack.Add ( tParent );
				tParent = { pNode->next, 0, dOut.ReserveSize (), JSON_OBJECT };
				dOut.StoreInt ( 0 ); // place for mask
				pNode = pNode->child;
				continue;
			}

				// mixed array
			case JSON_MIXED_VECTOR:
			{
				dStack.Add ( tParent );
				tParent = { pNode->next, 0, dOut.ReserveSize (), JSON_MIXED_VECTOR };
				dOut.PackInt ( iSize );
				pNode = pNode->child;
				continue;
			}

				// optimized (generic) arrays
			case JSON_INT32_VECTOR: dOut.PackInt ( iSize );
				cJSON_ArrayForEach( pNode, pNode )
					dOut.StoreInt ( ( int ) pNode->valueint );
				break;
			case JSON_INT64_VECTOR: dOut.PackInt ( iSize );
				cJSON_ArrayForEach( pNode, pNode )
					dOut.StoreBigint ( pNode->valueint );
				break;
			case JSON_DOUBLE_VECTOR: dOut.PackInt ( iSize );
				cJSON_ArrayForEach( pNode, pNode )
					dOut.StoreBigint ( sphD2QW ( pNode->valuedouble ) );
				break;
			case JSON_STRING_VECTOR:
			{
				int iOfs = dOut.ReserveSize ();
				dOut.PackInt ( iSize );
				cJSON_ArrayForEach( pNode, pNode )
					dOut.PackStr ( pNode->string );
				dOut.PackSize ( iOfs );
				break;
			}
			default: assert ( 0 && "internal error: unhandled type" );
				return false;
		}
		pNode = pNode->next;
	}
}
*/

static const char * g_dTypeNames[JSON_TOTAL] =
{
	"EOF", "INT32", "INT64", "DOUBLE", "STRING", "STRING_VECTOR", "INT32_VECTOR", "INT64_VECTOR", "DOUBLE_VECTOR", "MIXED_VECTOR", "OBJECT", "BOOL", "BOOL", "NULL", "ROOT_OBJECT"
};

const char * JsonTypeName ( ESphJsonType eType )
{
	assert ( eType>=JSON_EOF && eType<JSON_TOTAL );
	return g_dTypeNames[eType];
}

void bson::Assoc_c::StartTypedNode ( ESphJsonType eType, const char * szName )
{
	m_uBloomMask |= KeyMask ( szName );
	m_dBson.Add ( eType );

	if ( szName )
		PackStr ( m_dBson, szName );
}

void bson::Assoc_c::AddInt ( const char * szName, int64_t iValue )
{
	if ( int ( iValue )==iValue )
	{
		StartTypedNode ( JSON_INT32, szName );
		StoreInt ( m_dBson, (int) iValue );
	} else
	{
		StartTypedNode ( JSON_INT64, szName );
		StoreBigint ( m_dBson, iValue );
	}
}

void bson::Assoc_c::AddDouble ( const char * szName, double fValue )
{
	StartTypedNode ( JSON_DOUBLE, szName );
	StoreBigint ( m_dBson, sphD2QW ( fValue ) );
}

void bson::Assoc_c::AddString ( const char * szName, const char * szValue )
{
	StartTypedNode ( JSON_STRING, szName );
	PackStr ( m_dBson, szValue );
}

void bson::Assoc_c::AddBlob ( const char * szName, const ByteBlob_t & dValue )
{
	StartTypedNode ( JSON_STRING, szName );
	PackStr ( m_dBson, (const char *) dValue.first, dValue.second );
}

void bson::Assoc_c::AddBool ( const char * szName, bool bValue )
{
	StartTypedNode ( bValue ? JSON_TRUE : JSON_FALSE, szName );
}

void bson::Assoc_c::AddNull( const char * szName )
{
	StartTypedNode ( JSON_NULL, szName );
}

void bson::Assoc_c::AddStringVec ( const char * szName, const VecTraits_T<CSphString> & dData )
{
	StartTypedNode ( JSON_STRING_VECTOR, szName );
	AddStringData ( m_dBson, dData );
}

void bson::Assoc_c::AddStringVec ( const char * szName, const VecTraits_T<const char *> & dData )
{
	StartTypedNode ( JSON_STRING_VECTOR, szName );
	AddStringData ( m_dBson, dData );
}

void bson::Assoc_c::AddIntVec ( const char * szName, const VecTraits_T<int> & dData )
{
	StartTypedNode ( JSON_INT32_VECTOR, szName );
	AddIntData ( m_dBson, dData );
}

void bson::Assoc_c::AddIntVec ( const char * szName, const VecTraits_T<int64_t> & dData )
{
	if ( dData.any_of ( [] ( int64_t iValue ) { return ( iValue!=int ( iValue ) ); } ) )
	{
		StartTypedNode ( JSON_INT64_VECTOR, szName );
		AddBigintData ( m_dBson, dData );
	} else
	{
		StartTypedNode ( JSON_INT32_VECTOR, szName );
		AddIntData ( m_dBson, dData );
	}
}

void bson::Assoc_c::AddDoubleVec ( const char * szName, const VecTraits_T<double> & dData )
{
	StartTypedNode ( JSON_DOUBLE_VECTOR, szName );
	AddDoubleData ( m_dBson, dData );
}


CSphVector<BYTE> & bson::Assoc_c::StartObj ( const char * szName )
{
	StartTypedNode ( JSON_OBJECT, szName );
	return m_dBson;
}

CSphVector<BYTE> & bson::Assoc_c::StartStringVec ( const char * szName )
{
	StartTypedNode ( JSON_STRING_VECTOR, szName );
	return m_dBson;
}

CSphVector<BYTE> & bson::Assoc_c::StartIntVec ( const char * szName )
{
	StartTypedNode ( JSON_INT32_VECTOR, szName );
	return m_dBson;
}

CSphVector<BYTE> & bson::Assoc_c::StartBigintVec ( const char * szName )
{
	StartTypedNode ( JSON_INT64_VECTOR, szName );
	return m_dBson;
}

CSphVector<BYTE> & bson::Assoc_c::StartDoubleVec ( const char * szName )
{
	StartTypedNode ( JSON_DOUBLE_VECTOR, szName );
	return m_dBson;
}

CSphVector<BYTE> & bson::Assoc_c::StartMixedVec ( const char * szName )
{
	StartTypedNode ( JSON_MIXED_VECTOR, szName );
	return m_dBson;
}

bson::Root_c::Root_c ( CSphVector<BYTE> & dBsonTarget )
	: Assoc_c { dBsonTarget }
{
	StoreInt ( m_dBson, 0 );
}

bson::Root_c::~Root_c ()
{
	Finalize ( m_dBson );
	StoreMask ( m_dBson, 0, m_uBloomMask );
}

bson::Obj_c::Obj_c ( CSphVector<BYTE> & dBsonTarget )
	: Assoc_c { dBsonTarget }
{
	m_iStartPos = ReserveSize ( m_dBson );
	StoreInt ( m_dBson, 0 );
}

bson::Obj_c::~Obj_c ()
{
	Finalize ( m_dBson );
	StoreMask ( m_dBson, m_iStartPos+1, m_uBloomMask );
	PackSize ( m_dBson, m_iStartPos ); // MUST be in this order, because PackSize() might move the data!
}

bson::MixedVector_c::MixedVector_c ( CSphVector<BYTE> & dBson, int iSize )
		: m_dBson ( dBson )
{
	m_iStartPos = ReserveSize ( m_dBson );
	PackInt ( m_dBson, iSize );
};

bson::MixedVector_c::~MixedVector_c()
{
	PackSize ( m_dBson, m_iStartPos );
}

void bson::MixedVector_c::AddInt (  int64_t iValue )
{
	if ( int ( iValue )==iValue )
	{
		m_dBson.Add ( JSON_INT32 );
		StoreInt ( m_dBson, (int) iValue );
	} else
	{
		m_dBson.Add ( JSON_INT64 );
		StoreBigint ( m_dBson, iValue );
	}
}

void bson::MixedVector_c::AddDouble (  double fValue )
{
	m_dBson.Add ( JSON_DOUBLE );
	StoreBigint ( m_dBson, sphD2QW ( fValue ) );
}

void bson::MixedVector_c::AddString ( const char * szValue )
{
	m_dBson.Add ( JSON_STRING );
	PackStr ( m_dBson, szValue );
}

void bson::MixedVector_c::AddBlob ( const ByteBlob_t & dValue )
{
	m_dBson.Add ( JSON_STRING );
	PackStr ( m_dBson, (const char *) dValue.first, dValue.second );
}

void bson::MixedVector_c::AddBool ( bool bValue )
{
	m_dBson.Add ( bValue ? JSON_TRUE : JSON_FALSE );
}

void bson::MixedVector_c::AddNull()
{
	m_dBson.Add ( JSON_NULL );
}

void bson::MixedVector_c::AddStringVec ( const VecTraits_T<CSphString> & dData )
{
	m_dBson.Add ( JSON_STRING_VECTOR );
	AddStringData ( m_dBson, dData );
}

void bson::MixedVector_c::AddStringVec ( const VecTraits_T<const char *> & dData )
{
	m_dBson.Add ( JSON_STRING_VECTOR );
	AddStringData ( m_dBson, dData );
}

void bson::MixedVector_c::AddIntVec ( const VecTraits_T<int> & dData )
{
	m_dBson.Add ( JSON_INT32_VECTOR );
	AddIntData ( m_dBson, dData );
}

void bson::MixedVector_c::AddIntVec ( const VecTraits_T<int64_t> & dData )
{
	if ( dData.any_of ( [] ( int64_t iValue ) { return ( iValue!=int ( iValue ) ); } ) )
	{
		m_dBson.Add ( JSON_INT64_VECTOR );
		AddBigintData ( m_dBson, dData );
	} else
	{
		m_dBson.Add ( JSON_INT32_VECTOR );
		AddIntData ( m_dBson, dData );
	}
}

void bson::MixedVector_c::AddDoubleVec ( const VecTraits_T<double> & dData )
{
	m_dBson.Add ( JSON_DOUBLE_VECTOR );
	AddDoubleData ( m_dBson, dData );
}

CSphVector<BYTE> & bson::MixedVector_c::StartObj ()
{
	m_dBson.Add ( JSON_OBJECT );
	return m_dBson;
}

CSphVector<BYTE> & bson::MixedVector_c::StartStringVec ()
{
	m_dBson.Add ( JSON_STRING_VECTOR );
	return m_dBson;
}

CSphVector<BYTE> & bson::MixedVector_c::StartIntVec ()
{
	m_dBson.Add ( JSON_INT32_VECTOR );
	return m_dBson;
}

CSphVector<BYTE> & bson::MixedVector_c::StartBigintVec ()
{
	m_dBson.Add ( JSON_INT64_VECTOR );
	return m_dBson;
}

CSphVector<BYTE> & bson::MixedVector_c::StartDoubleVec ()
{
	m_dBson.Add ( JSON_DOUBLE_VECTOR );
	return m_dBson;
}

CSphVector<BYTE> & bson::MixedVector_c::StartMixedVec ()
{
	m_dBson.Add ( JSON_MIXED_VECTOR );
	return m_dBson;
}

bson::StringVector_c::StringVector_c ( CSphVector<BYTE> & dBson, int iSize )
		: m_dBson ( dBson )
{
	m_iStartPos = ReserveSize ( m_dBson );
	PackInt ( m_dBson, iSize );
};

bson::StringVector_c::~StringVector_c ()
{
	PackSize ( m_dBson, m_iStartPos );
}

void bson::StringVector_c::AddValue ( const char * szValue )
{
	PackStr ( m_dBson, szValue );
}

void bson::StringVector_c::AddValue ( const ByteBlob_t & dValue )
{
	PackStr ( m_dBson, (const char*) dValue.first, dValue.second );
}

void bson::StringVector_c::AddValues ( const VecTraits_T<CSphString> & dData )
{
	dData.Apply ( [this] ( const CSphString & s ) { PackStr ( m_dBson, s.cstr () ); } );
}

void bson::StringVector_c::AddValues ( const VecTraits_T<const char *> & dData )
{
	dData.Apply ( [this] ( const char * sz ) { PackStr ( m_dBson, sz ); } );
}

bson::IntVector_c::IntVector_c ( CSphVector<BYTE> & dBson, int iSize )
		: m_dBson ( dBson )
{
	PackInt ( m_dBson, iSize );
};

void bson::IntVector_c::AddValue ( int iValue )
{
	StoreInt ( m_dBson, iValue );
}

void bson::IntVector_c::AddValues ( const VecTraits_T<int> & dData )
{
	dData.Apply ( [this] ( int i ) { StoreInt ( m_dBson, i ); } );
}

bson::BigintVector_c::BigintVector_c ( CSphVector<BYTE> & dBson, int iSize )
		: m_dBson ( dBson )
{
	PackInt ( m_dBson, iSize );
};

void bson::BigintVector_c::AddValue ( int64_t iValue )
{
	StoreBigint ( m_dBson, iValue );
}

void bson::BigintVector_c::AddValues ( const VecTraits_T<int64_t> & dData )
{
	dData.Apply ( [this] ( int64_t i ) { StoreBigint ( m_dBson, i ); } );
}

bson::DoubleVector_c::DoubleVector_c ( CSphVector<BYTE> & dBson, int iSize )
		: m_dBson ( dBson )
{
	PackInt ( m_dBson, iSize );
};

void bson::DoubleVector_c::AddValue ( double fValue )
{
	StoreBigint ( m_dBson, sphD2QW ( fValue ) );
}

void bson::DoubleVector_c::AddValues ( const VecTraits_T<double> & dData )
{
	dData.Apply ( [this] ( double f ) { StoreBigint ( m_dBson, sphD2QW ( f ) ); } );
}

