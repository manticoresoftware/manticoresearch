//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>

#include "sphinxint.h"
#include "sphinxpq.h"


class PQ_merge : public ::testing::Test
{
protected:
	virtual void SetUp ()
	{
		dContexts[0].m_iDocsMatched = 3;
		dContexts[0].m_iEarlyPassed = 1;
		dContexts[0].m_iOnlyTerms = 1;
		dContexts[0].m_iQueriesFailed = 1;
		for ( auto i : { 0, 1, } )
		{
			dContexts[0].m_dQueryMatched.Add ( dQueries[i] );
			dContexts[0].m_dDt.Add ( i+1000 );
		}
		for ( auto i : {2, 10, 11, 1, 12}) dContexts[0].m_dDocsMatched.Add ( i );

		dContexts[1].m_iDocsMatched = 2;
		dContexts[1].m_iEarlyPassed = 1;
		dContexts[1].m_iOnlyTerms = 1;
		dContexts[1].m_iQueriesFailed = 0;
		for ( auto i : { 2, 3, } )
		{
			dContexts[1].m_dQueryMatched.Add ( dQueries[i] );
			dContexts[1].m_dDt.Add ( i + 2000 );
		}
		for ( auto i : { 1, 21, 1, 22 } ) dContexts[1].m_dDocsMatched.Add ( i );

		dContexts[2].m_iDocsMatched = 2;
		dContexts[2].m_iEarlyPassed = 1;
		dContexts[2].m_iOnlyTerms = 0;
		dContexts[2].m_iQueriesFailed = 0;
		for ( auto i : { 8, 9, } )
		{
			dContexts[2].m_dQueryMatched.Add ( dQueries[i] );
			dContexts[2].m_dDt.Add ( i + 3000 );
		}
		for ( auto i : { 1, 31, 1, 32 } ) dContexts[2].m_dDocsMatched.Add ( i );

		dResult.m_iTotalQueries = 10;
	}

	PercolateQueryDesc dQueries[10] {
		{   100, "query0", "tag0", "filter0", false }
		, { 101, "query1", "tag1", "filter1", false }
		, { 102, "query2", "tag2", "filter2", false }
		, { 103, "query3", "tag3", "filter3", false }
		, { 104, "query4", "tag4", "filter4", false }
		, { 150, "query5", "tag5", "filter5", false }
		, { 160, "query6", "tag6", "filter6", false }
		, { 170, "query7", "tag7", "filter7", false }
		, { 180, "query8", "tag8", "filter8", false }
		, { 190, "query9", "tag9", "filter9", false } };

	PQMatchContextResult_t dContexts[3];
	PercolateMatchResult_t dResult;

};

//////////////////////////////////////////////////////////////////////////
// one simple result, without details
TEST_F ( PQ_merge, JustOneResult )
{
	CSphVector<PQMatchContextResult_t *> dSrc;
	dSrc.Add ( &dContexts[0] );

	PercolateMergeResults ( dSrc, dResult );

	// general numbers
	ASSERT_EQ ( dResult.m_iEarlyOutQueries, 9 );
	ASSERT_EQ ( dResult.m_iOnlyTerms, 1 );
	ASSERT_EQ ( dResult.m_iQueriesFailed, 1 );
	ASSERT_EQ ( dResult.m_iQueriesMatched, 2 );
	ASSERT_EQ ( dResult.m_iDocsMatched, 3 );

	// ensure that without verbosity we have no DT
	ASSERT_FALSE ( dResult.m_bVerbose );
	ASSERT_TRUE ( dResult.m_dQueryDT.IsEmpty() );

	// ensure that if no docs requested, nothing returned
	ASSERT_FALSE ( dResult.m_bGetDocs );
	ASSERT_TRUE ( dResult.m_dDocs.IsEmpty () );

	// ensure we take correct matches
	ASSERT_EQ ( dResult.m_dQueryDesc.GetLength(), dResult.m_iQueriesMatched );
	ASSERT_EQ ( dResult.m_dQueryDesc[0].m_iQUID, 100 );
	ASSERT_EQ ( dResult.m_dQueryDesc[1].m_iQUID, 101 );
}

