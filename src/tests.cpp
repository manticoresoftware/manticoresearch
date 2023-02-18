//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxexpr.h"
#include "sphinxutils.h"
#include "sphinxquery.h"
#include "sphinxrt.h"
#include "sphinxint.h"
#include "threadutils.h"
#include "stripper/html_stripper.h"
#include "tokenizer/charset_definition_parser.h"
#include "searchdaemon.h"
#include "sphinxsort.h"
#include "costestimate.h"
#include "histogram.h"
#include "secondaryindex.h"
#include "secondarylib.h"
#include <cmath>

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


#if _WIN32
#define NOINLINE __declspec(noinline)
#else
#define NOINLINE
#endif


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
				if ( !Threads::Create ( pThd+i, [&] { if (iRun!=1)  MutexBenchThread ( &tMutex ); } ) )
					sphDie ( "failed to create thread (batch %d thd %d)", iBatch, i );

			for ( int i=0; i<BATCH_THREADS; i++ )
				if ( !Threads::Join ( pThd+i ) )
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
		( a.m_uWordID==b.m_uWordID && a.m_tRowID<b.m_tRowID ) || \
		( a.m_uWordID==b.m_uWordID && a.m_tRowID==b.m_tRowID && a.m_uWordPos<b.m_uWordPos ) );
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
	auto iLen = (int) fread ( pRaw, 1, POOLSIZE, fp );
	printf ( "read %d bytes\n", iLen );
	fclose ( fp );

	TokenizerRefPtr_c pTok { Tokenizer::Detail::CreateUTF8Tokenizer() };
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

	SafeDeleteArray ( pRaw );
}

//////////////////////////////////////////////////////////////////////////


static CSphVector<int64_t> GetFilterEstimates ( const CSphQuery & tQuery, CSphIndex * pIndex )
{
	assert ( pIndex );

	auto pHistograms = pIndex->Debug_GetHistograms();
	assert ( pHistograms );

	CSphVector<int64_t> dRes;
	for ( const auto & i : tQuery.m_dFilters )
	{
		auto pHistogram = pHistograms->Get ( i.m_sAttrName );
		assert ( pHistogram );
		HistogramRset_t tEstimate;
		Verify ( pHistogram->EstimateRsetSize ( i, tEstimate ) );
		dRes.Add ( tEstimate.m_iTotal );
	}

	return dRes;
}


static float GetEstimatedCost ( CSphQuery & tQuery, CSphIndex * pIndex, SecondaryIndexType_e eType )
{
	auto tStats = pIndex->GetStats();
	CSphVector<int64_t> dFilterRsetEstimates = GetFilterEstimates ( tQuery, pIndex );
	const CSphVector<SecondaryIndexInfo_t> dSIInfo ( dFilterRsetEstimates.GetLength() );
	ARRAY_FOREACH ( i, dSIInfo )
	{
		dSIInfo[i].m_iRsetEstimate = dFilterRsetEstimates[i];
		dSIInfo[i].m_eType = eType;
	}

	bool bImplicitCutoff;
	int iCutoff;
	std::tie ( bImplicitCutoff, iCutoff ) = ApplyImplicitCutoff ( tQuery, {} );
	SelectIteratorCtx_t tCtx ( tQuery.m_dFilters, tQuery.m_dFilterTree, tQuery.m_dIndexHints, pIndex->GetMatchSchema(), pIndex->Debug_GetHistograms(), pIndex->GetColumnar(), pIndex->Debug_GetSI(), tQuery.m_eCollation, iCutoff, tStats.m_iTotalDocuments, 1 );
	std::unique_ptr<CostEstimate_i> pEstimate ( CreateCostEstimate ( dSIInfo, tCtx ) );
	return pEstimate->CalcQueryCost();
}


static int RunGenericQuery ( CSphQuery & tQuery, CSphIndex * pIndex, int64_t & iTime, uint64_t & uHash )
{
	auto pParser = sphCreatePlainQueryParser();
	tQuery.m_pQueryParser = pParser.get();

	AggrResult_t tResult;
	CSphQueryResult tQueryResult;
	tQueryResult.m_pMeta = &tResult;
	CSphMultiQueryArgs tArgs ( 1 );

	SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema() );
	tQueueSettings.m_bComputeItems = true;
	SphQueueRes_t tRes;
	CSphString sError;
	std::unique_ptr<ISphMatchSorter> pSorter ( sphCreateQueue ( tQueueSettings, tQuery, sError, tRes ) );
	if ( !pSorter )
		sphDie ( "failed to create sorter; error=%s", tResult.m_sError.cstr() );

	int64_t iLocalTime = sphMicroTimer();
	auto pRawSorter = pSorter.get();
	if ( !pIndex->MultiQuery ( tQueryResult, tQuery, { &pRawSorter, 1 }, tArgs ) )
		sphDie ( "query failed; error=%s", pIndex->GetLastError().cstr() );
	iTime += sphMicroTimer()-iLocalTime;

	int64_t iTotal = pRawSorter->GetTotalCount();

	auto & tOneRes = tResult.m_dResults.Add ();
	tOneRes.FillFromSorter ( pRawSorter );
	ARRAY_FOREACH ( i, tOneRes.m_dMatches )
	{
		auto tRowID = tOneRes.m_dMatches[i].m_tRowID;
		uHash = sphFNV64 ( &tRowID, sizeof(tRowID), uHash );
	}

	return iTotal;
}


