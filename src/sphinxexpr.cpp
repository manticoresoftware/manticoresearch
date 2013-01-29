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

#include "sphinx.h"
#include "sphinxexpr.h"

extern "C"
{
	#include "sphinxudf.h"
}

#include "sphinxutils.h"
#include "sphinxint.h"
#include "sphinxjson.h"
#include <time.h>
#include <math.h>

#if !USE_WINDOWS
#include <unistd.h>
#include <sys/time.h>
#ifdef HAVE_DLOPEN
#include <dlfcn.h>
#endif // HAVE_DLOPEN
#endif // !USE_WINDOWS

//////////////////////////////////////////////////////////////////////////

#ifndef M_LOG2E
#define M_LOG2E		1.44269504088896340736
#endif

#ifndef M_LOG10E
#define M_LOG10E	0.434294481903251827651
#endif

#if !USE_WINDOWS
#ifndef HAVE_DLERROR
#define dlerror() ""
#endif // HAVE_DLERROR
#endif // !USE_WINDOWS


typedef int ( *UdfVer_fn ) ();
typedef int ( *UdfInit_fn ) ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error );
typedef void ( *UdfDeinit_fn ) ( SPH_UDF_INIT * init );


/// loaded UDF library
struct UdfLib_t
{
	void *				m_pHandle;	///< handle from dlopen()
	int					m_iFuncs;	///< number of registered functions from this library
};


/// registered UDF function
struct UdfFunc_t
{
	UdfLib_t *			m_pLib;			///< library descriptor (pointer to library hash value)
	const CSphString *	m_pLibName;		///< library name (pointer to library hash key; filename only, no path!)
	ESphAttr			m_eRetType;		///< function type, currently FLOAT or INT
	UdfInit_fn			m_fnInit;		///< per-query init function, mandatory
	UdfDeinit_fn		m_fnDeinit;		///< per-query deinit function, optional
	void *				m_fnFunc;		///< per-row worker function, mandatory
	int					m_iUserCount;	///< number of active users currently working this function
	bool				m_bToDrop;		///< scheduled for DROP; do not use
};


/// UDF call site
struct UdfCall_t
{
	UdfFunc_t *			m_pUdf;
	SPH_UDF_INIT		m_tInit;
	SPH_UDF_ARGS		m_tArgs;

	UdfCall_t();
	~UdfCall_t();
};

//////////////////////////////////////////////////////////////////////////
// GLOBALS
//////////////////////////////////////////////////////////////////////////

// hack hack hack
UservarIntSet_c * ( *g_pUservarsHook )( const CSphString & sUservar );

static bool								g_bUdfEnabled = false;		///< is there any UDF support all?
static bool								g_bUdfLocked = false;		///< do we allow CREATE/DROP at this point?
static CSphString						g_sUdfDir;
static CSphStaticMutex					g_tUdfMutex;
static SmallStringHash_T<UdfLib_t>		g_hUdfLibs;					///< key is the filename (no path)
static SmallStringHash_T<UdfFunc_t>		g_hUdfFuncs;				///< key is the function name

//////////////////////////////////////////////////////////////////////////
// UDF CALL SITE
//////////////////////////////////////////////////////////////////////////

void * UdfMalloc ( int iLen )
{
	return new BYTE [ iLen ];
}

UdfCall_t::UdfCall_t ()
{
	m_pUdf = NULL;
	m_tInit.func_data = NULL;
	m_tInit.is_const = false;
	m_tArgs.arg_count = 0;
	m_tArgs.arg_types = NULL;
	m_tArgs.arg_values = NULL;
	m_tArgs.arg_names = NULL;
	m_tArgs.str_lengths = NULL;
	m_tArgs.fn_malloc = UdfMalloc;
}

UdfCall_t::~UdfCall_t ()
{
	if ( m_pUdf )
	{
		g_tUdfMutex.Lock ();
		m_pUdf->m_iUserCount--;
		g_tUdfMutex.Unlock ();
	}
	SafeDeleteArray ( m_tArgs.arg_types );
	SafeDeleteArray ( m_tArgs.arg_values );
	SafeDeleteArray ( m_tArgs.arg_names );
	SafeDeleteArray ( m_tArgs.str_lengths );
}

//////////////////////////////////////////////////////////////////////////
// EVALUATION ENGINE
//////////////////////////////////////////////////////////////////////////

struct ExprLocatorTraits_t : public ISphExpr
{
	CSphAttrLocator m_tLocator;
	int m_iLocator;

	ExprLocatorTraits_t ( const CSphAttrLocator & tLocator, int iLocator ) : m_tLocator ( tLocator ), m_iLocator ( iLocator ) {}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		if ( eCmd==SPH_EXPR_GET_DEPENDENT_COLS )
			static_cast < CSphVector<int>* >(pArg)->Add (m_iLocator );
	}
};


struct Expr_GetInt_c : public ExprLocatorTraits_t
{
	Expr_GetInt_c ( const CSphAttrLocator & tLocator, int iLocator ) : ExprLocatorTraits_t ( tLocator, iLocator ) {}
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float) tMatch.GetAttr ( m_tLocator ); } // FIXME! OPTIMIZE!!! we can go the short route here
	virtual int IntEval ( const CSphMatch & tMatch ) const { return (int)tMatch.GetAttr ( m_tLocator ); }
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return (int64_t)tMatch.GetAttr ( m_tLocator ); }
};


struct Expr_GetBits_c : public ExprLocatorTraits_t
{
	Expr_GetBits_c ( const CSphAttrLocator & tLocator, int iLocator ) : ExprLocatorTraits_t ( tLocator, iLocator ) {}
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float) tMatch.GetAttr ( m_tLocator ); }
	virtual int IntEval ( const CSphMatch & tMatch ) const { return (int)tMatch.GetAttr ( m_tLocator ); }
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return (int64_t)tMatch.GetAttr ( m_tLocator ); }
};


struct Expr_GetSint_c : public ExprLocatorTraits_t
{
	Expr_GetSint_c ( const CSphAttrLocator & tLocator, int iLocator ) : ExprLocatorTraits_t ( tLocator, iLocator ) {}
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float)(int)tMatch.GetAttr ( m_tLocator ); }
	virtual int IntEval ( const CSphMatch & tMatch ) const { return (int)tMatch.GetAttr ( m_tLocator ); }
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return (int)tMatch.GetAttr ( m_tLocator ); }
};


struct Expr_GetFloat_c : public ExprLocatorTraits_t
{
	Expr_GetFloat_c ( const CSphAttrLocator & tLocator, int iLocator ) : ExprLocatorTraits_t ( tLocator, iLocator ) {}
	virtual float Eval ( const CSphMatch & tMatch ) const { return tMatch.GetAttrFloat ( m_tLocator ); }
};


struct Expr_GetString_c : public ExprLocatorTraits_t
{
	const BYTE * m_pStrings;

	Expr_GetString_c ( const CSphAttrLocator & tLocator, int iLocator ) : ExprLocatorTraits_t ( tLocator, iLocator ) {}
	virtual float Eval ( const CSphMatch & ) const { assert ( 0 ); return 0; }
	virtual void Command ( ESphExprCommand eCmd, void * pArg ) { if ( eCmd==SPH_EXPR_SET_STRING_POOL ) m_pStrings = (const BYTE*)pArg; }

	virtual int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
	{
		SphAttr_t iOff = tMatch.GetAttr ( m_tLocator );
		if ( iOff>0 )
			return sphUnpackStr ( m_pStrings + iOff, ppStr );

		*ppStr = NULL;
		return 0;
	}
};


struct Expr_GetMva_c : public ExprLocatorTraits_t
{
	const DWORD * m_pMva;

	Expr_GetMva_c ( const CSphAttrLocator & tLocator, int iLocator ) : ExprLocatorTraits_t ( tLocator, iLocator ) {}
	virtual float Eval ( const CSphMatch & ) const { assert ( 0 ); return 0; }
	virtual void Command ( ESphExprCommand eCmd, void * pArg ) { if ( eCmd==SPH_EXPR_SET_MVA_POOL ) m_pMva = (const DWORD*)pArg; }
	virtual const DWORD * MvaEval ( const CSphMatch & tMatch ) const { return tMatch.GetAttrMVA ( m_tLocator, m_pMva ); }
};


struct Expr_GetFactorsAttr_c : public ExprLocatorTraits_t
{
	Expr_GetFactorsAttr_c ( const CSphAttrLocator & tLocator, int iLocator ) : ExprLocatorTraits_t ( tLocator, iLocator ) {}
	virtual float Eval ( const CSphMatch & ) const { assert ( 0 ); return 0; }
	virtual const DWORD * FactorEval ( const CSphMatch & tMatch ) const { return (DWORD *)tMatch.GetAttr ( m_tLocator ); }
};


struct Expr_GetConst_c : public ISphExpr
{
	float m_fValue;
	explicit Expr_GetConst_c ( float fValue ) : m_fValue ( fValue ) {}
	virtual float Eval ( const CSphMatch & ) const { return m_fValue; }
	virtual int IntEval ( const CSphMatch & ) const { return (int)m_fValue; }
	virtual int64_t Int64Eval ( const CSphMatch & ) const { return (int64_t)m_fValue; }
};


struct Expr_GetIntConst_c : public ISphExpr
{
	int m_iValue;
	explicit Expr_GetIntConst_c ( int iValue ) : m_iValue ( iValue ) {}
	virtual float Eval ( const CSphMatch & ) const { return (float) m_iValue; } // no assert() here cause generic float Eval() needs to work even on int-evaluator tree
	virtual int IntEval ( const CSphMatch & ) const { return m_iValue; }
	virtual int64_t Int64Eval ( const CSphMatch & ) const { return m_iValue; }
};


struct Expr_GetInt64Const_c : public ISphExpr
{
	int64_t m_iValue;
	explicit Expr_GetInt64Const_c ( int64_t iValue ) : m_iValue ( iValue ) {}
	virtual float Eval ( const CSphMatch & ) const { return (float) m_iValue; } // no assert() here cause generic float Eval() needs to work even on int-evaluator tree
	virtual int IntEval ( const CSphMatch & ) const { assert ( 0 ); return (int)m_iValue; }
	virtual int64_t Int64Eval ( const CSphMatch & ) const { return m_iValue; }
};


struct Expr_GetStrConst_c : public ISphStringExpr
{
	CSphString m_sVal;
	int m_iLen;

	explicit Expr_GetStrConst_c ( const char * sVal, int iLen )
	{
		if ( iLen>0 )
			SqlUnescape ( m_sVal, sVal, iLen );
		m_iLen = m_sVal.Length();
	}

	virtual int StringEval ( const CSphMatch &, const BYTE ** ppStr ) const
	{
		*ppStr = (const BYTE*) m_sVal.cstr();
		return m_iLen;
	}

	virtual float Eval ( const CSphMatch & ) const { assert ( 0 ); return 0; }
	virtual int IntEval ( const CSphMatch & ) const { assert ( 0 ); return 0; }
	virtual int64_t Int64Eval ( const CSphMatch & ) const { assert ( 0 ); return 0; }
};


struct Expr_GetZonespanlist_c : public ISphStringExpr
{
	CSphString m_sVal;
	int m_iLen;
	CSphVector<int> * m_pData;

	explicit Expr_GetZonespanlist_c ()
		: m_iLen ( 0 )
		, m_pData ( NULL )
	{}

	virtual int StringEval ( const CSphMatch &tMatch, const BYTE ** ppStr ) const
	{
		assert ( ppStr );
		if ( !m_pData )
		{
			*ppStr = NULL;
			return 0;
		}
		CSphString sValue = "";
		const int* pValues = &(*m_pData)[tMatch.m_iTag];
		int iSize = *pValues++;
		for ( int i=0; i<(iSize/2); ++i )
		{
			sValue.SetSprintf ( "%s %d:%d", sValue.cstr(), pValues[0]+1, pValues[1]+1 );
			pValues+=2;
		}
		*ppStr = (const BYTE *) sValue.Leak();
		return sValue.Length();
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		if ( eCmd==SPH_EXPR_SET_EXTRA_DATA )
			static_cast<ISphExtra*>(pArg)->ExtraData ( EXTRA_GET_DATA_ZONESPANS, (void**)&m_pData );
	}

	virtual bool IsStringPtr() const
	{
		return true;
	}
};


struct Expr_GetRankFactors_c : public ISphStringExpr
{
	/// hash type MUST BE IN SYNC with RankerState_Export_fn in sphinxsearch.cpp
	CSphOrderedHash < CSphString, SphDocID_t, IdentityHash_fn, 256 > * m_pFactors;

	explicit Expr_GetRankFactors_c ()
		: m_pFactors ( NULL )
	{}

	virtual int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
	{
		assert ( ppStr );
		if ( !m_pFactors )
		{
			*ppStr = NULL;
			return 0;
		}

		CSphString * sVal = (*m_pFactors) ( tMatch.m_iDocID );
		if ( !sVal )
		{
			*ppStr = NULL;
			return 0;
		}
		int iLen = sVal->Length();
		*ppStr = (const BYTE*)sVal->Leak();
		m_pFactors->Delete ( tMatch.m_iDocID );
		return iLen;
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		if ( eCmd==SPH_EXPR_SET_EXTRA_DATA )
			static_cast<ISphExtra*>(pArg)->ExtraData ( EXTRA_GET_DATA_RANKFACTORS, (void**)&m_pFactors );
	}

	virtual bool IsStringPtr() const
	{
		return true;
	}
};


struct Expr_GetPackedFactors_c : public ISphStringExpr
{
	/// hash type MUST BE IN SYNC with RankerState_Expr_fn in sphinxsearch.cpp
	struct FactorHashEntry_t
	{
		SphDocID_t			m_iId;
		int					m_iRefCount;
		BYTE *				m_pData;
		FactorHashEntry_t *	m_pPrev;
		FactorHashEntry_t *	m_pNext;
	};

	CSphTightVector<FactorHashEntry_t *> * m_pHash;

	explicit Expr_GetPackedFactors_c ()
		: m_pHash ( NULL )
	{}

	virtual const DWORD * FactorEval ( const CSphMatch & tMatch ) const
	{
		if ( !m_pHash || !m_pHash->GetLength() )
			return NULL;

		FactorHashEntry_t * pEntry = (*m_pHash)[tMatch.m_iDocID % m_pHash->GetLength()];
		assert ( pEntry );

		while ( pEntry && pEntry->m_iId!=tMatch.m_iDocID )
			pEntry = pEntry->m_pNext;

		if ( !pEntry )
			return NULL;

		DWORD uDataLen = (BYTE *)pEntry - (BYTE *)pEntry->m_pData;

		BYTE * pData = new BYTE[uDataLen];
		memcpy ( pData, pEntry->m_pData, uDataLen );

		return (DWORD *)pData;
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		if ( eCmd==SPH_EXPR_SET_EXTRA_DATA )
			static_cast<ISphExtra*>(pArg)->ExtraData ( EXTRA_GET_DATA_PACKEDFACTORS, (void**)&m_pHash );
	}

	virtual bool IsStringPtr() const
	{
		return true;
	}
};


struct Expr_GetId_c : public ISphExpr
{
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float)tMatch.m_iDocID; }
	virtual int IntEval ( const CSphMatch & tMatch ) const { return (int)tMatch.m_iDocID; }
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return (int64_t)tMatch.m_iDocID; }
};


struct Expr_GetWeight_c : public ISphExpr
{
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float)tMatch.m_iWeight; }
	virtual int IntEval ( const CSphMatch & tMatch ) const { return (int)tMatch.m_iWeight; }
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return (int64_t)tMatch.m_iWeight; }
};

//////////////////////////////////////////////////////////////////////////

struct Expr_Arglist_c : public ISphExpr
{
	CSphVector<ISphExpr *> m_dArgs;

	Expr_Arglist_c ( ISphExpr * pLeft, ISphExpr * pRight )
	{
		AddArgs ( pLeft );
		AddArgs ( pRight );
	}

	~Expr_Arglist_c ()
	{
		ARRAY_FOREACH ( i, m_dArgs )
			SafeRelease ( m_dArgs[i] );
	}

	void AddArgs ( ISphExpr * pExpr )
	{
		// not an arglist? just add it
		if ( !pExpr->IsArglist() )
		{
			m_dArgs.Add ( pExpr );
			return;
		}

		// arglist? take ownership of its args, and dismiss it
		Expr_Arglist_c * pArgs = (Expr_Arglist_c *) pExpr;
		ARRAY_FOREACH ( i, pArgs->m_dArgs )
		{
			m_dArgs.Add ( pArgs->m_dArgs[i] );
			pArgs->m_dArgs[i] = NULL;
		}
		SafeRelease ( pExpr );
	}

	virtual bool IsArglist () const
	{
		return true;
	}

	virtual ISphExpr * GetArg ( int i ) const
	{
		if ( i>=m_dArgs.GetLength() )
			return NULL;
		return m_dArgs[i];
	}

	virtual float Eval ( const CSphMatch & ) const
	{
		assert ( 0 && "internal error: Eval() must not be explicitly called on arglist" );
		return 0.0f;
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		ARRAY_FOREACH ( i, m_dArgs )
			m_dArgs[i]->Command ( eCmd, pArg );
	}
};

//////////////////////////////////////////////////////////////////////////

struct Expr_Unary_c : public ISphExpr
{
	ISphExpr * m_pFirst;

	Expr_Unary_c ( ISphExpr * p ) : m_pFirst(p) {}
	~Expr_Unary_c() { SafeRelease ( m_pFirst ); }

	virtual void Command ( ESphExprCommand eCmd, void * pArg ) { m_pFirst->Command ( eCmd, pArg ); }
};


struct Expr_Crc32_c : public Expr_Unary_c
{
	explicit Expr_Crc32_c ( ISphExpr * pFirst ) : Expr_Unary_c ( pFirst ) {}
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float)IntEval ( tMatch ); }
	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		const BYTE * pStr;
		return sphCRC32 ( pStr, m_pFirst->StringEval ( tMatch, &pStr ) );
	}
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return IntEval ( tMatch ); }
};


static inline int Fibonacci ( int i )
{
	if ( i<0 )
		return 0;
	int f0 = 0;
	int f1 = 1;
	int j = 0;
	for ( j=0; j+1<i; j+=2 )
	{
		f0 += f1; // f_j
		f1 += f0; // f_{j+1}
	}
	return ( i & 1 ) ? f1 : f0;
}


struct Expr_Fibonacci_c : public Expr_Unary_c
{
	explicit Expr_Fibonacci_c ( ISphExpr * pFirst ) : Expr_Unary_c ( pFirst ) {}

	virtual float Eval ( const CSphMatch & tMatch ) const { return (float)IntEval ( tMatch ); }
	virtual int IntEval ( const CSphMatch & tMatch ) const { return Fibonacci ( m_pFirst->IntEval ( tMatch ) ); }
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return IntEval ( tMatch ); }
};


struct Expr_ToString_c : public Expr_Unary_c
{
protected:
	ESphAttr	m_eArg;

public:
	Expr_ToString_c ( ISphExpr * pArg, ESphAttr eArg )
		: Expr_Unary_c ( pArg )
		, m_eArg ( eArg )
	{}

	virtual float Eval ( const CSphMatch & ) const
	{
		assert ( 0 );
		return 0.0f;
	}

	virtual int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
	{
		CSphString sBuf;
		switch ( m_eArg )
		{
			case SPH_ATTR_INTEGER:	sBuf.SetSprintf ( "%u", m_pFirst->IntEval ( tMatch ) ); break;
			case SPH_ATTR_BIGINT:	sBuf.SetSprintf ( INT64_FMT, m_pFirst->Int64Eval ( tMatch ) ); break;
			case SPH_ATTR_FLOAT:	sBuf.SetSprintf ( "%f", m_pFirst->Eval ( tMatch ) ); break;
			case SPH_ATTR_UINT32SET:
			case SPH_ATTR_INT64SET:
				{
					const DWORD * pValues = m_pFirst->MvaEval ( tMatch );
					if ( !pValues || !*pValues )
						break;

					DWORD nValues = *pValues++;
					assert (!( m_eArg==SPH_ATTR_INT64SET && ( nValues & 1 ) ));

					// OPTIMIZE? minibuffer on stack, less allocs, manual formatting vs printf, etc
					if ( m_eArg==SPH_ATTR_UINT32SET )
					{
						while ( nValues-- )
						{
							if ( sBuf.cstr() )
								sBuf.SetSprintf ( "%s,%u", sBuf.cstr(), *pValues++ );
							else
								sBuf.SetSprintf ( "%u", *pValues++ );
						}
					} else
					{
						for ( ; nValues; nValues-=2, pValues+=2 )
						{
							if ( sBuf.cstr() )
								sBuf.SetSprintf ( "%s,"INT64_FMT, sBuf.cstr(), MVA_UPSIZE ( pValues ) );
							else
								sBuf.SetSprintf ( INT64_FMT, MVA_UPSIZE ( pValues ) );
						}
					}
				}
				break;
			default:
				assert ( 0 && "unhandled arg type in TO_STRING()" );
				break;
		}
		if ( sBuf.IsEmpty() )
		{
			*ppStr = NULL;
			return 0;
		}
		*ppStr = (const BYTE *) sBuf.Leak();
		return strlen ( (const char*) *ppStr );
	}

	virtual bool IsStringPtr() const
	{
		return true;
	}
};


