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

#include "sphinxjson.h"
#include "sphinxint.h"

#if USE_WINDOWS
#include <io.h> // for isatty() in llsphinxjson.c
#endif

//////////////////////////////////////////////////////////////////////////

/// parser view on a generic node
struct JsonNode_t
{
	ESphJsonType	m_eType;		///< node type
	int64_t			m_iValue;		///< integer value, only used for JSON_INT32 and JSON_INT64
	double			m_fValue;		///< floating point value, only used for JSON_DOUBLE
	int				m_iStart;		///< string value, start index (inclusive) into m_pBuf, only used for JSON_STRING
	int				m_iEnd;			///< string value, end index (exclusive) into m_pBuf, only used for JSON_STRING
	int				m_iHandle;		///< subobject value, index into m_dNodes storage
	int				m_iKeyStart;	///< node name, start index (inclusive) into m_pBuf
	int				m_iKeyEnd;		///< node name, end index (exclusive) into m_pBuf

	JsonNode_t ()
	{}
};
#define YYSTYPE JsonNode_t

// must be included after YYSTYPE declaration
#include "yysphinxjson.h"

/// actually, JSON-to-SphinxBSON converter helper, but who cares
class JsonParser_c : ISphNoncopyable
{
public:
	void *				m_pScanner;
	const char *		m_pLastToken;
	CSphVector<BYTE> &	m_dBuffer;
	CSphString &		m_sError;
	bool				m_bAutoconv;
	bool				m_bToLowercase;
	char *				m_pBuf;
	CSphVector < CSphVector<JsonNode_t> >	m_dNodes;
	CSphVector<JsonNode_t>					m_dEmpty;

public:
	JsonParser_c ( CSphVector<BYTE> & dBuffer, bool bAutoconv, bool bToLowercase, CSphString & sError )
		: m_pScanner ( NULL )
		, m_pLastToken ( NULL )
		, m_dBuffer ( dBuffer )
		, m_sError ( sError )
		, m_bAutoconv ( bAutoconv )
		, m_bToLowercase ( bToLowercase )
	{
		// reserve 4 bytes for Bloom mask
		StoreInt ( 0 );
	}

protected:
	BYTE * BufAlloc ( int iLen )
	{
		int iPos = m_dBuffer.GetLength();
		m_dBuffer.Resize ( m_dBuffer.GetLength()+iLen );
		return m_dBuffer.Begin()+iPos;
	}

	void StoreInt ( int v )
	{
		BYTE * p = BufAlloc ( 4 );
		*p++ = BYTE(DWORD(v));
		*p++ = BYTE(DWORD(v) >> 8);
		*p++ = BYTE(DWORD(v) >> 16);
		*p++ = BYTE(DWORD(v) >> 24);
	}

	void StoreBigint ( int64_t v )
	{
		StoreInt ( (DWORD)( v & 0xffffffffUL ) );
		StoreInt ( (int)( v>>32 ) );
	}

	int PackLen ( DWORD v )
	{
		if ( v<=251 )
			return 1;
		else if ( v<65536 )
			return 3;
		else if ( v<16777216 )
			return 4;
		else
			return 5;
	}

	void PackInt ( DWORD v )
	{
		assert ( v<16777216 ); // strings over 16M bytes and arrays over 16M entries are not supported
		if ( v<252 )
		{
			m_dBuffer.Add ( BYTE(v) );
		} else if ( v<65536 )
		{
			m_dBuffer.Add ( 252 );
			m_dBuffer.Add ( BYTE ( v & 255 ) );
			m_dBuffer.Add ( BYTE ( v>>8 ) );
		} else
		{
			m_dBuffer.Add ( 253 );
			m_dBuffer.Add ( BYTE ( v & 255 ) );
			m_dBuffer.Add ( BYTE ( ( v>>8 ) & 255 ) );
			m_dBuffer.Add ( BYTE ( v>>16 ) );
		}
	}

	void PackStr ( const char * s, int iLen )
	{
		iLen = Min ( iLen, 0xffffff );
		PackInt ( iLen );
		if ( iLen )
		{
			BYTE * p = BufAlloc ( iLen );
			memcpy ( p, s, iLen );
		}
	}

