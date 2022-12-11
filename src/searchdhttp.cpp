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

#include "searchdhttp.h"
#include "jsonqueryfilter.h"
#include "attribute.h"
#include "sphinxpq.h"

#include "http/http_parser.h"
#include "searchdaemon.h"
#include "searchdha.h"
#include "searchdreplication.h"
#include "accumulator.h"
#include "networking_daemon.h"
#include "client_session.h"
#include "tracer.h"
#include "searchdbuddy.h"

#define LOG_COMPONENT_HTTP ""
#define LOG_LEVEL_HTTP false

#define HTTPINFO LOGMSG ( INFO, HTTP, HTTP )


static const char * g_dHttpStatus[] = { "100 Continue", "200 OK", "206 Partial Content", "400 Bad Request", "403 Forbidden", "404 Not Found", "405 Method Not Allowed", "409 Conflict", "500 Internal Server Error",
								 "501 Not Implemented", "503 Service Unavailable", "526 Invalid SSL Certificate" };

STATIC_ASSERT ( sizeof(g_dHttpStatus)/sizeof(g_dHttpStatus[0])==SPH_HTTP_STATUS_TOTAL, SPH_HTTP_STATUS_SHOULD_BE_SAME_AS_SPH_HTTP_STATUS_TOTAL );

extern CSphString g_sStatusVersion;

int HttpGetStatusCodes ( ESphHttpStatus eStatus )
{
	switch ( eStatus )
	{
	case SPH_HTTP_STATUS_100: return 100;
	case SPH_HTTP_STATUS_200: return 200;
	case SPH_HTTP_STATUS_206: return 206;
	case SPH_HTTP_STATUS_400: return 400;
	case SPH_HTTP_STATUS_403: return 403;
	case SPH_HTTP_STATUS_404: return 404;
	case SPH_HTTP_STATUS_405: return 405;
	case SPH_HTTP_STATUS_409: return 409;
	case SPH_HTTP_STATUS_500: return 500;
	case SPH_HTTP_STATUS_501: return 501;
	case SPH_HTTP_STATUS_503: return 503;
	case SPH_HTTP_STATUS_526: return 526;
	default: return 503;
	};
};

extern CSphString g_sMySQLVersion;

static void HttpBuildReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * sBody, int iBodyLen, bool bHtml, bool bHeadReply )
{
	assert ( sBody && iBodyLen );

	const char * sContent = ( bHtml ? "text/html" : "application/json" );
	CSphString sHttp;
	sHttp.SetSprintf ( "HTTP/1.1 %s\r\nServer: %s\r\nContent-Type: %s; charset=UTF-8\r\nContent-Length:%d\r\n\r\n", g_dHttpStatus[eCode], g_sMySQLVersion.cstr(), sContent, iBodyLen );

	int iHeaderLen = sHttp.Length();
	int iBufLen = iHeaderLen;
	if ( !bHeadReply )
		iBufLen += iBodyLen;
	dData.Resize ( iBufLen );
	memcpy ( dData.Begin(), sHttp.cstr(), iHeaderLen );
	if ( !bHeadReply )
		memcpy ( dData.Begin() + iHeaderLen, sBody, iBodyLen );
}

void HttpBuildReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * sBody, int iBodyLen, bool bHtml )
{
	HttpBuildReply ( dData, eCode, sBody, iBodyLen, bHtml, false );
}

void HttpBuildReplyHead ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * sBody, int iBodyLen, bool bHeadReply )
{
	HttpBuildReply ( dData, eCode, sBody, iBodyLen, false, bHeadReply );
}


void HttpErrorReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * szError )
{
	JsonObj_c tErr;
	tErr.AddStr ( "error", szError );
	CSphString sJsonError = tErr.AsString();
	HttpBuildReply ( dData, eCode, sJsonError.cstr(), sJsonError.Length(), false );
}

struct Endpoint_t
{
	const char* m_szName1;
	const char* m_szName2;
};

static Endpoint_t g_dEndpoints[] =
	{
		{ "index.html", nullptr },
		{ "sql", nullptr },
		{ "search", "json/search" },
		{ "index", "json/index" },
		{ "create", "json/create" },
		{ "insert", "json/insert" },
		{ "replace", "json/replace" },
		{ "update", "json/update" },
		{ "delete", "json/delete" },
		{ "bulk", "json/bulk" },
		{ "pq", "json/pq" },
		{ "cli", nullptr },
		{ "_bulk", nullptr }
};

STATIC_ASSERT ( sizeof ( g_dEndpoints ) / sizeof ( g_dEndpoints[0] ) == SPH_HTTP_ENDPOINT_TOTAL, SPH_HTTP_ENDPOINT_SHOULD_BE_SAME_AS_SPH_HTTP_ENDPOINT_TOTAL );

ESphHttpEndpoint StrToHttpEndpoint ( const CSphString& sEndpoint )
{
	if ( sEndpoint.Begins ( g_dEndpoints[SPH_HTTP_ENDPOINT_PQ].m_szName1 ) || sEndpoint.Begins ( g_dEndpoints[SPH_HTTP_ENDPOINT_PQ].m_szName2 ) )
		return SPH_HTTP_ENDPOINT_PQ;

	for ( int i = 0; i < SPH_HTTP_ENDPOINT_TOTAL; ++i )
		if ( sEndpoint == g_dEndpoints[i].m_szName1 || ( g_dEndpoints[i].m_szName2 && sEndpoint == g_dEndpoints[i].m_szName2 ) )
			return ESphHttpEndpoint ( i );

	return SPH_HTTP_ENDPOINT_TOTAL;
}

///////////////////////////////////////////////////////////////////////
/// Stream reader

/// stub - returns feed string
class BlobStream_c final: public CharStream_c
{
	Str_t m_sData;

public:
	explicit BlobStream_c ( const CSphString& sData )
		: m_sData { FromStr ( sData ) }
	{}
	explicit BlobStream_c ( const Str_t & sData )
		: m_sData ( sData )
	{}

	Str_t Read() final
	{
		if ( m_bDone )
			return dEmptyStr;

		m_bDone = true;
		return m_sData;
	}

	Str_t ReadAll() final
	{
		auto sData = Read();

		Str_t sDescr = { sData.first, Min ( sData.second, 100 ) };
		myinfo::SetDescription ( sDescr, sDescr.second );
		return sData;
	}
};

CharStream_c * CreateBlobStream ( const Str_t & sData )
{
	return new BlobStream_c ( sData );
}

/// stream with known content length and no special massage over socket
class RawSocketStream_c final : public CharStream_c
{
	AsyncNetInputBuffer_c& m_tIn;
	int m_iContentLength;
	bool m_bTerminated = false;
	BYTE m_uOldTerminator = 0;

public:
	RawSocketStream_c ( AsyncNetInputBuffer_c& tIn, int iContentLength )
		: m_tIn { tIn }
		, m_iContentLength ( iContentLength )
	{
		m_bDone = !m_iContentLength;
	}

	~RawSocketStream_c() final
	{
		if ( m_bTerminated )
			m_tIn.Terminate ( 0, m_uOldTerminator );
		m_tIn.DiscardProcessed ( 0 );
	}

	Str_t Read() final
	{
		if ( m_bDone )
			return dEmptyStr;

		m_tIn.DiscardProcessed ( 0 );
		if ( !m_tIn.HasBytes() && m_tIn.ReadAny()<0 )
		{
			sphWarning ( "failed to receive HTTP request (error='%s')", sphSockError() );
			m_bDone = true;
			return dEmptyStr;
		}

		auto iChunk = Min ( m_iContentLength, m_tIn.HasBytes() );
		m_iContentLength -= iChunk;
		m_bDone = !m_iContentLength;

		// Temporary write \0 at the end, since parser wants z-terminated buf
		if ( m_bDone )
		{
			m_uOldTerminator = m_tIn.Terminate ( iChunk, '\0' );
			m_bTerminated = true;
		}

		return B2S ( m_tIn.PopTail ( iChunk ) );
	}

	Str_t ReadAll() final
	{
		if ( m_bDone )
			return dEmptyStr;

		// that is oneshot read - we sure, we're done
		m_bDone = true;

		if ( m_iContentLength && !m_tIn.ReadFrom ( m_iContentLength ) ) {
			sphWarning ( "failed to receive HTTP request (error='%s')", sphSockError() );
			return dEmptyStr;
		}

		m_uOldTerminator = m_tIn.Terminate ( m_iContentLength, '\0' );
		m_bTerminated = true;
		return B2S ( m_tIn.PopTail ( m_iContentLength ) );
	}
};

/// stream with known content length and no special massage over socket
class ChunkedSocketStream_c final: public CharStream_c
{
	AsyncNetInputBuffer_c& m_tIn;
	CSphVector<BYTE>	m_dData;	// used only in ReadAll() call
	int m_iLastParsed;
	bool m_bBodyDone;
	CSphVector<Str_t> m_dBodies;
	http_parser_settings m_tParserSettings;
	http_parser* m_pParser;
	const char* m_szError = nullptr;

private:
	// callbacks
	static int cbParserBody ( http_parser* pParser, const char* sAt, size_t iLen )
	{
		assert ( pParser->data );
		auto pThis = static_cast<ChunkedSocketStream_c*> ( pParser->data );
		return pThis->ParserBody ( { sAt, (int)iLen } );
	}

	static int cbMessageComplete ( http_parser* pParser )
	{
		assert ( pParser->data );
		auto pThis = static_cast<ChunkedSocketStream_c*> ( pParser->data );
		return pThis->MessageComplete();
	}

	inline int MessageComplete()
	{
		HTTPINFO << "ChunkedSocketStream_c::MessageComplete";

		m_bBodyDone = true;
		return 0;
	}

	inline int ParserBody ( Str_t sData )
	{
		HTTPINFO << "ChunkedSocketStream_c::ParserBody with " << sData.second << " bytes '" << sData << "'";

		if ( !IsEmpty ( sData ) )
			m_dBodies.Add ( sData );
		return 0;
	}

	void ParseBody ( ByteBlob_t sData )
	{
		HTTPINFO << "ChunkedSocketStream_c::ParseBody with " << sData.second << " bytes '" << sData << "'";
		m_iLastParsed = (int)http_parser_execute ( m_pParser, &m_tParserSettings, (const char*)sData.first, sData.second );
		if ( m_iLastParsed != sData.second )
		{
			HTTPINFO << "ParseBody error: parsed " << m_iLastParsed << ", chunk " << sData.second;
			m_szError = http_errno_description ( (http_errno)m_pParser->http_errno );
		}
	}

public:
	ChunkedSocketStream_c ( AsyncNetInputBuffer_c& tIn, http_parser* pParser, bool bBodyDone, CSphVector<Str_t> dBodies, int iLastParsed )
		: m_tIn { tIn }
		, m_iLastParsed ( iLastParsed )
		, m_bBodyDone ( bBodyDone )
		, m_pParser ( pParser )
	{
		m_dBodies = std::move ( dBodies );
		http_parser_settings_init ( &m_tParserSettings );
		m_tParserSettings.on_body = cbParserBody;
		m_tParserSettings.on_message_complete = cbMessageComplete;
		m_pParser->data = this;
	}

	void DiscardLast()
	{
		m_tIn.PopTail ( std::exchange ( m_iLastParsed, 0 ) );
		m_tIn.DiscardProcessed ( 0 );
	}

	~ChunkedSocketStream_c() final
	{
		DiscardLast();
	}

	Str_t Read() final
	{
		if ( m_bDone )
			return dEmptyStr;

		while ( m_dBodies.IsEmpty() )
		{
			if ( m_bBodyDone )
			{
				m_bDone = true;
				return dEmptyStr;
			}

			DiscardLast();
			if ( !m_tIn.HasBytes() )
			{
				auto iStart = sphMicroTimer();
				switch ( m_tIn.ReadAny() )
				{
				case -1:
					if ( m_tIn.GetError() )
						sphLogDebug ( "failed to receive HTTP request -1 (error='%s') after %d us", sphSockError(), (int)( sphMicroTimer() - iStart ) );
				case 0:
					m_bDone = true;
					return dEmptyStr;
				default:
					break;
				}
			}
			ParseBody ( m_tIn.Tail() );
		}

		auto sResult = m_dBodies.First();
		m_dBodies.Remove ( 0 );

		if ( m_bBodyDone && m_dBodies.IsEmpty() )
		{
			m_bDone = true;
			const_cast<char&> ( sResult.first[sResult.second] ) = '\0';
		}
		return sResult;
	}

	Str_t ReadAll() final
	{
		auto sFirst = Read();

		if ( m_bDone )
			return sFirst;

		sphWarning ( "Reading whole body with chunked transfer-encoding is non-effective" );

		m_dData.Append ( sFirst );
		do
			m_dData.Append ( Read() );
		while ( !m_bDone );

		m_dData.Add ( '\0' );
		m_dData.Resize ( m_dData.GetLength() - 1 );
		return m_dData;
	}
};

///////////////////////////////////////////////////////////////////////

CSphString HttpEndpointToStr ( ESphHttpEndpoint eEndpoint )
{
	assert ( eEndpoint >= SPH_HTTP_ENDPOINT_INDEX && eEndpoint < SPH_HTTP_ENDPOINT_TOTAL );
	return g_dEndpoints[eEndpoint].m_szName1;
}

void HttpBuildReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, Str_t sReply, bool bHtml )
{
	const char * sContent = ( bHtml ? "text/html" : "application/json" );
	StringBuilder_c sHttp;
	sHttp.Sprintf ( "HTTP/1.1 %s\r\nServer: %s\r\nContent-Type: %s; charset=UTF-8\r\nContent-Length: %d\r\n\r\n", g_dHttpStatus[eCode], g_sStatusVersion.cstr(), sContent, sReply.second );

	dData.Reserve ( sHttp.GetLength() + sReply.second );
	dData.Append ( (Str_t)sHttp );
	dData.Append ( sReply );
}


