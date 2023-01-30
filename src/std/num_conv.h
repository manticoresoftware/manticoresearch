//
// Copyright (c) 2022-2023, Manticore Software LTD (https://manticoresearch.com)
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


namespace sph
{
/// my own converter num to string.
template<typename Num> // let compiler deduce whether signed or unsigned...
int NtoA ( char* pOutput, Num nVal, int iBase = 10, int iWidth = 0, int iPrec = 0, char cFill = ' ' );

/// my own fixed-point floats. iPrec - num of digits after point. i.e. 100000, 3 -> 100.000
int IFtoA ( char* pOutput, int nVal, int iPrec = 3 );
int IFtoA ( char* pOutput, int64_t nVal, int iPrec = 6 );
}

#include "num_conv_impl.h"