	int JsonUnescape ( char ** pEscaped, int iLen )
	{
		assert ( pEscaped );
		char * s = *pEscaped;

		// skip heading and trailing quotes
		if ( ( s[0]=='\'' && s[iLen-1]=='\'' ) || ( s[0]=='"' && s[iLen-1]=='"' ) )
		{
			s++;
			iLen -= 2;
		}

		char * sMax = s+iLen;
		char * d = s;
		char * pStart = d;
		char sBuf[8] = { 0 };

		while ( s<sMax )
		{
			if ( s[0]=='\\' )
			{
				switch ( s[1] )
				{
				case 'b': *d++ = '\b'; break;
				case 'n': *d++ = '\n'; break;
				case 'r': *d++ = '\r'; break;
				case 't': *d++ = '\t'; break;
				case 'f': *d++ = '\f'; break; // formfeed (rfc 4627)
				case 'u':
					// convert 6-byte sequences \u four-hex-digits (rfc 4627) to UTF-8
					if ( s+6<=sMax && isxdigit ( s[2] ) && isxdigit ( s[3] ) && isxdigit ( s[4] ) && isxdigit ( s[5] ) )
					{
						memcpy ( sBuf, s+2, 4 );
						d += sphUTF8Encode ( (BYTE*)d, (int)strtol ( sBuf, NULL, 16 ) );
						s += 4;
					} else
						*d++ = s[1];
					break;
				default:
					*d++ = s[1];
				}
				s += 2;
			} else
				*d++ = *s++;
		}

		*pEscaped = pStart;
		return d - pStart;
	}

	void PackNodeStr ( const JsonNode_t & tNode )
	{
		int iLen = tNode.m_iEnd-tNode.m_iStart;
		char *s = m_pBuf + tNode.m_iStart;
		iLen = JsonUnescape ( &s, iLen );
		PackStr ( s, iLen );
	}

	int KeyUnescape ( char ** ppKey, int iLen )
	{
		char * s = *ppKey;
		iLen = JsonUnescape ( &s, iLen );
		if ( m_bToLowercase )
			for ( int i=0; i<iLen; i++ )
				s[i] = (char)tolower ( s[i] ); // OPTIMIZE! not sure if significant, but known to be hell slow
		*ppKey = s;
		return iLen;
	}

	void StoreMask ( int iOfs, DWORD uMask )
	{
		for ( int i=0; i<4; i++ )
		{
			m_dBuffer[iOfs+i] = BYTE ( uMask & 0xff );
			uMask >>= 8;
		}
	}

	/// reserve a single byte for a yet-unknown length, to be written later with PackSize()
	/// returns its offset, to be used by PackSize() to both calculate and stored the length
	int ReserveSize()
	{
		int iOfs = m_dBuffer.GetLength();
		m_dBuffer.Resize ( iOfs+1 );
		return iOfs;
	}

	/// compute current length from the offset reserved with ReserveSize(), and pack the value back there
	/// in most cases that single byte is enough; if not, we make room by memmove()ing the data
	void PackSize ( int iOfs )
	{
		int iSize = m_dBuffer.GetLength()-iOfs-1;
		int iPackLen = PackLen ( iSize );

		if ( iPackLen!=1 )
		{
			BYTE * pOfs = m_dBuffer.Begin()+iOfs;
			memmove ( pOfs+iPackLen, pOfs+1, iSize );
		}

		m_dBuffer.Resize ( iOfs );
		PackInt ( iSize );
		m_dBuffer.Resize ( iOfs+iPackLen+iSize );
	}

public:
	void Finalize()
	{
		m_dBuffer.Add ( JSON_EOF );
	}

