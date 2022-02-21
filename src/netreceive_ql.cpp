//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

extern int g_iClientQlTimeoutS;    // sec
extern volatile bool g_bMaintenance;
extern CSphString g_sMySQLVersion;

namespace { // c++ way of 'static'

/////////////////////////////////////////////////////////////////////////////
// encodes Mysql Length-coded binary
BYTE * MysqlPackInt ( BYTE * pOutput, int64_t iValue )
{
	if ( iValue<0 )
		return pOutput;

	if ( iValue<251 )
	{
		*pOutput++ = (BYTE)iValue;
		return pOutput;
	}

	if ( iValue<=0xFFFF )
	{
		*pOutput++ = (BYTE)'\xFC'; // 252
		*pOutput++ = (BYTE)iValue;
		*pOutput++ = (BYTE)( iValue>>8 );
		return pOutput;
	}

	if ( iValue<=0xFFFFFF )
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

//////////////////////////////////////////////////////////////////////////
// MYSQLD PRETENDER
//////////////////////////////////////////////////////////////////////////

struct MYSQL_FLAG
{
	static constexpr WORD STATUS_IN_TRANS = 1;		// mysql.h: SERVER_STATUS_IN_TRANS
	static constexpr WORD STATUS_AUTOCOMMIT = 2;	// mysql.h: SERVER_STATUS_AUTOCOMMIT
	static constexpr WORD MORE_RESULTS = 8;		// mysql.h: SERVER_MORE_RESULTS_EXISTS
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
	const char * sError, MysqlErrors_e iErr )
{
	if ( sError==NULL )
		sError = "(null)";

	LogSphinxqlError ( sStmt, sError );

	auto iErrorLen = (int) strlen(sError);

	// cut the error message to fix isseue with long message for popular clients
	if ( iErrorLen>MYSQL_ERROR::MAX_LENGTH )
	{
		iErrorLen = MYSQL_ERROR::MAX_LENGTH;
		char * sErr = const_cast<char *>( sError );
		sErr[iErrorLen-3] = '.';
		sErr[iErrorLen-2] = '.';
		sErr[iErrorLen-1] = '.';
		sErr[iErrorLen] = '\0';
	}
	int iLen = 9 + iErrorLen;
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
		default:
			tOut.SendBytes ( "#42000", 6 );
			break;
	}

	// send error message
	tOut.SendBytes ( sError, iErrorLen );
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

class SqlRowBuffer_c : public RowBuffer_i, private LazyVector_T<BYTE>
{
	BYTE & m_uPacketID;
	ISphOutputBuffer & m_tOut;
	ClientSession_c* m_pSession = nullptr;
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

	SqlRowBuffer_c ( BYTE * pPacketID, ISphOutputBuffer * pOut )
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
			: sph::PrintVarFloat (( char* ) pSize + 1, fVal );
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	void PutDoubleAsString ( double fVal, const char * szFormat ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
		int iLen = szFormat
			? snprintf (( char* ) pSize + 1, SPH_MAX_NUMERIC_STR - 1, szFormat, fVal )
			: sph::PrintVarDouble (( char* ) pSize + 1, fVal );
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	void PutNumAsString ( int64_t iVal ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
		int iLen = sph::NtoA ( ( char * ) pSize + 1, iVal );
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	void PutNumAsString ( uint64_t uVal ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
		int iLen = sph::NtoA ( ( char * ) pSize + 1, uVal );
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	void PutNumAsString ( int iVal ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
		int iLen = sph::NtoA ( ( char * ) pSize + 1, iVal );
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	void PutNumAsString ( DWORD uVal ) override
	{
		ReserveGap ( SPH_MAX_NUMERIC_STR );
		auto pSize = End();
		int iLen = sph::NtoA ( ( char * ) pSize + 1, uVal );
		*pSize = BYTE ( iLen );
		AddN ( iLen + 1 );
	}

	// pack raw array (i.e. packed length, then blob) into proto mysql
	void PutArray ( const void * pBlob, int iLen, bool bSendEmpty ) override
	{
		if ( iLen<0 )
			return;

		if ( !iLen && bSendEmpty )
		{
			PutNULL();
			return;
		}

		auto pSpace = AddN ( iLen + 9 ); // 9 is taken from MysqlPack() implementation (max possible offset)
		auto * pStr = MysqlPackInt ( pSpace, iLen );
		if ( iLen )
			memcpy ( pStr, pBlob, iLen );
		Resize ( Idx ( pStr ) + iLen );
	}

	// pack zero-terminated string (or "" if it is zero itself)
	void PutString ( const char * sMsg, int iMaxLen=-1 ) override
	{
		int iLen = ( sMsg && *sMsg ) ? ( int ) strlen ( sMsg ) : 0;

		if (!sMsg)
			sMsg = "";

		if ( iMaxLen>=0 )
			iLen = Min ( iLen, iMaxLen );

		PutArray ( sMsg, iLen, false );
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
		m_tOut.SendLSBDword ( ((m_uPacketID++)<<24) + ( GetLength() ) );
		m_tOut.SendBytes ( *this );
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
		SendMysqlErrorPacket ( m_tOut, m_uPacketID, sStmt, sError, iErr );
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
	static constexpr DWORD RESERVED = 16384; // DEPRECATED: Old flag for 4.1 protocol
	static constexpr DWORD RESERVED2 = 32768; // DEPRECATED: Old flag for 4.1 authentication \ CLIENT_SECURE_CONNECTION.
	static constexpr DWORD PLUGIN_AUTH = ( 1UL << 19 );
	static constexpr DWORD ZSTD_COMPRESSION_ALGORITHM = ( 1UL << 26 );
};

// send MySQL wire protocol handshake packets
void SendMysqlProtoHandshake ( ISphOutputBuffer& tOut, bool bSsl, bool bCanUseCompression, bool bCanUseZstdCompression, DWORD uConnID )
{
	// packed header here (packedID = 0)

	const BYTE uHandshake1 = 0x0A; // protocol version 10

	// z-terminated version string here

	// LSB DWORD connection id here

	const char sHandshake2[] =
		"\x01\x02\x03\x04\x05\x06\x07\x08" // auth-plugin-data-part-1
		"\x00"; // filler

	DWORD uCapabilities = CLIENT::CONNECT_WITH_DB
						| CLIENT::PROTOCOL_41
						| CLIENT::RESERVED2 // deprecated
//						| CLIENT::RESERVED
						| CLIENT::PLUGIN_AUTH
						;
	const char sHandshake3[] =	"\x21" // server language; let it be ut8_general_ci to make different clients happy
		"\x02\x00"; // server status AUTO_COMMIT
	const char sHandshake4[] =	"\x15" // length of auth-plugin-data - 21 bytes
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // unuzed 10 bytes
		"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x00" // auth-plugin-data-part-2 (12 bytes + zero) (for auth, 4.1+)
		"mysql_native_password"; // auth plugin name. try to remove last z byte here...

	int iLen = g_sMySQLVersion.Length()+1; // +1 for z-terminator

	DWORD uHandshakeLen = 1 + iLen + 4 + sizeof(sHandshake2)-1 + 1 + 1 + sizeof(sHandshake3)-1 + 2 + sizeof ( sHandshake4 );
	tOut.SendLSBDword ( uHandshakeLen & 0x00FFFFFFU );
	tOut.SendByte( uHandshake1 );
	tOut.SendBytes ( g_sMySQLVersion.scstr(), iLen );
	tOut.SendLSBDword( uConnID );
	tOut.SendBytes ( sHandshake2, sizeof ( sHandshake2 )-1 );

	if (bCanUseCompression)
		uCapabilities |= CLIENT::COMPRESS;

	if (bCanUseZstdCompression)
		uCapabilities |= CLIENT::ZSTD_COMPRESSION_ALGORITHM;

	if ( bSsl )	// fixme! SSL capability must be set only if keys are valid!
		uCapabilities |= CLIENT::SSL;

	tOut.SendLSBWord ( uCapabilities & 0xFFFF );
	tOut.SendBytes ( sHandshake3, sizeof ( sHandshake3 )-1 );
	tOut.SendLSBWord ( uCapabilities >> 16 );
	tOut.SendBytes ( sHandshake4, sizeof ( sHandshake4 ) ); // incl. z-terminator
}

const int MAX_PACKET_LEN = 0xffffffL; // 16777215 bytes, max low level packet size

inline bool UsernameIsFEDERATED ( const ByteBlob_t& tPacket )
{
	// handshake packet structure
	// 4              capability flags, CLIENT_PROTOCOL_41 always set
	// 4              max-packet size
	// 1              character set
	// string[23]     reserved (all [0])
	// string[NUL]    username

	if ( !tPacket.first || tPacket.second<(4+4+1+23+1) )
		return false;

	const char * sFederated = "FEDERATED";
	const char * sSrc = (const char *) tPacket.first + 4+4+1+23;

	return ( strncmp ( sFederated, sSrc, tPacket.second-(4+4+1+23) )==0 );
}

inline bool UserWantsSSL ( DWORD uCapabilities )
{
	return ( uCapabilities & CLIENT::SSL ) != 0;
}

inline bool UserWantsCompression ( DWORD uCapabilities )
{
	return ( uCapabilities & CLIENT::COMPRESS ) != 0;
}

inline bool UserWantsZstdCompression ( DWORD uCapabilities )
{
	return ( uCapabilities & CLIENT::ZSTD_COMPRESSION_ALGORITHM ) != 0;
}

inline int UserWantsZstdCompressionLevel ( const ByteBlob_t& tPacket )
{
	return tPacket.first[tPacket.second-1];
}

bool LoopClientMySQL ( BYTE & uPacketID, int iPacketLen,
		QueryProfile_c * pProfile, AsyncNetBufferPtr_c pBuf )
{
	auto& tSess = session::Info();
	assert ( pBuf );
	auto& tIn = *(AsyncNetInputBuffer_c *) pBuf;
	auto& tOut = *(NetGenericOutputBuffer_c *) pBuf;

	auto uHasBytesIn = tIn.HasBytes ();
	// get command, handle special packets
	const BYTE uMysqlCmd = tIn.GetByte ();

	if ( uMysqlCmd!=MYSQL_COM_QUERY )
		sphLogDebugv ( "LoopClientMySQL command %d", uMysqlCmd );

	if ( uMysqlCmd==MYSQL_COM_QUIT )
		return false;

	CSphString sError;
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
			SqlRowBuffer_c tRows ( &uPacketID, &tOut );
			bKeepProfile = session::Execute ( myinfo::UnsafeDescription(), tRows );
		}
		break;

		default:
			// default case, unknown command
			sError.SetSprintf ( "unknown command (code=%d)", uMysqlCmd );
			SendMysqlErrorPacket ( tOut, uPacketID, NULL, sError.cstr(), MYSQL_ERR_UNKNOWN_COM_ERROR );
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
void RunSingleSphinxqlCommand ( Str_t sCommand, ISphOutputBuffer & tOut )
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


// hack to interrupt session
// fixme! If more session-wide access expected, m.b. place hare more general things, like session pointer?
struct DebugClose_t : public TaskInfo_t
{
	DECLARE_RENDER( DebugClose_t );
	bool m_bClose = false;
};

// no specific render, just storage
DEFINE_RENDER( DebugClose_t ) {}

void DebugClose ()
{
	myinfo::ref<DebugClose_t> ()->m_bClose = true;
}

// main sphinxql server
void SqlServe ( AsyncNetBufferPtr_c pBuf )
{
	auto& tSess = session::Info();
	// to close current connection from inside
	auto pCloseFlag = PublishTaskInfo ( new DebugClose_t );

	// to display 'compressed' flag, if any.
	auto pCompressedFlag = PublishTaskInfo ( new QlCompressedInfo_t );

	tSess.SetProto ( Proto_e::MYSQL41 );

	// non-vip connections in maintainance should be already rejected on accept
	assert ( !g_bMaintenance || tSess.GetVip() );

	// set off query guard
	GlobalCrashQueryGetRef ().m_eType = QUERY_SQL;
	const bool bCanZlibCompression = IsZlibCompressionAvailable();
	const bool bCanZstdCompression = IsZstdCompressionAvailable();

	int iCID = tSess.GetConnID();
	const char * sClientIP = tSess.szClientName();

	// fixme! durty macros to transparently substitute pBuf on the fly (to ssl, compressed, whatever)
	#define tOut (*(NetGenericOutputBuffer_c*)pBuf)
	#define tIn (*(AsyncNetInputBuffer_c*)pBuf)

	/// mysql is pro-active, we NEED to send handshake before client send us something.
	/// So, no passive probing possible.
	// send handshake first
	tSess.SetTaskState ( TaskState_e::HANDSHAKE );
	sphLogDebugv ("Sending handshake...");
	SendMysqlProtoHandshake ( tOut, CheckWeCanUseSSL (), bCanZlibCompression, bCanZstdCompression, iCID );
	tSess.SetTaskState ( TaskState_e::NET_WRITE );
	if ( !tOut.Flush () )
	{
		int iErrno = sphSockGetErrno ();
		sphWarning ( "failed to send server version (client=%s(%d), error: %d '%s')",
				sClientIP, iCID, iErrno, sphSockError ( iErrno ) );
		return;
	}

	bool bAuthed = false;
	BYTE uPacketID = 1;
	int iPacketLen;
	int iTimeoutS = -1;
	do
	{
		// check for updated timeout
		auto iCurrentTimeout = tSess.GetTimeoutS(); // by default -1, means 'default'
		if ( iCurrentTimeout<0 )
			iCurrentTimeout = g_iClientQlTimeoutS;

		if ( iCurrentTimeout!=iTimeoutS )
		{
			iTimeoutS = iCurrentTimeout;
			tIn.SetTimeoutUS ( S2US * iTimeoutS );
		}

		tIn.DiscardProcessed ();
		iPacketLen = 0;

		// get next packet
		// we want interruptible calls here, so that shutdowns could be honored
		sphLogDebugv ( "Receiving command... %d bytes in buf", tIn.HasBytes() );

		// setup per-query profiling
		auto pProfile = session::StartProfiling ( SPH_QSTATE_TOTAL );
		if ( pProfile )
			tOut.SetProfiler ( pProfile );

		int iChunkLen = MAX_PACKET_LEN;

		auto iStartPacketPos = tIn.GetBufferPos ();
		while (iChunkLen==MAX_PACKET_LEN)
		{
			// inlined AsyncReadMySQLPacketHeader
			if ( !tIn.ReadFrom ( iPacketLen+4 ))
			{
				sphLogDebugv ( "conn %s(%d): bailing on failed MySQL header (sockerr=%s)",
						sClientIP, iCID, sphSockError ());
				return;
			}
			tIn.SetBufferPos ( iStartPacketPos + iPacketLen ); // will read at the end of the buffer
			DWORD uAddon = tIn.GetLSBDword ();
			tIn.DiscardProcessed ( sizeof ( uAddon )); // move out this header to keep rest of the buff solid
			tIn.SetBufferPos ( iStartPacketPos ); // rewind back after the read.
			uPacketID = 1+(BYTE) ( uAddon >> 24 );
			iChunkLen = ( uAddon & MAX_PACKET_LEN );

			sphLogDebugv ( "AsyncReadMySQLPacketHeader returned %d len...", iChunkLen );
			iPacketLen += iChunkLen;

			// receive package body
			if ( !tIn.ReadFrom ( iPacketLen ))
			{
				sphWarning ( "failed to receive MySQL request body (client=%s(%d), exp=%d, error='%s')",
						sClientIP, iCID, iPacketLen, sphSockError ());
				return;
			}
		}

		SwitchProfile ( pProfile, SPH_QSTATE_UNKNOWN );

		// handle auth packet
		if ( !bAuthed )
		{
			tSess.SetTaskState ( TaskState_e::HANDSHAKE );
			DWORD uClientCapabilities = tIn.GetLSBDword();
			auto tAnswer = tIn.PopTail ( iPacketLen - sizeof ( DWORD ) );

			// switch to ssl by demand.
			// You need to set a bit in handshake (g_sMysqlHandshake) in order to suggest client such switching.
			// Client set this desirable bit only if we say that 'we can' about it before.

			if ( !tSess.GetSsl() && UserWantsSSL( uClientCapabilities ) ) // want SSL
			{
				tSess.SetSsl ( MakeSecureLayer ( pBuf ) );
				tSess.SetPersistent( !tOut.GetError () );
				continue; // next packet will be 'login' again, but received over SSL
			}

			if ( IsMaxedOut() )
			{
				sphWarning ( "%s", g_sMaxedOutMessage );
				SendMysqlErrorPacket ( tOut, uPacketID, nullptr, g_sMaxedOutMessage, MYSQL_ERR_UNKNOWN_COM_ERROR );
				tOut.Flush ();
				gStats().m_iMaxedOut.fetch_add ( 1, std::memory_order_relaxed );
				break;
			}

			if ( UsernameIsFEDERATED ( tAnswer ))
				session::SetFederatedUser();
			SendMysqlOkPacket ( tOut, uPacketID, session::IsAutoCommit(), session::IsInTrans ());
			tSess.SetPersistent ( tOut.Flush () );
			bAuthed = true;

			if ( bCanZstdCompression && UserWantsZstdCompression ( uClientCapabilities ) )
			{
				MakeZstdMysqlCompressedLayer ( pBuf, UserWantsZstdCompressionLevel ( tAnswer ) );
				pCompressedFlag->m_bCompressed = true;
			}
			else if ( bCanZlibCompression && UserWantsCompression ( uClientCapabilities ) )
			{
				MakeZlibMysqlCompressedLayer ( pBuf );
				pCompressedFlag->m_bCompressed = true;
			}
			continue;
		}

		tSess.SetPersistent ( LoopClientMySQL ( uPacketID, iPacketLen, pProfile, pBuf ) && !pCloseFlag->m_bClose );
	} while ( tSess.GetPersistent() );
}
