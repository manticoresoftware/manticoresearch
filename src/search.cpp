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

#include "sphinx.h"
#include "sphinxutils.h"
#include <time.h>


#define CONF_CHECK(_hash,_key,_msg,_add) \
	if (!( _hash.Exists ( _key ) )) \
	{ \
		fprintf ( stdout, "ERROR: key '%s' not found " _msg, _key, _add ); \
		continue; \
	}


const char * myctime ( DWORD uStamp )
{
	static char sBuf[256];
	time_t tStamp = uStamp; // for 64-bit
	strncpy ( sBuf, ctime(&tStamp), sizeof(sBuf) );

	char * p = sBuf;
	while ( (*p) && (*p)!='\n' && (*p)!='\r' ) p++;
	*p = '\0';

	return sBuf;
}


void DumpHeader ( const char * sHeaderName )
{
	CSphIndex * pIndex = sphCreateIndexPhrase ( "", false, false );
	pIndex->DumpHeader ( stdout, sHeaderName );
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
			"-c, --config <file>\tuse given config file instead of defaults\n"
			"-i, --index <index>\tsearch given index only (default: all indexes)\n"
			"-a, --any\t\tmatch any query word (default: match all words)\n"
			"-b, --boolean\t\tmatch in boolean mode\n"
			"-p, --phrase\t\tmatch exact phrase\n"
			"-e, --extended\t\tmatch in extended mode\n"
			"-f, --filter <attr> <v>\tonly match if attribute attr value is v\n"
			"-s, --sortby <CLAUSE>\tsort matches by 'CLAUSE' in sort_extended mode\n"
			"-S, --sortexpr <EXPR>\tsort matches by 'EXPR' DESC in sort_expr mode\n"
			"-o, --offset <offset>\tprint matches starting from this offset (default: 0)\n"
			"-l, --limit <count>\tprint this many matches (default: 20)\n"
			"-q, --noinfo\t\tdon't print document info from SQL database\n"
			"-g, --group <attr>\tgroup by attribute named attr\n"
			"-gs,--groupsort <expr>\tsort groups by <expr>\n"
			"--sort=date\t\tsort by date, descending\n"
			"--rsort=date\t\tsort by date, ascending\n"
			"--sort=ts\t\tsort by time segments\n"
			"--stdin\t\t\tread query from stdin\n"
			"\n"
			"This program (CLI search) is for testing and debugging purposes only;\n"
			"it is NOT intended for production use.\n"
		);
		exit ( 0 );
	}

	///////////////////////////////////////////
	// get query and other commandline options
	///////////////////////////////////////////

	CSphQuery tQuery;
	char sQuery [ 1024 ];
	sQuery[0] = '\0';

	const char * sOptConfig = NULL;
	const char * sIndex = NULL;
	bool bNoInfo = false;
	bool bStdin = false;
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
			OPT ( "-b", "--boolean" )	tQuery.m_eMode = SPH_MATCH_BOOLEAN;
			OPT ( "-p", "--phrase" )	tQuery.m_eMode = SPH_MATCH_PHRASE;
			OPT ( "-e", "--ext" )		tQuery.m_eMode = SPH_MATCH_EXTENDED;
			OPT ( "-e2", "--ext2" )		tQuery.m_eMode = SPH_MATCH_EXTENDED2;
			OPT ( "-q", "--noinfo" )	bNoInfo = true;
			OPT1 ( "--sort=date" )		tQuery.m_eSort = SPH_SORT_ATTR_DESC;
			OPT1 ( "--rsort=date" )		tQuery.m_eSort = SPH_SORT_ATTR_ASC;
			OPT1 ( "--sort=ts" )		tQuery.m_eSort = SPH_SORT_TIME_SEGMENTS;
			OPT1 ( "--stdin" )			bStdin = true;

			else if ( (i+1)>=argc )		break;
			OPT ( "-o", "--offset" )	iStart = atoi ( argv[++i] );
			OPT ( "-l", "--limit" )		iLimit = atoi ( argv[++i] );
			OPT ( "-c", "--config" )	sOptConfig = argv[++i];
			OPT ( "-i", "--index" )		sIndex = argv[++i];
			OPT ( "-g", "--group" )		{ tQuery.m_eGroupFunc = SPH_GROUPBY_ATTR; tQuery.m_sGroupBy = argv[++i]; }
			OPT ( "-gs","--groupsort" )	{ tQuery.m_sGroupSortBy = argv[++i]; }
			OPT ( "-s", "--sortby" )	{ tQuery.m_eSort = SPH_SORT_EXTENDED; tQuery.m_sSortBy = argv[++i]; }
			OPT ( "-S", "--sortexpr" )	{ tQuery.m_eSort = SPH_SORT_EXPR; tQuery.m_sSortBy = argv[++i]; }
			OPT1 ( "--dumpheader" )		{ DumpHeader ( argv[++i] ); exit ( 0 ); } // my secret option

			else if ( (i+2)>=argc )		break;
			OPT ( "-f", "--filter" )
			{
				DWORD uVal = strtoul ( argv[i+2], NULL, 10 );
				tQuery.m_dFilters.Reset ();
				tQuery.m_dFilters.Resize ( 1 );
				tQuery.m_dFilters[0].m_eType = SPH_FILTER_VALUES;
				tQuery.m_dFilters[0].m_dValues.Reset ();
				tQuery.m_dFilters[0].m_dValues.Add ( uVal );
				tQuery.m_dFilters[0].m_sAttrName = argv[i+1];
				i += 2;
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
	iLimit = Max ( iLimit, 0 );

	if ( i!=argc )
	{
		fprintf ( stdout, "ERROR: malformed or unknown option near '%s'.\n", argv[i] );
		return 1;
	}

	#undef OPT

	if ( bStdin )
	{
		int iPos = 0, iLeft = sizeof(sQuery)-1;
		char sThrowaway [ 256 ];

		while ( !feof(stdin) )
		{
			if ( iLeft>0 )
			{
				int iLen = fread ( sQuery, 1, iLeft, stdin );
				iPos += iLen;
				iLeft -= iLen;
			} else
			{
				fread ( sThrowaway, 1, sizeof(sThrowaway), stdin );
			}
		}

		assert ( iPos<(int)sizeof(sQuery) );
		sQuery[iPos] = '\0';
	}

	/////////////
	// configure
	/////////////

	tQuery.m_iMaxMatches = Max ( 1000, iStart + iLimit );

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

	fprintf ( stdout, "using config file '%s'...\n", sOptConfig );

	// load config
	CSphConfigParser cp;
	if ( !cp.Parse ( sOptConfig ) )
		sphDie ( "failed to parse config file '%s'", sOptConfig );

	CSphConfig & hConf = cp.m_tConf;
	if ( !hConf.Exists ( "index" ) )
		sphDie ( "no indexes found in config file '%s'", sOptConfig );

	/////////////////////
	// search each index
	/////////////////////

	hConf["index"].IterateStart ();
	while ( hConf["index"].IterateNext () )
	{
		const CSphConfigSection & hIndex = hConf["index"].IterateGet ();
		const char * sIndexName = hConf["index"].IterateGetKey().cstr();

		if ( sIndex && strcmp ( sIndex, sIndexName ) )
			continue;

		if ( hIndex("type") && hIndex["type"]=="distributed" )
			continue;

		if ( !hIndex.Exists ( "path" ) )
			sphDie ( "key 'path' not found in index '%s'", sIndexName );

		// configure charset_type
		CSphString sError;
		ISphTokenizer * pTokenizer = sphConfTokenizer ( hIndex, sError );
		if ( !pTokenizer )
			sphDie ( "index '%s': %s", sIndexName, sError.cstr() );

		// do we want to show document info from database?
		#if USE_MYSQL
		MYSQL tSqlDriver;
		const char * sQueryInfo = NULL;

		while ( !bNoInfo )
		{
			if ( !hIndex.Exists ( "source" )
				|| !hConf.Exists ( "source" )
				|| !hConf["source"].Exists ( hIndex["source"] ) )
			{
				break;
			}
			const CSphConfigSection & hSource = hConf["source"][ hIndex["source"] ];

			if ( !hSource.Exists ( "sql_host" )
				|| !hSource.Exists ( "sql_user" )
				|| !hSource.Exists ( "sql_db" )
				|| !hSource.Exists ( "sql_pass" ) )
			{
				break;
			}

			if ( !hSource.Exists ( "sql_query_info" ) )
				break;
			sQueryInfo = hSource["sql_query_info"].cstr();
			if ( !strstr ( sQueryInfo, "$id" ) )
				sphDie ( "'sql_query_info' value must contain '$id'" );

			int iPort = 3306;
			if ( hSource.Exists ( "sql_port" ) && hSource["sql_port"].intval() )
				iPort = hSource["sql_port"].intval();

			mysql_init ( &tSqlDriver );
			if ( !mysql_real_connect ( &tSqlDriver,
				hSource["sql_host"].cstr(),
				hSource["sql_user"].cstr(),
				hSource["sql_pass"].cstr(),
				hSource["sql_db"].cstr(),
				iPort,
				hSource.Exists ( "sql_sock" ) ? hSource["sql_sock"].cstr() : NULL,
				0 ) )
			{
				sphDie ( "failed to connect to MySQL (error=%s)", mysql_error ( &tSqlDriver ) );
			}

			// all good
			break;
		}
		#endif

		// create dict
		CSphDict * pDict = sphCreateDictionaryCRC ( hIndex ("morphology"), hIndex.Exists ( "stopwords" ) ? hIndex["stopwords"].cstr () : NULL,
								hIndex.Exists ( "wordforms" ) ? hIndex ["wordforms"].cstr () : NULL, pTokenizer, sError );
		assert ( pDict );

		if ( !sError.IsEmpty () )
			fprintf ( stdout, "WARNING: index '%s': %s\n", sIndexName, sError.cstr() );	

		//////////
		// search
		//////////

		tQuery.m_sQuery = sQuery;
		CSphQueryResult * pResult = NULL;

		CSphIndex * pIndex = sphCreateIndexPhrase ( hIndex["path"].cstr(), hIndex.GetInt ( "enable_star" ) != 0, false );
		sError = "could not create index (check that files exist)";
		for ( ; pIndex; )
		{
			const CSphSchema * pSchema = pIndex->Prealloc ( false, NULL );
			if ( !pSchema || !pIndex->Preread() )
			{
				sError = pIndex->GetLastError ();
				break;
			}

			// lookup first timestamp if needed
			// FIXME! remove this?
			if ( tQuery.m_eSort!=SPH_SORT_RELEVANCE && tQuery.m_eSort!=SPH_SORT_EXTENDED && tQuery.m_eSort!=SPH_SORT_EXPR )
			{
				int iTS = -1;
				for ( int i=0; i<pSchema->GetAttrsCount(); i++ )
					if ( pSchema->GetAttr(i).m_eAttrType==SPH_ATTR_TIMESTAMP )
				{
					tQuery.m_sSortBy = pSchema->GetAttr(i).m_sName;
					iTS = i;
					break;
				}
				if ( iTS<0 )
				{
					fprintf ( stdout, "index '%s': no timestamp attributes found, sorting by relevance.\n", sIndexName );
					tQuery.m_eSort = SPH_SORT_RELEVANCE;
				}
			}

			pResult = pIndex->Query ( pTokenizer, pDict, &tQuery );
			if ( !pResult )
				sError = pIndex->GetLastError ();

			break;
		}

		/////////
		// print
		/////////

		if ( !pResult )
		{
			fprintf ( stdout, "index '%s': search error: %s.\n", sIndexName, sError.cstr() );
			return 1;
		}

		fprintf ( stdout, "index '%s': query '%s': returned %d matches of %d total in %d.%03d sec\n",
			sIndexName, sQuery, pResult->m_dMatches.GetLength(), pResult->m_iTotalMatches,
			pResult->m_iQueryTime/1000, pResult->m_iQueryTime%1000 );

		if ( pResult->m_dMatches.GetLength() )
		{
			fprintf ( stdout, "\ndisplaying matches:\n" );

			int iMaxIndex = Min ( iStart+iLimit, pResult->m_dMatches.GetLength() );
			for ( int i=iStart; i<iMaxIndex; i++ )
			{
				CSphMatch & tMatch = pResult->m_dMatches[i];
				fprintf ( stdout, "%d. document=" DOCID_FMT ", weight=%d", 1+i, tMatch.m_iDocID, tMatch.m_iWeight );

				for ( int j=0; j<pResult->m_tSchema.GetAttrsCount(); j++ )
				{
					const CSphColumnInfo & tAttr = pResult->m_tSchema.GetAttr(j);
					fprintf ( stdout, ", %s=", tAttr.m_sName.cstr() );

					if ( tAttr.m_eAttrType & SPH_ATTR_MULTI )
					{
						fprintf ( stdout, "(" );
						int iIndex = tMatch.GetAttr ( tAttr.m_iRowitem );
						if ( iIndex )
						{
							const DWORD * pValues = pResult->m_pMva + iIndex;
							int iValues = *pValues++;
							for ( int k=0; k<iValues; k++ )
								fprintf ( stdout, k ? ",%u" : "%u", *pValues++ );
						}
						fprintf ( stdout, ")" );

					} else switch ( tAttr.m_eAttrType )
					{
						case SPH_ATTR_INTEGER:
						case SPH_ATTR_ORDINAL:
						case SPH_ATTR_BOOL:			fprintf ( stdout, "%u", tMatch.GetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount ) ); break;
						case SPH_ATTR_TIMESTAMP:	fprintf ( stdout, "%s", myctime ( tMatch.GetAttr ( tAttr.m_iBitOffset, tAttr.m_iBitCount ) ) ); break;
						case SPH_ATTR_FLOAT:		fprintf ( stdout, "%f", tMatch.GetAttrFloat ( tAttr.m_iRowitem ) ); break;
						default:					fprintf ( stdout, "(unknown-type-%d)", tAttr.m_eAttrType );
					}
				}
				fprintf ( stdout,"\n" );

				#if USE_MYSQL
				if ( sQueryInfo )
				{
					char * sQuery = sphStrMacro ( sQueryInfo, "$id", tMatch.m_iDocID );
					const char * sError = NULL;

					#define LOC_MYSQL_ERROR(_arg) { sError = _arg; break; }
					for ( ;; )
					{
						if ( mysql_query ( &tSqlDriver, sQuery ) )
							LOC_MYSQL_ERROR ( "mysql_query" );

						MYSQL_RES * pSqlResult = mysql_use_result ( &tSqlDriver );
						if ( !pSqlResult )
							LOC_MYSQL_ERROR ( "mysql_use_result" );

						MYSQL_ROW tRow = mysql_fetch_row ( pSqlResult );
						if ( !tRow )
							LOC_MYSQL_ERROR ( "mysql_fetch_row" );

						for ( int iField=0; iField<(int)pSqlResult->field_count; iField++ )
							fprintf ( stdout, "\t%s=%s\n", pSqlResult->fields[iField].name, tRow[iField] );

						mysql_free_result ( pSqlResult );
						break;
					}

					if ( sError )
						sphDie ( "sql_query_info: %s: %s", sError, mysql_error ( &tSqlDriver ) );

					delete [] sQuery;
				}
				#endif
			}
		}

		fprintf ( stdout, "\nwords:\n" );
		for ( int i=0; i<pResult->m_iNumWords; i++ )
		{
			fprintf ( stdout, "%d. '%s': %d documents, %d hits\n",
				1+i,
				pResult->m_tWordStats[i].m_sWord.cstr(),
				pResult->m_tWordStats[i].m_iDocs,
				pResult->m_tWordStats[i].m_iHits );
		}
		fprintf ( stdout, "\n" );

		///////////
		// cleanup
		///////////

		SafeDelete ( pIndex );
		SafeDelete ( pDict );
		SafeDelete ( pTokenizer );
	}

	sphShutdownWordforms ();
}

//
// $Id$
//
