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

#include "sphinx.h"
#include "sphinxutils.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>

#if USE_WINDOWS
	#define I64FMT		"%I64d"
	#define snprintf	_snprintf
#else
	#define I64FMT		"%lld"

	#include <unistd.h>
#endif

/////////////////////////////////////////////////////////////////////////////

template < typename T > struct CSphMTFHashEntry
{
	char *					m_sKey;
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

	/// accessor
	T * operator [] ( const char * sKey )
	{
		return Find ( sKey );
	}

	/// accesor
	T * Find ( const char * sKey )
	{
		DWORD uHash = HASHFUNC::Hash ( sKey ) % SIZE;

		// find matching entry
		CSphMTFHashEntry<T> * pEntry = m_pData [ uHash ];
		CSphMTFHashEntry<T> * pPrev = NULL;
		while ( pEntry && strcmp ( sKey, pEntry->m_sKey ) )
		{
			pPrev = pEntry;
			pEntry = pEntry->pNext;
		}

		// move to front on access
		if ( pPrev )
		{
			pPrev->m_pNext = pEntry->m_pNext;
			pEntry->m_pNext = m_pData [ uHash ];
			m_pData [ uHash ] = pEntry;
		}

		return pEntry ? &pEntry->m_tValue : NULL;
	}

	/// add record to hash
	/// OPTIMIZE: should pass T not by reference for simple types
	T & Add ( const char * sKey, T & tValue )
	{
		DWORD uHash = HASHFUNC::Hash ( sKey ) % SIZE;

		// find matching entry
		CSphMTFHashEntry<T> * pEntry = m_pData [ uHash ];
		CSphMTFHashEntry<T> * pPrev = NULL;
		while ( pEntry && strcmp ( sKey, pEntry->m_sKey ) )
		{
			pPrev = pEntry;
			pEntry = pEntry->m_pNext;
		}

		if ( !pEntry )
		{
			// not found, add it, but don't MTF
			pEntry = new CSphMTFHashEntry<T>;
			pEntry->m_sKey = sphDup ( sKey );
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


class CSphStopwordBuilderDict : public CSphDict_CRC32
{
public:
						CSphStopwordBuilderDict ();
	virtual				~CSphStopwordBuilderDict () {}
	void				Save ( const char * sOutput, int iTop );

public:
	virtual DWORD		GetWordID ( BYTE * pWord );
	virtual void		LoadStopwords ( const char * sFiles );

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


CSphStopwordBuilderDict::CSphStopwordBuilderDict ()
	: CSphDict_CRC32 ( SPH_MORPH_NONE )
{
}


void CSphStopwordBuilderDict::Save ( const char * sOutput, int iTop )
{
	FILE * fp = fopen ( sOutput, "w+" );
	if ( !fp )
		return;

	CSphVector<Word_t> dTop;
	const CSphMTFHashEntry<int> * it;
	HASH_FOREACH ( it, m_hWords )
	{
		Word_t t;
		t.m_sWord = it->m_sKey;
		t.m_iCount = it->m_tValue;
		dTop.Add ( t );
	}

	dTop.RSort ();

	ARRAY_FOREACH ( i, dTop )
	{
		if ( i>=iTop )
			break;
		fprintf ( fp, "%s\n", dTop[i].m_sWord );
	}

	fclose ( fp );
}


DWORD CSphStopwordBuilderDict::GetWordID ( BYTE * pWord )
{
	int iZero = 0;
	m_hWords.Add ( (const char *)pWord, iZero )++;
	return 1;
}


void CSphStopwordBuilderDict::LoadStopwords ( const char * sFiles )
{
	sFiles = sFiles;
}

/////////////////////////////////////////////////////////////////////////////

void ShowProgress ( const CSphIndexProgress * pProgress )
{
	switch ( pProgress->m_ePhase )
	{
		case CSphIndexProgress::PHASE_COLLECT:
			fprintf ( stdout, "collected %d docs, %.1f MB\r",
				pProgress->m_iDocuments, float(pProgress->m_iBytes)/1000000.0f );
			break;

		case CSphIndexProgress::PHASE_SORT:
			fprintf ( stdout, "sorted %.1f Mhits, %.1f%% done\r",
				float(pProgress->m_iHits)/1000000,
				100.0f*float(pProgress->m_iHits) / float(pProgress->m_iHitsTotal) );
			break;

		case CSphIndexProgress::PHASE_COLLECT_END:
		case CSphIndexProgress::PHASE_SORT_END:
			fprintf ( stdout, "\n" );
			break;
	}
	fflush ( stdout );
}


int main ( int argc, char ** argv )
{
	CSphSource * pSource = NULL;
	const char * sConfName = "sphinx.conf";
	const char * sBuildStops = NULL;
	int iTopStops = 100;
	bool bRotate = false;

	fprintf ( stdout, SPHINX_BANNER );

	int i;
	for ( i=1; i<argc; i++ )
	{
		if ( strcasecmp ( argv[i], "--config" )==0 && (i+1)<argc )
		{
			struct stat tStat;
			if ( !stat ( argv[i+1], &tStat ) )
				sConfName = argv[i+1];
			else
				fprintf ( stderr, "WARNING: can not stat config file '%s', using default 'sphinx.conf'.\n", argv[i+1] );
			i++;
		
		} else if ( strcasecmp ( argv[i], "--buildstops" )==0 && (i+2)<argc )
		{
			sBuildStops = argv[i+1];
			iTopStops = atoi ( argv[i+2] );
			if ( iTopStops<=0 )
				break;
			i += 2;

#if !USE_WINDOWS
		} else if ( strcasecmp ( argv[i], "--rotate" )==0 )
		{
			bRotate = true;
#endif

		} else
		{
			break;
		}
	}
	if ( i!=argc )
	{
		fprintf ( stderr, "ERROR: malformed or unknown option near '%s'.\n\n", argv[i] );
		fprintf ( stderr, "usage: indexer [--config file.conf] [--buildstops output.txt count]" );
#if !USE_WINDOWS
		fprintf ( stderr, " [--rotate]" );
#endif
		fprintf ( stderr, "\n" );
		return 1;
	}

	///////////////
	// load config
	///////////////

	CSphConfig conf;
	CSphHash * confCommon;
	CSphHash * confIndexer;

	fprintf ( stdout, "using config file '%s'...\n", sConfName );
	if ( !conf.Open ( sConfName ) )
	{
		fprintf ( stderr, "FATAL: unable to open config file '%s'.\n", sConfName );
		return 1;
	}
	confCommon = conf.LoadSection ( "common", g_dSphKeysCommon );
	confIndexer = conf.LoadSection ( "indexer", g_dSphKeysIndexer );

	#define CHECK_CONF(_hash,_sect,_key) \
		if ( !_hash->Get(_key) ) \
		{ \
			fprintf ( stderr, "FATAL: key '%s' not found in config file '%s' section '%s'.\n", _key, sConfName, _sect ); \
			return 1; \
		}

	CHECK_CONF ( confIndexer, "indexer", "type" );
	CHECK_CONF ( confCommon, "common", "index_path" );

	/////////////////////////
	// check index lock file
	/////////////////////////
	
	if ( !bRotate )
	{
		char sLockFile [ SPH_MAX_FILENAME_LEN ];
		snprintf ( sLockFile, sizeof(sLockFile), "%s.spl", confCommon->Get ( "index_path" ) );
		sLockFile [ sizeof(sLockFile)-1 ] = '\0';

		struct stat tStat;
		if ( !stat ( sLockFile, &tStat ) )
		{
			fprintf ( stderr, "FATAL: index lock file '%s' exists, will not index. Try --rotate option.\n",
				sLockFile );
			return 1;
		}
	}

	////////////////////
	// spawn datasource
	////////////////////

	const char * sType = confIndexer->Get ( "type" );

	#if USE_MYSQL
	if ( !strcmp ( sType, "mysql" ) )
	{
		CHECK_CONF ( confIndexer, "indexer", "sql_host" );
		CHECK_CONF ( confIndexer, "indexer", "sql_user" );
		CHECK_CONF ( confIndexer, "indexer", "sql_pass" );
		CHECK_CONF ( confIndexer, "indexer", "sql_db" );
		CHECK_CONF ( confIndexer, "indexer", "sql_query" );

		CSphSourceParams_MySQL tParams;

		tParams.m_sQuery		= confIndexer->Get ( "sql_query" );
		tParams.m_sQueryPre		= confIndexer->Get ( "sql_query_pre" );
		tParams.m_sQueryPost	= confIndexer->Get ( "sql_query_post" );
		tParams.m_sQueryRange	= confIndexer->Get ( "sql_query_range" );
		tParams.m_sGroupColumn	= confIndexer->Get ( "sql_group_column" );
		tParams.m_sDateColumn	= confIndexer->Get ( "sql_date_column" );

		tParams.m_sHost			= confIndexer->Get ( "sql_host" );
		tParams.m_sUser			= confIndexer->Get ( "sql_user" );
		tParams.m_sPass			= confIndexer->Get ( "sql_pass" );
		tParams.m_sDB			= confIndexer->Get ( "sql_db" );
		tParams.m_sUsock		= confIndexer->Get ( "sql_sock" );

		const char * sTmp;
		
		sTmp = confIndexer->Get ( "sql_port" );
		if ( sTmp && atoi(sTmp) )
			tParams.m_iPort = atoi(sTmp);

		sTmp = confIndexer->Get ( "sql_range_step" );
		if ( sTmp && atoi(sTmp) )
			tParams.m_iRangeStep = atoi(sTmp);

		CSphSource_MySQL * pSrcMySQL = new CSphSource_MySQL ();
		assert ( pSrcMySQL );

		if ( !pSrcMySQL->Init ( &tParams ) )
		{
			SafeDelete ( pSrcMySQL );
			return 1;
		}

		pSource = pSrcMySQL;
	}
	#endif

	if ( !strcmp ( sType, "xmlpipe" ) )
	{
		CHECK_CONF ( confIndexer, "indexer", "xmlpipe_command" );

		CSphSource_XMLPipe * pSrcXML = new CSphSource_XMLPipe ();
		if ( !pSrcXML->Init ( confIndexer->Get ( "xmlpipe_command" ) ) )
		{
			fprintf ( stderr, "FATAL: CSphSource_XMLPipe: unable to popen '%s'.\n",
				confIndexer->Get ( "xmlpipe_command" ) );
			SafeDelete ( pSrcXML );
			return 1;
		}

		pSource = pSrcXML;
	}

	///////////////////////////////////
	// check and configure data source
	///////////////////////////////////

	if ( !pSource )
		sphDie ( "FATAL: unknown data source type '%s'.", sType );

	// strip_html, index_html_attrs
	if ( confIndexer->Get ( "strip_html" ) )
	{
		const char * sAttrs = NULL;
		if ( atoi ( confIndexer->Get ( "strip_html" ) )==1 )
		{
			sAttrs = confIndexer->Get ( "index_html_attrs" );
			if ( !sAttrs )
				sAttrs = "";
		}
		sAttrs = pSource->SetStripHTML ( sAttrs );
		if ( sAttrs )
			sphDie ( "FATAL: error in section [indexer] key 'index_html_attrs' syntax near '%s'.", sAttrs );
	}

	// charset_type
	ISphTokenizer * pTokenizer = NULL;
	if ( confCommon->Get ( "charset_type" ) )
	{
		if ( !strcmp ( confCommon->Get ( "charset_type" ), "sbcs" ) )
			pTokenizer = sphCreateSBCSTokenizer ();
		else if ( !strcmp ( confCommon->Get ( "charset_type" ), "utf-8" ) )
			pTokenizer = sphCreateUTF8Tokenizer ();
		else
			sphDie ( "FATAL: unknown charset type '%s'.\n", confCommon->Get ( "charset_type" ) );
	} else
	{
		pTokenizer = sphCreateSBCSTokenizer ();
	}

	// charset_table
	assert ( pTokenizer );
	if ( confCommon->Get ( "charset_table" ) )
		if ( !pTokenizer->SetCaseFolding ( confCommon->Get ( "charset_table" ) ) )
			sphDie ( "FATAL: failed to parse 'charset_table', fix your configuration.\n" );

	pSource->SetTokenizer ( pTokenizer );

	///////////
	// do work
	///////////

	float fTime = sphLongTimer ();

	if ( sBuildStops )
	{
		///////////////////
		// build stopwords
		///////////////////

		fprintf ( stdout, "building stopwords list...\n" );
		fflush ( stdout );

		CSphStopwordBuilderDict * pDict = new CSphStopwordBuilderDict ();
		assert ( pDict );

		pSource->SetDict ( pDict );
		while ( pSource->Next() );

		pDict->Save ( sBuildStops, iTopStops );

		SafeDelete ( pDict );

	} else
	{
		///////////////
		// create dict
		///////////////

		// configure morphology
		DWORD iMorph = SPH_MORPH_NONE;
		const char * pMorph = confCommon->Get ( "morphology" );
		if ( pMorph )
		{
			if ( !strcmp ( pMorph, "stem_en" ) )
				iMorph = SPH_MORPH_STEM_EN;
			else if ( !strcmp ( pMorph, "stem_ru" ) )
				iMorph = SPH_MORPH_STEM_RU;
			else if ( !strcmp ( pMorph, "stem_enru" ) )
				iMorph = SPH_MORPH_STEM_EN | SPH_MORPH_STEM_RU;
			else
			{
				fprintf ( stderr, "WARNING: unknown morphology type '%s' ignored.\n", pMorph );
			}
		}

		// create dict
		CSphDict_CRC32 * pDict = new CSphDict_CRC32 ( iMorph );
		assert ( pDict );

		// configure stops
		pDict->LoadStopwords ( confCommon->Get ( "stopwords" ) );

		// configure memlimit
		int iMemLimit = 0;
		char * sMemLimit = sphDup ( confIndexer->Get ( "mem_limit" ) );

		int iLen = sMemLimit ? (int)strlen ( sMemLimit ) : 0;
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
				fprintf ( stderr, "WARNING: bad mem_limit value '%s', using default.\n", sMemLimit );
			} else
			{
				iMemLimit = iScale*iRes;
			}
		}
		sphFree ( sMemLimit );

		//////////
		// index!
		//////////

		fprintf ( stdout, "indexing...\n" );
		fflush ( stdout );

		// do it
		char sIndexPath [ SPH_MAX_FILENAME_LEN ];
		snprintf ( sIndexPath, sizeof(sIndexPath),
			bRotate ? "%s.new" : "%s",
			confCommon->Get ( "index_path" ) );
		sIndexPath [ sizeof(sIndexPath)-1 ] = '\0';

		CSphIndex * pIndex = sphCreateIndexPhrase ( sIndexPath );
		assert ( pIndex );

		pIndex->SetProgressCallback ( ShowProgress );
		pIndex->build ( pDict, pSource, iMemLimit );

		SafeDelete ( pIndex );
		SafeDelete ( pDict );
	}

	// trip report
	const CSphSourceStats * pStats = pSource->GetStats ();
	fTime = sphLongTimer () - fTime;

	fprintf ( stdout, "total %d docs, " I64FMT " bytes\n",
		pStats->m_iTotalDocuments, pStats->m_iTotalBytes );

	fprintf ( stdout, "total %.3f sec, %.2f bytes/sec, %.2f docs/sec\n",
		fTime, pStats->m_iTotalBytes/fTime, pStats->m_iTotalDocuments/fTime );

	////////////////////////////
	// rotating searchd indices
	////////////////////////////

#if !USE_WINDOWS
	bool bOK = false;
	CSphHash * confSearchd = NULL;

	while ( bRotate )
	{
		// load config
		confSearchd = conf.LoadSection ( "searchd", g_dSphKeysSearchd );
		if ( !confSearchd )
		{
			fprintf ( stdout, "WARNING: 'searchd' section not found in config file.\n" );
			break;
		}
		if ( !confSearchd->Get ( "pid_file" ) )
		{
			fprintf ( stdout, "WARNING: 'pid_file' parameter not found in 'searchd' config section.\n" );
			break;
		}

		// read in PID
		int iPID;
		FILE * fp = fopen ( confSearchd->Get ( "pid_file" ), "r" );
		if ( !fp )
		{
			fprintf ( stdout, "WARNING: failed to read pid_file '%s'.\n", confSearchd->Get ( "pid_file" ) );
			break;
		}
		if ( fscanf ( fp, "%d", &iPID )!=1 || iPID<=0 )
		{
			fprintf ( stdout, "WARNING: failed to scanf pid from pid_file '%s'.\n", confSearchd->Get ( "pid_file" ) );
			break;
		}
		fclose ( fp );

		// signal
		int iErr = kill ( iPID, SIGHUP );
		if ( iErr==0 )
		{
			fprintf ( stdout, "rotating indices: succesfully sent SIGHUP to searchd.\n" );
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

		// all ok
		bOK = true;
		break;
	}

	if ( bRotate )
	{
		if ( !bOK )
			fprintf ( stdout, "WARNING: indices NOT rotated.\n" );
		SafeDelete ( confSearchd );
	}
#endif

	////////////////////
	// cleanup/shutdown
	////////////////////

	SafeDelete ( pSource );
	SafeDelete ( pTokenizer );
	return 0;
}

//
// $Id$
//
