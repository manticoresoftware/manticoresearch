//
// $Id$
//

//
// Copyright (c) 2001-2012, Andrew Aksyonoff
// Copyright (c) 2008-2012, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxstd_
#define _sphinxstd_

#if _MSC_VER>=1400
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

#if (_MSC_VER>=1000) && !defined(__midl) && defined(_PREFAST_)
typedef int __declspec("SAL_nokernel") __declspec("SAL_nodriver") __prefast_flag_kernel_driver_mode;
#endif

#if defined(_MSC_VER) && (_MSC_VER<1400)
#define vsnprintf _vsnprintf
#endif

#ifndef __GNUC__
#define __attribute__(x)
#endif

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>

// for 64-bit types
#if HAVE_STDINT_H
#include <stdint.h>
#endif

#if HAVE_INTTYPES_H
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifndef USE_WINDOWS
#ifdef _MSC_VER
#define USE_WINDOWS 1
#else
#define USE_WINDOWS 0
#endif // _MSC_VER
#endif

#if !USE_WINDOWS
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
#ifdef __FreeBSD__
#include <semaphore.h>
#endif
#endif

/////////////////////////////////////////////////////////////////////////////
// COMPILE-TIME CHECKS
/////////////////////////////////////////////////////////////////////////////

#define STATIC_ASSERT(_cond,_name)		typedef char STATIC_ASSERT_FAILED_ ## _name [ (_cond) ? 1 : -1 ]
#define STATIC_SIZE_ASSERT(_type,_size)	STATIC_ASSERT ( sizeof(_type)==_size, _type ## _MUST_BE_ ## _size ## _BYTES )


#ifndef __analysis_assume
#define __analysis_assume(_arg)
#endif

/////////////////////////////////////////////////////////////////////////////
// PORTABILITY
/////////////////////////////////////////////////////////////////////////////

#if _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <intrin.h> // for bsr
#pragma intrinsic(_BitScanReverse)

#define strcasecmp			strcmpi
#define strncasecmp			_strnicmp
#define snprintf			_snprintf
#define strtoll				_strtoi64
#define strtoull			_strtoui64

#else

#if USE_ODBC
// UnixODBC compatible DWORD
#if defined(__alpha) || defined(__sparcv9) || defined(__LP64__) || (defined(__HOS_AIX__) && defined(_LP64)) || defined(__APPLE__)
typedef unsigned int		DWORD;
#else
typedef unsigned long		DWORD;
#endif
#else
// default DWORD
typedef unsigned int		DWORD;
#endif // USE_ODBC

typedef unsigned short		WORD;
typedef unsigned char		BYTE;

#endif // _WIN32

/////////////////////////////////////////////////////////////////////////////
// 64-BIT INTEGER TYPES AND MACROS
/////////////////////////////////////////////////////////////////////////////

#if defined(U64C) || defined(I64C)
#error "Internal 64-bit integer macros already defined."
#endif

#if !HAVE_STDINT_H

#if defined(_MSC_VER)
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#define U64C(v) v ## UI64
#define I64C(v) v ## I64
#define PRIu64 "I64d"
#define PRIi64 "I64d"
#else // !defined(_MSC_VER)
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif // !defined(_MSC_VER)

#endif // no stdint.h

// if platform-specific macros were not supplied, use common defaults
#ifndef U64C
#define U64C(v) v ## ULL
#endif

#ifndef I64C
#define I64C(v) v ## LL
#endif

#ifndef PRIu64
#define PRIu64 "llu"
#endif

#ifndef PRIi64
#define PRIi64 "lld"
#endif

#define UINT64_FMT "%" PRIu64
#define INT64_FMT "%" PRIi64

#ifndef UINT64_MAX
#define UINT64_MAX U64C(0xffffffffffffffff)
#endif

#ifndef INT64_MAX
#define INT64_MAX I64C(0x7fffffffffffffff)
#endif

STATIC_SIZE_ASSERT ( uint64_t, 8 );
STATIC_SIZE_ASSERT ( int64_t, 8 );

// conversion macros that suppress %lld format warnings vs printf
// problem is, on 64-bit Linux systems with gcc and stdint.h, int64_t is long int
// and despite sizeof(long int)==sizeof(long long int)==8, gcc bitches about that
// using PRIi64 instead of %lld is of course The Right Way, but ugly like fuck
// so lets wrap them args in INT64() instead
#define INT64(_v) ((long long int)(_v))
#define UINT64(_v) ((unsigned long long int)(_v))

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

void			sphMemStatMMapAdd ( int64_t iSize );
void			sphMemStatMMapDel ( int64_t iSize );

#undef new
#define new		new(__FILE__,__LINE__)

#if USE_RE2
void			operator delete ( void * pPtr ) throw ();
void			operator delete [] ( void * pPtr ) throw ();
#else
/// delete for my new
void			operator delete ( void * pPtr );

/// delete for my new
void			operator delete [] ( void * pPtr );
#endif
#endif // SPH_DEBUG_LEAKS || SPH_ALLOCS_PROFILER

/////////////////////////////////////////////////////////////////////////////
// HELPERS
/////////////////////////////////////////////////////////////////////////////

inline int sphBitCount ( DWORD n )
{
	// MIT HACKMEM count
	// works for 32-bit numbers only
	// fix last line for 64-bit numbers
	register DWORD tmp;
	tmp = n - ((n >> 1) & 033333333333) - ((n >> 2) & 011111111111);
	return ( (tmp + (tmp >> 3) ) & 030707070707) % 63;
}

typedef			bool ( *SphDieCallback_t ) ( const char * );

/// crash with an error message
void			sphDie ( const char * sMessage, ... ) __attribute__ ( ( format ( printf, 1, 2 ) ) );

/// setup a callback function to call from sphDie() before exit
/// if callback returns false, sphDie() will not log to stdout
void			sphSetDieCallback ( SphDieCallback_t pfDieCallback );

/// how much bits do we need for given int
inline int sphLog2 ( uint64_t uValue )
{
#if USE_WINDOWS
	DWORD uRes;
	if ( BitScanReverse ( &uRes, (DWORD)( uValue>>32 ) ) )
		return 33+uRes;
	BitScanReverse ( &uRes, DWORD(uValue) );
	return 1+uRes;
#elif __GNUC__
	if ( !uValue )
		return 0;
	return 64 - __builtin_clzl(uValue);
#else
	int iBits = 0;
	while ( uValue )
	{
		uValue >>= 1;
		iBits++;
	}
	return iBits;
#endif
}

/// float vs dword conversion
inline DWORD sphF2DW ( float f )	{ union { float f; DWORD d; } u; u.f = f; return u.d; }

/// dword vs float conversion
inline float sphDW2F ( DWORD d )	{ union { float f; DWORD d; } u; u.d = d; return u.f; }

/// double to bigint conversion
inline uint64_t sphD2QW ( double f )	{ union { double f; uint64_t d; } u; u.f = f; return u.d; }

/// bigint to double conversion
inline double sphQW2D ( uint64_t d )	{ union { double f; uint64_t d; } u; u.d = d; return u.f; }

/// microsecond precision timestamp
/// current UNIX timestamp in seconds multiplied by 1000000, plus microseconds since the beginning of current second
int64_t		sphMicroTimer ();

//////////////////////////////////////////////////////////////////////////
// RANDOM NUMBERS GENERATOR
//////////////////////////////////////////////////////////////////////////

/// seed RNG
void		sphSrand ( DWORD uSeed );

/// auto-seed RNG based on time and PID
void		sphAutoSrand ();

/// generate another random
DWORD		sphRand ();

/////////////////////////////////////////////////////////////////////////////
// DEBUGGING
/////////////////////////////////////////////////////////////////////////////

#if USE_WINDOWS
#ifndef NDEBUG

void sphAssert ( const char * sExpr, const char * sFile, int iLine );

#undef assert
#define assert(_expr) (void)( (_expr) || ( sphAssert ( #_expr, __FILE__, __LINE__ ), 0 ) )

#endif // !NDEBUG
#endif // USE_WINDOWS


#ifndef NDEBUG
#define Verify(_expr) assert(_expr)
#else
#define Verify(_expr) _expr
#endif

/////////////////////////////////////////////////////////////////////////////
// GENERICS
/////////////////////////////////////////////////////////////////////////////

#define Min(a,b)			((a)<(b)?(a):(b))
#define Max(a,b)			((a)>(b)?(a):(b))
#define SafeDelete(_x)		{ if (_x) { delete (_x); (_x) = NULL; } }
#define SafeDeleteArray(_x)	{ if (_x) { delete [] (_x); (_x) = NULL; } }
#define SafeRelease(_x)		{ if (_x) { (_x)->Release(); (_x) = NULL; } }

/// swap
template < typename T > inline void Swap ( T & v1, T & v2 )
{
	T temp = v1;
	v1 = v2;
	v2 = temp;
}

/// prevent copy
class ISphNoncopyable
{
public:
								ISphNoncopyable () {}

private:
								ISphNoncopyable ( const ISphNoncopyable & ) {}
	const ISphNoncopyable &		operator = ( const ISphNoncopyable & ) { return *this; }
};

