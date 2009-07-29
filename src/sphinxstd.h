//
// $Id$
//

#ifndef _sphinxstd_
#define _sphinxstd_

#if _MSC_VER>=1400
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

#if (_MSC_VER >= 1000) && !defined(__midl) && defined(_PREFAST_)
typedef int __declspec("SAL_nokernel") __declspec("SAL_nodriver") __prefast_flag_kernel_driver_mode;
#endif

#if defined(_MSC_VER) && (_MSC_VER<1400)
#define vsnprintf _vsnprintf
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

#if USE_WINDOWS
// for intrinsic __rdtsc()
// must be included here, otherwise it breaks our assert macro
#include <intrin.h>
#else
#include <sys/mman.h>
#include <errno.h>
#include <pthread.h>
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
#include <windows.h>

#define strcasecmp			strcmpi
#define strncasecmp			_strnicmp
#define snprintf			_snprintf
#define strtoll				_strtoi64
#define strtoull			_strtoui64

#else

#if USE_ODBC
// UnixODBC compatible DWORD
#if defined(__alpha) || defined(__sparcv9) || defined(__LP64__) || (defined(__HOS_AIX__) && defined(_LP64))
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

STATIC_SIZE_ASSERT ( uint64_t, 8 );
STATIC_SIZE_ASSERT ( int64_t, 8 );

/////////////////////////////////////////////////////////////////////////////
// MEMORY MANAGEMENT
/////////////////////////////////////////////////////////////////////////////

#define SPH_DEBUG_LEAKS			0

#if SPH_DEBUG_LEAKS

/// debug new that tracks memory leaks
void *			operator new ( size_t iSize, const char * sFile, int iLine );

/// debug new that tracks memory leaks
void *			operator new [] ( size_t iSize, const char * sFile, int iLine );

/// get current allocs count
int				sphAllocsCount ();

/// total allocated bytes
int				sphAllocBytes ();

/// get last alloc id
int				sphAllocsLastID ();

/// dump all allocs since given id
void			sphAllocsDump ( int iFile, int iSinceID );

/// dump stats to stdout
void			sphAllocsStats ();

/// check all exitsing allocs; raises assertion failure in cases of errors
void			sphAllocsCheck ();

#undef new
#define new		new(__FILE__,__LINE__)

#endif // SPH_DEBUG_LEAKS

/// delete for my new
void			operator delete ( void * pPtr );

/// delete for my new
void			operator delete [] ( void * pPtr );

/////////////////////////////////////////////////////////////////////////////
// HELPERS
/////////////////////////////////////////////////////////////////////////////

typedef			bool (* SphDieCallback_t) ( const char * );

/// crash with an error message
void			sphDie ( const char * sMessage, ... );

/// setup a callback function to call from sphDie() before exit
/// if callback returns false, sphDie() will not log to stdout
void			sphSetDieCallback ( SphDieCallback_t pfDieCallback );

/// how much bits do we need for given int
inline int		sphLog2 ( uint64_t iValue )
{
	int iBits = 0;
	while ( iValue )
	{
		iValue >>= 1;
		iBits++;
	}
	return iBits;
}

/// float vs dword conversion
inline DWORD sphF2DW ( float f )	{ union { float f; DWORD d; } u; u.f = f; return u.d; }

/// dword vs float conversion
inline float sphDW2F ( DWORD d )	{ union { float f; DWORD d; } u; u.d = d; return u.f; }

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
		return ((&a)->*m_pMember) < ((&b)->*m_pMember);
	}
};

template < typename T, typename C >
inline SphMemberLess_T<T,C>
sphMemberLess ( T C::* pMember)
{
	return SphMemberLess_T<T,C> ( pMember );
}


/// generic accessor
template < typename T >
struct SphAccessor_T
{
	typedef T MEDIAN_TYPE;

	T & Get ( T * pBase, int iIndex ) const
	{
		return pBase[iIndex];
	}

	void GetMedian ( MEDIAN_TYPE & tMedian, const T & tValue ) const
	{
		tMedian = tValue;
	}

	void Swap ( T & a, T & b ) const
	{
		::Swap ( a, b );
	}
};


