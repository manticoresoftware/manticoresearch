//
// Copyright (c) 2017-2018, Manticore Software LTD (http://manticoresearch.com)
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

#if USE_WINDOWS
#include <io.h> // for isatty() in llsphinxjson.c
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
int JsonUnescape ( char* pTarget, const char * pEscaped, int iLen )
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
			case 'u': s += parseUTF16 ( d, s, sMax ); break;
			default: *d++ = s[1];
			}
			s += 2;
			continue;
		}
		assert ( s<sMax );
		// this snippet replaces a line '*d++=*s++'. On typical string (10% escapes) it provides total time about
		// 50% better then byte-by-byte copying.
		sEscape = ( char * ) memchr ( s, '\\', sMax - s );
		if ( !sEscape )
			sEscape = sMax;
		auto iChunk = sEscape - s;
		memmove ( d, s, iChunk ); // consider also if ((d+iChunk)<s) memcpy (d, s, iChunk); else memmove...
		d += iChunk;
		s += iChunk;
	}
	return d - pTarget;
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
		WIDEST<DWORD, int64_t,double,BlobLocator_t>::T VARIABLE_IS_NOT_USED m_initializer { 0 };
	};
	union {
		BlobLocator_t		m_sName;    		///< node name locator
		WIDEST<int64_t, BlobLocator_t>::T VARIABLE_IS_NOT_USED m_initname {0};
	};
	int 				m_iNext = -1;		///< idx of next sibling
	ESphJsonType m_eType { JSON_TOTAL };	///< node type

	inline int GetLength() const { return m_dChildren.m_iLen; }

};
#define YYSTYPE JsonNode_t

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

	void StoreInt ( int v )
	{
		StoreNUM32LE ( m_dBsonBuffer.AddN ( sizeof ( DWORD ) ), v );
	}

	void StoreBigint ( int64_t v )
	{
		StoreBigintLE ( m_dBsonBuffer.AddN ( sizeof ( int64_t ) ), v );
	}

	inline void Add ( BYTE b )
	{
		m_dBsonBuffer.Add ( b );
	}

	inline void PackStr ( const char * s, int iLen )
	{
		assert ( iLen<=0x00FFFFFFFF );
		iLen = Min ( iLen, 0x00ffffff );
		PackInt ( iLen );
		auto * pOut = m_dBsonBuffer.AddN ( iLen );
		if ( iLen )
			memmove ( pOut, s, iLen );
	}

	inline void PackStr ( const char * s )
	{
		PackStr ( s, strlen(s));
	}

	void PackInt ( DWORD v )
	{
//		assert ( v<16777216 ); // strings over 16M bytes and arrays over 16M entries are not supported
		if ( v<0x000000FC )
		{
			m_dBsonBuffer.Add ( BYTE ( v ) );
		} else if ( v<0x00010000 )
		{
			m_dBsonBuffer.Add ( 252 );
			m_dBsonBuffer.Add ( BYTE ( v & 255 ) );
			m_dBsonBuffer.Add ( BYTE ( v >> 8 ) );
		} else if ( v<0x1000000 )
		{
			m_dBsonBuffer.Add ( 253 );
			m_dBsonBuffer.Add ( BYTE ( v & 255 ) );
			m_dBsonBuffer.Add ( BYTE ( ( v >> 8 ) & 255 ) );
			m_dBsonBuffer.Add ( BYTE ( v >> 16 ) );
		} else
		{
			m_dBsonBuffer.Add ( 254 );
			StoreInt ( v );
		}
	}

	BYTE * PackStrUnescaped ( const char * s, int iLen )
	{
		iLen = Min ( iLen, 0x00ffffff );
		auto iLenSize = PackedLen ( iLen );
		int iOfs = m_dBsonBuffer.GetLength ();
		auto * pOut = m_dBsonBuffer.AddN ( iLen + iLenSize ) + iLenSize;
		if ( iLen )
			iLen = JsonUnescape ( ( char * ) pOut, s, iLen );

		iLenSize -= PackedLen ( iLen ); // difference between place for original and unescaped len
		if ( iLenSize )
		{
			assert ( iLenSize>0 && "Strange, unescaped json need more place than escaped" );
			memmove ( pOut - iLenSize, pOut, iLen );
		}
		m_dBsonBuffer.Resize ( iOfs );
		PackInt ( iLen );
		return m_dBsonBuffer.AddN ( iLen );
	}

	inline DWORD JsonKeyMask ( const BlobLocator_t &sKey )
	{
		return sphJsonKeyMask ( ( const char * ) &m_dBsonBuffer[sKey.m_iStart], sKey.m_iLen );
	}

	void StoreMask ( int iOfs, DWORD uMask )
	{
#if UNALIGNED_RAM_ACCESS && USE_LITTLE_ENDIAN
		*( DWORD * ) ( m_dBsonBuffer.begin () + iOfs ) = uMask;
#else
		for ( int i=0; i<4; ++i )
		{
			m_dBsonBuffer[iOfs+i] = BYTE ( uMask & 0xff );
			uMask >>= 8;
		}
#endif

	}

	/// reserve a single byte for a yet-unknown length, to be written later with PackSize()
	/// returns its offset, to be used by PackSize() to both calculate and stored the length
	int ReserveSize ()
	{
		int iOfs = m_dBsonBuffer.GetLength ();
		m_dBsonBuffer.Resize ( iOfs + 1 );
		return iOfs;
	}

	/// compute current length from the offset reserved with ReserveSize(), and pack the value back there
	/// in most cases that single byte is enough; if not, we make room by memmove()ing the data
	void PackSize ( int iOfs )
	{
		int iSize = m_dBsonBuffer.GetLength () - iOfs - 1;
		int iPackLen = PackedLen ( iSize );

		if ( iPackLen!=1 )
		{
			m_dBsonBuffer.Resize ( iOfs + iPackLen + iSize );
			memmove ( m_dBsonBuffer.Begin () + iOfs + iPackLen, m_dBsonBuffer.Begin () + iOfs + 1, iSize );
		}

		m_dBsonBuffer.Resize ( iOfs );
		PackInt ( iSize );
		m_dBsonBuffer.AddN ( iSize );
	}

	void Finalize ()
	{
		m_dBsonBuffer.Add ( JSON_EOF );
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
			printf ( "%s (bloom mask: 0x%08x)\n", eType==JSON_OBJECT ? "JSON_OBJECT" : "JSON_ROOT", uMask );
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

	void DebugDump ( const BYTE * p )
	{
		JsonEscapedBuilder dOut;
		sphJsonFormat ( dOut, m_dBsonBuffer.Begin () );
		printf ( "sphJsonFormat: %s\n", dOut.cstr () );
		printf ( "Blob size: %d bytes\n", m_dBsonBuffer.GetLength () );
		ESphJsonType eType = sphJsonFindFirst ( &p );
		DebugDump ( eType, &p, 0 );
		printf ( "\n" );
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
			WriteNode ( dNode );
			uMask |= JsonKeyMask ( dNode.m_sName );
		}
		m_dBsonBuffer.Add ( JSON_EOF );
		StoreMask ( 0, uMask );
		return true;
	}

	// main proc which do whole magic over the topmost obj/array
	bool WriteNode ( JsonNode_t &tNode )
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
		if ( tNode.m_sName.m_iLen )
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
				WriteNode ( dNode );
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
};

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_NO_UNISTD_H 1
#define YY_DECL static int my_lex ( YYSTYPE * lvalp, void * yyscanner, JsonParser_c * pParser )

