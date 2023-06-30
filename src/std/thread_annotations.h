//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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


/////////////////////////////////////////////////////////////////////////////
// THREAD ANNOTATIONS
/////////////////////////////////////////////////////////////////////////////

#if defined( __clang__ )
#define THREAD_ANNOTATION_ATTRIBUTE__( x ) __attribute__ ( ( x ) )
#else
#define THREAD_ANNOTATION_ATTRIBUTE__( x ) // no-op
#endif

#define CAPABILITY( x ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( capability ( x ) )

#define SCOPED_CAPABILITY \
    THREAD_ANNOTATION_ATTRIBUTE__ ( scoped_lockable )

#define GUARDED_BY( x ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( guarded_by ( x ) )

#define PT_GUARDED_BY( x ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( pt_guarded_by ( x ) )

#define ACQUIRED_BEFORE( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( acquired_before ( __VA_ARGS__ ) )

#define ACQUIRED_AFTER( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( acquired_after ( __VA_ARGS__ ) )

#define REQUIRES( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( requires_capability ( __VA_ARGS__ ) )

#define REQUIRES_SHARED( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( requires_shared_capability ( __VA_ARGS__ ) )

#define ACQUIRE( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( acquire_capability ( __VA_ARGS__ ) )

#define ACQUIRE_SHARED( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( acquire_shared_capability ( __VA_ARGS__ ) )

#define RELEASE( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( release_capability ( __VA_ARGS__ ) )

#define RELEASE_SHARED( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( release_shared_capability ( __VA_ARGS__ ) )

#define TRY_ACQUIRE( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( try_acquire_capability ( __VA_ARGS__ ) )

#define TRY_ACQUIRE_SHARED( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( try_acquire_shared_capability ( __VA_ARGS__ ) )

#define EXCLUDES( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( locks_excluded ( __VA_ARGS__ ) )

#define ASSERT_CAPABILITY( x ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( assert_capability ( x ) )

#define ASSERT_SHARED_CAPABILITY( x ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( assert_shared_capability ( x ) )

#define RETURN_CAPABILITY( x ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( lock_returned ( x ) )

#define NO_THREAD_SAFETY_ANALYSIS \
    THREAD_ANNOTATION_ATTRIBUTE__ ( no_thread_safety_analysis )

// Replaced by TRY_ACQUIRE
#define EXCLUSIVE_TRYLOCK_FUNCTION( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( exclusive_trylock_function ( __VA_ARGS__ ) )

// Replaced by TRY_ACQUIRE_SHARED
#define SHARED_TRYLOCK_FUNCTION( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( shared_trylock_function ( __VA_ARGS__ ) )

// Replaced by RELEASE and RELEASE_SHARED
#define UNLOCK_FUNCTION( ... ) \
    THREAD_ANNOTATION_ATTRIBUTE__ ( unlock_function ( __VA_ARGS__ ) )

#if defined( __clang__ )
#define LIKELY [[likely]]
#define UNLIKELY [[unlikely]]
#else
#define LIKELY
#define UNLIKELY
#endif