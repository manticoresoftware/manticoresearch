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

/// @file sphinxutils.cpp
/// Implementations for Sphinx utilities shared classes.

#include "sphinx.h"
#include "sphinxutils.h"
#include "sphinxint.h"
#include "sphinxplugin.h"

#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#if HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#if USE_WINDOWS
#include <io.h> // for ::open on windows
#include <dbghelp.h>
#pragma comment(linker, "/defaultlib:dbghelp.lib")
#pragma message("Automatically linking with dbghelp.lib")
#else
#include <sys/wait.h>
#include <signal.h>
#include <glob.h>

#ifdef HAVE_DLOPEN
#include <dlfcn.h>
#endif // HAVE_DLOPEN

#ifndef HAVE_DLERROR
#define dlerror() ""
#endif // HAVE_DLERROR

#endif

//////////////////////////////////////////////////////////////////////////
// STRING FUNCTIONS
//////////////////////////////////////////////////////////////////////////

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


void sphSplit ( CSphVector<CSphString> & dOut, const char * sIn )
{
	if ( !sIn )
		return;

	const char * p = (char*)sIn;
	while ( *p )
	{
		// skip non-alphas
		while ( (*p) && !sphIsAlpha(*p) )
			p++;
		if ( !(*p) )
			break;

		// this is my next token
		assert ( sphIsAlpha(*p) );
		const char * sNext = p;
		while ( sphIsAlpha(*p) )
			p++;
		if ( sNext!=p )
			dOut.Add().SetBinary ( sNext, p-sNext );
	}
}


void sphSplit ( CSphVector<CSphString> & dOut, const char * sIn, const char * sBounds )
{
	if ( !sIn )
		return;

	const char * p = (char*)sIn;
	while ( *p )
	{
		// skip until the first non-boundary character
		const char * sNext = p;
		while ( *p && !strchr ( sBounds, *p ) )
			p++;

		// add the token, skip the char
		dOut.Add().SetBinary ( sNext, p-sNext );
		p++;
	}
}


bool sphWildcardMatch ( const char * sString, const char * sPattern )
{
	if ( !sString || !sPattern )
		return false;

	const char * s = sString;
	const char * p = sPattern;
	while ( *s )
	{
		switch ( *p )
		{
		case '\\':
			// escaped char, strict match the next one literally
			p++;
			if ( *s++!=*p++ )
				return false;
			break;

		case '?':
			// match any character
			s++;
			p++;
			break;

		case '%':
			// gotta match either 0 or 1 characters
			// well, lets look ahead and see what we need to match next
			p++;

			// just a shortcut, %* can be folded to just *
			if ( *p=='*' )
				break;

			// plain char after a hash? check the non-ambiguous cases
			if ( !sphIsWild(*p) )
			{
				if ( s[0]!=*p )
				{
					// hash does not match 0 chars
					// check if we can match 1 char, or it's a no-match
					if ( s[1]!=*p )
						return false;
					s++;
					break;
				} else
				{
					// hash matches 0 chars
					// check if we could ambiguously match 1 char too, though
					if ( s[1]!=*p )
						break;
					// well, fall through to "scan both options" route
				}
			}

			// could not decide yet
			// so just recurse both options
			if ( sphWildcardMatch ( s, p ) )
				return true;
			if ( sphWildcardMatch ( s+1, p ) )
				return true;
			return false;

		case '*':
			// skip all the extra stars and question marks
			for ( p++; *p=='*' || *p=='?'; p++ )
				if ( *p=='?' )
				{
					s++;
					if ( !*s )
						return p[1]=='\0';
				}

				// short-circuit trailing star
				if ( !*p )
					return true;

				// so our wildcard expects a real character
				// scan forward for its occurrences and recurse
				for ( ;; )
				{
					if ( !*s )
						return false;
					if ( *s==*p && sphWildcardMatch ( s+1, p+1 ) )
						return true;
					s++;
				}
				break;

		default:
			// default case, strict match
			if ( *s++!=*p++ )
				return false;
			break;
		}
	}

	// string done
	// pattern should be either done too, or a trailing star, or a trailing hash
	return p[0]=='\0'
		|| ( p[0]=='*' && p[1]=='\0' )
		|| ( p[0]=='%' && p[1]=='\0' );
}

//////////////////////////////////////////////////////////////////////////

int64_t CSphConfigSection::GetSize64 ( const char * sKey, int64_t iDefault ) const
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
	if ( toupper ( sMemLimit[iLen] )=='K' )
	{
		iScale = 1024;
		sMemLimit[iLen] = '\0';

	} else if ( toupper ( sMemLimit[iLen] )=='M' )
	{
		iScale = 1048576;
		sMemLimit[iLen] = '\0';
	}

	char * sErr;
	int64_t iRes = strtoll ( sMemLimit, &sErr, 10 );

	if ( !*sErr )
	{
		iRes *= iScale;
	} else
	{
		sphWarning ( "'%s = %s' parse error '%s'", sKey, pEntry->cstr(), sErr );
		iRes = iDefault;
	}

	return iRes;
}

int CSphConfigSection::GetSize ( const char * sKey, int iDefault ) const
{
	int64_t iSize = GetSize64 ( sKey, iDefault );
	if ( iSize>INT_MAX )
	{
		iSize = INT_MAX;
		sphWarning ( "'%s = "INT64_FMT"' clamped to %d(INT_MAX)", sKey, iSize, INT_MAX );
	}
	return (int)iSize;
}

//////////////////////////////////////////////////////////////////////////
// CONFIG PARSER
//////////////////////////////////////////////////////////////////////////

