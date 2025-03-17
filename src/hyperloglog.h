//
// Copyright (c) 2023-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _hyperloglog_
#define _hyperloglog_

#include "sphinxstd.h"
#include "farmhash/farmhash.h"
#include <algorithm>
#include <vector>
#include <cmath>

template<typename T>
struct FarmHash_T
{
	static inline uint64_t Hash (const T & x );
};

template<>
inline uint64_t FarmHash_T<uint64_t>::Hash ( const uint64_t & x )
{
	return farmhash::Fingerprint(x);
}


inline double FastInversePow2 ( int iExp )
{
	union
	{
		double d;
		uint64_t i;
	} tValue;

	tValue.i = ( 1023ULL - iExp ) << 52;
	return tValue.d;
}


double LinearCounting ( int iV, int iM );

// Non-packed registers; uses 6 bits out of 8
class RegistersNonPacked_c
{
public:
	RegistersNonPacked_c ( int iBitsPerBucket, int iBuckets );

	FORCE_INLINE void Update ( int iBucket, uint8_t uValue )
	{
		assert ( uValue<=m_uMaxValue );
		auto & tStored = m_dValues[iBucket];
		if ( uValue > tStored )
		{
			m_iUsed += !tStored;
			tStored = uValue;
		}
	}

	FORCE_INLINE uint8_t Get ( int iBucket ) const { return m_dValues[iBucket]; }
	int			GetNumUnused() const { return m_dValues.GetLength()-m_iUsed; }
	void		Reset();

private:
	CSphFixedVector<uint8_t> m_dValues;
	uint8_t		m_uMaxValue = 0;
	int			m_iUsed = 0;
};


template <typename T=uint64_t>
class RegistersPacked_T
{
public:
	RegistersPacked_T ( int iBitsPerBucket, int iBuckets )
		: m_dData ( ( iBuckets*iBitsPerBucket + ELEM_BITS - 1 ) / ELEM_BITS )
		, m_iBitsPerBucket ( iBitsPerBucket )
		, m_iNumBuckets ( iBuckets )
		, m_uMask ( ( 1 << iBitsPerBucket ) - 1 )
	{
		m_dData.ZeroVec();
	}

	FORCE_INLINE void Update ( int iBucket, uint8_t uValue )
	{
		int iBitOffset = uint64_t(iBucket)*m_iBitsPerBucket;      
		int iIdx = iBitOffset >> ELEM_SHIFT;
		iBitOffset -= iIdx << ELEM_SHIFT;
		if ( iBitOffset + m_iBitsPerBucket <= ELEM_BITS)
		{
			T & tElem = m_dData[iIdx];
			uint8_t uStored = ( tElem >> iBitOffset ) & m_uMask;
			if ( uValue > uStored )
			{
				m_iUsed += !uStored;
				tElem = ( tElem & ~( m_uMask << iBitOffset ) ) | ( (T)uValue << iBitOffset );
			}
		}
		else
		{
			T & tElem1 = m_dData[iIdx];
			T & tElem2 = m_dData[iIdx + 1];

			int iBitShift = ELEM_BITS-iBitOffset;
			uint8_t uStored = ( tElem1 >> iBitOffset ) | ( ( tElem2 << iBitShift ) & m_uMask );
			if ( uValue > uStored )
			{
				m_iUsed += !uStored;
				tElem1 = ( tElem1 & ~( m_uMask << iBitOffset ) ) | ( (T)uValue << iBitOffset );
				tElem2 = ( tElem2 & ~( m_uMask >> m_iBitsPerBucket ) ) | ( (T)uValue >> iBitShift );
			}
		}
	}

