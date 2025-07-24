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

#include "sphinxquery.h"
#include "xqdebug.h"

CSphTransformation::CSphTransformation ( XQNode_t ** ppRoot, const ISphKeywordsStat * pKeywords )
	: m_pKeywords ( pKeywords )
	, m_ppRoot ( ppRoot )
{}


template < typename Group, typename SubGroup >
void CSphTransformation::TreeCollectInfo ( XQNode_t * pNode, const Checker_fn pfnChecker, int iDeep )
{
	if ( !pNode )
		return;

	if ( pfnChecker ( pNode ) )
	{
		// "Similar nodes" are nodes which are suited to a template (like 'COMMON NOT', 'COMMON COMPOUND NOT', ...)
		const uint64_t uGroup = (const uintptr_t) Group::From ( pNode );
		const uint64_t uSubGroup = SubGroup::By ( pNode );

		auto & hGroup = m_hSimilar.AddUnique ( uGroup );
		hGroup.iDeep = iDeep;
		hGroup.tHash.AddUnique ( uSubGroup ).Add ( pNode );
	}

	for ( const auto& dChild : pNode->dChildren() )
		TreeCollectInfo<Group, SubGroup> ( dChild, pfnChecker, iDeep + 1 );
}


template < typename Group, typename SubGroup >
bool CSphTransformation::CollectInfo ( XQNode_t * pNode, Checker_fn pfnChecker )
{
	( *m_ppRoot )->Check ( true );
	m_hSimilar.Reset();

	TreeCollectInfo<Group, SubGroup> ( pNode, pfnChecker, 1 );
	return ( m_hSimilar.GetLength()>0 );
}


void CSphTransformation::SetCosts ( XQNode_t * pNode, const CSphVector<XQNode_t *> & dNodes ) const noexcept
{
	assert ( pNode || dNodes.GetLength() );

	if ( !m_pKeywords )
		return;

	CSphVector<XQNode_t*> dChildren ( dNodes.GetLength() + 1 );
	dChildren[dNodes.GetLength()] = pNode;
	ARRAY_FOREACH ( i, dNodes )
	{
		dChildren[i] = dNodes[i];
		dChildren[i]->m_iUser = 0;
	}

	// collect unknown keywords from all children
	CSphVector<CSphKeywordInfo> dKeywords;
	SmallStringHash_T<int> hCosts;
	ARRAY_FOREACH ( i, dChildren ) // don't use 'ranged for' here, as dChildren modified during the loop
	{
		const XQNode_t * pChild = dChildren[i];
		for ( XQNode_t* pGrandChild : pChild->dChildren() )
		{
			dChildren.Add ( pGrandChild );
			dChildren.Last()->m_iUser = 0;
			assert ( dChildren.Last()->m_pParent==pChild );
		}
		for ( const auto& dWord : pChild->dWords() )
		{
			const CSphString & sWord = dWord.m_sWord;
			int * pCost = hCosts ( sWord );
			if ( pCost )
				continue;

			Verify ( hCosts.Add ( 0, sWord ) );
			dKeywords.Add();
			dKeywords.Last().m_sTokenized = sWord;
			dKeywords.Last().m_iDocs = 0;
		}
	}

	// get keywords info from index dictionary
	if ( dKeywords.GetLength() )
	{
		m_pKeywords->FillKeywords ( dKeywords );
		for_each ( dKeywords, [&hCosts] (const auto& tKeyword) { hCosts[tKeyword.m_sTokenized] = tKeyword.m_iDocs; } );
	}

	// propagate cost bottom-up (from children to parents)
	for ( int i=dChildren.GetLength()-1; i>=0; --i )
	{
		XQNode_t * pChild = dChildren[i];
		int iCost = 0;
		for_each ( pChild->dWords(), [&iCost,&hCosts] (const auto& dWord) { iCost += hCosts[dWord.m_sWord]; } );
		pChild->m_iUser += iCost;
		if ( pChild->m_pParent )
			pChild->m_pParent->m_iUser += pChild->m_iUser;
	}
}

void CSphTransformation::DumpSimilar () const noexcept
{
#if XQ_DUMP_TRANSFORMED_TREE
	for ( const auto& [hKey1, hSimGroup] : m_hSimilar )
	{
		printf ( "\n%p\n", (void *) hKey1 );
		for (const auto& [hKey2, dleaves] : hSimGroup.tHash) {
			printf ( "\t%p %d\n", (void*)hKey2, hSimGroup.iDeep );
			for ( const XQNode_t * pLeaf : dleaves )
			{
				const uint64_t uParentHash = pLeaf->GetHash();
				printf ( "\t\t%p, leaf %p\n", (void*)uParentHash, pLeaf );
			}
		}
	}
	fflush ( stdout );
#endif
}

#if XQDEBUG
#define DUMP(n,NameID) do {StringBuilder_c foo; foo << "\n" << m_uTotalTransformations << " After transformation " << (int)NameID+1 << ", " << NameOfTransformation(NameID); Dump ( bDump ? *m_ppRoot : nullptr, foo.cstr() ); } while (false)
#else
#define DUMP(n,NameID)
#endif

#define TRANSFORM(Group,Subgroup,fhChecker,fnTransformer,NameID) if ( CollectInfo <Group, Subgroup> ( *m_ppRoot, &fhChecker ) ) \
{ \
	const bool bDump = fnTransformer (); \
	if ( bDump ) { bRecollect = true; ++m_uTotalTransformations; ++m_dTransformations[size_t(NameID)]; } \
	DUMP ( m_uTotalTransformations, NameID );\
}


