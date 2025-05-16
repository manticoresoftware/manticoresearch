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

// excess and-not
// ((A !N1) !N2) -> (A !(N1 | N2))

// excess brackets
// ((A | B) | C) -> ( A | B | C )
// ((A B) C) -> ( A B C )

namespace {

struct XQNodeHash_fn
{
	static uint64_t Hash ( XQNode_t * pNode ) { return (uint64_t) pNode; }
};

/// return either index of pNode among Parent.m_dChildren, or -1
int GetNodeChildIndex ( const XQNode_t * pParent, const XQNode_t * pNode )
{
	assert ( pParent && pNode );
	ARRAY_FOREACH ( i, pParent->dChildren() )
		if ( pParent->dChild ( i ) == pNode )
			return i;

	return -1;
}

XQNode_t * sphMoveSiblingsUp ( const XQNode_t * pNode )
{
	XQNode_t * pParent = pNode->m_pParent;
	assert ( pParent );
	XQNode_t * pGrand = pParent->m_pParent;
	assert ( pGrand );

	assert ( pGrand->dChildren().Contains ( pParent ) );
	const int iParent = GetNodeChildIndex ( pGrand, pParent );

	const int iParentChildren = pParent->dChildren().GetLength();
	const int iGrandChildren = pGrand->dChildren().GetLength();
	const int iTotalChildren = iParentChildren+iGrandChildren-1;

	// parent.children + grand.parent.children - parent itself
	CSphVector<XQNode_t *> dChildren ( iTotalChildren );

	// grand head prior parent
	for ( int i=0; i<iParent; ++i )
		dChildren[i] = pGrand->dChildren()[i];

	// grand tail after parent
	for ( int i=0; i<iGrandChildren-iParent-1; ++i )
		dChildren[i+iParent+iParentChildren] = pGrand->dChildren()[i+iParent+1];

	// all parent children
	for ( int i=0; i<iParentChildren; ++i )
	{
		XQNode_t * pChild = pParent->dChildren()[i];
		pChild->m_pParent = pGrand;
		dChildren[i+iParent] = pChild;
	}

	pGrand->WithChildren ( [&dChildren](auto& dMyChildren) { dMyChildren.SwapData ( dChildren );} );
	// all children at grand now
	pParent->WithChildren ( [] ( auto & dMyChildren ) { dMyChildren.Resize ( 0 ); } );
	delete (pParent);
	return nullptr;
}

} // namespace



//  NOT:
//	                      AND NOT
//	                       /   |
//	                     AND  NOT
//	                     |
//	                AND NOT
//	              /      |
//	         AND(pNode) NOT
//            |          |
//           ..         ...
//
bool CSphTransformation::CheckExcessAndNot ( const XQNode_t * pNode ) noexcept
{
	return Grand2Node::Valid ( pNode )
			&& pNode->GetOp() == SPH_QUERY_AND
			&& !(pNode->dChildren().GetLength() == 1 && pNode->dChild ( 0 )->GetOp() == SPH_QUERY_ANDNOT)
			&& ParentNode::From ( pNode)->GetOp()==SPH_QUERY_ANDNOT
		&& GrandNode::From(pNode)->GetOp()==SPH_QUERY_AND
		&& Grand2Node::From(pNode)->GetOp()==SPH_QUERY_ANDNOT
	// FIXME!!! check performance with OR node at 2nd grand instead of regular not NOT
		&& Grand2Node::From(pNode)->dChildren().GetLength()>1
		&& Grand2Node::From(pNode)->dChild(1)->GetOp()==SPH_QUERY_NOT;
}


bool CSphTransformation::TransformExcessAndNot () const
{
	bool bRecollect = false;
	CSphOrderedHash<int, XQNode_t *, XQNodeHash_fn, 64> hDeleted;
	for ( auto& [_, hSimGroup] : m_hSimilar )
		for ( auto& [_, dNodes] : hSimGroup )
			for ( XQNode_t* pAnd : dNodes ) // Nodes with the same iFuzzyHash
			{
				XQNode_t * pParentAndNot = pAnd->m_pParent;

				// node environment might be changed due prior nodes transformations
				if ( hDeleted.Exists ( pParentAndNot ) || !CheckExcessAndNot ( pAnd ) )
					continue;

				assert ( pParentAndNot->dChildren().GetLength()==2 );
				XQNode_t * pNot = pParentAndNot->dChildren()[1];
				XQNode_t * pGrandAnd = pParentAndNot->m_pParent;
				const XQNode_t * pGrand2AndNot = pGrandAnd->m_pParent;
				assert ( pGrand2AndNot->dChildren().GetLength()==2 );
				XQNode_t * pGrand2Not = pGrand2AndNot->dChildren()[1];

				assert ( pGrand2Not->dChildren().GetLength()==1 );
				auto * pNewOr = new XQNode_t ( XQLimitSpec_t() );

				pNot->WithChildren([pNewOr](auto& dChildren) {
					pNewOr->SetOp ( SPH_QUERY_OR, dChildren );
					dChildren.Resize(0);
				});

				pNewOr->AddNewChild ( pGrand2Not->dChildren()[0] );
				pGrand2Not->dChildren()[0] = pNewOr;
				pNewOr->m_pParent = pGrand2Not;

				assert ( pGrandAnd->dChildren().Contains ( pParentAndNot ) );
				if ( const int iChild = GetNodeChildIndex ( pGrandAnd, pParentAndNot ); iChild>=0 )
					pGrandAnd->dChildren()[iChild] = pAnd;
				pAnd->m_pParent = pGrandAnd;

				// Delete excess nodes
				hDeleted.Add ( 1, pParentAndNot );
				pParentAndNot->dChildren()[0] = nullptr;
				SafeDelete ( pParentAndNot );
				bRecollect = true;
			}

	return bRecollect;
}

//  NOT:
//	         OR|AND (grand)
//            /     |
//	   OR|AND (parent) ...
//		  |
//	    pNode
bool CSphTransformation::CheckExcessBrackets ( const XQNode_t * pNode ) noexcept
{
	return GrandNode::Valid(pNode)
		&& ( ( ParentNode::From(pNode)->GetOp()==SPH_QUERY_AND
				&& ParentNode::From(pNode)->dWords().IsEmpty()
				&& GrandNode::From(pNode)->GetOp()==SPH_QUERY_AND )
			|| ( ParentNode::From(pNode)->GetOp()==SPH_QUERY_OR
				&& GrandNode::From(pNode)->GetOp()==SPH_QUERY_OR ) );
}

bool CSphTransformation::TransformExcessBrackets () const noexcept
{
	bool bRecollect = false;
	CSphOrderedHash<int, XQNode_t *, XQNodeHash_fn, 64> hDeleted;
	for ( auto& [_, hSimGroup] : m_hSimilar )
		for ( auto& [_, dNodes] : hSimGroup )
			for ( XQNode_t* pNode : dNodes ) // Nodes with the same iFuzzyHash
			{
				// node environment might be changed due prior nodes transformations
				if ( hDeleted.Exists ( pNode ) || !CheckExcessBrackets ( pNode ) )
					continue;

				XQNode_t * pDel = sphMoveSiblingsUp ( pNode );
				hDeleted.Add ( 1, pDel );
				bRecollect = true;
			}

	return bRecollect;
}
