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

#include "grouper.h"
#include "datetime.h"
#include "exprdatetime.h"
#include "sphinxjson.h"
#include "sphinxint.h"
#include "columnargrouper.h"

class GrouperTraits_c : public CSphGrouper
{
public:
					explicit GrouperTraits_c ( const CSphAttrLocator & tLoc ) : m_tLocator ( tLoc ) {}

	void			GetLocator ( CSphAttrLocator & tOut ) const override		{ tOut = m_tLocator; }
	ESphAttr		GetResultType() const override								{ return m_tLocator.m_iBitCount>8*(int)sizeof(DWORD) ? SPH_ATTR_BIGINT : SPH_ATTR_INTEGER; }
	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const override	{ return KeyFromValue ( tMatch.GetAttr ( m_tLocator ) ); }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const override { assert(0); }

protected:
	CSphAttrLocator m_tLocator;
};


class GrouperAttr_c : public GrouperTraits_c
{
	using GrouperTraits_c::GrouperTraits_c;

public:
	CSphGrouper *	Clone() const override								{ return new GrouperAttr_c(m_tLocator); }
	SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const override	{ return uValue; }
};

template <bool UTC>
class GrouperDay_T : public GrouperTraits_c
{
	using GrouperTraits_c::GrouperTraits_c;

public:
	CSphGrouper *	Clone() const override								{ return new GrouperDay_T(m_tLocator); }
	SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const override	{ return CalcYearMonthDay ( ConvertGroupbyTime<UTC> ( (time_t)uValue ) ); }
};

template <bool UTC>
class GrouperWeek_T : public GrouperTraits_c
{
	using GrouperTraits_c::GrouperTraits_c;

public:
	CSphGrouper *	Clone() const override								{ return new GrouperWeek_T(m_tLocator); }
	SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const override	{ return CalcYearWeek ( ConvertGroupbyTime<UTC> ( (time_t)uValue ) ); }
};

template <bool UTC>
class GrouperMonth_T : public GrouperTraits_c
{
	using GrouperTraits_c::GrouperTraits_c;

public:
	CSphGrouper *	Clone() const override								{ return new GrouperMonth_T(m_tLocator); }
	SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const override	{ return CalcYearMonth ( ConvertGroupbyTime<UTC> ( (time_t)uValue ) ); }
};

template <bool UTC>
class GrouperYear_T : public GrouperTraits_c
{
	using GrouperTraits_c::GrouperTraits_c;

public:
	CSphGrouper *	Clone() const override								{ return new GrouperYear_T(m_tLocator); }
	SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const override	{ return ConvertGroupbyTime<UTC> ( (time_t)uValue ).year(); }
};

template <class PRED>
class CSphGrouperString final : public GrouperAttr_c, public PRED
{
public:
	explicit CSphGrouperString ( const CSphAttrLocator & tLoc ) : GrouperAttr_c ( tLoc ) {}

	ESphAttr		GetResultType () const override { return SPH_ATTR_BIGINT; }
	CSphGrouper *	Clone() const final { return new CSphGrouperString ( m_tLocator ); }

	SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const override
	{
		auto dBlobAttr = tMatch.FetchAttrData ( m_tLocator, GetBlobPool() );
		if ( IsEmpty ( dBlobAttr ) )
			return 0;

		return PRED::Hash ( dBlobAttr.first,dBlobAttr.second );
	}
};

/// lookup JSON key, group by looked up value (used in CSphKBufferJsonGroupSorter)
class CSphGrouperJsonField final : public CSphGrouper
{
public:
	CSphGrouperJsonField ( const CSphAttrLocator & tLoc, ISphExpr * pExpr )
		: m_tLocator ( tLoc )
		, m_pExpr ( pExpr )
	{
		SafeAddRef ( pExpr );
	}

	void SetBlobPool ( const BYTE * pBlobPool ) final
	{
		CSphGrouper::SetBlobPool ( pBlobPool );
		if ( m_pExpr )
			m_pExpr->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
	}

	SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const final
	{
		if ( !m_pExpr )
			return SphGroupKey_t();

		return m_pExpr->Int64Eval ( tMatch );
	}

