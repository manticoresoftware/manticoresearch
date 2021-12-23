//
// Copyright (c) 2020-2021, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "columnargrouper.h"
#include "sphinxsort.h"

class GrouperColumnarInt_c : public CSphGrouper
{
public:
					GrouperColumnarInt_c ( const CSphColumnInfo & tAttr );
					GrouperColumnarInt_c ( const GrouperColumnarInt_c & rhs );

	void			GetLocator ( CSphAttrLocator & tOut ) const final {}
	ESphAttr		GetResultType () const final { return m_eAttrType; }
	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const final;
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const override { assert(0); }
	void			SetColumnar ( const columnar::Columnar_i * pColumnar ) final;
	CSphGrouper *	Clone() const final;

private:
	ESphAttr							m_eAttrType = SPH_ATTR_INTEGER;
	CSphString							m_sAttrName;
	CSphScopedPtr<columnar::Iterator_i>	m_pIterator {nullptr};
};


GrouperColumnarInt_c::GrouperColumnarInt_c ( const CSphColumnInfo & tAttr )
	: m_eAttrType ( tAttr.m_eAttrType )
	, m_sAttrName ( tAttr.m_sName )
{}


GrouperColumnarInt_c::GrouperColumnarInt_c ( const GrouperColumnarInt_c & rhs )
	: m_eAttrType ( rhs.m_eAttrType )
	, m_sAttrName ( rhs.m_sAttrName )
{}


SphGroupKey_t GrouperColumnarInt_c::KeyFromMatch ( const CSphMatch & tMatch ) const
{
	if ( m_pIterator.Ptr() && m_pIterator->AdvanceTo ( tMatch.m_tRowID ) == tMatch.m_tRowID )
		return m_pIterator->Get();

	return SphGroupKey_t(0);
}


void GrouperColumnarInt_c::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	std::string sError; // fixme! report errors
	m_pIterator = pColumnar->CreateIterator ( m_sAttrName.cstr(), {}, nullptr, sError );
}


CSphGrouper * GrouperColumnarInt_c::Clone() const
{
	return new GrouperColumnarInt_c(*this);
}

//////////////////////////////////////////////////////////////////////////

template <typename HASH>
class GrouperColumnarString_T : public CSphGrouper, public HASH
{
public:
					GrouperColumnarString_T ( const CSphColumnInfo & tAttr, ESphCollation eCollation );
					GrouperColumnarString_T ( const GrouperColumnarString_T & rhs );

	void			GetLocator ( CSphAttrLocator & tOut ) const final {}
	ESphAttr		GetResultType () const final { return SPH_ATTR_BIGINT; }
	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const final;
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const final { assert(0); }
	void			SetColumnar ( const columnar::Columnar_i * pColumnar ) final;
	CSphGrouper *	Clone() const final;

private:
	CSphString							m_sAttrName;
	ESphCollation						m_eCollation = SPH_COLLATION_DEFAULT;
	CSphScopedPtr<columnar::Iterator_i>	m_pIterator {nullptr};
	bool								m_bHasHashes = false;
};

template <typename HASH>
GrouperColumnarString_T<HASH>::GrouperColumnarString_T ( const CSphColumnInfo & tAttr, ESphCollation eCollation )
	: m_sAttrName ( tAttr.m_sName )
	, m_eCollation ( eCollation )
{}

template <typename HASH>
GrouperColumnarString_T<HASH>::GrouperColumnarString_T ( const GrouperColumnarString_T & rhs )
	: m_sAttrName ( rhs.m_sAttrName )
	, m_eCollation ( rhs.m_eCollation )
{}

template <typename HASH>
SphGroupKey_t GrouperColumnarString_T<HASH>::KeyFromMatch ( const CSphMatch & tMatch ) const
{
	if ( !m_pIterator.Ptr() || m_pIterator->AdvanceTo ( tMatch.m_tRowID ) != tMatch.m_tRowID )
		return 0;

	if ( m_bHasHashes )
		return m_pIterator->Get();

	const BYTE * pStr = nullptr;
	int iLen = m_pIterator->Get(pStr);
	if ( !iLen )
		return 0;

	return HASH::Hash ( pStr, iLen );
}