//////////////////////////////////////////////////////////////////////////////

/// generic comparator
template < typename T >
struct SphLess_T
{
	inline bool IsLess ( const T & a, const T & b ) const
	{
		return a < b;
	}
};


/// generic comparator
template < typename T >
struct SphGreater_T
{
	inline bool IsLess ( const T & a, const T & b ) const
	{
		return b < a;
	}
};


/// generic comparator
template < typename T, typename C >
struct SphMemberLess_T
{
	const T C::* m_pMember;

	explicit SphMemberLess_T ( T C::* pMember )
		: m_pMember ( pMember )
	{}

	inline bool IsLess ( const C & a, const C & b ) const
	{
		return ( (&a)->*m_pMember ) < ( (&b)->*m_pMember );
	}
};

template < typename T, typename C >
inline SphMemberLess_T<T,C>
sphMemberLess ( T C::* pMember )
{
	return SphMemberLess_T<T,C> ( pMember );
}


/// generic accessor
template < typename T >
struct SphAccessor_T
{
	typedef T MEDIAN_TYPE;

	MEDIAN_TYPE & Key ( T * a ) const
	{
		return *a;
	}

	void CopyKey ( MEDIAN_TYPE * pMed, T * pVal ) const
	{
		*pMed = Key(pVal);
	}

	void Swap ( T * a, T * b ) const
	{
		::Swap ( *a, *b );
	}

	T * Add ( T * p, int i ) const
	{
		return p+i;
	}

	int Sub ( T * b, T * a ) const
	{
		return (int)(b-a);
	}
};


/// heap sort helper
template < typename T, typename U, typename V >
void sphSiftDown ( T * pData, int iStart, int iEnd, U COMP, V ACC )
{
	for ( ;; )
	{
		int iChild = iStart*2+1;
		if ( iChild>iEnd )
			break;

		int iChild1 = iChild+1;
		if ( iChild1<=iEnd && COMP.IsLess ( ACC.Key ( ACC.Add ( pData, iChild ) ), ACC.Key ( ACC.Add ( pData, iChild1 ) ) ) )
			iChild = iChild1;

		if ( COMP.IsLess ( ACC.Key ( ACC.Add ( pData, iChild ) ), ACC.Key ( ACC.Add ( pData, iStart ) ) ) )
			return;
		ACC.Swap ( ACC.Add ( pData, iChild ), ACC.Add ( pData, iStart ) );
		iStart = iChild;
	}
}


/// heap sort
template < typename T, typename U, typename V >
void sphHeapSort ( T * pData, int iCount, U COMP, V ACC )
{
	if ( !pData || iCount<=1 )
		return;

	// build a max-heap, so that the largest element is root
	for ( int iStart=( iCount-2 )>>1; iStart>=0; iStart-- )
		sphSiftDown ( pData, iStart, iCount-1, COMP, ACC );

	// now keep popping root into the end of array
	for ( int iEnd=iCount-1; iEnd>0; )
	{
		ACC.Swap ( pData, ACC.Add ( pData, iEnd ) );
		sphSiftDown ( pData, 0, --iEnd, COMP, ACC );
	}
}


/// generic sort
template < typename T, typename U, typename V >
void sphSort ( T * pData, int iCount, U COMP, V ACC )
{
	if ( iCount<2 )
		return;

	typedef T * P;
	P st0[32], st1[32], a, b, i, j;
	typename V::MEDIAN_TYPE x;
	int k;

	const int SMALL_THRESH = 32;
	int iDepthLimit = sphLog2 ( iCount );
	iDepthLimit = ( ( iDepthLimit<<2 ) + iDepthLimit ) >> 1; // x2.5

	k = 1;
	st0[0] = pData;
	st1[0] = ACC.Add ( pData, iCount-1 );
	while ( k )
	{
		k--;
		i = a = st0[k];
		j = b = st1[k];

		// if quicksort fails on this data; switch to heapsort
		if ( !k )
		{
			if ( !--iDepthLimit )
			{
				sphHeapSort ( a, ACC.Sub ( b, a )+1, COMP, ACC );
				return;
			}
		}

		// for tiny arrays, switch to insertion sort
		int iLen = ACC.Sub ( b, a );
		if ( iLen<=SMALL_THRESH )
		{
			for ( i=ACC.Add ( a, 1 ); i<=b; i=ACC.Add ( i, 1 ) )
			{
				for ( j=i; j>a; )
				{
					P j1 = ACC.Add ( j, -1 );
					if ( COMP.IsLess ( ACC.Key(j1), ACC.Key(j) ) )
						break;
					ACC.Swap ( j, j1 );
					j = j1;
				}
			}
			continue;
		}

		ACC.CopyKey ( &x, ACC.Add ( a, iLen/2 ) );
		while ( a<b )
		{
			while ( i<=j )
			{
				while ( COMP.IsLess ( ACC.Key(i), x ) )
					i = ACC.Add ( i, 1 );
				while ( COMP.IsLess ( x, ACC.Key(j) ) )
					j = ACC.Add ( j, -1 );
				if ( i<=j )
				{
					ACC.Swap ( i, j );
					i = ACC.Add ( i, 1 );
					j = ACC.Add ( j, -1 );
				}
			}

			if ( ACC.Sub ( j, a )>=ACC.Sub ( b, i ) )
			{
				if ( a<j ) { st0[k] = a; st1[k] = j; k++; }
				a = i;
			} else
			{
				if ( i<b ) { st0[k] = i; st1[k] = b; k++; }
				b = j;
			}
		}
	}
}


template < typename T, typename U >
void sphSort ( T * pData, int iCount, U COMP )
{
	sphSort ( pData, iCount, COMP, SphAccessor_T<T>() );
}


template < typename T >
void sphSort ( T * pData, int iCount )
{
	sphSort ( pData, iCount, SphLess_T<T>() );
}

//////////////////////////////////////////////////////////////////////////

/// member functor, wraps object member access
template < typename T, typename CLASS >
struct SphMemberFunctor_T
{
	const T CLASS::*	m_pMember;

	explicit			SphMemberFunctor_T ( T CLASS::* pMember )	: m_pMember ( pMember ) {}
	const T &			operator () ( const CLASS & arg ) const		{ return (&arg)->*m_pMember; }

	inline bool IsLess ( const CLASS & a, const CLASS & b ) const
	{
		return (&a)->*m_pMember < (&b)->*m_pMember;
	}
};


/// handy member functor generator
template < typename T, typename CLASS >
inline SphMemberFunctor_T < T, CLASS >
bind ( T CLASS::* ptr )
{
	return SphMemberFunctor_T < T, CLASS > ( ptr );
}


/// identity functor
template < typename T >
struct SphIdentityFunctor_T
{
	const T &			operator () ( const T & arg ) const			{ return arg; }
};

//////////////////////////////////////////////////////////////////////////

/// generic binary search
template < typename T, typename U, typename PRED >
T * sphBinarySearch ( T * pStart, T * pEnd, const PRED & tPred, U tRef )
{
	if ( !pStart || pEnd<pStart )
		return NULL;

	if ( tPred(*pStart)==tRef )
		return pStart;

	if ( tPred(*pEnd)==tRef )
		return pEnd;

	while ( pEnd-pStart>1 )
	{
		if ( tRef<tPred(*pStart) || tPred(*pEnd)<tRef )
			break;
		assert ( tPred(*pStart)<tRef );
		assert ( tRef<tPred(*pEnd) );

		T * pMid = pStart + (pEnd-pStart)/2;
		if ( tRef==tPred(*pMid) )
			return pMid;

		if ( tRef<tPred(*pMid) )
			pEnd = pMid;
		else
			pStart = pMid;
	}
	return NULL;
}


/// generic binary search
template < typename T >
T * sphBinarySearch ( T * pStart, T * pEnd, T & tRef )
{
	return sphBinarySearch ( pStart, pEnd, SphIdentityFunctor_T<T>(), tRef );
}

//////////////////////////////////////////////////////////////////////////

/// default vector policy
/// grow 2x and copy using assignment operator on resize
template < typename T >
class CSphVectorPolicy
{
protected:
	static const int MAGIC_INITIAL_LIMIT = 8;

public:
	static inline void Copy ( T * pNew, T * pData, int iLength )
	{
		for ( int i=0; i<iLength; i++ )
			pNew[i] = pData[i];
	}

	static inline int Relimit ( int iLimit, int iNewLimit )
	{
		if ( !iLimit )
			iLimit = MAGIC_INITIAL_LIMIT;
		while ( iLimit<iNewLimit )
			iLimit *= 2;
		return iLimit;
	}
};

