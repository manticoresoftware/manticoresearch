//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>

#include "sphinxint.h"
#include "attribute.h"
#include "sphinxrt.h"
#include "sphinxsort.h"
#include "searchdaemon.h"
#include "binlog.h"
#include "accumulator.h"

#include <gmock/gmock.h>


//////////////////////////////////////////////////////////////////////////
static void DeleteIndexFiles ( const char * sIndex )
{
	if ( !sIndex )
		return;

	const char * sExts[] = {
		"kill", "lock", "meta", "ram", "0.spa", "0.spd", "0.spe", "0.sph", "0.spi", "0.spk", "0.spm", "0.spp" };

	CSphString sName;
	for (auto & sExt : sExts)
	{
		sName.SetSprintf ( "%s.%s", sIndex, sExt );
		unlink ( sName.cstr () );
	}
}

void TestRTInit ()
{
	CSphConfigSection tRTConfig;

	sphRTInit ( tRTConfig, true, nullptr );
	Binlog::Configure ( tRTConfig, true, 0 );

	SmallStringHash_T<CSphIndex *> hIndexes;
	Binlog::Replay ( hIndexes );
}

#define RT_INDEX_FILE_NAME "test_temp"

class MockTestDoc_c : public CSphSource
{
public:
	explicit MockTestDoc_c ( const CSphSchema &tSchema, BYTE ** ppDocs, int iDocs, int iFields )
		: CSphSource ( "test_doc" )
	{
		m_tSchema = tSchema;
		m_ppDocs = ppDocs;
		m_iDocCount = iDocs;
		m_iFields = iFields;
		m_dFieldLengths.Resize ( m_iFields );
		m_dFields.Reserve ( iFields );
	}

	BYTE ** NextDocument ( bool & bEOF, CSphString & ) override
	{
		bEOF = false;
		int iDoc = (int)++m_iDocsCounter;
		if ( iDoc>=m_iDocCount )
		{
			bEOF = true;
			return nullptr;
		}

		for ( int i = 0; i<m_iFields; i++ )
		{
			char * szField = ( char * ) ( m_ppDocs + iDoc * m_iFields )[i];
			m_dFieldLengths[i] = (int) strlen ( szField );
		}

		const CSphColumnInfo * pId = m_tSchema.GetAttr ( sphGetDocidName() );
		assert ( pId );
		m_tDocInfo.SetAttr( pId->m_tLocator, iDoc+1 );

		return m_ppDocs + iDoc * m_iFields;
	}

	MOCK_CONST_METHOD0( GetFieldLengths, const int *() ); // return m_dFieldLengths.Begin();
	MOCK_METHOD1 ( Connect, bool ( CSphString & ) ); // return true;
	MOCK_METHOD0 ( Disconnect, void() );

	bool IterateStart ( CSphString & ) final
	{
		m_tDocInfo.Reset ( m_tSchema.GetRowSize () );
		m_iPlainFieldsLength = m_tSchema.GetFieldsCount();
		m_iDocsCounter = -1;
		return true;
	}

	MOCK_METHOD2 ( IterateMultivaluedStart, bool ( int, CSphString& )); // return false;
	MOCK_METHOD2 ( IterateMultivaluedNext, bool(int64_t &, int64_t &)); // return false;
	MOCK_METHOD1 ( IterateKillListStart, bool (CSphString & ) ); // return false;
	MOCK_METHOD1 ( IterateKillListNext, bool (DocID_t & ) ) ; // return false
	int GetFieldCount () const { return m_iFields; }

	CSphVector<VecTraits_T<const char>> GetFields ()
	{
		m_dFields.Resize(0);
		for ( int i=0; i<m_iFields; ++i)
		{
			auto pStr = (const char*) m_ppDocs[m_iDocsCounter*m_iFields + i];
			m_dFields.Add ( VecTraits_T<const char> (pStr,strlen(pStr)));
		}
		return m_dFields;
	}

	int m_iDocsCounter;
	int m_iDocCount;
	int m_iFields;
	BYTE ** m_ppDocs;
	CSphVector<VecTraits_T<const char> > m_dFields;
	CSphVector<int> m_dFieldLengths;
};