struct Expr_JsonField_c : public ISphExpr
{
protected:
	const BYTE *	m_pStrings;
	JsonKey_t		m_tField;
	CSphAttrLocator	m_tLocator;
	int				m_iAttr;

public:
	Expr_JsonField_c ( const CSphColumnInfo & tCol, int iAttr, const char * sField )
		: m_pStrings ( NULL )
		, m_tField ( sField )
		, m_tLocator ( tCol.m_tLocator )
		, m_iAttr ( iAttr )
	{}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		if ( eCmd==SPH_EXPR_SET_STRING_POOL )
			m_pStrings = (const BYTE*)pArg;
		if ( eCmd==SPH_EXPR_GET_DEPENDENT_COLS )
			static_cast < CSphVector<int>* > ( pArg )->Add ( m_iAttr );
	}

	virtual float Eval ( const CSphMatch & ) const
	{
		assert ( 0 && "one just does not simply evaluate a JSON as float" );
		return 0;
	}

	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const
	{
		DWORD uOff = (DWORD)tMatch.GetAttr ( m_tLocator );
		if ( !uOff )
			return 0;

		const BYTE * pVal = NULL;
		const BYTE * pSrc = NULL;
		sphUnpackStr ( m_pStrings+uOff, &pSrc );
		ESphJsonType eJson = sphJsonFindKey ( &pVal, pSrc, m_tField );
		if ( eJson==JSON_EOF )
			return 0;

		assert ( m_pStrings+uOff<pVal );

		// keep actual attribute type and offset to data packed
		int64_t iPacked = ( ( (int64_t)( pVal-m_pStrings ) ) | ( ( (int64_t)eJson )<<32 ) );
		return iPacked;
	}
};


//////////////////////////////////////////////////////////////////////////

#define FIRST	m_pFirst->Eval(tMatch)
#define SECOND	m_pSecond->Eval(tMatch)
#define THIRD	m_pThird->Eval(tMatch)

#define INTFIRST	m_pFirst->IntEval(tMatch)
#define INTSECOND	m_pSecond->IntEval(tMatch)
#define INTTHIRD	m_pThird->IntEval(tMatch)

#define INT64FIRST	m_pFirst->Int64Eval(tMatch)
#define INT64SECOND	m_pSecond->Int64Eval(tMatch)
#define INT64THIRD	m_pThird->Int64Eval(tMatch)

#define DECLARE_UNARY_TRAITS(_classname,_expr) \
	struct _classname : public Expr_Unary_c \
	{ \
		explicit _classname ( ISphExpr * pFirst ) : Expr_Unary_c ( pFirst ) {}; \
		virtual float Eval ( const CSphMatch & tMatch ) const { return _expr; } \

#define DECLARE_UNARY_FLT(_classname,_expr) \
		DECLARE_UNARY_TRAITS ( _classname, _expr ) \
	};

#define DECLARE_UNARY_INT(_classname,_expr,_expr2,_expr3) \
		DECLARE_UNARY_TRAITS ( _classname, _expr ) \
		virtual int IntEval ( const CSphMatch & tMatch ) const { return _expr2; } \
		virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return _expr3; } \
	};

#define IABS(_arg) ( (_arg)>0 ? (_arg) : (-_arg) )

DECLARE_UNARY_INT ( Expr_Neg_c,		-FIRST,					-INTFIRST,			-INT64FIRST )
DECLARE_UNARY_INT ( Expr_Abs_c,		fabs(FIRST),			IABS(INTFIRST),		IABS(INT64FIRST) )
DECLARE_UNARY_INT ( Expr_Ceil_c,	float(ceil(FIRST)),		int(ceil(FIRST)),	int64_t(ceil(FIRST)) )
DECLARE_UNARY_INT ( Expr_Floor_c,	float(floor(FIRST)),	int(floor(FIRST)),	int64_t(floor(FIRST)) )

DECLARE_UNARY_FLT ( Expr_Sin_c,		float(sin(FIRST)) )
DECLARE_UNARY_FLT ( Expr_Cos_c,		float(cos(FIRST)) )
DECLARE_UNARY_FLT ( Expr_Ln_c,		float(log(FIRST)) )
DECLARE_UNARY_FLT ( Expr_Log2_c,	float(log(FIRST)*M_LOG2E) )
DECLARE_UNARY_FLT ( Expr_Log10_c,	float(log(FIRST)*M_LOG10E) )
DECLARE_UNARY_FLT ( Expr_Exp_c,		float(exp(FIRST)) )
DECLARE_UNARY_FLT ( Expr_Sqrt_c,	float(sqrt(FIRST)) )

DECLARE_UNARY_INT ( Expr_NotInt_c,		(float)(INTFIRST?0:1),		INTFIRST?0:1,	INTFIRST?0:1 );
DECLARE_UNARY_INT ( Expr_NotInt64_c,	(float)(INT64FIRST?0:1),	INT64FIRST?0:1,	INT64FIRST?0:1 );
DECLARE_UNARY_INT ( Expr_Sint_c,		(float)(INTFIRST),			INTFIRST,		INTFIRST )

//////////////////////////////////////////////////////////////////////////

#define DECLARE_BINARY_TRAITS(_classname) \
	struct _classname : public ISphExpr \
	{ \
		ISphExpr * m_pFirst; \
		ISphExpr * m_pSecond; \
		_classname ( ISphExpr * pFirst, ISphExpr * pSecond ) : m_pFirst ( pFirst ), m_pSecond ( pSecond ) {} \
		~_classname () { SafeRelease ( m_pFirst ); SafeRelease ( m_pSecond ); } \
		virtual void Command ( ESphExprCommand eCmd, void * pArg ) { m_pFirst->Command ( eCmd, pArg ); m_pSecond->Command ( eCmd, pArg ); }

#define DECLARE_END() };

#define DECLARE_BINARY_FLT(_classname,_expr) \
		DECLARE_BINARY_TRAITS ( _classname ) \
		virtual float Eval ( const CSphMatch & tMatch ) const { return _expr; } \
	};

#define DECLARE_BINARY_INT(_classname,_expr,_expr2,_expr3) \
		DECLARE_BINARY_TRAITS ( _classname ) \
		virtual float Eval ( const CSphMatch & tMatch ) const { return _expr; } \
		virtual int IntEval ( const CSphMatch & tMatch ) const { return _expr2; } \
		virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return _expr3; } \
	};

#define DECLARE_BINARY_POLY(_classname,_expr,_expr2,_expr3) \
	DECLARE_BINARY_INT ( _classname##Float_c,	_expr,						(int)Eval(tMatch),		(int64_t)Eval(tMatch ) ) \
	DECLARE_BINARY_INT ( _classname##Int_c,		(float)IntEval(tMatch),		_expr2,					(int64_t)IntEval(tMatch) ) \
	DECLARE_BINARY_INT ( _classname##Int64_c,	(float)Int64Eval(tMatch),	(int)Int64Eval(tMatch),	_expr3 )

#define IFFLT(_expr)	( (_expr) ? 1.0f : 0.0f )
#define IFINT(_expr)	( (_expr) ? 1 : 0 )

DECLARE_BINARY_INT ( Expr_Add_c,	FIRST + SECOND,						INTFIRST + INTSECOND,				INT64FIRST + INT64SECOND )
DECLARE_BINARY_INT ( Expr_Sub_c,	FIRST - SECOND,						INTFIRST - INTSECOND,				INT64FIRST - INT64SECOND )
DECLARE_BINARY_INT ( Expr_Mul_c,	FIRST * SECOND,						INTFIRST * INTSECOND,				INT64FIRST * INT64SECOND )
DECLARE_BINARY_FLT ( Expr_Div_c,	FIRST / SECOND )
DECLARE_BINARY_INT ( Expr_BitAnd_c,	(float)(int(FIRST)&int(SECOND)),	INTFIRST & INTSECOND,				INT64FIRST & INT64SECOND )
DECLARE_BINARY_INT ( Expr_BitOr_c,	(float)(int(FIRST)|int(SECOND)),	INTFIRST | INTSECOND,				INT64FIRST | INT64SECOND )
DECLARE_BINARY_INT ( Expr_Mod_c,	(float)(int(FIRST)%int(SECOND)),	INTFIRST % INTSECOND,				INT64FIRST % INT64SECOND )

DECLARE_BINARY_TRAITS ( Expr_Idiv_c )
	virtual float Eval ( const CSphMatch & tMatch ) const
	{
		int iSecond = int(SECOND);
		return iSecond ? float(int(FIRST)/iSecond) : 0.0f;
	}

	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		int iSecond = INTSECOND;
		return iSecond ? ( INTFIRST / iSecond ) : 0;
	}

	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const
	{
		int64_t iSecond = INT64SECOND;
		return iSecond ? ( INT64FIRST / iSecond ) : 0;
	}
DECLARE_END()

DECLARE_BINARY_POLY ( Expr_Lt,		IFFLT ( FIRST<SECOND ),					IFINT ( INTFIRST<INTSECOND ),		IFINT ( INT64FIRST<INT64SECOND ) )
DECLARE_BINARY_POLY ( Expr_Gt,		IFFLT ( FIRST>SECOND ),					IFINT ( INTFIRST>INTSECOND ),		IFINT ( INT64FIRST>INT64SECOND ) )
DECLARE_BINARY_POLY ( Expr_Lte,		IFFLT ( FIRST<=SECOND ),				IFINT ( INTFIRST<=INTSECOND ),		IFINT ( INT64FIRST<=INT64SECOND ) )
DECLARE_BINARY_POLY ( Expr_Gte,		IFFLT ( FIRST>=SECOND ),				IFINT ( INTFIRST>=INTSECOND ),		IFINT ( INT64FIRST>=INT64SECOND ) )
DECLARE_BINARY_POLY ( Expr_Eq,		IFFLT ( fabs ( FIRST-SECOND )<=1e-6 ),	IFINT ( INTFIRST==INTSECOND ),		IFINT ( INT64FIRST==INT64SECOND ) )
DECLARE_BINARY_POLY ( Expr_Ne,		IFFLT ( fabs ( FIRST-SECOND )>1e-6 ),	IFINT ( INTFIRST!=INTSECOND ),		IFINT ( INT64FIRST!=INT64SECOND ) )

DECLARE_BINARY_INT ( Expr_Min_c,	Min ( FIRST, SECOND ),					Min ( INTFIRST, INTSECOND ),		Min ( INT64FIRST, INT64SECOND ) )
DECLARE_BINARY_INT ( Expr_Max_c,	Max ( FIRST, SECOND ),					Max ( INTFIRST, INTSECOND ),		Max ( INT64FIRST, INT64SECOND ) )
DECLARE_BINARY_FLT ( Expr_Pow_c,	float ( pow ( FIRST, SECOND ) ) )

DECLARE_BINARY_POLY ( Expr_And,		FIRST!=0.0f && SECOND!=0.0f,		IFINT ( INTFIRST && INTSECOND ),	IFINT ( INT64FIRST && INT64SECOND ) )
DECLARE_BINARY_POLY ( Expr_Or,		FIRST!=0.0f || SECOND!=0.0f,		IFINT ( INTFIRST || INTSECOND ),	IFINT ( INT64FIRST || INT64SECOND ) )

//////////////////////////////////////////////////////////////////////////

/// boring base stuff
struct ExprThreeway_c : public ISphExpr
{
	ISphExpr * m_pFirst;
	ISphExpr * m_pSecond;
	ISphExpr * m_pThird;

	ExprThreeway_c ( ISphExpr * pFirst, ISphExpr * pSecond, ISphExpr * pThird )
		: m_pFirst ( pFirst )
		, m_pSecond ( pSecond )
		, m_pThird ( pThird )
	{}

	~ExprThreeway_c()
	{
		SafeRelease ( m_pFirst );
		SafeRelease ( m_pSecond );
		SafeRelease ( m_pThird );
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		m_pFirst->Command ( eCmd, pArg );
		m_pSecond->Command ( eCmd, pArg );
		m_pThird->Command ( eCmd, pArg );
	}
};

#define DECLARE_TERNARY(_classname,_expr,_expr2,_expr3) \
	struct _classname : public ExprThreeway_c \
	{ \
		_classname ( ISphExpr * pFirst, ISphExpr * pSecond, ISphExpr * pThird ) \
			: ExprThreeway_c ( pFirst, pSecond, pThird ) {} \
		\
		virtual float Eval ( const CSphMatch & tMatch ) const { return _expr; } \
		virtual int IntEval ( const CSphMatch & tMatch ) const { return _expr2; } \
		virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return _expr3; } \
	};

DECLARE_TERNARY ( Expr_If_c,	( FIRST!=0.0f ) ? SECOND : THIRD,	INTFIRST ? INTSECOND : INTTHIRD,	INT64FIRST ? INT64SECOND : INT64THIRD )
DECLARE_TERNARY ( Expr_Madd_c,	FIRST*SECOND+THIRD,					INTFIRST*INTSECOND + INTTHIRD,		INT64FIRST*INT64SECOND + INT64THIRD )
DECLARE_TERNARY ( Expr_Mul3_c,	FIRST*SECOND*THIRD,					INTFIRST*INTSECOND*INTTHIRD,		INT64FIRST*INT64SECOND*INT64THIRD )

//////////////////////////////////////////////////////////////////////////

#define DECLARE_TIMESTAMP(_classname,_expr) \
	DECLARE_UNARY_TRAITS ( _classname, (float)IntEval(tMatch) ) \
		virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return IntEval(tMatch); } \
		virtual int IntEval ( const CSphMatch & tMatch ) const \
		{ \
			time_t ts = (time_t)INTFIRST;	\
			struct tm s; \
			localtime_r ( &ts, &s ); \
			return _expr; \
		} \
	};

DECLARE_TIMESTAMP ( Expr_Day_c,				s.tm_mday );
DECLARE_TIMESTAMP ( Expr_Month_c,			s.tm_mon+1 );
DECLARE_TIMESTAMP ( Expr_Year_c,			s.tm_year+1900 );
DECLARE_TIMESTAMP ( Expr_YearMonth_c,		(s.tm_year+1900)*100+s.tm_mon+1 );
DECLARE_TIMESTAMP ( Expr_YearMonthDay_c,	(s.tm_year+1900)*10000+(s.tm_mon+1)*100+s.tm_mday );

//////////////////////////////////////////////////////////////////////////
// PARSER INTERNALS
//////////////////////////////////////////////////////////////////////////

#include "yysphinxexpr.h"

/// known functions
enum Func_e
{
	FUNC_NOW=0,

	FUNC_ABS,
	FUNC_CEIL,
	FUNC_FLOOR,
	FUNC_SIN,
	FUNC_COS,
	FUNC_LN,
	FUNC_LOG2,
	FUNC_LOG10,
	FUNC_EXP,
	FUNC_SQRT,
	FUNC_BIGINT,
	FUNC_SINT,
	FUNC_CRC32,
	FUNC_FIBONACCI,

	FUNC_DAY,
	FUNC_MONTH,
	FUNC_YEAR,
	FUNC_YEARMONTH,
	FUNC_YEARMONTHDAY,

	FUNC_MIN,
	FUNC_MAX,
	FUNC_POW,
	FUNC_IDIV,

	FUNC_IF,
	FUNC_MADD,
	FUNC_MUL3,

	FUNC_INTERVAL,
	FUNC_IN,
	FUNC_BITDOT,

	FUNC_GEODIST,
	FUNC_EXIST,
	FUNC_POLY2D,
	FUNC_GEOPOLY2D,
	FUNC_CONTAINS,
	FUNC_ZONESPANLIST,
	FUNC_TO_STRING,
	FUNC_RANKFACTORS,
	FUNC_PACKEDFACTORS
};


struct FuncDesc_t
{
	const char *	m_sName;
	int				m_iArgs;
	Func_e			m_eFunc;
	ESphAttr		m_eRet;
};


static FuncDesc_t g_dFuncs[] =
{
	{ "now",			0,	FUNC_NOW,			SPH_ATTR_INTEGER },

	{ "abs",			1,	FUNC_ABS,			SPH_ATTR_NONE },
	{ "ceil",			1,	FUNC_CEIL,			SPH_ATTR_INTEGER },
	{ "floor",			1,	FUNC_FLOOR,			SPH_ATTR_INTEGER },
	{ "sin",			1,	FUNC_SIN,			SPH_ATTR_FLOAT },
	{ "cos",			1,	FUNC_COS,			SPH_ATTR_FLOAT },
	{ "ln",				1,	FUNC_LN,			SPH_ATTR_FLOAT },
	{ "log2",			1,	FUNC_LOG2,			SPH_ATTR_FLOAT },
	{ "log10",			1,	FUNC_LOG10,			SPH_ATTR_FLOAT },
	{ "exp",			1,	FUNC_EXP,			SPH_ATTR_FLOAT },
	{ "sqrt",			1,	FUNC_SQRT,			SPH_ATTR_FLOAT },
	{ "bigint",			1,	FUNC_BIGINT,		SPH_ATTR_BIGINT },	// type-enforcer special as-if-function
	{ "sint",			1,	FUNC_SINT,			SPH_ATTR_BIGINT },	// type-enforcer special as-if-function
	{ "crc32",			1,	FUNC_CRC32,			SPH_ATTR_INTEGER },
	{ "fibonacci",		1,	FUNC_FIBONACCI,		SPH_ATTR_INTEGER },

	{ "day",			1,	FUNC_DAY,			SPH_ATTR_INTEGER },
	{ "month",			1,	FUNC_MONTH,			SPH_ATTR_INTEGER },
	{ "year",			1,	FUNC_YEAR,			SPH_ATTR_INTEGER },
	{ "yearmonth",		1,	FUNC_YEARMONTH,		SPH_ATTR_INTEGER },
	{ "yearmonthday",	1,	FUNC_YEARMONTHDAY,	SPH_ATTR_INTEGER },

	{ "min",			2,	FUNC_MIN,			SPH_ATTR_NONE },
	{ "max",			2,	FUNC_MAX,			SPH_ATTR_NONE },
	{ "pow",			2,	FUNC_POW,			SPH_ATTR_FLOAT },
	{ "idiv",			2,	FUNC_IDIV,			SPH_ATTR_NONE },

	{ "if",				3,	FUNC_IF,			SPH_ATTR_NONE },
	{ "madd",			3,	FUNC_MADD,			SPH_ATTR_NONE },
	{ "mul3",			3,	FUNC_MUL3,			SPH_ATTR_NONE },

	{ "interval",		-2,	FUNC_INTERVAL,		SPH_ATTR_INTEGER },
	{ "in",				-1, FUNC_IN,			SPH_ATTR_INTEGER },
	{ "bitdot",			-1, FUNC_BITDOT,		SPH_ATTR_NONE },

	{ "geodist",		4,	FUNC_GEODIST,		SPH_ATTR_FLOAT },
	{ "exist",			2,	FUNC_EXIST,			SPH_ATTR_NONE },
	{ "poly2d",			-1,	FUNC_POLY2D,		SPH_ATTR_POLY2D },
	{ "geopoly2d",		-1,	FUNC_GEOPOLY2D,		SPH_ATTR_POLY2D },
	{ "contains",		3,	FUNC_CONTAINS,		SPH_ATTR_INTEGER },
	{ "zonespanlist",	0,	FUNC_ZONESPANLIST,	SPH_ATTR_STRINGPTR },
	{ "to_string",		1,	FUNC_TO_STRING,		SPH_ATTR_STRINGPTR },
	{ "rankfactors",	0,	FUNC_RANKFACTORS,	SPH_ATTR_STRINGPTR },
	{ "packedfactors",	0,	FUNC_PACKEDFACTORS, SPH_ATTR_FACTORS }
};


// helper to generate input data for gperf
// run this, run gperf, that will generate a C program
// copy dAsso from asso_values in that C source
// copy dIndexes from the program output
#if 0
int HashGen()
{
	printf ( "struct func { char *name; int num; };\n%%%%\n" );
	for ( int i=0; i<sizeof(g_dFuncs)/sizeof(g_dFuncs[0]); i++ )
		printf ( "%s, %d\n", g_dFuncs[i].m_sName, i );
	printf ( "%%%%\n" );
	printf ( "void main()\n" );
	printf ( "{\n" );
	printf ( "\tint i;\n" );
	printf ( "\tfor ( i=0; i<=MAX_HASH_VALUE; i++ )\n" );
	printf ( "\t\tprintf ( \"%%d,%%s\", wordlist[i].name[0] ? wordlist[i].num : -1, (i%%10)==9 ? \"\\n\" : \" \" );\n" );
	printf ( "}\n" );
	printf ( "// gperf -Gt 1.p > 1.c\n" );
	exit ( 0 );
	return 0;
}

static int G_HASHGEN = HashGen();
#endif


// FIXME? can remove this by preprocessing the assoc table
static inline BYTE FuncHashLower ( BYTE u )
{
	return ( u>='A' && u<='Z' ) ? ( u | 0x20 ) : u;
}


static int FuncHashLookup ( const char * sKey )
{
	static BYTE dAsso[] =
	{
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 5, 77, 0, 15, 15,
		50, 0, 15, 5, 77, 5, 77, 77, 0, 0,
		0, 5, 10, 10, 0, 25, 50, 25, 77, 50,
		10, 0, 10, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77
	};

	const BYTE * s = (const BYTE*) sKey;
	int iHash = strlen ( sKey );
	switch ( iHash )
	{
		default:		iHash += dAsso [ FuncHashLower ( s[2] ) ];
		case 2:			iHash += dAsso [ FuncHashLower ( s[1] ) ];
		case 1:			iHash += dAsso [ FuncHashLower ( s[0] ) ];
		case 0:
						break;
	}

	static int dIndexes[] =
	{
		-1, -1, 6, -1, 17, -1, -1, 28, 20, 18,
		16, 37, 19, 21, 7, 8, -1, 30, -1, 33,
		31, 32, 24, 9, 2, 3, -1, 35, 34, 26,
		-1, 11, -1, 4, 12, 13, -1, -1, 38, 10,
		-1, -1, -1, 1, 14, -1, -1, -1, 5, -1,
		-1, -1, -1, 15, 25, -1, -1, -1, 0, -1,
		-1, -1, -1, 27, 23, -1, -1, -1, 22, 36,
		-1, -1, -1, -1, -1, -1, 29
	};

	if ( iHash<0 || iHash>=(int)(sizeof(dIndexes)/sizeof(dIndexes[0])) )
		return -1;

	int iFunc = dIndexes[iHash];
	if ( iFunc>=0 && strcasecmp ( g_dFuncs[iFunc].m_sName, sKey )==0 )
		return iFunc;
	return -1;
}


