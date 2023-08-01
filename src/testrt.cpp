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

#include "sphinxrt.h"
#include "accumulator.h"
#include "binlog.h"
#include "sphinxutils.h"
#include "sphinxsort.h"
#include "searchdaemon.h"
#include "indexing_sources/source_mysql.h"

#if HAVE_RTESTCONFIG_H
#include "rtestconfig.h"
#else
const char * rtestconfig = "error";
#endif

#if !defined (DATAFLD)
#define DATAFLD "data/"
#endif

#if _WIN32
#include "psapi.h"
#endif

int			COMMIT_STEP = 1;
float		g_fTotalMB = 0.0f;

void SetupIndexing ( CSphSource * pSrc )
{
	CSphString sError;
	if ( !pSrc->Connect ( sError ) )
		sphDie ( "connect failed: %s", sError.cstr() );
	if ( !pSrc->IterateStart ( sError ) )
		sphDie ( "iterate-start failed: %s", sError.cstr() );
}


void DoSearch ( CSphIndex * pIndex )
{
	printf ( "---\nsearching... " );

	CSphQuery tQuery;
	AggrResult_t tResult;
	CSphQueryResult tQueryResult;
	tQueryResult.m_pMeta = &tResult;
	CSphMultiQueryArgs tArgs ( 1 );
	tQuery.m_sQuery = "@title cat";
	auto pParser = sphCreatePlainQueryParser();
	tQuery.m_pQueryParser = pParser.get();

	SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema() );
	CSphString sError;
	SphQueueRes_t tRes;
	ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings, tQuery, sError, tRes );
	if ( !pSorter )
	{
		printf ( "failed to create sorter; error=%s", tResult.m_sError.cstr() );

	} else if ( !pIndex->MultiQuery ( tQueryResult, tQuery, { &pSorter, 1 }, tArgs ) )
	{
		printf ( "query failed; error=%s", pIndex->GetLastError().cstr() );

	} else
	{
		auto & tOneRes = tResult.m_dResults.Add ();
		tOneRes.FillFromSorter ( pSorter );
		printf ( "%d results found in %d.%03d sec!\n", tOneRes.m_dMatches.GetLength(), tResult.m_iQueryTime/1000, tResult.m_iQueryTime%1000 );
		ARRAY_FOREACH ( i, tOneRes.m_dMatches )
			printf ( "%d. rowid=%u, weight=%d\n", 1+i, tOneRes.m_dMatches[i].m_tRowID, tOneRes.m_dMatches[i].m_iWeight );
	}

	SafeDelete ( pSorter );
	printf ( "---\n" );
}

static int g_iFieldsCount = 0;

