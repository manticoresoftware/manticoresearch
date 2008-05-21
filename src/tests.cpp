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
#include "sphinxexpr.h"
#include "sphinxutils.h"
#include "sphinxquery.h"
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


void TestTokenizer ( bool bUTF8, bool bEscaped = false )
{
	const char * sPrefix = bUTF8 
		? "testing UTF8 tokenizer"
		: "testing SBCS tokenizer";

	for ( int iRun=1; iRun<=2; iRun++ )
	{
		// simple "one-line" tests
		const char * sMagic = bUTF8
			? "\xD1\x82\xD0\xB5\xD1\x81\xD1\x82\xD1\x82\xD1\x82" // valid UTF-8
			: "\xC0\xC1\xF5\xF6"; // valid SBCS but invalid UTF-8

		assert ( CreateSynonymsFile ( sMagic ) );
		ISphTokenizer * pTokenizer = CreateTestTokenizer ( bUTF8, iRun==2, bEscaped );

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
			"2", "MS DOSS feat.Deskview.MS DOS",			"ms", "doss", "featuring", "deskview", "MS-DOS", NULL,
			"2", sMagic,									"test", NULL,
			"2", "U.S. U.S.A. U.S.A.F.",					"US", "USA", "USAF", NULL,
			"2", "U.S.AB U.S.A. U.S.B.U.S.D.U.S.U.S.A.F.",	"US", "ab", "USA", "USB", "USD", "US", "USAF", NULL,
			"3", "phon\\e",						"phone", NULL,
			"3", "\\thephone",					"thephone",  NULL,
			"3", "the\\!phone",					"the!phone", NULL,
			"3", "\\!phone",					"!phone", NULL,
			NULL
		};

		for ( int iCur=0; dTests[iCur] && atoi(dTests[iCur++])<=iRun; )
		{
			printf ( "%s, run=%d, line=%s\n", sPrefix, iRun, dTests[iCur] );
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

		// test misc SBCS-only and UTF8-only one-liners
		char * dTests2[] =
		{
			"0", "\x80\x81\x82",				"\x80\x81\x82", NULL,
			"1", "\xC2\x80\xC2\x81\xC2\x82",	"\xC2\x80\xC2\x81\xC2\x82", NULL,
			NULL
		};

		for ( int iCur=0; dTests2[iCur] && atoi(dTests2[iCur++])==int(bUTF8); )
		{
			printf ( "%s, run=%d, line=%s\n", sPrefix, iRun, dTests2[iCur] );
			pTokenizer->SetBuffer ( (BYTE*)dTests2[iCur], strlen(dTests2[iCur]) );
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
			char * sLine3 = "hi\xd0\xffh";

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

		char * dTestsShort[] =
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
		assert  ( pTokenizer->SetBoundary ( "?", sError ) );

		char sLine5[] = "hello world? testing boundaries?";
		pTokenizer->SetBuffer ( (BYTE*)sLine5, strlen(sLine5) );

		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "hello" ) ); assert ( !pTokenizer->GetBoundary() );
		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "world" ) ); assert ( !pTokenizer->GetBoundary() );
		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "testing" ) ); assert ( pTokenizer->GetBoundary() );
		assert ( !strcmp ( (const char*)pTokenizer->GetToken(), "boundaries" ) ); assert ( !pTokenizer->GetBoundary() );

		// check escaped sequences
		if ( bEscaped )
		{
			int iRun = 3;
			for ( int iCur=0; dTests[iCur] && atoi(dTests[iCur++])<=iRun; )
			{
				if ( atoi(dTests[iCur-1])!=iRun )
				{
					while ( dTests [iCur++] );
					continue;
				}

				printf ( "%s, run=%d, line=%s\n", sPrefix, iRun, dTests[iCur] );
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
		}

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
		{ "testing <img src=\"g/smth.jpg\" alt=\"nice picture\" rel=anotherattr junk=throwaway>inline tags vs attr indexing", "img=alt,rel", "", "testing nice picture anotherattr inline tags vs attr indexing" }
	};

	for ( int iTest=0; iTest<(int)(sizeof(sTests)/sizeof(sTests[0])); iTest++ )
	{
		printf ( "testing HTML stripper, test %d\n", 1+iTest );

		CSphString sError;
		CSphHTMLStripper tStripper;
		assert ( tStripper.SetIndexedAttrs ( sTests[iTest][1], sError ) );
		assert ( tStripper.SetRemovedElements ( sTests[iTest][2], sError ) );

		CSphString sBuf ( sTests[iTest][0] );
		tStripper.Strip ( (BYTE*)sBuf.cstr() );
		assert ( strcmp ( sBuf.cstr(), sTests[iTest][3] )==0 );
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
	strcpy ( sRef, sBuf );

	for ( int iRun=0; iRun<2; iRun++ )
	{
		CSphString sError;
		CSphHTMLStripper tStripper;
		if ( iRun==1 )
			tStripper.SetRemovedElements ( "style, script", sError );

		const int iPasses = 50;
		float fTime = -sphLongTimer ();
		for ( int iPass=0; iPass<iPasses; iPass++ )
		{
			tStripper.Strip ( (BYTE*)sBuf );
			strcpy ( sBuf, sRef );
		}
		fTime += sphLongTimer ();

		printf ( "run %d: %d bytes, %.3f ms, %.3f MB/sec\n", iRun, iLen, 1000.0f*fTime/float(iPasses), iLen*float(iPasses)/fTime/1000000.0f );
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
	tCol.m_sName = "aaa"; tSchema.AddAttr ( tCol );
	tCol.m_sName = "bbb"; tSchema.AddAttr ( tCol );
	tCol.m_sName = "ccc"; tSchema.AddAttr ( tCol );

	CSphMatch tMatch;
	tMatch.m_iDocID = 123;
	tMatch.m_iWeight = 456;
	tMatch.m_iRowitems = tSchema.GetRowSize();
	tMatch.m_pRowitems = new CSphRowitem [ tMatch.m_iRowitems ];
	for ( int i=0; i<tMatch.m_iRowitems; i++ )
		tMatch.m_pRowitems[i] = 1+i;

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

	const int iTests = sizeof(dTests)/sizeof(dTests[0]);
	for ( int iTest=0; iTest<iTests; iTest++ )
	{
		printf ( "testing expression evaluation, test %d/%d... ", 1+iTest, iTests );

		CSphString sError;
		CSphScopedPtr<ISphExpr> pExpr ( sphExprParse ( dTests[iTest].m_sExpr, tSchema, sError ) );
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
}


#if USE_WINDOWS
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif

#define AAA float(tMatch.m_pRowitems[0])
#define BBB float(tMatch.m_pRowitems[1])
#define CCC float(tMatch.m_pRowitems[2])

NOINLINE float ExprNative1 ( const CSphMatch & tMatch )	{ return AAA+BBB*CCC-0.75f;}
NOINLINE float ExprNative2 ( const CSphMatch & tMatch )	{ return AAA+BBB*CCC*2.0f-3.0f/4.0f*5.0f/6.0f*BBB; }
NOINLINE float ExprNative3 ( const CSphMatch & )		{ return (float)sqrt ( 2.0f ); }


void BenchExpr ()
{
	printf ( "benchmarking expressions\n" );

	CSphColumnInfo tCol;
	tCol.m_eAttrType = SPH_ATTR_INTEGER;

	CSphSchema tSchema;
	tCol.m_sName = "aaa"; tSchema.AddAttr ( tCol );
	tCol.m_sName = "bbb"; tSchema.AddAttr ( tCol );
	tCol.m_sName = "ccc"; tSchema.AddAttr ( tCol );

	CSphMatch tMatch;
	tMatch.m_iDocID = 123;
	tMatch.m_iWeight = 456;
	tMatch.m_iRowitems = tSchema.GetRowSize();
	tMatch.m_pRowitems = new CSphRowitem [ tMatch.m_iRowitems ];
	for ( int i=0; i<tMatch.m_iRowitems; i++ )
		tMatch.m_pRowitems[i] = 1+i;

	struct ExprBench_t
	{
		const char *	m_sExpr;
		float			(*m_pFunc)( const CSphMatch & );
	};
	ExprBench_t dBench[] =
	{
		{ "aaa+bbb*(ccc)-0.75",				ExprNative1 },
		{ "aaa+bbb*ccc*2-3/4*5/6*bbb",		ExprNative2 },
		{ "sqrt(2)",						ExprNative3 }
	};

	for ( int iRun=0; iRun<int(sizeof(dBench)/sizeof(dBench[0])); iRun++ )
	{
		printf ( "run %d: ", iRun+1 );

		CSphString sError;
		CSphScopedPtr<ISphExpr> pExpr ( sphExprParse ( dBench[iRun].m_sExpr, tSchema, sError ) );
		if ( !pExpr.Ptr() )
		{
			printf ( "FAILED; %s\n", sError.cstr() );
			return;
		}

		const int NRUNS = 1000000;

		volatile float fValue = 0.0f;
		float fTime = sphLongTimer ();
		for ( int i=0; i<NRUNS; i++ ) fValue += pExpr->Eval(tMatch);
		fTime = sphLongTimer() - fTime;

		float fTimeNative = sphLongTimer ();
		for ( int i=0; i<NRUNS; i++ ) fValue += dBench[iRun].m_pFunc ( tMatch );
		fTimeNative = sphLongTimer() - fTimeNative;

		printf ( "interpreted %.1f Mcalls/sec, native %.1f Mcalls/sec\n",
			float(NRUNS)/float(1000000.0f)/fTime,
			float(NRUNS)/float(1000000.0f)/fTimeNative );
	}
}

//////////////////////////////////////////////////////////////////////////

CSphString ReconstructNode ( const CSphExtendedQueryNode * pNode )
{
	CSphString sRes ( "" );

	if ( pNode->IsPlain() )
	{
		// say just words to me
		const CSphExtendedQueryAtom & tAtom = pNode->m_tAtom;
		const CSphVector<CSphExtendedQueryAtomWord> & dWords = tAtom.m_dWords;
		ARRAY_FOREACH ( i, dWords )
			sRes.SetSprintf ( "%s %s", sRes.cstr(), dWords[i].m_sWord.cstr() );
		sRes.Chop ();

		if ( tAtom.m_bQuorum || tAtom.m_iMaxDistance>0 )
		{
			sRes.SetSprintf ( "\"%s\"%c%d", sRes.cstr(), tAtom.m_bQuorum ? '/' : '~', tAtom.m_iMaxDistance ); // quorum or proximity
		
		} else if ( dWords.GetLength()>1 )
		{
			if ( tAtom.m_iMaxDistance==0 )
				sRes.SetSprintf ( "\"%s\"", sRes.cstr() ); // phrase
			else
				sRes.SetSprintf ( "( %s )", sRes.cstr() ); // just bag of words
		}

	} else
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
		{
			if ( !i )
				sRes = ReconstructNode ( pNode->m_dChildren[i] );
			else
				sRes.SetSprintf ( "%s %s %s", sRes.cstr(), pNode->m_bAny ? "OR" : "AND", ReconstructNode ( pNode->m_dChildren[i] ).cstr() );
		}

		if ( pNode->m_dChildren.GetLength()>1 )
			sRes.SetSprintf ( "( %s )", sRes.cstr() );
	}

	return sRes;
}

CSphString ReconstructQuery ( const CSphExtendedQuery & tQuery )
{
	CSphString sAccept = ReconstructNode ( tQuery.m_pAccept );
	CSphString sReject = ReconstructNode ( tQuery.m_pReject );

	if ( !sReject.IsEmpty () )
		sAccept.SetSprintf ( "( %s ) AND NOT ( %s )", sAccept.cstr(), sReject.cstr() );

	return sAccept;
}

void TestQueryParser ()
{
	CSphString sTmp;

	CSphSchema tSchema;
	CSphColumnInfo tCol;
	tCol.m_sName = "title"; tSchema.m_dFields.Add ( tCol );
	tCol.m_sName = "content"; tSchema.m_dFields.Add ( tCol );

	CSphDictSettings tDictSettings;
	CSphScopedPtr<ISphTokenizer> pTokenizer ( sphCreateSBCSTokenizer () );
	CSphScopedPtr<CSphDict> pDict ( sphCreateDictionaryCRC ( tDictSettings, pTokenizer.Ptr(), sTmp ) );
	assert ( pTokenizer.Ptr() );
	assert ( pDict.Ptr() );

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
		{ "aaa bbb|(ccc ddd)|eee|(fff)",	"( aaa AND ( ( ( bbb OR ( ccc AND ddd ) ) OR eee ) OR fff ) )" },
		{ "aaa bbb|(ccc ddd) eee|(fff)",	"( ( aaa AND ( bbb OR ( ccc AND ddd ) ) ) AND ( eee OR fff ) )" },
		{ "aaa (ccc ddd)|bbb|eee|(fff)",	"( aaa AND ( ( ( ( ccc AND ddd ) OR bbb ) OR eee ) OR fff ) )" },
		{ "aaa (ccc ddd)|bbb eee|(fff)",	"( ( aaa AND ( ( ccc AND ddd ) OR bbb ) ) AND ( eee OR fff ) )" },
		{ "aaa \"bbb ccc\"~5|ddd",			"( aaa AND ( \"bbb ccc\"~5 OR ddd ) )" },
		{ "aaa bbb|\"ccc ddd\"~5",			"( aaa AND ( bbb OR \"ccc ddd\"~5 ) )" },
	};

	int nTests = sizeof(dTest)/sizeof(dTest[0]);
	for ( int i=0; i<nTests; i++ )
	{
		printf ( "testing query parser, test %d/%d... ", i+1, nTests );

		CSphExtendedQuery tQuery;
		sphParseExtendedQuery ( tQuery, dTest[i].m_sQuery, pTokenizer.Ptr(), &tSchema, pDict.Ptr() );

		CSphString sReconst = ReconstructQuery ( tQuery );
		assert ( sReconst==dTest[i].m_sReconst );

		printf ( "ok\n" );
	}
}

//////////////////////////////////////////////////////////////////////////

int main ()
{
	printf ( "RUNNING INTERNAL LIBSPHINX TESTS\n\n" );

#ifdef NDEBUG
	BenchStripper ();
	BenchTokenizer ( false );
	BenchTokenizer ( true );
	BenchExpr ();
#else
	TestQueryParser ();
	TestStripper ();
	TestTokenizer ( false );
	TestTokenizer ( true );
	TestTokenizer ( false, true );
	TestTokenizer ( true, true );
	TestExpr ();
#endif

	unlink ( g_sTmpfile );
	printf ( "\nSUCCESS\n" );
	return 0;
}

//
// $Id$
//
