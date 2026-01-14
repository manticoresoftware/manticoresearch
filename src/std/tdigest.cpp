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
			return m_dMap.begin()->first;
		if ( fQuantile>=1.0 )
			return std::prev ( m_dMap.end() )->first;

		if ( m_iCount<=1 )
			return m_dMap.begin()->first;

		double fRank = fQuantile * ( m_iCount - 1 );
		int64_t iLower = (int64_t)std::floor ( fRank );
		double fFrac = fRank - iLower;

		double fLower = ValueAtRank ( iLower );
		if ( fFrac==0.0 )
			return fLower;

		double fUpper = ValueAtRank ( iLower+1 );
		return fLower + fFrac * ( fUpper - fLower );
	}

	double Cdf ( double fValue ) const noexcept
	{
		if ( m_dMap.empty() )
			return 0.0;

		auto iMapFirst = m_dMap.begin();
		auto iMapLast = std::prev ( m_dMap.end() );

		if ( fValue <= iMapFirst->first )
			return 0.0;
		if ( fValue >= iMapLast->first )
			return 1.0;

		double fCumulative = 0.0;
		auto iPrev = iMapFirst;
		for ( auto i = m_dMap.begin(); i!=m_dMap.end(); ++i )
		{
			double fMean = i->first;
			if ( fValue < fMean )
			{
				if ( i==iMapFirst )
					return 0.0;

				double fLeft = iPrev->first;
				double fSpan = fMean - fLeft;
				double fFraction = ( fSpan>0.0 ) ? ( ( fValue - fLeft ) / fSpan ) : 0.0;
				double fBetween = ( iPrev->second + i->second ) / 2.0;
				double fBefore = fCumulative - iPrev->second;
				double fValueCount = ( iPrev->second / 2.0 ) + fFraction * fBetween;
				return std::min ( 1.0, std::max ( 0.0, ( fBefore + fValueCount ) / m_iCount ) );
			}

			fCumulative += i->second;
			iPrev = i;
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

private:
	using BalancedTree_c = std::multimap<double, int64_t, std::less<double>, managed_allocator<std::pair<const double, int64_t>>>;
	BalancedTree_c		m_dMap;
	int64_t				m_iCount;
	double				m_fCompression;

	double WeightedAvg ( double fX1, int64_t iW1, double fX2, int64_t iW2 )
	{
		return ( fX1*iW1 + fX2*iW2 ) / ( iW1 + iW2 );
	}

	void Reset()
	{
		m_dMap.clear();
		m_iCount = 0;
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

		Reset();

		while ( dValues.GetLength() )
		{
			int iValue = sphRand() % dValues.GetLength();
			Add ( dValues[iValue].m_fMean, dValues[iValue].m_iCount );
			dValues.RemoveFast(iValue);
		}
	}

	double ValueAtRank ( int64_t iRank ) const
	{
		if ( m_dMap.empty() )
			return 0.0;

		if ( iRank<=0 )
			return m_dMap.begin()->first;

		int64_t iLastRank = m_iCount>0 ? (m_iCount-1) : 0;
		if ( iRank>=iLastRank )
			return std::prev ( m_dMap.end() )->first;

		int64_t iAccum = 0;
		for ( const auto & tEntry : m_dMap )
		{
			if ( iRank < iAccum + tEntry.second )
				return tEntry.first;
			iAccum += tEntry.second;
		}

		return std::prev ( m_dMap.end() )->first;
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
