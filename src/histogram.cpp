//
// Copyright (c) 2018-2021, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "histogram.h"

#include "attribute.h"
#include "sphinxint.h"
#include "conversion.h"
#include <math.h>

template <typename T>
struct HSBucket_T
{
    T	m_tCentroid;
    int m_iCount;

	HSBucket_T Merge ( const HSBucket_T & tB )
	{
		T tDeltaCentroid = tB.m_tCentroid - m_tCentroid;

		HSBucket_T<T> tVal;
		tVal.m_tCentroid = m_tCentroid + tDeltaCentroid * tB.m_iCount / ( m_iCount + tB.m_iCount );
		tVal.m_iCount = m_iCount + tB.m_iCount;
		return tVal;
	}

	bool IsCenterEq ( const HSBucket_T & tB ) const
	{
		return ( m_tCentroid==tB.m_tCentroid );
	}
};

template<> bool HSBucket_T<float>::IsCenterEq ( const HSBucket_T & tB ) const
{
	return ( fabs ( m_tCentroid - tB.m_tCentroid )<=FLT_EPSILON );
}

// operators for FindSpan

template <typename T>
bool operator < ( const HSBucket_T<T> & a, T b )
{
	return ( a.m_tCentroid<b );
}

template <typename T>
bool operator < ( T a, const HSBucket_T<T> & b )
{
	return ( a<b.m_tCentroid);
}

template <typename T>
bool operator == ( const HSBucket_T<T> & a, T b )
{
	return ( a.m_tCentroid==b );
}

template<> bool operator == ( const HSBucket_T<float> & a, float b )
{
	return ( fabs ( a.m_tCentroid - b )<=FLT_EPSILON );
}


template <typename T>
struct HSQueueItem_T
{
	T	m_tDelta { 0 };
	int m_iId { 0 };

	HSQueueItem_T() = default;
	HSQueueItem_T ( T tDelta, int iId )
		: m_tDelta ( tDelta )
		, m_iId ( iId )
	{}

	static inline bool IsLess ( const HSQueueItem_T & tA, const HSQueueItem_T & tB )
	{
		if ( tA.m_tDelta==tB.m_tDelta )
			return ( tA.m_iId<tB.m_iId );

		return ( tA.m_tDelta<tB.m_tDelta );
	}
};

template<> bool HSQueueItem_T<float>::IsLess ( const HSQueueItem_T & tA, const HSQueueItem_T & tB )
{
	if ( fabs ( tA.m_tDelta-tB.m_tDelta )<=FLT_EPSILON )
		return ( tA.m_iId<tB.m_iId );

	return ( tA.m_tDelta<tB.m_tDelta );
}

template <typename T>
struct HSListItem_T
{
	// list items
	HSListItem_T *	m_pPrev { nullptr };
	HSListItem_T *	m_pNext { nullptr };
	bool			m_bAlive { true };
	HSBucket_T<T> *	m_pItem { nullptr };
	int				m_iId { 0 };

	static T Delta ( const HSListItem_T & tCur )
	{
		return ( tCur.m_pNext->m_pItem->m_tCentroid - tCur.m_pItem->m_tCentroid );
	}

	static void Delete ( HSListItem_T * pNode )
	{
		if ( !pNode )
			return;

		if ( pNode->m_pNext )
			pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		if ( pNode->m_pPrev )
			pNode->m_pPrev->m_pNext = pNode->m_pNext;

		pNode->m_pNext = nullptr;
		pNode->m_pPrev = nullptr;

		pNode->m_bAlive = false;
	}
};

struct HSBucketTrait_t
{
	int m_iBucket = 0;
	int m_iCount = 0;

	HSBucketTrait_t() = default;
	HSBucketTrait_t ( int iBucket, int iCount )
		: m_iBucket ( iBucket )
		, m_iCount ( iCount )
	{}
};

//////////////////////////////////////////////////////////////////////////

template <typename T>
class HistogramStreamed_T : public Histogram_i
{
public:
						HistogramStreamed_T ( CSphString sAttr, int iBins );

