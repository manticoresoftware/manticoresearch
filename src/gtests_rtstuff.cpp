//
// Created by alexey on 09.08.17.
//

#include <gtest/gtest.h>

#include "sphinxint.h"

#include <gmock/gmock.h>


//////////////////////////////////////////////////////////////////////////
static void DeleteIndexFiles ( const char * sIndex )
{
	if ( !sIndex )
		return;

	const char * sExts[] = {
		"kill", "lock", "meta", "ram", "0.spa", "0.spd", "0.spe", "0.sph", "0.spi", "0.spk", "0.spm", "0.spp"
		, "0.sps" };

	CSphString sName;
	for ( int i = 0; i<( int ) ( sizeof ( sExts ) / sizeof ( sExts[0] ) ); i++ )
	{
		sName.SetSprintf ( "%s.%s", sIndex, sExts[i] );
		unlink ( sName.cstr () );
	}
}

void TestRTInit ()
{
	CSphConfigSection tRTConfig;

	sphRTInit ( tRTConfig, true, nullptr );
	sphRTConfigure ( tRTConfig, true );

	SmallStringHash_T<CSphIndex *> hIndexes;
	BinlogFlushInfo_t tBinlogFlush;
	sphReplayBinlog ( hIndexes, 0, nullptr, tBinlogFlush );
}

#define RT_INDEX_FILE_NAME "test_temp"

class MockTestDoc_c : public CSphSource_Document
{
public:
	explicit MockTestDoc_c ( const CSphSchema &tSchema, BYTE ** ppDocs, int iDocs, int iFields )
		: CSphSource_Document ( "test_doc" )
	{
		m_tSchema = tSchema;
		m_ppDocs = ppDocs;
		m_iDocCount = iDocs;
		m_iFields = iFields;
		m_dFieldLengths.Resize ( m_iFields );
	}

	virtual BYTE ** NextDocument ( CSphString & )
	{
		if ( m_tDocInfo.m_uDocID>=( SphDocID_t ) m_iDocCount )
		{
			m_tDocInfo.m_uDocID = 0;
			return NULL;
		}

		int iDoc = ( int ) m_tDocInfo.m_uDocID;
		m_tDocInfo.m_uDocID++;
		for ( int i = 0; i<m_iFields; i++ )
		{
			char * szField = ( char * ) ( m_ppDocs + iDoc * m_iFields )[i];
			m_dFieldLengths[i] = strlen ( szField );
		}

		return m_ppDocs + iDoc * m_iFields;
	}

	MOCK_CONST_METHOD0( GetFieldLengths, const int *() ); // return m_dFieldLengths.Begin();
	MOCK_METHOD1 ( Connect, bool ( CSphString & ) ); // return true;
	MOCK_METHOD0 ( Disconnect, void() );
	MOCK_METHOD0 ( HasAttrsConfigured, bool() ); // return true

	bool IterateStart ( CSphString & )
	{
		m_tDocInfo.Reset ( m_tSchema.GetRowSize () );
		m_iPlainFieldsLength = m_tSchema.GetFieldsCount();
		return true;
	}

	MOCK_METHOD2 ( IterateMultivaluedStart, bool ( int, CSphString& )); // return false;
	MOCK_METHOD0 ( IterateMultivaluedNext, bool()); // return false;
	MOCK_METHOD1 ( IterateFieldMVAStart, SphRange_t ( int ) ); // return false
	MOCK_METHOD0 ( IterateFieldMVANext, bool() ); // return false;
	MOCK_METHOD1 ( IterateKillListStart, bool (CSphString & ) ); // return false;
	MOCK_METHOD1 ( IterateKillListNext, bool (SphDocID_t & ) ) ; // return false
	int GetFieldCount () const { return m_iFields; }

	const char ** GetFields ()
	{ return ( const char ** ) ( m_ppDocs + ( m_tDocInfo.m_uDocID - 1 ) * m_iFields ); }

	int m_iDocCount;
	int m_iFields;
	BYTE ** m_ppDocs;
	CSphVector<int> m_dFieldLengths;
};


