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

/// @file sphinxutils.cpp
/// Implementations for Sphinx utilities shared classes.

#include "sphinx.h"
#include "sphinxutils.h"
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#if USE_WINDOWS
	#include <io.h> // for ::open on windows
#else
	#include <sys/wait.h>
	#include <signal.h>
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

int CSphConfigSection::GetSize ( const char * sKey, int iDefault ) const
{
	CSphVariant * pEntry = (*this)( sKey );
	if ( !pEntry )
		return iDefault;

	char sMemLimit[256];
	strncpy ( sMemLimit, pEntry->cstr(), sizeof(sMemLimit) );
	sMemLimit [ sizeof(sMemLimit)-1 ] = '\0';

	int iLen = strlen ( sMemLimit );
	if ( !iLen )
		return iDefault;

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
	if ( !*sErr )
		return iScale*iRes;

	// FIXME! report syntax error here
	return iDefault;
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
	{ "strip_html",				KEY_DEPRECATED, "html_strip (per-index)" },
	{ "index_html_attrs",		KEY_DEPRECATED, "html_index_attrs (per-index)" },
	{ "sql_host",				0, NULL },
	{ "sql_user",				0, NULL },
	{ "sql_pass",				0, NULL },
	{ "sql_db",					0, NULL },
	{ "sql_port",				0, NULL },
	{ "sql_sock",				0, NULL },
	{ "mysql_connect_flags",	0, NULL },
	{ "mysql_ssl_key",			0, NULL },
	{ "mysql_ssl_cert",			0, NULL },
	{ "mysql_ssl_ca",			0, NULL },
	{ "mssql_winauth",			0, NULL },
	{ "mssql_unicode",			0, NULL },
	{ "sql_query_pre",			KEY_LIST, NULL },
	{ "sql_query",				0, NULL },
	{ "sql_query_range",		0, NULL },
	{ "sql_range_step",			0, NULL },
	{ "sql_query_killlist",		0, NULL },
	{ "sql_attr_uint",			KEY_LIST, NULL },
	{ "sql_attr_bool",			KEY_LIST, NULL },
	{ "sql_attr_timestamp",		KEY_LIST, NULL },
	{ "sql_attr_str2ordinal",	KEY_LIST, NULL },
	{ "sql_attr_float",			KEY_LIST, NULL },
	{ "sql_attr_bigint",		KEY_LIST, NULL },
	{ "sql_attr_multi",			KEY_LIST, NULL },
	{ "sql_query_post",			KEY_LIST, NULL },
	{ "sql_query_post_index",	KEY_LIST, NULL },
	{ "sql_ranged_throttle",	0, NULL },
	{ "sql_query_info",			0, NULL },
	{ "xmlpipe_command",		0, NULL },
	{ "xmlpipe_field",			KEY_LIST, NULL },
	{ "xmlpipe_attr_uint",		KEY_LIST, NULL },
	{ "xmlpipe_attr_timestamp",	KEY_LIST, NULL },
	{ "xmlpipe_attr_str2ordinal",KEY_LIST, NULL },
	{ "xmlpipe_attr_bool",		KEY_LIST, NULL },
	{ "xmlpipe_attr_float",		KEY_LIST, NULL },
	{ "xmlpipe_attr_multi",		KEY_LIST, NULL },
	{ "xmlpipe_fixup_utf8",		0, NULL },
	{ "sql_group_column",		KEY_LIST | KEY_DEPRECATED, "sql_attr_uint"  },
	{ "sql_date_column",		KEY_LIST | KEY_DEPRECATED, "sql_attr_timestamp" },
	{ "sql_str2ordinal_column",	KEY_LIST | KEY_DEPRECATED, "sql_attr_str2ordinal" },
	{ "unpack_zlib",			KEY_LIST, NULL },
	{ "unpack_mysqlcompress",	KEY_LIST, NULL },
	{ "unpack_mysqlcompress_maxsize", 0, NULL },
	{ "odbc_dsn",				0, NULL },
	{ "sql_joined_field",		KEY_LIST, NULL },
	{ "sql_attr_string",		KEY_LIST, NULL },
	{ "sql_attr_str2wordcount",	KEY_LIST, NULL },
	{ "sql_field_string",		KEY_LIST, NULL },
	{ "sql_field_str2wordcount",KEY_LIST, NULL },
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
	{ "synonyms",				KEY_DEPRECATED, "exceptions" },
	{ "exceptions",				0, NULL },
	{ "wordforms",				0, NULL },
	{ "min_word_len",			0, NULL },
	{ "charset_type",			0, NULL },
	{ "charset_table",			0, NULL },
	{ "ignore_chars",			0, NULL },
	{ "min_prefix_len",			0, NULL },
	{ "min_infix_len",			0, NULL },
	{ "prefix_fields",			0, NULL },
	{ "infix_fields",			0, NULL },
	{ "enable_star",			0, NULL },
	{ "ngram_len",				0, NULL },
	{ "ngram_chars",			0, NULL },
	{ "phrase_boundary",		0, NULL },
	{ "phrase_boundary_step",	0, NULL },
	{ "ondisk_dict",			0, NULL },
	{ "type",					0, NULL },
	{ "local",					KEY_LIST, NULL },
	{ "agent",					KEY_LIST, NULL },
	{ "agent_blackhole",		KEY_LIST, NULL },
	{ "agent_connect_timeout",	0, NULL },
	{ "agent_query_timeout",	0, NULL },
	{ "html_strip",				0, NULL },
	{ "html_index_attrs",		0, NULL },
	{ "html_remove_elements",	0, NULL },
	{ "preopen",				0, NULL },
	{ "inplace_enable",			0, NULL },
	{ "inplace_hit_gap",		0, NULL },
	{ "inplace_docinfo_gap",	0, NULL },
	{ "inplace_reloc_factor",	0, NULL },
	{ "inplace_write_factor",	0, NULL },
	{ "index_exact_words",		0, NULL },
	{ "min_stemming_len",		0, NULL },
	{ "overshort_step",			0, NULL },
	{ "stopword_step",			0, NULL },
	{ "blend_chars",			0, NULL },
	{ "expand_keywords",		0, NULL },
	{ "hitless_words",			KEY_LIST, NULL },
	{ "hit_format",				0, NULL },
	{ NULL,						0, NULL }
};

