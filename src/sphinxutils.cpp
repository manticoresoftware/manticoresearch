//
// $Id$
//

//
// Copyright (c) 2001-2007, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

/// @file sphinxutils.cpp
/// Implementations for Sphinx utilities shared classes.

#include "sphinx.h"
#include "sphinxutils.h"
#include <ctype.h>
#include <fcntl.h>

#if USE_WINDOWS
#include <io.h> // for ::open on windows
#endif

/////////////////////////////////////////////////////////////////////////////

static char * ltrim ( char * sLine )
{
	while ( *sLine && isspace(*sLine) )
		sLine++;
	return sLine;
}


static char * rtrim ( char * sLine )
{
	char * p = sLine + strlen(sLine) - 1;
	while ( p>=sLine && isspace(*p) )
		p--;
	p[1] = '\0';
	return sLine;
}


static char * trim ( char * sLine )
{
	return ltrim ( rtrim ( sLine ) );
}

//////////////////////////////////////////////////////////////////////////
// CONFIG PARSER
//////////////////////////////////////////////////////////////////////////

/// key flags
enum
{
	KEY_DEPRECATED		= 1UL<<0,
	KEY_LIST			= 1UL<<1
};

/// key descriptor for validation purposes
struct KeyDesc_t
{
	const char *		m_sKey;		///< key name
	int					m_iFlags;	///< flags
	const char *		m_sExtra;	///< extra stuff (deprecated name, for now)
};

/// allowed keys for source section
static KeyDesc_t g_dKeysSource[] =
{
	{ "type",					0, NULL },
	{ "strip_html",				0, NULL },
	{ "index_html_attrs",		0, NULL },
	{ "sql_host",				0, NULL },
	{ "sql_user",				0, NULL },
	{ "sql_pass",				0, NULL },
	{ "sql_db",					0, NULL },
	{ "sql_port",				0, NULL },
	{ "sql_sock",				0, NULL },
	{ "mysql_connect_flags",	0, NULL },
	{ "sql_query_pre",			KEY_LIST, NULL },
	{ "sql_query",				0, NULL },
	{ "sql_query_range",		0, NULL },
	{ "sql_range_step",			0, NULL },
	{ "sql_attr_uint",			KEY_LIST, NULL },
	{ "sql_attr_bool",			KEY_LIST, NULL },
	{ "sql_attr_timestamp",		KEY_LIST, NULL },
	{ "sql_attr_str2ordinal",	KEY_LIST, NULL },
	{ "sql_attr_float",			KEY_LIST, NULL },
	{ "sql_attr_multi",			KEY_LIST, NULL },
	{ "sql_query_post",			KEY_LIST, NULL },
	{ "sql_query_post_index",	KEY_LIST, NULL },
	{ "sql_ranged_throttle",	0, NULL },
	{ "sql_query_info",			0, NULL },
	{ "xmlpipe_command",		0, NULL },
	{ "sql_group_column",		KEY_LIST | KEY_DEPRECATED, "sql_attr_uint"  },
	{ "sql_date_column",		KEY_LIST | KEY_DEPRECATED, "sql_attr_timestamp" },
	{ "sql_str2ordinal_column",	KEY_LIST | KEY_DEPRECATED, "sql_attr_str2ordinal" },
	{ NULL,						0, NULL }
};

/// allowed keys for index section
static KeyDesc_t g_dKeysIndex[] =
{
	{ "source",					KEY_LIST, NULL },
	{ "path",					0, NULL },
	{ "docinfo",				0, NULL },
	{ "mlock",					0, NULL },
	{ "morphology",				0, NULL },
	{ "stopwords",				0, NULL },
	{ "synonyms",				0, NULL },
	{ "min_word_len",			0, NULL },
	{ "charset_type",			0, NULL },
	{ "charset_table",			0, NULL },
	{ "min_prefix_len",			0, NULL },
	{ "min_infix_len",			0, NULL },
	{ "prefix_fields",			0, NULL },
	{ "infix_fields",			0, NULL },
	{ "enable_star",			0, NULL },
	{ "ngram_len",				0, NULL },
	{ "ngram_chars",			0, NULL },
	{ "phrase_boundary",		0, NULL },
	{ "phrase_boundary_step",	0, NULL },
	{ "type",					0, NULL },
	{ "local",					KEY_LIST, NULL },
	{ "agent",					KEY_LIST, NULL },
	{ "agent_connect_timeout",	0, NULL },
	{ "agent_query_timeout",	0, NULL },
	{ NULL,						0, NULL }
};

