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
		auto & tEntry = m_dBuffer.Add();
		tEntry.m_fMean = fValue;
		tEntry.m_iCount = iWeight;

		m_iCount += iWeight;

		if ( m_dBuffer.GetLength() >= m_iBufferLimit )
			FlushBuffer();
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
		EnsureFlushed();

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
		EnsureFlushed();

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
		tOther.EnsureFlushed();
		EnsureFlushed();

		if ( !tOther.m_iCount )
			return;

		MergeExtremes ( tOther.m_fMin, tOther.m_fMax );
		m_iCount += tOther.m_iCount;

		for ( const TDigestCentroid_t & tCentroid : tOther.m_dCentroids )
		{
			auto & tEntry = m_dBuffer.Add();
			tEntry = tCentroid;

			if ( m_dBuffer.GetLength() >= m_iBufferLimit )
				FlushBuffer();
		}

		FlushBuffer();
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
		m_iBufferLimit = BufferLimit();
		if ( !m_dCentroids.IsEmpty() )
			Compress();
	}
	void Export ( CSphVector<TDigestCentroid_t> & dOut ) const
	{
		EnsureFlushed();

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

		std::sort ( m_dCentroids.Begin(), m_dCentroids.End(),
			[] ( const TDigestCentroid_t & a, const TDigestCentroid_t & b ) { return a.m_fMean < b.m_fMean; } );
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
	CSphVector<TDigestCentroid_t>	m_dBuffer;
	CSphVector<TDigestCentroid_t>	m_dTemp;
	CSphVector<TDigestCentroid_t>	m_dScratch;
	int64_t							m_iCount = 0;
	double							m_fCompression = 200.0;
	int								m_iBufferLimit = 0;
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

	void MergeExtremes ( double fMin, double fMax )
	{
		if ( std::isinf ( fMin ) )
			return;

		if ( std::isinf ( m_fMin ) )
		{
			m_fMin = fMin;
			m_fMax = fMax;
		}
		else
		{
			m_fMin = std::min ( m_fMin, fMin );
			m_fMax = std::max ( m_fMax, fMax );
		}
	}

	static double WeightedAvg ( double fX1, int64_t iW1, double fX2, int64_t iW2 )
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
		m_dBuffer.Reset();
		m_dTemp.Reset();
		m_dScratch.Reset();
		m_iCount = 0;
		m_fMin = std::numeric_limits<double>::infinity();
		m_fMax = -std::numeric_limits<double>::infinity();
		m_iBufferLimit = BufferLimit();
	}

	void Compress()
	{
		FlushBuffer();
		RebuildFromSorted ( m_dCentroids );
	}

	void FlushBuffer()
	{
		if ( m_dBuffer.IsEmpty() )
			return;

		std::sort ( m_dBuffer.Begin(), m_dBuffer.End(),
			[] ( const TDigestCentroid_t & a, const TDigestCentroid_t & b ) { return a.m_fMean < b.m_fMean; } );

		m_dTemp.Resize ( 0 );
		m_dTemp.Reserve ( m_dCentroids.GetLength() + m_dBuffer.GetLength() );
		MergeSortedIntoTemp();
		m_dBuffer.Resize ( 0 );
		RebuildFromSorted ( m_dTemp );
		m_dTemp.Resize ( 0 );
	}

	void EnsureFlushed() const
	{
		if ( !m_dBuffer.IsEmpty() )
			const_cast<Impl_c*>(this)->FlushBuffer();
	}

	void MergeSortedIntoTemp()
	{
		int i = 0;
		int j = 0;
		while ( i < m_dCentroids.GetLength() && j < m_dBuffer.GetLength() )
		{
			if ( m_dCentroids[i].m_fMean <= m_dBuffer[j].m_fMean )
				m_dTemp.Add ( m_dCentroids[i++] );
			else
				m_dTemp.Add ( m_dBuffer[j++] );
		}

		while ( i < m_dCentroids.GetLength() )
			m_dTemp.Add ( m_dCentroids[i++] );

		while ( j < m_dBuffer.GetLength() )
			m_dTemp.Add ( m_dBuffer[j++] );
	}

	void RebuildFromSorted ( CSphVector<TDigestCentroid_t> & dSorted )
	{
		if ( dSorted.IsEmpty() )
		{
			m_dCentroids.Reset();
			return;
		}

		const double fTotal = (double)m_iCount;
		if ( fTotal<=0.0 )
		{
			m_dCentroids.SwapData ( dSorted );
			return;
		}

		m_dScratch.Resize ( 0 );
		m_dScratch.Reserve ( dSorted.GetLength() );

		double fCumulative = 0.0;
		TDigestCentroid_t tCurrent = dSorted[0];

		for ( int i = 1; i < dSorted.GetLength(); ++i )
		{
			const TDigestCentroid_t & tNext = dSorted[i];
			if ( CanMerge ( tCurrent, tNext, fCumulative, fTotal ) )
			{
				MergeInto ( tCurrent, tNext );
			}
			else
			{
				m_dScratch.Add ( tCurrent );
				fCumulative += tCurrent.m_iCount;
				tCurrent = tNext;
			}
		}

		m_dScratch.Add ( tCurrent );
		m_dCentroids.SwapData ( m_dScratch );
	}

	bool CanMerge ( const TDigestCentroid_t & tCurrent, const TDigestCentroid_t & tNext, double fCumulative, double fTotal ) const
	{
		if ( fTotal<=0.0 )
			return true;

		const double fProposed = (double)tCurrent.m_iCount + (double)tNext.m_iCount;
		const double fQ = ( fCumulative + fProposed / 2.0 ) / fTotal;
		const double fLimit = MaxCentroidWeight ( fQ );
		return fProposed <= fLimit;
	}

	void MergeInto ( TDigestCentroid_t & tDst, const TDigestCentroid_t & tSrc )
	{
		tDst.m_fMean = WeightedAvg ( tDst.m_fMean, tDst.m_iCount, tSrc.m_fMean, tSrc.m_iCount );
		tDst.m_iCount += tSrc.m_iCount;
	}

	double MaxCentroidWeight ( double fQ ) const
	{
		if ( m_fCompression<=0.0 )
			return (double)m_iCount;

		double fLimit = 4.0 * (double)m_iCount * fQ * ( 1.0 - fQ ) / m_fCompression;
		return std::max ( fLimit, 1.0 );
	}

	int BufferLimit() const
	{
		return std::max ( 32, (int)std::ceil ( m_fCompression * 8.0 ) );
	}
};


TDigest_c::TDigest_c ( double fCompression )
		: m_pImpl { std::make_unique<TDigest_c::Impl_c> ( fCompression ) }
{}


TDigest_c::~TDigest_c () = default;

TDigest_c::TDigest_c ( TDigest_c && ) noexcept = default;
TDigest_c & TDigest_c::operator= ( TDigest_c && ) noexcept = default;

TDigest_c::TDigest_c ( const TDigest_c & rhs )
{
	if ( rhs.m_pImpl )
		m_pImpl = std::make_unique<TDigest_c::Impl_c> ( *rhs.m_pImpl );
	else
		m_pImpl.reset();
}

TDigest_c & TDigest_c::operator= ( const TDigest_c & rhs )
{
	if ( this==&rhs )
		return *this;

	if ( rhs.m_pImpl )
		m_pImpl = std::make_unique<TDigest_c::Impl_c> ( *rhs.m_pImpl );
	else
		m_pImpl.reset();

	return *this;
}


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
