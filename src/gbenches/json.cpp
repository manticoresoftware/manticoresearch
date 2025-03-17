//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//


#include <benchmark/benchmark.h>

#include "json/cJSON.h"
#include "sphinxjson.h"

// Miscelaneous short tests for json/cjson

//////////////////////////////////////////////////////////////////////////

// defined in sphinxjson
int sphJsonUnescape ( char ** pEscaped, int iLen );
int sphJsonUnescape1 ( char ** pEscaped, int iLen );

// unescaped pEscaped, modify pEscaped to unescaped chain, return the length of unescaped
// that is actually used, defined in sphinxjson.cpp
int JsonUnescape ( char* pTarget, const char* pEscaped, int iLen );

// defined in cJSON_test
extern "C"
{
int cJsonunescape ( char ** buf, cJSON * pOut );
}


class bench_jsonunescape : public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State & state )
	{
		iLen = (int) strlen ( sLiteral );
		buf = new char[iLen];
		pJson = cJSON_CreateObject();
	}

	void TearDown ( const ::benchmark::State & state )
	{
		SafeDelete ( pJson );
		SafeDeleteArray (buf);
	}

	const char* sLiteral = R"("In `docs/searching/expressions,_functions,_and_operators.rst` which reflected into\\nhttps://manticoresearch.gitlab.io/dev/searching/expressions,_functions,_and_operators.html\\n\\n1. At the top there is a kind of TOC with shortcuts to the functions described in the section.\\nHowever this TOC is not consistent. I.e., it doesn't refer to all function actually described there.\\n\\nMost prominent example is 'PACKEDFACTORS()' - it absent in the TOC.\\n\\n2. Also consider whether it is better or not to sort function descriptions in the section alphabetically ('REMAP' at the end looks strange, as 'WEIGHT' is before it).")";
	int iLen;
	char* buf = nullptr;
	cJSON* pJson = nullptr;
};

BENCHMARK_F( bench_jsonunescape, plain ) ( benchmark::State & st )
{
	char * sBuf = nullptr;
	for ( auto _ : st )
	{
		st.PauseTiming ();
		memcpy ( buf, sLiteral, iLen );
		sBuf = buf;
		st.ResumeTiming ();
		sphJsonUnescape ( &sBuf, iLen );
	}
	st.SetBytesProcessed ( st.iterations() * iLen );
	st.SetLabel("unescaped version by memmove in-place");
}

BENCHMARK_F( bench_jsonunescape, nomove ) ( benchmark::State & st )
{
	char * sBuf = nullptr;
	for ( auto _ : st )
	{
		st.PauseTiming ();
		memcpy ( buf, sLiteral, iLen );
		sBuf = buf;
		st.ResumeTiming ();
		sphJsonUnescape1 ( &sBuf, iLen );
	}
	st.SetBytesProcessed ( st.iterations() * iLen );
	st.SetLabel ( "moves nothing" );
}

BENCHMARK_F( bench_jsonunescape, cjson ) ( benchmark::State & st )
{
	cJSON * pJson = cJSON_CreateObject ();
	char * sBuf = nullptr;
	for ( auto _ : st )
	{
		st.PauseTiming ();
		memcpy ( buf, sLiteral, iLen );
		sBuf = buf;
		st.ResumeTiming ();
		cJsonunescape ( & sBuf, pJson );
	}
	SafeDelete ( pJson );
	st.SetBytesProcessed ( st.iterations() * iLen );
	st.SetLabel ( "use cjson" );
}


class bson_vs_cjson : public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State & state )
	{
		iLen = (int)strlen ( sLiteral );
	}

	void TearDown ( const ::benchmark::State & state )
	{
	}

	const char* sLiteral = R"({"query":{"percolate":{"document":{"title":"A new tree test in the office office"}}}})";
	int iLen = 0;
	CSphString sBuf;
};

using namespace bson;


