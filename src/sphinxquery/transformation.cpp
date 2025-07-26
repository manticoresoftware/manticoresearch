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
#include "std/openhash.h"

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

struct CSphHornerStrHashFunc
{
	static int Hash ( const CSphString& sKey )
	{
		int iHash = 0;
		if ( !sKey.cstr() )
			return iHash;

		for ( auto * pStr = sKey.cstr(); *pStr; ++pStr )
			iHash = iHash * 257 + *pStr;

		return iHash;
	}
};

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
	OpenHashTable_T<const CSphString*, int, CSphStrPtrHashFunc<CSphHornerStrHashFunc>> hCosts;
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
			int * pCost = hCosts.Find ( &sWord );
			if ( pCost )
				continue;

			Verify ( hCosts.Add ( &sWord, 0 ) );
			dKeywords.Add();
			dKeywords.Last().m_sTokenized = sWord;
			dKeywords.Last().m_iDocs = 0;
		}
	}

	// get keywords info from index dictionary
	if ( !dKeywords.IsEmpty() )
	{
		m_pKeywords->FillKeywords ( dKeywords );
		for_each ( dKeywords, [&hCosts] (const auto& tKeyword) { *hCosts.Find ( &tKeyword.m_sTokenized) = tKeyword.m_iDocs; } );
	}

	// propagate cost bottom-up (from children to parents)
	for ( int i=dChildren.GetLength()-1; i>=0; --i )
	{
		XQNode_t * pChild = dChildren[i];
		int iCost = 0;
		for_each ( pChild->dWords(), [&iCost,&hCosts] (const auto& dWord) { iCost += *hCosts.Find(&dWord.m_sWord); } );
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
#define DUMP(NameID) do {StringBuilder_c foo; foo << "\n" << m_uTotalSuccess << " After transformation " << (int)NameID+1 << ", " << NameOfTransformation(NameID); Dump ( bDump ? *m_ppRoot : nullptr, foo.cstr() ); } while (false)
#else
#define DUMP(NameID)
#endif

#define TRANSFORM(Group,Subgroup,fhChecker,fnTransformer,NameID) if ( CollectInfo <Group, Subgroup> ( *m_ppRoot, &fhChecker ) ) \
{ \
	const bool bDump = fnTransformer (); \
	Tick (NameID, bDump); \
	if ( bDump ) { bRecollect = true; } \
	DUMP ( NameID );\
}

void CSphTransformation::Tick ( eTransformations eOp, bool bResult) noexcept
{
#if XQDEBUG_STAT
	const auto uTimestamp = sphMicroTimer();
	const auto uTime = uTimestamp - std::exchange (m_tmStartTime, uTimestamp);
	auto& tStats = m_dTransStats[size_t ( eOp )];
	if ( bResult )
	{
		++tStats.m_uSuccess;
		tStats.m_uTimeSuccess += uTime;
		++m_uTotalSuccess;
	} else
	{
		++tStats.m_uFailed;
		tStats.m_uTimeFailed += uTime;
		++m_uTotalFailed;
	}
#endif
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
	m_tmStartTime = sphMicroTimer();
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
	OpenHashSet_T<uintptr_t, IdentityHash_fn> hDupes {32};
	return dSimilarNodes.any_of ( [&hDupes] ( auto * pX ) { return !hDupes.Add ( (uintptr_t) pX->m_pParent ); } );
}

bool HasSameGrand ( const VecTraits_T<XQNode_t *> & dSimilarNodes ) noexcept
{
	OpenHashSet_T<uintptr_t, IdentityHash_fn> hDupes {32};
	return dSimilarNodes.any_of ( [&hDupes] ( auto * pX ) { return !hDupes.Add ( (uintptr_t) pX->m_pParent->m_pParent ); } );
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

// remove nodes without children up the tree
bool SubtreeRemoveEmpty ( XQNode_t * pNode )
{
	if ( !pNode->IsEmpty() )
		return false;

	// climb up
	XQNode_t * pParent = pNode->m_pParent;
	while ( pParent && pParent->dChildren().GetLength()<=1 && pParent->dWords().IsEmpty() )
		pNode = std::exchange ( pParent, pParent->m_pParent );

	if ( pParent )
		pParent->RemoveChild ( pNode );

	// free subtree
	SafeDelete ( pNode );
	return true;
}

// eliminate composite ( AND / OR ) nodes with only one child
void CompositeFixup ( XQNode_t * pNode, XQNode_t ** ppRoot )
{
	assert ( pNode );
	if ( !pNode->dWords().IsEmpty() || pNode->dChildren().GetLength()!=1 || ( pNode->GetOp()!=SPH_QUERY_OR && pNode->GetOp()!=SPH_QUERY_AND ) )
		return;

	XQNode_t * pChild = pNode->dChildren()[0];
	pChild->m_pParent = nullptr;
	pNode->WithChildren ( [] ( auto & dChildren ) { dChildren.Resize ( 0 ); } );

	// climb up
	XQNode_t * pParent = pNode->m_pParent;
	while ( pParent && pParent->dChildren().GetLength()==1 && pParent->dWords().IsEmpty() &&
		( pParent->GetOp()==SPH_QUERY_OR || pParent->GetOp()==SPH_QUERY_AND ) )
	{
		pNode = std::exchange (pParent, pParent->m_pParent);
	}

	if ( pParent )
	{
		for ( auto& dChild : pParent->dChildren() )
		{
			if ( dChild!=pNode )
				continue;

			dChild = pChild;
			pChild->m_pParent = pParent;
			pParent->Rehash();
			break;
		}
	} else
	{
		*ppRoot = pChild;
	}

	// free subtree
	SafeDelete ( pNode );
}

void CleanupSubtree ( XQNode_t * pNode, XQNode_t ** ppRoot )
{
	if ( SubtreeRemoveEmpty ( pNode ) )
		return;

	CompositeFixup ( pNode, ppRoot );
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
	StringBuilder_c tFormat;
	tFormat.Sprintf ( "optimized boolean in %.3t, %d loop(s), %d success, %d failed", tmDelta, qInfo.m_uTurns, qInfo.m_uTotalSuccess, qInfo.m_uTotalFailed );
	printf ( "%s\n", tFormat.cstr() );
	tFormat.Clear ();
	for (int i=0; i<std::size(qInfo.m_dTransStats); ++i)
	{
		const auto& tStats = qInfo.m_dTransStats[i];
		if ( 0==(tStats.m_uSuccess+tStats.m_uFailed )) continue;
		tFormat.Sprintf ( "%d/%d\t%.3Fs/%.3Fs\t%s", tStats.m_uSuccess, tStats.m_uFailed, tStats.m_uTimeSuccess/1000, tStats.m_uTimeFailed/1000, NameOfTransformation (static_cast<eTransformations> (i)));
		printf ( "%s\n", tFormat.cstr() );
		tFormat.Clear ();
	}
#endif
}
