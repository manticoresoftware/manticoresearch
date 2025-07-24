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

#pragma once

#include "sphinxquery.h"
#include "std/generics.h"
#include "xqdebug.h"

using BigramHash_t = CSphOrderedHash<CSphVector<XQNode_t *>, uint64_t, IdentityHash_fn, 128>;

class CSphTransformation : public ISphNoncopyable
{
public:
	CSphTransformation ( XQNode_t ** ppRoot, const ISphKeywordsStat * pKeywords );

	void Transform ();

private:
	using HashSimilar_t = CSphOrderedHash<CSphVector<XQNode_t *>, uintptr_t, IdentityHash_fn, 32>;
	struct DeepHashSimilar_t {HashSimilar_t tHash; int iDeep;};
	CSphOrderedHash<DeepHashSimilar_t, uint64_t, IdentityHash_fn, 256> m_hSimilar;
	CSphVector<XQNode_t *> m_dRelatedNodes;
	const ISphKeywordsStat * m_pKeywords;
	XQNode_t ** m_ppRoot;

	using Checker_fn = bool ( * ) ( const XQNode_t * );

private:
	void DumpSimilar () const noexcept;

	void SetCosts ( XQNode_t * pNode, const CSphVector<XQNode_t *> & dNodes ) const noexcept;

	static int GetWeakestIndex ( const CSphVector<XQNode_t *> & dNodes );
	static int GetWeakestChildIndex ( const CSphVector<XQNode_t *> & dNodes );

	template<typename Group, typename SubGroup>
	void TreeCollectInfo ( XQNode_t * pNode, Checker_fn pfnChecker, int iDeep );

	template<typename Group, typename SubGroup>
	bool CollectInfo ( XQNode_t * pNode, Checker_fn pfnChecker );

	template<typename Excluder, typename Parenter>
	bool CollectRelatedNodes ( const CSphVector<XQNode_t *> & dSimilarNodes );

	// ((A !N) | (B !N)) -> ((A|B) !N)
	static bool CheckCommonNot ( const XQNode_t * pNode ) noexcept;
	bool TransformCommonNot () noexcept;
	void MakeTransformCommonNot ( const CSphVector<XQNode_t *> & dSimilarNodes );

	// ((A !(N AA)) | (B !(N BB))) -> (((A|B) !N) | (A !AA) | (B !BB)) [ if cost(N) > cost(A) + cost(B) ]
	static bool CheckCommonCompoundNot ( const XQNode_t * pNode ) noexcept;
	bool TransformCommonCompoundNot () noexcept;
	void MakeTransformCommonCompoundNot ( const CSphVector<XQNode_t *> & dSimilarNodes ) const;

	// ((A (AA | X)) | (B (BB | X))) -> ((A AA) | (B BB) | ((A|B) X)) [ if cost(X) > cost(A) + cost(B) ]
	static bool CheckCommonSubTerm ( const XQNode_t * pNode ) noexcept;
	bool TransformCommonSubTerm () noexcept;
	void MakeTransformCommonSubTerm ( const CSphVector<XQNode_t *> & dX ) const;

	// (A | "A B"~N) -> A ; ("A B" | "A B C") -> "A B" ; ("A B"~N | "A B C"~N) -> ("A B"~N)
	static bool CheckCommonKeywords ( const XQNode_t * pNode ) noexcept;
	bool TransformCommonKeywords () const noexcept;

	// ("X A B" | "Y A B") -> (("X|Y") "A B")
	// ("A B X" | "A B Y") -> (("X|Y") "A B")
	static bool CheckCommonPhrase ( const XQNode_t * pNode ) noexcept;
	bool TransformCommonPhrase () const noexcept;
	static void MakeTransformCommonPhrase ( const CSphVector<XQNode_t *> & dCommonNodes, int iCommonLen, bool bHeadIsCommon );

	// ((A !X) | (A !Y) | (A !Z)) -> (A !(X Y Z))
	static bool CheckCommonAndNotFactor ( const XQNode_t * pNode ) noexcept;
	bool TransformCommonAndNotFactor () noexcept;
	void MakeTransformCommonAndNotFactor ( const CSphVector<XQNode_t *> & dSimilarNodes ) noexcept;

