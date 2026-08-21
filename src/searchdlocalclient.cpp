// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)
//
// SQL client support for configless local searchd mode.

#include "searchdlocalinternal.h"
#include "searchdlocal.h"

#include "json/cJSON.h"
#include "sphinxint.h"
#include "sphinxjson.h"

#include <cerrno>
#include <algorithm>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#if !_WIN32
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>
#endif

namespace localmode
{

std::string TrimInput ( const std::string & sValue )
{
	CSphString sTrimmed = sValue.c_str();
	sTrimmed.Trim();
	return sTrimmed.cstr();
}

template<typename T>
T ClientError ( CSphString & sError, T tResult, const char * szMessage, const char * szDetail=nullptr )
{
	if ( szDetail ) sError.SetSprintf ( "%s: %s", szMessage, szDetail ); else sError = szMessage;
	return tResult;
}

JsonObj_c GetCaseSensitiveItem ( JsonObj_c & tObject, const char * szName )
{
	return JsonObj_c ( cJSON_GetObjectItemCaseSensitive ( tObject.GetRoot(), szName ), false );
}

struct LocalStatement_t
{
	std::string m_sSql;
	bool m_bVertical = false;
	bool m_bUnterminatedBlockComment = false;
};

std::vector<LocalStatement_t> SplitSqlBatch ( const char * szSql )
{
	enum class State_e { NORMAL, QUOTE, LINE_COMMENT, BLOCK_COMMENT };
	State_e eState = State_e::NORMAL;
	char cQuote = 0;
	std::vector<LocalStatement_t> dStatements;
	std::string sCurrent;
	const size_t iLength = szSql ? strlen ( szSql ) : 0;
	auto AddSpace = [&] { if ( !sCurrent.empty() && !isspace ( (unsigned char)sCurrent.back() ) ) sCurrent.push_back ( ' ' ); };
	auto AddStatement = [&] ( bool bVertical, bool bUnterminatedBlockComment=false )
	{
		std::string sStatement = TrimInput ( sCurrent );
		if ( !sStatement.empty() || bUnterminatedBlockComment )
			dStatements.push_back ( { std::move(sStatement), bVertical, bUnterminatedBlockComment } );
		sCurrent.clear();
	};

	for ( size_t i=0; i<iLength; ++i )
	{
		char c = szSql[i];
		if ( eState==State_e::QUOTE )
		{
			sCurrent.push_back ( c );
			if ( c=='\\' && i+1<iLength )
				sCurrent.push_back ( szSql[++i] );
			else if ( c==cQuote )
			{
				if ( i+1<iLength && szSql[i+1]==cQuote )
					sCurrent.push_back ( szSql[++i] );
				else
					eState = State_e::NORMAL;
			}
			continue;
		}
		if ( eState==State_e::LINE_COMMENT )
		{
			if ( c=='\n' || c=='\r' )
			{
				AddSpace();
				eState = State_e::NORMAL;
			}
			continue;
		}
		if ( eState==State_e::BLOCK_COMMENT )
		{
			if ( c=='*' && i+1<iLength && szSql[i+1]=='/' )
			{
				AddSpace();
				++i;
				eState = State_e::NORMAL;
			}
			continue;
		}

		if ( c=='\'' || c=='"' || c=='`' )
		{
			cQuote = c;
			eState = State_e::QUOTE;
			sCurrent.push_back ( c );
		}
		else if ( c=='#' )
		{
			AddSpace();
			eState = State_e::LINE_COMMENT;
		}
		else if ( c=='-' && i+1<iLength && szSql[i+1]=='-' && ( i+2==iLength || isspace ( (unsigned char)szSql[i+2] ) ) )
		{
			AddSpace();
			++i;
			eState = State_e::LINE_COMMENT;
		}
		else if ( c=='/' && i+1<iLength && szSql[i+1]=='*' )
		{
			AddSpace();
			++i;
			eState = State_e::BLOCK_COMMENT;
		}
		else if ( c==';' )
			AddStatement ( false );
		else if ( c=='\\' && i+1<iLength && szSql[i+1]=='G' )
		{
			AddStatement ( true );
			++i;
		}
		else
			sCurrent.push_back ( c );
	}

	AddStatement ( false, eState==State_e::BLOCK_COMMENT );
	return dStatements;
}

std::vector<std::string> SplitSqlStatements ( const char * szSql )
{
	std::vector<std::string> dStatements;
	for ( LocalStatement_t & tStatement : SplitSqlBatch(szSql) )
		if ( !tStatement.m_sSql.empty() )
			dStatements.push_back ( std::move(tStatement.m_sSql) );
	return dStatements;
}

#if !_WIN32
bool HasSchemaChange ( const std::string & sLine )
{
	for ( const std::string & sStatement : SplitSqlStatements ( sLine.c_str() ) )
	{
		size_t iWordEnd = 0;
		while ( iWordEnd<sStatement.size() && isalpha ( (unsigned char)sStatement[iWordEnd] ) )
			++iWordEnd;
		std::string sCommand = sStatement.substr ( 0, iWordEnd );
		if ( strcasecmp ( sCommand.c_str(), "CREATE" )==0 || strcasecmp ( sCommand.c_str(), "ALTER" )==0
			|| strcasecmp ( sCommand.c_str(), "DROP" )==0 || strcasecmp ( sCommand.c_str(), "TRUNCATE" )==0 )
			return true;
	}
	return false;
}

bool ReportsAffectedRows ( const std::string & sStatement )
{
	std::string sTrimmed = TrimInput ( sStatement );
	static constexpr const char * dStatements[] = { "DELETE", "INSERT", "REPLACE", "UPDATE" };
	for ( const char * szKeyword : dStatements )
	{
		size_t iLength = strlen ( szKeyword );
		if ( sTrimmed.size()>=iLength && strncasecmp ( sTrimmed.c_str(), szKeyword, iLength )==0
			&& ( sTrimmed.size()==iLength || isspace ( (unsigned char)sTrimmed[iLength] ) ) )
			return true;
	}
	return false;
}
#endif

#if !_WIN32
int64_t EffectiveDeadline ( int64_t iDeadlineUS )
{
	return iDeadlineUS ? iDeadlineUS : sphMicroTimer()+30000000;
}

bool WaitSocket ( int iSocket, short iEvents, int64_t iDeadlineUS, CSphString & sError )
{
	while ( true )
	{
		int64_t iRemaining = iDeadlineUS-sphMicroTimer();
		if ( iRemaining<=0 )
			return ClientError ( sError, false, "Manticore socket operation timed out" );
		pollfd tPoll { iSocket, iEvents, 0 };
		int iWaitMS = (int)std::min<int64_t> ( ( iRemaining+999 )/1000, INT_MAX );
		int iReady = poll ( &tPoll, 1, iWaitMS );
		if ( iReady>0 )
			return true;
		if ( iReady<0 && errno==EINTR )
			return ClientError ( sError, false, "Manticore socket operation interrupted" );
		if ( iReady<0 )
			return ClientError ( sError, false, "failed waiting for Manticore socket", strerror(errno) );
	}
}

bool SendAll ( int iSocket, const char * pData, size_t iLength, CSphString & sError, int64_t iDeadlineUS=0 )
{
	iDeadlineUS = EffectiveDeadline ( iDeadlineUS );
	while ( iLength )
	{
		ssize_t iSent = send ( iSocket, pData, iLength, MSG_NOSIGNAL );
		if ( iSent<0 && ( errno==EAGAIN || errno==EWOULDBLOCK ) )
		{
			if ( !WaitSocket(iSocket,POLLOUT,iDeadlineUS,sError) )
				return false;
			continue;
		}
		if ( iSent<0 && errno==EINTR )
			return ClientError ( sError, false, "Manticore socket operation interrupted" );
		if ( iSent<=0 )
			return ClientError ( sError, false, "failed to send query", strerror(errno) );
		pData += iSent;
		iLength -= (size_t)iSent;
	}
	return true;
}

bool StartNonblockingConnect ( int iSocket, const sockaddr * pAddress, socklen_t iLength, int64_t iDeadlineUS, CSphString & sError )
{
	int iFlags = fcntl ( iSocket, F_GETFL, 0 );
	if ( iFlags<0 || fcntl(iSocket,F_SETFL,iFlags|O_NONBLOCK)<0 )
		return ClientError ( sError, false, "failed to make Manticore socket nonblocking", strerror(errno) );
	if ( connect(iSocket,pAddress,iLength)==0 )
		return true;
	if ( errno!=EINPROGRESS && errno!=EWOULDBLOCK )
		return false;
	if ( !WaitSocket(iSocket,POLLOUT,iDeadlineUS,sError) )
		return false;
	int iConnectError = 0;
	socklen_t iErrorLength = sizeof(iConnectError);
	if ( getsockopt(iSocket,SOL_SOCKET,SO_ERROR,&iConnectError,&iErrorLength)<0 || iConnectError )
	{
		errno = iConnectError ? iConnectError : errno;
		return false;
	}
	return true;
}

int ConnectSocket ( CSphString & sError, const SqlEndpoint_t & tEndpoint, int64_t iDeadlineUS )
{
	iDeadlineUS = EffectiveDeadline ( iDeadlineUS );
	sError = "";
	if ( tEndpoint.m_sUnix.IsEmpty() )
	{
		int iSocket = socket ( AF_INET, SOCK_STREAM, 0 );
		if ( iSocket<0 )
			return ClientError ( sError, -1, "failed to create configured socket", strerror(errno) );

		sockaddr_in tAddress {};
		tAddress.sin_family = AF_INET;
		tAddress.sin_port = htons ( tEndpoint.m_iPort );
		tAddress.sin_addr.s_addr = tEndpoint.m_uIP;
		if ( !StartNonblockingConnect(iSocket,(sockaddr*)&tAddress,sizeof(tAddress),iDeadlineUS,sError) )
		{
			if ( sError.IsEmpty() )
				sError.SetSprintf ( "cannot connect to Manticore instance at %s: %s", tEndpoint.m_sDescription.cstr(), strerror(errno) );
			close ( iSocket );
			return -1;
		}
		return iSocket;
	}

	sockaddr_un tAddress {};
	if ( tEndpoint.m_sUnix.Length()>= (int)sizeof(tAddress.sun_path) )
		return ClientError ( sError, -1, "Manticore socket path is too long", tEndpoint.m_sUnix.cstr() );

	int iSocket = socket ( AF_UNIX, SOCK_STREAM, 0 );
	if ( iSocket<0 )
		return ClientError ( sError, -1, "failed to create Manticore socket", strerror(errno) );

	tAddress.sun_family = AF_UNIX;
	strncpy ( tAddress.sun_path, tEndpoint.m_sUnix.cstr(), sizeof(tAddress.sun_path)-1 );
	if ( !StartNonblockingConnect(iSocket,(sockaddr*)&tAddress,sizeof(tAddress),iDeadlineUS,sError) )
	{
		if ( sError.IsEmpty() )
			sError.SetSprintf ( "cannot connect to Manticore instance at %s: %s", tEndpoint.m_sDescription.cstr(), strerror(errno) );
		close ( iSocket );
		return -1;
	}
	return iSocket;
}

bool IsSocketClosed ( int iSocket )
{
	char cProbe;
	ssize_t iRead = recv ( iSocket, &cProbe, 1, MSG_PEEK | MSG_DONTWAIT );
	return iRead==0 || ( iRead<0 && errno!=EAGAIN && errno!=EWOULDBLOCK && errno!=EINTR );
}

bool ReadHttpResponse ( int iSocket, int & iStatus, std::string & sBody, CSphString & sError, int64_t iDeadlineUS=0 )
{
	iDeadlineUS = EffectiveDeadline ( iDeadlineUS );
	std::string sResponse;
	char sBuffer[8192];
	size_t iHeaderEnd = std::string::npos;

	while ( ( iHeaderEnd=sResponse.find("\r\n\r\n") )==std::string::npos )
	{
		ssize_t iRead = recv ( iSocket, sBuffer, sizeof(sBuffer), 0 );
		if ( iRead<0 && ( errno==EAGAIN || errno==EWOULDBLOCK ) )
		{
			if ( !WaitSocket(iSocket,POLLIN,iDeadlineUS,sError) ) return false;
			continue;
		}
		if ( iRead<0 && errno==EINTR ) return ClientError ( sError, false, "Manticore socket operation interrupted" );
		if ( iRead<=0 )
			return ClientError ( sError, false, "failed to read Manticore HTTP response", iRead==0 ? "connection closed" : strerror(errno) );
		sResponse.append ( sBuffer, (size_t)iRead );
		if ( sResponse.size()>1024*1024 )
			return ClientError ( sError, false, "Manticore HTTP response header is too large" );
	}

	if ( sscanf ( sResponse.c_str(), "HTTP/%*d.%*d %d", &iStatus )!=1 )
		return ClientError ( sError, false, "invalid response from Manticore instance" );

	size_t iContentLength = std::string::npos;
	size_t iLine = sResponse.find ( "\r\n" )+2;
	while ( iLine<iHeaderEnd )
	{
		size_t iLineEnd = sResponse.find ( "\r\n", iLine );
		std::string sHeader = sResponse.substr ( iLine, iLineEnd-iLine );
		if ( strncasecmp ( sHeader.c_str(), "content-length:", 15 )==0 )
			iContentLength = strtoull ( sHeader.c_str()+15, nullptr, 10 );
		iLine = iLineEnd+2;
	}

	if ( iContentLength==std::string::npos )
		return ClientError ( sError, false, "Manticore HTTP response has no Content-Length" );

	const size_t iBodyBegin = iHeaderEnd+4;
	while ( sResponse.size()-iBodyBegin<iContentLength )
	{
		ssize_t iRead = recv ( iSocket, sBuffer, sizeof(sBuffer), 0 );
		if ( iRead<0 && ( errno==EAGAIN || errno==EWOULDBLOCK ) )
		{
			if ( !WaitSocket(iSocket,POLLIN,iDeadlineUS,sError) ) return false;
			continue;
		}
		if ( iRead<0 && errno==EINTR ) return ClientError ( sError, false, "Manticore socket operation interrupted" );
		if ( iRead<=0 )
			return ClientError ( sError, false, "incomplete Manticore HTTP response", iRead==0 ? "connection closed" : strerror(errno) );
		sResponse.append ( sBuffer, (size_t)iRead );
	}

	sBody.assign ( sResponse, iBodyBegin, iContentLength );
	return true;
}

QueryResult_e ProbeEndpoint ( const SqlEndpoint_t & tEndpoint, CSphString & sError, int64_t iDeadlineUS )
{
	iDeadlineUS = EffectiveDeadline ( iDeadlineUS );
	int iSocket = ConnectSocket ( sError, tEndpoint, iDeadlineUS );
	if ( iSocket<0 )
		return QueryResult_e::CONNECTION_ERROR;
	AT_SCOPE_EXIT ( [&] { close ( iSocket ); } );

	static constexpr char sQuery[] = "SELECT 1";
	std::string sRequest = "POST /sql?mode=raw HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\nConnection: close\r\nContent-Length: ";
	sRequest += std::to_string ( sizeof(sQuery)-1 );
	sRequest += "\r\n\r\n";
	sRequest += sQuery;
	if ( !SendAll ( iSocket, sRequest.data(), sRequest.size(), sError, iDeadlineUS ) )
		return QueryResult_e::CONNECTION_ERROR;

	int iStatus = 0;
	std::string sBody;
	if ( !ReadHttpResponse ( iSocket, iStatus, sBody, sError, iDeadlineUS ) )
		return QueryResult_e::CONNECTION_ERROR;
	if ( iStatus<200 || iStatus>=300 )
	{
		sError.SetSprintf ( "Manticore readiness query returned HTTP status %d", iStatus );
		return QueryResult_e::SQL_ERROR;
	}
	JsonObj_c tRoot ( Str_t { sBody.data(), (int)sBody.size() } );
	if ( !tRoot || !tRoot.IsArray() || tRoot.Size()!=1 )
	{
		sError = "invalid response to Manticore readiness query";
		return QueryResult_e::SQL_ERROR;
	}
	JsonObj_c tResult;
	for ( JsonObj_c tItem : tRoot )
	{
		tResult = std::move(tItem);
		break;
	}
	JsonObj_c tError = GetCaseSensitiveItem ( tResult, "error" );
	if ( tError && tError.IsStr() && *tError.SzVal() )
	{
		sError = tError.SzVal();
		return QueryResult_e::SQL_ERROR;
	}
	JsonObj_c tRows = GetCaseSensitiveItem ( tResult, "data" );
	if ( !tRows || !tRows.IsArray() || tRows.Size()!=1 )
	{
		sError = "invalid data in Manticore readiness response";
		return QueryResult_e::SQL_ERROR;
	}
	JsonObj_c tRow;
	for ( JsonObj_c tItem : tRows )
	{
		tRow = std::move(tItem);
		break;
	}
	JsonObj_c tValue = GetCaseSensitiveItem ( tRow, "1" );
	if ( !tValue || !( tValue.IsInt() || tValue.IsUint() ) || tValue.IntVal()!=1 )
	{
		sError = "unexpected result from Manticore readiness query";
		return QueryResult_e::SQL_ERROR;
	}
	return QueryResult_e::OK;
}
#endif

std::string EscapeValue ( const char * szValue )
{
	if ( !szValue )
		return "NULL";

	std::string sResult;
	for ( const unsigned char * p=(const unsigned char*)szValue; *p; ++p )
	{
		switch ( *p )
		{
		case '\n': sResult += "\\n"; break;
		case '\r': sResult += "\\r"; break;
		case 9: sResult.push_back ( '\\' ); sResult.push_back ( 't' ); break;
		case '\\': sResult += "\\\\"; break;
		default: sResult.push_back ( (char)*p ); break;
		}
	}
	return sResult;
}

std::string JsonValueToString ( const JsonObj_c & tValue )
{
	if ( !tValue || tValue.IsNull() )
		return "NULL";
	if ( tValue.IsStr() )
		return EscapeValue ( tValue.SzVal() );
	if ( tValue.IsBool() )
		return tValue.BoolVal() ? "1" : "0";
	if ( tValue.IsInt() )
		return std::to_string ( tValue.IntVal() );
	if ( tValue.IsUint() )
		return std::to_string ( (uint64_t)tValue.IntVal() );
	if ( tValue.IsDbl() )
	{
		char szNumber[64];
		snprintf ( szNumber, sizeof(szNumber), "%.17g", tValue.DblVal() );
		return szNumber;
	}

	return EscapeValue ( tValue.AsString().cstr() );
}

struct LocalColumn_t
{
	std::string m_sName;
	bool m_bNumeric = false;
};

bool IsUtf8Terminal()
{
	const char * szLocale = getenv ( "LC_ALL" );
	if ( !szLocale || !*szLocale ) szLocale = getenv ( "LC_CTYPE" );
	if ( !szLocale || !*szLocale ) szLocale = getenv ( "LANG" );
	if ( !szLocale )
		return false;
	std::string sLocale = szLocale;
	std::transform ( sLocale.begin(), sLocale.end(), sLocale.begin(), [] ( unsigned char c ) { return (char)toupper(c); } );
	return sLocale.find("UTF-8")!=std::string::npos || sLocale.find("UTF8")!=std::string::npos;
}

struct FrameChars_t
{
	const char * m_szTopLeft;
	const char * m_szTopMiddle;
	const char * m_szTopRight;
	const char * m_szMiddleLeft;
	const char * m_szMiddle;
	const char * m_szMiddleRight;
	const char * m_szBottomLeft;
	const char * m_szBottomMiddle;
	const char * m_szBottomRight;
	const char * m_szHorizontal;
	const char * m_szVertical;
	const char * m_szEllipsis;
};

FrameChars_t GetFrameChars()
{
	if ( IsUtf8Terminal() )
		return { "┌", "┬", "┐", "├", "┼", "┤", "└", "┴", "┘", "─", "│", "…" };
	return { "+", "+", "+", "+", "+", "+", "+", "+", "+", "-", "|", "~" };
}

size_t CodepointWidth ( int iCode )
{
	bool bCombining = ( iCode>=0x0300 && iCode<=0x036F ) || ( iCode>=0x1AB0 && iCode<=0x1AFF )
		|| ( iCode>=0x1DC0 && iCode<=0x1DFF ) || ( iCode>=0x20D0 && iCode<=0x20FF ) || ( iCode>=0xFE20 && iCode<=0xFE2F );
	bool bWide = ( iCode>=0x1100 && iCode<=0x115F ) || iCode==0x2329 || iCode==0x232A
		|| ( iCode>=0x2E80 && iCode<=0xA4CF && iCode!=0x303F ) || ( iCode>=0xAC00 && iCode<=0xD7A3 )
		|| ( iCode>=0xF900 && iCode<=0xFAFF ) || ( iCode>=0xFE10 && iCode<=0xFE19 )
		|| ( iCode>=0xFE30 && iCode<=0xFE6F ) || ( iCode>=0xFF00 && iCode<=0xFF60 )
		|| ( iCode>=0xFFE0 && iCode<=0xFFE6 ) || ( iCode>=0x1F300 && iCode<=0x1FAFF )
		|| ( iCode>=0x20000 && iCode<=0x3FFFD );
	return bCombining ? 0 : bWide ? 2 : 1;
}

size_t TerminalWidth ( const std::string & sValue )
{
	size_t iWidth = 0;
	const BYTE * p = (const BYTE*)sValue.c_str();
	while ( *p )
	{
		int iCode = sphUTF8Decode ( p );
		iWidth += iCode<0 ? 1 : CodepointWidth ( iCode );
	}
	return iWidth;
}

std::string TruncateCell ( const std::string & sValue, size_t iLimit, const char * szEllipsis )
{
	if ( TerminalWidth(sValue)<=iLimit )
		return sValue;
	if ( !iLimit )
		return {};
	std::string sResult;
	size_t iWidth = 0;
	const BYTE * p = (const BYTE*)sValue.c_str();
	while ( *p )
	{
		const BYTE * pStart = p;
		int iCode = sphUTF8Decode ( p );
		size_t iCodeWidth = iCode<0 ? 1 : CodepointWidth ( iCode );
		if ( iWidth+iCodeWidth>=iLimit )
			break;
		sResult.append ( (const char*)pStart, p-pStart );
		iWidth += iCodeWidth;
	}
	return sResult + szEllipsis;
}

void LimitWidthsToTerminal ( std::vector<size_t> & dWidths )
{
#if !_WIN32
	winsize tSize {};
	if ( ioctl ( STDOUT_FILENO, TIOCGWINSZ, &tSize )<0 || !tSize.ws_col || dWidths.empty() )
		return;
	size_t iFixed = dWidths.size()*3+1;
	if ( iFixed+dWidths.size()>tSize.ws_col )
		return;
	size_t iAvailable = tSize.ws_col-iFixed;
	size_t iTotal = 0;
	for ( size_t iWidth : dWidths )
		iTotal += iWidth;
	if ( iTotal<=iAvailable )
		return;
	size_t iInitial = Max ( (size_t)1, iAvailable/dWidths.size() );
	std::vector<size_t> dOriginal = dWidths;
	for ( size_t & iWidth : dWidths )
		iWidth = Min ( iWidth, iInitial );
	iTotal = 0;
	for ( size_t iWidth : dWidths )
		iTotal += iWidth;
	while ( iTotal<iAvailable )
	{
		bool bChanged = false;
		for ( size_t i=0; i<dWidths.size() && iTotal<iAvailable; ++i )
			if ( dWidths[i]<dOriginal[i] )
			{
				++dWidths[i];
				++iTotal;
				bChanged = true;
			}
		if ( !bChanged )
			break;
	}
#endif
}

bool IsNumericType ( const char * szType )
{
	if ( !szType )
		return false;
	std::string sType = szType;
	for ( char & c : sType )
		c = (char)tolower ( (unsigned char)c );
	for ( const char * szNumeric : { "int", "long", "float", "double", "decimal", "uint", "bool" } )
		if ( sType.find ( szNumeric )!=std::string::npos )
			return true;
	return false;
}

void PrintBorder ( const char * szLeft, const char * szMiddle, const char * szRight, const char * szHorizontal, const std::vector<size_t> & dWidths )
{
	fputs ( szLeft, stdout );
	for ( size_t i=0; i<dWidths.size(); ++i )
	{
		if ( i )
			fputs ( szMiddle, stdout );
		for ( size_t j=0; j<dWidths[i]+2; ++j )
			fputs ( szHorizontal, stdout );
	}
	fprintf ( stdout, "%s\n", szRight );
}

void PrintInteractiveRow ( const std::vector<std::string> & dRow, const std::vector<size_t> & dWidths, const std::vector<LocalColumn_t> & dColumns, const FrameChars_t & tFrame, bool bHeader=false )
{
	fputs ( tFrame.m_szVertical, stdout );
	for ( size_t i=0; i<dRow.size(); ++i )
	{
		std::string sCell = TruncateCell ( dRow[i], dWidths[i], tFrame.m_szEllipsis );
		fputc ( ' ', stdout );
		size_t iPadding = dWidths[i]-TerminalWidth ( sCell );
		if ( !bHeader && dColumns[i].m_bNumeric )
			fprintf ( stdout, "%*s", (int)iPadding, "" );
		fputs ( sCell.c_str(), stdout );
		if ( bHeader || !dColumns[i].m_bNumeric )
			fprintf ( stdout, "%*s", (int)iPadding, "" );
		fprintf ( stdout, " %s", tFrame.m_szVertical );
	}
	fputc ( '\n', stdout );
}

void PrintVerticalRows ( const std::vector<std::string> & dColumnNames, const std::vector<std::vector<std::string>> & dRows )
{
	size_t iNameWidth = 0;
	for ( const std::string & sName : dColumnNames )
		iNameWidth = Max ( iNameWidth, TerminalWidth(sName) );

	for ( size_t iRow=0; iRow<dRows.size(); ++iRow )
	{
		fprintf ( stdout, "*************************** %zu. row ***************************\n", iRow+1 );
		for ( size_t iColumn=0; iColumn<dColumnNames.size(); ++iColumn )
		{
			fprintf ( stdout, "%*s", (int)( iNameWidth-TerminalWidth(dColumnNames[iColumn]) ), "" );
			fprintf ( stdout, "%s: %s\n", dColumnNames[iColumn].c_str(), dRows[iRow][iColumn].c_str() );
		}
	}
}

void PrintElapsed ( int64_t iElapsedUS )
{
	if ( iElapsedUS>=0 )
		fprintf ( stdout, " (%.3f ms)", iElapsedUS/1000.0 );
	fputc ( '\n', stdout );
}

bool PrintSqlResponse ( const std::string & sBody, CSphString & sError, bool bInteractive, int64_t iElapsedUS, bool bAffectedRows, bool bVertical )
{
	JsonObj_c tRoot ( Str_t { sBody.data(), (int)sBody.size() } );
	if ( !tRoot )
		return ClientError ( sError, false, "invalid JSON response from Manticore instance" );

	if ( !tRoot.IsArray() )
	{
		JsonObj_c tError = GetCaseSensitiveItem ( tRoot, "error" );
		sError = tError && tError.IsStr() ? tError.SzVal() : sBody.c_str();
		return false;
	}

	for ( JsonObj_c tResult : tRoot )
	{
		JsonObj_c tError = GetCaseSensitiveItem ( tResult, "error" );
		if ( tError && tError.IsStr() && *tError.SzVal() )
		{
			sError = tError.SzVal();
			return false;
		}

		std::vector<LocalColumn_t> dColumnInfo;
		JsonObj_c tColumns = GetCaseSensitiveItem ( tResult, "columns" );
		if ( tColumns && tColumns.IsArray() )
			for ( JsonObj_c tColumn : tColumns )
			{
				if ( !tColumn.IsObj() )
					return ClientError ( sError, false, "invalid column metadata in Manticore response" );
				LocalColumn_t tInfo;
				for ( JsonObj_c tMetadata : tColumn )
				{
					if ( tMetadata.Name() )
						tInfo.m_sName = tMetadata.Name();
					JsonObj_c tType = GetCaseSensitiveItem ( tMetadata, "type" );
					tInfo.m_bNumeric = tType && tType.IsStr() && IsNumericType ( tType.SzVal() );
					break;
				}
				if ( tInfo.m_sName.empty() )
					return ClientError ( sError, false, "invalid column metadata in Manticore response" );
				dColumnInfo.push_back ( std::move(tInfo) );
			}

		std::vector<std::vector<std::string>> dRows;
		std::vector<size_t> dWidths;
		dWidths.reserve ( dColumnInfo.size() );
		for ( const LocalColumn_t & tColumn : dColumnInfo )
			dWidths.push_back ( TerminalWidth ( tColumn.m_sName ) );

		JsonObj_c tRows = GetCaseSensitiveItem ( tResult, "data" );
		if ( tRows && tRows.IsArray() )
			for ( JsonObj_c tRow : tRows )
			{
				std::vector<std::string> dRow;
				dRow.reserve ( dColumnInfo.size() );
				for ( size_t i=0; i<dColumnInfo.size(); ++i )
				{
					dRow.push_back ( JsonValueToString ( GetCaseSensitiveItem ( tRow, dColumnInfo[i].m_sName.c_str() ) ) );
					dWidths[i] = Max ( dWidths[i], TerminalWidth ( dRow.back() ) );
				}
				dRows.push_back ( std::move(dRow) );
			}

		if ( !dColumnInfo.empty() && ( !bInteractive || !dRows.empty() ) )
		{
			std::vector<std::string> dColumnNames;
			for ( const LocalColumn_t & tColumn : dColumnInfo )
				dColumnNames.push_back ( tColumn.m_sName );
			if ( bVertical )
				PrintVerticalRows ( dColumnNames, dRows );
			else if ( bInteractive )
			{
				FrameChars_t tFrame = GetFrameChars();
				LimitWidthsToTerminal ( dWidths );
				PrintBorder ( tFrame.m_szTopLeft, tFrame.m_szTopMiddle, tFrame.m_szTopRight, tFrame.m_szHorizontal, dWidths );
				PrintInteractiveRow ( dColumnNames, dWidths, dColumnInfo, tFrame, true );
				PrintBorder ( tFrame.m_szMiddleLeft, tFrame.m_szMiddle, tFrame.m_szMiddleRight, tFrame.m_szHorizontal, dWidths );
				for ( const auto & dRow : dRows )
					PrintInteractiveRow ( dRow, dWidths, dColumnInfo, tFrame );
				PrintBorder ( tFrame.m_szBottomLeft, tFrame.m_szBottomMiddle, tFrame.m_szBottomRight, tFrame.m_szHorizontal, dWidths );
			}
			else
			{
				auto PrintTsv = [] ( const std::vector<std::string> & dRow )
				{
					for ( size_t i=0; i<dRow.size(); ++i )
					{
						if ( i ) fputc ( '	', stdout );
						fputs ( dRow[i].c_str(), stdout );
					}
					fputc ( '\n', stdout );
				};
				PrintTsv ( dColumnNames );
				for ( const auto & dRow : dRows )
					PrintTsv ( dRow );
			}
		}

		if ( bInteractive )
		{
			if ( !dColumnInfo.empty() )
			{
				if ( dRows.empty() )
					fputs ( "Empty set", stdout );
				else
					fprintf ( stdout, "%zu row%s in set", dRows.size(), dRows.size()==1 ? "" : "s" );
			}
			else
			{
				JsonObj_c tTotal = GetCaseSensitiveItem ( tResult, "total" );
				int64_t iAffected = tTotal && ( tTotal.IsInt() || tTotal.IsUint() ) ? tTotal.IntVal() : 0;
				if ( bAffectedRows || iAffected )
					fprintf ( stdout, "Query OK, %lld row%s affected", (long long)iAffected, iAffected==1 ? "" : "s" );
				else
					fputs ( "Query OK", stdout );
			}
			PrintElapsed ( iElapsedUS );
		}

		JsonObj_c tWarning = GetCaseSensitiveItem ( tResult, "warning" );
		if ( tWarning && tWarning.IsStr() && *tWarning.SzVal() )
			fprintf ( stderr, "Warning: %s\n", tWarning.SzVal() );
	}

	return true;
}

bool ParseHttpError ( const std::string & sBody, CSphString & sError )
{
	JsonObj_c tRoot ( Str_t { sBody.data(), (int)sBody.size() } );
	if ( !tRoot )
		return ClientError ( sError, false, sBody.c_str() );
	JsonObj_c tError = GetCaseSensitiveItem ( tRoot, "error" );
	sError = tError && tError.IsStr() ? tError.SzVal() : sBody.c_str();
	return true;
}

#if !_WIN32
bool FetchCompletionColumn ( const std::string & sStatement, std::vector<std::string> & dValues, const SqlEndpoint_t & tEndpoint )
{
	CSphString sError;
	int iSocket = ConnectSocket ( sError, tEndpoint );
	if ( iSocket<0 )
		return false;
	AT_SCOPE_EXIT ( [&] { close ( iSocket ); } );

	std::string sRequest = "POST /sql?mode=raw HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\nConnection: close\r\nContent-Length: ";
	sRequest += std::to_string ( sStatement.size() );
	sRequest += "\r\n\r\n";
	sRequest += sStatement;
	if ( !SendAll ( iSocket, sRequest.data(), sRequest.size(), sError ) )
		return false;

	int iStatus = 0;
	std::string sBody;
	if ( !ReadHttpResponse ( iSocket, iStatus, sBody, sError ) || iStatus<200 || iStatus>=300 )
		return false;
	JsonObj_c tRoot ( Str_t { sBody.data(), (int)sBody.size() } );
	if ( !tRoot || !tRoot.IsArray() || !tRoot.Size() )
		return false;
	JsonObj_c tResult;
	for ( JsonObj_c tItem : tRoot )
	{
		tResult = std::move(tItem);
		break;
	}
	JsonObj_c tColumns = GetCaseSensitiveItem ( tResult, "columns" );
	JsonObj_c tRows = GetCaseSensitiveItem ( tResult, "data" );
	if ( !tColumns || !tColumns.IsArray() || !tColumns.Size() || !tRows || !tRows.IsArray() )
		return false;
	JsonObj_c tFirstColumn;
	for ( JsonObj_c tColumn : tColumns )
	{
		tFirstColumn = std::move(tColumn);
		break;
	}
	const char * szName = nullptr;
	for ( JsonObj_c tMetadata : tFirstColumn )
	{
		szName = tMetadata.Name();
		break;
	}
	if ( !szName )
		return false;
	for ( JsonObj_c tRow : tRows )
	{
		JsonObj_c tValue = GetCaseSensitiveItem ( tRow, szName );
		if ( tValue && tValue.IsStr() )
			dValues.emplace_back ( tValue.SzVal() );
	}
	return true;
}

std::string QuoteCompletionIdentifier ( const std::string & sIdentifier )
{
	std::string sQuoted = "`";
	for ( char c : sIdentifier )
	{
		sQuoted.push_back ( c );
		if ( c=='`' )
			sQuoted.push_back ( '`' );
	}
	sQuoted.push_back ( '`' );
	return sQuoted;
}
#endif

CompletionProvider_fn CreateCompletionProvider ( SqlEndpoint_t tEndpoint )
{
	auto pCachedWords = std::make_shared<std::vector<std::string>>();
	return [pCachedWords,tEndpoint=std::move(tEndpoint)] ( const std::string & sLine )
	{
		static constexpr const char * dKeywords[] =
		{
			"ALTER", "AND", "AS", "ASC", "BEGIN", "BETWEEN", "BY", "CALL", "COMMIT", "CREATE",
			"DELETE", "DESC", "DESCRIBE", "DISTINCT", "DROP", "FACET", "FALSE", "FROM", "GROUP", "HAVING",
			"IN", "INDEX", "INSERT", "INTO", "JOIN", "KILL", "LIKE", "LIMIT", "MATCH", "NOT", "NULL",
			"OFFSET", "OPTION", "OR", "ORDER", "REPLACE", "ROLLBACK", "SELECT", "SET", "SHOW", "TABLE",
			"TABLES", "TRANSACTION", "TRUE", "TRUNCATE", "UNION", "UPDATE", "VALUES", "WHERE",
			"BIGINT", "BOOL", "DOUBLE", "FLOAT", "FLOAT_VECTOR", "INTEGER", "JSON", "MULTI", "MULTI64", "STRING", "TEXT", "TIMESTAMP",
			"charset_table", "html_strip", "index_exact_words", "min_infix_len", "morphology", "rt_mem_limit", "stopwords", "stored_fields", "wordforms"
		};
		if ( !sLine.empty() )
		{
#if !_WIN32
			if ( HasSchemaChange(sLine) )
				pCachedWords->clear();
#endif
			return *pCachedWords;
		}
		if ( !pCachedWords->empty() )
			return *pCachedWords;

		std::vector<std::string> dWords ( std::begin(dKeywords), std::end(dKeywords) );
#if !_WIN32
		std::vector<std::string> dTables;
		if ( FetchCompletionColumn ( "SHOW TABLES", dTables, tEndpoint ) )
		{
			dWords.insert ( dWords.end(), dTables.begin(), dTables.end() );
			constexpr size_t MAX_TABLES_WITH_COLUMN_COMPLETION = 100;
			for ( size_t i=0; i<Min ( dTables.size(), MAX_TABLES_WITH_COLUMN_COMPLETION ); ++i )
				FetchCompletionColumn ( "DESCRIBE " + QuoteCompletionIdentifier(dTables[i]), dWords, tEndpoint );
		}
#endif
		*pCachedWords = std::move(dWords);
		return *pCachedWords;
	};
}

#if !_WIN32
QueryResult_e ExecuteSqlBatch ( int & iSocket, const char * szQuery, bool bPrintStatements, bool bAligned, const SqlEndpoint_t & tEndpoint )
{
	auto dStatements = SplitSqlBatch ( szQuery );
	if ( dStatements.empty() )
		return QueryResult_e::OK;

	CSphString sQueryError;
	auto ConnectionError = [&] { fprintf ( stderr, "manticore: %s\n", sQueryError.cstr() ); return QueryResult_e::CONNECTION_ERROR; };
	for ( const LocalStatement_t & tStatement : dStatements )
	{
		const std::string & sStatement = tStatement.m_sSql;
		if ( tStatement.m_bUnterminatedBlockComment )
		{
			fputs ( "ERROR: unterminated block comment\n", stderr );
			return QueryResult_e::SQL_ERROR;
		}
		if ( IsSocketClosed ( iSocket ) )
		{
			close ( iSocket );
			iSocket = ConnectSocket ( sQueryError, tEndpoint );
			if ( iSocket<0 )
				return ConnectionError();
		}

		if ( bPrintStatements )
		{
			fprintf ( stdout, "--------------\n%s\n--------------\n\n", sStatement.c_str() );
			fflush ( stdout );
		}

		int64_t iStartedUS = sphMicroTimer();
		std::string sRequest = "POST /sql?mode=raw HTTP/1.1\r\nHost: localhost\r\nContent-Type: text/plain\r\nConnection: keep-alive\r\nContent-Length: ";
		sRequest += std::to_string ( sStatement.size() );
		sRequest += "\r\n\r\n";
		sRequest += sStatement;
		if ( !SendAll ( iSocket, sRequest.data(), sRequest.size(), sQueryError ) )
			return ConnectionError();

		int iStatus = 0;
		std::string sBody;
		if ( !ReadHttpResponse ( iSocket, iStatus, sBody, sQueryError ) )
			return ConnectionError();

		if ( iStatus<200 || iStatus>=300 )
		{
			ParseHttpError ( sBody, sQueryError );
			fprintf ( stderr, "ERROR %d: %s\n", iStatus, sQueryError.cstr() );
			return QueryResult_e::SQL_ERROR;
		}

		if ( !PrintSqlResponse ( sBody, sQueryError, bAligned, sphMicroTimer()-iStartedUS, ReportsAffectedRows(sStatement), tStatement.m_bVertical ) )
		{
			fprintf ( stderr, "ERROR: %s\n", sQueryError.cstr() );
			return QueryResult_e::SQL_ERROR;
		}
	}

	return QueryResult_e::OK;
}
#endif
}

std::vector<std::string> SplitLocalSqlStatements ( const char * szSql )
{
	return localmode::SplitSqlStatements ( szSql );
}
