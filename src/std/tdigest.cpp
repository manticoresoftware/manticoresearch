//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "tdigest.h"

#include "mem.h"
#include "rand.h"
#include "vector.h"

#include <cfloat>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <limits>

#if _WIN32
#pragma warning(push,1)
#pragma warning(disable:4530)
#endif

#if NEW_IS_OVERRIDED
#undef new
#endif

#include <map>

#if NEW_IS_OVERRIDED
#define new        new(__FILE__,__LINE__)
#endif

#if _WIN32
#pragma warning(pop)
#endif

class TDigest_c::Impl_c final
{
public:
	explicit Impl_c ( double fCompression )
		: m_fCompression ( std::max ( 1.0, fCompression ) )
	{
		Reset();
	}

	void Add ( double fValue, int64_t iWeight )
	{
		if ( std::isinf ( m_fMin ) )
		{
			m_fMin = fValue;
			m_fMax = fValue;
		}
		else
		{
			m_fMin = std::min ( m_fMin, fValue );
			m_fMax = std::max ( m_fMax, fValue );
		}

		if ( m_dMap.empty() )
		{
			m_dMap.insert ( std::pair<double, int64_t> ( fValue, iWeight ) );
			m_iCount = iWeight;
			return;
		}

		auto tStart = m_dMap.lower_bound(fValue);
		if ( tStart==m_dMap.end() )
			tStart = m_dMap.begin();
		else
		{
			while ( tStart!=m_dMap.begin() && tStart->first==fValue )
				tStart--;
		}

		double fMinDist = DBL_MAX;
		auto tLastNeighbor = m_dMap.end();
		for ( auto i=tStart; i!=m_dMap.end(); ++i )
		{
			double fDist = fabs ( i->first - fValue );
			if ( fDist < fMinDist )
			{
				tStart = i;
				fMinDist = fDist;
			} else if ( fDist > fMinDist )
			{
				// we've passed the nearest nearest neighbor
				tLastNeighbor = i;
				break;
			}
		}

		auto tClosest = m_dMap.end();
		int64_t iSum = 0;
		for ( auto i=m_dMap.begin(); i!=tStart; ++i )
			iSum += i->second;

		int64_t iN = 0;
		const double COMPRESSION = m_fCompression;
		for ( auto i=tStart; i!=tLastNeighbor; ++i )
		{
			double fQuantile = m_iCount==1 ? 0.5 : (iSum + (i->second - 1) / 2.0) / (m_iCount - 1);
			double fThresh = 4.0 * m_iCount * fQuantile * (1 - fQuantile) / COMPRESSION;

			if ( i->second+iWeight<=fThresh )
			{
				iN++;
				if ( ( double ( sphRand() ) / UINT_MAX )<1.0/iN )
					tClosest = i;
			}

			iSum += i->second;
		}

		if ( tClosest==m_dMap.end() )
			m_dMap.insert ( std::pair<double, int64_t> ( fValue, iWeight ) );
		else
		{
			double fNewMean = WeightedAvg ( tClosest->first, tClosest->second, fValue, iWeight );
			int64_t iNewCount = tClosest->second+iWeight;
			m_dMap.erase ( tClosest );
			m_dMap.insert ( std::pair<double, int64_t> ( fNewMean, iNewCount ) );
		}

		m_iCount += iWeight;

		const DWORD K=20;
		if ( m_dMap.size() > K * COMPRESSION )
			Compress();
	}

	double Percentile ( int iPercent ) const noexcept
	{
		return Percentile ( double ( iPercent ) );
	}

	double Percentile ( double fPercent ) const noexcept
	{
		if ( fPercent<=0.0 )
			return Quantile ( 0.0 );
		if ( fPercent>=100.0 )
			return Quantile ( 1.0 );

		return Quantile ( fPercent / 100.0 );
	}

