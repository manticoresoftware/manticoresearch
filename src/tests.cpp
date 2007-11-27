//
// $Id$
//

//
// Copyright (c) 2001-2007, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"


const char * g_sTmpfile = "__libsphinxtest.tmp";


bool CreateSynonymsFile ()
{
	FILE * fp = fopen ( g_sTmpfile, "w+" );
	if ( !fp )
		return false;

	fprintf ( fp,
		"AT&T      => AT&T\n"
		"   AT & T => AT & T  \n"
		"standarten fuehrer => Standartenfuehrer\n"
		"standarten fuhrer  => Standartenfuehrer\n"
		"OS/2 => OS/2\n" 
		"Ms-Dos => MS-DOS\n"
		);
	fclose ( fp );
	return true;
}


ISphTokenizer * CreateTestTokenizer ( bool bSynonyms )
{
	CSphString sError;
	ISphTokenizer * pTokenizer = sphCreateUTF8Tokenizer ();
	assert ( pTokenizer->SetCaseFolding ( "-, 0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F", sError ) );
	pTokenizer->SetMinWordLen ( 2 );
	pTokenizer->AddSpecials ( "!-" );
	if ( bSynonyms )
		assert ( pTokenizer->LoadSynonyms ( g_sTmpfile, sError ) );
	return pTokenizer;
}


void TestUTF8Tokenizer ()
{
	assert ( CreateSynonymsFile () );
	for ( int iRun=1; iRun<=2; iRun++ )
	{
		ISphTokenizer * pTokenizer = CreateTestTokenizer ( iRun==2 );

		// simple "one-line" tests
		char * dTests[] =
		{
			"1", "",							NULL,								// test that empty strings work
			"1", "this is my rifle",			"this", "is", "my", "rifle", NULL,	// test that tokenizing works
			"1", "This is MY rifle",			"this", "is", "my", "rifle", NULL,	// test that folding works
			"1", "i-phone",						"i-phone", NULL,					// test that duals (specials in the middle of the word) work ok
			"1", "i phone",						"phone", NULL,						// test that short words are skipped
			"1", "this is m",					"this", "is", NULL,					// test that short words at the end are skipped
			"1", "the -phone",					"the", "-", "phone", NULL,			// test that specials work
			"1", "the!phone",					"the", "!", "phone", NULL,			// test that specials work
			"1", "i!phone",						"!", "phone", NULL,					// test that short words preceding specials are skipped
			"1", "/-hi",						"-", "hi", NULL,					// test that synonym-dual but folded-special chars work ok
			"2", "AT&T",						"AT&T", NULL,						// test that synonyms work
			"2", "AT & T",						"AT & T", NULL,						// test that synonyms with spaces work
			"2", "AT    &  T",					"AT & T", NULL,						// test that synonyms with continuous spaces work
			"2", "-AT&T",						"-", "AT&T", NULL,					// test that synonyms with specials work
			"2", "AT&",							"at", NULL,							// test that synonyms prefixes are not lost on eof
			"2", "AT&tee.yo",					"at", "tee", "yo", NULL,			// test that non-synonyms with partially matching prefixes work
			"2", "standarten fuehrer",			"Standartenfuehrer", NULL,
			"2", "standarten fuhrer",			"Standartenfuehrer", NULL,
			"2", "standarten fuehrerr",			"standarten", "fuehrerr", NULL,
			"2", "standarten fuehrer Stirlitz",	"Standartenfuehrer", "stirlitz", NULL,
			"2", "OS/2 vs OS/360 vs Ms-Dos",	"OS/2", "vs", "os", "360", "vs", "MS-DOS", NULL,
			"2", "AT ",							"at", NULL,							// test that prefix-whitespace-eof combo does not hang
			"2", "AT&T&TT",						"at", "tt", NULL,
			"2", "http://OS/2",					"http", "OS/2", NULL,
			"2", "AT*&*T",						"at", NULL,
			"2", "# OS/2's system install",		"OS/2", "system", "install", NULL,
			0
		};

		int iCur = 0;
		while ( dTests[iCur] )
		{
			if ( atoi(dTests[iCur++])>iRun )
				break;

			printf ( "testing tokenizer, run=%d, line=%s\n", iRun, dTests[iCur] );
			pTokenizer->SetBuffer ( (BYTE*)dTests[iCur], strlen(dTests[iCur]) );
			iCur++;

			for ( BYTE * pToken=pTokenizer->GetToken(); pToken; pToken=pTokenizer->GetToken() )
			{
				assert ( dTests[iCur] && strcmp ( (const char*)pToken, dTests[iCur] )==0 );
				iCur++;
			}

			assert ( dTests[iCur]==NULL );
			iCur++;
		}

		// test that decoder does not go over the buffer boundary on errors in UTF-8
		printf ( "testing tokenizer for proper UTF-8 error handling\n" );
		char * sLine3 = "hi\xd0\xffh";

		pTokenizer->SetBuffer ( (BYTE*)sLine3, 4 );
		assert ( !strcmp ( (char*)pTokenizer->GetToken(), "hi" ) );

		// test uberlong tokens
		printf ( "testing tokenizer for uberlong token handling\n" );

		const int UBERLONG = 4096;
		char * sLine4 = new char [ UBERLONG+1 ]; 
		memset ( sLine4, 'a', UBERLONG );
		sLine4[UBERLONG] = '\0';

		char sTok4[SPH_MAX_WORD_LEN+1];
		memset ( sTok4, 'a', SPH_MAX_WORD_LEN );
		sTok4[SPH_MAX_WORD_LEN] = '\0';

		pTokenizer->SetBuffer ( (BYTE*)sLine4, strlen(sLine4) );
		assert ( !strcmp ( (char*)pTokenizer->GetToken(), sTok4 ) );
		assert ( pTokenizer->GetToken()==NULL );

		// test uberlong synonym-only tokens
		if ( iRun==2 )
		{
			printf ( "testing tokenizer for uberlong synonym-only char token handling\n" );

			memset ( sLine4, '/', UBERLONG );
			sLine4[UBERLONG] = '\0';

			pTokenizer->SetBuffer ( (BYTE*)sLine4, strlen(sLine4) );
			assert ( pTokenizer->GetToken()==NULL );

			printf ( "testing tokenizer for uberlong synonym token handling\n" );

			for ( int i=0; i<UBERLONG-3; i+=3 )
			{
				sLine4[i+0] = 'a';
				sLine4[i+1] = 'a';
				sLine4[i+2] = '/';
				sLine4[i+3] = '\0';
			}

			pTokenizer->SetBuffer ( (BYTE*)sLine4, strlen(sLine4) );
			for ( int i=0; i<UBERLONG-3; i+=3 )
				assert ( !strcmp ( (char*)pTokenizer->GetToken(), "aa" ) );
			assert ( pTokenizer->GetToken()==NULL );
		}

		SafeDelete ( sLine4 );

		// test boundaries 
		printf ( "testing tokenizer for boundaries handling, run=%d\n", iRun );

		CSphString sError;
		assert  ( pTokenizer->SetBoundary ( "., ?", sError ) );

		char sLine5[] = "hello world. testing boundaries.";
		pTokenizer->SetBuffer ( (BYTE*)sLine5, strlen(sLine5) );

		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "hello" ) ); assert ( !pTokenizer->GetBoundary() );
		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "world" ) ); assert ( !pTokenizer->GetBoundary() );
		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "testing" ) ); assert ( pTokenizer->GetBoundary() );
		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "boundaries" ) ); assert ( !pTokenizer->GetBoundary() );

		// done
		SafeDelete ( pTokenizer );
	}
}


