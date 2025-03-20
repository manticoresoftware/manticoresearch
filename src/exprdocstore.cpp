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

#include "exprdocstore.h"

#include "exprtraits.h"
#include "docstore.h"

//////////////////////////////////////////////////////////////////////////

template <bool POSTLIMIT>
class Expr_GetStored_T : public ISphExpr
{
public:
				Expr_GetStored_T ( CSphString sField, DocstoreDataType_e eDocstoreType, ESphAttr eAttrType );

	float		Eval ( const CSphMatch & tMatch ) const final;
	int			IntEval ( const CSphMatch & tMatch ) const final;
	int64_t		Int64Eval ( const CSphMatch & tMatch ) const final;
	bool		IsDataPtrAttr() const final		{ return sphIsBlobAttr(m_eAttrType); }
	bool		IsStored() const final			{ return !POSTLIMIT; }
	bool		UsesDocstore() const			{ return true; }
	int			StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const;
	const BYTE * StringEvalPacked ( const CSphMatch & tMatch ) const final { return GetBlobPacked(tMatch); }
	void		Command ( ESphExprCommand eCmd, void * pArg ) final;
	void		FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) final {}
	uint64_t	GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) final;
	ISphExpr *	Clone() const final			{ return new Expr_GetStored_T ( m_sField, m_eDocstoreType, m_eAttrType ); }

private:
	CSphString			m_sField;
	DocstoreDataType_e	m_eDocstoreType = DOCSTORE_TEXT;
	ESphAttr			m_eAttrType = SPH_ATTR_INTEGER;
	CSphVector<int>		m_dFieldIds;

	DocstoreSession_c::InfoRowID_t	m_tSessionRowID;
	DocstoreSession_c::InfoDocID_t	m_tSessionDocID;

	VecTraits_T<const BYTE>	GetBlob ( DocstoreDoc_t & tDoc, const CSphMatch & tMatch ) const;
	const BYTE *			GetBlobPacked ( const CSphMatch & tMatch ) const;
	template <typename T> T	ConvertBlobType ( const VecTraits_T<const BYTE> & dBlob ) const;
};

template <bool POSTLIMIT>
Expr_GetStored_T<POSTLIMIT>::Expr_GetStored_T ( CSphString sField, DocstoreDataType_e eDocstoreType, ESphAttr eAttrType )
	: m_sField ( std::move(sField) )
	, m_eDocstoreType ( eDocstoreType )
	, m_eAttrType ( eAttrType )
{}

template <bool POSTLIMIT>
float Expr_GetStored_T<POSTLIMIT>::Eval ( const CSphMatch & tMatch ) const
{
	assert ( m_eDocstoreType==DOCSTORE_ATTR );
	assert ( m_eAttrType!=SPH_ATTR_STRING && m_eAttrType!=SPH_ATTR_UINT32SET && m_eAttrType!=SPH_ATTR_INT64SET && m_eAttrType!=SPH_ATTR_BIGINT );

	DocstoreDoc_t tDoc;
	VecTraits_T<const BYTE> tBlob = GetBlob ( tDoc, tMatch );
	return tBlob.Begin() ? *(const float*)tBlob.Begin() : 0.0f;
}

template <bool POSTLIMIT>
int	Expr_GetStored_T<POSTLIMIT>::IntEval ( const CSphMatch & tMatch ) const
{
	assert ( m_eDocstoreType==DOCSTORE_ATTR );

	DocstoreDoc_t tDoc;
	return ConvertBlobType<DWORD> ( GetBlob ( tDoc, tMatch ) );
}

template <bool POSTLIMIT>
int64_t	Expr_GetStored_T<POSTLIMIT>::Int64Eval ( const CSphMatch & tMatch ) const
{
	assert ( m_eDocstoreType==DOCSTORE_ATTR );

	DocstoreDoc_t tDoc;
	switch ( m_eAttrType )
	{
	case SPH_ATTR_UINT32SET_PTR:
	case SPH_ATTR_INT64SET_PTR:
	case SPH_ATTR_FLOAT_VECTOR_PTR:
		return (int64_t)sphPackPtrAttr ( GetBlob ( tDoc, tMatch ) );

	default:
		return ConvertBlobType<int64_t> ( GetBlob ( tDoc, tMatch ) );
	}
}

template <bool POSTLIMIT>
int Expr_GetStored_T<POSTLIMIT>::StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
{
	DocstoreDoc_t tDoc;
	VecTraits_T<const BYTE> tRes = GetBlob ( tDoc, tMatch );
	if ( tDoc.m_dFields.GetLength() )
		*ppStr = tDoc.m_dFields[0].LeakData();
	return tRes.GetLength();
}

template <bool POSTLIMIT>
void Expr_GetStored_T<POSTLIMIT>::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( eCmd!=SPH_EXPR_SET_DOCSTORE_DOCID )
		return;

	m_dFieldIds.Resize(0);
	assert(pArg);
	m_tSessionDocID = *(DocstoreSession_c::InfoDocID_t*)pArg;
	assert ( m_tSessionDocID.m_pDocstore );
	int iFieldId = m_tSessionDocID.m_pDocstore->GetFieldId ( m_sField.cstr(), m_eDocstoreType );
	if ( iFieldId!=-1 )
		m_dFieldIds.Add(iFieldId);
}

