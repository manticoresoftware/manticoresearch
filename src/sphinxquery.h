//
// $Id$
//

//
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxquery_
#define _sphinxquery_

#include "sphinx.h"

//////////////////////////////////////////////////////////////////////////////

/// extended query word with attached position within atom
struct XQKeyword_t
{
	CSphString			m_sWord;
	int					m_iAtomPos;
	int					m_iSkippedBefore; ///< positions skipped before this token (because of blended chars)
	bool				m_bFieldStart;	///< must occur at very start
	bool				m_bFieldEnd;	///< must occur at very end
	float				m_fBoost;		///< keyword IDF will be multiplied by this
	bool				m_bExpanded;	///< added by prefix expansion
	bool				m_bExcluded;	///< excluded by query (rval to operator NOT)
	bool				m_bMorphed;		///< morphology processing (wordforms, stemming etc) already done
	void *				m_pPayload;

	XQKeyword_t ()
		: m_iAtomPos ( -1 )
		, m_iSkippedBefore ( 0 )
		, m_bFieldStart ( false )
		, m_bFieldEnd ( false )
		, m_fBoost ( 1.0f )
		, m_bExpanded ( false )
		, m_bExcluded ( false )
		, m_bMorphed ( false )
		, m_pPayload ( NULL )
	{}

	XQKeyword_t ( const char * sWord, int iPos )
		: m_sWord ( sWord )
		, m_iAtomPos ( iPos )
		, m_iSkippedBefore ( 0 )
		, m_bFieldStart ( false )
		, m_bFieldEnd ( false )
		, m_fBoost ( 1.0f )
		, m_bExpanded ( false )
		, m_bExcluded ( false )
		, m_bMorphed ( false )
		, m_pPayload ( NULL )
	{}
};


/// extended query operator
enum XQOperator_e
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
	SPH_QUERY_SENTENCE,
	SPH_QUERY_PARAGRAPH,
	SPH_QUERY_NULL,

	SPH_QUERY_TOTAL
};

// the limit of field or zone or zonespan
struct XQLimitSpec_t
{
	bool					m_bFieldSpec;	///< whether field spec was already explicitly set
	FieldMask_t			m_dFieldMask;	///< fields mask (spec part)
	int						m_iFieldMaxPos;	///< max position within field (spec part)
	CSphVector<int>			m_dZones;		///< zone indexes in per-query zones list
	bool					m_bZoneSpan;	///< if we need to hits within only one span

public:
	XQLimitSpec_t ()
	{
		Reset();
	}

	inline void Reset ()
	{
		m_bFieldSpec = false;
		m_iFieldMaxPos = 0;
		m_bZoneSpan = false;
		m_dFieldMask.SetAll();
		m_dZones.Reset();
	}

	bool IsEmpty() const
	{
		return m_bFieldSpec==false && m_iFieldMaxPos==0 && m_bZoneSpan==false && m_dZones.GetLength()==0;
	}

	XQLimitSpec_t ( const XQLimitSpec_t& dLimit )
	{
		if ( this==&dLimit )
			return;
		Reset();
		*this = dLimit;
	}

	XQLimitSpec_t & operator = ( const XQLimitSpec_t& dLimit )
	{
		if ( this==&dLimit )
			return *this;

		if ( dLimit.m_bFieldSpec )
			SetFieldSpec ( dLimit.m_dFieldMask, dLimit.m_iFieldMaxPos );

		if ( dLimit.m_dZones.GetLength() )
			SetZoneSpec ( dLimit.m_dZones, dLimit.m_bZoneSpan );

		return *this;
	}
public:
	void SetZoneSpec ( const CSphVector<int> & dZones, bool bZoneSpan );
	void SetFieldSpec ( const FieldMask_t& uMask, int iMaxPos );
};

/// extended query node
/// plain nodes are just an atom
/// non-plain nodes are a logical function over children nodes
struct XQNode_t : public ISphNoncopyable
{
	XQNode_t *				m_pParent;		///< my parent node (NULL for root ones)

private:
	XQOperator_e			m_eOp;			///< operation over childen
	int						m_iOrder;
	int						m_iCounter;

private:
	mutable uint64_t		m_iMagicHash;
	mutable uint64_t		m_iFuzzyHash;

public:
	CSphVector<XQNode_t*>	m_dChildren;	///< non-plain node children
	XQLimitSpec_t			m_dSpec;		///< specification by field, zone(s), etc.