void BenchUTF8Tokenizer ()
{
	printf ( "benchmarking UTF-8 tokenizer\n" );
	if ( !CreateSynonymsFile() )
	{
		printf ( "benchmark failed: error writing temp synonyms file\n" );
		return;
	}


	const char * sTestfile = "./configure";
	for ( int iRun=1; iRun<=2; iRun++ )
	{
		FILE * fp = fopen ( sTestfile, "rb" );
		if ( !fp )
		{
			printf ( "benchmark failed: error opening %s\n", sTestfile );
			return;
		}
		const int MAX_DATA = 10485760;
		char * sData = new char [ MAX_DATA ];
		int iData = fread ( sData, 1, MAX_DATA, fp );
		fclose ( fp );
		if ( iData<=0 )
		{
			printf ( "benchmark failed: error reading %s\n", sTestfile );
			SafeDeleteArray ( sData );
			return;
		}

		CSphString sError;
		ISphTokenizer * pTokenizer = sphCreateUTF8Tokenizer ();
		pTokenizer->SetCaseFolding ( "-, 0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F", sError );
		if ( iRun==2 )
			pTokenizer->LoadSynonyms ( g_sTmpfile, sError );
		pTokenizer->AddSpecials ( "!-" );

		const int iPasses = 50;
		int iTokens = 0;

		float fTime = -sphLongTimer ();
		for ( int iPass=0; iPass<iPasses; iPass++ )
		{
			pTokenizer->SetBuffer ( (BYTE*)sData, iData );
			while ( pTokenizer->GetToken() ) iTokens++;
		}
		fTime += sphLongTimer ();

		iTokens /= iPasses;
		fTime /= iPasses;

		printf ( "run %d: %d bytes, %d tokens, %.3f ms, %.3f MB/sec\n", iRun, iData, iTokens, 1000.0f*fTime, iData/fTime/1000000.0f );
		SafeDeleteArray ( sData );
	}
}


void main ()
{
	printf ( "RUNNING INTERNAL LIBSPHINX TESTS\n\n" );

#ifdef NDEBUG
	BenchUTF8Tokenizer ();
#else
	TestUTF8Tokenizer ();
#endif

	unlink ( g_sTmpfile );
	printf ( "\nSUCCESS\n" );
	exit ( 0 );
}

//
// $Id$
//
