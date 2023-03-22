//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "stackmock.h"
#include "sphinxexpr.h"
#include "coro_stack.h"
#include "coroutine.h"
#include "searchdsql.h"
#include "attribute.h"

// hard-coded definitions to avoid probing (that is - to avoid confusing memcheck programs)
// run searchd with --logdebug --console once, read values, then write them here and uncomment these lines
//#define KNOWN_CREATE_SIZE 4208
//#define KNOWN_EXPR_SIZE 48
//#define KNOWN_FILTER_SIZE 400

class StackMeasurer_c
{
protected:
	CSphFixedVector<BYTE> m_dMockStack { (int) Threads::DEFAULT_CORO_STACK_SIZE };
	int m_iComplexity;

protected:
	int CalcUsedStackEdge ( BYTE uFiller )
	{
		ARRAY_CONSTFOREACH ( i, m_dMockStack )
			if ( m_dMockStack[i]!=uFiller )
				return m_dMockStack.GetLength ()-i;

		return m_dMockStack.GetLength ();
	}

	void MockInitMem ( BYTE uFiller )
	{
		::memset ( m_dMockStack.begin (), uFiller, m_dMockStack.GetLengthBytes () );
	}

	int MeasureStackWithPattern ( BYTE uPattern )
	{
		MockInitMem ( uPattern );
		MockParseTest ();
		auto iUsedStackEdge = CalcUsedStackEdge ( uPattern );
		return sphRoundUp ( iUsedStackEdge, 4 );
	}

	int MeasureStack ()
	{
		auto iStartStackDE = MeasureStackWithPattern ( 0xDE );
		auto iStartStackAD = MeasureStackWithPattern ( 0xAD );
		return Max ( iStartStackDE, iStartStackAD );
	}

	virtual void BuildMockExpr ( int iComplexity ) = 0;
	virtual void MockParseTest () = 0;

	void BuildMockExprWrapper ( int iComplexity )
	{
		m_iComplexity = iComplexity + 1;
		BuildMockExpr ( iComplexity );
	}

public:
	int MockMeasureStack ( int iNodes )
	{
		BuildMockExprWrapper ( 0 );

		int iStartStack = MeasureStack ();
		int iDelta = 0;

		// Find edge of stack where expr length became visible
		// (we need quite big expr in order to touch deepest of the stack)
		int iHeight = 0;
		while ( iDelta<=0 )
		{
			++iHeight;
			BuildMockExprWrapper ( iHeight );
			auto iCurStack = MeasureStack ();
			iDelta = iCurStack - iStartStack;
		}

		iStartStack += iDelta;

		// add iNodes frames and average stack from them
		BuildMockExprWrapper ( iHeight + iNodes );

		auto iCurStack = MeasureStack ();
		iDelta = iCurStack-iStartStack;
		iDelta/=iNodes;
		iDelta = sphRoundUp ( iDelta, 16 );
		return iDelta;
	}

	virtual ~StackMeasurer_c () = default;
};

/////////////////////////////////////////////////////////////////////
/// calculate stack for expressions
class CreateExprStackSize_c : public StackMeasurer_c
{
	void BuildMockExpr ( int iComplexity ) final
	{
		m_sExpr.Clear();
		m_sExpr << "((attr_a=0)*1)";

		for ( int i = 1; i<iComplexity+1; ++i ) // ((attr_a=0)*1) + ((attr_b=1)*3) + ((attr_b=2)*5) + ...
			m_sExpr << "+((attr_b=" << i << ")*" << i * 2+1 << ")";
	}

	void MockParseTest () override
	{
		struct
		{
			ExprParseArgs_t m_tArgs;
			CSphString m_sError;
			CSphSchema m_tSchema;
			const char * m_sExpr = nullptr;
			bool m_bSuccess = false;
			ISphExpr * m_pExprBase = nullptr;
		} tParams;

		CSphColumnInfo tAttr;
		tAttr.m_eAttrType = SPH_ATTR_INTEGER;
		tAttr.m_sName = "attr_a";
		tParams.m_tSchema.AddAttr ( tAttr, false );
		tAttr.m_sName = "attr_b";
		tParams.m_tSchema.AddAttr ( tAttr, false );

		tParams.m_sExpr = m_sExpr.cstr();

		Threads::MockCallCoroutine ( m_dMockStack, [&tParams] {
			tParams.m_pExprBase = sphExprParse ( tParams.m_sExpr, tParams.m_tSchema, tParams.m_sError, tParams.m_tArgs );
		} );

		tParams.m_bSuccess = !!tParams.m_pExprBase;
		SafeRelease ( tParams.m_pExprBase );

		if ( !tParams.m_bSuccess || !tParams.m_sError.IsEmpty () )
			sphWarning ( "stack check expression error: %s", tParams.m_sError.cstr () );
	}

protected:
	StringBuilder_c m_sExpr;

public:
	static int MockMeasure();
	static void PublishValue (int iStack);
};