static int RunGenericQuery ( CSphQuery & tQuery, CSphIndex * pIndex, int64_t & iTime, uint64_t & uHash, int iRepeats )
{
	iTime = 0;
	int64_t iRsetItems = 0;
	const int WARMUP_RUNS = 2;
	for ( auto i = 0; i < iRepeats; i++ )
	{
		iRsetItems = RunGenericQuery ( tQuery, pIndex, iTime, uHash );
		if ( i < WARMUP_RUNS )
			iTime = 0;
	}

	iTime /= iRepeats-WARMUP_RUNS;
	return iRsetItems;
}


static void PrintStats ( const char * szHeader, int64_t iTime, float fEstimatedCost, uint64_t uHash )
{
	printf ( "\n%s\n", szHeader );
	printf ( "\t%d msec, estimated=%.3f\n", (int)( iTime/1000 ), fEstimatedCost );
	printf ( "\thash=" UINT64_FMT "\n", uHash );
}


static void IgnoreAll ( CSphQuery & tQuery, const char * szName )
{
	tQuery.m_dIndexHints.Add ( { szName, SecondaryIndexType_e::INDEX, false } );
	tQuery.m_dIndexHints.Add ( { szName, SecondaryIndexType_e::ANALYZER, false } );
	tQuery.m_dIndexHints.Add ( { szName, SecondaryIndexType_e::LOOKUP, false } );
}


static void ForceColumnar ( CSphQuery & tQuery, const char * szName )
{
	tQuery.m_dIndexHints.Add ( { szName, SecondaryIndexType_e::ANALYZER, true } );
}


static void ForceSI( CSphQuery & tQuery, const char * szName )
{
	tQuery.m_dIndexHints.Add ( { szName, SecondaryIndexType_e::INDEX, true } );
}


static void ForceLookup( CSphQuery & tQuery, const char * szName )
{
	tQuery.m_dIndexHints.Add ( { szName, SecondaryIndexType_e::LOOKUP, true } );
}


static CSphQuery CreateFullscanQuery ()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	return tQuery;
}


static CSphQuery CreateFullscanQuery1()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );
	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_iMaxValue = 5;
		tFilter.m_bOpenLeft = true;
	}

	IgnoreAll ( tQuery, "comment_ranking" );
	return tQuery;
}


static CSphQuery CreateFullscanQuery2()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );
	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_iMinValue = 1000;
		tFilter.m_bOpenRight = true;
	}

	IgnoreAll ( tQuery, "comment_ranking" );
	return tQuery;
}


static CSphQuery CreateFullscanQuery3()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );
	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_iMinValue = 50;
		tFilter.m_bOpenRight = true;
	}
	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "story_comment_count";
		tFilter.m_iMinValue = 50;
		tFilter.m_bOpenRight = true;
	}
	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "author_comment_count";
		tFilter.m_iMinValue = 500;
		tFilter.m_bOpenRight = true;
	}

	IgnoreAll ( tQuery, "comment_ranking" );
	IgnoreAll ( tQuery, "story_comment_count" );
	IgnoreAll ( tQuery, "author_comment_count" );

	return tQuery;
}


static CSphQuery CreateFullscanQuery4()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );
	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_iMaxValue = 5;
		tFilter.m_bOpenLeft = true;
	}

	tQuery.m_sSortBy = "story_comment_count asc";
	IgnoreAll ( tQuery, "comment_ranking" );

	return tQuery;
}


static void CalcCoeffsInit()
{
	CSphString sError;
	if ( !sphInitCharsetAliasTable(sError) )
		sphDie ( "failed to init charset alias table: %s", sError.cstr() );

	if ( !InitColumnar(sError) )
		sphDie ( "failed to init MCL: %s", sError.cstr() );

	if ( !InitSecondary ( sError ) )
		sphDie ( "failed to init SI: %s", sError.cstr() );
}


static const float SCALE = 1000.0f;

