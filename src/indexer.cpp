//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxutils.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#if USE_WINDOWS
	#define snprintf	_snprintf

	#include <io.h>
	#include <tlhelp32.h>
#else
	#include <unistd.h>
#endif

/////////////////////////////////////////////////////////////////////////////

bool			g_bQuiet		= false;
bool			g_bProgress		= true;

const char *	g_sBuildStops	= NULL;
int				g_iTopStops		= 100;
bool			g_bRotate		= false;
bool			g_bBuildFreqs	= false;

int				g_iMemLimit		= 0;

const int		EXT_COUNT = 6;
const char *	g_dExt[EXT_COUNT] = { "sph", "spa", "spi", "spd", "spp", "spm" };

/////////////////////////////////////////////////////////////////////////////

template < typename T > struct CSphMTFHashEntry
{
	CSphString				m_sKey;
	CSphMTFHashEntry<T> *	m_pNext;
	int						m_iSlot;
	T						m_tValue;
};


template < typename T, int SIZE, class HASHFUNC > class CSphMTFHash
{
public:
	/// ctor
	CSphMTFHash ()
	{
		m_pData = new CSphMTFHashEntry<T> * [ SIZE ];
		for ( int i=0; i<SIZE; i++ )
			m_pData[i] = NULL;
	}

	/// dtor
	~CSphMTFHash ()
	{
		for ( int i=0; i<SIZE; i++ )
		{
			CSphMTFHashEntry<T> * pHead = m_pData[i];
			while ( pHead )
			{
				CSphMTFHashEntry<T> * pNext = pHead->m_pNext;
				SafeDelete ( pHead );
				pHead = pNext;
			}
		}
	}

	/// add record to hash
	/// OPTIMIZE: should pass T not by reference for simple types
	T & Add ( const char * sKey, int iKeyLen, T & tValue )
	{
		DWORD uHash = HASHFUNC::Hash ( sKey ) % SIZE;

		// find matching entry
		CSphMTFHashEntry<T> * pEntry = m_pData [ uHash ];
		CSphMTFHashEntry<T> * pPrev = NULL;
		while ( pEntry && strcmp ( sKey, pEntry->m_sKey.cstr() ) )
		{
			pPrev = pEntry;
			pEntry = pEntry->m_pNext;
		}

		if ( !pEntry )
		{
			// not found, add it, but don't MTF
			pEntry = new CSphMTFHashEntry<T>;
			if ( iKeyLen )
				pEntry->m_sKey.SetBinary ( sKey, iKeyLen );
			else
				pEntry->m_sKey = sKey;
			pEntry->m_pNext = NULL;
			pEntry->m_iSlot = (int)uHash;
			pEntry->m_tValue = tValue;
			if ( !pPrev )
				m_pData [ uHash ] = pEntry;
			else
				pPrev->m_pNext = pEntry;
		} else
		{
			// MTF on access
			if ( pPrev )
			{
				pPrev->m_pNext = pEntry->m_pNext;
				pEntry->m_pNext = m_pData [ uHash ];
				m_pData [ uHash ] = pEntry;
			}
		}

		return pEntry->m_tValue;
	}

	/// find first non-empty entry
	const CSphMTFHashEntry<T> * FindFirst ()
	{
		for ( int i=0; i<SIZE; i++ )
			if ( m_pData[i] )
				return m_pData[i];
		return NULL;
	}

	/// find next non-empty entry
	const CSphMTFHashEntry<T> * FindNext ( const CSphMTFHashEntry<T> * pEntry )
	{
		assert ( pEntry );
		if ( pEntry->m_pNext )
			return pEntry->m_pNext;

		for ( int i=1+pEntry->m_iSlot; i<SIZE; i++ )
			if ( m_pData[i] )
				return m_pData[i];
		return NULL;
	}

protected:
	CSphMTFHashEntry<T> **	m_pData;
};

#define HASH_FOREACH(_it,_hash) \
	for ( _it=_hash.FindFirst(); _it; _it=_hash.FindNext(_it) )

/////////////////////////////////////////////////////////////////////////////

struct Word_t
{
	const char *	m_sWord;
	int				m_iCount;
};


inline bool operator < ( const Word_t & a, const Word_t & b)
{
	return a.m_iCount < b.m_iCount;
};


class CSphStopwordBuilderDict : public CSphDict
{
public:
						CSphStopwordBuilderDict () {}
	void				Save ( const char * sOutput, int iTop, bool bFreqs );

public:
	virtual SphWordID_t	GetWordID ( BYTE * pWord );
	virtual SphWordID_t	GetWordID ( const BYTE * pWord, int iLen );

	virtual void		LoadStopwords ( const char *, ISphTokenizer * ) {}
	virtual bool		LoadWordforms ( const char *, ISphTokenizer * ) { return true; }
	virtual bool		SetMorphology ( const CSphVariant *, bool, CSphString & ) { return true; }

protected:
	struct HashFunc_t
	{
		static inline DWORD Hash ( const char * sKey )
		{
			return sphCRC32 ( (const BYTE*)sKey );
		}
	};

protected:
	CSphMTFHash < int, 1048576, HashFunc_t >	m_hWords;
};


void CSphStopwordBuilderDict::Save ( const char * sOutput, int iTop, bool bFreqs )
{
	FILE * fp = fopen ( sOutput, "w+" );
	if ( !fp )
		return;

	CSphVector<Word_t> dTop;
	dTop.Reserve ( 1024 );

	const CSphMTFHashEntry<int> * it;
	HASH_FOREACH ( it, m_hWords )
	{
		Word_t t;
		t.m_sWord = it->m_sKey.cstr();
		t.m_iCount = it->m_tValue;
		dTop.Add ( t );
	}

	dTop.RSort ();

	ARRAY_FOREACH ( i, dTop )
	{
		if ( i>=iTop )
			break;
		if ( bFreqs )
			fprintf ( fp, "%s %d\n", dTop[i].m_sWord, dTop[i].m_iCount );
		else
			fprintf ( fp, "%s\n", dTop[i].m_sWord );
	}

	fclose ( fp );
}