#ifdef CMAKE_GENERATED_LEXER

#ifdef __GNUC__
	#pragma GCC diagnostic push 
	#pragma GCC diagnostic ignored "-Wsign-compare"
	#pragma GCC diagnostic ignored "-Wpragmas"
	#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
#endif

#include "flexsphinxjson.c"

#ifdef __GNUC__
	#pragma GCC diagnostic pop
#endif

#else
	#include "llsphinxjson.c"
#endif

void yyerror ( JsonParser_c * pParser, const char * sMessage )
{
	yy2lex_unhold ( pParser->m_pScanner );
	pParser->m_sError.Sprintf ( "%s near '%s'", sMessage, pParser->m_pLastToken );
}

static int yylex ( YYSTYPE * lvalp, JsonParser_c * pParser )
{
	return my_lex ( lvalp, pParser->m_pScanner, pParser );
}

#ifdef CMAKE_GENERATED_GRAMMAR
	#include "bissphinxjson.c"
#include "sphinxutils.h"

#else
	#include "yysphinxjson.c"
#endif

bool sphJsonParse ( CSphVector<BYTE> & dData, char * sData, bool bAutoconv, bool bToLowercase, CSphString & sError )
{
	StringBuilder_c sMsg;
	auto bResult = sphJsonParse ( dData, sData, bAutoconv, bToLowercase, sMsg );
	sMsg.MoveTo ( sError );
	return bResult;
}