/// generic vector
/// (don't even ask why it's not std::vector)
template < typename T, typename POLICY=CSphVectorPolicy<T> > class CSphVector
{
public:
	/// ctor
	CSphVector ()
		: m_iLength	( 0 )
		, m_iLimit	( 0 )
		, m_pData	( NULL )
	{
	}

	/// ctor with initial size
	CSphVector ( int iCount )
		: m_iLength	( 0 )
		, m_iLimit	( 0 )
		, m_pData	( NULL )
	{
		Resize ( iCount );
	}

	/// copy ctor
	CSphVector ( const CSphVector<T> & rhs )
	{
		m_iLength = 0;
		m_iLimit = 0;
		m_pData = NULL;
		*this = rhs;
	}

	/// dtor
	~CSphVector ()
	{
		Reset ();
	}

	/// add entry
	T & Add ()
	{
		if ( m_iLength>=m_iLimit )
			Reserve ( 1+m_iLength );
		return m_pData [ m_iLength++ ];
	}

	/// add entry
	void Add ( const T & tValue )
	{
		assert ( (&tValue<m_pData || &tValue>=(m_pData+m_iLength)) && "inserting own value (like last()) by ref!" );
		if ( m_iLength>=m_iLimit )
			Reserve ( 1+m_iLength );
		m_pData [ m_iLength++ ] = tValue;
	}

	/// add unique entry (ie. do not add if equal to last one)
	void AddUnique ( const T & tValue )
	{
		assert ( (&tValue<m_pData || &tValue>=(m_pData+m_iLength)) && "inserting own value (like last()) by ref!" );
		if ( m_iLength>=m_iLimit )
			Reserve ( 1+m_iLength );

		if ( m_iLength==0 || m_pData[m_iLength-1]!=tValue )
			m_pData [ m_iLength++ ] = tValue;
	}

	/// get first entry ptr
	T * Begin ()
	{
		return m_iLength ? m_pData : NULL;
	}

	/// get first entry ptr
	const T * Begin () const
	{
		return m_iLength ? m_pData : NULL;
	}

	/// get last entry
	T & Last ()
	{
		return (*this) [ m_iLength-1 ];
	}

	/// get last entry
	const T & Last () const
	{
		return (*this) [ m_iLength-1 ];
	}

	/// remove element by index
	void Remove ( int iIndex )
	{
		assert ( iIndex>=0 && iIndex<m_iLength );

		m_iLength--;
		for ( int i=iIndex; i<m_iLength; i++ )
			m_pData[i] = m_pData[i+1];
	}

	/// remove element by index, swapping it with the tail
	void RemoveFast ( int iIndex )
	{
		assert ( iIndex>=0 && iIndex<m_iLength );
		if ( iIndex!=--m_iLength )
			Swap ( m_pData[iIndex], m_pData[m_iLength] );
	}

	/// remove element by value (warning, linear O(n) search)
	bool RemoveValue ( T tValue )
	{
		for ( int i=0; i<m_iLength; i++ )
		{
			if ( m_pData[i]==tValue )
			{
				Remove ( i );
				return true;
			}
		}
		return false;
	}

	/// pop last value
	const T & Pop ()
	{
		assert ( m_iLength>0 );
		return m_pData[--m_iLength];
	}

public:
	/// grow enough to hold that much entries, if needed, but do *not* change the length
	void Reserve ( int iNewLimit )
	{
		// check that we really need to be called
		assert ( iNewLimit>=0 );
		if ( iNewLimit<=m_iLimit )
			return;

		// calc new limit
		m_iLimit = POLICY::Relimit ( m_iLimit, iNewLimit );

		// realloc
		// FIXME! optimize for POD case
		T * pNew = new T [ m_iLimit ];
		__analysis_assume ( m_iLength<=m_iLimit );

		POLICY::Copy ( pNew, m_pData, m_iLength );
		delete [] m_pData;

		m_pData = pNew;
	}

	/// resize
	void Resize ( int iNewLength )
	{
		if ( (unsigned int)iNewLength>=(unsigned int)m_iLength )
			Reserve ( iNewLength );
		m_iLength = iNewLength;
	}

	/// reset
	void Reset ()
	{
		m_iLength = 0;
		m_iLimit = 0;
		SafeDeleteArray ( m_pData );
	}

	/// query current length
	inline int GetLength () const
	{
		return m_iLength;
	}

	/// query current reserved size
	inline int GetLimit () const
	{
		return m_iLimit;
	}

public:
	/// filter unique
	void Uniq ()
	{
		if ( !m_iLength )
			return;

		Sort ();

		int iSrc = 0, iDst = 0;
		while ( iSrc<m_iLength )
		{
			if ( iDst>0 && m_pData[iDst-1]==m_pData[iSrc] )
				iSrc++;
			else
				m_pData[iDst++] = m_pData[iSrc++];
		}

		Resize ( iDst );
	}

	/// default sort
	void Sort ( int iStart=0, int iEnd=-1 )
	{
		Sort ( SphLess_T<T>(), iStart, iEnd );
	}

	/// default reverse sort
	void RSort ( int iStart=0, int iEnd=-1 )
	{
		Sort ( SphGreater_T<T>(), iStart, iEnd );
	}

	/// generic sort
	template < typename F > void Sort ( F COMP, int iStart=0, int iEnd=-1 )
	{
		if ( m_iLength<2 ) return;
		if ( iStart<0 ) iStart = m_iLength+iStart;
		if ( iEnd<0 ) iEnd = m_iLength+iEnd;
		assert ( iStart<=iEnd );

		sphSort ( m_pData+iStart, iEnd-iStart+1, COMP );
	}

	/// accessor by forward index
	const T & operator [] ( int iIndex ) const
	{
		assert ( iIndex>=0 && iIndex<m_iLength );
		return m_pData [ iIndex ];
	}

	/// accessor by forward index
	T & operator [] ( int iIndex )
	{
		assert ( iIndex>=0 && iIndex<m_iLength );
		return m_pData [ iIndex ];
	}

	/// copy
	const CSphVector<T> & operator = ( const CSphVector<T> & rhs )
	{
		Reset ();

		m_iLength = rhs.m_iLength;
		m_iLimit = rhs.m_iLimit;
		m_pData = new T [ m_iLimit ];
		__analysis_assume ( m_iLength<=m_iLimit );
		for ( int i=0; i<rhs.m_iLength; i++ )
			m_pData[i] = rhs.m_pData[i];

		return *this;
	}

	/// swap
	void SwapData ( CSphVector<T, POLICY> & rhs )
	{
		Swap ( m_iLength, rhs.m_iLength );
		Swap ( m_iLimit, rhs.m_iLimit );
		Swap ( m_pData, rhs.m_pData );
	}

	/// leak
	T * LeakData ()
	{
		T * pData = m_pData;
		m_pData = NULL;
		Reset();
		return pData;
	}

	/// generic binary search
	/// assumes that the array is sorted in ascending order
	template < typename U, typename PRED >
	const T * BinarySearch ( const PRED & tPred, U tRef ) const
	{
		return sphBinarySearch ( m_pData, m_pData+m_iLength-1, tPred, tRef );
	}

	/// generic binary search
	/// assumes that the array is sorted in ascending order
	const T * BinarySearch ( T tRef ) const
	{
		return sphBinarySearch ( m_pData, m_pData+m_iLength-1, tRef );
	}

	/// generic linear search
	bool Contains ( T tRef ) const
	{
		for ( int i=0; i<m_iLength; i++ )
			if ( m_pData[i]==tRef )
				return true;
		return false;
	}

	/// fill with given value
	void Fill ( const T & rhs )
	{
		for ( int i=0; i<m_iLength; i++ )
			m_pData[i] = rhs;
	}

	/// insert into a middle
	void Insert ( int iIndex, const T & tValue )
	{
		if ( iIndex==m_iLength )
		{
			Add ( tValue );
			return;
		}

		if ( m_iLength>=m_iLimit )
			Reserve ( m_iLength+1 );

		memmove ( m_pData+iIndex+1, m_pData+iIndex, ( m_iLength++-iIndex ) * sizeof tValue );
		memset ( m_pData+iIndex, 0, sizeof tValue );
		m_pData[iIndex] = tValue;
	}

protected:
	int		m_iLength;		///< entries actually used
	int		m_iLimit;		///< entries allocated
	T *		m_pData;		///< entries
};


#define ARRAY_FOREACH(_index,_array) \
	for ( int _index=0; _index<_array.GetLength(); _index++ )

#define ARRAY_FOREACH_COND(_index,_array,_cond) \
	for ( int _index=0; _index<_array.GetLength() && (_cond); _index++ )

#define ARRAY_ANY(_res,_array,_cond) \
	false; \
	for ( int _any=0; _any<_array.GetLength() && !_res; _any++ ) \
		_res |= ( _cond ); \

#define ARRAY_ALL(_res,_array,_cond) \
	true; \
	for ( int _all=0; _all<_array.GetLength() && _res; _all++ ) \
		_res &= ( _cond ); \

//////////////////////////////////////////////////////////////////////////

/// swap-vector policy (for non-copyable classes)
/// use Swap() instead of assignment on resize
template < typename T >
class CSphSwapVectorPolicy : public CSphVectorPolicy<T>
{
public:
	static inline void Copy ( T * pNew, T * pData, int iLength )
	{
		for ( int i=0; i<iLength; i++ )
			Swap ( pNew[i], pData[i] );
	}
};