	void			GetLocator ( CSphAttrLocator & tOut ) const final	{ tOut = m_tLocator; }
	ESphAttr		GetResultType() const final							{ return SPH_ATTR_BIGINT; }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const final { assert(0); }
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final				{ assert(0); return SphGroupKey_t(); }
	CSphGrouper *	Clone() const final									{ return new CSphGrouperJsonField (*this); }

protected:
	CSphGrouperJsonField ( const CSphGrouperJsonField & rhs )
		: m_tLocator ( rhs.m_tLocator )
		, m_pExpr ( SafeClone ( rhs.m_pExpr ) )
	{}

	CSphAttrLocator		m_tLocator;
	ISphExprRefPtr_c	m_pExpr;
};


template <class PRED>
class GrouperStringExpr_T final : public CSphGrouper, public PRED
{
public:
	explicit GrouperStringExpr_T ( ISphExpr * pExpr )
		: m_pExpr ( pExpr )
	{
		assert(m_pExpr);
		SafeAddRef(pExpr);
	}

	void			GetLocator ( CSphAttrLocator & tOut ) const final {}
	ESphAttr		GetResultType () const final { return SPH_ATTR_BIGINT; }
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const final { assert(0); }

	SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const final
	{
		assert ( !m_pExpr->IsDataPtrAttr() );
		const BYTE * pStr = nullptr;
		int iLen = m_pExpr->StringEval ( tMatch, &pStr );
		if ( !iLen )
			return 0;

		return PRED::Hash ( pStr, iLen );
	}

	CSphGrouper *	Clone() const final { return new GrouperStringExpr_T(*this); }
	void			SetColumnar ( const columnar::Columnar_i * pColumnar ) final { m_pExpr->Command ( SPH_EXPR_SET_COLUMNAR, (void*)pColumnar ); }

protected:
	GrouperStringExpr_T (const GrouperStringExpr_T& rhs)
		: m_pExpr { SafeClone ( rhs.m_pExpr ) }
	{}

	ISphExprRefPtr_c	m_pExpr;
};

template <typename MVA, typename ADDER>
static void AddGroupedMVA ( ADDER && fnAdd, const ByteBlob_t& dRawMVA )
{
	VecTraits_T<MVA> dMvas {dRawMVA};
	for ( auto & tValue : dMvas )
		fnAdd ( sphUnalignedRead(tValue) );
}

template<typename T, bool PTR>
static void FetchMVAKeys ( CSphVector<SphGroupKey_t> & dKeys, const CSphMatch & tMatch, const CSphAttrLocator & tLocator, const BYTE * pBlobPool )
{
	dKeys.Resize(0);

	int iLengthBytes = 0;
	const BYTE * pMva = nullptr;

	if constexpr ( PTR )
	{
		auto pPacked = (const BYTE *)tMatch.GetAttr(tLocator);
		ByteBlob_t dUnpacked = sphUnpackPtrAttr(pPacked);
		pMva = dUnpacked.first;
		iLengthBytes = dUnpacked.second;
	}
	else
	{
		if ( !pBlobPool )
			return;

		pMva = sphGetBlobAttr ( tMatch, tLocator, pBlobPool, iLengthBytes );
	}

	int iNumValues = iLengthBytes / sizeof(T);
	const T * pValues = (const T*)pMva;

	dKeys.Resize(iNumValues);
	for ( int i = 0; i<iNumValues; i++ )
		dKeys[i] = (SphGroupKey_t)pValues[i];
}

template <class PRED, bool HAVE_COLUMNAR>
class CSphGrouperMulti final: public CSphGrouper, public PRED
{
	using MYTYPE = CSphGrouperMulti<PRED,HAVE_COLUMNAR>;

public:
					CSphGrouperMulti ( const CSphVector<CSphColumnInfo> & dAttrs, VecRefPtrs_t<ISphExpr *> dJsonKeys, ESphCollation eCollation );

	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const final;
	void			SetBlobPool ( const BYTE * pBlobPool ) final;
	void			SetColumnar ( const columnar::Columnar_i * pColumnar ) final;
	CSphGrouper *	Clone() const final;
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const final;
	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const final { assert(0); return SphGroupKey_t(); }
	void			GetLocator ( CSphAttrLocator & ) const final { assert(0); }
	ESphAttr		GetResultType() const final { return SPH_ATTR_BIGINT; }
	bool			IsMultiValue() const final;

private:
	CSphVector<CSphColumnInfo>	m_dAttrs;
	VecRefPtrs_t<ISphExpr *>	m_dJsonKeys;
	ESphCollation				m_eCollation = SPH_COLLATION_DEFAULT;
	CSphVector<CSphRefcountedPtr<CSphGrouper>> m_dSingleKeyGroupers;
	CSphVector<CSphRefcountedPtr<CSphGrouper>> m_dMultiKeyGroupers;