SphWordID_t CSphStopwordBuilderDict::GetWordID ( BYTE * pWord )
{
	int iZero = 0;
	m_hWords.Add ( (const char *)pWord, 0, iZero )++;
	return 1;
}


SphWordID_t CSphStopwordBuilderDict::GetWordID ( const BYTE * pWord, int iLen )
{
	int iZero = 0;
	m_hWords.Add ( (const char *)pWord, iLen, iZero )++;
	return 1;
}

/////////////////////////////////////////////////////////////////////////////

void ShowProgress ( const CSphIndexProgress * pProgress, bool bPhaseEnd )
{
	// if in quiet mode, do not show anything at all
	// if in no-progress mode, only show phase ends
	if ( g_bQuiet || ( !g_bProgress && !bPhaseEnd ) )
		return;

	switch ( pProgress->m_ePhase )
	{
		case CSphIndexProgress::PHASE_COLLECT:
			fprintf ( stdout, "collected %d docs, %.1f MB", pProgress->m_iDocuments, float(pProgress->m_iBytes)/1000000.0f );
			break;

		case CSphIndexProgress::PHASE_SORT:
			fprintf ( stdout, "sorted %.1f Mhits, %.1f%% done", float(pProgress->m_iHits)/1000000,
				100.0f*float(pProgress->m_iHits) / float(pProgress->m_iHitsTotal) );
			break;

		case CSphIndexProgress::PHASE_COLLECT_MVA:
			fprintf ( stdout, "collected %"PRIu64" attr values", pProgress->m_iAttrs );
			break;

		case CSphIndexProgress::PHASE_SORT_MVA:
			fprintf ( stdout, "sorted %.1f Mvalues, %.1f%% done", float(pProgress->m_iAttrs)/1000000,
				100.0f*float(pProgress->m_iAttrs) / float(pProgress->m_iAttrsTotal) );
			break;
		case CSphIndexProgress::PHASE_MERGE:
			fprintf ( stdout, "merged %.1f Kwords", float(pProgress->m_iWords)/1000 );
			break;
	}

	fprintf ( stdout, bPhaseEnd ? "\n" : "\r" );
	fflush ( stdout );
}

/////////////////////////////////////////////////////////////////////////////

/// parse multi-valued attr definition
bool ParseMultiAttr ( const char * sBuf, CSphColumnInfo & tAttr, const char * sSourceName )
{
	// format is as follows:
	//
	// multi-valued-attr := ATTR-TYPE ATTR-NAME 'from' SOURCE-TYPE [;QUERY] [;RANGE-QUERY]
	// ATTR-TYPE := 'uint' | 'timestamp'
	// SOURCE-TYPE := 'field' | 'query' | 'ranged-query'

	const char * sTok = NULL;
	int iTokLen = -1;

#define LOC_ERR(_arg,_pos) \
	{ \
		if ( !*(_pos) ) \
			fprintf ( stdout, "ERROR: source '%s': unexpected end of line in sql_attr_multi.\n", sSourceName ); \
		else \
			fprintf ( stdout, "ERROR: source '%s': expected " _arg " in sql_attr_multi, got '%s'.\n", sSourceName, _pos ); \
		return false; \
	}
#define LOC_SPACE0()		{ while ( isspace(*sBuf) ) sBuf++; }
#define LOC_SPACE1()		{ if ( !isspace(*sBuf) ) LOC_ERR ( "token", sBuf ) ; LOC_SPACE0(); }
#define LOC_TOK()			{ sTok = sBuf; while ( sphIsAlpha(*sBuf) ) sBuf++; iTokLen = sBuf-sTok; }
#define LOC_TOKEQ(_arg)		( iTokLen==(int)strlen(_arg) && strncasecmp ( sTok, _arg, iTokLen )==0 )
#define LOC_TEXT()			{ if ( *sBuf!=';') LOC_ERR ( "';'", sBuf ); sTok = ++sBuf; while ( *sBuf && *sBuf!=';' ) sBuf++; iTokLen = sBuf-sTok; }

	// handle ATTR-TYPE
	LOC_SPACE0(); LOC_TOK();
	if ( LOC_TOKEQ("uint") )				tAttr.m_eAttrType = SPH_ATTR_INTEGER | SPH_ATTR_MULTI;
	else if ( LOC_TOKEQ("timestamp") )		tAttr.m_eAttrType = SPH_ATTR_INTEGER | SPH_ATTR_MULTI;
	else									LOC_ERR ( "attr type ('uint' or 'timestamp')", sTok );

	// handle ATTR-NAME
	LOC_SPACE1(); LOC_TOK ();
	if ( iTokLen )							tAttr.m_sName.SetBinary ( sTok, iTokLen );
	else									LOC_ERR ( "attr name", sTok );

	// handle 'from'
	LOC_SPACE1(); LOC_TOK();
	if ( !LOC_TOKEQ("from") )				LOC_ERR ( "'from' keyword", sTok );

	// handle SOURCE-TYPE
	LOC_SPACE1(); LOC_TOK(); LOC_SPACE0();
	if ( LOC_TOKEQ("field") )				tAttr.m_eSrc = SPH_ATTRSRC_FIELD;
	else if ( LOC_TOKEQ("query") )			tAttr.m_eSrc = SPH_ATTRSRC_QUERY;
	else if ( LOC_TOKEQ("ranged-query") )	tAttr.m_eSrc = SPH_ATTRSRC_RANGEDQUERY;
	else									LOC_ERR ( "value source type ('field', or 'query', or 'ranged-query')", sTok );

	if ( tAttr.m_eSrc==SPH_ATTRSRC_FIELD )	return true;

	// handle QUERY
	LOC_TEXT();
	if ( iTokLen )							tAttr.m_sQuery.SetBinary ( sTok, iTokLen );
	else									LOC_ERR ( "query", sTok );

	if ( tAttr.m_eSrc==SPH_ATTRSRC_QUERY )	return true;

	// handle RANGE-QUERY
	LOC_TEXT();
	if ( iTokLen )							tAttr.m_sQueryRange.SetBinary ( sTok, iTokLen );
	else									LOC_ERR ( "range query", sTok );

#undef LOC_ERR
#undef LOC_SPACE0
#undef LOC_SPACE1
#undef LOC_TOK
#undef LOC_TOKEQ
#undef LOC_TEXT

	return true;
}