void DoIndexing ( CSphSource_SQL * pSrc, RtIndex_i * pIndex )
{
	CSphString sError, sWarning, sFilter;

	int64_t tmStart = sphMicroTimer ();
	int64_t tmAvgCommit = 0;
	int64_t tmMaxCommit = 0;
	int iCommits = 0;

	InsertDocData_t tDoc ( pIndex->GetMatchSchema() );
	tDoc.m_dFields.Resize(g_iFieldsCount);

	int iDynamic = pIndex->GetMatchSchema().GetRowSize();
	RtAccum_t tAcc;

	while (true)
	{
		bool bEOF = false;
		const char ** pFields = (const char **)pSrc->NextDocument ( bEOF, sError );
		if ( !pFields )
			break;

		ARRAY_FOREACH ( i, tDoc.m_dFields )
			tDoc.m_dFields[i] = VecTraits_T<const char> ( pFields[i], strlen ( pFields[i] ) );

		tDoc.m_tDoc.Combine ( pSrc->m_tDocInfo, iDynamic );

		if ( !bEOF )
			pIndex->AddDocument ( tDoc, false, sFilter, sError, sWarning, &tAcc );

		auto& const_stat = pSrc->GetStats ();
		++const_cast<CSphSourceStats&>(const_stat).m_iTotalDocuments;

		if ( ( pSrc->GetStats().m_iTotalDocuments % COMMIT_STEP )==0 || bEOF )
		{
			int64_t tmCommit = sphMicroTimer();
			pIndex->Commit ( NULL, &tAcc );
			tmCommit = sphMicroTimer()-tmCommit;

			iCommits++;
			tmAvgCommit += tmCommit;
			tmMaxCommit = Max ( tmMaxCommit, tmCommit );

			if ( bEOF )
			{
				tmAvgCommit /= iCommits;
				break;
			}
		}

		if (!( pSrc->GetStats().m_iTotalDocuments % 100 ))
			printf ( "%d docs\r", (int)pSrc->GetStats().m_iTotalDocuments );

		static bool bOnce = true;
		if ( iCommits*COMMIT_STEP>=5000 && bOnce )
		{
			printf ( "\n" );
			DoSearch ( pIndex );
			bOnce = false;
		}
	}

	pSrc->Disconnect();

	int64_t tmEnd = sphMicroTimer ();
	float fTotalMB = (float)pSrc->GetStats().m_iTotalBytes/1000000.0f;
	printf ( "commit-step %d, %d docs, %d bytes, %d.%03d sec, %.2f MB/sec\n",
		COMMIT_STEP,
		(int)pSrc->GetStats().m_iTotalDocuments,
		(int)pSrc->GetStats().m_iTotalBytes,
		(int)((tmEnd-tmStart)/1000000), (int)(((tmEnd-tmStart)%1000000)/1000),
		fTotalMB*1000000.0f/(tmEnd-tmStart) );
	printf ( "commit-docs %d, avg %d.%03d msec, max %d.%03d msec\n", COMMIT_STEP,
		(int)(tmAvgCommit/1000), (int)(tmAvgCommit%1000),
		(int)(tmMaxCommit/1000), (int)(tmMaxCommit%1000) );
	g_fTotalMB += fTotalMB;
}

// copy-pasted chunk from indexer.cpp
// FIXME! it would be good to isolate that code and reuse instead of c-pasting

static bool g_bPrintQueries = false;
static int g_iMaxFileFieldBuffer = 8 * 1024 * 1024;
static ESphOnFileFieldError g_eOnFileFieldError = FFE_IGNORE_FIELD;

/////////////////////////////////////////////////////////////////////////////

