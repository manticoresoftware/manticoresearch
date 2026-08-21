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

// Coroutine stack size.
//   Release builds: 128 KiB — sufficient because release-mode frame sizes
//     are 3–5× smaller (inlining, SROA, dead-store elimination).
//   Debug builds: 1 MiB — Rust's debug codegen and C++'s un-optimised debug
//     frames cumulatively blow a 128 KiB budget when the daemon calls into
//     any non-trivial C dependency (e.g. embeddings BERT forward via dlopen).
//     The overflow silently corrupts adjacent memory and manifests later as
//     a glibc heap abort in unrelated code paths (test_481/490/508). 1 MiB
//     gives comfortable headroom for any reasonable call chain.
//
// CMake defines NDEBUG for release-type builds and leaves it undefined for
// debug, so this conditional aligns with CMAKE_BUILD_TYPE without touching
// CMakeLists.txt.
#ifdef NDEBUG
static constexpr size_t DEFAULT_CORO_STACK_SIZE = 1024 * 128; // 128 KiB (release)
#else
static constexpr size_t DEFAULT_CORO_STACK_SIZE = 256 * 1024; // 1 MiB (debug)
#endif

enum class StackFlavour_E { fixedsize, protected_fixedsize, mocked_prealloc }; // what allocator is in game
using CoroStack_t = std::pair<boost::context::stack_context, StackFlavour_E>;

CoroStack_t AllocateStack ( size_t iSize );
CoroStack_t MockedStack ( VecTraits_T<BYTE> dStack );
void DeallocateStack ( CoroStack_t tStack );

} // namespace Threads