HttpRequestParser_c::HttpRequestParser_c()
{
	http_parser_settings_init ( &m_tParserSettings );

	m_tParserSettings.on_url = cbParserUrl;
	m_tParserSettings.on_header_field = cbParserHeaderField;
	m_tParserSettings.on_header_value = cbParserHeaderValue;
	m_tParserSettings.on_headers_complete = cbParseHeaderCompleted;
	m_tParserSettings.on_body = cbParserBody;

	m_tParserSettings.on_message_begin = cbMessageBegin;
	m_tParserSettings.on_message_complete = cbMessageComplete;
	m_tParserSettings.on_status = cbMessageStatus;

	Reinit();
}

void HttpRequestParser_c::Reinit()
{
	HTTPINFO << "HttpRequestParser_c::Reinit()";

	http_parser_init ( &m_tParser, HTTP_REQUEST );
	m_sEndpoint = "";
	m_sCurField.Clear();
	m_sCurValue.Clear();
	m_hOptions.Reset();
	m_eType = HTTP_GET;
	m_sUrl.Clear();
	m_bHeaderDone = false;
	m_bBodyDone = false;
	m_dParsedBodies.Reset();
	m_iParsedBodyLength = 0;
	m_iLastParsed = 0;
	m_szError = nullptr;
	m_tParser.data = this;
}

bool HttpRequestParser_c::ParseHeader ( ByteBlob_t sData )
{
	HTTPINFO << "ParseChunk with " << sData.second << " bytes '" << sData << "'";

	m_iLastParsed = (int) http_parser_execute ( &m_tParser, &m_tParserSettings, (const char *)sData.first, sData.second );
	if ( m_iLastParsed != sData.second )
	{
		sphLogDebug ( "ParseChunk error: parsed %d, chunk %d", m_iLastParsed, sData.second );
		m_szError = http_errno_description ( (http_errno)m_tParser.http_errno );
		return true;
	}

	return m_bHeaderDone;
}


int HttpRequestParser_c::ParsedBodyLength() const
{
	return m_iParsedBodyLength;
}

bool HttpRequestParser_c::Expect100() const
{
	return m_hOptions.Exists ( "Expect" ) && m_hOptions["Expect"] == "100-continue";
}

bool HttpRequestParser_c::KeepAlive() const
{
	return m_bKeepAlive;
}

const char* HttpRequestParser_c::Error() const
{
	return m_szError;
}

inline int Char2Hex ( BYTE uChar )
{
	switch (uChar)
	{
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'a': case 'A': return 10;
	case 'b': case 'B': return 11;
	case 'c': case 'C': return 12;
	case 'd': case 'D': return 13;
	case 'e': case 'E': return 14;
	case 'f': case 'F': return 15;
	default: break;
	}
	return -1;
}

inline int Chars2Hex ( const char* pSrc )
{
	int iRes = Char2Hex ( *( pSrc + 1 ) );
	return iRes < 0 ? iRes : iRes + Char2Hex ( *pSrc ) * 16;
}

void UriPercentReplace ( Str_t & sEntity, bool bAlsoPlus )
{
	if ( IsEmpty ( sEntity ) )
		return;

	const char* pSrc = sEntity.first;
	auto* pDst = const_cast<char*> ( pSrc );
	char cPlus = bAlsoPlus ? ' ' : '+';
	auto* pEnd = pSrc + sEntity.second;
	while ( pSrc < pEnd )
	{
		if ( *pSrc=='%' && *(pSrc+1) && *(pSrc+2) )
		{
			auto iCode = Chars2Hex ( pSrc + 1 );
			if ( iCode<0 )
			{
				*pDst++ = *pSrc++;
				continue;
			}
			pSrc += 3;
			*pDst++ = (char) iCode;
		} else
		{
			*pDst++ = ( *pSrc=='+' ? cPlus : *pSrc );
			pSrc++;
		}
	}
	sEntity.second = int ( pDst - sEntity.first );
}

void StoreRawQuery ( OptionsHash_t& hOptions, const Str_t& sWholeData )
{
	if ( IsEmpty ( sWholeData ) )
		return;

	// store raw query
	CSphString sRawBody ( sWholeData );				   // copy raw data, important!
	Str_t sRaw { sRawBody.cstr(), sWholeData.second }; // FromStr implies strlen(), but we don't need it
	UriPercentReplace ( sRaw, false );				   // avoid +-decoding
	*const_cast<char*> ( sRaw.first + sRaw.second ) = '\0';
	hOptions.Add ( std::move ( sRawBody ), "raw_query" );
}

void HttpRequestParser_c::ParseList ( Str_t sData, OptionsHash_t & hOptions )
{
	HTTPINFO << "ParseList with " << sData.second << " bytes '" << sData << "'";

	const char * sCur = sData.first;
	const char* sLast = sCur;
	const char * sEnd = sCur + sData.second;

	Str_t sName = dEmptyStr;
	for ( ; sCur<sEnd; ++sCur )
	{
		switch (*sCur)
		{
		case '=':
			{
				sName = { sLast, int ( sCur - sLast ) };
				UriPercentReplace ( sName );
				sLast = sCur + 1;
				break;
			}
		case '&':
			{
				Str_t sVal { sLast, int ( sCur - sLast ) };
				UriPercentReplace ( sVal );
				hOptions.Add ( sVal, sName );
				sLast = sCur + 1;
				sName = dEmptyStr;
				break;
			}
		default:
			break;
		}
	}

	if ( IsEmpty ( sName ) )
		return;

	Str_t sVal { sLast, int ( sCur - sLast ) };
	UriPercentReplace ( sVal );
	hOptions.Add ( sVal, sName );
}

inline int HttpRequestParser_c::ParserUrl ( Str_t sData )
{
	HTTPINFO << "ParseUrl with " << sData.second << " bytes '" << sData << "'";

	m_sUrl << sData;
	return 0;
}

inline void HttpRequestParser_c::FinishParserUrl ()
{
	HTTPINFO <<"FinishParserUrl '" << m_sUrl << "'";
	if ( m_sUrl.IsEmpty() )
		return;

	auto _ = AtScopeExit ( [this] { m_sUrl.Clear(); } );
	auto sData = (Str_t) m_sUrl;
	http_parser_url tUri;
	if ( http_parser_parse_url ( sData.first, sData.second, 0, &tUri ) )
		return;

	DWORD uPath = ( 1UL<<UF_PATH );
	DWORD uQuery = ( 1UL<<UF_QUERY );

	if ( ( tUri.field_set & uPath )!=0 )
	{
		const char * sPath = sData.first + tUri.field_data[UF_PATH].off;
		int iPathLen = tUri.field_data[UF_PATH].len;
		if ( *sPath=='/' )
		{
			++sPath;
			--iPathLen;
		}

		// URL should be split fully to point to proper endpoint 
		m_sEndpoint.SetBinary ( sPath, iPathLen );
		// transfer endpoint for further parse
		m_hOptions.Add ( m_sEndpoint, "endpoint" );
	}

	if ( ( tUri.field_set & uQuery )!=0 )
	{
		Str_t sRawGetQuery { sData.first + tUri.field_data[UF_QUERY].off, tUri.field_data[UF_QUERY].len };
		if ( m_eType == HTTP_GET )
			StoreRawQuery ( m_hOptions, sRawGetQuery );
		ParseList ( sRawGetQuery, m_hOptions );
	}

	CSphString sFullURL;
	if ( ( tUri.field_set & uPath )!=0 && ( tUri.field_set & uQuery )!=0 )
	{
		const char * sStart = sData.first + tUri.field_data[UF_PATH].off;
		const char * sEnd = sData.first + tUri.field_data[UF_QUERY].off + tUri.field_data[UF_QUERY].len;
		sFullURL.SetBinary ( sStart, sEnd-sStart );
		m_hOptions.Add ( sFullURL, "full_url" );
	} else if ( ( tUri.field_set & uPath )!=0 )
	{
		const char * sPath = sData.first + tUri.field_data[UF_PATH].off;
		int iPathLen = tUri.field_data[UF_PATH].len;

		// URL should be split fully to point to proper endpoint 
		sFullURL.SetBinary ( sPath, iPathLen );
		m_hOptions.Add ( sFullURL, "full_url" );
	}
}

inline int HttpRequestParser_c::ParserHeaderField ( Str_t sData )
{
	HTTPINFO << "ParserHeaderField with '" << sData << "'";

	FinishParserKeyVal();
	m_sCurField << sData;
	return 0;
}

inline int HttpRequestParser_c::ParserHeaderValue ( Str_t sData )
{
	HTTPINFO << "ParserHeaderValue with '" << sData << "'";

	m_sCurValue << sData;
	return 0;
}

inline void HttpRequestParser_c::FinishParserKeyVal()
{
	if ( m_sCurValue.IsEmpty() )
		return;

	HTTPINFO << "FinishParserKeyVal with '" << m_sCurField << "':'" << m_sCurValue << "'";

	m_hOptions.Add ( (CSphString)m_sCurValue, (CSphString)m_sCurField );
	m_sCurField.Clear();
	m_sCurValue.Clear();
}

inline int HttpRequestParser_c::ParserBody ( Str_t sData )
{
	HTTPINFO << "ParserBody with " << sData.second << " bytes '" << sData << "'";

	if ( !m_dParsedBodies.IsEmpty() )
	{
		auto& sLast = m_dParsedBodies.Last();
		if ( sLast.first + sLast.second == sData.first )
			sLast.second += sData.second;
		else
			m_dParsedBodies.Add ( sData );
	} else
		m_dParsedBodies.Add ( sData );
	m_iParsedBodyLength += sData.second;
	return 0;
}

inline int HttpRequestParser_c::MessageComplete ()
{
	HTTPINFO << "MessageComplete";

	m_bBodyDone = true;
	return 0;
}

inline int HttpRequestParser_c::ParseHeaderCompleted ()
{
	HTTPINFO << "ParseHeaderCompleted. Upgrade=" << (unsigned int)m_tParser.upgrade << ", length=" << (int64_t) m_tParser.content_length;

	// we're not support connection upgrade - so just reset upgrade flag, if detected.
	// rfc7540 section-3.2 (for http/2) says, we just should continue as if no 'upgrade' header was found
	m_tParser.upgrade = 0;

	// connection wide http options
	m_bKeepAlive = ( http_should_keep_alive ( &m_tParser ) != 0 );
	m_eType = (http_method)m_tParser.method;

	FinishParserKeyVal();
	FinishParserUrl();
	m_bHeaderDone = true;
	return 0;
}

int HttpRequestParser_c::cbParserUrl ( http_parser* pParser, const char* sAt, size_t iLen )
{
	assert ( pParser->data );
	auto pThis = static_cast<HttpRequestParser_c*> ( pParser->data );
	return pThis->ParserUrl ( { sAt, (int)iLen } );
}

int HttpRequestParser_c::cbParserHeaderField ( http_parser* pParser, const char* sAt, size_t iLen )
{
	assert ( pParser->data );
	auto pThis = static_cast<HttpRequestParser_c*> ( pParser->data );
	return pThis->ParserHeaderField ( { sAt, (int)iLen } );
}

int HttpRequestParser_c::cbParserHeaderValue ( http_parser* pParser, const char* sAt, size_t iLen )
{
	assert ( pParser->data );
	auto pThis = static_cast<HttpRequestParser_c*> ( pParser->data );
	return pThis->ParserHeaderValue ( { sAt, (int)iLen } );
}

int HttpRequestParser_c::cbParseHeaderCompleted ( http_parser* pParser )
{
	assert ( pParser->data );
	auto pThis = static_cast<HttpRequestParser_c*> ( pParser->data );
	return pThis->ParseHeaderCompleted ();
}

int HttpRequestParser_c::cbMessageBegin ( http_parser* pParser )
{
	HTTPINFO << "cbMessageBegin";
	return 0;
}

int HttpRequestParser_c::cbMessageComplete ( http_parser* pParser )
{
	assert ( pParser->data );
	auto pThis = static_cast<HttpRequestParser_c*> ( pParser->data );
	return pThis->MessageComplete();
}

int HttpRequestParser_c::cbMessageStatus ( http_parser* pParser, const char* sAt, size_t iLen )
{
	HTTPINFO << "cbMessageStatus with '" << Str_t { sAt, (int)iLen } << "'";
	return 0;
}

int HttpRequestParser_c::cbParserBody ( http_parser* pParser, const char* sAt, size_t iLen )
{
	assert ( pParser->data );
	auto pThis = static_cast<HttpRequestParser_c*> ( pParser->data );
	return pThis->ParserBody ( { sAt, (int)iLen } );
}

static const char * g_sIndexPage =
R"index(<!DOCTYPE html>
<html>
<head>
<title>Manticore</title>
</head>
<body>
<h1>Manticore daemon</h1>
<p>%s</p>
</body>
</html>)index";


static void HttpHandlerIndexPage ( CSphVector<BYTE> & dData )
{
	StringBuilder_c sIndexPage;
	sIndexPage.Appendf ( g_sIndexPage, g_sStatusVersion.cstr() );
	HttpBuildReply ( dData, SPH_HTTP_STATUS_200, (Str_t)sIndexPage, true );
}

//////////////////////////////////////////////////////////////////////////
class JsonRequestBuilder_c : public RequestBuilder_i
{
public:
	JsonRequestBuilder_c ( const char* szQuery, CSphString sEndpoint )
			: m_sEndpoint ( std::move ( sEndpoint ) )
			, m_tQuery ( szQuery )
	{
		// fixme: we can implement replacing indexes in a string (without parsing) if it becomes a performance issue
	}

	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const final
	{
		// replace "index" value in the json query
		m_tQuery.DelItem ( "index" );
		m_tQuery.AddStr ( "index", tAgent.m_tDesc.m_sIndexes.cstr() );

		CSphString sRequest = m_tQuery.AsString();

		auto tWr = APIHeader ( tOut, SEARCHD_COMMAND_JSON, VER_COMMAND_JSON ); // API header
		tOut.SendString ( m_sEndpoint.cstr() );
		tOut.SendString ( sRequest.cstr() );
	}

private:
	CSphString			m_sEndpoint;
	mutable JsonObj_c	m_tQuery;
};


