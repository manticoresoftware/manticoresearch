//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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
	}

	void TearDown ( const ::benchmark::State & state )
	{
		SafeDeleteArray (buf);
	}

	const char* sLiteral = R"("In `docs/searching/expressions,_functions,_and_operators.rst` which reflected into\\nhttps://manticoresearch.gitlab.io/dev/searching/expressions,_functions,_and_operators.html\\n\\n1. At the top there is a kind of TOC with shortcuts to the functions described in the section.\\nHowever this TOC is not consistent. I.e., it doesn't refer to all function actually described there.\\n\\nMost prominent example is 'PACKEDFACTORS()' - it absent in the TOC.\\n\\n2. Also consider whether it is better or not to sort function descriptions in the section alphabetically ('REMAP' at the end looks strange, as 'WEIGHT' is before it).")";
	int iLen;
	char* buf = nullptr;
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
}


class bson_vs_cjson : public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State & state )
	{
		iLen = (int) strlen ( sLiteral );
	}

	void TearDown ( const ::benchmark::State & state )
	{
	}

	const char* sLiteral = R"({"query":{"percolate":{"document":{"title":"A new tree test in the office office"}}}})";
	int iLen;
	CSphString sBuf;
	const volatile void * pRes = nullptr;
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
		pRes = dBson.ChildByName ( "query" ).first;
	}
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
		pRes = dBson.ChildByName ( "query" ).first;
	}
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
		pRes = cJSON_GetObjectItem ( pBson, "query" );
		if ( pBson )
			cJSON_Delete ( pBson );
	}
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
		pRes = cJSON_GetObjectItem ( pCjson, "query" );
		CSphVector<BYTE> m_Bson ( iLen );
		bson::cJsonToBson ( pCjson, m_Bson, false, false );

		if ( pCjson )
			cJSON_Delete ( pCjson );
	}
}

BENCHMARK_F( bson_vs_cjson, cJsonToBson ) ( benchmark::State & st )
{
	auto pCjson = cJSON_Parse ( sLiteral );
	for ( auto _ : st )
	{
		st.PauseTiming ();
		CSphVector<BYTE> m_Bson ( iLen );
		st.ResumeTiming ();
		bson::cJsonToBson ( pCjson, m_Bson, false, false );
	}
}

BENCHMARK_F( bson_vs_cjson, cJsonToBson_with_lowercase ) ( benchmark::State & st )
{
	auto pCjson = cJSON_Parse ( sLiteral );
	for ( auto _ : st )
	{
		st.PauseTiming ();
		CSphVector<BYTE> m_Bson ( iLen );
		st.ResumeTiming ();
		bson::cJsonToBson ( pCjson, m_Bson );
	}
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
	}

	void TearDown ( const ::benchmark::State & state )
	{
	}

	char x[193];
	char l[193];
	char result[193];
};

BENCHMARK_F( BM_tolower, call_tolower ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		for ( int j = 0; j<193; ++j )
			result[j] = (char) tolower ( x[j] );
	}
}

BENCHMARK_F( BM_tolower, lookup_table ) ( benchmark::State & st )
{
	for ( auto _ : st )
	{
		for ( int j = 0; j<193; ++j )
			result[j] = l[(int) x[j]];
	}
}

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
}
