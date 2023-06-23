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

#include <utility>


// switch off clang-specific warning about non-necessary capturing of constants
#ifdef __clang__
_Pragma ( "clang diagnostic ignored \"-Wunused-lambda-capture\"" )
#endif


	/////////////////////////////////////////////////////////////////////////////
// GENERICS
/////////////////////////////////////////////////////////////////////////////

template<typename T>
T Min ( T a, T b )
{
	return a < b ? a : b;
}
template<typename T, typename U>
typename std::common_type<T, U>::type Min ( T a, U b )
{
	using common_type = typename std::common_type<T, U>::type;
	return static_cast<common_type>(a) < static_cast<common_type>(b) ? a : b;
}
template<typename T>
T Max ( T a, T b )
{
	return a < b ? b : a;
}
template<typename T, typename U>
typename std::common_type<T, U>::type Max ( T a, U b )
{
	using common_type = typename std::common_type<T, U>::type;
	return static_cast<common_type>(a) < static_cast<common_type>(b) ? b : a;
}

inline int sphRoundUp ( int iValue, int iLimit )
{
	return ( iValue + iLimit - 1 ) & ~( iLimit - 1 );
}

#define SafeDelete( _x )		do { if ( _x ) { delete ( _x ); ( _x ) = nullptr; } } while ( 0 )
#define SafeDeleteArray( _x )	do { if ( _x ) { delete[] ( _x ); ( _x ) = nullptr; } } while ( 0 )
#define SafeReleaseAndZero( _x)	do { if ( _x ) { ( _x )->Release(); ( _x ) = nullptr; } } while ( 0 )
#define SafeRelease( _x )		do { if ( _x ) { ( _x )->Release(); } } while ( 0 )
#define SafeAddRef( _x )        do { if ( _x ) { ( _x )->AddRef(); } } while ( 0 )

/// swap
template<typename T>
inline void Swap ( T& v1, T& v2 )
{
	T temp = std::move ( v1 );
	v1 = std::move ( v2 );
	v2 = std::move ( temp );
}

/// prevent copy
class ISphNoncopyable
{
public:
	ISphNoncopyable() = default;
	ISphNoncopyable ( const ISphNoncopyable& ) = delete;
	const ISphNoncopyable& operator= ( const ISphNoncopyable& ) = delete;
};

/// prevent move
class ISphNonmovable
{
public:
	ISphNonmovable() = default;
	ISphNonmovable ( ISphNonmovable&& ) noexcept = delete;
	ISphNonmovable& operator= ( ISphNonmovable&& ) noexcept = delete;
};

// implement moving ctr and moving= using swap-and-release
#define MOVE_BYSWAP( class_c )								\
    class_c ( class_c&& rhs ) noexcept {Swap ( rhs );}		\
    class_c& operator= ( class_c&& rhs ) noexcept			\
 		{ Swap ( rhs ); return *this;  }

// take all ctr definitions from parent
#define FWD_CTOR( type_c, base_c )							\
    template<typename... V>                         		\
    type_c ( V&&... v )										\
        : base_c { std::forward<V> ( v )... }				\
    {														\
    }

// take all ctr definitions from BASE parent
#define FWD_BASECTOR( type_c ) FWD_CTOR ( type_c, BASE )



#if _WIN32
#define DISABLE_CONST_COND_CHECK \
	__pragma ( warning ( push ) ) \
	__pragma ( warning ( disable:4127 ) )
#define ENABLE_CONST_COND_CHECK \
	__pragma ( warning ( pop ) )
#else
#define DISABLE_CONST_COND_CHECK
#define ENABLE_CONST_COND_CHECK
#endif

#define if_const(_arg) \
	DISABLE_CONST_COND_CHECK \
	if ( _arg ) \
	ENABLE_CONST_COND_CHECK

#ifndef FORCE_INLINE
	#ifndef NDEBUG
		#define FORCE_INLINE inline
	#else
		#ifdef _MSC_VER
			#define FORCE_INLINE __forceinline
		#else
			#if defined (__cplusplus) || defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L   /* C99 */
				#ifdef __GNUC__
					#define FORCE_INLINE inline __attribute__((always_inline))
				#else
					#define FORCE_INLINE inline
				#endif
			#else
				#define FORCE_INLINE
			#endif
		#endif
	#endif
#endif

#define SPH_INTERNAL_CONCAT2( a, b ) a##b
#define SPH_INTERNAL_CONCAT( a, b ) SPH_INTERNAL_CONCAT2 ( a, b )
#define SPH_UID( prefix ) SPH_INTERNAL_CONCAT ( prefix, __LINE__ )

#if _WIN32
#define ALIGNED_UNION_ZERO {0}
#else
#define ALIGNED_UNION_ZERO {{0}}
#endif

#define INIT_WITH_0( ... ) std::aligned_union_t<1, __VA_ARGS__ > SPH_UID ( m_init_ ) ALIGNED_UNION_ZERO;