	void NumericFixup ( JsonNode_t & tNode, bool bAutoconv )
	{
		// parser emits int64 values, fixup them to int32
		if ( tNode.m_eType==JSON_INT64 )
		{
			int iVal = int(tNode.m_iValue);
			if ( tNode.m_iValue==int64_t(iVal) )
				tNode.m_eType = JSON_INT32;
			return;
		}

		// check for the autoconversion of string values
		if ( !bAutoconv || tNode.m_eType!=JSON_STRING )
			return;

		// check whether the (quoted) string looks like a numeric
		int iLen = tNode.m_iEnd - tNode.m_iStart - 2;
		if ( iLen<=0 || iLen>=32 )
			return;
		const char * sValue = m_pBuf + tNode.m_iStart+1;
		const char * p = sValue;
		const char * pEnd = p+iLen-1;

		bool bNumeric = ( *p=='-' || *p=='.' || ( *p>='0' && *p<='9' ) );
		bool bDot = ( *p=='.' );
		while ( bNumeric && p<pEnd )
		{
			p++;
			if ( *p=='.' )
			{
				if ( bDot )
					bNumeric = false;
				bDot = true;
			} else
			{
				if ( *p<'0' || *p >'9' )
					bNumeric = false;
			}
		}
		if ( !bNumeric )
			return;

		// ok, looks numeric, try integer conversion
		// OPTIMIZE?
		char sVal[32];
		memcpy ( sVal, sValue, iLen );
		sVal[iLen] = '\0';

		if ( !bDot )
		{
			int64_t iVal = strtoll ( sVal, NULL, 10 );
			snprintf ( sVal, sizeof(sVal), INT64_FMT, iVal );
			if ( !memcmp ( sValue, sVal, iLen ) )
			{
				tNode.m_eType = int64_t(int(iVal))==iVal ? JSON_INT32 : JSON_INT64;
				tNode.m_iValue = iVal;
			}
		} else
		{
			tNode.m_eType = JSON_DOUBLE;
			tNode.m_fValue = strtod ( sVal, NULL );
		}
	}

	void WriteNode ( JsonNode_t & tNode, const char * sKey=NULL, int iKeyLen=0 )
	{
		// convert int64 to int32, strings to numbers if needed
		NumericFixup ( tNode, m_bAutoconv );

		ESphJsonType eType = tNode.m_eType;

		// note m_iHandle may be uninitialized on simple nodes
		CSphVector<JsonNode_t> & dNodes = ( ( eType==JSON_MIXED_VECTOR || eType==JSON_OBJECT ) && tNode.m_iHandle>=0 )
			? m_dNodes[ tNode.m_iHandle ]
			: m_dEmpty;

		// process mixed vector, convert to generic vector if possible
		if ( eType==JSON_MIXED_VECTOR )
		{
			ARRAY_FOREACH ( i, dNodes )
				NumericFixup ( dNodes[i], m_bAutoconv );

			ESphJsonType eBase = dNodes.GetLength()>0 ? dNodes[0].m_eType : JSON_EOF;
			bool bGeneric = ARRAY_ALL ( bGeneric, dNodes, dNodes[_all].m_eType==eBase );

			if ( bGeneric )
				switch ( eBase )
			{
				case JSON_INT32:	eType = JSON_INT32_VECTOR; break;
				case JSON_INT64:	eType = JSON_INT64_VECTOR; break;
				case JSON_DOUBLE:	eType = JSON_DOUBLE_VECTOR; break;
				case JSON_STRING:	eType = JSON_STRING_VECTOR; break;
				default:			break; // type matches across all entries, but we do not have a special format for that type
			}
		}

		// check for the root (bson v1), note sKey shouldn't be set
		if ( eType==JSON_OBJECT && m_dBuffer.GetLength()==4 && !sKey )
			eType = JSON_ROOT;

		// write node type
		if ( eType!=JSON_ROOT )
			m_dBuffer.Add ( (BYTE)eType );

		// write key if given
		if ( sKey )
			PackStr ( sKey, iKeyLen );

		switch ( eType )
		{
		// basic types
		case JSON_INT32:	StoreInt ( (int)tNode.m_iValue ); break;
		case JSON_INT64:	StoreBigint ( tNode.m_iValue ); break;
		case JSON_DOUBLE:	StoreBigint ( sphD2QW ( tNode.m_fValue ) ); break;
		case JSON_STRING:	PackNodeStr ( tNode ); break;

		// literals
		case JSON_TRUE:
		case JSON_FALSE:
		case JSON_NULL:
			// no content
			break;

		// associative arrays
		case JSON_ROOT:
		case JSON_OBJECT:
			{
				DWORD uMask = 0;
				int iOfs = 0;

				if ( eType==JSON_OBJECT )
				{
					iOfs = ReserveSize();
					StoreInt ( uMask );
				}

				ARRAY_FOREACH ( i, dNodes )
				{
					char * sKey = m_pBuf + dNodes[i].m_iKeyStart;
					int iLen = KeyUnescape ( &sKey, dNodes[i].m_iKeyEnd-dNodes[i].m_iKeyStart );
					WriteNode ( dNodes[i], sKey, iLen );
					uMask |= sphJsonKeyMask ( sKey, iLen );
				}
				m_dBuffer.Add ( JSON_EOF );

				if ( eType==JSON_OBJECT )
				{
					StoreMask ( iOfs+1, uMask );
					PackSize ( iOfs ); // MUST be in this order, because PackSize() might move the data!
				} else
				{
					assert ( eType==JSON_ROOT );
					StoreMask ( 0, uMask );
				}
				break;
			}

		// mixed array
		case JSON_MIXED_VECTOR:
			{
				int iOfs = ReserveSize();
				PackInt ( dNodes.GetLength() );
				ARRAY_FOREACH ( i, dNodes )
					WriteNode ( dNodes[i] );
				PackSize ( iOfs );
				break;
			}

		// optimized (generic) arrays
		case JSON_INT32_VECTOR:
			PackInt ( dNodes.GetLength() );
			ARRAY_FOREACH ( i, dNodes )
				StoreInt ( (int)dNodes[i].m_iValue );
			break;
		case JSON_INT64_VECTOR:
			PackInt ( dNodes.GetLength() );
			ARRAY_FOREACH ( i, dNodes )
				StoreBigint ( dNodes[i].m_iValue );
			break;
		case JSON_DOUBLE_VECTOR:
			PackInt ( dNodes.GetLength() );
			ARRAY_FOREACH ( i, dNodes )
				StoreBigint ( sphD2QW ( dNodes[i].m_fValue ) );
			break;
		case JSON_STRING_VECTOR:
			{
				int iOfs = ReserveSize();
				PackInt ( dNodes.GetLength() );
				ARRAY_FOREACH ( i, dNodes )
					PackNodeStr ( dNodes[i] );
				PackSize ( iOfs );
				break;
			}
		default:
			assert ( 0 && "internal error: unhandled type" );
			break;
		}
	}