// measure stack for evaluate expression
class EvalExprStackSize_c : public CreateExprStackSize_c
{
	void MockParseTest () override
	{
		struct
		{
			ExprParseArgs_t m_tArgs;
			CSphString m_sError;
			CSphSchema m_tSchema;
			const char * m_sExpr = nullptr;
			bool m_bSuccess = false;
			ISphExpr * m_pExprBase = nullptr;
			CSphMatch m_tMatch;
		} tParams;

		CSphColumnInfo tAttr;
		tAttr.m_eAttrType = SPH_ATTR_INTEGER;
		tAttr.m_sName = "attr_a";
		tParams.m_tSchema.AddAttr ( tAttr, false );
		tAttr.m_sName = "attr_b";
		tParams.m_tSchema.AddAttr ( tAttr, false );

		CSphFixedVector<CSphRowitem> dRow { tParams.m_tSchema.GetRowSize () };
		auto * pRow = dRow.Begin();
		for ( int i = 1; i<tParams.m_tSchema.GetAttrsCount (); ++i )
			sphSetRowAttr ( pRow, tParams.m_tSchema.GetAttr ( i ).m_tLocator, i );
		sphSetRowAttr ( pRow, tParams.m_tSchema.GetAttr ( 0 ).m_tLocator, 123 );

		tParams.m_tMatch.m_tRowID = 123;
		tParams.m_tMatch.m_iWeight = 456;
		tParams.m_tMatch.m_pStatic = pRow;

		tParams.m_sExpr = m_sExpr.cstr();

		{ // parse in dedicated coro (hope, 100K frame per level should fit any arch)
		CSphFixedVector<BYTE> dSafeStack { m_iComplexity * 100 * 1024 };
		Threads::MockCallCoroutine ( dSafeStack, [&tParams] {	// do in coro as for fat expr it might already require dedicated stack
			tParams.m_pExprBase = sphExprParse ( tParams.m_sExpr, tParams.m_tSchema, tParams.m_sError, tParams.m_tArgs );
		});
		}

		tParams.m_bSuccess = !!tParams.m_pExprBase;
		assert ( tParams.m_pExprBase );

		Threads::MockCallCoroutine ( m_dMockStack, [&tParams] {
			tParams.m_pExprBase->Eval ( tParams.m_tMatch );
		} );

		if ( !tParams.m_bSuccess || !tParams.m_sError.IsEmpty () )
			sphWarning ( "stack check expression error: %s", tParams.m_sError.cstr () );
	}

public:
	static int MockMeasure();
	static void PublishValue ( int iStack );
};

/////////////////////////////////////////////////////////////////////
class FilterCreationMeasureStack_c : public StackMeasurer_c
{
	void BuildMockExpr ( int iComplexity ) final
	{
		m_sQuery.Clear ();
		m_sQuery << "select * from test where id between 1 and 10";
		for ( int i = 0; i<iComplexity; i++ )
			m_sQuery << " OR id between 1 and 10";
	}

	void MockParseTest () final
	{
		struct
		{
			CSphString m_sQuery;
			CSphVector<SqlStmt_t> m_dStmt;
			CSphSchema m_tSchema;
			CSphString m_sError;
			bool m_bSuccess = false;
		} tParams;

		tParams.m_sQuery = m_sQuery.cstr();

		CSphColumnInfo tAttr;
		tAttr.m_eAttrType = SPH_ATTR_BIGINT;
		tAttr.m_sName = sphGetDocidName ();
		tParams.m_tSchema.AddAttr ( tAttr, false );

		Threads::MockCallCoroutine ( m_dMockStack, [&tParams] {
			tParams.m_bSuccess = sphParseSqlQuery ( tParams.m_sQuery.cstr (), tParams.m_sQuery.Length ()
													, tParams.m_dStmt, tParams.m_sError, SPH_COLLATION_DEFAULT );
			if ( !tParams.m_bSuccess )
				return;

			const CSphQuery & tQuery = tParams.m_dStmt[0].m_tQuery;
			CreateFilterContext_t tFCtx;
			tFCtx.m_pFilters = &tQuery.m_dFilters;
			tFCtx.m_pFilterTree = &tQuery.m_dFilterTree;
			tFCtx.m_pSchema = &tParams.m_tSchema;
			tFCtx.m_bScan = true;

			CSphString sWarning;

			CSphQueryContext tCtx ( tQuery );
			tParams.m_bSuccess = tCtx.CreateFilters ( tFCtx, tParams.m_sError, sWarning );
		} );

		if ( !tParams.m_bSuccess || !tParams.m_sError.IsEmpty () )
			sphWarning ( "stack check filter error: %s", tParams.m_sError.cstr () );
	}

protected:
	StringBuilder_c m_sQuery;

public:
	static int MockMeasure();
	static void PublishValue ( int iStack );
};