	FORCE_INLINE uint8_t Get ( int iBucket ) const
	{
		int iBitOffset = uint64_t(iBucket)*m_iBitsPerBucket;      
		int iIdx = iBitOffset >> ELEM_SHIFT;
		iBitOffset -= iIdx << ELEM_SHIFT;
		if ( iBitOffset + m_iBitsPerBucket <= ELEM_BITS)
			return ( m_dData[iIdx] >> iBitOffset ) & m_uMask;

		return ( m_dData[iIdx] >> iBitOffset ) | ( ( m_dData[iIdx + 1] << ( ELEM_BITS-iBitOffset ) ) & m_uMask );
	}

	int	GetNumUnused() const { return m_iNumBuckets-m_iUsed; }

	void Reset()
	{
		m_dData.ZeroVec();
		m_iUsed = 0;
	}

private:
	static const int ELEM_BITS = sizeof(T)*8;
	static const int ELEM_SHIFT = sphLog2const(ELEM_BITS) - 1;

	CSphFixedVector<T>	m_dData;
	int		m_iBitsPerBucket = 0;
	int		m_iNumBuckets = 0;
	T		m_uMask = 0;
	int		m_iUsed = 0;
};

extern const std::vector<std::vector<double>> g_dRawEstimateData;
extern const std::vector<std::vector<double>> g_dBiasData;

// implementation based on "HyperLogLog in Practice: Algorithmic Engineering of a State of The Art Cardinality Estimation Algorithm"
template<typename T=uint64_t, typename HASH=FarmHash_T<T>, typename STORAGE=RegistersPacked_T<>>
class HyperLogLogDense_T
{
	template<int, int, typename, typename> friend class HyperLogLogSparse_T;

public:
	HyperLogLogDense_T ( int iAccuracy=14 )
		: m_tStorage ( 6, 1 << iAccuracy )
		, m_iP ( iAccuracy )
		, m_iM ( 1 << m_iP )
		, m_uMask ( ( 1ULL << ( 64-m_iP ) ) - 1 )
	{
		assert ( iAccuracy>=MIN_P && iAccuracy<=18 );
	}

	FORCE_INLINE void Add ( const T & tValue )
	{
		uint64_t uHash = HASH::Hash(tValue);
		int iRegister = uHash >> ( 64 - m_iP );
		uint8_t uCount = ( uHash ? GetLeadingZeroBits ( uHash & m_uMask ) : sizeof(uint64_t)*8 ) - m_iP + 1;
		m_tStorage.Update ( iRegister, uCount );
	}

	uint64_t Estimate() const
	{
		int iV = m_tStorage.GetNumUnused();
		if ( iV )
		{
			uint64_t uH = LinearCounting ( iV, m_iM );
			if ( uH<=GetThreshold() )
				return uH;
		}

		return (uint64_t)round ( CalcEstimate() );
	}

	FORCE_INLINE void Clear() { m_tStorage.Reset(); }

	void Merge ( const HyperLogLogDense_T & tRhs )
	{
		assert ( m_iP==tRhs.m_iP && m_iM==tRhs.m_iM );

		for ( int i = 0; i < m_iM; i++ )
			m_tStorage.Update ( i, tRhs.m_tStorage.Get(i) );
	}

private:
	static const int MIN_P = 14;

	STORAGE		m_tStorage;
	int			m_iP = 0;
	int			m_iM = 0;
	uint64_t	m_uMask = 0;

	double GetAlpha() const { return 0.7213 / ( 1.0 + (1.079 / m_iM ) ); }

	double GetThreshold() const
	{
		switch ( m_iP )
		{
		case 14: return 11500;
		case 15: return 20000;
		case 16: return 50000;
		case 17: return 120000;
		case 18: return 350000;
		default: assert ( 0 && "Unsupported HLL accuracy" ); return 0;
		}
	}