	void DebugIndent ( int iLevel )
	{
		for ( int i=0; i<iLevel; i++ )
			printf ( "    " );
	}

	void DebugDump ( ESphJsonType eType, const BYTE ** ppData, int iLevel )
	{
		DebugIndent ( iLevel );

		const BYTE * p = *ppData;

		switch ( eType )
		{
		case JSON_INT32: printf ( "JSON_INT32 %d\n", sphJsonLoadInt ( &p ) ); break;
		case JSON_INT64: printf ( "JSON_INT64 "INT64_FMT"\n", sphJsonLoadBigint ( &p ) ); break;
		case JSON_DOUBLE: printf ( "JSON_DOUBLE %lf\n", sphQW2D ( sphJsonLoadBigint ( &p ) ) ); break;
		case JSON_STRING:
			{
				int iLen = sphJsonUnpackInt ( &p );
				CSphString sVal;
				sVal.SetBinary ( (const char*)p, iLen );
				printf ( "JSON_STRING \"%s\"\n", sVal.cstr() );
				p += iLen;
				break;
			}

		case JSON_TRUE:		printf ( "JSON_TRUE\n" ); break;
		case JSON_FALSE:	printf ( "JSON_FALSE\n" ); break;
		case JSON_NULL:		printf ( "JSON_NULL\n" ); break;
		case JSON_EOF:		printf ( "JSON_EOF\n" ); break;

		// associative arrays
		case JSON_ROOT:
		case JSON_OBJECT:
			{
				if ( eType==JSON_OBJECT )
					sphJsonUnpackInt ( &p );

				DWORD uMask = p[0] + ( p[1]<<8 ) + ( p[2]<<16 ) + ( p[3]<<24 );
				printf ( "%s (bloom mask: 0x%08x)\n", eType==JSON_OBJECT ? "JSON_OBJECT" : "JSON_ROOT", uMask );
				p += 4; // skip bloom table
				for ( ;; )
				{
					ESphJsonType eType = (ESphJsonType) *p++;
					if ( eType==JSON_EOF )
						break;
					const int iStrLen = sphJsonUnpackInt ( &p );
					CSphString sVal;
					sVal.SetBinary ( (const char*)p, iStrLen );
					DebugIndent ( iLevel+1 );
					printf ( "\"%s\"", sVal.cstr() );
					p += iStrLen;
					DebugDump ( eType, &p, iLevel+1 );
				}
				break;
			}

		case JSON_MIXED_VECTOR:
			{
				int iTotalLen = sphJsonUnpackInt ( &p );
				int iLen = sphJsonUnpackInt ( &p );
				printf ( "JSON_MIXED_VECTOR [%d] (%d bytes)\n", iLen, iTotalLen );
				for ( int i=0; i<iLen; i++ )
				{
					ESphJsonType eType = (ESphJsonType)*p++;
					DebugDump ( eType, &p, iLevel+1 );
				}
				break;
			}

		// optimized arrays ( note they can't be empty )
		case JSON_STRING_VECTOR:
			{
				sphJsonUnpackInt ( &p );
				int iLen = sphJsonUnpackInt ( &p );
				printf ( "JSON_STRING_VECTOR (%d) [", iLen );
				for ( int i=0; i<iLen; i++ )
				{
					int iStrLen = sphJsonUnpackInt ( &p );
					CSphString sVal;
					sVal.SetBinary ( (const char*)p, iStrLen );
					printf ( "\"%s\"%s", sVal.cstr(), i<iLen-1 ? "," : "]\n" );
					p += iStrLen;
				}
			break;
			}
		case JSON_INT32_VECTOR:
			{
				int iLen = sphJsonUnpackInt ( &p );
				printf ( "JSON_INT32_VECTOR (%d) [", iLen );
				for ( int i=0; i<iLen; i++ )
					printf ( "%d%s", sphJsonLoadInt ( &p ), i<iLen-1 ? "," : "]\n" );
				break;
			}
		case JSON_INT64_VECTOR:
			{
				int iLen = sphJsonUnpackInt ( &p );
				printf ( "JSON_INT64_VECTOR (%d) [", iLen );
				for ( int i=0; i<iLen; i++ )
					printf ( INT64_FMT"%s", sphJsonLoadBigint ( &p ), i<iLen-1 ? "," : "]\n" );
				break;
			}
		case JSON_DOUBLE_VECTOR:
			{
				int iLen = sphJsonUnpackInt ( &p );
				printf ( "JSON_DOUBLE_VECTOR (%d) [", iLen );
				for ( int i=0; i<iLen; i++ )
					printf ( "%lf%s", sphQW2D ( sphJsonLoadBigint ( &p ) ), i<iLen-1 ? "," : "]\n" );
				break;
			}

		default:
			printf ( "UNKNOWN\n" );
			break;
		}
		*ppData = p;
	}