/// key flags
enum
{
	KEY_DEPRECATED		= 1UL<<0,
	KEY_LIST			= 1UL<<1,
	KEY_HIDDEN			= 1UL<<2,
	KEY_REMOVED			= 1UL<<3
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
	{ "sql_host",				0, NULL },
	{ "sql_user",				0, NULL },
	{ "sql_pass",				0, NULL },
	{ "sql_db",					0, NULL },
	{ "sql_port",				0, NULL },
	{ "sql_sock",				0, NULL },
	{ "mysql_connect_flags",	0, NULL },
	{ "mysql_ssl_key",			0, NULL }, // check.pl mysql_ssl
	{ "mysql_ssl_cert",			0, NULL }, // check.pl mysql_ssl
	{ "mysql_ssl_ca",			0, NULL }, // check.pl mysql_ssl
	{ "mssql_winauth",			0, NULL },
	{ "mssql_unicode",			KEY_REMOVED, NULL },
	{ "sql_query_pre",			KEY_LIST, NULL },
	{ "sql_query",				0, NULL },
	{ "sql_query_range",		0, NULL },
	{ "sql_range_step",			0, NULL },
	{ "sql_query_killlist",		0, NULL },
	{ "sql_attr_uint",			KEY_LIST, NULL },
	{ "sql_attr_bool",			KEY_LIST, NULL },
	{ "sql_attr_timestamp",		KEY_LIST, NULL },
	{ "sql_attr_str2ordinal",	KEY_REMOVED | KEY_LIST, NULL },
	{ "sql_attr_float",			KEY_LIST, NULL },
	{ "sql_attr_bigint",		KEY_LIST, NULL },
	{ "sql_attr_multi",			KEY_LIST, NULL },
	{ "sql_query_post",			KEY_LIST, NULL },
	{ "sql_query_post_index",	KEY_LIST, NULL },
	{ "sql_ranged_throttle",	0, NULL },
	{ "sql_query_info",			KEY_REMOVED, NULL },
	{ "xmlpipe_command",		0, NULL },
	{ "xmlpipe_field",			KEY_LIST, NULL },
	{ "xmlpipe_attr_uint",		KEY_LIST, NULL },
	{ "xmlpipe_attr_timestamp",	KEY_LIST, NULL },
	{ "xmlpipe_attr_str2ordinal", KEY_REMOVED | KEY_LIST, NULL },
	{ "xmlpipe_attr_bool",		KEY_LIST, NULL },
	{ "xmlpipe_attr_float",		KEY_LIST, NULL },
	{ "xmlpipe_attr_bigint",	KEY_LIST, NULL },
	{ "xmlpipe_attr_multi",		KEY_LIST, NULL },
	{ "xmlpipe_attr_multi_64",	KEY_LIST, NULL },
	{ "xmlpipe_attr_string",	KEY_LIST, NULL },
	{ "xmlpipe_attr_wordcount", KEY_REMOVED | KEY_LIST, NULL },
	{ "xmlpipe_attr_json",		KEY_LIST, NULL },
	{ "xmlpipe_field_string",	KEY_LIST, NULL },
	{ "xmlpipe_field_wordcount", KEY_REMOVED | KEY_LIST, NULL },
	{ "xmlpipe_fixup_utf8",		0, NULL },
	{ "sql_str2ordinal_column", KEY_LIST | KEY_REMOVED, NULL },
	{ "unpack_zlib",			KEY_LIST, NULL },
	{ "unpack_mysqlcompress",	KEY_LIST, NULL },
	{ "unpack_mysqlcompress_maxsize", 0, NULL },
	{ "odbc_dsn",				0, NULL },
	{ "sql_joined_field",		KEY_LIST, NULL },
	{ "sql_attr_string",		KEY_LIST, NULL },
	{ "sql_attr_str2wordcount", KEY_REMOVED | KEY_LIST, NULL },
	{ "sql_field_string",		KEY_LIST, NULL },
	{ "sql_field_str2wordcount", KEY_REMOVED | KEY_LIST, NULL },
	{ "sql_file_field",			KEY_LIST, NULL },
	{ "sql_column_buffers",		0, NULL },
	{ "sql_attr_json",			KEY_LIST, NULL },
	{ "hook_connect",			KEY_HIDDEN, NULL },
	{ "hook_query_range",		KEY_HIDDEN, NULL },
	{ "hook_post_index",		KEY_HIDDEN, NULL },
	{ "tsvpipe_command",		0, NULL },
	{ "tsvpipe_field",			KEY_LIST, NULL },
	{ "tsvpipe_attr_uint",		KEY_LIST, NULL },
	{ "tsvpipe_attr_timestamp",	KEY_LIST, NULL },
	{ "tsvpipe_attr_bool",		KEY_LIST, NULL },
	{ "tsvpipe_attr_float",		KEY_LIST, NULL },
	{ "tsvpipe_attr_bigint",	KEY_LIST, NULL },
	{ "tsvpipe_attr_multi",		KEY_LIST, NULL },
	{ "tsvpipe_attr_multi_64",	KEY_LIST, NULL },
	{ "tsvpipe_attr_string",	KEY_LIST, NULL },
	{ "tsvpipe_attr_json",		KEY_LIST, NULL },
	{ "tsvpipe_field_string",	KEY_LIST, NULL },
	{ "csvpipe_command",		0, NULL },
	{ "csvpipe_field",			KEY_LIST, NULL },
	{ "csvpipe_attr_uint",		KEY_LIST, NULL },
	{ "csvpipe_attr_timestamp",	KEY_LIST, NULL },
	{ "csvpipe_attr_bool",		KEY_LIST, NULL },
	{ "csvpipe_attr_float",		KEY_LIST, NULL },
	{ "csvpipe_attr_bigint",	KEY_LIST, NULL },
	{ "csvpipe_attr_multi",		KEY_LIST, NULL },
	{ "csvpipe_attr_multi_64",	KEY_LIST, NULL },
	{ "csvpipe_attr_string",	KEY_LIST, NULL },
	{ "csvpipe_attr_json",		KEY_LIST, NULL },
	{ "csvpipe_field_string",	KEY_LIST, NULL },
	{ "csvpipe_delimiter",		0, NULL },
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
	{ "exceptions",				0, NULL },
	{ "wordforms",				KEY_LIST, NULL },
	{ "embedded_limit",			0, NULL },
	{ "min_word_len",			0, NULL },
	{ "charset_type",			KEY_REMOVED, NULL },
	{ "charset_table",			0, NULL },
	{ "ignore_chars",			0, NULL },
	{ "min_prefix_len",			0, NULL },
	{ "min_infix_len",			0, NULL },
	{ "max_substring_len",		0, NULL },
	{ "prefix_fields",			0, NULL },
	{ "infix_fields",			0, NULL },
	{ "enable_star",			KEY_REMOVED, NULL },
	{ "ngram_len",				0, NULL },
	{ "ngram_chars",			0, NULL },
	{ "phrase_boundary",		0, NULL },
	{ "phrase_boundary_step",	0, NULL },
	{ "ondisk_dict",			KEY_REMOVED, NULL },
	{ "type",					0, NULL },
	{ "local",					KEY_LIST, NULL },
	{ "agent",					KEY_LIST, NULL },
	{ "agent_blackhole",		KEY_LIST, NULL },
	{ "agent_persistent",		KEY_LIST, NULL },
	{ "agent_connect_timeout",	0, NULL },
	{ "ha_strategy",			0, NULL	},
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
	{ "hitless_words",			0, NULL },
	{ "hit_format",				KEY_HIDDEN | KEY_DEPRECATED, "default value" },
	{ "rt_field",				KEY_LIST, NULL },
	{ "rt_attr_uint",			KEY_LIST, NULL },
	{ "rt_attr_bigint",			KEY_LIST, NULL },
	{ "rt_attr_float",			KEY_LIST, NULL },
	{ "rt_attr_timestamp",		KEY_LIST, NULL },
	{ "rt_attr_string",			KEY_LIST, NULL },
	{ "rt_attr_multi",			KEY_LIST, NULL },
	{ "rt_attr_multi_64",		KEY_LIST, NULL },
	{ "rt_attr_json",			KEY_LIST, NULL },
	{ "rt_attr_bool",			KEY_LIST, NULL },
	{ "rt_mem_limit",			0, NULL },
	{ "dict",					0, NULL },
	{ "index_sp",				0, NULL },
	{ "index_zones",			0, NULL },
	{ "blend_mode",				0, NULL },
	{ "regexp_filter",			KEY_LIST, NULL },
	{ "bigram_freq_words",		0, NULL },
	{ "bigram_index",			0, NULL },
	{ "index_field_lengths",	0, NULL },
	{ "divide_remote_ranges",	KEY_HIDDEN, NULL },
	{ "stopwords_unstemmed",	0, NULL },
	{ "global_idf",				0, NULL },
	{ "rlp_context",			0, NULL },
	{ "ondisk_attrs",			0, NULL },
	{ "index_token_filter",		0, NULL },
	{ NULL,						0, NULL }
};

/// allowed keys for indexer section
static KeyDesc_t g_dKeysIndexer[] =
{
	{ "mem_limit",				0, NULL },
	{ "max_iops",				0, NULL },
	{ "max_iosize",				0, NULL },
	{ "max_xmlpipe2_field",		0, NULL },
	{ "max_file_field_buffer",	0, NULL },
	{ "write_buffer",			0, NULL },
	{ "on_file_field_error",	0, NULL },
	{ "on_json_attr_error",		KEY_DEPRECATED, "on_json_attr_error in common{..} section" },
	{ "json_autoconv_numbers",	KEY_DEPRECATED, "json_autoconv_numbers in common{..} section" },
	{ "json_autoconv_keynames",	KEY_DEPRECATED, "json_autoconv_keynames in common{..} section" },
	{ "lemmatizer_cache",		0, NULL },
	{ NULL,						0, NULL }
};

/// allowed keys for searchd section
static KeyDesc_t g_dKeysSearchd[] =
{
	{ "address",				KEY_REMOVED, NULL },
	{ "port",					KEY_REMOVED, NULL },
	{ "listen",					KEY_LIST, NULL },
	{ "log",					0, NULL },
	{ "query_log",				0, NULL },
	{ "read_timeout",			0, NULL },
	{ "client_timeout",			0, NULL },
	{ "max_children",			0, NULL },
	{ "pid_file",				0, NULL },
	{ "max_matches",			KEY_REMOVED, NULL },
	{ "seamless_rotate",		0, NULL },
	{ "preopen_indexes",		0, NULL },
	{ "unlink_old",				0, NULL },
	{ "ondisk_dict_default",	KEY_REMOVED, NULL },
	{ "attr_flush_period",		0, NULL },
	{ "max_packet_size",		0, NULL },
	{ "mva_updates_pool",		0, NULL },
	{ "max_filters",			0, NULL },
	{ "max_filter_values",		0, NULL },
	{ "listen_backlog",			0, NULL },
	{ "read_buffer",			0, NULL },
	{ "read_unhinted",			0, NULL },
	{ "max_batch_queries",		0, NULL },
	{ "subtree_docs_cache",		0, NULL },
	{ "subtree_hits_cache",		0, NULL },
	{ "workers",				0, NULL },
	{ "prefork",				KEY_HIDDEN, NULL },
	{ "dist_threads",			0, NULL },
	{ "binlog_flush",			0, NULL },
	{ "binlog_path",			0, NULL },
	{ "binlog_max_log_size",	0, NULL },
	{ "thread_stack",			0, NULL },
	{ "expansion_limit",		0, NULL },
	{ "rt_flush_period",		0, NULL },
	{ "query_log_format",		0, NULL },
	{ "mysql_version_string",	0, NULL },
	{ "plugin_dir",				KEY_DEPRECATED, "plugin_dir in common{..} section" },
	{ "collation_server",		0, NULL },
	{ "collation_libc_locale",	0, NULL },
	{ "watchdog",				0, NULL },
	{ "prefork_rotation_throttle", KEY_REMOVED, NULL },
	{ "snippets_file_prefix",	0, NULL },
	{ "sphinxql_state",			0, NULL },
	{ "rt_merge_iops",			0, NULL },
	{ "rt_merge_maxiosize",		0, NULL },
	{ "ha_ping_interval",		0, NULL },
	{ "ha_period_karma",		0, NULL },
	{ "predicted_time_costs",	0, NULL },
	{ "persistent_connections_limit",	0, NULL },
	{ "ondisk_attrs_default",	0, NULL },
	{ "shutdown_timeout",		0, NULL },
	{ "query_log_min_msec",		0, NULL },
	{ "agent_connect_timeout",	0, NULL },
	{ "agent_query_timeout",	0, NULL },
	{ "agent_retry_delay",		0, NULL },
	{ "agent_retry_count",		0, NULL },
	{ "net_wait_tm",			0, NULL },
	{ "net_throttle_action",	0, NULL },
	{ "net_throttle_accept",	0, NULL },
	{ "net_send_job",			0, NULL },
	{ "net_workers",			0, NULL },
	{ "queue_max_length",		0, NULL },
	{ "qcache_ttl_sec",			0, NULL },
	{ "qcache_max_bytes",		0, NULL },
	{ "qcache_thresh_msec",		0, NULL },
	{ "sphinxql_timeout",		0, NULL },
	{ NULL,						0, NULL }
};

