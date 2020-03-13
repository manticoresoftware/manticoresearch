//
// Copyright (c) 2018-2020, Manticore Software LTD (http://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "secondaryindex.h"

#include "sphinxint.h"
#include "killlist.h"
#include "attribute.h"


static bool HaveIndex ( const CSphString & sAttr )
{
	return sAttr=="id";
}

//////////////////////////////////////////////////////////////////////////
template <typename T>
inline T ConvertType ( SphAttr_t tValue )
{
	return (T)tValue;
}

template <>
inline float ConvertType<float>( SphAttr_t tValue )
{
	return sphDW2F(tValue);
}


template <typename T>
class Histogram_T : public Histogram_i
{
public:
	Histogram_T ( const CSphString & sAttrName )
		: m_sAttrName ( sAttrName )
	{}

	void Setup ( SphAttr_t tMinValue, SphAttr_t tMaxValue ) override
	{
		assert ( TYPE==HISTOGRAM_UINT32 || TYPE==HISTOGRAM_INT64 );

		m_tMinValue = (T)tMinValue;
		m_tMaxValue = (T)tMaxValue;

		int iNumBuckets;
		if ( double(m_tMaxValue)-m_tMinValue+1.0 >= (double)MAX_BUCKETS )
		{
			iNumBuckets = MAX_BUCKETS;
			m_tBucketSize = T( ceil ( ( (double)m_tMaxValue-m_tMinValue) / iNumBuckets ) );
		}
		else
		{
			iNumBuckets = int(m_tMaxValue-m_tMinValue+1);
			m_tBucketSize = T(1);
		}

		m_dBuckets.Reset ( iNumBuckets+2 );
		for ( auto & i : m_dBuckets )
			i = T(0);

		m_bInitialized = true;
	}

	void Insert ( SphAttr_t tValue ) override
	{
		assert ( m_bInitialized );
		m_dBuckets[GetBucketFor ( ConvertType<T>(tValue) )]++;
		m_uValues++;
	}

	void Delete ( SphAttr_t tValue ) override
	{
		assert ( m_bInitialized );
		int iBucket = GetBucketFor ( ConvertType<T>(tValue) );
		assert ( m_dBuckets[iBucket]>0 && m_uValues>0 );
		m_dBuckets[iBucket]--;
		m_uValues--;
	}

	bool EstimateRsetSize ( const CSphFilterSettings & tFilter, int64_t & iEstimate ) const override
	{
		iEstimate = GetNumValues();

		switch ( tFilter.m_eType )
		{
		case SPH_FILTER_VALUES:
			assert ( TYPE==HISTOGRAM_UINT32 || TYPE==HISTOGRAM_INT64 );

			if ( tFilter.m_bExclude )
				return false;

			iEstimate = EstimateValues ( tFilter.GetValueArray(), tFilter.GetNumValues() );
			return true;

		case SPH_FILTER_RANGE:
			assert ( TYPE==HISTOGRAM_UINT32 || TYPE==HISTOGRAM_INT64 );
			iEstimate = EstimateRangeFilter ( tFilter.m_bExclude, tFilter.m_bHasEqualMin, tFilter.m_bHasEqualMax, tFilter.m_bOpenLeft, tFilter.m_bOpenRight, (T)tFilter.m_iMinValue, (T)tFilter.m_iMaxValue );
			return true;

		case SPH_FILTER_FLOATRANGE:
			assert ( TYPE==HISTOGRAM_FLOAT );
			iEstimate = EstimateRangeFilter ( tFilter.m_bExclude, tFilter.m_bHasEqualMin, tFilter.m_bHasEqualMax, tFilter.m_bOpenLeft, tFilter.m_bOpenRight, tFilter.m_fMinValue, tFilter.m_fMaxValue );
			return true;

		default:
			break;
		}

		return false;
	}

	DWORD GetNumValues() const override
	{
		assert ( m_bInitialized );
		return m_uValues;
	}

	bool IsOutdated() const override
	{
		assert ( m_bInitialized );
		const float MAX_OUT_OF_RANGE = 0.3f;
		return float ( m_dBuckets[0] + m_dBuckets.Last() ) / float ( m_uValues ) >= MAX_OUT_OF_RANGE;
	}

	HistogramType_e GetType() const override
	{
		return TYPE;
	}

	const CSphString & GetAttrName() const override
	{
		return m_sAttrName;
	}