bool sphJsonParse ( CSphVector<BYTE> &dData, char * sData, bool bAutoconv, bool bToLowercase, StringBuilder_c &sMsg )
{
	int iLen = strlen ( sData );
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

	if ( dData.AllocatedBytes () >= 0x400000 )
	{
		sMsg << "data exceeds 0x400000 bytes";
		iRes = -1;
	}

	if ( iRes!=0 )
		dData.Reset();

	return iRes==0;
}

//////////////////////////////////////////////////////////////////////////

DWORD sphJsonKeyMask ( const char * sKey, int iLen )
{
	DWORD uCrc = sphCRC32 ( sKey, iLen );
	return
		( 1UL<<( uCrc & 31 ) ) +
		( 1UL<<( ( uCrc>>8 ) & 31 ) );
}

static inline DWORD KeyMask ( const char * s )
{
	return sphJsonKeyMask ( s, strlen ( s ) );
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
		return p - pData + iLen * 4;
	case JSON_INT64_VECTOR:
	case JSON_DOUBLE_VECTOR:
		if ( !p )
			return -1;
		iLen = sphJsonUnpackInt ( &p );
		return p - pData + iLen * 8;
	case JSON_STRING:
	case JSON_STRING_VECTOR:
	case JSON_MIXED_VECTOR:
	case JSON_OBJECT:
		if ( !p )
			return -1;
		iLen = sphJsonUnpackInt ( &p );
		return p - pData + iLen;
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
		return p - pData;
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
		break;
	}
}

//////////////////////////////////////////////////////////////////////////