	double Quantile ( double fQuantile ) const noexcept
	{
		if ( m_dMap.empty() )
			return 0.0;

		if ( fQuantile<=0.0 )
			return m_fMin;
		if ( fQuantile>=1.0 )
			return m_fMax;

		if ( m_dMap.size()==1 )
			return m_dMap.begin()->first;

		const double fTotalWeight = (double)m_iCount;
		const double fIndex = fQuantile * fTotalWeight;

		if ( fIndex < 1.0 )
			return m_fMin;

		auto itFirst = m_dMap.begin();
		if ( itFirst->second>1 && fIndex < itFirst->second / 2.0 )
		{
			double fDen = itFirst->second / 2.0 - 1.0;
			if ( fDen>0.0 )
				return m_fMin + ( fIndex - 1.0 ) / fDen * ( itFirst->first - m_fMin );
			return itFirst->first;
		}

		if ( fIndex > fTotalWeight - 1.0 )
			return m_fMax;

		auto itLast = std::prev ( m_dMap.end() );
		if ( itLast->second>1 && fTotalWeight - fIndex <= itLast->second / 2.0 )
		{
			double fDen = itLast->second / 2.0 - 1.0;
			if ( fDen>0.0 )
				return m_fMax - ( ( fTotalWeight - fIndex - 1.0 ) / fDen ) * ( m_fMax - itLast->first );
			return itLast->first;
		}

		double fWeightSoFar = itFirst->second / 2.0;
		auto it = itFirst;
		auto itNext = std::next ( it );
		while ( itNext!=m_dMap.end() )
		{
			double fDw = ( it->second + itNext->second ) / 2.0;
			if ( fWeightSoFar + fDw > fIndex )
			{
				double fLeftUnit = 0.0;
				if ( it->second==1 )
				{
					if ( fIndex - fWeightSoFar < 0.5 )
						return it->first;
					fLeftUnit = 0.5;
				}

				double fRightUnit = 0.0;
				if ( itNext->second==1 )
				{
					if ( fWeightSoFar + fDw - fIndex < 0.5 )
						return itNext->first;
					fRightUnit = 0.5;
				}

				double fZ1 = fIndex - fWeightSoFar - fLeftUnit;
				double fZ2 = fWeightSoFar + fDw - fIndex - fRightUnit;
				return WeightedAverage ( it->first, fZ2, itNext->first, fZ1 );
			}
			fWeightSoFar += fDw;
			++it;
			++itNext;
		}

		double fZ1 = fIndex - fTotalWeight - itLast->second / 2.0;
		double fZ2 = itLast->second / 2.0 - fZ1;
		return WeightedAverage ( itLast->first, fZ1, m_fMax, fZ2 );
	}

	double Cdf ( double fValue ) const noexcept
	{
		if ( m_dMap.empty() )
			return 0.0;

		const double fTotalWeight = (double)m_iCount;

		if ( m_dMap.size()==1 )
		{
			if ( fValue < m_fMin )
				return 0.0;
			if ( fValue > m_fMax )
				return 1.0;

			double fWidth = m_fMax - m_fMin;
			if ( fWidth<=0.0 )
				return 0.5;

			return ( fValue - m_fMin ) / fWidth;
		}

		if ( fValue < m_fMin )
			return 0.0;
		if ( fValue > m_fMax )
			return 1.0;

		auto itFirst = m_dMap.begin();
		auto itLast = std::prev ( m_dMap.end() );

		if ( fValue < itFirst->first )
		{
			double fDelta = itFirst->first - m_fMin;
			if ( fDelta>0.0 )
			{
				if ( fValue==m_fMin )
					return 0.5 / fTotalWeight;
				return ( 1.0 + ( fValue - m_fMin ) / fDelta * ( itFirst->second / 2.0 - 1.0 ) ) / fTotalWeight;
			}
			return 0.0;
		}

		if ( fValue > itLast->first )
		{
			double fDelta = m_fMax - itLast->first;
			if ( fDelta>0.0 )
			{
				if ( fValue==m_fMax )
					return 1.0 - 0.5 / fTotalWeight;
				double fDq = ( 1.0 + ( m_fMax - fValue ) / fDelta * ( itLast->second / 2.0 - 1.0 ) ) / fTotalWeight;
				return 1.0 - fDq;
			}
			return 1.0;
		}

		double fLeftWidth = ( std::next ( itFirst )->first - itFirst->first ) / 2.0;
		double fWeightSoFar = 0.0;

		for ( auto it = itFirst; it!=itLast; ++it )
		{
			auto itNext = std::next ( it );
			double fRightWidth = ( itNext->first - it->first ) / 2.0;
			if ( fValue < it->first + fRightWidth )
			{
				double fInterp = TdInterpolate ( fValue, it->first - fLeftWidth, it->first + fRightWidth );
				double fValueCdf = ( fWeightSoFar + it->second * fInterp ) / fTotalWeight;
				return std::max ( fValueCdf, 0.0 );
			}
			fWeightSoFar += it->second;
			fLeftWidth = fRightWidth;
		}

		double fRightWidth = ( itLast->first - std::prev ( itLast )->first ) / 2.0;
		if ( fValue < itLast->first + fRightWidth )
		{
			double fInterp = TdInterpolate ( fValue, itLast->first - fRightWidth, itLast->first + fRightWidth );
			return ( fWeightSoFar + itLast->second * fInterp ) / fTotalWeight;
		}

		if ( fValue < m_fMax )
		{
			double fTailWidth = ( m_fMax - itLast->first );
			if ( fTailWidth>0.0 )
			{
				double fInterp = TdInterpolate ( fValue, itLast->first, itLast->first + fTailWidth );
				return ( fWeightSoFar + itLast->second + ( fInterp * ( fTotalWeight - fWeightSoFar - itLast->second ) ) ) / fTotalWeight;
			}
		}

		return 1.0;
	}

