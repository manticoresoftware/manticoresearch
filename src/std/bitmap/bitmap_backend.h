//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once
#include "bitmap.h"

namespace bitmap::detail
{
extern const Operations_t g_tGeneric;
extern const Operations_t g_tPopcnt;
extern const Operations_t g_tSSSE3;
extern const Operations_t g_tAVX2;
extern const Operations_t g_tAVX512;
} // namespace bitmap::detail
