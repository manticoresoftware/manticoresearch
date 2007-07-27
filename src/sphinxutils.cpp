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

/// known keys for [common] config section
const char * g_dSphKeysCommon[] =
{
	"index_path",
	"morphology",
	"stopwords",
	"charset_type",
	"charset_table",
	NULL
};


/// known keys for [indexer] config section
const char * g_dSphKeysIndexer[] =
{
	"type",
	"sql_host",
	"sql_port",
	"sql_sock",
	"sql_user",
	"sql_pass",
	"sql_db",
	"sql_query_pre",
	"sql_query_range",
	"sql_query",
	"sql_query_post",
	"sql_group_column",
	"sql_date_column",
	"sql_range_step",
	"xmlpipe_command",
	"mem_limit",
	"strip_html",
	"index_html_attrs",
	NULL
};


/// known keys for [searchd] config section
const char * g_dSphKeysSearchd[] =
{
	"port",
	"log",
	"query_log",
	"read_timeout",
	"max_children",
	"pid_file",
	NULL
};


/// known keys for [search] config section
const char * g_dSphKeysSearch[] =
{
	"sql_query_info",
	NULL
};

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

/////////////////////////////////////////////////////////////////////////////
// CONFIG PARSER
/////////////////////////////////////////////////////////////////////////////

CSphConfigParser::CSphConfigParser ()
	: m_sFileName ( "" )
	, m_iLine ( -1 )
{
}


bool CSphConfigParser::ValidateKey ( const char * sKey, const char ** dKnownKeys )
{
	// no validation requested
	if ( dKnownKeys==NULL )
		return true;

	while ( *dKnownKeys )
	{
		if ( strcmp ( *dKnownKeys, sKey )==0 )
			return true;
		dKnownKeys++;
	}

	fprintf ( stdout, "WARNING: error in %s:%d, unknown key '%s' in section [%s]\n",
		m_sFileName.cstr(), m_iLine, sKey, m_sSectionName.cstr() );
	return false;
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


bool CSphConfigParser::Parse ( const char * sFileName )
{
	const int L_STEPBACK	= 16;
	const int L_TOKEN		= 64;
	const int L_BUFFER		= 256;

	// open file
	FILE * fp = fopen ( sFileName, "rb" );
	if ( !fp )
		return 0;

	// init parser
	m_sFileName = sFileName;
	m_iLine = 1;

	char * p = NULL;
	char * pEnd = NULL;
	char sBuf [ L_BUFFER ];
	char sStepback [ L_STEPBACK+1 ];
	char sToken [ L_TOKEN ];
	int iToken = 0;
	int iCh = -1;

	const char * sLine = p;
	int iStepback = 0;

	enum { S_TOP, S_SKIP2NL, S_TOK, S_TYPE, S_SEC, S_CHR, S_VALUE, S_SECNAME, S_SECBASE } eState = S_TOP, eStack[8];
	int iStack = 0;

	int iValue = 0, iValueMax = 65535;
	char * sValue = new char [ iValueMax+1 ];

	#define LOC_ERROR(_msg) { strncpy ( m_sError, _msg, sizeof(m_sError) ); break; }
	#define LOC_ERROR2(_msg,_a) { snprintf ( m_sError, sizeof(m_sError), _msg, _a ); break; }
	#define LOC_ERROR3(_msg,_a,_b) { snprintf ( m_sError, sizeof(m_sError), _msg, _a, _b ); break; }
	#define LOC_ERROR4(_msg,_a,_b,_c) { snprintf ( m_sError, sizeof(m_sError), _msg, _a, _b, _c ); break; }

	#define LOC_PUSH(_new) { assert ( iStack<int(sizeof(eStack)/sizeof(eState)) ); eStack[iStack++] = eState; eState = _new; }
	#define LOC_POP() { assert ( iStack>0 ); eState = eStack[--iStack]; }
	#define LOC_BACK() { if ( *p--=='\n' ) m_iLine--; }

	m_sError[0] = '\0';

	for ( ; ; p++ )
	{
		// if this chunk is over, load next chunk
		if ( p>=pEnd )
		{
			iStepback = Min ( p-sLine, L_STEPBACK );
			memcpy ( sStepback, p-iStepback, iStepback );
			sLine = sBuf-(p-sLine);

			int iLen = fread ( sBuf, 1, sizeof(sBuf), fp );
			if ( iLen<=0 )
				break;

			p = sBuf;
			pEnd = sBuf + iLen;
		}
		if ( *p=='\n' )
		{
			iStepback = 0;
			sLine = p+1;
			m_iLine++;
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
			if ( *p=='\n' )					{ LOC_POP (); continue; }
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
			if ( sphIsAlpha(*p) )			{ LOC_PUSH ( S_VALUE ); LOC_PUSH ( S_CHR ); iCh = '='; LOC_PUSH ( S_TOK ); LOC_BACK(); iValue = 0; sValue[0] = '\0'; continue; }
											LOC_ERROR2 ( "section contents: expected token, got '%c'", *p );

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

	if ( strlen(m_sError) )
	{
		int iCtx = Min ( L_STEPBACK, p-sLine+1 ); // error context is upto L_STEPBACK chars back, but never going to prev line
		const char * sCtx = p-iCtx+1;
		if ( sCtx<sBuf )
		{
			// copy proper amount of chars from stepback buffer
			int iFromStepback = iCtx - (p-sBuf+1);
			assert ( iFromStepback>0 );
			assert ( iFromStepback<=iStepback );
			memmove ( sStepback, sStepback+iStepback-iFromStepback, iFromStepback );

			// add proper amount of chars from main buffer
			assert ( iFromStepback+p-sBuf+1<=L_STEPBACK );
			memcpy ( sStepback+iFromStepback, sBuf, p-sBuf+1 );
			sStepback[iFromStepback+p-sBuf+1] = '\0';
		} else
		{
			memcpy ( sStepback, sLine, iCtx );
			sStepback[iCtx] = '\0';
		}

		fprintf ( stdout, "%s line %d col %d: %s near '%s'\n", m_sFileName.cstr(), m_iLine, int(p-sLine+1),
			m_sError, sStepback );
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////

ISphTokenizer * sphConfTokenizer ( const CSphConfigSection & hIndex, CSphString & sError )
{
	char sErrorBuf[256];

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
		snprintf ( sErrorBuf, sizeof(sErrorBuf), "unknown charset type '%s'", hIndex["charset_type"].cstr() );
		sError = sErrorBuf;
		return NULL;
	}

	assert ( pTokenizer );

	// charset_table
	if ( hIndex("charset_table") )
		if ( !pTokenizer->SetCaseFolding ( hIndex["charset_table"].cstr(), sError ) )
	{
		SafeDelete ( pTokenizer );

		snprintf ( sErrorBuf, sizeof(sErrorBuf), "'charset_table': %s", sError.cstr() );
		sError = sErrorBuf;
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

		snprintf ( sErrorBuf, sizeof(sErrorBuf), "'ngram_chars': %s", sError.cstr() );
		sError = sErrorBuf;
		return NULL;
	}

	// ngram_len
	int iNgramLen = hIndex("ngram_len") ? Max ( hIndex["ngram_len"].intval(), 0 ) : 0;
	if ( iNgramLen )
		pTokenizer->SetNgramLen ( iNgramLen );

	return pTokenizer;
}

//
// $Id$
//
