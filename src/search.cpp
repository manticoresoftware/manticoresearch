//
// $Id$
//

#include "sphinx.h"

int main ( int argc, char ** argv )
{
	if ( argc<=1 )
		sphDie ( "usage: search <word1> [word2 [word3 [...]]]\n" );

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

	for ( int i=1; i<argc; i++ )
	{
		if ( strlen(sQuery) + strlen(argv[i]) + 1 >= sizeof(sQuery) )
			break;
		strcat ( sQuery, argv[i] );
		strcat ( sQuery, " " );
	}

	//////////
	// search
	//////////

	CSphIndex * pIndex = new CSphIndex_VLN ( pIndexPath );
	CSphQueryResult * pResult = pIndex->query ( new CSphDict_CRC32 ( iMorph ), sQuery,
		NULL, SPH_MAX_FIELD_COUNT );

	fprintf ( stdout, "query '%s': %d matches, %.2f sec\n",
		sQuery, pResult->m_pMatches->count, pResult->m_fQueryTime );

	if ( pResult->m_pMatches->count )
	{
		fprintf ( stdout, "\nmatches:\n" );
		for ( int i=0; i<pResult->m_pMatches->count; i++ )
		{
			fprintf ( stdout, "%d. group=%d, document=%d, weight=%d\n",
				1+i,
				pResult->m_pMatches->data[i].m_iGroupID,
				pResult->m_pMatches->data[i].m_iDocID,
				pResult->m_pMatches->data[i].m_iWeight );
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
