//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxfilter_
#define _sphinxfilter_

#include "columnarlib.h"
#include "sphinx.h"

class ISphFilter : public columnar::BlockTester_i
{
public:
				ISphFilter() = default;

	virtual void SetLocator ( const CSphAttrLocator & ) {}
	virtual void SetRange ( SphAttr_t, SphAttr_t ) {}
	virtual void SetRangeFloat ( float, float ) {}
	virtual void SetValues ( const VecTraits_T<SphAttr_t>& ) {}
	virtual void SetBlobStorage ( const BYTE * ) {}

	virtual void SetColumnar ( const columnar::Columnar_i * ) {}

	/// similar to EvalBlock w/pMinDocinfo and pMaxDocinfo, but for filter expressions
	/// their results are not stored into matches, so no docinfo here
	bool Test ( const columnar::MinMaxVec_t & dMinMax ) const override
	{
		// if filter does not implement block-level evaluation we assume the block will pass
		return true;
	}

	virtual void SetRefString ( const CSphString * , int ) {}
	virtual std::unique_ptr<ISphFilter> Optimize() { return std::unique_ptr<ISphFilter>(this); }

	/// evaluate filter for a given match
	/// returns true if match satisfies the filter critertia (i.e. in range, found in values list etc)
	virtual bool Eval ( const CSphMatch & tMatch ) const = 0;

	/// evaluate filter for a given block
	/// args are pMinDocinfo and pMaxDocinfo
	/// returns false if no document in block can possibly pass through the filter
	virtual bool EvalBlock ( const DWORD *, const DWORD * ) const
	{
		// if filter does not implement block-level evaluation we assume the block will pass
		return true;
	}

	/// returns true if the filter can handle exclude flag in settings
	/// otherwise a NOT filter will be spawned on top of this filter
	virtual bool CanExclude() const { return false; }
	virtual std::unique_ptr<ISphFilter> Join ( std::unique_ptr<ISphFilter> pFilter );
};

// fwd
using UservarIntSetValues_c = CSphVector<SphAttr_t>;
using UservarIntSet_c = SharedPtr_t<UservarIntSetValues_c>;
class HistogramContainer_c;

struct CreateFilterContext_t
{
	const VecTraits_T<CSphFilterSettings> * m_pFilters = nullptr;
	const VecTraits_T<FilterTreeItem_t> * m_pFilterTree = nullptr;

	const ISphSchema *			m_pSchema = nullptr;
	const BYTE *				m_pBlobPool = nullptr;
	const columnar::Columnar_i * m_pColumnar = nullptr;

	ESphCollation				m_eCollation = SPH_COLLATION_DEFAULT;
	bool						m_bScan = false;

	std::unique_ptr<ISphFilter>	m_pFilter;
	std::unique_ptr<ISphFilter>	m_pWeightFilter;
	CSphVector<UservarIntSet_c>	m_dUserVals;

	const HistogramContainer_c * m_pHistograms = nullptr;
	int64_t						m_iTotalDocs = 0;

	CreateFilterContext_t ( const ISphSchema * pSchema=nullptr )
		: m_pSchema ( pSchema ) {}
};

std::unique_ptr<ISphFilter> sphCreateFilter ( const CSphFilterSettings &tSettings, const CreateFilterContext_t &tCtx, CSphString &sError, CSphString &sWarning);
std::unique_ptr<ISphFilter> sphCreateAggrFilter ( const CSphFilterSettings * pSettings, const CSphString & sAttrName, const ISphSchema & tSchema, CSphString & sError );
std::unique_ptr<ISphFilter> sphJoinFilters ( std::unique_ptr<ISphFilter>, std::unique_ptr<ISphFilter> );

bool sphCreateFilters ( CreateFilterContext_t & tCtx, CSphString & sError, CSphString & sWarning );

