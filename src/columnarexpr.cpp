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

#include "columnarexpr.h"
#include "exprtraits.h"
#include "sphinxint.h"
#include "conversion.h"

template <typename T>
class Expr_Columnar_MVAIn_T : public Expr_ArgVsConstSet_T<int64_t>
{
	using BASE = Expr_ArgVsConstSet_T<int64_t>;

public:
				Expr_Columnar_MVAIn_T ( const CSphString & sName, ConstList_c * pConsts );

	ByteBlob_t	MvaEval ( const CSphMatch & ) const final { assert ( 0 && "not implemented" ); return {nullptr,0}; }
	int			IntEval ( const CSphMatch & tMatch ) const final;
	void		Command ( ESphExprCommand eCmd, void * pArg ) final;
	uint64_t	GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final;
	ISphExpr *	Clone() const final { return new Expr_Columnar_MVAIn_T ( *this ); }

private:
	std::unique_ptr<columnar::Iterator_i> m_pIterator;
	CSphString	m_sName;

				Expr_Columnar_MVAIn_T ( const Expr_Columnar_MVAIn_T & rhs );
};

template <typename T>
Expr_Columnar_MVAIn_T<T>::Expr_Columnar_MVAIn_T ( const CSphString & sName, ConstList_c * pConsts )
	: BASE ( nullptr, pConsts, false )
	, m_sName ( sName )
{
	assert ( pConsts );
	BASE::m_dValues.Sort();
}

template <typename T>
int Expr_Columnar_MVAIn_T<T>::IntEval ( const CSphMatch & tMatch ) const
{
	const uint8_t * pData = nullptr;
	int iLen = m_pIterator->Get ( tMatch.m_tRowID, pData );
	return MvaEval_Any<T> ( { (T*)const_cast<uint8_t*>(pData), int64_t(iLen/sizeof(T)) }, m_dValues );
}

template <typename T>
void Expr_Columnar_MVAIn_T<T>::Command ( ESphExprCommand eCmd, void * pArg )
{
	BASE::Command ( eCmd, pArg );

	if ( eCmd==SPH_EXPR_SET_COLUMNAR )
	{
		auto pColumnar = (const columnar::Columnar_i*)pArg;
		if ( pColumnar )
		{
			std::string sError; // FIXME! report errors
			m_pIterator = CreateColumnarIterator ( pColumnar, m_sName.cstr(), sError );
		}
	}
}

template <typename T>
uint64_t Expr_Columnar_MVAIn_T<T>::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_Columnar_MVAIn_T");
	return CALC_DEP_HASHES_EX(m_uValueHash);
}

template <typename T>
Expr_Columnar_MVAIn_T<T>::Expr_Columnar_MVAIn_T ( const Expr_Columnar_MVAIn_T & rhs )
	: BASE(rhs)
	, m_sName ( rhs.m_sName )
{}

/////////////////////////////////////////////////////////////////////

class Expr_Columnar_StringIn_c : public Expr_ArgVsConstSet_T<int64_t>
{
	using BASE = Expr_ArgVsConstSet_T<int64_t>;

public:
				Expr_Columnar_StringIn_c ( const CSphString & sName, ConstList_c * pConsts, ESphCollation eCollation );

	int			IntEval ( const CSphMatch & tMatch ) const final;
	void		Command ( ESphExprCommand eCmd, void * pArg ) final;
	uint64_t	GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final;
	ISphExpr *	Clone() const final { return new Expr_Columnar_StringIn_c ( *this ); }

private:
	std::unique_ptr<columnar::Iterator_i> m_pIterator;
	CSphString				m_sName;
	StrHashCalc_fn			m_fnHashCalc = nullptr;
	CSphVector<uint64_t>	m_dHashes;
	ESphCollation			m_eCollation = SPH_COLLATION_DEFAULT;
	bool					m_bHasHashes = false;

				Expr_Columnar_StringIn_c ( const Expr_Columnar_StringIn_c & rhs );

	uint64_t	GetStringHash ( RowID_t tRowID ) const;
};


