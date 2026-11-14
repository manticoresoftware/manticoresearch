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
#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace
{
struct PopcntTraits_t : ScalarTraits_t
{
	static Vec_t Count ( Vec_t u )
	{
#if defined(_MSC_VER)
		return __popcnt64(u);
#else
		return __builtin_popcountll(u);
#endif
	}
};
}

const bitmap::Operations_t bitmap::detail::g_tPopcnt = MakeOperations<PopcntTraits_t> ( bitmap::Backend_e::POPCNT, "popcnt" );
