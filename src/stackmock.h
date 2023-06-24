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

#pragma once

#include "client_task_info.h"
#include "coroutine.h"
#include "std/sys.h"

using StackSizeTuplet_t = std::pair<int,int>; // create, eval

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

template <typename T>
bool EvalStackForTree ( const VecTraits_T<T> & dTree, int iStartNode, StackSizeTuplet_t tNodeStackSize, int iTreeSizeThresh, int & iStackNeeded, const char * szName, CSphString & sError )
{
	enum eStackSizePurpose { CREATE, EVAL };
	iStackNeeded = -1;
	int64_t iCalculatedStack = Threads::GetStackUsed() + (int64_t)dTree.GetLength()*std::get<EVAL>(tNodeStackSize);
	int64_t iCurStackSize = Threads::MyStackSize();
	if ( dTree.GetLength()<=iTreeSizeThresh )
		return true;

	int iMaxHeight = EvalMaxTreeHeight ( dTree, iStartNode );
	iCalculatedStack = Threads::GetStackUsed() + iMaxHeight* std::get<CREATE> ( tNodeStackSize );

	if ( iCalculatedStack<=iCurStackSize )
		return true;

	if ( iCalculatedStack > Threads::GetMaxCoroStackSize() )
	{
		sError.SetSprintf ( "query %s too complex, not enough stack (thread_stack=%dK or higher required)", szName, (int)( ( iCalculatedStack + 1024 - ( iCalculatedStack%1024 ) ) / 1024 ) );
		return false;
	}

	iStackNeeded = (int)iCalculatedStack + 32*1024;
	iStackNeeded = sphRoundUp( iStackNeeded, GetMemPageSize() ); // round up to memory page.

	// in case we're in real query processing - propagate size of stack need for evaluations (only additional part)
	session::ExpandDesiredStack ( iMaxHeight * std::get<EVAL> ( tNodeStackSize ));

	return true;
}

void DetermineNodeItemStackSize();
void DetermineFilterItemStackSize();
void DetermineMatchStackSize();