static void CalcCoeffsRowwise()
{
	CSphString sError, sWarning;
	if ( !sphInitCharsetAliasTable ( sError ) )
		sphDie ( "failed to init charset alias table: %s", sError.cstr() );

	CSphString sPath = "hackernews_10x";
	std::unique_ptr<CSphIndex> pIndex = sphCreateIndexPhrase ( sPath, sPath );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );

	pIndex->Preread();

	auto tStats = pIndex->GetStats();
	int64_t iTotalDocs = tStats.m_iTotalDocuments;

	float fCostOfPush = 0.0f;
	float fCostOfFilter = 0.0f;

	const int REPEATS = 20;

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;
		CSphQuery tQuery = CreateFullscanQuery();

		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fCost = float(iTime) / iRsetItems*SCALE;
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::FILTER );
		fCostOfPush = fCost;

		PrintStats ( "Fullscan, 0 filters", iTime, fEstimatedCost, uHash );
		printf ( "\tCOST_PUSH=%.3f\n", fCost );
	}

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;
		CSphQuery tQuery = CreateFullscanQuery1();

		// calculate the cost of single filter
		// run a query that doesn't early reject any blocks of attributes
		// FIXME! depends on the dataset
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fTotalCost = float(iTime)*SCALE;
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::FILTER );
		fCostOfFilter = ( fTotalCost - fCostOfPush*iRsetItems )/( float(iTotalDocs)/DOCINFO_INDEX_FREQ + iTotalDocs );

		PrintStats ( "Fullscan, 1 non-selective filter", iTime, fEstimatedCost, uHash );
		printf ( "\tCOST_FILTER=%.3f\n", fCostOfFilter );
	}

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;
		CSphQuery tQuery = CreateFullscanQuery2();

		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::FILTER );

		PrintStats ( "Fullscan, 1 selective filter", iTime, fEstimatedCost, uHash );
	}

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;
		CSphQuery tQuery = CreateFullscanQuery3();

		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::FILTER );

		PrintStats ( "Fullscan, 3 filters", iTime, fEstimatedCost, uHash );
	}

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;
		CSphQuery tQuery = CreateFullscanQuery4();

		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::FILTER );

		PrintStats ( "Fullscan, 1 non-selective filter and sorting by 1 attribute", iTime, fEstimatedCost, uHash );
	}
}


static CSphQuery CreateFullscanQueryC1()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"comment_ranking","comment_ranking"} );
	tQuery.m_sSortBy = "story_comment_count asc";

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_iMaxValue = 0;
		tFilter.m_bOpenLeft = true;
	}

	ForceColumnar ( tQuery, "comment_ranking" );
	return tQuery;
}


static CSphQuery CreateFullscanQueryC1b()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "story_comment_count";
		tFilter.m_iMinValue = 1000;
		tFilter.m_bOpenRight = true;
	}

	ForceColumnar ( tQuery, "comment_ranking" );
	return tQuery;
}


static CSphQuery CreateFullscanQueryC2()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_iMinValue = 0;
		tFilter.m_bOpenRight = true;
	}

	ForceColumnar ( tQuery, "comment_ranking" );
	return tQuery;
}


static CSphQuery CreateFullscanQueryC3()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_iMinValue = 50;
		tFilter.m_bOpenRight = true;
	}
	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "story_comment_count";
		tFilter.m_iMinValue = 50;
		tFilter.m_bOpenRight = true;
	}
	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "author_comment_count";
		tFilter.m_iMinValue = 500;
		tFilter.m_bOpenRight = true;
	}

	ForceColumnar ( tQuery, "comment_ranking" );
	ForceColumnar ( tQuery, "story_comment_count" );
	ForceColumnar ( tQuery, "author_comment_count" );

	return tQuery;
}

class RowIdIteratorStub_c : public RowidIterator_i
{
public:
	RowIdIteratorStub_c()
	{
		m_dValues.Resize ( rand()*150 );
		for ( auto & i : m_dValues )
			i = ( rand()*100 ) + ( rand() % 100 );

		m_dValues.Uniq();
	}

	bool	HintRowID ( RowID_t tRowID ) override { return true; }
	int64_t	GetNumProcessed() const override { return 0; }
	void	SetCutoff ( int iCutoff ) override {}
	bool	WasCutoffHit() const override { return 0; }
	void	AddDesc ( CSphVector<IteratorDesc_t> & dDesc ) const override {}
	int		GetNumValues() const { return m_dValues.GetLength(); }

	bool GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock ) override
	{
		if ( m_iOffset>=m_dValues.GetLength() )
			return false;

		RowID_t * pStart = m_dValues.Begin() + m_iOffset;
		RowID_t * pEnd = Min ( pStart + OFFSET_STEP, m_dValues.End() );
		m_iOffset += OFFSET_STEP;
		return ReturnIteratorResult ( pEnd, pStart, dRowIdBlock );
	}

private:
	CSphVector<RowID_t> m_dValues;
	int m_iOffset = 0;
	static const int OFFSET_STEP = 128;
};


