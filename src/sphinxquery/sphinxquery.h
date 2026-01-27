//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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

#include "sphinxstd.h"
#include "sphinx.h"
#include "xqdebug.h"

//////////////////////////////////////////////////////////////////////////////

/// extended query word with attached position within atom
struct XQKeyword_t
{
	CSphString			m_sWord;
	int					m_iAtomPos = -1;
	int					m_iSkippedBefore = 0;	///< positions skipped before this token (because of blended chars)
	mutable bool		m_bFieldStart = false;	///< must occur at very start
	mutable bool		m_bFieldEnd = false;	///< must occur at very end
	float				m_fBoost = 1.0f;		///< keyword IDF will be multiplied by this
	mutable bool		m_bExpanded = false;	///< added by prefix expansion
	mutable bool		m_bExcluded = false;	///< excluded by query (rval to operator NOT)
	mutable bool		m_bMorphed = false;		///< morphology processing (wordforms, stemming etc) already done
	mutable void *		m_pPayload = nullptr;
	mutable bool		m_bRegex = false;

	XQKeyword_t() = default;
	XQKeyword_t ( const char * sWord, int iPos )
		: m_sWord ( sWord )
		, m_iAtomPos ( iPos )
	{}
};


/// extended query operator
enum XQOperator_e : BYTE
{
	SPH_QUERY_AND,
	SPH_QUERY_OR,
	SPH_QUERY_MAYBE,
	SPH_QUERY_NOT,
	SPH_QUERY_ANDNOT,
	SPH_QUERY_BEFORE,
	SPH_QUERY_PHRASE,
	SPH_QUERY_PROXIMITY,
	SPH_QUERY_QUORUM,
	SPH_QUERY_NEAR,
	SPH_QUERY_NOTNEAR,
	SPH_QUERY_SENTENCE,
	SPH_QUERY_PARAGRAPH,
	SPH_QUERY_NULL,
	SPH_QUERY_SCAN,

	SPH_QUERY_TOTAL
};

constexpr const char* XQOperatorNameSz (XQOperator_e eOperator)
{
	constexpr std::array <const char*, SPH_QUERY_TOTAL> dNames { "AND", "OR", "MAYBE", "NOT", "ANDNOT", "BEFORE", "PHRASE",
	"PROXIMITY", "QUORUM", "NEAR", "NOTNEAR", "SENTENCE", "PARAGRAPH", "NULL", "SCAN" };
	return dNames[eOperator];
}


// the limit of field or zone or zonespan
struct XQLimitSpec_t
{
	bool					m_bFieldSpec = false;	///< whether field spec was already explicitly set
	FieldMask_t				m_dFieldMask;			///< fields mask (spec part)
	int						m_iFieldMaxPos = 0;		///< max position within field (spec part)
	CSphVector<int>			m_dZones;				///< zone indexes in per-query zones list
	bool					m_bZoneSpan = false;	///< if we need to hits within only one span

public:
	XQLimitSpec_t ()
	{
		m_dFieldMask.SetAll ();
	}

	void Reset ()
	{
		m_bFieldSpec = false;
		m_iFieldMaxPos = 0;
		m_bZoneSpan = false;
		m_dFieldMask.SetAll();
		m_dZones.Reset();
	}

	bool IsEmpty() const
	{
		return !m_bFieldSpec && m_iFieldMaxPos==0 && !m_bZoneSpan && m_dZones.GetLength()==0;
	}

	XQLimitSpec_t ( const XQLimitSpec_t& dLimit )
	{
		if ( dLimit.m_bFieldSpec )
			SetFieldSpec ( dLimit.m_dFieldMask, dLimit.m_iFieldMaxPos );
		else
			m_dFieldMask.SetAll ();

		if ( !dLimit.m_dZones.IsEmpty ())
			SetZoneSpec ( dLimit.m_dZones, dLimit.m_bZoneSpan );
	}

	XQLimitSpec_t ( XQLimitSpec_t&& rhs ) noexcept
		: XQLimitSpec_t ()
	{
		Swap ( rhs );
	}

	void Swap ( XQLimitSpec_t& rhs ) noexcept
	{
		::Swap ( m_bFieldSpec, rhs.m_bFieldSpec );
		::Swap ( m_dFieldMask, rhs.m_dFieldMask );
		::Swap ( m_iFieldMaxPos, rhs.m_iFieldMaxPos );
		::Swap ( m_dZones, rhs.m_dZones );
		::Swap ( m_bZoneSpan, rhs.m_bZoneSpan );
	}

	XQLimitSpec_t & operator = ( XQLimitSpec_t dLimit )
	{
		Swap ( dLimit );
		return *this;
	}


	void SetZoneSpec ( const CSphVector<int> & dZones, bool bZoneSpan );
	void SetFieldSpec ( const FieldMask_t& uMask, int iMaxPos );
	uint64_t Hash () const noexcept;
};

