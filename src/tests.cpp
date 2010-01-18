//
// $Id$
//

//
// Copyright (c) 2001-2010, Andrew Aksyonoff
// Copyright (c) 2008-2010, Sphinx Technologies Inc
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
#include <math.h>

//////////////////////////////////////////////////////////////////////////

const char * g_sTmpfile = "__libsphinxtest.tmp";

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


ISphTokenizer * CreateTestTokenizer ( bool bUTF8, bool bSynonyms, bool bEscaped = false )
{
	CSphString sError;
	CSphTokenizerSettings tSettings;
	tSettings.m_iType = bUTF8 ? TOKENIZER_UTF8 : TOKENIZER_SBCS;
	tSettings.m_iMinWordLen = 2;
	ISphTokenizer * pTokenizer = ISphTokenizer::Create ( tSettings, sError );
	assert ( pTokenizer->SetCaseFolding ( "-, 0..9, A..Z->a..z, _, a..z, U+80..U+FF", sError ) );
	pTokenizer->AddSpecials ( "!-" );
	pTokenizer->EnableQueryParserMode ( true );
	if ( bSynonyms )
		assert ( pTokenizer->LoadSynonyms ( g_sTmpfile, sError ) );

	if ( bEscaped )
	{
		ISphTokenizer * pOldTokenizer = pTokenizer;
		pTokenizer = pTokenizer->Clone ( true );
		SafeDelete ( pOldTokenizer );
	}

	return pTokenizer;
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
		bool bEscaped = iRun==3;
		ISphTokenizer * pTokenizer = CreateTestTokenizer ( bUTF8, iRun>=2, bEscaped );

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
		ISphTokenizer * pShortTokenizer = pTokenizer->Clone ( bEscaped );

		CSphRemapRange tStar ( '*', '*', '*' );
		pShortTokenizer->AddCaseFolding ( tStar );

		CSphTokenizerSettings tSettings = pShortTokenizer->GetSettings();
		tSettings.m_iMinWordLen = 5;
		pShortTokenizer->Setup ( tSettings );

		pShortTokenizer->EnableQueryParserMode ( true );

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

		SafeDelete ( sLine4 );

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

		// done
		SafeDelete ( pTokenizer );
	}
}


void BenchTokenizer ( bool bUTF8 )
{
	printf ( "benchmarking %s tokenizer\n", bUTF8 ? "UTF8" : "SBCS" );
	if ( !CreateSynonymsFile ( NULL ) )
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
		ISphTokenizer * pTokenizer = bUTF8 ? sphCreateUTF8Tokenizer () : sphCreateSBCSTokenizer ();
		pTokenizer->SetCaseFolding ( "-, 0..9, A..Z->a..z, _, a..z", sError );
		if ( iRun==2 )
			pTokenizer->LoadSynonyms ( g_sTmpfile, sError );
		pTokenizer->AddSpecials ( "!-" );

		const int iPasses = 10;
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

		printf ( "run %d: %d bytes, %d tokens, %d.%03d ms, %.3f MB/sec\n", iRun, iData, iTokens, (int)(tmTime/1000), (int)(tmTime%1000), float(iData)/tmTime );
		SafeDeleteArray ( sData );
	}
}

//////////////////////////////////////////////////////////////////////////