static int FuncHashCheck()
{
	for ( int i=0; i<(int)(sizeof(g_dFuncs)/sizeof(g_dFuncs[0])); i++ )
	{
		CSphString sKey ( g_dFuncs[i].m_sName );
		sKey.ToLower();
		if ( FuncHashLookup ( sKey.cstr() )!=i )
			sphDie ( "INTERNAL ERROR: lookup for %s() failed, rebuild function hash", sKey.cstr() );
		sKey.ToUpper();
		if ( FuncHashLookup ( sKey.cstr() )!=i )
			sphDie ( "INTERNAL ERROR: lookup for %s() failed, rebuild function hash", sKey.cstr() );
	}
	if ( FuncHashLookup("")!=-1 )
		sphDie ( "INTERNAL ERROR: lookup for empty-func-name succeeded, rebuild function hash" );
	if ( FuncHashLookup("A")!=-1 )
		sphDie ( "INTERNAL ERROR: lookup for A() succeeded, rebuild function hash" );
	return 1;
}


static int G_FUNC_HASH_CHECK = FuncHashCheck();

//////////////////////////////////////////////////////////////////////////

/// check for type based on int value
static inline ESphAttr GetIntType ( int64_t iValue )
{
	return ( iValue>=(int64_t)INT_MIN && iValue<=(int64_t)INT_MAX ) ? SPH_ATTR_INTEGER : SPH_ATTR_BIGINT;
}

/// list of constants
class ConstList_c
{
public:
	CSphVector<int64_t>		m_dInts;		///< dword/int64 storage
	CSphVector<float>		m_dFloats;		///< float storage
	ESphAttr				m_eRetType;		///< SPH_ATTR_INTEGER, SPH_ATTR_BIGINT, or SPH_ATTR_FLOAT

public:
	ConstList_c ()
		: m_eRetType ( SPH_ATTR_INTEGER )
	{}

	void Add ( int64_t iValue )
	{
		if ( m_eRetType!=SPH_ATTR_FLOAT )
		{
			m_eRetType = GetIntType ( iValue );
			m_dInts.Add ( iValue );
		} else
		{
			m_dFloats.Add ( (float)iValue );
		}
	}

	void Add ( float fValue )
	{
		if ( m_eRetType!=SPH_ATTR_FLOAT )
		{
			assert ( m_dFloats.GetLength()==0 );
			ARRAY_FOREACH ( i, m_dInts )
				m_dFloats.Add ( (float)m_dInts[i] );
			m_dInts.Reset ();
			m_eRetType = SPH_ATTR_FLOAT;
		}
		m_dFloats.Add ( fValue );
	}
};


/// hash of constants
class ConstHash_c
{
public:
	CSphVector<CSphNamedInt> m_dPairs;

public:
	void Add ( const char * sKey, int64_t iValue )
	{
		CSphNamedInt & t = m_dPairs.Add();
		t.m_sName = sKey;
		t.m_iValue = (int)iValue;
	}
};


/// expression tree node
struct ExprNode_t
{
	int				m_iToken;	///< token type, including operators
	ESphAttr		m_eRetType;	///< result type
	ESphAttr		m_eArgType;	///< args type
	CSphAttrLocator	m_tLocator;	///< attribute locator, for TOK_ATTR type
	int				m_iLocator; ///< index of attribute locator in schema
	union
	{
		int64_t			m_iConst;		///< constant value, for TOK_CONST_INT type
		float			m_fConst;		///< constant value, for TOK_CONST_FLOAT type
		int				m_iFunc;		///< built-in function id, for TOK_FUNC type
		int				m_iArgs;		///< args count, for arglist (token==',') type
		ConstList_c *	m_pConsts;		///< constants list, for TOK_CONST_LIST type
		ConstHash_c *	m_pConsthash;	///< constants hash (name to const), for TOK_CONST_HASH type
	};
	int				m_iLeft;
	int				m_iRight;

	ExprNode_t () : m_iToken ( 0 ), m_eRetType ( SPH_ATTR_NONE ), m_eArgType ( SPH_ATTR_NONE ),
		m_iLocator ( -1 ), m_iLeft ( -1 ), m_iRight ( -1 ) {}

	float FloatVal() const
	{
		assert ( m_iToken==TOK_CONST_INT || m_iToken==TOK_CONST_FLOAT );
		return ( m_iToken==TOK_CONST_INT ) ? (float)m_iConst : m_fConst;
	}
};


/// expression parser
class ExprParser_t
{
	friend int				yylex ( YYSTYPE * lvalp, ExprParser_t * pParser );
	friend int				yyparse ( ExprParser_t * pParser );
	friend void				yyerror ( ExprParser_t * pParser, const char * sMessage );

public:
	ExprParser_t ( CSphSchema * pExtra, ISphExprHook * pHook )
		: m_pHook ( pHook )
		, m_pExtra ( pExtra )
		, m_bHasZonespanlist ( false )
		, m_bHasPackedFactors ( false )
		, m_eEvalStage ( SPH_EVAL_FINAL )
	{}

							~ExprParser_t ();
	ISphExpr *				Parse ( const char * sExpr, const CSphSchema & tSchema, ESphAttr * pAttrType, bool * pUsesWeight, CSphString & sError );

protected:
	int						m_iParsed;	///< filled by yyparse() at the very end
	CSphString				m_sLexerError;
	CSphString				m_sParserError;
	CSphString				m_sCreateError;
	ISphExprHook *			m_pHook;

protected:
	ESphAttr				GetWidestRet ( int iLeft, int iRight );

	int						AddNodeInt ( int64_t iValue );
	int						AddNodeFloat ( float fValue );
	int						AddNodeString ( int64_t iValue );
	int						AddNodeAttr ( int iTokenType, uint64_t uAttrLocator );
	int						AddNodeID ();
	int						AddNodeWeight ();
	int						AddNodeGroupby ();
	int						AddNodeOp ( int iOp, int iLeft, int iRight );
	int						AddNodeFunc ( int iFunc, int iLeft, int iRight=-1 );
	int						AddNodeUdf ( int iCall, int iArg );
	int						AddNodeConstlist ( int64_t iValue );
	int						AddNodeConstlist ( float iValue );
	void					AppendToConstlist ( int iNode, int64_t iValue );
	void					AppendToConstlist ( int iNode, float iValue );
	int						AddNodeUservar ( int iUservar );
	int						AddNodeHookIdent ( int iID );
	int						AddNodeHookFunc ( int iID, int iLeft );
	int						AddNodeConsthash ( const char * sKey, int64_t iValue );
	void					AppendToConsthash ( int iNode, const char * sKey, int64_t iValue );
	const char *			Attr2Ident ( uint64_t uAttrLoc );

private:
	const char *			m_sExpr;
	const char *			m_pCur;
	const char *			m_pLastTokenStart;
	const CSphSchema *		m_pSchema;
	CSphVector<ExprNode_t>	m_dNodes;
	CSphVector<CSphString>	m_dUservars;
	CSphVector<UdfCall_t*>	m_dUdfCalls;
	CSphVector<char*>		m_dIdents;

	CSphSchema *			m_pExtra;

	int						m_iConstNow;

public:
	bool					m_bHasZonespanlist;
	bool					m_bHasPackedFactors;
	ESphEvalStage			m_eEvalStage;

private:
	int						GetToken ( YYSTYPE * lvalp );

	void					GatherArgTypes ( int iNode, CSphVector<int> & dTypes );
	void					GatherArgNodes ( int iNode, CSphVector<int> & dNodes );
	void					GatherArgRetTypes ( int iNode, CSphVector<ESphAttr> & dTypes );

	bool					CheckForConstSet ( int iArgsNode, int iSkip );
	int						ParseAttr ( int iAttr, const char* sTok, YYSTYPE * lvalp );

	template < typename T >
	void					WalkTree ( int iRoot, T & FUNCTOR );

	void					Optimize ( int iNode );
	void					Dump ( int iNode );

	ISphExpr *				CreateTree ( int iNode );
	ISphExpr *				CreateIntervalNode ( int iArgsNode, CSphVector<ISphExpr *> & dArgs );
	ISphExpr *				CreateInNode ( int iNode );
	ISphExpr *				CreateGeodistNode ( int iArgs );
	ISphExpr *				CreateBitdotNode ( int iArgsNode, CSphVector<ISphExpr *> & dArgs );
	ISphExpr *				CreateUdfNode ( int iCall, ISphExpr * pLeft );
	ISphExpr *				CreateExistNode ( const ExprNode_t & tNode );
	ISphExpr *				CreateContainsNode ( const ExprNode_t & tNode );
};

//////////////////////////////////////////////////////////////////////////

/// parse that numeric constant
static int ParseNumeric ( YYSTYPE * lvalp, const char ** ppStr )
{
	assert ( lvalp && ppStr && *ppStr );

	// try float route
	char * pEnd = NULL;
	float fRes = (float) strtod ( *ppStr, &pEnd );

	// try int route
	int64_t iRes = 0;
	bool bInt = true;
	for ( const char * p=(*ppStr); p<pEnd; p++ && bInt )
	{
		if ( isdigit(*p) )
			iRes = iRes*10 + (int)( (*p)-'0' ); // FIXME! missing overflow check, missing octal/hex handling
		else
			bInt = false;
	}

	// choose your destiny
	*ppStr = pEnd;
	if ( bInt )
	{
		lvalp->iConst = iRes;
		return TOK_CONST_INT;
	} else
	{
		lvalp->fConst = fRes;
		return TOK_CONST_FLOAT;
	}
}

static uint64_t sphPackAttrLocator ( const CSphAttrLocator & tLoc, int iLocator )
{
	assert ( iLocator>=0 && iLocator<=0xff );
	uint64_t uIndex = 0;
	uIndex = ( tLoc.m_iBitOffset<<16 ) + tLoc.m_iBitCount + ( (uint64_t)iLocator<<32 );
	if ( tLoc.m_bDynamic )
		uIndex |= ( U64C(1)<<63 );

	return uIndex;
}

static void sphUnpackAttrLocator ( uint64_t uIndex, ExprNode_t * pNode )
{
	assert ( pNode );
	pNode->m_tLocator.m_iBitOffset = (int)( ( uIndex>>16 ) & 0xffff );
	pNode->m_tLocator.m_iBitCount = (int)( uIndex & 0xffff );
	pNode->m_tLocator.m_bDynamic = ( ( uIndex & ( U64C(1)<<63 ) )!=0 );

	pNode->m_iLocator = (int)( ( uIndex>>32 ) & 0xff );
}

int ExprParser_t::ParseAttr ( int iAttr, const char* sTok, YYSTYPE * lvalp )
{
	// check attribute type and width
	const CSphColumnInfo & tCol = m_pSchema->GetAttr ( iAttr );

	int iRes = -1;
	switch ( tCol.m_eAttrType )
	{
	case SPH_ATTR_FLOAT:		iRes = TOK_ATTR_FLOAT;	break;
	case SPH_ATTR_UINT32SET:	iRes = TOK_ATTR_MVA32; break;
	case SPH_ATTR_INT64SET:		iRes = TOK_ATTR_MVA64; break;
	case SPH_ATTR_STRING:		iRes = TOK_ATTR_STRING; break;
	case SPH_ATTR_JSON:			iRes = TOK_ATTR_STRING; break; // FIXME? for now, just handle as string in exprs
	case SPH_ATTR_FACTORS:		iRes = TOK_ATTR_FACTORS; break;
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_TIMESTAMP:
	case SPH_ATTR_BOOL:
	case SPH_ATTR_BIGINT:
	case SPH_ATTR_WORDCOUNT:
	case SPH_ATTR_TOKENCOUNT:
		iRes = tCol.m_tLocator.IsBitfield() ? TOK_ATTR_BITS : TOK_ATTR_INT;
		break;
	default:
		m_sLexerError.SetSprintf ( "attribute '%s' is of unsupported type (type=%d)", sTok, tCol.m_eAttrType );
		return -1;
	}

	if ( m_pExtra )
		m_pExtra->AddAttr ( tCol, true );
	lvalp->iAttrLocator = sphPackAttrLocator ( tCol.m_tLocator, iAttr );
	return iRes;
}


static inline bool IsDigit ( char c )
{
	return c>='0' && c<='9';
}

/// a lexer of my own
/// returns token id and fills lvalp on success
/// returns -1 and fills sError on failure
int ExprParser_t::GetToken ( YYSTYPE * lvalp )
{
	// skip whitespace, check eof
	while ( isspace ( *m_pCur ) ) m_pCur++;
	m_pLastTokenStart = m_pCur;
	if ( !*m_pCur ) return 0;

	// check for constant
	if ( IsDigit ( m_pCur[0] ) )
		return ParseNumeric ( lvalp, &m_pCur );

	// check for field, function, or magic name
	if ( sphIsAttr ( m_pCur[0] )
		|| ( m_pCur[0]=='@' && sphIsAttr ( m_pCur[1] ) && !isdigit ( m_pCur[1] ) ) )
	{
		// get token
		const char * pStart = m_pCur++;
		while ( sphIsAttr ( *m_pCur ) ) m_pCur++;

		CSphString sTok;
		sTok.SetBinary ( pStart, m_pCur-pStart );
		sTok.ToLower ();

		// check for magic name
		if ( sTok=="@id" )			return TOK_ATID;
		if ( sTok=="@weight" )		return TOK_ATWEIGHT;
		if ( sTok=="id" )			return TOK_ID;
		if ( sTok=="weight" )		return TOK_WEIGHT;
		if ( sTok=="groupby" )		return TOK_GROUPBY;
		if ( sTok=="distinct" )		return TOK_DISTINCT;
		if ( sTok=="@geodist" )
		{
			int iGeodist = m_pSchema->GetAttrIndex("@geodist");
			if ( iGeodist==-1 )
			{
				m_sLexerError = "geoanchor is not set, @geodist expression unavailable";
				return -1;
			}
			const CSphAttrLocator & tLoc = m_pSchema->GetAttr ( iGeodist ).m_tLocator;
			lvalp->iAttrLocator = sphPackAttrLocator ( tLoc, iGeodist );
			return TOK_ATTR_FLOAT;
		}

		// check for uservar
		if ( pStart[0]=='@' )
		{
			lvalp->iNode = m_dUservars.GetLength();
			m_dUservars.Add ( sTok );
			return TOK_USERVAR;
		}

		// check for keyword
		if ( sTok=="and" )		{ return TOK_AND; }
		if ( sTok=="or" )		{ return TOK_OR; }
		if ( sTok=="not" )		{ return TOK_NOT; }
		if ( sTok=="div" )		{ return TOK_DIV; }
		if ( sTok=="mod" )		{ return TOK_MOD; }

		if ( sTok=="count" )
		{
			int iAttr = m_pSchema->GetAttrIndex ( "count" );
			if ( iAttr>=0 )
				ParseAttr ( iAttr, sTok.cstr(), lvalp );
			return TOK_COUNT;
		}

		// check for attribute
		int iAttr = m_pSchema->GetAttrIndex ( sTok.cstr() );
		if ( iAttr>=0 )
			return ParseAttr ( iAttr, sTok.cstr(), lvalp );

		// check for function
		int iFunc = FuncHashLookup ( sTok.cstr() );
		if ( iFunc>=0 )
		{
			assert ( !strcasecmp ( g_dFuncs[iFunc].m_sName, sTok.cstr() ) );
			lvalp->iFunc = iFunc;
			return g_dFuncs[iFunc].m_eFunc==FUNC_IN ? TOK_FUNC_IN : TOK_FUNC;
		}

		// ask hook
		if ( m_pHook )
		{
			int iID = m_pHook->IsKnownIdent ( sTok.cstr() );
			if ( iID>=0 )
			{
				lvalp->iNode = iID;
				return TOK_HOOK_IDENT;
			}

			iID = m_pHook->IsKnownFunc ( sTok.cstr() );
			if ( iID>=0 )
			{
				lvalp->iNode = iID;
				return TOK_HOOK_FUNC;
			}
		}

		// check for UDF
		if ( g_bUdfEnabled )
		{
			g_tUdfMutex.Lock();
			UdfFunc_t * pUdf = g_hUdfFuncs ( sTok );
			if ( pUdf )
			{
				if ( pUdf->m_bToDrop )
					pUdf = NULL; // DROP in progress, can not use
				else
					pUdf->m_iUserCount++; // protection against concurrent DROP (decrements in ~UdfCall_t())
				g_tUdfMutex.Unlock();

				lvalp->iNode = m_dUdfCalls.GetLength();
				m_dUdfCalls.Add ( new UdfCall_t() );
				m_dUdfCalls.Last()->m_pUdf = pUdf;
				return TOK_UDF;
			}
			g_tUdfMutex.Unlock();
		}

		// arbitrary identifier, then
		m_dIdents.Add ( sTok.Leak() );
		lvalp->sIdent = m_dIdents.Last();
		return TOK_IDENT;
	}

	// check for known operators, then
	switch ( *m_pCur )
	{
		case '+':
		case '-':
		case '*':
		case '/':
		case '(':
		case ')':
		case ',':
		case '&':
		case '|':
		case '%':
		case '{':
		case '}':
			return *m_pCur++;

		case '<':
			m_pCur++;
			if ( *m_pCur=='>' ) { m_pCur++; return TOK_NE; }
			if ( *m_pCur=='=' ) { m_pCur++; return TOK_LTE; }
			return '<';

		case '>':
			m_pCur++;
			if ( *m_pCur=='=' ) { m_pCur++; return TOK_GTE; }
			return '>';

		case '=':
			m_pCur++;
			if ( *m_pCur=='=' ) m_pCur++;
			return TOK_EQ;

		// special case for float values without leading zero
		case '.':
			{
				char * pEnd = NULL;
				lvalp->fConst = (float) strtod ( m_pCur, &pEnd );
				if ( pEnd )
				{
					m_pCur = pEnd;
					return TOK_CONST_FLOAT;
				}
				break;
			}

		case '\'':
		case '"':
			{
				const char cEnd = *m_pCur;
				for ( const char * s = m_pCur+1; *s; s++ )
				{
					if ( *s==cEnd )
					{
						int iBeg = (int)( m_pCur-m_sExpr );
						int iLen = (int)( s-m_sExpr ) - iBeg + 1;
						lvalp->iConst = ( int64_t(iBeg)<<32 ) + iLen;
						m_pCur = s+1;
						return TOK_CONST_STRING;

					} else if ( *s=='\\' )
					{
						s++;
						if ( !*s )
							break;
					}
				}
				m_sLexerError.SetSprintf ( "unterminated string constant near '%s'", m_pCur );
				return -1;
			}
	}

	m_sLexerError.SetSprintf ( "unknown operator '%c' near '%s'", *m_pCur, m_pCur );
	return -1;
}

/// is add/sub?
static inline bool IsAddSub ( const ExprNode_t * pNode )
{
	return pNode->m_iToken=='+' || pNode->m_iToken=='-';
}

/// is arithmetic?
static inline bool IsAri ( const ExprNode_t * pNode )
{
	int iTok = pNode->m_iToken;
	return iTok=='+' || iTok=='-' || iTok=='*' || iTok=='/';
}

/// is constant?
static inline bool IsConst ( const ExprNode_t * pNode )
{
	return pNode->m_iToken==TOK_CONST_INT || pNode->m_iToken==TOK_CONST_FLOAT;
}

/// float value of a constant
static inline float FloatVal ( const ExprNode_t * pNode )
{
	assert ( IsConst(pNode) );
	return pNode->m_iToken==TOK_CONST_INT
		? (float)pNode->m_iConst
		: pNode->m_fConst;
}

