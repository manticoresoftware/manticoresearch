//
// $Id$
//

#include "sphinx.h"

#include <assert.h>
#include <sys/stat.h>
#if USE_WINDOWS
#else
#include <unistd.h>
#endif

// *** MAIN ***

int main(int argc, char **argv)
{
	CSphSource *	pSource = NULL;
	const char *	sConfName = "sphinx.conf";

	struct stat tStat;
	if ( argc>1 )
	{
		if ( !stat ( argv[1], &tStat ) )
			sConfName = argv[1];
		else
			fprintf ( stderr, "WARNING: can not stat config file '%s', using default 'sphinx.conf'.\n", argv[1] );
	}

	///////////////
	// load config
	///////////////

	CSphConfig		conf;
	CSphHash *		confCommon;
	CSphHash *		confIndexer;

	fprintf ( stdout, "using config file '%s'...\n", sConfName );
	if ( !conf.open ( sConfName ) )
	{
		fprintf ( stderr, "FATAL: unable to open config file '%s'.\n", sConfName );
		return 1;
	}
	confCommon = conf.loadSection ( "common" );
	confIndexer = conf.loadSection ( "indexer" );

	#define CHECK_CONF(_hash,_sect,_key) \
		if ( !_hash->get ( _key ) ) \
		{ \
			fprintf ( stderr, "FATAL: key '%s' not found in config file '%s' section '%s'.\n", sConfName, _key, _sect ); \
			return 1; \
		}

	CHECK_CONF ( confIndexer, "indexer", "type" );
	CHECK_CONF ( confCommon, "common", "index_path" );

	////////////////////
	// spawn datasource
	////////////////////

	char * sType = confIndexer->get ( "type" );

	#if USE_MYSQL
	if ( !strcmp ( sType, "mysql" ) )
	{
		CHECK_CONF ( confIndexer, "indexer", "sql_host" );
		CHECK_CONF ( confIndexer, "indexer", "sql_user" );
		CHECK_CONF ( confIndexer, "indexer", "sql_pass" );
		CHECK_CONF ( confIndexer, "indexer", "sql_db" );
		CHECK_CONF ( confIndexer, "indexer", "sql_query" );

		CSphSource_MySQL * pSrcMysql = new CSphSource_MySQL (
			confIndexer->get ( "sql_query" ),
			confIndexer->get ( "sql_query_pre" ),
			confIndexer->get ( "sql_query_post" ) );
		if ( !pSrcMysql->Connect (
			confIndexer->get ( "sql_host" ),
			confIndexer->get ( "sql_user" ),
			confIndexer->get ( "sql_pass" ),
			confIndexer->get ( "sql_db" ),
			confIndexer->get ( "sql_port" ) ? atoi ( confIndexer->get ( "sql_port" ) ) : 3306,
			confIndexer->get ( "sql_sock" ) ) )
		{
			fprintf ( stderr, "FATAL: CSphSource_MySql: unable to connect to MySQL at mysql://%s@%s:%d/%s.\n",
				confIndexer->get ( "sql_user" ),
				confIndexer->get ( "sql_host" ),
				confIndexer->get ( "sql_port" ) ? atoi ( confIndexer->get ( "sql_port" ) ) : 3306,
				confIndexer->get ( "sql_sock" ) ? confIndexer->get ( "sql_sock" ) : "" );
			delete pSrcMysql;
			return 1;
		}

		pSource = pSrcMysql;
	}
	#endif

	if ( !strcmp ( sType, "xmlpipe" ) )
	{
		CHECK_CONF ( confIndexer, "indexer", "xmlpipe_command" );

		CSphSource_XMLPipe * pSrcXML = new CSphSource_XMLPipe ();
		if ( !pSrcXML->Init ( confIndexer->get ( "xmlpipe_command" ) ) )
		{
			fprintf ( stderr, "FATAL: CSphSource_XMLPipe: unable to popen '%s'.\n",
				confIndexer->get ( "xmlpipe_command" ) );
			delete pSrcXML;
			return 1;
		}

		pSource = pSrcXML;
	}

	if ( !pSource )
	{
		fprintf ( stderr, "FATAL: unknown source type '%s'.", sType );
	}

	//////////
	// index!
	//////////

	// configure morphology
	DWORD iMorph = SPH_MORPH_NONE;
	char * pMorph = confCommon->get ( "morphology" );
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

	// do index
	float fTime = sphLongTimer ();
	fprintf ( stdout, "indexing...\n" );
	fflush ( stdout );

	assert ( pSource );
	CSphIndex * pIndex = sphCreateIndexPhrase ( confCommon->get ( "index_path" ) );
	CSphDict_CRC32 * pDict = new CSphDict_CRC32 ( iMorph );

	// configure stopwords
	char * pStop = confIndexer->get ( "stopwords" );
	if ( pStop )
		if ( !pDict->LoadStopwords ( pStop ) )
			sphDie ( "FATAL: unable to load stopwords from '%s'\n", pStop );

	pIndex->build ( pDict, pSource );

	// trip report
	const CSphSourceStats * pStats = pSource->GetStats ();
	fTime = sphLongTimer () - fTime;

	fprintf ( stdout,
		"indexed %d bytes, %d docs\n"
		"indexed in %.3f sec, %.2f bytes/sec, %.2f docs/sec\n",

		pStats->m_iTotalBytes, pStats->m_iTotalDocuments, fTime, 
		pStats->m_iTotalBytes/fTime, pStats->m_iTotalDocuments/fTime );

	////////////////////
	// cleanup/shutdown
	////////////////////

	delete pIndex;
	delete pDict;
	delete pSource;
}

//
// $Id$
//
