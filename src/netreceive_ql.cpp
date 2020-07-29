//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
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
#include "compressed_mysql.h"

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

#define SPH_MYSQL_FLAG_STATUS_AUTOCOMMIT 2	// mysql.h: SERVER_STATUS_AUTOCOMMIT
#define SPH_MYSQL_FLAG_MORE_RESULTS 8		// mysql.h: SERVER_MORE_RESULTS_EXISTS

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

#define SPH_MYSQL_ERROR_MAX_LENGTH 512

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
	if ( iErrorLen>SPH_MYSQL_ERROR_MAX_LENGTH )
	{
		iErrorLen = SPH_MYSQL_ERROR_MAX_LENGTH;
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

void SendMysqlEofPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, int iWarns, bool bMoreResults, bool bAutoCommit )
{
	if ( iWarns<0 ) iWarns = 0;
	if ( iWarns>65535 ) iWarns = 65535;
	if ( bMoreResults )
		iWarns |= ( SPH_MYSQL_FLAG_MORE_RESULTS<<16 );
	if ( bAutoCommit )
		iWarns |= ( SPH_MYSQL_FLAG_STATUS_AUTOCOMMIT<<16 );

	tOut.SendLSBDword ( (uPacketID<<24) + 5 );
	tOut.SendByte ( 0xfe );
	tOut.SendLSBDword ( iWarns ); // N warnings, 0 status
}


// was defaults ( ISphOutputBuffer & tOut, BYTE uPacketID, int iAffectedRows=0, int iWarns=0, const char * sMessage=nullptr, bool bMoreResults=false, bool bAutoCommit )
void SendMysqlOkPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, int iAffectedRows, int iWarns, const char * sMessage, bool bMoreResults, bool bAutoCommit, int64_t iLastID )
{
	BYTE sVarLen[20] = {0}; // max 18 for packed number, +1 more just for fun
	BYTE * pBuf = sVarLen;
	pBuf = MysqlPackInt ( pBuf, iAffectedRows );
	pBuf = MysqlPackInt ( pBuf, iLastID );
	int iLen = pBuf - sVarLen;

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
		uWarnStatus |= SPH_MYSQL_FLAG_MORE_RESULTS;
	if ( bAutoCommit )
		uWarnStatus |= SPH_MYSQL_FLAG_STATUS_AUTOCOMMIT;

	tOut.SendLSBDword ( uWarnStatus );		// 0 status, N warnings
	if ( sMessage )
		tOut.SendBytes ( sMessage, iMsgLen );
}


void SendMysqlOkPacket ( ISphOutputBuffer & tOut, BYTE uPacketID, bool bAutoCommit )
{
	SendMysqlOkPacket ( tOut, uPacketID, 0, 0, nullptr, false, bAutoCommit, 0 );
}

//////////////////////////////////////////////////////////////////////////
// Mysql row buffer and command handler

class SqlRowBuffer_c : public RowBuffer_i, private LazyVector_T<BYTE>
{
	BYTE & m_uPacketID;
	ISphOutputBuffer & m_tOut;
	bool m_bAutoCommit = false;
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

	void SendSqlFieldPacket ( const char * sCol, MysqlColumnType_e eType, WORD uFlags )
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

public:

	SqlRowBuffer_c ( BYTE * pPacketID, ISphOutputBuffer * pOut, bool bAutoCommit )
		: m_uPacketID ( *pPacketID )
		, m_tOut ( *pOut )
		, m_bAutoCommit ( bAutoCommit )
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
	void Commit() override
	{
		m_tOut.SendLSBDword ( ((m_uPacketID++)<<24) + ( GetLength() ) );
		m_tOut.SendBytes ( *this );
		Resize(0);
	}

	// wrappers for popular packets
	void Eof ( bool bMoreResults, int iWarns ) override
	{
		SendMysqlEofPacket ( m_tOut, m_uPacketID++, iWarns, bMoreResults, m_bAutoCommit );
	}

	void Error ( const char * sStmt, const char * sError, MysqlErrors_e iErr ) override
	{
		SendMysqlErrorPacket ( m_tOut, m_uPacketID, sStmt, sError, iErr );
	}

