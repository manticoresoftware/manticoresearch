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


template <typename T>
static inline void FetchMVAValues ( const std::unique_ptr<columnar::Iterator_i> & pIterator, CSphVector<SphGroupKey_t> & dKeys, const CSphMatch & tMatch )
{
	const BYTE * pMVA = nullptr;
	int iLen = pIterator->Get ( tMatch.m_tRowID, pMVA );
	int iNumValues = iLen/sizeof(T);
	auto pValues = (const T*)pMVA;

	dKeys.Resize(iNumValues);
	for ( int i = 0; i < iNumValues; i++ )
		dKeys[i] = (SphGroupKey_t)pValues[i];
}


bool NextSet ( CSphFixedVector<int> & dSet, const CSphFixedVector<CSphVector<int64_t>> & dAllKeys )
{
	for ( int i = 0; i < dSet.GetLength(); i++ )
	{
		int iMaxValues = dAllKeys[i].GetLength();
		if ( !iMaxValues )
			continue;

		dSet[i]++;
		if ( dSet[i]>=iMaxValues )
			dSet[i] = 0;
		else
			return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////

class GrouperColumnarInt_c : public CSphGrouper
{
public:
					GrouperColumnarInt_c ( const CSphColumnInfo & tAttr );
					GrouperColumnarInt_c ( const GrouperColumnarInt_c & rhs );

	void			GetLocator ( CSphAttrLocator & tOut ) const final {}
	ESphAttr		GetResultType () const final { return m_eAttrType; }
	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const final { return m_pIterator->Get ( tMatch.m_tRowID ); }
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const override { assert(0); }
	void			SetColumnar ( const columnar::Columnar_i * pColumnar ) final;
	CSphGrouper *	Clone() const final { return new GrouperColumnarInt_c(*this); }

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


void GrouperColumnarInt_c::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	std::string sError; // fixme! report errors
	m_pIterator = CreateColumnarIterator ( pColumnar, m_sAttrName.cstr(), sError );
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
	if ( m_bHasHashes )
		return m_pIterator->Get ( tMatch.m_tRowID );

	const BYTE * pStr = nullptr;
	int iLen = m_pIterator->Get ( tMatch.m_tRowID, pStr );
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
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const final { FetchMVAValues<T> ( m_pIterator, dKeys, tMatch ); }
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void			SetColumnar ( const columnar::Columnar_i * pColumnar ) final;
	CSphGrouper *	Clone() const final { return new GrouperColumnarMVA_T(*this); }
	bool			IsMultiValue() const final { return true; }

private:
	CSphString							m_sAttrName;
	std::unique_ptr<columnar::Iterator_i> m_pIterator;
};

template <typename T>
void GrouperColumnarMVA_T<T>::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	std::string sError; // fixme! report errors
	m_pIterator = CreateColumnarIterator ( pColumnar, m_sAttrName.cstr(),sError );
}

//////////////////////////////////////////////////////////////////////////

template <class HASH>
class GrouperColumnarMulti final : public CSphGrouper, public HASH
{
public:
					GrouperColumnarMulti ( const CSphVector<CSphColumnInfo> & dAttrs, ESphCollation eCollation );

	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const final;
	void			SetColumnar ( const columnar::Columnar_i * pColumnar ) final;
	CSphGrouper *	Clone() const final { return new GrouperColumnarMulti<HASH> ( m_dAttrs, m_eCollation ); }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const final;
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void			GetLocator ( CSphAttrLocator & ) const final { assert(0); }
	ESphAttr		GetResultType() const final { return SPH_ATTR_BIGINT; }
	bool			IsMultiValue() const final;

private:
	CSphVector<CSphColumnInfo>		m_dAttrs;
	ESphCollation					m_eCollation = SPH_COLLATION_DEFAULT;
	CSphVector<std::unique_ptr<columnar::Iterator_i>> m_dIterators;
	CSphVector<bool>				m_dHaveStringHashes;
	const columnar::Columnar_i *	m_pColumnar = nullptr;

	SphGroupKey_t	FetchStringHash ( int iAttr, const CSphMatch & tMatch, SphGroupKey_t tPrevKey ) const;
	SphGroupKey_t	FetchStringHash ( int iAttr, const CSphMatch & tMatch ) const;
	void			SpawnIterators();
};

template <class HASH>
GrouperColumnarMulti<HASH>::GrouperColumnarMulti ( const CSphVector<CSphColumnInfo> & dAttrs, ESphCollation eCollation )
	: m_dAttrs ( dAttrs )
	, m_eCollation ( eCollation )
{
	assert ( dAttrs.GetLength()>1 );
}

template <class HASH>
SphGroupKey_t GrouperColumnarMulti<HASH>::KeyFromMatch ( const CSphMatch & tMatch ) const
{
	auto tKey = ( SphGroupKey_t ) SPH_FNV64_SEED;

	for ( int i=0; i<m_dAttrs.GetLength(); i++ )
	{
		auto & pIterator = m_dIterators[i];

		if ( m_dAttrs[i].m_eAttrType==SPH_ATTR_STRING || m_dAttrs[i].m_eAttrType==SPH_ATTR_STRINGPTR )
			tKey = FetchStringHash ( i, tMatch, tKey );
		else
			tKey = ( SphGroupKey_t ) sphFNV64 ( pIterator->Get ( tMatch.m_tRowID ), tKey );
	}

	return tKey;
}

template <class HASH>
void GrouperColumnarMulti<HASH>::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	CSphGrouper::SetColumnar ( pColumnar );
	m_pColumnar = pColumnar;
	SpawnIterators();
}