/// parse multi-valued attr definition
bool ParseMultiAttr ( const char * sBuf, CSphColumnInfo &tAttr, const char * sSourceName )
{
	// format is as follows:
	//
	// multi-valued-attr := ATTR-TYPE ATTR-NAME 'from' SOURCE-TYPE [;QUERY] [;RANGE-QUERY]
	// ATTR-TYPE := 'uint' | 'timestamp' | 'bigint'
	// SOURCE-TYPE := 'field' | 'query' | 'ranged-query'

	const char * sTok = NULL;
	int iTokLen = -1;

#define LOC_ERR( _arg, _pos ) \
    { \
        if ( !*(_pos) ) \
            fprintf ( stdout, "ERROR: source '%s': unexpected end of line in sql_attr_multi.\n", sSourceName ); \
        else \
            fprintf ( stdout, "ERROR: source '%s': expected " _arg " in sql_attr_multi, got '%s'.\n", sSourceName, _pos ); \
        return false; \
    }
#define LOC_SPACE0()        { while ( isspace(*sBuf) ) sBuf++; }
#define LOC_SPACE1()        { if ( !isspace(*sBuf) ) LOC_ERR ( "token", sBuf ) ; LOC_SPACE0(); }
#define LOC_TOK()            { sTok = sBuf; while ( sphIsAlpha(*sBuf) ) sBuf++; iTokLen = sBuf-sTok; }
#define LOC_TOKEQ( _arg )        ( iTokLen==(int)strlen(_arg) && strncasecmp ( sTok, _arg, iTokLen )==0 )
#define LOC_TEXT()            { if ( *sBuf!=';') LOC_ERR ( "';'", sBuf ); sTok = ++sBuf; while ( *sBuf && *sBuf!=';' ) sBuf++; iTokLen = sBuf-sTok; }

	// handle ATTR-TYPE
	LOC_SPACE0();
	LOC_TOK();
	if ( LOC_TOKEQ( "uint" ) )
		tAttr.m_eAttrType = SPH_ATTR_UINT32SET;
	else if ( LOC_TOKEQ( "timestamp" ) )
		tAttr.m_eAttrType = SPH_ATTR_UINT32SET;
	else if ( LOC_TOKEQ( "bigint" ) )
		tAttr.m_eAttrType = SPH_ATTR_INT64SET;
	else LOC_ERR ( "attr type ('uint' or 'timestamp' or 'bigint')", sTok );

	// handle ATTR-NAME
	LOC_SPACE1();
	LOC_TOK ();
	if ( iTokLen )
		tAttr.m_sName.SetBinary ( sTok, iTokLen );
	else LOC_ERR ( "attr name", sTok );

	// handle 'from'
	LOC_SPACE1();
	LOC_TOK();
	if ( !LOC_TOKEQ( "from" ) ) LOC_ERR ( "'from' keyword", sTok );

	// handle SOURCE-TYPE
	LOC_SPACE1();
	LOC_TOK();
	LOC_SPACE0();
	if ( LOC_TOKEQ( "field" ) )
		tAttr.m_eSrc = SPH_ATTRSRC_FIELD;
	else if ( LOC_TOKEQ( "query" ) )
		tAttr.m_eSrc = SPH_ATTRSRC_QUERY;
	else if ( LOC_TOKEQ( "ranged-query" ) )
		tAttr.m_eSrc = SPH_ATTRSRC_RANGEDQUERY;
	else if ( LOC_TOKEQ( "ranged-main-query" ) )
		tAttr.m_eSrc = SPH_ATTRSRC_RANGEDMAINQUERY;
	else LOC_ERR ( "value source type ('field', or 'query', or 'ranged-query', or 'ranged-main-query')", sTok );

	if ( tAttr.m_eSrc==SPH_ATTRSRC_FIELD )
		return true;

	// handle QUERY
	LOC_TEXT();
	if ( iTokLen )
		tAttr.m_sQuery.SetBinary ( sTok, iTokLen );
	else LOC_ERR ( "query", sTok );

	if ( tAttr.m_eSrc==SPH_ATTRSRC_QUERY || tAttr.m_eSrc==SPH_ATTRSRC_RANGEDMAINQUERY )
		return true;

	// handle RANGE-QUERY
	LOC_TEXT();
	if ( iTokLen )
		tAttr.m_sQueryRange.SetBinary ( sTok, iTokLen );
	else LOC_ERR ( "range query", sTok );

#undef LOC_ERR
#undef LOC_SPACE0
#undef LOC_SPACE1
#undef LOC_TOK
#undef LOC_TOKEQ
#undef LOC_TEXT

	return true;
}


#define LOC_CHECK( _hash, _key, _msg, _add ) \
    if (!( _hash.Exists ( _key ) )) \
    { \
        fprintf ( stdout, "ERROR: key '%s' not found " _msg "\n", _key, _add ); \
        return false; \
    }

// get string
#define LOC_GETS( _arg, _key ) \
    if ( hSource.Exists(_key) ) \
        _arg = hSource[_key].strval();

// get int
#define LOC_GETI( _arg, _key ) \
    if ( hSource.Exists(_key) && hSource[_key].intval() ) \
        _arg = hSource[_key].intval();

// get int64_t
#define LOC_GETL( _arg, _key ) \
    if ( hSource.Exists(_key) ) \
        _arg = hSource[_key].int64val();

// get bool
#define LOC_GETB( _arg, _key ) \
    if ( hSource.Exists(_key) ) \
        _arg = ( hSource[_key].intval()!=0 );

