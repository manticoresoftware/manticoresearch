//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxfilter.h"
#include "columnarfilter.h"
#include "sphinxint.h"
#include "sphinxjson.h"
#include "attribute.h"
#include "histogram.h"
#include "coroutine.h"
#include "stackmock.h"

#include <boost/icl/interval.hpp>

#if _WIN32
#pragma warning(disable:4250) // inheritance via dominance is our intent
#endif

/// attribute-based
struct IFilter_Attr: virtual ISphFilter
{
	CSphAttrLocator m_tLocator;

	void SetLocator ( const CSphAttrLocator & tLocator ) override
	{
		m_tLocator = tLocator;
	}
};

/// values
struct IFilter_Values : virtual ISphFilter
{
	const SphAttr_t *	m_pValues = nullptr;
	int					m_iValueCount = 0;

	void SetValues ( const SphAttr_t * pStorage, int iCount ) final
	{
		assert ( pStorage );
		assert ( iCount > 0 );
		#ifndef NDEBUG // values must be sorted
		for ( int i = 1; i < iCount; i++ )
			assert ( pStorage[i-1]<=pStorage[i] );
		#endif

		m_pValues = pStorage;
		m_iValueCount = iCount;
	}

	inline SphAttr_t GetValue ( int iIndex ) const
	{
		assert ( iIndex>=0 && iIndex<m_iValueCount );
		return m_pValues[iIndex];
	}

	inline bool EvalValues ( SphAttr_t uValue ) const;
	inline bool EvalBlockValues ( SphAttr_t uBlockMin, SphAttr_t uBlockMax ) const;
};


bool IFilter_Values::EvalValues ( SphAttr_t uValue ) const
{
	if ( !m_pValues )
		return true;

	const SphAttr_t * pA = m_pValues;
	const SphAttr_t * pB = m_pValues + m_iValueCount - 1;

	if ( uValue==*pA || uValue==*pB ) return true;
	if ( uValue<(*pA) || uValue>(*pB) ) return false;

	while ( pB-pA>1 )
	{
		const SphAttr_t * pM = pA + ((pB-pA)/2);
		if ( uValue==(*pM) )
			return true;
		if ( uValue<(*pM) )
			pB = pM;
		else
			pA = pM;
	}
	return false;
}


// OPTIMIZE: use binary search
bool IFilter_Values::EvalBlockValues ( SphAttr_t uBlockMin, SphAttr_t uBlockMax ) const
{
	// is any of our values inside the block?
	for ( int i = 0; i < m_iValueCount; ++i )
		if ( GetValue(i)>=uBlockMin && GetValue(i)<=uBlockMax )
			return true;
	return false;
}


/// range
struct IFilter_Range: virtual ISphFilter
{
	SphAttr_t m_iMinValue;
	SphAttr_t m_iMaxValue;

	void SetRange ( SphAttr_t tMin, SphAttr_t tMax ) override
	{
		m_iMinValue = tMin;
		m_iMaxValue = tMax;
	}
};


/// filters

// attr

class Filter_Values : public IFilter_Attr, public IFilter_Values
{
public:
	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return EvalValues ( tMatch.GetAttr ( m_tLocator ) );
	}

	bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const final
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		SphAttr_t uBlockMin = sphGetRowAttr ( pMinDocinfo, m_tLocator );
		SphAttr_t uBlockMax = sphGetRowAttr ( pMaxDocinfo, m_tLocator );

		return EvalBlockValues ( uBlockMin, uBlockMax );
	}
};


class Filter_SingleValue : public IFilter_Attr
{
public:
#ifndef NDEBUG
	void SetValues ( const SphAttr_t * pStorage, int iCount ) final
#else
	void SetValues ( const SphAttr_t * pStorage, int ) final
#endif
	{
		assert ( pStorage );
		assert ( iCount==1 );
		m_RefValue = (*pStorage);
	}

	bool Eval ( const CSphMatch & tMatch ) const override
	{
		return tMatch.GetAttr ( m_tLocator )==m_RefValue;
	}

	bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const final
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		SphAttr_t uBlockMin = sphGetRowAttr ( pMinDocinfo, m_tLocator );
		SphAttr_t uBlockMax = sphGetRowAttr ( pMaxDocinfo, m_tLocator );
		return ( uBlockMin<=m_RefValue && m_RefValue<=uBlockMax );
	}

protected:
	SphAttr_t m_RefValue;
};


class Filter_SingleValueStatic32 : public Filter_SingleValue
{
public:
	Filter_SingleValueStatic32() = default;

	void SetLocator ( const CSphAttrLocator & tLoc ) final
	{
		assert ( tLoc.m_iBitCount==32 );
		assert ( ( tLoc.m_iBitOffset % 32 )==0 );
		assert ( !tLoc.m_bDynamic );
		m_tLocator = tLoc;
		m_iIndex = tLoc.m_iBitOffset / 32;
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return tMatch.m_pStatic [ m_iIndex ]==m_RefValue;
	}

private:
	int m_iIndex;
};


template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT >
struct Filter_Range : public IFilter_Attr, public IFilter_Range
{
	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return EvalRange<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT> ( tMatch.GetAttr ( m_tLocator ), m_iMinValue, m_iMaxValue );
	}

	bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const final
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		auto uBlockMin = sphGetRowAttr ( pMinDocinfo, m_tLocator );
		auto uBlockMax = sphGetRowAttr ( pMaxDocinfo, m_tLocator );

		// not-reject
		return EvalBlockRangeAny<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT> ( uBlockMin, uBlockMax, m_iMinValue, m_iMaxValue );
	}
};

// float

template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
struct Filter_FloatRange : public IFilter_Attr
{
	float m_fMinValue;
	float m_fMaxValue;

	void SetRangeFloat ( float fMin, float fMax ) final
	{
		m_fMinValue = fMin;
		m_fMaxValue = fMax;
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return EvalRange<HAS_EQUAL_MIN,HAS_EQUAL_MAX> ( tMatch.GetAttrFloat ( m_tLocator ), m_fMinValue, m_fMaxValue );
	}

	bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const final
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		float fBlockMin = sphDW2F ( (DWORD)sphGetRowAttr ( pMinDocinfo, m_tLocator ) );
		float fBlockMax = sphDW2F ( (DWORD)sphGetRowAttr ( pMaxDocinfo, m_tLocator ) );

		// not-reject
		return EvalBlockRangeAny<HAS_EQUAL_MIN,HAS_EQUAL_MAX> ( fBlockMin, fBlockMax, m_fMinValue, m_fMaxValue );
	}
};

struct Filter_WeightValues: public IFilter_Values
{
	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return EvalValues ( tMatch.m_iWeight );
	}
};

template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
struct Filter_WeightRange: public IFilter_Range
{
	virtual bool IsEarly () { return false; }
	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return EvalRange<HAS_EQUAL_MIN, HAS_EQUAL_MAX> ( (SphAttr_t)tMatch.m_iWeight, m_iMinValue, m_iMaxValue );
	}
};

//////////////////////////////////////////////////////////////////////////
// MVA
//////////////////////////////////////////////////////////////////////////

class Filter_MVA_c: public virtual IFilter_Attr
{
public:
	void SetBlobStorage ( const BYTE * pBlobPool ) override
	{
		m_pBlobPool = pBlobPool;
	}

protected:
	const BYTE * m_pBlobPool {nullptr};
};


template < typename T >
class Filter_MVAValues_Any_c : public Filter_MVA_c, public IFilter_Values
{
public:
	bool Eval ( const CSphMatch & tMatch ) const final
	{
		auto dMva = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		return MvaEval_Any<T> ( dMva, {m_pValues, m_iValueCount} );
	}
};


template < typename T >
class Filter_MVAValues_All_c : public Filter_MVA_c, IFilter_Values
{
public:
	bool Eval ( const CSphMatch & tMatch ) const final
	{
		auto dMva = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		return MvaEval_All<T> ( dMva, {m_pValues, m_iValueCount} );
	}
};


template < typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
class Filter_MVARange_Any_c : public Filter_MVA_c, IFilter_Range
{
public:
	bool Eval ( const CSphMatch & tMatch ) const final
	{
		auto dMva = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		return MvaEval_RangeAny<T, HAS_EQUAL_MIN, HAS_EQUAL_MAX> ( (const T*)dMva.first, dMva.second/sizeof(T),
			m_iMinValue, m_iMaxValue );
	}
};


template < typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
class Filter_MVARange_All_c : public Filter_MVA_c, IFilter_Range
{
public:
	bool Eval ( const CSphMatch & tMatch ) const final
	{
		auto dMva = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		return MvaEval_RangeAll<T, HAS_EQUAL_MIN, HAS_EQUAL_MAX> ( (const T*)dMva.first, dMva.second/sizeof(T),
			m_iMinValue, m_iMaxValue );
	}
};


class IFilter_Str : public IFilter_Attr
{

protected:
	const BYTE * m_pBlobPool = nullptr;

public:
	void SetBlobStorage ( const BYTE * pBlobPool ) final
	{
		m_pBlobPool = pBlobPool;
	}
};


class FilterString_c : public IFilter_Str
{
public:
	FilterString_c ( ESphCollation eCollation, bool bEq )
		: m_fnStrCmp ( GetStringCmpFunc ( eCollation ) )
		, m_dVal ( 0 )
		, m_bEq ( bEq )
	{}

	void SetRefString ( const CSphString * pRef, int iCount ) override
	{
		assert ( iCount<2 );
		const char * sVal = pRef ? pRef->cstr() : nullptr;
		int iLen = pRef ? pRef->Length() : 0;
		m_dVal.Reset ( iLen );
		memcpy ( m_dVal.Begin(), sVal, iLen );
	}

	bool Eval ( const CSphMatch & tMatch ) const override
	{
		auto dStr = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		bool bEq = m_fnStrCmp ( dStr, m_dVal, false )==0;
		return ( m_bEq==bEq );
	}

protected:
	SphStringCmp_fn			m_fnStrCmp;

private:
	CSphFixedVector<BYTE>	m_dVal;
	bool					m_bEq;
};