Expr_Columnar_StringIn_c::Expr_Columnar_StringIn_c ( const CSphString & sName, ConstList_c * pConsts, ESphCollation eCollation )
	: BASE ( nullptr, pConsts, false )
	, m_sName ( sName )
	, m_fnHashCalc ( GetStringHashCalcFunc(eCollation) )
	, m_eCollation ( eCollation )
{
	assert(pConsts);

	const char * szExpr = pConsts->m_sExpr.first;
	int iExprLen = pConsts->m_sExpr.second;

	for ( int64_t iVal : m_dValues  )
	{
		int iOfs = GetConstStrOffset ( iVal );
		int iLen = GetConstStrLength ( iVal );
		if ( iOfs>0 && iOfs+iLen<=iExprLen )
		{
			auto tRes = SqlUnescapeN ( szExpr + iOfs, iLen );
			m_dHashes.Add ( tRes.first.IsEmpty() ? 0 : m_fnHashCalc ( (const BYTE*)tRes.first.cstr(), tRes.second, SPH_FNV64_SEED ) );
		}
	}
}


int Expr_Columnar_StringIn_c::IntEval ( const CSphMatch & tMatch ) const
{
	uint64_t uHash = GetStringHash ( tMatch.m_tRowID );
	for ( auto i : m_dHashes )
		if ( i==uHash )
			return 1;

	return 0;
}


void Expr_Columnar_StringIn_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	BASE::Command ( eCmd, pArg );

	if ( eCmd==SPH_EXPR_SET_COLUMNAR )
	{
		auto pColumnar = (const columnar::Columnar_i*)pArg;
		if ( pColumnar )
		{
			columnar::IteratorHints_t tHints;
			columnar::IteratorCapabilities_t tCapabilities;
			tHints.m_bNeedStringHashes = m_eCollation==SPH_COLLATION_DEFAULT;

			std::string sError; // FIXME! report errors
			m_pIterator = CreateColumnarIterator ( pColumnar, m_sName.cstr(), sError, tHints, &tCapabilities );
			assert ( m_pIterator );
			m_bHasHashes = tCapabilities.m_bStringHashes;
		}
	}
}


uint64_t Expr_Columnar_StringIn_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_Columnar_StringIn_c");
	CALC_STR_HASH ( m_sName, m_sName.Length() );
	CALC_POD_HASH(m_fnHashCalc);
	CALC_POD_HASHES(m_dHashes);
	CALC_POD_HASH(m_eCollation);
	CALC_POD_HASH(m_bHasHashes);
	return CALC_PARENT_HASH_EX(m_uValueHash);
}


Expr_Columnar_StringIn_c::Expr_Columnar_StringIn_c ( const Expr_Columnar_StringIn_c & rhs )
	: BASE (rhs)
	, m_sName ( rhs.m_sName )
	, m_fnHashCalc ( rhs.m_fnHashCalc )
	, m_dHashes ( rhs.m_dHashes )
{}


uint64_t Expr_Columnar_StringIn_c::GetStringHash ( RowID_t tRowID ) const
{
	if ( m_bHasHashes )
		return m_pIterator->Get(tRowID);

	const BYTE * pStr = nullptr;
	int iLen = m_pIterator->Get ( tRowID, pStr );
	if ( !iLen )
		return 0;

	return m_fnHashCalc ( pStr, iLen, SPH_FNV64_SEED );
}

/////////////////////////////////////////////////////////////////////

class Expr_Columnar_StringLength_c : public ISphExpr
{
public:
				Expr_Columnar_StringLength_c ( const CSphString & sName ) : m_sName ( sName ) {}

	int			IntEval ( const CSphMatch & tMatch ) const override { return m_pIterator->GetLength( tMatch.m_tRowID ); }
	void		Command ( ESphExprCommand eCmd, void * pArg ) override;
	float		Eval ( const CSphMatch & tMatch ) const final { return (float)IntEval ( tMatch ); }
	uint64_t	GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) override;
	void		FixupLocator ( const ISphSchema * /*pOldSchema*/, const ISphSchema * /*pNewSchema*/ ) final {}
	ISphExpr *	Clone() const override{ return new Expr_Columnar_StringLength_c(m_sName); }

protected:
	CSphString	m_sName;
	std::unique_ptr<columnar::Iterator_i> m_pIterator;
};