	bool Save ( CSphWriter & tWriter ) const override
	{
		assert ( m_bInitialized );
		tWriter.PutDword ( VERSION );
		tWriter.PutBytes ( &m_tMinValue, sizeof(T) );
		tWriter.PutBytes ( &m_tMaxValue, sizeof(T) );
		tWriter.PutBytes ( &m_tBucketSize, sizeof(T) );
		tWriter.PutDword ( m_uValues );
		tWriter.PutDword ( m_dBuckets.GetLength() );
		tWriter.PutBytes ( m_dBuckets.Begin(), m_dBuckets.GetLength()*sizeof(m_dBuckets[0]) );

		return !tWriter.IsError();
	}

	bool Load ( CSphReader & tReader, CSphString & sError ) override
	{
		m_bInitialized = false;

		DWORD uVersion = tReader.GetDword();
		if ( uVersion > VERSION )
			return false;

		tReader.GetBytes ( &m_tMinValue, sizeof(T) );
		tReader.GetBytes ( &m_tMaxValue, sizeof(T) );
		tReader.GetBytes ( &m_tBucketSize, sizeof(T) );
		m_uValues = tReader.GetDword();
		int iBuckets = tReader.GetDword();
		m_dBuckets.Reset(iBuckets);
		tReader.GetBytes ( m_dBuckets.Begin(), m_dBuckets.GetLength()*sizeof(m_dBuckets[0]) );
		m_bInitialized = true;

		if ( tReader.GetErrorFlag() )
		{
			sError = tReader.GetErrorMessage();
			return false;
		}

		return true;
	}

private:
	static const int				MAX_BUCKETS {1024};
	static const DWORD				VERSION {1};
	static const HistogramType_e	TYPE;
	static const T					MIN_BY_TYPE;
	static const T					MAX_BY_TYPE;


	CSphString				m_sAttrName;
	bool					m_bInitialized {false};
	T						m_tMinValue {0};
	T						m_tMaxValue {0};
	T						m_tBucketSize {0};
	DWORD					m_uValues {0};
	CSphFixedVector<DWORD>	m_dBuckets {0};


	inline int GetBucketFor ( const T & tValue ) const
	{
		if ( tValue<m_tMinValue )
			return 0;

		if ( tValue>m_tMaxValue )
			return m_dBuckets.GetLength()-1;

		return 1 + int ( ((double)tValue-m_tMinValue) / m_tBucketSize );
	}

	DWORD EstimateInterval ( T tMin, T tMax, bool bHasEqualMin, bool bHasEqualMax, bool bOpenLeft, bool bOpenRight ) const
	{
		assert ( TYPE==HISTOGRAM_UINT32 || TYPE==HISTOGRAM_INT64 );
		assert ( m_bInitialized );

		if ( !bOpenLeft && !bHasEqualMin && tMin < MAX_BY_TYPE )
			tMin++;

		if ( !bOpenRight && !bHasEqualMax && tMax > MIN_BY_TYPE )
			tMax--;
		
		int iMinBucket = bOpenLeft ? 0 : GetBucketFor ( tMin );
		int iMaxBucket = bOpenRight ? m_dBuckets.GetLength()-1 : GetBucketFor ( tMax );

		DWORD uTotal = 0;
		for ( int i=iMinBucket; i<=iMaxBucket; i++ )
			uTotal += m_dBuckets[i];

		return uTotal;
	}

	DWORD EstimateValues ( const SphAttr_t * pValues, int nValues ) const
	{
		assert ( m_bInitialized );
		DWORD uTotal = 0;
		int iPrevBucket = INT_MIN;
		for ( int i = 0; i < nValues; i++ )
		{
			int iBucket = GetBucketFor ( ConvertType<T>(pValues[i]) );
			if ( iBucket!=iPrevBucket )
			{
				uTotal += m_dBuckets[iBucket];
				iPrevBucket = iBucket;
			}
		}

		return uTotal;
	}

	DWORD EstimateRangeFilter ( bool bExclude, bool bHasEqualMin, bool bHasEqualMax, bool bOpenLeft, bool bOpenRight, T tMinValue, T tMaxValue ) const
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
};

template<> const HistogramType_e Histogram_T<DWORD>::TYPE = HISTOGRAM_UINT32;
template<> const DWORD Histogram_T<DWORD>::MIN_BY_TYPE = 0;
template<> const DWORD Histogram_T<DWORD>::MAX_BY_TYPE = UINT32_MAX;

template<> const HistogramType_e Histogram_T<int64_t>::TYPE = HISTOGRAM_INT64;
template<> const int64_t Histogram_T<int64_t>::MIN_BY_TYPE = 0;
template<> const int64_t Histogram_T<int64_t>::MAX_BY_TYPE = INT64_MAX;

