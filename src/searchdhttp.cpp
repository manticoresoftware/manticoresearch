//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "jsonqueryfilter.h"
#include "attribute.h"
#include "sphinxpq.h"

#include "http/http_parser.h"
#include "searchdaemon.h"
#include "searchdha.h"
#include "searchdreplication.h"
#include "accumulator.h"

static const char * g_dHttpStatus[] = { "200 OK", "206 Partial Content", "400 Bad Request", "403 Forbidden", "500 Internal Server Error",
								 "501 Not Implemented", "503 Service Unavailable", "526 Invalid SSL Certificate" };

STATIC_ASSERT ( sizeof(g_dHttpStatus)/sizeof(g_dHttpStatus[0])==SPH_HTTP_STATUS_TOTAL, SPH_HTTP_STATUS_SHOULD_BE_SAME_AS_SPH_HTTP_STATUS_TOTAL );

extern CSphString g_sStatusVersion;

static void HttpBuildReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * sBody, int iBodyLen, bool bHtml )
{
	assert ( sBody && iBodyLen );

	const char * sContent = ( bHtml ? "text/html" : "application/json" );
	CSphString sHttp;
	sHttp.SetSprintf ( "HTTP/1.1 %s\r\nServer: %s\r\nContent-Type: %s; charset=UTF-8\r\nContent-Length: %d\r\n\r\n", g_dHttpStatus[eCode], g_sStatusVersion.cstr(), sContent, iBodyLen );

	int iHeaderLen = sHttp.Length();
	dData.Resize ( iHeaderLen + iBodyLen );
	memcpy ( dData.Begin(), sHttp.cstr(), iHeaderLen );
	memcpy ( dData.Begin() + iHeaderLen, sBody, iBodyLen );
}


static void HttpErrorReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * szError )
{
	JsonObj_c tErr;
	tErr.AddStr ( "error", szError );
	CSphString sJsonError = tErr.AsString();
	HttpBuildReply ( dData, eCode, sJsonError.cstr(), sJsonError.Length(), false );
}


using OptionsHash_t = SmallStringHash_T<CSphString>;

class HttpRequestParser_c : public ISphNoncopyable
{
public:
	bool					Parse ( const BYTE * pData, int iDataLen );
	bool					ParseList ( const char * sAt, int iLen );

	const CSphString &		GetBody() const { return m_sRawBody; }
	ESphHttpEndpoint		GetEndpoint() const { return m_eEndpoint; }
	const OptionsHash_t &	GetOptions() const { return m_hOptions; }
	const CSphString &		GetInvalidEndpoint() const { return m_sInvalidEndpoint; }
	const char *			GetError() const { return m_szError; }
	bool					GetKeepAlive() const { return m_bKeepAlive; }
	http_method				GetRequestType() const { return m_eType; }

	static int				ParserUrl ( http_parser * pParser, const char * sAt, size_t iLen );
	static int				ParserHeaderField ( http_parser * pParser, const char * sAt, size_t iLen );
	static int				ParserHeaderValue ( http_parser * pParser, const char * sAt, size_t iLen );
	static int				ParserBody ( http_parser * pParser, const char * sAt, size_t iLen );

private:
	bool					m_bKeepAlive {false};
	const char *			m_szError {nullptr};
	ESphHttpEndpoint		m_eEndpoint {SPH_HTTP_ENDPOINT_TOTAL};
	CSphString				m_sInvalidEndpoint;
	CSphString				m_sRawBody;
	CSphString				m_sCurField;
	OptionsHash_t			m_hOptions;
	CSphString				m_sEndpoint;
	http_method				m_eType = HTTP_GET;
};


bool HttpRequestParser_c::Parse ( const BYTE * pData, int iDataLen )
{
	http_parser_settings tParserSettings;
	http_parser_settings_init ( &tParserSettings );
	tParserSettings.on_url = HttpRequestParser_c::ParserUrl;
	tParserSettings.on_header_field = HttpRequestParser_c::ParserHeaderField;
	tParserSettings.on_header_value = HttpRequestParser_c::ParserHeaderValue;
	tParserSettings.on_body = HttpRequestParser_c::ParserBody;

	http_parser tParser;
	tParser.data = this;
	http_parser_init ( &tParser, HTTP_REQUEST );
	auto iParsed = (int) http_parser_execute ( &tParser, &tParserSettings, (const char *)pData, iDataLen );
	if ( iParsed!=iDataLen )
	{
		m_szError = http_errno_description ( (http_errno)tParser.http_errno );
		return false;
	}

	// connection wide http options
	m_bKeepAlive = ( http_should_keep_alive ( &tParser )!=0 );
	// transfer endpoint for further parse
	m_hOptions.Add ( m_sEndpoint, "endpoint" );
	m_eType = (http_method)tParser.method;

	return true;
}

static int Char2Hex ( BYTE uChar )
{
	if ( uChar>=0x41 && uChar<=0x46 )
		return ( uChar - 'A' ) + 10;
	else if ( uChar>=0x61 && uChar<=0x66 )
		return ( uChar - 'a' ) + 10;
	else if ( uChar>='0' && uChar<='9' )
		return uChar - '0';
	return -1;
}