class Filter_StringValues_c : public FilterString_c
{
public:
	Filter_StringValues_c ( ESphCollation eCollation )
		: FilterString_c ( eCollation, false )
	{}

	void SetRefString ( const CSphString * pRef, int iCount ) final
	{
		assert ( pRef );
		assert ( iCount>0 );

		m_dValues.Resize ( iCount );
		for ( int i=0; i<iCount; ++i )
		{
			const char * sRef = ( pRef + i )->cstr();
			int iLen = ( pRef + i )->Length();

			m_dValues[i].Resize ( iLen );
			memcpy ( m_dValues[i].Begin(), sRef, iLen );
		}
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		auto dStr = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );
		return m_dValues.any_of( [this, &dStr] ( const VecTraits_T<BYTE>& i )
		{
			return m_fnStrCmp ( dStr, i, false )==0;
		});
	}

private:
	CSphTightVector<CSphTightVector<BYTE>> m_dValues;
};

struct Filter_StringTags_c : IFilter_Str
{
protected:
	CSphVector<uint64_t> m_dTags;
	mutable CSphVector<uint64_t> m_dMatchTags;

public:
	void SetRefString ( const CSphString * pRef, int iCount ) final
	{
		assert ( pRef );
		assert ( iCount>0 );

		m_dTags.Resize ( iCount );

		for ( int i = 0; i<iCount; ++i )
			m_dTags[i] = sphFNV64 ( pRef[i].cstr () );

		m_dTags.Uniq ();
	}

protected:
	inline void GetMatchTags ( const CSphMatch &tMatch ) const
	{
		auto dStr = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool );

		m_dMatchTags.Resize ( 0 );
		sphSplitApply ( ( const char * ) dStr.first, dStr.second, [this] ( const char * pTag, int iLen ) {
			m_dMatchTags.Add ( sphFNV64 ( pTag, iLen, SPH_FNV64_SEED ) );
		} );
		m_dMatchTags.Uniq ();
	}
};

struct Filter_StringTagsAny_c : Filter_StringTags_c
{
public:
	bool Eval ( const CSphMatch &tMatch ) const final
	{
		GetMatchTags ( tMatch );

		auto pFilter = m_dMatchTags.begin ();
		auto pQueryTags = m_dTags.begin ();
		auto pFilterEnd = m_dMatchTags.end ();
		auto pTagsEnd = m_dTags.end ();

		while ( pFilter<pFilterEnd && pQueryTags<pTagsEnd )
		{
			if ( *pQueryTags<*pFilter )
				++pQueryTags;
			else if ( *pFilter<*pQueryTags )
				++pFilter;
			else if ( *pQueryTags==*pFilter )
				return true;
		}
		return false;
	}
};

struct Filter_StringTagsAll_c : Filter_StringTags_c
{
public:
	bool Eval ( const CSphMatch &tMatch ) const final
	{
		GetMatchTags ( tMatch );

		auto pFilter = m_dMatchTags.begin ();
		auto pQueryTags = m_dTags.begin ();
		auto pFilterEnd = m_dMatchTags.end ();
		auto pTagsEnd = m_dTags.end ();
		int iExpectedTags = m_dTags.GetLength();

		while ( pFilter<pFilterEnd && pQueryTags<pTagsEnd && iExpectedTags>0 )
		{
			if ( *pQueryTags<*pFilter )
				++pQueryTags;
			else if ( *pFilter<*pQueryTags )
				++pFilter;
			else if ( *pQueryTags==*pFilter )
			{
				--iExpectedTags;
				++pQueryTags;
				++pFilter;
			}
		}
		return !iExpectedTags;
	}
};


struct Filter_And2 final : public ISphFilter
{
	ISphFilter * m_pArg1;
	ISphFilter * m_pArg2;

	explicit Filter_And2 ( ISphFilter * pArg1, ISphFilter * pArg2 )
		: m_pArg1 ( pArg1 )
		, m_pArg2 ( pArg2 )
	{}

	~Filter_And2 () final
	{
		SafeDelete ( m_pArg1 );
		SafeDelete ( m_pArg2 );
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return m_pArg1->Eval ( tMatch ) && m_pArg2->Eval ( tMatch );
	}

	bool EvalBlock ( const DWORD * pMin, const DWORD * pMax ) const final
	{
		return m_pArg1->EvalBlock ( pMin, pMax ) && m_pArg2->EvalBlock ( pMin, pMax );
	}

	bool Test ( const columnar::MinMaxVec_t & dMinMax ) const final
	{
		return m_pArg1->Test(dMinMax) && m_pArg2->Test(dMinMax);
	}

	void SetColumnar ( const columnar::Columnar_i * pColumnar ) final
	{
		m_pArg1->SetColumnar(pColumnar);
		m_pArg2->SetColumnar(pColumnar);
	}

	ISphFilter * Join ( ISphFilter * pFilter ) final
	{
		m_pArg2 = new Filter_And2 ( m_pArg2, pFilter );
		return this;
	}

	void SetBlobStorage ( const BYTE * pBlobPool ) final
	{
		m_pArg1->SetBlobStorage ( pBlobPool );
		m_pArg2->SetBlobStorage ( pBlobPool );
	}
};


struct Filter_And3 final : public ISphFilter
{
	ISphFilter * m_pArg1;
	ISphFilter * m_pArg2;
	ISphFilter * m_pArg3;

	explicit Filter_And3 ( ISphFilter * pArg1, ISphFilter * pArg2, ISphFilter * pArg3 )
		: m_pArg1 ( pArg1 )
		, m_pArg2 ( pArg2 )
		, m_pArg3 ( pArg3 )
	{}

	~Filter_And3 () final
	{
		SafeDelete ( m_pArg1 );
		SafeDelete ( m_pArg2 );
		SafeDelete ( m_pArg3 );
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return m_pArg1->Eval ( tMatch ) && m_pArg2->Eval ( tMatch ) && m_pArg3->Eval ( tMatch );
	}

	bool EvalBlock ( const DWORD * pMin, const DWORD * pMax ) const final
	{
		return m_pArg1->EvalBlock ( pMin, pMax ) && m_pArg2->EvalBlock ( pMin, pMax ) && m_pArg3->EvalBlock ( pMin, pMax );
	}

	bool Test ( const columnar::MinMaxVec_t & dMinMax ) const final
	{
		return m_pArg1->Test(dMinMax) && m_pArg2->Test(dMinMax) && m_pArg3->Test(dMinMax);
	}

	void SetColumnar ( const columnar::Columnar_i * pColumnar ) final
	{
		m_pArg1->SetColumnar(pColumnar);
		m_pArg2->SetColumnar(pColumnar);
		m_pArg3->SetColumnar(pColumnar);
	}

	ISphFilter * Join ( ISphFilter * pFilter ) final
	{
		m_pArg3 = new Filter_And2 ( m_pArg3, pFilter );
		return this;
	}

	void SetBlobStorage ( const BYTE * pBlobPool ) final
	{
		m_pArg1->SetBlobStorage ( pBlobPool );
		m_pArg2->SetBlobStorage ( pBlobPool );
		m_pArg3->SetBlobStorage ( pBlobPool );
	}
};


struct Filter_And final : public ISphFilter
{
	CSphVector<ISphFilter *> m_dFilters;

	~Filter_And () final
	{
		for ( auto &pFilter : m_dFilters )
			SafeDelete ( pFilter );
	}

	void Add ( ISphFilter * pFilter )
	{
		m_dFilters.Add ( pFilter );
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		for ( auto pFilter: m_dFilters )
			if ( !pFilter->Eval ( tMatch ) )
				return false;
		return true;
	}

	bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const final
	{
		for ( auto pFilter : m_dFilters )
			if ( !pFilter->EvalBlock ( pMinDocinfo, pMaxDocinfo ) )
				return false;
		return true;
	}

	bool Test ( const columnar::MinMaxVec_t & dMinMax ) const final		{ return m_dFilters.all_of ( [&dMinMax]( ISphFilter * pFilter ){ return pFilter->Test(dMinMax); } ); }
	void SetColumnar ( const columnar::Columnar_i * pColumnar ) final	{ m_dFilters.for_each ( [pColumnar]( ISphFilter * pFilter ){ pFilter->SetColumnar(pColumnar); } ); }

	ISphFilter * Join ( ISphFilter * pFilter ) final
	{
		Add ( pFilter );
		return this;
	}

	void SetBlobStorage ( const BYTE * pBlobPool ) final
	{
		for ( auto &pFilter : m_dFilters )
			pFilter->SetBlobStorage ( pBlobPool );
	}

	ISphFilter * Optimize() final
	{
		if ( m_dFilters.GetLength()==2 )
		{
			ISphFilter * pOpt = new Filter_And2 ( m_dFilters[0], m_dFilters[1] );
			m_dFilters.Reset();
			delete this;
			return pOpt;
		}
		if ( m_dFilters.GetLength()==3 )
		{
			ISphFilter * pOpt = new Filter_And3 ( m_dFilters[0], m_dFilters[1], m_dFilters[2] );
			m_dFilters.Reset();
			delete this;
			return pOpt;
		}
		return this;
	}
};


struct Filter_Or final : public ISphFilter
{
	ISphFilter * m_pLeft;
	ISphFilter * m_pRight;

	Filter_Or ( ISphFilter * pLeft, ISphFilter * pRight )
		: m_pLeft ( pLeft )
		, m_pRight ( pRight )
	{}

	~Filter_Or () final
	{
		SafeDelete ( m_pLeft );
		SafeDelete ( m_pRight );
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return ( m_pLeft->Eval ( tMatch ) || m_pRight->Eval ( tMatch ) );
	}

	bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const final
	{
		return ( m_pLeft->EvalBlock ( pMinDocinfo, pMaxDocinfo ) || m_pRight->EvalBlock ( pMinDocinfo, pMaxDocinfo ) );
	}

	bool Test ( const columnar::MinMaxVec_t & dMinMax ) const final
	{
		return ( m_pLeft->Test(dMinMax) || m_pRight->Test(dMinMax) );
	}

