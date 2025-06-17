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

#include "sphinxquery.h"

//////////////////////////////////////////////////////////////////////////
// COMMON SUBTREES DETECTION
//////////////////////////////////////////////////////////////////////////

struct MarkedNode_t
{
	const XQNode_t*	m_pTree;
	int			m_iCounter = 1;
	bool		m_bMarked = false;
	int			m_iOrder = 0;

	explicit MarkedNode_t ( const XQNode_t * pTree=nullptr )
		: m_pTree ( pTree )
	{}

	void Mark()
	{
		++m_iCounter;
		m_bMarked = true;
	}

	void Unmark()
	{
		if ( m_bMarked && m_iCounter>1 )
			--m_iCounter;
		if ( m_iCounter<2 )
			m_bMarked = false;
	}
};

struct XqTreeComparator_t
{
	CSphVector<const XQKeyword_t *>		m_dTerms1;
	CSphVector<const XQKeyword_t *>		m_dTerms2;

	bool IsEqual ( const XQNode_t * pNode1, const XQNode_t * pNode2 );
	bool CheckCollectTerms ( const XQNode_t * pNode1, const XQNode_t * pNode2 );
};

using CDwordHash = CSphOrderedHash < DWORD, uint64_t, IdentityHash_fn, 128 >;
using CSubtreeHash = CSphOrderedHash < MarkedNode_t, uint64_t, IdentityHash_fn, 128 >;

namespace {
/// Decides if given pTree is appropriate for caching or not. Currently, we don't cache
/// the end values (leafs).
bool IsAppropriate ( const XQNode_t * pTree )
{
	if ( !pTree ) return false;

	// skip nodes that actually are leaves (eg. "AND smth" node instead of merely "smth")
	return !( pTree->dWords().GetLength()==1 && pTree->GetOp()!=SPH_QUERY_NOT );
}

/// check hashes, then check subtrees, then flag
void FlagCommonSubtrees ( const XQNode_t * pTree, CSubtreeHash & hSubTrees, XqTreeComparator_t & tCmp, bool bFlag, bool bMark = false )
{
	if ( !IsAppropriate ( pTree ) )
		return;

	// we do not yet have any collisions stats,
	// but chances are we don't actually need IsEqualTo() at all
	uint64_t iHash = pTree->GetHash();
	if ( bFlag && hSubTrees.Exists ( iHash ) && tCmp.IsEqual ( hSubTrees [ iHash ].m_pTree, pTree ) )
	{
		hSubTrees[iHash].Mark();

		// we just add all the children but do NOT mark them as common
		// so that only the subtree root is marked.
		// also we unmark all the cases which were eaten by bigger trees
		for ( const XQNode_t * pChild : pTree->dChildren() )
			if ( !hSubTrees.Exists ( pChild->GetHash() ) )
				FlagCommonSubtrees ( pChild, hSubTrees, tCmp, false, bMark );
			else
				FlagCommonSubtrees ( pChild, hSubTrees, tCmp, false );
	} else
	{
		if ( bMark )
			hSubTrees.Add ( MarkedNode_t ( pTree ), iHash );
		else
			hSubTrees[iHash].Unmark();

		for ( const XQNode_t* pChild : pTree->dChildren() )
			FlagCommonSubtrees ( pChild, hSubTrees, tCmp, bFlag, bMark );
	}
}


void SignCommonSubtrees ( const XQNode_t * pTree, const CSubtreeHash & hSubTrees )
{
	if ( !pTree )
		return;

	uint64_t iHash = pTree->GetHash();
	const MarkedNode_t * pCommon = hSubTrees ( iHash );
	if ( pCommon && pCommon->m_bMarked )
		pTree->TagAsCommon ( pCommon->m_iOrder, pCommon->m_iCounter );

	for ( XQNode_t* pChild : pTree->dChildren() )
		SignCommonSubtrees ( pChild, hSubTrees );
}

} // namespace


