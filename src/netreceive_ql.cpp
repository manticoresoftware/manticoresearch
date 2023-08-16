//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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
#include "searchdbuddy.h"

extern int g_iClientQlTimeoutS;    // sec
extern volatile bool g_bMaintenance;
extern CSphString g_sMySQLVersion;

namespace { // c++ way of 'static'

/////////////////////////////////////////////////////////////////////////////
// encodes Mysql Length-coded binary
BYTE * MysqlPackInt ( BYTE * pOutput, int64_t iValue )
{
	if ( iValue>=0 && iValue<251 )
	{
		*pOutput++ = (BYTE)iValue;
		return pOutput;
	}

	if ( iValue>=0 && iValue<=0xFFFF )
	{
		*pOutput++ = (BYTE)'\xFC'; // 252
		*pOutput++ = (BYTE)iValue;
		*pOutput++ = (BYTE)( iValue>>8 );
		return pOutput;
	}

	if ( iValue>=0 && iValue<=0xFFFFFF )
	{
		*pOutput++ = (BYTE)'\xFD'; // 253
		*pOutput++ = (BYTE)iValue;
		*pOutput++ = (BYTE)( iValue>>8 );
		*pOutput++ = (BYTE)( iValue>>16 );
		return pOutput;
	}

	*pOutput++ = (BYTE)'\xFE'; // 254
	*pOutput++ = (BYTE)iValue;
	*pOutput++ = (BYTE)( iValue>>8 );
	*pOutput++ = (BYTE)( iValue>>16 );
	*pOutput++ = (BYTE)( iValue>>24 );
	*pOutput++ = (BYTE)( iValue>>32 );
	*pOutput++ = (BYTE)( iValue>>40 );
	*pOutput++ = (BYTE)( iValue>>48 );
	*pOutput++ = (BYTE)( iValue>>56 );
	return pOutput;
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
	MYSQL_COM_STATISTICS = 9,
	MYSQL_COM_PING		= 14,
	MYSQL_COM_SET_OPTION	= 27
};

void SendMysqlErrorPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, const char * sStmt,
	Str_t sError, MysqlErrors_e iErr )
{
	if ( IsEmpty ( sError ) )
		sError = FROMS("(null)");

	LogSphinxqlError ( sStmt, sError );

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
	int iLen = 9 + sError.second;
	int iError = iErr; // pretend to be mysql syntax error for now

	// send packet header
	tOut.SendLSBDword ( (uPacketID<<24) + iLen );
	tOut.SendByte ( 0xff ); // field count, always 0xff for error packet
	tOut.SendByte ( (BYTE)( iError & 0xff ) );
	tOut.SendByte ( (BYTE)( iError>>8 ) );

	// send sqlstate (1 byte marker, 5 byte state)
	switch ( iErr )
	{
		case MYSQL_ERR_SERVER_SHUTDOWN:
		case MYSQL_ERR_UNKNOWN_COM_ERROR:
			tOut.SendBytes ( "#08S01", 6 );
			break;
		case MYSQL_ERR_NO_SUCH_TABLE:
			tOut.SendBytes ( "#42S02", 6 );
			break;
		case MYSQL_ERR_NO_SUCH_THREAD:
			tOut.SendBytes ( "#HY000", 6 );
			break;
		default:
			tOut.SendBytes ( "#42000", 6 );
			break;
	}

	// send error message
	tOut.SendBytes ( sError );
}

void SendMysqlEofPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, int iWarns, bool bMoreResults, bool bAutoCommit, bool bIsInTrans )
{
	tOut.SendLSBDword ( ( uPacketID << 24 ) + 5 );
	tOut.SendByte ( 0xfe );

	if ( iWarns<0 ) iWarns = 0;
	if ( iWarns>65535 ) iWarns = 65535;
	tOut.SendLSBWord ( iWarns );

	WORD uStatus = 0;
	if ( bMoreResults )
		uStatus |= MYSQL_FLAG::MORE_RESULTS;
	if ( bAutoCommit )
		uStatus |= MYSQL_FLAG::STATUS_AUTOCOMMIT;
	if ( bIsInTrans )
		uStatus |= MYSQL_FLAG::STATUS_IN_TRANS;
	tOut.SendLSBWord ( uStatus );
}