// get array of strings
#define LOC_GETA( _arg, _key ) \
    for ( CSphVariant * pVal = hSource(_key); pVal; pVal = pVal->m_pNext ) \
        _arg.Add ( pVal->cstr() );

void SqlAttrsConfigure ( CSphSourceParams_SQL &tParams, const CSphVariant * pHead, ESphAttr eAttrType
						 , const char * sSourceName, bool bIndexedAttr = false )
{
	for ( const CSphVariant * pCur = pHead; pCur; pCur = pCur->m_pNext )
	{
		CSphColumnInfo tCol ( pCur->cstr (), eAttrType );
		char * pColon = strchr ( const_cast<char *> ( tCol.m_sName.cstr () ), ':' );
		if ( pColon )
		{
			*pColon = '\0';

			if ( eAttrType==SPH_ATTR_INTEGER )
			{
				int iBits = strtol ( pColon + 1, NULL, 10 );
				if ( iBits<=0 || iBits>ROWITEM_BITS )
				{
					fprintf ( stdout, "WARNING: source '%s': attribute '%s': invalid bitcount=%d (bitcount ignored)\n"
							  , sSourceName, tCol.m_sName.cstr (), iBits );
					iBits = -1;
				}
				tCol.m_tLocator.m_iBitCount = iBits;

			} else
			{
				fprintf ( stdout, "WARNING: source '%s': attribute '%s': bitcount is only supported for integer types\n"
						  , sSourceName, tCol.m_sName.cstr () );
			}
		}
		tParams.m_dAttrs.Add ( tCol );
		if ( bIndexedAttr )
			tParams.m_dAttrs.Last ().m_bIndexed = true;
	}
}


#if WITH_ZLIB

bool ConfigureUnpack ( CSphVariant * pHead, ESphUnpackFormat eFormat, CSphSourceParams_SQL &tParams, const char * )
{
	for ( CSphVariant * pVal = pHead; pVal; pVal = pVal->m_pNext )
	{
		CSphUnpackInfo &tUnpack = tParams.m_dUnpack.Add ();
		tUnpack.m_sName = CSphString ( pVal->cstr () );
		tUnpack.m_eFormat = eFormat;
	}
	return true;
}

#else

bool ConfigureUnpack ( CSphVariant * pHead, ESphUnpackFormat, CSphSourceParams_SQL &, const char * sSourceName )
{
	if ( pHead )
	{
		fprintf ( stdout, "ERROR: source '%s': unpack is not supported, rebuild with zlib\n", sSourceName );
		return false;
	}
	return true;
}
#endif // WITH_ZLIB