class JsonReplyParser_c : public ReplyParser_i
{
public:
	JsonReplyParser_c ( int & iAffected, int & iWarnings )
		: m_iAffected ( iAffected )
		, m_iWarnings ( iWarnings )
	{}

	bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const final
	{
		CSphString sEndpoint = tReq.GetString();
		ESphHttpEndpoint eEndpoint = StrToHttpEndpoint ( sEndpoint );
		if ( eEndpoint!=SPH_HTTP_ENDPOINT_JSON_UPDATE && eEndpoint!=SPH_HTTP_ENDPOINT_JSON_DELETE )
			return false;

		DWORD uLength = tReq.GetDword();
		CSphFixedVector<BYTE> dResult ( uLength+1 );
		tReq.GetBytes ( dResult.Begin(), (int)uLength );
		dResult[uLength] = '\0';

		return sphGetResultStats ( (const char *)dResult.Begin(), m_iAffected, m_iWarnings, eEndpoint==SPH_HTTP_ENDPOINT_JSON_UPDATE );
	}

protected:
	int &	m_iAffected;
	int &	m_iWarnings;
};

std::unique_ptr<QueryParser_i> CreateQueryParser( bool bJson )
{
	if ( bJson )
		return sphCreateJsonQueryParser();
	else
		return sphCreatePlainQueryParser();
}

std::unique_ptr<RequestBuilder_i> CreateRequestBuilder ( Str_t sQuery, const SqlStmt_t & tStmt )
{
	if ( tStmt.m_bJson )
	{
		assert ( !tStmt.m_sEndpoint.IsEmpty() );
		return std::make_unique<JsonRequestBuilder_c> ( sQuery.first, tStmt.m_sEndpoint );
	} else
	{
		return std::make_unique<SphinxqlRequestBuilder_c> ( sQuery, tStmt );
	}
}

std::unique_ptr<ReplyParser_i> CreateReplyParser ( bool bJson, int & iUpdated, int & iWarnings )
{
	if ( bJson )
		return std::make_unique<JsonReplyParser_c> ( iUpdated, iWarnings );
	else
		return std::make_unique<SphinxqlReplyParser_c> ( &iUpdated, &iWarnings );
}

//////////////////////////////////////////////////////////////////////////
class HttpErrorReporter_c final : public StmtErrorReporter_i
{
public:
	void			Ok ( int iAffectedRows, const CSphString & /*sWarning*/, int64_t /*iLastInsertId*/ ) final { m_iAffected = iAffectedRows; }
	void			Ok ( int iAffectedRows, int /*nWarnings*/ ) final { m_iAffected = iAffectedRows; }
	void			ErrorEx ( MysqlErrors_e iErr, const char * sError ) final;
	RowBuffer_i * 	GetBuffer() final { return nullptr; }

	bool			IsError() const { return m_bError; }
	const char *	GetError() const { return m_sError.cstr(); }
	int				GetAffectedRows() const { return m_iAffected; }

private:
	bool			m_bError {false};
	CSphString		m_sError;
	int				m_iAffected {0};
};


void HttpErrorReporter_c::ErrorEx ( MysqlErrors_e /*iErr*/, const char * sError )
{
	m_bError = true;
	m_sError = sError;
}

StmtErrorReporter_i * CreateHttpErrorReporter()
{
	return new HttpErrorReporter_c();
}

//////////////////////////////////////////////////////////////////////////
// all the handlers for http queries

void ReplyBuf ( Str_t sResult, ESphHttpStatus eStatus, bool bNeedHttpResponse, CSphVector<BYTE> & dData )
{
	if ( bNeedHttpResponse )
		HttpBuildReply ( dData, eStatus, sResult, false );
	else
	{
		dData.Resize ( 0 );
		dData.Append ( sResult );
	}
}


class HttpHandler_c
{
public:
	virtual ~HttpHandler_c() = default;

	virtual bool Process () = 0;

	void SetErrorFormat ( bool bNeedHttpResponse )
	{
		m_bNeedHttpResponse = bNeedHttpResponse;
	}
	
	CSphVector<BYTE> & GetResult()
	{
		return m_dData;
	}

	const CSphString & GetError () const
	{
		return m_sError;
	}

protected:
	bool				m_bNeedHttpResponse {false};
	CSphVector<BYTE>	m_dData;
	CSphString			m_sError;

	void ReportError ( const char * szError, ESphHttpStatus eStatus )
	{
		m_sError = szError;
		if ( m_bNeedHttpResponse )
			sphHttpErrorReply ( m_dData, eStatus, szError );
		else
		{
			m_dData.Resize ( (int) strlen ( szError ) );
			memcpy ( m_dData.Begin(), szError, m_dData.GetLength() );
		}
	}

	void FormatError ( ESphHttpStatus eStatus, const char * sError, ... )
	{
		va_list ap;
		va_start ( ap, sError );
		m_sError.SetSprintfVa ( sError, ap );
		va_end ( ap );

		if ( m_bNeedHttpResponse )
			sphHttpErrorReply ( m_dData, eStatus, m_sError.cstr() );
		else
		{
			int iLen = m_sError.Length();
			m_dData.Resize ( iLen+1 );
			memcpy ( m_dData.Begin(), m_sError.cstr(), iLen );
			m_dData[iLen] = '\0';
		}
	}

	void BuildReply ( const CSphString & sResult, ESphHttpStatus eStatus )
	{
		ReplyBuf ( FromStr ( sResult ), eStatus, m_bNeedHttpResponse, m_dData );
	}

	void BuildReply ( const char* szResult, ESphHttpStatus eStatus )
	{
		ReplyBuf ( FromSz( szResult ), eStatus, m_bNeedHttpResponse, m_dData );
	}

	void BuildReply ( const StringBuilder_c & sResult, ESphHttpStatus eStatus )
	{
		ReplyBuf ( (Str_t)sResult, eStatus, m_bNeedHttpResponse, m_dData );
	}

	// check whether given served index is exist and has requested type
	bool CheckValid ( const ServedIndex_c* pServed, const CSphString& sIndex, IndexType_e eType )
	{
		if ( !pServed )
		{
			FormatError ( SPH_HTTP_STATUS_500, "no such index '%s'", sIndex.cstr () );
			return false;
		}
		if ( pServed->m_eType!=eType )
		{
			FormatError ( SPH_HTTP_STATUS_500, "index '%s' is not %s", sIndex.cstr(), GetTypeName ( eType ).cstr() );
			return false;
		}
		return true;
	}
};

std::unique_ptr<PubSearchHandler_c> CreateMsearchHandler ( std::unique_ptr<QueryParser_i> pQueryParser, QueryType_e eQueryType, JsonQuery_c & tQuery )
{
	tQuery.m_pQueryParser = pQueryParser.get();

	int iQueries = ( 1 + tQuery.m_dAggs.GetLength() );
	std::unique_ptr<PubSearchHandler_c> pHandler = std::make_unique<PubSearchHandler_c> ( iQueries, std::move ( pQueryParser ), eQueryType, true );

	if ( !tQuery.m_dAggs.GetLength() || eQueryType==QUERY_SQL )
	{
		pHandler->SetQuery ( 0, tQuery, nullptr );
		return pHandler;
	}

	tQuery.m_dRefItems = tQuery.m_dItems;
	CSphQueryItem & tCountItem = tQuery.m_dItems.Add();
	tCountItem.m_sExpr = "count(*)";
	tCountItem.m_sAlias = "count(*)";

	sph::StringSet hAttrs;
	for ( const auto & tItem : tQuery.m_dItems )
		hAttrs.Add ( tItem.m_sAlias );

	for ( const JsonAggr_t & tBucket : tQuery.m_dAggs )
	{
		// add only new items
		if ( hAttrs[tBucket.m_sCol] )
			continue;

		CSphQueryItem & tItem = tQuery.m_dItems.Add();
		tItem.m_sExpr = tBucket.m_sCol;
		tItem.m_sAlias = tBucket.m_sCol;

		ARRAY_FOREACH ( iNested, tBucket.m_dNested )
		{
			if ( tBucket.m_dNested[iNested].m_eAggrFunc==SPH_AGGR_NONE )
				continue;

			CSphQueryItem & tItem = tQuery.m_dItems.Add();
			tItem.m_sExpr = tBucket.m_dNested[iNested].m_sCol;
			tItem.m_sAlias = tBucket.m_dNested[iNested].GetAliasName();
			tItem.m_eAggrFunc = tBucket.m_dNested[iNested].m_eAggrFunc;
		}
	}

	tQuery.m_bFacetHead = true;
	pHandler->SetQuery ( 0, tQuery, nullptr );
	int iRefLimit = tQuery.m_iLimit;
	int iRefOffset = tQuery.m_iOffset;

	ARRAY_FOREACH ( i, tQuery.m_dAggs )
	{
		const JsonAggr_t & tBucket = tQuery.m_dAggs[i];

		// common to main query but flags, select list and ref items should uniq

		// facet flags
		tQuery.m_bFacetHead = false;
		tQuery.m_bFacet = true;

		// select list to facet query
		tQuery.m_sSelect.SetSprintf ( "%s", tBucket.m_sCol.cstr() );

		// ref items to facet query
		tQuery.m_dRefItems.Resize ( 0 );
		CSphQueryItem & tItem = tQuery.m_dRefItems.Add();
		tItem.m_sExpr = tBucket.m_sCol;
		tItem.m_sAlias = tBucket.m_sCol;
		CSphQueryItem & tAggCountItem = tQuery.m_dRefItems.Add();
		tAggCountItem.m_sExpr = "count(*)";
		tAggCountItem.m_sAlias = "count(*)";
		ARRAY_FOREACH ( iNested, tBucket.m_dNested )
		{
			if ( tBucket.m_dNested[iNested].m_eAggrFunc==SPH_AGGR_NONE )
				continue;

			CSphQueryItem & tItem = tQuery.m_dRefItems.Add();
			tItem.m_sExpr = tBucket.m_dNested[iNested].m_sCol;
			tItem.m_sAlias = tBucket.m_dNested[iNested].GetAliasName();
			tItem.m_eAggrFunc = tBucket.m_dNested[iNested].m_eAggrFunc;
		}

		tQuery.m_sGroupBy = tBucket.m_sCol;
		tQuery.m_sFacetBy = tBucket.m_sCol;
		tQuery.m_sGroupSortBy = "@groupby desc";
		tQuery.m_sOrderBy = "@weight desc";

		// aggregate and main query could have different sizes
		if ( tBucket.m_iSize )
		{
			tQuery.m_iLimit = tBucket.m_iSize;
			tQuery.m_iOffset = 0;
		} else
		{
			tQuery.m_iLimit = iRefLimit;
			tQuery.m_iOffset = iRefOffset;
		}

		pHandler->SetQuery ( i+1, tQuery, nullptr );
	}

	return pHandler;
}


class HttpSearchHandler_c : public HttpHandler_c
{
public:
	bool Process () final
	{
		TRACE_CONN ( "conn", "HttpSearchHandler_c::Process" );
		CSphString sWarning;
		std::unique_ptr<QueryParser_i> pQueryParser = PreParseQuery();
		if ( !pQueryParser )
			return false;

		int iQueries = ( 1 + m_tQuery.m_dAggs.GetLength() );

		std::unique_ptr<PubSearchHandler_c> tHandler = CreateMsearchHandler ( std::move ( pQueryParser ), m_eQueryType, m_tQuery );
		SetStmt ( *tHandler );

		QueryProfile_c tProfile;
		if ( m_bProfile )
			tHandler->SetProfile ( &tProfile );

		// search
		tHandler->RunQueries();

		if ( m_bProfile )
			tProfile.Stop();

		AggrResult_t * pRes = tHandler->GetResult ( 0 );
		if ( !pRes->m_sError.IsEmpty() )
		{
			ReportError ( pRes->m_sError.cstr(), SPH_HTTP_STATUS_500 );
			return false;
		}

		// fixme: handle more than one warning at once?
		if ( pRes->m_sWarning.IsEmpty() && !m_sWarning.IsEmpty() )
			pRes->m_sWarning = m_sWarning;

		CSphFixedVector<AggrResult_t *> dAggsRes ( iQueries );
		dAggsRes[0] = tHandler->GetResult ( 0 );
		ARRAY_FOREACH ( i, m_tQuery.m_dAggs )
			dAggsRes[i+1] = tHandler->GetResult ( i+1 );

		CSphString sResult = EncodeResult ( dAggsRes, m_bProfile ? &tProfile : nullptr );
		BuildReply ( sResult, SPH_HTTP_STATUS_200 );

		return true;
	}

protected:
	bool					m_bProfile = false;
	QueryType_e				m_eQueryType {QUERY_SQL};
	JsonQuery_c				m_tQuery;
	CSphString				m_sWarning;

	virtual std::unique_ptr<QueryParser_i> PreParseQuery() = 0;
	virtual CSphString		EncodeResult ( const VecTraits_T<AggrResult_t *> & dRes, QueryProfile_c * pProfile ) = 0;
	virtual void			SetStmt ( PubSearchHandler_c & tHandler ) {};
};

static void AddAggs ( const VecTraits_T<SqlStmt_t> & dStmt, JsonQuery_c & tQuery )
{
	assert ( dStmt.GetLength()>1 && dStmt[0].m_tQuery.m_bFacetHead );
	tQuery.m_dAggs.Reserve ( dStmt.GetLength()-1 );

	for ( int i=1; i<dStmt.GetLength(); i++ )
	{
		const CSphQuery & tRef = dStmt[i].m_tQuery;
		assert ( tRef.m_dRefItems.GetLength() );

		JsonAggr_t & tBucket = tQuery.m_dAggs.Add();
		tBucket.m_sBucketName = tRef.m_dRefItems[0].m_sExpr;
		tBucket.m_sCol = tRef.m_dRefItems[0].m_sAlias;
	}
}