void TestStripper ()
{
	const char * sTests[][4] =
	{
		// source-data, index-attrs, remove-elements, expected-results
		{ "<html>trivial test</html>", "", "", " trivial test " },
		{ "<html>lets <img src=\"g/smth.jpg\" alt=\"nice picture\">index attrs</html>", "img=alt", "", " lets nice picture index attrs " },
		{ "<html>   lets  also<script> whatever here; a<b</script>remove scripts", "", "script, style", "    lets  also remove scripts" },
		{ "testing in<b><font color='red'>line</font> ele<em>men</em>ts", "", "", "testing inline elements" },
		{ "testing non<p>inline</h1>elements", "", "", "testing non inline elements" },
		{ "testing&nbsp;entities&amp;stuff", "", "", "testing entities&stuff" },
		{ "testing &#1040;&#1041;&#1042; utf encoding", "", "", "testing \xD0\x90\xD0\x91\xD0\x92 utf encoding" },
		{ "testing <1 <\" <\x80 <\xe0 <\xff </3 malformed tags", "", "", "testing <1 <\" <\x80 <\xe0 <\xff </3 malformed tags" },
		{ "testing comm<!--comm-->ents", "", "", "testing comments" },
		{ "&lt; &gt; &thetasym; &somethingverylong; &the", "", "", "< > \xCF\x91 &somethingverylong; &the" },
		{ "testing <img src=\"g/smth.jpg\" alt=\"nice picture\" rel=anotherattr junk=throwaway>inline tags vs attr indexing", "img=alt,rel", "", "testing nice picture anotherattr inline tags vs attr indexing" },
		{ "this <?php $code = \"must be stripped\"; ?> away", "", "", "this  away" },
		{ "<a href=\"http://www.com\">content1</a>", "a=title", "", "content1" },
		{ "<a href=\"http://www.com\" title=\"my test title\">content2</a>", "a=title", "", "my test title content2" },
		{ "testing <img src=\"g/smth.jpg\" alt=\"nice picture\" rel=anotherattr junk=\"throwaway\">inline tags vs attr indexing", "img=alt,rel", "", "testing nice picture anotherattr inline tags vs attr indexing" },
		{ "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"><html>test</html>", "", "", " test " },
		{ "<!smth \"that>can<break\"><html>test</html>", "", "", " test " }
	};

	int nTests = (int)(sizeof(sTests)/sizeof(sTests[0]));
	for ( int iTest=0; iTest<nTests; iTest++ )
	{
		printf ( "testing HTML stripper, test %d/%d... ", 1+iTest, nTests );

		CSphString sError;
		CSphHTMLStripper tStripper;
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
		CSphHTMLStripper tStripper;
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
	};

	const int nTests = sizeof(dTests)/sizeof(dTests[0]);
	for ( int iTest=0; iTest<nTests; iTest++ )
	{
		printf ( "testing expression evaluation, test %d/%d... ", 1+iTest, nTests );

		CSphString sError;
		CSphScopedPtr<ISphExpr> pExpr ( sphExprParse ( dTests[iTest].m_sExpr, tSchema, NULL, NULL, sError ) );
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

		DWORD uType;
		CSphString sError;
		CSphScopedPtr<ISphExpr> pExpr ( sphExprParse ( dBench[iRun].m_sExpr, tSchema, &uType, NULL, sError ) );
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

CSphString ReconstructNode ( const XQNode_t * pNode, const CSphSchema & tSchema )
{
	CSphString sRes ( "" );

	if ( pNode->IsPlain() )
	{
		// say just words to me
		const CSphVector<XQKeyword_t> & dWords = pNode->m_dWords;
		ARRAY_FOREACH ( i, dWords )
			sRes.SetSprintf ( "%s %s", sRes.cstr(), dWords[i].m_sWord.cstr() );
		sRes.Chop ();

		if ( pNode->m_bQuorum || pNode->m_iMaxDistance>0 )
		{
			sRes.SetSprintf ( "\"%s\"%c%d", sRes.cstr(), pNode->m_bQuorum ? '/' : '~', pNode->m_iMaxDistance ); // quorum or proximity
		} else if ( dWords.GetLength()>1 )
		{
			if ( pNode->m_iMaxDistance==0 )
				sRes.SetSprintf ( "\"%s\"", sRes.cstr() ); // phrase
			else
				sRes.SetSprintf ( "%s", sRes.cstr() ); // just bag of words
		}

		if ( pNode->m_uFieldMask!=0xFFFFFFFFUL )
		{
			CSphString sFields ( "" );
			for ( int i=0; i<32; i++ )
				if ( pNode->m_uFieldMask & (1<<i) )
					sFields.SetSprintf ( "%s,%s", sFields.cstr(), tSchema.m_dFields[i].m_sName.cstr() );

			sRes.SetSprintf ( "( @%s: %s )", sFields.cstr()+1, sRes.cstr() );
		} else
		{
			if ( !pNode->m_bQuorum && pNode->m_iMaxDistance<0 && dWords.GetLength()>1 )
				sRes.SetSprintf ( "( %s )", sRes.cstr() ); // wrap bag of words
		}

	} else
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
		{
			if ( !i )
				sRes = ReconstructNode ( pNode->m_dChildren[i], tSchema );
			else
			{
				const char * sOp = "(unknown-op)";
				switch ( pNode->GetOp() )
				{
					case SPH_QUERY_AND:		sOp = "AND"; break;
					case SPH_QUERY_OR:		sOp = "OR"; break;
					case SPH_QUERY_NOT:		sOp = "NOT"; break;
					case SPH_QUERY_ANDNOT:	sOp = "AND NOT"; break;
					case SPH_QUERY_BEFORE:	sOp = "BEFORE"; break;
				}
				sRes.SetSprintf ( "%s %s %s", sRes.cstr(), sOp, ReconstructNode ( pNode->m_dChildren[i], tSchema ).cstr() );
			}
		}

		if ( pNode->m_dChildren.GetLength()>1 )
			sRes.SetSprintf ( "( %s )", sRes.cstr() );
	}

	return sRes;
}


void TestQueryParser ()
{
	CSphString sTmp;

	CSphSchema tSchema;
	CSphColumnInfo tCol;
	tCol.m_sName = "title"; tSchema.m_dFields.Add ( tCol );
	tCol.m_sName = "body"; tSchema.m_dFields.Add ( tCol );

	CSphDictSettings tDictSettings;
	CSphScopedPtr<ISphTokenizer> pTokenizer ( sphCreateSBCSTokenizer () );
	CSphScopedPtr<CSphDict> pDict ( sphCreateDictionaryCRC ( tDictSettings, pTokenizer.Ptr(), sTmp ) );
	assert ( pTokenizer.Ptr() );
	assert ( pDict.Ptr() );

	CSphTokenizerSettings tTokenizerSetup;
	tTokenizerSetup.m_iMinWordLen = 2;
	tTokenizerSetup.m_sSynonymsFile = g_sTmpfile;
	pTokenizer->Setup ( tTokenizerSetup );

	CSphString sError;
	assert ( CreateSynonymsFile ( NULL ) );
	assert ( pTokenizer->LoadSynonyms ( g_sTmpfile, sError ) );

	struct QueryTest_t
	{
		const char *	m_sQuery;
		const char *	m_sReconst;
	};
	const QueryTest_t dTest[] =
	{
		{ "aaa bbb ccc",					"( aaa AND bbb AND ccc )" },
		{ "aaa|bbb ccc",					"( ( aaa OR bbb ) AND ccc )" },
		{ "aaa bbb|ccc",					"( aaa AND ( bbb OR ccc ) )" },
		{ "aaa (bbb ccc)|ddd",				"( aaa AND ( ( bbb AND ccc ) OR ddd ) )" },
		{ "aaa bbb|(ccc ddd)",				"( aaa AND ( bbb OR ( ccc AND ddd ) ) )" },
		{ "aaa bbb|(ccc ddd)|eee|(fff)",	"( aaa AND ( bbb OR ( ccc AND ddd ) OR eee OR fff ) )" },
		{ "aaa bbb|(ccc ddd) eee|(fff)",	"( aaa AND ( bbb OR ( ccc AND ddd ) ) AND ( eee OR fff ) )" },
		{ "aaa (ccc ddd)|bbb|eee|(fff)",	"( aaa AND ( ( ccc AND ddd ) OR bbb OR eee OR fff ) )" },
		{ "aaa (ccc ddd)|bbb eee|(fff)",	"( aaa AND ( ( ccc AND ddd ) OR bbb ) AND ( eee OR fff ) )" },
		{ "aaa \"bbb ccc\"~5|ddd",			"( aaa AND ( \"bbb ccc\"~5 OR ddd ) )" },
		{ "aaa bbb|\"ccc ddd\"~5",			"( aaa AND ( bbb OR \"ccc ddd\"~5 ) )" },
		{ "aaa ( ( \"bbb ccc\"~3|ddd ) eee | ( fff -ggg ) )",	"( aaa AND ( ( \"bbb ccc\"~3 OR ddd ) AND ( eee OR ( fff AND NOT ggg ) ) ) )" },
		{ "@title aaa @body ccc|(@title ddd eee)|fff ggg",		"( ( @title: aaa ) AND ( ( @body: ccc ) OR ( ( @title: ddd ) AND ( @title: eee ) ) OR ( @body: fff ) ) AND ( @body: ggg ) )" },
		{ "@title hello world | @body sample program",			"( ( @title: hello ) AND ( ( @title: world ) OR ( @body: sample ) ) AND ( @body: program ) )" },
		{ "@title one two three four",							"( ( @title: one ) AND ( @title: two ) AND ( @title: three ) AND ( @title: four ) )" },
		{ "@title one (@body two three) four",					"( ( @title: one ) AND ( ( @body: two ) AND ( @body: three ) ) AND ( @title: four ) )" },
		{ "windows 7 2000",										"( windows AND 2000 )" },
		{ "aaa a|bbb",											"( aaa AND bbb )" },
		{ "aaa bbb|x y z|ccc",									"( aaa AND bbb AND ccc )" },
		{ "a",													"" },
		{ "hello -world",										"( hello AND NOT world )" },
		{ "-hello world",										"( world AND NOT hello )" },
		{ "\"phrase (query)/3 ~on steroids\"",					"\"phrase query on steroids\"" },
		{ "hello a world",										"( hello AND world )" },
		{ "-one",												"" },
		{ "-one -two",											"" },
		{ "\"\"",												"" },
		{ "\"()\"",												"" },
		{ "\"]\"",												"" },
		{ "@title hello @body -world",							"( ( @title: hello ) AND NOT ( @body: world ) )" },
		{ "Ms-Dos",							"MS-DOS" }
	};


	int nTests = sizeof(dTest)/sizeof(dTest[0]);
	for ( int i=0; i<nTests; i++ )
	{
		printf ( "testing query parser, test %d/%d... ", i+1, nTests );

		XQQuery_t tQuery;
		sphParseExtendedQuery ( tQuery, dTest[i].m_sQuery, pTokenizer.Ptr(), &tSchema, pDict.Ptr() );

		CSphString sReconst = ReconstructNode ( tQuery.m_pRoot, tSchema );
		assert ( sReconst==dTest[i].m_sReconst );

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
		tLoc[i].m_iBitOffset = 32*( rand() % MAX_ITEMS );
		tLoc[i].m_bDynamic = ( rand() % 2 )==1;
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
int g_iRwlock;
CSphRwlock g_tRwlock;

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

volatile int g_iMutexBench = 0;

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

SortDataGenDesc_t g_dSortDataGens[] =
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
	const DWORD * pPrev = pData;
	if ( !fn.IsKeyDataSynced ( pPrev ) )
		return false;

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
	printf ( "testing sort stride=%d count=%d ...\t", iStride, iCount );

	assert ( iStride && iCount );

	DWORD * pData = new DWORD [ iCount * iStride ];
	assert ( pData );

	// checked elements are random
	memset ( pData, 0, sizeof ( DWORD ) * iCount * iStride );
	TestAccCmp_fn fnSort ( iStride );
	RandomFill ( pData, iCount, fnSort, false );

	// random sort
	sphSort ( pData, iCount, fnSort, fnSort );
	assert ( IsSorted ( pData, iCount, fnSort ) );
	printf ( "ok\n" );

	// already sorted sort
	printf ( "\t\talready sorted testing...\t" );
	sphSort ( pData, iCount, fnSort, fnSort );
	assert ( IsSorted ( pData, iCount, fnSort ) );
	printf ( "ok\n" );

	// reverse order sort
	printf ( "\t\treverse sorted testing...\t" );
	for ( int i = 0; i < iCount; ++i )
	{
		::Swap ( pData[i], pData [ iCount - i - 1 ] );
	}
	sphSort ( pData, iCount, fnSort, fnSort );
	assert ( IsSorted ( pData, iCount, fnSort ) );
	printf ( "ok\n" );

	// random chainsaw sort
	printf ( "\t\tchainsaw sort testing...\t" );
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
	printf ( "\nrandom values sort testing...\n" );
	for ( int i = 0; i < 10; ++i )
	{
		const int iRndStride = sphRand() % 64;
		const int iNrmStride = Max ( iRndStride, 1 );
		const int iRndCount = sphRand() % 1000;
		const int iNrmCount = Max ( iRndCount, 1 );
		TestStridedSortPass ( iNrmStride, iNrmCount );
	}
}

//////////////////////////////////////////////////////////////////////////

const char * g_sFieldsData[] = { "33", "1033", "If I were a cat...", "We are the greatest cat" };

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
			return NULL;

		m_tDocInfo.m_iDocID++;
		return (BYTE **) &g_sFieldsData[2];
	}

	bool Connect ( CSphString & ) { return true; }
	void Disconnect () {}
	bool HasAttrsConfigured () { return true; }
	bool IterateHitsStart ( CSphString & ) { m_tDocInfo.Reset ( m_tSchema.GetRowSize() ); return true; }
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


void TestRT ()
{
	const int iMaxPasses = 5;
	for ( int iPass = 0; iPass < iMaxPasses; ++iPass )
	{
		printf ( "testing rt indexing, test %d/%d... ", 1+iPass, iMaxPasses );

		CSphConfigSection tRTConfig;
		sphRTInit ( tRTConfig );
		CSphVector< ISphRtIndex * > dTemp;
		sphReplayBinlog ( dTemp );

		CSphString sError;
		CSphDictSettings tDictSettings;

		ISphTokenizer * pTok = sphCreateUTF8Tokenizer();
		CSphDict * pDict = sphCreateDictionaryCRC ( tDictSettings, pTok, sError );

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
		assert ( pSrc->Connect ( sError ) );
		assert ( pSrc->IterateHitsStart ( sError ) );

		assert ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

		CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
		tSchema.m_dFields = tSrcSchema.m_dFields;
		for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
			tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

		ISphRtIndex * pIndex = sphCreateIndexRT ( tSchema, "testrt", 32*1024*1024, "test_temp" );

		pIndex->SetTokenizer ( pTok ); // index will own this pair from now on
		pIndex->SetDictionary ( pDict );
		assert ( pIndex->Prealloc ( false, sError ) );

		while ( pSrc->IterateHitsNext ( sError ) && pSrc->m_tDocInfo.m_iDocID )
		{
			pIndex->AddDocument ( pSrc->m_dHits, pSrc->m_tDocInfo );
			pIndex->Commit ();
		}

		pSrc->Disconnect();

		CheckRT ( pSrc->GetStats().m_iTotalDocuments, 1, "docs committed" );

		CSphQuery tQuery;
		CSphQueryResult tResult;
		tQuery.m_sQuery = "@title cat";

		ISphMatchSorter * pSorter = sphCreateQueue ( &tQuery, *pIndex->GetSchema(), tResult.m_sError, false );
		assert ( pSorter );
		assert ( pIndex->MultiQuery ( &tQuery, &tResult, 1, &pSorter, NULL ) );
		sphFlattenQueue ( pSorter, &tResult, 0 );
		CheckRT ( tResult.m_dMatches.GetLength(), 1, "results found" );
		CheckRT ( tResult.m_dMatches[0].m_iDocID, 1, "docID" );
		CheckRT ( tResult.m_dMatches[0].m_iWeight, 1500, "weight" );
		SafeDelete ( pSorter );
		SafeDelete ( pIndex );

		sphRTDone ();

		printf ( "ok\n" );
	}
}
#endif

//////////////////////////////////////////////////////////////////////////

int main ()
{
	printf ( "RUNNING INTERNAL LIBSPHINX TESTS\n\n" );

#if 0
	BenchSort ();
#endif

#ifdef NDEBUG
	BenchStripper ();
	BenchTokenizer ( false );
	BenchTokenizer ( true );
	BenchExpr ();
	BenchLocators ();
	BenchThreads ();
#else
	TestQueryParser ();
	TestStripper ();
	TestTokenizer ( false );
	TestTokenizer ( true );
	TestExpr ();
	TestMisc ();
	TestRwlock ();
	TestCleanup ();
	TestStridedSort ();
	TestRT ();
#endif

	unlink ( g_sTmpfile );
	printf ( "\nSUCCESS\n" );
	return 0;
}

//
// $Id$
//
