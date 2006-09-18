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


/// parses the query and returns the resulting tree
/// WARNING, this function MAY return NULL (if the query is empty)
CSphBooleanQueryExpr *		sphParseBooleanQuery ( const char * sQuery, const ISphTokenizer * pTokenizer );

#endif // _sphinxquery_

//
// $Id$
//