// stores the pair of a tree, and the bitmask of common nodes
// which contains the tree.
class BitMask_t
{
	const XQNode_t *		m_pTree = nullptr;
	uint64_t		m_uMask = 0ull;

public:
	BitMask_t() = default;
	BitMask_t ( const XQNode_t * pTree, uint64_t uMask ) : m_pTree(pTree), m_uMask(uMask)
	{}

	uint64_t GetMask() const { return m_uMask; }
	const XQNode_t * GetTree() const { return m_pTree; }
};

// a list of unique values.
class Associations_t : public CDwordHash
{
public:

	// returns true when add the second member.
	// The reason is that only one is not interesting for us,
	// but more than two will flood the caller.
	bool Associate2nd ( uint64_t uTree )
	{
		if ( Exists ( uTree ) )
			return false;
		Add ( 0, uTree );
		return GetLength()==2;
	}

	// merge with another similar
	void Merge ( const Associations_t& parents )
	{
		for ( const auto& tAssoc : parents )
			Associate2nd ( tAssoc.first );
	}
};

// associate set of nodes, common bitmask for these nodes,
// and gives the < to compare different pairs
class BitAssociation_t
{
	const Associations_t *	m_pAssociations = nullptr;
	mutable int				m_iBits = 0;

	// The key method of subtree selection.
	// Most 'heavy' subtrees will be extracted first.
	int GetWeight() const
	{
		assert ( m_pAssociations );
		int iNodes = m_pAssociations->GetLength();
		if ( m_iBits==0 && m_uMask!=0 )
		{
			for ( uint64_t dMask = m_uMask; dMask; dMask >>=1 )
				m_iBits += (int)( dMask & 1 );
		}

		// current working formula is num_nodes^2 * num_hits
		return iNodes * iNodes * m_iBits;
	}

public:
	uint64_t			m_uMask = 0;

	BitAssociation_t() = default;

	void Init ( uint64_t uMask, const Associations_t* dNodes )
	{
		m_uMask = uMask;
		m_pAssociations = dNodes;
		m_iBits = 0;
	}

	bool operator< (const BitAssociation_t& second) const
	{
		return GetWeight() < second.GetWeight();
	}
};

// for pairs of values builds and stores the association "key -> list of values"
class CAssociations_t
	: public CSphOrderedHash < Associations_t, uint64_t, IdentityHash_fn, 128 >
{
	int		m_iBits = 0;;			// number of non-unique associations

public:
	CAssociations_t() = default;

	// Add the given pTree into the list of pTrees, associated with given uHash
	int Associate ( const XQNode_t * pTree, uint64_t uHash )
	{
		if ( !Exists ( uHash ) )
			Add ( Associations_t(), uHash );
		if ( operator[]( uHash ).Associate2nd ( pTree->GetHash() ) )
			++m_iBits;
		return m_iBits;
	}

	// merge the existing association of uHash with given chain
	void MergeAssociations ( const Associations_t & chain, uint64_t uHash )
	{
		if ( !Exists ( uHash ) )
			Add ( chain, uHash );
		else
			operator[]( uHash ).Merge ( chain );
	}

	int GetSize() const { return m_iBits; }
};

// The main class for working with common subtrees
class RevealCommon_t : ISphNoncopyable
{
	static constexpr int		MAX_MULTINODES = 64;
	CSphVector<BitMask_t>		m_dBitmasks;		// all bitmasks for all the nodes
	CSphVector<uint64_t>		m_dSubQueries;		// final vector with roadmap for tree division.
	CAssociations_t				m_hNodes;			// initial accumulator for nodes
	CAssociations_t				m_hInterSections;	// initial accumulator for nodes
	CDwordHash					m_hBitOrders;		// order numbers for found common subnodes
	XQOperator_e				m_eOp;				// my operator which I process

	// returns the order for given uHash (if any).
	int GetBitOrder ( uint64_t uHash ) const
	{
		if ( !m_hBitOrders.Exists ( uHash ) )
			return -1;
		return m_hBitOrders[uHash];
	}