	CSphVector<XQKeyword_t>	m_dWords;		///< query words (plain node)
	int						m_iOpArg;		///< operator argument (proximity distance, quorum count)
	int						m_iAtomPos;		///< atom position override (currently only used within expanded nodes)
	int						m_iUser;
	bool					m_bVirtuallyPlain;	///< "virtually plain" flag (currently only used by expanded nodes)
	bool					m_bNotWeighted;	///< this our expanded but empty word's node
	bool					m_bPercentOp;

public:
	/// ctor
	explicit XQNode_t ( const XQLimitSpec_t & dSpec );

	/// dtor
	~XQNode_t ();

	/// check if i'm empty
	bool IsEmpty () const
	{
		assert ( m_dWords.GetLength()==0 || m_dChildren.GetLength()==0 );
		return m_dWords.GetLength()==0 && m_dChildren.GetLength()==0;
	}

	/// setup field limits
	void SetFieldSpec ( const FieldMask_t& uMask, int iMaxPos );

	/// setup zone limits
	void SetZoneSpec ( const CSphVector<int> & dZones, bool bZoneSpan=false );

	/// copy field/zone limits from another node
	void CopySpecs ( const XQNode_t * pSpecs );

	/// unconditionally clear field mask
	void ClearFieldMask ();

public:
	/// get my operator
	XQOperator_e GetOp () const
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
	void TagAsCommon ( int iOrder, int iCounter )
	{
		m_iCounter = iCounter;
		m_iOrder = iOrder;
	}

	/// hash me
	uint64_t GetHash () const;

	/// fuzzy hash ( a hash value is equal for proximity and phrase nodes
	/// with similar keywords )
	uint64_t GetFuzzyHash () const;

	/// setup new operator and args
	void SetOp ( XQOperator_e eOp, XQNode_t * pArg1, XQNode_t * pArg2=NULL );

	/// setup new operator and args
	void SetOp ( XQOperator_e eOp, CSphVector<XQNode_t*> & dArgs )
	{
		m_eOp = eOp;
		m_dChildren.SwapData(dArgs);
		ARRAY_FOREACH ( i, m_dChildren )
			m_dChildren[i]->m_pParent = this;
	}

	/// setup new operator (careful parser/transform use only)
	void SetOp ( XQOperator_e eOp )
	{
		m_eOp = eOp;
	}

	/// fixup atom positions in case of proximity queries and blended chars
	/// we need to handle tokens with blended characters as simple tokens in this case
	/// and we need to remove possible gaps in atom positions
	int FixupAtomPos();

	/// return node like current
	inline XQNode_t * Clone ();