#define LOC_CHECK(_hash,_key,_msg,_add) \
	if (!( _hash.Exists ( _key ) )) \
	{ \
		fprintf ( stdout, "ERROR: key '%s' not found " _msg "\n", _key, _add ); \
		return false; \
	}

// get string
#define LOC_GETS(_arg,_key) \
	if ( hSource.Exists(_key) ) \
		_arg = hSource[_key];

// get int
#define LOC_GETI(_arg,_key) \
	if ( hSource.Exists(_key) && hSource[_key].intval() ) \
		_arg = hSource[_key].intval();

// get array of strings
#define LOC_GETAS(_arg,_key) \
	for ( CSphVariant * pVal = hSource(_key); pVal; pVal = pVal->m_pNext ) \
		_arg.Add ( pVal->cstr() );


void SqlAttrsConfigure ( CSphSourceParams_SQL & tParams, const CSphVariant * pHead, DWORD uAttrType, const char * sSourceName )
{
	for ( const CSphVariant * pCur = pHead; pCur; pCur= pCur->m_pNext )
	{
		CSphColumnInfo tCol ( pCur->cstr(), uAttrType );
		char * pColon = strchr ( const_cast<char*>(tCol.m_sName.cstr()), ':' );
		if ( pColon )
		{
			*pColon = '\0';

			if ( uAttrType==SPH_ATTR_INTEGER )
			{
				int iBits = strtol ( pColon+1, NULL, 10 );
				if ( iBits<=0 || iBits>ROWITEM_BITS )
				{
					fprintf ( stdout, "WARNING: source '%s': attribute '%s': invalid bitcount=%d (bitcount ignored)\n",
						sSourceName, tCol.m_sName.cstr(), iBits );
					iBits = -1;
				}
				tCol.m_iBitCount = iBits;

			} else
			{
				fprintf ( stdout, "WARNING: source '%s': attribute '%s': bitcount is only supported for integer types\n",
					sSourceName, tCol.m_sName.cstr() );
			}
		}
		tParams.m_dAttrs.Add ( tCol );
	}
}


bool SqlParamsConfigure ( CSphSourceParams_SQL & tParams, const CSphConfigSection & hSource, const char * sSourceName )
{
	LOC_CHECK ( hSource, "sql_host", "in source '%s'", sSourceName );
	LOC_CHECK ( hSource, "sql_user", "in source '%s'", sSourceName );
	LOC_CHECK ( hSource, "sql_pass", "in source '%s'", sSourceName );
	LOC_CHECK ( hSource, "sql_db", "in source '%s'", sSourceName );
	LOC_CHECK ( hSource, "sql_query", "in source '%s'", sSourceName );

	LOC_GETS ( tParams.m_sHost,				"sql_host" );
	LOC_GETS ( tParams.m_sUser,				"sql_user" );
	LOC_GETS ( tParams.m_sPass,				"sql_pass" );
	LOC_GETS ( tParams.m_sDB,				"sql_db" );
	LOC_GETI ( tParams.m_iPort,				"sql_port");

	LOC_GETS ( tParams.m_sQuery,			"sql_query" );
	LOC_GETAS( tParams.m_dQueryPre,			"sql_query_pre" );
	LOC_GETAS( tParams.m_dQueryPost,		"sql_query_post" );
	LOC_GETS ( tParams.m_sQueryRange,		"sql_query_range" );
	LOC_GETAS( tParams.m_dQueryPostIndex,	"sql_query_post_index" );
	LOC_GETI ( tParams.m_iRangeStep,		"sql_range_step" );

	LOC_GETI ( tParams.m_iRangedThrottle,	"sql_ranged_throttle" );

	SqlAttrsConfigure ( tParams,	hSource("sql_group_column"),		SPH_ATTR_INTEGER,	sSourceName );
	SqlAttrsConfigure ( tParams,	hSource("sql_date_column"),			SPH_ATTR_TIMESTAMP,	sSourceName );
	SqlAttrsConfigure ( tParams,	hSource("sql_str2ordinal_column"),	SPH_ATTR_ORDINAL,	sSourceName );

	SqlAttrsConfigure ( tParams,	hSource("sql_attr_uint"),			SPH_ATTR_INTEGER,	sSourceName );
	SqlAttrsConfigure ( tParams,	hSource("sql_attr_timestamp"),		SPH_ATTR_TIMESTAMP,	sSourceName );
	SqlAttrsConfigure ( tParams,	hSource("sql_attr_str2ordinal"),	SPH_ATTR_ORDINAL,	sSourceName );
	SqlAttrsConfigure ( tParams,	hSource("sql_attr_bool"),			SPH_ATTR_BOOL,		sSourceName );
	SqlAttrsConfigure ( tParams,	hSource("sql_attr_float"),			SPH_ATTR_FLOAT,		sSourceName );

	// parse multi-attrs
	for ( CSphVariant * pVal = hSource("sql_attr_multi"); pVal; pVal = pVal->m_pNext )
	{
		CSphColumnInfo tAttr;
		if ( !ParseMultiAttr ( pVal->cstr(), tAttr, sSourceName ) )
			return false;
		tParams.m_dAttrs.Add ( tAttr );
	}

	// additional checks
	if ( tParams.m_iRangedThrottle<0 )
	{
		fprintf ( stdout, "WARNING: sql_ranged_throttle must not be negative; throttling disabled\n" );
		tParams.m_iRangedThrottle = 0;
	}

	return true;
}


#if USE_PGSQL
CSphSource * SpawnSourcePgSQL ( const CSphConfigSection & hSource, const char * sSourceName )
{
	assert ( hSource["type"]=="pgsql" );

	CSphSourceParams_PgSQL tParams;
	if ( !SqlParamsConfigure ( tParams, hSource, sSourceName ) )
		return NULL;

	LOC_GETS ( tParams.m_sClientEncoding,	"sql_client_encoding" );

	CSphSource_PgSQL * pSrcPgSQL = new CSphSource_PgSQL ( sSourceName );
	if ( !pSrcPgSQL->Setup ( tParams ) )
		SafeDelete ( pSrcPgSQL );

	return pSrcPgSQL;
}
#endif // USE_PGSQL


