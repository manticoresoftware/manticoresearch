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

#ifndef _exprtraits_
#define _exprtraits_

#include "sphinxexpr.h"
#include "sphinxfilter.h"
#include "conversion.h"
#include "match.h"

#if _WIN32 && !defined(__clang__)
#ifndef NDEBUG
#define EXPR_CLASS_NAME(name) \
	{\
		const char * szFuncName = __FUNCTION__; \
		const char * szClassNameEnd = strstr ( szFuncName, "::" ); \
		assert ( szClassNameEnd ); \
		const char * szTemplateNameEnd = strstr ( szFuncName, "<" ); \
		if ( szTemplateNameEnd ) szClassNameEnd = szTemplateNameEnd; \
			size_t iLen = szClassNameEnd-szFuncName; \
		assert ( strlen(name)==iLen && "Wrong expression name specified in ::GetHash" ); \
		assert ( !strncmp(name, szFuncName, iLen) && "Wrong expression name specified in ::GetHash" ); \
	}\
	const char * szClassName = name; \
	uint64_t uHash = uPrevHash;
#else
#define EXPR_CLASS_NAME(name) \
	const char * szClassName = name; \
	uint64_t uHash = uPrevHash;
#endif
#else
#define EXPR_CLASS_NAME(name) \
	const char * szClassName = name; \
	uint64_t uHash = uPrevHash;
#endif

#define EXPR_CLASS_NAME_NOCHECK(name) \
	const char * szClassName = name; \
	uint64_t uHash = uPrevHash;

#define CALC_DEP_HASHES() sphCalcExprDepHash ( szClassName, this, tSorterSchema, uHash, bDisable );
#define CALC_DEP_HASHES_EX(hash) sphCalcExprDepHash ( szClassName, this, tSorterSchema, uHash^hash, bDisable );
#define CALC_PARENT_HASH() CalcHash ( szClassName, tSorterSchema, uHash, bDisable );
#define CALC_PARENT_HASH_EX(hash) CalcHash ( szClassName, tSorterSchema, uHash^hash, bDisable );

#define CALC_POD_HASH(value) uHash = sphFNV64 ( &value, sizeof(value), uHash );
#define CALC_POD_HASHES(values) uHash = sphFNV64 ( values.Begin(), values.GetLength()*sizeof(values[0]), uHash );
#define CALC_STR_HASH(str,len) uHash = sphFNV64 ( str.cstr(), len, uHash );
#define CALC_CHILD_HASH(child) if (child) uHash = child->GetHash ( tSorterSchema, uHash, bDisable );
#define CALC_CHILD_HASHES(children) ARRAY_FOREACH ( i, children ) if (children[i]) uHash = children[i]->GetHash ( tSorterSchema, uHash, bDisable );

bool		IsNumeric ( ESphAttr eType );			// check whether the type is numeric
ESphAttr	GetIntType ( int64_t iValue );			// check for type based on int value
ESphAttr	WidestType ( ESphAttr a, ESphAttr b );	// get the widest numeric type of the two

uint64_t	sphCalcLocatorHash ( const CSphAttrLocator & tLoc, uint64_t uPrevHash );
uint64_t	sphCalcExprDepHash ( const char * szTag, ISphExpr * pExpr, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable );
uint64_t	sphCalcExprDepHash ( ISphExpr * pExpr, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable );

int			GetConstStrOffset ( int64_t iValue );
int			GetConstStrLength ( int64_t iValue );

/// list of constants
class ConstList_c
{
public:
	CSphVector<int64_t>		m_dInts;					///< dword/int64 storage
	CSphVector<float>		m_dFloats;					///< float storage
	ESphAttr				m_eRetType { SPH_ATTR_INTEGER };		///< SPH_ATTR_INTEGER, SPH_ATTR_BIGINT, SPH_ATTR_STRING, or SPH_ATTR_FLOAT
	Str_t					m_sExpr;					///< pointer to original whole expr. Not owned.
	bool					m_bPackedStrings = false;	// packed string are offset:len from m_dInts over m_sExpr

	void Add ( int64_t iValue );
	void Add ( float fValue );
};

/// arg-vs-set function (currently, IN or INTERVAL) evaluator traits
template < typename T >
class Expr_ArgVsSet_T : public ISphExpr
{
public:
	explicit Expr_ArgVsSet_T ( ISphExpr * pArg ) : m_pArg ( pArg ) { SafeAddRef ( pArg ); }

	float	Eval ( const CSphMatch & tMatch ) const override { return (float) IntEval ( tMatch ); }
	int64_t Int64Eval ( const CSphMatch & tMatch ) const override { return IntEval ( tMatch ); }
	void	FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		if ( m_pArg )
			m_pArg->FixupLocator ( pOldSchema, pNewSchema );
	}

	void	Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		if ( m_pArg )
			m_pArg->Command ( eCmd, pArg );
	}