	// recursively scans the whole tree and builds the maps
	// where a list of parents associated with every "leaf" nodes (i.e. with children)
	bool BuildAssociations ( const XQNode_t * pTree )
	{
		if ( IsAppropriate ( pTree ) )
		{
			ARRAY_FOREACH ( i, pTree->dChildren() )
			if ( ( !BuildAssociations ( pTree->dChild(i) ) )
				|| ( ( m_eOp==pTree->GetOp() )
				&& ( m_hNodes.Associate ( pTree, pTree->dChild(i)->GetHash() )>=MAX_MULTINODES ) ) )
			{
				return false;
			}
		}
		return true;
	}

	// Find all leafs, non-unique across the tree,
	// and associate the order number with every of them
	bool CalcCommonNodes ()
	{
		if ( !m_hNodes.GetSize() )
			return false; // there is totally no non-unique leaves
		int iBit = 0;
		for ( const auto& tNode : m_hNodes )
			if ( tNode.second.GetLength() > 1 )
				m_hBitOrders.Add ( iBit++, tNode.first );
		assert ( m_hNodes.GetSize()==m_hBitOrders.GetLength() );
		m_hNodes.Reset(); ///< since from now we don't need this data anymore
		return true;
	}

	// recursively builds for every node the bitmaks
	// of common nodes it has as children
	void BuildBitmasks ( const XQNode_t * pTree )
	{
		if ( !IsAppropriate ( pTree ) )
			return;

		if ( m_eOp==pTree->GetOp() )
		{
			// calculate the bitmask
			uint64_t dMask = 0;
			for ( const XQNode_t* pChild : pTree->dChildren() )
			{
				int iOrder = GetBitOrder ( pChild->GetHash() );
				if ( iOrder>=0 )
					dMask |= 1ull << iOrder;
			}

			// add the bitmask into the array
			if ( dMask )
				m_dBitmasks.Add ( { pTree, dMask } );
		}

		// recursively process all the children
		for ( const XQNode_t* pChild : pTree->dChildren() )
			BuildBitmasks ( pChild );
	}

	// Collect all possible intersections of Bitmasks.
	// For every non-zero intersection we collect the list of trees which contain it.
	void CalcIntersections ()
	{
		// Round 1. Intersect all content of bitmasks one-by-one.
		ARRAY_FOREACH ( i, m_dBitmasks )
			for ( int j = i+1; j<m_dBitmasks.GetLength(); ++j )
			{
				// intersect one-by-one and group (grouping is done by nature of a hash)
				if ( uint64_t uMask = m_dBitmasks[i].GetMask() & m_dBitmasks[j].GetMask() )
				{
					m_hInterSections.Associate ( m_dBitmasks[i].GetTree(), uMask );
					m_hInterSections.Associate ( m_dBitmasks[j].GetTree(), uMask );
				}
			}

		// Round 2. Intersect again all collected intersection one-by-one - until zero.
		for ( auto pIt1 = m_hInterSections.begin(); pIt1 != CAssociations_t::end(); ++pIt1 )
		{
			auto pIt2 = pIt1;
			for ( ++pIt2; pIt2 != CAssociations_t::end(); ++pIt2 )
			{
				assert ( pIt1->first != pIt2->first );
				if ( auto uMask = pIt1->first & pIt2->first )
				{
					m_hInterSections.MergeAssociations ( pIt1->second, uMask );
					m_hInterSections.MergeAssociations ( pIt2->second, uMask );
				}
			}
		}
	}

	// create the final kit of common-subsets
	// which we will actually reveal (extract) from original trees
	void MakeQueries()
	{
		CSphVector<BitAssociation_t> dSubnodes; // masks for our selected subnodes
		dSubnodes.Reserve ( m_hInterSections.GetLength() );
		for ( const auto& tInterSection : m_hInterSections )
			dSubnodes.Add().Init( tInterSection.first, & tInterSection.second );

		// sort by weight descending (weight sorting is hold by operator <)
		dSubnodes.RSort();
		m_dSubQueries.Reset();

		// make the final subtrees vector: get one-by-one from the beginning,
		// intersect with all the next and throw out zeros.
		// The final subqueries will not be intersected between each other.
		ARRAY_FOREACH ( i, dSubnodes )
		{
			uint64_t uMask = dSubnodes[i].m_uMask;
			m_dSubQueries.Add ( uMask );
			int j = i+1;
			while ( j < dSubnodes.GetLength() )
			{
				if ( !( dSubnodes[j].m_uMask &= ~uMask ) )
					dSubnodes.Remove(j);
				else
					++j;
			}
		}
	}

