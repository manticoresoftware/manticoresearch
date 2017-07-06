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
#include "sphinxrt.h"
#include "sphinxutils.h"

#if USE_WINDOWS
#include "psapi.h"
#pragma comment(linker, "/defaultlib:psapi.lib")
#pragma message("Automatically linking with psapi.lib")
#endif

int			COMMIT_STEP = 1;
float		g_fTotalMB = 0.0f;

void SetupIndexing ( CSphSource_MySQL * pSrc, const CSphSourceParams_MySQL & tParams )
{
	CSphString sError;
	if ( !pSrc->Setup ( tParams ) )
		sphDie ( "setup failed" );
	if ( !pSrc->Connect ( sError ) )
		sphDie ( "connect failed: %s", sError.cstr() );
	if ( !pSrc->IterateStart ( sError ) )
		sphDie ( "iterate-start failed: %s", sError.cstr() );
}


void DoSearch ( CSphIndex * pIndex )
{
	printf ( "---\nsearching... " );

	CSphQuery tQuery;
	CSphQueryResult tResult;
	KillListVector dDummyKlist;
	CSphMultiQueryArgs tArgs ( dDummyKlist, 1 );
	tQuery.m_sQuery = "@title cat";

	SphQueueSettings_t tQueueSettings ( tQuery, pIndex->GetMatchSchema(), tResult.m_sError, NULL );
	tQueueSettings.m_bComputeItems = false;
	ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings );
	if ( !pSorter )
	{
		printf ( "failed to create sorter; error=%s", tResult.m_sError.cstr() );

	} else if ( !pIndex->MultiQuery ( &tQuery, &tResult, 1, &pSorter, tArgs ) )
	{
		printf ( "query failed; error=%s", pIndex->GetLastError().cstr() );

	} else
	{
		sphFlattenQueue ( pSorter, &tResult, 0 );
		printf ( "%d results found in %d.%03d sec!\n", tResult.m_dMatches.GetLength(), tResult.m_iQueryTime/1000, tResult.m_iQueryTime%1000 );
		ARRAY_FOREACH ( i, tResult.m_dMatches )
			printf ( "%d. id=" DOCID_FMT ", weight=%d\n", 1+i, tResult.m_dMatches[i].m_uDocID, tResult.m_dMatches[i].m_iWeight );
	}

	SafeDelete ( pSorter );
	printf ( "---\n" );
}

static int g_iFieldsCount = 0;

void DoIndexing ( CSphSource_MySQL * pSrc, ISphRtIndex * pIndex )
{
	CSphString sError, sWarning, sFilter;
	CSphVector<DWORD> dMvas;

	int64_t tmStart = sphMicroTimer ();
	int64_t tmAvgCommit = 0;
	int64_t tmMaxCommit = 0;
	int iCommits = 0;
	for ( ;; )
	{
		const char ** pFields = (const char **)pSrc->NextDocument ( sError );
		if ( !pFields )
			break;

		if ( pSrc->m_tDocInfo.m_uDocID )
			pIndex->AddDocument ( pIndex->CloneIndexingTokenizer(), g_iFieldsCount, pFields, pSrc->m_tDocInfo, false, sFilter, NULL, dMvas, sError, sWarning, NULL );

		if ( ( pSrc->GetStats().m_iTotalDocuments % COMMIT_STEP )==0 || !pSrc->m_tDocInfo.m_uDocID )
		{
			int64_t tmCommit = sphMicroTimer();
			pIndex->Commit ( NULL, NULL );
			tmCommit = sphMicroTimer()-tmCommit;

			iCommits++;
			tmAvgCommit += tmCommit;
			tmMaxCommit = Max ( tmMaxCommit, tmCommit );

			if ( !pSrc->m_tDocInfo.m_uDocID )
			{
				tmAvgCommit /= iCommits;
				break;
			}
		}

		if (!( pSrc->GetStats().m_iTotalDocuments % 100 ))
			printf ( "%d docs\r", (int)pSrc->GetStats().m_iTotalDocuments );

		static bool bOnce = true;
		if ( iCommits*COMMIT_STEP>=5000 && bOnce )
		{
			printf ( "\n" );
			DoSearch ( pIndex );
			bOnce = false;
		}
	}

	pSrc->Disconnect();

	int64_t tmEnd = sphMicroTimer ();
	float fTotalMB = (float)pSrc->GetStats().m_iTotalBytes/1000000.0f;
	printf ( "commit-step %d, %d docs, %d bytes, %d.%03d sec, %.2f MB/sec\n",
		COMMIT_STEP,
		(int)pSrc->GetStats().m_iTotalDocuments,
		(int)pSrc->GetStats().m_iTotalBytes,
		(int)((tmEnd-tmStart)/1000000), (int)(((tmEnd-tmStart)%1000000)/1000),
		fTotalMB*1000000.0f/(tmEnd-tmStart) );
	printf ( "commit-docs %d, avg %d.%03d msec, max %d.%03d msec\n", COMMIT_STEP,
		(int)(tmAvgCommit/1000), (int)(tmAvgCommit%1000),
		(int)(tmMaxCommit/1000), (int)(tmMaxCommit%1000) );
	g_fTotalMB += fTotalMB;
}


