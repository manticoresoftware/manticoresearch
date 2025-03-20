//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "client_task_info.h"
#include "coroutine.h"
#include "std/sys.h"

struct StackSizeTuplet_t
{
	int m_iCreate;
	int m_iEval;

	StackSizeTuplet_t() = default;
	StackSizeTuplet_t (int iCreate, int iEval)
		: m_iCreate { iCreate }
		, m_iEval { iEval }
	{}
};

template<typename T>
int EvalMaxTreeHeight ( const VecTraits_T<T>& dTree, int iStartNode )
{
	CSphVector<std::pair<int, int>> dNodes;
	dNodes.Reserve ( dTree.GetLength() / 2 );
	int iMaxHeight = 1;
	dNodes.Add ( { iStartNode, 1 } );
	while ( dNodes.GetLength() )
	{
		auto tParent = dNodes.Pop();
		const auto& tItem = dTree[tParent.first];
		iMaxHeight = Max ( iMaxHeight, tParent.second );
		if ( tItem.m_iLeft >= 0 )
			dNodes.Add ( { tItem.m_iLeft, tParent.second + 1 } );
		if ( tItem.m_iRight >= 0 )
			dNodes.Add ( { tItem.m_iRight, tParent.second + 1 } );
	}
	return iMaxHeight;
}

struct StackSizeParams_t
{
	int iMaxDepth;
	StackSizeTuplet_t tNodeStackSize;
	const char* szName;
};


inline std::pair<int, CSphString> EvalStackForExpr ( const StackSizeParams_t & tParams ) noexcept
{
	const auto iCREATE = tParams.tNodeStackSize.m_iCreate;
	const auto iEVAL = tParams.tNodeStackSize.m_iEval;

	int64_t iCalculatedStack = iCREATE+(int64_t) tParams.iMaxDepth*iEVAL;
	int64_t iCurStackSize = Threads::MyStackSize ();

	//	sphWarning ( "used %d, height %d, node %d, start %d, calculated %d, current %d", Threads::GetStackUsed (), iMaxHeight, iEVAL, iCREATE, iCalculatedStack, iCurStackSize );
	if ( ( Threads::GetStackUsed ()+iCalculatedStack+( iCREATE >> 1 ) )<=iCurStackSize )
		return { -1, {} };

	auto iStackNeeded = (int) iCalculatedStack+32*1024;
	iStackNeeded = sphRoundUp ( iStackNeeded, GetMemPageSize () ); // round up to memory page.
	if ( iStackNeeded>session::GetMaxStackSize () )
		return { -1, SphSprintf ( "query %s too complex, not enough stack (thread_stack=%dK or higher required)",
								  tParams.szName, (int) ( ( iStackNeeded+1024-( iStackNeeded%1024 ) )/1024 ) ) };
	return { iStackNeeded, {} };
}

void DetermineNodeItemStackSize ( StringBuilder_c& sExport );
void DetermineFilterItemStackSize ( StringBuilder_c& sExport );
void DetermineMatchStackSize ( StringBuilder_c& sExport );
