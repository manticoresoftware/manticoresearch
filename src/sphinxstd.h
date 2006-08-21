//
// $Id$
//

#ifndef _sphinxstd_
#define _sphinxstd_

#if _MSC_VER>=1400
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

#include <string.h>
#include <stdlib.h>
#include <assert.h>

/////////////////////////////////////////////////////////////////////////////
// GENERICS
/////////////////////////////////////////////////////////////////////////////

#define Min(a,b)			((a)<(b)?(a):(b))
#define Max(a,b)			((a)>(b)?(a):(b))
#define SafeDelete(_x)		{ if (_x) { delete (_x); (_x) = NULL; } }
#define SafeDeleteArray(_x)	{ if (_x) { delete [] (_x); (_x) = NULL; } }

/// swap
template < typename T > inline void Swap ( T & v1, T & v2 )
{
	T temp = v1;
	v1 = v2;
	v2 = temp;
}

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

/// compile time error struct
template<int> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

namespace Private
{
	template<int x>		struct static_assert_test{};
	template<bool x>	struct SizeError;
	template<>			struct SizeError<true>{};
}

#define STATIC_SIZE_ASSERT(_Type,_ExpSize)	\
	typedef ::Private::static_assert_test<sizeof(::Private::SizeError \
	< (bool) (sizeof(_Type) == (_ExpSize)) >)> static_assert_typedef_

/////////////////////////////////////////////////////////////////////////////

