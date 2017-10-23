//
// Created by alexey on 28.08.17.
//

#include <gtest/gtest.h>

#include "sphinxint.h"

// global stuff

const char * g_sTmpfile = "__libsphinxtest.tmp";
const char * g_sMagickTmpfile = "__libsphinxtest2.tmp";
const char * g_sMagic = "\xD1\x82\xD0\xB5\xD1\x81\xD1\x82\xD1\x82\xD1\x82";

bool CreateSynonymsFile ( const char * sMagic = nullptr )
{
	FILE * fp = fopen ( sMagic ? g_sMagickTmpfile : g_sTmpfile, "w+" );
	if ( !fp )
		return false;

	fprintf ( fp, "AT&T      => AT&T\n"
			"   AT & T => AT & T  \n"
			"standarten fuehrer => Standartenfuehrer\n"
			"standarten   fuhrer  => Standartenfuehrer\n"
			"OS/2 => OS/2\n"
			"Ms-Dos => MS-DOS\n"
			"MS DOS => MS-DOS\n"
			"feat. => featuring\n"
			"U.S. => US\n"
			"U.S.A. => USA\n"
			"U.S.B. => USB\n"
			"U.S.D. => USD\n"
			"U.S.P. => USP\n"
			"U.S.A.F. => USAF\n"
			"life:) => life:)\n"
			"; => ;\n"
			);
	if ( sMagic )
		fprintf ( fp, "%s => test\n", sMagic );
	fclose ( fp );
	return true;
}

class Environment : public ::testing::Environment
{
public:
	// Override this to define how to set up the environment.
	void SetUp () override
	{
		char cTopOfMainStack;
		sphThreadInit ();
		MemorizeStack ( &cTopOfMainStack );
		CreateSynonymsFile ();
		CreateSynonymsFile ( g_sMagic );
	}

	// Override this to define how to tear down the environment.
	void TearDown () override
	{
		unlink ( g_sTmpfile );
		unlink ( g_sMagickTmpfile );
	}
};

// it will create 2 synonyms file before all tests (globally), and delete them on finish.
::testing::Environment * const env = ::testing::AddGlobalTestEnvironment ( new Environment );