    void				Insert ( SphAttr_t tAttrVal ) override;
    void				Finalize() override;
	bool				Save ( CSphWriter & tWriter ) const override;
	bool				Load ( CSphReader & tReader, CSphString & sError ) override;
	DWORD				GetNumValues() const override { return m_uValues; }
	HistogramType_e		GetType() const override { return TYPE; }
	const CSphString &	GetAttrName() const override { return m_sAttr; }
	void				Delete ( SphAttr_t tAttr ) override;
	void				UpdateCounter ( SphAttr_t tAttr ) override;
	bool				IsOutdated() const override;
	void				Dump ( StringBuilder_c & tOut ) const override;

	bool				EstimateRsetSize ( const CSphFilterSettings & tFilter, int64_t & iEstimate ) const override;

private:
	static const HistogramType_e TYPE;
	static const T		MIN_BY_TYPE;
	static const T		MAX_BY_TYPE;
	static const int	m_iKbufferFactor = 2;
	static const DWORD	VERSION = 3;

	CSphString			m_sAttr;
	T					m_tMinValue;
	T					m_tMaxValue;

	int					m_iMaxBins = 0;
	int					m_iSize = 0;
	DWORD				m_uValues = 0;
	DWORD				m_uOutdated = 0;
	CSphFixedVector<HSBucket_T<T>> m_dBuckets { 0 };

	bool				m_bUpdateMode = false;

	DWORD				GetLength() const { return Min ( m_iSize, m_iMaxBins ); }
	void				DumpValue ( const HSBucket_T < T > & tVal, StringBuilder_c & tBuf ) const;
    void				Push ( T tValue, int iCount );
	void				Aggregate ( int iBins );
	int					GetBucket ( T tValue, bool bCounterLess ) const;
	int					LerpCounter ( int iBucket, T tVal ) const;
	HSBucketTrait_t		GetBucket ( T tValue ) const;

	DWORD				EstimateValues ( const SphAttr_t * pValues, int nValues ) const;
	DWORD				EstimateRangeFilter ( bool bExclude, bool bHasEqualMin, bool bHasEqualMax, bool bOpenLeft, bool bOpenRight, T tMinValue, T tMaxValue ) const;
	T					Saturate ( T tVal ) const;
	DWORD				EstimateInterval ( T tMin, T tMax, bool bHasEqualMin, bool bHasEqualMax, bool bOpenLeft, bool bOpenRight ) const;
	bool				IsOutdated ( SphAttr_t tAttr ) const;
	void				UpdateMinMax();
};


template<typename T>
HistogramStreamed_T<T>::HistogramStreamed_T ( CSphString sAttr, int iBins )
	: m_sAttr ( std::move(sAttr) )
	, m_iMaxBins ( iBins )
	, m_dBuckets ( iBins * m_iKbufferFactor )
{
	m_dBuckets.ZeroVec();
	UpdateMinMax();
}

template<typename T>
void HistogramStreamed_T<T>::Insert ( SphAttr_t tAttrVal )
{
	assert ( !m_bUpdateMode );
	Push ( ConvertType<T>(tAttrVal), 1 );
}

template<typename T>
void HistogramStreamed_T<T>::Finalize()
{
	Aggregate(m_iMaxBins);

	// fold close values like uniq but with merge operations
	if ( m_iSize )
	{
		int iSrc = 1;
		int iDst = 1;
		while ( iSrc<m_iSize )
		{
			if ( m_dBuckets[iDst-1].IsCenterEq ( m_dBuckets[iSrc] ) )
			{
				m_dBuckets[iDst-1] = m_dBuckets[iDst-1].Merge ( m_dBuckets[iSrc] );
				iSrc++;
			} else
			{
				m_dBuckets[iDst] = m_dBuckets[iSrc];
				iDst++;
				iSrc++;
			}
		}
		m_iSize = iDst;
	}

	UpdateMinMax();
}

template<typename T>
bool HistogramStreamed_T<T>::Save ( CSphWriter & tWriter ) const
{
	tWriter.PutDword ( VERSION );
	tWriter.PutBytes ( &m_tMinValue, sizeof(T) );
	tWriter.PutBytes ( &m_tMaxValue, sizeof(T) );
	tWriter.PutDword ( m_iMaxBins );
	tWriter.PutDword ( m_iSize );
	tWriter.PutDword ( m_uValues );
	tWriter.PutDword ( m_uOutdated );
	tWriter.PutBytes ( m_dBuckets.Begin(), m_iSize*sizeof(m_dBuckets[0]) );

	return true;
}

