//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxexpr_
#define _sphinxexpr_

#include "collation.h"
#include "std/refcounted_mt.h"
#include "std/string.h"
#include "std/sharedptr.h"

/// forward decls
class CSphMatch;
class ISphSchema;
class CSphSchema;
struct CSphString;
struct CSphColumnInfo;

/// known attribute types
enum ESphAttr
{
	// these types are full types
	// their typecodes are saved in the index schema, and thus,
	// TYPECODES MUST NOT CHANGE ONCE INTRODUCED
	SPH_ATTR_NONE		= 0,			///< not an attribute at all
	SPH_ATTR_INTEGER	= 1,			///< unsigned 32-bit integer
	SPH_ATTR_TIMESTAMP	= 2,			///< this attr is a timestamp
	// there was SPH_ATTR_ORDINAL=3 once
	SPH_ATTR_BOOL		= 4,			///< this attr is a boolean bit field
	SPH_ATTR_FLOAT		= 5,			///< floating point number (IEEE 32-bit)
	SPH_ATTR_BIGINT		= 6,			///< signed 64-bit integer
	SPH_ATTR_STRING		= 7,			///< string (binary; in-memory)
	// there was SPH_ATTR_WORDCOUNT=8 once
	SPH_ATTR_POLY2D		= 9,			///< vector of floats, 2D polygon (see POLY2D)
	SPH_ATTR_STRINGPTR	= 10,			///< string (binary, in-memory, stored as pointer to the zero-terminated string)
	SPH_ATTR_TOKENCOUNT	= 11,			///< field token count, 32-bit integer
	SPH_ATTR_JSON		= 12,			///< JSON subset; converted, packed, and stored as string
	SPH_ATTR_DOUBLE		= 13,			///< floating point number (IEEE 64-bit)
	SPH_ATTR_UINT64		= 14,			///< unsigned 64-bit integer

	SPH_ATTR_UINT32SET	= 0x40000001UL,	///< MVA, set of unsigned 32-bit integers
	SPH_ATTR_INT64SET	= 0x40000002UL,	///< MVA, set of signed 64-bit integers

	// these types are runtime only
	// used as intermediate types in the expression engine
	SPH_ATTR_MAPARG		= 1000,
	SPH_ATTR_FACTORS	= 1001,			///< packed search factors (binary, in-memory, pooled)
	SPH_ATTR_JSON_FIELD	= 1002,			///< points to particular field in JSON column subset
	SPH_ATTR_FACTORS_JSON	= 1003,		///< packed search factors (binary, in-memory, pooled, provided to client json encoded)

	SPH_ATTR_UINT32SET_PTR,				// in-memory version of MVA32
	SPH_ATTR_INT64SET_PTR,				// in-memory version of MVA64
	SPH_ATTR_JSON_PTR,					// in-memory version of JSON
	SPH_ATTR_JSON_FIELD_PTR,			// in-memory version of JSON_FIELD
	SPH_ATTR_STORED_FIELD
};

/// column evaluation stage
enum ESphEvalStage
{
	SPH_EVAL_STATIC = 0,		///< static data, no real evaluation needed
	SPH_EVAL_PREFILTER,			///< expression needed for candidate matches filtering
	SPH_EVAL_PRESORT,			///< expression needed for final matches sorting
	SPH_EVAL_SORTER,			///< expression evaluated by sorter object
	SPH_EVAL_FINAL,				///< expression not (!) used in filters/sorting; can be postponed until final result set cooking
	SPH_EVAL_POSTLIMIT			///< expression needs to be postponed until we apply all the LIMIT clauses (say, too expensive)
};

/// expression tree wide commands
/// FIXME? maybe merge with ExtraData_e?
enum ESphExprCommand
{
	SPH_EXPR_SET_BLOB_POOL,
	SPH_EXPR_SET_DOCSTORE_ROWID,	///< interface to fetch docs by rowid (final stage)
	SPH_EXPR_SET_DOCSTORE_DOCID,	///< interface to fetch docs by docid (postlimit stage)
	SPH_EXPR_SET_QUERY,
	SPH_EXPR_SET_EXTRA_DATA,
	SPH_EXPR_GET_DEPENDENT_COLS,	///< used to determine proper evaluating stage
	SPH_EXPR_UPDATE_DEPENDENT_COLS,
	SPH_EXPR_GET_UDF,
	SPH_EXPR_GET_STATEFUL_UDF,
	SPH_EXPR_SET_COLUMNAR,
	SPH_EXPR_SET_COLUMNAR_COL,
	SPH_EXPR_GET_COLUMNAR_COL,
	SPH_EXPR_SET_ITERATOR			///< set link between JsonIn expr and iterator
};

/// expression evaluator
/// can always be evaluated in floats using Eval()
/// can sometimes be evaluated in integers using IntEval(), depending on type as returned from sphExprParse()
struct ISphExpr : public ISphRefcountedMT
{
public:
	/// evaluate this expression for that match
	virtual float Eval ( const CSphMatch & tMatch ) const = 0;

