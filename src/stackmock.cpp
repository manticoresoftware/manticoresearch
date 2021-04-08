//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
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
#include "coroutine.h"
#include "searchdsql.h"
#include "attribute.h"

static int CalcUsedStackEdge ( VecTraits_T<BYTE> dStack, BYTE uFiller )
{
	ARRAY_CONSTFOREACH ( i, dStack )
		if ( dStack[i]!=uFiller )
			return dStack.GetLength ()-i;
	
	return dStack.GetLength();
}


static int CalcUsedStackBytes ( VecTraits_T<BYTE> dStack, BYTE uFiller )
{
	int iRes=0;
	dStack.Apply ( [&] ( BYTE uData ) { iRes += ( uData!=uFiller ) ? 1 : 0; } );
	return iRes;
}


static void MockInitMem ( VecTraits_T<BYTE> dStack, BYTE uFiller )
{
	::memset ( dStack.begin (), uFiller, dStack.GetLengthBytes () );
}


static void MockParseExpr ( VecTraits_T<BYTE> dStack, const char * sExpr )
{
	struct
	{
		ExprParseArgs_t m_tArgs;
		CSphString m_sError;
		CSphSchema m_tSchema;
		const char * m_sExpr;
		bool m_bSuccess;
	} tParams;

	CSphColumnInfo tAttr;
	tAttr.m_eAttrType = SPH_ATTR_INTEGER;
	tAttr.m_sName = "attr_a";
	tParams.m_tSchema.AddAttr ( tAttr, false );
	tAttr.m_sName = "attr_b";
	tParams.m_tSchema.AddAttr ( tAttr, false );
	tParams.m_sExpr = sExpr;

	Threads::MockCallCoroutine ( dStack, [&tParams]
		{
			ISphExpr * pExprBase = sphExprParse ( tParams.m_sExpr, tParams.m_tSchema, tParams.m_sError, tParams.m_tArgs );
			tParams.m_bSuccess = !!pExprBase;
			SafeRelease ( pExprBase );
		});

	if ( !tParams.m_bSuccess || !tParams.m_sError.IsEmpty () )
		sphWarning ( "stack check expression error: %s", tParams.m_sError.cstr () );
}


template <typename ACTION>
static int MockMeasureStack ( VecTraits_T<BYTE> dStack, const char * szParam, BYTE uPattern, ACTION && fnAction )
{
	MockInitMem ( dStack, uPattern );
	fnAction ( dStack, szParam );
	auto iUsedStack = CalcUsedStackBytes ( dStack, uPattern );
	auto iUsedStackEdge = CalcUsedStackEdge ( dStack, uPattern );
	return ( Max ( iUsedStack, iUsedStackEdge )+3 ) & ~3;
}

template <typename ACTION>
static int MockStack ( VecTraits_T<BYTE> dStack, const char * szParam, ACTION && fnAction )
{
	auto iStartStack55 = MockMeasureStack ( dStack, szParam, 0xDE, fnAction );
	auto iStartStackAA = MockMeasureStack ( dStack, szParam, 0xAD, fnAction );
	return Max ( iStartStack55, iStartStackAA );
}

template <typename GETMOCK, typename ACTION >
int DetermineStackSize ( GETMOCK && fnGetMock, ACTION && fnAction, int iNodes )
{
	CSphFixedVector<BYTE> dMockStack { (int)Threads::GetDefaultCoroStackSize() };
	StringBuilder_c sExpr;
	fnGetMock ( sExpr, 0 );

	int iStartStack = MockStack ( (VecTraits_T<BYTE>)dMockStack, sExpr.cstr(), fnAction );
	int iDelta = 0;

	// Find edge of stack where expr length became visible
	// (we need quite big expr in order to touch deepest of the stack)
	int iHeight = 0;
	while ( !iDelta )
	{
		iHeight++;
		fnGetMock ( sExpr, iHeight );
		auto iCurStack = MockStack ( dMockStack, sExpr.cstr(), fnAction );
		iDelta = iCurStack - iStartStack;
	}

	iStartStack += iDelta;
	iDelta = 0;

	// add iNodes frames and average stack from them (1-st already added, so add iNodes-1)
	fnGetMock ( sExpr, iHeight + iNodes - 1 );

	auto iCurStack = MockStack ( dMockStack, sExpr.cstr(), fnAction );
	iDelta = iCurStack-iStartStack;
	iDelta/=iNodes;
	iDelta = (iDelta+15)&~15;

	return iDelta;
}