template<typename T>
bool HistogramStreamed_T<T>::Load ( CSphReader & tReader, CSphString & sError )
{
	DWORD uVersion = tReader.GetDword();
	if ( uVersion > VERSION )
	{
		sError.SetSprintf ( "index histogram version (%u) greater than code histogram version (%u)", uVersion, VERSION );
		return false;
	}

	if ( uVersion<=1 )
	{
		sError.SetSprintf ( "non-streamed histograms are no longer supported" );
		return false;
	}

	tReader.GetBytes ( &m_tMinValue, sizeof(T) );
	tReader.GetBytes ( &m_tMaxValue, sizeof(T) );
	m_iMaxBins = tReader.GetDword();
	m_iSize = tReader.GetDword();
	m_uValues = tReader.GetDword();
	m_dBuckets.Reset ( m_iSize );
	if ( uVersion>=3 )
		m_uOutdated = tReader.GetDword();
	tReader.GetBytes ( m_dBuckets.Begin(), m_iSize*sizeof(m_dBuckets[0]) );

	if ( tReader.GetErrorFlag() )
	{
		sError = tReader.GetErrorMessage();
		return false;
	}

	// can not insert values after load - only update existed
	m_bUpdateMode = true;

	return true;
}

template<typename T>
bool HistogramStreamed_T<T>::IsOutdated ( SphAttr_t tAttr ) const
{
	T tVal = ConvertType<T> ( tAttr );
	return ( tVal<m_tMinValue || m_tMaxValue<tVal );
}

template<typename T>
void HistogramStreamed_T<T>::Delete ( SphAttr_t tAttr )
{
	// select bucket with larger counter to decrease
	int iBucket = GetBucket ( tAttr, false );
	assert ( iBucket>=0 && iBucket<m_iSize && m_uValues>0 );
	if ( m_dBuckets[iBucket].m_iCount )
		m_dBuckets[iBucket].m_iCount--;

	m_uValues--;
	if ( IsOutdated ( tAttr ) )
		m_uOutdated--;
}

template<typename T>
void HistogramStreamed_T<T>::UpdateCounter ( SphAttr_t tAttr )
{
	// select bucket with smaller counter to update
	int iBucket = GetBucket ( tAttr, true );
	assert ( iBucket>=0 && iBucket<m_iSize );
	m_dBuckets[iBucket].m_iCount++;
	m_uValues++;
	if ( IsOutdated ( tAttr ) )
		m_uOutdated++;
}

template<typename T>
bool HistogramStreamed_T<T>::IsOutdated() const
{
	if ( !m_uValues )
		return true;
	
	// outdated values should be less than 30% for histogram to estimate properly
	const float	MAX_OUT_OF_RANGE = 0.3f;
	return ( ( (float)m_uOutdated / (float)m_uValues )>=MAX_OUT_OF_RANGE );
}

template<typename T>
void HistogramStreamed_T<T>::Dump ( StringBuilder_c & tOut ) const
{
	StringBuilder_c tBuf ( ";" );
	for ( int i=0; i<m_iSize; i++ )
		DumpValue ( m_dBuckets[i], tBuf );

	tOut.Appendf ( "%s hist-streamed\nvalues:%d\n%s", m_sAttr.cstr(), m_iSize, tBuf.cstr() );
}


template<typename T>
void HistogramStreamed_T<T>::Push ( T tValue, int iCount )
{
	m_dBuckets[m_iSize].m_tCentroid = tValue;
	m_dBuckets[m_iSize].m_iCount = iCount;
	m_iSize++;
	m_uValues++;

	if ( m_iSize>=m_dBuckets.GetLength() )
		Aggregate(m_iMaxBins);
}

template<typename T>
void HistogramStreamed_T<T>::UpdateMinMax()
{
	if ( m_iSize )
	{
		m_tMinValue = m_dBuckets[0].m_tCentroid;
		m_tMaxValue = m_dBuckets[m_iSize-1].m_tCentroid;
	} else
	{
		m_tMinValue = MAX_BY_TYPE;
		m_tMaxValue = MIN_BY_TYPE;
	}
}

