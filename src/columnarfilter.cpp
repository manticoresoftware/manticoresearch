//
// Copyright (c) 2020-2021, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "columnarfilter.h"
#include "collation.h"
#include "attribute.h"
#include "sphinxint.h"

#if USE_COLUMNAR

class ColumnarFilter_c : public ISphFilter
{
public:
			ColumnarFilter_c ( const CSphString & sAttrName );

	void	SetColumnar ( const columnar::Columnar_i * pColumnar ) override;

protected:
	CSphString							m_sAttrName;
	int									m_iColumnarCol = -1;
	const columnar::Columnar_i *		m_pColumnar = nullptr;
	CSphScopedPtr<columnar::Iterator_i>	m_pIterator {nullptr};

	inline bool	GetValue ( RowID_t tRowID, SphAttr_t & tValue ) const;
	inline bool	GetValue ( RowID_t tRowID, ByteBlob_t & tData ) const;
};


ColumnarFilter_c::ColumnarFilter_c ( const CSphString & sAttrName )
	: m_sAttrName ( sAttrName )
{}


void ColumnarFilter_c::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	m_pColumnar = pColumnar;

	if ( !pColumnar )	// this can happen on RT columnar setup, when we have the filters but each chunk has its own columnar storage
	{
		m_pIterator.Reset();
		return;
	}

	std::string sError; // fixme! report errors
	m_pIterator = pColumnar->CreateIterator ( m_sAttrName.cstr(), columnar::IteratorHints_t(), sError );
	m_iColumnarCol = pColumnar->GetAttributeId ( m_sAttrName.cstr() );
}


bool ColumnarFilter_c::GetValue ( RowID_t tRowID, SphAttr_t & tValue ) const
{
	if ( m_pIterator.Ptr() && m_pIterator->AdvanceTo(tRowID) == tRowID )
	{
		tValue = m_pIterator->Get();
		return true;
	}

	return false;
}


