//
// Copyright (c) 2020-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _columnarfilter_
#define _columnarfilter_

#include "sphinxfilter.h"

std::unique_ptr<ISphFilter>	TryToCreateColumnarFilter ( int iAttr, const ISphSchema & tSchema, const CSphFilterSettings & tSettings, const CommonFilterSettings_t & tFixedSettings, ESphCollation eCollation, CSphString & sError, CSphString & sWarning );
bool AddColumnarFilter ( std::vector<common::Filter_t> & dDst, const CSphFilterSettings & tSrc, ESphCollation eCollation, const ISphSchema & tSchema, CSphString & sWarning );
bool ToColumnarFilter ( common::Filter_t & tFilter, const CSphFilterSettings & tSrc, ESphCollation eCollation, const ISphSchema & tSchema, CSphString & sWarning );
void ToColumnarFilters ( VecTraits_T<const CSphFilterSettings> & dFilters, std::vector<common::Filter_t> & dColumnarFilters, std::vector<int> & dFilterMap, const ISphSchema & tSchema, ESphCollation eCollation, CSphString & sWarning );

#endif // _columnarfilter_