	/// force resetting magic hash value ( that changed after transformation )
	inline bool ResetHash ();

#ifndef NDEBUG
	/// consistency check
	void Check ( bool bRoot )
	{
		assert ( bRoot || !IsEmpty() ); // empty leaves must be removed from the final tree; empty root is allowed
		assert (!( m_dWords.GetLength() && m_eOp!=SPH_QUERY_AND && m_eOp!=SPH_QUERY_OR && m_eOp!=SPH_QUERY_PHRASE
			&& m_eOp!=SPH_QUERY_PROXIMITY && m_eOp!=SPH_QUERY_QUORUM )); // words are only allowed in these node types
		assert ( ( m_dWords.GetLength()==1 && ( m_eOp==SPH_QUERY_AND || m_eOp==SPH_QUERY_OR ) ) ||
			m_dWords.GetLength()!=1 ); // 1-word leaves must be of AND | OR types

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
struct XQQuery_t : public ISphNoncopyable
{
	CSphString				m_sParseError;
	CSphString				m_sParseWarning;

	CSphVector<CSphString>	m_dZones;
	XQNode_t *				m_pRoot;
	bool					m_bNeedSZlist;
	bool					m_bSingleWord;

	/// ctor
	XQQuery_t ()
	{
		m_pRoot = NULL;
		m_bNeedSZlist = false;
		m_bSingleWord = false;
	}

	/// dtor
	~XQQuery_t ()
	{
		SafeDelete ( m_pRoot );
	}
};


class QueryParser_i
{
public:
	QueryParser_i () {}
	virtual ~QueryParser_i () {}
	virtual bool IsFullscan ( const CSphQuery & tQuery ) const = 0;
	virtual bool IsFullscan ( const XQQuery_t & tQuery ) const = 0;
	virtual bool ParseQuery ( XQQuery_t & tParsed, const char * sQuery, const CSphQuery * pQuery, const ISphTokenizer * pQueryTokenizer, const ISphTokenizer * pQueryTokenizerJson, const CSphSchema * pSchema, CSphDict * pDict, const CSphIndexSettings & tSettings ) const = 0;
};

class PluginQueryTokenFilter_c;

class XQParseHelper_c
{
public:
	void			SetString ( const char * szString );

	bool			AddField ( FieldMask_t & dFields, const char * szField, int iLen );
	bool			ParseFields ( FieldMask_t & dFields, int & iMaxFieldPos, bool & bIgnore );

	void			Setup ( const CSphSchema * pSchema, ISphTokenizer * pTokenizer, CSphDict * pDict, XQQuery_t * pXQQuery, const CSphIndexSettings & tSettings );
	bool			Error ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
	void			Warning ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
	XQNode_t *		FixupTree ( XQNode_t * pRoot, const XQLimitSpec_t & tLimitSpec );
	
	bool			IsError() { return m_bError; }
	virtual void	Cleanup();

protected:
	struct MultiformNode_t
	{
		XQNode_t *	m_pNode;
		int			m_iDestStart;
		int			m_iDestCount;
	};

	static const int MAX_TOKEN_BYTES = 3*SPH_MAX_WORD_LEN + 16;

	const CSphSchema *		m_pSchema {nullptr};
	ISphTokenizer *			m_pTokenizer {nullptr};
	CSphDict *				m_pDict {nullptr};
	bool					m_bStopOnInvalid {true};
	XQQuery_t *				m_pParsed {nullptr};
	bool					m_bError {false};

	const PluginQueryTokenFilter_c * m_pPlugin {nullptr};
	void *					m_pPluginData {nullptr};

	int						m_iAtomPos {0};
	bool					m_bEmptyStopword {false};
	bool					m_bWasBlended {false};

	CSphVector<XQNode_t*>		m_dSpawned;
	CSphVector<CSphString>		m_dDestForms;
	CSphVector<MultiformNode_t>	m_dMultiforms;

	virtual bool	HandleFieldBlockStart ( const char * & pPtr ) = 0;
	virtual bool	HandleSpecialFields ( const char * & /*pPtr*/, FieldMask_t & /*dFields*/ ) { return false; }
	virtual bool	NeedTrailingSeparator() { return true; }

private:
	XQNode_t *		SweepNulls ( XQNode_t * pNode );
	bool			FixupNots ( XQNode_t * pNode );
	void			FixupNulls ( XQNode_t * pNode );
	void			DeleteNodesWOFields ( XQNode_t * pNode );
	void			FixupDestForms();
	bool			CheckQuorumProximity ( XQNode_t * pNode );
};

//////////////////////////////////////////////////////////////////////////////

/// setup tokenizer for query parsing (ie. add all specials and whatnot)
void	sphSetupQueryTokenizer ( ISphTokenizer * pTokenizer, bool bWildcards, bool bExact, bool bJson );

// a wrapper for sphParseExtendedQuery
QueryParser_i * sphCreatePlainQueryParser();

/// parses the query and returns the resulting tree
/// return false and fills tQuery.m_sParseError on error
/// WARNING, parsed tree might be NULL (eg. if query was empty)
/// lots of arguments here instead of simply the index pointer, because
/// a) we do not always have an actual real index class, and
/// b) might need to tweak stuff even we do
/// FIXME! remove either pQuery or sQuery
bool	sphParseExtendedQuery ( XQQuery_t & tQuery, const char * sQuery, const CSphQuery * pQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict, const CSphIndexSettings & tSettings );

// perform boolean optimization on tree
void	sphOptimizeBoolean ( XQNode_t ** pXQ, const ISphKeywordsStat * pKeywords );

/// analyze vector of trees and tag common parts of them (to cache them later)
int		sphMarkCommonSubtrees ( int iXQ, const XQQuery_t * pXQ );

#endif // _sphinxquery_

//
// $Id$
//
