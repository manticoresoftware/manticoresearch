//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"
#include "sphinxjson.h"
#include "sphinxjsonquery.h"
#include "sphinxpq.h"

#include "http/http_parser.h"
#include "searchdaemon.h"
#include "searchdha.h"
#include "searchdreplication.h"

static void EncodeResultJson ( const AggrResult_t & tRes, JsonEscapedBuilder & tOut )
{
	const ISphSchema & tSchema = tRes.m_tSchema;
	StringBuilder_c sTmp;

	int iAttrsCount = sphSendGetAttrCount(tSchema);

	tOut.StartBlock ( ",", "{", "}");

	// column names
	tOut.StartBlock (",", R"("attrs":[)","]");
	for ( int i=0; i<iAttrsCount; ++i )
		tOut.AppendString ( tSchema.GetAttr(i).m_sName, '"');
	tOut.FinishBlock(false); // attrs

	// attribute values
	tOut.StartBlock ( "," , R"("matches":[)", "]" );
	for ( int iMatch=tRes.m_iOffset; iMatch<tRes.m_iOffset+tRes.m_iCount; ++iMatch )
	{
		tOut.StartBlock ( ",", "[", "]");

		const CSphMatch & tMatch = tRes.m_dMatches [ iMatch ];
		for ( int iAttr=0; iAttr<iAttrsCount; ++iAttr )
		{

			CSphAttrLocator tLoc = tSchema.GetAttr(iAttr).m_tLocator;
			ESphAttr eAttrType = tSchema.GetAttr(iAttr).m_eAttrType;

			assert ( sphPlainAttrToPtrAttr(eAttrType)==eAttrType );

			switch ( eAttrType )
			{
			case SPH_ATTR_FLOAT:
				tOut.Appendf ( "%f", tMatch.GetAttrFloat(tLoc) );
				break;

			case SPH_ATTR_UINT32SET_PTR:
			case SPH_ATTR_INT64SET_PTR:
				{
					ScopedComma_c sNoComma ( tOut, nullptr );
					tOut << "[";
					sphPackedMVA2Str ( (const BYTE *)tMatch.GetAttr ( tLoc ), eAttrType==SPH_ATTR_INT64SET_PTR, tOut );
					tOut << "]";
				}
				break;

			case SPH_ATTR_STRINGPTR:
				{
					auto * pString = ( const BYTE * ) tMatch.GetAttr ( tLoc );
					int iLen = sphUnpackPtrAttr ( pString, &pString );
					tOut.AppendEscaped ( (const char*) pString, EscBld::eEscape, iLen );
				}
				break;

			case SPH_ATTR_JSON_PTR:
				{
					const BYTE * pJSON = (const BYTE *)tMatch.GetAttr ( tLoc );
					sphUnpackPtrAttr ( pJSON, &pJSON );
					auto pTest = tOut.end();
					sphJsonFormat ( tOut, pJSON );
					// empty string (no objects) - return NULL
					// (canonical "{}" and "[]" are handled by sphJsonFormat)

					if ( pTest == tOut.end() )
						tOut += "null";
				}
				break;

			case SPH_ATTR_FACTORS:
			case SPH_ATTR_FACTORS_JSON:
				{
					const BYTE * pFactors = (const BYTE *)tMatch.GetAttr ( tLoc );
					sphUnpackPtrAttr ( pFactors, &pFactors );
					if ( pFactors )
					{
						if ( eAttrType==SPH_ATTR_FACTORS_JSON ) // fixme! need test for it!
							sphFormatFactors ( tOut, ( unsigned int * ) pFactors, true );
						else
						{
							sTmp.Clear ();
							sphFormatFactors ( sTmp, ( unsigned int * ) pFactors, false );
							tOut.AppendEscaped ( sTmp.cstr(), EscBld::eEscape );
						}
					} else
						tOut += "null";
				}
				break;

			case SPH_ATTR_JSON_FIELD_PTR:
				{
					const BYTE * pField = (const BYTE *)tMatch.GetAttr ( tLoc );
					sphUnpackPtrAttr ( pField, &pField );
					if ( !pField )
					{
						tOut += "null";
						break;
					}

					auto eJson = ESphJsonType ( *pField++ );
					if ( eJson==JSON_NULL )
					{
						// no key found - NULL value
						tOut += "null";
					} else
					{
						// send string to client
						sphJsonFieldFormat ( tOut, pField, eJson, true );
					}
				}
				break;

			case SPH_ATTR_INTEGER:
			case SPH_ATTR_TIMESTAMP:
			case SPH_ATTR_BOOL:
			case SPH_ATTR_TOKENCOUNT:
			case SPH_ATTR_BIGINT:
			default:
				tOut.Appendf ( INT64_FMT, tMatch.GetAttr(tLoc) );
			}
		}
		tOut.FinishBlock (); // one match
	}
	tOut.FinishBlock ( false ); // matches

	// meta information
	tOut.StartBlock ( ",", R"("meta":{)", "}" );
	tOut.Sprintf ( R"("total":%d, "total_found":%l, "time":%.3F)",
		tRes.m_iMatches, tRes.m_iTotalMatches, tRes.m_iQueryTime );

	// word statistics
	tOut.StartBlock ( ",", R"("words":[)", "]" );
	auto &hWS = tRes.m_hWordStats;//.IterateStart();
	for ( hWS.IterateStart (); hWS.IterateNext (); )
	{
		const auto &tS = tRes.m_hWordStats.IterateGet ();
		tOut.Sprintf ( R"({"word":"%s", "docs":%l, "hits":%l})", hWS.IterateGetKey().cstr(), tS.m_iDocs, tS.m_iHits );
	}
	tOut.FinishBlock ( false ); // words
	tOut.FinishBlock ( false ); // meta

	if ( !tRes.m_sWarning.IsEmpty() )
	{
		tOut << R"("warning":)";
		tOut.AppendEscaped ( tRes.m_sWarning.cstr (), EscBld::eAll | EscBld::eSkipComma );
	}
	tOut.FinishBlock (false);
}