	void SetColumnar ( const columnar::Columnar_i * pColumnar ) final
	{
		m_pLeft->SetColumnar(pColumnar);
		m_pRight->SetColumnar(pColumnar);
	}

	void SetBlobStorage ( const BYTE * pBlobPool ) final
	{
		m_pLeft->SetBlobStorage ( pBlobPool );
		m_pRight->SetBlobStorage ( pBlobPool );
	}

	ISphFilter * Optimize() final
	{
		m_pLeft->Optimize();
		m_pRight->Optimize();
		return this;
	}
};


// not

struct Filter_Not final : public ISphFilter
{
	ISphFilter * m_pFilter;

	explicit Filter_Not ( ISphFilter * pFilter )
		: m_pFilter ( pFilter )
	{
		assert ( pFilter );
	}

	~Filter_Not () final
	{
		SafeDelete ( m_pFilter );
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return !m_pFilter->Eval ( tMatch );
	}

	bool EvalBlock ( const DWORD *, const DWORD * ) const final
	{
		// if block passes through the filter we can't just negate the
		// result since it's imprecise at this point
		return true;
	}

	void SetBlobStorage ( const BYTE * pBlobPool ) final
	{
		m_pFilter->SetBlobStorage ( pBlobPool );
	}

	void SetColumnar ( const columnar::Columnar_i * pColumnar ) final
	{
		m_pFilter->SetColumnar(pColumnar);
	}
};

/// impl

ISphFilter * ISphFilter::Join ( ISphFilter * pFilter )
{
	auto pAnd = new Filter_And();

	pAnd->Add ( this );
	pAnd->Add ( pFilter );

	return pAnd;
}

/// helper functions

CSphString FilterType2Str ( ESphFilter eFilterType )
{
	CSphString sFilterName;
	switch ( eFilterType )
	{
	case SPH_FILTER_VALUES:			sFilterName = "intvalues"; break;
	case SPH_FILTER_RANGE:			sFilterName = "intrange"; break;
	case SPH_FILTER_FLOATRANGE:		sFilterName = "floatrange"; break;
	case SPH_FILTER_STRING:			sFilterName = "string"; break;
	case SPH_FILTER_NULL:			sFilterName = "null"; break;
	default:						sFilterName.SetSprintf ( "(filter-type-%d)", eFilterType ); break;
	}

	return sFilterName;
}


static inline ISphFilter * ReportError ( CSphString & sError, const char * sMessage, ESphFilter eFilterType )
{
	sError.SetSprintf ( sMessage, FilterType2Str(eFilterType).cstr() );
	return nullptr;
}


#define CREATE_RANGE_FILTER(FILTER,SETTINGS) \
{ \
	if ( SETTINGS.m_bHasEqualMin ) \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<true,true>; \
		else \
			return new FILTER<true,false>; \
	} else \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<false,true>; \
		else \
			return new FILTER<false,false>; \
	} \
}


#define CREATE_RANGE_FILTER_WITH_OPEN(FILTER,SETTINGS) \
{ \
	if ( SETTINGS.m_bOpenLeft ) \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<true,true,true,false>; \
		else \
			return new FILTER<true,false,true,false>; \
	} else if ( SETTINGS.m_bOpenRight ) \
	{ \
		if ( SETTINGS.m_bHasEqualMin ) \
			return new FILTER<true,true,false,true>; \
		else \
			return new FILTER<false,true,false,true>; \
	} \
	assert ( !SETTINGS.m_bOpenLeft && !SETTINGS.m_bOpenRight ); \
	if ( SETTINGS.m_bHasEqualMin ) \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<true,true,false,false>; \
		else \
			return new FILTER<true,false,false,false>; \
	} else \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<false,true,false,false>; \
		else \
			return new FILTER<false,false,false,false>; \
	} \
}


#define CREATE_MVA_RANGE_FILTER(FILTER,T,SETTINGS) \
{ \
	if ( SETTINGS.m_bHasEqualMin ) \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<T,true,true>; \
		else \
			return new FILTER<T,true,false>; \
	} else \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<T,false,true>; \
		else \
			return new FILTER<T,false,false>; \
	} \
}


#define CREATE_EXPR_RANGE_FILTER(FILTER,EXPR,bHasEqualMin,bHasEqualMax) \
{ \
	if ( bHasEqualMin ) \
	{ \
		if ( bHasEqualMax ) \
			return new FILTER<true,true>(EXPR); \
		else \
			return new FILTER<true,false>(EXPR); \
	} else \
	{ \
		if ( bHasEqualMax ) \
			return new FILTER<false,true>(EXPR); \
		else \
			return new FILTER<false,false>(EXPR); \
	} \
}


static ISphFilter * CreateSpecialFilter ( const CSphString & sName, const CSphFilterSettings & tSettings, CSphString & sError )
{
	if ( sName=="@weight" )
	{
		switch ( tSettings.m_eType )
		{
		case SPH_FILTER_VALUES:	return new Filter_WeightValues;
		case SPH_FILTER_RANGE:	CREATE_RANGE_FILTER ( Filter_WeightRange, tSettings );
		default:
			return ReportError ( sError, "unsupported filter type '%s' on @weight", tSettings.m_eType );
		}
	}

	return nullptr;
}


static ISphFilter * CreateFilter ( const CSphFilterSettings & tSettings, ESphFilter eFilterType,ESphAttr eAttrType, const CSphAttrLocator & tLoc, ESphCollation eCollation, CSphString & sError, CSphString & sWarning )
{
	if ( IsMvaAttr(eAttrType) )
	{
		if (!( eFilterType==SPH_FILTER_VALUES || eFilterType==SPH_FILTER_RANGE ))
			return ReportError ( sError, "unsupported filter type '%s' on MVA column", eFilterType );

		if ( tSettings.m_eMvaFunc==SPH_MVAFUNC_NONE )
			sWarning.SetSprintf ( "use an explicit ANY()/ALL() around a filter on MVA column" );

		bool bWide = eAttrType==SPH_ATTR_INT64SET || eAttrType==SPH_ATTR_INT64SET_PTR;
		bool bRange = ( eFilterType==SPH_FILTER_RANGE );
		bool bAll = ( tSettings.m_eMvaFunc==SPH_MVAFUNC_ALL );
		int iIndex = bWide*4 + bRange*2 + bAll;

		switch ( iIndex )
		{
			case 0:	return new Filter_MVAValues_Any_c<DWORD>();
			case 1:	return new Filter_MVAValues_All_c<DWORD>();

			case 2:	CREATE_MVA_RANGE_FILTER ( Filter_MVARange_Any_c, DWORD, tSettings );
			case 3:	CREATE_MVA_RANGE_FILTER ( Filter_MVARange_All_c, DWORD, tSettings );

			case 4:	return new Filter_MVAValues_Any_c<int64_t>();
			case 5:	return new Filter_MVAValues_All_c<int64_t>();

			case 6:	CREATE_MVA_RANGE_FILTER ( Filter_MVARange_Any_c, int64_t, tSettings );
			case 7:	CREATE_MVA_RANGE_FILTER ( Filter_MVARange_All_c, int64_t, tSettings );
			default:
				assert (false && "UB in CreateFilter");
		}
	}

	// float
	if ( eAttrType==SPH_ATTR_FLOAT )
	{
		if ( eFilterType==SPH_FILTER_FLOATRANGE || eFilterType==SPH_FILTER_RANGE )
			CREATE_RANGE_FILTER ( Filter_FloatRange, tSettings );

		return ReportError ( sError, "unsupported filter type '%s' on float column", eFilterType );
	}

	if ( eAttrType==SPH_ATTR_STRING || eAttrType==SPH_ATTR_STRINGPTR )
	{
		if ( eFilterType==SPH_FILTER_STRING_LIST )
		{
			if ( tSettings.m_eMvaFunc==SPH_MVAFUNC_NONE )
				return new Filter_StringValues_c ( eCollation );
			else if ( tSettings.m_eMvaFunc==SPH_MVAFUNC_ANY )
				return new Filter_StringTagsAny_c;
			else if ( tSettings.m_eMvaFunc==SPH_MVAFUNC_ALL )
				return new Filter_StringTagsAll_c;
		}
		else
			return new FilterString_c ( eCollation, tSettings.m_bHasEqualMin || tSettings.m_bHasEqualMax );
	}

	// non-float, non-MVA
	switch ( eFilterType )
	{
		case SPH_FILTER_VALUES:
			if ( tSettings.GetNumValues()==1 && ( eAttrType==SPH_ATTR_INTEGER || eAttrType==SPH_ATTR_BIGINT || eAttrType==SPH_ATTR_TOKENCOUNT ) )
			{
				if ( ( eAttrType==SPH_ATTR_INTEGER || eAttrType==SPH_ATTR_TOKENCOUNT ) && !tLoc.m_bDynamic && tLoc.m_iBitCount==32 && ( tLoc.m_iBitOffset % 32 )==0 )
					return new Filter_SingleValueStatic32;
				else
					return new Filter_SingleValue;
			} else
				return new Filter_Values;

		case SPH_FILTER_RANGE:	CREATE_RANGE_FILTER_WITH_OPEN ( Filter_Range, tSettings );
		default:				return ReportError ( sError, "unsupported filter type '%s' on int column", eFilterType );
	}
}

//////////////////////////////////////////////////////////////////////////
// EXPRESSION STUFF
//////////////////////////////////////////////////////////////////////////

template<typename BASE>
class ExprFilter_c : public BASE
{
public:
	explicit ExprFilter_c ( ISphExpr * pExpr )
		: m_pExpr ( pExpr )
	{
		SafeAddRef ( pExpr );
	}

	void SetBlobStorage ( const BYTE * pBlobPool ) final
	{
		m_pBlobPool = pBlobPool;
		if ( m_pExpr )
			m_pExpr->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
	}

	void SetColumnar ( const columnar::Columnar_i * pColumnar ) final
	{
		if ( !m_pExpr )
			return;

		m_pExpr->Command ( SPH_EXPR_SET_COLUMNAR, (void*)pColumnar );
	}

protected:
	const BYTE *				m_pBlobPool {nullptr};
	CSphRefcountedPtr<ISphExpr>	m_pExpr;
};