template <typename HASH>
void GrouperColumnarString_T<HASH>::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	columnar::IteratorHints_t tHints;
	columnar::IteratorCapabilities_t tCapabilities;
	tHints.m_bNeedStringHashes = m_eCollation==SPH_COLLATION_DEFAULT;

	std::string sError; // fixme! report errors
	m_pIterator = pColumnar->CreateIterator ( m_sAttrName.cstr(), tHints, &tCapabilities, sError );
	m_bHasHashes = tCapabilities.m_bStringHashes;
}

template <typename HASH>
CSphGrouper * GrouperColumnarString_T<HASH>::Clone() const
{
	return new GrouperColumnarString_T<HASH>(*this);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
class GrouperColumnarMVA_T : public CSphGrouper
{
public:
					GrouperColumnarMVA_T ( const CSphColumnInfo & tAttr ) : m_sAttrName ( tAttr.m_sName ) {}
					GrouperColumnarMVA_T ( const GrouperColumnarMVA_T & rhs ) : m_sAttrName ( rhs.m_sAttrName ) {}

	void			GetLocator ( CSphAttrLocator & tOut ) const final {}
	ESphAttr		GetResultType () const final { return SPH_ATTR_BIGINT; }
	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const final { assert(0); return SphGroupKey_t(); }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const final;
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void			SetColumnar ( const columnar::Columnar_i * pColumnar ) final;
	CSphGrouper *	Clone() const final { return new GrouperColumnarMVA_T(*this); }
	bool			IsMultiValue() const final { return true; }

private:
	CSphString							m_sAttrName;
	CSphScopedPtr<columnar::Iterator_i>	m_pIterator {nullptr};
};

template <typename T>
void GrouperColumnarMVA_T<T>::MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const
{
	dKeys.Resize(0);

	if ( !m_pIterator.Ptr() || m_pIterator->AdvanceTo ( tMatch.m_tRowID ) != tMatch.m_tRowID )
		return;

	const BYTE * pMVA = nullptr;
	int iLen = m_pIterator->Get(pMVA);
	int iNumValues = iLen/sizeof(T);
	auto pValues = (const T*)pMVA;

	dKeys.Resize(iNumValues);
	for ( int i = 0; i < iNumValues; i++ )
		dKeys[i] = (SphGroupKey_t)pValues[i];
}

template <typename T>
void GrouperColumnarMVA_T<T>::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	std::string sError; // fixme! report errors
	m_pIterator = pColumnar->CreateIterator ( m_sAttrName.cstr(), {}, nullptr, sError );
}

//////////////////////////////////////////////////////////////////////////

CSphGrouper * CreateGrouperColumnarInt ( const CSphColumnInfo & tAttr )
{
	return new GrouperColumnarInt_c(tAttr);
}


CSphGrouper * CreateGrouperColumnarString ( const CSphColumnInfo & tAttr, ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI:	return new GrouperColumnarString_T<Utf8CIHash_fn> ( tAttr, eCollation );
	case SPH_COLLATION_LIBC_CI:			return new GrouperColumnarString_T<LibcCIHash_fn> ( tAttr, eCollation );
	case SPH_COLLATION_LIBC_CS:			return new GrouperColumnarString_T<LibcCSHash_fn> ( tAttr, eCollation );
	default:							return new GrouperColumnarString_T<BinaryHash_fn> ( tAttr, eCollation );
	}
}


CSphGrouper * CreateGrouperColumnarMVA ( const CSphColumnInfo & tAttr )
{
	if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_UINT32SET_PTR )
		return new GrouperColumnarMVA_T<DWORD>(tAttr);

	return new GrouperColumnarMVA_T<int64_t>(tAttr);
}
