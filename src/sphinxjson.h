//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2011-2016, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxjson_
#define _sphinxjson_

#include "sphinxutils.h"
#include "sphinxdefs.h"

struct ISphExpr;

/// supported JSON value types
enum ESphJsonType : BYTE
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


#if UNALIGNED_RAM_ACCESS && USE_LITTLE_ENDIAN

template < typename NUM32 >
inline NUM32 GetNUM32LE ( const BYTE * p )
{
	return *( const NUM32 * ) p;
}

template < typename NUM32 >
inline void StoreNUM32LE ( BYTE * p, NUM32 v )
{
	*( NUM32 * ) ( p ) = v;
}

inline void StoreBigintLE ( BYTE * p, int64_t v )
{
	*( int64_t * ) ( p ) = v;
}

/// get stored value from SphinxBSON blob
inline int64_t sphJsonLoadBigint ( const BYTE ** pp )
{
	int64_t iRes = *( const int64_t * ) ( *pp );
	*pp += 8;
	return iRes;
}

#else

template < typename NUM32 >
inline NUM32 GetNUM32LE ( const BYTE * p )
{
	return p[0] + ( p[1]<<8 ) + ( p[2]<<16 ) + ( p[3]<<24 );
}

template < typename NUM32 >
inline void StoreNUM32LE ( BYTE * p, NUM32 v )
{
	p[0] = BYTE ( DWORD ( v ) );
	p[1] = BYTE ( DWORD ( v ) >> 8 );
	p[2] = BYTE ( DWORD ( v ) >> 16 );
	p[3] = BYTE ( DWORD ( v ) >> 24 );
}

inline void StoreBigintLE ( BYTE * p, int64_t v )
{
	StoreNUM32LE ( p, ( DWORD ) ( v & 0xffffffffUL ) );
	StoreNUM32LE ( p + 4, ( int ) ( v >> 32 ) );
}

/// get stored value from SphinxBSON blob
inline int64_t sphJsonLoadBigint ( const BYTE ** pp )
{
	uint64_t uRes = GetNUM32LE<DWORD> ( *pp );
	uRes += ( uint64_t ) GetNUM32LE<DWORD> ( (*pp)+4 ) << 32;
	*pp+=8;
	return uRes;
}

#endif


/// get stored value from SphinxBSON blob
inline int sphJsonLoadInt ( const BYTE ** pp )
{
	auto uRes = GetNUM32LE<int> ( *pp );
	*pp += 4;
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
			uRes = GetNUM32LE<DWORD> (&p[1]);
			(*pp) += 5;
			break;
		case 255:
			assert ( 0 && "unexpected reserved length code 255" );
			(*pp) += 1;
			break;
	}
	return uRes;
}

// lookup table
// combo of IsEscapeChar + GetEscapedChar with the table works 25..150 times faster!
alignas ( 128 ) constexpr BYTE g_JsonEscapingLookupTable[] = {
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, 'b'|0x80, 't'|0x80, 'n'|0x80, 0x0b, 'f'|0x80, 'r'|0x80, 0x0e, 0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17, 0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,'\"'|0x80,0x23,0x24,0x25,0x26,0x27, 0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37, 0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47, 0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57, 0x58,0x59,0x5a,0x5b,'\\'|0x80,0x5d,0x5e,0x5f,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67, 0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77, 0x78,0x79,0x7a,0x7b,0x7c,0x7d,0x7e,0x7f,
};

alignas ( 64 ) constexpr BYTE g_JsonEscapeControlTable[] = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f";

struct EscapeJsonString_t
{
	static const char cQuote = '"';

	static constexpr BYTE EscapingSpace ( BYTE c )
	{
		if ( c & 0x80 )
			return 0; // high table with charcodes >= 0x80 - no escaping

		if ( g_JsonEscapingLookupTable[c] & 0x80 )
			return 1; // escaping 1 char will add 1 more char
		return ( c & 0xE0 ) ? 0 : 5; // escaping 1 control char will add 5 more chars.

		//return strchr ( "\"\\\b\f\n\r\t", c )!=nullptr; // \ is \x5C, " is \x22
	}