static void UriPercentReplace ( CSphString & sEntity, bool bAlsoPlus=true )
{
	if ( sEntity.IsEmpty() )
		return;

	char * pDst = const_cast<char *> ( sEntity.cstr() );
	const char * pSrc = pDst;
	char cPlus = bAlsoPlus ? ' ' : '+';
	while ( *pSrc )
	{
		if ( *pSrc=='%' && *(pSrc+1) && *(pSrc+2) )
		{
			auto iCode = Char2Hex ( *(pSrc+1) ) * 16 + Char2Hex ( *(pSrc+2) );
			if ( iCode<0 )
			{
				*pDst++ = *pSrc++;
				continue;
			}
			pSrc += 3;
			*pDst++ = (BYTE) iCode;
		} else
		{
			*pDst++ = ( *pSrc=='+' ? cPlus : *pSrc );
			pSrc++;
		}
	}
	*pDst = '\0';
}


bool HttpRequestParser_c::ParseList ( const char * sAt, int iLen )
{
	const char * sCur = sAt;
	const char * sEnd = sAt + iLen;
	const char * sLast = sCur;
	CSphString sName;
	CSphString sVal;
	const char * sRawData = nullptr;
	CSphString sRawName;
	for ( ; sCur<sEnd; sCur++ )
	{
		if ( *sCur!='&' && *sCur!='=' )
			continue;

		int iValueLen = int ( sCur - sLast );
		if ( *sCur=='&' )
		{
			sVal.SetBinary ( sLast, iValueLen );
			UriPercentReplace ( sName );
			UriPercentReplace ( sVal );
			m_hOptions.Add ( sVal, sName );
			sName = "";
			sVal = "";
		} else
		{
			sName.SetBinary ( sLast, iValueLen );
			if ( !sRawData )
			{
				sRawData = sCur + 1;
				sRawName = sName;
			}
		}
		sLast = sCur+1;
	}

	if ( !sName.IsEmpty() )
	{
		sVal.SetBinary ( sLast, int ( sCur - sLast ) );
		UriPercentReplace ( sName );
		UriPercentReplace ( sVal );
		m_hOptions.Add ( sVal, sName );
	}

	if ( sRawData )
	{
		sVal.SetBinary ( sRawData, int ( sCur - sRawData ) );
		sName.SetSprintf ( "decoded_%s", sRawName.cstr() );
		UriPercentReplace ( sName );
		UriPercentReplace ( sVal, false ); // avoid +-decoding
		m_hOptions.Add ( sVal, sName );
	}

	return true;
}


int HttpRequestParser_c::ParserUrl ( http_parser * pParser, const char * sAt, size_t iLen )
{
	http_parser_url tUri;
	if ( http_parser_parse_url ( sAt, iLen, 0, &tUri ) )
		return 0;

	DWORD uPath = ( 1UL<<UF_PATH );
	DWORD uQuery = ( 1UL<<UF_QUERY );

	if ( ( tUri.field_set & uPath )!=0 )
	{
		const char * sPath = sAt + tUri.field_data[UF_PATH].off;
		int iPathLen = tUri.field_data[UF_PATH].len;
		if ( *sPath=='/' )
		{
			sPath++;
			iPathLen--;
		}

		// URL should be split fully to point to proper endpoint 
		CSphString & sEndpoint = ( (HttpRequestParser_c *)pParser->data )->m_sEndpoint;
		sEndpoint.SetBinary ( sPath, iPathLen );
		ESphHttpEndpoint eEndpoint = sphStrToHttpEndpoint ( sEndpoint );
		( (HttpRequestParser_c *)pParser->data )->m_eEndpoint = eEndpoint;
		if ( eEndpoint==SPH_HTTP_ENDPOINT_TOTAL )
			( (HttpRequestParser_c *)pParser->data )->m_sInvalidEndpoint.SetBinary ( sPath, iPathLen );
	}

	if ( ( tUri.field_set & uQuery )!=0 )
		( (HttpRequestParser_c *)pParser->data )->ParseList ( sAt + tUri.field_data[UF_QUERY].off, tUri.field_data[UF_QUERY].len );

	return 0;
}

int HttpRequestParser_c::ParserHeaderField ( http_parser * pParser, const char * sAt, size_t iLen )
{
	assert ( pParser->data );
	( (HttpRequestParser_c *)pParser->data )->m_sCurField.SetBinary ( sAt, (int) iLen );
	return 0;
}

int HttpRequestParser_c::ParserHeaderValue ( http_parser * pParser, const char * sAt, size_t iLen )
{
	assert ( pParser->data );
	CSphString sVal;
	sVal.SetBinary ( sAt, (int) iLen );
	auto * pHttpParser = (HttpRequestParser_c *)pParser->data;
	pHttpParser->m_hOptions.Add ( sVal, pHttpParser->m_sCurField );
	pHttpParser->m_sCurField = "";
	return 0;
}

int HttpRequestParser_c::ParserBody ( http_parser * pParser, const char * sAt, size_t iLen )
{
	assert ( pParser->data );
	auto * pHttpParser = (HttpRequestParser_c *)pParser->data;
	pHttpParser->ParseList ( sAt, (int) iLen );
	pHttpParser->m_sRawBody.SetBinary ( sAt, (int) iLen );
	return 0;
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
	HttpBuildReply ( dData, SPH_HTTP_STATUS_200, sIndexPage.cstr(), sIndexPage.GetLength(), true );
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
		ESphHttpEndpoint eEndpoint = sphStrToHttpEndpoint ( sEndpoint );
		if ( eEndpoint!=SPH_HTTP_ENDPOINT_JSON_UPDATE && eEndpoint!=SPH_HTTP_ENDPOINT_JSON_DELETE )
			return false;

		DWORD uLength = tReq.GetDword();
		CSphFixedVector<BYTE> dResult ( uLength+1 );
		tReq.GetBytes ( dResult.Begin(), uLength );
		dResult[uLength] = '\0';

		return sphGetResultStats ( (const char *)dResult.Begin(), m_iAffected, m_iWarnings, eEndpoint==SPH_HTTP_ENDPOINT_JSON_UPDATE );
	}