// one result, but with list of docs
TEST_F ( PQ_merge, JustOneResultWithDocs )
{
	CSphVector<PQMatchContextResult_t *> dSrc;
	dSrc.Add ( &dContexts[0] );

	dResult.m_bGetDocs = true;
	PercolateMergeResults ( dSrc, dResult );

	// general numbers
	ASSERT_EQ ( dResult.m_iEarlyOutQueries, 9 );
	ASSERT_EQ ( dResult.m_iOnlyTerms, 1 );
	ASSERT_EQ ( dResult.m_iQueriesFailed, 1 );
	ASSERT_EQ ( dResult.m_iQueriesMatched, 2 );
	ASSERT_EQ ( dResult.m_iDocsMatched, 3 );

	// ensure that without verbosity we have no DT
	ASSERT_FALSE ( dResult.m_bVerbose );
	ASSERT_TRUE ( dResult.m_dQueryDT.IsEmpty () );

	// ensure that if no docs requested, nothing returned
	ASSERT_TRUE ( dResult.m_bGetDocs );
	ASSERT_EQ ( dResult.m_dDocs.GetLength (), 5 );
	int j = 0; for ( auto i : { 2, 10, 11, 1, 12 }) ASSERT_EQ ( dResult.m_dDocs[j++], i);

	// ensure we take correct matches
	ASSERT_EQ ( dResult.m_dQueryDesc.GetLength (), dResult.m_iQueriesMatched );
	ASSERT_EQ ( dResult.m_dQueryDesc[0].m_iQUID, 100 );
	ASSERT_EQ ( dResult.m_dQueryDesc[1].m_iQUID, 101 );
}

// one result, but with times
TEST_F ( PQ_merge, JustOneResultVerbose )
{
	CSphVector<PQMatchContextResult_t *> dSrc;
	dSrc.Add ( &dContexts[0] );

	dResult.m_bVerbose = true;
	PercolateMergeResults ( dSrc, dResult );

	// general numbers
	ASSERT_EQ ( dResult.m_iEarlyOutQueries, 9 );
	ASSERT_EQ ( dResult.m_iOnlyTerms, 1 );
	ASSERT_EQ ( dResult.m_iQueriesFailed, 1 );
	ASSERT_EQ ( dResult.m_iQueriesMatched, 2 );
	ASSERT_EQ ( dResult.m_iDocsMatched, 3 );

	// ensure that without verbosity we have no DT
	ASSERT_TRUE ( dResult.m_bVerbose );
	ASSERT_EQ ( dResult.m_dQueryDT.GetLength (), dResult.m_iQueriesMatched );
	ASSERT_EQ ( dResult.m_dQueryDT[0], 1000 );
	ASSERT_EQ ( dResult.m_dQueryDT[1], 1001 );

	// ensure that if no docs requested, nothing returned
	ASSERT_FALSE ( dResult.m_bGetDocs );
	ASSERT_TRUE ( dResult.m_dDocs.IsEmpty () );

	// ensure we take correct matches
	ASSERT_EQ ( dResult.m_dQueryDesc.GetLength (), dResult.m_iQueriesMatched );
	ASSERT_EQ ( dResult.m_dQueryDesc[0].m_iQUID, 100 );
	ASSERT_EQ ( dResult.m_dQueryDesc[1].m_iQUID, 101 );
}

