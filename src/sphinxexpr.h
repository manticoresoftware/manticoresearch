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

/// expression opcodes container
class CSphExpr : public CSphVector<DWORD>
{
public:
	/// evalaute this expression for that match
	float Eval ( const CSphMatch & tMatch ) const;
};

/// parses given expression, builds opcodes
bool sphExprParse ( const char * sExpr, const CSphSchema & tSchema, CSphExpr & tOutExpr, CSphString & sError );

#endif // _sphinxexpr_

//
// $Id$
//
