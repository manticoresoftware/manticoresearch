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

/// generic binary search
template<typename T, typename U, typename PRED>
T* sphBinarySearch ( T* pStart, T* pEnd, PRED&& tPred, U tRef );

template<typename T>
T* sphBinarySearch ( T* pStart, T* pEnd, T& tRef );

// returns first (leftmost) occurrence of the value
// returns -1 if not found
template<typename T, typename U, typename PRED>
int sphBinarySearchFirst ( T* pValues, int iStart, int iEnd, PRED&& tPred, U tRef );

template<typename T, typename U, typename PRED>
T* sphBinarySearchFirst ( T* pStart, T* pEnd, PRED&& tPred, U tRef );

#include "binarysearch_impl.h"
