//
// $Id$
//

//
// Copyright (c) 2001-2013, Andrew Aksyonoff
// Copyright (c) 2008-2013, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxexpr.h"
#include "sphinxutils.h"
#include "sphinxquery.h"
#include "sphinxrt.h"
#include "sphinxint.h"
#include "sphinxstem.h"
#include <math.h>

#define SNOWBALL 0
#define CROSSCHECK 0
#define PORTER1 0

#if SNOWBALL
#include "header.h"
#include "api.c"
#include "utilities.c"
#include "stem.c"
#endif

#if PORTER1
#include "porter1.c"
#endif

//////////////////////////////////////////////////////////////////////////

static const char * g_sTmpfile = "__libsphinxtest.tmp";

//////////////////////////////////////////////////////////////////////////

bool CreateSynonymsFile ( const char * sMagic )
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
		"MS DOS => MS-DOS\n"
		"feat. => featuring\n"
		"U.S. => US\n"
		"U.S.A. => USA\n"
		"U.S.B. => USB\n"
		"U.S.D. => USD\n"
		"U.S.P. => USP\n"
		"U.S.A.F. => USAF\n"
		);
	if ( sMagic )
		fprintf ( fp, "%s => test\n", sMagic );
	fclose ( fp );
	return true;
}


const DWORD TOK_EXCEPTIONS		= 1;
const DWORD TOK_NO_DASH			= 2;

ISphTokenizer * CreateTestTokenizer ( bool bUTF8, DWORD uMode )
{
	CSphString sError;
	CSphTokenizerSettings tSettings;
	tSettings.m_iType = bUTF8 ? TOKENIZER_UTF8 : TOKENIZER_SBCS;
	tSettings.m_iMinWordLen = 2;

	ISphTokenizer * pTokenizer = ISphTokenizer::Create ( tSettings, NULL, sError );
	if (!( uMode & TOK_NO_DASH ))
	{
		assert ( pTokenizer->SetCaseFolding ( "-, 0..9, A..Z->a..z, _, a..z, U+80..U+FF", sError ) );
		pTokenizer->AddSpecials ( "!-" );
	} else
	{
		assert ( pTokenizer->SetCaseFolding ( "0..9, A..Z->a..z, _, a..z, U+80..U+FF", sError ) );
		pTokenizer->AddSpecials ( "!" );
	}
	if ( uMode & TOK_EXCEPTIONS )
		assert ( pTokenizer->LoadSynonyms ( g_sTmpfile, NULL, sError ) );

	// tricky little shit!
	// we want to create a query mode tokenizer
	// the official way is to Clone() an indexing mode one, so we do that
	// however, Clone() adds backslash as a special
	// and that must be done *after* SetCaseFolding, otherwise it's not special any more
	ISphTokenizer * pTokenizer1 = pTokenizer->Clone ( SPH_CLONE_QUERY );
	SafeDelete ( pTokenizer );

	return pTokenizer1;
}


void TestTokenizer ( bool bUTF8 )
{
	const char * sPrefix = bUTF8
		? "testing UTF8 tokenizer"
		: "testing SBCS tokenizer";

	for ( int iRun=1; iRun<=3; iRun++ )
	{
		// simple "one-line" tests
		const char * sMagic = bUTF8
			? "\xD1\x82\xD0\xB5\xD1\x81\xD1\x82\xD1\x82\xD1\x82" // valid UTF-8
			: "\xC0\xC1\xF5\xF6"; // valid SBCS but invalid UTF-8

		assert ( CreateSynonymsFile ( sMagic ) );
		bool bExceptions = ( iRun>=2 );
		ISphTokenizer * pTokenizer = CreateTestTokenizer ( bUTF8, bExceptions*TOK_EXCEPTIONS );

		const char * dTests[] =
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
			"2", "AT&T&TT",						"AT&T", "tt", NULL,
			"2", "http://OS/2",					"http", "OS/2", NULL,
			"2", "AT*&*T",						"at", NULL,
			"2", "# OS/2's system install",		"OS/2", "system", "install", NULL,
			"2", "IBM-s/OS/2/Merlin",			"ibm-s", "OS/2", "merlin", NULL,
			"2", "U.S.A",						"US", NULL,
			"2", "AT&T!",						"AT&T", "!", NULL,					// exceptions vs specials
			"2", "AT&T!!!",						"AT&T", "!", "!", "!", NULL,		// exceptions vs specials
			"2", "U.S.A.!",						"USA", "!", NULL,					// exceptions vs specials
			"2", "MS DOSS feat.Deskview.MS DOS",			"ms", "doss", "featuring", "deskview", "MS-DOS", NULL,
			"2", sMagic,									"test", NULL,
			"2", "U.S. U.S.A. U.S.A.F.",					"US", "USA", "USAF", NULL,
			"2", "U.S.AB U.S.A. U.S.B.U.S.D.U.S.U.S.A.F.",	"US", "ab", "USA", "USB", "USD", "US", "USAF", NULL,
			"3", "phon\\e",						"phone", NULL,
			"3", "\\thephone",					"thephone", NULL,
			"3", "the\\!phone",					"the", "phone", NULL,
			"3", "\\!phone",					"phone", NULL,
			"3", "\\\\phone",					"phone", NULL,						// the correct behavior if '\' is not in charset
			"3", "pho\\\\ne",					"pho", "ne", NULL,
			"3", "phon\\\\e",					"phon", NULL,
			"3", "trailing\\",					"trailing", NULL,
			NULL
		};

		for ( int iCur=0; dTests[iCur] && atoi ( dTests[iCur++] )<=iRun; )
		{
			printf ( "%s, run=%d, line=%s\n", sPrefix, iRun, dTests[iCur] );
			pTokenizer->SetBuffer ( (BYTE*)dTests[iCur], strlen ( dTests[iCur] ) );
			iCur++;

			for ( BYTE * pToken=pTokenizer->GetToken(); pToken; pToken=pTokenizer->GetToken() )
			{
				assert ( dTests[iCur] && strcmp ( (const char*)pToken, dTests[iCur] )==0 );
				iCur++;
			}

			assert ( dTests[iCur]==NULL );
			iCur++;
		}

		// test misc SBCS-only and UTF8-only one-liners
		const char * dTests2[] =
		{
			"0", "\x80\x81\x82",				"\x80\x81\x82", NULL,
			"1", "\xC2\x80\xC2\x81\xC2\x82",	"\xC2\x80\xC2\x81\xC2\x82", NULL,
			NULL
		};

		for ( int iCur=0; dTests2[iCur] && atoi ( dTests2[iCur++] )==int(bUTF8); )
		{
			printf ( "%s, run=%d, line=%s\n", sPrefix, iRun, dTests2[iCur] );
			pTokenizer->SetBuffer ( (BYTE*)dTests2[iCur], strlen ( dTests2[iCur] ) );
			iCur++;

			for ( BYTE * pToken=pTokenizer->GetToken(); pToken; pToken=pTokenizer->GetToken() )
			{
				assert ( dTests2[iCur] && strcmp ( (const char*)pToken, dTests2[iCur] )==0 );
				iCur++;
			}

			assert ( dTests2[iCur]==NULL );
			iCur++;
		}


		// test that decoder does not go over the buffer boundary on errors in UTF-8
		if ( bUTF8 )
		{
			printf ( "%s for proper UTF-8 error handling\n", sPrefix );
			const char * sLine3 = "hi\xd0\xffh";

			pTokenizer->SetBuffer ( (BYTE*)sLine3, 4 );
			assert ( !strcmp ( (char*)pTokenizer->GetToken(), "hi" ) );
		}

		// test uberlong tokens
		printf ( "%s for uberlong token handling\n", sPrefix );

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

		// test short word callbacks
		printf ( "%s for short token handling\n", sPrefix );
		ISphTokenizer * pShortTokenizer = pTokenizer->Clone ( SPH_CLONE_QUERY );
		pShortTokenizer->AddPlainChar ( '*' );

		CSphTokenizerSettings tSettings = pShortTokenizer->GetSettings();
		tSettings.m_iMinWordLen = 5;
		pShortTokenizer->Setup ( tSettings );

		const char * dTestsShort[] =
		{
			"ab*",		"ab*",		NULL,
			"*ab",		"*ab",		NULL,
			"abcdef",	"abcdef",	NULL,
			"ab *ab* abc", "*ab*",	NULL,
			NULL
		};

		for ( int iCur=0; dTestsShort[iCur]; )
		{
			pShortTokenizer->SetBuffer ( (BYTE*)(dTestsShort [iCur]), strlen ( (const char*)dTestsShort [iCur] ) );
			iCur++;
			for ( BYTE * pToken=pShortTokenizer->GetToken(); pToken; pToken=pShortTokenizer->GetToken() )
			{
				assert ( dTestsShort[iCur] && strcmp ( (const char*)pToken, dTestsShort[iCur] )==0 );
				iCur++;
			}

			assert ( !dTestsShort [iCur] );
			iCur++;
		}

		SafeDelete ( pShortTokenizer );

		// test uberlong synonym-only tokens
		if ( iRun==2 )
		{
			printf ( "%s for uberlong synonym-only char token handling\n", sPrefix );

			memset ( sLine4, '/', UBERLONG );
			sLine4[UBERLONG] = '\0';

			pTokenizer->SetBuffer ( (BYTE*)sLine4, strlen(sLine4) );
			assert ( pTokenizer->GetToken()==NULL );

			printf ( "%s for uberlong synonym token handling\n", sPrefix );

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

		SafeDeleteArray ( sLine4 );

		// test boundaries
		printf ( "%s for boundaries handling, run=%d\n", sPrefix, iRun );

		CSphString sError;
		assert ( pTokenizer->SetBoundary ( "?", sError ) );

		char sLine5[] = "hello world? testing boundaries?";
		pTokenizer->SetBuffer ( (BYTE*)sLine5, strlen(sLine5) );

		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "hello" ) ); assert ( !pTokenizer->GetBoundary() );
		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "world" ) ); assert ( !pTokenizer->GetBoundary() );
		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "testing" ) ); assert ( pTokenizer->GetBoundary() );
		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "boundaries" ) ); assert ( !pTokenizer->GetBoundary() );

		// test specials vs token start/end ptrs
		printf ( "%s vs specials vs token start/end ptrs\n", sPrefix );

		char sLine6[] = "abc!def";
		pTokenizer->SetBuffer ( (BYTE*)sLine6, strlen(sLine6) );

		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "abc" ) );
		assert ( *pTokenizer->GetTokenStart()=='a' );
		assert ( *pTokenizer->GetTokenEnd()=='!' );

		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "!" ) );
		assert ( *pTokenizer->GetTokenStart()=='!' );
		assert ( *pTokenizer->GetTokenEnd()=='d' );

		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "def" ) );
		assert ( *pTokenizer->GetTokenStart()=='d' );
		assert ( *pTokenizer->GetTokenEnd()=='\0' );

		// test embedded zeroes
		printf ( "%s vs embedded zeroes\n", sPrefix );

		char sLine7[] = "abc\0\0\0defgh";
		pTokenizer->SetBuffer ( (BYTE*)sLine7, 9 );

		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "abc" ) );
		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "def" ) );
		assert ( !pTokenizer->GetToken() );
		assert ( !pTokenizer->GetToken() );
		assert ( !pTokenizer->GetToken() );

		// done
		SafeDelete ( pTokenizer );
	}

	// test blended
	printf ( "%s vs escaping vs blend_chars edge cases\n", sPrefix );

	CSphString sError;
	ISphTokenizer * pTokenizer = CreateTestTokenizer ( bUTF8, 0 );
	assert ( pTokenizer->SetBlendChars ( "., @", sError ) );
	pTokenizer->AddSpecials ( "()!-\"@" );

	char sTest1[] = "(texas.\\\")";
	pTokenizer->SetBuffer ( (BYTE*)sTest1, strlen(sTest1) );

	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "(" ) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "texas." ) );
	assert ( pTokenizer->TokenIsBlended() );
	pTokenizer->SkipBlended ();
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), ")" ) );
	assert ( pTokenizer->GetToken()==NULL );

	char sTest2[] = "\"series 2003\\-\\\"\"";
	printf ( "test %s\n", sTest2 );
	pTokenizer->SetBuffer ( (BYTE*)sTest2, strlen(sTest2) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "\"" ) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "series" ) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "2003-" ) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "\"" ) );
	assert ( pTokenizer->GetToken()==NULL );

	char sTest3[] = "aa lock.up bb";
	printf ( "test %s\n", sTest3 );
	pTokenizer->SetBuffer ( (BYTE*)sTest3, strlen(sTest3) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "aa" ) );
	assert ( !pTokenizer->TokenIsBlended() );
	assert ( !pTokenizer->TokenIsBlendedPart() );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "lock.up" ) );
	assert ( pTokenizer->TokenIsBlended() );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "lock" ) );
	assert ( !pTokenizer->TokenIsBlended() );
	assert ( pTokenizer->TokenIsBlendedPart() );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "up" ) );
	assert ( !pTokenizer->TokenIsBlended() );
	assert ( pTokenizer->TokenIsBlendedPart() );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "bb" ) );
	assert ( !pTokenizer->TokenIsBlended() );
	assert ( !pTokenizer->TokenIsBlendedPart() );

	char sTest4[] = "3.rd text";
	printf ( "test %s\n", sTest4 );
	pTokenizer->SetBuffer ( (BYTE*)sTest4, strlen(sTest4) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "3.rd" ) );
	assert ( pTokenizer->TokenIsBlended() );
	assert ( pTokenizer->SkipBlended()==1 );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "text" ) );
	assert ( !pTokenizer->TokenIsBlended() );
	assert ( !pTokenizer->GetToken() );

	char sTest5[] = "123\\@rd text";
	printf ( "test %s\n", sTest5 );
	pTokenizer->SetBuffer ( (BYTE*)sTest5, strlen(sTest5) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "123@rd" ) );
	assert ( pTokenizer->TokenIsBlended() );
	assert ( pTokenizer->SkipBlended()==2 );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "text" ) );
	assert ( !pTokenizer->TokenIsBlended() );
	assert ( !pTokenizer->GetToken() );

	char sTest6[] = "at.ta\\.c.da\\.bl.ok yo pest";
	printf ( "test %s\n", sTest6 );
	pTokenizer->SetBuffer ( (BYTE*)sTest6, strlen(sTest6) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "at.ta.c.da.bl.ok" ) );
	assert ( pTokenizer->TokenIsBlended() );
	assert ( pTokenizer->SkipBlended()==5 );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "yo" ) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "pest" ) );
	assert ( !pTokenizer->GetToken() );

	char sTest7[] = "3\\@rd text";
	printf ( "test %s\n", sTest7 );
	pTokenizer->SetBuffer ( (BYTE*)sTest7, strlen(sTest7) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "3@rd" ) );
	assert ( pTokenizer->TokenIsBlended() );
	assert ( pTokenizer->SkipBlended()==1 ); // because 3 is overshort!
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "text" ) );
	assert ( !pTokenizer->TokenIsBlended() );
	assert ( !pTokenizer->GetToken() );

	// blended/special vs query mode vs modifier.. hell, this is complicated
	SafeDelete ( pTokenizer );
	pTokenizer = CreateTestTokenizer ( bUTF8, TOK_NO_DASH );
	assert ( pTokenizer->SetBlendChars ( "., -", sError ) );
	pTokenizer->AddSpecials ( "-" );
	pTokenizer->AddPlainChar ( '=' );
	assert ( pTokenizer->SetBlendMode ( "trim_none, skip_pure", sError ) );

	char sTest10[] = "hello =- =world";
	printf ( "test %s\n", sTest10 );
	pTokenizer->SetBuffer ( (BYTE*)sTest10, strlen(sTest10) );

	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "hello" ) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "=world" ) );

	SafeDelete ( pTokenizer );

	printf ( "test utf8 len 1\n" );
	assert ( sphUTF8Len ( "ab\0cd", 256 )==2 );

	printf ( "test utf8 len 2\n" );
	assert ( sphUTF8Len ( "", 256 )==0 && sphUTF8Len ( NULL, 256 )==0 );

	printf ( "test noascii case\n" );
	pTokenizer = sphCreateUTF8Tokenizer();
	assert ( pTokenizer->SetCaseFolding ( "U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451", sError ) );
	char sTest20[] = "abc \xD0\xBE\xD0\xBF\xD0\xB0\x58\xD1\x87\xD0\xB0 def";
	pTokenizer->SetBuffer ( (BYTE*)sTest20, strlen(sTest20) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "\xD0\xBE\xD0\xBF\xD0\xB0" ) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "\xD1\x87\xD0\xB0" ) );
	assert ( !pTokenizer->GetToken() );
	SafeDelete ( pTokenizer );
	printf ( "test utf8 4-bytes codepoint\n" );
	BYTE sTest21[] = "\xF4\x80\x80\x80\x32\x34\x20";
	BYTE sTest22[] = "\xEC\x97\xB0";
	BYTE sRes21[SPH_MAX_WORD_LEN];

	memset ( sRes21, 0, sizeof(sRes21) );
	const BYTE * pTest21 = sTest21;
	int iCode21 = sphUTF8Decode ( pTest21 );
	assert ( sphUTF8Encode ( sRes21, iCode21 )==4 );
	assert ( sTest21[0]==sRes21[0] && sTest21[1]==sRes21[1] && sTest21[2]==sRes21[2] && sTest21[3]==sRes21[3] );
	memset ( sRes21, 0, sizeof(sRes21) );
	BYTE * pRes21 = sRes21;
	SPH_UTF8_ENCODE ( pRes21, iCode21 );
	assert ( sTest21[0]==sRes21[0] && sTest21[1]==sRes21[1] && sTest21[2]==sRes21[2] && sTest21[3]==sRes21[3] );

	memset ( sRes21, 0, sizeof(sRes21) );
	pTest21 = sTest22;
	int iCode22 = sphUTF8Decode ( pTest21 );
	assert ( iCode22==0xC5F0 );
	assert ( sphUTF8Encode ( sRes21, iCode22 )==3 );
	assert ( memcmp ( sTest22, sRes21, sizeof(sTest22) )==0 );
	memset ( sRes21, 0, sizeof(sRes21) );
	pRes21 = sRes21;
	SPH_UTF8_ENCODE ( pRes21, iCode22 );
	assert ( memcmp ( sTest22, sRes21, sizeof(sTest22) )==0 );

	pTokenizer = sphCreateUTF8Tokenizer();
	pTokenizer->SetBuffer ( (BYTE*)sTest21, sizeof(sTest21) );
	assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "\xF4\x80\x80\x80\x32\x34" ) );
}