	void Ok ( int iAffectedRows, int iWarns, const char * sMessage, bool bMoreResults, int64_t iLastInsertId ) override
	{
		SendMysqlOkPacket ( m_tOut, m_uPacketID, iAffectedRows, iWarns, sMessage, bMoreResults, m_bAutoCommit, iLastInsertId );
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

	void HeadEnd ( bool bMoreResults, int iWarns ) override
	{
		Eof ( bMoreResults, iWarns );
		Resize(0);
	}

	// add the next column. The EOF after the tull set will be fired automatically
	void HeadColumn ( const char * sName, MysqlColumnType_e uType, WORD uFlags ) override
	{
		assert ( m_iColumns-->0 && "you try to send more mysql columns than declared in InitHead" );
		SendSqlFieldPacket ( sName, uType, uFlags );
	}

	void Add ( BYTE uVal ) override
	{
		LazyVector_T<BYTE>::Add ( uVal );
	}
};

// send MySQL wire protocol handshake packets
void SendMysqlProtoHandshake ( ISphOutputBuffer& tOut, bool bSsl, bool bUseCompression, DWORD uConnID )
{
	// packed header here (packedID = 0)

	const BYTE uHandshake1 = 0x0A; // protocol version 10

	// z-terminated version string here

	// LSB DWORD connection id here

	const char sHandshake2[] =
		"\x01\x02\x03\x04\x05\x06\x07\x08" // auth-plugin-data-part-1
		"\x00"; // filler

	const BYTE uCapatibilities1stByte = 0x08; // Server capatibilities 1-st byte: CLIENT_CONNECT_WITH_DB
  	const BYTE uCapatibilities2ndByte = 0xC2; // server capabilities 2-nd byte: CLIENT_PROTOCOL_41 | CLIENT_SECURE_CONNECTION | INTERACTIVE_CLIENT

	const char sHandshake3[] =	"\x21" // server language; let it be ut8_general_ci to make different clients happy
		"\x02\x00" // server status AUTO_COMMIT
		"\x08\x00" // server capabilities hi WORD; CLIENT_PLUGIN_AUTH
		"\x15" // length of auth-plugin-data - 21 bytes
		"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // unuzed 10 bytes
		"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x00" // auth-plugin-data-part-2 (12 bytes + zero) (for auth, 4.1+)
		"mysql_native_password"; // auth plugin name. try to remove last z byte here...

	int iLen = g_sMySQLVersion.Length()+1; // +1 for z-terminator

	DWORD uHandshakeLen = 1 + iLen + 4 + sizeof(sHandshake2)-1 + 1 + 1 + sizeof(sHandshake3);
	tOut.SendLSBDword ( uHandshakeLen & 0x00FFFFFFU );
	tOut.SendByte( uHandshake1 );
	tOut.SendBytes ( g_sMySQLVersion.scstr(), iLen );
	tOut.SendLSBDword( uConnID );
	tOut.SendBytes ( sHandshake2, sizeof ( sHandshake2 )-1 );
	tOut.SendByte ( uCapatibilities1stByte | ( bUseCompression ? 0x20U : 0U ) );
	// fixme! SSL capability must be set only if keys are valid!
	tOut.SendByte ( uCapatibilities2ndByte | ( bSsl ? 8U : 0U ) );
	tOut.SendBytes ( sHandshake3, sizeof ( sHandshake3 ) ); // incl. z-terminator
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

inline bool UserWantsSSL ( const ByteBlob_t & tPacket )
{
	return ( tPacket.first[1] & 8 )!=0;
}

inline bool UserWantsCompression ( const ByteBlob_t & tPacket )
{
	return ( tPacket.first[0] & 0x20U)!=0;
}

bool LoopClientMySQL ( BYTE & uPacketID, SphinxqlSessionPublic & tSession, int iPacketLen,
		CSphQueryProfile * pProfile, AsyncNetBufferPtr_c pBuf )
{
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
			SendMysqlOkPacket ( tOut, uPacketID, tSession.IsAutoCommit() );
			break;

		case MYSQL_COM_SET_OPTION:
			// bMulti = ( tIn.GetWord()==MYSQL_OPTION_MULTI_STATEMENTS_ON ); // that's how we could double check and validate multi query
			// server reporting success in response to COM_SET_OPTION and COM_DEBUG
			SendMysqlEofPacket ( tOut, uPacketID, 0, false, tSession.IsAutoCommit () );
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
			myinfo::TaskState ( TaskState_e::QUERY );
			SqlRowBuffer_c tRows ( &uPacketID, &tOut, tSession.IsAutoCommit () );
			bKeepProfile = tSession.Execute ( myinfo::UnsafeDescription(), tRows );
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
	myinfo::TaskState ( TaskState_e::NET_WRITE );
	if ( !tOut.Flush () )
		return false;

	// finalize query profile
	if ( pProfile )
		pProfile->Stop();
	if ( uMysqlCmd==MYSQL_COM_QUERY && bKeepProfile )
		tSession.SaveLastProfile();
	tOut.SetProfiler ( nullptr );
	return true;
}

} // static namespace

// that is used from sphinxql command over API
void RunSingleSphinxqlCommand ( Str_t sCommand, ISphOutputBuffer & tOut )
{
	BYTE uDummy = 0;

	// todo: move upper, if the session variables are also necessary in API access mode.
	SphinxqlSessionPublic tSession; // FIXME!!! check that no accum related command used via API

	SqlRowBuffer_c tRows ( &uDummy, &tOut, true );
	tSession.Execute ( sCommand, tRows );
}

// add 'compressed' flag
struct QlCompressedInfo_t : public TaskInfo_t
{
	DECLARE_RENDER( QlCompressedInfo_t );
	bool m_bCompressed = false;
};

DEFINE_RENDER( QlCompressedInfo_t )
{
	auto & tInfo = *(QlCompressedInfo_t *) pSrc;
	if ( tInfo.m_bCompressed )
		dDst.m_sProto << "compressed";
	dDst.m_sChain << (int) tInfo.m_eType << ":QlCompressed ";
}

// main sphinxql server
void SqlServe ( AsyncNetBufferPtr_c pBuf )
{
	// to display 'compressed' flag, if any.
	auto pCompressedFlag = PublishTaskInfo ( new QlCompressedInfo_t );

	myinfo::SetProto ( Proto_e::MYSQL41 );

	// non-vip connections in maintainance should be already rejected on accept
	assert ( !g_bMaintenance || myinfo::IsVIP () );

	// set off query guard
	GlobalCrashQueryGetRef ().m_eType = QUERY_SQL;
	const bool bCanCompression = IsCompressionAvailable();

	int iCID = myinfo::ConnID();
	const char * sClientIP = myinfo::szClientName();

	// fixme! durty macros to transparently substitute pBuf on the fly (to ssl, compressed, whatever)
	#define tOut (*(NetGenericOutputBuffer_c*)pBuf)
	#define tIn (*(AsyncNetInputBuffer_c*)pBuf)

	/// mysql is pro-active, we NEED to send handshake before client send us something.
	/// So, no passive probing possible.
	// send handshake first
	myinfo::TaskState ( TaskState_e::HANDSHAKE );
	sphLogDebugv ("Sending handshake...");
	SendMysqlProtoHandshake ( tOut, CheckWeCanUseSSL (), bCanCompression, iCID );
	myinfo::TaskState ( TaskState_e::NET_WRITE );
	if ( !tOut.Flush () )
	{
		int iErrno = sphSockGetErrno ();
		sphWarning ( "failed to send server version (client=%s(%d), error: %d '%s')",
				sClientIP, iCID, iErrno, sphSockError ( iErrno ) );
		return;
	}

	SphinxqlSessionPublic tSession; // session variables and state
	bool bAuthed = false;
	BYTE uPacketID = 1;
	bool bKeepAlive;
	int iPacketLen;
	int iTimeoutS = -1;
	int iThrottlingMS = -1;
	int iDistThreads = 0;
	do
	{
		// check for updated timeout
		auto iCurrentTimeout = tSession.GetBackendTimeoutS(); // by default -1, means 'default'
		if ( iCurrentTimeout<0 )
			iCurrentTimeout = g_iClientQlTimeoutS;

		if ( iCurrentTimeout!=iTimeoutS )
		{
			iTimeoutS = iCurrentTimeout;
			tIn.SetTimeoutUS ( S2US * iTimeoutS );
		}

		// check for throttling
		auto iCurrentThrottling = tSession.GetBackendThrottlingMS();
		if ( iCurrentThrottling!=iThrottlingMS )
		{
			iThrottlingMS = iCurrentThrottling;
			myinfo::SetThrottlingPeriodMS ( iThrottlingMS );
		}

		// check for changed iDistThreads
		auto iCurrentDistThreads = tSession.GetBackendDistThreads ();
		if ( iCurrentDistThreads!=iDistThreads )
		{
			iDistThreads = iCurrentDistThreads;
			myinfo::SetDistThreads ( iDistThreads );
		}

		tIn.DiscardProcessed ();
		iPacketLen = 0;

		// get next packet
		// we want interruptible calls here, so that shutdowns could be honored
		sphLogDebugv ( "Receiving command... %d bytes in buf", tIn.HasBytes() );

		// setup per-query profiling
		auto pProfile = tSession.StartProfiling ( SPH_QSTATE_NET_READ ); // fixme! there is SPH_QSTATE_TOTAL there
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
			myinfo::TaskState ( TaskState_e::HANDSHAKE );
			auto tAnswer = tIn.PopTail ( iPacketLen );

			// switch to ssl by demand.
			// You need to set a bit in handshake (g_sMysqlHandshake) in order to suggest client such switching.
			// Client set this desirable bit only if we say that 'we can' about it before.

			if ( !myinfo::IsSSL() && UserWantsSSL(tAnswer) ) // want SSL
			{
				myinfo::SetSSL ( MakeSecureLayer ( pBuf ) );
				bKeepAlive = !tOut.GetError ();
				continue; // next packet will be 'login' again, but received over SSL
			}

			if ( IsMaxedOut() )
			{
				sphWarning ( "%s", g_sMaxedOutMessage );
				SendMysqlErrorPacket ( tOut, uPacketID, nullptr, g_sMaxedOutMessage, MYSQL_ERR_UNKNOWN_COM_ERROR );
				tOut.Flush ();
				break;
			}

			if ( UsernameIsFEDERATED ( tAnswer ))
				tSession.SetFederatedUser();
			SendMysqlOkPacket ( tOut, uPacketID, tSession.IsAutoCommit());
			bKeepAlive = tOut.Flush ();
			bAuthed = true;

			if ( bCanCompression && UserWantsCompression ( tAnswer ) )
			{
				MakeMysqlCompressedLayer ( pBuf );
				pCompressedFlag->m_bCompressed = true;
			}
			continue;
		}

		bKeepAlive = LoopClientMySQL ( uPacketID, tSession, iPacketLen, pProfile, pBuf );
	} while ( bKeepAlive );
}