protected:
	int &	m_iAffected;
	int &	m_iWarnings;
};

QueryParser_i * CreateQueryParser( bool bJson )
{
	if ( bJson )
		return sphCreateJsonQueryParser();
	else
		return sphCreatePlainQueryParser();
}

RequestBuilder_i * CreateRequestBuilder ( Str_t sQuery, const SqlStmt_t & tStmt )
{
	if ( tStmt.m_bJson )
	{
		assert ( !tStmt.m_sEndpoint.IsEmpty() );
		return new JsonRequestBuilder_c ( sQuery.first, tStmt.m_sEndpoint );
	} else
	{
		return new SphinxqlRequestBuilder_c ( sQuery, tStmt );
	}
}

ReplyParser_i * CreateReplyParser ( bool bJson, int & iUpdated, int & iWarnings )
{
	if ( bJson )
		return new JsonReplyParser_c ( iUpdated, iWarnings );
	else
		return new SphinxqlReplyParser_c ( &iUpdated, &iWarnings );
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

//////////////////////////////////////////////////////////////////////////
// all the handlers for http queries

class HttpHandler_c
{
public:
	HttpHandler_c ( const char * sQuery )
		: m_sQuery ( sQuery )
	{}

	virtual ~HttpHandler_c()
	{}

	virtual bool Process () = 0;

	void SetErrorFormat ( bool bNeedHttpResponse )
	{
		m_bNeedHttpResponse = bNeedHttpResponse;
	}
	
	CSphVector<BYTE> & GetResult()
	{
		return m_dData;
	}

protected:
	const char *		m_sQuery;
	bool				m_bNeedHttpResponse {false};
	CSphVector<BYTE>	m_dData;

	void ReportError ( const char * szError, ESphHttpStatus eStatus )
	{
		if ( m_bNeedHttpResponse )
			HttpErrorReply ( m_dData, eStatus, szError );
		else
		{
			m_dData.Resize ( (int) strlen ( szError ) );
			memcpy ( m_dData.Begin(), szError, m_dData.GetLength() );
		}
	}

	void FormatError ( ESphHttpStatus eStatus, const char * sError, ... )
	{
		char sBuf[1024] = {0};
		va_list ap;

		va_start ( ap, sError );
		int iPrinted = vsnprintf ( sBuf, sizeof(sBuf), sError, ap );
		va_end ( ap );

		iPrinted = Max ( 0, iPrinted );
		iPrinted = Min ( iPrinted, (int)sizeof(sBuf)-1 );
		sBuf[iPrinted] = '\0';

		if ( m_bNeedHttpResponse )
			HttpErrorReply ( m_dData, eStatus, sBuf );
		else
		{
			m_dData.Resize ( iPrinted );
			memcpy ( m_dData.Begin(), sBuf, iPrinted );
		}
	}

	void BuildReply ( const CSphString & sResult, ESphHttpStatus eStatus )
	{
		BuildReply ( sResult.cstr(), sResult.Length(), eStatus );
	}

	void BuildReply ( const StringBuilder_c & sResult, ESphHttpStatus eStatus )
	{
		BuildReply ( sResult.cstr(), sResult.GetLength(), eStatus );
	}

	void BuildReply ( const char * sResult, int iLen, ESphHttpStatus eStatus )
	{
		if ( m_bNeedHttpResponse )
			HttpBuildReply ( m_dData, eStatus, sResult, iLen, false );
		else
		{
			m_dData.Resize ( iLen );
			memcpy ( m_dData.Begin(), sResult, m_dData.GetLength() );
		}
	}

	// check whether given served index is exist and has requested type
	bool CheckValid ( const ServedDesc_t* pServed, const CSphString& sIndex, IndexType_e eType )
	{
		if ( !pServed )
		{
			FormatError ( SPH_HTTP_STATUS_500, "no such index '%s'", sIndex.cstr () );
			return false;
		}
		if ( pServed->m_eType!=eType || !pServed->m_pIndex )
		{
			FormatError ( SPH_HTTP_STATUS_500, "index '%s' is not %s (enabled=%d)", sIndex.cstr(), GetTypeName ( eType ).cstr(), (int)(!!pServed->m_pIndex) );
			return false;
		}
		return true;
	}
};


class HttpOptionsTraits_c
{
protected:
	const OptionsHash_t & m_tOptions;

	HttpOptionsTraits_c ( const OptionsHash_t & tOptions )
		: m_tOptions ( tOptions )
	{}
};

static PubSearchHandler_c * CreateMsearchHandler ( const QueryParser_i * pQueryParser, QueryType_e eQueryType, JsonQuery_c & tQuery )
{
	tQuery.m_pQueryParser = pQueryParser;

	int iQueries = ( 1 + tQuery.m_dAggs.GetLength() );
	PubSearchHandler_c * pHandler = { new PubSearchHandler_c ( iQueries, pQueryParser, eQueryType, true ) };

	if ( !tQuery.m_dAggs.GetLength() )
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
	}

	tQuery.m_bFacetHead = true;
	pHandler->SetQuery ( 0, tQuery, nullptr );
	int iRefLimit = tQuery.m_iLimit;

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

		tQuery.m_sGroupBy = tBucket.m_sCol;
		tQuery.m_sFacetBy = tBucket.m_sCol;
		tQuery.m_sGroupSortBy = "@groupby desc";
		tQuery.m_sOrderBy = "@weight desc";

		// aggregate and main query could have different sizes
		tQuery.m_iLimit = ( tBucket.m_iSize ? tBucket.m_iSize : iRefLimit );

		pHandler->SetQuery ( i+1, tQuery, nullptr );
	}

	return pHandler;
}