/// allowed keys for indexer section
static KeyDesc_t g_dKeysIndexer[] =
{
	{ "mem_limit",				0, NULL },
	{ NULL,						0, NULL }
};

/// allowed keys for searchd section
static KeyDesc_t g_dKeysSearchd[] =
{
	{ "address",				0, NULL },
	{ "port",					0, NULL },
	{ "log",					0, NULL },
	{ "query_log",				0, NULL },
	{ "read_timeout",			0, NULL },
	{ "max_children",			0, NULL },
	{ "pid_file",				0, NULL },
	{ "max_matches",			0, NULL },
	{ "seamless_rotate",		0, NULL },
	{ NULL,						0, NULL }
};

//////////////////////////////////////////////////////////////////////////

CSphConfigParser::CSphConfigParser ()
	: m_sFileName ( "" )
	, m_iLine ( -1 )
{
}


bool CSphConfigParser::IsPlainSection ( const char * sKey )
{
	if ( !strcasecmp ( sKey, "indexer" ) )		return true;
	if ( !strcasecmp ( sKey, "searchd" ) )		return true;
	if ( !strcasecmp ( sKey, "search" ) )		return true;
	return false;
}


bool CSphConfigParser::IsNamedSection ( const char * sKey )
{
	if ( !strcasecmp ( sKey, "source" ) )		return true;
	if ( !strcasecmp ( sKey, "index" ) )		return true;
	return false;
}


bool CSphConfigParser::AddSection ( const char * sType, const char * sName )
{
	m_sSectionType = sType;
	m_sSectionName = sName;

	if ( !m_tConf.Exists ( m_sSectionType ) )
		m_tConf.Add ( CSphConfigType(), m_sSectionType ); // FIXME! be paranoid, verify that it returned true

	if ( m_tConf[m_sSectionType].Exists ( m_sSectionName ) )
	{
		snprintf ( m_sError, sizeof(m_sError), "section '%s' (type='%s') already exists", sName, sType );
		return false;
	}
	m_tConf[m_sSectionType].Add ( CSphConfigSection(), m_sSectionName ); // FIXME! be paranoid, verify that it returned true

	return true;
}


void CSphConfigParser::AddKey ( const char * sKey, char * sValue )
{
	assert ( m_tConf.Exists ( m_sSectionType ) );
	assert ( m_tConf[m_sSectionType].Exists ( m_sSectionName ) );

	sValue = trim ( sValue );
	CSphConfigSection & tSec = m_tConf[m_sSectionType][m_sSectionName];
	if ( tSec(sKey) )
	{
		if ( tSec[sKey].m_bTag )
		{
			// override value or list with a new value
			SafeDelete ( tSec[sKey].m_pNext ); // only leave the first array element
			tSec[sKey] = sValue; // update its value
			tSec[sKey].m_bTag = false; // mark it as overridden

		} else
		{
			// chain to tail, to keep the order
			CSphVariant * pTail = &tSec[sKey];
			while ( pTail->m_pNext )
				pTail = pTail->m_pNext;
			pTail->m_pNext = new CSphVariant ( sValue );
		}

	} else
	{
		// just add
		tSec.Add ( sValue, sKey ); // FIXME! be paranoid, verify that it returned true
	}
}


bool CSphConfigParser::ValidateKey ( const char * sKey )
{
	// get proper descriptor table
	// OPTIMIZE! move lookup to AddSection
	const KeyDesc_t * pDesc = NULL;
	if ( m_sSectionType=="source" )			pDesc = g_dKeysSource;
	else if ( m_sSectionType=="index" )		pDesc = g_dKeysIndex;
	else if ( m_sSectionType=="indexer" )	pDesc = g_dKeysIndexer;
	else if ( m_sSectionType=="searchd" )	pDesc = g_dKeysSearchd;
	if ( !pDesc )
	{
		snprintf ( m_sError, sizeof(m_sError), "unknown section type '%s'", m_sSectionType.cstr() );
		return false;
	}

	// check if the key is known
	while ( pDesc->m_sKey && strcasecmp ( pDesc->m_sKey, sKey ) )
		pDesc++;
	if ( !pDesc->m_sKey )
	{
		snprintf  ( m_sError, sizeof(m_sError), "unknown key name '%s'", sKey );
		return false;
	}

	// warn about deprecate keys
	if ( pDesc->m_iFlags & KEY_DEPRECATED )
		if ( ++m_iWarnings<=WARNS_THRESH )
			fprintf ( stdout, "WARNING: key '%s' is deprecated in %s line %d; use '%s' instead.\n", sKey, m_sFileName.cstr(), m_iLine, pDesc->m_sExtra );

	// warn about list/non-list keys
	if (!( pDesc->m_iFlags & KEY_LIST ))
	{
		CSphConfigSection & tSec = m_tConf[m_sSectionType][m_sSectionName];
		if ( tSec(sKey) && !tSec[sKey].m_bTag )
			if ( ++m_iWarnings<=WARNS_THRESH )
				fprintf ( stdout, "WARNING: key '%s' is not multi-value; value in %s line %d will be ignored.\n", sKey, m_sFileName.cstr(), m_iLine );
	}

	return true;
}


