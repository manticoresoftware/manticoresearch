//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
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
	: m_sFileName ( NULL )
	, m_iLine ( -1 )
{
}


CSphConfigParser::~CSphConfigParser ()
{
	sphFree ( m_sFileName );
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
			m_sFileName, m_iLine, sKey, m_sSectionName.cstr() );
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
		m_tConf.Add ( CSphConfigType(), m_sSectionType );

	if ( m_tConf[m_sSectionType].Exists ( m_sSectionName ) )
		return false;

	m_tConf[m_sSectionType].Add ( CSphConfigSection(), m_sSectionName );
	return true;
}


bool CSphConfigParser::AddKey ( const char * sKey, char * sValue )
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
		tSec.Add ( sValue, sKey );
	}
	return true;
}


bool CSphConfigParser::Parse ( const char * file )
{
	const int L_STEPBACK	= 16;
	const int L_TOKEN		= 64;
	const int L_BUFFER		= 256;
	const int L_ERROR		= 1024;

	// open file
	FILE * fp = fopen ( file, "rb" );
	if ( !fp )
		return 0;

	// init parser
	m_sFileName = sphDup ( file );
	m_iLine = 1;

	char * p = NULL;
	char * pEnd = NULL;
	char sBuf [ L_BUFFER ];
	char sStepback [ L_STEPBACK+1 ];
	char sToken [ L_TOKEN ];
	int iToken = 0;
	int iCh = -1;

	const char * sLine = p;
	char sError [ L_ERROR ];
	int iStepback = 0;
	int iError = 0;

	enum { TOP, SKIP2NL, TOK, TYPE, SEC, CHR, VALUE, SECNAME, SECBASE } eState = TOP, eStack[8];
	int iStack = 0;

	int iValue = 0, iValueMax = 256;
	char * sValue = (char*) sphMalloc ( iValueMax );

	#define LOC_ERROR(_msg) { strncpy ( sError, _msg, sizeof(sError) ); iError = 1; break; }
	#define LOC_ERROR2(_msg,_a) { snprintf ( sError, sizeof(sError), _msg, _a ); iError = 1; break; }
	#define LOC_ERROR3(_msg,_a,_b) { snprintf ( sError, sizeof(sError), _msg, _a, _b ); iError = 1; break; }
	#define LOC_ERROR4(_msg,_a,_b,_c) { snprintf ( sError, sizeof(sError), _msg, _a, _b, _c ); iError = 1; break; }
	#define LOC_PUSH(_new) { assert ( iStack<int(sizeof(eStack)/sizeof(eState)) ); eStack[iStack++] = eState; eState = _new; }
	#define LOC_POP() { assert ( iStack>0 ); eState = eStack[--iStack]; }

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

		// handle TOP state
		if ( eState==TOP )
		{
			if ( isspace(*p) )				continue;
			if ( *p=='#' )					{ LOC_PUSH ( SKIP2NL ); continue; }
			if ( !sphIsAlpha(*p) )			LOC_ERROR ( "invalid token" );
											iToken = 0; LOC_PUSH ( TYPE ); LOC_PUSH ( TOK ); p--; continue;
		}

		// handle SKIP2NL state
		if ( eState==SKIP2NL )
		{
			if ( *p=='\n' )					{ LOC_POP (); continue; }
											continue;
		}

		// handle TOK state
		if ( eState==TOK )
		{
			if ( !iToken && !sphIsAlpha(*p) )LOC_ERROR ( "internal error (non-alpha in TOK pos 0)" );
			if ( iToken==sizeof(sToken) )	LOC_ERROR ( "token too long" );
			if ( !sphIsAlpha(*p) )			{ LOC_POP (); sToken [ iToken ] = '\0'; iToken = 0; p--; continue; }
			if ( !iToken )					{ sToken[0] = '\0'; }
											sToken [ iToken++ ] = *p; continue;
		}

		// handle TYPE state
		if ( eState==TYPE )
		{
			if ( isspace(*p) )				continue;
			if ( *p=='#' )					{ LOC_PUSH ( SKIP2NL ); continue; }
			if ( !sToken[0] )				{ LOC_ERROR ( "internal error (empty token in TYPE)" ); }
			if ( IsPlainSection(sToken) )	{ AddSection ( sToken, sToken ); sToken[0] = '\0'; LOC_POP (); LOC_PUSH ( SEC ); LOC_PUSH ( CHR ); iCh = '{'; p--; continue; }
			if ( IsNamedSection(sToken) )	{  m_sSectionType = sToken; sToken[0] = '\0'; LOC_POP (); LOC_PUSH ( SECNAME ); p--; continue; }
											LOC_ERROR2 ( "invalid section type '%s'", sToken );
		}

		// handle CHR state
		if ( eState==CHR )
		{
			if ( isspace(*p) )				continue;
			if ( *p=='#' )					{ LOC_PUSH ( SKIP2NL ); continue; }
			if ( *p!=iCh )					LOC_ERROR3 ( "expected '%c', got '%c'", iCh, *p );
											LOC_POP (); continue;
		}

		// handle SEC state
		if ( eState==SEC )
		{
			if ( isspace(*p) )				continue;
			if ( *p=='#' )					{ LOC_PUSH ( SKIP2NL ); continue; }
			if ( *p=='}' )					{ LOC_POP (); continue; }
			if ( sphIsAlpha(*p) )			{ LOC_PUSH ( VALUE ); LOC_PUSH ( CHR ); iCh = '='; LOC_PUSH ( TOK ); p--; iValue = 0; sValue[0] = '\0'; continue; }
											LOC_ERROR2 ( "section contents: expected token, got '%c'", *p );

		}

		// handle VALUE state
		if ( eState==VALUE )
		{
			if ( *p=='\n' )					{ AddKey ( sToken, sValue ); iValue = 0; LOC_POP (); continue; }
			if ( *p=='#' )					{ AddKey ( sToken, sValue ); iValue = 0; LOC_POP (); LOC_PUSH ( SKIP2NL ); continue; }
			if ( *p=='\\' )					{ LOC_PUSH ( SKIP2NL ); continue; }

			if ( iValue==iValueMax )		{ iValueMax *= 2; sValue = (char*) sphRealloc ( sValue, iValueMax ); }
											sValue[iValue++] = *p; sValue[iValue] = '\0'; continue;
		}

		// handle SECNAME state
		if ( eState==SECNAME )
		{
			if ( isspace(*p) )				{ continue; }
			if ( !sToken[0]&&!sphIsAlpha(*p)){ LOC_ERROR2 ( "named section: expected name, got '%c'", *p ); }
			if ( !sToken[0] )				{ LOC_PUSH ( TOK ); p--; continue; }
											if ( !AddSection ( m_sSectionType.cstr(), sToken ) ) LOC_ERROR3 ( "section '%s' (type=%s) already exists", sToken, m_sSectionType.cstr() ); sToken[0] = '\0';

			if ( *p==':' )					{ eState = SECBASE; continue; }
			if ( *p=='{' )					{ eState = SEC; continue; }
											LOC_ERROR2 ( "named section: expected ':' or '{', got '%c'", *p );
		}

		// handle SECBASE state
		if ( eState==SECBASE )
		{
			if ( isspace(*p) )				{ continue; }
			if ( !sToken[0]&&!sphIsAlpha(*p)){ LOC_ERROR2 ( "named section: expected parent name, got '%c'", *p ); }
			if ( !sToken[0] )				{ LOC_PUSH ( TOK ); p--; continue; }

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

			p--;
			eState = SEC;
			LOC_PUSH ( CHR );
			iCh = '{';
			continue;
		}

		LOC_ERROR2 ( "internal error (unhandled state %d)", eState );
	}

	#undef LOC_POP
	#undef LOC_PUSH
	#undef LOC_ERROR

	fclose ( fp );
	sphFree ( sValue );

	if ( iError )
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

		fprintf ( stdout, "%s line %d col %d: %s near '%s'\n", m_sFileName, m_iLine, int(p-sLine+1), sError, sStepback );
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////

DWORD sphParseMorphology ( const CSphString & sOption, bool bUseUTF8 )
{
	DWORD iMorph = SPH_MORPH_UNKNOWN;
	DWORD iStemRu = ( bUseUTF8 ? SPH_MORPH_STEM_RU_UTF8 : SPH_MORPH_STEM_RU_CP1251 );

	if ( sOption=="stem_en" )
		iMorph = SPH_MORPH_STEM_EN;

	else if ( sOption=="stem_ru" )
		iMorph = iStemRu;

	else if ( sOption=="stem_enru" )
		iMorph = iStemRu | SPH_MORPH_STEM_EN;

	else if ( sOption=="soundex" )
		iMorph = SPH_MORPH_SOUNDEX;

	else if ( sOption.IsEmpty() || sOption=="none" )
		iMorph = SPH_MORPH_NONE;

	return iMorph;
}

//
// $Id$
//
