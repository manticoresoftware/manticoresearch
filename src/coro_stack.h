//
// Copyright (c) 2021-2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#if __has_include( <valgrind/valgrind.h>)
#define BOOST_USE_VALGRIND 1
#include <valgrind/valgrind.h>
#else
#undef BOOST_USE_VALGRIND
#endif

#include <boost/context/stack_context.hpp>
#include "std/ints.h"
#include "std/vectraits.h"

namespace Threads
{

#if defined(__has_feature)
#   if __has_feature(address_sanitizer) // for clang
#       define __SANITIZE_ADDRESS__ // GCC already sets this
#   endif
#endif

static constexpr size_t STACK_ALIGN = 16;					  // stack align - let it be 16 bytes for convenience
// Coroutine stack size. Previously 128 KiB, which is too small to safely run
// any non-trivial C dependency in *debug* builds — Rust's debug codegen and
// C++'s un-optimised debug frames cumulatively exceed 128 KiB when the
// daemon calls into the embeddings library's BERT forward pass. The overflow
// silently corrupts adjacent memory and manifests later as a glibc heap
// abort in unrelated code paths (test_481/490/508). Bumped to 1 MiB to give
// the call chain comfortable headroom. Release builds were unaffected by
// the smaller value; the bigger budget is "free" on 64-bit systems because
// page mappings are committed on first touch, so unused tail of the stack
// costs nothing.
static constexpr size_t DEFAULT_CORO_STACK_SIZE = 1024 * 1024; // stack size - 1 MiB

enum class StackFlavour_E { fixedsize, protected_fixedsize, mocked_prealloc }; // what allocator is in game
using CoroStack_t = std::pair<boost::context::stack_context, StackFlavour_E>;

CoroStack_t AllocateStack ( size_t iSize );
CoroStack_t MockedStack ( VecTraits_T<BYTE> dStack );
void DeallocateStack ( CoroStack_t tStack );

} // namespace Threads