	double EstimateBias ( double fEstimate ) const
	{
		const auto & dRawEstimate = g_dRawEstimateData[m_iP - MIN_P];
		const auto & dBias = g_dBiasData[m_iP - MIN_P];

		auto tFound = std::lower_bound ( dRawEstimate.begin(), dRawEstimate.end(), fEstimate );
		int iFound = 0;
		int iNumRawEstimates = (int)dRawEstimate.size();
		if ( tFound==dRawEstimate.end() )
			iFound = iNumRawEstimates - 1;
		else
			iFound = int ( tFound - dRawEstimate.begin() );

		const int NUM_NEIGHBOURS = 6;
		std::pair<int, double> dNearest[NUM_NEIGHBOURS*2 + 1];
		int iNumNearest = 0;
		for ( int i = std::max ( iFound - NUM_NEIGHBOURS, 0 ); i <= std::min ( iFound + NUM_NEIGHBOURS, iNumRawEstimates-1 ); i++ )
			dNearest[iNumNearest++] = { i, std::fabs ( dRawEstimate[i] - fEstimate ) };

		sphSort ( dNearest, iNumNearest, ::bind ( &std::pair<int, double>::second ) );

		double fAvg = 0;
		for ( int i = 0; i < NUM_NEIGHBOURS; i++ )
			fAvg += dBias[dNearest[i].first];

		return fAvg/NUM_NEIGHBOURS;
	}

	double CalcEstimate() const
	{
		double fE = 0.0;
		for ( int i = 0; i < m_iM; i++ )
			fE += FastInversePow2 ( m_tStorage.Get(i) );

		fE = GetAlpha()*m_iM*m_iM/fE;
		if ( fE <= 5.0*m_iM )
			fE = std::max ( fE - EstimateBias(fE), 0.0 );

		return fE;
	}
};

// Sparse HyperLogLog implementation. Unused for now.
#if 0
template<int SORTED_ARRAY_SIZE, int UNSORTED_ARRAY_SIZE, typename T=uint64_t, typename HASH=FarmHash_T<T>>
class HyperLogLogSparse_T
{
public:
	HyperLogLogSparse_T ( int iAccuracy=14 )
		: m_pCurSorted ( &m_dSorted1 )
		, m_iP ( iAccuracy )
		, m_iM ( 1 << m_iP )
		, m_uHashMask ( ( 1ULL << ( 64-m_iP ) ) - 1 )
		, m_uCountMask ( ( 1UL << 6 ) - 1 )
		, m_uRegMask ( ~m_uCountMask )
	{
		m_dSorted1.Reserve ( SORTED_ARRAY_SIZE/4 );
		m_dSorted2.Reserve ( SORTED_ARRAY_SIZE/4 );
	}

	FORCE_INLINE void Add ( const T & tValue )
	{
		if ( !tValue )
			return;

		uint64_t uHash = HASH::Hash(tValue);
		uint32_t uRegister = uHash >> ( 64 - m_iP );
		uint8_t uCount = ( uHash ? GetLeadingZeroBits ( uHash & m_uMask ) : sizeof(uint64_t)*8 ) - m_iP + 1;
		m_dUnsorted.Add ( ( uRegister << 6 ) + uCount );
		if ( m_dUnsorted.GetLength()==UNSORTED_ARRAY_SIZE )
			MergeUnsorted();
	}

	FORCE_INLINE void Clear()
	{
		m_pCurSorted = &m_dSorted1;
		m_dSorted1.Resize(0);
		m_dSorted2.Resize(0);
		m_dUnsorted.Resize(0);
	}

	FORCE_INLINE bool IsFull() const
	{
		return m_pCurSorted && ( m_pCurSorted->GetLength() + m_dUnsorted.GetLength() )>=SORTED_ARRAY_SIZE;
	}

	uint64_t Estimate()
	{
		assert ( m_pCurSorted );
		MergeUnsorted();
		return LinearCounting ( m_iM - m_pCurSorted->GetLength(), m_iM );
	}