template<> const HistogramType_e Histogram_T<float>::TYPE = HISTOGRAM_FLOAT;
template<> const float Histogram_T<float>::MIN_BY_TYPE = FLT_MIN;
template<> const float Histogram_T<float>::MAX_BY_TYPE = FLT_MAX;


template<>
void Histogram_T<float>::Setup ( SphAttr_t tMinValue, SphAttr_t tMaxValue )
{
	assert ( TYPE==HISTOGRAM_FLOAT );

	m_tMinValue = ConvertType<float>(tMinValue);
	m_tMaxValue = ConvertType<float>(tMaxValue);

	int iNumBuckets;
	if ( m_tMaxValue==m_tMinValue )
	{
		iNumBuckets = 1;
		m_tBucketSize = 1.0f;
	}
	else
	{
		iNumBuckets = MAX_BUCKETS;
		m_tBucketSize = ( m_tMaxValue-m_tMinValue )/iNumBuckets;
	}

	m_dBuckets.Reset ( iNumBuckets+2 );
	for ( auto & i : m_dBuckets )
		i = 0;

	m_bInitialized = true;
}


template<>
DWORD Histogram_T<float>::EstimateInterval ( float tMin, float tMax, bool bHasEqualMin, bool bHasEqualMax, bool bOpenLeft, bool bOpenRight ) const
{
	assert ( TYPE==HISTOGRAM_FLOAT );
	assert ( m_bInitialized );

	int iMinBucket = bOpenLeft ? 0 : GetBucketFor(tMin);
	int iMaxBucket = bOpenRight ? m_dBuckets.GetLength()-1 : GetBucketFor(tMax);

	DWORD uTotal = 0;
	for ( int i=iMinBucket; i<=iMaxBucket; i++ )
		uTotal += m_dBuckets[i];

	return uTotal;
}


//////////////////////////////////////////////////////////////////////////

static bool CanCreateHistogram ( const CSphString sAttrName, ESphAttr eAttrType )
{
	if ( sphIsInternalAttr ( sAttrName ) )
		return false;

	return eAttrType==SPH_ATTR_INTEGER || eAttrType==SPH_ATTR_BIGINT || eAttrType==SPH_ATTR_BOOL || eAttrType==SPH_ATTR_FLOAT || eAttrType==SPH_ATTR_TIMESTAMP;
}


static Histogram_i * CreateHistogram ( const CSphString & sAttr, HistogramType_e eType )
{
	switch ( eType )
	{
	case HISTOGRAM_UINT32:
		return new Histogram_T<DWORD> ( sAttr );

	case HISTOGRAM_INT64:
		return new Histogram_T<int64_t> ( sAttr );

	case HISTOGRAM_FLOAT:
		return new Histogram_T<float> ( sAttr );

	default:
		return nullptr;
	}
}


Histogram_i * CreateHistogram ( const CSphString & sAttr, ESphAttr eAttrType )
{
	if ( !CanCreateHistogram ( sAttr, eAttrType ) )
		return nullptr;

	HistogramType_e eType = HISTOGRAM_NONE;

	switch ( eAttrType )
	{
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_TIMESTAMP:
	case SPH_ATTR_BOOL:
		eType = HISTOGRAM_UINT32;
		break;

	case SPH_ATTR_BIGINT:
		eType = HISTOGRAM_INT64;
		break;

	case SPH_ATTR_FLOAT:
		eType = HISTOGRAM_FLOAT;
		break;

	default:
		break;
	}

	return CreateHistogram ( sAttr, eType );
}

//////////////////////////////////////////////////////////////////////////

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
		Histogram_i * pHistogram = CreateHistogram ( sAttr, eType );
		if ( !pHistogram )
		{
			sError.SetSprintf ( "error loading histograms from %s", sFile.cstr() );
			return false;
		}

		if ( !pHistogram->Load ( tReader, sError ) )
			return false;

		if ( !m_dHistogramHash.Add ( pHistogram, sAttr ) )
		{
			sError.SetSprintf ( "duplicate histograms found in %s", sFile.cstr() );
			return false;
		}
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
	return m_dHistogramHash.Add ( pHistogram, pHistogram->GetAttrName() );
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

class RowidIterator_LookupValues_c : public RowidIterator_i
{
public:
						RowidIterator_LookupValues_c ( const SphAttr_t * pValues, int nValues, const BYTE * pDocidLookup );

	RowID_t				GetNextRowID() override;
	int64_t				GetNumProcessed() const override;

private:
	int64_t				m_iProcessed {0};
	LookupReaderIterator_c m_tLookupReader;
	DocidListReader_c	m_tFilterReader;
	bool				m_bHaveFilterDocs {false};
	bool				m_bHaveLookupDocs {false};
	DocID_t				m_tFilterDocID {0};
	DocID_t				m_tLookupDocID {0};
	RowID_t				m_tLookupRowID {INVALID_ROWID};
};