class HttpSearchHandler_c : public HttpHandler_c, public HttpOptionsTraits_c
{
public:
	HttpSearchHandler_c ( const char * sQuery, const OptionsHash_t & tOptions )
		: HttpHandler_c ( sQuery )
		, HttpOptionsTraits_c ( tOptions )
	{}

	bool Process () final
	{
		CSphString sWarning;
		QueryParser_i * pQueryParser = PreParseQuery();
		if ( !pQueryParser )
			return false;

		int iQueries = ( 1 + m_tQuery.m_dAggs.GetLength() );

		CSphScopedPtr<PubSearchHandler_c> tHandler { CreateMsearchHandler ( pQueryParser, m_eQueryType, m_tQuery )};
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
		if ( pRes->m_sWarning.IsEmpty() )
			pRes->m_sWarning = m_sWarning;

		CSphFixedVector<AggrResult_t *> dAggsRes ( iQueries );
		dAggsRes[0] = tHandler->GetResult ( 0 );
		ARRAY_FOREACH ( i, m_tQuery.m_dAggs )
			dAggsRes[i+1] = tHandler->GetResult ( i+1 );

		CSphString sResult = EncodeResult ( dAggsRes, m_bProfile ? &tProfile : NULL );
		BuildReply ( sResult, SPH_HTTP_STATUS_200 );

		return true;
	}

protected:
	bool					m_bProfile {false};
	QueryType_e				m_eQueryType {QUERY_SQL};
	JsonQuery_c				m_tQuery;
	CSphString				m_sWarning;

	virtual QueryParser_i * PreParseQuery() = 0;
	virtual CSphString		EncodeResult ( const VecTraits_T<AggrResult_t *> & dRes, QueryProfile_c * pProfile ) = 0;
	virtual void			SetStmt ( PubSearchHandler_c & tHandler ) {};
};


class HttpSearchHandler_SQL_c : public HttpSearchHandler_c
{
public:
	HttpSearchHandler_SQL_c ( const char * sQuery, const OptionsHash_t & tOptions )
		: HttpSearchHandler_c ( sQuery, tOptions )
	{}

protected:
	CSphVector<SqlStmt_t> m_dStmt;

	QueryParser_i * PreParseQuery() override
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
		if ( m_dStmt.GetLength()>1 )
		{
			ReportError ( "multiple queries not supported", SPH_HTTP_STATUS_501 );
			return nullptr;
		} else if ( m_dStmt[0].m_eStmt!=STMT_SELECT )
		{
			ReportError ( "only SELECT queries are supported", SPH_HTTP_STATUS_501 );
			return nullptr;
		}

		m_eQueryType = QUERY_SQL;

		return sphCreatePlainQueryParser();
	}

	CSphString EncodeResult ( const VecTraits_T<AggrResult_t *> & dRes, QueryProfile_c * pProfile ) override
	{
		return sphEncodeResultJson ( dRes, m_tQuery, pProfile );
	}

	void SetStmt ( PubSearchHandler_c & tHandler ) override
	{
		tHandler.SetStmt ( m_dStmt[0] );
	}
};

typedef std::pair<CSphString,MysqlColumnType_e> ColumnNameType_t;

static const char * GetTypeName ( MysqlColumnType_e eType )
{
	switch ( eType )
	{
		case MYSQL_COL_DECIMAL: return "decimal";
		case MYSQL_COL_LONG: return "long";
		case MYSQL_COL_FLOAT: return "float";
		case MYSQL_COL_LONGLONG: return "long long";
		case MYSQL_COL_STRING: return "string";
		default: return "unknown";
	};
}

class JsonRowBuffer_c : public RowBuffer_i
{
public:
	JsonRowBuffer_c () {}

	void PutFloatAsString ( float fVal, const char * sFormat ) override
	{
		AddDataColumn();
		m_dBuf.Appendf( "%f", fVal );
	}

	void PutNumAsString ( int64_t iVal ) override
	{
		AddDataColumn();
		m_dBuf.Appendf( INT64_FMT, iVal );
	}

	void PutNumAsString ( uint64_t uVal ) override
	{
		AddDataColumn();
		m_dBuf.Appendf( UINT64_FMT, uVal );
	}

	void PutNumAsString ( int iVal ) override
	{
		AddDataColumn();
		m_dBuf.Appendf ( "%d", iVal );
	}
	
	void PutNumAsString ( DWORD uVal ) override
	{
		AddDataColumn();
		m_dBuf.Appendf ( "%u", uVal );
	}

	void PutArray ( const void * pBlob, int iLen, bool bSendEmpty ) override
	{
		AddDataColumn();
		m_dBuf.FixupSpacedAndAppendEscaped ( (const char*) pBlob, iLen );
	}

	void PutString ( const char * sMsg, int iLen=-1 ) override
	{
		AddDataColumn();
		if ( iLen<0 )
			m_dBuf.FixupSpacedAndAppendEscaped ( sMsg );
		else
			m_dBuf.FixupSpacedAndAppendEscaped ( sMsg, iLen );
	}

	void PutMicrosec ( int64_t iUsec ) override
	{
		PutNumAsString ( iUsec );
	}

	void PutNULL() override
	{
		AddDataColumn();
		m_dBuf += "null";
	}