	// Now we finally extract the common subtrees from original tree
	// and (recursively) from its children
	void Reorganize ( XQNode_t * pTree )
	{
		if ( !IsAppropriate ( pTree ) )
			return;

		if ( m_eOp==pTree->GetOp() )
		{
			pTree->WithChildren ( [this,pTree] ( auto & dChildren ) {
			// pBranch is for common subset of children, pOtherChildren is for the rest.
			CSphOrderedHash < CSphVector<XQNode_t *>, int, IdentityHash_fn, 64 > hBranches;
			CSphVector<XQNode_t*> dOtherChildren;
			int iOptimizations = 0;
			ARRAY_FOREACH ( i, dChildren )
			{
				int iBit = GetBitOrder ( dChildren[i]->GetHash() );

				// works only with children which are actually common with somebody else
				if ( iBit>=0 )
				{
					// since subqueries doesn't intersect between each other,
					// the first hit we found in this loop is exactly what we searched.
					ARRAY_FOREACH ( j, m_dSubQueries )
						if ( ( 1ull << iBit ) & m_dSubQueries[j] )
						{
							auto& dNode = hBranches.AddUnique (j);
							dNode.Add ( dChildren[i] );

							// Count essential subtrees (with at least 2 children)
							if ( dNode.GetLength()==2 )
								++iOptimizations;

							break;
						}
					// another nodes add to the set of "other" children
				} else
				{
					dOtherChildren.Add ( dChildren[i] );
				}
			}

			// we don't reorganize explicit simple case - as no "others" and only one common.
			// Also reject optimization if there is nothing to optimize.
			if ( iOptimizations && !( dOtherChildren.IsEmpty() && hBranches.GetLength()==1 ) )
			{
				// reorganize the tree: replace the common subset to explicit node with
				// only common members inside. This will give the possibility
				// to cache the node.
				dChildren.Reset();
				dChildren.SwapData ( dOtherChildren );

				for ( auto& tBranch : hBranches )
				{
					if ( tBranch.second.GetLength()==1 )
						dChildren.Add ( tBranch.second[0] );
					else
					{
						auto pNode = new XQNode_t ( pTree->m_dSpec );
						pNode->SetOp ( m_eOp, tBranch.second );
						dChildren.Add ( pNode );
						pNode->m_pParent = pTree;
					}
				}
			}
			} );
		}

		// recursively process all the children
		pTree->WithChildren ( [this](auto& dChildren) {
			dChildren.for_each ( [this]( XQNode_t* pChild ) { Reorganize ( pChild ); });
		});
	}

public:
	explicit RevealCommon_t ( XQOperator_e eOp )
		: m_eOp ( eOp )
	{}

	// actual method for processing tree and reveal (extract) common subtrees
	void Transform ( int iXQ, const XQQuery_t * pXQ )
	{
		// collect all non-unique nodes
		for ( int i=0; i<iXQ; i++ )
			if ( !BuildAssociations ( pXQ[i].m_pRoot ) )
				return;

		// count and order all non-unique nodes
		if ( !CalcCommonNodes() )
			return;

		// create and collect bitmask for every node
		for ( int i=0; i<iXQ; i++ )
			BuildBitmasks ( pXQ[i].m_pRoot );

		// intersect all bitmasks one-by-one, and also intersect all intersections
		CalcIntersections();

		// the die-hard: actually select the set of subtrees which we'll process
		MakeQueries();

		// ... and finally - process all our trees.
		for ( int i=0; i<iXQ; i++ )
			Reorganize ( pXQ[i].m_pRoot );
	}
};


