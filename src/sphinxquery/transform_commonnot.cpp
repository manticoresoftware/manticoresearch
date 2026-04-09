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

// common not
// ((A !N) | (B !N)) -> ((A|B) !N)

// minimum words per phrase that might be optimized by CommonSuffix optimization
//  NOT:
//		 _______ OR (gGOr) ___________
// 		/          |                   |
// 	 ...        AND NOT (grandAndNot)  ...
//                 /       |
//         relatedNode    NOT (parentNot)
//         	               |
//         	             pNode
//
bool CSphTransformation::CheckCommonNot ( const XQNode_t * pNode ) noexcept
{
	return Grand2Node::Valid ( pNode )
			&& ParentNode::From ( pNode )->GetOp() == SPH_QUERY_NOT
			&& GrandNode::From ( pNode )->GetOp() == SPH_QUERY_ANDNOT
			&& Grand2Node::From ( pNode )->GetOp() == SPH_QUERY_OR;
}


bool CSphTransformation::TransformCommonNot () noexcept
{
	int iActiveDeep = 0;
	for ( auto & [_, hSimGroup] : m_hSimilar )
		for ( auto& [_, dSimilarNodes] : hSimGroup.tHash )
		{
			if ( iActiveDeep && iActiveDeep<hSimGroup.iDeep )
				continue;
			// Nodes with the same iFuzzyHash
			if ( dSimilarNodes.GetLength() < 2 )
				continue;

			assert (!HasSameParent ( dSimilarNodes ));

			if ( !CollectRelatedNodes<ParentNode, GrandNode> ( dSimilarNodes ) )
				continue;

			MakeTransformCommonNot ( dSimilarNodes );
			iActiveDeep = hSimGroup.iDeep;
			// Don't make transformation for other nodes from the same OR-node,
			// because query tree was changed and further transformations
			// might be invalid.
			break;
		}

	return iActiveDeep;
}

// Returns index of weakest node from the equal.
// The example of equal nodes:
// "aaa bbb" (PHRASE), "aaa bbb"~10 (PROXIMITY), "aaa bbb"~20 (PROXIMITY)
// Such nodes have the same magic hash value.
// The weakest is "aaa bbb"~20
int CSphTransformation::GetWeakestIndex ( const CSphVector<XQNode_t *> & dNodes )
{
	int iWeakestIndex = 0;
	int iProximity = -1;

	ARRAY_CONSTFOREACH ( i, dNodes )
	{
		XQNode_t * pNode = dNodes[i];
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
void CSphTransformation::MakeTransformCommonNot ( const CSphVector<XQNode_t *> & dSimilarNodes )
{
	const int iWeakestIndex = GetWeakestIndex ( dSimilarNodes );

	// the weakest node is new parent of transformed expression
	XQNode_t * pWeakestAndNot = m_dRelatedNodes[iWeakestIndex]->m_pParent;
	assert ( pWeakestAndNot->dChild(0)==m_dRelatedNodes[iWeakestIndex] );
	XQNode_t * pCommonOr = pWeakestAndNot->m_pParent;
	assert ( pCommonOr->GetOp()==SPH_QUERY_OR && pCommonOr->dChildren().Contains ( pWeakestAndNot ) );
	XQNode_t * pGrandCommonOr = pCommonOr->m_pParent;

	const bool bKeepOr = (pCommonOr->dChildren().GetLength() > 2);

	// reset ownership of related nodes
	ARRAY_FOREACH ( i, m_dRelatedNodes )
	{
		XQNode_t * pAnd = m_dRelatedNodes[i];
		XQNode_t * pAndNot = pAnd->m_pParent;
		assert ( pAndNot->m_pParent==pCommonOr );

		if ( i != iWeakestIndex )
		{
			Verify ( pAndNot->RemoveChild ( pAnd ) );
			if ( bKeepOr )
			{
				pCommonOr->RemoveChild ( pAndNot );
				SafeDelete ( pAndNot );
			}
		}
	}

	// move all related to new OR
	XQNode_t * pHubOr = new XQNode_t ( XQLimitSpec_t() );
	pHubOr->SetOp ( SPH_QUERY_OR, m_dRelatedNodes );

	// insert hub OR via hub AND to new parent ( AND NOT )
	XQNode_t * pHubAnd = new XQNode_t ( XQLimitSpec_t() );
	pHubAnd->SetOp ( SPH_QUERY_AND, pHubOr );
	// replace old AND at new parent ( AND NOT ) 0 already at OR children
	pHubAnd->m_pParent = pWeakestAndNot;
	pWeakestAndNot->dChildren()[0] = pHubAnd;
	pWeakestAndNot->Rehash();

	// in case common OR had only 2 children
	if ( bKeepOr )
		return;

	// replace old OR with AND_NOT at parent
	if ( !pGrandCommonOr )
	{
		pWeakestAndNot->m_pParent = nullptr;
		*m_ppRoot = pWeakestAndNot;
	} else
	{
		pWeakestAndNot->m_pParent = pGrandCommonOr;
		pGrandCommonOr->WithChildren ( [pCommonOr,pWeakestAndNot] ( auto & dChildren ) {
			for ( XQNode_t *& pChild: dChildren )
			{
				if ( pChild == pCommonOr )
				{
					pChild = pWeakestAndNot;
					break;
				}
			}
		} );
	}

	// remove new parent ( AND OR ) from OR children
	Verify ( pCommonOr->RemoveChild ( pWeakestAndNot ) );

	// free OR and all children
	SafeDelete ( pCommonOr );
}