class HttpSearchHandler_SQL_c final: public HttpSearchHandler_c
{
	const OptionsHash_t& m_tOptions;

public:
	explicit HttpSearchHandler_SQL_c ( const OptionsHash_t & tOptions )
		: m_tOptions ( tOptions )
	{}

protected:
	CSphVector<SqlStmt_t> m_dStmt;
	std::unique_ptr<QueryParser_i> PreParseQuery() final
	{
		const CSphString * pRawQl = m_tOptions ( "query" );
		if ( !pRawQl || pRawQl->IsEmpty() )
		{
			ReportError ( "query missing", SPH_HTTP_STATUS_400 );
			return nullptr;
		}

		CSphString sError;
		if ( !sphParseSqlQuery ( pRawQl->cstr(), pRawQl->Length(), m_dStmt, sError, SPH_COLLATION_DEFAULT ) )
		{
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return nullptr;
		}

		( (CSphQuery &) m_tQuery ) = m_dStmt[0].m_tQuery;
		bool bFacet = ( m_dStmt.GetLength()>1 );
		for ( const auto & tStmt : m_dStmt )
		{
			// should be all FACET in case of multiple queries
			bFacet &= ( tStmt.m_tQuery.m_bFacet || tStmt.m_tQuery.m_bFacetHead );

			if ( tStmt.m_eStmt!=STMT_SELECT )
			{
				ReportError ( "only SELECT queries are supported", SPH_HTTP_STATUS_501 );
				return nullptr;
			}
		}

		if ( m_dStmt.GetLength()>1 && !bFacet )
		{
			ReportError ( "only FACET multiple queries supported", SPH_HTTP_STATUS_501 );
			return nullptr;
		}
		if ( bFacet )
			AddAggs ( m_dStmt, m_tQuery );

		m_eQueryType = QUERY_SQL;

		return sphCreatePlainQueryParser();
	}

	CSphString EncodeResult ( const VecTraits_T<AggrResult_t *> & dRes, QueryProfile_c * pProfile ) final
	{
		return sphEncodeResultJson ( dRes, m_tQuery, pProfile, false );
	}

	void SetStmt ( PubSearchHandler_c & tHandler ) final
	{
		tHandler.SetStmt ( m_dStmt[0] );
		for  ( int i=1; i<m_dStmt.GetLength(); i++ )
			tHandler.SetQuery ( i, m_dStmt[i].m_tQuery, nullptr );
	}
};

typedef std::pair<CSphString,MysqlColumnType_e> ColumnNameType_t;

static const char * GetTypeName ( MysqlColumnType_e eType )
{
	switch ( eType )
	{
		case MYSQL_COL_DECIMAL:		return "decimal";
		case MYSQL_COL_LONG:		return "long";
		case MYSQL_COL_FLOAT:		return "float";
		case MYSQL_COL_DOUBLE:		return "double";
		case MYSQL_COL_LONGLONG:	return "long long";
		case MYSQL_COL_STRING:		return "string";
		default: return "unknown";
	};
}

static MysqlColumnType_e GetTypeName ( const CSphString& sType )
{
	if ( sType=="decimal")
		return MYSQL_COL_DECIMAL;

	if ( sType == "long" )
		return MYSQL_COL_LONG;

	if ( sType == "float" )
		return MYSQL_COL_FLOAT;

	if ( sType == "double" )
		return MYSQL_COL_DOUBLE;

	if ( sType == "long long" )
		return MYSQL_COL_LONGLONG;

	if ( sType == "string" )
		return MYSQL_COL_STRING;

	assert (false && "Unknown column");
	return MYSQL_COL_STRING;
}

JsonEscapedBuilder& operator<< ( JsonEscapedBuilder& tOut, MysqlColumnType_e eType )
{
	tOut.FixupSpacedAndAppendEscaped ( GetTypeName ( eType ) );
	return tOut;
}

const StrBlock_t dJsonObjCustom { { ",\n", 2 }, { "[", 1 }, { "]", 1 } }; // json object with custom formatting

class JsonRowBuffer_c : public RowBuffer_i
{
public:
	JsonRowBuffer_c()
	{
		m_dBuf.StartBlock ( dJsonObjCustom );
	}

	void PutFloatAsString ( float fVal, const char * ) override
	{
		AddDataColumn();
		m_dBuf << fVal;
	}

	void PutDoubleAsString ( double fVal, const char * ) override
	{
		AddDataColumn();
		m_dBuf << fVal;
	}

	void PutNumAsString ( int64_t iVal ) override
	{
		AddDataColumn();
		m_dBuf << iVal;
	}

	void PutNumAsString ( uint64_t uVal ) override
	{
		AddDataColumn();
		m_dBuf << uVal;
	}

	void PutNumAsString ( int iVal ) override
	{
		AddDataColumn();
		m_dBuf << iVal;
	}
	
	void PutNumAsString ( DWORD uVal ) override
	{
		AddDataColumn();
		m_dBuf << uVal;
	}

	void PutArray ( const ByteBlob_t& dBlob, bool ) override
	{
		AddDataColumn();
		m_dBuf.FixupSpacedAndAppendEscaped ( (const char*)dBlob.first, dBlob.second );
	}

	void PutString ( Str_t sMsg ) override
	{
		PutArray ( S2B ( sMsg ), false );
	}

	void PutMicrosec ( int64_t iUsec ) override
	{
		PutNumAsString ( iUsec );
	}

	void PutNULL() override
	{
		AddDataColumn();
		m_dBuf << "null";
	}

	bool Commit() override
	{
		m_dBuf.FinishBlock ( false ); // finish previous item
		m_dBuf.ObjectBlock(); // start new item
		++m_iTotalRows;
		m_iCol = 0;
		return true;
	}

	void Eof ( bool bMoreResults , int iWarns ) override
	{
		m_dBuf.FinishBlock ( true ); // last doc, allow empty
		m_dBuf.FinishBlock ( false ); // docs section
		DataFinish ( m_iTotalRows, nullptr, nullptr );
		m_dBuf.FinishBlock ( false ); // root object
	}

	void Error ( const char *, const char * szError, MysqlErrors_e iErr ) override
	{
		auto _ = m_dBuf.Object ( false );
		DataFinish ( 0, szError, nullptr );
		m_bError = true;
		m_sError = szError;
	}

	void Ok ( int iAffectedRows, int iWarns, const char * sMessage, bool bMoreResults, int64_t iLastInsertId ) override
	{
		auto _ = m_dBuf.Object ( false );
		DataFinish ( iAffectedRows, nullptr, sMessage );
	}

	void HeadBegin ( int iCols ) override
	{
		m_iExpectedColumns = iCols;
		m_iTotalRows = 0;
		m_dBuf.ObjectWBlock();
		m_dBuf.Named ( "columns" );
		m_dBuf.ArrayBlock();
	}

	bool HeadEnd ( bool , int ) override
	{
		assert ( m_iExpectedColumns == 0 );
		m_dBuf.FinishBlock(false);
		m_dBuf.Named ( "data" );
		m_dBuf.ArrayWBlock();
		m_dBuf.ObjectBlock();
		return true;
	}

	void HeadColumn ( const char * szName, MysqlColumnType_e eType ) override
	{
		JsonEscapedBuilder sEscapedName;
		sEscapedName.FixupSpacedAndAppendEscaped ( szName );
		ColumnNameType_t tCol { (CSphString)sEscapedName, eType };
		auto _ = m_dBuf.Object(false);
		m_dBuf.AppendName ( tCol.first.cstr(), false );
		auto tTypeBlock = m_dBuf.Object(false);
		m_dBuf.NamedVal ( "type", eType );
		m_dColumns.Add ( tCol );
		--m_iExpectedColumns;
	}

	void Add ( BYTE ) override {}

	const JsonEscapedBuilder & Finish()
	{
		m_dBuf.FinishBlocks();
		return m_dBuf;
	}

private:
	JsonEscapedBuilder m_dBuf;
	CSphVector<ColumnNameType_t> m_dColumns;
	int m_iExpectedColumns = 0;
	int m_iTotalRows = 0;
	int m_iCol = 0;

	void AddDataColumn()
	{
		m_dBuf.AppendName ( m_dColumns[m_iCol].first.cstr(), false );
		++m_iCol;
	}

	void DataFinish ( int iTotal, const char* szError, const char* szWarning )
	{
		m_dBuf.NamedVal ( "total", iTotal );
		m_dBuf.NamedString ( "error", szError );
		m_dBuf.NamedString ( "warning", szWarning );
		m_iCol = 0;
		m_dColumns.Reset();
	}
};

/* Below is typical answer sent back by sql endpoint query mode=raw
[{
"columns":[{"id":{"type":"long long"}},{"proto":{"type":"string"}},{"state":{"type":"string"}},{"host":{"type":"string"}},{"connid":{"type":"long long"}},{"killed":{"type":"string"}},{"last cmd":{"type":"string"}}],
"data":[
{"id":2,"proto":"http","state":"query","host":"127.0.0.1:50787","connid":9,"killed":"0","last cmd":"select"},
{"id":1,"proto":"mysql,ssl","state":"query","host":"127.0.0.1:50514","connid":1,"killed":"0","last cmd":"show queries"}
],
"total":2,
"error":"",
"warning":""
}]
 */


void ConvertJsonDataset ( const bson::Bson_c & tBson, const char * sStmt, RowBuffer_i & tOut )
{
	using namespace bson;

	assert ( tBson.IsArray() );
	int iItem = 0;

	for ( BsonIterator_c dItem ( tBson ); dItem; dItem.Next() )
	{
		int iTotal = Int ( dItem.ChildByName ( "total" ) );
		CSphString sError = String ( dItem.ChildByName ( "error" ) );
		CSphString sWarning = String ( dItem.ChildByName ( "warning" ) );

		if ( !sError.IsEmpty() )
		{
			session::GetClientSession()->m_sError = sError;
			session::GetClientSession()->m_tLastMeta.m_sError = sError;
			tOut.Error ( sStmt, sError.cstr() );
			break;
		}
		if ( !iItem ) // only zero result set sets meta
		{
			session::GetClientSession()->m_tLastMeta.m_iTotalMatches = iTotal;
			session::GetClientSession()->m_tLastMeta.m_sWarning = sWarning;
		}

		using ColType_t = std::pair<CSphString, MysqlColumnType_e>;
		CSphVector<ColType_t> dSqlColumns;
		assert ( dItem.IsAssoc() );
		auto tColumnsNode = dItem.ChildByName ( "columns" );
		for ( BsonIterator_c tColumnNode ( tColumnsNode ); tColumnNode; tColumnNode.Next() )
		{
			assert ( tColumnNode.IsAssoc() ); // like {"id":{"type":"long long"}}
			tColumnNode.ForEach( [&] ( CSphString&& sName, const NodeHandle_t& tNode ) {
				auto eType = GetTypeName ( String ( Bson_c ( tNode ).ChildByName ( "type" ) ) );
				dSqlColumns.Add ( {sName,eType});
			} );
		}


		// fill headers
		if ( !dSqlColumns.IsEmpty() )
		{
			tOut.HeadBegin ( dSqlColumns.GetLength() );
			dSqlColumns.for_each ( [&] ( const auto& tColumn ) { tOut.HeadColumn ( tColumn.first.cstr(), tColumn.second ); } );
			tOut.HeadEnd();
		} else
		{
			// just simple OK reply without table
			tOut.Ok ( iTotal, ( sWarning.IsEmpty() ? 0 : 1 ) );
			break;
		}

		auto tDataNodes = dItem.ChildByName ( "data" );
		assert ( bson::IsNullNode ( tDataNodes ) || IsArray ( tDataNodes ) );
		for ( BsonIterator_c tDataRow ( tDataNodes ); tDataRow; tDataRow.Next() )
		{
			assert ( tDataRow.IsAssoc() ); // like {"id":2,"proto":"http","state":"query","host":"127.0.0.1:50787","connid":9,"killed":"0","last cmd":"select"}
			tDataRow.ForEach ( [&] ( const NodeHandle_t& tDataCol ) {
				if ( IsInt ( tDataCol ) )
					tOut.PutNumAsString ( Int ( tDataCol ) );
				else if ( IsDouble ( tDataCol ) )
					tOut.PutDoubleAsString( Double ( tDataCol ) );
				else
					tOut.PutString ( String ( tDataCol ) );
			} );
			tOut.Commit();
		}

		tOut.Eof ( iItem+1!=dItem.NumElems(), ( sWarning.IsEmpty() ? 0 : 1 ) );
		iItem++;
	}
}

class HttpRawSqlHandler_c final: public HttpHandler_c
{
	Str_t m_sQuery;

public:
	explicit HttpRawSqlHandler_c ( Str_t sQuery )
		: m_sQuery ( sQuery )
	{}

	bool Process () final
	{
		TRACE_CONN ( "conn", "HttpRawSqlHandler_c::Process" );
		if ( IsEmpty ( m_sQuery ) )
		{
			ReportError ( "query missing", SPH_HTTP_STATUS_400 );
			return false;
		}

		JsonRowBuffer_c tOut;
		session::Execute ( m_sQuery, tOut );
		if ( tOut.IsError() )
		{
			ReportError ( tOut.GetError().scstr(), SPH_HTTP_STATUS_500 );
			return false;
		}
		BuildReply ( tOut.Finish(), SPH_HTTP_STATUS_200 );
		return true;
	}
};