// one result, with everything
TEST_F ( PQ_merge, JustOneResultWithEverything )
{
	CSphVector<PQMatchContextResult_t *> dSrc;
	dSrc.Add ( &dContexts[0] );

	dResult.m_bVerbose = true;
	dResult.m_bGetDocs = true;
	PercolateMergeResults ( dSrc, dResult );

	// general numbers
	ASSERT_EQ ( dResult.m_iEarlyOutQueries, 9 );
	ASSERT_EQ ( dResult.m_iOnlyTerms, 1 );
	ASSERT_EQ ( dResult.m_iQueriesFailed, 1 );
	ASSERT_EQ ( dResult.m_iQueriesMatched, 2 );
	ASSERT_EQ ( dResult.m_iDocsMatched, 3 );

	// ensure that without verbosity we have no DT
	ASSERT_TRUE ( dResult.m_bVerbose );
	ASSERT_EQ ( dResult.m_dQueryDT.GetLength (), dResult.m_iQueriesMatched );
	ASSERT_EQ ( dResult.m_dQueryDT[0], 1000 );
	ASSERT_EQ ( dResult.m_dQueryDT[1], 1001 );

	// ensure that if no docs requested, nothing returned
	ASSERT_TRUE ( dResult.m_bGetDocs );
	ASSERT_EQ ( dResult.m_dDocs.GetLength (), 5 );
	int j = 0;
	for ( auto i : { 2, 10, 11, 1, 12 } )
		ASSERT_EQ ( dResult.m_dDocs[j++], i );

	// ensure we take correct matches
	ASSERT_EQ ( dResult.m_dQueryDesc.GetLength (), dResult.m_iQueriesMatched );
	ASSERT_EQ ( dResult.m_dQueryDesc[0].m_iQUID, 100 );
	ASSERT_EQ ( dResult.m_dQueryDesc[1].m_iQUID, 101 );
}

// full result, with everything
TEST_F ( PQ_merge, FullResult )
{
	CSphVector<PQMatchContextResult_t *> dSrc;
	dSrc.Add ( &dContexts[2] );
	dSrc.Add ( &dContexts[0] );
	dSrc.Add ( &dContexts[1] );

	PercolateMergeResults ( dSrc, dResult );

	// general numbers
	ASSERT_EQ ( dResult.m_iEarlyOutQueries, 7 );
	ASSERT_EQ ( dResult.m_iOnlyTerms, 2 );
	ASSERT_EQ ( dResult.m_iQueriesFailed, 1 );
	ASSERT_EQ ( dResult.m_iQueriesMatched, 6 );
	ASSERT_EQ ( dResult.m_iDocsMatched, 7 );

	// ensure that without verbosity we have no DT
	ASSERT_FALSE ( dResult.m_bVerbose );
	ASSERT_TRUE ( dResult.m_dQueryDT.IsEmpty () );

	// ensure that if no docs requested, nothing returned
	ASSERT_FALSE ( dResult.m_bGetDocs );
	ASSERT_TRUE ( dResult.m_dDocs.IsEmpty () );

	// ensure we take correct matches
	ASSERT_EQ ( dResult.m_dQueryDesc.GetLength (), dResult.m_iQueriesMatched );
	int j = 0;
	for ( auto qid : { 100, 101, 102, 103, 180, 190 } )
		ASSERT_EQ ( dResult.m_dQueryDesc[j++].m_iQUID, qid );
}

// full result, with everything
TEST_F ( PQ_merge, FullResultWithDocs )
{
	CSphVector<PQMatchContextResult_t *> dSrc;
	dSrc.Add ( &dContexts[2] );
	dSrc.Add ( &dContexts[0] );
	dSrc.Add ( &dContexts[1] );

	dResult.m_bGetDocs = true;
	PercolateMergeResults ( dSrc, dResult );

	// general numbers
	ASSERT_EQ ( dResult.m_iEarlyOutQueries, 7 );
	ASSERT_EQ ( dResult.m_iOnlyTerms, 2 );
	ASSERT_EQ ( dResult.m_iQueriesFailed, 1 );
	ASSERT_EQ ( dResult.m_iQueriesMatched, 6 );
	ASSERT_EQ ( dResult.m_iDocsMatched, 7 );

	// ensure that without verbosity we have no DT
	ASSERT_FALSE ( dResult.m_bVerbose );
	ASSERT_TRUE ( dResult.m_dQueryDT.IsEmpty () );

	// ensure that if no docs requested, nothing returned
	ASSERT_TRUE ( dResult.m_bGetDocs );
	ASSERT_EQ ( dResult.m_dDocs.GetLength (), 13 );
	int j = 0;
	for ( auto i : { 2, 10, 11, 1, 12, 1, 21, 1, 22, 1, 31, 1, 32 } )
		ASSERT_EQ ( dResult.m_dDocs[j++], i );

	// ensure we take correct matches
	ASSERT_EQ ( dResult.m_dQueryDesc.GetLength (), dResult.m_iQueriesMatched );
	j = 0;
	for ( auto qid : { 100, 101, 102, 103, 180, 190 } )
		ASSERT_EQ ( dResult.m_dQueryDesc[j++].m_iQUID, qid );
}