template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
class ExprFilterFloatRange_c : public ExprFilter_c<IFilter_Range>
{
public:
	explicit ExprFilterFloatRange_c ( ISphExpr * pExpr )
		: ExprFilter_c<IFilter_Range> ( pExpr )
	{}

	float m_fMinValue = 0.0f;
	float m_fMaxValue = 0.0f;

	void SetRangeFloat ( float fMin, float fMax ) override
	{
		m_fMinValue = fMin;
		m_fMaxValue = fMax;
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return EvalRange<HAS_EQUAL_MIN, HAS_EQUAL_MAX,false,false,float> ( m_pExpr->Eval ( tMatch ), m_fMinValue, m_fMaxValue );
	}
};


template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
class ExprFilterRange_c : public ExprFilter_c<IFilter_Range>
{
public:
	explicit ExprFilterRange_c ( ISphExpr * pExpr )
		: ExprFilter_c<IFilter_Range> ( pExpr )
	{}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		return EvalRange<HAS_EQUAL_MIN, HAS_EQUAL_MAX> ( m_pExpr->Int64Eval(tMatch), m_iMinValue, m_iMaxValue );
	}
};


class ExprFilterValues_c : public ExprFilter_c<IFilter_Values>
{
public:
	explicit ExprFilterValues_c ( ISphExpr * pExpr )
		: ExprFilter_c<IFilter_Values> ( pExpr )
	{}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		assert ( this->m_pExpr );
		return EvalValues ( m_pExpr->Int64Eval ( tMatch ) );
	}
};


class ExprFilterString_c : public ExprFilter_c<ISphFilter>
{
protected:
	CSphString				m_sVal;
	int						m_iValLength = 0;
	SphStringCmp_fn			m_fnStrCmp;
	bool					m_bEq;

public:
	explicit ExprFilterString_c ( ISphExpr * pExpr, ESphCollation eCollation, bool bEq )
		: ExprFilter_c<ISphFilter> ( pExpr )
		, m_fnStrCmp ( GetStringCmpFunc ( eCollation ) )
		, m_bEq ( bEq )
	{}

	void SetRefString ( const CSphString * pRef, int iCount ) final
	{
		assert ( iCount<2 );
		if ( pRef )
		{
			m_sVal = *pRef;
			m_iValLength = m_sVal.Length();
		}
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		// attribute storages can fetch string length without reading the string itself
		int iLen = m_pExpr->StringLenEval ( tMatch );
		// StringLenEval returns -1 if not supported by expression
		if ( iLen!=-1 && m_iValLength!=iLen )
			return false;

		const BYTE * pVal = nullptr;
		iLen = m_pExpr->StringEval ( tMatch, &pVal );
		bool bEq = m_fnStrCmp ( {pVal, iLen}, {(const BYTE *) m_sVal.cstr (), m_iValLength}, false )==0;
		return ( m_bEq==bEq );
	}
};


class ExprFilterNull_c : public ExprFilter_c<ISphFilter>
{
public:
	ExprFilterNull_c ( ISphExpr * pExpr, bool bEquals, bool bCheckOnlyKey )
		: ExprFilter_c<ISphFilter> ( pExpr )
		, m_bEquals ( bEquals )
		, m_bCheckOnlyKey ( bCheckOnlyKey )
	{}

	void SetLocator ( const CSphAttrLocator & tLocator ) final
	{
		m_tLoc = tLocator;
	}

	ESphJsonType GetKey ( const CSphMatch & tMatch ) const
	{
		if ( !m_pExpr )
			return JSON_EOF;

		uint64_t uValue = m_pExpr->Int64Eval ( tMatch );
		if ( uValue==0 ) // either no data or invalid path
			return JSON_EOF;

		return sphJsonUnpackType ( uValue );
	}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		if ( !m_pExpr ) // regular attribute? return true if blob size is null
		{
			int iLen = sphGetBlobAttrLen ( tMatch, m_tLoc, m_pBlobPool );
			return m_bEquals ^ ( iLen!=0 );
		}

		// possibly json
		ESphJsonType eRes = GetKey ( tMatch );
		if ( m_bCheckOnlyKey )
			return ( eRes!=JSON_EOF );

		return m_bEquals ^ ( eRes!=JSON_EOF && eRes!=JSON_NULL );
	}

protected:
	CSphAttrLocator	m_tLoc;
	const bool		m_bEquals;
	const bool		m_bCheckOnlyKey;
};

// TODO: implement expression -> filter tree optimization to extract filters from general expression and got rid of all filters wo block-level optimizer
// wrapper for whole expression that evaluates to 0 and 1 on filtering
class ExprFilterProxy_c : public ExprFilter_c<ISphFilter>
{
protected:
	ESphAttr m_eAttrType = SPH_ATTR_NONE;

public:
	ExprFilterProxy_c ( ISphExpr * pExpr, ESphAttr eAttrType )
		: ExprFilter_c<ISphFilter> ( pExpr )
		, m_eAttrType ( eAttrType )
	{}

	bool Eval ( const CSphMatch & tMatch ) const final
	{
		switch ( m_eAttrType )
		{
			case SPH_ATTR_INTEGER:
				return ( m_pExpr->IntEval ( tMatch )>0 );

			case SPH_ATTR_BIGINT:
			case SPH_ATTR_JSON_FIELD:
				return ( m_pExpr->Int64Eval ( tMatch )>0 );

			case SPH_ATTR_INT64SET:
			case SPH_ATTR_UINT32SET:
				return ( m_pExpr->IntEval ( tMatch )>0 );

			default:
				return ( m_pExpr->Eval ( tMatch )>0.0f );
		}
	}
};



static ISphFilter * CreateFilterExpr ( ISphExpr * _pExpr, const CSphFilterSettings & tSettings, const CommonFilterSettings_t & tFixedSettings, CSphString & sError, ESphCollation eCollation, ESphAttr eAttrType )
{
	CSphRefcountedPtr<ISphExpr> pExpr { _pExpr };
	SafeAddRef ( _pExpr );

	// auto-convert all JSON types except SPH_FILTER_NULL, it needs more info
	bool bAutoConvert = false;
	bool bJsonExpr = false;
	if ( pExpr && tFixedSettings.m_eType!=SPH_FILTER_NULL )
		bJsonExpr = pExpr->IsJson ( bAutoConvert );
	if ( bJsonExpr && !bAutoConvert )
		pExpr = sphJsonFieldConv ( pExpr );

	switch ( tFixedSettings.m_eType )
	{
		case SPH_FILTER_VALUES:			return new ExprFilterValues_c ( pExpr );
		case SPH_FILTER_FLOATRANGE:		CREATE_EXPR_RANGE_FILTER ( ExprFilterFloatRange_c, pExpr, tSettings.m_bHasEqualMin, tSettings.m_bHasEqualMax );
		case SPH_FILTER_RANGE:			CREATE_EXPR_RANGE_FILTER ( ExprFilterRange_c, pExpr, tSettings.m_bHasEqualMin, tSettings.m_bHasEqualMax );
		case SPH_FILTER_STRING:			return new ExprFilterString_c ( pExpr, eCollation, tSettings.m_bHasEqualMin || tSettings.m_bHasEqualMax );
		case SPH_FILTER_NULL:			return new ExprFilterNull_c ( pExpr, tSettings.m_bIsNull, false );
		case SPH_FILTER_EXPRESSION:
		{
			if ( eAttrType!=SPH_ATTR_INTEGER && eAttrType!=SPH_ATTR_BIGINT && eAttrType!=SPH_ATTR_JSON_FIELD && eAttrType!=SPH_ATTR_FLOAT
				&& eAttrType!=SPH_ATTR_INT64SET && eAttrType!=SPH_ATTR_UINT32SET )
			{
				sError = "filter expression must evaluate to integer or float";
				return nullptr;
			} else
			{
				return new ExprFilterProxy_c ( pExpr, eAttrType );
			}
		}
		default:
			sError = "this filter type on expressions is not implemented yet";
			return nullptr;
	}
}


static ISphFilter * TryToCreateExpressionFilter ( CSphRefcountedPtr<ISphExpr> & pExpr, const CSphString & sAttrName, const ISphSchema & tSchema, const CSphFilterSettings & tSettings,
	const CommonFilterSettings_t & tFixedSettings, ExprParseArgs_t & tExprArgs, CSphString & sError )
{
	pExpr = sphExprParse ( sAttrName.cstr(), tSchema, sError, tExprArgs );
	if ( pExpr )
		return CreateFilterExpr ( pExpr, tSettings, tFixedSettings, sError, tExprArgs.m_eCollation, *tExprArgs.m_pAttrType );
	else
	{
		sError.SetSprintf ( "no such filter attribute '%s'", sAttrName.cstr() );
		return nullptr;
	}
}


static void SetFilterLocator ( ISphFilter * pFilter, const CSphFilterSettings & tSettings, const CreateFilterContext_t & tCtx, const CSphString & sAttrName )
{
	if ( !pFilter )
		return;

	assert ( tCtx.m_pSchema );
	const ISphSchema & tSchema = *tCtx.m_pSchema;

	int iAttr = ( tSettings.m_eType!=SPH_FILTER_EXPRESSION ? tSchema.GetAttrIndex ( sAttrName.cstr() ) : -1 );
	if ( iAttr==-1 )
		return;

	const CSphColumnInfo & tAttr = tSchema.GetAttr(iAttr);

	pFilter->SetLocator ( tAttr.m_tLocator );
}


