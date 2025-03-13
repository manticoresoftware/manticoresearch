//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _columnarsort_
#define _columnarsort_

#include "sphinxsort.h"

ISphMatchSorter * CreateColumnarProxySorter ( ISphMatchSorter * pSorter, int iMaxMatches, const ISphSchema & tSchema, const CSphMatchComparatorState & tState, ESphSortFunc eSortFunc, bool bNeedFactors, bool bComputeItems, bool bMulti );

#endif // _columnarsort_
