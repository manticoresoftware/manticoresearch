//
// Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include <benchmark/benchmark.h>

#include "fileutils.h"
#include "std/string.h"
#include "tokenizer/tokenizer.h"

//////////////////////////////////////////////////////////////////////////

static constexpr char g_sTmpfile[] = "__libsphinxtest.tmp";

//////////////////////////////////////////////////////////////////////////

bool CreateSynonymsFile ()
{
	FILE* fp = fopen ( g_sTmpfile, "w+" );
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
		"; => ;\n" );
	fclose ( fp );
	return true;
}

char* LoadFile ( const char* sName, int* pLen, bool bReportErrors )
{
	auto sPath = GetPathOnly ( __FILE__ );
	auto sTestFile = SphSprintf ( "%s%s", sPath.cstr(), sName );

	FILE* fp = fopen ( sTestFile.cstr(), "rb" );
	if ( !fp )
	{
		if ( bReportErrors )
			printf ( "benchmark failed: error opening %s\n", sTestFile.cstr() );
		return NULL;
	}
	const int MAX_DATA = 10485760;
	char* sData = new char[MAX_DATA];
	auto iData = (int)fread ( sData, 1, MAX_DATA, fp );
	fclose ( fp );
	if ( iData <= 0 )
	{
		if ( bReportErrors )
			printf ( "benchmark failed: error reading %s\n", sTestFile.cstr() );
		SafeDeleteArray ( sData );
		return NULL;
	}
	*pLen = iData;
	return sData;
}

class BM_tokenizer: public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State& state )
	{
		if ( !CreateSynonymsFile () )
		{
			printf ( "benchmark failed: error writing temp synonyms file\n" );
			return;
		}
		iBytes = 0;
		sData = LoadFile ( "../CMakeLists.txt", &iBytes, true );
		pTokenizer = Tokenizer::Detail::CreateUTF8Tokenizer();
	}

	void TearDown ( const ::benchmark::State& state )
	{
		SafeDeleteArray ( sData );
		unlink ( g_sTmpfile );
	}

	int iBytes = 0;
	StrVec_t dWarnings;
	CSphString sError;
	char* sData = nullptr;
	TokenizerRefPtr_c pTokenizer;
};


BENCHMARK_F ( BM_tokenizer, nosynonyms )
( benchmark::State& st )
{
	pTokenizer->AddSpecials ( "!-" );
	int iTokens = 0;
	int iAllBytes = 0;
	for ( auto _ : st )
	{
		pTokenizer->SetBuffer ( (BYTE*)sData, iBytes );
		while ( pTokenizer->GetToken() )
			++iTokens;
		iAllBytes += iBytes;
	}
	st.SetBytesProcessed ( iAllBytes );
	st.SetItemsProcessed ( iTokens );
}

BENCHMARK_F ( BM_tokenizer, synonyms )
( benchmark::State& st )
{
	pTokenizer->LoadSynonyms ( g_sTmpfile, NULL, dWarnings, sError );
	pTokenizer->AddSpecials ( "!-" );
	int iTokens = 0;
	int iAllBytes = 0;
	for ( auto _ : st )
	{
		pTokenizer->SetBuffer ( (BYTE*)sData, iBytes );
		while ( pTokenizer->GetToken() )
			++iTokens;
		iAllBytes += iBytes;
	}
	st.SetBytesProcessed ( iAllBytes );
	st.SetItemsProcessed ( iTokens );
}