char * LoadFile ( const char * sName, int * pLen, bool bReportErrors )
{
	FILE * fp = fopen ( sName, "rb" );
	if ( !fp )
	{
		if ( bReportErrors )
			printf ( "benchmark failed: error opening %s\n", sName );
		return NULL;
	}
	const int MAX_DATA = 10485760;
	char * sData = new char [ MAX_DATA ];
	int iData = fread ( sData, 1, MAX_DATA, fp );
	fclose ( fp );
	if ( iData<=0 )
	{
		if ( bReportErrors )
			printf ( "benchmark failed: error reading %s\n", sName );
		SafeDeleteArray ( sData );
		return NULL;
	}
	*pLen = iData;
	return sData;
}


void BenchTokenizer ( bool bUTF8 )
{
	printf ( "benchmarking %s tokenizer\n", bUTF8 ? "UTF8" : "SBCS" );
	if ( !CreateSynonymsFile ( NULL ) )
	{
		printf ( "benchmark failed: error writing temp synonyms file\n" );
		return;
	}

	CSphString sError;
	for ( int iRun=1; iRun<=2; iRun++ )
	{
		int iData = 0;
		char * sData = LoadFile ( "./configure", &iData, true );

		ISphTokenizer * pTokenizer = bUTF8 ? sphCreateUTF8Tokenizer () : sphCreateSBCSTokenizer ();
		pTokenizer->SetCaseFolding ( "-, 0..9, A..Z->a..z, _, a..z", sError );
		if ( iRun==2 )
			pTokenizer->LoadSynonyms ( g_sTmpfile, NULL, sError );
		pTokenizer->AddSpecials ( "!-" );

		const int iPasses = 200;
		int iTokens = 0;

		int64_t tmTime = -sphMicroTimer();
		for ( int iPass=0; iPass<iPasses; iPass++ )
		{
			pTokenizer->SetBuffer ( (BYTE*)sData, iData );
			while ( pTokenizer->GetToken() ) iTokens++;
		}
		tmTime += sphMicroTimer();

		iTokens /= iPasses;
		tmTime /= iPasses;

		printf ( "run %d: %d bytes, %d tokens, %d.%03d ms, %.3f MB/sec\n", iRun, iData, iTokens,
			(int)(tmTime/1000), (int)(tmTime%1000), float(iData)/tmTime );
		SafeDeleteArray ( sData );
	}

	if ( !bUTF8 )
		return;

	int iData;
	char * sData = LoadFile ( "./utf8.txt", &iData, false );
	if ( sData )
	{
		ISphTokenizer * pTokenizer = sphCreateUTF8Tokenizer ();

		const int iPasses = 200;
		int iTokens = 0;

		int64_t tmTime = -sphMicroTimer();
		for ( int iPass=0; iPass<iPasses; iPass++ )
		{
			pTokenizer->SetBuffer ( (BYTE*)sData, iData );
			while ( pTokenizer->GetToken() )
				iTokens++;
		}
		tmTime += sphMicroTimer();

		iTokens /= iPasses;
		tmTime /= iPasses;

		printf ( "run 3: %d bytes, %d tokens, %d.%03d ms, %.3f MB/sec\n", iData, iTokens,
			(int)(tmTime/1000), (int)(tmTime%1000), float(iData)/tmTime );
	}
	SafeDeleteArray ( sData );
}

//////////////////////////////////////////////////////////////////////////