BENCHMARK_F( bson_vs_cjson, bson ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		st.PauseTiming ();
		sBuf.SetBinary ( sLiteral, iLen );
		auto buf = (char *) sBuf.cstr ();
		st.ResumeTiming ();
		BsonContainer_c dBson { buf };
		benchmark::DoNotOptimize ( dBson );
	}
	st.SetBytesProcessed ( st.iterations() * iLen );
}

BENCHMARK_F( bson_vs_cjson, bson_without_lowercase ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		st.PauseTiming ();
		sBuf.SetBinary ( sLiteral, iLen );
		auto buf = (char *) sBuf.cstr ();
		st.ResumeTiming ();
		BsonContainer_c dBson { buf, false };
		benchmark::DoNotOptimize ( dBson );
	}
	st.SetBytesProcessed ( st.iterations() * iLen );
}

BENCHMARK_F( bson_vs_cjson, cjson ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		st.PauseTiming ();
		sBuf.SetBinary ( sLiteral, iLen );
		auto buf = (char *) sBuf.cstr ();
		st.ResumeTiming ();
		auto pBson = cJSON_Parse ( buf );
		benchmark::DoNotOptimize ( pBson );
		if ( pBson )
			cJSON_Delete ( pBson );
	}
	st.SetBytesProcessed ( st.iterations() * iLen );
}

BENCHMARK_F( bson_vs_cjson, bson_via_cjson ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		st.PauseTiming ();
		sBuf.SetBinary ( sLiteral, iLen );
		auto buf = (char *) sBuf.cstr ();
		st.ResumeTiming ();
		auto pCjson = cJSON_Parse ( buf );
		CSphVector<BYTE> dBson ( iLen );
		bson::cJsonToBson ( pCjson, dBson, false, false );
		benchmark::DoNotOptimize ( dBson );
		if ( pCjson )
			cJSON_Delete ( pCjson );
	}
	st.SetBytesProcessed ( st.iterations() * iLen );
}

BENCHMARK_F( bson_vs_cjson, cJsonToBson ) ( benchmark::State & st )
{
	auto pCjson = cJSON_Parse ( sLiteral );
	for ( auto _ : st )
	{
		st.PauseTiming ();
		CSphVector<BYTE> dBson ( iLen );
		st.ResumeTiming ();
		bson::cJsonToBson ( pCjson, dBson, false, false );
		benchmark::DoNotOptimize ( dBson );
	}
	st.SetBytesProcessed ( st.iterations() * iLen );
}

BENCHMARK_F( bson_vs_cjson, cJsonToBson_with_lowercase ) ( benchmark::State & st )
{
	auto pCjson = cJSON_Parse ( sLiteral );
	for ( auto _ : st )
	{
		st.PauseTiming ();
		CSphVector<BYTE> dBson ( iLen );
		st.ResumeTiming ();
		bson::cJsonToBson ( pCjson, dBson );
		benchmark::DoNotOptimize ( dBson );
	}
	st.SetBytesProcessed ( st.iterations() * iLen );
}

BENCHMARK_F ( bson_vs_cjson, bson_child_by_name )
( benchmark::State& st )
{
	sBuf.SetBinary ( sLiteral, iLen );
	auto buf = (char*)sBuf.cstr();
	BsonContainer_c dBson { buf };

	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( dBson.ChildByName ( "query" ).first );
	}
	st.SetItemsProcessed( st.iterations() );
}

BENCHMARK_F ( bson_vs_cjson, bson_child_by_namelw )
( benchmark::State& st )
{
	sBuf.SetBinary ( sLiteral, iLen );
	auto buf = (char*)sBuf.cstr();
	BsonContainer_c dBson { buf, false };
	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( dBson.ChildByName ( "query" ).first );
	}
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK_F ( bson_vs_cjson, cjson_getobjitem )
( benchmark::State& st )
{
	sBuf.SetBinary ( sLiteral, iLen );
	auto buf = (char*)sBuf.cstr();
	auto pBson = cJSON_Parse ( buf );
	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( cJSON_GetObjectItem ( pBson, "query" ) );
	}
	st.SetItemsProcessed ( st.iterations() );
	if ( pBson )
		cJSON_Delete ( pBson );
}