protected:
	CSphRefcountedPtr<ISphExpr> m_pArg; /* { nullptr }; */

			Expr_ArgVsSet_T ( const Expr_ArgVsSet_T & rhs ) : m_pArg ( SafeClone ( rhs.m_pArg ) ) {}
	T		ExprEval ( ISphExpr * pArg, const CSphMatch & tMatch ) const;

	uint64_t CalcHash ( const char * szTag, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
	{
		EXPR_CLASS_NAME_NOCHECK(szTag);
		CALC_CHILD_HASH(m_pArg);
		return CALC_DEP_HASHES();
	}
};


template<> inline int Expr_ArgVsSet_T<int>::ExprEval ( ISphExpr * pArg, const CSphMatch & tMatch ) const
{
	return pArg->IntEval ( tMatch );
}


template<> inline DWORD Expr_ArgVsSet_T<DWORD>::ExprEval ( ISphExpr * pArg, const CSphMatch & tMatch ) const
{
	return (DWORD)pArg->IntEval ( tMatch );
}


template<> inline float Expr_ArgVsSet_T<float>::ExprEval ( ISphExpr * pArg, const CSphMatch & tMatch ) const
{
	return pArg->Eval ( tMatch );
}


template<> inline int64_t Expr_ArgVsSet_T<int64_t>::ExprEval ( ISphExpr * pArg, const CSphMatch & tMatch ) const
{
	return pArg->Int64Eval ( tMatch );
}

/// arg-vs-constant-set
template < typename T >
class Expr_ArgVsConstSet_T : public Expr_ArgVsSet_T<T>
{
public:
	/// pre-evaluate and dismiss turn points
	Expr_ArgVsConstSet_T ( ISphExpr * pArg, const CSphVector<ISphExpr *> & dArgs, int iSkip )
		: Expr_ArgVsSet_T<T> ( pArg )
		, m_bFloat ( false )
	{
		CSphMatch tDummy;
		for ( int i=iSkip; i<dArgs.GetLength(); ++i )
			m_dValues.Add ( Expr_ArgVsSet_T<T>::ExprEval ( dArgs[i], tDummy ) );

		CalcValueHash();
	}

	/// copy that constlist
	Expr_ArgVsConstSet_T ( ISphExpr * pArg, ConstList_c * pConsts, bool bKeepFloat )
		: Expr_ArgVsSet_T<T> ( pArg )
		, m_bFloat ( false )
	{
		assert ( pConsts );
		if ( pConsts->m_eRetType==SPH_ATTR_FLOAT )
		{
			m_dValues.Reserve ( pConsts->m_dFloats.GetLength() );
			if ( !bKeepFloat )
			{
				for ( float fV : pConsts->m_dFloats )
					m_dValues.Add ( (T)fV );
			} else
			{
				m_bFloat = true;
				for ( float fV : pConsts->m_dFloats )
					m_dValues.Add ( (T) sphF2DW ( fV ) );
			}
		} else
		{
			m_dValues.Reserve ( pConsts->m_dInts.GetLength() );
			for ( auto iV : pConsts->m_dInts )
				m_dValues.Add ( (T)iV );
		}

		CalcValueHash();
	}

	/// copy that uservar
	Expr_ArgVsConstSet_T ( ISphExpr * pArg, const UservarIntSet_c & pUservar )
		: Expr_ArgVsSet_T<T> ( pArg )
		, m_bFloat ( false )
	{
		assert ( pUservar );
		m_dValues.Reserve ( pUservar->GetLength() );
		for ( auto iV : *pUservar )
			m_dValues.Add ( (T)iV );

		CalcValueHash();
	}

	uint64_t GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) override
	{
		EXPR_CLASS_NAME("Expr_ArgVsConstSet_T");
		return CALC_PARENT_HASH();
	}

protected:
	CSphVector<T>	m_dValues;
	uint64_t		m_uValueHash=0;
	bool			m_bFloat = false;

	Expr_ArgVsConstSet_T ( const Expr_ArgVsConstSet_T & rhs )
		: Expr_ArgVsSet_T<T> ( rhs )
		, m_dValues ( rhs.m_dValues )
		, m_uValueHash ( rhs.m_uValueHash )
		, m_bFloat ( rhs.m_bFloat )
	{}

	void CalcValueHash()
	{
		ARRAY_FOREACH ( i, m_dValues )
			m_uValueHash = sphFNV64 ( &m_dValues[i], sizeof(m_dValues[i]), i ? m_uValueHash : SPH_FNV64_SEED );
	}

	uint64_t CalcHash ( const char * szTag, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
	{
		return Expr_ArgVsSet_T<T>::CalcHash ( szTag, tSorterSchema, uPrevHash^m_uValueHash, bDisable );
	}
};

#endif // _exprtraits_