/// allowed keys for common section
static KeyDesc_t g_dKeysCommon[] =
{
	{ "lemmatizer_base",		0, NULL },
	{ "on_json_attr_error",		0, NULL },
	{ "json_autoconv_numbers",	0, NULL },
	{ "json_autoconv_keynames",	0, NULL },
	{ "rlp_root",				0, NULL },
	{ "rlp_environment",		0, NULL },
	{ "rlp_max_batch_size",		0, NULL },
	{ "rlp_max_batch_docs",		0, NULL },
	{ "plugin_dir",				0, NULL },
	{ NULL,						0, NULL }
};

struct KeySection_t
{
	const char *		m_sKey;		///< key name
	KeyDesc_t *			m_pSection; ///< section to refer
	bool				m_bNamed; ///< true if section is named. false if plain
};

static KeySection_t g_dConfigSections[] =
{
	{ "source",		g_dKeysSource,	true },
	{ "index",		g_dKeysIndex,	true },
	{ "indexer",	g_dKeysIndexer,	false },
	{ "searchd",	g_dKeysSearchd,	false },
	{ "common",		g_dKeysCommon,	false },
	{ NULL,			NULL,			false }
};

//////////////////////////////////////////////////////////////////////////

CSphConfigParser::CSphConfigParser ()
	: m_sFileName ( "" )
	, m_iLine ( -1 )
{
}

bool CSphConfigParser::IsPlainSection ( const char * sKey )
{
	assert ( sKey );
	const KeySection_t * pSection = g_dConfigSections;
	while ( pSection->m_sKey && strcasecmp ( sKey, pSection->m_sKey ) )
		++pSection;
	return pSection->m_sKey && !pSection->m_bNamed;
}

bool CSphConfigParser::IsNamedSection ( const char * sKey )
{
	assert ( sKey );
	const KeySection_t * pSection = g_dConfigSections;
	while ( pSection->m_sKey && strcasecmp ( sKey, pSection->m_sKey ) )
		++pSection;
	return pSection->m_sKey && pSection->m_bNamed;
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
	int iTag = tSec.m_iTag;
	tSec.m_iTag++;
	if ( tSec(sKey) )
	{
		if ( tSec[sKey].m_bTag )
		{
			// override value or list with a new value
			SafeDelete ( tSec[sKey].m_pNext ); // only leave the first array element
			tSec[sKey] = CSphVariant ( sValue, iTag ); // update its value
			tSec[sKey].m_bTag = false; // mark it as overridden

		} else
		{
			// chain to tail, to keep the order
			CSphVariant * pTail = &tSec[sKey];
			while ( pTail->m_pNext )
				pTail = pTail->m_pNext;
			pTail->m_pNext = new CSphVariant ( sValue, iTag );
		}

	} else
	{
		// just add
		tSec.Add ( CSphVariant ( sValue, iTag ), sKey ); // FIXME! be paranoid, verify that it returned true
	}
}


bool CSphConfigParser::ValidateKey ( const char * sKey )
{
	// get proper descriptor table
	// OPTIMIZE! move lookup to AddSection
	const KeySection_t * pSection = g_dConfigSections;
	const KeyDesc_t * pDesc = NULL;
	while ( pSection->m_sKey && m_sSectionType!=pSection->m_sKey )
		++pSection;
	if ( pSection->m_sKey )
		pDesc = pSection->m_pSection;

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
		snprintf ( m_sError, sizeof(m_sError), "unknown key name '%s'", sKey );
		return false;
	}

	// warn about deprecate keys
	if ( pDesc->m_iFlags & KEY_DEPRECATED )
		if ( ++m_iWarnings<=WARNS_THRESH )
			fprintf ( stdout, "WARNING: key '%s' is deprecated in %s line %d; use '%s' instead.\n",
				sKey, m_sFileName.cstr(), m_iLine, pDesc->m_sExtra );

	// warn about list/non-list keys
	if (!( pDesc->m_iFlags & KEY_LIST ))
	{
		CSphConfigSection & tSec = m_tConf[m_sSectionType][m_sSectionName];
		if ( tSec(sKey) && !tSec[sKey].m_bTag )
			if ( ++m_iWarnings<=WARNS_THRESH )
				fprintf ( stdout, "WARNING: key '%s' is not multi-value; value in %s line %d will be ignored.\n", sKey, m_sFileName.cstr(), m_iLine );
	}

	if ( pDesc->m_iFlags & KEY_REMOVED )
		if ( ++m_iWarnings<=WARNS_THRESH )
			fprintf ( stdout, "WARNING: key '%s' was permanently removed from Sphinx configuration. Refer to documentation for details.\n", sKey );

	return true;
}

#if !USE_WINDOWS

