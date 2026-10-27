// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// Distributed under the GNU General Public License, version 3 or later.

#include <benchmark/benchmark.h>
#include "docidlookup.h"
#include "sphinxstd.h"

#if WITH_BBTREE_DOCID
#include <algorithm>
#include <chrono>
#include <random>
#include <unistd.h>

// Exercise the same file writer, reader and cache construction as disk chunks.
// Setup and query generation are excluded from lookup timing. Odd IDs miss;
// even IDs are unique random positive int64 values, sorted in unsigned order.
static void DocidPointLookup ( benchmark::State & tState, bool bCache )
{
	const int nDocs = (int)tState.range(0);
	const int iMissPercent = (int)tState.range(1);
	ConfigureDocidLookupCache ( 256*1024*1024 );
	CSphVector<DocidRowidPair_t> dPairs;
	std::mt19937_64 tRandom(42);
	for ( int i=0; i<nDocs; ++i )
		dPairs.Add ( { (DocID_t)(tRandom() & 0x7ffffffffffffffeULL), (RowID_t)i } );
	std::sort ( dPairs.Begin(), dPairs.End(), [] ( const auto & a, const auto & b ) { return a.m_tDocID<b.m_tDocID; } );
	char sPath[] = "/tmp/manticore-docid-bench-XXXXXX";
	int iFd = mkstemp(sPath);
	if ( iFd<0 )
	{
		tState.SkipWithError("mkstemp failed");
		return;
	}
	close(iFd);
	CSphString sError;
	CSphMappedBuffer<BYTE> tData;
	if ( !WriteDocidLookup ( sPath, dPairs, sError ) || !tData.Setup ( sPath, sError, false ) )
	{
		unlink(sPath);
		tState.SkipWithError(sError.cstr());
		return;
	}
	unlink(sPath);
	LookupReader_c tReader ( tData.GetReadPtr(), 71 );
	auto tStart = std::chrono::steady_clock::now();
	if ( bCache && !tReader.BuildCache(71) )
	{
		tState.SkipWithError("cache refused");
		return;
	}
	double fBuildMs = std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-tStart).count();
	std::vector<DocID_t> dQueries;
	for ( int i=0; i<65536; ++i )
	{
		const auto & tPair = dPairs[tRandom()%nDocs];
		bool bMiss = (int)(tRandom()%100)<iMissPercent;
		DocID_t tID = tPair.m_tDocID + bMiss;
		if ( tReader.Find(tID)!=(bMiss ? INVALID_ROWID : tPair.m_tRowID) )
		{
			tState.SkipWithError("incorrect lookup");
			return;
		}
		dQueries.push_back(tID);
	}
	for ( auto _ : tState )
		for ( DocID_t tID : dQueries )
			benchmark::DoNotOptimize(tReader.Find(tID));
	tState.SetItemsProcessed(tState.iterations()*dQueries.size());
	tState.counters["cache_bytes"] = (double)tReader.GetCacheBytes();
	tState.counters["disk_bytes"] = (double)tData.GetLengthBytes();
	tState.counters["cache_build_ms"] = bCache ? fBuildMs : 0;
}

static void DocidCompressed ( benchmark::State & tState ) { DocidPointLookup ( tState, false ); }
static void DocidBbtree ( benchmark::State & tState ) { DocidPointLookup ( tState, true ); }
BENCHMARK(DocidCompressed)->ArgsProduct({{100000,1000000},{0,50,100}});
BENCHMARK(DocidBbtree)->ArgsProduct({{100000,1000000},{0,50,100}});
#endif