class MockDocRandomizer_c : public CSphSource_Document
{
public:
	static const int m_iMaxFields = 2;
	static const int m_iMaxFieldLen = 512;
	char m_dFields[m_iMaxFields][m_iMaxFieldLen];
	BYTE * m_ppFields[m_iMaxFields];
	int m_dFieldLengths[m_iMaxFields];

	explicit MockDocRandomizer_c ( const CSphSchema & tSchema ) : CSphSource_Document ( "test_doc" )
	{
		m_tSchema = tSchema;
		for ( int i=0; i<m_iMaxFields; i++ )
			m_ppFields[i] = (BYTE *)&m_dFields[i];
	}

	virtual BYTE ** NextDocument ( CSphString & )
	{
		if ( m_tDocInfo.m_uDocID>800 )
		{
			m_tDocInfo.m_uDocID = 0;
			return NULL;
		}

		m_tDocInfo.m_uDocID++;

		m_tDocInfo.SetAttr ( m_tSchema.GetAttr(0).m_tLocator, m_tDocInfo.m_uDocID+1000 );
		m_tDocInfo.SetAttr ( m_tSchema.GetAttr(1).m_tLocator, 1313 );

		snprintf ( m_dFields[0], m_iMaxFieldLen, "cat title%d title%d title%d title%d title%d"
			, sphRand(), sphRand(), sphRand(), sphRand(), sphRand() );

		snprintf ( m_dFields[1], m_iMaxFieldLen, "dog contentwashere%d contentwashere%d contentwashere%d contentwashere%d contentwashere%d"
			, sphRand(), sphRand(), sphRand(), sphRand(), sphRand() );

		for ( int i=0; i < m_iMaxFields; i++ )
			m_dFieldLengths[i] = strlen ( (char*)m_ppFields[i] );

		return &m_ppFields[0];
	}


	MOCK_CONST_METHOD0( GetFieldLengths, const int *() ); // return m_dFieldLengths.Begin();
	MOCK_METHOD1 ( Connect, bool ( CSphString & ) ); // return true;
	MOCK_METHOD0 ( Disconnect, void () );
	MOCK_METHOD0 ( HasAttrsConfigured, bool () ); // return true

	bool IterateStart ( CSphString & )
	{
		m_tDocInfo.Reset ( m_tSchema.GetRowSize () );
		m_iPlainFieldsLength = m_tSchema.GetFieldsCount();
		return true;
	}

	MOCK_METHOD2 ( IterateMultivaluedStart, bool ( int, CSphString & ) ); // return false;
	MOCK_METHOD0 ( IterateMultivaluedNext, bool () ); // return false;
	MOCK_METHOD1 ( IterateFieldMVAStart, SphRange_t ( int ) ); // return false
	MOCK_METHOD0 ( IterateFieldMVANext, bool () ); // return false;
	MOCK_METHOD1 ( IterateKillListStart, bool (CSphString & ) ); // return false;
	MOCK_METHOD1 ( IterateKillListNext, bool (SphDocID_t & ) ); // return false
	int  GetFieldCount () const { return m_iMaxFields; }
	const char ** GetFields () { return (const char **)( m_ppFields ); }
};


//////////////////////////////////////////////////////////////////////////



class RT : public ::testing::Test
{

protected:
	virtual void SetUp ()
	{
		DeleteIndexFiles ( RT_INDEX_FILE_NAME );
		TestRTInit();
		tDictSettings.m_bWordDict = false;

		pTok = sphCreateUTF8Tokenizer ();

		tSrcSchema.Reset ();
		tSrcSchema.AddField ( "title" );
		tSrcSchema.AddField ( "content" );
	}

	virtual void TearDown ()
	{
		SafeDelete ( pTok );
		sphRTDone ();
		DeleteIndexFiles ( RT_INDEX_FILE_NAME );
	}

	CSphColumnInfo tCol;
	CSphSchema tSrcSchema;
	CSphString sError, sWarning;