void FormatFilterQL ( const CSphFilterSettings & tFilter, StringBuilder_c & tBuf, int iCompactIN );
void FormatFiltersQL ( const VecTraits_T<CSphFilterSettings> & dFilters, const VecTraits_T<FilterTreeItem_t> & dFilterTree, StringBuilder_c & tBuf, int iCompactIN=5 );
void FixupFilterSettings ( const CSphFilterSettings & tSettings, ESphAttr eAttrType, CommonFilterSettings_t & tFixedSettings );

void OptimizeFilters ( CSphVector<CSphFilterSettings> & dFilters );

CSphString FilterType2Str ( ESphFilter eFilterType );

void SetFilterStackItemSize ( int iSize );

// fwd

template<bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT = false, bool OPEN_RIGHT = false, typename T = SphAttr_t>
inline bool EvalRange ( T tValue, T tMin, T tMax )
{
	if_const ( OPEN_LEFT )
		return HAS_EQUAL_MAX ? ( tValue<=tMax ) : ( tValue<tMax );

	if_const ( OPEN_RIGHT )
		return  HAS_EQUAL_MIN ? ( tValue>=tMin ) : ( tValue>tMin );

	auto bMinOk = HAS_EQUAL_MIN ? ( tValue>=tMin ) : ( tValue>tMin );
	auto bMaxOk = HAS_EQUAL_MAX ? ( tValue<=tMax ) : ( tValue<tMax );

	return bMinOk && bMaxOk;
}

template<bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT = false, bool OPEN_RIGHT = false, typename T = SphAttr_t>
inline bool EvalBlockRangeAny ( T tMin1, T tMax1, T tMin2, T tMax2 )
{
	if_const ( OPEN_LEFT )
		return HAS_EQUAL_MAX ? ( tMin1<=tMax2 ) : ( tMin1<tMax2 );

	if_const ( OPEN_RIGHT )
		return HAS_EQUAL_MIN ? ( tMax1>=tMin2 ) : ( tMax1>tMin2 );

	auto bMinOk = HAS_EQUAL_MIN ? ( tMin1<=tMax2 ) : ( tMin1<tMax2 );
	auto bMaxOk = HAS_EQUAL_MAX ? ( tMax1>=tMin2 ) : ( tMax1>tMin2 );

	return bMinOk && bMaxOk;
}

template < typename T >
inline bool MvaEval_Any ( const VecTraits_T<T> & dMvas, const VecTraits_T<const SphAttr_t> & dFilters )
{
	if ( dMvas.IsEmpty() || dFilters.IsEmpty() )
		return false;

	const T * L = dMvas.begin();

	for ( const auto & tFilter : dFilters )
	{
		const T * R = &dMvas.Last();
		while ( L<=R )
		{
			const T * pVal = L + (R - L) / 2;
			T iValue = sphUnalignedRead ( *pVal );
			if ( tFilter > iValue )
				L = pVal + 1;
			else if ( tFilter < iValue )
				R = pVal - 1;
			else
				return true;
		}
	}

	return false;
}

template < typename T >
inline bool MvaEval_All ( const VecTraits_T<T> & dMvas, const VecTraits_T<const SphAttr_t> & dFilters )
{
	if ( dMvas.IsEmpty() || dFilters.IsEmpty() )
		return false;

	for ( const T & tValue : dMvas )
	{
		const SphAttr_t iCheck = sphUnalignedRead(tValue);
		if ( !dFilters.BinarySearch(iCheck) )
			return false;
	}

	return true;
}

template <typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX>
inline bool MvaEval_RangeAny ( const T * pMva, int nValues, SphAttr_t m_iMinValue, SphAttr_t m_iMaxValue )
{
	if ( !pMva )
		return false;

	const T * pEnd = pMva+nValues;
	const T * L = pMva;
	const T * R = pEnd - 1;

	while ( L<=R )
	{
		const T * pVal = L + (R - L) / 2;
		T iMva = sphUnalignedRead ( *pVal );

		if ( m_iMinValue>iMva )
			L = pVal + 1;
		else if ( m_iMinValue<iMva )
			R = pVal - 1;
		else
			return ( HAS_EQUAL_MIN || pVal+1<pEnd );
	}
	if ( L==pEnd )
		return false;

	T iMvaL = sphUnalignedRead ( *L );
	if_const ( HAS_EQUAL_MAX )
		return iMvaL<=m_iMaxValue;
	else
		return iMvaL<m_iMaxValue;
}