static const BYTE * JsonFormatStr ( JsonEscapedBuilder &sOut, const BYTE * p, bool bQuote=true )
{
	int iLen = sphJsonUnpackInt ( &p );
	sOut.GrowEnough ( iLen );
	if ( bQuote )
		sOut.AppendEscaped ( ( const char * ) p, EscBld::eEscape, iLen );
	else
		sOut.AppendChars ( ( const char * ) p, iLen );
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
			sOut.AppendChars ( sDouble, iLen );
			break;
		}
		case JSON_STRING:
			p = JsonFormatStr ( sOut, p, bQuoteString );
			break;

		case JSON_STRING_VECTOR:
			sOut.GrowEnough ( sphJsonUnpackInt ( &p ) );
			sOut.StartBlock ( ",", "[", "]");
			for ( int i= sphJsonUnpackInt ( &p ); i>0; --i )
				p = JsonFormatStr ( sOut, p );
			sOut.FinishBlock ( false );
			break;

		case JSON_INT32_VECTOR:
			sOut.StartBlock ( ",", "[", "]" );
			for ( int i = sphJsonUnpackInt ( &p ); i>0; --i )
				sOut.Sprintf ( "%d", sphJsonLoadInt ( &p ) );
			sOut.FinishBlock ( false );
		break;
		case JSON_INT64_VECTOR:
			sOut.StartBlock ( ",", "[", "]" );
			for ( int i = sphJsonUnpackInt ( &p ); i>0; --i )
				sOut.Sprintf ( "%l", sphJsonLoadBigint ( &p ) );
			sOut.FinishBlock ( false );
			break;
		case JSON_DOUBLE_VECTOR:
			sOut.StartBlock ( ",", "[", "]" );
			for ( int i = sphJsonUnpackInt ( &p ); i>0; --i )
			{
				auto iLen = snprintf ( sDouble, szDouble, "%lf", sphQW2D ( sphJsonLoadBigint ( &p ) ) ); // NOLINT
				sOut.AppendChars ( sDouble, iLen );
			}
			sOut.FinishBlock ( false );
			break;

		case JSON_MIXED_VECTOR:
			{
				sOut.StartBlock ( ",", "[", "]" );
				sphJsonUnpackInt ( &p );
				for ( int i = sphJsonUnpackInt ( &p ); i>0; --i )
				{
					auto eNode = ( ESphJsonType ) *p++;
					p = sphJsonFieldFormat ( sOut, p, eNode, true );
				}
				sOut.FinishBlock ( false );
				break;
			}
		case JSON_ROOT:
		case JSON_OBJECT:
			{
				if ( eType==JSON_OBJECT )
					sphJsonUnpackInt ( &p );
				p += 4; // skip bloom table
				sOut.StartBlock ( ",", "{", "}" );
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


bool sphJsonNameSplit ( const char * sName, CSphString * sColumn, CSphString * sKey )
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

	int iSep = pSep - sName;
	if ( sColumn )
	{
		sColumn->SetBinary ( sName, iSep );
		sColumn->Trim();
	}

	if ( sKey )
		*sKey = sName + iSep + ( *pSep=='.' ? 1 : 0 );

	return true;
}

JsonKey_t::JsonKey_t ( const char * sKey, int iLen )
{
	m_iLen = iLen;
	m_uMask = sphJsonKeyMask ( sKey, m_iLen );
	m_sKey.SetBinary ( sKey, m_iLen );
}

bool sphJsonInplaceUpdate ( ESphJsonType eValueType, int64_t iValue, ISphExpr * pExpr, BYTE * pStrings, const CSphRowitem * pRow, bool bUpdate )
{
	if ( !pExpr || !pStrings )
		return false;

	pExpr->Command ( SPH_EXPR_SET_STRING_POOL, (void*)pStrings );

	CSphMatch tMatch;
	tMatch.m_pStatic = pRow;

	auto uPacked = (uint64_t)pExpr->Int64Eval ( tMatch );
	BYTE * pData = pStrings + ( uPacked & 0xffffffff );
	auto eType = (ESphJsonType)( uPacked >> 32 );

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

	iLen = pVal-sVal;
	if ( !iLen || iLen==64 )
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

bool bson::Bool ( const NodeHandle_t &tLocator )
{
	switch ( tLocator.second )
	{
	case JSON_INT32:
	case JSON_INT64: return !!Int ( tLocator );
	case JSON_DOUBLE: return 0.0!=Double ( tLocator );
	case JSON_TRUE: return true;
		// fixme! Do we need also process here values like "True" (the string)?
	default: break;
	}
	return false;
}

int64_t bson::Int ( const NodeHandle_t &tLocator )
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
	return 0;
}

double bson::Double ( const NodeHandle_t &tLocator )
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
	return 0.0;
}

CSphString bson::String ( const NodeHandle_t &tLocator )
{
	if ( tLocator.second!=JSON_STRING )
		return "";

	auto dBlob = bson::RawBlob ( tLocator );
	CSphString sResult;
	sResult.SetBinary ( dBlob.first, dBlob.second );
	return sResult;
}