int sphMarkCommonSubtrees ( int iXQ, const XQQuery_t * pXQ )
{
	if ( iXQ<=0 || !pXQ )
		return 0;

	{ // Optional reorganize tree to extract common parts
		RevealCommon_t ( SPH_QUERY_AND ).Transform ( iXQ, pXQ );
		RevealCommon_t ( SPH_QUERY_OR ).Transform ( iXQ, pXQ );
	}

	// flag common subtrees and refcount them
	XqTreeComparator_t tCmp;
	CSubtreeHash hSubtrees;
	for ( int i=0; i<iXQ; i++ )
		FlagCommonSubtrees ( pXQ[i].m_pRoot, hSubtrees, tCmp, true, true );

	// number marked subtrees and assign them order numbers.
	int iOrder = 0;
	for ( auto& tSubtree : hSubtrees )
		if ( tSubtree.second.m_bMarked )
			tSubtree.second.m_iOrder = iOrder++;

	// copy the flags and orders to original trees
	for ( int i=0; i<iXQ; i++ )
		SignCommonSubtrees ( pXQ[i].m_pRoot, hSubtrees );

	return iOrder;
}

bool XqTreeComparator_t::IsEqual ( const XQNode_t * pNode1, const XQNode_t * pNode2 )
{
	// early out check to skip allocations
	if ( !pNode1 || !pNode2 || pNode1->GetHash()!=pNode2->GetHash() || pNode1->GetOp()!=pNode2->GetOp () )
		return false;

	// need reset data from previous compare
	m_dTerms1.Resize ( 0 );
	m_dTerms2.Resize ( 0 );
	// need reserve some space for first compare
	m_dTerms1.Reserve ( 64 );
	m_dTerms2.Reserve ( 64 );

	if ( !CheckCollectTerms ( pNode1, pNode2 ) )
		return false;

	assert ( m_dTerms1.GetLength ()==m_dTerms2.GetLength () );

	if ( !m_dTerms1.GetLength() )
		return true;

	m_dTerms1.Sort ( Lesser ( [] ( const auto& l, const auto& r ) { return l->m_iAtomPos < r->m_iAtomPos; } ) );
	m_dTerms2.Sort ( Lesser ( [] ( const auto& l, const auto& r ) { return l->m_iAtomPos < r->m_iAtomPos; } ) );

	if ( m_dTerms1[0]->m_sWord!=m_dTerms2[0]->m_sWord )
		return false;

	for ( int i=1; i<m_dTerms1.GetLength(); i++ )
	{
		int iDelta1 = m_dTerms1[i]->m_iAtomPos - m_dTerms1[i-1]->m_iAtomPos;
		int iDelta2 = m_dTerms2[i]->m_iAtomPos - m_dTerms2[i-1]->m_iAtomPos;
		if ( iDelta1!=iDelta2 || m_dTerms1[i]->m_sWord!=m_dTerms2[i]->m_sWord )
			return false;
	}

	return true;
}

bool XqTreeComparator_t::CheckCollectTerms ( const XQNode_t * pNode1, const XQNode_t * pNode2 )
{
	// early out
	if ( !pNode1 || !pNode2
			|| pNode1->GetHash ()!=pNode2->GetHash () || pNode1->GetOp ()!=pNode2->GetOp ()
			|| pNode1->dWords().GetLength ()!=pNode2->dWords().GetLength ()
			|| pNode1->dChildren().GetLength ()!=pNode2->dChildren().GetLength () )
		return false;

	// for plain nodes compare keywords
	ARRAY_FOREACH ( i, pNode1->dWords() )
		m_dTerms1.Add ( pNode1->dWords().Begin() + i );

	ARRAY_FOREACH ( i, pNode2->dWords() )
		m_dTerms2.Add ( pNode2->dWords().Begin () + i );

	// for non-plain nodes compare children
	ARRAY_FOREACH ( i, pNode1->dChildren() )
	{
		if ( !CheckCollectTerms ( pNode1->dChild(i), pNode2->dChild(i) ) )
			return false;
	}

	return true;
}