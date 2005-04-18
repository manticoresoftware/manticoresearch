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
#if USE_MYSQL
#include <mysql/mysql.h>
#endif


#define CHECK_CONF(_hash,_sect,_key) \
	if ( !_hash->get ( _key ) ) \
	{ \
		fprintf ( stderr, "FATAL: key '%s' not found in config file '%s' section '%s'.\n", _key, sConfName, _sect ); \
		return 1; \
	}


char * strmacro ( const char * sTemplate, const char * sMacro, int iValue )
{
	// expand macro
	char sExp [ 16 ];
	sprintf ( sExp, "%d", iValue );

	// calc lengths
	int iExp = strlen ( sExp );
	int iMacro = strlen ( sMacro );
	int iDelta = iExp-iMacro;

	// calc result length
	int iRes = strlen ( sTemplate );
	const char * sCur = sTemplate;
	while ( ( sCur = strstr ( sCur, sMacro ) ) )
	{
		iRes += iDelta;
		sCur++;
	}

	// build result
	char * sRes = new char [ iRes+1 ];
	char * sOut = sRes;
	const char * sLast = sTemplate;
	sCur = sTemplate;

	while ( ( sCur = strstr ( sCur, sMacro ) ) )
	{
		strncpy ( sOut, sLast, sCur-sLast ); sOut += sCur-sLast;
		strcpy ( sOut, sExp ); sOut += iExp;
		sCur += iMacro;
		sLast = sCur;
	}

	if ( *sLast )
		strcpy ( sOut, sLast );

	assert ( (int)strlen(sRes)==iRes );
	return sRes;
}