/// generic vector
/// (don't even ask why it's not std::vector)
template < typename T, int INITIAL_LIMIT=1024 > class CSphVector
{
public:
	/// ctor
	CSphVector ()
		: m_iLength	( 0 )
		, m_iLimit	( 0 )
		, m_pData	( NULL )
	{
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
			Grow ( 1+m_iLength );
		return m_pData [ m_iLength++ ];
	}

	/// add entry
	void Add ( const T & tValue )
	{
		if ( m_iLength>=m_iLimit )
			Grow ( 1+m_iLength );
		m_pData [ m_iLength++ ] = tValue;
	}

	/// get last entry
	T & Last ()
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

	/// grow enough to hold that much entries, if needed
	void Grow ( int iNewLimit )
	{
		// check that we really need to be called
		assert ( iNewLimit>=0 );
		if ( iNewLimit<=m_iLimit )
			return;

		// calc new limit
		if ( !m_iLimit )
			m_iLimit = INITIAL_LIMIT;
		while ( m_iLimit<iNewLimit )
			m_iLimit *= 2;

		// realloc
		// FIXME! optimize for POD case
		T * pNew = new T [ m_iLimit ];
		for ( int i=0; i<m_iLength; i++ )
			pNew[i] = m_pData[i];
		delete [] m_pData;
		m_pData = pNew;
	}

	/// resize
	void Resize ( int iNewLength )
	{
		if ( iNewLength>=m_iLength )
			Grow ( iNewLength );
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

	/// sort
	void Sort ( int iStart=0, int iEnd=-1 )
	{
		_Sort<false> ( iStart, iEnd );
	}

	/// reverse sort
	void RSort ( int iStart=0, int iEnd=-1 )
	{
		_Sort<true> ( iStart, iEnd );
	}

	/// sort
	template < typename F > void Sort ( F COMP, int iStart=0, int iEnd=-1 )
	{
		if ( m_iLength<2 ) return;
		if ( iStart<0 ) iStart = m_iLength+iStart;
		if ( iEnd<0 ) iEnd = m_iLength+iEnd;
		assert ( iStart<=iEnd );

		int st0[32], st1[32], a, b, k, i, j;
		T x;

		k = 1;
		st0[0] = iStart;
		st1[0] = iEnd;
		while ( k )
		{
			k--;
			i = a = st0[k];
			j = b = st1[k];
			x = m_pData [ (a+b)/2 ]; // FIXME! add better median at least
			while ( a<b )
			{
				while ( i<=j )
				{
					while ( COMP ( m_pData[i], x ) ) i++;
					while ( COMP ( x, m_pData[j] ) ) j--;
					if (i <= j) { Swap ( m_pData[i], m_pData[j] ); i++; j--; }
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

	/// access
	/// FIXME! optimize for POD case
	const T & operator [] ( int iIndex ) const
	{
		assert ( iIndex>=0 && iIndex<m_iLength );
		return m_pData [ iIndex ];
	}

	/// access
	/// FIXME! optimize for POD case
	T & operator [] ( int iIndex )
	{
		assert ( iIndex>=0 && iIndex<m_iLength );
		return m_pData [ iIndex ];
	}

	/// copy
	const CSphVector < T, INITIAL_LIMIT > & operator = ( const CSphVector < T, INITIAL_LIMIT > & rhs )
	{
		Reset ();

		m_iLength = rhs.m_iLength;
		m_iLimit = rhs.m_iLimit;
		m_pData = new T [ m_iLimit ];
		for ( int i=0; i<rhs.m_iLength; i++ )
			m_pData[i] = rhs.m_pData[i];

		return *this;
	}

private:
	int		m_iLength;		///< entries actually used
	int		m_iLimit;		///< entries allocated
	T *		m_pData;		///< entries

private:
	/// sort the array
	template < bool REVERSE > void _Sort ( int iStart, int iEnd )
	{
		if ( m_iLength<2 ) return;
		if ( iStart<0 ) iStart = m_iLength+iStart;
		if ( iEnd<0 ) iEnd = m_iLength+iEnd;
		assert ( iStart<=iEnd );

		int st0[32], st1[32], a, b, k, i, j;
		T x;

		k = 1;
		st0[0] = iStart;
		st1[0] = iEnd;
		while ( k )
		{
			k--;
			i = a = st0[k];
			j = b = st1[k];
			x = m_pData [ (a+b)/2 ]; // FIXME! add better median at least
			while ( a<b )
			{
				while ( i<=j )
				{
					#if USE_WINDOWS
					#pragma warning(disable:4127)
					#endif
					if ( !REVERSE )
					#if USE_WINDOWS
					#pragma warning(default:4127)
					#endif
					{
						while ( m_pData[i]<x ) i++;
						while ( x<m_pData[j] ) j--;
					} else
					{
						while ( x<m_pData[i] ) i++;
						while ( m_pData[j]<x ) j--;
					}
					if (i <= j) { Swap ( m_pData[i], m_pData[j] ); i++; j--; }
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
};


#define ARRAY_FOREACH(_index,_array) \
	for ( int _index=0; _index<_array.GetLength(); _index++ )

/////////////////////////////////////////////////////////////////////////////

/// simple generic hash
/// keeps the order, so Iterate() return the entries in the order they was inserted
template < typename T, typename KEY, typename HASHFUNC, int LENGTH, int INCREASE >
class CSphGenericHash
{
protected:
	struct HashEntry_t
	{
		T 				m_tData;			///< data, owned by the hash
		KEY				m_tKey;				///< key, owned by the hash
		bool			m_bDeleted;			///< is this entry deleted
		bool			m_bEmpty;			///< is this entry empty
		int				m_iNext;			///< next entry in the insertion order
	};

protected:
	HashEntry_t	*	m_pHash;				///< all the hash entries
	int				m_iFirst;				///< first entry inserted
	int				m_iLast;				///< last entry inserted

protected:
	/// this function finds hash entry by it's key.
	template < bool STOP_ON_DELETED >
	const HashEntry_t * Search ( const KEY & key ) const
	{
		if ( !m_pHash )
			return NULL;

		int index = HASHFUNC() ( key ) ;
		assert ( index >= 0 && index < LENGTH );

		for ( int count = 0; count < LENGTH * 2 / 3; count++ )
		{
			// deleted slot
			if ( m_pHash[index].m_bDeleted )
			{
				// found deleted slot
				#if USE_WINDOWS
				#pragma warning(disable:4127)
				#endif
				if ( STOP_ON_DELETED )
				#if USE_WINDOWS
				#pragma warning(default:4127)
				#endif
					return m_pHash + index;
			}
			else
			{
				// empty slot
				if ( m_pHash[index].m_bEmpty )
					return m_pHash + index;

				// found the same
				if ( m_pHash[index].m_tKey == key )
					return m_pHash + index;
			}

			// next index
			index = ( index+INCREASE ) & ( LENGTH-1 );
		}

		return NULL;
	}

public:
	/// ctor
	CSphGenericHash ()
		: m_pHash ( NULL )
		, m_iFirst ( -1 )
		, m_iLast ( -1 )
		, m_iIterator ( -1 )
	{
	}

	/// dtor
	~CSphGenericHash ()
	{
		Reset ();
	}

	/// reset
	void Reset ()
	{
		SafeDeleteArray ( m_pHash );
	}

	/// add new entry
	void Add ( const T & tData, const KEY & tKey )
	{
		bool bFirst = false;
		if ( !m_pHash )
		{
			Create ();
			bFirst = true;
		}

		HashEntry_t * pEntry = const_cast<HashEntry_t *> ( Search<true> ( tKey ) );
		assert ( pEntry && "hash overflow" );
		assert ( pEntry->m_bEmpty && "already hashed" );

		pEntry->m_tData		= tData;
		pEntry->m_tKey		= tKey;
		pEntry->m_bDeleted	= false;
		pEntry->m_bEmpty	= false;

		if ( bFirst )
		{
			m_iFirst = pEntry-m_pHash;
			m_iLast = m_iFirst;
		} else
		{
			assert ( m_iLast>=0 && m_iLast<LENGTH );
			m_pHash[m_iLast].m_iNext = pEntry-m_pHash;
			m_iLast = pEntry-m_pHash;
		}
	}

	/// check if key exists
	bool Exists ( const KEY & key ) const
	{
		const HashEntry_t * pEntry = Search<false> ( key );
		return !( pEntry==NULL || pEntry->m_bEmpty );
	}

	/// get value pointer by key
	T * operator () ( const KEY & tKey ) const
	{
		HashEntry_t * pEntry = const_cast<HashEntry_t *> ( Search<false> ( tKey ) );
		if ( pEntry && !pEntry->m_bEmpty )
			return &pEntry->m_tData;
		return NULL;
	}

	/// get value reference by key, asserting that the key exists in hash
	T & operator [] ( const KEY & tKey ) const
	{
		HashEntry_t * pEntry = const_cast<HashEntry_t *> ( Search<false> ( tKey ) );
		if ( pEntry && !pEntry->m_bEmpty )
			return pEntry->m_tData;

		assert ( 0 && "hash missing value in operator []" );
		return m_pHash[0].m_tData;
	}

	/// copying
	const CSphGenericHash<T,KEY,HASHFUNC,LENGTH,INCREASE> & operator = ( const CSphGenericHash<T,KEY,HASHFUNC,LENGTH,INCREASE> & rhs )
	{
		SafeDeleteArray ( m_pHash );
		if ( rhs.m_pHash )
		{
			Create ();
			for ( int i=0; i<LENGTH; i++ )
			{
				// FIXME! check if assignment operator gets called on key/data, and remove this
				m_pHash[i].m_tKey = rhs.m_pHash[i].m_tKey;
				m_pHash[i].m_tData = rhs.m_pHash[i].m_tData;
				m_pHash[i].m_bDeleted = rhs.m_pHash[i].m_bDeleted;
				m_pHash[i].m_bEmpty = rhs.m_pHash[i].m_bEmpty;
				m_pHash[i].m_iNext = rhs.m_pHash[i].m_iNext;
			}
		}

		m_iFirst = rhs.m_iFirst;
		m_iLast = rhs.m_iLast;
		return *this;
	}

public:
	/// start iterating
	void IterateStart () const
	{
		m_iIterator = -1;
	}

	/// go to next existing entry
	bool IterateNext () const
	{
		if ( !m_pHash )
			return false;

		if ( m_iIterator<0 )
		{
			assert ( m_iFirst>=0 );
			m_iIterator = m_iFirst;
		} else
		{
			assert ( m_iIterator>=0 && m_iIterator<LENGTH );
			m_iIterator = m_pHash[m_iIterator].m_iNext;
			if ( m_iIterator<0 )
				return false;
		}

		return true;
	}

	/// get entry value
	T & IterateGet () const
	{
		assert ( m_pHash );
		assert ( m_iIterator>=0 && m_iIterator<LENGTH );
		return m_pHash[m_iIterator].m_tData;
	}

	/// get entry key
	const KEY & IterateGetKey () const
	{
		assert ( m_pHash );
		assert ( m_iIterator>=0 && m_iIterator<LENGTH );
		return m_pHash[m_iIterator].m_tKey;
	}

protected:
	/// allocate the hash on demand
	void Create ()
	{
		assert ( !m_pHash );
		m_pHash = new HashEntry_t [ LENGTH ];
		for ( int i=0; i<LENGTH; i++ )
		{
			m_pHash[i].m_bEmpty = true;
			m_pHash[i].m_bDeleted = false;
			m_pHash[i].m_iNext = -1;
		}
	}

private:
	/// current iterator
	mutable int		m_iIterator;
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

	CSphString SubString ( int iStart, int iCount )
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

	bool IsEmpty () const
	{
		if ( !m_sValue )
			return true;
		return ( (*m_sValue)=='\0' );
	}
};

/////////////////////////////////////////////////////////////////////////////

/// immutable string/int variant list proxy
struct CSphVariant : public CSphString
{
protected:
	int				m_iValue;

public:
	CSphVariant *	m_pNext;
	bool			m_bTag;

public:
	/// default ctor
	CSphVariant ()
		: CSphString ()
		, m_iValue ( 0 )
		, m_pNext ( NULL )
		, m_bTag ( false )
	{
	}

	/// ctor from C string
	CSphVariant ( const char * sString )
		: CSphString ( sString )
		, m_iValue ( atoi ( m_sValue ) )
		, m_pNext ( NULL )
	{
	}

	/// copy ctor
	CSphVariant ( const CSphVariant & rhs )
		: CSphString ()
		, m_iValue ( 0 )
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

	/// default copy operator
	const CSphVariant & operator = ( const CSphVariant & rhs )
	{
		assert ( !m_pNext );
		if ( rhs.m_pNext )
			m_pNext = new CSphVariant ( *rhs.m_pNext );

		CSphString::operator = ( rhs );
		m_iValue = rhs.m_iValue;

		return *this;
	}
};

/////////////////////////////////////////////////////////////////////////////

/// match-sorting priority min-queue interface
template < typename T > class ISphQueue
{
public:
	/// virtualizing dtor
	virtual				~ISphQueue () {}

	/// base push
	virtual void		Push ( const T & tEntry ) = 0;

	/// base pop
	virtual void		Pop () = 0;

	/// get entries count
	virtual int			GetLength () const = 0;

	/// get current root
	virtual const T &	Root () const = 0;

public:
	/// match-queue specific, set if this queue needs attr values
						ISphQueue ( bool bUsesAttrs ) : m_bUsesAttrs ( bUsesAttrs ) {}

	/// match-queue specific, check if this queue needs attr values
	bool				UsesAttrs () { return m_bUsesAttrs; }

	/// match-queue specific, set attr to sort by
	virtual void		SetAttr ( int ) {};

	/// match-queue specific, get attr to sort by
	virtual int			GetAttr () { return 0; }

protected:
	bool				m_bUsesAttrs;	///<  match-queue specific, whether this queue uses attrs
};

#endif // _sphinxstd_

//
// $Id$
//