void Expr_Columnar_StringLength_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( eCmd==SPH_EXPR_SET_COLUMNAR )
	{
		auto pColumnar = (const columnar::Columnar_i*)pArg;
		if ( pColumnar )
		{
			std::string sError; // FIXME! report errors
			m_pIterator = CreateColumnarIterator ( pColumnar, m_sName.cstr(), sError );
		}
	}
}


uint64_t Expr_Columnar_StringLength_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_Columnar_StringLength_c");
	CALC_STR_HASH ( m_sName, m_sName.Length() );
	return CALC_DEP_HASHES();
}

/////////////////////////////////////////////////////////////////////

template<typename T>
class Expr_Columnar_MvaLength_T : public Expr_Columnar_StringLength_c
{
	using Expr_Columnar_StringLength_c::Expr_Columnar_StringLength_c;

public:
	int			IntEval ( const CSphMatch & tMatch ) const final { return m_pIterator->GetLength ( tMatch.m_tRowID ) / sizeof(T); }
	uint64_t	GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final;
	ISphExpr *	Clone() const final	{ return new Expr_Columnar_MvaLength_T(m_sName); }
};

template<typename T>
uint64_t Expr_Columnar_MvaLength_T<T>::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_Columnar_MvaLength_T");
	CALC_STR_HASH ( m_sName, m_sName.Length() );
	return CALC_DEP_HASHES();
}

/////////////////////////////////////////////////////////////////////

// aggregate functions evaluator for columnar MVAs
template <typename T>
class Expr_ColumnarMVAAggr_T : public Expr_NoLocator_c
{
public:
				Expr_ColumnarMVAAggr_T ( ISphExpr * pExpr, ESphAggrFunc eFunc );

	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final;
	float		Eval ( const CSphMatch & tMatch ) const final { return (float)Int64Eval ( tMatch ); }
	int			IntEval ( const CSphMatch & tMatch ) const final { return (int)Int64Eval ( tMatch ); }
	void		Command ( ESphExprCommand eCmd, void * pArg ) final;
	uint64_t	GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final;
	ISphExpr *	Clone() const final	{ return new Expr_ColumnarMVAAggr_T ( *this ); }

protected:
	CSphRefcountedPtr<ISphExpr> m_pExpr;
	ESphAggrFunc m_eFunc = SPH_AGGR_NONE;

private:
				Expr_ColumnarMVAAggr_T ( const Expr_ColumnarMVAAggr_T & rhs );
};

template <typename T>
Expr_ColumnarMVAAggr_T<T>::Expr_ColumnarMVAAggr_T ( ISphExpr * pExpr, ESphAggrFunc eFunc )
	: m_pExpr ( pExpr )
	, m_eFunc ( eFunc )
{
	SafeAddRef(m_pExpr);
}

template <typename T>
int64_t Expr_ColumnarMVAAggr_T<T>::Int64Eval ( const CSphMatch & tMatch ) const
{
	if ( !m_pExpr )
		return 0;

	ByteBlob_t tMva = m_pExpr->MvaEval(tMatch);
	if ( !tMva.second )
		return 0;

	int nValues = tMva.second / sizeof(T);

	const T * L = (const T *)tMva.first;
	const T * R = L+nValues-1;

	switch ( m_eFunc )
	{
	case SPH_AGGR_MIN:	return *L;
	case SPH_AGGR_MAX:	return *R;
	default:			return 0;
	}
}

template <typename T>
void Expr_ColumnarMVAAggr_T<T>::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( m_pExpr )
		m_pExpr->Command ( eCmd, pArg );
}

template <typename T>
uint64_t Expr_ColumnarMVAAggr_T<T>::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_ColumnarMVAAggr_T");
	CALC_POD_HASH(m_eFunc);
	CALC_CHILD_HASH(m_pExpr);
	return CALC_DEP_HASHES();
}

template <typename T>
Expr_ColumnarMVAAggr_T<T>::Expr_ColumnarMVAAggr_T ( const Expr_ColumnarMVAAggr_T & rhs )
	: m_pExpr ( SafeClone ( rhs.m_pExpr ) )
	, m_eFunc ( rhs.m_eFunc )
{}

/////////////////////////////////////////////////////////////////////