	/// evaluate this expression for that match, using int math
	virtual int IntEval ( const CSphMatch & tMatch ) const { assert ( 0 ); return (int) Eval ( tMatch ); }

	/// evaluate this expression for that match, using int64 math
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { assert ( 0 ); return (int64_t) Eval ( tMatch ); }

	/// Evaluate string attr.
	/// Note, that sometimes this method returns pointer to a static buffer
	/// and sometimes it allocates a new buffer, so aware of memory leaks.
	/// IsDataPtrAttr() returns true if this method allocates a new buffer and false otherwise.
	virtual int StringEval ( const CSphMatch &, const BYTE ** ppStr ) const { *ppStr = NULL; return 0; }

	/// Evaluate string as a packed data ptr attr. By default it re-packs StringEval result, but can be overridden
	virtual const BYTE * StringEvalPacked ( const CSphMatch & tMatch ) const;

	/// return string len without calculating/fetching the string (if supported)
	virtual int StringLenEval ( const CSphMatch & tMatch ) const { return -1; }

	/// evaluate MVA attr
	virtual ByteBlob_t MvaEval ( const CSphMatch & ) const { assert( 0 ); return {nullptr, 0}; }

	/// evaluate PACKEDFACTORS
	virtual const BYTE * FactorEval ( const CSphMatch & ) const { assert ( 0 ); return nullptr; }

	/// evaluate PACKEDFACTORS as a packed data ptr attr
	virtual const BYTE * FactorEvalPacked ( const CSphMatch & ) const { assert ( 0 ); return nullptr; }

	/// check for arglist subtype
	/// FIXME? replace with a single GetType() call?
	virtual bool IsArglist () const { return false; }

	/// was this expression spawned in place of a columnar attr?
	virtual bool IsColumnar ( bool * pStored = nullptr ) const { return false; }

	/// was this expression spawned in place of a columnar expression?
	virtual bool IsStored() const { return false; }

	/// does this expression use docstore (at any eval stage)?
	virtual bool UsesDocstore() const { return false; }

	/// check for stringptr subtype
	virtual bool IsDataPtrAttr () const { return false; }

	/// get Nth arg of an arglist
	virtual ISphExpr * GetArg ( int ) const { return NULL; }

	/// get the number of args in an arglist
	virtual int GetNumArgs() const { return 0; }

	/// change the expressions's locator when changing schemas
	virtual void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) = 0;

	/// run a tree wide action (1st arg is an action, 2nd is its parameter)
	/// usually sets something into ISphExpr like string pool or gets something from it like dependent columns
	virtual void Command ( ESphExprCommand, void * ) {}

	/// check for const type
	virtual bool IsConst () const { return false; }

	virtual bool IsJson ( bool & bConverted ) const { return false; }

	/// get expression hash (for query cache)
	virtual uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) = 0;

	/// make undependent clone of self
	virtual ISphExpr* Clone() const = 0;

protected:
	~ISphExpr() override {}
};

using ISphExprRefPtr_c = CSphRefcountedPtr<ISphExpr>;

inline ISphExpr* SafeClone ( ISphExpr * pRhs )
{
	if ( pRhs )
		return pRhs->Clone();
	return nullptr;
}

inline void FreeDataPtr ( const ISphExpr & tExpr, const void * pData )
{
	if ( tExpr.IsDataPtrAttr () && pData )
		delete[] ( (const BYTE *) pData );
}

inline void FreeDataPtr ( const ISphExpr * pExpr, const void * pData )
{
	if ( pExpr )
		FreeDataPtr ( *pExpr, pData );
}

/// set global behavior of grouping by day/week/month/year functions:
/// if invoked true, params treated as UTC timestamps,
/// and as local timestamps otherwise (default)
void SetGroupingInUtcExpr ( bool bGroupingInUtc );

/// named int/string variant
/// used for named expression function arguments block
/// ie. {..} part in, for example, BM25F(1.2, 0.8, {title=3}) call
struct CSphNamedVariant
{
	CSphString		m_sKey;		///< key
	CSphString		m_sValue;	///< value for strings, empty for ints
	int				m_iValue;	///< value for ints
};


/// string expression traits
/// can never be evaluated in floats or integers, only StringEval() is allowed
struct ISphStringExpr : public ISphExpr
{
	float		Eval ( const CSphMatch & ) const override { assert ( 0 && "one just does not simply evaluate a string as float" ); return 0; }
	int			IntEval ( const CSphMatch & ) const override { assert ( 0 && "one just does not simply evaluate a string as int" ); return 0; }
	int64_t		Int64Eval ( const CSphMatch & ) const override { assert ( 0 && "one just does not simply evaluate a string as bigint" ); return 0; }
};

/// hook to extend expressions
/// lets one to add her own identifier and function handlers
struct ISphExprHook
{
	virtual ~ISphExprHook () {}
	/// checks for an identifier known to the hook
	/// returns -1 on failure, a non-negative OID on success
	virtual int IsKnownIdent ( const char * sIdent ) const = 0;

