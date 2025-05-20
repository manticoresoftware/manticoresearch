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

// common and-not factor
// ((A !X) | (A !Y) | (A !Z)) -> (A !(X Y Z))


//  NOT:
//		 _______ OR (gGOr) ________________
// 		/          |                       |
// 	 ...        AND NOT (grandAndNot)     ...
//                /       |
//               AND     NOT
//                |       |
//              pNode  relatedNode
//
bool CSphTransformation::CheckCommonAndNotFactor ( const XQNode_t * pNode ) noexcept
{
	return Grand2Node::Valid ( pNode )
			&& ParentNode::From ( pNode )->GetOp() == SPH_QUERY_AND
			&& GrandNode::From ( pNode )->GetOp() == SPH_QUERY_ANDNOT
			&& Grand2Node::From ( pNode )->GetOp() == SPH_QUERY_OR
			// FIXME!!! check performance with OR node at 2nd grand instead of regular not NOT
			&& GrandNode::From(pNode)->dChildren().GetLength()>=2
			&& GrandNode::From(pNode)->dChild(1)->GetOp()==SPH_QUERY_NOT;
}


bool CSphTransformation::TransformCommonAndNotFactor () const noexcept
{
	int iActiveDeep = 0;
	for ( auto& [_, hSimGroup] : m_hSimilar )
		for ( auto& [_, dSimilarNodes] : hSimGroup.tHash )
		{
			if ( iActiveDeep && iActiveDeep < hSimGroup.iDeep )
				continue;

			// Nodes with the same iFuzzyHash
			if ( dSimilarNodes.GetLength()<2 )
				continue;

			MakeTransformCommonAndNotFactor ( dSimilarNodes );
			iActiveDeep = hSimGroup.iDeep;
		}
	return iActiveDeep;
}

// Pick the weakest node from the equal
// PROXIMITY and PHRASE nodes with same keywords have an equal magic hash
// so they are considered as equal nodes.
void CSphTransformation::MakeTransformCommonAndNotFactor ( const CSphVector<XQNode_t *> & dSimilarNodes )
{
	const int iWeakestIndex = GetWeakestIndex ( dSimilarNodes );

	const XQNode_t * pFirstAndNot = GrandNode::From ( dSimilarNodes [iWeakestIndex] );
	XQNode_t * pCommonOr = pFirstAndNot->m_pParent;

	assert ( pFirstAndNot->dChildren().GetLength()==2 );
	XQNode_t * pFirstNot = pFirstAndNot->dChildren()[1];
	assert ( pFirstNot->dChildren().GetLength()==1 );

	CSphVector<XQNode_t *> dAndNewChildren;
	dAndNewChildren.Reserve ( dSimilarNodes.GetLength() );
	dAndNewChildren.Add (pFirstNot->dChildren()[0]);
	auto * pAndNew = new XQNode_t ( XQLimitSpec_t() );
	pFirstNot->dChildren()[0] = pAndNew;
	pAndNew->m_pParent = pFirstNot;

	ARRAY_CONSTFOREACH ( i, dSimilarNodes )
	{
		assert ( CheckCommonAndNotFactor ( dSimilarNodes[i] ) );
		if ( i==iWeakestIndex )
			continue;

		XQNode_t * pAndNot = GrandNode::From ( dSimilarNodes[i] );
		assert ( pAndNot->dChildren().GetLength()==2 );
		const XQNode_t * pNot = pAndNot->dChildren()[1];
		assert ( pNot->dChildren().GetLength()==1 );
		assert ( &pAndNew->dChildren()!=&pNot->dChildren() );
		dAndNewChildren.Add ( pNot->dChildren()[0] );
		pNot->dChildren()[0] = nullptr;

		Verify ( pCommonOr->RemoveChild ( pAndNot ) );
		dSimilarNodes[i] = nullptr;
		SafeDelete ( pAndNot );
	}
	pAndNew->SetOp ( SPH_QUERY_AND, dAndNewChildren );
}