	void Clear ()
	{
		Reset();
	}

	void Merge ( const Impl_c & tOther )
	{
		for ( const auto & tEntry : tOther.m_dMap )
			Add ( tEntry.first, tEntry.second );

		if ( tOther.m_iCount )
		{
			if ( std::isinf ( m_fMin ) )
			{
				m_fMin = tOther.m_fMin;
				m_fMax = tOther.m_fMax;
			}
			else
			{
				m_fMin = std::min ( m_fMin, tOther.m_fMin );
				m_fMax = std::max ( m_fMax, tOther.m_fMax );
			}
		}
	}

	int64_t GetCount () const
	{
		return m_iCount;
	}

	double GetCompression () const
	{
		return m_fCompression;
	}

	void SetCompression ( double fCompression )
	{
		m_fCompression = std::max ( 1.0, fCompression );
		if ( !m_dMap.empty() )
			Compress();
	}

	void Export ( CSphVector<TDigestCentroid_t> & dOut ) const
	{
		dOut.Resize ( 0 );
		dOut.Reserve ( (int) m_dMap.size() );
		for ( const auto & tEntry : m_dMap )
		{
			TDigestCentroid_t & tCentroid = dOut.Add();
			tCentroid.m_fMean = tEntry.first;
			tCentroid.m_iCount = tEntry.second;
		}
	}

	void Import ( const VecTraits_T<TDigestCentroid_t> & dCentroids )
	{
		Reset();
		for ( const TDigestCentroid_t & tCentroid : dCentroids )
			Add ( tCentroid.m_fMean, tCentroid.m_iCount );
	}

	double GetMin () const noexcept
	{
		return m_iCount ? m_fMin : 0.0;
	}

	double GetMax () const noexcept
	{
		return m_iCount ? m_fMax : 0.0;
	}

	void SetExtremes ( double fMin, double fMax, bool bHasData )
	{
		if ( bHasData )
		{
			m_fMin = fMin;
			m_fMax = fMax;
		}
		else
		{
			m_fMin = std::numeric_limits<double>::infinity();
			m_fMax = -std::numeric_limits<double>::infinity();
		}
	}

private:
	using BalancedTree_c = std::multimap<double, int64_t, std::less<double>, managed_allocator<std::pair<const double, int64_t>>>;
	BalancedTree_c		m_dMap;
	int64_t				m_iCount;
	double				m_fCompression;
	double				m_fMin;
	double				m_fMax;

	double WeightedAvg ( double fX1, int64_t iW1, double fX2, int64_t iW2 )
	{
		return ( fX1*iW1 + fX2*iW2 ) / ( iW1 + iW2 );
	}

	double WeightedAverage ( double fX1, double fW1, double fX2, double fW2 ) const
	{
		if ( fX1<=fX2 )
			return WeightedAverageSorted ( fX1, fW1, fX2, fW2 );
		return WeightedAverageSorted ( fX2, fW2, fX1, fW1 );
	}