bool ParseJoinedField ( const char * sBuf, CSphJoinedField * pField, const char * sSourceName )
{
	// sanity checks
	assert ( pField );
	if ( !sBuf || !sBuf[0] )
	{
		fprintf ( stdout, "ERROR: source '%s': sql_joined_field must not be empty.\n", sSourceName );
		return false;
	}

#define LOC_ERR( _exp ) \
    { \
        fprintf ( stdout, "ERROR: source '%s': expected " _exp " in sql_joined_field, got '%s'.\n", sSourceName, sBuf ); \
        return false; \
    }
#define LOC_TEXT()            { if ( *sBuf!=';') LOC_ERR ( "';'" ); sTmp = ++sBuf; while ( *sBuf && *sBuf!=';' ) sBuf++; iTokLen = sBuf-sTmp; }

	// parse field name
	while ( isspace ( *sBuf ) )
		sBuf++;

	const char * sName = sBuf;
	while ( sphIsAlpha ( *sBuf ) )
		sBuf++;
	if ( sBuf==sName ) LOC_ERR ( "field name" );
	pField->m_sName.SetBinary ( sName, sBuf - sName );

	if ( !isspace ( *sBuf ) ) LOC_ERR ( "space" );
	while ( isspace ( *sBuf ) )
		sBuf++;

	// parse 'from'
	if ( strncasecmp ( sBuf, "from", 4 ) ) LOC_ERR ( "'from'" );
	sBuf += 4;

	if ( !isspace ( *sBuf ) ) LOC_ERR ( "space" );
	while ( isspace ( *sBuf ) )
		sBuf++;

	bool bGotRanged = false;
	pField->m_bPayload = false;
	pField->m_bRangedMain = false;

	// parse 'query'
	if ( strncasecmp ( sBuf, "payload-query", 13 )==0 )
	{
		pField->m_bPayload = true;
		sBuf += 13;

	} else if ( strncasecmp ( sBuf, "query", 5 )==0 )
	{
		sBuf += 5;

	} else if ( strncasecmp ( sBuf, "ranged-query", 12 )==0 )
	{
		bGotRanged = true;
		sBuf += 12;

	} else if ( strncasecmp ( sBuf, "ranged-main-query", 17 )==0 )
	{
		pField->m_bRangedMain = true;
		sBuf += 17;

	} else LOC_ERR ( "'query'" );

	// parse ';'
	while ( isspace ( *sBuf ) && *sBuf!=';' )
		sBuf++;

	if ( *sBuf!=';' ) LOC_ERR ( "';'" );

	// handle QUERY
	const char * sTmp = sBuf;
	int iTokLen = 0;
	LOC_TEXT();
	if ( iTokLen )
		pField->m_sQuery.SetBinary ( sTmp, iTokLen );
	else LOC_ERR ( "query" );

	if ( !bGotRanged )
		return true;

	// handle RANGE-QUERY
	LOC_TEXT();
	if ( iTokLen )
		pField->m_sRanged.SetBinary ( sTmp, iTokLen );
	else LOC_ERR ( "range query" );

#undef LOC_ERR
#undef LOC_TEXT

	return true;
}