template <bool POSTLIMIT>
uint64_t Expr_GetStored_T<POSTLIMIT>::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	EXPR_CLASS_NAME("Expr_GetStored_T");
	CALC_STR_HASH(m_sField, m_sField.Length());
	CALC_POD_HASHES(m_dFieldIds);
	return CALC_DEP_HASHES();
}

template <bool POSTLIMIT>
VecTraits_T<const BYTE>	Expr_GetStored_T<POSTLIMIT>::GetBlob ( DocstoreDoc_t & tDoc, const CSphMatch & tMatch ) const
{
	if ( !m_tSessionDocID.m_pDocstore || !m_dFieldIds.GetLength() )
		return { nullptr, 0 };

	DocID_t tDocID = sphGetDocID ( tMatch.m_pDynamic ? tMatch.m_pDynamic : tMatch.m_pStatic );
	if ( !m_tSessionDocID.m_pDocstore->GetDoc ( tDoc, tDocID, &m_dFieldIds, m_tSessionDocID.m_iSessionId, false ) )
		return { nullptr, 0 };

	if ( tDoc.m_dFields.GetLength() )
		return tDoc.m_dFields[0];
	else
		return { nullptr, 0 };
}

template <bool POSTLIMIT>
const BYTE * Expr_GetStored_T<POSTLIMIT>::GetBlobPacked ( const CSphMatch & tMatch ) const
{
	if ( !m_tSessionDocID.m_pDocstore || !m_dFieldIds.GetLength() )
		return nullptr;

	DocID_t tDocID = sphGetDocID ( tMatch.m_pDynamic ? tMatch.m_pDynamic : tMatch.m_pStatic );
	DocstoreDoc_t tDoc;
	if ( !m_tSessionDocID.m_pDocstore->GetDoc ( tDoc, tDocID, &m_dFieldIds, m_tSessionDocID.m_iSessionId, true ) )
		return nullptr;

	if ( tDoc.m_dFields.GetLength() )
		return tDoc.m_dFields[0].LeakData();
	else
		return nullptr;
}

template <bool POSTLIMIT>
template <typename T>
T Expr_GetStored_T<POSTLIMIT>::ConvertBlobType ( const VecTraits_T<const BYTE> & dBlob ) const
{
	int64_t iValue = 0;
	switch ( dBlob.GetLength() )
	{
	case 4:	iValue = *(const DWORD*)dBlob.Begin(); break;
	case 8: iValue = *(const int64_t*)dBlob.Begin(); break;
	default: break;
	}

	return (T)iValue;
}

template <>
void Expr_GetStored_T<false>::Command ( ESphExprCommand eCmd, void * pArg )
{
	switch ( eCmd )
	{
	case SPH_EXPR_GET_COLUMNAR_COL:
		*(CSphString*)pArg = m_sField;
		break;

	case SPH_EXPR_SET_DOCSTORE_ROWID:
		{
		m_dFieldIds.Resize(0);
		assert(pArg);
		m_tSessionRowID = *(DocstoreSession_c::InfoRowID_t*)pArg;
		assert ( m_tSessionRowID.m_pDocstore );
		int iFieldId = m_tSessionRowID.m_pDocstore->GetFieldId ( m_sField.cstr(), m_eDocstoreType );
		if ( iFieldId!=-1 )
			m_dFieldIds.Add(iFieldId);
	}
		break;

	default:
		break;
	}
}

template <>
VecTraits_T<const BYTE>	Expr_GetStored_T<false>::GetBlob ( DocstoreDoc_t & tDoc, const CSphMatch & tMatch ) const
{
	if ( !m_tSessionRowID.m_pDocstore || !m_dFieldIds.GetLength() )
		return { nullptr, 0 };

	tDoc = m_tSessionRowID.m_pDocstore->GetDoc ( tMatch.m_tRowID, &m_dFieldIds, m_tSessionRowID.m_iSessionId, false );
	if ( tDoc.m_dFields.GetLength() )
		return tDoc.m_dFields[0];
	else
		return { nullptr, 0 };
}

template <>
const BYTE * Expr_GetStored_T<false>::GetBlobPacked ( const CSphMatch & tMatch ) const
{
	if ( !m_tSessionRowID.m_pDocstore || !m_dFieldIds.GetLength() )
		return nullptr;

	DocstoreDoc_t tDoc = m_tSessionRowID.m_pDocstore->GetDoc ( tMatch.m_tRowID, &m_dFieldIds, m_tSessionRowID.m_iSessionId, true );
	if ( tDoc.m_dFields.GetLength() )
		return tDoc.m_dFields[0].LeakData();
	else
		return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

ISphExpr * CreateExpr_GetStoredField ( const CSphString & sName )
{
	return new Expr_GetStored_T<true> ( sName, DOCSTORE_TEXT, SPH_ATTR_STRING );
}

ISphExpr * CreateExpr_GetStoredAttr ( const CSphString & sName, ESphAttr eAttr )
{
	return new Expr_GetStored_T<false> ( sName, DOCSTORE_ATTR, eAttr );
}