	ISphTokenizer * pTok = nullptr;

	CSphDictSettings tDictSettings;
};

class RTN : public RT, public ::testing::WithParamInterface<DWORD>
{};

TEST_P ( RTN, WeightBoundary )
{
	using namespace testing;
	auto pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "weight", sError );

	tCol.m_sName = "channel_id";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	const char * dFields[] = { "If I were a cat...", "We are the greatest cat" };
	//SphTestDoc_c * pSrc = new SphTestDoc_c ( tSrcSchema, ( BYTE ** ) dFields, 1, 2 );
	MockTestDoc_c * pSrc = new MockTestDoc_c ( tSrcSchema, ( BYTE ** ) dFields, 1, 2 );

	EXPECT_CALL ( *pSrc, Connect ( _ ) ).WillOnce ( Return ( true ) );
	EXPECT_CALL ( *pSrc, GetFieldLengths () ).WillOnce ( Return ( pSrc->m_dFieldLengths.Begin () ) );
	EXPECT_CALL ( *pSrc, Disconnect () );

	pSrc->SetTokenizer ( pTok );
	pSrc->SetDict ( pDict );
	pSrc->Setup ( CSphSourceSettings() );

	EXPECT_TRUE ( pSrc->Connect ( sError ) );
	EXPECT_TRUE ( pSrc->IterateStart ( sError ) );
	EXPECT_TRUE ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	for ( int i=0; i<tSrcSchema.GetFieldsCount(); i++ )
		tSchema.AddField ( tSrcSchema.GetField(i) );

	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

	ISphRtIndex * pIndex = sphCreateIndexRT ( tSchema, "testrt", 32 * 1024 * 1024, RT_INDEX_FILE_NAME, false );

	// tricky bit
	// index owns its tokenizer/dict pair, and MAY do whatever it wants
	// and starting with meta v4, it WILL deallocate tokenizer/dict in Prealloc()
	// in favor of tokenizer/dict loaded from the saved settings in meta
	// however, source still needs those guys!
	// so for simplicity i just clone them
	pIndex->SetTokenizer ( pTok->Clone ( SPH_CLONE_INDEX ) );
	pIndex->SetDictionary ( pDict->Clone () );
	pIndex->PostSetup ();
	EXPECT_TRUE ( pIndex->Prealloc ( false ) );

	CSphVector<DWORD> dMvas;
	CSphString sFilter;
	while (true)
	{
		EXPECT_TRUE ( pSrc->IterateDocument ( sError ) );
		if ( !pSrc->m_tDocInfo.m_uDocID )
			break;

		pIndex->AddDocument ( pIndex->CloneIndexingTokenizer (), pSrc->GetFieldCount (), pSrc->GetFields ()
							  , pSrc->m_tDocInfo, false, sFilter, NULL, dMvas, sError, sWarning, NULL );
		pIndex->Commit ( NULL, NULL );
	}

	pSrc->Disconnect ();

	ASSERT_EQ ( pSrc->GetStats ().m_iTotalDocuments, 1) << "docs committed";

	CSphQuery tQuery;
	CSphQueryResult tResult;
	KillListVector kList;
	CSphMultiQueryArgs tArgs ( kList, 1) ;
	tQuery.m_sQuery = "@title cat";
	tQuery.m_pQueryParser = sphCreatePlainQueryParser();

	SphQueueSettings_t tQueueSettings ( tQuery, pIndex->GetMatchSchema (), tResult.m_sError, NULL );
	tQueueSettings.m_bComputeItems = false;
	ISphMatchSorter * pSorter = sphCreateQueue ( tQueueSettings );
	ASSERT_TRUE ( pSorter );
	ASSERT_TRUE ( pIndex->MultiQuery ( &tQuery, &tResult, 1, &pSorter, tArgs ) );
	sphFlattenQueue ( pSorter, &tResult, 0 );
	ASSERT_EQ ( tResult.m_dMatches.GetLength (), 1 ) << "results found";
	ASSERT_EQ ( tResult.m_dMatches[0].m_uDocID, 1 ) << "docID" ;
	ASSERT_EQ ( tResult.m_dMatches[0].m_iWeight, GetParam ()) << "weight" ;

	SafeDelete ( pSorter );
	SafeDelete ( tQuery.m_pQueryParser );
	SafeDelete ( pIndex );
	SafeDelete ( pDict );
	SafeDelete ( pSrc );
}