void TestStripper ()
{
	const char * sTests[][4] =
	{
		// source-data, index-attrs, remove-elements, expected-results
		{ "<html>trivial test</html>", "", "", " trivial test " },
		{ "<html>lets <img src=\"g/smth.jpg\" alt=\"nice picture\">index attrs</html>", "img=alt", "", " lets nice picture index attrs " },
		{ "<html>   lets  also<script> whatever here; a<b</script>remove scripts", "", "script, style", " lets also remove scripts" },
		{ "testing in<b><font color='red'>line</font> ele<em>men</em>ts", "", "", "testing inline elements" },
		{ "testing non<p>inline</h1>elements", "", "", "testing non inline elements" },
		{ "testing&nbsp;entities&amp;stuff", "", "", "testing entities&stuff" },
		{ "testing &#1040;&#1041;&#1042; utf encoding", "", "", "testing \xD0\x90\xD0\x91\xD0\x92 utf encoding" },
		{ "testing <1 <\" <\x80 <\xe0 <\xff </3 malformed tags", "", "", "testing <1 <\" <\x80 <\xe0 <\xff </3 malformed tags" },
		{ "testing comm<!--comm-->ents", "", "", "testing comments" },
		{ "&lt; &gt; &thetasym; &somethingverylong; &the", "", "", "< > \xCF\x91 &somethingverylong; &the" },
		{ "testing <img src=\"g/smth.jpg\" alt=\"nice picture\" rel=anotherattr junk=throwaway>inline tags vs attr indexing", "img=alt,rel", "", "testing nice picture anotherattr inline tags vs attr indexing" },
		{ "this <?php $code = \"must be stripped\"; ?> away", "", "", "this away" },
		{ "<a href=\"http://www.com\">content1</a>", "a=title", "", "content1" },
		{ "<a href=\"http://www.com\" title=\"my test title\">content2</a>", "a=title", "", "my test title content2" },
		{ "testing <img src=\"g/smth.jpg\" alt=\"nice picture\" rel=anotherattr junk=\"throwaway\">inline tags vs attr indexing", "img=alt,rel", "", "testing nice picture anotherattr inline tags vs attr indexing" },
		{ "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"><html>test</html>", "", "", " test " },
		{ "<!smth \"that>can<break\"><html>test</html>", "", "", " test " },
		{ "<TABLE CLASS=\"MSONORMALTABLE\" STYLE=\"BORDER-COLLAPSE: COLLAPSE; MARGIN-LEFT: ID=\"TABLE76\"><TR><TD>ohai</TD></TR></TABLE>", "", "", " ohai " },
		{ "ohai2<table class", "", "", "ohai2 " },
		{ "ohai<table class>3", "", "", "ohai 3" },
		{ "ohai<table class >4", "", "", "ohai 4" },
		{ "ohai<table class =>5", "", "", "ohai 5" },
		{ "ohai<table class =\"smth><tr><td>6</td></tr></table> some more content", "", "", "ohai 6 some more content" },
		{ "ohai<table nowrap class=\"a>b\">7", "", "", "ohai 7" },
		{ "ohai<table nowrap class =\"a>b\">8", "", "", "ohai 8" },
		{ "ohai<table nowrap class= \"a>b\">9", "", "", "ohai 9" },
		{ "ohai<table now rap class=\"a>b\">10", "", "", "ohai 10" },
		{ "ohai<table class = \"smth><tr><td>6</td><td class=\"test\">11</td></tr></table> gimme more", "", "", "ohai 11 gimme more" },
		{ "<P ALIGN=\"LEFT STYLE=\"MARGIN:0IN 0IN .0001PT;TEXT-ALIGN:LEFT;\"><B><FONT SIZE=\"2\" FACE=\"TIMES NEW ROMAN\" STYLE=\"FONT-SIZE:10.0PT;FONT-WEIGHT:BOLD;\">Commission File Number: 333-155507", "", "", " Commission File Number: 333-155507" },
		{ "<TD NOWRAP ALIGN=RIGHT STYLE=\"BORDER-BOTTOM: #000000 1PX SOLID; BORDER-TOP: #000000 1PX SOLID;\"\"><B>SGX", "", "", " SGX" },
		{ "tango & cash", "", "", "tango & cash" },
		{ "<font CLASS=\"MSONORMALTABLE\" STYLE=\"BORDER-COLLAPSE: COLLAPSE; MARGIN-LEFT: ID=\"TABLE76\">ahoy\"mate", "font=zzz", "", "ahoy\"mate" },
		{ "ahoy<font class =>2", "font=zzz", "", "ahoy2" },
		{ "ahoy<font class =\"smth><b>3</b></font>there", "font=zzz", "", "ahoy3there" },
		{ "ahoy<font nowrap class=\"a>b\">4", "font=zzz", "", "ahoy4" },
		{ "ahoy<font now rap class=\"a>b\">5", "font=zzz", "", "ahoy5" },
		{ "ahoy<font class = \"smth><b><i>6</i><b class=\"test\">seven</b></i></font>eight", "font=zzz", "", "ahoyseveneight" }
	};

	int nTests = (int)(sizeof(sTests)/sizeof(sTests[0]));
	for ( int iTest=0; iTest<nTests; iTest++ )
	{
		printf ( "testing HTML stripper, test %d/%d... ", 1+iTest, nTests );

		CSphString sError;
		CSphHTMLStripper tStripper ( true );
		assert ( tStripper.SetIndexedAttrs ( sTests[iTest][1], sError ) );
		assert ( tStripper.SetRemovedElements ( sTests[iTest][2], sError ) );

		CSphString sBuf ( sTests[iTest][0] );
		tStripper.Strip ( (BYTE*)sBuf.cstr() );
		assert ( strcmp ( sBuf.cstr(), sTests[iTest][3] )==0 );

		printf ( "ok\n" );
	}
}

void BenchStripper ()
{
	printf ( "benchmarking HTML stripper\n" );

	FILE * fp = fopen ( "doc/sphinx.html", "rb" );
	if ( !fp )
	{
		printf ( "benchmark failed: unable to read doc/sphinx.html\n" );
		return;
	}

	const int MAX_SIZE = 1048576;
	char * sBuf = new char [ MAX_SIZE ];
	int iLen = fread ( sBuf, 1, MAX_SIZE-1, fp );
	fclose ( fp );

	char * sRef = new char [ MAX_SIZE ];
	sBuf[iLen] = '\0';
	memcpy ( sRef, sBuf, iLen+1 );

	for ( int iRun=0; iRun<2; iRun++ )
	{
		CSphString sError;
		CSphHTMLStripper tStripper ( true );
		if ( iRun==1 )
			tStripper.SetRemovedElements ( "style, script", sError );

		const int iPasses = 50;
		int64_t tmTime = -sphMicroTimer();
		for ( int iPass=0; iPass<iPasses; iPass++ )
		{
			tStripper.Strip ( (BYTE*)sBuf );
			memcpy ( sBuf, sRef, iLen+1 );
		}
		tmTime += sphMicroTimer();

		tmTime /= iPasses;
		printf ( "run %d: %d bytes, %d.%03d ms, %.3f MB/sec\n", iRun, iLen, (int)(tmTime/1000), (int)(tmTime%1000), float(iLen)/tmTime );
	}

	SafeDeleteArray ( sBuf );
	SafeDeleteArray ( sRef );
}

//////////////////////////////////////////////////////////////////////////

void TestExpr ()
{
	CSphColumnInfo tCol;
	tCol.m_eAttrType = SPH_ATTR_INTEGER;

	CSphSchema tSchema;
	tCol.m_sName = "aaa"; tSchema.AddAttr ( tCol, false );
	tCol.m_sName = "bbb"; tSchema.AddAttr ( tCol, false );
	tCol.m_sName = "ccc"; tSchema.AddAttr ( tCol, false );

	CSphRowitem * pRow = new CSphRowitem [ tSchema.GetRowSize() ];
	for ( int i=0; i<tSchema.GetRowSize(); i++ )
		pRow[i] = 1+i;

	CSphMatch tMatch;
	tMatch.m_iDocID = 123;
	tMatch.m_iWeight = 456;
	tMatch.m_pStatic = pRow;

	struct ExprTest_t
	{
		const char *	m_sExpr;
		float			m_fValue;
	};
	ExprTest_t dTests[] =
	{
		{ "ccc/2",							1.5f },
		{ "1*2*3*4*5*6*7*8*9*10",			3628800.0f },
		{ "aaa+bbb*sin(0)*ccc",				1.0f },
		{ "if(pow(sqrt(2),2)=2,123,456)",	123.0f },
		{ "if(2<2,3,4)",					4.0f },
		{ "if(2>=2,3,4)",					3.0f },
		{ "pow(7,5)",						16807.f },
		{ "sqrt(3)",						1.7320508f },
		{ "log2((2+2)*(2+2))",				4.0f },
		{ "min(3,15)",						3.0f },
		{ "max(3,15)",						15.0f },
		{ "if(3<15,bbb,ccc)",				2.0f },
		{ "@id+@weight",					579.0f },
		{ "abs(-3-ccc)",					6.0f },
		{ "(aaa+bbb)*(ccc-aaa)",			6.0f },
		{ "(((aaa)))",						1.0f },
		{ "aaa-bbb*ccc",					-5.0f },
		{ " aaa    -\tbbb *\t\t\tccc ",		-5.0f },
		{ "bbb+123*aaa",					125.0f },
		{ "2.000*2e+1+2",					42.0f },
		{ "3<5",							1.0f },
		{ "1 + 2*3 > 4*4",					0.0f },
		{ "aaa/-bbb",						-0.5f, },
		{ "-10*-10",						100.0f },
		{ "aaa+-bbb*-5",					11.0f },
		{ "-aaa>-bbb",						1.0f },
		{ "1-aaa+2-3+4",					3.0f },
		{ "bbb/1*2/6*3",					2.0f },
		{ "(aaa+bbb)/sqrt(3)/sqrt(3)",		1.0f },
		{ "aaa-bbb-2",						-3.0f },
		{ "ccc/2*4/bbb",					3.0f },
		{ "(2+(aaa*bbb))+3",				7.0f }
	};

	const int nTests = sizeof(dTests)/sizeof(dTests[0]);
	for ( int iTest=0; iTest<nTests; iTest++ )
	{
		printf ( "testing expression evaluation, test %d/%d... ", 1+iTest, nTests );

		CSphString sError;
		CSphScopedPtr<ISphExpr> pExpr ( sphExprParse ( dTests[iTest].m_sExpr, tSchema, NULL, NULL, sError, NULL ) );
		if ( !pExpr.Ptr() )
		{
			printf ( "FAILED; %s\n", sError.cstr() );
			assert ( 0 );
		}

		float fValue = pExpr->Eval(tMatch);
		if ( fabs ( fValue - dTests[iTest].m_fValue )>=0.0001f )
		{
			printf ( "FAILED; expected %.3f, got %.3f\n", dTests[iTest].m_fValue, fValue );
			assert ( 0 );
		}

		printf ( "ok\n" );
	}

	SafeDeleteArray ( pRow );
}


#if USE_WINDOWS
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif

#define AAA float(tMatch.m_pStatic[0])
#define BBB float(tMatch.m_pStatic[1])
#define CCC float(tMatch.m_pStatic[2])

NOINLINE float ExprNative1 ( const CSphMatch & tMatch )	{ return AAA+BBB*CCC-1.0f;}
NOINLINE float ExprNative2 ( const CSphMatch & tMatch )	{ return AAA+BBB*CCC*2.0f-3.0f/4.0f*5.0f/6.0f*BBB; }
NOINLINE float ExprNative3 ( const CSphMatch & )		{ return (float)sqrt ( 2.0f ); }


void BenchExpr ()
{
	printf ( "benchmarking expressions\n" );

	CSphColumnInfo tCol;
	tCol.m_eAttrType = SPH_ATTR_INTEGER;

	CSphSchema tSchema;
	tCol.m_sName = "aaa"; tSchema.AddAttr ( tCol, false );
	tCol.m_sName = "bbb"; tSchema.AddAttr ( tCol, false );
	tCol.m_sName = "ccc"; tSchema.AddAttr ( tCol, false );

	CSphRowitem * pRow = new CSphRowitem [ tSchema.GetRowSize() ];
	for ( int i=0; i<tSchema.GetRowSize(); i++ )
		pRow[i] = 1+i;

	CSphMatch tMatch;
	tMatch.m_iDocID = 123;
	tMatch.m_iWeight = 456;
	tMatch.m_pStatic = pRow;

	struct ExprBench_t
	{
		const char *	m_sExpr;
		float			( *m_pFunc )( const CSphMatch & );
	};
	ExprBench_t dBench[] =
	{
		{ "aaa+bbb*(ccc)-1",				ExprNative1 },
		{ "aaa+bbb*ccc*2-3/4*5/6*bbb",		ExprNative2 },
		{ "sqrt(2)",						ExprNative3 }
	};

	for ( int iRun=0; iRun<int(sizeof(dBench)/sizeof(dBench[0])); iRun++ )
	{
		printf ( "run %d: ", iRun+1 );

		ESphAttr uType;
		CSphString sError;
		CSphScopedPtr<ISphExpr> pExpr ( sphExprParse ( dBench[iRun].m_sExpr, tSchema, &uType, NULL, sError, NULL ) );
		if ( !pExpr.Ptr() )
		{
			printf ( "FAILED; %s\n", sError.cstr() );
			return;
		}

		const int NRUNS = 1000000;

		volatile float fValue = 0.0f;
		int64_t tmTime = sphMicroTimer();
		for ( int i=0; i<NRUNS; i++ ) fValue += pExpr->Eval(tMatch);
		tmTime = sphMicroTimer() - tmTime;

		int64_t tmTimeInt = sphMicroTimer();
		if ( uType==SPH_ATTR_INTEGER )
		{
			int uValue = 0;
			for ( int i=0; i<NRUNS; i++ ) uValue += pExpr->IntEval(tMatch);
		}
		tmTimeInt = sphMicroTimer() - tmTimeInt;

		int64_t tmTimeNative = sphMicroTimer();
		for ( int i=0; i<NRUNS; i++ ) fValue += dBench[iRun].m_pFunc ( tMatch );
		tmTimeNative = sphMicroTimer() - tmTimeNative;

		if ( uType==SPH_ATTR_INTEGER )
			printf ( "int-eval %.1fM/sec, ", float(NRUNS)/tmTimeInt );

		printf ( "flt-eval %.1fM/sec, native %.1fM/sec\n",
			float(NRUNS)/tmTime,
			float(NRUNS)/tmTimeNative );
	}

	SafeDeleteArray ( pRow );
}

//////////////////////////////////////////////////////////////////////////


void TestQueryParser ()
{
	CSphString sError;

	CSphSchema tSchema;
	CSphColumnInfo tCol;
	tCol.m_sName = "title"; tSchema.m_dFields.Add ( tCol );
	tCol.m_sName = "body"; tSchema.m_dFields.Add ( tCol );

	CSphScopedPtr<ISphTokenizer> pBase ( sphCreateSBCSTokenizer () );
	CSphTokenizerSettings tTokenizerSetup;
	tTokenizerSetup.m_iMinWordLen = 2;
	tTokenizerSetup.m_sSynonymsFile = g_sTmpfile;
	pBase->Setup ( tTokenizerSetup );
	assert ( CreateSynonymsFile ( NULL ) );
	assert ( pBase->LoadSynonyms ( g_sTmpfile, NULL, sError ) );

	CSphScopedPtr<ISphTokenizer> pTokenizer ( pBase->Clone ( SPH_CLONE_QUERY ) );
	sphSetupQueryTokenizer ( pTokenizer.Ptr() );

	CSphDictSettings tDictSettings;
	tDictSettings.m_bWordDict = false;
	CSphScopedPtr<CSphDict> pDict ( sphCreateDictionaryCRC ( tDictSettings, NULL, pTokenizer.Ptr(), "query", sError ) );

	assert ( pTokenizer.Ptr() );
	assert ( pDict.Ptr() );

	struct QueryTest_t
	{
		const char *	m_sQuery;
		const char *	m_sReconst;
	};
	const QueryTest_t dTest[] =
	{
		{ "aaa bbb ccc",					"( aaa   bbb   ccc )" },
		{ "aaa|bbb ccc",					"( ( aaa | bbb )   ccc )" },
		{ "aaa bbb|ccc",					"( aaa   ( bbb | ccc ) )" },
		{ "aaa (bbb ccc)|ddd",				"( aaa   ( ( bbb   ccc ) | ddd ) )" },
		{ "aaa bbb|(ccc ddd)",				"( aaa   ( bbb | ( ccc   ddd ) ) )" },
		{ "aaa bbb|(ccc ddd)|eee|(fff)",	"( aaa   ( bbb | ( ccc   ddd ) | eee | fff ) )" },
		{ "aaa bbb|(ccc ddd) eee|(fff)",	"( aaa   ( bbb | ( ccc   ddd ) )   ( eee | fff ) )" },
		{ "aaa (ccc ddd)|bbb|eee|(fff)",	"( aaa   ( ( ccc   ddd ) | bbb | eee | fff ) )" },
		{ "aaa (ccc ddd)|bbb eee|(fff)",	"( aaa   ( ( ccc   ddd ) | bbb )   ( eee | fff ) )" },
		{ "aaa \"bbb ccc\"~5|ddd",			"( aaa   ( \"bbb ccc\"~5 | ddd ) )" },
		{ "aaa bbb|\"ccc ddd\"~5",			"( aaa   ( bbb | \"ccc ddd\"~5 ) )" },
		{ "aaa ( ( \"bbb ccc\"~3|ddd ) eee | ( fff -ggg ) )",	"( aaa   ( ( \"bbb ccc\"~3 | ddd )   ( eee | ( fff AND NOT ggg ) ) ) )" },
		{ "@title aaa @body ccc|(@title ddd eee)|fff ggg",		"( ( @title: aaa )   ( ( @body: ccc ) | ( ( @title: ddd )   ( @title: eee ) ) | ( @body: fff ) )   ( @body: ggg ) )" },
		{ "@title hello world | @body sample program",			"( ( @title: hello )   ( ( @title: world ) | ( @body: sample ) )   ( @body: program ) )" },
		{ "@title one two three four",							"( ( @title: one )   ( @title: two )   ( @title: three )   ( @title: four ) )" },
		{ "@title one (@body two three) four",					"( ( @title: one )   ( ( @body: two )   ( @body: three ) )   ( @title: four ) )" },
		{ "windows 7 2000",										"( windows   2000 )" },
		{ "aaa a|bbb",											"( aaa   bbb )" },
		{ "aaa bbb|x y z|ccc",									"( aaa   bbb   ccc )" },
		{ "a",													"" },
		{ "hello -world",										"( hello AND NOT world )" },
		{ "-hello world",										"( world AND NOT hello )" },
		{ "\"phrase (query)/3 ~on steroids\"",					"\"phrase query on steroids\"" },
		{ "hello a world",										"( hello   world )" },
		{ "-one",												"" },
		{ "-one -two",											"" },
		{ "\"\"",												"" },
		{ "\"()\"",												"" },
		{ "\"]\"",												"" },
		{ "@title hello @body -world",							"( ( @title: hello ) AND NOT ( @body: world ) )" },
		{ "Ms-Dos",							"MS-DOS" }
	};


	CSphIndexSettings tTmpSettings;
	int nTests = sizeof(dTest)/sizeof(dTest[0]);
	for ( int i=0; i<nTests; i++ )
	{
		printf ( "testing query parser, test %d/%d... ", i+1, nTests );

		XQQuery_t tQuery;
		sphParseExtendedQuery ( tQuery, dTest[i].m_sQuery, pTokenizer.Ptr(), &tSchema, pDict.Ptr(), tTmpSettings );
		CSphString sReconst = sphReconstructNode ( tQuery.m_pRoot, &tSchema );
		assert ( sReconst==dTest[i].m_sReconst );

		printf ( "ok\n" );
	}
}

static CSphSourceStats g_tTmpDummyStat;
class CSphDummyIndex : public CSphIndex
{
public:
	CSphDummyIndex () : CSphIndex ( NULL, NULL ) {}
	virtual SphAttr_t *			GetKillList () const { return NULL; }
	virtual int					GetKillListSize () const { return 0 ; }
	virtual bool				HasDocid ( SphDocID_t ) const { return false; }
	virtual int					Build ( const CSphVector<CSphSource*> & , int , int ) { return 0; }
	virtual bool				Merge ( CSphIndex * , const CSphVector<CSphFilterSettings> & , bool ) {return false; }
	virtual bool				Prealloc ( bool , bool , CSphString & ) { return false; }
	virtual void				Dealloc () {}
	virtual bool				Preread () { return false; }
	virtual void				SetBase ( const char * ) {}
	virtual bool				Rename ( const char * ) { return false; }
	virtual bool				Lock () { return false; }
	virtual void				Unlock () {}
	virtual bool				Mlock () { return false; }
	virtual void				PostSetup() {}
	virtual bool				EarlyReject ( CSphQueryContext * , CSphMatch & ) const { return false; }
	virtual const CSphSourceStats &	GetStats () const { return g_tTmpDummyStat; }
	virtual CSphIndexStatus			GetStatus () const { CSphIndexStatus tRes; tRes.m_iRamUse = 0; return tRes; }
	virtual bool				MultiQuery ( const CSphQuery * , CSphQueryResult * , int , ISphMatchSorter ** , const CSphMultiQueryArgs & ) const { return false; }
	virtual bool				MultiQueryEx ( int , const CSphQuery * , CSphQueryResult ** , ISphMatchSorter ** , const CSphMultiQueryArgs & ) const { return false; }
	virtual bool				GetKeywords ( CSphVector <CSphKeywordInfo> & , const char * , bool , CSphString * ) const { return false; }
	virtual bool				FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const;
	virtual int					UpdateAttributes ( const CSphAttrUpdate & , int , CSphString &, CSphString & ) { return -1; }
	virtual bool				SaveAttributes ( CSphString & ) const { return false; }
	virtual DWORD				GetAttributeStatus () const { return 0; }
	virtual bool				CreateFilesWithAttr ( int, const CSphString &, ESphAttr, CSphString & ) { return true; }
	virtual bool				AddAttribute ( const CSphString &, ESphAttr, CSphString & ) { return true; }
	virtual void				DebugDumpHeader ( FILE *, const char *, bool ) {}
	virtual void				DebugDumpDocids ( FILE * ) {}
	virtual void				DebugDumpHitlist ( FILE * , const char * , bool ) {}
	virtual int					DebugCheck ( FILE * ) { return 0; } // NOLINT
	virtual void				DebugDumpDict ( FILE * ) {}
	virtual	void				SetProgressCallback ( CSphIndexProgress::IndexingProgress_fn ) {}

	SmallStringHash_T < int > m_hHits;
};


bool CSphDummyIndex::FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const
{
	ARRAY_FOREACH ( i, dKeywords )
	{
		int * pDocs = m_hHits ( dKeywords[i].m_sTokenized );
		dKeywords[i].m_iDocs = pDocs ? *pDocs : 0;
	}

	return true;
}


void TestQueryTransforms ()
{
	CSphSchema tSchema;
	CSphColumnInfo tCol;
	tCol.m_sName = "title"; tSchema.m_dFields.Add ( tCol );
	tCol.m_sName = "body"; tSchema.m_dFields.Add ( tCol );

	CSphString sError;
	CSphDictSettings tDictSettings;
	tDictSettings.m_bWordDict = false;
	CSphScopedPtr<ISphTokenizer> pBase ( sphCreateSBCSTokenizer () );
	CSphScopedPtr<CSphDict> pDict ( sphCreateDictionaryCRC ( tDictSettings, NULL, pBase.Ptr(), "query", sError ) );
	assert ( pBase.Ptr() );
	assert ( pDict.Ptr() );
	assert ( sError.IsEmpty() );

	CSphTokenizerSettings tTokenizerSetup;
	tTokenizerSetup.m_iMinWordLen = 2;
	tTokenizerSetup.m_sSynonymsFile = g_sTmpfile;
	pBase->Setup ( tTokenizerSetup );

	assert ( CreateSynonymsFile ( NULL ) );
	assert ( pBase->LoadSynonyms ( g_sTmpfile, NULL, sError ) );

	CSphScopedPtr<ISphTokenizer> pTokenizer ( pBase->Clone ( SPH_CLONE_QUERY ) );
	sphSetupQueryTokenizer ( pTokenizer.Ptr() );

	struct CKeywordHits {
		const char * 	m_sKeyword;
		int 			m_iHits;
	};

	struct QueryTest_t
	{
		const char *			m_sQuery;
		const char *			m_sReconst;
		const char *			m_sReconstTransformed;
		const CKeywordHits *	m_pKeywordHits;
	};

	const CKeywordHits dPseudoHits [][10] =
	{
		{ { "nnn", 10 }, { "aaa", 1 }, { "bbb", 1 }, { 0, 0 } },
		{ { "nnn", 10 }, { "aaa", 100 }, { "bbb", 200 }, { 0, 0 } },
		{ { "nnn", 10 }, { "aaa", 1 }, { "bbb", 2 }, { "qqq", 500 }, { "www", 100 }, { 0, 0 } }
	};

	const QueryTest_t dTest[] =
	{
		// COMMON NOT
		{
			"( aaa !ccc ) | ( bbb !ccc )",
			"( ( aaa AND NOT ccc ) | ( bbb AND NOT ccc ) )",
			"( ( aaa | bbb ) AND NOT ccc )",
			NULL
		},
		{
			"( aaa bbb !ccc) | ( ddd eee !ccc ) ",
			"( ( ( aaa   bbb ) AND NOT ccc ) | ( ( ddd   eee ) AND NOT ccc ) )",
			"( ( ( aaa   bbb ) | ( ddd   eee ) ) AND NOT ccc )",
			NULL
		},
		{
			"( aaa bbb !ccc) | ( ddd eee !ccc ) | fff | ( ggg !jjj )",
			"( ( ( aaa   bbb ) AND NOT ccc ) | ( ( ddd   eee ) AND NOT ccc ) | fff | ( ggg AND NOT jjj ) )",
			"( ( ( ( aaa   bbb ) | ( ddd   eee ) ) AND NOT ccc ) | fff | ( ggg AND NOT jjj ) )",
			NULL
		},
		{
			"(aaa !bbb) | (ccc !bbb) | (ccc !eee) | (ddd !eee)",
			"( ( aaa AND NOT bbb ) | ( ccc AND NOT bbb ) | ( ccc AND NOT eee ) | ( ddd AND NOT eee ) )",
			"( ( ( aaa | ccc ) AND NOT bbb ) | ( ( ccc | ddd ) AND NOT eee ) )",
			NULL
		},
		{
			"((( aaa & bbb & ccc ) !eee) | ((kkk | jjj & kkk & (zzz | jjj)) !eee))",
			"( ( ( aaa   bbb   ccc ) AND NOT eee ) | ( ( ( kkk | jjj )   kkk   ( zzz | jjj ) ) AND NOT eee ) )",
			"( ( ( aaa   bbb   ccc ) | ( ( kkk | jjj )   kkk   ( zzz | jjj ) ) ) AND NOT eee )",
			NULL
		},
		{
			"(aaa !(aaa !nnn)) | (bbb !(aaa !nnn))",
			"( ( aaa AND NOT ( aaa AND NOT nnn ) ) | ( bbb AND NOT ( aaa AND NOT nnn ) ) )",
			"( ( aaa | bbb ) AND NOT ( aaa AND NOT nnn ) )",
			NULL
		},

		// COMMON NOT WITH MIXED PHRASES/PROXIMITY terms
		{
			"(aaa !(\"zzz yyy\")) | (bbb !(\"zzz yyy\"~30)) | (ccc !(\"zzz yyy\"~20))",
			"( ( aaa AND NOT \"zzz yyy\" ) | ( bbb AND NOT \"zzz yyy\"~30 ) | ( ccc AND NOT \"zzz yyy\"~20 ) )",
			"( ( aaa | bbb | ccc ) AND NOT \"zzz yyy\"~30 )",
			NULL
		},

		// COMMON COMPOUND NOT
		{
			"(aaa !(nnn ccc)) | (bbb !(nnn ddd))",
			"( ( aaa AND NOT ( nnn   ccc ) ) | ( bbb AND NOT ( nnn   ddd ) ) )",
			"( ( aaa AND NOT ccc ) | ( bbb AND NOT ddd ) | ( ( aaa | bbb ) AND NOT nnn ) )",
			( const CKeywordHits * ) &dPseudoHits[0]
		},
		{
			"(aaa !(ccc nnn)) | (bbb !(nnn ddd)) | (ccc !nnn)",
			"( ( aaa AND NOT ( ccc   nnn ) ) | ( bbb AND NOT ( nnn   ddd ) ) | ( ccc AND NOT nnn ) )",
			"( ( aaa AND NOT ccc ) | ( bbb AND NOT ddd ) | ( ( ccc | aaa | bbb ) AND NOT nnn ) )",
			( const CKeywordHits * ) &dPseudoHits[0]
		},
		{
			"(aaa !(ccc nnn)) | (bbb !(nnn ddd))",
			"( ( aaa AND NOT ( ccc   nnn ) ) | ( bbb AND NOT ( nnn   ddd ) ) )",
			"( ( aaa AND NOT ( ccc   nnn ) ) | ( bbb AND NOT ( nnn   ddd ) ) )",
			( const CKeywordHits * ) &dPseudoHits[1]
		},

		// COMMON COMPOUND NOT WITH MIXED PHRASES/PROXIMITY terms
		{
			"(aaa !(ccc \"nnn zzz\"~20)) | (bbb !(\"nnn zzz\"~10 ddd)) | (ccc !\"nnn zzz\")",
			"( ( aaa AND NOT ( ccc   \"nnn zzz\"~20 ) ) | ( bbb AND NOT ( \"nnn zzz\"~10   ddd ) ) | ( ccc AND NOT \"nnn zzz\" ) )",
			"( ( aaa AND NOT ccc ) | ( bbb AND NOT ddd ) | ( ( ccc | aaa | bbb ) AND NOT \"nnn zzz\"~20 ) )",
			( const CKeywordHits * ) &dPseudoHits[0]
		},

		// COMMON SUBTERM
		{
			"(aaa (nnn | ccc)) | (bbb (nnn | ddd))",
			"( ( aaa   ( nnn | ccc ) ) | ( bbb   ( nnn | ddd ) ) )",
			"( ( aaa   ccc ) | ( bbb   ddd ) | ( ( aaa | bbb )   nnn ) )",
			( const CKeywordHits * ) &dPseudoHits[0]
		},
		{
			"(aaa (ccc | nnn)) | (bbb (nnn | ddd)) | (ccc | nnn)",
			"( ( aaa   ( ccc | nnn ) ) | ( bbb   ( nnn | ddd ) ) | ( ccc | nnn ) )",
			"( ( aaa   ccc ) | ( bbb   ddd ) | ccc | nnn | ( ( aaa | bbb )   nnn ) )",
			( const CKeywordHits * ) &dPseudoHits[0]
		},
		{
			"(aaa (ccc | nnn)) | (bbb (nnn | ddd))",
			"( ( aaa   ( ccc | nnn ) ) | ( bbb   ( nnn | ddd ) ) )",
			"( ( aaa   ( ccc | nnn ) ) | ( bbb   ( nnn | ddd ) ) )",
			( const CKeywordHits * ) &dPseudoHits[1]
		},

		// COMMON SUBTERM WITH MIXED PHRASES/PROXIMITY terms
		{
			"(aaa (ccc | \"qqq www\"~10)) | (bbb (\"qqq www\" | ddd)) | (ccc | \"qqq www\"~20)",
			"( ( aaa   ( ccc | \"qqq www\"~10 ) ) | ( bbb   ( \"qqq www\" | ddd ) ) | ( ccc | \"qqq www\"~20 ) )",
			"( ( aaa   ccc ) | ( bbb   ddd ) | ccc | \"qqq www\"~20 | ( ( aaa | bbb )   \"qqq www\"~10 ) )",
			( const CKeywordHits * ) &dPseudoHits[2]
		},

		// COMMON KEYWORDS
		{
			"\"aaa bbb ccc ddd jjj\" | \"aaa bbb\"",
			"( \"aaa bbb ccc ddd jjj\" | \"aaa bbb\" )",
			"\"aaa bbb\"",
			NULL
		},
		{
			"bbb | \"aaa bbb ccc\"",
			"( bbb | \"aaa bbb ccc\" )",
			"bbb",
			NULL
		},
		{
			"\"aaa bbb ccc ddd jjj\" | \"bbb ccc\"",
			"( \"aaa bbb ccc ddd jjj\" | \"bbb ccc\" )",
			"\"bbb ccc\"",
			NULL
		},
		{
			"\"aaa bbb ccc ddd jjj\" | \"bbb jjj\"",
			"( \"aaa bbb ccc ddd jjj\" | \"bbb jjj\" )",
			"( \"aaa bbb ccc ddd jjj\" | \"bbb jjj\" )",
			NULL
		},
		// FIXME!!! add exact phrase elimination
		{
			"\"aaa bbb ccc\"~10 | \"aaa bbb ccc ddd\"~20 | \"aaa bbb ccc\"~10 | \"aaa bbb ccc\"~10",
			"( \"aaa bbb ccc\"~10 | \"aaa bbb ccc ddd\"~20 | \"aaa bbb ccc\"~10 | \"aaa bbb ccc\"~10 )",
			// "( \"aaa bbb ccc ddd\"~20 | \"aaa bbb ccc\"~10 )",
			"( \"aaa bbb ccc\"~10 | \"aaa bbb ccc ddd\"~20 | \"aaa bbb ccc\"~10 | \"aaa bbb ccc\"~10 )",
			NULL
		},
		{
			"\"aaa bbb ccc\"~10 | \"aaa bbb ccc ddd\"~10",
			"( \"aaa bbb ccc\"~10 | \"aaa bbb ccc ddd\"~10 )",
			"\"aaa bbb ccc\"~10",
			NULL
		},
		{
			"\"aaa bbb ccc\"~10 | \"aaa bbb ccc\"~10",
			"( \"aaa bbb ccc\"~10 | \"aaa bbb ccc\"~10 )",
			// "\"aaa bbb ccc\"~10",
			"( \"aaa bbb ccc\"~10 | \"aaa bbb ccc\"~10 )",
			NULL
		},
		{
			"\"aaa bbb ccc\"~10 | \"aaa bbb ccc\"~9",
			"( \"aaa bbb ccc\"~10 | \"aaa bbb ccc\"~9 )",
			// "\"aaa bbb ccc\"~10",
			"( \"aaa bbb ccc\"~10 | \"aaa bbb ccc\"~9 )",
			NULL
		},
		{
			"\"aaa bbb ccc ddd eee\" | \"bbb ccc ddd\"~10",
			"( \"aaa bbb ccc ddd eee\" | \"bbb ccc ddd\"~10 )",
			"\"bbb ccc ddd\"~10",
			NULL
		},
		{
			"\"bbb ccc ddd\"~10 | \"ccc ddd\" | \"aaa bbb\"",
			"( \"bbb ccc ddd\"~10 | \"ccc ddd\" | \"aaa bbb\" )",
			"( \"bbb ccc ddd\"~10 | \"ccc ddd\" | \"aaa bbb\" )",
			NULL
		},
		{
			"\"aaa bbb ccc ddd eee\" | \"bbb ccc ddd\"~10 | \"ccc ddd\" | \"aaa bbb\"",
			"( \"aaa bbb ccc ddd eee\" | \"bbb ccc ddd\"~10 | \"ccc ddd\" | \"aaa bbb\" )",
			"( \"bbb ccc ddd\"~10 | \"ccc ddd\" | \"aaa bbb\" )",
			NULL
		},
		{
			"aaa | \"aaa bbb\"~10 | \"aaa ccc\"",
			"( aaa | \"aaa bbb\"~10 | \"aaa ccc\" )",
			"aaa",
			NULL
		},

		// COMMON PHRASES
		{
			"\"aaa bbb ccc ddd\" | \"eee fff ccc ddd\"",
			"( \"aaa bbb ccc ddd\" | \"eee fff ccc ddd\" )",
			"( \"( \"aaa bbb\" | \"eee fff\" ) \"ccc ddd\"\" )",
			NULL
		},
		{
			"\"ccc ddd aaa bbb\" | \"ccc ddd eee fff\"",
			"( \"ccc ddd aaa bbb\" | \"ccc ddd eee fff\" )",
			"( \"\"ccc ddd\" ( \"aaa bbb\" | \"eee fff\" )\" )",
			NULL
		},
		{
			"\"aaa bbb ccc ddd\" | \"eee fff ccc ddd\" | \"jjj lll\"",
			"( \"aaa bbb ccc ddd\" | \"eee fff ccc ddd\" | \"jjj lll\" )",
			"( \"jjj lll\" | ( \"( \"aaa bbb\" | \"eee fff\" ) \"ccc ddd\"\" ) )",
			NULL
		},
		{
			"\"ccc ddd aaa bbb\" | \"ccc ddd eee fff\" | \"jjj lll\"",
			"( \"ccc ddd aaa bbb\" | \"ccc ddd eee fff\" | \"jjj lll\" )",
			"( \"jjj lll\" | ( \"\"ccc ddd\" ( \"aaa bbb\" | \"eee fff\" )\" ) )",
			NULL
		},
		{
			"\"aaa bbb ccc ddd xxx yyy zzz\" | \"eee fff ddd xxx yyy zzz\" | \"jjj lll\"",
			"( \"aaa bbb ccc ddd xxx yyy zzz\" | \"eee fff ddd xxx yyy zzz\" | \"jjj lll\" )",
			"( \"jjj lll\" | ( \"( \"aaa bbb ccc\" | \"eee fff\" ) \"ddd xxx yyy zzz\"\" ) )",
			NULL
		},
		{
			"\"ddd xxx yyy zzz aaa bbb\" | \"ddd xxx yyy zzz ccc eee fff\" | \"jjj lll\"",
			"( \"ddd xxx yyy zzz aaa bbb\" | \"ddd xxx yyy zzz ccc eee fff\" | \"jjj lll\" )",
			"( \"jjj lll\" | ( \"\"ddd xxx yyy zzz\" ( \"aaa bbb\" | \"ccc eee fff\" )\" ) )",
			NULL
		},
		{
			"\"xxx zzz ccc ddd\" | \"xxx zzz yyy jjj kkk\" | \"xxx zzz yyy mmm nnn\"",
			"( \"xxx zzz ccc ddd\" | \"xxx zzz yyy jjj kkk\" | \"xxx zzz yyy mmm nnn\" )",
			"( \"\"xxx zzz\" ( \"ccc ddd\" | \"yyy jjj kkk\" | \"yyy mmm nnn\" )\" )",
			NULL
		},
		{
			"\"aaa bbb ddd www xxx yyy zzz\" | \"aaa bbb eee www xxx yyy zzz\"",
			"( \"aaa bbb ddd www xxx yyy zzz\" | \"aaa bbb eee www xxx yyy zzz\" )",
			"( \"( \"aaa bbb ddd\" | \"aaa bbb eee\" ) \"www xxx yyy zzz\"\" )",
			NULL
		},
		{
			"\"www xxx yyy zzz ddd aaa bbb\" | \"www xxx yyy zzz eee aaa bbb\"",
			"( \"www xxx yyy zzz ddd aaa bbb\" | \"www xxx yyy zzz eee aaa bbb\" )",
			"( \"\"www xxx yyy zzz\" ( \"ddd aaa bbb\" | \"eee aaa bbb\" )\" )",
			NULL
		},
		{
			"\"xxx yyy zzz ddd\" | \"xxx yyy zzz eee\"",
			"( \"xxx yyy zzz ddd\" | \"xxx yyy zzz eee\" )",
			"( \"\"xxx yyy zzz\" ( ddd | eee )\" )",
			NULL
		},
		{
			"\"ddd xxx yyy zzz\" | \"eee xxx yyy zzz\"",
			"( \"ddd xxx yyy zzz\" | \"eee xxx yyy zzz\" )",
			"( \"( ddd | eee ) \"xxx yyy zzz\"\" )",
			NULL
		},

		// COMMON AND NOT FACTOR
		{
			"( aaa !xxx ) | ( aaa !yyy ) | ( aaa !zzz )",
			"( ( aaa AND NOT xxx ) | ( aaa AND NOT yyy ) | ( aaa AND NOT zzz ) )",
			"( aaa AND NOT ( xxx   yyy   zzz ) )",
			NULL
		},

		{
			"( aaa !xxx ) | ( aaa !yyy ) | ( aaa !zzz ) | ( bbb !xxx ) | ( bbb !yyy ) | ( bbb !zzz )",
			"( ( aaa AND NOT xxx ) | ( aaa AND NOT yyy ) | ( aaa AND NOT zzz ) | ( bbb AND NOT xxx ) | ( bbb AND NOT yyy ) | ( bbb AND NOT zzz ) )",
			"( ( aaa | bbb ) AND NOT ( xxx   yyy   zzz ) )",
			NULL
		},

		// COMMON AND NOT FACTOR WITH MIXED PHRASES/PROXIMITY terms
		{
			"( \"aaa bbb\"~10 !xxx ) | ( \"aaa bbb\"~20 !yyy ) | ( \"aaa bbb\" !zzz )",
			"( ( \"aaa bbb\"~10 AND NOT xxx ) | ( \"aaa bbb\"~20 AND NOT yyy ) | ( \"aaa bbb\" AND NOT zzz ) )",
			"( \"aaa bbb\"~20 AND NOT ( yyy   xxx   zzz ) )",
			NULL
		},

		// COMMON | NOT
		{
			"( aaa !(nnn | nnn1) ) | ( bbb !(nnn2 | nnn) )",
			"( ( aaa AND NOT ( nnn | nnn1 ) ) | ( bbb AND NOT ( nnn2 | nnn ) ) )",
			"( ( ( aaa AND NOT nnn1 ) | ( bbb AND NOT nnn2 ) ) AND NOT nnn )",
			NULL
		},

		// ExcessAndNot
		{
			"( (aaa ( ( ( (fff (xxx !hhh)) !kkk ) ) bbb !ccc)) !ddd ) ( ( (zzz (xxx !vvv)) !kkk ) )",
			"( ( aaa   ( ( fff   ( xxx AND NOT hhh )   bbb ) AND NOT ( kkk | ccc ) )   ( ( zzz   ( xxx AND NOT vvv ) ) AND NOT kkk ) ) AND NOT ddd )",
			"( ( aaa   fff   xxx   bbb   zzz   xxx ) AND NOT ( vvv | hhh | kkk | kkk | ccc | ddd ) )",
			NULL
		},

		// COMMON | NOT WITH MIXED PHRASES/PROXIMITY terms
		{
			"( aaa !( \"jjj kkk\"~10 | (aaa|nnn) ) ) | ( bbb !( fff | \"jjj kkk\" ) ) | ( ccc !( (hhh kkk) | \"jjj kkk\"~20 ) )",
			"( ( aaa AND NOT ( \"jjj kkk\"~10 | ( aaa | nnn ) ) ) | ( bbb AND NOT ( fff | \"jjj kkk\" ) ) | ( ccc AND NOT ( ( hhh   kkk ) | \"jjj kkk\"~20 ) ) )",
			"( ( ( aaa AND NOT ( aaa | nnn ) ) | ( bbb AND NOT fff ) | ( ccc AND NOT ( hhh   kkk ) ) ) AND NOT \"jjj kkk\"~20 )",
			NULL
		},

		{
			NULL, NULL, NULL, NULL
		}
	};

	CSphIndexSettings tTmpSettings;
	const QueryTest_t * pTest = dTest;
	while ( pTest->m_sQuery )
	{
		printf ( "testing query transformations, test %d/%d... ", (int)( pTest-dTest+1 ), (int)( sizeof(dTest)/sizeof(dTest[0])-1 ) );

		XQQuery_t tQuery;
		sphParseExtendedQuery ( tQuery, pTest->m_sQuery, pTokenizer.Ptr(), &tSchema, pDict.Ptr(), tTmpSettings );

		CSphString sReconst = sphReconstructNode ( tQuery.m_pRoot, &tSchema );

		CSphDummyIndex tIndex;
		if ( pTest->m_pKeywordHits )
		{
			for ( const CKeywordHits * pHits = pTest->m_pKeywordHits; pHits->m_sKeyword; ++pHits )
				Verify ( tIndex.m_hHits.Add ( pHits->m_iHits, pHits->m_sKeyword ) );
		}

		sphTransformExtendedQuery ( &tQuery.m_pRoot, tTmpSettings, true, &tIndex );

		CSphString sReconstTransformed = sphReconstructNode ( tQuery.m_pRoot, &tSchema );

		if ( sReconst!=pTest->m_sReconst || sReconstTransformed!=pTest->m_sReconstTransformed )
			printf ( "\n\"%s\"\n\"%s\"\n\"%s\"\n\"%s\" -\n\"%s\" +\n", pTest->m_sQuery,
				sReconst.cstr(), pTest->m_sReconst, pTest->m_sReconstTransformed, sReconstTransformed.cstr() );

		assert ( sReconst==pTest->m_sReconst );
		assert ( sReconstTransformed==pTest->m_sReconstTransformed );

		pTest++;

		printf ( "ok\n" );
	}
}


//////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
void TestMisc ()
{
	BYTE dBuffer [ 128 ];
	int dValues[] = { 16383, 0, 1, 127, 128, 129, 256, 4095, 4096, 4097, 8192, 16383, 16384, 16385, 123456, 4194303, -1 };

	printf ( "testing string attr length packer/unpacker... " );

	BYTE * pRow = dBuffer;
	for ( int i=0; dValues[i]>=0; i++ )
		pRow += sphPackStrlen ( pRow, dValues[i] );

	const BYTE * pUnp = dBuffer;
	for ( int i=0; dValues[i]>=0; i++ )
	{
		int iUnp = sphUnpackStr ( pUnp, &pUnp );
		assert ( iUnp==dValues[i] );
	}

	printf ( "ok\n" );
}
#endif

//////////////////////////////////////////////////////////////////////////

void BenchLocators ()
{
	const int MAX_ITEMS = 10;
	const int NUM_MATCHES = 1000;
	const int NUM_RUNS = 100000;

	CSphRowitem dStatic[MAX_ITEMS];
	CSphRowitem dDynamic[MAX_ITEMS];
	CSphAttrLocator tLoc[NUM_MATCHES];
	CSphMatch tMatch[NUM_MATCHES];

	for ( int i=0; i<MAX_ITEMS; i++ )
		dStatic[i] = dDynamic[i] = i;

	srand ( 0 );
	for ( int i=0; i<NUM_MATCHES; i++ )
	{
		tLoc[i].m_iBitCount = 32;
		tLoc[i].m_iBitOffset = 32*( rand() % MAX_ITEMS ); // NOLINT
		tLoc[i].m_bDynamic = ( rand() % 2 )==1; // NOLINT
		tMatch[i].m_pStatic = dStatic;
		tMatch[i].m_pDynamic = dDynamic;
	}

	printf ( "benchmarking locators\n" );
	for ( int iRun=1; iRun<=3; iRun++ )
	{
		uint64_t tmLoc = sphMicroTimer();
		int iSum = 0;
		for ( int i=0; i<NUM_RUNS; i++ )
			for ( int j=0; j<NUM_MATCHES; j++ )
				iSum += (int)tMatch[j].GetAttr ( tLoc[j] );
		tmLoc = sphMicroTimer() - tmLoc;
		printf ( "run %d: sum=%d time=%d.%d msec\n", iRun, iSum, (int)(tmLoc/1000), (int)((tmLoc%1000)/100) );
	}

	// manually cleanup to avoid automatic delete
	for ( int i=0; i<NUM_MATCHES; i++ )
		tMatch[i].m_pDynamic = NULL;
}

//////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
static int g_iRwlock;
static CSphRwlock g_tRwlock;

void RwlockReader ( void * pArg )
{
	assert ( g_tRwlock.ReadLock() );
	sphSleepMsec ( 10 );
	*(int*)pArg = g_iRwlock;
	assert ( g_tRwlock.Unlock() );
}

void RwlockWriter ( void * pArg )
{
	assert ( g_tRwlock.WriteLock() );
	g_iRwlock += size_t(pArg);
	sphSleepMsec ( 3 );
	assert ( g_tRwlock.Unlock() );
}

void TestRwlock ()
{
	printf ( "testing rwlock... " );
	assert ( g_tRwlock.Init() );

	const int NPAIRS = 10;
	SphThread_t dReaders[NPAIRS];
	SphThread_t dWriters[NPAIRS];
	int iRead[NPAIRS];

	g_iRwlock = 0;
	for ( int i=0; i<NPAIRS; i++ )
	{
		assert ( sphThreadCreate ( &dReaders[i], RwlockReader, (void*)&iRead[i] ) );
		assert ( sphThreadCreate ( &dWriters[i], RwlockWriter, reinterpret_cast<void*>(1+i) ) );
	}

	for ( int i=0; i<NPAIRS; i++ )
	{
		assert ( sphThreadJoin ( &dReaders[i] ) );
		assert ( sphThreadJoin ( &dWriters[i] ) );
	}

	assert ( g_iRwlock==NPAIRS*(1+NPAIRS)/2 );
	assert ( g_tRwlock.Done() );

	int iReadSum = 0;
	for ( int i=0; i<NPAIRS; i++ )
		iReadSum += iRead[i];

	printf ( "ok (read_sum=%d)\n", iReadSum );
}
#endif // NDEBUG

//////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
void CleanupCallback ( void * pArg )
{
	*(bool*)pArg = true;
}

void CleanupThread ( void * pArg )
{
	sphThreadOnExit ( CleanupCallback, pArg );
}

void TestCleanup ()
{
	printf ( "testing thread cleanup... " );

	const int CLEANUP_COUNT = 10;

	bool bCleanup[CLEANUP_COUNT];
	for ( int i=0; i<CLEANUP_COUNT; i++ )
		bCleanup[i] = false;

	SphThread_t thd[CLEANUP_COUNT];
	for ( int i=0; i<CLEANUP_COUNT; i++ )
		assert ( sphThreadCreate ( &thd[i], CleanupThread, &bCleanup[i] ) );

	for ( int i=0; i<CLEANUP_COUNT; i++ )
		assert ( sphThreadJoin ( &thd[i] ) );

	for ( int i=0; i<CLEANUP_COUNT; i++ )
		assert ( bCleanup[i]==true );

	printf ( "ok\n" );
}
#endif // NDEBUG

//////////////////////////////////////////////////////////////////////////

static volatile int g_iMutexBench = 0;

void DummyThread ( void * )
{}

void MutexBenchThread ( void * pArg )
{
	CSphMutex * pMutex = (CSphMutex *) pArg;
	pMutex->Lock();
	for ( int i=0; i<100; i++ )
		g_iMutexBench++;
	g_iMutexBench -= 99;
	pMutex->Unlock();
}

void BenchThreads ()
{
	printf ( "benchmarking threads\n" );

	const int BATCHES = 100;
	const int BATCH_THREADS = 100;
	const int TOTAL_THREADS = BATCHES*BATCH_THREADS;

	SphThread_t * pThd = new SphThread_t [ BATCH_THREADS ];

	CSphMutex tMutex;
	if ( !tMutex.Init() )
		sphDie ( "failed to init mutex" );

	for ( int iRun=1; iRun<=2; iRun++ )
	{
		int64_t tmThd = sphMicroTimer();
		for ( int iBatch=0; iBatch<BATCHES; iBatch++ )
		{
			for ( int i=0; i<BATCH_THREADS; i++ )
				if ( !sphThreadCreate ( pThd+i, ( iRun==1 ) ? DummyThread : MutexBenchThread, &tMutex) )
					sphDie ( "failed to create thread (batch %d thd %d)", iBatch, i );

			for ( int i=0; i<BATCH_THREADS; i++ )
				if ( !sphThreadJoin ( pThd+i ) )
					sphDie ( "failed to join to thread (batch %d thd %d)", iBatch, i );
		}
		tmThd = sphMicroTimer()-tmThd;

		if ( iRun==2 && g_iMutexBench!=TOTAL_THREADS )
			sphDie ( "failed mutex benchmark (expected=%d, got=%d)", TOTAL_THREADS, g_iMutexBench );

		int iThdSec10 = (int)( int64_t(TOTAL_THREADS)*1000000*10/tmThd );
		const char * sDesc = ( iRun==1 ) ? "dummy" : "mutex";

		printf ( "run %d: %d %s threads in %d.%d msec; %d.%d thd/sec\n",
			iRun, TOTAL_THREADS, sDesc,
			(int)(tmThd/1000), (int)((tmThd%1000)/100),
			iThdSec10/10, iThdSec10%10 );
	}

	tMutex.Done ();
	SafeDeleteArray ( pThd );
}

//////////////////////////////////////////////////////////////////////////

void SortDataRepeat1245 ( DWORD * pData, int iCount )
{
	const int dFill[4] = { 1, 2, 4, 5 };
	for ( int i=0; i<iCount; i++ )
		pData[i] = dFill[i%4];
}

void SortDataEnd0 ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = i+1;
	pData[iCount-1] = 0;
}