bool TryToExec ( char * pBuffer, const char * szFilename, CSphVector<char> & dResult, char * sError, int iErrorLen )
{
	int dPipe[2] = { -1, -1 };

	if ( pipe ( dPipe ) )
	{
		snprintf ( sError, iErrorLen, "pipe() failed (error=%s)", strerror(errno) );
		return false;
	}

	pBuffer = trim ( pBuffer );

	int iRead = dPipe[0];
	int iWrite = dPipe[1];

	int iChild = fork();

	if ( iChild==0 )
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
			execl ( pBuffer, pBuffer, pArgs, szFilename, (char*)NULL );
		else
			execl ( pBuffer, pBuffer, szFilename, (char*)NULL );

		exit ( 1 );

	} else if ( iChild==-1 )
	{
		snprintf ( sError, iErrorLen, "fork failed: [%d] %s", errno, strerror(errno) );
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
			if ( iBytesRead==-1 && errno==EINTR ) // we can get SIGCHLD just before eof
				continue;
			break;
		}
		iTotalRead += iBytesRead;
	}
	while ( iBytesRead > 0 );
	close ( iRead );

	int iStatus, iResult;
	do
	{
		// can be interrupted by pretty much anything (e.g. SIGCHLD from other searchd children)
		iResult = waitpid ( iChild, &iStatus, 0 );

		// they say this can happen if child exited and SIGCHLD was ignored
		// a cleaner one would be to temporary handle it here, but can we be bothered
		if ( iResult==-1 && errno==ECHILD )
		{
			iResult = iChild;
			iStatus = 0;
		}

		if ( iResult==-1 && errno!=EINTR )
		{
			snprintf ( sError, iErrorLen, "waitpid() failed: [%d] %s", errno, strerror(errno) );
			return false;
		}
	}
	while ( iResult!=iChild );

	if ( WIFEXITED ( iStatus ) && WEXITSTATUS ( iStatus ) )
	{
		// FIXME? read stderr and log that too
		snprintf ( sError, iErrorLen, "error executing '%s' status = %d", pBuffer, WEXITSTATUS ( iStatus ) );
		return false;
	}

	if ( WIFSIGNALED ( iStatus ) )
	{
		snprintf ( sError, iErrorLen, "error executing '%s', killed by signal %d", pBuffer, WTERMSIG ( iStatus ) );
		return false;
	}

	if ( iBytesRead < 0 )
	{
		snprintf ( sError, iErrorLen, "pipe read error: [%d] %s", errno, strerror(errno) );
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

	while ( nCopied < iMax-1 && szSource[nCopied] && ( nCopied==0 || szSource[nCopied-1]!='\n' ) )
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

bool CSphConfigParser::ReParse ( const char * sFileName, const char * pBuffer )
{
	CSphConfig tOldConfig = m_tConf;
	m_tConf.Reset();
	if ( Parse ( sFileName, pBuffer ) )
		return true;
	m_tConf = tOldConfig;
	return false;
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

	char sBuf [ L_BUFFER ] = { 0 };

	char sToken [ L_TOKEN ] = { 0 };
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
					if ( TryToExec ( p+2, sFileName, dResult, m_sError, sizeof(m_sError) ) )
						Parse ( sFileName, &dResult[0] );
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
			if ( IsPlainSection(sToken) )
			{
				if ( !AddSection ( sToken, sToken ) )
					break;
				sToken[0] = '\0';
				LOC_POP();
				LOC_PUSH ( S_SEC );
				LOC_PUSH ( S_CHR );
				iCh = '{';
				LOC_BACK();
				continue;
			}
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
			if ( isspace(*p) )					{ continue; }
			if ( !sToken[0]&&!sphIsAlpha(*p))	{ LOC_ERROR2 ( "named section: expected name, got '%c'", *p ); }

			if ( !sToken[0] )				{ LOC_PUSH ( S_TOK ); LOC_BACK(); continue; }
											if ( !AddSection ( m_sSectionType.cstr(), sToken ) ) break; sToken[0] = '\0';
			if ( *p==':' )					{ eState = S_SECBASE; continue; }
			if ( *p=='{' )					{ eState = S_SEC; continue; }
											LOC_ERROR2 ( "named section: expected ':' or '{', got '%c'", *p );
		}

		// handle S_SECBASE state
		if ( eState==S_SECBASE )
		{
			if ( isspace(*p) )					{ continue; }
			if ( !sToken[0]&&!sphIsAlpha(*p))	{ LOC_ERROR2 ( "named section: expected parent name, got '%c'", *p ); }
			if ( !sToken[0] )					{ LOC_PUSH ( S_TOK ); LOC_BACK(); continue; }

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

void sphConfTokenizer ( const CSphConfigSection & hIndex, CSphTokenizerSettings & tSettings )
{
	tSettings.m_iNgramLen = Max ( hIndex.GetInt ( "ngram_len" ), 0 );

	if ( hIndex ( "ngram_chars" ) )
	{
		if ( tSettings.m_iNgramLen )
			tSettings.m_iType = TOKENIZER_NGRAM;
		else
			sphWarning ( "ngram_chars specified, but ngram_len=0; IGNORED" );
	}

	tSettings.m_sCaseFolding = hIndex.GetStr ( "charset_table" );
	tSettings.m_iMinWordLen = Max ( hIndex.GetInt ( "min_word_len", 1 ), 1 );
	tSettings.m_sNgramChars = hIndex.GetStr ( "ngram_chars" );
	tSettings.m_sSynonymsFile = hIndex.GetStr ( "exceptions" ); // new option name
	tSettings.m_sIgnoreChars = hIndex.GetStr ( "ignore_chars" );
	tSettings.m_sBlendChars = hIndex.GetStr ( "blend_chars" );
	tSettings.m_sBlendMode = hIndex.GetStr ( "blend_mode" );
	tSettings.m_sIndexingPlugin = hIndex.GetStr ( "index_token_filter" );

	// phrase boundaries
	int iBoundaryStep = Max ( hIndex.GetInt ( "phrase_boundary_step" ), -1 );
	if ( iBoundaryStep!=0 )
		tSettings.m_sBoundary = hIndex.GetStr ( "phrase_boundary" );
}

void sphConfDictionary ( const CSphConfigSection & hIndex, CSphDictSettings & tSettings )
{
	tSettings.m_sMorphology = hIndex.GetStr ( "morphology" );
	tSettings.m_sStopwords = hIndex.GetStr ( "stopwords" );
	tSettings.m_iMinStemmingLen = hIndex.GetInt ( "min_stemming_len", 1 );
	tSettings.m_bStopwordsUnstemmed = hIndex.GetInt ( "stopwords_unstemmed" )!=0;

	for ( CSphVariant * pWordforms = hIndex("wordforms"); pWordforms; pWordforms = pWordforms->m_pNext )
	{
		if ( !pWordforms->cstr() || !*pWordforms->cstr() )
			continue;

		CSphVector<CSphString> dFilesFound;

#if USE_WINDOWS
		WIN32_FIND_DATA tFFData;
		const char * sLastSlash = NULL;
		for ( const char * s = pWordforms->cstr(); *s; s++ )
			if ( *s=='/' || *s=='\\' )
				sLastSlash = s;

		CSphString sPath;
		if ( sLastSlash )
			sPath = pWordforms->strval().SubString ( 0, sLastSlash - pWordforms->cstr() + 1 );

		HANDLE hFind = FindFirstFile ( pWordforms->cstr(), &tFFData );
		if ( hFind!=INVALID_HANDLE_VALUE )
		{
			if ( !sPath.IsEmpty() )
			{
				dFilesFound.Resize ( dFilesFound.GetLength()+1 );
				dFilesFound.Last().SetSprintf ( "%s%s", sPath.cstr(), tFFData.cFileName );
			} else
				dFilesFound.Add ( tFFData.cFileName );

			while ( FindNextFile ( hFind, &tFFData )!=0 )
			{
				if ( !sPath.IsEmpty() )
				{
					dFilesFound.Resize ( dFilesFound.GetLength()+1 );
					dFilesFound.Last().SetSprintf ( "%s%s", sPath.cstr(), tFFData.cFileName );
				} else
					dFilesFound.Add ( tFFData.cFileName );
			}

			FindClose ( hFind );
		}
#else
		glob_t tGlob;
		glob ( pWordforms->cstr(), GLOB_MARK | GLOB_NOSORT, NULL, &tGlob );
		if ( tGlob.gl_pathv )
			for ( int i = 0; i < (int)tGlob.gl_pathc; i++ )
			{
				const char * szPathName = tGlob.gl_pathv[i];
				if ( !szPathName )
					continue;

				int iLen = strlen ( szPathName );
				if ( !iLen || szPathName[iLen-1]=='/' )
					continue;

				dFilesFound.Add ( szPathName );
			}

		globfree ( &tGlob );
#endif

		dFilesFound.Uniq();
		ARRAY_FOREACH ( i, dFilesFound )
			tSettings.m_dWordforms.Add ( dFilesFound[i] );
	}

	if ( hIndex("dict") )
	{
		tSettings.m_bWordDict = true; // default to keywords
		if ( hIndex["dict"]=="crc" )
		{
			sphWarning ( "dict=crc deprecated, use dict=keywords instead" );
			tSettings.m_bWordDict = false;

		} else if ( hIndex["dict"]!="keywords" )
			fprintf ( stdout, "WARNING: unknown dict=%s, defaulting to keywords\n", hIndex["dict"].cstr() );
	}
}

#if USE_RE2
bool sphConfFieldFilter ( const CSphConfigSection & hIndex, CSphFieldFilterSettings & tSettings, CSphString & )
{
	// regular expressions
	tSettings.m_dRegexps.Resize ( 0 );
	for ( CSphVariant * pFilter = hIndex("regexp_filter"); pFilter; pFilter = pFilter->m_pNext )
		tSettings.m_dRegexps.Add ( pFilter->cstr() );

	return tSettings.m_dRegexps.GetLength() > 0;
}
#else
bool sphConfFieldFilter ( const CSphConfigSection & hIndex, CSphFieldFilterSettings &, CSphString & sError )
{
	if ( hIndex ( "regexp_filter" ) )
		sError.SetSprintf ( "regexp_filter specified but no regexp support compiled" );

	return false;
}
#endif

bool sphConfIndex ( const CSphConfigSection & hIndex, CSphIndexSettings & tSettings, CSphString & sError )
{
	// misc settings
	tSettings.m_iMinPrefixLen = Max ( hIndex.GetInt ( "min_prefix_len" ), 0 );
	tSettings.m_iMinInfixLen = Max ( hIndex.GetInt ( "min_infix_len" ), 0 );
	tSettings.m_iMaxSubstringLen = Max ( hIndex.GetInt ( "max_substring_len" ), 0 );
	tSettings.m_iBoundaryStep = Max ( hIndex.GetInt ( "phrase_boundary_step" ), -1 );
	tSettings.m_bIndexExactWords = hIndex.GetInt ( "index_exact_words" )!=0;
	tSettings.m_iOvershortStep = Min ( Max ( hIndex.GetInt ( "overshort_step", 1 ), 0 ), 1 );
	tSettings.m_iStopwordStep = Min ( Max ( hIndex.GetInt ( "stopword_step", 1 ), 0 ), 1 );
	tSettings.m_iEmbeddedLimit = hIndex.GetSize ( "embedded_limit", 16384 );
	tSettings.m_bIndexFieldLens = hIndex.GetInt ( "index_field_lengths" )!=0;
	tSettings.m_sIndexTokenFilter = hIndex.GetStr ( "index_token_filter" );

	// prefix/infix fields
	CSphString sFields;

	sFields = hIndex.GetStr ( "prefix_fields" );
	sFields.ToLower();
	sphSplit ( tSettings.m_dPrefixFields, sFields.cstr() );

	sFields = hIndex.GetStr ( "infix_fields" );
	sFields.ToLower();
	sphSplit ( tSettings.m_dInfixFields, sFields.cstr() );

	if ( tSettings.m_iMinPrefixLen==0 && tSettings.m_dPrefixFields.GetLength()!=0 )
	{
		fprintf ( stdout, "WARNING: min_prefix_len=0, prefix_fields ignored\n" );
		tSettings.m_dPrefixFields.Reset();
	}

	if ( tSettings.m_iMinInfixLen==0 && tSettings.m_dInfixFields.GetLength()!=0 )
	{
		fprintf ( stdout, "WARNING: min_infix_len=0, infix_fields ignored\n" );
		tSettings.m_dInfixFields.Reset();
	}

	// the only way we could have both prefixes and infixes enabled is when specific field subsets are configured
	if ( tSettings.m_iMinInfixLen>0 && tSettings.m_iMinPrefixLen>0
		&& ( !tSettings.m_dPrefixFields.GetLength() || !tSettings.m_dInfixFields.GetLength() ) )
	{
		sError.SetSprintf ( "prefixes and infixes can not both be enabled on all fields" );
		return false;
	}

	tSettings.m_dPrefixFields.Uniq();
	tSettings.m_dInfixFields.Uniq();

	ARRAY_FOREACH ( i, tSettings.m_dPrefixFields )
		if ( tSettings.m_dInfixFields.Contains ( tSettings.m_dPrefixFields[i] ) )
	{
		sError.SetSprintf ( "field '%s' marked both as prefix and infix", tSettings.m_dPrefixFields[i].cstr() );
		return false;
	}

	if ( tSettings.m_iMaxSubstringLen && tSettings.m_iMaxSubstringLen<tSettings.m_iMinInfixLen )
	{
		sError.SetSprintf ( "max_substring_len=%d is less than min_infix_len=%d", tSettings.m_iMaxSubstringLen, tSettings.m_iMinInfixLen );
		return false;
	}

	if ( tSettings.m_iMaxSubstringLen && tSettings.m_iMaxSubstringLen<tSettings.m_iMinPrefixLen )
	{
		sError.SetSprintf ( "max_substring_len=%d is less than min_prefix_len=%d", tSettings.m_iMaxSubstringLen, tSettings.m_iMinPrefixLen );
		return false;
	}

	bool bWordDict = ( strcmp ( hIndex.GetStr ( "dict", "keywords" ), "keywords" )==0 );
	if ( !bWordDict )
		sphWarning ( "dict=crc deprecated, use dict=keywords instead" );

	if ( hIndex("type") && hIndex["type"]=="rt" && ( tSettings.m_iMinInfixLen>0 || tSettings.m_iMinPrefixLen>0 ) && !bWordDict )
	{
		sError.SetSprintf ( "RT indexes support prefixes and infixes with only dict=keywords" );
		return false;
	}

	if ( bWordDict && tSettings.m_iMaxSubstringLen>0 )
	{
		sError.SetSprintf ( "max_substring_len can not be used with dict=keywords" );
		return false;
	}

	// html stripping
	if ( hIndex ( "html_strip" ) )
	{
		tSettings.m_bHtmlStrip = hIndex.GetInt ( "html_strip" )!=0;
		tSettings.m_sHtmlIndexAttrs = hIndex.GetStr ( "html_index_attrs" );
		tSettings.m_sHtmlRemoveElements = hIndex.GetStr ( "html_remove_elements" );
	}

	// docinfo
	tSettings.m_eDocinfo = SPH_DOCINFO_EXTERN;
	if ( hIndex("docinfo") )
	{
		if ( hIndex["docinfo"]=="none" )		tSettings.m_eDocinfo = SPH_DOCINFO_NONE;
		else if ( hIndex["docinfo"]=="inline" )	tSettings.m_eDocinfo = SPH_DOCINFO_INLINE;
		else if ( hIndex["docinfo"]=="extern" )	tSettings.m_eDocinfo = SPH_DOCINFO_EXTERN;
		else
			fprintf ( stdout, "WARNING: unknown docinfo=%s, defaulting to extern\n", hIndex["docinfo"].cstr() );

		if ( tSettings.m_eDocinfo==SPH_DOCINFO_INLINE )
			fprintf ( stdout, "WARNING: docinfo=inline is deprecated, use ondisc_attrs=1 instead\n" );

		if ( tSettings.m_eDocinfo==SPH_DOCINFO_INLINE && tSettings.m_bIndexFieldLens )
		{
			sError.SetSprintf ( "index_field_lengths must be disabled for docinfo=inline" );
			return false;
		}
	}

	// hit format
	// TODO! add the description into documentation.
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
		const CSphString & sValue = hIndex["hitless_words"].strval();
		if ( sValue=="all" )
		{
			tSettings.m_eHitless = SPH_HITLESS_ALL;
		} else
		{
			tSettings.m_eHitless = SPH_HITLESS_SOME;
			tSettings.m_sHitlessFiles = sValue;
		}
	}

	// sentence and paragraph indexing
	tSettings.m_bIndexSP = ( hIndex.GetInt ( "index_sp" )!=0 );
	tSettings.m_sZones = hIndex.GetStr ( "index_zones" );

	// bigrams
	tSettings.m_eBigramIndex = SPH_BIGRAM_NONE;
	if ( hIndex("bigram_index") )
	{
		CSphString s = hIndex["bigram_index"].strval();
		s.ToLower();
		if ( s=="all" )
			tSettings.m_eBigramIndex = SPH_BIGRAM_ALL;
		else if ( s=="first_freq" )
			tSettings.m_eBigramIndex = SPH_BIGRAM_FIRSTFREQ;
		else if ( s=="both_freq" )
			tSettings.m_eBigramIndex = SPH_BIGRAM_BOTHFREQ;
		else
		{
			sError.SetSprintf ( "unknown bigram_index=%s (must be all, first_freq, or both_freq)", s.cstr() );
			return false;
		}
	}

	tSettings.m_sBigramWords = hIndex.GetStr ( "bigram_freq_words" );
	tSettings.m_sBigramWords.Trim();

	bool bEmptyOk = tSettings.m_eBigramIndex==SPH_BIGRAM_NONE || tSettings.m_eBigramIndex==SPH_BIGRAM_ALL;
	if ( bEmptyOk!=tSettings.m_sBigramWords.IsEmpty() )
	{
		sError.SetSprintf ( "bigram_index=%s, bigram_freq_words must%s be empty", hIndex["bigram_index"].cstr(),
			bEmptyOk ? "" : " not" );
		return false;
	}

	// aot
	CSphVector<CSphString> dMorphs;
	sphSplit ( dMorphs, hIndex.GetStr ( "morphology" ) );

	tSettings.m_uAotFilterMask = 0;
	for ( int j=0; j<AOT_LENGTH; ++j )
	{
		char buf_all[20];
		sprintf ( buf_all, "lemmatize_%s_all", AOT_LANGUAGES[j] ); //NOLINT
		ARRAY_FOREACH ( i, dMorphs )
			if ( dMorphs[i]==buf_all )
			{
				tSettings.m_uAotFilterMask |= (1UL) << j;
				break;
			}
	}

	bool bPlainRLP = ARRAY_ANY ( bPlainRLP, dMorphs, dMorphs[_any]=="rlp_chinese" );
	bool bBatchedRLP = ARRAY_ANY ( bBatchedRLP, dMorphs, dMorphs[_any]=="rlp_chinese_batched" );

	if ( bPlainRLP && bBatchedRLP )
	{
		fprintf ( stdout, "WARNING: both rlp_chinese and rlp_chinese_batched options specified; switching to rlp_chinese\n" );
		bBatchedRLP = false;
	}

	tSettings.m_eChineseRLP = bPlainRLP ? SPH_RLP_PLAIN : ( bBatchedRLP ? SPH_RLP_BATCHED : SPH_RLP_NONE );
	tSettings.m_sRLPContext = hIndex.GetStr ( "rlp_context" );

#if !USE_RLP
	if ( tSettings.m_eChineseRLP!=SPH_RLP_NONE || !tSettings.m_sRLPContext.IsEmpty() )
	{
		tSettings.m_eChineseRLP = SPH_RLP_NONE;
		fprintf ( stdout, "WARNING: RLP options specified, but no RLP support compiled; ignoring\n" );
	}
#endif

	// all good
	return true;
}


bool sphFixupIndexSettings ( CSphIndex * pIndex, const CSphConfigSection & hIndex, CSphString & sError, bool bTemplateDict )
{
	bool bTokenizerSpawned = false;

	if ( !pIndex->GetTokenizer () )
	{
		CSphTokenizerSettings tSettings;
		sphConfTokenizer ( hIndex, tSettings );

		ISphTokenizer * pTokenizer = ISphTokenizer::Create ( tSettings, NULL, sError );
		if ( !pTokenizer )
			return false;

		bTokenizerSpawned = true;
		pIndex->SetTokenizer ( pTokenizer );
	}

	if ( !pIndex->GetDictionary () )
	{
		CSphDict * pDict = NULL;
		CSphDictSettings tSettings;
		if ( bTemplateDict )
		{
			sphConfDictionary ( hIndex, tSettings );
			pDict = sphCreateDictionaryTemplate ( tSettings, NULL, pIndex->GetTokenizer (), pIndex->GetName(), sError );
			CSphIndexSettings tIndexSettings = pIndex->GetSettings();
			tIndexSettings.m_uAotFilterMask = sphParseMorphAot ( tSettings.m_sMorphology.cstr() );
			pIndex->Setup ( tIndexSettings );
		} else
		{
			sphConfDictionary ( hIndex, tSettings );
			pDict = sphCreateDictionaryCRC ( tSettings, NULL, pIndex->GetTokenizer (), pIndex->GetName(), sError );
		}
		if ( !pDict )
		{
			sphWarning ( "index '%s': %s", pIndex->GetName(), sError.cstr() );
			return false;
		}

		pIndex->SetDictionary ( pDict );
	}

	if ( bTokenizerSpawned )
	{
		pIndex->SetTokenizer ( ISphTokenizer::CreateMultiformFilter ( pIndex->LeakTokenizer(),
			pIndex->GetDictionary()->GetMultiWordforms () ) );
	}

	pIndex->SetupQueryTokenizer();

	if ( !pIndex->IsStripperInited () )
	{
		CSphIndexSettings tSettings = pIndex->GetSettings ();

		if ( hIndex ( "html_strip" ) )
		{
			tSettings.m_bHtmlStrip = hIndex.GetInt ( "html_strip" )!=0;
			tSettings.m_sHtmlIndexAttrs = hIndex.GetStr ( "html_index_attrs" );
			tSettings.m_sHtmlRemoveElements = hIndex.GetStr ( "html_remove_elements" );
		}
		tSettings.m_sZones = hIndex.GetStr ( "index_zones" );

		pIndex->Setup ( tSettings );
	}

	// exact words fixup, needed for RT indexes
	// cloned from indexer, remove somehow?
	CSphDict * pDict = pIndex->GetDictionary();
	assert ( pDict );

	CSphIndexSettings tSettings = pIndex->GetSettings ();
	bool bNeedExact = ( pDict->HasMorphology() || pDict->GetWordformsFileInfos().GetLength() );
	if ( tSettings.m_bIndexExactWords && !bNeedExact )
	{
		tSettings.m_bIndexExactWords = false;
		pIndex->Setup ( tSettings );
		fprintf ( stdout, "WARNING: no morphology, index_exact_words=1 has no effect, ignoring\n" );
	}

	if ( pDict->GetSettings().m_bWordDict && pDict->HasMorphology() &&
		( tSettings.m_iMinPrefixLen || tSettings.m_iMinInfixLen ) && !tSettings.m_bIndexExactWords )
	{
		tSettings.m_bIndexExactWords = true;
		pIndex->Setup ( tSettings );
		fprintf ( stdout, "WARNING: dict=keywords and prefixes and morphology enabled, forcing index_exact_words=1\n" );
	}

	pIndex->PostSetup();
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
		if ( sphIsReadable ( sOptConfig ) )
			break;
#endif

		sOptConfig = "./sphinx.conf";
		if ( sphIsReadable ( sOptConfig ) )
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

static void StdoutLogger ( ESphLogLevel eLevel, const char * sFmt, va_list ap )
{
	if ( eLevel>=SPH_LOG_DEBUG )
		return;

	switch ( eLevel )
	{
	case SPH_LOG_FATAL: fprintf ( stdout, "FATAL: " ); break;
	case SPH_LOG_WARNING: fprintf ( stdout, "WARNING: " ); break;
	case SPH_LOG_INFO: fprintf ( stdout, "WARNING: " ); break;
	case SPH_LOG_DEBUG: // yes, I know that this branch will never execute because of the condition above.
	case SPH_LOG_VERBOSE_DEBUG:
	case SPH_LOG_VERY_VERBOSE_DEBUG: fprintf ( stdout, "DEBUG: " ); break;
	}

	vfprintf ( stdout, sFmt, ap );
	fprintf ( stdout, "\n" );
}

static SphLogger_fn g_pLogger = &StdoutLogger;

inline void Log ( ESphLogLevel eLevel, const char * sFmt, va_list ap )
{
	if ( !g_pLogger ) return;
	( *g_pLogger ) ( eLevel, sFmt, ap );
}

void sphWarning ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	Log ( SPH_LOG_WARNING, sFmt, ap );
	va_end ( ap );
}


void sphInfo ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	Log ( SPH_LOG_INFO, sFmt, ap );
	va_end ( ap );
}