class MockDocRandomizer_c : public CSphSource
{
public:
	static const int m_iMaxFields = 2;
	static const int m_iMaxFieldLen = 512;
	char m_dFields[m_iMaxFields][m_iMaxFieldLen];
	char * m_ppFields[m_iMaxFields];
	CSphVector<VecTraits_T<const char> > m_dMeasuredFields;
	int m_dFieldLengths[m_iMaxFields];
	int	m_iDocsCounter;

	explicit MockDocRandomizer_c ( const CSphSchema & tSchema ) : CSphSource ( "test_doc" )
	{
		m_tSchema = tSchema;
		m_dMeasuredFields.Reserve(m_iMaxFields);
		for ( int i=0; i<m_iMaxFields; ++i )
			m_ppFields[i] = (char *)&m_dFields[i];
	}

	BYTE ** NextDocument ( bool & bEOF, CSphString & ) override
	{
		bEOF = false;
		if ( m_iDocsCounter>800 )
		{
			bEOF = true;
			return nullptr;
		}

		++m_tDocInfo.m_tRowID;
		++m_iDocsCounter;

		m_tDocInfo.SetAttr ( m_tSchema.GetAttr(0).m_tLocator, m_tDocInfo.m_tRowID+1000 );
		m_tDocInfo.SetAttr ( m_tSchema.GetAttr(1).m_tLocator, 1313 );

		snprintf ( m_dFields[0], m_iMaxFieldLen, "cat title%d title%d title%d title%d title%d"
			, sphRand(), sphRand(), sphRand(), sphRand(), sphRand() );

		snprintf ( m_dFields[1], m_iMaxFieldLen, "dog contentwashere%d contentwashere%d contentwashere%d contentwashere%d contentwashere%d"
			, sphRand(), sphRand(), sphRand(), sphRand(), sphRand() );

		for ( int i=0; i < m_iMaxFields; ++i )
			m_dFieldLengths[i] = (int) strlen ( m_ppFields[i] );

		return (BYTE**) &m_ppFields[0];
	}


	MOCK_CONST_METHOD0( GetFieldLengths, const int *() ); // return m_dFieldLengths.Begin();
	MOCK_METHOD1 ( Connect, bool ( CSphString & ) ); // return true;
	MOCK_METHOD0 ( Disconnect, void () );

	bool IterateStart ( CSphString & ) final
	{
		m_tDocInfo.Reset ( m_tSchema.GetRowSize () );
		m_iDocsCounter = 0;
		m_iPlainFieldsLength = m_tSchema.GetFieldsCount();
		return true;
	}

	MOCK_METHOD2 ( IterateMultivaluedStart, bool ( int, CSphString & ) ); // return false;
	MOCK_METHOD2 ( IterateMultivaluedNext, bool (int64_t &, int64_t &) ); // return false;
	MOCK_METHOD1 ( IterateKillListStart, bool (CSphString & ) ); // return false;
	MOCK_METHOD1 ( IterateKillListNext, bool (DocID_t & ) ); // return false
	int  GetFieldCount () const { return m_iMaxFields; }

	CSphVector<VecTraits_T<const char>> GetFields ()
	{
		m_dMeasuredFields.Resize ( 0 );
		for ( const char * pStr : m_ppFields )
			m_dMeasuredFields.Add ( VecTraits_T<const char> ( pStr, strlen ( pStr ) ) );
		return m_dMeasuredFields;
	}
};


//////////////////////////////////////////////////////////////////////////



class RT : public ::testing::Test
{

protected:
	void SetUp() override
	{
		DeleteIndexFiles ( RT_INDEX_FILE_NAME );
		TestRTInit();
		tDictSettings.m_bWordDict = false;

		pTok = Tokenizer::Detail::CreateUTF8Tokenizer ();

		tSrcSchema.Reset ();
		tSrcSchema.AddField ( "title" );
		tSrcSchema.AddField ( "content" );
	}

	void TearDown() override
	{
		Binlog::Deinit ();
		DeleteIndexFiles ( RT_INDEX_FILE_NAME );
	}