INSTANTIATE_TEST_CASE_P ( RT_N, RTN, ::testing::Values ( 1500, 1500, 1500, 1500, 1500 ) );


TEST_F ( RT, RankerFactors )
{
	using namespace testing;

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

	tCol.m_sName = "idd";
	tCol.m_eAttrType = SPH_ATTR_INTEGER;
	tSrcSchema.AddAttr ( tCol, true );

	auto pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt", sError );

	auto pSrc = new MockTestDoc_c ( tSrcSchema, ( BYTE ** ) dFields, sizeof ( dFields ) / sizeof ( dFields[0] ) / 2
									, 2 );

	EXPECT_CALL ( *pSrc, Connect ( _ ) ).WillOnce ( Return ( true ) );
	EXPECT_CALL ( *pSrc, GetFieldLengths () ).Times ( 7 ).WillRepeatedly ( Return ( pSrc->m_dFieldLengths.Begin () ) );
	EXPECT_CALL ( *pSrc, Disconnect () );

	pSrc->SetTokenizer ( pTok );
	pSrc->SetDict ( pDict );

	pSrc->Setup ( CSphSourceSettings () );
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
	pIndex->SetDictionary ( sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt", sError ) );
	pIndex->PostSetup ();
	Verify ( pIndex->Prealloc ( false ) );

	CSphString sFilter;
	CSphVector<DWORD> dMvas;
	while (true)
	{
		Verify ( pSrc->IterateDocument ( sError ) );
		if ( !pSrc->m_tDocInfo.m_uDocID )
			break;

		pIndex->AddDocument ( pIndex->CloneIndexingTokenizer (), pSrc->GetFieldCount (), pSrc->GetFields ()
							  , pSrc->m_tDocInfo, false, sFilter, NULL, dMvas, sError, sWarning, NULL );
	}
	pIndex->Commit ( NULL, NULL );
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
	tQuery.m_pQueryParser = sphCreatePlainQueryParser();
	CSphQueryResult tResult;
	KillListVector tKill;
	CSphMultiQueryArgs tArgs ( tKill, 1 );
	SphQueueSettings_t tQueueSettings ( tQuery, pIndex->GetMatchSchema (), tResult.m_sError, NULL );
	tQueueSettings.m_bComputeItems = true;
	tArgs.m_uPackedFactorFlags = SPH_FACTOR_ENABLE | SPH_FACTOR_CALC_ATC;

	for ( auto szQuery : dQueries )
	{
		tQuery.m_sQuery = szQuery;

		auto pSorter = sphCreateQueue ( tQueueSettings );
		ASSERT_TRUE ( pSorter );
		ASSERT_TRUE ( pIndex->MultiQuery ( &tQuery, &tResult, 1, &pSorter, tArgs ) );
		sphFlattenQueue ( pSorter, &tResult, 0 );

		tResult.m_tSchema = *pSorter->GetSchema ();
		const CSphAttrLocator &tLoc = tResult.m_tSchema.GetAttr ( "pf" )->m_tLocator;

		for ( int iMatch = 0; iMatch<tResult.m_dMatches.GetLength (); iMatch++ )
		{
			const BYTE * pAttr = (const BYTE *)tResult.m_dMatches[iMatch].GetAttr ( tLoc );
			ASSERT_TRUE ( pAttr );

			sphUnpackPtrAttr ( pAttr, &pAttr );

			const unsigned int * pFactors = (const unsigned int *)pAttr;

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
			for ( int iField = 0; iField<tUnpacked.num_fields; iField++ )
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

	SafeDelete ( tQuery.m_pQueryParser );
	SafeDelete ( pIndex );
	SafeDelete ( pSrc );
	pTok = nullptr; // owned and deleted by index
}


TEST_F ( RT, SendVsMerge )
{
	using namespace testing;

	auto pDict = sphCreateDictionaryCRC ( tDictSettings, NULL, pTok, "rt", sError );

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

	pSrc->Setup ( CSphSourceSettings() );
	ASSERT_TRUE ( pSrc->Connect ( sError ) );
	ASSERT_TRUE ( pSrc->IterateStart ( sError ) );

	ASSERT_TRUE ( pSrc->UpdateSchema ( &tSrcSchema, sError ) );

	CSphSchema tSchema; // source schema must be all dynamic attrs; but index ones must be static
	for ( int i=0; i<tSrcSchema.GetFieldsCount(); i++ )
		tSchema.AddField ( tSrcSchema.GetField(i) );

	for ( int i=0; i<tSrcSchema.GetAttrsCount(); i++ )
		tSchema.AddAttr ( tSrcSchema.GetAttr(i), false );

	ISphRtIndex * pIndex = sphCreateIndexRT ( tSchema, "testrt", 128 * 1024, RT_INDEX_FILE_NAME, false );

	pIndex->SetTokenizer ( pTok ); // index will own this pair from now on
	pIndex->SetDictionary ( pDict );
	pIndex->PostSetup ();
	ASSERT_TRUE ( pIndex->Prealloc ( false ) );

	CSphQuery tQuery;
	CSphQueryResult tResult;
	KillListVector tKill;
	CSphMultiQueryArgs tArgs ( tKill, 1 );
	tQuery.m_sQuery = "@title cat";
	tQuery.m_pQueryParser = sphCreatePlainQueryParser();

	SphQueueSettings_t tQueueSettings ( tQuery, pIndex->GetMatchSchema (), tResult.m_sError, NULL );
	tQueueSettings.m_bComputeItems = false;
	auto pSorter = sphCreateQueue ( tQueueSettings );
	ASSERT_TRUE ( pSorter );

	CSphString sFilter;
	CSphVector<DWORD> dMvas;
	while (true)
	{
		ASSERT_TRUE ( pSrc->IterateDocument ( sError ) );
		if ( !pSrc->m_tDocInfo.m_uDocID )
			break;

		pIndex->AddDocument ( pIndex->CloneIndexingTokenizer (), pSrc->GetFieldCount (), pSrc->GetFields ()
							  , pSrc->m_tDocInfo, false, sFilter, NULL, dMvas, sError, sWarning, NULL );
		if ( pSrc->m_tDocInfo.m_uDocID==350 )
		{
			pIndex->Commit ( NULL, NULL );
			EXPECT_TRUE ( pIndex->MultiQuery ( &tQuery, &tResult, 1, &pSorter, tArgs ) );
			sphFlattenQueue ( pSorter, &tResult, 0 );
		}
	}
	pIndex->Commit ( NULL, NULL );

	pSrc->Disconnect ();

	tResult.m_tSchema = *pSorter->GetSchema ();

	for ( int i = 0; i<tResult.m_dMatches.GetLength (); i++ )
	{
		const SphDocID_t uID = tResult.m_dMatches[i].m_uDocID;
		const SphAttr_t tTag1 = tResult.m_dMatches[i].GetAttr ( tResult.m_tSchema.GetAttr ( 0 ).m_tLocator );
		const SphAttr_t tTag2 = tResult.m_dMatches[i].GetAttr ( tResult.m_tSchema.GetAttr ( 1 ).m_tLocator );
		ASSERT_TRUE ( ( SphDocID_t ) tTag1==uID + 1000 );
		ASSERT_TRUE ( tTag2==1313 );
	}

	SafeDelete ( tQuery.m_pQueryParser );
	SafeDelete ( pSorter );
	SafeDelete ( pIndex );
	SafeDelete ( pSrc );
	pTok = nullptr; // owned and deleted by index
}