// full result, with everything
TEST_F ( PQ_merge, FullResultVerbose )
{
	CSphVector<PQMatchContextResult_t *> dSrc;
	dSrc.Add ( &dContexts[2] );
	dSrc.Add ( &dContexts[0] );
	dSrc.Add ( &dContexts[1] );

	dResult.m_bVerbose = true;
	PercolateMergeResults ( dSrc, dResult );

	// general numbers
	ASSERT_EQ ( dResult.m_iEarlyOutQueries, 7 );
	ASSERT_EQ ( dResult.m_iOnlyTerms, 2 );
	ASSERT_EQ ( dResult.m_iQueriesFailed, 1 );
	ASSERT_EQ ( dResult.m_iQueriesMatched, 6 );
	ASSERT_EQ ( dResult.m_iDocsMatched, 7 );

	// ensure that without verbosity we have no DT
	ASSERT_TRUE ( dResult.m_bVerbose );
	ASSERT_EQ ( dResult.m_dQueryDT.GetLength (), dResult.m_iQueriesMatched );
	int j = 0;
	for ( auto dt : { 1000, 1001, 2002, 2003, 3008, 3009 } )
		ASSERT_EQ ( dResult.m_dQueryDT[j++], dt );

	// ensure that if no docs requested, nothing returned
	ASSERT_FALSE ( dResult.m_bGetDocs );
	ASSERT_TRUE ( dResult.m_dDocs.IsEmpty () );

	// ensure we take correct matches
	ASSERT_EQ ( dResult.m_dQueryDesc.GetLength (), dResult.m_iQueriesMatched );
	j = 0;
	for ( auto qid : { 100, 101, 102, 103, 180, 190 } )
		ASSERT_EQ ( dResult.m_dQueryDesc[j++].m_iQUID, qid );
}

// full result, with everything
TEST_F ( PQ_merge, FullResultWithEverything )
{
	CSphVector<PQMatchContextResult_t *> dSrc;
	dSrc.Add ( &dContexts[2] );
	dSrc.Add ( &dContexts[0] );
	dSrc.Add ( &dContexts[1] );

	dResult.m_bVerbose = true;
	dResult.m_bGetDocs = true;
	PercolateMergeResults ( dSrc, dResult );

	// general numbers
	ASSERT_EQ ( dResult.m_iEarlyOutQueries, 7 );
	ASSERT_EQ ( dResult.m_iOnlyTerms, 2 );
	ASSERT_EQ ( dResult.m_iQueriesFailed, 1 );
	ASSERT_EQ ( dResult.m_iQueriesMatched, 6 );
	ASSERT_EQ ( dResult.m_iDocsMatched, 7 );

	// ensure that without verbosity we have no DT
	ASSERT_TRUE ( dResult.m_bVerbose );
	ASSERT_EQ ( dResult.m_dQueryDT.GetLength (), dResult.m_iQueriesMatched );
	int j = 0;
	for ( auto dt : { 1000, 1001, 2002, 2003, 3008, 3009 } ) ASSERT_EQ ( dResult.m_dQueryDT[j++], dt );

	// ensure that if no docs requested, nothing returned
	ASSERT_TRUE ( dResult.m_bGetDocs );
	ASSERT_EQ ( dResult.m_dDocs.GetLength (), 13 );
	j = 0;
	for ( auto i : { 2, 10, 11, 1, 12, 1, 21, 1, 22, 1, 31, 1, 32 } )
		ASSERT_EQ ( dResult.m_dDocs[j++], i );

	// ensure we take correct matches
	ASSERT_EQ ( dResult.m_dQueryDesc.GetLength (), dResult.m_iQueriesMatched );
	j=0;
	for ( auto qid : { 100, 101, 102, 103, 180, 190 } )
		ASSERT_EQ ( dResult.m_dQueryDesc[j++].m_iQUID, qid );
}