bool ColumnarFilter_c::GetValue ( RowID_t tRowID, ByteBlob_t & tData ) const
{
	if ( m_pIterator.Ptr() && m_pIterator->AdvanceTo(tRowID) == tRowID )
	{
		tData.second = m_pIterator->Get ( tData.first );
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////

// direct access to columnar storage to avoid expression overhead
class Filter_SingleValueColumnar_c : public ColumnarFilter_c
{
	using ColumnarFilter_c::ColumnarFilter_c;

public:
	void	SetValues ( const SphAttr_t * pStorage, int iCount ) final;
	bool	Eval ( const CSphMatch & tMatch ) const override;
	bool	Test ( const columnar::MinMaxVec_t & dMinMax ) const final;

protected:
	SphAttr_t		m_tRefValue;
};


void Filter_SingleValueColumnar_c::SetValues ( const SphAttr_t * pStorage, int DEBUGARG(iCount) )
{
	assert ( pStorage );
	assert ( iCount==1 );
	m_tRefValue = (*pStorage);
}


bool Filter_SingleValueColumnar_c::Eval ( const CSphMatch & tMatch ) const
{
	SphAttr_t tValue;
	if ( !GetValue ( tMatch.m_tRowID, tValue ) )
		return false;

	return tValue==m_tRefValue;
}


bool Filter_SingleValueColumnar_c::Test ( const columnar::MinMaxVec_t & dMinMax ) const
{
	if ( m_iColumnarCol<0 )
		return true;

	return ( dMinMax[m_iColumnarCol].first<=m_tRefValue && m_tRefValue<=dMinMax[m_iColumnarCol].second );
}

//////////////////////////////////////////////////////////////////////////

class Filter_ValuesColumnar_c : public ColumnarFilter_c
{
	using ColumnarFilter_c::ColumnarFilter_c;

public:
	bool		Eval ( const CSphMatch & tMatch ) const final;
	bool		Test ( const columnar::MinMaxVec_t & dMinMax ) const final;
	void		SetValues ( const SphAttr_t * pValues, int iNumValues ) final;

private:
	VecTraits_T<const SphAttr_t>	m_dValues;
	bool							m_bDegenerate = false;

	bool (Filter_ValuesColumnar_c::*m_fnEval)( SphAttr_t tValue ) const = nullptr;
	bool (Filter_ValuesColumnar_c::*m_fnEvalBlock)( SphAttr_t tMin, SphAttr_t tMax ) const = nullptr;

	bool		EvalLinear ( SphAttr_t tValue ) const;
	bool		EvalBinary ( SphAttr_t tValue ) const;
	bool		EvalBlockLinear ( SphAttr_t uMin, SphAttr_t uMax ) const;
	bool		EvalBlockBinary ( SphAttr_t uMin, SphAttr_t uMax ) const;
	bool		IsDegenerate() const;
};


bool Filter_ValuesColumnar_c::Eval ( const CSphMatch & tMatch ) const
{
	if ( m_bDegenerate )
		return true;
	
	SphAttr_t tValue;
	if ( !GetValue ( tMatch.m_tRowID, tValue ) )
		return false;

	return (*this.*m_fnEval)(tValue);
}


bool Filter_ValuesColumnar_c::Test ( const columnar::MinMaxVec_t & dMinMax ) const
{
	if ( m_iColumnarCol<0 || m_bDegenerate )
		return true;

	return (*this.*m_fnEvalBlock)( dMinMax[m_iColumnarCol].first, dMinMax[m_iColumnarCol].second );
}


void Filter_ValuesColumnar_c::SetValues ( const SphAttr_t * pValues, int iNumValues )
{
	assert ( pValues );
	assert ( iNumValues > 0 );

#ifndef NDEBUG
	for ( int i = 1; i < iNumValues; i++ )
		assert ( pValues[i-1]<=pValues[i] );
#endif

	m_dValues = { pValues, iNumValues };

	const int SEARCH_THRESH=128;
	if ( iNumValues<SEARCH_THRESH )
	{
		m_fnEval = &Filter_ValuesColumnar_c::EvalLinear;
		m_fnEvalBlock = &Filter_ValuesColumnar_c::EvalBlockLinear;
	}
	else
	{
		m_fnEval = &Filter_ValuesColumnar_c::EvalBinary;
		m_fnEvalBlock = &Filter_ValuesColumnar_c::EvalBlockBinary;
	}

	m_bDegenerate = IsDegenerate();
}


bool Filter_ValuesColumnar_c::EvalLinear ( SphAttr_t tValue ) const
{
	for ( auto i : m_dValues )
		if ( i==tValue )
			return true;

	return false;
}


bool Filter_ValuesColumnar_c::EvalBinary ( SphAttr_t tValue ) const
{
	return !!m_dValues.BinarySearch(tValue);
}


bool Filter_ValuesColumnar_c::EvalBlockLinear ( SphAttr_t uMin, SphAttr_t uMax ) const
{
	for ( auto i : m_dValues )
		if ( uMin<=i && i<=uMax )
			return true;

	return false;
}


bool Filter_ValuesColumnar_c::EvalBlockBinary ( SphAttr_t uMin, SphAttr_t uMax ) const
{
	// find first value greater or equal than uMin
	const SphAttr_t * pFound = sphBinarySearchFirst ( m_dValues.Begin(), m_dValues.End()-1, SphIdentityFunctor_T<SphAttr_t>(), uMin );
	if ( *pFound<=uMax )
		return true;

	return false;
}


bool Filter_ValuesColumnar_c::IsDegenerate() const
{
	if ( !m_pColumnar )
		return false;

	columnar::Filter_t tFilter;
	tFilter.m_sName = m_sAttrName.cstr();
	tFilter.m_eType = columnar::FilterType_e::VALUES;

	int iNumValues = m_dValues.GetLength();
	tFilter.m_dValues.resize(iNumValues);
	if ( iNumValues )
		memcpy ( &tFilter.m_dValues[0], m_dValues.Begin(), iNumValues*sizeof ( m_dValues[0] ) );

	return m_pColumnar->IsFilterDegenerate(tFilter);
}

//////////////////////////////////////////////////////////////////////////

template <bool MULTI>
class Filter_StringColumnar_T : public ColumnarFilter_c
{
public:
				Filter_StringColumnar_T ( const CSphString & sAttrName, ESphCollation eCollation, bool bEquals );

	void		SetRefString ( const CSphString * pRef, int iCount ) final;
	bool		Eval ( const CSphMatch & tMatch ) const final;
	bool		Test ( const columnar::MinMaxVec_t & dMinMax ) const final;
	void		SetColumnar ( const columnar::Columnar_i * pColumnar ) final;
	bool		CanExclude() const final { return true; }

protected:
	int						m_iMinLength = 0;
	int						m_iMaxLength = 0;
	CSphVector<uint64_t>	m_dHashes;
	bool					m_bHasHashes = false;
	ESphCollation			m_eCollation = SPH_COLLATION_DEFAULT;
	StrHashCalc_fn			m_fnHashCalc = nullptr;
	bool					m_bEquals = true;

	uint64_t	GetStringHash() const;
};


template <bool MULTI>
Filter_StringColumnar_T<MULTI>::Filter_StringColumnar_T ( const CSphString & sAttrName, ESphCollation eCollation, bool bEquals )
	: ColumnarFilter_c ( sAttrName )
	, m_eCollation ( eCollation )
	, m_fnHashCalc ( GetStringHashCalcFunc(eCollation) )
	, m_bEquals ( bEquals )
{}

template <bool MULTI>
void Filter_StringColumnar_T<MULTI>::SetRefString ( const CSphString * pRef, int iCount )
{
	if_const ( !MULTI )
		assert ( iCount<=1 );

	m_iMinLength = m_iMaxLength = 0;
	for ( int i = 0; i < iCount; i++ )
	{
		const CSphString & sRef = pRef[i];
		int iLength = sRef.Length();
		m_iMinLength = Min ( iLength, m_iMinLength );
		m_iMaxLength = Max ( iLength, m_iMaxLength );
		m_dHashes.Add ( iLength ? m_fnHashCalc ( (const BYTE*)sRef.cstr(), iLength, SPH_FNV64_SEED ) : 0 );	
	}
}

template <bool MULTI>
bool Filter_StringColumnar_T<MULTI>::Eval ( const CSphMatch & tMatch ) const
{
	if ( !m_pIterator.Ptr() || m_pIterator->AdvanceTo ( tMatch.m_tRowID ) != tMatch.m_tRowID )
		return false;

	uint64_t uHash = GetStringHash();
	if_const ( !MULTI )
		return ( m_dHashes[0]==uHash ) ^ (!m_bEquals);

	for ( auto i : m_dHashes )
		if ( i==uHash )
			return true ^ (!m_bEquals);

	return false ^ (!m_bEquals);
}

template <bool MULTI>
bool Filter_StringColumnar_T<MULTI>::Test ( const columnar::MinMaxVec_t & dMinMax ) const
{
	if ( m_iColumnarCol<0 )
		return true;

	int64_t iMin = dMinMax[m_iColumnarCol].first;
	int64_t iMax = dMinMax[m_iColumnarCol].second;

	if ( m_bEquals )	
		return m_iMaxLength>=iMin && m_iMinLength<=iMax;

	// reject the case when all strings are empty and we request non-empty strings
	if ( !iMin && !iMax && !m_iMinLength && !m_iMaxLength )
		return false;

	return true;
}

template <bool MULTI>
void Filter_StringColumnar_T<MULTI>::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	if ( !pColumnar )
	{
		m_pIterator.Reset();
		return;
	}

	columnar::IteratorHints_t tHints;
	tHints.m_bNeedStringHashes = m_eCollation==SPH_COLLATION_DEFAULT;

	std::string sError; // fixme! report errors
	m_pIterator = pColumnar->CreateIterator ( m_sAttrName.cstr(), tHints, sError );
	m_bHasHashes = m_pIterator.Ptr() && m_pIterator->HaveStringHashes();
}

template <bool MULTI>
uint64_t Filter_StringColumnar_T<MULTI>::GetStringHash() const
{
	if ( m_bHasHashes )
		return m_pIterator->GetStringHash();

	const BYTE * pStr = nullptr;
	int iLen = m_pIterator->Get(pStr);
	if ( !iLen )
		return 0;

	return m_fnHashCalc ( pStr, iLen, SPH_FNV64_SEED );
}


//////////////////////////////////////////////////////////////////////////

template <typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
class Filter_RangeColumnar_T : public ColumnarFilter_c
{
	using ColumnarFilter_c::ColumnarFilter_c;

public:
	bool	Eval ( const CSphMatch & tMatch ) const final;
	bool	Test ( const columnar::MinMaxVec_t & dMinMax ) const final;
	void	SetRange ( SphAttr_t tMin, SphAttr_t tMax ) final;
	void	SetRangeFloat ( float fMin, float fMax ) final;

private:
	T m_tMinValue;
	T m_tMaxValue;
};

template <typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
bool Filter_RangeColumnar_T<T, HAS_EQUAL_MIN, HAS_EQUAL_MAX, OPEN_LEFT, OPEN_RIGHT>::Eval ( const CSphMatch & tMatch ) const
{
	SphAttr_t tValue;
	if ( !GetValue ( tMatch.m_tRowID, tValue ) )
		return false;

	return EvalRange<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT> ( ConvertType<T>(tValue), m_tMinValue, m_tMaxValue );
}

template <typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
bool Filter_RangeColumnar_T<T, HAS_EQUAL_MIN, HAS_EQUAL_MAX, OPEN_LEFT, OPEN_RIGHT>::Test ( const columnar::MinMaxVec_t & dMinMax ) const
{
	if ( m_iColumnarCol<0 )
		return true;

	T tBlockMin = ConvertType<T> ( dMinMax[m_iColumnarCol].first );
	T tBlockMax = ConvertType<T> ( dMinMax[m_iColumnarCol].second );

	return EvalBlockRangeAny<HAS_EQUAL_MIN,HAS_EQUAL_MAX> ( tBlockMin, tBlockMax, m_tMinValue, m_tMaxValue );
}

template <typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
void Filter_RangeColumnar_T<T, HAS_EQUAL_MIN, HAS_EQUAL_MAX, OPEN_LEFT, OPEN_RIGHT>::SetRange ( SphAttr_t tMin, SphAttr_t tMax )
{
	m_tMinValue = ConvertType<T>(tMin);
	m_tMaxValue = ConvertType<T>(tMax);
}

template <typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
void Filter_RangeColumnar_T<T, HAS_EQUAL_MIN, HAS_EQUAL_MAX, OPEN_LEFT, OPEN_RIGHT>::SetRangeFloat ( float fMin, float fMax )
{
	m_tMinValue = (T)fMin;
	m_tMaxValue = (T)fMax;
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
static ISphFilter * CreateColumnarRangeFilter ( const CSphString & sName, const CSphFilterSettings & tSettings )
{
	int iIndex = tSettings.m_bHasEqualMin*8 + tSettings.m_bHasEqualMax*4 + tSettings.m_bOpenLeft*2 + tSettings.m_bOpenRight;
	switch ( iIndex )
	{
	case 0:		return new Filter_RangeColumnar_T<T, false, false, false, false> (sName);
	case 1:		return new Filter_RangeColumnar_T<T, false, false, false, true>  (sName);
	case 2:		return new Filter_RangeColumnar_T<T, false, false, true,  false> (sName);
	case 3:		return new Filter_RangeColumnar_T<T, false, false, true,  true>  (sName);
	case 4:		return new Filter_RangeColumnar_T<T, false, true,  false, false> (sName);
	case 5:		return new Filter_RangeColumnar_T<T, false, true,  false, true>  (sName);
	case 6:		return new Filter_RangeColumnar_T<T, false, true,  true,  false> (sName);
	case 7:		return new Filter_RangeColumnar_T<T, false, true,  true,  true>  (sName);
	case 8:		return new Filter_RangeColumnar_T<T, true,  false, false, false> (sName);
	case 9:		return new Filter_RangeColumnar_T<T, true,  false, false, true>  (sName);
	case 10:	return new Filter_RangeColumnar_T<T, true,  false, true,  false> (sName);
	case 11:	return new Filter_RangeColumnar_T<T, true,  false, true,  true>  (sName);
	case 12:	return new Filter_RangeColumnar_T<T, true,  true,  false, false> (sName);
	case 13:	return new Filter_RangeColumnar_T<T, true,  true,  false, true>  (sName);
	case 14:	return new Filter_RangeColumnar_T<T, true,  true,  true,  false> (sName);
	case 15:	return new Filter_RangeColumnar_T<T, true,  true,  true,  true>  (sName);
	default:	return nullptr;
	}
}

template < typename T, typename FUNC >
class Filter_SingleValueColumnar_MVA_T : public Filter_SingleValueColumnar_c
{
	using Filter_SingleValueColumnar_c::Filter_SingleValueColumnar_c;

public:
	bool Eval ( const CSphMatch & tMatch ) const final
	{
		ByteBlob_t tData;
		if ( !GetValue ( tMatch.m_tRowID, tData ) )
			return false;

		VecTraits_T<const T> tCheck ( (const T*)tData.first, tData.second/sizeof(T) );
		return FUNC::Eval ( tCheck, m_tRefValue );
	}
};

//////////////////////////////////////////////////////////////////////////

template < typename T, typename FUNC >
class Filter_ValuesColumnar_MVA_T : public ColumnarFilter_c
{
	using ColumnarFilter_c::ColumnarFilter_c;

public:
	bool		Eval ( const CSphMatch & tMatch ) const final;
	bool		Test ( const columnar::MinMaxVec_t & dMinMax ) const final;
	void		SetValues ( const SphAttr_t * pValues, int iNumValues ) final;

private:
	VecTraits_T<const SphAttr_t>	m_dValues;
};

template < typename T, typename FUNC >
bool Filter_ValuesColumnar_MVA_T<T,FUNC>::Eval ( const CSphMatch & tMatch ) const
{
	ByteBlob_t tData;
	if ( !GetValue ( tMatch.m_tRowID, tData ) )
		return false;

	VecTraits_T<const T> tCheck ( (const T*)tData.first, tData.second/sizeof(T) );
	return FUNC::Eval ( tCheck, m_dValues );
}

template < typename T, typename FUNC >
bool Filter_ValuesColumnar_MVA_T<T,FUNC>::Test ( const columnar::MinMaxVec_t & dMinMax ) const
{
	if ( m_iColumnarCol<0 )
		return true;

	return FUNC::EvalBlock ( m_dValues, dMinMax[m_iColumnarCol].first, dMinMax[m_iColumnarCol].second );
}

template < typename T, typename FUNC >
void Filter_ValuesColumnar_MVA_T<T,FUNC>::SetValues ( const SphAttr_t * pValues, int iNumValues )
{
	assert ( pValues );
	assert ( iNumValues > 0 );

#ifndef NDEBUG
	for ( int i = 1; i < iNumValues; i++ )
		assert ( pValues[i-1]<=pValues[i] );
#endif

	m_dValues = { pValues, iNumValues };
}

//////////////////////////////////////////////////////////////////////////

template <typename T, typename FUNC, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
class Filter_RangeColumnar_MVA_T : public ColumnarFilter_c
{
	using ColumnarFilter_c::ColumnarFilter_c;

public:
	bool	Eval ( const CSphMatch & tMatch ) const final;
	bool	Test ( const columnar::MinMaxVec_t & dMinMax ) const final;
	void	SetRange ( SphAttr_t tMin, SphAttr_t tMax ) final;

private:
	SphAttr_t m_tMinValue;
	SphAttr_t m_tMaxValue;
};

template <typename T, typename FUNC, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
bool Filter_RangeColumnar_MVA_T<T, FUNC, HAS_EQUAL_MIN, HAS_EQUAL_MAX, OPEN_LEFT, OPEN_RIGHT>::Eval ( const CSphMatch & tMatch ) const
{
	ByteBlob_t tData;
	if ( !GetValue ( tMatch.m_tRowID, tData ) )
		return false;

	VecTraits_T<const T> tCheck ( (const T*)tData.first, tData.second/sizeof(T) );
	return FUNC::template EvalRange<T,HAS_EQUAL_MIN,HAS_EQUAL_MAX> ( tCheck, m_tMinValue, m_tMaxValue );
}

template <typename T, typename FUNC, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
bool Filter_RangeColumnar_MVA_T<T, FUNC, HAS_EQUAL_MIN, HAS_EQUAL_MAX, OPEN_LEFT, OPEN_RIGHT>::Test ( const columnar::MinMaxVec_t & dMinMax ) const
{
	if ( m_iColumnarCol<0 )
		return true;

	return EvalBlockRangeAny<HAS_EQUAL_MIN,HAS_EQUAL_MAX> ( dMinMax[m_iColumnarCol].first, dMinMax[m_iColumnarCol].second, m_tMinValue, m_tMaxValue );
}

template <typename T, typename FUNC, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
void Filter_RangeColumnar_MVA_T<T, FUNC, HAS_EQUAL_MIN, HAS_EQUAL_MAX, OPEN_LEFT, OPEN_RIGHT>::SetRange ( SphAttr_t tMin, SphAttr_t tMax )
{
	m_tMinValue = tMin;
	m_tMaxValue = tMax;
}

//////////////////////////////////////////////////////////////////////////

class Filter_NullColumnar_c : public ISphFilter
{
public:
	bool		Eval ( const CSphMatch & tMatch ) const final { return true; }
};

//////////////////////////////////////////////////////////////////////////

template < typename T, typename FUNC>
static ISphFilter * CreateColumnarMvaFilterValues ( const CSphString & sName, const CSphFilterSettings & tSettings )
{
	if ( tSettings.GetNumValues()==1 )
		return new Filter_SingleValueColumnar_MVA_T<T,FUNC>(sName);

	return new Filter_ValuesColumnar_MVA_T<T,FUNC>(sName);
}


template < typename T, typename FUNC>
static ISphFilter * CreateColumnarMvaRangeFilter ( const CSphString & sName, const CSphFilterSettings & tSettings )
{
	int iIndex = tSettings.m_bHasEqualMin*8 + tSettings.m_bHasEqualMax*4 + tSettings.m_bOpenLeft*2 + tSettings.m_bOpenRight;
	switch ( iIndex )
	{
	case 0:		return new Filter_RangeColumnar_MVA_T<T, FUNC, false, false, false, false> (sName);
	case 1:		return new Filter_RangeColumnar_MVA_T<T, FUNC, false, false, false, true>  (sName);
	case 2:		return new Filter_RangeColumnar_MVA_T<T, FUNC, false, false, true,  false> (sName);
	case 3:		return new Filter_RangeColumnar_MVA_T<T, FUNC, false, false, true,  true>  (sName);
	case 4:		return new Filter_RangeColumnar_MVA_T<T, FUNC, false, true,  false, false> (sName);
	case 5:		return new Filter_RangeColumnar_MVA_T<T, FUNC, false, true,  false, true>  (sName);
	case 6:		return new Filter_RangeColumnar_MVA_T<T, FUNC, false, true,  true,  false> (sName);
	case 7:		return new Filter_RangeColumnar_MVA_T<T, FUNC, false, true,  true,  true>  (sName);
	case 8:		return new Filter_RangeColumnar_MVA_T<T, FUNC, true,  false, false, false> (sName);
	case 9:		return new Filter_RangeColumnar_MVA_T<T, FUNC, true,  false, false, true>  (sName);
	case 10:	return new Filter_RangeColumnar_MVA_T<T, FUNC, true,  false, true,  false> (sName);
	case 11:	return new Filter_RangeColumnar_MVA_T<T, FUNC, true,  false, true,  true>  (sName);
	case 12:	return new Filter_RangeColumnar_MVA_T<T, FUNC, true,  true,  false, false> (sName);
	case 13:	return new Filter_RangeColumnar_MVA_T<T, FUNC, true,  true,  false, true>  (sName);
	case 14:	return new Filter_RangeColumnar_MVA_T<T, FUNC, true,  true,  true,  false> (sName);
	case 15:	return new Filter_RangeColumnar_MVA_T<T, FUNC, true,  true,  true,  true>  (sName);
	default:	return nullptr;
	}
}


static CSphString GetAttributeName ( int iAttr, const ISphSchema & tSchema )
{
	const CSphColumnInfo & tAttr = tSchema.GetAttr(iAttr);

	// if it is a columnar expression working over an aliased attribute, fetch that attribute name
	if ( tAttr.IsColumnarExpr() )
	{
		CSphString sAliasedCol;
		tAttr.m_pExpr->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sAliasedCol );
		return sAliasedCol;
	}

	return tAttr.m_sName;
}


static ISphFilter * CreateColumnarFilterMVA ( int iAttr, const ISphSchema & tSchema, const CSphFilterSettings & tSettings, const CommonFilterSettings_t & tFixedSettings )
{
	const CSphColumnInfo & tAttr = tSchema.GetAttr(iAttr);
	CSphString sAttrName = GetAttributeName ( iAttr, tSchema );

	bool bWide = tAttr.m_eAttrType==SPH_ATTR_INT64SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET_PTR;
	bool bRange = tFixedSettings.m_eType==SPH_FILTER_RANGE;
	bool bAll = tSettings.m_eMvaFunc==SPH_MVAFUNC_ALL;
	int iIndex = bWide*4 + bRange*2 + bAll;

	ISphFilter * pFilter = nullptr;

	switch ( iIndex )
	{
	case 0:	pFilter = CreateColumnarMvaFilterValues<uint32_t,MvaEvalAny_c> ( sAttrName, tSettings ); break;
	case 1: pFilter = CreateColumnarMvaFilterValues<uint32_t,MvaEvalAll_c> ( sAttrName, tSettings ); break;

	case 2:	pFilter = CreateColumnarMvaRangeFilter<uint32_t,MvaEvalAny_c> ( sAttrName, tSettings ); break;
	case 3:	pFilter = CreateColumnarMvaRangeFilter<uint32_t,MvaEvalAll_c> ( sAttrName, tSettings ); break;

	case 4:	pFilter = CreateColumnarMvaFilterValues<int64_t,MvaEvalAny_c> ( sAttrName, tSettings ); break;
	case 5:	pFilter = CreateColumnarMvaFilterValues<int64_t,MvaEvalAll_c> ( sAttrName, tSettings ); break;

	case 6:	pFilter = CreateColumnarMvaRangeFilter<int64_t,MvaEvalAny_c> ( sAttrName, tSettings ); break;
	case 7:	pFilter = CreateColumnarMvaRangeFilter<int64_t,MvaEvalAll_c> ( sAttrName, tSettings ); break;

	default:
		assert ( 0 && "Unsupported MVA filter type" );
	}

	return pFilter;
}


static ISphFilter * CreateColumnarFilterPlain ( int iAttr, const ISphSchema & tSchema, const CSphFilterSettings & tSettings, const CommonFilterSettings_t & tFixedSettings, ESphCollation eCollation )
{
	ISphFilter * pFilter = nullptr;
	CSphString sAttrName = GetAttributeName ( iAttr, tSchema );

	switch ( tFixedSettings.m_eType )
	{
	case SPH_FILTER_VALUES:
	{
		if ( tSettings.GetNumValues()==1 )
			pFilter = new Filter_SingleValueColumnar_c(sAttrName);
		else
			pFilter = new Filter_ValuesColumnar_c(sAttrName);
	}
	break;

	case SPH_FILTER_RANGE:		pFilter = CreateColumnarRangeFilter<SphAttr_t> ( sAttrName, tSettings ); break;
	case SPH_FILTER_FLOATRANGE:	pFilter = CreateColumnarRangeFilter<float> ( sAttrName, tSettings ); break;
	case SPH_FILTER_STRING:		pFilter = new Filter_StringColumnar_T<false> ( sAttrName, eCollation, !tSettings.m_bExclude ); break;
	case SPH_FILTER_STRING_LIST:pFilter = new Filter_StringColumnar_T<true> ( sAttrName, eCollation, !tSettings.m_bExclude ); break;

	default:
		assert ( 0 && "Unhandled columnar filter type" );
		break;
	}

	return pFilter;
}


ISphFilter * TryToCreateColumnarFilter ( int iAttr, const ISphSchema & tSchema, const CSphFilterSettings & tSettings, const CommonFilterSettings_t & tFixedSettings, ESphCollation eCollation,
	CSphString & sError, CSphString & sWarning )
{
	if ( iAttr<0 )
		return nullptr;

	const CSphColumnInfo & tAttr = tSchema.GetAttr(iAttr);
	if ( !tAttr.IsColumnar() && !tAttr.IsColumnarExpr() )
		return nullptr;

	// when we created a columnar expression, we removed it from PREFILTER stage
	// that means that we have to create a specialized filter here because a generic expression filter will no longer work

	bool bFound = false;
	static const ESphAttr dSupportedTypes[] = { SPH_ATTR_INTEGER, SPH_ATTR_BIGINT, SPH_ATTR_TIMESTAMP, SPH_ATTR_BOOL, SPH_ATTR_FLOAT, SPH_ATTR_STRING, SPH_ATTR_STRINGPTR,
		SPH_ATTR_UINT32SET, SPH_ATTR_UINT32SET_PTR, SPH_ATTR_INT64SET, SPH_ATTR_INT64SET_PTR };

	for ( auto i : dSupportedTypes )
		bFound |= tAttr.m_eAttrType==i;

	if ( !bFound )
	{
		assert ( 0 && "Unhandled columnar filter type" );
		return nullptr;
	}

	if ( tFixedSettings.m_eType==SPH_FILTER_NULL )
		return new Filter_NullColumnar_c;

	if ( IsMvaAttr(tAttr.m_eAttrType) )
	{
		if ( tFixedSettings.m_eType!=SPH_FILTER_VALUES && tFixedSettings.m_eType!=SPH_FILTER_RANGE )
		{
			sError.SetSprintf ( "unsupported filter type '%s' on MVA column", FilterType2Str ( tFixedSettings.m_eType ).cstr() );
			return nullptr;
		}

		if ( tSettings.m_eMvaFunc==SPH_MVAFUNC_NONE )
			sWarning.SetSprintf ( "use an explicit ANY()/ALL() around a filter on MVA column" );

		return CreateColumnarFilterMVA ( iAttr, tSchema, tSettings, tFixedSettings );
	}

	return CreateColumnarFilterPlain ( iAttr, tSchema, tSettings, tFixedSettings, eCollation );
}


columnar::FilterType_e ToColumnarFilterType ( ESphFilter eType )
{
	switch ( eType )
	{
	case SPH_FILTER_VALUES:		return columnar::FilterType_e::VALUES;
	case SPH_FILTER_RANGE:		return columnar::FilterType_e::RANGE;
	case SPH_FILTER_FLOATRANGE:	return columnar::FilterType_e::FLOATRANGE;
	case SPH_FILTER_STRING:
	case SPH_FILTER_STRING_LIST:
		return columnar::FilterType_e::STRINGS;

	default:					return columnar::FilterType_e::NONE;
	}
}


columnar::MvaAggr_e ToColumnarAggr ( ESphMvaFunc eAggr )
{
	switch ( eAggr )
	{
	case SPH_MVAFUNC_ANY:	return columnar::MvaAggr_e::ANY;
	case SPH_MVAFUNC_ALL:	return columnar::MvaAggr_e::ALL;
	default:				return columnar::MvaAggr_e::NONE;
	}
}


bool AddColumnarFilter ( std::vector<columnar::Filter_t> & dDst, const CSphFilterSettings & tSrc, ESphCollation eCollation, const ISphSchema & tSchema, CSphString & sWarning )
{
	columnar::FilterType_e eColumnarFilterType = ToColumnarFilterType ( tSrc.m_eType );
	if ( eColumnarFilterType==columnar::FilterType_e::NONE )
		return false;

	dDst.emplace_back();
	auto & tDst = dDst.back();

	tDst.m_sName			= tSrc.m_sAttrName.cstr();
	tDst.m_bExclude			= tSrc.m_bExclude;
	tDst.m_eType			= eColumnarFilterType;
	tDst.m_eMvaAggr			= ToColumnarAggr ( tSrc.m_eMvaFunc );
	tDst.m_iMinValue		= tSrc.m_iMinValue;
	tDst.m_iMaxValue		= tSrc.m_iMaxValue;
	tDst.m_fMinValue		= tSrc.m_fMinValue;
	tDst.m_fMaxValue		= tSrc.m_fMaxValue;
	tDst.m_bLeftUnbounded	= tSrc.m_bOpenLeft;
	tDst.m_bRightUnbounded	= tSrc.m_bOpenRight;
	tDst.m_bLeftClosed		= tSrc.m_bHasEqualMin;
	tDst.m_bRightClosed		= tSrc.m_bHasEqualMax;

	int iNumValues = tSrc.GetNumValues();
	tDst.m_dValues.resize(iNumValues);
	if ( iNumValues )
		memcpy ( &tDst.m_dValues[0], tSrc.GetValueArray(), iNumValues*sizeof ( tDst.m_dValues[0] ) );

	int iNumStrValues = tSrc.m_dStrings.GetLength();
	tDst.m_dStringValues.resize(iNumStrValues);
	for ( int i = 0; i < iNumStrValues; i++ )
	{
		auto & dDstStr = tDst.m_dStringValues[i];
		dDstStr.resize ( tSrc.m_dStrings[i].Length() );
		memcpy ( dDstStr.data(), tSrc.m_dStrings[i].cstr(), dDstStr.size() );
	}

	const CSphColumnInfo * pAttr = tSchema.GetAttr ( tSrc.m_sAttrName.cstr() );
	if ( pAttr && IsMvaAttr ( pAttr->m_eAttrType ) && ( tDst.m_eMvaAggr==columnar::MvaAggr_e::NONE ) )
	{
		sWarning = "use an explicit ANY()/ALL() around a filter on MVA column";
		tDst.m_eMvaAggr = columnar::MvaAggr_e::ANY;
	}

	// FIXME! add support for arbitrary collations in columnar storage
	tDst.m_fnCalcStrHash = eCollation==SPH_COLLATION_DEFAULT ? LibcCIHash_fn::Hash : nullptr;
	tDst.m_fnStrCmp = GetStringCmpFunc(eCollation);

	return true;
}


#endif // USE_COLUMNAR