/// optimize subtree
void ExprParser_t::Optimize ( int iNode )
{
	if ( iNode<0 )
		return;

	Optimize ( m_dNodes[iNode].m_iLeft );
	Optimize ( m_dNodes[iNode].m_iRight );

	ExprNode_t * pRoot = &m_dNodes[iNode];
	ExprNode_t * pLeft = ( pRoot->m_iLeft>=0 ) ? &m_dNodes[pRoot->m_iLeft] : NULL;
	ExprNode_t * pRight = ( pRoot->m_iRight>=0 ) ? &m_dNodes[pRoot->m_iRight] : NULL;

	// arithmetic expression with constants
	if ( IsAri(pRoot) )
	{
		// optimize fully-constant expressions
		if ( IsConst(pLeft) && IsConst(pRight) )
		{
			if ( pLeft->m_iToken==TOK_CONST_INT && pRight->m_iToken==TOK_CONST_INT && pRoot->m_iToken!='/' )
			{
				switch ( pRoot->m_iToken )
				{
					case '+':	pRoot->m_iConst = pLeft->m_iConst + pRight->m_iConst; break;
					case '-':	pRoot->m_iConst = pLeft->m_iConst - pRight->m_iConst; break;
					case '*':	pRoot->m_iConst = pLeft->m_iConst * pRight->m_iConst; break;
					default:	assert ( 0 && "internal error: unhandled arithmetic token during const-int optimization" );
				}
				pRoot->m_iToken = TOK_CONST_INT;

			} else
			{
				float fLeft = FloatVal(pLeft);
				float fRight = FloatVal(pRight);
				switch ( pRoot->m_iToken )
				{
					case '+':	pRoot->m_fConst = fLeft + fRight; break;
					case '-':	pRoot->m_fConst = fLeft - fRight; break;
					case '*':	pRoot->m_fConst = fLeft * fRight; break;
					case '/':	pRoot->m_fConst = fLeft / fRight; break;
					default:	assert ( 0 && "internal error: unhandled arithmetic token during const-float optimization" );
				}
				pRoot->m_iToken = TOK_CONST_FLOAT;
			}
			return;
		}

		// canonize (expr op const), move const to the left
		if ( IsConst(pRight) )
		{
			assert ( !IsConst(pLeft) );
			Swap ( pRoot->m_iLeft, pRoot->m_iRight );
			Swap ( pLeft, pRight );

			// fixup (expr-const) to ((-const)+expr)
			if ( pRoot->m_iToken=='-' )
			{
				pRoot->m_iToken = '+';
				if ( pLeft->m_iToken==TOK_CONST_INT )
					pLeft->m_iConst *= -1;
				else
					pLeft->m_fConst *= -1;
			}

			// fixup (expr/const) to ((1/const)*expr)
			if ( pRoot->m_iToken=='/' )
			{
				pRoot->m_iToken = '*';
				pLeft->m_fConst = 1.0f / FloatVal(pLeft);
				pLeft->m_iToken = TOK_CONST_FLOAT;
			}
		}

		// optimize compatible operations with constants
		if ( IsConst(pLeft) && IsAri(pRight) && IsAddSub(pRoot)==IsAddSub(pRight) && IsConst ( &m_dNodes[pRight->m_iLeft] ) )
		{
			ExprNode_t * pConst = &m_dNodes[pRight->m_iLeft];
			ExprNode_t * pExpr = &m_dNodes[pRight->m_iRight];
			assert ( !IsConst(pExpr) ); // must had been optimized

			// optimize (left op (const op2 expr)) to ((left op const) op*op2 expr)
			if ( IsAddSub(pRoot) )
			{
				// fold consts
				int iSign = ( ( pRoot->m_iToken=='+' ) ? 1 : -1 );
				if ( pLeft->m_iToken==TOK_CONST_INT && pConst->m_iToken==TOK_CONST_INT )
				{
					pLeft->m_iConst += iSign*pConst->m_iConst;
				} else
				{
					pLeft->m_fConst = FloatVal(pLeft) + iSign*FloatVal(pConst);
					pLeft->m_iToken = TOK_CONST_FLOAT;
				}

				// fold ops
				pRoot->m_iToken = ( pRoot->m_iToken==pRight->m_iToken ) ? '+' : '-';

			} else
			{
				// fols consts
				if ( pRoot->m_iToken=='*' && pLeft->m_iToken==TOK_CONST_INT && pConst->m_iToken==TOK_CONST_INT )
				{
					pLeft->m_iConst *= pConst->m_iConst;
				} else
				{
					if ( pRoot->m_iToken=='*' )
						pLeft->m_fConst = FloatVal(pLeft) * FloatVal(pConst);
					else
						pLeft->m_fConst = FloatVal(pLeft) / FloatVal(pConst);
					pLeft->m_iToken = TOK_CONST_FLOAT;
				}

				// fold ops
				pRoot->m_iToken = ( pRoot->m_iToken==pRight->m_iToken ) ? '*' : '/';
			}

			// promote expr arg
			pRoot->m_iRight = pRight->m_iRight;
			pRight = pExpr;
		}

		// promote children constants
		if ( IsAri(pLeft) && IsAddSub(pLeft)==IsAddSub(pRoot) && IsConst ( &m_dNodes[pLeft->m_iLeft] ) )
		{
			// ((const op lr) op2 right) gets replaced with (const op (lr op2/op right))
			// constant gets promoted one level up
			int iConst = pLeft->m_iLeft;
			pLeft->m_iLeft = pLeft->m_iRight;
			pLeft->m_iRight = pRoot->m_iRight; // (c op lr) -> (lr ... r)

			switch ( pLeft->m_iToken )
			{
				case '+':
				case '*':
					// (c + lr) op r -> c + (lr op r)
					// (c * lr) op r -> c * (lr op r)
					Swap ( pLeft->m_iToken, pRoot->m_iToken );
					break;

				case '-':
					// (c - lr) + r -> c - (lr - r)
					// (c - lr) - r -> c - (lr + r)
					pLeft->m_iToken = ( pRoot->m_iToken=='+' ? '-' : '+' );
					pRoot->m_iToken = '-';
					break;

				case '/':
					// (c / lr) * r -> c * (r / lr)
					// (c / lr) / r -> c / (r * lr)
					Swap ( pLeft->m_iLeft, pLeft->m_iRight );
					pLeft->m_iToken = ( pRoot->m_iToken=='*' ) ? '/' : '*';
					break;

				default:
					assert ( 0 && "internal error: unhandled op in left-const promotion" );
			}

			pRoot->m_iRight = pRoot->m_iLeft;
			pRoot->m_iLeft = iConst;

			pLeft = &m_dNodes[pRoot->m_iLeft];
			pRight = &m_dNodes[pRoot->m_iRight];
		}
	}

	// madd, mul3
	// FIXME! separate pass for these? otherwise (2+(a*b))+3 won't get const folding
	if ( ( pRoot->m_iToken=='+' || pRoot->m_iToken=='*' ) && ( pLeft->m_iToken=='*' || pRight->m_iToken=='*' ) )
	{
		if ( pLeft->m_iToken!='*' )
		{
			Swap ( pRoot->m_iLeft, pRoot->m_iRight );
			Swap ( pLeft, pRight );
		}

		pLeft->m_iToken = ',';

		int iLeft = pRoot->m_iLeft;
		int iRight = pRoot->m_iRight;

		pRoot->m_iFunc = ( pRoot->m_iToken=='+' ) ? FUNC_MADD : FUNC_MUL3;
		pRoot->m_iToken = TOK_FUNC;
		pRoot->m_iLeft = m_dNodes.GetLength();
		pRoot->m_iRight = -1;
		assert ( g_dFuncs[pRoot->m_iFunc].m_eFunc==pRoot->m_iFunc );

		ExprNode_t & tArgs = m_dNodes.Add(); // invalidates all pointers!
		tArgs.m_iToken = ',';
		tArgs.m_iLeft = iLeft;
		tArgs.m_iRight = iRight;
		return;
	}

	// division by a constant (replace with multiplication by inverse)
	if ( pRoot->m_iToken=='/' && pRight->m_iToken==TOK_CONST_FLOAT )
	{
		pRight->m_fConst = 1.0f / pRight->m_fConst;
		pRoot->m_iToken = '*';
		return;
	}

	// unary function from a constant
	if ( pRoot->m_iToken==TOK_FUNC && g_dFuncs[pRoot->m_iFunc].m_iArgs==1 && IsConst(pLeft) )
	{
		float fArg = pLeft->m_iToken==TOK_CONST_FLOAT ? pLeft->m_fConst : float(pLeft->m_iConst);
		switch ( g_dFuncs[pRoot->m_iFunc].m_eFunc )
		{
			case FUNC_ABS:
				pRoot->m_iToken = pLeft->m_iToken;
				pRoot->m_iLeft = -1;
				if ( pLeft->m_iToken==TOK_CONST_INT )
					pRoot->m_iConst = IABS ( pLeft->m_iConst );
				else
					pRoot->m_fConst = fabs(fArg);
				break;
			case FUNC_CEIL:		pRoot->m_iToken = TOK_CONST_INT; pRoot->m_iLeft = -1; pRoot->m_iConst = (int)ceil(fArg); break;
			case FUNC_FLOOR:	pRoot->m_iToken = TOK_CONST_INT; pRoot->m_iLeft = -1; pRoot->m_iConst = (int)floor(fArg); break;
			case FUNC_SIN:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = float(sin(fArg)); break;
			case FUNC_COS:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = float(cos(fArg)); break;
			case FUNC_LN:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = float(log(fArg)); break;
			case FUNC_LOG2:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = float(log(fArg)*M_LOG2E); break;
			case FUNC_LOG10:	pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = float(log(fArg)*M_LOG10E); break;
			case FUNC_EXP:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = float(exp(fArg)); break;
			case FUNC_SQRT:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = float(sqrt(fArg)); break;
			default:			break;
		}
		return;
	}

	// constant function (such as NOW())
	if ( pRoot->m_iToken==TOK_FUNC && pRoot->m_iFunc==FUNC_NOW )
	{
		pRoot->m_iToken = TOK_CONST_INT;
		pRoot->m_iConst = m_iConstNow;
		return;
	}

	// SINT(int-attr)
	if ( pRoot->m_iToken==TOK_FUNC && pRoot->m_iFunc==FUNC_SINT
		&& ( pLeft->m_iToken==TOK_ATTR_INT || pLeft->m_iToken==TOK_ATTR_BITS ) )
	{
		pRoot->m_iToken = TOK_ATTR_SINT;
		pRoot->m_tLocator = pLeft->m_tLocator;
	}

	// NEG(const)
	if ( pRoot->m_iToken==TOK_NEG && IsConst(pLeft) )
	{
		pRoot->m_iToken = pLeft->m_iToken;
		if ( pRoot->m_iToken==TOK_CONST_INT )
			pRoot->m_iConst = -pLeft->m_iConst;
		else
			pRoot->m_fConst = -pLeft->m_fConst;
	}
}


// debug dump
void ExprParser_t::Dump ( int iNode )
{
	if ( iNode<0 )
		return;

	ExprNode_t & tNode = m_dNodes[iNode];
	switch ( tNode.m_iToken )
	{
		case TOK_CONST_INT:
			printf ( INT64_FMT, tNode.m_iConst );
			break;

		case TOK_CONST_FLOAT:
			printf ( "%f", tNode.m_fConst );
			break;

		case TOK_ATTR_INT:
		case TOK_ATTR_SINT:
			printf ( "row[%d]", tNode.m_tLocator.m_iBitOffset/32 );
			break;

		default:
			printf ( "(" );
			Dump ( tNode.m_iLeft );
			printf ( ( tNode.m_iToken<256 ) ? " %c " : " op-%d ", tNode.m_iToken );
			Dump ( tNode.m_iRight );
			printf ( ")" );
			break;
	}
}


/// fold arglist into array
static void FoldArglist ( ISphExpr * pLeft, CSphVector<ISphExpr *> & dArgs )
{
	if ( !pLeft || !pLeft->IsArglist() )
	{
		dArgs.Add ( pLeft );
		return;
	}

	Expr_Arglist_c * pArgs = dynamic_cast<Expr_Arglist_c *> ( pLeft );
	assert ( pLeft );

	Swap ( dArgs, pArgs->m_dArgs );
	SafeRelease ( pLeft );
}


typedef sphinx_int64_t ( *UdfInt_fn ) ( SPH_UDF_INIT *, SPH_UDF_ARGS *, char * );
typedef double ( *UdfDouble_fn ) ( SPH_UDF_INIT *, SPH_UDF_ARGS *, char * );
typedef char * ( *UdfCharptr_fn) ( SPH_UDF_INIT *, SPH_UDF_ARGS *, char * );


class Expr_Udf_c : public ISphExpr
{
public:
	CSphVector<ISphExpr*>			m_dArgs;

protected:
	UdfCall_t *						m_pCall;
	mutable CSphVector<int64_t>		m_dArgvals;
	mutable char					m_bError;

public:
	explicit Expr_Udf_c ( UdfCall_t * pCall )
		: m_pCall ( pCall )
		, m_bError ( 0 )
	{
		SPH_UDF_ARGS & tArgs = m_pCall->m_tArgs;

		assert ( tArgs.arg_values==NULL );
		tArgs.arg_values = new char * [ tArgs.arg_count ];
		tArgs.str_lengths = new int [ tArgs.arg_count ];

		m_dArgvals.Resize ( tArgs.arg_count );
		ARRAY_FOREACH ( i, m_dArgvals )
			tArgs.arg_values[i] = (char*) &m_dArgvals[i];
	}

	~Expr_Udf_c ()
	{
		if ( m_pCall->m_pUdf->m_fnDeinit )
			m_pCall->m_pUdf->m_fnDeinit ( &m_pCall->m_tInit );
		SafeDeleteArray ( m_pCall->m_tArgs.arg_names );
		SafeDeleteArray ( m_pCall->m_tArgs.arg_types );
		SafeDeleteArray ( m_pCall->m_tArgs.arg_values );
		SafeDeleteArray ( m_pCall->m_tArgs.str_lengths );
		SafeDelete ( m_pCall );

		ARRAY_FOREACH ( i, m_dArgs )
			SafeRelease ( m_dArgs[i] );
	}

	void FillArgs ( const CSphMatch & tMatch ) const
	{
		// FIXME? a cleaner way to reinterpret?
		SPH_UDF_ARGS & tArgs = m_pCall->m_tArgs;
		ARRAY_FOREACH ( i, m_dArgs )
		{
			switch ( tArgs.arg_types[i] )
			{
				case SPH_UDF_TYPE_UINT32:		*(DWORD*)&m_dArgvals[i] = m_dArgs[i]->IntEval ( tMatch ); break;
				case SPH_UDF_TYPE_INT64:		m_dArgvals[i] = m_dArgs[i]->Int64Eval ( tMatch ); break;
				case SPH_UDF_TYPE_FLOAT:		*(float*)&m_dArgvals[i] = m_dArgs[i]->Eval ( tMatch ); break;
				case SPH_UDF_TYPE_STRING:		tArgs.str_lengths[i] = m_dArgs[i]->StringEval ( tMatch, (const BYTE**)&tArgs.arg_values[i] ); break;
				case SPH_UDF_TYPE_UINT32SET:	tArgs.arg_values[i] = (char*) m_dArgs[i]->MvaEval ( tMatch ); break;
				case SPH_UDF_TYPE_UINT64SET:	tArgs.arg_values[i] = (char*) m_dArgs[i]->MvaEval ( tMatch ); break;
				case SPH_UDF_TYPE_FACTORS:		tArgs.arg_values[i] = (char*) m_dArgs[i]->FactorEval ( tMatch ); break;
				default:						assert ( 0 ); m_dArgvals[i] = 0; break;
			}
		}
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		ARRAY_FOREACH ( i, m_dArgs )
			m_dArgs[i]->Command ( eCmd, pArg );
	}
};


class Expr_UdfInt_c : public Expr_Udf_c
{
public:
	explicit Expr_UdfInt_c ( UdfCall_t * pCall )
		: Expr_Udf_c ( pCall )
	{
		assert ( pCall->m_pUdf->m_eRetType==SPH_ATTR_INTEGER || pCall->m_pUdf->m_eRetType==SPH_ATTR_BIGINT );
	}

	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const
	{
		if ( m_bError )
			return 0;
		FillArgs ( tMatch );
		UdfInt_fn pFn = (UdfInt_fn) m_pCall->m_pUdf->m_fnFunc;
		return (int) pFn ( &m_pCall->m_tInit, &m_pCall->m_tArgs, &m_bError );
	}

	virtual int IntEval ( const CSphMatch & tMatch ) const { return (int) Int64Eval ( tMatch ); }
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float) Int64Eval ( tMatch ); }
};


class Expr_UdfFloat_c : public Expr_Udf_c
{
public:
	explicit Expr_UdfFloat_c ( UdfCall_t * pCall )
		: Expr_Udf_c ( pCall )
	{
		assert ( pCall->m_pUdf->m_eRetType==SPH_ATTR_FLOAT );
	}

	virtual float Eval ( const CSphMatch & tMatch ) const
	{
		if ( m_bError )
			return 0;
		FillArgs ( tMatch );
		UdfDouble_fn pFn = (UdfDouble_fn) m_pCall->m_pUdf->m_fnFunc;
		return (float) pFn ( &m_pCall->m_tInit, &m_pCall->m_tArgs, &m_bError );
	}

	virtual int IntEval ( const CSphMatch & tMatch ) const { return (int) Eval ( tMatch ); }
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return (int64_t) Eval ( tMatch ); }
};


class Expr_UdfStringptr_c : public Expr_Udf_c
{
public:
	explicit Expr_UdfStringptr_c ( UdfCall_t * pCall )
		: Expr_Udf_c ( pCall )
	{
		assert ( pCall->m_pUdf->m_eRetType==SPH_ATTR_STRINGPTR );
	}

	virtual float Eval ( const CSphMatch & ) const
	{
		assert ( 0 && "internal error: stringptr udf evaluated as float" );
		return 0.0f;
	}

	virtual int IntEval ( const CSphMatch & ) const
	{
		assert ( 0 && "internal error: stringptr udf evaluated as int" );
		return 0;
	}

	virtual int64_t Int64Eval ( const CSphMatch & ) const
	{
		assert ( 0 && "internal error: stringptr udf evaluated as bigint" );
		return 0;
	}

	virtual int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
	{
		FillArgs ( tMatch );
		UdfCharptr_fn pFn = (UdfCharptr_fn) m_pCall->m_pUdf->m_fnFunc;
		char * pRes = pFn ( &m_pCall->m_tInit, &m_pCall->m_tArgs, &m_bError ); // owned now!
		*ppStr = (const BYTE*) pRes;
		return pRes ? strlen(pRes) : 0;
	}

	virtual bool IsStringPtr() const
	{
		return true;
	}
};


ISphExpr * ExprParser_t::CreateUdfNode ( int iCall, ISphExpr * pLeft )
{
	Expr_Udf_c * pRes = NULL;
	switch ( m_dUdfCalls[iCall]->m_pUdf->m_eRetType )
	{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_BIGINT:
			pRes = new Expr_UdfInt_c ( m_dUdfCalls[iCall] );
			break;
		case SPH_ATTR_FLOAT:
			pRes = new Expr_UdfFloat_c ( m_dUdfCalls[iCall] );
			break;
		case SPH_ATTR_STRINGPTR:
			pRes = new Expr_UdfStringptr_c ( m_dUdfCalls[iCall] );
			break;
		default:
			m_sCreateError.SetSprintf ( "internal error: unhandled type %d in CreateUdfNode()", m_dUdfCalls[iCall]->m_pUdf->m_eRetType );
			break;
	}
	if ( pRes )
	{
		if ( pLeft )
			FoldArglist ( pLeft, pRes->m_dArgs );
		m_dUdfCalls[iCall] = NULL; // evaluator owns it now
	}
	return pRes;
}


ISphExpr * ExprParser_t::CreateExistNode ( const ExprNode_t & tNode )
{
	assert ( m_dNodes[tNode.m_iLeft].m_iToken==',' );
	int iAttrName = m_dNodes[tNode.m_iLeft].m_iLeft;
	int iAttrDefault = m_dNodes[tNode.m_iLeft].m_iRight;
	assert ( iAttrName>=0 && iAttrName<m_dNodes.GetLength()
		&& iAttrDefault>=0 && iAttrDefault<m_dNodes.GetLength() );

	int iNameStart = (int)( m_dNodes[iAttrName].m_iConst>>32 );
	int iNameLen = (int)( m_dNodes[iAttrName].m_iConst & 0xffffffffUL );
	// skip head and tail non attribute name symbols
	while ( m_sExpr[iNameStart]!='\0' && ( m_sExpr[iNameStart]=='\'' || m_sExpr[iNameStart]==' ' ) && iNameLen )
	{
		iNameStart++;
		iNameLen--;
	}
	while ( m_sExpr[iNameStart+iNameLen-1]!='\0'
		&& ( m_sExpr[iNameStart+iNameLen-1]=='\'' || m_sExpr[iNameStart+iNameLen-1]==' ' )
		&& iNameLen )
	{
		iNameLen--;
	}

	if ( iNameLen<=0 )
	{
		m_sCreateError.SetSprintf ( "first EXIST() argument must be valid string" );
		return NULL;
	}

	assert ( iNameStart>=0 && iNameLen>0 && iNameStart+iNameLen<=(int)strlen ( m_sExpr ) );

	CSphString sAttr ( m_sExpr+iNameStart, iNameLen );
	int iLoc = m_pSchema->GetAttrIndex ( sAttr.cstr() );

	if ( iLoc>=0 )
	{
		const CSphColumnInfo & tCol = m_pSchema->GetAttr ( iLoc );
		if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET || tCol.m_eAttrType==SPH_ATTR_STRING )
		{
			m_sCreateError = "MVA and STRING in EXIST() prohibited";
			return NULL;
		}

		const CSphAttrLocator & tLoc = tCol.m_tLocator;
		if ( tNode.m_eRetType==SPH_ATTR_FLOAT )
			return new Expr_GetFloat_c ( tLoc, iLoc );
		else
			return new Expr_GetInt_c ( tLoc, iLoc );
	} else
	{
		if ( tNode.m_eRetType==SPH_ATTR_INTEGER )
			return new Expr_GetIntConst_c ( (int)m_dNodes[iAttrDefault].m_iConst );
		else if ( tNode.m_eRetType==SPH_ATTR_BIGINT )
			return new Expr_GetInt64Const_c ( m_dNodes[iAttrDefault].m_iConst );
		else
			return new Expr_GetConst_c ( m_dNodes[iAttrDefault].m_fConst );
	}
}

//////////////////////////////////////////////////////////////////////////

static inline bool IsNumeric ( ESphAttr eType )
{
	return eType==SPH_ATTR_INTEGER || eType==SPH_ATTR_BIGINT || eType==SPH_ATTR_FLOAT;
}


class Expr_Contains_c : public ISphExpr
{
protected:
	ISphExpr * m_pLat;
	ISphExpr * m_pLon;

	static bool Contains ( float x, float y, int n, const float * p )
	{
		bool bIn = false;
		for ( int ii=0; ii<n; ii+=2 )
		{
			// get that edge
			float ax = p[ii];
			float ay = p[ii+1];
			float bx = ( ii==n-2 ) ? p[0] : p[ii+2];
			float by = ( ii==n-2 ) ? p[1] : p[ii+3];

			// check point vs edge
			float t1 = (x-ax)*(by-ay);
			float t2 = (y-ay)*(bx-ax);
			if ( t1==t2 )
			{
				// so AP and AB are colinear
				// because (AP dot (-AB.y, AB.x)) aka (t1-t2) is 0
				// check (AP dot AB) vs (AB dot AB) then
				float t3 = (x-ax)*(bx-ax) + (y-ay)*(by-ay); // AP dot AP
				float t4 = (bx-ax)*(bx-ax) + (by-ay)*(by-ay); // AB dot AB
				if ( t3>=0 && t3<=t4 )
					return true;
			}

			// count edge crossings
			if ( ( ay>y )!=(by>y) )
				if ( ( t1<t2 ) ^ ( by<ay ) )
					bIn = !bIn;
		}
		return bIn;
	}

public:
	Expr_Contains_c ( ISphExpr * pLat, ISphExpr * pLon )
		: m_pLat ( pLat )
		, m_pLon ( pLon )
	{}