	void DebugDump ( const BYTE * p )
	{
		CSphVector<BYTE> dOut;
		sphJsonFormat ( dOut, m_dBuffer.Begin() );
		dOut.Add ( '\0' );
		printf ( "sphJsonFormat: %s\n", (char*)dOut.Begin() );

		printf ( "Blob size: %d bytes\n", m_dBuffer.GetLength() );
		ESphJsonType eType = sphJsonFindFirst ( &p );
		DebugDump ( eType, &p, 0 );
		printf ( "\n" );
	}
};

// unused parameter, simply to avoid type clash between all my yylex() functions
#define YY_NO_UNISTD_H 1
#define YYLEX_PARAM pParser->m_pScanner, pParser
#define YY_DECL int yylex ( YYSTYPE * lvalp, void * yyscanner, JsonParser_c * pParser )

#include "llsphinxjson.c"

void yyerror ( JsonParser_c * pParser, const char * sMessage )
{
	yy2lex_unhold ( pParser->m_pScanner );
	pParser->m_sError.SetSprintf ( "%s near '%s'", sMessage, pParser->m_pLastToken );
}

#include "yysphinxjson.c"

bool sphJsonParse ( CSphVector<BYTE> & dData, char * sData, bool bAutoconv, bool bToLowercase, CSphString & sError )
{
	int iLen = strlen ( sData );
	if ( sData[iLen+1]!=0 )
	{
		sError = "internal error: input data passed to sphJsonParse() must be terminated with a double zero";
		return false;
	}

	JsonParser_c tParser ( dData, bAutoconv, bToLowercase, sError );
	yy2lex_init ( &tParser.m_pScanner );

	/*!COMMIT seems redundant, sphJsonParse() is intentionally destructive; so either remove, or document why we need this*/
	tParser.m_pBuf = (char*) malloc ( iLen );
	memcpy ( tParser.m_pBuf, sData, iLen );

	YY_BUFFER_STATE tLexerBuffer = yy2_scan_buffer ( sData, iLen+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		SafeDelete ( tParser.m_pBuf );
		sError = "internal error: yy_scan_buffer() failed";
		return false;
	}

	int iRes = yyparse ( &tParser );
	yy2_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yy2lex_destroy ( tParser.m_pScanner );

	SafeDelete ( tParser.m_pBuf );
	tParser.Finalize();
	return iRes==0;
}