RowidIterator_LookupValues_c::RowidIterator_LookupValues_c ( const SphAttr_t * pValues, int nValues, const BYTE * pDocidLookup )
	: m_tLookupReader ( pDocidLookup )
	, m_tFilterReader ( pValues, nValues )
{
	// warmup
	m_bHaveFilterDocs = m_tFilterReader.ReadDocID ( m_tFilterDocID );
	m_bHaveLookupDocs = m_tLookupReader.Read ( m_tLookupDocID, m_tLookupRowID );
	m_iProcessed += m_bHaveFilterDocs ? 1 : 0;
	m_iProcessed += m_bHaveLookupDocs ? 1 : 0;
}


RowID_t RowidIterator_LookupValues_c::GetNextRowID()
{
	while ( m_bHaveFilterDocs && m_bHaveLookupDocs )
	{
		if ( m_tFilterDocID < m_tLookupDocID )
		{
			m_tFilterReader.HintDocID ( m_tLookupDocID );
			m_bHaveFilterDocs = m_tFilterReader.ReadDocID ( m_tFilterDocID );
			m_iProcessed++;
		}
		else if ( m_tFilterDocID > m_tLookupDocID )
		{
			m_tLookupReader.HintDocID ( m_tFilterDocID );
			m_bHaveLookupDocs = m_tLookupReader.Read ( m_tLookupDocID, m_tLookupRowID );
			m_iProcessed++;
		}
		else
		{
			// lookup reader can have duplicates; filter reader can't have duplicates
			// advance only the lookup reader
			RowID_t tMatchedRowID = m_tLookupRowID;
			m_bHaveLookupDocs = m_tLookupReader.Read ( m_tLookupDocID, m_tLookupRowID );
			m_iProcessed++;
			return tMatchedRowID;
		}
	}

	return INVALID_ROWID;
}


int64_t	RowidIterator_LookupValues_c::GetNumProcessed() const
{
	return m_iProcessed;
}

//////////////////////////////////////////////////////////////////////////

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
class RowidIterator_LookupRange_T : public RowidIterator_i
{
public:
						RowidIterator_LookupRange_T ( DocID_t tMinValue, DocID_t tMaxValue, const BYTE * pDocidLookup );

	RowID_t				GetNextRowID() override;
	int64_t				GetNumProcessed() const override;

protected:
	int64_t				m_iProcessed {0};
	LookupReaderIterator_c m_tLookupReader;
	DocID_t				m_tMinValue {0};
	DocID_t				m_tMaxValue {0};
};


template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT>::RowidIterator_LookupRange_T ( DocID_t tMinValue, DocID_t tMaxValue, const BYTE * pDocidLookup )
	: m_tLookupReader ( pDocidLookup )
	, m_tMinValue ( tMinValue )
	, m_tMaxValue ( tMaxValue )
{
	if_const ( !OPEN_LEFT )
		m_tLookupReader.HintDocID ( tMinValue );
}


template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
RowID_t RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT>::GetNextRowID()
{
	DocID_t tLookupDocID = 0;
	RowID_t tLookupRowID = INVALID_ROWID;

	while ( m_tLookupReader.Read ( tLookupDocID, tLookupRowID ) )
	{
		m_iProcessed++;

		if ( !OPEN_LEFT && ( tLookupDocID < m_tMinValue || ( !HAS_EQUAL_MIN && tLookupDocID==m_tMinValue ) ) )
			continue;

		if ( !OPEN_RIGHT && ( tLookupDocID > m_tMaxValue || ( !HAS_EQUAL_MAX && tLookupDocID==m_tMaxValue ) ) )
			return INVALID_ROWID;

		return tLookupRowID;
	}

	return INVALID_ROWID;
}


template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
int64_t RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT>::GetNumProcessed() const
{
	return m_iProcessed;
}


//////////////////////////////////////////////////////////////////////////

template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
class RowidIterator_LookupRangeExclude_T : public RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT>
{
public:
						RowidIterator_LookupRangeExclude_T ( DocID_t tMinValue, DocID_t tMaxValue, const BYTE * pDocidLookup );

	RowID_t				GetNextRowID() override;

private:
	bool				m_bLeft {true};
};


template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
RowidIterator_LookupRangeExclude_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT>::RowidIterator_LookupRangeExclude_T ( DocID_t tMinValue, DocID_t tMaxValue, const BYTE * pDocidLookup )
	: RowidIterator_LookupRange_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT> ( tMinValue, tMaxValue, pDocidLookup )
{
	if ( OPEN_LEFT && !OPEN_RIGHT )
		this->m_tLookupReader.HintDocID ( this->m_tMaxValue );

	m_bLeft = OPEN_RIGHT || ( !OPEN_LEFT && !OPEN_RIGHT ); 
}