void SortDataIdentical ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = 123;
}

void SortDataMed3Killer ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount/2; i++ )
		pData[i] = 1 + i + ( i & 1 )*(iCount/2-1);
	for ( int i=iCount/2; i<iCount; i++ )
		pData[i] = 2*(i-iCount/2+1);
}

void SortDataMidKiller ( DWORD * pData, int iCount )
{
	for ( int i=0; i<2; i++ )
		for ( int j=0; j<iCount/2; j++ )
			*pData++ = j*2+i;
}

void SortDataRandDupes ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = sphRand() % ( iCount/10 );
}

void SortDataRandUniq ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = i;
	for ( int i=0; i<iCount; i++ )
		Swap ( pData[i], pData[sphRand()%iCount] );
}

void SortDataRandSteps ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i+=100 )
	{
		int a = i;
		int b = Min ( i+100, iCount );
		for ( int j=a; j<b; j++ )
			pData[j] = j;
		for ( int j=a; j<b; j++ )
			Swap ( pData[j], pData [ a + sphRand()%(b-a) ] );
	}
}

void SortDataRevEnds ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = i;
	Swap ( pData[0], pData[iCount-1] );
}

void SortDataRevPartial ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = iCount-i;
	for ( int i=0; i<iCount/10; i++ )
		Swap ( pData[sphRand()%iCount], pData[sphRand()%iCount] );
}