void sphLogFatal ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	Log ( SPH_LOG_FATAL, sFmt, ap );
	va_end ( ap );
}

void sphLogDebug ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	Log ( SPH_LOG_DEBUG, sFmt, ap );
	va_end ( ap );
}

void sphLogDebugv ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	Log ( SPH_LOG_VERBOSE_DEBUG, sFmt, ap );
	va_end ( ap );
}

void sphLogDebugvv ( const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	Log ( SPH_LOG_VERY_VERBOSE_DEBUG, sFmt, ap );
	va_end ( ap );
}

void sphSetLogger ( SphLogger_fn fnLog )
{
	g_pLogger = fnLog;
}

//////////////////////////////////////////////////////////////////////////
// CRASH REPORTING
//////////////////////////////////////////////////////////////////////////

template <typename Uint>
static void UItoA ( char** ppOutput, Uint uVal, int iBase=10, int iWidth=0, int iPrec=0, const char cFill=' ' )
{
	assert ( ppOutput );
	assert ( *ppOutput );

	const char cDigits[] = "0123456789abcdef";

	if ( iWidth && iPrec )
	{
		iPrec = iWidth;
		iWidth = 0;
	}

	if ( !uVal )
	{
		if ( !iPrec && !iWidth )
			*(*ppOutput)++ = cDigits[0];
		else
		{
			while ( iPrec-- )
				*(*ppOutput)++ = cDigits[0];
			if ( iWidth )
			{
				while ( --iWidth )
					*(*ppOutput)++ = cFill;
				*(*ppOutput)++ = cDigits[0];
			}
		}
		return;
	}

	const BYTE uMaxIndex = 31; // 20 digits for MAX_INT64 in decimal; let it be 31 (32 digits max).
	char CBuf[uMaxIndex+1];
	char *pRes = &CBuf[uMaxIndex];
	char *& pOutput = *ppOutput;

	while ( uVal )
	{
		*pRes-- = cDigits [ uVal % iBase ];
		uVal /= iBase;
	}

	BYTE uLen = (BYTE)( uMaxIndex - (pRes-CBuf) );

	if ( iWidth )
		while ( uLen < iWidth )
		{
			*pOutput++ = cFill;
			iWidth--;
		}

		if ( iPrec )
		{
			while ( uLen < iPrec )
			{
				*pOutput++=cDigits[0];
				iPrec--;
			}
			iPrec = uLen-iPrec;
		}

		while ( pRes < CBuf+uMaxIndex-iPrec )
			*pOutput++ = *++pRes;
}