void CSphTransformation::Transform ()
{
	// (A | "A B"~N) -> A
	// ("A B" | "A B C") -> "A B"
	// ("A B"~N | "A B C"~N) -> ("A B"~N)
	if ( CollectInfo<ParentNode, NullNode> ( *m_ppRoot, &CheckCommonKeywords ) )
	{
		const bool bDump = TransformCommonKeywords ();
		Dump ( bDump ? *m_ppRoot : nullptr, "\nAfter transformation of 'COMMON KEYWORDS'" );
	}

	// ("X A B" | "Y A B") -> (("X|Y") "A B")
	// ("A B X" | "A B Y") -> (("X|Y") "A B")
	if ( CollectInfo <ParentNode, NullNode> ( *m_ppRoot, &CheckCommonPhrase ) )
	{
//		DumpSimilar();
		const bool bDump = TransformCommonPhrase ();
		Dump ( bDump ? *m_ppRoot : nullptr, "\nAfter transformation of 'COMMON PHRASES'" );
	}

	bool bRecollect = false;
	m_uTotalTransformations=0;
	do
	{
		bRecollect = false;

		// AND(OR) node with only 1 child
		TRANSFORM ( NullNode, NullNode, CheckHungOperand, TransformHungOperand, eTransformations::eHung );

		// ((A | B) | C) -> ( A | B | C )
		// ((A B) C) -> ( A B C )
		TRANSFORM ( NullNode, NullNode, CheckExcessBrackets, TransformExcessBrackets, eTransformations::eExcessBrackets );

		// (foo -- bar) -> (foo bar)
		TRANSFORM ( NullNode, NullNode, CheckAndNotNotOperand, TransformAndNotNotOperand, eTransformations::eAndNotNot );

		// ((A !N1) !N2) -> (A !(N1 | N2))
		TRANSFORM ( ParentNode, CurrentNode, CheckExcessAndNot, TransformExcessAndNot, eTransformations::eExcessAndNot );

		// ((A !X) | (A !Y) | (A !Z)) -> (A !(X Y Z))
		TRANSFORM ( GrandNode, CurrentNode, CheckCommonAndNotFactor, TransformCommonAndNotFactor, eTransformations::eCommonAndNotFactor );

		// ((A !N) | (B !N)) -> ((A|B) !N)
		TRANSFORM ( Grand2Node, CurrentNode, CheckCommonNot, TransformCommonNot, eTransformations::eCommonNot );

		// ((A (AA | X)) | (B (BB | X))) -> ((A AA) | (B BB) | ((A|B) X)) [ if cost(X) > cost(A) + cost(B) ]
		TRANSFORM ( Grand2Node, CurrentNode, CheckCommonSubTerm, TransformCommonSubTerm, eTransformations::eCommonSubterm );

		// ((A !(N AA)) | (B !(N BB))) -> (((A|B) !N) | (A !AA) | (B !BB)) [ if cost(N) > cost(A) + cost(B) ]
		TRANSFORM ( Grand3Node, CurrentNode, CheckCommonCompoundNot, TransformCommonCompoundNot, eTransformations::eCommonCompoundNot);

		// ((A !(N | N1)) | (B !(N | N2))) -> (( (A !N1) | (B !N2) ) !N)
		TRANSFORM ( Grand3Node, CurrentNode, CheckCommonOrNot, TransformCommonOrNot, eTransformations::eCommonOrNot );

		++m_uTurns;
	} while ( bRecollect );

	( *m_ppRoot )->Check ( true );
}

#undef DUMP
#undef TRANSFORM

bool HasSameParent ( const VecTraits_T<XQNode_t *> & dSimilarNodes ) noexcept
{
	CSphOrderedHash<int, uintptr_t, IdentityHash_fn, 32> hDupes;
	return dSimilarNodes.any_of ( [&hDupes] ( auto * pX ) { return !hDupes.Add ( 0, (uintptr_t) pX->m_pParent ); } );
}

bool CSphTransformation::ReplaceNode ( XQNode_t * pOldNode, XQNode_t * pNewNode ) noexcept
{
	XQNode_t * pParent = pOldNode->m_pParent;
	if ( !pParent )
		return false;

	assert ( pParent->dChildren().Contains ( pOldNode ) );
	for ( XQNode_t *& pChild: pParent->dChildren() )
	{
		if ( pChild != pOldNode )
			continue;

		pChild = pNewNode;
		pNewNode->m_pParent = pParent;
		pParent->Rehash();
		break;
	}
	return true;
}

void CSphTransformation::AddOrReplaceNode ( XQNode_t * pParent, XQNode_t * pChild ) noexcept
{
	if ( pParent->dChildren().IsEmpty() && pParent->m_pParent )
	{
		ReplaceNode ( pParent, pChild );
		pParent->ResetChildren();
		SafeDelete ( pParent );
		return;
	}
	pParent->AddNewChild ( pChild );
}

void sphOptimizeBoolean ( XQNode_t ** ppRoot, const ISphKeywordsStat * pKeywords )
{
#if XQDEBUG_STAT
	int64_t tmDelta = sphMicroTimer();
#endif

	CSphTransformation qInfo ( ppRoot, pKeywords );
	qInfo.Transform ();

#if XQDEBUG_STAT
	tmDelta = sphMicroTimer() - tmDelta;
	if ( tmDelta<11 )
		return;
	printf ( "optimized boolean in %d.%03d msec, " UINT64_FMT " loops, " UINT64_FMT " ops\n", (int)(tmDelta/1000), (int)(tmDelta%1000), qInfo.m_uTurns, qInfo.m_uTotalTransformations );
	for (int i=0; i<std::size(qInfo.m_dTransformations); ++i)
	{
		const auto iCounter = qInfo.m_dTransformations[i];
		if (!iCounter)
			continue;
		printf ( "%d\t%s\n", iCounter, NameOfTransformation (static_cast<eTransformations> (i)) );
	}
#endif
}