CSphSource_MySQL * SpawnSource ( const char * sQuery, ISphTokenizer * pTok, CSphDict * pDict )
{
	CSphSource_MySQL * pSrc = new CSphSource_MySQL ( "test" );
	pSrc->SetTokenizer ( pTok );
	pSrc->SetDict ( pDict );

	CSphSourceParams_MySQL tParams;
	tParams.m_sHost = "localhost";
	tParams.m_sUser = "root";
	tParams.m_sDB = "lj";
	tParams.m_dQueryPre.Add ( "SET NAMES utf8" );
	tParams.m_sQuery = sQuery;

	CSphColumnInfo tCol;
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tCol.m_sName = "channel_id";
	tParams.m_dAttrs.Add ( tCol );
	tCol.m_eAttrType = SPH_ATTR_TIMESTAMP;
	tCol.m_sName = "published";
	tParams.m_dAttrs.Add ( tCol );

	SetupIndexing ( pSrc, tParams );
	return pSrc;
}


static ISphRtIndex * g_pIndex = NULL;


void IndexingThread ( void * pArg )
{
	CSphSource_MySQL * pSrc = (CSphSource_MySQL *) pArg;
	DoIndexing ( pSrc, g_pIndex );
}


int main ( int argc, char ** argv )
{
	if ( argc==2 )
		COMMIT_STEP = atoi ( argv[1] );

	// threads should be initialized before memory allocations
	char cTopOfMainStack;
	sphThreadInit();
	MemorizeStack ( &cTopOfMainStack );

	CSphString sError;
	CSphDictSettings tDictSettings;
	tDictSettings.m_bWordDict = false;

	ISphTokenizer * pTok = sphCreateUTF8Tokenizer();
	CSphDict * pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt1", sError );
	CSphSource_MySQL * pSrc = SpawnSource ( "SELECT id, channel_id, UNIX_TIMESTAMP(published) published, "
		"title, UNCOMPRESS(content) content FROM posting WHERE id<=10000 AND id%2=0", pTok, pDict );

	ISphTokenizer * pTok2 = sphCreateUTF8Tokenizer();
	CSphDict * pDict2 = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt2", sError );
	CSphSource_MySQL * pSrc2 = SpawnSource ( "SELECT id, channel_id, UNIX_TIMESTAMP(published) published, "
		"title, UNCOMPRESS(content) content FROM posting WHERE id<=10000 AND id%2=1", pTok2, pDict2 );

	CSphSchema tSrcSchema;
	if ( !pSrc->UpdateSchema ( &tSrcSchema, sError ) )
		sphDie ( "update-schema failed: %s", sError.cstr() );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	tSchema.m_dFields = tSrcSchema.m_dFields;
	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );
	g_iFieldsCount = tSrcSchema.m_dFields.GetLength();

	CSphConfigSection tRTConfig;
	sphRTInit ( tRTConfig, true );
	sphRTConfigure ( tRTConfig, true );
	SmallStringHash_T< CSphIndex * > dTemp;
	sphReplayBinlog ( dTemp, 0 );
	ISphRtIndex * pIndex = sphCreateIndexRT ( tSchema, "testrt", 32*1024*1024, "data/dump", false );
	pIndex->SetTokenizer ( pTok ); // index will own this pair from now on
	pIndex->SetDictionary ( pDict );
	if ( !pIndex->Prealloc ( false ) )
		sphDie ( "prealloc failed: %s", pIndex->GetLastError().cstr() );
	pIndex->PostSetup();
	g_pIndex = pIndex;

	// initial indexing
	int64_t tmStart = sphMicroTimer();

	SphThread_t t1, t2;
	sphThreadCreate ( &t1, IndexingThread, pSrc );
	sphThreadCreate ( &t2, IndexingThread, pSrc2 );
	sphThreadJoin ( &t1 );
	sphThreadJoin ( &t2 );

#if 0
	// update
	tParams.m_sQuery = "SELECT id, channel_id, UNIX_TIMESTAMP(published) published, title, "
		"UNCOMPRESS(content) content FROM rt2 WHERE id<=10000";
	SetupIndexing ( pSrc, tParams );
	DoIndexing ( pSrc, pIndex );
#endif

	// search
	DoSearch ( pIndex );

	// shutdown index (should cause dump)
	int64_t tmShutdown = sphMicroTimer();

#if SPH_ALLOCS_PROFILER
	printf ( "pre-shutdown allocs=%d, bytes=" INT64_FMT "\n", sphAllocsCount(), sphAllocBytes() );
#endif
	SafeDelete ( pIndex );
#if SPH_ALLOCS_PROFILER
	printf ( "post-shutdown allocs=%d, bytes=" INT64_FMT "\n", sphAllocsCount(), sphAllocBytes() );
#endif

	int64_t tmEnd = sphMicroTimer();
	printf ( "shutdown done in %d.%03d sec\n", (int)((tmEnd-tmShutdown)/1000000), (int)(((tmEnd-tmShutdown)%1000000)/1000) );
	printf ( "total with shutdown %d.%03d sec, %.2f MB/sec\n",
		(int)((tmEnd-tmStart)/1000000), (int)(((tmEnd-tmStart)%1000000)/1000),
		g_fTotalMB*1000000.0f/(tmEnd-tmStart) );

#if SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER
	sphAllocsStats();
#endif
#if USE_WINDOWS
	PROCESS_MEMORY_COUNTERS pmc;
	HANDLE hProcess = OpenProcess ( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId() );
	if ( hProcess && GetProcessMemoryInfo ( hProcess, &pmc, sizeof(pmc)) )
	{
		printf ( "--- peak-wss=%d, peak-pagefile=%d\n", (int)pmc.PeakWorkingSetSize, (int)pmc.PeakPagefileUsage );
	}
#endif

	SafeDelete ( pIndex );
	sphRTDone ();
}

//
// $Id$
//