//////////////////////////////////////////////////////////////////////////

DWORD sphJsonKeyMask ( const char * sKey, int iLen )
{
	DWORD uCrc = sphCRC32 ( (const BYTE*)sKey, iLen );
	return
		( 1UL<<( uCrc & 31 ) ) +
		( 1UL<<( ( uCrc>>8 ) & 31 ) );
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
		for ( ;; )
		{
			ESphJsonType eNode = (ESphJsonType) *p++;
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
	case JSON_ROOT:
		if ( eType==JSON_OBJECT )
			sphJsonUnpackInt ( &p ); // skip size
		p += 4; // skip filter
		for ( ;; )
		{
			ESphJsonType eNode = (ESphJsonType) *p++;
			if ( eNode==JSON_EOF )
				break;
			int iLen = sphJsonUnpackInt ( &p );
			p += iLen;
			sphJsonSkipNode ( eNode, &p );
			iCount++;
		}
		return iCount;
	default:
		return 0;
	}
}


ESphJsonType sphJsonFindFirst ( const BYTE ** ppData )
{
	ESphJsonType eType;
	const BYTE * p = *ppData;

	// maybe make a macro for this some day..
#if UNALIGNED_RAM_ACCESS && USE_LITTLE_ENDIAN
	DWORD uCols = *(DWORD*)p;
#else
	DWORD uCols = p[0] + ( p[1]<<8 ) + ( p[2]<<16 ) + ( p[3]<<24 );
#endif

	if ( uCols==0 )
	{
		// jump 4 bytes ahead, next byte is either JSON_EOF or any other type
		p+=4;
		eType = (ESphJsonType) *p++;
	} else
	{
		// start from JSON_ROOT (basically a JSON_OBJECT without node header)
		eType = JSON_ROOT;
	}

	*ppData = p;
	return eType;
}