static int64_t RunIntersectIterator ( int64_t & iTotalValues, uint64_t & uHash )
{
	srand(0);

	int64_t iTotalTime = 0;
	const int REPEATS = 10;
	for ( int i = 0; i < REPEATS; i++ )
	{
		const int NUM_ITERATORS = 3;
		CSphVector<RowidIterator_i *> dIterators(NUM_ITERATORS);
		for ( auto & i : dIterators )
		{
			auto pStub = new RowIdIteratorStub_c;
			iTotalValues += pStub->GetNumValues();
			i = pStub;
		}

		int64_t iTime = sphMicroTimer();
		std::unique_ptr<RowidIterator_i> pIterator ( CreateIteratorIntersect ( dIterators, nullptr ) );
		RowIdBlock_t dRowIdBlock;
		while ( pIterator->GetNextRowIdBlock ( dRowIdBlock ) )
			for ( auto i : dRowIdBlock )
				uHash = sphFNV64 ( &i, sizeof(i), uHash );

		iTotalTime += sphMicroTimer() - iTime;
	}

	return iTotalTime;
}


static void CalcCoeffsColumnar()
{
	CSphString sPath = "hackernews_col10x";
	std::unique_ptr<CSphIndex> pIndex = sphCreateIndexPhrase ( sPath, sPath );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );

	pIndex->Preread();

	const int REPEATS = 20;

	float fCostOfPush = 0.0f;

	// pure scan; should be the same speed as rowwise
	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQuery();
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		fCostOfPush = float(iTime) / iRsetItems*SCALE;
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::ANALYZER );

		PrintStats ( "Columnar fullscan, 0 filters", iTime, fEstimatedCost, uHash );
	}

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryC1();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::ANALYZER );

		PrintStats ( "Columnar fullscan, 1 selective filter", iTime, fEstimatedCost, uHash );
	}

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryC1b();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::ANALYZER );

		PrintStats ( "Columnar fullscan, 1 extremely selective filter", iTime, fEstimatedCost, uHash );
	}

	
	// determine analyzer read cost
	// scan with sorting by 1 attribute
	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryC2();
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );

		float fTotalCost = float(iTime)*SCALE;
		float fCostOfFilter = ( fTotalCost - fCostOfPush*iRsetItems )/iRsetItems;
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::ANALYZER );

		PrintStats ( "Columnar fullscan, 1 filter", iTime, fEstimatedCost, uHash );
		printf ( "\tCOST_COLUMNAR_FILTER=%.3f\n", fCostOfFilter );
	}


	{
		uint64_t uHash = SPH_FNV64_SEED;
		int64_t iTotalValues = 0;
		int64_t iTime = RunIntersectIterator ( iTotalValues, uHash );
		float fCostPerValue = float(iTime) / float(iTotalValues)*SCALE;
		printf ( "\nIntersect iterator\n" );
		printf ( "\tCOST_INTERSECT=%.3f\n", fCostPerValue );
		printf ( "\thash=" UINT64_FMT "\n", uHash );
	}

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryC3();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::ANALYZER );

		PrintStats ( "Columnar fullscan, 3 filters", iTime, fEstimatedCost, uHash );
	}
}


static CSphQuery CreateFullscanQueryC2_1()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_VALUES;
		tFilter.m_sAttrName = "a_mva";
		tFilter.m_dValues.Add(2652701560);
		tFilter.m_dValues.Add(3800433230);
		tFilter.m_eMvaFunc = SPH_MVAFUNC_ANY;
	}

	ForceColumnar ( tQuery, "a_mva" );
	return tQuery;
}


static CSphQuery CreateFullscanQueryC2_2()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "a_int";
		tFilter.m_iMinValue = 100;
		tFilter.m_bOpenRight = true;
		tFilter.m_eMvaFunc = SPH_MVAFUNC_ANY;
	}

	ForceColumnar ( tQuery, "a_int" );
	return tQuery;
}


static void CalcCoeffsColumnar2()
{
	CSphString sPath = "synth";
	std::unique_ptr<CSphIndex> pIndex = sphCreateIndexPhrase ( sPath, sPath );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );

	pIndex->Preread();

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryC2_1();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, 100 );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::ANALYZER );

		PrintStats ( "Columnar fullscan, mva filter of 2 values", iTime, fEstimatedCost, uHash );
	}

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryC2_2();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, 100 );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::ANALYZER );

		PrintStats ( "Columnar fullscan, nonselective int filter; generic encoding", iTime, fEstimatedCost, uHash );
	}
}


static CSphQuery CreateFullscanQueryC3_1()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_STRING;
		tFilter.m_sAttrName = "pickup_boroname";
		tFilter.m_dStrings.Add("0");
		tFilter.m_bExclude = true;
	}

	ForceColumnar ( tQuery, "pickup_boroname" );
	return tQuery;
}


static CSphQuery CreateFullscanQueryC3_2()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "id";
		tFilter.m_iMinValue = 1000;
		tFilter.m_bOpenRight = true;
	}

	ForceColumnar ( tQuery, "id" );
	return tQuery;
}


