//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
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

#ifndef MANTICORE_TASKOPTIMIZE_H
#define MANTICORE_TASKOPTIMIZE_H

#include "sphinxstd.h"

void EnqueueForOptimize ( CSphString sIndex, int iCutoff = 0, int iFrom = -1, int iTo = -1, const char* szUvarFilter = nullptr, bool byOrder = true );

#endif //MANTICORE_TASKOPTIMIZE_H
