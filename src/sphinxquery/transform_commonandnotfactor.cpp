//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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
// 	 ...        AND NOT (parentAndNot)     ...
//                /       |
//         pNode(AND)     NOT
//                        |
//                     relatedNode
//
bool CSphTransformation::CheckCommonAndNotFactor ( const XQNode_t * pNode ) noexcept
{
	return GrandNode::Valid ( pNode )
			&& pNode->GetOp() == SPH_QUERY_AND
			&& ParentNode::From ( pNode )->GetOp() == SPH_QUERY_ANDNOT
			&& GrandNode::From ( pNode )->GetOp() == SPH_QUERY_OR
			// FIXME!!! check performance with OR node at 2nd grand instead of regular not NOT
			&& ParentNode::From(pNode)->dChildren().GetLength()>=2
			&& ParentNode::From(pNode)->dChild(1)->GetOp()==SPH_QUERY_NOT;
}


bool CSphTransformation::TransformCommonAndNotFactor () noexcept
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

//			StringBuilder_c sHead;
//			sHead << "\nTransformCommonAndNotFactor  ((A !X) | (A !Y) | (A !Z)) -> (A !(X Y Z))";
//			Dump ( *m_ppRoot, sHead.cstr() );
			MakeTransformCommonAndNotFactor ( dSimilarNodes );
			iActiveDeep = hSimGroup.iDeep;
			break;
		}
	return iActiveDeep;
}

// Returns index of the node with weakest child.
// The example of equal nodes:
// "aaa bbb" (PHRASE), "aaa bbb"~10 (PROXIMITY), "aaa bbb"~20 (PROXIMITY)
// Such nodes have the same magic hash value.
// The weakest is "aaa bbb"~20.
int CSphTransformation::GetWeakestChildIndex ( const CSphVector<XQNode_t *> & dNodes )
{
	int iWeakestIndex = 0;
	int iProximity = -1;

	ARRAY_CONSTFOREACH ( i, dNodes )
	{
		const XQNode_t * pNode = dNodes[i];
		if ( pNode->dChildren().IsEmpty() )
			continue;
		pNode = pNode->dChild ( 0 );
		if ( pNode->GetOp() == SPH_QUERY_PROXIMITY && pNode->m_iOpArg > iProximity )
		{
			iProximity = pNode->m_iOpArg;
			iWeakestIndex = i;
		}
	}
	return iWeakestIndex;
}

// Pick the weakest node from the equal
// PROXIMITY and PHRASE nodes with same keywords have an equal magic hash
// so they are considered as equal nodes.
void CSphTransformation::MakeTransformCommonAndNotFactor ( const CSphVector<XQNode_t *> & dSimilarNodes ) noexcept
{
	const int iWeakestIndex = GetWeakestChildIndex ( dSimilarNodes );

	XQNode_t * pFirstAndNot = ParentNode::From ( dSimilarNodes [iWeakestIndex] );
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
	pFirstNot->Rehash();

	ARRAY_CONSTFOREACH ( i, dSimilarNodes )
	{
		assert ( CheckCommonAndNotFactor ( dSimilarNodes[i] ) );
		if ( i==iWeakestIndex )
			continue;

		XQNode_t * pAndNot = ParentNode::From ( dSimilarNodes[i] );
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
	CompositeFixup ( pCommonOr, m_ppRoot );
}
