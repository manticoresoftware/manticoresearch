//
// $Id$
//

#include <assert.h>
#include "sphinx.hpp"

// *** MAIN ***

int main(int argc, char **argv)
{
	CSphSource *	pSource = NULL;

	///////////////
	// load config
	///////////////

	CSphConfig		conf;
	CSphHash *		confCommon;
	CSphHash *		confIndexer;

	if ( !conf.open ( "sphinx.conf" ) )
	{
		fprintf ( stderr, "FATAL: unable to open 'sphinx.conf'.\n" );
		return 1;
	}
	confCommon = conf.loadSection ( "common" );
	confIndexer = conf.loadSection ( "indexer" );

	#define CHECK_CONF(_hash,_sect,_key) \
		if ( !_hash->get ( _key ) ) \
		{ \
			fprintf ( stderr, "FATAL: key '%s' not found in 'sphinx.conf' section '%s'.\n", _key, _sect ); \
			return 1; \
		}

	CHECK_CONF ( confIndexer, "indexer", "type" );
	CHECK_CONF ( confCommon, "common", "index_path" );

	////////////////////
	// spawn datasource
	////////////////////

	char * sType = confIndexer->get ( "type" );
	if ( !strcmp ( sType, "mysql" ) )
	{
		CHECK_CONF ( confIndexer, "indexer", "sql_host" );
		CHECK_CONF ( confIndexer, "indexer", "sql_user" );
		CHECK_CONF ( confIndexer, "indexer", "sql_pass" );
		CHECK_CONF ( confIndexer, "indexer", "sql_db" );
		CHECK_CONF ( confIndexer, "indexer", "sql_query" );

		CSphSource_MySQL * pSrcMysql = new CSphSource_MySQL ( confIndexer->get ( "sql_query" ) );
		if ( !pSrcMysql->connect (
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

	} else if ( !strcmp ( sType, "xmlpipe" ) )
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

	} else
	{
		fprintf ( stderr, "FATAL: unknown source type '%s'.", sType );
	}

	//////////
	// index!
	//////////

	assert ( pSource );
	CSphIndex_VLN * pIndex = new CSphIndex_VLN ( confCommon->get ( "index_path" ) );
	CSphDict_CRC32 * pDict = new CSphDict_CRC32 ();
	pIndex->build ( pDict, pSource );

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
