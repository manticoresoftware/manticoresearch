//
// Copyright (c) 2025, Manticore Software LTD (https://manticoresearch.com)
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

// common keywords
// (A | "A B"~N) -> A
// ("A B" | "A B C") -> "A B"
// ("A B"~N | "A B C"~N) -> ("A B"~N)

// common phrase
// ("X A B" | "Y A B") -> (("X|Y") "A B")
// ("A B X" | "A B Y") -> (("X|Y") "A B")

namespace {

constexpr BYTE g_sPhraseDelimiter[] = { 1 };

struct CommonInfo_t
{
	CSphVector<XQNode_t *> *	m_pPhrases;
	int							m_iCommonLen;
	bool						m_bHead;
	bool						m_bHasBetter;
};

struct Node2Common_t
{
	XQNode_t * m_pNode;
	CommonInfo_t * m_pCommon;
};


struct CommonDupElimination_fn
{
	static bool IsLess ( const Node2Common_t & a, const Node2Common_t & b ) noexcept
	{
		if ( a.m_pNode!=b.m_pNode )
			return a.m_pNode<b.m_pNode;

		if ( a.m_pCommon->m_iCommonLen!=b.m_pCommon->m_iCommonLen )
			return a.m_pCommon->m_iCommonLen>b.m_pCommon->m_iCommonLen;

		return a.m_pCommon->m_bHead;
	}
};


struct XQNodeAtomPos_fn
{
	static bool IsLess ( const XQNode_t * a, const XQNode_t * b ) noexcept
	{
		return a->m_iAtomPos<b->m_iAtomPos;
	}
};


uint64_t sphHashPhrase ( const XQNode_t * pNode )
{
	assert ( pNode );
	uint64_t uHash = SPH_FNV64_SEED;

	auto iWords = pNode->dWords().GetLength();
	if ( !iWords )
		return uHash;

	uHash = sphFNV64cont ( pNode->dWord(0).m_sWord.cstr(), uHash );
	if ( iWords==1 )
		return uHash;

	for ( int i=1; i<iWords; ++i )
	{
		uHash = sphFNV64 ( g_sPhraseDelimiter, sizeof(g_sPhraseDelimiter), uHash );
		uHash = sphFNV64cont ( pNode->dWord(i).m_sWord.cstr(), uHash );
	}
	return uHash;
}

//
// The cases when query won't be optimized:
// 1. Proximities with different distance: "A B C"~N | "A B C D"~M (N != M)
// 2. Partial intersection in the middle: "A B C D" | "D B C E" (really they won't be found)
// 3. Weaker phrase for proximity. Example: "A B C D"~N | "B C"
//
// The cases when query will be optimized:
// 1. Found weaker term (phrase or proximity type) for sub-query with phrase type.
// Examples:
// "D A B C E" | "A B C" (weaker phrase) => "A B C"
// "A B C D E" | "B C D"~N (weaker proximity) => "B C D"~N
//
// 2. Equal proximities with the different distance.
// Example: "A B C"~N | "A B C"~M => "A B C"~min(M,N)
//
// 3. Found weaker term with proximity type with the same distance.
// Example: "D A B C E"~N | "A B"~N => "A B"~N
//
bool sphIsNodeStrongest ( const XQNode_t * pNode, const CSphVector<XQNode_t *> & dSimilar )
{
	assert ( pNode );
	const XQOperator_e eNode = pNode->GetOp();
	int iWords = pNode->dWords().GetLength();

	for ( const XQNode_t * pSim : dSimilar )
	{
		const XQOperator_e eSimilar = pSim->GetOp();

		if ( const int iSimilarWords = pSim->dWords().GetLength(); eNode == SPH_QUERY_PROXIMITY && eSimilar == SPH_QUERY_PROXIMITY && iWords > iSimilarWords )
			return false;

		if ( (eNode == SPH_QUERY_PHRASE || eNode == SPH_QUERY_AND) && (eSimilar == SPH_QUERY_PROXIMITY && (iWords > 1 || pNode->dChildren().GetLength())) )
			return false;

		const bool bSimilar = (eNode == SPH_QUERY_PHRASE && eSimilar == SPH_QUERY_PHRASE)
			|| ((eNode == SPH_QUERY_PHRASE || eNode == SPH_QUERY_AND) && (eSimilar == SPH_QUERY_PHRASE || eSimilar==SPH_QUERY_PROXIMITY ))
			|| ( eNode==SPH_QUERY_PROXIMITY && ( eSimilar==SPH_QUERY_AND || eSimilar==SPH_QUERY_PHRASE ) )
			|| ( eNode==SPH_QUERY_PROXIMITY && eSimilar==SPH_QUERY_PROXIMITY && pNode->m_iOpArg>=pSim->m_iOpArg );

		if ( !bSimilar )
			return false;
	}

	return true;
}

int sphBigramAddNode ( XQNode_t * pNode, int64_t uHash, BigramHash_t & hBirgam )
{
	CSphVector<XQNode_t *> * ppNodes = hBirgam ( uHash );
	if ( !ppNodes )
	{
		const CSphVector<XQNode_t *> dNode ( 1 );
		dNode[0] = pNode;
		hBirgam.Add ( dNode, uHash );
		return 1;
	}

	ppNodes->Add ( pNode );
	return ppNodes->GetLength();
}


void sphHashSubphrases ( XQNode_t * pNode, BigramHash_t & hBirgam )
{
	assert ( pNode );
	// skip whole phrase
	if ( pNode->dWords().GetLength()<=1 )
		return;

	const CSphVector<XQKeyword_t> & dWords = pNode->dWords();
	const int iLen = dWords.GetLength();
	for ( int i=0; i<iLen; ++i )
	{
		uint64_t uSubPhrase = sphFNV64cont ( dWords[i].m_sWord.cstr(), SPH_FNV64_SEED );
		sphBigramAddNode ( pNode, uSubPhrase, hBirgam );

		// skip whole phrase
		const int iSubLen = i ? iLen : (iLen-1);
		for ( int j=i+1; j<iSubLen; ++j )
		{
			uSubPhrase = sphFNV64 ( g_sPhraseDelimiter, sizeof(g_sPhraseDelimiter), uSubPhrase );
			uSubPhrase = sphFNV64cont ( dWords[j].m_sWord.cstr(), uSubPhrase );
			sphBigramAddNode ( pNode, uSubPhrase, hBirgam );
		}
	}

	// loop all children
	for ( XQNode_t * pChild : pNode->dChildren() )
		sphHashSubphrases ( pChild, hBirgam );
}
} // namespace