template <class HASH>
void GrouperColumnarMulti<HASH>::MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const
{
	dKeys.Resize(0);

	CSphFixedVector<CSphVector<SphGroupKey_t>> dAllKeys { m_dAttrs.GetLength() };
	for ( int i=0; i<m_dAttrs.GetLength(); i++ )
	{
		auto & dCurKeys = dAllKeys[i];
		auto & pIterator = m_dIterators[i];

		switch ( m_dAttrs[i].m_eAttrType )
		{
		case SPH_ATTR_UINT32SET:
			FetchMVAValues<DWORD> ( pIterator, dKeys, tMatch );
			break;

		case SPH_ATTR_INT64SET:
			FetchMVAValues<int64_t> ( pIterator, dKeys, tMatch );
			break;

		case SPH_ATTR_STRING:
		case SPH_ATTR_STRINGPTR:
			{
				SphGroupKey_t tStringKey = FetchStringHash ( i, tMatch );
				if ( tStringKey!=(SphGroupKey_t)SPH_FNV64_SEED )
					dCurKeys.Add ( tStringKey );
			}
			break;

		default:
			dCurKeys.Add ( pIterator->Get ( tMatch.m_tRowID ) );
			break;
		}
	}

	CSphFixedVector<int> dIndexes { m_dAttrs.GetLength() };
	dIndexes.ZeroVec();

	do
	{
		auto tKey = ( SphGroupKey_t ) SPH_FNV64_SEED;
		ARRAY_FOREACH ( i, dAllKeys )
			if ( dAllKeys[i].GetLength() )
				tKey = (SphGroupKey_t)sphFNV64 ( dAllKeys[i][dIndexes[i]], tKey );

		dKeys.Add(tKey);
	}
	while ( NextSet ( dIndexes, dAllKeys ) );
}

template <class HASH>
bool GrouperColumnarMulti<HASH>::IsMultiValue() const
{
	return m_dAttrs.any_of ( []( auto & tAttr ){ return tAttr.m_eAttrType==SPH_ATTR_JSON || tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET; } );
}

template <class HASH>
SphGroupKey_t GrouperColumnarMulti<HASH>::FetchStringHash ( int iAttr, const CSphMatch & tMatch, SphGroupKey_t tPrevKey ) const
{
	auto & pIterator = m_dIterators[iAttr];
	if ( m_dHaveStringHashes[iAttr] )
		return sphFNV64 ( pIterator->Get ( tMatch.m_tRowID ), tPrevKey );

	const BYTE * pStr = nullptr;
	int iLen = pIterator->Get ( tMatch.m_tRowID, pStr );
	if ( !iLen )
		return tPrevKey;

	return HASH::Hash ( pStr, iLen, tPrevKey );
}

template <class HASH>
SphGroupKey_t GrouperColumnarMulti<HASH>::FetchStringHash ( int iAttr, const CSphMatch & tMatch ) const
{
	auto & pIterator = m_dIterators[iAttr];
	if ( m_dHaveStringHashes[iAttr] )
		return pIterator->Get ( tMatch.m_tRowID );

	const BYTE * pStr = nullptr;
	int iLen = pIterator->Get ( tMatch.m_tRowID, pStr );
	if ( !iLen )
		return SPH_FNV64_SEED;

	return HASH::Hash ( pStr, iLen, SPH_FNV64_SEED );
}

