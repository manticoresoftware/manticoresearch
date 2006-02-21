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

#include "sphinx.h"
#include "sphinxutils.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <io.h>

#if USE_WINDOWS
	#define I64FMT		"%I64d"
	#define snprintf	_snprintf
#else
	#define I64FMT		"%lld"

	#include <unistd.h>
#endif

/////////////////////////////////////////////////////////////////////////////

bool g_bQuiet			= false;
bool g_bProgress		= false;

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
	void				Save ( const char * sOutput, int iTop, bool bFreqs );

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


void CSphStopwordBuilderDict::Save ( const char * sOutput, int iTop, bool bFreqs )
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
		if ( bFreqs )
			fprintf ( fp, "%s %d\n", dTop[i].m_sWord, dTop[i].m_iCount );
		else
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
	if ( g_bQuiet )
		return;

	if (
		( g_bProgress && pProgress->m_ePhase==CSphIndexProgress::PHASE_COLLECT ) ||
		( pProgress->m_ePhase==CSphIndexProgress::PHASE_COLLECT_END ) )
	{
		fprintf ( stdout, "collected %d docs, %.1f MB",
			pProgress->m_iDocuments, float(pProgress->m_iBytes)/1000000.0f );
		fprintf ( stdout, pProgress->m_ePhase==CSphIndexProgress::PHASE_COLLECT_END ? "\n" : "\r" );
	}

	if (
		( g_bProgress && pProgress->m_ePhase==CSphIndexProgress::PHASE_SORT ) ||
		( pProgress->m_ePhase==CSphIndexProgress::PHASE_SORT_END ) )
	{
		fprintf ( stdout, "sorted %.1f Mhits, %.1f%% done\r",
			float(pProgress->m_iHits)/1000000,
			100.0f*float(pProgress->m_iHits) / float(pProgress->m_iHitsTotal) );
		fprintf ( stdout, pProgress->m_ePhase==CSphIndexProgress::PHASE_SORT_END ? "\n" : "\r" );
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
	bool bBuildFreqs = false;
	CSphVector < const char *, 16 > dIndexes;
	bool bIndexAll = false;

	int i;
	for ( i=1; i<argc; i++ )
	{
		if ( strcasecmp ( argv[i], "--config" )==0 && (i+1)<argc )
		{
			struct stat tStat;
			if ( !stat ( argv[i+1], &tStat ) )
				sConfName = argv[i+1];
			else
				fprintf ( stdout, "WARNING: can not stat config file '%s', using default 'sphinx.conf'.\n", argv[i+1] );
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

		} else if ( strcasecmp ( argv[i], "--buildfreqs" )==0 )
		{
			bBuildFreqs = true;

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

		} else if ( isalpha(argv[i][0]) || isdigit(argv[i][0]) || argv[i][0]=='_' ) // myisalpha
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
				"--config <file>\t\t\tread configuration from specified file\n"
				"\t\t\t\t(default is sphinx.conf)\n"
				"--all\t\t\t\treindex all configured indexes\n"
				"--quiet\t\t\t\tbe quiet, only print errors\n"
				"--noprogress\t\t\tdo not display progress\n"
				"\t\t\t\t(automatically on if output is not to a tty)\n"
#if !USE_WINDOWS
				"--rotate\t\t\tsend SIGHUP to searchd when indexing is over\n"
				"\t\t\t\tto rotate updated indexes automatically\n"
#endif
				"--buildstops <output.txt> <N>\tbuild top N stopwords and write them\n"
				"\t\t\t\tto specifed file\n"
				"--buildfreqs\t\t\tstore words frequencies to output.txt\n"
				"\t\t\t\t(used with --buildstops only)\n"
				"\n"
				"Examples:\n"
				"indexer --quiet myidx1\t\treindex 'myidx1' defined in 'sphinx.conf'\n"
				"indexer --all\t\t\treindex all indexes defined in 'sphinx.conf'\n" );
		}

		return 1;
	}

	///////////////
	// load config
	///////////////

	if ( !g_bQuiet )
		fprintf ( stdout, "using config file '%s'...\n", sConfName );

	// FIXME! add key validation here. g_dSphKeysCommon, g_dSphKeysIndexer
	CSphConfigParser cp;
	if ( !cp.Parse ( sConfName ) )
	{
		fprintf ( stdout, "FATAL: failed to parse config file '%s'.\n", sConfName );
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
	int iMemLimit = 0;
	if ( hConf.Exists ( "indexer" ) )
	{
		char * sMemLimit = sphDup ( hConf["indexer"]["indexer"]["mem_limit"].cstr() );
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
				fprintf ( stdout, "WARNING: bad mem_limit value '%s', using default.\n", sMemLimit );
			} else
			{
				iMemLimit = iScale*iRes;
			}
		}
		sphFree ( sMemLimit );
	}

	/////////////////////
	// index each index
	////////////////////

	bool bIndexedOk = false; // if any of the indexes are ok

	#define CONF_CHECK(_hash,_key,_msg,_add) \
		if (!( _hash.Exists ( _key ) )) \
		{ \
			fprintf ( stdout, "ERROR: key '%s' not found " _msg "\n", _key, _add ); \
			continue; \
		}

	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext() )
	{
		const CSphConfigSection & hIndex = hConf["index"].IterateGet ();
		const char * sIndexName = hConf["index"].IterateGetKey ().cstr();

		if ( hIndex("type") && hIndex["type"]=="distributed" )
		{
			fprintf ( stdout, "skipping index '%s' (distributed indexes can not be directly indexed)...\n", sIndexName );
			fflush ( stdout );
			continue;
		}

		if ( !bIndexAll )
		{
			bool bIndex = false;
			ARRAY_FOREACH ( i, dIndexes )
				if ( strcasecmp ( sIndexName, dIndexes[i] )==0 )
			{
				bIndex = true;
				break;
			}
			if ( !bIndex )
			{
				if ( !g_bQuiet )
				{
					fprintf ( stdout, "skipping index '%s' (not specified in command line)...\n", sIndexName );
					fflush ( stdout );
				}
				continue;
			}
		}

		if ( !g_bQuiet )
		{
			fprintf ( stdout, "indexing index '%s'...\n", sIndexName );
			fflush ( stdout );
		}

		// check config
		CONF_CHECK ( hIndex, "path", "in index '%s'.", sIndexName );
		CONF_CHECK ( hIndex, "source", "in index '%s'.", sIndexName );
		CONF_CHECK ( hConf["source"], hIndex["source"].cstr(), "in config file '%s'.", sConfName );

		const CSphConfigSection & hSource = hConf["source"][ hIndex["source"] ];
		CSphString & sSourceName = hIndex["source"];

		/////////////////////////
		// check index lock file
		/////////////////////////
	
		if ( !bRotate && !sBuildStops )
		{
			char sLockFile [ SPH_MAX_FILENAME_LEN ];
			snprintf ( sLockFile, sizeof(sLockFile), "%s.spl", hIndex["path"].cstr() );
			sLockFile [ sizeof(sLockFile)-1 ] = '\0';

			struct stat tStat;
			if ( !stat ( sLockFile, &tStat ) )
			{
				fprintf ( stdout, "FATAL: index lock file '%s' exists, will not index. Try --rotate option.\n",
					sLockFile );
				return 1;
			}
		}

		////////////////////
		// spawn datasource
		////////////////////

		if ( !hSource.Exists ( "type" ) )
		{
			fprintf ( stdout, "ERROR: source '%s': type not found.\n", sSourceName.cstr() );
			continue;
		}

		#if USE_MYSQL
		if ( hSource["type"]=="mysql" )
		{
			CONF_CHECK ( hSource, "sql_host", "in source '%s'", hIndex["source"].cstr() );
			CONF_CHECK ( hSource, "sql_user", "in source '%s'", hIndex["source"].cstr() );
			CONF_CHECK ( hSource, "sql_pass", "in source '%s'", hIndex["source"].cstr() );
			CONF_CHECK ( hSource, "sql_db", "in source '%s'", hIndex["source"].cstr() );
			CONF_CHECK ( hSource, "sql_query", "in source '%s'", hIndex["source"].cstr() );

			#define LOC_GET(_key) \
				hSource.Exists(_key) ? hSource[_key].cstr() : NULL;

			CSphSourceParams_MySQL tParams;

			tParams.m_sQuery			= LOC_GET ( "sql_query" );
			tParams.m_sQueryPre			= LOC_GET ( "sql_query_pre" );
			tParams.m_sQueryPost		= LOC_GET ( "sql_query_post" );
			tParams.m_sQueryRange		= LOC_GET ( "sql_query_range" );
			tParams.m_sQueryPostIndex	= LOC_GET ( "sql_query_post_index" );
			tParams.m_sGroupColumn		= LOC_GET ( "sql_group_column" );
			tParams.m_sDateColumn		= LOC_GET ( "sql_date_column" );
			tParams.m_sHost				= LOC_GET ( "sql_host" );
			tParams.m_sUser				= LOC_GET ( "sql_user" );
			tParams.m_sPass				= LOC_GET ( "sql_pass" );
			tParams.m_sDB				= LOC_GET ( "sql_db" );
			tParams.m_sUsock			= LOC_GET ( "sql_sock" );

			#undef LOC_GET
			#define LOC_GET(_arg,_key) \
				if ( hSource.Exists(_key) && hSource[_key].intval() ) \
					_arg = hSource[_key].intval();

			LOC_GET ( tParams.m_iPort,		"sql_port" );
			LOC_GET ( tParams.m_iRangeStep,	"sql_range_step" );

			#undef LOC_GET

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

		if ( hSource["type"]=="xmlpipe" )
		{
			CONF_CHECK ( hSource, "xmlpipe_command", "in source '%s'.", hIndex["source"].cstr() );

			CSphSource_XMLPipe * pSrcXML = new CSphSource_XMLPipe ();
			if ( !pSrcXML->Init ( hSource["xmlpipe_command"].cstr() ) )
			{
				fprintf ( stdout, "FATAL: CSphSource_XMLPipe: unable to popen '%s'.\n", hSource["xmlpipe_command"].cstr() );
				SafeDelete ( pSrcXML );
				return 1;
			}

			pSource = pSrcXML;
		}

		///////////////////////////////////
		// check and configure data source
		///////////////////////////////////

		if ( !pSource )
			sphDie ( "FATAL: unknown data source type '%s' in source '%s'\n.", hSource["type"].cstr(), hIndex["source"].cstr() );

		// strip_html, index_html_attrs
		if ( hSource.Exists ( "strip_html" ) )
		{
			const char * sAttrs = NULL;
			if ( hSource["strip_html"].intval() )
			{
				if ( hSource.Exists ( "index_html_attrs" ) )
					sAttrs = hSource["index_html_attrs"].cstr();
				if ( !sAttrs )
					sAttrs = "";
			}
			sAttrs = pSource->SetStripHTML ( sAttrs );
			if ( sAttrs )
				sphDie ( "FATAL: error in section [indexer] key 'index_html_attrs' syntax near '%s'.", sAttrs );
		}

		// charset_type
		ISphTokenizer * pTokenizer = NULL;
		bool bUseUTF8 = false;
		if ( hIndex.Exists ( "charset_type" ) )
		{
			if ( hIndex["charset_type"]=="sbcs" )
				pTokenizer = sphCreateSBCSTokenizer ();

			else if ( hIndex["charset_type"]=="utf-8" )
			{
				pTokenizer = sphCreateUTF8Tokenizer ();
				bUseUTF8 = true;

			} else
				sphDie ( "FATAL: unknown charset type '%s' in index '%s'.\n",
					hIndex["charset_type"].cstr(), sIndexName );
		} else
		{
			pTokenizer = sphCreateSBCSTokenizer ();
		}

		// charset_table
		assert ( pTokenizer );
		if ( hIndex.Exists ( "charset_table" ) )
			if ( !pTokenizer->SetCaseFolding ( hIndex["charset_table"].cstr() ) )
				sphDie ( "FATAL: failed to parse 'charset_table' in index '%s', fix your configuration.\n", sIndexName );

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

			if ( !g_bQuiet )
			{
				fprintf ( stdout, "building stopwords list...\n" );
				fflush ( stdout );
			}

			CSphStopwordBuilderDict * pDict = new CSphStopwordBuilderDict ();
			assert ( pDict );

			pSource->SetDict ( pDict );
			while ( pSource->Next() );

			pDict->Save ( sBuildStops, iTopStops, bBuildFreqs );

			SafeDelete ( pDict );

		} else
		{
			///////////////
			// create dict
			///////////////

			// configure morphology
			DWORD iMorph = SPH_MORPH_NONE;
			if ( hIndex.Exists ( "morphology" ) )
			{
				if ( hIndex["morphology"]=="stem_en" )
					iMorph = SPH_MORPH_STEM_EN;
				else if ( hIndex["morphology"]=="stem_ru" )
					iMorph = ( bUseUTF8 ? SPH_MORPH_STEM_RU_UTF8 : SPH_MORPH_STEM_RU_CP1251 );
				else if ( hIndex["morphology"]=="stem_enru" )
					iMorph = SPH_MORPH_STEM_EN | ( bUseUTF8 ? SPH_MORPH_STEM_RU_UTF8 : SPH_MORPH_STEM_RU_CP1251 );
				else if ( !hIndex["morphology"].IsEmpty() && hIndex["morphology"]!="none" )
					fprintf ( stdout, "WARNING: unknown morphology type '%s' ignored in index '%s'.\n",
						hIndex["morphology"].cstr(), sIndexName );
			}

			// create dict
			CSphDict_CRC32 * pDict = new CSphDict_CRC32 ( iMorph );
			assert ( pDict );

			// configure stops
			if ( hIndex.Exists ( "stopwords" ) )
				pDict->LoadStopwords ( hIndex["stopwords"].cstr(), pTokenizer );

			//////////
			// index!
			//////////

			// do it
			char sIndexPath [ SPH_MAX_FILENAME_LEN ];
			snprintf ( sIndexPath, sizeof(sIndexPath), bRotate ? "%s.new" : "%s", hIndex["path"].cstr() );
			sIndexPath [ sizeof(sIndexPath)-1 ] = '\0';

			CSphIndex * pIndex = sphCreateIndexPhrase ( sIndexPath );
			assert ( pIndex );

			pIndex->SetProgressCallback ( ShowProgress );
			if ( pIndex->Build ( pDict, pSource, iMemLimit ) )
				bIndexedOk = true;

			SafeDelete ( pIndex );
			SafeDelete ( pDict );
		}

		// trip report
		const CSphSourceStats * pStats = pSource->GetStats ();
		fTime = sphLongTimer () - fTime;

		if ( !g_bQuiet )
		{
			fprintf ( stdout, "total %d docs, " I64FMT " bytes\n",
				pStats->m_iTotalDocuments, pStats->m_iTotalBytes );

			fprintf ( stdout, "total %.3f sec, %.2f bytes/sec, %.2f docs/sec\n",
				fTime, pStats->m_iTotalBytes/fTime, pStats->m_iTotalDocuments/fTime );
		}

		// cleanup and go on
		SafeDelete ( pSource );
		SafeDelete ( pTokenizer );
	}

	////////////////////////////
	// rotating searchd indices
	////////////////////////////

#if !USE_WINDOWS
	if ( bIndexedOk )
	{
		bool bOK = false;
		while ( bRotate )
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
				fprintf ( stdout, "WARNING: failed to read pid_file '%s'.\n", hSearchd["pid_file"].cstr() );
				break;
			}
			if ( fscanf ( fp, "%d", &iPID )!=1 || iPID<=0 )
			{
				fprintf ( stdout, "WARNING: failed to scanf pid from pid_file '%s'.\n", hSearchd["pid_file"].cstr() );
				break;
			}
			fclose ( fp );

			// signal
			int iErr = kill ( iPID, SIGHUP );
			if ( iErr==0 )
			{
				if ( !g_bQuiet )
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
		}
	}
#endif

	return 0;
}

//
// $Id$
//