class HttpHandler_JsonSearch_c : public HttpSearchHandler_c
{
	Str_t m_sQuery;

public:
	explicit HttpHandler_JsonSearch_c ( Str_t sQuery )
		: m_sQuery ( sQuery )
	{}

	std::unique_ptr<QueryParser_i> PreParseQuery() override
	{
		CSphString sError;
		if ( !sphParseJsonQuery ( m_sQuery, m_tQuery, m_bProfile, sError, m_sWarning ) )
		{
			sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return nullptr;
		}

		m_eQueryType = QUERY_JSON;
		return sphCreateJsonQueryParser();
	}

protected:
	CSphString EncodeResult ( const VecTraits_T<AggrResult_t *> & dRes, QueryProfile_c * pProfile ) override
	{
		return sphEncodeResultJson ( dRes, m_tQuery, pProfile, false );
	}
};


class HttpJsonTxnTraits_c
{
protected:
	void ProcessBegin ( const CSphString& sIndex )
	{
		// for now - only local mutable indexes are suitable
		{
			auto pIndex = GetServed ( sIndex );
			if ( !ServedDesc_t::IsMutable ( pIndex ) )
				return;
		}

		HttpErrorReporter_c tReporter;
		sphHandleMysqlBegin ( tReporter, FromStr (sIndex) );
		m_iInserts = 0;
		m_iUpdates = 0;
	}

	bool ProcessCommitRollback ( Str_t sIndex, DocID_t tDocId, JsonObj_c& tResult ) const
	{
		HttpErrorReporter_c tReporter;
		sphHandleMysqlCommitRollback ( tReporter, sIndex, true );

		if ( tReporter.IsError() )
		{
			tResult = sphEncodeInsertErrorJson ( sIndex.first, tReporter.GetError() );
		} else
		{
			auto iDeletes = tReporter.GetAffectedRows();
			auto dLastIds = session::LastIds();
			if ( !dLastIds.IsEmpty() )
				tDocId = dLastIds[0];
			tResult = sphEncodeTxnResultJson ( sIndex.first, tDocId, m_iInserts, iDeletes, m_iUpdates );
		}
		return !tReporter.IsError();
	}

	int m_iInserts = 0;
	int m_iUpdates = 0;
};

static bool ProcessInsert ( SqlStmt_t& tStmt, DocID_t tDocId, JsonObj_c& tResult )
{
	HttpErrorReporter_c tReporter;
	sphHandleMysqlInsert ( tReporter, tStmt );

	if ( tReporter.IsError() )
	{
		tResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
	} else
	{
		auto dLastIds = session::LastIds();
		if ( !dLastIds.IsEmpty() )
			tDocId = dLastIds[0];
		tResult = sphEncodeInsertResultJson ( tStmt.m_sIndex.cstr(), tStmt.m_eStmt == STMT_REPLACE, tDocId );
	}

	return !tReporter.IsError();
}

static bool ProcessDelete ( Str_t sRawRequest, const SqlStmt_t& tStmt, DocID_t tDocId, JsonObj_c& tResult )
{
	HttpErrorReporter_c tReporter;
	sphHandleMysqlDelete ( tReporter, tStmt, std::move ( sRawRequest ) );

	if ( tReporter.IsError() )
		tResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
	else
		tResult = sphEncodeDeleteResultJson ( tStmt.m_sIndex.cstr(), tDocId, tReporter.GetAffectedRows() );

	return !tReporter.IsError();
}

class HttpHandler_JsonInsert_c final : public HttpHandler_c
{
	Str_t m_sQuery;
	bool m_bReplace;

public:
	HttpHandler_JsonInsert_c ( Str_t sQuery, bool bReplace )
		: m_sQuery ( sQuery )
		, m_bReplace ( bReplace )
	{}

	bool Process () final
	{
		TRACE_CONN ( "conn", "HttpHandler_JsonInsert_c::Process" );
		SqlStmt_t tStmt;
		DocID_t tDocId = 0;
		CSphString sError;
		if ( !sphParseJsonInsert ( m_sQuery.first, tStmt, tDocId, m_bReplace, false, sError ) )
		{
			sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return false;
		}

		tStmt.m_sEndpoint = HttpEndpointToStr ( m_bReplace ? SPH_HTTP_ENDPOINT_JSON_REPLACE : SPH_HTTP_ENDPOINT_JSON_INSERT );
		JsonObj_c tResult = JsonNull;
		bool bResult = ProcessInsert ( tStmt, tDocId, tResult );

		CSphString sResult = tResult.AsString();
		BuildReply ( sResult, bResult ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500 );

		return bResult;
	}
};


class HttpJsonUpdateTraits_c
{
	int m_iLastUpdated = 0;

protected:
	bool ProcessUpdate ( Str_t sRawRequest, const SqlStmt_t & tStmt, DocID_t tDocId, JsonObj_c & tResult )
	{
		HttpErrorReporter_c tReporter;
		sphHandleMysqlUpdate ( tReporter, tStmt, sRawRequest );

		if ( tReporter.IsError() )
			tResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
		else
			tResult = sphEncodeUpdateResultJson ( tStmt.m_sIndex.cstr(), tDocId, tReporter.GetAffectedRows() );

		m_iLastUpdated = tReporter.GetAffectedRows();

		return !tReporter.IsError();
	}

	int GetLastUpdated() const
	{
		return m_iLastUpdated;
	}
};


class HttpHandler_JsonUpdate_c : public HttpHandler_c, HttpJsonUpdateTraits_c
{
protected:
	Str_t m_sQuery;

public:
	explicit HttpHandler_JsonUpdate_c ( Str_t sQuery )
		: m_sQuery ( sQuery )
	{}

	bool Process () final
	{
		TRACE_CONN ( "conn", "HttpHandler_JsonUpdate_c::Process" );
		SqlStmt_t tStmt;
		tStmt.m_bJson = true;
		tStmt.m_sEndpoint = HttpEndpointToStr ( SPH_HTTP_ENDPOINT_JSON_UPDATE );

		DocID_t tDocId = 0;
		CSphString sError;
		if ( !ParseQuery ( tStmt, tDocId, sError ) )
		{
			sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return false;
		}

		JsonObj_c tResult = JsonNull;
		bool bResult = ProcessQuery ( tStmt, tDocId, tResult );
		BuildReply ( tResult.AsString(), bResult ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500 );

		return bResult;
	}

protected:
	virtual bool ParseQuery ( SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError )
	{
		return sphParseJsonUpdate ( m_sQuery, tStmt, tDocId, sError );
	}

	virtual bool ProcessQuery ( const SqlStmt_t & tStmt, DocID_t tDocId, JsonObj_c & tResult )
	{
		return ProcessUpdate ( m_sQuery, tStmt, tDocId, tResult );
	}
};


class HttpHandler_JsonDelete_c final : public HttpHandler_JsonUpdate_c
{
public:
	explicit HttpHandler_JsonDelete_c ( Str_t sQuery )
		: HttpHandler_JsonUpdate_c ( sQuery )
	{}

protected:
	bool ParseQuery ( SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError ) final
	{
		tStmt.m_sEndpoint = HttpEndpointToStr ( SPH_HTTP_ENDPOINT_JSON_DELETE );
		return sphParseJsonDelete ( m_sQuery, tStmt, tDocId, sError );
	}

	bool ProcessQuery ( const SqlStmt_t & tStmt, DocID_t tDocId, JsonObj_c & tResult ) final
	{
		return ProcessDelete ( m_sQuery, tStmt, tDocId, tResult );
	}
};

// stream for ndjsons
class NDJsonStream_c
{
	CharStream_c& m_tIn;
	CSphVector<char> m_dLastChunk;
	Str_t m_sCurChunk { dEmptyStr };
	bool m_bDone;

	int m_iJsons = 0;
	int m_iReads = 0;
	int m_iTotallyRead = 0; // not used, but provides data during debug

public:
	explicit NDJsonStream_c ( CharStream_c& tIn )
		: m_tIn { tIn }
		, m_bDone { m_tIn.Eof() }
	{}

	inline bool Eof() const { return m_bDone;}

	Str_t Read()
	{
		assert ( !m_bDone );
		while (true)
		{
			if ( IsEmpty ( m_sCurChunk ) )
			{
				if ( m_tIn.Eof() )
					break;
				m_sCurChunk = m_tIn.Read();
				m_iTotallyRead += m_sCurChunk.second;
				++m_iReads;
			}

			const char* szJson = m_sCurChunk.first;
			const char* pEnd = szJson + m_sCurChunk.second;
			const char* p = szJson;

			while ( p < pEnd && *p != '\r' && *p != '\n' )
				++p;

			if ( p==pEnd )
			{
				m_dLastChunk.Append ( szJson, p-szJson );
				m_sCurChunk = dEmptyStr;
				continue;
			}

			*( const_cast<char*> ( p ) ) = '\0';
			++p;
			m_sCurChunk = { p, pEnd - p };

			Str_t sResult;
			if ( m_dLastChunk.IsEmpty () )
			{
				sResult =  { szJson, p - szJson - 1 };
				if ( IsEmpty ( sResult ) )
					continue;
				++m_iJsons;
				HTTPINFO << "non-last chunk " << m_iJsons << " " << sResult;;
			}
			else
			{
				m_dLastChunk.Append ( szJson, p - szJson );
				sResult = m_dLastChunk;
				--sResult.second; // exclude terminating \0
				m_dLastChunk.Resize ( 0 );
				++m_iJsons;
				HTTPINFO << "Last chunk " << m_iJsons << " " << sResult;
			}
			return sResult;
		}

		m_bDone = true;
		m_dLastChunk.Add ( '\0' );
		m_dLastChunk.Resize ( m_dLastChunk.GetLength() - 1 );
		Str_t sResult = m_dLastChunk;
		++m_iJsons;
		HTTPINFO << "Termination chunk " << m_iJsons << " " << sResult;
		return sResult;
	}
};


class HttpHandler_JsonBulk_c : public HttpHandler_c, public HttpJsonUpdateTraits_c, public HttpJsonTxnTraits_c
{
protected:
	NDJsonStream_c m_tSource;
	const OptionsHash_t& m_tOptions;

public:
	HttpHandler_JsonBulk_c ( CharStream_c& tSource, const OptionsHash_t & tOptions )
		: m_tSource ( tSource )
		, m_tOptions ( tOptions )
	{}

	bool Process ()
	{
		TRACE_CONN ( "conn", "HttpHandler_JsonBulk_c::Process" );
		const char* szError = IsNDJson ();
		if ( szError )
		{
			ReportError ( szError, SPH_HTTP_STATUS_400 );
			return false;
		}

		if ( m_tSource.Eof() )
		{
			BuildReply ( R"({"items":[],"errors":false})", SPH_HTTP_STATUS_200 );
			return true;
		}

		// originally we execute txn for single index
		// if there is combo, we fall-back to query-by-query commits
		JsonObj_c tRoot;
		JsonObj_c tItems ( true );
		CSphString sTxnIdx;
		CSphString sStmt;
		bool bResult = false;
		int iInserted = 0;
		while ( !m_tSource.Eof() )
		{
			auto tQuery = m_tSource.Read();
			if ( IsEmpty ( tQuery ) )
				continue;
			++iInserted;
			auto& tCrashQuery = GlobalCrashQueryGetRef();
			tCrashQuery.m_dQuery = { (const BYTE*) tQuery.first, tQuery.second };
			const char* szStmt = tQuery.first;
			SqlStmt_t tStmt;
			tStmt.m_bJson = true;
			DocID_t tDocId = 0;
			CSphString sError;
			CSphString sQuery;
			if ( !sphParseJsonStatement ( szStmt, tStmt, sStmt, sQuery, tDocId, sError ) )
			{
				sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
				ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
				HTTPINFO << "inserted  " << iInserted;
				return false;
			}

			JsonObj_c tResult = JsonNull;
			bResult = false;

			if ( sTxnIdx.IsEmpty() )
			{
				sTxnIdx = tStmt.m_sIndex;
				ProcessBegin ( sTxnIdx );
			}
			else if ( session::IsInTrans() && sTxnIdx!=tStmt.m_sIndex )
			{
				assert ( !sTxnIdx.IsEmpty() );
				// we should finish current txn, as we got another index
				bResult = ProcessCommitRollback ( FromStr ( sTxnIdx ), tDocId, tResult );
				sStmt = "bulk";
				AddResult ( tItems, sStmt, tResult );
				if ( !bResult )
					break;
				sTxnIdx = tStmt.m_sIndex;
				ProcessBegin ( sTxnIdx );
			}

			switch ( tStmt.m_eStmt )
			{
			case STMT_INSERT:
			case STMT_REPLACE:
				bResult = ProcessInsert ( tStmt, tDocId, tResult );
				if ( bResult )
					++m_iInserts;
				break;

			case STMT_UPDATE:
				tStmt.m_sEndpoint = HttpEndpointToStr ( SPH_HTTP_ENDPOINT_JSON_UPDATE );
				bResult = ProcessUpdate ( FromStr ( sQuery ), tStmt, tDocId, tResult );
				if ( bResult )
					m_iUpdates += GetLastUpdated();
				break;

			case STMT_DELETE:
				tStmt.m_sEndpoint = HttpEndpointToStr ( SPH_HTTP_ENDPOINT_JSON_DELETE );
				bResult = ProcessDelete ( FromStr ( sQuery ), tStmt, tDocId, tResult );
				break;

			default:
				ReportError ( "Unknown statement", SPH_HTTP_STATUS_400 );
				HTTPINFO << "inserted  " << iInserted;
				return false;
			}

			if ( !bResult || !session::IsInTrans() )
				AddResult ( tItems, sStmt, tResult );

			// no further than the first error
			if ( !bResult )
				break;
		}

		if ( bResult && session::IsInTrans() )
		{
			assert ( !sTxnIdx.IsEmpty() );
			// We're in txn - that is, nothing committed, and we should do it right now
			JsonObj_c tResult;
			bResult = ProcessCommitRollback ( FromStr ( sTxnIdx ), 0, tResult );
			sStmt = "bulk";
			AddResult ( tItems, sStmt, tResult );
		}

		session::SetInTrans ( false );

		tRoot.AddItem ( "items", tItems );
		tRoot.AddBool ( "errors", !bResult );
		BuildReply ( tRoot.AsString(), bResult ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500 );
		HTTPINFO << "inserted  " << iInserted;
		return true;
	}

protected:
	static void AddResult ( JsonObj_c & tRoot, CSphString & sStmt, JsonObj_c & tResult )
	{
		JsonObj_c tItem;
		tItem.AddItem ( sStmt.cstr(), tResult );
		tRoot.AddItem ( tItem );
	}

