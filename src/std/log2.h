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

constexpr int sphLog2const ( unsigned uValue );
constexpr int sphLog2const ( unsigned long uValue );
constexpr int sphLog2const ( unsigned long long uValue );

/// how much bits do we need for given int
int sphLog2 ( unsigned uValue );
int sphLog2 ( unsigned long uValue );
int sphLog2 ( unsigned long long uValue );

/// signed ints just casted to unsigned
int sphLog2 ( int iValue );
int sphLog2 ( long iValue );
int sphLog2 ( long long iValue );

#include "log2_impl.h"