	SphGroupKey_t	FetchStringKey ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, SphGroupKey_t tPrevKey ) const;
	void			SpawnColumnarGroupers();
};

template <class PRED, bool HAVE_COLUMNAR>
CSphGrouperMulti<PRED,HAVE_COLUMNAR>::CSphGrouperMulti ( const CSphVector<CSphColumnInfo> & dAttrs, VecRefPtrs_t<ISphExpr *> dJsonKeys, ESphCollation eCollation )
	: m_dAttrs ( dAttrs )
	, m_dJsonKeys ( std::move(dJsonKeys) )
	, m_eCollation ( eCollation )
{
	assert ( dAttrs.GetLength()>1 );
	assert ( dAttrs.GetLength()==m_dJsonKeys.GetLength() );
	
	if constexpr ( HAVE_COLUMNAR )
		SpawnColumnarGroupers();
}

template <class PRED, bool HAVE_COLUMNAR>
SphGroupKey_t CSphGrouperMulti<PRED,HAVE_COLUMNAR>::KeyFromMatch ( const CSphMatch & tMatch ) const
{
	auto tKey = ( SphGroupKey_t ) SPH_FNV64_SEED;

	for ( int i=0; i<m_dAttrs.GetLength(); i++ )
	{
		if constexpr ( HAVE_COLUMNAR )
		{
			if ( m_dSingleKeyGroupers[i] )
			{
				// use pre-spawned grouper
				SphGroupKey_t tColumnarKey = m_dSingleKeyGroupers[i]->KeyFromMatch(tMatch);
				tKey = ( SphGroupKey_t ) sphFNV64 ( tColumnarKey, tKey );
				continue;
			}
		}

		switch ( m_dAttrs[i].m_eAttrType )
		{
		case SPH_ATTR_STRING:
		case SPH_ATTR_STRINGPTR:
			tKey = FetchStringKey ( tMatch, m_dAttrs[i].m_tLocator, tKey );
			break;

		default:
			{
				SphAttr_t tAttr = tMatch.GetAttr ( m_dAttrs[i].m_tLocator );
				tKey = ( SphGroupKey_t ) sphFNV64 ( tAttr, tKey );
			}
			break;
		}
	}

	return tKey;
}

template <class PRED, bool HAVE_COLUMNAR>
void CSphGrouperMulti<PRED, HAVE_COLUMNAR>::SetBlobPool ( const BYTE * pBlobPool )
{
	CSphGrouper::SetBlobPool ( pBlobPool );
	for ( auto & i : m_dJsonKeys )
		if ( i )
			i->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
	// might be JSON whole attribute not the json.field stored in the m_dJsonKeys
	for ( auto & tAttr : m_dAttrs )
	{
		if ( tAttr.m_pExpr )
			tAttr.m_pExpr->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
	}
}

template <class PRED, bool HAVE_COLUMNAR>
void CSphGrouperMulti<PRED,HAVE_COLUMNAR>::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	CSphGrouper::SetColumnar ( pColumnar );

	for ( auto & i : m_dSingleKeyGroupers )
		if ( i )
			i->SetColumnar ( pColumnar );

	for ( auto & i : m_dMultiKeyGroupers )
		if ( i )
			i->SetColumnar ( pColumnar );
}

template <class PRED, bool HAVE_COLUMNAR>
CSphGrouper * CSphGrouperMulti<PRED,HAVE_COLUMNAR>::Clone() const
{
	VecRefPtrs_t<ISphExpr *> dJsonKeys;
	m_dJsonKeys.for_each ( [&dJsonKeys] ( ISphExpr * p ) { dJsonKeys.Add ( SafeClone ( p ) ); } );
	return new MYTYPE ( m_dAttrs, std::move(dJsonKeys), m_eCollation );
}