	// ((A !(N | N1)) | (B !(N | N2))) -> (( (A !N1) | (B !N2) ) !N)
	static bool CheckCommonOrNot ( const XQNode_t * pNode ) noexcept;
	bool TransformCommonOrNot () noexcept;
	void MakeTransformCommonOrNot ( const CSphVector<XQNode_t *> & dSimilarNodes ) const;

	// The main goal of transformations below is tree clarification and
	// further applying of standard transformations above.

	// "hung" operand ( AND(OR) node with only 1 child ) appears after an internal transformation
	static bool CheckHungOperand ( const XQNode_t * pNode ) noexcept;
	bool TransformHungOperand () const noexcept;

	// ((A | B) | C) -> ( A | B | C )
	// ((A B) C) -> ( A B C )
	static bool CheckExcessBrackets ( const XQNode_t * pNode ) noexcept;
	bool TransformExcessBrackets () const noexcept;

	// ((A !N1) !N2) -> (A !(N1 | N2))
	static bool CheckExcessAndNot ( const XQNode_t * pNode ) noexcept;
	bool TransformExcessAndNot () const;

	// "notnot" operand ( -- node in 'and-not' ) appears after an internal transformation
	// (A !!B) -> (A B)
	static bool CheckAndNotNotOperand ( const XQNode_t * pNode ) noexcept;
	bool TransformAndNotNotOperand () const noexcept;

private:
	static constexpr uint64_t CONST_GROUP_FACTOR = 0;

	struct NullNode
	{
		static uint64_t By ( XQNode_t * ) noexcept { return CONST_GROUP_FACTOR; }
		static const XQNode_t * From ( const XQNode_t * ) noexcept { return nullptr; }
		static XQNode_t * From ( XQNode_t * p ) noexcept { return nullptr; }
	};


	template<BYTE INHERITANCE>
	struct Grand
	{
		static uint64_t By ( const XQNode_t * p ) noexcept { return From ( p )->GetFuzzyHash(); }
		static constexpr const XQNode_t * From ( const XQNode_t * p ) noexcept { return From ( const_cast<XQNode_t *> (p) ); }


		static constexpr XQNode_t * From ( XQNode_t * p ) noexcept
		{
			if constexpr ( INHERITANCE )
			{
				assert ( p );
				return Grand<INHERITANCE - 1>::From ( p )->m_pParent;
			}
			return p;
		}


		static constexpr bool Valid ( const XQNode_t * p ) noexcept
		{
			if constexpr ( !INHERITANCE )
				return p;
			else
				return Grand<INHERITANCE - 1>::Valid ( p ) && From ( p );
		}
	};

	using CurrentNode = Grand<0>;
	using ParentNode = Grand<1>;
	using GrandNode = Grand<2>;
	using Grand2Node = Grand<3>;
	using Grand3Node = Grand<4>;

	static bool ReplaceNode ( XQNode_t * pNewNode, XQNode_t * pOldNode ) noexcept;
	static void AddOrReplaceNode ( XQNode_t * pParent, XQNode_t * pChild ) noexcept;
};


template < typename Excluder, typename Parenter >
bool CSphTransformation::CollectRelatedNodes ( const CSphVector<XQNode_t *> & dSimilarNodes )
{
	m_dRelatedNodes.Resize ( 0 );
	for ( XQNode_t * pSimilar : dSimilarNodes )
	{
		// Eval node that should be excluded
		const XQNode_t * pExclude = Excluder::From ( pSimilar );

		// Eval node that points to related nodes
		const XQNode_t * pParent = Parenter::From ( pSimilar );

		assert ( &pParent->dChildren()!=&m_dRelatedNodes );
		for ( XQNode_t * pChild : pParent->dChildren() )
		{
			if ( pChild==pExclude )
				continue;
			m_dRelatedNodes.Add ( pChild );
//			Dump ( pParent, "RelatedNodes", false );
		}
	}
	return m_dRelatedNodes.GetLength()>1;
}

bool HasSameParent ( const VecTraits_T<XQNode_t *> & dSimilarNodes ) noexcept;