void SortDataRevSaw ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i+=100 )
	{
		int a = i;
		int b = Min ( i+100, iCount );
		for ( int j=a; j<b; j++ )
			pData[j] = b-j;
	}
}

void SortDataReverse ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = iCount-i;
}

void SortDataStart1000 ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = 1+i;
	pData[0] = 1000;
}

void SortDataSeqPartial ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = 1+i;
	for ( int i=0; i<iCount/10; i++ )
		Swap ( pData[sphRand()%iCount], pData[sphRand()%iCount] );
}

void SortDataSeqSaw ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i+=100 )
	{
		int a = i;
		int b = Min ( i+100, iCount );
		for ( int j=a; j<b; j++ )
			pData[j] = j-a+1;
	}
}

void SortDataSeq ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = 1+i;
}

void SortDataAscDesc ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount/2; i++ )
		pData[i] = 1+i;
	for ( int i=iCount/2; i<iCount; i++ )
		pData[i] = iCount-i;
}

void SortDataDescAsc ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount/2; i++ )
		pData[i] = iCount/2-i;
	for ( int i=iCount/2; i<iCount; i++ )
		pData[i] = i-iCount/2+1;
}

void SortDataRand01 ( DWORD * pData, int iCount )
{
	for ( int i=0; i<iCount; i++ )
		pData[i] = ( sphRand()>>3 ) & 1;
}

typedef void (*SortDataGen_fn)( DWORD *, int );

struct SortDataGenDesc_t
{
	SortDataGen_fn		m_fnGen;
	const char *		m_sName;
};

static SortDataGenDesc_t g_dSortDataGens[] =
{
	{ SortDataRepeat1245,	"repeat1245" },
	{ SortDataEnd0,			"end0" },
	{ SortDataIdentical,	"identical" },
	{ SortDataMed3Killer,	"med3killer" },
	{ SortDataMidKiller,	"midkiller" },
	{ SortDataRandDupes,	"randdupes" },
	{ SortDataRandUniq,		"randuniq" },
	{ SortDataRandSteps,	"randsteps" },
	{ SortDataRevEnds,		"revends" },
	{ SortDataRevPartial,	"revpartial" },
	{ SortDataRevSaw,		"revsaw" },
	{ SortDataReverse,		"reverse" },
	{ SortDataStart1000,	"start1000" },
	{ SortDataSeqPartial,	"seqpartial" },
	{ SortDataSeqSaw,		"seqsaw" },
	{ SortDataSeq,			"sequential" },
	{ SortDataAscDesc,		"ascdesc" },
	{ SortDataDescAsc,		"descasc" },
	{ SortDataRand01,		"rand01" },
};

struct SortPayload_t
{
	DWORD m_uKey;
	DWORD m_uPayload[3];

	bool operator < ( const SortPayload_t & rhs ) const
	{
		return m_uKey < rhs.m_uKey;
	}
};

inline bool operator < ( const CSphWordHit & a, const CSphWordHit & b )
{
	return
		( a.m_iWordID<b.m_iWordID || \
		( a.m_iWordID==b.m_iWordID && a.m_iDocID<b.m_iDocID ) || \
		( a.m_iWordID==b.m_iWordID && a.m_iDocID==b.m_iDocID && a.m_iWordPos<b.m_iWordPos ) );
}

template < typename T >
int64_t BenchSort ( T * pData, int iCount, bool bCheck )
{
	int64_t tmSort = sphMicroTimer();
	sphSort ( pData, iCount );
	tmSort = sphMicroTimer() - tmSort;

	if ( bCheck )
	{
		for ( int i=0; i<iCount-1 && bCheck; i++ )
			if ( pData[i+1] < pData[i] )
				bCheck = false;
		if ( !bCheck )
			sphDie ( "sorting results check failed!" );
	}

	return tmSort;
}

void BenchSort ()
{
	const int MINSIZE = 100;
	const int MAXSIZE = 100000;
	const int RUNS = 100;

	FILE * fpRes = fopen ( "benchsort/res.csv", "w+" );
	if ( !fpRes )
		sphDie ( "failed to create benchsort/res.csv" );
	fprintf ( fpRes, "test-name;runs-by-size;total-time\n" );

	// bench synthetic payloads
	DWORD * pKeys = new DWORD [ MAXSIZE ];
	SortPayload_t * pValues = new SortPayload_t [ MAXSIZE ];

	for ( int iGen=0; iGen<(int)(sizeof(g_dSortDataGens)/sizeof(g_dSortDataGens[0])); iGen++ )
		for ( int iLen=MINSIZE; iLen<=MAXSIZE; iLen*=10 )
	{
		int64_t tmSort = 0;
		for ( int iRun=0; iRun<RUNS; iRun++ )
		{
			g_dSortDataGens[iGen].m_fnGen ( pKeys, iLen );
			for ( int i=0; i<iLen; i++ )
				pValues[i].m_uKey = pKeys[i];
			tmSort += BenchSort ( pValues, iLen, iRun==0 );
		}

		printf ( "%-12s 100x%-8d %d.%03d msec\n", g_dSortDataGens[iGen].m_sName, iLen, (int)(tmSort/1000), (int)(tmSort%1000) );
		fprintf ( fpRes, "%s;100x%d;%d,%03d\n", g_dSortDataGens[iGen].m_sName, iLen, (int)(tmSort/1000), (int)(tmSort%1000) );

		CSphString sFile;
		sFile.SetSprintf ( "benchsort/%s.%d.txt", g_dSortDataGens[iGen].m_sName, iLen );

#if 0
		FILE * fp = fopen ( sFile.cstr(), "w+" );
		if ( fp )
		{
			g_dSortDataGens[iGen].m_fnGen ( pKeys, iLen );
			for ( int i=0; i<iLen; i++ )
				fprintf ( fp, "%d\n", pKeys[i] );
			fclose ( fp );
		}
#endif
	}

	SafeDeleteArray ( pKeys );
	SafeDeleteArray ( pValues );

	// bench real hits
	const int MAXHITS = 10000000;
	const char * sHits = "benchsort/hits.bin";

	CSphWordHit * pHits = new CSphWordHit [ MAXHITS ];
	int HITS = MAXHITS;

	FILE * fp = fopen ( sHits, "rb+" );
	if ( !fp )
		sphDie ( "failed to open %s", sHits );
	if ( (int)fread ( pHits, sizeof(CSphWordHit), HITS, fp )!=HITS )
		sphDie ( "failed to read %s", sHits );
	fclose ( fp );

	int64_t tmSort = BenchSort ( pHits, HITS, true );

	printf ( "%-12s 100x%-8d %d.%03d msec\n", "hits", HITS, (int)(tmSort/1000), (int)(tmSort%1000) );
	fprintf ( fpRes, "%s;100x%d;%d,%03d\n", "hits", HITS, (int)(tmSort/1000), (int)(tmSort%1000) );

	SafeDeleteArray ( pHits );

	// owl down
	fclose ( fpRes );
	exit ( 0 );
}

//////////////////////////////////////////////////////////////////////////

extern DWORD sphCRC32 ( const BYTE * pString, int iLen );

struct TestAccCmp_fn
{
	typedef DWORD MEDIAN_TYPE;
	typedef DWORD * PTR_TYPE;

	int m_iStride;

	explicit TestAccCmp_fn ( int iStride )
		: m_iStride ( iStride )
	{}

	DWORD Key ( DWORD * pData ) const
	{
		return *pData;
	}

	void CopyKey ( DWORD * pMed, DWORD * pVal ) const
	{
		*pMed = Key ( pVal );
	}

	bool IsLess ( DWORD a, DWORD b ) const
	{
		return a < b;
	}

	void Swap ( DWORD * a, DWORD * b ) const
	{
		for ( int i=0; i<m_iStride; i++ )
			::Swap ( a[i], b[i] );
	}

	DWORD * Add ( DWORD * p, int i ) const
	{
		return p+i*m_iStride;
	}

	int Sub ( DWORD * b, DWORD * a ) const
	{
		return (int)((b-a)/m_iStride);
	}

	bool IsKeyDataSynced ( const DWORD * pData ) const
	{
		DWORD uKey = *pData;
		DWORD uHash = GenerateKey ( pData );
		return uKey==uHash;
	}

	DWORD GenerateKey ( const DWORD * pData ) const
	{
		return m_iStride > 1 ? sphCRC32 ( ( ( const BYTE * ) ( pData + 1 ) ), ( m_iStride - 1 ) * 4 ) : ( *pData );
	}
};


#ifndef NDEBUG
static bool IsSorted ( DWORD * pData, int iCount, const TestAccCmp_fn & fn )
{
	if ( iCount<1 )
		return true;

	const DWORD * pPrev = pData;
	if ( !fn.IsKeyDataSynced ( pPrev ) )
		return false;

	if ( iCount<2 )
		return true;

	for ( int i = 1; i < iCount; ++i )
	{
		const DWORD * pCurr = fn.Add ( pData, i );

		if ( fn.IsLess ( *pCurr , *pPrev ) || !fn.IsKeyDataSynced ( pCurr ) )
			return false;

		pPrev = pCurr;
	}

	return true;
}
#endif


void RandomFill ( DWORD * pData, int iCount, const TestAccCmp_fn & fn, bool bChainsaw )
{
	for ( int i = 0; i < iCount; ++i )
	{
		DWORD * pCurr = fn.Add ( pData, i );
		const DWORD * pNext = fn.Add ( pData, i + 1 );

		DWORD * pElem = pCurr;
		DWORD * pChainHill = bChainsaw && ( i % 2 ) ? fn.Add ( pData, i -1 ) : NULL;
		do
		{
			*pElem = pChainHill ? *pChainHill / 2 : sphRand();
			++pElem;
			pChainHill = pChainHill ? pChainHill + 1 : pChainHill;
		} while ( pElem!=pNext );

		*pCurr = fn.GenerateKey ( pCurr );
	}
}

void TestStridedSortPass ( int iStride, int iCount )
{
	printf ( "testing strided sort, stride=%d, count=%d... ", iStride, iCount );

	assert ( iStride && iCount );

	DWORD * pData = new DWORD [ iCount * iStride ];
	assert ( pData );

	// checked elements are random
	memset ( pData, 0, sizeof ( DWORD ) * iCount * iStride );
	TestAccCmp_fn fnSort ( iStride );
	RandomFill ( pData, iCount, fnSort, false );

	// crash on sort of mini-arrays
	TestAccCmp_fn fnSortDummy ( 1 );
	DWORD dMini[1] = { 1 };
	sphSort ( dMini, 1, fnSortDummy, fnSortDummy );
	sphSort ( dMini, 0, fnSortDummy, fnSortDummy );
	assert ( IsSorted ( dMini, 1, fnSortDummy ) );

	// random sort
	sphSort ( pData, iCount, fnSort, fnSort );
	assert ( IsSorted ( pData, iCount, fnSort ) );

	// already sorted sort
	sphSort ( pData, iCount, fnSort, fnSort );
	assert ( IsSorted ( pData, iCount, fnSort ) );

	// reverse order sort
	for ( int i = 0; i < iCount; ++i )
	{
		::Swap ( pData[i], pData [ iCount - i - 1 ] );
	}
	sphSort ( pData, iCount, fnSort, fnSort );
	assert ( IsSorted ( pData, iCount, fnSort ) );

	// random chainsaw sort
	RandomFill ( pData, iCount, fnSort, true );
	sphSort ( pData, iCount, fnSort, fnSort );
	assert ( IsSorted ( pData, iCount, fnSort ) );

	printf ( "ok\n" );
	SafeDeleteArray ( pData );
}

void TestStridedSort ()
{
	TestStridedSortPass ( 1, 2 );
	TestStridedSortPass ( 3, 2 );
	TestStridedSortPass ( 37, 2 );

	// SMALL_THRESH case
	TestStridedSortPass ( 1, 30 );
	TestStridedSortPass ( 7, 13 );
	TestStridedSortPass ( 113, 5 );

	TestStridedSortPass ( 1, 1000 );
	TestStridedSortPass ( 5, 1000 );
	TestStridedSortPass ( 17, 50 );
	TestStridedSortPass ( 31, 1367 );

	// rand cases
	for ( int i = 0; i < 10; ++i )
	{
		const int iRndStride = sphRand() % 64;
		const int iNrmStride = Max ( iRndStride, 1 );
		const int iRndCount = sphRand() % 1000;
		const int iNrmCount = Max ( iRndCount, 1 );
		TestStridedSortPass ( iNrmStride, iNrmCount );
	}

	// regression of uniq vs empty array
	DWORD dUniq[] = { 1, 1, 3, 1 };
	int iCount = sizeof(dUniq)/sizeof(dUniq[0]);
	assert ( sphUniq ( dUniq, 0 )==0 );
	sphSort ( dUniq, iCount );
	assert ( sphUniq ( dUniq, iCount )==2 && dUniq[0]==1 && dUniq[1]==3 );
	CSphVector<DWORD> dUniq1;
	dUniq1.Uniq();
	assert ( dUniq1.GetLength()==0 );
	dUniq1.Add ( 1 );
	dUniq1.Add ( 3 );
	dUniq1.Add ( 1 );
	dUniq1.Add ( 1 );
	dUniq1.Uniq();
	assert ( dUniq1.GetLength()==2 && dUniq1[0]==1 && dUniq1[1]==3 );
}