template <class PRED, bool HAVE_COLUMNAR>
void CSphGrouperMulti<PRED,HAVE_COLUMNAR>::MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const
{
	dKeys.Resize(0);

	CSphFixedVector<CSphVector<SphGroupKey_t>> dAllKeys { m_dAttrs.GetLength() };
	for ( int i=0; i<m_dAttrs.GetLength(); i++ )
	{
		auto & dCurKeys = dAllKeys[i];

		if constexpr ( HAVE_COLUMNAR )
		{
			if ( m_dMultiKeyGroupers[i] )
			{
				// use pre-spawned grouper
				m_dMultiKeyGroupers[i]->MultipleKeysFromMatch ( tMatch, dCurKeys );
				continue;
			}
		}

		switch ( m_dAttrs[i].m_eAttrType )
		{
		case SPH_ATTR_UINT32SET:
			FetchMVAKeys<DWORD,false> ( dCurKeys, tMatch, m_dAttrs[i].m_tLocator, GetBlobPool() );
			break;

		case SPH_ATTR_INT64SET:
			FetchMVAKeys<int64_t,false> ( dCurKeys, tMatch, m_dAttrs[i].m_tLocator, GetBlobPool() );
			break;

		case SPH_ATTR_JSON:
			PushJsonField ( m_dJsonKeys[i]->Int64Eval(tMatch), m_pBlobPool, [&dCurKeys]( SphAttr_t * pAttr, SphGroupKey_t uMatchGroupKey ){ dCurKeys.Add(uMatchGroupKey); return true; } );
			break;

		case SPH_ATTR_JSON_FIELD:
			{
				assert ( m_dAttrs[i].m_pExpr );
				PushJsonField ( m_dAttrs[i].m_pExpr->Int64Eval ( tMatch ), m_pBlobPool, [&dCurKeys]( SphAttr_t * pAttr, SphGroupKey_t uMatchGroupKey )
				{
					dCurKeys.Add ( uMatchGroupKey );
					return true;
				});
			}
			break;

		case SPH_ATTR_STRING:
		case SPH_ATTR_STRINGPTR:
			{
				SphGroupKey_t tStringKey = FetchStringKey ( tMatch, m_dAttrs[i].m_tLocator, SPH_FNV64_SEED );
				if ( tStringKey!=(SphGroupKey_t)SPH_FNV64_SEED )
					dAllKeys[i].Add ( tStringKey );
			}
			break;

		default:
			dAllKeys[i].Add ( tMatch.GetAttr ( m_dAttrs[i].m_tLocator ) );
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

template <class PRED, bool HAVE_COLUMNAR>
bool CSphGrouperMulti<PRED,HAVE_COLUMNAR>::IsMultiValue() const
{
	return m_dAttrs.any_of ( []( auto & tAttr ){ return tAttr.m_eAttrType==SPH_ATTR_JSON || tAttr.m_eAttrType==SPH_ATTR_JSON_FIELD || tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET; } );
}

template <class PRED, bool HAVE_COLUMNAR>
SphGroupKey_t CSphGrouperMulti<PRED,HAVE_COLUMNAR>::FetchStringKey ( const CSphMatch & tMatch, const CSphAttrLocator & tLocator, SphGroupKey_t tPrevKey ) const
{
	ByteBlob_t tData = tMatch.FetchAttrData ( tLocator, GetBlobPool() );
	if ( !tData.first || !tData.second )
		return tPrevKey;

	return PRED::Hash ( tData.first, tData.second, tPrevKey );
}

template <class PRED, bool HAVE_COLUMNAR>
void CSphGrouperMulti<PRED,HAVE_COLUMNAR>::SpawnColumnarGroupers()
{
	m_dSingleKeyGroupers.Resize ( m_dAttrs.GetLength() );
	m_dMultiKeyGroupers.Resize ( m_dAttrs.GetLength() );

	ARRAY_FOREACH ( i, m_dAttrs )
	{
		const auto & tAttr = m_dAttrs[i];
		if ( !tAttr.IsColumnar() && !tAttr.IsColumnarExpr() )
			continue;

		switch ( tAttr.m_eAttrType )
		{
		case SPH_ATTR_STRING:
		case SPH_ATTR_STRINGPTR:
			m_dSingleKeyGroupers[i] = CreateGrouperColumnarString ( tAttr.m_sName, m_eCollation );
			break;

		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_UINT32SET_PTR:
		case SPH_ATTR_INT64SET:
		case SPH_ATTR_INT64SET_PTR:
			m_dMultiKeyGroupers[i] = CreateGrouperColumnarMVA ( tAttr.m_sName, tAttr.m_eAttrType );
			break;

		default:
			m_dSingleKeyGroupers[i] = CreateGrouperColumnarInt ( tAttr.m_sName, tAttr.m_eAttrType );
			break;
		}
	}
}

template<typename T, bool PTR>
class GrouperMVA_T : public CSphGrouper
{
public:
					explicit GrouperMVA_T ( const CSphAttrLocator & tLocator ) : m_tLocator ( tLocator ) {}

	SphGroupKey_t	KeyFromValue ( SphAttr_t ) const override					{ assert(0); return SphGroupKey_t(); }
	SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const override	{ assert(0); return SphGroupKey_t(); }
	void			MultipleKeysFromMatch ( const CSphMatch & tMatch, CSphVector<SphGroupKey_t> & dKeys ) const override { FetchMVAKeys<T,PTR> ( dKeys, tMatch, m_tLocator, GetBlobPool() ); }
	void			GetLocator ( CSphAttrLocator & tOut ) const override { tOut = m_tLocator; }
	ESphAttr		GetResultType () const override;
	CSphGrouper *	Clone() const override { return new GrouperMVA_T ( m_tLocator ); }
	bool			IsMultiValue() const override { return true; }

private:
	CSphAttrLocator	m_tLocator;
};

template<> ESphAttr GrouperMVA_T<DWORD,true>::GetResultType() const		{ return SPH_ATTR_INTEGER; }
template<> ESphAttr GrouperMVA_T<DWORD,false>::GetResultType() const	{ return SPH_ATTR_INTEGER; }
template<> ESphAttr GrouperMVA_T<int64_t,true>::GetResultType() const	{ return SPH_ATTR_BIGINT; }
template<> ESphAttr GrouperMVA_T<int64_t,false>::GetResultType() const	{ return SPH_ATTR_BIGINT; }

/////////////////////////////////////////////////////////////////////////////
CSphGrouper * CreateGrouperDay ( const CSphAttrLocator & tLoc )
{
	return GetGroupingInUTC() ? (CSphGrouper *)new GrouperDay_T<true>(tLoc) : (CSphGrouper *)new GrouperDay_T<false>(tLoc);
}


CSphGrouper * CreateGrouperWeek ( const CSphAttrLocator & tLoc )
{
	return GetGroupingInUTC() ? (CSphGrouper *)new GrouperWeek_T<true>(tLoc) : (CSphGrouper *)new GrouperWeek_T<false>(tLoc);
}


CSphGrouper * CreateGrouperMonth ( const CSphAttrLocator & tLoc )
{
	return GetGroupingInUTC() ? (CSphGrouper *)new GrouperMonth_T<true>(tLoc) : (CSphGrouper *)new GrouperMonth_T<false>(tLoc);
}


CSphGrouper * CreateGrouperYear ( const CSphAttrLocator & tLoc )
{
	return GetGroupingInUTC() ? (CSphGrouper *)new GrouperYear_T<true>(tLoc) : (CSphGrouper *)new GrouperYear_T<false>(tLoc);
}


CSphGrouper * CreateGrouperJsonField ( const CSphAttrLocator & tLoc, ISphExpr * pExpr )
{
	return new CSphGrouperJsonField ( tLoc, pExpr );
}


CSphGrouper * CreateGrouperMVA32 ( const CSphAttrLocator & tLoc )
{
	if ( tLoc.m_bDynamic )
		return new GrouperMVA_T<DWORD,true>(tLoc);

	return new GrouperMVA_T<DWORD,false>(tLoc);
}


CSphGrouper * CreateGrouperMVA64 ( const CSphAttrLocator & tLoc )
{
	if ( tLoc.m_bDynamic )
		return new GrouperMVA_T<int64_t,true>(tLoc);

	return new GrouperMVA_T<int64_t,false>(tLoc);
}


CSphGrouper * CreateGrouperAttr ( const CSphAttrLocator & tLoc )
{
	return new GrouperAttr_c(tLoc);
}

CSphGrouper * CreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI:	return new CSphGrouperString<Utf8CIHash_fn>(tLoc);
	case SPH_COLLATION_LIBC_CI:			return new CSphGrouperString<LibcCIHash_fn>(tLoc);
	case SPH_COLLATION_LIBC_CS:			return new CSphGrouperString<LibcCSHash_fn>(tLoc);
	default:							return new CSphGrouperString<BinaryHash_fn>(tLoc);
	}
}


CSphGrouper * CreateGrouperStringExpr ( ISphExpr * pExpr, ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI:	return new GrouperStringExpr_T<Utf8CIHash_fn>(pExpr);
	case SPH_COLLATION_LIBC_CI:			return new GrouperStringExpr_T<LibcCIHash_fn>(pExpr);
	case SPH_COLLATION_LIBC_CS:			return new GrouperStringExpr_T<LibcCSHash_fn>(pExpr);
	default:							return new GrouperStringExpr_T<BinaryHash_fn>(pExpr);
	}
}


