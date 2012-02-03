//
// $Id$
//

//
// Copyright (c) 2001-2012, Andrew Aksyonoff
// Copyright (c) 2008-2012, Sphinx Technologies Inc
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
	SPH_ATTR_UINT32SET	= 0x40000001UL,	///< MVA, set of unsigned 32-bit integers
	SPH_ATTR_UINT64SET	= 0x40000002UL	///< MVA, set of unsigned 64-bit integers
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

	/// evaluate string attr
	virtual int StringEval ( const CSphMatch &, const BYTE ** ppStr ) const { *ppStr = NULL; return 0; }

	/// evaluate MVA attr
	virtual const DWORD * MvaEval ( const CSphMatch & ) const { assert ( 0 ); return NULL; }

	/// check for arglist subtype
	virtual bool IsArglist () const { return false; }

	/// setup MVA pool
	virtual void SetMVAPool ( const DWORD * ) {}

	/// setup sting pool
	virtual void SetStringPool ( const BYTE * ) {}

	/// get schema columns index which affect expression
	virtual void GetDependencyColumns ( CSphVector<int> & ) const {}
};

/// hook to extend expressions
/// lets one to add her own identifier and function handlers
struct ISphExprHook
{
	virtual ~ISphExprHook () {}
	/// checks for an identifier known to the hook
	/// returns -1 on failure, a non-negative OID on success
	virtual int IsKnownIdent ( const char * sIdent ) = 0;

	/// checks for a valid function call
	/// returns -1 on failure, a non-negative OID on success (possibly adjusted)
	virtual int IsKnownFunc ( const char * sFunc ) = 0;

	/// create node by OID
	virtual ISphExpr * CreateNode ( int iID, ISphExpr * pLeft ) = 0;

	/// get identifier return type by OID
	virtual ESphAttr GetIdentType ( int iID ) = 0;

	/// get function return type by OID and argument types vector
	/// must return SPH_ATTR_NONE and fill the message on failure
	virtual ESphAttr GetReturnType ( int iID, const CSphVector<ESphAttr> & dArgs, bool bAllConst, CSphString & sError ) = 0;

	/// recursive scope check
	virtual void CheckEnter ( int iID ) = 0;

	/// recursive scope check
	virtual void CheckExit ( int iID ) = 0;
};

/// parses given expression, builds evaluator
/// returns NULL and fills sError on failure
/// returns pointer to evaluator on success
/// fills pAttrType with result type (for now, can be SPH_ATTR_SINT or SPH_ATTR_FLOAT)
/// fills pUsesWeight with a flag whether match relevance is referenced in expression AST
ISphExpr * sphExprParse ( const char * sExpr, const CSphSchema & tSchema, ESphAttr * pAttrType, bool * pUsesWeight, CSphString & sError, CSphSchema * pExtra=NULL, ISphExprHook * pHook=NULL );

//////////////////////////////////////////////////////////////////////////

/// initialize UDF manager
void sphUDFInit ( const char * sUdfDir );

/// load UDF function
bool sphUDFCreate ( const char * szLib, const char * szFunc, ESphAttr eRetType, CSphString & sError );

/// unload UDF function
bool sphUDFDrop ( const char * szFunc, CSphString & sError );

#endif // _sphinxexpr_

//
// $Id$
//