// was defaults ( ISphOutputBuffer & tOut, BYTE uPacketID, int iAffectedRows=0, int iWarns=0, const char * sMessage=nullptr, bool bMoreResults=false, bool bAutoCommit )
void SendMysqlOkPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, int iAffectedRows, int iWarns, const char * sMessage,
		bool bMoreResults, bool bAutoCommit, bool bIsInTrans, int64_t iLastID )
{
	BYTE sVarLen[20] = {0}; // max 18 for packed number, +1 more just for fun
	BYTE * pBuf = sVarLen;
	pBuf = MysqlPackInt ( pBuf, iAffectedRows );
	pBuf = MysqlPackInt ( pBuf, iLastID );
	int iLen = int ( pBuf - sVarLen );

	int iMsgLen = 0;
	if ( sMessage )
		iMsgLen = (int) strlen(sMessage);

	tOut.SendLSBDword ( DWORD (uPacketID<<24) + iLen + iMsgLen + 5);
	tOut.SendByte ( 0 );				// ok packet
	tOut.SendBytes ( sVarLen, iLen );	// packed affected rows & insert_id
	if ( iWarns<0 ) iWarns = 0;
	if ( iWarns>65535 ) iWarns = 65535;
	DWORD uWarnStatus = ( iWarns<<16 );
	// order of WORDs is opposite to EOF packet above
	if ( bMoreResults )
		uWarnStatus |= MYSQL_FLAG::MORE_RESULTS;
	if ( bAutoCommit )
		uWarnStatus |= MYSQL_FLAG::STATUS_AUTOCOMMIT;
	if ( bIsInTrans )
		uWarnStatus |= MYSQL_FLAG::STATUS_IN_TRANS;

	tOut.SendLSBDword ( uWarnStatus );		// 0 status, N warnings
	if ( sMessage )
		tOut.SendBytes ( sMessage, iMsgLen );
}


void SendMysqlOkPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, bool bAutoCommit, bool bIsInTrans )
{
	SendMysqlOkPacket ( tOut, uPacketID, 0, 0, nullptr, false, bAutoCommit, bIsInTrans, 0 );
}

//////////////////////////////////////////////////////////////////////////
// Mysql row buffer and command handler

class SqlRowBuffer_c final : public RowBuffer_i, private LazyVector_T<BYTE>
{
	BYTE & m_uPacketID;
	GenericOutputBuffer_c & m_tOut;
	ClientSession_c* m_pSession = nullptr;
	size_t m_iTotalSent = 0;
#ifndef NDEBUG
	size_t m_iColumns = 0; // used for head/data columns num sanitize check
#endif

	// how many bytes this int will occupy in proto mysql
	static inline int SqlSizeOf ( int iLen )
	{
		if ( iLen<251 )
			return 1;
		if ( iLen<=0xffff )
			return 3;
		if ( iLen<=0xffffff )
			return 4;
		return 9;
	}

	// how many bytes this string will occupy in proto mysql
	static int SqlStrlen ( const char * sStr )
	{
		auto iLen = ( int ) strlen ( sStr );
		return SqlSizeOf ( iLen ) + iLen;
	}

	void SendSqlInt ( int iVal )
	{
		BYTE dBuf[12];
		auto * pBuf = MysqlPackInt ( dBuf, iVal );
		m_tOut.SendBytes ( dBuf, ( int ) ( pBuf - dBuf ) );
	}

	void SendSqlString ( const char * sStr )
	{
		auto iLen = (int) strlen ( sStr );
		SendSqlInt ( iLen );
		m_tOut.SendBytes ( sStr, iLen );
	}