class Expr_GetColumnar_Traits_c : public ISphExpr
{
public:
				Expr_GetColumnar_Traits_c ( const CSphString & sName, bool bStored );
				Expr_GetColumnar_Traits_c ( const Expr_GetColumnar_Traits_c & rhs );

	void		FixupLocator ( const ISphSchema * /*pOldSchema*/, const ISphSchema * /*pNewSchema*/ ) final {}
	void		Command ( ESphExprCommand eCmd, void * pArg ) final;
	bool		IsColumnar ( bool * pStored ) const final;

protected:
	CSphString	m_sName;
	bool		m_bStored = false;
	std::unique_ptr<columnar::Iterator_i> m_pIterator;

	uint64_t CalcHash ( const char * szTag, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
	{
		EXPR_CLASS_NAME_NOCHECK(szTag);
		CALC_STR_HASH(m_sName, m_sName.Length());
		CALC_POD_HASH(m_bStored);
		return CALC_DEP_HASHES();
	}
};


Expr_GetColumnar_Traits_c::Expr_GetColumnar_Traits_c ( const CSphString & sName, bool bStored )
	: m_sName ( sName )
	, m_bStored ( bStored )
{}


Expr_GetColumnar_Traits_c::Expr_GetColumnar_Traits_c ( const Expr_GetColumnar_Traits_c & rhs )
	: m_sName ( rhs.m_sName )
	, m_bStored ( rhs.m_bStored )
{}


bool Expr_GetColumnar_Traits_c::IsColumnar ( bool * pStored ) const
{
	if ( pStored )
		*pStored = m_bStored;

	return true;
}


void Expr_GetColumnar_Traits_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	switch ( eCmd )
	{
	case SPH_EXPR_SET_COLUMNAR:
	{
		auto pColumnar = (const columnar::Columnar_i*)pArg;
		if ( pColumnar )
		{
			std::string sError; // FIXME! report errors
			m_pIterator = CreateColumnarIterator ( pColumnar, m_sName.cstr(), sError );
		}
		else
			m_pIterator.reset();
	}
	break;

	case SPH_EXPR_GET_COLUMNAR_COL:
		*(CSphString*)pArg = m_sName;
		break;

	default:
		break;
	}
}

/////////////////////////////////////////////////////////////////////

class Expr_GetColumnarInt_c : public Expr_GetColumnar_Traits_c
{
	using Expr_GetColumnar_Traits_c::Expr_GetColumnar_Traits_c;

public:
	float		Eval ( const CSphMatch & tMatch ) const override		{ return (float)FetchValue(tMatch); }
	int			IntEval ( const CSphMatch & tMatch ) const override		{ return (int)FetchValue(tMatch); }
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const override	{ return FetchValue(tMatch); }
	uint64_t	GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final;
	ISphExpr *	Clone() const override									{ return new Expr_GetColumnarInt_c ( m_sName, m_bStored ); }

protected:
	inline SphAttr_t FetchValue ( const CSphMatch & tMatch ) const		{  return m_pIterator->Get ( tMatch.m_tRowID ); }
};


uint64_t Expr_GetColumnarInt_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_GetColumnarInt_c");
	return CALC_PARENT_HASH();
}

/////////////////////////////////////////////////////////////////////

class Expr_GetColumnarFloat_c : public Expr_GetColumnarInt_c
{
	using Expr_GetColumnarInt_c::Expr_GetColumnarInt_c;

public:
	float	Eval ( const CSphMatch & tMatch ) const final		{ return sphDW2F ( (DWORD)FetchValue(tMatch) ); }
	int		IntEval ( const CSphMatch & tMatch ) const final	{ return (int)sphDW2F ( (DWORD)FetchValue(tMatch) ); }
	int64_t	Int64Eval ( const CSphMatch & tMatch ) const final	{ return (int64_t)sphDW2F ( (DWORD)FetchValue(tMatch) ); }
	ISphExpr *	Clone() const final								{ return new Expr_GetColumnarFloat_c ( m_sName, m_bStored ); }
};

/////////////////////////////////////////////////////////////////////