	const char* IsNDJson() const
	{
		if ( !m_tOptions.Exists ( "Content-Type" ) )
			return "Content-Type must be set";

		auto sContentType = m_tOptions["Content-Type"].ToLower();
		auto dParts = sphSplit ( sContentType.cstr(), ";" );
		if ( dParts.IsEmpty() || dParts[0] != "application/x-ndjson" )
			return "Content-Type must be application/x-ndjson";
		return nullptr;
	}
};

class HttpHandlerPQ_c final : public HttpHandler_c
{
	Str_t m_sQuery;
	const OptionsHash_t& m_tOptions;
public:
	HttpHandlerPQ_c ( Str_t sQuery, const OptionsHash_t& tOptions )
		: m_sQuery ( sQuery )
		, m_tOptions ( tOptions )
	{}

	bool Process () final;

private:
	// FIXME!!! handle replication for InsertOrReplaceQuery and Delete
	bool DoCallPQ ( const CSphString & sIndex, const JsonObj_c & tPercolate, bool bVerbose );
	bool InsertOrReplaceQuery ( const CSphString& sIndex, const JsonObj_c& tJsonQuery, const JsonObj_c& tRoot, CSphString* pUID, bool bReplace );
	bool ListQueries ( const CSphString & sIndex );
	bool Delete ( const CSphString & sIndex, const JsonObj_c & tRoot );
};

struct BulkDoc_t
{
	CSphString m_sAction;
	CSphString m_sIndex;
	DocID_t m_tDocid { 0 };
	Str_t m_tDocLine;
};

struct BulkTnx_t
{
	int m_iFrom { -1 };
	int m_iCount { 0 };
};


class HttpHandlerEsBulk_c : public HttpHandler_c, public HttpJsonUpdateTraits_c, public HttpJsonTxnTraits_c
{
protected:
	Str_t m_tSource;
	const OptionsHash_t & m_hOptions;

public:
	HttpHandlerEsBulk_c ( Str_t tSource, const OptionsHash_t & hOptions )
		: m_tSource ( tSource )
		, m_hOptions ( hOptions )
	{}

	bool Process () override;

private:
	bool ProcessTnx ( const VecTraits_T<BulkTnx_t> & dTnx, VecTraits_T<BulkDoc_t> & dDocs, JsonObj_c & tItems );
};

static std::unique_ptr<HttpHandler_c> CreateHttpHandler ( ESphHttpEndpoint eEndpoint, CharStream_c & tSource, Str_t & sQuery, OptionsHash_t & tOptions, http_method eRequestType )
{
	const CSphString * pOption = nullptr;
	sQuery = dEmptyStr;

	auto SetQuery = [&sQuery] ( Str_t&& sData ) {
		auto& tCrashQuery = GlobalCrashQueryGetRef();
		tCrashQuery.m_dQuery = { (const BYTE*)sData.first, sData.second };
		sQuery = sData;
	};

	// SPH_HTTP_ENDPOINT_SQL SPH_HTTP_ENDPOINT_CLI these endpoints url-encoded, all others are plain json, and we don't want to waste time pre-parsing them
	if ( eEndpoint == SPH_HTTP_ENDPOINT_SQL || eEndpoint == SPH_HTTP_ENDPOINT_CLI )
	{
		auto sWholeData = tSource.ReadAll();
		StoreRawQuery ( tOptions, sWholeData );
		HttpRequestParser_c::ParseList ( sWholeData, tOptions );
	}

	switch ( eEndpoint )
	{
	case SPH_HTTP_ENDPOINT_SQL:
		pOption = tOptions ( "mode" );
		if ( pOption && *pOption=="raw" )
		{
			auto pQuery = tOptions ( "query" );
			if ( pQuery )
				SetQuery ( FromStr ( *pQuery ) );
			return std::make_unique<HttpRawSqlHandler_c> ( sQuery ); // non-json
		}
		else
		{
			return std::make_unique<HttpSearchHandler_SQL_c> ( tOptions ); // non-json
		}

	case SPH_HTTP_ENDPOINT_CLI:
		{
			pOption = tOptions ( "raw_query" );
			if ( pOption )
			{
				SetQuery ( FromStr (*pOption) );
			} else
			{
				SetQuery ( tSource.ReadAll() );
			}
			return std::make_unique<HttpRawSqlHandler_c> ( sQuery ); // non-json
		}

	case SPH_HTTP_ENDPOINT_JSON_SEARCH:
		SetQuery ( tSource.ReadAll() );
		return std::make_unique<HttpHandler_JsonSearch_c> ( sQuery ); // json

	case SPH_HTTP_ENDPOINT_JSON_INDEX:
	case SPH_HTTP_ENDPOINT_JSON_CREATE:
	case SPH_HTTP_ENDPOINT_JSON_INSERT:
	case SPH_HTTP_ENDPOINT_JSON_REPLACE:
		SetQuery ( tSource.ReadAll() );
		return std::make_unique<HttpHandler_JsonInsert_c> ( sQuery, eEndpoint==SPH_HTTP_ENDPOINT_JSON_INDEX || eEndpoint==SPH_HTTP_ENDPOINT_JSON_REPLACE ); // json

	case SPH_HTTP_ENDPOINT_JSON_UPDATE:
		SetQuery ( tSource.ReadAll() );
		return std::make_unique<HttpHandler_JsonUpdate_c> ( sQuery ); // json

	case SPH_HTTP_ENDPOINT_JSON_DELETE:
		SetQuery ( tSource.ReadAll() );
		return std::make_unique<HttpHandler_JsonDelete_c> ( sQuery ); // json

	case SPH_HTTP_ENDPOINT_JSON_BULK:
		return std::make_unique<HttpHandler_JsonBulk_c> ( tSource, tOptions ); // json

	case SPH_HTTP_ENDPOINT_PQ:
		SetQuery ( tSource.ReadAll() );
		return std::make_unique<HttpHandlerPQ_c> ( sQuery, tOptions ); // json

	case SPH_HTTP_ENDPOINT_ES_BULK:
		return std::make_unique<HttpHandlerEsBulk_c> ( tSource.ReadAll(), tOptions );

	default:
		break;
	}

	return nullptr;
}

HttpProcessResult_t ProcessHttpQuery ( CharStream_c & tSource, Str_t & sQuery, OptionsHash_t & hOptions, CSphVector<BYTE> & dResult, bool bNeedHttpResponse, http_method eRequestType )
{
	TRACE_CONN ( "conn", "ProcessHttpQuery" );

	HttpProcessResult_t tRes;

	const CSphString & sEndpoint = hOptions["endpoint"];
	tRes.m_eEndpoint = StrToHttpEndpoint ( sEndpoint );

	std::unique_ptr<HttpHandler_c> pHandler = CreateHttpHandler ( tRes.m_eEndpoint, tSource, sQuery, hOptions, eRequestType );
	if ( !pHandler )
	{
		if ( tRes.m_eEndpoint == SPH_HTTP_ENDPOINT_INDEX )
		{
			HttpHandlerIndexPage ( dResult );
		} else
		{
			DumpHttp ( eRequestType, sEndpoint, tSource.ReadAll() );
			tRes.m_sError.SetSprintf ( "/%s - unsupported endpoint", sEndpoint.cstr() );
			sphHttpErrorReply ( dResult, SPH_HTTP_STATUS_501, tRes.m_sError.cstr() );
		}
		return tRes;
	}

	pHandler->SetErrorFormat ( bNeedHttpResponse );
	tRes.m_bOk = pHandler->Process();
	tRes.m_sError = pHandler->GetError();
	dResult = std::move ( pHandler->GetResult() );

	return tRes;
}

bool sphProcessHttpQueryNoResponce ( const CSphString & sEndpoint, const CSphString& sQuery, CSphVector<BYTE> & dResult )
{
	OptionsHash_t hOptions;
	hOptions.Add ( sEndpoint, "endpoint" );

	BlobStream_c tQuery { sQuery };
	return ProcessHttpQueryBuddy ( tQuery, hOptions, dResult, false, HTTP_GET ).m_bOk;
}

bool HttpRequestParser_c::ProcessClientHttp ( AsyncNetInputBuffer_c& tIn, CSphVector<BYTE>& dResult )
{
	assert ( !m_szError );
	std::unique_ptr<CharStream_c> pSource;

	if ( m_tParser.flags & F_CHUNKED )
	{
		pSource = std::make_unique<ChunkedSocketStream_c> ( tIn, &m_tParser, m_bBodyDone, std::move ( m_dParsedBodies ), m_iLastParsed );
	} else
	{
		// for non-chunked - need to throw out beginning of the packet (with header). Only body rest in the buffer.
		tIn.PopTail ( m_iLastParsed - ParsedBodyLength() );
		int iFullLength = ParsedBodyLength() + ( (int)m_tParser.content_length > 0 ? (int)m_tParser.content_length : 0 );
		pSource = std::make_unique<RawSocketStream_c> ( tIn, iFullLength );
	}

	ESphHttpEndpoint eEndpoint = StrToHttpEndpoint ( m_hOptions["endpoint"] );
	if ( IsLogManagementEnabled() && eEndpoint==SPH_HTTP_ENDPOINT_TOTAL && m_sEndpoint.Ends ( "_bulk" ) )
		eEndpoint = SPH_HTTP_ENDPOINT_ES_BULK;
	if ( IsLogManagementEnabled() && eEndpoint==SPH_HTTP_ENDPOINT_TOTAL )
	{
		ProcessCompatHttp ( HttpRequestParsed_t ( pSource->ReadAll(), m_eType, m_hOptions ), dResult );
		return true;
	}

	return ProcessHttpQueryBuddy ( *pSource, m_hOptions, dResult, true, m_eType ).m_bOk;
}

void sphHttpErrorReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * szError )
{
	JsonObj_c tErr;
	tErr.AddStr ( "error", szError );
	CSphString sJsonError = tErr.AsString();
	HttpBuildReply ( dData, eCode, FromStr (sJsonError), false );
}

static void EncodePercolateMatchResult ( const PercolateMatchResult_t & tRes, const CSphFixedVector<int64_t> & dDocids,	const CSphString & sIndex, JsonEscapedBuilder & tOut )
{
	ScopedComma_c sRootBlock ( tOut, ",", "{", "}" );

	// column names
	tOut.Sprintf ( R"("took":%d,"timed_out":false)", ( int ) ( tRes.m_tmTotal / 1000 ));

	// hits {
	ScopedComma_c sHitsBlock ( tOut, ",", R"("hits":{)", "}");
	tOut.Sprintf ( R"("total":%d,"max_score":1)", tRes.m_dQueryDesc.GetLength()); // FIXME!!! track and provide weight
	if ( tRes.m_bVerbose )
		tOut.Sprintf ( R"("early_out_queries":%d,"matched_queries":%d,"matched_docs":%d,"only_terms_queries":%d,"total_queries":%d)",
			tRes.m_iEarlyOutQueries, tRes.m_iQueriesMatched, tRes.m_iDocsMatched, tRes.m_iOnlyTerms, tRes.m_iTotalQueries );

	// documents
	tOut.StartBlock ( ",", R"("hits":[)", "]");

	int iDocOff = 0;
	for ( const auto& tDesc : tRes.m_dQueryDesc )
	{
		ScopedComma_c sQueryComma ( tOut, ",","{"," }");
		tOut.Sprintf ( R"("_index":"%s","_type":"doc","_id":"%U","_score":"1")", sIndex.cstr(), tDesc.m_iQUID );
		if ( !tDesc.m_bQL )
			tOut.Sprintf ( R"("_source":{"query":%s})", tDesc.m_sQuery.cstr () );
		else
		{
			ScopedComma_c sBrackets ( tOut, nullptr, R"("_source":{ "query": {"ql":)", " } }");
			tOut.AppendEscapedWithComma ( tDesc.m_sQuery.cstr() );
		}

		// document count + document id(s)
		if ( tRes.m_bGetDocs )
		{
			ScopedComma_c sFields ( tOut, ",",R"("fields":{"_percolator_document_slot": [)", "] }");
			int iDocs = tRes.m_dDocs[iDocOff];
			for ( int iDoc = 1; iDoc<=iDocs; ++iDoc )
			{
				auto iRow = tRes.m_dDocs[iDocOff + iDoc];
				tOut.Sprintf ("%l", DocID_t ( dDocids.IsEmpty () ? iRow : dDocids[iRow] ) );
			}
			iDocOff += iDocs + 1;
		}
	}

	tOut.FinishBlock ( false ); // hits[]
	// all the rest blocks (root, hits) will be auto-closed here.
}