template<typename T>
void HistogramStreamed_T<T>::Aggregate ( int iBins )
{
	// order by centers
	if ( m_iSize )
		m_dBuckets.Sort ( bind ( &HSBucket_T<T>::m_tCentroid ), 0, m_iSize-1 );

	int iSize = m_iSize;
	if ( m_iSize<=iBins )
		return;

	using ListItem_t = HSListItem_T < T >;
	CSphFixedVector<ListItem_t> dList ( m_iSize );
	for (int i = 0; i<m_iSize; ++i)
	{
		dList[i].m_pPrev = dList.Begin() + i - 1;
		dList[i].m_pNext = dList.Begin() + i + 1;
		dList[i].m_iId = i;
		dList[i].m_pItem = m_dBuckets.Begin() + i;
	}
	dList[0].m_pPrev = nullptr;
	dList[m_iSize-1].m_pNext = nullptr;

	using QItem_t = HSQueueItem_T<T>;
	CSphQueue<QItem_t, QItem_t> tQueue ( 2 * m_iSize - iBins );
	// do not all last item as it has wrong delta from head to tail
	for (int i = 0; i<m_iSize-1; ++i)
	{
		const ListItem_t & tItem = dList[i];
		Verify ( tQueue.Push ( QItem_t ( ListItem_t::Delta ( tItem ), tItem.m_iId ) ) );
	}

	while ( iSize>iBins && tQueue.GetLength() )
	{
		QItem_t tMin = tQueue.Root();
		tQueue.Pop();
		ListItem_t & tItem = dList[tMin.m_iId];

		// check alive item and next to compare then compare delta from pqueue BUT not from actual entry
		if ( !tItem.m_bAlive || !tItem.m_pNext || !tItem.m_pNext->m_bAlive || tMin.m_tDelta<ListItem_t::Delta ( tItem ) )
			continue;

		*tItem.m_pItem = tItem.m_pItem->Merge ( *tItem.m_pNext->m_pItem );
		ListItem_t::Delete ( tItem.m_pNext );

		// check and add curent -> next due changed delta from current, could be dupe and will be rejected above
		if ( tItem.m_pNext && tItem.m_pNext->m_bAlive )
			Verify ( tQueue.Push ( QItem_t ( ListItem_t::Delta ( tItem ), tMin.m_iId ) ) );

		// check and add prev -> curent due changed delta to current, could be dupe and will be rejected above
		if ( tItem.m_pPrev && tItem.m_pPrev->m_bAlive )
			Verify ( tQueue.Push ( QItem_t ( ListItem_t::Delta ( *tItem.m_pPrev ), tItem.m_pPrev->m_iId ) ) );

		iSize--;
	}

	// copy buckets
	int iSrc = 0;
	int iDst = 0;
	while ( iSrc<m_iSize )
	{
		if ( dList[iSrc].m_bAlive )
		{
			m_dBuckets[iDst] = m_dBuckets[iSrc];
			iDst++;
		}
		iSrc++;
	}
	m_iSize = iSize;
}

template<typename T>
int HistogramStreamed_T<T>::GetBucket ( T tValue, bool bCounterLess ) const
{
	if ( tValue<m_tMinValue )
		return 0;
	if ( tValue>m_tMaxValue )
		return ( m_iSize - 1 );

	// m_dBuckets is larger than m_iSize
	int iBestBucket = FindSpan ( m_dBuckets.Slice ( 0, m_iSize ), tValue );

	assert ( iBestBucket>=0 && iBestBucket<m_iSize );

	if ( iBestBucket<m_iSize-1 )
	{
		const HSBucket_T<T> & tBucketL = m_dBuckets[iBestBucket];
		const HSBucket_T<T> & tBucketR = m_dBuckets[iBestBucket+1];
		T tDistL = tValue - tBucketL.m_tCentroid;
		T tDist = tBucketR.m_tCentroid - tBucketL.m_tCentroid;

		if ( tDistL>( tDist/3 ) && tDistL<( tDist*2/3 ) ) // center case - select bucket with smaller \ larger counter
		{
			if ( bCounterLess )
				iBestBucket = ( tBucketL.m_iCount<tBucketR.m_iCount ? iBestBucket : iBestBucket+1 );
			else
				iBestBucket = ( tBucketL.m_iCount>tBucketR.m_iCount ? iBestBucket : iBestBucket+1 );

		} else // select closest bucket
			iBestBucket = ( tDistL<( tDist-tDistL ) ? iBestBucket : iBestBucket+1 );
	}

	return iBestBucket;

}