template <bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT>
RowID_t RowidIterator_LookupRangeExclude_T<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT>::GetNextRowID()
{
	if_const ( OPEN_LEFT && OPEN_RIGHT )
		return INVALID_ROWID;

	DocID_t tLookupDocID = 0;
	RowID_t tLookupRowID = INVALID_ROWID;

	while ( this->m_tLookupReader.Read ( tLookupDocID, tLookupRowID ) )
	{
		this->m_iProcessed++;

		if ( m_bLeft )
		{
			// use everything ending with m_tMinValue
			if ( tLookupDocID > this->m_tMinValue || ( HAS_EQUAL_MIN && tLookupDocID==this->m_tMinValue ) )
			{
				// switch to right interval
				if_const ( !OPEN_LEFT && !OPEN_RIGHT )
				{
					m_bLeft = false;
					this->m_tLookupReader.HintDocID ( this->m_tMaxValue );
					continue;
				}

				return INVALID_ROWID;
			}
		}
		else
		{
			// use everything starting from m_tMaxValue
			if ( tLookupDocID < this->m_tMaxValue || ( HAS_EQUAL_MAX && tLookupDocID==this->m_tMaxValue ) )
				continue;
		}

		return tLookupRowID;
	}

	return INVALID_ROWID;
}

//////////////////////////////////////////////////////////////////////////

class RowidIterator_Intersect_c : public RowidIterator_i
{
public:
				RowidIterator_Intersect_c ( const CSphVector<RowidIterator_i*> & dIterators );
				~RowidIterator_Intersect_c() override;

	RowID_t		GetNextRowID() override;
	int64_t		GetNumProcessed() const override;

private:
	const CSphVector<RowidIterator_i*>	m_dIterators;
	const CSphVector<RowID_t>			m_dMinRowID;
	int									m_iMaxIndex	{0};

	void		AdvanceIterators();
};


RowidIterator_Intersect_c::RowidIterator_Intersect_c ( const CSphVector<RowidIterator_i*> & dIterators )
	: m_dIterators		( dIterators )
	, m_dMinRowID		( dIterators.GetLength() )
{
	AdvanceIterators();
}


RowidIterator_Intersect_c::~RowidIterator_Intersect_c()
{
	for ( auto & i : m_dIterators )
		SafeDelete(i);
}


RowID_t RowidIterator_Intersect_c::GetNextRowID()
{
	if ( m_dMinRowID[m_iMaxIndex]==INVALID_ROWID )
		return INVALID_ROWID;

	int iNewMaxIndex = m_iMaxIndex;

	do
	{
		m_iMaxIndex = iNewMaxIndex;

		ARRAY_FOREACH ( i, m_dIterators )
			if ( i!=m_iMaxIndex )
			{
				while ( m_dMinRowID[i]<m_dMinRowID[m_iMaxIndex] && m_dMinRowID[i]!=INVALID_ROWID )
					m_dMinRowID[i] = m_dIterators[i]->GetNextRowID();

				if ( m_dMinRowID[i]==INVALID_ROWID )
					return INVALID_ROWID;

				if ( m_dMinRowID[i] > m_dMinRowID[iNewMaxIndex] )
					iNewMaxIndex = i;
			}
	}
	while ( iNewMaxIndex!=m_iMaxIndex );

	RowID_t tRowID = m_dMinRowID[m_iMaxIndex];
	AdvanceIterators();

	return tRowID;
}


int64_t RowidIterator_Intersect_c::GetNumProcessed() const
{
	int64_t iTotal = 0;
	for ( auto i : m_dIterators )
		iTotal += i->GetNumProcessed();

	return iTotal;
}


void RowidIterator_Intersect_c::AdvanceIterators()
{
	m_iMaxIndex = 0;

	ARRAY_FOREACH ( i, m_dIterators )
	{
		m_dMinRowID[i] = m_dIterators[i]->GetNextRowID();
		if ( m_dMinRowID[i] > m_dMinRowID[m_iMaxIndex] )
			m_iMaxIndex = i;
	}
}

//////////////////////////////////////////////////////////////////////////

