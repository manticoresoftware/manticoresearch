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
		sphDie ( "FATAL: failed to open 'sphinx.conf'." );

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
	strcat ( sQuery, "" );

	for ( int i=1; i<argc; i++ )
	{
		if ( strlen(sQuery) + strlen(argv[i]) + 1 >= sizeof(sQuery) )
			break;
		strcat ( sQuery, argv[i] );
	}

	// build weights
	int dWeights [ SPH_MAX_FIELD_COUNT ];
	for ( int i=0; i<SPH_MAX_FIELD_COUNT; i++ )
		dWeights[i] = 0;

	//////////
	// search
	//////////

	CSphIndex * pIndex = new CSphIndex_VLN ( pIndexPath );
	CSphQueryResult * pResult = pIndex->query ( new CSphDict_CRC32 ( iMorph ), sQuery,
		dWeights, SPH_MAX_FIELD_COUNT );

	fprintf ( stdout, "query '%s': %d matches, %.2f sec\n",
		sQuery, pResult->matches->count, pResult->queryTime/1000000.0f );

	if ( pResult->matches->count )
	{
		fprintf ( stdout, "\nmatches:\n" );
		for ( int i=0; i<pResult->matches->count; i++ )
		{
			fprintf ( stdout, "%d. group=%d, title=%d, weight=%d\n",
				i,
				pResult->matches->data[i].m_iGroupID,
				pResult->matches->data[i].m_iDocID,
				pResult->matches->data[i].m_iWeight );
		}

		fprintf ( stdout, "\nwords:\n" );
		for ( int i=0; i<pResult->numWords; i++ )
		{
			fprintf ( stdout, "%d. '%s': %d documents, %d hits\n",
				i, pResult->wordStats[i].word,
				pResult->wordStats[i].docs, pResult->wordStats[i].hits );
		}
	}
}

//
// $Id$
//