template<typename T>
int HistogramStreamed_T<T>::LerpCounter ( int iBucket, T tVal ) const
{
	const HSBucket_T<T> & tBucketL = m_dBuckets[iBucket];
	const HSBucket_T<T> & tBucketR = m_dBuckets[iBucket+1];
	assert ( tBucketL.m_tCentroid<=tVal && tVal<=tBucketR.m_tCentroid );

	T tDistL = tVal - tBucketL.m_tCentroid;
	T tDist = tBucketR.m_tCentroid - tBucketL.m_tCentroid;

	float fLerp = (float)tDistL / (float)tDist;
	assert ( fLerp>=0.0f && fLerp<=1.0f );

	int iCount = int ( fLerp * tBucketL.m_iCount + ( 1.0f - fLerp ) * tBucketR.m_iCount );

	return iCount;
}

template<typename T>
HSBucketTrait_t HistogramStreamed_T<T>::GetBucket ( T tValue ) const
{
	if ( tValue<m_tMinValue )
		return HSBucketTrait_t ( 0, m_dBuckets[0].m_iCount );

	if ( tValue>m_tMaxValue )
		return HSBucketTrait_t (  m_iSize - 1, m_dBuckets[m_iSize-1].m_iCount );

	int iItem = FindSpan ( m_dBuckets.Slice ( 0, m_iSize ), tValue );
	int iCount = 0;

	assert ( iItem>=0 && iItem<m_iSize );

	if ( iItem==m_iSize-1 )
		iCount = m_dBuckets[iItem].m_iCount;
	else
		iCount = LerpCounter ( iItem, tValue );

	return HSBucketTrait_t ( iItem, iCount );
}

template<typename T>
bool HistogramStreamed_T<T>::EstimateRsetSize ( const CSphFilterSettings & tFilter, int64_t & iEstimate ) const
{
	if ( !m_iSize )
		return false;

	iEstimate = GetNumValues();

	CommonFilterSettings_t tFixedSettings = tFilter;
	if ( TYPE==HISTOGRAM_STREAMED_FLOAT )
		FixupFilterSettings ( tFilter, SPH_ATTR_FLOAT, tFixedSettings );

	switch ( tFixedSettings.m_eType )
	{
	case SPH_FILTER_VALUES:
		assert ( TYPE==HISTOGRAM_STREAMED_UINT32 || TYPE==HISTOGRAM_STREAMED_INT64 );

		if ( tFilter.m_bExclude )
			return false;

		iEstimate = EstimateValues ( tFilter.GetValueArray(), tFilter.GetNumValues() );
		return true;

	case SPH_FILTER_RANGE:
		assert ( TYPE==HISTOGRAM_STREAMED_UINT32 || TYPE==HISTOGRAM_STREAMED_INT64 );
		iEstimate = EstimateRangeFilter ( tFilter.m_bExclude, tFilter.m_bHasEqualMin, tFilter.m_bHasEqualMax, tFilter.m_bOpenLeft, tFilter.m_bOpenRight, (T)tFixedSettings.m_iMinValue, (T)tFixedSettings.m_iMaxValue );
		return true;

	case SPH_FILTER_FLOATRANGE:
		assert ( TYPE==HISTOGRAM_STREAMED_FLOAT );
		iEstimate = EstimateRangeFilter ( tFilter.m_bExclude, tFilter.m_bHasEqualMin, tFilter.m_bHasEqualMax, tFilter.m_bOpenLeft, tFilter.m_bOpenRight, (T)tFixedSettings.m_fMinValue, (T)tFixedSettings.m_fMaxValue );
		return true;

	default:
		break;
	}

	return false;
}

template<typename T>
DWORD HistogramStreamed_T<T>::EstimateValues ( const SphAttr_t * pValues, int nValues ) const
{
	DWORD uTotal = 0;
	int iPrevBucket = INT_MIN;
	for ( int i = 0; i < nValues; i++ )
	{
		HSBucketTrait_t tItem = GetBucket ( pValues[i] );
		if ( tItem.m_iBucket!=iPrevBucket )
		{
			uTotal += tItem.m_iCount;
			iPrevBucket = tItem.m_iBucket;
		}
	}

	return uTotal;
}