void bson::ForEach ( const NodeHandle_t &tLocator, Action_f fAction )
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
	}
	case JSON_OBJECT: sphJsonUnpackInt ( &p );
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
	default: break;
	}
}

void bson::ForEach ( const NodeHandle_t &tLocator, NamedAction_f fAction )
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
	}

	case JSON_OBJECT: sphJsonUnpackInt ( &p );
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
			fAction ( sName, { p, eType } );
			sphJsonSkipNode ( eType, &p );
		}
	}
	default: break;
	}
}

void bson::ForSome ( const NodeHandle_t &tLocator, CondAction_f fAction )
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
	}
	case JSON_OBJECT: sphJsonUnpackInt ( &p );
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
	default: break;
	}
}

void bson::ForSome ( const NodeHandle_t &tLocator, CondNamedAction_f fAction )
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
	}

	case JSON_OBJECT: sphJsonUnpackInt ( &p );
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
			if ( !fAction ( sName, { p, eType } ) )
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
	int iLen = strlen ( sValue );
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

	int iLen = strlen ( sName );

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
	m_sError << "No such member " << sName;
	return nullnode;
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
		int iLen = strlen ( sName );
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
			int iLen = strlen ( sName );
			if ( iStrLen==iLen && !memcmp ( p, sName, iStrLen ) )
				return true;
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

bool Bson_c::BsonToJson ( CSphString &sResult ) const
{
	JsonEscapedBuilder sBuilder;
	if ( !m_dData.first )
		return false;

	// check for the empty root
	if ( m_dData.second==JSON_EOF )
		sBuilder << "{}";
	else
		sphJsonFieldFormat ( sBuilder, m_dData.first, m_dData.second );

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
			if ( !sphJsonStringToNumber ( pNode->valuestring, strlen ( pNode->valuestring ), eOrigType, iFoo, pNode->valuedouble ) )
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
		ESphJsonType eOutType;
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
					int iLen = strlen ( pNode->string );
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
				int iLen = strlen ( pNode->string );
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
bool bson::cJsonToBson ( cJSON * pCJSON, CSphVector<BYTE> &dData, bool bAutoconv, bool bToLowercase/*, StringBuilder_c &sMsg*/ )
{
	dData.Resize (0);
	CJsonHelper dOut ( dData, bAutoconv, bToLowercase ); // that will write bloom at the beginning
	return dOut.cJsonToBson ( pCJSON );
}


BsonContainer_c::BsonContainer_c ( char * sJson, bool bAutoconv, bool bToLowercase )
{
	sphJsonParse ( m_Bson, sJson, bAutoconv, bToLowercase, m_sError );
	if ( m_Bson.IsEmpty () )
		return;
	const BYTE * pData = m_Bson.begin ();
	m_dData.second = sphJsonFindFirst ( &pData );
	m_dData.first = pData;
}

BsonContainer2_c::BsonContainer2_c ( const char * sJson, bool bAutoconv, bool bToLowercase )
{
	auto pCjson = cJSON_Parse ( sJson );
	m_Bson.Reserve ( strlen ( sJson ) );
	bson::cJsonToBson ( pCjson, m_Bson, bAutoconv, bToLowercase/*, m_sError*/ );
	if ( pCjson )
		cJSON_Delete ( pCjson );
	if ( m_Bson.IsEmpty () )
		return;
	const BYTE * pData = m_Bson.begin ();
	m_dData.second = sphJsonFindFirst ( &pData );
	m_dData.first = pData;
}

// bench, then m.b. throw out
bool sphJsonStringToNumberOld ( const char * s, int iLen, ESphJsonType & eType, int64_t & iVal, double & fVal )
{
	// skip whitespace
	while ( iLen>0 && ( *s==' ' || *s=='\n' || *s=='\r' || *s=='\t' || *s=='\f' ) )
		++s, --iLen;

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
}

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
	return d - pStart;
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
	return d - pStart;
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