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

/// Sphinx query expression type
enum SphQueryExpr_e
{
	NODE_UNDEF,
	NODE_AND,
	NODE_OR
};


/// Sphinx query expression
struct SphQueryExpr_t
{
	SphQueryExpr_e		m_eType;	///< this node's type
	CSphString			m_sWord;	///< used for simple, ie. 1-word, subexpressions
	SphQueryExpr_t *	m_pExpr;	///< used for composite, ie. non-word, subexperssions
	SphQueryExpr_t *	m_pPrev;	///< next node in the list
	SphQueryExpr_t *	m_pNext;	///< prev node in the list
	SphQueryExpr_t *	m_pParent;	///< parent node
	bool				m_bInvert;	///< whether the documents matching the subexpression match this node or not

	/// ctor. zeroes out everything
						SphQueryExpr_t ();

	/// dtor. automatically kills the child and all siblings to the right
						~SphQueryExpr_t ();

	/// detaches this node from its siblings, parent, and subexpressions
	void				Detach ();

	/// create a new tail, if i'm the tail
	SphQueryExpr_t *	NewTail ();

	/// check if this node is totally empty
	bool				IsNull ();

	/// check if this node has no siblings
	bool				IsAlone ();
};


/// parses the query and returns the resulting tree
/// WARNING, this function MAY return NULL (if the query is empty)
SphQueryExpr_t *		sphParseQuery ( const char * sQuery, const ISphTokenizer * pTokenizer );

#endif // _sphinxquery_

//
// $Id$
//