ESphJsonType sphJsonFindByKey ( ESphJsonType eType, const BYTE ** ppValue, const void * pKey, int iLen, DWORD uMask )
{
	if ( eType!=JSON_OBJECT && eType!=JSON_ROOT )
		return JSON_EOF;

	const BYTE * p = *ppValue;
	if ( eType==JSON_OBJECT )
		sphJsonUnpackInt ( &p );

#if UNALIGNED_RAM_ACCESS && USE_LITTLE_ENDIAN
	DWORD uCols = *(DWORD*)p;
#else
	DWORD uCols = p[0] + ( p[1]<<8 ) + ( p[2]<<16 ) + ( p[3]<<24 );
#endif
	if ( ( uCols & uMask )!=uMask )
		return JSON_EOF;

	p += 4;
	for ( ;; )
	{
		ESphJsonType eType = (ESphJsonType) *p++;
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
			for ( int i=0; i<iIndex; i++ )
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
			for ( int i=0; i<iIndex; i++ )
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

static const BYTE * JsonFormatStr ( CSphVector<BYTE> & dOut, const BYTE * p, bool bQuote=true )
{
	int iLen = sphJsonUnpackInt ( &p );
	dOut.Reserve ( dOut.GetLength()+iLen );
	if ( bQuote )
		dOut.Add ( '"' );
	while ( iLen-- )
	{
		if ( *p=='"' )
			dOut.Add ( '\\' );
		dOut.Add ( *p );
		p++;
	}
	if ( bQuote )
		dOut.Add ( '"' );
	return p;
}


void JsonAddStr ( CSphVector<BYTE> & dOut, const char * pStr )
{
	while ( *pStr )
		dOut.Add ( *pStr++ );
}


void sphJsonFormat ( CSphVector<BYTE> & dOut, const BYTE * pData )
{
	if ( !pData )
		return;
	ESphJsonType eType = sphJsonFindFirst ( &pData );

	// check for the empty root
	if ( eType==JSON_EOF )
	{
		JsonAddStr ( dOut, "{}" );
		return;
	}

	sphJsonFieldFormat ( dOut, pData, eType );
}


const BYTE * sphJsonFieldFormat ( CSphVector<BYTE> & dOut, const BYTE * pData, ESphJsonType eType, bool bQuoteString )
{
	const BYTE * p = pData;

	// format value
	switch ( eType )
	{
		case JSON_INT32:
		{
			int iOff = dOut.GetLength();
			dOut.Resize ( iOff+32 );
			int iLen = snprintf ( (char *)dOut.Begin()+iOff, 32, "%d", sphJsonLoadInt ( &p ) ); // NOLINT
			dOut.Resize ( iOff+iLen );
			break;
		}
		case JSON_INT64:
		{
			int iOff = dOut.GetLength();
			dOut.Resize ( iOff+32 );
			int iLen = snprintf ( (char *)dOut.Begin()+iOff, 32, INT64_FMT, sphJsonLoadBigint ( &p ) ); // NOLINT
			dOut.Resize ( iOff+iLen );
			break;
		}
		case JSON_DOUBLE:
		{
			int iOff = dOut.GetLength();
			dOut.Resize ( iOff+32 );
			int iLen = snprintf ( (char *)dOut.Begin()+iOff, 32, "%lf", sphQW2D ( sphJsonLoadBigint ( &p ) ) ); // NOLINT
			dOut.Resize ( iOff+iLen );
			break;
		}
		case JSON_STRING:
			p = JsonFormatStr ( dOut, p, bQuoteString );
			break;
		case JSON_STRING_VECTOR:
		{
			int iLen = sphJsonUnpackInt ( &p );
			dOut.Reserve ( dOut.GetLength()+iLen );
			int iVals = sphJsonUnpackInt ( &p );
			dOut.Add ( '[' );
			for ( int i=0; i<iVals; i++ )
			{
				if ( i>0 )
					dOut.Add ( ',' );
				p = JsonFormatStr ( dOut, p );
			}
			dOut.Add ( ']' );
			break;
		}
		case JSON_INT32_VECTOR:
		case JSON_INT64_VECTOR:
		case JSON_DOUBLE_VECTOR:
		{
			int iVals = sphJsonUnpackInt ( &p );
			dOut.Add ( '[' );
			for ( int i=0; i<iVals; i++ )
			{
				if ( i>0 )
					dOut.Add ( ',' );
				int iOff = dOut.GetLength();
				dOut.Resize ( iOff+32 );
				int iLen = 0;
				char * b = (char *)dOut.Begin()+iOff;
				switch ( eType )
				{
				case JSON_INT32_VECTOR: iLen = snprintf ( b, 32, "%d", sphJsonLoadInt ( &p ) ); break; // NOLINT
				case JSON_INT64_VECTOR: iLen = snprintf ( b, 32, INT64_FMT, sphJsonLoadBigint ( &p ) ); break; // NOLINT
				case JSON_DOUBLE_VECTOR: iLen = snprintf ( b, 32, "%lf", sphQW2D ( sphJsonLoadBigint ( &p ) ) ); break; // NOLINT
				default:
					break;
				}
				dOut.Resize ( iOff+iLen );
			}
			dOut.Add ( ']' );
			break;
		}
		case JSON_MIXED_VECTOR:
			{
				sphJsonUnpackInt ( &p );
				int iVals = sphJsonUnpackInt ( &p );
				dOut.Add ( '[' );
				for ( int i=0; i<iVals; i++ )
				{
					if ( i>0 )
						dOut.Add ( ',' );
					ESphJsonType eNode = (ESphJsonType) *p++;
					p = sphJsonFieldFormat ( dOut, p, eNode, true );
				}
				dOut.Add ( ']' );
				break;
			}
		case JSON_ROOT:
		case JSON_OBJECT:
			{
				if ( eType==JSON_OBJECT )
					sphJsonUnpackInt ( &p );
				p += 4; // skip bloom table
				dOut.Add ( '{' );
				for ( int i=0;;i++ )
				{
					ESphJsonType eNode = (ESphJsonType) *p++;
					if ( eNode==JSON_EOF )
						break;
					if ( i>0 )
						dOut.Add ( ',' );
					p = JsonFormatStr ( dOut, p );
					dOut.Add ( ':' );
					p = sphJsonFieldFormat ( dOut, p, eNode, true );
				}
				dOut.Add ( '}' );
				break;
			}
		case JSON_TRUE:		JsonAddStr ( dOut, bQuoteString ? "true" : "True" ); break;
		case JSON_FALSE:	JsonAddStr ( dOut, bQuoteString ? "false" : "False" ); break;
		case JSON_NULL:		JsonAddStr ( dOut, bQuoteString ? "null" : "" ); break;
		case JSON_EOF:
			break;
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
		pSep++;

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


JsonKey_t::JsonKey_t ()
	: m_uMask ( 0 )
	, m_iLen ( 0 )
{}


JsonKey_t::JsonKey_t ( const char * sKey, int iLen )
{
	m_iLen = iLen;
	m_uMask = sphJsonKeyMask ( sKey, m_iLen );
	m_sKey.SetBinary ( sKey, m_iLen );
}


void JsonStoreInt ( BYTE * p, int v )
{
	*p++ = BYTE(DWORD(v));
	*p++ = BYTE(DWORD(v) >> 8);
	*p++ = BYTE(DWORD(v) >> 16);
	*p++ = BYTE(DWORD(v) >> 24);
}


void JsonStoreBigint ( BYTE * p, int64_t v )
{
	JsonStoreInt ( p, (DWORD)( v & 0xffffffffUL ) );
	JsonStoreInt ( p+4, (int)( v>>32 ) );
}


bool sphJsonInplaceUpdate ( ESphJsonType eValueType, int64_t iValue, ISphExpr * pExpr, BYTE * pStrings, const CSphRowitem * pRow, bool bUpdate )
{
	if ( !pExpr || !pStrings )
		return false;

	pExpr->Command ( SPH_EXPR_SET_STRING_POOL, (void*)pStrings );

	CSphMatch tMatch;
	tMatch.m_pStatic = pRow;

	uint64_t uPacked = pExpr->Int64Eval ( tMatch );
	BYTE * pData = pStrings + ( uPacked & 0xffffffff );
	ESphJsonType eType = (ESphJsonType)( uPacked >> 32 );

	switch ( eType )
	{
	case JSON_INT32:
		if ( eValueType==JSON_DOUBLE )
			iValue = (int64_t)sphQW2D ( iValue );
		if ( int64_t(int(iValue))!=iValue )
			return false;
		if ( bUpdate )
			JsonStoreInt ( pData, (int)iValue );
		break;
	case JSON_INT64:
		if ( bUpdate )
			JsonStoreBigint ( pData, eValueType==JSON_DOUBLE ? (int64_t)sphQW2D ( iValue ) : iValue );
		break;
	case JSON_DOUBLE:
		if ( bUpdate )
			JsonStoreBigint ( pData, eValueType==JSON_DOUBLE ? iValue : sphD2QW ( (double)iValue ) );
		break;
	default:
		return false;
	}
	return true;
}

//
// $Id$
//