static CSphQuery CreateFullscanQueryC3_3()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_STRING;
		tFilter.m_sAttrName = "pickup_ntaname";
		tFilter.m_dStrings.Add("Upper West Side");
		tFilter.m_bExclude = true;
	}

	ForceColumnar ( tQuery, "pickup_ntaname" );
	return tQuery;
}


static void CalcCoeffsColumnar3()
{
	CSphString sPath = "taxi1";
	std::unique_ptr<CSphIndex> pIndex = sphCreateIndexPhrase ( "taxi1", sPath.cstr() );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );

	pIndex->Preread();

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryC3_1();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, 10 );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::ANALYZER );

		PrintStats ( "Columnar fullscan, string(hash) filter of 1 value; table encoding", iTime, fEstimatedCost, uHash );
	}

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryC3_2();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, 10 );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::ANALYZER );

		PrintStats ( "Columnar fullscan, int range filter; generic encoding", iTime, fEstimatedCost, uHash );
	}

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryC3_3();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, 10 );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::ANALYZER );

		PrintStats ( "Columnar fullscan, string(hash) filter of 1 value; table encoding", iTime, fEstimatedCost, uHash );
	}
}


static CSphQuery CreateFullscanQueryS1 ( int iValue )
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_VALUES;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_dValues.Add(iValue);
		tFilter.m_bOpenRight = true;
	}

	ForceSI ( tQuery, "comment_ranking" );
	return tQuery;
}


static CSphQuery CreateFullscanQueryS2()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_iMinValue = 0;
		tFilter.m_iMaxValue = 20;
		tFilter.m_bHasEqualMin = true;
		tFilter.m_bHasEqualMax = false;
	}

	ForceSI ( tQuery, "comment_ranking" );
	return tQuery;
}


static CSphQuery CreateFullscanQueryS3 ( int iMin, int iMax )
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_iMinValue = iMin;
		tFilter.m_iMaxValue = iMax;
		tFilter.m_bHasEqualMin = true;
		tFilter.m_bHasEqualMax = false;
	}

	ForceSI ( tQuery, "comment_ranking" );
	return tQuery;
}

static CSphQuery CreateFullscanQueryS4()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "comment_ranking";
		tFilter.m_iMaxValue = 10;
		tFilter.m_bOpenLeft = true;
	}

	ForceSI ( tQuery, "comment_ranking" );
	return tQuery;
}


float g_fSICostOfPush = 0.0f;

// internal value; must be in sync with costestimate.cpp
const float COST_INDEX_ITERATOR_INIT = 200.0f;

static void CalcCoeffsSI()
{
	CSphString sPath = "hackernews_10x";
	std::unique_ptr<CSphIndex> pIndex = sphCreateIndexPhrase ( sPath, sPath );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );

	pIndex->Preread();

	float fCostOfIndexReadSingle = 0.0f;
	float fCostOfIndexReadDense = 0.0f;

	// pure scan; should be the same speed as rowwise/columnar
	{
		const int REPEATS = 20;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQuery();
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		g_fSICostOfPush = float(iTime) / iRsetItems*SCALE;
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		PrintStats ( "SI fullscan, 0 filters", iTime, fEstimatedCost, uHash );
	}

	{
		const int REPEATS = 200;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS1(10);
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );

		float fTotalCost = float(iTime)*SCALE;
		fCostOfIndexReadSingle = fTotalCost/iRsetItems - g_fSICostOfPush;
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		PrintStats ( "SI fullscan, 1 non-selective filter of 1 value", iTime, fEstimatedCost, uHash );
		printf ( "\tCOST_INDEX_READ_SINGLE=%.3f\n", fCostOfIndexReadSingle );
	}

	{
		const int REPEATS = 50;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS4();
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		// we have 11 iterators and 500k rset values
		const int NUM_ITERATORS=11;

		float fTotalCost = float(iTime)*SCALE - COST_INDEX_ITERATOR_INIT*NUM_ITERATORS;
		fCostOfIndexReadDense = fTotalCost/iRsetItems - g_fSICostOfPush;

		PrintStats ( "SI fullscan, range filter of 500k values", iTime, fEstimatedCost, uHash );
		printf ( "\tCOST_INDEX_READ_DENSE=%.3f\n", fCostOfIndexReadDense );
	}

	{
		const int REPEATS = 200;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS1(500);
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		PrintStats ( "SI fullscan, 1 selective filter of 1 value", iTime, fEstimatedCost, uHash );
	}

	{
		const int REPEATS = 50;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS2();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );
		PrintStats ( "SI fullscan, range filter of 20 values (bitmap union)", iTime, fEstimatedCost, uHash );
	}

	{
		const int REPEATS = 50;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS3 ( 0, 5 );
		int64_t iRangeValues = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		float fTotalCostWUnion = float(iTime)*SCALE;
		float fTotalCostWoUnion = iRangeValues * ( fCostOfIndexReadSingle + g_fSICostOfPush );
		float fUnionCost = ( fTotalCostWUnion-fTotalCostWoUnion );
		float fNLogN = iRangeValues*log2f(iRangeValues);
		float fCoeff = fNLogN/fUnionCost;

		PrintStats ( "SI fullscan, range filter of 5 values (queue union)", iTime, fEstimatedCost, uHash );
		printf ( "\tunion coeff=%.3f, values=%d\n", fCoeff, (int)iRangeValues );
	}

	{
		const int REPEATS = 50;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS3 ( 25, 30 );
		int64_t iRangeValues = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		float fTotalCostWUnion = float(iTime)*SCALE;
		float fTotalCostWoUnion = iRangeValues * ( fCostOfIndexReadSingle + g_fSICostOfPush );
		float fUnionCost = ( fTotalCostWUnion-fTotalCostWoUnion );
		float fNLogN = iRangeValues*log2f(iRangeValues);
		float fCoeff = fNLogN/fUnionCost;

		PrintStats ( "SI fullscan, range filter of 5 values (queue union)", iTime, fEstimatedCost, uHash );
		printf ( "\tunion coeff=%.3f, values=%d\n", fCoeff, (int)iRangeValues );
	}

	{
		const int REPEATS = 50;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS3 ( 50, 55 );
		int64_t iRangeValues = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		float fTotalCostWUnion = float(iTime)*SCALE;
		float fTotalCostWoUnion = iRangeValues * ( fCostOfIndexReadSingle + g_fSICostOfPush );
		float fUnionCost = ( fTotalCostWUnion-fTotalCostWoUnion );
		float fNLogN = iRangeValues*log2f(iRangeValues);
		float fCoeff = fNLogN/fUnionCost;

		PrintStats ( "SI fullscan, range filter of 5 values (queue union)", iTime, fEstimatedCost, uHash );
		printf ( "\tunion coeff=%.3f, values=%d\n", fCoeff, (int)iRangeValues );
	}
}