/// tight-vector policy
/// grow only 1.2x on resize (not 2x) starting from a certain threshold
template < typename T >
class CSphTightVectorPolicy : public CSphVectorPolicy<T>
{
protected:
	static const int SLOW_GROW_TRESHOLD = 1024;

public:
	static inline int Relimit ( int iLimit, int iNewLimit )
	{
		if ( !iLimit )
			iLimit = CSphVectorPolicy<T>::MAGIC_INITIAL_LIMIT;
		while ( iLimit<iNewLimit && iLimit<SLOW_GROW_TRESHOLD )
			iLimit *= 2;
		while ( iLimit<iNewLimit )
			iLimit = (int)( iLimit*1.2f );
		return iLimit;
	}
};

/// swap-vector
template < typename T >
class CSphSwapVector : public CSphVector < T, CSphSwapVectorPolicy<T> >
{
};

/// tight-vector
template < typename T >
class CSphTightVector : public CSphVector < T, CSphTightVectorPolicy<T> >
{
};

//////////////////////////////////////////////////////////////////////////

/// dynamically allocated fixed-size vector
template < typename T >
class CSphFixedVector : public ISphNoncopyable
{
protected:
	T *			m_pData;
	int			m_iSize;

public:
	explicit CSphFixedVector ( int iSize )
		: m_iSize ( iSize )
	{
		assert ( iSize>=0 );
		m_pData = ( iSize>0 ) ? new T [ iSize ] : NULL;
	}

	~CSphFixedVector ()
	{
		SafeDeleteArray ( m_pData );
	}

	T & operator [] ( int iIndex ) const
	{
		assert ( iIndex>=0 && iIndex<m_iSize );
		return m_pData[iIndex];
	}

	T * Begin () const
	{
		return m_pData;
	}

	T & Last () const
	{
		return (*this) [ m_iSize-1 ];
	}

	void Reset ( int iSize )
	{
		SafeDeleteArray ( m_pData );
		assert ( iSize>=0 );
		m_pData = ( iSize>0 ) ? new T [ iSize ] : NULL;
		m_iSize = iSize;
	}

	int GetLength() const
	{
		return m_iSize;
	}

	T * LeakData ()
	{
		T * pData = m_pData;
		m_pData = NULL;
		Reset ( 0 );
		return pData;
	}
};

//////////////////////////////////////////////////////////////////////////

/// simple dynamic hash
/// keeps the order, so Iterate() return the entries in the order they was inserted
template < typename T, typename KEY, typename HASHFUNC, int LENGTH >
class CSphOrderedHash
{
protected:
	struct HashEntry_t
	{
		KEY				m_tKey;				///< key, owned by the hash
		T 				m_tValue;			///< data, owned by the hash
		HashEntry_t *	m_pNextByHash;		///< next entry in hash list
		HashEntry_t *	m_pPrevByOrder;		///< prev entry in the insertion order
		HashEntry_t *	m_pNextByOrder;		///< next entry in the insertion order
	};


protected:
	HashEntry_t *	m_dHash [ LENGTH ];		///< all the hash entries
	HashEntry_t *	m_pFirstByOrder;		///< first entry in the insertion order
	HashEntry_t *	m_pLastByOrder;			///< last entry in the insertion order
	int				m_iLength;				///< entries count

protected:
	/// find entry by key
	HashEntry_t * FindByKey ( const KEY & tKey ) const
	{
		unsigned int uHash = ( (unsigned int) HASHFUNC::Hash ( tKey ) ) % LENGTH;
		HashEntry_t * pEntry = m_dHash [ uHash ];

		while ( pEntry )
		{
			if ( pEntry->m_tKey==tKey )
				return pEntry;
			pEntry = pEntry->m_pNextByHash;
		}
		return NULL;
	}

public:
	/// ctor
	CSphOrderedHash ()
		: m_pFirstByOrder ( NULL )
		, m_pLastByOrder ( NULL )
		, m_iLength ( 0 )
		, m_pIterator ( NULL )
	{
		for ( int i=0; i<LENGTH; i++ )
			m_dHash[i] = NULL;
	}

	/// dtor
	~CSphOrderedHash ()
	{
		Reset ();
	}

	/// reset
	void Reset ()
	{
		HashEntry_t * pKill = m_pFirstByOrder;
		while ( pKill )
		{
			HashEntry_t * pNext = pKill->m_pNextByOrder;
			SafeDelete ( pKill );
			pKill = pNext;
		}

		for ( int i=0; i<LENGTH; i++ )
			m_dHash[i] = 0;

		m_pFirstByOrder = NULL;
		m_pLastByOrder = NULL;
		m_pIterator = NULL;
		m_iLength = 0;
	}

	/// add new entry
	/// returns true on success
	/// returns false if this key is already hashed
	bool Add ( const T & tValue, const KEY & tKey )
	{
		unsigned int uHash = ( (unsigned int) HASHFUNC::Hash ( tKey ) ) % LENGTH;

		// check if this key is already hashed
		HashEntry_t * pEntry = m_dHash [ uHash ];
		HashEntry_t ** ppEntry = &m_dHash [ uHash ];
		while ( pEntry )
		{
			if ( pEntry->m_tKey==tKey )
				return false;

			ppEntry = &pEntry->m_pNextByHash;
			pEntry = pEntry->m_pNextByHash;
		}

		// it's not; let's add the entry
		assert ( !pEntry );
		assert ( !*ppEntry );

		pEntry = new HashEntry_t;
		pEntry->m_tKey = tKey;
		pEntry->m_tValue = tValue;
		pEntry->m_pNextByHash = NULL;
		pEntry->m_pPrevByOrder = NULL;
		pEntry->m_pNextByOrder = NULL;

		*ppEntry = pEntry;

		if ( !m_pFirstByOrder )
			m_pFirstByOrder = pEntry;

		if ( m_pLastByOrder )
		{
			assert ( !m_pLastByOrder->m_pNextByOrder );
			assert ( !pEntry->m_pNextByOrder );
			m_pLastByOrder->m_pNextByOrder = pEntry;
			pEntry->m_pPrevByOrder = m_pLastByOrder;
		}
		m_pLastByOrder = pEntry;

		m_iLength++;
		return true;
	}

	/// add new entry
	/// returns the pointer to just inserted or previously cached (if dupe) value
	T & AddUnique ( const KEY & tKey )
	{
		unsigned int uHash = ( (unsigned int) HASHFUNC::Hash ( tKey ) ) % LENGTH;

		// check if this key is already hashed
		HashEntry_t * pEntry = m_dHash [ uHash ];
		HashEntry_t ** ppEntry = &m_dHash [ uHash ];
		while ( pEntry )
		{
			if ( pEntry->m_tKey==tKey )
				return pEntry->m_tValue;

			ppEntry = &pEntry->m_pNextByHash;
			pEntry = pEntry->m_pNextByHash;
		}

		// it's not; let's add the entry
		assert ( !pEntry );
		assert ( !*ppEntry );

		pEntry = new HashEntry_t;
		pEntry->m_tKey = tKey;
		pEntry->m_pNextByHash = NULL;
		pEntry->m_pPrevByOrder = NULL;
		pEntry->m_pNextByOrder = NULL;

		*ppEntry = pEntry;

		if ( !m_pFirstByOrder )
			m_pFirstByOrder = pEntry;

		if ( m_pLastByOrder )
		{
			assert ( !m_pLastByOrder->m_pNextByOrder );
			assert ( !pEntry->m_pNextByOrder );
			m_pLastByOrder->m_pNextByOrder = pEntry;
			pEntry->m_pPrevByOrder = m_pLastByOrder;
		}
		m_pLastByOrder = pEntry;

		m_iLength++;
		return pEntry->m_tValue;
	}

	/// delete an entry
	bool Delete ( const KEY & tKey )
	{
		unsigned int uHash = ( (unsigned int) HASHFUNC::Hash ( tKey ) ) % LENGTH;
		HashEntry_t * pEntry = m_dHash [ uHash ];

		HashEntry_t * pPrevEntry = NULL;
		HashEntry_t * pToDelete = NULL;
		while ( pEntry )
		{
			if ( pEntry->m_tKey==tKey )
			{
				pToDelete = pEntry;
				if ( pPrevEntry )
					pPrevEntry->m_pNextByHash = pEntry->m_pNextByHash;
				else
					m_dHash [ uHash ] = pEntry->m_pNextByHash;

				break;
			}

			pPrevEntry = pEntry;
			pEntry = pEntry->m_pNextByHash;
		}

		if ( !pToDelete )
			return false;

		if ( pToDelete->m_pPrevByOrder )
			pToDelete->m_pPrevByOrder->m_pNextByOrder = pToDelete->m_pNextByOrder;
		else
			m_pFirstByOrder = pToDelete->m_pNextByOrder;

		if ( pToDelete->m_pNextByOrder )
			pToDelete->m_pNextByOrder->m_pPrevByOrder = pToDelete->m_pPrevByOrder;
		else
			m_pLastByOrder = pToDelete->m_pPrevByOrder;

		// step the iterator one item back - to gracefully hold deletion in iteration cycle
		if ( pToDelete==m_pIterator )
			m_pIterator = pToDelete->m_pPrevByOrder;

		SafeDelete ( pToDelete );
		--m_iLength;

		return true;
	}