bool HttpHandlerPQ_c::DoCallPQ ( const CSphString & sIndex, const JsonObj_c & tPercolate, bool bVerbose )
{
	CSphString sWarning, sError, sTmp;
	BlobVec_t dDocs;

	// single document
	JsonObj_c tJsonDoc = tPercolate.GetObjItem ( "document", sTmp );
	if ( tJsonDoc )
	{
		auto & tDoc = dDocs.Add();
		if ( !bson::JsonObjToBson ( tJsonDoc, tDoc, g_bJsonAutoconvNumbers, g_bJsonKeynamesToLowercase ) )
		{
			ReportError ( "Bad cjson", SPH_HTTP_STATUS_400 );
			return false;
		}
	}

	// multiple documents
	JsonObj_c tJsonDocs = tPercolate.GetArrayItem ( "documents", sError, true );
	if ( !sError.IsEmpty() )
	{
		ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
		return false;
	}

	for ( auto i : tJsonDocs )
	{
		auto & tDoc = dDocs.Add();
		if ( !bson::JsonObjToBson ( i, tDoc, g_bJsonAutoconvNumbers, g_bJsonKeynamesToLowercase ) )
		{
			ReportError ( "Bad cjson", SPH_HTTP_STATUS_400 );
			return false;
		}
	}

	if ( dDocs.IsEmpty() )
	{
		ReportError ( "no documents found", SPH_HTTP_STATUS_400 );
		return false;
	}

	PercolateOptions_t tOpts;
	tOpts.m_sIndex = sIndex;
	tOpts.m_bGetDocs = true;
	tOpts.m_bVerbose = bVerbose;
	tOpts.m_bGetQuery = true;
	// fixme! id alias here is 'id' or 'uid'. Process it!

	CSphSessionAccum tAcc;
	CPqResult tResult;
	tResult.m_dResult.m_bGetFilters = false;

	PercolateMatchDocuments ( dDocs, tOpts, tAcc, tResult );

	JsonEscapedBuilder sRes;
	EncodePercolateMatchResult ( tResult.m_dResult, tResult.m_dDocids, sIndex, sRes );
	BuildReply ( sRes, SPH_HTTP_STATUS_200 );

	return true;
}


static void EncodePercolateQueryResult ( bool bReplace, const CSphString & sIndex, int64_t iID, StringBuilder_c & tOut )
{
	if ( bReplace )
		tOut.Sprintf (R"({"index":"%s","type":"doc","_id":"%U","result":"updated","forced_refresh":true})", sIndex.cstr(), iID);
	else
		tOut.Sprintf ( R"({"index":"%s","type":"doc","_id":"%U","result":"created"})", sIndex.cstr (), iID );
}


bool HttpHandlerPQ_c::InsertOrReplaceQuery ( const CSphString& sIndex, const JsonObj_c& tJsonQuery,
	const JsonObj_c& tRoot, CSphString* pUID, bool bReplace )
{
	CSphString sTmp, sError, sWarning;

	bool bQueryQL = true;
	CSphQuery tQuery;
	const char * sQuery = nullptr;
	JsonObj_c tQueryQL = tJsonQuery.GetStrItem ( "ql", sTmp );
	if ( tQueryQL )
		sQuery = tQueryQL.SzVal();
	else
	{
		bQueryQL = false;
		if ( !ParseJsonQueryFilters ( tJsonQuery, tQuery, sError, sWarning ) )
		{
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return false;
		}

		if ( NonEmptyQuery ( tJsonQuery ) )
			sQuery = tQuery.m_sQuery.cstr();
	}

	if ( !sQuery || *sQuery=='\0' )
	{
		ReportError ( "no query found", SPH_HTTP_STATUS_400 );
		return false;
	}

	int64_t iID = 0;
	if ( pUID && !pUID->IsEmpty() )
		iID = strtoll ( pUID->cstr(), nullptr, 10 );

	JsonObj_c tTagsArray = tRoot.GetArrayItem ( "tags", sError, true );
	if ( !sError.IsEmpty() )
	{
		ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
		return false;
	}

	StringBuilder_c sTags (", ");
	for ( const auto & i : tTagsArray )
		sTags << i.SzVal();

	JsonObj_c tFilters = tRoot.GetStrItem ( "filters", sError, true );
	if ( !sError.IsEmpty() )
	{
		ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
		return false;
	}

	if ( tFilters && !bQueryQL && tQuery.m_dFilters.GetLength() )
	{
		ReportError ( "invalid combination of SphinxQL and query filter provided", SPH_HTTP_STATUS_501 );
		return false;
	}

	CSphVector<CSphFilterSettings> dFilters;
	CSphVector<FilterTreeItem_t> dFilterTree;
	if ( tFilters )
	{
		auto pServed = GetServed ( sIndex );
		if ( !CheckValid ( pServed, sIndex, IndexType_e::PERCOLATE ) )
			return false;

		RIdx_T<const PercolateIndex_i*> pIndex { pServed };

		if ( !PercolateParseFilters ( tFilters.SzVal(), SPH_COLLATION_UTF8_GENERAL_CI, pIndex->GetInternalSchema (), dFilters, dFilterTree, sError ) )
		{
			ReportError ( sError.scstr(), SPH_HTTP_STATUS_400 );
			return false;
		}
	} else
	{
		dFilters.SwapData ( tQuery.m_dFilters );
		dFilterTree.SwapData ( tQuery.m_dFilterTree );
	}

	// scope for index lock
	bool bOk = false;
	{
		auto pServed = GetServed ( sIndex );
		if ( !CheckValid ( pServed, sIndex, IndexType_e::PERCOLATE ))
			return false;

		RIdx_T<PercolateIndex_i*> pIndex { pServed };

		PercolateQueryArgs_t tArgs ( dFilters, dFilterTree );
		tArgs.m_sQuery = sQuery;
		tArgs.m_sTags = sTags.cstr();
		tArgs.m_iQUID = iID;
		tArgs.m_bReplace = bReplace;
		tArgs.m_bQL = bQueryQL;

		// add query
		auto pStored = pIndex->CreateQuery ( tArgs, sError );
		if ( pStored )
		{
			auto* pSession = session::GetClientSession();
			auto& tAcc = pSession->m_tAcc;
			auto* pAccum = tAcc.GetAcc( pIndex, sError );

			ReplicationCommand_t * pCmd = pAccum->AddCommand ( ReplicationCommand_e::PQUERY_ADD, sIndex );
			// refresh query's UID for reply as it might be auto-generated
			iID = pStored->m_iQUID;
			pCmd->m_pStored = std::move ( pStored );

			bOk = HandleCmdReplicate ( *pAccum, sError );
		}
	}

	if ( !bOk )
		ReportError ( sError.scstr(), SPH_HTTP_STATUS_500 );
	else
	{
		StringBuilder_c sRes;
		EncodePercolateQueryResult ( bReplace, sIndex, iID, sRes );
		BuildReply ( sRes, SPH_HTTP_STATUS_200 );
	}

	return bOk;
}

// for now - forcibly route query as /json/search POST {"index":"<idx>"}. Later matter of deprecate/delete
bool HttpHandlerPQ_c::ListQueries ( const CSphString & sIndex )
{
	StringBuilder_c sQuery;
	sQuery.Sprintf(R"({"index":"%s"})", sIndex.scstr());
	auto pHandler = std::make_unique<HttpHandler_JsonSearch_c> ( (Str_t)sQuery ) ;
	if ( !pHandler )
		return false;

	pHandler->SetErrorFormat (m_bNeedHttpResponse);
	pHandler->Process ();
	m_dData = std::move ( pHandler->GetResult ());
	return true;
}

bool HttpHandlerPQ_c::Delete ( const CSphString & sIndex, const JsonObj_c & tRoot )
{
	auto* pSession = session::GetClientSession();
	auto& tAcc = pSession->m_tAcc;
	auto* pAccum = tAcc.GetAcc ();

	ReplicationCommand_t * pCmd = pAccum->AddCommand ( ReplicationCommand_e::PQUERY_DELETE, sIndex );

	CSphString sError;
	JsonObj_c tTagsArray = tRoot.GetArrayItem ( "tags", sError, true );
	if ( !sError.IsEmpty() )
	{
		ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
		return false;
	}

	StringBuilder_c sTags ( ", " );
	for ( const auto & i : tTagsArray )
		sTags << i.SzVal();

	JsonObj_c tUidsArray = tRoot.GetArrayItem ( "id", sError, true );
	if ( !sError.IsEmpty() )
	{
		ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
		return false;
	}

	for ( const auto & i : tUidsArray )
		pCmd->m_dDeleteQueries.Add ( i.IntVal() );

	if ( !sTags.GetLength() && !pCmd->m_dDeleteQueries.GetLength() )
	{
		ReportError ( "no tags or id field arrays found", SPH_HTTP_STATUS_400 );
		return false;
	}

	pCmd->m_sDeleteTags = sTags.cstr();
	uint64_t tmStart = sphMicroTimer();

	int iDeleted = 0;
	bool bOk = HandleCmdReplicate ( *pAccum, sError, iDeleted );

	uint64_t tmTotal = sphMicroTimer() - tmStart;

	if ( !bOk )
	{
		FormatError ( SPH_HTTP_STATUS_400, "%s", sError.cstr() );
		return false;
	}

	StringBuilder_c tOut;
	tOut.Sprintf (R"({"took":%d,"timed_out":false,"deleted":%d,"total":%d,"failures":[]})",
		( int ) ( tmTotal / 1000 ), iDeleted, iDeleted );
	BuildReply ( tOut, SPH_HTTP_STATUS_200 );

	return true;
}

bool HttpHandlerPQ_c::Process()
{
	TRACE_CONN ( "conn", "HttpHandlerPQ_c::Process" );
	CSphString * sEndpoint = m_tOptions ( "endpoint" );
	if ( !sEndpoint || sEndpoint->IsEmpty() )
	{
		FormatError ( SPH_HTTP_STATUS_400, "invalid empty endpoint, should be /json/pq/index_name/operation");
		return false;
	}

	assert ( sEndpoint->Begins ( "json/pq/" ) );
	StrVec_t dPoints;
	sphSplit ( dPoints, sEndpoint->cstr() + sizeof("json/pq/") - 1, "/" );
	if ( dPoints.GetLength()<2 )
	{
		FormatError ( SPH_HTTP_STATUS_400, "invalid endpoint '%s', should be /json/pq/index_name/operation", sEndpoint->scstr() );
		return false;
	}

	const CSphString & sIndex = dPoints[0];
	const CSphString & sOp = dPoints[1];
	CSphString * pUID = nullptr;
	if ( dPoints.GetLength()>2 )
		pUID = dPoints.Begin() + 2;

	bool bMatch = false;
	bool bDelete = false;
	if ( sOp=="_delete_by_query" )
		bDelete = true;
	else if ( sOp!="doc" )
		bMatch = true;

	if ( IsEmpty ( m_sQuery ) )
		return ListQueries ( sIndex );

	const JsonObj_c tRoot ( m_sQuery );
	if ( !tRoot )
	{
		ReportError ( "bad JSON object", SPH_HTTP_STATUS_400 );
		return false;
	}

	if ( !tRoot.Size() )
		return ListQueries ( sIndex );

	CSphString sError;
	JsonObj_c tQuery = tRoot.GetObjItem ( "query", sError );
	if ( !tQuery && !bDelete )
	{
		ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
		return false;
	}

	JsonObj_c tPerc = bMatch ? tQuery.GetObjItem ( "percolate", sError ) : JsonNull;
	if ( bMatch && !tPerc )
	{
		ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
		return false;
	}

	bool bVerbose = false;
	JsonObj_c tVerbose = tRoot.GetItem ( "verbose" );
	if ( tVerbose )
	{
		if ( tVerbose.IsDbl() )
			bVerbose = tVerbose.DblVal()!=0.0;
		else if ( tVerbose.IsInt() )
			bVerbose = tVerbose.IntVal()!=0;
		else if ( tVerbose.IsBool() )
			bVerbose = tVerbose.BoolVal();
	}

	if ( bMatch )
		return DoCallPQ ( sIndex, tPerc, bVerbose );
	else if ( bDelete )
		return Delete ( sIndex, tRoot );
	else
	{
		bool bRefresh = false;
		CSphString * pRefresh = m_tOptions ( "refresh" );
		if ( pRefresh && !pRefresh->IsEmpty() )
		{
			if ( *pRefresh=="0" )
				bRefresh = false;
			else if ( *pRefresh=="1" )
				bRefresh = true;
		}

		return InsertOrReplaceQuery ( sIndex, tQuery, tRoot, pUID, bRefresh );
	}
}

static bool ParseMetaLine ( const char * sLine, BulkDoc_t & tDoc, CSphString & sError )
{
	JsonObj_c tLineMeta ( sLine );
	JsonObj_c tAction = tLineMeta[0];
	if ( !tAction )
	{
		sError = "no statement found";
		return false;
	}

	tDoc.m_sAction = tAction.Name();

	if ( !tAction.IsObj() )
	{
		sError.SetSprintf ( "statement %s should be an object", tDoc.m_sAction.cstr() );
		return false;
	}

	JsonObj_c tIndex = tAction.GetStrItem ( "_index", sError );
	if ( !tIndex )
		return false;

	tDoc.m_sIndex = tIndex.StrVal();
	
	JsonObj_c tId = tAction.GetItem ( "_id" );
	if ( tId && tId.IsNum() )
		tDoc.m_tDocid = tId.IntVal();

	return true;
}

static void AddDocid ( SqlStmt_t & tStmt, DocID_t & tDocId )
{
	int iDocidPos = tStmt.m_dInsertSchema.GetFirst ( [&] ( const CSphString & sName ) { return sName=="id"; } );
	if ( iDocidPos!=-1 )
	{
		SqlInsert_t & tVal = tStmt.m_dInsertValues[iDocidPos];
		// check and convert to int
		if ( tVal.m_iType!=SqlInsert_t::CONST_INT )
		{
			tVal.m_iVal = GetDocID ( tVal.m_sVal.cstr() );
			tVal.m_iType = SqlInsert_t::CONST_INT;
		}
		tDocId = tVal.m_iVal;
		return;
	}

	if ( !tDocId )
		return;

	tStmt.m_dInsertSchema.Add ( sphGetDocidName() );
	SqlInsert_t & tId = tStmt.m_dInsertValues.Add();
	tId.m_iType = SqlInsert_t::CONST_INT;
	tId.m_iVal = tDocId;

	tStmt.m_iSchemaSz = tStmt.m_dInsertSchema.GetLength();
}