template<typename T>
DWORD HistogramStreamed_T<T>::EstimateRangeFilter ( bool bExclude, bool bHasEqualMin, bool bHasEqualMax, bool bOpenLeft, bool bOpenRight, T tMinValue, T tMaxValue ) const
{
	if ( !bExclude )
		return EstimateInterval ( tMinValue, tMaxValue, bHasEqualMin, bHasEqualMax, bOpenLeft, bOpenRight );

	assert ( !bOpenLeft || !bOpenRight );

	DWORD uEstimate = 0;
	if ( bOpenRight )
		uEstimate = EstimateInterval ( (T)0, tMinValue, false, !bHasEqualMin, true, false );
	else if ( bOpenLeft )
		uEstimate = EstimateInterval ( tMaxValue, (T)0, !bHasEqualMax, false, false, true );
	else
	{
		uEstimate = EstimateInterval ( (T)0, tMinValue, false, !bHasEqualMin, true, false );
		uEstimate += EstimateInterval ( tMaxValue, (T)0, !bHasEqualMax, false, false, true );
	}

	return uEstimate;
}

template<typename T>
T HistogramStreamed_T<T>::Saturate ( T tVal ) const
{
	if ( tVal<m_tMinValue )
		return m_tMinValue;
	if ( tVal>m_tMaxValue )
		return m_tMaxValue;

	return tVal;
}

template<typename T>
DWORD HistogramStreamed_T<T>::EstimateInterval ( T tMin, T tMax, bool bHasEqualMin, bool bHasEqualMax, bool bOpenLeft, bool bOpenRight ) const
{
	if ( TYPE==HISTOGRAM_STREAMED_UINT32 || TYPE==HISTOGRAM_STREAMED_INT64 )
	{
		if ( !bOpenLeft && !bHasEqualMin && tMin < MAX_BY_TYPE )
			tMin++;

		if ( !bOpenRight && !bHasEqualMax && tMax > MIN_BY_TYPE )
			tMax--;
	}

	tMin = Saturate ( tMin );
	tMax = Saturate ( tMax );

	DWORD uTotal = 0;

	// open left means to process all buckets from start
	int iStartBucket = 0;
	if ( !bOpenLeft )
		iStartBucket = FindSpan ( m_dBuckets.Slice ( 0, m_iSize ), tMin );

	int iChecked = 0;
	for ( int iBucket=iStartBucket+1; iBucket<m_iSize; iBucket++ )
	{
		const HSBucket_T<T> & tBucket = m_dBuckets[iBucket];
		// open right means to process all buckets till end
		if ( !bOpenRight && tBucket.m_tCentroid>tMax )
			break;

		uTotal += tBucket.m_iCount;
		iChecked++;
	}

	if ( !iChecked ) // interval inside single bucket
	{
		uTotal = m_dBuckets[iStartBucket].m_iCount;
		if ( iStartBucket+1<m_iSize )
		{
			DWORD uMinCount = 0;
			DWORD uMaxCount = 0;
			if ( m_dBuckets[iStartBucket].m_tCentroid<tMin && tMin<m_dBuckets[iStartBucket+1].m_tCentroid )
				uMinCount = LerpCounter ( iStartBucket, tMin );
			if ( m_dBuckets[iStartBucket].m_tCentroid<tMax && tMax<m_dBuckets[iStartBucket+1].m_tCentroid )
				uMaxCount = LerpCounter ( iStartBucket, tMax );
			if ( uMinCount || uMaxCount )
				uTotal = Max ( uMinCount, uMaxCount );
		}
	} else // count head bucket interval
	{
		if ( bOpenLeft )
			uTotal += m_dBuckets[iStartBucket].m_iCount;
		else
		{
			int iMinCount = LerpCounter ( iStartBucket, tMin );
			// substract from total range with tMin value and add more preceise counter
			uTotal = uTotal - m_dBuckets[iStartBucket+1].m_iCount / 2 + iMinCount;
		}
	}

	return uTotal;
}

template<> const HistogramType_e HistogramStreamed_T<DWORD>::TYPE = HISTOGRAM_STREAMED_UINT32;
template<> const DWORD HistogramStreamed_T<DWORD>::MIN_BY_TYPE = 0;
template<> const DWORD HistogramStreamed_T<DWORD>::MAX_BY_TYPE = UINT32_MAX;