	~Expr_Contains_c()
	{
		SafeRelease ( m_pLat );
		SafeRelease ( m_pLon );
	}

	virtual float Eval ( const CSphMatch & tMatch ) const
	{
		return (float)IntEval ( tMatch );
	}

	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const
	{
		return IntEval ( tMatch );
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		m_pLat->Command ( eCmd, pArg );
		m_pLon->Command ( eCmd, pArg );
	}

	// FIXME! implement SetStringPool?
};


static inline double sphSqr ( double v )
{
	return v * v;
}


static inline float CalcGeodist ( float fPointLat, float fPointLon, float fAnchorLat, float fAnchorLon )
{
	const double R = 6384000;
	double dlat = fPointLat - fAnchorLat;
	double dlon = fPointLon - fAnchorLon;
	double a = sphSqr ( sin ( dlat/2 ) ) + cos ( fPointLat ) * cos ( fAnchorLat ) * sphSqr ( sin ( dlon/2 ) );
	double c = 2*asin ( Min ( 1, sqrt(a) ) );
	return (float)(R*c);
}


static inline void GeoTesselate ( CSphVector<float> & dIn )
{
	// conversion between degrees and radians
	static const float TO_RAD = (float)( 3.14159265358979323846 / 180.0 );
	static const float TO_DEG = (float)( 180.0 / 3.14159265358979323846 );

	// 1 minute of latitude, max
	// (it varies from 1842.9 to 1861.57 at 0 to 90 respectively)
	static const float LAT_MINUTE = 1861.57f;

	// 1 minute of longitude in metres, at different latitudes
	static const float LON_MINUTE[] =
	{
		1855.32f, 1848.31f, 1827.32f, 1792.51f, // 0, 5, 10, 15
		1744.12f, 1682.50f, 1608.10f, 1521.47f, // 20, 25, 30, 35
		1423.23f, 1314.11f, 1194.93f, 1066.57f, // 40, 45, 50, 55
		930.00f, 786.26f, 636.44f, 481.70f, // 60, 65 70, 75
		323.22f, 162.24f, 0.0f // 80, 85, 90
	};

	// tesselation threshold
	// FIXME! make this configurable?
	static const float TESSELATE_TRESH = 500000.0f; // 500 km, error under 150m or 0.03%

	CSphVector<float> dOut;
	for ( int i=0; i<dIn.GetLength(); i+=2 )
	{
		// add the current vertex in any event
		dOut.Add ( dIn[i] );
		dOut.Add ( dIn[i+1] );

		// get edge lat/lon, convert to radians
		bool bLast = ( i==dIn.GetLength()-2 );
		float fLat1 = dIn[i];
		float fLon1 = dIn[i+1];
		float fLat2 = dIn [ bLast ? 0 : (i+2) ];
		float fLon2 = dIn [ bLast ? 1 : (i+3) ];

		// quick rough geodistance estimation
		float fMinLat = Min ( fLat1, fLat2 );
		int iLatBand = (int) floor ( fabs ( fMinLat ) / 5.0f );
		iLatBand = iLatBand % 18;

		float d = 60.0f*( LAT_MINUTE*fabs ( fLat1-fLat2 ) + LON_MINUTE [ iLatBand ]*fabs ( fLon1-fLon2 ) );
		if ( d<=TESSELATE_TRESH )
			continue;

		// convert to radians
		// FIXME! make units configurable
		fLat1 *= TO_RAD;
		fLon1 *= TO_RAD;
		fLat2 *= TO_RAD;
		fLon2 *= TO_RAD;

		// compute precise geodistance
		d = CalcGeodist ( fLat1, fLon1, fLat2, fLon2 );
		if ( d<=TESSELATE_TRESH )
			continue;
		int iSegments = (int) ceil ( d / TESSELATE_TRESH );

		// compute arc distance
		// OPTIMIZE! maybe combine with CalcGeodist?
		d = acos ( sin(fLat1)*sin(fLat2) + cos(fLat1)*cos(fLat2)*cos(fLon1-fLon2) );
		const float isd = 1.0f / sin(d);
		const float clat1 = cos(fLat1);
		const float slat1 = sin(fLat1);
		const float clon1 = cos(fLon1);
		const float slon1 = sin(fLon1);
		const float clat2 = cos(fLat2);
		const float slat2 = sin(fLat2);
		const float clon2 = cos(fLon2);
		const float slon2 = sin(fLon2);

		for ( int j=1; j<iSegments; j++ )
		{
			float f = float(j) / float(iSegments); // needed distance fraction
			float a = sin ( (1-f)*d ) * isd;
			float b = sin ( f*d ) * isd;
			float x = a*clat1*clon1 + b*clat2*clon2;
			float y = a*clat1*slon1 + b*clat2*slon2;
			float z = a*slat1 + b*slat2;
			dOut.Add ( (float)( TO_DEG * atan2 ( z, sqrt ( x*x+y*y ) ) ) );
			dOut.Add ( (float)( TO_DEG * atan2 ( y, x ) ) );
		}
	}

	// swap 'em results
	dIn.SwapData ( dOut );
}


class Expr_ContainsConstvec_c : public Expr_Contains_c
{
protected:
	CSphVector<float> m_dPoly;
	float m_fMinX;
	float m_fMinY;
	float m_fMaxX;
	float m_fMaxY;

public:
	Expr_ContainsConstvec_c ( ISphExpr * pLat, ISphExpr * pLon, const CSphVector<int> & dNodes, const ExprNode_t * pNodes, bool bGeoTesselate )
		: Expr_Contains_c ( pLat, pLon )
	{
		// copy polygon data
		assert ( dNodes.GetLength()>=6 );
		m_dPoly.Resize ( dNodes.GetLength() );

		ARRAY_FOREACH ( i, dNodes )
			m_dPoly[i] = pNodes[dNodes[i]].FloatVal();

		// handle (huge) geosphere polygons
		if ( bGeoTesselate )
			GeoTesselate ( m_dPoly );

		// compute bbox
		m_fMinX = m_fMaxX = m_dPoly[0];
		for ( int i=2; i<m_dPoly.GetLength(); i+=2 )
		{
			m_fMinX = Min ( m_fMinX, m_dPoly[i] );
			m_fMaxX = Max ( m_fMaxX, m_dPoly[i] );
		}

		m_fMinY = m_fMaxY = m_dPoly[1];
		for ( int i=3; i<m_dPoly.GetLength(); i+=2 )
		{
			m_fMinY = Min ( m_fMinY, m_dPoly[i] );
			m_fMaxY = Max ( m_fMaxY, m_dPoly[i] );
		}
	}

	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		// eval args, do bbox check
		float fLat = m_pLat->Eval(tMatch);
		if ( fLat<m_fMinX || fLat>m_fMaxX )
			return 0;

		float fLon = m_pLon->Eval(tMatch);
		if ( fLon<m_fMinY || fLon>m_fMaxY )
			return 0;

		// do the polygon check
		return Contains ( fLat, fLon, m_dPoly.GetLength(), m_dPoly.Begin() );
	}
};


class Expr_ContainsExprvec_c : public Expr_Contains_c
{
protected:
	mutable CSphVector<float> m_dPoly;
	CSphVector<ISphExpr*> m_dExpr;

public:
	Expr_ContainsExprvec_c ( ISphExpr * pLat, ISphExpr * pLon, CSphVector<ISphExpr*> dExprs )
		: Expr_Contains_c ( pLat, pLon )
	{
		m_dExpr.SwapData ( dExprs );
		m_dPoly.Resize ( m_dExpr.GetLength() );
	}

	~Expr_ContainsExprvec_c()
	{
		ARRAY_FOREACH ( i, m_dExpr )
			SafeRelease ( m_dExpr[i] );
	}

	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		ARRAY_FOREACH ( i, m_dExpr )
			m_dPoly[i] = m_dExpr[i]->Eval ( tMatch );
		return Contains ( m_pLat->Eval(tMatch), m_pLon->Eval(tMatch), m_dPoly.GetLength(), m_dPoly.Begin() );
	}
};


class Expr_ContainsStrattr_c : public Expr_Contains_c
{
protected:
	ISphExpr * m_pStr;
	bool m_bGeo;

public:
	Expr_ContainsStrattr_c ( ISphExpr * pLat, ISphExpr * pLon, ISphExpr * pStr, bool bGeo )
		: Expr_Contains_c ( pLat, pLon )
	{
		m_pStr = pStr;
		m_bGeo = bGeo;
	}

	~Expr_ContainsStrattr_c()
	{
		SafeRelease ( m_pStr );
	}

	static void ParsePoly ( const char * p, int iLen, CSphVector<float> & dPoly )
	{
		const char * pMax = p+iLen;
		while ( p<pMax )
		{
			if ( isdigit(p[0]) || ( p+1<pMax && p[0]=='-' && isdigit(p[1]) ) )
				dPoly.Add ( (float)strtod ( p, (char**)&p ) );
			else
				p++;
		}
	}

	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		const char * pStr;
		int iLen = m_pStr->StringEval ( tMatch, (const BYTE **)&pStr );

		CSphVector<float> dPoly;
		ParsePoly ( pStr, iLen, dPoly );
		if ( dPoly.GetLength()<6 )
			return 0;
		// OPTIMIZE? add quick bbox check too?

		if ( m_bGeo )
			GeoTesselate ( dPoly );
		return Contains ( m_pLat->Eval(tMatch), m_pLon->Eval(tMatch), dPoly.GetLength(), dPoly.Begin() );
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		Expr_Contains_c::Command ( eCmd, pArg );
		m_pStr->Command ( eCmd, pArg );
	}
};


ISphExpr * ExprParser_t::CreateContainsNode ( const ExprNode_t & tNode )
{
	// get and check them args
	const ExprNode_t & tArglist = m_dNodes [ tNode.m_iLeft ];
	const int iPoly = m_dNodes [ tArglist.m_iLeft ].m_iLeft;
	const int iLat = m_dNodes [ tArglist.m_iLeft ].m_iRight;
	const int iLon = tArglist.m_iRight;
	assert ( IsNumeric ( m_dNodes[iLat].m_eRetType ) );
	assert ( IsNumeric ( m_dNodes[iLat].m_eRetType ) );
	assert ( m_dNodes[iPoly].m_eRetType==SPH_ATTR_POLY2D );

	// create evaluator
	// gotta handle an optimized constant poly case
	CSphVector<int> dPolyArgs;
	GatherArgNodes ( m_dNodes[iPoly].m_iLeft, dPolyArgs );

	bool bGeoTesselate = ( m_dNodes[iPoly].m_iToken==TOK_FUNC && m_dNodes[iPoly].m_iFunc==FUNC_GEOPOLY2D );

	if ( dPolyArgs.GetLength()==1 && m_dNodes[dPolyArgs[0]].m_iToken==TOK_ATTR_STRING )
	{
		return new Expr_ContainsStrattr_c ( CreateTree(iLat), CreateTree(iLon),
			CreateTree ( dPolyArgs[0] ), bGeoTesselate );
	}

	bool bConst = ARRAY_ALL ( bConst, dPolyArgs, IsConst ( &m_dNodes [ dPolyArgs[_all] ] ) );
	if ( bConst )
	{
		// POLY2D(numeric-consts)
		return new Expr_ContainsConstvec_c ( CreateTree(iLat), CreateTree(iLon),
			dPolyArgs, m_dNodes.Begin(), bGeoTesselate );
	} else
	{
		// POLY2D(generic-exprs)
		CSphVector<ISphExpr*> dExprs ( dPolyArgs.GetLength() );
		ARRAY_FOREACH ( i, dExprs )
			dExprs[i] = CreateTree ( dPolyArgs[i] );
		return new Expr_ContainsExprvec_c ( CreateTree(iLat), CreateTree(iLon), dExprs );
	}
}

//////////////////////////////////////////////////////////////////////////

/// fold nodes subtree into opcodes
ISphExpr * ExprParser_t::CreateTree ( int iNode )
{
	if ( iNode<0 )
		return NULL;

	const ExprNode_t & tNode = m_dNodes[iNode];

	// avoid spawning argument node in some cases
	bool bSkipLeft = false;
	bool bSkipRight = false;
	if ( tNode.m_iToken==TOK_FUNC )
	{
		switch ( g_dFuncs[tNode.m_iFunc].m_eFunc )
		{
		case FUNC_IN:
		case FUNC_EXIST:
			bSkipRight = true;
			// no break
		case FUNC_GEODIST:
		case FUNC_CONTAINS:
		case FUNC_ZONESPANLIST:
		case FUNC_RANKFACTORS:
		case FUNC_PACKEDFACTORS:
			bSkipLeft = true;
		default:
			break;
		}
	}

	ISphExpr * pLeft = bSkipLeft ? NULL : CreateTree ( tNode.m_iLeft );
	ISphExpr * pRight = bSkipRight ? NULL : CreateTree ( tNode.m_iRight );

#define LOC_SPAWN_POLY(_classname) \
	if ( tNode.m_eArgType==SPH_ATTR_INTEGER )		return new _classname##Int_c ( pLeft, pRight ); \
	else if ( tNode.m_eArgType==SPH_ATTR_BIGINT )	return new _classname##Int64_c ( pLeft, pRight ); \
	else											return new _classname##Float_c ( pLeft, pRight );

	switch ( tNode.m_iToken )
	{
		case TOK_ATTR_INT:		return new Expr_GetInt_c ( tNode.m_tLocator, tNode.m_iLocator );
		case TOK_ATTR_BITS:		return new Expr_GetBits_c ( tNode.m_tLocator, tNode.m_iLocator );
		case TOK_ATTR_FLOAT:	return new Expr_GetFloat_c ( tNode.m_tLocator, tNode.m_iLocator );
		case TOK_ATTR_SINT:		return new Expr_GetSint_c ( tNode.m_tLocator, tNode.m_iLocator );
		case TOK_ATTR_STRING:	return new Expr_GetString_c ( tNode.m_tLocator, tNode.m_iLocator );
		case TOK_ATTR_MVA64:
		case TOK_ATTR_MVA32:	return new Expr_GetMva_c ( tNode.m_tLocator, tNode.m_iLocator );
		case TOK_ATTR_FACTORS:	return new Expr_GetFactorsAttr_c ( tNode.m_tLocator, tNode.m_iLocator );

		case TOK_CONST_FLOAT:	return new Expr_GetConst_c ( tNode.m_fConst );
		case TOK_CONST_INT:
			if ( tNode.m_eRetType==SPH_ATTR_INTEGER )
				return new Expr_GetIntConst_c ( (int)tNode.m_iConst );
			else if ( tNode.m_eRetType==SPH_ATTR_BIGINT )
				return new Expr_GetInt64Const_c ( tNode.m_iConst );
			else
				return new Expr_GetConst_c ( float(tNode.m_iConst) );
			break;
		case TOK_CONST_STRING:
			return new Expr_GetStrConst_c ( m_sExpr+(int)( tNode.m_iConst>>32 ), (int)( tNode.m_iConst & 0xffffffffUL ) );

		case TOK_ID:			return new Expr_GetId_c ();
		case TOK_WEIGHT:		return new Expr_GetWeight_c ();

		case '+':				return new Expr_Add_c ( pLeft, pRight ); break;
		case '-':				return new Expr_Sub_c ( pLeft, pRight ); break;
		case '*':				return new Expr_Mul_c ( pLeft, pRight ); break;
		case '/':				return new Expr_Div_c ( pLeft, pRight ); break;
		case '&':				return new Expr_BitAnd_c ( pLeft, pRight ); break;
		case '|':				return new Expr_BitOr_c ( pLeft, pRight ); break;
		case '%':				return new Expr_Mod_c ( pLeft, pRight ); break;

		case '<':				LOC_SPAWN_POLY ( Expr_Lt ); break;
		case '>':				LOC_SPAWN_POLY ( Expr_Gt ); break;
		case TOK_LTE:			LOC_SPAWN_POLY ( Expr_Lte ); break;
		case TOK_GTE:			LOC_SPAWN_POLY ( Expr_Gte ); break;
		case TOK_EQ:			LOC_SPAWN_POLY ( Expr_Eq ); break;
		case TOK_NE:			LOC_SPAWN_POLY ( Expr_Ne ); break;
		case TOK_AND:			LOC_SPAWN_POLY ( Expr_And ); break;
		case TOK_OR:			LOC_SPAWN_POLY ( Expr_Or ); break;
		case TOK_NOT:
			if ( tNode.m_eArgType==SPH_ATTR_BIGINT )
				return new Expr_NotInt64_c ( pLeft );
			else
				return new Expr_NotInt_c ( pLeft );
			break;

		case ',':				return new Expr_Arglist_c ( pLeft, pRight ); break;
		case TOK_NEG:			assert ( pRight==NULL ); return new Expr_Neg_c ( pLeft ); break;
		case TOK_FUNC:
			{
				// fold arglist to array
				Func_e eFunc = g_dFuncs[tNode.m_iFunc].m_eFunc;

				CSphVector<ISphExpr *> dArgs;
				if ( !bSkipLeft )
					FoldArglist ( pLeft, dArgs );

				// spawn proper function
				assert ( tNode.m_iFunc>=0 && tNode.m_iFunc<int(sizeof(g_dFuncs)/sizeof(g_dFuncs[0])) );
				assert (
					( bSkipLeft ) || // function will handle its arglist,
					( g_dFuncs[tNode.m_iFunc].m_iArgs>=0 && g_dFuncs[tNode.m_iFunc].m_iArgs==dArgs.GetLength() ) || // arg count matches,
					( g_dFuncs[tNode.m_iFunc].m_iArgs<0 && -g_dFuncs[tNode.m_iFunc].m_iArgs<=dArgs.GetLength() ) ); // or min vararg count reached

				switch ( eFunc )
				{
					case FUNC_NOW:		assert ( 0 ); break; // prevent gcc bitching

					case FUNC_ABS:		return new Expr_Abs_c ( dArgs[0] );
					case FUNC_CEIL:		return new Expr_Ceil_c ( dArgs[0] );
					case FUNC_FLOOR:	return new Expr_Floor_c ( dArgs[0] );
					case FUNC_SIN:		return new Expr_Sin_c ( dArgs[0] );
					case FUNC_COS:		return new Expr_Cos_c ( dArgs[0] );
					case FUNC_LN:		return new Expr_Ln_c ( dArgs[0] );
					case FUNC_LOG2:		return new Expr_Log2_c ( dArgs[0] );
					case FUNC_LOG10:	return new Expr_Log10_c ( dArgs[0] );
					case FUNC_EXP:		return new Expr_Exp_c ( dArgs[0] );
					case FUNC_SQRT:		return new Expr_Sqrt_c ( dArgs[0] );
					case FUNC_BIGINT:	return dArgs[0];
					case FUNC_SINT:		return new Expr_Sint_c ( dArgs[0] );
					case FUNC_CRC32:	return new Expr_Crc32_c ( dArgs[0] );
					case FUNC_FIBONACCI:return new Expr_Fibonacci_c ( dArgs[0] );

					case FUNC_DAY:			return new Expr_Day_c ( dArgs[0] );
					case FUNC_MONTH:		return new Expr_Month_c ( dArgs[0] );
					case FUNC_YEAR:			return new Expr_Year_c ( dArgs[0] );
					case FUNC_YEARMONTH:	return new Expr_YearMonth_c ( dArgs[0] );
					case FUNC_YEARMONTHDAY:	return new Expr_YearMonthDay_c ( dArgs[0] );

					case FUNC_MIN:		return new Expr_Min_c ( dArgs[0], dArgs[1] );
					case FUNC_MAX:		return new Expr_Max_c ( dArgs[0], dArgs[1] );
					case FUNC_POW:		return new Expr_Pow_c ( dArgs[0], dArgs[1] );
					case FUNC_IDIV:		return new Expr_Idiv_c ( dArgs[0], dArgs[1] );

					case FUNC_IF:		return new Expr_If_c ( dArgs[0], dArgs[1], dArgs[2] );
					case FUNC_MADD:		return new Expr_Madd_c ( dArgs[0], dArgs[1], dArgs[2] );
					case FUNC_MUL3:		return new Expr_Mul3_c ( dArgs[0], dArgs[1], dArgs[2] );

					case FUNC_INTERVAL:	return CreateIntervalNode ( tNode.m_iLeft, dArgs );
					case FUNC_IN:		return CreateInNode ( iNode );
					case FUNC_BITDOT:	return CreateBitdotNode ( tNode.m_iLeft, dArgs );

					case FUNC_GEODIST:	return CreateGeodistNode ( tNode.m_iLeft );
					case FUNC_EXIST:	return CreateExistNode ( tNode );
					case FUNC_CONTAINS:	return CreateContainsNode ( tNode );

					case FUNC_POLY2D:
					case FUNC_GEOPOLY2D:break; // just make gcc happy

					case FUNC_ZONESPANLIST:
						m_bHasZonespanlist = true;
						m_eEvalStage = SPH_EVAL_PRESORT;
						return new Expr_GetZonespanlist_c ();
					case FUNC_TO_STRING:
						return new Expr_ToString_c ( dArgs[0], m_dNodes [ tNode.m_iLeft ].m_eRetType );
					case FUNC_RANKFACTORS:
						m_eEvalStage = SPH_EVAL_PRESORT;
						return new Expr_GetRankFactors_c();
					case FUNC_PACKEDFACTORS:
						m_bHasPackedFactors = true;
						m_eEvalStage = SPH_EVAL_FINAL;
						return new Expr_GetPackedFactors_c();
				}
				assert ( 0 && "unhandled function id" );
				break;
			}

		case TOK_UDF:			return CreateUdfNode ( tNode.m_iFunc, pLeft ); break;
		case TOK_HOOK_IDENT:	return m_pHook->CreateNode ( tNode.m_iFunc, NULL, NULL ); break;
		case TOK_HOOK_FUNC:		return m_pHook->CreateNode ( tNode.m_iFunc, pLeft, &m_eEvalStage ); break;
		case TOK_CONST_HASH:
			// tricky bit
			// data gets moved (!) from node to ISphExpr at this point
			return new Expr_ConstHash_c ( tNode.m_pConsthash->m_dPairs );
			break;
		default:				assert ( 0 && "unhandled token type" ); break;
	}

#undef LOC_SPAWN_POLY

	// fire exit
	SafeRelease ( pLeft );
	SafeRelease ( pRight );
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

/// arg-vs-set function (currently, IN or INTERVAL) evaluator traits
template < typename T >
class Expr_ArgVsSet_c : public ISphExpr
{
protected:
	ISphExpr *			m_pArg;

public:
	explicit Expr_ArgVsSet_c ( ISphExpr * pArg ) : m_pArg ( pArg ) {}
	~Expr_ArgVsSet_c () { SafeRelease ( m_pArg ); }

	virtual int IntEval ( const CSphMatch & tMatch ) const = 0;
	virtual float Eval ( const CSphMatch & tMatch ) const { return (float) IntEval ( tMatch ); }
	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const { return IntEval ( tMatch ); }

protected:
	T ExprEval ( ISphExpr * pArg, const CSphMatch & tMatch ) const;
};

template<> int Expr_ArgVsSet_c<int>::ExprEval ( ISphExpr * pArg, const CSphMatch & tMatch ) const
{
	return pArg->IntEval ( tMatch );
}

template<> DWORD Expr_ArgVsSet_c<DWORD>::ExprEval ( ISphExpr * pArg, const CSphMatch & tMatch ) const
{
	return (DWORD)pArg->IntEval ( tMatch );
}

template<> float Expr_ArgVsSet_c<float>::ExprEval ( ISphExpr * pArg, const CSphMatch & tMatch ) const
{
	return pArg->Eval ( tMatch );
}

template<> int64_t Expr_ArgVsSet_c<int64_t>::ExprEval ( ISphExpr * pArg, const CSphMatch & tMatch ) const
{
	return pArg->Int64Eval ( tMatch );
}


/// arg-vs-constant-set
template < typename T >
class Expr_ArgVsConstSet_c : public Expr_ArgVsSet_c<T>
{
protected:
	CSphVector<T> m_dValues;

public:
	/// take ownership of arg, pre-evaluate and dismiss turn points
	Expr_ArgVsConstSet_c ( ISphExpr * pArg, CSphVector<ISphExpr *> & dArgs, int iSkip )
		: Expr_ArgVsSet_c<T> ( pArg )
	{
		CSphMatch tDummy;
		for ( int i=iSkip; i<dArgs.GetLength(); i++ )
		{
			m_dValues.Add ( Expr_ArgVsSet_c<T>::ExprEval ( dArgs[i], tDummy ) );
			SafeRelease ( dArgs[i] );
		}
	}

	/// take ownership of arg, and copy that constlist
	Expr_ArgVsConstSet_c ( ISphExpr * pArg, ConstList_c * pConsts )
		: Expr_ArgVsSet_c<T> ( pArg )
	{
		if ( !pConsts )
			return; // can happen on uservar path
		if ( pConsts->m_eRetType==SPH_ATTR_FLOAT )
		{
			m_dValues.Reserve ( pConsts->m_dFloats.GetLength() );
			ARRAY_FOREACH ( i, pConsts->m_dFloats )
				m_dValues.Add ( (T)pConsts->m_dFloats[i] );
		} else
		{
			m_dValues.Reserve ( pConsts->m_dInts.GetLength() );
			ARRAY_FOREACH ( i, pConsts->m_dInts )
				m_dValues.Add ( (T)pConsts->m_dInts[i] );
		}
	}
};

//////////////////////////////////////////////////////////////////////////

/// INTERVAL() evaluator for constant turn point values case
template < typename T >
class Expr_IntervalConst_c : public Expr_ArgVsConstSet_c<T>
{
public:
	/// take ownership of arg, pre-evaluate and dismiss turn points
	explicit Expr_IntervalConst_c ( CSphVector<ISphExpr *> & dArgs )
		: Expr_ArgVsConstSet_c<T> ( dArgs[0], dArgs, 1 )
	{}

	/// evaluate arg, return interval id
	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		T val = this->ExprEval ( this->m_pArg, tMatch ); // 'this' fixes gcc braindamage
		ARRAY_FOREACH ( i, this->m_dValues ) // FIXME! OPTIMIZE! perform binary search here
			if ( val<this->m_dValues[i] )
				return i;
		return this->m_dValues.GetLength();
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg ) { this->m_pArg->Command ( eCmd, pArg ); }
};