	/// check if key exists
	bool Exists ( const KEY & tKey ) const
	{
		return FindByKey ( tKey )!=NULL;
	}

	/// get value pointer by key
	T * operator () ( const KEY & tKey ) const
	{
		HashEntry_t * pEntry = FindByKey ( tKey );
		return pEntry ? &pEntry->m_tValue : NULL;
	}

	/// get value reference by key, asserting that the key exists in hash
	T & operator [] ( const KEY & tKey ) const
	{
		HashEntry_t * pEntry = FindByKey ( tKey );
		assert ( pEntry && "hash missing value in operator []" );

		return pEntry->m_tValue;
	}

	/// get pointer to key storage
	const KEY * GetKeyPtr ( const KEY & tKey ) const
	{
		HashEntry_t * pEntry = FindByKey ( tKey );
		return pEntry ? &pEntry->m_tKey : NULL;
	}

	/// copying
	const CSphOrderedHash<T,KEY,HASHFUNC,LENGTH> & operator = ( const CSphOrderedHash<T,KEY,HASHFUNC,LENGTH> & rhs )
	{
		if ( this!=&rhs )
		{
			Reset ();

			rhs.IterateStart ();
			while ( rhs.IterateNext() )
				Add ( rhs.IterateGet(), rhs.IterateGetKey() );
		}
		return *this;
	}

	/// copyint ctor
	CSphOrderedHash<T,KEY,HASHFUNC,LENGTH> ( const CSphOrderedHash<T,KEY,HASHFUNC,LENGTH> & rhs )
		: m_pFirstByOrder ( NULL )
		, m_pLastByOrder ( NULL )
		, m_iLength ( 0 )
		, m_pIterator ( NULL )
	{
		for ( int i=0; i<LENGTH; i++ )
			m_dHash[i] = NULL;
		*this = rhs;
	}

	/// length query
	int GetLength () const
	{
		return m_iLength;
	}

public:
	/// start iterating
	void IterateStart () const
	{
		m_pIterator = NULL;
	}

	/// start iterating from key element
	bool IterateStart ( const KEY & tKey ) const
	{
		m_pIterator = FindByKey ( tKey );
		return m_pIterator!=NULL;
	}

	/// go to next existing entry
	bool IterateNext () const
	{
		m_pIterator = m_pIterator ? m_pIterator->m_pNextByOrder : m_pFirstByOrder;
		return m_pIterator!=NULL;
	}

	/// get entry value
	T & IterateGet () const
	{
		assert ( m_pIterator );
		return m_pIterator->m_tValue;
	}

	/// get entry key
	const KEY & IterateGetKey () const
	{
		assert ( m_pIterator );
		return m_pIterator->m_tKey;
	}

	/// go to next existing entry in terms of external independed iterator
	bool IterateNext ( void ** ppCookie ) const
	{
		HashEntry_t ** ppIterator = reinterpret_cast < HashEntry_t** > ( ppCookie );
		*ppIterator = ( *ppIterator ) ? ( *ppIterator )->m_pNextByOrder : m_pFirstByOrder;
		return ( *ppIterator )!=NULL;
	}

	/// get entry value in terms of external independed iterator
	static T & IterateGet ( void ** ppCookie )
	{
		assert ( ppCookie );
		HashEntry_t ** ppIterator = reinterpret_cast < HashEntry_t** > ( ppCookie );
		assert ( *ppIterator );
		return ( *ppIterator )->m_tValue;
	}

	/// get entry key in terms of external independed iterator
	static const KEY & IterateGetKey ( void ** ppCookie )
	{
		assert ( ppCookie );
		HashEntry_t ** ppIterator = reinterpret_cast < HashEntry_t** > ( ppCookie );
		assert ( *ppIterator );
		return ( *ppIterator )->m_tKey;
	}


private:
	/// current iterator
	mutable HashEntry_t *	m_pIterator;
};

/// very popular and so, moved here
struct IdentityHash_fn
{
	template <typename INT>
	static inline INT Hash ( INT iValue )	{ return iValue; }
};

/////////////////////////////////////////////////////////////////////////////

/// immutable C string proxy
struct CSphString
{
protected:
	char *				m_sValue;

private:
	/// safety gap after the string end; for instance, UTF-8 Russian stemmer
	/// which treats strings as 16-bit word sequences needs this in some cases.
	/// note that this zero-filled gap does NOT include trailing C-string zero,
	/// and does NOT affect strlen() as well.
	static const int	SAFETY_GAP	= 4;

public:
	CSphString ()
		: m_sValue ( NULL )
	{
	}

	CSphString ( const CSphString & rhs )
		: m_sValue ( NULL )
	{
		*this = rhs;
	}

	virtual ~CSphString ()
	{
		SafeDeleteArray ( m_sValue );
	}

	const char * cstr () const
	{
		return m_sValue;
	}

	inline bool operator == ( const char * t ) const
	{
		if ( !t || !m_sValue )
			return ( ( !t && !m_sValue ) || ( !t && m_sValue && !*m_sValue ) || ( !m_sValue && t && !*t ) );
		return strcmp ( m_sValue, t )==0;
	}

	inline bool operator == ( const CSphString & t ) const
	{
		return operator==( t.cstr() );
	}

	inline bool operator != ( const CSphString & t ) const
	{
		return !operator==( t );
	}

	bool operator != ( const char * t ) const
	{
		return !operator==( t );
	}

	CSphString ( const char * sString ) // NOLINT
	{
		if ( sString )
		{
			int iLen = 1+strlen(sString);
			m_sValue = new char [ iLen+SAFETY_GAP ];

			strcpy ( m_sValue, sString ); // NOLINT
			memset ( m_sValue+iLen, 0, SAFETY_GAP );
		} else
		{
			m_sValue = NULL;
		}
	}

	CSphString ( const char * sValue, int iLen )
		: m_sValue ( NULL )
	{
		SetBinary ( sValue, iLen );
	}

	const CSphString & operator = ( const CSphString & rhs )
	{
		if ( m_sValue==rhs.m_sValue )
			return *this;
		SafeDeleteArray ( m_sValue );
		if ( rhs.m_sValue )
		{
			int iLen = 1+strlen(rhs.m_sValue);
			m_sValue = new char [ iLen+SAFETY_GAP ];

			strcpy ( m_sValue, rhs.m_sValue ); // NOLINT
			memset ( m_sValue+iLen, 0, SAFETY_GAP );
		}
		return *this;
	}

	CSphString SubString ( int iStart, int iCount ) const
	{
		#ifndef NDEBUG
		int iLen = strlen(m_sValue);
		#endif
		assert ( iStart>=0 && iStart<iLen );
		assert ( iCount>0 );
		assert ( (iStart+iCount)>=0 && (iStart+iCount)<=iLen );

		CSphString sRes;
		sRes.m_sValue = new char [ 1+SAFETY_GAP+iCount ];
		strncpy ( sRes.m_sValue, m_sValue+iStart, iCount );
		memset ( sRes.m_sValue+iCount, 0, 1+SAFETY_GAP );
		return sRes;
	}

	void SetBinary ( const char * sValue, int iLen )
	{
		SafeDeleteArray ( m_sValue );
		if ( sValue )
		{
			m_sValue = new char [ 1+SAFETY_GAP+iLen ];
			memcpy ( m_sValue, sValue, iLen );
			memset ( m_sValue+iLen, 0, 1+SAFETY_GAP );
		}
	}

	void Reserve ( int iLen )
	{
		SafeDeleteArray ( m_sValue );
		m_sValue = new char [ 1+SAFETY_GAP+iLen ];
		memset ( m_sValue, 0, 1+SAFETY_GAP+iLen );
	}

