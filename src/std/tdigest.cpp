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
		if ( iWeight<=0 )
			return;

		UpdateExtremes ( fValue );

		if ( m_dCentroids.IsEmpty() )
		{
			auto & tCentroid = m_dCentroids.Add();
			tCentroid.m_fMean = fValue;
			tCentroid.m_iCount = iWeight;
			m_iCount = iWeight;
			return;
		}

		const int n = m_dCentroids.GetLength();
		int iClosest = 0;
		double fMinDist = std::fabs ( m_dCentroids[0].m_fMean - fValue );
		for ( int i = 1; i < n; ++i )
		{
			double fDist = std::fabs ( m_dCentroids[i].m_fMean - fValue );
			if ( fDist < fMinDist )
			{
				fMinDist = fDist;
				iClosest = i;
			} else if ( m_dCentroids[i].m_fMean > fValue && fDist>fMinDist )
			{
				break;
			}
		}

		int64_t iPrefix = 0;
		for ( int i = 0; i < iClosest; ++i )
			iPrefix += m_dCentroids[i].m_iCount;

		int iCandidate = -1;
		int iSeen = 0;
		const double fCompression = m_fCompression;
		for ( int i = iClosest; i < n; ++i )
		{
			double fQuantile = ( m_iCount <= 1 )
				? 0.5
				: ( iPrefix + ( m_dCentroids[i].m_iCount - 1 ) / 2.0 ) / ( m_iCount - 1 );
			double fThresh = 4.0 * (double)m_iCount * fQuantile * ( 1.0 - fQuantile ) / fCompression;

			if ( m_dCentroids[i].m_iCount + iWeight <= fThresh )
			{
				++iSeen;
				if ( (double)sphRand() / UINT_MAX < 1.0 / iSeen )
					iCandidate = i;
			}

			iPrefix += m_dCentroids[i].m_iCount;

			if ( std::fabs ( m_dCentroids[i].m_fMean - fValue ) > fMinDist && i>iClosest )
				break;
		}

		if ( iCandidate==-1 )
		{
			InsertCentroid ( fValue, iWeight );
		}
		else
		{
			TDigestCentroid_t tCentroid = m_dCentroids[iCandidate];
			tCentroid.m_fMean = WeightedAvg ( tCentroid.m_fMean, tCentroid.m_iCount, fValue, iWeight );
			tCentroid.m_iCount += iWeight;
			m_dCentroids.Remove ( iCandidate );
			InsertCentroid ( tCentroid );
		}

		m_iCount += iWeight;

		const double K = 20.0;
		if ( (double)m_dCentroids.GetLength() > K * fCompression )
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
		if ( m_dCentroids.IsEmpty() )
			return 0.0;

		if ( fQuantile<=0.0 )
			return m_fMin;
		if ( fQuantile>=1.0 )
			return m_fMax;

		if ( m_dCentroids.GetLength()==1 )
			return m_dCentroids[0].m_fMean;

		const double fTotalWeight = (double)m_iCount;
		const double fIndex = fQuantile * fTotalWeight;

		if ( fIndex < 1.0 )
			return m_fMin;

		const TDigestCentroid_t & tFirst = m_dCentroids[0];
		if ( tFirst.m_iCount>1 && fIndex < tFirst.m_iCount / 2.0 )
		{
			double fDen = tFirst.m_iCount / 2.0 - 1.0;
			if ( fDen>0.0 )
				return m_fMin + ( fIndex - 1.0 ) / fDen * ( tFirst.m_fMean - m_fMin );
			return tFirst.m_fMean;
		}

		if ( fIndex > fTotalWeight - 1.0 )
			return m_fMax;

		const TDigestCentroid_t & tLast = m_dCentroids[m_dCentroids.GetLength()-1];
		if ( tLast.m_iCount>1 && fTotalWeight - fIndex <= tLast.m_iCount / 2.0 )
		{
			double fDen = tLast.m_iCount / 2.0 - 1.0;
			if ( fDen>0.0 )
				return m_fMax - ( ( fTotalWeight - fIndex - 1.0 ) / fDen ) * ( m_fMax - tLast.m_fMean );
			return tLast.m_fMean;
		}

		double fWeightSoFar = tFirst.m_iCount / 2.0;
		for ( int i = 0; i < m_dCentroids.GetLength() - 1; ++i )
		{
			const auto & tLeft = m_dCentroids[i];
			const auto & tRight = m_dCentroids[i+1];
			double fDw = ( tLeft.m_iCount + tRight.m_iCount ) / 2.0;
			if ( fWeightSoFar + fDw > fIndex )
			{
				double fLeftUnit = 0.0;
				if ( tLeft.m_iCount==1 )
				{
					if ( fIndex - fWeightSoFar < 0.5 )
						return tLeft.m_fMean;
					fLeftUnit = 0.5;
				}

				double fRightUnit = 0.0;
				if ( tRight.m_iCount==1 )
				{
					if ( fWeightSoFar + fDw - fIndex < 0.5 )
						return tRight.m_fMean;
					fRightUnit = 0.5;
				}

				double fZ1 = fIndex - fWeightSoFar - fLeftUnit;
				double fZ2 = fWeightSoFar + fDw - fIndex - fRightUnit;
				return WeightedAverage ( tLeft.m_fMean, fZ2, tRight.m_fMean, fZ1 );
			}
			fWeightSoFar += fDw;
		}

		double fZ1 = fIndex - fTotalWeight - tLast.m_iCount / 2.0;
		double fZ2 = tLast.m_iCount / 2.0 - fZ1;
		return WeightedAverage ( tLast.m_fMean, fZ1, m_fMax, fZ2 );
	}

	double Cdf ( double fValue ) const noexcept
	{
		if ( m_dCentroids.IsEmpty() )
			return 0.0;

		const double fTotalWeight = (double)m_iCount;

		if ( m_dCentroids.GetLength()==1 )
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
		if ( fValue >= m_fMax )
			return 1.0;

		const TDigestCentroid_t & tFirst = m_dCentroids[0];
		const TDigestCentroid_t & tLast = m_dCentroids[m_dCentroids.GetLength()-1];

		if ( fValue < tFirst.m_fMean )
		{
			double fDelta = tFirst.m_fMean - m_fMin;
			if ( fDelta>0.0 )
			{
				if ( fValue==m_fMin )
					return 0.5 / fTotalWeight;
				return ( 1.0 + ( fValue - m_fMin ) / fDelta * ( tFirst.m_iCount / 2.0 - 1.0 ) ) / fTotalWeight;
			}
			return 0.0;
		}

		if ( fValue > tLast.m_fMean )
		{
			double fDelta = m_fMax - tLast.m_fMean;
			if ( fDelta>0.0 )
			{
				double fDq = ( 1.0 + ( m_fMax - fValue ) / fDelta * ( tLast.m_iCount / 2.0 - 1.0 ) ) / fTotalWeight;
				return 1.0 - fDq;
			}
			return 1.0;
		}

		double fLeftWidth = ( m_dCentroids[1].m_fMean - tFirst.m_fMean ) / 2.0;
		double fWeightSoFar = 0.0;

		for ( int i = 0; i < m_dCentroids.GetLength() - 1; ++i )
		{
			const auto & tLeft = m_dCentroids[i];
			const auto & tRight = m_dCentroids[i+1];
			double fRightWidth = ( tRight.m_fMean - tLeft.m_fMean ) / 2.0;
			if ( fValue < tLeft.m_fMean + fRightWidth )
			{
				double fInterp = TdInterpolate ( fValue, tLeft.m_fMean - fLeftWidth, tLeft.m_fMean + fRightWidth );
				double fValueCdf = ( fWeightSoFar + tLeft.m_iCount * fInterp ) / fTotalWeight;
				return std::max ( fValueCdf, 0.0 );
			}
			fWeightSoFar += tLeft.m_iCount;
			fLeftWidth = fRightWidth;
		}

		double fRightWidth = ( tLast.m_fMean - m_dCentroids[m_dCentroids.GetLength()-2].m_fMean ) / 2.0;
		if ( fValue < tLast.m_fMean + fRightWidth )
		{
			double fInterp = TdInterpolate ( fValue, tLast.m_fMean - fRightWidth, tLast.m_fMean + fRightWidth );
			return ( fWeightSoFar + tLast.m_iCount * fInterp ) / fTotalWeight;
		}

		if ( fValue < m_fMax )
		{
			double fTailWidth = ( m_fMax - tLast.m_fMean );
			if ( fTailWidth>0.0 )
			{
				double fInterp = TdInterpolate ( fValue, tLast.m_fMean, tLast.m_fMean + fTailWidth );
				return ( fWeightSoFar + tLast.m_iCount + ( fInterp * ( fTotalWeight - fWeightSoFar - tLast.m_iCount ) ) ) / fTotalWeight;
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
		for ( const TDigestCentroid_t & tCentroid : tOther.m_dCentroids )
			Add ( tCentroid.m_fMean, tCentroid.m_iCount );

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
		if ( !m_dCentroids.IsEmpty() )
			Compress();
	}

	void Export ( CSphVector<TDigestCentroid_t> & dOut ) const
	{
		dOut.Resize ( 0 );
		dOut.Reserve ( m_dCentroids.GetLength() );
		for ( const TDigestCentroid_t & tCentroid : m_dCentroids )
		{
			TDigestCentroid_t & tOut = dOut.Add();
			tOut = tCentroid;
		}
	}

	void Import ( const VecTraits_T<TDigestCentroid_t> & dCentroids )
	{
		Reset();
		if ( dCentroids.IsEmpty() )
			return;

		m_dCentroids.Resize ( dCentroids.GetLength() );
		m_iCount = 0;
		for ( int i = 0; i < dCentroids.GetLength(); ++i )
		{
			m_dCentroids[i] = dCentroids[i];
			m_iCount += dCentroids[i].m_iCount;
		}
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
	CSphVector<TDigestCentroid_t>	m_dCentroids;
	int64_t							m_iCount = 0;
	double							m_fCompression = 200.0;
	double							m_fMin = std::numeric_limits<double>::infinity();
	double							m_fMax = -std::numeric_limits<double>::infinity();

	void UpdateExtremes ( double fValue )
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
	}

	void InsertCentroid ( double fMean, int64_t iCount )
	{
		TDigestCentroid_t tCentroid { fMean, iCount };
		InsertCentroid ( tCentroid );
	}

	void InsertCentroid ( const TDigestCentroid_t & tCentroid )
	{
		int iPos = LowerBoundIndex ( tCentroid.m_fMean );
		m_dCentroids.Insert ( iPos, tCentroid );
	}

	int LowerBoundIndex ( double fMean ) const
	{
		int iLeft = 0;
		int iRight = m_dCentroids.GetLength();
		while ( iLeft < iRight )
		{
			int iMid = ( iLeft + iRight ) / 2;
			if ( m_dCentroids[iMid].m_fMean < fMean )
				iLeft = iMid + 1;
			else
				iRight = iMid;
		}
		return iLeft;
	}

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
		m_dCentroids.Reset();
		m_iCount = 0;
		m_fMin = std::numeric_limits<double>::infinity();
		m_fMax = -std::numeric_limits<double>::infinity();
	}

	void Compress()
	{
		CSphVector<TDigestCentroid_t> dValues;
		dValues.Reserve ( m_dCentroids.GetLength() );
		dValues.Append ( m_dCentroids );

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
			dValues.RemoveFast ( iValue );
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
