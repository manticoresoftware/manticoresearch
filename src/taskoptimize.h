//
// Copyright (c) 2019, Manticore Software LTD (http://manticoresearch.com)
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

void EnqueueForOptimize ( CSphString sIndex );

#endif //MANTICORE_TASKOPTIMIZE_H