	bool Commit() override
	{
		m_dBuf += "}";
		m_iCol = 0;
		m_iRow++;
		return true;
	}

	void Eof ( bool bMoreResults , int iWarns ) override
	{
		m_dBuf += "\n],\n";
	}

	void Error ( const char *, const char * sError, MysqlErrors_e iErr ) override
	{
		m_sError = sError;
	}

	void Ok ( int iAffectedRows, int iWarns, const char * sMessage, bool bMoreResults, int64_t iLastInsertId ) override
	{
		m_iTotalRows = iAffectedRows;
		m_sWarning = sMessage;
	}

	void HeadBegin ( int ) override
	{
		m_dBuf.Clear();
		m_dBuf += "{";
	}

	bool HeadEnd ( bool , int ) override
	{
		{
			ScopedComma_c tComma ( m_dBuf, ",", R"("columns":[)", "],\n", false );
			for ( const ColumnNameType_t & tCol : m_dColumns )
			{
				ScopedComma_c tColBlock ( m_dBuf.Object(true) );
				m_dBuf.AppendName ( tCol.first.cstr() );
				ScopedComma_c tTypeBlock ( m_dBuf.Object(true) );
				m_dBuf.AppendName ( "type" );
				m_dBuf.FixupSpacedAndAppendEscaped ( GetTypeName ( tCol.second ) );
			}
		}
		
		m_dBuf.AppendName ( "data" );
		m_dBuf += "[\n";
		return true;
	}

	void HeadColumn ( const char * sName, MysqlColumnType_e eType ) override
	{
		m_dColumns.Add ( ColumnNameType_t { sName, eType } );
	}

	void Add ( BYTE ) override {}

	const JsonEscapedBuilder & Finish()
	{
		if ( m_dBuf.IsEmpty() )
			m_dBuf += "{\n";

		m_dBuf.Appendf ( R"("total":%d)", m_iTotalRows );

		m_dBuf += ",\n";
		m_dBuf += R"("error":)";
		m_dBuf.AppendEscapedWithComma ( m_sError.cstr () );

		m_dBuf += ",\n";
		m_dBuf += R"("warning":)";
		m_dBuf.AppendEscapedWithComma ( m_sWarning.cstr () );

		m_dBuf += "\n}";

		return m_dBuf;
	}

private:
	JsonEscapedBuilder m_dBuf;
	CSphVector<ColumnNameType_t> m_dColumns;
	int m_iCol = 0;
	int m_iRow = 0;

	CSphString m_sError;
	CSphString m_sWarning;
	int m_iTotalRows = 0;

	void AddDataColumn()
	{
		if ( !m_iCol && m_iRow )
			m_dBuf += ",\n";
		if ( !m_iCol )
			m_dBuf += "{";
		if ( m_iCol )
			m_dBuf += ", ";

		m_dBuf.AppendName ( m_dColumns[m_iCol].first.cstr() );
		m_iCol++;
	}
};

static const char g_sBypassToken[] = "raw&query=";

class HttpRawSqlHandler_c : public HttpHandler_c, public HttpOptionsTraits_c
{
public:
	HttpRawSqlHandler_c ( const char * sQuery, const OptionsHash_t & tOptions )
		: HttpHandler_c ( sQuery )
		, HttpOptionsTraits_c ( tOptions )
	{
	}

	bool Process () override
	{
		CSphString sQuery = m_sQuery + sizeof ( g_sBypassToken ) - 1;
		CSphString sError;

		JsonRowBuffer_c tOut;

		SphinxqlSessionPublic tSession;
		tSession.Execute ( FromStr ( sQuery ), tOut );

		BuildReply ( tOut.Finish(), SPH_HTTP_STATUS_200 );

		return true;
	}
};


class HttpHandler_JsonSearch_c : public HttpSearchHandler_c
{
public:	
	HttpHandler_JsonSearch_c ( const char * sQuery, const OptionsHash_t & tOptions )
		: HttpSearchHandler_c ( sQuery, tOptions )
	{}

	QueryParser_i * PreParseQuery() override
	{
		CSphString sError;
		if ( !sphParseJsonQuery ( m_sQuery, m_tQuery, m_bProfile, sError, m_sWarning ) )
		{
			sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return NULL;
		}

		m_eQueryType = QUERY_JSON;

		return sphCreateJsonQueryParser();
	}

protected:
	CSphString EncodeResult ( const VecTraits_T<AggrResult_t *> & dRes, QueryProfile_c * pProfile ) override
	{
		return sphEncodeResultJson ( dRes, m_tQuery, pProfile );
	}
};


class HttpJsonInsertTraits_c
{
protected:
	bool ProcessInsert ( SqlStmt_t & tStmt, DocID_t tDocId, bool bReplace, JsonObj_c & tResult )
	{
		CSphSessionAccum tAcc;
		CSphString sWarning;
		HttpErrorReporter_c tReporter;
		CSphVector<int64_t> dLastIds;
		sphHandleMysqlInsert ( tReporter, tStmt, bReplace, true, sWarning, tAcc, dLastIds );

		if ( tReporter.IsError() )
		{
			tResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
		} else
		{
			if ( dLastIds.GetLength() )
				tDocId = dLastIds[0];
			tResult = sphEncodeInsertResultJson ( tStmt.m_sIndex.cstr(), bReplace, tDocId );
		}

		return !tReporter.IsError();
	}
};


class HttpHandler_JsonInsert_c : public HttpHandler_c, public HttpJsonInsertTraits_c
{
public:
	HttpHandler_JsonInsert_c ( const char * sQuery, bool bReplace )
		: HttpHandler_c ( sQuery )
		, m_bReplace ( bReplace )
	{}