	static void EscapeChar ( BYTE*& pOut, BYTE c )
	{
		// high chars with code > 0x7F
		if ( c & 0x80 )
		{
			*pOut++=c;
			return;
		}

		// lookup escaping chars
		if ( g_JsonEscapingLookupTable[c] & 0x80 )
		{
			pOut[0] = '\\';
			pOut[1] = g_JsonEscapingLookupTable[c] & 0x7F;
			pOut += 2;
			return;
		}

		// not yet escaped chars with codes > 0x1F
		if ( c & 0xE0 )
		{
			*pOut++ = c;
			return;
		}

		// not yet escaped control chars - transform to \u00XX
		memcpy ( pOut, "\\u00", 4 );
		memcpy ( pOut + 4, g_JsonEscapeControlTable + c * 2, 2 );
		pOut += 6;
	}

	static void EscapeCharWithSpaces ( BYTE*& pOut, BYTE c )
	{
		EscapeChar ( pOut, c );
	}

	static void FixupSpace ( BYTE*& pOut, BYTE c )
	{
		EscapeChar ( pOut, c );
	}

};

const StrBlock_t dJsonObj { FROMS ( "," ), FROMS ( "{" ), FROMS ( "}" ) }; // json object
const StrBlock_t dJsonArr { FROMS ( "," ), FROMS ( "[" ), FROMS ( "]" ) }; // json array
const StrBlock_t dJsonObjW { FROMS ( ",\n" ), FROMS ( "{\n" ), FROMS ( "\n}" ) }; // json object with formatting
const StrBlock_t dJsonArrW { FROMS ( ",\n" ), FROMS ( "[\n" ), FROMS ( "\n]" ) }; // json array with formatting

class JsonEscapedBuilder : public EscapedStringBuilder_T<EscapeJsonString_t>
{
	using Base_T = EscapedStringBuilder_T<EscapeJsonString_t>;
public:

	ScopedComma_c Named ( const char * sName )
	{
		Base_T::FixupSpacedAndAppendEscaped ( sName );
		AppendRawChunk ( {":", 1} );
		SkipNextComma ();
		return { *this, nullptr };
	}

	ScopedComma_c Object ( bool bAllowEmpty = false )
	{
		return { *this, dJsonObj, bAllowEmpty };
	}

	ScopedComma_c ObjectW ( bool bAllowEmpty = false )
	{
		return { *this, dJsonObjW, bAllowEmpty };
	}

	ScopedComma_c Array ( bool bAllowEmpty = false )
	{
		return { *this, dJsonArr, bAllowEmpty };
	}

	ScopedComma_c ArrayW ( bool bAllowEmpty = false )
	{
		return { *this, dJsonArrW, bAllowEmpty };
	}

	int NamedBlock( const char* sName )
	{
		Base_T::FixupSpacedAndAppendEscaped( sName );
		AppendRawChunk( { ":", 1 } );
		SkipNextComma();
		return MuteBlock();
	}

	int ObjectBlock()
	{
		return StartBlock( dJsonObj );
	}

	int ObjectWBlock()
	{
		return StartBlock( dJsonObjW );
	}

	int ArrayBlock()
	{
		return StartBlock ( dJsonArr );
	}

	int ArrayWBlock()
	{
		return StartBlock ( dJsonArrW );
	}

	void NamedString ( const char* szName, const char* szValue )
	{
		Named ( szName );
		Base_T::FixupSpacedAndAppendEscaped ( szValue );
	}

	void NamedString ( const char* szName, Str_t sValue )
	{
		Named ( szName );
		Base_T::FixupSpacedAndAppendEscaped ( sValue.first, sValue.second );
	}

	void NamedString ( const char* szName, const CSphString& sValue )
	{
		NamedString ( szName, sValue.cstr() );
	}

	void NamedStringNonEmpty ( const char* szName, const CSphString& sValue )
	{
		if ( !sValue.IsEmpty() )
			NamedString ( szName, sValue );
	}

	template<typename T>
	void NamedVal ( const char* szName, T tValue )
	{
		Named ( szName );
		*this << tValue;
	}

	template<typename T>
	void NamedValNonDefault ( const char* szName, T tValue, T tDefault=0 )
	{
		if ( tValue != tDefault )
			NamedVal ( szName, tValue );
	}