/// extended query node
/// plain nodes are just an atom
/// non-plain nodes are a logical function over children nodes
struct XQNode_t : public ISphNoncopyable
{
	XQNode_t *				m_pParent = nullptr;		///< my parent node (NULL for root ones)

private:
	XQOperator_e			m_eOp { SPH_QUERY_AND };	///< operation over children
	mutable int				m_iOrder = 0;
	mutable int				m_iCounter = 0;

	CSphVector<XQKeyword_t>		m_dWords;		///< query words (plain node). Private to keep hashes valid
	CSphVector<XQNode_t*>	m_dChildren;		///< non-plain node children

	mutable uint64_t		m_iMagicHash = 0;
#if XQDEBUG
public: // debug dump prints the hash
#endif
	mutable uint64_t		m_iFuzzyHash = 0;

public:
	XQLimitSpec_t			m_dSpec;			///< specification by field, zone(s), etc.

	int						m_iOpArg = 0;		///< operator argument (proximity distance, quorum count)
	int						m_iAtomPos = -1;	///< atom position override (currently only used within expanded nodes)
	int						m_iUser = -1;
	bool					m_bVirtuallyPlain = false;	///< "virtually plain" flag (currently only used by expanded nodes)
	bool					m_bNotWeighted = false;	///< this our expanded but empty word's node
	bool					m_bPercentOp = false;

	const CSphVector<XQKeyword_t>& dWords() const noexcept { return m_dWords; };
	const XQKeyword_t& dWord(int64_t i) const noexcept { return m_dWords[i]; };

	void WithWord ( int64_t iWord, std::function<void (XQKeyword_t&)> fnAction ) noexcept
	{
		fnAction(m_dWords[iWord]);
		Rehash();
	}

	void WithWords ( std::function<void (CSphVector<XQKeyword_t>&)> fnAction ) noexcept
	{
		fnAction(m_dWords);
		Rehash();
	}

	void ResetWords() noexcept
	{
		m_dWords.Reset();
		Rehash();
	}

	void AddDirtyWord (XQKeyword_t dWord); // add word without invalidating hashes. OK for fresh (new-born) nodes.

	const CSphVector<XQNode_t*>& dChildren() const noexcept { return m_dChildren; };
	const XQNode_t* dChild(int64_t i) const noexcept { return m_dChildren[i]; };

	// manipulate with children, caring hash
	void WithChild ( int64_t iChild, std::function<void (XQNode_t*)> fnAction ) noexcept
	{
		fnAction(m_dChildren[iChild]);
		Rehash();
	}

	void WithChildren (std::function<void (CSphVector<XQNode_t*>&)> fnAction ) noexcept
	{
		fnAction(m_dChildren);
		Rehash();
	}

	void ResetChildren (bool bWithRehash=false) noexcept
	{
		m_dChildren.Reset();
		if ( bWithRehash )
			Rehash();
	}

	void AddNewChild ( XQNode_t* pNode ) noexcept
	{
		pNode->m_pParent = this;
		m_dChildren.Add ( pNode);
		Rehash();
	}

	bool RemoveChild ( XQNode_t* pNode ) noexcept
	{
		Rehash();
		return m_dChildren.RemoveValue ( pNode );
	}

	/// ctor
	explicit XQNode_t ( const XQLimitSpec_t & dSpec );

	/// dtor
	~XQNode_t ();

	/// check if i'm empty
	bool IsEmpty () const
	{
		assert ( m_dWords.IsEmpty() || m_dChildren.IsEmpty() );
		return m_dWords.IsEmpty() && m_dChildren.IsEmpty();
	}

	/// setup field limits
	void SetFieldSpec ( const FieldMask_t& uMask, int iMaxPos );

	/// setup zone limits
	void SetZoneSpec ( const CSphVector<int> & dZones, bool bZoneSpan=false );

	/// copy field/zone limits from another node
	void CopySpecs ( const XQNode_t * pSpecs );

	/// unconditionally clear field mask
	void ClearFieldMask ();

	/// get my operator
	XQOperator_e GetOp () const noexcept
	{
		return m_eOp;
	}

	/// get my cache order
	DWORD GetOrder () const
	{
		return m_iOrder;
	}

	/// get my cache counter
	int GetCount () const
	{
		return m_iCounter;
	}

	/// setup common nodes for caching
	void TagAsCommon ( int iOrder, int iCounter ) const noexcept
	{
		m_iCounter = iCounter;
		m_iOrder = iOrder;
	}

	/// hash me
	uint64_t GetHash () const noexcept;

	/// fuzzy hash ( a hash value is equal for proximity and phrase nodes
	/// with similar keywords )
	uint64_t GetFuzzyHash () const noexcept;

	void Rehash () const noexcept
	{
		if ( !m_iFuzzyHash && !m_iMagicHash )
			return;

		m_iMagicHash = m_iFuzzyHash = 0;
		if ( m_pParent )
			m_pParent->Rehash();
	}

	/// setup new operator and args
	void SetOp ( XQOperator_e eOp, XQNode_t * pArg1, XQNode_t * pArg2=nullptr );

