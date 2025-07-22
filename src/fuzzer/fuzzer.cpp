//
// Copyright (c) 2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include <cstdint>
#include <cstddef>
#include <array>

#include "dict/dict_base.h"
#include "schema/schema.h"
#include "sphinxquery/sphinxquery.h"
#include "sphinxquery/xqparser.h"
#include "std/string.h"
#include "std/stringbuilder.h"
#include "tokenizer/tokenizer.h"
#include "sphinxint.h"

// stuff taken from gtests_tokenizer.cpp

enum : DWORD
{
  TOK_EXCEPTIONS = 1, TOK_NO_DASH = 2, TOK_NO_SHORT = 4
};

class Tokenizertest
{
protected:
  TokenizerRefPtr_c CreateTestTokenizer ( DWORD uMode )
  {
    StrVec_t dWarnings;
    CSphString sError;
    CSphTokenizerSettings tSettings;
    if ( !(uMode & TOK_NO_SHORT) )
      tSettings.m_iMinWordLen = 2;

    TokenizerRefPtr_c pTokenizer = Tokenizer::Create ( tSettings, nullptr, nullptr, dWarnings, sError );
    if ( !(uMode & TOK_NO_DASH) )
    {
      Verify ( pTokenizer->SetCaseFolding ( "-, 0..9, A..Z->a..z, _, a..z, U+80..U+FF", sError ) );
      pTokenizer->AddSpecials ( "!-" );
    } else
    {
      Verify ( pTokenizer->SetCaseFolding ( "0..9, A..Z->a..z, _, a..z, U+80..U+FF", sError ) );
      pTokenizer->AddSpecials ( "!" );
    }

    // tricky little shit!
    // we want to create a query mode tokenizer
    // the official way is to Clone() an indexing mode one, so we do that
    // however, Clone() adds backslash as a special
    // and that must be done *after* SetCaseFolding, otherwise it's not special any more
    return pTokenizer->Clone ( SPH_CLONE_QUERY );
  }


  TokenizerRefPtr_c m_pTokenizer;
  TokenizerRefPtr_c & pTokenizer = m_pTokenizer;
  CSphString sError;
};

class CSphDummyIndex : public CSphIndexStub
{
public:
  SmallStringHash_T<int> m_hHits;


  CSphDummyIndex ()
    : CSphIndexStub ( nullptr, nullptr )
  { }

  bool FillKeywords ( CSphVector<CSphKeywordInfo> & dKeywords ) const override;
};

struct CKeywordHits
{
  const char * m_szKeyword;
  int m_iHits;
};


class QueryParser : public Tokenizertest
{
protected:

  void SetUp ()
  {
    tSchema.AddField ( "title" );
    tSchema.AddField ( "body" );

    TokenizerRefPtr_c pBase = Tokenizer::Detail::CreateUTF8Tokenizer();
    CSphTokenizerSettings tTokenizerSetup;
    tTokenizerSetup.m_iMinWordLen = 2;
//    tTokenizerSetup.m_sSynonymsFile = g_sTmpfile;
    pBase->Setup ( tTokenizerSetup );

    StrVec_t dWarnings;

    pTokenizer = sphCloneAndSetupQueryTokenizer ( pBase, true, false, false );

    CSphDictSettings tDictSettings;
    tDictSettings.m_bWordDict = false;
    pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTokenizer, "query", false, 32, nullptr, sError );
    constexpr CKeywordHits dPseudoHits[] = { { "aaa", 0 }, { "bbb", 0 }, { "ccc", 35 }, { "ddd", 63 }, { "eee", 2445 } };


//  if ( pKeywordHits )
//  {
    for ( const CKeywordHits& tHit : dPseudoHits )
      tIndex.m_hHits.Add ( tHit.m_iHits, tHit.m_szKeyword );
//  }
  }

public:
  QueryParser ()
  {
    SetUp();
  }

  void Transform ( const char * szQuery ) const;

  DictRefPtr_c pDict;
  CSphSchema tSchema;
  CSphIndexSettings tTmpSettings;
  CSphDummyIndex tIndex;
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

void QueryParser::Transform ( const char * szQuery ) const
{

  XQQuery_t tQuery;
  if (!sphParseExtendedQuery ( tQuery, szQuery, nullptr, pTokenizer, &tSchema, pDict, tTmpSettings, nullptr ))
    return;

  if (!tQuery.m_pRoot)
    return;

  sphTransformExtendedQuery ( &tQuery.m_pRoot, tTmpSettings, true, &tIndex );
}

/// fuzzing stuff
////////////////

void get_case ( StringBuilder_c & cBuilder, const uint8_t * Data, size_t Size )
{
  cBuilder.Clear();
  cBuilder << Str_t { (const char *) Data, Size };
}

extern
"C"
int LLVMFuzzerTestOneInput ( const uint8_t * Data, size_t Size )
{
  StringBuilder_c cBuilder;
  static QueryParser transformer;

  get_case ( cBuilder, Data, Size );
  const char * sParam = cBuilder.cstr();
//  printf("%s\n", sParam);
  transformer.Transform ( sParam );
  return 0; // Values other than 0 and -1 are reserved for future use.
}

// need dictionary - provided here as dict.txt
// need folder to store corpus - investigated cases which increases coverage
// also can be run in parallel as -jobs=8 -workers=8
// see https://github.com/google/fuzzing/blob/master/tutorial/libFuzzerTutorial.md for details

/*
mkdir build && cd build
export CC=`which clang`
export CXX=`which clang++`
cmake -DQFUZZER=ON /path/to/manticore
cmake --build . --target qfuzzer

cd src/fuzzer
mkdir corpus
./qfuzzer -dict=/path/to/manticore/src/fuzzer/dict.txt corpus
*/