class BM_tolower : public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State & state )
	{
		for ( int i = 0; i<192; ++i )
		{
			x[i] = char ( i+64 );
			l[i] = (char) ::tolower ( i+64 );
		}
		x[192] = '\0';
		l[192] = '\0';
		NRUNS = state.range ( 0 );
		j = 0;
	}

	void TearDown ( const ::benchmark::State & state )
	{
	}

	char x[193];
	char l[193];
	int64_t NRUNS = 0;
	int j;
};

BENCHMARK_DEFINE_F( BM_tolower, call_tolower ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < NRUNS; ++i )
		{
			benchmark::DoNotOptimize ( (char) tolower ( x[j++] ) );
			if (j>192)
				j = 0;
		}
	}
	st.SetBytesProcessed ( st.iterations() * NRUNS );
}

BENCHMARK_DEFINE_F( BM_tolower, lookup_table ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		for ( auto i = 0; i < NRUNS; ++i )
		{
			benchmark::DoNotOptimize ( l[(int)x[j]] );
			if ( j > 192 )
				j = 0;
		}
	}
	st.SetBytesProcessed( st.iterations() * NRUNS );
}

BENCHMARK_REGISTER_F ( BM_tolower, call_tolower )->Range ( 1, 4096 );
BENCHMARK_REGISTER_F ( BM_tolower, lookup_table )->Range ( 1, 4096 );

class BM_formatter : public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State & state )
	{
		dIndexes.Add ( { "test1", "test1_\tpath" } );
		dIndexes.Add ( { "test2", "test2_\"path" } );
		dIndexes.Add ( { "test3", "test3_path" } );
	}

	void TearDown ( const ::benchmark::State & state )
	{
	}

	struct MyIndex_t
	{
		CSphString m_sName;
		CSphString m_sPath;
	};

	CSphString sResult;
	CSphVector<MyIndex_t> dIndexes;
};

BENCHMARK_F( BM_formatter, cjson ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		cJSON * pRoot = cJSON_CreateObject ();
		cJSON * pIndexes = cJSON_CreateArray ();
		cJSON_AddItemToObject ( pRoot, "indexes", pIndexes );

		for ( auto & dIdx : dIndexes )
		{
			cJSON * pIndex = cJSON_CreateObject ();
			cJSON_AddItemToArray ( pIndexes, pIndex );
			cJSON_AddStringToObject ( pIndex, "name", dIdx.m_sName.cstr () );
			cJSON_AddStringToObject ( pIndex, "path", dIdx.m_sPath.cstr () );
		}

		char * szResult = cJSON_Print ( pRoot );
		sResult = szResult;
		cJSON_Delete ( pRoot );
	}
	st.SetItemsProcessed ( st.iterations() );
}

BENCHMARK_F( BM_formatter, stringbuilder ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		JsonEscapedBuilder tOut;
		tOut.StartBlock ( dJsonObj );
		{
			ScopedComma_c sIndexes ( tOut, ",", "\"indexes\":[", "]" );
			for ( auto & dIdx : dIndexes )
			{
				ScopedComma_c sIndex ( tOut, ",", "{", "}" );
				tOut.AppendName ( "name" ).AppendEscaped ( dIdx.m_sName.cstr (), EscBld::eEscape );
				tOut.AppendName ( "path" ).AppendEscaped ( dIdx.m_sPath.cstr (), EscBld::eEscape );
			}
		}
		tOut.FinishBlocks ();
		tOut.MoveTo ( sResult );
	}
	st.SetItemsProcessed ( st.iterations() );
}