/// Common keywords

//	NOT:
// 		 ______________________ OR (parentOr) _______
//		/                            |               |
//	  pNode (PHRASE|AND|PROXIMITY)  ...	            ...
//
bool CSphTransformation::CheckCommonKeywords ( const XQNode_t * pNode ) noexcept
{
	return ParentNode::Valid ( pNode )
			&& ParentNode::From ( pNode )->GetOp() == SPH_QUERY_OR
			&& !pNode->dWords().IsEmpty();
}


bool CSphTransformation::TransformCommonKeywords () const noexcept
{
	CSphVector<XQNode_t *> dPendingDel;
	for ( auto& [_, hSimGroup] : m_hSimilar )
	{
		BigramHash_t hBigrams;
		for ( auto& [_, dPhrases] : hSimGroup.tHash )
		{
			// Nodes with the same iFuzzyHash
			if ( dPhrases.GetLength()<2 )
				continue;

			for ( XQNode_t * pNode : dPhrases )
				sphHashSubphrases ( pNode, hBigrams );

			for ( const XQNode_t * pNode : dPhrases )
			{
				uint64_t uPhraseHash = sphHashPhrase ( pNode );
				if ( const CSphVector<XQNode_t *> * ppCommon = hBigrams ( uPhraseHash ); ppCommon && sphIsNodeStrongest ( pNode, *ppCommon ) )
				{
					for ( const auto pCommon : *ppCommon )
						dPendingDel.Add ( pCommon );
				}
			}
		}
	}

	const bool bTransformed = ( dPendingDel.GetLength()>0 );
	dPendingDel.Sort(Lesser ( [] ( XQNode_t * a, XQNode_t * b ) { return a < b; } ));
	// Delete stronger terms
	XQNode_t * pLast = nullptr;
	for ( XQNode_t * pPending : dPendingDel ) // by value, as later that pointer will be compared
	{
		// skip dupes
		if ( pLast == pPending )
			continue;

		pLast = pPending;
		Verify ( pLast->m_pParent->RemoveChild ( pLast ) );
		delete ( pPending );
	}

	return bTransformed;
}