#if defined( __clang__ ) || defined( __GNUC__ )
#define ATTRIBUTE_NO_SANITIZE_ADDRESS __attribute__ ( ( no_sanitize_address ) )
#else
#define ATTRIBUTE_NO_SANITIZE_ADDRESS
#endif

ATTRIBUTE_NO_SANITIZE_ADDRESS int CreateExprStackSize_c::MockMeasure()
{
	CreateExprStackSize_c tCreateMeter;
	return tCreateMeter.MockMeasureStack ( 5 );
}

ATTRIBUTE_NO_SANITIZE_ADDRESS int EvalExprStackSize_c::MockMeasure()
{
	EvalExprStackSize_c tEvalMeter;
	return tEvalMeter.MockMeasureStack ( 20 );
}

ATTRIBUTE_NO_SANITIZE_ADDRESS int FilterCreationMeasureStack_c::MockMeasure()
{
	FilterCreationMeasureStack_c tCreateMeter;
	return tCreateMeter.MockMeasureStack ( 100 );
}

void CreateExprStackSize_c::PublishValue ( int iStack )
{
	SetExprNodeStackItemSize ( iStack, 0 );
}

void EvalExprStackSize_c::PublishValue ( int iStack )
{
	SetExprNodeStackItemSize ( 0, iStack );
}

void FilterCreationMeasureStack_c::PublishValue ( int iStack )
{
	SetFilterStackItemSize ( iStack );
}


template<typename MOCK, int COMPILEDVAL>
ATTRIBUTE_NO_SANITIZE_ADDRESS void DetermineStackSize ( const char* szReport, const char* szEnv )
{
	int iSize = COMPILEDVAL;
	int iNewSize = 0;
	bool bMocked = false;
	if ( !COMPILEDVAL || Threads::StackMockingAllowed() )
	{
		StringBuilder_c sName;
		sName << "MANTICORE_" << szEnv;
		iNewSize = val_from_env ( sName.cstr(), 0 );

		if ( !iNewSize )
		{
			iNewSize = MOCK::MockMeasure();
			bMocked = true;

#ifdef NDEBUG
			if ( COMPILEDVAL && COMPILEDVAL < iNewSize )
				sphLogDebug ( "Compiled-in value %s (%d) is less than measured (%d).", szEnv, COMPILEDVAL, iNewSize );
#endif
		}
		iSize = iNewSize;
		if ( bMocked )
			sphLogDebug ( "%s is %d (mocked, as no env MANTICORE_%s=%d found)", szReport, iSize, szEnv, iNewSize );
		else
			sphLogDebug ( "%s %d (from env MANTICORE_%s)", szReport, iSize, szEnv );
	} else
	{
		sphLogDebug ( "%s is %d (compiled-in)", szReport, iSize );
	}

	MOCK::PublishValue ( iSize );
}


void DetermineNodeItemStackSize()
{
	// some values for x86_64: clang 12.0.1 relwithdebinfo = 768, debug = 4208. gcc 9.3 relwithdebinfo = 16, debug = 256
#ifdef KNOWN_CREATE_SIZE
	DetermineStackSize<CreateExprStackSize_c, KNOWN_CREATE_SIZE>
#else
	DetermineStackSize<CreateExprStackSize_c, 0>
#endif
			( "expression stack for creation", "KNOWN_CREATE_SIZE" );

	// some values for x86_64: clang 12.0.1 relwithdebinfo = 32, debug = 48. gcc 9.3 relwithdebinfo = 48, debug = 48
#ifdef KNOWN_EXPR_SIZE
	DetermineStackSize<EvalExprStackSize_c, KNOWN_EXPR_SIZE>
#else
	DetermineStackSize<EvalExprStackSize_c, 0>
#endif
			( "expression stack for eval/deletion", "KNOWN_EXPR_SIZE" );
}

void DetermineFilterItemStackSize ()
{
	// some values for x86_64: clang 12.0.1 relwithdebinfo = 208, debug = 400. gcc 9.3 relwithdebinfo = 240, debug = 272
#ifdef KNOWN_FILTER_SIZE
	DetermineStackSize<FilterCreationMeasureStack_c, KNOWN_FILTER_SIZE>
#else
	DetermineStackSize<FilterCreationMeasureStack_c, 0>
#endif
			( "filter stack delta", "KNOWN_FILTER_SIZE" );
}