template <class HASH>
void GrouperColumnarMulti<HASH>::SpawnIterators()
{
	m_dHaveStringHashes.Resize ( m_dAttrs.GetLength() );
	m_dHaveStringHashes.Fill(false);
	m_dIterators.Resize ( m_dAttrs.GetLength() );

	ARRAY_FOREACH ( i, m_dAttrs )
	{
		const auto & tAttr = m_dAttrs[i];
		assert ( tAttr.IsColumnar() || tAttr.IsColumnarExpr() );

		std::string sError; // fixme! report errors
		if ( tAttr.m_eAttrType==SPH_ATTR_STRING || tAttr.m_eAttrType==SPH_ATTR_STRINGPTR )
		{
			columnar::IteratorHints_t tHints;
			columnar::IteratorCapabilities_t tCapabilities;
			tHints.m_bNeedStringHashes = m_eCollation==SPH_COLLATION_DEFAULT;

			m_dIterators[i] = CreateColumnarIterator ( m_pColumnar, tAttr.m_sName.cstr(), sError, tHints, &tCapabilities );
			m_dHaveStringHashes[i] = tCapabilities.m_bStringHashes;
		}
		else
			m_dIterators[i] = CreateColumnarIterator ( m_pColumnar, tAttr.m_sName.cstr(), sError );
	}
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


CSphGrouper * CreateGrouperColumnarMulti ( const CSphVector<CSphColumnInfo> & dAttrs, ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI:	return new GrouperColumnarMulti<Utf8CIHash_fn> ( dAttrs, eCollation );
	case SPH_COLLATION_LIBC_CI:			return new GrouperColumnarMulti<LibcCIHash_fn> ( dAttrs, eCollation );
	case SPH_COLLATION_LIBC_CS:			return new GrouperColumnarMulti<LibcCSHash_fn> ( dAttrs, eCollation );
	default:							return new GrouperColumnarMulti<BinaryHash_fn> ( dAttrs, eCollation );
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


class DistinctFetcherColumnarPlain_c : public DistinctFetcherColumnar_c
{
	using DistinctFetcherColumnar_c::DistinctFetcherColumnar_c;

public:
	void	GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override { assert ( 0 && " Requesting multiple keys from plain distinct fetcher" ); }
	bool	IsMultiValue() const override { return false; }
};


class DistinctFetcherColumnarMulti_c : public DistinctFetcherColumnar_c
{
	using DistinctFetcherColumnar_c::DistinctFetcherColumnar_c;

public:
	SphAttr_t	GetKey ( const CSphMatch & tMatch ) const override { assert ( 0 && " Requesting single keys from multi distinct fetcher" ); return 0; }
	bool		IsMultiValue() const override { return true; }
};


class DistinctFetcherColumnarInt_c : public DistinctFetcherColumnarPlain_c
{
	using DistinctFetcherColumnarPlain_c::DistinctFetcherColumnarPlain_c;

public:
	SphAttr_t	GetKey ( const CSphMatch & tMatch ) const override { return m_pIterator->Get ( tMatch.m_tRowID ); }
	void		SetColumnar ( const columnar::Columnar_i * pColumnar ) override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherColumnarInt_c(m_sName); }
};


void DistinctFetcherColumnarInt_c::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	assert(pColumnar);
	std::string sError; // fixme! report errors
	m_pIterator = CreateColumnarIterator ( pColumnar, m_sName.cstr(), sError );
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
class DistinctFetcherColumnarMva_T : public DistinctFetcherColumnarMulti_c
{
	using DistinctFetcherColumnarMulti_c::DistinctFetcherColumnarMulti_c;

public:
	void	GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	void	SetColumnar ( const columnar::Columnar_i * pColumnar ) override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherColumnarMva_T(m_sName); }
};

template <typename T>
void DistinctFetcherColumnarMva_T<T>::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	const BYTE * pMVA = nullptr;
	int iLen = m_pIterator->Get ( tMatch.m_tRowID, pMVA );
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
class DistinctFetcherColumnarString_T : public DistinctFetcherColumnarPlain_c, public HASH
{
	using DistinctFetcherColumnarPlain_c::DistinctFetcherColumnarPlain_c;

public:
	SphAttr_t	GetKey ( const CSphMatch & tMatch ) const override;
	void		SetColumnar ( const columnar::Columnar_i * pColumnar ) override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherColumnarString_T<HASH>(m_sName); }

private:
	bool		m_bHasHashes = false;
};

template <typename HASH>
SphAttr_t DistinctFetcherColumnarString_T<HASH>::GetKey ( const CSphMatch & tMatch ) const
{
	if ( m_bHasHashes )
		return m_pIterator->Get ( tMatch.m_tRowID );

	const BYTE * pStr = nullptr;
	int iLen = m_pIterator->Get ( tMatch.m_tRowID, pStr );
	if ( !iLen )
		return 0;

	return HASH::Hash ( pStr, iLen );
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