#define CREATE_ITERATOR_WITH_OPEN(ITERATOR,SETTINGS,MIN,MAX,LOOKUP) \
{ \
	if ( SETTINGS.m_bOpenLeft ) \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new ITERATOR<true,true,true,false>(MIN,MAX,LOOKUP); \
		else \
			return new ITERATOR<true,false,true,false>(MIN,MAX,LOOKUP); \
	} else if ( SETTINGS.m_bOpenRight ) \
	{ \
		if ( SETTINGS.m_bHasEqualMin ) \
			return new ITERATOR<true,true,false,true>(MIN,MAX,LOOKUP); \
		else \
			return new ITERATOR<false,true,false,true>(MIN,MAX,LOOKUP); \
	} \
	assert ( !SETTINGS.m_bOpenLeft && !SETTINGS.m_bOpenRight ); \
	if ( SETTINGS.m_bHasEqualMin ) \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new ITERATOR<true,true,false,false>(MIN,MAX,LOOKUP); \
		else \
			return new ITERATOR<true,false,false,false>(MIN,MAX,LOOKUP); \
	} else \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new ITERATOR<false,true,false,false>(MIN,MAX,LOOKUP); \
		else \
			return new ITERATOR<false,false,false,false>(MIN,MAX,LOOKUP); \
	} \
}


static RowidIterator_i * CreateIterator ( const CSphFilterSettings & tFilter, const BYTE * pDocidLookup )
{
	if ( !HaveIndex ( tFilter.m_sAttrName ) )
		return nullptr;

	switch ( tFilter.m_eType )
	{
	case SPH_FILTER_VALUES:
		return new RowidIterator_LookupValues_c ( tFilter.GetValueArray(), tFilter.GetNumValues(), pDocidLookup );

	case SPH_FILTER_RANGE:
		if ( tFilter.m_bExclude )
		{
			CREATE_ITERATOR_WITH_OPEN ( RowidIterator_LookupRangeExclude_T, tFilter, tFilter.m_iMinValue, tFilter.m_iMaxValue, pDocidLookup );
		}
		else
		{
			CREATE_ITERATOR_WITH_OPEN ( RowidIterator_LookupRange_T, tFilter, tFilter.m_iMinValue, tFilter.m_iMaxValue, pDocidLookup );
		}
		break;
		
	default:
		break;
	}

	return nullptr;
}


struct IndexWithEstimate_t : public SecondaryIndexInfo_t
{
	bool	m_bEnabled {false};
	bool	m_bForce {false};
	int64_t	m_iRsetEstimate {0};
};


bool NextSet ( CSphBitvec & dSet, const CSphVector<IndexWithEstimate_t> & dSecondaryIndexes )
{
	for ( int i = 0; i < dSet.GetBits(); i++ )
	{
		if ( dSecondaryIndexes[i].m_bForce )
			continue;

		bool bState = dSet.BitGet(i);
		if ( bState )
			dSet.BitClear(i);
		else
		{
			dSet.BitSet(i);
			return true;
		}
	}

	return false;
}


class CostEstimate_c
{
public:
	float LookupRead ( int64_t iDocs ) const
	{
		return 10.0f*float(iDocs)*SCALE;
	}

	float Filter ( int64_t iDocs, int nFilters ) const
	{
		return 5.0f*nFilters*float(iDocs)*SCALE;
	}

	float Fullscan ( int64_t iDocs ) const
	{
		return 5.0f*float(iDocs)*SCALE;
	}

	float IndexIntersect ( int64_t iDocs ) const
	{
		return 8.0f*float(iDocs)*SCALE;
	}

private:
	static constexpr float SCALE = 1.0f/1000000.0f;
};


float CalcQueryCost ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<IndexWithEstimate_t> & dSecondaryIndexes, int64_t iTotalDocs )
{
	float fCost = 0.0f;
	int64_t iDocsProcessedByIndexes = 0;
	float fTotalIndexProbability = 1.0f;

	int nEnabled = 0;
	for ( const auto & tIndex : dSecondaryIndexes )
	{
		if ( !tIndex.m_bEnabled )
			continue;

		nEnabled++;
		iDocsProcessedByIndexes += tIndex.m_iRsetEstimate;

		float fIndexProbability = float(tIndex.m_iRsetEstimate) / iTotalDocs;
		fTotalIndexProbability *= fIndexProbability;
	}


	CostEstimate_c tCost;

	if ( !nEnabled )
	{
		fCost += tCost.Fullscan ( iTotalDocs );
		fCost += tCost.Filter ( iTotalDocs, dFilters.GetLength() );
	}
	else
	{
		fCost += tCost.LookupRead ( iDocsProcessedByIndexes );

		if ( nEnabled>1 )
		{
			fCost += tCost.IndexIntersect ( iDocsProcessedByIndexes );
			fCost += tCost.Filter ( uint64_t(fTotalIndexProbability*iTotalDocs), dFilters.GetLength()-nEnabled );
		}
		else
			fCost += tCost.Filter ( iDocsProcessedByIndexes, dFilters.GetLength()-nEnabled );
	}

	return fCost;
}