static void SetFilterSettings ( ISphFilter * pFilter, const CSphFilterSettings & tSettings, const CommonFilterSettings_t & tFixedSettings, const CreateFilterContext_t & tCtx )
{
	if ( !pFilter )
		return;

	pFilter->SetBlobStorage ( tCtx.m_pBlobPool );
	pFilter->SetColumnar ( tCtx.m_pColumnar );
	pFilter->SetRange ( tSettings.m_iMinValue, tSettings.m_iMaxValue );
	if ( tFixedSettings.m_eType==SPH_FILTER_FLOATRANGE )
		pFilter->SetRangeFloat ( (float)tFixedSettings.m_fMinValue, (float)tFixedSettings.m_fMaxValue );

	pFilter->SetRefString ( tSettings.m_dStrings.Begin(), tSettings.m_dStrings.GetLength() );
	if ( tSettings.GetNumValues() > 0 )
	{
		pFilter->SetValues ( tSettings.GetValueArray(), tSettings.GetNumValues() );
#ifndef NDEBUG
		// check that the values are actually sorted
		const SphAttr_t * pValues = tSettings.GetValueArray();
		int iValues = tSettings.GetNumValues ();
		for ( int i=1; i<iValues; ++i )
			assert ( pValues[i]>=pValues[i-1] );
#endif
	}
}


static ISphFilter * TryToCreateExcludeFilter ( ISphFilter * pFilter, const CSphFilterSettings & tSettings )
{
	if ( !pFilter )
		return nullptr;

	// some filters can handle exclude flag themselves
	if ( pFilter->CanExclude() )
		return pFilter;

	if ( tSettings.m_bExclude )
		return new Filter_Not(pFilter);

	return pFilter;
}


static ISphFilter * TryToCreateJsonNullFilter ( ISphFilter * pFilter, const CSphFilterSettings & tSettings, const CreateFilterContext_t & tCtx, ESphAttr eAttrType, ISphExpr * pExpr )
{
	if ( !pFilter || !pExpr )
		return pFilter;

	// filter for json || json.field needs to check that key exists
	if ( ( eAttrType==SPH_ATTR_JSON || eAttrType==SPH_ATTR_JSON_FIELD ) && tSettings.m_eType!=SPH_FILTER_NULL )
	{
		ISphFilter * pNotNull = new ExprFilterNull_c ( pExpr, false, true );
		pNotNull->SetBlobStorage ( tCtx.m_pBlobPool );

		return new Filter_And2 ( pFilter, pNotNull );
	}

	return pFilter;
}


static bool TryToCreateSpecialFilter ( ISphFilter * & pFilter, const CSphFilterSettings & tSettings, bool bHaving, const CSphString & sAttrName, CSphString & sError )
{
	assert ( !pFilter );

	// try to create a filter on a special attribute
	if ( sAttrName.Begins("@") && !bHaving && ( sAttrName=="@groupby" || sAttrName=="@count" || sAttrName=="@distinct" ) )
	{
		sError.SetSprintf ( "unsupported filter column '%s'", sAttrName.cstr() );
		return false;
	}

	if ( sAttrName.Begins("@") )
	{
		pFilter = CreateSpecialFilter ( sAttrName, tSettings, sError );
		if ( !pFilter && !sError.IsEmpty() )
			return false;
	}

	return true;
}


static bool CanSpawnColumnarFilter ( int iAttr, const ISphSchema & tSchema )
{
	if ( iAttr<0 )
		return false;

	const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
	if ( tCol.IsColumnar() )
		return true;

	// spawn special filter even if we have an expression in the column
	// because the filter is (usually) faster
	if ( tCol.IsColumnarExpr() && tCol.m_eStage>SPH_EVAL_PREFILTER )
		return true;

	return false;
}


static ISphFilter * TryToCreateExpressionFilter ( ISphFilter * pFilter, const CSphFilterSettings & tSettings, const CreateFilterContext_t & tCtx, const CSphString & sAttrName,
	const CommonFilterSettings_t & tFixedSettings, ESphAttr & eAttrType, CSphRefcountedPtr<ISphExpr> & pExpr, CSphString & sError, CSphString & sWarning )
{
	if ( pFilter )
		return pFilter;

	assert ( tCtx.m_pSchema );
	const ISphSchema & tSchema = *tCtx.m_pSchema;

	int iAttr = ( tFixedSettings.m_eType!=SPH_FILTER_EXPRESSION ? tSchema.GetAttrIndex ( sAttrName.cstr() ) : -1 );
	bool bColumnar = CanSpawnColumnarFilter ( iAttr, tSchema );

	if ( iAttr>=0 && !bColumnar )
		return nullptr;

	pFilter = TryToCreateColumnarFilter ( iAttr, tSchema, tSettings, tFixedSettings, tCtx.m_eCollation, sError, sWarning );
	if ( !pFilter )
	{
		ExprParseArgs_t tExprArgs;
		tExprArgs.m_pAttrType = &eAttrType;
		tExprArgs.m_eCollation = tCtx.m_eCollation;
		pFilter = TryToCreateExpressionFilter ( pExpr, sAttrName, tSchema, tSettings, tFixedSettings, tExprArgs, sError );
	}

	return pFilter;
}


static ISphFilter * TryToCreatePlainAttrFilter ( ISphFilter * pFilter, const CSphFilterSettings & tSettings, const CreateFilterContext_t & tCtx, bool bHaving, const CSphString & sAttrName,
	CommonFilterSettings_t & tFixedSettings, ESphAttr & eAttrType, CSphRefcountedPtr<ISphExpr> & pExpr, CSphString & sError, CSphString & sWarning )
{
	if ( pFilter )
		return pFilter;

	assert ( tCtx.m_pSchema );
	const ISphSchema & tSchema = *tCtx.m_pSchema;

	int iAttr = ( tSettings.m_eType!=SPH_FILTER_EXPRESSION ? tSchema.GetAttrIndex ( sAttrName.cstr() ) : -1 );
	bool bColumnar = iAttr>=0 && tSchema.GetAttr(iAttr).IsColumnar();

	if ( iAttr<0 || bColumnar )
		return nullptr;

	const CSphColumnInfo & tAttr =  tSchema.GetAttr(iAttr);
	if ( !bHaving && tAttr.m_eAggrFunc!=SPH_AGGR_NONE )
	{
		sError.SetSprintf ( "unsupported filter '%s' on aggregate column", sAttrName.cstr() );
		return nullptr;
	}

	if ( IsNotRealAttribute(tAttr) )
	{
		sError.SetSprintf ( "unsupported column '%s' (stored field, NOT attribute)", sAttrName.cstr ());
		return nullptr;
	}

	if ( tAttr.m_eAttrType==SPH_ATTR_JSON || tAttr.m_eAttrType==SPH_ATTR_JSON_FIELD )
	{
		pExpr = tAttr.m_pExpr;
		eAttrType = tAttr.m_eAttrType;
		if ( tAttr.m_eAttrType==SPH_ATTR_JSON && !pExpr && tSettings.m_eType!=SPH_FILTER_NULL )
		{
			ExprParseArgs_t tExprArgs;
			tExprArgs.m_pAttrType = &eAttrType;
			tExprArgs.m_eCollation = tCtx.m_eCollation;
			pExpr = sphExprParse ( sAttrName.cstr(), tSchema, sError, tExprArgs );
		}
		pFilter = CreateFilterExpr ( pExpr, tSettings, tFixedSettings, sError, tCtx.m_eCollation, tAttr.m_eAttrType );

	}
	else
		pFilter = CreateFilter ( tSettings, tFixedSettings.m_eType, tAttr.m_eAttrType, tAttr.m_tLocator, tCtx.m_eCollation, sError, sWarning );

	return pFilter;
}


static bool FixupFilterSettings ( const CSphFilterSettings & tSettings, CommonFilterSettings_t & tFixedSettings, const CreateFilterContext_t & tCtx, const CSphString & sAttrName, CSphString & sError )
{
	assert ( tCtx.m_pSchema );
	const ISphSchema & tSchema = *tCtx.m_pSchema;

	int iAttr = ( tSettings.m_eType!=SPH_FILTER_EXPRESSION ? tSchema.GetAttrIndex ( sAttrName.cstr() ) : -1 );
	if ( iAttr<0 )
		return true;

	const CSphColumnInfo & tAttr = tSchema.GetAttr(iAttr);
	bool bIntFilter = tSettings.m_eType==SPH_FILTER_VALUES || tSettings.m_eType==SPH_FILTER_RANGE || tSettings.m_eType==SPH_FILTER_FLOATRANGE;
	if ( bIntFilter && ( tAttr.m_eAttrType==SPH_ATTR_STRING || tAttr.m_eAttrType==SPH_ATTR_STRINGPTR ) )
	{
		sError.SetSprintf ( "unsupported filter on a '%s' string column", tAttr.m_sName.cstr() );
		return false;
	}

	FixupFilterSettings ( tSettings, tAttr.m_eAttrType, tFixedSettings );
	return true;
}

void FixupFilterSettings ( const CSphFilterSettings & tSettings, ESphAttr eAttrType, CommonFilterSettings_t & tFixedSettings )
{
	// fixup "fltcol=intval" conditions
	if ( eAttrType!=SPH_ATTR_FLOAT )
		return;

	if ( tSettings.m_eType==SPH_FILTER_VALUES && tSettings.GetNumValues()==1 )
	{
		tFixedSettings.m_eType = SPH_FILTER_FLOATRANGE;
		tFixedSettings.m_fMinValue = tFixedSettings.m_fMaxValue = (float)tSettings.GetValue(0);
	}

	if ( tSettings.m_eType==SPH_FILTER_RANGE )
	{
		tFixedSettings.m_eType = SPH_FILTER_FLOATRANGE;
		tFixedSettings.m_fMinValue = (float)tSettings.m_iMinValue;
		tFixedSettings.m_fMaxValue = (float)tSettings.m_iMaxValue;
	}
}


static bool ValuesAreSame ( const CSphVector<SphAttr_t> & dLeft, const CSphVector<SphAttr_t> & dRight )
{
	if ( dLeft.GetLength()!=dRight.GetLength() )
		return false;

	return !memcmp ( dLeft.Begin(), dRight.Begin(), dLeft.GetLengthBytes() );
}


using namespace boost::icl;