//////////////////////////////////////////////////////////////////////////

static const char * g_sFieldsData[] = { "33", "1033", "If I were a cat...", "We are the greatest cat" };

class SphTestDoc_c : public CSphSource_Document
{
public:
	explicit SphTestDoc_c ( const CSphSchema & tSchema ) : CSphSource_Document ( "test_doc" )
	{
		m_tSchema = tSchema;
	}

	virtual BYTE ** NextDocument ( CSphString & )
	{
		if ( m_tDocInfo.m_iDocID )
		{
			m_tDocInfo.m_iDocID = 0;
			return NULL;
		}

		m_tDocInfo.m_iDocID++;
		return (BYTE **) &g_sFieldsData[2];
	}

	bool Connect ( CSphString & ) { return true; }
	void Disconnect () {}
	bool HasAttrsConfigured () { return true; }
	bool IterateStart ( CSphString & ) { m_tDocInfo.Reset ( m_tSchema.GetRowSize() ); m_iPlainFieldsLength = m_tSchema.m_dFields.GetLength(); return true; }
	bool IterateMultivaluedStart ( int, CSphString & ) { return false; }
	bool IterateMultivaluedNext () { return false; }
	bool IterateFieldMVAStart ( int, CSphString & ) { return false; }
	bool IterateFieldMVANext () { return false; }
	bool IterateKillListStart ( CSphString & ) { return false; }
	bool IterateKillListNext ( SphDocID_t & ) { return false; }
};


#ifndef NDEBUG

static void CheckRT ( int iVal, int iRef, const char * sMsg )
{
#if 1
	assert ( iRef==iVal && sMsg );
#else
	if ( iRef!=iVal )
		printf ( "\t%s=%d ( %d )\n", sMsg, iVal, iRef );
#endif
}

static void DeleteIndexFiles ( const char * sIndex )
{
	if ( !sIndex )
		return;

	CSphString sName;
	sName.SetSprintf ( "%s.kill", sIndex );
	unlink ( sName.cstr() );
	sName.SetSprintf ( "%s.lock", sIndex );
	unlink ( sName.cstr() );
	sName.SetSprintf ( "%s.meta", sIndex );
	unlink ( sName.cstr() );
	sName.SetSprintf ( "%s.ram", sIndex );
	unlink ( sName.cstr() );

	sName.SetSprintf ( "%s.0.spa", sIndex );
	unlink ( sName.cstr() );
	sName.SetSprintf ( "%s.0.spd", sIndex );
	unlink ( sName.cstr() );
	sName.SetSprintf ( "%s.0.sph", sIndex );
	unlink ( sName.cstr() );
	sName.SetSprintf ( "%s.0.spi", sIndex );
	unlink ( sName.cstr() );
	sName.SetSprintf ( "%s.0.spk", sIndex );
	unlink ( sName.cstr() );
	sName.SetSprintf ( "%s.0.spm", sIndex );
	unlink ( sName.cstr() );
	sName.SetSprintf ( "%s.0.spp", sIndex );
	unlink ( sName.cstr() );
	sName.SetSprintf ( "%s.0.sps", sIndex );
	unlink ( sName.cstr() );
}


void TestRTInit ()
{
	CSphConfigSection tRTConfig;

	sphRTInit ( tRTConfig, true );
	sphRTConfigure ( tRTConfig, true );

	SmallStringHash_T<CSphIndex*> hIndexes;
	sphReplayBinlog ( hIndexes, 0 );
}


#define RT_INDEX_FILE_NAME "test_temp"
#define RT_PASS_COUNT 5
static const int g_iWeights[RT_PASS_COUNT] = { 1500, 1500, 1500, 1500, 1500 }; // { 1500, 1302, 1252, 1230, 1219 };

void TestRTWeightBoundary ()
{
	DeleteIndexFiles ( RT_INDEX_FILE_NAME );
	for ( int iPass = 0; iPass < RT_PASS_COUNT; ++iPass )
	{
		printf ( "testing rt indexing, test %d/%d... ", 1+iPass, RT_PASS_COUNT );
		TestRTInit ();

		CSphString sError, sWarning;
		CSphDictSettings tDictSettings;
		tDictSettings.m_bWordDict = false;

		ISphTokenizer * pTok = sphCreateUTF8Tokenizer();
		CSphDict * pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "weight", sError );

		CSphColumnInfo tCol;
		CSphSchema tSrcSchema;

		CSphSourceSettings tParams;
		tSrcSchema.Reset();

		tCol.m_sName = "channel_id";
		tCol.m_eAttrType = SPH_ATTR_INTEGER;
		tSrcSchema.AddAttr ( tCol, true );

		tCol.m_sName = "title";
		tSrcSchema.m_dFields.Add ( tCol );

		tCol.m_sName = "content";
		tSrcSchema.m_dFields.Add ( tCol );

		SphTestDoc_c * pSrc = new SphTestDoc_c ( tSrcSchema );

		pSrc->SetTokenizer ( pTok );
		pSrc->SetDict ( pDict );

		pSrc->Setup ( tParams );
		Verify ( pSrc->Connect ( sError ) );
		Verify ( pSrc->IterateStart ( sError ) );

		Verify ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

		CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
		tSchema.m_dFields = tSrcSchema.m_dFields;
		for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
			tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

		ISphRtIndex * pIndex = sphCreateIndexRT ( tSchema, "testrt", 32*1024*1024, RT_INDEX_FILE_NAME, false );

		// tricky bit
		// index owns its tokenizer/dict pair, and MAY do whatever it wants
		// and starting with meta v4, it WILL deallocate tokenizer/dict in Prealloc()
		// in favor of tokenizer/dict loaded from the saved settings in meta
		// however, source still needs those guys!
		// so for simplicity i just clone them
		pIndex->SetTokenizer ( pTok->Clone ( SPH_CLONE_INDEX ) );
		pIndex->SetDictionary ( pDict->Clone() );
		Verify ( pIndex->Prealloc ( false, false, sError ) );

		ISphHits * pHits;
		CSphVector<DWORD> dMvas;
		for ( ;; )
		{
			Verify ( pSrc->IterateDocument ( sError ) );
			if ( !pSrc->m_tDocInfo.m_iDocID )
				break;

			pHits = pSrc->IterateHits ( sError );
			if ( !pHits )
				break;

			pIndex->AddDocument ( pHits, pSrc->m_tDocInfo, NULL, dMvas, sError, sWarning );
			pIndex->Commit ();
		}

		pSrc->Disconnect();

		CheckRT ( (int)pSrc->GetStats().m_iTotalDocuments, 1, "docs committed" );

		CSphQuery tQuery;
		CSphQueryResult tResult;
		CSphMultiQueryArgs tArgs ( NULL, 1 );
		tQuery.m_sQuery = "@title cat";

		ISphMatchSorter * pSorter = sphCreateQueue ( &tQuery, pIndex->GetMatchSchema(), tResult.m_sError, NULL, false );
		assert ( pSorter );
		Verify ( pIndex->MultiQuery ( &tQuery, &tResult, 1, &pSorter, tArgs ) );
		sphFlattenQueue ( pSorter, &tResult, 0 );
		CheckRT ( tResult.m_dMatches.GetLength(), 1, "results found" );
		CheckRT ( (int)tResult.m_dMatches[0].m_iDocID, 1, "docID" );
		CheckRT ( tResult.m_dMatches[0].m_iWeight, g_iWeights[iPass], "weight" );
		SafeDelete ( pSorter );
		SafeDelete ( pIndex );

		SafeDelete ( pDict );
		SafeDelete ( pTok );
		sphRTDone ();

		printf ( "ok\n" );
	}
	DeleteIndexFiles ( RT_INDEX_FILE_NAME );
}

void TestWriter()
{
	printf ( "testing CSphWriter... " );
	const CSphString sTmpWriteout = "__writeout.tmp";
	CSphString sErr;

#define WRITE_OUT_DATA_SIZE 0x40000
	BYTE * pData = new BYTE[WRITE_OUT_DATA_SIZE];
	memset ( pData, 0xfe, WRITE_OUT_DATA_SIZE );

	{
		CSphWriter tWrDef;
		tWrDef.OpenFile ( sTmpWriteout, sErr );
		tWrDef.PutBytes ( pData, WRITE_OUT_DATA_SIZE );
		tWrDef.PutByte ( 0xff );
	}
	{
		CSphWriter tWr;
		tWr.SetBufferSize ( WRITE_OUT_DATA_SIZE );
		tWr.OpenFile ( sTmpWriteout, sErr );
		tWr.PutBytes ( pData, WRITE_OUT_DATA_SIZE );
		tWr.PutByte ( 0xff );
	}
	unlink ( sTmpWriteout.cstr() );
	printf ( "ok\n" );
}

class SphDocRandomizer_c : public CSphSource_Document
{
	static const int m_iMaxFields = 2;
	static const int m_iMaxFieldLen = 512;
	char m_dFields[m_iMaxFields][m_iMaxFieldLen];
	BYTE * m_ppFields[m_iMaxFields];
public:
	explicit SphDocRandomizer_c ( const CSphSchema & tSchema ) : CSphSource_Document ( "test_doc" )
	{
		m_tSchema = tSchema;
		for ( int i=0; i<m_iMaxFields; i++ )
			m_ppFields[i] = (BYTE *)&m_dFields[i];
	}

	virtual BYTE ** NextDocument ( CSphString & )
	{
		if ( m_tDocInfo.m_iDocID>800 )
		{
			m_tDocInfo.m_iDocID = 0;
			return NULL;
		}

		m_tDocInfo.m_iDocID++;

		m_tDocInfo.SetAttr ( m_tSchema.GetAttr(0).m_tLocator, m_tDocInfo.m_iDocID+1000 );
		m_tDocInfo.SetAttr ( m_tSchema.GetAttr(1).m_tLocator, 1313 );

		snprintf ( m_dFields[0], m_iMaxFieldLen, "cat title%d title%d title%d title%d title%d"
			, sphRand(), sphRand(), sphRand(), sphRand(), sphRand() );

		snprintf ( m_dFields[1], m_iMaxFieldLen, "dog contentwashere%d contentwashere%d contentwashere%d contentwashere%d contentwashere%d"
			, sphRand(), sphRand(), sphRand(), sphRand(), sphRand() );

		return &m_ppFields[0];
	}

	bool Connect ( CSphString & ) { return true; }
	void Disconnect () {}
	bool HasAttrsConfigured () { return true; }
	bool IterateStart ( CSphString & ) { m_tDocInfo.Reset ( m_tSchema.GetRowSize() ); m_iPlainFieldsLength = m_tSchema.m_dFields.GetLength(); return true; }
	bool IterateMultivaluedStart ( int, CSphString & ) { return false; }
	bool IterateMultivaluedNext () { return false; }
	bool IterateFieldMVAStart ( int, CSphString & ) { return false; }
	bool IterateFieldMVANext () { return false; }
	bool IterateKillListStart ( CSphString & ) { return false; }
	bool IterateKillListNext ( SphDocID_t & ) { return false; }
};

void TestRTSendVsMerge ()
{
	DeleteIndexFiles ( RT_INDEX_FILE_NAME );
	printf ( "testing rt send result during merge... " );

	TestRTInit ();

	CSphString sError, sWarning;
	CSphDictSettings tDictSettings;
	tDictSettings.m_bWordDict = false;

	ISphTokenizer * pTok = sphCreateUTF8Tokenizer();
	CSphDict * pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt", sError );

	CSphColumnInfo tCol;
	CSphSchema tSrcSchema;

	CSphSourceSettings tParams;
	tSrcSchema.Reset();

	tCol.m_sName = "title";
	tSrcSchema.m_dFields.Add ( tCol );

	tCol.m_sName = "content";
	tSrcSchema.m_dFields.Add ( tCol );

	tCol.m_sName = "tag1";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	tCol.m_sName = "tag2";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	SphDocRandomizer_c * pSrc = new SphDocRandomizer_c ( tSrcSchema );

	pSrc->SetTokenizer ( pTok );
	pSrc->SetDict ( pDict );

	pSrc->Setup ( tParams );
	Verify ( pSrc->Connect ( sError ) );
	Verify ( pSrc->IterateStart ( sError ) );

	Verify ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	tSchema.m_dFields = tSrcSchema.m_dFields;
	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

	ISphRtIndex * pIndex = sphCreateIndexRT ( tSchema, "testrt", 128*1024, RT_INDEX_FILE_NAME, false );

	pIndex->SetTokenizer ( pTok ); // index will own this pair from now on
	pIndex->SetDictionary ( pDict );
	Verify ( pIndex->Prealloc ( false, false, sError ) );

	CSphQuery tQuery;
	CSphQueryResult tResult;
	CSphMultiQueryArgs tArgs ( NULL, 1 );
	tQuery.m_sQuery = "@title cat";

	ISphMatchSorter * pSorter = sphCreateQueue ( &tQuery, pIndex->GetMatchSchema(), tResult.m_sError, NULL, false );
	assert ( pSorter );

	CSphVector<DWORD> dMvas;
	for ( ;; )
	{
		Verify ( pSrc->IterateDocument ( sError ) );
		if ( !pSrc->m_tDocInfo.m_iDocID )
			break;

		ISphHits * pHits = pSrc->IterateHits ( sError );
		if ( !pHits )
			break;

		pIndex->AddDocument ( pHits, pSrc->m_tDocInfo, NULL, dMvas, sError, sWarning );
		if ( pSrc->m_tDocInfo.m_iDocID==350 )
		{
			pIndex->Commit ();
			Verify ( pIndex->MultiQuery ( &tQuery, &tResult, 1, &pSorter, tArgs ) );
			sphFlattenQueue ( pSorter, &tResult, 0 );
		}
	}
	pIndex->Commit ();

	pSrc->Disconnect();

	tResult.m_tSchema = pSorter->GetSchema(); // can SwapOut

	for ( int i=0; i<tResult.m_dMatches.GetLength(); i++ )
	{
		const SphDocID_t tID = tResult.m_dMatches[i].m_iDocID;
		const SphAttr_t tTag1 = tResult.m_dMatches[i].GetAttr ( tResult.m_tSchema.GetAttr ( 0 ).m_tLocator );
		const SphAttr_t tTag2 = tResult.m_dMatches[i].GetAttr ( tResult.m_tSchema.GetAttr ( 1 ).m_tLocator );
		assert ( (SphDocID_t)tTag1==tID+1000 );
		assert ( tTag2==1313 );
	}
	SafeDelete ( pSorter );
	SafeDelete ( pIndex );

	sphRTDone ();

	printf ( "ok\n" );

	DeleteIndexFiles ( RT_INDEX_FILE_NAME );
}

#endif

//////////////////////////////////////////////////////////////////////////

void TestSentenceTokenizer()
{
	printf ( "testing sentence detection in tokenizer... " );

	CSphTokenizerSettings tSettings;
	tSettings.m_iType = TOKENIZER_SBCS;
	tSettings.m_iMinWordLen = 1;

	CSphString sError;
	ISphTokenizer * pTok = ISphTokenizer::Create ( tSettings, NULL, sError );

	assert ( pTok->SetCaseFolding ( "-, 0..9, A..Z->a..z, _, a..z, U+80..U+FF", sError ) );
//	assert ( pTok->SetBlendChars ( "., &", sError ) ); // NOLINT
	assert ( pTok->EnableSentenceIndexing ( sError ) );

	const char * SENTENCE = "\2"; // MUST be in sync with sphinx.cpp
	const char * sTest[] =
	{
		"Bill Gates Jr. attended", "bill", "gates", "jr", "attended", NULL,
		"Very good, Dr. Watson", "very", "good", "dr", "watson", NULL,
		"VERY GOOD, DR. WATSON", "very", "good", "dr", "watson", NULL,
		"He left US. Went abroad", "he", "left", "us", SENTENCE, "went", "abroad", NULL,
		"Known as Mr. Doe", "known", "as", "mr", "doe", NULL,
		"Survived by Mrs. Doe", "survived", "by", "mrs", "doe", NULL,
		"J. R. R. Tolkien", "j", "r", "r", "tolkien", NULL,
		"That is it. A boundary", "that", "is", "it", SENTENCE, "a", "boundary", NULL,
		"Just a sentence. And then some.", "just", "a", "sentence", SENTENCE, "and", "then", "some", SENTENCE, NULL,
		"Right, guy number two? Yes, guy number one!", "right", "guy", "number", "two", SENTENCE, "yes", "guy", "number", "one", SENTENCE, NULL,
		"S.T.A.L.K.E.R. sold well in the U.K and elsewhere. Including Russia.", "s", "t", "a", "l", "k", "e", "r", "sold", "well", "in", "the", "u", "k", "and", "elsewhere", SENTENCE, "including", "russia", SENTENCE, NULL,
		"Yoyodine Inc. exists since 1800", "yoyodine", "inc", "exists", "since", "1800", NULL,
		"John D. Doe, our CEO", "john", "d", "doe", "our", "ceo", NULL,
		"Yoyodine Inc. (the Company)", "yoyodine", "inc", "the", "company", NULL,
		NULL
	};

	int i = 0;
	while ( sTest[i] )
	{
		pTok->SetBuffer ( (BYTE*)sTest[i], strlen ( sTest[i] ) );
		i++;

		BYTE * sTok;
		while ( ( sTok = pTok->GetToken() )!=NULL )
		{
			assert ( !strcmp ( (char*)sTok, sTest[i] ) );
			i++;
		}

		assert ( sTest[i]==NULL );
		i++;
	}

	printf ( "ok\n" );
}

//////////////////////////////////////////////////////////////////////////

