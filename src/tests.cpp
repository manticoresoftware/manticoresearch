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


void TestUTF8Tokenizer ()
{
	ISphTokenizer * pTokenizer = sphCreateUTF8Tokenizer ();

	CSphString sError;
	//	assert ( pTokenizer->LoadSynonyms ( "syn.txt", sError ) );
	assert ( pTokenizer->SetCaseFolding ( "-, 0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F", sError ) );
	pTokenizer->SetMinWordLen ( 2 );
	pTokenizer->AddSpecials ( "!-" );

	// simple "one-line" tests
	const char * dTests[] =
	{
		"this is my rifle",	"this", "is", "my", "rifle", NULL,	// test that tokenizing works
		"This is MY rifle",	"this", "is", "my", "rifle", NULL,	// test that folding works
		"i-phone",			"i-phone", NULL,					// test that duals (specials in the middle of the word) work ok
		"i phone",			"phone", NULL,						// test that short words are skipped
		"this is m",		"this", "is", NULL,					// test that short words at the end are skipped
		"the -phone",		"the", "-", "phone", NULL,			// test that specials work
		"the!phone",		"the", "!", "phone", NULL,			// test that specials work
		"i!phone",			"!", "phone", NULL,					// test that short words preceding specials are skipped
		NULL
	};

	int iCur = 0;
	while ( dTests[iCur] )
	{
		printf ( "testing tokenizer, line=%s\n", dTests[iCur] );
		pTokenizer->SetBuffer ( (BYTE*)dTests[iCur], strlen(dTests[iCur]), true );
		iCur++;

		for ( BYTE * pToken=pTokenizer->GetToken(); pToken; pToken=pTokenizer->GetToken() )
			assert ( strcmp ( (const char*)pToken, dTests[iCur++] )==0 );

		assert ( dTests[iCur++]==NULL );
	}

	// test that short words at buffer boundary are accumulated
	printf ( "testing tokenizer for short words at buffer boundary\n" );
	char * sLine1 = "this is m";
	char * sLine2 = "y rifle";

	pTokenizer->SetBuffer ( (BYTE*)sLine1, strlen(sLine1), false );
	assert ( !strcmp ( (char*)pTokenizer->GetToken(), "this" ) );
	assert ( !strcmp ( (char*)pTokenizer->GetToken(), "is" ) );
	assert ( pTokenizer->GetToken()==NULL );

	pTokenizer->SetBuffer ( (BYTE*)sLine2, strlen(sLine2), true );
	assert ( !strcmp ( (char*)pTokenizer->GetToken(), "my" ) );
	assert ( !strcmp ( (char*)pTokenizer->GetToken(), "rifle" ) );
	assert ( pTokenizer->GetToken()==NULL );

	// test that decoder does not go over the buffer boundary on errors in UTF-8
	printf ( "testing tokenizer for proper UTF-8 error handling\n" );
	char * sLine3 = "hi\xd0\xffh";

	pTokenizer->SetBuffer ( (BYTE*)sLine3, 4, true );
	assert ( !strcmp ( (char*)pTokenizer->GetToken(), "hi" ) );
}


void main ()
{
	printf ( "RUNNING INTERNAL LIBSPHINX TESTS\n\n" );

	TestUTF8Tokenizer ();

	// done
	printf ( "ALL TESTS SUCCESFUL\n" );
	exit ( 0 );
}