/// allowed keys for indexer section
static KeyDesc_t g_dKeysIndexer[] =
{
	{ "mem_limit",				0, NULL },
	{ "max_iops",				0, NULL },
	{ "max_iosize",				0, NULL },
	{ "max_xmlpipe2_field",		0, NULL },
	{ "write_buffer",			0, NULL },
	{ NULL,						0, NULL }
};

/// allowed keys for searchd section
static KeyDesc_t g_dKeysSearchd[] =
{
	{ "address",				KEY_DEPRECATED, "listen" },
	{ "port",					KEY_DEPRECATED, "listen" },
	{ "listen",					KEY_LIST, NULL },
	{ "log",					0, NULL },
	{ "query_log",				0, NULL },
	{ "read_timeout",			0, NULL },
	{ "client_timeout",			0, NULL },
	{ "max_children",			0, NULL },
	{ "pid_file",				0, NULL },
	{ "max_matches",			0, NULL },
	{ "seamless_rotate",		0, NULL },
	{ "preopen_indexes",		0, NULL },
	{ "unlink_old",				0, NULL },
	{ "ondisk_dict_default",	0, NULL },
	{ "attr_flush_period",		0, NULL },
	{ "max_packet_size",		0, NULL },
	{ "mva_updates_pool",		0, NULL },
	{ "crash_log_path",			0, NULL },
	{ "max_filters",			0, NULL },
	{ "max_filter_values",		0, NULL },
	{ "listen_backlog",			0, NULL },
	{ "read_buffer",			0, NULL },
	{ "read_unhinted",			0, NULL },
	{ "max_batch_queries",		0, NULL },
	{ "subtree_docs_cache",		0, NULL },
	{ "subtree_hits_cache",		0, NULL },
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

#if !USE_WINDOWS

bool CSphConfigParser::TryToExec ( char * pBuffer, char * pEnd, const char * szFilename, CSphVector<char> & dResult )
{
	int dPipe[2] = { -1, -1 };

	if ( pipe ( dPipe ) )
	{
		snprintf ( m_sError, sizeof ( m_sError ), "pipe() failed (error=%s)", strerror(errno) );
		return false;
	}

	pBuffer = trim ( pBuffer );

	int iRead  = dPipe [0];
	int iWrite = dPipe [1];

	int iChild = fork();

	if ( iChild == 0 )
	{
		close ( iRead );
		close ( STDOUT_FILENO );
		dup2 ( iWrite, STDOUT_FILENO );

		char * pPtr = pBuffer;
		char * pArgs = NULL;
		while ( *pPtr )
		{
			if ( sphIsSpace ( *pPtr ) )
			{
				*pPtr = '\0';
				pArgs = trim ( pPtr+1 );
				break;
			}

			pPtr++;
		}

		if ( pArgs )
			execl ( pBuffer, pBuffer, pArgs, szFilename, NULL );
		else
			execl ( pBuffer, pBuffer, szFilename, NULL );

		exit ( 1 );
	}
	else
		if ( iChild == -1 )
		{
			snprintf ( m_sError, sizeof ( m_sError ), "fork failed: [%d] %s", errno, strerror(errno) );
			return false;
		}

	close ( iWrite );

	int iBytesRead, iTotalRead = 0;
	const int BUFFER_SIZE = 65536;

	dResult.Reset ();

	do
	{
		dResult.Resize ( iTotalRead + BUFFER_SIZE );
		for ( ;; )
		{
			iBytesRead = read ( iRead, (void*)&(dResult [iTotalRead]), BUFFER_SIZE );
			if ( iBytesRead == -1 && errno == EINTR ) // we can get SIGCHLD just before eof
				continue;
			break;
		}
		iTotalRead += iBytesRead;
	}
	while ( iBytesRead > 0 );

	int iStatus, iResult;
	do
	{
		// can be interrupted by pretty much anything (e.g. SIGCHLD from other searchd children)
		iResult = waitpid ( iChild, &iStatus, 0 );
		if ( iResult == -1 && errno != EINTR )
		{
			snprintf ( m_sError, sizeof ( m_sError ), "waitpid() failed: [%d] %s", errno, strerror(errno) );
			return false;
		}
	}
	while ( iResult != iChild );

	if ( WIFEXITED ( iStatus ) && WEXITSTATUS ( iStatus ) )
	{
		// FIXME? read stderr and log that too
		snprintf ( m_sError, sizeof ( m_sError ), "error executing '%s' status = %d", pBuffer, WEXITSTATUS ( iStatus ) );
		return false;
	}

	if ( WIFSIGNALED ( iStatus ) )
	{
		snprintf ( m_sError, sizeof ( m_sError ), "error executing '%s', killed by signal %d", pBuffer, WTERMSIG ( iStatus ) );
		return false;
	}

	if ( iBytesRead < 0  )
	{
		snprintf ( m_sError, sizeof ( m_sError ), "pipe read error: [%d] %s", errno, strerror(errno) );
		return false;
	}

	dResult.Resize ( iTotalRead + 1 );
	dResult [iTotalRead] = '\0';

	return true;
}
#endif


char * CSphConfigParser::GetBufferString ( char * szDest, int iMax, const char * & szSource )
{
	int nCopied = 0;

	while ( nCopied < iMax-1 && szSource [nCopied] && ( nCopied == 0 || szSource [nCopied-1] != '\n' ) )
	{
		szDest [nCopied] = szSource [nCopied];
		nCopied++;
	}

	if ( !nCopied )
		return NULL;

	szSource += nCopied;
	szDest [nCopied] = '\0';

	return szDest;
}


bool CSphConfigParser::Parse ( const char * sFileName, const char * pBuffer )
{
	const int L_STEPBACK	= 16;
	const int L_TOKEN		= 64;
	const int L_BUFFER		= 8192;

	FILE * fp = NULL;
	if ( !pBuffer )
	{
		// open file
		fp = fopen ( sFileName, "rb" );
		if ( !fp )
			return false;
	}

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
			char * szResult = pBuffer ? GetBufferString ( sBuf, L_BUFFER, pBuffer ) : fgets ( sBuf, L_BUFFER, fp );
			if ( !szResult )
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

			if ( *p=='#' )
			{
#if !USE_WINDOWS
				if ( !pBuffer && m_iLine==1 && p==sBuf && p[1]=='!' )
				{
					CSphVector<char> dResult;
					if ( TryToExec ( p+2, pEnd, sFileName, dResult ) )
						Parse ( sFileName, &dResult [0] );
					break;
				} else
#endif
				{
					LOC_PUSH ( S_SKIP2NL );
					continue;
				}
			}

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
			LOC_BACK(); // because we did not work the char at all
			continue;
		}

		// handle S_VALUE state
		if ( eState==S_VALUE )
		{
			if ( *p=='\n' )					{ AddKey ( sToken, sValue ); iValue = 0; LOC_POP (); continue; }
			if ( *p=='#' )					{ AddKey ( sToken, sValue ); iValue = 0; LOC_POP (); LOC_PUSH ( S_SKIP2NL ); continue; }
			if ( *p=='\\' )
			{
				// backslash at the line end: continuation operator; let the newline be unhanlded
				if ( p[1]=='\r' || p[1]=='\n' ) { LOC_PUSH ( S_SKIP2NL ); continue; }

				// backslash before number sign: comment start char escaping; advance and pass it
				if ( p[1]=='#' ) { p++; }

				// otherwise: just a char, pass it
			}
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

	if ( !pBuffer )
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

bool sphConfTokenizer ( const CSphConfigSection & hIndex, CSphTokenizerSettings & tSettings, CSphString & sError )
{
	// charset_type
	CSphScopedPtr<ISphTokenizer> pTokenizer ( NULL );

	if ( !hIndex("charset_type") || hIndex["charset_type"]=="sbcs" )
	{
		tSettings.m_iType = TOKENIZER_SBCS;
	}
	else if ( hIndex["charset_type"]=="utf-8" )
	{
		tSettings.m_iType = hIndex("ngram_chars") ? TOKENIZER_NGRAM : TOKENIZER_UTF8;
	}
	else
	{
		sError.SetSprintf ( "unknown charset type '%s'", hIndex["charset_type"].cstr() );
		return false;
	}

	tSettings.m_sCaseFolding	= hIndex.GetStr ( "charset_table" );
	tSettings.m_iMinWordLen		= Max ( hIndex.GetInt ( "min_word_len" ), 0 );
	tSettings.m_sNgramChars		= hIndex.GetStr ( "ngram_chars" );
	tSettings.m_iNgramLen		= Max ( hIndex.GetInt ( "ngram_len" ), 0 );
	tSettings.m_sSynonymsFile	= hIndex.GetStr ( "exceptions" ); // new option name
	if ( tSettings.m_sSynonymsFile.IsEmpty() )
		tSettings.m_sSynonymsFile = hIndex.GetStr ( "synonyms" ); // deprecated option name
	tSettings.m_sIgnoreChars	= hIndex.GetStr ( "ignore_chars" );
	tSettings.m_sBlendChars		= hIndex.GetStr ( "blend_chars" );

	// phrase boundaries
	int iBoundaryStep = Max ( hIndex.GetInt ( "phrase_boundary_step" ), -1 );
	if ( iBoundaryStep!=0 )
		tSettings.m_sBoundary = hIndex.GetStr ( "phrase_boundary" );

	return true;
}

void sphConfDictionary ( const CSphConfigSection & hIndex, CSphDictSettings & tSettings )
{
	tSettings.m_sMorphology = hIndex.GetStr ( "morphology" );
	tSettings.m_sStopwords	= hIndex.GetStr ( "stopwords" );
	tSettings.m_sWordforms	= hIndex.GetStr ( "wordforms" );
	tSettings.m_iMinStemmingLen	= hIndex.GetInt ( "min_stemming_len", 1 );
}


void sphConfIndex ( const CSphConfigSection & hIndex, CSphIndexSettings & tSettings )
{
	tSettings.m_iMinPrefixLen = Max ( hIndex.GetInt ( "min_prefix_len" ), 0 );
	tSettings.m_iMinInfixLen  = Max ( hIndex.GetInt ( "min_infix_len" ), 0 );
	tSettings.m_iBoundaryStep = Max ( hIndex.GetInt ( "phrase_boundary_step" ), -1 );
	tSettings.m_bIndexExactWords = hIndex.GetInt ( "index_exact_words" )!=0;
	tSettings.m_iOvershortStep = Min ( Max ( hIndex.GetInt ( "overshort_step", 1 ), 0 ), 1 );
	tSettings.m_iStopwordStep = Min ( Max ( hIndex.GetInt ( "stopword_step", 1 ), 0 ), 1 );

	if ( hIndex ( "html_strip" ) )
	{
		tSettings.m_bHtmlStrip			= hIndex.GetInt ( "html_strip" )!=0;
		tSettings.m_sHtmlIndexAttrs		= hIndex.GetStr ( "html_index_attrs" );
		tSettings.m_sHtmlRemoveElements	= hIndex.GetStr ( "html_remove_elements" );
	}

	tSettings.m_eDocinfo = SPH_DOCINFO_EXTERN;
	if ( hIndex ("docinfo") )
	{
		if ( hIndex["docinfo"]=="none" )		tSettings.m_eDocinfo = SPH_DOCINFO_NONE;
		else if ( hIndex["docinfo"]=="inline" )	tSettings.m_eDocinfo = SPH_DOCINFO_INLINE;
		else if ( hIndex["docinfo"]=="extern" )	tSettings.m_eDocinfo = SPH_DOCINFO_EXTERN;
		else
			fprintf ( stdout, "WARNING: unknown docinfo=%s, defaulting to extern\n", hIndex["docinfo"].cstr() );
	}

	tSettings.m_eHitFormat = SPH_HIT_FORMAT_INLINE;
	if ( hIndex("hit_format") )
	{
		if ( hIndex["hit_format"]=="plain" )		tSettings.m_eHitFormat = SPH_HIT_FORMAT_PLAIN;
		else if ( hIndex["hit_format"]=="inline" )	tSettings.m_eHitFormat = SPH_HIT_FORMAT_INLINE;
		else
			fprintf ( stdout, "WARNING: unknown hit_format=%s, defaulting to inline\n", hIndex["hit_format"].cstr() );
	}

	// hit-less indices
	if ( hIndex("hitless_words") )
	{
		for ( const CSphVariant * pVariant = &hIndex["hitless_words"]; pVariant; pVariant = pVariant->m_pNext )
		{
			const CSphString & sValue = *pVariant;
			if ( sValue.Ends("%") )
			{
				tSettings.m_eHitless = SPH_HITLESS_SOME;
				tSettings.m_fHitlessThreshold = float(pVariant->intval()) / 100.0f;
			}
			else if ( sValue=="all" )
			{
				tSettings.m_eHitless = SPH_HITLESS_ALL;
			}
			else
			{
				tSettings.m_eHitless = SPH_HITLESS_SOME;
				tSettings.m_sHitlessFile = sValue;
			}
		}
	}
}


bool sphFixupIndexSettings ( CSphIndex * pIndex, const CSphConfigSection & hIndex, CSphString & sError )
{
	bool bTokenizerSpawned = false;

	if ( !pIndex->GetTokenizer () )
	{
		CSphTokenizerSettings tSettings;
		if ( !sphConfTokenizer ( hIndex, tSettings, sError ) )
			return false;

		ISphTokenizer * pTokenizer = ISphTokenizer::Create ( tSettings, sError );
		if ( !pTokenizer )
			return false;

		bTokenizerSpawned = true;
		pIndex->SetTokenizer ( pTokenizer );
	}

	if ( !pIndex->GetDictionary () )
	{
		CSphDictSettings tSettings;
		sphConfDictionary ( hIndex, tSettings );
		CSphDict * pDict = sphCreateDictionaryCRC ( tSettings, pIndex->GetTokenizer (), sError );
		if ( !pDict )
			return false;

		pIndex->SetDictionary ( pDict );
	}

	if ( bTokenizerSpawned )
	{
		ISphTokenizer * pTokenizer = pIndex->LeakTokenizer ();
		ISphTokenizer * pTokenFilter = ISphTokenizer::CreateTokenFilter ( pTokenizer, pIndex->GetDictionary ()->GetMultiWordforms () );
		pIndex->SetTokenizer ( pTokenFilter ? pTokenFilter : pTokenizer );
 	}

	if ( !pIndex->IsStripperInited () )
	{
		CSphIndexSettings tSettings = pIndex->GetSettings ();

		if ( hIndex ( "html_strip" ) )
		{
			tSettings.m_bHtmlStrip			= hIndex.GetInt ( "html_strip" )!=0;
			tSettings.m_sHtmlIndexAttrs		= hIndex.GetStr ( "html_index_attrs" );
			tSettings.m_sHtmlRemoveElements	= hIndex.GetStr ( "html_remove_elements" );
		}

		pIndex->Setup ( tSettings );
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

const char * sphLoadConfig ( const char * sOptConfig, bool bQuiet, CSphConfigParser & cp )
{
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

	if ( !bQuiet )
		fprintf ( stdout, "using config file '%s'...\n", sOptConfig );

	// load config
	if ( !cp.Parse ( sOptConfig ) )
		sphDie ( "failed to parse config file '%s'", sOptConfig );

	CSphConfig & hConf = cp.m_tConf;
	if ( !hConf ( "index" ) )
		sphDie ( "no indexes found in config file '%s'", sOptConfig );

	return sOptConfig;
}

//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS

void sphSetupSignals ()
{
}

#else

static void DummyHandler ( int )
{
}

void sphSetupSignals ()
{
	struct sigaction tAction;
	
	sigfillset ( &tAction.sa_mask );
	tAction.sa_flags = SA_NOCLDSTOP;
	tAction.sa_handler = DummyHandler;
	if ( sigaction ( SIGCHLD, &tAction, NULL ) == -1 )
		sphDie ( "sigaction() failed: [%d] %s", errno, strerror(errno) );
}

#endif

//
// $Id$
//