	CSphColumnInfo tCol;
	CSphSchema tSrcSchema;
	CSphString sError, sWarning;

	TokenizerRefPtr_c pTok;

	CSphDictSettings tDictSettings;
};

/*
 * It was instantiated several times, but that wasn't work, since on every instantiation couple of attributes was inserted into schema, having idex's schema the same.
 */

TEST_F ( RT, WeightBoundary )
{
	DWORD uParam = 1500;
	using namespace testing;
	Threads::CallCoroutine ( [&] {
	DictRefPtr_c pDict { sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "weight", false, 32, nullptr, sError ) };

	tCol.m_sName = "id";
	tCol.m_eAttrType = SPH_ATTR_BIGINT;
	tSrcSchema.AddAttr ( tCol, true );

	tCol.m_sName = "channel_id";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	const char * dFields[] = { "If I were a cat...", "We are the greatest cat" };
	auto * pSrc = new MockTestDoc_c ( tSrcSchema, ( BYTE ** ) dFields, 1, 2 );

	EXPECT_CALL ( *pSrc, Connect ( _ ) ).WillOnce ( Return ( true ) );
	EXPECT_CALL ( *pSrc, GetFieldLengths () ).WillOnce ( Return ( pSrc->m_dFieldLengths.Begin () ) );
	EXPECT_CALL ( *pSrc, Disconnect () );

	pSrc->SetTokenizer ( pTok );
	pSrc->SetDict ( pDict );
	pSrc->Setup ( CSphSourceSettings(), nullptr );

	EXPECT_TRUE ( pSrc->Connect ( sError ) );
	EXPECT_TRUE ( pSrc->IterateStart ( sError ) );
	EXPECT_TRUE ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	for ( int i=0; i<tSrcSchema.GetFieldsCount(); i++ )
		tSchema.AddField ( tSrcSchema.GetField(i) );

	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

	auto pIndex = sphCreateIndexRT ( tSchema, "testrt", 32 * 1024 * 1024, RT_INDEX_FILE_NAME, false );

	// tricky bit
	// index owns its tokenizer/dict pair, and MAY do whatever it wants
	// and starting with meta v4, it WILL deallocate tokenizer/dict in Prealloc()
	// in favor of tokenizer/dict loaded from the saved settings in meta
	// however, source still needs those guys!
	// so for simplicity i just clone them
	pIndex->SetTokenizer ( pTok->Clone ( SPH_CLONE_INDEX ) );
	pIndex->SetDictionary ( pDict->Clone () );
	pIndex->PostSetup ();
	StrVec_t dWarnings;
	EXPECT_TRUE ( pIndex->Prealloc ( false, nullptr, dWarnings ) );

	InsertDocData_t tDoc ( pIndex->GetMatchSchema() );
	int iDynamic = pIndex->GetMatchSchema().GetRowSize();

	RtAccum_t tAcc;

	CSphString sFilter;
	bool bEOF = false;
	while (true)
	{
		EXPECT_TRUE ( pSrc->IterateDocument ( bEOF, sError ) );
		if ( bEOF )
			break;

		tDoc.m_dFields = pSrc->GetFields();
		tDoc.m_tDoc.Combine ( pSrc->m_tDocInfo, iDynamic );
		pIndex->AddDocument ( tDoc, false, sFilter, sError, sWarning, &tAcc );
		pIndex->Commit ( nullptr, &tAcc );
	}

	pSrc->Disconnect ();

	ASSERT_EQ ( pSrc->GetStats ().m_iTotalDocuments, 1) << "docs committed";

	CSphQuery tQuery;
	AggrResult_t tResult;
	CSphQueryResult tQueryResult;
	tQueryResult.m_pMeta = &tResult;
	CSphMultiQueryArgs tArgs ( 1 );
	tQuery.m_sQuery = "@title cat";
	auto pParser = sphCreatePlainQueryParser();
	tQuery.m_pQueryParser = pParser.get();

	SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema () );
	SphQueueRes_t tRes;
	ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings, tQuery, tResult.m_sError, tRes );
	ASSERT_TRUE ( pSorter );
	ASSERT_TRUE ( pIndex->MultiQuery ( tQueryResult, tQuery, { &pSorter, 1 }, tArgs ) );
	auto & tOneRes = tResult.m_dResults.Add ();
	tOneRes.FillFromSorter ( pSorter );
	ASSERT_EQ ( tResult.GetLength (), 1 ) << "results found";
	ASSERT_EQ ( tOneRes.m_dMatches[0].m_tRowID, 0 ) << "rowID" ;
	ASSERT_EQ ( tOneRes.m_dMatches[0].m_iWeight, uParam) << "weight" ;

	SafeDelete ( pSorter );
	SafeDelete ( pSrc );
	});
}