void TestSpanSearch()
{
	printf ( "testing span search... " );

	CSphVector<int> dVec;
	dVec.Add ( 1 );
	dVec.Add ( 3 );
	dVec.Add ( 4 );

	assert ( FindSpan ( dVec, 1, 5 )==0 );
	assert ( FindSpan ( dVec, 3, 5 )==1 );
	assert ( FindSpan ( dVec, 4, 5 )==2 );

	dVec.Add ( 15 );
	dVec.Add ( 17 );
	dVec.Add ( 22 );
	dVec.Add ( 23 );

	assert ( FindSpan ( dVec, 1, 5 )==0 );
	assert ( FindSpan ( dVec, 18, 5 )==4 );
	assert ( FindSpan ( dVec, 23, 5 )==6 );

	printf ( "ok\n" );
}

//////////////////////////////////////////////////////////////////////////

static const char * CORPUS = "corpus.txt";
const int POOLSIZE = 80*1048576;
const int GAP = 4;

void BenchStemmer ()
{
	CSphString sError;

#if SNOWBALL
	SN_env * pSnow = english_ISO_8859_1_create_env();
#if 1
	char test[] = "this";
	SN_set_current ( pSnow, strlen(test), (const symbol *)test );
	pSnow->p [ pSnow->l ] = 0;
	english_ISO_8859_1_stem ( pSnow );
	stem_en ( (BYTE*)test, strlen(test) );
#endif
#endif

#if PORTER1
	struct stemmer * z = create_stemmer();
#endif

	BYTE * pRaw = new BYTE [ POOLSIZE ];
	FILE * fp = fopen ( CORPUS, "rb" );
	if ( !fp )
		sphDie ( "fopen %s failed", CORPUS );
	int iLen = fread ( pRaw, 1, POOLSIZE, fp );
	printf ( "read %d bytes\n", iLen );
	fclose ( fp );

	ISphTokenizer * pTok = sphCreateSBCSTokenizer();
	if ( !pTok->SetCaseFolding ( "A..Z->a..z, a..z", sError ) )
		sphDie ( "oops: %s", sError.cstr() );


	pTok->SetBuffer ( pRaw, iLen );

	BYTE * pTokens = new BYTE [ POOLSIZE ];
	BYTE * p = pTokens;
	BYTE * sTok;
	int iToks = 0;
	int iBytes = 0;
	int iStemmed = 0;
	while ( ( sTok = pTok->GetToken() )!=NULL )
	{
		BYTE * pStart = p++; // 1 byte for length
		while ( *sTok )
			*p++ = *sTok++;
		*pStart = (BYTE)( p-pStart-1 ); // store length
		for ( int i=0; i<GAP; i++ )
			*p++ = '\0'; // trailing zero and a safety gap
		if ( p>=pTokens+POOLSIZE )
			sphDie ( "out of buffer at tok %d", iToks );
		iToks++;
	}
	*p++ = '\0';
	iBytes = (int)( p - pTokens );
	printf ( "tokenized %d tokens\n", iToks );

#if 0
	int dCharStats[256];
	memset ( dCharStats, 0, sizeof(dCharStats) );
	for ( BYTE * t = pTokens; t<pTokens+iBytes; t++ )
		dCharStats[*t]++;

	const char * sDump = "aeiouywxY";
	for ( const char * s = sDump; *s; s++ )
		printf ( "%c: %d\n", *s, dCharStats[*s] );

#endif
	int64_t tmStem = sphMicroTimer();
	p = pTokens;
	iToks = 0;
	int iDiff = 0;
	while ( *p )
	{
#if !SNOWBALL && !PORTER1
		stem_en ( p+1, *p );
#endif

#if SNOWBALL
		int iLen = *p;
		SN_set_current ( pSnow, iLen, p+1 );
		english_ISO_8859_1_stem ( pSnow );

#if !CROSSCHECK
		// benchmark
		memcpy ( p+1, pSnow->p, pSnow->l );
		p[pSnow->l+1] = 0;
#else
		// crosscheck
		char buf[256];
		memcpy ( buf, p+1, *p+1 );

		stem_en ( p+1, *p );
		int ll = strlen ( (char*)p+1 );
		if ( ll!=pSnow->l || memcmp ( p+1, pSnow->p, ll ) )
		{
			pSnow->p[pSnow->l] = 0;
			printf ( "%s[%d] vs %s[%d] for orig %s\n", p+1, ll, pSnow->p, pSnow->l, buf );
			iDiff++;
		}
#endif
#endif

#if PORTER1
		p [ stem ( z, (char*)p+1, *p-1 )+2 ] = 0;
#endif

		p += *p + GAP + 1;
		iToks++;
	}
	tmStem = sphMicroTimer() - tmStem;

	if ( iDiff )
		printf ( "%d tokens are different\n", iDiff );

	if ( iStemmed )
		printf ( "%d data bytes stemmed\n", iStemmed );

#if SNOWBALL
	english_ISO_8859_1_close_env ( pSnow );
#endif

	uint64_t uHash = sphFNV64 ( pTokens, iBytes );
	printf ( "stemmed %d tokens (%d bytes) in %d msec, hash %08x %08x\n",
		iToks, iBytes, (int)(tmStem/1000),
		(DWORD)( uHash>>32 ), (DWORD)( uHash & 0xffffffffUL ) );
	if ( uHash!=U64C ( 0x54ef4f21994b67db ) )
		printf ( "ERROR, HASH MISMATCH\n" );

	SafeDelete ( pTok );
	SafeDeleteArray ( pRaw );
}

//////////////////////////////////////////////////////////////////////////

void TestWildcards()
{
	printf ( "testing wildcards... " );
	assert ( sphWildcardMatch ( "abc", "abc" ) );
	assert ( sphWildcardMatch ( "abc", "?bc" ) );
	assert ( sphWildcardMatch ( "abc", "a?c" ) );
	assert ( sphWildcardMatch ( "abc", "ab?" ) );
	assert ( !sphWildcardMatch ( "abc", "?ab" ) );
	assert ( sphWildcardMatch ( "abac", "a*c" ) );
	assert ( sphWildcardMatch ( "abac", "a*?c" ) );
	assert ( sphWildcardMatch ( "abac", "a*??c" ) );
	assert ( sphWildcardMatch ( "abac", "a?*?c" ) );
	assert ( !sphWildcardMatch ( "abac", "a*???c" ) );
	assert ( sphWildcardMatch ( "abac", "a?a?" ) );
	assert ( !sphWildcardMatch ( "abac", "a?a??" ) );
	assert ( !sphWildcardMatch ( "abac", "a??a" ) );
	assert ( sphWildcardMatch ( "abracadabra", "a*" ) );
	assert ( sphWildcardMatch ( "abracadabra", "a*a" ) );
	assert ( !sphWildcardMatch ( "abracadabra", "a*c" ) );
	assert ( sphWildcardMatch ( "abracadabra", "?b*r?" ) );
	assert ( sphWildcardMatch ( "abracadabra", "?b*r*" ) );
	assert ( sphWildcardMatch ( "abracadabra", "?b*r*r*" ) );
	assert ( sphWildcardMatch ( "abracadabra", "*a*a*a*" ) );
	assert ( sphWildcardMatch ( "abracadabra", "*a*a*a*a*a*" ) );
	assert ( !sphWildcardMatch ( "a", "a*a?" ) );
	assert ( !sphWildcardMatch ( "abracadabra", "*a*a*a*a*a?" ) );
	assert ( sphWildcardMatch ( "car", "car%" ) );
	assert ( sphWildcardMatch ( "cars", "car%" ) );
	assert ( sphWildcardMatch ( "card", "car%" ) );
	assert ( !sphWildcardMatch ( "carded", "car%" ) );
	assert ( sphWildcardMatch ( "abc", "abc%" ) );
	assert ( sphWildcardMatch ( "abcd", "abc%" ) );
	assert ( !sphWildcardMatch ( "abcde", "abc%" ) );
	assert ( sphWildcardMatch ( "ab", "a%b" ) );
	assert ( sphWildcardMatch ( "acb", "a%b" ) );
	assert ( !sphWildcardMatch ( "acdb", "a%b" ) );
	assert ( sphWildcardMatch ( "abc", "a%bc" ) );
	assert ( sphWildcardMatch ( "abbc", "a%bc" ) );
	assert ( !sphWildcardMatch ( "abbbc", "a%bc" ) );
	assert ( sphWildcardMatch ( "ab", "a%%b" ) );
	assert ( sphWildcardMatch ( "axb", "a%%b" ) );
	assert ( sphWildcardMatch ( "axyb", "a%%b" ) );
	assert ( !sphWildcardMatch ( "axyzb", "a%%b" ) );
	assert ( sphWildcardMatch ( "a*b", "a?b" ) );
	assert ( sphWildcardMatch ( "a*b", "a*b" ) );
	assert ( sphWildcardMatch ( "a*b", "a\\*b" ) );
	assert ( !sphWildcardMatch ( "acb", "a\\*b" ) );
	assert ( !sphWildcardMatch ( "acdeb", "a\\*b" ) );
	printf ( "ok\n" );
}

//////////////////////////////////////////////////////////////////////////

void TestLog2()
{
	printf ( "testing integer log2 implementation... " );
	assert ( sphLog2(1)==1 );
	assert ( sphLog2(2)==2 );
	assert ( sphLog2(3)==2 );
	assert ( sphLog2(4)==3 );
	assert ( sphLog2(5)==3 );
	assert ( sphLog2(6)==3 );
	assert ( sphLog2(7)==3 );
	assert ( sphLog2(8)==4 );
	assert ( sphLog2(9)==4 );
	assert ( sphLog2(10)==4 );
	assert ( sphLog2(65535)==16 );
	assert ( sphLog2(65536)==17 );
	assert ( sphLog2 ( 0xffffffffUL )==32 );
	assert ( sphLog2 ( 0x100000000ULL )==33 );
	assert ( sphLog2 ( 0x100000001ULL )==33 );
	assert ( sphLog2 ( 0x1ffffffffULL )==33 );
	assert ( sphLog2 ( 0x200000000ULL )==34 );
	assert ( sphLog2 ( 0xffffffffffffffffULL )==64 );
	assert ( sphLog2 ( 0xfffffffffffffffeULL )==64 );
	assert ( sphLog2 ( 0xefffffffffffffffULL )==64 );
	assert ( sphLog2 ( 0x7fffffffffffffffULL )==63 );
	printf ( "ok\n" );
}

const int TIMER_THREAD_NRUNS = 10*1000*1000;

void BenchTimerThread ( void * pEndtime )
{
	volatile int iRes = 0;
	for ( int i=0; i<TIMER_THREAD_NRUNS; i++ )
		iRes += (int)sphMicroTimer();
	*(int64_t*)pEndtime = sphMicroTimer();
}

void BenchMisc()
{
	printf ( "benchmarking rand... " );

	const int NRUNS = 100*1000*1000;
	volatile int iRes = 0;
	int64_t t;

	sphSrand ( 0 );
	t = sphMicroTimer();
	for ( int i=0; i<NRUNS; i++ )
		iRes += sphRand();
	t = sphMicroTimer() - t;
	printf ( "%d msec per %dM calls, res %d\n", (int)( t/1000 ), (int)( NRUNS/1000000 ), iRes );

	printf ( "benchmarking rand+log2... " );
	sphSrand ( 0 );
	iRes = 0;
	t = sphMicroTimer();
	for ( int i=0; i<NRUNS; i++ )
		iRes += sphLog2 ( sphRand() );
	t = sphMicroTimer() - t;
	printf ( "%d msec per %dM calls, res %d\n", (int)( t/1000 ), (int)( NRUNS/1000000 ), iRes );

	printf ( "benchmarking timer... " );
	t = sphMicroTimer();
	for ( int i=0; i<NRUNS; i++ )
		iRes += (int)sphMicroTimer();
	t = sphMicroTimer() - t;
	printf ( "%d msec per %dM calls, res %d\n", (int)( t/1000 ), (int)( NRUNS/1000000 ), iRes );

	printf ( "benchmarking threaded timer... " );

	const int THREADS = 10;
	SphThread_t dThd [ THREADS ];

	int64_t tmStart = sphMicroTimer();
	int64_t tmEnd [ THREADS ];
	sphThreadInit ( false );

	for ( int i=0; i<THREADS; i++ )
		sphThreadCreate ( &dThd[i], BenchTimerThread, &tmEnd[i], false );
	for ( int i=0; i<THREADS; i++ )
		if ( !sphThreadJoin ( &dThd[i] ) )
			sphDie ( "thread_join failed" );

	int64_t iMin = INT64_MAX;
	int64_t iMax = 0;
	int64_t iAvg = 0;
	for ( int i=0; i<THREADS; i++ )
	{
		t = tmEnd[i] - tmStart;
		iMin = Min ( iMin, t );
		iMax = Max ( iMax, t );
		iAvg += t;
	}
	iMin /= 1000;
	iMax /= 1000;
	iAvg /= 1000*THREADS;

	printf ( "avg %d, min %d, max %d msec (%dM calls, %d threads)\n",
		int(iAvg), int(iMin), int(iMax),
		(int)( TIMER_THREAD_NRUNS/1000000 ), THREADS );
}

//////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
void TestArabicStemmer()
{
	printf ( "testing arabic stemmer... " );

	// a few words, cross-verified using NLTK implementation
	const char * dTests[] =
	{
		"\xd8\xb0\xd9\x87\xd8\xa8\xd8\xaa\0", "\xd8\xb0\xd9\x87\xd8\xa8\0",
		"\xd8\xa7\xd9\x84\xd8\xb7\xd8\xa7\xd9\x84\xd8\xa8\xd8\xa9\0", "\xd8\xb7\xd9\x84\xd8\xa8\0",
		"\xd8\xa7\xd9\x84\xd8\xb5\xd8\xba\xd9\x8a\xd8\xb1\xd8\xa9\0", "\xd8\xb5\xd8\xba\xd8\xb1\0",
		"\xd8\xa7\xd9\x84\xd9\x89\0", "\xd8\xa7\xd9\x84\xd9\x89\0",
		"\xd8\xa7\xd9\x84\xd9\x85\xd8\xaf\xd8\xb1\xd8\xb3\xd8\xa9\0", "\xd8\xaf\xd8\xb1\xd8\xb3\0",
		"\xd9\x88\xd8\xaf\xd8\xb1\xd8\xb3\xd8\xaa\0", "\xd8\xaf\xd8\xb1\xd8\xb3\0",
		"\xd8\xa7\xd9\x84\xd8\xaf\xd8\xb1\xd9\x88\xd8\xb3\0", "\xd8\xaf\xd8\xb1\xd8\xb3\0",
		"\xd8\xac\xd9\x85\xd9\x8a\xd8\xb9\xd9\x87\xd8\xa7\0", "\xd8\xac\xd9\x85\xd8\xb9\0",
		"\xd9\x88\xd8\xad\xd9\x8a\xd9\x86\0", "\xd9\x88\xd8\xad\xd9\x86\0",
		// "\xd8\xac\xd8\xa7\xd8\xa1\0", "\xd8\xac\xd8\xa7\xd8\xa1\0",
		"\xd9\x88\xd9\x82\xd8\xaa\0", "\xd9\x88\xd9\x82\xd8\xaa\0",
		// "\xd8\xa7\xd9\x84\xd8\xa7\xd8\xae\xd8\xaa\xd8\xa8\xd8\xa7\xd8\xb1\0", "\xd8\xae\xd8\xa8\xd8\xb1\0",
		"\xd9\x86\xd8\xac\xd8\xad\xd8\xaa\0", "\xd9\x86\xd8\xac\xd8\xad\0",
		"\xd8\xb7\xd8\xa7\xd9\x84\xd8\xa8\xd8\xaa\xd9\x86\xd8\xa7\0", "\xd8\xb7\xd9\x84\xd8\xa8\0",
		"\xd8\xa8\xd8\xa7\xd9\x85\xd8\xaa\xd9\x8a\xd8\xa7\xd8\xb2\0", "\xd9\x85\xd9\x8a\xd8\xb2\0",
		"\xd8\xa7\xd9\x84\xd9\x85\xd8\xaf\xd8\xa7\xd8\xb1\xd8\xb3\0", "\xd8\xaf\xd8\xb1\xd8\xb3\0",
		"\xd9\x84\xd9\x87\xd8\xa7\0", "\xd9\x84\xd9\x87\xd8\xa7\0",
		"\xd8\xaf\xd9\x88\xd8\xb1\0", "\xd8\xaf\xd9\x88\xd8\xb1\0",
		"\xd9\x83\xd8\xa8\xd9\x8a\xd8\xb1\0", "\xd9\x83\xd8\xa8\xd8\xb1\0",
		"\xd9\x81\xd9\x8a\0", "\xd9\x81\xd9\x8a\0",
		"\xd8\xaa\xd8\xb9\xd9\x84\xd9\x8a\xd9\x85\0", "\xd8\xb9\xd9\x84\xd9\x85\0",
		"\xd8\xa7\xd8\xa8\xd9\x86\xd8\xa7\xd9\x8a\xd9\x86\xd8\xa7\0", "\xd8\xa8\xd9\x86\xd9\x8a\0",
		// "\xd8\xa7\xd9\x84\xd8\xa7\xd8\xad\xd8\xa8\xd8\xa7\xd8\xa1\0", "\xd8\xad\xd8\xa8\xd8\xa1\0",
	};

	for ( int i=0; i<int(sizeof(dTests)/sizeof(dTests[0])); i+=2 )
	{
		char sBuf[64];
		snprintf ( sBuf, sizeof(sBuf), "%s", dTests[i] );
		stem_ar_utf8 ( (BYTE*)sBuf );
		assert ( strcmp ( sBuf, dTests[i+1] )==0 );
	}

	char sTest1[16] = "\xD9\x80\xD9\x80\xD9\x80\xD9\x80\0abcdef";
	char sRef1[16] = "\0\0\0\0\0\0\0\0\0abcdef";

	stem_ar_utf8 ( (BYTE*)sTest1 );
	assert ( memcmp ( sTest1, sRef1, sizeof(sTest1) )==0 );

	printf ( "ok\n" );
}
#endif // !NDEBUG

