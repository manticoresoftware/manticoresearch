//
// Copyright (c) 2022-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "std/string.h"

CSphString FetchUrl ( const CSphString& sUrl );

std::pair<bool, CSphString> PostToHelperUrl ( CSphString sUrl, Str_t sQuery, const VecTraits_T<CSphString>& dHeaders );

void ShutdownCurl();

bool IsCurlAvailable();