	const CSphString & SetSprintf ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) )
	{
		char sBuf[1024];
		va_list ap;

		va_start ( ap, sTemplate );
		vsnprintf ( sBuf, sizeof(sBuf), sTemplate, ap );
		va_end ( ap );

		(*this) = sBuf;
		return (*this);
	}

	const CSphString & SetSprintfVa ( const char * sTemplate, va_list ap )
	{
		char sBuf[1024];
		vsnprintf ( sBuf, sizeof(sBuf), sTemplate, ap );

		(*this) = sBuf;
		return (*this);
	}

	bool IsEmpty () const
	{
		if ( !m_sValue )
			return true;
		return ( (*m_sValue)=='\0' );
	}

	void ToLower ()
	{
		if ( m_sValue )
			for ( char * s=m_sValue; *s; s++ )
				*s = (char) tolower ( *s );
	}

	void ToUpper ()
	{
		if ( m_sValue )
			for ( char * s=m_sValue; *s; s++ )
				*s = (char) toupper ( *s );
	}

	void Swap ( CSphString & rhs )
	{
		::Swap ( m_sValue, rhs.m_sValue );
	}

	bool Begins ( const char * sPrefix ) const
	{
		if ( !m_sValue || !sPrefix )
			return false;
		return strncmp ( m_sValue, sPrefix, strlen(sPrefix) )==0;
	}

	bool Ends ( const char * sSuffix ) const
	{
		if ( !m_sValue || !sSuffix )
			return false;

		int iVal = strlen ( m_sValue );
		int iSuffix = strlen ( sSuffix );
		if ( iVal<iSuffix )
			return false;
		return strncmp ( m_sValue+iVal-iSuffix, sSuffix, iSuffix )==0;
	}

	void Trim ()
	{
		if ( m_sValue )
		{
			const char * sStart = m_sValue;
			const char * sEnd = m_sValue + strlen(m_sValue) - 1;
			while ( sStart<=sEnd && isspace ( (unsigned char)*sStart ) ) sStart++;
			while ( sStart<=sEnd && isspace ( (unsigned char)*sEnd ) ) sEnd--;
			memmove ( m_sValue, sStart, sEnd-sStart+1 );
			m_sValue [ sEnd-sStart+1 ] = '\0';
		}
	}

	int Length () const
	{
		return m_sValue ? (int)strlen(m_sValue) : 0;
	}

	char * Leak ()
	{
		char * pBuf = m_sValue;
		m_sValue = NULL;
		return pBuf;
	}

	// opposite to Leak()
	void Adopt ( char ** sValue )
	{
		SafeDeleteArray ( m_sValue );
		m_sValue = *sValue;
		*sValue = NULL;
	}

	bool operator < ( const CSphString & b ) const
	{
		if ( !m_sValue && !b.m_sValue )
			return false;
		if ( !m_sValue || !b.m_sValue )
			return !m_sValue;
		return strcmp ( m_sValue, b.m_sValue ) < 0;
	}
};

/// string swapper
inline void Swap ( CSphString & v1, CSphString & v2 )
{
	v1.Swap ( v2 );
}

/////////////////////////////////////////////////////////////////////////////

/// immutable string/int/float variant list proxy
struct CSphVariant : public CSphString
{
protected:
	int				m_iValue;
	int64_t			m_i64Value;
	float			m_fValue;

public:
	CSphVariant *	m_pNext;
	bool			m_bTag;

public:
	/// default ctor
	CSphVariant ()
		: CSphString ()
		, m_iValue ( 0 )
		, m_i64Value ( 0 )
		, m_fValue ( 0.0f )
		, m_pNext ( NULL )
		, m_bTag ( false )
	{
	}

	/// ctor from C string
	CSphVariant ( const char * sString ) // NOLINT desired implicit conversion
		: CSphString ( sString )
		, m_iValue ( m_sValue ? atoi ( m_sValue ) : 0 )
		, m_i64Value ( m_sValue ? (int64_t)strtoull ( m_sValue, NULL, 10 ) : 0 )
		, m_fValue ( m_sValue ? (float)atof ( m_sValue ) : 0.0f )
		, m_pNext ( NULL )
	{
	}

	/// copy ctor
	CSphVariant ( const CSphVariant & rhs )
		: CSphString ()
		, m_iValue ( 0 )
		, m_i64Value ( 0 )
		, m_fValue ( 0.0f )
		, m_pNext ( NULL )
	{
		*this = rhs;
	}

	/// default dtor
	/// WARNING: automatically frees linked items!
	virtual ~CSphVariant ()
	{
		SafeDelete ( m_pNext );
	}

	/// int value getter
	int intval () const
	{
		return m_iValue;
	}

	/// int64_t value getter
	int64_t int64val () const
	{
		return m_i64Value;
	}

	/// float value getter
	float floatval () const
	{
		return m_fValue;
	}

	/// default copy operator
	const CSphVariant & operator = ( const CSphVariant & rhs )
	{
		assert ( !m_pNext );
		if ( rhs.m_pNext )
			m_pNext = new CSphVariant ( *rhs.m_pNext );

		CSphString::operator = ( rhs );
		m_iValue = rhs.m_iValue;
		m_i64Value = rhs.m_i64Value;
		m_fValue = rhs.m_fValue;

		return *this;
	}
};

//////////////////////////////////////////////////////////////////////////

/// name+int pair
struct CSphNamedInt
{
	CSphString	m_sName;
	int			m_iValue;

	CSphNamedInt () : m_iValue ( 0 ) {}
};

/////////////////////////////////////////////////////////////////////////////

/// string hash function
struct CSphStrHashFunc
{
	static int Hash ( const CSphString & sKey );
};

/// small hash with string keys
template < typename T >
class SmallStringHash_T : public CSphOrderedHash < T, CSphString, CSphStrHashFunc, 256 > {};

//////////////////////////////////////////////////////////////////////////

/// pointer with automatic safe deletion when going out of scope
template < typename T >
class CSphScopedPtr : public ISphNoncopyable
{
public:
	explicit		CSphScopedPtr ( T * pPtr )	{ m_pPtr = pPtr; }
					~CSphScopedPtr ()			{ SafeDelete ( m_pPtr ); }
	T *				operator -> () const		{ return m_pPtr; }
	T *				Ptr () const				{ return m_pPtr; }
	CSphScopedPtr &	operator = ( T * pPtr )		{ SafeDelete ( m_pPtr ); m_pPtr = pPtr; return *this; }
	T *				LeakPtr ()					{ T * pPtr = m_pPtr; m_pPtr = NULL; return pPtr; }
	void			Reset ()					{ SafeDelete ( m_pPtr ); }

protected:
	T *				m_pPtr;
};

//////////////////////////////////////////////////////////////////////////

/// refcounted base
/// WARNING, FOR SINGLE-THREADED USE ONLY
struct ISphRefcounted : public ISphNoncopyable
{
protected:
					ISphRefcounted () : m_iRefCount ( 1 ) {}
	virtual			~ISphRefcounted () {}

public:
	void			AddRef () const		{ m_iRefCount++; }
	void			Release () const	{ --m_iRefCount; assert ( m_iRefCount>=0 ); if ( m_iRefCount==0 ) delete this; }

protected:
	mutable int		m_iRefCount;
};


/// automatic pointer wrapper for refcounted objects
/// construction from or assignment of a raw pointer takes over (!) the ownership
template < typename T >
class CSphRefcountedPtr
{
public:
	explicit		CSphRefcountedPtr ()			{ m_pPtr = NULL; }	///< default NULL wrapper construction (for vectors)
	explicit		CSphRefcountedPtr ( T * pPtr )	{ m_pPtr = pPtr; }	///< construction from raw pointer, takes over ownership!
					~CSphRefcountedPtr ()			{ if ( m_pPtr ) m_pPtr->Release(); }

	T *				Ptr () const					{ return m_pPtr; }
	T *				operator -> () const			{ return m_pPtr; }
	bool			operator ! () const				{ return m_pPtr==NULL; }

public:
	/// assignment of a raw pointer, takes over ownership!
	CSphRefcountedPtr<T> & operator = ( T * pPtr )
	{
		if ( m_pPtr && m_pPtr!=pPtr )
			m_pPtr->Release();
		m_pPtr = pPtr;
		return *this;
	}

	/// wrapper assignment, does automated reference tracking
	CSphRefcountedPtr<T> & operator = ( const CSphRefcountedPtr<T> & rhs )
	{
		if ( rhs.m_pPtr )
			rhs.m_pPtr->AddRef();
		if ( m_pPtr )
			m_pPtr->Release();
		m_pPtr = rhs.m_pPtr;
		return *this;
	}

protected:
	T *				m_pPtr;
};

//////////////////////////////////////////////////////////////////////////

extern bool g_bHeadProcess;
void sphWarn ( const char *, ... ) __attribute__ ( ( format ( printf, 1, 2 ) ) );