/// generic INTERVAL() evaluator
template < typename T >
class Expr_Interval_c : public Expr_ArgVsSet_c<T>
{
protected:
	CSphVector<ISphExpr *> m_dTurnPoints;

public:
	/// take ownership of arg and turn points
	explicit Expr_Interval_c ( const CSphVector<ISphExpr *> & dArgs )
		: Expr_ArgVsSet_c<T> ( dArgs[0] )
	{
		for ( int i=1; i<dArgs.GetLength(); i++ )
			m_dTurnPoints.Add ( dArgs[i] );
	}

	/// evaluate arg, return interval id
	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		T val = this->ExprEval ( this->m_pArg, tMatch ); // 'this' fixes gcc braindamage
		ARRAY_FOREACH ( i, m_dTurnPoints )
			if ( val < Expr_ArgVsSet_c<T>::ExprEval ( m_dTurnPoints[i], tMatch ) )
				return i;
		return m_dTurnPoints.GetLength();
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		this->m_pArg->Command ( eCmd, pArg );
		ARRAY_FOREACH ( i, m_dTurnPoints )
			m_dTurnPoints[i]->Command ( eCmd, pArg );
	}
};

//////////////////////////////////////////////////////////////////////////

/// IN() evaluator, arbitrary scalar expression vs. constant values
template < typename T >
class Expr_In_c : public Expr_ArgVsConstSet_c<T>
{
public:
	/// pre-sort values for binary search
	Expr_In_c ( ISphExpr * pArg, ConstList_c * pConsts ) :
		Expr_ArgVsConstSet_c<T> ( pArg, pConsts )
	{
		this->m_dValues.Sort();
	}

	/// evaluate arg, check if the value is within set
	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		T val = this->ExprEval ( this->m_pArg, tMatch ); // 'this' fixes gcc braindamage
		return this->m_dValues.BinarySearch ( val )!=NULL;
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg ) { this->m_pArg->Command ( eCmd, pArg ); }
};


/// IN() evaluator, arbitrary scalar expression vs. uservar
/// (for the sake of evaluator, uservar is a pre-sorted, refcounted external vector)
class Expr_InUservar_c : public Expr_ArgVsSet_c<int64_t>
{
protected:
	UservarIntSet_c * m_pConsts;

public:
	/// just get hold of args
	explicit Expr_InUservar_c ( ISphExpr * pArg, UservarIntSet_c * pConsts )
		: Expr_ArgVsSet_c<int64_t> ( pArg )
		, m_pConsts ( pConsts ) // no addref, hook should have addref'd (otherwise there'd be a race)
	{}

	/// release the uservar value
	~Expr_InUservar_c()
	{
		SafeRelease ( m_pConsts );
	}

	/// evaluate arg, check if the value is within set
	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		int64_t iVal = ExprEval ( this->m_pArg, tMatch ); // 'this' fixes gcc braindamage
		return m_pConsts->BinarySearch ( iVal )!=NULL;
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg ) { this->m_pArg->Command ( eCmd, pArg ); }
};


/// IN() evaluator, MVA attribute vs. constant values
template < bool MVA64 >
class Expr_MVAIn_c : public Expr_ArgVsConstSet_c<int64_t>
{
public:
	/// pre-sort values for binary search
	Expr_MVAIn_c ( const CSphAttrLocator & tLoc, int iLocator, ConstList_c * pConsts, UservarIntSet_c * pUservar )
		: Expr_ArgVsConstSet_c<int64_t> ( NULL, pConsts )
		, m_tLocator ( tLoc )
		, m_iLocator ( iLocator )
		, m_pMvaPool ( NULL )
		, m_pUservar ( pUservar )
	{
		assert ( tLoc.m_iBitOffset>=0 && tLoc.m_iBitCount>0 );
		assert ( !pConsts || !pUservar ); // either constlist or uservar, not both
		this->m_dValues.Sort();
	}

	~Expr_MVAIn_c()
	{
		SafeRelease ( m_pUservar );
	}

	int MvaEval ( const DWORD * pMva ) const;

	/// evaluate arg, check if any values are within set
	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		const DWORD * pMva = tMatch.GetAttrMVA ( m_tLocator, m_pMvaPool );
		if ( !pMva )
			return 0;

		return MvaEval ( pMva );
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		if ( eCmd==SPH_EXPR_SET_MVA_POOL )
			m_pMvaPool = (const DWORD*)pArg;
		if ( eCmd==SPH_EXPR_GET_DEPENDENT_COLS )
			static_cast < CSphVector<int>* > ( pArg )->Add ( m_iLocator );
	}

protected:
	CSphAttrLocator		m_tLocator;
	int					m_iLocator;
	const DWORD *		m_pMvaPool;
	UservarIntSet_c *	m_pUservar;
};


template<>
int Expr_MVAIn_c<false>::MvaEval ( const DWORD * pMva ) const
{
	// OPTIMIZE! FIXME! factor out a common function with Filter_MVAValues::Eval()
	DWORD uLen = *pMva++;
	const DWORD * pMvaMax = pMva+uLen;

	const int64_t * pFilter = m_pUservar ? m_pUservar->Begin() : m_dValues.Begin();
	const int64_t * pFilterMax = pFilter + ( m_pUservar ? m_pUservar->GetLength() : m_dValues.GetLength() );

	const DWORD * L = pMva;
	const DWORD * R = pMvaMax - 1;
	for ( ; pFilter < pFilterMax; pFilter++ )
	{
		while ( L<=R )
		{
			const DWORD * m = L + (R - L) / 2;

			if ( *pFilter > *m )
				L = m + 1;
			else if ( *pFilter < *m )
				R = m - 1;
			else
				return 1;
		}
		R = pMvaMax - 1;
	}
	return 0;
}