#if USE_MYSQL
CSphSource * SpawnSourceMySQL ( const CSphConfigSection & hSource, const char * sSourceName )
{
	assert ( hSource["type"]=="mysql" );

	CSphSourceParams_MySQL tParams;
	if ( !SqlParamsConfigure ( tParams, hSource, sSourceName ) )
		return NULL;

	LOC_GETS ( tParams.m_sUsock,			"sql_sock" );
	LOC_GETI ( tParams.m_iFlags,			"mysql_connect_flags" );

	CSphSource_MySQL * pSrcMySQL = new CSphSource_MySQL ( sSourceName );
	if ( !pSrcMySQL->Setup ( tParams ) )
		SafeDelete ( pSrcMySQL );

	return pSrcMySQL;
}
#endif // USE_MYSQL


CSphSource * SpawnSourceXMLPipe ( const CSphConfigSection & hSource, const char * sSourceName, bool bUTF8 )
{
	assert ( hSource["type"]=="xmlpipe" || hSource["type"]=="xmlpipe2" );
	
	LOC_CHECK ( hSource, "xmlpipe_command", "in source '%s'.", sSourceName );

	CSphSource * pSrcXML = NULL;

	CSphString sCommand = hSource["xmlpipe_command"];
	const int MAX_BUF_SIZE = 128;
	BYTE dBuffer [MAX_BUF_SIZE];
	int iBufSize = 0;
	bool bUsePipe2 = true;

	FILE * pPipe = sphDetectXMLPipe ( sCommand.cstr (), dBuffer, iBufSize, MAX_BUF_SIZE, bUsePipe2 );
	if ( !pPipe )
	{
		fprintf ( stdout, "ERROR: xmlpipe: failed to popen '%s'", sCommand.cstr() );
		return NULL;
	}	

	if ( bUsePipe2 )
	{
#if USE_LIBEXPAT || USE_LIBXML
		pSrcXML = sphCreateSourceXmlpipe2 ( &hSource, pPipe, dBuffer, iBufSize, sSourceName );

		if ( !bUTF8 )
		{
			SafeDelete ( pSrcXML );
			fprintf ( stdout, "ERROR: source '%s': xmlpipe2 should only be used with charset_type=utf-8\n", sSourceName );
		}
#else
		fprintf ( stdout, "WARNING: source '%s': unknown source type '%s' (missing libexpat/libxml2?)\n", sSourceName, hSource["type"].cstr() );
#endif
	}
	else
	{
		CSphSource_XMLPipe * pXmlPipe = new CSphSource_XMLPipe ( dBuffer, iBufSize, sSourceName );
		if ( !pXmlPipe->Setup ( pPipe, sCommand.cstr () ) )
			SafeDelete ( pXmlPipe );

		pSrcXML = pXmlPipe;
	}

	return pSrcXML;
}


CSphSource * SpawnSource ( const CSphConfigSection & hSource, const char * sSourceName, bool bUTF8 )
{
	if ( !hSource.Exists ( "type" ) )
	{
		fprintf ( stdout, "ERROR: source '%s': type not found; skipping.\n", sSourceName );
		return NULL;
	}

	#if USE_PGSQL
	if ( hSource["type"]=="pgsql")
		return SpawnSourcePgSQL ( hSource, sSourceName );
	#endif

	#if USE_MYSQL
	if ( hSource["type"]=="mysql")
		return SpawnSourceMySQL ( hSource, sSourceName );
	#endif

	if ( hSource["type"]=="xmlpipe" || hSource["type"]=="xmlpipe2" )
	{
		if ( hSource["type"]=="xmlpipe2" )
			fprintf ( stdout, "WARNING: xmlpipe2 source type is deprecated; use xmlpipe\n" );

		return SpawnSourceXMLPipe ( hSource, sSourceName, bUTF8 );
	}

	fprintf ( stdout, "ERROR: source '%s': unknown type '%s'; skipping.\n", sSourceName,
		hSource["type"].cstr() );
	return NULL;
}

#undef LOC_CHECK
#undef LOC_GETS
#undef LOC_GETI
#undef LOC_GETAS
#undef LOC_GETAA

//////////////////////////////////////////////////////////////////////////
// INDEXING
//////////////////////////////////////////////////////////////////////////

