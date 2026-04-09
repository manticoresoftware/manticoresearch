//
// Copyright (c) 2024-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// Shared TDigest aggregation helpers used by both JSON and SQL result renderers.
//

#pragma once

#include "match.h"
#include "schema/columninfo.h"
#include "std/tdigest.h"
#include "std/tdigest_runtime.h"

#include <algorithm>
#include <cmath>

namespace tdigest_aggr
{
struct MadDeviationEntry_t
{
	double m_fDeviation = 0.0;
	int64_t m_iWeight = 0;
};

inline float GetCompression ( const CSphColumnInfo & tCol )
{
	return tCol.m_fTdigestCompression ? tCol.m_fTdigestCompression : 200.0f;
}

inline bool LoadFromMatch ( const CSphMatch & tMatch, const CSphColumnInfo & tCol, TDigest_c & tDigest )
{
	ByteBlob_t dBlob = tMatch.FetchAttrData ( tCol.m_tLocator, nullptr );
	if ( !dBlob.first )
		return false;

	sphTDigestLoadFromBlob ( dBlob, tDigest, GetCompression ( tCol ) );
	return tDigest.GetCount()>0;
}

template <typename DeviationEntry_t>
inline bool CalcMad ( const TDigest_c & tDigest, double & fMad, CSphVector<TDigestCentroid_t> & dCentroids, CSphVector<DeviationEntry_t> & dDeviations )
{
	if ( tDigest.GetCount()==0 )
		return false;

	double fMedian = tDigest.Quantile ( 0.5 );

	tDigest.Export ( dCentroids );
	if ( dCentroids.IsEmpty() )
		return false;

	dDeviations.Resize ( dCentroids.GetLength() );

	int64_t iTotalWeight = 0;
	for ( int i = 0; i < dCentroids.GetLength(); ++i )
	{
		const auto & tCentroid = dCentroids[i];
		auto & tDeviation = dDeviations[i];
		tDeviation.m_fDeviation = std::fabs ( tCentroid.m_fMean - fMedian );
		tDeviation.m_iWeight = tCentroid.m_iCount;
		iTotalWeight += tCentroid.m_iCount;
	}

	std::sort ( dDeviations.Begin(), dDeviations.End(),
		[] ( const DeviationEntry_t & a, const DeviationEntry_t & b ) { return a.m_fDeviation < b.m_fDeviation; } );

	const double fTarget = 0.5 * (double)iTotalWeight;
	double fAccumulated = 0.0;

	for ( const auto & tEntry : dDeviations )
	{
		fAccumulated += (double)tEntry.m_iWeight;
		if ( fAccumulated>=fTarget )
		{
			fMad = tEntry.m_fDeviation;
			return true;
		}
	}

	fMad = dDeviations.Last().m_fDeviation;
	return true;
}

inline bool CalcMad ( const TDigest_c & tDigest, double & fMad )
{
	CSphVector<TDigestCentroid_t> dCentroids;
	CSphVector<MadDeviationEntry_t> dDeviations;
	return CalcMad ( tDigest, fMad, dCentroids, dDeviations );
}

inline CSphString FormatNumeric ( double fValue )
{
	CSphString sValue;
	sValue.SetSprintf ( "%.15g", fValue );
	return sValue;
}

inline CSphString FormatKey ( double fValue )
{
	CSphString sKey;
	sKey.SetSprintf ( "%.12g", fValue );
	return sKey;
}

} // namespace tdigest_aggr