	// non-escaped.
	// Imply, that only internal strings no-need of escape (say, pre-escaped) in use,
	// works faster, because skip escaping pass.

	template<typename STR>
	void NameNE ( STR sName )
	{
		StringBuilder_c::AppendName ( sName );
	}

	template<typename STR1, typename STR2>
	void NamedStringNE ( STR1 sName, STR2 sValue )
	{
		NameNE ( sName );
		AppendString ( sValue, '"' );
	}

	template<typename STR, typename T>
	void NamedValNE ( STR sName, T tValue )
	{
		NameNE ( sName );
		*this << tValue;
	}

	template<typename STR, typename T>
	void NamedValNonNull ( STR sName, T&& tValue )
	{
		if ( tValue )
			NamedValNE ( sName, std::forward<T&&> ( tValue ) );
	}
};

/// parse JSON, convert it into SphinxBSON blob
bool sphJsonParse ( CSphVector<BYTE> & dData, char * sData, bool bAutoconv, bool bToLowercase, bool bCheckSize, CSphString & sError );
bool sphJsonParse ( CSphVector<BYTE> & dData, char * sData, bool bAutoconv, bool bToLowercase, bool bCheckSize, StringBuilder_c & sMsg );
bool sphJsonParse ( CSphVector<BYTE> & dData, const CSphString& sFileName, CSphString & sError );

/// convert SphinxBSON blob back to JSON document
void sphJsonFormat ( JsonEscapedBuilder & dOut, const BYTE * pData );

/// convert SphinxBSON blob back to JSON document
/// NOTE, bQuoteString==false is intended to format individual values only (and avoid quoting string values in that case)
const BYTE * sphJsonFieldFormat ( JsonEscapedBuilder & sOut, const BYTE * pData, ESphJsonType eType, bool bQuoteString=true );

/// compute key mask (for Bloom filtering) from the key name
DWORD sphJsonKeyMask ( const char * sKey, int iLen );

/// returns core type for collections (like JSON_INT32 for JSON_INT32_VECTOR), or JSON_EOF if not suitable
ESphJsonType sphJsonGetCoreType ( ESphJsonType eType );

/// find first value in SphinxBSON blob, return associated type
ESphJsonType sphJsonFindFirst ( const BYTE ** ppData );

/// find value by key in SphinxBSON blob, return associated type
ESphJsonType sphJsonFindByKey ( ESphJsonType eType, const BYTE ** ppValue, const void * pKey, int iLen, DWORD uMask );

/// find value by index in SphinxBSON blob, return associated type
ESphJsonType sphJsonFindByIndex ( ESphJsonType eType, const BYTE ** ppValue, int iIndex );

/// extract object part from the name; return false if not JSON name
bool sphJsonNameSplit ( const char * sName, CSphString * sColumn=nullptr );

/// compute node size, in bytes
/// returns -1 when data itself is required to compute the size, but pData is NULL
int sphJsonNodeSize ( ESphJsonType eType, const BYTE * pData );

/// size of note type eType (-1 for arrays)
int sphJsonSingleNodeSize ( ESphJsonType eType );

/// skip the current node, and update the pointer
void sphJsonSkipNode ( ESphJsonType eType, const BYTE ** ppData );

/// return object length or array length, in elements
/// POD types return 1, empty objects return 0
int sphJsonFieldLength ( ESphJsonType eType, const BYTE * pData );

/// inplace JSON update, both for realtime and non-realtime indexes, returns true if update is possible
bool sphJsonInplaceUpdate ( ESphJsonType eValueType, int64_t iValue, ISphExpr * pExpr, BYTE * pBlobPool, const CSphRowitem * pRow, bool bUpdate );

/// converts string to number
bool sphJsonStringToNumber ( const char * s, int iLen, ESphJsonType & eType, int64_t & iVal, double & fVal );

/// unpack json data offset from uint64_t
uint64_t sphJsonUnpackOffset ( uint64_t uPacked );

/// unpack json data type from uint64_t
ESphJsonType sphJsonUnpackType ( uint64_t uPacked );

/// pack json type and offset to uint64_t
uint64_t sphJsonPackTypeOffset ( ESphJsonType eType, uint64_t uOffset );

