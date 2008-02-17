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

#ifndef _sphinxexpr_
#define _sphinxexpr_

#include "sphinx.h"

/// expression evaluator
struct ISphExpr
{
	/// virtualize dtor
	virtual ~ISphExpr () {}

	/// evaluate this expression for that match
	virtual float Eval ( const CSphMatch & tMatch ) const = 0;

	/// check for arglist subtype
	virtual bool IsArglist () const { return false; }
};

/// parses given expression, builds evaluator
/// returns NULL and fills sError on failure
/// returns pointer to evaluator on success
ISphExpr * sphExprParse ( const char * sExpr, const CSphSchema & tSchema, CSphString & sError );

#endif // _sphinxexpr_

//
// $Id$
//