static int sphVSprintf ( char * pOutput, const char * sFmt, va_list ap )
{
	enum eStates { SNORMAL, SPERCENT, SHAVEFILL, SINWIDTH, SINPREC };
	eStates state = SNORMAL;
	int iPrec = 0;
	int iWidth = 0;
	char cFill = ' ';
	const char * pBegin = pOutput;
	bool bHeadingSpace = true;

	char c;
	while ( ( c = *sFmt++ )!=0 )
	{
		// handle percent
		if ( c=='%' )
		{
			if ( state==SNORMAL )
			{
				state = SPERCENT;
				iPrec = 0;
				iWidth = 0;
				cFill = ' ';
			} else
			{
				state = SNORMAL;
				*pOutput++ = c;
			}
			continue;
		}

		// handle regular chars
		if ( state==SNORMAL )
		{
			*pOutput++ = c;
			continue;
		}

		// handle modifiers
		switch ( c )
		{
		case '0':
			if ( state==SPERCENT )
			{
				cFill = '0';
				state = SHAVEFILL;
				break;
			}
		case '1': case '2': case '3':
		case '4': case '5': case '6':
		case '7': case '8': case '9':
			if ( state==SPERCENT || state==SHAVEFILL )
			{
				state = SINWIDTH;
				iWidth = c - '0';
			} else if ( state==SINWIDTH )
				iWidth = iWidth * 10 + c - '0';
			else if ( state==SINPREC )
				iPrec = iPrec * 10 + c - '0';
			break;

		case '-':
			if ( state==SPERCENT )
				bHeadingSpace = false;
			else
				state = SNORMAL; // FIXME? means that bad/unhandled syntax with dash will be just ignored
			break;

		case '.':
			state = SINPREC;
			iPrec = 0;
			break;

		case 's': // string
			{
				const char * pValue = va_arg ( ap, const char * );
				if ( !pValue )
					pValue = "(null)";
				int iValue = strlen ( pValue );

				if ( iWidth && bHeadingSpace )
					while ( iValue < iWidth-- )
						*pOutput++ = ' ';

				if ( iPrec && iPrec < iValue )
					while ( iPrec-- )
						*pOutput++ = *pValue++;
				else
					while ( *pValue )
						*pOutput++ = *pValue++;

				if ( iWidth && !bHeadingSpace )
					while ( iValue < iWidth-- )
						*pOutput++ = ' ';

				state = SNORMAL;
				break;
			}

		case 'p': // pointer
			{
				void * pValue = va_arg ( ap, void * );
				uint64_t uValue = uint64_t ( pValue );
				UItoA ( &pOutput, uValue, 16, iWidth, iPrec, cFill );
				state = SNORMAL;
				break;
			}

		case 'x': // hex integer
		case 'd': // decimal integer
			{
				DWORD uValue = va_arg ( ap, DWORD );
				UItoA ( &pOutput, uValue, ( c=='x' ) ? 16 : 10, iWidth, iPrec, cFill );
				state = SNORMAL;
				break;
			}

		case 'l': // decimal int64
			{
				int64_t iValue = va_arg ( ap, int64_t );
				UItoA ( &pOutput, iValue, 10, iWidth, iPrec, cFill );
				state = SNORMAL;
				break;
			}

		default:
			state = SNORMAL;
			*pOutput++ = c;
		}
	}

	// final zero to EOL
	*pOutput++ = '\n';
	return pOutput - pBegin;
}


bool sphWrite ( int iFD, const void * pBuf, size_t iSize )
{
	return ( iSize==(size_t)::write ( iFD, pBuf, iSize ) );
}


static char g_sSafeInfoBuf [ 1024 ];

void sphSafeInfo ( int iFD, const char * sFmt, ... )
{
	if ( iFD<0 || !sFmt )
		return;

	va_list ap;
	va_start ( ap, sFmt );
	int iLen = sphVSprintf ( g_sSafeInfoBuf, sFmt, ap ); // FIXME! make this vsnprintf
	va_end ( ap );
	sphWrite ( iFD, g_sSafeInfoBuf, iLen );
}


int sphSafeInfo ( char * pBuf, const char * sFmt, ... )
{
	va_list ap;
	va_start ( ap, sFmt );
	int iLen = sphVSprintf ( pBuf, sFmt, ap ); // FIXME! make this vsnprintf
	va_end ( ap );
	return iLen;
}


#if !USE_WINDOWS

