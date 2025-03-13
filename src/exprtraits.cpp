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

#include "exprtraits.h"

ESphAttr GetIntType ( int64_t iValue )
{
	return ( iValue>=(int64_t)INT_MIN && iValue<=(int64_t)INT_MAX ) ? SPH_ATTR_INTEGER : SPH_ATTR_BIGINT;
}


bool IsNumeric ( ESphAttr eType )
{
	return eType==SPH_ATTR_INTEGER || eType==SPH_ATTR_BIGINT || eType==SPH_ATTR_FLOAT;
}


ESphAttr WidestType ( ESphAttr a, ESphAttr b )
{
	assert ( ( IsNumeric(a) && IsNumeric(b) ) || ( IsNumeric(a) && b==SPH_ATTR_JSON_FIELD ) || ( a==SPH_ATTR_JSON_FIELD && IsNumeric(b) ) );
	if ( a==SPH_ATTR_DOUBLE || b==SPH_ATTR_DOUBLE )
		return SPH_ATTR_DOUBLE;
	if ( a==SPH_ATTR_FLOAT || b==SPH_ATTR_FLOAT )
		return SPH_ATTR_FLOAT;
	if ( a==SPH_ATTR_BIGINT || b==SPH_ATTR_BIGINT )
		return SPH_ATTR_BIGINT;
	if ( a==SPH_ATTR_UINT64 || b==SPH_ATTR_UINT64 )
		return SPH_ATTR_BIGINT;
	if ( a==SPH_ATTR_JSON_FIELD || b==SPH_ATTR_JSON_FIELD )
		return SPH_ATTR_BIGINT;
	return SPH_ATTR_INTEGER;
}


uint64_t sphCalcLocatorHash ( const CSphAttrLocator & tLoc, uint64_t uPrevHash )
{
	auto uHash = uPrevHash;
	CALC_POD_HASH ( tLoc.m_iBitOffset );
	CALC_POD_HASH ( tLoc.m_iBitCount );
	CALC_POD_HASH ( tLoc.m_iBlobAttrId );
	CALC_POD_HASH ( tLoc.m_iBlobRowOffset );
	CALC_POD_HASH ( tLoc.m_nBlobAttrs );
	CALC_POD_HASH ( tLoc.m_bDynamic );
	return uHash;
}


uint64_t sphCalcExprDepHash ( const char * szTag, ISphExpr * pExpr, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	uint64_t uHash = sphFNV64 ( szTag, (int) strlen(szTag), uPrevHash );
	return sphCalcExprDepHash ( pExpr, tSorterSchema, uHash, bDisable );
}


uint64_t sphCalcExprDepHash ( ISphExpr * pExpr, const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	CSphVector<CSphString> dCols;
	pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCols );

	uint64_t uHash = uPrevHash;
	ARRAY_FOREACH ( i, dCols )
	{
		const CSphColumnInfo * pCol = tSorterSchema.GetAttr ( dCols[i].cstr() );
		assert(pCol);
		if ( pCol->m_pExpr )
		{
			// one more expression
			uHash = pCol->m_pExpr->GetHash ( tSorterSchema, uHash, bDisable );
			if ( bDisable )
				return 0;
		}
		else
			uHash = sphCalcLocatorHash ( pCol->m_tLocator, uHash ); // plain column, add locator to hash
	}

	return uHash;
}


int GetConstStrOffset ( int64_t iValue )
{
	return (int)( iValue>>32 );
}


int GetConstStrLength ( int64_t iValue )
{
	return (int)( iValue & 0xffffffffUL );
}

/////////////////////////////////////////////////////////////////////

void ConstList_c::Add ( int64_t iValue )
{
	if ( m_eRetType==SPH_ATTR_FLOAT )
		m_dFloats.Add ( (float)iValue );
	else
	{
		m_eRetType = WidestType ( m_eRetType, GetIntType(iValue) );
		m_dInts.Add(iValue);
	}
}

void ConstList_c::Add ( float fValue )
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

/////////////////////////////////////////////////////////////////////

Expr_Unary_c::Expr_Unary_c ( const char * szClassName, ISphExpr * pFirst )
	: m_pFirst ( pFirst )
	, m_szExprName ( szClassName )
{
	SafeAddRef ( pFirst );
}


void Expr_Unary_c::FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	if ( m_pFirst )
		m_pFirst->FixupLocator ( pOldSchema, pNewSchema );
}


void Expr_Unary_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( m_pFirst )
		m_pFirst->Command ( eCmd, pArg );
}


uint64_t Expr_Unary_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME_NOCHECK(m_szExprName);
	CALC_CHILD_HASH(m_pFirst);
	return CALC_DEP_HASHES();
}


Expr_Unary_c::Expr_Unary_c ( const Expr_Unary_c & rhs )
	: m_pFirst ( SafeClone (rhs.m_pFirst) )
	, m_szExprName ( rhs.m_szExprName )
{}


Expr_Binary_c::Expr_Binary_c ( const char * szClassName, ISphExpr * pFirst, ISphExpr * pSecond )
	: m_pFirst ( pFirst )
	, m_pSecond ( pSecond )
	, m_szExprName ( szClassName )
{
	SafeAddRef ( pFirst );
	SafeAddRef ( pSecond );
}


void Expr_Binary_c::FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	m_pFirst->FixupLocator ( pOldSchema, pNewSchema );

	if ( m_pSecond )
		m_pSecond->FixupLocator ( pOldSchema, pNewSchema );
}


void Expr_Binary_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	m_pFirst->Command ( eCmd, pArg );

	if ( m_pSecond )
		m_pSecond->Command ( eCmd, pArg );
}


uint64_t Expr_Binary_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME_NOCHECK(m_szExprName);
	CALC_CHILD_HASH(m_pFirst);
	CALC_CHILD_HASH(m_pSecond);
	return CALC_DEP_HASHES();
}


Expr_Binary_c::Expr_Binary_c ( const Expr_Binary_c & rhs )
	: m_pFirst ( SafeClone (rhs.m_pFirst) )
	, m_pSecond ( SafeClone (rhs.m_pSecond) )
	, m_szExprName ( rhs.m_szExprName )
{}
