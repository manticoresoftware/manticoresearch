//
// Copyright (c) 2024-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxsort.h"

ISphMatchSorter * CreateScrollSorter ( ISphMatchSorter * pSorter, const ISphSchema & tSchema, ESphSortFunc eSortFunc, const ScrollSettings_t & tScroll, bool bMulti );
bool ParseScroll ( CSphQuery & tQuery, const CSphString & sVal, CSphString & sError );
bool SetupScroll ( CSphQuery & tQuery, CSphString & sError );