	template<typename STORAGE=RegistersNonPacked_c>
	void CopyToDense ( HyperLogLogDense_T<T,HASH, STORAGE> & tDense )
	{
		assert ( m_iP > tDense.m_iP );

		MergeUnsorted();

		int iPDiff = m_iP - tDense.m_iP;
		int iBitsToNewReg = 6 + iPDiff;
		int iRegDiff = 32 - iBitsToNewReg;
		uint32_t uNewCountMask = ( 1ULL << iBitsToNewReg ) - 1;
		uint32_t uPDiffMask = uNewCountMask & ~m_uCountMask;

		assert ( m_pCurSorted );
		for ( auto i : *m_pCurSorted )
		{
			int iNewRegister = i >> ( 6 + iPDiff );
			uint8_t uNewCount;
			uint32_t uPDiff = i & uPDiffMask;
			if ( uPDiff )
				uNewCount = GetLeadingZeroBits ( uPDiff ) - iRegDiff + 1;
			else
				uNewCount = ( i & m_uCountMask ) + iPDiff;

			tDense.m_tStorage.Update ( iNewRegister, uNewCount );
		}
	}

private:
	using SortedArray_c = CSphTightVector<uint32_t>;
	using UnsortedArray_c = LazyVector_T<uint32_t, sph::DefaultRelimit, UNSORTED_ARRAY_SIZE>;

	SortedArray_c	m_dSorted1;
	SortedArray_c	m_dSorted2;
	UnsortedArray_c m_dUnsorted;
	SortedArray_c * m_pCurSorted = nullptr;
	int				m_iP = 0;
	int				m_iM = 0;
	uint64_t		m_uHashMask = 0;
	uint32_t		m_uCountMask = 0;
	uint32_t		m_uRegMask = 0;

	FORCE_INLINE void MergeRegisters ( uint32_t * & pRes, uint32_t * & pPtr )
	{
		uint32_t & uLastAdded = *(pRes-1);
		uint32_t uRegister1 = uLastAdded & m_uRegMask;
		uint32_t uRegister2 = (*pPtr) & m_uRegMask;
		if ( uRegister1==uRegister2 )
		{
			uint8_t uMaxCount = Max ( uLastAdded & m_uCountMask, (*pPtr) & m_uCountMask );
			uLastAdded = uRegister1 | uMaxCount;
			pPtr++;
		}
		else
			*pRes++ = *pPtr++;
	}

	void MergeUnsorted()
	{
		if ( m_dUnsorted.IsEmpty() )
			return;

		m_dUnsorted.Sort();

		assert ( m_pCurSorted );
		SortedArray_c &	tRes = m_pCurSorted==&m_dSorted1 ? m_dSorted2 : m_dSorted1;
		tRes.Resize ( m_pCurSorted->GetLength() + m_dUnsorted.GetLength() );

		uint32_t * pA = m_pCurSorted->Begin();
		uint32_t * pMaxA = m_pCurSorted->end();	// end() accounts for empty array; End() does not
		uint32_t * pB = m_dUnsorted.Begin();
		uint32_t * pMaxB = m_dUnsorted.end();
		uint32_t * pRes = tRes.Begin();

		// add at least one value (for the next loop to work)
		if ( pA < pMaxA && pB < pMaxB )
		{
			if ( *pA < *pB )
				*pRes++ = *pA++;
			else
				*pRes++ = *pB++;
		}

		while ( pA < pMaxA && pB < pMaxB )
		{
			if ( *pA < *pB )
				MergeRegisters ( pRes, pA );
			else
				MergeRegisters ( pRes, pB );
		}

		// we can have a duplicate here, but only the first one
		if ( pRes!=tRes.Begin() && pA < pMaxA )
			MergeRegisters ( pRes, pA );

		while ( pA < pMaxA )
			*pRes++ = *pA++;

		// add at least one value (for the next loop to work)
		if ( pRes==tRes.Begin() && pB < pMaxB )
			*pRes++ = *pB++;

		while ( pB < pMaxB )
			MergeRegisters ( pRes, pB );

		tRes.Resize ( pRes-tRes.Begin() );
		m_dUnsorted.Resize(0);
		m_pCurSorted = &tRes;
	}
};
#endif

#endif