/// in-memory buffer shared between processes
template < typename T > class CSphSharedBuffer
{
public:
	/// ctor
	CSphSharedBuffer ()
		: m_pData ( NULL )
		, m_iLength ( 0 )
		, m_iEntries ( 0 )
		, m_bMlock ( false )
	{}

	/// dtor
	~CSphSharedBuffer ()
	{
		Reset ();
	}

	/// set locking mode for subsequent Alloc()s
	void SetMlock ( bool bMlock )
	{
		m_bMlock = bMlock;
	}

public:
	/// allocate storage
#if USE_WINDOWS
	bool Alloc ( int64_t iEntries, CSphString & sError, CSphString & )
#else
	bool Alloc ( int64_t iEntries, CSphString & sError, CSphString & sWarning )
#endif
	{
		assert ( !m_pData );

		int64_t uCheck = sizeof(T);
		uCheck *= iEntries;

		m_iLength = (size_t)uCheck;
		if ( uCheck!=(int64_t)m_iLength )
		{
			sError.SetSprintf ( "impossible to mmap() over 4 GB on 32-bit system" );
			m_iLength = 0;
			return false;
		}

#if USE_WINDOWS
		m_pData = new T [ (size_t)iEntries ];
#else
		m_pData = (T *) mmap ( NULL, m_iLength, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0 );
		if ( m_pData==MAP_FAILED )
		{
			if ( m_iLength>0x7fffffffUL )
				sError.SetSprintf ( "mmap() failed: %s (length="INT64_FMT" is over 2GB, impossible on some 32-bit systems)",
					strerror(errno), (int64_t)m_iLength );
			else
				sError.SetSprintf ( "mmap() failed: %s (length="INT64_FMT")", strerror(errno), (int64_t)m_iLength );
			m_iLength = 0;
			return false;
		}

		if ( m_bMlock )
			if ( -1==mlock ( m_pData, m_iLength ) )
				sWarning.SetSprintf ( "mlock() failed: %s", strerror(errno) );

#if SPH_ALLOCS_PROFILER
		sphMemStatMMapAdd ( m_iLength );
#endif

#endif // USE_WINDOWS

		assert ( m_pData );
		m_iEntries = (size_t)iEntries;
		return true;
	}


	/// relock again (for daemonization only)
#if USE_WINDOWS
	bool Mlock ( const char *, CSphString & )
	{
		return true;
	}
#else
	bool Mlock ( const char * sPrefix, CSphString & sError )
	{
		if ( !m_bMlock )
			return true;

		if ( mlock ( m_pData, m_iLength )!=-1 )
			return true;

		if ( sError.IsEmpty() )
			sError.SetSprintf ( "%s mlock() failed: bytes="INT64_FMT", error=%s", sPrefix, (int64_t)m_iLength, strerror(errno) );
		else
			sError.SetSprintf ( "%s; %s mlock() failed: bytes="INT64_FMT", error=%s", sError.cstr(), sPrefix, (int64_t)m_iLength, strerror(errno) );
		return false;
	}
#endif


	/// deallocate storage
	void Reset ()
	{
		if ( !m_pData )
			return;

#if USE_WINDOWS
		delete [] m_pData;
#else
		if ( g_bHeadProcess )
		{
			int iRes = munmap ( m_pData, m_iLength );
			if ( iRes )
				sphWarn ( "munmap() failed: %s", strerror(errno) );

#if SPH_ALLOCS_PROFILER
			sphMemStatMMapDel ( m_iLength );
#endif
		}
#endif // USE_WINDOWS

		m_pData = NULL;
		m_iLength = 0;
		m_iEntries = 0;
	}

public:
	/// accessor
	inline const T & operator [] ( int64_t iIndex ) const
	{
		assert ( iIndex>=0 && iIndex<(int64_t)m_iEntries );
		return m_pData[iIndex];
	}

	/// get write address
	T * GetWritePtr () const
	{
		return m_pData;
	}

	/// check if i'm empty
	bool IsEmpty () const
	{
		return m_pData==NULL;
	}

	/// get length in bytes
	size_t GetLength () const
	{
		return m_iLength;
	}

	/// get length in entries
	size_t GetNumEntries () const
	{
		return m_iEntries;
	}

protected:
	T *					m_pData;	///< data storage
	size_t				m_iLength;	///< data length, bytes
	size_t				m_iEntries;	///< data length, entries
	bool				m_bMlock;	///< whether to lock data in RAM
};

//////////////////////////////////////////////////////////////////////////

/// process-shared mutex that survives fork
class CSphProcessSharedMutex
{
public:
	explicit CSphProcessSharedMutex ( int iExtraSize=0 );
	~CSphProcessSharedMutex(); // not virtual for now.
	void	Lock () const;
	void	Unlock () const;
	bool	TimedLock ( int tmSpin ) const; // wait at least tmSpin microseconds the lock will available
	const char * GetError () const;
	BYTE *	GetSharedData() const;

protected:
#if !USE_WINDOWS
	CSphSharedBuffer<BYTE>		m_pStorage;
#ifdef __FreeBSD__
	sem_t *						m_pMutex;
#else
	pthread_mutex_t *			m_pMutex;
#endif
	CSphString					m_sError;
#endif
};

#if !USE_WINDOWS
/// process-shared mutex variable that survives fork
template < typename T > class CSphProcessSharedVariable : protected CSphProcessSharedMutex, public ISphNoncopyable
{
public:

	explicit CSphProcessSharedVariable ( const T& tInitValue )
		: CSphProcessSharedMutex ( sizeof(T) )
		, m_pValue ( NULL )
	{
		if ( m_pMutex )
		{
			m_pValue = reinterpret_cast<T*> ( GetSharedData() );
			*m_pValue = tInitValue;
		}
	}
	T ReadValue() const
	{
		if ( !m_pValue )
			return 0;
		Lock();
		T val = *m_pValue;
		Unlock();
		return val;
	}
	void WriteValue ( const T& tNewValue )
	{
		if ( !m_pValue )
			return;
		Lock();
		*m_pValue = tNewValue;
		Unlock();
	}

protected:
	T *		m_pValue;
};
#endif // #if !USE_WINDOWS

//////////////////////////////////////////////////////////////////////////

extern int g_iThreadStackSize;

/// my thread handle and thread func magic
#if USE_WINDOWS
typedef HANDLE SphThread_t;
typedef DWORD SphThreadKey_t;
#else
typedef pthread_t SphThread_t;
typedef pthread_key_t SphThreadKey_t;
#endif

/// my threading initialize routine
void * sphThreadInit ( bool bDetached=false );

/// my threading deinitialize routine
void sphThreadDone ( int iFD );

/// my create thread wrapper
bool sphThreadCreate ( SphThread_t * pThread, void (*fnThread)(void*), void * pArg, bool bDetached=false );

/// my join thread wrapper
bool sphThreadJoin ( SphThread_t * pThread );

/// add (cleanup) callback to run on thread exit
void sphThreadOnExit ( void (*fnCleanup)(void*), void * pArg );

/// alloc thread-local key
bool sphThreadKeyCreate ( SphThreadKey_t * pKey );

/// free thread-local key
void sphThreadKeyDelete ( SphThreadKey_t tKey );

/// get thread-local key value
void * sphThreadGet ( SphThreadKey_t tKey );

/// get the pointer to my thread's stack
void * sphMyStack ();

/// get size of used stack
int64_t sphGetStackUsed();

/// set the size of my thread's stack
void sphSetMyStackSize ( int iStackSize );

/// store the address in the TLS
void MemorizeStack ( void* PStack );

/// set thread-local key value
bool sphThreadSet ( SphThreadKey_t tKey, void * pValue );

#if !USE_WINDOWS
/// what kind of threading lib do we have? The number of frames in the stack depends from it
bool sphIsLtLib();
#endif

//////////////////////////////////////////////////////////////////////////

/// mutex implementation
class CSphMutex
{
public:
	CSphMutex () : m_bInitialized ( false ) {}
	~CSphMutex () { assert ( !m_bInitialized ); }

	bool Init ();
	bool Done ();
	bool Lock ();
	bool Unlock ();

protected:
	bool m_bInitialized;
#if USE_WINDOWS
	HANDLE m_hMutex;
#else
	pthread_mutex_t m_tMutex;
public:
	inline pthread_mutex_t* GetInternalMutex()
	{
		return &m_tMutex;
	}
#endif
};

// event implementation
class CSphAutoEvent
{
public:
	CSphAutoEvent () {};
	~CSphAutoEvent() {};

	bool Init(CSphMutex *);
	bool Done();
	void SetEvent();
	bool WaitEvent();

protected:
	bool m_bInitialized;
	bool m_bSent;
#if USE_WINDOWS
	HANDLE m_hEvent;
#else
	pthread_cond_t m_tCond;
	pthread_mutex_t* m_pMutex;
#endif
};


/// static mutex (for globals)
class CSphStaticMutex : public CSphMutex
{
public:
	CSphStaticMutex()
	{
		Verify ( Init() );
	}

	~CSphStaticMutex()
	{
		Done();
	}
};


/// scoped mutex lock
template < typename T >
class CSphScopedLock : ISphNoncopyable
{
public:
	/// lock on creation
	explicit CSphScopedLock ( T & tMutex )
		: m_tMutexRef ( tMutex )
	{
		m_tMutexRef.Lock();
	}

	/// unlock on going out of scope
	~CSphScopedLock ()
	{
		m_tMutexRef.Unlock ();
	}

protected:
	T &	m_tMutexRef;
};

/// MT-aware refcounted base
/// mutex protected, might be slow
struct ISphRefcountedMT : public ISphNoncopyable
{
protected:
	ISphRefcountedMT ()
		: m_iRefCount ( 1 )
	{
		m_tLock.Init();
	}

	virtual ~ISphRefcountedMT ()
	{
		m_tLock.Done();
	}

public:
	void AddRef () const
	{
		m_tLock.Lock();
		m_iRefCount++;
		m_tLock.Unlock();
	}

	void Release () const
	{
		m_tLock.Lock();
		int iRefs = --m_iRefCount;
		assert ( iRefs>=0 );
		m_tLock.Unlock();
		if ( iRefs==0 )
			delete this;
	}

protected:
	mutable int			m_iRefCount;
	mutable CSphMutex	m_tLock;
};


/// rwlock implementation
class CSphRwlock : public ISphNoncopyable
{
public:
	CSphRwlock ();
	~CSphRwlock () {}