const char * g_dHttpStatus[] = { "200 OK", "206 Partial Content", "400 Bad Request", "500 Internal Server Error",
								 "501 Not Implemented", "503 Service Unavailable" };
STATIC_ASSERT ( sizeof(g_dHttpStatus)/sizeof(g_dHttpStatus[0])==SPH_HTTP_STATUS_TOTAL, SPH_HTTP_STATUS_SHOULD_BE_SAME_AS_SPH_HTTP_STATUS_TOTAL );


static void HttpBuildReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * sBody, int iBodyLen, bool bHtml )
{
	assert ( sBody && iBodyLen );

	const char * sContent = ( bHtml ? "text/html" : "application/json" );
	CSphString sHttp;
	sHttp.SetSprintf ( "HTTP/1.1 %s\r\nServer: %s\r\nContent-Type: %s; charset=UTF-8\r\nContent-Length:%d\r\n\r\n", g_dHttpStatus[eCode], SPHINX_VERSION, sContent, iBodyLen );

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
	int iParsed = http_parser_execute ( &tParser, &tParserSettings, (const char *)pData, iDataLen );
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

	char * pDst = (char *)sEntity.cstr();
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

		int iValueLen = sCur - sLast;
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
		sVal.SetBinary ( sLast, sCur - sLast );
		UriPercentReplace ( sName );
		UriPercentReplace ( sVal );
		m_hOptions.Add ( sVal, sName );
	}

	if ( sRawData ) {
		sVal.SetBinary ( sRawData, sCur - sRawData );
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
	( (HttpRequestParser_c *)pParser->data )->m_sCurField.SetBinary ( sAt, iLen );
	return 0;
}

int HttpRequestParser_c::ParserHeaderValue ( http_parser * pParser, const char * sAt, size_t iLen )
{
	assert ( pParser->data );
	CSphString sVal;
	sVal.SetBinary ( sAt, iLen );
	HttpRequestParser_c * pHttpParser = (HttpRequestParser_c *)pParser->data;
	pHttpParser->m_hOptions.Add ( sVal, pHttpParser->m_sCurField );
	pHttpParser->m_sCurField = "";
	return 0;
}

int HttpRequestParser_c::ParserBody ( http_parser * pParser, const char * sAt, size_t iLen )
{
	assert ( pParser->data );
	HttpRequestParser_c * pHttpParser = (HttpRequestParser_c *)pParser->data;
	pHttpParser->ParseList ( sAt, iLen );
	pHttpParser->m_sRawBody.SetBinary ( sAt, iLen );
	return 0;
}



static const CSphString * GetAnyValue ( const SmallStringHash_T<CSphString> & hOptions, const char * sKey1, const char * sKey2 )
{
	const CSphString * pVal1 = hOptions ( sKey1 );
	return pVal1 ? pVal1 : hOptions ( sKey2 );
}

static void ParseSearchOptions ( const SmallStringHash_T<CSphString> & hOptions, CSphQuery & tQuery )
{
	const CSphString * pMatch = hOptions ( "match" );
	if ( pMatch )
		tQuery.m_sQuery = *pMatch;

	const CSphString * pIdx = GetAnyValue ( hOptions, "index", "indexes" );
	if ( pIdx )
		tQuery.m_sIndexes = *pIdx;

	const CSphString * pSel = GetAnyValue ( hOptions, "select", "select_list" );
	if ( pSel )
		tQuery.m_sSelect = *pSel;

	const CSphString * pGroup = GetAnyValue ( hOptions, "group", "group_by" );
	if ( pGroup )
		tQuery.m_sGroupBy = *pGroup;

	const CSphString * pOrder = GetAnyValue ( hOptions, "order", "order_by" );
	if ( pOrder )
		tQuery.m_sSortBy = *pOrder;

	const CSphString * pLimit = hOptions ( "limit" );
	if ( pLimit )
		tQuery.m_iLimit = atoi ( pLimit->cstr() );
}

static const char * g_sIndexPage =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>Manticore</title>"
"</head>"
"<body>"
"<h1>Manticore daemon</h1>"
"<p>%s</p>"
"</body>"
"</html>";