/// common phrase

//  NOT:
//		 ______________________ OR (parentOr) ___
// 		/                        |               |
// 	  pNode (PHRASE)            ...	             ...
bool CSphTransformation::CheckCommonPhrase ( const XQNode_t * pNode ) noexcept
{
	if ( !pNode
		|| !pNode->m_pParent
		|| pNode->m_pParent->GetOp() != SPH_QUERY_OR
		|| pNode->GetOp() != SPH_QUERY_PHRASE
		|| pNode->dWords().GetLength()<2 )
	{
		return false;
	}

	// single word phrase not allowed
	assert ( pNode->dWords().GetLength()>=2 );

	// phrase there words not one after another not allowed
	for ( int i=1; i<pNode->dWords().GetLength(); ++i )
		if ( pNode->dWord(i).m_iAtomPos-pNode->dWord(i-1).m_iAtomPos!=1 )
			return false;

	return true;
}

bool CSphTransformation::TransformCommonPhrase () const noexcept
{
	int iActiveDeep = 0;
	for ( auto& [_, hSimGroup] : m_hSimilar )
		for ( auto& [_, dNodes] : hSimGroup.tHash )
		{
			if ( iActiveDeep && iActiveDeep < hSimGroup.iDeep )
				continue;

			// Nodes with the same iFuzzyHash
			if ( dNodes.GetLength()<2 )
				continue;

			bool bHasCommonPhrases = false;
			BigramHash_t tBigramHead;
			BigramHash_t tBigramTail;
			// 1st check only 2 words at head tail at phrases
			ARRAY_FOREACH ( iPhrase, dNodes )
			{
				const CSphVector<XQKeyword_t> & dWords = dNodes[iPhrase]->dWords();
				assert ( dWords.GetLength()>=2 );
				dNodes[iPhrase]->m_iAtomPos = dWords.Begin()->m_iAtomPos;

				uint64_t uHead = sphFNV64cont ( dWords[0].m_sWord.cstr(), SPH_FNV64_SEED );
				uint64_t uTail = sphFNV64cont ( dWords [ dWords.GetLength() - 1 ].m_sWord.cstr(), SPH_FNV64_SEED );
				uHead = sphFNV64 ( g_sPhraseDelimiter, sizeof(g_sPhraseDelimiter), uHead );
				uHead = sphFNV64cont ( dWords[1].m_sWord.cstr(), uHead );

				uTail = sphFNV64 ( g_sPhraseDelimiter, sizeof(g_sPhraseDelimiter), uTail );
				uTail = sphFNV64cont ( dWords[dWords.GetLength()-2].m_sWord.cstr(), uTail );

				const int iHeadLen = sphBigramAddNode ( dNodes[iPhrase], uHead, tBigramHead );
				const int iTailLen = sphBigramAddNode ( dNodes[iPhrase], uTail, tBigramTail );
				bHasCommonPhrases |= ( iHeadLen>1 || iTailLen>1 );
			}

			if ( !bHasCommonPhrases )
				continue;

			// 2nd step find minimum for each phrase group
			CSphVector<CommonInfo_t> dCommon;
			for ( auto& [_, dBigrams] : tBigramHead )
			{
				// only phrases that share same words at head
				if ( dBigrams.GetLength()<2 )
					continue;

				CommonInfo_t & tElem = dCommon.Add();
				tElem.m_pPhrases = &dBigrams;
				tElem.m_iCommonLen = 2;
				tElem.m_bHead = true;
				tElem.m_bHasBetter = false;
			}
			for ( auto& [_, dBigrams] : tBigramTail )
			{
				// only phrases that share same words at tail
				if ( dBigrams.GetLength()<2 )
					continue;

				CommonInfo_t & tElem = dCommon.Add();
				tElem.m_pPhrases = &dBigrams;
				tElem.m_iCommonLen = 2;
				tElem.m_bHead = false;
				tElem.m_bHasBetter = false;
			}

			// for each set of phrases with common words at the head or tail
			// each word that is same at all phrases makes common length longer
			for ( CommonInfo_t & tCommon : dCommon )
			{
				const bool bHead = tCommon.m_bHead;
				const CSphVector<XQNode_t *> & dPhrases = *tCommon.m_pPhrases;
				// start from third word ( two words at each phrase already matched at bigram hashing )
				for ( int iCount=3; ; ++iCount )
				{
					// is the shortest phrase words over
					if ( iCount>=dPhrases.First()->dWords().GetLength() )
						break;

					const int iWordRef = ( bHead ? iCount-1 : dPhrases.First()->dWords().GetLength() - iCount );
					const uint64_t uHash = sphFNV64 ( dPhrases.First()->dWord(iWordRef).m_sWord.cstr() );

					bool bPhrasesMatch = false;
					bool bSomePhraseOver = false;
					for ( int iPhrase=1; iPhrase<dPhrases.GetLength(); ++iPhrase )
					{
						bSomePhraseOver = iCount>=dPhrases[iPhrase]->dWords().GetLength();
						if ( bSomePhraseOver )
							break;

						const int iWord = ( bHead ? iCount-1 : dPhrases[iPhrase]->dWords().GetLength() - iCount );
						bPhrasesMatch = uHash==sphFNV64 ( dPhrases[iPhrase]->dWord(iWord).m_sWord.cstr() );
						if ( !bPhrasesMatch )
							break;
					}

					// no need to check further in case the shortest phrase has no more words or sequence over
					if ( bSomePhraseOver || !bPhrasesMatch )
						break;

					tCommon.m_iCommonLen = iCount;
				}
			}

			// mark all dupes (that has smaller common length) as deleted
			if ( dCommon.GetLength()>=2 )
			{
				CSphVector<Node2Common_t> dDups ( dCommon.GetLength()*2 );
				dDups.Resize ( 0 );
				for ( CommonInfo_t & tCommon : dCommon )
				{
					for ( XQNode_t * pPhrase : *tCommon.m_pPhrases )
					{
						Node2Common_t & tDup = dDups.Add();
						tDup.m_pNode = pPhrase;
						tDup.m_pCommon = &tCommon;
					}
				}
				dDups.Sort ( CommonDupElimination_fn() );
				for ( int i=0; i<dDups.GetLength()-1; ++i )
				{
					auto & [pCurNode, pCurCommon] = dDups[i];
					auto & [pNextNode, pNextCommon] = dDups[i+1];
					if ( pCurNode==pNextNode )
					{
						if ( pCurCommon->m_iCommonLen<=pNextCommon->m_iCommonLen )
							pCurCommon->m_bHasBetter = true;
						else
							pNextCommon->m_bHasBetter = true;
					}
				}
			}

			for ( const CommonInfo_t & tElem : dCommon )
			{
				if ( tElem.m_bHasBetter )
					continue;

				tElem.m_pPhrases->Sort ( XQNodeAtomPos_fn() );
				MakeTransformCommonPhrase ( *tElem.m_pPhrases, tElem.m_iCommonLen, tElem.m_bHead );
				iActiveDeep = hSimGroup.iDeep;
				break;
			}
		}

	return iActiveDeep;
}


