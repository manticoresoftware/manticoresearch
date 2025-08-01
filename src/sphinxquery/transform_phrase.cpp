//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "sphinxquery.h"
#include "std/generics.h"
#include <stack>

static int g_iMaxVariants = 1024;

using VecKw = CSphVector < const XQKeyword_t * >;
using VVKw = CSphVector < VecKw >;

static VVKw ComputeCartesianProduct ( const VVKw & dNodes, VecTraits_T<int> & dComponentBounds, int iMaxVariants, CSphString & sError ) 
{
	if ( dNodes.IsEmpty() )
		return {};

	// fail after calc complexity
	int iTotalVariants = 1;
	int iStart = 0;
	for ( int iEnd : dComponentBounds )
	{
		int iSize = iEnd - iStart;
		iStart = iEnd;
		if ( !iSize )
			continue;

		iTotalVariants *= iSize;
		if ( iTotalVariants>iMaxVariants )
		{
			sError.SetSprintf ( "query too complex, exceeds max variants limit during OR combination (complexity %d, expansion_phrase_limit %d)", iTotalVariants, iMaxVariants );
			return {};
		}
	}

	VVKw dRes;
	dRes.Add();
	VVKw dTmp;

	iStart = 0;
	for ( int iEnd : dComponentBounds )
	{
		int iSize = iEnd - iStart;
		int iCur = iStart;
		iStart = iEnd;
		if ( !iSize )
			continue;

		dTmp.Resize ( 0 );
		dTmp.Reserve ( dRes.GetLength() * iSize );

		for (const VecKw & dCur : dRes )
		{
			for ( int i=iCur; i<iEnd; ++i )
			{
				const VecKw & dNew = dNodes[i];
				VecKw dCombined = dCur;
				dCombined.Append ( dNew );
				dTmp.Add ( std::move ( dCombined ) );
			}
		}
		// replace with longer seq
		dRes.SwapData( dTmp );
	}

	return dRes;
}

struct PtrHash_fn
{
	static uint64_t Hash ( const XQNode_t * pNode ) { return (uint64_t) pNode; }
};

VVKw GenVariants ( const XQNode_t * pRoot, int iMaxVariants, CSphString & sError )
{
	if ( !pRoot )
		return {};

	// stack for post order travers
	std::stack<const XQNode_t*> dNodes;
	dNodes.push ( pRoot );
	
	CSphOrderedHash<VVKw, const XQNode_t *, PtrHash_fn, 256> hResolved;
	VVKw dComponents;
	CSphVector<int> dComponentBounds;

	while ( !dNodes.empty() && sError.IsEmpty() )
	{
		const XQNode_t * pCur = dNodes.top();

		bool bResolved = true;
		for ( const XQNode_t * pChild : pCur->dChildren() )
		{
			if ( !hResolved.Exists ( pChild ) )
			{
				dNodes.push ( pChild );
				bResolved = false;
			}
		}

		if ( !bResolved )
			continue;

		dNodes.pop();
		VVKw dCurVariants;

		switch ( pCur->GetOp() )
		{
			case SPH_QUERY_OR:
			{
				// OR additive op ( union of all variants )
				for ( const auto & tWord : pCur->dWords() )
					dCurVariants.Add().Add( &tWord );

				for ( const XQNode_t * pChild : pCur->dChildren() )
				{
					const auto & dChildVariants = hResolved[ pChild ];
					dCurVariants.Append ( dChildVariants );
				}
			}
			break;

			case SPH_QUERY_PHRASE:
			case SPH_QUERY_PROXIMITY:
			case SPH_QUERY_QUORUM:
			case SPH_QUERY_AND:
			{
				// PHRASE multiplicative op ( cartesian product )
				dComponents.Resize ( 0 );
				dComponentBounds.Resize ( 0 );

				for ( const auto & tWord : pCur->dWords() )
				{
					auto & dCur = dComponents.Add();
					dCur.Resize ( 0 ); // could be reused vector with previous data - need to reset!!!
					dCur.Add( &tWord );
					dComponentBounds.Add ( dComponents.GetLength() );
				}

				for ( const XQNode_t * tChild : pCur->dChildren() )
				{
					const VVKw & dChild = hResolved [ tChild ];
					dComponents.Append ( dChild );
					dComponentBounds.Add ( dComponents.GetLength() );
				}
				
				// order matters for phrase
				dCurVariants = ComputeCartesianProduct ( dComponents, dComponentBounds, iMaxVariants, sError );
			}
			break;

			default:
				sError.SetSprintf ( "unsupported node '%s' at the phrase", XQOperatorNameSz ( pCur->GetOp() ) );
				break;
		}
		
		if  ( dCurVariants.GetLength()>iMaxVariants )
		{
			 sError.SetSprintf ( "query too complex, exceeds max variants limit during OR combination (complexity %d, expansion_phrase_limit %d)", dCurVariants.GetLength(), iMaxVariants );
			 break;
		}

		hResolved.AddUnique ( pCur ) = std::move ( dCurVariants );
	}

	// result variants for the root node
	if ( sError.IsEmpty() )
		return hResolved[ pRoot ];
	else
		return {};
}

static bool CheckTransform ( const XQNode_t * pNode )
{
	assert ( pNode );

	XQOperator_e eOp = pNode->GetOp();
	return ( eOp==SPH_QUERY_PHRASE || eOp==SPH_QUERY_QUORUM || eOp==SPH_QUERY_PROXIMITY );
}

static void DoTransform ( XQNode_t * pNode, CSphString & sError )
{
	assert ( pNode );

	VVKw dVariants = GenVariants ( pNode, g_iMaxVariants, sError );
	if ( !sError.IsEmpty() )
		return;

	CSphVector < XQNode_t * > dPhrases;
	for ( const auto & dTerms : dVariants )
	{
		assert ( dTerms.GetLength() );
		int iStartPos = dTerms[0]->m_iAtomPos;
		XQNode_t * pPhrase = new XQNode_t ( pNode->m_dSpec );

		for ( const auto & tSrcTerm : dTerms )
		{
			XQKeyword_t tTerm = *tSrcTerm;
			tTerm.m_iAtomPos = iStartPos++;
			pPhrase->AddDirtyWord ( tTerm );
		}

		pPhrase->m_iOpArg = pNode->m_iOpArg;
		pPhrase->m_bPercentOp = pNode->m_bPercentOp;
		pPhrase->SetOp ( pNode->GetOp() );
		dPhrases.Add ( pPhrase );
	}

	pNode->ResetWords();
	pNode->WithChildren ( [] ( auto & dChildren ) { for ( auto * pChild : dChildren ) SafeDelete ( pChild ); } );
	pNode->ResetChildren();
	pNode->SetOp ( SPH_QUERY_OR, dPhrases );
}

bool TransformPhraseBased ( XQNode_t ** ppNode, CSphString & sError )
{
	XQNode_t *& pNode = *ppNode;

	if ( CheckTransform ( pNode ) && pNode->dChildren().GetLength() )
		DoTransform ( pNode, sError );
	else
	{
		ARRAY_FOREACH ( i, pNode->dChildren() )
			TransformPhraseBased ( &pNode->dChildren()[i], sError );
	}

	return sError.IsEmpty();
}

void SetExpansionPhraseLimit ( int iMaxVariants )
{
	g_iMaxVariants = iMaxVariants;
}