template<> const HistogramType_e HistogramStreamed_T<int64_t>::TYPE = HISTOGRAM_STREAMED_INT64;
template<> const int64_t HistogramStreamed_T<int64_t>::MIN_BY_TYPE = 0;
template<> const int64_t HistogramStreamed_T<int64_t>::MAX_BY_TYPE = INT64_MAX;

template<> const HistogramType_e HistogramStreamed_T<float>::TYPE = HISTOGRAM_STREAMED_FLOAT;
template<> const float HistogramStreamed_T<float>::MIN_BY_TYPE = FLT_MIN;
template<> const float HistogramStreamed_T<float>::MAX_BY_TYPE = FLT_MAX;

template<> void HistogramStreamed_T<DWORD>::DumpValue ( const HSBucket_T < DWORD > & tVal, StringBuilder_c & tBuf ) const
{
	tBuf.Sprintf ( "%u,%d", tVal.m_tCentroid, tVal.m_iCount );
}

template<> void HistogramStreamed_T<int64_t>::DumpValue ( const HSBucket_T < int64_t > & tVal, StringBuilder_c & tBuf ) const
{
	tBuf.Sprintf ( INT64_FMT ",%d", tVal.m_tCentroid, tVal.m_iCount );
}

template<> void HistogramStreamed_T<float>::DumpValue ( const HSBucket_T < float > & tVal, StringBuilder_c & tBuf ) const
{
	tBuf.Sprintf ( "%.3f,%d", tVal.m_tCentroid, tVal.m_iCount );
}

//////////////////////////////////////////////////////////////////////////

static Histogram_i * CreateHistogram ( const CSphString & sAttr, HistogramType_e eType, int iSize );

HistogramContainer_c::~HistogramContainer_c()
{
	Reset();
}


void HistogramContainer_c::Reset()
{
	m_dHistogramHash.IterateStart();
	while ( m_dHistogramHash.IterateNext() )
		SafeDelete ( m_dHistogramHash.IterateGet() );

	m_dHistogramHash.Reset();
	m_dHistograms.Resize(0);
}


bool HistogramContainer_c::Save ( const CSphString & sFile, CSphString & sError )
{
	CSphWriter tWriter;
	if ( !tWriter.OpenFile ( sFile, sError ) )
		return false;

	tWriter.PutDword ( m_dHistogramHash.GetLength() );

	m_dHistogramHash.IterateStart();
	while ( m_dHistogramHash.IterateNext() )
	{
		Histogram_i * pHistogram = m_dHistogramHash.IterateGet();
		assert ( pHistogram );
		pHistogram->Finalize();
		tWriter.PutString ( pHistogram->GetAttrName() );
		tWriter.PutDword ( pHistogram->GetType() );

		if ( !pHistogram->Save ( tWriter ) )
		{
			sError.SetSprintf ( "error saving histograms to %s", sFile.cstr() );
			return false;
		}
	}

	tWriter.CloseFile();
	if ( tWriter.IsError() )
	{
		sError.SetSprintf ( "error saving histograms to %s", sFile.cstr() );
		return false;
	}

	return true;
}


bool HistogramContainer_c::Load ( const CSphString & sFile, CSphString & sError )
{
	Reset();

	CSphAutoreader tReader;
	if ( !tReader.Open ( sFile, sError ) )
		return false;

	int nHistograms = tReader.GetDword();
	for ( int i = 0; i < nHistograms; i++ )
	{
		CSphString sAttr = tReader.GetString();
		HistogramType_e eType = (HistogramType_e)tReader.GetDword();
		CSphScopedPtr<Histogram_i> pHistogram ( CreateHistogram ( sAttr, eType, 0 ) );
		if ( !pHistogram.Ptr() )
		{
			sError.SetSprintf ( "error loading histograms from %s", sFile.cstr() );
			return false;
		}

		if ( !pHistogram->Load ( tReader, sError ) )
			return false;

		if ( !m_dHistogramHash.Add ( pHistogram.Ptr(), sAttr ) )
		{
			sError.SetSprintf ( "duplicate histograms found in %s", sFile.cstr() );
			return false;
		}

		pHistogram.LeakPtr();
	}

	if ( tReader.GetErrorFlag() )
	{
		sError = tReader.GetErrorMessage();
		return false;
	}

	return true;
}