void CSphTransformation::MakeTransformCommonPhrase ( const CSphVector<XQNode_t *> & dCommonNodes, int iCommonLen, bool bHeadIsCommon )
{
	auto * pCommonPhrase = new XQNode_t ( XQLimitSpec_t() );
	pCommonPhrase->SetOp ( SPH_QUERY_PHRASE );

	XQNode_t * pGrandOr = dCommonNodes[0]->m_pParent;
	if ( bHeadIsCommon )
	{
		// fill up common suffix
		const XQNode_t * pPhrase = dCommonNodes[0];
		pCommonPhrase->m_iAtomPos = pPhrase->dWord(0).m_iAtomPos;
		pCommonPhrase->WithWords ( [pPhrase,iCommonLen] ( auto& dWords ) {
		for ( int i=0; i<iCommonLen; ++i )
			dWords.Add ( pPhrase->dWord(i) );
		});
	} else
	{
		const XQNode_t * pPhrase = dCommonNodes[0];
		// set the farthest atom position
		int iAtomPos = pPhrase->dWord ( pPhrase->dWords().GetLength() - iCommonLen ).m_iAtomPos;
		for ( const XQNode_t * pCur : dCommonNodes )
		{
			int iCurAtomPos = pCur->dWord(pCur->dWords().GetLength() - iCommonLen).m_iAtomPos;
			if ( iAtomPos < iCurAtomPos )
			{
				pPhrase = pCur;
				iAtomPos = iCurAtomPos;
			}
		}
		pCommonPhrase->m_iAtomPos = iAtomPos;

		pCommonPhrase->WithWords ( [pPhrase,iCommonLen] ( auto& dWords ) {
			for ( int i=pPhrase->dWords().GetLength() - iCommonLen; i<pPhrase->dWords().GetLength(); ++i )
				dWords.Add ( pPhrase->dWord(i) );
		});
	}


	std::optional<XQNode_t*> pMaybeNewOr;

	for ( XQNode_t * pPhrase : dCommonNodes )
	{
		// remove phrase from parent and eliminate in case of common phrase duplication
		Verify ( pGrandOr->RemoveChild ( pPhrase ) );
		if ( pPhrase->dWords().GetLength()==iCommonLen )
		{
			SafeDelete ( pPhrase );
			continue;
		}

		if (!pMaybeNewOr)
		{
			pMaybeNewOr.emplace ( new XQNode_t ( XQLimitSpec_t() ) );
			(*pMaybeNewOr)->SetOp ( SPH_QUERY_OR );
		}

		assert ( pMaybeNewOr.has_value() );
		auto* pNewOr = *pMaybeNewOr;

		// move phrase to new OR
		pNewOr->AddNewChild ( pPhrase );

		// shift down words and enumerate words atom positions
		pPhrase->WithWords ( [pCommonPhrase,bHeadIsCommon,iCommonLen] ( auto& dPhraseWords )
		{
			if ( bHeadIsCommon )
			{
				const int iEndCommonAtom = pCommonPhrase->dWords().Last().m_iAtomPos+1;
				for ( int j=iCommonLen; j<dPhraseWords.GetLength(); ++j )
				{
					int iTo = j-iCommonLen;
					dPhraseWords[iTo] = dPhraseWords[j];
					dPhraseWords[iTo].m_iAtomPos = iEndCommonAtom + iTo;
				}
			}
			dPhraseWords.Resize ( dPhraseWords.GetLength() - iCommonLen );
			if ( !bHeadIsCommon )
			{
				const int iStartAtom = pCommonPhrase->dWord(0).m_iAtomPos - dPhraseWords.GetLength();
				ARRAY_FOREACH ( j, dPhraseWords )
					dPhraseWords[j].m_iAtomPos = iStartAtom + j;
			}
		});

		if ( pPhrase->dWords().GetLength()==1 )
			pPhrase->SetOp ( SPH_QUERY_AND );
	}

	if ( !pMaybeNewOr )
	{
		// common phrases with same words elimination
		AddOrReplaceNode ( pGrandOr, pCommonPhrase );
		return;
	}

	auto * pNewOr = *pMaybeNewOr;

	// parent phrase need valid atom position of children
	pNewOr->m_iAtomPos = pNewOr->dChild(0)->dWord(0).m_iAtomPos;

	auto * pNewPhrase = new XQNode_t ( XQLimitSpec_t() );
	if ( bHeadIsCommon )
		pNewPhrase->SetOp ( SPH_QUERY_PHRASE, pCommonPhrase, pNewOr );
	else
		pNewPhrase->SetOp ( SPH_QUERY_PHRASE, pNewOr, pCommonPhrase );

	AddOrReplaceNode ( pGrandOr, pNewPhrase );
}