#define SPH_BACKTRACE_ADDR_COUNT 128
#define SPH_BT_BINARY_NAME 2
#define SPH_BT_ADDRS 3
static void * g_pBacktraceAddresses [SPH_BACKTRACE_ADDR_COUNT];
static char g_pBacktrace[4096];
static const char g_sSourceTail[] = "> source.txt\n";
static const char * g_pArgv[128] = { "addr2line", "-e", "./searchd", "0x0", NULL };
static CSphString g_sBinaryName;

#if HAVE_BACKTRACE & HAVE_BACKTRACE_SYMBOLS
const char * DoBacktrace ( int iDepth, int iSkip )
{
	if ( !iDepth || iDepth > SPH_BACKTRACE_ADDR_COUNT )
		iDepth = SPH_BACKTRACE_ADDR_COUNT;
	iDepth = backtrace ( g_pBacktraceAddresses, iDepth );
	char ** ppStrings = backtrace_symbols ( g_pBacktraceAddresses, iDepth );
	if ( !ppStrings )
		return NULL;
	char * pDst = g_pBacktrace;
	for ( int i=iSkip; i<iDepth; ++i )
	{
		const char * pStr = ppStrings[i];
		do
			*pDst++ = *pStr++;
		while (*pStr);
		*pDst++='\n';
	}
	*pDst = '\0';
	free ( ppStrings );
	return g_pBacktrace; ///< sorry, no backtraces on Windows...
}
#else
const char * DoBacktrace ( int iDepth, int iSkip )
{
	return NULL; ///< sorry, no backtraces on Windows...
}
#endif

void sphBacktrace ( int iFD, bool bSafe )
{
	if ( iFD<0 )
		return;

	sphSafeInfo ( iFD, "-------------- backtrace begins here ---------------" );
#ifdef COMPILER
	sphSafeInfo ( iFD, "Program compiled with " COMPILER );
#endif

#ifdef CONFIGURE_FLAGS
	sphSafeInfo ( iFD, "Configured with flags: "CONFIGURE_FLAGS );
#endif

#ifdef OS_UNAME
	sphSafeInfo ( iFD, "Host OS is "OS_UNAME );
#endif

	bool bOk = true;

	void * pMyStack = NULL;
	int iStackSize = 0;
	if ( !bSafe )
	{
		pMyStack = sphMyStack();
		iStackSize = g_iThreadStackSize;
	}
	sphSafeInfo ( iFD, "Stack bottom = 0x%p, thread stack size = 0x%x", pMyStack, iStackSize );

	while ( pMyStack && !bSafe )
	{
		sphSafeInfo ( iFD, "Trying manual backtrace:" );
		BYTE ** pFramePointer = NULL;

		int iFrameCount = 0;
		int iReturnFrameCount = sphIsLtLib() ? 2 : 1;

#ifdef __i386__
#define SIGRETURN_FRAME_OFFSET 17
		__asm __volatile__ ( "movl %%ebp,%0":"=r"(pFramePointer):"r"(pFramePointer) );
#endif

#ifdef __x86_64__
#define SIGRETURN_FRAME_OFFSET 23
		__asm __volatile__ ( "movq %%rbp,%0":"=r"(pFramePointer):"r"(pFramePointer) );
#endif

#ifndef SIGRETURN_FRAME_OFFSET
#define SIGRETURN_FRAME_OFFSET 0
#endif

		if ( !pFramePointer )
		{
			sphSafeInfo ( iFD, "Frame pointer is null, manual backtrace failed (did you build with -fomit-frame-pointer?)" );
			break;
		}

		if ( !pMyStack || (BYTE*) pMyStack > (BYTE*) &pFramePointer )
		{
			int iRound = Min ( 65536, iStackSize );
			pMyStack = (void *) ( ( (size_t) &pFramePointer + iRound ) & ~(size_t)65535 );
			sphSafeInfo ( iFD, "Something wrong with thread stack, manual backtrace may be incorrect (fp=0x%p)", pFramePointer );

			if ( pFramePointer > (BYTE**) pMyStack || pFramePointer < (BYTE**) pMyStack - iStackSize )
			{
				sphSafeInfo ( iFD, "Wrong stack limit or frame pointer, manual backtrace failed (fp=0x%p, stack=0x%p, stacksize=0x%x)",
					pFramePointer, pMyStack, iStackSize );
				break;
			}
		}

		sphSafeInfo ( iFD, "Stack looks OK, attempting backtrace." );

		BYTE** pNewFP = NULL;
		while ( pFramePointer < (BYTE**) pMyStack )
		{
			pNewFP = (BYTE**) *pFramePointer;
			sphSafeInfo ( iFD, "0x%p", iFrameCount==iReturnFrameCount ? *(pFramePointer + SIGRETURN_FRAME_OFFSET) : *(pFramePointer + 1) );

			bOk = pNewFP > pFramePointer;
			if ( !bOk ) break;

			pFramePointer = pNewFP;
			iFrameCount++;
		}

		if ( !bOk )
			sphSafeInfo ( iFD, "Something wrong in frame pointers, manual backtrace failed (fp=%p)", pNewFP );

		break;
	}

	int iDepth = 0;
#if HAVE_BACKTRACE
	sphSafeInfo ( iFD, "Trying system backtrace:" );
	iDepth = backtrace ( g_pBacktraceAddresses, SPH_BACKTRACE_ADDR_COUNT );
	if ( iDepth>0 )
		bOk = true;
#if HAVE_BACKTRACE_SYMBOLS
	sphSafeInfo ( iFD, "begin of system symbols:" );
	backtrace_symbols_fd ( g_pBacktraceAddresses, iDepth, iFD );
#elif !HAVE_BACKTRACE_SYMBOLS
	sphSafeInfo ( iFD, "begin of manual symbols:" );
	for ( int i=0; i<iDepth; i++ )
		sphSafeInfo ( iFD, "%p", g_pBacktraceAddresses[i] );
#endif // HAVE_BACKTRACE_SYMBOLS
#endif // !HAVE_BACKTRACE

	sphSafeInfo ( iFD, "-------------- backtrace ends here ---------------" );

	if ( bOk )
		sphSafeInfo ( iFD, "Please, create a bug report in our bug tracker (http://sphinxsearch.com/bugs) and attach there:\n"
							"a) searchd log, b) searchd binary, c) searchd symbols.\n"
							"Look into the chapter 'Reporting bugs' in the documentation\n"
							"(/usr/share/doc/sphinx/sphinx.txt or http://sphinxsearch.com/docs/current.html#reporting-bugs)" );

	// convert all BT addresses to source code lines
	int iCount = Min ( iDepth, (int)( sizeof(g_pArgv)/sizeof(g_pArgv[0]) - SPH_BT_ADDRS - 1 ) );
	sphSafeInfo ( iFD, "--- BT to source lines (depth %d): ---", iCount );
	char * pCur = g_pBacktrace;
	for ( int i=0; i<iCount; i++ )
	{
		// early our on strings buffer overrun
		if ( pCur>=g_pBacktrace+sizeof(g_pBacktrace)-48 )
		{
			iCount = i;
			break;
		}
		g_pArgv[i+SPH_BT_ADDRS] = pCur;
		pCur += sphSafeInfo ( pCur, "0x%x", g_pBacktraceAddresses[i] );
		*(pCur-1) = '\0'; // make null terminated string from EOL string
	}
	g_pArgv[iCount+SPH_BT_ADDRS] = NULL;

	int iChild = fork();

	if ( iChild==0 )
	{
		// map stdout to log file
		if ( iFD!=1 )
		{
			close ( 1 );
			dup2 ( iFD, 1 );
		}

		execvp ( g_pArgv[0], const_cast<char **> ( g_pArgv ) ); // using execvp instead execv to auto find addr2line in directories

		// if we here - execvp failed, ask user to do conversion manually
		sphSafeInfo ( iFD, "conversion failed (error '%s'):\n"
			"  1. Run the command provided below over the crashed binary (for example, '%s'):\n"
			"  2. Attach the source.txt to the bug report.", strerror ( errno ), g_pArgv[SPH_BT_BINARY_NAME] );

		int iColumn = 0;
		for ( int i=0; g_pArgv[i]!=NULL; i++ )
		{
			const char * s = g_pArgv[i];
			while ( *s )
				s++;
			int iLen = s-g_pArgv[i];
			sphWrite ( iFD, g_pArgv[i], iLen );
			sphWrite ( iFD, " ", 1 );
			int iWas = iColumn % 80;
			iColumn += iLen;
			int iNow = iColumn % 80;
			if ( iNow<iWas )
				sphWrite ( iFD, "\n", 1 );
		}
		sphWrite ( iFD, g_sSourceTail, sizeof(g_sSourceTail)-1 );
		exit ( 1 );

	} else
	if ( iChild==-1 )
	{
		sphSafeInfo ( iFD, "fork for running execvp failed: [%d] %s", errno, strerror(errno) );
		return;
	}

	int iStatus, iResult;
	do
	{
		// can be interrupted by pretty much anything (e.g. SIGCHLD from other searchd children)
		iResult = waitpid ( iChild, &iStatus, 0 );

		// they say this can happen if child exited and SIGCHLD was ignored
		// a cleaner one would be to temporary handle it here, but can we be bothered
		if ( iResult==-1 && errno==ECHILD )
		{
			iResult = iChild;
			iStatus = 0;
		}

		if ( iResult==-1 && errno!=EINTR )
		{
			sphSafeInfo ( iFD, "waitpid() failed: [%d] %s", errno, strerror(errno) );
			return;
		}
	} while ( iResult!=iChild );

	sphSafeInfo ( iFD, "--- BT to source lines finished ---" );
}

