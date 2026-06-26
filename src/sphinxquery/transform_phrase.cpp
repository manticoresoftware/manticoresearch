//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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
static bool g_bExpansionPhraseWarning = false;

using VecKw = CSphVector < const XQKeyword_t * >;
using VVKw = CSphVector < VecKw >;

static VVKw ComputeCartesianProduct ( const VVKw & dNodes, VecTraits_T<int> & dComponentBounds, int iMaxVariants, bool bAllowPartial, CSphString & sError, CSphString & sWarning ) 
{
	if ( dNodes.IsEmpty() )
		return {};

	// could fail after calc complexity
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
			if ( bAllowPartial )
			{
				sWarning.SetSprintf ( "query is too complex, partial results generated (limited to %d variants)", iMaxVariants );
				break;

			} else
			{
				sError.SetSprintf ( "query too complex, exceeds max variants limit during OR combination (complexity %d, expansion_phrase_limit %d)", iTotalVariants, iMaxVariants );
				return {};
			}
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
		// should be clumped to iMaxVariants
		int iTmpSize = Min ( iMaxVariants, dRes.GetLength() * iSize );
		dTmp.Reserve ( iTmpSize );

		for (const VecKw & dCur : dRes )
		{
			for ( int i=iCur; i<iEnd; ++i )
			{
				// if already hit the limit - stop generate variants for this component
				if ( dTmp.GetLength()>=iMaxVariants )
					break;

				const VecKw & dNew = dNodes[i];
				VecKw dCombined = dCur;
				dCombined.Append ( dNew );
				dTmp.Add ( std::move ( dCombined ) );
			}
			// stop generate from previous results
			if ( dTmp.GetLength()>=iMaxVariants )
				break;
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

VVKw GenVariants ( const XQNode_t * pRoot, int iMaxVariants, bool bAllowPartial, CSphString & sError, CSphString & sWarning )
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
				dCurVariants = ComputeCartesianProduct ( dComponents, dComponentBounds, iMaxVariants, bAllowPartial, sError, sWarning );
			}
			break;

			default:
				sError.SetSprintf ( "unsupported node '%s' at the phrase", XQOperatorNameSz ( pCur->GetOp() ) );
				break;
		}
		
		if ( sError.IsEmpty() && dCurVariants.GetLength()>iMaxVariants )
		{
			if ( bAllowPartial )
			{
				dCurVariants.Resize ( iMaxVariants ); // truncate to allowed size
				sWarning.SetSprintf ( "query is too complex, partial results generated (limited to %d variants)", iMaxVariants );
			} else
			{
				sError.SetSprintf ( "query too complex, exceeds max variants limit during OR combination (complexity %d, expansion_phrase_limit %d)", dCurVariants.GetLength(), iMaxVariants );
			}
		}

		hResolved.AddUnique ( pCur ) = std::move ( dCurVariants );
	}

	// result variants for the root node
	if ( !sError.IsEmpty() )
		return {};
	else
		return hResolved[pRoot];
}

static bool CheckTransform ( const XQNode_t * pNode )
{
	assert ( pNode );

	XQOperator_e eOp = pNode->GetOp();
	return ( eOp==SPH_QUERY_PHRASE || eOp==SPH_QUERY_QUORUM || eOp==SPH_QUERY_PROXIMITY );
}

static void DoTransform ( XQNode_t * pNode, CSphString & sError, CSphString & sWarning )
{
	assert ( pNode );

	VVKw dVariants = GenVariants ( pNode, g_iMaxVariants, g_bExpansionPhraseWarning, sError, sWarning );
	if ( !sError.IsEmpty() )
		return;

	if ( !dVariants.IsEmpty() )
	{
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
}

bool TransformPhraseBased ( XQNode_t ** ppNode, CSphString & sError, CSphString & sWarning )
{
	XQNode_t *& pNode = *ppNode;

	if ( CheckTransform ( pNode ) && pNode->dChildren().GetLength() )
		DoTransform ( pNode, sError, sWarning );
	else
	{
		ARRAY_FOREACH ( i, pNode->dChildren() )
			TransformPhraseBased ( &pNode->dChildren()[i], sError, sWarning );
	}

	return sError.IsEmpty();
}

void SetExpansionPhraseLimit ( int iMaxVariants, bool bExpansionPhraseWarning )
{
	g_iMaxVariants = iMaxVariants;
	g_bExpansionPhraseWarning = bExpansionPhraseWarning;
}