static interval<int64_t>::type ConstructInterval ( const CSphFilterSettings & tFilter )
{
	assert ( !tFilter.m_bExclude );
	
	int64_t iMin = tFilter.m_iMinValue;
	int64_t iMax = tFilter.m_iMaxValue;
	bool bHasEqualMin = tFilter.m_bHasEqualMin;
	bool bHasEqualMax = tFilter.m_bHasEqualMax;

	if ( tFilter.m_bOpenLeft )
	{
		iMin = INT64_MIN;
		bHasEqualMin = true;
	}

	if ( tFilter.m_bOpenRight )
	{
		iMax = INT64_MAX;
		bHasEqualMax = true;
	}

	if ( bHasEqualMin && bHasEqualMax )	return interval<int64_t>::closed ( iMin, iMax );
	if ( bHasEqualMin )					return interval<int64_t>::right_open ( iMin, iMax );
	if ( bHasEqualMax )					return interval<int64_t>::left_open ( iMin, iMax );

	return interval<int64_t>::open ( iMin, iMax );
}


static void DeconstructInterval ( CSphFilterSettings & tFilter, const interval<int64_t>::type & tInterval )
{
	tFilter.m_iMinValue = tInterval.lower();
	tFilter.m_iMaxValue = tInterval.upper();
	tFilter.m_bHasEqualMin = tInterval.bounds() == interval_bounds::closed() || tInterval.bounds() == interval_bounds::right_open();
	tFilter.m_bHasEqualMax = tInterval.bounds() == interval_bounds::closed() || tInterval.bounds() == interval_bounds::left_open();
	tFilter.m_bOpenLeft = tInterval.lower()==INT64_MIN && tFilter.m_bHasEqualMin;
	tFilter.m_bOpenRight = tInterval.lower()==INT64_MAX && tFilter.m_bHasEqualMax;
}


static bool CalcRangeMerge ( CSphFilterSettings & tLeft, const CSphFilterSettings & tRight )
{
	auto tInterval1 = ConstructInterval(tLeft);
	auto tInterval2 = ConstructInterval(tRight);

	auto tResult = tInterval1 & tInterval2;
	if ( boost::icl::is_empty(tResult) )
		return false;

	DeconstructInterval ( tLeft, tResult );
	return true;
}


static bool AllInRange ( const CSphVector<int64_t> & dValues, const CSphFilterSettings & tFilter )
{
	int iIndex = 0;
	iIndex += 2*( tFilter.m_bHasEqualMin ? 1 : 0 ) + ( tFilter.m_bHasEqualMax ? 1 : 0 );

	switch ( iIndex )
	{
	case 0:	return MvaEval_RangeAll<int64_t,false,false> ( dValues.Begin(), dValues.GetLength(), tFilter.m_iMinValue, tFilter.m_iMaxValue );
	case 1:	return MvaEval_RangeAll<int64_t,false,true>  ( dValues.Begin(), dValues.GetLength(), tFilter.m_iMinValue, tFilter.m_iMaxValue );
	case 2:	return MvaEval_RangeAll<int64_t,true,false>  ( dValues.Begin(), dValues.GetLength(), tFilter.m_iMinValue, tFilter.m_iMaxValue );
	case 3:	return MvaEval_RangeAll<int64_t,true,true>   ( dValues.Begin(), dValues.GetLength(), tFilter.m_iMinValue, tFilter.m_iMaxValue );
	default:
		assert(0);
		return false;
	}
}


static bool AllOutsideRange ( const CSphVector<int64_t> & dValues, const CSphFilterSettings & tFilter )
{
	int iIndex = 0;
	iIndex += 2*( tFilter.m_bHasEqualMin ? 1 : 0 ) + ( tFilter.m_bHasEqualMax ? 1 : 0 );

	switch ( iIndex )
	{
	case 0:	return !MvaEval_RangeAny<int64_t,false,false> ( dValues.Begin(), dValues.GetLength(), tFilter.m_iMinValue, tFilter.m_iMaxValue );
	case 1:	return !MvaEval_RangeAny<int64_t,false,true>  ( dValues.Begin(), dValues.GetLength(), tFilter.m_iMinValue, tFilter.m_iMaxValue );
	case 2:	return !MvaEval_RangeAny<int64_t,true,false>  ( dValues.Begin(), dValues.GetLength(), tFilter.m_iMinValue, tFilter.m_iMaxValue );
	case 3:	return !MvaEval_RangeAny<int64_t,true,true>   ( dValues.Begin(), dValues.GetLength(), tFilter.m_iMinValue, tFilter.m_iMaxValue );
	default:
		assert(0);
		return false;
	}
}


static bool MergeFilters ( CSphFilterSettings & tLeft, const CSphFilterSettings & tRight, bool & bRemoveLeft, bool & bRejectIndex )
{
	bRemoveLeft = false;
	bRejectIndex = false;

	if ( tLeft.m_eType==SPH_FILTER_VALUES )
	{
		if ( tRight.m_eType==SPH_FILTER_VALUES )
		{
			if ( tLeft.m_bExclude!=tRight.m_bExclude )
			{
				// IN (a) and NOT IN (a) => null
				if ( ValuesAreSame ( tLeft.m_dValues, tRight.m_dValues ) )
				{
					bRejectIndex = true;
					return true;
				}
			}
			else if ( tLeft.m_bExclude )
			{
				// NOT IN (a) and NOT IN(b) => NOT IN(a,b)
				SphAttr_t * pValues = tLeft.m_dValues.AddN ( tRight.m_dValues.GetLength() );
				memcpy ( pValues, tRight.m_dValues.Begin(), tRight.m_dValues.GetLength()*sizeof ( tRight.m_dValues[0] ) );
				tLeft.m_dValues.Uniq();
				return true;
			}
		}

		if ( tRight.m_eType==SPH_FILTER_RANGE )
		{
			// we have "values", new filter is "range"
			if ( !tLeft.m_bExclude && !tRight.m_bExclude )
			{
				if ( AllOutsideRange ( tLeft.m_dValues, tRight ) )
				{
					bRejectIndex = true;
					return true;
				}

				if ( AllInRange ( tLeft.m_dValues, tRight ) )
					return true;
			}
		}
	}

	if ( tLeft.m_eType==SPH_FILTER_RANGE )
	{
		if ( tRight.m_eType==SPH_FILTER_RANGE )
		{
			if ( !tLeft.m_bExclude && !tRight.m_bExclude )
			{
				if ( !CalcRangeMerge ( tLeft, tRight ) )
					bRejectIndex = true;

				return true;
			}
		}

		if ( tRight.m_eType==SPH_FILTER_VALUES )
		{
			// we have "range", new filter is "values"
			if ( !tLeft.m_bExclude && !tRight.m_bExclude )
			{
				if ( AllOutsideRange ( tRight.m_dValues, tLeft ) )
				{
					bRejectIndex = true;
					return true;
				}

				if ( AllInRange ( tRight.m_dValues, tLeft ) )
				{
					bRemoveLeft = true;
					return true;
				}
			}
		}
	}

	return false;
}


void OptimizeFilters ( CSphVector<CSphFilterSettings> & dFilters )
{
	bool bRejectIndex = false;

	SmallStringHash_T<int> hAttrNames;
	for ( auto & i : dFilters )
	{
		int * pFound = hAttrNames ( i.m_sAttrName );
		if ( pFound )
			(*pFound)++;
		else
			hAttrNames.Add ( 1, i.m_sAttrName );
	}

	for ( auto & i : hAttrNames )
	{
		if ( i.second<2 )
			continue;

		bool bOk = true;
		for ( const auto & tFilter : dFilters )
			if ( tFilter.m_sAttrName==i.first && ( tFilter.m_eType!=SPH_FILTER_VALUES && tFilter.m_eType!=SPH_FILTER_RANGE ) )
			{
				bOk = false;
				break;
			}

		if ( !bOk )
			continue;

		int iLeft = -1;
		ARRAY_FOREACH ( iFilter, dFilters )
		{
			auto & tFilter = dFilters[iFilter];
			if ( tFilter.m_sAttrName!=i.first )
				continue;

			if ( iLeft==-1 )
			{
				iLeft = iFilter;
				continue;
			}

			bool bRemoveLeft = false;
			if ( MergeFilters ( dFilters[iLeft], tFilter, bRemoveLeft, bRejectIndex ) )
			{
				dFilters.RemoveFast ( bRemoveLeft ? iLeft : iFilter );
				iLeft = -1;
				iFilter = 0;
			}
		}
	}

	if ( bRejectIndex )
	{
		// create a fake filter that will fail index early reject
		CSphFilterSettings tFake;
		tFake.m_sAttrName = sphGetDocidName();
		tFake.m_eType = SPH_FILTER_VALUES;
		tFake.m_dValues.Add(-1);

		dFilters.Resize(0);
		dFilters.Add(tFake);
	}
}


static ISphFilter * CreateFilter ( const CSphFilterSettings & tSettings, const CSphString & sAttrName, const CreateFilterContext_t & tCtx, bool bHaving, CSphString & sError, CSphString & sWarning )
{
	ISphFilter * pFilter = nullptr;

	if ( !TryToCreateSpecialFilter ( pFilter, tSettings, bHaving, sAttrName, sError ) )
		return nullptr;

	CommonFilterSettings_t tFixedSettings = (CommonFilterSettings_t)tSettings;
	ESphAttr eAttrType = SPH_ATTR_NONE;
	CSphRefcountedPtr<ISphExpr> pExpr;

	if ( !FixupFilterSettings ( tSettings, tFixedSettings, tCtx, sAttrName, sError ) )
		return nullptr;

	pFilter = TryToCreateExpressionFilter ( pFilter, tSettings, tCtx, sAttrName, tFixedSettings, eAttrType, pExpr, sError, sWarning );
	pFilter = TryToCreatePlainAttrFilter ( pFilter, tSettings, tCtx, bHaving, sAttrName, tFixedSettings, eAttrType, pExpr, sError, sWarning );

	SetFilterLocator ( pFilter, tSettings, tCtx, sAttrName );
	SetFilterSettings ( pFilter, tSettings, tFixedSettings, tCtx );

	pFilter = TryToCreateExcludeFilter ( pFilter, tSettings );
	pFilter = TryToCreateJsonNullFilter ( pFilter, tSettings, tCtx, eAttrType, pExpr );

	return pFilter;
}


ISphFilter * sphCreateFilter ( const CSphFilterSettings & tSettings, const CreateFilterContext_t & tCtx, CSphString & sError, CSphString & sWarning )
{
	return CreateFilter ( tSettings, tSettings.m_sAttrName, tCtx, false, sError, sWarning );
}