CSphGrouper * CreateGrouperMulti ( const CSphVector<CSphColumnInfo> & dAttrs, VecRefPtrs_t<ISphExpr *> dJsonKeys, ESphCollation eCollation )
{
	bool bHaveColumnar = dAttrs.any_of ( []( auto & tAttr ){ return tAttr.IsColumnar() || tAttr.IsColumnarExpr(); } );
	bool bAllColumnar = dAttrs.all_of ( []( auto & tAttr ){ return tAttr.IsColumnar() || tAttr.IsColumnarExpr(); } );

	if ( bAllColumnar )
		return CreateGrouperColumnarMulti ( dAttrs, eCollation );

	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI:
		if ( bHaveColumnar )
			return new CSphGrouperMulti<Utf8CIHash_fn,true> ( dAttrs, std::move(dJsonKeys), eCollation );
		else
			return new CSphGrouperMulti<Utf8CIHash_fn,false> ( dAttrs, std::move(dJsonKeys), eCollation );

	case SPH_COLLATION_LIBC_CI:
		if ( bHaveColumnar )
			return new CSphGrouperMulti<LibcCIHash_fn,true> ( dAttrs, std::move(dJsonKeys), eCollation );
		else
			return new CSphGrouperMulti<LibcCIHash_fn,false> ( dAttrs, std::move(dJsonKeys), eCollation );

	case SPH_COLLATION_LIBC_CS:
		if ( bHaveColumnar )
			return new CSphGrouperMulti<LibcCSHash_fn,true> ( dAttrs, std::move(dJsonKeys), eCollation );
		else
			return new CSphGrouperMulti<LibcCSHash_fn,false> ( dAttrs, std::move(dJsonKeys), eCollation );

	default:
		if ( bHaveColumnar )
			return new CSphGrouperMulti<BinaryHash_fn,true> ( dAttrs, std::move(dJsonKeys), eCollation );
		else
			return new CSphGrouperMulti<BinaryHash_fn,false> ( dAttrs, std::move(dJsonKeys), eCollation );
	}
}

