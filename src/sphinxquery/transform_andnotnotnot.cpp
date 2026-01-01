//
// Copyright (c) 2025-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "transformation.h"

// "andnotnot" foo AND NOT (NOT bar) -> foo bar ( appears after an internal transformation )

//  AND_NOT
//    /  \
//  AND   NOT
//          \
//          NOT
//           |
//          pNode
bool CSphTransformation::CheckAndNotNotOperand ( const XQNode_t * pNode ) noexcept
{
	return Grand2Node::Valid ( pNode )
			&& ParentNode::From ( pNode )->GetOp() == SPH_QUERY_NOT
			&& GrandNode::From ( pNode )->GetOp() == SPH_QUERY_NOT
			&& Grand2Node::From ( pNode )->GetOp() == SPH_QUERY_ANDNOT;
}

bool CSphTransformation::TransformAndNotNotOperand () const noexcept
{
	if ( !m_hSimilar.GetLength()
		|| !m_hSimilar.Exists ( CONST_GROUP_FACTOR )
		|| !m_hSimilar[CONST_GROUP_FACTOR].tHash.Exists ( CONST_GROUP_FACTOR ) )
		return false;

	const CSphVector<XQNode_t *> & dSimilarNodes = m_hSimilar[CONST_GROUP_FACTOR].tHash[CONST_GROUP_FACTOR];
	for ( XQNode_t * pChildOfNotNode: dSimilarNodes )
	{
		XQNode_t * pParent = pChildOfNotNode->m_pParent;
		XQNode_t * pGrand = pParent->m_pParent;
		ReplaceNode ( pGrand, pChildOfNotNode );
		XQNode_t *pGrand2 = pGrand->m_pParent;
		pGrand2->SetOp ( SPH_QUERY_AND );
		pParent->dChildren()[0] = nullptr;
		SafeDelete ( pGrand );
	}
	return true;
}
