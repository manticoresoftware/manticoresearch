//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
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

/// Sphinx boolean query expression type
enum ESphBooleanQueryExpr
{
	NODE_UNDEF,
	NODE_AND,
	NODE_OR
};


/// Sphinx boolean query expression
struct CSphBooleanQueryExpr
{
	ESphBooleanQueryExpr	m_eType;		///< this node's type
	CSphString				m_sWord;		///< used for simple, ie. 1-word, subexpressions
	CSphBooleanQueryExpr *	m_pExpr;		///< used for composite, ie. non-word, subexperssions
	CSphBooleanQueryExpr *	m_pPrev;		///< next node in the list
	CSphBooleanQueryExpr *	m_pNext;		///< prev node in the list
	CSphBooleanQueryExpr *	m_pParent;		///< parent node
	bool					m_bInvert;		///< whether to invert word/subexpr matching result
	bool					m_bEvaluable;	///< whether this node evaluates to a document list or can only be used as a filter

	/// ctor. zeroes out everything
							CSphBooleanQueryExpr ();

	/// dtor. automatically kills the child and all siblings to the right
							~CSphBooleanQueryExpr ();

	/// detaches this node from its siblings, parent, and subexpressions
	void					Detach ();

	/// create a new tail, if i'm the tail
	CSphBooleanQueryExpr *	NewTail ();

	/// check if this node is totally empty
	bool					IsNull ();

	/// check if this node has no siblings
	bool					IsAlone ();
};


struct CSphBooleanQuery : ISphNoncopyable
{
	CSphString				m_sParseError;
	CSphBooleanQueryExpr *	m_pTree;

	CSphBooleanQuery () : m_pTree ( NULL )	{}
	~CSphBooleanQuery ()					{ SafeDelete ( m_pTree ); }
};

//////////////////////////////////////////////////////////////////////////////

/// extended query word with attached position within atom
struct CSphExtendedQueryAtomWord
{
	CSphString	m_sWord;
	int			m_iAtomPos;

	CSphExtendedQueryAtomWord () : m_iAtomPos ( -1 ) {}
	CSphExtendedQueryAtomWord ( const char * sWord, int iPos ) : m_sWord ( sWord ), m_iAtomPos ( iPos ) {}
};


/// extended query atom
/// atom is a list of required query words with field and proximity constraints
struct CSphExtendedQueryAtom
{
	CSphVector<CSphExtendedQueryAtomWord>	m_dWords;
	DWORD		m_uFields;
	int			m_iMaxDistance;

	/// default ctor
	CSphExtendedQueryAtom ()
		: m_uFields		 ( 0xFFFFFFFF )
		, m_iMaxDistance ( -1 )
	{}

	/// default dtor
	virtual ~CSphExtendedQueryAtom () {}

	/// clears but does NOT free everything
	/// NOTE: preserves field ID!
	virtual void Reset ()
	{
		m_dWords.Reset ();
		m_iMaxDistance = -1;
	}

	/// check if we're empty
	bool IsEmpty () const
	{
		return m_dWords.GetLength()==0;
	}
};


/// extended query node
/// plain nodes are just an atom
/// non-plain nodes are a logical function over children nodes
struct CSphExtendedQueryNode : public ISphNoncopyable
{
	CSphExtendedQueryNode *					m_pParent;		///< my parent node (NULL for root ones)

	CSphExtendedQueryAtom					m_tAtom;		///< plain node atom
	bool									m_bAny;			///< whether to match any or all children (ie. OR or AND)
	CSphVector<CSphExtendedQueryNode*>		m_dChildren;	///< non-plain node children

public:
	/// ctor
	CSphExtendedQueryNode ()
		: m_pParent ( NULL )
		, m_bAny ( false )
	{}

	/// dtor
	~CSphExtendedQueryNode ()
	{
		ARRAY_FOREACH ( i, m_dChildren )
			SafeDelete ( m_dChildren[i] );
	}

	/// check if i'm empty
	bool IsEmpty () const
	{
		assert ( m_tAtom.IsEmpty() || m_dChildren.GetLength()==0 );
		return m_tAtom.IsEmpty() && ( m_dChildren.GetLength()==0 );
	}

	/// check if i'm plain
	bool IsPlain () const
	{
		return m_dChildren.GetLength()==0;
	}

	/// merge in expr
	/// WARNING! pNew contents may no longer be used, and pointer gets set to NULL
	void Submit ( CSphExtendedQueryNode * & pNew, bool bAny );

	/// make a sublevel from this node contents and given node
	/// WARNING! pNew contents may no longer be used, and pointer gets set to NULL
	void Sublevelize ( CSphExtendedQueryNode * & pNew, bool bAny );
};


/// extended query
struct CSphExtendedQuery : public ISphNoncopyable
{
	CSphString					m_sParseError;
	CSphString					m_sParseWarning;
	CSphExtendedQueryNode *		m_pAccept;
	CSphExtendedQueryNode *		m_pReject;

	/// ctor
	CSphExtendedQuery ()
	{
		m_pAccept = new CSphExtendedQueryNode ();
		m_pReject = new CSphExtendedQueryNode ();
	}

	/// dtor
	~CSphExtendedQuery ()
	{
		SafeDelete ( m_pAccept );
		SafeDelete ( m_pReject );
	}
};

//////////////////////////////////////////////////////////////////////////////

/// parses the query and returns the resulting tree
/// return false and fills tQuery.m_sParseError on error
/// WARNING, parsed tree might be NULL (eg. if query was empty)
bool	sphParseBooleanQuery ( CSphBooleanQuery & tQuery, const char * sQuery, const ISphTokenizer * pTokenizer );

/// parses the query and returns the resulting tree
/// return false and fills tQuery.m_sParseError on error
/// WARNING, parsed tree might be NULL (eg. if query was empty)
bool	sphParseExtendedQuery ( CSphExtendedQuery & tQuery, const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict );

#endif // _sphinxquery_

//
// $Id$
//
