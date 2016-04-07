//
// $Id$
//

//
// Copyright (c) 2001-2015, Andrew Aksyonoff
// Copyright (c) 2008-2015, Sphinx Technologies Inc
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

#include "http/http_parser.h"
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

typedef SphStringBuilder_T<EscapeJsonString_t> CSphStringBuilderJson;

static void AppendJsonKey ( const char * sName, CSphStringBuilderJson & tOut )
{
	tOut += "\"";
	tOut += sName;
	tOut += "\":";
}


static void EncodeResultJson ( const AggrResult_t & tRes, CSphStringBuilderJson & tOut )
{
	const CSphRsetSchema & tSchema = tRes.m_tSchema;
	CSphVector<BYTE> dTmp;

	int iAttrsCount = tSchema.GetAttrsCount();
	if ( iAttrsCount && sphIsSortStringInternal ( tSchema.GetAttr ( iAttrsCount-1 ).m_sName.cstr() ) )
	{
		for ( int i=iAttrsCount-1; i>=0 && sphIsSortStringInternal ( tSchema.GetAttr(i).m_sName.cstr() ); i-- )
			iAttrsCount = i;
	}

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
					const char * sSep = "";
					if ( pValues )
					{
						DWORD nValues = *pValues++;
						assert ( eAttrType==SPH_ATTR_UINT32SET || ( nValues%2 )==0 );
						bool bWide = ( eAttrType==SPH_ATTR_INT64SET );
						int iStep = ( bWide ? 2 : 1 );
						for ( ; nValues; nValues-=iStep, pValues+=iStep )
						{
							int64_t iVal = ( bWide ? MVA_UPSIZE ( pValues ) : *pValues );
							tOut.Appendf ( "%s" INT64_FMT, sSep, iVal );
							sSep = ",";
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
						sphJsonFieldFormat ( dTmp, pStrings+uOff, eJson, false );
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
		tOut.Appendf ( ",\"warning\":\"%s\"", tRes.m_sWarning.cstr() );

	tOut += "}";
}

enum ESphHttpStatus
{
	SPH_HTTP_STATUS_200,
	SPH_HTTP_STATUS_206,
	SPH_HTTP_STATUS_400,
	SPH_HTTP_STATUS_500,
	SPH_HTTP_STATUS_501,

	SPH_HTTP_STATUS_TOTAL
};

const char * g_dHttpStatus[] = { "200 OK", "206 Partial Content", "400 Bad Request", "500 Internal Server Error", "501 Not Implemented" };
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
	char sBuf[1024] = "{}";
	char sErr[1008] = "\0";
	if ( sTemplate && *sTemplate!='\0' )
	{
		va_list ap;
		va_start ( ap, sTemplate );
		vsnprintf ( sErr, sizeof(sErr), sTemplate, ap );
		va_end ( ap );
		snprintf ( sBuf, sizeof(sBuf), "{\"error\":\"%s\"}", sErr );
	}

	HttpBuildReply ( dData, eCode, sBuf, strnlen ( sBuf, sizeof(sBuf) ), false );
}

enum ESphHttpEndpoint
{
	SPH_HTTP_ENDPOINT_INDEX,
	SPH_HTTP_ENDPOINT_SEARCH,
	SPH_HTTP_ENDPOINT_SQL,

	SPH_HTTP_ENDPOINT_TOTAL,

	SPH_HTTP_ENDPOINT_MISSED,
	SPH_HTTP_ENDPOINT_DEFAULT = SPH_HTTP_ENDPOINT_MISSED
};

const char * g_sEndpoints[] = { "index.html", "search", "sql" };
STATIC_ASSERT ( sizeof(g_sEndpoints)/sizeof(g_sEndpoints[0])==SPH_HTTP_ENDPOINT_TOTAL, SPH_HTTP_ENDPOINT_SHOULD_BE_SAME_AS_SPH_HTTP_ENDPOINT_TOTAL );


struct HttpRequestParser_t : public ISphNoncopyable
{
	SmallStringHash_T<CSphString>	m_hOptions;
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

		int iLen = sCur - sLast;
		if ( *sCur=='&' )
		{
			sVal.SetBinary ( sLast, iLen );
			UriPercentReplace ( sName );
			UriPercentReplace ( sVal );
			m_hOptions.Add ( sVal, sName );
			sName = "";
			sVal = "";
		} else
		{
			sName.SetBinary ( sLast, iLen );
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
		int iLen = tUri.field_data[UF_PATH].len;
		if ( *sPath=='/' )
		{
			sPath++;
			iLen--;
		}
		ESphHttpEndpoint eEndpoint = ParseEndpointPath ( sPath, iLen );
		( (HttpRequestParser_t *)pParser->data )->m_eEndpoint = eEndpoint;
		if ( eEndpoint==SPH_HTTP_ENDPOINT_MISSED )
		{
			( (HttpRequestParser_t *)pParser->data )->m_sInvalidEndpoint.SetBinary ( sPath, iLen );
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
	( (HttpRequestParser_t *)pParser->data )->ParseList ( sAt, iLen );
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
		CSphStringBuilder sIndexPage;
		sIndexPage.Appendf ( g_sIndexPage, SPHINX_VERSION );
		HttpBuildReply ( dData, SPH_HTTP_STATUS_200, sIndexPage.cstr(), sIndexPage.Length(), true );
	} else
	{
		HttpErrorReply ( dData, SPH_HTTP_STATUS_501, "/%s - unsupported endpoint", sInvalidEndpoint.cstr() );
	}
}

static void HttpHandlerSearch ( bool bQL, const SmallStringHash_T<CSphString> & hOptions, int iCID, CSphVector<BYTE> & dData )
{
	CSphScopedPtr<ISphSearchHandler> tHandler ( sphCreateSearchHandler ( 1, true, true, iCID ) );
	CSphQuery * pQuery = tHandler->GetQuery ( 0 );

	if ( bQL )
	{
		const CSphString * pRawQl = hOptions ( "query" );
		if ( !pRawQl || pRawQl->IsEmpty() )
		{
			HttpErrorReply ( dData, SPH_HTTP_STATUS_400, "query missed" );
			return;
		}

		CSphString sError;
		CSphVector<SqlStmt_t> dStmt;
		if ( !sphParseSqlQuery ( pRawQl->cstr(), pRawQl->Length(), dStmt, sError, SPH_COLLATION_DEFAULT ) )
		{
			HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
			return;
		}

		*pQuery = dStmt[0].m_tQuery;
		if ( dStmt.GetLength()>1 )
		{
			HttpErrorReply ( dData, SPH_HTTP_STATUS_501, "multiple queries not supported" );
			return;
		} else if ( dStmt[0].m_eStmt!=STMT_SELECT )
		{
			HttpErrorReply ( dData, SPH_HTTP_STATUS_501, "only SELECT queries supported" );
			return;
		}
	} else
	{
		CSphString sError;
		ParseSearchOptions ( hOptions, *pQuery );
		if ( !pQuery->ParseSelectList ( sError ) )
		{
			HttpErrorReply ( dData, SPH_HTTP_STATUS_400, sError.cstr() );
			return;
		}

		if ( !pQuery->m_sSortBy.IsEmpty() )
			pQuery->m_eSort = SPH_SORT_EXTENDED;
	}

	// search
	tHandler->RunQueries();

	AggrResult_t * pRes = tHandler->GetResult ( 0 );
	if ( !pRes->m_sError.IsEmpty() )
	{
		HttpErrorReply ( dData, SPH_HTTP_STATUS_500, pRes->m_sError.cstr() );
		return;
	}

	// coping result set
	CSphStringBuilderJson tResBuilder;
	EncodeResultJson ( *pRes, tResBuilder );
	HttpBuildReply ( dData, SPH_HTTP_STATUS_200, tResBuilder.cstr(), tResBuilder.Length(), false );
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
	case SPH_HTTP_ENDPOINT_SQL:
		HttpHandlerSearch ( tParser.m_eEndpoint==SPH_HTTP_ENDPOINT_SQL, tParser.m_hOptions, iCID, dData );
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