int main ( int argc, char ** argv )
{
	const char * sConfName = "sphinx.conf";

	fprintf ( stdout, SPHINX_BANNER );
	if ( argc<=1 )
	{
		sphDie (
			"Usage: search [OPTIONS] <word1 [word2 [word3 [...]]]>\n"
			"\n"
			"Options are:\n"
			"--any\t\t\tmatch document if any query word is matched\n"
			"\t\t\t(default is to match all)\n"
			"-g, -group <id>\t\tlimit matching documents to this group\n"
			"\t\t\t(default is not to limit by group)\n"
			"-s, --start <offset>\tstart matches list output from this offset\n"
			"\t\t\t(default is 0)\n"
			"-l, --limit <count>\tlimit matches list output size\n"
			"\t\t\t(default is 20)\n"
			"-q, --noinfo\t\tdo not output document info from SQL database\n"
			"\t\t\t(default is to output)\n"
		);
	}

	/////////////
	// configure
	/////////////

	// load config
	CSphConfig tConf;
	if ( !tConf.open ( sConfName ) )
		sphDie ( "FATAL: failed to open '%s'.\n", sConfName );

	CSphHash * hCommonConf = tConf.loadSection ( "common", g_dSphKeysCommon );

	// get index path
	char * pIndexPath = hCommonConf->get ( "index_path" );
	if ( !pIndexPath )
			sphDie ( "FATAL: key 'index_path' not found in config file.\n" );

	// get morphology type
	DWORD iMorph = SPH_MORPH_NONE;
	char * pMorph = hCommonConf ->get ( "morphology" );
	if ( pMorph )
	{
		if ( !strcmp ( pMorph, "stem_en" ) )		iMorph = SPH_MORPH_STEM_EN;
		else if ( !strcmp ( pMorph, "stem_ru" ) )	iMorph = SPH_MORPH_STEM_RU;
		else if ( !strcmp ( pMorph, "stem_enru" ) )	iMorph = SPH_MORPH_STEM_EN | SPH_MORPH_STEM_RU;
		else
		{
			fprintf ( stderr, "WARNING: unknown morphology type '%s' ignored.\n", pMorph );
		}
	}

	///////////////////////////////////////////
	// get query and other commandline options
	///////////////////////////////////////////

	char sQuery [ 1024 ];
	sQuery[0] = '\0';

	bool bAny = false;
	bool bNoInfo = false;
	int iGroup = 0;
	int iStart = 0;
	int iLimit = 20;

	for ( int i=1; i<argc; i++ )
	{
		if ( strcmp ( argv[i], "--any" )==0 )
		{
			bAny = true;

		} else if ( strcmp ( argv[i], "--noinfo" )==0
			|| strcmp ( argv[i], "-q" )==0 )
		{
			bNoInfo = true;

		} else if ( strcmp ( argv[i], "--group" )==0
			|| strcmp ( argv[i], "-g" )==0 )
		{
			if ( ++i<argc )
				iGroup = atoi ( argv[i] );

		} else if ( strcmp ( argv[i], "--start" )==0
			|| strcmp ( argv[i], "-s" )==0 )
		{
			if ( ++i<argc )
				iStart = atoi ( argv[i] );

		} else if ( strcmp ( argv[i], "--limit" )==0
			|| strcmp ( argv[i], "-l" )==0 )
		{
			if ( ++i<argc )
				iLimit = atoi ( argv[i] );

		} else if ( strlen(sQuery) + strlen(argv[i]) + 1 < sizeof(sQuery) )
		{
			strcat ( sQuery, argv[i] );
			strcat ( sQuery, " " );
		}
	}
	iStart = Max ( iStart, 0 );
	iLimit = Min ( Max ( iLimit, 0 ), 1000 );

	// do we want to show document info from database?
	#if USE_MYSQL
	MYSQL tSqlDriver;
	const char * sQueryInfo = NULL;

	while ( !bNoInfo )
	{
		CSphHash * hSearch = tConf.loadSection ( "search", g_dSphKeysSearch );
		if ( !hSearch )
			break;

		sQueryInfo = hSearch->get ( "sql_query_info" );
		if ( !sQueryInfo )
			break;

		if ( !strstr ( sQueryInfo, "$id" ) )
			sphDie ( "FATAL: 'sql_query_info' value must contain '$id'." );

		CSphHash * hIndexer = tConf.loadSection ( "indexer", g_dSphKeysIndexer );
		if ( !hIndexer )
			sphDie ( "FATAL: section 'indexer' not found in config file." );

		CHECK_CONF ( hIndexer, "indexer", "sql_host" );
		CHECK_CONF ( hIndexer, "indexer", "sql_user" );
		CHECK_CONF ( hIndexer, "indexer", "sql_pass" );
		CHECK_CONF ( hIndexer, "indexer", "sql_db" );

		int iPort = 3306;
		char * sTmp;
		sTmp = hIndexer->get ( "sql_port" );
		if ( sTmp && atoi(sTmp) )
			iPort = atoi(sTmp);

		mysql_init ( &tSqlDriver );
		if ( !mysql_real_connect ( &tSqlDriver,
			hIndexer->get ( "sql_host" ),
			hIndexer->get ( "sql_user" ),
			hIndexer->get ( "sql_pass" ),
			hIndexer->get ( "sql_db" ),
			iPort,
			hIndexer->get ( "sql_sock" ), 0 ) )
		{
			sphDie ( "FATAL: failed to connect to MySQL (error='%s').",
				mysql_error ( &tSqlDriver ) );
		}

		// all good
		break;
	}
	#endif

	// create dict and configure stopwords
	CSphDict * pDict = new CSphDict_CRC32 ( iMorph );
	pDict->LoadStopwords ( hCommonConf->get ( "stopwords" ) );

	//////////
	// search
	//////////

	CSphQuery tQuery;
	tQuery.m_sQuery = sQuery;
	tQuery.m_bAll = !bAny;
	tQuery.m_iGroup = iGroup;

	CSphIndex * pIndex = sphCreateIndexPhrase ( pIndexPath );
	CSphQueryResult * pResult = pIndex->query ( pDict, &tQuery );

	delete pIndex;
	delete pDict;

	fprintf ( stdout, "query '%s': %d matches, %.2f sec\n",
		sQuery, pResult->m_dMatches.GetLength(), pResult->m_fQueryTime );

	if ( pResult->m_dMatches.GetLength() )
	{
		fprintf ( stdout, "\nmatches:\n" );

		int iMaxIndex = Min ( iStart+iLimit, pResult->m_dMatches.GetLength() );
		for ( int i=iStart; i<iMaxIndex; i++ )
		{
			CSphMatch & tMatch = pResult->m_dMatches[i];
			fprintf ( stdout, "%d. group=%d, document=%d, weight=%d\n",
				1+i,
				tMatch.m_iGroupID,
				tMatch.m_iDocID,
				tMatch.m_iWeight );

			#if USE_MYSQL
			if ( sQueryInfo )
			{
				char * sQuery = strmacro ( sQueryInfo, "$id", tMatch.m_iDocID );
				const char * sError = NULL;

				#define LOC_MYSQL_ERROR(_arg) { sError = _arg; break; }
				do
				{
					if ( mysql_query ( &tSqlDriver, sQuery ) )
						LOC_MYSQL_ERROR ( "mysql_query" );

					MYSQL_RES * pSqlResult = mysql_use_result ( &tSqlDriver );
					if ( !pSqlResult )
						LOC_MYSQL_ERROR ( "mysql_use_result" );

					MYSQL_ROW tRow = mysql_fetch_row ( pSqlResult );
					if ( !tRow )
						LOC_MYSQL_ERROR ( "mysql_fetch_row" );

					fprintf ( stdout, "\n" );
					for ( int iField=0; iField<(int)pSqlResult->field_count; iField++ )
						fprintf ( stdout, "%s=%s\n", pSqlResult->fields[iField].name, tRow[iField] );
					fprintf ( stdout, "\n" );

					mysql_free_result ( pSqlResult );

				} while ( false );

				if ( sError )
					sphDie ( "FATAL: sql_query_info: %s: %s.\n", sError, mysql_error ( &tSqlDriver ) );

				delete [] sQuery;
			}
			#endif
		}

		fprintf ( stdout, "\nwords:\n" );
		for ( int i=0; i<pResult->m_iNumWords; i++ )
		{
			fprintf ( stdout, "%d. '%s': %d documents, %d hits\n",
				1+i,
				pResult->m_tWordStats[i].m_sWord,
				pResult->m_tWordStats[i].m_iDocs,
				pResult->m_tWordStats[i].m_iHits );
		}
	}
}

//
// $Id$
//
