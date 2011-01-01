//
// $Id$
//

//
// Copyright (c) 2001-2011, Andrew Aksyonoff
// Copyright (c) 2008-2011, Sphinx Technologies Inc
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

enum XQStarPosition
{
	STAR_NONE	= 0,
	STAR_FRONT	= 1,
	STAR_BACK	= 2,
	STAR_BOTH	= 3
};

/// extended query word with attached position within atom
struct XQKeyword_t
{
	CSphString			m_sWord;
	int					m_iAtomPos;
	bool				m_bFieldStart;	///< must occur at very start
	bool				m_bFieldEnd;	///< must occur at very end
	DWORD				m_uStarPosition;
	bool				m_bUseSmallBuffers;

	XQKeyword_t ()
		: m_iAtomPos ( -1 )
		, m_bFieldStart ( false )
		, m_bFieldEnd ( false )
		, m_uStarPosition ( STAR_NONE )
		, m_bUseSmallBuffers ( false )
	{}

	XQKeyword_t ( const char * sWord, int iPos )
		: m_sWord ( sWord )
		, m_iAtomPos ( iPos )
		, m_bFieldStart ( false )
		, m_bFieldEnd ( false )
		, m_uStarPosition ( STAR_NONE )
		, m_bUseSmallBuffers ( false )
	{}
};


/// extended query operator
enum XQOperator_e
{
	SPH_QUERY_AND,
	SPH_QUERY_OR,
	SPH_QUERY_NOT,
	SPH_QUERY_ANDNOT,
	SPH_QUERY_BEFORE,
	SPH_QUERY_PHRASE,
	SPH_QUERY_PROXIMITY,
	SPH_QUERY_QUORUM,
	SPH_QUERY_NEAR,
	SPH_QUERY_SENTENCE,
	SPH_QUERY_PARAGRAPH
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

public:
	CSphVector<XQNode_t*>	m_dChildren;	///< non-plain node children

	bool					m_bFieldSpec;	///< whether field spec was already explicitly set
	DWORD					m_uFieldMask;	///< fields mask (spec part)
	int						m_iFieldMaxPos;	///< max position within field (spec part)

	CSphVector<int>			m_dZones;		///< zone indexes in per-query zones list

	CSphVector<XQKeyword_t>	m_dWords;		///< query words (plain node)
	int						m_iOpArg;		///< operator argument (proximity distance, quorum count)
	int						m_iAtomPos;		///< atom position override (currently only used within expanded nodes)
	bool					m_bVirtuallyPlain;	///< "virtually plain" flag (currently only used by expanded nodes)
	bool					m_bNotWeighted;	///< this our expanded but empty word's node

public:
	/// ctor
	XQNode_t ()
		: m_pParent ( NULL )
		, m_eOp ( SPH_QUERY_AND )
		, m_iOrder ( 0 )
		, m_iCounter ( 0 )
		, m_iMagicHash ( 0 )
		, m_bFieldSpec ( false )
		, m_uFieldMask ( 0xFFFFFFFFUL )
		, m_iFieldMaxPos ( 0 )
		, m_iOpArg ( 0 )
		, m_iAtomPos ( -1 )
		, m_bVirtuallyPlain ( false )
		, m_bNotWeighted ( false )
	{}

	/// dtor
	~XQNode_t ()
	{
		ARRAY_FOREACH ( i, m_dChildren )
			SafeDelete ( m_dChildren[i] );
	}

	/// check if i'm empty
	bool IsEmpty () const
	{
		assert ( m_dWords.GetLength()==0 || m_dChildren.GetLength()==0 );
		return m_dWords.GetLength()==0 && m_dChildren.GetLength()==0;
	}

	/// setup field limits
	void SetFieldSpec ( DWORD uMask, int iMaxPos );

	/// setup zone limits
	void SetZoneSpec ( const CSphVector<int> & dZones );

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

	/// precise comparison
	bool IsEqualTo ( const XQNode_t * pNode );

	/// hash me
	uint64_t GetHash () const;

	/// setup new operator and args
	void SetOp ( XQOperator_e eOp, XQNode_t * pArg1, XQNode_t * pArg2=NULL );

	/// setup new operator and args
	void SetOp ( XQOperator_e eOp, CSphVector<XQNode_t*> & dArgs )
	{
		m_eOp = eOp;
		m_dChildren.SwapData(dArgs);
	}

	/// setup new operator (careful parser/transform use only)
	void SetOp ( XQOperator_e eOp )
	{
		m_eOp = eOp;
	}

#ifndef NDEBUG
	/// consistency check
	void Check ( bool bRoot )
	{
		assert ( bRoot || !IsEmpty() ); // empty leaves must be removed from the final tree; empty root is allowed
		assert (!( m_dWords.GetLength() && m_eOp!=SPH_QUERY_AND && m_eOp!=SPH_QUERY_PHRASE && m_eOp!=SPH_QUERY_PROXIMITY && m_eOp!=SPH_QUERY_QUORUM )); // words are only allowed in these node types
		assert (!( m_dWords.GetLength()==1 && m_eOp!=SPH_QUERY_AND )); // 1-word leaves must be of AND type

		ARRAY_FOREACH ( i, m_dChildren )
			m_dChildren[i]->Check ( false );
	}
#endif
};


/// extended query
struct XQQuery_t : public ISphNoncopyable
{
	CSphString				m_sParseError;
	CSphString				m_sParseWarning;

	CSphVector<CSphString>	m_dZones;
	XQNode_t *				m_pRoot;

	/// ctor
	XQQuery_t ()
	{
		m_pRoot = NULL;
	}

	/// dtor
	~XQQuery_t ()
	{
		SafeDelete ( m_pRoot );
	}
};

//////////////////////////////////////////////////////////////////////////////

/// parses the query and returns the resulting tree
/// return false and fills tQuery.m_sParseError on error
/// WARNING, parsed tree might be NULL (eg. if query was empty)
bool	sphParseExtendedQuery ( XQQuery_t & tQuery, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict );

/// analyse vector of trees and tag common parts of them (to cache them later)
int		sphMarkCommonSubtrees ( int iXQ, const XQQuery_t * pXQ );

#endif // _sphinxquery_

//
// $Id$
//
