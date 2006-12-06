//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
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

//////////////////////////////////////////////////////////////////////////////

/// prevent copy
class ISphNoncopyable
{
public:
								ISphNoncopyable () {}

private:
								ISphNoncopyable ( const ISphNoncopyable & ) {}
	const ISphNoncopyable &		operator = ( const ISphNoncopyable & ) {}
};


/// extended query atom
/// atom is a list of required query words with field and proximity constraints
struct CSphExtendedQueryAtom
{
	CSphVector<CSphString,8>	m_dWords;
	int							m_iField;
	int							m_iMaxDistance;

	/// default ctor
	CSphExtendedQueryAtom ()
		: m_iField ( -1 )
		, m_iMaxDistance ( -1 )
	{}

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
	CSphVector<CSphExtendedQueryNode*,8>	m_dChildren;	///< non-plain node children

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
	bool IsPlain ()
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
/// WARNING, this function MAY return NULL (if the query is empty)
CSphBooleanQueryExpr *		sphParseBooleanQuery ( const char * sQuery, const ISphTokenizer * pTokenizer );

/// parses the query and returns the resulting tree
/// WARNING, this function MAY return NULL (if the query is empty or there were errors)
CSphExtendedQuery *			sphParseExtendedQuery ( const char * sQuery, const ISphTokenizer * pTokenizer, const CSphSchema * pSchema, CSphDict * pDict );

#endif // _sphinxquery_

//
// $Id$
//