bool DoIndex ( const CSphConfigSection & hIndex, const char * sIndexName, const CSphConfigType & hSources )
{
	if ( hIndex("type") && hIndex["type"]=="distributed" )
	{
		if ( !g_bQuiet )
		{
			fprintf ( stdout, "distributed index '%s' can not be directly indexed; skipping.\n", sIndexName );
			fflush ( stdout );
		}
		return false;
	}


	if ( !g_bQuiet )
	{
		fprintf ( stdout, "indexing index '%s'...\n", sIndexName );
		fflush ( stdout );
	}

	// check config
	if ( !hIndex("path") )
	{
		fprintf ( stdout, "ERROR: index '%s': key 'path' not found.\n", sIndexName );
		return false;
	}

	if ( ( hIndex.GetInt ( "min_prefix_len", 0 ) > 0 || hIndex.GetInt ( "min_infix_len", 0 ) > 0 ) 
		&& hIndex.GetInt ( "enable_star" ) == 0 )
	{
		const char * szMorph = hIndex.GetStr ( "morphology", "" );
		if ( szMorph && *szMorph && strcmp ( szMorph, "none" ) )
		{
			fprintf ( stdout, "ERROR: index '%s': infixes and morphology are enabled, enable_star=0", sIndexName );
			return false;
		}
	}

	///////////////////
	// spawn tokenizer
	///////////////////

	CSphString sError;
	ISphTokenizer * pTokenizer = sphConfTokenizer ( hIndex, sError );
	if ( !pTokenizer )
		sphDie ( "index '%s': %s", sIndexName, sError.cstr() );

	// prefix/infix indexing
	int iPrefix = hIndex("min_prefix_len") ? hIndex["min_prefix_len"].intval() : 0;
	int iInfix = hIndex("min_infix_len") ? hIndex["min_infix_len"].intval() : 0;
	iPrefix = Max ( iPrefix, 0 );
	iInfix = Max ( iInfix, 0 );

	CSphString sPrefixFields, sInfixFields;

	if ( hIndex.Exists ( "prefix_fields" ) ) 
		sPrefixFields = hIndex ["prefix_fields"].cstr ();

	if ( hIndex.Exists ( "infix_fields" ) ) 
		sInfixFields = hIndex ["infix_fields"].cstr ();

	if ( iPrefix == 0 && !sPrefixFields.IsEmpty () )
		fprintf ( stdout, "WARNING: min_prefix_len = 0. prefix_fields are ignored\n" );

	if ( iInfix == 0 && !sInfixFields.IsEmpty () )
		fprintf ( stdout, "WARNING: min_infix_len = 0. infix_fields are ignored\n" );

	// boundary
	int iBoundaryStep = hIndex("phrase_boundary_step") ? Max ( hIndex["phrase_boundary_step"].intval(), 0 ) : 0;

	/////////////////////
	// spawn datasources
	/////////////////////

	// check for per-index HTML stipping override
	bool bStripOverride = false;

	bool bHtmlStrip = false;
	CSphString sHtmlIndexAttrs, sHtmlRemoveElements;

	if ( hIndex("html_strip") )
	{
		bStripOverride = true;
		bHtmlStrip = hIndex.GetInt ( "html_strip" )!=0;
		sHtmlIndexAttrs = hIndex.GetStr ( "html_index_attrs" );
		sHtmlRemoveElements = hIndex.GetStr ( "html_remove_elements" );
	}

	// parse all sources
	CSphVector<CSphSource*> dSources;
	bool bGotAttrs = false;

	for ( CSphVariant * pSourceName = hIndex("source"); pSourceName; pSourceName = pSourceName->m_pNext )
	{
		if ( !hSources ( pSourceName->cstr() ) )
		{
			fprintf ( stdout, "ERROR: index '%s': source '%s' not found.\n", sIndexName, pSourceName->cstr() );
			continue;
		}
		const CSphConfigSection & hSource = hSources [ pSourceName->cstr() ];

		CSphSource * pSource = SpawnSource ( hSource, pSourceName->cstr(), pTokenizer->IsUtf8 () );
		if ( !pSource )
			continue;

		if ( pSource->HasAttrsConfigured() )
			bGotAttrs = true;

		pSource->SetupFieldMatch ( sPrefixFields.cstr (), sInfixFields.cstr () );

		// strip_html, index_html_attrs
		CSphString sError;
		if ( bStripOverride )
		{
			// apply per-index overrides
			if ( bHtmlStrip )
			{
				if ( !pSource->SetStripHTML ( sHtmlIndexAttrs.cstr(), sHtmlRemoveElements.cstr(), sError ) )
				{
					fprintf ( stdout, "ERROR: source '%s': %s.\n", pSourceName->cstr(), sError.cstr() );
					return false;
				}
			}

		} else if ( hSource.GetInt ( "strip_html" ) )
		{
			// apply deprecated per-source settings if there are no overrides
			if ( !pSource->SetStripHTML ( hSource.GetStr ( "index_html_attrs" ), "", sError ) )
			{
				fprintf ( stdout, "ERROR: source '%s': %s.\n", pSourceName->cstr(), sError.cstr() );
				return false;
			}
		}

		pSource->SetTokenizer ( pTokenizer );
		dSources.Add ( pSource );
	}

	if ( !dSources.GetLength() )
	{
		fprintf ( stdout, "ERROR: index '%s': no valid sources configured; skipping.\n", sIndexName );
		return false;
	}

	// configure docinfo storage
	ESphDocinfo eDocinfo = SPH_DOCINFO_EXTERN;
	if ( hIndex("docinfo") )
	{
		if ( hIndex["docinfo"]=="none" )	eDocinfo = SPH_DOCINFO_NONE;
		if ( hIndex["docinfo"]=="inline" )	eDocinfo = SPH_DOCINFO_INLINE;
	}
	if ( bGotAttrs && eDocinfo==SPH_DOCINFO_NONE )
	{
		fprintf ( stdout, "FATAL: index '%s': got attributes, but docinfo is 'none' (fix your config file).\n", sIndexName );
		exit ( 1 );
	}

	///////////
	// do work
	///////////

	float fTime = sphLongTimer ();
	bool bOK = false;

	if ( g_sBuildStops )
	{
		///////////////////
		// build stopwords
		///////////////////

		if ( !g_bQuiet )
		{
			fprintf ( stdout, "building stopwords list...\n" );
			fflush ( stdout );
		}

		CSphStopwordBuilderDict tDict;
		ARRAY_FOREACH ( i, dSources )
		{
			CSphString sError;
			dSources[i]->SetDict ( &tDict );
			if ( !dSources[i]->Connect ( sError ) || !dSources[i]->IterateHitsStart ( sError ) )
				continue;
			while ( dSources[i]->IterateHitsNext ( sError ) && dSources[i]->m_tDocInfo.m_iDocID );
		}
		tDict.Save ( g_sBuildStops, g_iTopStops, g_bBuildFreqs );

	} else
	{
		///////////////
		// create dict
		///////////////

		// create dict
		CSphDict * pDict = sphCreateDictionaryCRC ( hIndex("morphology"), hIndex.GetStr("stopwords"),
			hIndex.GetStr("wordforms"), pTokenizer, sError );
		assert ( pDict );

		if ( !sError.IsEmpty () )
			fprintf ( stdout, "WARNING: index '%s': %s\n", sIndexName, sError.cstr() );	

		//////////
		// index!
		//////////

		// if searchd is running, we want to reindex to .tmp files
		CSphString sIndexPath;
		sIndexPath.SetSprintf ( g_bRotate ? "%s.tmp" : "%s", hIndex["path"].cstr() );

		// do index
		CSphIndex * pIndex = sphCreateIndexPhrase ( sIndexPath.cstr() );
		assert ( pIndex );

		// check lock file
		if ( !pIndex->Lock() )
		{
			fprintf ( stdout, "FATAL: %s, will not index. Try --rotate option.\n", pIndex->GetLastError().cstr() );
			exit ( 1 );
		}

		pIndex->SetProgressCallback ( ShowProgress );
		pIndex->SetInfixIndexing ( iPrefix, iInfix );
		pIndex->SetBoundaryStep ( iBoundaryStep );

		bOK = pIndex->Build ( pDict, dSources, g_iMemLimit, eDocinfo )!=0;
		if ( bOK && g_bRotate )
		{
			sIndexPath.SetSprintf ( "%s.new", hIndex["path"].cstr() );
			bOK = pIndex->Rename ( sIndexPath.cstr() );
		}

		if ( !bOK )
			fprintf ( stdout, "ERROR: index '%s': %s.\n", sIndexName, pIndex->GetLastError().cstr() );

		pIndex->Unlock ();

		SafeDelete ( pIndex );
		SafeDelete ( pDict );
	}

	// trip report
	fTime = sphLongTimer () - fTime;
	if ( !g_bQuiet )
	{
		fTime = Max ( fTime, 0.01f );

		CSphSourceStats tTotal;
		ARRAY_FOREACH ( i, dSources )
		{
			const CSphSourceStats & tSource = dSources[i]->GetStats();
			tTotal.m_iTotalDocuments += tSource.m_iTotalDocuments;
			tTotal.m_iTotalBytes += tSource.m_iTotalBytes;
		}

		fprintf ( stdout, "total %d docs, %"PRIi64" bytes\n",
			tTotal.m_iTotalDocuments, tTotal.m_iTotalBytes );

		fprintf ( stdout, "total %.3f sec, %.2f bytes/sec, %.2f docs/sec\n",
			fTime, tTotal.m_iTotalBytes/fTime, tTotal.m_iTotalDocuments/fTime );
	}

	// cleanup and go on
	ARRAY_FOREACH ( i, dSources )
		SafeDelete ( dSources[i] );
	SafeDelete ( pTokenizer );

	return bOK;
}