/// internal cJSON init
void sphInitCJson();

const char * JsonTypeName ( ESphJsonType eType );

struct cJSON;

/// simple cJSON wrapper
class JsonObj_c
{
public:
	explicit		JsonObj_c ( bool bArray = false );
	explicit		JsonObj_c ( cJSON * pRoot, bool bOwner = true );
	explicit		JsonObj_c ( const char * szJson );
	explicit		JsonObj_c ( Str_t sJson );
					JsonObj_c ( JsonObj_c && rhs ) noexcept;
					~JsonObj_c();

					// a shortcut for !Empty()
					operator bool() const;
	void			Swap ( JsonObj_c& rhs ) noexcept;
	JsonObj_c &		operator = ( JsonObj_c rhs );
	JsonObj_c		operator[] ( int iItem ) const;
	JsonObj_c &		operator++();
	JsonObj_c 		operator*();

	void			AddStr ( const char * szName, const char * szValue );
	void			AddStr ( const char * szName, const CSphString & sValue );
	void			AddInt ( const char * szName, int64_t iValue );
	void			AddFlt ( const char * szName, float fValue );
	void			AddBool ( const char * szName, bool bValue );
	void			AddNull ( const char * szName );
	void			AddItem ( const char * szName, JsonObj_c & tObj );
	void			AddItem ( JsonObj_c & tObj );
	void			DelItem ( const char * szName );
	void			ReplaceItem ( int iIndex, JsonObj_c & tObj );
	JsonObj_c		Clone () const;

	int				Size() const;
	JsonObj_c		GetItem ( const char * szName ) const;
	JsonObj_c		GetIntItem ( const char * szName, CSphString & sError, bool bIgnoreMissing=false ) const;
	JsonObj_c		GetIntItem ( const char * szName1, const char * szName2, CSphString & sError ) const;
	JsonObj_c		GetBoolItem ( const char * szName, CSphString & sError, bool bIgnoreMissing=false ) const;
	JsonObj_c		GetStrItem ( const char * szName, CSphString & sError, bool bIgnoreMissing=false ) const;
	JsonObj_c		GetStrItem ( const char * szName1, const char * szName2, CSphString & sError ) const;
	JsonObj_c		GetObjItem ( const char * szName, CSphString & sError, bool bIgnoreMissing=false ) const;
	JsonObj_c		GetArrayItem ( const char * szName, CSphString & sError, bool bIgnoreMissing=false ) const;
	bool			FetchIntItem ( int & iValue, const char * szName, CSphString & sError, bool bIgnoreMissing=false ) const;
	bool			FetchBoolItem ( bool & bValue, const char * szName, CSphString & sError, bool bIgnoreMissing=false ) const;
	bool			FetchStrItem ( CSphString & sValue, const char * szName, CSphString & sError, bool bIgnoreMissing=false ) const;
	bool			HasItem ( const char * szName ) const;

	static JsonObj_c CreateInt ( int64_t iInt );
	static JsonObj_c CreateStr ( const CSphString & sStr );

	bool			IsInt() const;
	bool			IsDbl() const;
	bool			IsNum() const;
	bool			IsBool() const;
	bool			IsObj() const;
	bool			IsStr() const;
	bool			IsArray() const;
	bool			Empty() const;
	const char *	Name() const;

	int64_t			IntVal() const;
	bool			BoolVal() const;
	float			FltVal() const;
	double			DblVal() const;
	const char *	SzVal() const;
	CSphString		StrVal() const;

	const char *	GetErrorPtr() const;
	bool			GetError ( const char * szBuf, int iBufLen, CSphString & sError ) const;
	bool			GetError ( const char* szBuf, int iBufLen ) const;
	cJSON *			GetRoot();
	CSphString		AsString ( bool bFormat=false ) const;

	JsonObj_c		begin() const;
	JsonObj_c		end() const;

protected:
	cJSON *			m_pRoot {nullptr};
	bool			m_bOwner {true};

	cJSON *			Leak();
	JsonObj_c		GetChild ( const char * szName, CSphString & sError, bool bIgnoreMissing ) const;
};

#ifndef JsonNull
#define JsonNull JsonObj_c(nullptr,false)
#endif