template<>
int Expr_MVAIn_c<true>::MvaEval ( const DWORD * pMva ) const
{
	// OPTIMIZE! FIXME! factor out a common function with Filter_MVAValues::Eval()
	DWORD uLen = *pMva++;
	assert ( ( uLen%2 )==0 );
	const DWORD * pMvaMax = pMva+uLen;

	const int64_t * pFilter = m_pUservar ? m_pUservar->Begin() : m_dValues.Begin();
	const int64_t * pFilterMax = pFilter + ( m_pUservar ? m_pUservar->GetLength() : m_dValues.GetLength() );

	const int64_t * L = (const int64_t *)pMva;
	const int64_t * R = (const int64_t *)( pMvaMax - 2 );
	for ( ; pFilter < pFilterMax; pFilter++ )
	{
		while ( L<=R )
		{
			const int64_t * pVal = L + (R - L) / 2;
			int64_t iMva = MVA_UPSIZE ( (const DWORD *)pVal );

			if ( *pFilter > iMva )
				L = pVal + 1;
			else if ( *pFilter < iMva )
				R = pVal - 1;
			else
				return 1;
		}
		R = (const int64_t *) ( pMvaMax - 2 );
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////

/// generic BITDOT() evaluator
template < typename T >
class Expr_Bitdot_c : public Expr_ArgVsSet_c<T>
{
protected:
	CSphVector<ISphExpr *> m_dBitWeights;

public:
	/// take ownership of arg and turn points
	explicit Expr_Bitdot_c ( const CSphVector<ISphExpr *> & dArgs )
		: Expr_ArgVsSet_c<T> ( dArgs[0] )
	{
		for ( int i=1; i<dArgs.GetLength(); i++ )
			m_dBitWeights.Add ( dArgs[i] );
	}

protected:
	/// generic evaluate
	virtual T DoEval ( const CSphMatch & tMatch ) const
	{
		int64_t uArg = this->m_pArg->Int64Eval ( tMatch ); // 'this' fixes gcc braindamage
		T tRes = 0;

		int iBit = 0;
		while ( uArg && iBit<m_dBitWeights.GetLength() )
		{
			if ( uArg & 1 )
				tRes += Expr_ArgVsSet_c<T>::ExprEval ( m_dBitWeights[iBit], tMatch );
			uArg >>= 1;
			iBit++;
		}

		return tRes;
	}

public:
	virtual float Eval ( const CSphMatch & tMatch ) const
	{
		return (float) DoEval ( tMatch );
	}

	virtual int IntEval ( const CSphMatch & tMatch ) const
	{
		return (int) DoEval ( tMatch );
	}

	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const
	{
		return (int64_t) DoEval ( tMatch );
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		this->m_pArg->Command ( eCmd, pArg );
		ARRAY_FOREACH ( i, m_dBitWeights )
			m_dBitWeights[i]->Command ( eCmd, pArg );
	}
};

//////////////////////////////////////////////////////////////////////////

/// geodist() - attr point, constant anchor
class Expr_GeodistAttrConst_c: public ISphExpr
{
public:
	Expr_GeodistAttrConst_c ( CSphAttrLocator tLat, CSphAttrLocator tLon, float fAnchorLat, float fAnchorLon, int iLat, int iLon )
		: m_tLat ( tLat )
		, m_tLon ( tLon )
		, m_fAnchorLat ( fAnchorLat )
		, m_fAnchorLon ( fAnchorLon )
		, m_iLat ( iLat )
		, m_iLon ( iLon )
	{}

	virtual float Eval ( const CSphMatch & tMatch ) const
	{
		return CalcGeodist ( tMatch.GetAttrFloat ( m_tLat ), tMatch.GetAttrFloat ( m_tLon ), m_fAnchorLat, m_fAnchorLon );
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg ) const
	{
		if ( eCmd==SPH_EXPR_GET_DEPENDENT_COLS )
		{
			static_cast < CSphVector<int>* > ( pArg )->Add ( m_iLat );
			static_cast < CSphVector<int>* > ( pArg )->Add ( m_iLon );
		}
	}

private:
	CSphAttrLocator	m_tLat;
	CSphAttrLocator	m_tLon;

	float		m_fAnchorLat;
	float		m_fAnchorLon;

	int			m_iLat;
	int			m_iLon;
};

/// geodist() - expr point, constant anchor
class Expr_GeodistConst_c: public ISphExpr
{
public:
	Expr_GeodistConst_c ( ISphExpr * pLat, ISphExpr * pLon, float fAnchorLat, float fAnchorLon )
		: m_pLat ( pLat )
		, m_pLon ( pLon )
		, m_fAnchorLat ( fAnchorLat )
		, m_fAnchorLon ( fAnchorLon )
	{}

	~Expr_GeodistConst_c ()
	{
		SafeRelease ( m_pLon );
		SafeRelease ( m_pLat );
	}

	virtual float Eval ( const CSphMatch & tMatch ) const
	{
		return CalcGeodist ( m_pLat->Eval(tMatch), m_pLon->Eval(tMatch), m_fAnchorLat, m_fAnchorLon );
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		m_pLat->Command ( eCmd, pArg );
		m_pLon->Command ( eCmd, pArg );
	}

private:
	ISphExpr *	m_pLat;
	ISphExpr *	m_pLon;

	float		m_fAnchorLat;
	float		m_fAnchorLon;
};

/// geodist() - expr point, expr anchor
class Expr_Geodist_c: public ISphExpr
{
public:
	Expr_Geodist_c ( ISphExpr * pLat, ISphExpr * pLon, ISphExpr * pAnchorLat, ISphExpr * pAnchorLon )
		: m_pLat ( pLat )
		, m_pLon ( pLon )
		, m_pAnchorLat ( pAnchorLat )
		, m_pAnchorLon ( pAnchorLon )
	{}

	~Expr_Geodist_c ()
	{
		SafeRelease ( m_pAnchorLon );
		SafeRelease ( m_pAnchorLat );
		SafeRelease ( m_pLon );
		SafeRelease ( m_pLat );
	}

	virtual float Eval ( const CSphMatch & tMatch ) const
	{
		return CalcGeodist ( m_pLat->Eval(tMatch), m_pLon->Eval(tMatch), m_pAnchorLat->Eval(tMatch), m_pAnchorLon->Eval(tMatch) );
	}

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		m_pLat->Command ( eCmd, pArg );
		m_pLon->Command ( eCmd, pArg );
		m_pAnchorLat->Command ( eCmd, pArg );
		m_pAnchorLon->Command ( eCmd, pArg );
	}

private:
	ISphExpr *	m_pLat;
	ISphExpr *	m_pLon;

	ISphExpr *	m_pAnchorLat;
	ISphExpr *	m_pAnchorLon;
};

//////////////////////////////////////////////////////////////////////////

void ExprParser_t::GatherArgTypes ( int iNode, CSphVector<int> & dTypes )
{
	if ( iNode<0 )
		return;

	const ExprNode_t & tNode = m_dNodes[iNode];
	if ( tNode.m_iToken==',' )
	{
		GatherArgTypes ( tNode.m_iLeft, dTypes );
		GatherArgTypes ( tNode.m_iRight, dTypes );
	} else
	{
		dTypes.Add ( tNode.m_iToken );
	}
}

void ExprParser_t::GatherArgNodes ( int iNode, CSphVector<int> & dNodes )
{
	if ( iNode<0 )
		return;

	const ExprNode_t & tNode = m_dNodes[iNode];
	if ( tNode.m_iToken==',' )
	{
		GatherArgNodes ( tNode.m_iLeft, dNodes );
		GatherArgNodes ( tNode.m_iRight, dNodes );
	} else
		dNodes.Add ( iNode );
}

void ExprParser_t::GatherArgRetTypes ( int iNode, CSphVector<ESphAttr> & dTypes )
{
	if ( iNode<0 )
		return;

	const ExprNode_t & tNode = m_dNodes[iNode];
	if ( tNode.m_iToken==',' )
	{
		GatherArgRetTypes ( tNode.m_iLeft, dTypes );
		GatherArgRetTypes ( tNode.m_iRight, dTypes );
	} else
	{
		dTypes.Add ( tNode.m_eRetType );
	}
}

bool ExprParser_t::CheckForConstSet ( int iArgsNode, int iSkip )
{
	CSphVector<int> dTypes;
	GatherArgTypes ( iArgsNode, dTypes );

	for ( int i=iSkip; i<dTypes.GetLength(); i++ )
		if ( dTypes[i]!=TOK_CONST_INT && dTypes[i]!=TOK_CONST_FLOAT && dTypes[i]!=TOK_CONST_HASH )
			return false;
	return true;
}


template < typename T >
void ExprParser_t::WalkTree ( int iRoot, T & FUNCTOR )
{
	if ( iRoot>=0 )
	{
		const ExprNode_t & tNode = m_dNodes[iRoot];
		FUNCTOR.Enter ( tNode, m_dNodes );
		WalkTree ( tNode.m_iLeft, FUNCTOR );
		WalkTree ( tNode.m_iRight, FUNCTOR );
		FUNCTOR.Exit ( tNode );
	}
}


ISphExpr * ExprParser_t::CreateIntervalNode ( int iArgsNode, CSphVector<ISphExpr *> & dArgs )
{
	assert ( dArgs.GetLength()>=2 );

	bool bConst = CheckForConstSet ( iArgsNode, 1 );
	ESphAttr eAttrType = m_dNodes[iArgsNode].m_eArgType;
	if ( bConst )
	{
		switch ( eAttrType )
		{
			case SPH_ATTR_INTEGER:	return new Expr_IntervalConst_c<int> ( dArgs ); break;
			case SPH_ATTR_BIGINT:	return new Expr_IntervalConst_c<int64_t> ( dArgs ); break;
			default:				return new Expr_IntervalConst_c<float> ( dArgs ); break;
		}
	} else
	{
		switch ( eAttrType )
		{
			case SPH_ATTR_INTEGER:	return new Expr_Interval_c<int> ( dArgs ); break;
			case SPH_ATTR_BIGINT:	return new Expr_Interval_c<int64_t> ( dArgs ); break;
			default:				return new Expr_Interval_c<float> ( dArgs ); break;
		}
	}
}


ISphExpr * ExprParser_t::CreateInNode ( int iNode )
{
	const ExprNode_t & tLeft = m_dNodes[m_dNodes[iNode].m_iLeft];
	const ExprNode_t & tRight = m_dNodes[m_dNodes[iNode].m_iRight];

	switch ( tRight.m_iToken )
	{
		// create IN(arg,constlist)
		case TOK_CONST_LIST:
			switch ( tLeft.m_iToken )
			{
				case TOK_ATTR_MVA32:
					return new Expr_MVAIn_c<false> ( tLeft.m_tLocator, tLeft.m_iLocator, tRight.m_pConsts, NULL );
				case TOK_ATTR_MVA64:
					return new Expr_MVAIn_c<true> ( tLeft.m_tLocator, tLeft.m_iLocator, tRight.m_pConsts, NULL );
				default:
				{
					ISphExpr * pArg = CreateTree ( m_dNodes[iNode].m_iLeft );
					switch ( tRight.m_pConsts->m_eRetType )
					{
						case SPH_ATTR_INTEGER:	return new Expr_In_c<int> ( pArg, tRight.m_pConsts ); break;
						case SPH_ATTR_BIGINT:	return new Expr_In_c<int64_t> ( pArg, tRight.m_pConsts ); break;
						default:				return new Expr_In_c<float> ( pArg, tRight.m_pConsts ); break;
					}
				}
			}
			break;

		// create IN(arg,uservar)
		case TOK_USERVAR:
		{
			if ( !g_pUservarsHook )
			{
				m_sCreateError.SetSprintf ( "internal error: no uservars hook" );
				return NULL;
			}

			UservarIntSet_c * pUservar = g_pUservarsHook ( m_dUservars[(int)tRight.m_iConst] );
			if ( !pUservar )
			{
				m_sCreateError.SetSprintf ( "undefined user variable '%s'", m_dUservars[(int)tRight.m_iConst].cstr() );
				return NULL;
			}

			switch ( tLeft.m_iToken )
			{
				case TOK_ATTR_MVA32:
					return new Expr_MVAIn_c<false> ( tLeft.m_tLocator, tLeft.m_iLocator, NULL, pUservar );
				case TOK_ATTR_MVA64:
					return new Expr_MVAIn_c<true> ( tLeft.m_tLocator, tLeft.m_iLocator, NULL, pUservar );
				default:
					return new Expr_InUservar_c ( CreateTree ( m_dNodes[iNode].m_iLeft ), pUservar );
			}
			break;
		}

		// oops, unhandled case
		default:
			m_sCreateError = "IN() arguments must be constants (except the 1st one)";
			return NULL;
	}
}


ISphExpr * ExprParser_t::CreateGeodistNode ( int iArgs )
{
	CSphVector<int> dArgs;
	GatherArgNodes ( iArgs, dArgs );
	assert ( dArgs.GetLength()==4 );

	bool bConst1 = ( IsConst ( &m_dNodes[dArgs[0]] ) && IsConst ( &m_dNodes[dArgs[1]] ) );
	bool bConst2 = ( IsConst ( &m_dNodes[dArgs[2]] ) && IsConst ( &m_dNodes[dArgs[3]] ) );

	if ( bConst1 && bConst2 )
	{
		return new Expr_GetConst_c ( CalcGeodist (
			m_dNodes[dArgs[0]].FloatVal(), m_dNodes[dArgs[1]].FloatVal(),
			m_dNodes[dArgs[2]].FloatVal(), m_dNodes[dArgs[3]].FloatVal() ) );
	}

	if ( bConst1 )
	{
		Swap ( dArgs[0], dArgs[2] );
		Swap ( dArgs[1], dArgs[3] );
		Swap ( bConst1, bConst2 );
	}

	if ( bConst2 )
	{
		// constant anchor
		if ( m_dNodes[dArgs[0]].m_iToken==TOK_ATTR_FLOAT && m_dNodes[dArgs[1]].m_iToken==TOK_ATTR_FLOAT )
		{
			// attr point
			return new Expr_GeodistAttrConst_c (
				m_dNodes[dArgs[0]].m_tLocator, m_dNodes[dArgs[1]].m_tLocator,
				m_dNodes[dArgs[2]].FloatVal(), m_dNodes[dArgs[3]].FloatVal(),
				m_dNodes[dArgs[0]].m_iLocator, m_dNodes[dArgs[1]].m_iLocator );
		} else
		{
			// expr point
			return new Expr_GeodistConst_c (
				CreateTree ( dArgs[0] ), CreateTree ( dArgs[1] ),
				m_dNodes[dArgs[2]].FloatVal(), m_dNodes[dArgs[3]].FloatVal() );
		}
	}

	// four expressions
	CSphVector<ISphExpr *> dExpr;
	FoldArglist ( CreateTree ( iArgs ), dExpr );
	assert ( dExpr.GetLength()==4 );
	return new Expr_Geodist_c ( dExpr[0], dExpr[1], dExpr[2], dExpr[3] );
}


ISphExpr * ExprParser_t::CreateBitdotNode ( int iArgsNode, CSphVector<ISphExpr *> & dArgs )
{
	assert ( dArgs.GetLength()>=1 );

	ESphAttr eAttrType = m_dNodes[iArgsNode].m_eRetType;
	switch ( eAttrType )
	{
		case SPH_ATTR_INTEGER:	return new Expr_Bitdot_c<int> ( dArgs ); break;
		case SPH_ATTR_BIGINT:	return new Expr_Bitdot_c<int64_t> ( dArgs ); break;
		default:				return new Expr_Bitdot_c<float> ( dArgs ); break;
	}
}

//////////////////////////////////////////////////////////////////////////

int yylex ( YYSTYPE * lvalp, ExprParser_t * pParser )
{
	return pParser->GetToken ( lvalp );
}

void yyerror ( ExprParser_t * pParser, const char * sMessage )
{
	pParser->m_sParserError.SetSprintf ( "Sphinx expr: %s near '%s'", sMessage, pParser->m_pLastTokenStart );
}

#if USE_WINDOWS
#pragma warning(push,1)
#endif

#include "yysphinxexpr.c"

#if USE_WINDOWS
#pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////

ExprParser_t::~ExprParser_t ()
{
	// i kinda own those constlists
	ARRAY_FOREACH ( i, m_dNodes )
		if ( m_dNodes[i].m_iToken==TOK_CONST_LIST )
			SafeDelete ( m_dNodes[i].m_pConsts );

	// free any UDF calls that weren't taken over
	ARRAY_FOREACH ( i, m_dUdfCalls )
		SafeDelete ( m_dUdfCalls[i] );

	// free temp const hash storage
	ARRAY_FOREACH ( i, m_dIdents )
		SafeDeleteArray ( m_dIdents[i] );
}

ESphAttr ExprParser_t::GetWidestRet ( int iLeft, int iRight )
{
	ESphAttr uLeftType = ( iLeft<0 ) ? SPH_ATTR_INTEGER : m_dNodes[iLeft].m_eRetType;
	ESphAttr uRightType = ( iRight<0 ) ? SPH_ATTR_INTEGER : m_dNodes[iRight].m_eRetType;

	ESphAttr uRes = SPH_ATTR_FLOAT; // default is float
	if ( ( uLeftType==SPH_ATTR_INTEGER || uLeftType==SPH_ATTR_BIGINT ) &&
		( uRightType==SPH_ATTR_INTEGER || uRightType==SPH_ATTR_BIGINT ) )
	{
		// both types are integer (int32 or int64), compute in integers
		uRes = ( uLeftType==SPH_ATTR_INTEGER && uRightType==SPH_ATTR_INTEGER )
			? SPH_ATTR_INTEGER
			: SPH_ATTR_BIGINT;
	}
	return uRes;
}

int ExprParser_t::AddNodeInt ( int64_t iValue )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_CONST_INT;
	tNode.m_eRetType = GetIntType ( iValue );
	tNode.m_iConst = iValue;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeFloat ( float fValue )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_CONST_FLOAT;
	tNode.m_eRetType = SPH_ATTR_FLOAT;
	tNode.m_fConst = fValue;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeString ( int64_t iValue )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_CONST_STRING;
	tNode.m_eRetType = SPH_ATTR_STRING;
	tNode.m_iConst = iValue;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeAttr ( int iTokenType, uint64_t uAttrLocator )
{
	assert ( iTokenType==TOK_ATTR_INT || iTokenType==TOK_ATTR_BITS || iTokenType==TOK_ATTR_FLOAT
		|| iTokenType==TOK_ATTR_MVA32 || iTokenType==TOK_ATTR_MVA64 || iTokenType==TOK_ATTR_STRING
		|| iTokenType==TOK_ATTR_FACTORS );
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = iTokenType;
	sphUnpackAttrLocator ( uAttrLocator, &tNode );

	if ( iTokenType==TOK_ATTR_FLOAT )			tNode.m_eRetType = SPH_ATTR_FLOAT;
	else if ( iTokenType==TOK_ATTR_MVA32 )		tNode.m_eRetType = SPH_ATTR_UINT32SET;
	else if ( iTokenType==TOK_ATTR_MVA64 )		tNode.m_eRetType = SPH_ATTR_INT64SET;
	else if ( iTokenType==TOK_ATTR_STRING )		tNode.m_eRetType = SPH_ATTR_STRING;
	else if ( iTokenType==TOK_ATTR_FACTORS )	tNode.m_eRetType = SPH_ATTR_FACTORS;
	else if ( tNode.m_tLocator.m_iBitCount>32 )	tNode.m_eRetType = SPH_ATTR_BIGINT;
	else										tNode.m_eRetType = SPH_ATTR_INTEGER;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeID ()
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_ID;
	tNode.m_eRetType = USE_64BIT ? SPH_ATTR_BIGINT : SPH_ATTR_INTEGER;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeWeight ()
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_WEIGHT;
	tNode.m_eRetType = SPH_ATTR_INTEGER;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeGroupby ()
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_GROUPBY;
	tNode.m_eRetType = SPH_ATTR_INTEGER; /// will be corrected later, using context.
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeOp ( int iOp, int iLeft, int iRight )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = iOp;

	// deduce type
	tNode.m_eRetType = SPH_ATTR_FLOAT; // default to float
	if ( iOp==TOK_NEG )
	{
		// special case, NEG(const)
		ExprNode_t & tArg = m_dNodes[iLeft];
		if ( tArg.m_iToken==TOK_CONST_INT || tArg.m_iToken==TOK_CONST_FLOAT )
		{
			if ( tArg.m_iToken==TOK_CONST_INT )
				tArg.m_iConst = -tArg.m_iConst;
			else
				tArg.m_fConst = -tArg.m_fConst;
			m_dNodes.Pop();
			return iLeft;
		}

		// NEG just inherits the type
		tNode.m_eArgType = m_dNodes[iLeft].m_eRetType;
		tNode.m_eRetType = tNode.m_eArgType;

	} else if ( iOp==TOK_NOT )
	{
		// NOT result is integer, and its argument must be integer
		tNode.m_eArgType = m_dNodes[iLeft].m_eRetType;
		tNode.m_eRetType = SPH_ATTR_INTEGER;
		if (!( tNode.m_eArgType==SPH_ATTR_INTEGER || tNode.m_eArgType==SPH_ATTR_BIGINT ))
		{
			m_sParserError.SetSprintf ( "NOT argument must be integer" );
			return -1;
		}

	} else if ( iOp==TOK_LTE || iOp==TOK_GTE || iOp==TOK_EQ || iOp==TOK_NE
		|| iOp=='<' || iOp=='>' || iOp==TOK_AND || iOp==TOK_OR
		|| iOp=='+' || iOp=='-' || iOp=='*' || iOp==','
		|| iOp=='&' || iOp=='|' || iOp=='%' )
	{
		tNode.m_eArgType = GetWidestRet ( iLeft, iRight );

		// arithmetical operations return arg type, logical return int
		tNode.m_eRetType = ( iOp=='+' || iOp=='-' || iOp=='*' || iOp==',' || iOp=='&' || iOp=='|' || iOp=='%' )
			? tNode.m_eArgType
			: SPH_ATTR_INTEGER;

		// both logical and bitwise AND/OR can only be over ints
		if ( ( iOp==TOK_AND || iOp==TOK_OR || iOp=='&' || iOp=='|' )
			&& !( tNode.m_eArgType==SPH_ATTR_INTEGER || tNode.m_eArgType==SPH_ATTR_BIGINT ))
		{
			m_sParserError.SetSprintf ( "%s arguments must be integer", ( iOp==TOK_AND || iOp=='&' ) ? "AND" : "OR" );
			return -1;
		}

		// MOD can only be over ints
		if ( iOp=='%'
			&& !( tNode.m_eArgType==SPH_ATTR_INTEGER || tNode.m_eArgType==SPH_ATTR_BIGINT ))
		{
			m_sParserError.SetSprintf ( "MOD arguments must be integer" );
			return -1;
		}

	} else
	{
		// check for unknown op
		assert ( iOp=='/' && "unknown op in AddNodeOp() type deducer" );
	}

	tNode.m_iArgs = 0;
	if ( iOp==',' )
	{
		if ( iLeft>=0 )		tNode.m_iArgs += ( m_dNodes[iLeft].m_iToken==',' ) ? m_dNodes[iLeft].m_iArgs : 1;
		if ( iRight>=0 )	tNode.m_iArgs += ( m_dNodes[iRight].m_iToken==',' ) ? m_dNodes[iRight].m_iArgs : 1;
	}
	tNode.m_iLeft = iLeft;
	tNode.m_iRight = iRight;
	return m_dNodes.GetLength()-1;
}


int ExprParser_t::AddNodeFunc ( int iFunc, int iLeft, int iRight )
{
	// regular case, iLeft is entire arglist, iRight is -1
	// special case for IN(), iLeft is arg, iRight is constlist
	assert ( iFunc>=0 && iFunc<int(sizeof(g_dFuncs)/sizeof(g_dFuncs[0])) );
	Func_e eFunc = g_dFuncs[iFunc].m_eFunc;

	// check args count
	if ( iRight<0 || eFunc==FUNC_IN )
	{
		int iExpectedArgc = g_dFuncs[iFunc].m_iArgs;
		int iArgc = 0;
		if ( iLeft>=0 )
			iArgc = ( m_dNodes[iLeft].m_iToken==',' ) ? m_dNodes[iLeft].m_iArgs : 1;
		if ( iExpectedArgc<0 )
		{
			if ( iArgc<-iExpectedArgc )
			{
				m_sParserError.SetSprintf ( "%s() called with %d args, at least %d args expected", g_dFuncs[iFunc].m_sName, iArgc, -iExpectedArgc );
				return -1;
			}
		} else if ( iArgc!=iExpectedArgc )
		{
			m_sParserError.SetSprintf ( "%s() called with %d args, %d args expected", g_dFuncs[iFunc].m_sName, iArgc, iExpectedArgc );
			return -1;
		}
	}

	// check arg types
	//
	// check for string args
	// most builtin functions take numeric args only
	bool bGotString = false, bGotMva = false;
	CSphVector<ESphAttr> dRetTypes;
	if ( iRight<0 )
	{
		GatherArgRetTypes ( iLeft, dRetTypes );
		ARRAY_FOREACH ( i, dRetTypes )
		{
			bGotString |= ( dRetTypes[i]==SPH_ATTR_STRING );
			bGotMva |= ( dRetTypes[i]==SPH_ATTR_UINT32SET || dRetTypes[i]==SPH_ATTR_UINT32SET );
		}
	}
	if ( bGotString && !( eFunc==FUNC_CRC32 || eFunc==FUNC_EXIST || eFunc==FUNC_POLY2D || eFunc==FUNC_GEOPOLY2D ) )
	{
		m_sParserError.SetSprintf ( "%s() arguments can not be string", g_dFuncs[iFunc].m_sName );
		return -1;
	}
	if ( bGotMva && !(eFunc==FUNC_IN || eFunc==FUNC_TO_STRING ) )
	{
		m_sParserError.SetSprintf ( "%s() arguments can not be MVA", g_dFuncs[iFunc].m_sName );
		return -1;
	}

	// check that first BITDOT arg is integer or bigint
	if ( eFunc==FUNC_BITDOT )
	{
		int iLeftmost = iLeft;
		while ( m_dNodes[iLeftmost].m_iToken==',' )
			iLeftmost = m_dNodes[iLeftmost].m_iLeft;

		ESphAttr eArg = m_dNodes[iLeftmost].m_eRetType;
		if ( eArg!=SPH_ATTR_INTEGER && eArg!=SPH_ATTR_BIGINT )
		{
			m_sParserError.SetSprintf ( "first BITDOT() argument must be integer" );
			return -1;
		}
	}

	if ( eFunc==FUNC_EXIST )
	{
		int iExistLeft = m_dNodes[iLeft].m_iLeft;
		int iExistRight = m_dNodes[iLeft].m_iRight;
		bool bIsLeftGood = ( m_dNodes[iExistLeft].m_eRetType==SPH_ATTR_STRING );
		ESphAttr eRight = m_dNodes[iExistRight].m_eRetType;
		bool bIsRightGood = ( eRight==SPH_ATTR_INTEGER || eRight==SPH_ATTR_TIMESTAMP || eRight==SPH_ATTR_ORDINAL || eRight==SPH_ATTR_BOOL
			|| eRight==SPH_ATTR_FLOAT || eRight==SPH_ATTR_BIGINT || eRight==SPH_ATTR_WORDCOUNT );

		if ( !bIsLeftGood || !bIsRightGood )
		{
			if ( bIsRightGood )
				m_sParserError.SetSprintf ( "first EXIST() argument must be string" );
			else
				m_sParserError.SetSprintf ( "ill-formed EXIST" );
			return -1;
		}
	}


	// check that first SINT or timestamp family arg is integer
	if ( eFunc==FUNC_SINT || eFunc==FUNC_DAY || eFunc==FUNC_MONTH || eFunc==FUNC_YEAR || eFunc==FUNC_YEARMONTH || eFunc==FUNC_YEARMONTHDAY
		|| eFunc==FUNC_FIBONACCI )
	{
		assert ( iLeft>=0 );
		if ( m_dNodes[iLeft].m_eRetType!=SPH_ATTR_INTEGER )
		{
			m_sParserError.SetSprintf ( "%s() argument must be integer", g_dFuncs[iFunc].m_sName );
			return -1;
		}
	}

	// check that CONTAINS args are poly, float, float
	if ( eFunc==FUNC_CONTAINS )
	{
		assert ( dRetTypes.GetLength()==3 );
		if ( dRetTypes[0]!=SPH_ATTR_POLY2D )
		{
			m_sParserError.SetSprintf ( "1st CONTAINS() argument must be a 2D polygon (see POLY2D)" );
			return -1;
		}
		if ( !IsNumeric ( dRetTypes[1] ) || !IsNumeric ( dRetTypes[2] ) )
		{
			m_sParserError.SetSprintf ( "2nd and 3rd CONTAINS() arguments must be numeric" );
			return -1;
		}
	}

	// check POLY2D args
	if ( eFunc==FUNC_POLY2D || eFunc==FUNC_GEOPOLY2D )
	{
		if ( dRetTypes.GetLength()==1 )
		{
			// handle 1 arg version, POLY2D(string-attr)
			if ( dRetTypes[0]!=SPH_ATTR_STRING )
			{
				m_sParserError.SetSprintf ( "%s() argument must be a string attribute", g_dFuncs[iFunc].m_sName );
				return -1;
			}
		} else if ( dRetTypes.GetLength()<6 )
		{
			// handle 2..5 arg versions, invalid
			m_sParserError.SetSprintf ( "bad %s() argument count, must be either 1 (string) or 6+ (x/y pairs list)", g_dFuncs[iFunc].m_sName );
			return -1;

		} else
		{
			// handle 6+ arg version, POLY2D(xy-list)
			if ( dRetTypes.GetLength() & 1 )
			{
				m_sParserError.SetSprintf ( "bad %s() argument count, must be even", g_dFuncs[iFunc].m_sName );
				return -1;
			}
			ARRAY_FOREACH ( i, dRetTypes )
				if ( !IsNumeric ( dRetTypes[i] ) )
			{
				m_sParserError.SetSprintf ( "%s() argument %d must be numeric", g_dFuncs[iFunc].m_sName, 1+i );
				return -1;
			}
		}
	}

	// do add
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = iFunc;
	tNode.m_iLeft = iLeft;
	tNode.m_iRight = iRight;
	tNode.m_eArgType = ( iLeft>=0 ) ? m_dNodes[iLeft].m_eRetType : SPH_ATTR_INTEGER;
	tNode.m_eRetType = g_dFuncs[iFunc].m_eRet;

	// fixup return type in a few special cases
	if ( eFunc==FUNC_MIN || eFunc==FUNC_MAX || eFunc==FUNC_MADD || eFunc==FUNC_MUL3 || eFunc==FUNC_ABS || eFunc==FUNC_IDIV )
		tNode.m_eRetType = tNode.m_eArgType;

	if ( eFunc==FUNC_EXIST )
	{
		int iExistRight = m_dNodes[iLeft].m_iRight;
		ESphAttr eType = m_dNodes[iExistRight].m_eRetType;
		tNode.m_eArgType = eType;
		tNode.m_eRetType = eType;
	}

	if ( eFunc==FUNC_BIGINT && tNode.m_eRetType==SPH_ATTR_FLOAT )
		tNode.m_eRetType = SPH_ATTR_FLOAT; // enforce if we can; FIXME! silently ignores BIGINT() on floats; should warn or raise an error

	if ( eFunc==FUNC_IF || eFunc==FUNC_BITDOT )
		tNode.m_eRetType = GetWidestRet ( iLeft, iRight );

	// all ok
	assert ( tNode.m_eRetType!=SPH_ATTR_NONE );
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeUdf ( int iCall, int iArg )
{
	UdfCall_t * pCall = m_dUdfCalls[iCall];
	SPH_UDF_INIT & tInit = pCall->m_tInit;
	SPH_UDF_ARGS & tArgs = pCall->m_tArgs;

	// initialize UDF right here, at AST creation stage
	// just because it's easy to gather arg types here
	if ( iArg>=0 )
	{
		// gather arg types
		CSphVector<DWORD> dArgTypes;

		int iCur = iArg;
		while ( iCur>=0 )
		{
			if ( m_dNodes[iCur].m_iToken!=',' )
			{
				dArgTypes.Add ( m_dNodes[iCur].m_eRetType );
				break;
			}

			int iRight = m_dNodes[iCur].m_iRight;
			if ( iRight>=0 )
			{
				assert ( m_dNodes[iRight].m_iToken!=',' );
				dArgTypes.Add ( m_dNodes[iRight].m_eRetType );
			}

			iCur = m_dNodes[iCur].m_iLeft;
		}

		assert ( dArgTypes.GetLength() );
		tArgs.arg_count = dArgTypes.GetLength();
		tArgs.arg_types = new sphinx_udf_argtype [ tArgs.arg_count ];

		// we gathered internal type ids in right-to-left order
		// reverse and remap
		// FIXME! eliminate remap, maybe?
		ARRAY_FOREACH ( i, dArgTypes )
		{
			sphinx_udf_argtype & eRes = tArgs.arg_types [ tArgs.arg_count-1-i ];
			switch ( dArgTypes[i] )
			{
				case SPH_ATTR_INTEGER:
				case SPH_ATTR_TIMESTAMP:
				case SPH_ATTR_ORDINAL:
				case SPH_ATTR_BOOL:
				case SPH_ATTR_WORDCOUNT:
					eRes = SPH_UDF_TYPE_UINT32;
					break;
				case SPH_ATTR_FLOAT:
					eRes = SPH_UDF_TYPE_FLOAT;
					break;
				case SPH_ATTR_BIGINT:
					eRes = SPH_UDF_TYPE_INT64;
					break;
				case SPH_ATTR_STRING:
					eRes = SPH_UDF_TYPE_STRING;
					break;
				case SPH_ATTR_UINT32SET:
					eRes = SPH_UDF_TYPE_UINT32SET;
					break;
				case SPH_ATTR_INT64SET:
					eRes = SPH_UDF_TYPE_UINT64SET;
					break;
				case SPH_ATTR_FACTORS:
					eRes = SPH_UDF_TYPE_FACTORS;
					break;
				default:
					m_sParserError.SetSprintf ( "internal error: unmapped UDF argument type (arg=%d, type=%d)", i, dArgTypes[i] );
					return -1;
			}
		}
	}

	// init
	if ( pCall->m_pUdf->m_fnInit )
	{
		char sError [ SPH_UDF_ERROR_LEN ];
		if ( pCall->m_pUdf->m_fnInit ( &tInit, &tArgs, sError ) )
		{
			m_sParserError = sError;
			return -1;
		}
	}

	// do add
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_UDF;
	tNode.m_iFunc = iCall;
	tNode.m_iLeft = iArg;
	tNode.m_iRight = -1;

	// deduce type
	tNode.m_eArgType = ( iArg>=0 ) ? m_dNodes[iArg].m_eRetType : SPH_ATTR_INTEGER;
	tNode.m_eRetType = pCall->m_pUdf->m_eRetType;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeConstlist ( int64_t iValue )
{
	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = TOK_CONST_LIST;
	tNode.m_pConsts = new ConstList_c();
	tNode.m_pConsts->Add ( iValue );
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeConstlist ( float iValue )
{
	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = TOK_CONST_LIST;
	tNode.m_pConsts = new ConstList_c();
	tNode.m_pConsts->Add ( iValue );
	return m_dNodes.GetLength()-1;
}

void ExprParser_t::AppendToConstlist ( int iNode, int64_t iValue )
{
	m_dNodes[iNode].m_pConsts->Add ( iValue );
}

void ExprParser_t::AppendToConstlist ( int iNode, float iValue )
{
	m_dNodes[iNode].m_pConsts->Add ( iValue );
}

int ExprParser_t::AddNodeUservar ( int iUservar )
{
	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = TOK_USERVAR;
	tNode.m_iConst = iUservar;
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeHookIdent ( int iID )
{
	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = TOK_HOOK_IDENT;
	tNode.m_iFunc = iID;
	tNode.m_eRetType = m_pHook->GetIdentType ( iID );
	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeHookFunc ( int iID, int iLeft )
{
	CSphVector<ESphAttr> dArgTypes;
	GatherArgRetTypes ( iLeft, dArgTypes );

	ESphAttr eRet = m_pHook->GetReturnType ( iID, dArgTypes, CheckForConstSet ( iLeft, 0 ), m_sParserError );
	if ( eRet==SPH_ATTR_NONE )
		return -1;

	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = TOK_HOOK_FUNC;
	tNode.m_iFunc = iID;
	tNode.m_iLeft = iLeft;
	tNode.m_iRight = -1;

	// deduce type
	tNode.m_eArgType = ( iLeft>=0 ) ? m_dNodes[iLeft].m_eRetType : SPH_ATTR_INTEGER;
	tNode.m_eRetType = eRet;

	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeConsthash ( const char * sKey, int64_t iValue )
{
	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = TOK_CONST_HASH;
	tNode.m_pConsthash = new ConstHash_c();
	tNode.m_pConsthash->Add ( sKey, iValue );
	tNode.m_eRetType = SPH_ATTR_CONSTHASH;
	return m_dNodes.GetLength()-1;
}

void ExprParser_t::AppendToConsthash ( int iNode, const char * sKey, int64_t iValue )
{
	m_dNodes[iNode].m_pConsthash->Add ( sKey, iValue );
}

const char * ExprParser_t::Attr2Ident ( uint64_t uAttrLoc )
{
	ExprNode_t tAttr;
	sphUnpackAttrLocator ( uAttrLoc, &tAttr );

	CSphString sIdent;
	sIdent = m_pSchema->GetAttr ( tAttr.m_iLocator ).m_sName;
	m_dIdents.Add ( sIdent.Leak() );
	return m_dIdents.Last();
}

//////////////////////////////////////////////////////////////////////////

struct TypeCheck_fn
{
	bool * m_pRes;

	explicit TypeCheck_fn ( bool * pRes )
		: m_pRes ( pRes )
	{
		*m_pRes = false;
	}

	void Enter ( const ExprNode_t & tNode, const CSphVector<ExprNode_t> & dNodes )
	{
		bool bNumberOp = tNode.m_iToken=='+' || tNode.m_iToken=='-' || tNode.m_iToken=='*' || tNode.m_iToken=='/';
		if ( bNumberOp )
		{
			bool bLeftNumeric =	tNode.m_iLeft==-1 ? false : IsNumericNode ( dNodes[tNode.m_iLeft] );
			bool bRightNumeric = tNode.m_iRight==-1 ? false : IsNumericNode ( dNodes[tNode.m_iRight] );

			if ( !bLeftNumeric || !bRightNumeric )
				*m_pRes = true;
		}
	}

	void Exit ( const ExprNode_t & )
	{}

	bool IsNumericNode ( const ExprNode_t & tNode )
	{
		return tNode.m_eRetType==SPH_ATTR_INTEGER || tNode.m_eRetType==SPH_ATTR_ORDINAL || tNode.m_eRetType==SPH_ATTR_BOOL || tNode.m_eRetType==SPH_ATTR_FLOAT ||
			tNode.m_eRetType==SPH_ATTR_BIGINT || tNode.m_eRetType==SPH_ATTR_WORDCOUNT || tNode.m_eRetType==SPH_ATTR_TOKENCOUNT || tNode.m_eRetType==SPH_ATTR_TIMESTAMP;
	}
};


struct WeightCheck_fn
{
	bool * m_pRes;

	explicit WeightCheck_fn ( bool * pRes )
		: m_pRes ( pRes )
	{
		assert ( m_pRes );
		*m_pRes = false;
	}

	void Enter ( const ExprNode_t & tNode, const CSphVector<ExprNode_t> & )
	{
		if ( tNode.m_iToken==TOK_WEIGHT )
			*m_pRes = true;
	}

	void Exit ( const ExprNode_t & )
	{}
};


struct HookCheck_fn
{
	ISphExprHook * m_pHook;

	explicit HookCheck_fn ( ISphExprHook * pHook )
		: m_pHook ( pHook )
	{}

	void Enter ( const ExprNode_t & tNode, const CSphVector<ExprNode_t> & )
	{
		if ( tNode.m_iToken==TOK_HOOK_IDENT || tNode.m_iToken==TOK_HOOK_FUNC )
			m_pHook->CheckEnter ( tNode.m_iFunc );
	}

	void Exit ( const ExprNode_t & tNode )
	{
		if ( tNode.m_iToken==TOK_HOOK_IDENT || tNode.m_iToken==TOK_HOOK_FUNC )
			m_pHook->CheckExit ( tNode.m_iFunc );
	}
};


ISphExpr * ExprParser_t::Parse ( const char * sExpr, const CSphSchema & tSchema,
	ESphAttr * pAttrType, bool * pUsesWeight, CSphString & sError )
{
	m_sLexerError = "";
	m_sParserError = "";
	m_sCreateError = "";

	// setup lexer
	m_sExpr = sExpr;
	m_pCur = sExpr;
	m_pSchema = &tSchema;

	// setup constant functions
	m_iConstNow = (int) time ( NULL );

	// build tree
	m_iParsed = -1;
	yyparse ( this );

	// handle errors
	if ( m_iParsed<0 || !m_sLexerError.IsEmpty() || !m_sParserError.IsEmpty() )
	{
		sError = !m_sLexerError.IsEmpty() ? m_sLexerError : m_sParserError;
		if ( sError.IsEmpty() ) sError = "general parsing error";
		return NULL;
	}

	// deduce return type
	ESphAttr eAttrType = m_dNodes[m_iParsed].m_eRetType;
// assert ( IsNumeric ( eAttrType ) );

	// check expression stack
	if ( m_dNodes.GetLength()>100 )
	{
		CSphVector<int> dNodes;
		dNodes.Reserve ( m_dNodes.GetLength()/2 );
		int iMaxHeight = 1;
		int iHeight = 1;
		dNodes.Add ( m_iParsed );
		while ( dNodes.GetLength() )
		{
			const ExprNode_t & tExpr = m_dNodes[dNodes.Pop()];
			iHeight += ( tExpr.m_iLeft>=0 || tExpr.m_iRight>=0 ? 1 : -1 );
			iMaxHeight = Max ( iMaxHeight, iHeight );
			if ( tExpr.m_iRight>=0 )
				dNodes.Add ( tExpr.m_iRight );
			if ( tExpr.m_iLeft>=0 )
				dNodes.Add ( tExpr.m_iLeft );
		}

#define SPH_EXPRNODE_STACK_SIZE 160
		int64_t iExprStack = sphGetStackUsed() + iMaxHeight*SPH_EXPRNODE_STACK_SIZE;
		if ( g_iThreadStackSize<=iExprStack )
		{
			sError.SetSprintf ( "query too complex, not enough stack (thread_stack=%dK or higher required)",
				(int)( ( iExprStack + 1024 - ( iExprStack%1024 ) ) / 1024 ) );
			return NULL;
		}
	}

	// perform optimizations
	Optimize ( m_iParsed );
#if 0
	Dump ( m_iParsed );
#endif

	bool bTypeMismatch;
	TypeCheck_fn tFunctor ( &bTypeMismatch );
	WalkTree ( m_iParsed, tFunctor );
	if ( bTypeMismatch )
	{
		sError.SetSprintf ( "numeric operation applied to non-numeric operands" );
		return NULL;
	}

	// create evaluator
	ISphExpr * pRes = CreateTree ( m_iParsed );
	if ( !m_sCreateError.IsEmpty() )
	{
		sError = m_sCreateError;
		SafeRelease ( pRes );
	} else if ( !pRes )
	{
		sError.SetSprintf ( "empty expression" );
	}

	if ( pAttrType )
		*pAttrType = eAttrType;

	if ( pUsesWeight )
	{
		WeightCheck_fn tFunctor ( pUsesWeight );
		WalkTree ( m_iParsed, tFunctor );
	}

	if ( m_pHook )
	{
		HookCheck_fn tFunctor ( m_pHook );
		WalkTree ( m_iParsed, tFunctor );
	}

	return pRes;
}

//////////////////////////////////////////////////////////////////////////
// UDF MANAGER
//////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
#define HAVE_DLOPEN		1
#define RTLD_LAZY		0
#define RTLD_LOCAL		0

void * dlsym ( void * lib, const char * name )
{
	return GetProcAddress ( (HMODULE)lib, name );
}

void * dlopen ( const char * libname, int )
{
	return LoadLibraryEx ( libname, NULL, 0 );
}

int dlclose ( void * lib )
{
	return FreeLibrary ( (HMODULE)lib )
		? 0
		: GetLastError();
}

const char * dlerror()
{
	static char sError[256];
	DWORD uError = GetLastError();
	FormatMessage ( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
		uError, LANG_SYSTEM_DEFAULT, (LPTSTR)sError, sizeof(sError), NULL );
	return sError;
}
#endif // USE_WINDOWS

#if !HAVE_DLOPEN

void sphUDFInit ( const char * )
{
	return;
}

void sphUDFLock ( bool bLocked )
{
	g_bUdfLocked = bLocked;
}

bool sphUDFCreate ( const char *, const char *, ESphAttr, CSphString & sError )
{
	sError = "no dlopen(); UDF support disabled";
	return false;
}

bool sphUDFDrop ( const char *, CSphString & sError )
{
	sError = "no dlopen(); UDF support disabled";
	return false;
}

#else

void sphUDFInit ( const char * sUdfDir )
{
	if ( !sUdfDir || !*sUdfDir )
		return;

	g_sUdfDir = sUdfDir;
	g_bUdfEnabled = true;
	g_bUdfLocked = false;
}


void sphUDFLock ( bool bLocked )
{
	g_bUdfLocked = bLocked;
}


bool sphUDFCreate ( const char * szLib, const char * szFunc, ESphAttr eRetType, CSphString & sError )
{
	if ( !g_bUdfEnabled )
	{
		sError = "UDF support disabled (requires a valid plugin_dir)";
		return false;
	}
	if ( g_bUdfLocked )
	{
		sError = "CREATE FUNCTION is disabled (fully dynamic UDFs require workers=threads)";
		return false;
	}

	// validate library name
	for ( const char * p = szLib; *p; p++ )
		if ( *p=='/' || *p=='\\' )
	{
		sError = "restricted character (path delimiter) in a library file name";
		return false;
	}

	// from here, we need a lock (we intend to update UDF hash)
	g_tUdfMutex.Lock();

	// validate function name
	CSphString sFunc ( szFunc );
	sFunc.ToLower();

	if ( g_hUdfFuncs ( sFunc ) )
	{
		sError.SetSprintf ( "UDF '%s' already exists", sFunc.cstr() );
		g_tUdfMutex.Unlock();
		return false;
	}

	// lookup or load library
	CSphString sLibfile;
	sLibfile.SetSprintf ( "%s/%s", g_sUdfDir.cstr(), szLib );

	UdfFunc_t tFunc;
	tFunc.m_eRetType = eRetType;
	tFunc.m_iUserCount = 0;
	tFunc.m_bToDrop = false;

	bool bLoaded = false;
	void * pHandle = NULL;
	tFunc.m_pLib = g_hUdfLibs ( szLib );
	if ( !tFunc.m_pLib )
	{
		bLoaded = true;
		pHandle = dlopen ( sLibfile.cstr(), RTLD_LAZY | RTLD_LOCAL );
		if ( !pHandle )
		{
			const char * sDlerror = dlerror();
			sError.SetSprintf ( "dlopen() failed: %s", sDlerror ? sDlerror : "(null)" );
			g_tUdfMutex.Unlock();
			return false;
		}
		sphLogDebug ( "dlopen(%s)=%p", sLibfile.cstr(), pHandle );

	} else
	{
		pHandle = tFunc.m_pLib->m_pHandle;
	}
	assert ( pHandle );

	// lookup and check function symbols
	CSphString sName;
	tFunc.m_fnFunc = dlsym ( pHandle, sFunc.cstr() );
	tFunc.m_fnInit = (UdfInit_fn) dlsym ( pHandle, sName.SetSprintf ( "%s_init", sFunc.cstr() ).cstr() );
	tFunc.m_fnDeinit = (UdfDeinit_fn) dlsym ( pHandle, sName.SetSprintf ( "%s_deinit", sFunc.cstr() ).cstr() );

	if ( !tFunc.m_fnFunc || !tFunc.m_fnInit )
	{
		sError.SetSprintf ( "symbol '%s%s' not found in '%s'", sFunc.cstr(), tFunc.m_fnFunc ? "_init" : "", szLib );
		if ( bLoaded )
			dlclose ( pHandle );
		g_tUdfMutex.Unlock();
		return false;
	}

	// add library
	if ( bLoaded )
	{
		CSphString sBasename = szLib;
		const char * pDot = strchr ( sBasename.cstr(), '.' );
		if ( pDot )
			sBasename = sBasename.SubString ( 0, pDot-sBasename.cstr() );

		UdfVer_fn fnVer = (UdfVer_fn) dlsym ( pHandle, sName.SetSprintf ( "%s_ver", sBasename.cstr() ).cstr() );
		if ( !fnVer )
		{
			sError.SetSprintf ( "symbol '%s_ver' not found in '%s': update your UDF implementation", sBasename.cstr(), szLib );
			dlclose ( pHandle );
			g_tUdfMutex.Unlock();
			return false;
		}

		if ( fnVer() < SPH_UDF_VERSION )
		{
			sError.SetSprintf ( "library '%s' was compiled using an older version of sphinxudf.h; it needs to be recompiled", szLib );
			dlclose ( pHandle );
			g_tUdfMutex.Unlock();
			return false;
		}

		UdfLib_t tLib;
		tLib.m_iFuncs = 1;
		tLib.m_pHandle = pHandle;
		Verify ( g_hUdfLibs.Add ( tLib, szLib ) );
		tFunc.m_pLib = g_hUdfLibs ( szLib );
	} else
	{
		tFunc.m_pLib->m_iFuncs++;
	}
	tFunc.m_pLibName = g_hUdfLibs.GetKeyPtr ( szLib );
	assert ( tFunc.m_pLib );

	// add function
	Verify ( g_hUdfFuncs.Add ( tFunc, sFunc ) );

	// all ok
	g_tUdfMutex.Unlock();
	return true;
}


bool sphUDFDrop ( const char * szFunc, CSphString & sError )
{
	if ( g_bUdfLocked )
	{
		sError = "DROP FUNCTION is disabled (fully dynamic UDFs require workers=threads)";
		return false;
	}

	CSphString sFunc ( szFunc );
	sFunc.ToLower();

	g_tUdfMutex.Lock();
	UdfFunc_t * pFunc = g_hUdfFuncs ( sFunc );
	if ( !pFunc || pFunc->m_bToDrop ) // handle concurrent drop in progress as "not exists"
	{
		sError.SetSprintf ( "UDF '%s' does not exist", sFunc.cstr() );
		g_tUdfMutex.Unlock();
		return false;
	}

	const int UDF_DROP_TIMEOUT_SEC = 30; // in seconds
	int64_t tmEnd = sphMicroTimer() + UDF_DROP_TIMEOUT_SEC*1000000;

	// mark function for deletion, to prevent new users
	pFunc->m_bToDrop = true;
	if ( pFunc->m_iUserCount )
		for ( ;; )
	{
		// release lock and wait
		// so that concurrent users could complete and release the function
		g_tUdfMutex.Unlock();
		sphSleepMsec ( 50 );

		// re-acquire lock
		g_tUdfMutex.Lock();

		// everyone out? proceed with dropping
		assert ( pFunc->m_iUserCount>=0 );
		if ( pFunc->m_iUserCount<=0 )
			break;

		// timed out? clear deletion flag, and bail
		if ( sphMicroTimer() > tmEnd )
		{
			pFunc->m_bToDrop = false;
			g_tUdfMutex.Unlock();

			sError.SetSprintf ( "DROP timed out in (still got %d users after waiting for %d seconds); please retry",
				pFunc->m_iUserCount, UDF_DROP_TIMEOUT_SEC );
			return false;
		}
	}

	UdfLib_t * pLib = pFunc->m_pLib;
	const CSphString * pLibName = pFunc->m_pLibName;

	Verify ( g_hUdfFuncs.Delete ( sFunc ) );
	if ( --pLib->m_iFuncs<=0 )
	{
		// FIXME! running queries might be using this function
		int iRes = dlclose ( pLib->m_pHandle );
		sphLogDebug ( "dlclose(%s)=%d", pLibName->cstr(), iRes );
		Verify ( g_hUdfLibs.Delete ( *pLibName ) );
	}

	g_tUdfMutex.Unlock();
	return true;
}


static const char * UdfReturnType ( ESphAttr eType )
{
	switch ( eType )
	{
		case SPH_ATTR_INTEGER:		return "INT";
		case SPH_ATTR_FLOAT:		return "FLOAT";
		case SPH_ATTR_STRINGPTR:	return "STRING";
		default:					assert ( 0 && "unknown UDF return type" ); return "???";
	}
}


void sphUDFSaveState ( CSphWriter & tWriter )
{
	g_tUdfMutex.Lock();
	g_hUdfFuncs.IterateStart();
	while ( g_hUdfFuncs.IterateNext() )
	{
		const CSphString & sName = g_hUdfFuncs.IterateGetKey();
		const UdfFunc_t & tDesc = g_hUdfFuncs.IterateGet();
		if ( !tDesc.m_bToDrop )
		{
			CSphString sBuf;
			sBuf.SetSprintf ( "CREATE FUNCTION %s RETURNS %s SONAME '%s';\n",
				sName.cstr(), UdfReturnType ( tDesc.m_eRetType ), tDesc.m_pLibName->cstr() );
			tWriter.PutBytes ( sBuf.cstr(), sBuf.Length() );
		}
	}
	g_tUdfMutex.Unlock();
}

#endif // HAVE_DLOPEN


ISphExpr * sphExprJsonField ( const CSphColumnInfo & tCol, int iAttr, const char * sField )
{
	return new Expr_JsonField_c ( tCol, iAttr, sField );
}


//////////////////////////////////////////////////////////////////////////
// PUBLIC STUFF
//////////////////////////////////////////////////////////////////////////

/// parser entry point
ISphExpr * sphExprParse ( const char * sExpr, const CSphSchema & tSchema, ESphAttr * pAttrType, bool * pUsesWeight,
	CSphString & sError, CSphSchema * pExtra, ISphExprHook * pHook, bool * pZonespanlist, bool * pPackedFactors, ESphEvalStage * pEvalStage )
{
	// parse into opcodes
	ExprParser_t tParser ( pExtra, pHook );
	ISphExpr * bRes = tParser.Parse ( sExpr, tSchema, pAttrType, pUsesWeight, sError );
	if ( pZonespanlist )
		*pZonespanlist = tParser.m_bHasZonespanlist;
	if ( pEvalStage )
		*pEvalStage = tParser.m_eEvalStage;
	if ( pPackedFactors )
		*pPackedFactors = tParser.m_bHasPackedFactors;
	return bRes;
}

//
// $Id$
//