TEST_F ( RT, RankerFactors )
{
	using namespace testing;
	Threads::CallCoroutine ( [&] {

	const char * dFields[] = {
		"Seven lies multiplied by seven", "", "Multiplied by seven again", "", "Seven lies multiplied by seven"
		, "Multiplied by seven again", "Mary vs Lamb", "Mary had a little lamb little lamb little lamb"
		, "Mary vs Lamb 2: Return of The Lamb", "...whose fleece was white as snow", "Mary vs Lamb 3: The Resurrection"
		, "Snow! Bloody snow!", "the who", "what the foo"
	};
	const char * dQueries[] = {
		"seven !(angels !by)", // matched by 0-2
		"Mary lamb", // matched by 3-5
		"(the who) | (the foo)", // matched by 6
	};

	tCol.m_sName = "id";
	tCol.m_eAttrType = SPH_ATTR_BIGINT;
	tSrcSchema.AddAttr ( tCol, true );

	tCol.m_sName = "idd";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	auto pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt", false, 32, nullptr, sError );

	auto pSrc = new MockTestDoc_c ( tSrcSchema, ( BYTE ** ) dFields, sizeof ( dFields ) / sizeof ( dFields[0] ) / 2
									, 2 );

	EXPECT_CALL ( *pSrc, Connect ( _ ) ).WillOnce ( Return ( true ) );
	EXPECT_CALL ( *pSrc, GetFieldLengths () ).Times ( 7 ).WillRepeatedly ( Return ( pSrc->m_dFieldLengths.Begin () ) );
	EXPECT_CALL ( *pSrc, Disconnect () );

	pSrc->SetTokenizer ( pTok );
	pSrc->SetDict ( pDict );

	pSrc->Setup ( CSphSourceSettings(), nullptr );
	ASSERT_TRUE ( pSrc->Connect ( sError ) );
	ASSERT_TRUE ( pSrc->IterateStart ( sError ) );

	ASSERT_TRUE ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	for ( int i=0; i<tSrcSchema.GetFieldsCount(); i++ )
		tSchema.AddField ( tSrcSchema.GetField(i) );

	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

	auto pIndex = sphCreateIndexRT ( tSchema, "testrt", 128 * 1024, RT_INDEX_FILE_NAME, false );

	pIndex->SetTokenizer ( pTok ); // index will own this pair from now on
	pIndex->SetDictionary ( sphCreateDictionaryCRC ( tDictSettings, nullptr, pTok, "rt", false, 32, nullptr, sError ) );
	pIndex->PostSetup ();
	StrVec_t dWarnings;
	Verify ( pIndex->Prealloc ( false, nullptr, dWarnings ) );

	CSphString sFilter;
	InsertDocData_t tDoc ( pIndex->GetMatchSchema() );
	int iDynamic = pIndex->GetMatchSchema().GetRowSize();

	RtAccum_t tAcc;
	bool bEOF = false;
	while (true)
	{
		Verify ( pSrc->IterateDocument ( bEOF, sError ) );
		if ( bEOF )
			break;

		tDoc.m_dFields = pSrc->GetFields();
		tDoc.m_tDoc.Combine ( pSrc->m_tDocInfo, iDynamic );
		pIndex->AddDocument ( tDoc, false, sFilter, sError, sWarning, &tAcc );
	}
	pIndex->Commit ( nullptr, &tAcc );
	pSrc->Disconnect ();

	CSphQuery tQuery;
	CSphQueryItem &tFactor = tQuery.m_dItems.Add ();
	tFactor.m_sExpr = "packedfactors()";
	tFactor.m_sAlias = "pf";
	tQuery.m_sRankerExpr = "1";
	tQuery.m_eRanker = SPH_RANK_EXPR;
	tQuery.m_eMode = SPH_MATCH_EXTENDED2;
	tQuery.m_eSort = SPH_SORT_EXTENDED;
	tQuery.m_sSortBy = "@weight desc";
	tQuery.m_sOrderBy = "@weight desc";
	auto pParser = sphCreatePlainQueryParser();
	tQuery.m_pQueryParser = pParser.get();
	AggrResult_t tResult;
	CSphQueryResult tQueryResult;
	tQueryResult.m_pMeta = &tResult;
	CSphMultiQueryArgs tArgs ( 1 );
	tArgs.m_uPackedFactorFlags = SPH_FACTOR_ENABLE | SPH_FACTOR_CALC_ATC;

	SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema () );
	tQueueSettings.m_bComputeItems = true;
	SphQueueRes_t tRes;

	for ( auto szQuery : dQueries )
	{
		tQuery.m_sQuery = szQuery;

		auto pSorter = sphCreateQueue ( tQueueSettings, tQuery, tResult.m_sError, tRes );
		ASSERT_TRUE ( pSorter );
		ASSERT_TRUE ( pIndex->MultiQuery ( tQueryResult, tQuery, { &pSorter, 1 }, tArgs ) );
		auto & tOneRes = tResult.m_dResults.Add ();
		tOneRes.FillFromSorter ( pSorter );

		tResult.m_tSchema = *pSorter->GetSchema ();
		const CSphAttrLocator &tLoc = tResult.m_tSchema.GetAttr ( "pf" )->m_tLocator;

		for ( int iMatch = 0; iMatch<tOneRes.m_dMatches.GetLength (); ++iMatch )
		{
			const BYTE * pAttr = (const BYTE *) tOneRes.m_dMatches[iMatch].GetAttr ( tLoc );
			ASSERT_TRUE ( pAttr );

			auto * pFactors = (const unsigned int *) sphUnpackPtrAttr ( pAttr ).first;

			SPH_UDF_FACTORS tUnpacked;
			sphinx_factors_init ( &tUnpacked );
			sphinx_factors_unpack ( pFactors, &tUnpacked );

			// doc level factors
			ASSERT_EQ ( tUnpacked.doc_bm25, sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_BM25 ) );
			ASSERT_EQ ( tUnpacked.doc_bm25a, sphinx_get_doc_factor_float ( pFactors, SPH_DOCF_BM25A ) );
			ASSERT_EQ ( tUnpacked.matched_fields, sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_MATCHED_FIELDS ) );
			ASSERT_EQ ( tUnpacked.doc_word_count, sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_DOC_WORD_COUNT ) );
			ASSERT_EQ ( tUnpacked.num_fields, sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_NUM_FIELDS ) );
			ASSERT_EQ ( tUnpacked.max_uniq_qpos, sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_MAX_UNIQ_QPOS ) );

			// field level factors
			for ( int iField = 0; iField<tUnpacked.num_fields; ++iField )
			{
				if ( !tUnpacked.field[iField].hit_count )
					continue;

				const unsigned int * pField = sphinx_get_field_factors ( pFactors, iField );
				ASSERT_TRUE ( pField );
				ASSERT_EQ ( tUnpacked.field[iField].hit_count, sphinx_get_field_factor_int ( pField
																							 , SPH_FIELDF_HIT_COUNT ) );
				ASSERT_EQ ( tUnpacked.field[iField].lcs, sphinx_get_field_factor_int ( pField, SPH_FIELDF_LCS ) );
				ASSERT_EQ ( tUnpacked.field[iField].word_count, sphinx_get_field_factor_int ( pField
																							  , SPH_FIELDF_WORD_COUNT ) );
				ASSERT_EQ ( tUnpacked.field[iField].tf_idf, sphinx_get_field_factor_float ( pField
																							, SPH_FIELDF_TF_IDF ) );
				ASSERT_EQ ( tUnpacked.field[iField].min_idf, sphinx_get_field_factor_float ( pField
																							 , SPH_FIELDF_MIN_IDF ) );
				ASSERT_EQ (
					tUnpacked.field[iField].max_idf, sphinx_get_field_factor_float ( pField, SPH_FIELDF_MAX_IDF ) );
				ASSERT_EQ (
					tUnpacked.field[iField].sum_idf, sphinx_get_field_factor_float ( pField, SPH_FIELDF_SUM_IDF ) );
				ASSERT_EQ ( tUnpacked.field[iField].min_hit_pos, sphinx_get_field_factor_int ( pField
																							   , SPH_FIELDF_MIN_HIT_POS ) );
				ASSERT_EQ ( tUnpacked.field[iField].min_best_span_pos, sphinx_get_field_factor_int ( pField
																									 , SPH_FIELDF_MIN_BEST_SPAN_POS ) );
				ASSERT_EQ ( tUnpacked.field[iField].max_window_hits, sphinx_get_field_factor_int ( pField
																								   , SPH_FIELDF_MAX_WINDOW_HITS ) );
				ASSERT_EQ (
					tUnpacked.field[iField].min_gaps, sphinx_get_field_factor_int ( pField, SPH_FIELDF_MIN_GAPS ) );
				ASSERT_EQ ( tUnpacked.field[iField].atc, sphinx_get_field_factor_float ( pField, SPH_FIELDF_ATC ) );
				ASSERT_EQ ( tUnpacked.field[iField].lccs, sphinx_get_field_factor_int ( pField, SPH_FIELDF_LCCS ) );
				ASSERT_EQ ( tUnpacked.field[iField].wlccs, sphinx_get_field_factor_float ( pField, SPH_FIELDF_WLCCS ) );
				bool bExactHitSame = ( ( ( tUnpacked.field[iField].exact_hit << iField )
					& sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_EXACT_HIT_MASK ) )!=0 );
				ASSERT_TRUE ( tUnpacked.field[iField].exact_hit==0 || bExactHitSame );
				bool bExactOrderSame = ( ( ( tUnpacked.field[iField].exact_order << iField )
					& sphinx_get_doc_factor_int ( pFactors, SPH_DOCF_EXACT_ORDER_MASK ) )!=0 );
				ASSERT_TRUE ( tUnpacked.field[iField].exact_order==0 || bExactOrderSame );
			}

			// term level factors
			for ( int iWord = 0; iWord<tUnpacked.max_uniq_qpos; iWord++ )
			{
				if ( !tUnpacked.term[iWord].keyword_mask )
					continue;

				const unsigned int * pTerm = sphinx_get_term_factors ( pFactors, iWord + 1 );
				ASSERT_TRUE ( pTerm );
				ASSERT_EQ ( tUnpacked.term[iWord].tf, sphinx_get_term_factor_int ( pTerm, SPH_TERMF_TF ) );
				ASSERT_EQ ( tUnpacked.term[iWord].idf, sphinx_get_term_factor_float ( pTerm, SPH_TERMF_IDF ) );
			}

			sphinx_factors_deinit ( &tUnpacked );

			SafeDelete ( pSorter );
		}
	}

	SafeDelete ( pSrc );
	pTok = nullptr; // owned and deleted by index
	});
}