//////////////////////////////////////////////////////////////////////////
// MERGING
//////////////////////////////////////////////////////////////////////////

bool DoMerge ( const CSphConfigSection & hDst, const char * sDst,
	const CSphConfigSection & hSrc, const char * sSrc, CSphVector<CSphFilter> & tPurge, bool bRotate )
{
	// check config
	if ( !hDst("path") )
	{
		fprintf ( stdout, "ERROR: index '%s': key 'path' not found.\n", sDst );
		return false;
	}
	if ( !hSrc("path") )
	{
		fprintf ( stdout, "ERROR: index '%s': key 'path' not found.\n", sSrc );
		return false;
	}

	// do the merge
	CSphIndex * pSrc = sphCreateIndexPhrase ( hSrc["path"].cstr() );
	CSphIndex * pDst = sphCreateIndexPhrase ( hDst["path"].cstr() );
	assert ( pSrc );
	assert ( pDst );

	if ( !pSrc->Lock() && !bRotate )
	{
		fprintf ( stdout, "ERROR: index '%s' is already locked; lock: %s\n", sSrc, pSrc->GetLastError().cstr() );
		return false;
	}

	if ( !pDst->Lock() && !bRotate )
	{
		fprintf ( stdout, "ERROR: index '%s' is already locked; lock: %s\n", sDst, pDst->GetLastError().cstr() );
		return false;
	}

	pDst->SetProgressCallback ( ShowProgress );

	float tmMerge = -sphLongTimer ();
	if ( !pDst->Merge ( pSrc, tPurge ) )
		sphDie ( "failed to merge index '%s' into index '%s': %s", sSrc, sDst, pDst->GetLastError().cstr() );
	tmMerge += sphLongTimer ();
	printf ( "merged in %.1f sec\n", tmMerge );

	// pick up merge result
	const char * sPath = hDst["path"].cstr();
	char sFrom [ SPH_MAX_FILENAME_LEN ];
	char sTo [ SPH_MAX_FILENAME_LEN ];
	struct stat tFileInfo; 

	int iExt;
	for ( iExt=0; iExt<EXT_COUNT; iExt++ )
	{
		snprintf ( sFrom, sizeof(sFrom), "%s.%s.tmp", sPath, g_dExt[iExt] );
		sFrom [ sizeof(sFrom)-1 ] = '\0';

		if ( g_bRotate )
			snprintf ( sTo, sizeof(sTo), "%s.new.%s", sPath, g_dExt[iExt] );
		else
			snprintf ( sTo, sizeof(sTo), "%s.%s", sPath, g_dExt[iExt] );

		sTo [ sizeof(sTo)-1 ] = '\0';

		if ( !stat( sTo, &tFileInfo ) )
		{		
			if ( remove ( sTo ) )
			{
				fprintf ( stdout, "ERROR: index '%s': failed to delete '%s': %s",
					sDst, sTo, strerror(errno) );
				break;			
			}
		}

		if ( rename ( sFrom, sTo ) )
		{
			fprintf ( stdout, "ERROR: index '%s': failed to rename '%s' to '%s': %s",
				sDst, sFrom, sTo, strerror(errno) );
			break;
		}
	}

	if ( !bRotate )
	{
		pSrc->Unlock();
		pDst->Unlock();
	}

	SafeDelete ( pSrc );
	SafeDelete ( pDst );

	// all good?
	return ( iExt==EXT_COUNT );
}

//////////////////////////////////////////////////////////////////////////
// ENTRY
//////////////////////////////////////////////////////////////////////////