static bool ParseSourceLine ( const char * sLine, const CSphString & sAction, SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError )
{
	// FIXME!!! update and delete ES compat endpoints
	if ( sAction=="index" )
	{
		JsonObj_c tRoot ( sLine );
		if ( !ParseJsonInsertSource ( tRoot, tStmt, true, true, sError ) )
			return false;
		AddDocid ( tStmt, tDocId );

	}  else if ( sAction=="create" )
	{
		JsonObj_c tRoot ( sLine );
		if ( !ParseJsonInsertSource ( tRoot, tStmt, false, true, sError ) )
			return false;
		AddDocid ( tStmt, tDocId );

	} else if ( sAction=="update" )
	{
		if ( !sphParseJsonUpdate ( FromSz ( sLine ), tStmt, tDocId, sError ) )
			return false;
	} else if ( sAction=="delete" )
	{
		if ( !sphParseJsonDelete ( FromSz ( sLine ), tStmt, tDocId, sError ) )
			return false;
	}

	return true;
}

char * SkipSpace ( char * p )
{
	while ( sphIsSpace ( *p ) )
		p++;

	return p;
}

static CSphString sphHttpEndpointToStr ( ESphHttpEndpoint eEndpoint )
{
	assert ( eEndpoint>=SPH_HTTP_ENDPOINT_INDEX && eEndpoint<SPH_HTTP_ENDPOINT_TOTAL );
	return g_dEndpoints[eEndpoint].m_szName1;
}

bool Ends ( const Str_t tVal, const char * sSuffix )
{
	if ( IsEmpty ( tVal ) || !sSuffix )
		return false;

	auto iSuffix = (int) strlen ( sSuffix );
	if ( tVal.second<iSuffix )
		return false;

	return strncmp ( tVal.first + tVal.second - iSuffix, sSuffix, iSuffix )==0;
}

static void ReportLogError ( const char * sError, const char * sErrorType , ESphHttpStatus eStatus, bool bLogOnly, const OptionsHash_t & hOpts, Str_t tReq, CSphVector<BYTE> & dResult )
{
	if ( !bLogOnly )
		ReportError ( sError, sErrorType, eStatus, dResult );

	for ( char * sCur = (char *)tReq.first; sCur<tReq.first+tReq.second; sCur++ )
	{
		if ( *sCur=='\0' )
			*sCur = '\n';
	}

	const CSphString * pUrl = hOpts ( "full_url" );
	sphWarning ( "%s\n%s\n%s", sError, ( pUrl ? pUrl->scstr() : "" ), tReq.first );
}

static bool Validate( const OptionsHash_t & hOpts, const Str_t & tSource, CSphVector<BYTE> & dReply )
{
	CSphString sError;

	// case insitive option get: content-type
	CSphString * pOptContentType = hOpts ( "Content-Type" );
	if ( !pOptContentType )
		pOptContentType = hOpts ( "content-type" );
	if ( !pOptContentType )
	{
		ReportLogError ( "Content-Type must be set", "illegal_argument_exception", SPH_HTTP_STATUS_400, false, hOpts, tSource, dReply );
		return false;
	}

	// HTTP field could have multiple values
	StrVec_t dOptContentType = sphSplit ( pOptContentType->cstr(), ",; " );
	if ( !dOptContentType.Contains ( "application/x-ndjson" ) && !dOptContentType.Contains ( "application/json" ) )
	{
		sError.SetSprintf ( "Content-Type header [%s] is not supported", pOptContentType->cstr() );
		ReportLogError ( sError.cstr(), "illegal_argument_exception", SPH_HTTP_STATUS_400, false, hOpts, tSource, dReply );
		return false;
	}

	if ( IsEmpty ( tSource ) )
	{
		ReportLogError ( "request body is required", "parse_exception", SPH_HTTP_STATUS_400, false, hOpts, tSource, dReply );
		return false;
	}
	if ( !Ends ( tSource, "\n" ) )
	{
		ReportLogError ( "The bulk request must be terminated by a newline [\n]", "illegal_argument_exception", SPH_HTTP_STATUS_400, false, hOpts, tSource, dReply );
		return true;
	}

	return true;
}

bool HttpHandlerEsBulk_c::Process()
{
	CSphString sError;
	if ( !Validate ( m_hOptions, m_tSource, m_dData ) )
		return false;

	auto & tCrashQuery = GlobalCrashQueryGetRef();
	tCrashQuery.m_dQuery = { (const BYTE *)m_tSource.first, m_tSource.second };

	CSphVector<Str_t> dLines;
	SplitNdJson ( m_tSource.first, m_tSource.second, [&] ( const char * sLine, int iLen ) { dLines.Add ( Str_t ( sLine, iLen ) ); } );
	
	CSphVector<BulkDoc_t> dDocs;
	dDocs.Reserve ( dLines.GetLength() / 2 );
	bool bMetaLine = true;
	for ( const Str_t & tLine : dLines )
	{
		if ( !bMetaLine )
		{
			dDocs.Last().m_tDocLine = tLine;
			bMetaLine = true;
		} else
		{
			// skip empty lines if they are meta information
			if ( IsEmpty ( tLine ) )
				continue;

			// any bad meta result in general error
			BulkDoc_t & tDoc = dDocs.Add();
			if ( !ParseMetaLine ( tLine.first, tDoc, sError ) )
			{
				ReportLogError ( sError.cstr(), "action_request_validation_exception", SPH_HTTP_STATUS_400, false, m_hOptions, m_tSource, m_dData );
				return false;
			}
			bMetaLine = false;
		}
	}
	CSphVector<BulkTnx_t> dTnx;
	const BulkDoc_t * pLastDoc = dDocs.Begin();
	for ( int i=1; i<dDocs.GetLength(); i++ )
	{
		const BulkDoc_t & tCurDoc = dDocs[i];
		if ( pLastDoc->m_sIndex==tCurDoc.m_sIndex && pLastDoc->m_sAction==tCurDoc.m_sAction )
			continue;

		BulkTnx_t & tTnx = dTnx.Add();
		tTnx.m_iFrom = pLastDoc - dDocs.Begin();
		tTnx.m_iCount = i - tTnx.m_iFrom;
	}
	if ( pLastDoc )
	{
		BulkTnx_t & tTnx = dTnx.Add();
		tTnx.m_iFrom = pLastDoc - dDocs.Begin();
		tTnx.m_iCount = dDocs.GetLength() - tTnx.m_iFrom;
	}

	JsonObj_c tItems ( true );
	bool bOk = ProcessTnx ( dTnx, dDocs, tItems );

	JsonObj_c tRoot;
	tRoot.AddItem ( "items", tItems );
	tRoot.AddBool ( "errors", !bOk );
	tRoot.AddInt ( "took", 1 ); // FIXME!!! add delta
	BuildReply ( tRoot.AsString(), ( bOk ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500 ) );

	if ( !bOk )
		ReportLogError ( "failed to commit", "", SPH_HTTP_STATUS_400, true, m_hOptions, m_tSource, m_dData );

	return true;

}

static const JsonObj_c g_tShards ( "{ \"total\": 1, \"successful\": 1, \"failed\": 0 }" );

static void AddEsReply ( const BulkDoc_t & tDoc, JsonObj_c & tRoot )
{
	char sBuf[70];
	snprintf ( sBuf, sizeof(sBuf), UINT64_FMT, (uint64_t)tDoc.m_tDocid );
	const char * sActionRes = "created";
	if ( tDoc.m_sAction=="delete" )
		sActionRes = "deleted";
	else if ( tDoc.m_sAction=="update" )
		sActionRes = "updated";
	JsonObj_c tShard ( g_tShards.Clone() );

	JsonObj_c tRes;
	tRes.AddStr ( "_index", tDoc.m_sIndex.cstr() );
	tRes.AddStr ( "_type", "doc" );
	tRes.AddStr ( "_id", sBuf );
	tRes.AddInt ( "_version", 1 );
	tRes.AddStr ( "result", sActionRes );
	tRes.AddItem ( "_shards", tShard );
	tRes.AddInt ( "_seq_no", 0 );
	tRes.AddInt ( "_primary_term", 1 );
	tRes.AddInt ( "status", 201 );

	JsonObj_c tAction;
	tAction.AddItem ( tDoc.m_sAction.cstr(), tRes );
	tRoot.AddItem ( tAction );
}

static void AddEsError ( int iReply, const CSphString & sError, const BulkDoc_t & tDoc, JsonObj_c & tRoot )
{
	char sBuf[70];
	snprintf ( sBuf, sizeof(sBuf), UINT64_FMT, (uint64_t)tDoc.m_tDocid );

	JsonObj_c tErrorObj;
	tErrorObj.AddStr ( "type", "mapper_parsing_exception" );
	tErrorObj.AddStr ( "reason", sError.cstr() );

	JsonObj_c tRes;
	tRes.AddStr ( "_index", tDoc.m_sIndex.cstr() );
	tRes.AddStr ( "_type", "doc" );
	tRes.AddStr ( "_id", sBuf );
	tRes.AddInt ( "status", 400 );
	tRes.AddItem ( "error", tErrorObj );

	JsonObj_c tAction;
	tAction.AddItem ( tDoc.m_sAction.cstr(), tRes );

	tRoot.ReplaceItem ( iReply, tAction );
}

bool HttpHandlerEsBulk_c::ProcessTnx ( const VecTraits_T<BulkTnx_t> & dTnx, VecTraits_T<BulkDoc_t> & dDocs, JsonObj_c & tItems )
{
	bool bOk = true;

	CSphVector<std::pair<int, CSphString>> dErrors;

	for ( const BulkTnx_t & tTnx : dTnx )
	{
		const CSphString & sIdx = dDocs[tTnx.m_iFrom].m_sIndex;
		assert ( !sIdx.IsEmpty() );
		ProcessBegin ( sIdx );

		bOk &= dErrors.IsEmpty();
		dErrors.Resize ( 0 );
		for ( int i = 0; i<tTnx.m_iCount; i++ )
		{
			int iDoc = tTnx.m_iFrom + i;
			BulkDoc_t & tDoc = dDocs[iDoc];

			if ( IsEmpty ( tDoc.m_tDocLine ) )
			{
				dErrors.Add ( { iDoc, "failed to parse, document is empty" } );
				continue;
			}

			SqlStmt_t tStmt;
			tStmt.m_tQuery.m_sIndexes = tDoc.m_sIndex;
			tStmt.m_sIndex = tDoc.m_sIndex;
			tStmt.m_sStmt = tDoc.m_tDocLine.first;
			CSphString sError;

			bool bParsed = ParseSourceLine ( tDoc.m_tDocLine.first, tDoc.m_sAction, tStmt, tDoc.m_tDocid, sError );
			if ( !bParsed )
			{
				dErrors.Add ( { iDoc, sError } );
				continue;
			}

			bool bAction = false;
			JsonObj_c tResult = JsonNull;

			switch ( tStmt.m_eStmt )
			{
			case STMT_INSERT:
			case STMT_REPLACE:
				bAction = ProcessInsert ( tStmt, tDoc.m_tDocid, tResult );
				break;

			case STMT_UPDATE:
				tStmt.m_sEndpoint = sphHttpEndpointToStr ( SPH_HTTP_ENDPOINT_JSON_UPDATE );
				bAction = ProcessUpdate ( tDoc.m_tDocLine, tStmt, tDoc.m_tDocid, tResult );
				break;

			case STMT_DELETE:
				tStmt.m_sEndpoint = sphHttpEndpointToStr ( SPH_HTTP_ENDPOINT_JSON_DELETE );
				bAction = ProcessDelete ( tDoc.m_tDocLine, tStmt, tDoc.m_tDocid, tResult );
				break;

			default:
				sphWarning ( "unknown statement \"%s\":%s", tStmt.m_sStmt, tDoc.m_tDocLine.first );
				break; // ignore statement as ES does
			}

			if ( !bAction )
				dErrors.Add ( { iDoc, tResult.GetItem ( "error" ).GetItem ( "type" ).StrVal() } );
		}

		// FIXME!!! check commit of empty accum
		JsonObj_c tResult;
		bool bCommited = ProcessCommitRollback ( FromStr ( sIdx ), DocID_t(), tResult );
		if ( bCommited )
		{
			for ( int i=0; i<tTnx.m_iCount; i++ )
				AddEsReply ( dDocs[tTnx.m_iFrom], tItems );
		} else
		{
			CSphString sError;
			for ( int i=0; i<tTnx.m_iCount; i++ )
			{
				JsonObj_c tTmp ( JsonNull );
				tItems.AddItem ( tTmp );
				assert ( tItems.Size()==tTnx.m_iFrom+i+1 );
				AddEsError ( tTnx.m_iFrom+i, tResult.GetStrItem ( "error", sError, false ).StrVal(), dDocs[tTnx.m_iFrom+i], tItems );
			}
		}

		for ( const auto & tErr : dErrors )
			AddEsError ( tErr.first, tErr.second, dDocs[tErr.first], tItems );
	}

	bOk &= dErrors.IsEmpty();
	session::SetInTrans ( false );
	return bOk;
}

void SplitNdJson ( const char * sBody, int iLen, SplitAction_fn && fnAction )
{
	const char * sBodyEnd = sBody + iLen;
	while ( sBody<sBodyEnd )
	{
		const char* sNext = sBody;
		// break on CR or LF
		while ( sNext<sBodyEnd && *sNext != '\r' && *sNext != '\n' )
			sNext++;

		if ( sNext==sBodyEnd )
			break;

		*(const_cast<char*>(sNext)) = '\0';
		fnAction ( sBody, sNext-sBody );
		sBody = sNext + 1;

		// skip new lines
		while ( sBody<sBodyEnd && *sBody == '\n' )
			sBody++;
	}
}
