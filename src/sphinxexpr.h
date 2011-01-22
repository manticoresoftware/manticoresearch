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

#ifndef _sphinxexpr_
#define _sphinxexpr_

#include "sphinxstd.h"

/// forward decls
class CSphMatch;
struct CSphSchema;
struct CSphString;

/// known attribute types
enum ESphAttr
{
	SPH_ATTR_NONE		= 0,			///< not an attribute at all
	SPH_ATTR_INTEGER	= 1,			///< unsigned 32-bit integer
	SPH_ATTR_TIMESTAMP	= 2,			///< this attr is a timestamp
	SPH_ATTR_ORDINAL	= 3,			///< this attr is an ordinal string number (integer at search time, specially handled at indexing time)
	SPH_ATTR_BOOL		= 4,			///< this attr is a boolean bit field
	SPH_ATTR_FLOAT		= 5,			///< floating point number (IEEE 32-bit)
	SPH_ATTR_BIGINT		= 6,			///< signed 64-bit integer
	SPH_ATTR_STRING		= 7,			///< string (binary; in-memory)
	SPH_ATTR_WORDCOUNT	= 8,			///< string word count (integer at search time,tokenized and counted at indexing time)
	SPH_ATTR_UINT32SET	= 0x40000001UL	///< MVA, set of unsigned 32-bit integers
};

/// expression evaluator
/// can always be evaluated in floats using Eval()
/// can sometimes be evaluated in integers using IntEval(), depending on type as returned from sphExprParse()
struct ISphExpr : public ISphRefcounted
{
public:
	/// evaluate this expression for that match
	virtual float Eval ( const CSphMatch & tMatch ) const = 0;

	/// evaluate this expression for that match, using int math
	virtual int IntEval ( const CSphMatch & tMatch ) const { assert ( 0 ); return (int) Eval ( tMatch ); }

	/// evaluate this expression for that match, using int64 math
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { assert ( 0 ); return (int64_t) Eval ( tMatch ); }

	/// check for arglist subtype
	virtual bool IsArglist () const { return false; }

	/// setup MVA pool
	virtual void SetMVAPool ( const DWORD * ) {}

	/// setup sting pool
	virtual void SetStringPool ( const BYTE * ) {}

	/// get schema columns index which affect expression
	virtual void GetDependencyColumns ( CSphVector<int> & ) const {}
};

/// parses given expression, builds evaluator
/// returns NULL and fills sError on failure
/// returns pointer to evaluator on success
/// fills pAttrType with result type (for now, can be SPH_ATTR_SINT or SPH_ATTR_FLOAT)
ISphExpr * sphExprParse ( const char * sExpr, const CSphSchema & tSchema, ESphAttr * pAttrType, bool * pUsesWeight, CSphString & sError, CSphSchema * pExtra=NULL );

#endif // _sphinxexpr_

//
// $Id$
//
