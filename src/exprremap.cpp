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

#include "exprremap.h"

#include "sphinx.h"
#include "exprtraits.h"
#include "grouper.h"
#include "sphinxjson.h"

//////////////////////////////////////////////////////////////////////////
// expression that transform string pool base + offset -> ptr
class ExprSortStringAttrFixup_c : public BlobPool_c, public ISphExpr
{
public:
	explicit ExprSortStringAttrFixup_c ( const CSphAttrLocator & tLocator ) : m_tLocator ( tLocator ) {}

	float			Eval ( const CSphMatch & ) const override { assert ( 0 ); return 0.0f; }
	const BYTE *	StringEvalPacked ( const CSphMatch & tMatch ) const override;
	void			FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override { sphFixupLocator ( m_tLocator, pOldSchema, pNewSchema ); }
	void			Command ( ESphExprCommand eCmd, void * pArg ) override;
	uint64_t		GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) override;
	ISphExpr *		Clone() const final { return new ExprSortStringAttrFixup_c ( *this ); }

public:
	CSphAttrLocator		m_tLocator;				///< string attribute to fix

private:
	ExprSortStringAttrFixup_c ( const ExprSortStringAttrFixup_c & rhs ) : m_tLocator ( rhs.m_tLocator ) {}
};


const BYTE * ExprSortStringAttrFixup_c::StringEvalPacked ( const CSphMatch & tMatch ) const
{
	// our blob strings are not null-terminated!
	// we can either store nulls in .SPB or add them here
	return sphPackPtrAttr ( sphGetBlobAttr ( tMatch, m_tLocator, GetBlobPool() ) );
}


void ExprSortStringAttrFixup_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
		SetBlobPool( (const BYTE*)pArg);
}


uint64_t ExprSortStringAttrFixup_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME_NOCHECK("ExprSortStringAttrFixup_c");
	uHash = sphFNV64 ( &m_tLocator, sizeof(m_tLocator), uHash );

	return CALC_DEP_HASHES();
}


// expression that transform string pool base + offset -> ptr
class ExprSortJson2StringPtr_c : public BlobPool_c, public ISphExpr
{
public:
				ExprSortJson2StringPtr_c ( const CSphAttrLocator & tLocator, ISphExpr * pExpr );

	bool		IsDataPtrAttr () const final { return true; }
	float		Eval ( const CSphMatch & ) const override { assert ( 0 ); return 0.0f; }
	int			StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const override;
	void		FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override;
	void		Command ( ESphExprCommand eCmd, void * pArg ) override;
	uint64_t	GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) override;
	ISphExpr *	Clone() const final { return new ExprSortJson2StringPtr_c ( *this ); }

private:
	CSphAttrLocator		m_tJsonCol;				///< JSON attribute to fix
	ISphExprRefPtr_c	m_pExpr;

private:
				ExprSortJson2StringPtr_c ( const ExprSortJson2StringPtr_c & rhs );
};


ExprSortJson2StringPtr_c::ExprSortJson2StringPtr_c ( const CSphAttrLocator & tLocator, ISphExpr * pExpr )
	: m_tJsonCol ( tLocator )
	, m_pExpr ( pExpr )
{
	if ( pExpr ) // adopt the expression
		pExpr->AddRef();
}


int ExprSortJson2StringPtr_c::StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
{
	if ( !GetBlobPool() || !m_pExpr )
	{
		*ppStr = nullptr;
		return 0;
	}

	uint64_t uPacked = m_pExpr->Int64Eval ( tMatch );
	CSphString sResult = FormatJsonAsSortStr ( GetBlobPool() + sphJsonUnpackOffset(uPacked), sphJsonUnpackType(uPacked) );
	int iStrLen = sResult.Length();
	*ppStr = (const BYTE *)sResult.Leak();
	return iStrLen;
}


void ExprSortJson2StringPtr_c::FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	sphFixupLocator ( m_tJsonCol, pOldSchema, pNewSchema );
	if ( m_pExpr )
		m_pExpr->FixupLocator ( pOldSchema, pNewSchema );
}


void ExprSortJson2StringPtr_c::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
	{
		SetBlobPool((const BYTE*)pArg);
		if ( m_pExpr )
			m_pExpr->Command ( eCmd, pArg );
	}
}


uint64_t ExprSortJson2StringPtr_c::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME_NOCHECK("ExprSortJson2StringPtr_c");
	CALC_CHILD_HASH(m_pExpr);

	// uHash = sphFNV64 ( &m_tJsonCol, sizeof ( m_tJsonCol ), uHash );	//< that is wrong! Locator may have padding uninitialized data, valgrind will warn!
	uHash = sphCalcLocatorHash ( m_tJsonCol, uHash );					//< that is right, only meaningful fields processed without padding.

	return CALC_DEP_HASHES();
}


ExprSortJson2StringPtr_c::ExprSortJson2StringPtr_c ( const ExprSortJson2StringPtr_c & rhs )
	: m_tJsonCol ( rhs.m_tJsonCol )
	, m_pExpr ( SafeClone (rhs.m_pExpr) )
{}


///////////////////////////////////////////////////////////////////////////////

ISphExpr * CreateExprSortStringFixup ( const CSphAttrLocator & tLoc )
{
	return new ExprSortStringAttrFixup_c(tLoc);
}


ISphExpr * CreateExprSortJson2String ( const CSphAttrLocator & tLoc, ISphExpr * pExpr )
{
	return new ExprSortJson2StringPtr_c ( tLoc, pExpr );
}