/////////////////////////////////////////////////////////////////////////////
class DistinctFetcher_c : public DistinctFetcher_i
{
public:
			explicit DistinctFetcher_c ( const CSphAttrLocator & tLocator ) : m_tLocator(tLocator) {}

	void	SetColumnar ( const columnar::Columnar_i * pColumnar ) override {}
	void	SetBlobPool ( const BYTE * pBlobPool ) override { m_pBlobPool = pBlobPool; }
	void	FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override { sphFixupLocator ( m_tLocator, pOldSchema, pNewSchema ); }

protected:
	CSphAttrLocator m_tLocator;
	const BYTE * m_pBlobPool = nullptr;
};


class DistinctFetcherPlain_c : public DistinctFetcher_c
{
	using DistinctFetcher_c::DistinctFetcher_c;

public:
	void	GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override { assert ( 0 && " Requesting multiple keys from plain distinct fetcher" ); }
	bool	IsMultiValue() const override { return false; }
};


class DistinctFetcherMulti_c : public DistinctFetcher_c
{
	using DistinctFetcher_c::DistinctFetcher_c;

public:
	SphAttr_t	GetKey ( const CSphMatch & tMatch ) const override { assert ( 0 && " Requesting single keys from multi distinct fetcher" ); return 0; }
	bool		IsMultiValue() const override { return true; }
};