/// generic sort
template < typename T, typename U, typename V >
void sphSort ( T * pData, int iCount, U COMP, V ACC )
{
	int st0[32], st1[32], a, b, k, i, j;
	typename V::MEDIAN_TYPE x;

	k = 1;
	st0[0] = 0;
	st1[0] = iCount-1;
	while ( k )
	{
		k--;
		i = a = st0[k];
		j = b = st1[k];
		ACC.GetMedian ( x, ACC.Get ( pData, (a+b)/2 ) ); // FIXME! add better median at least
		while ( a<b )
		{
			while ( i<=j )
			{
				while ( COMP.IsLess ( ACC.Get ( pData, i ), x ) ) i++;
				while ( COMP.IsLess ( x, ACC.Get ( pData, j ) ) ) j--;
				if (i <= j) { ACC.Swap ( ACC.Get ( pData, i ), ACC.Get ( pData, j ) ); i++; j--; }
			}

			if ( j-a>=b-i )
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

						SphMemberFunctor_T ( T CLASS::* pMember )	: m_pMember ( pMember ) {}
	const T &			operator () ( const CLASS & arg ) const		{ return (&arg)->*m_pMember; }
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
		if ( tRef<tPred(*pStart) || tRef>tPred(*pEnd) )
			break;
		assert ( tRef>tPred(*pStart) );
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

/// generic vector
/// (don't even ask why it's not std::vector)
template < typename T, bool SWAP=false > class CSphVector
{
protected:
	static const int MAGIC_INITIAL_LIMIT = 8;

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
		if ( m_iLength>=m_iLimit )
			Reserve ( 1+m_iLength );
		m_pData [ m_iLength++ ] = tValue;
	}

	/// add unique entry (ie. do not add if equal to last one)
	void AddUnique ( const T & tValue )
	{
		if ( m_iLength>=m_iLimit )
			Reserve ( 1+m_iLength );

		if ( m_iLength==0 || m_pData[m_iLength-1]!=tValue )
			m_pData [ m_iLength++ ] = tValue;
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

private:
	template < typename U, bool V > struct CopyImpl
	{
		static inline void DoCopy ( U * pNew, U * pData, int iLength )
		{
			for ( int i=0; i<iLength; i++ )
				pNew[i] = pData[i];
		}
	};

	template < typename U > struct CopyImpl<U,true>
	{
		static inline void DoCopy ( U * pNew, U * pData, int iLength )
		{
			for ( int i=0; i<iLength; i++ )
				Swap ( pNew[i], pData[i] );
		}
	};

public:
	/// grow enough to hold that much entries, if needed, but do *not* change the length
	void Reserve ( int iNewLimit )
	{
		// check that we really need to be called
		assert ( iNewLimit>=0 );
		if ( iNewLimit<=m_iLimit )
			return;

		// calc new limit
		if ( !m_iLimit )
			m_iLimit = MAGIC_INITIAL_LIMIT;
		while ( m_iLimit<iNewLimit )
			m_iLimit *= 2;

		// realloc
		// FIXME! optimize for POD case
		T * pNew = new T [ m_iLimit ];
		__analysis_assume ( m_iLength<=m_iLimit );

		CopyImpl<T,SWAP>::DoCopy ( pNew, m_pData, m_iLength );
		delete [] m_pData;

		m_pData = pNew;
	}

	/// resize
	void Resize ( int iNewLength )
	{
		if ( iNewLength>=m_iLength )
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
	void SwapData ( CSphVector<T> & rhs )
	{
		Swap ( m_iLength, rhs.m_iLength );
		Swap ( m_iLimit, rhs.m_iLimit );
		Swap ( m_pData, rhs.m_pData );
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

	/// fill with given value
	void Fill ( const T & rhs )
	{
		for ( int i=0; i<m_iLength; i++ )
			m_pData[i] = rhs;
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

/////////////////////////////////////////////////////////////////////////////

/// simple dynamic hash
/// keeps the order, so Iterate() return the entries in the order they was inserted
template < typename T, typename KEY, typename HASHFUNC, int LENGTH, int STEP >
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
	/// returns false if this key is alredy hashed
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

		pEntry = new HashEntry_t ();
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

	/// copying
	const CSphOrderedHash<T,KEY,HASHFUNC,LENGTH,STEP> & operator = ( const CSphOrderedHash<T,KEY,HASHFUNC,LENGTH,STEP> & rhs )
	{
		Reset ();

		rhs.IterateStart ();
		while ( rhs.IterateNext() )
			Add ( rhs.IterateGet(), rhs.IterateGetKey() );

		return *this;
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

private:
	/// current iterator
	mutable HashEntry_t *	m_pIterator;
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

	bool operator == ( const CSphString & t ) const
	{
		return strcmp ( m_sValue, t.m_sValue )==0;
	}

	bool operator == ( const char * t ) const
	{
		return strcmp ( m_sValue, t )==0;
	}

	bool operator != ( const CSphString & t ) const
	{
		return strcmp ( m_sValue, t.m_sValue )!=0;
	}

	bool operator != ( const char * t ) const
	{
		return strcmp ( m_sValue, t )!=0;
	}

	CSphString ( const char * sString )
	{
		if ( sString )
		{
			int iLen = 1+strlen(sString);
			m_sValue = new char [ iLen+SAFETY_GAP ];

			strcpy ( m_sValue, sString );
			memset ( m_sValue+iLen, 0, SAFETY_GAP );
		} else
		{
			m_sValue = NULL;
		}
	}

	const CSphString & operator = ( const CSphString & rhs )
	{
		SafeDeleteArray ( m_sValue );
		if ( rhs.m_sValue )
		{
			int iLen = 1+strlen(rhs.m_sValue);
			m_sValue = new char [ iLen+SAFETY_GAP ];

			strcpy ( m_sValue, rhs.m_sValue );
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

	const CSphString & SetSprintf ( const char * sTemplate, ... )
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

	bool Ends ( const char * sPrefix ) const
	{
		if ( !m_sValue || !sPrefix )
			return false;

		int iVal = strlen ( m_sValue );
		int iPrefix = strlen ( sPrefix );
		if ( iVal<iPrefix )
			return false;
		return strncmp ( m_sValue+iVal-iPrefix, sPrefix, iPrefix )==0;
	}

	void Chop ()
	{
		if ( m_sValue )
		{
			const char * sStart = m_sValue;
			const char * sEnd = m_sValue + strlen(m_sValue) - 1;
			while ( sStart<=sEnd && isspace((unsigned char)*sStart) ) sStart++;
			while ( sStart<=sEnd && isspace((unsigned char)*sEnd) ) sEnd--;
			memmove ( m_sValue, sStart, sEnd-sStart+1 );
			m_sValue [ sEnd-sStart+1 ] = '\0';
		}
	}

	int Length () const
	{
		return m_sValue ? (int)strlen(m_sValue) : 0;
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
	float			m_fValue;

public:
	CSphVariant *	m_pNext;
	bool			m_bTag;

public:
	/// default ctor
	CSphVariant ()
		: CSphString ()
		, m_iValue ( 0 )
		, m_fValue ( 0.0f )
		, m_pNext ( NULL )
		, m_bTag ( false )
	{
	}

	/// ctor from C string
	CSphVariant ( const char * sString )
		: CSphString ( sString )
		, m_iValue ( atoi ( m_sValue ) )
		, m_fValue ( (float)atof ( m_sValue ) )
		, m_pNext ( NULL )
	{
	}

	/// copy ctor
	CSphVariant ( const CSphVariant & rhs )
		: CSphString ()
		, m_iValue ( 0 )
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
		m_fValue = rhs.m_fValue;

		return *this;
	}
};

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

protected:
	T *				m_pPtr;
};

//////////////////////////////////////////////////////////////////////////

/// refcounted base
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
template < typename T >
class CSphRefcountedPtr
{
public:
	explicit		CSphRefcountedPtr ( T * pPtr )	{ m_pPtr = pPtr; }
					~CSphRefcountedPtr ()			{ if ( m_pPtr ) m_pPtr->Release(); }

	T *				Ptr () const					{ return m_pPtr; }
	T *				operator -> () const			{ return m_pPtr; }
	bool			operator ! () const				{ return m_pPtr==NULL; }

public:
	CSphRefcountedPtr<T> &	operator = ( T * pPtr )								{ if ( m_pPtr ) m_pPtr->Release(); m_pPtr = pPtr; return *this; }
	CSphRefcountedPtr<T> &	operator = ( const CSphRefcountedPtr<T> & rhs )		{ if ( rhs.m_pPtr ) rhs.m_pPtr->AddRef(); if ( m_pPtr ) m_pPtr->Release(); m_pPtr = rhs.m_pPtr; return *this; }

protected:
	T *				m_pPtr;
};

//////////////////////////////////////////////////////////////////////////

extern bool g_bHeadProcess;
void sphWarn ( const char *, ... );

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
	bool Alloc ( DWORD iEntries, CSphString & sError, CSphString & )
#else
	bool Alloc ( DWORD iEntries, CSphString & sError, CSphString & sWarning )
#endif
	{
		assert ( !m_pData );

		int64_t uCheck = sizeof(T);
		uCheck *= (int64_t)iEntries;

		m_iLength = (size_t)uCheck;
		if ( uCheck!=(int64_t)m_iLength )
		{
			sError.SetSprintf ( "impossible to mmap() over 4 GB on 32-bit system" );
			m_iLength = 0;
			return false;
		}

#if USE_WINDOWS
		m_pData = new T [ iEntries ];
#else
		m_pData = (T *) mmap ( NULL, m_iLength, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANON, -1, 0 );
		if ( m_pData==MAP_FAILED )
		{
			if ( m_iLength>0x7fffffffUL )
				sError.SetSprintf ( "mmap() failed: %s (length=%"PRIi64" is over 2GB, impossible on some 32-bit systems)", strerror(errno), (int64_t)m_iLength );
			else
				sError.SetSprintf ( "mmap() failed: %s (length=%"PRIi64")", strerror(errno), (int64_t)m_iLength );
			m_iLength = 0;
			return false;
		}

		if ( m_bMlock )
			if ( -1==mlock ( m_pData, m_iLength ) )
				sWarning.SetSprintf ( "mlock() failed: %s", strerror(errno) );
#endif // USE_WINDOWS

		assert ( m_pData );
		m_iEntries = iEntries;
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
			sError.SetSprintf ( "%s mlock() failed: bytes=%"PRIu64", error=%s", sPrefix, (uint64_t)m_iLength, strerror(errno) );
		else
			sError.SetSprintf ( "%s; %s mlock() failed: bytes=%"PRIu64", error=%s", sError.cstr(), sPrefix, (uint64_t)m_iLength, strerror(errno) );
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
		}
#endif // USE_WINDOWS

		m_pData = NULL;
		m_iLength = 0;
		m_iEntries = 0;
	}

public:
	/// accessor
	inline const T & operator [] ( DWORD iIndex ) const
	{
		assert ( iIndex>=0 && iIndex<m_iEntries );
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
	DWORD GetNumEntries () const
	{
		return m_iEntries;
	}

protected:
	T *					m_pData;	///< data storage
	size_t				m_iLength;	///< data length, bytes
	DWORD				m_iEntries;	///< data length, entries
	bool				m_bMlock;	///< whether to lock data in RAM
};

//////////////////////////////////////////////////////////////////////////

/// process-shared mutex that survives fork
class CSphProcessSharedMutex
{
public:
	CSphProcessSharedMutex ();
	void	Lock ();
	void	Unlock ();

protected:
#if !USE_WINDOWS
	CSphSharedBuffer<BYTE>		m_pStorage;
	pthread_mutex_t *			m_pMutex;
#endif
};

//////////////////////////////////////////////////////////////////////////

/// my thread handle and thread func magic
#if USE_WINDOWS
typedef HANDLE SphThread_t;
typedef DWORD SphThreadKey_t;
#else
typedef pthread_t SphThread_t;
typedef pthread_key_t SphThreadKey_t;
#endif

/// my create thread wrapper
bool sphThreadCreate ( SphThread_t * pThread, void (*fnThread)(void*), void * pArg );

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

/// set thread-local key value
bool sphThreadSet ( SphThreadKey_t tKey, void * pValue );

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
#endif
};


/// rwlock implementation
class CSphRwlock
{
public:
	CSphRwlock ();
	~CSphRwlock () {}

	bool Init ();
	bool Done ();

	bool ReadLock ();
	bool WriteLock ();
	bool Unlock ();

#if USE_WINDOWS
private:
	HANDLE				m_hWriteMutex;
	HANDLE				m_hReadEvent;
	LONG				m_iReaders;
#else
	pthread_rwlock_t	m_tLock;
#endif
};

#endif // _sphinxstd_

//
// $Id$
//