static void HttpHandlerIndexPage ( CSphVector<BYTE> & dData )
{
	StringBuilder_c sIndexPage;
	sIndexPage.Appendf ( g_sIndexPage, SPHINX_VERSION );
	HttpBuildReply ( dData, SPH_HTTP_STATUS_200, sIndexPage.cstr(), sIndexPage.GetLength(), true );
}

class CSphQueryProfileFormatJson : public CSphQueryProfile
{
public:

	void BuildResult ( XQNode_t * pRoot, const CSphSchema &tSchema, const StrVec_t &dZones ) final
	{
		sphBuildProfileJson ( m_sResult, pRoot, tSchema, dZones );
	}

	const char* GetResultAsStr () const final
	{
		return m_sResult.cstr();
	}

private:
	JsonEscapedBuilder m_sResult;
};

//////////////////////////////////////////////////////////////////////////
class JsonRequestBuilder_c : public IRequestBuilder_t
{
public:
	JsonRequestBuilder_c ( const CSphString & sQuery, const SqlStmt_t & /*tStmt*/, ESphHttpEndpoint eEndpoint )
		: m_eEndpoint ( eEndpoint )
		, m_tQuery ( sQuery.cstr() )
	{
		// fixme: we can implement replacing indexes in a string (without parsing) if it becomes a performance issue
	}

	void BuildRequest ( const AgentConn_t & tAgent, CachedOutputBuffer_c & tOut ) const final
	{
		// replace "index" value in the json query
		m_tQuery.DelItem ( "index" );
		m_tQuery.AddStr ( "index", tAgent.m_tDesc.m_sIndexes.cstr() );

		CSphString sRequest = m_tQuery.AsString();
		CSphString sEndpoint = sphHttpEndpointToStr ( m_eEndpoint );

		APICommand_t tWr { tOut, SEARCHD_COMMAND_JSON, VER_COMMAND_JSON }; // API header
		tOut.SendString ( sEndpoint.cstr() );
		tOut.SendString ( sRequest.cstr() );
	}

private:
	ESphHttpEndpoint	m_eEndpoint {SPH_HTTP_ENDPOINT_TOTAL};
	mutable JsonObj_c	m_tQuery;
};


class JsonReplyParser_c : public IReplyParser_t
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
		CSphFixedVector<BYTE> dResult ( uLength );
		tReq.GetBytes ( dResult.Begin(), uLength );

		return sphGetResultStats ( (const char *)dResult.Begin(), m_iAffected, m_iWarnings, eEndpoint==SPH_HTTP_ENDPOINT_JSON_UPDATE );
	}

protected:
	int &	m_iAffected;
	int &	m_iWarnings;
};


class JsonParserFactory_c : public QueryParserFactory_i
{
public:
	JsonParserFactory_c ( ESphHttpEndpoint eEndpoint )
		: m_eEndpoint ( eEndpoint )
	{}

	virtual QueryParser_i * CreateQueryParser() const override
	{
		return sphCreateJsonQueryParser();
	}

	IRequestBuilder_t * CreateRequestBuilder ( const CSphString & sQuery, const SqlStmt_t & tStmt ) const override
	{
		return new JsonRequestBuilder_c ( sQuery, tStmt, m_eEndpoint );
	}

	IReplyParser_t * CreateReplyParser ( int & iUpdated, int & iWarnings ) const override
	{
		return new JsonReplyParser_c ( iUpdated, iWarnings );
	}

private:
	ESphHttpEndpoint m_eEndpoint {SPH_HTTP_ENDPOINT_TOTAL};
};

//////////////////////////////////////////////////////////////////////////
class HttpErrorReporter_c : public StmtErrorReporter_i
{
public:
	virtual void	Ok ( int iAffectedRows, const CSphString & /*sWarning*/ ) { m_iAffected = iAffectedRows; }
	virtual void	Ok ( int iAffectedRows, int /*nWarnings*/ ) { m_iAffected = iAffectedRows; }
	virtual void	Error ( const char * sStmt, const char * sError, MysqlErrors_e iErr );
	virtual SqlRowBuffer_c * GetBuffer() { return NULL; }

	bool			IsError() const { return m_bError; }
	const char *	GetError() const { return m_sError.cstr(); }
	int				GetAffectedRows() const { return m_iAffected; }

private:
	bool			m_bError {false};
	CSphString		m_sError;
	int				m_iAffected {0};
};


void HttpErrorReporter_c::Error ( const char * /*sStmt*/, const char * sError, MysqlErrors_e /*iErr*/ )
{
	m_bError = true;
	m_sError = sError;
}

//////////////////////////////////////////////////////////////////////////
// all the handlers for http queries

class HttpHandler_c
{
public:
	HttpHandler_c ( const CSphString & sQuery, const ThdDesc_t & tThd, bool bNeedHttpResponse )
		: m_sQuery ( sQuery )
		, m_bNeedHttpResponse ( bNeedHttpResponse )
		, m_tThd ( tThd )
	{}

	virtual ~HttpHandler_c()
	{}

	virtual bool Process () = 0;
	
	CSphVector<BYTE> & GetResult()
	{
		return m_dData;
	}

protected:
	const CSphString &	m_sQuery;
	bool				m_bNeedHttpResponse {false};
	CSphVector<BYTE>	m_dData;
	const ThdDesc_t &	m_tThd;