	double WeightedAverageSorted ( double fX1, double fW1, double fX2, double fW2 ) const
	{
		if ( fX1>fX2 )
			std::swap ( fX1, fX2 );
		double fSum = fW1 + fW2;
		if ( fSum==0.0 )
			return ( fX1 + fX2 ) / 2.0;
		double fValue = ( fX1*fW1 + fX2*fW2 ) / fSum;
		if ( fValue < fX1 )
			return fX1;
		if ( fValue > fX2 )
			return fX2;
		return fValue;
	}

	static double TdInterpolate ( double fValue, double fLeft, double fRight )
	{
		if ( fRight<=fLeft )
			return 0.0;
		if ( fValue<=fLeft )
			return 0.0;
		if ( fValue>=fRight )
			return 1.0;
		return ( fValue - fLeft ) / ( fRight - fLeft );
	}

	void Reset()
	{
		m_dMap.clear();
		m_iCount = 0;
		m_fMin = std::numeric_limits<double>::infinity();
		m_fMax = -std::numeric_limits<double>::infinity();
	}

	void Compress()
	{
		struct Centroid_t
		{
			double	m_fMean;
			int64_t	m_iCount;
		};

		CSphTightVector<Centroid_t> dValues;
		dValues.Reserve ( (int) m_dMap.size() );
		for ( auto i : m_dMap )
		{
			Centroid_t & tCentroid = dValues.Add();
			tCentroid.m_fMean = i.first;
			tCentroid.m_iCount = i.second;
		}

		double fMin = m_fMin;
		double fMax = m_fMax;
		bool bHadData = ( m_iCount>0 );
		Reset();
		if ( bHadData )
		{
			m_fMin = fMin;
			m_fMax = fMax;
		}

		while ( dValues.GetLength() )
		{
			int iValue = sphRand() % dValues.GetLength();
			Add ( dValues[iValue].m_fMean, dValues[iValue].m_iCount );
			dValues.RemoveFast(iValue);
		}
	}
};


TDigest_c::TDigest_c ( double fCompression )
		: m_pImpl { std::make_unique<TDigest_c::Impl_c> ( fCompression ) }
{}


TDigest_c::~TDigest_c () = default;


void TDigest_c::Add ( double fValue, int64_t iWeight )
{
	m_pImpl->Add ( fValue, iWeight );
}


double TDigest_c::Percentile ( int iPercent ) const noexcept
{
	return m_pImpl->Percentile (iPercent);
}

double TDigest_c::Percentile ( double fPercent ) const noexcept
{
	return m_pImpl->Percentile ( fPercent );
}

double TDigest_c::Quantile ( double fQuantile ) const noexcept
{
	return m_pImpl->Quantile ( fQuantile );
}

double TDigest_c::Cdf ( double fValue ) const noexcept
{
	return m_pImpl->Cdf ( fValue );
}

double TDigest_c::GetMin () const noexcept
{
	return m_pImpl->GetMin();
}

double TDigest_c::GetMax () const noexcept
{
	return m_pImpl->GetMax();
}

void TDigest_c::SetExtremes ( double fMin, double fMax, bool bHasData )
{
	m_pImpl->SetExtremes ( fMin, fMax, bHasData );
}

void TDigest_c::Clear()
{
	m_pImpl->Clear();
}

int64_t TDigest_c::GetCount () const noexcept
{
	return m_pImpl->GetCount();
}

void TDigest_c::Merge ( const TDigest_c & tOther )
{
	m_pImpl->Merge ( *tOther.m_pImpl );
}

void TDigest_c::Export ( CSphVector<TDigestCentroid_t> & dOut ) const
{
	m_pImpl->Export ( dOut );
}

void TDigest_c::Import ( const VecTraits_T<TDigestCentroid_t> & dCentroids )
{
	m_pImpl->Import ( dCentroids );
}

void TDigest_c::SetCompression ( double fCompression )
{
	m_pImpl->SetCompression ( fCompression );
}

double TDigest_c::GetCompression () const noexcept
{
	return m_pImpl->GetCompression ();
}


std::unique_ptr<TDigest_c> sphCreateTDigest()
{
	return std::make_unique<TDigest_c>();
}