	/// checks for a valid function call
	/// returns -1 on failure, a non-negative OID on success (possibly adjusted)
	virtual int IsKnownFunc ( const char * sFunc ) const = 0;

	/// create node by OID
	/// pEvalStage is an optional out-parameter
	/// hook may fill it, but that is *not* required
	virtual ISphExpr * CreateNode ( int iID, ISphExpr * pLeft, const ISphSchema * pRsetSchema, ESphEvalStage * pEvalStage, bool * pNeedDocIds, CSphString & sError ) = 0;

	/// get identifier return type by OID
	virtual ESphAttr GetIdentType ( int iID ) const = 0;

	/// get function return type by OID and argument types vector
	/// must return SPH_ATTR_NONE and fill the message on failure
	virtual ESphAttr GetReturnType ( int iID, const CSphVector<ESphAttr> & dArgs, bool bAllConst, CSphString & sError ) const = 0;

	/// recursive scope check
	virtual void CheckEnter ( int iID ) = 0;

	/// recursive scope check
	virtual void CheckExit ( int iID ) = 0;
};


// an expression that has no locator
class Expr_NoLocator_c : public ISphExpr
{
public:
	void FixupLocator ( const ISphSchema * /*pOldSchema*/, const ISphSchema * /*pNewSchema*/ ) override {}
};


/// a container used to pass maps of constants/variables around the evaluation tree
class Expr_MapArg_c : public Expr_NoLocator_c
{
public:
	SharedPtrArr_t<CSphNamedVariant>	m_pValues;
	int64_t								m_iCount = 0;

	// c-tr from raw vector - adopt values
	explicit Expr_MapArg_c ( CSphVector<CSphNamedVariant> & dValues )
	{
		m_iCount = dValues.GetLength();
		m_pValues = dValues.LeakData ();
	}

	float Eval ( const CSphMatch & ) const final
	{
		assert ( 0 && "one just does not simply evaluate a const hash" );
		return 0.0f;
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & ) final
	{
		assert ( 0 && "calling GetHash from a const hash" );
		return 0;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_MapArg_c ( *this );
	}

private:
	Expr_MapArg_c ( const Expr_MapArg_c& rhs )
		: m_pValues ( rhs.m_pValues )
		, m_iCount ( rhs.m_iCount )
	{}
};


enum
{
	SPH_FACTOR_DISABLE		= 0,
	SPH_FACTOR_ENABLE		= 1,
	SPH_FACTOR_CALC_ATC		= 1 << 1,
	SPH_FACTOR_JSON_OUT		= 1 << 2
};


/// parses given expression, builds evaluator
/// returns NULL and fills sError on failure
/// returns pointer to evaluator on success
/// fills pAttrType with result type (for now, can be SPH_ATTR_SINT or SPH_ATTR_FLOAT)
/// fills pUsesWeight with a flag whether match relevance is referenced in expression AST
/// fills pEvalStage with a required (!) evaluation stage
class QueryProfile_c;

struct ExprParseArgs_t
{
	ESphAttr *			m_pAttrType = nullptr;
	bool *				m_pUsesWeight = nullptr;
	QueryProfile_c *	m_pProfiler = nullptr;
	ESphCollation		m_eCollation = SPH_COLLATION_DEFAULT;
	ISphExprHook *		m_pHook = nullptr;
	bool *				m_pZonespanlist = nullptr;
	DWORD *				m_pPackedFactorsFlags = nullptr;
	ESphEvalStage *		m_pEvalStage = nullptr;
	DWORD *				m_pStoredField = nullptr;
	bool *				m_pNeedDocIds = nullptr;
};

ISphExpr * sphExprParse ( const char * sExpr, const ISphSchema & tSchema, CSphString & sError, ExprParseArgs_t & tArgs );

ISphExpr * sphJsonFieldConv ( ISphExpr * pExpr );

void SetExprNodeStackItemSize ( int iCreateSize, int iEvalSize );

//////////////////////////////////////////////////////////////////////////

/// init tables used by our geodistance functions
void GeodistInit();

/// haversine sphere distance, radians
float GeodistSphereRad ( float lat1, float lon1, float lat2, float lon2 );

/// haversine sphere distance, degrees
float GeodistSphereDeg ( float lat1, float lon1, float lat2, float lon2 );

/// flat ellipsoid distance, degrees
float GeodistFlatDeg ( float fLat1, float fLon1, float fLat2, float fLon2 );

/// adaptive flat/haversine distance, degrees
float GeodistAdaptiveDeg ( float lat1, float lon1, float lat2, float lon2 );

/// adaptive flat/haversine distance, radians
float GeodistAdaptiveRad ( float lat1, float lon1, float lat2, float lon2 );

/// get geodist conversion coeff
bool sphGeoDistanceUnit ( const char * szUnit, float & fCoeff );

#endif // _sphinxexpr_