bool SqlParamsConfigure ( CSphSourceParams_SQL &tParams, const CSphConfigSection &hSource, const char * sSourceName )
{
	if ( !hSource.Exists (
		"odbc_dsn" ) ) // in case of odbc source, the host, user, pass and db are not mandatory, since they may be already defined in dsn string.
	{
		LOC_CHECK ( hSource, "sql_host", "in source '%s'", sSourceName );
		LOC_CHECK ( hSource, "sql_user", "in source '%s'", sSourceName );
		LOC_CHECK ( hSource, "sql_pass", "in source '%s'", sSourceName );
		LOC_CHECK ( hSource, "sql_db", "in source '%s'", sSourceName );
	}
	LOC_CHECK ( hSource, "sql_query", "in source '%s'", sSourceName );

	LOC_GETS ( tParams.m_sHost, "sql_host" );
	LOC_GETS ( tParams.m_sUser, "sql_user" );
	LOC_GETS ( tParams.m_sPass, "sql_pass" );
	LOC_GETS ( tParams.m_sDB, "sql_db" );
	LOC_GETI ( tParams.m_uPort, "sql_port" );

	LOC_GETS ( tParams.m_sQuery, "sql_query" );
	LOC_GETA ( tParams.m_dQueryPre, "sql_query_pre" );
	LOC_GETA ( tParams.m_dQueryPreAll, "sql_query_pre_all" );
	LOC_GETA ( tParams.m_dQueryPost, "sql_query_post" );
	LOC_GETS ( tParams.m_sQueryRange, "sql_query_range" );
	LOC_GETA ( tParams.m_dQueryPostIndex, "sql_query_post_index" );
	LOC_GETL ( tParams.m_iRangeStep, "sql_range_step" );
	LOC_GETS ( tParams.m_sQueryKilllist, "sql_query_killlist" );
	LOC_GETS ( tParams.m_sHookConnect, "hook_connect" );
	LOC_GETS ( tParams.m_sHookQueryRange, "hook_query_range" );
	LOC_GETS ( tParams.m_sHookPostIndex, "hook_post_index" );

	LOC_GETI ( tParams.m_iRangedThrottleMs, "sql_ranged_throttle" );

	SqlAttrsConfigure ( tParams, hSource ( "sql_attr_uint" ), SPH_ATTR_INTEGER, sSourceName );
	SqlAttrsConfigure ( tParams, hSource ( "sql_attr_timestamp" ), SPH_ATTR_TIMESTAMP, sSourceName );
	SqlAttrsConfigure ( tParams, hSource ( "sql_attr_bool" ), SPH_ATTR_BOOL, sSourceName );
	SqlAttrsConfigure ( tParams, hSource ( "sql_attr_float" ), SPH_ATTR_FLOAT, sSourceName );
	SqlAttrsConfigure ( tParams, hSource ( "sql_attr_bigint" ), SPH_ATTR_BIGINT, sSourceName );
	SqlAttrsConfigure ( tParams, hSource ( "sql_attr_string" ), SPH_ATTR_STRING, sSourceName );
	SqlAttrsConfigure ( tParams, hSource ( "sql_attr_json" ), SPH_ATTR_JSON, sSourceName );

	SqlAttrsConfigure ( tParams, hSource ( "sql_field_string" ), SPH_ATTR_STRING, sSourceName, true );

	LOC_GETA ( tParams.m_dFileFields, "sql_file_field" );

	tParams.m_iMaxFileBufferSize = g_iMaxFileFieldBuffer;
	tParams.m_iRefRangeStep = tParams.m_iRangeStep;
	tParams.m_eOnFileFieldError = g_eOnFileFieldError;

	// unpack
	if ( !ConfigureUnpack ( hSource ( "unpack_zlib" ), SPH_UNPACK_ZLIB, tParams, sSourceName ) )
		return false;

	if ( !ConfigureUnpack ( hSource ( "unpack_mysqlcompress" ), SPH_UNPACK_MYSQL_COMPRESS, tParams, sSourceName ) )
		return false;

	tParams.m_uUnpackMemoryLimit = hSource.GetSize ( "unpack_mysqlcompress_maxsize", 16777216 );

	// parse multi-attrs
	for ( CSphVariant * pVal = hSource ( "sql_attr_multi" ); pVal; pVal = pVal->m_pNext )
	{
		CSphColumnInfo tAttr;
		if ( !ParseMultiAttr ( pVal->cstr (), tAttr, sSourceName ) )
			return false;
		tParams.m_dAttrs.Add ( tAttr );
	}

	// parse joined fields
	for ( CSphVariant * pVal = hSource ( "sql_joined_field" ); pVal; pVal = pVal->m_pNext )
		if ( !ParseJoinedField ( pVal->cstr (), &tParams.m_dJoinedFields.Add (), sSourceName ) )
			return false;

	// make sure attr names are unique
	ARRAY_FOREACH ( i, tParams.m_dAttrs )
		for ( int j = i + 1; j<tParams.m_dAttrs.GetLength (); j++ )
		{
			const CSphString &sName = tParams.m_dAttrs[i].m_sName;
			if ( sName==tParams.m_dAttrs[j].m_sName )
			{
				fprintf ( stdout, "ERROR: duplicate attribute name: %s\n", sName.cstr () );
				return false;
			}
		}

	// additional checks
	if ( tParams.m_iRangedThrottleMs<0 )
	{
		fprintf ( stdout, "WARNING: sql_ranged_throttle must not be negative; throttling disabled\n" );
		tParams.m_iRangedThrottleMs = 0;
	}

	// debug printer
	if ( g_bPrintQueries )
		tParams.m_bPrintQueries = true;

	return true;
}


