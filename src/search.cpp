//
// $Id$
//

#include "sphinx.h"

int main ( int argc, char ** argv )
{
	if ( argc<=1 )
		sphDie ( "usage: search [--any] [--group <id>] <word1 [word2 [word3 [...]]]>\n" );

	/////////////
	// configure
	/////////////

	// load config
	CSphConfig tConf;
	if ( !tConf.open ( "sphinx.conf" ) )
		sphDie ( "FATAL: failed to open 'sphinx.conf'.\n" );

	CSphHash * hCommonConf = tConf.loadSection ( "common" );

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

	// get query
	char sQuery [ 1024 ];
	sQuery[0] = '\0';

	bool bAny = false;
	int iGroup = 0;
	for ( int i=1; i<argc; i++ )
	{
		if ( strcmp(argv[i], "--any")==0 )
		{
			bAny = true;
		} else if ( strcmp ( argv[i], "--group" )==0 )
		{
			if ( ++i<argc )
				iGroup = atoi ( argv[i] );
		} else if ( strlen(sQuery) + strlen(argv[i]) + 1 < sizeof(sQuery) )
		{
			strcat ( sQuery, argv[i] );
			strcat ( sQuery, " " );
		}
	}

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

		ARRAY_FOREACH ( i, pResult->m_dMatches )
		{
			CSphMatch & tMatch = pResult->m_dMatches[i];
			fprintf ( stdout, "%d. group=%d, document=%d, weight=%d\n",
				1+i,
				tMatch.m_iGroupID,
				tMatch.m_iDocID,
				tMatch.m_iWeight );
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
