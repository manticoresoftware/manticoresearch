//
// $Id$
//

//
// Copyright (c) 2001-2005, Andrew Aksyonoff. All rights reserved.
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
// HASH
/////////////////////////////////////////////////////////////////////////////

CSphHash::CSphHash ()
{
	m_iCount = 0;
	m_iMax = 32;
	m_ppKeys = new char * [ m_iMax ];
	m_ppValues = new char * [ m_iMax ];
}


CSphHash::~CSphHash()
{
	delete [] m_ppKeys;
	delete [] m_ppValues;
}


void CSphHash::Add ( const char * sKey, const char * sValue )
{
	if ( m_iMax==m_iCount )
	{
		int iNewMax = 2*m_iMax;
		char ** ppKeys = new char * [ iNewMax ];
		char ** ppValues = new char * [ iNewMax ];
		memcpy ( ppKeys, m_ppKeys, m_iMax*sizeof(char*) );
		memcpy ( ppValues, m_ppValues, m_iMax*sizeof(char*) );
		delete [] m_ppKeys;
		delete [] m_ppValues;
		m_ppKeys = ppKeys;
		m_ppValues = ppValues;
	}

	m_ppKeys[m_iCount] = sphDup(sKey);
	m_ppValues[m_iCount] = sphDup(sValue);
	m_iCount++;
}


const char * CSphHash::Get ( const char * sKey )
{
	// NOTE: linear search is ok here, because these hashes
	// are intended to be rather tiny (config sections)
	if ( sKey )
		for ( int i=0; i<m_iCount; i++ )
			if ( !strcmp ( sKey, m_ppKeys[i] ) )
				return m_ppValues[i];
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CONFIG FILE PARSER
/////////////////////////////////////////////////////////////////////////////

CSphConfig::CSphConfig ()
{
	m_pFP = NULL;
	m_sFileName = NULL;
}


CSphConfig::~CSphConfig ()
{
	if ( m_pFP )
	{
		fclose ( m_pFP );
		m_pFP = NULL;
	}
	sphFree ( m_sFileName );
}


int CSphConfig::Open ( const char * file )
{
	m_pFP = fopen ( file, "r" );
	if ( m_pFP )
	{
		m_sFileName = sphDup ( file );
		return 1;
	} else
	{
		return 0;
	}
}


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


static char * sphCat ( char * sDst, const char * sSrc )
{
	sDst = (char *) sphRealloc ( sDst, strlen(sDst) + strlen(sSrc) + 1 );
	strcat ( sDst, sSrc );
	return sDst;
}


bool CSphConfig::ValidateKey ( const char * sKey, const char ** dKnownKeys )
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

	fprintf ( stderr, "WARNING: error in %s:%d, unknown key '%s' in section [%s]\n",
			m_sFileName, m_iLine, sKey, m_sSection );
	return false;
}


CSphHash * CSphConfig::LoadSection ( const char * sSection, const char ** dKnownKeys )
{
	char buf[2048], *p, *pp, *sKey = NULL, *sValue = NULL;
	int l;
	CSphHash * pRes;

	// alloc result set
	pRes = new CSphHash ();
	if ( fseek ( m_pFP, 0, SEEK_SET )<0 )
		 return pRes;

	m_iLine = 0;
	m_sSection = sSection;

	#define CLEAN_CONFIG_LINE() \
		p = trim ( buf ); \
		l = strlen ( p ); \
		pp = strchr ( p, '#' ); \
		if ( pp ) \
		{ \
			*pp = '\0'; \
			l = pp - p; \
		}

	// skip until we find the section
	while ( ( p = fgets ( buf, sizeof(buf), m_pFP ) )!=NULL )
	{
		m_iLine++;
		CLEAN_CONFIG_LINE ();
		if ( p[0]=='['
			&& p[l-1] == ']'
			&& strncmp ( p+1, sSection, strlen(sSection) )==0
			&& 2+(int)strlen(sSection)==l )
		{
			break;
		}
	}
	if ( !p )
	{
		m_sSection = NULL;
		return pRes;
	}

	// load all the config lines until next section or EOF
	while ( ( p = fgets ( buf, sizeof(buf), m_pFP ) )!=NULL )
	{
		m_iLine++;
		CLEAN_CONFIG_LINE ();

		// handle empty strings
		if ( !l )
			continue;

		// next section, bail out
		// FIXME! add more validation
		if ( p[0]=='[')
			break;

		// handle split strings
		if ( p[l-1]=='\\' )
		{
			if ( sKey )
			{
				p[l-1] = ' '; // insert space at the end of the split
				sValue = sphCat ( sValue, p );
				continue;

			} else
			{
				pp = strchr ( p, '=' );
				if ( pp )
				{
					*pp++ = '\0';
					sKey = rtrim ( p );
					if ( ValidateKey ( sKey, dKnownKeys ) )
					{
						p[l-1] = ' '; // insert space at the end of the split
						sKey = sphDup ( p );
						sValue = sphDup ( ltrim ( pp ) );
					} else
					{
						sKey = NULL;
					}
					continue;

				} else
				{
					fprintf ( stderr, "WARNING: error in %s:%d, stray line '%s' in section [%s]\n",
						m_sFileName, m_iLine, p, m_sSection );
					continue;
				}
			}

		} else if ( sKey )
		{
			// previous split-string just ended, so add it to result
			sValue = sphCat ( sValue, p );
			pRes->Add ( sKey, sValue );

			sphFree ( sKey );
			sphFree ( sValue );
			sKey = NULL;
			continue;
		}

		// handle one-line pairs
		pp = strchr ( p, '=' );
		if ( pp )
		{
			*pp++ = '\0';
			p = rtrim ( p );
			if ( ValidateKey ( p, dKnownKeys ) )
			{
				pRes->Add ( p, trim ( pp ) );
				sKey = NULL;
			}
			continue;
		}

		fprintf ( stderr, "WARNING: error in %s:%d, stray line '%s' in section [%s]\n",
			m_sFileName, m_iLine, p, m_sSection );
	}

	// return
	m_sSection = NULL;
	return pRes;
}

//
// $Id$
//