namespace bson {

using NodeHandle_t = std::pair< const BYTE *, ESphJsonType>;
const NodeHandle_t nullnode = { nullptr, JSON_EOF };

inline NodeHandle_t MakeHandle ( const VecTraits_T<BYTE>& dData )
{
	return dData.IsEmpty () ? nullnode : NodeHandle_t ( { dData.begin (), JSON_ROOT } );
}

bool IsAssoc ( const NodeHandle_t & );
bool IsArray ( const NodeHandle_t & );
bool IsPODBlob ( const NodeHandle_t & );
bool IsString ( const NodeHandle_t & );
bool IsInt ( const NodeHandle_t & );
bool IsDouble ( const NodeHandle_t & );
bool IsNumeric ( const NodeHandle_t & );

// access to values by locator
bool Bool ( const NodeHandle_t& tLocator, bool bDefault=false );
int64_t Int ( const NodeHandle_t & tLocator, int64_t iDefault=0 );
double Double ( const NodeHandle_t & tLocator, double fDefault=0.0 );
CSphString String ( const NodeHandle_t & tLocator, CSphString sDefault="" );
Str_t ToStr ( const NodeHandle_t & tLocator );
inline bool IsNullNode ( const NodeHandle_t & dNode ) { return dNode==nullnode; }

// iterate over collection (without names).
using Action_f = std::function<void ( const NodeHandle_t &tNode )>;
void ForEach ( const NodeHandle_t &tLocator, Action_f&& fAction );

// iterate over collection (incl. names).
using NamedAction_f = std::function<void ( CSphString&& sName, const NodeHandle_t &tNode )>;
void ForEach ( const NodeHandle_t &tLocator, NamedAction_f&& fAction );

// iterate over collection (without names). Return false from action means 'stop iteration'
using CondAction_f = std::function<bool ( const NodeHandle_t &tNode )>;
void ForSome ( const NodeHandle_t &tLocator, CondAction_f&& fAction );

// iterate over collection (incl. names). Return false from action means 'stop iteration'
using CondNamedAction_f = std::function<bool ( CSphString&& sName, const NodeHandle_t &tNode )>;
void ForSome ( const NodeHandle_t &tLocator, CondNamedAction_f&& fAction );

// suitable for strings and vectors of pods as int, int64, double.
std::pair<const char*, int> RawBlob ( const NodeHandle_t &tLocator );

// many internals might be represented as vector
template<typename BLOB> VecTraits_T<BLOB> Vector ( const NodeHandle_t &tLocator )
{
	auto dBlob = RawBlob ( tLocator );
	return VecTraits_T<BLOB> ( (BLOB*) dBlob.first, dBlob.second );
}

// access to encoded bson
class Bson_c
{
protected:
	NodeHandle_t	m_dData { nullnode };
	mutable StringBuilder_c	m_sError;

public:
	Bson_c ( NodeHandle_t dBson = nullnode )
		: m_dData { std::move (dBson) }
	{}
	explicit Bson_c ( const VecTraits_T<BYTE>& dBlob );

	// traits
	bool IsAssoc () const { return bson::IsAssoc ( m_dData ); } /// whether we can access members by name
	bool IsArray () const { return bson::IsArray ( m_dData ); } /// whether we can access members by index
	bool IsNull () const { return m_dData==nullnode; } /// whether node valid (i.e. not eof type, not nullptr locator).
	operator bool() const { return !IsNull(); } // same as IsNull.
	bool IsString () const { return m_dData.second==JSON_STRING; }
	bool IsInt () const { return bson::IsInt ( m_dData ); }
	bool IsDouble () const { return bson::IsDouble ( m_dData ); }
	bool IsNumeric () const { return bson::IsNumeric (m_dData); }

	bool IsNonEmptyString () const { return bson::IsString ( m_dData ) && !IsEmpty (); } /// whether we can return non-empty string
	bool IsEmpty () const; /// whether container bson has child elements, or string is empty.
	int CountValues() const; /// count of elems. Objs and root will linearly iterate, other returns immediately.
	int StandaloneSize() const; /// size of blob need to save node as root (standalone) bson
	bool StrEq ( const char* sValue ) const; // true if value is string and eq to sValue