	void ReportError ( const char * szError, ESphHttpStatus eStatus )
	{
		if ( m_bNeedHttpResponse )
			HttpErrorReply ( m_dData, eStatus, szError );
		else
		{
			m_dData.Resize ( strlen(szError)+1 );
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
			m_dData.Resize ( iPrinted+1 );
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
			m_dData.Resize ( iLen+1 );
			memcpy ( m_dData.Begin(), sResult, m_dData.GetLength() );
		}
	}

	ServedDescPtr_c GetIndex ( const CSphString & sIndex, IndexType_e eType )
	{
		ServedDescPtr_c pServed ( GetServed ( sIndex ), false );
		if ( !pServed )
		{
			FormatError ( SPH_HTTP_STATUS_500, "no such index '%s'", sIndex.cstr () );
			return ServedDescPtr_c();
		}
		if ( pServed->m_eType!=eType || !pServed->m_pIndex )
		{
			FormatError ( SPH_HTTP_STATUS_500, "index '%s' is not %s (enabled=%d)", sIndex.cstr(), GetTypeName ( eType ).cstr(), (int)(!!pServed->m_pIndex) );
			return ServedDescPtr_c();
		}

		return pServed;
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


class HttpSearchHandler_c : public HttpHandler_c, public HttpOptionsTraits_c
{
public:
	HttpSearchHandler_c ( const CSphString & sQuery, const OptionsHash_t & tOptions, const ThdDesc_t & tThd, bool bNeedHttpResponse )
		: HttpHandler_c ( sQuery, tThd, bNeedHttpResponse )
		, HttpOptionsTraits_c ( tOptions )
	{}

	virtual bool Process () override
	{
		CSphQuery tQuery;
		CSphString sWarning;
		QueryParser_i * pQueryParser = PreParseQuery();
		if ( !pQueryParser )
			return false;

		m_tQuery.m_pQueryParser = pQueryParser;
		CSphScopedPtr<ISphSearchHandler> tHandler ( sphCreateSearchHandler ( 1, pQueryParser, m_eQueryType, true, m_tThd ) );

		CSphQueryProfileFormatJson tProfile;
		if ( m_bProfile )
			tHandler->SetProfile ( &tProfile );

		tHandler->SetQuery ( 0, m_tQuery, nullptr );

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

		CSphString sResult = EncodeResult ( *pRes, m_bProfile ? &tProfile : NULL );
		BuildReply ( sResult, SPH_HTTP_STATUS_200 );

		return true;
	}

protected:
	bool					m_bProfile {false};
	bool					m_bAttrHighlight {false};
	QueryType_e				m_eQueryType {QUERY_SQL};
	CSphQuery				m_tQuery;
	CSphString				m_sWarning;

	virtual QueryParser_i * PreParseQuery() = 0;
	virtual CSphString		EncodeResult ( const AggrResult_t & tRes, CSphQueryProfile * pProfile ) = 0;
};


class HttpSearchHandler_Plain_c : public HttpSearchHandler_c
{
public:
	HttpSearchHandler_Plain_c ( const CSphString & sQuery, const OptionsHash_t & tOptions, const ThdDesc_t & tThd, bool bNeedHttpResponse )
		: HttpSearchHandler_c ( sQuery, tOptions, tThd, bNeedHttpResponse )
	{}

protected:
	QueryParser_i * PreParseQuery() override
	{
		CSphString sError;
		ParseSearchOptions ( m_tOptions, m_tQuery );
		if ( !ParseSelectList ( sError, m_tQuery ) )
		{
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return NULL;
		}

		if ( !m_tQuery.m_sSortBy.IsEmpty() )
			m_tQuery.m_eSort = SPH_SORT_EXTENDED;

		m_eQueryType = QUERY_SQL;

		return sphCreatePlainQueryParser();
	}

	CSphString EncodeResult ( const AggrResult_t & tRes, CSphQueryProfile * /*pProfile*/ ) override
	{
		JsonEscapedBuilder tResBuilder;
		EncodeResultJson ( tRes, tResBuilder );
		return tResBuilder.cstr();
	}
};


class HttpSearchHandler_SQL_c : public HttpSearchHandler_Plain_c
{
public:
	HttpSearchHandler_SQL_c ( const CSphString & sQuery, const OptionsHash_t & tOptions, const ThdDesc_t & tThd, bool bNeedHttpResponse )
		: HttpSearchHandler_Plain_c ( sQuery, tOptions, tThd, bNeedHttpResponse )
	{}

protected:
	QueryParser_i * PreParseQuery() override
	{
		const CSphString * pRawQl = nullptr;
		CSphString sQuery;
		auto pBaypass = m_tOptions ( "decoded_mode" );
		if ( pBaypass && !pBaypass->IsEmpty () ) // test for 'mode=raw&query=...'
		{
			CSphString &a = *pBaypass;
			if ( a.SubString (0,10) == "raw&query=" )
			{
				sQuery = a.cstr()+10;
				pRawQl = &sQuery;
			}
		}

		if (!pRawQl)
			pRawQl = m_tOptions ( "query" );
		if ( !pRawQl || pRawQl->IsEmpty() )
		{
			ReportError ( "query missing", SPH_HTTP_STATUS_400 );
			return nullptr;
		}

		CSphString sError;
		CSphVector<SqlStmt_t> dStmt;
		if ( !sphParseSqlQuery ( pRawQl->cstr(), pRawQl->Length(), dStmt, sError, SPH_COLLATION_DEFAULT ) )
		{
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return nullptr;
		}

		m_tQuery = dStmt[0].m_tQuery;
		if ( dStmt.GetLength()>1 )
		{
			ReportError ( "multiple queries not supported", SPH_HTTP_STATUS_501 );
			return nullptr;
		} else if ( dStmt[0].m_eStmt!=STMT_SELECT )
		{
			ReportError ( "only SELECT queries are supported", SPH_HTTP_STATUS_501 );
			return nullptr;
		}

		m_eQueryType = QUERY_SQL;

		return sphCreatePlainQueryParser();
	}
};


class HttpHandler_JsonSearch_c : public HttpSearchHandler_c
{
public:	
	HttpHandler_JsonSearch_c ( const CSphString & sQuery, const OptionsHash_t & tOptions, const ThdDesc_t & tThd, bool bNeedHttpResponse )
		: HttpSearchHandler_c ( sQuery, tOptions, tThd, bNeedHttpResponse )
	{}

	QueryParser_i * PreParseQuery() override
	{
		CSphString sError;
		if ( !sphParseJsonQuery ( m_sQuery.cstr(), m_tQuery, m_bProfile, m_bAttrHighlight, sError, m_sWarning ) )
		{
			sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return NULL;
		}

		m_eQueryType = QUERY_JSON;

		return sphCreateJsonQueryParser();
	}

protected:
	CSphString EncodeResult ( const AggrResult_t & tRes, CSphQueryProfile * pProfile ) override
	{
		return sphEncodeResultJson ( tRes, m_tQuery, pProfile, m_bAttrHighlight );
	}
};


class HttpJsonInsertTraits_c
{
protected:
	bool ProcessInsert ( SqlStmt_t & tStmt, SphDocID_t tDocId, bool bReplace, JsonObj_c & tResult )
	{
		CSphSessionAccum tAcc ( false );
		CSphString sWarning;
		HttpErrorReporter_c tReporter;
		sphHandleMysqlInsert ( tReporter, tStmt, bReplace, true, sWarning, tAcc, SPH_COLLATION_DEFAULT );

		if ( tReporter.IsError() )
			tResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
		else
			tResult = sphEncodeInsertResultJson ( tStmt.m_sIndex.cstr(), bReplace, tDocId );

		return !tReporter.IsError();
	}
};


class HttpHandler_JsonInsert_c : public HttpHandler_c, public HttpJsonInsertTraits_c
{
public:
	HttpHandler_JsonInsert_c ( const CSphString & sQuery, bool bReplace, bool bNeedHttpResponse, const ThdDesc_t & tThd )
		: HttpHandler_c ( sQuery, tThd, bNeedHttpResponse )
		, m_bReplace ( bReplace )
	{}

	bool Process () override
	{
		SqlStmt_t tStmt;
		SphDocID_t tDocId = DOCID_MAX;
		CSphString sError;
		if ( !sphParseJsonInsert ( m_sQuery.cstr(), tStmt, tDocId, m_bReplace, sError ) )
		{
			sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return false;
		}

		JsonObj_c tResult;
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
	bool ProcessUpdate ( const char * szRawRequest, const SqlStmt_t & tStmt, SphDocID_t tDocId, const ThdDesc_t & tThd, JsonObj_c & tResult )
	{
		HttpErrorReporter_c tReporter;
		CSphString sWarning;
		JsonParserFactory_c tFactory ( SPH_HTTP_ENDPOINT_JSON_UPDATE );
		sphHandleMysqlUpdate ( tReporter, tFactory, tStmt, szRawRequest, sWarning, tThd );

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
	HttpHandler_JsonUpdate_c ( const CSphString & sQuery, const ThdDesc_t & tThd, bool bNeedHttpResponse )
		: HttpHandler_c ( sQuery, tThd, bNeedHttpResponse )
	{}

	bool Process () override
	{
		SqlStmt_t tStmt;
		SphDocID_t tDocId = DOCID_MAX;
		CSphString sError;
		if ( !ParseQuery ( tStmt, tDocId, sError ) )
		{
			sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return false;
		}

		JsonObj_c tResult;
		bool bResult = ProcessQuery ( tStmt, tDocId, tResult );
		BuildReply ( tResult.AsString(), bResult ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500 );

		return bResult;
	}

protected:
	virtual bool ParseQuery ( SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError )
	{
		return sphParseJsonUpdate ( m_sQuery.cstr(), tStmt, tDocId, sError );
	}

	virtual bool ProcessQuery ( const SqlStmt_t & tStmt, SphDocID_t tDocId, JsonObj_c & tResult )
	{
		return ProcessUpdate ( m_sQuery.cstr(), tStmt, tDocId, m_tThd, tResult );
	}
};


class HttpJsonDeleteTraits_c
{
protected:
	bool ProcessDelete ( const char * szRawRequest, const SqlStmt_t & tStmt, SphDocID_t tDocId, const ThdDesc_t & tThd, JsonObj_c & tResult )
	{
		CSphSessionAccum tAcc ( false );
		HttpErrorReporter_c tReporter;
		CSphString sWarning;
		JsonParserFactory_c tFactory ( SPH_HTTP_ENDPOINT_JSON_DELETE );
		sphHandleMysqlDelete ( tReporter, tFactory, tStmt, szRawRequest, true, tAcc, tThd );

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
	HttpHandler_JsonDelete_c ( const CSphString & sQuery, const ThdDesc_t & tThd, bool bNeedHttpResponse )
		: HttpHandler_JsonUpdate_c ( sQuery, tThd, bNeedHttpResponse )
	{}

protected:
	bool ParseQuery ( SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError ) override
	{
		return sphParseJsonDelete ( m_sQuery.cstr(), tStmt, tDocId, sError );
	}

	bool ProcessQuery ( const SqlStmt_t & tStmt, SphDocID_t tDocId, JsonObj_c & tResult ) override
	{
		return ProcessDelete ( m_sQuery.cstr(), tStmt, tDocId, m_tThd, tResult );
	}
};


class HttpHandler_JsonBulk_c : public HttpHandler_c, public HttpOptionsTraits_c, public HttpJsonInsertTraits_c, public HttpJsonUpdateTraits_c, public HttpJsonDeleteTraits_c
{
public:
	HttpHandler_JsonBulk_c ( const CSphString & sQuery, const OptionsHash_t & tOptions, const ThdDesc_t & tThd, bool bNeedHttpResponse )
		: HttpHandler_c ( sQuery, tThd, bNeedHttpResponse )
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
		char * p = const_cast<char*>(m_sQuery.cstr());

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
			SphDocID_t tDocId = DOCID_MAX;
			CSphString sStmt;
			CSphString sError;
			CSphString sQuery;
			if ( !sphParseJsonStatement ( szStmt, tStmt, sStmt, sQuery, tDocId, sError ) )
			{
				sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
				ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
				return false;
			}

			JsonObj_c tResult;
			bResult = false;

			switch ( tStmt.m_eStmt )
			{
			case STMT_INSERT:
			case STMT_REPLACE:
				bResult = ProcessInsert ( tStmt, tDocId, tStmt.m_eStmt==STMT_REPLACE, tResult );
				break;

			case STMT_UPDATE:
				bResult = ProcessUpdate ( sQuery.cstr(), tStmt, tDocId, m_tThd, tResult );
				break;

			case STMT_DELETE:
				bResult = ProcessDelete ( sQuery.cstr(), tStmt, tDocId, m_tThd, tResult );
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
	HttpHandlerPQ_c (  const CSphString & sQuery, const ThdDesc_t & tThd, bool bNeedHttpResponse, const OptionsHash_t & tOptions )
		: HttpHandler_c ( sQuery, tThd, bNeedHttpResponse )
		, m_tOptions ( tOptions )
	{}

	virtual bool Process () override;

private:
	const OptionsHash_t & m_tOptions;

	// FIXME!!! handle replication for GotQuery and Delete
	bool DoCallPQ ( const CSphString & sIndex, const JsonObj_c & tPercolate, bool bVerbose );
	bool GotQuery ( const CSphString & sIndex, const JsonObj_c & tJsonQuery, const JsonObj_c & tRoot, CSphString * pUID, bool bReplace );
	bool ListQueries ( const CSphString & sIndex );
	bool Delete ( const CSphString & sIndex, const JsonObj_c & tRoot );
};


static HttpHandler_c * CreateHttpHandler ( ESphHttpEndpoint eEndpoint, const CSphString & sQuery, const OptionsHash_t & tOptions, const ThdDesc_t & tThd, bool bNeedHttpResonse, http_method eRequestType )
{
	switch ( eEndpoint )
	{
	case SPH_HTTP_ENDPOINT_SEARCH:
		return new HttpSearchHandler_Plain_c ( sQuery, tOptions, tThd, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_SQL:
		return new HttpSearchHandler_SQL_c ( sQuery, tOptions, tThd, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_JSON_SEARCH:
		return new HttpHandler_JsonSearch_c ( sQuery, tOptions, tThd, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_JSON_INDEX:
	case SPH_HTTP_ENDPOINT_JSON_CREATE:
	case SPH_HTTP_ENDPOINT_JSON_INSERT:
	case SPH_HTTP_ENDPOINT_JSON_REPLACE:
		return new HttpHandler_JsonInsert_c ( sQuery, eEndpoint==SPH_HTTP_ENDPOINT_JSON_INDEX || eEndpoint==SPH_HTTP_ENDPOINT_JSON_REPLACE, bNeedHttpResonse, tThd );

	case SPH_HTTP_ENDPOINT_JSON_UPDATE:
		return new HttpHandler_JsonUpdate_c ( sQuery, tThd, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_JSON_DELETE:
		return new HttpHandler_JsonDelete_c ( sQuery, tThd, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_JSON_BULK:
		return new HttpHandler_JsonBulk_c ( sQuery, tOptions, tThd, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_PQ:
		return new HttpHandlerPQ_c ( sQuery, tThd, bNeedHttpResonse, tOptions );

	default:
		break;
	}

	return nullptr;
}


static bool sphProcessHttpQuery ( ESphHttpEndpoint eEndpoint, const CSphString & sQuery, const SmallStringHash_T<CSphString> & tOptions, const ThdDesc_t & tThd, CSphVector<BYTE> & dResult, bool bNeedHttpResponse, http_method eRequestType )
{
	CSphScopedPtr<HttpHandler_c> pHandler ( CreateHttpHandler ( eEndpoint, sQuery, tOptions, tThd, bNeedHttpResponse, eRequestType ) );
	if ( !pHandler.Ptr() )
		return false;

	pHandler->Process();
	dResult = std::move ( pHandler->GetResult() );
	return true;
}


bool sphProcessHttpQueryNoResponce ( ESphHttpEndpoint eEndpoint, const CSphString & sQuery, const SmallStringHash_T<CSphString> & tOptions, const ThdDesc_t & tThd, CSphVector<BYTE> & dResult )
{
	return sphProcessHttpQuery ( eEndpoint, sQuery, tOptions, tThd, dResult, false, HTTP_GET );
}


bool sphLoopClientHttp ( const BYTE * pRequest, int iRequestLen, CSphVector<BYTE> & dResult, const ThdDesc_t & tThd )
{
	HttpRequestParser_c tParser;
	if ( !tParser.Parse ( pRequest, iRequestLen ) )
	{
		HttpErrorReply ( dResult, SPH_HTTP_STATUS_400, tParser.GetError() );
		return tParser.GetKeepAlive();
	}

	ESphHttpEndpoint eEndpoint = tParser.GetEndpoint();
	if ( !sphProcessHttpQuery ( eEndpoint, tParser.GetBody(), tParser.GetOptions(), tThd, dResult, true, tParser.GetRequestType() ) )
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


const char * g_dEndpoints[] = { "index.html", "search", "sql", "json/search", "json/index", "json/create", "json/insert", "json/replace", "json/update", "json/delete", "json/bulk", "json/pq" };
STATIC_ASSERT ( sizeof(g_dEndpoints)/sizeof(g_dEndpoints[0])==SPH_HTTP_ENDPOINT_TOTAL, SPH_HTTP_ENDPOINT_SHOULD_BE_SAME_AS_SPH_HTTP_ENDPOINT_TOTAL );

ESphHttpEndpoint sphStrToHttpEndpoint ( const CSphString & sEndpoint )
{
	if ( sEndpoint.Begins ( g_dEndpoints[SPH_HTTP_ENDPOINT_PQ] ) )
		return SPH_HTTP_ENDPOINT_PQ;

	for ( int i = 0; i < SPH_HTTP_ENDPOINT_TOTAL; i++ )
		if ( sEndpoint==g_dEndpoints[i] )
			return ESphHttpEndpoint(i);

	return SPH_HTTP_ENDPOINT_TOTAL;
}


CSphString sphHttpEndpointToStr ( ESphHttpEndpoint eEndpoint )
{
	assert ( eEndpoint>=SPH_HTTP_ENDPOINT_INDEX && eEndpoint<SPH_HTTP_ENDPOINT_TOTAL );
	return g_dEndpoints[eEndpoint];
}


static void EncodePercolateMatchResult ( const PercolateMatchResult_t & tRes, const CSphFixedVector<int64_t> & dDocids,
	const CSphString & sIndex, JsonEscapedBuilder & tOut )
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
		tOut.Sprintf ( R"("_index":"%s","_type":"doc","_id":"%u","_score":"1")", sIndex.cstr(), tDesc.m_uQID );
		if ( !tDesc.m_bQL )
			tOut.Sprintf ( R"("_source":{"query":%s})", tDesc.m_sQuery.cstr () );
		else
		{
			ScopedComma_c sBrackets ( tOut, nullptr, R"("_source":{ "query": {"ql":)", " } }");
			tOut.AppendEscaped ( tDesc.m_sQuery.cstr(), EscBld::eEscape );
		}

		// document count + document id(s)
		if ( tRes.m_bGetDocs )
		{
			ScopedComma_c sFields ( tOut, ",",R"("fields":{"_percolator_document_slot": [)", "] }");
			for ( int iDoc = 1; iDoc<=tRes.m_dDocs[iDocOff]; ++iDoc )
			{
				int iRow = tRes.m_dDocs[iDocOff + iDoc];
				tOut.Sprintf ("%U", ( dDocids.GetLength () ? dDocids[iRow] : iRow ) ); // docid
			}
			iDocOff += tRes.m_dDocs[iDocOff] + 1;
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

	CSphSessionAccum tAcc ( true );
	CPqResult tResult;
	tResult.m_dResult.m_bGetFilters = false;

	PercolateMatchDocuments ( dDocs, tOpts, tAcc, tResult );

	JsonEscapedBuilder sRes;
	EncodePercolateMatchResult ( tResult.m_dResult, tResult.m_dDocids, sIndex, sRes );
	BuildReply ( sRes, SPH_HTTP_STATUS_200 );

	return true;
}


static void EncodePercolateQueryResult ( bool bReplace, const CSphString & sIndex, uint64_t uID, StringBuilder_c & tOut )
{
	if ( bReplace )
		tOut.Sprintf (R"({"index":"%s","type":"doc","_id":"%U","result":"updated","forced_refresh":true})", sIndex.cstr(), uID);
	else
		tOut.Sprintf ( R"({"index":"%s","type":"doc","_id":"%U","result":"created"})", sIndex.cstr (), uID );
}


bool HttpHandlerPQ_c::GotQuery ( const CSphString & sIndex, const JsonObj_c & tJsonQuery, const JsonObj_c & tRoot, CSphString * pUID, bool bReplace )
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

	uint64_t uID = 0;
	if ( pUID && !pUID->IsEmpty() )
		uID = strtoull ( pUID->cstr(), nullptr, 10 );

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
		ServedDescPtr_c pServed ( GetIndex ( sIndex, IndexType_e::PERCOLATE ) );
		if ( !pServed )
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

	StoredQuery_i * pStored = nullptr;
	// scope for index lock
	{
		ServedDescPtr_c pServed ( GetIndex ( sIndex, IndexType_e::PERCOLATE ) );
		if ( !pServed )
			return false;

		auto pIndex = (PercolateIndex_i *)pServed->m_pIndex;

		PercolateQueryArgs_t tArgs ( dFilters, dFilterTree );
		tArgs.m_sQuery = sQuery;
		tArgs.m_sTags = sTags.cstr();
		tArgs.m_uQUID = uID;
		tArgs.m_bReplace = bReplace;
		tArgs.m_bQL = bQueryQL;

		// add query
		pStored = pIndex->Query ( tArgs, sError );
	}
	bool bOk = ( pStored!=nullptr );
	if ( pStored )
	{
		ReplicationCommand_t tCmd;
		tCmd.m_eCommand = RCOMMAND_PQUERY_ADD;
		tCmd.m_sIndex = sIndex;
		tCmd.m_pStored = pStored;
		// refresh query's UID for reply as it might be auto-generated
		uID = pStored->m_uQUID;

		bOk = HandleCmdReplicate ( tCmd, sError, nullptr );
	}

	if ( !bOk )
		ReportError ( sError.scstr(), SPH_HTTP_STATUS_500 );
	else
	{
		StringBuilder_c sRes;
		EncodePercolateQueryResult ( bReplace, sIndex, uID, sRes );
		BuildReply ( sRes, SPH_HTTP_STATUS_200 );
	}

	return bOk;
}

bool HttpHandlerPQ_c::ListQueries ( const CSphString & sIndex )
{
	// FIXME!!! provide filters
	const char * sFilterTags = nullptr;
	const CSphFilterSettings * pUID = nullptr;

	ServedDescPtr_c pServed ( GetIndex ( sIndex, IndexType_e::PERCOLATE ) );
	if ( !pServed )
		return false;

	auto pIndex = (PercolateIndex_i *)pServed->m_pIndex;

	uint64_t tmStart = sphMicroTimer();

	CSphVector<PercolateQueryDesc> dQueries;
	pIndex->GetQueries ( sFilterTags, true, pUID, 0, 0, dQueries );

	PercolateMatchResult_t tRes;
	tRes.m_bGetDocs = false;
	tRes.m_dQueryDesc.Set ( dQueries.Begin(), dQueries.GetLength() );
	dQueries.LeakData();
	tRes.m_dDocs.Reset ( tRes.m_dQueryDesc.GetLength() );
	tRes.m_dDocs.Fill ( 0 );

	tRes.m_tmTotal = sphMicroTimer() - tmStart;

	CSphFixedVector<int64_t> dTmpids ( 0 );
	JsonEscapedBuilder sRes;
	EncodePercolateMatchResult ( tRes, dTmpids, sIndex, sRes );
	BuildReply ( sRes, SPH_HTTP_STATUS_200 );

	return true;
}

bool HttpHandlerPQ_c::Delete ( const CSphString & sIndex, const JsonObj_c & tRoot )
{
	ReplicationCommand_t tCmd;
	tCmd.m_eCommand = RCOMMAND_DELETE;
	tCmd.m_sIndex = sIndex;

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
		tCmd.m_dDeleteQueries.Add ( i.IntVal() );

	if ( !sTags.GetLength() && !tCmd.m_dDeleteQueries.GetLength() )
	{
		ReportError ( "no tags or id field arrays found", SPH_HTTP_STATUS_400 );
		return false;
	}

	tCmd.m_sDeleteTags = sTags.cstr();
	uint64_t tmStart = sphMicroTimer();

	int iDeleted = 0;
	bool bOk = HandleCmdReplicate ( tCmd, sError, &iDeleted );

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

	if ( m_sQuery.IsEmpty() )
		return ListQueries ( sIndex );

	const JsonObj_c tRoot ( m_sQuery.cstr() );
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

		return GotQuery ( sIndex, tQuery, tRoot, pUID, bRefresh );
	}
}