void SelectIterators ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<IndexHint_t> & dHints, CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, const HistogramContainer_c & tHistograms )
{
	dEnabledIndexes.Resize(0);

	bool bHaveUseHint = false;
	ARRAY_FOREACH_COND ( i, dHints, !bHaveUseHint )
		bHaveUseHint = dHints[i].m_eHint==INDEX_HINT_USE;

	CSphVector<IndexWithEstimate_t> dSecondaryIndexes;
	ARRAY_FOREACH ( i, dFilters )
	{
		const CSphFilterSettings & tFilter = dFilters[i];

		if ( !HaveIndex ( tFilter.m_sAttrName.cstr() ) )
			continue;

		const Histogram_i * pHistogram = tHistograms.Get ( tFilter.m_sAttrName );
		if ( !pHistogram )
			continue;

		IndexHint_e * pHint = nullptr;
		ARRAY_FOREACH_COND ( i, dHints, !pHint )
			if ( dHints[i].m_sIndex==tFilter.m_sAttrName )
				pHint = &dHints[i].m_eHint;

		if ( pHint && *pHint==INDEX_HINT_IGNORE )
			continue;

		if ( bHaveUseHint && !pHint )
			continue;

		IndexWithEstimate_t tNewIndex;

		bool bUsable = pHistogram->EstimateRsetSize ( tFilter, tNewIndex.m_iRsetEstimate );
		if ( bUsable )
		{
			tNewIndex.m_iFilterId = i;
			tNewIndex.m_bForce = pHint && *pHint==INDEX_HINT_FORCE;

			dSecondaryIndexes.Add ( tNewIndex );
		}
	}

	int nAvailableIndexes = dSecondaryIndexes.GetLength();
	if ( !nAvailableIndexes )
		return;

	CSphBitvec dEnabled ( nAvailableIndexes );
	CSphBitvec dBestEnabled ( nAvailableIndexes );

	for ( int i = 0; i < dEnabled.GetBits(); i++ )
		if ( dSecondaryIndexes[i].m_bForce )
			dEnabled.BitSet(i);

	float fBestCost = FLT_MAX;

	const int MAX_TRIES = 1024;
	for ( int iTry = 0; iTry < MAX_TRIES; iTry++ )
	{
		for ( int i = 0; i < dEnabled.GetBits(); i++ )
			dSecondaryIndexes[i].m_bEnabled = dEnabled.BitGet(i);

		float fCost = CalcQueryCost ( dFilters, dSecondaryIndexes, tHistograms.GetNumValues() );
		if ( fCost < fBestCost )
		{
			dBestEnabled = dEnabled;
			fBestCost = fCost;
		}

		if ( !NextSet ( dEnabled, dSecondaryIndexes ) )
			break;
	}

	for ( int i = 0; i < dBestEnabled.GetBits(); i++ )
		if ( dBestEnabled.BitGet(i) )
		{
			SecondaryIndexInfo_t & tInfo = dEnabledIndexes.Add();
			tInfo.m_iFilterId = dSecondaryIndexes[i].m_iFilterId;
		}
}


RowidIterator_i * CreateFilteredIterator ( const CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphFilterSettings> & dModifiedFilters, const CSphVector<FilterTreeItem_t> & dFilterTree, const CSphVector<IndexHint_t> & dHints, const HistogramContainer_c & tHistograms, const BYTE * pDocidLookup )
{
	// no iterators with OR queries
	if ( dFilterTree.GetLength() )
		return nullptr;

	CSphVector<SecondaryIndexInfo_t> dEnabledIndexes;
	SelectIterators ( dFilters, dHints, dEnabledIndexes, tHistograms );

	CSphVector<RowidIterator_i *> dIterators;
	for ( auto & i : dEnabledIndexes )
	{
		RowidIterator_i * pIterator = CreateIterator ( dFilters[i.m_iFilterId], pDocidLookup );
		if ( pIterator )
			dIterators.Add ( pIterator );
	}

	dEnabledIndexes.Sort ( bind ( &SecondaryIndexInfo_t::m_iFilterId ) );
	ARRAY_FOREACH ( i, dFilters )
	{
		if ( !dEnabledIndexes.FindFirst ( [i] ( const SecondaryIndexInfo_t & tInfo ) { return tInfo.m_iFilterId==i; } ) )
			dModifiedFilters.Add ( dFilters[i] );
	}

	int nIterators = dIterators.GetLength();
	if ( nIterators==1 )
		return dIterators[0];
	else if ( nIterators>1 )
		return new RowidIterator_Intersect_c ( dIterators );

	return nullptr;	
}

