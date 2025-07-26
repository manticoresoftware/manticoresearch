//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "transformation.h"

// "hung" operand ( AND(OR) node with only 1 child ) appears after an internal transformation

//  NOT:
//	OR|AND (parent)
//		  |
//	    pNode\?
//
bool CSphTransformation::CheckHungOperand ( const XQNode_t * pNode ) noexcept
{
	return ParentNode::Valid(pNode)
		&& ( ParentNode::From(pNode)->GetOp()==SPH_QUERY_OR || ParentNode::From(pNode)->GetOp()==SPH_QUERY_AND )
		&& ParentNode::From(pNode)->dChildren().GetLength()<=1
//		&& pNode->dWords().IsEmpty()
		&& !( GrandNode::Valid(pNode)
			&& ParentNode::From(pNode)->GetOp()==SPH_QUERY_AND
			&& GrandNode::From(pNode)->GetOp()==SPH_QUERY_ANDNOT );
}


bool CSphTransformation::TransformHungOperand () const noexcept
{
	if ( !m_hSimilar.GetLength()
		|| !m_hSimilar.Exists ( CONST_GROUP_FACTOR )
		|| !m_hSimilar[CONST_GROUP_FACTOR].tHash.Exists ( CONST_GROUP_FACTOR ) )
		return false;

	const CSphVector<XQNode_t *> & dSimilarNodes = m_hSimilar[CONST_GROUP_FACTOR].tHash[CONST_GROUP_FACTOR];
	for ( XQNode_t * pHungNode: dSimilarNodes )
	{
		XQNode_t * pParent = pHungNode->m_pParent;
		if ( XQNode_t * pGrand = pParent->m_pParent; !pGrand )
		{
			*m_ppRoot = pHungNode;
			pHungNode->m_pParent = nullptr;
		} else
		{
			assert ( pGrand->dChildren().Contains ( pParent ) );
			for ( XQNode_t *& pChild : pGrand->dChildren() )
			{
				if ( pChild!=pParent )
					continue;

				pChild = pHungNode;
				pHungNode->m_pParent = pGrand;
				pHungNode->Rehash();
				break;
			}
		}

		pParent->dChildren()[0] = nullptr;
		SafeDelete ( pParent );
	}

	return true;
}