CSphSource_SQL * SpawnSource ( const char * sSourceName, const CSphConfigType &hSources, TokenizerRefPtr_c pTok, DictRefPtr_c pDict )
{
	const CSphConfigSection &hSource = hSources[sSourceName];

	assert ( hSource["type"]=="mysql" );

	CSphSourceParams_MySQL tParams;
	if ( !SqlParamsConfigure ( tParams, hSource, sSourceName ) )
		return nullptr;

	LOC_GETS ( tParams.m_sUsock, "sql_sock" );
	LOC_GETI ( tParams.m_iFlags, "mysql_connect_flags" );
	LOC_GETS ( tParams.m_sSslKey, "mysql_ssl_key" );
	LOC_GETS ( tParams.m_sSslCert, "mysql_ssl_cert" );
	LOC_GETS ( tParams.m_sSslCA, "mysql_ssl_ca" );

	//auto * pSrc = new CSphSource_MySQL ( "sSourceName" );
	auto * pSrc = CreateSourceMysql ( tParams, "sSourceName" );
	if (!pSrc)
		sphDie ( "setup failed" );
	pSrc->SetTokenizer ( std::move ( pTok ) );
	pSrc->SetDict ( pDict );

	SetupIndexing ( pSrc );
	return (CSphSource_SQL*) pSrc;
}


static RtIndex_i * g_pIndex = NULL;


void IndexingThread ( void * pArg )
{
	auto * pSrc = (CSphSource_SQL *) pArg;
	DoIndexing ( pSrc, g_pIndex );
}