static CSphQuery CreateFullscanQueryS2_1()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "a_big_super_lc";
		tFilter.m_iMinValue = 1000000000;
		tFilter.m_bOpenRight = true;
	}

	ForceSI ( tQuery, "a_big_super_lc" );
	return tQuery;
}


static CSphQuery CreateFullscanQueryS2_2()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "a_big";
		tFilter.m_iMinValue = 1000000000;
		tFilter.m_bOpenRight = true;
	}

	ForceSI ( tQuery, "a_big" );
	return tQuery;
}


static void CalcCoeffsSI2()
{
	CSphString sPath = "synth";
	std::unique_ptr<CSphIndex> pIndex = sphCreateIndexPhrase ( sPath, sPath );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );

	pIndex->Preread();

	float fCostOfIndexRead = 0.0f;
	{
		const int REPEATS = 10;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS2_1();
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );

		// we have about 5k iterators and 500k rset values
		// so each iterator fetches ~100 values
		// index read speed here is a lot slower
		float fTotalCost = float(iTime)*SCALE - COST_INDEX_ITERATOR_INIT*5000;
		fCostOfIndexRead = fTotalCost/iRsetItems - g_fSICostOfPush;
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		PrintStats ( "SI fullscan, 1 non-selective filter of 1 value", iTime, fEstimatedCost, uHash );
		printf ( "\tCOST_INDEX_READ_SPARSE=%.3f\n", fCostOfIndexRead );
	}

	{
		const int REPEATS = 10;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS2_2();
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		// we have about ~500k iterators and ~500k rset values in this query
		// we don't have direct access to SI so we just hardcode it
		const int NUM_ITERATORS = 500000;
		float fCostOfIndexInit = ( float(iTime)*SCALE - ( g_fSICostOfPush + fCostOfIndexRead )*iRsetItems ) / NUM_ITERATORS;

		PrintStats ( "SI fullscan, 500k iterators", iTime, fEstimatedCost, uHash );
		printf ( "\tCOST_INDEX_ITERATOR_INIT=%.3f\n", fCostOfIndexInit );
	}
}


static CSphQuery CreateFullscanQueryS3_1()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_VALUES;
		tFilter.m_sAttrName = "a_mva";
		tFilter.m_dValues.Add(2652701560);
		tFilter.m_dValues.Add(3800433230);
		tFilter.m_eMvaFunc = SPH_MVAFUNC_ANY;
	}

	ForceSI ( tQuery, "a_mva" );
	return tQuery;
}


static void CalcCoeffsSI3()
{
	// this is supposed to be columnar synth
	CSphString sPath = "synth";
	std::unique_ptr<CSphIndex> pIndex = sphCreateIndexPhrase ( sPath, sPath );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );

	pIndex->Preread();

	{
		const int REPEATS = 1000;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS3_1();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		PrintStats ( "SI fullscan, any(mva) of 2 values", iTime, fEstimatedCost, uHash );
	}
}