bool HistogramContainer_c::Add ( Histogram_i * pHistogram )
{
	assert ( pHistogram );
	if ( !m_dHistogramHash.Add ( pHistogram, pHistogram->GetAttrName() ) )
		return false;

	m_dHistograms.Add(pHistogram);
	return true;
}


void HistogramContainer_c::Remove ( const CSphString & sAttr )
{
	Histogram_i * pHistogram = Get(sAttr);
	if ( !pHistogram )
		return;

	SafeDelete ( pHistogram );
	m_dHistogramHash.Delete(sAttr);
}


Histogram_i * HistogramContainer_c::Get ( const CSphString & sAttr ) const
{
	Histogram_i ** ppHistogram = m_dHistogramHash(sAttr);
	return ppHistogram ? *ppHistogram : nullptr;
}


DWORD HistogramContainer_c::GetNumValues() const
{
	// all histograms should have the same amount of values
	m_dHistogramHash.IterateStart();
	if ( !m_dHistogramHash.IterateNext() )
		return 0;

	return m_dHistogramHash.IterateGet()->GetNumValues();
}

//////////////////////////////////////////////////////////////////////////

static bool CanCreateHistogram ( const CSphString sAttrName, ESphAttr eAttrType )
{
	if ( sphIsInternalAttr ( sAttrName ) )
		return false;

	return eAttrType==SPH_ATTR_INTEGER || eAttrType==SPH_ATTR_BIGINT || eAttrType==SPH_ATTR_BOOL || eAttrType==SPH_ATTR_FLOAT || eAttrType==SPH_ATTR_TIMESTAMP;
}


static Histogram_i * CreateHistogram ( const CSphString & sAttr, HistogramType_e eType, int iSize )
{
	const int MAX_BUCKETS = 1024;

	if ( !iSize )
		iSize = MAX_BUCKETS;

	switch ( eType )
	{
		case HISTOGRAM_STREAMED_UINT32:	return new HistogramStreamed_T<DWORD> ( sAttr, iSize );
		case HISTOGRAM_STREAMED_INT64:	return new HistogramStreamed_T<int64_t> ( sAttr, iSize );
		case HISTOGRAM_STREAMED_FLOAT:	return new HistogramStreamed_T<float> ( sAttr, iSize );
		default:						return nullptr;
	}
}


Histogram_i * CreateHistogram ( const CSphString & sAttr, ESphAttr eAttrType, int iSize )
{
	if ( !CanCreateHistogram ( sAttr, eAttrType ) )
		return nullptr;

	switch ( eAttrType )
	{
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_TIMESTAMP:
	case SPH_ATTR_BOOL:
		return CreateHistogram ( sAttr, HISTOGRAM_STREAMED_UINT32, iSize );

	case SPH_ATTR_BIGINT:	return CreateHistogram ( sAttr, HISTOGRAM_STREAMED_INT64, iSize );
	case SPH_ATTR_FLOAT:	return CreateHistogram ( sAttr, HISTOGRAM_STREAMED_FLOAT, iSize );
	default:				return nullptr;
	}
}


int64_t EstimateFilterSelectivity ( const CSphFilterSettings & tSettings, const HistogramContainer_c * pHistogramContainer )
{
	if ( !pHistogramContainer )
		return INT64_MAX;

	Histogram_i * pHistogram = pHistogramContainer->Get ( tSettings.m_sAttrName );
	if ( !pHistogram || pHistogram->IsOutdated() )
		return INT64_MAX;

	int64_t iEstimate = INT64_MAX;
	if ( !pHistogram->EstimateRsetSize ( tSettings, iEstimate ) )
		return INT64_MAX;

	return iEstimate;
}


void CreateHistograms ( HistogramContainer_c & tHistograms, CSphVector<PlainOrColumnar_t> & dAttrsForHistogram, const ISphSchema & tSchema )
{
	int iColumnar = 0;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		Histogram_i * pHistogram = CreateHistogram ( tAttr.m_sName, tAttr.m_eAttrType );
		if ( pHistogram )
		{
			tHistograms.Add(pHistogram);
			PlainOrColumnar_t & tNewAttr = dAttrsForHistogram.Add();
			if ( tAttr.IsColumnar() )
				tNewAttr.m_iColumnarId = iColumnar;
			else
				tNewAttr.m_tLocator = tAttr.m_tLocator;
		}

		if ( tAttr.IsColumnar() )
			iColumnar++;
	}
}