	// navigate over bson
	NodeHandle_t ChildByName ( const char* sName ) const; // look by direct child name
	NodeHandle_t ChildByIndex ( int iIdx ) const; // look by direct child idx
	NodeHandle_t ChildByPath ( const char * sPath ) const; // complex look like 'query.percolate.documents[3].title'

	// rapid lookup by name helpers. Ellipsis must be list of str literals,
	// like HasAnyOf (2, "foo", "bar"); HasAnyOf (3, "foo", "bar", "baz")
	bool HasAnyOf ( int iNames, ... ) const;
	bool HasAnyOf ( std::initializer_list<const char*> dNames ) const;
	std::pair<int,NodeHandle_t> GetFirstOf ( std::initializer_list<const char *> dNames ) const;

	// access to my values
	bool Bool () const;
	int64_t Int () const;
	double Double () const;
	CSphString String () const;
	void ForEach ( Action_f&& fAction ) const;
	void ForEach ( NamedAction_f&& fAction ) const;
	void ForSome ( CondAction_f&& fAction ) const;
	void ForSome ( CondNamedAction_f&& fAction ) const;

	// format back to json
	bool BsonToJson ( CSphString& ) const;

	// save as standalone (root) bson.
	bool BsonToBson ( BYTE* ) const;
	bool BsonToBson ( CSphVector<BYTE>& dOutput ) const;

	// helpers
	inline ESphJsonType GetType() const { return m_dData.second; }
	operator NodeHandle_t () const { return m_dData; }
	const char * sError () const;
};

// iterate over Bson_c
class BsonIterator_c : public Bson_c
{
	const BYTE * m_pData = nullptr;
	ESphJsonType m_eType = JSON_EOF; // parent's type
	int m_iSize = -1; // for nodes with known size
	CSphString m_sName;

	inline bool Finish () // used as shortcut return
	{
		if ( m_iSize>0 )
			m_iSize = 0;
		m_dData = bson::nullnode;
		return false;
	}

public:
	explicit BsonIterator_c ( const NodeHandle_t &dParent );
	bool Next();
	int NumElems() const { return m_iSize; } // how many items we still not iterated ( actual only for arrays; otherwise it is -1 )
	CSphString GetName() const { return m_sName; }
};

// parse and access to encoded bson
class BsonContainer_c : public Bson_c
{
	CSphVector<BYTE> m_Bson;
public:
	explicit BsonContainer_c ( char* sJson, bool bAutoconv=false, bool bToLowercase=true, bool bCheckSize=true );
	explicit BsonContainer_c ( const char * sJsonc, bool bAutoconv=false, bool bToLowercase = true, bool bCheckSize=true )
	: BsonContainer_c ( const_cast<char *> ( CSphString ( sJsonc ).cstr() ), bToLowercase, bCheckSize ) {}
};

class BsonContainer2_c : public Bson_c
{
	CSphVector<BYTE> m_Bson;
public:
	explicit BsonContainer2_c ( const char * sJsonc, bool bAutoconv = false, bool bToLowercase = true );
};

bool	JsonObjToBson ( JsonObj_c & tJSON, CSphVector<BYTE> &dData, bool bAutoconv, bool bToLowercase/*, StringBuilder_c &sMsg*/ );
bool	cJsonToBson ( cJSON * pCJSON, CSphVector<BYTE> &dData, bool bAutoconv=false, bool bToLowercase = true /*, StringBuilder_c &sMsg */);

enum class JsonParser_e
{
	BSON,
	CJSON,
};

bool ValidateJson ( const char * sJson, JsonParser_e eParse, CSphString * pError=nullptr );
bool ValidateJson ( const char* sJson, CSphString* pError=nullptr );


// this are generic purpose serializer (bson as any object)
class Assoc_c
{
protected:
	CSphVector<BYTE>&	m_dBson;
	DWORD				m_uBloomMask = 0;
	void StartTypedNode ( ESphJsonType eType, const char* szName=nullptr );

public:
	explicit Assoc_c ( CSphVector<BYTE> & dBsonTarget )
			: m_dBson ( dBsonTarget )
	{}

