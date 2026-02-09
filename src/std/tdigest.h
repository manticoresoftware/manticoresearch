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


#pragma once

#include "ints.h"
#include "vector.h"
#include <memory>


//////////////////////////////////////////////////////////////////////////
struct TDigestCentroid_t
{
	double		m_fMean = 0.0;
	int64_t		m_iCount = 0;
};


class TDigest_c
{
	class Impl_c;
	std::unique_ptr<Impl_c> m_pImpl;

public:
	explicit TDigest_c ( double fCompression = 200.0 );
	~TDigest_c();

	TDigest_c ( TDigest_c && ) noexcept;
	TDigest_c & operator= ( TDigest_c && ) noexcept;
	TDigest_c ( const TDigest_c & );
	TDigest_c & operator= ( const TDigest_c & );

	void Add ( double fValue, int64_t iWeight = 1 );
	void AddBulk ( const double * pValues, int iCount );
	double Percentile ( int iPercent ) const noexcept;
	double Percentile ( double fPercent ) const noexcept;
	double Quantile ( double fQuantile ) const noexcept;
	double Cdf ( double fValue ) const noexcept;
	double GetMin () const noexcept;
	double GetMax () const noexcept;
	void SetExtremes ( double fMin, double fMax, bool bHasData );

	void Clear();
	int64_t GetCount () const noexcept;

	void Merge ( const TDigest_c & tOther );

	void Export ( CSphVector<TDigestCentroid_t> & dOut ) const;
	void Import ( const VecTraits_T<TDigestCentroid_t> & dCentroids );

	void SetCompression ( double fCompression );
	double GetCompression () const noexcept;
};

std::unique_ptr<TDigest_c> sphCreateTDigest();