void sphBacktraceSetBinaryName ( const char * sName )
{
	g_sBinaryName = sName;
	g_pArgv[SPH_BT_BINARY_NAME] = g_sBinaryName.cstr();
}

#else // USE_WINDOWS

const char * DoBacktrace ( int, int )
{
	return NULL; ///< sorry, no backtraces on Windows...
}

void sphBacktrace ( EXCEPTION_POINTERS * pExc, const char * sFile )
{
	if ( !pExc || !sFile || !(*sFile) )
	{
		sphInfo ( "can't generate minidump" );
		return;
	}

	HANDLE hFile = CreateFile ( sFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0 );
	if ( hFile==INVALID_HANDLE_VALUE )
	{
		sphInfo ( "can't create minidump file '%s'", sFile );
		return;
	}

	MINIDUMP_EXCEPTION_INFORMATION tExcInfo;
	tExcInfo.ExceptionPointers = pExc;
	tExcInfo.ClientPointers = FALSE;
	tExcInfo.ThreadId = GetCurrentThreadId();

	bool bDumped = ( MiniDumpWriteDump ( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &tExcInfo, 0, 0 )==TRUE );
	CloseHandle ( hFile );

	if ( !bDumped )
		sphInfo ( "can't dump minidump" );
}

void sphBacktraceSetBinaryName ( const char * )
{
}

#endif // USE_WINDOWS


static bool g_bUnlinkOld = true;
void sphSetUnlinkOld ( bool bUnlink )
{
	g_bUnlinkOld = bUnlink;
}


void sphUnlinkIndex ( const char * sName, bool bForce )
{
	if ( !( g_bUnlinkOld || bForce ) )
		return;

	char sFileName[SPH_MAX_FILENAME_LEN];

	// +1 is for .mvp
	for ( int i=0; i<sphGetExtCount()+1; i++ )
	{
		snprintf ( sFileName, sizeof(sFileName), "%s%s", sName, sphGetExts ( SPH_EXT_TYPE_CUR )[i] );
		// 'mvp' is optional file
		if ( ::unlink ( sFileName ) && errno!=ENOENT )
			sphWarning ( "unlink failed (file '%s', error '%s'", sFileName, strerror(errno) );
	}
}


void sphCheckDuplicatePaths ( const CSphConfig & hConf )
{
	CSphOrderedHash < CSphString, CSphString, CSphStrHashFunc, 256 > hPaths;
	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext() )
	{
		CSphConfigSection & hIndex = hConf["index"].IterateGet ();
		if ( hIndex ( "path" ) )
		{
			const CSphString & sIndex = hConf["index"].IterateGetKey ();
			if ( hPaths ( hIndex["path"].strval() ) )
				sphDie ( "duplicate paths: index '%s' has the same path as '%s'.\n", sIndex.cstr(), hPaths[hIndex["path"].strval()].cstr() );
			hPaths.Add ( sIndex, hIndex["path"].strval() );
		}
	}
}


void sphConfigureCommon ( const CSphConfig & hConf )
{
	if ( !hConf("common") || !hConf["common"]("common") )
		return;

	CSphConfigSection & hCommon = hConf["common"]["common"];
	g_sLemmatizerBase = hCommon.GetStr ( "lemmatizer_base" );
#if USE_RLP
	g_sRLPRoot = hCommon.GetStr ( "rlp_root" );
	g_sRLPEnv = hCommon.GetStr ( "rlp_environment" );
	g_iRLPMaxBatchSize = hCommon.GetSize ( "rlp_max_batch_size", 51200 );
	g_iRLPMaxBatchDocs = hCommon.GetInt ( "rlp_max_batch_docs", 50 );
#endif
	bool bJsonStrict = false;
	bool bJsonAutoconvNumbers = false;
	bool bJsonKeynamesToLowercase = false;

	if ( hCommon("on_json_attr_error") )
	{
		const CSphString & sVal = hCommon["on_json_attr_error"].strval();
		if ( sVal=="ignore_attr" )
			bJsonStrict = false;
		else if ( sVal=="fail_index" )
			bJsonStrict = true;
		else
			sphDie ( "unknown on_json_attr_error value (must be one of ignore_attr, fail_index)" );
	}

	if ( hCommon("json_autoconv_keynames") )
	{
		const CSphString & sVal = hCommon["json_autoconv_keynames"].strval();
		if ( sVal=="lowercase" )
			bJsonKeynamesToLowercase = true;
		else
			sphDie ( "unknown json_autoconv_keynames value (must be 'lowercase')" );
	}

	bJsonAutoconvNumbers = ( hCommon.GetInt ( "json_autoconv_numbers", 0 )!=0 );
	sphSetJsonOptions ( bJsonStrict, bJsonAutoconvNumbers, bJsonKeynamesToLowercase );

	if ( hCommon("plugin_dir") )
		sphPluginInit ( hCommon["plugin_dir"].cstr() );
}

bool sphIsChineseCode ( int iCode )
{
	return ( ( iCode>=0x2E80 && iCode<=0x2EF3 ) ||	// CJK radicals
		( iCode>=0x2F00 && iCode<=0x2FD5 ) ||	// Kangxi radicals
		( iCode>=0x3105 && iCode<=0x312D ) ||	// Bopomofo
		( iCode>=0x31C0 && iCode<=0x31E3 ) ||	// CJK strokes
		( iCode>=0x3400 && iCode<=0x4DB5 ) ||	// CJK Ideograph Extension A
		( iCode>=0x4E00 && iCode<=0x9FCC ) ||	// Ideograph
		( iCode>=0xF900 && iCode<=0xFAD9 ) ||	// compatibility ideographs
		( iCode>=0x20000 && iCode<=0x2FA1D ) );	// CJK Ideograph Extensions B/C/D, and compatibility ideographs
}

bool sphDetectChinese ( const BYTE * szBuffer, int iLength )
{
	if ( !szBuffer || !iLength )
		return false;

	const BYTE * pBuffer = szBuffer;
	while ( pBuffer<szBuffer+iLength )
	{
		int iCode = sphUTF8Decode ( pBuffer );
		if ( sphIsChineseCode ( iCode ) )
			return true;
	}

	return false;
}


static const char * g_dRankerNames[] =
{
	"proximity_bm25",
	"bm25",
	"none",
	"wordcount",
	"proximity",
	"matchany",
	"fieldmask",
	"sph04",
	"expr",
	"export",
	NULL
};


const char * sphGetRankerName ( ESphRankMode eRanker )
{
	if ( eRanker<SPH_RANK_PROXIMITY_BM25 || eRanker>=SPH_RANK_TOTAL )
		return NULL;

	return g_dRankerNames[eRanker];
}

#if HAVE_DLOPEN

void CSphDynamicLibrary::FillError ( const char * sMessage )
{
	const char* sDlerror = dlerror();
	if ( sMessage )
		m_sError.SetSprintf ( "%s: %s", sMessage, sDlerror ? sDlerror : "(null)" );
	else
		m_sError.SetSprintf ( "%s", sDlerror ? sDlerror : "(null)" );
}

bool CSphDynamicLibrary::Init ( const char * sPath, bool bGlobal )
{
	if ( m_pLibrary )
		return true;
	int iFlags = bGlobal?(RTLD_NOW | RTLD_GLOBAL):(RTLD_LAZY|RTLD_LOCAL);
	m_pLibrary = dlopen ( sPath, iFlags );
	if ( !m_pLibrary )
	{
		FillError ( "dlopen() failed" );
		return false;
	}
	sphLogDebug ( "dlopen(%s)=%p", sPath, m_pLibrary );
	m_bReady = true;
	return m_bReady;
}

bool CSphDynamicLibrary::LoadSymbol ( const char* sName, void** ppFunc )
{
	if ( !m_pLibrary )
		return false;

	if ( !m_bReady )
		return false;

	void * pResult = dlsym ( m_pLibrary, sName );
	if ( !pResult )
	{
		FillError ( "Symbol not found" );
		return false;
	}
	*ppFunc = pResult;
	return true;
}

bool CSphDynamicLibrary::LoadSymbols ( const char** sNames, void*** pppFuncs, int iNum )
{
	if ( !m_pLibrary )
		return false;

	if ( !m_bReady )
		return false;

	for ( int i=0; i<iNum; ++i )
	{
		void* pResult = dlsym ( m_pLibrary, sNames[i] );
		if ( !pResult )
		{
			FillError ( "Symbol not found" );
			return false;
		}
		// yes, it is void*** - triple pointer.
		// void* is the legacy pointer (to the function, in this case).
		// void** is the variable where we store the pointer to the function.
		// that is to cast all different pointers to legacy void*.
		// we put the addresses to these variables into array, and it adds
		// one more level of indirection. void*** actually is void**[]
		*pppFuncs[i] = pResult;
	}

	return true;
};

#else

void CSphDynamicLibrary::FillError ( const char * e ) { m_sError = e; }
bool CSphDynamicLibrary::Init ( const char *, bool ) { return false; }
bool CSphDynamicLibrary::LoadSymbol ( const char *, void ** ) { return false; }
bool CSphDynamicLibrary::LoadSymbols ( const char **, void ***, int ) { return false; }

#endif

//
// $Id$
//
