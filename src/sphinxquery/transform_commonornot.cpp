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

// common or-not
// ((A !(N | N1)) | (B !(N | N2))) -> (( (A !N1) | (B !N2) ) !N)

//  NOT:
//		 __ OR (Grand3 = CommonOr) __
//		/    |                       |
//	 ...  AND NOT (Grand2)          ...
//          /        |
//     relatedNode  NOT (grandNot)
//        	         |
//        	      OR (parentOr)
//                 /    |
//               pNode  ...
bool CSphTransformation::CheckCommonOrNot ( const XQNode_t * pNode ) noexcept
{
	return Grand3Node::Valid ( pNode )
			&& ParentNode::From ( pNode )->GetOp() == SPH_QUERY_OR
			&& GrandNode::From ( pNode )->GetOp() == SPH_QUERY_NOT
			&& Grand2Node::From ( pNode )->GetOp() == SPH_QUERY_ANDNOT
			&& Grand3Node::From ( pNode )->GetOp() == SPH_QUERY_OR;
}


bool CSphTransformation::TransformCommonOrNot () noexcept
{
	int iActiveDeep = 0;
	for ( auto& [_, hSimGroup] : m_hSimilar )
		for ( auto& [_, dSimilarNodes] : hSimGroup.tHash )
		{
			if ( iActiveDeep && iActiveDeep < hSimGroup.iDeep )
				continue;

			// Nodes with the same iFuzzyHash
			if ( dSimilarNodes.GetLength()<2
				|| HasSameParent ( dSimilarNodes ) )
				continue;

			if ( CollectRelatedNodes < GrandNode, Grand2Node> ( dSimilarNodes ) )
			{
				MakeTransformCommonOrNot ( dSimilarNodes );
				iActiveDeep = hSimGroup.iDeep;
				// Don't make transformation for other nodes from the same OR-node,
				// because query tree was changed and further transformations
				// might be invalid.
				break;
			}
		}

	return iActiveDeep;
}

// Pick the weakest node from the equal
// PROXIMITY and PHRASE nodes with same keywords have an equal magic hash
// so they are considered as equal nodes.
void CSphTransformation::MakeTransformCommonOrNot ( const CSphVector<XQNode_t *> & dSimilarNodes ) const
{
	int iWeakestIndex = GetWeakestIndex ( dSimilarNodes );
	assert ( iWeakestIndex!=-1 );
	XQNode_t * pWeakestSimilar = dSimilarNodes [ iWeakestIndex ];

	// Common OR node (that is Grand3Node::From)
	XQNode_t * pCommonOr = Grand3Node::From (pWeakestSimilar);

	// Delete/unlink similar nodes ( except weakest )
	ARRAY_FOREACH ( i, dSimilarNodes )
	{
		XQNode_t * pParent = dSimilarNodes[i]->m_pParent;
		Verify ( pParent->RemoveChild ( dSimilarNodes[i] ) );

		if ( i!=iWeakestIndex )
			SafeDelete ( dSimilarNodes[i] );
	}

	auto * pNewAndNot = new XQNode_t ( XQLimitSpec_t() );
	auto * pNewAnd = new XQNode_t ( XQLimitSpec_t() );
	auto * pNewNot = new XQNode_t ( XQLimitSpec_t() );
	if ( !pCommonOr->m_pParent )
	{
		*m_ppRoot = pNewAndNot;
	} else
	{
		XQNode_t * pParent = pCommonOr->m_pParent;
		assert ( pParent->dChildren().Contains ( pCommonOr ) );
		for ( XQNode_t *& pChild : pParent->dChildren() )
		{
			if ( pChild!=pCommonOr )
				continue;

			pChild = pNewAndNot;
			pNewAndNot->m_pParent = pParent;
			pChild->Rehash();
			break;
		}
	}

	pNewNot->SetOp ( SPH_QUERY_NOT, pWeakestSimilar );
	pNewAnd->SetOp ( SPH_QUERY_AND, pCommonOr );
	pNewAndNot->SetOp ( SPH_QUERY_ANDNOT, pNewAnd, pNewNot );
}