bool CSphConfigParser::Parse ( const char * sFileName )
{
	const int L_STEPBACK	= 16;
	const int L_TOKEN		= 64;
	const int L_BUFFER		= 8192;

	// open file
	FILE * fp = fopen ( sFileName, "rb" );
	if ( !fp )
		return false;

	// init parser
	m_sFileName = sFileName;
	m_iLine = 0;
	m_iWarnings = 0;

	char * p = NULL;
	char * pEnd = NULL;

	char sBuf [ L_BUFFER ];
	char sToken [ L_TOKEN ];
	int iToken = 0;
	int iCh = -1;

	enum { S_TOP, S_SKIP2NL, S_TOK, S_TYPE, S_SEC, S_CHR, S_VALUE, S_SECNAME, S_SECBASE, S_KEY } eState = S_TOP, eStack[8];
	int iStack = 0;

	int iValue = 0, iValueMax = 65535;
	char * sValue = new char [ iValueMax+1 ];

	#define LOC_ERROR(_msg) { strncpy ( m_sError, _msg, sizeof(m_sError) ); break; }
	#define LOC_ERROR2(_msg,_a) { snprintf ( m_sError, sizeof(m_sError), _msg, _a ); break; }
	#define LOC_ERROR3(_msg,_a,_b) { snprintf ( m_sError, sizeof(m_sError), _msg, _a, _b ); break; }
	#define LOC_ERROR4(_msg,_a,_b,_c) { snprintf ( m_sError, sizeof(m_sError), _msg, _a, _b, _c ); break; }

	#define LOC_PUSH(_new) { assert ( iStack<int(sizeof(eStack)/sizeof(eState)) ); eStack[iStack++] = eState; eState = _new; }
	#define LOC_POP() { assert ( iStack>0 ); eState = eStack[--iStack]; }
	#define LOC_BACK() { p--; }

	m_sError[0] = '\0';

	for ( ; ; p++ )
	{
		// if this line is over, load next line
		if ( p>=pEnd )
		{
			if ( !fgets ( sBuf, L_BUFFER, fp ) )
				break; // FIXME! check for read error

			m_iLine++;
			int iLen = strlen(sBuf);
			if ( iLen<=0 )
				LOC_ERROR ( "internal error; fgets() returned empty string" );

			p = sBuf;
			pEnd = sBuf + iLen;
			if ( pEnd[-1]!='\n' )
			{
				if ( iLen==L_BUFFER-1 )
					LOC_ERROR ( "line too long" );
			}
		}

		// handle S_TOP state
		if ( eState==S_TOP )
		{
			if ( isspace(*p) )				continue;
			if ( *p=='#' )					{ LOC_PUSH ( S_SKIP2NL ); continue; }
			if ( !sphIsAlpha(*p) )			LOC_ERROR ( "invalid token" );
											iToken = 0; LOC_PUSH ( S_TYPE ); LOC_PUSH ( S_TOK ); LOC_BACK(); continue;
		}

		// handle S_SKIP2NL state
		if ( eState==S_SKIP2NL )
		{
			LOC_POP ();
			p = pEnd;
			continue;
		}

		// handle S_TOK state
		if ( eState==S_TOK )
		{
			if ( !iToken && !sphIsAlpha(*p) )LOC_ERROR ( "internal error (non-alpha in S_TOK pos 0)" );
			if ( iToken==sizeof(sToken) )	LOC_ERROR ( "token too long" );
			if ( !sphIsAlpha(*p) )			{ LOC_POP (); sToken [ iToken ] = '\0'; iToken = 0; LOC_BACK(); continue; }
			if ( !iToken )					{ sToken[0] = '\0'; }
											sToken [ iToken++ ] = *p; continue;
		}

		// handle S_TYPE state
		if ( eState==S_TYPE )
		{
			if ( isspace(*p) )				continue;
			if ( *p=='#' )					{ LOC_PUSH ( S_SKIP2NL ); continue; }
			if ( !sToken[0] )				{ LOC_ERROR ( "internal error (empty token in S_TYPE)" ); }
			if ( IsPlainSection(sToken) )	{ if ( !AddSection ( sToken, sToken ) ) break; sToken[0] = '\0'; LOC_POP (); LOC_PUSH ( S_SEC ); LOC_PUSH ( S_CHR ); iCh = '{'; LOC_BACK(); continue; }
			if ( IsNamedSection(sToken) )	{ m_sSectionType = sToken; sToken[0] = '\0'; LOC_POP (); LOC_PUSH ( S_SECNAME ); LOC_BACK(); continue; }
											LOC_ERROR2 ( "invalid section type '%s'", sToken );
		}

		// handle S_CHR state
		if ( eState==S_CHR )
		{
			if ( isspace(*p) )				continue;
			if ( *p=='#' )					{ LOC_PUSH ( S_SKIP2NL ); continue; }
			if ( *p!=iCh )					LOC_ERROR3 ( "expected '%c', got '%c'", iCh, *p );
											LOC_POP (); continue;
		}

		// handle S_SEC state
		if ( eState==S_SEC )
		{
			if ( isspace(*p) )				continue;
			if ( *p=='#' )					{ LOC_PUSH ( S_SKIP2NL ); continue; }
			if ( *p=='}' )					{ LOC_POP (); continue; }
			if ( sphIsAlpha(*p) )			{ LOC_PUSH ( S_KEY ); LOC_PUSH ( S_TOK ); LOC_BACK(); iValue = 0; sValue[0] = '\0'; continue; }
											LOC_ERROR2 ( "section contents: expected token, got '%c'", *p );

		}

		// handle S_KEY state
		if ( eState==S_KEY )
		{
			// validate the key
			if ( !ValidateKey ( sToken ) ) 
				break;

			// an assignment operator and a value must follow
			LOC_POP (); LOC_PUSH ( S_VALUE ); LOC_PUSH ( S_CHR ); iCh = '='; 
			continue;
		}

		// handle S_VALUE state
		if ( eState==S_VALUE )
		{
			if ( *p=='\n' )					{ AddKey ( sToken, sValue ); iValue = 0; LOC_POP (); continue; }
			if ( *p=='#' )					{ AddKey ( sToken, sValue ); iValue = 0; LOC_POP (); LOC_PUSH ( S_SKIP2NL ); continue; }
			if ( *p=='\\' )					if ( p[1]=='\r' || p[1]=='\n' ) { LOC_PUSH ( S_SKIP2NL ); continue; }
			if ( iValue<iValueMax )			{ sValue[iValue++] = *p; sValue[iValue] = '\0'; }
											continue;
		}

		// handle S_SECNAME state
		if ( eState==S_SECNAME )
		{
			if ( isspace(*p) )				{ continue; }
			if ( !sToken[0]&&!sphIsAlpha(*p)){ LOC_ERROR2 ( "named section: expected name, got '%c'", *p ); }
			if ( !sToken[0] )				{ LOC_PUSH ( S_TOK ); LOC_BACK(); continue; }
											if ( !AddSection ( m_sSectionType.cstr(), sToken ) ) break; sToken[0] = '\0';
			if ( *p==':' )					{ eState = S_SECBASE; continue; }
			if ( *p=='{' )					{ eState = S_SEC; continue; }
											LOC_ERROR2 ( "named section: expected ':' or '{', got '%c'", *p );
		}

		// handle S_SECBASE state
		if ( eState==S_SECBASE )
		{
			if ( isspace(*p) )				{ continue; }
			if ( !sToken[0]&&!sphIsAlpha(*p)){ LOC_ERROR2 ( "named section: expected parent name, got '%c'", *p ); }
			if ( !sToken[0] )				{ LOC_PUSH ( S_TOK ); LOC_BACK(); continue; }

			// copy the section
			assert ( m_tConf.Exists ( m_sSectionType ) );

			if ( !m_tConf [ m_sSectionType ].Exists ( sToken ) )
				LOC_ERROR4 ( "inherited section '%s': parent doesn't exist (parent name='%s', type='%s')", 
					m_sSectionName.cstr(), sToken, m_sSectionType.cstr() );

			CSphConfigSection & tDest = m_tConf [ m_sSectionType ][ m_sSectionName ];
			tDest = m_tConf [ m_sSectionType ][ sToken ];

			// mark all values in the target section as "to be overridden"
			tDest.IterateStart ();
			while ( tDest.IterateNext() )
				tDest.IterateGet().m_bTag = true;

			LOC_BACK();
			eState = S_SEC;
			LOC_PUSH ( S_CHR );
			iCh = '{';
			continue;
		}

		LOC_ERROR2 ( "internal error (unhandled state %d)", eState );
	}

	#undef LOC_POP
	#undef LOC_PUSH
	#undef LOC_ERROR

	fclose ( fp );
	SafeDeleteArray ( sValue );

	if ( m_iWarnings>WARNS_THRESH )
		fprintf ( stdout, "WARNING: %d more warnings skipped.\n", m_iWarnings-WARNS_THRESH );

	if ( strlen(m_sError) )
	{
		int iCol = (int)(p-sBuf+1);

		int iCtx = Min ( L_STEPBACK, iCol ); // error context is upto L_STEPBACK chars back, but never going to prev line
		const char * sCtx = p-iCtx+1;
		if ( sCtx<sBuf )
			sCtx = sBuf;

		char sStepback [ L_STEPBACK+1 ];
		memcpy ( sStepback, sCtx, iCtx );
		sStepback[iCtx] = '\0';

		fprintf ( stdout, "ERROR: %s in %s line %d col %d.\n", m_sError, m_sFileName.cstr(), m_iLine, iCol );
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////

ISphTokenizer * sphConfTokenizer ( const CSphConfigSection & hIndex, CSphString & sError )
{
	// charset_type
	ISphTokenizer * pTokenizer = NULL;

	if ( !hIndex("charset_type") || hIndex["charset_type"]=="sbcs" )
	{
		pTokenizer = sphCreateSBCSTokenizer ();

	} else if ( hIndex["charset_type"]=="utf-8" )
	{
		pTokenizer = hIndex("ngram_chars")
			? sphCreateUTF8NgramTokenizer ()
			: sphCreateUTF8Tokenizer ();

	} else
	{
		sError.SetSprintf ( "unknown charset type '%s'", hIndex["charset_type"].cstr() );
		return NULL;
	}

	assert ( pTokenizer );

	// charset_table
	if ( hIndex("charset_table") )
		if ( !pTokenizer->SetCaseFolding ( hIndex["charset_table"].cstr(), sError ) )
	{
		SafeDelete ( pTokenizer );
		sError.SetSprintf ( "'charset_table': %s", sError.cstr() );
		return NULL;
	}

	// min_word_len
	int iMinWordLen = hIndex("min_word_len") ? Max ( hIndex["min_word_len"].intval(), 0 ) : 0;
	if ( iMinWordLen )
		pTokenizer->SetMinWordLen ( iMinWordLen );

	// ngram_chars
	if ( hIndex("ngram_chars") )
		if ( !pTokenizer->SetNgramChars ( hIndex["ngram_chars"].cstr(), sError ) )
	{
		SafeDelete ( pTokenizer );
		sError.SetSprintf ( "'ngram_chars': %s", sError.cstr() );
		return NULL;
	}

	// ngram_len
	int iNgramLen = hIndex("ngram_len") ? Max ( hIndex["ngram_len"].intval(), 0 ) : 0;
	if ( iNgramLen )
		pTokenizer->SetNgramLen ( iNgramLen );

	// synonyms
	if ( hIndex("synonyms") )
		if ( !pTokenizer->LoadSynonyms ( hIndex["synonyms"].cstr(), sError ) )
	{
		SafeDelete ( pTokenizer );
		sError.SetSprintf ( "'synonyms': %s", sError.cstr() );
		return NULL;
	}

	// phrase boundaries
	int iBoundaryStep = hIndex("phrase_boundary_step") ? Max ( hIndex["phrase_boundary_step"].intval(), 0 ) : 0;
	if ( iBoundaryStep>0 && hIndex("phrase_boundary") )
		if ( !pTokenizer->SetBoundary ( hIndex["phrase_boundary"].cstr(), sError ) )
	{
		SafeDelete ( pTokenizer );
		sError.SetSprintf ( "'phrase_boundary': %s", sError.cstr() );
		return NULL;
	}

	return pTokenizer;
}

//
// $Id$
//