TEST_F ( RT, SendVsMerge )
{
	using namespace testing;
	Threads::CallCoroutine ( [&] {

	auto pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt", false, 32, nullptr, sError );

	tCol.m_sName = "id";
	tCol.m_eAttrType = SPH_ATTR_BIGINT;
	tSrcSchema.AddAttr ( tCol, true );

	tCol.m_sName = "tag1";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	tCol.m_sName = "tag2";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	auto pSrc = new MockDocRandomizer_c ( tSrcSchema );

	EXPECT_CALL ( *pSrc, Connect ( _ ) ).WillOnce ( Return ( true ) );
	EXPECT_CALL ( *pSrc, GetFieldLengths () ).Times ( 801 ).WillRepeatedly ( Return ( pSrc->m_dFieldLengths ) );
	EXPECT_CALL ( *pSrc, Disconnect () );

	pSrc->SetTokenizer ( pTok );
	pSrc->SetDict ( pDict );

	pSrc->Setup ( CSphSourceSettings(), nullptr );
	ASSERT_TRUE ( pSrc->Connect ( sError ) );
	ASSERT_TRUE ( pSrc->IterateStart ( sError ) );

	ASSERT_TRUE ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	for ( int i=0; i<tSrcSchema.GetFieldsCount(); i++ )
		tSchema.AddField ( tSrcSchema.GetField(i) );

	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

	auto pIndex = sphCreateIndexRT ( tSchema, "testrt", 128 * 1024, RT_INDEX_FILE_NAME, false );

	pIndex->SetTokenizer ( pTok ); // index will own this pair from now on
	pIndex->SetDictionary ( pDict );
	pIndex->PostSetup ();
	StrVec_t dWarnings;
	ASSERT_TRUE ( pIndex->Prealloc ( false, nullptr, dWarnings ) );

	CSphQuery tQuery;
	AggrResult_t tResult;
	CSphQueryResult tQueryResult;
	tQueryResult.m_pMeta = &tResult;
	CSphMultiQueryArgs tArgs ( 1 );
	tQuery.m_sQuery = "@title cat";
	auto pParser = sphCreatePlainQueryParser();
	tQuery.m_pQueryParser = pParser.get();

	CSphQueryItem & tItem = tQuery.m_dItems.Add ();
	tItem.m_sExpr = "*";
	tItem.m_sAlias = "*";
	tQuery.m_sSelect = "*";

	SphQueueSettings_t tQueueSettings ( pIndex->GetMatchSchema () );
	tQueueSettings.m_bComputeItems = true;
	SphQueueRes_t tRes;
	auto pSorter = sphCreateQueue ( tQueueSettings, tQuery, tResult.m_sError, tRes );
	ASSERT_TRUE ( pSorter );

	CSphString sFilter;
	InsertDocData_t tDoc ( pIndex->GetMatchSchema() );
	int iDynamic = pIndex->GetMatchSchema().GetRowSize();

	RtAccum_t tAcc;

	bool bEOF = false;
	while (true)
	{
		ASSERT_TRUE ( pSrc->IterateDocument ( bEOF, sError ) );
		if ( bEOF )
			break;

		tDoc.m_dFields = pSrc->GetFields();
		tDoc.m_tDoc.Combine ( pSrc->m_tDocInfo, iDynamic );
		pIndex->AddDocument ( tDoc, false, sFilter, sError, sWarning, &tAcc );
		sError = ""; // need to reset error message
		if ( pSrc->m_iDocsCounter==350 )
		{
			pIndex->Commit ( NULL, &tAcc );
			EXPECT_TRUE ( pIndex->MultiQuery ( tQueryResult, tQuery, { &pSorter, 1 }, tArgs ) );
			auto & tOneRes = tResult.m_dResults.Add ();
			tOneRes.FillFromSorter ( pSorter );
		}
	}
	pIndex->Commit ( NULL, &tAcc );

	pSrc->Disconnect ();

	tResult.m_tSchema = *pSorter->GetSchema ();

	auto & tOneRes = tResult.m_dResults.First ();
	ASSERT_EQ ( tResult.GetLength (), 20 );
	for ( int i = 0; i<tResult.GetLength (); ++i )
	{
		const RowID_t uID = tOneRes.m_dMatches[i].m_tRowID;
		const SphAttr_t tTag1 = tOneRes.m_dMatches[i].GetAttr ( tResult.m_tSchema.GetAttr ( 0 ).m_tLocator );
		const SphAttr_t tTag2 = tOneRes.m_dMatches[i].GetAttr ( tResult.m_tSchema.GetAttr ( 1 ).m_tLocator );
		ASSERT_TRUE ( ( RowID_t ) tTag1==uID + 1000 );
		ASSERT_TRUE ( tTag2==1313 );
	}

	SafeDelete ( pSorter );
	SafeDelete ( pSrc );
	pTok = nullptr; // owned and deleted by index
	});
}