static CSphQuery CreateFullscanQueryS4_1()
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_STRING;
		tFilter.m_sAttrName = "pickup_ntaname";
		tFilter.m_dStrings.Add("Upper West Side");
	}

	ForceSI ( tQuery, "pickup_ntaname" );
	return tQuery;
}

static void CalcCoeffsSI4()
{
	CSphString sPath = "taxi6";
	std::unique_ptr<CSphIndex> pIndex = sphCreateIndexPhrase ( "taxi", sPath.cstr() );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );

	pIndex->Preread();

	{
		const int REPEATS = 100;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryS4_1();
		RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::INDEX );

		PrintStats ( "SI fullscan, string filter of 1 value", iTime, fEstimatedCost, uHash );
	}
}


static CSphQuery CreateFullscanQueryL1 ( int iMin, int iMax )
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_dItems.Add ( {"id","id"} );

	{
		auto & tFilter = tQuery.m_dFilters.Add();
		tFilter.m_eType = SPH_FILTER_RANGE;
		tFilter.m_sAttrName = "id";
		tFilter.m_iMinValue = iMin;
		tFilter.m_iMaxValue = iMax;
		tFilter.m_bHasEqualMin = true;
		tFilter.m_bHasEqualMax = false;
	}

	ForceLookup ( tQuery, "id" );
	return tQuery;
}


static void CalcCoeffsLookup()
{
	CSphString sPath = "hackernews_10x";
	std::unique_ptr<CSphIndex> pIndex = sphCreateIndexPhrase ( sPath, sPath );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );

	pIndex->Preread();

	float fCostOfPush = 0.0f;

	// pure scan; should be the same speed as any other
	{
		const int REPEATS = 20;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQuery();
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		fCostOfPush = float(iTime) / iRsetItems*SCALE;
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::LOOKUP );

		PrintStats ( "Lookup fullscan, 0 filters", iTime, fEstimatedCost, uHash );
	}

	{
		const int REPEATS = 200;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFullscanQueryL1(1,200000);
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::LOOKUP );
		float fCostOfLookupRead = float(iTime) / iRsetItems*SCALE - fCostOfPush;

		PrintStats ( "Lookup fullscan, 1 range filter of 200k values", iTime, fEstimatedCost, uHash );
		printf ( "\tCOST_LOOKUP_READ=%.3f\n", fCostOfLookupRead );
		printf ( "\tvalues=%d\n", (int)iRsetItems );
	}
}


static CSphQuery CreateFTTermQuery ( ESphRankMode eRanker, const char * szTerm )
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_sQuery = szTerm;
	tQuery.m_eRanker = eRanker;
	return tQuery;
}


static CSphQuery CreateFTQueryOR ( ESphRankMode eRanker )
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_sQuery = "hello | world";
	tQuery.m_eRanker = eRanker;
	return tQuery;
}


static CSphQuery CreateFTQueryAND ( ESphRankMode eRanker )
{
	CSphQuery tQuery;
	tQuery.m_iCutoff = 0;
	tQuery.m_sQuery = "hello world";
	tQuery.m_eRanker = eRanker;
	return tQuery;
}