	bool SomethingWasSent() final {
		auto iPrevSent = std::exchange ( m_iTotalSent, m_tOut.GetTotalSent() + m_tOut.GetSentCount() + GetLength() );
		return iPrevSent != m_iTotalSent;
	}

	void SendSqlFieldPacket ( const char * sCol, MysqlColumnType_e eType, WORD uFlags=0 )
	{
		const char * sDB = "";
		const char * sTable = "";

		int iLen = 17 + SqlStrlen ( sDB ) + 2 * ( SqlStrlen ( sTable ) + SqlStrlen ( sCol ) );

		int iColLen = 0;
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
		case MYSQL_COL_STRING: iColLen = 255;
			break;
		}

		m_tOut.SendLSBDword ( ( ( m_uPacketID++ ) << 24 ) + iLen );
		SendSqlString ( "def" ); // catalog
		SendSqlString ( sDB ); // db
		SendSqlString ( sTable ); // table
		SendSqlString ( sTable ); // org_table
		SendSqlString ( sCol ); // name
		SendSqlString ( sCol ); // org_name

		m_tOut.SendByte ( 12 ); // filler, must be 12 (following pseudo-string length)
		m_tOut.SendByte ( 0x21 ); // charset_nr, 0x21 is utf8
		m_tOut.SendByte ( 0 ); // charset_nr
		m_tOut.SendLSBDword ( iColLen ); // length
		m_tOut.SendByte ( BYTE ( eType ) ); // type (0=decimal)
		m_tOut.SendByte ( uFlags & 255 );
		m_tOut.SendByte ( uFlags >> 8 );
		m_tOut.SendByte ( 0 ); // decimals
		m_tOut.SendWord ( 0 ); // filler
	}

	bool IsAutoCommit() const
	{
		if ( !m_pSession )
			return true;
		return session::IsAutoCommit ( m_pSession );
	}

	bool IsInTrans () const
	{
		if ( !m_pSession )
			return false;
		return session::IsInTrans ( m_pSession );
	}