	bool Process () override
	{
		SqlStmt_t tStmt;
		DocID_t tDocId = 0;
		CSphString sError;
		if ( !sphParseJsonInsert ( m_sQuery, tStmt, tDocId, m_bReplace, sError ) )
		{
			sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return false;
		}

		tStmt.m_sEndpoint = sphHttpEndpointToStr ( m_bReplace ? SPH_HTTP_ENDPOINT_JSON_REPLACE : SPH_HTTP_ENDPOINT_JSON_INSERT );
		JsonObj_c tResult = JsonNull;
		bool bResult = ProcessInsert ( tStmt, tDocId, m_bReplace, tResult );

		CSphString sResult = tResult.AsString();
		BuildReply ( sResult, bResult ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500 );

		return bResult;
	}

private:
	bool m_bReplace {false};
};


class HttpJsonUpdateTraits_c
{
protected:
	bool ProcessUpdate ( const char * szRawRequest, const SqlStmt_t & tStmt, DocID_t tDocId, JsonObj_c & tResult )
	{
		HttpErrorReporter_c tReporter;
		CSphString sWarning;
		sphHandleMysqlUpdate ( tReporter, tStmt, FromSz ( szRawRequest ), sWarning );

		if ( tReporter.IsError() )
			tResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
		else
			tResult = sphEncodeUpdateResultJson ( tStmt.m_sIndex.cstr(), tDocId, tReporter.GetAffectedRows() );

		return !tReporter.IsError();
	}
};


class HttpHandler_JsonUpdate_c : public HttpHandler_c, HttpJsonUpdateTraits_c
{
public:
	HttpHandler_JsonUpdate_c ( const char * sQuery )
		: HttpHandler_c ( sQuery )
	{}