int main ( int argc, char ** argv )
{
	if ( argc==2 )
		COMMIT_STEP = atoi ( argv[1] );

	// threads should be initialized before memory allocations
	char cTopOfMainStack;
	Threads::Init();
	Threads::PrepareMainThread ( &cTopOfMainStack );

	CSphConfig hConf;
	ParseConfig ( &hConf, "internal", FROMS(rtestconfig) );
	const CSphConfigType &hSources = hConf["source"];

	CSphString sError;
	CSphDictSettings tDictSettings;
	tDictSettings.m_bWordDict = false;

	TokenizerRefPtr_c pTok = Tokenizer::Detail::CreateUTF8Tokenizer();
	DictRefPtr_c pDict {sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt1", false, 32, nullptr, sError )};
	auto * pSrc = SpawnSource ( "test1", hSources, pTok, pDict );

	TokenizerRefPtr_c pTok2 { Tokenizer::Detail::CreateUTF8Tokenizer() };
	DictRefPtr_c pDict2 {sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt2", false, 32, nullptr, sError )};
	auto * pSrc2 = SpawnSource ( "test2", hSources, pTok2, pDict2 );

	CSphSchema tSrcSchema;
	if ( !pSrc->UpdateSchema ( &tSrcSchema, sError ) )
		sphDie ( "update-schema failed: %s", sError.cstr() );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	for ( int i=0; i<tSrcSchema.GetFieldsCount(); i++ )
		tSchema.AddField ( tSrcSchema.GetField(i) );

	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

	g_iFieldsCount = tSrcSchema.GetFieldsCount();

	CSphConfigSection tRTConfig;
	sphRTInit ( tRTConfig, true, nullptr );
	Binlog::Configure ( tRTConfig, true, 0, false );
	SmallStringHash_T< CSphIndex * > dTemp;
	Binlog::Replay ( dTemp );
	auto pIndex = sphCreateIndexRT ( "testrt", DATAFLD "dump", tSchema, 32*1024*1024, false ).release();
	pIndex->SetTokenizer ( pTok ); // index will own this pair from now on
	pIndex->SetDictionary ( pDict );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );
	pIndex->PostSetup();
	g_pIndex = pIndex;

	// initial indexing
	int64_t tmStart = sphMicroTimer();

	SphThread_t t1, t2;
	Threads::Create ( &t1, [pSrc] { IndexingThread ( pSrc ); } );
	Threads::Create ( &t2, [pSrc2] { IndexingThread ( pSrc2 ); } );
	Threads::Join ( &t1 );
	Threads::Join ( &t2 );

#if 0
	// update
	tParams.m_sQuery = "SELECT id, channel_id, UNIX_TIMESTAMP(published) published, title, "
		"UNCOMPRESS(content) content FROM rt2 WHERE id<=10000";
	SetupIndexing ( pSrc, tParams );
	DoIndexing ( pSrc, pIndex );
#endif

	// search
	DoSearch ( pIndex );

	// shutdown index (should cause dump)
	int64_t tmShutdown = sphMicroTimer();

#if SPH_ALLOCS_PROFILER
	printf ( "pre-shutdown allocs=%d, bytes=" INT64_FMT "\n", sphAllocsCount(), sphAllocBytes() );
#endif
	SafeDelete ( pIndex );
#if SPH_ALLOCS_PROFILER
	printf ( "post-shutdown allocs=%d, bytes=" INT64_FMT "\n", sphAllocsCount(), sphAllocBytes() );
#endif

	int64_t tmEnd = sphMicroTimer();
	printf ( "shutdown done in %d.%03d sec\n", (int)((tmEnd-tmShutdown)/1000000), (int)(((tmEnd-tmShutdown)%1000000)/1000) );
	printf ( "total with shutdown %d.%03d sec, %.2f MB/sec\n",
		(int)((tmEnd-tmStart)/1000000), (int)(((tmEnd-tmStart)%1000000)/1000),
		g_fTotalMB*1000000.0f/(tmEnd-tmStart) );

#if SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER
	sphAllocsStats();
#endif
#if _WIN32
	PROCESS_MEMORY_COUNTERS pmc;
	HANDLE hProcess = OpenProcess ( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId() );
	if ( hProcess && GetProcessMemoryInfo ( hProcess, &pmc, sizeof(pmc)) )
	{
		printf ( "--- peak-wss=%d, peak-pagefile=%d\n", (int)pmc.PeakWorkingSetSize, (int)pmc.PeakPagefileUsage );
	}
#endif

	SafeDelete ( pIndex );
	Binlog::Deinit ();
}


// BLOODY DIRTY HACK!!!
// definitions of AggrResult_t members just copy-pasted 'as is' from searchdaemon.cpp

int AggrResult_t::GetLength () const
{
	int iCount = 0;
	m_dResults.Apply ( [&iCount] ( const OneResultset_t & a ) { iCount += a.m_dMatches.GetLength (); } );
	return iCount;
}

void AggrResult_t::ClampMatches ( int iLimit )
{
	assert ( m_bSingle );
	if ( !m_dResults.IsEmpty () )
		m_dResults.First ().ClampMatches ( iLimit );
}

void AggrResult_t::ClampAllMatches ()
{
	for ( auto& dResult : m_dResults )
		dResult.ClampAllMatches();
}

int OneResultset_t::FillFromSorter ( ISphMatchSorter * pQueue )
{
	if ( !pQueue )
		return 0;

	assert ( m_dMatches.IsEmpty () );
	m_tSchema = *pQueue->GetSchema ();
	if ( !pQueue->GetLength () )
		return 0;

	int iCopied = pQueue->Flatten ( m_dMatches.AddN ( pQueue->GetLength () ) );
	m_dMatches.Resize ( iCopied );
	return iCopied;
}

void OneResultset_t::ClampAllMatches ()
{
	for ( auto& dMatch : m_dMatches )
		m_tSchema.FreeDataPtrs ( dMatch );
	m_dMatches.Reset();
}

void OneResultset_t::ClampMatches ( int iLimit )
{
	assert ( iLimit>0 );

	int iMatches = m_dMatches.GetLength ();
	for ( int i = iLimit; i<iMatches; ++i )
		m_tSchema.FreeDataPtrs ( m_dMatches[i] );
	m_dMatches.Resize ( iLimit );
}

OneResultset_t::~OneResultset_t()
{
	for ( auto & dMatch : m_dMatches )
		m_tSchema.FreeDataPtrs ( dMatch );
}