	void AddInt ( const char* szName, int64_t iValue );
	void AddDouble ( const char * szName, double fValue );
	void AddString ( const char * szName, const char* szValue );
	void AddBlob ( const char * szName, const ByteBlob_t & dValue ); // blob of bytes as string
	void AddBool ( const char * szName, bool bValue );
	void AddNull ( const char * szName );
	void AddStringVec ( const char * szName, const VecTraits_T<CSphString> & dData );
	void AddStringVec ( const char * szName, const VecTraits_T<const char *> & dData );
	void AddIntVec ( const char * szName, const VecTraits_T<int> & dData );
	void AddIntVec ( const char * szName, const VecTraits_T<int64_t> & dData );
	void AddDoubleVec ( const char * szName, const VecTraits_T<double> & dData );
	CSphVector<BYTE> & StartObj ( const char * szName );
	CSphVector<BYTE> & StartStringVec ( const char * szName );
	CSphVector<BYTE> & StartIntVec ( const char * szName );
	CSphVector<BYTE> & StartBigintVec ( const char * szName );
	CSphVector<BYTE> & StartDoubleVec ( const char * szName );
	CSphVector<BYTE> & StartMixedVec ( const char * szName );
};

class Root_c : public Assoc_c
{
public:
	explicit Root_c ( CSphVector<BYTE> & dBsonTarget );
	~Root_c ();
};

class Obj_c : public Assoc_c
{
	int m_iStartPos;
public:
	explicit Obj_c ( CSphVector<BYTE> & dBsonTarget );
	~Obj_c ();
};

class MixedVector_c
{
	CSphVector<BYTE>& m_dBson;
	int m_iStartPos;

public:
	MixedVector_c ( CSphVector<BYTE> & dBson, int iSize );
	~MixedVector_c();

	void AddInt ( int64_t iValue );
	void AddDouble ( double fValue );
	void AddString ( const char* szValue );
	void AddBlob ( const ByteBlob_t& dValue ); // blob of bytes as string
	void AddBool ( bool bValue );
	void AddNull ();
	void AddStringVec ( const VecTraits_T<CSphString> & dData );
	void AddStringVec ( const VecTraits_T<const char*> & dData );
	void AddIntVec ( const VecTraits_T<int> & dData );
	void AddIntVec ( const VecTraits_T<int64_t> & dData );
	void AddDoubleVec ( const VecTraits_T<double> & dData );
	CSphVector<BYTE> & StartObj ();
	CSphVector<BYTE> & StartStringVec ();
	CSphVector<BYTE> & StartIntVec ();
	CSphVector<BYTE> & StartBigintVec ();
	CSphVector<BYTE> & StartDoubleVec ();
	CSphVector<BYTE> & StartMixedVec ();
};


class StringVector_c
{
	CSphVector<BYTE>& m_dBson;
	int m_iStartPos;

public:
	StringVector_c ( CSphVector<BYTE> & dBson, int iSize );
	~StringVector_c();

	void AddValue ( const char * szValue );
	void AddValue ( const ByteBlob_t& dValue );
	void AddValues ( const VecTraits_T<CSphString> & dData );
	void AddValues ( const VecTraits_T<const char *> & dData );
};

class IntVector_c
{
	CSphVector<BYTE>& m_dBson;

public:
	IntVector_c ( CSphVector<BYTE> & dBson, int iSize );
	void AddValue ( int iValue );
	void AddValues ( const VecTraits_T<int> & dData );
};

class BigintVector_c
{
	CSphVector<BYTE>& m_dBson;

public:
	BigintVector_c ( CSphVector<BYTE> & dBson, int iSize );
	void AddValue ( int64_t iValue );
	void AddValues ( const VecTraits_T<int64_t> & dData );
};

class DoubleVector_c
{
	CSphVector<BYTE>& m_dBson;

public:
	DoubleVector_c ( CSphVector<BYTE> & dBson, int iSize );
	void AddValue ( double iValue );
	void AddValues ( const VecTraits_T<double> & dData );
};

}; // namespace bson

int sphJsonUnescape ( char ** pEscaped, int iLen );
int sphJsonUnescape1 ( char ** pEscaped, int iLen );
int JsonUnescape ( char * pTarget, const char * pEscaped, int iLen );

#endif // _sphinxjson_

