//
// Copyright (c) 2022, Manticore Software LTD (https://manticoresearch.com)
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

#include "schema/columninfo.h"
#include "schema/schema.h"
#include "match.h"
#include "sphinxexpr.h"
#include "sphinxdefs.h"
#include "std/generics.h"
#include <cmath>

#if defined( __GNUC__ ) || defined( __clang__ )
#define NOINLINE __attribute__ ( ( noinline ) )
#elif _WIN32
#define NOINLINE __declspec(noinline)
#endif

#define AAA float(tMatch.m_pStatic[0])
#define BBB float(tMatch.m_pStatic[1])
#define CCC float(tMatch.m_pStatic[2])

NOINLINE float ExprNative1 ( const CSphMatch & tMatch )	{ return AAA+BBB*CCC-1.0f;}
NOINLINE float ExprNative2 ( const CSphMatch & tMatch )	{ return AAA+BBB*CCC*2.0f-3.0f/4.0f*5.0f/6.0f*BBB; }
NOINLINE float ExprNative3 ( const CSphMatch & )		{ return (float)sqrt ( 2.0f ); }

struct ExprBench_t
{
	const char* m_sExpr;
	float ( *m_pFunc ) ( const CSphMatch& );
};
ExprBench_t dBench[] =
	{
		{ "aaa+bbb*(ccc)-1", ExprNative1 },
		{ "aaa+bbb*ccc*2-3/4*5/6*bbb", ExprNative2 },
		{ "sqrt(2)", ExprNative3 } };

class BM_expressions: public benchmark::Fixture
{
public:
	void SetUp ( const ::benchmark::State& state ) override
	{
		NBENCH = state.range ( 0 );
		CSphColumnInfo tCol;
		tCol.m_eAttrType = SPH_ATTR_INTEGER;
		tCol.m_sName = "aaa";
		tSchema.AddAttr ( tCol, false );
		tCol.m_sName = "bbb";
		tSchema.AddAttr ( tCol, false );
		tCol.m_sName = "ccc";
		tSchema.AddAttr ( tCol, false );

		pRow = new CSphRowitem[tSchema.GetRowSize()];
		for ( int i = 0; i < tSchema.GetRowSize(); i++ )
			pRow[i] = 1 + i;

		tMatch.m_tRowID = 123;
		tMatch.m_iWeight = 456;
		tMatch.m_pStatic = pRow;
		tExprArgs.m_pAttrType = &uType;
	}

	void TearDown ( const ::benchmark::State& state ) override
	{
		SafeDeleteArray ( pRow );
	}

	int64_t NBENCH = 0;
	CSphSchema tSchema;
	CSphRowitem* pRow = nullptr;
	CSphMatch tMatch;
	ESphAttr uType;
	CSphString sError;
	ExprParseArgs_t tExprArgs;
};

BENCHMARK_DEFINE_F ( BM_expressions, floats )
( benchmark::State& st )
{
	ISphExprRefPtr_c pExpr ( sphExprParse ( dBench[NBENCH].m_sExpr, tSchema, sError, tExprArgs ) );
	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( pExpr->Eval ( tMatch ) );
	}
	st.SetItemsProcessed ( st.iterations() );
	st.SetLabel ( dBench[NBENCH].m_sExpr );
}


BENCHMARK_DEFINE_F ( BM_expressions, ints )
( benchmark::State& st )
{
	ISphExprRefPtr_c pExpr ( sphExprParse ( dBench[NBENCH].m_sExpr, tSchema, sError, tExprArgs ) );
//	if ( uType != SPH_ATTR_INTEGER ) {
//		st.SkipWithError ( "attr is NOT integer" );
//		return;
//	}

	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( pExpr->IntEval ( tMatch ) );
	}
	st.SetItemsProcessed ( st.iterations() );
	st.SetLabel ( dBench[NBENCH].m_sExpr );
}

BENCHMARK_DEFINE_F ( BM_expressions, natives )
( benchmark::State& st )
{
	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( dBench[NBENCH].m_pFunc ( tMatch ) );
	}
	st.SetItemsProcessed ( st.iterations() );
	st.SetLabel ( dBench[NBENCH].m_sExpr );
}

BENCHMARK_REGISTER_F ( BM_expressions, floats )->DenseRange ( 0, 2, 1 );
BENCHMARK_REGISTER_F ( BM_expressions, ints )->Arg ( 0 ); // the only integer here
//BENCHMARK_REGISTER_F ( BM_expressions, floats )->DenseRange ( 0, 2, 1 );
BENCHMARK_REGISTER_F ( BM_expressions, natives )->DenseRange ( 0, 2, 1 );
