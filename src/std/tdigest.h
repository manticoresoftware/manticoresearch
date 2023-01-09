//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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
#include <memory>


//////////////////////////////////////////////////////////////////////////
class TDigest_i
{
public:
	virtual ~TDigest_i() {}

	virtual void Add ( double fValue, int64_t iWeight = 1 ) = 0;
	virtual double Percentile ( int iPercent ) const = 0;
};

std::unique_ptr<TDigest_i> sphCreateTDigest();