template<bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, typename T = SphAttr_t>
inline bool EvalBlockRangeAll ( T tMin1, T tMax1, T tMin2, T tMax2 )
{
	auto bMinOk = HAS_EQUAL_MIN ? ( tMin1>=tMin2 ) : ( tMin1>tMin2 );
	auto bMaxOk = HAS_EQUAL_MAX ? ( tMax1<=tMax2 ) : ( tMax1<tMax2 );

	return bMinOk && bMaxOk;
}

template <typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX>
inline bool MvaEval_RangeAll ( const T * pMva, int nValues, SphAttr_t m_iMinValue, SphAttr_t m_iMaxValue )
{
	if ( !pMva )
		return false;

	const T * L = pMva;
	const T * R = pMva+nValues-1;
	return EvalBlockRangeAll<HAS_EQUAL_MIN,HAS_EQUAL_MAX> ( *L, *R, (T)m_iMinValue, (T)m_iMaxValue );
}


struct MvaEvalAll_c
{
	template<typename T>
	static inline bool Eval ( const VecTraits_T<T> & dMvas, const VecTraits_T<SphAttr_t> & dFilters )
	{
		return MvaEval_All ( dMvas, dFilters );
	}

	template<typename T>
	static inline bool Eval ( const VecTraits_T<T> & dMvas, SphAttr_t tValue )
	{
		for ( T tMVA : dMvas )
			if ( tMVA!=tValue )
				return false;

		return true;
	}

	template<typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
	static inline bool EvalRange ( const VecTraits_T<T> & dMvas, SphAttr_t tMin, SphAttr_t tMax )
	{
		return MvaEval_RangeAll<T, HAS_EQUAL_MIN, HAS_EQUAL_MAX> ( dMvas.Begin(), dMvas.GetLength(), tMin, tMax );
	}

	static inline bool EvalBlock ( const VecTraits_T<SphAttr_t> & dFilters, SphAttr_t tBlockMin, SphAttr_t tBlockMax )
	{
		return dFilters.all_of ( [&]( const SphAttr_t & tFilter ) { return tFilter>=tBlockMin && tFilter<=tBlockMax; } );
	}
};


struct MvaEvalAny_c
{
	template<typename T>
	static inline bool Eval ( const VecTraits_T<T> & dMvas, const VecTraits_T<const SphAttr_t> & dFilters )
	{
		return MvaEval_Any ( dMvas, dFilters );
	}

	template<typename T>
	static inline bool Eval ( const VecTraits_T<T> & dMvas, SphAttr_t tValue )
	{
		return !!dMvas.BinarySearch((T)tValue);
	}

	template<typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
	static inline bool EvalRange ( const VecTraits_T<T> & dMvas, SphAttr_t tMin, SphAttr_t tMax )
	{
		return MvaEval_RangeAny<T, HAS_EQUAL_MIN, HAS_EQUAL_MAX> ( dMvas.Begin(), dMvas.GetLength(), tMin, tMax );
	}

	static inline bool EvalBlock ( const VecTraits_T<SphAttr_t> & dFilters, SphAttr_t tBlockMin, SphAttr_t tBlockMax )
	{
		return dFilters.any_of ( [&]( const SphAttr_t & tFilter ) { return tFilter>=tBlockMin && tFilter<=tBlockMax; } );
	}
};


struct RowIdBoundaries_t
{
	RowID_t m_tMinRowID = 0;
	RowID_t m_tMaxRowID = INVALID_ROWID;
};

RowIdBoundaries_t GetFilterRowIdBoundaries ( const CSphFilterSettings & tFilter, RowID_t tTotalDocs );

bool FixupFilterSettings ( const CSphFilterSettings & tSettings, CommonFilterSettings_t & tFixedSettings, const CreateFilterContext_t & tCtx, const CSphString & sAttrName, CSphString & sError );

#endif // _sphinxfilter_
