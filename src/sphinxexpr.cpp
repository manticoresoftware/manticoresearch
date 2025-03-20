//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxexpr.h"
#include "sphinxplugin.h"

#include "sphinxutils.h"
#include "attribute.h"
#include "sphinxint.h"
#include "sphinxjson.h"
#include "docstore.h"
#include "coroutine.h"
#include "stackmock.h"
#include "client_task_info.h"
#include "exprtraits.h"
#include "columnarexpr.h"
#include "conversion.h"
#include "geodist.h"
#include "knnmisc.h"
#include <time.h>
#include <math.h>
#include "uni_algo/case.h"
#include "datetime.h"
#include "exprdatetime.h"
#include "exprdocstore.h"

#if WITH_RE2
#include <re2/re2.h>
#endif

#ifndef M_LOG2E
#define M_LOG2E		1.44269504088896340736
#endif

#ifndef M_LOG10E
#define M_LOG10E	0.434294481903251827651
#endif

namespace { // static
fnGetUserVar& refUservars()
{
	static fnGetUserVar pUservarsHook = nullptr;
	return pUservarsHook;
}
}

void SetUserVarsHook ( fnGetUserVar fnHook )
{
	refUservars () = fnHook;
}

bool UservarsAvailable ()
{
	return refUservars ()!=nullptr;
}

UservarIntSet_c Uservars ( const CSphString & sUservar )
{
	assert ( UservarsAvailable () );
	return refUservars () ( sUservar );
}

inline Str_t CurrentUser()
{
	if ( session::GetVip () )
		return { "VIP", 3 };
	return { "Usual", 5 };
}

CSphString& sphinxexpr::MySQLVersion()
{
	static CSphString sSQLVersion;
	return sSQLVersion;
}

inline int ConnID ()
{
	return session::GetConnID ();
}


//////////////////////////////////////////////////////////////////////////
// EVALUATION ENGINE
//////////////////////////////////////////////////////////////////////////

struct ExprLocatorTraits_t
{
	CSphAttrLocator m_tLocator;
	CSphString		m_sAttr;
	CSphString		m_sColumnarAttr;

	ExprLocatorTraits_t ( const CSphAttrLocator & tLocator, const CSphString & sAttr ) : m_tLocator ( tLocator ), m_sAttr ( sAttr ) {}
	virtual ~ExprLocatorTraits_t() = default;

	virtual void HandleCommand ( ESphExprCommand eCmd, void * pArg )
	{
		switch ( eCmd )
		{
		case SPH_EXPR_GET_DEPENDENT_COLS:
			if ( !m_sAttr.IsEmpty() )
				static_cast<StrVec_t*>(pArg)->Add(m_sAttr);
			break;

		case SPH_EXPR_SET_COLUMNAR_COL:
			m_sColumnarAttr = *static_cast < CSphString* >(pArg);
			break;

		case SPH_EXPR_GET_COLUMNAR_COL:
			*static_cast < CSphString* >(pArg) = m_sColumnarAttr;
			break;

		default:
			break;
		}
	}

	virtual void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
	{
		sphFixupLocator ( m_tLocator, pOldSchema, pNewSchema );
	}

protected:
	ExprLocatorTraits_t ( const ExprLocatorTraits_t& ) = default;

};


const BYTE * ISphExpr::StringEvalPacked ( const CSphMatch & tMatch ) const
{
	const BYTE * pStr = nullptr;
	int iStrLen = StringEval ( tMatch, &pStr );
	auto pRes = sphPackPtrAttr ( { pStr, iStrLen } );
	FreeDataPtr ( *this, pStr );
	return pRes;
}

template<class BaseExpr_T>
class Expr_WithLocator_T : public BaseExpr_T, public ExprLocatorTraits_t
{
public:
	Expr_WithLocator_T ( const CSphAttrLocator & tLocator, const CSphString & sAttr )
		: ExprLocatorTraits_t ( tLocator, sAttr )
	{}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		sphFixupLocator ( m_tLocator, pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		HandleCommand ( eCmd, pArg );
	}

protected:
	Expr_WithLocator_T ( const Expr_WithLocator_T & rhs )
		: ExprLocatorTraits_t (rhs) {}
};

class Expr_WithLocator_c : public Expr_WithLocator_T<ISphExpr>
{
public:
	Expr_WithLocator_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr )
		: Expr_WithLocator_T ( tLocator, sAttr )
	{}
};

// has string expression traits, but has no locator
class Expr_StrNoLocator_c : public ISphStringExpr
{
public:
	void FixupLocator ( const ISphSchema * /*pOldSchema*/, const ISphSchema * /*pNewSchema*/ ) override {}
};

//	base class does not convert string to float
float ISphStringExpr::Eval ( const CSphMatch & tMatch ) const
{
	float fVal = 0.f;
	const char * pBuf = nullptr;
	int  iLen = StringEval ( tMatch, (const BYTE **) &pBuf );

	if ( iLen && pBuf )
	{
		const char * pMax = sphFindLastNumeric ( pBuf, iLen );
		if ( pBuf<pMax )
		{
			fVal = (float) strtod ( pBuf, nullptr );
		}
		else
		{
			CSphString sBuf;
			sBuf.SetBinary ( pBuf, iLen );
			fVal = (float) strtod ( sBuf.cstr(), nullptr );
		}
	}

	FreeDataPtr ( *this, pBuf );
	return fVal;
}

//	base class does not convert string to int
int ISphStringExpr::IntEval ( const CSphMatch & tMatch ) const
{
	int iVal = 0;
	const char * pBuf = nullptr;
	int  iLen = StringEval ( tMatch, (const BYTE **) &pBuf );

	if ( iLen && pBuf )
	{
		const char * pMax = sphFindLastNumeric ( pBuf, iLen );
		if ( pBuf<pMax )
		{
			iVal = strtol ( pBuf, NULL, 10 );
		}
		else
		{
			CSphString sBuf;
			sBuf.SetBinary ( pBuf, iLen );
			iVal = strtol ( sBuf.cstr(), NULL, 10 );
		}
	}

	FreeDataPtr ( *this, pBuf );
	return iVal;
}

//	base class does not convert string to int64
int64_t ISphStringExpr::Int64Eval ( const CSphMatch & tMatch ) const
{
	int64_t iVal = 0;
	const char * pBuf = nullptr;
	int  iLen = StringEval ( tMatch, (const BYTE **) &pBuf );

	if ( iLen && pBuf )
	{
		const char * pMax = sphFindLastNumeric ( pBuf, iLen );
		if ( pBuf<pMax )
		{
			iVal = strtoll ( pBuf, nullptr, 10 );
		}
		else
		{
			CSphString sBuf;
			sBuf.SetBinary ( pBuf, iLen );
			iVal = strtoll ( sBuf.cstr(), nullptr, 10 );
		}
	}

	FreeDataPtr ( *this, pBuf );
	return iVal;
}


//////////////////////////////////////////////////////////////////////////

class Expr_GetInt_c : public Expr_WithLocator_c
{
public:
	Expr_GetInt_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr ) : Expr_WithLocator_c ( tLocator, sAttr ) {}
	float Eval ( const CSphMatch & tMatch ) const final { return (float) tMatch.GetAttr ( m_tLocator ); } // FIXME! OPTIMIZE!!! we can go the short route here
	int IntEval ( const CSphMatch & tMatch ) const final { return (int)tMatch.GetAttr ( m_tLocator ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t)tMatch.GetAttr ( m_tLocator ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetInt_c");
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetInt_c ( *this );
	}

private:
	Expr_GetInt_c ( const Expr_GetInt_c& ) = default;
};


class Expr_GetBits_c : public Expr_WithLocator_c
{
public:
	Expr_GetBits_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr ) : Expr_WithLocator_c ( tLocator, sAttr ) {}
	float Eval ( const CSphMatch & tMatch ) const final { return (float) tMatch.GetAttr ( m_tLocator ); }
	int IntEval ( const CSphMatch & tMatch ) const final { return (int)tMatch.GetAttr ( m_tLocator ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t)tMatch.GetAttr ( m_tLocator ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetBits_c");
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetBits_c ( *this );
	}

private:
	Expr_GetBits_c ( const Expr_GetBits_c& ) = default;
};


class Expr_GetSint_c : public Expr_WithLocator_c
{
public:
	Expr_GetSint_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr ) : Expr_WithLocator_c ( tLocator, sAttr ) {}
	float Eval ( const CSphMatch & tMatch ) const final { return (float)(int)tMatch.GetAttr ( m_tLocator ); }
	int IntEval ( const CSphMatch & tMatch ) const final { return (int)tMatch.GetAttr ( m_tLocator ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int)tMatch.GetAttr ( m_tLocator ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetSint_c");
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetSint_c ( *this );
	}

private:
	Expr_GetSint_c ( const Expr_GetSint_c& ) = default;
};


class Expr_GetFloat_c : public Expr_WithLocator_c
{
public:
	Expr_GetFloat_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr ) : Expr_WithLocator_c ( tLocator, sAttr ) {}
	float Eval ( const CSphMatch & tMatch ) const final { return tMatch.GetAttrFloat ( m_tLocator ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetFloat_c");
		return CALC_DEP_HASHES();
	}

	ISphExpr* Clone() const final
	{
		return new Expr_GetFloat_c ( *this );
	}

private:
	Expr_GetFloat_c ( const Expr_GetFloat_c& ) = default;
};


class Expr_GetString_c : public Expr_WithLocator_T<ISphStringExpr>
{
public:
	Expr_GetString_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr )
		: Expr_WithLocator_T ( tLocator, sAttr )
	{}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_WithLocator_T::Command ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE*)pArg;
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		if ( m_tLocator.IsBlobAttr() && !m_pBlobPool )
			return 0;

		auto dStr = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		*ppStr = dStr.first; /// FIXME! m.b. all StringEval return BytesBlob_t?
		return dStr.second;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetString_c");
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetString_c ( *this );
	}

private:
	const BYTE * m_pBlobPool {nullptr};

	Expr_GetString_c ( const Expr_GetString_c& rhs ) : Expr_WithLocator_T ( rhs ) {}
};


class Expr_GetMva_c : public Expr_WithLocator_c
{
public:
	Expr_GetMva_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr )
		: Expr_WithLocator_c ( tLocator, sAttr )
	{}

	float Eval ( const CSphMatch & ) const final { assert ( 0 ); return 0; }
	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_WithLocator_c::Command ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE *)pArg;
	}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const final
	{
		return (int64_t)sphPackPtrAttr ( tMatch.FetchAttrData ( m_tLocator, m_pBlobPool ) );
	}

	ByteBlob_t MvaEval ( const CSphMatch & tMatch ) const final
	{
		return tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetMva_c");
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetMva_c ( *this );
	}

private:
	const BYTE * m_pBlobPool {nullptr};

	Expr_GetMva_c ( const Expr_GetMva_c& rhs ) : Expr_WithLocator_c ( rhs ) {}
};


class Expr_GetFactorsAttr_c : public Expr_WithLocator_c
{
public:
	Expr_GetFactorsAttr_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr )
		: Expr_WithLocator_c ( tLocator, sAttr )
	{}

	float Eval ( const CSphMatch & ) const final { assert ( 0 ); return 0; }
	const BYTE * FactorEval ( const CSphMatch & tMatch ) const final
	{
		auto * pPacked = (const BYTE *)tMatch.GetAttr ( m_tLocator );
		return sphUnpackPtrAttr ( pPacked ).first;
	}

	const BYTE * FactorEvalPacked ( const CSphMatch & tMatch ) const final
	{
		return (const BYTE *)tMatch.GetAttr ( m_tLocator );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetFactorsAttr_c");
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetFactorsAttr_c ( *this );
	}

private:
	Expr_GetFactorsAttr_c ( const Expr_GetFactorsAttr_c& ) = default;
};


class Expr_GetConst_c : public Expr_NoLocator_c
{
public:
	explicit Expr_GetConst_c ( float fValue )
		: m_fValue ( fValue )
	{}

	float Eval ( const CSphMatch & ) const final { return m_fValue; }
	int IntEval ( const CSphMatch & ) const final { return (int)m_fValue; }
	int64_t Int64Eval ( const CSphMatch & ) const final { return (int64_t)m_fValue; }
	bool IsConst () const final { return true; }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetConst_c");
		CALC_POD_HASH(m_fValue);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetConst_c ( *this );
	}

private:
	float m_fValue {0.0f};

	Expr_GetConst_c ( const Expr_GetConst_c& rhs ) : m_fValue ( rhs.m_fValue ) {}
};


class Expr_GetIntConst_c : public Expr_NoLocator_c
{
public:
	explicit Expr_GetIntConst_c ( int iValue )
		: m_iValue ( iValue )
	{}

	float Eval ( const CSphMatch & ) const final { return (float) m_iValue; } // no assert() here cause generic float Eval() needs to work even on int-evaluator tree
	int IntEval ( const CSphMatch & ) const final { return m_iValue; }
	int64_t Int64Eval ( const CSphMatch & ) const final { return m_iValue; }
	bool IsConst () const final { return true; }
	
	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetIntConst_c");
		CALC_POD_HASH(m_iValue);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetIntConst_c ( *this );
	}

private:
	int m_iValue {0};

	Expr_GetIntConst_c ( const Expr_GetIntConst_c& rhs ) : m_iValue ( rhs.m_iValue ) {}
};


class Expr_GetInt64Const_c : public Expr_NoLocator_c
{
public:
	explicit Expr_GetInt64Const_c ( int64_t iValue )
		: m_iValue ( iValue )
	{}

	float Eval ( const CSphMatch & ) const final { return (float) m_iValue; } // no assert() here cause generic float Eval() needs to work even on int-evaluator tree
	int IntEval ( const CSphMatch & ) const final { assert ( 0 ); return (int)m_iValue; }
	int64_t Int64Eval ( const CSphMatch & ) const final { return m_iValue; }
	bool IsConst () const final { return true; }
	
	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetInt64Const_c");
		CALC_POD_HASH(m_iValue);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetInt64Const_c ( *this );
	}

private:
	int64_t m_iValue {0};

	Expr_GetInt64Const_c ( const Expr_GetInt64Const_c& rhs ) : m_iValue ( rhs.m_iValue ) {}
};


class Expr_GetStrConst_c : public Expr_StrNoLocator_c
{
public:
	Expr_GetStrConst_c ( const char * sVal, int iLen, bool bUnescape )
	{
		if ( iLen>0 )
		{
			if ( bUnescape )
				std::tie ( m_sVal, m_iLen ) = SqlUnescapeN ( sVal, iLen );
			else {
				m_sVal.SetBinary ( sVal, iLen );
				m_iLen = iLen;
			}
		} else
			m_iLen = m_sVal.Length();
	}

	Expr_GetStrConst_c ( Str_t sVal, bool bUnescape )
		: Expr_GetStrConst_c ( (const char*) sVal.first, (int) sVal.second, bUnescape )
	{}

	int StringEval ( const CSphMatch &, const BYTE ** ppStr ) const final
	{
		*ppStr = (const BYTE*) m_sVal.cstr();
		return m_iLen;
	}

	bool IsConst () const final { return true; }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetStrConst_c");
		CALC_STR_HASH(m_sVal, m_iLen);
		return CALC_DEP_HASHES();
	}

	const CSphString &	GetStr() { return m_sVal; }

	ISphExpr * Clone () const final
	{
		return new Expr_GetStrConst_c ( *this );
	}

private:
	CSphString	m_sVal;
	int			m_iLen {0};

	Expr_GetStrConst_c ( const Expr_GetStrConst_c& rhs ) : m_sVal ( rhs.m_sVal ), m_iLen ( rhs.m_iLen ) {}
};


class Expr_GetZonespanlist_c : public Expr_StrNoLocator_c
{
public:
	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		assert ( ppStr );
		if ( !m_pData || !m_pData->GetLength() )
		{
			*ppStr = nullptr;
			return 0;
		}
		m_sBuilder.Clear();
		const CSphVector<int> & dSpans = *m_pData;
		int iStart = tMatch.m_iTag + 1; // spans[tag] contains the length, so the 1st data index is tag+1
		int iEnd = iStart + dSpans [ tMatch.m_iTag ]; // [start,end) now covers all data indexes
		for ( int i=iStart; i<iEnd; i+=2 )
			m_sBuilder.Appendf ( " %d:%d", 1+dSpans[i], 1+dSpans[i+1] ); // convert our 0-based span numbers to human 1-based ones
		auto iRes = m_sBuilder.GetLength ();
		*ppStr = m_sBuilder.Leak();
		return iRes;
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		if ( eCmd==SPH_EXPR_SET_EXTRA_DATA )
			static_cast<ISphExtra*>(pArg)->ExtraData ( EXTRA_GET_DATA_ZONESPANS, (void**)const_cast<IntVec_t**>(&m_pData) );
	}

	bool IsDataPtrAttr() const final
	{
		return true;
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & bDisable ) final
	{
		bDisable = true; // disable caching for now, might add code to process if necessary
		return 0;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetZonespanlist_c;
	}

private:
	const CSphVector<int> *	m_pData {nullptr};
	mutable StringBuilder_c	m_sBuilder;
};


class Expr_GetRankFactors_c : public Expr_StrNoLocator_c
{
public:
	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		assert ( ppStr );
		if ( !m_pFactors )
		{
			*ppStr = nullptr;
			return 0;
		}

		CSphString * sVal = (*m_pFactors) ( tMatch.m_tRowID );
		if ( !sVal )
		{
			*ppStr = nullptr;
			return 0;
		}
		int iLen = sVal->Length();
		*ppStr = (const BYTE*)sVal->Leak();
		m_pFactors->Delete ( tMatch.m_tRowID );
		return iLen;
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		if ( eCmd==SPH_EXPR_SET_EXTRA_DATA )
			static_cast<ISphExtra*>(pArg)->ExtraData ( EXTRA_GET_DATA_RANKFACTORS, (void**)&m_pFactors );
	}

	bool IsDataPtrAttr() const final
	{
		return true;
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & bDisable ) final
	{
		bDisable = true; // disable caching for now, might add code to process if necessary
		return 0;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetRankFactors_c;
	}

private:
	/// hash type MUST BE IN SYNC with RankerState_Export_fn in sphinxsearch.cpp
	CSphOrderedHash < CSphString, RowID_t, IdentityHash_fn, 256 > * m_pFactors {nullptr};
};


class Expr_GetPackedFactors_c : public Expr_StrNoLocator_c
{
public:
	const BYTE * FactorEval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pData = nullptr;
		int iDataLen = FetchHashEntry ( tMatch, pData );
		if ( !pData )
			return nullptr;

		auto * pResult = new BYTE[iDataLen];
		memcpy ( pResult, pData, iDataLen );

		return pResult;
	}

	const BYTE * FactorEvalPacked ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pData = nullptr;
		int iDataLen = FetchHashEntry ( tMatch, pData );
		if ( !pData )
			return nullptr;

		return sphPackPtrAttr( {pData, iDataLen} );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		if ( eCmd==SPH_EXPR_SET_EXTRA_DATA )
			static_cast<ISphExtra*>(pArg)->ExtraData ( EXTRA_GET_DATA_PACKEDFACTORS, (void**)&m_pHash );
	}

	bool IsDataPtrAttr() const final
	{
		return true;
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & bDisable ) final
	{
		bDisable = true; // disable caching for now, might add code to process if necessary
		return 0;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetPackedFactors_c;
	}

private:
	SphFactorHash_t * m_pHash {nullptr};

	int FetchHashEntry ( const CSphMatch & tMatch, const BYTE * & pData ) const
	{
		pData = nullptr;

		if ( !m_pHash || !m_pHash->GetLength() )
			return 0;

		DWORD uKey = FactorPoolHash ( RowTagged_t ( tMatch ), m_pHash->GetLength() );
		SphFactorHashEntry_t * pEntry = (*m_pHash)[ uKey ];
		assert ( pEntry );

		while ( pEntry && pEntry->m_tRow!=RowTagged_t ( tMatch ) )
			pEntry = pEntry->m_pNext;

		if ( !pEntry )
			return 0;

		pData = pEntry->m_pData;
		return int((BYTE *)pEntry - pEntry->m_pData);
	}
};


class Expr_BM25F_c : public Expr_NoLocator_c
{
public:
	Expr_BM25F_c ( float k1, float b, CSphVector<CSphNamedVariant> * pFieldWeights )
		: m_fK1 (k1)
		, m_fB ( b )
	{
		if ( pFieldWeights )
		{
			m_iCount = pFieldWeights->GetLength ();
			m_pFieldWeights = pFieldWeights->LeakData ();
		}
	}

	float Eval ( const CSphMatch & tMatch ) const final
	{
		if ( !m_pHash || !m_pHash->GetLength() )
			return 0.0f;

		DWORD uKey = FactorPoolHash ( RowTagged_t ( tMatch ), m_pHash->GetLength() );
		SphFactorHashEntry_t * pEntry = (*m_pHash)[ uKey ];
		assert ( pEntry );

		while ( pEntry && pEntry->m_tRow!=RowTagged_t ( tMatch ) )
			pEntry = pEntry->m_pNext;

		if ( !pEntry )
			return 0.0f;

		SPH_UDF_FACTORS tUnpacked;
		sphinx_factors_init ( &tUnpacked );
#ifndef NDEBUG
		Verify ( sphinx_factors_unpack ( (const unsigned int*)pEntry->m_pData, &tUnpacked )==0 );
#else
		sphinx_factors_unpack ( (const unsigned int*)pEntry->m_pData, &tUnpacked ); // fix MSVC Release warning
#endif

		// compute document length
		// OPTIMIZE? could precompute and store total dl in attrs, but at a storage cost
		// OPTIMIZE? could at least share between multiple BM25F instances, if there are many
		float dl = 0;
		CSphAttrLocator tLoc = m_tRankerState.m_tFieldLensLoc;
		if ( tLoc.m_iBitOffset>=0 )
		{
			for ( int i=0; i<m_tRankerState.m_iFields; i++ )
			{
				dl += tMatch.GetAttr ( tLoc ) * m_dWeights[i];
				tLoc.m_iBitOffset += 32;
			}
		}

		// compute (the current instance of) BM25F
		float fRes = 0.0f;
		for ( int iWord=0; iWord<m_tRankerState.m_iMaxQpos; iWord++ )
		{
			if ( !tUnpacked.term[iWord].keyword_mask )
				continue;

			// compute weighted TF
			float tf = 0.0f;
			for ( int i=0; i<m_tRankerState.m_iFields; i++ )
			{
				tf += tUnpacked.field_tf[ iWord + 1 + i * ( 1 + m_tRankerState.m_iMaxQpos ) ] * m_dWeights[i];
			}
			float idf = tUnpacked.term[iWord].idf; // FIXME? zeroed out for dupes!
			fRes += tf / ( tf + m_fK1 * ( 1.0f - m_fB + m_fB * dl / m_fWeightedAvgDocLen ) ) * idf;
		}

		sphinx_factors_deinit ( &tUnpacked );

		return fRes + 0.5f; // map to [0..1] range
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		if ( eCmd!=SPH_EXPR_SET_EXTRA_DATA )
			return;

		bool bGotHash = static_cast<ISphExtra*>(pArg)->ExtraData ( EXTRA_GET_DATA_PACKEDFACTORS, (void**)&m_pHash );
		if ( !bGotHash )
			return;

		bool bGotState = static_cast<ISphExtra*>(pArg)->ExtraData ( EXTRA_GET_DATA_RANKER_STATE, (void**)&m_tRankerState );
		if ( !bGotState )
			return;

		// bind weights
		m_dWeights.Resize ( m_tRankerState.m_iFields );
		m_dWeights.Fill ( 1 );
		if ( m_iCount )
		{
			for ( int i=0; i<m_iCount; ++i )
			{
				// FIXME? report errors if field was not found?
				CSphString & sField = m_pFieldWeights[i].m_sKey;
				int iField = m_tRankerState.m_pSchema->GetFieldIndex ( sField.cstr() );
				if ( iField>=0 )
					m_dWeights[iField] = m_pFieldWeights[i].m_iValue;
			}
		}

		// compute weighted avgdl
		m_fWeightedAvgDocLen = 1.0f;
		if ( m_tRankerState.m_pFieldLens )
		{
			m_fWeightedAvgDocLen = 0.0f;
			ARRAY_FOREACH ( i, m_dWeights )
				m_fWeightedAvgDocLen += m_tRankerState.m_pFieldLens[i] * m_dWeights[i];
		}
		m_fWeightedAvgDocLen /= m_tRankerState.m_iTotalDocuments;
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & bDisable ) final
	{
		bDisable = true; // disable caching for now, might add code to process if necessary
		return 0;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_BM25F_c ( *this );
	}

private:
	SphExtraDataRankerState_t	m_tRankerState;
	float						m_fK1 {0.0f};
	float						m_fB  {0.0f};
	float						m_fWeightedAvgDocLen {0.0f};
	CSphVector<int>				m_dWeights;		///< per field weights
	SphFactorHash_t *			m_pHash {nullptr};
	SharedPtrArr_t<CSphNamedVariant> m_pFieldWeights;
	int64_t						m_iCount = 0;

	Expr_BM25F_c ( const Expr_BM25F_c& rhs )
		: m_fK1 ( rhs.m_fK1 )
		, m_fB ( rhs.m_fB )
		, m_pFieldWeights ( rhs.m_pFieldWeights )
		, m_iCount ( rhs.m_iCount )
	{}
};


class Expr_GetWeight_c : public Expr_NoLocator_c
{
public:
	float Eval ( const CSphMatch & tMatch ) const final { return (float)tMatch.m_iWeight; }
	int IntEval ( const CSphMatch & tMatch ) const final { return (int)tMatch.m_iWeight; }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t)tMatch.m_iWeight; }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetWeight_c");
		return CALC_DEP_HASHES();
	}

	ISphExpr* Clone() const final
	{
		return new Expr_GetWeight_c;
	}
};

//////////////////////////////////////////////////////////////////////////

class Expr_Arglist_c : public ISphExpr
{
	friend void MoveToArgList ( ISphExpr * pLeft, VecRefPtrs_t<ISphExpr*> & dArgs );

public:
	Expr_Arglist_c ( ISphExpr * pLeft, ISphExpr * pRight )
	{
		AddArgs ( pLeft );
		AddArgs ( pRight );
	}

	bool IsArglist () const final
	{
		return true;
	}

	ISphExpr * GetArg ( int i ) const final
	{
		if ( i>=m_dArgs.GetLength() )
			return nullptr;
		return m_dArgs[i];
	}

	int GetNumArgs() const final
	{
		return m_dArgs.GetLength();
	}

	float Eval ( const CSphMatch & ) const final
	{
		assert ( 0 && "internal error: Eval() must not be explicitly called on arglist" );
		return 0.0f;
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final
	{
		for ( auto i : m_dArgs )
			i->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		for ( auto i : m_dArgs )
			i->Command ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & ) final
	{
		assert ( 0 && "internal error: GetHash() must not be explicitly called on arglist" );
		return 0;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_Arglist_c ( *this );
	}

private:
	VecRefPtrs_t<ISphExpr*> m_dArgs;

	void AddArgs ( ISphExpr * pExpr )
	{
		// not an arglist? just add it
		if ( !pExpr->IsArglist () )
		{
			m_dArgs.Add ( pExpr );
			SafeAddRef ( pExpr );
			return;
		}

		// arglist? take ownership of its args, and dismiss it
		auto * pArgs = ( Expr_Arglist_c * ) pExpr;
		m_dArgs.Append ( pArgs->m_dArgs );
		pArgs->m_dArgs.Reset ();
	}

	Expr_Arglist_c ( const Expr_Arglist_c& rhs )
	{
		m_dArgs.Resize ( rhs.m_dArgs.GetLength () );
		ARRAY_FOREACH ( i, m_dArgs )
			m_dArgs[i] = SafeClone (rhs.m_dArgs[i]);
	}
};

//////////////////////////////////////////////////////////////////////////

// very deep expression needs special stack for destroy
class Expr_ProxyFat_c final : public Expr_Unary_c
{
public:
	explicit Expr_ProxyFat_c ( ISphExpr * pExpr )
		: Expr_Unary_c ( "Expr_ProxyFat_c", pExpr )
	{}

	float Eval ( const CSphMatch & tMatch ) const final { return m_pFirst->Eval(tMatch); }

	int IntEval ( const CSphMatch & tMatch ) const final { return m_pFirst->IntEval(tMatch); }

	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return m_pFirst->Int64Eval(tMatch); }

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final { return m_pFirst->StringEval(tMatch,ppStr); }

	const BYTE * StringEvalPacked ( const CSphMatch & tMatch ) const final { return m_pFirst->StringEvalPacked(tMatch); }

	ByteBlob_t MvaEval ( const CSphMatch & tMatch) const final { return m_pFirst->MvaEval(tMatch); }

	const BYTE * FactorEval ( const CSphMatch & tMatch) const final { return m_pFirst->FactorEval(tMatch); }

	const BYTE * FactorEvalPacked ( const CSphMatch & tMatch) const final { return m_pFirst->FactorEvalPacked(tMatch); }

	bool IsArglist () const final { return m_pFirst->IsArglist(); }

	bool IsColumnar ( bool * pStored ) const final { return m_pFirst->IsColumnar(pStored); }

	bool IsDataPtrAttr () const final { return m_pFirst->IsDataPtrAttr(); }

	ISphExpr * GetArg ( int i ) const final { return m_pFirst->GetArg(i); }

	int GetNumArgs() const final { return m_pFirst->GetNumArgs(); }

	bool IsConst () const final { return m_pFirst->IsConst(); }

	bool IsJson ( bool & bConverted ) const final { return m_pFirst->IsJson( bConverted); }

	ISphExpr * Clone () const final { return new Expr_ProxyFat_c ( *this ); }

protected:
	Expr_ProxyFat_c ( const Expr_ProxyFat_c & ) = default;

	~Expr_ProxyFat_c() final
	{
		if ( !m_pFirst || !m_pFirst->IsLast() )
			return;

		auto iStackNeeded = Threads::GetStackUsed ();
		auto iCurStackSize = Threads::MyStackSize ();
		int iNeedInAdvance = session::GetDesiredStack ();
		if ( iNeedInAdvance<=0 )
			iStackNeeded = iCurStackSize * 2; // just from the fact that we're here, as ProxyFat is created by demand.
		else
			iStackNeeded += iNeedInAdvance * 2; // fixme! * 2 is not precise grade!

		if ( iStackNeeded<=iCurStackSize )
			return;

		// special deep-expression delete - take subexpr and release it from dedicated coro with increased stack.
		auto pExpr = m_pFirst.Leak();
		assert ( pExpr && pExpr->IsLast () );
		Threads::Coro::Continue ( (int) iStackNeeded, [pExpr] { pExpr->Release(); } );
	}
};

class Expr_StrLength_c : public Expr_Unary_c
{
public:
	explicit Expr_StrLength_c ( ISphExpr * pArg )
		: Expr_Unary_c ( "Expr_StrLength_c", pArg )
	{}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pStr = nullptr;
		int iLen = m_pFirst->StringEval ( tMatch, &pStr );
		FreeDataPtr ( *m_pFirst, pStr );
		return iLen;
	}

	float Eval ( const CSphMatch & tMatch ) const final { return (float)IntEval ( tMatch ); }

	ISphExpr * Clone () const final
	{
		return new Expr_StrLength_c ( *this );
	}

private:
	Expr_StrLength_c ( const Expr_StrLength_c& ) = default;
};


class Expr_Crc32_c : public Expr_Unary_c
{
public:
	explicit Expr_Crc32_c ( ISphExpr * pFirst )
		: Expr_Unary_c ( "Expr_Crc32_c", pFirst )
	{}

	float Eval ( const CSphMatch & tMatch ) const final { return (float)IntEval ( tMatch ); }
	int IntEval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pStr;
		int iLen = m_pFirst->StringEval ( tMatch, &pStr );
		DWORD uCrc = sphCRC32 ( pStr, iLen );
		FreeDataPtr ( *m_pFirst, pStr );
		return uCrc;
	}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t)(DWORD)IntEval ( tMatch ); }

	ISphExpr * Clone () const final
	{
		return new Expr_Crc32_c ( *this );
	}

private:
	Expr_Crc32_c ( const Expr_Crc32_c& ) = default;
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


class Expr_Fibonacci_c : public Expr_Unary_c
{
public:
	explicit Expr_Fibonacci_c ( ISphExpr * pFirst )
		: Expr_Unary_c ( "Expr_Fibonacci_c", pFirst )
	{}

	float Eval ( const CSphMatch & tMatch ) const final { return (float)IntEval ( tMatch ); }
	int IntEval ( const CSphMatch & tMatch ) const final { return Fibonacci ( m_pFirst->IntEval ( tMatch ) ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return IntEval ( tMatch ); }

	ISphExpr * Clone () const final
	{
		return new Expr_Fibonacci_c ( *this );
	}

private:
	Expr_Fibonacci_c ( const Expr_Fibonacci_c& ) = default;
};


class Expr_Concat_c : public Expr_NoLocator_c
{
public:
	explicit Expr_Concat_c ( const CSphVector<ISphExpr *> & dArgs, const CSphVector<bool> & dConstStr )
	{
		// pre-eval const strings
		CSphVector<const BYTE *> dEvaluated ( dArgs.GetLength() );
		m_dArgs.Resize ( dArgs.GetLength() );
		CSphMatch tMatch; // fake match
		ARRAY_FOREACH ( i, m_dArgs )
		{
			if ( dConstStr[i] )
				dArgs[i]->StringEval ( tMatch, &dEvaluated[i] );
			else
				dEvaluated[i] = nullptr;
		}

		// pre-concat const strings
		int iArg = 0;
		while ( iArg < m_dArgs.GetLength() )
		{
			StringOrExpr_t & tArg = m_dArgs[iArg];
			if ( dEvaluated[iArg] )
			{
				for ( ; iArg < dEvaluated.GetLength() && dEvaluated[iArg]; iArg++ )
				{
					auto iLen = (int) strlen ( (const char *)dEvaluated[iArg] );
					memcpy ( tArg.m_dBuffer.AddN (iLen), dEvaluated[iArg], iLen );
				}
			}
			else
				tArg.m_pExpr = dArgs[iArg++];
		}

		// remove gaps (if any)
		ARRAY_FOREACH ( i, m_dArgs )
			if ( !m_dArgs[i].m_dBuffer.GetLength() && !m_dArgs[i].m_pExpr )
				m_dArgs.Remove(i--);

		for ( auto & i : m_dArgs )
			SafeAddRef ( i.m_pExpr );
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		CSphVector<BYTE> dResult;
		int iResultLen = EvalMatch ( tMatch, dResult );
		*ppStr = dResult.LeakData();
		return iResultLen;
	}

	const BYTE * StringEvalPacked ( const CSphMatch & tMatch ) const final
	{
		// this is done to avoid reallocation while re-packing the result of StringEval call
		TightPackedVec_T<BYTE> dResult;
		int iResultLen = EvalMatch ( tMatch, dResult );
		sphPackPtrAttrInPlace ( dResult, iResultLen );
		return dResult.LeakData();
	}

	float Eval ( const CSphMatch & ) const final { assert ( 0 ); return 0; }

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		for ( auto & i : m_dArgs )
			if ( i.m_pExpr )
				i.m_pExpr->Command ( eCmd, pArg );
	}

	bool IsDataPtrAttr() const final
	{
		return true;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_Concat_c");
		for ( const auto & i : m_dArgs )
			if ( i.m_pExpr ) uHash = i.m_pExpr->GetHash ( tSorterSchema, uHash, bDisable );
		return CALC_DEP_HASHES();
	}

	ISphExpr* Clone() const final
	{
		return new Expr_Concat_c ( *this );
	}

protected:
	struct StringOrExpr_t
	{
		CSphVector<BYTE>			m_dBuffer;
		CSphRefcountedPtr<ISphExpr>	m_pExpr {nullptr};
	};

	CSphVector<StringOrExpr_t> m_dArgs;

	template<class POLICY, class LIMIT, class STORE>
	int EvalMatch ( const CSphMatch & tMatch, sph::Vector_T<BYTE, POLICY, LIMIT, STORE> & dResult ) const
	{
		for ( auto & i : m_dArgs )
		{
			const BYTE * pStr = nullptr;
			int iLen;
			if ( i.m_pExpr )
				iLen = i.m_pExpr->StringEval ( tMatch, &pStr );
			else
			{
				iLen = i.m_dBuffer.GetLength();
				pStr = i.m_dBuffer.Begin();
			}

			if ( pStr )
				dResult.Append ( const_cast<BYTE*>(pStr), iLen );

			FreeDataPtr ( i.m_pExpr, pStr );
		}

		dResult.Add('\0');
		return dResult.GetLength()-1;
	}

private:
	Expr_Concat_c ( const Expr_Concat_c& rhs )
	{
		m_dArgs.Resize ( rhs.m_dArgs.GetLength ());
		ARRAY_FOREACH ( i, m_dArgs )
		{
			m_dArgs[i].m_dBuffer = rhs.m_dArgs[i].m_dBuffer;
			m_dArgs[i].m_pExpr = SafeClone ( rhs.m_dArgs[i].m_pExpr );
		}
	}
};


class Expr_ToString_c : public Expr_Unary_c
{
public:
	Expr_ToString_c ( ISphExpr * pArg, ESphAttr eArg )
		: Expr_Unary_c ( "Expr_ToString_c", pArg )
		, m_eArg ( eArg )
	{}

	float Eval ( const CSphMatch & ) const final
	{
		assert ( 0 );
		return 0.0f;
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		m_sBuilder.Clear();
		uint64_t uPacked = 0;
		ESphJsonType eJson = JSON_NULL;
		uint64_t uOff = 0;
		int iLen = 0;

		switch ( m_eArg )
		{
			case SPH_ATTR_INTEGER:	m_sBuilder.Appendf ( "%u", m_pFirst->IntEval ( tMatch ) ); break;
			case SPH_ATTR_BIGINT:	m_sBuilder.Appendf ( INT64_FMT, m_pFirst->Int64Eval ( tMatch ) ); break;
			case SPH_ATTR_FLOAT:	m_sBuilder.Appendf ( "%f", m_pFirst->Eval ( tMatch ) ); break;
			case SPH_ATTR_UINT32SET:
			case SPH_ATTR_INT64SET:
			case SPH_ATTR_UINT32SET_PTR:
			case SPH_ATTR_INT64SET_PTR:
				{
					auto dMva = m_pFirst->MvaEval ( tMatch );
					sphMVA2Str ( dMva, m_eArg==SPH_ATTR_INT64SET || m_eArg==SPH_ATTR_INT64SET_PTR, m_sBuilder );
				}
				break;

			case SPH_ATTR_FLOAT_VECTOR:
			case SPH_ATTR_FLOAT_VECTOR_PTR:
				sphFloatVec2Str ( m_pFirst->MvaEval(tMatch), m_sBuilder );
				break;

			case SPH_ATTR_STRING:
			{
				CSphVector<BYTE> dTmp;
				iLen = m_pFirst->StringEval ( tMatch, ppStr );
				dTmp.Resize(iLen+1);
				if ( ppStr )
				{
					memcpy ( dTmp.Begin(), *ppStr, iLen );
					dTmp[iLen] = '\0';
				}
				else
					dTmp[0] = '\0';

				*ppStr = dTmp.LeakData();
				return iLen;
			}

			case SPH_ATTR_STRINGPTR:
				return m_pFirst->StringEval ( tMatch, ppStr );

			case SPH_ATTR_JSON_FIELD:
				uPacked = m_pFirst->Int64Eval ( tMatch );

				eJson = sphJsonUnpackType ( uPacked );
				uOff = sphJsonUnpackOffset ( uPacked );

				if ( !uOff || eJson==JSON_NULL )
				{
					*ppStr = nullptr;
					iLen = 0;
				} else
				{
					JsonEscapedBuilder dTmp;
					sphJsonFieldFormat ( dTmp, m_pBlobPool+uOff, eJson, false );
					iLen = dTmp.GetLength();
					*ppStr = dTmp.Leak();
				}
				return iLen;

			default:
				assert ( 0 && "unhandled arg type in TO_STRING()" );
				break;
		}

		if ( !m_sBuilder.GetLength() )
		{
			*ppStr = nullptr;
			return 0;
		}

		auto iRes = m_sBuilder.GetLength ();
		*ppStr = m_sBuilder.Leak();
		return iRes;
	}

	bool IsDataPtrAttr() const final
	{
		if ( m_eArg==SPH_ATTR_STRINGPTR )
			return m_pFirst->IsDataPtrAttr();

		return true;
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE*)pArg;

		m_pFirst->Command ( eCmd, pArg );
	}

	ISphExpr* Clone() const final
	{
		return new Expr_ToString_c ( *this );
	}

private:
	ESphAttr					m_eArg;
	const BYTE *				m_pBlobPool {nullptr};
	mutable StringBuilder_c		m_sBuilder;

private:
	Expr_ToString_c ( const Expr_ToString_c& rhs )
		: Expr_Unary_c ( rhs )
		, m_eArg ( rhs.m_eArg )
	{}
};

//////////////////////////////////////////////////////////////////////////

/// generic JSON value evaluation
/// can handle arbitrary stacks of jsoncol.key1.arr2[indexexpr3].key4[keynameexpr5]
/// m_dArgs holds the expressions that return actual accessors (either keynames or indexes)
/// m_dRetTypes holds their respective types
class Expr_JsonField_c : public Expr_WithLocator_c
{
public:
	/// takes over the expressions
	Expr_JsonField_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr, CSphVector<ISphExpr*> & dArgs, CSphVector<ESphAttr> & dRetTypes )
		: Expr_WithLocator_c ( tLocator, sAttr )
	{
		assert ( dArgs.GetLength()==dRetTypes.GetLength() );
		m_dArgs.SwapData ( dArgs );
		m_dRetTypes.SwapData ( dRetTypes );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		Expr_WithLocator_c::Command ( eCmd, pArg );

		switch ( eCmd )
		{
		case SPH_EXPR_SET_BLOB_POOL:
			m_pBlobPool = (const BYTE*)pArg;
			break;

		case SPH_EXPR_GET_DEPENDENT_COLS:
			if ( !m_sAttr.IsEmpty() )
				static_cast<StrVec_t*>(pArg)->Add(m_sAttr);
			break;

		case SPH_EXPR_FORMAT_AS_TEXT:
			if ( !m_sAttr.IsEmpty() && m_dArgs.all_of( []( auto & pExpr ){ return pExpr->IsConst(); } ) )
			{
				auto pSchemaWithName = static_cast<std::pair<const ISphSchema*,CSphString>*>(pArg);
				const CSphColumnInfo * pAttr = pSchemaWithName->first->GetAttr ( m_sAttr.cstr() );
				assert(pAttr);

				if ( m_dArgs.IsEmpty() )
					pSchemaWithName->second = pAttr->m_sName;
				else
				{
					CSphString sAllFields;
					ARRAY_FOREACH ( i, m_dArgs )
					{
						CSphMatch tStub;
						CSphString sArg;

						switch ( m_dRetTypes[i] )
						{
						case SPH_ATTR_INTEGER:
							sArg.SetSprintf ( "[%d]", m_dArgs[i]->IntEval(tStub) );
							break;

						case SPH_ATTR_BIGINT:
							sArg.SetSprintf ( "[" INT64_FMT "]", m_dArgs[i]->Int64Eval(tStub) );
							break;

						case SPH_ATTR_STRING:
						{
							const BYTE * pStr;
							int iLen = m_dArgs[i]->StringEval ( tStub, &pStr );
							sArg.SetSprintf ( "['%s']", CSphString ( (const char*)pStr, iLen ).cstr() );
						}
						break;

						default:
							break;
						}

						if ( sAllFields.IsEmpty() )
							sAllFields = sArg;
						else
							sAllFields.SetSprintf ( "%s%s", sAllFields.cstr(), sArg.cstr() );
					}

					pSchemaWithName->second.SetSprintf ( "%s%s", pAttr->m_sName.cstr(), sAllFields.cstr() );
				}
			}
			break;

		default:
			break;
		}

		for ( auto & pExpr : m_dArgs )
			if ( pExpr )
				pExpr->Command ( eCmd, pArg );
	}

	float Eval ( const CSphMatch & ) const final
	{
		assert ( 0 && "one just does not simply evaluate a JSON as float" );
		return 0;
	}

	int64_t DoEval ( ESphJsonType eJson, const BYTE * pVal, const CSphMatch & tMatch ) const
	{
		int iLen;
		const BYTE * pStr;

		ARRAY_FOREACH ( i, m_dRetTypes )
		{
			switch ( m_dRetTypes[i] )
			{
			case SPH_ATTR_INTEGER:	eJson = sphJsonFindByIndex ( eJson, &pVal, m_dArgs[i]->IntEval ( tMatch ) ); break;
			case SPH_ATTR_BIGINT:	eJson = sphJsonFindByIndex ( eJson, &pVal, (int)m_dArgs[i]->Int64Eval ( tMatch ) ); break;
			case SPH_ATTR_FLOAT:	eJson = sphJsonFindByIndex ( eJson, &pVal, (int)m_dArgs[i]->Eval ( tMatch ) ); break;
			case SPH_ATTR_STRING:
				// is this assert will fail someday it's ok
				// just remove it and add this code instead to handle possible memory leak
				// if ( m_dArgv[i]->IsDataPtrAttr() ) SafeDeleteArray ( pStr );
				assert ( !m_dArgs[i]->IsDataPtrAttr() );
				iLen = m_dArgs[i]->StringEval ( tMatch, &pStr );
				eJson = sphJsonFindByKey ( eJson, &pVal, (const void *)pStr, iLen, sphJsonKeyMask ( (const char *)pStr, iLen ) );
				break;
			case SPH_ATTR_JSON_FIELD: // handle cases like "json.a [ json.b ]"
				{
					uint64_t uPacked = m_dArgs[i]->Int64Eval ( tMatch );

					ESphJsonType eType = sphJsonUnpackType ( uPacked );
					const BYTE * p = m_pBlobPool + sphJsonUnpackOffset ( uPacked );

					switch ( eType )
					{
					case JSON_INT32:	eJson = sphJsonFindByIndex ( eJson, &pVal, sphJsonLoadInt ( &p ) ); break;
					case JSON_INT64:	eJson = sphJsonFindByIndex ( eJson, &pVal, (int)sphJsonLoadBigint ( &p ) ); break;
					case JSON_DOUBLE:	eJson = sphJsonFindByIndex ( eJson, &pVal, (int)sphQW2D ( sphJsonLoadBigint ( &p ) ) ); break;
					case JSON_STRING:
						iLen = sphJsonUnpackInt ( &p );
						eJson = sphJsonFindByKey ( eJson, &pVal, (const void *)p, iLen, sphJsonKeyMask ( (const char *)p, iLen ) );
						break;
					default:
						return 0;
					}
					break;
				}
			default:
				return 0;
			}

			if ( eJson==JSON_EOF )
				return 0;
		}

		// keep actual attribute type and offset to data packed
		return sphJsonPackTypeOffset ( eJson, pVal-m_pBlobPool );
	}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const override
	{
		if ( !m_pBlobPool )
			return 0;

		if ( m_tLocator.m_bDynamic )
		{
			// extends precalculated (aliased) field
			uint64_t uPacked = tMatch.GetAttr ( m_tLocator );
			if ( !uPacked )
				return 0;

			ESphJsonType eType = sphJsonUnpackType ( uPacked );
			const BYTE * pVal = m_pBlobPool + sphJsonUnpackOffset ( uPacked );

			return DoEval ( eType, pVal, tMatch );
		}

		int iLengthBytes = 0;
		const BYTE * pVal = sphGetBlobAttr ( tMatch, m_tLocator, m_pBlobPool, iLengthBytes );
		if ( !pVal )
			return 0;

		ESphJsonType eJson = sphJsonFindFirst ( &pVal );
		return DoEval ( eJson, pVal, tMatch );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_JsonField_c");
		CALC_POD_HASHES(m_dRetTypes);
		CALC_CHILD_HASHES(m_dArgs);
		return CALC_DEP_HASHES();
	}

	bool IsJson ( bool & bConverted ) const final
	{
		bConverted = false;
		return true;
	}

	ISphExpr* Clone() const override
	{
		return new Expr_JsonField_c ( *this );
	}

protected:
	const BYTE *			m_pBlobPool {nullptr};

private:
	VecRefPtrs_t<ISphExpr*>	m_dArgs;
	CSphVector<ESphAttr>	m_dRetTypes;

protected:
	Expr_JsonField_c ( const Expr_JsonField_c & rhs )
		: Expr_WithLocator_c ( rhs )
		, m_dRetTypes ( rhs.m_dRetTypes )
	{
		m_dArgs.Resize ( rhs.m_dArgs.GetLength ());
		ARRAY_FOREACH ( i, m_dArgs )
			m_dArgs[i] = SafeClone (rhs.m_dArgs[i]);
	}
};


/// fastpath (instead of generic JsonField_c) for jsoncol.key access by a static key name
class Expr_JsonFastKey_c : public Expr_WithLocator_c
{
public:
	/// takes over the expressions
	Expr_JsonFastKey_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr, ISphExpr * pArg )
		: Expr_WithLocator_c ( tLocator, sAttr )
	{
		auto * pKey = (Expr_GetStrConst_c*)pArg;
		m_sKey = pKey->GetStr();
		m_iKeyLen = m_sKey.Length();
		m_uKeyBloom = sphJsonKeyMask ( m_sKey.cstr(), m_iKeyLen );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_WithLocator_c::Command ( eCmd, pArg );

		switch ( eCmd )
		{
		case SPH_EXPR_SET_BLOB_POOL:
			m_pBlobPool = (const BYTE*)pArg;
			break;

		case SPH_EXPR_FORMAT_AS_TEXT:
			if ( !m_sAttr.IsEmpty() )
			{
				auto pSchemaWithName = static_cast<std::pair<const ISphSchema*,CSphString>*>(pArg);
				const CSphColumnInfo * pAttr = pSchemaWithName->first->GetAttr ( m_sAttr.cstr() );
				assert(pAttr);
				pSchemaWithName->second.SetSprintf ( "%s['%s']", pAttr->m_sName.cstr(), m_sKey.cstr() );
			}
			break;

		default:
			break;
		}
	}

	float Eval ( const CSphMatch & ) const final
	{
		assert ( 0 && "one just does not simply evaluate a JSON as float" );
		return 0;
	}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const final
	{
		// get pointer to JSON blob data
		const BYTE * pJson = sphGetBlobAttr ( tMatch, m_tLocator, m_pBlobPool ).first;
		if ( !pJson )
			return 0;

		// all root objects start with a Bloom mask; quickly check it
		if ( ( sphGetDword(pJson) & m_uKeyBloom )!=m_uKeyBloom )
			return 0;

		// OPTIMIZE? FindByKey does an extra (redundant) bloom check inside
		ESphJsonType eJson = sphJsonFindByKey ( JSON_ROOT, &pJson, m_sKey.cstr(), m_iKeyLen, m_uKeyBloom );
		if ( eJson==JSON_EOF )
			return 0;

		// keep actual attribute type and offset to data packed
		return sphJsonPackTypeOffset ( eJson, pJson-m_pBlobPool );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_JsonFastKey_c");
		CALC_STR_HASH(m_sKey,m_iKeyLen);
		return CALC_DEP_HASHES();
	}

	bool IsJson ( bool & bConverted ) const final
	{
		bConverted = false;
		return true;
	}

	ISphExpr* Clone() const final
	{
		return new Expr_JsonFastKey_c ( *this );
	}

protected:
	const BYTE *	m_pBlobPool {nullptr};
	CSphString		m_sKey;
	int				m_iKeyLen {0};
	DWORD			m_uKeyBloom {0};

private:
	Expr_JsonFastKey_c ( const Expr_JsonFastKey_c & rhs )
		: Expr_WithLocator_c ( rhs )
		, m_sKey ( rhs.m_sKey )
		, m_iKeyLen ( rhs.m_iKeyLen )
		, m_uKeyBloom ( rhs.m_uKeyBloom )
	{}
};


static ESphJsonType GetKey ( const BYTE ** ppKey, const CSphMatch & tMatch, const BYTE * pBlobPool, const CSphRefcountedPtr<ISphExpr> & pArg )
{
	assert ( ppKey );
	if ( !pBlobPool || !pArg )
		return JSON_EOF;

	uint64_t uPacked = pArg->Int64Eval ( tMatch );
	*ppKey = pBlobPool + sphJsonUnpackOffset ( uPacked );

	return sphJsonUnpackType ( uPacked );
}


class Expr_JsonFieldConv_c : public ISphExpr
{
public:
	explicit Expr_JsonFieldConv_c ( ISphExpr * pArg )
		: m_pArg { pArg }
	{
		SafeAddRef ( pArg );
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const override
	{
		const BYTE * pVal = nullptr;
		ESphJsonType eJson = GetKey ( &pVal, tMatch );
		if ( eJson!=JSON_STRING)
			return 0;

		//      using sphUnpackStr() is wrong, because BSON uses different store format of string length
		int iLen = sphJsonUnpackInt ( &pVal );
		*ppStr = pVal;
		return iLen;
	}

	float Eval ( const CSphMatch & tMatch ) const override { return DoEval<float> ( tMatch ); }
	int IntEval ( const CSphMatch & tMatch ) const override { return DoEval<int> ( tMatch ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const override { return DoEval<int64_t> ( tMatch ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) override
	{
		EXPR_CLASS_NAME("Expr_JsonFieldConv_c");
		return CALC_PARENT_HASH();
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE*)pArg;
		if ( m_pArg )
			m_pArg->Command ( eCmd, pArg );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		if ( m_pArg )
			m_pArg->FixupLocator ( pOldSchema, pNewSchema );
	}

	ISphExpr* Clone() const override
	{
		return new Expr_JsonFieldConv_c ( *this );
	}

protected:
	const BYTE *	m_pBlobPool {nullptr};
	CSphRefcountedPtr<ISphExpr> m_pArg;

	ESphJsonType GetKey ( const BYTE ** ppKey, const CSphMatch & tMatch ) const
	{
		return ::GetKey ( ppKey, tMatch, m_pBlobPool, m_pArg );
	}

	// generic evaluate
	template < typename T >
	T DoEval ( const CSphMatch & tMatch ) const
	{
		const BYTE * pVal = nullptr;
		ESphJsonType eJson = GetKey ( &pVal, tMatch );
		switch ( eJson )
		{
		case JSON_INT32:	return (T)sphJsonLoadInt ( &pVal );
		case JSON_INT64:	return (T)sphJsonLoadBigint ( &pVal );
		case JSON_DOUBLE:	return (T)sphQW2D ( sphJsonLoadBigint ( &pVal ) );
		case JSON_TRUE:		return 1;
		case JSON_STRING:
		{
			int iLen = sphJsonUnpackInt ( &pVal );
			int64_t iVal;
			double fVal;
			ESphJsonType eType;
			if ( sphJsonStringToNumber ( (const char*)pVal, iLen, eType, iVal, fVal ) )
				return eType==JSON_DOUBLE ? (T)fVal : (T)iVal;
			return 0;
		}
		default:			return 0;
		}
	}

	uint64_t CalcHash ( const char * szTag, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
	{
		EXPR_CLASS_NAME_NOCHECK(szTag);
		CALC_CHILD_HASH(m_pArg);
		return CALC_DEP_HASHES();
	}

	
	bool IsJson ( bool & bConverted ) const final
	{
		bConverted = true;
		return true;
	}

	Expr_JsonFieldConv_c ( const Expr_JsonFieldConv_c& rhs )
		: m_pArg { SafeClone (rhs.m_pArg) }
	{}
};


template <typename T>
T JsonAggr ( ESphJsonType eJson, const BYTE * pVal, ESphAggrFunc eFunc, CSphString * pBuf )
{
	if ( !pVal || ( eFunc!=SPH_AGGR_MIN && eFunc!=SPH_AGGR_MAX ) )
		return 0;

	switch ( eJson )
	{
	case JSON_INT32_VECTOR:
		{
			int iVals = sphJsonUnpackInt ( &pVal );
			if ( iVals==0 )
				return 0;

			auto * p = (const int*) pVal;
			int iRes = *p; // first value

			switch ( eFunc )
			{
			case SPH_AGGR_MIN: while ( --iVals ) if ( *++p<iRes ) iRes = *p; break;
			case SPH_AGGR_MAX: while ( --iVals ) if ( *++p>iRes ) iRes = *p; break;
			default: return 0;
			}
			return (T)iRes;
		}
	case JSON_DOUBLE_VECTOR:
		{
			int iLen = sphJsonUnpackInt ( &pVal );
			if ( !iLen || ( eFunc!=SPH_AGGR_MIN && eFunc!=SPH_AGGR_MAX ) )
				return 0;

			double fRes = ( eFunc==SPH_AGGR_MIN ? FLT_MAX : FLT_MIN );
			const BYTE * p = pVal;
			for ( int i=0; i<iLen; i++ )
			{
				double fStored = sphQW2D ( sphJsonLoadBigint ( &p ) );
				switch ( eFunc )
				{
				case SPH_AGGR_MIN:
					fRes = Min ( fRes, fStored );
					break;
				case SPH_AGGR_MAX:
					fRes = Max ( fRes, fStored );
					break;
				default: return 0;
				}
			}
			return (T)fRes;
		}
	case JSON_STRING_VECTOR:
		{
			if ( !pBuf )
				return 0;

			sphJsonUnpackInt ( &pVal ); // skip node length

			int iVals = sphJsonUnpackInt ( &pVal );
			if ( iVals==0 )
				return 0;

			// first value
			int iLen = sphJsonUnpackInt ( &pVal );
			auto * pRes = (const char* )pVal;
			int iResLen = iLen;

			while ( --iVals )
			{
				pVal += iLen;
				iLen = sphJsonUnpackInt ( &pVal );

				// binary string comparison
				int iCmp = memcmp ( pRes, (const char*)pVal, iLen<iResLen ? iLen : iResLen );
				if ( iCmp==0 && iLen!=iResLen )
					iCmp = iResLen-iLen;

				if ( ( eFunc==SPH_AGGR_MIN && iCmp>0 ) || ( eFunc==SPH_AGGR_MAX && iCmp<0 ) )
				{
					pRes = (const char*)pVal;
					iResLen = iLen;
				}
			}

			pBuf->SetBinary ( pRes, iResLen );
			return (T)iResLen;
		}
	case JSON_MIXED_VECTOR:
		{
			sphJsonUnpackInt ( &pVal ); // skip node length
			int iLen = sphJsonUnpackInt ( &pVal );
			if ( !iLen || ( eFunc!=SPH_AGGR_MIN && eFunc!=SPH_AGGR_MAX ) )
				return 0;

			double fRes = ( eFunc==SPH_AGGR_MIN ? FLT_MAX : FLT_MIN );
			for ( int i=0; i<iLen; i++ )
			{
				double fVal = ( eFunc==SPH_AGGR_MIN ? FLT_MAX : FLT_MIN );

				auto eType = (ESphJsonType)*pVal++;
				switch (eType)
				{
					case JSON_INT32:
					case JSON_INT64:
						fVal = (double)( eType==JSON_INT32 ? sphJsonLoadInt ( &pVal ) : sphJsonLoadBigint ( &pVal ) );
					break;
					case JSON_DOUBLE:
						fVal = sphQW2D ( sphJsonLoadBigint ( &pVal ) );
						break;
					default:
						sphJsonSkipNode ( eType, &pVal );
						break; // for weird subobjects, just let min
				}

				switch ( eFunc )
				{
				case SPH_AGGR_MIN:
					fRes = Min ( fRes, fVal );
					break;
				case SPH_AGGR_MAX:
					fRes = Max ( fRes, fVal );
					break;
				default: return 0;
				}
			}
			return (T)fRes;
		}
	default: return 0;
	}
}


class Expr_JsonFieldAggr_c : public Expr_JsonFieldConv_c
{
public:
	Expr_JsonFieldAggr_c ( ISphExpr * pArg, ESphAggrFunc eFunc )
		: Expr_JsonFieldConv_c ( pArg )
		, m_eFunc ( eFunc )
	{}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pVal = nullptr;
		ESphJsonType eJson = GetKey ( &pVal, tMatch );
		return JsonAggr<int> ( eJson, pVal, m_eFunc, nullptr );
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		CSphString sBuf;
		*ppStr = nullptr;
		const BYTE * pVal = nullptr;
		ESphJsonType eJson = GetKey ( &pVal, tMatch );

		int iLen = 0;
		int iVal = 0;
		float fVal = 0.0f;
		
		switch ( eJson )
		{
		case JSON_INT32_VECTOR:
			iVal = JsonAggr<int> ( eJson, pVal, m_eFunc, nullptr );
			sBuf.SetSprintf ( "%u", iVal );
			iLen = sBuf.Length();
			*ppStr = (const BYTE *) sBuf.Leak();
			return iLen;

		case JSON_STRING_VECTOR:
			JsonAggr<int> ( eJson, pVal, m_eFunc, &sBuf );
			iLen = sBuf.Length();
			*ppStr = (const BYTE *) sBuf.Leak();
			return iLen;

		case JSON_DOUBLE_VECTOR:
		case JSON_MIXED_VECTOR:
			fVal = JsonAggr<float> ( eJson, pVal, m_eFunc, nullptr );
			sBuf.SetSprintf ( "%f", fVal );
			iLen = sBuf.Length();
			*ppStr = (const BYTE *) sBuf.Leak();
			return iLen;

		default: return 0;
		}
	}

	float Eval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pVal = nullptr;
		ESphJsonType eJson = GetKey ( &pVal, tMatch );
		return JsonAggr<float> ( eJson, pVal, m_eFunc, nullptr );
	}
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pVal = nullptr;
		ESphJsonType eJson = GetKey ( &pVal, tMatch );
		return JsonAggr<int64_t> ( eJson, pVal, m_eFunc, nullptr );
	}

	bool IsDataPtrAttr() const final { return true; }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_JsonFieldAggr_c");
		CALC_POD_HASH(m_eFunc);
		return CALC_PARENT_HASH();
	}

	ISphExpr* Clone() const final
	{
		return new Expr_JsonFieldAggr_c ( *this );
	}

protected:
	ESphAggrFunc m_eFunc{SPH_AGGR_NONE};

	Expr_JsonFieldAggr_c ( const Expr_JsonFieldAggr_c& ) = default;
};


class Expr_JsonFieldLength_c : public Expr_JsonFieldConv_c
{
public:
	explicit Expr_JsonFieldLength_c ( ISphExpr * pArg )
		: Expr_JsonFieldConv_c ( pArg )
	{}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pVal = nullptr;
		ESphJsonType eJson = GetKey ( &pVal, tMatch );
		return sphJsonFieldLength ( eJson, pVal );
	}

	float	Eval ( const CSphMatch & tMatch ) const final { return (float)IntEval ( tMatch ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t)IntEval ( tMatch ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_JsonFieldLength_c");
		return CALC_PARENT_HASH();
	}

	ISphExpr* Clone() const final
	{
		return new Expr_JsonFieldLength_c ( *this );
	}

private:
	Expr_JsonFieldLength_c ( const Expr_JsonFieldLength_c& ) = default;
};


class Expr_SubstringIndex_c : public ISphStringExpr
{
private:
	CSphRefcountedPtr<ISphExpr> m_pArg;
	CSphString m_sDelim;
	int m_iCount = 0;
	int m_iLenDelim = 0;
	bool m_bFreeResPtr = false;

public:
	explicit Expr_SubstringIndex_c ( ISphExpr * pArg, ISphExpr * pDelim, ISphExpr * pCount )
		: m_pArg ( pArg )
		, m_iCount ( 0 )
		, m_bFreeResPtr ( false )
	{
		assert ( pArg && pDelim && pCount );
		SafeAddRef ( pArg );
		m_bFreeResPtr = m_pArg->IsDataPtrAttr();

		const BYTE * pBuf = nullptr;
		CSphMatch tTmp;
		
		m_iLenDelim = pDelim->StringEval ( tTmp, &pBuf );
		m_sDelim.SetBinary ( (const char *)pBuf, m_iLenDelim );
		FreeDataPtr ( *pDelim, pBuf );

		m_iCount = pCount->IntEval ( tTmp );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		if ( m_pArg )
			m_pArg->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		if ( m_pArg )
			m_pArg->Command ( eCmd, pArg );
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		const char* pDoc = nullptr;
		int iDocLen = m_pArg->StringEval ( tMatch, (const BYTE **)&pDoc );
		int iLength = 0;
		*ppStr = nullptr;

		if ( pDoc && iDocLen>0 && m_iLenDelim>0 && m_iCount!=0 )
		{
			if ( m_iCount>0 )
				LeftSearch ( pDoc, iDocLen, m_iCount, false, ppStr, &iLength );
			else
				RightSearch ( pDoc, iDocLen, m_iCount, ppStr, &iLength );
		}

		FreeDataPtr ( *m_pArg, pDoc );

		return iLength;
	}

	bool IsDataPtrAttr() const final
	{
		return m_bFreeResPtr;
	}

	bool IsConst () const final { return false; }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_SubstringIndex_c");
		CALC_CHILD_HASH(m_pArg);
		CALC_POD_HASH(m_sDelim);
		CALC_POD_HASH(m_iCount);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_SubstringIndex_c ( *this );
	}

private:
	int SetResultString ( const char * pDoc, int iDocLen, const BYTE ** ppResStr ) const;
	int LeftSearch ( const char * pDoc, int iDocLen, int iCount, bool bGetRight, const BYTE ** ppResStr, int * pResLen ) const;
	int RightSearch ( const char * pDoc, int iDocLen, int iCount, const BYTE ** ppResStr, int * pResLen ) const;

	Expr_SubstringIndex_c ( const Expr_SubstringIndex_c& rhs )
		: m_pArg ( SafeClone (rhs.m_pArg) )
		, m_sDelim ( rhs.m_sDelim )
		, m_iCount ( rhs.m_iCount )
		, m_iLenDelim ( rhs.m_iLenDelim )
		, m_bFreeResPtr ( rhs.m_bFreeResPtr )
	{}
};

//	in case of input static string, function returns only pointer and length of substring. buffer is not allocated
//	in case of input dynamic string, function allocates buffer for substring and copy substring to it
int Expr_SubstringIndex_c::SetResultString ( const char * pDoc, int iDocLen, const BYTE ** ppResStr ) const
{
	if ( !IsDataPtrAttr() )
	{
		*ppResStr = (const BYTE *) pDoc;
	}
	else
	{
		CSphString  sRetVal;
		sRetVal.SetBinary ( pDoc, iDocLen );
		*ppResStr = (const BYTE *) sRetVal.Leak();
	}
	return iDocLen;
}

int Expr_SubstringIndex_c::LeftSearch ( const char * pDoc, int iDocLen, int iCount, bool bGetRight, const BYTE ** ppResStr, int * pResLen ) const
{
	int	  iTotalDelim = 0;
	const char * pDelBeg = m_sDelim.cstr();
	const char * pDelEnd = pDelBeg + m_iLenDelim;
	const char * pStrBeg = pDoc;
	const char * pStrEnd = (pStrBeg + iDocLen) - m_iLenDelim + 1;

	while ( pStrBeg<pStrEnd )
	{
		//	check first delimer string's char with current char from pStr
		if ( *pStrBeg==*pDelBeg )
		{
			//	first char is found, now we compare next chars in delimer string
			bool	bMatched = true;
			const char * p1 = pStrBeg + 1;
			const char * p2 = pDelBeg + 1;
			while ( bMatched && p2!=pDelEnd )
			{
				if ( *p1!=*p2 )
					bMatched = false;
				p1++;
				p2++;
			}

			//	if we found matched delimer string, then return left substring or search next delimer string
			if ( bMatched )
			{
				iTotalDelim++;
				iCount--;

				if ( iCount==0 )
				{
					if ( ppResStr && !bGetRight )
						*pResLen = SetResultString ( pDoc, int ( pStrBeg - pDoc ), ppResStr );

					if ( ppResStr && bGetRight )
					{
						pStrBeg += m_iLenDelim;
						*pResLen = SetResultString ( pStrBeg, iDocLen - int (pStrBeg - pDoc), ppResStr );
					}

					return iTotalDelim;
				}
				pStrBeg += m_iLenDelim;
				continue;
			}
		}

		//	delimer string does not maatch with current ptr, goto to next char and repeat comparation
		int  iCharLen = sphUTF8Len ( pStrBeg, 1 );
		pStrBeg += ( iCharLen > 0 ) ? iCharLen : 1;
	}

	//	not found, return original string
	if ( iCount && ppResStr )
		*pResLen = SetResultString ( pDoc, iDocLen, ppResStr );

	return iTotalDelim;
}

int Expr_SubstringIndex_c::RightSearch ( const char * pDoc, int iDocLen, int iCount, const BYTE ** ppResStr, int * pResLen ) const
{
	//	find and count (iNumFoundDelim) of all delimer sub strings
	int  iNumFoundDelim = LeftSearch ( pDoc, iDocLen, iDocLen+1, false, NULL, NULL );

	//	correct iCount (which is negative) to positive index from left to right
	iCount += iNumFoundDelim + 1;

	//	if not found, return original string
	if ( iCount<=0 )
		*pResLen = SetResultString ( pDoc, iDocLen, ppResStr );

	//	find delimer sub string according to iCount and return result
	return LeftSearch ( pDoc, iDocLen, iCount, true, ppResStr, pResLen );
}

class ExprCaseBase_c : public ISphStringExpr
{
public:
	explicit ExprCaseBase_c ( ISphExpr * pArg, const char * sClassName )
		: m_pArg ( pArg )
		, m_sClassName ( sClassName )
	{
		assert( pArg );
		SafeAddRef( pArg );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		if ( m_pArg )
			m_pArg->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		if ( m_pArg )
			m_pArg->Command ( eCmd, pArg );
	}

	bool IsDataPtrAttr() const final
	{
		return true;
	}

	bool IsConst () const final { return false; }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME(m_sClassName);
		CALC_CHILD_HASH(m_pArg);
		return CALC_DEP_HASHES();
	}

protected:
	CSphRefcountedPtr<ISphExpr> m_pArg;

	ExprCaseBase_c ( const ExprCaseBase_c & rhs )
		: m_pArg ( SafeClone ( rhs.m_pArg ) )
		, m_sClassName ( rhs.m_sClassName )
	{}

private:
	const char * m_sClassName = nullptr;
};

template<bool UPPER>
class ExprCaseTrival_c : public ExprCaseBase_c
{
public:
	explicit ExprCaseTrival_c ( ISphExpr * pArg )
		: ExprCaseBase_c ( pArg, "ExprCaseTrival_c" )
	{
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		const char * pDoc = nullptr;
		int iDocLen = m_pArg->StringEval ( tMatch, (const BYTE **)&pDoc );
		*ppStr = nullptr;

		// create CSphVector and store the value
		CSphVector<BYTE> dStrBuffer;
		dStrBuffer.Append ( pDoc, iDocLen );

		BYTE * pStrBeg = dStrBuffer.begin();
		const BYTE * pStrEnd = ( pStrBeg + iDocLen );

		if ( pDoc && iDocLen>0 )
		{
			while( pStrBeg<pStrEnd )
			{
				// convert the current character to its uppercase or lowercase version if it exists
				DoCase ( (char *)pStrBeg );
				pStrBeg++;
			}
		}

		*ppStr = dStrBuffer.LeakData();
		FreeDataPtr ( *m_pArg, pDoc );

		// return the resultant string
		return iDocLen;
	}

	ISphExpr * Clone () const final
	{
		return new ExprCaseTrival_c ( *this );
	}

private:
	void DoCase ( char * pString ) const;

	ExprCaseTrival_c ( const ExprCaseTrival_c & rhs )
		: ExprCaseBase_c ( rhs )
	{}
};

// For upper() function
template<>
void ExprCaseTrival_c<true>::DoCase ( char * pString ) const
{
	*pString = toupper ( *pString );
}

// For lower() function
template<>
void ExprCaseTrival_c<false>::DoCase ( char * pString ) const
{
	*pString = tolower ( *pString );
}

void UTF8ToLower( std::vector<char> & dBuf, std::basic_string_view<char> source )
{
	return una::detail::t_map<std::vector<char>, std::basic_string_view<char>, una::detail::impl_x_case_map_utf8, una::detail::impl_case_map_loc_utf8> ( dBuf, source, una::detail::impl_case_map_mode_lowercase );
}

void UTF8ToUpper( std::vector<char> & dBuf, std::basic_string_view<char> source )
{
	return una::detail::t_map<std::vector<char>, std::basic_string_view<char>, una::detail::impl_x_case_map_utf8, una::detail::impl_case_map_loc_utf8> ( dBuf, source, una::detail::impl_case_map_mode_uppercase );
}

template<bool UPPER>
class ExprCaseComplex_c : public ExprCaseBase_c
{
public:
	explicit ExprCaseComplex_c ( ISphExpr * pArg )
		: ExprCaseBase_c ( pArg, "ExprCaseComplex_c" )
	{
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		*ppStr = nullptr;

		const char * pSrcDoc = nullptr;
		int iSrcDocLen = m_pArg->StringEval ( tMatch, (const BYTE **)&pSrcDoc );

		std::vector<char> & dBuf = const_cast<std::vector<char> &> ( m_dBuf );
		dBuf.resize ( 0 );
		if ( UPPER )
			UTF8ToUpper ( dBuf, std::basic_string_view<char> ( pSrcDoc, iSrcDocLen ) );
		else
			UTF8ToLower ( dBuf, std::basic_string_view<char> ( pSrcDoc, iSrcDocLen ) );

		int iDstDocLen = dBuf.size();
		CSphFixedVector<BYTE> dDst ( iDstDocLen );
		memcpy ( dDst.Begin(), dBuf.data(), iDstDocLen );
		*ppStr = dDst.LeakData();

		// return the resultant string
		return iDstDocLen;
	}

	ISphExpr * Clone () const final
	{
		return new ExprCaseComplex_c ( *this );
	}

private:

	ExprCaseComplex_c ( const ExprCaseComplex_c & rhs )
		: ExprCaseBase_c ( rhs )
	{}

	std::vector<char> m_dBuf;
};

class Expr_Iterator_c : public Expr_JsonField_c
{
public:
	Expr_Iterator_c ( const CSphAttrLocator & tLocator, const CSphString & sAttr, CSphVector<ISphExpr*> & dArgs, CSphVector<ESphAttr> & dRetTypes, SphAttr_t * pData )
		: Expr_JsonField_c ( tLocator, sAttr, dArgs, dRetTypes )
		, m_pData ( pData )
	{}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const final
	{
		uint64_t uPacked = m_pData ? *m_pData : 0;

		ESphJsonType eType = sphJsonUnpackType ( uPacked );
		const BYTE * pVal = m_pBlobPool + sphJsonUnpackOffset ( uPacked );

		return DoEval ( eType, pVal, tMatch );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_JsonField_c::Command ( eCmd, pArg );
		if ( eCmd==SPH_EXPR_SET_ITERATOR )
			m_pData = (SphAttr_t *) pArg;
	}

	ISphExpr* Clone() const final
	{
		return new Expr_Iterator_c ( *this );
		// note that m_pData most probably wrong and need to be set with SPH_EXPR_SET_ITERATOR
	}

private:
	SphAttr_t * m_pData {nullptr};

	Expr_Iterator_c ( const Expr_Iterator_c& ) = default;
};

////////////////////////////////////////////////////////////////////

class ExprConstArgs_c : public CSphFilterSettings
{
public:
	bool	m_bOr = false;
	bool	m_bAnd = false;

	void	AppendFilter ( const CSphFilterSettings & tFilter );
	bool	IsError() const { return !m_bSet || m_bError; }

private:
	bool	m_bSet = false;
	bool	m_bError = false;
};


void ExprConstArgs_c::AppendFilter ( const CSphFilterSettings & tFilter )
{
	if ( !m_bSet )
	{
		*(CSphFilterSettings*)this = tFilter;
		m_bSet = true;
		return;
	}

	if ( m_eType!=tFilter.m_eType || m_bExclude || tFilter.m_bExclude )
	{
		m_bError = true;
		return;
	}

	switch ( m_eType )
	{
	case SPH_FILTER_STRING_LIST:
		for ( auto & i : tFilter.m_dStrings )
			m_dStrings.Add(i);
		break;

	case SPH_FILTER_VALUES:
		for ( auto & i : tFilter.m_dValues )
			m_dValues.Add(i);
		break;

	case SPH_FILTER_RANGE:
	case SPH_FILTER_FLOATRANGE:
		m_iMinValue = Max ( m_iMinValue, tFilter.m_iMinValue );
		m_iMaxValue = Min ( m_iMaxValue, tFilter.m_iMaxValue );
		m_fMinValue = Max ( m_fMinValue, tFilter.m_fMinValue );
		m_fMaxValue = Min ( m_fMaxValue, tFilter.m_fMaxValue );
		m_bHasEqualMin |= tFilter.m_bHasEqualMin;
		m_bHasEqualMax |= tFilter.m_bHasEqualMax;
		m_bOpenLeft	 &= tFilter.m_bOpenLeft;
		m_bOpenRight &= tFilter.m_bOpenRight;
		break;

	default:
		m_bError = true;
		break;
	}
}

////////////////////////////////////////////////////////////////////
class Expr_BinaryFilter_c : public Expr_Binary_c
{
	using Expr_Binary_c::Expr_Binary_c;

protected:
	void		PopulateConstArgsStr ( ESphExprCommand eCmd, void * pArg ) const;
	void		PopulateConstArgsLtInt ( ESphExprCommand eCmd, void * pArg ) const;
	void		PopulateConstArgsGtInt ( ESphExprCommand eCmd, void * pArg ) const;
	void		PopulateConstArgsLteInt ( ESphExprCommand eCmd, void * pArg ) const;
	void		PopulateConstArgsGteInt ( ESphExprCommand eCmd, void * pArg ) const;
	void		PopulateConstArgsEqInt ( ESphExprCommand eCmd, void * pArg ) const;
	void		PopulateConstArgsNeInt ( ESphExprCommand eCmd, void * pArg ) const;
	void		SetFlagAnd ( ESphExprCommand eCmd, void * pArg ) const;
	void		SetFlagOr ( ESphExprCommand eCmd, void * pArg ) const;

private:
	const ISphExpr * GetExprForConsts ( ESphExprCommand eCmd ) const;

	template <typename ACTION>
	void		AddFilter ( ESphExprCommand eCmd, void * pArg, ACTION && fnAction ) const;
};

template <typename ACTION>
void Expr_BinaryFilter_c::AddFilter ( ESphExprCommand eCmd, void * pArg, ACTION && fnAction ) const
{
	const ISphExpr * pToEval = GetExprForConsts(eCmd);
	if ( !pToEval )
		return;

	CSphFilterSettings tFilter;
	CSphMatch tStub;
	fnAction ( tFilter, tStub, pToEval );
	((ExprConstArgs_c*)pArg)->AppendFilter(tFilter);
}


void Expr_BinaryFilter_c::PopulateConstArgsStr ( ESphExprCommand eCmd, void * pArg ) const
{
	AddFilter ( eCmd, pArg, []( CSphFilterSettings & tFilter, const CSphMatch & tMatch, const ISphExpr * pExpr )
		{
			const BYTE * pStr = nullptr;
			int iLen = pExpr->StringEval ( tMatch, &pStr );
			tFilter.m_eType = SPH_FILTER_STRING_LIST;
			tFilter.m_dStrings.Add ( CSphString ( (const char*)pStr, iLen ) );
		} );
}


void Expr_BinaryFilter_c::PopulateConstArgsLtInt ( ESphExprCommand eCmd, void * pArg ) const
{
	AddFilter ( eCmd, pArg, []( CSphFilterSettings & tFilter, const CSphMatch & tMatch, const ISphExpr * pExpr )
		{
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_iMaxValue = pExpr->Int64Eval(tMatch);
			tFilter.m_bHasEqualMax = false;
			tFilter.m_bOpenLeft = true;
		} );
}


void Expr_BinaryFilter_c::PopulateConstArgsGtInt ( ESphExprCommand eCmd, void * pArg ) const
{
	AddFilter ( eCmd, pArg, []( CSphFilterSettings & tFilter, const CSphMatch & tMatch, const ISphExpr * pExpr )
		{
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_iMinValue = pExpr->Int64Eval(tMatch);
			tFilter.m_bHasEqualMin = false;
			tFilter.m_bOpenRight = true;
		} );
}


void Expr_BinaryFilter_c::PopulateConstArgsLteInt ( ESphExprCommand eCmd, void * pArg ) const
{
	AddFilter ( eCmd, pArg, []( CSphFilterSettings & tFilter, const CSphMatch & tMatch, const ISphExpr * pExpr )
		{
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_iMaxValue = pExpr->Int64Eval(tMatch);
			tFilter.m_bOpenLeft = true;
		} );
}


void Expr_BinaryFilter_c::PopulateConstArgsGteInt ( ESphExprCommand eCmd, void * pArg ) const
{
	AddFilter ( eCmd, pArg, []( CSphFilterSettings & tFilter, const CSphMatch & tMatch, const ISphExpr * pExpr )
		{
			tFilter.m_eType = SPH_FILTER_RANGE;
			tFilter.m_iMinValue = pExpr->Int64Eval(tMatch);
			tFilter.m_bOpenRight = true;
		} );
}


void Expr_BinaryFilter_c::PopulateConstArgsEqInt ( ESphExprCommand eCmd, void * pArg ) const
{
	AddFilter ( eCmd, pArg, []( CSphFilterSettings & tFilter, const CSphMatch & tMatch, const ISphExpr * pExpr )
		{
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_dValues.Add ( pExpr->Int64Eval(tMatch) );
		} );
}


void Expr_BinaryFilter_c::PopulateConstArgsNeInt ( ESphExprCommand eCmd, void * pArg ) const
{
	AddFilter ( eCmd, pArg, []( CSphFilterSettings & tFilter, const CSphMatch & tMatch, const ISphExpr * pExpr )
		{
			tFilter.m_eType = SPH_FILTER_VALUES;
			tFilter.m_bExclude = true;
			tFilter.m_dValues.Add ( pExpr->Int64Eval(tMatch) );
		} );
}


const ISphExpr * Expr_BinaryFilter_c::GetExprForConsts ( ESphExprCommand eCmd ) const
{
	if ( eCmd!=SPH_EXPR_COLLECT_CONST_ARGS )
		return nullptr;

	if ( m_pFirst->IsConst() && !m_pSecond->IsConst() )
		return m_pFirst;

	if ( !m_pFirst->IsConst() && m_pSecond->IsConst() )
		return m_pSecond;

	return nullptr;
}


void Expr_BinaryFilter_c::SetFlagAnd ( ESphExprCommand eCmd, void * pArg ) const
{
	if ( eCmd==SPH_EXPR_COLLECT_CONST_ARGS )
		((ExprConstArgs_c*)pArg)->m_bAnd = true;
}


void Expr_BinaryFilter_c::SetFlagOr ( ESphExprCommand eCmd, void * pArg ) const
{
	if ( eCmd==SPH_EXPR_COLLECT_CONST_ARGS )
		((ExprConstArgs_c*)pArg)->m_bOr = true;
}

////////////////////////////////////////////////////////////////////

bool CanAliasedExprSetupAsFilter ( const CSphFilterSettings & tFilter, bool & bExclude )
{
	switch ( tFilter.m_eType )
	{
	case SPH_FILTER_VALUES:
		// fixme! this means "return nothing"
		if ( tFilter.m_dValues.GetLength()!=1 )
			return false;

		if ( tFilter.m_dValues[0]!=0 && tFilter.m_dValues[0]!=1 )
			return false;

		bExclude = ( tFilter.m_dValues[0]==0 ) ^ tFilter.m_bExclude;
		return true;

	case SPH_FILTER_RANGE:
	{
		bool bFilterIncludes0 = EvalRange ( 0, tFilter );
		bool bFilterIncludes1 = EvalRange ( 1, tFilter );

		// fixme! this means "return all"
		if ( bFilterIncludes0 && bFilterIncludes1 )
			return false;

		// fixme! this means "return nothing"
		if ( !bFilterIncludes0 && !bFilterIncludes1 )
			return false;

		bExclude = !bFilterIncludes1;
	}
	return true;

	default:
		return false;
	}
}

////////////////////////////////////////////////////////////////////
class Expr_ForIn_c : public Expr_JsonFieldConv_c
{
public:
	Expr_ForIn_c ( ISphExpr * pArg, bool bStrict, bool bIndex )
		: Expr_JsonFieldConv_c ( pArg )
		, m_bStrict ( bStrict )
		, m_bIndex ( bIndex )
	{}

	SphAttr_t * GetRef ()
	{
		return (SphAttr_t*)&m_uData;
	}

	void SetExpr ( ISphExpr * pExpr )
	{
		if ( pExpr==m_pExpr )
			return;

		SafeAddRef ( pExpr );
		m_pExpr = pExpr;
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final
	{
		Expr_JsonFieldConv_c::FixupLocator ( pOldSchema, pNewSchema );
		if ( m_pExpr )
			m_pExpr->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_JsonFieldConv_c::Command ( eCmd, pArg );
		if ( m_pExpr )
			m_pExpr->Command ( eCmd, pArg );
	}

	bool ExprEval ( int * pResult, const CSphMatch & tMatch, int iIndex, ESphJsonType eType, const BYTE * pVal ) const
	{
		m_uData = sphJsonPackTypeOffset ( eType, pVal-m_pBlobPool );
		bool bMatch = m_pExpr->Eval ( tMatch )!=0;
		*pResult = bMatch ? ( m_bIndex ? iIndex : 1 ) : ( m_bIndex ? -1 : 0 );
		return m_bStrict==bMatch;
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		int iResult = m_bIndex ? -1 : 0;

		if ( !m_pExpr )
			return iResult;

		const BYTE * p = nullptr;
		ESphJsonType eJson = GetKey ( &p, tMatch );

		switch ( eJson )
		{
		case JSON_INT32_VECTOR:
		case JSON_INT64_VECTOR:
		case JSON_DOUBLE_VECTOR:
			{
				int iSize = eJson==JSON_INT32_VECTOR ? 4 : 8;
				ESphJsonType eType = eJson==JSON_INT32_VECTOR ? JSON_INT32
					: eJson==JSON_INT64_VECTOR ? JSON_INT64
					: JSON_DOUBLE;
				int iLen = sphJsonUnpackInt ( &p );
				for ( int i=0; i<iLen; i++, p+=iSize )
					if ( !ExprEval ( &iResult, tMatch, i, eType, p ) )
						break;
				break;
			}
		case JSON_STRING_VECTOR:
			{
				sphJsonUnpackInt ( &p );
				int iLen = sphJsonUnpackInt ( &p );
				for ( int i=0;i<iLen;i++ )
				{
					if ( !ExprEval ( &iResult, tMatch, i, JSON_STRING, p ) )
						break;
					sphJsonSkipNode ( JSON_STRING, &p );
				}
				break;
			}
		case JSON_MIXED_VECTOR:
			{
				sphJsonUnpackInt ( &p );
				int iLen = sphJsonUnpackInt ( &p );
				for ( int i=0; i<iLen; ++i )
				{
					auto eType = (ESphJsonType)*p++;
					if ( !ExprEval ( &iResult, tMatch, i, eType, p ) )
						break;
					sphJsonSkipNode ( eType, &p );
				}
				break;
			}
		default:
			break;
		}

		return iResult;
	}

	float Eval ( const CSphMatch & tMatch ) const final { return (float)IntEval ( tMatch ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t)IntEval ( tMatch ); }

	bool SetupAsFilter ( CSphFilterSettings & tFilter, const ISphSchema & tSchema, const SIContainer_c & tSI ) const override
	{
		bool bExclude = false;
		if ( !CanAliasedExprSetupAsFilter ( tFilter, bExclude ) )
			return false;

		if ( m_bStrict )
			return false;

		ExprConstArgs_c tConstArgs;
		m_pExpr->Command ( SPH_EXPR_COLLECT_CONST_ARGS, (void*)&tConstArgs );
		if ( tConstArgs.IsError() )
			return false;

		if ( tConstArgs.m_bAnd && tConstArgs.m_bOr )
			return false;

		std::pair<const ISphSchema*,CSphString> tSchemaWithName;
		tSchemaWithName.first = &tSchema;
		m_pArg->Command ( SPH_EXPR_FORMAT_AS_TEXT, (void*)&tSchemaWithName );
		if ( tSchemaWithName.second.IsEmpty() )
			return false;

		if ( !tSI.IsEnabled ( tSchemaWithName.second ) )
			return false;

		bool bOk = false;
		switch ( tConstArgs.m_eType )
		{
		case SPH_FILTER_VALUES:
		case SPH_FILTER_STRING_LIST:
			bOk = !tConstArgs.m_bAnd;
			break;

		case SPH_FILTER_RANGE:
		case SPH_FILTER_FLOATRANGE:
			bOk = !tConstArgs.m_bOr;
			break;

		default:
			break;
		}

		if ( !bOk )
			return false;

		tFilter = tConstArgs;
		tFilter.m_sAttrName = tSchemaWithName.second;
		tFilter.m_eMvaFunc = SPH_MVAFUNC_ANY;
		tFilter.m_bExclude = bExclude;

		if ( tFilter.m_eType==SPH_FILTER_VALUES )
			tFilter.m_dValues.Uniq();

		return true;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_ForIn_c");
		CALC_POD_HASH(m_bStrict);
		CALC_POD_HASH(m_bIndex);
		CALC_CHILD_HASH(m_pExpr);
		return CALC_PARENT_HASH();
	}

	ISphExpr * Clone() const final { return new Expr_ForIn_c ( *this ); }

private:
	CSphRefcountedPtr<ISphExpr> m_pExpr;
	bool				m_bStrict {false};
	bool				m_bIndex {false};
	mutable uint64_t	m_uData {0};

	Expr_ForIn_c ( const Expr_ForIn_c& rhs )
		: Expr_JsonFieldConv_c ( rhs )
		, m_pExpr ( SafeClone ( rhs.m_pExpr ) )
		, m_bStrict ( rhs.m_bStrict )
		, m_bIndex ( rhs.m_bIndex )
	{
		if ( m_pExpr )
			m_pExpr->Command ( SPH_EXPR_SET_ITERATOR, &m_uData );
	}
};


class Expr_StrCmp_c : public Expr_BinaryFilter_c
{
public:
	Expr_StrCmp_c ( ISphExpr * pLeft, ISphExpr * pRight, ESphCollation eCollation, bool bExclude, EStrCmpDir eStrCmpDir )
		: Expr_BinaryFilter_c ( "Expr_StrEq_c", pLeft, pRight )
		, m_bExclude ( bExclude )
		, m_eStrCmpDir ( eStrCmpDir )
	{
		m_fnStrCmp = GetStringCmpFunc ( eCollation );
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pLeft = nullptr;
		const BYTE * pRight = nullptr;
		int iLeft = m_pFirst->StringEval ( tMatch, &pLeft );
		int iRight = m_pSecond->StringEval ( tMatch, &pRight );

		int iCmp = m_fnStrCmp ( {pLeft, iLeft}, {pRight, iRight}, false );

		FreeDataPtr ( *m_pFirst, pLeft );
		FreeDataPtr ( *m_pSecond, pRight );

		switch ( m_eStrCmpDir )
		{
			case EStrCmpDir::LT: return (iCmp<0) ^ m_bExclude;
			case EStrCmpDir::GT: return (iCmp>0) ^ m_bExclude;
			case EStrCmpDir::EQ: return (!iCmp) ^ m_bExclude;
			default: return 0;
		}
	}

	float Eval ( const CSphMatch & tMatch ) const final { return (float)IntEval ( tMatch ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t)IntEval ( tMatch ); }

	void Command ( ESphExprCommand eCmd, void * pArg )
	{
		Expr_Binary_c::Command ( eCmd, pArg );
		PopulateConstArgsStr ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_StrCmp_c");
		CALC_POD_HASH(m_fnStrCmp);
		CALC_POD_HASH(m_bExclude);
		CALC_POD_HASH(m_eStrCmpDir);
		CALC_CHILD_HASH(m_pFirst);
		CALC_CHILD_HASH(m_pSecond);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_StrCmp_c ( *this );
	}

private:
	SphStringCmp_fn m_fnStrCmp {nullptr};
	bool m_bExclude = false;
	EStrCmpDir m_eStrCmpDir;

	Expr_StrCmp_c ( const Expr_StrCmp_c & ) = default;
};


class Expr_JsonFieldIsNull_c : public Expr_JsonFieldConv_c
{
public:
	Expr_JsonFieldIsNull_c ( ISphExpr * pArg, bool bEquals )
		: Expr_JsonFieldConv_c ( pArg )
		, m_bEquals ( bEquals )
	{}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pVal = nullptr;
		ESphJsonType eJson = GetKey ( &pVal, tMatch );
		return m_bEquals ^ ( eJson!=JSON_EOF && eJson!=JSON_NULL );
	}

	float	Eval ( const CSphMatch & tMatch ) const final { return (float)IntEval ( tMatch ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t)IntEval ( tMatch ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_JsonFieldIsNull_c");
		CALC_POD_HASH(m_bEquals);
		return CALC_PARENT_HASH();
	}

	ISphExpr* Clone() const final
	{
		return new Expr_JsonFieldIsNull_c ( *this );
	}

private:
	bool m_bEquals;

	Expr_JsonFieldIsNull_c ( const Expr_JsonFieldIsNull_c& ) = default;
};

//////////////////////////////////////////////////////////////////////////

class Expr_MinTopWeight_c : public Expr_NoLocator_c
{
public:
	int IntEval ( const CSphMatch & ) const final			{ return m_pWeight ? *m_pWeight : -INT_MAX; }
	float Eval ( const CSphMatch & ) const final			{ return m_pWeight ? (float)*m_pWeight : -FLT_MAX; }
	int64_t Int64Eval ( const CSphMatch & ) const final	{ return m_pWeight ? *m_pWeight : -LLONG_MAX; }

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		CSphMatch * pWorst;
		if ( eCmd!=SPH_EXPR_SET_EXTRA_DATA )
			return;
		if ( static_cast<ISphExtra*>(pArg)->ExtraData ( EXTRA_GET_QUEUE_WORST, (void**)&pWorst ) )
			m_pWeight = &pWorst->m_iWeight;
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & bDisable ) final
	{
		bDisable = true;
		return 0;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_MinTopWeight_c;
	}

private:
	int * m_pWeight {nullptr};
};


class Expr_MinTopSortval_c : public Expr_NoLocator_c
{
public:
	float Eval ( const CSphMatch & ) const final
	{
		if ( m_pWorst && m_pWorst->m_pDynamic && m_iSortval>=0 )
			return *(float*)( m_pWorst->m_pDynamic + m_iSortval );
		return -FLT_MAX;
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		if ( eCmd!=SPH_EXPR_SET_EXTRA_DATA )
			return;
		auto * p = (ISphExtra*)pArg;
		if ( !p->ExtraData ( EXTRA_GET_QUEUE_WORST, (void**)&m_pWorst )
			|| !p->ExtraData ( EXTRA_GET_QUEUE_SORTVAL, (void**)&m_iSortval ) )
		{
			m_pWorst = nullptr;
		}
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & bDisable ) final
	{
		bDisable = true;
		return 0;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_MinTopSortval_c;
	}

private:
	CSphMatch *	m_pWorst {nullptr};
	int			m_iSortval {-1};
};


class Expr_Rand_c : public Expr_Unary_c
{
public:
	Expr_Rand_c ( ISphExpr * pFirst, bool bConst )
		: Expr_Unary_c ( "Expr_Rand_c", pFirst )
		, m_bConst ( bConst )
	{
		sphAutoSrand ();
		m_uState = ( (uint64_t)sphRand() << 32 ) + sphRand();
	}

	uint64_t XorShift64Star() const
	{
		m_uState ^= m_uState >> 12;
		m_uState ^= m_uState << 25;
		m_uState ^= m_uState >> 27;
		return m_uState * 2685821657736338717ULL;
	}

	float Eval ( const CSphMatch & tMatch ) const final
	{
		if ( m_pFirst )
		{
			uint64_t uSeed = (uint64_t)m_pFirst->Int64Eval ( tMatch );
			if ( !m_bConst )
				m_uState = uSeed;
			else if ( m_bFirstEval )
			{
				m_uState = uSeed;
				m_bFirstEval = false;
			}
		}
		return (float)( XorShift64Star() / (double)UINT64_MAX );
	}

	int IntEval ( const CSphMatch & tMatch ) const final { return (int)Eval ( tMatch ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t)Eval ( tMatch ); }

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & bDisable ) final
	{
		bDisable = true;
		return 0;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_Rand_c ( *this );
	}

private:
	bool				m_bConst {false};
	mutable bool		m_bFirstEval {true};
	mutable uint64_t	m_uState {0};

private:
	Expr_Rand_c ( const Expr_Rand_c& rhs )
		: Expr_Unary_c ( rhs )
		, m_bConst ( rhs.m_bConst )
	{}
};


#define EVALFIRST	m_pFirst->Eval(tMatch)
#define EVALSECOND	m_pSecond->Eval(tMatch)
#define EVALTHIRD	m_pThird->Eval(tMatch)

#define INTFIRST	m_pFirst->IntEval(tMatch)
#define INTSECOND	m_pSecond->IntEval(tMatch)
#define INTTHIRD	m_pThird->IntEval(tMatch)

#define INT64FIRST	m_pFirst->Int64Eval(tMatch)
#define INT64SECOND	m_pSecond->Int64Eval(tMatch)
#define INT64THIRD	m_pThird->Int64Eval(tMatch)

#define DECLARE_UNARY_TRAITS(_classname) \
	class _classname : public Expr_Unary_c \
	{ \
	public: \
		explicit _classname ( ISphExpr * pFirst ) : Expr_Unary_c ( #_classname, pFirst ) {} \
		_classname ( const _classname& rhs ) : Expr_Unary_c (rhs) {} \
		ISphExpr* Clone() const final { return new _classname(*this); }

#define DECLARE_END() };

#define DECLARE_UNARY_FLT(_classname,_expr) \
		DECLARE_UNARY_TRAITS ( _classname ) \
		float Eval ( const CSphMatch & tMatch ) const final { return _expr; } \
	};

#define DECLARE_UNARY_INT(_classname,_expr,_expr2,_expr3) \
		DECLARE_UNARY_TRAITS ( _classname ) \
		float Eval ( const CSphMatch & tMatch ) const final { return (float)_expr; } \
		int IntEval ( const CSphMatch & tMatch ) const final { return _expr2; } \
		int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return _expr3; } \
	};

#define IABS(_arg) ( (_arg)>0 ? (_arg) : (-_arg) )

DECLARE_UNARY_INT ( Expr_Neg_c,		-EVALFIRST,					-INTFIRST,			-INT64FIRST )
DECLARE_UNARY_INT ( Expr_Abs_c,		fabs(EVALFIRST),			IABS(INTFIRST),		IABS(INT64FIRST) )
DECLARE_UNARY_INT ( Expr_Ceil_c,	float(ceil(EVALFIRST)),		int(ceil(EVALFIRST)),	int64_t(ceil(EVALFIRST)) )
DECLARE_UNARY_INT ( Expr_Floor_c,	float(floor(EVALFIRST)),	int(floor(EVALFIRST)),	int64_t(floor(EVALFIRST)) )

DECLARE_UNARY_FLT ( Expr_Sin_c,		float(sin(EVALFIRST)) )
DECLARE_UNARY_FLT ( Expr_Cos_c,		float(cos(EVALFIRST)) )
DECLARE_UNARY_FLT ( Expr_Exp_c,		float(exp(EVALFIRST)) )

DECLARE_UNARY_INT ( Expr_NotInt_c,		(float)(INTFIRST?0:1),		INTFIRST?0:1,	INTFIRST?0:1 )
DECLARE_UNARY_INT ( Expr_NotInt64_c,	(float)(INT64FIRST?0:1),	INT64FIRST?0:1,	INT64FIRST?0:1 )
DECLARE_UNARY_INT ( Expr_Sint_c,		(float)(INTFIRST),			INTFIRST,		INTFIRST )

DECLARE_UNARY_TRAITS ( Expr_Ln_c )
	   float Eval ( const CSphMatch & tMatch ) const final
	   {
			   float fFirst = m_pFirst->Eval ( tMatch );
			   // ideally this would be SQLNULL instead of plain 0.0f
			   return fFirst>0.0f ? (float)log ( fFirst ) : 0.0f;
	   }
DECLARE_END()

DECLARE_UNARY_TRAITS ( Expr_Log2_c )
	   float Eval ( const CSphMatch & tMatch ) const final
	   {
			   float fFirst = m_pFirst->Eval ( tMatch );
			   // ideally this would be SQLNULL instead of plain 0.0f
			   return fFirst>0.0f ? (float)( log ( fFirst )*M_LOG2E ) : 0.0f;
	   }
DECLARE_END()

DECLARE_UNARY_TRAITS ( Expr_Log10_c )
	   float Eval ( const CSphMatch & tMatch ) const final
	   {
			   float fFirst = m_pFirst->Eval ( tMatch );
			   // ideally this would be SQLNULL instead of plain 0.0f
			   return fFirst>0.0f ? (float)( log ( fFirst )*M_LOG10E ) : 0.0f;
	   }
DECLARE_END()

DECLARE_UNARY_TRAITS ( Expr_Sqrt_c )
	   float Eval ( const CSphMatch & tMatch ) const final
	   {
			   float fFirst = m_pFirst->Eval ( tMatch );
			   // ideally this would be SQLNULL instead of plain 0.0f in case of negative argument
			   // MEGA optimization: do not call sqrt for 0.0f
			   return fFirst>0.0f ? (float)sqrt ( fFirst ) : 0.0f;
	   }
DECLARE_END()

//////////////////////////////////////////////////////////////////////////

#define DECLARE_BINARY_TRAITS(_classname,_parent) \
	class _classname : public _parent \
	{ \
		public: \
		_classname ( ISphExpr * pFirst, ISphExpr * pSecond ) : _parent ( #_classname, pFirst, pSecond ) {} \
		 _classname ( const _classname& rhs ) : _parent (rhs) {} \
		 ISphExpr* Clone() const final { return new _classname(*this); }


#define DECLARE_BINARY_FLT(_classname,_expr) \
		DECLARE_BINARY_TRAITS ( _classname, Expr_Binary_c ) \
		float Eval ( const CSphMatch & tMatch ) const final { return _expr; } \
	};

#define DECLARE_BINARY_INT(_classname,_expr,_expr2,_expr3) \
		DECLARE_BINARY_TRAITS ( _classname, Expr_Binary_c ) \
		float Eval ( const CSphMatch & tMatch ) const final { return _expr; } \
		int IntEval ( const CSphMatch & tMatch ) const final { return _expr2; } \
		int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return _expr3; } \
	};

#define DECLARE_BINARY_INT_EXPR(_classname,_expr,_expr2,_expr3,_expr4) \
		DECLARE_BINARY_TRAITS ( _classname, Expr_BinaryFilter_c ) \
		float Eval ( const CSphMatch & tMatch ) const final { return _expr; } \
		int IntEval ( const CSphMatch & tMatch ) const final { return _expr2; } \
		int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return _expr3; } \
		void Command ( ESphExprCommand eCmd, void * pArg ) final \
		{ \
			Expr_Binary_c::Command ( eCmd, pArg ); \
			_expr4 ( eCmd, pArg ); \
		} \
	};

#define DECLARE_BINARY_POLY(_classname,_expr,_expr2,_expr3,_expr4) \
	DECLARE_BINARY_INT_EXPR ( _classname##Float_c,	_expr,						(int)Eval(tMatch),		(int64_t)Eval(tMatch ),		_expr4 ) \
	DECLARE_BINARY_INT_EXPR ( _classname##Int_c,	(float)IntEval(tMatch),		_expr2,					(int64_t)IntEval(tMatch),	_expr4 ) \
	DECLARE_BINARY_INT_EXPR ( _classname##Int64_c,	(float)Int64Eval(tMatch),	(int)Int64Eval(tMatch),	_expr3,						_expr4 )

#define IFFLT(_expr)	( (_expr) ? 1.0f : 0.0f )
#define IFINT(_expr)	( (_expr) ? 1 : 0 )

DECLARE_BINARY_INT ( Expr_Add_c,	EVALFIRST + EVALSECOND,						(DWORD)INTFIRST + (DWORD)INTSECOND,				(uint64_t)INT64FIRST + (uint64_t)INT64SECOND )
DECLARE_BINARY_INT ( Expr_Sub_c,	EVALFIRST - EVALSECOND,						(DWORD)INTFIRST - (DWORD)INTSECOND,				(uint64_t)INT64FIRST - (uint64_t)INT64SECOND )
DECLARE_BINARY_INT ( Expr_Mul_c,	EVALFIRST * EVALSECOND,						(DWORD)INTFIRST * (DWORD)INTSECOND,				(uint64_t)INT64FIRST * (uint64_t)INT64SECOND )
DECLARE_BINARY_INT ( Expr_BitAnd_c,	(float)(int(EVALFIRST)&int(EVALSECOND)),	INTFIRST & INTSECOND,				INT64FIRST & INT64SECOND )
DECLARE_BINARY_INT ( Expr_BitOr_c,	(float)(int(EVALFIRST)|int(EVALSECOND)),	INTFIRST | INTSECOND,				INT64FIRST | INT64SECOND )
DECLARE_BINARY_INT ( Expr_Mod_c,	(float)(int(EVALFIRST)%int(EVALSECOND)),	INTFIRST % INTSECOND,				INT64FIRST % INT64SECOND )

DECLARE_BINARY_TRAITS ( Expr_Div_c, Expr_Binary_c )
	   float Eval ( const CSphMatch & tMatch ) const final
	   {
			   float fSecond = m_pSecond->Eval ( tMatch );
			   // ideally this would be SQLNULL instead of plain 0.0f
			   return fSecond!=0.0f ? m_pFirst->Eval ( tMatch )/fSecond : 0.0f;
	   }
DECLARE_END()

DECLARE_BINARY_TRAITS ( Expr_Idiv_c, Expr_Binary_c )
	float Eval ( const CSphMatch & tMatch ) const final
	{
		auto iSecond = int(EVALSECOND);
		// ideally this would be SQLNULL instead of plain 0.0f
		return iSecond ? float(int(EVALFIRST)/iSecond) : 0.0f;
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		int iSecond = INTSECOND;
		// ideally this would be SQLNULL instead of plain 0
		return iSecond ? ( INTFIRST / iSecond ) : 0;
	}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const final
	{
		int64_t iSecond = INT64SECOND;
		// ideally this would be SQLNULL instead of plain 0
		return iSecond ? ( INT64FIRST / iSecond ) : 0;
	}
DECLARE_END()

DECLARE_BINARY_POLY ( Expr_Lt,		IFFLT ( EVALFIRST<EVALSECOND ),					IFINT ( INTFIRST<INTSECOND ),		IFINT ( INT64FIRST<INT64SECOND  ), PopulateConstArgsLtInt )
DECLARE_BINARY_POLY ( Expr_Gt,		IFFLT ( EVALFIRST>EVALSECOND ),					IFINT ( INTFIRST>INTSECOND ),		IFINT ( INT64FIRST>INT64SECOND  ), PopulateConstArgsGtInt )
DECLARE_BINARY_POLY ( Expr_Lte,		IFFLT ( EVALFIRST<=EVALSECOND ),				IFINT ( INTFIRST<=INTSECOND ),		IFINT ( INT64FIRST<=INT64SECOND ), PopulateConstArgsLteInt )
DECLARE_BINARY_POLY ( Expr_Gte,		IFFLT ( EVALFIRST>=EVALSECOND ),				IFINT ( INTFIRST>=INTSECOND ),		IFINT ( INT64FIRST>=INT64SECOND ), PopulateConstArgsGteInt )
DECLARE_BINARY_POLY ( Expr_Eq,		IFFLT ( fabs ( EVALFIRST-EVALSECOND )<=1e-6 ),	IFINT ( INTFIRST==INTSECOND ),		IFINT ( INT64FIRST==INT64SECOND ), PopulateConstArgsEqInt )
DECLARE_BINARY_POLY ( Expr_Ne,		IFFLT ( fabs ( EVALFIRST-EVALSECOND )>1e-6 ),	IFINT ( INTFIRST!=INTSECOND ),		IFINT ( INT64FIRST!=INT64SECOND ), PopulateConstArgsNeInt )

DECLARE_BINARY_INT ( Expr_Min_c,	Min ( EVALFIRST, EVALSECOND ),					Min ( INTFIRST, INTSECOND ),		Min ( INT64FIRST, INT64SECOND ) )
DECLARE_BINARY_INT ( Expr_Max_c,	Max ( EVALFIRST, EVALSECOND ),					Max ( INTFIRST, INTSECOND ),		Max ( INT64FIRST, INT64SECOND ) )
DECLARE_BINARY_FLT ( Expr_Pow_c,	float ( pow ( EVALFIRST, EVALSECOND ) ) )

DECLARE_BINARY_POLY ( Expr_And,		EVALFIRST!=0.0f && EVALSECOND!=0.0f,		IFINT ( INTFIRST && INTSECOND ),	IFINT ( INT64FIRST && INT64SECOND ), SetFlagAnd )
DECLARE_BINARY_POLY ( Expr_Or,		EVALFIRST!=0.0f || EVALSECOND!=0.0f,		IFINT ( INTFIRST || INTSECOND ),	IFINT ( INT64FIRST || INT64SECOND ), SetFlagOr )

DECLARE_BINARY_FLT ( Expr_Atan2_c,	float ( atan2 ( EVALFIRST, EVALSECOND ) ) )

//////////////////////////////////////////////////////////////////////////

/// boring base stuff
class ExprThreeway_c : public ISphExpr
{
public:
	ExprThreeway_c ( const char * szClassName, ISphExpr * pFirst, ISphExpr * pSecond, ISphExpr * pThird )
		: m_pFirst ( pFirst )
		, m_pSecond ( pSecond )
		, m_pThird ( pThird )
		, m_szExprName ( szClassName )
	{
		SafeAddRef ( pFirst );
		SafeAddRef ( pSecond );
		SafeAddRef ( pThird );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		m_pFirst->FixupLocator ( pOldSchema, pNewSchema );
		m_pSecond->FixupLocator ( pOldSchema, pNewSchema );
		m_pThird->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		m_pFirst->Command ( eCmd, pArg );
		m_pSecond->Command ( eCmd, pArg );
		m_pThird->Command ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) override
	{
		EXPR_CLASS_NAME_NOCHECK( m_szExprName );
		CALC_CHILD_HASH(m_pFirst);
		CALC_CHILD_HASH(m_pSecond);
		CALC_CHILD_HASH(m_pThird);
		return CALC_DEP_HASHES();
	}


protected:
	CSphRefcountedPtr<ISphExpr>	m_pFirst;
	CSphRefcountedPtr<ISphExpr>	m_pSecond;
	CSphRefcountedPtr<ISphExpr>	m_pThird;
	const char*					m_szExprName;

protected:
	ExprThreeway_c ( const ExprThreeway_c & rhs )
		: m_pFirst ( SafeClone (rhs.m_pFirst) )
		, m_pSecond ( SafeClone (rhs.m_pSecond) )
		, m_pThird ( SafeClone (rhs.m_pThird) )
		, m_szExprName ( rhs.m_szExprName )
	{}
};

#define DECLARE_TERNARY(_classname,_expr,_expr2,_expr3) \
	class _classname : public ExprThreeway_c \
	{ \
	public: \
		_classname ( ISphExpr * pFirst, ISphExpr * pSecond, ISphExpr * pThird ) \
			: ExprThreeway_c ( #_classname, pFirst, pSecond, pThird ) {} \
		\
		float Eval ( const CSphMatch & tMatch ) const final { return _expr; } \
		int IntEval ( const CSphMatch & tMatch ) const final { return _expr2; } \
		int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return _expr3; } \
		_classname ( const _classname& rhs ) : ExprThreeway_c (rhs) {} \
		ISphExpr* Clone() const final { return new _classname(*this); } \
	};

DECLARE_TERNARY ( Expr_If_c,	( EVALFIRST!=0.0f ) ? EVALSECOND : EVALTHIRD,	INTFIRST ? INTSECOND : INTTHIRD,	INT64FIRST ? INT64SECOND : INT64THIRD )
#if defined (__aarch64__)
DECLARE_TERNARY ( Expr_Madd_c,	[this](const CSphMatch& tMatch){return EVALFIRST*EVALSECOND;}(tMatch)+EVALTHIRD,					INTFIRST*INTSECOND + INTTHIRD,		INT64FIRST*INT64SECOND + INT64THIRD )
#else
DECLARE_TERNARY ( Expr_Madd_c, EVALFIRST* EVALSECOND + EVALTHIRD, INTFIRST* INTSECOND + INTTHIRD, INT64FIRST* INT64SECOND + INT64THIRD )
#endif
DECLARE_TERNARY ( Expr_Mul3_c,	EVALFIRST*EVALSECOND*EVALTHIRD,					INTFIRST*INTSECOND*INTTHIRD,		INT64FIRST*INT64SECOND*INT64THIRD )

//////////////////////////////////////////////////////////////////////////
// UDF CALL SITE
//////////////////////////////////////////////////////////////////////////

static void * UdfMalloc ( int iLen )
{
	return new BYTE [ iLen ];
}

/// UDF call site
struct UdfCall_t
{
	PluginUDFRefPtr_c	m_pUdf;
	SPH_UDF_INIT		m_tInit;
	SPH_UDF_ARGS		m_tArgs;
	CSphVector<int>		m_dArgs2Free; // these args should be freed explicitly

	UdfCall_t()
	{
		m_tInit.func_data = nullptr;
		m_tInit.is_const = false;
		m_tArgs.arg_count = 0;
		m_tArgs.arg_types = nullptr;
		m_tArgs.arg_values = nullptr;
		m_tArgs.arg_names = nullptr;
		m_tArgs.str_lengths = nullptr;
		m_tArgs.fn_malloc = UdfMalloc;
	}

	~UdfCall_t ()
	{
		SafeDeleteArray ( m_tArgs.arg_types );
		SafeDeleteArray ( m_tArgs.arg_values );
		SafeDeleteArray ( m_tArgs.arg_names );
		SafeDeleteArray ( m_tArgs.str_lengths );
	}
};

//////////////////////////////////////////////////////////////////////////
// PARSER INTERNALS
//////////////////////////////////////////////////////////////////////////
class ExprParser_t;

#include "bissphinxexpr.h"

/// known operations, columns and functions
enum Tokh_e : BYTE
{
	// functions came first
	FUNC_NOW = 0,

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
	FUNC_KNN_DIST,

	FUNC_DAY,
	FUNC_WEEK,
	FUNC_MONTH,
	FUNC_YEAR,
	FUNC_YEARMONTH,
	FUNC_YEARMONTHDAY,
	FUNC_YEARWEEK,
	FUNC_HOUR,
	FUNC_MINUTE,
	FUNC_SECOND,
	FUNC_DAYOFWEEK,
	FUNC_DAYOFYEAR,
	FUNC_QUARTER,

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
	FUNC_REMAP,

	FUNC_GEODIST,
	FUNC_EXIST,
	FUNC_POLY2D,
	FUNC_GEOPOLY2D,
	FUNC_CONTAINS,
	FUNC_ZONESPANLIST,
	FUNC_CONCAT,
	FUNC_TO_STRING,
	FUNC_RANKFACTORS,
	FUNC_FACTORS,
	FUNC_BM25F,
	FUNC_INTEGER,
	FUNC_DOUBLE,
	FUNC_LENGTH,
	FUNC_LEAST,
	FUNC_GREATEST,
	FUNC_UINT,
	FUNC_UINT64,
	FUNC_QUERY,

	FUNC_CURTIME,
	FUNC_CURDATE,
	FUNC_TIME,
	FUNC_DATE,
	FUNC_DAYNAME,
	FUNC_MONTHNAME,
	FUNC_UTC_TIME,
	FUNC_UTC_TIMESTAMP,
	FUNC_TIMEDIFF,
	FUNC_DATEDIFF,
	FUNC_DATEADD,
	FUNC_DATESUB,
	FUNC_CURRENT_USER,
	FUNC_CONNECTION_ID,
	FUNC_ALL,
	FUNC_ANY,
	FUNC_INDEXOF,

	FUNC_MIN_TOP_WEIGHT,
	FUNC_MIN_TOP_SORTVAL,

	FUNC_ATAN2,
	FUNC_RAND,

	FUNC_REGEX,

	FUNC_SUBSTRING_INDEX,
	FUNC_UPPER,
	FUNC_LOWER,

	FUNC_LAST_INSERT_ID,
	FUNC_LEVENSHTEIN,
	FUNC_DATE_FORMAT,
	FUNC_DATABASE,
	FUNC_USER,
	FUNC_VERSION,

	FUNC_RANGE,
	FUNC_HISTOGRAM,
	FUNC_DATE_RANGE,
	FUNC_DATE_HISTOGRAM,

	FUNC_UUID_SHORT,

	FUNC_FUNCS_COUNT, // insert any new functions ABOVE this one
	TOKH_TOKH_OFFSET = FUNC_FUNCS_COUNT,

	// general operations and operators
	TOKH_COUNT = TOKH_TOKH_OFFSET,  // check m.b. column with name 'count' exists, and override, if so
	TOKH_WEIGHT,
	TOKH_GROUPBY,
	TOKH_DISTINCT,
	TOKH_AND,
	TOKH_OR,
	TOKH_NOT,
	TOKH_DIV,
	TOKH_MOD,
	TOKH_FOR,
	TOKH_IS,
	TOKH_NULL,

	TOKH_TOKH_COUNT,
	TOKH_UNKNOWN = TOKH_TOKH_COUNT
};

// dHash2Op [i-FUNC_FUNCS_COUNT] returns 1:1 mapping from hash to the token
const static int g_dHash2Op[TOKH_TOKH_COUNT-TOKH_TOKH_OFFSET] = { TOK_COUNT, TOK_WEIGHT, TOK_GROUPBY, TOK_DISTINCT, TOK_AND, TOK_OR, TOK_NOT, TOK_DIV, TOK_MOD, TOK_FOR, TOK_IS, TOK_NULL, };

struct TokhKeyVal_t
{
	const char * m_sName = nullptr;
	Tokh_e m_eTok = TOKH_UNKNOWN;
	int m_iLen = 0;

	TokhKeyVal_t ( const char * sName, Tokh_e eTok )
		: m_sName ( sName )
		, m_eTok ( eTok )
	{
		m_iLen = (int)strlen ( m_sName );
	}
};

const static TokhKeyVal_t g_dKeyValTokens[] = // no order is necessary, but created hash may depend from it a bit
{
	// functions
	{ "now",			FUNC_NOW			},

	{ "abs",			FUNC_ABS			},
	{ "ceil",			FUNC_CEIL			},
	{ "floor",			FUNC_FLOOR			},
	{ "sin",			FUNC_SIN			},
	{ "cos",			FUNC_COS			},
	{ "ln",				FUNC_LN				},
	{ "log2",			FUNC_LOG2			},
	{ "log10",			FUNC_LOG10			},
	{ "exp",			FUNC_EXP			},
	{ "sqrt",			FUNC_SQRT			},
	{ "bigint",			FUNC_BIGINT			},	// type-enforcer special as-if-function
	{ "sint",			FUNC_SINT			},	// type-enforcer special as-if-function
	{ "crc32",			FUNC_CRC32			},
	{ "fibonacci",		FUNC_FIBONACCI		},
	{ "knn_dist",		FUNC_KNN_DIST		},

	{ "day",			FUNC_DAY			},
	{ "week",			FUNC_WEEK			},
	{ "month",			FUNC_MONTH			},
	{ "year",			FUNC_YEAR			},
	{ "yearmonth",		FUNC_YEARMONTH		},
	{ "yearmonthday",	FUNC_YEARMONTHDAY	},
	{ "yearweek",		FUNC_YEARWEEK		},
	{ "hour",			FUNC_HOUR			},
	{ "minute",			FUNC_MINUTE			},
	{ "second",			FUNC_SECOND			},
	{ "dayofweek",		FUNC_DAYOFWEEK		},
	{ "dayofyear",		FUNC_DAYOFYEAR		},
	{ "quarter",		FUNC_QUARTER		},

	{ "min",			FUNC_MIN			},
	{ "max",			FUNC_MAX			},
	{ "pow",			FUNC_POW			},
	{ "idiv",			FUNC_IDIV			},

	{ "if",				FUNC_IF				},
	{ "madd",			FUNC_MADD			},
	{ "mul3",			FUNC_MUL3			},

	{ "interval",		FUNC_INTERVAL		},
	{ "in",				FUNC_IN				},
	{ "bitdot",			FUNC_BITDOT			},
	{ "remap",			FUNC_REMAP			},

	{ "geodist",		FUNC_GEODIST		},
	{ "exist",			FUNC_EXIST			},
	{ "poly2d",			FUNC_POLY2D			},
	{ "geopoly2d",		FUNC_GEOPOLY2D		},
	{ "contains",		FUNC_CONTAINS		},
	{ "zonespanlist",	FUNC_ZONESPANLIST	},
	{ "concat",			FUNC_CONCAT			},
	{ "to_string",		FUNC_TO_STRING		},
	{ "rankfactors",	FUNC_RANKFACTORS	},
	{ "packedfactors",	FUNC_FACTORS		},
	{ "factors",		FUNC_FACTORS		}, // just an alias for PACKEDFACTORS()
	{ "bm25f",			FUNC_BM25F			},
	{ "integer",		FUNC_INTEGER		},
	{ "double",			FUNC_DOUBLE			},
	{ "length",			FUNC_LENGTH			},
	{ "least",			FUNC_LEAST			},
	{ "greatest",		FUNC_GREATEST		},
	{ "uint",			FUNC_UINT			},
	{ "uint64",			FUNC_UINT64			},
	{ "query",			FUNC_QUERY			},

	{ "curtime",		FUNC_CURTIME		},
	{ "curdate",		FUNC_CURDATE		},
	{ "time",			FUNC_TIME			},
	{ "date",			FUNC_DATE			},
	{ "dayname",		FUNC_DAYNAME		},
	{ "monthname",		FUNC_MONTHNAME		},
	{ "utc_time",		FUNC_UTC_TIME		},
	{ "utc_timestamp",	FUNC_UTC_TIMESTAMP	},
	{ "timediff",		FUNC_TIMEDIFF		},
	{ "datediff",		FUNC_DATEDIFF		},
	{ "date_add",		FUNC_DATEADD		},
	{ "date_sub",		FUNC_DATESUB		},
	{ "current_user",	FUNC_CURRENT_USER	},
	{ "connection_id",	FUNC_CONNECTION_ID	},
	{ "all",			FUNC_ALL			},
	{ "any",			FUNC_ANY			},
	{ "indexof",		FUNC_INDEXOF		},

	{ "min_top_weight",	FUNC_MIN_TOP_WEIGHT	 },
	{ "min_top_sortval",FUNC_MIN_TOP_SORTVAL },

	{ "atan2",			FUNC_ATAN2			 },
	{ "rand",			FUNC_RAND			 },

	{ "regex",			FUNC_REGEX			 },

	{ "substring_index",FUNC_SUBSTRING_INDEX },
	{ "upper",          FUNC_UPPER           },
	{ "lower",          FUNC_LOWER           },

	{ "last_insert_id",	FUNC_LAST_INSERT_ID	 },
	{ "levenshtein",	FUNC_LEVENSHTEIN	 },
	{ "date_format",	FUNC_DATE_FORMAT	 },
	{ "database",		FUNC_DATABASE		 },
	{ "user",			FUNC_USER			 },
	{ "version",		FUNC_VERSION		 },

	{ "range",			FUNC_RANGE			},
	{ "histogram",		FUNC_HISTOGRAM		},
	{ "date_range",		FUNC_DATE_RANGE		},
	{ "date_histogram",	FUNC_DATE_HISTOGRAM	},

	{ "uuid_short",		FUNC_UUID_SHORT	},

	// other reserved (operators, columns, etc.)
	{ "count",			TOKH_COUNT			},
	{ "weight",			TOKH_WEIGHT			},
	{ "groupby",		TOKH_GROUPBY		},
	{ "distinct",		TOKH_DISTINCT		},
	{ "and",			TOKH_AND			},
	{ "or",				TOKH_OR				},
	{ "not",			TOKH_NOT			},
	{ "div",			TOKH_DIV			},
	{ "mod",			TOKH_MOD			},
	{ "for",			TOKH_FOR			},
	{ "is",				TOKH_IS				},
	{ "null",			TOKH_NULL			},
};


// helper to generate input data for gperf
// change #if 0 to #if 1. Compile and run any code includes this file (for example, searchd or gmanticoretests)
// grap output and place to file '1.p'. Execute command printed at the end of 1.p.
// copy dAsso from asso_values in that C source
// modify iHash switch according to that C source, if needed
// compile and run the program and copy dIndexes from the output
// to ignore case: in asso table values 65..90 (A..Z) copy from 97..122 (a..z), and change strcmp to strcasecmp
#if 0
int HashGen()
{
	printf ( "struct func { char *name; int num; };\n%%%%\n" );
	for ( int i=0; i<int( sizeof ( g_dKeyValTokens )/sizeof ( g_dKeyValTokens[0] )); ++i )
	printf ( "%s, %d\n", g_dKeyValTokens[i].m_sName, i );
	printf ( "%%%%\n" );
	printf ( "void main()\n" );
	printf ( "{\n" );
	printf ( "\tint i;\n" );
	printf ( "\tfor ( i=0; i<=MAX_HASH_VALUE; ++i )\n" );
	printf ( "\t\tprintf ( \"%%d,%%s\", wordlist[i].name[0] ? wordlist[i].num : -1, (i%%10)==9 ? \"\\n\" : \" \" );\n" );
	printf ( "}\n" );
	printf ( "// gperf -Gt -tm5000 1.p > 1.c\n" );
	exit(0);
//	sphDie ( "INTERNAL: HashGen() finished. Grab result, then change #if 1 to #if 0 few lines above %s:%d", __FILE__, __LINE__ );
}

static int G_HASHGEN = HashGen();
#endif

static Tokh_e TokHashLookup ( Str_t sKey )
{
	assert ( sKey.first && sKey.second && sKey.first[0] );

	const static BYTE dAsso[] =
	{
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 10, 10,
		27, 49, 9, 6, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 29, 64, 14, 5, 5,
		31, 25, 64, 6, 167, 14, 41, 7, 7, 38,
		16, 16, 20, 13, 6, 36, 75, 58, 35, 32,
		15, 167, 167, 167, 167, 49, 167, 29, 64, 14,
		5, 5, 31, 25, 64, 6, 167, 14, 41, 7,
		7, 38, 16, 16, 20, 13, 6, 36, 75, 58,
		35, 32, 15, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167, 167, 167, 167, 167,
		167, 167, 167, 167, 167, 167
	};

	const static short dIndexes[] =
	{
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1, 37, -1, -1, -1, -1,
		104, 106, 102, -1, 29, 63, 34, 62, -1, 70,
		4, 93, -1, 87, 65, 41, 12, 94, 99, 33,
		9, 80, 100, 5, 52, 45, 73, 46, 44, 10,
		6, 61, 60, 88, 76, 69, 64, 68, 1, 57,
		101, 24, 91, 95, 58, 48, 36, -1, 96, -1,
		49, 50, 16, 56, 105, -1, 25, 39, 59, 85,
		30, 40, 83, 77, 15, 89, 72, 38, 71, 18,
		81, 8, 28, 43, 55, 17, 75, 79, 26, 92,
		42, 97, 47, 22, 27, 19, 2, 11, -1, 13,
		-1, -1, 66, -1, 74, -1, 53, -1, -1, 78,
		-1, -1, 90, -1, 7, 21, 0, -1, 67, 84,
		-1, -1, 3, 51, 107, 31, -1, -1, 98, 86,
		82, -1, -1, 54, 23, -1, -1, -1, 14, -1,
		35, -1, -1, -1, 20, -1, -1, -1, 103, -1,
		-1, -1, -1, -1, -1, -1, 32
	};

	auto * s = (const BYTE*) sKey.first;
	auto iLen = sKey.second;
	auto iHash = iLen;

	switch ( iHash )
	{
	default:
		iHash += dAsso[(unsigned char)s[3]];
		/*FALLTHROUGH*/
	case 3:
		iHash += dAsso[(unsigned char)s[2]];
		/*FALLTHROUGH*/
	case 2:
	case 1:
		iHash += dAsso[(unsigned char)s[0]];
		break;
	}
	iHash += dAsso[(unsigned char)s[iLen-1]];

	if ( iHash>=(int)(sizeof(dIndexes)/sizeof(dIndexes[0])) )
		return TOKH_UNKNOWN;

	auto iFunc = dIndexes[iHash];
	if ( iFunc>=0 && strncasecmp ( g_dKeyValTokens[iFunc].m_sName, sKey.first, iLen )==0 && g_dKeyValTokens[iFunc].m_iLen==iLen )
		return g_dKeyValTokens[iFunc].m_eTok;
	return TOKH_UNKNOWN;
}


static int TokHashCheck()
{
	// //in asso table values 65..90 (A..Z) copy from 97..122 (a..z), and change strcmp to strcasecmp
	// //values 66..91 (A..Z) copy from 98..123 (a..z)
	//int iFrom = 65;
	//int iTo = 90;
	//int iBase = 97;
	//for ( int i=0; i<=iTo-iFrom; i++ )
	//	g_dAsso[iFrom+i] = g_dAsso[iBase+i];

	//for ( int i=0; i<sizeof(g_dAsso); i++ )
	//{
	//	if ( (i%10)==0 )
	//		printf ( "\n");
	//	printf ( "%d, ", g_dAsso[i] );
	//}

	for ( const auto & kv : g_dKeyValTokens )
	{
		CSphString sKey ( kv.m_sName );
		sKey.ToLower ();
		Str_t sKeyStr { sKey.cstr (), sKey.Length () };
		auto uHash = TokHashLookup ( sKeyStr );
		if ( uHash!=kv.m_eTok )
			sphDie ( "INTERNAL ERROR: lookup for %s failed, got %d, expected %d, rebuild token hash", sKey.cstr(), uHash, kv.m_eTok );
		sKey.ToUpper ();
		uHash = TokHashLookup ( sKeyStr );
		if ( uHash!=kv.m_eTok )
			sphDie ( "INTERNAL ERROR: lookup for %s failed, got %d, expected %d, rebuild token hash", sKey.cstr(), uHash, kv.m_eTok );
	}
	if ( TokHashLookup ( { "A", 1 } )!=TOKH_UNKNOWN )
		sphDie ( "INTERNAL ERROR: lookup for A() succeeded, rebuild token hash" );
	return 1;
}

static int VARIABLE_IS_NOT_USED G_FUNC_HASH_CHECK = TokHashCheck();

// additional functions traits

struct FuncDesc_t
{
	//const char *	m_sName;
	int				m_iArgs;	// positive assume exact N, negative assume 'at least'
	int				m_iNodeType; // usually TOK_FUNC, but sometimes not
	//	Tokh_e			m_eFunc;
	ESphAttr		m_eRet;
};

static FuncDesc_t g_dFuncs[FUNC_FUNCS_COUNT] = // Keep same order as in Tokh_e
{
	{ /*"now",			*/		0,	TOK_FUNC,		/*FUNC_NOW,				*/	SPH_ATTR_INTEGER },

	{ /*"abs",			*/		1,	TOK_FUNC,		/*FUNC_ABS,				*/	SPH_ATTR_NONE },
	{ /*"ceil",			*/		1,	TOK_FUNC,		/*FUNC_CEIL,			*/	SPH_ATTR_BIGINT },
	{ /*"floor",		*/		1,	TOK_FUNC,		/*FUNC_FLOOR,			*/	SPH_ATTR_BIGINT },
	{ /*"sin",			*/		1,	TOK_FUNC,		/*FUNC_SIN,				*/	SPH_ATTR_FLOAT },
	{ /*"cos",			*/		1,	TOK_FUNC,		/*FUNC_COS,				*/	SPH_ATTR_FLOAT },
	{ /*"ln",			*/		1,	TOK_FUNC,		/*FUNC_LN,				*/	SPH_ATTR_FLOAT },
	{ /*"log2",			*/		1,	TOK_FUNC,		/*FUNC_LOG2,			*/	SPH_ATTR_FLOAT },
	{ /*"log10",		*/		1,	TOK_FUNC,		/*FUNC_LOG10,			*/	SPH_ATTR_FLOAT },
	{ /*"exp",			*/		1,	TOK_FUNC,		/*FUNC_EXP,				*/	SPH_ATTR_FLOAT },
	{ /*"sqrt",			*/		1,	TOK_FUNC,		/*FUNC_SQRT,			*/	SPH_ATTR_FLOAT },
	{ /*"bigint",		*/		1,	TOK_FUNC,		/*FUNC_BIGINT,			*/	SPH_ATTR_BIGINT },	// type-enforcer special as-if-function
	{ /*"sint",			*/		1,	TOK_FUNC,		/*FUNC_SINT,			*/	SPH_ATTR_BIGINT },	// type-enforcer special as-if-function
	{ /*"crc32",		*/		1,	TOK_FUNC,		/*FUNC_CRC32,			*/	SPH_ATTR_INTEGER },
	{ /*"fibonacci",	*/		1,	TOK_FUNC,		/*FUNC_FIBONACCI,		*/	SPH_ATTR_INTEGER },
	{ /*"knn_dist"",	*/		0,	TOK_FUNC,		/*FUNC_KNN_DIST,		*/	SPH_ATTR_FLOAT },

	{ /*"day",			*/		1,	TOK_FUNC_DAY,	/*FUNC_DAY,				*/	SPH_ATTR_INTEGER },
	{ /*"week",			*/		-1,	TOK_FUNC_WEEK,	/*FUNC_WEEK,			*/	SPH_ATTR_INTEGER },
	{ /*"month",		*/		1,	TOK_FUNC_MONTH,	/*FUNC_MONTH,			*/	SPH_ATTR_INTEGER },
	{ /*"year",			*/		1,	TOK_FUNC_YEAR,	/*FUNC_YEAR,			*/	SPH_ATTR_INTEGER },
	{ /*"yearmonth",	*/		1,	TOK_FUNC,		/*FUNC_YEARMONTH,		*/	SPH_ATTR_INTEGER },
	{ /*"yearmonthday",	*/		1,	TOK_FUNC,		/*FUNC_YEARMONTHDAY,	*/	SPH_ATTR_INTEGER },
	{ /*"yearweek",		*/		1,	TOK_FUNC,		/*FUNC_YEARWEEK,		*/	SPH_ATTR_INTEGER },
	{ /*"hour",			*/		1,	TOK_FUNC_HOUR,	/*FUNC_HOUR,			*/	SPH_ATTR_INTEGER },
	{ /*"minute",		*/		1,	TOK_FUNC_MINUTE,/*FUNC_MINUTE,			*/	SPH_ATTR_INTEGER },
	{ /*"second",		*/		1,	TOK_FUNC_SECOND,/*FUNC_SECOND,			*/	SPH_ATTR_INTEGER },
	{ /*"dayofweek",	*/		1,	TOK_FUNC,		/*FUNC_DAYOFWEEK,		*/	SPH_ATTR_INTEGER },
	{ /*"dayofyear",	*/		1,	TOK_FUNC,		/*FUNC_DAYOFYEAR,		*/	SPH_ATTR_INTEGER },
	{ /*"quarter",		*/		1,	TOK_FUNC_QUARTER,/*FUNC_QUARTER,		*/	SPH_ATTR_INTEGER },

	{ /*"min",			*/		2,	TOK_FUNC,		/*FUNC_MIN,				*/	SPH_ATTR_NONE },
	{ /*"max",			*/		2,	TOK_FUNC,		/*FUNC_MAX,				*/	SPH_ATTR_NONE },
	{ /*"pow",			*/		2,	TOK_FUNC,		/*FUNC_POW,				*/	SPH_ATTR_FLOAT },
	{ /*"idiv",			*/		2,	TOK_FUNC,		/*FUNC_IDIV,			*/	SPH_ATTR_NONE },

	{ /*"if",			*/		3,	TOK_FUNC,		/*FUNC_IF,				*/	SPH_ATTR_NONE },
	{ /*"madd",			*/		3,	TOK_FUNC,		/*FUNC_MADD,			*/	SPH_ATTR_NONE },
	{ /*"mul3",			*/		3,	TOK_FUNC,		/*FUNC_MUL3,			*/	SPH_ATTR_NONE },

	{ /*"interval",		*/		-2,	TOK_FUNC_INTERVAL,/*FUNC_INTERVAL,		*/	SPH_ATTR_INTEGER },
	{ /*"in",			*/		-1,	TOK_FUNC_IN,	/*FUNC_IN,				*/	SPH_ATTR_INTEGER },
	{ /*"bitdot",		*/		-1, TOK_FUNC,		/*FUNC_BITDOT,			*/	SPH_ATTR_NONE },
	{ /*"remap",		*/		4,	TOK_FUNC_REMAP,	/*FUNC_REMAP,			*/	SPH_ATTR_INTEGER },

	{ /*"geodist",		*/		-4,	TOK_FUNC,		/*FUNC_GEODIST,			*/	SPH_ATTR_FLOAT },
	{ /*"exist",		*/		2,	TOK_FUNC,		/*FUNC_EXIST,			*/	SPH_ATTR_NONE },
	{ /*"poly2d",		*/		-1,	TOK_FUNC,		/*FUNC_POLY2D,			*/	SPH_ATTR_POLY2D },
	{ /*"geopoly2d",	*/		-1,	TOK_FUNC,		/*FUNC_GEOPOLY2D,		*/	SPH_ATTR_POLY2D },
	{ /*"contains",		*/		3,	TOK_FUNC,		/*FUNC_CONTAINS,		*/	SPH_ATTR_INTEGER },
	{ /*"zonespanlist",	*/		0,	TOK_FUNC,		/*FUNC_ZONESPANLIST,	*/	SPH_ATTR_STRINGPTR },
	{ /*"concat",		*/		-1,	TOK_FUNC,		/*FUNC_CONCAT,			*/	SPH_ATTR_STRINGPTR },
	{ /*"to_string",	*/		1,	TOK_FUNC,		/*FUNC_TO_STRING,		*/	SPH_ATTR_STRINGPTR },
	{ /*"rankfactors",	*/		0,	TOK_FUNC,		/*FUNC_RANKFACTORS,		*/	SPH_ATTR_STRINGPTR },
	{ /*"packedfactors",*/		0,	TOK_FUNC_PF,	/*FUNC_FACTORS, 		*/	SPH_ATTR_FACTORS }, // and also 'factors'
	{ /*"bm25f",		*/		-2,	TOK_FUNC,		/*FUNC_BM25F,			*/	SPH_ATTR_FLOAT },
	{ /*"integer",		*/		1,	TOK_FUNC,		/*FUNC_INTEGER,			*/	SPH_ATTR_BIGINT },
	{ /*"double",		*/		1,	TOK_FUNC,		/*FUNC_DOUBLE,			*/	SPH_ATTR_FLOAT },
	{ /*"length",		*/		1,	TOK_FUNC,		/*FUNC_LENGTH,			*/	SPH_ATTR_INTEGER },
	{ /*"least",		*/		1,	TOK_FUNC,		/*FUNC_LEAST,			*/	SPH_ATTR_STRINGPTR },
	{ /*"greatest",		*/		1,	TOK_FUNC,		/*FUNC_GREATEST,		*/	SPH_ATTR_STRINGPTR },
	{ /*"uint",			*/		1,	TOK_FUNC,		/*FUNC_UINT,			*/	SPH_ATTR_INTEGER },
	{ /*"uint64",		*/		1,	TOK_FUNC,		/*FUNC_UINT64,			*/	SPH_ATTR_UINT64 },
	{ /*"query",		*/		0,	TOK_FUNC,		/*FUNC_QUERY,			*/	SPH_ATTR_STRINGPTR },

	{ /*"curtime",		*/		0,	TOK_FUNC,		/*FUNC_CURTIME,			*/	SPH_ATTR_STRINGPTR }, // also evals numerics
	{ /*"curdate",		*/		0,	TOK_FUNC,		/*FUNC_CURDATE,			*/	SPH_ATTR_STRINGPTR },
	{ /*"time",			*/		1,	TOK_FUNC,		/*FUNC_TIME,			*/	SPH_ATTR_STRINGPTR },
	{ /*"date",			*/		1,	TOK_FUNC,		/*FUNC_DATE,			*/	SPH_ATTR_STRINGPTR },
	{ /*"dayname",		*/		1,	TOK_FUNC,		/*FUNC_DAYNAME,			*/	SPH_ATTR_STRINGPTR },
	{ /*"monthname",	*/		1,	TOK_FUNC,		/*FUNC_MONTHNAME,		*/	SPH_ATTR_STRINGPTR },
	{ /*"utc_time",		*/		0,	TOK_FUNC,		/*FUNC_UTC_TIME,		*/	SPH_ATTR_STRINGPTR }, // also evals numerics
	{ /*"utc_timestamp",*/		0,	TOK_FUNC,		/*FUNC_UTC_TIMESTAMP,	*/	SPH_ATTR_STRINGPTR }, // also evals numerics
	{ /*"timediff",		*/		2,	TOK_FUNC,		/*FUNC_TIMEDIFF,		*/	SPH_ATTR_STRINGPTR }, // also evals numerics
	{ /*"datediff",		*/		2,	TOK_FUNC,		/*FUNC_DATEDIFF,		*/	SPH_ATTR_BIGINT },
	{ /*"date_add",		*/		-1,	TOK_FUNC_DATE,	/*FUNC_DATEADD,			*/	SPH_ATTR_BIGINT },
	{ /*"date_sub",		*/		-1,	TOK_FUNC_DATE,	/*FUNC_DATESUB,			*/	SPH_ATTR_BIGINT },

	{ /*"current_user",	*/		0,	TOK_FUNC,		/*FUNC_CURRENT_USER,	*/	SPH_ATTR_STRINGPTR },
	{ /*"connection_id",*/		0,	TOK_FUNC,		/*FUNC_CONNECTION_ID,	*/	SPH_ATTR_INTEGER },
	{ /*"all",			*/		-1,	TOK_FUNC_JA,	/*FUNC_ALL,				*/	SPH_ATTR_INTEGER },
	{ /*"any",			*/		-1,	TOK_FUNC_JA,	/*FUNC_ANY,				*/	SPH_ATTR_INTEGER },
	{ /*"indexof",		*/		-1,	TOK_FUNC_JA,	/*FUNC_INDEXOF,			*/	SPH_ATTR_BIGINT },

	{ /*"min_top_weight",*/		0,	TOK_FUNC,		/*FUNC_MIN_TOP_WEIGHT,	*/	SPH_ATTR_INTEGER },
	{ /*"min_top_sortval",*/	0,	TOK_FUNC,		/*FUNC_MIN_TOP_SORTVAL,	*/	SPH_ATTR_FLOAT },

	{ /*"atan2",		*/		2,	TOK_FUNC,		/*FUNC_ATAN2,			*/	SPH_ATTR_FLOAT },
	{ /*"rand",		*/			-1,	TOK_FUNC_RAND,	/*FUNC_RAND,			*/	SPH_ATTR_FLOAT },

	{  /*"regex",		*/		2,	TOK_FUNC,		/*FUNC_REGEX,			*/	SPH_ATTR_INTEGER },

	{  /*"substring_index",*/	3,	TOK_FUNC,		/*FUNC_SUBSTRING_INDEX,	*/	SPH_ATTR_STRINGPTR }, // also evals numerics
	{  /*"upper",          */	1,	TOK_FUNC,		/*FUNC_UPPER,           */	SPH_ATTR_STRINGPTR }, // also evals numerics
	{  /*"lower",          */	1,	TOK_FUNC,		/*FUNC_LOWER,           */	SPH_ATTR_STRINGPTR }, // also evals numerics

	{  /*"last_insert_id",*/	0,	TOK_FUNC,		/*FUNC_LAST_INSERT_ID,	*/	SPH_ATTR_STRINGPTR },
	{ /*"levenshtein", */		-1,	TOK_FUNC,		/*FUNC_LEVENSHTEIN,		*/	SPH_ATTR_NONE },
	{ /*"date_format", */		2,	TOK_FUNC,		/*FUNC_DATE_FORMAT,		*/	SPH_ATTR_STRINGPTR },
	{ /*"database", */			0,	TOK_FUNC,		/*FUNC_DATABASE,		*/	SPH_ATTR_STRINGPTR },
	{ /*"user", */				0,	TOK_FUNC,		/*FUNC_USER,			*/	SPH_ATTR_STRINGPTR },
	{ /*"version", */			0,	TOK_FUNC,		/*FUNC_VERSION,			*/	SPH_ATTR_STRINGPTR },

	{ /*"range",	*/			-2,	TOK_FUNC,		/*FUNC_RANGE,			*/	SPH_ATTR_INTEGER },
	{ /*"histogram",*/			2,	TOK_FUNC,		/*FUNC_HISTOGRAM,		*/	SPH_ATTR_INTEGER },
	{ /*"date_range",	*/		-2,	TOK_FUNC,		/*FUNC_DATE_RANGE,		*/	SPH_ATTR_INTEGER },
	{ /*"date_histogram",*/		2,	TOK_FUNC,		/*FUNC_DATE_HISTOGRAM,	*/	SPH_ATTR_INTEGER },
	
	{ /*"uuid_short",*/			0,	TOK_FUNC,		/*FUNC_UUID_SHORT,		*/	SPH_ATTR_BIGINT },
};


static inline const char* FuncNameByHash ( int iFunc )
{
	if ( iFunc<0 || iFunc >=FUNC_FUNCS_COUNT )
		return ( "unknown");

	static const char * dNames[FUNC_FUNCS_COUNT] =
		{ "now", "abs", "ceil", "floor", "sin", "cos", "ln", "log2", "log10", "exp", "sqrt", "bigint"
		, "sint", "crc32", "fibonacci", "knn_dist", "day", "week", "month", "year", "yearmonth"
		, "yearmonthday", "yearweek", "hour", "minute", "second", "dayofweek", "dayofyear", "quarter"
		, "min", "max", "pow", "idiv", "if", "madd", "mul3", "interval", "in", "bitdot", "remap"
		, "geodist", "exist", "poly2d", "geopoly2d", "contains", "zonespanlist", "concat", "to_string"
		, "rankfactors", "packedfactors", "bm25f", "integer", "double", "length", "least", "greatest"
		, "uint", "uint64", "query", "curtime", "curdata", "time", "date", "dayname", "monthname"
		, "utc_time", "utc_timestamp", "timediff", "datediff", "date_add", "date_sub", "current_user"
		, "connection_id", "all", "any", "indexof", "min_top_weight", "min_top_sortval", "atan2", "rand"
		, "regex", "substring_index", "upper", "lower", "last_insert_id", "levenshtein", "date_format"
		, "database", "user", "version", "range", "histogram", "date_range", "date_histogram", "uuid_short" };

	return dNames[iFunc];
}

// set of functions which evals to SPH_ATTR_STRINGPTR, but also can eval to numerics
static inline bool CanEvalNumbers ( int iFunc )
{
	switch (iFunc)
	{
	case FUNC_CURTIME:
	case FUNC_UTC_TIME:
	case FUNC_UTC_TIMESTAMP:
	case FUNC_TIMEDIFF:
	case FUNC_SUBSTRING_INDEX:
	case FUNC_UPPER:
	case FUNC_LOWER:
	case FUNC_DATE_FORMAT:
		return true;
	default: return false;
	}
}

//////////////////////////////////////////////////////////////////////////

static ISphExpr * ConvertExprJson ( ISphExpr * pExpr );
static void ConvertArgsJson ( VecRefPtrs_t<ISphExpr*> & dArgs );

/// check whether the type is int or bigint
static inline bool IsInt ( ESphAttr eType )
{
	return eType==SPH_ATTR_INTEGER || eType==SPH_ATTR_BIGINT;
}

/// check whether the type can be promoted to integer
static inline bool IsNumericLike ( ESphAttr eType )
{
	switch ( eType )
	{
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_TIMESTAMP:
	case SPH_ATTR_BOOL:
	case SPH_ATTR_BIGINT:
	case SPH_ATTR_TOKENCOUNT:
	case SPH_ATTR_UINT64: return true;
	default: return false;
	}
}

/// check whether the type can be promoted to float
static inline bool IsFloatLike ( ESphAttr eType )
{
	switch ( eType )
	{
	case SPH_ATTR_FLOAT:
	case SPH_ATTR_DOUBLE: return true;
	default: return false;
	}
}

static inline bool IsJson ( ESphAttr eAttr )
{
	return ( eAttr==SPH_ATTR_JSON_FIELD );
}

/// {title=2, body=1}
/// {in=deg, out=mi}
/// argument to functions like BM25F() and GEODIST()
class MapArg_c
{
public:
	CSphVector<CSphNamedVariant> m_dPairs;

public:
	void Add ( CSphString sKey, const char * sValue, int64_t iValue, float fValue, VariantType_e eType )
	{
		CSphNamedVariant & t = m_dPairs.Add();
		t.m_sKey = std::move(sKey);
		t.m_eType = eType;
		switch ( eType )
		{
		case VariantType_e::BIGINT: t.m_iValue = iValue; break;
		case VariantType_e::FLOAT: t.m_fValue = fValue; break;
		case VariantType_e::STRING:
		default: t.m_sValue = sValue;
		};
	}
};


/// expression tree node
/// used to build an AST (Abstract Syntax Tree)
struct ExprNode_t
{
	int				m_iToken = 0;	///< token type, including operators
	ESphAttr		m_eRetType { SPH_ATTR_NONE };	///< result type
	ESphAttr		m_eArgType { SPH_ATTR_NONE };	///< args type
	CSphAttrLocator	m_tLocator;	///< attribute locator, for TOK_ATTR type
	int				m_iLocator = -1; ///< index of attribute locator in schema

	union
	{
		int64_t			m_iConst;		///< constant value, for TOK_CONST_INT type
		float			m_fConst;		///< constant value, for TOK_CONST_FLOAT type
		int				m_iFunc;		///< built-in function id, for TOK_FUNC type
		int				m_iArgs;		///< args count, for arglist (token==',') type
		ConstList_c *	m_pConsts;		///< constants list, for TOK_CONST_LIST type
		MapArg_c	*	m_pMapArg;		///< map argument (maps name to const or name to expr), for TOK_MAP_ARG type
		const char	*	m_sIdent;		///< pointer to const char, for TOK_IDENT type
		SphAttr_t	*	m_pAttr;		///< pointer to 64-bit value, for TOK_ITERATOR type
		INIT_WITH_0 ( int64_t, float, int, ConstList_c*, MapArg_c*, const char*, SphAttr_t* );
	};
	int				m_iLeft = -1;
	int				m_iRight = -1;
};

struct StackNode_t
{
	int m_iNode;
	int m_iLeft;
	int m_iRight;
};

/// expression parser
class ExprParser_t
{
	friend int				yy1lex ( YYSTYPE * lvalp, void * yyscanner, ExprParser_t * pParser );
	friend int				yy1lex ( YYSTYPE *, ExprParser_t * );
	friend int				yylex ( YYSTYPE * lvalp, ExprParser_t * pParser );
	friend int				yyparse ( ExprParser_t * pParser );
	friend void				yyerror ( ExprParser_t * pParser, const char * sMessage );

public:
	ExprParser_t ( ISphExprHook * pHook, QueryProfile_c * pProfiler, ESphCollation eCollation )
		: m_pHook ( pHook )
		, m_pProfiler ( pProfiler )
		, m_eCollation ( eCollation )
	{
		m_dGatherStack.Reserve ( 64 );
	}

							~ExprParser_t ();
	ISphExpr *				Parse ( const char * szExpr, const ISphSchema & tSchema, const CSphString * pJoinIdx, ESphAttr * pAttrType, bool * pUsesWeight, CSphString & sError );

protected:
	int						m_iParsed = 0;	///< filled by yyparse() at the very end
	CSphString				m_sLexerError;
	CSphString				m_sParserError;
	CSphString				m_sCreateError;
	ISphExprHook *			m_pHook;
	QueryProfile_c *		m_pProfiler;

protected:
	ESphAttr				GetWidestRet ( int iLeft, int iRight );

	int						AddNodeInt ( int64_t iValue );
	int						AddNodeFloat ( float fValue );
	int						AddNodeString ( int64_t iValue );
	int						AddNodeAttr ( int iTokenType, uint64_t uAttrLocator );
	int						AddNodeField ( int iTokenType, uint64_t uAttrLocator );
	int						AddNodeColumnar ( int iTokenType, uint64_t uAttrLocator );
	int						AddNodeWeight();
	int						AddNodeOp ( int iOp, int iLeft, int iRight );
	int						AddNodeFunc0 ( int iFunc );
	int						AddNodeFunc ( int iFunc, int iArg );
	int						AddNodeFor ( int iFunc, int iExpr, int iLoop );
	int						AddNodeDate ( int iFunc, int iExpr1, int iExpr2, int iUnit );
	int						AddNodeIn ( int iArg, int iList );
	int						AddNodeRemap ( int iExpr1, int iExpr2, int iList1, int iList2 );
	int						AddNodeRand ( int iArg );
	int						AddNodeUdf ( int iCall, int iArg );
	int						AddNodePF ( int iFunc, int iArg );
	int						AddNodeConstlist ( int64_t iValue, bool bPackedString );
	int						AddNodeConstlist ( float iValue );
	void					AppendToConstlist ( int iNode, int64_t iValue );
	void					AppendToConstlist ( int iNode, float iValue );
	int						AddNodeUservar ( int iUservar );
	int						AddNodeHookIdent ( int iID );
	int						AddNodeHookFunc ( int iID, int iLeft );
	int						AddNodeHookFunc ( int iID );
	int						AddNodeMapArg ( const char * szKey, const char * szValue, int64_t iValue, float fValue, VariantType_e eType );
	void					AppendToMapArg ( int iNode, const char * szKey, const char * szValue, int64_t iValue, float fValue, VariantType_e eType );
	const char *			Attr2Ident ( uint64_t uAttrLoc );
	const char *			Field2Ident ( uint64_t uAttrLoc );
	int						AddNodeJsonField ( uint64_t uAttrLocator, int iLeft );
	int						AddNodeJsonSubkey ( int64_t iValue );
	int						AddNodeDotNumber ( int64_t iValue );
	int						AddNodeIdent ( const char * sKey, int iLeft );

	int						AddNodeWithTable ( const char * szTable, uint64_t uOffset );
	int						AddWeightWithTable ( const char * szTable, uint64_t uOffset );
	uint64_t				ParseAttrWithTable ( const char * szTable, uint64_t uOffset );

private:
	void *					m_pScanner = nullptr;
	Str_t					m_sExpr;
	const ISphSchema *		m_pSchema = nullptr;
	const CSphString *		m_pJoinIdx = nullptr;
	CSphVector<ExprNode_t>	m_dNodes;
	StrVec_t				m_dUservars;
	CSphVector<char*>		m_dIdents;
	int						m_iConstNow = 0;
	CSphVector<StackNode_t>	m_dGatherStack;
	CSphVector<UdfCall_t*>	m_dUdfCalls;

public:
	bool					m_bHasZonespanlist = false;
	DWORD					m_uPackedFactorFlags { SPH_FACTOR_DISABLE };
	ESphEvalStage			m_eEvalStage { SPH_EVAL_FINAL };
	ESphCollation			m_eCollation;
	DWORD					m_uStoredField = CSphColumnInfo::FIELD_NONE;
	bool					m_bNeedDocIds = false;

private:
	bool					CheckGeodist ( YYSTYPE * lvalp );
	void					AddUservar (  const char * sBegin, int iLen, YYSTYPE * lvalp );
	int						ProcessRawToken (  const char * sBegin, int iLen, YYSTYPE * lvalp );
	int						ProcessAtRawToken (  const char * sBegin, int iLen, YYSTYPE * lvalp );
	int						ErrLex ( const char * sTemplate, ...); // issue lexer error
	int						CheckForFields ( Tokh_e eTok, YYSTYPE * lvalp );

	CSphVector<int>			GatherArgTypes ( int iNode );
	CSphVector<int>			GatherArgNodes ( int iNode );
	void					GatherArgRetTypes ( int iNode, CSphVector<ESphAttr> & dTypes );
	template < typename FN >
	void					GatherArgFN ( int iNode, FN && fnFunctor );

	bool					CheckForConstSet ( int iArgsNode, int iSkip );
	int						ParseAttr ( int iAttr, const char* sTok, YYSTYPE * lvalp );
	static int				ParseField ( int iField, const char* sTok, YYSTYPE * lvalp );
	int						ParseAttrsAndFields ( const char * szTok, YYSTYPE * lvalp );
	int						ParseJoinAttr ( const char * szTable, uint64_t uOffset );

	template < typename T >
	void					WalkTree ( int iRoot, T & FUNCTOR );

	void					Optimize ( int iNode );
	void					CanonizePass ( int iNode );
	void					ConstantFoldPass ( int iNode );
	void					VariousOptimizationsPass ( int iNode );
	void					MultiNEPass ( int iNode );
	bool					MultiNEMatch ( const ExprNode_t * pLeft, const ExprNode_t * pRight, ExprNode_t & tRes, CSphVector<int64_t> & dValues );
	bool					TransformNENE ( ExprNode_t * pRoot, ExprNode_t * pLeft, ExprNode_t * pRight );
	bool					TransformInNE ( ExprNode_t * pRoot, ExprNode_t * pLeft, ExprNode_t * pRight );
	void					Dump ( int iNode );

	ISphExpr *				CreateTree ( int iNode );
	ISphExpr *				CreateIntervalNode ( int iArgsNode, CSphVector<ISphExpr *> & dArgs );
	ISphExpr *				CreateInNode ( int iNode );
	ISphExpr *				CreateLengthNode ( const ExprNode_t & tNode, ISphExpr * pLeft );
	ISphExpr *				CreateGeodistNode ( int iArgs );
	ISphExpr *				CreatePFNode ( int iArg );
	ISphExpr *				CreateBitdotNode ( int iArgsNode, CSphVector<ISphExpr *> & dArgs );
	ISphExpr *				CreateUdfNode ( int iCall, ISphExpr * pLeft );
	ISphExpr *				CreateExistNode ( const ExprNode_t & tNode );
	ISphExpr *				CreateContainsNode ( const ExprNode_t & tNode );
	ISphExpr *				CreateAggregateNode ( const ExprNode_t & tNode, ESphAggrFunc eFunc, ISphExpr * pLeft );
	ISphExpr *				CreateForInNode ( int iNode );
	ISphExpr *				CreateExprDateAdd ( int iNode, bool bAdd );
	ISphExpr *				CreateRegexNode ( ISphExpr * pAttr, ISphExpr * pString );
	ISphExpr *				CreateConcatNode ( int iArgsNode, CSphVector<ISphExpr *> & dArgs );
	ISphExpr *				CreateFieldNode ( int iField );

	ISphExpr *				CreateColumnarIntNode ( int iAttr, ESphAttr eAttrType );
	ISphExpr *				CreateColumnarFloatNode ( int iAttr );
	ISphExpr *				CreateColumnarStringNode ( int iAttr );
	ISphExpr *				CreateColumnarMvaNode ( int iAttr );

	void					FixupIterators ( int iNode, const char * sKey, SphAttr_t * pAttr );
	ISphExpr *				CreateLevenshteinNode ( ISphExpr * pPattern, ISphExpr * pAttr, ISphExpr * pOpts );

	ISphExpr *				CreateCmp ( const ExprNode_t & tNode, ISphExpr * pLeft, ISphExpr * pRight );

	bool					CheckStoredArg ( ISphExpr * pExpr );
	bool					PrepareFuncArgs ( const ExprNode_t & tNode, bool bSkipChildren, CSphRefcountedPtr<ISphExpr> & pLeft, CSphRefcountedPtr<ISphExpr> & pRight, VecRefPtrs_t<ISphExpr*> & dArgs );
	ISphExpr *				CreateFuncExpr ( int iNode, VecRefPtrs_t<ISphExpr*> & dArgs );
	CSphString				GetNameByLocator ( int iNode ) const;
	CSphString				GetNameByLocator ( const ExprNode_t & tNode ) const;

	bool					GetError () const { return !( m_sLexerError.IsEmpty() && m_sParserError.IsEmpty() && m_sCreateError.IsEmpty() ); }
	bool					GetCreateError () const { return !m_sCreateError.IsEmpty(); }

	ISphExpr *				Create ( bool * pUsesWeight, CSphString & sError );
};

//////////////////////////////////////////////////////////////////////////

// used to store in 8 bytes in Bison lvalp variable
static uint64_t sphPackAttrLocator ( const CSphAttrLocator & tLoc, int iLocator )
{
	assert ( iLocator>=0 && iLocator<=0x7fff );

	uint64_t uIndex = 0;
	bool bBlob = tLoc.m_iBlobAttrId>=0;
	if ( bBlob )
	{
		assert ( tLoc.m_nBlobAttrs>=0 && tLoc.m_nBlobAttrs<=0x7fff );
		uIndex = tLoc.m_iBlobAttrId + ( tLoc.m_nBlobAttrs<<15 ) + ( tLoc.m_iBlobRowOffset<<30 ) + ( (uint64_t)iLocator<<32 );
	}
	else
		uIndex = tLoc.m_iBitCount + ( tLoc.m_iBitOffset<<16 ) + ( (uint64_t)iLocator<<32 );

	if ( tLoc.m_bDynamic )
		uIndex |= ( U64C(1)<<63 );

	if ( bBlob )
		uIndex |= ( U64C(1)<<62 );

	return uIndex;
}

static void sphUnpackAttrLocator ( uint64_t uIndex, ExprNode_t * pNode )
{
	assert ( pNode );
	bool bBlob = ( uIndex & ( U64C(1)<<62 ) )!=0;

	if ( bBlob )
	{
		pNode->m_tLocator.m_iBlobAttrId = (int)( uIndex & 0x7fff );
		pNode->m_tLocator.m_nBlobAttrs = (int)( ( uIndex>>15 ) & 0x7fff );
		pNode->m_tLocator.m_iBlobRowOffset = (int)( ( uIndex>>30 ) & 1 );

		pNode->m_tLocator.m_iBitCount = -1;
		pNode->m_tLocator.m_iBitOffset = -1;
	}
	else
	{
		pNode->m_tLocator.m_iBitCount = (int)( uIndex & 0xffff );
		pNode->m_tLocator.m_iBitOffset = (int)( ( uIndex>>16 ) & 0xffff );

		pNode->m_tLocator.m_iBlobAttrId = -1;
		pNode->m_tLocator.m_nBlobAttrs = 0;
	}

	pNode->m_tLocator.m_bDynamic = ( ( uIndex & ( U64C(1)<<63 ) )!=0 );
	pNode->m_iLocator = (int)( ( uIndex>>32 ) & 0x7fff );
}


static int GetConstStrOffset ( const ExprNode_t & tNode )
{
	return GetConstStrOffset ( tNode.m_iConst );
}


static int GetConstStrLength ( const ExprNode_t & tNode )
{
	return GetConstStrLength ( tNode.m_iConst );
}

/// format error
int ExprParser_t::ErrLex ( const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	m_sLexerError.SetSprintfVa ( sTemplate, ap );
	va_end ( ap );
	return -1;
}

static int ConvertToColumnarType ( ESphAttr eAttr )
{
	switch ( eAttr )
	{
	case SPH_ATTR_INTEGER:		return TOK_COLUMNAR_INT;
	case SPH_ATTR_TIMESTAMP:	return TOK_COLUMNAR_TIMESTAMP;
	case SPH_ATTR_FLOAT:		return TOK_COLUMNAR_FLOAT;
	case SPH_ATTR_BIGINT:		return TOK_COLUMNAR_BIGINT;
	case SPH_ATTR_BOOL:			return TOK_COLUMNAR_BOOL;
	case SPH_ATTR_STRING:		return TOK_COLUMNAR_STRING;
	case SPH_ATTR_UINT32SET:	return TOK_COLUMNAR_UINT32SET;
	case SPH_ATTR_INT64SET:		return TOK_COLUMNAR_INT64SET;
	case SPH_ATTR_FLOAT_VECTOR:	return TOK_COLUMNAR_FLOATVEC;
	default:
		assert ( 0 && "Unknown columnar type" );
		return -1;
	}
}


int ExprParser_t::ParseAttr ( int iAttr, const char* sTok, YYSTYPE * lvalp )
{
	// check attribute type and width
	const CSphColumnInfo & tCol = m_pSchema->GetAttr ( iAttr );

	// check for a duplicate attribute created for showing a stored field in the result set
	if ( tCol.m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
	{
		const CSphVector<CSphColumnInfo> & dFields = m_pSchema->GetFields();
		ARRAY_FOREACH ( i, dFields )
			if ( dFields[i].m_sName==tCol.m_sName )
			{
				lvalp->iAttrLocator = i;
				return TOK_FIELD;
			}
	}

	if ( tCol.IsColumnar() )
	{
		lvalp->iAttrLocator = iAttr;
		return ConvertToColumnarType ( tCol.m_eAttrType );
	}

	int iRes = -1;
	switch ( tCol.m_eAttrType )
	{
	case SPH_ATTR_FLOAT:			iRes = TOK_ATTR_FLOAT;	break;

	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_UINT32SET_PTR:	iRes = TOK_ATTR_MVA32; break;

	case SPH_ATTR_INT64SET:
	case SPH_ATTR_INT64SET_PTR:		iRes = TOK_ATTR_MVA64; break;

	case SPH_ATTR_STRING:
	case SPH_ATTR_STRINGPTR:		iRes = TOK_ATTR_STRING; break;

	case SPH_ATTR_JSON:
	case SPH_ATTR_JSON_PTR:
	case SPH_ATTR_JSON_FIELD:
	case SPH_ATTR_JSON_FIELD_PTR:	iRes = TOK_ATTR_JSON; break;

	case SPH_ATTR_FACTORS:			iRes = TOK_ATTR_FACTORS; break;

	case SPH_ATTR_INTEGER:
	case SPH_ATTR_TIMESTAMP:
	case SPH_ATTR_BOOL:
	case SPH_ATTR_BIGINT:
	case SPH_ATTR_TOKENCOUNT:
		iRes = tCol.m_tLocator.IsBitfield() ? TOK_ATTR_BITS : TOK_ATTR_INT;
		break;
	default:
		m_sLexerError.SetSprintf ( "attribute '%s' is of unsupported type (type=%d)", sTok, tCol.m_eAttrType );
		return -1;
	}

	lvalp->iAttrLocator = sphPackAttrLocator ( tCol.m_tLocator, iAttr );
	return iRes;
}


int ExprParser_t::ParseField ( int iField, const char* sTok, YYSTYPE * lvalp )
{
	lvalp->iAttrLocator = iField;
	return TOK_FIELD;
}

bool ExprParser_t::CheckGeodist ( YYSTYPE * lvalp )
{
	int iGeodist = m_pSchema->GetAttrIndex ( "@geodist" );
	if ( iGeodist==-1 )
		return false;
	const CSphAttrLocator & tLoc = m_pSchema->GetAttr ( iGeodist ).m_tLocator;
	lvalp->iAttrLocator = sphPackAttrLocator ( tLoc, iGeodist );
	return true;
}

void ExprParser_t::AddUservar ( const char* sBegin, int iLen, YYSTYPE * lvalp )
{
	lvalp->iNode = m_dUservars.GetLength ();
	CSphString sTok { sBegin, iLen };
	m_dUservars.Add ( sTok );
}

int ExprParser_t::ParseAttrsAndFields ( const char * szTok, YYSTYPE * lvalp )
{
	// check for attribute
	int iCol = m_pSchema->GetAttrIndex ( szTok );
	if ( iCol>=0 )
		return ParseAttr ( iCol, szTok, lvalp );

	// check for field
	iCol = m_pSchema->GetFieldIndex ( szTok );
	if ( iCol>=0 )
		return ParseField ( iCol, szTok, lvalp );

	return -1;
}

// process tokens starting with @
int ExprParser_t::ProcessAtRawToken ( const char * sBegin, int iLen, YYSTYPE * lvalp )
{
	int iRes = -1;
	if ( strncasecmp ( sBegin, "@id", iLen )==0 )
		return ParseAttrsAndFields ( "id", lvalp );
	else if ( strncasecmp ( sBegin, "@geodist", iLen )==0 )
	{
		iRes = ParseAttrsAndFields ( "@geodist", lvalp );
		if (iRes<0)
			m_sLexerError = "geoanchor is not set, @geodist expression unavailable";
		return iRes;
	} else if ( strncasecmp ( sBegin, "@weight", iLen )==0 )
		return TOK_ATWEIGHT;

	CSphString sTok { sBegin, iLen };
	lvalp->iNode = m_dUservars.GetLength ();
	m_dUservars.Add ( sTok );
	return TOK_USERVAR;
}

inline static bool IsFunc ( Tokh_e e )
{
	return e<FUNC_FUNCS_COUNT;
}

inline static bool IsTok ( Tokh_e e )
{
	return e<TOKH_TOKH_COUNT && e>=FUNC_FUNCS_COUNT;
}


int ExprParser_t::CheckForFields ( Tokh_e eTok, YYSTYPE * lvalp )
{
	if ( eTok==TOKH_COUNT ) // in case someone used 'count' as a name for an attribute
		return ParseAttrsAndFields ("count", lvalp);

	if ( eTok==TOKH_WEIGHT ) // in case someone used 'weight' as a name for an attribute
		return ParseAttrsAndFields ("weight", lvalp);

	return -1;
}

// general flow: flex parser do most generic tokenization, and provides raw token.
// here we look it in the schema, perfect hash, overrides, udfs, etc. and provide concrete result
int ExprParser_t::ProcessRawToken ( const char * sToken, int iLen, YYSTYPE * lvalp )
{
	int iRes = -1;
	const bool bFunc = lvalp->iTrailingBr!=0;
	if ( lvalp->iTrailingBr==2 ) // trim trailing 	[ \t\n\r]
	{
		while ( sphIsSpace ( sToken[iLen-1] ) )
			--iLen;
	}
	lvalp->iTrailingBr = 0;

	auto eTok = TokHashLookup ( { sToken, iLen } );
	if ( IsTok(eTok) )
	{
		if ( !bFunc )
		{
			iRes = CheckForFields ( eTok, lvalp );
			if ( iRes>=0 )
				return iRes;
		}

		return g_dHash2Op[eTok-FUNC_FUNCS_COUNT];
	}

	CSphString sTok;
	sTok.SetBinary ( sToken, iLen );
	sTok.ToLower ();

	// check for attributes and fields
	if ( !bFunc )
	{
		iRes = ParseAttrsAndFields ( sTok.cstr(), lvalp );
		if ( iRes>=0 )
			return iRes;
	}

	// check for table name
	if ( m_pJoinIdx && *m_pJoinIdx==sTok )
	{
		CSphString sTokMixed { sToken, iLen };
		m_dIdents.Add ( sTokMixed.Leak() );
		lvalp->sIdent = m_dIdents.Last();
		return TOK_TABLE_NAME;
	}

	if ( sToken[0]=='@' )
		return ProcessAtRawToken ( sToken, iLen, lvalp );

	// ask hook func, it may override
	if ( m_pHook && eTok==FUNC_BM25F ) // tiny ad-hoc - as only known override bm25f, so hardcode it
	{
		int iID = m_pHook->IsKnownFunc ( sTok.cstr () );
		if ( iID>=0 )
		{
			lvalp->iFunc = iID;
			return TOK_HOOK_FUNC;
		}
	}

	// check for function
	if ( IsFunc ( eTok ) )
	{
		lvalp->iFunc = eTok;
		return g_dFuncs[eTok].m_iNodeType;
	}

	// ask hook ident
	if ( m_pHook )
	{
		int iID = m_pHook->IsKnownFunc ( sTok.cstr () );
		if ( iID>=0 )
		{
			lvalp->iFunc = iID;
			return TOK_HOOK_FUNC;
		}
		iID = m_pHook->IsKnownIdent ( sTok.cstr () );
		if ( iID>=0 )
		{
			lvalp->iNode = iID;
			return TOK_HOOK_IDENT;
		}
	}

	// check for UDF
	auto pUdf = PluginGet<PluginUDF_c> ( PLUGIN_FUNCTION, sTok.cstr() );
	if ( pUdf )
	{
		lvalp->iNode = m_dUdfCalls.GetLength();
		m_dUdfCalls.Add ( new UdfCall_t() );
		m_dUdfCalls.Last()->m_pUdf = std::move ( pUdf );
		return TOK_UDF;
	}

	// arbitrary identifier, then
	CSphString sTokMixed { sToken, iLen };
	m_dIdents.Add ( sTokMixed.Leak() );
	lvalp->sIdent = m_dIdents.Last();
	return TOK_IDENT;
}

/// is add/sub?
static inline bool IsAddSub ( const ExprNode_t * pNode )
{
	if ( pNode )
		return pNode->m_iToken=='+' || pNode->m_iToken=='-';
	assert ( 0 && "null node passed to IsAddSub()" );
	return false;
}

/// is unary operator?
static inline bool IsUnary ( const ExprNode_t * pNode )
{
	if ( pNode )
		return pNode->m_iToken==TOK_NEG || pNode->m_iToken==TOK_NOT;
	assert ( 0 && "null node passed to IsUnary() ");
	return false;
}

/// is arithmetic?
static inline bool IsAri ( const ExprNode_t * pNode )
{
	if ( pNode )
	{
		int iTok = pNode->m_iToken;
		return iTok=='+' || iTok=='-' || iTok=='*' || iTok=='/';
	}
	assert ( 0 && "null node passed to IsAri()" );
	return false;
}

/// is constant?
static inline bool IsConst ( const ExprNode_t * pNode )
{
	if ( pNode )
		return pNode->m_iToken==TOK_CONST_INT || pNode->m_iToken==TOK_CONST_FLOAT;
	assert ( 0 && "null node passed to IsConst()" );
	return false;
}

/// float value of a constant
static inline float FloatVal ( const ExprNode_t * pNode )
{
	assert ( IsConst(pNode) );
	return pNode->m_iToken==TOK_CONST_INT
		? (float)pNode->m_iConst
		: pNode->m_fConst;
}

void ExprParser_t::CanonizePass ( int iNode )
{
	if ( iNode<0 )
		return;

	CanonizePass ( m_dNodes [ iNode ].m_iLeft );
	CanonizePass ( m_dNodes [ iNode ].m_iRight );

	ExprNode_t * pRoot = &m_dNodes [ iNode ];
	ExprNode_t * pLeft = ( pRoot->m_iLeft>=0 ) ? &m_dNodes [ pRoot->m_iLeft ] : nullptr;
	ExprNode_t * pRight = ( pRoot->m_iRight>=0 ) ? &m_dNodes [ pRoot->m_iRight ] : nullptr;

	// canonize (expr op const), move const to the left
	if ( pLeft && pRight && IsAri ( pRoot ) && !IsConst ( pLeft ) && IsConst ( pRight ) )
	{
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
			pLeft->m_fConst = 1.0f / FloatVal ( pLeft );
			pLeft->m_iToken = TOK_CONST_FLOAT;
		}
	}

	// promote children constants
	if ( pLeft && IsAri ( pRoot ) && IsAri ( pLeft ) && IsAddSub ( pLeft )==IsAddSub ( pRoot ) &&
		IsConst ( &m_dNodes [ pLeft->m_iLeft ] ) )
	{
		// ((const op lr) op2 right) gets replaced with (const op (lr op2/op right))
		// constant gets promoted one level up
		int iConst = pLeft->m_iLeft;
		int iCenter = pLeft->m_iRight;
		int iRight = pRoot->m_iRight;
		int iOpLeft = pLeft->m_iToken;
		int iOp = pRoot->m_iToken;

		int iOpNode = pRoot->m_iLeft;

		// swap here is necessary in order to keep constraight (iRoot>iLeft) && (iRoot>iRight)
		// that is, in turn, makes possible to create tree sequentally instead of recursive.
		Swap ( m_dNodes[iOpNode], m_dNodes[iRight] );
		Swap ( iOpNode, iRight );

		switch ( iOpLeft )
		{
		case '+':
		case '*':
			// (c + lr) op r -> c + (lr op r)
			// (c * lr) op r -> c * (lr op r)
			Swap ( iOpLeft, iOp );
			break;

		case '-':
			// (c - lr) + r -> c - (lr - r)
			// (c - lr) - r -> c - (lr + r)
			iOpLeft = ( iOp=='+' ? '-' : '+' );
			iOp = '-';
			break;

		case '/':
			// (c / lr) * r -> c * (r / lr)
			// (c / lr) / r -> c / (r * lr)
			Swap ( iCenter, iRight );
			iOpLeft = ( iOp=='*' ? '/' : '*' );
			break;

		default:
			assert ( 0 && "internal error: unhandled op in left-const promotion" );
		}

		pRoot->m_iLeft = iConst;

		pRoot->m_iRight = iOpNode;
		m_dNodes[iOpNode].m_iLeft = iCenter;
		m_dNodes[iOpNode].m_iRight = iRight;
		m_dNodes[iOpNode].m_iToken = iOpLeft;
		pRoot->m_iToken = iOp;
	}
}

void ExprParser_t::ConstantFoldPass ( int iNode )
{
	if ( iNode<0 )
		return;

	ConstantFoldPass ( m_dNodes [ iNode ].m_iLeft );
	ConstantFoldPass ( m_dNodes [ iNode ].m_iRight );

	ExprNode_t * pRoot = &m_dNodes [ iNode ];
	ExprNode_t * pLeft = ( pRoot->m_iLeft>=0 ) ? &m_dNodes [ pRoot->m_iLeft ] : nullptr;
	ExprNode_t * pRight = ( pRoot->m_iRight>=0 ) ? &m_dNodes [ pRoot->m_iRight ] : nullptr;

	// unary arithmetic expression with constant
	if ( IsUnary ( pRoot ) && pLeft && IsConst ( pLeft ) )
	{
		if ( pLeft->m_iToken==TOK_CONST_INT )
		{
			switch ( pRoot->m_iToken )
			{
				case TOK_NEG:	pRoot->m_iConst = -pLeft->m_iConst; break;
				case TOK_NOT:	pRoot->m_iConst = !pLeft->m_iConst; break;
				default:		assert ( 0 && "internal error: unhandled arithmetic token during const-int optimization" );
			}

		} else
		{
			switch ( pRoot->m_iToken )
			{
				case TOK_NEG:	pRoot->m_fConst = -pLeft->m_fConst; break;
				case TOK_NOT:	pRoot->m_fConst = pLeft->m_fConst==0.0f; break;
				default:		assert ( 0 && "internal error: unhandled arithmetic token during const-float optimization" );
			}
		}

		pRoot->m_iToken = pLeft->m_iToken;
		pRoot->m_iLeft = -1;
		pLeft->m_iToken = 0;
		return;
	}

	// arithmetic expression with constants
	if ( IsAri ( pRoot ) )
	{
		assert ( pLeft && pRight );

		// optimize fully-constant expressions
		if ( IsConst ( pLeft ) && IsConst ( pRight ) )
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
				float fLeft = FloatVal ( pLeft );
				float fRight = FloatVal ( pRight );
				switch ( pRoot->m_iToken )
				{
					case '+':	pRoot->m_fConst = fLeft + fRight; break;
					case '-':	pRoot->m_fConst = fLeft - fRight; break;
					case '*':	pRoot->m_fConst = fLeft * fRight; break;
					case '/':	pRoot->m_fConst = fRight!=0.0f ? fLeft / fRight : 0.0f; break; // FIXME! We don't have 'NULL', cant distinguish from 0.0f
					default:	assert ( 0 && "internal error: unhandled arithmetic token during const-float optimization" );
				}
				pRoot->m_iToken = TOK_CONST_FLOAT;
			}
			pRoot->m_iLeft = -1;
			pLeft->m_iToken = 0;
			pRoot->m_iRight = -1;
			pRight->m_iToken = 0;
			return;
		}

		if ( IsConst ( pLeft ) && IsAri ( pRight ) && IsAddSub ( pRoot )==IsAddSub ( pRight ) &&
			IsConst ( &m_dNodes[pRight->m_iLeft] ) )
		{
			ExprNode_t * pConst = &m_dNodes[pRight->m_iLeft];
			assert ( !IsConst ( &m_dNodes [ pRight->m_iRight ] ) ); // must had been optimized

			// optimize (left op (const op2 expr)) to ((left op const) op*op2 expr)
			if ( IsAddSub ( pRoot ) )
			{
				// fold consts
				int iSign = ( ( pRoot->m_iToken=='+' ) ? 1 : -1 );
				if ( pLeft->m_iToken==TOK_CONST_INT && pConst->m_iToken==TOK_CONST_INT )
				{
					pLeft->m_iConst += iSign*pConst->m_iConst;
				} else
				{
					pLeft->m_iToken = TOK_CONST_FLOAT;
					pLeft->m_fConst = FloatVal ( pLeft ) + iSign*FloatVal ( pConst );
				}

				// fold ops
				pRoot->m_iToken = ( pRoot->m_iToken==pRight->m_iToken ) ? '+' : '-';

			} else
			{
				// fold consts
				if ( pRoot->m_iToken=='*' && pLeft->m_iToken==TOK_CONST_INT && pConst->m_iToken==TOK_CONST_INT )
				{
					pLeft->m_iConst *= pConst->m_iConst;
				} else
				{
					if ( pRoot->m_iToken=='*' )
						pLeft->m_fConst = FloatVal ( pLeft ) * FloatVal ( pConst );
					else
						pLeft->m_fConst = FloatVal ( pLeft ) / FloatVal ( pConst );
					pLeft->m_iToken = TOK_CONST_FLOAT;
				}

				// fold ops
				pRoot->m_iToken = ( pRoot->m_iToken==pRight->m_iToken ) ? '*' : '/';
			}

			// promote expr arg
			pRoot->m_iRight = pRight->m_iRight;
			pRight->m_iToken = 0;
		}
	}

	// unary function from a constant
	if ( pRoot->m_iToken==TOK_FUNC && g_dFuncs [ pRoot->m_iFunc ].m_iArgs==1 && IsConst ( pLeft ) )
	{
		float fArg = pLeft->m_iToken==TOK_CONST_FLOAT ? pLeft->m_fConst : float ( pLeft->m_iConst );
		switch ( pRoot->m_iFunc )
		{
		case FUNC_ABS:
			pRoot->m_iToken = pLeft->m_iToken;
			pRoot->m_iLeft = -1;
			if ( pLeft->m_iToken==TOK_CONST_INT )
				pRoot->m_iConst = IABS ( pLeft->m_iConst );
			else
				pRoot->m_fConst = (float)fabs ( fArg );
			pLeft->m_iToken = 0;
			break;
		case FUNC_CEIL:		pRoot->m_iToken = TOK_CONST_INT; pRoot->m_iLeft = -1; pRoot->m_iConst = (int64_t)ceil ( fArg ); pLeft->m_iToken = 0; break;
		case FUNC_FLOOR:	pRoot->m_iToken = TOK_CONST_INT; pRoot->m_iLeft = -1; pRoot->m_iConst = (int64_t)floor ( fArg ); pLeft->m_iToken = 0; break;
		case FUNC_SIN:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = float ( sin ( fArg) ); pLeft->m_iToken = 0; break;
		case FUNC_COS:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = float ( cos ( fArg ) ); pLeft->m_iToken = 0; break;
		case FUNC_LN:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = fArg>0.0f ? (float) log(fArg) : 0.0f; pLeft->m_iToken = 0; break;
		case FUNC_LOG2:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = fArg>0.0f ? (float)( log(fArg)*M_LOG2E ) : 0.0f; pLeft->m_iToken = 0; break;
		case FUNC_LOG10:	pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = fArg>0.0f ? (float)( log(fArg)*M_LOG10E ) : 0.0f; pLeft->m_iToken = 0; break;
		case FUNC_EXP:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = float ( exp ( fArg ) ); pLeft->m_iToken = 0; break;
		case FUNC_SQRT:		pRoot->m_iToken = TOK_CONST_FLOAT; pRoot->m_iLeft = -1; pRoot->m_fConst = fArg>0.0f ? (float)sqrt(fArg) : 0.0f; pLeft->m_iToken = 0; break;
		default:			break;
		}
		return;
	}
}

void ExprParser_t::VariousOptimizationsPass ( int iNode )
{
	if ( iNode<0 )
		return;

	VariousOptimizationsPass ( m_dNodes [ iNode ].m_iLeft );
	VariousOptimizationsPass ( m_dNodes [ iNode ].m_iRight );

	ExprNode_t * pRoot = &m_dNodes [ iNode ];
	int iLeft = pRoot->m_iLeft;
	int iRight = pRoot->m_iRight;
	ExprNode_t * pLeft = ( iLeft>=0 ) ? &m_dNodes [ iLeft ] : nullptr;
	ExprNode_t * pRight = ( iRight>=0 ) ? &m_dNodes [ iRight ] : nullptr;

	// madd, mul3
	// FIXME! separate pass for these? otherwise (2+(a*b))+3 won't get const folding
	if ( ( pRoot->m_iToken=='+' || pRoot->m_iToken=='*' ) && pLeft && pRight && ( pLeft->m_iToken=='*' || pRight->m_iToken=='*' ) )
	{
		if ( pLeft->m_iToken!='*' )
		{
			Swap ( pRoot->m_iLeft, pRoot->m_iRight );
			Swap ( pLeft, pRight );
			Swap ( iLeft, iRight );
		}

		pLeft->m_iToken = ',';
		pRoot->m_iFunc = ( pRoot->m_iToken=='+' ) ? FUNC_MADD : FUNC_MUL3;
		pRoot->m_iToken = TOK_FUNC;
		return;
	}

	// division by a constant (replace with multiplication by inverse)
	if ( pRoot->m_iToken=='/' && pRight && pRight->m_iToken==TOK_CONST_FLOAT )
	{
		pRight->m_fConst = 1.0f / pRight->m_fConst;
		pRoot->m_iToken = '*';
		return;
	}


	// SINT(int-attr)
	if ( pRoot->m_iToken==TOK_FUNC && pRoot->m_iFunc==FUNC_SINT && pLeft )
	{
		if ( pLeft->m_iToken==TOK_ATTR_INT || pLeft->m_iToken==TOK_ATTR_BITS )
		{
			pRoot->m_iToken = TOK_ATTR_SINT;
			pRoot->m_tLocator = pLeft->m_tLocator;
			pRoot->m_iLeft = -1;
			pLeft->m_iToken = 0;
		}
	}
}


static bool IsSupportedNEType ( int iType )
{
	return iType==TOK_COLUMNAR_INT || iType==TOK_COLUMNAR_BIGINT || iType==TOK_COLUMNAR_BOOL || iType==TOK_ATTR_INT;
}


static bool CheckAndSwap ( ExprNode_t * & pLeft, ExprNode_t * & pRight )
{
	if ( IsSupportedNEType ( pRight->m_iToken ) && pLeft->m_iToken==TOK_CONST_INT )
		Swap ( pLeft, pRight );

	return IsSupportedNEType ( pLeft->m_iToken ) && pRight->m_iToken==TOK_CONST_INT;
}


bool ExprParser_t::MultiNEMatch ( const ExprNode_t * pLeft, const ExprNode_t * pRight, ExprNode_t & tRes, CSphVector<int64_t> & dValues )
{
	assert ( pLeft->m_iLeft!=-1 && pLeft->m_iRight!=-1 );
	assert ( pRight->m_iLeft!=-1 && pRight->m_iRight!=-1 );

	ExprNode_t * pLeft0 = &m_dNodes [ pLeft->m_iLeft ];
	ExprNode_t * pLeft1 = &m_dNodes [ pLeft->m_iRight ];
	ExprNode_t * pRight0 = &m_dNodes [ pRight->m_iLeft ];
	ExprNode_t * pRight1 = &m_dNodes [ pRight->m_iRight ];

	if ( !CheckAndSwap ( pLeft0, pLeft1 ) )
		return false;

	if ( !CheckAndSwap ( pRight0, pRight1 ) )
		return false;

	if ( pRight0->m_iLocator!=pLeft0->m_iLocator || pRight0->m_iToken!=pLeft0->m_iToken )
		return false;

	tRes = *pLeft0;

	dValues.Add ( pLeft1->m_iConst );
	dValues.Add ( pRight1->m_iConst );
	
	return true;
}


bool ExprParser_t::TransformNENE ( ExprNode_t * pRoot, ExprNode_t * pLeft, ExprNode_t * pRight )
{
	assert ( pRoot && pLeft && pRight );
	assert ( pRoot->m_iToken==TOK_AND && pLeft->m_iToken==TOK_NE && pRight->m_iToken==TOK_NE );

	ExprNode_t tRes;
	CSphVector<int64_t> dValues;
	if ( MultiNEMatch ( pLeft, pRight, tRes, dValues ) )
	{
		pRoot->m_iToken = TOK_NOT;
		pRoot->m_iRight = -1;

		// discard optimized tokens
		pRight->m_iToken = 0;
		m_dNodes[pRight->m_iLeft].m_iToken = 0;
		m_dNodes[pRight->m_iRight].m_iToken = 0;

		pLeft->m_iToken = TOK_FUNC;
		pLeft->m_iFunc = FUNC_IN;

		ExprNode_t * pLeft0 = &m_dNodes [ pLeft->m_iLeft ];
		ExprNode_t * pLeft1 = &m_dNodes [ pLeft->m_iRight ];

		pLeft0->m_iToken	= tRes.m_iToken;
		pLeft0->m_iLocator	= tRes.m_iLocator;
		pLeft0->m_tLocator	= tRes.m_tLocator;

		pLeft1->m_iToken = TOK_CONST_LIST;
		pLeft1->m_pConsts = new ConstList_c();
		for ( auto i : dValues )
			pLeft1->m_pConsts->Add(i);

		return true;
	}

	return false;
}


bool ExprParser_t::TransformInNE ( ExprNode_t * pRoot, ExprNode_t * pLeft, ExprNode_t * pRight )
{
	assert ( pRoot && pLeft && pRight );
	assert ( pRoot->m_iToken==TOK_AND && ( ( pLeft->m_iToken==TOK_NOT && pRight->m_iToken==TOK_NE ) || ( pLeft->m_iToken==TOK_NE && pRight->m_iToken==TOK_NOT ) ) );

	ExprNode_t * pNotNode = pLeft->m_iToken==TOK_NOT ? pLeft : pRight;
	ExprNode_t * pNENode = pLeft->m_iToken==TOK_NE ? pLeft : pRight;

	assert ( pNotNode->m_iLeft!=-1 && pNotNode->m_iRight==-1 );
	ExprNode_t * pInNode = &m_dNodes [ pNotNode->m_iLeft ];

	bool bCond = pInNode->m_iToken==TOK_FUNC && pInNode->m_iFunc==FUNC_IN;
	assert ( pInNode->m_iLeft!=-1 && pNotNode->m_iRight==-1 );
	ExprNode_t * pIn0 = &m_dNodes [ pInNode->m_iLeft ];
	ExprNode_t * pIn1 = &m_dNodes [ pInNode->m_iRight ];
	bCond &= IsSupportedNEType ( pIn0->m_iToken ) && pIn1->m_iToken==TOK_CONST_LIST;
	bCond &= pIn1->m_pConsts->m_eRetType==SPH_ATTR_INTEGER || pIn1->m_pConsts->m_eRetType==SPH_ATTR_BIGINT;

	ExprNode_t * pNE0 = &m_dNodes [ pNENode->m_iLeft ];
	ExprNode_t * pNE1 = &m_dNodes [ pNENode->m_iRight ];
	bCond &= CheckAndSwap ( pNE0, pNE1 );
	bCond &= pNE0->m_iToken == pIn0->m_iToken && pNE0->m_iLocator==pIn0->m_iLocator;

	if ( bCond )
	{
		pIn1->m_pConsts->Add ( pNE1->m_iConst );
		*pRoot = *pNotNode;
		// discard optimized tokens
		pNotNode->m_iToken = pNENode->m_iToken = pNE0->m_iToken = pNE1->m_iToken = 0;
		return true;
	}

	return false;
}

// transform "var<>1 AND var<>2 AND var<>3" into "not var in (1,2,3)"
void ExprParser_t::MultiNEPass ( int iNode )
{
	if ( iNode<0 )
		return;

	MultiNEPass ( m_dNodes [ iNode ].m_iLeft );
	MultiNEPass ( m_dNodes [ iNode ].m_iRight );

	ExprNode_t * pRoot = &m_dNodes[iNode];
	if ( pRoot->m_iLeft==-1 || pRoot->m_iRight==-1 )
		return;

	ExprNode_t * pLeft = &m_dNodes [ pRoot->m_iLeft ];
	ExprNode_t * pRight = &m_dNodes [ pRoot->m_iRight ];

	if ( pRoot->m_iToken==TOK_AND && pLeft->m_iToken==TOK_NE && pRight->m_iToken==TOK_NE )
	{
		if ( TransformNENE ( pRoot, pLeft, pRight ) )
			return;
	}

	if ( pRoot->m_iToken==TOK_AND && ( ( pLeft->m_iToken==TOK_NOT && pRight->m_iToken==TOK_NE ) || ( pLeft->m_iToken==TOK_NE && pRight->m_iToken==TOK_NOT ) ) )
	{
		if ( TransformInNE ( pRoot, pLeft, pRight ) )
			return;
	}
}

static const char * TokName (int iTok, int iFunc)
{
	if ( iTok<256 )
	{
		if ( iTok<-0 )
			return "deleted_token_fixme";
		static char onechar[2] = { 0 };
		onechar[0] = char(iTok);
		return onechar;
	}
	switch ( (yytokentype) iTok)
	{
		case TOK_CONST_INT:    return "const_int";
		case TOK_CONST_FLOAT:  return "const_float";
		case TOK_CONST_STRING: return "const_string";
		case TOK_SUBKEY:       return "subkey";
		case TOK_DOT_NUMBER:   return "dot_number";
		case TOK_ATTR_INT:     return "attr_int";
		case TOK_ATTR_BITS:    return "attr_bits";
		case TOK_ATTR_FLOAT:   return "attr_float";
		case TOK_ATTR_MVA32:   return "attr_mva32";
		case TOK_ATTR_MVA64:   return "attr_mva64";
		case TOK_ATTR_STRING:  return "attr_string";
		case TOK_ATTR_FACTORS: return "attr_factors";
		case TOK_IF:           return "if";
		case TOK_FUNC:         return FuncNameByHash ( iFunc );
		case TOK_FUNC_IN:      return "func_in";
		case TOK_FUNC_RAND:    return "func_rand";
		case TOK_FUNC_REMAP:   return "func_remap";
		case TOK_FUNC_PF:      return "func_pf";
		case TOK_FUNC_JA:      return "func_ja";
		case TOK_FUNC_DATE:    return "func_date";
		case TOK_USERVAR:      return "uservar";
		case TOK_UDF:          return "udf";
		case TOK_HOOK_IDENT:   return "hook_ident";
		case TOK_HOOK_FUNC:    return "hook_func";
		case TOK_IDENT:        return "ident";
		case TOK_ATTR_JSON:    return "attr_json";
		case TOK_FIELD:        return "field";
		case TOK_COLUMNAR_INT:       return "columnar_int";
		case TOK_COLUMNAR_TIMESTAMP: return "columnar_timestamp";
		case TOK_COLUMNAR_BIGINT:    return "columnar_bigint";
		case TOK_COLUMNAR_BOOL:      return "columnar_bool";
		case TOK_COLUMNAR_FLOAT:     return "columnar_float";
		case TOK_COLUMNAR_STRING:    return "columnar_string";
		case TOK_COLUMNAR_UINT32SET: return "columnar_uint32set";
		case TOK_COLUMNAR_INT64SET:  return "columnar_int64set";
		case TOK_COLUMNAR_FLOATVEC:  return "columnar_floatvec";
		case TOK_ATWEIGHT:     return "atweight";
		case TOK_GROUPBY:      return "groupby";
		case TOK_WEIGHT:       return "weight";
		case TOK_COUNT:        return "count";
		case TOK_DISTINCT:     return "distinct";
		case TOK_CONST_LIST:   return "const_list";
		case TOK_ATTR_SINT:    return "attr_sint";
		case TOK_MAP_ARG:      return "map_arg";
		case TOK_FOR:          return "for";
		case TOK_ITERATOR:     return "iterator";
		case TOK_IS:           return "is";
		case TOK_NULL:         return "null";
		case TOK_IS_NULL:      return "is_null";
		case TOK_IS_NOT_NULL:  return "is_not_null";
		case TOK_OR:           return "or";
		case TOK_AND:          return "and";
		case TOK_NE:           return "!=";
		case TOK_EQ:           return "=";
		case TOK_GTE:          return ">=";
		case TOK_LTE:          return "<=";
		case TOK_MOD:          return "mod";
		case TOK_DIV:          return "div";
		case TOK_NOT:          return "not";
		case TOK_NEG:          return "neg";
		default: return "Unknown_need_to_fix";
	}
}

// debug dump
static void Dump ( int iNode, const VecTraits_T<ExprNode_t>& dNodes, StringBuilder_c& tOut )
{
	if ( iNode<0 )
		return;

	ExprNode_t & tNode = dNodes[iNode];
	switch ( tNode.m_iToken )
	{
	case TOK_CONST_INT:
		tOut << tNode.m_iConst;
		break;

	case TOK_CONST_FLOAT:
		tOut << tNode.m_fConst;
		break;

	case TOK_ATTR_INT:
	case TOK_ATTR_SINT:
		tOut << "row[" << tNode.m_tLocator.m_iBitOffset / 32 << "]";
		break;

	default:
		tOut << "(";
		Dump ( tNode.m_iLeft, dNodes, tOut );
		tOut << " ";
		if ( tNode.m_iToken<256 )
			tOut.RawC ( (char) tNode.m_iToken);
		else
			tOut << TokName (tNode.m_iToken,tNode.m_iFunc);
		tOut << " ";
		Dump ( tNode.m_iRight, dNodes, tOut );
		tOut << ")";
		break;
	}
}

void ExprParser_t::Dump ( int iNode )
{
	if ( iNode<0 )
		return;

	StringBuilder_c tOut;
	::Dump ( iNode, m_dNodes, tOut );
	printf ("%s\n", tOut.cstr());
}

static void PrintArrow ( StringBuilder_c & tRes, CSphVector<int>& dPref, int iFrom, int iTo, const char* szSuff )
{
	const char* szColor = ( iTo>iFrom ) ? " color=red": "";
	dPref.Add ( tRes.GetLength () );
	tRes.Sprintf ( "_%d%s->", iFrom, szSuff );
	dPref.Add ( tRes.GetLength () );
	tRes.Sprintf ( "_%d[label=%d%s]\n", iTo, iTo, szColor );
	// 10:l -> 6 [label=6]
}

static void DumpNode2Dot (StringBuilder_c& tRes, CSphVector<int>& dPref, const VecTraits_T<ExprNode_t>& dNodes, int iNode )
{
	if ( iNode<0 )
		return;

	ExprNode_t & tNode = dNodes[iNode];

	if ( tNode.m_iToken<=0 )
		return;

	dPref.Add ( tRes.GetLength () );
	tRes << "_" << iNode; // node num

	switch ( tNode.m_iToken )
	{
	case TOK_CONST_INT:
		tRes.Sprintf ( "[shape=circle label=%d]\n", tNode.m_iConst );
		break;

	case TOK_CONST_FLOAT:
		tRes.Sprintf ( "[shape=circle label=%f]\n", tNode.m_fConst );
		break;

	case TOK_ATTR_INT:
	case TOK_ATTR_SINT:
		tRes.Sprintf ( "[shape=oval label=row_%d]\n", tNode.m_tLocator.m_iBitOffset / 32 );
		break;

	default:
		tRes << "[label=\"";
		if ( tNode.m_iLeft>=0 )
			tRes << "<l>|";
		tRes << TokName ( tNode.m_iToken, tNode.m_iFunc );
		if ( tNode.m_iRight>=0 )
			tRes << "|<r>";
		tRes << "\"]\n";
	break;
	}

	if ( tNode.m_iLeft>=0 )
		PrintArrow ( tRes, dPref, iNode, tNode.m_iLeft, ":l"  );

	if ( tNode.m_iRight>=0 )
		PrintArrow ( tRes, dPref, iNode, tNode.m_iRight, ":r" );
}

static void DumpTree2Dot ( StringBuilder_c& tRes, CSphVector<int>& dPref, const VecTraits_T<ExprNode_t>& dNodes, int iRoot )
{
	// use https://dreampuf.github.io/GraphvizOnline to visualize the graph
	// note, that is NOT recursive, so no stack hit expected.

	ARRAY_CONSTFOREACH ( i, dNodes )
		DumpNode2Dot ( tRes, dPref, dNodes, i );

	// output header (root node and pointer)
	tRes << "root";
	dPref.Add ( tRes.GetLength () );
	tRes << "_[shape=invhouse label=root]\nroot";
	dPref.Add ( tRes.GetLength () );
	tRes << "_->";
	dPref.Add ( tRes.GetLength () );
	tRes << "_" << iRoot << "[label=" << iRoot << "]";
}

using StrWithPlaces_t = std::pair<CSphString, CSphVector<int>>;
static void Render2Dot ( StrWithPlaces_t& tRes, VecTraits_T<ExprNode_t>& dNodes, int iRoot )
{
	auto & dPlaces = tRes.second;
	StringBuilder_c sDot;
	DumpTree2Dot ( sDot, dPlaces, dNodes, iRoot );
	sDot.MoveTo ( tRes.first );
}

using NamedDot = std::pair<CSphString, StrWithPlaces_t>;
static void RenderAndAddWithName ( NamedDot & tOut, VecTraits_T<ExprNode_t> & dNodes, int iRoot, const char* szName )
{
	tOut.first = szName;
	Render2Dot ( tOut.second, dNodes, iRoot );
}

static void PlacePrefix ( StrWithPlaces_t & tRes, const char* sPrefix )
{
	char c = *sPrefix;
	auto * sRes = const_cast<char *> (tRes.first.cstr ());
	tRes.second.Apply ( [sRes, c] ( int i ) { sRes[i] = c; } );
}

static CSphString Dots2String ( CSphVector<NamedDot>& dDots )
{
	static const BYTE uPREFIXES = 8;
	static const char* dPrefixes[uPREFIXES] = {"a", "b", "c", "d", "e", "f", "g", "h"};
	int iPrefix = 0;

	StringBuilder_c tOut;

	tOut << "digraph A {\nedge[fontsize=9]\nnode[shape=record]\ncolor=blue\n";
	int j = 0;

	for ( int i = 1; i<dDots.GetLength (); ++i )
	{
		if ( dDots[i].second.first == dDots[j].second.first )
			continue;
		tOut << "subgraph cluster_" << dPrefixes[iPrefix] << " {\nlabel=\"" << dDots[j].first << "\"\n";
		PlacePrefix ( dDots[j].second, dPrefixes[iPrefix] );
		tOut << dDots[j].second.first << "\n}\n";
		++iPrefix;
		j = i;
	}
	tOut << "subgraph cluster_" << dPrefixes[iPrefix] << " {\nlabel=\"" << dDots[j].first << "\"\n";
	PlacePrefix ( dDots[j].second, dPrefixes[iPrefix] );
	tOut << dDots[j].second.first << "\n}\n";
	tOut << "}";

	CSphString sResult;
	tOut.MoveTo(sResult);
	return sResult;
}

alignas ( 128 ) static const BYTE g_UrlEncodeTable[] = { // 0 if need escape, 1 if not
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, // -.
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, // 0123456789
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // ABCDEFGHIJKLMNO
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, // PQRSTUVWXYZ_
	0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // abcdefghijklmno
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, // pqrstuvwxyz~
};

static CSphString UrlEncode ( const CSphString& sSource )
{
	StringBuilder_c sRes;
	for ( const auto* pC = sSource.cstr (); *pC; ++pC )
	{
		auto c = (BYTE)*pC;
		if ( ( c & 0x80 ) || !g_UrlEncodeTable[c] )
			sRes.Sprintf("%%%02x",c);
		else
			sRes.RawC((char)c);
	}

	CSphString sResult;
	sRes.MoveTo (sResult);
	return sResult;
}

/// optimize subtree
void ExprParser_t::Optimize ( int iNode )
{
	auto eProfile = session::GetProfile();
	if ( eProfile==Profile_e::DOTEXPR || eProfile==Profile_e::DOTEXPRURL )
	{
		// fixme! m.b. iteratively repeat while something changes?
		CSphVector<NamedDot> dDots;
		RenderAndAddWithName ( dDots.Add (), m_dNodes, m_iParsed, "Raw (non-optimized)" );
		CanonizePass ( iNode );
		RenderAndAddWithName ( dDots.Add (), m_dNodes, m_iParsed, "CanonizePass" );
		ConstantFoldPass ( iNode );
		RenderAndAddWithName ( dDots.Add (), m_dNodes, m_iParsed, "ConstantFoldPass" );
		VariousOptimizationsPass ( iNode );
		RenderAndAddWithName ( dDots.Add (), m_dNodes, m_iParsed, "VariousOptimizationsPass" );
		MultiNEPass ( iNode );
		RenderAndAddWithName ( dDots.Add (), m_dNodes, m_iParsed, "MultiNEPass" );
		auto sDot = Dots2String ( dDots );
		dDots.Reset();

		StringBuilder_c tOut;
		tOut << "Expr was: " << m_sExpr << "\n";
		if ( eProfile==Profile_e::DOTEXPR )
			tOut << sDot;
		else
			tOut << "https://dreampuf.github.io/GraphvizOnline/#" << UrlEncode(sDot);
		printf ( "%s\n", tOut.cstr () );
		fflush ( stdout );
		return;
	}
	CanonizePass ( iNode );
	ConstantFoldPass ( iNode );
	VariousOptimizationsPass ( iNode );
	MultiNEPass ( iNode );
}

/// fold arglist into array
/// moves also ownership (so, 1-st param owned by dArgs on exit)
void MoveToArgList ( ISphExpr * pLeft, VecRefPtrs_t<ISphExpr*> &dArgs )
{
	if ( !pLeft || !pLeft->IsArglist ())
	{
		dArgs.Add ( pLeft );
		return;
	}

	auto * pArgs = (Expr_Arglist_c *)pLeft;
	if ( dArgs.IsEmpty () )
		dArgs.SwapData ( pArgs->m_dArgs );
	else {
		dArgs.Append ( pArgs->m_dArgs );
		pArgs->m_dArgs.Reset();
	}
	SafeRelease ( pArgs );
}


using UdfInt_fn = sphinx_int64_t ( * ) ( SPH_UDF_INIT *, SPH_UDF_ARGS *, char * );
using UdfDouble_fn = double ( * ) ( SPH_UDF_INIT *, SPH_UDF_ARGS *, char * );
using UdfCharptr_fn = char * ( * ) ( SPH_UDF_INIT *, SPH_UDF_ARGS *, char * );

class Expr_Udf_c : public ISphExpr
{
public:
	explicit Expr_Udf_c ( UdfCall_t * pCall, QueryProfile_c * pProfiler )
		: m_pCall ( pCall )
		, m_pProfiler ( pProfiler )
	{
		SPH_UDF_ARGS & tArgs = m_pCall->m_tArgs;

		assert ( tArgs.arg_values==nullptr );
		tArgs.arg_values = new char * [ tArgs.arg_count ];
		tArgs.str_lengths = new int [ tArgs.arg_count ];

		m_dArgvals.Resize ( tArgs.arg_count );
		ARRAY_FOREACH ( i, m_dArgvals )
			tArgs.arg_values[i] = (char*) &m_dArgvals[i];
	}

	~Expr_Udf_c () override
	{
		if ( m_pCall->m_pUdf->m_fnDeinit )
			m_pCall->m_pUdf->m_fnDeinit ( &m_pCall->m_tInit );
		SafeDelete ( m_pCall );
	}

	void FillArgs ( const CSphMatch & tMatch ) const
	{
		uint64_t uPacked = 0;
		ESphJsonType eJson = JSON_NULL;
		uint64_t uOff = 0;
		CSphVector<BYTE> dTmp;

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
			case SPH_UDF_TYPE_UINT32SET:
			case SPH_UDF_TYPE_INT64SET:
			{
				auto dMva = m_dArgs[i]->MvaEval ( tMatch );
				tArgs.arg_values[i] = (char*)const_cast<BYTE*>(dMva.first);
				tArgs.str_lengths[i] = dMva.second / (( tArgs.arg_types[i]==SPH_UDF_TYPE_UINT32SET ) ? sizeof(DWORD) : sizeof(int64_t));
				break;
			}

			case SPH_UDF_TYPE_FACTORS:
				tArgs.arg_values[i] = (char *)const_cast<BYTE*>( m_dArgs[i]->FactorEval(tMatch) );
				break;

			case SPH_UDF_TYPE_JSON:
				uPacked = m_dArgs[i]->Int64Eval ( tMatch );

				eJson = sphJsonUnpackType ( uPacked );
				uOff = sphJsonUnpackOffset ( uPacked );

				if ( !uOff || eJson==JSON_NULL )
				{
					tArgs.arg_values[i] = nullptr;
					tArgs.str_lengths[i] = 0;
				} else
				{
					JsonEscapedBuilder sTmp;
					sphJsonFieldFormat ( sTmp, m_pBlobPool+uOff, eJson, false );
					tArgs.str_lengths[i] = sTmp.GetLength();
					tArgs.arg_values[i] = (char*) sTmp.Leak();
				}
			break;

			default:						assert ( 0 ); m_dArgvals[i] = 0; break;
			}
		}
	}

	void FreeArgs() const
	{
		assert ( !m_pCall->m_dArgs2Free.GetLength() || ( m_pCall->m_dArgs2Free.GetLength() && m_pCall->m_tArgs.arg_count )  );
		for ( int iAttr : m_pCall->m_dArgs2Free )
			SafeDeleteArray ( m_pCall->m_tArgs.arg_values[iAttr] );
	}

	void AdoptArgs ( ISphExpr * pArglist )
	{
		MoveToArgList ( pArglist, m_dArgs );
	}

	CSphVector<ISphExpr*> & GetArgs()
	{
		return m_dArgs;
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		for ( auto i : m_dArgs )
			i->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		switch ( eCmd )
		{
		case SPH_EXPR_GET_UDF:
			*((bool*)pArg) = true;
			return;

		case SPH_EXPR_GET_STATEFUL_UDF:
			if ( m_pCall && m_pCall->m_tInit.func_data )
				*((bool*)pArg) = true;
			return;

		case SPH_EXPR_SET_BLOB_POOL:
			m_pBlobPool = (const BYTE*)pArg;
			break;

		default:
			break;
		}

		for ( auto & pExpr : m_dArgs )
			pExpr->Command ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & bDisable ) override
	{
		bDisable = true;
		return 0;
	}

protected:
	UdfCall_t *						m_pCall {nullptr};
	VecRefPtrs_t<ISphExpr*>			m_dArgs;
	mutable CSphVector<int64_t>		m_dArgvals;
	const BYTE *					m_pBlobPool {nullptr};
	mutable char					m_bError  {0};
	QueryProfile_c *				m_pProfiler {nullptr};

	Expr_Udf_c ( const Expr_Udf_c& rhs )
		: m_pCall ( new UdfCall_t )
		, m_pProfiler ( rhs.m_pProfiler )
	{
		assert ( !rhs.m_pCall->m_tInit.func_data );

		m_pBlobPool = rhs.m_pBlobPool;

		m_pCall->m_pUdf = rhs.m_pCall->m_pUdf;
		m_pCall->m_dArgs2Free = rhs.m_pCall->m_dArgs2Free;
		SPH_UDF_ARGS & tArgs = m_pCall->m_tArgs;
		const SPH_UDF_ARGS & tSrcArgs = rhs.m_pCall->m_tArgs;

		tArgs.arg_count = tSrcArgs.arg_count;

		m_dArgs.Resize ( tArgs.arg_count );
		ARRAY_FOREACH ( i, m_dArgs )
			m_dArgs[i] = SafeClone ( rhs.m_dArgs[i] );

		tArgs.arg_types = new sphinx_udf_argtype [ tArgs.arg_count ];
		memcpy ( tArgs.arg_types, tSrcArgs.arg_types, sizeof ( sphinx_udf_argtype ) * tArgs.arg_count );

		tArgs.arg_values = new char * [tArgs.arg_count];
		tArgs.str_lengths = new int[tArgs.arg_count];

		m_dArgvals.Resize ( tArgs.arg_count );
		ARRAY_FOREACH ( i, m_dArgvals )
			tArgs.arg_values[i] = (char *) &m_dArgvals[i];
	}
};


class Expr_UdfInt_c : public Expr_Udf_c
{
public:
	explicit Expr_UdfInt_c ( UdfCall_t * pCall, QueryProfile_c * pProfiler )
		: Expr_Udf_c ( pCall, pProfiler )
	{
		assert ( IsInt ( pCall->m_pUdf->m_eRetType ) );
	}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const final
	{
		if ( m_bError )
			return 0;

		CSphScopedProfile tProf ( m_pProfiler, SPH_QSTATE_EVAL_UDF );
		FillArgs ( tMatch );
		auto pFn = (UdfInt_fn) m_pCall->m_pUdf->m_fnFunc;
		auto iRes = (int64_t) pFn ( &m_pCall->m_tInit, &m_pCall->m_tArgs, &m_bError );
		FreeArgs();
		return iRes;
	}

	int IntEval ( const CSphMatch & tMatch ) const final { return (int) Int64Eval ( tMatch ); }
	float Eval ( const CSphMatch & tMatch ) const final { return (float) Int64Eval ( tMatch ); }

	ISphExpr * Clone () const final
	{
		return new Expr_UdfInt_c ( *this );
	}

private:
	Expr_UdfInt_c ( const Expr_UdfInt_c& ) = default;
};


class Expr_UdfFloat_c : public Expr_Udf_c
{
public:
	explicit Expr_UdfFloat_c ( UdfCall_t * pCall, QueryProfile_c * pProfiler )
		: Expr_Udf_c ( pCall, pProfiler )
	{
		assert ( pCall->m_pUdf->m_eRetType==SPH_ATTR_FLOAT );
		m_pFn = (UdfDouble_fn)m_pCall->m_pUdf->m_fnFunc;
	}

	float Eval ( const CSphMatch & tMatch ) const final
	{
		if ( m_bError )
			return 0;

		CSphScopedProfile tProf ( m_pProfiler, SPH_QSTATE_EVAL_UDF );
		FillArgs ( tMatch );
		assert ( m_pFn == m_pCall->m_pUdf->m_fnFunc );
		auto fRes = (float)m_pFn ( &m_pCall->m_tInit, &m_pCall->m_tArgs, &m_bError );
		FreeArgs();
		return fRes;
	}

	int IntEval ( const CSphMatch & tMatch ) const final { return (int) Eval ( tMatch ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const final { return (int64_t) Eval ( tMatch ); }

	ISphExpr * Clone () const final
	{
		return new Expr_UdfFloat_c ( *this );
	}

private:
	Expr_UdfFloat_c ( const Expr_UdfFloat_c& ) = default;
	UdfDouble_fn m_pFn; // to avoid dereference on each Eval() call
};


class Expr_UdfStringptr_c : public Expr_Udf_c
{
public:
	explicit Expr_UdfStringptr_c ( UdfCall_t * pCall, QueryProfile_c * pProfiler )
		: Expr_Udf_c ( pCall, pProfiler )
	{
		assert ( pCall->m_pUdf->m_eRetType==SPH_ATTR_STRINGPTR );
	}

	float Eval ( const CSphMatch & ) const final
	{
		assert ( 0 && "internal error: stringptr udf evaluated as float" );
		return 0.0f;
	}

	int IntEval ( const CSphMatch & ) const final
	{
		assert ( 0 && "internal error: stringptr udf evaluated as int" );
		return 0;
	}

	int64_t Int64Eval ( const CSphMatch & ) const final
	{
		assert ( 0 && "internal error: stringptr udf evaluated as bigint" );
		return 0;
	}

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final
	{
		if ( m_bError )
		{
			*ppStr = nullptr;
			return 0;
		}

		CSphScopedProfile tProf ( m_pProfiler, SPH_QSTATE_EVAL_UDF );
		FillArgs ( tMatch );
		auto pFn = (UdfCharptr_fn) m_pCall->m_pUdf->m_fnFunc;
		char * pRes = pFn ( &m_pCall->m_tInit, &m_pCall->m_tArgs, &m_bError ); // owned now!
		*ppStr = (const BYTE*) pRes;
		int iLen = ( pRes ?(int) strlen(pRes) : 0 );
		FreeArgs();
		return iLen;
	}

	bool IsDataPtrAttr() const final
	{
		return true;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_UdfStringptr_c ( *this );
	}

private:
	Expr_UdfStringptr_c ( const Expr_UdfStringptr_c& ) = default;
};


ISphExpr * ExprParser_t::CreateUdfNode ( int iCall, ISphExpr * pLeft )
{
	if ( !CheckStoredArg(pLeft) )
		return nullptr;

	Expr_Udf_c * pRes = nullptr;
	switch ( m_dUdfCalls[iCall]->m_pUdf->m_eRetType )
	{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_BIGINT:
			pRes = new Expr_UdfInt_c ( m_dUdfCalls[iCall], m_pProfiler );
			break;
		case SPH_ATTR_FLOAT:
			pRes = new Expr_UdfFloat_c ( m_dUdfCalls[iCall], m_pProfiler );
			break;
		case SPH_ATTR_STRINGPTR:
			pRes = new Expr_UdfStringptr_c ( m_dUdfCalls[iCall], m_pProfiler );
			break;
		default:
			m_sCreateError.SetSprintf ( "internal error: unhandled type %d in CreateUdfNode()", m_dUdfCalls[iCall]->m_pUdf->m_eRetType );
			break;
	}

	if ( pRes )
	{
		SafeAddRef ( pLeft );
		if ( pLeft )
			pRes->AdoptArgs ( pLeft );

		m_dUdfCalls[iCall] = nullptr; // evaluator owns it now
	}

	return pRes;
}


CSphString ExprParser_t::GetNameByLocator ( int iNode ) const
{
	return GetNameByLocator ( m_dNodes[iNode] );
}


CSphString ExprParser_t::GetNameByLocator ( const ExprNode_t & tNode ) const
{
	int iLocator = tNode.m_iLocator;
	if ( iLocator==-1 )
		return "";

	return m_pSchema->GetAttr(iLocator).m_sName;
}


ISphExpr * ExprParser_t::CreateExistNode ( const ExprNode_t & tNode )
{
	assert ( m_dNodes[tNode.m_iLeft].m_iToken==',' );
	int iAttrName = m_dNodes[tNode.m_iLeft].m_iLeft;
	int iAttrDefault = m_dNodes[tNode.m_iLeft].m_iRight;
	assert ( iAttrName>=0 && iAttrName<m_dNodes.GetLength()
		&& iAttrDefault>=0 && iAttrDefault<m_dNodes.GetLength() );

	auto iNameStart = GetConstStrOffset ( m_dNodes[iAttrName] );
	auto iNameLen = GetConstStrLength ( m_dNodes[iAttrName] );
	// skip head and tail non attribute name symbols
	const char* sExpr = m_sExpr.first;
	while ( sExpr[iNameStart]!='\0' && ( sExpr[iNameStart]=='\'' || sExpr[iNameStart]==' ' ) && iNameLen )
	{
		iNameStart++;
		--iNameLen;
	}
	while ( sExpr[iNameStart+iNameLen-1]!='\0'
		&& ( sExpr[iNameStart+iNameLen-1]=='\'' || sExpr[iNameStart+iNameLen-1]==' ' )
		&& iNameLen )
	{
		--iNameLen;
	}

	if ( iNameLen<=0 )
	{
		m_sCreateError.SetSprintf ( "first EXIST() argument must be valid string" );
		return nullptr;
	}

	assert ( iNameStart>=0 && iNameLen>0 && iNameStart+iNameLen<=m_sExpr.second );

	CSphString sAttr ( sExpr+iNameStart, iNameLen );
	sphColumnToLowercase ( const_cast<char *>( sAttr.cstr() ) );
	int iLoc = m_pSchema->GetAttrIndex ( sAttr.cstr() );

	if ( iLoc>=0 )
	{
		const CSphColumnInfo & tCol = m_pSchema->GetAttr ( iLoc );
		if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET || tCol.m_eAttrType==SPH_ATTR_STRING )
		{
			m_sCreateError = "MVA and STRING in EXIST() prohibited";
			return nullptr;
		}

		bool bColumnar = tCol.IsColumnar();
		bool bStored = tCol.m_uAttrFlags & CSphColumnInfo::ATTR_STORED;
		const CSphAttrLocator & tLoc = tCol.m_tLocator;
		if ( tNode.m_eRetType==SPH_ATTR_FLOAT )
		{
			if ( bColumnar )
				return CreateExpr_GetColumnarFloat ( tCol.m_sName, bStored );
			else
				return new Expr_GetFloat_c ( tLoc, tCol.m_sName );
		}
		else
		{
			if ( bColumnar )
				return CreateExpr_GetColumnarInt ( tCol.m_sName, bStored );
			else
				return new Expr_GetInt_c ( tLoc, tCol.m_sName );
		}
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

class Expr_Contains_c : public ISphExpr
{
protected:
	CSphRefcountedPtr<ISphExpr>	m_pLat;
	CSphRefcountedPtr<ISphExpr>	m_pLon;

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
			if ( t1==t2 && !( ax==bx && ay==by ) )
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
	{
		SafeAddRef ( pLat );
		SafeAddRef ( pLon );
	}

	Expr_Contains_c() = default;

	float Eval ( const CSphMatch & tMatch ) const override
	{
		return (float)IntEval ( tMatch );
	}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const override
	{
		return IntEval ( tMatch );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		m_pLat->FixupLocator ( pOldSchema, pNewSchema );
		m_pLon->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		m_pLat->Command ( eCmd, pArg );
		m_pLon->Command ( eCmd, pArg );
	}

protected:
	uint64_t CalcHash ( const char * szTag, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
	{
		EXPR_CLASS_NAME_NOCHECK(szTag);
		CALC_CHILD_HASH(m_pLat);
		CALC_CHILD_HASH(m_pLon);
		return CALC_DEP_HASHES();
	}

	// FIXME! implement SetBlobPool?

	Expr_Contains_c ( const Expr_Contains_c& rhs )
		: m_pLat ( SafeClone (rhs.m_pLat) )
		, m_pLon ( SafeClone (rhs.m_pLon) )
	{}
};


class Expr_ContainsConstvec_c : public Expr_Contains_c, public Poly2dBBox_t
{
	using BASE = Expr_Contains_c;

public:
	Expr_ContainsConstvec_c ( ISphExpr * pLat, ISphExpr * pLon, const CSphString & sAttrLat, const CSphString & sAttrLon, const CSphVector<int> & dNodes, const ExprNode_t * pNodes, bool bGeoTesselate )
		: Expr_Contains_c ( pLat, pLon )
	{
		m_sAttrLat = sAttrLat;
		m_sAttrLon = sAttrLon;

		// copy polygon data
		assert ( dNodes.GetLength()>=6 );
		m_dPoly.Resize ( dNodes.GetLength() );

		ARRAY_FOREACH ( i, dNodes )
			m_dPoly[i] = FloatVal ( &pNodes[dNodes[i]] );

		// handle (huge) geosphere polygons
		if ( bGeoTesselate )
			GeoTesselate ( m_dPoly );

		m_iNumPoints = m_dPoly.GetLength()/2;

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

	int IntEval ( const CSphMatch & tMatch ) const final
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

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_ContainsConstvec_c");
		CALC_POD_HASHES(m_dPoly);
		return CALC_PARENT_HASH();
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		BASE::Command ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_GET_POLY2D_BBOX )
		{
			auto pBBox = (std::pair<Poly2dBBox_t *, bool>*)pArg;
			assert(pBBox);
			pBBox->first = this;
			pBBox->second = true;
		}
	}

	ISphExpr * Clone() const final { return new Expr_ContainsConstvec_c ( *this ); }

protected:
	CSphVector<float> m_dPoly;

private:
	Expr_ContainsConstvec_c ( const Expr_ContainsConstvec_c& ) = default;
};


class Expr_ContainsExprvec_c : public Expr_Contains_c
{
protected:
	mutable CSphVector<float> m_dPoly;
	VecRefPtrs_t<ISphExpr*> m_dExpr;

public:
	Expr_ContainsExprvec_c ( ISphExpr * pLat, ISphExpr * pLon, CSphVector<ISphExpr*> & dExprs )
		: Expr_Contains_c ( pLat, pLon )
	{
		m_dExpr.SwapData ( dExprs );
		m_dPoly.Resize ( m_dExpr.GetLength() );
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		ARRAY_FOREACH ( i, m_dExpr )
			m_dPoly[i] = m_dExpr[i]->Eval ( tMatch );
		return Contains ( m_pLat->Eval(tMatch), m_pLon->Eval(tMatch), m_dPoly.GetLength(), m_dPoly.Begin() );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final
	{
		Expr_Contains_c::FixupLocator ( pOldSchema, pNewSchema );
		for ( auto i : m_dExpr )
			i->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_Contains_c::Command ( eCmd, pArg );
		ARRAY_FOREACH ( i, m_dExpr )
			m_dExpr[i]->Command ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_ContainsExprvec_c");
		CALC_CHILD_HASHES(m_dExpr);
		return CALC_PARENT_HASH();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_ContainsExprvec_c ( *this );
	}

private:
	Expr_ContainsExprvec_c ( const Expr_ContainsExprvec_c& rhs )
		: Expr_Contains_c ( rhs )
		, m_dPoly ( rhs.m_dPoly )
	{
		m_dExpr.Resize ( rhs.m_dExpr.GetLength () );
		ARRAY_FOREACH ( i, m_dExpr )
			m_dExpr[i] = SafeClone (rhs.m_dExpr[i]);
	}
};


class Expr_ContainsStrattr_c : public Expr_Contains_c
{
protected:
	CSphRefcountedPtr<ISphExpr> m_pStr;
	bool m_bGeo;

public:
	Expr_ContainsStrattr_c ( ISphExpr * pLat, ISphExpr * pLon, ISphExpr * pStr, bool bGeo )
		: Expr_Contains_c ( pLat, pLon )
		, m_pStr ( pStr )
		, m_bGeo ( bGeo )
	{
		SafeAddRef ( pStr );
	}

	static void ParsePoly ( const char * p, int iLen, CSphVector<float> & dPoly )
	{
		const char * pBegin = p;
		const char * pMax = sphFindLastNumeric ( p, iLen );
		while ( p<pMax )
		{
			if ( isdigit(p[0]) || ( p+1<pMax && p[0]=='-' && isdigit(p[1]) ) )
			{
				char * pValue = const_cast<char*>(p);
				dPoly.Add ( (float)strtod ( p, &pValue ) );
				p = pValue;
			}
			else
				p++;
		}

		// edge case - last numeric touches the end
		iLen -= pMax - pBegin;
		if ( iLen )
			dPoly.Add ( (float)strtod ( CSphString(pMax, iLen).cstr (), nullptr ) );
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		const char * pStr;
		assert ( !m_pStr->IsDataPtrAttr() ); // aware of mem leaks caused by some StringEval implementations
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

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final
	{
		Expr_Contains_c::FixupLocator ( pOldSchema, pNewSchema );
		m_pStr->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_Contains_c::Command ( eCmd, pArg );
		m_pStr->Command ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_ContainsStrattr_c");
		CALC_CHILD_HASH(m_pStr);
		return CALC_PARENT_HASH();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_ContainsStrattr_c ( *this );
	}

private:
	Expr_ContainsStrattr_c ( const Expr_ContainsStrattr_c& rhs )
		: Expr_Contains_c ( rhs )
		, m_pStr ( SafeClone (rhs.m_pStr) )
		, m_bGeo ( rhs.m_bGeo )
	{}
};


ISphExpr * ExprParser_t::CreateContainsNode ( const ExprNode_t & tNode )
{
	// get and check them args
	const ExprNode_t & tArglist = m_dNodes [ tNode.m_iLeft ];
	const int iPoly = m_dNodes [ tArglist.m_iLeft ].m_iLeft;
	const int iLat = m_dNodes [ tArglist.m_iLeft ].m_iRight;
	const int iLon = tArglist.m_iRight;
	assert ( IsNumeric ( m_dNodes[iLat].m_eRetType ) || IsJson ( m_dNodes[iLat].m_eRetType )  );
	assert ( IsNumeric ( m_dNodes[iLon].m_eRetType ) || IsJson ( m_dNodes[iLon].m_eRetType ) );
	assert ( m_dNodes[iPoly].m_eRetType==SPH_ATTR_POLY2D );

	// create evaluator
	// gotta handle an optimized constant poly case
	CSphVector<int> dPolyArgs = GatherArgNodes ( m_dNodes[iPoly].m_iLeft );

	CSphRefcountedPtr<ISphExpr> pLat { ConvertExprJson ( CreateTree ( iLat ) ) };
	CSphRefcountedPtr<ISphExpr> pLon { ConvertExprJson ( CreateTree ( iLon ) ) };

	bool bGeoTesselate = ( m_dNodes[iPoly].m_iToken==TOK_FUNC && m_dNodes[iPoly].m_iFunc==FUNC_GEOPOLY2D );

	if ( dPolyArgs.GetLength()==1 && ( m_dNodes[dPolyArgs[0]].m_iToken==TOK_ATTR_STRING || m_dNodes[dPolyArgs[0]].m_iToken==TOK_COLUMNAR_STRING || m_dNodes[dPolyArgs[0]].m_iToken==TOK_ATTR_JSON ) )
	{
		CSphRefcountedPtr<ISphExpr> dPolyArgs0 { ConvertExprJson ( CreateTree ( dPolyArgs[0] ) ) };
		return new Expr_ContainsStrattr_c ( pLat, pLon, dPolyArgs0, bGeoTesselate );
	}

	if ( dPolyArgs.all_of ( [&] ( int iArg ) { return IsConst ( &m_dNodes[iArg] ); } ) )
		return new Expr_ContainsConstvec_c ( pLat, pLon, GetNameByLocator(iLat), GetNameByLocator(iLon), dPolyArgs, m_dNodes.Begin(), bGeoTesselate ); // POLY2D(numeric-consts)
	else
	{
		// POLY2D(generic-exprs)
		VecRefPtrs_t<ISphExpr*> dExprs;
		dExprs.Resize ( dPolyArgs.GetLength() );
		ARRAY_FOREACH ( i, dExprs )
			dExprs[i] = CreateTree ( dPolyArgs[i] );

		ConvertArgsJson ( dExprs );

		// will adopt dExprs and utilize them on d-tr
		return new Expr_ContainsExprvec_c ( pLat, pLon, dExprs );
	}
}

class Expr_Remap_c : public ISphExpr
{
	struct CondValPair_t
	{
		int64_t m_iCond;
		union
		{
			int64_t m_iVal;
			float m_fVal;
		};

		explicit CondValPair_t ( int64_t iCond=0 ) : m_iCond ( iCond ), m_iVal ( 0 ) {}
		bool operator< ( const CondValPair_t & rhs ) const { return m_iCond<rhs.m_iCond; }
		bool operator== ( const CondValPair_t & rhs ) const { return m_iCond==rhs.m_iCond; }
	};

	CSphRefcountedPtr<ISphExpr> m_pCond;
	CSphRefcountedPtr<ISphExpr> m_pVal;
	CSphVector<CondValPair_t> m_dPairs;

public:
	Expr_Remap_c ( ISphExpr * pCondExpr, ISphExpr * pValExpr, const CSphVector<int64_t> & dConds, const ConstList_c & tVals )
		: m_pCond ( pCondExpr )
		, m_pVal ( pValExpr )
		, m_dPairs ( dConds.GetLength() )
	{
		assert ( pCondExpr && pValExpr );
		assert ( dConds.GetLength() );
		assert ( dConds.GetLength()==tVals.m_dInts.GetLength() ||
				dConds.GetLength()==tVals.m_dFloats.GetLength() );

		SafeAddRef ( pCondExpr );
		SafeAddRef ( pValExpr );

		if ( tVals.m_dInts.GetLength() )
			ARRAY_FOREACH ( i, m_dPairs )
			{
				m_dPairs[i].m_iCond = dConds[i];
				m_dPairs[i].m_iVal = tVals.m_dInts[i];
			}
		else
			ARRAY_FOREACH ( i, m_dPairs )
			{
				m_dPairs[i].m_iCond = dConds[i];
				m_dPairs[i].m_fVal = tVals.m_dFloats[i];
			}

		m_dPairs.Uniq();
	}

	float Eval ( const CSphMatch & tMatch ) const final
	{
		const CondValPair_t * p = m_dPairs.BinarySearch ( CondValPair_t ( m_pCond->Int64Eval ( tMatch ) ) );
		if ( p )
			return p->m_fVal;
		return m_pVal->Eval ( tMatch );
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		return (int)Int64Eval ( tMatch );
	}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const final
	{
		const CondValPair_t * p = m_dPairs.BinarySearch ( CondValPair_t ( m_pCond->Int64Eval ( tMatch ) ) );
		if ( p )
			return p->m_iVal;
		return m_pVal->Int64Eval ( tMatch );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final
	{
		m_pCond->FixupLocator ( pOldSchema, pNewSchema );
		m_pVal->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		m_pCond->Command ( eCmd, pArg );
		m_pVal->Command ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_Remap_c");
		CALC_POD_HASHES(m_dPairs);
		CALC_CHILD_HASH(m_pCond);
		CALC_CHILD_HASH(m_pVal);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_Remap_c ( *this );
	}

private:
	Expr_Remap_c ( const Expr_Remap_c& rhs )
		: m_pCond ( SafeClone (rhs.m_pCond) )
		, m_pVal ( SafeClone (rhs.m_pVal) )
		, m_dPairs ( rhs.m_dPairs )
	{}
};

//////////////////////////////////////////////////////////////////////////

class Expr_GetQuery_c final : public Expr_StrNoLocator_c
{
public:
	Expr_GetQuery_c() = default;

	int StringEval ( const CSphMatch &, const BYTE ** ppStr ) const final
	{
		assert ( ppStr );
		CSphString sVal = m_sQuery;
		int iLen = sVal.Length();
		*ppStr = (const BYTE *)sVal.Leak();
		return iLen;
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		if ( eCmd==SPH_EXPR_SET_QUERY )
			m_sQuery = (const char*)pArg;
	}

	bool IsDataPtrAttr() const final { return true; }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GetQuery_c");
		CALC_STR_HASH(m_sQuery, m_sQuery.Length());
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GetQuery_c ( *this );
	}

private:
	Expr_GetQuery_c ( const Expr_GetQuery_c& rhs )
		: m_sQuery ( rhs.m_sQuery )
	{}
	CSphString m_sQuery;
};


//////////////////////////////////////////////////////////////////////////

class Expr_LastInsertID_c : public Expr_StrNoLocator_c
{
public:

	int StringEval ( const CSphMatch &, const BYTE ** ppStr ) const final
	{
		assert ( ppStr );
		if ( m_sIds.IsEmpty() )
		{
			*ppStr = nullptr;
			return 0;
		}

		CSphString sVal = m_sIds;
		int iLen = sVal.Length();
		*ppStr = (const BYTE *)sVal.Leak();
		return iLen;
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		if ( eCmd==SPH_EXPR_SET_EXTRA_DATA )
		{
			static_cast<ISphExtra*>(pArg)->ExtraData ( EXTRA_GET_LAST_INSERT_ID, (void **)&m_sIds );
		}
	}

	bool IsDataPtrAttr() const final { return true; }

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & bDisable ) final
	{
		bDisable = true;
		return 0;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_LastInsertID_c;
	}

private:
	CSphString m_sIds;
};

class Expr_UuidShort_c : public Expr_NoLocator_c
{
public:
	Expr_UuidShort_c () = default;

	float Eval ( const CSphMatch & ) const final { return (float) UidShort(); }
	int IntEval ( const CSphMatch & ) const final { return (int)UidShort(); }
	int64_t Int64Eval ( const CSphMatch & ) const final { return UidShort(); }
	bool IsConst () const final { return true; }
	
	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_UuidShort_c");
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_UuidShort_c ( *this );
	}

private:
	Expr_UuidShort_c ( const Expr_UuidShort_c & rhs ) {};
};

//////////////////////////////////////////////////////////////////////////

ISphExpr * ConvertExprJson ( ISphExpr * pExpr )
{
	if ( !pExpr )
		return nullptr;

	bool bConverted = false;
	bool bJson = pExpr->IsJson ( bConverted );
	if ( bJson && !bConverted )
	{
		ISphExpr * pConv = new Expr_JsonFieldConv_c ( pExpr );
		pExpr->Release();
		return pConv;
	} else
	{
		return pExpr;
	}
}

void ConvertArgsJson ( VecRefPtrs_t<ISphExpr*> & dArgs )
{
	ARRAY_FOREACH ( i, dArgs )
	{
		dArgs[i] = ConvertExprJson ( dArgs[i] );
	}
}


ISphExpr * ExprParser_t::CreateFieldNode ( int iField )
{
	m_eEvalStage = SPH_EVAL_POSTLIMIT;
	m_uStoredField = CSphColumnInfo::FIELD_STORED;
	m_bNeedDocIds = true;

	const CSphColumnInfo & tField = m_pSchema->GetField(iField);
	if ( !(tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED) )
	{
		m_sCreateError.SetSprintf ( "field '%s' is not stored, see 'stored_fields' option", tField.m_sName.cstr() );
		return nullptr;
	}

	return CreateExpr_GetStoredField ( tField.m_sName );
}


ISphExpr * ExprParser_t::CreateColumnarIntNode ( int iAttr, ESphAttr eAttrType )
{
	const CSphColumnInfo & tAttr = m_pSchema->GetAttr(iAttr);
	return CreateExpr_GetColumnarInt ( tAttr.m_sName, tAttr.m_uAttrFlags & CSphColumnInfo::ATTR_STORED );
}


ISphExpr * ExprParser_t::CreateColumnarFloatNode ( int iAttr )
{
	const CSphColumnInfo & tAttr = m_pSchema->GetAttr(iAttr);
	return CreateExpr_GetColumnarFloat ( tAttr.m_sName, tAttr.m_uAttrFlags & CSphColumnInfo::ATTR_STORED );
}


ISphExpr * ExprParser_t::CreateColumnarStringNode ( int iAttr )
{
	const CSphColumnInfo & tAttr = m_pSchema->GetAttr(iAttr);
	return CreateExpr_GetColumnarString ( tAttr.m_sName, tAttr.m_uAttrFlags & CSphColumnInfo::ATTR_STORED );
}


ISphExpr * ExprParser_t::CreateColumnarMvaNode ( int iAttr )
{
	const CSphColumnInfo & tAttr = m_pSchema->GetAttr(iAttr);
	return CreateExpr_GetColumnarMva ( tAttr.m_sName, tAttr.m_uAttrFlags & CSphColumnInfo::ATTR_STORED );
}

//////////////////////////////////////////////////////////////////////////

struct LevenshteinOptions_t
{
	bool	m_bNormalize = false;
	int		m_iLengthDelta = 0;
};

static LevenshteinOptions_t GetOptions ( const CSphNamedVariant * pValues, int iCount )
{
	LevenshteinOptions_t tOpts;

	for ( int i=0; i<iCount; i++, pValues++ )
	{
		if ( pValues->m_sKey=="normalize" )
			tOpts.m_bNormalize = ( !!pValues->m_iValue );
		else if ( pValues->m_sKey=="length_delta" )
			tOpts.m_iLengthDelta = pValues->m_iValue;
	}

	return tOpts;
}

template<bool PATTERN_STRING>
class Expr_Levenshtein_c : public Expr_Binary_c
{
public:
	Expr_Levenshtein_c ( ISphExpr * pPattern, ISphExpr * pAttr, const LevenshteinOptions_t tOpts )
		: Expr_Binary_c ( "Expr_Levenshtein_c", pPattern, pAttr )
		, m_tOpts ( tOpts )
	{
		if_const(PATTERN_STRING)
		{
			const BYTE * pBuf = nullptr;
			CSphMatch tTmp;
			m_iPattersLen = pPattern->StringEval ( tTmp, &pBuf );
			m_sPattern.SetBinary ( (const char *)pBuf, m_iPattersLen );
			FreeDataPtr ( *pPattern, pBuf );
		}
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		assert ( !m_tOpts.m_bNormalize );
		return GetDistance ( tMatch ).first;
	}

	float Eval ( const CSphMatch & tMatch ) const final
	{
		assert ( m_tOpts.m_bNormalize );
		auto tDist = GetDistance ( tMatch );

		float fDist = 1.0f;
		if ( tDist.second )
			fDist = (float)tDist.first / tDist.second;

		return fDist;
	}

	ISphExpr * Clone () const final
	{
		return new Expr_Levenshtein_c ( *this );
	}

private:
	LevenshteinOptions_t	m_tOpts;
	CSphString				m_sPattern;
	int						m_iPattersLen = 0;
	mutable CSphVector<int>	m_dTmp;

	Expr_Levenshtein_c ( const Expr_Levenshtein_c& rhs )
		: Expr_Binary_c ( rhs )
		, m_tOpts ( rhs.m_tOpts )
		, m_sPattern ( rhs.m_sPattern )
		, m_iPattersLen ( rhs.m_iPattersLen )
	{}

	std::pair<int, int> GetDistance ( const CSphMatch & tMatch ) const
	{
		const BYTE * sPattern = (const BYTE *)m_sPattern.cstr();
		int iPatternLen = m_iPattersLen;
		if_const(!PATTERN_STRING)
			iPatternLen = m_pFirst->StringEval ( tMatch, &sPattern );

		const BYTE * pStr = nullptr;
		int iLen = m_pSecond->StringEval ( tMatch, &pStr );

		std::pair<int, int> tDist;
		tDist.second = Max ( iPatternLen, iLen );
		tDist.first = tDist.second;
		if ( !m_tOpts.m_iLengthDelta || ( abs ( iPatternLen-iLen )<m_tOpts.m_iLengthDelta ) )
			tDist.first = sphLevenshtein ( (const char *)sPattern, iPatternLen, (const char *)pStr, iLen, m_dTmp );

		FreeDataPtr ( *m_pSecond, pStr );
		if_const(!PATTERN_STRING)
			FreeDataPtr ( *m_pFirst, sPattern );

		return tDist;
	}
};

ISphExpr * ExprParser_t::CreateLevenshteinNode ( ISphExpr * pPattern, ISphExpr * pAttr, ISphExpr * pOpts )
{
	LevenshteinOptions_t tOpts;
	if ( pOpts )
	{
		Expr_MapArg_c * pOptsArg = (Expr_MapArg_c *)pOpts;
		tOpts = GetOptions ( pOptsArg->m_pValues, int ( pOptsArg->m_iCount ) );
	}

	if ( pPattern->IsConst() )
		return new Expr_Levenshtein_c<true> ( pPattern, pAttr, tOpts );
	else
		return new Expr_Levenshtein_c<false> ( pPattern, pAttr, tOpts );
}

//////////////////////////////////////////////////////////////////////////

bool ExprParser_t::CheckStoredArg ( ISphExpr * pExpr )
{
	if ( pExpr && pExpr->UsesDocstore() )
	{
		m_sCreateError.SetSprintf ( "stored fields can't be used in expressions" );
		return false;
	}

	return true;
}

ISphExpr * ExprParser_t::CreateCmp ( const ExprNode_t & tNode, ISphExpr * pLeft, ISphExpr * pRight )
{
	int iOp = tNode.m_iToken;

	// str case
	if ( ( m_dNodes[tNode.m_iLeft].m_eRetType==SPH_ATTR_STRING
			|| m_dNodes[tNode.m_iLeft].m_eRetType==SPH_ATTR_STRINGPTR
			|| m_dNodes[tNode.m_iLeft].m_eRetType==SPH_ATTR_JSON_FIELD
		)
		&& ( m_dNodes[tNode.m_iRight].m_eRetType==SPH_ATTR_STRING
			|| m_dNodes[tNode.m_iRight].m_eRetType==SPH_ATTR_STRINGPTR )
		)
	{
		if ( !CheckStoredArg(pLeft) || !CheckStoredArg(pRight) )
			return nullptr;

		switch ( iOp )
		{
		case '<':
			return new Expr_StrCmp_c ( pLeft, pRight, m_eCollation, false, EStrCmpDir::LT );
		case '>':
			return new Expr_StrCmp_c ( pLeft, pRight, m_eCollation, false, EStrCmpDir::GT );
		case TOK_LTE:
			return new Expr_StrCmp_c ( pLeft, pRight, m_eCollation, true, EStrCmpDir::GT );
		case TOK_GTE:
			return new Expr_StrCmp_c ( pLeft, pRight, m_eCollation, true, EStrCmpDir::LT );
		case TOK_EQ:
			return new Expr_StrCmp_c ( pLeft, pRight, m_eCollation, false, EStrCmpDir::EQ );
		case TOK_NE:
			return new Expr_StrCmp_c ( pLeft, pRight, m_eCollation, true, EStrCmpDir::EQ );

		default:				assert ( 0 && "unhandled token type" ); break;
		}
	}

	// numeric case
#define LOC_SPAWN_POLY(_classname) switch (tNode.m_eArgType) { \
	case SPH_ATTR_INTEGER:	return new _classname##Int_c ( pLeft, pRight ); \
	case SPH_ATTR_BIGINT:	return new _classname##Int64_c ( pLeft, pRight ); \
	default:				return new _classname##Float_c ( pLeft, pRight ); }

	switch ( iOp )
	{
		case '<':				LOC_SPAWN_POLY ( Expr_Lt );
		case '>':				LOC_SPAWN_POLY ( Expr_Gt );
		case TOK_LTE:			LOC_SPAWN_POLY ( Expr_Lte );
		case TOK_GTE:			LOC_SPAWN_POLY ( Expr_Gte );
		case TOK_EQ:			LOC_SPAWN_POLY ( Expr_Eq );
		case TOK_NE:			LOC_SPAWN_POLY ( Expr_Ne );

		default:				assert ( 0 && "unhandled token type" ); break;
	}
#undef LOC_SPAWN_POLY

	return nullptr;
}


bool ExprParser_t::PrepareFuncArgs ( const ExprNode_t & tNode, bool bSkipChildren, CSphRefcountedPtr<ISphExpr> & pLeft, CSphRefcountedPtr<ISphExpr> & pRight, VecRefPtrs_t<ISphExpr*> & dArgs )
{
	// fold arglist to array
	if ( !bSkipChildren )
	{
		SafeAddRef (pLeft);
		MoveToArgList ( pLeft, dArgs );
		if ( pRight )
		{
			pRight->AddRef ();
			MoveToArgList ( pRight, dArgs );
		}
	}

	for ( auto & i : dArgs )
		if ( !CheckStoredArg(i) )
			return false;

	// spawn proper function
	assert ( tNode.m_iFunc>=0 && tNode.m_iFunc<int(sizeof(g_dFuncs)/sizeof(g_dFuncs[0])) );
	assert (
		( bSkipChildren ) || // function will handle its arglist,
		( g_dFuncs[tNode.m_iFunc].m_iArgs>=0 && g_dFuncs[tNode.m_iFunc].m_iArgs==dArgs.GetLength() ) || // arg count matches,
		( g_dFuncs[tNode.m_iFunc].m_iArgs<0 && -g_dFuncs[tNode.m_iFunc].m_iArgs<=dArgs.GetLength() ) ); // or min vararg count reached

	auto eFunc = (Tokh_e)tNode.m_iFunc;
	switch ( eFunc )
	{
	case FUNC_TO_STRING:
	case FUNC_INTERVAL:
	case FUNC_IN:
	case FUNC_LENGTH:
	case FUNC_LEAST:
	case FUNC_GREATEST:
	case FUNC_ALL:
	case FUNC_ANY:
	case FUNC_INDEXOF:
		break; // these have its own JSON converters

		// all others will get JSON auto-converter
	default:
		ConvertArgsJson ( dArgs );
		break;
	}

	return true;
}


ISphExpr * ExprParser_t::CreateFuncExpr ( int iNode, VecRefPtrs_t<ISphExpr*> & dArgs )
{
	const ExprNode_t & tNode = m_dNodes[iNode];
	auto eFunc = (Tokh_e)tNode.m_iFunc;
	switch ( eFunc )
	{
	case FUNC_NOW:		return CreateExprNow(m_iConstNow);

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
	case FUNC_SINT:		return new Expr_Sint_c ( dArgs[0] );
	case FUNC_CRC32:	return new Expr_Crc32_c ( dArgs[0] );
	case FUNC_FIBONACCI:return new Expr_Fibonacci_c ( dArgs[0] );
	case FUNC_KNN_DIST:	return new Expr_GetFloat_c ( m_pSchema->GetAttr ( GetKnnDistAttrName() )->m_tLocator, GetKnnDistAttrName() );

	case FUNC_DAY:			return CreateExprDay ( dArgs[0] );
	case FUNC_WEEK:			return CreateExprWeek ( dArgs[0], dArgs.GetLength()>1 ? dArgs[1] : nullptr );
	case FUNC_MONTH:		return CreateExprMonth ( dArgs[0] );
	case FUNC_YEAR:			return CreateExprYear ( dArgs[0] );
	case FUNC_YEARMONTH:	return CreateExprYearMonth ( dArgs[0] );
	case FUNC_YEARMONTHDAY:	return CreateExprYearMonthDay ( dArgs[0] );
	case FUNC_YEARWEEK:		return CreateExprYearWeek ( dArgs[0] );
	case FUNC_HOUR:			return CreateExprHour ( dArgs[0] );
	case FUNC_MINUTE:		return CreateExprMinute ( dArgs[0] );
	case FUNC_SECOND:		return CreateExprSecond ( dArgs[0] );
	case FUNC_DAYOFWEEK:	return CreateExprDayOfWeek ( dArgs[0] );
	case FUNC_DAYOFYEAR:	return CreateExprDayOfYear ( dArgs[0] );
	case FUNC_QUARTER:		return CreateExprQuarter ( dArgs[0] );

	case FUNC_MIN:		return new Expr_Min_c ( dArgs[0], dArgs[1] );
	case FUNC_MAX:		return new Expr_Max_c ( dArgs[0], dArgs[1] );
	case FUNC_POW:		return new Expr_Pow_c ( dArgs[0], dArgs[1] );
	case FUNC_IDIV:		return new Expr_Idiv_c ( dArgs[0], dArgs[1] );

	case FUNC_IF:		return new Expr_If_c ( dArgs[0], dArgs[1], dArgs[2] );
	case FUNC_MADD:		return new Expr_Madd_c ( dArgs[0], dArgs[1], dArgs[2] );
	case FUNC_MUL3:		return new Expr_Mul3_c ( dArgs[0], dArgs[1], dArgs[2] );
	case FUNC_ATAN2:	return new Expr_Atan2_c ( dArgs[0], dArgs[1] );
	case FUNC_RAND:		return new Expr_Rand_c ( dArgs.GetLength() ? dArgs[0] : nullptr,
		tNode.m_iLeft<0 ? false : IsConst ( &m_dNodes[tNode.m_iLeft] ));

	case FUNC_INTERVAL:	return CreateIntervalNode ( tNode.m_iLeft, dArgs );
	case FUNC_IN:		return CreateInNode ( iNode );
	case FUNC_LENGTH:	return CreateLengthNode ( tNode, dArgs[0] );
	case FUNC_BITDOT:	return CreateBitdotNode ( tNode.m_iLeft, dArgs );
	case FUNC_REMAP:
	{
		CSphRefcountedPtr<ISphExpr> pCond ( CreateTree ( tNode.m_iLeft ) );
		CSphRefcountedPtr<ISphExpr> pVal ( CreateTree ( tNode.m_iRight ) );
		assert ( pCond && pVal );
		// This is a hack. I know how parser fills m_dNodes and thus know where to find constlists.
		const CSphVector<int64_t> & dConds = m_dNodes [ iNode-2 ].m_pConsts->m_dInts;
		const ConstList_c & tVals = *m_dNodes [ iNode-1 ].m_pConsts;
		return new Expr_Remap_c ( pCond, pVal, dConds, tVals );
	}

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
		if ( !CheckStoredArg(dArgs[0]) )
			return nullptr;

		return new Expr_ToString_c ( dArgs[0], m_dNodes [ tNode.m_iLeft ].m_eRetType );
	case FUNC_CONCAT:
		return CreateConcatNode ( tNode.m_iLeft, dArgs );
	case FUNC_RANKFACTORS:
		m_eEvalStage = SPH_EVAL_PRESORT;
		return new Expr_GetRankFactors_c();
	case FUNC_FACTORS:
		return CreatePFNode ( tNode.m_iLeft );
	case FUNC_BM25F:
	{
		m_uPackedFactorFlags |= SPH_FACTOR_ENABLE;

		CSphVector<int> dBM25FArgs = GatherArgNodes ( tNode.m_iLeft );
		const ExprNode_t & tLeft = m_dNodes [ dBM25FArgs[0] ];
		const ExprNode_t & tRight = m_dNodes [ dBM25FArgs[1] ];
		float fK1 = tLeft.m_fConst;
		float fB = tRight.m_fConst;
		fK1 = Max ( fK1, 0.001f );
		fB = Min ( Max ( fB, 0.0f ), 1.0f );

		CSphVector<CSphNamedVariant> * pFieldWeights = nullptr;
		if ( dBM25FArgs.GetLength()>2 )
			pFieldWeights = &m_dNodes [ dBM25FArgs[2] ].m_pMapArg->m_dPairs;

		return new Expr_BM25F_c ( fK1, fB, pFieldWeights );
	}

	case FUNC_QUERY:
		return new Expr_GetQuery_c;

	case FUNC_BIGINT:
	case FUNC_INTEGER:
	case FUNC_DOUBLE:
	case FUNC_UINT:
	case FUNC_UINT64:
		SafeAddRef ( dArgs[0] );
		return dArgs[0];

	case FUNC_LEAST:	return CreateAggregateNode ( tNode, SPH_AGGR_MIN, dArgs[0] );
	case FUNC_GREATEST:	return CreateAggregateNode ( tNode, SPH_AGGR_MAX, dArgs[0] );

	case FUNC_CURTIME:			return CreateExprCurTime ( false, false );
	case FUNC_CURDATE:			return CreateExprCurDate();
	case FUNC_TIME:				return CreateExprTime ( dArgs[0] );
	case FUNC_DATE:				return CreateExprDate ( dArgs[0] );
	case FUNC_DAYNAME:			return CreateExprDayName ( dArgs[0] );
	case FUNC_MONTHNAME:		return CreateExprMonthName ( dArgs[0] );
	case FUNC_UTC_TIME:			return CreateExprCurTime ( true, false );
	case FUNC_UTC_TIMESTAMP:	return CreateExprCurTime ( true, true );
	case FUNC_TIMEDIFF:			return CreateExprTimeDiff ( dArgs[0], dArgs[1] );
	case FUNC_DATEDIFF:			return CreateExprDateDiff ( dArgs[0], dArgs[1] );
	case FUNC_DATEADD:			return CreateExprDateAdd ( iNode, true );
	case FUNC_DATESUB:			return CreateExprDateAdd ( iNode, false );

	case FUNC_ALL:
	case FUNC_ANY:
	case FUNC_INDEXOF:
		return CreateForInNode ( iNode );

	case FUNC_MIN_TOP_WEIGHT:
		m_eEvalStage = SPH_EVAL_PRESORT;
		return new Expr_MinTopWeight_c();

	case FUNC_MIN_TOP_SORTVAL:
		m_eEvalStage = SPH_EVAL_PRESORT;
		return new Expr_MinTopSortval_c();

	case FUNC_REGEX:
		return CreateRegexNode ( dArgs[0], dArgs[1] );

	case FUNC_SUBSTRING_INDEX:
		if ( !CheckStoredArg(dArgs[0]) || !CheckStoredArg(dArgs[1]) )
			return nullptr;

		return new Expr_SubstringIndex_c ( dArgs[0], dArgs[1], dArgs[2] );

	case FUNC_UPPER:
	{
		if ( IsGlobalLocaleSet() && GlobalLocale()==std::locale::classic() )
			return new ExprCaseTrival_c<true> ( dArgs[0] );
		else
			return new ExprCaseComplex_c<true> ( dArgs[0] );
	}
	case FUNC_LOWER:
	{
		if ( IsGlobalLocaleSet() && GlobalLocale()==std::locale::classic() )
			return new ExprCaseTrival_c<false> ( dArgs[0] );
		else
			return new ExprCaseComplex_c<false> ( dArgs[0] );
	}

	case FUNC_LAST_INSERT_ID: return new Expr_LastInsertID_c();
	case FUNC_CURRENT_USER:
	case FUNC_USER:
	{
		auto sUser = CurrentUser();
		return new Expr_GetStrConst_c ( sUser.first, sUser.second, false );
	}
	case FUNC_CONNECTION_ID: return new Expr_GetIntConst_c ( ConnID() );
	case FUNC_LEVENSHTEIN: return CreateLevenshteinNode ( dArgs[0], dArgs[1], ( dArgs.GetLength()>2 ? dArgs[2] : nullptr ) );

	case FUNC_DATE_FORMAT: return CreateExprDateFormat ( dArgs[0], dArgs[1] );
	case FUNC_DATABASE: return new Expr_GetStrConst_c ( FROMS ( "Manticore" ), false ) ;
	case FUNC_VERSION: return new Expr_GetStrConst_c ( FromStr ( sphinxexpr::MySQLVersion() ), false );
	
	case FUNC_RANGE:
	case FUNC_DATE_RANGE:
	{
		CSphRefcountedPtr<ISphExpr> pVal ( nullptr );
		CSphVector<VecTraits_T < CSphNamedVariant > > dSrcRanges;
		GatherArgFN ( tNode.m_iLeft, [&] ( int i )
		{
			if ( m_dNodes[i].m_eRetType==SPH_ATTR_MAPARG )
				dSrcRanges.Add ( m_dNodes[i].m_pMapArg->m_dPairs );
			else
				pVal = CreateTree ( i );
		});
		AggrRangeSetting_t tRanges;
		if ( !ParseAggrRange ( dSrcRanges, ( eFunc==FUNC_DATE_RANGE ), m_iConstNow, tRanges, m_sCreateError ) )
			return nullptr;
		return CreateExprRange ( pVal, tRanges );
	}

	case FUNC_HISTOGRAM:
	case FUNC_DATE_HISTOGRAM:
	{
		CSphRefcountedPtr<ISphExpr> pVal;
		VecTraits_T < CSphNamedVariant > dSrcOpt;
		GatherArgFN ( tNode.m_iLeft, [&] ( int i )
		{
			if ( m_dNodes[i].m_eRetType==SPH_ATTR_MAPARG )
				dSrcOpt = m_dNodes[i].m_pMapArg->m_dPairs;
			else
				pVal = CreateTree ( i );
		});
		if ( eFunc==FUNC_HISTOGRAM )
		{
			AggrHistSetting_t tHist;
			if ( !ParseAggrHistogram ( dSrcOpt, tHist, m_sCreateError ) )
				return nullptr;
			return CreateExprHistogram ( pVal, tHist );
		} else
		{
			AggrDateHistSetting_t tHist;
			if ( !ParseAggrDateHistogram ( dSrcOpt, tHist, m_sCreateError ) )
				return nullptr;
			return CreateExprDateHistogram ( pVal, tHist );
		}
	}

	case FUNC_UUID_SHORT: return new Expr_UuidShort_c();

	default: // just make gcc happy
		assert ( 0 && "unhandled function id" );
	}

	return nullptr;
}

/// fold nodes subtree into opcodes
ISphExpr * ExprParser_t::CreateTree ( int iNode )
{
	if ( iNode<0 || GetCreateError() )
		return nullptr;

	const ExprNode_t & tNode = m_dNodes[iNode];
	int iOp = tNode.m_iToken;

	if ( iOp<=0 )	// tree doesn't need to be created (usually it was optimized away).
		return nullptr;

	// avoid spawning argument node in some cases
	bool bSkipChildren = false;
	if ( iOp==TOK_FUNC )
	{
		switch ( tNode.m_iFunc )
		{
		case FUNC_NOW:
		case FUNC_IN:
		case FUNC_EXIST:
		case FUNC_GEODIST:
		case FUNC_CONTAINS:
		case FUNC_ZONESPANLIST:
		case FUNC_RANKFACTORS:
		case FUNC_FACTORS:
		case FUNC_BM25F:
		case FUNC_CURTIME:
		case FUNC_CURDATE:
		case FUNC_UTC_TIME:
		case FUNC_UTC_TIMESTAMP:
		case FUNC_ALL:
		case FUNC_ANY:
		case FUNC_INDEXOF:
		case FUNC_MIN_TOP_WEIGHT:
		case FUNC_MIN_TOP_SORTVAL:
		case FUNC_REMAP:
		case FUNC_LAST_INSERT_ID:
		case FUNC_QUERY:
		case FUNC_CURRENT_USER:
		case FUNC_CONNECTION_ID:
		case FUNC_DATABASE:
		case FUNC_USER:
		case FUNC_VERSION:
		case FUNC_KNN_DIST:
		case FUNC_RANGE:
		case FUNC_HISTOGRAM:
		case FUNC_DATE_RANGE:
		case FUNC_DATE_HISTOGRAM:
		case FUNC_UUID_SHORT:
			bSkipChildren = true;
			break;
		default:
			break;
		}
	}

	CSphRefcountedPtr<ISphExpr> pLeft ( (tNode.m_iLeft<0 || bSkipChildren) ? nullptr : CreateTree ( tNode.m_iLeft ) );
	CSphRefcountedPtr<ISphExpr> pRight ( (tNode.m_iRight<0 || bSkipChildren) ? nullptr : CreateTree ( tNode.m_iRight ) );

	if ( GetCreateError() )
		return nullptr;

#define LOC_SPAWN_POLY(_classname) switch (tNode.m_eArgType) { \
	case SPH_ATTR_INTEGER:	return new _classname##Int_c ( pLeft, pRight ); \
	case SPH_ATTR_BIGINT:	return new _classname##Int64_c ( pLeft, pRight ); \
	default:				return new _classname##Float_c ( pLeft, pRight ); }

	switch (iOp)
	{
		case '+':
		case '-':
		case '*':
		case '/':
		case '&':
		case '|':
		case '%':
		case '<':
		case '>':
		case TOK_LTE:
		case TOK_GTE:
		case TOK_EQ:
		case TOK_NE:
		case TOK_AND:
		case TOK_OR:
		case TOK_NOT:
		if ( pLeft && m_dNodes[tNode.m_iLeft].m_eRetType==SPH_ATTR_JSON_FIELD && m_dNodes[tNode.m_iLeft].m_iToken==TOK_ATTR_JSON )
			pLeft = new Expr_JsonFieldConv_c ( pLeft );
		if ( pRight && m_dNodes[tNode.m_iRight].m_eRetType==SPH_ATTR_JSON_FIELD && m_dNodes[tNode.m_iRight].m_iToken==TOK_ATTR_JSON )
			pRight = new Expr_JsonFieldConv_c ( pRight );
		break;

		default:
			break;
	}

	switch (iOp)
	{
		case TOK_ATTR_INT:		return new Expr_GetInt_c ( tNode.m_tLocator, GetNameByLocator(tNode) );
		case TOK_ATTR_BITS:		return new Expr_GetBits_c ( tNode.m_tLocator, GetNameByLocator(tNode) );
		case TOK_ATTR_FLOAT:	return new Expr_GetFloat_c ( tNode.m_tLocator, GetNameByLocator(tNode) );
		case TOK_ATTR_SINT:		return new Expr_GetSint_c ( tNode.m_tLocator, GetNameByLocator(tNode) );
		case TOK_ATTR_STRING:	return new Expr_GetString_c ( tNode.m_tLocator, GetNameByLocator(tNode) );
		case TOK_ATTR_MVA64:
		case TOK_ATTR_MVA32:	return new Expr_GetMva_c ( tNode.m_tLocator, GetNameByLocator(tNode) );
		case TOK_ATTR_FACTORS:	return new Expr_GetFactorsAttr_c ( tNode.m_tLocator, GetNameByLocator(tNode) );

		case TOK_COLUMNAR_INT:		return CreateColumnarIntNode ( tNode.m_iLocator, SPH_ATTR_INTEGER );
		case TOK_COLUMNAR_TIMESTAMP:return CreateColumnarIntNode ( tNode.m_iLocator, SPH_ATTR_TIMESTAMP );
		case TOK_COLUMNAR_BIGINT:	return CreateColumnarIntNode ( tNode.m_iLocator, SPH_ATTR_BIGINT );
		case TOK_COLUMNAR_BOOL:		return CreateColumnarIntNode ( tNode.m_iLocator, SPH_ATTR_BOOL );
		case TOK_COLUMNAR_FLOAT:	return CreateColumnarFloatNode ( tNode.m_iLocator );
		case TOK_COLUMNAR_STRING:	return CreateColumnarStringNode ( tNode.m_iLocator );

		case TOK_COLUMNAR_UINT32SET:
		case TOK_COLUMNAR_INT64SET:
		case TOK_COLUMNAR_FLOATVEC:	return CreateColumnarMvaNode ( tNode.m_iLocator );

		case TOK_FIELD:			return CreateFieldNode ( tNode.m_iLocator );

		case TOK_CONST_FLOAT:	return new Expr_GetConst_c ( tNode.m_fConst );
		case TOK_CONST_INT:
			switch (tNode.m_eRetType)
			{
				case SPH_ATTR_INTEGER:	return new Expr_GetIntConst_c ( (int) tNode.m_iConst );
				case SPH_ATTR_BIGINT:	return new Expr_GetInt64Const_c ( tNode.m_iConst );
				default:				return new Expr_GetConst_c ( float ( tNode.m_iConst ) );
			}
		case TOK_CONST_STRING:
			return new Expr_GetStrConst_c ( m_sExpr.first+GetConstStrOffset(tNode), GetConstStrLength(tNode), true );
		case TOK_SUBKEY:
			return new Expr_GetStrConst_c ( m_sExpr.first+GetConstStrOffset(tNode), GetConstStrLength(tNode), false );

		case TOK_WEIGHT:		return new Expr_GetWeight_c();

		case '+':				return new Expr_Add_c ( pLeft, pRight );
		case '-':				return new Expr_Sub_c ( pLeft, pRight );
		case '*':				return new Expr_Mul_c ( pLeft, pRight );
		case '/':				return new Expr_Div_c ( pLeft, pRight );
		case '&':				return new Expr_BitAnd_c ( pLeft, pRight );
		case '|':				return new Expr_BitOr_c ( pLeft, pRight );
		case '%':				return new Expr_Mod_c ( pLeft, pRight );

		case '<':
		case '>':
		case TOK_LTE:
		case TOK_GTE:
		case TOK_EQ:
		case TOK_NE:
			return CreateCmp ( tNode, pLeft, pRight );

		case TOK_AND:			LOC_SPAWN_POLY ( Expr_And );
		case TOK_OR:			LOC_SPAWN_POLY ( Expr_Or );
		case TOK_NOT:
			return ( tNode.m_eArgType==SPH_ATTR_BIGINT )
				? (ISphExpr * ) new Expr_NotInt64_c ( pLeft )
				: (ISphExpr * ) new Expr_NotInt_c ( pLeft );

		case ',':
			if ( pLeft && pRight )
				return new Expr_Arglist_c ( pLeft, pRight );
			break;

		case TOK_NEG:			assert ( !pRight ); return new Expr_Neg_c ( pLeft );
		case TOK_FUNC:
			{
				VecRefPtrs_t<ISphExpr*> dArgs;
				if ( !PrepareFuncArgs ( tNode, bSkipChildren, pLeft, pRight, dArgs ) )
					return nullptr;

				return CreateFuncExpr ( iNode, dArgs );
			}

		case TOK_UDF:			return CreateUdfNode ( tNode.m_iFunc, pLeft );
		case TOK_HOOK_IDENT:	return m_pHook->CreateNode ( tNode.m_iFunc, nullptr, nullptr, nullptr, nullptr, m_sCreateError );
		case TOK_HOOK_FUNC:		return m_pHook->CreateNode ( tNode.m_iFunc, pLeft, m_pSchema, &m_eEvalStage, &m_bNeedDocIds, m_sCreateError );

		case TOK_MAP_ARG:
			// tricky bit
			// data gets moved (!) from node to ISphExpr at this point
			return new Expr_MapArg_c ( tNode.m_pMapArg->m_dPairs );

		case TOK_ATTR_JSON:
			if ( pLeft && m_dNodes[tNode.m_iLeft].m_iToken==TOK_SUBKEY && !tNode.m_tLocator.m_bDynamic )
			{
				// json key is a single static subkey, switch to fastpath
				return new Expr_JsonFastKey_c ( tNode.m_tLocator, GetNameByLocator(tNode), pLeft );
			} else
			{
				// json key is a generic expression, use generic catch-all JsonField
				VecRefPtrs_t<ISphExpr*> dArgs;
				CSphVector<ESphAttr> dTypes;
				if ( pLeft ) // may be NULL (top level array)
				{
					MoveToArgList ( pLeft.Leak (), dArgs );
					GatherArgRetTypes ( tNode.m_iLeft, dTypes );
				}
				return new Expr_JsonField_c ( tNode.m_tLocator, GetNameByLocator(tNode), dArgs, dTypes );
			}

		case TOK_ITERATOR:
			{
				// iterator, e.g. handles "x.gid" in SELECT ALL(x.gid=1 FOR x IN json.array)
				VecRefPtrs_t<ISphExpr*> dArgs;
				CSphVector<ESphAttr> dTypes;
				if ( pLeft )
				{
					MoveToArgList ( pLeft.Leak (), dArgs );
					GatherArgRetTypes ( tNode.m_iLeft, dTypes );
				}
				CSphRefcountedPtr<ISphExpr> pIterator { new Expr_Iterator_c ( tNode.m_tLocator, GetNameByLocator(tNode), dArgs, dTypes, tNode.m_pAttr ) };
				return new Expr_JsonFieldConv_c ( pIterator );
			}
		case TOK_IDENT:			m_sCreateError.SetSprintf ( "unknown column: %s", tNode.m_sIdent ); break;

		case TOK_IS_NULL:
		case TOK_IS_NOT_NULL:
			if ( m_dNodes[tNode.m_iLeft].m_eRetType==SPH_ATTR_JSON_FIELD )
				return new Expr_JsonFieldIsNull_c ( pLeft, tNode.m_iToken==TOK_IS_NULL );
			else
				return new Expr_GetIntConst_c ( tNode.m_iToken!=TOK_IS_NULL );

		default:				assert ( 0 && "unhandled token type" ); break;
	}

#undef LOC_SPAWN_POLY

	// fire exit
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////

/// INTERVAL() evaluator for constant turn point values case
template < typename T >
class Expr_IntervalConst_c : public Expr_ArgVsConstSet_T<T>
{
public:
	/// pre-evaluate and dismiss turn points
	explicit Expr_IntervalConst_c ( CSphVector<ISphExpr *> & dArgs )
		: Expr_ArgVsConstSet_T<T> ( dArgs[0], dArgs, 1 )
	{}

	/// evaluate arg, return interval id
	int IntEval ( const CSphMatch & tMatch ) const final
	{
		T val = this->ExprEval ( this->m_pArg, tMatch ); // 'this' fixes gcc braindamage
		ARRAY_FOREACH ( i, this->m_dValues ) // FIXME! OPTIMIZE! perform binary search here
			if ( val<this->m_dValues[i] )
				return i;
		return this->m_dValues.GetLength();
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_IntervalConst_c");		
		return Expr_ArgVsConstSet_T<T>::CalcHash ( szClassName, tSorterSchema, uHash, bDisable );		// can't do CALC_PARENT_HASH because of gcc and templates
	}

	ISphExpr* Clone () const final
	{
		return new Expr_IntervalConst_c ( *this );
	}

private:
	Expr_IntervalConst_c ( const Expr_IntervalConst_c& ) = default;
};


/// generic INTERVAL() evaluator
template < typename T >
class Expr_Interval_c : public Expr_ArgVsSet_T<T>
{
protected:
	VecRefPtrs_t<ISphExpr*> m_dTurnPoints;

public:

	explicit Expr_Interval_c ( const CSphVector<ISphExpr *> & dArgs )
		: Expr_ArgVsSet_T<T> ( dArgs[0] )
	{
		for ( int i=1; i<dArgs.GetLength(); ++i )
		{
			SafeAddRef ( dArgs[i] );
			m_dTurnPoints.Add ( dArgs[i] );
		}
	}

	/// evaluate arg, return interval id
	int IntEval ( const CSphMatch & tMatch ) const final
	{
		T val = this->ExprEval ( this->m_pArg, tMatch ); // 'this' fixes gcc braindamage
		ARRAY_FOREACH ( i, m_dTurnPoints )
			if ( val < Expr_ArgVsSet_T<T>::ExprEval ( m_dTurnPoints[i], tMatch ) )
				return i;
		return m_dTurnPoints.GetLength();
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_ArgVsSet_T<T>::Command ( eCmd, pArg );
		ARRAY_FOREACH ( i, m_dTurnPoints )
			m_dTurnPoints[i]->Command ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_Interval_c");
		CALC_CHILD_HASHES(m_dTurnPoints);
		return Expr_ArgVsSet_T<T>::CalcHash ( szClassName, tSorterSchema, uHash, bDisable );		// can't do CALC_PARENT_HASH because of gcc and templates
	}

	ISphExpr * Clone () const final
	{
		return new Expr_Interval_c ( *this );
	}

private:
	Expr_Interval_c ( const Expr_Interval_c& rhs )
		: Expr_ArgVsSet_T<T> ( rhs )
	{
		m_dTurnPoints.Resize ( rhs.m_dTurnPoints.GetLength() );
		ARRAY_FOREACH ( i, m_dTurnPoints )
			m_dTurnPoints[i] = rhs.m_dTurnPoints[i]->Clone();
	}
};

//////////////////////////////////////////////////////////////////////////

/// IN() evaluator, arbitrary scalar expression vs. constant values
template < typename T, bool BINARY >
class Expr_In_c : public Expr_ArgVsConstSet_T<T>
{
public:
	/// pre-sort values for binary search
	Expr_In_c ( ISphExpr * pArg, ConstList_c * pConsts ) :
		Expr_ArgVsConstSet_T<T> ( pArg, pConsts, false )
	{
		this->m_dValues.Uniq();
	}

	/// evaluate arg, check if the value is within set
	int IntEval ( const CSphMatch & tMatch ) const final
	{
		T val = this->ExprEval ( this->m_pArg, tMatch ); // 'this' fixes gcc braindamage

		if_const ( BINARY )
			return this->m_dValues.BinarySearch ( val )!=nullptr;
		else
		{
			for ( auto i : this->m_dValues )
				if ( i==val )
					return 1;

			return 0;
		}
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_In_c");
		return Expr_ArgVsConstSet_T<T>::CalcHash ( szClassName, tSorterSchema, uHash, bDisable );		// can't do CALC_PARENT_HASH because of gcc and templates
	}

	ISphExpr * Clone () const final
	{
		return new Expr_In_c ( *this );
	}

private:
	Expr_In_c ( const Expr_In_c& ) = default;
};


/// IN() evaluator, arbitrary scalar expression vs. uservar
/// (for the sake of evaluator, uservar is a pre-sorted, refcounted external vector)
class Expr_InUservar_c : public Expr_ArgVsConstSet_T<int64_t>
{
public:
	/// just get hold of args
	explicit Expr_InUservar_c ( ISphExpr * pArg, const UservarIntSet_c& pConsts )
		: Expr_ArgVsConstSet_T<int64_t> ( pArg, pConsts )
	{
		this->m_dValues.Sort ();
	}

	/// evaluate arg, check if the value is within set
	int IntEval ( const CSphMatch & tMatch ) const final
	{
		int64_t iVal = ExprEval ( this->m_pArg, tMatch ); // 'this' fixes gcc braindamage
		return this->m_dValues.BinarySearch ( iVal )!=nullptr;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_InUservar_c");
		return Expr_ArgVsConstSet_T<int64_t>::CalcHash ( szClassName, tSorterSchema, uHash, bDisable );
	}

	ISphExpr * Clone () const final
	{
		return new Expr_InUservar_c ( *this );
	}

private:
	Expr_InUservar_c ( const Expr_InUservar_c& ) = default;
};


/// IN() evaluator, MVA attribute vs. constant values
template < typename T >
class Expr_MVAIn_c : public Expr_ArgVsConstSet_T<int64_t>, public ExprLocatorTraits_t
{
public:
	/// pre-sort values for binary search
	Expr_MVAIn_c ( const CSphAttrLocator & tLoc, const CSphString & sAttr, ConstList_c * pConsts )
		: Expr_ArgVsConstSet_T<int64_t> ( nullptr, pConsts, false )
		, ExprLocatorTraits_t ( tLoc, sAttr )
	{
		assert ( pConsts );
		this->m_dValues.Sort();
	}

	ByteBlob_t MvaEval ( const CSphMatch & ) const final { assert ( 0 && "not implemented" ); return {nullptr,0}; }

	/// evaluate arg, check if any values are within set
	int IntEval ( const CSphMatch & tMatch ) const final
	{
		auto dMva = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		return MvaEval_Any<T> ( dMva, m_dValues );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_ArgVsConstSet_T<int64_t>::Command ( eCmd, pArg );
		ExprLocatorTraits_t::HandleCommand ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE *)pArg;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_MVAIn_c");
		return CALC_DEP_HASHES_EX(m_uValueHash);
	}

	ISphExpr * Clone () const final
	{
		return new Expr_MVAIn_c ( *this );
	}

protected:
	const BYTE *		m_pBlobPool {nullptr};

private:
	Expr_MVAIn_c ( const Expr_MVAIn_c & rhs )
		: Expr_ArgVsConstSet_T<int64_t> ( rhs )
		, ExprLocatorTraits_t ( rhs )
	{}
};

/// IN() evaluator, MVA attribute vs. uservars
template < typename T >
class Expr_MVAInU_c : public Expr_ArgVsConstSet_T<int64_t>, public ExprLocatorTraits_t
{
public:
	/// pre-sort values for binary search
	Expr_MVAInU_c ( const CSphAttrLocator & tLoc, const CSphString & sAttr, const UservarIntSet_c & pUservar )
		: Expr_ArgVsConstSet_T<int64_t> ( nullptr, pUservar )
		, ExprLocatorTraits_t ( tLoc, sAttr )
	{
		assert ( pUservar );
		this->m_dValues.Sort();
	}

	ByteBlob_t MvaEval ( const CSphMatch & ) const final { assert ( 0 && "not implemented" ); return { nullptr, 0}; }

	/// evaluate arg, check if any values are within set
	int IntEval ( const CSphMatch & tMatch ) const final
	{
		auto dMva = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		return MvaEval_Any<T> ( dMva, m_dValues );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_ArgVsConstSet_T<int64_t>::Command ( eCmd, pArg );
		ExprLocatorTraits_t::HandleCommand ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE *)pArg;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_MVAInU_c");
		return CALC_DEP_HASHES_EX(m_uValueHash);
	}

	ISphExpr * Clone () const final
	{
		return new Expr_MVAInU_c ( *this );
	}

protected:
	const BYTE *		m_pBlobPool {nullptr};

private:
	Expr_MVAInU_c ( const Expr_MVAInU_c & rhs )
		: Expr_ArgVsConstSet_T<int64_t> ( rhs )
		, ExprLocatorTraits_t ( rhs )
	{}
};


/// LENGTH() evaluator for MVAs
class Expr_MVALength_c : public Expr_WithLocator_c
{
public:
	Expr_MVALength_c ( const CSphAttrLocator & tLoc, const CSphString & sAttr, bool b64 )
		: Expr_WithLocator_c ( tLoc, sAttr )
		, m_b64 ( b64 )
	{}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		auto dMva = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		return (int)( m_b64 ? dMva.second/sizeof(int64_t) : dMva.second/sizeof(DWORD) );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_WithLocator_c::Command ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE*)pArg;
	}

	float Eval ( const CSphMatch & tMatch ) const final { return (float)IntEval ( tMatch ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_MVALength_c");
		CALC_POD_HASH(m_b64);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_MVALength_c ( *this );
	}

protected:
	bool				m_b64;
	const BYTE *		m_pBlobPool { nullptr };

private:
	Expr_MVALength_c ( const Expr_MVALength_c& rhs )
		: Expr_WithLocator_c ( rhs )
		, m_b64 ( rhs.m_b64 )
	{}
};


/// aggregate functions evaluator for MVA attribute
template < typename T >
class Expr_MVAAggr_c : public Expr_WithLocator_c
{
public:
	Expr_MVAAggr_c ( const CSphAttrLocator & tLoc, const CSphString & sAttr, ESphAggrFunc eFunc )
		: Expr_WithLocator_c ( tLoc, sAttr )
		, m_eFunc ( eFunc )
	{}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const final
	{
		auto dMva = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		if ( !dMva.second )
			return 0;

		int nValues = dMva.second / sizeof(T);

		const T * L = (const T *)dMva.first;
		const T * R = L+nValues-1;

		switch ( m_eFunc )
		{
			case SPH_AGGR_MIN:	return *L;
			case SPH_AGGR_MAX:	return *R;
			default:			return 0;
		}
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_WithLocator_c::Command ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE *)pArg;
	}

	float	Eval ( const CSphMatch & tMatch ) const final { return (float)Int64Eval ( tMatch ); }
	int		IntEval ( const CSphMatch & tMatch ) const final { return (int)Int64Eval ( tMatch ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_MVAAggr_c");
		CALC_POD_HASH(m_eFunc);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_MVAAggr_c ( *this );
	}

protected:
	const BYTE *	m_pBlobPool {nullptr};
	ESphAggrFunc	m_eFunc {SPH_AGGR_NONE};

private:
	Expr_MVAAggr_c ( const Expr_MVAAggr_c& rhs )
		: Expr_WithLocator_c ( rhs )
		, m_eFunc ( rhs.m_eFunc )
	{}
};

/// IN() evaluator, JSON array vs. constant values
class Expr_JsonFieldIn_c : public Expr_ArgVsConstSet_T<int64_t>
{
public:
	Expr_JsonFieldIn_c ( ConstList_c * pConsts, ISphExpr * pArg, ESphCollation eCollation )
		: Expr_ArgVsConstSet_T<int64_t> ( pArg, pConsts, true )
		, m_fnHashCalc ( GetStringHashCalcFunc(eCollation) )
	{
		assert ( pConsts );

		const char * szExpr = pConsts->m_sExpr.first;
		int iExprLen = pConsts->m_sExpr.second;

		if ( pConsts->m_bPackedStrings )
		{
			assert(m_fnHashCalc);

			for ( int64_t iVal : m_dValues )
			{
				auto iOfs = GetConstStrOffset ( iVal );
				auto iLen = GetConstStrLength ( iVal );
				if ( iOfs>0 && iLen>0 && iOfs+iLen<=iExprLen )
				{
					auto tRes = SqlUnescapeN ( szExpr + iOfs, iLen );
					int iLen = tRes.second;
					m_dHashes.Add ( iLen ? m_fnHashCalc ( (const BYTE*)tRes.first.cstr(), iLen, SPH_FNV64_SEED ) : 0 );
					m_dStrings.Add ( tRes.first );
				}
			}
			m_dHashes.Sort();
		}
	}

	Expr_JsonFieldIn_c ( const UservarIntSet_c & pUserVar, ISphExpr * pArg, ESphCollation eCollation )
		: Expr_ArgVsConstSet_T<int64_t> ( pArg, pUserVar )
		, m_fnHashCalc ( GetStringHashCalcFunc(eCollation) )
	{
		assert ( pUserVar );
		m_dHashes.Sort();
	}

	Expr_JsonFieldIn_c ( const VecTraits_T<CSphString> & dVals, ISphExpr * pArg, ESphCollation eCollation )
		: Expr_ArgVsConstSet_T<int64_t> ( pArg )
		, m_fnHashCalc ( GetStringHashCalcFunc(eCollation) )
	{
		m_dHashes.Resize ( dVals.GetLength() );
		m_dStrings.Resize ( dVals.GetLength() );
		m_uValueHash = SPH_FNV64_SEED;

		assert(m_fnHashCalc);
		ARRAY_FOREACH ( i, dVals )
		{
			const CSphString & sVal = dVals[i];
			m_dStrings[i] = sVal;
			int iLen = sVal.Length();
			m_dHashes[i] = iLen ? m_fnHashCalc ( (const BYTE*)sVal.cstr(), iLen, SPH_FNV64_SEED ) : 0;
			m_uValueHash = sphFNV64cont ( sVal.cstr(), m_uValueHash );
		}
		m_dHashes.Uniq();
	}

	Expr_JsonFieldIn_c ( const VecTraits_T<int64_t> & dVals, ISphExpr * pArg, ESphCollation eCollation )
		: Expr_ArgVsConstSet_T<int64_t> ( pArg )
		, m_fnHashCalc ( GetStringHashCalcFunc(eCollation) )
	{
		m_dValues.Resize ( dVals.GetLength() );
		ARRAY_FOREACH ( i, dVals )
			m_dValues[i] = dVals[i];

		m_dValues.Uniq();
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_ArgVsConstSet_T<int64_t>::Command ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE*)pArg;
	}

	/// evaluate arg, check if any values are within set
	int IntEval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pVal = nullptr;
		ESphJsonType eJson = GetKey ( &pVal, tMatch );
		int64_t iVal = 0;
		switch ( eJson )
		{
			case JSON_INT32_VECTOR:		return ArrayEval<int> ( pVal );
			case JSON_INT64_VECTOR:		return ArrayEval<int64_t> ( pVal );
			case JSON_STRING_VECTOR:	return StringArrayEval ( pVal, false );
			case JSON_DOUBLE_VECTOR:	return ArrayFloatEval ( pVal );
			case JSON_STRING:			return StringArrayEval ( pVal, true );
			case JSON_INT32:
			case JSON_INT64:
				iVal = ( eJson==JSON_INT32 ? sphJsonLoadInt ( &pVal ) : sphJsonLoadBigint ( &pVal ) );
				if ( m_bFloat )
					return FloatEval ( (float)iVal );
				else
					return ValueEval ( iVal  );
			case JSON_DOUBLE:
				iVal = sphJsonLoadBigint ( &pVal );
				if ( m_bFloat )
					return FloatEval ( sphQW2D ( iVal ) );
				else
					return ValueEval ( iVal  );

			case JSON_TRUE:				return ValueEval(1);
			case JSON_FALSE:			return ValueEval(0);

			case JSON_MIXED_VECTOR:
				{
					const BYTE * p = pVal;
					sphJsonUnpackInt ( &p ); // skip node length
					int iLen = sphJsonUnpackInt ( &p );
					for ( int i=0; i<iLen; i++ )
					{
						auto eType = (ESphJsonType)*p++;
						pVal = p;
						int iRes = 0;
						switch (eType)
						{
							case JSON_STRING:
								iRes =  StringArrayEval ( pVal, true );
							break;
							case JSON_INT32:
							case JSON_INT64:
								iVal = ( eType==JSON_INT32 ? sphJsonLoadInt ( &pVal ) : sphJsonLoadBigint ( &pVal ) );
								if ( m_bFloat )
									iRes = FloatEval ( (float)iVal );
								else
									iRes = ValueEval ( iVal );
							break;
							case JSON_DOUBLE:
								iVal = sphJsonLoadBigint ( &pVal );
								if ( m_bFloat )
									iRes = FloatEval ( sphQW2D ( iVal ) );
								else
									iRes = ValueEval ( iVal  );
								break;
							default: break; // for weird subobjects, just let IN() return false
						}
						if ( iRes )
							return 1;
						sphJsonSkipNode ( eType, &p );
					}
					return 0;
				}
			default:					return 0;
		}
	}

	bool SetupAsFilter ( CSphFilterSettings & tFilter, const ISphSchema & tSchema, const SIContainer_c & tSI ) const override
	{
		bool bExclude = tFilter.m_bExclude;
		if ( !CanAliasedExprSetupAsFilter ( tFilter, bExclude ) )
			return false;

		if ( !m_dValues.GetLength() )
			return false;
		
		std::pair<const ISphSchema*,CSphString> tSchemaWithName;
		tSchemaWithName.first = &tSchema;
		m_pArg->Command ( SPH_EXPR_FORMAT_AS_TEXT, (void*)&tSchemaWithName );
		if ( tSchemaWithName.second.IsEmpty() )
			return false;

		if ( !tSI.IsEnabled ( tSchemaWithName.second ) )
			return false;

		tFilter.m_bExclude = bExclude;
		if ( m_dStrings.IsEmpty() )
		{
			tFilter.m_dValues.Resize(0);
			for ( auto i : m_dValues )
				tFilter.m_dValues.Add(i);
		}
		else
		{
			tFilter.m_eType = m_dStrings.GetLength()==1 ? SPH_FILTER_STRING : SPH_FILTER_STRING_LIST;
			tFilter.m_dStrings = m_dStrings;
		}

		tFilter.m_sAttrName = tSchemaWithName.second;	
		return true;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_JsonFieldIn_c");
		return CALC_PARENT_HASH_EX(m_uValueHash);
	}

	ISphExpr * Clone () const final
	{
		return new Expr_JsonFieldIn_c ( *this );
	}

protected:
	const BYTE *		m_pBlobPool {nullptr};
	CSphVector<int64_t>	m_dHashes;
	StrVec_t			m_dStrings;
	StrHashCalc_fn		m_fnHashCalc = nullptr;

	ESphJsonType GetKey ( const BYTE ** ppKey, const CSphMatch & tMatch ) const
	{
		return ::GetKey ( ppKey, tMatch, m_pBlobPool, m_pArg );
	}

	int ValueEval ( const int64_t iVal ) const
	{
		for ( int64_t iValue: m_dValues )
			if ( iVal==iValue )
				return 1;
		return 0;
	}

	int FloatEval ( const double fVal ) const
	{
		assert ( m_bFloat );
		for ( int64_t iFilterVal : m_dValues )
		{
			double fFilterVal = sphDW2F ( (DWORD)iFilterVal );
			if ( fabs ( fVal - fFilterVal )<=1e-6 )
				return 1;
		}
		return 0;
	}

	// cannot apply MvaEval() on unordered JSON arrays, using linear search
	template <typename T>
	int ArrayEval ( const BYTE * pVal ) const
	{
		int iLen = sphJsonUnpackInt ( &pVal );
		auto * pArray = (const T *)pVal;
		const T * pArrayMax = pArray+iLen;
		for ( int64_t dValue : m_dValues )
		{
			auto iVal = (T)dValue;
			for ( const T * m = pArray; m<pArrayMax; ++m )
				if ( iVal==*m )
					return 1;
		}
		return 0;
	}

	int StringArrayEval ( const BYTE * pVal, bool bValueEval ) const
	{
		if ( !bValueEval )
			sphJsonUnpackInt ( &pVal );

		int iCount = bValueEval ? 1 : sphJsonUnpackInt ( &pVal );
		assert(m_fnHashCalc);

		while ( iCount-- )
		{
			int iLen = sphJsonUnpackInt ( &pVal );
			if ( m_dHashes.BinarySearch ( iLen ? m_fnHashCalc ( pVal, iLen, SPH_FNV64_SEED ) : 0 ) )
				return 1;
			pVal += iLen;
		}
		return 0;
	}

	int ArrayFloatEval ( const BYTE * pVal ) const
	{
		int iLen = sphJsonUnpackInt ( &pVal );

		for ( int64_t iFilterVal : m_dValues )
		{
			double fFilterVal = ( m_bFloat ? sphDW2F ( (DWORD)iFilterVal ) : iFilterVal );

			const BYTE * p = pVal;
			for ( int i=0; i<iLen; i++ )
			{
				double fStored = sphQW2D ( sphJsonLoadBigint ( &p ) );
				if ( fabs ( fStored - fFilterVal )<=1e-6 )
					return 1;
			}
		}
		return 0;
	}

	bool IsJson ( bool & bConverted ) const final
	{
		bConverted = true;
		return true;
	}

private:
	Expr_JsonFieldIn_c ( const Expr_JsonFieldIn_c& rhs )
		: Expr_ArgVsConstSet_T<int64_t> ( rhs )
		, m_dHashes ( rhs.m_dHashes )
		, m_dStrings ( rhs.m_dStrings )
		, m_fnHashCalc ( rhs.m_fnHashCalc )
	{}
};


ISphExpr * ExprJsonIn ( const VecTraits_T<CSphString> & dVals, ISphExpr * pArg, ESphCollation eCollation )
{
	return new Expr_JsonFieldIn_c ( dVals, pArg, eCollation );
}


ISphExpr * ExprJsonIn ( const VecTraits_T<int64_t> & dVals, ISphExpr * pArg, ESphCollation eCollation )
{
	return new Expr_JsonFieldIn_c ( dVals, pArg, eCollation );
}

////////////////////////////////////////////////////////////////////
/// JSON field vs constant range
class Expr_JsonFieldRange_c : public Expr_ArgVsSet_T<int64_t>
{
	using BASE = Expr_ArgVsSet_T<int64_t>;

public:
	Expr_JsonFieldRange_c ( const CommonFilterSettings_t & tFilter, ISphExpr * pArg )
		: BASE ( pArg )
		, m_tFilter ( tFilter )
	{}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		BASE::Command ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE*)pArg;
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		const BYTE * pVal = nullptr;
		ESphJsonType eJson = GetKey ( &pVal, tMatch );
		switch ( eJson )
		{
		case JSON_INT32_VECTOR:		return ArrayEval<int>(pVal);
		case JSON_INT64_VECTOR:		return ArrayEval<int64_t>(pVal);
		case JSON_DOUBLE_VECTOR:	return ArrayFloatEval(pVal);
		case JSON_INT32:
		case JSON_INT64:			return ValueEval ( eJson==JSON_INT32 ? sphJsonLoadInt ( &pVal ) : sphJsonLoadBigint ( &pVal ) );
		case JSON_DOUBLE:			return ValueEval ( sphQW2D ( sphJsonLoadBigint ( &pVal ) ) );
		default:
			return 0;
		}
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_JsonFieldRange_c");
		CALC_POD_HASH ( m_tFilter.m_iMinValue );
		CALC_POD_HASH ( m_tFilter.m_iMaxValue );
		CALC_POD_HASH ( m_tFilter.m_bHasEqualMin );
		CALC_POD_HASH ( m_tFilter.m_bHasEqualMax );
		CALC_POD_HASH ( m_tFilter.m_bOpenLeft );
		CALC_POD_HASH ( m_tFilter.m_bOpenRight );
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_JsonFieldRange_c ( *this );
	}

protected:
	const BYTE *			m_pBlobPool {nullptr};
	CommonFilterSettings_t	m_tFilter;

	ESphJsonType GetKey ( const BYTE ** ppKey, const CSphMatch & tMatch ) const
	{
		return ::GetKey ( ppKey, tMatch, m_pBlobPool, m_pArg );
	}

	template<typename T>
	int ValueEval ( T tVal ) const
	{
		if ( m_tFilter.m_eType==SPH_FILTER_FLOATRANGE )
		{
			if ( EvalRange ( (float)tVal, m_tFilter ) )
				return 1;
		}
		else
		{
			if ( EvalRange ( tVal, m_tFilter ) )
				return 1;
		}

		return 0;
	}

	template <typename T>
	int ArrayEval ( const BYTE * pVal ) const
	{
		int iLen = sphJsonUnpackInt ( &pVal );
		auto * pArray = (const T *)pVal;
		if ( m_tFilter.m_eType==SPH_FILTER_FLOATRANGE )
		{
			for ( const T * pValue = pArray; pValue < pArray+iLen; pValue++ )
				if ( EvalRange ( (float)*pValue, m_tFilter ) )
					return 1;
		}
		else
		{
			for ( const T * pValue = pArray; pValue < pArray+iLen; pValue++ )
				if ( EvalRange ( *pValue, m_tFilter ) )
					return 1;
		}

		return 0;
	}

	int ArrayFloatEval ( const BYTE * pVal ) const
	{
		int iLen = sphJsonUnpackInt ( &pVal );
		const BYTE * p = pVal;
		for ( int i=0; i<iLen; i++ )
			if ( EvalRange ( sphQW2D ( sphJsonLoadBigint ( &p ) ), m_tFilter ) )
				return 1;

		return 0;
	}

	bool IsJson ( bool & bConverted ) const final
	{
		bConverted = true;
		return true;
	}

private:
	Expr_JsonFieldRange_c ( const Expr_JsonFieldRange_c & rhs )
		: Expr_ArgVsSet_T<int64_t> ( rhs )
		, m_tFilter ( rhs.m_tFilter )
	{}
};


ISphExpr * ExprJsonRange ( const CommonFilterSettings_t & tFilter, ISphExpr * pArg )
{
	return new Expr_JsonFieldRange_c ( tFilter, pArg );
}

////////////////////////////////////////////////////////////////////

// fixme! Expr_ArgVsConstSet_T collects raw packed strings in the case.
// m.b. store FNV hashes there instead, and use them to boost search speed?
class Expr_StrIn_c : public Expr_ArgVsConstSet_T<int64_t>, public ExprLocatorTraits_t
{
public:
	Expr_StrIn_c ( const CSphAttrLocator & tLoc, const CSphString & sAttr, ConstList_c * pConsts, ESphCollation eCollation )
		: Expr_ArgVsConstSet_T<int64_t> ( nullptr, pConsts, false )
		, ExprLocatorTraits_t ( tLoc, sAttr )
	{
		assert ( pConsts );

		m_fnStrCmp = GetStringCmpFunc ( eCollation );

		const char * sExpr = pConsts->m_sExpr.first;
		int iExprLen = pConsts->m_sExpr.second;

		for ( int64_t iVal : m_dValues  )
		{
			auto iOfs = GetConstStrOffset ( iVal );
			auto iLen = GetConstStrLength ( iVal );
			if ( iOfs>0 && iOfs+iLen<=iExprLen )
			{
				auto sRes = SqlUnescape ( sExpr + iOfs, iLen );
				m_dStringValues.Add ( sRes );
			}
		}
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		auto tVal = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		for ( const auto & tString : m_dStringValues )
			if ( m_fnStrCmp ( tVal, ByteBlob_t ( tString ), false )==0 )
				return 1;

		return 0;
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_ArgVsConstSet_T<int64_t>::Command ( eCmd, pArg );
		ExprLocatorTraits_t::HandleCommand ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE*)pArg;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_StrIn_c");
		CALC_POD_HASH(m_fnStrCmp);
		return CALC_PARENT_HASH_EX(m_uValueHash);
	}

	ISphExpr* Clone() const final
	{
		return new Expr_StrIn_c ( *this );
	}

protected:
	const BYTE *			m_pBlobPool {nullptr};
	SphStringCmp_fn			m_fnStrCmp {nullptr};
	StrVec_t				m_dStringValues;

private:
	Expr_StrIn_c ( const Expr_StrIn_c& rhs )
		: Expr_ArgVsConstSet_T<int64_t> (rhs)
		, ExprLocatorTraits_t (rhs)
		, m_fnStrCmp ( rhs.m_fnStrCmp )
		, m_dStringValues ( rhs.m_dStringValues )
	{}
};

// fixme! m.b. it is better to keep uservar ref instead of deep copy in Expr_ArgVsConstSet_T<int64_t>?
class Expr_StrInU_c : public Expr_ArgVsConstSet_T<int64_t>, public ExprLocatorTraits_t
{
public:
	Expr_StrInU_c ( const CSphAttrLocator & tLoc, const CSphString & sAttr, const UservarIntSet_c& pUservar, ESphCollation eCollation )
		: Expr_ArgVsConstSet_T<int64_t> ( nullptr, pUservar )
		, ExprLocatorTraits_t ( tLoc, sAttr )
		, m_fnStrCmp ( GetStringCmpFunc ( eCollation ))
	{
		assert ( pUservar );
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		return 0;
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_ArgVsConstSet_T<int64_t>::Command ( eCmd, pArg );
		ExprLocatorTraits_t::HandleCommand ( eCmd, pArg );

		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE*)pArg;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_StrInU_c");
		CALC_POD_HASH(m_fnStrCmp);
		return CALC_PARENT_HASH_EX(m_uValueHash);
	}

	ISphExpr * Clone () const final
	{
		return new Expr_StrInU_c ( *this );
	}

protected:
	const BYTE *			m_pBlobPool {nullptr};
	SphStringCmp_fn			m_fnStrCmp {nullptr};

private:
	Expr_StrInU_c ( const Expr_StrInU_c& rhs )
		: Expr_ArgVsConstSet_T<int64_t> (rhs)
		, ExprLocatorTraits_t (rhs)
		, m_fnStrCmp ( rhs.m_fnStrCmp )
	{}
};

//////////////////////////////////////////////////////////////////////////

/// generic BITDOT() evaluator
/// first argument is a bit mask and the rest ones are bit weights
/// function returns sum of bits multiplied by their weights
/// BITDOT(5, 11, 33, 55) => 1*11 + 0*33 + 1*55 = 66
/// BITDOT(4, 11, 33, 55) => 0*11 + 0*33 + 1*55 = 55
template < typename T >
class Expr_Bitdot_c : public Expr_ArgVsSet_T<T>
{
public:
	/// take ownership of arg and turn points
	explicit Expr_Bitdot_c ( const CSphVector<ISphExpr *> & dArgs )
			: Expr_ArgVsSet_T<T> ( dArgs[0] )
	{
		for ( int i = 1; i<dArgs.GetLength (); ++i )
		{
			SafeAddRef ( dArgs[i] );
			m_dBitWeights.Add ( dArgs[i] );
		}
	}

	float Eval ( const CSphMatch & tMatch ) const final
	{
		return (float) DoEval ( tMatch );
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		return (int) DoEval ( tMatch );
	}

	int64_t Int64Eval ( const CSphMatch & tMatch ) const final
	{
		return (int64_t) DoEval ( tMatch );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		Expr_ArgVsSet_T<T>::Command ( eCmd, pArg );
		ARRAY_FOREACH ( i, m_dBitWeights )
			m_dBitWeights[i]->Command ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_Bitdot_c");
		CALC_CHILD_HASHES(m_dBitWeights);
		return Expr_ArgVsSet_T<T>::CalcHash ( szClassName, tSorterSchema, uHash, bDisable );		// can't do CALC_PARENT_HASH because of gcc and templates
	}

	ISphExpr* Clone() const final
	{
		return new Expr_Bitdot_c ( *this );
	}

protected:
	VecRefPtrs_t<ISphExpr*> m_dBitWeights;

	/// generic evaluate
	T DoEval ( const CSphMatch & tMatch ) const
	{
		int64_t uArg = this->m_pArg->Int64Eval ( tMatch ); // 'this' fixes gcc braindamage
		T tRes = 0;

		int iBit = 0;
		while ( uArg && iBit<m_dBitWeights.GetLength() )
		{
			if ( uArg & 1 )
				tRes += Expr_ArgVsSet_T<T>::ExprEval ( m_dBitWeights[iBit], tMatch );
			uArg >>= 1;
			iBit++;
		}

		return tRes;
	}

private:
	Expr_Bitdot_c ( const Expr_Bitdot_c& rhs )
		: Expr_ArgVsSet_T<T> ( rhs )
	{
		m_dBitWeights.Resize ( rhs.m_dBitWeights.GetLength () );
		ARRAY_FOREACH ( i, m_dBitWeights )
			m_dBitWeights[i] = SafeClone (rhs.m_dBitWeights[i]);
	}
};

//////////////////////////////////////////////////////////////////////////
class GeodistTraits_c : public GeoDistSettings_t
{
public:
	void HandleCommand ( ESphExprCommand eCmd, void * pArg )
	{
		switch ( eCmd )
		{
		case SPH_EXPR_GET_GEODIST_SETTINGS:
		{
			auto pSettings = (std::pair<GeoDistSettings_t *, bool>*)pArg;
			assert ( pSettings );
			pSettings->first = this;
			pSettings->second = true;
		}
		break;

		case SPH_EXPR_GET_DEPENDENT_COLS:
			static_cast<StrVec_t*> ( pArg )->Add ( m_sAttrLat );
			static_cast<StrVec_t*> ( pArg )->Add ( m_sAttrLon );
			break;

		default:
			break;
		}
	}
};

//////////////////////////////////////////////////////////////////////////
/// geodist() - attr point, constant anchor
class Expr_GeodistAttrConst_c : public ISphExpr, public GeodistTraits_c
{
public:
	Expr_GeodistAttrConst_c ( Geofunc_fn pFunc, float fOut, CSphAttrLocator tLat, CSphAttrLocator tLon, float fAnchorLat, float fAnchorLon, const CSphString & sAttrLat, const CSphString & sAttrLon )
		: m_tLat ( tLat )
		, m_tLon ( tLon )
	{
		m_pFunc = pFunc;
		m_fScale = fOut;
		m_fAnchorLat = fAnchorLat;
		m_fAnchorLon = fAnchorLon;
		m_sAttrLat = sAttrLat;
		m_sAttrLon = sAttrLon;
	}

	float Eval ( const CSphMatch & tMatch ) const final
	{
		return m_fScale*m_pFunc ( tMatch.GetAttrFloat ( m_tLat ), tMatch.GetAttrFloat ( m_tLon ), m_fAnchorLat, m_fAnchorLon );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final
	{
		sphFixupLocator ( m_tLat, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tLon, pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final	{ HandleCommand ( eCmd, pArg ); }

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GeodistAttrConst_c");
		CALC_POD_HASH(m_fAnchorLat);
		CALC_POD_HASH(m_fAnchorLon);
		CALC_POD_HASH(m_fScale);
		CALC_POD_HASH(m_pFunc);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone() const final	{ return new Expr_GeodistAttrConst_c ( *this ); }

private:
	CSphAttrLocator	m_tLat;
	CSphAttrLocator	m_tLon;

	Expr_GeodistAttrConst_c ( const Expr_GeodistAttrConst_c& rhs )
		: m_tLat ( rhs.m_tLat )
		, m_tLon ( rhs.m_tLon )
	{
		*((GeoDistSettings_t*)this) = rhs;
	}
};

/// geodist() - expr point, constant anchor
class Expr_GeodistConst_c: public ISphExpr, public GeodistTraits_c
{
public:
	Expr_GeodistConst_c ( Geofunc_fn pFunc, float fOut, ISphExpr * pLat, ISphExpr * pLon, float fAnchorLat, float fAnchorLon, const CSphString & sAttrLat, const CSphString & sAttrLon )
		: m_pLat ( pLat )
		, m_pLon ( pLon )
	{
		SafeAddRef ( pLat );
		SafeAddRef ( pLon );

		m_pFunc = pFunc;
		m_fScale = fOut;
		m_fAnchorLat = fAnchorLat;
		m_fAnchorLon = fAnchorLon;
		m_sAttrLat = sAttrLat;
		m_sAttrLon = sAttrLon;
	}

	float Eval ( const CSphMatch & tMatch ) const final
	{
		return m_fScale*m_pFunc ( m_pLat->Eval(tMatch), m_pLon->Eval(tMatch), m_fAnchorLat, m_fAnchorLon );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final
	{
		m_pLat->FixupLocator ( pOldSchema, pNewSchema );
		m_pLon->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		m_pLat->Command ( eCmd, pArg );
		m_pLon->Command ( eCmd, pArg );

		HandleCommand ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_GeodistConst_c");
		CALC_POD_HASH(m_fAnchorLat);
		CALC_POD_HASH(m_fAnchorLon);
		CALC_POD_HASH(m_fScale);
		CALC_POD_HASH(m_pFunc);
		CALC_CHILD_HASH(m_pLat);
		CALC_CHILD_HASH(m_pLon);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_GeodistConst_c ( *this );
	}

private:
	CSphRefcountedPtr<ISphExpr>	m_pLat;
	CSphRefcountedPtr<ISphExpr>	m_pLon;

	Expr_GeodistConst_c ( const Expr_GeodistConst_c& rhs )
		: m_pLat ( SafeClone (rhs.m_pLat) )
		, m_pLon ( SafeClone (rhs.m_pLon) )
	{
		*((GeoDistSettings_t*)this) = rhs;
	}
};

/// geodist() - expr point, expr anchor
class Expr_Geodist_c: public ISphExpr
{
public:
	Expr_Geodist_c ( Geofunc_fn pFunc, float fOut, ISphExpr * pLat, ISphExpr * pLon, ISphExpr * pAnchorLat, ISphExpr * pAnchorLon )
		: m_pFunc ( pFunc )
		, m_fOut ( fOut )
		, m_pLat ( pLat )
		, m_pLon ( pLon )
		, m_pAnchorLat ( pAnchorLat )
		, m_pAnchorLon ( pAnchorLon )
	{
		SafeAddRef ( pLat );
		SafeAddRef ( pLon );
		SafeAddRef ( pAnchorLat );
		SafeAddRef ( pAnchorLon );
	}

	float Eval ( const CSphMatch & tMatch ) const final
	{
		return m_fOut*m_pFunc ( m_pLat->Eval(tMatch), m_pLon->Eval(tMatch), m_pAnchorLat->Eval(tMatch), m_pAnchorLon->Eval(tMatch) );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final
	{
		m_pLat->FixupLocator ( pOldSchema, pNewSchema );
		m_pLon->FixupLocator ( pOldSchema, pNewSchema );
		m_pAnchorLat->FixupLocator ( pOldSchema, pNewSchema );
		m_pAnchorLon->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) final
	{
		m_pLat->Command ( eCmd, pArg );
		m_pLon->Command ( eCmd, pArg );
		m_pAnchorLat->Command ( eCmd, pArg );
		m_pAnchorLon->Command ( eCmd, pArg );
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_Geodist_c");
		CALC_POD_HASH(m_fOut);
		CALC_POD_HASH(m_pFunc);
		CALC_CHILD_HASH(m_pLat);
		CALC_CHILD_HASH(m_pLon);
		CALC_CHILD_HASH(m_pAnchorLat);
		CALC_CHILD_HASH(m_pAnchorLon);
		return CALC_DEP_HASHES();
	}

	ISphExpr * Clone () const final
	{
		return new Expr_Geodist_c ( *this );
	}

private:
	Geofunc_fn	m_pFunc;
	float		m_fOut;
	CSphRefcountedPtr<ISphExpr>	m_pLat;
	CSphRefcountedPtr<ISphExpr>	m_pLon;
	CSphRefcountedPtr<ISphExpr>	m_pAnchorLat;
	CSphRefcountedPtr<ISphExpr>	m_pAnchorLon;

	Expr_Geodist_c ( const Expr_Geodist_c& rhs )
		: m_pFunc ( rhs.m_pFunc )
		, m_fOut ( rhs.m_fOut )
		, m_pLat ( SafeClone (rhs.m_pLat) )
		, m_pLon ( SafeClone (rhs.m_pLon) )
		, m_pAnchorLat ( SafeClone (rhs.m_pAnchorLat) )
		, m_pAnchorLon ( SafeClone (rhs.m_pAnchorLon) )
	{}
};

class Expr_Regex_c final : public Expr_ArgVsSet_T<int>
{
public:
	Expr_Regex_c ( ISphExpr * pAttr, ISphExpr * pString )
		: Expr_ArgVsSet_T ( pAttr )
	{
		CSphMatch tTmp;
		const BYTE * sVal = nullptr;
		int iLen = pString->StringEval ( tTmp, &sVal );
		m_sRegex = CSphString ( (const char*)sVal, iLen );
		if ( iLen )
			m_uFilterHash = sphFNV64 ( sVal, iLen );

		SetupRE2();
	}

#if WITH_RE2
	RE2 * GetRE2() const { return m_pRE2; }
#endif

	~Expr_Regex_c() final
	{
#if WITH_RE2
		SafeDelete ( m_pRE2 );
#endif
	}

	int IntEval ( const CSphMatch & tMatch ) const final
	{
		int iRes = 0;
#if WITH_RE2
		if ( !m_pRE2 )
			return 0;

		const BYTE * sVal = nullptr;
		int iLen = m_pArg->StringEval ( tMatch, &sVal );

		re2::StringPiece tBuf ( (const char *)sVal, iLen );
		iRes = !!( RE2::PartialMatchN ( tBuf, *m_pRE2, nullptr, 0 ) );
		if ( m_pArg->IsDataPtrAttr () ) SafeDeleteArray ( sVal );
#endif

		return iRes;
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final
	{
		EXPR_CLASS_NAME("Expr_Regex_c");
		uHash ^= m_uFilterHash;
		return CALC_DEP_HASHES();
	}

	ISphExpr* Clone() const final
	{
		return new Expr_Regex_c ( *this );
	}

protected:
	CSphString	m_sRegex;
	uint64_t	m_uFilterHash = SPH_FNV64_SEED;
#if WITH_RE2
	RE2 *		m_pRE2 = nullptr;
#endif

	void SetupRE2()
	{
#if WITH_RE2
		SafeDelete(m_pRE2);

		re2::StringPiece tBuf ( m_sRegex.cstr(), m_sRegex.Length() );
		RE2::Options tOpts;
		tOpts.set_encoding ( RE2::Options::Encoding::EncodingUTF8 );
		m_pRE2 = new RE2 ( tBuf, tOpts );
#endif
	}

private:
	Expr_Regex_c ( const Expr_Regex_c & rhs )
		: Expr_ArgVsSet_T ( rhs )
		, m_sRegex ( rhs.m_sRegex )
		, m_uFilterHash ( rhs.m_uFilterHash )
	{
		SetupRE2();
	}
};


//////////////////////////////////////////////////////////////////////////

struct DistanceUnit_t
{
	CSphString	m_dNames[3];
	float		m_fConversion;
};


bool sphGeoDistanceUnit ( const char * szUnit, float & fCoeff )
{
	static DistanceUnit_t dUnits[] = 
	{
		{ { "mi", "miles" },				1609.34f },
		{ { "yd", "yards" },				0.9144f },
		{ { "ft", "feet" },					0.3048f },
		{ { "in", "inch" },					0.0254f },
		{ { "km", "kilometers" },			1000.0f },
		{ { "m", "meters" },				1.0f },
		{ { "cm", "centimeters" },			0.01f },
		{ { "mm", "millimeters" },			0.001f },
		{ { "NM", "nmi", "nauticalmiles" },	1852.0f }
	};

	if ( !szUnit || !*szUnit )
	{
		fCoeff = 1.0f;
		return true;
	}

	for ( const auto & i : dUnits )
		for ( const auto & j : i.m_dNames )
			if ( j==szUnit )
			{
				fCoeff = i.m_fConversion;
				return true;
			}

	return false;
}


CSphVector<int> ExprParser_t::GatherArgTypes ( int iNode )
{
	CSphVector<int> dTypes;
	GatherArgFN ( iNode, [this, &dTypes] ( int i ) { dTypes.Add ( m_dNodes[i].m_iToken ); } );
	return dTypes;
}

CSphVector<int> ExprParser_t::GatherArgNodes ( int iNode )
{
	CSphVector<int> dNodes;
	GatherArgFN ( iNode, [&dNodes] ( int i ) { dNodes.Add ( i ); } );
	return dNodes;
}

void ExprParser_t::GatherArgRetTypes ( int iNode, CSphVector<ESphAttr> & dTypes )
{
	GatherArgFN ( iNode, [this, &dTypes] ( int i ) { dTypes.Add ( m_dNodes[i].m_eRetType ); } );
}

template < typename FN >
void ExprParser_t::GatherArgFN ( int iNode, FN && fnFunctor )
{
	if ( iNode<0 )
		return;

	m_dGatherStack.Resize ( 0 );
	StackNode_t & tInitial = m_dGatherStack.Add();
	const ExprNode_t & tNode = m_dNodes[iNode];
	tInitial.m_iNode = iNode;
	tInitial.m_iLeft = tNode.m_iLeft;
	tInitial.m_iRight = tNode.m_iRight;

	while ( !m_dGatherStack.IsEmpty() )
	{
		StackNode_t & tCur = m_dGatherStack.Last();
		if ( m_dNodes[tCur.m_iNode].m_iToken!=',' )
		{
			fnFunctor ( tCur.m_iNode );
			m_dGatherStack.Pop();
			continue;
		}
		if ( tCur.m_iLeft==-1 && tCur.m_iRight==-1 )
		{
			m_dGatherStack.Pop();
			continue;
		}

		int iChild = -1;
		if ( tCur.m_iLeft>=0 )
			Swap ( iChild, tCur.m_iLeft );
		else if ( tCur.m_iRight>=0 )
			Swap ( iChild, tCur.m_iRight );
		else
			continue;

		assert ( iChild>=0 );
		const ExprNode_t & tChild = m_dNodes[iChild];
		StackNode_t & tNext = m_dGatherStack.Add();
		tNext.m_iNode = iChild;
		tNext.m_iLeft = tChild.m_iLeft;
		tNext.m_iRight = tChild.m_iRight;
	}
}

bool ExprParser_t::CheckForConstSet ( int iArgsNode, int iSkip )
{
	CSphVector<int> dTypes = GatherArgTypes ( iArgsNode );
	return dTypes.Slice ( iSkip ).all_of (
			[] ( int t ) { return t==TOK_CONST_INT || t==TOK_CONST_FLOAT || t==TOK_MAP_ARG; } );
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

	CSphVector<ESphAttr> dTypes;
	GatherArgRetTypes ( iArgsNode, dTypes );

	// force type conversion, where possible
	if ( dTypes[0]==SPH_ATTR_JSON_FIELD )
	{
		auto pConverted = new Expr_JsonFieldConv_c ( dArgs[0] );
		SafeRelease ( dArgs[0] );
		dArgs[0] = pConverted;
	}

	bool bConst = CheckForConstSet ( iArgsNode, 1 );
	ESphAttr eAttrType = m_dNodes[iArgsNode].m_eArgType;
	if ( bConst )
	{
		switch ( eAttrType )
		{
			case SPH_ATTR_INTEGER:	return new Expr_IntervalConst_c<int> ( dArgs );
			case SPH_ATTR_BIGINT:	return new Expr_IntervalConst_c<int64_t> ( dArgs );
			default:				return new Expr_IntervalConst_c<float> ( dArgs );
		}
	} else
	{
		switch ( eAttrType )
		{
			case SPH_ATTR_INTEGER:	return new Expr_Interval_c<int> ( dArgs );
			case SPH_ATTR_BIGINT:	return new Expr_Interval_c<int64_t> ( dArgs );
			default:				return new Expr_Interval_c<float> ( dArgs );
		}
	}
#if !_WIN32
	return nullptr;
#endif
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
				case TOK_ATTR_MVA32:		return new Expr_MVAIn_c<DWORD> ( tLeft.m_tLocator, GetNameByLocator(tLeft), tRight.m_pConsts );
				case TOK_ATTR_MVA64:		return new Expr_MVAIn_c<int64_t> ( tLeft.m_tLocator, GetNameByLocator(tLeft), tRight.m_pConsts );
				case TOK_ATTR_STRING:		return new Expr_StrIn_c ( tLeft.m_tLocator, GetNameByLocator(tLeft), tRight.m_pConsts, m_eCollation );
				case TOK_ATTR_JSON:			return new Expr_JsonFieldIn_c ( tRight.m_pConsts, CSphRefcountedPtr<ISphExpr> { CreateTree ( m_dNodes [ iNode ].m_iLeft ) }, m_eCollation );
				case TOK_COLUMNAR_UINT32SET:return CreateExpr_ColumnarMva32In ( GetNameByLocator(tLeft), tRight.m_pConsts );
				case TOK_COLUMNAR_INT64SET:	return CreateExpr_ColumnarMva64In ( GetNameByLocator(tLeft), tRight.m_pConsts );
				case TOK_COLUMNAR_STRING:	return CreateExpr_ColumnarStringIn ( GetNameByLocator(tLeft), tRight.m_pConsts, m_eCollation );

				default:
				{
					CSphRefcountedPtr<ISphExpr> pArg ( CreateTree ( m_dNodes[iNode].m_iLeft ) );
					int iConsts = tRight.m_pConsts->m_eRetType==SPH_ATTR_INTEGER ? tRight.m_pConsts->m_dInts.GetLength() : tRight.m_pConsts->m_dFloats.GetLength();
					bool bBinary = iConsts>128;
					switch ( WidestType ( tLeft.m_eRetType, tRight.m_pConsts->m_eRetType ) )
					{
						case SPH_ATTR_INTEGER:
							if ( bBinary )
								return new Expr_In_c<int,true> ( pArg, tRight.m_pConsts );
							else
								return new Expr_In_c<int,false> ( pArg, tRight.m_pConsts );

						case SPH_ATTR_BIGINT:
							if ( bBinary )
								return new Expr_In_c<int64_t,true> ( pArg, tRight.m_pConsts );
							else
								return new Expr_In_c<int64_t,false> ( pArg, tRight.m_pConsts );

						default:
							if ( bBinary )
								return new Expr_In_c<float,true> ( pArg, tRight.m_pConsts );
							else
								return new Expr_In_c<float,false> ( pArg, tRight.m_pConsts );
					}
				}
			}

		// create IN(arg,uservar)
		case TOK_USERVAR:
		{
			if ( !UservarsAvailable() )
			{
				m_sCreateError.SetSprintf ( "internal error: no uservars hook" );
				return nullptr;
			}

			UservarIntSet_c pUservar = Uservars ( m_dUservars[(int)tRight.m_iConst] );
			if ( !pUservar )
			{
				m_sCreateError.SetSprintf ( "undefined user variable '%s'", m_dUservars[(int)tRight.m_iConst].cstr() );
				return nullptr;
			}

			switch ( tLeft.m_iToken )
			{
				case TOK_ATTR_MVA32:	return new Expr_MVAInU_c<DWORD> ( tLeft.m_tLocator, GetNameByLocator(tLeft), pUservar );
				case TOK_ATTR_MVA64:	return new Expr_MVAInU_c<int64_t> ( tLeft.m_tLocator, GetNameByLocator(tLeft), pUservar );
				case TOK_ATTR_STRING:	return new Expr_StrInU_c ( tLeft.m_tLocator, GetNameByLocator(tLeft), pUservar, m_eCollation );
				case TOK_ATTR_JSON:		return new Expr_JsonFieldIn_c ( pUservar, CSphRefcountedPtr<ISphExpr> { CreateTree ( m_dNodes[iNode].m_iLeft ) }, m_eCollation );
				default:				return new Expr_InUservar_c ( CSphRefcountedPtr<ISphExpr> { CreateTree ( m_dNodes[iNode].m_iLeft ) }, pUservar );
			}
		}

		// oops, unhandled case
		default:
			m_sCreateError = "IN() arguments must be constants (except the 1st one)";
	}
	return nullptr;
}


ISphExpr * ExprParser_t::CreateLengthNode ( const ExprNode_t & tNode, ISphExpr * pLeft )
{
	const ExprNode_t & tLeft = m_dNodes [ tNode.m_iLeft ];
	switch ( tLeft.m_iToken )
	{
		case TOK_FUNC:					
		case TOK_ATTR_STRING:			return new Expr_StrLength_c(pLeft);
		case TOK_ATTR_MVA32:
		case TOK_ATTR_MVA64:			return new Expr_MVALength_c ( tLeft.m_tLocator, m_pSchema->GetAttr(tLeft.m_iLocator).m_sName, tLeft.m_iToken==TOK_ATTR_MVA64 );
		case TOK_COLUMNAR_UINT32SET:	return CreateExpr_ColumnarMva32Length ( m_pSchema->GetAttr(tLeft.m_iLocator).m_sName );
		case TOK_COLUMNAR_INT64SET:		return CreateExpr_ColumnarMva64Length ( m_pSchema->GetAttr(tLeft.m_iLocator).m_sName );
		case TOK_COLUMNAR_STRING:		return CreateExpr_ColumnarStringLength ( m_pSchema->GetAttr(tLeft.m_iLocator).m_sName );
		case TOK_ATTR_JSON:				return new Expr_JsonFieldLength_c ( pLeft );
		default:
			m_sCreateError = "LENGTH() argument must be MVA or JSON or STRING field";
			return nullptr;
	}
}


ISphExpr * ExprParser_t::CreateGeodistNode ( int iArgs )
{
	CSphVector<int> dArgs = GatherArgNodes ( iArgs );
	assert ( dArgs.GetLength()==4 || dArgs.GetLength()==5 );

	float fOut = 1.0f; // result scale, defaults to out=meters
	bool bDeg = false; // arg units, defaults to in=radians
	GeoFunc_e eMethod = GEO_ADAPTIVE; // geodist function to use, defaults to adaptive

	if ( dArgs.GetLength()==5 )
	{
		assert ( m_dNodes [ dArgs[4] ].m_eRetType==SPH_ATTR_MAPARG );

		// FIXME! handle errors in options somehow?
		for ( const auto& t : m_dNodes[dArgs[4]].m_pMapArg->m_dPairs )
		{
			if ( t.m_sKey=="in" )
			{
				if ( t.m_sValue=="deg" || t.m_sValue=="degrees" )
					bDeg = true;
				else if ( t.m_sValue=="rad" || t.m_sValue=="radians" )
					bDeg = false;

			} else if ( t.m_sKey=="out" )
			{
				float fCoeff = 1.0f;
				if ( sphGeoDistanceUnit ( t.m_sValue.cstr(), fCoeff ) )
					fOut = 1.0f / fCoeff;
			} else if ( t.m_sKey=="method" )
			{
				if ( t.m_sValue=="haversine" )
					eMethod = GEO_HAVERSINE;
				else if ( t.m_sValue=="adaptive" )
					eMethod = GEO_ADAPTIVE;
			}
		}
	}

	bool bConst1 = ( IsConst ( &m_dNodes[dArgs[0]] ) && IsConst ( &m_dNodes[dArgs[1]] ) );
	bool bConst2 = ( IsConst ( &m_dNodes[dArgs[2]] ) && IsConst ( &m_dNodes[dArgs[3]] ) );

	if ( bConst1 && bConst2 )
	{
		float t[4];
		for ( int i=0; i<4; i++ )
			t[i] = FloatVal ( &m_dNodes[dArgs[i]] );
		return new Expr_GetConst_c ( fOut*CalcGeodist ( eMethod, bDeg, t[0], t[1], t[2], t[3] ) );
	}

	if ( bConst1 )
	{
		Swap ( dArgs[0], dArgs[2] );
		Swap ( dArgs[1], dArgs[3] );
		Swap ( bConst1, bConst2 );
	}

	if ( bConst2 )
	{
		CSphString sLat = GetNameByLocator(dArgs[0]);
		CSphString sLon = GetNameByLocator(dArgs[1]);

		// constant anchor
		if ( m_dNodes[dArgs[0]].m_iToken==TOK_ATTR_FLOAT && m_dNodes[dArgs[1]].m_iToken==TOK_ATTR_FLOAT )
		{
			// attr point
			return new Expr_GeodistAttrConst_c ( GetGeodistFn ( eMethod, bDeg ), fOut,
				m_dNodes[dArgs[0]].m_tLocator, m_dNodes[dArgs[1]].m_tLocator,
				FloatVal ( &m_dNodes[dArgs[2]] ), FloatVal ( &m_dNodes[dArgs[3]] ),
				sLat, sLon );
		} else
		{
			CSphRefcountedPtr<ISphExpr> pAttr0 { ConvertExprJson ( CreateTree ( dArgs[0] ) ) };
			CSphRefcountedPtr<ISphExpr> pAttr1 { ConvertExprJson ( CreateTree ( dArgs[1] ) ) };

			// expr point
			return new Expr_GeodistConst_c ( GetGeodistFn ( eMethod, bDeg ), fOut,
				pAttr0, pAttr1,
				FloatVal ( &m_dNodes[dArgs[2]] ), FloatVal ( &m_dNodes[dArgs[3]] ),
				sLat, sLon );
		}
	}

	// four expressions
	VecRefPtrs_t<ISphExpr*> dExpr;
	MoveToArgList ( CreateTree ( iArgs ), dExpr );
	assert ( dExpr.GetLength()==4 );
	ConvertArgsJson ( dExpr );
	return new Expr_Geodist_c ( GetGeodistFn ( eMethod, bDeg ), fOut, dExpr[0], dExpr[1], dExpr[2], dExpr[3] );
}


ISphExpr * ExprParser_t::CreatePFNode ( int iArg )
{
	m_eEvalStage = SPH_EVAL_FINAL;

	DWORD uNodeFactorFlags = SPH_FACTOR_ENABLE | SPH_FACTOR_CALC_ATC;

	CSphVector<int> dArgs = GatherArgNodes ( iArg );
	assert ( dArgs.GetLength()==0 || dArgs.GetLength()==1 );

	bool bNoATC = false;
	bool bJsonOut = false;

	if ( dArgs.GetLength()==1 )
	{
		assert ( m_dNodes[dArgs[0]].m_eRetType==SPH_ATTR_MAPARG );

		for ( const auto& dOpt : m_dNodes[dArgs[0]].m_pMapArg->m_dPairs )
		{
			if ( dOpt.m_sKey=="no_atc" && dOpt.m_iValue>0)
				bNoATC = true;
			else if ( dOpt.m_sKey=="json" && dOpt.m_iValue>0 )
				bJsonOut = true;
		}
	}

	if ( bNoATC )
		uNodeFactorFlags &= ~SPH_FACTOR_CALC_ATC;
	if ( bJsonOut )
		uNodeFactorFlags |= SPH_FACTOR_JSON_OUT;

	m_uPackedFactorFlags |= uNodeFactorFlags;

	return new Expr_GetPackedFactors_c();
}



ISphExpr * ExprParser_t::CreateBitdotNode ( int iArgsNode, CSphVector<ISphExpr *> & dArgs )
{
	assert ( dArgs.GetLength()>=1 );

	ESphAttr eAttrType = m_dNodes[iArgsNode].m_eRetType;
	switch ( eAttrType )
	{
		case SPH_ATTR_INTEGER:	return new Expr_Bitdot_c<int> ( dArgs );
		case SPH_ATTR_BIGINT:	return new Expr_Bitdot_c<int64_t> ( dArgs );
		default:				return new Expr_Bitdot_c<float> ( dArgs );
	}
}


ISphExpr * ExprParser_t::CreateAggregateNode ( const ExprNode_t & tNode, ESphAggrFunc eFunc, ISphExpr * pLeft )
{
	const ExprNode_t & tLeft = m_dNodes [ tNode.m_iLeft ];
	switch ( tLeft.m_iToken )
	{
		case TOK_ATTR_JSON:			return new Expr_JsonFieldAggr_c ( pLeft, eFunc );
		case TOK_ATTR_MVA32:		return new Expr_MVAAggr_c<DWORD> ( tLeft.m_tLocator, GetNameByLocator(tLeft), eFunc );
		case TOK_ATTR_MVA64:		return new Expr_MVAAggr_c<int64_t> ( tLeft.m_tLocator, GetNameByLocator(tLeft), eFunc );
		case TOK_COLUMNAR_UINT32SET:return CreateExpr_ColumnarMva32Aggr ( pLeft, eFunc );
		case TOK_COLUMNAR_INT64SET:	return CreateExpr_ColumnarMva64Aggr ( pLeft, eFunc );
		default:					return nullptr;
	}
}


void ExprParser_t::FixupIterators ( int iNode, const char * sKey, SphAttr_t * pAttr )
{
	if ( iNode==-1 )
		return;

	ExprNode_t & tNode = m_dNodes[iNode];

	if ( tNode.m_iToken==TOK_IDENT && !strcmp ( sKey, tNode.m_sIdent ) )
	{
		tNode.m_iToken = TOK_ITERATOR;
		tNode.m_pAttr = pAttr;
	}

	FixupIterators ( tNode.m_iLeft, sKey, pAttr );
	FixupIterators ( tNode.m_iRight, sKey, pAttr );
}


ISphExpr * ExprParser_t::CreateForInNode ( int iNode )
{
	ExprNode_t & tNode = m_dNodes[iNode];

	int iFunc = tNode.m_iFunc;
	int iExprNode = tNode.m_iLeft;
	int iNameNode = tNode.m_iRight;
	int iDataNode = m_dNodes[iNameNode].m_iLeft;

	CSphRefcountedPtr<ISphExpr> pArg { CreateTree ( iDataNode )};
	bool bConverted = false;
	if ( !pArg->IsJson ( bConverted ) )
	{
		m_sCreateError.SetSprintf ( "%s() argument must be JSON", FuncNameByHash ( iFunc ) );
		return nullptr;
	}

	auto * pFunc = new Expr_ForIn_c ( pArg, iFunc==FUNC_ALL, iFunc==FUNC_INDEXOF );

	FixupIterators ( iExprNode, m_dNodes[iNameNode].m_sIdent, pFunc->GetRef() );
	pFunc->SetExpr ( CSphRefcountedPtr<ISphExpr> { CreateTree ( iExprNode ) } );

	return pFunc;
}


ISphExpr * ExprParser_t::CreateExprDateAdd ( int iNode, bool bAdd )
{
	ExprNode_t & tNode = m_dNodes[iNode];

	int iExprNode1 = tNode.m_iLeft;
	int iExprNode2 = tNode.m_iRight;

	CSphRefcountedPtr<ISphExpr> pExpr1 { CreateTree ( iExprNode1 ) };
	CSphRefcountedPtr<ISphExpr> pExpr2 { CreateTree ( iExprNode2 ) };

	// This is a hack ala REMAP
	int iUnit = m_dNodes [ iNode-1 ].m_iConst;
	return ::CreateExprDateAdd ( pExpr1, pExpr2, TimeUnit_e(iUnit), bAdd );
}


ISphExpr * ExprParser_t::CreateRegexNode ( ISphExpr * pAttr, ISphExpr * pString )
{
	auto pExpr = new Expr_Regex_c ( pAttr, pString );
#if WITH_RE2
	auto* pRe2 = pExpr->GetRE2();
	if ( !pRe2->ok() )
	{
		m_sCreateError.SetSprintf ( "RE2: error parsing '%s': %s", pRe2->pattern().c_str(), pRe2->error().c_str() );
		SafeDelete ( pExpr );
	}
#endif
	return pExpr;
}


ISphExpr * ExprParser_t::CreateConcatNode ( int iArgsNode, CSphVector<ISphExpr *> & dArgs )
{
	for ( auto & i : dArgs )
		if ( !CheckStoredArg(i) )
			return nullptr;

	CSphVector<ESphAttr> dTypes;
	GatherArgRetTypes ( iArgsNode, dTypes );

	ARRAY_FOREACH ( i, dTypes )
		if ( dTypes[i]!=SPH_ATTR_STRING && dTypes[i]!=SPH_ATTR_STRINGPTR )
		{
			m_sCreateError.SetSprintf ( "all CONCAT() arguments must be strings (arg %d is not)", i+1 );
			return nullptr;
		}


	CSphVector<bool> dConstStr;
	GatherArgFN ( iArgsNode, [this, &dConstStr] (int i) { dConstStr.Add ( m_dNodes[i].m_iToken==TOK_CONST_STRING );});

	return new Expr_Concat_c ( dArgs, dConstStr );
}


//////////////////////////////////////////////////////////////////////////
#define YY_DECL inline int yy1lex ( YYSTYPE * lvalp, void * yyscanner, ExprParser_t * pParser )

#include "flexsphinxexpr.c"

#ifndef NDEBUG
// using a proxy to be possible to debug inside yylex
inline int yylex ( YYSTYPE * lvalp, ExprParser_t * pParser )
{
	int res = yy1lex ( lvalp, pParser->m_pScanner, pParser );
	return res;
}
#else
inline int yylex ( YYSTYPE * lvalp, ExprParser_t * pParser )
{
	return yy1lex ( lvalp, pParser->m_pScanner, pParser );
}
#endif

void yyerror ( ExprParser_t * pParser, const char * sMessage )
{
	// flex put a zero at last token boundary; make it undo that
	const auto* szToken = yy1lex_unhold ( pParser->m_pScanner );
	pParser->m_sParserError.SetSprintf ( "P09: %s near '%s'", sMessage, szToken );
}

#include "bissphinxexpr.c"

//////////////////////////////////////////////////////////////////////////

ExprParser_t::~ExprParser_t ()
{
	// i kinda own those things
	ARRAY_FOREACH ( i, m_dNodes )
	{
		if ( m_dNodes[i].m_iToken==TOK_CONST_LIST )
			SafeDelete ( m_dNodes[i].m_pConsts );
		if ( m_dNodes[i].m_iToken==TOK_MAP_ARG )
			SafeDelete ( m_dNodes[i].m_pMapArg );
	}

	// free any UDF calls that weren't taken over
	ARRAY_FOREACH ( i, m_dUdfCalls )
		SafeDelete ( m_dUdfCalls[i] );

	// free temp map arguments storage
	ARRAY_FOREACH ( i, m_dIdents )
		SafeDeleteArray ( m_dIdents[i] );
}

ESphAttr ExprParser_t::GetWidestRet ( int iLeft, int iRight )
{
	ESphAttr uLeftType = ( iLeft<0 ) ? SPH_ATTR_INTEGER : m_dNodes[iLeft].m_eRetType;
	ESphAttr uRightType = ( iRight<0 ) ? SPH_ATTR_INTEGER : m_dNodes[iRight].m_eRetType;

	if ( uLeftType==SPH_ATTR_INTEGER && uRightType==SPH_ATTR_INTEGER )
		return SPH_ATTR_INTEGER;

	if ( IsInt ( uLeftType ) && IsInt ( uRightType ) )
		return SPH_ATTR_BIGINT;

	// if json vs numeric then return numeric type (for the autoconversion)
	if ( uLeftType==SPH_ATTR_JSON_FIELD && IsNumeric ( uRightType ) )
		return uRightType;

	if ( uRightType==SPH_ATTR_JSON_FIELD && IsNumeric ( uLeftType ) )
		return uLeftType;

	return SPH_ATTR_FLOAT;
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
		|| iTokenType==TOK_ATTR_FACTORS || iTokenType==TOK_ATTR_JSON );

	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = iTokenType;
	sphUnpackAttrLocator ( uAttrLocator, &tNode );

	bool bPtrAttr = tNode.m_tLocator.m_iBlobAttrId<0;

	switch ( iTokenType )
	{
	case TOK_ATTR_FLOAT:	tNode.m_eRetType = SPH_ATTR_FLOAT;			break;
	case TOK_ATTR_MVA32:	tNode.m_eRetType = bPtrAttr ? SPH_ATTR_UINT32SET_PTR : SPH_ATTR_UINT32SET;	break;
	case TOK_ATTR_MVA64:	tNode.m_eRetType = bPtrAttr ? SPH_ATTR_INT64SET_PTR : SPH_ATTR_INT64SET;	break;
	case TOK_ATTR_STRING:	tNode.m_eRetType = SPH_ATTR_STRING;			break;
	case TOK_ATTR_FACTORS:	tNode.m_eRetType = SPH_ATTR_FACTORS;		break;
	case TOK_ATTR_JSON:		tNode.m_eRetType = SPH_ATTR_JSON_FIELD;		break;
	default:
		tNode.m_eRetType = ( tNode.m_tLocator.m_iBitCount>32 ) ? SPH_ATTR_BIGINT : SPH_ATTR_INTEGER;
	}
	return m_dNodes.GetLength()-1;
}


int ExprParser_t::AddNodeColumnar ( int iTokenType, uint64_t uAttrLocator )
{
	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = iTokenType;
	tNode.m_iLocator = (int)uAttrLocator;

	switch ( iTokenType )
	{
	case TOK_COLUMNAR_INT:			tNode.m_eRetType = SPH_ATTR_INTEGER; break;
	case TOK_COLUMNAR_BOOL:			tNode.m_eRetType = SPH_ATTR_BOOL; break;
	case TOK_COLUMNAR_TIMESTAMP:	tNode.m_eRetType = SPH_ATTR_TIMESTAMP; break;
	case TOK_COLUMNAR_FLOAT:		tNode.m_eRetType = SPH_ATTR_FLOAT; break;
	case TOK_COLUMNAR_BIGINT:		tNode.m_eRetType = SPH_ATTR_BIGINT; break;
	case TOK_COLUMNAR_STRING:		tNode.m_eRetType = SPH_ATTR_STRINGPTR; break;
	case TOK_COLUMNAR_UINT32SET:	tNode.m_eRetType = SPH_ATTR_UINT32SET_PTR; break;
	case TOK_COLUMNAR_INT64SET:		tNode.m_eRetType = SPH_ATTR_INT64SET_PTR; break;
	case TOK_COLUMNAR_FLOATVEC:		tNode.m_eRetType = SPH_ATTR_FLOAT_VECTOR_PTR; break;
	default:
		assert ( 0 && "Unsupported columnar type" );
		break;
	}
	return m_dNodes.GetLength()-1;
}



int ExprParser_t::AddNodeField ( int iTokenType, uint64_t uAttrLocator )
{
	assert ( iTokenType==TOK_FIELD );

	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = iTokenType;
	tNode.m_iLocator = (int)uAttrLocator;
	tNode.m_eRetType = SPH_ATTR_STRINGPTR;
	return m_dNodes.GetLength()-1;
}


int ExprParser_t::AddNodeWeight ()
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_WEIGHT;
	tNode.m_eRetType = SPH_ATTR_BIGINT;
	return m_dNodes.GetLength()-1;
}


int ExprParser_t::AddNodeOp ( int iOp, int iLeft, int iRight )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = iOp;

	// deduce type
	tNode.m_eRetType = SPH_ATTR_FLOAT; // default to float
	switch (iOp)
	{
	case TOK_NEG: // NEG just inherits the type
		tNode.m_eArgType = m_dNodes[iLeft].m_eRetType;
		tNode.m_eRetType = tNode.m_eArgType;
		break;

	case TOK_NOT: // NOT result is integer, and its argument must be integer
		tNode.m_eArgType = m_dNodes[iLeft].m_eRetType;
		tNode.m_eRetType = SPH_ATTR_INTEGER;
		if ( !IsInt ( tNode.m_eArgType ) )
		{
			m_sParserError.SetSprintf ( "NOT argument must be integer" );
			return -1;
		}
		break;

	case '&': case '|':
		tNode.m_eArgType = GetWidestRet ( iLeft, iRight );
		tNode.m_eRetType = tNode.m_eArgType;

		if ( !IsInt ( tNode.m_eArgType ) ) // bitwise AND/OR can only be over ints
		{
			m_sParserError.SetSprintf ( "%s arguments must be integer", ( iOp=='&' ) ? "&" : "|" );
			return -1;
		}
		break;

	case TOK_LTE: case TOK_GTE: case TOK_EQ: case TOK_NE:
	case '<': case '>': case TOK_IS_NULL: case TOK_IS_NOT_NULL:
		tNode.m_eArgType = GetWidestRet ( iLeft, iRight );
		tNode.m_eRetType = SPH_ATTR_INTEGER;
		break;

	case TOK_AND: case TOK_OR:
		tNode.m_eArgType = GetWidestRet ( iLeft, iRight );
		tNode.m_eRetType = SPH_ATTR_INTEGER;

		if ( !IsInt ( tNode.m_eArgType )) // logical AND/OR can only be over ints
		{
			m_sParserError.SetSprintf ( "%s arguments must be integer", ( iOp==TOK_AND ) ? "AND" : "OR" );
			return -1;
		}
		break;

	case '+': case '-': case '*': case ',':
		tNode.m_eArgType = GetWidestRet ( iLeft, iRight );
		tNode.m_eRetType = tNode.m_eArgType;
		break;

	case '%':
		tNode.m_eArgType = GetWidestRet ( iLeft, iRight );
		tNode.m_eRetType = tNode.m_eArgType;

		// MOD can only be over ints
		if ( !IsInt ( tNode.m_eArgType ) )
		{
			m_sParserError.SetSprintf ( "MOD arguments must be integer" );
			return -1;
		}
		break;
	default:
		// check for unknown op
		assert ( iOp=='/' && "unknown op in AddNodeOp() type deducer" );
	}

	tNode.m_iArgs = 0;
	if ( iOp==',' )
	{
		if ( iLeft>=0 )		tNode.m_iArgs += ( m_dNodes[iLeft].m_iToken==',' ) ? m_dNodes[iLeft].m_iArgs : 1;
		if ( iRight>=0 )	tNode.m_iArgs += ( m_dNodes[iRight].m_iToken==',' ) ? m_dNodes[iRight].m_iArgs : 1;
	}

	// argument type conversion for functions like INDEXOF(), ALL() and ANY()
	// we need no conversion for operands of comma!
	if ( iOp!=',' && iLeft>=0 && iRight>=0 )
	{
		if ( m_dNodes[iRight].m_eRetType==SPH_ATTR_STRING && m_dNodes[iLeft].m_iToken==TOK_IDENT )
			m_dNodes[iLeft].m_eRetType = SPH_ATTR_STRING;
		else if ( m_dNodes[iLeft].m_eRetType==SPH_ATTR_STRING && m_dNodes[iRight].m_iToken==TOK_IDENT )
			m_dNodes[iRight].m_eRetType = SPH_ATTR_STRING;
	}

	tNode.m_iLeft = iLeft;
	tNode.m_iRight = iRight;
	return m_dNodes.GetLength()-1;
}

// functions without args
int ExprParser_t::AddNodeFunc0 ( int iFunc )
{
	// regular case, iFirst is entire arglist, iSecond is -1
	// special case for IN(), iFirst is arg, iSecond is constlist
	// special case for REMAP(), iFirst and iSecond are expressions, iThird and iFourth are constlists
	assert ( iFunc>=0 && iFunc<int ( sizeof ( g_dFuncs ) / sizeof ( g_dFuncs[0] ) ) );
//	assert ( g_dFuncs[iFunc].m_eFunc==(Tokh_e ) iFunc );

	// check args count
	int iExpectedArgc = g_dFuncs[iFunc].m_iArgs;
	if ( iExpectedArgc )
	{
		m_sParserError.SetSprintf ( "%s() called without args, %d args expected", FuncNameByHash(iFunc), iExpectedArgc );
		return -1;
	}
	// do add
	ExprNode_t &tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = iFunc;
	tNode.m_iLeft = -1;
	tNode.m_iRight = -1;
	tNode.m_eArgType = SPH_ATTR_INTEGER;
	tNode.m_eRetType = g_dFuncs[iFunc].m_eRet;

	// all ok
	assert ( tNode.m_eRetType!=SPH_ATTR_NONE );
	return m_dNodes.GetLength () - 1;
}

// functions with 1 arg
int ExprParser_t::AddNodeFunc ( int iFunc, int iArg )
{
	// regular case, iFirst is entire arglist, iSecond is -1
	// special case for IN(), iFirst is arg, iSecond is constlist
	// special case for REMAP(), iFirst and iSecond are expressions, iThird and iFourth are constlists
	assert ( iFunc>=0 && iFunc< int ( sizeof ( g_dFuncs )/sizeof ( g_dFuncs[0]) ) );
	auto eFunc = (Tokh_e)iFunc;
//	assert ( g_dFuncs [ iFunc ].m_eFunc==eFunc );
	const char * sFuncName = FuncNameByHash ( iFunc );

	// check args count
	int iExpectedArgc = g_dFuncs [ iFunc ].m_iArgs;
	int iArgc = 0;
	if ( iArg>=0 )
		iArgc = ( m_dNodes [ iArg ].m_iToken==',' ) ? m_dNodes [ iArg ].m_iArgs : 1;

	if ( iExpectedArgc<0 )
	{
		if ( iArgc < -iExpectedArgc ) // space placed to avoid confusing ligature <-
		{
			m_sParserError.SetSprintf ( "%s() called with %d args, at least %d args expected", sFuncName, iArgc, -iExpectedArgc );
			return -1;
		}
	} else if ( iArgc!=iExpectedArgc )
	{
		m_sParserError.SetSprintf ( "%s() called with %d args, %d args expected", sFuncName, iArgc, iExpectedArgc );
		return -1;
	}

	// check arg types
	//
	// check for string args
	// most builtin functions take numeric args only
	bool bGotString = false, bGotMva = false;
	CSphVector<ESphAttr> dRetTypes;
	GatherArgRetTypes ( iArg, dRetTypes );
	for ( ESphAttr eRetType: dRetTypes )
	{
		switch ( eRetType )
		{
			case SPH_ATTR_UINT32SET: case SPH_ATTR_INT64SET: case SPH_ATTR_UINT32SET_PTR: case SPH_ATTR_INT64SET_PTR: bGotMva = true; break;
			case SPH_ATTR_STRING : bGotString = true;
			default:;
		}
	}

	if ( bGotString )
	{
		switch ( eFunc )
		{
			default: m_sParserError.SetSprintf ( "%s() arguments can not be string", sFuncName ); return -1;
			case FUNC_LENGTH: case FUNC_TO_STRING: case FUNC_CONCAT: case FUNC_SUBSTRING_INDEX: case FUNC_UPPER: case FUNC_LOWER: case FUNC_CRC32:
			case FUNC_EXIST: case FUNC_POLY2D: case FUNC_GEOPOLY2D: case FUNC_REGEX: case FUNC_LEVENSHTEIN: case FUNC_DATE_FORMAT: case FUNC_BIGINT:
			case FUNC_DAY: case FUNC_MONTH: case FUNC_YEAR: case FUNC_YEARMONTH: case FUNC_YEARMONTHDAY:
			case FUNC_SECOND: case FUNC_MINUTE: case FUNC_HOUR:
			break;
		}
	}

	if ( bGotMva )
	{
		switch ( eFunc )
		{
			default: m_sParserError.SetSprintf ( "%s() arguments can not be MVA", sFuncName ); return -1;
			case FUNC_TO_STRING: case FUNC_LENGTH: case FUNC_LEAST: case FUNC_GREATEST:;
		}
	}

	auto & dArg = m_dNodes[iArg];

	switch ( eFunc )
	{
	case FUNC_BITDOT:
		{    // check that first BITDOT arg is integer or bigint

			int iLeftmost = iArg;
			while ( m_dNodes[iLeftmost].m_iToken==',' )
				iLeftmost = m_dNodes[iLeftmost].m_iLeft;

			ESphAttr eArg = m_dNodes[iLeftmost].m_eRetType;
			if ( !IsInt ( eArg ) )
			{
				m_sParserError.SetSprintf ( "first %s() argument must be integer", sFuncName );
				return -1;
			}
		}
		break;
	case FUNC_EXIST:
		{
			ESphAttr eLeft = m_dNodes[dArg.m_iLeft].m_eRetType, eRight = m_dNodes[dArg.m_iRight].m_eRetType;
			bool bIsLeftGood = ( eLeft==SPH_ATTR_STRING );
			bool bIsRightGood = ( eRight==SPH_ATTR_INTEGER || eRight==SPH_ATTR_TIMESTAMP || eRight==SPH_ATTR_BOOL || eRight==SPH_ATTR_FLOAT || eRight==SPH_ATTR_BIGINT );
			if ( !bIsLeftGood || !bIsRightGood )
			{
				if ( bIsRightGood )
					m_sParserError.SetSprintf ( "first %s() argument must be string", sFuncName );
				else
					m_sParserError.SetSprintf ( "ill-formed %s", sFuncName );
				return -1;
			}
		}
		break;

	case FUNC_SINT: 		// check that first SINT or timestamp family arg is integer or timestamp
	case FUNC_FIBONACCI:
	case FUNC_DAY:
	case FUNC_MONTH:
	case FUNC_YEAR:
	case FUNC_YEARMONTH:
	case FUNC_YEARMONTHDAY:
	case FUNC_HOUR:
	case FUNC_MINUTE:
	case FUNC_SECOND:
		assert ( iArg >= 0 );
		if ( !( dArg.m_eRetType==SPH_ATTR_INTEGER
				 || dArg.m_eRetType==SPH_ATTR_TIMESTAMP
				 || dArg.m_eRetType==SPH_ATTR_BIGINT
				 || dArg.m_eRetType==SPH_ATTR_JSON_FIELD
				 || dArg.m_eRetType==SPH_ATTR_STRING
				 || CanEvalNumbers ( dArg.m_iFunc ) ) )
		{
			m_sParserError.SetSprintf ( "%s() argument must be integer, bigint, timestamp, json, string or evaluated to number", sFuncName );
			return -1;
		}
		break;

	case FUNC_BIGINT:
		assert ( iArg >= 0 );
		if ( !( dArg.m_eRetType == SPH_ATTR_JSON_FIELD
				 || IsFloatLike ( dArg.m_eRetType )
				 || IsNumericLike ( dArg.m_eRetType )
				 || CanEvalNumbers ( dArg.m_iFunc ) ) )
		{
			m_sParserError.SetSprintf ( "%s() argument must be number, or evaluated to number", sFuncName );
			return -1;
		}
		break;

	case FUNC_CONTAINS: // check that CONTAINS args are poly, float, float
		assert ( dRetTypes.GetLength ()==3 );
		if ( dRetTypes[0]!=SPH_ATTR_POLY2D )
		{
			m_sParserError.SetSprintf ( "1st CONTAINS() argument must be a 2D polygon (see POLY2D)" );
			return -1;
		}
		if ( !( IsNumeric ( dRetTypes[1] ) || IsJson ( dRetTypes[1] ) ) || ! ( IsNumeric ( dRetTypes[2] ) || IsJson ( dRetTypes[2] ) ) )
		{
			m_sParserError.SetSprintf ( "2nd and 3rd CONTAINS() arguments must be numeric or JSON" );
			return -1;
		}
		break;
	case FUNC_POLY2D:
	case FUNC_GEOPOLY2D:
		if ( dRetTypes.GetLength ()==1 )
		{
			// handle 1 arg version, POLY2D(string-attr)
			if ( dRetTypes[0]!=SPH_ATTR_STRING && dRetTypes[0]!=SPH_ATTR_STRINGPTR && dRetTypes[0]!=SPH_ATTR_JSON_FIELD )
			{
				m_sParserError.SetSprintf ( "%s() argument must be a string or JSON field attribute", sFuncName );
				return -1;
			}
		} else if ( dRetTypes.GetLength ()<6 )
		{
			// handle 2..5 arg versions, invalid
			m_sParserError.SetSprintf ( "bad %s() argument count, must be either 1 (string) or 6+ (x/y pairs list)"
										, sFuncName );
			return -1;
		} else
		{
			// handle 6+ arg version, POLY2D(xy-list)
			if ( dRetTypes.GetLength () & 1 )
			{
				m_sParserError.SetSprintf ( "bad %s() argument count, must be even", sFuncName );
				return -1;
			}
			ARRAY_FOREACH ( i, dRetTypes )
				if ( !( IsNumeric ( dRetTypes[i] ) || IsJson ( dRetTypes[i] ) ) )
				{
					m_sParserError.SetSprintf ( "%s() argument %d must be numeric or JSON field", sFuncName, 1 + i );
					return -1;
				}
		}
		break;
	case FUNC_BM25F: // check that BM25F args are float, float [, {file_name=weight}]
		if ( dRetTypes.GetLength ()>3 )
		{
			m_sParserError.SetSprintf ( "%s() called with %d args, at most 3 args expected", sFuncName
										, dRetTypes.GetLength () );
			return -1;
		}

		if ( dRetTypes[0]!=SPH_ATTR_FLOAT || dRetTypes[1]!=SPH_ATTR_FLOAT )
		{
			m_sParserError.SetSprintf ( "%s() arguments 1,2 must be numeric", sFuncName );
			return -1;
		}

		if ( dRetTypes.GetLength ()==3 && dRetTypes[2]!=SPH_ATTR_MAPARG )
		{
			m_sParserError.SetSprintf ( "%s() argument 3 must be map", sFuncName );
			return -1;
		}
		break;
	case FUNC_SUBSTRING_INDEX:
		if ( dRetTypes.GetLength()!=3 )
		{
			m_sParserError.SetSprintf ( "%s() called with %d args, but 3 args expected", sFuncName
				, dRetTypes.GetLength () );
			return -1;
		}
		
		if ( dRetTypes[0]!=SPH_ATTR_STRING && dRetTypes[0]!=SPH_ATTR_STRINGPTR && dRetTypes[0]!=SPH_ATTR_JSON && dRetTypes[0]!=SPH_ATTR_JSON_FIELD )
		{
			m_sParserError.SetSprintf ( "%s() argument 1 must be string or json", sFuncName );
			return -1;
		}

		if ( dRetTypes[1]!=SPH_ATTR_STRING )
		{
			m_sParserError.SetSprintf ( "%s() arguments 2 must be string", sFuncName );
			return -1;
		}

		if ( dRetTypes[2]!=SPH_ATTR_INTEGER )
		{
			m_sParserError.SetSprintf ( "%s() arguments 3 must be numeric", sFuncName );
			return -1;
		}
		break;
	case FUNC_UPPER:
	case FUNC_LOWER:
		if ( dRetTypes.GetLength()!=1 )
		{
			m_sParserError.SetSprintf ( "%s() called with %d args, but 1 arg expected", sFuncName, dRetTypes.GetLength () );
			return -1;
		}

		if ( dRetTypes[0]!=SPH_ATTR_STRING && dRetTypes[0]!=SPH_ATTR_STRINGPTR && dRetTypes[0]!=SPH_ATTR_JSON && dRetTypes[0]!=SPH_ATTR_JSON_FIELD )
		{
			m_sParserError.SetSprintf ( "%s() argument 1 must be string or json", sFuncName );
			return -1;
		}
		break;
	case FUNC_GEODIST: // check GEODIST args count, and that optional arg 5 is a map argument
		if ( dRetTypes.GetLength ()>5 )
		{
			m_sParserError.SetSprintf ( "%s() called with %d args, at most 5 args expected", sFuncName
										, dRetTypes.GetLength () );
			return -1;
		}

		if ( dRetTypes.GetLength ()==5 && dRetTypes[4]!=SPH_ATTR_MAPARG )
		{
			m_sParserError.SetSprintf ( "%s() argument 5 must be map", sFuncName );
			return -1;
		}
		break;
	case FUNC_REGEX:
		{
#if WITH_RE2
			ESphAttr eLeft = m_dNodes[dArg.m_iLeft].m_eRetType;
			bool bIsLeftGood = ( eLeft==SPH_ATTR_STRING || eLeft==SPH_ATTR_STRINGPTR || eLeft==SPH_ATTR_JSON_FIELD );
			if ( !bIsLeftGood )
			{
				m_sParserError.SetSprintf ( "first %s() argument must be string or JSON.field", sFuncName );
				return -1;
			}

			ESphAttr eRight = m_dNodes[dArg.m_iRight].m_eRetType;
			bool bIsRightGood = ( eRight==SPH_ATTR_STRING );
			if ( !bIsRightGood )
			{
				m_sParserError.SetSprintf ( "second %s() argument must be string", sFuncName );
				return -1;
			}
#else
			m_sParserError.SetSprintf ( "%s() used but no regexp support compiled", sFuncName );
			return -1;
#endif
		}
		break;
	case FUNC_DATE_FORMAT:
		if ( dRetTypes.GetLength()!=2 )
		{
			m_sParserError.SetSprintf ( "%s() called with %d args, but 2 args expected", sFuncName, dRetTypes.GetLength() );
			return -1;
		}
		if ( m_dNodes[0].m_eRetType!=SPH_ATTR_INTEGER && m_dNodes[0].m_eRetType!=SPH_ATTR_TIMESTAMP && m_dNodes[0].m_eRetType!=SPH_ATTR_BIGINT && m_dNodes[0].m_eRetType!=SPH_ATTR_STRING )
		{
			m_sParserError.SetSprintf ( "%s() argument 1 must be integer, bigint or timestamp, json, string", sFuncName );
			return -1;
		}
		if ( dRetTypes[1]!=SPH_ATTR_STRING )
		{
			m_sParserError.SetSprintf ( "%s() arguments 2 must be string", sFuncName );
			return -1;
		}
		break;

	default:;
	}

	// do add
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = iFunc;
	tNode.m_iLeft = iArg;
	tNode.m_iRight = -1;
	tNode.m_eArgType = ( iArg>=0 ) ? m_dNodes [ iArg ].m_eRetType : SPH_ATTR_INTEGER;
	tNode.m_eRetType = g_dFuncs [ iFunc ].m_eRet;

	// fixup return type in a few special cases
	switch ( eFunc )
	{
	case FUNC_MIN:
	case FUNC_MAX:
	case FUNC_MADD:
	case FUNC_MUL3:
	case FUNC_ABS:
	case FUNC_IDIV:
		if ( IsJson ( tNode.m_eArgType ) ) // auto-converter from JSON field for universal (SPH_ATTR_NONE return type) nodes
			tNode.m_eRetType = SPH_ATTR_BIGINT;
		else
			tNode.m_eRetType = tNode.m_eArgType;
		break;

	case FUNC_EXIST:
		{
			ESphAttr eType = m_dNodes[m_dNodes[iArg].m_iRight].m_eRetType;
			tNode.m_eArgType = eType;
			tNode.m_eRetType = eType;
			break;
		}
	case FUNC_BIGINT:
		if ( tNode.m_eArgType==SPH_ATTR_FLOAT )
			tNode.m_eRetType = SPH_ATTR_FLOAT; // enforce if we can; FIXME! silently ignores BIGINT() on floats; should warn or raise an error
		break;
	case FUNC_IF:
	case FUNC_BITDOT:
		tNode.m_eRetType = tNode.m_eArgType;
		break;

	case FUNC_GREATEST:
	case FUNC_LEAST: // fixup MVA return type according to the leftmost argument
		{
			int iLeftmost = iArg;
			while ( m_dNodes [ iLeftmost ].m_iToken==',' )
				iLeftmost = m_dNodes [ iLeftmost ].m_iLeft;
			ESphAttr eArg = m_dNodes [ iLeftmost ].m_eRetType;
			if ( eArg==SPH_ATTR_INT64SET || eArg==SPH_ATTR_INT64SET_PTR )
				tNode.m_eRetType = SPH_ATTR_BIGINT;
			if ( eArg==SPH_ATTR_UINT32SET || eArg==SPH_ATTR_UINT32SET_PTR )
				tNode.m_eRetType = SPH_ATTR_INTEGER;
		}
		break;
	case FUNC_LEVENSHTEIN:
	{
		if ( dRetTypes.GetLength()<2 )
		{
			m_sParserError.SetSprintf ( "%s() called with %d args, but at least 2 args expected", sFuncName, dRetTypes.GetLength () );
			return -1;
		}
		
		if ( dRetTypes[0]!=SPH_ATTR_STRING && dRetTypes[0]!=SPH_ATTR_STRINGPTR && dRetTypes[0]!=SPH_ATTR_JSON_FIELD )
		{
			m_sParserError.SetSprintf ( "%s() arguments 1 must be string", sFuncName );
			return -1;
		}

		if ( dRetTypes[1]!=SPH_ATTR_STRING && dRetTypes[1]!=SPH_ATTR_STRINGPTR && dRetTypes[1]!=SPH_ATTR_JSON_FIELD )
		{
			m_sParserError.SetSprintf ( "%s() arguments 2 must be string", sFuncName );
			return -1;
		}

		LevenshteinOptions_t tOpts;
		if ( dRetTypes.GetLength()>2 )
		{
			if ( dRetTypes[2]!=SPH_ATTR_MAPARG )
			{
				m_sParserError.SetSprintf ( "%s() arguments 3 must be a map", sFuncName );
				return -1;
			}

			CSphVector<int> dArgs = GatherArgNodes ( iArg );
			assert ( dArgs.GetLength()==dRetTypes.GetLength() );

			const CSphVector<CSphNamedVariant> & dOpts = m_dNodes[dArgs[2]].m_pMapArg->m_dPairs;
			tOpts = GetOptions ( dOpts.Begin(), dOpts.GetLength() );
		}

		tNode.m_eRetType = ( tOpts.m_bNormalize ? SPH_ATTR_FLOAT : SPH_ATTR_INTEGER );

		break;
	}

	case FUNC_RANGE:
	case FUNC_DATE_RANGE:
	{
		if ( dRetTypes.GetLength()<2 )
		{
			m_sParserError.SetSprintf ( "%s() called with %d args, but at least 2 args expected", sFuncName, dRetTypes.GetLength () );
			return -1;
		}

		if ( dRetTypes[0]!=SPH_ATTR_INTEGER && dRetTypes[0]!=SPH_ATTR_BIGINT && dRetTypes[0]!=SPH_ATTR_FLOAT && dRetTypes[0]!=SPH_ATTR_TIMESTAMP )
		{
			m_sParserError.SetSprintf ( "%s() argument 1 must be number, or evaluated to number", sFuncName );
			return -1;
		}

		GatherArgFN ( iArg, [&] ( int i )
		{
			if ( i!=0 && m_dNodes[i].m_eRetType!=SPH_ATTR_MAPARG )
				m_sParserError.SetSprintf ( "%s() argument %d must be a map", sFuncName, i+1 );
		} );
	}
	break;

	case FUNC_HISTOGRAM:
	case FUNC_DATE_HISTOGRAM:
	{
		if ( dRetTypes.GetLength()!=2 )
		{
			m_sParserError.SetSprintf ( "%s() called with %d args, but at 2 args expected", sFuncName, dRetTypes.GetLength () );
			return -1;
		}

		if ( dRetTypes[0]!=SPH_ATTR_INTEGER && dRetTypes[0]!=SPH_ATTR_BIGINT && dRetTypes[0]!=SPH_ATTR_FLOAT && dRetTypes[0]!=SPH_ATTR_TIMESTAMP )
		{
			m_sParserError.SetSprintf ( "%s() argument 1 must be number, or evaluated to number", sFuncName );
			return -1;
		}

		if ( dRetTypes[1]!=SPH_ATTR_MAPARG )
		{
			m_sParserError.SetSprintf ( "%s() argument 2 must be map", sFuncName );
			return -1;
		}
	}
	break;

	default:;
	}

	// all ok
	assert ( tNode.m_eRetType!=SPH_ATTR_NONE );
	return m_dNodes.GetLength()-1;
}

// special branch for all/any/indexof ( expr for x in arglist )
int ExprParser_t::AddNodeFor ( int iFunc, int iExpr, int iLoop )
{
	assert ( iFunc>=0 && iFunc<int ( sizeof ( g_dFuncs ) / sizeof ( g_dFuncs[0] ) ) );
//	assert ( g_dFuncs [ iFunc ].m_eFunc==eFunc );
	const char * sFuncName = FuncNameByHash ( iFunc );

	// check args count
	if ( iLoop<0 )
	{
		int iArgc = 0;
		if ( iExpr>=0 )
			iArgc = ( m_dNodes[iExpr].m_iToken==',' ) ? m_dNodes[iExpr].m_iArgs : 1;

		m_sParserError.SetSprintf ( "%s() called with %d args, at least 1 args expected", sFuncName, iArgc );
		return -1;
	}
	// do add
	ExprNode_t &tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = iFunc;
	tNode.m_iLeft = iExpr;
	tNode.m_iRight = iLoop;
	tNode.m_eArgType = ( iExpr>=0 ) ? m_dNodes[iExpr].m_eRetType : SPH_ATTR_INTEGER;
	tNode.m_eRetType = g_dFuncs[iFunc].m_eRet;

	// all ok
	assert ( tNode.m_eRetType!=SPH_ATTR_NONE );
	return m_dNodes.GetLength () - 1;
}

int ExprParser_t::AddNodeDate ( int iFunc, int iExpr1, int iExpr2, int iUnit )
{
	assert ( iFunc>=0 && iFunc<int ( sizeof ( g_dFuncs ) / sizeof ( g_dFuncs[0] ) ) );

	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = iFunc;
	tNode.m_iLeft = iExpr1;
	tNode.m_iRight = iExpr2;
	tNode.m_eArgType = SPH_ATTR_INTEGER;
	tNode.m_eRetType = g_dFuncs[iFunc].m_eRet;

	// all ok
	assert ( tNode.m_eRetType!=SPH_ATTR_NONE );
	return m_dNodes.GetLength () - 1;
}

int ExprParser_t::AddNodeIn ( int iArg, int iList )
{
//	assert ( g_dFuncs[FUNC_IN].m_eFunc==FUNC_IN );

	// check args count
	if ( iList<0 )
	{
		m_sParserError.SetSprintf ( "in() called with <2 args, at least 2 args expected" );
		return -1;
	}

	// do add
	ExprNode_t &tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = FUNC_IN;
	tNode.m_iLeft = iArg;
	tNode.m_iRight = iList;
	tNode.m_eArgType = ( iArg>=0 ) ? m_dNodes[iArg].m_eRetType : SPH_ATTR_INTEGER;
	tNode.m_eRetType = g_dFuncs[FUNC_IN].m_eRet;

	// all ok
	assert ( tNode.m_eRetType!=SPH_ATTR_NONE );
	return m_dNodes.GetLength () - 1;
}

int ExprParser_t::AddNodeRemap ( int iExpr1, int iExpr2, int iList1, int iList2 )
{
	//assert ( g_dFuncs[FUNC_REMAP].m_eFunc==FUNC_REMAP );

	if ( m_dNodes[iExpr1].m_iToken==TOK_IDENT )
	{
		m_sParserError.SetSprintf ( "remap() incorrect first argument (not integer?)" );
		return 1;
	}
	if ( m_dNodes[iExpr2].m_iToken==TOK_IDENT )
	{
		m_sParserError.SetSprintf ( "remap() incorrect second argument (not integer/float?)" );
		return 1;
	}

	if ( !IsInt ( m_dNodes[iExpr1].m_eRetType ) )
	{
		m_sParserError.SetSprintf ( "remap() first argument should result in integer value" );
		return -1;
	}

	ESphAttr eSecondRet = m_dNodes[iExpr2].m_eRetType;
	if ( !IsNumeric ( eSecondRet ) )
	{
		m_sParserError.SetSprintf ( "remap() second argument should result in integer or float value" );
		return -1;
	}

	ConstList_c &tFirstList = *m_dNodes[iList1].m_pConsts;
	ConstList_c &tSecondList = *m_dNodes[iList2].m_pConsts;
	if ( tFirstList.m_dInts.GetLength ()==0 )
	{
		m_sParserError.SetSprintf ( "remap() first constlist should consist of integer values" );
		return -1;
	}
	if ( tFirstList.m_dInts.GetLength ()!=tSecondList.m_dInts.GetLength () &&
		tFirstList.m_dInts.GetLength ()!=tSecondList.m_dFloats.GetLength () )
	{
		m_sParserError.SetSprintf ( "remap() both constlists should have the same length" );
		return -1;
	}

	if ( eSecondRet==SPH_ATTR_FLOAT && tSecondList.m_dFloats.GetLength ()==0 )
	{
		m_sParserError.SetSprintf ( "remap() second argument results in float value and thus fourth argument should be a list of floats" );
		return -1;
	}
	if ( eSecondRet!=SPH_ATTR_FLOAT && tSecondList.m_dInts.GetLength ()==0 )
	{
		m_sParserError.SetSprintf ("remap() second argument results in integer value and thus fourth argument should be a list of integers" );
		return -1;
	}

	// do add
	ExprNode_t &tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = FUNC_REMAP;
	tNode.m_iLeft = iExpr1;
	tNode.m_iRight = iExpr2;
	tNode.m_eArgType = m_dNodes[iExpr1].m_eRetType;
	tNode.m_eRetType = m_dNodes[iExpr2].m_eRetType;
	return m_dNodes.GetLength () - 1;
}

// functions RAND with 0 or 1 arg
int ExprParser_t::AddNodeRand ( int iArg )
{
//	assert ( g_dFuncs[FUNC_RAND].m_eFunc==FUNC_RAND );

	if ( iArg>=0 )
	{
		if ( !IsNumeric ( m_dNodes[iArg].m_eRetType ) )
		{
			m_sParserError.SetSprintf ( "rand() argument must be numeric" );
			return -1;
		}
		int iArgc = ( m_dNodes[iArg].m_iToken==',' ) ? m_dNodes[iArg].m_iArgs : 1;
		if ( iArgc>1 )
		{
			m_sParserError.SetSprintf ( "rand() called with %d args, either 0 or 1 args expected", iArgc );
			return -1;
		}
	}

	// do add
	ExprNode_t &tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = FUNC_RAND;
	tNode.m_iLeft = iArg;
	tNode.m_iRight = -1;
	tNode.m_eArgType = ( iArg>=0 ) ? m_dNodes[iArg].m_eRetType : SPH_ATTR_INTEGER;
	tNode.m_eRetType = g_dFuncs[FUNC_RAND].m_eRet;

	// all ok
	assert ( tNode.m_eRetType!=SPH_ATTR_NONE );
	return m_dNodes.GetLength () - 1;
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
				const ExprNode_t & tNode = m_dNodes[iCur];
				if ( tNode.m_iToken==TOK_FUNC && ( tNode.m_iFunc==FUNC_RANKFACTORS || tNode.m_iFunc==FUNC_FACTORS ) )
					pCall->m_dArgs2Free.Add ( dArgTypes.GetLength() );
				if ( tNode.m_eRetType==SPH_ATTR_JSON || tNode.m_eRetType==SPH_ATTR_JSON_FIELD )
					pCall->m_dArgs2Free.Add ( dArgTypes.GetLength() );
				dArgTypes.Add ( tNode.m_eRetType );
				break;
			}

			int iRight = m_dNodes[iCur].m_iRight;
			if ( iRight>=0 )
			{
				const ExprNode_t & tNode = m_dNodes[iRight];
				assert ( tNode.m_iToken!=',' );
				if ( tNode.m_iToken==TOK_FUNC && ( tNode.m_iFunc==FUNC_RANKFACTORS || tNode.m_iFunc==FUNC_FACTORS) )
					pCall->m_dArgs2Free.Add ( dArgTypes.GetLength() );
				if ( tNode.m_eRetType==SPH_ATTR_JSON || tNode.m_eRetType==SPH_ATTR_JSON_FIELD )
					pCall->m_dArgs2Free.Add ( dArgTypes.GetLength() );
				dArgTypes.Add ( tNode.m_eRetType );
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
				case SPH_ATTR_BOOL:
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
				case SPH_ATTR_UINT32SET_PTR:
					eRes = SPH_UDF_TYPE_UINT32SET;
					break;
				case SPH_ATTR_INT64SET:
				case SPH_ATTR_INT64SET_PTR:
					eRes = SPH_UDF_TYPE_INT64SET;
					break;
				case SPH_ATTR_FACTORS:
					eRes = SPH_UDF_TYPE_FACTORS;
					pCall->m_dArgs2Free.Add ( i );
					break;
				case SPH_ATTR_JSON_FIELD:
					eRes = SPH_UDF_TYPE_JSON;
					break;
				default:
					m_sParserError.SetSprintf ( "internal error: unmapped UDF argument type (arg=%d, type=%u)", i, dArgTypes[i] );
					return -1;
			}
		}

		ARRAY_FOREACH ( i, pCall->m_dArgs2Free )
			pCall->m_dArgs2Free[i] = tArgs.arg_count - 1 - pCall->m_dArgs2Free[i];
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

int	ExprParser_t::AddNodePF ( int iFunc, int iArg )
{
	assert ( iFunc>=0 && iFunc< int ( sizeof ( g_dFuncs )/sizeof ( g_dFuncs[0]) ) );
	const char * sFuncName = FuncNameByHash ( iFunc );

	CSphVector<ESphAttr> dRetTypes;
	GatherArgRetTypes ( iArg, dRetTypes );

	assert ( dRetTypes.GetLength()==0 || dRetTypes.GetLength()==1 );

	if ( dRetTypes.GetLength()==1 && dRetTypes[0]!=SPH_ATTR_MAPARG )
	{
		m_sParserError.SetSprintf ( "%s() argument must be a map", sFuncName );
		return -1;
	}

	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_FUNC;
	tNode.m_iFunc = iFunc;
	tNode.m_iLeft = iArg;
	tNode.m_iRight = -1;
	tNode.m_eArgType = SPH_ATTR_MAPARG;
	tNode.m_eRetType = g_dFuncs[iFunc].m_eRet;

	return m_dNodes.GetLength()-1;
}

int ExprParser_t::AddNodeConstlist ( int64_t iValue, bool bPackedString )
{
	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = TOK_CONST_LIST;
	tNode.m_pConsts = new ConstList_c();
	tNode.m_pConsts->Add ( iValue );
	tNode.m_pConsts->m_sExpr = m_sExpr;
	tNode.m_pConsts->m_bPackedStrings = bPackedString;
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


int ExprParser_t::AddNodeHookFunc ( int iID )
{
	CSphVector<ESphAttr> dArgTypes;
	ESphAttr eRet = m_pHook->GetReturnType ( iID, dArgTypes, true, m_sParserError );
	if ( eRet==SPH_ATTR_NONE )
		return -1;

	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = TOK_HOOK_FUNC;
	tNode.m_iFunc = iID;
	tNode.m_eRetType = eRet;

	return m_dNodes.GetLength()-1;
}


int ExprParser_t::AddNodeMapArg ( const char * szKey, const char * szValue, int64_t iValue, float fValue, VariantType_e eType )
{
	ExprNode_t & tNode = m_dNodes.Add();
	tNode.m_iToken = TOK_MAP_ARG;
	tNode.m_pMapArg = new MapArg_c();

	CSphString sValue;
	if ( szKey )
	{
		if ( eType==VariantType_e::STRING )
		{
			sValue = SqlUnescape ( m_sExpr.first + GetConstStrOffset(iValue), GetConstStrLength(iValue) );
			szValue = sValue.cstr();
		}

		tNode.m_pMapArg->Add ( szKey, szValue, iValue, fValue, eType );
	}

	tNode.m_eRetType = SPH_ATTR_MAPARG;
	return m_dNodes.GetLength()-1;
}


void ExprParser_t::AppendToMapArg ( int iNode, const char * szKey, const char * szValue, int64_t iValue, float fValue, VariantType_e eType )
{
	CSphString sValue;
	if ( eType==VariantType_e::STRING )
	{
		sValue = SqlUnescape ( m_sExpr.first + GetConstStrOffset(iValue), GetConstStrLength(iValue) );
		szValue = sValue.cstr();
	}

	m_dNodes[iNode].m_pMapArg->Add ( szKey, szValue, iValue, fValue, eType );
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


const char * ExprParser_t::Field2Ident ( uint64_t uAttrLoc )
{
	CSphString sIdent;
	sIdent = m_pSchema->GetField ( (int)uAttrLoc ).m_sName;
	m_dIdents.Add ( sIdent.Leak() );
	return m_dIdents.Last();
}


int ExprParser_t::AddNodeJsonField ( uint64_t uAttrLocator, int iLeft )
{
	int iNode = AddNodeAttr ( TOK_ATTR_JSON, uAttrLocator );
	m_dNodes[iNode].m_iLeft = iLeft;
	return m_dNodes.GetLength()-1;
}


int ExprParser_t::AddNodeJsonSubkey ( int64_t iValue )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_SUBKEY;
	tNode.m_eRetType = SPH_ATTR_STRING;
	tNode.m_iConst = iValue;
	return m_dNodes.GetLength()-1;
}


int ExprParser_t::AddNodeDotNumber ( int64_t iValue )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_iToken = TOK_CONST_FLOAT;
	tNode.m_eRetType = SPH_ATTR_FLOAT;
	const char * pCur = m_sExpr.first + (int)( iValue>>32 );
	tNode.m_fConst = (float) strtod ( pCur-1, nullptr );
	return m_dNodes.GetLength()-1;
}


int ExprParser_t::AddNodeIdent ( const char * sKey, int iLeft )
{
	ExprNode_t & tNode = m_dNodes.Add ();
	tNode.m_sIdent = sKey;
	tNode.m_iLeft = iLeft;
	tNode.m_iToken = TOK_IDENT;
	tNode.m_eRetType = SPH_ATTR_JSON_FIELD;
	return m_dNodes.GetLength()-1;
}


int ExprParser_t::ParseJoinAttr ( const char * szTable, uint64_t uOffset )
{
	CSphString sAttrName;
	sAttrName.SetBinary ( m_sExpr.first + GetConstStrOffset(uOffset), GetConstStrLength(uOffset) );

	CSphString sAttrWithTable;
	sAttrWithTable.SetSprintf ( "%s.%s", szTable, sAttrName.cstr() );	
	int iAttr = m_pSchema->GetAttrIndex ( sAttrWithTable.cstr() );
	if ( iAttr==-1 )
		m_sParserError.SetSprintf ( "unknown attribute '%s'", sAttrWithTable.cstr() );

	return iAttr;
}


int	ExprParser_t::AddNodeWithTable ( const char * szTable, uint64_t uOffset )
{
	int iAttr = ParseJoinAttr ( szTable, uOffset );
	if ( iAttr==-1 )
		return -1;

	YYSTYPE yylval;
	int iType = ParseAttr ( iAttr, m_pSchema->GetAttr(iAttr).m_sName.cstr(), &yylval );

	bool bColumnar = iType==TOK_COLUMNAR_INT || iType==SPH_ATTR_TIMESTAMP || iType==TOK_COLUMNAR_TIMESTAMP || iType==SPH_ATTR_FLOAT || iType==TOK_COLUMNAR_FLOAT || iType==TOK_COLUMNAR_BIGINT || iType==TOK_COLUMNAR_BOOL || iType==TOK_COLUMNAR_STRING || iType==TOK_COLUMNAR_UINT32SET || iType==TOK_COLUMNAR_INT64SET || iType==TOK_COLUMNAR_FLOATVEC;
	return bColumnar ? AddNodeColumnar ( iType, yylval.iAttrLocator ) : AddNodeAttr ( iType, yylval.iAttrLocator );
}


int ExprParser_t::AddWeightWithTable ( const char * szTable, uint64_t uOffset )
{
	CSphString sAttrName;
	sAttrName.SetBinary ( m_sExpr.first + GetConstStrOffset(uOffset), GetConstStrLength(uOffset) );
	sAttrName.SetSprintf ( "%s.%s()", szTable, sAttrName.cstr() );

	int iAttr = m_pSchema->GetAttrIndex ( sAttrName.cstr() );
	if ( iAttr==-1 )
	{
		m_sParserError.SetSprintf ( "unknown attribute '%s'", sAttrName.cstr() );
		return -1;
	}

	YYSTYPE yylval;
	int iType = ParseAttr ( iAttr, m_pSchema->GetAttr(iAttr).m_sName.cstr(), &yylval );

	bool bColumnar = iType==TOK_COLUMNAR_INT || iType==SPH_ATTR_TIMESTAMP || iType==TOK_COLUMNAR_TIMESTAMP || iType==SPH_ATTR_FLOAT || iType==TOK_COLUMNAR_FLOAT || iType==TOK_COLUMNAR_BIGINT || iType==TOK_COLUMNAR_BOOL || iType==TOK_COLUMNAR_STRING || iType==TOK_COLUMNAR_UINT32SET || iType==TOK_COLUMNAR_INT64SET || iType==TOK_COLUMNAR_FLOATVEC;
	return bColumnar ? AddNodeColumnar ( iType, yylval.iAttrLocator ) : AddNodeAttr ( iType, yylval.iAttrLocator );
}


uint64_t ExprParser_t::ParseAttrWithTable ( const char * szTable, uint64_t uOffset )
{
	int iAttr = ParseJoinAttr ( szTable, uOffset );
	if ( iAttr==-1 )
		return 0;

	// lvalp->iAttrLocator 
	return sphPackAttrLocator ( m_pSchema->GetAttr(iAttr).m_tLocator, iAttr );
}

//////////////////////////////////////////////////////////////////////////

// performs simple semantic analysis
// checks operand types for some arithmetic operators
struct TypeCheck_fn
{
	CSphString m_sError;

	void Enter ( const ExprNode_t & tNode, const CSphVector<ExprNode_t> & dNodes )
	{
		if ( !m_sError.IsEmpty() )
			return;

		bool bNumberOp = tNode.m_iToken=='+' || tNode.m_iToken=='-' || tNode.m_iToken=='*' || tNode.m_iToken=='/';
		if ( bNumberOp )
		{
			bool bLeftNumeric =	tNode.m_iLeft<0 ? false : IsNumericNode ( dNodes[tNode.m_iLeft] );
			bool bRightNumeric = tNode.m_iRight<0 ? false : IsNumericNode ( dNodes[tNode.m_iRight] );

			// if json vs numeric then let it pass (for the autoconversion)
			if ( ( bLeftNumeric && !bRightNumeric && dNodes[tNode.m_iRight].m_eRetType==SPH_ATTR_JSON_FIELD )
				|| ( bRightNumeric && !bLeftNumeric && dNodes[tNode.m_iLeft].m_eRetType==SPH_ATTR_JSON_FIELD ) )
					return;

			if ( !bLeftNumeric || !bRightNumeric )
			{
				m_sError = "numeric operation applied to non-numeric operands";
				return;
			}
		}

		if ( tNode.m_iToken==TOK_EQ )
		{
			// string equal must work with string columns only
			ESphAttr eLeftRet = tNode.m_iLeft<0 ? SPH_ATTR_NONE : dNodes[tNode.m_iLeft].m_eRetType;
			ESphAttr eRightRet = tNode.m_iRight<0 ? SPH_ATTR_NONE : dNodes[tNode.m_iRight].m_eRetType;
			bool bLeftStr = ( eLeftRet==SPH_ATTR_STRING || eLeftRet==SPH_ATTR_STRINGPTR || eLeftRet==SPH_ATTR_JSON_FIELD );
			bool bRightStr = ( eRightRet==SPH_ATTR_STRING || eRightRet==SPH_ATTR_STRINGPTR || eRightRet==SPH_ATTR_JSON_FIELD );
			if ( bLeftStr!=bRightStr && eLeftRet!=SPH_ATTR_JSON_FIELD && eRightRet!=SPH_ATTR_JSON_FIELD )
			{
				m_sError = "equal operation applied to part string operands";
				return;
			}
		}
	}

	void Exit ( const ExprNode_t & )
	{}

	bool IsNumericNode ( const ExprNode_t & tNode )
	{
		return tNode.m_eRetType==SPH_ATTR_INTEGER || tNode.m_eRetType==SPH_ATTR_BOOL || tNode.m_eRetType==SPH_ATTR_FLOAT ||
			tNode.m_eRetType==SPH_ATTR_BIGINT || tNode.m_eRetType==SPH_ATTR_TOKENCOUNT || tNode.m_eRetType==SPH_ATTR_TIMESTAMP;
	}
};


// checks whether we have a WEIGHT() in expression
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

// checks whether expression has functions defined not in this file like
// searchd-level function or ranker-level functions
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



static int EXPR_STACK_PARSE_CREATE = 400;
static int EXPR_STACK_PARSE = 150;

void SetExprNodeParseStackItemSize ( std::pair<int, int> tSize )
{
	EXPR_STACK_PARSE_CREATE = tSize.first;
	EXPR_STACK_PARSE = tSize.second;
}


static int EXPR_STACK_EVAL_CREATE = 100;
static int EXPR_STACK_EVAL = 8;


void SetExprNodeEvalStackItemSize ( std::pair<int, int> tSize )
{
	EXPR_STACK_EVAL_CREATE = tSize.first;
	EXPR_STACK_EVAL = tSize.second;
}


void SetMaxExprNodeEvalStackItemSize ( std::pair<int, int> tSize )
{
	if ( tSize.first>EXPR_STACK_EVAL_CREATE )
		EXPR_STACK_EVAL_CREATE = tSize.first;
	if ( tSize.second>EXPR_STACK_EVAL )
		EXPR_STACK_EVAL = tSize.second;
}


ISphExpr * ExprParser_t::Parse ( const char * sExpr, const ISphSchema & tSchema, const CSphString * pJoinIdx, ESphAttr * pAttrType, bool * pUsesWeight, CSphString & sError )
{
	const char* szExpr = sExpr;

	// fixme! provide shared access to semi-parsed items.
	CSphString sCopy ( sExpr ); szExpr = sCopy.cstr();

	m_sLexerError = ""; //lexer
	m_sParserError = "";
	m_sCreateError = "";

	// setup lexer
	m_sExpr = { szExpr, (int)strlen (szExpr) };
	m_pSchema = &tSchema;
	m_pJoinIdx = pJoinIdx;

	// setup constant functions
	m_iConstNow = (int) time ( nullptr );

	// build abstract syntax tree
	m_iParsed = -1;

	// alternative parser
	yy1lex_init ( &m_pScanner );

	char * sEnd = const_cast<char *>( szExpr+m_sExpr.second );
	char cMemLast = sEnd[1];
	if ( cMemLast )
		sEnd[1] = 0; // this is ok because string allocates a small gap

	YY_BUFFER_STATE tLexerBuffer = yy1_scan_buffer ( const_cast<char*>(szExpr), m_sExpr.second+2, m_pScanner );

	yyparse ( this );
	yy1_delete_buffer ( tLexerBuffer, m_pScanner );
	yy1lex_destroy ( m_pScanner );

	if ( cMemLast )
		sEnd[1] = cMemLast; // this is ok because string allocates a small gap

	// handle errors
	if ( m_iParsed<0 || !m_sLexerError.IsEmpty() || !m_sParserError.IsEmpty() )
	{
		sError = !m_sLexerError.IsEmpty() ? m_sLexerError : m_sParserError;
		if ( sError.IsEmpty() ) sError = "general parsing error";
		return nullptr;
	}

	// deduce return type
	ESphAttr eAttrType = m_dNodes[m_iParsed].m_eRetType;

	// pooled MVA/string attributes are ok to use in expressions, but storing them into schema requires their _PTR counterparts
	if ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET || eAttrType==SPH_ATTR_STRING )
		eAttrType = sphPlainAttrToPtrAttr(eAttrType);

	// Check expression stack to fit for mutual recursive function calls.
	// This check is an approximation, because different compilers with
	// different settings produce code which requires different stack size.
	int iStackNeeded = -1;
	int iStackEval = -1;
	const int TREE_SIZE_THRESH = 20;
	if ( m_dNodes.GetLength ()>TREE_SIZE_THRESH )
	{
		StackSizeParams_t tParams;
		tParams.iMaxDepth = EvalMaxTreeHeight ( m_dNodes, m_iParsed );
		tParams.tNodeStackSize = { EXPR_STACK_PARSE_CREATE, EXPR_STACK_PARSE };
		tParams.szName = "expressions";
		std::tie ( iStackNeeded, sError ) = EvalStackForExpr ( tParams );
		if ( !sError.IsEmpty() )
			return nullptr;

		tParams.tNodeStackSize = { EXPR_STACK_EVAL_CREATE, EXPR_STACK_EVAL };
		tParams.szName = "eval expressions";
		std::tie ( iStackEval, sError ) = EvalStackForExpr ( tParams );
	}

	ISphExpr * pExpr = nullptr;

	Threads::Coro::Continue ( iStackNeeded, [&] {

	pExpr = Create ( pUsesWeight, sError );

	if ( pAttrType )
		*pAttrType = eAttrType;
	} );

	if ( pExpr && iStackEval>0 )
	{
		// in case we're in real query processing - propagate size of stack need for evaluations (only additional part)
		session::ExpandDesiredStack ( iStackEval );
		auto pChildExpr = pExpr;
		pExpr = new Expr_ProxyFat_c ( pChildExpr );
		pChildExpr->Release();
	}

	return pExpr;
}

#ifndef NDEBUG
static void CheckDescendingNodes ( const CSphVector<ExprNode_t> & dNodes )
{
	ARRAY_CONSTFOREACH( i, dNodes )
	{
		assert ( i>dNodes[i].m_iLeft );
		assert ( i>dNodes[i].m_iRight );
	}
}
#endif

ISphExpr * ExprParser_t::Create ( bool * pUsesWeight, CSphString & sError )
{
	if ( GetError () )
		return nullptr;

#ifndef NDEBUG
	CheckDescendingNodes ( m_dNodes );
#endif
	// perform optimizations (tree transformations)
	Optimize ( m_iParsed );

// fixme! canonize pass breaks constraight on "1+2+3*aaa"
#ifndef NDEBUG
//	CheckDescendingNodes ( m_dNodes );
#endif

	// simple semantic analysis
	TypeCheck_fn tTypeChecker;
	WalkTree ( m_iParsed, tTypeChecker );
	if ( !tTypeChecker.m_sError.IsEmpty() )
	{
		sError.Swap ( tTypeChecker.m_sError );
		return nullptr;
	}

	// create evaluator
	CSphRefcountedPtr<ISphExpr> pRes { CreateTree ( m_iParsed ) };
	if ( !m_sCreateError.IsEmpty() )
	{
		pRes = nullptr;
		sError = m_sCreateError;
	}
	else if ( !pRes )
	{
		sError.SetSprintf ( "empty expression" );
	}

	if ( pUsesWeight )
	{
		WeightCheck_fn tWeightFunctor ( pUsesWeight );
		WalkTree ( m_iParsed, tWeightFunctor );
	}

	if ( m_pHook )
	{
		HookCheck_fn tHookFunctor ( m_pHook );
		WalkTree ( m_iParsed, tHookFunctor );
	}

	return pRes.Leak();
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC STUFF
//////////////////////////////////////////////////////////////////////////

JoinArgs_t::JoinArgs_t ( const ISphSchema & tJoinedSchema, const CSphString & sIndex1, const CSphString & sIndex2 )
	: m_tJoinedSchema ( tJoinedSchema )
	, m_sIndex1 ( sIndex1 )
	, m_sIndex2 ( sIndex2 )
{}

/// parser entry point
ISphExpr * sphExprParse ( const char * szExpr, const ISphSchema & tSchema, const CSphString * pJoinIdx, CSphString & sError, ExprParseArgs_t & tArgs )
{
	// parse into opcodes
	ExprParser_t tParser ( tArgs.m_pHook, tArgs.m_pProfiler, tArgs.m_eCollation );
	ISphExpr * pRes = tParser.Parse ( szExpr, tSchema, pJoinIdx, tArgs.m_pAttrType, tArgs.m_pUsesWeight, sError );
	if ( tArgs.m_pZonespanlist )
		*tArgs.m_pZonespanlist = tParser.m_bHasZonespanlist;
	if ( tArgs.m_pEvalStage )
		*tArgs.m_pEvalStage = tParser.m_eEvalStage;
	if ( tArgs.m_pPackedFactorsFlags )
		*tArgs.m_pPackedFactorsFlags = tParser.m_uPackedFactorFlags;
	if ( tArgs.m_pStoredField )
		*tArgs.m_pStoredField = tParser.m_uStoredField;
	if ( tArgs.m_pNeedDocIds )
		*tArgs.m_pNeedDocIds = tParser.m_bNeedDocIds;

	return pRes;
}

/// json type autoconversion
ISphExpr * sphJsonFieldConv ( ISphExpr * pExpr )
{
	return new Expr_JsonFieldConv_c ( pExpr );
}


void FetchAttrDependencies ( StrVec_t & dAttrNames, const ISphSchema & tSchema )
{
	for ( const auto & i : dAttrNames )
	{
		const CSphColumnInfo * pAttr = tSchema.GetAttr ( i.cstr() );
		if ( !pAttr || !pAttr->m_pExpr )
			continue;

		int iOldLen = dAttrNames.GetLength();
		pAttr->m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dAttrNames );
		for ( int iNewAttr = iOldLen; iNewAttr < dAttrNames.GetLength(); iNewAttr++ )
			if ( dAttrNames[iNewAttr]==i )
				dAttrNames.Remove(iNewAttr);
	}

	dAttrNames.Uniq();
}