class DistinctFetcherInt_c : public DistinctFetcherPlain_c
{
	using DistinctFetcherPlain_c::DistinctFetcherPlain_c;

public:
	SphAttr_t			GetKey ( const CSphMatch & tMatch ) const override { return tMatch.GetAttr(m_tLocator); }
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherInt_c(m_tLocator); }
};


class DistinctFetcherString_c : public DistinctFetcherPlain_c
{
	using DistinctFetcherPlain_c::DistinctFetcherPlain_c;

public:
	SphAttr_t			GetKey ( const CSphMatch & tMatch ) const override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherString_c(m_tLocator); }
};


SphAttr_t DistinctFetcherString_c::GetKey ( const CSphMatch & tMatch ) const
{
	auto dBlob = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
	return (SphAttr_t) sphFNV64 ( dBlob );
}


class DistinctFetcherJsonField_c : public DistinctFetcherMulti_c
{
	using DistinctFetcherMulti_c::DistinctFetcherMulti_c;

public:
	void				GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherJsonField_c(m_tLocator); }
};


void DistinctFetcherJsonField_c::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	dKeys.Resize(0);
	PushJsonField ( tMatch.GetAttr(m_tLocator), m_pBlobPool, [&dKeys]( SphAttr_t * pAttr, SphGroupKey_t uGroupKey )
		{
			if ( uGroupKey )
				dKeys.Add(uGroupKey);

			return true;
		} );
}

class DistinctFetcherJsonFieldPtr_c : public DistinctFetcherMulti_c
{
	using DistinctFetcherMulti_c::DistinctFetcherMulti_c;

public:
	void				GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherJsonField_c(m_tLocator); }
};


void DistinctFetcherJsonFieldPtr_c::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	dKeys.Resize(0);
	auto pValue = (const BYTE *)tMatch.GetAttr(m_tLocator);
	if ( !pValue )
		return;

	auto tBlob = sphUnpackPtrAttr(pValue);
	pValue = tBlob.first;
	ESphJsonType eJson = (ESphJsonType)*pValue++;
	PushJsonFieldPtr ( pValue, eJson, [&dKeys]( SphGroupKey_t uGroupKey )
		{
			if ( uGroupKey )
				dKeys.Add(uGroupKey);

			return true;
		}
	);
}

template<typename T>
class DistinctFetcherMva_T : public DistinctFetcherMulti_c
{
	using DistinctFetcherMulti_c::DistinctFetcherMulti_c;

public:
	void				GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const override;
	DistinctFetcher_i *	Clone() const override { return new DistinctFetcherMva_T(m_tLocator); }
};

template<typename T>
void DistinctFetcherMva_T<T>::GetKeys ( const CSphMatch & tMatch, CSphVector<SphAttr_t> & dKeys ) const
{
	dKeys.Resize(0);
	AddGroupedMVA<T> ( [&dKeys]( SphAttr_t tAttr ){ dKeys.Add(tAttr); }, tMatch.FetchAttrData ( m_tLocator, m_pBlobPool ) );
}


DistinctFetcher_i * CreateDistinctFetcher ( const CSphString & sName, const CSphAttrLocator & tLocator, ESphAttr eType )
{
	switch ( eType )
	{
	case SPH_ATTR_STRING:
	case SPH_ATTR_STRINGPTR:		return new DistinctFetcherString_c(tLocator);
	case SPH_ATTR_JSON_FIELD:		return new DistinctFetcherJsonField_c(tLocator);
	case SPH_ATTR_JSON_FIELD_PTR:	return new DistinctFetcherJsonFieldPtr_c(tLocator);
	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_UINT32SET_PTR:	return new DistinctFetcherMva_T<DWORD>(tLocator);
	case SPH_ATTR_INT64SET:
	case SPH_ATTR_INT64SET_PTR:		return new DistinctFetcherMva_T<int64_t>(tLocator);
	default:						return new DistinctFetcherInt_c(tLocator);
	}
}