	bool Init ();
	bool Done ();

	bool ReadLock ();
	bool WriteLock ();
	bool Unlock ();

private:
	bool				m_bInitialized;
#if USE_WINDOWS
	HANDLE				m_hWriteMutex;
	HANDLE				m_hReadEvent;
	LONG				m_iReaders;
#else
	pthread_rwlock_t	m_tLock;
#endif
};

// small bitvector of 256 elements.
class CSphSmallBitvec
{
public:
	static const int iTOTALBITS = 256;

private:
	static const int iELEMBITS = sizeof(DWORD) * 8;
	static const int iBYTESIZE = iTOTALBITS / 8;
	static const int IELEMENTS = iTOTALBITS / iELEMBITS;
	static const DWORD uALLBITS = (DWORD)(~(0UL));

	STATIC_ASSERT ( IELEMENTS>=1, 8_BITS_MINIMAL_SIZE_OF_VECTOR );

public:
	DWORD m_dFieldsMask[IELEMENTS];

public:
	// no custom cstr and d-tor - to be usable from inside unions
	// deep copy for it is ok - so, no explicit copying constructor and operator=

	// old-fashion layer to work with DWORD (32-bit) mask.
	// all bits above 32 assumed to be unset.
	void Assign32 ( DWORD uMask )
	{
		Unset();
		m_dFieldsMask[0] = uMask;
	}

	DWORD GetMask32 () const
	{
		return (DWORD) ( m_dFieldsMask[0] & 0xFFFFFFFFUL );
	}

	// set n-th bit, or all
	void Set ( int iIdx=-1 )
	{
		assert ( iIdx < iTOTALBITS );
		if ( iIdx<0 )
			for ( int i=0; i<IELEMENTS; i++ )
				m_dFieldsMask[i] = uALLBITS;
		else
			m_dFieldsMask[iIdx/iELEMBITS] |= 1UL << ( iIdx & ( iELEMBITS-1 ) );
	}

	// unset n-th bit, or all
	void Unset ( int iIdx=-1 )
	{
		assert ( iIdx < iTOTALBITS );
		if ( iIdx<0 )
			for ( int i=0; i<IELEMENTS; i++ )
				m_dFieldsMask[i] = 0UL;
		else
			m_dFieldsMask[iIdx/iELEMBITS] &= ~(1UL << ( iIdx & ( iELEMBITS-1 ) ));
	}

	// test if n-th bit is set
	bool Test ( int iIdx ) const
	{
		assert ( iIdx>=0 && iIdx<iTOTALBITS );
		return ( m_dFieldsMask[iIdx/iELEMBITS] & ( 1UL << ( iIdx & ( iELEMBITS-1 ) ) ) )!=0;
	}

	// test the given mask (with &-operator)
	bool Test ( const CSphSmallBitvec& dParam ) const
	{
		for ( int i=0; i<IELEMENTS; i++ )
			if ( m_dFieldsMask[i] & dParam.m_dFieldsMask[i] )
				return true;
		return false;
	}

	// test if all bits are set or unset
	bool TestAll ( bool bSet ) const
	{
		DWORD uTest = bSet?uALLBITS:0;
		for ( int i=0; i<IELEMENTS; i++ )
			if ( m_dFieldsMask[i]!=uTest )
				return false;
		return true;
	}

	friend CSphSmallBitvec operator & ( const CSphSmallBitvec& dFirst, const CSphSmallBitvec& dSecond );
	friend CSphSmallBitvec operator | ( const CSphSmallBitvec& dFirst, const CSphSmallBitvec& dSecond );
	friend bool operator == ( const CSphSmallBitvec& dFirst, const CSphSmallBitvec& dSecond );
	CSphSmallBitvec& operator |= ( const CSphSmallBitvec& dSecond )
	{
		if ( &dSecond!=this )
			for ( int i=0; i<IELEMENTS; i++ )
				m_dFieldsMask[i] |= dSecond.m_dFieldsMask[i];
		return *this;
	}

	// cut out all the bits over given number
	void LimitBits ( int iBits )
	{
		if ( iBits>=iTOTALBITS )
			return;

		int iMaskPos = iBits / iELEMBITS;
		DWORD uMask = ( 1UL << ( iBits % iELEMBITS ) ) - 1;
		m_dFieldsMask[iMaskPos++] &= uMask;
		for ( ; iMaskPos < IELEMENTS; iMaskPos++ )
			m_dFieldsMask[iMaskPos] = 0UL;
	}

	void Negate()
	{
		for ( int i=0; i<IELEMENTS; i++ )
			m_dFieldsMask[i] = ~m_dFieldsMask[i];
	}
};

#if USE_WINDOWS
#pragma warning(push,1)
#pragma warning(disable:4701)
#endif

inline CSphSmallBitvec operator & ( const CSphSmallBitvec& dFirst, const CSphSmallBitvec& dSecond )
{
	if ( &dFirst==&dSecond )
		return dFirst;

	CSphSmallBitvec dResult;
	for ( int i=0; i<CSphSmallBitvec::IELEMENTS; i++ )
			dResult.m_dFieldsMask[i] = dFirst.m_dFieldsMask[i] & dSecond.m_dFieldsMask[i];
	return dResult;
}

inline CSphSmallBitvec operator | ( const CSphSmallBitvec& dFirst, const CSphSmallBitvec& dSecond )
{
	if ( &dFirst==&dSecond )
		return dFirst;

	CSphSmallBitvec dResult;
	for ( int i=0; i<CSphSmallBitvec::IELEMENTS; i++ )
		dResult.m_dFieldsMask[i] = dFirst.m_dFieldsMask[i] | dSecond.m_dFieldsMask[i];
	return dResult;
}

inline bool operator == ( const CSphSmallBitvec& dFirst, const CSphSmallBitvec& dSecond )
{
	if ( &dFirst==&dSecond )
		return true;

	return !memcmp ( &dFirst.m_dFieldsMask, &dSecond.m_dFieldsMask, CSphSmallBitvec::iBYTESIZE );
}

inline bool operator != ( const CSphSmallBitvec& dFirst, const CSphSmallBitvec& dSecond )
{
	return !( dFirst==dSecond );
}

#if USE_WINDOWS
#pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////

/// generic dynamic bitvector
/// with a preallocated part for small-size cases, and a dynamic route for big-size ones
class CSphBitvec
{
protected:
	DWORD *		m_pData;
	DWORD		m_uStatic[4];
	int			m_iElements;

public:
	CSphBitvec ()
		: m_pData ( NULL )
		, m_iElements ( 0 )
	{}

	~CSphBitvec ()
	{
		if ( m_pData && m_pData!=m_uStatic )
			SafeDeleteArray ( m_pData );
	}

	void Init ( int iElements )
	{
		assert ( iElements>0 );
		m_iElements = iElements;
		if ( iElements > int(sizeof(m_uStatic)*8) )
		{
			int iSize = (m_iElements+31)/32;
			m_pData = new DWORD [ iSize ];
			memset ( m_pData, 0, sizeof(DWORD)*iSize );
		} else
		{
			m_pData = m_uStatic;
			for ( int i=0; i<int(sizeof(m_uStatic)/sizeof(m_uStatic[0])); i++ )
				m_uStatic[i] = 0;
		}
	}

	bool BitGet ( int iIndex ) const
	{
		assert ( m_pData );
		assert ( iIndex>=0 );
		assert ( iIndex<m_iElements );
		return ( m_pData [ iIndex>>5 ] & ( 1UL<<( iIndex&31 ) ) )!=0; // NOLINT
	}

	void BitSet ( int iIndex )
	{
		assert ( iIndex>=0 );
		assert ( iIndex<m_iElements );
		m_pData [ iIndex>>5 ] |= ( 1UL<<( iIndex&31 ) ); // NOLINT
	}

	void BitClear ( int iIndex )
	{
		assert ( iIndex>=0 );
		assert ( iIndex<m_iElements );
		m_pData [ iIndex>>5 ] &= ~( 1UL<<( iIndex&31 ) ); // NOLINT
	}
};

/// generic COM-like uids
enum ExtraData_e
{
	EXTRA_GET_DATA_ZONESPANS,
	EXTRA_GET_DATA_ZONESPANLIST,
	EXTRA_GET_DATA_RANKFACTORS,
	EXTRA_GET_DATA_PACKEDFACTORS,
	EXTRA_SET_MVAPOOL,
	EXTRA_SET_STRINGPOOL,
	EXTRA_SET_MAXMATCHES,
	EXTRA_SET_MATCHPUSHED,
	EXTRA_SET_MATCHPOPPED
};

/// generic COM-like interface
class ISphExtra
{
public:
	virtual						~ISphExtra () {}
	inline bool					ExtraData	( ExtraData_e eType, void** ppData )
	{
		return ExtraDataImpl ( eType, ppData );
	}
private:
	virtual bool ExtraDataImpl ( ExtraData_e, void** )
	{
		return false;
	}
};

#endif // _sphinxstd_

//
// $Id$
//