ISphFilter * sphCreateAggrFilter ( const CSphFilterSettings * pSettings, const CSphString & sAttrName, const ISphSchema & tSchema, CSphString & sError )
{
	assert ( pSettings );
	CSphString sWarning;
	CreateFilterContext_t tCtx;
	tCtx.m_pSchema = &tSchema;
	tCtx.m_eCollation = SPH_COLLATION_DEFAULT;

	ISphFilter * pRes = CreateFilter ( *pSettings, sAttrName, tCtx, true, sError, sWarning );
	assert ( sWarning.IsEmpty() );
	return pRes;
}


ISphFilter * sphJoinFilters ( ISphFilter * pA, ISphFilter * pB )
{
	if ( pA )
		return pB ? pA->Join(pB) : pA;
	return pB;
}

static bool IsWeightColumn ( const CSphString & sAttr, const ISphSchema & tSchema )
{
	if ( sAttr=="@weight" )
		return true;

	const CSphColumnInfo * pCol = tSchema.GetAttr ( sAttr.cstr() );
	return ( pCol && pCol->m_bWeight );
}

CreateFilterContext_t::~CreateFilterContext_t()
{
	SafeDelete ( m_pFilter );
	SafeDelete ( m_pWeightFilter );
}


static ISphFilter * CreateFilterNode ( CreateFilterContext_t & tCtx, int iNode, CSphString & sError, CSphString & sWarning, bool & bHasWeight )
{
	const FilterTreeItem_t * pCur = tCtx.m_pFilterTree->Begin() + iNode;
	if ( pCur->m_iFilterItem!=-1 )
	{
		const CSphFilterSettings * pFilterSettings = tCtx.m_pFilters->Begin() + pCur->m_iFilterItem;
		if ( pFilterSettings->m_sAttrName.IsEmpty() )
		{
			sError = "filter with empty name";
			return nullptr;
		}

		bHasWeight |= IsWeightColumn ( pFilterSettings->m_sAttrName, *tCtx.m_pSchema );

		// bind user variable local to that daemon
		CSphFilterSettings tUservar;
		if ( pFilterSettings->m_eType==SPH_FILTER_USERVAR )
		{
			const CSphString * sVar = pFilterSettings->m_dStrings.GetLength()==1 ? pFilterSettings->m_dStrings.Begin() : NULL;
			if ( !UservarsAvailable() || !sVar )
			{
				sError = "no global variables found";
				return nullptr;
			}

			const UservarIntSet_c pUservar = Uservars( *sVar );
			if ( !pUservar )
			{
				sError.SetSprintf ( "undefined global variable '%s'", sVar->cstr() );
				return nullptr;
			}

			// NON-obvious, legacy! Liveness of value in tCtx.m_pFilters depends from tCtx.m_dUserVals here.
			tCtx.m_dUserVals.Add ( pUservar );
			tUservar = *pFilterSettings;
			tUservar.m_eType = SPH_FILTER_VALUES;
			tUservar.SetExternalValues ( pUservar->Begin(), pUservar->GetLength() );
			pFilterSettings = &tUservar;
		}

		return sphCreateFilter ( *pFilterSettings, tCtx, sError, sWarning );
	}

	assert ( pCur->m_iLeft!=-1 && pCur->m_iRight!=-1 );
	ISphFilter * pLeft = CreateFilterNode ( tCtx, pCur->m_iLeft, sError, sWarning, bHasWeight );
	ISphFilter * pRight = CreateFilterNode ( tCtx, pCur->m_iRight, sError, sWarning, bHasWeight );

	if ( !pLeft || !pRight )
	{
		SafeDelete ( pLeft );
		SafeDelete ( pRight );
		return nullptr;
	}

	if ( pCur->m_bOr )
		return new Filter_Or ( pLeft, pRight );

	return new Filter_And2 ( pLeft, pRight );
}


static bool CreateFilterTree ( CreateFilterContext_t & tCtx, CSphString & sError, CSphString & sWarning )
{
	bool bWeight = false;
	ISphFilter * pFilter = CreateFilterNode ( tCtx, tCtx.m_pFilterTree->GetLength() - 1, sError, sWarning, bWeight );
	if ( !pFilter )
		return false;

	// weight filter phase only on match path
	if ( bWeight && tCtx.m_bScan )
		tCtx.m_pWeightFilter = pFilter;
	else
		tCtx.m_pFilter = pFilter;

	return true;
}


struct FilterInfo_t
{
	ISphFilter *	m_pFilter {nullptr};
	int64_t			m_iSelectivity {INT64_MAX};
	int				m_iOrder {0};

	static bool IsLess ( const FilterInfo_t & tA, const FilterInfo_t & tB )
	{
		if ( tA.m_iSelectivity==tB.m_iSelectivity )
			return tA.m_iOrder<tB.m_iOrder;

		return tA.m_iSelectivity < tB.m_iSelectivity;
	}
};


static ISphFilter * ReorderAndCombine ( CSphVector<FilterInfo_t> & dFilters )
{
	ISphFilter * pCombinedFilter = nullptr;

	dFilters.Sort ( FilterInfo_t() );
	for ( const auto & i : dFilters )
		pCombinedFilter = sphJoinFilters ( pCombinedFilter, i.m_pFilter );

	return pCombinedFilter;
}

static int g_iFilterStackSize = 200;

void SetFilterStackItemSize ( int iSize )
{
	if ( iSize>g_iFilterStackSize )
		g_iFilterStackSize = iSize;
}

bool sphCreateFilters ( CreateFilterContext_t & tCtx, CSphString & sError, CSphString & sWarning )
{
	if ( !tCtx.m_pFilters || !tCtx.m_pFilters->GetLength() )
		return true;

	if ( tCtx.m_pFilterTree && tCtx.m_pFilterTree->GetLength() )
	{
		const int TREE_SIZE_THRESH = 200;
		const StackSizeTuplet_t tFilterStack = { g_iFilterStackSize, g_iFilterStackSize }; // fixme! tune eval calc
		int iStackNeeded = -1;
		if ( !EvalStackForTree ( *tCtx.m_pFilterTree, tCtx.m_pFilterTree->GetLength()-1, tFilterStack,
						   TREE_SIZE_THRESH, iStackNeeded, "filters", sError ) )
			return false;

		bool bOk = false;
		Threads::CoContinue ( iStackNeeded, [&] { bOk = CreateFilterTree ( tCtx, sError, sWarning ); } );
		return bOk;
	}

	assert ( tCtx.m_pSchema );
	CSphVector<FilterInfo_t> dFilters, dWeightFilters;

	bool bSingleFilter = tCtx.m_pFilters->GetLength()==1;

	ARRAY_FOREACH ( i, (*tCtx.m_pFilters) )
	{
		const CSphFilterSettings * pFilterSettings = tCtx.m_pFilters->Begin() + i;
		if ( pFilterSettings->m_sAttrName.IsEmpty() )
			continue;

		bool bWeight = IsWeightColumn ( pFilterSettings->m_sAttrName, *tCtx.m_pSchema );
		if ( tCtx.m_bScan && bWeight )
			continue; // @weight is not available in fullscan mode

		// bind user variable local to that daemon
		CSphFilterSettings tUservar;
		if ( pFilterSettings->m_eType==SPH_FILTER_USERVAR )
		{
			const CSphString * sVar = pFilterSettings->m_dStrings.GetLength()==1 ? pFilterSettings->m_dStrings.Begin() : nullptr;
			if ( !UservarsAvailable() || !sVar )
			{
				sError = "no global variables found";
				return false;
			}

			const UservarIntSet_c pUservar = Uservars ( *sVar );
			if ( !pUservar )
			{
				sError.SetSprintf ( "undefined global variable '%s'", sVar->cstr() );
				return false;
			}

			tCtx.m_dUserVals.Add ( pUservar );
			tUservar = *pFilterSettings;
			tUservar.m_eType = SPH_FILTER_VALUES;
			tUservar.SetExternalValues ( pUservar->Begin(), pUservar->GetLength() );
			pFilterSettings = &tUservar;
		}

		ISphFilter * pFilter = sphCreateFilter ( *pFilterSettings, tCtx, sError, sWarning );
		if ( !pFilter )
			return false;

		FilterInfo_t * pFilterInfo;
		if ( bWeight )
			pFilterInfo = &dWeightFilters.Add();
		else
			pFilterInfo = &dFilters.Add();

		pFilterInfo->m_pFilter = pFilter;
		pFilterInfo->m_iOrder = i;

		// no need to calculate if we only have one filter
		if ( !bSingleFilter )
			pFilterInfo->m_iSelectivity = EstimateFilterSelectivity ( *pFilterSettings, tCtx.m_pHistograms );
	}

	// sort by selectivity, preserve order if equal
	tCtx.m_pFilter = ReorderAndCombine ( dFilters );
	tCtx.m_pWeightFilter = ReorderAndCombine ( dWeightFilters );

	return true;
}