int main ( int argc, char ** argv )
{
	const char * sOptConfig = NULL;
	bool bMerge = false;	
	CSphVector<CSphFilter> dMergeDstFilters;

	CSphVector<const char *> dIndexes;
	bool bIndexAll = false;

	int i;
	for ( i=1; i<argc; i++ )
	{
		if ( ( !strcmp ( argv[i], "--config" ) || !strcmp ( argv[i], "-c" ) ) && (i+1)<argc )
		{
			sOptConfig = argv[++i];
			if ( !sphIsReadable ( sOptConfig ) )
				sphDie ( "config file '%s' does not exist or is not readable", sOptConfig );
		}
		else if ( strcasecmp ( argv[i], "--merge" )==0 && (i+2)<argc )
		{
			bMerge = true;
			dIndexes.Add ( argv[i+1] );
			dIndexes.Add ( argv[i+2] );
			i += 2;
		}
		else if ( bMerge && strcasecmp ( argv[i], "--merge-dst-range" )==0 && (i+3)<argc )
		{
			dMergeDstFilters.Resize ( dMergeDstFilters.GetLength()+1 );
			dMergeDstFilters.Last().m_eType = SPH_FILTER_RANGE;
			dMergeDstFilters.Last().m_sAttrName = argv[i+1];
			dMergeDstFilters.Last().m_uMinValue = (SphAttr_t) strtoull ( argv[i+2], NULL, 10 );
			dMergeDstFilters.Last().m_uMaxValue = (SphAttr_t) strtoull ( argv[i+3], NULL, 10 );
			i += 3;
		}
		else if ( strcasecmp ( argv[i], "--buildstops" )==0 && (i+2)<argc )
		{
			g_sBuildStops = argv[i+1];
			g_iTopStops = atoi ( argv[i+2] );
			if ( g_iTopStops<=0 )
				break;
			i += 2;

		} else if ( strcasecmp ( argv[i], "--rotate" )==0 )
		{
			g_bRotate = true;

		} else if ( strcasecmp ( argv[i], "--buildfreqs" )==0 )
		{
			g_bBuildFreqs = true;

		} else if ( strcasecmp ( argv[i], "--quiet" )==0 )
		{
			g_bQuiet = true;
			sphSetQuiet ( true );

		} else if ( strcasecmp ( argv[i], "--noprogress" )==0 )
		{
			g_bProgress = false;

		} else if ( strcasecmp ( argv[i], "--all" )==0 )
		{
			bIndexAll = true;

		} else if ( sphIsAlpha(argv[i][0]) )
		{
			dIndexes.Add ( argv[i] );

		} else
		{
			break;
		}
	}

	if ( !g_bQuiet )
		fprintf ( stdout, SPHINX_BANNER );

	if ( !isatty ( fileno(stdout) ) )
		g_bProgress = false;

	if ( i!=argc || argc<2 )
	{
		if ( argc>1 )
		{
			fprintf ( stdout, "ERROR: malformed or unknown option near '%s'.\n", argv[i] );
		
		} else
		{
			fprintf ( stdout,
				"Usage: indexer [OPTIONS] [indexname1 [indexname2 [...]]]\n"
				"\n"
				"Options are:\n"
				"--config <file>\t\tread configuration from specified file\n"
				"\t\t\t(default is sphinx.conf)\n"
				"--all\t\t\treindex all configured indexes\n"
				"--quiet\t\t\tbe quiet, only print errors\n"
				"--noprogress\t\tdo not display progress\n"
				"\t\t\t(automatically on if output is not to a tty)\n"
#if !USE_WINDOWS
				"--rotate\t\tsend SIGHUP to searchd when indexing is over\n"
				"\t\t\tto rotate updated indexes automatically\n"
#endif
				"--buildstops <output.txt> <N>\n"
				"\t\t\tbuild top N stopwords and write them to given file\n"
				"--buildfreqs\t\tstore words frequencies to output.txt\n"
				"\t\t\t(used with --buildstops only)\n"
				"--merge <dst-index> <src-index>\n"
				"\t\t\tmerge 'src-index' into 'dst-index'\n"
				"\t\t\t'dst-index' will receive merge result\n"
				"\t\t\t'src-index' will not be modified\n"
				"--merge-dst-range <attr> <min> <max>\n"
				"\t\t\tfilter 'dst-index' on merge, keep only those documents\n"
				"\t\t\twhere 'attr' is between 'min' and 'max' (inclusive)\n"
				"\n"
				"Examples:\n"
				"indexer --quiet myidx1\treindex 'myidx1' defined in 'sphinx.conf'\n"
				"indexer --all\t\treindex all indexes defined in 'sphinx.conf'\n" );
		}

		return 1;
	}

	///////////////
	// load config
	///////////////

	// fallback to defaults if there was no explicit config specified
	while ( !sOptConfig )
	{
#ifdef SYSCONFDIR
		sOptConfig = SYSCONFDIR "/sphinx.conf";
		if ( sphIsReadable(sOptConfig) )
			break;
#endif

		sOptConfig = "./sphinx.conf";
		if ( sphIsReadable(sOptConfig) )
			break;

		sOptConfig = NULL;
		break;
	}

	if ( !sOptConfig )
		sphDie ( "no readable config file (looked in "
#ifdef SYSCONFDIR
			SYSCONFDIR "/sphinx.conf, "
#endif
			"./sphinx.conf)" );

	if ( !g_bQuiet )
		fprintf ( stdout, "using config file '%s'...\n", sOptConfig );

	CSphConfigParser cp;
	if ( !cp.Parse ( sOptConfig ) )
	{
		fprintf ( stdout, "FATAL: failed to parse config file '%s'.\n", sOptConfig );
		return 1;
	}
	const CSphConfig & hConf = cp.m_tConf;

	if ( !hConf.Exists ( "source" ) )
	{
		fprintf ( stdout, "FATAL: no sources found in config file.\n" );
		return 1;
	}
	if ( !hConf.Exists ( "index" ) )
	{
		fprintf ( stdout, "FATAL: no indexes found in config file.\n" );
		return 1;
	}

	// configure memlimit
	g_iMemLimit = 0;
	if ( hConf("indexer")
		&& hConf["indexer"]("indexer") )
	{
		if ( hConf["indexer"]["indexer"]("mem_limit") )
		{
			CSphString sBuf = hConf["indexer"]["indexer"]["mem_limit"];

			char sMemLimit[256];
			strncpy ( sMemLimit, sBuf.cstr(), sizeof(sMemLimit) );

			int iLen = strlen ( sMemLimit );
			if ( iLen )
			{
				iLen--;
				int iScale = 1;
				if ( toupper(sMemLimit[iLen])=='K' )
				{
					iScale = 1024;
					sMemLimit[iLen] = '\0';
				} else if ( toupper(sMemLimit[iLen])=='M' )
				{
					iScale = 1048576;
					sMemLimit[iLen] = '\0';
				}

				char * sErr;
				int iRes = strtol ( sMemLimit, &sErr, 10 );
				if ( *sErr )
				{
					fprintf ( stdout, "WARNING: bad mem_limit value '%s', using default.\n", sBuf.cstr() );
				} else
				{
					g_iMemLimit = iScale*iRes;
				}
			}
		}

		sphSetThrottling ( hConf["indexer"]["indexer"].GetInt ( "max_iops", 0 ),
						   hConf["indexer"]["indexer"].GetInt ( "max_iosize", 0 ));
	}

	/////////////////////
	// index each index
	////////////////////

	bool bIndexedOk = false; // if any of the indexes are ok
	if ( bMerge )
	{
		if ( dIndexes.GetLength()!=2 )
			sphDie ( "there must be 2 indexes to merge specified" );

		if ( !hConf["index"](dIndexes[0]) )
			sphDie ( "no merge destination index '%s'", dIndexes[0] );

		if ( !hConf["index"](dIndexes[1]) )
			sphDie ( "no merge source index '%s'", dIndexes[1] );

		bIndexedOk = DoMerge (
			hConf["index"][dIndexes[0]], dIndexes[0],
			hConf["index"][dIndexes[1]], dIndexes[1], dMergeDstFilters, g_bRotate );
	} else if ( bIndexAll )
	{
		hConf["index"].IterateStart ();
		while ( hConf["index"].IterateNext() )
			bIndexedOk |= DoIndex ( hConf["index"].IterateGet (), hConf["index"].IterateGetKey().cstr(), hConf["source"] );
	} else
	{
		ARRAY_FOREACH ( i, dIndexes )
		{
			if ( !hConf["index"](dIndexes[i]) )
				fprintf ( stdout, "WARNING: no such index '%s', skipping.\n", dIndexes[i] );
			else
				bIndexedOk |= DoIndex ( hConf["index"][dIndexes[i]], dIndexes[i], hConf["source"] );
		}
	}

	sphShutdownWordforms ();

	////////////////////////////
	// rotating searchd indices
	////////////////////////////

	if ( bIndexedOk )
	{
		bool bOK = false;
		while ( g_bRotate )
		{
			// load config
			if ( !hConf.Exists ( "searchd" ) )
			{
				fprintf ( stdout, "WARNING: 'searchd' section not found in config file.\n" );
				break;
			}

			const CSphConfigSection & hSearchd = hConf["searchd"]["searchd"];
			if ( !hSearchd.Exists ( "pid_file" ) )
			{
				fprintf ( stdout, "WARNING: 'pid_file' parameter not found in 'searchd' config section.\n" );
				break;
			}

			// read in PID
			int iPID;
			FILE * fp = fopen ( hSearchd["pid_file"].cstr(), "r" );
			if ( !fp )
			{
				fprintf ( stdout, "WARNING: failed to open pid_file '%s'.\n", hSearchd["pid_file"].cstr() );
				break;
			}
			if ( fscanf ( fp, "%d", &iPID )!=1 || iPID<=0 )
			{
				fprintf ( stdout, "WARNING: failed to scanf pid from pid_file '%s'.\n", hSearchd["pid_file"].cstr() );
				break;
			}
			fclose ( fp );

#if USE_WINDOWS
			char szPipeName [64];
			sprintf ( szPipeName, "\\\\.\\pipe\\searchd_%d", iPID );

			HANDLE hPipe = INVALID_HANDLE_VALUE;

			while ( hPipe == INVALID_HANDLE_VALUE ) 
			{ 
				hPipe = CreateFile ( szPipeName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );

				if ( hPipe == INVALID_HANDLE_VALUE ) 
				{
					if ( GetLastError () != ERROR_PIPE_BUSY ) 
					{
						fprintf ( stdout, "WARNING: could not open pipe (GetLastError()=%d)\n", GetLastError () );
						break;
					}

					if ( !WaitNamedPipe ( szPipeName, 1000 ) ) 
					{ 
						fprintf ( stdout, "WARNING: could not open pipe (GetLastError()=%d)\n", GetLastError () );
						break;
					} 
				}
			} 

			if ( hPipe != INVALID_HANDLE_VALUE )
			{	
				DWORD uWritten = 0;
				BYTE uWrite = 0;
				BOOL bResult = WriteFile ( hPipe, &uWrite, 1, &uWritten, NULL );
				if ( bResult )
					fprintf ( stdout, "rotating indices: succesfully sent SIGHUP to searchd (pid=%d).\n", iPID );
				else
					fprintf ( stdout, "WARNING: failed to send SIGHUP to searchd (pid=%d, GetLastError()=%d)\n", iPID, GetLastError () );

				CloseHandle ( hPipe );
			}
#else
			// signal
			int iErr = kill ( iPID, SIGHUP );
			if ( iErr==0 )
			{
				if ( !g_bQuiet )
					fprintf ( stdout, "rotating indices: succesfully sent SIGHUP to searchd (pid=%d).\n", iPID );
			} else
			{
				switch ( errno )
				{
					case ESRCH:	fprintf ( stdout, "WARNING: no process found by PID %d.\n", iPID ); break;
					case EPERM:	fprintf ( stdout, "WARNING: access denied to PID %d.\n", iPID ); break;
					default:	fprintf ( stdout, "WARNING: kill() error: %s.\n", strerror(errno) ); break;
				}
				break;
			}
#endif

			// all ok
			bOK = true;
			break;
		}

		if ( g_bRotate )
		{
			if ( !bOK )
				fprintf ( stdout, "WARNING: indices NOT rotated.\n" );
		}
	}

#if SPH_DEBUG_LEAKS
	sphAllocsStats ();
#endif

	return bIndexedOk ? 0 : 1;
}

//
// $Id$
//