//////////////////////////////////////////////////////////////////////////

void TestAppendf()
{
	CSphStringBuilder sRes;
	sRes.Appendf ( "12345678" );
	sRes.Appendf ( "this is my rifle this is my gun" );
	sRes.Appendf ( " int=%d float=%f string=%s", 123, 456.789, "helloworld" );
	assert ( strcmp ( sRes.cstr(), "12345678this is my rifle this is my gun int=123 float=456.789000 string=helloworld" )==0 );
}

void BenchAppendf()
{
	int64_t tm1 = sphMicroTimer();
	CSphStringBuilder sRes1;
	for ( int i=0; i<200; i++ )
		sRes1.Appendf ( "%d ", i );
	tm1 = sphMicroTimer() - tm1;

	int64_t tm2 = sphMicroTimer();
	CSphString sRes2;
	sRes2.SetSprintf ( "%d ", 0 );
	for ( int i=1; i<200; i++ )
		sRes2.SetSprintf ( "%s%d ", sRes2.cstr(), i );
	tm2 = sphMicroTimer() - tm2;

	printf ( "benchmarking stringbuilder... %d microsec builder vs %d microsec string\n", int(tm1), int(tm2) );
}

//////////////////////////////////////////////////////////////////////////

// conversion between degrees and radians
static const double MY_PI = 3.14159265358979323846;
static const float TO_RADF = float ( MY_PI / 180.0 );
static const float TO_RADF2 = float ( MY_PI / 360.0 );
static const float TO_DEGF = float ( 180.0 / MY_PI );
static const double TO_RADD = MY_PI / 180.0;
static const double TO_RADD2 = MY_PI / 360.0;
static const double TO_DEGD = 180.0 / MY_PI;

static inline float GeodistVincenty ( double lat1, double lon1, double lat2, double lon2 )
{
	lat1 *= TO_RADD;
	lon1 *= TO_RADD;
	lat2 *= TO_RADD;
	lon2 *= TO_RADD;
	const double a = 6378137;
	const double b = 6356752.314245;
	double f = (a-b)/a;
	double L = lon2 - lon1;
	double u1 = atan ( (1-f) * tan(lat1) );
	double u2 = atan ( (1-f) * tan(lat2) );
	double sin_u1 = sin(u1);
	double cos_u1 = cos(u1);
	double sin_u2 = sin(u2);
	double cos_u2 = cos(u2);
	double lambda = L;
	double lambda_pi = 2*MY_PI;
	double sin_sigma = 0, cos_sigma = 0, sigma = 0, cos_sq_alpha = 0, cos2sigma_m = 0;
	while ( fabs ( lambda-lambda_pi ) > 1e-12 )
	{
		double sin_lambda = sin(lambda);
		double cos_lambda = cos(lambda);
		sin_sigma = sqrt ( (cos_u2 * sin_lambda) * (cos_u2*sin_lambda) +
			( cos_u1*sin_u2-sin_u1*cos_u2*cos_lambda ) * ( cos_u1*sin_u2-sin_u1*cos_u2*cos_lambda ) );
		cos_sigma = sin_u1*sin_u2 + cos_u1*cos_u2*cos_lambda;
		sigma = atan2 ( sin_sigma, cos_sigma );
		double alpha = asin ( cos_u1 * cos_u2 * sin_lambda / sin_sigma );
		cos_sq_alpha = cos(alpha) * cos(alpha);
		cos2sigma_m = cos_sigma - 2*sin_u1*sin_u2/cos_sq_alpha;
		double cc = f/16*cos_sq_alpha*(4+f*(4-3*cos_sq_alpha));
		lambda_pi = lambda;
		lambda = L + (1-cc) * f * sin(alpha) *
			(sigma + cc*sin_sigma*(cos2sigma_m+cc*cos_sigma*(-1+2*cos2sigma_m*cos2sigma_m)));
	}
	double usq = cos_sq_alpha*(a*a-b*b)/(b*b);
	double aa = 1 + usq/16384*(4096+usq*(-768+usq*(320-175*usq)));
	double bb = usq/1024 * (256+usq*(-128+usq*(74-47*usq)));
	double delta_sigma = bb*sin_sigma*(cos2sigma_m+bb/4*(cos_sigma*(-1+2*cos2sigma_m*cos2sigma_m)-
		bb/6*cos2sigma_m*(-3+4*sin_sigma*sin_sigma)*(-3+4*cos2sigma_m*cos2sigma_m)));
	double c = b*aa*(sigma-delta_sigma);
	return (float)c;
}

void DestVincenty ( double lat1, double lon1, double brng, double dist, double *lat2, double *lon2 )
{
	double a = 6378137, b = 6356752.3142, f = 1/298.257223563; // WGS-84 ellipsiod
	double s = dist;
	double alpha1 = brng*TO_RADD;
	double sinAlpha1 = sin(alpha1);
	double cosAlpha1 = cos(alpha1);

	double tanU1 = (1-f) * tan ( lat1*TO_RADD );
	double cosU1 = 1 / sqrt ( 1 + tanU1*tanU1 ), sinU1 = tanU1*cosU1;
	double sigma1 = atan2 ( tanU1, cosAlpha1 );
	double sinAlpha = cosU1 * sinAlpha1;
	double cosSqAlpha = 1 - sinAlpha*sinAlpha;
	double uSq = cosSqAlpha * (a*a - b*b) / (b*b);
	double A = 1 + uSq/16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
	double B = uSq/1024 * (256+uSq*(-128+uSq*(74-47*uSq)));

	double sigma = s / (b*A), sigmaP = 2*MY_PI;
	double cos2SigmaM = 0, sinSigma = 0, cosSigma = 0;
	while ( fabs ( sigma-sigmaP ) > 1e-12 )
	{
		cos2SigmaM = cos ( 2*sigma1 + sigma );
		sinSigma = sin(sigma);
		cosSigma = cos(sigma);
		double deltaSigma = B*sinSigma*(cos2SigmaM+B/4*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-
			B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
		sigmaP = sigma;
		sigma = s / (b*A) + deltaSigma;
	}

	double tmp = sinU1*sinSigma - cosU1*cosSigma*cosAlpha1;
	*lat2 = atan2 ( sinU1*cosSigma + cosU1*sinSigma*cosAlpha1,
		(1-f)*sqrt ( sinAlpha*sinAlpha + tmp*tmp ) );
	double lambda = atan2 ( sinSigma*sinAlpha1, cosU1*cosSigma - sinU1*sinSigma*cosAlpha1 );
	double C = f/16*cosSqAlpha*(4+f*(4-3*cosSqAlpha));
	double L = lambda - (1-C) * f * sinAlpha *
		(sigma + C*sinSigma*(cos2SigmaM+C*cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)));
	*lon2 = (lon1*TO_RADD+L+3*MY_PI);
	while ( *lon2>2*MY_PI )
		*lon2 -= 2*MY_PI;
	*lon2 -= MY_PI;
	*lat2 *= TO_DEGD;
	*lon2 *= TO_DEGD;
}

static const int NFUNCS = 3;

float CalcGeofunc ( int iFunc, double * t )
{
	switch ( iFunc )
	{
		case 0: return GeodistSphereDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
		case 1: return GeodistAdaptiveDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
		case 2: return GeodistFlatDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
	}
	return 0;
}


class GeodistStatic
{
public:
	float m_lat1;
	float m_lon1;
	double m_cos1d2;
	double m_sin1;

public:
	GeodistStatic ( float lat1, float lon1 )
	{
		m_lat1 = lat1;
		m_lon1 = lon1;
		m_cos1d2 = 111415.13 * 0.5 * cos ( TO_RADD*lat1 );
		m_sin1 = 111415.13 * sin ( TO_RADD*lat1 );
	}

	float Dist ( float lat2, float lon2 )
	{
		float dlat = m_lat1 - lat2;
		float dlon = m_lon1 - lon2;

		// fallback to haversine for distant enough points
		if ( dlat>16 || dlon>16 || dlat<-16 || dlon<-16 )
			return GeodistSphereDeg ( m_lat1, m_lon1, lat2, lon2 );

		// we approximate cos() using Taylor expansions
		// even in 2013, this is still quicker
		// midpoint = lat1 + (lat2-lat1)/2 = lat1 + b
		// b <= 16*pi/180 ~= 0.2792
		// cos(midpoint) = cos(lat1)*cos(b) - sin(lat1)*sin(b)
		// cos(b) = 1 - b^2/2 + b^4/24 ... ~= 1-b*b/2 because b^4/24 <= 0.0002
		// sin(b) = b - b^3/6 + b^5/120 ... ~= b because b^3/6 <= 0.036
		// cos(midpoint) ~= cos(lat1) - cos(lat1)*b*b/2 + sin(lat1)*b
		double b = (lat2-m_lat1)*TO_RADD2;
		double k2 = 2*m_cos1d2 - m_cos1d2*b*b - m_sin1*b; // cos(midpoint)
		return (float)sqrt ( 111132.09*111132.09*dlat*dlat + k2*k2*dlon*dlon );
	}
};

inline float MaxErr ( float fMax, float fRes, double fRef )
{
	float fErr = (float)( fabs ( fRes-fRef )*100 / fRef );
	return fErr > fMax ? fErr : fMax;
}

void TestGeodist()
{
	CSphVector<double> dBench;
	for ( int adist=10; adist<=10*1000*1000; adist*=10 )
		for ( int dist=adist; dist<10*adist && dist<20*1000*1000; dist+=2*adist )
	{
		double avgerr[NFUNCS] = {0}, maxerr[NFUNCS] = {0};
		int n = 0;
		for ( int lat=-80; lat<=80; lat+=10 )
		{
			for ( int lon=-179; lon<180; lon+=3 )
			{
				for ( int b=0; b<360; b+=3, n++ )
				{
					double t[4] = { lat, lon, 0, 0 };
					DestVincenty ( t[0], t[1], b, dist, t+2, t+3 );
					for ( int j=0; j<4; j++ )
						dBench.Add ( t[j] );
					for ( int f=0; f<NFUNCS; f++ )
					{
						float fDist = CalcGeofunc ( f, t );
						double err = fabs ( 100*( double(fDist) - double(dist) ) / double(dist) ); // relative error, in percents
						avgerr[f] += err;
						maxerr[f] = Max ( err, maxerr[f] );
					}
				}
			}
		}
		if ( dist>=1000 )
			printf ( "%5dkm", dist/1000 );
		else
			printf ( "%6dm", dist );
		for ( int f=0; f<NFUNCS; f++ )
			printf ( ", f%d %5.2f%% %5.2f%%", f, avgerr[f]/n, maxerr[f] );
		printf ( "\n" );
	}

	const int RUNS = 10;
	float fDist = 0;
	double * tmax = dBench.Begin() + dBench.GetLength();
	int64_t tm;
	printf ( "%d calls in bench\n", RUNS*dBench.GetLength() );

	tm = sphMicroTimer();
	for ( int r=0; r<RUNS; r++ )
		for ( double * t = dBench.Begin(); t<tmax; t+=4 )
			fDist += GeodistSphereDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) );
	printf ( INT64_FMT" us sphere\n", sphMicroTimer()-tm );

	tm = sphMicroTimer();
	for ( int r=0; r<RUNS; r++ )
		for ( double * t = dBench.Begin(); t<tmax; t+=4 )
			fDist += GeodistFlatDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) );
	printf ( INT64_FMT" us flat\n", sphMicroTimer()-tm );

	tm = sphMicroTimer();
	for ( int r=0; r<RUNS; r++ )
		for ( double * t = dBench.Begin(); t<tmax; t+=4 )
			fDist += GeodistAdaptiveDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) );
	printf ( INT64_FMT" us adaptive\n", sphMicroTimer()-tm );

	printf ( "res %f\n", fDist );

#if 0
	// coordinates from Wikimapia/Googlemaps
	//
	// distances by Wolfram Alpha (supposedly defaults to Vincenty method)
	// geodistance[{51.5007788, -0.1246771}, {46.2041222, 6.1524349}]
	//
	// 40.6890895, -74.0446899 center of the torch of the Statue of Liberty, New York, USA
	// 40.7041146, -74.0152399 center of The Sphere in Battery Park, New York, USA
	// 40.7643929, -73.9997683 tip of Lockheed A-12 (SR-71) on Intrepid, NY, USA
	// 40.7642578, -73.9994565 tail of Lockheed A-12 (SR-71) on Intrepid, NY, USA
	// 55.7535204, 37.6195371 center of Senatskaya tower, Red Square, Moscow, Russia
	// 51.6606654, 39.1999751 center of Lenin statue, Lenin Square, Voronezh, Russia
	// 49.2055275, -123.2014474 NW corner of Runway 08L-26R, YVR airport, Vancouver, Canada
	// 49.2007563, -123.1596478 NE corner of Runway 08L-26R, YVR airport, Vancouver, Canada
	// 37.6284983, -122.3927365 N corner of L on Runway 10L-28R, SFO airport, San Francisco, USA
	// 37.6137799, -122.3577954 S corner of R on Runway 10L-28R, SFO airport, San Francisco, USA
	// 68.974714, 33.0611873 tip of Lenin icebreaker, Murmansk, Russia
	// -22.9519125, -43.2105616 center of the head of Christ the Redeemer statue, Rio de Janeiro, Brazil
	// 51.5007788, -0.1246771 tip of Big Ben tower, London, England
	// 29.97973, 31.1342695 tip of Pyramid of Cheops, Cairo, Egypt
	// 41.4034549, 2.1741718 tip of the southern tower of Sagrada Familia, Barcelona, Spain
	// 42.6848586, 23.3188623 tip of National Palace of Culture, Sofia, Bulgaria
	// 46.2041222, 6.1524349 center of the fountain in English garden, Geneva, Switzerland
	// 37.8106517, -122.4174678 tip of SS Jeremiah O'Brien, Pier 45, San Francisco, USA
	// 37.8114358, -122.4186279 tail of SS Jeremiah O'Brien, Pier 45, San Francisco, USA
	// 64.1475975, -21.9224185 center of Sun Voyager in Reykjavik, Iceland
	// 63.8079982, -19.5589042 center of Eyjafjallajokull volcano, Iceland
	double dTest[][5] =
	{
		{ 40.7643929, -73.9997683, 40.7642578, -73.9994565, 30.3013 }, // Lockheed A-12 (SR-71) length (30.97m per wiki)
		{ 37.8106517, -122.4174678, 37.8114358, -122.4186279, 134.20 }, // SS Jeremiah O'Brien length ((134.57m per wiki)
		{ 40.6890895, -74.0446899, 40.7041146, -74.0152399, 2996.59 }, // Statue of Liberty to The Sphere
		{ 49.2055275, -123.2014474, 49.2007563, -123.1596478, 3091.96 }, // YVR Runway 08L-26R length (3030m per wiki)
		{ 37.6284983, -122.3927365, 37.6137799, -122.3577954, 3490.54 }, // SFO Runway 10L-28R length (3618m per wiki)
		{ 64.1475975, -21.9224185, 63.8079982, -19.5589042, 121768.14 }, // Reykjavik to Eyjafjallajokull
		{ 55.7535204, 37.6195371, 51.6606654, 39.1999751, 467301.55 }, // Moscow to Voronezh
		{ 51.5007788, -0.1246771, 46.2041222, 6.1524349, 747189.88 }, // London to Geneva
		{ 51.5007788, -0.1246771, 41.4034549, 2.1741718, 1136075.00 }, // London to Barcelona
		{ 51.5007788, -0.1246771, 42.6848586, 23.3188623, 2019138.10 }, // London to Sofia
		{ 51.5007788, -0.1246771, 29.97973, 31.1342695, 3513002.04 }, // London to Cairo
		{ 68.974714, 33.0611873, -22.9519125, -43.2105616, 11833803.11 }, // Murmansk to Rio
		{ 0, 0, 0.5, 179.5, 19936288.579 }, // antipodes, direct Vincenty killer
		// { 0, 0, 0.5, 179.7, 19944127.421 }, // antipodes, inverse Vincenty killer
	};

	for ( int i=0; i<sizeof(dTest)/sizeof(dTest[0]); i++ )
	{
		double * t = dTest[i];
		printf ( "%2d: ref %10.1f", i, t[4] );
		for ( int iFunc=0; iFunc<NFUNCS; iFunc++ )
		{
			float fDist = CalcGeofunc ( iFunc, t );
			printf ( ", f%d %5.2f%%", iFunc, 100*(fDist-t[4])/t[4] );
		}
		printf ( "\n" );
	}
	printf ( "\n" );
#endif
}

//////////////////////////////////////////////////////////////////////////

int main ()
{
	// threads should be initialized before memory allocations
	char cTopOfMainStack;
	sphThreadInit();
	MemorizeStack ( &cTopOfMainStack );

	printf ( "RUNNING INTERNAL LIBSPHINX TESTS\n\n" );

#if 0
	GeodistInit();
	TestGeodist();
	BenchSort ();
#endif

#ifdef NDEBUG
	BenchAppendf();
	BenchMisc();
	BenchStripper ();
	BenchTokenizer ( false );
	BenchTokenizer ( true );
	BenchExpr ();
	BenchLocators ();
	BenchThreads ();
#else
	TestAppendf();
	TestQueryParser ();
	TestQueryTransforms ();
	TestStripper ();
	TestTokenizer ( false );
	TestTokenizer ( true );
	TestExpr ();
	TestMisc ();
	TestRwlock ();
	TestCleanup ();
	TestStridedSort ();
	TestRTWeightBoundary ();
	TestWriter();
	TestRTSendVsMerge ();
	TestSentenceTokenizer ();
	TestSpanSearch ();
	TestWildcards();
	TestLog2();
	TestArabicStemmer();
#endif

	unlink ( g_sTmpfile );
	printf ( "\nSUCCESS\n" );
	return 0;
}

//
// $Id$
//