	/// setup new operator and args
	void SetOp ( XQOperator_e eOp, CSphVector<XQNode_t*> & dArgs )
	{
		SetOp (eOp);
		assert ( m_dChildren.IsEmpty() && "Ensure your node has no children. You need to explicitly reset them, or delete - to avoid memleak here" );
		m_dChildren.SwapData(dArgs);
		for ( auto* pChild : m_dChildren )
			pChild->m_pParent = this;
	}

	/// setup new operator (careful parser/transform use only)
	void SetOp ( XQOperator_e eOp )
	{
		m_eOp = eOp;
		Rehash();
	}

	/// fixup atom positions in case of proximity queries and blended chars
	/// we need to handle tokens with blended characters as simple tokens in this case
	/// and we need to remove possible gaps in atom positions
	int FixupAtomPos() const noexcept;

	/// return node like current
	XQNode_t * Clone () const;

#ifndef NDEBUG
	/// consistency check
	void Check ( bool bRoot )
	{
		assert ( bRoot || !IsEmpty() || m_eOp==SPH_QUERY_SCAN ); // empty leaves must be removed from the final tree; empty root is allowed
		assert (!( m_dWords.GetLength() && m_eOp!=SPH_QUERY_AND && m_eOp!=SPH_QUERY_OR && m_eOp!=SPH_QUERY_PHRASE
			&& m_eOp!=SPH_QUERY_PROXIMITY && m_eOp!=SPH_QUERY_QUORUM )); // words are only allowed in these node types
		assert ( ( m_dWords.GetLength()==1 && ( m_eOp==SPH_QUERY_AND || m_eOp==SPH_QUERY_OR ) ) ||
			m_dWords.GetLength()!=1 ); // 1-word leaves must be of AND | OR

		ARRAY_FOREACH ( i, m_dChildren )
		{
			assert ( m_dChildren[i]->m_pParent==this );
			m_dChildren[i]->Check ( false );
		}
	}
#else
	void Check ( bool ) {}
#endif
};


/// extended query
struct XQQuery_t : ISphNoncopyable
{
	CSphString				m_sParseError;
	CSphString				m_sParseWarning;

	StrVec_t				m_dZones;
	XQNode_t *				m_pRoot = nullptr;
	bool					m_bNeedSZlist = false;
	bool					m_bSingleWord = false;
	bool					m_bEmpty = false;
	// was node full-text (even folded into empty)
	bool					m_bWasFullText = false;
	bool					m_bNeedPhraseTransform = false;

	/// dtor
	~XQQuery_t ()
	{
		SafeDelete ( m_pRoot );
	}
};


struct NodeEstimate_t
{
	float	m_fCost = 0.0f;
	int64_t	m_iDocs = 0;
	int64_t	m_iTerms = 0;

	NodeEstimate_t & operator+= ( const NodeEstimate_t & tRhs );
};


class QueryParser_i
{
public:
	virtual			~QueryParser_i() = default;

	virtual bool	IsFullscan ( const CSphQuery & tQuery ) const { return tQuery.m_sQuery.IsEmpty(); };
	virtual bool	IsFullscan ( const XQQuery_t & tQuery ) const { return !tQuery.m_bWasFullText; };
	virtual bool	ParseQuery ( XQQuery_t & tParsed, const char * sQuery, const CSphQuery * pQuery, TokenizerRefPtr_c pQueryTokenizer, TokenizerRefPtr_c pQueryTokenizerJson, const CSphSchema * pSchema, const DictRefPtr_c& pDict, const CSphIndexSettings & tSettings, const CSphBitvec * pMorphFields ) const = 0;
	virtual QueryParser_i * Clone() const = 0;
};

//////////////////////////////////////////////////////////////////////////////

// perform boolean optimization on tree
void	sphOptimizeBoolean ( XQNode_t ** pXQ, const ISphKeywordsStat * pKeywords );

/// analyze vector of trees and tag common parts of them (to cache them later)
int		sphMarkCommonSubtrees ( int iXQ, const XQQuery_t * pXQ );
XQQuery_t * CloneXQQuery ( const XQQuery_t & tQuery );
XQNode_t * CloneKeyword ( const XQNode_t * pNode );

/// whatever to allow alone operator NOT at query
void	AllowOnlyNot ( bool bAllowed );
bool	IsAllowOnlyNot();

/// global setting for boolean simplification
void	SetBooleanSimplify ( bool bSimplify );
bool	GetBooleanSimplify ( const CSphQuery & tQuery );
CSphString sphReconstructNode ( const XQNode_t * pNode, const CSphSchema * pSchema = nullptr, StrVec_t * pZones = nullptr );
inline int GetExpansionLimit ( int iQueryLimit, int iIndexLimit  )
{
	return ( iQueryLimit!=DEFAULT_QUERY_EXPANSION_LIMIT ? iQueryLimit : iIndexLimit );
}

bool TransformPhraseBased ( XQNode_t ** ppNode, CSphString & sError, CSphString & sWarning );
void SetExpansionPhraseLimit ( int iMaxVariants, bool bExpansionPhraseWarning );