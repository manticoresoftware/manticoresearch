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

// common compound not
// ((A !(N AA)) | (B !(N BB))) -> (((A|B) !N) | (A !AA) | (B !BB)) [ if cost(N) > cost(A) + cost(B) ]


//  NOT:
//		 __ OR (Grand3 = CommonOr) __
//		/    |                       |
//	 ...  AND NOT (Grand2)          ...
//          /        |
//     relatedNode  NOT (grandNot)
//        	         |
//        	      AND (parentAnd)
//                 /    |
//               pNode  ...
//
bool CSphTransformation::CheckCommonCompoundNot ( const XQNode_t * pNode ) noexcept
{
	return Grand3Node::Valid ( pNode )
			&& ParentNode::From ( pNode )->GetOp() == SPH_QUERY_AND
			&& GrandNode::From ( pNode )->GetOp() == SPH_QUERY_NOT
			&& Grand2Node::From ( pNode )->GetOp() == SPH_QUERY_ANDNOT
			&& Grand3Node::From ( pNode )->GetOp() == SPH_QUERY_OR;
}


bool CSphTransformation::TransformCommonCompoundNot () noexcept
{
	int iActiveDeep = 0;
	for ( auto& [_, hSimGroup] : m_hSimilar )
		for ( auto& [_, dSimilarNodes] : hSimGroup.tHash )
		{
			if ( iActiveDeep && iActiveDeep < hSimGroup.iDeep )
				continue;

			// Nodes with the same iFuzzyHash
			if ( dSimilarNodes.GetLength()<2
				|| !CollectRelatedNodes<GrandNode, Grand2Node> ( dSimilarNodes ) )
				continue;

			// if related similar are from the same tree, related will be the same.
			// by 'uniq' we will reduce dupes. Notice, here stable uniq; we don't want change the sequence
			// depend on pointers values
			m_dRelatedNodes.Uniq(sph::stable);
			// Load cost of the first node from the group
			// of the common nodes. The cost of nodes from
			// TransformableNodes are the same.
			SetCosts ( dSimilarNodes[0], m_dRelatedNodes );
			const int iCommon = dSimilarNodes[0]->m_iUser;
			const int iRelated = m_dRelatedNodes.sum_of<int> ([]( auto & tNode ) { return tNode->m_iUser; } );

			// Check that optimization will be useful.
			if ( iCommon<=iRelated )
				continue;

			MakeTransformCommonCompoundNot ( dSimilarNodes );
			iActiveDeep = hSimGroup.iDeep;
			// Don't make transformation for other nodes from the same OR-node,
			// because qtree was changed and further transformations
			// might be invalid.
			break;
		}

	return iActiveDeep;
}


void CSphTransformation::MakeTransformCommonCompoundNot ( const CSphVector<XQNode_t *> & dSimilarNodes ) const
{
	// Pick the weakest node from the equal
	// PROXIMITY and PHRASE nodes with same keywords have an equal magic hash
	// so they are considered as equal nodes.
	const int iWeakestIndex = GetWeakestIndex ( dSimilarNodes );
	assert ( iWeakestIndex!=-1 );
	XQNode_t * pWeakestSimilar = dSimilarNodes [ iWeakestIndex ];

	// Common OR node (that is Grand3Node::From)
	XQNode_t * pCommonOr = Grand3Node::From ( pWeakestSimilar );

	// Factor out and delete/unlink similar nodes ( except weakest )
	ARRAY_FOREACH ( i, dSimilarNodes )
	{
		XQNode_t * pParent = dSimilarNodes[i]->m_pParent;
		Verify ( pParent->RemoveChild ( dSimilarNodes[i] ) );
		if ( i!=iWeakestIndex )
			SafeDelete ( dSimilarNodes[i] );
	}

	// Create yet another ANDNOT node
	// with related nodes and one common node
	auto * pNewNot = new XQNode_t ( XQLimitSpec_t() );
	pNewNot->SetOp ( SPH_QUERY_NOT, pWeakestSimilar );

	CSphVector<XQNode_t *> dNewOrChildren {m_dRelatedNodes.GetLength()};
	ARRAY_FOREACH ( i, m_dRelatedNodes )
	{
		// ANDNOT operation implies AND and NOT nodes.
		// The related nodes point to AND node
		if ( m_dRelatedNodes[i]->dChildren().GetLength()==1 )
			dNewOrChildren[i] = m_dRelatedNodes[i]->dChild (0)->Clone();
		else
			dNewOrChildren[i] = m_dRelatedNodes[i]->Clone();
	}

	XQNode_t * pSingleRelatedOrNewOr;
	if ( dNewOrChildren.GetLength()>1 )
	{
		pSingleRelatedOrNewOr = new XQNode_t ( XQLimitSpec_t() );
		pSingleRelatedOrNewOr->SetOp ( SPH_QUERY_OR, dNewOrChildren );
	} else
		pSingleRelatedOrNewOr = dNewOrChildren[0];

	auto * pNewAnd = new XQNode_t ( XQLimitSpec_t() );
	pNewAnd->SetOp ( SPH_QUERY_AND, pSingleRelatedOrNewOr );
	auto * pNewAndNot = new XQNode_t ( XQLimitSpec_t() );
	pNewAndNot->SetOp ( SPH_QUERY_ANDNOT, pNewAnd, pNewNot );
	pCommonOr->AddNewChild ( pNewAndNot );
}
