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
	ESphJsonType			m_eType;
	CSphString				m_sValue;
	int64_t					m_iValue;
	double					m_fValue;
	CSphVector<CSphString>	m_dValue; // !COMMIT warning! slow!! implement Swap!!!

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
	int					m_iTopLevelKeys;
	DWORD				m_uTopLevelMask;
	bool				m_bAutoconv;
	bool				m_bToLowercase;

public:
	JsonParser_c ( CSphVector<BYTE> & dBuffer, bool bAutoconv, bool bToLowercase, CSphString & sError )
		: m_pScanner ( NULL )
		, m_pLastToken ( NULL )
		, m_dBuffer ( dBuffer )
		, m_sError ( sError )
		, m_iTopLevelKeys ( 0 )
		, m_uTopLevelMask ( 0 )
		, m_bAutoconv ( bAutoconv )
		, m_bToLowercase ( bToLowercase )
	{
		// reserve space for Bloom mask
		for ( int i=0; i<4; i++ )
			m_dBuffer.Add ( 0 );
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
		if ( v<=251 )
		{
			m_dBuffer.Add ( BYTE(v) );
		} else if ( v<65536 )
		{
			m_dBuffer.Add ( 252 );
			m_dBuffer.Add ( BYTE ( v & 255 ) );
			m_dBuffer.Add ( BYTE ( ( v>>8 ) & 255 ) );
		} else if ( v<16777216 )
		{
			m_dBuffer.Add ( 253 );
			m_dBuffer.Add ( BYTE ( v & 255 ) );
			m_dBuffer.Add ( BYTE ( ( v>>8 ) & 255 ) );
			m_dBuffer.Add ( BYTE ( ( v>>16 ) & 255 ) );
		} else
		{
			m_dBuffer.Add ( 253 );
			m_dBuffer.Add ( BYTE ( v & 255 ) );
			m_dBuffer.Add ( BYTE ( ( v>>8 ) & 255 ) );
			m_dBuffer.Add ( BYTE ( ( v>>16 ) & 255 ) );
			m_dBuffer.Add ( BYTE ( ( v>>24 ) & 255 ) );
		}
	}

	void PackStr ( const char * s )
	{
		int iLen = s ? strlen(s) : 0;
		iLen = Min ( iLen, 16777215 );

		PackInt ( iLen );
		if ( iLen )
		{
			BYTE * p = BufAlloc ( iLen );
			memcpy ( p, s, iLen );
		}
	}