static void CalcCoeffsFT()
{
	CSphString sPath = "hn";
	std::unique_ptr<CSphIndex> pIndex = sphCreateIndexPhrase ( sPath, sPath );
	StrVec_t dWarnings;
	if ( !pIndex->Prealloc ( false, nullptr, dWarnings ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );

	pIndex->Preread();

	float fCostOfPush = 0.0f;

	{
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;
		CSphQuery tQuery = CreateFullscanQuery();

		const int REPEATS = 20;
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fCost = float(iTime) / iRsetItems*SCALE;
		float fEstimatedCost = GetEstimatedCost ( tQuery, pIndex.get(), SecondaryIndexType_e::FILTER );
		fCostOfPush = fCost;

		PrintStats ( "Fullscan, 0 filters", iTime, fEstimatedCost, uHash );
		printf ( "\tCOST_PUSH=%.3f\n", fCost );
	}

	float fCostOfRankerNone = 0.0f;
	int64_t iRawTimeHello = 0;
	{
		const int REPEATS = 100;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFTTermQuery ( SPH_RANK_NONE, "hello" );
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		fCostOfRankerNone = float(iTime) / iRsetItems*SCALE - fCostOfPush;
		iRawTimeHello = iTime - int64_t(fCostOfPush/SCALE*iRsetItems);

		PrintStats ( "FT term, ranker=none", iTime, 0.0f, uHash );
		printf ( "\tCOST_FT_TERM=%.3f\n", fCostOfRankerNone );
	}

	int64_t iRawTimeWorld = 0;
	{
		const int REPEATS = 100;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFTTermQuery ( SPH_RANK_NONE, "world" );
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		fCostOfRankerNone = float(iTime) / iRsetItems*SCALE - fCostOfPush;
		iRawTimeWorld = iTime - int64_t(fCostOfPush/SCALE*iRsetItems);

		PrintStats ( "FT term, ranker=none", iTime, 0.0f, uHash );
		printf ( "\tCOST_FT_TERM=%.3f\n", fCostOfRankerNone );
	}

	{
		const int REPEATS = 100;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFTTermQuery ( SPH_RANK_DEFAULT, "hello" );
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fCost = float(iTime) / iRsetItems*SCALE - fCostOfRankerNone;

		PrintStats ( "FT term, ranker=default", iTime, 0.0f, uHash );
		printf ( "\tCOST_RANKER_DEFAULT=%.3f\n", fCost );
	}

	{
		const int REPEATS = 100;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFTTermQuery ( SPH_RANK_PROXIMITY, "hello" );
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fCost = float(iTime) / iRsetItems*SCALE - fCostOfRankerNone;

		PrintStats ( "FT term, ranker=proximity", iTime, 0.0f, uHash );
		printf ( "\tCOST_RANKER_PROXIMITY=%.3f\n", fCost );
	}

	{
		const int REPEATS = 100;
		int64_t iTime = 0;
		uint64_t uHash = SPH_FNV64_SEED;

		CSphQuery tQuery = CreateFTQueryOR ( SPH_RANK_NONE );
		int64_t iRsetItems = RunGenericQuery ( tQuery, pIndex.get(), iTime, uHash, REPEATS );
		float fCost = float(iTime - iRawTimeHello - iRawTimeWorld ) / iRsetItems*SCALE;

		PrintStats ( "OR query, ranker=none", iTime, 0.0f, uHash );
		printf ( "\tCOST_QUERY_OR=%.3f\n", fCost );
	}
}


//////////////////////////////////////////////////////////////////////////
// BLOODY DIRTY HACK!!!
// definitions of AggrResult_t members just copy-pasted 'as is' from searchdaemon.cpp

int AggrResult_t::GetLength () const
{
	int iCount = 0;
	m_dResults.Apply ( [&iCount] ( const OneResultset_t & a ) { iCount += a.m_dMatches.GetLength (); } );
	return iCount;
}

void AggrResult_t::ClampMatches ( int iLimit )
{
	assert ( m_bSingle ); 
	if ( !m_dResults.IsEmpty () )
		m_dResults.First ().ClampMatches ( iLimit );
}

void AggrResult_t::ClampAllMatches ()
{
	for ( auto& dResult : m_dResults )
		dResult.ClampAllMatches();
}

int OneResultset_t::FillFromSorter ( ISphMatchSorter * pQueue )
{
	if ( !pQueue )
		return 0;

	assert ( m_dMatches.IsEmpty () );
	m_tSchema = *pQueue->GetSchema ();
	if ( !pQueue->GetLength () )
		return 0;

	int iCopied = pQueue->Flatten ( m_dMatches.AddN ( pQueue->GetLength () ) );
	m_dMatches.Resize ( iCopied );
	return iCopied;
}

void OneResultset_t::ClampAllMatches ()
{
	for ( auto& dMatch : m_dMatches )
		m_tSchema.FreeDataPtrs ( dMatch );
	m_dMatches.Reset();
}

void OneResultset_t::ClampMatches ( int iLimit )
{
	assert ( iLimit>0 );

	int iMatches = m_dMatches.GetLength ();
	for ( int i = iLimit; i<iMatches; ++i )
		m_tSchema.FreeDataPtrs ( m_dMatches[i] );
	m_dMatches.Resize ( iLimit );
}

OneResultset_t::~OneResultset_t()
{
	for ( auto & dMatch : m_dMatches )
		m_tSchema.FreeDataPtrs ( dMatch );
}

//////////////////////////////////////////////////////////////////////////

int main ()
{
	// threads should be initialized before memory allocations
	char cTopOfMainStack;
	Threads::Init();
	Threads::PrepareMainThread ( &cTopOfMainStack );
	setvbuf ( stdout, NULL, _IONBF, 0 );

#if _WIN32
	SetProcessAffinityMask ( GetCurrentProcess(), 1 );
#endif

	printf ( "RUNNING INTERNAL LIBSPHINX TESTS\n\n" );

#if 0
	SetMaxChildrenThreads(1)0
	WipeGlobalSchedulerOnShutdownAndFork()0
	StartGlobalWorkPool()0

	Threads::CallCoroutine ( [&] 0

	CalcCoeffsInit()0
	CalcCoeffsRowwise()0
	CalcCoeffsColumnar();
	CalcCoeffsColumnar2();
	CalcCoeffsColumnar3();
	CalcCoeffsSI();
	CalcCoeffsSI2();
	CalcCoeffsSI3();
	CalcCoeffsSI4();
	CalcCoeffsLookup();

	CalcCoeffsFT();
	} );
#endif

	printf ( "\nSUCCESS\n" );
	return 0;
}
