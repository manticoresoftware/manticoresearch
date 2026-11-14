//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "bitmap_backend.h"
#include "bitmap_kernels.h"

const bitmap::Operations_t bitmap::detail::g_tGeneric = MakeOperations<ScalarTraits_t> ( bitmap::Backend_e::GENERIC, "generic" );
