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
	const CSphRsetSchema & tSchema = tRes.m_tSchema;
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

			switch ( eAttrType )
			{
			case SPH_ATTR_FLOAT:
				tOut.Appendf ( "%f", tMatch.GetAttrFloat(tLoc) );
				break;

			case SPH_ATTR_INT64SET:
			case SPH_ATTR_UINT32SET:
				{
					tOut += "[";
					assert ( tMatch.GetAttr ( tLoc )==0 || tRes.m_dTag2Pools [ tMatch.m_iTag ].m_pMva || ( MVA_DOWNSIZE ( tMatch.GetAttr ( tLoc ) ) & MVA_ARENA_FLAG ) );
					const PoolPtrs_t & tPools = tRes.m_dTag2Pools [ tMatch.m_iTag ];
					const DWORD * pValues = tMatch.GetAttrMVA ( tLoc, tPools.m_pMva, tPools.m_bArenaProhibit );
					const char * sSeparator = "";
					if ( pValues )
					{
						DWORD nValues = *pValues++;
						assert ( eAttrType==SPH_ATTR_UINT32SET || ( nValues%2 )==0 );
						bool bWide = ( eAttrType==SPH_ATTR_INT64SET );
						int iStep = ( bWide ? 2 : 1 );
						for ( ; nValues; nValues-=iStep, pValues+=iStep )
						{
							int64_t iVal = ( bWide ? MVA_UPSIZE ( pValues ) : *pValues );
							tOut.Appendf ( "%s" INT64_FMT, sSeparator, iVal );
							sSeparator = ",";
						}
					}
					tOut += "]";
				}
				break;

			case SPH_ATTR_STRING:
			case SPH_ATTR_JSON:
				{
					const BYTE * pStrings = tRes.m_dTag2Pools [ tMatch.m_iTag ].m_pStrings;
					bool bStr = ( eAttrType==SPH_ATTR_STRING );

					// get that string
					const BYTE * pStr = NULL;
					int iLen = 0;
					DWORD uOffset = (DWORD) tMatch.GetAttr ( tLoc );
					assert ( !uOffset || pStrings );
					if ( uOffset )
						iLen = sphUnpackStr ( pStrings+uOffset, &pStr );

					if ( eAttrType==SPH_ATTR_JSON )
					{
						// no object at all? return NULL
						if ( !pStr )
						{
							tOut += "null";
							break;
						}
						dTmp.Resize ( 0 );
						sphJsonFormat ( dTmp, pStr );
						if ( !dTmp.GetLength() )
						{
							// empty string (no objects) - return NULL
							// (canonical "{}" and "[]" are handled by sphJsonFormat)
							tOut += "null";
							break;
						}
					} else
					{
						dTmp.Resize ( iLen );
						memcpy ( dTmp.Begin(), pStr, iLen );
					}

					dTmp.Add ( '\0' );
					if ( bStr )
					{
						tOut += "\"";
						tOut.AppendEscaped ( (const char *)dTmp.Begin(), true, false );
						tOut += "\"";
					} else
					{
						tOut += (const char *)dTmp.Begin();
					}
				}
				break;

			case SPH_ATTR_STRINGPTR:
				{
					const char * pString = (const char *)tMatch.GetAttr ( tLoc );
					int iLen = pString && *pString ? strlen ( pString ) : 0;
					dTmp.Resize ( iLen+1 );
					memcpy ( dTmp.Begin(), pString, iLen );
					dTmp[iLen] = '\0';
					tOut += "\"";
					tOut.AppendEscaped ( ( const char *)dTmp.Begin(), true, false );
					tOut += "\"";
				}
				break;

			case SPH_ATTR_FACTORS:
			case SPH_ATTR_FACTORS_JSON:
				{
					const unsigned int * pFactors = (unsigned int *)tMatch.GetAttr ( tLoc );
					if ( pFactors )
					{
						bool bStr = ( eAttrType==SPH_ATTR_FACTORS );
						dTmp.Resize ( 0 );
						sphFormatFactors ( dTmp, pFactors, !bStr );
						dTmp.Add ( '\0' );
						if ( bStr )
						{
							tOut += "\"";
							tOut.AppendEscaped ( (const char *)dTmp.Begin(), true, false );
							tOut += "\"";
						} else
						{
							tOut += (const char *)dTmp.Begin();
						}
					}
				}
				break;

			case SPH_ATTR_JSON_FIELD:
				{
					uint64_t uTypeOffset = tMatch.GetAttr ( tLoc );
					ESphJsonType eJson = ESphJsonType ( uTypeOffset>>32 );
					DWORD uOff = (DWORD)uTypeOffset;
					if ( !uOff || eJson==JSON_NULL )
					{
						// no key found - NULL value
						tOut += "null";
					} else
					{
						// send string to client
						dTmp.Resize ( 0 );
						const BYTE * pStrings = tRes.m_dTag2Pools [ tMatch.m_iTag ].m_pStrings;
						sphJsonFieldFormat ( dTmp, pStrings+uOff, eJson, true );
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

static void HttpErrorReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * sTemplate, ... )
{
	cJSON * pError = cJSON_CreateObject();
	assert ( pError );

	char sErr[1008] = "\0";
	if ( sTemplate && *sTemplate!='\0' )
	{
		va_list ap;
		va_start ( ap, sTemplate );
		vsnprintf ( sErr, sizeof(sErr), sTemplate, ap );
		va_end ( ap );
		
		cJSON_AddStringToObject ( pError, "error", sErr );
	}

	CSphString sJsonError;
	sJsonError.Adopt ( cJSON_Print ( pError ) );
	cJSON_Delete ( pError );

	HttpBuildReply ( dData, eCode, sJsonError.cstr(), sJsonError.Length(), false );
}

enum ESphHttpEndpoint
{
	SPH_HTTP_ENDPOINT_INDEX,
	SPH_HTTP_ENDPOINT_SEARCH,
	SPH_HTTP_ENDPOINT_SQL,
	SPH_HTTP_ENDPOINT_JSON_SEARCH,
	SPH_HTTP_ENDPOINT_JSON_INDEX,
	SPH_HTTP_ENDPOINT_JSON_CREATE,
	SPH_HTTP_ENDPOINT_JSON_INSERT,
	SPH_HTTP_ENDPOINT_JSON_REPLACE,
	SPH_HTTP_ENDPOINT_JSON_UPDATE,
	SPH_HTTP_ENDPOINT_JSON_DELETE,
	SPH_HTTP_ENDPOINT_JSON_BULK,

	SPH_HTTP_ENDPOINT_TOTAL,

	SPH_HTTP_ENDPOINT_MISSED,
	SPH_HTTP_ENDPOINT_DEFAULT = SPH_HTTP_ENDPOINT_MISSED
};

const char * g_sEndpoints[] = { "index.html", "search", "sql", "json/search", "json/index", "json/create", "json/insert", "json/replace", "json/update", "json/delete", "json/bulk" };
STATIC_ASSERT ( sizeof(g_sEndpoints)/sizeof(g_sEndpoints[0])==SPH_HTTP_ENDPOINT_TOTAL, SPH_HTTP_ENDPOINT_SHOULD_BE_SAME_AS_SPH_HTTP_ENDPOINT_TOTAL );


struct HttpRequestParser_t : public ISphNoncopyable
{
	SmallStringHash_T<CSphString>	m_hOptions;
	CSphString						m_sRawBody;

	ESphHttpEndpoint				m_eEndpoint;
	bool							m_bKeepAlive;

	CSphString						m_sCurField;

	const char *					m_sError;
	CSphString						m_sInvalidEndpoint;

	HttpRequestParser_t ();
	~HttpRequestParser_t () {}

	bool Parse ( const CSphVector<BYTE> & dData );
	bool ParseList ( const char * sAt, int iLen );

	static int ParserUrl ( http_parser * pParser, const char * sAt, size_t iLen );
	static int ParserHeaderField ( http_parser * pParser, const char * sAt, size_t iLen );
	static int ParserHeaderValue ( http_parser * pParser, const char * sAt, size_t iLen );
	static int ParserBody ( http_parser * pParser, const char * sAt, size_t iLen );
};

HttpRequestParser_t::HttpRequestParser_t ()
{
	m_eEndpoint = SPH_HTTP_ENDPOINT_DEFAULT;
	m_bKeepAlive = false;
	m_sError = NULL;
}

bool HttpRequestParser_t::Parse ( const CSphVector<BYTE> & dData )
{
	http_parser_settings tParserSettings;
	http_parser_settings_init ( &tParserSettings );
	tParserSettings.on_url = HttpRequestParser_t::ParserUrl;
	tParserSettings.on_header_field = HttpRequestParser_t::ParserHeaderField;
	tParserSettings.on_header_value = HttpRequestParser_t::ParserHeaderValue;
	tParserSettings.on_body = HttpRequestParser_t::ParserBody;

	http_parser tParser;
	http_parser_init ( &tParser, HTTP_REQUEST );
	tParser.data = this;
	int iParsed = http_parser_execute ( &tParser, &tParserSettings, (const char *)dData.Begin(), dData.GetLength() );

	if ( iParsed!=dData.GetLength() )
	{
		m_sError = http_errno_description ( (http_errno)tParser.http_errno );
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


bool HttpRequestParser_t::ParseList ( const char * sAt, int iLen )
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

static ESphHttpEndpoint ParseEndpointPath ( const char * sPath, int iLen )
{
	if ( !iLen )
		return SPH_HTTP_ENDPOINT_INDEX;

	for ( int i=0; i<SPH_HTTP_ENDPOINT_TOTAL; i++ )
	{
		if ( strncmp ( sPath, g_sEndpoints[i], Min ( iLen, (int)strlen ( g_sEndpoints[i] ) ) )==0 )
			return (ESphHttpEndpoint)i;
	}

	return SPH_HTTP_ENDPOINT_MISSED;
}

int HttpRequestParser_t::ParserUrl ( http_parser * pParser, const char * sAt, size_t iLen )
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
		ESphHttpEndpoint eEndpoint = ParseEndpointPath ( sPath, iPathLen );
		( (HttpRequestParser_t *)pParser->data )->m_eEndpoint = eEndpoint;
		if ( eEndpoint==SPH_HTTP_ENDPOINT_MISSED )
		{
			( (HttpRequestParser_t *)pParser->data )->m_sInvalidEndpoint.SetBinary ( sPath, iPathLen );
		}
	}

	if ( ( tUri.field_set & uQuery )!=0 )
		( (HttpRequestParser_t *)pParser->data )->ParseList ( sAt + tUri.field_data[UF_QUERY].off, tUri.field_data[UF_QUERY].len );

	return 0;
}

int HttpRequestParser_t::ParserHeaderField ( http_parser * pParser, const char * sAt, size_t iLen )
{
	assert ( pParser->data );
	( (HttpRequestParser_t *)pParser->data )->m_sCurField.SetBinary ( sAt, iLen );
	return 0;
}

int HttpRequestParser_t::ParserHeaderValue ( http_parser * pParser, const char * sAt, size_t iLen )
{
	assert ( pParser->data );
	CSphString sVal;
	sVal.SetBinary ( sAt, iLen );
	HttpRequestParser_t * pHttpParser = (HttpRequestParser_t *)pParser->data;
	pHttpParser->m_hOptions.Add ( sVal, pHttpParser->m_sCurField );
	pHttpParser->m_sCurField = "";
	return 0;
}

int HttpRequestParser_t::ParserBody ( http_parser * pParser, const char * sAt, size_t iLen )
{
	assert ( pParser->data );
	HttpRequestParser_t * pHttpParser = (HttpRequestParser_t *)pParser->data;
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
"<title>Sphinx</title>"
"</head>"
"<body>"
"<h1>Sphinx daemon</h1>"
"<p>%s</p>"
"</body>"
"</html>";

static void HttpHandlerPage ( bool bPage, const CSphString & sInvalidEndpoint, CSphVector<BYTE> & dData )
{
	if ( bPage )
	{
		StringBuilder_c sIndexPage;
		sIndexPage.Appendf ( g_sIndexPage, SPHINX_VERSION );
		HttpBuildReply ( dData, SPH_HTTP_STATUS_200, sIndexPage.cstr(), sIndexPage.Length(), true );
	} else
	{
		HttpErrorReply ( dData, SPH_HTTP_STATUS_501, "/%s - unsupported endpoint", sInvalidEndpoint.cstr() );
	}
}

struct HttpQuerySettings_t
{
	bool m_bProfile;
	bool m_bSphinxQL;
	bool m_bAttrHighlight;

	HttpQuerySettings_t ()
		: m_bProfile ( false )
		, m_bSphinxQL ( false )
		, m_bAttrHighlight ( false )
	{}
};

static QueryParser_i * ParseQuerySql ( CSphQuery & tQuery, const HttpRequestParser_t & tParser, CSphVector<BYTE> & dData, HttpQuerySettings_t & tSettings, CSphString & /*sWarning*/ )
{
	const CSphString * pRawQl = tParser.m_hOptions ( "query" );
	if ( !pRawQl || pRawQl->IsEmpty() )
	{
		HttpErrorReply ( dData, SPH_HTTP_STATUS_400, "query missed" );
		return NULL;
	}

	CSphString sError;
	CSphVector<SqlStmt_t> dStmt;
	if ( !sphParseSqlQuery ( pRawQl->cstr(), pRawQl->Length(), dStmt, sError, SPH_COLLATION_DEFAULT ) )
	{
		HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
		return NULL;
	}

	tQuery = dStmt[0].m_tQuery;
	if ( dStmt.GetLength()>1 )
	{
		HttpErrorReply ( dData, SPH_HTTP_STATUS_501, "multiple queries not supported" );
		return NULL;
	} else if ( dStmt[0].m_eStmt!=STMT_SELECT )
	{
		HttpErrorReply ( dData, SPH_HTTP_STATUS_501, "only SELECT queries supported" );
		return NULL;
	}

	tSettings.m_bSphinxQL = true;

	return sphCreatePlainQueryParser();
}


static QueryParser_i * ParseQueryPlain ( CSphQuery & tQuery, const HttpRequestParser_t & tParser, CSphVector<BYTE> & dData, HttpQuerySettings_t & tSettings, CSphString & /*sWarning*/ )
{
	CSphString sError;
	ParseSearchOptions ( tParser.m_hOptions, tQuery );
	if ( !tQuery.ParseSelectList ( sError ) )
	{
		HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
		return NULL;
	}

	if ( !tQuery.m_sSortBy.IsEmpty() )
		tQuery.m_eSort = SPH_SORT_EXTENDED;

	tSettings.m_bSphinxQL = true;

	return sphCreatePlainQueryParser();
}


static QueryParser_i * ParseQueryJson ( CSphQuery & tQuery, const HttpRequestParser_t & tParser, CSphVector<BYTE> & dData, HttpQuerySettings_t & tSettings, CSphString & sWarning )
{
	CSphString sError;
	if ( !sphParseJsonQuery ( tParser.m_sRawBody.cstr(), tQuery, tSettings.m_bProfile, tSettings.m_bAttrHighlight, sError, sWarning ) )
	{
		sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
		HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
		return NULL;
	}

	tSettings.m_bSphinxQL = true;

	return sphCreateJsonQueryParser();
}


void EncodeResultPlain ( const AggrResult_t & tRes, const CSphQuery & , CSphQueryProfile *, bool , CSphString & sResult )
{
	JsonEscapedBuilder tResBuilder;
	EncodeResultJson ( tRes, tResBuilder );
	sResult.Adopt ( tResBuilder.Leak() );
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



using ParseQueryFunc_fn = QueryParser_i * (CSphQuery &, const HttpRequestParser_t &, CSphVector<BYTE> &, HttpQuerySettings_t &, CSphString & );
using ResultEncodeFunc_fn = void ( const AggrResult_t &, const CSphQuery &, CSphQueryProfile *, bool, CSphString & );

static void HttpHandlerSearch ( ParseQueryFunc_fn * pParseQueryFunc, ResultEncodeFunc_fn * pResultEncodeFunc, const HttpRequestParser_t & tParser, int iCID, CSphVector<BYTE> & dData )
{
	assert ( pParseQueryFunc && pResultEncodeFunc );

	HttpQuerySettings_t tQuerySettings;
	CSphQuery tQuery;
	CSphString sWarning;
	QueryParser_i * pQueryParser = pParseQueryFunc ( tQuery, tParser, dData, tQuerySettings, sWarning );
	if ( !pQueryParser )
		return;

	CSphQueryProfileJson tProfile;

	tQuery.m_pQueryParser = pQueryParser;
	CSphScopedPtr<ISphSearchHandler> tHandler ( sphCreateSearchHandler ( 1, pQueryParser, tQuerySettings.m_bSphinxQL, true, iCID ) );

	if ( tQuerySettings.m_bProfile )
		tHandler->SetProfile ( &tProfile );

	tHandler->SetQuery ( 0, tQuery );

	// search
	tHandler->RunQueries();

	if ( tQuerySettings.m_bProfile )
		tProfile.Stop();

	AggrResult_t * pRes = tHandler->GetResult ( 0 );
	if ( !pRes->m_sError.IsEmpty() )
	{
		HttpErrorReply ( dData, SPH_HTTP_STATUS_500, pRes->m_sError.cstr() );
		return;
	}

	// fixme: handle more than one warning at once?
	if ( pRes->m_sWarning.IsEmpty() )
		pRes->m_sWarning = sWarning;
	
	CSphString sResult;
	pResultEncodeFunc ( *pRes, tQuery, tQuerySettings.m_bProfile ? &tProfile : NULL, tQuerySettings.m_bAttrHighlight, sResult );
	HttpBuildReply ( dData, SPH_HTTP_STATUS_200, sResult.cstr(), sResult.Length(), false );
}


class HttpErrorReporter_c : public StmtErrorReporter_i
{
public:
	virtual void	Ok ( int iAffectedRows, const CSphString & /*sWarning*/ ) { m_iAffected = iAffectedRows; }
	virtual void	Ok ( int iAffectedRows, int /*nWarnings*/ ) { m_iAffected = iAffectedRows; }
	virtual void	Error ( const char * sStmt, const char * sError, MysqlErrors_e iErr );

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


void CreateHttpResponse ( bool bResult, cJSON * pResult, CSphVector<BYTE> & dData )
{
	assert ( pResult );

	CSphString sResult;
	sResult.Adopt ( cJSON_Print ( pResult ) );
	HttpBuildReply ( dData, bResult ? SPH_HTTP_STATUS_200 : SPH_HTTP_STATUS_500, sResult.cstr(), sResult.Length(), false );
}


static bool ProcessInsert ( SqlStmt_t & tStmt, SphDocID_t tDocId, bool bReplace, cJSON * & pResult )
{
	CSphSessionAccum tAcc ( false );
	CSphString sWarning;
	HttpErrorReporter_c tReporter;
	sphHandleMysqlInsert ( tReporter, tStmt, bReplace, true, sWarning, tAcc );

	if ( tReporter.IsError() )
		pResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
	else
		pResult = sphEncodeInsertResultJson ( tStmt.m_sIndex.cstr(), bReplace, tDocId );

	return !tReporter.IsError();
}


static void HttpHandlerInsert ( const HttpRequestParser_t & tParser, CSphVector<BYTE> & dData, bool bReplace )
{
	SqlStmt_t tStmt;
	SphDocID_t tDocId;
	CSphString sError;
	if ( !sphParseJsonInsert ( tParser.m_sRawBody.cstr(), tStmt, tDocId, bReplace, sError ) )
	{
		sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
		HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
		return;
	}

	cJSON * pResult = NULL;
	bool bResult = ProcessInsert ( tStmt, tDocId, bReplace, pResult );
	CreateHttpResponse ( bResult, pResult, dData );
	cJSON_Delete ( pResult );
}


static bool ProcessUpdate ( SqlStmt_t & tStmt, SphDocID_t tDocId, int iCID, cJSON * & pResult )
{
	HttpErrorReporter_c tReporter;
	CSphString sWarning;
	sphHandleMysqlUpdate ( tReporter, tStmt, "", sWarning, iCID );

	if ( tReporter.IsError() )
		pResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
	else
		pResult = sphEncodeUpdateResultJson ( tStmt.m_sIndex.cstr(), tDocId, tReporter.GetAffectedRows() );

	return !tReporter.IsError();
}


static void HttpHandlerUpdate ( const HttpRequestParser_t & tParser, int iCID, CSphVector<BYTE> & dData )
{
	SqlStmt_t tStmt;
	SphDocID_t tDocId;
	CSphString sError;
	if ( !sphParseJsonUpdate ( tParser.m_sRawBody.cstr(), tStmt, tDocId, sError ) )
	{
		sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
		HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
		return;
	}

	cJSON * pResult = NULL;
	bool bResult = ProcessUpdate ( tStmt, tDocId, iCID, pResult );
	CreateHttpResponse ( bResult, pResult, dData );
	cJSON_Delete ( pResult );
}


static bool ProcessDelete ( SqlStmt_t & tStmt, SphDocID_t tDocId, int iCID, cJSON * & pResult )
{
	CSphSessionAccum tAcc ( false );
	HttpErrorReporter_c tReporter;
	CSphString sWarning;
	sphHandleMysqlDelete ( tReporter, tStmt, "", true, tAcc, iCID );

	if ( tReporter.IsError() )
		pResult = sphEncodeInsertErrorJson ( tStmt.m_sIndex.cstr(), tReporter.GetError() );
	else
		pResult = sphEncodeDeleteResultJson ( tStmt.m_sIndex.cstr(), tDocId, tReporter.GetAffectedRows() );

	return !tReporter.IsError();
}


static void HttpHandlerDelete ( const HttpRequestParser_t & tParser, int iCID, CSphVector<BYTE> & dData )
{
	SqlStmt_t tStmt;
	SphDocID_t tDocId;
	CSphString sError;
	if ( !sphParseJsonDelete ( tParser.m_sRawBody.cstr(), tStmt, tDocId, sError ) )
	{
		sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
		HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
		return;
	}

	cJSON * pResult = NULL;
	bool bResult = ProcessDelete ( tStmt, tDocId, iCID, pResult );
	CreateHttpResponse ( bResult, pResult, dData );
	cJSON_Delete ( pResult );
}


static void AddResultToBulk ( cJSON * pRoot, CSphString & sStmt, cJSON * pResult )
{
	assert ( pRoot && pResult );

	cJSON * pItem = cJSON_CreateObject();
	assert ( pItem );
	cJSON_AddItemToArray ( pRoot, pItem );

	cJSON_AddItemToObject( pItem, sStmt.cstr(), pResult );
}


static void HttpHandlerBulk ( const HttpRequestParser_t & tParser, int iCID, CSphVector<BYTE> & dData )
{
	if ( !tParser.m_hOptions.Exists ("Content-Type") )
	{
		CSphString sError = "Content-Type must be set";
		HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
	}

	if ( tParser.m_hOptions["Content-Type"].ToLower() != "application/x-ndjson" )
	{
		CSphString sError = "Content-Type must be application/x-ndjson";
		HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
	}

	cJSON * pRoot = cJSON_CreateObject();
	cJSON * pItems = cJSON_CreateArray();
	assert ( pRoot && pItems );
	cJSON_AddItemToObject ( pRoot, "items", pItems );

	char * p = const_cast<char*>(tParser.m_sRawBody.cstr());

	bool bResult = false;
	while ( *p )
	{
		char * szStmt = p;
		while ( *p && *p!='\r' && *p!='\n' )
			p++;

		if ( p-szStmt==0 )
			break;

		*p++ = '\0';
		SqlStmt_t tStmt;
		SphDocID_t tDocId;
		CSphString sStmt;
		CSphString sError;
		if ( !sphParseJsonStatement ( szStmt, tStmt, sStmt, tDocId, sError ) )
		{
			sError.SetSprintf( "Error parsing json query: %s", sError.cstr() );
			HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
			cJSON_Delete ( pRoot );
			return;
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
			bResult = ProcessUpdate ( tStmt, tDocId, iCID, pResult );
			break;

		case STMT_DELETE:
			bResult = ProcessDelete ( tStmt, tDocId, iCID, pResult );
			break;

		default:
			sError = "Unknown statement";
			HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
			cJSON_Delete ( pRoot );
			return;
		}

		AddResultToBulk ( pItems, sStmt, pResult );
		
		// no further than the first error
		if ( !bResult )
			break;

		while ( *p=='\r' || *p=='\n' )
			p++;
	}

	cJSON_AddBoolToObject ( pRoot, "errors", bResult ? 0 : 1 );

	CreateHttpResponse ( bResult, pRoot, dData );
	cJSON_Delete ( pRoot );
}


bool sphLoopClientHttp ( CSphVector<BYTE> & dData, int iCID )
{
	HttpRequestParser_t tParser;
	if ( !tParser.Parse ( dData ) )
	{
		HttpErrorReply ( dData, SPH_HTTP_STATUS_400, tParser.m_sError );
		return tParser.m_bKeepAlive;
	}

	switch ( tParser.m_eEndpoint )
	{
	case SPH_HTTP_ENDPOINT_SEARCH:
		HttpHandlerSearch ( ParseQueryPlain, EncodeResultPlain, tParser, iCID, dData );
		break;

	case SPH_HTTP_ENDPOINT_SQL:
		HttpHandlerSearch ( ParseQuerySql, EncodeResultPlain, tParser, iCID, dData );
		break;

	case SPH_HTTP_ENDPOINT_JSON_SEARCH:
		HttpHandlerSearch ( ParseQueryJson, sphEncodeResultJson, tParser, iCID, dData );
		break;

	case SPH_HTTP_ENDPOINT_JSON_INDEX:
	case SPH_HTTP_ENDPOINT_JSON_CREATE:
	case SPH_HTTP_ENDPOINT_JSON_INSERT:
	case SPH_HTTP_ENDPOINT_JSON_REPLACE:
		HttpHandlerInsert ( tParser, dData, tParser.m_eEndpoint==SPH_HTTP_ENDPOINT_JSON_INDEX || tParser.m_eEndpoint==SPH_HTTP_ENDPOINT_JSON_REPLACE );
		break;

	case SPH_HTTP_ENDPOINT_JSON_UPDATE:
		HttpHandlerUpdate ( tParser, iCID, dData );
		break;

	case SPH_HTTP_ENDPOINT_JSON_DELETE:
		HttpHandlerDelete ( tParser, iCID, dData );
		break;

	case SPH_HTTP_ENDPOINT_JSON_BULK:
		HttpHandlerBulk ( tParser, iCID, dData );
		break;

	case SPH_HTTP_ENDPOINT_INDEX:
		HttpHandlerPage ( true, tParser.m_sInvalidEndpoint, dData );
		break;

	case SPH_HTTP_ENDPOINT_MISSED:
	default:
		HttpHandlerPage ( false, tParser.m_sInvalidEndpoint, dData );
	}

	return tParser.m_bKeepAlive;
}


void sphHttpErrorReply ( CSphVector<BYTE> & dData, ESphHttpStatus eCode, const char * szError )
{
	HttpErrorReply( dData, eCode, "%s", szError );
}