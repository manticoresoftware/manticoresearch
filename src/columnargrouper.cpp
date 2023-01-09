//
// Copyright (c) 2020-2023, Manticore Software LTD (https://manticoresearch.com)
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
	std::unique_ptr<columnar::Iterator_i>	m_pIterator;
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
	if ( m_pIterator && m_pIterator->AdvanceTo ( tMatch.m_tRowID ) == tMatch.m_tRowID )
		return m_pIterator->Get();

	return SphGroupKey_t(0);
}


void GrouperColumnarInt_c::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	std::string sError; // fixme! report errors
	m_pIterator = CreateColumnarIterator ( pColumnar, m_sAttrName.cstr(), sError );
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
	std::unique_ptr<columnar::Iterator_i>	m_pIterator;
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
	if ( !m_pIterator || m_pIterator->AdvanceTo ( tMatch.m_tRowID ) != tMatch.m_tRowID )
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
	m_pIterator = CreateColumnarIterator ( pColumnar, m_sAttrName.cstr(), sError, tHints, &tCapabilities );
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
	std::unique_ptr<columnar::Iterator_i> m_pIterator;
};

template <typename T>
void GrouperColumnarMVA_T<T>::MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const
{
	dKeys.Resize(0);

	if ( !m_pIterator || m_pIterator->AdvanceTo ( tMatch.m_tRowID ) != tMatch.m_tRowID )
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
	m_pIterator = CreateColumnarIterator ( pColumnar, m_sAttrName.cstr(),sError );
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

//////////////////////////////////////////////////////////////////////////

class DistinctFetcherColumnar_c : public DistinctFetcher_i
{
public:
			DistinctFetcherColumnar_c ( const CSphString & sName ) : m_sName(sName) {}

	void	SetBlobPool ( const BYTE * pBlobPool ) override {}
	void	FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override {}

protected:
	CSphString	m_sName;
	std::unique_ptr<columnar::Iterator_i> m_pIterator;
};


class DistinctFetcherColumnarInt_c : public DistinctFetcherColumnar_c
{
	using BASE = DistinctFetcherColumnar_c;
	using BASE::BASE;

public:
	void	GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	void	SetColumnar ( const columnar::Columnar_i * pColumnar ) override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherColumnarInt_c(m_sName); }
};


void DistinctFetcherColumnarInt_c::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	dKeys.Resize(0);

	if ( !m_pIterator || m_pIterator->AdvanceTo ( tMatch.m_tRowID ) != tMatch.m_tRowID )
		return;

	dKeys.Add ( m_pIterator->Get() );
}


void DistinctFetcherColumnarInt_c::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	std::string sError; // fixme! report errors
	m_pIterator = CreateColumnarIterator ( pColumnar, m_sName.cstr(), sError );
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
class DistinctFetcherColumnarMva_T : public DistinctFetcherColumnar_c
{
	using BASE = DistinctFetcherColumnar_c;
	using BASE::BASE;

public:
	void	GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	void	SetColumnar ( const columnar::Columnar_i * pColumnar ) override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherColumnarMva_T(m_sName); }
};

template <typename T>
void DistinctFetcherColumnarMva_T<T>::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	dKeys.Resize(0);

	if ( !m_pIterator || m_pIterator->AdvanceTo ( tMatch.m_tRowID ) != tMatch.m_tRowID )
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
void DistinctFetcherColumnarMva_T<T>::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	std::string sError; // fixme! report errors
	m_pIterator = CreateColumnarIterator ( pColumnar, m_sName.cstr(), sError );
}

//////////////////////////////////////////////////////////////////////////

template <typename HASH>
class DistinctFetcherColumnarString_T : public DistinctFetcherColumnar_c, public HASH
{
	using BASE = DistinctFetcherColumnar_c;
	using BASE::BASE;

public:
	void	GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	void	SetColumnar ( const columnar::Columnar_i * pColumnar ) override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherColumnarString_T<HASH>(m_sName); }

private:
	bool m_bHasHashes = false;
};

template <typename HASH>
void DistinctFetcherColumnarString_T<HASH>::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	dKeys.Resize(0);

	if ( !m_pIterator || m_pIterator->AdvanceTo ( tMatch.m_tRowID ) != tMatch.m_tRowID )
		return;

	if ( m_bHasHashes )
	{
		dKeys.Add ( m_pIterator->Get() );
		return;
	}

	const BYTE * pStr = nullptr;
	int iLen = m_pIterator->Get(pStr);
	if ( !iLen )
	{
		dKeys.Add(0);
		return;
	}

	dKeys.Add ( HASH::Hash ( pStr, iLen ) );
}

template <typename HASH>
void DistinctFetcherColumnarString_T<HASH>::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	columnar::IteratorHints_t tHints;
	columnar::IteratorCapabilities_t tCapabilities;
	tHints.m_bNeedStringHashes = true;

	std::string sError; // fixme! report errors
	m_pIterator = CreateColumnarIterator ( pColumnar, m_sName.cstr(), sError, tHints, &tCapabilities );
	m_bHasHashes = tCapabilities.m_bStringHashes;
}

//////////////////////////////////////////////////////////////////////////

DistinctFetcher_i * CreateColumnarDistinctFetcher ( const CSphString & sName, ESphAttr eType, ESphCollation eCollation )
{
	switch ( eType )
	{
	case SPH_ATTR_STRING:
		{
			switch ( eCollation )
			{
			case SPH_COLLATION_UTF8_GENERAL_CI:	return new DistinctFetcherColumnarString_T<Utf8CIHash_fn>(sName);
			case SPH_COLLATION_LIBC_CI:			return new DistinctFetcherColumnarString_T<LibcCIHash_fn>(sName);
			case SPH_COLLATION_LIBC_CS:			return new DistinctFetcherColumnarString_T<LibcCSHash_fn>(sName);
			default:							return new DistinctFetcherColumnarString_T<BinaryHash_fn>(sName);
			}
		}

	case SPH_ATTR_UINT32SET:return new DistinctFetcherColumnarMva_T<DWORD>(sName);
	case SPH_ATTR_INT64SET:	return new DistinctFetcherColumnarMva_T<int64_t>(sName);
	default:				return new DistinctFetcherColumnarInt_c(sName);
	}
}