void FormatFilterQL ( const CSphFilterSettings & f, StringBuilder_c & tBuf, int iCompactIN )
{
	ScopedComma_c sEmpty (tBuf,nullptr); // disable outer scope separator
	switch ( f.m_eType )
	{
		case SPH_FILTER_VALUES:
			//tBuf << " " << f.m_sAttrName;
			tBuf << f.m_sAttrName;
			if ( f.m_dValues.GetLength()==1 )
				tBuf.Sprintf ( ( f.m_bExclude ? "!=%l" : "=%l" ), (int64_t)f.m_dValues[0] );
			else
			{
				ScopedComma_c tInComma ( tBuf, ",", ( f.m_bExclude ? " NOT IN (" : " IN (" ),")");
				if ( iCompactIN>0 && ( iCompactIN+1<f.m_dValues.GetLength() ) )
				{
					// for really long IN-lists optionally format them as N,N,N,N,...N,N,N, with ellipsis inside.
					int iLimit = Max ( iCompactIN-3, 3 );
					for ( int j=0; j<iLimit; ++j )
						tBuf.Sprintf ( "%l", (int64_t)f.m_dValues[j] );
					iLimit = f.m_dValues.GetLength();
					ScopedComma_c tElipsis ( tBuf, ",","...");
					for ( int j=iLimit-3; j<iLimit; ++j )
						tBuf.Sprintf ( "%l", (int64_t)f.m_dValues[j] );
				} else
					for ( int64_t iValue : f.m_dValues )
						tBuf.Sprintf ( "%l", iValue );
			}
			break;

		case SPH_FILTER_RANGE:
			if ( f.m_iMinValue==int64_t(INT64_MIN) || ( f.m_iMinValue==0 && f.m_sAttrName=="@id" ) )
			{
				// no min, thus (attr<maxval)
				const char * sOps[2][2] = { { "<", "<=" }, { ">=", ">" } };
				tBuf.Sprintf ( "%s%s%l", f.m_sAttrName.cstr(), sOps [ f.m_bExclude ][ f.m_bHasEqualMax ], f.m_iMaxValue );
			} else if ( f.m_iMaxValue==INT64_MAX || ( f.m_iMaxValue==-1 && f.m_sAttrName=="@id" ) )
			{
				// mo max, thus (attr>minval)
				const char * sOps[2][2] = { { ">", ">=" }, { "<", "<=" } };
				tBuf.Sprintf ( "%s%s%l", f.m_sAttrName.cstr(), sOps [ f.m_bExclude ][ f.m_bHasEqualMin ], f.m_iMinValue );
			} else
			{
				if ( f.m_bHasEqualMin!=f.m_bHasEqualMax)
				{
					const char * sOps[2]= { "<", "<=" };
					const char * sFmt = f.m_bExclude ? "NOT %l%s%s%s%l" : "%l%s%s%s%l";
					tBuf.Sprintf ( sFmt, f.m_iMinValue, sOps[f.m_bHasEqualMin], f.m_sAttrName.cstr(), sOps[f.m_bHasEqualMax], f.m_iMaxValue );
				} else
				{
					const char * sFmt = f.m_bExclude ? "%s NOT BETWEEN %l AND %l" : "%s BETWEEN %l AND %l";
					tBuf.Sprintf ( sFmt, f.m_sAttrName.cstr(), f.m_iMinValue + !f.m_bHasEqualMin, f.m_iMaxValue - !f.m_bHasEqualMax );
				}
			}
			break;

		case SPH_FILTER_FLOATRANGE:
			if ( f.m_fMinValue==-FLT_MAX )
			{
				// no min, thus (attr<maxval)
				const char * sOps[2][2] = { { "<", "<=" }, { ">=", ">" } };
				tBuf.Sprintf ( "%s%s%f", f.m_sAttrName.cstr(),
					sOps [ f.m_bExclude ][ f.m_bHasEqualMax ], f.m_fMaxValue );
			} else if ( f.m_fMaxValue==FLT_MAX )
			{
				// mo max, thus (attr>minval)
				const char * sOps[2][2] = { { ">", ">=" }, { "<", "<=" } };
				tBuf.Sprintf ( "%s%s%f", f.m_sAttrName.cstr(),
					sOps [ f.m_bExclude ][ f.m_bHasEqualMin ], f.m_fMinValue );
			} else
			{
				if ( f.m_bHasEqualMin!=f.m_bHasEqualMax)
				{
					const char * sOps[2]= { "<", "<=" };
					tBuf.Sprintf ( "%s%f%s%s%s%f", f.m_bExclude ? "NOT ": "", f.m_fMinValue, sOps[f.m_bHasEqualMin], f.m_sAttrName.cstr(), sOps[f.m_bHasEqualMax], f.m_fMaxValue );
				} else // FIXME? need we handle m_bHasEqual here?					
					tBuf.Sprintf ( "%s%s BETWEEN %f AND %f", f.m_sAttrName.cstr(), f.m_bExclude ? " NOT" : "",	f.m_fMinValue, f.m_fMaxValue );
			}
			break;

		case SPH_FILTER_USERVAR:
		case SPH_FILTER_STRING:
			tBuf.Sprintf ( "%s%s'%s'", f.m_sAttrName.cstr(), ( f.m_bExclude ? "!=" : "=" ), ( f.m_dStrings.GetLength()==1 ? f.m_dStrings[0].cstr() : "" ) );
			break;

		case SPH_FILTER_NULL:
			tBuf << f.m_sAttrName << ( f.m_bIsNull ? " IS NULL" : " IS NOT NULL" );
			break;

		case SPH_FILTER_STRING_LIST:
			tBuf << f.m_sAttrName;// << " IN (";
			if ( f.m_bExclude )
				tBuf << " NOT";
			if ( f.m_eMvaFunc==SPH_MVAFUNC_ANY )
				tBuf.StartBlock ( ", '", " ANY ('", "')" );
			else if ( f.m_eMvaFunc==SPH_MVAFUNC_ALL )
				tBuf.StartBlock ( ", '", " ALL ('", "')" );
			else
				tBuf.StartBlock ( ", '", " IN ('", "')" );
			for ( const auto &sString : f.m_dStrings )
				tBuf << sString;

			tBuf.FinishBlock ();
			break;

		case SPH_FILTER_EXPRESSION:
			tBuf << f.m_sAttrName;
			break;

		default:
			tBuf += "1 /""* oops, unknown filter type *""/";
			break;
	}
}


static void FormatTreeItem ( StringBuilder_c & tBuf, const FilterTreeItem_t & tItem, const CSphVector<CSphFilterSettings> & dFilters, int iCompactIN )
{
	if ( tItem.m_iFilterItem!=-1 )
		FormatFilterQL ( dFilters[tItem.m_iFilterItem], tBuf, iCompactIN );
	else
		tBuf << ( tItem.m_bOr ? " OR " : " AND " );
}


static CSphString LogFilterTree ( int iStartItem, const CSphVector<FilterTreeItem_t> & dTree, const CSphVector<CSphFilterSettings> & dFilters, int iCompactIN )
{
	struct LogTreeNode_t
	{
		int m_iNode;
		int m_iLeftCount;
		int m_iRightCount;
	};

	CSphVector<LogTreeNode_t> dNodes;
	StringBuilder_c tBuf;
	dNodes.Reserve ( dTree.GetLength() );
	LogTreeNode_t tCurNode = { iStartItem, 0, 0 };
	while ( tCurNode.m_iNode!=-1 || dNodes.GetLength() )
	{
		if ( tCurNode.m_iNode!=-1 )
		{
			dNodes.Add(tCurNode);

			bool bTop = tCurNode.m_iNode==iStartItem;
			tCurNode.m_iNode = dTree[tCurNode.m_iNode].m_iLeft;
			if ( !bTop )
				tCurNode.m_iLeftCount++;
			tCurNode.m_iRightCount = 0;
		}
		else
		{
			tCurNode = dNodes.Pop();
			const FilterTreeItem_t & tItem = dTree[tCurNode.m_iNode];

			if ( tItem.m_iLeft==-1 )
			{
				for ( int i = 0; i < tCurNode.m_iLeftCount; i++ )
					tBuf << "(";
			}

			FormatTreeItem ( tBuf, tItem, dFilters, iCompactIN );

			if ( tItem.m_iRight==-1 )
			{
				for ( int i = 0; i < tCurNode.m_iRightCount; i++ )
					tBuf << ")";
			}

			bool bTop = tCurNode.m_iNode==iStartItem;
			tCurNode.m_iNode = tItem.m_iRight;
			tCurNode.m_iLeftCount = 0;
			if ( !bTop )
				tCurNode.m_iRightCount++;
		}
	}

	return tBuf.cstr();
}


void FormatFiltersQL ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> & dFilterTree, StringBuilder_c & tBuf, int iCompactIN )
{
	if ( dFilterTree.IsEmpty() )
	{
		ScopedComma_c sAND ( tBuf, " AND " );
		for ( const auto& dFilter : dFilters )
			FormatFilterQL ( dFilter, tBuf, iCompactIN );
	}
	else
		tBuf << LogFilterTree ( dFilterTree.GetLength() - 1, dFilterTree, dFilters, iCompactIN );
}


class PercolateFilterValues_c final : public PercolateFilter_i
{
public:
	explicit PercolateFilterValues_c ( const CSphFilterSettings & tUID )
		: m_pValues ( tUID.GetValueArray() )
		, m_iCount ( tUID.GetNumValues() )
	{}

	bool Eval ( SphAttr_t uUID ) final
	{
		if ( !m_pValues || !m_iCount )
			return true;

		return ( sphBinarySearch ( m_pValues, m_pValues + m_iCount - 1, SphIdentityFunctor_T<SphAttr_t>(), uUID )!=nullptr );
	}

private:
	const SphAttr_t * m_pValues;
	int m_iCount;
};

template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT >
class PercolateFilterRange_c final : public PercolateFilter_i
{
public:
	PercolateFilterRange_c () = default;

	bool Eval ( SphAttr_t uUID ) final
	{
		return EvalRange<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT> ( uUID, m_iMinValue, m_iMaxValue );
	}

	void SetRange ( SphAttr_t tMin, SphAttr_t tMax ) final
	{
		m_iMinValue = tMin;
		m_iMaxValue = tMax;
	}

	SphAttr_t m_iMinValue = 0;
	SphAttr_t m_iMaxValue = 0;
};

static PercolateFilter_i * CreatePercolateRangeFilter ( const CSphFilterSettings & tOpt )
{
	CREATE_RANGE_FILTER_WITH_OPEN ( PercolateFilterRange_c, tOpt );
}

PercolateFilter_i * CreatePercolateFilter ( const CSphFilterSettings * pUID )
{
	if ( !pUID )
		return nullptr;

	const CSphFilterSettings & tOpt = *pUID;

	if ( pUID->m_eType==SPH_FILTER_VALUES )
		return new PercolateFilterValues_c ( tOpt );

	PercolateFilter_i * pFilter = CreatePercolateRangeFilter ( tOpt );
	pFilter->SetRange ( tOpt.m_iMinValue, tOpt.m_iMaxValue );
	return pFilter;
}