class Expr_GetColumnarString_c : public Expr_GetColumnar_Traits_c
{
	using Expr_GetColumnar_Traits_c::Expr_GetColumnar_Traits_c;

public:
	float			Eval ( const CSphMatch & ) const final { assert ( 0 ); return 0; }
	int				StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const final	{ return m_pIterator->Get ( tMatch.m_tRowID, *ppStr ); }
	const BYTE *	StringEvalPacked ( const CSphMatch & tMatch ) const final					{ return m_pIterator->GetPacked ( tMatch.m_tRowID ); }
	int				StringLenEval ( const CSphMatch & tMatch ) const final						{ return m_pIterator->GetLength ( tMatch.m_tRowID ); }
	uint64_t		GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final;
	ISphExpr *		Clone() const final { return new Expr_GetColumnarString_c ( m_sName, m_bStored ); }
};


uint64_t Expr_GetColumnarString_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_GetColumnarString_c");
	return CALC_PARENT_HASH();
}

//////////////////////////////////////////////////////////////////////////

class Expr_GetColumnarMva_c : public Expr_GetColumnar_Traits_c
{
	using Expr_GetColumnar_Traits_c::Expr_GetColumnar_Traits_c;

public:
	float		Eval ( const CSphMatch & ) const final { assert ( 0 ); return 0; }
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final	{ return (int64_t)m_pIterator->GetPacked ( tMatch.m_tRowID ); }
	ByteBlob_t	MvaEval ( const CSphMatch & tMatch ) const final;
	uint64_t	GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final;
	ISphExpr *	Clone() const final { return new Expr_GetColumnarMva_c ( m_sName, m_bStored ); }
};


ByteBlob_t Expr_GetColumnarMva_c::MvaEval ( const CSphMatch & tMatch ) const
{
	const BYTE * pResult = nullptr;
	int iBytes = m_pIterator->Get ( tMatch.m_tRowID, pResult );
	return {pResult, iBytes};
}


uint64_t Expr_GetColumnarMva_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_GetColumnarMva_c");
	return CALC_PARENT_HASH();
}

/////////////////////////////////////////////////////////////////////

ISphExpr * CreateExpr_ColumnarMva32In ( const CSphString & sName, ConstList_c * pConsts )		{ return new Expr_Columnar_MVAIn_T<DWORD> ( sName, pConsts ); }
ISphExpr * CreateExpr_ColumnarMva64In ( const CSphString & sName, ConstList_c * pConsts )		{ return new Expr_Columnar_MVAIn_T<int64_t> ( sName, pConsts ); }
ISphExpr * CreateExpr_ColumnarStringIn ( const CSphString & sName, ConstList_c * pConsts, ESphCollation eCollation ) { return new Expr_Columnar_StringIn_c ( sName, pConsts, eCollation ); }

ISphExpr * CreateExpr_ColumnarStringLength ( const CSphString & sName )	{ return new Expr_Columnar_StringLength_c(sName); }
ISphExpr * CreateExpr_ColumnarMva32Length ( const CSphString & sName )	{ return new Expr_Columnar_MvaLength_T<DWORD>(sName); }
ISphExpr * CreateExpr_ColumnarMva64Length ( const CSphString & sName )	{ return new Expr_Columnar_MvaLength_T<uint64_t>(sName); }

ISphExpr * CreateExpr_ColumnarMva32Aggr ( ISphExpr * pExpr, ESphAggrFunc eFunc )	{ return new Expr_ColumnarMVAAggr_T<DWORD> ( pExpr, eFunc ); }
ISphExpr * CreateExpr_ColumnarMva64Aggr ( ISphExpr * pExpr, ESphAggrFunc eFunc )	{ return new Expr_ColumnarMVAAggr_T<int64_t> ( pExpr, eFunc ); }

ISphExpr * CreateExpr_GetColumnarInt ( const CSphString & sName, bool bStored )		{ return new Expr_GetColumnarInt_c ( sName, bStored ); }
ISphExpr * CreateExpr_GetColumnarFloat ( const CSphString & sName, bool bStored )	{ return new Expr_GetColumnarFloat_c ( sName, bStored ); }
ISphExpr * CreateExpr_GetColumnarString ( const CSphString & sName, bool bStored )	{ return new Expr_GetColumnarString_c ( sName, bStored ); }
ISphExpr * CreateExpr_GetColumnarMva ( const CSphString & sName, bool bStored )		{ return new Expr_GetColumnarMva_c ( sName, bStored ); }
