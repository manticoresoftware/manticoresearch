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
#include <time.h>
#if USE_MYSQL
#include <mysql/mysql.h>
#endif


#define CHECK_CONF(_hash,_sect,_key) \
	if ( !_hash->Get ( _key ) ) \
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
	while ( ( sCur = strstr ( sCur, sMacro ) )!=NULL )
	{
		iRes += iDelta;
		sCur++;
	}

	// build result
	char * sRes = new char [ iRes+1 ];
	char * sOut = sRes;
	const char * sLast = sTemplate;
	sCur = sTemplate;

	while ( ( sCur = strstr ( sCur, sMacro ) )!=NULL )
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
	fprintf ( stdout, SPHINX_BANNER );
	if ( argc<=1 )
	{
		fprintf ( stdout,
			"Usage: search [OPTIONS] <word1 [word2 [word3 [...]]]>\n"
			"\n"
			"Options are:\n"
			"-c, --config <file>\tread configuration from specified file\n"
			"\t\t\t(default is sphinx.conf)\n"
			"-a, --any\t\tmatch document if any query word is matched\n"
			"\t\t\t(default is to match all)\n"
			"-g, -group <id>\t\tlimit matching documents to this group\n"
			"\t\t\t(default is not to limit by group)\n"
			"-s, --start <offset>\tstart matches list output from this offset\n"
			"\t\t\t(default is 0)\n"
			"-l, --limit <count>\tlimit matches list output size\n"
			"\t\t\t(default is 20)\n"
			"-q, --noinfo\t\tdo not output document info from SQL database\n"
			"\t\t\t(default is to output)\n"
			"--sort=date\t\tsort by date\n"
			"--rsort=date\t\treverse sort by date\n"
		);
		exit ( 0 );
	}

	///////////////////////////////////////////
	// get query and other commandline options
	///////////////////////////////////////////

	CSphQuery tQuery;
	char sQuery [ 1024 ];
	sQuery[0] = '\0';

	const char * sConfName = "sphinx.conf";
	CSphVector<DWORD> dGroups;
	bool bNoInfo = false;
	int iStart = 0;
	int iLimit = 20;

	#define OPT(_a1,_a2)	else if ( !strcmp(argv[i],_a1) || !strcmp(argv[i],_a2) )
	#define OPT1(_a1)		else if ( !strcmp(argv[i],_a1) )

	int i;
	for ( i=1; i<argc; i++ )
	{
		if ( argv[i][0]=='-' )
		{
			// this is an option
			if ( i==0 );
			OPT ( "-a", "--any" )		tQuery.m_eMode = SPH_MATCH_ANY;
			OPT ( "-p", "--phrase" )	tQuery.m_eMode = SPH_MATCH_PHRASE;
			OPT ( "-q", "--noinfo" )	bNoInfo = true;
			OPT1 ( "--sort=date" )		tQuery.m_eSort = SPH_SORT_DATE_DESC;
			OPT1 ( "--rsort=date" )		tQuery.m_eSort = SPH_SORT_DATE_ASC;
			OPT1 ( "--sort=ts" )		tQuery.m_eSort = SPH_SORT_TIME_SEGMENTS;
			else if ( (i+1)<argc )
			{
				if ( i==0 );
				OPT ( "-g", "--group")		{ if ( atoi ( argv[++i] ) ) dGroups.Add ( atoi ( argv[i] ) ); }
				OPT ( "-s", "--start" )		iStart = atoi ( argv[++i] );
				OPT ( "-l", "--limit" )		iLimit = atoi ( argv[++i] );
				OPT ( "-c", "--config" )	sConfName = argv[++i];
				else break; // unknown option
			}
			else break; // unknown option

		} else if ( strlen(sQuery) + strlen(argv[i]) + 1 < sizeof(sQuery) )
		{
			// this is a search term
			strcat ( sQuery, argv[i] );
			strcat ( sQuery, " " );
		}
	}
	iStart = Max ( iStart, 0 );
	iLimit = Min ( Max ( iLimit, 0 ), 1000 );

	if ( i!=argc )
	{
		fprintf ( stderr, "ERROR: malformed or unknown option near '%s'.\n", argv[i] );
		return 1;
	}

	#undef OPT

	/////////////
	// configure
	/////////////

	// load config
	CSphConfig tConf;
	if ( !tConf.Open ( sConfName ) )
		sphDie ( "FATAL: failed to open config file '%s'.\n", sConfName );

	CSphHash * hCommonConf = tConf.LoadSection ( "common", g_dSphKeysCommon );

	// get index path
	const char * pIndexPath = hCommonConf->Get ( "index_path" );
	if ( !pIndexPath )
			sphDie ( "FATAL: key 'index_path' not found in config file.\n" );

	// get morphology type
	DWORD iMorph = SPH_MORPH_NONE;
	const char * pMorph = hCommonConf ->Get ( "morphology" );
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

	// do we want to show document info from database?
	#if USE_MYSQL
	MYSQL tSqlDriver;
	const char * sQueryInfo = NULL;

	while ( !bNoInfo )
	{
		CSphHash * hSearch = tConf.LoadSection ( "search", g_dSphKeysSearch );
		if ( !hSearch )
			break;

		sQueryInfo = hSearch->Get ( "sql_query_info" );
		if ( !sQueryInfo )
			break;

		if ( !strstr ( sQueryInfo, "$id" ) )
			sphDie ( "FATAL: 'sql_query_info' value must contain '$id'." );

		CSphHash * hIndexer = tConf.LoadSection ( "indexer", g_dSphKeysIndexer );
		if ( !hIndexer )
			sphDie ( "FATAL: section 'indexer' not found in config file." );

		CHECK_CONF ( hIndexer, "indexer", "sql_host" );
		CHECK_CONF ( hIndexer, "indexer", "sql_user" );
		CHECK_CONF ( hIndexer, "indexer", "sql_pass" );
		CHECK_CONF ( hIndexer, "indexer", "sql_db" );

		int iPort = 3306;
		const char * sTmp = hIndexer->Get ( "sql_port" );
		if ( sTmp && atoi(sTmp) )
			iPort = atoi(sTmp);

		mysql_init ( &tSqlDriver );
		if ( !mysql_real_connect ( &tSqlDriver,
			hIndexer->Get ( "sql_host" ),
			hIndexer->Get ( "sql_user" ),
			hIndexer->Get ( "sql_pass" ),
			hIndexer->Get ( "sql_db" ),
			iPort,
			hIndexer->Get ( "sql_sock" ), 0 ) )
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
	pDict->LoadStopwords ( hCommonConf->Get ( "stopwords" ) );

	//////////
	// search
	//////////

	tQuery.m_sQuery = sQuery;
	if ( dGroups.GetLength() )
	{
		tQuery.m_pGroups = new DWORD [ dGroups.GetLength() ];
		tQuery.m_iGroups = dGroups.GetLength();
		memcpy ( tQuery.m_pGroups, &dGroups[0], sizeof(DWORD)*dGroups.GetLength() );
	}

	CSphIndex * pIndex = sphCreateIndexPhrase ( pIndexPath );
	CSphQueryResult * pResult = pIndex->query ( pDict, &tQuery );

	delete pIndex;
	delete pDict;

	if ( !pResult )
	{
		fprintf ( stdout, "query '%s': search error: can not open index.\n", sQuery );
		return 1;
	}

	fprintf ( stdout, "query '%s': returned %d matches of %d total in %.2f sec\n",
		sQuery, pResult->m_dMatches.GetLength(), pResult->m_iTotalMatches, pResult->m_fQueryTime );

	if ( pResult->m_dMatches.GetLength() )
	{
		fprintf ( stdout, "\ndisplaying matches:\n" );

		int iMaxIndex = Min ( iStart+iLimit, pResult->m_dMatches.GetLength() );
		for ( int i=iStart; i<iMaxIndex; i++ )
		{
			CSphMatch & tMatch = pResult->m_dMatches[i];
			fprintf ( stdout, "%d. group=%d, document=%d, weight=%d, time=%s",
				1+i,
				tMatch.m_iGroupID,
				tMatch.m_iDocID,
				tMatch.m_iWeight,
				ctime ( (time_t*)&tMatch.m_iTimestamp ) );

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
