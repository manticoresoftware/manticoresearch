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

#if USE_COLUMNAR

class GrouperColumnarInt_c : public CSphGrouper
{
public:
					GrouperColumnarInt_c ( const CSphColumnInfo & tAttr );
					GrouperColumnarInt_c ( const GrouperColumnarInt_c & rhs );

	void			GetLocator ( CSphAttrLocator & tOut ) const final {}
	ESphAttr		GetResultType () const final { return m_eAttrType; }
	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const final;
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final;
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


SphGroupKey_t GrouperColumnarInt_c::KeyFromValue ( SphAttr_t ) const
{
	assert(0);
	return SphGroupKey_t();
}


void GrouperColumnarInt_c::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	std::string sError; // fixme! report errors
	m_pIterator = pColumnar->CreateIterator ( m_sAttrName.cstr(), columnar::IteratorHints_t(), sError );
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
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final;
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
		return m_pIterator->GetStringHash();

	const BYTE * pStr = nullptr;
	int iLen = m_pIterator->Get(pStr);
	if ( !iLen )
		return 0;

	return HASH::Hash ( pStr, iLen );
}

template <typename HASH>
SphGroupKey_t GrouperColumnarString_T<HASH>::KeyFromValue ( SphAttr_t ) const
{
	assert(0);
	return SphGroupKey_t();
}

template <typename HASH>
void GrouperColumnarString_T<HASH>::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	columnar::IteratorHints_t tHints;
	tHints.m_bNeedStringHashes = m_eCollation==SPH_COLLATION_DEFAULT;

	std::string sError; // fixme! report errors
	m_pIterator = pColumnar->CreateIterator ( m_sAttrName.cstr(), tHints, sError );
	m_bHasHashes = tHints.m_bNeedStringHashes && m_pIterator.Ptr() && m_pIterator->HaveStringHashes();
}

template <typename HASH>
CSphGrouper * GrouperColumnarString_T<HASH>::Clone() const
{
	return new GrouperColumnarString_T<HASH>(*this);
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

#endif // USE_COLUMNAR