//////////////////////////////////////////////////////////////////////////

DocidLookupWriter_c::DocidLookupWriter_c ( DWORD nDocs )
	: m_nDocs ( nDocs )
{}


DocidLookupWriter_c::~DocidLookupWriter_c()
{
	SafeDelete ( m_pWriter );
}


bool DocidLookupWriter_c::Open ( const CSphString & sFilename, CSphString & sError )
{
	assert ( !m_pWriter );
	m_pWriter = new CSphWriter;

	if ( !m_pWriter->OpenFile ( sFilename, sError ) )
		return false;

	m_pWriter->PutDword ( m_nDocs );
	m_pWriter->PutDword ( DOCS_PER_LOOKUP_CHECKPOINT );

	m_tCheckpointStart = m_pWriter->GetPos();
	m_pWriter->PutOffset ( 0 );	// reserve space for max docid

	int nCheckpoints = (m_nDocs+DOCS_PER_LOOKUP_CHECKPOINT-1)/DOCS_PER_LOOKUP_CHECKPOINT;
	m_dCheckpoints.Reset ( nCheckpoints );
	for ( int i = 0; i < nCheckpoints; i++ )
	{
		// reserve space for checkpoints
		m_pWriter->PutOffset(0);
		m_pWriter->PutOffset(0);
	}

	return true;
}


void DocidLookupWriter_c::AddPair ( const DocidRowidPair_t & tPair )
{
	assert ( tPair.m_tDocID>=m_tLastDocID );

	if ( !(m_iProcessed % DOCS_PER_LOOKUP_CHECKPOINT) )
	{
		m_dCheckpoints[m_iCheckpoint].m_tBaseDocID = tPair.m_tDocID;
		m_dCheckpoints[m_iCheckpoint].m_tOffset = m_pWriter->GetPos();
		m_iCheckpoint++;

		// no need to store docid for 1st entry
		m_pWriter->PutDword ( tPair.m_tRowID );
	}
	else
	{
		m_pWriter->ZipOffset ( tPair.m_tDocID-m_tLastDocID );
		m_pWriter->PutDword ( tPair.m_tRowID );
	}

	m_tLastDocID = tPair.m_tDocID;
	m_iProcessed++;
}


bool DocidLookupWriter_c::Finalize ( CSphString & sError )
{
	m_pWriter->Flush();
	m_pWriter->SeekTo ( m_tCheckpointStart );
	m_pWriter->PutOffset ( m_tLastDocID );
	for ( const auto & i : m_dCheckpoints )
	{
		m_pWriter->PutOffset ( i.m_tBaseDocID );
		m_pWriter->PutOffset ( i.m_tOffset );
	}

	m_pWriter->CloseFile();
	if ( m_pWriter->IsError() )
	{
		sError = "error writing .SPT";
		return false;
	}

	return true;
}


CSphWriter & DocidLookupWriter_c::GetWriter()
{
	assert ( m_pWriter );
	return *m_pWriter;
}


bool WriteDocidLookup ( const CSphString & sFilename, const CSphFixedVector<DocidRowidPair_t> & dLookup, CSphString & sError )
{
	DocidLookupWriter_c tWriter ( dLookup.GetLength() );
	if ( !tWriter.Open ( sFilename, sError ) )
		return false;

	for ( const auto & i : dLookup )
		tWriter.AddPair(i);

	if ( !tWriter.Finalize ( sError ) )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////

LookupReader_c::LookupReader_c ( const BYTE * pData )
{
	SetData ( pData );
}


void LookupReader_c::SetData ( const BYTE * pData )
{
	m_pData = pData;

	if ( !pData )
		return;

	const BYTE * p = pData;
	m_nDocs = *(DWORD*)p;
	p += sizeof(DWORD);
	m_nDocsPerCheckpoint = *(DWORD*)p;
	p += sizeof(DWORD);
	m_tMaxDocID = *(DocID_t*)p;
	p += sizeof(DocID_t);

	m_nCheckpoints = (m_nDocs+m_nDocsPerCheckpoint-1)/m_nDocsPerCheckpoint;
	m_pCheckpoints = (DocidLookupCheckpoint_t *)p;
}

//////////////////////////////////////////////////////////////////////////

LookupReaderIterator_c::LookupReaderIterator_c ( const BYTE * pData )
{
	SetData(pData);
}


void LookupReaderIterator_c::SetData ( const BYTE * pData )
{
	LookupReader_c::SetData(pData);
	SetCheckpoint ( m_pCheckpoints );
}
