//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "netreceive_ql.h"
#include "coroutine.h"
#include "searchdssl.h"
#include "compressed_zlib_mysql.h"
#include "compressed_zstd_mysql.h"
#include "daemon/logger.h"
#include "searchdbuddy.h"
#include "client_session.h"
#include "searchdha.h"
#include "sphinxexpr.h"
#include "std/escaped_builder.h"
#include <functional>

extern int g_iClientQlTimeoutS;    // sec
extern volatile bool g_bMaintenance;
extern CSphString g_sMySQLVersion;
namespace { // c++ way of 'static'

/// proto details are here: https://dev.mysql.com/doc/dev/mysql-server/latest/page_protocol_basic_packets.html

bool OmitEof() noexcept
{
	return session::GetDeprecatedEOF();
}

/////////////////////////////////////////////////////////////////////////////
/// how many bytes this int will occupy in proto mysql
template<typename INT>
int SqlSizeOf ( INT _iLen ) noexcept
{
	auto iLen = (uint64_t)_iLen;
	if ( iLen < 251 )
		return 1;
	if ( iLen <= 0xffff )
		return 3;
	if ( iLen <= 0xffffff )
		return 4;
	return 9;
}

/////////////////////////////////////////////////////////////////////////////
/// encodes Mysql Length-coded binary
int MysqlPackInt ( BYTE* pOutput, int64_t iValue )
{
	switch ( SqlSizeOf ( iValue ) )
	{
	case 1: *pOutput = (BYTE)iValue; return 1;
#if USE_LITTLE_ENDIAN
	case 3:
		*pOutput = (BYTE)'\xFC'; // 252
		memcpy ( pOutput + 1, &iValue, 2 );
		return 3;
	case 4:
		*pOutput = (BYTE)'\xFD'; // 253
		memcpy ( pOutput + 1, &iValue, 3 );
		return 4;
	case 9:
	default:
		*pOutput = (BYTE)'\xFE'; // 254
		memcpy ( pOutput + 1, &iValue, 8 );
		return 9;
#else
	case 3:
		pOutput[0] = (BYTE)'\xFC'; // 252
		pOutput[1] = (BYTE)iValue;
		pOutput[2] = (BYTE)( iValue >> 8 );
		return 3;
	case 4:
		pOutput[0] = (BYTE)'\xFD'; // 253
		pOutput[1] = (BYTE)iValue;
		pOutput[2] = (BYTE)( iValue >> 8 );
		pOutput[3] = (BYTE)( iValue >> 16 );
		return 4;
	case 9:
	default:
		pOutput[0] = (BYTE)'\xFE'; // 254
		pOutput[1] = (BYTE)iValue;
		pOutput[2] = (BYTE)( iValue >> 8 );
		pOutput[3] = (BYTE)( iValue >> 16 );
		pOutput[4] = (BYTE)( iValue >> 24 );
		pOutput[5] = (BYTE)( iValue >> 32 );
		pOutput[6] = (BYTE)( iValue >> 40 );
		pOutput[7] = (BYTE)( iValue >> 48 );
		pOutput[8] = (BYTE)( iValue >> 56 );
		return 9;
#endif
	}
}

void MysqlSendInt ( ISphOutputBuffer& dOut, int64_t iValue )
{
	std::array<BYTE, 10> dBuf; // ok unitialized
	auto iLen = MysqlPackInt ( dBuf.data(), iValue );
	dOut.SendBytes ( dBuf.data(), iLen );
}

int64_t MysqlReadPackedInt ( InputBuffer_c& tIn )
{
	BYTE uVal = tIn.GetByte();
	int64_t iRes = 0;

	switch ( uVal )
	{
	case 0xFC:
		{
			iRes = tIn.GetByte();
			iRes += tIn.GetByte() << 8;
			return iRes;
		}
	case 0xFD:
		{
			iRes = tIn.GetByte();
			iRes += tIn.GetByte() << 8;
			iRes += tIn.GetByte() << 16;
			return iRes;
		}
	case 0xFE:
		{
			iRes = tIn.GetByte();
			iRes += tIn.GetByte() << 8;
			iRes += tIn.GetByte() << 16;
			iRes += tIn.GetByte() << 24;
			iRes += int64_t ( tIn.GetByte() ) << 32;
			iRes += int64_t ( tIn.GetByte() ) << 40;
			iRes += int64_t ( tIn.GetByte() ) << 48;
			iRes += int64_t ( tIn.GetByte() ) << 56;
			return iRes;
		}
	default:
		return int64_t (uVal);
	}
}

CSphString MysqlReadSzStr ( InputBuffer_c& tIn )
{
	Str_t sData = FromSz ( (const char*)tIn.GetBufferPtr() );
	CSphString sResult ( sData );
	tIn.SetBufferPos ( tIn.GetBufferPos() + sData.second + 1 ); // +1 to skip z-terminator
	return sResult;
}

CSphString MysqlReadVlStr ( InputBuffer_c& tIn )
{
	auto iLen = MysqlReadPackedInt ( tIn );
	return tIn.GetRawString ( iLen );
}

WORD MysqlReadLSBWord ( InputBuffer_c & tIn )
{
	WORD uLo = tIn.GetByte();
	WORD uHi = tIn.GetByte();
	return WORD ( uLo | ( uHi << 8 ) );
}

DWORD MysqlReadLSBDword ( InputBuffer_c & tIn )
{
	DWORD u0 = tIn.GetByte();
	DWORD u1 = tIn.GetByte();
	DWORD u2 = tIn.GetByte();
	DWORD u3 = tIn.GetByte();
	return u0 | ( u1 << 8 ) | ( u2 << 16 ) | ( u3 << 24 );
}

uint64_t MysqlReadLSBQword ( InputBuffer_c & tIn )
{
	uint64_t u0 = tIn.GetByte();
	uint64_t u1 = tIn.GetByte();
	uint64_t u2 = tIn.GetByte();
	uint64_t u3 = tIn.GetByte();
	uint64_t u4 = tIn.GetByte();
	uint64_t u5 = tIn.GetByte();
	uint64_t u6 = tIn.GetByte();
	uint64_t u7 = tIn.GetByte();
	return u0 | ( u1 << 8 ) | ( u2 << 16 ) | ( u3 << 24 ) | ( u4 << 32 ) | ( u5 << 40 ) | ( u6 << 48 ) | ( u7 << 56 );
}

enum MysqlType_e
{
	MYSQL_TYPE_DECIMAL = 0,
	MYSQL_TYPE_TINY = 1,
	MYSQL_TYPE_SHORT = 2,
	MYSQL_TYPE_LONG = 3,
	MYSQL_TYPE_FLOAT = 4,
	MYSQL_TYPE_DOUBLE = 5,
	MYSQL_TYPE_NULL = 6,
	MYSQL_TYPE_TIMESTAMP = 7,
	MYSQL_TYPE_LONGLONG = 8,
	MYSQL_TYPE_INT24 = 9,
	MYSQL_TYPE_DATE = 10,
	MYSQL_TYPE_TIME = 11,
	MYSQL_TYPE_DATETIME = 12,
	MYSQL_TYPE_YEAR = 13,
	MYSQL_TYPE_VARCHAR = 15,
	MYSQL_TYPE_BIT = 16,
	MYSQL_TYPE_JSON = 245,
	MYSQL_TYPE_NEWDECIMAL = 246,
	MYSQL_TYPE_ENUM = 247,
	MYSQL_TYPE_SET = 248,
	MYSQL_TYPE_TINY_BLOB = 249,
	MYSQL_TYPE_MEDIUM_BLOB = 250,
	MYSQL_TYPE_LONG_BLOB = 251,
	MYSQL_TYPE_BLOB = 252,
	MYSQL_TYPE_VAR_STRING = 253,
	MYSQL_TYPE_STRING = 254
};

// RAII Mysql API block: in ctr reserve place for size, in dtr write LSB with size and packet ID
class SQLPacketHeader_c
{
	ISphOutputBuffer& m_dOut;
	BYTE m_uPacketID;
	intptr_t m_iPos;

public:
	explicit SQLPacketHeader_c ( ISphOutputBuffer& dOut, BYTE uPacketID = 0 )
		: m_dOut ( dOut )
		, m_uPacketID ( uPacketID )
		, m_iPos { (intptr_t)m_dOut.GetSentCount() }
	{
		m_dOut.SendLSBDword ( 0 );
	}

