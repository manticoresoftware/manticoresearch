//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "config.h"
#include <memory>

/////////////////////////////////////////////////////////////////////////////
// MEMORY MANAGEMENT
/////////////////////////////////////////////////////////////////////////////

#define SPH_DEBUG_LEAKS			0
#define SPH_ALLOC_FILL			0
#define SPH_ALLOCS_PROFILER		0
#define SPH_DEBUG_BACKTRACES 0 // will add not only file/line, but also full backtrace

#if SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER

/// debug new that tracks memory leaks
void *			operator new ( size_t iSize, const char * sFile, int iLine );

/// debug new that tracks memory leaks
void *			operator new [] ( size_t iSize, const char * sFile, int iLine );

/// debug allocate to use in custom allocator
void * debugallocate ( size_t );

/// debug deallocate to use in custom allocator
void debugdeallocate ( void * );

/// get current allocs count
int				sphAllocsCount ();

/// total allocated bytes
int64_t			sphAllocBytes ();

/// get last alloc id
int				sphAllocsLastID ();

/// dump all allocs since given id
void			sphAllocsDump ( int iFile, int iSinceID );

/// dump stats to stdout
void			sphAllocsStats ();

/// check all existing allocs; raises assertion failure in cases of errors
void			sphAllocsCheck ();

void			sphMemStatDump ( int iFD );

/// per thread cleanup of memory statistic's
void			sphMemStatThdCleanup ( void * pTLS );

void *			sphMemStatThdInit ();

void			sphMemStatMMapAdd ( int64_t iSize );
void			sphMemStatMMapDel ( int64_t iSize );

#undef new
#define new		new(__FILE__,__LINE__)
#define NEW_IS_OVERRIDED 1

#if WITH_RE2
#define MYTHROW() throw()
#else
#define MYTHROW() noexcept
#endif

/// delete for my new
void			operator delete ( void * pPtr ) MYTHROW();

/// delete for my new
void			operator delete [] ( void * pPtr ) MYTHROW();

template<typename T>
class managed_allocator
{
public:
    typedef size_t size_type;
    typedef T * pointer;
    typedef const T * const_pointer;
	typedef T value_type;

    template<typename _Tp1>
    struct rebind
    {
        typedef managed_allocator <_Tp1> other;
    };

    pointer allocate ( size_type n, const void * = 0 )
    {
		return ( T * ) debugallocate ( n * sizeof ( T ) );
    }

    void deallocate ( pointer p, size_type )
    {
		debugdeallocate (p);
    }
};
#else
template<typename T> using managed_allocator = std::allocator<T>;
#endif // SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER

/// init of memory statistic's data
void sphMemStatInit();

/// cleanup of memory statistic's data
void sphMemStatDone();