	bool Process () override
	{
		SqlStmt_t tStmt;
		tStmt.m_bJson = true;
		tStmt.m_sEndpoint = sphHttpEndpointToStr ( SPH_HTTP_ENDPOINT_JSON_UPDATE );

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


class HttpJsonDeleteTraits_c
{
protected:
	bool ProcessDelete ( const char * szRawRequest, const SqlStmt_t & tStmt, DocID_t tDocId, JsonObj_c & tResult )
	{
		CSphSessionAccum tAcc;
		HttpErrorReporter_c tReporter;
		CSphString sWarning;
		sphHandleMysqlDelete ( tReporter, tStmt, FromSz ( szRawRequest ), true, tAcc );

		if ( tReporter.IsError() )
			tResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
		else
			tResult = sphEncodeDeleteResultJson ( tStmt.m_sIndex.cstr(), tDocId, tReporter.GetAffectedRows() );

		return !tReporter.IsError();
	}
};


class HttpHandler_JsonDelete_c : public HttpHandler_JsonUpdate_c, public HttpJsonDeleteTraits_c
{
public:
	HttpHandler_JsonDelete_c ( const char * sQuery )
		: HttpHandler_JsonUpdate_c ( sQuery )
	{}

protected:
	bool ParseQuery ( SqlStmt_t & tStmt, DocID_t & tDocId, CSphString & sError ) override
	{
		tStmt.m_sEndpoint = sphHttpEndpointToStr ( SPH_HTTP_ENDPOINT_JSON_DELETE );
		return sphParseJsonDelete ( m_sQuery, tStmt, tDocId, sError );
	}

	bool ProcessQuery ( const SqlStmt_t & tStmt, DocID_t tDocId, JsonObj_c & tResult ) override
	{
		return ProcessDelete ( m_sQuery, tStmt, tDocId, tResult );
	}
};


class HttpHandler_JsonBulk_c : public HttpHandler_c, public HttpOptionsTraits_c, public HttpJsonInsertTraits_c, public HttpJsonUpdateTraits_c, public HttpJsonDeleteTraits_c
{
public:
	HttpHandler_JsonBulk_c ( const char * sQuery, const OptionsHash_t & tOptions )
		: HttpHandler_c ( sQuery )
		, HttpOptionsTraits_c ( tOptions )
	{}

	bool Process () override
	{
		if ( !m_tOptions.Exists ("Content-Type") )
		{
			ReportError ( "Content-Type must be set", SPH_HTTP_STATUS_400 );
			return false;
		}

		if ( m_tOptions["Content-Type"].ToLower() != "application/x-ndjson" )
		{
			ReportError ( "Content-Type must be application/x-ndjson", SPH_HTTP_STATUS_400 );
			return false;
		}

		JsonObj_c tRoot;
		JsonObj_c tItems(true);

		// fixme: we're modifying the original query at this point
		char * p = const_cast<char*>(m_sQuery);

		bool bResult = false;
		while ( p && *p )
		{
			while ( sphIsSpace(*p) )
				p++;

			char * szStmt = p;
			while ( *p && *p!='\r' && *p!='\n' )
				p++;

			if ( p-szStmt==0 )
				break;

			*p++ = '\0';
			SqlStmt_t tStmt;
			tStmt.m_bJson = true;
			DocID_t tDocId = 0;
			CSphString sStmt;
			CSphString sError;
			CSphString sQuery;
			if ( !sphParseJsonStatement ( szStmt, tStmt, sStmt, sQuery, tDocId, sError ) )
			{
				sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
				ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
				return false;
			}

			JsonObj_c tResult = JsonNull;
			bResult = false;

			switch ( tStmt.m_eStmt )
			{
			case STMT_INSERT:
			case STMT_REPLACE:
				bResult = ProcessInsert ( tStmt, tDocId, tStmt.m_eStmt==STMT_REPLACE, tResult );
				break;

			case STMT_UPDATE:
				tStmt.m_sEndpoint = sphHttpEndpointToStr ( SPH_HTTP_ENDPOINT_JSON_UPDATE );
				bResult = ProcessUpdate ( sQuery.cstr(), tStmt, tDocId, tResult );
				break;

			case STMT_DELETE:
				tStmt.m_sEndpoint = sphHttpEndpointToStr ( SPH_HTTP_ENDPOINT_JSON_DELETE );
				bResult = ProcessDelete ( sQuery.cstr(), tStmt, tDocId, tResult );
				break;

			default:
				ReportError ( "Unknown statement", SPH_HTTP_STATUS_400 );
				return false;
			}

			AddResult ( tItems, sStmt, tResult );

			// no further than the first error
			if ( !bResult )
				break;

			while ( sphIsSpace(*p) )
				p++;
		}

		tRoot.AddItem ( "items", tItems );
		tRoot.AddBool ( "errors", !bResult );
		BuildReply ( tRoot.AsString(), bResult ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500 );

		return true;
	}

private:
	void AddResult ( JsonObj_c & tRoot, CSphString & sStmt, JsonObj_c & tResult )
	{
		JsonObj_c tItem;
		tItem.AddItem ( sStmt.cstr(), tResult );
		tRoot.AddItem ( tItem );
	}
};

class HttpHandlerPQ_c : public HttpHandler_c
{
public:	
	HttpHandlerPQ_c (  const char * sQuery, const OptionsHash_t& tOptions )
		: HttpHandler_c ( sQuery )
		, m_tOptions ( tOptions )
	{}

	bool Process () final;

private:
	const OptionsHash_t & m_tOptions;

	// FIXME!!! handle replication for InsertOrReplaceQuery and Delete
	bool DoCallPQ ( const CSphString & sIndex, const JsonObj_c & tPercolate, bool bVerbose );
	bool InsertOrReplaceQuery ( const CSphString& sIndex, const JsonObj_c& tJsonQuery, const JsonObj_c& tRoot,
		CSphString* pUID, bool bReplace );
	bool ListQueries ( const CSphString & sIndex );
	bool Delete ( const CSphString & sIndex, const JsonObj_c & tRoot );
};


static HttpHandler_c * CreateHttpHandler ( ESphHttpEndpoint eEndpoint, const char * sQuery, const OptionsHash_t & tOptions, http_method eRequestType )
{
	const CSphString * pRawSQL = nullptr;

	switch ( eEndpoint )
	{
	case SPH_HTTP_ENDPOINT_SQL:
		pRawSQL = tOptions ( "decoded_mode" );
		if ( pRawSQL && pRawSQL->Begins ( g_sBypassToken ) ) // test for 'raw&query=...'
		{
			if (!sQuery)
				sQuery = pRawSQL->cstr ();
			else
				sQuery += sizeof ( "mode=" )-1;
			return new HttpRawSqlHandler_c ( sQuery, tOptions );
		}
		else
			return new HttpSearchHandler_SQL_c ( sQuery, tOptions );

	case SPH_HTTP_ENDPOINT_JSON_SEARCH:
		return new HttpHandler_JsonSearch_c ( sQuery, tOptions );

	case SPH_HTTP_ENDPOINT_JSON_INDEX:
	case SPH_HTTP_ENDPOINT_JSON_CREATE:
	case SPH_HTTP_ENDPOINT_JSON_INSERT:
	case SPH_HTTP_ENDPOINT_JSON_REPLACE:
		return new HttpHandler_JsonInsert_c ( sQuery, eEndpoint==SPH_HTTP_ENDPOINT_JSON_INDEX || eEndpoint==SPH_HTTP_ENDPOINT_JSON_REPLACE );

	case SPH_HTTP_ENDPOINT_JSON_UPDATE:
		return new HttpHandler_JsonUpdate_c ( sQuery );

	case SPH_HTTP_ENDPOINT_JSON_DELETE:
		return new HttpHandler_JsonDelete_c ( sQuery );

	case SPH_HTTP_ENDPOINT_JSON_BULK:
		return new HttpHandler_JsonBulk_c ( sQuery, tOptions );

	case SPH_HTTP_ENDPOINT_PQ:
		return new HttpHandlerPQ_c ( sQuery, tOptions );

	default:
		break;
	}

	return nullptr;
}


static bool sphProcessHttpQuery ( ESphHttpEndpoint eEndpoint, const char * sQuery, const SmallStringHash_T<CSphString> & tOptions, CSphVector<BYTE> & dResult, bool bNeedHttpResponse, http_method eRequestType )
{
	CSphScopedPtr<HttpHandler_c> pHandler ( CreateHttpHandler ( eEndpoint, sQuery, tOptions, eRequestType ) );
	if ( !pHandler )
		return false;

	pHandler->SetErrorFormat ( bNeedHttpResponse );

	pHandler->Process();
	dResult = std::move ( pHandler->GetResult() );
	return true;
}


bool sphProcessHttpQueryNoResponce ( ESphHttpEndpoint eEndpoint, const char * sQuery, const SmallStringHash_T<CSphString> & tOptions, CSphVector<BYTE> & dResult )
{
	return sphProcessHttpQuery ( eEndpoint, sQuery, tOptions, dResult, false, HTTP_GET );
}


bool sphLoopClientHttp ( const BYTE * pRequest, int iRequestLen, CSphVector<BYTE> & dResult )
{
	HttpRequestParser_c tParser;
	if ( !tParser.Parse ( pRequest, iRequestLen ) )
	{
		HttpErrorReply ( dResult, SPH_HTTP_STATUS_400, tParser.GetError() );
		return tParser.GetKeepAlive();
	}

	ESphHttpEndpoint eEndpoint = tParser.GetEndpoint();
	if ( !sphProcessHttpQuery ( eEndpoint, tParser.GetBody().cstr(), tParser.GetOptions(), dResult, true, tParser.GetRequestType() ) )
	{
		if ( eEndpoint==SPH_HTTP_ENDPOINT_INDEX )
			HttpHandlerIndexPage ( dResult );
		else
		{
			CSphString sError;
			sError.SetSprintf ( "/%s - unsupported endpoint", tParser.GetInvalidEndpoint().cstr() );
			HttpErrorReply ( dResult, SPH_HTTP_STATUS_501, sError.cstr() );
		}
	}

	return tParser.GetKeepAlive();
}


void sphHttpErrorReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * szError )
{
	HttpErrorReply ( dData, eCode, szError );
}


struct Endpoint_t
{
	const char * m_szName1;
	const char * m_szName2;
};


static Endpoint_t g_dEndpoints[] =
{
	{ "index.html",	nullptr },
	{ "sql",		nullptr },
	{ "search",		"json/search" },
	{ "index",		"json/index" },
	{ "create",		"json/create" },
	{ "insert",		"json/insert" },
	{ "replace",	"json/replace" },
	{ "update",		"json/update" },
	{ "delete",		"json/delete" },
	{ "bulk",		"json/bulk" },
	{ "pq",			"json/pq" }
};

STATIC_ASSERT ( sizeof(g_dEndpoints)/sizeof(g_dEndpoints[0])==SPH_HTTP_ENDPOINT_TOTAL, SPH_HTTP_ENDPOINT_SHOULD_BE_SAME_AS_SPH_HTTP_ENDPOINT_TOTAL );


ESphHttpEndpoint sphStrToHttpEndpoint ( const CSphString & sEndpoint )
{
	if ( sEndpoint.Begins ( g_dEndpoints[SPH_HTTP_ENDPOINT_PQ].m_szName1 ) || sEndpoint.Begins ( g_dEndpoints[SPH_HTTP_ENDPOINT_PQ].m_szName2 ) )
		return SPH_HTTP_ENDPOINT_PQ;

	for ( int i = 0; i < SPH_HTTP_ENDPOINT_TOTAL; i++ )
		if ( sEndpoint==g_dEndpoints[i].m_szName1 || ( g_dEndpoints[i].m_szName2 && sEndpoint==g_dEndpoints[i].m_szName2 ) )
			return ESphHttpEndpoint(i);

	return SPH_HTTP_ENDPOINT_TOTAL;
}


CSphString sphHttpEndpointToStr ( ESphHttpEndpoint eEndpoint )
{
	assert ( eEndpoint>=SPH_HTTP_ENDPOINT_INDEX && eEndpoint<SPH_HTTP_ENDPOINT_TOTAL );
	return g_dEndpoints[eEndpoint].m_szName1;
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
		ServedDescRPtr_c pServed ( GetServed ( sIndex ));
		if ( !CheckValid ( pServed, sIndex, IndexType_e::PERCOLATE ) )
			return false;

		auto pIndex = (PercolateIndex_i *)pServed->m_pIndex;

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
		ServedDescRPtr_c pServed ( GetServed ( sIndex ));
		if ( !CheckValid ( pServed, sIndex, IndexType_e::PERCOLATE ))
			return false;

		auto pIndex = (PercolateIndex_i *)pServed->m_pIndex;

		PercolateQueryArgs_t tArgs ( dFilters, dFilterTree );
		tArgs.m_sQuery = sQuery;
		tArgs.m_sTags = sTags.cstr();
		tArgs.m_iQUID = iID;
		tArgs.m_bReplace = bReplace;
		tArgs.m_bQL = bQueryQL;

		// add query
		StoredQuery_i * pStored = pIndex->CreateQuery ( tArgs, sError );
		if ( pStored )
		{
			RtAccum_t tAcc ( false );
			tAcc.SetIndex ( pIndex );
			ReplicationCommand_t * pCmd = tAcc.AddCommand ( ReplicationCommand_e::PQUERY_ADD );
			pCmd->m_sIndex = sIndex;
			pCmd->m_pStored = pStored;
			// refresh query's UID for reply as it might be auto-generated
			iID = pStored->m_iQUID;

			bOk = HandleCmdReplicate ( tAcc, sError );
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
	CSphScopedPtr<HttpHandler_c> pHandler (
		new HttpHandler_JsonSearch_c ( sQuery.cstr(), m_tOptions ));
	if ( !pHandler )
		return false;

	pHandler->SetErrorFormat (m_bNeedHttpResponse);
	pHandler->Process ();
	m_dData = std::move ( pHandler->GetResult ());
	return true;
}

bool HttpHandlerPQ_c::Delete ( const CSphString & sIndex, const JsonObj_c & tRoot )
{
	RtAccum_t tAcc ( false );
	ReplicationCommand_t * pCmd = tAcc.AddCommand ( ReplicationCommand_e::PQUERY_DELETE );
	pCmd->m_sIndex = sIndex;

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
	bool bOk = HandleCmdReplicate ( tAcc, sError, iDeleted );

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

	if ( !m_sQuery || !m_sQuery[0] )
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