/////////////////////////////////////////////////////////////////////

static const CSphString g_sMockExpr = "(4*attr_a+2*(attr_b-1)+3)*10";

static void GetMockExpr ( StringBuilder_c & sExpr, int iCount )
{
	sExpr.Clear();
	for (int i=0; i<iCount; ++i)
		sExpr << "(";

	sExpr << g_sMockExpr;

	for (int i=0; i<iCount; ++i)
		sExpr << "*(10+1))";
}


void DetermineNodeItemStackSize()
{
	auto fnGetMock = [](StringBuilder_c & sExpr, int iHeight){ GetMockExpr ( sExpr, iHeight ); };
	auto fnAction = [](VecTraits_T<BYTE> dStack, const char * szExpr) { MockParseExpr ( dStack, szExpr ); };
	int iDelta = DetermineStackSize ( fnGetMock, fnAction, 5 );
	sphLogDebug ( "expression stack delta %d", iDelta );
	SetExprNodeStackItemSize(iDelta);
}

/////////////////////////////////////////////////////////////////////

static void GetMockFilterQuery ( StringBuilder_c & sQuery, int iCount )
{
	sQuery.Clear();
	sQuery << "select * from test where id between 1 and 10";
	for ( int i=0; i < iCount; i++ )
		sQuery << " OR id between 1 and 10";
}


static void MockParseQuery ( VecTraits_T<BYTE> dStack, const char * szQuery )
{
	struct
	{
		CSphString				m_sQuery;
		CSphVector<SqlStmt_t>	m_dStmt;
		CSphSchema				m_tSchema;
		CSphString				m_sError;
		bool					m_bSuccess;
	} tParams;

	tParams.m_sQuery = szQuery;

	CSphColumnInfo tAttr;
	tAttr.m_eAttrType = SPH_ATTR_BIGINT;
	tAttr.m_sName = sphGetDocidName();
	tParams.m_tSchema.AddAttr ( tAttr, false );

	Threads::MockCallCoroutine ( dStack, [&tParams]
		{
			tParams.m_bSuccess = sphParseSqlQuery ( tParams.m_sQuery.cstr(), tParams.m_sQuery.Length(), tParams.m_dStmt, tParams.m_sError, SPH_COLLATION_DEFAULT );
			if ( !tParams.m_bSuccess )
				return;

			const CSphQuery & tQuery = tParams.m_dStmt[0].m_tQuery;
			CreateFilterContext_t tFCtx;
			tFCtx.m_pFilters	= &tQuery.m_dFilters;
			tFCtx.m_pFilterTree	= &tQuery.m_dFilterTree;
			tFCtx.m_pSchema		= &tParams.m_tSchema;
			tFCtx.m_bScan		= true;

			CSphString sWarning;

			CSphQueryContext tCtx(tQuery);
			tParams.m_bSuccess = tCtx.CreateFilters ( tFCtx, tParams.m_sError, sWarning );
		});

	if ( !tParams.m_bSuccess || !tParams.m_sError.IsEmpty () )
		sphWarning ( "stack check filter error: %s", tParams.m_sError.cstr () );
}


void DetermineFilterItemStackSize()
{
	auto fnGetMock = [](StringBuilder_c & sQuery, int iHeight){ GetMockFilterQuery ( sQuery, iHeight ); };
	auto fnAction = [](VecTraits_T<BYTE> dStack, const char * szQuery) { MockParseQuery ( dStack, szQuery ); };
	int iDelta = DetermineStackSize ( fnGetMock, fnAction, 100 );
	sphLogDebug ( "filter stack delta %d", iDelta );
	SetFilterStackItemSize(iDelta);
}
