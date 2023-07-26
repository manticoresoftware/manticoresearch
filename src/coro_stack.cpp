//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "coro_stack.h"

#include "std/env.h"

// that is bug in protected_fixedsize on Win, this header is missed on inclusion
#include <boost/assert.hpp>

#include <boost/context/protected_fixedsize_stack.hpp>
#include <boost/context/fixedsize_stack.hpp>

namespace Threads {

static StackFlavour_E g_eStackFlavour = val_from_env ( "MANTICORE_GUARDED_STACK", false ) ? StackFlavour_E::protected_fixedsize : StackFlavour_E::fixedsize;

inline size_t AlignStackSize ( size_t iSize )
{
	return ( iSize + STACK_ALIGN - 1 ) & ~( STACK_ALIGN - 1 );
}

CoroStack_t AllocateStack ( size_t iStack )
{
	switch ( g_eStackFlavour )
	{
	case StackFlavour_E::fixedsize:
		{
			boost::context::fixedsize_stack allocator { iStack ? AlignStackSize ( iStack ) : DEFAULT_CORO_STACK_SIZE };
			return { allocator.allocate(), StackFlavour_E::fixedsize };
		}
	case StackFlavour_E::protected_fixedsize:
		{
			boost::context::protected_fixedsize_stack allocator { iStack ? AlignStackSize ( iStack ) : DEFAULT_CORO_STACK_SIZE };
			auto tStack = allocator.allocate(); tStack.size -= boost::context::protected_fixedsize_stack::traits_type::page_size(); // align guard page
			return { tStack, StackFlavour_E::protected_fixedsize };
		}
	default:
		assert(false && "should not be here");
	}
}

CoroStack_t MockedStack ( VecTraits_T<BYTE> dStack )
{
	boost::context::stack_context tStack;
	tStack.sp = &dStack.Last();
	tStack.size = dStack.GetLength();
#if defined( BOOST_USE_VALGRIND )
	tStack.valgrind_stack_id = VALGRIND_STACK_REGISTER ( dStack.begin(), tStack.sp );
#endif
	return { tStack, StackFlavour_E::mocked_prealloc };
}

void DeallocateStack ( CoroStack_t tStack )
{
	switch ( tStack.second )
	{
		case StackFlavour_E::fixedsize:
		{
			boost::context::fixedsize_stack allocator { 0 };
			allocator.deallocate ( tStack.first );
			break;
		}
		case StackFlavour_E::protected_fixedsize:
		{
			boost::context::protected_fixedsize_stack allocator { 0 };
			tStack.first.size += boost::context::protected_fixedsize_stack::traits_type::page_size(); // undo guard page align
			allocator.deallocate ( tStack.first );
			break;
		}
		case StackFlavour_E::mocked_prealloc:
		{
#if defined( BOOST_USE_VALGRIND )
			VALGRIND_STACK_DEREGISTER ( tStack.first.valgrind_stack_id );
#endif
		}
	}
}


} // namespace Threads