public:

	SqlRowBuffer_c ( BYTE * pPacketID, GenericOutputBuffer_c * pOut )
		: m_uPacketID ( *pPacketID )
		, m_tOut ( *pOut )
		, m_pSession ( session::GetClientSession() )
	{}

	void PutFloatAsString ( float fVal, const char * sFormat ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
		int iLen = sFormat
			? snprintf (( char* ) pSize + 1, SPH_MAX_NUMERIC_STR - 1, sFormat, fVal )
			: sph::PrintVarFloat (( char* ) pSize + 1, SPH_MAX_NUMERIC_STR - 1, fVal );
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	void PutDoubleAsString ( double fVal, const char * szFormat ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
		int iLen = szFormat
			? snprintf (( char* ) pSize + 1, SPH_MAX_NUMERIC_STR - 1, szFormat, fVal )
			: sph::PrintVarDouble (( char* ) pSize + 1, SPH_MAX_NUMERIC_STR - 1, fVal );
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	void PutNumAsString ( int64_t iVal ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
#if __has_include( <charconv>)
		int iLen = std::to_chars ( (char*)pSize + 1, (char*)pSize + SPH_MAX_NUMERIC_STR, iVal ).ptr - (char*)( pSize + 1 );
#else
	int iLen = sph::NtoA ( ( char * ) pSize + 1, iVal );
#endif
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	void PutNumAsString ( uint64_t uVal ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
#if __has_include( <charconv>)
		int iLen = std::to_chars ( (char*)pSize + 1, (char*)pSize + SPH_MAX_NUMERIC_STR, uVal ).ptr - (char*)( pSize + 1 );
#else
		int iLen = sph::NtoA ( (char*)pSize + 1, uVal );
#endif
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	void PutNumAsString ( int iVal ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
#if __has_include( <charconv>)
		int iLen = std::to_chars ( (char*)pSize + 1, (char*)pSize + SPH_MAX_NUMERIC_STR, iVal ).ptr - (char*)( pSize + 1 );
#else
		int iLen = sph::NtoA ( ( char * ) pSize + 1, iVal );
#endif

		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	void PutNumAsString ( DWORD uVal ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
#if __has_include( <charconv>)
		int iLen = std::to_chars ( (char*)pSize + 1, (char*)pSize + SPH_MAX_NUMERIC_STR, uVal ).ptr - (char*)( pSize + 1 );
#else
		int iLen = sph::NtoA ( ( char * ) pSize + 1, uVal );
#endif
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
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

		auto pSpace = AddN ( dBlob.second + 9 ); // 9 is taken from MysqlPack() implementation (max possible offset)
		auto * pStr = MysqlPackInt ( pSpace, dBlob.second );
		if ( dBlob.second )
			memcpy ( pStr, dBlob.first, dBlob.second );
		Resize ( Idx ( pStr ) + dBlob.second );
	}

	// pack string (or "")
	void PutString ( Str_t sMsg ) override
	{
		PutArray ( S2B ( sMsg ), false );
	}

	void PutMicrosec ( int64_t iUsec ) override
	{
		iUsec = Max ( iUsec, 0 );

		ReserveGap ( SPH_MAX_NUMERIC_STR+1 );
		auto pSize = (char*) End();
		int iLen = sph::IFtoA ( pSize + 1, iUsec, 6 );
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
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

		int iLeft = GetLength();
		const BYTE * pBuf = Begin();
		while ( iLeft )
		{
			int iSize = Min ( iLeft, MAX_PACKET_LEN );
			m_tOut.SendLSBDword ( ((m_uPacketID++)<<24) + iSize );
			m_tOut.SendBytes ( pBuf, iSize );
			pBuf += iSize;
			iLeft -= iSize;
			if ( m_tOut.GetSentCount() > MAX_PACKET_LEN )
				if ( !m_tOut.Flush() )
				{
					m_bError = true;
					return false;
				}
		}
		Resize(0);
		return true;
	}

	// wrappers for popular packets
	void Eof ( bool bMoreResults, int iWarns ) override
	{
		SendMysqlEofPacket ( m_tOut, m_uPacketID++, iWarns, bMoreResults, IsAutoCommit(), IsInTrans() );
	}

	void Error ( const char * sStmt, const char * sError, MysqlErrors_e iErr ) override
	{
		m_bError = true;
		m_sError = sError;
		SendMysqlErrorPacket ( m_tOut, m_uPacketID, sStmt, FromSz(sError), iErr );
	}

	void Ok ( int iAffectedRows, int iWarns, const char * sMessage, bool bMoreResults, int64_t iLastInsertId ) override
	{
		SendMysqlOkPacket ( m_tOut, m_uPacketID, iAffectedRows, iWarns, sMessage, bMoreResults, IsAutoCommit(), IsInTrans(), iLastInsertId );
		if ( bMoreResults )
			m_uPacketID++;
	}

	// Header of the table with defined num of columns
	inline void HeadBegin ( int iColumns ) override
	{
		m_tOut.SendLSBDword ( ((m_uPacketID++)<<24) + SqlSizeOf ( iColumns ) );
		SendSqlInt ( iColumns );
#ifndef NDEBUG
		m_iColumns = iColumns;
#endif
	}

	bool HeadEnd ( bool bMoreResults, int iWarns ) override
	{
		Eof ( bMoreResults, iWarns );
		Resize(0);
		return true;
	}

	// add the next column. The EOF after the tull set will be fired automatically
	void HeadColumn ( const char * sName, MysqlColumnType_e uType ) override
	{
		assert ( m_iColumns-->0 && "you try to send more mysql columns than declared in InitHead" );
		SendSqlFieldPacket ( sName, uType );
	}

	void Add ( BYTE uVal ) override
	{
		LazyVector_T<BYTE>::Add ( uVal );
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
	char m_sAuthData[AUTH_DATA_LEN];
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
			memcpy ( m_sAuthData + i, &uRand, sizeof ( DWORD ) );
			uRand = sphRand() | 0x01010101;
		}
		if ( i < AUTH_DATA_LEN )
			memcpy ( m_sAuthData + i, &uRand, AUTH_DATA_LEN - i );
		memset ( m_sAuthData + AUTH_DATA_LEN - 1, 0, 1);
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

		const BYTE dFiller[10] { 0 };
		sphLogDebugv ( "Sending handshake..." );
		DWORD uHandshakeLen = sizeof ( m_uVersion )
							+ m_sVersionString.second
							+ sizeof ( m_uConnID )
							+ AUTH_DATA_LEN
							+ sizeof ( AUTH_DATA_LEN )
							+ 1 // 0-filler after first 8 bytes of a scramble
							+ sizeof ( m_uCapabilities )
							+ sizeof ( m_uCharSet )
							+ sizeof ( m_uServerStatusFlag )
							+ sizeof ( dFiller )
							+ m_sAuthPluginName.second;
		assert ( ( uHandshakeLen & 0xFF000000U ) == 0 ); // package ID is 0 in initial package

		// generic mysql packet header
		tOut.SendLSBDword ( uHandshakeLen );

		// Protocol::HandshakeV10
		tOut.SendByte ( m_uVersion );
		tOut.SendBytes ( m_sVersionString );
		tOut.SendLSBDword ( m_uConnID );
		tOut.SendBytes ( m_sAuthData, 8 );
		tOut.SendByte ( 0 );
		tOut.SendLSBWord ( m_uCapabilities & 0xFFFF );
		tOut.SendByte ( m_uCharSet );
		tOut.SendLSBWord ( m_uServerStatusFlag );
		tOut.SendLSBWord ( m_uCapabilities >> 16 );
		tOut.SendByte ( AUTH_DATA_LEN );
		tOut.SendBytes ( dFiller, sizeof ( dFiller ) );
		tOut.SendBytes ( m_sAuthData + 8, AUTH_DATA_LEN - 8 );
		tOut.SendBytes ( m_sAuthPluginName );
	}
};

// HandshakeResponse truncated right before username field
class HandshakeResponse41
{
	CSphString m_sLoginUserName;
	CSphString m_sAuthResponse;
	CSphString m_sDatabase;
	CSphString m_sClientPluginName;
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

		sphLogDebugv ( "HandshakeResponse41. PackedLen=%d, hasBytes=%d", iPacketLen, tIn.HasBytes() );
		// ssl auth is finished here
		if ( tIn.HasBytes() <=0 )
			return;

		// login name
		m_sLoginUserName = MysqlReadSzStr ( tIn );
		sphLogDebugv ( "User: %s", m_sLoginUserName.cstr() );

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
			m_sDatabase = MysqlReadSzStr ( tIn );
			sphLogDebugv ( "DB: %s", m_sDatabase.cstr() );
		}

		// db name
		if ( m_uCapabilities & CLIENT::PLUGIN_AUTH )
			m_sClientPluginName = MysqlReadSzStr ( tIn );

		// attributes
		if ( m_uCapabilities & CLIENT::CONNECT_ATTRS )
		{
			auto iWatermark = MysqlReadPackedInt ( tIn );
			sphLogDebugv ( "%d bytes of attrs", (int) iWatermark );
			iWatermark = tIn.HasBytes() - iWatermark;
			while ( iWatermark < tIn.HasBytes() )
			{
				auto sKey = MysqlReadVlStr ( tIn );
				auto sVal = MysqlReadVlStr ( tIn );
				sphLogDebugv ( "%s: %s", sKey.cstr(), sVal.cstr() );
				m_hAttributes.Add ( std::move ( sVal ), sKey );
			}
		}

		// compression level
		if ( tIn.HasBytes()>0 )
			m_uCompressionLevel = tIn.GetByte();
	}

	inline const CSphString& GetUsername() const
	{
		return m_sLoginUserName;
	}

	inline const CSphString& GetDB() const
	{
		return m_sDatabase;
	}

	inline bool WantSSL() const
	{
		return ( m_uCapabilities & CLIENT::SSL ) != 0;
	}

	inline bool WantZlib() const
	{
		return ( m_uCapabilities & CLIENT::COMPRESS ) != 0;
	}

	inline bool WantZstd() const
	{
		return ( m_uCapabilities & CLIENT::ZSTD_COMPRESSION_ALGORITHM ) != 0;
	}

	inline int WantZstdLev() const
	{
		return m_uCompressionLevel;
	}
};


static bool LoopClientMySQL ( BYTE & uPacketID, int iPacketLen, QueryProfile_c * pProfile, AsyncNetBuffer_c * pBuf )
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
		case MYSQL_COM_PING:
		case MYSQL_COM_INIT_DB:
			// client wants a pong
			SendMysqlOkPacket ( tOut, uPacketID, session::IsAutoCommit(), session::IsInTrans() );
			break;

		case MYSQL_COM_SET_OPTION:
			// bMulti = ( tIn.GetWord()==MYSQL_OPTION_MULTI_STATEMENTS_ON ); // that's how we could double check and validate multi query
			// server reporting success in response to COM_SET_OPTION and COM_DEBUG
			SendMysqlEofPacket ( tOut, uPacketID, 0, false, session::IsAutoCommit (), session::IsInTrans() );
			break;

		case MYSQL_COM_STATISTICS:
		{
			StringBuilder_c sStats;
			BuildStatusOneline ( sStats );
			auto iLen = sStats.GetLength();
			tOut.SendLSBDword ( ( uPacketID << 24 )+iLen );
			tOut.SendBytes ( sStats );
			break;
		}

		case MYSQL_COM_QUERY:
		{
			// handle query packet
			myinfo::SetDescription ( tIn.GetRawString ( iPacketLen-1 ), iPacketLen-1 ); // OPTIMIZE? could be huge, but string is hazard.
			assert ( !tIn.GetError() );
			sphLogDebugv ( "LoopClientMySQL command %d, '%s'", uMysqlCmd, myinfo::UnsafeDescription().first );
			tSess.SetTaskState ( TaskState_e::QUERY );
			bKeepProfile = ProcessSqlQueryBuddy ( myinfo::UnsafeDescription(), uPacketID, tOut );
		}
		break;

		default:
			// default case, unknown command
			StringBuilder_c sError;
			sError << "unknown command (code=" << uMysqlCmd << ")";
			SendMysqlErrorPacket ( tOut, uPacketID, NULL, Str_t(sError), MYSQL_ERR_UNKNOWN_COM_ERROR );
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
	if ( uMysqlCmd==MYSQL_COM_QUERY && bKeepProfile )
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
					SendMysqlErrorPacket ( *pOut, uPacketID, nullptr, FromStr ( sError ), MYSQL_ERR_UNKNOWN_COM_ERROR );
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
				SendMysqlErrorPacket ( *pOut, uPacketID, nullptr, FromStr ( sError ), MYSQL_ERR_UNKNOWN_COM_ERROR );
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
				SendMysqlErrorPacket ( *pOut, uPacketID, nullptr, g_sMaxedOutMessage, MYSQL_ERR_UNKNOWN_COM_ERROR );
				pOut->Flush ();
				gStats().m_iMaxedOut.fetch_add ( 1, std::memory_order_relaxed );
				break;
			}

			if ( tResponse.GetUsername() == "FEDERATED" )
				session::SetFederatedUser();
			session::SetDumpUser ( tResponse.GetUsername() );
			SendMysqlOkPacket ( *pOut, uPacketID, session::IsAutoCommit(), session::IsInTrans ());
			tSess.SetPersistent ( pOut->Flush () );
			bAuthed = true;

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
