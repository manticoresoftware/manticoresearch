//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

/////////////////////////////////////////////////////////////////////////////
// HELPERS
/////////////////////////////////////////////////////////////////////////////

// make percent honouring fixed-point floats
// sprintf ( "%.2D", MakePercent (1.014,10,2) ) will print "10.14"
template<typename NUM>
int64_t PercentOf ( NUM tVal, NUM tBase, int iFloatDigits = 1 );


#include "helpers_impl.h"