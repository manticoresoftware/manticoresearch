//
// $Id$
//

//
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

#include "http/http_parser.h"
#include "json/cJSON.h"
#include "searchdaemon.h"
#include "searchdha.h"

struct EscapeJsonString_t
{
	static bool IsEscapeChar ( char c )
	{
		return ( c=='"' || c=='\\' || c=='/' || c=='\b' || c=='\n' || c=='\r' || c=='\t'|| c=='\f' );
	}

	static char GetEscapedChar ( char c )
	{
		switch ( c )
		{
		case '\b': return 'b';
		case '\n': return 'n';
		case '\r': return 'r';
		case '\t': return 't';
		case '\f': return 'f';
		default: return c;
		}
	}
};

using JsonEscapedBuilder = EscapedStringBuilder_T <EscapeJsonString_t>;

static void AppendJsonKey ( const char * sName, JsonEscapedBuilder & tOut )
{
	tOut += "\"";
	tOut += sName;
	tOut += "\":";
}


static void EncodeResultJson ( const AggrResult_t & tRes, JsonEscapedBuilder & tOut )
{
	const ISphSchema & tSchema = tRes.m_tSchema;
	CSphVector<BYTE> dTmp;

	int iAttrsCount = sphSendGetAttrCount(tSchema);

	tOut += "{";

	// column names
	AppendJsonKey ( "attrs", tOut );
	tOut += "[";
	for ( int i=0; i<iAttrsCount; i++ )
		tOut.Appendf ( "%s\"%s\"", ( i==0 ? "" : "," ), tSchema.GetAttr ( i ).m_sName.cstr() );
	tOut += "],";

	// attribute values
	AppendJsonKey ( "matches", tOut );
	tOut += "[";
	for ( int iMatch=tRes.m_iOffset; iMatch<tRes.m_iOffset+tRes.m_iCount; iMatch++ )
	{
		tOut += ( iMatch==tRes.m_iOffset ? "[" : ",[" );

		const CSphMatch & tMatch = tRes.m_dMatches [ iMatch ];
		const char * sSep = "";
		for ( int iAttr=0; iAttr<iAttrsCount; iAttr++ )
		{
			tOut += sSep;

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
					tOut += "[";
					CSphVector<char> dStr;
					sphPackedMVA2Str ( (const BYTE *)tMatch.GetAttr ( tLoc ), eAttrType==SPH_ATTR_INT64SET_PTR, dStr );
					dStr.Add('\0');
					tOut += dStr.Begin();
					tOut += "]";
				}
				break;

			case SPH_ATTR_STRINGPTR:
				{
					const BYTE * pString = (const BYTE *)tMatch.GetAttr ( tLoc );
					int iLen = sphUnpackPtrAttr ( pString, &pString );
					dTmp.Resize ( iLen+1 );
					memcpy ( dTmp.Begin(), pString, iLen );
					dTmp[iLen] = '\0';
					tOut += "\"";
					tOut.AppendEscaped ( ( const char *)dTmp.Begin(), true, false );
					tOut += "\"";
				}
				break;

			case SPH_ATTR_JSON_PTR:
				{
					const BYTE * pJSON = (const BYTE *)tMatch.GetAttr ( tLoc );
					sphUnpackPtrAttr ( pJSON, &pJSON );

					// no object at all? return NULL
					if ( !pJSON )
					{
						tOut += "null";
						break;
					}

					dTmp.Resize ( 0 );
					sphJsonFormat ( dTmp, pJSON );
					if ( !dTmp.GetLength() )
					{
						// empty string (no objects) - return NULL
						// (canonical "{}" and "[]" are handled by sphJsonFormat)
						tOut += "null";
						break;
					}

					dTmp.Add(0);
					tOut += (const char *)dTmp.Begin();
				}
				break;

			case SPH_ATTR_FACTORS:
			case SPH_ATTR_FACTORS_JSON:
				{
					const BYTE * pFactors = (const BYTE *)tMatch.GetAttr ( tLoc );
					sphUnpackPtrAttr ( pFactors, &pFactors );
					if ( pFactors )
					{
						bool bStr = ( eAttrType==SPH_ATTR_FACTORS );
						dTmp.Resize ( 0 );
						sphFormatFactors ( dTmp, (unsigned int *)pFactors, !bStr );
						dTmp.Add ( '\0' );
						if ( bStr )
						{
							tOut += "\"";
							tOut.AppendEscaped ( (const char *)dTmp.Begin(), true, false );
							tOut += "\"";
						} else
							tOut += (const char *)dTmp.Begin();
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

					ESphJsonType eJson = ESphJsonType ( *pField++ );
					if ( eJson==JSON_NULL )
					{
						// no key found - NULL value
						tOut += "null";
					} else
					{
						// send string to client
						dTmp.Resize ( 0 );
						sphJsonFieldFormat ( dTmp, pField, eJson, true );
						dTmp.Add ( '\0' );
						tOut += (const char *)dTmp.Begin();
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

			sSep = ",";
		}

		tOut += "]";
	}
	tOut += "],";

	// meta information
	AppendJsonKey ( "meta", tOut );
	tOut += "{";

	tOut.Appendf ( "\"total\":%d, \"total_found\":" INT64_FMT ", \"time\":%d.%03d,", tRes.m_iMatches, tRes.m_iTotalMatches, tRes.m_iQueryTime/1000, tRes.m_iQueryTime%1000 );

	// word statistics
	AppendJsonKey ( "words", tOut );
	tOut += "[";
	tRes.m_hWordStats.IterateStart();
	const char * sSep = "";
	while ( tRes.m_hWordStats.IterateNext() )
	{
		const CSphQueryResultMeta::WordStat_t & tStat = tRes.m_hWordStats.IterateGet();
		tOut.Appendf ( "%s{\"word\":\"%s\", \"docs\":" INT64_FMT ", \"hits\":" INT64_FMT "}", sSep, tRes.m_hWordStats.IterateGetKey().cstr(), tStat.m_iDocs, tStat.m_iHits );
		sSep = ",";
	}
	tOut += "]}";

	if ( !tRes.m_sWarning.IsEmpty() )
	{
		tOut.Appendf ( ",\"warning\":\"" );
		tOut.AppendEscaped ( tRes.m_sWarning.cstr() );
		tOut.Appendf ( "\"" );
	}	

	tOut += "}";
}


const char * g_dHttpStatus[] = { "200 OK", "206 Partial Content", "400 Bad Request", "500 Internal Server Error", "501 Not Implemented", "503 Service Unavailable" };
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
	cJSON * pError = cJSON_CreateObject();
	assert ( pError );

	cJSON_AddStringToObject ( pError, "error", szError );
	CSphString sJsonError = sphJsonToString ( pError );
	cJSON_Delete ( pError );

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

	return true;
}

static BYTE Char2Hex ( BYTE uChar )
{
	if ( uChar>=0x41 && uChar<=0x46 )
		return ( uChar - 'A' ) + 10;
	else if ( uChar>=0x61 && uChar<=0x66 )
		return ( uChar - 'a' ) + 10;
	else
		return uChar - '0';
}

static void UriPercentReplace ( CSphString & sEntity )
{
	if ( sEntity.IsEmpty() )
		return;

	char * pDst = (char *)sEntity.cstr();
	const char * pSrc = pDst;
	while ( *pSrc )
	{
		if ( *pSrc=='%' && *(pSrc+1) && *(pSrc+2) )
		{
			BYTE uCode = Char2Hex ( *(pSrc+1) ) * 16 + Char2Hex ( *(pSrc+2) );
			pSrc += 3;
			*pDst++ = uCode;
		} else
		{
			*pDst++ = ( *pSrc=='+' ? ' ' : *pSrc );
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

		CSphString sEndpoint ( sPath, iPathLen );
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
	HttpBuildReply ( dData, SPH_HTTP_STATUS_200, sIndexPage.cstr(), sIndexPage.Length(), true );
}


class CSphQueryProfileJson : public CSphQueryProfile
{
public:
	virtual					~CSphQueryProfileJson();

	virtual void			BuildResult ( XQNode_t * pRoot, const CSphSchema & tSchema, const CSphVector<CSphString> & dZones );
	virtual cJSON *			LeakResultAsJson();
	virtual const char *	GetResultAsStr() const;

private:
	cJSON *					m_pResult {nullptr};
};


CSphQueryProfileJson::~CSphQueryProfileJson()
{
	cJSON_Delete(m_pResult);
}


void CSphQueryProfileJson::BuildResult ( XQNode_t * pRoot, const CSphSchema & tSchema, const CSphVector<CSphString> & /*dZones*/ )
{
	assert ( !m_pResult );
	m_pResult = sphBuildProfileJson ( pRoot, tSchema );
}


cJSON *	CSphQueryProfileJson::LeakResultAsJson()
{
	cJSON * pTmp = m_pResult;
	m_pResult = nullptr;
	return pTmp;
}


const char * CSphQueryProfileJson::GetResultAsStr() const
{
	assert ( 0 && "Not implemented" );
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
class JsonRequestBuilder_c : public IRequestBuilder_t
{
public:
	JsonRequestBuilder_c ( const CSphString & sQuery, const SqlStmt_t & /*tStmt*/, ESphHttpEndpoint eEndpoint )
		: m_eEndpoint ( eEndpoint )
	{
		// fixme: we can implement replacing indexes in a string (without parsing) if it becomes a performance issue
		m_pQuery = cJSON_Parse ( sQuery.cstr() );
		assert ( m_pQuery );
	}

	~JsonRequestBuilder_c()
	{
		cJSON_Delete ( m_pQuery );
	}

	void BuildRequest ( const AgentConn_t & tAgent, ISphOutputBuffer & tOut ) const override
	{
		// replace "index" value in the json query
		cJSON_DeleteItemFromObject ( m_pQuery, "index" );
		cJSON_AddStringToObject ( m_pQuery, "index", tAgent.m_tDesc.m_sIndexes.cstr() );
		CSphString sRequest = sphJsonToString ( m_pQuery );
		CSphString sEndpoint = sphHttpEndpointToStr ( m_eEndpoint );

		tOut.SendWord ( SEARCHD_COMMAND_JSON );
		tOut.SendWord ( VER_COMMAND_JSON );
		tOut.SendInt ( sEndpoint.Length() + sRequest.Length() + 8 );

		tOut.SendString ( sEndpoint.cstr() );
		tOut.SendString ( sRequest.cstr() );
	}

private:
	cJSON *				m_pQuery {nullptr};
	ESphHttpEndpoint	m_eEndpoint {SPH_HTTP_ENDPOINT_TOTAL};
};


class JsonReplyParser_c : public IReplyParser_t
{
public:
	JsonReplyParser_c ( int & iAffected, int & iWarnings )
		: m_iAffected ( iAffected )
		, m_iWarnings ( iWarnings )
	{}

	virtual bool ParseReply ( MemInputBuffer_c & tReq, AgentConn_t & ) const
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
	HttpHandler_c ( const CSphString & sQuery, int iCID, bool bNeedHttpResponse )
		: m_sQuery { sQuery }
		, m_iCID ( iCID )
		, m_bNeedHttpResponse ( bNeedHttpResponse )
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
	int					m_iCID {0};
	bool				m_bNeedHttpResponse {false};
	CSphVector<BYTE>	m_dData;

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

	void BuildReply ( const CSphString & sResult, ESphHttpStatus eStatus )
	{
		if ( m_bNeedHttpResponse )
			HttpBuildReply ( m_dData, eStatus, sResult.cstr(), sResult.Length(), false );
		else
		{
			m_dData.Resize ( sResult.Length()+1 );
			memcpy ( m_dData.Begin(), sResult.cstr(), m_dData.GetLength() );
		}
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
	HttpSearchHandler_c ( const CSphString & sQuery, const OptionsHash_t & tOptions, int iCID, bool bNeedHttpResponse )
		: HttpHandler_c ( sQuery, iCID, bNeedHttpResponse )
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
		CSphScopedPtr<ISphSearchHandler> tHandler ( sphCreateSearchHandler ( 1, pQueryParser, m_eQueryType, true, m_iCID ) );

		CSphQueryProfileJson tProfile;
		if ( m_bProfile )
			tHandler->SetProfile ( &tProfile );

		tHandler->SetQuery ( 0, m_tQuery );

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
	HttpSearchHandler_Plain_c ( const CSphString & sQuery, const OptionsHash_t & tOptions, int iCID, bool bNeedHttpResponse )
		: HttpSearchHandler_c ( sQuery, tOptions, iCID, bNeedHttpResponse )
	{}

protected:
	QueryParser_i * PreParseQuery() override
	{
		CSphString sError;
		ParseSearchOptions ( m_tOptions, m_tQuery );
		if ( !m_tQuery.ParseSelectList ( sError ) )
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
		CSphString sResult;
		sResult.Adopt ( tResBuilder.Leak() );
		return sResult;
	}
};


class HttpSearchHandler_SQL_c : public HttpSearchHandler_Plain_c
{
public:
	HttpSearchHandler_SQL_c ( const CSphString & sQuery, const OptionsHash_t & tOptions, int iCID, bool bNeedHttpResponse )
		: HttpSearchHandler_Plain_c ( sQuery, tOptions, iCID, bNeedHttpResponse )
	{}

protected:
	QueryParser_i * PreParseQuery() override
	{
		const CSphString * pRawQl = m_tOptions ( "query" );
		if ( !pRawQl || pRawQl->IsEmpty() )
		{
			ReportError ( "query missing", SPH_HTTP_STATUS_400 );
			return NULL;
		}

		CSphString sError;
		CSphVector<SqlStmt_t> dStmt;
		if ( !sphParseSqlQuery ( pRawQl->cstr(), pRawQl->Length(), dStmt, sError, SPH_COLLATION_DEFAULT ) )
		{
			ReportError ( sError.cstr(), SPH_HTTP_STATUS_400 );
			return NULL;
		}

		m_tQuery = dStmt[0].m_tQuery;
		if ( dStmt.GetLength()>1 )
		{
			ReportError ( "multiple queries not supported", SPH_HTTP_STATUS_501 );
			return NULL;
		} else if ( dStmt[0].m_eStmt!=STMT_SELECT )
		{
			ReportError ( "only SELECT queries are supported", SPH_HTTP_STATUS_501 );
			return NULL;
		}

		m_eQueryType = QUERY_SQL;

		return sphCreatePlainQueryParser();
	}
};


class HttpHandler_JsonSearch_c : public HttpSearchHandler_c
{
public:	
	HttpHandler_JsonSearch_c ( const CSphString & sQuery, const OptionsHash_t & tOptions, int iCID, bool bNeedHttpResponse )
		: HttpSearchHandler_c ( sQuery, tOptions, iCID, bNeedHttpResponse )
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
	bool ProcessInsert ( SqlStmt_t & tStmt, SphDocID_t tDocId, bool bReplace, cJSON * & pResult )
	{
		CSphSessionAccum tAcc ( false );
		CSphString sWarning;
		HttpErrorReporter_c tReporter;
		sphHandleMysqlInsert ( tReporter, tStmt, bReplace, true, sWarning, tAcc, SPH_COLLATION_DEFAULT );

		if ( tReporter.IsError() )
			pResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
		else
			pResult = sphEncodeInsertResultJson ( tStmt.m_sIndex.cstr(), bReplace, tDocId );

		return !tReporter.IsError();
	}
};


class HttpHandler_JsonInsert_c : public HttpHandler_c, public HttpJsonInsertTraits_c
{
public:
	HttpHandler_JsonInsert_c ( const CSphString & sQuery, bool bReplace, bool bNeedHttpResponse )
		: HttpHandler_c ( sQuery, 0, bNeedHttpResponse )
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

		cJSON * pResult = NULL;
		bool bResult = ProcessInsert ( tStmt, tDocId, m_bReplace, pResult );

		CSphString sResult = sphJsonToString ( pResult );
		BuildReply( sResult, bResult ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500 );

		cJSON_Delete ( pResult );
		return bResult;
	}

private:
	bool m_bReplace {false};
};


class HttpJsonUpdateTraits_c
{
protected:
	bool ProcessUpdate ( const char * szRawRequest, const SqlStmt_t & tStmt, SphDocID_t tDocId, int iCID, cJSON * & pResult )
	{
		HttpErrorReporter_c tReporter;
		CSphString sWarning;
		JsonParserFactory_c tFactory ( SPH_HTTP_ENDPOINT_JSON_UPDATE );
		sphHandleMysqlUpdate ( tReporter, tFactory, tStmt, szRawRequest, sWarning, iCID );

		if ( tReporter.IsError() )
			pResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
		else
			pResult = sphEncodeUpdateResultJson ( tStmt.m_sIndex.cstr(), tDocId, tReporter.GetAffectedRows() );

		return !tReporter.IsError();
	}
};


class HttpHandler_JsonUpdate_c : public HttpHandler_c, HttpJsonUpdateTraits_c
{
public:
	HttpHandler_JsonUpdate_c ( const CSphString & sQuery, int iCID, bool bNeedHttpResponse )
		: HttpHandler_c ( sQuery, iCID, bNeedHttpResponse )
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

		cJSON * pResult = NULL;
		bool bResult = ProcessQuery ( tStmt, tDocId, pResult );

		CSphString sResult = sphJsonToString ( pResult );
		BuildReply( sResult, bResult ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500 );

		cJSON_Delete ( pResult );
		return bResult;
	}

protected:
	virtual bool ParseQuery ( SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError )
	{
		return sphParseJsonUpdate ( m_sQuery.cstr(), tStmt, tDocId, sError );
	}

	virtual bool ProcessQuery ( const SqlStmt_t & tStmt, SphDocID_t tDocId, cJSON * & pResult )
	{
		return ProcessUpdate ( m_sQuery.cstr(), tStmt, tDocId, m_iCID, pResult );
	}
};


class HttpJsonDeleteTraits_c
{
protected:
	bool ProcessDelete ( const char * szRawRequest, const SqlStmt_t & tStmt, SphDocID_t tDocId, int iCID, cJSON * & pResult )
	{
		CSphSessionAccum tAcc ( false );
		HttpErrorReporter_c tReporter;
		CSphString sWarning;
		JsonParserFactory_c tFactory ( SPH_HTTP_ENDPOINT_JSON_DELETE );
		sphHandleMysqlDelete ( tReporter, tFactory, tStmt, szRawRequest, true, tAcc, iCID );

		if ( tReporter.IsError() )
			pResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
		else
			pResult = sphEncodeDeleteResultJson ( tStmt.m_sIndex.cstr(), tDocId, tReporter.GetAffectedRows() );

		return !tReporter.IsError();
	}
};


class HttpHandler_JsonDelete_c : public HttpHandler_JsonUpdate_c, public HttpJsonDeleteTraits_c
{
public:
	HttpHandler_JsonDelete_c ( const CSphString & sQuery, int iCID, bool bNeedHttpResponse )
		: HttpHandler_JsonUpdate_c ( sQuery, iCID, bNeedHttpResponse )
	{}

protected:
	bool ParseQuery ( SqlStmt_t & tStmt, SphDocID_t & tDocId, CSphString & sError ) override
	{
		return sphParseJsonDelete ( m_sQuery.cstr(), tStmt, tDocId, sError );
	}

	bool ProcessQuery ( const SqlStmt_t & tStmt, SphDocID_t tDocId, cJSON * & pResult ) override
	{
		return ProcessDelete ( m_sQuery.cstr(), tStmt, tDocId, m_iCID, pResult );
	}
};


class HttpHandler_JsonBulk_c : public HttpHandler_c, public HttpOptionsTraits_c, public HttpJsonInsertTraits_c, public HttpJsonUpdateTraits_c, public HttpJsonDeleteTraits_c
{
public:
	HttpHandler_JsonBulk_c ( const CSphString & sQuery, const OptionsHash_t & tOptions, int iCID, bool bNeedHttpResponse )
		: HttpHandler_c ( sQuery, iCID, bNeedHttpResponse )
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

		cJSON * pRoot = cJSON_CreateObject();
		cJSON * pItems = cJSON_CreateArray();
		assert ( pRoot && pItems );
		cJSON_AddItemToObject ( pRoot, "items", pItems );

		// fixme: we're modifying the original query at this point
		char * p = const_cast<char*>(m_sQuery.cstr());

		bool bResult = false;
		while ( *p )
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
				cJSON_Delete ( pRoot );
				return false;
			}

			cJSON * pResult = NULL;
			bResult = false;

			switch ( tStmt.m_eStmt )
			{
			case STMT_INSERT:
			case STMT_REPLACE:
				bResult = ProcessInsert ( tStmt, tDocId, tStmt.m_eStmt==STMT_REPLACE, pResult );
				break;

			case STMT_UPDATE:
				bResult = ProcessUpdate ( sQuery.cstr(), tStmt, tDocId, m_iCID, pResult );
				break;

			case STMT_DELETE:
				bResult = ProcessDelete ( sQuery.cstr(), tStmt, tDocId, m_iCID, pResult );
				break;

			default:
				ReportError ( "Unknown statement", SPH_HTTP_STATUS_400 );
				cJSON_Delete ( pRoot );
				return false;
			}

			AddResult ( pItems, sStmt, pResult );

			// no further than the first error
			if ( !bResult )
				break;

			while ( sphIsSpace(*p) )
				p++;
		}

		cJSON_AddBoolToObject ( pRoot, "errors", bResult ? 0 : 1 );

		CSphString sResult = sphJsonToString ( pRoot );
		BuildReply ( sResult, bResult ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500 );
		cJSON_Delete ( pRoot );

		return true;
	}

private:
	void AddResult ( cJSON * pRoot, CSphString & sStmt, cJSON * pResult )
	{
		assert ( pRoot && pResult );

		cJSON * pItem = cJSON_CreateObject();
		assert ( pItem );
		cJSON_AddItemToArray ( pRoot, pItem );

		cJSON_AddItemToObject( pItem, sStmt.cstr(), pResult );
	}
};


