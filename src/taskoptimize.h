//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//
/// @file taskoptimize.h
/// On-demand index optimization

#pragma once

#include "sphinxrt.h"

void RunOptimizeRtIndex ( OptimizeTask_t tTask );

// link OptimizeRtIndex with rt indexes (using strong link requires task to be available in indexer/indextool/converter/whatever, but we want serve only from searchd)
void ServeAutoOptimize();
