//
// $Id$
//

//
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
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
#include "json/cJSON.h"
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


void BenchTokenizer ( ISphTokenizer * pTokenizer, BYTE * sData, int iBytes )
{
	const int iPasses = 1000;
	int iTokens = 0;
	CSphVector<int> dTimes;

	// do several benchmark passes
	for ( int iPass=0; iPass<iPasses; iPass++ )
	{
		int64_t tmTime = -sphMicroTimer();
		pTokenizer->SetBuffer ( sData, iBytes );
		while ( pTokenizer->GetToken() )
			iTokens++;
		tmTime += sphMicroTimer();
		dTimes.Add ( (int)tmTime ); // 2 bil usec == 2000 sec, should be enough for one pass
	}
	iTokens /= iPasses;

	// analyse results
	int64_t iMin = INT_MAX, iAvg = 0;
	ARRAY_FOREACH ( i, dTimes )
	{
		if ( dTimes[i]<iMin )
			iMin = dTimes[i];
		iAvg += dTimes[i];
	}

	// report
	printf ( "%d bytes, %d tokens, max %.1f MB/sec, avg %.1f MB/sec\n",
		iBytes, iTokens, double(iBytes)/iMin, double(iBytes*iPasses)/iAvg );
}


void BenchTokenizer ()
{
	printf ( "benchmarking tokenizer\n" );
	if ( !CreateSynonymsFile ( NULL ) )
	{
		printf ( "benchmark failed: error writing temp synonyms file\n" );
		return;
	}

	int iBytes = 0;
	CSphString sError;
	for ( int iRun=1; iRun<=2; iRun++ )
	{
		char * sData = LoadFile ( "./configure", &iBytes, true );

		ISphTokenizer * pTokenizer = sphCreateUTF8Tokenizer ();
		// pTokenizer->SetCaseFolding ( "-, 0..9, A..Z->a..z, _, a..z", sError );
		if ( iRun==2 )
			pTokenizer->LoadSynonyms ( g_sTmpfile, NULL, sError );
		pTokenizer->AddSpecials ( "!-" );

		printf ( "run %d: ", iRun );
		BenchTokenizer ( pTokenizer, (BYTE*)sData, iBytes );
		SafeDeleteArray ( sData );
		SafeDelete ( pTokenizer );
	}

	char * sData = LoadFile ( "./utf8.txt", &iBytes, false );
	if ( sData )
	{
		ISphTokenizer * pTokenizer = sphCreateUTF8Tokenizer ();
		printf ( "run 3: " );
		BenchTokenizer ( pTokenizer, (BYTE*)sData, iBytes );
		SafeDelete ( pTokenizer );
	}
	SafeDeleteArray ( sData );
}

//////////////////////////////////////////////////////////////////////////

void BenchStripper ()
{
	printf ( "benchmarking HTML stripper\n" );

	const char * testfile = "test/bench/sphinx.html";
	FILE * fp = fopen ( testfile, "rb" );
	if ( !fp )
	{
		printf ( "benchmark failed: unable to read %s\n", testfile );
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
	tMatch.m_uDocID = 123;
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
		( a.m_uWordID<b.m_uWordID || \
		( a.m_uWordID==b.m_uWordID && a.m_uDocID<b.m_uDocID ) || \
		( a.m_uWordID==b.m_uWordID && a.m_uDocID==b.m_uDocID && a.m_uWordPos<b.m_uWordPos ) );
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

	ISphTokenizer * pTok = sphCreateUTF8Tokenizer();
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

void BenchAppendf()
{
	int64_t tm1 = sphMicroTimer();
	StringBuilder_c sRes1;
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

int main ()
{
	// threads should be initialized before memory allocations
	char cTopOfMainStack;
	sphThreadInit();
	MemorizeStack ( &cTopOfMainStack );
	setvbuf ( stdout, NULL, _IONBF, 0 );

#if USE_WINDOWS
	SetProcessAffinityMask ( GetCurrentProcess(), 1 );
#endif

	printf ( "RUNNING INTERNAL LIBSPHINX TESTS\n\n" );

#if 0
	BenchSort ();
#endif

#ifdef NDEBUG
	BenchAppendf();
	BenchMisc();
	BenchStripper ();
	BenchTokenizer ();
	BenchExpr ();
	BenchLocators ();
	BenchThreads ();
#endif

	unlink ( g_sTmpfile );
	printf ( "\nSUCCESS\n" );
	return 0;
}

//
// $Id$
//