static HttpHandler_c * CreateHttpHandler ( ESphHttpEndpoint eEndpoint, const CSphString & sQuery, const OptionsHash_t & tOptions, int iCID, bool bNeedHttpResonse )
{
	switch ( eEndpoint )
	{
	case SPH_HTTP_ENDPOINT_SEARCH:
		return new HttpSearchHandler_Plain_c ( sQuery, tOptions, iCID, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_SQL:
		return new HttpSearchHandler_SQL_c ( sQuery, tOptions, iCID, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_JSON_SEARCH:
		return new HttpHandler_JsonSearch_c ( sQuery, tOptions, iCID, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_JSON_INDEX:
	case SPH_HTTP_ENDPOINT_JSON_CREATE:
	case SPH_HTTP_ENDPOINT_JSON_INSERT:
	case SPH_HTTP_ENDPOINT_JSON_REPLACE:
		return new HttpHandler_JsonInsert_c ( sQuery, eEndpoint==SPH_HTTP_ENDPOINT_JSON_INDEX || eEndpoint==SPH_HTTP_ENDPOINT_JSON_REPLACE, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_JSON_UPDATE:
		return new HttpHandler_JsonUpdate_c ( sQuery, iCID, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_JSON_DELETE:
		return new HttpHandler_JsonDelete_c ( sQuery, iCID, bNeedHttpResonse );

	case SPH_HTTP_ENDPOINT_JSON_BULK:
		return new HttpHandler_JsonBulk_c ( sQuery, tOptions, iCID, bNeedHttpResonse );

	default:
		break;
	}

	return nullptr;
}


bool sphProcessHttpQuery ( ESphHttpEndpoint eEndpoint, const CSphString & sQuery, const SmallStringHash_T<CSphString> & tOptions, int iCID, CSphVector<BYTE> & dResult, bool bNeedHttpResponse )
{
	CSphScopedPtr<HttpHandler_c> pHandler ( CreateHttpHandler ( eEndpoint, sQuery, tOptions, iCID, bNeedHttpResponse ) );
	if ( !pHandler.Ptr() )
		return false;

	pHandler->Process();
	dResult = std::move ( pHandler->GetResult() );
	return true;
}


bool sphLoopClientHttp ( const BYTE * pRequest, int iRequestLen, CSphVector<BYTE> & dResult, int iCID )
{
	HttpRequestParser_c tParser;
	if ( !tParser.Parse ( pRequest, iRequestLen ) )
	{
		HttpErrorReply ( dResult, SPH_HTTP_STATUS_400, tParser.GetError() );
		return tParser.GetKeepAlive();
	}

	ESphHttpEndpoint eEndpoint = tParser.GetEndpoint();
	if ( !sphProcessHttpQuery ( eEndpoint, tParser.GetBody(), tParser.GetOptions(), iCID, dResult, true ) )
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


const char * g_dEndpoints[] = { "index.html", "search", "sql", "json/search", "json/index", "json/create", "json/insert", "json/replace", "json/update", "json/delete", "json/bulk" };
STATIC_ASSERT ( sizeof(g_dEndpoints)/sizeof(g_dEndpoints[0])==SPH_HTTP_ENDPOINT_TOTAL, SPH_HTTP_ENDPOINT_SHOULD_BE_SAME_AS_SPH_HTTP_ENDPOINT_TOTAL );

ESphHttpEndpoint sphStrToHttpEndpoint ( const CSphString & sEndpoint )
{
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