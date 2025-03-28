//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "config.h"

#if __has_include( <valgrind/valgrind.h>)
#define BOOST_USE_VALGRIND 1
#include <valgrind/valgrind.h>
#else
#undef BOOST_USE_VALGRIND
#endif

#include <boost/context/stack_context.hpp>
#include "std/vector.h"
#include "std/ints.h"

namespace Threads
{

static constexpr size_t STACK_ALIGN = 16;					  // stack align - let it be 16 bytes for convenience
static constexpr size_t DEFAULT_CORO_STACK_SIZE = 1024 * 128; // stack size - 128K

enum class StackFlavour_E { fixedsize, protected_fixedsize, mocked_prealloc }; // what allocator is in game
using CoroStack_t = std::pair<boost::context::stack_context, StackFlavour_E>;

CoroStack_t AllocateStack ( size_t iSize );
CoroStack_t MockedStack ( VecTraits_T<BYTE> dStack );
void DeallocateStack ( CoroStack_t tStack );

} // namespace Threads