public:
	bool Add ( const char * sKey, JsonNode_t & tNode )
	{
		const char * sFixedKey = sKey;
		CSphString sLowercasedKey;
		if ( m_bToLowercase )
		{
			sLowercasedKey = sKey;
			sLowercasedKey.ToLower();
			sFixedKey = sLowercasedKey.cstr();
		}

		// for now, every key is top-level
		m_iTopLevelKeys++;
		m_uTopLevelMask |= sphJsonKeyMask ( sFixedKey );

		// attempt to fixup type
		// convert int/float formatted as string back to numeric, if possible
		while ( m_bAutoconv && tNode.m_eType==JSON_STRING )
		{
			// check whether the string looks like a numeric
			const char * p = tNode.m_sValue.cstr();
			bool bNumeric = ( *p=='-' || *p=='.' || ( *p>='0' && *p<='9' ) );
			bool bDot = ( *p=='.' );
			while ( bNumeric && *++p )
			{
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
				break;
			assert ( !*p );

			// ignore floats for now.. way big precision loss
			if ( bDot )
				break;

			// ok, looks numeric, try integer conversion
			// OPTIMIZE?
			char sVal[32];
			int64_t iVal = strtoll ( tNode.m_sValue.cstr(), NULL, 10 );
			snprintf ( sVal, sizeof(sVal), INT64_FMT, iVal );
			if ( tNode.m_sValue==sVal )
			{
				tNode.m_eType = JSON_INT64;
				tNode.m_iValue = iVal;
			}
			break;
		}

		// parser emits int64 values, lets fixup them to int32
		if ( tNode.m_eType==JSON_INT64 )
		{
			int iVal = int(tNode.m_iValue);
			if ( tNode.m_iValue==int64_t(iVal) )
				tNode.m_eType = JSON_INT32;
		}

		// pack the type and the key
		m_dBuffer.Add ( (BYTE)tNode.m_eType );
		PackStr ( sFixedKey );

		// now pack the data
		switch ( tNode.m_eType )
		{
		case JSON_STRING:
			PackStr ( tNode.m_sValue.cstr() );
			break;
		case JSON_INT32:
			StoreInt ( (int)tNode.m_iValue );
			break;
		case JSON_INT64:
			StoreBigint ( tNode.m_iValue );
			break;
		case JSON_DOUBLE:
			StoreBigint ( sphD2QW ( tNode.m_fValue ) );
			break;
		case JSON_STRING_VECTOR:
			{
				int iTotalLen = PackLen ( tNode.m_dValue.GetLength() ); // vector-len
				ARRAY_FOREACH ( i, tNode.m_dValue )
				{
					int iStrLen = tNode.m_dValue[i].Length();
					iTotalLen += PackLen ( iStrLen ) + iStrLen;
				}

				PackInt ( iTotalLen );
				PackInt ( tNode.m_dValue.GetLength() );
				ARRAY_FOREACH ( i, tNode.m_dValue )
					PackStr ( tNode.m_dValue[i].cstr() );
			}
			break;
		default:
			m_sError = "internal error: unknown json attribute type";
			return false;
		}
		return true;
	}

public:
	void Finalize()
	{
		// store bloom mask, eof marker
		for ( int i=0; i<4; i++ )
		{
			m_dBuffer[i] = BYTE ( m_uTopLevelMask & 0xff );
			m_uTopLevelMask >>= 8;
		}
		m_dBuffer.Add ( JSON_EOF );
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

	YY_BUFFER_STATE tLexerBuffer = yy2_scan_buffer ( sData, iLen+2, tParser.m_pScanner );
	if ( !tLexerBuffer )
	{
		sError = "internal error: yy_scan_buffer() failed";
		return false;
	}

	int iRes = yyparse ( &tParser );
	yy2_delete_buffer ( tLexerBuffer, tParser.m_pScanner );
	yy2lex_destroy ( tParser.m_pScanner );

	tParser.Finalize();
	return iRes==0;
}

//////////////////////////////////////////////////////////////////////////

DWORD sphJsonKeyMask ( const char * sKey )
{
	DWORD uCrc = sphCRC32 ( (const BYTE*)sKey );
	return
		( 1UL<<( uCrc & 31 ) ) +
		( 1UL<<( ( uCrc>>8 ) & 31 ) );
}

ESphJsonType sphJsonFindKey ( const BYTE ** ppValue, const BYTE * pData, const JsonKey_t & tKey )
{
	// quick check using Bloom filter
#if UNALIGNED_RAM_ACCESS && USE_LITTLE_ENDIAN
	DWORD uCols = *(DWORD*)pData;
#else
	DWORD uCols = pData[0] + ( pData[1]<<8 ) + ( pData[2]<<16 ) + ( pData[3]<<24 );
#endif
	if ( ( uCols & tKey.m_uMask )!=tKey.m_uMask )
		return JSON_EOF;

	// skip Bloom filter mask, and scan the entire object
	const BYTE * p = pData + 4;
	ESphJsonType eType;
	for ( ;; )
	{
		// get type
		eType = (ESphJsonType) *p++;
		if ( eType==JSON_EOF )
		{
			// oops, key not found
			*ppValue = NULL;
			return JSON_EOF;
		}

		// check if key matches
		int iNameLen = sphJsonUnpackInt ( &p );
		if ( iNameLen==tKey.m_iLen && tKey.m_uKey==sphFNV64 ( p, iNameLen, SPH_FNV64_SEED ) )
		{
			*ppValue = p + iNameLen;
			return eType;
		}

		// no match, so just skip key name and data
		p += iNameLen;
		int iSkip;
		switch ( eType )
		{
		case JSON_INT32:
			p += 4;
			break;
		case JSON_INT64:
		case JSON_DOUBLE:
			p += 8;
			break;
		case JSON_STRING:
		case JSON_STRING_VECTOR:
			iSkip = sphJsonUnpackInt ( &p );
			p += iSkip;
			break;
		case JSON_EOF:
		default:
			break;
		}
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

void sphJsonFormat ( CSphVector<BYTE> & dOut, const BYTE * pData )
{
	dOut.Add ( '{' );
	if ( !pData )
	{
		dOut.Add ( '}' );
		return;
	}
	const BYTE * p = pData + 4; // skip Bloom filter
	ESphJsonType eType;
	for ( ;; )
	{
		// get value type
		eType = (ESphJsonType) *p++;
		if ( eType==JSON_EOF )
		{
			dOut.Add ( '}' );
			break;
		}

		// format key
		if ( dOut.GetLength()!=1 )
			dOut.Add ( ',' );
		p = JsonFormatStr ( dOut, p );
		dOut.Add ( ':' );

		p = sphJsonFieldFormat ( dOut, p, eType );
	}
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
		int iLen = snprintf ( (char *)dOut.Begin()+iOff, 32, "%f", sphQW2D ( sphJsonLoadBigint ( &p ) ) ); // NOLINT
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
		if ( iVals )
			p = JsonFormatStr ( dOut, p );

		for ( int i=1; i<iVals; i++ )
		{
			dOut.Add ( ',' );
			p = JsonFormatStr ( dOut, p );
		}
		dOut.Add ( ']' );
		break;
	}
	case JSON_EOF:
		break;
	}

	return p;
}


bool sphJsonNameSplit ( const char * sName, CSphString * sColumn, CSphString * sKey )
{
	if ( !sName )
		return false;

	const char * pDot = strchr ( sName, '.' );
	if ( !pDot )
		return false;

	int iDot = pDot - sName;
	if ( sColumn )
		sColumn->SetBinary ( sName, iDot );

	if ( sKey )
		*sKey = sName + iDot + 1;

	return true;
}


JsonKey_t::JsonKey_t ()
	: m_uKey ( 0 )
	, m_uMask ( 0 )
	, m_iLen ( 0 )
{}


JsonKey_t::JsonKey_t ( const char * sKey )
{
	m_uMask = sphJsonKeyMask ( sKey );
	m_iLen = strlen ( sKey );
	m_uKey = sphFNV64 ( (const BYTE *)sKey, m_iLen, SPH_FNV64_SEED );
}


//
// $Id$
//