	~SQLPacketHeader_c()
	{
		auto iBlobLen = m_dOut.GetSentCount() - m_iPos - sizeof ( int );
		m_dOut.WriteLSBDword( m_iPos, ( m_uPacketID << 24 ) + iBlobLen );
	}
};

//////////////////////////////////////////////////////////////////////////
// MYSQLD PRETENDER
//////////////////////////////////////////////////////////////////////////

struct MYSQL_FLAG
{
	static constexpr WORD STATUS_IN_TRANS = 1;		// mysql.h: SERVER_STATUS_IN_TRANS
	static constexpr WORD STATUS_AUTOCOMMIT = 2;	// mysql.h: SERVER_STATUS_AUTOCOMMIT
	static constexpr WORD MORE_RESULTS = 8;		// mysql.h: SERVER_MORE_RESULTS_EXISTS
};

constexpr int MAX_PACKET_LEN = 0x00FFFFFFL; // 16777215 bytes, max low level packet size. Notice, also used as mask.

struct MYSQL_CHARSET
{
	static constexpr BYTE utf8_general_ci = 0x21;
//	static constexpr BYTE binary = 0x3f;
};

enum class PreparedScanState_e
{
	NORMAL,
	SINGLE_QUOTE,
	DOUBLE_QUOTE,
	BACKTICK,
	LINE_COMMENT,
	BLOCK_COMMENT
};

static bool IsLineCommentStart ( const char * p, int iPos, int iLen )
{
	if ( iPos + 1 >= iLen )
		return false;
	if ( p[iPos]=='#' )
		return true;
	if ( p[iPos]=='-' && p[iPos+1]=='-' )
	{
		if ( iPos + 2 >= iLen )
			return true;
		char c = p[iPos+2];
		return c==' ' || c=='\t' || c=='\r' || c=='\n';
	}
	return false;
}

static int CountPreparedParams ( Str_t tQuery )
{
	const char * p = tQuery.first;
	int iLen = (int)tQuery.second;
	int iCount = 0;
	auto eState = PreparedScanState_e::NORMAL;

	for ( int i = 0; i < iLen; ++i )
	{
		char c = p[i];
		switch ( eState )
		{
		case PreparedScanState_e::NORMAL:
			if ( c=='\'' )
				eState = PreparedScanState_e::SINGLE_QUOTE;
			else if ( c=='\"' )
				eState = PreparedScanState_e::DOUBLE_QUOTE;
			else if ( c=='`' )
				eState = PreparedScanState_e::BACKTICK;
			else if ( IsLineCommentStart ( p, i, iLen ) )
				eState = PreparedScanState_e::LINE_COMMENT;
			else if ( c=='/' && i+1<iLen && p[i+1]=='*' )
				eState = PreparedScanState_e::BLOCK_COMMENT;
			else if ( c=='?' )
				++iCount;
			break;

		case PreparedScanState_e::SINGLE_QUOTE:
			if ( c=='\\' && i+1<iLen )
				++i;
			else if ( c=='\'' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::DOUBLE_QUOTE:
			if ( c=='\\' && i+1<iLen )
				++i;
			else if ( c=='\"' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::BACKTICK:
			if ( c=='`' )
			{
				if ( i+1<iLen && p[i+1]=='`' )
					++i;
				else
					eState = PreparedScanState_e::NORMAL;
			}
			break;

		case PreparedScanState_e::LINE_COMMENT:
			if ( c=='\n' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::BLOCK_COMMENT:
			if ( c=='*' && i+1<iLen && p[i+1]=='/' )
			{
				++i;
				eState = PreparedScanState_e::NORMAL;
			}
			break;
		}
	}

	return iCount;
}

static bool IsVectorListParam ( const CSphString & sParam, CSphString & sError )
{
	const char * p = sParam.cstr();
	if ( !p )
	{
		sError = "empty list parameter";
		return false;
	}
	const char * pEnd = p + sParam.Length();
	while ( p < pEnd && sphIsSpace ( *p ) )
		++p;
	while ( pEnd > p && sphIsSpace ( pEnd[-1] ) )
		--pEnd;
	if ( pEnd - p < 2 || *p!='(' || pEnd[-1]!=')' )
	{
		sError = "list parameter must be in '(...)' format";
		return false;
	}
	for ( const char * pCur = p + 1; pCur < pEnd - 1; ++pCur )
	{
		const char c = *pCur;
		if ( sphIsSpace ( c ) || c==',' || c=='+' || c=='-' || c=='.' || c=='e' || c=='E' )
			continue;
		if ( c>='0' && c<='9' )
			continue;
		sError.SetSprintf ( "invalid list character '%c'", c );
		return false;
	}
	return true;
}

static bool RenderPreparedQuery ( Str_t tQuery, const CSphVector<CSphString> & dParams, CSphString & sOut, CSphString & sError, const CSphVector<bool> * pRawParams )
{
	const char * p = tQuery.first;
	int iLen = (int)tQuery.second;
	StringBuilder_c sSql;
	auto eState = PreparedScanState_e::NORMAL;
	int iParam = 0;

	for ( int i = 0; i < iLen; ++i )
	{
		char c = p[i];
		switch ( eState )
		{
		case PreparedScanState_e::NORMAL:
			if ( c=='\'' )
				eState = PreparedScanState_e::SINGLE_QUOTE;
			else if ( c=='\"' )
				eState = PreparedScanState_e::DOUBLE_QUOTE;
			else if ( c=='`' )
				eState = PreparedScanState_e::BACKTICK;
			else if ( IsLineCommentStart ( p, i, iLen ) )
				eState = PreparedScanState_e::LINE_COMMENT;
			else if ( c=='/' && i+1<iLen && p[i+1]=='*' )
				eState = PreparedScanState_e::BLOCK_COMMENT;
			else if ( c=='?' )
			{
				if ( iParam>=dParams.GetLength() )
				{
					sError = "not enough parameters for prepared statement";
					return false;
				}
				if ( pRawParams && iParam < pRawParams->GetLength() && (*pRawParams)[iParam] )
				{
					CSphString sVal = dParams[iParam];
					sVal.Unquote();
					CSphString sErr;
					if ( !IsVectorListParam ( sVal, sErr ) )
					{
						sError.SetSprintf ( "invalid list parameter %d: %s", iParam+1, sErr.cstr() );
						return false;
					}
					sSql << sVal.cstr();
					iParam++;
				} else
				{
					sSql << dParams[iParam++].cstr();
				}
				continue;
			}
			break;

		case PreparedScanState_e::SINGLE_QUOTE:
			if ( c=='\\' && i+1<iLen )
			{
				sSql.AppendRawChunk ( Str_t { p + i, 1 } );
				++i;
			}
			else if ( c=='\'' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::DOUBLE_QUOTE:
			if ( c=='\\' && i+1<iLen )
			{
				sSql.AppendRawChunk ( Str_t { p + i, 1 } );
				++i;
			}
			else if ( c=='\"' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::BACKTICK:
			if ( c=='`' )
			{
				if ( i+1<iLen && p[i+1]=='`' )
				{
					sSql.AppendRawChunk ( Str_t { p + i, 1 } );
					++i;
				}
				else
					eState = PreparedScanState_e::NORMAL;
			}
			break;

		case PreparedScanState_e::LINE_COMMENT:
			if ( c=='\n' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::BLOCK_COMMENT:
			if ( c=='*' && i+1<iLen && p[i+1]=='/' )
			{
				++i;
				eState = PreparedScanState_e::NORMAL;
			}
			break;
		}

		sSql.AppendRawChunk ( Str_t { p + i, 1 } );
	}

	if ( iParam!=dParams.GetLength() )
	{
		sError = "too many parameters for prepared statement";
		return false;
	}

	sOut = sSql.cstr();
	return true;
}

static bool TokenEq ( const char * sTok, int iLen, const char * sRef )
{
	const int iRefLen = (int)strlen ( sRef );
	if ( iLen!=iRefLen )
		return false;
	for ( int i = 0; i < iLen; ++i )
	{
		const char a = (char)tolower ( (unsigned char)sTok[i] );
		const char b = sRef[i];
		if ( a!=b )
			return false;
	}
	return true;
}

static bool NormalizeVectorFunctions ( Str_t tQuery, CSphString & sOut, CSphVector<int> & dExplicitRaw, CSphString & sError )
{
	const char * p = tQuery.first;
	int iLen = (int)tQuery.second;
	StringBuilder_c sSql;
	auto eState = PreparedScanState_e::NORMAL;
	int iParam = 0;

	for ( int i = 0; i < iLen; ++i )
	{
		char c = p[i];
		switch ( eState )
		{
		case PreparedScanState_e::NORMAL:
			if ( c=='\'' )
				eState = PreparedScanState_e::SINGLE_QUOTE;
			else if ( c=='\"' )
				eState = PreparedScanState_e::DOUBLE_QUOTE;
			else if ( c=='`' )
				eState = PreparedScanState_e::BACKTICK;
			else if ( IsLineCommentStart ( p, i, iLen ) )
				eState = PreparedScanState_e::LINE_COMMENT;
			else if ( c=='/' && i+1<iLen && p[i+1]=='*' )
				eState = PreparedScanState_e::BLOCK_COMMENT;
			else if ( ( c>='A' && c<='Z' ) || ( c>='a' && c<='z' ) || c=='_' )
			{
				int iStart = i;
				int iEnd = i + 1;
				while ( iEnd<iLen )
				{
					char cc = p[iEnd];
					if ( ( cc>='A' && cc<='Z' ) || ( cc>='a' && cc<='z' ) || ( cc>='0' && cc<='9' ) || cc=='_' )
						++iEnd;
					else
						break;
				}
				if ( TokenEq ( p + iStart, iEnd - iStart, "to_vector" ) || TokenEq ( p + iStart, iEnd - iStart, "to_multi" ) )
				{
					int k = iEnd;
					while ( k<iLen && sphIsSpace ( p[k] ) )
						++k;
					if ( k<iLen && p[k]=='(' )
					{
						++k;
						while ( k<iLen && sphIsSpace ( p[k] ) )
							++k;
						if ( k<iLen && p[k]=='?' )
						{
							++k;
							while ( k<iLen && sphIsSpace ( p[k] ) )
								++k;
							if ( k<iLen && p[k]==')' )
							{
								sSql << '?';
								dExplicitRaw.Add ( iParam++ );
								i = k;
								continue;
							}
						}
					}
				}
				sSql.AppendRawChunk ( Str_t { p + iStart, iEnd - iStart } );
				i = iEnd - 1;
				continue;
			}
			else if ( c=='?' )
			{
				sSql << '?';
				++iParam;
				continue;
			}
			break;

		case PreparedScanState_e::SINGLE_QUOTE:
			if ( c=='\\' && i+1<iLen )
			{
				sSql.AppendRawChunk ( Str_t { p + i, 1 } );
				++i;
			}
			else if ( c=='\'' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::DOUBLE_QUOTE:
			if ( c=='\\' && i+1<iLen )
			{
				sSql.AppendRawChunk ( Str_t { p + i, 1 } );
				++i;
			}
			else if ( c=='\"' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::BACKTICK:
			if ( c=='`' )
			{
				if ( i+1<iLen && p[i+1]=='`' )
				{
					sSql.AppendRawChunk ( Str_t { p + i, 1 } );
					++i;
				}
				else
					eState = PreparedScanState_e::NORMAL;
			}
			break;

		case PreparedScanState_e::LINE_COMMENT:
			if ( c=='\n' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::BLOCK_COMMENT:
			if ( c=='*' && i+1<iLen && p[i+1]=='/' )
			{
				++i;
				eState = PreparedScanState_e::NORMAL;
			}
			break;
		}

		sSql.AppendRawChunk ( Str_t { p + i, 1 } );
	}

	sOut = sSql.cstr();
	return true;
}

static bool ReplacePreparedParams ( Str_t tQuery, CSphString & sOut, const std::function<CSphString(int)> & fnRepl, CSphString & sError )
{
	const char * p = tQuery.first;
	int iLen = (int)tQuery.second;
	StringBuilder_c sSql;
	auto eState = PreparedScanState_e::NORMAL;
	int iParam = 0;

	for ( int i = 0; i < iLen; ++i )
	{
		char c = p[i];
		switch ( eState )
		{
		case PreparedScanState_e::NORMAL:
			if ( c=='\'' )
				eState = PreparedScanState_e::SINGLE_QUOTE;
			else if ( c=='\"' )
				eState = PreparedScanState_e::DOUBLE_QUOTE;
			else if ( c=='`' )
				eState = PreparedScanState_e::BACKTICK;
			else if ( IsLineCommentStart ( p, i, iLen ) )
				eState = PreparedScanState_e::LINE_COMMENT;
			else if ( c=='/' && i+1<iLen && p[i+1]=='*' )
				eState = PreparedScanState_e::BLOCK_COMMENT;
			else if ( c=='?' )
			{
				CSphString sVal = fnRepl ( iParam++ );
				sSql << sVal.cstr();
				continue;
			}
			break;

		case PreparedScanState_e::SINGLE_QUOTE:
			if ( c=='\\' && i+1<iLen )
			{
				sSql.AppendRawChunk ( Str_t { p + i, 1 } );
				++i;
			}
			else if ( c=='\'' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::DOUBLE_QUOTE:
			if ( c=='\\' && i+1<iLen )
			{
				sSql.AppendRawChunk ( Str_t { p + i, 1 } );
				++i;
			}
			else if ( c=='\"' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::BACKTICK:
			if ( c=='`' )
			{
				if ( i+1<iLen && p[i+1]=='`' )
				{
					sSql.AppendRawChunk ( Str_t { p + i, 1 } );
					++i;
				}
				else
					eState = PreparedScanState_e::NORMAL;
			}
			break;

		case PreparedScanState_e::LINE_COMMENT:
			if ( c=='\n' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::BLOCK_COMMENT:
			if ( c=='*' && i+1<iLen && p[i+1]=='/' )
			{
				++i;
				eState = PreparedScanState_e::NORMAL;
			}
			break;
		}

		sSql.AppendRawChunk ( Str_t { p + i, 1 } );
	}

	sOut = sSql.cstr();
	return true;
}

static bool ParseMarkerIndex ( const CSphString & sVal, const char * sPrefix, int & iIdx )
{
	if ( !sVal.Begins ( sPrefix ) )
		return false;
	const char * pNum = sVal.cstr() + strlen ( sPrefix );
	if ( !*pNum )
		return false;
	char * pEnd = nullptr;
	long iVal = strtol ( pNum, &pEnd, 10 );
	if ( !pEnd || *pEnd )
		return false;
	iIdx = (int)iVal;
	return true;
}

static void BuildDefaultInsertSchema ( const CSphSchema & tSchema, StrVec_t & dOut )
{
	dOut.Reset();
	dOut.Add ( tSchema.GetAttr(0).m_sName );
	for ( int i = 0; i < tSchema.GetFieldsCount(); ++i )
		dOut.Add ( tSchema.GetField(i).m_sName );
	for ( int i = 1; i < tSchema.GetAttrsCount(); ++i )
	{
		const auto & tAttr = tSchema.GetAttr(i);
		if ( sphIsInternalAttr ( tAttr ) )
			continue;
		if ( tAttr.m_sName==sphGetBlobLocatorName() )
			continue;
		if ( tSchema.GetFieldIndex ( tAttr.m_sName.cstr() )!=-1 )
			continue;
		dOut.Add ( tAttr.m_sName );
	}
}

static void ApplyImplicitInsertTypes ( const SqlStmt_t & tStmt, const CSphSchema & tSchema, const char * sPrefix, CSphVector<bool> & dRawParams )
{
	StrVec_t dCols;
	if ( tStmt.m_dInsertSchema.GetLength() )
		dCols = tStmt.m_dInsertSchema;
	else
		BuildDefaultInsertSchema ( tSchema, dCols );

	const int iCols = dCols.GetLength();
	if ( iCols<=0 )
		return;

	for ( int i = 0; i < tStmt.m_dInsertValues.GetLength(); ++i )
	{
		const auto & tVal = tStmt.m_dInsertValues[i];
		if ( tVal.m_iType!=SqlInsert_t::QUOTED_STRING )
			continue;
		int iParam = -1;
		if ( !ParseMarkerIndex ( tVal.m_sVal, sPrefix, iParam ) )
			continue;
		const int iCol = i % iCols;
		if ( iCol<0 || iCol>=dCols.GetLength() || iParam<0 || iParam>=dRawParams.GetLength() )
			continue;
		const auto * pAttr = tSchema.GetAttr ( dCols[iCol].cstr() );
		if ( !pAttr )
			continue;
		if ( pAttr->m_eAttrType==SPH_ATTR_UINT32SET || pAttr->m_eAttrType==SPH_ATTR_INT64SET || pAttr->m_eAttrType==SPH_ATTR_FLOAT_VECTOR )
			dRawParams[iParam] = true;
	}
}

static bool ExtractUpdateParamColumns ( Str_t tQuery, CSphString & sTable, CSphVector<std::pair<int, CSphString>> & dParamCols )
{
	const char * p = tQuery.first;
	int iLen = (int)tQuery.second;
	auto eState = PreparedScanState_e::NORMAL;
	int iParam = 0;
	bool bAfterUpdate = false;
	bool bInSet = false;
	bool bExpectValue = false;
	CSphString sCurCol;

	for ( int i = 0; i < iLen; ++i )
	{
		char c = p[i];
		switch ( eState )
		{
		case PreparedScanState_e::NORMAL:
			if ( c=='\'' )
				eState = PreparedScanState_e::SINGLE_QUOTE;
			else if ( c=='\"' )
				eState = PreparedScanState_e::DOUBLE_QUOTE;
			else if ( c=='`' )
				eState = PreparedScanState_e::BACKTICK;
			else if ( IsLineCommentStart ( p, i, iLen ) )
				eState = PreparedScanState_e::LINE_COMMENT;
			else if ( c=='/' && i+1<iLen && p[i+1]=='*' )
				eState = PreparedScanState_e::BLOCK_COMMENT;
			else if ( ( c>='A' && c<='Z' ) || ( c>='a' && c<='z' ) || c=='_' )
			{
				int iStart = i;
				int iEnd = i + 1;
				while ( iEnd<iLen )
				{
					char cc = p[iEnd];
					if ( ( cc>='A' && cc<='Z' ) || ( cc>='a' && cc<='z' ) || ( cc>='0' && cc<='9' ) || cc=='_' || cc=='.' )
						++iEnd;
					else
						break;
				}
				const char * sTok = p + iStart;
				const int iTokLen = iEnd - iStart;
				if ( TokenEq ( sTok, iTokLen, "update" ) )
				{
					bAfterUpdate = true;
				} else if ( bAfterUpdate && sTable.IsEmpty() )
				{
					sTable.SetBinary ( sTok, iTokLen );
					sTable.ToLower();
					bAfterUpdate = false;
				} else if ( TokenEq ( sTok, iTokLen, "set" ) )
				{
					bInSet = true;
					bExpectValue = false;
					sCurCol = "";
				} else if ( bInSet && TokenEq ( sTok, iTokLen, "where" ) )
				{
					bInSet = false;
					bExpectValue = false;
					sCurCol = "";
				} else if ( bInSet )
				{
					sCurCol.SetBinary ( sTok, iTokLen );
					sCurCol.ToLower();
				}
				i = iEnd - 1;
				continue;
			}
			else if ( c=='?' )
			{
				if ( bInSet && bExpectValue && !sCurCol.IsEmpty() )
					dParamCols.Add ( { iParam, sCurCol } );
				++iParam;
				bExpectValue = false;
				continue;
			}
			else if ( c=='=' )
			{
				if ( bInSet && !sCurCol.IsEmpty() )
					bExpectValue = true;
			}
			else if ( c==',' )
			{
				if ( bInSet )
				{
					sCurCol = "";
					bExpectValue = false;
				}
			}
			break;

		case PreparedScanState_e::SINGLE_QUOTE:
			if ( c=='\\' && i+1<iLen )
				++i;
			else if ( c=='\'' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::DOUBLE_QUOTE:
			if ( c=='\\' && i+1<iLen )
				++i;
			else if ( c=='\"' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::BACKTICK:
			if ( c=='`' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::LINE_COMMENT:
			if ( c=='\n' )
				eState = PreparedScanState_e::NORMAL;
			break;

		case PreparedScanState_e::BLOCK_COMMENT:
			if ( c=='*' && i+1<iLen && p[i+1]=='/' )
			{
				++i;
				eState = PreparedScanState_e::NORMAL;
			}
			break;
		}
	}

	return !sTable.IsEmpty();
}

struct PreparedColumnDef_t
{
	CSphString m_sName;
	MysqlColumnType_e m_eType = MYSQL_COL_STRING;
};

static const CSphSchema * GetPreparedSchema ( const CSphQuery & tQuery )
{
	StrVec_t dIndexes;
	ParseIndexList ( tQuery.m_sIndexes, dIndexes );
	for ( const auto & sIndex : dIndexes )
	{
		auto pServed = GetServed ( sIndex );
		if ( pServed )
			return &RIdx_c ( pServed )->GetMatchSchema();
	}
	return nullptr;
}

static MysqlColumnType_e GuessPreparedColumnType ( const ISphSchema * pSchema, const CSphQueryItem & tItem, const CSphQuery & tQuery )
{
	if ( !pSchema )
		return MYSQL_COL_STRING;

	if ( const auto * pAttr = pSchema->GetAttr ( tItem.m_sExpr.cstr() ) )
		return ESphAttr2MysqlColumn ( pAttr->m_eAttrType );

	if ( pSchema->GetField ( tItem.m_sExpr.cstr() ) )
		return MYSQL_COL_STRING;

	CSphString sError;
	ESphAttr eAttrType = SPH_ATTR_STRING;
	ExprParseArgs_t tExprArgs;
	tExprArgs.m_pAttrType = &eAttrType;
	const CSphString * pJoinIdx = tQuery.m_sJoinIdx.IsEmpty() ? nullptr : &tQuery.m_sJoinIdx;
	ISphExprRefPtr_c pExpr { sphExprParse ( tItem.m_sExpr.cstr(), *pSchema, pJoinIdx, sError, tExprArgs ) };
	if ( pExpr )
		return ESphAttr2MysqlColumn ( eAttrType );

	return MYSQL_COL_STRING;
}

static void BuildPreparedColumnDefs ( const SqlStmt_t & tStmt, CSphVector<PreparedColumnDef_t> & dColumns )
{
	if ( tStmt.m_eStmt != STMT_SELECT )
		return;

	const CSphSchema * pSchema = GetPreparedSchema ( tStmt.m_tQuery );
	for ( const auto & tItem : tStmt.m_tQuery.m_dItems )
	{
		PreparedColumnDef_t tCol;
		tCol.m_sName = tItem.m_sAlias.IsEmpty() ? tItem.m_sExpr : tItem.m_sAlias;
		tCol.m_eType = GuessPreparedColumnType ( pSchema, tItem, tStmt.m_tQuery );
		dColumns.Add ( std::move ( tCol ) );
	}
}

static CSphString BuildSqlStringLiteral ( Str_t sVal )
{
	EscapedStringBuilder_T<BaseQuotation_T<SqlQuotator_t>> sEscaped;
	const char * pData = sVal.first ? sVal.first : "";
	sEscaped.AppendEscaped ( pData, EscBld::eEscape | EscBld::eSkipComma, (int)sVal.second );
	return CSphString ( sEscaped.cstr() );
}

static int64_t ReadSignedLE ( InputBuffer_c & tIn, int iBytes )
{
	uint64_t uVal = 0;
	for ( int i = 0; i < iBytes; ++i )
		uVal |= uint64_t ( tIn.GetByte() ) << ( i * 8 );

	uint64_t uSignBit = uint64_t ( 1 ) << ( iBytes * 8 - 1 );
	if ( uVal & uSignBit )
	{
		uint64_t uMask = ~uint64_t ( 0 ) << ( iBytes * 8 );
		uVal |= uMask;
	}
	return (int64_t)uVal;
}

static uint64_t ReadUnsignedLE ( InputBuffer_c & tIn, int iBytes )
{
	uint64_t uVal = 0;
	for ( int i = 0; i < iBytes; ++i )
		uVal |= uint64_t ( tIn.GetByte() ) << ( i * 8 );
	return uVal;
}

static CSphString FormatDateLiteral ( int iYear, int iMonth, int iDay )
{
	CSphString sVal;
	sVal.SetSprintf ( "'%04d-%02d-%02d'", iYear, iMonth, iDay );
	return sVal;
}

static CSphString FormatDateTimeLiteral ( int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond, int iMicrosec )
{
	CSphString sVal;
	if ( iMicrosec>0 )
		sVal.SetSprintf ( "'%04d-%02d-%02d %02d:%02d:%02d.%06d'", iYear, iMonth, iDay, iHour, iMinute, iSecond, iMicrosec );
	else
		sVal.SetSprintf ( "'%04d-%02d-%02d %02d:%02d:%02d'", iYear, iMonth, iDay, iHour, iMinute, iSecond );
	return sVal;
}

static CSphString FormatTimeLiteral ( bool bNegative, int iDays, int iHour, int iMinute, int iSecond, int iMicrosec )
{
	int iTotalHours = iDays * 24 + iHour;
	CSphString sVal;
	if ( iMicrosec>0 )
		sVal.SetSprintf ( "'%s%02d:%02d:%02d.%06d'", bNegative ? "-" : "", iTotalHours, iMinute, iSecond, iMicrosec );
	else
		sVal.SetSprintf ( "'%s%02d:%02d:%02d'", bNegative ? "-" : "", iTotalHours, iMinute, iSecond );
	return sVal;
}

static bool ParseStmtParamValues ( InputBuffer_c & tIn, PreparedStmt_t & tStmt, CSphVector<CSphString> & dParams, CSphString & sError )
{
	const int iParamCount = tStmt.m_iParamCount;
	dParams.Resize ( iParamCount );
	if ( iParamCount==0 )
		return true;

	const int iNullBytes = ( iParamCount + 7 ) / 8;
	CSphVector<BYTE> dNull;
	dNull.Resize ( iNullBytes );
	if ( iNullBytes )
		tIn.GetBytes ( dNull.Begin(), iNullBytes );

	BYTE bNewParams = tIn.GetByte();
	if ( bNewParams )
	{
		for ( int i = 0; i < iParamCount; ++i )
			tStmt.m_dParamTypes[i] = MysqlReadLSBWord ( tIn );
	}

	for ( int i = 0; i < iParamCount; ++i )
	{
		bool bNull = ( dNull[i/8] >> ( i % 8 ) ) & 1;
		if ( bNull )
		{
			if ( i < tStmt.m_dLongData.GetLength() && !tStmt.m_dLongData[i].IsEmpty() )
			{
				Str_t tVal ( tStmt.m_dLongData[i].cstr(), tStmt.m_dLongData[i].Length() );
				dParams[i] = BuildSqlStringLiteral ( tVal );
			}
			else
			{
				dParams[i] = "NULL";
			}
			continue;
		}

		if ( i >= tStmt.m_dParamTypes.GetLength() || !tStmt.m_dParamTypes[i] )
		{
			sError.SetSprintf ( "no parameter type for index %d", i );
			return false;
		}

		WORD uType = tStmt.m_dParamTypes[i];
		BYTE uKind = BYTE ( uType & 0xFF );
		BYTE uFlags = BYTE ( uType >> 8 );
		bool bUnsigned = ( uFlags & 0x80 )!=0;
		CSphString sLiteral;

		switch ( uKind )
		{
		case MYSQL_TYPE_NULL:
			sLiteral = "NULL";
			break;

		case MYSQL_TYPE_TINY:
			sLiteral.SetSprintf ( "%d", (int)ReadSignedLE ( tIn, 1 ) );
			break;

		case MYSQL_TYPE_SHORT:
			sLiteral.SetSprintf ( "%d", (int)ReadSignedLE ( tIn, 2 ) );
			break;

		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_INT24:
			if ( bUnsigned )
				sLiteral.SetSprintf ( "%u", (unsigned)ReadUnsignedLE ( tIn, 4 ) );
			else
				sLiteral.SetSprintf ( "%d", (int)ReadSignedLE ( tIn, 4 ) );
			break;

		case MYSQL_TYPE_LONGLONG:
			if ( bUnsigned )
			{
				StringBuilder_c sVal;
				sVal << (unsigned long long) ReadUnsignedLE ( tIn, 8 );
				sLiteral = sVal.cstr();
			}
			else
			{
				StringBuilder_c sVal;
				sVal << (long long) ReadSignedLE ( tIn, 8 );
				sLiteral = sVal.cstr();
			}
			break;

		case MYSQL_TYPE_FLOAT:
			{
				DWORD uVal = MysqlReadLSBDword ( tIn );
				float fVal = sphDW2F ( uVal );
				StringBuilder_c sVal;
				sVal << fVal;
				sLiteral = sVal.cstr();
			}
			break;

		case MYSQL_TYPE_DOUBLE:
			{
				uint64_t uVal = MysqlReadLSBQword ( tIn );
				double fVal = sphQW2D ( uVal );
				StringBuilder_c sVal;
				sVal << fVal;
				sLiteral = sVal.cstr();
			}
			break;

		case MYSQL_TYPE_DATE:
		{
			BYTE uLen = tIn.GetByte();
			if ( !uLen )
				sLiteral = FormatDateLiteral ( 0, 0, 0 );
			else
			{
				int iYear = MysqlReadLSBWord ( tIn );
				int iMonth = tIn.GetByte();
				int iDay = tIn.GetByte();
				sLiteral = FormatDateLiteral ( iYear, iMonth, iDay );
			}
			break;
		}

		case MYSQL_TYPE_DATETIME:
		case MYSQL_TYPE_TIMESTAMP:
		{
			BYTE uLen = tIn.GetByte();
			if ( !uLen )
			{
				sLiteral = FormatDateTimeLiteral ( 0, 0, 0, 0, 0, 0, 0 );
			} else
			{
				int iYear = MysqlReadLSBWord ( tIn );
				int iMonth = tIn.GetByte();
				int iDay = tIn.GetByte();
				int iHour = 0;
				int iMinute = 0;
				int iSecond = 0;
				int iMicrosec = 0;
				if ( uLen>=7 )
				{
					iHour = tIn.GetByte();
					iMinute = tIn.GetByte();
					iSecond = tIn.GetByte();
				}
				if ( uLen>=11 )
					iMicrosec = (int)MysqlReadLSBDword ( tIn );
				sLiteral = FormatDateTimeLiteral ( iYear, iMonth, iDay, iHour, iMinute, iSecond, iMicrosec );
			}
			break;
		}

		case MYSQL_TYPE_TIME:
		{
			BYTE uLen = tIn.GetByte();
			if ( !uLen )
			{
				sLiteral = FormatTimeLiteral ( false, 0, 0, 0, 0, 0 );
			} else
			{
				bool bNegative = tIn.GetByte()!=0;
				int iDays = (int)MysqlReadLSBDword ( tIn );
				int iHour = tIn.GetByte();
				int iMinute = tIn.GetByte();
				int iSecond = tIn.GetByte();
				int iMicrosec = 0;
				if ( uLen>=12 )
					iMicrosec = (int)MysqlReadLSBDword ( tIn );
				sLiteral = FormatTimeLiteral ( bNegative, iDays, iHour, iMinute, iSecond, iMicrosec );
			}
			break;
		}

		case MYSQL_TYPE_STRING:
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_VARCHAR:
		case MYSQL_TYPE_BLOB:
		case MYSQL_TYPE_TINY_BLOB:
		case MYSQL_TYPE_MEDIUM_BLOB:
		case MYSQL_TYPE_LONG_BLOB:
		case MYSQL_TYPE_JSON:
		case MYSQL_TYPE_DECIMAL:
		case MYSQL_TYPE_NEWDECIMAL:
		case MYSQL_TYPE_SET:
		case MYSQL_TYPE_ENUM:
		{
			auto iLen = MysqlReadPackedInt ( tIn );
			auto sVal = tIn.GetRawString ( iLen );
			Str_t tVal ( sVal.cstr(), sVal.Length() );
			sLiteral = BuildSqlStringLiteral ( tVal );
			break;
		}

		case MYSQL_TYPE_BIT:
		{
			auto iLen = MysqlReadPackedInt ( tIn );
			uint64_t uVal = 0;
			for ( int j = 0; j < iLen; ++j )
				uVal = ( uVal << 8 ) | tIn.GetByte();
			StringBuilder_c sVal;
			sVal << (unsigned long long) uVal;
			sLiteral = sVal.cstr();
			break;
		}

		default:
		{
			auto iLen = MysqlReadPackedInt ( tIn );
			auto sVal = tIn.GetRawString ( iLen );
			Str_t tVal ( sVal.cstr(), sVal.Length() );
			sLiteral = BuildSqlStringLiteral ( tVal );
			break;
		}
		}

		if ( i < tStmt.m_dLongData.GetLength() && !tStmt.m_dLongData[i].IsEmpty() )
		{
			StringBuilder_c sCombined;
			sCombined.AppendRawChunk ( Str_t { tStmt.m_dLongData[i].cstr(), tStmt.m_dLongData[i].Length() } );
			sCombined.AppendRawChunk ( Str_t { sLiteral.cstr(), sLiteral.Length() } );
			Str_t tVal ( sCombined.cstr(), (int)sCombined.GetLength() );
			sLiteral = BuildSqlStringLiteral ( tVal );
		}

		dParams[i] = sLiteral;
	}

	for ( auto & sBlob : tStmt.m_dLongData )
		sBlob = "";

	return true;
}

// our copy of enum_field_types
// we can't rely on mysql_com.h because it might be unavailable
//
// MYSQL_TYPE_DECIMAL = 0
// MYSQL_TYPE_TINY = 1
// MYSQL_TYPE_SHORT = 2
// MYSQL_TYPE_LONG = 3
// MYSQL_TYPE_FLOAT = 4
// MYSQL_TYPE_DOUBLE = 5
// MYSQL_TYPE_NULL = 6
// MYSQL_TYPE_TIMESTAMP = 7
// MYSQL_TYPE_LONGLONG = 8
// MYSQL_TYPE_INT24 = 9
// MYSQL_TYPE_DATE = 10
// MYSQL_TYPE_TIME = 11
// MYSQL_TYPE_DATETIME = 12
// MYSQL_TYPE_YEAR = 13
// MYSQL_TYPE_NEWDATE = 14
// MYSQL_TYPE_VARCHAR = 15
// MYSQL_TYPE_BIT = 16
// MYSQL_TYPE_NEWDECIMAL = 246
// MYSQL_TYPE_ENUM = 247
// MYSQL_TYPE_SET = 248
// MYSQL_TYPE_TINY_BLOB = 249
// MYSQL_TYPE_MEDIUM_BLOB = 250
// MYSQL_TYPE_LONG_BLOB = 251
// MYSQL_TYPE_BLOB = 252
// MYSQL_TYPE_VAR_STRING = 253
// MYSQL_TYPE_STRING = 254
// MYSQL_TYPE_GEOMETRY = 255

struct MYSQL_ERROR
{
	static constexpr int MAX_LENGTH = 512;
};

// our copy of enum_server_command
// we can't rely on mysql_com.h because it might be unavailable
//
// MYSQL_COM_SLEEP = 0
// MYSQL_COM_QUIT = 1
// MYSQL_COM_INIT_DB = 2
// MYSQL_COM_QUERY = 3
// MYSQL_COM_FIELD_LIST = 4
// MYSQL_COM_CREATE_DB = 5
// MYSQL_COM_DROP_DB = 6
// MYSQL_COM_REFRESH = 7
// MYSQL_COM_SHUTDOWN = 8
// MYSQL_COM_STATISTICS = 9
// MYSQL_COM_PROCESS_INFO = 10
// MYSQL_COM_CONNECT = 11
// MYSQL_COM_PROCESS_KILL = 12
// MYSQL_COM_DEBUG = 13
// MYSQL_COM_PING = 14
// MYSQL_COM_TIME = 15
// MYSQL_COM_DELAYED_INSERT = 16
// MYSQL_COM_CHANGE_USER = 17
// MYSQL_COM_BINLOG_DUMP = 18
// MYSQL_COM_TABLE_DUMP = 19
// MYSQL_COM_CONNECT_OUT = 20
// MYSQL_COM_REGISTER_SLAVE = 21
// MYSQL_COM_STMT_PREPARE = 22
// MYSQL_COM_STMT_EXECUTE = 23
// MYSQL_COM_STMT_SEND_LONG_DATA = 24
// MYSQL_COM_STMT_CLOSE = 25
// MYSQL_COM_STMT_RESET = 26
// MYSQL_COM_SET_OPTION = 27
// MYSQL_COM_STMT_FETCH = 28

enum
{
	MYSQL_COM_QUIT		= 1,
	MYSQL_COM_INIT_DB	= 2,
	MYSQL_COM_QUERY		= 3,
	MYSQL_COM_FIELD_LIST = 4,
	MYSQL_COM_STATISTICS = 9,
	MYSQL_COM_PING		= 14,
	MYSQL_COM_STMT_PREPARE = 22,
	MYSQL_COM_STMT_EXECUTE = 23,
	MYSQL_COM_STMT_SEND_LONG_DATA = 24,
	MYSQL_COM_STMT_CLOSE = 25,
	MYSQL_COM_STMT_RESET = 26,
	MYSQL_COM_SET_OPTION	= 27,
	MYSQL_COM_STMT_FETCH = 28
};

void SendMysqlErrorPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, Str_t sError, EMYSQL_ERR eErr )
{
	if ( IsEmpty ( sError ) )
		sError = FROMS("(null)");

	// cut the error message to fix issue with long message for popular clients
	if ( sError.second>MYSQL_ERROR::MAX_LENGTH )
	{
		sError.second = MYSQL_ERROR::MAX_LENGTH;
		char * sErr = const_cast<char *>( sError.first );
		sErr[sError.second-3] = '.';
		sErr[sError.second-2] = '.';
		sErr[sError.second-1] = '.';
		sErr[sError.second] = '\0';
	}
	auto uError = (WORD)eErr; // pretend to be mysql syntax error for now

	// send packet header
	SQLPacketHeader_c tHdr { tOut, uPacketID };
	tOut.SendByte ( 0xff ); // field count, always 0xff for error packet
	tOut.SendByte ( (BYTE)( uError & 0xff ) );
	tOut.SendByte ( (BYTE)( uError>>8 ) );

	// send sqlstate (1 byte marker, 5 byte state)
	switch ( eErr )
	{
		case EMYSQL_ERR::SERVER_SHUTDOWN:
		case EMYSQL_ERR::UNKNOWN_COM_ERROR:
			tOut.SendBytes ( FROMS ( "#08S01" ) );
			break;
		case EMYSQL_ERR::NO_SUCH_TABLE:
			tOut.SendBytes ( FROMS ( "#42S02" ) );
			break;
		case EMYSQL_ERR::NO_SUCH_THREAD:
			tOut.SendBytes ( FROMS ( "#HY000" ) );
			break;
		default:
			tOut.SendBytes ( FROMS ( "#42000" ) );
			break;
	}

	// send error message
	tOut.SendBytes ( sError );
}

WORD MysqlStatus ( bool bMoreResults, bool bAutoCommit, bool bIsInTrans )
{
	WORD uStatus = 0;
	if ( bMoreResults )
			uStatus |= MYSQL_FLAG::MORE_RESULTS;
	if ( bAutoCommit )
			uStatus |= MYSQL_FLAG::STATUS_AUTOCOMMIT;
	if ( bIsInTrans )
			uStatus |= MYSQL_FLAG::STATUS_IN_TRANS;
	return uStatus;
}

/// https://dev.mysql.com/doc/dev/mysql-server/latest/page_protocol_basic_ok_packet.html
void SendMysqlOkPacketBody ( ISphOutputBuffer& tOut, int iAffectedRows, int iWarns, const char* szMessage, bool bMoreResults, bool bAutoCommit, bool bIsInTrans, int64_t iLastID )
{
	MysqlSendInt ( tOut, iAffectedRows );
	MysqlSendInt ( tOut, iLastID );

	// order of WORDs is opposite to EOF packet below
	tOut.SendLSBWord ( MysqlStatus ( bMoreResults, bAutoCommit, bIsInTrans ) );
	tOut.SendLSBWord ( iWarns < 0 ? 0 : ( iWarns > 65536 ? 65535 : iWarns ) );

	if ( !szMessage )
			return;

	auto iLen = (int)strlen ( szMessage );
	MysqlSendInt ( tOut, iLen );
	tOut.SendBytes ( szMessage, iLen );
}

void SendMysqlOkPacket ( ISphOutputBuffer& tOut, BYTE uPacketID, int iAffectedRows, int iWarns, const char* szMessage, bool bMoreResults, bool bAutoCommit, bool bIsInTrans, int64_t iLastID )
{
	SQLPacketHeader_c tHdr { tOut, uPacketID };
	tOut.SendByte ( 0 ); // ok packet
	SendMysqlOkPacketBody ( tOut, iAffectedRows, iWarns, szMessage, bMoreResults, bAutoCommit, bIsInTrans, iLastID );
}

void SendMysqlOkPacket ( ISphOutputBuffer& tOut, BYTE uPacketID, bool bAutoCommit, bool bIsInTrans )
{
	SendMysqlOkPacket ( tOut, uPacketID, 0, 0, nullptr, false, bAutoCommit, bIsInTrans, 0 );
}

void SendMysqlEofPacketRaw ( ISphOutputBuffer & tOut, BYTE uPacketID, int iWarns, bool bMoreResults, bool bAutoCommit, bool bIsInTrans );
/// https://dev.mysql.com/doc/dev/mysql-server/latest/page_protocol_basic_eof_packet.html
void SendMysqlEofPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, int iWarns, bool bMoreResults, bool bAutoCommit, bool bIsInTrans, const char* szMeta = nullptr )
{
	if ( OmitEof() )
	{
		if ( session::GetForceMetadataEof() )
			return SendMysqlEofPacketRaw ( tOut, uPacketID, iWarns, bMoreResults, bAutoCommit, bIsInTrans );
		SQLPacketHeader_c tHdr { tOut, uPacketID };
		tOut.SendByte ( 0x00 );
		return SendMysqlOkPacketBody ( tOut, 0, iWarns, szMeta, bMoreResults, bAutoCommit, bIsInTrans, 0 );
	}

	SQLPacketHeader_c tHdr { tOut, uPacketID };
	tOut.SendByte ( 0xfe );

	tOut.SendLSBWord ( iWarns < 0 ? 0 : ( iWarns > 65536 ? 65535 : iWarns ) );
	tOut.SendLSBWord ( MysqlStatus ( bMoreResults, bAutoCommit, bIsInTrans ) );
}

void SendMysqlEofPacketRaw ( ISphOutputBuffer & tOut, BYTE uPacketID, int iWarns, bool bMoreResults, bool bAutoCommit, bool bIsInTrans )
{
	SQLPacketHeader_c tHdr { tOut, uPacketID };
	tOut.SendByte ( 0xfe );
	tOut.SendLSBWord ( iWarns < 0 ? 0 : ( iWarns > 65536 ? 65535 : iWarns ) );
	tOut.SendLSBWord ( MysqlStatus ( bMoreResults, bAutoCommit, bIsInTrans ) );
}

void SendMysqlFieldPacket ( ISphOutputBuffer & tOut, BYTE & uPacketID, const char * szDB, const char * sTable, const char * sCol, MysqlColumnType_e eType )
{
	int iColLen = 0;
	WORD uFlags = 0;
	WORD uCollation = 0x2100;

	switch ( eType )
	{
	case MYSQL_COL_LONG: iColLen = 11; break;
	case MYSQL_COL_DECIMAL:
	case MYSQL_COL_FLOAT:
	case MYSQL_COL_DOUBLE:
	case MYSQL_COL_UINT64:
	case MYSQL_COL_LONGLONG: iColLen = 20; break;
	case MYSQL_COL_STRING:
	default:
		iColLen = 255;
		uCollation = 0x2100;
		break;
	}

	SQLPacketHeader_c dBlob { tOut, uPacketID++ };
	auto fnSendSqlString = [&tOut] ( const char * sStr )
	{
		auto iLen = sStr ? (int) strlen ( sStr ) : 0;
		MysqlSendInt ( tOut, iLen );
		tOut.SendBytes ( sStr, iLen );
	};

	fnSendSqlString ( "def" );
	fnSendSqlString ( szDB );
	fnSendSqlString ( sTable ? sTable : "" );
	fnSendSqlString ( sTable ? sTable : "" );
	fnSendSqlString ( sCol );
	fnSendSqlString ( sCol );

	tOut.SendByte ( 12 );
	tOut.SendWord ( uCollation );
	tOut.SendLSBDword ( iColLen );
	tOut.SendByte ( BYTE ( eType ) );
	tOut.SendWord ( uFlags );
	tOut.SendByte ( 0 );
	tOut.SendWord ( 0 );
}

void SendMysqlPrepareOkPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, DWORD uStmtId, WORD uColumns, WORD uParams, WORD uWarnings )
{
	SQLPacketHeader_c tHdr { tOut, uPacketID };
	tOut.SendByte ( 0x00 );
	tOut.SendLSBDword ( uStmtId );
	tOut.SendLSBWord ( uColumns );
	tOut.SendLSBWord ( uParams );
	tOut.SendByte ( 0x00 );
	tOut.SendLSBWord ( uWarnings );
}



//////////////////////////////////////////////////////////////////////////
// Mysql row buffer and command handler

class SqlRowBuffer_c final : public RowBuffer_i
{
	BYTE & m_uPacketID;
	GenericOutputBuffer_c & m_tOut;
	ClientSession_c* m_pSession = nullptr;
	size_t m_iTotalSent = 0;
	bool m_bWasFlushed = false;
	CSphVector<std::pair<CSphString, MysqlColumnType_e>> m_dHead;
	LazyVector_T<BYTE> m_tBuf {0};
	CSphString m_sTable;

	// how many bytes this string will occupy in proto mysql
	static int SqlStrlen ( const char * sStr )
	{
		auto iLen = ( int ) strlen ( sStr );
		return SqlSizeOf ( iLen ) + iLen;
	}

	void SendSqlInt ( int iVal )
	{
		MysqlSendInt ( m_tOut, iVal );
	}

	void SendSqlString ( const char * sStr )
	{
		auto iLen = sStr? (int) strlen ( sStr ) : 0;
		SendSqlInt ( iLen );
		m_tOut.SendBytes ( sStr, iLen );
	}

	bool SomethingWasSent() override
	{
		auto iPrevSent = std::exchange ( m_iTotalSent, m_tOut.GetTotalSent() + m_tOut.GetSentCount() + m_tBuf.GetLength() );
		return iPrevSent != m_iTotalSent;
	}

	void SendSqlFieldPacket ( const char * szDB, const char * sCol, MysqlColumnType_e eType, bool bFromFieldList )
	{
		const char * sTable = m_sTable.scstr();
		WORD uFlags = 0;

		int iColLen = 0;
		WORD uCollation = 0x2100; // utf8. Better to have 0x3f00, i.e. 'binary', but it breaks mysqldump
		switch ( eType )
		{
		case MYSQL_COL_LONG: iColLen = 11;
			break;
		case MYSQL_COL_DECIMAL:
		case MYSQL_COL_FLOAT:
		case MYSQL_COL_DOUBLE:
		case MYSQL_COL_UINT64:
		case MYSQL_COL_LONGLONG: iColLen = 20;
			break;
		case MYSQL_COL_STRING:
			iColLen = 255;
			uCollation = 0x2100; // utf8
			break;
		}

		SQLPacketHeader_c dBlob { m_tOut, m_uPacketID++ };
		SendSqlString ( "def" ); // catalog
		SendSqlString ( szDB ); // db
		SendSqlString ( sTable ); // table
		SendSqlString ( sTable ); // org_table
		SendSqlString ( sCol ); // name
		SendSqlString ( sCol ); // org_name

		m_tOut.SendByte ( 12 ); // filler, must be 12 (following pseudo-string length)
		m_tOut.SendWord ( uCollation ); // charset_nr, 0x21 is utf8
		m_tOut.SendLSBDword ( iColLen ); // length
		m_tOut.SendByte ( BYTE ( eType ) ); // type (0=decimal)
		m_tOut.SendWord ( uFlags );
		m_tOut.SendByte ( 0 ); // decimals
		m_tOut.SendWord ( 0 ); // filler
		if ( bFromFieldList )
			m_tOut.SendByte ( 0xFB );
	}

	bool IsAutoCommit() const
	{
		return !m_pSession || session::IsAutoCommit ( m_pSession );
	}

	bool IsInTrans () const
	{
		return m_pSession != nullptr && session::IsInTrans ( m_pSession );
	}

	template<typename NUM>
	void PutNumAsStringT ( NUM iVal )
	{
		m_tBuf.ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = (char*) m_tBuf.End();
#if __has_include ( <charconv> )
		int iLen = std::to_chars ( pSize + 1, pSize + SPH_MAX_NUMERIC_STR, iVal ).ptr - (pSize + 1);
#else
		int iLen = sph::NtoA ( pSize + 1, iVal );
#endif
		*pSize = BYTE ( iLen );
		m_tBuf.AddN ( iLen + 1 );
	}

public:
	SqlRowBuffer_c ( BYTE * pPacketID, GenericOutputBuffer_c * pOut )
		: m_uPacketID ( *pPacketID )
		, m_tOut ( *pOut )
		, m_pSession ( session::GetClientSession() )
	{}

	void PutFloatAsString ( float fVal, const char * sFormat ) override
	{
		m_tBuf.ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = m_tBuf.End();
		int iLen = sFormat
			? snprintf (( char* ) pSize + 1, SPH_MAX_NUMERIC_STR - 1, sFormat, fVal )
			: sph::PrintVarFloat (( char* ) pSize + 1, SPH_MAX_NUMERIC_STR - 1, fVal );
		*pSize = BYTE ( iLen );
		m_tBuf.AddN ( iLen + 1 );
	}

	void PutDoubleAsString ( double fVal, const char * szFormat ) override
	{
		m_tBuf.ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = m_tBuf.End();
		int iLen = szFormat
			? snprintf (( char* ) pSize + 1, SPH_MAX_NUMERIC_STR - 1, szFormat, fVal )
			: sph::PrintVarDouble (( char* ) pSize + 1, SPH_MAX_NUMERIC_STR - 1, fVal );
		*pSize = BYTE ( iLen );
		m_tBuf.AddN ( iLen + 1 );
	}

	void PutNumAsString ( int64_t iVal ) override
	{
		PutNumAsStringT ( iVal );
	}

	void PutNumAsString ( uint64_t uVal ) override
	{
		PutNumAsStringT ( uVal );
	}

	void PutNumAsString ( int iVal ) override
	{
		PutNumAsStringT ( iVal );
	}

	void PutNumAsString ( DWORD uVal ) override
	{
		PutNumAsStringT ( uVal );
	}

	// pack raw array (i.e. packed length, then blob) into proto mysql
	void PutArray ( const ByteBlob_t& dBlob, bool bSendEmpty ) override
	{
		if ( !IsValid ( dBlob ) )
			return;

		if ( ::IsEmpty ( dBlob ) && bSendEmpty )
		{
			PutNULL();
			return;
		}

		auto pSpace = m_tBuf.AddN ( dBlob.second + 9 ); // 9 is taken from MysqlPack() implementation (max possible offset)
		auto iNumLen = MysqlPackInt ( pSpace, dBlob.second );
		if ( dBlob.second )
			memcpy ( pSpace+iNumLen, dBlob.first, dBlob.second );
		m_tBuf.Resize ( m_tBuf.Idx ( pSpace ) + iNumLen + dBlob.second );
	}

	// pack string (or "")
	void PutString ( Str_t sMsg ) override
	{
		PutArray ( S2B ( sMsg ), false );
	}

	void PutMicrosec ( int64_t iUsec ) override
	{
		iUsec = Max ( iUsec, 0 );

		m_tBuf.ReserveGap ( SPH_MAX_NUMERIC_STR+1 );
		auto pSize = (char*) m_tBuf.End();
		int iLen = sph::IFtoA ( pSize + 1, iUsec, 6 );
		*pSize = BYTE ( iLen );
		m_tBuf.AddN ( iLen + 1 );
	}

	void PutNULL () override
	{
		Add ( 0xfb ); // MySQL NULL is 0xfb at VLB length
	}

public:
	/// more high level. Processing the whole tables.
	// sends collected data, then reset
	bool Commit() override
	{
		if ( m_bError )
			return false;

		int iLeft = m_tBuf.GetLength();
		const BYTE * pBuf = m_tBuf.Begin();
		while ( iLeft )
		{
			int iSize = Min ( iLeft, MAX_PACKET_LEN );
			{ // scope to write header BEFORE possible flush below
				SQLPacketHeader_c dBlob { m_tOut, m_uPacketID++ };
				m_tOut.SendBytes ( pBuf, iSize );
			}
			pBuf += iSize;
			iLeft -= iSize;
			if ( m_tOut.GetSentCount() > MAX_PACKET_LEN )
			{
				if ( !m_tOut.Flush() )
				{
					m_bError = true;
					return false;
				}
				m_bWasFlushed = true;
			}
		}
		m_tBuf.Resize(0);
		return true;
	}

	// wrappers for popular packets
	void Eof ( bool bMoreResults, int iWarns, const char* szMeta ) override
	{
		SendMysqlEofPacket ( m_tOut, m_uPacketID++, iWarns, bMoreResults, IsAutoCommit(), IsInTrans(), szMeta );
	}
	using RowBuffer_i::Eof;

	void Error ( const char * sError, EMYSQL_ERR eErr ) override
	{
		m_bError = true;
		m_sError = sError;
		SendMysqlErrorPacket ( m_tOut, m_uPacketID, FromSz(sError), eErr );
	}

	void Ok ( int iAffectedRows, int iWarns, const char * sMessage, bool bMoreResults, int64_t iLastInsertId ) override
	{
		SendMysqlOkPacket ( m_tOut, m_uPacketID, iAffectedRows, iWarns, sMessage, bMoreResults, IsAutoCommit(), IsInTrans(), iLastInsertId );
		if ( bMoreResults )
			m_uPacketID++;
	}

	void SendColumnDefinitions (bool bFromFieldList = false)
	{
		const char* szDB = session::GetCurrentDbName();
		if (!szDB)
			szDB = szManticore;
		for ( const auto & dCol: m_dHead )
			SendSqlFieldPacket ( szDB, dCol.first.cstr(), dCol.second, bFromFieldList );

		m_dHead.Reset();
	}

	// Header of the table with defined num of columns
	void HeadBegin () override
	{
		m_dHead.Reset();
	}

	void HeadBegin ( CSphString sTable )
	{
		m_sTable = std::move ( sTable );
		HeadBegin();
	}

	bool HeadEnd ( bool bMoreResults, int iWarns ) override
	{
		{
			SQLPacketHeader_c dHead { m_tOut, m_uPacketID++ };
			SendSqlInt ( m_dHead.GetLength() );
		}

		SendColumnDefinitions ();
		Eof ( bMoreResults, iWarns );

		return true;
	}

	// add the next column. The EOF after the tull set will be fired automatically
	void HeadColumn ( const char * sName, MysqlColumnType_e uType ) override
	{
		m_dHead.Add ( { sName, uType } );
	}

	void Add ( BYTE uVal ) override
	{
		m_tBuf.Add ( uVal );
	}

	[[nodiscard]] bool WasFlushed() const noexcept { return m_bWasFlushed; }

	[[nodiscard]] std::pair<int, BYTE> GetCurrentPositionState() noexcept
	{
		// we track flushes just for current position (that is - flushing invalidates position)
		m_bWasFlushed = false;
		return { m_tOut.GetSentCount(), m_uPacketID };
	};

	void ResetToPositionState ( std::pair<int, BYTE> tPoint )
	{
		assert ( !m_bWasFlushed && "Can't rewind already flushed stream!");

		// reset to initial state (as after ctr)
		Reset();
		m_tBuf.Reset();

		m_pSession = session::GetClientSession();
		m_iTotalSent = 0;
		m_bWasFlushed = false;

		// rewind stream and packetID
		assert ( !m_bError );
		m_tOut.Rewind ( tPoint.first );
		m_uPacketID = tPoint.second;
	}
};

class BinarySqlRowBuffer_c final : public RowBuffer_i
{
	BYTE & m_uPacketID;
	GenericOutputBuffer_c & m_tOut;
	ClientSession_c* m_pSession = nullptr;
	size_t m_iTotalSent = 0;
	bool m_bWasFlushed = false;
	CSphVector<std::pair<CSphString, MysqlColumnType_e>> m_dHead;
	CSphVector<MysqlColumnType_e> m_dTypes;
	LazyVector_T<BYTE> m_tRowBuf {0};
	CSphVector<BYTE> m_dNullBitmap;
	int m_iRowCol = 0;
	int m_iCols = 0;
	CSphString m_sTable;
	bool m_bForceStrings = true;

	void SendSqlInt ( int iVal )
	{
		MysqlSendInt ( m_tOut, iVal );
	}

	bool IsAutoCommit() const
	{
		return !m_pSession || session::IsAutoCommit ( m_pSession );
	}

	bool IsInTrans () const
	{
		return m_pSession != nullptr && session::IsInTrans ( m_pSession );
	}

	void ResetRow()
	{
		m_tRowBuf.Resize ( 0 );
		m_iRowCol = 0;
		m_dNullBitmap.Resize ( 0 );
		if ( m_iCols>0 )
		{
			int iNullBytes = ( m_iCols + 7 + 2 ) / 8;
			m_dNullBitmap.Resize ( iNullBytes );
			memset ( m_dNullBitmap.Begin(), 0, iNullBytes );
		}
	}

	void AddNullBit()
	{
		int iBit = m_iRowCol + 2;
		int iByte = iBit / 8;
		int iShift = iBit % 8;
		if ( iByte>=0 && iByte < m_dNullBitmap.GetLength() )
			m_dNullBitmap[iByte] |= BYTE ( 1 << iShift );
	}

	void AppendLengthEncoded ( const BYTE * pData, int iLen )
	{
		auto pSpace = m_tRowBuf.AddN ( iLen + 9 );
		auto iNumLen = MysqlPackInt ( pSpace, iLen );
		if ( iLen )
			memcpy ( pSpace + iNumLen, pData, iLen );
		m_tRowBuf.Resize ( m_tRowBuf.Idx ( pSpace ) + iNumLen + iLen );
	}

	void AppendInteger ( int64_t iVal, MysqlColumnType_e eType )
	{
		switch ( eType )
		{
		case MYSQL_COL_LONG:
			{
				auto pSpace = m_tRowBuf.AddN ( 4 );
				DWORD uVal = (DWORD)iVal;
#if USE_LITTLE_ENDIAN
				memcpy ( pSpace, &uVal, 4 );
#else
				pSpace[0] = BYTE ( uVal & 0xFF );
				pSpace[1] = BYTE ( ( uVal >> 8 ) & 0xFF );
				pSpace[2] = BYTE ( ( uVal >> 16 ) & 0xFF );
				pSpace[3] = BYTE ( ( uVal >> 24 ) & 0xFF );
#endif
			}
			break;
		case MYSQL_COL_LONGLONG:
		case MYSQL_COL_UINT64:
			{
				auto pSpace = m_tRowBuf.AddN ( 8 );
				uint64_t uVal = (uint64_t)iVal;
#if USE_LITTLE_ENDIAN
				memcpy ( pSpace, &uVal, 8 );
#else
				for ( int i = 0; i < 8; ++i )
					pSpace[i] = BYTE ( ( uVal >> ( i * 8 ) ) & 0xFF );
#endif
			}
			break;
		default:
			{
				StringBuilder_c sVal;
				sVal << iVal;
				AppendLengthEncoded ( (const BYTE*)sVal.cstr(), (int)strlen ( sVal.cstr() ) );
			}
			break;
		}
	}

	void AppendUnsigned64 ( uint64_t uVal )
	{
		auto pSpace = m_tRowBuf.AddN ( 8 );
#if USE_LITTLE_ENDIAN
		memcpy ( pSpace, &uVal, 8 );
#else
		for ( int i = 0; i < 8; ++i )
			pSpace[i] = BYTE ( ( uVal >> ( i * 8 ) ) & 0xFF );
#endif
	}

	void AppendFloat ( float fVal, MysqlColumnType_e eType )
	{
		if ( eType==MYSQL_COL_DOUBLE )
		{
			auto pSpace = m_tRowBuf.AddN ( 8 );
			double fDouble = (double)fVal;
#if USE_LITTLE_ENDIAN
			memcpy ( pSpace, &fDouble, 8 );
#else
			uint64_t uVal = sphD2QW ( fDouble );
			for ( int i = 0; i < 8; ++i )
				pSpace[i] = BYTE ( ( uVal >> ( i * 8 ) ) & 0xFF );
#endif
		} else
		{
			auto pSpace = m_tRowBuf.AddN ( 4 );
#if USE_LITTLE_ENDIAN
			memcpy ( pSpace, &fVal, 4 );
#else
			DWORD uVal = sphF2DW ( fVal );
			pSpace[0] = BYTE ( uVal & 0xFF );
			pSpace[1] = BYTE ( ( uVal >> 8 ) & 0xFF );
			pSpace[2] = BYTE ( ( uVal >> 16 ) & 0xFF );
			pSpace[3] = BYTE ( ( uVal >> 24 ) & 0xFF );
#endif
		}
	}

	void AppendDouble ( double fVal )
	{
		auto pSpace = m_tRowBuf.AddN ( 8 );
#if USE_LITTLE_ENDIAN
		memcpy ( pSpace, &fVal, 8 );
#else
		uint64_t uVal = sphD2QW ( fVal );
		for ( int i = 0; i < 8; ++i )
			pSpace[i] = BYTE ( ( uVal >> ( i * 8 ) ) & 0xFF );
#endif
	}

	void AppendString ( Str_t sMsg )
	{
		const BYTE * pData = (const BYTE*)sMsg.first;
		int iLen = (int)sMsg.second;
		if ( !pData )
			iLen = 0;
		AppendLengthEncoded ( pData, iLen );
	}

	void AdvanceCol()
	{
		++m_iRowCol;
	}

public:
	BinarySqlRowBuffer_c ( BYTE * pPacketID, GenericOutputBuffer_c * pOut )
		: m_uPacketID ( *pPacketID )
		, m_tOut ( *pOut )
		, m_pSession ( session::GetClientSession() )
	{
	}

	void SetForceStrings ( bool bForce ) noexcept
	{
		m_bForceStrings = bForce;
	}

	bool SomethingWasSent() override
	{
		auto iPrevSent = std::exchange ( m_iTotalSent, m_tOut.GetTotalSent() + m_tOut.GetSentCount() + m_tRowBuf.GetLength() );
		return iPrevSent != m_iTotalSent;
	}

	bool IsBinary() const override { return true; }

	void PutFloatAsString ( float fVal, const char * ) override
	{
		if ( m_bForceStrings )
		{
			StringBuilder_c sVal;
			sVal << fVal;
			AppendString ( (Str_t)sVal );
		} else if ( m_iRowCol < m_dTypes.GetLength() )
			AppendFloat ( fVal, m_dTypes[m_iRowCol] );
		AdvanceCol();
	}

	void PutDoubleAsString ( double fVal, const char * ) override
	{
		if ( m_bForceStrings )
		{
			StringBuilder_c sVal;
			sVal << fVal;
			AppendString ( (Str_t)sVal );
		} else
			AppendDouble ( fVal );
		AdvanceCol();
	}

	void PutNumAsString ( int64_t iVal ) override
	{
		if ( m_bForceStrings )
		{
			StringBuilder_c sVal;
			sVal << iVal;
			AppendString ( (Str_t)sVal );
		} else if ( m_iRowCol < m_dTypes.GetLength() )
			AppendInteger ( iVal, m_dTypes[m_iRowCol] );
		AdvanceCol();
	}

	void PutNumAsString ( uint64_t uVal ) override
	{
		if ( m_bForceStrings )
		{
			StringBuilder_c sVal;
			sVal << (unsigned long long) uVal;
			AppendString ( (Str_t)sVal );
		} else if ( m_iRowCol < m_dTypes.GetLength() )
		{
			auto eType = m_dTypes[m_iRowCol];
			if ( eType==MYSQL_COL_UINT64 )
				AppendUnsigned64 ( uVal );
			else
				AppendInteger ( (int64_t)uVal, eType );
		}
		AdvanceCol();
	}

	void PutNumAsString ( int iVal ) override
	{
		PutNumAsString ( (int64_t)iVal );
	}

	void PutNumAsString ( DWORD uVal ) override
	{
		PutNumAsString ( (uint64_t)uVal );
	}

	void PutArray ( const ByteBlob_t& dBlob, bool bSendEmpty ) override
	{
		if ( !IsValid ( dBlob ) )
			return;

		if ( ::IsEmpty ( dBlob ) && bSendEmpty )
		{
			PutNULL();
			return;
		}

		AppendString ( B2S ( dBlob ) );
		AdvanceCol();
	}

	void PutString ( Str_t sMsg ) override
	{
		AppendString ( sMsg );
		AdvanceCol();
	}

	void PutMicrosec ( int64_t iUsec ) override
	{
		StringBuilder_c sVal;
		sVal << iUsec;
		AppendString ( (Str_t)sVal );
		AdvanceCol();
	}

	void PutNULL () override
	{
		AddNullBit();
		AdvanceCol();
	}

	bool Commit() override
	{
		if ( m_bError )
			return false;

		LazyVector_T<BYTE> dRow {0};
		auto pRow = dRow.AddN ( 1 );
		*pRow = 0x00;
		if ( m_dNullBitmap.GetLength() )
		{
			auto pNulls = dRow.AddN ( m_dNullBitmap.GetLength() );
			memcpy ( pNulls, m_dNullBitmap.Begin(), m_dNullBitmap.GetLength() );
		}
		if ( m_tRowBuf.GetLength() )
		{
			auto pData = dRow.AddN ( m_tRowBuf.GetLength() );
			memcpy ( pData, m_tRowBuf.Begin(), m_tRowBuf.GetLength() );
		}

		int iLeft = dRow.GetLength();
		const BYTE * pBuf = dRow.Begin();
		while ( iLeft )
		{
			int iSize = Min ( iLeft, MAX_PACKET_LEN );
			{
				SQLPacketHeader_c dBlob { m_tOut, m_uPacketID++ };
				m_tOut.SendBytes ( pBuf, iSize );
			}
			pBuf += iSize;
			iLeft -= iSize;
			if ( m_tOut.GetSentCount() > MAX_PACKET_LEN )
			{
				if ( !m_tOut.Flush() )
				{
					m_bError = true;
					return false;
				}
				m_bWasFlushed = true;
			}
		}

		ResetRow();
		return true;
	}

	void Eof ( bool bMoreResults, int iWarns, const char* szMeta ) override
	{
		SendMysqlEofPacket ( m_tOut, m_uPacketID++, iWarns, bMoreResults, IsAutoCommit(), IsInTrans(), szMeta );
	}
	using RowBuffer_i::Eof;

	void Error ( const char * sError, EMYSQL_ERR eErr ) override
	{
		m_bError = true;
		m_sError = sError;
		SendMysqlErrorPacket ( m_tOut, m_uPacketID, FromSz(sError), eErr );
	}

	void Ok ( int iAffectedRows, int iWarns, const char * sMessage, bool bMoreResults, int64_t iLastInsertId ) override
	{
		SendMysqlOkPacket ( m_tOut, m_uPacketID, iAffectedRows, iWarns, sMessage, bMoreResults, IsAutoCommit(), IsInTrans(), iLastInsertId );
		if ( bMoreResults )
			m_uPacketID++;
	}

	void SendColumnDefinitions ()
	{
		const char* szDB = session::GetCurrentDbName();
		if ( !szDB )
			szDB = szManticore;
		for ( const auto & dCol: m_dHead )
			SendMysqlFieldPacket ( m_tOut, m_uPacketID, szDB, m_sTable.scstr(), dCol.first.cstr(), dCol.second );

		m_dHead.Reset();
	}

	void HeadBegin () override
	{
		m_dHead.Reset();
		m_dTypes.Reset();
	}

	void HeadBegin ( CSphString sTable )
	{
		m_sTable = std::move ( sTable );
		HeadBegin();
	}

	bool HeadEnd ( bool bMoreResults, int iWarns ) override
	{
		m_iCols = m_dTypes.GetLength();
		{
			SQLPacketHeader_c dHead { m_tOut, m_uPacketID++ };
			SendSqlInt ( m_iCols );
		}

		SendColumnDefinitions();
		Eof ( bMoreResults, iWarns );

		ResetRow();
		return true;
	}

	void HeadColumn ( const char * sName, MysqlColumnType_e uType ) override
	{
		auto eType = m_bForceStrings ? MYSQL_COL_STRING : uType;
		m_dHead.Add ( { sName, eType } );
		m_dTypes.Add ( eType );
	}

	void Add ( BYTE uVal ) override
	{
		m_tRowBuf.Add ( uVal );
	}

	[[nodiscard]] bool WasFlushed() const noexcept { return m_bWasFlushed; }

	[[nodiscard]] std::pair<int, BYTE> GetCurrentPositionState() noexcept
	{
		m_bWasFlushed = false;
		return { m_tOut.GetSentCount(), m_uPacketID };
	};

	void ResetToPositionState ( std::pair<int, BYTE> tPoint )
	{
		assert ( !m_bWasFlushed && "Can't rewind already flushed stream!" );

		Reset();
		m_tRowBuf.Reset();
		m_dNullBitmap.Reset();
		m_pSession = session::GetClientSession();
		m_iTotalSent = 0;
		m_bWasFlushed = false;

		assert ( !m_bError );
		m_tOut.Rewind ( tPoint.first );
		m_uPacketID = tPoint.second;
	}
};

struct CLIENT
{
	// see https://dev.mysql.com/doc/dev/mysql-server/latest/group__group__cs__capabilities__flags.html for reference
	// we use same non-consistent definitions to match the reference (i.e. some constants defined as decimals, some as (1UL << X). Just keep it for easier match with ref page).
	static constexpr DWORD CONNECT_WITH_DB = 8;
	static constexpr DWORD COMPRESS = 32;
	static constexpr DWORD PROTOCOL_41 = 512;
	static constexpr DWORD SSL = 2048;
//	static constexpr DWORD RESERVED = 16384; // DEPRECATED: Old flag for 4.1 protocol
	static constexpr DWORD RESERVED2 = 32768; // DEPRECATED: Old flag for 4.1 authentication \ CLIENT_SECURE_CONNECTION.
	static constexpr DWORD MULTI_RESULTS = ( 1UL << 17 );
//	static constexpr DWORD PS_MULTI_RESULTS = ( 1UL << 18 );
	static constexpr DWORD PLUGIN_AUTH = ( 1UL << 19 );
	static constexpr DWORD CONNECT_ATTRS = ( 1UL << 20 );
	static constexpr DWORD PLUGIN_AUTH_LENENC_CLIENT_DATA = ( 1UL << 21 );
	static constexpr DWORD DEPRECATE_EOF = ( 1UL << 24 );
	static constexpr DWORD ZSTD_COMPRESSION_ALGORITHM = ( 1UL << 26 );
};

// handshake package we send to client
class HandshakeV10_c
{
	static constexpr BYTE AUTH_DATA_LEN = 21;
	const BYTE m_uVersion = 0x0A; // protocol version 10
	const BYTE m_uCharSet = MYSQL_CHARSET::utf8_general_ci;
	const WORD m_uServerStatusFlag = MYSQL_FLAG::STATUS_AUTOCOMMIT;
	const Str_t m_sAuthPluginName { "mysql_native_password", 22 };

	Str_t m_sVersionString;
	DWORD m_uConnID;
	std::array<char, AUTH_DATA_LEN> m_sAuthData {};
	DWORD m_uCapabilities = CLIENT::CONNECT_WITH_DB
						| CLIENT::PROTOCOL_41
						| CLIENT::RESERVED2 // deprecated
//						| CLIENT::RESERVED
						| CLIENT::MULTI_RESULTS
						| CLIENT::PLUGIN_AUTH
						| CLIENT::CONNECT_ATTRS;

public:
	explicit HandshakeV10_c( DWORD uConnID )
		: m_uConnID ( uConnID )
	{
		static bool bExtraCapabilitiesSet = false;
		static WORD uExtraCapabilities = 0;
		if ( !bExtraCapabilitiesSet )
		{
			uExtraCapabilities = dwval_from_env ( "MANTICORE_MYSQL_EXTRA_CAPABILITIES", 0 );
			bExtraCapabilitiesSet = true;
		}
		m_uCapabilities |= uExtraCapabilities;

		// fill scramble auth data (random)
		DWORD i = 0;
		DWORD uRand = sphRand() | 0x01010101;
		for ( ; i < AUTH_DATA_LEN - sizeof ( DWORD ); i += sizeof ( DWORD ) )
		{
			memcpy ( m_sAuthData.data() + i, &uRand, sizeof ( DWORD ) );
			uRand = sphRand() | 0x01010101;
		}
		if ( i < AUTH_DATA_LEN )
			memcpy ( m_sAuthData.data() + i, &uRand, AUTH_DATA_LEN - i );
		memset ( m_sAuthData.data() + AUTH_DATA_LEN - 1, 0, 1);
		// version string (plus 0-terminator)
		m_sVersionString = FromStr ( g_sMySQLVersion );
		++m_sVersionString.second; // encount also z-terminator
	}

	void SetCanSsl ( bool bCan )
	{
		if ( bCan )
			m_uCapabilities |= CLIENT::SSL;
	}

	void SetCanZlib( bool bCan )
	{
		if ( bCan )
			m_uCapabilities |= CLIENT::COMPRESS;
	}

	void SetCanZstd ( bool bCan )
	{
		if ( bCan )
			m_uCapabilities |= CLIENT::ZSTD_COMPRESSION_ALGORITHM;
	}

	void Send ( ISphOutputBuffer& tOut )
	{
		// see https://dev.mysql.com/doc/dev/mysql-server/latest/page_protocol_connection_phase_packets_protocol_handshake_v10.html for reference

		constexpr int iFillerSize = 10;
		const std::array<BYTE, iFillerSize> dFiller { 0 };
		SQLPacketHeader_c tHeader { tOut };

		// Protocol::HandshakeV10
		tOut.SendByte ( m_uVersion );
		tOut.SendBytes ( m_sVersionString );
		tOut.SendLSBDword ( m_uConnID );
		tOut.SendBytes ( m_sAuthData.data(), 8 );
		tOut.SendByte ( 0 );
		tOut.SendLSBWord ( m_uCapabilities & 0xFFFF );
		tOut.SendByte ( m_uCharSet );
		tOut.SendLSBWord ( m_uServerStatusFlag );
		tOut.SendLSBWord ( m_uCapabilities >> 16 );
		tOut.SendByte ( AUTH_DATA_LEN );
		tOut.SendBytes ( dFiller.data(), iFillerSize );
		tOut.SendBytes ( &m_sAuthData[8], AUTH_DATA_LEN - 8 );
		tOut.SendBytes ( m_sAuthPluginName );
	}
};

// HandshakeResponse truncated right before username field
class HandshakeResponse41
{
	CSphString m_sLoginUserName;
	CSphString m_sAuthResponse;
	std::optional<CSphString> m_sDatabase;
	std::optional<CSphString> m_sClientPluginName;
	SmallStringHash_T<CSphString> m_hAttributes;
	DWORD m_uCapabilities;
	DWORD m_uMaxPacketSize;
	BYTE m_uCharset;
	BYTE m_uCompressionLevel = 0;

public:
	// see https://dev.mysql.com/doc/dev/mysql-server/latest/page_protocol_connection_phase_packets_protocol_handshake_response.html for ref
	explicit HandshakeResponse41 ( AsyncNetInputBuffer_c& tRawIn, int iPacketLen )
	{
		InputBuffer_c tIn { tRawIn.PopTail ( iPacketLen ) };
		m_uCapabilities = tIn.GetLSBDword();
		assert ( m_uCapabilities & CLIENT::PROTOCOL_41 );
		m_uMaxPacketSize = tIn.GetLSBDword();
		m_uCharset = tIn.GetByte();
		tIn.SetBufferPos ( tIn.GetBufferPos() + 23 );

		// ssl auth is finished here
		if ( tIn.HasBytes() <=0 )
			return;

		// login name
		m_sLoginUserName = MysqlReadSzStr ( tIn );

		// auth
		if ( m_uCapabilities & CLIENT::PLUGIN_AUTH_LENENC_CLIENT_DATA )
			m_sAuthResponse = MysqlReadVlStr ( tIn );
		else
		{
			auto uLen = tIn.GetByte();
			m_sAuthResponse = tIn.GetRawString ( uLen );
		}

		// db name
		if ( m_uCapabilities & CLIENT::CONNECT_WITH_DB )
		{
			m_sDatabase.emplace ( MysqlReadSzStr ( tIn ) );
		}

		// db name
		if ( m_uCapabilities & CLIENT::PLUGIN_AUTH )
			m_sClientPluginName.emplace ( MysqlReadSzStr ( tIn ) );

		// attributes
		if ( m_uCapabilities & CLIENT::CONNECT_ATTRS )
		{
			auto iWatermark = MysqlReadPackedInt ( tIn );
			iWatermark = tIn.HasBytes() - iWatermark;
			while ( iWatermark < tIn.HasBytes() )
			{
				auto sKey = MysqlReadVlStr ( tIn );
				auto sVal = MysqlReadVlStr ( tIn );
				m_hAttributes.Add ( std::move ( sVal ), sKey );
			}
		}

		// compression level
		if ( tIn.HasBytes()>0 )
			m_uCompressionLevel = tIn.GetByte();
	}

	[[nodiscard]] const CSphString& GetUsername() const noexcept
	{
		return m_sLoginUserName;
	}

	[[nodiscard]] const std::optional<CSphString>& GetDB() const noexcept
	{
		return m_sDatabase;
	}

	[[nodiscard]] const CSphString* GetAttr ( const char * szKey ) const noexcept
	{
		return m_hAttributes ( szKey );
	}

	[[nodiscard]] bool WantSSL() const noexcept
	{
		return ( m_uCapabilities & CLIENT::SSL ) != 0;
	}

	[[nodiscard]] bool WantZlib() const noexcept
	{
		return ( m_uCapabilities & CLIENT::COMPRESS ) != 0;
	}

	[[nodiscard]] bool WantZstd() const noexcept
	{
		return ( m_uCapabilities & CLIENT::ZSTD_COMPRESSION_ALGORITHM ) != 0;
	}

	[[nodiscard]] int WantZstdLev() const noexcept
	{
		return m_uCompressionLevel;
	}

	[[nodiscard]] bool DeprecateEOF() const noexcept
	{
		return ( m_uCapabilities & CLIENT::DEPRECATE_EOF ) != 0;
	}
};


void SendTableSchema ( SqlRowBuffer_c & tSqlOut, CSphString sName )
{
	auto pServed = GetServed ( sName );
	if ( !pServed )
	{
		return;
	}

	tSqlOut.HeadBegin(std::move(sName));

	// data
	const CSphSchema * pSchema = &RIdx_c ( pServed )->GetMatchSchema();
	const CSphSchema & tSchema = *pSchema;
	assert ( tSchema.GetAttr ( 0 ).m_sName == sphGetDocidName() );
	const auto & tId = tSchema.GetAttr ( 0 );

	tSqlOut.HeadColumn ( tId.m_sName.cstr(), ESphAttr2MysqlColumn ( tId.m_eAttrType ) );
	for ( int i = 0; i < tSchema.GetFieldsCount(); ++i )
	{
		const auto & tField = tSchema.GetField ( i );
		const CSphColumnInfo * pAttr = tSchema.GetAttr ( tField.m_sName.cstr() );
		if (pAttr)
			tSqlOut.HeadColumn ( pAttr->m_sName.cstr(), ESphAttr2MysqlColumn ( pAttr->m_eAttrType ) );
		else
			tSqlOut.HeadColumn ( tField.m_sName.cstr(), MYSQL_COL_STRING );
	}

	for ( int i = 1; i < tSchema.GetAttrsCount(); ++i ) // from 1, as 0 is docID and already emerged
	{
		const auto & tAttr = tSchema.GetAttr ( i );
		if ( sphIsInternalAttr ( tAttr ) )
			continue;

		if ( tSchema.GetField ( tAttr.m_sName.cstr() ) )
			continue; // already described it as a field property

		tSqlOut.HeadColumn ( tAttr.m_sName.cstr(), ESphAttr2MysqlColumn ( tAttr.m_eAttrType ) );
	}

	tSqlOut.SendColumnDefinitions (true); // true means - from field list
}

bool ValidateDBName (Str_t tSrcQueryReference)
{
	return StrEqN ( tSrcQueryReference, szManticore );
}

bool ValidateDBName ( const std::optional<CSphString>& tSrcQueryReference )
{
	return ValidateDBName ( FromStr ( tSrcQueryReference.value_or ( szManticore ) ) );
}


bool LoopClientMySQL ( BYTE & uPacketID, int iPacketLen, QueryProfile_c * pProfile, AsyncNetBuffer_c * pBuf )
{
	auto& tSess = session::Info();
	assert ( pBuf );
	auto& tIn = *(AsyncNetInputBuffer_c *) pBuf;
	auto& tOut = *(GenericOutputBuffer_c *) pBuf;

	auto uHasBytesIn = tIn.HasBytes ();
	// get command, handle special packets
	const BYTE uMysqlCmd = tIn.GetByte ();

	if ( uMysqlCmd!=MYSQL_COM_QUERY )
		sphLogDebugv ( "LoopClientMySQL command %d", uMysqlCmd );

	if ( uMysqlCmd==MYSQL_COM_QUIT )
		return false;

	bool bKeepProfile = true;
	switch ( uMysqlCmd )
	{
		// client wants a pong
		case MYSQL_COM_PING:
			SendMysqlOkPacket ( tOut, uPacketID, session::IsAutoCommit(), session::IsInTrans() );
			break;

		// handle 'use DB'
		case MYSQL_COM_INIT_DB:
		{
			Str_t tSrcQueryReference ( nullptr, iPacketLen - 1 );
			tIn.GetBytesZerocopy ( ( const BYTE ** )( &tSrcQueryReference.first ), tSrcQueryReference.second );
			CSphString sInitDB { tSrcQueryReference };
			sphLogDebugv ( "LoopClientMySQL command %d, COM_INIT_DB '%s'", uMysqlCmd, sInitDB.cstr() );
			if ( !ValidateDBName ( tSrcQueryReference ) )
			{
				StringBuilder_c sError;
				sError << "no such database " << tSrcQueryReference;
				LogSphinxqlError ( "", Str_t ( sError ) );
				SendMysqlErrorPacket ( tOut, uPacketID, Str_t(sError), EMYSQL_ERR::NO_DB_ERROR );
				break;
			}
			// commented, because it is 'Manticore' by default; no need to write another one
			// session::SetCurrentDbName ( { tSrcQueryReference } );
			SendMysqlOkPacket ( tOut, uPacketID, session::IsAutoCommit(), session::IsInTrans() );
			break;
		}

		case MYSQL_COM_SET_OPTION:
			// bMulti = ( tIn.GetWord()==MYSQL_OPTION_MULTI_STATEMENTS_ON ); // that's how we could double check and validate multi query
			// server reporting success in response to COM_SET_OPTION and COM_DEBUG
			SendMysqlEofPacket ( tOut, uPacketID, 0, false, session::IsAutoCommit (), session::IsInTrans() );
			break;

		case MYSQL_COM_STATISTICS:
		{
			StringBuilder_c sStats;
			BuildStatusOneline ( sStats );
			SQLPacketHeader_c dBlob { tOut, uPacketID };
			tOut.SendBytes ( sStats );
			break;
		}

		case MYSQL_COM_QUERY:
		{
			// handle query packet
			Str_t tSrcQueryReference ( nullptr, iPacketLen-1 );
			tIn.GetBytesZerocopy ( ( const BYTE ** )( &tSrcQueryReference.first ), tSrcQueryReference.second );
			CSphString sRawQuery ( tSrcQueryReference );
			sRawQuery.Trim();
			if ( sRawQuery.Ends ( ";" ) )
			{
				sRawQuery = sRawQuery.SubString ( 0, sRawQuery.Length() - 1 );
				sRawQuery.Trim();
			}
			CSphString sLowerQuery = sRawQuery;
			sLowerQuery.ToLower();
			if ( sLowerQuery.Begins ( "select" ) )
			{
				CSphString sTail = sLowerQuery.SubString ( 6, sLowerQuery.Length() - 6 );
				sTail.Trim();
				if ( !strcmp ( sTail.cstr(), "$$" ) )
				{
					SendMysqlErrorPacket ( tOut, uPacketID, FromSz ( "syntax error near '$$'" ), EMYSQL_ERR::PARSE_ERROR );
					break;
				}
			}

			// string created from the tSrcQueryReference data got moved into myinfo then could be changed during query parsing
			myinfo::SetDescription ( CSphString ( tSrcQueryReference ), tSrcQueryReference.second ); // OPTIMIZE? could be huge, but string is hazard.
			AT_SCOPE_EXIT ( []() { myinfo::SetDescription ( {}, 0 ); } );
			assert ( !tIn.GetError() );
			sphLogDebugv ( "LoopClientMySQL command %d, '%s'", uMysqlCmd, myinfo::UnsafeDescription().first );
			tSess.SetTaskState ( TaskState_e::QUERY );

			SqlRowBuffer_c tRows ( &uPacketID, &tOut );
			tSess.m_pSqlRowBuffer = &tRows;
			auto tStoredPos = tRows.GetCurrentPositionState();
			bKeepProfile = session::Execute ( myinfo::UnsafeDescription(), tRows );
			if ( tRows.IsError() )
			{
				if ( !HasBuddy() || tRows.WasFlushed() )
				{
					LogSphinxqlError ( myinfo::UnsafeDescription().first, FromStr ( tRows.GetError() ) );
					if ( tRows.WasFlushed() )
						sphLogDebug ( "Can't invoke buddy, because output socket was flushed; unable to rewind/overwrite anything" );
				} else
				{
					ProcessSqlQueryBuddy ( tSrcQueryReference, FromStr ( tRows.GetError() ), tStoredPos, uPacketID, tOut );
				}
			}
			break;
		}
		case MYSQL_COM_STMT_PREPARE:
		{
			Str_t tSrcQueryReference ( nullptr, iPacketLen-1 );
			tIn.GetBytesZerocopy ( ( const BYTE ** )( &tSrcQueryReference.first ), tSrcQueryReference.second );

			CSphString sTemplate ( tSrcQueryReference );
			int iParamCount = CountPreparedParams ( tSrcQueryReference );

			CSphVector<PreparedColumnDef_t> dColumns;
			int iColumnCount = 0;
			CSphString sDummyQuery = sTemplate;
			if ( iParamCount>0 )
			{
				CSphVector<CSphString> dDummy;
				dDummy.Resize ( iParamCount );
				for ( auto & sVal : dDummy )
					sVal = "0";
				CSphString sErr;
				if ( !RenderPreparedQuery ( tSrcQueryReference, dDummy, sDummyQuery, sErr, nullptr ) )
					sDummyQuery = sTemplate;
			}

			if ( !sDummyQuery.IsEmpty() )
			{
				CSphVector<SqlStmt_t> dStmt;
				CSphString sParseError;
				if ( sphParseSqlQuery ( FromStr ( sDummyQuery ), dStmt, sParseError, tSess.GetCollation () ) && dStmt.GetLength() )
				{
					const SqlStmt_t & tStmt = dStmt[0];
					if ( tStmt.m_eStmt==STMT_SELECT )
					{
						BuildPreparedColumnDefs ( tStmt, dColumns );
						iColumnCount = dColumns.GetLength();
					}
				}
			}

			auto * pSession = session::GetClientSession();
			auto * pStmt = pSession->CreatePreparedStmt ( sTemplate, iParamCount, iColumnCount );
			SendMysqlPrepareOkPacket ( tOut, uPacketID++, pStmt->m_uId, WORD(iColumnCount), WORD(iParamCount), 0 );

			const char* szDB = session::GetCurrentDbName();
			if ( !szDB )
				szDB = szManticore;

			if ( iParamCount>0 )
			{
				for ( int i = 0; i < iParamCount; ++i )
				{
					CSphString sName;
					sName.SetSprintf ( "param_%d", i+1 );
					SendMysqlFieldPacket ( tOut, uPacketID, szDB, "", sName.cstr(), MYSQL_COL_STRING );
				}
				if ( OmitEof() )
				{
					if ( session::GetForceMetadataEof() )
						SendMysqlEofPacketRaw ( tOut, uPacketID++, 0, false, session::IsAutoCommit (), session::IsInTrans() );
				} else
					SendMysqlEofPacket ( tOut, uPacketID++, 0, false, session::IsAutoCommit (), session::IsInTrans() );
			}

			if ( iColumnCount>0 )
			{
				for ( int i = 0; i < iColumnCount; ++i )
				{
					CSphString sName;
					MysqlColumnType_e eType = MYSQL_COL_STRING;
					if ( i < dColumns.GetLength() )
					{
						if ( !dColumns[i].m_sName.IsEmpty() )
							sName = dColumns[i].m_sName;
						eType = dColumns[i].m_eType;
					}
					else
						sName.SetSprintf ( "col_%d", i+1 );
					SendMysqlFieldPacket ( tOut, uPacketID, szDB, "", sName.cstr(), eType );
				}
				if ( OmitEof() )
				{
					if ( session::GetForceMetadataEof() )
						SendMysqlEofPacketRaw ( tOut, uPacketID++, 0, false, session::IsAutoCommit (), session::IsInTrans() );
				} else
					SendMysqlEofPacket ( tOut, uPacketID++, 0, false, session::IsAutoCommit (), session::IsInTrans() );
			}
			break;
		}
		case MYSQL_COM_STMT_SEND_LONG_DATA:
		{
			DWORD uStmtId = MysqlReadLSBDword ( tIn );
			WORD uParamId = MysqlReadLSBWord ( tIn );
			int iDataLen = iPacketLen - 1 - 4 - 2;
			if ( iDataLen < 0 )
				iDataLen = 0;
			const BYTE * pData = nullptr;
			if ( iDataLen>0 )
				tIn.GetBytesZerocopy ( &pData, iDataLen );

			auto * pStmt = session::GetClientSession()->FindPreparedStmt ( uStmtId );
			if ( !pStmt || uParamId >= pStmt->m_iParamCount )
			{
				SendMysqlErrorPacket ( tOut, uPacketID, FromSz ( "unknown prepared statement or parameter" ), EMYSQL_ERR::UNKNOWN_COM_ERROR );
				break;
			}

			StringBuilder_c sCombined;
			if ( !pStmt->m_dLongData[uParamId].IsEmpty() )
				sCombined.AppendRawChunk ( Str_t { pStmt->m_dLongData[uParamId].cstr(), pStmt->m_dLongData[uParamId].Length() } );
			if ( iDataLen>0 )
				sCombined.AppendRawChunk ( Str_t { (const char*)pData, iDataLen } );
			pStmt->m_dLongData[uParamId] = sCombined.cstr();

			break;
		}
		case MYSQL_COM_STMT_EXECUTE:
		{
			DWORD uStmtId = MysqlReadLSBDword ( tIn );
			tIn.GetByte(); // flags
			MysqlReadLSBDword ( tIn ); // iteration count

			auto * pStmt = session::GetClientSession()->FindPreparedStmt ( uStmtId );
			if ( !pStmt )
			{
				SendMysqlErrorPacket ( tOut, uPacketID, FromSz ( "unknown prepared statement" ), EMYSQL_ERR::UNKNOWN_COM_ERROR );
				break;
			}

			CSphVector<CSphString> dParams;
			CSphString sErr;
			if ( !ParseStmtParamValues ( tIn, *pStmt, dParams, sErr ) )
			{
				SendMysqlErrorPacket ( tOut, uPacketID, FromStr ( sErr ), EMYSQL_ERR::UNKNOWN_COM_ERROR );
				break;
			}

			CSphString sExpanded;
			CSphString sNormalized;
			CSphVector<int> dExplicitRaw;
			NormalizeVectorFunctions ( FromStr ( pStmt->m_sTemplate ), sNormalized, dExplicitRaw, sErr );

			CSphVector<bool> dRawParams;
			dRawParams.Resize ( pStmt->m_iParamCount );
			for ( auto iIdx : dExplicitRaw )
				if ( iIdx>=0 && iIdx<dRawParams.GetLength() )
					dRawParams[iIdx] = true;

			if ( pStmt->m_iParamCount )
			{
				const char * sPrefix = "__mps_param_";
				CSphString sProbe;
				ReplacePreparedParams ( FromStr ( sNormalized ), sProbe, [sPrefix]( int iParam )
				{
					CSphString sVal;
					sVal.SetSprintf ( "'%s%d'", sPrefix, iParam );
					return sVal;
				}, sErr );

				CSphVector<SqlStmt_t> dStmt;
				CSphString sParseError;
				if ( sphParseSqlQuery ( FromStr ( sProbe ), dStmt, sParseError, tSess.GetCollation () ) && dStmt.GetLength() )
				{
					const SqlStmt_t & tParsed = dStmt[0];
					if ( tParsed.m_eStmt==STMT_INSERT || tParsed.m_eStmt==STMT_REPLACE )
					{
						auto pServed = GetServed ( tParsed.m_sIndex );
						if ( pServed )
							ApplyImplicitInsertTypes ( tParsed, RIdx_c ( pServed )->GetMatchSchema(), sPrefix, dRawParams );
					}
				}

				CSphString sUpdTable;
				CSphVector<std::pair<int, CSphString>> dUpdParams;
				if ( ExtractUpdateParamColumns ( FromStr ( sNormalized ), sUpdTable, dUpdParams ) )
				{
					auto pServed = GetServed ( sUpdTable );
					if ( pServed )
					{
						const auto & tSchema = RIdx_c ( pServed )->GetMatchSchema();
						for ( const auto & tMap : dUpdParams )
						{
							const int iParam = tMap.first;
							if ( iParam<0 || iParam>=dRawParams.GetLength() )
								continue;
							const auto * pAttr = tSchema.GetAttr ( tMap.second.cstr() );
							if ( pAttr && ( pAttr->m_eAttrType==SPH_ATTR_UINT32SET || pAttr->m_eAttrType==SPH_ATTR_INT64SET || pAttr->m_eAttrType==SPH_ATTR_FLOAT_VECTOR ) )
								dRawParams[iParam] = true;
						}
					}
				}
			}

			if ( !RenderPreparedQuery ( FromStr ( sNormalized ), dParams, sExpanded, sErr, &dRawParams ) )
			{
				SendMysqlErrorPacket ( tOut, uPacketID, FromStr ( sErr ), EMYSQL_ERR::UNKNOWN_COM_ERROR );
				break;
			}

			myinfo::SetDescription ( sExpanded, sExpanded.Length() );
			AT_SCOPE_EXIT ( []() { myinfo::SetDescription ( {}, 0 ); } );
			tSess.SetTaskState ( TaskState_e::QUERY );

			BinarySqlRowBuffer_c tRows ( &uPacketID, &tOut );
			tRows.SetForceStrings ( false );
			tSess.m_pSqlRowBuffer = &tRows;
			auto tStoredPos = tRows.GetCurrentPositionState();
			bKeepProfile = session::Execute ( FromStr ( sExpanded ), tRows );
			if ( tRows.IsError() )
			{
				if ( !HasBuddy() || tRows.WasFlushed() )
				{
					LogSphinxqlError ( myinfo::UnsafeDescription().first, FromStr ( tRows.GetError() ) );
					if ( tRows.WasFlushed() )
						sphLogDebug ( "Can't invoke buddy, because output socket was flushed; unable to rewind/overwrite anything" );
				} else
				{
					ProcessSqlQueryBuddy ( FromStr ( sExpanded ), FromStr ( tRows.GetError() ), tStoredPos, uPacketID, tOut );
				}
			}
			break;
		}
		case MYSQL_COM_STMT_RESET:
		{
			DWORD uStmtId = MysqlReadLSBDword ( tIn );
			if ( !session::GetClientSession()->ResetPreparedStmt ( uStmtId ) )
			{
				SendMysqlErrorPacket ( tOut, uPacketID, FromSz ( "unknown prepared statement" ), EMYSQL_ERR::UNKNOWN_COM_ERROR );
				break;
			}
			SendMysqlOkPacket ( tOut, uPacketID, session::IsAutoCommit(), session::IsInTrans() );
			break;
		}
		case MYSQL_COM_STMT_CLOSE:
		{
			DWORD uStmtId = MysqlReadLSBDword ( tIn );
			session::GetClientSession()->RemovePreparedStmt ( uStmtId );
			break; // no response for COM_STMT_CLOSE
		}
		case MYSQL_COM_STMT_FETCH:
			SendMysqlErrorPacket ( tOut, uPacketID, FromSz ( "COM_STMT_FETCH is not supported" ), EMYSQL_ERR::UNKNOWN_COM_ERROR );
			break;
		case MYSQL_COM_FIELD_LIST:
		{
			auto sTable = MysqlReadSzStr ( tIn );
			sphLogDebugv ( "LoopClientMySQL command %d, '%s'", uMysqlCmd, sTable.cstr() );
			SqlRowBuffer_c tRows ( &uPacketID, &tOut );
			tSess.m_pSqlRowBuffer = &tRows;
			SendTableSchema ( tRows, sTable );
			SendMysqlEofPacket ( tOut, uPacketID, 0, false, session::IsAutoCommit (), session::IsInTrans() );
			break;
		}

		default:
			// default case, unknown command
			StringBuilder_c sError;
			sError << "unknown command (code=" << uMysqlCmd << ")";
			LogSphinxqlError ( "", Str_t ( sError ) );
			SendMysqlErrorPacket ( tOut, uPacketID, Str_t(sError), EMYSQL_ERR::UNKNOWN_COM_ERROR );
			break;
	}

	auto uBytesConsumed = uHasBytesIn - tIn.HasBytes ();
	if ( uBytesConsumed<iPacketLen )
	{
		uBytesConsumed = iPacketLen - uBytesConsumed;
		sphLogDebugv ( "LoopClientMySQL disposing unused %d bytes", uBytesConsumed );
		const BYTE* pFoo = nullptr;
		tIn.GetBytesZerocopy (&pFoo, uBytesConsumed);
	}

	// send the response packet
	tSess.SetTaskState ( TaskState_e::NET_WRITE );
	if ( !tOut.Flush () )
		return false;

	// finalize query profile
	if ( pProfile )
		pProfile->Stop();
	if ( ( uMysqlCmd==MYSQL_COM_QUERY || uMysqlCmd==MYSQL_COM_STMT_EXECUTE ) && bKeepProfile )
		session::SaveLastProfile();
	tOut.SetProfiler ( nullptr );
	return true;
}

} // static namespace

// that is used from sphinxql command over API
void RunSingleSphinxqlCommand ( Str_t sCommand, GenericOutputBuffer_c & tOut )
{
	BYTE uDummy = 0;

	SqlRowBuffer_c tRows ( &uDummy, &tOut );
	session::Execute ( sCommand, tRows );
}

// add 'compressed' flag
struct QlCompressedInfo_t : public TaskInfo_t
{
	DECLARE_RENDER( QlCompressedInfo_t );
	bool m_bCompressed = false;
};

DEFINE_RENDER( QlCompressedInfo_t )
{
	auto & tInfo = *(QlCompressedInfo_t *)const_cast<void*>(pSrc);
	if ( tInfo.m_bCompressed )
	{
		dDst.m_sProto << "compressed";
		dDst.m_sChain << "gzip ";
	}
}

// main sphinxql server
void SqlServe ( std::unique_ptr<AsyncNetBuffer_c> pBuf )
{
	auto& tSess = session::Info();

	// to display 'compressed' flag, if any.
	auto pCompressedFlag = PublishTaskInfo ( new QlCompressedInfo_t );

	// non-vip connections in maintainance should be already rejected on accept
	assert ( !g_bMaintenance || tSess.GetVip() );

	// set off query guard
	GlobalCrashQueryGetRef ().m_eType = QUERY_SQL;
	const bool bCanZlibCompression = IsZlibCompressionAvailable();
	const bool bCanZstdCompression = IsZstdCompressionAvailable();

	int iCID = tSess.GetConnID();
	const char * sClientIP = tSess.szClientName();

	GenericOutputBuffer_c* pOut = pBuf.get();
	AsyncNetInputBuffer_c* pIn = pBuf.get();

	/// mysql is pro-active, we NEED to send handshake before client send us something.
	/// So, no passive probing possible.
	// send handshake first
	tSess.SetTaskState ( TaskState_e::HANDSHAKE );
	HandshakeV10_c tHandshake ( iCID );
	tHandshake.SetCanSsl ( CheckWeCanUseSSL() ); // fixme! SSL capability must be set only if keys are valid!
	tHandshake.SetCanZlib( bCanZlibCompression );
	tHandshake.SetCanZstd( bCanZstdCompression );
	tHandshake.Send ( *pOut );
	tSess.SetTaskState ( TaskState_e::NET_WRITE );
	if ( !pOut->Flush () )
	{
		int iErrno = sphSockGetErrno ();
		sphWarning ( "failed to send server version (client=%s(%d), error: %d '%s')",
				sClientIP, iCID, iErrno, sphSockError ( iErrno ) );
		return;
	}

	CSphString sError;
	bool bAuthed = false;
	BYTE uPacketID = 1;
	int iPacketLen;
	int iTimeoutS = -1;
	int iWTimeoutS = -1;
	do
	{
		tSess.SetKilled ( false );
		// check for updated timeout
		auto iCurrentTimeout = tSess.GetTimeoutS(); // by default -1, means 'default'
		if ( iCurrentTimeout<0 )
			iCurrentTimeout = g_iClientQlTimeoutS;

		if ( iCurrentTimeout!=iTimeoutS )
		{
			iTimeoutS = iCurrentTimeout;
			pIn->SetTimeoutUS ( S2US * iTimeoutS );
		}

		iCurrentTimeout = tSess.GetWTimeoutS(); // by default -1, means 'default'
		if ( iCurrentTimeout < 0 )
			iCurrentTimeout = g_iClientQlTimeoutS;

		if ( iCurrentTimeout != iWTimeoutS )
		{
			iWTimeoutS = iCurrentTimeout;
			pOut->SetWTimeoutUS( S2US * iWTimeoutS );
		}

		pIn->DiscardProcessed ();
		iPacketLen = 0;

		// get next packet
		// we want interruptible calls here, so that shutdowns could be honored
		sphLogDebugv ( "Receiving command... %d bytes in buf", pIn->HasBytes() );

		// setup per-query profiling
		auto pProfile = session::StartProfiling ( SPH_QSTATE_TOTAL );
		if ( pProfile )
			pOut->SetProfiler ( pProfile );

		int iChunkLen = MAX_PACKET_LEN;

		auto iStartPacketPos = pIn->GetBufferPos ();
		while (iChunkLen==MAX_PACKET_LEN)
		{
			// inlined AsyncReadMySQLPacketHeader
			if ( !pIn->ReadFrom ( iPacketLen+4 ))
			{
				// if there was eof, we're done from
				// comment from the SyncSockRead
				// while we wait the start of the packet - is ok to quit but right way is to send MYSQL_COM_QUERY
				bool bNotError = ( !iPacketLen );
				sError.SetSprintf ( "bailing on failed MySQL header, %s", ( pIn->GetError() ? pIn->GetErrorMessage().cstr() : sphSockError() ) );
				// still want to log this even into logdebugv along with all other net events
				LogNetError ( sError.cstr(), bNotError );
				if ( !bNotError )
				{
					SendMysqlErrorPacket ( *pOut, uPacketID, FromStr ( sError ), EMYSQL_ERR::UNKNOWN_COM_ERROR );
					pOut->Flush ();
				}
				return;
			}
			pIn->SetBufferPos ( iStartPacketPos + iPacketLen ); // will read at the end of the buffer
			DWORD uAddon = pIn->GetLSBDword ();
			pIn->DiscardProcessed ( sizeof ( uAddon )); // move out this header to keep rest of the buff solid
			pIn->SetBufferPos ( iStartPacketPos ); // rewind back after the read.
			uPacketID = 1+(BYTE) ( uAddon >> 24 );
			iChunkLen = ( uAddon & MAX_PACKET_LEN );

			sphLogDebugv ( "AsyncReadMySQLPacketHeader returned %d len...", iChunkLen );
			iPacketLen += iChunkLen;

			if ( !bAuthed && ( uAddon == SPHINX_CLIENT_VERSION || uAddon == 0x01000000UL ) )
			{
				sphLogDebug ( "conn %d from %s: seems, that non-mysql proto (sphinx?) packet received (%x). M.b. you've confused remote port in distributed table definition?", iCID, sClientIP, uAddon );
				return;
			}

			// receive package body
			if ( !pIn->ReadFrom ( iPacketLen ))
			{
				sError.SetSprintf ( "failed to receive MySQL request body, expected length %d, %s", iPacketLen, ( pIn->GetError() ? pIn->GetErrorMessage().cstr() : sphSockError() ) );
				LogNetError ( sError.cstr() );
				SendMysqlErrorPacket ( *pOut, uPacketID, FromStr ( sError ), EMYSQL_ERR::UNKNOWN_COM_ERROR );
				pOut->Flush ();
				return;
			}
		}

		SwitchProfile ( pProfile, SPH_QSTATE_UNKNOWN );

		// handle auth packet
		if ( !bAuthed )
		{
			tSess.SetTaskState ( TaskState_e::HANDSHAKE );
			HandshakeResponse41 tResponse ( *pIn, iPacketLen );

			// switch to ssl by demand.
			// You need to set a bit in handshake (g_sMysqlHandshake) in order to suggest client such switching.
			// Client set this desirable bit only if we say that 'we can' about it before.

			if ( !tSess.GetSsl() && tResponse.WantSSL() ) // want SSL
			{
				tSess.SetSsl ( MakeSecureLayer ( pBuf ) );
				pOut = pBuf.get();
				pIn = pBuf.get();
				tSess.SetPersistent( !pOut->GetError () );
				continue; // next packet will be 'login' again, but received over SSL
			}

			if ( IsMaxedOut() )
			{
				LogNetError ( g_sMaxedOutMessage.first );
				SendMysqlErrorPacket ( *pOut, uPacketID, g_sMaxedOutMessage, EMYSQL_ERR::UNKNOWN_COM_ERROR );
				pOut->Flush ();
				gStats().m_iMaxedOut.fetch_add ( 1, std::memory_order_relaxed );
				break;
			}

		if ( tResponse.GetUsername() == "FEDERATED" )
			session::SetFederatedUser();
		session::SetUser ( tResponse.GetUsername() );

		const CSphString * pClientName = tResponse.GetAttr ( "_client_name" );
		const CSphString * pProgramName = tResponse.GetAttr ( "program_name" );
		const bool bForceMetaEof = ( pClientName && *pClientName=="libmysql" )
			|| ( pProgramName && *pProgramName=="mysql" );

		if ( !ValidateDBName ( tResponse.GetDB() ) )
		{
			StringBuilder_c sError;
			sError << "no such database " << tResponse.GetDB().value_or ( "<empty>" );
			LogSphinxqlError ( "", Str_t ( sError ) );
				SendMysqlErrorPacket ( *pOut, uPacketID, Str_t(sError), EMYSQL_ERR::NO_DB_ERROR );
				pOut->Flush();
				break;
		}

		SendMysqlOkPacket ( *pOut, uPacketID, session::IsAutoCommit(), session::IsInTrans ());
		tSess.SetPersistent ( pOut->Flush () );
		bAuthed = true;
		session::SetDeprecatedEOF ( tResponse.DeprecateEOF() );
		session::SetForceMetadataEof ( bForceMetaEof );

			if ( bCanZstdCompression && tResponse.WantZstd() )
			{
				MakeZstdMysqlCompressedLayer ( pBuf, tResponse.WantZstdLev() );
				pOut = pBuf.get();
				pIn = pBuf.get();
				pCompressedFlag->m_bCompressed = true;
			}
			else if ( bCanZlibCompression && tResponse.WantZlib() )
			{
				MakeZlibMysqlCompressedLayer ( pBuf );
				pOut = pBuf.get();
				pIn = pBuf.get();
				pCompressedFlag->m_bCompressed = true;
			}
			continue;
		}

		tSess.SetPersistent ( LoopClientMySQL ( uPacketID, iPacketLen, pProfile, pBuf.get() ) );

		pBuf->SyncErrorState();
		if ( pIn->GetError() )
			LogNetError ( pIn->GetErrorMessage().cstr() );
		pBuf->ResetError();

	} while ( tSess.GetPersistent() );
}

RowBuffer_i * CreateSqlRowBuffer ( BYTE * pPacketID, GenericOutputBuffer_c * pOut )
{
	return new SqlRowBuffer_c ( pPacketID, pOut );
}

RowBuffer_i * CreateBinarySqlRowBuffer ( BYTE * pPacketID, GenericOutputBuffer_c * pOut )
{
	return new BinarySqlRowBuffer_c ( pPacketID, pOut );
}
